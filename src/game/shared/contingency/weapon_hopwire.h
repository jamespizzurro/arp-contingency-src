// Ported directly from Lethal Stigma, with a few adjustments

// Lethal Stigma
// ADDED: Hopwire

#ifndef HL2MP_WEAPON_HOPWIRE_H
#define HL2MP_WEAPON_HOPWIRE_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "npcevent.h"

#ifndef CLIENT_DLL
	#include "episodic/grenade_hopwire.h"
#endif

#ifdef CLIENT_DLL
	#define CWeaponHopwire C_WeaponHopwire
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponHopwire: public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponHopwire, CBaseHL2MPCombatWeapon );
public:
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponHopwire();

	void	Precache( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	// Lethal Stigma
	// ADDED: Hopwire
	//void	HandleFireOnEmpty( void );

	// Lethal Stigma
	// ADDED: Hopwire
	//bool	HasAnyAmmo( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	
	bool	Reload( void );

private:
	void	ThrowGrenade( CBasePlayer *pPlayer );
	void	RollGrenade( CBasePlayer *pPlayer );
	void	LobGrenade( CBasePlayer *pPlayer );
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	void Drop( const Vector &vecVelocity );

	CNetworkVar( bool,	m_bRedraw );	//Draw the weapon again after throwing a grenade
	CNetworkVar( int,	m_AttackPaused );
	CNetworkVar( bool,	m_fDrawbackFinished );

#ifndef CLIENT_DLL
	CHandle<CGrenadeHopwire>	m_hActiveHopWire;
#endif

	CWeaponHopwire( const CWeaponHopwire & );

	DECLARE_ACTTABLE();
};


#endif // HL2MP_WEAPON_HOPWIRE_H
