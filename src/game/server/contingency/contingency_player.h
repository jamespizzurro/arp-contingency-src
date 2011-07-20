#ifndef CONTINGENCY_PLAYER_H
#define CONTINGENCY_PLAYER_H
#pragma once

class CContingency_Player;

#include "hl2mp_player.h"
#include "contingency_player_shared.h"

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

private:
	bool accessed;
	const char *steamID;
	int health;
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

	void PickDefaultSpawnTeam( void );
	void ChangeTeam( int iTeam );

	// Added loadout system
	void ApplyLoadout( int requestedHealth );
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

	// Add a custom maximum health variable so that the client can get a player's maximum health
	int GetMaxHealth( void ) { return m_iHealthMax; }
	void SetHealthMax( int newMaxHealth ) { m_iHealthMax = newMaxHealth; }

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

private:

	// Health regeneration system
	float m_flHealthRegenDelay;

	// Add a custom maximum health variable so that the client can get a player's maximum health
	CNetworkVar( int, m_iHealthMax );

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
};

inline CContingency_Player *ToContingencyPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CContingency_Player*>( pEntity );
}

#endif // CONTINGENCY_PLAYER_H
