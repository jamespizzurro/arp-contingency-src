// Added spawnable prop system

#include "cbase.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#include "in_buttons.h"

#include "contingency_gamerules.h"

#ifdef CLIENT_DLL
	#include "c_contingency_spawnableprop.h"
#else
	#include "contingency_spawnableprop.h"
#endif

#include "contingency_system_propspawning.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	WRENCH_RANGE 128.0f
#define	WRENCH_REFIRE 0.8f
#define WRENCH_DAMAGE 0.0f

#ifdef CLIENT_DLL
	#define CWeaponWrench C_WeaponWrench
	#define CContingency_SpawnableProp C_Contingency_SpawnableProp
#endif

#ifdef CLIENT_DLL
	// The ConVars below are used by the server to read client-specific data
	ConVar wrench_previewprop_origin_x( "wrench_previewprop_origin_x", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
	ConVar wrench_previewprop_origin_y( "wrench_previewprop_origin_y", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
	ConVar wrench_previewprop_origin_z( "wrench_previewprop_origin_z", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
	ConVar wrench_previewprop_angles_pitch( "wrench_previewprop_angles_pitch", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
	ConVar wrench_previewprop_angles_yaw( "wrench_previewprop_angles_yaw", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
	ConVar wrench_previewprop_angles_roll( "wrench_previewprop_angles_roll", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
#endif

class CWeaponWrench : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponWrench, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponWrench();
	~CWeaponWrench( void );

	float GetRange( void ) { return WRENCH_RANGE; }
	float GetFireRate( void ) { return WRENCH_REFIRE; }
	float GetDamageForActivity( Activity hitActivity ) { return WRENCH_DAMAGE; }

	// Allow weapons to override players' view angles
	void OverrideMouseInput( float *x, float *y );
	bool OverrideViewAngles( void );

	void ShowPropSelectionPanel( void );
	void DrawSpawnablePropPreview( void );

	void Precache( void );
	bool Deploy( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void AddViewKick( void );
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	void Drop( const Vector &vecVelocity );

private:
	CNetworkVar( bool, m_bCanSpawnProp );

#ifdef CLIENT_DLL
	bool m_bShouldShowSpawnablePropPreview;
	CHandle<C_Contingency_SpawnableProp> m_hSpawnablePropPreview;
	QAngle m_angSpawnablePropPreviewLastSavedAngles;
#else
	Vector m_vecSpawnablePropOrigin;
	QAngle m_angSpawnablePropAngles;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponWrench, DT_WeaponWrench )

BEGIN_NETWORK_TABLE( CWeaponWrench, DT_WeaponWrench )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO(m_bCanSpawnProp) ),
#else
	SendPropBool( SENDINFO(m_bCanSpawnProp) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponWrench )
	DEFINE_PRED_FIELD( m_bCanSpawnProp, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_wrench, CWeaponWrench );
PRECACHE_WEAPON_REGISTER( weapon_wrench );

acttable_t	CWeaponWrench::m_acttable[] = 
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

IMPLEMENT_ACTTABLE( CWeaponWrench );

CWeaponWrench::CWeaponWrench( void )
{
#ifdef CLIENT_DLL
	m_bShouldShowSpawnablePropPreview = false;
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}
#else
	m_bCanSpawnProp = false;
#endif
}

CWeaponWrench::~CWeaponWrench( void )
{
#ifdef CLIENT_DLL
	m_bShouldShowSpawnablePropPreview = false;
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}
#endif
}

// Allow weapons to override players' view angles
void CWeaponWrench::OverrideMouseInput( float *x, float *y )
{
	return;
}

// Allow weapons to override players' view angles
bool CWeaponWrench::OverrideViewAngles( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return false;

	if ( pOwner->m_nButtons & IN_RELOAD )
		return true;

	return false;
}

// Shows our prop selection panel to the owner
void CWeaponWrench::ShowPropSelectionPanel( void )
{
#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->ShowViewPortPanel( "propspawningmenu", true, NULL );
#endif
}

void CWeaponWrench::DrawSpawnablePropPreview( void )
{
#ifdef CLIENT_DLL
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}

	if ( !m_bShouldShowSpawnablePropPreview )
		return;
#endif

	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	if ( !pOwner )
		return;
	
	trace_t tr;
	Vector forward;
	pOwner->EyeVectors( &forward );
	UTIL_TraceLine( pOwner->EyePosition(),
		pOwner->EyePosition() + forward * WRENCH_RANGE,
		MASK_ALL,
		pOwner,
		COLLISION_GROUP_PLAYER_MOVEMENT,
		&tr );
	if ( tr.fraction < 1.0 )
	{
		int iSpawnablePropIndex = pOwner->GetDesiredSpawnablePropIndex();

#ifdef CLIENT_DLL
		// Handle preview prop creation & spawning
		C_Contingency_SpawnableProp* pSpawnablePropPreview = new C_Contingency_SpawnableProp();
		pSpawnablePropPreview->InitializeAsClientEntity( kSpawnablePropTypes[iSpawnablePropIndex][3], RENDER_GROUP_TRANSLUCENT_ENTITY );
		pSpawnablePropPreview->SetRenderMode( kRenderTransTexture );
		pSpawnablePropPreview->SetRenderColorA( 200 );
		pSpawnablePropPreview->AddEffects( EF_NORECEIVESHADOW | EF_ITEM_BLINK );
#endif

		// Handle preview prop origin and angles
#ifdef CLIENT_DLL
		UTIL_SetOrigin( pSpawnablePropPreview, Vector(tr.endpos.x, tr.endpos.y, tr.endpos.z + Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][4])) );
		
		if ( pOwner->m_nButtons & IN_RELOAD )
		{
			pSpawnablePropPreview->SetLocalAngles( QAngle(m_angSpawnablePropPreviewLastSavedAngles[PITCH],
				m_angSpawnablePropPreviewLastSavedAngles[YAW] - (pOwner->GetCurrentUserCommand()->mousedx * 0.05f),
				m_angSpawnablePropPreviewLastSavedAngles[ROLL] + (pOwner->GetCurrentUserCommand()->mousedy * 0.05f)) );
		}
		else
			pSpawnablePropPreview->SetLocalAngles( QAngle(0, pOwner->EyeAngles().y + Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][5]), 0) );

		m_angSpawnablePropPreviewLastSavedAngles = pSpawnablePropPreview->GetLocalAngles();

		// Update origin and angles ConVars
		wrench_previewprop_origin_x.SetValue( pSpawnablePropPreview->GetLocalOrigin().x );
		wrench_previewprop_origin_y.SetValue( pSpawnablePropPreview->GetLocalOrigin().y );
		wrench_previewprop_origin_z.SetValue( pSpawnablePropPreview->GetLocalOrigin().z );
		wrench_previewprop_angles_pitch.SetValue( pSpawnablePropPreview->GetLocalAngles()[PITCH] );
		wrench_previewprop_angles_yaw.SetValue( pSpawnablePropPreview->GetLocalAngles()[YAW] );
		wrench_previewprop_angles_roll.SetValue( pSpawnablePropPreview->GetLocalAngles()[ROLL] );
