#ifndef CONTINGENCY_GAMERULES_H
#define CONTINGENCY_GAMERULES_H
#pragma once

#include "hl2mp_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "gamevars_shared.h"

// Added phase system
#include "contingency_system_phase.h"

// Added wave system
#include "contingency_system_wave.h"

// Added loadout system
#include "contingency_system_loadout.h"

// Added sound cue and background music system
#include "contingency_system_music.h"

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

// Added wave system
extern ConVar contingency_wave_challenge_frequency;

enum CONTINGENCY_TEAMS
{
	TEAM_PLAYER = 0,

	NUM_TEAMS
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

public:

	//////////////////////
	// HELPER FUNCTIONS //
	//////////////////////
	// (everything in this particular "public:" block)

	// These are functions that can function (no pun intended?)
	// more or less independently and be called anytime without much fuss

	const char *GetGameDescription( void ) { return "Contingency"; }
	int GetMaxNumPlayers( void ) { return 5; }	// defines the maximum number of players allowed on a server

	// Do not allow players to hurt each other
	int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

	bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	bool IsPlayerPlaying( CContingency_Player *pPlayer = NULL );

#ifndef CLIENT_DLL
	int GetTotalNumPlayers( void );
	int GetNumPlayingPlayers( void );

	// Added loadout system
	void UpdatePlayerLoadouts( void );

	void HealPlayers( void );

	void RespawnDeadPlayers( void );

	// Added announcements system
	// This function allows the server to display a certain block of text at the center of a particular player's or all players' HUDs
	void DisplayAnnouncement( const char* announcementText, float timeOnScreen = 8.0f, bool shouldFade = true, CBasePlayer *pTargetPlayer = NULL );

	void RemoveSatchelsAndTripmines( CContingency_Player *pPlayer = NULL );

	// Added a non-restorative health system
	CContingency_Player_Info *FindPlayerInfoBySteamID( const char *steamID );
#endif

public:

#ifndef CLIENT_DLL
	void ResetPhaseVariables( void );
#endif

	void CreateStandardEntities( void );

	bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );

	// Added loadout system
	// Prevent players from being able to change their player models
	void ClientSettingsChanged( CBasePlayer *pPlayer );

	void Precache( void );

#ifndef CLIENT_DLL
	// Precaching called by each player (server-side)
	// This is in gamerules because all the precaching done here is really gameplay-dependent
	void PrecacheStuff( void );
#endif

	void ClientDisconnected( edict_t *pClient );

	// Added wave system
#ifndef CLIENT_DLL
	void PerformWaveCalculations();
	void HandleNPCDeath( CAI_BaseNPC *pNPC, const CTakeDamageInfo &info );
	void HandleNPCRemoval( CAI_BaseNPC *pNPC );
#endif

	void Think( void );

	void CheckRestartGame( void );
	void RestartGame( void );

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
		{
			Warning( "There are currently no players on the server. Standby mode has been activated and will remain active until at least one player joins the server.\n" );
			return;
		}

		if ( m_iCurrentPhase == PHASE_INTERIM )
		{
			// Post-PHASE_COMBAT cleanup...
			SetWaveType( WAVE_NONE );
			SetNumEnemiesRemaining( 0 );
			SetCalculatedNumEnemies( 0 );
			m_bPlayersDefeated = false;

			//RemoveSatchelsAndTripmines();	// more cleaning (all players' satchels and tripmines)

			SetInterimPhaseTimeLeft( GetMapInterimPhaseLength() );

			// Added loadout system
			UpdatePlayerLoadouts();

			HealPlayers();	// heal all living players
			RespawnDeadPlayers();	// respawn dead ones
		}
		else if ( m_iCurrentPhase == PHASE_COMBAT )
		{
			// Post-PHASE_INTERIM cleanup...
			SetInterimPhaseTimeLeft( 0 );
			m_flInterimPhaseTime = 0.0f;

			// Added loadout system
			UpdatePlayerLoadouts();

			HealPlayers();	// heal all living players
			RespawnDeadPlayers();	// respawn dead ones
		}
	}
