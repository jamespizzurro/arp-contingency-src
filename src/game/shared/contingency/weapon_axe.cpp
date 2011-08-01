// A little something from Situation Outbreak :D

#include "cbase.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	AXE_RANGE 75.0f
#define	AXE_REFIRE 1.2f

#ifdef CLIENT_DLL
#define CWeaponAxe C_WeaponAxe
#endif

class CWeaponAxe : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponAxe, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponAxe();

	float GetRange( void ) { return AXE_RANGE; }
	float GetFireRate( void ) { return AXE_REFIRE; }

	void AddViewKick( void );
	float GetDamageForActivity( Activity hitActivity );
	void SecondaryAttack( void ) { return; }

	void Drop( const Vector &vecVelocity );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAxe, DT_WeaponAxe )

BEGIN_NETWORK_TABLE( CWeaponAxe, DT_WeaponAxe )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAxe )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_axe, CWeaponAxe );
PRECACHE_WEAPON_REGISTER( weapon_axe );

acttable_t	CWeaponAxe::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE( CWeaponAxe );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponAxe::CWeaponAxe( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponAxe::GetDamageForActivity( Activity hitActivity )
{
	return 60.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponAxe::AddViewKick( void )
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
void CWeaponAxe::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}
