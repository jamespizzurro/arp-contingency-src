// THE HEALTH KIT

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "contingency_gamerules.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#define CWeaponHealthKit C_WeaponHealthKit
	#include "c_contingency_player.h"
#else
	#include "contingency_player.h"
	#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponHealthKit : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponHealthKit, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponHealthKit();

	void Precache( void );
	bool Deploy( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );

	void AddViewKick( void );
	float GetDamageForActivity( Activity hitActivity ) { return 0.0f; }

	float GetRange( void ) { return 75.0f; }
	float GetFireRate( void ) { return 1.0f; }

	void Drop( const Vector &vecVelocity );

	bool ReloadOrSwitchWeapons( void );

	bool Holster( CBaseCombatWeapon *pSwitchingTo );

	CWeaponHealthKit( const CWeaponHealthKit & );

private:
	CNetworkVar( bool, m_bSuccessfulHeal );

#ifndef CLIENT_DLL
	bool m_bShouldShowHint;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHealthKit, DT_WeaponHealthKit )

BEGIN_NETWORK_TABLE( CWeaponHealthKit, DT_WeaponHealthKit )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO(m_bSuccessfulHeal) ),
#else
	SendPropBool( SENDINFO(m_bSuccessfulHeal) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponHealthKit )
	DEFINE_PRED_FIELD( m_bSuccessfulHeal, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_healthkit, CWeaponHealthKit );

PRECACHE_WEAPON_REGISTER( weapon_healthkit );

acttable_t	CWeaponHealthKit::m_acttable[] = 
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

IMPLEMENT_ACTTABLE( CWeaponHealthKit );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponHealthKit::CWeaponHealthKit( void )
{
	m_bSuccessfulHeal = false;

#ifndef CLIENT_DLL
	m_bShouldShowHint = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponHealthKit::AddViewKick( void )
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
void CWeaponHealthKit::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	// This weapon cannot and should not be dropped at this time
	UTIL_Remove( this );
#endif
}

void CWeaponHealthKit::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "HealthVial.Touch" );
}

bool CWeaponHealthKit::Deploy( void )
{
	if ( BaseClass::Deploy() )
	{
#ifndef CLIENT_DLL
	m_bShouldShowHint = true;
#endif

		return true;
	}

	return false;
}

void CWeaponHealthKit::ItemPostFrame( void )
{
	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	if ( !pOwner )
		return;

#ifndef CLIENT_DLL
	if ( m_bShouldShowHint )
	{
		UTIL_HudHintText( pOwner, "#Contingency_Hint_HealthKit" );
		m_bShouldShowHint = false;
	}
#endif

	if ( m_bSuccessfulHeal && (gpGlobals->curtime >= (m_flNextPrimaryAttack - 0.25f)) )
	{
		m_bSuccessfulHeal = false;
		pOwner->SwitchToNextBestWeapon( this );
#ifndef CLIENT_DLL
		UTIL_Remove( this );
#endif

		return;	// we're done here
	}

	if ( (pOwner->m_nButtons & IN_ATTACK2) && (gpGlobals->curtime >= m_flNextSecondaryAttack) )
	{
		SecondaryAttack();
		return;
	}

	BaseClass::ItemPostFrame();
}

// Heals the player who is currently in possession of the health kit
void CWeaponHealthKit::PrimaryAttack()
{
	// If we've already healed using this health kit, it's useless
	if ( m_bSuccessfulHeal )
		return;

	if( gpGlobals->curtime >= m_flNextPrimaryAttack )
	{
		CContingency_Player *pPlayer = ToContingencyPlayer( GetOwner() );
		if ( !pPlayer )
			return;

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif

		if ( pPlayer->GetHealth() < pPlayer->GetMaxHealth() )
		{
#ifndef CLIENT_DLL
			pPlayer->EmitSound( "HealthVial.Touch" );
#endif

			pPlayer->SetHealth( pPlayer->GetMaxHealth() );

			// Call the appropriate player and weapon animations
			SendWeaponAnim( ACT_VM_HITCENTER );
			pPlayer->SetAnimation( PLAYER_ATTACK1 );
			ToContingencyPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

			AddViewKick();

			m_bSuccessfulHeal = true;	// heal assumed to be successful
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );
#endif
	}
}

// Heals other players the player in possession of the health kit is targeting
void CWeaponHealthKit::SecondaryAttack()
{
	// If we've already healed using this health kit, it's useless
	if ( m_bSuccessfulHeal )
		return;

	CContingency_Player *pPlayer = ToContingencyPlayer( GetOwner() );
	if ( !pPlayer )
		return;

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif

	if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
	{
		// Perform a traceline to see what our owner is currently targeting
		trace_t tr;
		Vector vecDir;
		Vector vecStart = pPlayer->Weapon_ShootPosition();
		pPlayer->EyeVectors( &vecDir, NULL, NULL );
		Vector vecStop = vecStart + vecDir * GetRange();
		UTIL_TraceLine( vecStart, vecStop, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr );

		CContingency_Player *pTargetPlayer = ToContingencyPlayer( tr.m_pEnt );
		if ( !pTargetPlayer )
			return;

		if ( ContingencyRules()->IsPlayerPlaying(pTargetPlayer) )
		{
			// Lots of checks, I know, but we have to be sure everything is right before we go ahead and heal someone.
			if ( pTargetPlayer->GetHealth() < pTargetPlayer->GetMaxHealth() )
			{
#ifndef CLIENT_DLL
				pTargetPlayer->EmitSound( "HealthVial.Touch" );
#endif

				pTargetPlayer->SetHealth( pTargetPlayer->GetMaxHealth() );

				// Call the appropriate player and weapon animations
				SendWeaponAnim( ACT_VM_MISSCENTER );
				pPlayer->SetAnimation( PLAYER_ATTACK1 );
				ToContingencyPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

				AddViewKick();

#ifndef CLIENT_DLL
				ClientPrint( pTargetPlayer, HUD_PRINTTALK, "%s1 healed you with a health kit.", pPlayer->GetPlayerName() );
				
				// Added credits system
				ClientPrint( pPlayer, HUD_PRINTTALK, "You have been granted 3 additional credits for healing a teammate." );
				pPlayer->AddCredits( 3 );
#endif

				m_bSuccessfulHeal = true;	// heal assumed to be successful
			}
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
	}

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CWeaponHealthKit::ReloadOrSwitchWeapons( void )
{
	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_flNextSecondaryAttack < gpGlobals->curtime) )
	{
		// weapon isn't useable, switch.
		if ( (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false )
		{
			m_bSuccessfulHeal = false;
			if ( pOwner )
				pOwner->SwitchToNextBestWeapon( this );
#ifndef CLIENT_DLL
			UTIL_Remove( this );
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

bool CWeaponHealthKit::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_bSuccessfulHeal )
		return false;

	return BaseClass::Holster( pSwitchingTo );
}
