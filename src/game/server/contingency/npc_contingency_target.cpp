#include "cbase.h"

#include "npc_contingency_target.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CNPC_Contingency_Target )
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_contingency_target, CNPC_Contingency_Target );

CNPC_Contingency_Target::CNPC_Contingency_Target( void )
{
}

CNPC_Contingency_Target::~CNPC_Contingency_Target( void )
{
}

Class_T	CNPC_Contingency_Target::Classify( void )
{
	// This makes us susceptible to attack by enemies of players, which is exactly what we want!
	return CLASS_PLAYER_ALLY;
}

bool CNPC_Contingency_Target::CanBeSeenBy( CAI_BaseNPC *pNPC )
{
	return CAI_BaseNPC::CanBeSeenBy( pNPC );
}

bool CNPC_Contingency_Target::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
	return CAI_BaseNPC::CanBeAnEnemyOf( pEnemy );
}
