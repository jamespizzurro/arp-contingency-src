#ifndef CONTINGENCY_GAMERULES_H
#define CONTINGENCY_GAMERULES_H
#pragma once

#include "hl2mp_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "gamevars_shared.h"

#include "contingency_weapon_types.h"

#ifndef CLIENT_DLL
	#include "contingency_player.h"
#else
	#include "c_contingency_player.h"
#endif

#ifdef CLIENT_DLL
	#define CContingency_Player C_Contingency_Player
	#define CContingencyRules C_ContingencyRules
	#define CContingencyRulesProxy C_ContingencyRulesProxy
#endif

#ifndef CLIENT_DLL
	// Added phase system
	extern ConVar contingency_phase_interimtime;
#endif

enum CONTINGENCY_TEAMS
{
	TEAM_PLAYER,

	NUM_TEAMS
};

// Added phase system
enum CONTINGENCY_PHASE
{
	PHASE_WAITING_FOR_PLAYERS,
	PHASE_INTERIM,
	PHASE_COMBAT,

	NUM_PHASES
};

// Added wave system
enum CONTINGENCY_WAVE
{
	WAVE_NONE,
	WAVE_HEADCRABS,
	WAVE_ANTLIONS,
	WAVE_ZOMBIES,
	WAVE_COMBINE,

	NUM_WAVES	// actually represents 1 more than the number of waves
				// if you don't include WAVE_NONE, but oh well...
				// there are a number of reasons why WAVE_NONE isn't -1, trust me!
};

// Added wave system
static const int NUM_HEADCRAB_NPCS = 3;
static const char* kWaveHeadcrabsNPCTypes[NUM_HEADCRAB_NPCS] =
{
	"npc_headcrab",
	"npc_headcrab_fast",
	"npc_headcrab_black"
};
static const int NUM_ANTLION_NPCS = 1;
static const char* kWaveAntlionsNPCTypes[NUM_ANTLION_NPCS] =
{
	"npc_antlion"
};
static const int NUM_ZOMBIE_NPCS = 4;
static const char* kWaveZombiesNPCTypes[NUM_ZOMBIE_NPCS] =
{
	"npc_zombie",
	"npc_zombie_torso",
	"npc_fastzombie",
	"npc_poisonzombie"
};
static const int NUM_COMBINE_NPCS = 5;
static const char* kWaveCombineNPCTypes[NUM_COMBINE_NPCS] =
{
	"npc_combine_s",
	"npc_metropolice",
	"npc_cscanner",
	"npc_manhack",
	"npc_stalker"
};
static const int NUM_COMBINE_S_WEAPONS = 3;
static const char* kWaveCombineSWeaponTypes[NUM_COMBINE_S_WEAPONS] =
{
	"weapon_shotgun",
	"weapon_smg1",
	"weapon_ar2"
};
static const int NUM_METROPOLICE_WEAPONS = 2;
static const char* kWaveMetropoliceWeaponTypes[NUM_METROPOLICE_WEAPONS] =
{
	"weapon_pistol",
	"weapon_smg1"
};
static const int NUM_SUPPORT_NPCS = 1;
static const char* kSupportWaveSupportNPCTypes[NUM_SUPPORT_NPCS] =
{
	"npc_citizen"
};
static const int NUM_CITIZEN_WEAPONS = 3;
static const char* kSupportWaveCitizenWeaponTypes[NUM_CITIZEN_WEAPONS] =
{
	"weapon_shotgun",
	"weapon_smg1",
	"weapon_ar2"
};

// Added sound cue and background music system
static const int NUM_BACKGROUND_MUSIC = 9;
static const char* kBackgroundMusic[NUM_BACKGROUND_MUSIC] =
{
	"music/HL1_song10_loop.wav",
	"music/HL1_song15_loop.wav",
	"music/HL2_song3_loop.wav",
	"music/HL2_song14_loop.wav",
	"music/HL2_song16_loop.wav",
	"music/HL2_song20_submix0_loop.wav",
	"music/HL2_song20_submix4_loop.wav",
	"music/HL2_song29_loop.wav",
	"music/HL2_song31_loop.wav"
};

