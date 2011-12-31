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
	ConVar wrench_previewprop_ready( "wrench_previewprop_ready", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN );
#endif

class CWeaponWrench : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponWrench, CBaseHL2MPBludgeonWeapon );

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
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

	bool CanSpawnProp( CContingency_Player *pPlayer, bool shouldPrintMessages );
	void ShowPropSelectionPanel( void );

#ifdef CLIENT_DLL
	void DrawSpawnablePropPreview( CContingency_Player *pOwner );
#endif

	void Precache( void );
	bool Deploy( void );
	void ItemPostFrame( void );
#ifndef CLIENT_DLL
	void WaitingOnSpawnablePropDataThink( void );
#endif
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void AddViewKick( void );
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	void Drop( const Vector &vecVelocity );
	void ItemHolsterFrame( void );

private:

#ifdef CLIENT_DLL
	CHandle<C_Contingency_SpawnableProp> m_hSpawnablePropPreview;
	QAngle m_angSpawnablePropPreviewLastSavedAngles;
	float m_flSpawnablePropPreviewOriginX;
	float m_flSpawnablePropPreviewOriginY;
	float m_flSpawnablePropPreviewOriginZ;
	float m_flSpawnablePropPreviewAnglesPitch;
	float m_flSpawnablePropPreviewAnglesYaw;
	float m_flSpawnablePropPreviewAnglesRoll;
#else
	Vector m_vecSpawnablePropOrigin;
	QAngle m_angSpawnablePropAngles;
#endif

#ifndef CLIENT_DLL
	bool m_bShouldShowHint;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponWrench, DT_WeaponWrench )

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponWrench )
	DEFINE_THINKFUNC( WaitingOnSpawnablePropDataThink ),
END_DATADESC()
#endif

BEGIN_NETWORK_TABLE( CWeaponWrench, DT_WeaponWrench )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponWrench )
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
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}

	m_flSpawnablePropPreviewOriginX = 0.0f;
	m_flSpawnablePropPreviewOriginY = 0.0f;
	m_flSpawnablePropPreviewOriginZ = 0.0f;
	m_flSpawnablePropPreviewAnglesPitch = 0.0f;
	m_flSpawnablePropPreviewAnglesYaw = 0.0f;
	m_flSpawnablePropPreviewAnglesRoll = 0.0f;
#endif

#ifndef CLIENT_DLL
	m_bShouldShowHint = false;
#endif
}

CWeaponWrench::~CWeaponWrench( void )
{
#ifdef CLIENT_DLL
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

bool CWeaponWrench::CanSpawnProp( CContingency_Player *pPlayer, bool shouldPrintMessages )
{
	if ( !ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		if ( shouldPrintMessages )
			ClientPrint( pPlayer, HUD_PRINTTALK, "Only living players can spawn props." );

		return false;
	}

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		if ( shouldPrintMessages )
			ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be spawned during interim phases." );
		
		return false;
	}

	int iSpawnablePropIndex = pPlayer->GetDesiredSpawnablePropIndex();
	if ( !pPlayer->HasCredits(Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1])) )
	{
		if ( shouldPrintMessages )
			ClientPrint( pPlayer, HUD_PRINTTALK, "You do not have enough credits to spawn this prop." );
		
		return false;
	}

	if ( pPlayer->GetNumSpawnableProps() >= ContingencyRules()->GetMapMaxPropsPerPlayer() )
	{
		if ( shouldPrintMessages )
			ClientPrint( pPlayer, HUD_PRINTTALK, "You have hit the map's maximum spawnable prop limit! Remove at least one of your existing props, then try again." );
		
		return false;
	}

	return true;
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

#ifdef CLIENT_DLL
void CWeaponWrench::DrawSpawnablePropPreview( CContingency_Player *pOwner )
{
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}

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
	//if ( tr.fraction < 1.0 )
	{
		int iSpawnablePropIndex = pOwner->GetDesiredSpawnablePropIndex();

		// Handle preview prop creation & spawning
		C_Contingency_SpawnableProp* pSpawnablePropPreview = new C_Contingency_SpawnableProp();
		pSpawnablePropPreview->InitializeAsClientEntity( kSpawnablePropTypes[iSpawnablePropIndex][3], RENDER_GROUP_TRANSLUCENT_ENTITY );
		pSpawnablePropPreview->SetRenderMode( kRenderTransTexture );
		pSpawnablePropPreview->SetRenderColorA( 200 );
		pSpawnablePropPreview->AddEffects( EF_NORECEIVESHADOW | EF_ITEM_BLINK );

		// Handle preview prop origin and angles
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

		// Update origin and angles variables
		m_flSpawnablePropPreviewOriginX = pSpawnablePropPreview->GetLocalOrigin().x;
		m_flSpawnablePropPreviewOriginY = pSpawnablePropPreview->GetLocalOrigin().y;
		m_flSpawnablePropPreviewOriginZ = pSpawnablePropPreview->GetLocalOrigin().z;
		m_flSpawnablePropPreviewAnglesPitch = pSpawnablePropPreview->GetLocalAngles()[PITCH];
		m_flSpawnablePropPreviewAnglesYaw = pSpawnablePropPreview->GetLocalAngles()[YAW];
		m_flSpawnablePropPreviewAnglesRoll = pSpawnablePropPreview->GetLocalAngles()[ROLL];

		// Handle preview prop's color
		color32 color = pSpawnablePropPreview->GetRenderColor();

		if ( CanSpawnProp(pOwner, false) )
			pSpawnablePropPreview->SetRenderColor( 0, color.g, 0 );
		else
			pSpawnablePropPreview->SetRenderColor( color.r, 0, 0 );

		m_hSpawnablePropPreview = pSpawnablePropPreview;
	}
}
#endif

