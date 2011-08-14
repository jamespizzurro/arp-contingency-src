// Contingency - James
// Added a modified version of Valve's floor turret
// Not much is commented here, so...

#include "cbase.h"
#include "npc_contingency_turret.h"
#include "ai_senses.h"
#include "ai_memory.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "hl2/hl2_player.h"
#include "soundenvelope.h"
#include "physics_saverestore.h"
#include "IEffects.h"
#include "basehlcombatweapon_shared.h"
#include "phys_controller.h"
#include "ai_interactions.h"
#include "Sprite.h"
#include "beam_shared.h"
#include "props.h"
#include "particle_parse.h"

#ifdef PORTAL
	#include "prop_portal_shared.h"
	#include "portal_util_shared.h"
#endif

#include "explode.h"
#include "contingency_player.h"
#include "contingency_system_loadout.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *GetMassEquivalent(float flMass);

#define	DISABLE_SHOT	0

//Debug visualization
ConVar	g_debug_turret( "g_debug_turret", "0" );

extern ConVar physcannon_tracelength;

// Interactions
int	g_interactionTurretStillStanding	= 0;

#define	LASER_BEAM_SPRITE			"effects/laser1.vmt"

#define	FLOOR_TURRET_MODEL			"models/combine_turrets/floor_turret.mdl"
#define	FLOOR_TURRET_MODEL_CITIZEN	"models/combine_turrets/citizen_turret.mdl"
#define FLOOR_TURRET_GLOW_SPRITE	"sprites/glow1.vmt"
// #define FLOOR_TURRET_BC_YAW			"aim_yaw"
// #define FLOOR_TURRET_BC_PITCH		"aim_pitch"
#define	FLOOR_TURRET_RANGE			1200
#define	FLOOR_TURRET_MAX_WAIT		5
#define FLOOR_TURRET_SHORT_WAIT		2.0		// Used for FAST_RETIRE spawnflag
#define	FLOOR_TURRET_PING_TIME		1.0f	//LPB!!

#define	FLOOR_TURRET_VOICE_PITCH_LOW	45
#define	FLOOR_TURRET_VOICE_PITCH_HIGH	100

//Aiming variables
#define	FLOOR_TURRET_MAX_NOHARM_PERIOD		0.0f
#define	FLOOR_TURRET_MAX_GRACE_PERIOD		3.0f

//Activities
int ACT_FLOOR_TURRET_OPEN;
int ACT_FLOOR_TURRET_CLOSE;
int ACT_FLOOR_TURRET_OPEN_IDLE;
int ACT_FLOOR_TURRET_CLOSED_IDLE;
int ACT_FLOOR_TURRET_FIRE;

// Implement client-side turret NPC class
IMPLEMENT_SERVERCLASS_ST( CNPC_FloorTurret, DT_NPC_FloorTurret )
END_SEND_TABLE()