class CContingencyRulesProxy : public CHL2MPGameRulesProxy
{
public:
	DECLARE_CLASS( CContingencyRulesProxy, CHL2MPGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CContingencyRules : public CHL2MPRules
{
public:
	DECLARE_CLASS( CContingencyRules, CHL2MPRules );

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.
#else
	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif

	CContingencyRules();
	~CContingencyRules();

public:	// helper functions

	bool IsPlayerPlaying( CContingency_Player *pPlayer = NULL )
	{
		if ( !pPlayer )
		{
			// Null pointers? Eww...

			return false;
		}

		if ( !pPlayer->IsAlive() || pPlayer->IsObserver() )
		{
			// Dead players aren't considered playing because
			// they're more like spectators

			return false;
		}

		// All players are forced to TEAM_PLAYER when they spawn
		// and they should not be able to leave that team,
		// so we shouldn't need to check for that here
		// (originally we did, hence this comment)

		return true;
	}

	// Do not allow players to respawn when they shouldn't be able to do so
	bool CanPlayersRespawn( void )
	{
		// Added phase system
		if ( GetCurrentPhase() == PHASE_COMBAT )
			return false;

		return true;
	}

#ifndef CLIENT_DLL
	int GetTotalNumPlayers( void )
	{
		int iNumPlayers = 0;

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *pClient = UTIL_PlayerByIndex( i );

			if ( !pClient || !pClient->edict() )
				continue;

			if ( !pClient->IsNetClient() )
				continue;

			iNumPlayers++;
		}

		return iNumPlayers;
	}
#endif

#ifndef CLIENT_DLL
	int GetNumPlayingPlayers( void )
	{
		int iNumPlayers = 0;

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CContingency_Player *pClient = ToContingencyPlayer( UTIL_PlayerByIndex(i) );

			if ( !pClient || !pClient->edict() )
				continue;

			if ( !pClient->IsNetClient() )
				continue;

			if ( !IsPlayerPlaying(pClient) )
				continue;

			iNumPlayers++;
		}

		return iNumPlayers;
	}
#endif

#ifndef CLIENT_DLL
	// Added loadout system
	void UpdatePlayerLoadouts( void )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer )
				continue;

			if ( !IsPlayerPlaying(pPlayer) )
				continue;	// dead players' loadouts will be updated when they spawn

			pPlayer->ApplyLoadout( pPlayer->GetHealth() );
		}
	}
#endif

#ifndef CLIENT_DLL
	void RespawnDeadPlayers( void )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer )
				continue;

			if ( IsPlayerPlaying(pPlayer) )
				continue;

			pPlayer->Spawn();
		}
	}
#endif

public:

	const char *GetGameDescription( void ) { return "Contingency"; }
	int GetMaxNumPlayers( void ) { return 5; }	// defines the maximum number of players allowed on a server

#ifndef CLIENT_DLL
	// Added announcements system
	// This function allows the server to display a certain block of text at the center of a particular player's or all players' HUDs
	void DisplayAnnouncement( const char* announcementText, float timeOnScreen = 8.0f, bool shouldFade = true, CBasePlayer *pTargetPlayer = NULL );
#endif

#ifndef CLIENT_DLL
	void ResetPhaseVariables( void );
#endif

	void CreateStandardEntities( void );

	// Added loadout system
	// Prevent players from being able to change their player models
	void ClientSettingsChanged( CBasePlayer *pPlayer );

	void Precache( void );

	// Added wave system
	// Precache all NPC models that are to be used here to ensure
	// players don't experience any annoying loading delays later
	void PrecacheWaveNPCs( void );

	void ClientDisconnected( edict_t *pClient );

	// Added wave system
