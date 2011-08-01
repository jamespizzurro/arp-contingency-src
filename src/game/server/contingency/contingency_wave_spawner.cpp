// Added wave system

#include "cbase.h"

#include "contingency_wave_spawner.h"

#include "ai_basenpc.h"
#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar contingency_wave_maxlivingnpcs;

static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

LINK_ENTITY_TO_CLASS( contingency_wave_spawner, CContingencyWaveSpawner );

BEGIN_DATADESC( CContingencyWaveSpawner )

	DEFINE_KEYFIELD( m_ChildTargetName,		FIELD_STRING,	"NPCTargetname" ),
	DEFINE_KEYFIELD( m_SquadName,			FIELD_STRING,	"NPCSquadName" ),
	DEFINE_KEYFIELD( m_strHintGroup,		FIELD_STRING,	"NPCHintGroup" ),

	// Add a custom rally point entity for wave spawners
	DEFINE_KEYFIELD( rallyPointName, FIELD_STRING, "rallyPointName" ),

	DEFINE_KEYFIELD( m_flMaximumDistanceFromNearestPlayer, FIELD_FLOAT, "MaxPlayerDistance" ),

END_DATADESC()

CContingencyWaveSpawner::CContingencyWaveSpawner( void )
{
}

CContingencyWaveSpawner::~CContingencyWaveSpawner( void )
{
}

void CContingencyWaveSpawner::Spawn( void )
{
	BaseClass::Spawn();

	// Add a custom rally point entity for wave spawners
	pRallyPoint = dynamic_cast<CContingencyRallyPoint*>( gEntList.FindEntityByName(NULL, rallyPointName, this) );
}

void CContingencyWaveSpawner::MakeNPC( void )
{
	if ( !CanMakeNPC() )
		return;

	// Only spawn NPCs during waves (combat phases), hence our name!
	if ( ContingencyRules()->GetCurrentPhase() != PHASE_COMBAT )
		return;

	// Do not spawn more NPCs than our server will allow to be living at any given time
	if ( ContingencyRules()->GetNumNPCsInCurrentWave() >= contingency_wave_maxlivingnpcs.GetInt() )
		return;

	// Do not spawn more NPCs than we're supposed to for this wave
	if ( ContingencyRules()->GetNumEnemiesSpawned() >= ContingencyRules()->GetCalculatedNumEnemies() )
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

	// Spawn a random type of NPC type associated with the current wave
	int currentWaveType = ContingencyRules()->GetWaveType();
	const char *NPCClassName = "";
	string_t equipmentName = NULL_STRING;
	if ( currentWaveType == WAVE_HEADCRABS )
	{
		// Don't spawn headcrab NPCs if this spawner isn't allowed to (map-defined)
		if ( !HasSpawnFlags(SF_WAVESPAWNER_HEADCRABS) )
			return;

		if ( HasSpawnFlags(SF_WAVESPAWNER_FLYINGNPCSONLY) )
			return;	// there are no flying headcrab NPCs (yet)
		else
			NPCClassName = kWaveHeadcrabsNPCTypes[random->RandomInt(0, NUM_HEADCRAB_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_ANTLIONS )
	{
		// Don't spawn antlion NPCs if this spawner isn't allowed to (map-defined)
		if ( !HasSpawnFlags(SF_WAVESPAWNER_ANTLIONS) )
			return;

		if ( HasSpawnFlags(SF_WAVESPAWNER_FLYINGNPCSONLY) )
			NPCClassName = kWaveAntlionsFlyingNPCTypes[random->RandomInt(0, NUM_ANTLION_FLYING_NPCS - 1)];
		else
			NPCClassName = kWaveAntlionsNPCTypes[random->RandomInt(0, NUM_ANTLION_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_ZOMBIES )
	{
		// Don't spawn zombie NPCs if this spawner isn't allowed to (map-defined)
		if ( !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIES) )
			return;

		if ( HasSpawnFlags(SF_WAVESPAWNER_FLYINGNPCSONLY) )
			return;	// there are no flying zombie NPCs (yet)
		else
			NPCClassName = kWaveZombiesNPCTypes[random->RandomInt(0, NUM_ZOMBIE_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_COMBINE )
	{
		// Don't spawn Combine NPCs if this spawner isn't allowed to (map-defined)
		if ( !HasSpawnFlags(SF_WAVESPAWNER_COMBINE) )
			return;

		if ( HasSpawnFlags(SF_WAVESPAWNER_FLYINGNPCSONLY) )
			NPCClassName = kWaveCombineFlyingNPCTypes[random->RandomInt(0, NUM_COMBINE_FLYING_NPCS - 1)];
		else
			NPCClassName = kWaveCombineNPCTypes[random->RandomInt(0, NUM_COMBINE_NPCS - 1)];

		if ( Q_strcmp(NPCClassName, "npc_combine_s") == 0 )
			equipmentName = MAKE_STRING(kWaveCombineSWeaponTypes[random->RandomInt( 0, NUM_COMBINE_S_WEAPONS - 1 )]);
		else if ( Q_strcmp(NPCClassName, "npc_metropolice") == 0 )
			equipmentName = MAKE_STRING(kWaveMetropoliceWeaponTypes[random->RandomInt( 0, NUM_METROPOLICE_WEAPONS - 1 )]);
	}
	else
		return;

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

	// Apply any defined squads and hint groups the mapper may have defined
	// as well as weapons (if applicable)
	pent->SetSquadName( m_SquadName );
	pent->SetHintGroup( m_strHintGroup );
	if ( equipmentName != NULL_STRING )
		pent->m_spawnEquipment = equipmentName;

	ChildPreSpawn( pent );

	DispatchSpawn( pent );
	pent->SetOwnerEntity( this );
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

	// Consider this NPC yet another enemy players will have to deal with
	// if they want to conquer this latest wave!
	ContingencyRules()->AddNPCToCurrentWave( pent );
	ContingencyRules()->IncrementNumEnemiesSpawned();
}
