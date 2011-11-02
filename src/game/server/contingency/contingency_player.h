#ifndef CONTINGENCY_PLAYER_H
#define CONTINGENCY_PLAYER_H
#pragma once

class CContingency_Player;

#include "hl2mp_player.h"
#include "contingency_player_shared.h"

// Added a modified version of Valve's floor turret
#include "npc_contingency_turret.h"

// Added spawnable prop system
#include "contingency_spawnableprop.h"

// Added a non-restorative health system
// A class dedicated to storing a player's information based on their SteamID
// for recall purposes even after they disconnect from the server
class CContingency_Player_Info
{
public:
	bool HasBeenAccessed( void ) { return accessed; }
	void HasBeenAccessed( bool boolean ) { accessed = boolean; }

	const char *GetSteamID( void ) { return steamID; }
	void SetSteamID( const char *newSteamID ) { steamID = newSteamID; }

	int GetHealth( void ) { return health; }
	void SetHealth( int newHealth ) { health = newHealth; }

	int GetCredits( void ) { return credits; }
	void SetCredits( int newCredits ) { credits = newCredits; }

private:
	bool accessed;
	const char *steamID;
	int health;
	int credits;
};

class CContingency_Player : public CHL2MP_Player
{
public:
	DECLARE_CLASS( CContingency_Player, CHL2MP_Player );

	CContingency_Player();
	~CContingency_Player( void );

	static CContingency_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CContingency_Player::s_PlayerEdict = ed;
		return (CContingency_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();

public:

	int GetHealth() { return m_iHealth; }
	int GetMaxHealth() { return m_iMaxHealth; }

	void PickDefaultSpawnTeam( void );
	void ChangeTeam( int iTeam );

	// Added loadout system
	void ApplyLoadout( void );
	void ReplenishAmmo( bool suppressSound );

	// Reworked spawnpoint system
	CBaseEntity* EntSelectSpawnPoint( void );

	void Precache( void );
	void Spawn( void );

	// Revert to normal HL2 footsteps
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	// Adjust players' max speed based on different factors
	float CContingency_Player::GetMaxWalkingSpeed( void )
	{
		return CONTINGENCY_WALK_SPEED;
	}
	float CContingency_Player::GetMaxNormalSpeed( void )
	{
		return CONTINGENCY_NORM_SPEED;
	}
	float CContingency_Player::GetMaxSprintingSpeed( void )
	{
		return CONTINGENCY_SPRINT_SPEED;
	}
	void SetMaxSpeed( float flMaxSpeed );

	void PostThink( void );

	// Do not allow players to change their team
	bool HandleCommand_JoinTeam( int team );

	bool ClientCommand( const CCommand &args );

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	bool BumpWeapon( CBaseCombatWeapon *pWeapon );

	int OnTakeDamage( const CTakeDamageInfo &inputInfo );

	// Add pain sounds when a player takes damage
	void PainSound( const CTakeDamageInfo &info );

	void Event_Killed( const CTakeDamageInfo &info );

	// Change death sounds when a player dies
	void DeathSound( const CTakeDamageInfo &info );

	// Rework respawning system
	bool StartObserverMode( int mode );
	void PlayerDeathThink( void );

	// Added drop system
	void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity );

	// Added chat bubble above players' heads while they type in chat
	void MakeChatBubble( int chatbubble );
	void KillChatBubble();
	void CheckChatBubble( CUserCmd *cmd );

	// Added loadout system
	const char *GetPreferredPrimaryWeaponClassname( void ) { return engine->GetClientConVarValue( entindex(), "contingency_client_preferredprimaryweapon" ); }
	const char *GetPreferredSecondaryWeaponClassname( void ) { return engine->GetClientConVarValue( entindex(), "contingency_client_preferredsecondaryweapon" ); }
	const char *GetPreferredMeleeWeaponClassname( void ) { return engine->GetClientConVarValue( entindex(), "contingency_client_preferredmeleeweapon" ); }
	const char *GetPreferredEquipmentClassname( void ) { return engine->GetClientConVarValue( entindex(), "contingency_client_preferredequipment" ); }

	// Added loadout system
	bool IsMarkedForLoadoutUpdate( void ) { return m_bMarkedForLoadoutUpdate; }
	void IsMarkedForLoadoutUpdate( bool boolean ) { m_bMarkedForLoadoutUpdate = boolean; }