#ifndef CLIENT_DLL
	void PerformWaveCalculations();
	void HandleNPCDeath( CAI_BaseNPC *pNPC, const CTakeDamageInfo &info );
#endif

	void Think( void );

#ifndef CLIENT_DLL
	void RemoveSatchelsAndTripmines( CContingency_Player *pPlayer = NULL );
#endif

	void CheckRestartGame( void );
	void RestartGame( void );

	// Do not allow players to hurt each other
	int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

#ifndef CLIENT_DLL
	// Move AI relationship tables to CContingencyRules
	void InitDefaultAIRelationships( void );
#endif

	// Added phase system
	int GetCurrentPhase( void ) { return m_iCurrentPhase; }
#ifndef CLIENT_DLL
	void SetCurrentPhase( int newPhase )
	{
		m_iCurrentPhase = newPhase;

		// When switching to our dummy phase, all we want to do
		// is reset everything, which should be done before
		// this function is called, so do nothing more here
		if ( m_iCurrentPhase == PHASE_WAITING_FOR_PLAYERS )
			return;

		// Phase changes warrant respawning dead players
		// to make sure they aren't left behind during
		// important game logic
		RespawnDeadPlayers();

		if ( m_iCurrentPhase == PHASE_INTERIM )
		{
			// Post-PHASE_COMBAT cleanup...
			PurgeCurrentWave();
			SetWaveType( WAVE_NONE );
			SetNumEnemiesRemaining( 0 );
			SetCalculatedNumEnemies( 0 );
			m_bPlayersDefeated = false;

			RemoveSatchelsAndTripmines();	// more cleaning (all players' satchels and tripmines)

			// Added loadout system
			UpdatePlayerLoadouts();

			SetInterimPhaseTimeLeft( contingency_phase_interimtime.GetInt() );

			// Added sound cue and background music system
			engine->ServerCommand( "stopplayingbackgroundmusic\n" );
		}
		else if ( m_iCurrentPhase == PHASE_COMBAT )
		{
			// Post-PHASE_INTERIM cleanup...
			SetInterimPhaseTimeLeft( 0 );
			m_flInterimPhaseTime = 0.0f;

			// Added sound cue and background music system
			engine->ServerCommand( "playbackgroundmusic\n" );
		}
	}
#endif
	const char *GetCurrentPhaseName( void )
	{
		switch ( m_iCurrentPhase )
		{
		case PHASE_WAITING_FOR_PLAYERS:
			return "Waiting for players...";
		case PHASE_INTERIM:
			return "INTERIM PHASE";
		case PHASE_COMBAT:
			return "COMBAT PHASE";
		}

		return "";
	}

	// Added phase system
	int GetInterimPhaseTimeLeft( void ) { return m_iInterimPhaseTimeLeft; }
#ifndef CLIENT_DLL
	void SetInterimPhaseTimeLeft( int newTime )
	{
		m_iInterimPhaseTimeLeft = newTime;
		m_flInterimPhaseTime = gpGlobals->curtime + m_iInterimPhaseTimeLeft;
	}
#endif

	// Added wave system
	int GetWaveNumber( void ) { return m_iWaveNum; }
#ifndef CLIENT_DLL
	void SetWaveNumber( int newWaveNum ) { m_iWaveNum = newWaveNum; }
	void IncrementWaveNumber( void ) { m_iWaveNum = m_iWaveNum + 1; }
#endif
	int GetWaveType( void ) { return m_iWaveType; }
#ifndef CLIENT_DLL
	void SetWaveType( int newWaveType ) { m_iWaveType = newWaveType; }
#endif
	int GetNumEnemiesRemaining( void ) { return m_iNumEnemiesRemaining; }
#ifndef CLIENT_DLL
	void SetNumEnemiesRemaining( int newNumEnemiesRemaining ) { m_iNumEnemiesRemaining = newNumEnemiesRemaining; }
	void DecrementNumEnemiesRemaining( void ) { m_iNumEnemiesRemaining = m_iNumEnemiesRemaining - 1; }
