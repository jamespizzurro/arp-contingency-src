// Added boss system

#include "cbase.h"

#include "contingency_boss_spawnpoint.h"

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

LINK_ENTITY_TO_CLASS( contingency_boss_spawnpoint, CContingencyBossSpawner );
LINK_ENTITY_TO_CLASS( contingency_boss_spawner, CContingencyBossSpawner );	// legacy support/backwards compatibility

BEGIN_DATADESC( CContingencyBossSpawner )

	DEFINE_KEYFIELD( m_ChildTargetName,		FIELD_STRING,	"NPCTargetname" ),
	DEFINE_KEYFIELD( m_SquadName,			FIELD_STRING,	"NPCSquadName" ),
	DEFINE_KEYFIELD( m_strHintGroup,		FIELD_STRING,	"NPCHintGroup" ),

	DEFINE_KEYFIELD( m_BossNPCType, FIELD_STRING, "BossNPCType" ),
	DEFINE_KEYFIELD( m_iBossSpawnFrequency, FIELD_INTEGER, "BossSpawnFrequency" ),

	// Add a custom rally point entity for wave spawners
	DEFINE_KEYFIELD( rallyPointName, FIELD_STRING, "rallyPointName" ),

	DEFINE_KEYFIELD( m_flMaximumDistanceFromNearestPlayer, FIELD_FLOAT, "MaxPlayerDistance" ),

END_DATADESC()

CContingencyBossSpawner::CContingencyBossSpawner( void )
{
	m_bHasSpawnedBoss = false;
}

CContingencyBossSpawner::~CContingencyBossSpawner( void )
{
}

void CContingencyBossSpawner::Spawn( void )
{
	BaseClass::Spawn();

	// Add a custom rally point entity for wave spawners
	pRallyPoint = dynamic_cast<CContingencyRallyPoint*>( gEntList.FindEntityByName(NULL, rallyPointName, this) );
}

void CContingencyBossSpawner::MakeNPC( void )
{
	if ( !CanMakeNPC() )
		return;

	// Only spawn NPCs during waves (combat phases), hence our name!
	if ( ContingencyRules()->GetCurrentPhase() != PHASE_COMBAT )
	{
		m_bHasSpawnedBoss = false;	// reset for when another combat phase begins
		return;
	}
	
	// Do not spawn more than one boss per wave (combat phase)
	if ( m_bHasSpawnedBoss )
		return;

	// Do not spawn more NPCs than our server will allow to be living at any given time
	if ( ContingencyRules()->GetCurrentWaveNPCList() && (ContingencyRules()->GetCurrentWaveNPCList()->Count() >= ContingencyRules()->GetMapMaxLivingNPCs()) )
		return;

	// Do not spawn more NPCs than we're supposed to for this wave
	if ( ContingencyRules()->GetNumEnemiesSpawned() >= ContingencyRules()->GetCalculatedNumEnemies() )
		return;
	
	// Only spawn boss NPCs however frequently the mapper has defined (by wave number)
	if ( m_iBossSpawnFrequency <= 0 )
		m_iBossSpawnFrequency = 1;	// assume the mapper is an idiot
	if ( ContingencyRules()->GetWaveNumber() % m_iBossSpawnFrequency != 0 )
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
	const char *NPCClassName = STRING(m_BossNPCType);
	string_t equipmentName = NULL_STRING;
	if ( currentWaveType == WAVE_HEADCRABS )
	{
		// There are no headcrab wave bosses yet! :(
		return;
	}
	else if ( currentWaveType == WAVE_ANTLIONS )
	{
		// Only spawn bosses associated with an antlion wave
		if ( Q_strcmp(NPCClassName, "npc_antlionguard") != 0 )
			return;
	}
	else if ( currentWaveType == WAVE_ZOMBIES )
	{
		// There are no zombie wave bosses yet! :(
		return;
	}
	else if ( currentWaveType == WAVE_COMBINE )
	{
		// There are no Combine wave bosses yet! :(
		return;
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

	m_bHasSpawnedBoss = true;
}
