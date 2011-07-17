#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "contingency_gameinterface.h"

// Change the maximum number of players
#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const
{
	// Change the maximum number of players
	minplayers = defaultMaxPlayers = 2; 
	maxplayers = ContingencyRules()->GetMaxNumPlayers();
}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
}
