// Added a modified version of Valve's floor turret
// Based on the one found in Lethal Stigma

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#define CWeaponDeployableTurret C_WeaponDeployableTurret
	#include "c_contingency_player.h"
#else
	#include "contingency_player.h"
	#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponDeployableTurret : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponDeployableTurret, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponDeployableTurret();

	void		Precache( void );

	void		ChangeAppearance( bool makeInvisible );

	bool		Deploy( void );
	void		ItemPostFrame( void );
	void		PrimaryAttack( void );

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{ return; }

	float		GetRange( void )		{	return	75.0f;	}
	float		GetFireRate( void )		{	return	1.0f;	}

	void		Drop( const Vector &vecVelocity );

	bool		ReloadOrSwitchWeapons( void );

	CWeaponDeployableTurret( const CWeaponDeployableTurret & );

private:
	CNetworkVar( bool, m_bSuccessfulPlacement );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDeployableTurret, DT_WeaponDeployableTurret )

BEGIN_NETWORK_TABLE( CWeaponDeployableTurret, DT_WeaponDeployableTurret )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO(m_bSuccessfulPlacement) ),
#else
	SendPropBool( SENDINFO(m_bSuccessfulPlacement) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponDeployableTurret )
	DEFINE_PRED_FIELD( m_bSuccessfulPlacement, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_deployableturret, CWeaponDeployableTurret );
PRECACHE_WEAPON_REGISTER( weapon_deployableturret );

acttable_t	CWeaponDeployableTurret::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },

	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SLAM,					false },
};

IMPLEMENT_ACTTABLE( CWeaponDeployableTurret );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponDeployableTurret::CWeaponDeployableTurret( void )
{
#ifndef CLIENT_DLL
	m_bSuccessfulPlacement = false;
#endif

	ChangeAppearance( false );

#ifndef CLIENT_DLL
	m_iHealth = CONTINGENCY_TURRET_MAX_HEALTH;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponDeployableTurret::GetDamageForActivity( Activity hitActivity )
{
	// Turrets don't do any damage, silly! (at least while they haven't been deployed yet)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponDeployableTurret::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	QAngle punchAng;

	punchAng.x = SharedRandomFloat( "crowbarpax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "crowbarpay", -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeployableTurret::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	// This weapon cannot and should not be dropped at this time
	UTIL_Remove( this );
#endif
}

void CWeaponDeployableTurret::Precache( void )
{
	BaseClass::Precache();
}

bool CWeaponDeployableTurret::Deploy( void )
{
	ChangeAppearance( false );

	return BaseClass::Deploy();
}

void CWeaponDeployableTurret::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( m_bSuccessfulPlacement && (gpGlobals->curtime >= (m_flNextPrimaryAttack - 0.25f)) )
	{
		pOwner->SwitchToNextBestWeapon( this );

#ifndef CLIENT_DLL
		Delete();
#endif

		return;	// we're done here
	}

	if ( pOwner->m_nButtons & IN_ATTACK2 )
		return;	// there's no secondary "attack" for the deployable turret "weapon" at this time

	BaseClass::ItemPostFrame();
}

// Changes the appearance (skin) of the turret
void CWeaponDeployableTurret::ChangeAppearance( bool makeInvisible )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );	// get the owner of this weapon

	CBaseViewModel *pViewModel = NULL;
	if ( pOwner )
		pViewModel = pOwner->GetViewModel();	// get the owner's view model (should be this weapon)
	
	CBaseAnimating *pBaseAnimating = dynamic_cast< CBaseAnimating* >( this );	// get the world model of this weapon

	if ( makeInvisible )	// make the turret invisible
	{
		// VIEW MODEL
		if ( pViewModel )
			pViewModel->m_nSkin = 3;

		// WORLD MODEL
		if ( pBaseAnimating )
			pBaseAnimating->m_nSkin = 3;
	}
	else
	{
		// VIEW MODEL
		if ( pViewModel )
			pViewModel->m_nSkin = 1;

		// WORLD MODEL
		if ( pBaseAnimating )
			pBaseAnimating->m_nSkin = 1;
	}
}

void CWeaponDeployableTurret::PrimaryAttack()
{
	// If placement was successful, then we're done here
	if ( m_bSuccessfulPlacement )
		return;

	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	if ( !pOwner )
		return;

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pOwner, pOwner->GetCurrentCommand() );
#endif

	if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
	{
		CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
		if ( !pOwner )
			return;

#ifndef CLIENT_DLL
		CNPC_FloorTurret *pDeployedTurret = dynamic_cast<CNPC_FloorTurret*>( Create("npc_turret_floor", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles(), pOwner) );
		if ( pDeployedTurret )
		{
			pDeployedTurret->SetHealth( GetHealth() );	// if applicable, restore deployed turret to its original health (a clever hack?)
			if ( pOwner->GetDeployedTurret() )	// not sure if this is possible at this point in time, but just in case...
				Warning( "Player (userid %i) has more than one deployed turret in the world! This is merely a report; no action has or will been taken with regards to this.\n", pOwner->GetUserID() );
			pOwner->SetDeployedTurret( pDeployedTurret );	// take note that our owner now has a deployable turret in the world
		}
#endif

		ChangeAppearance( true );

		// Call all appropriate animations.
		SendWeaponAnim( ACT_VM_HITCENTER );
		pOwner->SetAnimation( PLAYER_ATTACK1 );
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

		AddViewKick();

#ifndef CLIENT_DLL
		m_bSuccessfulPlacement = true;
#endif

		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	}

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pOwner );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CWeaponDeployableTurret::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_flNextSecondaryAttack < gpGlobals->curtime) )
	{
		// weapon isn't useable, switch.
		if ( (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false )
		{
			if ( pOwner )
				pOwner->SwitchToNextBestWeapon( this );

#ifndef CLIENT_DLL
			Delete();
#endif

			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 m_flNextPrimaryAttack < gpGlobals->curtime && 
			 m_flNextSecondaryAttack < gpGlobals->curtime )
		{
			// if we're successfully reloading, we're done
			if ( Reload() )
				return true;
		}
	}

	return false;
}