#else
		// Read from client's origin and angles ConVars
		m_vecSpawnablePropOrigin = Vector( Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_x")),
			Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_y")),
			Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_z")) );
		m_angSpawnablePropAngles = QAngle( Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_pitch")),
			Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_yaw")),
			Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_roll")) );
#endif

		// Handle preview prop color according to whether or not it fits in the given space
#ifndef CLIENT_DLL
		int boardModelIndex = modelinfo->GetModelIndex( kSpawnablePropTypes[iSpawnablePropIndex][3] );
		CPhysCollide *pSpawnablePropPreviewCollide = modelinfo->GetVCollide( boardModelIndex )->solids[0];
		Vector mins, maxs;
		physcollision->CollideGetAABB( &mins, &maxs, pSpawnablePropPreviewCollide, m_vecSpawnablePropOrigin, m_angSpawnablePropAngles );
		CBaseEntity *list[1];
		(UTIL_EntitiesInBox(list, 1, mins, maxs, MASK_ALL) > 0) ? m_bCanSpawnProp = false : m_bCanSpawnProp = true;
#else
		color32 color = pSpawnablePropPreview->GetRenderColor();

		if ( m_bCanSpawnProp )
			pSpawnablePropPreview->SetRenderColor( 0, color.g, 0 );
		else
			pSpawnablePropPreview->SetRenderColor( color.r, 0, 0 );

		m_hSpawnablePropPreview = pSpawnablePropPreview;
#endif
	}
#ifndef CLIENT_DLL
	else
		m_bCanSpawnProp = false;
#endif
}

