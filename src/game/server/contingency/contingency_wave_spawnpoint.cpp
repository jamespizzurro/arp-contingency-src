// Added wave system

#include "cbase.h"

#include "contingency_wave_spawnpoint.h"

#include "ai_basenpc.h"
#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void DispatchActivate( CBaseEntity *pEntity )
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, false );
	pEntity->Activate();
	mdlcache->SetAsyncLoad( MDLCACHE_ANIMBLOCK, bAsyncAnims );
}

LINK_ENTITY_TO_CLASS( contingency_wave_spawnpoint, CContingencyWaveSpawner );
LINK_ENTITY_TO_CLASS( contingency_wave_spawner, CContingencyWaveSpawner );	// legacy support/backwards compatibility

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
	if ( ContingencyRules()->GetCurrentWaveNPCList() && (ContingencyRules()->GetCurrentWaveNPCList()->Count() >= ContingencyRules()->GetMapMaxLivingNPCs()) )
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
		if ( !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_FAST) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_BLACK) )
			return;	// this spawner cannot spawn any headcrabs

		// Choose a random type of headcrab to spawn
		NPCClassName = kWaveHeadcrabsNPCTypes[random->RandomInt(0, NUM_HEADCRAB_NPCS - 1)];
		while ( ((Q_strcmp(NPCClassName, "npc_headcrab") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB)) ||
				((Q_strcmp(NPCClassName, "npc_headcrab_fast") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_FAST)) ||
				((Q_strcmp(NPCClassName, "npc_headcrab_black") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_BLACK)) )
			NPCClassName = kWaveHeadcrabsNPCTypes[random->RandomInt(0, NUM_HEADCRAB_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_ANTLIONS )
	{
		if ( !HasSpawnFlags(SF_WAVESPAWNER_ANTLION) )
			return;	// this spawner cannot spawn any antlions

		// Choose a random type of antlion to spawn
		NPCClassName = kWaveAntlionsNPCTypes[random->RandomInt(0, NUM_ANTLION_NPCS - 1)];
		while ( ((Q_strcmp(NPCClassName, "npc_antlion") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_ANTLION)) )
			NPCClassName = kWaveAntlionsNPCTypes[random->RandomInt(0, NUM_ANTLION_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_ZOMBIES )
	{
		if ( !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_TORSO) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_FAST) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_POISON) )
			return;	// this spawner cannot spawn any zombies

		// Choose a random type of zombie to spawn
		NPCClassName = kWaveZombiesNPCTypes[random->RandomInt(0, NUM_ZOMBIE_NPCS - 1)];
		while ( ((Q_strcmp(NPCClassName, "npc_zombie") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE)) ||
				((Q_strcmp(NPCClassName, "npc_zombie_torso") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_TORSO)) ||
				((Q_strcmp(NPCClassName, "npc_fastzombie") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_FAST)) ||
				((Q_strcmp(NPCClassName, "npc_poisonzombie") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_POISON)) )
			NPCClassName = kWaveZombiesNPCTypes[random->RandomInt(0, NUM_ZOMBIE_NPCS - 1)];
	}
	else if ( currentWaveType == WAVE_COMBINE )
	{
		if ( !HasSpawnFlags(SF_WAVESPAWNER_COMBINE) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_METRO) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_SCANNER) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_MANHACK) &&
			 !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_STALKER) )
			return;	// this spawner cannot spawn any zombies

		// Choose a random type of zombie to spawn
		NPCClassName = kWaveCombineNPCTypes[random->RandomInt(0, NUM_COMBINE_NPCS - 1)];
		while ( ((Q_strcmp(NPCClassName, "npc_combine_s") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_COMBINE)) ||
				((Q_strcmp(NPCClassName, "npc_metropolice") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_METRO)) ||
				((Q_strcmp(NPCClassName, "npc_cscanner") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_SCANNER)) ||
				((Q_strcmp(NPCClassName, "npc_manhack") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_MANHACK)) ||
				((Q_strcmp(NPCClassName, "npc_stalker") == 0) && !HasSpawnFlags(SF_WAVESPAWNER_COMBINE_STALKER)) )
			NPCClassName = kWaveCombineNPCTypes[random->RandomInt(0, NUM_COMBINE_NPCS - 1)];

		// Certain combine types should be given equipment (weapons)
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
	if ( ContingencyRules()->GetCurrentWaveNPCList() && (ContingencyRules()->GetCurrentWaveNPCList()->Find(pent) == -1) )
	{
		ContingencyRules()->GetCurrentWaveNPCList()->AddToTail( pent );
		ContingencyRules()->IncrementNumEnemiesSpawned();
	}
}
