#include "cbase.h"
#include "contingency_player.h"
#include "contingency_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "entitylist.h"
#include "physics.h"
#include "game.h"
#include "player_resource.h"
#include "engine/IEngineSound.h"
#include "team.h"
#include "viewport_panel_names.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Host_Say( edict_t *pEdict, bool teamonly );

extern CBaseEntity*	FindPickerEntityClass( CBasePlayer *pPlayer, char *classname );
extern bool	g_fGameOver;

void FinishClientPutInServer( CContingency_Player *pPlayer )
{
	// Added phase system
	// Now that we have a player on the server, set the phase to interim and begin the game
	if ( ContingencyRules()->GetCurrentPhase() == PHASE_WAITING_FOR_PLAYERS )
		ContingencyRules()->RestartGame();

	pPlayer->InitialSpawn();
	pPlayer->Spawn();

	char sName[128];
	Q_strncpy( sName, pPlayer->GetPlayerName(), sizeof( sName ) );
	
	// First parse the name and remove any %'s
	for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
	{
		// Replace it with a space
		if ( *pApersand == '%' )
				*pApersand = ' ';
	}

	// notify other clients of player joining the game
	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "#Game_connected", sName[0] != 0 ? sName : "<unconnected>" );

	const ConVar *hostname = cvar->FindVar( "hostname" );
	const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

	KeyValues *data = new KeyValues("data");
	data->SetString( "title", title );		// info panel title
	data->SetString( "type", "1" );			// show userdata from stringtable entry
	data->SetString( "msg",	"motd" );		// use this stringtable entry

	pPlayer->ShowViewPortPanel( PANEL_INFO, true, data );

	data->deleteThis();
}

/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer( edict_t *pEdict, const char *playername )
{
	// Allocate a CBaseTFPlayer for pev, and call spawn
	CContingency_Player *pPlayer = CContingency_Player::CreatePlayer( "player", pEdict );
	pPlayer->SetPlayerName( playername );
}

void ClientActive( edict_t *pEdict, bool bLoadGame )
{
	// Can't load games in CS!
	Assert( !bLoadGame );

	CContingency_Player *pPlayer = ToContingencyPlayer( CBaseEntity::Instance( pEdict ) );
	FinishClientPutInServer( pPlayer );
}

/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Contingency";
}

//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that 
//			classname that the player is nearest facing
//			
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity* FindEntity( edict_t *pEdict, char *classname )
{
	// If no name was given set bits based on the picked
	if ( FStrEq(classname, "") ) 
	{
		return (FindPickerEntityClass(static_cast<CBasePlayer*>(GetContainingEntity(pEdict)), classname) );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void ClientGamePrecache( void )
{
	CBaseEntity::PrecacheModel( "models/player.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/agibs.mdl" );
	CBaseEntity::PrecacheModel( "models/weapons/v_hands.mdl" );

	CBaseEntity::PrecacheScriptSound( "HUDQuickInfo.LowAmmo" );
	CBaseEntity::PrecacheScriptSound( "HUDQuickInfo.LowHealth" );

	CBaseEntity::PrecacheScriptSound( "FX_AntlionImpact.ShellImpact" );
	CBaseEntity::PrecacheScriptSound( "Missile.ShotDown" );
	CBaseEntity::PrecacheScriptSound( "Bullets.DefaultNearmiss" );
	CBaseEntity::PrecacheScriptSound( "Bullets.GunshipNearmiss" );
	CBaseEntity::PrecacheScriptSound( "Bullets.StriderNearmiss" );
	
	CBaseEntity::PrecacheScriptSound( "Geiger.BeepHigh" );
	CBaseEntity::PrecacheScriptSound( "Geiger.BeepLow" );
}

// called by ClientKill and DeadThink
void respawn( CBaseEntity *pEdict, bool fCopyCorpse )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( pEdict );

	if ( pPlayer )
	{
		if ( gpGlobals->curtime > pPlayer->GetDeathTime() + DEATH_ANIMATION_TIME )
		{		
			// respawn player
			pPlayer->Spawn();			
		}
		else
		{
			pPlayer->SetNextThink( gpGlobals->curtime + 0.1f );
		}
	}
}

void GameStartFrame( void )
{
	VPROF("GameStartFrame()");
	if ( g_fGameOver )
		return;

	// Teamplay should always be disabled
	//gpGlobals->teamplay = (teamplay.GetInt() != 0);
	gpGlobals->teamplay = false;

// Enable bots for testing and debugging purposes
//#ifdef DEBUG
	extern void Bot_RunAll();
	Bot_RunAll();
//#endif
}

void InstallGameRules()
{
	CreateGameRulesObject( "CContingencyRules" );
}