#endif
#ifndef CLIENT_DLL
	int GetCalculatedNumEnemies( void ) { return m_iCalculatedNumEnemies; }
	void SetCalculatedNumEnemies( int newCalculatedNumEnemies ) { m_iCalculatedNumEnemies = newCalculatedNumEnemies; }
#endif
#ifndef CLIENT_DLL
	int GetNumEnemiesSpawned( void ) { return m_iNumEnemiesSpawned; }
	void SetNumEnemiesSpawned( int newNumEnemiesSpawned ) { m_iNumEnemiesSpawned = newNumEnemiesSpawned; }
	void IncrementNumEnemiesSpawned( void ) { m_iNumEnemiesSpawned = m_iNumEnemiesSpawned + 1; }
#endif
#ifndef CLIENT_DLL
	int GetNumNPCsInCurrentWave( void )
	{
		return m_CurrentWaveNPCList.Count();
	}
	void AddNPCToCurrentWave( CAI_BaseNPC *pNPC )
	{
		if ( m_CurrentWaveNPCList.Find(pNPC) == -1 )
			m_CurrentWaveNPCList.AddToTail( pNPC );
	}
	void RemoveNPCFromCurrentWave( CAI_BaseNPC *pNPC )
	{
		m_CurrentWaveNPCList.FindAndRemove( pNPC );
	}
	bool IsNPCInCurrentWave( CAI_BaseNPC *pNPC )
	{
		return (m_CurrentWaveNPCList.Find(pNPC) != -1) ? true : false;
	}
	void PurgeCurrentWave( void )
	{
		m_CurrentWaveNPCList.Purge();
	}
#endif
#ifndef CLIENT_DLL
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	bool DoesMapSupportHeadcrabs( void ) { return m_bMapHeadcrabSupport; }
	void DoesMapSupportHeadcrabs( bool boolean ) { m_bMapHeadcrabSupport = boolean; }
	bool DoesMapSupportAntlions( void ) { return m_bMapAntlionSupport; }
	void DoesMapSupportAntlions( bool boolean ) { m_bMapAntlionSupport = boolean; }
	bool DoesMapSupportZombies( void ) { return m_bMapZombieSupport; }
	void DoesMapSupportZombies( bool boolean ) { m_bMapZombieSupport = boolean; }
	bool DoesMapSupportCombine( void ) { return m_bMapCombineSupport; }
	void DoesMapSupportCombine( bool boolean ) { m_bMapCombineSupport = boolean; }

	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	int GetMapHeadcrabWaveMultiplierOffset( void ) { return m_flMapHeadcrabWaveMultiplierOffset; }
	void SetMapHeadcrabWaveMultiplierOffset( int newMapHeadcrabWaveMultiplierOffset ) { m_flMapHeadcrabWaveMultiplierOffset = newMapHeadcrabWaveMultiplierOffset; }
	int GetMapAntlionWaveMultiplierOffset( void ) { return m_flMapAntlionWaveMultiplierOffset; }
	void SetMapAntlionWaveMultiplierOffset( int newMapAntlionWaveMultiplierOffset ) { m_flMapAntlionWaveMultiplierOffset = newMapAntlionWaveMultiplierOffset; }
	int GetMapZombieWaveMultiplierOffset( void ) { return m_flMapZombieWaveMultiplierOffset; }
	void SetMapZombieWaveMultiplierOffset( int newMapZombieWaveMultiplierOffset ) { m_flMapZombieWaveMultiplierOffset = newMapZombieWaveMultiplierOffset; }
	int GetMapCombineWaveMultiplierOffset( void ) { return m_flMapCombineWaveMultiplierOffset; }
	void SetMapCombineWaveMultiplierOffset( int newMapCombineWaveMultiplierOffset ) { m_flMapCombineWaveMultiplierOffset = newMapCombineWaveMultiplierOffset; }
