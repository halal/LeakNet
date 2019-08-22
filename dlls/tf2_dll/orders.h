//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Order handling
//
// $NoKeywords: $
//=============================================================================

#ifndef ORDERS_H
#define ORDERS_H
#ifdef _WIN32
#pragma once
#endif


class CTFTeam;
class CBaseTFPlayer;


#include "order_events.h"


//-----------------------------------------------------------------------------
// Purpose: Datatable container class for orders
//-----------------------------------------------------------------------------
class COrder : public CBaseEntity
{
public:
	DECLARE_CLASS( COrder, CBaseEntity );

	DECLARE_SERVERCLASS();

					COrder();
	virtual	void	UpdateOnRemove( void );

	virtual bool	ShouldTransmit( const edict_t *recipient, const void *pvs, int clientArea );
	
	// This is called when removing the order.
	void			DetachFromPlayer();
	

// Overridables.
public:

	// Purpose: for updates on the order. Return true if the order should be removed.
	virtual bool	Update( void );
	virtual bool	UpdateOnEvent( COrderEvent_Base *pEvent );



public:

	CBaseTFPlayer	*GetOwner( void );
	CBaseEntity		*GetTargetEntity( void );
	int				GetType( void );

	void	SetOwner( CBaseTFPlayer *pPlayer );
	void	SetType( int iOrderType );
	void	SetTarget( CBaseEntity *pTarget );
	void	SetDistance( float flDistance );
	void	SetLifetime( float flLifetime );

public:
	// Sent via datatable
	CNetworkVar( int, m_iOrderType );
	float			m_flDistanceToRemove;

	// When the order goes away.
	double			m_flDieTime;

	// Personal order owner
	CHandle< CBaseTFPlayer >	m_hOwningPlayer;
	EHANDLE						m_hTarget;
	CNetworkVar( int, m_iTargetEntIndex );
};



//-----------------------------------------------------------------------------
// ORDER CREATION DATA
//-----------------------------------------------------------------------------
// Time between personal order updates
#define PERSONAL_ORDER_UPDATE_TIME			2.0

// KILL orders
#define ORDER_KILL_ENEMY_DISTANCE			2048				// Distance the enemy must be within for this player to receive this order

// HEAL orders
#define ORDER_HEAL_FRIENDLY_DISTANCE		2048				// Distance the friendly must be within this player to receive this order

#endif // ORDERS_H