//Datatable
BEGIN_DATADESC( CNPC_FloorTurret )

	DEFINE_FIELD( m_iAmmoType,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bAutoStart,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bActive,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBlinkState,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEnabled,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNoAlarmSounds,	FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flShotTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flLastSight,	FIELD_TIME ),
	DEFINE_FIELD( m_flThrashTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flPingTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flNextActivateSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_flDestructStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_hFizzleEffect, FIELD_EHANDLE ),

	DEFINE_FIELD( m_vecGoalAngles,FIELD_VECTOR ),
	DEFINE_FIELD( m_iEyeAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iMuzzleAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iEyeState,		FIELD_INTEGER ),
	DEFINE_FIELD( m_hEyeGlow,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecEnemyLKP,	FIELD_VECTOR ),
	DEFINE_FIELD( m_hLaser,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_bSelfDestructing,	FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bHackedByAlyx, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD( m_iKeySkin, FIELD_INTEGER, "SkinNumber" ),
	
	DEFINE_THINKFUNC( Retire ),
	DEFINE_THINKFUNC( Deploy ),
	DEFINE_THINKFUNC( ActiveThink ),
	DEFINE_THINKFUNC( SearchThink ),
	DEFINE_THINKFUNC( AutoSearchThink ),
	DEFINE_THINKFUNC( InactiveThink ),
	DEFINE_THINKFUNC( SuppressThink ),
	DEFINE_THINKFUNC( DisabledThink ),
	DEFINE_THINKFUNC( SelfDestructThink ),
	DEFINE_THINKFUNC( BreakThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DepleteAmmo", InputDepleteAmmo ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RestoreAmmo", InputRestoreAmmo ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SelfDestruct", InputSelfDestruct ),

	// DEFINE_FIELD( m_ShotSounds, FIELD_SHORT ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_turret_floor, CNPC_FloorTurret );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CNPC_FloorTurret::CNPC_FloorTurret( void ) : 
	m_bActive( false ),
	m_hEyeGlow( NULL ),
	m_hLaser( NULL ),
	m_iAmmoType( -1 ),
	m_bAutoStart( false ),
	m_flPingTime( 0.0f ),
	m_flNextActivateSoundTime( 0.0f ),
	m_flShotTime( 0.0f ),
	m_flLastSight( 0.0f ),
	m_bBlinkState( false ),
	m_flThrashTime( 0.0f ),
	m_bEnabled( false ),
	m_bSelfDestructing( false )
{
	m_vecGoalAngles.Init();

	m_vecEnemyLKP = vec3_invalid;
}

CNPC_FloorTurret::~CNPC_FloorTurret( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Class_T	CNPC_FloorTurret::Classify( void ) 
{
	if ( m_bEnabled ) 
		return CLASS_PLAYER_ALLY;	// Contingency's turrets like players

	return CLASS_NONE;
}

void CNPC_FloorTurret::StickToFloor( bool shouldStick )
{
	// Make sure we've spawned before we do anything
	if ( VPhysicsGetObject() )
	{
		// Check to see what we should be doing exactly (sticking or unsticking)
		if ( shouldStick )
		{
			// Set up the vectors and traceline
			trace_t tr;
			Vector vecStart, vecStop, vecDir;

			// Get the angles downward
			AngleVectors( QAngle(0, 0, 0), NULL, NULL, &vecDir );
			vecDir *= -1;	// point downward instead of upward

			// Get the vectors
			vecStart = GetAbsOrigin();
			vecStop = vecStart + vecDir * MAX_TRACE_LENGTH;

			// Do the actual traceline
			UTIL_TraceLine( vecStart, vecStop, MASK_ALL, this, COLLISION_GROUP_DEBRIS, &tr );

			// Check to see if we hit anything
			if ( tr.fraction != 1.0 )
			{
				// We hit something, so teleport to this location and freeze us in place
				VPhysicsGetObject()->SetPosition( tr.endpos, QAngle(0, GetAbsAngles().y, 0), true );
				VPhysicsGetObject()->EnableMotion( false );
			}
		}
		else
		{
			// Disable sticking for one reason or another
			VPhysicsGetObject()->EnableMotion( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::UpdateOnRemove( void )
{
	if ( m_hLaser != NULL )
	{
		UTIL_Remove( m_hLaser );
		m_hLaser = NULL;
	}

	if ( m_hEyeGlow != NULL )
	{
		UTIL_Remove( m_hEyeGlow );
		m_hEyeGlow = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Precache( void )
{
	const char *pModelName = STRING( GetModelName() );
	pModelName = ( pModelName && pModelName[ 0 ] != '\0' ) ? pModelName : FLOOR_TURRET_MODEL;
	PrecacheModel( pModelName );
	PrecacheModel( FLOOR_TURRET_GLOW_SPRITE );

	PropBreakablePrecacheAll( MAKE_STRING( pModelName ) );

	if ( IsCitizenTurret() )
	{
		PrecacheModel( LASER_BEAM_SPRITE );
		PrecacheScriptSound( "NPC_FloorTurret.AlarmPing");
	}

	// Activities
	ADD_CUSTOM_ACTIVITY( CNPC_FloorTurret, ACT_FLOOR_TURRET_OPEN );
	ADD_CUSTOM_ACTIVITY( CNPC_FloorTurret, ACT_FLOOR_TURRET_CLOSE );
	ADD_CUSTOM_ACTIVITY( CNPC_FloorTurret, ACT_FLOOR_TURRET_CLOSED_IDLE );
	ADD_CUSTOM_ACTIVITY( CNPC_FloorTurret, ACT_FLOOR_TURRET_OPEN_IDLE );
	ADD_CUSTOM_ACTIVITY( CNPC_FloorTurret, ACT_FLOOR_TURRET_FIRE );
	
	PrecacheScriptSound( "NPC_FloorTurret.Retire" );
	PrecacheScriptSound( "NPC_FloorTurret.Deploy" );
	PrecacheScriptSound( "NPC_FloorTurret.Move" );
	PrecacheScriptSound( "NPC_Combine.WeaponBash" );
	PrecacheScriptSound( "NPC_FloorTurret.Activate" );
	PrecacheScriptSound( "NPC_FloorTurret.Alert" );
	m_ShotSounds = PrecacheScriptSound( "NPC_FloorTurret.ShotSounds" );
	PrecacheScriptSound( "NPC_FloorTurret.Die" );
	PrecacheScriptSound( "NPC_FloorTurret.Retract");
	PrecacheScriptSound( "NPC_FloorTurret.Alarm");
	PrecacheScriptSound( "NPC_FloorTurret.Ping");
	PrecacheScriptSound( "NPC_FloorTurret.DryFire");
	PrecacheScriptSound( "NPC_FloorTurret.Destruct" );

#ifdef HL2_EPISODIC
	PrecacheParticleSystem( "explosion_turret_break" );
#endif // HL2_EPISODIC
	
	BaseClass::Precache();
}

void CNPC_FloorTurret::CheckStuff( void )
{
	/*if ( !m_bSelfDestructing )	// commented to fix issue where explosion never occurs, so turret stays on the map forever
	{
		if ( (m_iHealth <= 0) ||
			 ((enginetrace->GetPointContents(GetAbsOrigin()) & (CONTENTS_WATER|CONTENTS_SLIME)) != 0) )
			Explode();
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Spawn( void )
{ 
	Precache();

	const char *pModelName = STRING( GetModelName() );
	SetModel( ( pModelName && pModelName[ 0 ] != '\0' ) ? pModelName : FLOOR_TURRET_MODEL );

	m_nSkin = 1;

	BaseClass::Spawn();

	SetBlocksLOS( false );

	m_HackedGunPos	= Vector( 0, 0, 12.75 );
	SetViewOffset( EyeOffset( ACT_IDLE ) );
	m_flFieldOfView	= 0.4f; // 60 degrees

	// Allow turret to take damage
	//m_takedamage	= DAMAGE_EVENTS_ONLY;
	//m_iHealth		= 100;
	//m_iMaxHealth	= 100;
	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = CONTINGENCY_TURRET_MAX_HEALTH;

	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( m_poseAim_Yaw, 0 );
	SetPoseParameter( m_poseAim_Pitch, 0 );

	// New ammo type for turrets
	//m_iAmmoType = GetAmmoDef()->Index( "PISTOL" );
	m_iAmmoType = GetAmmoDef()->Index( "TURRET" );

	m_iMuzzleAttachment = LookupAttachment( "eyes" );
	m_iEyeAttachment = LookupAttachment( "light" );

	// FIXME: Do we ever need m_bAutoStart? (Sawyer)
	m_spawnflags |= SF_FLOOR_TURRET_AUTOACTIVATE;

	//Set our autostart state
	m_bAutoStart = !!( m_spawnflags & SF_FLOOR_TURRET_AUTOACTIVATE );
	m_bEnabled	 = ( ( m_spawnflags & SF_FLOOR_TURRET_STARTINACTIVE ) == false );

	//Do we start active?
	if ( m_bAutoStart && m_bEnabled )
	{
		SetThink( &CNPC_FloorTurret::AutoSearchThink );
		SetEyeState( TURRET_EYE_DORMANT );
	}
	else
	{
		SetThink( &CNPC_FloorTurret::DisabledThink );
		SetEyeState( TURRET_EYE_DISABLED );
	}

	//Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	// Don't allow us to skip animation setup because our attachments are critical to us!
	SetBoneCacheFlags( BCF_NO_ANIMATION_SKIP );

	CreateVPhysics();

	SetState( NPC_STATE_IDLE );

	// Keep turret as upright as possible
	Activate();	// activate by default so that we can start firing right when we spawn
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Activate( void )
{
	BaseClass::Activate();

	// Force the eye state to the current state so that our glows are recreated after transitions
	SetEyeState( m_iEyeState );

	// Keep turret as upright as possible
	StickToFloor( true );
}

//-----------------------------------------------------------------------------

bool CNPC_FloorTurret::CreateVPhysics( void )
{
	//Spawn our physics hull
	if ( VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_ALLOW_OWNER_TRACING, false ) == NULL )
	{
		DevMsg( "npc_turret_floor unable to spawn physics object!\n" );
	}

	// This next bit appears to prevent a strange glitch in which npc_citizen NPCs
	// (perhaps others, but I don't know) can destroy a turret simply by
	// standing or walking over a part of it
	VPhysicsGetObject()->EnableCollisions( false );

	return true;
}

// Remember to update C_NPC_FloorTurret::ShouldCollide!
bool CNPC_FloorTurret::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( (collisionGroup == COLLISION_GROUP_PLAYER) ||
		 (collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT) )
		return false;	// prevent collisions with players (NOT their projectiles though!)

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

void CNPC_FloorTurret::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( pActivator );
	if ( !pPlayer )
		return;

	if ( GetOwnerEntity() != pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You cannot pick up turrets that do not belong to you." );
		return;	// only our owner can pick us up
	}

	const char *wpnClassname;
	for ( int i = 0; i < NUM_EQUIPMENT_TYPES; i++ )
	{
		wpnClassname = kEquipmentTypes[i][0];

		if ( !pPlayer->Weapon_OwnsThisType(wpnClassname) )
			continue;

		if ( Q_strcmp(wpnClassname, "weapon_deployableturret") == 0 )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "You already have a deployable turret! You can only carry one at a time." );
			return;	// our owner must not already have a deployable turret to pick us up
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "You already have a different type of equipment! You can only carry one type at a time." );
			return;	// our owner must not already have some other type of equipment to pick us up
		}
	}

	if ( IsExploding() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You cannot pick up turrets that have been destroyed." );
		return;	// our owner cannot pick us up if we're exploding (i.e. destroyed)
	}

	// If we've made it this far, our owner can pick us up!
	// We simulate this by giving our owner a deployable turret, then quickly removing us from the world

	pPlayer->IsGivingWeapons( true );
	pPlayer->GiveNamedItem( "weapon_deployableturret" );	// "pick us up"
	pPlayer->IsGivingWeapons( false );

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_deployableturret" );
	if ( pWeapon )
	{
		pWeapon->SetHealth( GetHealth() );	// store our current health in the weapon we've been given (a clever hack?)
		pPlayer->Weapon_Switch( pWeapon );
		Remove();	// remove us ASAP now that the player has "picked us up" (i.e. has been given a deployable turret)
	}
}

//-----------------------------------------------------------------------------
// Purpose: Retract and stop attacking
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Retire( void )
{
	if ( PreThink( TURRET_RETIRING ) )
		return;

	//Level out the turret
	m_vecGoalAngles = GetAbsAngles();
	SetNextThink( gpGlobals->curtime + 0.05f );

	//Set ourselves to close
	if ( GetActivity() != ACT_FLOOR_TURRET_CLOSE )
	{
		//Set our visible state to dormant
		SetEyeState( TURRET_EYE_DORMANT );

		SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN_IDLE );
		
		//If we're done moving to our desired facing, close up
		if ( UpdateFacing() == false )
		{
			SetActivity( (Activity) ACT_FLOOR_TURRET_CLOSE );
			EmitSound( "NPC_FloorTurret.Retire" );

			//Notify of the retraction
			m_OnRetire.FireOutput( NULL, this );
		}
	}
	else if ( IsActivityFinished() )
	{	
		m_bActive		= false;
		m_flLastSight	= 0;

		SetActivity( (Activity) ACT_FLOOR_TURRET_CLOSED_IDLE );

		//Go back to auto searching
		if ( m_bAutoStart )
		{
			SetThink( &CNPC_FloorTurret::AutoSearchThink );
			SetNextThink( gpGlobals->curtime + 0.05f );
		}
		else
		{
			//Set our visible state to dormant
			SetEyeState( TURRET_EYE_DISABLED );
			SetThink( &CNPC_FloorTurret::DisabledThink );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Deploy and start attacking
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Deploy( void )
{
	if ( PreThink( TURRET_DEPLOYING ) )
		return;

	m_vecGoalAngles = GetAbsAngles();

	SetNextThink( gpGlobals->curtime + 0.05f );

	//Show we've seen a target
	SetEyeState( TURRET_EYE_SEE_TARGET );

	//Open if we're not already
	if ( GetActivity() != ACT_FLOOR_TURRET_OPEN )
	{
		m_bActive = true;
		SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN );
		EmitSound( "NPC_FloorTurret.Deploy" );

		//Notify we're deploying
		m_OnDeploy.FireOutput( NULL, this );
	}

	//If we're done, then start searching
	if ( IsActivityFinished() )
	{
		SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN_IDLE );

		m_flShotTime  = gpGlobals->curtime + 1.0f;

		m_flPlaybackRate = 0;
		SetThink( &CNPC_FloorTurret::SearchThink );

		EmitSound( "NPC_FloorTurret.Move" );
	}

	m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_MAX_WAIT;	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	// We cannot be picked up
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason )
{
	// We cannot be picked up, therefore we cannot be dropped...right?
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	// We cannot be picked up
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter *sourceEnt)
{
	return false;	// we shouldn't be interacting with any other NPCs
}

//-----------------------------------------------------------------------------
// Purpose: Returns the speed at which the turret can face a target
//-----------------------------------------------------------------------------
float CNPC_FloorTurret::MaxYawSpeed( void )
{
	//TODO: Scale by difficulty?
	return 360.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Causes the turret to face its desired angles
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::UpdateFacing( void )
{
	bool  bMoved = false;
	UpdateMuzzleMatrix();

	Vector vecGoalDir;
	AngleVectors( m_vecGoalAngles, &vecGoalDir );

	Vector vecGoalLocalDir;
	VectorIRotate( vecGoalDir, m_muzzleToWorld, vecGoalLocalDir );

	if ( g_debug_turret.GetBool() )
	{
		Vector	vecMuzzle, vecMuzzleDir;

		MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzle );
		MatrixGetColumn( m_muzzleToWorld, 0, vecMuzzleDir );

		NDebugOverlay::Cross3D( vecMuzzle, -Vector(2,2,2), Vector(2,2,2), 255, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( vecMuzzle+(vecMuzzleDir*256), -Vector(2,2,2), Vector(2,2,2), 255, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMuzzle, vecMuzzle+(vecMuzzleDir*256), 255, 255, 0, false, 0.05 );
		
		NDebugOverlay::Cross3D( vecMuzzle, -Vector(2,2,2), Vector(2,2,2), 255, 0, 0, false, 0.05 );
		NDebugOverlay::Cross3D( vecMuzzle+(vecGoalDir*256), -Vector(2,2,2), Vector(2,2,2), 255, 0, 0, false, 0.05 );
		NDebugOverlay::Line( vecMuzzle, vecMuzzle+(vecGoalDir*256), 255, 0, 0, false, 0.05 );
	}

	QAngle vecGoalLocalAngles;
	VectorAngles( vecGoalLocalDir, vecGoalLocalAngles );

	// Update pitch
	float flDiff = AngleNormalize( UTIL_ApproachAngle(  vecGoalLocalAngles.x, 0.0, 0.05f * MaxYawSpeed() ) );
	
	SetPoseParameter( m_poseAim_Pitch, GetPoseParameter( m_poseAim_Pitch ) + ( flDiff / 1.5f ) );

	if ( fabs( flDiff ) > 0.1f )
	{
		bMoved = true;
	}

	// Update yaw
	flDiff = AngleNormalize( UTIL_ApproachAngle(  vecGoalLocalAngles.y, 0.0, 0.05f * MaxYawSpeed() ) );

	SetPoseParameter( m_poseAim_Yaw, GetPoseParameter( m_poseAim_Yaw ) + ( flDiff / 1.5f ) );

	if ( fabs( flDiff ) > 0.1f )
	{
		bMoved = true;
	}

	// You're going to make decisions based on this info.  So bump the bone cache after you calculate everything
	InvalidateBoneCache();

	return bMoved;
}

void CNPC_FloorTurret::DryFire( void )
{
	EmitSound( "NPC_FloorTurret.DryFire");
	EmitSound( "NPC_FloorTurret.Activate" );

 	if ( RandomFloat( 0, 1 ) > 0.5 )
	{
		m_flShotTime = gpGlobals->curtime + random->RandomFloat( 1, 2.5 );
	}
	else
	{
		m_flShotTime = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turret will continue to fire on a target's position when it loses sight of it
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SuppressThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( TURRET_SUPPRESSING ) )
		return;

	//Update our think time
	SetNextThink( gpGlobals->curtime + 0.1f );

	// Look for a new enemy
	HackFindEnemy();

	//If we've acquired an enemy, start firing at it
	if ( !GetEnemy() )
	{
		SetThink( &CNPC_FloorTurret::ActiveThink );
		return;
	}

	//See if we're done suppressing
	if ( gpGlobals->curtime > m_flLastSight )
	{
		// Should we look for a new target?
		ClearEnemyMemory();
		SetEnemy( NULL );
		SetThink( &CNPC_FloorTurret::SearchThink );
		m_vecGoalAngles = GetAbsAngles();
		
		SpinDown();

		if ( m_spawnflags & SF_FLOOR_TURRET_FASTRETIRE )
		{
			// Retire quickly in this case. (The case where we saw the player, but he hid again).
			m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_SHORT_WAIT;
		}
		else
		{
			m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_MAX_WAIT;
		}

		return;
	}

	//Get our shot positions
	Vector vecMid = EyePosition();
	Vector vecMidEnemy = m_vecEnemyLKP;

	//Calculate dir and dist to enemy
	Vector	vecDirToEnemy = vecMidEnemy - vecMid;	

	//We want to look at the enemy's eyes so we don't jitter
	Vector	vecDirToEnemyEyes = vecMidEnemy - vecMid;
	VectorNormalize( vecDirToEnemyEyes );

	QAngle vecAnglesToEnemy;
	VectorAngles( vecDirToEnemyEyes, vecAnglesToEnemy );

	//Draw debug info
	if ( g_debug_turret.GetBool() )
	{
		NDebugOverlay::Cross3D( vecMid, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( vecMidEnemy, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMid, vecMidEnemy, 0, 255, 0, false, 0.05f );
	}

	if ( m_flShotTime < gpGlobals->curtime && m_vecEnemyLKP != vec3_invalid )
	{
		Vector vecMuzzle, vecMuzzleDir;
		UpdateMuzzleMatrix();
		MatrixGetColumn( m_muzzleToWorld, 0, vecMuzzleDir );
		MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzle );

		//Fire the gun
		if ( DotProduct( vecDirToEnemy, vecMuzzleDir ) >= 0.9848 ) // 10 degree slop
		{
			if( m_spawnflags & SF_FLOOR_TURRET_OUT_OF_AMMO )
			{
				DryFire();
			}
			else
			{
				ResetActivity();
				SetActivity( (Activity) ACT_FLOOR_TURRET_FIRE );

				//Fire the weapon
#if !DISABLE_SHOT
				Shoot( vecMuzzle, vecMuzzleDir );
#endif
			}
		} 
	}
	else
	{
		SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN_IDLE );
	}

	//If we can see our enemy, face it
	m_vecGoalAngles.y = vecAnglesToEnemy.y;
	m_vecGoalAngles.x = vecAnglesToEnemy.x;

	//Turn to face
	UpdateFacing();
}

//-----------------------------------------------------------------------------
// Purpose: Allows the turret to fire on targets if they're visible
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::ActiveThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( TURRET_ACTIVE ) )
		return;

	HackFindEnemy();
	
	//Update our think time
	SetNextThink( gpGlobals->curtime + 0.1f );

	//If we've become inactive, go back to searching
	if ( ( m_bActive == false ) || ( GetEnemy() == NULL ) )
	{
		SetEnemy( NULL );
		m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_MAX_WAIT;
		SetThink( &CNPC_FloorTurret::SearchThink );
		m_vecGoalAngles = GetAbsAngles();
		return;
	}
	
	//Get our shot positions
	Vector vecMid = EyePosition();
	Vector vecMidEnemy = GetEnemy()->BodyTarget( vecMid );

	// Store off our last seen location so we can suppress it later
	m_vecEnemyLKP = vecMidEnemy;

	//Look for our current enemy
	bool bEnemyInFOV = FInViewCone( GetEnemy() );
	bool bEnemyVisible = FVisible( GetEnemy() ) && GetEnemy()->IsAlive();	

	// Robin: This is a hack to get around the fact that the muzzle for the turret
	// is outside it's vcollide. This means that if it leans against a thin wall, 
	// the muzzle can be on the other side of the wall, where it's then able to see
	// and shoot at targets. This check ensures that nothing has come between the
	// center of the turret and the muzzle.
	if ( bEnemyVisible )
	{
		trace_t tr;
		Vector vecCenter;
		CollisionProp()->CollisionToWorldSpace( Vector(0,0,52), &vecCenter );
		UTIL_TraceLine( vecCenter, vecMid, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			bEnemyVisible = false;
		}
	}

	//Calculate dir and dist to enemy
	Vector	vecDirToEnemy = vecMidEnemy - vecMid;	
	float	flDistToEnemy = VectorNormalize( vecDirToEnemy );

	//Draw debug info
	if ( g_debug_turret.GetBool() )
	{
		NDebugOverlay::Cross3D( vecMid, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( GetEnemy()->WorldSpaceCenter(), -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMid, GetEnemy()->WorldSpaceCenter(), 0, 255, 0, false, 0.05 );

		NDebugOverlay::Cross3D( vecMid, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( vecMidEnemy, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMid, vecMidEnemy, 0, 255, 0, false, 0.05f );
	}

	//See if they're past our FOV of attack
	if ( bEnemyInFOV == false )
	{
		// Should we look for a new target?
		ClearEnemyMemory();
		SetEnemy( NULL );
		
		if ( m_spawnflags & SF_FLOOR_TURRET_FASTRETIRE )
		{
			// Retire quickly in this case. (The case where we saw the player, but he hid again).
			m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_SHORT_WAIT;
		}
		else
		{
			m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_MAX_WAIT;
		}

		SetThink( &CNPC_FloorTurret::SearchThink );
		m_vecGoalAngles = GetAbsAngles();
		
		SpinDown();

		return;
	}

	//Current enemy is not visible
	if ( ( bEnemyVisible == false ) || ( flDistToEnemy > FLOOR_TURRET_RANGE ))
	{
		m_flLastSight = gpGlobals->curtime + 2.0f;

		ClearEnemyMemory();
		SetEnemy( NULL );
		SetThink( &CNPC_FloorTurret::SuppressThink );

		return;
	}

	if ( g_debug_turret.GetBool() )
	{
		Vector vecMuzzle, vecMuzzleDir;

		UpdateMuzzleMatrix();
		MatrixGetColumn( m_muzzleToWorld, 0, vecMuzzleDir );
		MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzle );

		// Visualize vertical firing ranges
		for ( int i = 0; i < 4; i++ )
		{
			QAngle angMaxDownPitch = GetAbsAngles();

			switch( i )
			{
			case 0:	angMaxDownPitch.x -= 15; break;
			case 1:	angMaxDownPitch.x += 15; break;
			case 2:	angMaxDownPitch.x -= 25; break;
			case 3:	angMaxDownPitch.x += 25; break;
			default:
				break;
			}

			Vector vecMaxDownPitch;
			AngleVectors( angMaxDownPitch, &vecMaxDownPitch );
			NDebugOverlay::Line( vecMuzzle, vecMuzzle + (vecMaxDownPitch*256), 255, 255, 255, false, 0.1 );
		}
	}

	if ( m_flShotTime < gpGlobals->curtime )
	{
		Vector vecMuzzle, vecMuzzleDir;

		UpdateMuzzleMatrix();
		MatrixGetColumn( m_muzzleToWorld, 0, vecMuzzleDir );
		MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzle );

		Vector2D vecDirToEnemy2D = vecDirToEnemy.AsVector2D();
		Vector2D vecMuzzleDir2D = vecMuzzleDir.AsVector2D();

		bool bCanShoot = true;
		float minCos3d = DOT_10DEGREE; // 10 degrees slop

		if ( flDistToEnemy < 60.0 )
		{
			vecDirToEnemy2D.NormalizeInPlace();
			vecMuzzleDir2D.NormalizeInPlace();

			bCanShoot = ( vecDirToEnemy2D.Dot(vecMuzzleDir2D) >= DOT_10DEGREE );
			minCos3d = 0.7071; // 45 degrees
		}

		//Fire the gun
		if ( bCanShoot ) // 10 degree slop XY
		{
			float dot3d = DotProduct( vecDirToEnemy, vecMuzzleDir );

			if( m_spawnflags & SF_FLOOR_TURRET_OUT_OF_AMMO )
			{
				DryFire();
			}
			else
			{
				if ( dot3d >= minCos3d ) 
				{
					ResetActivity();
					SetActivity( (Activity) ACT_FLOOR_TURRET_FIRE );

					//Fire the weapon
#if !DISABLE_SHOT
					Shoot( vecMuzzle, vecMuzzleDir, (dot3d < DOT_10DEGREE) );
#endif
				}
			}
		} 
	}
	else
	{
		SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN_IDLE );
	}

	//If we can see our enemy, face it
	if ( bEnemyVisible )
	{
		//We want to look at the enemy's eyes so we don't jitter
		Vector	vecDirToEnemyEyes = GetEnemy()->WorldSpaceCenter() - vecMid;
		VectorNormalize( vecDirToEnemyEyes );

		QAngle vecAnglesToEnemy;
		VectorAngles( vecDirToEnemyEyes, vecAnglesToEnemy );

		m_vecGoalAngles.y = vecAnglesToEnemy.y;
		m_vecGoalAngles.x = vecAnglesToEnemy.x;
	}

	//Turn to face
	UpdateFacing();
}

//-----------------------------------------------------------------------------
// Purpose: Target doesn't exist or has eluded us, so search for one
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SearchThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( TURRET_SEARCHING ) )
		return;

	SetNextThink( gpGlobals->curtime + 0.05f );

	SetActivity( (Activity) ACT_FLOOR_TURRET_OPEN_IDLE );

	//If our enemy has died, pick a new enemy
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->IsAlive() == false ) )
	{
		SetEnemy( NULL );
	}

	//Acquire the target
	if ( GetEnemy() == NULL )
	{
		HackFindEnemy();
	}

	//If we've found a target, spin up the barrel and start to attack
	if ( GetEnemy() != NULL )
	{
		//Give players a grace period
		if ( GetEnemy()->IsPlayer() )
		{
			m_flShotTime  = gpGlobals->curtime + 0.5f;
		}
		else
		{
			m_flShotTime  = gpGlobals->curtime + 0.1f;
		}

		m_flLastSight = 0;
		SetThink( &CNPC_FloorTurret::ActiveThink );
		SetEyeState( TURRET_EYE_SEE_TARGET );

		SpinUp();
 
		if ( gpGlobals->curtime > m_flNextActivateSoundTime )
		{
			EmitSound( "NPC_FloorTurret.Activate" );
			m_flNextActivateSoundTime = gpGlobals->curtime + 3.0;
		}
		return;
	}

	//Are we out of time and need to retract?
 	if ( gpGlobals->curtime > m_flLastSight )
	{
		//Before we retrace, make sure that we are spun down.
		m_flLastSight = 0;
		SetThink( &CNPC_FloorTurret::Retire );
		return;
	}
	
	//Display that we're scanning
	m_vecGoalAngles.x = GetAbsAngles().x + ( sin( gpGlobals->curtime * 1.0f ) * 15.0f );
	m_vecGoalAngles.y = GetAbsAngles().y + ( sin( gpGlobals->curtime * 2.0f ) * 60.0f );

	//Turn and ping
	UpdateFacing();
	Ping();
}

//-----------------------------------------------------------------------------
// Purpose: Watch for a target to wander into our view
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::AutoSearchThink( void )
{
	//Allow descended classes a chance to do something before the think function
	if ( PreThink( TURRET_AUTO_SEARCHING ) )
		return; 

	//Spread out our thinking
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.2f, 0.4f ) );

	//If the enemy is dead, find a new one
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->IsAlive() == false ) )
	{
		SetEnemy( NULL );
	}

	//Acquire Target
	if ( GetEnemy() == NULL )
	{
		HackFindEnemy();
	}

	//Deploy if we've got an active target
	if ( GetEnemy() != NULL )
	{
		SetThink( &CNPC_FloorTurret::Deploy );

		// Stop us from playing annoying looping sounds
		/*if ( !m_bNoAlarmSounds )
		{
			EmitSound( "NPC_FloorTurret.Alert" );
		}*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire!
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict )
{
	FireBulletsInfo_t info;

	if ( !bStrict && GetEnemy() != NULL )
	{
		Vector vecDir = GetActualShootTrajectory( vecSrc );

		info.m_vecSrc = vecSrc;
		info.m_vecDirShooting = vecDir;

		// Screwed up turret tracers (removed them for now)
		//info.m_iTracerFreq = 1;
		info.m_iTracerFreq = 0;

		info.m_iShots = 1;
		info.m_pAttacker = this;
		info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
		info.m_flDistance = MAX_COORD_RANGE;
		info.m_iAmmoType = m_iAmmoType;
	}
	else
	{
		info.m_vecSrc = vecSrc;
		info.m_vecDirShooting = vecDirToEnemy;

		// Screwed up turret tracers (removed them for now)
		//info.m_iTracerFreq = 1;
		info.m_iTracerFreq = 0;

		info.m_iShots = 1;
		info.m_pAttacker = this;
		info.m_vecSpread = GetAttackSpread( NULL, GetEnemy() );
		info.m_flDistance = MAX_COORD_RANGE;
		info.m_iAmmoType = m_iAmmoType;
	}

	FireBullets( info );
	EmitSound( "NPC_FloorTurret.ShotSounds", m_ShotSounds );
	DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::IsValidEnemy( CBaseEntity *pEnemy )
{
	if ( m_NPCState == NPC_STATE_DEAD )
		return false;

	// Don't shoot at other turrets.
	if ( pEnemy->m_iClassname == m_iClassname )
		return false;

	// If our eye is stuck in something, don't shoot
	if ( UTIL_PointContents(EyePosition()) & MASK_SHOT )
		return false;

	// Turrets have limited vertical aim capability
	//	- Can only aim +-15 degrees, + the 10 degree slop they're allowed.
	Vector vEnemyPos = pEnemy->EyePosition();

#ifdef PORTAL
	if ( !FInViewCone( pEnemy ) || !FVisible( pEnemy ) )
	{
		CProp_Portal *pPortal = FInViewConeThroughPortal( pEnemy );

		if ( pPortal )
		{
			// Translate our target across the portal
			UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vEnemyPos, vEnemyPos );
		}
	}
#endif

	Vector los = ( vEnemyPos - EyePosition() );

	QAngle angleToTarget;
	VectorAngles( los, angleToTarget );
	float flZDiff = fabs( AngleNormalize( angleToTarget.x - GetAbsAngles().x) );
	if ( flZDiff > 28.0f && los.LengthSqr() > 4096.0f )
		return false;

	return BaseClass::IsValidEnemy( pEnemy );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
	// If we're out of ammo, make friendly companions ignore us
	if ( m_spawnflags & SF_FLOOR_TURRET_OUT_OF_AMMO )
	{
		if ( pEnemy->Classify() == CLASS_PLAYER_ALLY_VITAL )
			return false;
	}

	return BaseClass::CanBeAnEnemyOf( pEnemy );
}

//-----------------------------------------------------------------------------
// Purpose: This turret is dead. See if it ever becomes upright again, and if 
//			so, become active again.
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InactiveThink( void )
{
	// Allow turret to take damage
	CheckStuff();	// check to see if we're dead yet

	// Update our PVS state
	CheckPVSCondition();

	// Wake up if we're not on our side
	if ( m_bEnabled )
	{
		ReturnToLife();
		return;
	}

	if ( IsCitizenTurret() )
	{
		// Blink if we have ammo or our current blink is "on" and we need to turn it off again
		if ( HasSpawnFlags( SF_FLOOR_TURRET_OUT_OF_AMMO ) == false || m_bBlinkState )
		{
			// If we're on our side, ping and complain to the player
			if ( m_bBlinkState == false )
			{
				// Ping when the light is going to come back on
				EmitSound( "NPC_FloorTurret.AlarmPing" );
			}

			SetEyeState( TURRET_EYE_ALARM );
			SetNextThink( gpGlobals->curtime + 0.25f );
		}
	}
	else
	{
		SetNextThink( gpGlobals->curtime + 1.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::ReturnToLife( void )
{
	m_flThrashTime = 0;

	// Return to life
	SetState( NPC_STATE_IDLE );
	m_lifeState = LIFE_ALIVE;
	SetCollisionGroup( COLLISION_GROUP_NONE );

	// Disable turrets that are being moved around by players
	// NOTE: Initial call to Enable() function appears to be necessary before calling Disable() function

	// Become active again
	Enable();
	Disable();
}	

//-----------------------------------------------------------------------------
// Purpose: The turret is not doing anything at all
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::DisabledThink( void )
{
	// Allow turret to take damage
	CheckStuff();	// check to see if we're dead yet

	SetNextThink( gpGlobals->curtime + 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: The turret doesn't run base AI properly, which is a bad decision.
//			As a result, it has to manually find enemies.
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::HackFindEnemy( void )
{
	// Allow turret to take damage
	CheckStuff();	// check to see if we're dead yet

	// We have to refresh our memories before finding enemies, so
	// dead enemies are cleared out before new ones are added.
	GetEnemies()->RefreshMemories();

	GetSenses()->Look( FLOOR_TURRET_RANGE );
	SetEnemy( BestEnemy() );
}

//-----------------------------------------------------------------------------
// Purpose: Allows a generic think function before the others are called
// Input  : state - which state the turret is currently in
//-----------------------------------------------------------------------------
bool CNPC_FloorTurret::PreThink( turretState_e state )
{
	// Allow turret to take damage
	CheckStuff();	// check to see if we're dead yet

	// Hack to disable turrets when ai is disabled
	if ( CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI )
	{
		// Push our think out into the future
		SetNextThink( gpGlobals->curtime + 0.1f );
		return true;
	}
 
	CheckPVSCondition();

	//Animate
	StudioFrameAdvance();

	// We're gonna blow up, so don't interrupt us
	if ( state == TURRET_SELF_DESTRUCTING )
		return false;

	//Do not interrupt current think function
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the state of the glowing eye attached to the turret
// Input  : state - state the eye should be in
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SetEyeState( eyeState_t state )
{
	// Must have a valid eye to affect
	if ( !m_hEyeGlow )
	{
		// Create our eye sprite
		m_hEyeGlow = CSprite::SpriteCreate( FLOOR_TURRET_GLOW_SPRITE, GetLocalOrigin(), false );
		if ( !m_hEyeGlow )
			return;

		m_hEyeGlow->SetTransparency( kRenderWorldGlow, 255, 0, 0, 128, kRenderFxNoDissipation );
		m_hEyeGlow->SetAttachment( this, m_iEyeAttachment );
	}

	// Add the laser if it doesn't already exist
	if ( IsCitizenTurret() && HasSpawnFlags( SF_FLOOR_TURRET_OUT_OF_AMMO ) == false && m_hLaser == NULL )
	{
		m_hLaser = CBeam::BeamCreate( LASER_BEAM_SPRITE, 1.0f );
		if ( m_hLaser == NULL )
			return;

		m_hLaser->EntsInit( this, this );
		m_hLaser->FollowEntity( this );
		m_hLaser->SetStartAttachment( LookupAttachment( "laser_start" ) );
		m_hLaser->SetEndAttachment( LookupAttachment( "laser_end" ) );
		m_hLaser->SetNoise( 0 );
		m_hLaser->SetColor( 255, 0, 0 );
		m_hLaser->SetScrollRate( 0 );
		m_hLaser->SetWidth( 1.0f );
		m_hLaser->SetEndWidth( 1.0f );
		m_hLaser->SetBrightness( 160 );
		m_hLaser->SetBeamFlags( SF_BEAM_SHADEIN );
	}

	m_iEyeState = state;

	//Set the state
	switch( state )
	{
	default:
	case TURRET_EYE_SEE_TARGET: //Fade in and scale up
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetBrightness( 164, 0.1f );
		m_hEyeGlow->SetScale( 0.4f, 0.1f );
		break;

	case TURRET_EYE_SEEKING_TARGET: //Ping-pongs
		
		//Toggle our state
		m_bBlinkState = !m_bBlinkState;
		m_hEyeGlow->SetColor( 255, 128, 0 );

		if ( m_bBlinkState )
		{
			//Fade up and scale up
			m_hEyeGlow->SetScale( 0.25f, 0.1f );
			m_hEyeGlow->SetBrightness( 164, 0.1f );
		}
		else
		{
			//Fade down and scale down
			m_hEyeGlow->SetScale( 0.2f, 0.1f );
			m_hEyeGlow->SetBrightness( 64, 0.1f );
		}

		break;

	case TURRET_EYE_DORMANT: //Fade out and scale down
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 0.5f );
		m_hEyeGlow->SetBrightness( 64, 0.5f );
		break;

	case TURRET_EYE_DEAD: //Fade out slowly
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetScale( 0.1f, 3.0f );
		m_hEyeGlow->SetBrightness( 0, 3.0f );
		break;

	case TURRET_EYE_DISABLED:
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 1.0f );
		m_hEyeGlow->SetBrightness( 0, 1.0f );
		break;
	
	case TURRET_EYE_ALARM:
		{
			//Toggle our state
			m_bBlinkState = !m_bBlinkState;
			m_hEyeGlow->SetColor( 255, 0, 0 );

			if ( m_bBlinkState )
			{
				//Fade up and scale up
				m_hEyeGlow->SetScale( 0.75f, 0.05f );
				m_hEyeGlow->SetBrightness( 192, 0.05f );
			}
			else
			{
				//Fade down and scale down
				m_hEyeGlow->SetScale( 0.25f, 0.25f );
				m_hEyeGlow->SetBrightness( 64, 0.25f );
			}
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Make a pinging noise so the player knows where we are
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Ping( void )
{
	//See if it's time to ping again
	if ( m_flPingTime > gpGlobals->curtime )
		return;

	//Ping!
	EmitSound( "NPC_FloorTurret.Ping" );

	SetEyeState( TURRET_EYE_SEEKING_TARGET );

	m_flPingTime = gpGlobals->curtime + FLOOR_TURRET_PING_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the turret's state
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Toggle( void )
{
	//This turret is on its side, it can't function
	if ( !IsAlive() )
		return;

	//Toggle the state
	if ( m_bEnabled )
	{
		Disable();
	}
	else 
	{
		Enable();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Enable the turret and deploy
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Enable( void )
{
	// Don't interrupt blowing up!
	if ( m_bSelfDestructing )
		return;

	// Always allow us to come back to life, even if not right now
	m_bEnabled = true;

	//This turret is on its side, it can't function
	if ( !IsAlive() )
		return;

	// if the turret is flagged as an autoactivate turret, re-enable its ability open self.
	if ( m_spawnflags & SF_FLOOR_TURRET_AUTOACTIVATE )
	{
		m_bAutoStart = true;
	}

	SetThink( &CNPC_FloorTurret::Deploy );
	SetNextThink( gpGlobals->curtime + 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: Retire the turret until enabled again
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::Disable( void )
{
	//This turret is on its side, it can't function
	if ( !IsAlive() || m_bSelfDestructing )
		return;

	if ( m_bEnabled )
	{
		m_bEnabled = false;
		m_bAutoStart = false;

		SetEnemy( NULL );
		SetThink( &CNPC_FloorTurret::Retire );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
		SetThink( &CNPC_FloorTurret::DisabledThink );
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the turret's state via input function
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputToggle( inputdata_t &inputdata )
{
	Toggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputEnable( inputdata_t &inputdata )
{
	Enable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: Stops the turret from firing live rounds (still attempts to though)
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputDepleteAmmo( inputdata_t &inputdata )
{
	AddSpawnFlags( SF_FLOOR_TURRET_OUT_OF_AMMO );
}

//-----------------------------------------------------------------------------
// Purpose: Allows the turret to fire live rounds again
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputRestoreAmmo( inputdata_t &inputdata )
{
	RemoveSpawnFlags( SF_FLOOR_TURRET_OUT_OF_AMMO );
}

//-----------------------------------------------------------------------------
// Purpose: Reduce physics forces from the front
//-----------------------------------------------------------------------------
int CNPC_FloorTurret::VPhysicsTakeDamage( const CTakeDamageInfo &info )
{
	if ( VPhysicsGetObject() && !VPhysicsGetObject()->IsMotionEnabled() )
		return 0;	// we move for no one!

	return BaseClass::VPhysicsTakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
int CNPC_FloorTurret::OnTakeDamage( const CTakeDamageInfo &info )
{
	// Allow turret to take damage
	CheckStuff();

	CTakeDamageInfo	newInfo = info;

	// Doesn't really apply to the mod...
	/*if ( info.GetDamageType() & (DMG_SLASH|DMG_CLUB) )
	{
		// Take extra force from melee hits
		newInfo.ScaleDamageForce( 2.0f );
		
		// Disable our upright controller for some time
		if ( m_pMotionController != NULL )
		{
			m_pMotionController->Suspend( 2.0f );
		}
	}
	else if ( info.GetDamageType() & DMG_BLAST )
	{
		newInfo.ScaleDamageForce( 2.0f );
	}
	else if ( (info.GetDamageType() & DMG_BULLET) && !(info.GetDamageType() & DMG_BUCKSHOT) )
	{
		// Bullets, but not buckshot, do extra push
		newInfo.ScaleDamageForce( 2.5f );
	}*/

	// Manually apply vphysics because AI_BaseNPC takedamage doesn't call back to CBaseEntity OnTakeDamage
	//VPhysicsTakeDamage( newInfo );

	// Bump up our search time
	m_flLastSight = gpGlobals->curtime + FLOOR_TURRET_MAX_WAIT;

	// Start looking around in anger if we were idle
	if ( IsAlive() && m_bEnabled && m_bAutoStart && GetActivity() == ACT_FLOOR_TURRET_CLOSED_IDLE && m_bSelfDestructing == false )
	{
		SetThink( &CNPC_FloorTurret::Deploy );
	}

	return BaseClass::OnTakeDamage( newInfo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SpinUp( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SpinDown( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_FloorTurret::GetAttackDamageScale( CBaseEntity *pVictim )
{
	// Fixed turrets doing extra damage to certain enemies (that's not really too fair, now is it?)

	/*CBaseCombatCharacter *pBCC = pVictim->MyCombatCharacterPointer();

	// Do extra damage to antlions & combine
	if ( pBCC )
	{
		if ( pBCC->Classify() == CLASS_ANTLION )
			return 2.0;
			
		if ( pBCC->Classify() == CLASS_COMBINE )
			return 2.0;
	}*/

	return BaseClass::GetAttackDamageScale( pVictim );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CNPC_FloorTurret::GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget ) 
{
	// Fixed accuracy of turret depending on target, replaced with constant 'perfect' accuracy

	//WeaponProficiency_t weaponProficiency = WEAPON_PROFICIENCY_AVERAGE;

	// Switch our weapon proficiency based upon our target
	/*if ( pTarget )
	{
		if ( pTarget->Classify() == CLASS_PLAYER || pTarget->Classify() == CLASS_ANTLION || pTarget->Classify() == CLASS_ZOMBIE )
		{
			// Make me much more accurate
			weaponProficiency = WEAPON_PROFICIENCY_PERFECT;
		}
		else if ( pTarget->Classify() == CLASS_COMBINE )
		{
			// Make me more accurate
			weaponProficiency = WEAPON_PROFICIENCY_VERY_GOOD;
		}
	}*/

	WeaponProficiency_t weaponProficiency = WEAPON_PROFICIENCY_PERFECT;

	return VECTOR_CONE_10DEGREES * ((CBaseHLCombatWeapon::GetDefaultProficiencyValues())[ weaponProficiency ].spreadscale);
}

//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CNPC_FloorTurret::DrawDebugTextOverlays( void ) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		if (VPhysicsGetObject())
		{
			char tempstr[512];
			Q_snprintf(tempstr, sizeof(tempstr),"Mass: %.2f kg / %.2f lb (%s)", VPhysicsGetObject()->GetMass(), kg2lbs(VPhysicsGetObject()->GetMass()), GetMassEquivalent(VPhysicsGetObject()->GetMass()));
			EntityText( text_offset, tempstr, 0);
			text_offset++;
		}
	}

	return text_offset;
}

void CNPC_FloorTurret::UpdateMuzzleMatrix()
{
	if ( gpGlobals->tickcount != m_muzzleToWorldTick )
	{
		m_muzzleToWorldTick = gpGlobals->tickcount;
		GetAttachment( m_iMuzzleAttachment, m_muzzleToWorld );
	}
}

//-----------------------------------------------------------------------------
// Purpose: We override this code because otherwise we start to move into the
//			tricky realm of player avoidance.  Since we don't go through the
//			normal NPC thinking but we ARE an NPC (...) we miss a bunch of 
//			book keeping.  This means we can become invisible and then never
//			reappear.
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::PlayerPenetratingVPhysics( void )
{
	// We don't care!
}

#define SELF_DESTRUCT_DURATION			4.0f
#define SELF_DESTRUCT_BEEP_MIN_DELAY	0.1f
#define SELF_DESTRUCT_BEEP_MAX_DELAY	0.75f
#define SELF_DESTRUCT_BEEP_MIN_PITCH	100.0f
#define SELF_DESTRUCT_BEEP_MAX_PITCH	225.0f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::BreakThink( void )
{
	Vector vecUp;
	GetVectors( NULL, NULL, &vecUp );
	Vector vecOrigin = WorldSpaceCenter() + ( vecUp * 12.0f );

	// Our effect
#ifdef HL2_EPISODIC
	DispatchParticleEffect( "explosion_turret_break", vecOrigin, GetAbsAngles() );
#endif // HL2_EPISODIC

	// K-boom
	RadiusDamage( CTakeDamageInfo( this, this, 15.0f, DMG_BLAST ), vecOrigin, (10*12), CLASS_NONE, this );

	// Blow a player's turrets up when he or she dies
	// Make a turret explode once its health has been depleated
	ExplosionCreate( GetAbsOrigin() + Vector( 0, 0, 16 ), GetAbsAngles(), NULL, 0, 200, 
		SF_ENVEXPLOSION_NODAMAGE | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this );

	breakablepropparams_t params( GetAbsOrigin(), GetAbsAngles(), vec3_origin, RandomAngularImpulse( -800.0f, 800.0f ) );
	params.impactEnergyScale = 1.0f;
	params.defCollisionGroup = COLLISION_GROUP_INTERACTIVE;

	// no damage/damage force? set a burst of 100 for some movement
	params.defBurstScale = 100;
	PropBreakableCreateAll( GetModelIndex(), VPhysicsGetObject(), params, this, -1, true );

	// Throw out some small chunks too obscure the explosion even more
	CPVSFilter filter( vecOrigin );
	for ( int i = 0; i < 4; i++ )
	{
		Vector gibVelocity = RandomVector(-100,100);
		int iModelIndex = modelinfo->GetModelIndex( g_PropDataSystem.GetRandomChunkModel( "MetalChunks" ) );	
		te->BreakModel( filter, 0.0, vecOrigin, GetAbsAngles(), Vector(40,40,40), gibVelocity, iModelIndex, 150, 4, 2.5, BREAK_METAL );
	}

	// We're done!
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: The countdown to destruction!
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::SelfDestructThink( void )
{
	// Continue to animate
	PreThink( TURRET_SELF_DESTRUCTING );

	// If we're done, explode
	if ( ( gpGlobals->curtime - m_flDestructStartTime ) >= SELF_DESTRUCT_DURATION )
	{
		SetThink( &CNPC_FloorTurret::BreakThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
		UTIL_Remove( m_hFizzleEffect );
		m_hFizzleEffect = NULL;
		return;
	}

	// Find out where we are in the cycle of our destruction
	float flDestructPerc = clamp( ( gpGlobals->curtime - m_flDestructStartTime ) / SELF_DESTRUCT_DURATION, 0.0f, 1.0f );

	// Figure out when our next beep should occur
	float flBeepTime = SELF_DESTRUCT_BEEP_MAX_DELAY + ( ( SELF_DESTRUCT_BEEP_MIN_DELAY - SELF_DESTRUCT_BEEP_MAX_DELAY ) * flDestructPerc );

	// If it's time to beep again, do so
	if ( gpGlobals->curtime > ( m_flPingTime + flBeepTime ) )
	{
		// Figure out what our beep pitch will be
		float flBeepPitch = SELF_DESTRUCT_BEEP_MIN_PITCH + ( ( SELF_DESTRUCT_BEEP_MAX_PITCH - SELF_DESTRUCT_BEEP_MIN_PITCH ) * flDestructPerc );
		
		StopSound( "NPC_FloorTurret.AlarmPing" );

		// Play the beep
		CPASAttenuationFilter filter( this, "NPC_FloorTurret.AlarmPing" );
		EmitSound_t params;
		params.m_pSoundName = "NPC_FloorTurret.AlarmPing";
		params.m_nPitch = floor( flBeepPitch );
		params.m_nFlags = SND_CHANGE_PITCH;
		EmitSound( filter, entindex(), params );
		
		// Flash our eye
		SetEyeState( TURRET_EYE_ALARM );
		
		// Save this as the last time we pinged
		m_flPingTime = gpGlobals->curtime;
		
		// Randomly twitch
		m_vecGoalAngles.x = GetAbsAngles().x + random->RandomFloat( -60*flDestructPerc, 60*flDestructPerc );
		m_vecGoalAngles.y = GetAbsAngles().y + random->RandomFloat( -60*flDestructPerc, 60*flDestructPerc );
	}
	
	UpdateFacing();

	// Think again!
	SetNextThink( gpGlobals->curtime + 0.05f );
}

// Blow a player's turrets up when he or she dies
// Make a turret explode instantly once its health has been depleated
void CNPC_FloorTurret::Explode( void )
{
	m_bSelfDestructing = true;

	m_flDestructStartTime = gpGlobals->curtime;
	m_flPingTime = gpGlobals->curtime;

	CContingency_Player *pOwner = ToContingencyPlayer( GetOwnerEntity() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Your deployed turret has been destroyed." );
		pOwner->SetDeployedTurret( NULL );	// take note that our owner no longer has a deployable turret in the world
	}

	SetThink( &CNPC_FloorTurret::BreakThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	UTIL_Remove( m_hFizzleEffect );
	m_hFizzleEffect = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Make us explode
//-----------------------------------------------------------------------------
void CNPC_FloorTurret::InputSelfDestruct( inputdata_t &inputdata )
{
	// Ka-boom!
	m_flDestructStartTime = gpGlobals->curtime;
	m_flPingTime = gpGlobals->curtime;
	m_bSelfDestructing = true;

	SetThink( &CNPC_FloorTurret::SelfDestructThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	// Create the dust effect in place
	m_hFizzleEffect = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
	if ( m_hFizzleEffect != NULL )
	{
		Vector vecUp;
		GetVectors( NULL, NULL, &vecUp );

		// Setup our basic parameters
		m_hFizzleEffect->KeyValue( "start_active", "1" );
		m_hFizzleEffect->KeyValue( "effect_name", "explosion_turret_fizzle" );
		m_hFizzleEffect->SetParent( this );
		m_hFizzleEffect->SetAbsOrigin( WorldSpaceCenter() + ( vecUp * 12.0f ) );
		DispatchSpawn( m_hFizzleEffect );
		m_hFizzleEffect->Activate();
	}
}

void CNPC_FloorTurret::Event_Killed( const CTakeDamageInfo &info )
{
	Explode();

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_turret_floor, CNPC_FloorTurret )

	DECLARE_INTERACTION( g_interactionTurretStillStanding );	

AI_END_CUSTOM_NPC()
