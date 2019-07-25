//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "c_baseentity.h"
#include "typeinfo.h"
#include "tier0/vprof.h"


//-----------------------------------------------------------------------------
// Invalidates the abs state of all children
//-----------------------------------------------------------------------------
void C_BaseEntity::InvalidatePhysicsRecursive( int nDirtyFlag, int nAdditionalChildFlag )
{
	m_iEFlags |= nDirtyFlag;
	nDirtyFlag |= nAdditionalChildFlag; 
	for (C_BaseEntity *pChild = FirstMoveChild(); pChild; pChild = pChild->NextMovePeer())
	{
		pChild->InvalidatePhysicsRecursive( nDirtyFlag );
	}
}


//-----------------------------------------------------------------------------
// Makes sure the entity is in the spatial partition in the right place
//-----------------------------------------------------------------------------
void C_BaseEntity::Relink()
{
	VPROF( "C_BaseEntity::Relink" );
	SetObjectCollisionBox();
	partition->ElementMoved( m_Partition, GetAbsMins(), GetAbsMaxs() );

	// Update the shadow...
	// FIXME: Should this call not actually need to be made?
	g_pClientShadowMgr->UpdateShadow( m_ShadowHandle );
}

//-----------------------------------------------------------------------------
// Purpose: What we're standing upon
// Input  : *ground - 
//-----------------------------------------------------------------------------
void C_BaseEntity::SetGroundEntity( C_BaseEntity *ground )
{
	m_hGroundEntity = ground;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : C_BaseEntity
//-----------------------------------------------------------------------------
C_BaseEntity *C_BaseEntity::GetGroundEntity( void )
{
	return m_hGroundEntity;
}


//-----------------------------------------------------------------------------
// helper method for trace hull as used by physics...
//-----------------------------------------------------------------------------

static void Physics_TraceHull( C_BaseEntity* pBaseEntity, const Vector &vecStart,
	const Vector &vecEnd, const Vector &hullMin, const Vector &hullMax,	
	unsigned int mask, trace_t *ptr )
{
	// FIXME: I really am not sure the best way of doing this
	// The TraceHull code below for shots will make sure the object passes
	// through shields which do not block that damage type. It will also 
	// send messages to the shields that they've been hit.
#if 0
	if (pBaseEntity->GetDamageType() != DMG_GENERIC)
	{
		GameRules()->WeaponTraceHull( vecStart, vecEnd, hullMin, hullMax, 
			mask, pBaseEntity, pBaseEntity->GetCollisionGroup(), 
			pBaseEntity, ptr );
	}
	else
#endif
	{
		UTIL_TraceHull( vecStart, vecEnd, hullMin, hullMax, mask, 
			pBaseEntity, pBaseEntity->GetCollisionGroup(), ptr );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Does not change the entities velocity at all
// Input  : push - 
// Output : trace_t
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsCheckSweep( const Vector& vecAbsStart, const Vector &vecAbsDelta, trace_t *pTrace )
{
	unsigned int mask = PhysicsSolidMaskForEntity();

	Vector vecAbsEnd;
	VectorAdd( vecAbsStart, vecAbsDelta, vecAbsEnd );

	// Set collision type
	if ( !IsSolid() || IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
	{
		// don't collide with monsters
		mask &= ~CONTENTS_MONSTER;
	}

	Physics_TraceHull( this, vecAbsStart, vecAbsEnd, WorldAlignMins(), WorldAlignMaxs(), mask, pTrace );
}




//-----------------------------------------------------------------------------
// Purpose: 
// Input  : push - 
// Output : trace_t
//-----------------------------------------------------------------------------
trace_t C_BaseEntity::PhysicsPushEntity( const Vector& push )
{

/*
	if ( m_pMoveParent )
	{
		Warning( "pushing entity (%s) that has m_pMoveParent!\n", STRING( pev->classname ) );
		Assert(0);
	}
*/

	// NOTE: absorigin and origin must be equal because there is no moveparent
	Vector prevOrigin;
	VectorCopy( GetAbsOrigin(), prevOrigin );

	trace_t		trace;
	PhysicsCheckSweep( prevOrigin, push, &trace );

	if ( trace.fraction )
	{
		SetAbsOrigin( trace.endpos );
	}

	// CLIENT DLL HACKS
	m_vecNetworkOrigin = GetLocalOrigin();
	m_angNetworkAngles = GetLocalAngles();

	m_iEFlags |= EFL_DIRTY_ABSTRANSFORM;
	Relink();

	if ( trace.m_pEnt )
	{
		PhysicsImpact( trace.m_pEnt, trace );
	}

	return trace;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNewPosition - 
//			*pNewVelocity - 
//			*pNewAngles - 
//			*pNewAngVelocity - 
//-----------------------------------------------------------------------------
void C_BaseEntity::PerformCustomPhysics( Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity )
{
	// If you're going to use custom physics, you need to implement this!
	Assert(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsCustom()
{
	PhysicsCheckWater();

	// regular thinking
	if ( !PhysicsRunThink() )
		return;

	// Moving upward, off the ground, or  resting on something that isn't ground
	if ( m_vecVelocity[2] > 0 || !GetGroundEntity() || !GetGroundEntity()->IsStandable() )
	{
		RemoveFlag( FL_ONGROUND );
	}

	// NOTE: The entity must set the position, angles, velocity in its custom movement
	Vector vecNewPosition = GetAbsOrigin();

	if ( vecNewPosition == vec3_origin )
	{
		// Shouldn't be at world origin
		Assert( 0 );
	}

	Vector vecNewVelocity = m_vecVelocity;
	QAngle angNewAngles = GetAbsAngles();
	QAngle angNewAngVelocity = m_vecAngVelocity;

	PerformCustomPhysics( &vecNewPosition, &vecNewVelocity, &angNewAngles, &angNewAngVelocity );

	// Store off all of the new state information...
	m_vecVelocity = vecNewVelocity;
	SetAbsAngles( angNewAngles );
	m_vecAngVelocity = angNewAngVelocity;

	Vector move;
	VectorSubtract( vecNewPosition, GetAbsOrigin(), move );

	// move origin
	trace_t trace;
	trace = PhysicsPushEntity( move );

	PhysicsCheckVelocity();

	if (trace.allsolid)
	{	
		// entity is trapped in another solid
		// UNDONE: does this entity needs to be removed?
		VectorCopy (vec3_origin, m_vecVelocity);
		VectorCopy (vec3_angle, m_vecAngVelocity);
		return;
	}
	
#if !defined( CLIENT_DLL )
	if (pev->free)
		return;
#endif

	// check for in water
	PhysicsCheckWaterTransition();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsStep()
{
	// Run all but the base think function
	PhysicsRunThink( THINK_FIRE_ALL_BUT_BASE );
PhysicsRunThink( THINK_FIRE_BASE_ONLY );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsNoclip( void )
{
	PhysicsRunThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsNone( void )
{
	PhysicsRunThink();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsPusher( void )
{
	PhysicsRunThink();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsParent( void )
{
	PhysicsRunThink();
}

//-----------------------------------------------------------------------------
// Purpose: Not yet supported on client .dll
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_BaseEntity::StartTouch( C_BaseEntity *pOther )
{
	// notify parent
//	if ( m_pParent != NULL )
//		m_pParent->StartTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: Call touch function if one is set
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_BaseEntity::Touch( C_BaseEntity *pOther )
{ 
	if ( m_pfnTouch ) 
		(this->*m_pfnTouch)( pOther );

	// notify parent of touch
//	if ( m_pParent != NULL )
//		m_pParent->Touch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: Call end touch
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_BaseEntity::EndTouch( C_BaseEntity *pOther )
{
	// notify parent
//	if ( m_pParent != NULL )
//	{
//		m_pParent->EndTouch( pOther );
//	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseEntity::IsCurrentlyTouching( void ) const
{
	if ( m_EntitiesTouched.nextLink != &m_EntitiesTouched )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : check - 
//-----------------------------------------------------------------------------
void C_BaseEntity::SetCheckUntouch( bool check )
{
	// Invalidate touchstamp
	if ( check )
	{
		touchStamp++;
	}
	m_bCheckUntouch = check;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseEntity::GetCheckUntouch() const
{
	return m_bCheckUntouch;
}

extern ConVar think_limit;

//-----------------------------------------------------------------------------
// Purpose: Called when it's time for a physically moved objects (plats, doors, etc)
//			to run it's game code.
//			All other entity thinking is done during worldspawn's think
//-----------------------------------------------------------------------------
void C_BaseEntity::PhysicsDispatchThink( BASEPTR thinkFunc )
{
	float thinkLimit = think_limit.GetFloat();
	float startTime = 0.0;

	/*
	// This doesn't apply on the client, really
	if ( IsDormant() )
	{
		Warning( "Dormant entity %s is thinking!!\n", GetClassname() );
		Assert(0);
	}
	*/

	if ( thinkLimit )
	{
		startTime = engine->Time();
	}
	
	if ( thinkFunc )
	{
		(this->*thinkFunc)();
	}

	if ( thinkLimit )
	{
		// calculate running time of the AI in milliseconds
		float time = ( engine->Time() - startTime ) * 1000.0f;
		if ( time > thinkLimit )
		{
#if 0
			// If its an NPC print out the shedule/task that took so long
			CAI_BaseNPC *pNPC = MyNPCPointer();
			if (pNPC && pNPC->GetCurSchedule())
			{
				pNPC->ReportOverThinkLimit( time );
			}
			else
#endif
			{
				Msg( "CLIENT:  %s(%s) thinking for %.02f ms!!!\n", GetClassname(), typeid(this).raw_name(), time );
			}
		}
	}
}