// THE NYAN GUN

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#include "contingency_gamerules.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifndef CLIENT_DLL
	#include "explode.h"
#endif

#ifdef CLIENT_DLL
	#define CWeaponNyanGun C_WeaponNyanGun
#else
	#define NYAN_SOUND "Weapon_NyanGun.AnnoyingAsFuckSound"
	#define MAX_DAMAGE_DISTANCE 500.0f
#endif

class CWeaponNyanGun : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponNyanGun, CBaseHL2MPCombatWeapon );

public:
	CWeaponNyanGun( void );
	~CWeaponNyanGun( void );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

public:
	void Precache( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void Think( void );
	void Drop( const Vector &vecVelocity );

private:
	CWeaponNyanGun( const CWeaponNyanGun & );

	CNetworkVar( bool, m_bIsPlayingSound );
	CNetworkVar( bool, m_bFired );

	CContingency_Player *pPlayer;
#ifndef CLIENT_DLL
	trace_t tr;
	Vector vecAbsStart, vecAbsEnd, vecDir;
	CBaseEntity *pTarget;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNyanGun, DT_WeaponNyanGun )

BEGIN_NETWORK_TABLE( CWeaponNyanGun, DT_WeaponNyanGun )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO(m_bIsPlayingSound) ),
	RecvPropBool( RECVINFO(m_bFired) ),
#else
	SendPropBool( SENDINFO(m_bIsPlayingSound) ),
	SendPropBool( SENDINFO(m_bFired) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponNyanGun )
	DEFINE_PRED_FIELD( m_bIsPlayingSound, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bFired, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_nyangun, CWeaponNyanGun );
PRECACHE_WEAPON_REGISTER( weapon_nyangun );

// Using RPG model (at least for now)
acttable_t CWeaponNyanGun::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_RPG,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_RPG,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_RPG,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_RPG,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_RPG,					false },
};

IMPLEMENT_ACTTABLE( CWeaponNyanGun );

CWeaponNyanGun::CWeaponNyanGun( void )
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
	m_bIsPlayingSound = false;
	m_flNextPrimaryAttack = 0.0f;
	m_bFired = false;
}

CWeaponNyanGun::~CWeaponNyanGun( void )
{
#ifndef CLIENT_DLL
	StopSound( NYAN_SOUND );
#endif
}

void CWeaponNyanGun::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	PrecacheScriptSound( NYAN_SOUND );
#endif
}

void CWeaponNyanGun::ItemPostFrame( void )
{
	pPlayer = ToContingencyPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
		//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
		//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
		//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
		//			first shot.  Right now that's too much of an architecture change -- jdw

		// If the firing button was just pressed, or the alt-fire just released, reset the firing time
		//if ( pOwner->m_afButtonPressed & IN_ATTACK )
		//	 m_flNextPrimaryAttack = gpGlobals->curtime;

		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
			PrimaryAttack();
	}
	else
	{
		m_bFired = false;
	}
}

void CWeaponNyanGun::PrimaryAttack( void )
{
	// If we've already fired, don't fire again until
	// our attack button is released or we're dropped or something
	if ( m_bFired )
		return;

	pPlayer = ToContingencyPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( m_bIsPlayingSound )
	{
#ifndef CLIENT_DLL
		StopSound( NYAN_SOUND );
#endif
		m_bIsPlayingSound = false;
	}
	else
	{
#ifndef CLIENT_DLL
		StopSound( NYAN_SOUND );
		EmitSound( NYAN_SOUND );
#endif
		m_bIsPlayingSound = true;
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;

	SetNextThink( gpGlobals->curtime );

	m_bFired = true;
}

void CWeaponNyanGun::Think( void )
{
	BaseClass::Think();

#ifndef CLIENT_DLL
	if ( m_bIsPlayingSound )
	{
		pPlayer = ToContingencyPlayer( GetOwner() );
		if ( pPlayer && (pPlayer->GetActiveWeapon() == this) )
		{
			AngleVectors( pPlayer->EyeAngles(), &vecDir );
			vecAbsStart = pPlayer->EyePosition();
			vecAbsEnd = vecAbsStart + (vecDir * MAX_DAMAGE_DISTANCE);
			UTIL_TraceLine( vecAbsStart, vecAbsEnd, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr );

			pTarget = tr.m_pEnt;
			if ( pTarget )
			{
				if ( pTarget->IsPlayer() || pTarget->IsNPC() )
				{
					if ( pPlayer->GetDefaultRelationshipDisposition(pTarget->Classify()) != D_LI )
						ExplosionCreate( pTarget->GetAbsOrigin(),
						pTarget->GetAbsAngles(),
						pPlayer,
						99999,
						200,
						SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE,
						0.0f,
						this,
						-1,
						NULL,
						CLASS_PLAYER );
				}
			}
		}
		else
		{
			StopSound( NYAN_SOUND );
			m_bIsPlayingSound = false;
			m_bFired = false;
		}
	}
#endif
 
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CWeaponNyanGun::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	StopSound( NYAN_SOUND );
#endif
	m_bIsPlayingSound = false;
	m_bFired = false;

	BaseClass::Drop( vecVelocity );
}
