// Contingency - James
// Added a modified version of Valve's floor turret
// Not much is commented here, so...

#ifndef NPC_TURRET_FLOOR_H
#define NPC_TURRET_FLOOR_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "player_pickup.h"
#include "particle_system.h"

static const int CONTINGENCY_TURRET_MAX_HEALTH = 50;

//Turret states
enum turretState_e
{
	TURRET_SEARCHING,
	TURRET_AUTO_SEARCHING,
	TURRET_ACTIVE,
	TURRET_SUPPRESSING,
	TURRET_DEPLOYING,
	TURRET_RETIRING,
	TURRET_SELF_DESTRUCTING,

	TURRET_STATE_TOTAL
};

//Eye states
enum eyeState_t
{
	TURRET_EYE_SEE_TARGET,			//Sees the target, bright and big
	TURRET_EYE_SEEKING_TARGET,		//Looking for a target, blinking (bright)
	TURRET_EYE_DORMANT,				//Not active
	TURRET_EYE_DEAD,				//Completely invisible
	TURRET_EYE_DISABLED,			//Turned off, must be reactivated before it'll deploy again (completely invisible)
	TURRET_EYE_ALARM,				// On side, but warning player to pick it back up
};

//Spawnflags
// BUG: These all stomp Base NPC spawnflags. Any Base NPC code called by this
//		this class may have undesired side effects due to these being set.
#define SF_FLOOR_TURRET_AUTOACTIVATE		0x00000020
#define SF_FLOOR_TURRET_STARTINACTIVE		0x00000040
#define SF_FLOOR_TURRET_FASTRETIRE			0x00000080
#define SF_FLOOR_TURRET_OUT_OF_AMMO			0x00000100
#define SF_FLOOR_TURRET_CITIZEN				0x00000200	// Citizen modified turret

class CBeam;
class CSprite;

//-----------------------------------------------------------------------------
// Purpose: Floor turret
//-----------------------------------------------------------------------------
class CNPC_FloorTurret : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_FloorTurret, CAI_BaseNPC );

	// Implement client-side turret NPC class
	DECLARE_SERVERCLASS();

