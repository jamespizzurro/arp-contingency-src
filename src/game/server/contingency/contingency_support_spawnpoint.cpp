// Added wave system

#include "cbase.h"

#include "contingency_support_spawnpoint.h"

#include "ai_basenpc.h"
#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Added support wave system
extern ConVar contingency_wave_support;

static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

LINK_ENTITY_TO_CLASS( contingency_support_wave_spawner, CContingencySupportWaveSpawner );
LINK_ENTITY_TO_CLASS( contingency_support_spawnpoint, CContingencySupportWaveSpawner );	// legacy support/backwards compatibility

BEGIN_DATADESC( CContingencySupportWaveSpawner )

	DEFINE_KEYFIELD( m_ChildTargetName,		FIELD_STRING,	"NPCTargetname" ),
	DEFINE_KEYFIELD( m_SquadName,			FIELD_STRING,	"NPCSquadName" ),
	DEFINE_KEYFIELD( m_strHintGroup,		FIELD_STRING,	"NPCHintGroup" ),

	// Add a custom rally point entity for wave spawners
	DEFINE_KEYFIELD( rallyPointName, FIELD_STRING, "rallyPointName" ),

	DEFINE_KEYFIELD( m_flMaximumDistanceFromNearestPlayer, FIELD_FLOAT, "MaxPlayerDistance" ),

END_DATADESC()

CContingencySupportWaveSpawner::CContingencySupportWaveSpawner( void )
{
}

CContingencySupportWaveSpawner::~CContingencySupportWaveSpawner( void )
{
}

void CContingencySupportWaveSpawner::Spawn( void )
{
	BaseClass::Spawn();

	// Add a custom rally point entity for wave spawners
	pRallyPoint = dynamic_cast<CContingencyRallyPoint*>( gEntList.FindEntityByName(NULL, rallyPointName, this) );
}

void CContingencySupportWaveSpawner::MakeNPC( void )
{
	if ( !CanMakeNPC() )
		return;

	// If the server doesn't want support NPCs, don't spawn them
	if ( !contingency_wave_support.GetBool() )
		return;

	// Only spawn support NPCs during an interim phase
	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
		return;

	// Only spawn as many support NPCs as it takes to fill the server with players
	// (e.g. Having 2 out of 5 players on a server means there are 3 spots to fill, so we should spawn 3 support NPCs)
	// Never spawn too many!
	if ( ContingencyRules()->GetCurrentSupportWaveNPCList() && (ContingencyRules()->GetCurrentSupportWaveNPCList()->Count() >= (ContingencyRules()->GetMaxNumPlayers() - ContingencyRules()->GetTotalNumPlayers())) )
		return;

	if ( m_flMaximumDistanceFromNearestPlayer > -1 )
	{
		// This spawner is restricted to a certain spawning distance
		// In other words, at least one player must be within the defined
		// maximum distance away (m_flMaximumDistanceFromNearestPlayer)
		// from this spawner in order for it to function

		CBasePlayer *pNearestPlayer = UTIL_GetNearestPlayer( this->GetAbsOrigin() );
		if ( !pNearestPlayer )
			return;	// no players found?

		if ( (this->GetAbsOrigin() - pNearestPlayer->GetAbsOrigin()).LengthSqr() > (m_flMaximumDistanceFromNearestPlayer * m_flMaximumDistanceFromNearestPlayer) )
			return;	// nearest player is too far away!
	}

	// Spawn a random type of support NPC
	const char *NPCClassName = "";
	string_t equipmentName = NULL_STRING;
	NPCClassName = kSupportWaveSupportNPCTypes[random->RandomInt( 0, NUM_SUPPORT_NPCS - 1 )];
	if ( Q_strcmp(NPCClassName, "npc_citizen") == 0 )
		equipmentName = MAKE_STRING(kSupportWaveCitizenWeaponTypes[random->RandomInt( 0, NUM_CITIZEN_WEAPONS - 1 )]);

	// Ensure the NPC type we've defined is valid
	CAI_BaseNPC *pent = dynamic_cast<CAI_BaseNPC*>( CreateEntityByName(NPCClassName) );
	if ( !pent )
		return;

	m_OnSpawnNPC.Set( pent, pent, this );

	pent->SetAbsOrigin( GetAbsOrigin() );

	// Strip pitch and roll from the spawner's angles while passing only yaw to the NPC
	QAngle angles = GetAbsAngles();
	angles.x = 0.0;
	angles.z = 0.0;
	pent->SetAbsAngles( angles );

	// Add a custom rally point entity for wave spawners
	if ( pRallyPoint )
		pent->SetRallyPoint( pRallyPoint );

	// Force all NPCs to fall to the ground when they spawn and fade away when they die
	pent->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );
	pent->AddSpawnFlags( SF_NPC_FADE_CORPSE );

	pent->AddSpawnFlags( SF_NPC_NO_WEAPON_DROP );

	pent->AddSpawnFlags( SF_NPC_ALWAYSTHINK );
	
	// As long as we aren't wielding a weapon, we're safe to look long ranges
	// NOTE: NPCs who have a weapon and this spawn flag enabled fire at insanely long distances,
	// hence why we're preventing that from happening here!
	if ( equipmentName == NULL_STRING )
		pent->AddSpawnFlags( SF_NPC_LONG_RANGE );

	// Apply any defined squads and hint groups the mapper may have defined
	// as well as weapons (if applicable)
	pent->SetSquadName( m_SquadName );
	pent->SetHintGroup( m_strHintGroup );
	if ( equipmentName != NULL_STRING )
		pent->m_spawnEquipment = equipmentName;

	ChildPreSpawn( pent );

	DispatchSpawn( pent );
	pent->SetOwnerEntity( this );

	// Fade spawned NPC in to the world
	pent->SetRenderColorA( 0 );
	pent->m_nRenderFX = kRenderFxSolidFast;
	pent->m_flSpeed = gpGlobals->curtime + 3.0f;

	DispatchActivate( pent );

	if ( m_ChildTargetName != NULL_STRING )
	{
		// if I have a netname (overloaded), give the child NPC that name as a targetname
		pent->SetName( m_ChildTargetName );
	}

	ChildPostSpawn( pent );

	m_nLiveChildren++;// count this NPC

	if ( !(m_spawnflags & SF_NPCMAKER_INF_CHILD) )
	{
		m_nMaxNumNPCs--;

		if ( IsDepleted() )
		{
			m_OnAllSpawned.FireOutput( this, this );

			// Disable this forever.  Don't kill it because it still gets death notices
			SetThink( NULL );
			SetUse( NULL );
		}
	}

	// Consider this NPC a new addition to our support wave
	if ( ContingencyRules()->GetCurrentSupportWaveNPCList() && (ContingencyRules()->GetCurrentSupportWaveNPCList()->Find(pent) == -1) )
		ContingencyRules()->GetCurrentSupportWaveNPCList()->AddToTail( pent );
}