#endif

// Added support wave system
#ifndef CLIENT_DLL
	int GetNumNPCsInCurrentSupportWave( void )
	{
		return m_CurrentSupportWaveNPCList.Count();
	}
	void AddNPCToCurrentSupportWave( CAI_BaseNPC *pNPC )
	{
		if ( m_CurrentSupportWaveNPCList.Find(pNPC) == -1 )
			m_CurrentSupportWaveNPCList.AddToTail( pNPC );
	}
	void RemoveNPCFromCurrentSupportWave( CAI_BaseNPC *pNPC )
	{
		m_CurrentSupportWaveNPCList.FindAndRemove( pNPC );
	}
	bool IsNPCInCurrentSupportWave( CAI_BaseNPC *pNPC )
	{
		return (m_CurrentSupportWaveNPCList.Find(pNPC) != -1) ? true : false;
	}
	void PurgeCurrentSupportWave( void )
	{
		m_CurrentSupportWaveNPCList.Purge();
	}
#endif

	// Added sound cue and background music system
#ifndef CLIENT_DLL
	void PlayAnnouncementSound( const char *soundName, CBasePlayer *pTargetPlayer = NULL );
#else
	void PlayBackgroundMusic( void );
	void StopPlayingBackgroundMusic( void );
#endif

	// Added a non-restorative health system
#ifndef CLIENT_DLL
	CContingency_Player_Info *FindPlayerInfoWithSteamID( const char *steamID )
	{
		// Search through our list of player infos looking for one that
		// corresponds with the specified SteamID
		for ( int i = 0; i < m_PlayerInfoList.Count(); i++ )
		{
			CContingency_Player_Info *pPlayerInfo = dynamic_cast<CContingency_Player_Info*>( m_PlayerInfoList[i] );
			if ( !pPlayerInfo )
				continue;	// this entry isn't valid, so move onto the next one in the list

			if ( pPlayerInfo->GetSteamID() == steamID )
				return m_PlayerInfoList[i];	// we've found something, so stop searching and return it right away!
		}

		return NULL;	// our search returned no results!
	}
#endif

	// Enable fall damage by default
	// Prevent players' screens from fading to black upon fall deaths
	bool FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) { return false; }

private:

	// Added phase system
	CNetworkVar( int, m_iCurrentPhase );
	CNetworkVar( int, m_iInterimPhaseTimeLeft );
#ifndef CLIENT_DLL
	float m_flInterimPhaseTime;
#endif

	// Added wave system
	CNetworkVar( int, m_iWaveNum );
	CNetworkVar( int, m_iWaveType );
	CNetworkVar( int, m_iNumEnemiesRemaining );
#ifndef CLIENT_DLL
	int m_iCalculatedNumEnemies;
	int m_iNumEnemiesSpawned;
	bool m_bPlayersDefeated;
	CUtlVector<CAI_BaseNPC*> m_CurrentWaveNPCList;

	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	bool m_bMapHeadcrabSupport;
	bool m_bMapAntlionSupport;
	bool m_bMapZombieSupport;
	bool m_bMapCombineSupport;

	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	float m_flMapHeadcrabWaveMultiplierOffset;
	float m_flMapAntlionWaveMultiplierOffset;
	float m_flMapZombieWaveMultiplierOffset;
	float m_flMapCombineWaveMultiplierOffset;
#endif

	int m_iRestartDelay;

	// Added support wave system
#ifndef CLIENT_DLL
	CUtlVector<CAI_BaseNPC*> m_CurrentSupportWaveNPCList;
#endif

	// Added a non-restorative health system
#ifndef CLIENT_DLL
	CUtlVector<CContingency_Player_Info*> m_PlayerInfoList;
#endif
};

inline CContingencyRules* ContingencyRules()
{
	return static_cast<CContingencyRules*>(g_pGameRules);
}

#endif // CONTINGENCY_GAMERULES_H