public:

	CNPC_FloorTurret( void );
	~CNPC_FloorTurret( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual bool	CreateVPhysics( void );

	virtual	bool ShouldCollide( int collisionGroup, int contentsMask ) const;

	// Use functions
	int ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual void	UpdateOnRemove( void );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	PlayerPenetratingVPhysics( void );
	virtual int		VPhysicsTakeDamage( const CTakeDamageInfo &info );
	virtual bool	CanBecomeServerRagdoll( void ) { return false; }

#ifdef HL2_EPISODIC
	// We don't want to be NPCSOLID because we'll collide with NPC clips
	virtual unsigned int PhysicsSolidMaskForEntity( void ) const { return MASK_SOLID; } 
#endif	// HL2_EPISODIC

	// Player pickup
	virtual void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void	OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );
	virtual bool	OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

	const char *GetTracerType( void ) { return "AR2Tracer"; }

	bool	ShouldSavePhysics() { return true; }

	bool	HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	// Think functions
	virtual void	Retire( void );
	virtual void	Deploy( void );
	virtual void	ActiveThink( void );
	virtual void	SearchThink( void );
	virtual void	AutoSearchThink( void );
	virtual void	InactiveThink( void );
	virtual void	SuppressThink( void );
	virtual void	DisabledThink( void );
	virtual void	SelfDestructThink( void );
	virtual void	BreakThink( void );
	virtual void	HackFindEnemy( void );

	virtual float	GetAttackDamageScale( CBaseEntity *pVictim );
	virtual Vector	GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget );

	// Inputs
	void	InputToggle( inputdata_t &inputdata );
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );
	void	InputDepleteAmmo( inputdata_t &inputdata );
	void	InputRestoreAmmo( inputdata_t &inputdata );
	void	InputSelfDestruct( inputdata_t &inputdata );

	virtual bool	IsValidEnemy( CBaseEntity *pEnemy );
	bool			CanBeAnEnemyOf( CBaseEntity *pEnemy );

	int		BloodColor( void ) { return DONT_BLEED; }
	float	MaxYawSpeed( void );

	virtual Class_T	Classify( void );

	Vector EyePosition( void )
	{
		UpdateMuzzleMatrix();
		
		Vector vecOrigin;
		MatrixGetColumn( m_muzzleToWorld, 3, vecOrigin );
		
		Vector vecForward;
		MatrixGetColumn( m_muzzleToWorld, 0, vecForward );
		
		// Note: We back up into the model to avoid an edge case where the eyes clip out of the world and
		//		 cause problems with the PVS calculations -- jdw

		vecOrigin -= vecForward * 8.0f;

		return vecOrigin;
	}

	Vector	EyeOffset( Activity nActivity ) { return Vector( 0, 0, 58 ); }

	// Restore the turret to working operation after falling over
	void	ReturnToLife( void );

	int		DrawDebugTextOverlays( void );

	// INPCInteractive Functions
	virtual bool	CanInteractWith( CAI_BaseNPC *pUser ) { return false; } // Disabled for now (sjb)
	virtual	bool	HasBeenInteractedWith()	{ return m_bHackedByAlyx; }
	virtual void	NotifyInteraction( CAI_BaseNPC *pUser )
	{
		// For now, turn green so we can tell who is hacked.
		SetRenderColor( 0, 255, 0 );
		m_bHackedByAlyx = true; 
	}

	// Keep turret as upright as possible
	void StickToFloor( bool shouldStick );

	void Event_Killed( const CTakeDamageInfo &info );

	// Blow a player's turrets up when he or she dies
	// Make a turret explode instantly once its health has been depleated
	void Explode( void );

	// Keep turret as upright as possible
	// Allow the player to pick up otherwise stationary floor turrets
	bool IsExploding( void ) { return m_bSelfDestructing; }

protected:

	virtual bool	PreThink( turretState_e state );
	virtual void	Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	virtual void	SetEyeState( eyeState_t state );
	void			Ping( void );	
	void			Toggle( void );
	void			Enable( void );
	void			Disable( void );
	void			SpinUp( void );
	void			SpinDown( void );

	// IsCitizenTurret is no longer valid, always make it return true (at least for now)
	//bool	IsCitizenTurret( void ) { return HasSpawnFlags( SF_FLOOR_TURRET_CITIZEN ); }
	bool	IsCitizenTurret( void ) { return true; }

	bool	UpdateFacing( void );
	void	DryFire( void );
	void	UpdateMuzzleMatrix();

protected:
	matrix3x4_t m_muzzleToWorld;
	int		m_muzzleToWorldTick;
	int		m_iAmmoType;

	bool	m_bAutoStart;
	bool	m_bActive;		//Denotes the turret is deployed and looking for targets
	bool	m_bBlinkState;
	bool	m_bEnabled;		//Denotes whether the turret is able to deploy or not
	bool	m_bNoAlarmSounds;
	bool	m_bSelfDestructing;	// Going to blow up

	float	m_flDestructStartTime;
	float	m_flShotTime;
	float	m_flLastSight;
	float	m_flThrashTime;
	float	m_flPingTime;
	float	m_flNextActivateSoundTime;
	int		m_iKeySkin;

	QAngle	m_vecGoalAngles;

	int						m_iEyeAttachment;
	int						m_iMuzzleAttachment;
	eyeState_t				m_iEyeState;
	CHandle<CSprite>		m_hEyeGlow;
	CHandle<CBeam>			m_hLaser;

	CHandle<CParticleSystem>	m_hFizzleEffect;
	Vector	m_vecEnemyLKP;

	static const char		*m_pShotSounds[];

	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	bool	m_bHackedByAlyx;
	HSOUNDSCRIPTHANDLE			m_ShotSounds;

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

#endif //#ifndef NPC_TURRET_FLOOR_H
