//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Utility code.
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "IEffects.h"
#include "fx.h"
#include "c_te_legacytempents.h"

//-----------------------------------------------------------------------------
// Client-server neutral effects interface
//-----------------------------------------------------------------------------
class CEffectsClient : public IEffects
{
public:
	CEffectsClient();
	virtual ~CEffectsClient();

	// Members of the IEffect interface
	virtual void Beam( const Vector &Start, const Vector &End, int nModelIndex, 
		int nHaloIndex, unsigned char frameStart, unsigned char frameRate,
		float flLife, unsigned char width, unsigned char endWidth, unsigned char fadeLength, 
		unsigned char noise, unsigned char red, unsigned char green,
		unsigned char blue, unsigned char brightness, unsigned char speed);
	virtual void Smoke( const Vector &origin, int modelIndex, float scale, float framerate );
	virtual void Sparks( const Vector &position, int nMagnitude = 1, int nTrailLength = 1, const Vector *pvecDir = NULL );
	virtual void Dust( const Vector &pos, const Vector &dir, float size, float speed );
	virtual void MuzzleFlash( const Vector &origin, const QAngle &angles, float fScale, int type );
	virtual void MetalSparks( const Vector &position, const Vector &direction ); 
	virtual void EnergySplash( const Vector &position, const Vector &direction, bool bExplosive = false );
	virtual void Ricochet( const Vector &position, const Vector &direction );

	// FIXME: Should these methods remain in this interface? Or go in some 
	// other client-server neutral interface?
	virtual float Time();
	virtual bool IsServer();
private:
	//-----------------------------------------------------------------------------
	// Purpose: Returning true means don't even call TE func
	// Input  : filter - 
	//			*suppress_host - 
	// Output : static bool
	//-----------------------------------------------------------------------------
	bool SuppressTE( C_RecipientFilter& filter )
	{
		if ( !CanPredict() )
			return true;

		if ( !filter.GetRecipientCount() )
		{
			// Suppress it
			return true;
		}

		// There's at least one recipient
		return false;
	}
};


//-----------------------------------------------------------------------------
// Client-server neutral effects interface accessor
//-----------------------------------------------------------------------------
static CEffectsClient s_EffectClient;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEffectsClient, IEffects, IEFFECTS_INTERFACE_VERSION, s_EffectClient);
IEffects *g_pEffects = &s_EffectClient;


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CEffectsClient::CEffectsClient()
{
}

CEffectsClient::~CEffectsClient()
{
}


//-----------------------------------------------------------------------------
// Generates a beam
//-----------------------------------------------------------------------------
void CEffectsClient::Beam( const Vector &vecStartPoint, const Vector &vecEndPoint, 
	int nModelIndex, int nHaloIndex, unsigned char frameStart, unsigned char nFrameRate,
	float flLife, unsigned char nWidth, unsigned char nEndWidth, unsigned char nFadeLength, 
	unsigned char noise, unsigned char r, unsigned char g,
	unsigned char b, unsigned char brightness, unsigned char nSpeed)
{
	Assert(0);
//	CBroadcastRecipientFilter filter;
//	if ( !SuppressTE( filter ) )
//	{
//	beams->CreateBeamPoints( vecStartPoint, vecEndPoint, nModelIndex, nHaloIndex, 
//		m_fHaloScale, 
//		flLife, 0.1 * nWidth,  0.1 * nEndWidth, nFadeLength, 0.01 * nAmplitude, a, 0.1 * nSpeed, 
//		m_nStartFrame, 0.1 * nFrameRate, r, g, b );
//	}
}


//-----------------------------------------------------------------------------
// Generates various tempent effects
//-----------------------------------------------------------------------------
void CEffectsClient::Smoke( const Vector &vecOrigin, int modelIndex, float scale, float framerate )
{
	CPVSFilter filter( vecOrigin );
	if ( !SuppressTE( filter ) )
	{
		tempents->Sprite_Smoke( tempents->DefaultSprite( vecOrigin, modelIndex, framerate ), scale * 0.1f );
	}
}

void CEffectsClient::Sparks( const Vector &position, int nMagnitude, int nTrailLength, const Vector *pVecDir )
{
	CPVSFilter filter( position );
	if ( !SuppressTE( filter ) )
	{
		FX_ElectricSpark( position, nMagnitude, nTrailLength, pVecDir );
	}
}

void CEffectsClient::Dust( const Vector &pos, const Vector &dir, float size, float speed )
{
	CPVSFilter filter( pos );
	if ( !SuppressTE( filter ) )
	{
		FX_Dust( pos, dir, size, speed );
	}
}

void CEffectsClient::MuzzleFlash( const Vector &vecOrigin, const QAngle &vecAngles, float flScale, int iType )
{
	CPVSFilter filter( vecOrigin );
	if ( !SuppressTE( filter ) )
	{
		switch( iType )
		{
		case MUZZLEFLASH_TYPE_DEFAULT:
			FX_MuzzleEffect( vecOrigin, vecAngles, flScale, 0 );
			break;

		case MUZZLEFLASH_TYPE_GUNSHIP:
			FX_GunshipMuzzleEffect( vecOrigin, vecAngles, flScale, 0 );
			break;
		
		default:
			Msg("No case for Muzzleflash type: %d\n", iType );
			break;
		}
	}
}

void CEffectsClient::MetalSparks( const Vector &position, const Vector &direction )
{
	CPVSFilter filter( position );
	if ( !SuppressTE( filter ) )
	{
		FX_MetalSpark( position, direction );
	}
}

void CEffectsClient::EnergySplash( const Vector &position, const Vector &direction, bool bExplosive )
{
	CPVSFilter filter( position );
	if ( !SuppressTE( filter ) )
	{
		FX_EnergySplash( position, direction, bExplosive );
	}
}

void CEffectsClient::Ricochet( const Vector &position, const Vector &direction )
{
	CPVSFilter filter( position );
	if ( !SuppressTE( filter ) )
	{
		FX_MetalSpark( position, direction );
		FX_RicochetSound( position );
	}
}

// FIXME: Should these methods remain in this interface? Or go in some 
// other client-server neutral interface?
float CEffectsClient::Time()
{
	return gpGlobals->curtime;
}


bool CEffectsClient::IsServer()
{
	return false;
}