#endif
	const char *GetCurrentPhaseName( void )
	{
		switch ( m_iCurrentPhase )
		{
		case PHASE_WAITING_FOR_PLAYERS:
			return "WAITING FOR PLAYERS";
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
	int GetPreferredWaveType( void ) { return m_iPreferredWaveType; }
#ifndef CLIENT_DLL
	void SetWaveType( int newWaveType ) { m_iWaveType = newWaveType; }
	void SetPreferredWaveType( int newPreferredWaveType ) { m_iPreferredWaveType = newPreferredWaveType; }
#endif
	bool IsChallengeWave( void ) {
		if ( contingency_wave_challenge_frequency.GetInt() <= 0 )
			return false;	// avoid math headaches

		return ((GetWaveNumber() % contingency_wave_challenge_frequency.GetInt()) == 0);
	}
#ifndef CLIENT_DLL
	const char *GetPreferredNPCType( void ) { return m_PreferredNPCType; }
	void SetPreferredNPCType( const char *newPreferredNPCType ) { m_PreferredNPCType = newPreferredNPCType; }
#endif
	int GetNumEnemiesRemaining( void ) { return m_iNumEnemiesRemaining; }
#ifndef CLIENT_DLL
	void SetNumEnemiesRemaining( int newNumEnemiesRemaining ) { m_iNumEnemiesRemaining = newNumEnemiesRemaining; }
	void IncrementNumEnemiesRemaining( void ) { m_iNumEnemiesRemaining = m_iNumEnemiesRemaining + 1; }
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
	CUtlVector<CAI_BaseNPC*> *GetCurrentWaveNPCList( void ) { return m_pCurrentWaveNPCList; }
	void SetCurrentWaveNPCList( CUtlVector<CAI_BaseNPC*> *pNewCurrentWaveNPCList ) { m_pCurrentWaveNPCList = pNewCurrentWaveNPCList; }
#endif

	// Added radar display
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	bool DoesMapAllowRadars( void ) { return m_bMapAllowsRadars; }
#ifndef CLIENT_DLL
	void DoesMapAllowRadars( bool boolean ) { m_bMapAllowsRadars = boolean; }
#endif

	// Added spawnable prop system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
#ifndef CLIENT_DLL
	void SetMapMaxPropsPerPlayer( int iNewMapMaxPropsPerPlayer ) { m_iMapMaxPropsPerPlayer = iNewMapMaxPropsPerPlayer; }
#endif
	int GetMapMaxPropsPerPlayer( void ) { return m_iMapMaxPropsPerPlayer; }

#ifndef CLIENT_DLL
	// Added phase system
	int GetMapInterimPhaseLength( void ) { return m_iInterimPhaseLength; }
	void SetMapInterimPhaseLength( int iNewInterimPhaseLength ) { m_iInterimPhaseLength = iNewInterimPhaseLength; }

	// Added credits system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	int GetMapStartingCredits( void ) { return m_iMapStartingCredits; }
	void SetMapStartingCredits( int iNewMapStartingCredits ) { m_iMapStartingCredits = iNewMapStartingCredits; }

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	int GetMapMaxLivingNPCs( void ) { return m_iMapMaxLivingNPCs; }
	void SetMapMaxLivingNPCs( int iNewMapMaxLivingNPCs ) { m_iMapMaxLivingNPCs = iNewMapMaxLivingNPCs; }
	/*bool DoesMapSupportHeadcrabs( void ) { return m_bMapHeadcrabSupport; }
	void DoesMapSupportHeadcrabs( bool boolean ) { m_bMapHeadcrabSupport = boolean; }*/
	bool DoesMapSupportAntlions( void ) { return m_bMapAntlionSupport; }
	void DoesMapSupportAntlions( bool boolean ) { m_bMapAntlionSupport = boolean; }
	bool DoesMapSupportZombies( void ) { return m_bMapZombieSupport; }
	void DoesMapSupportZombies( bool boolean ) { m_bMapZombieSupport = boolean; }
	bool DoesMapSupportCombine( void ) { return m_bMapCombineSupport; }
	void DoesMapSupportCombine( bool boolean ) { m_bMapCombineSupport = boolean; }

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	/*float GetMapHeadcrabWaveMultiplierOffset( void ) { return m_flMapHeadcrabWaveMultiplierOffset; }
	void SetMapHeadcrabWaveMultiplierOffset( float newMapHeadcrabWaveMultiplierOffset ) { m_flMapHeadcrabWaveMultiplierOffset = newMapHeadcrabWaveMultiplierOffset; }*/
	float GetMapAntlionWaveMultiplierOffset( void ) { return m_flMapAntlionWaveMultiplierOffset; }
	void SetMapAntlionWaveMultiplierOffset( float newMapAntlionWaveMultiplierOffset ) { m_flMapAntlionWaveMultiplierOffset = newMapAntlionWaveMultiplierOffset; }
	float GetMapZombieWaveMultiplierOffset( void ) { return m_flMapZombieWaveMultiplierOffset; }
	void SetMapZombieWaveMultiplierOffset( float newMapZombieWaveMultiplierOffset ) { m_flMapZombieWaveMultiplierOffset = newMapZombieWaveMultiplierOffset; }
	float GetMapCombineWaveMultiplierOffset( void ) { return m_flMapCombineWaveMultiplierOffset; }
	void SetMapCombineWaveMultiplierOffset( float newMapCombineWaveMultiplierOffset ) { m_flMapCombineWaveMultiplierOffset = newMapCombineWaveMultiplierOffset; }
#endif

// Added support wave system
#ifndef CLIENT_DLL
	CUtlVector<CAI_BaseNPC*> *GetCurrentSupportWaveNPCList( void ) { return m_pCurrentSupportWaveNPCList; }
	void SetCurrentSupportWaveNPCList( CUtlVector<CAI_BaseNPC*> *pNewCurrentSupportWaveNPCList ) { m_pCurrentSupportWaveNPCList = pNewCurrentSupportWaveNPCList; }
#endif

	// Enable fall damage by default
	// Prevent players' screens from fading to black upon fall deaths
	bool FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) { return false; }

	// Added a non-restorative health system
#ifndef CLIENT_DLL
	CUtlVector<CContingency_Player_Info*> m_PlayerInfoList;
#endif

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
	CNetworkVar( int, m_iPreferredWaveType );
	CNetworkVar( int, m_iNumEnemiesRemaining );
#ifndef CLIENT_DLL
	const char *m_PreferredNPCType;
	int m_iCalculatedNumEnemies;
	int m_iNumEnemiesSpawned;
	bool m_bPlayersDefeated;
	CUtlVector<CAI_BaseNPC*> *m_pCurrentWaveNPCList;
#endif

	// Added radar display
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	CNetworkVar( bool, m_bMapAllowsRadars );

	// Added spawnable prop system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	CNetworkVar( int, m_iMapMaxPropsPerPlayer );

#ifndef CLIENT_DLL
	// Added phase system
	int m_iInterimPhaseLength;

	// Added credits system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	int m_iMapStartingCredits;

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	int m_iMapMaxLivingNPCs;
	bool m_bMapHeadcrabSupport;
	bool m_bMapAntlionSupport;
	bool m_bMapZombieSupport;
	bool m_bMapCombineSupport;

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	float m_flMapHeadcrabWaveMultiplierOffset;
	float m_flMapAntlionWaveMultiplierOffset;
	float m_flMapZombieWaveMultiplierOffset;
	float m_flMapCombineWaveMultiplierOffset;
#endif

	int m_iRestartDelay;

	// Added support wave system
#ifndef CLIENT_DLL
	CUtlVector<CAI_BaseNPC*> *m_pCurrentSupportWaveNPCList;
#endif
};

inline CContingencyRules* ContingencyRules()
{
	return static_cast<CContingencyRules*>(g_pGameRules);
}

#endif // CONTINGENCY_GAMERULES_H