void CWeaponWrench::Precache( void )
{
	BaseClass::Precache();
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
#endif

#ifndef CLIENT_DLL
	m_bShouldShowHint = true;
#endif

		return true;
	}

	return false;
}

void CWeaponWrench::ItemPostFrame( void )
{
	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	if ( !pOwner )
		return;

#ifndef CLIENT_DLL
	if ( m_bShouldShowHint )
	{
		UTIL_HudHintText( pOwner, "#Contingency_Hint_Wrench" );
		m_bShouldShowHint = false;
	}
#endif

#ifdef CLIENT_DLL
	DrawSpawnablePropPreview( pOwner );
#endif

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

	if ( !CanSpawnProp(pPlayer, true) )
		goto FinishingStuff;

#ifdef CLIENT_DLL
	// Update origin and angles ConVars according to the values of our respective variables
	wrench_previewprop_origin_x.SetValue( m_flSpawnablePropPreviewOriginX );
	wrench_previewprop_origin_y.SetValue( m_flSpawnablePropPreviewOriginY );
	wrench_previewprop_origin_z.SetValue( m_flSpawnablePropPreviewOriginZ );
	wrench_previewprop_angles_pitch.SetValue( m_flSpawnablePropPreviewAnglesPitch );
	wrench_previewprop_angles_yaw.SetValue( m_flSpawnablePropPreviewAnglesYaw );
	wrench_previewprop_angles_roll.SetValue( m_flSpawnablePropPreviewAnglesRoll );
	wrench_previewprop_ready.SetValue( 1 );	// indicates to the server that new data has been written to our ConVars that needs to be read ASAP
#else
	// Get updated origin and angles from our client's ConVars as soon as possible
	SetThink( &CWeaponWrench::WaitingOnSpawnablePropDataThink );
	SetNextThink( gpGlobals->curtime );
#endif

	// Play a nice construction sound
	WeaponSound( SINGLE );

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

#ifndef CLIENT_DLL
void CWeaponWrench::WaitingOnSpawnablePropDataThink( void )
{
	// Here, we wait on the client to send us updated origin and angles data
	// that we'll need to accurately spawn our spawnable prop

	CContingency_Player *pOwner = ToContingencyPlayer( GetOwner() );
	if ( !pOwner )
	{
		SetNextThink( NULL );
		return;
	}

	if ( Q_strcmp(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_ready"), "0") == 0 )
	{
		// We have not yet received the data the client sent us,
		// so continue checking until we do
		SetNextThink( gpGlobals->curtime );
		return;
	}

	// We have received the data the client sent to us,
	// so we can now use that data to update ours

	m_vecSpawnablePropOrigin = Vector( Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_x")),
		Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_y")),
		Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_origin_z")) );
	m_angSpawnablePropAngles = QAngle( Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_pitch")),
		Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_yaw")),
		Q_atof(engine->GetClientConVarValue(engine->IndexOfEdict(pOwner->edict()), "wrench_previewprop_angles_roll")) );

	// Reset the client's ready ConVar now that the client's data
	// has been received and processed
	engine->ClientCommand( pOwner->edict(), "wrench_previewprop_ready 0" );

	// Spawn our spawnable prop now that we have the
	// latest origin and angles data from the client to do so
	CContingency_SpawnableProp *pSpawnableProp = dynamic_cast<CContingency_SpawnableProp*>( CreateEntityByName("contingency_spawnableprop") );
	if ( pSpawnableProp )
	{
		int iSpawnablePropIndex = pOwner->GetDesiredSpawnablePropIndex();
		pSpawnableProp->SetModel( kSpawnablePropTypes[iSpawnablePropIndex][3] );
		pSpawnableProp->SetAbsOrigin( m_vecSpawnablePropOrigin );
		pSpawnableProp->SetAbsAngles( m_angSpawnablePropAngles );

		pSpawnableProp->SetSpawnerPlayer( pOwner );
		pSpawnableProp->SetSpawnablePropIndex( iSpawnablePropIndex );
		if ( pOwner->m_SpawnablePropList.Find( pSpawnableProp ) == -1 )
		{
			pOwner->m_SpawnablePropList.AddToTail( pSpawnableProp );	// add to our spawner's list of spawnable props
			pOwner->SetNumSpawnableProps( pOwner->GetNumSpawnableProps() + 1 );
		}

		// Actually spawn it and stuff
		pSpawnableProp->Precache();
		DispatchSpawn( pSpawnableProp );
		pSpawnableProp->Activate();

		pOwner->UseCredits( Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1]) );	// spawned, so use up some of the player's credits
	}

	// Reaffirm the creation of the prop by playing our nice construction sound again
	WeaponSound( SINGLE );

	SetNextThink( NULL );	// our work here is done...until next time...
}
#endif

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

void CWeaponWrench::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

#ifdef CLIENT_DLL
	if ( m_hSpawnablePropPreview )
	{
		delete m_hSpawnablePropPreview;
		m_hSpawnablePropPreview = NULL;
	}
#endif	
}