	// Added shout system
	bool IsShowingShoutMenu( void ) { return m_bShowShoutMenu; }
	void ShouldShowShoutMenu( bool boolean ) { m_bShowShoutMenu = boolean; }
	float GetShoutDelay( void ) { return m_flShoutDelay; }
	void SetShoutDelay( float newShoutDelay ) { m_flShoutDelay = newShoutDelay; }

	// Added phase system
	bool IsAllowedToSpawn( void ) { return m_bAllowedToSpawn; }
	void IsAllowedToSpawn( bool boolean ) { m_bAllowedToSpawn = boolean; }

	// Added a modified version of Valve's floor turret
	CNPC_FloorTurret *GetDeployedTurret( void ) { return m_hDeployedTurret; }
	void SetDeployedTurret( CNPC_FloorTurret *pNewDeployedTurret ) { m_hDeployedTurret = pNewDeployedTurret; }

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	bool IsGivingWeapons( void ) { return m_bGivingWeapons; }
	void IsGivingWeapons( bool boolean ) { m_bGivingWeapons = boolean; }

	// Added credits system
	int GetCredits( void ) { return m_iCredits; }
	void SetCredits( int iNewCredits ) { m_iCredits = iNewCredits; }
	bool HasCredits( int iCreditsToCheck )
	{
		if ( GetCredits() < iCreditsToCheck )
			return false;	// player does not have enough credits

		return true;
	}
	bool UseCredits( int iCreditsToUse )
	{
		if ( !HasCredits(iCreditsToUse) )
			return false;	// player does not have enough credits to complete the transaction

		SubtractCredits( iCreditsToUse );
		return true;
	}
	void AddCredits( int iCreditsToAdd ) { m_iCredits = m_iCredits + iCreditsToAdd; }
	void SubtractCredits( int iCreditsToSubtract ) { m_iCredits = m_iCredits - iCreditsToSubtract; }
	void ResetCredits( void ) { m_iCredits = 0; }

	// Added spawnable prop system
	CContingency_SpawnableProp *GetSpawnablePropInFocus( void ) { return m_pSpawnablePropInFocus; }
	void SetSpawnablePropInFocus( CContingency_SpawnableProp *pNewSpawnablePropInFocus ) { m_pSpawnablePropInFocus = pNewSpawnablePropInFocus; }
	CUtlVector<CContingency_SpawnableProp*> m_SpawnablePropList;
	int GetNumSpawnableProps( void ) { return m_iNumSpawnableProps; }
	void SetNumSpawnableProps( int iNewNumSpawnableProps ) { m_iNumSpawnableProps = iNewNumSpawnableProps; }

	// Added spawnable prop system
	int GetDesiredSpawnablePropIndex( void ) { return m_iDesiredSpawnablePropIndex; }
	void SetDesiredSpawnablePropIndex( int iNewDesiredSpawnablePropIndex ) { m_iDesiredSpawnablePropIndex = iNewDesiredSpawnablePropIndex; }

private:

	// Health regeneration system
	float m_flHealthRegenDelay;

	// Add pain sounds when a player takes damage
	float m_flMinTimeBtwnPainSounds;

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	bool m_bGivingWeapons;

	// Adjust players' max speed based on different factors
	float m_flSpeedCheckDelay;

	// Added chat bubble above players' heads while they type in chat
	EHANDLE m_hChatBubble;

	// Added loadout system
	bool m_bMarkedForLoadoutUpdate;

	// Added shout system
	bool m_bShowShoutMenu;
	float m_flShoutDelay;

	// Added phase system
	bool m_bAllowedToSpawn;

	// Added a modified version of Valve's floor turret
	CHandle<CNPC_FloorTurret> m_hDeployedTurret;

	// Added credits system
	CNetworkVar( int, m_iCredits );

	// Added spawnable prop system
	CContingency_SpawnableProp *m_pSpawnablePropInFocus;
	CNetworkVar( int, m_iNumSpawnableProps );	// should match m_pSpawnablePropInFocus.Count() at all times, we just needed to network it somehow

	// Added spawnable prop system
	CNetworkVar( int, m_iDesiredSpawnablePropIndex );
};

inline CContingency_Player *ToContingencyPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CContingency_Player*>( pEntity );
}

#endif // CONTINGENCY_PLAYER_H