void CWeaponWrench::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	// TODO: Do this sooner (?)
	for ( int i = 0; i < NUM_SPAWNABLEPROP_TYPES; i++ )
		PrecacheModel( kSpawnablePropTypes[i][3] );
#endif
}

bool CWeaponWrench::Deploy( void )
{
	if ( BaseClass::Deploy() )
	{
#ifdef CLIENT_DLL
		if ( m_hSpawnablePropPreview )
		{
			delete m_hSpawnablePropPreview;
			m_hSpawnablePropPreview = NULL;
		}
		m_bShouldShowSpawnablePropPreview = true;
#endif

		return true;
	}

	return false;
}

void CWeaponWrench::ItemPostFrame( void )
{
	DrawSpawnablePropPreview();

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime) )
		PrimaryAttack();
	else if ( (pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime) )
		SecondaryAttack();
	else
		WeaponIdle();
}

void CWeaponWrench::PrimaryAttack( void )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( !ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Only living players can spawn props." );
		goto FinishingStuff;
	}

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be spawned during interim phases." );
		goto FinishingStuff;
	}

	int iSpawnablePropIndex = pPlayer->GetDesiredSpawnablePropIndex();
	if ( !pPlayer->HasCredits(Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1])) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You do not have enough credits to spawn this prop." );
		goto FinishingStuff;
	}

	if ( pPlayer->GetNumSpawnableProps() >= ContingencyRules()->GetMapMaxPropsPerPlayer() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You have hit the map's maximum spawnable prop limit! Remove at least one of your existing props, then try again." );
		goto FinishingStuff;
	}

	if ( !m_bCanSpawnProp )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You cannot spawn this prop here." );
		goto FinishingStuff;
	}

#ifndef CLIENT_DLL
	// Handle prop creation & spawning
	CContingency_SpawnableProp *pSpawnableProp = dynamic_cast<CContingency_SpawnableProp*>( CreateEntityByName("contingency_spawnableprop") );
	if ( pSpawnableProp )
	{
		pSpawnableProp->SetModel( kSpawnablePropTypes[iSpawnablePropIndex][3] );
		pSpawnableProp->SetAbsOrigin( m_vecSpawnablePropOrigin );
		pSpawnableProp->SetAbsAngles( m_angSpawnablePropAngles );

		pSpawnableProp->SetSpawnerPlayer( pPlayer );
		pSpawnableProp->SetSpawnablePropIndex( iSpawnablePropIndex );
		if ( pPlayer->m_SpawnablePropList.Find( pSpawnableProp ) == -1 )
		{
			pPlayer->m_SpawnablePropList.AddToTail( pSpawnableProp );	// add to our spawner's list of spawnable props
			pPlayer->SetNumSpawnableProps( pPlayer->GetNumSpawnableProps() + 1 );
		}

		// Actually spawn it and stuff
		pSpawnableProp->Precache();
		DispatchSpawn( pSpawnableProp );
		pSpawnableProp->Activate();

		pPlayer->UseCredits( Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1]) );	// spawned, so use up some of the player's credits
	}
#endif

	// Handle weapon animations
	SendWeaponAnim( ACT_VM_HITCENTER );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	ToContingencyPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	// Handle view changes
	AddViewKick();

FinishingStuff:
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
}

void CWeaponWrench::SecondaryAttack( void )
{
	// Pop up our prop selection panel for our owner
	ShowPropSelectionPanel();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
}

void CWeaponWrench::AddViewKick( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	QAngle punchAng;

	punchAng.x = SharedRandomFloat( "crowbarpax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "crowbarpay", -2.0f, -1.0f );
	punchAng.z = 0.0f;

	pPlayer->ViewPunch( punchAng );
}

bool CWeaponWrench::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( BaseClass::Holster(pSwitchingTo) )
	{
#ifdef CLIENT_DLL
		m_bShouldShowSpawnablePropPreview = false;
		if ( m_hSpawnablePropPreview )
		{
			delete m_hSpawnablePropPreview;
			m_hSpawnablePropPreview = NULL;
		}
#endif

		return true;
	}

	return false;
}

void CWeaponWrench::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}
