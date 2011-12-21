// Contingency - James
// Added a modified version of Valve's floor turret
// Implement client-side turret NPC class
// Not much is commented here, so...

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "c_npc_contingency_turret.h"

#include "c_contingency_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_NPC_FloorTurret, DT_NPC_FloorTurret, CNPC_FloorTurret )
END_RECV_TABLE()

C_NPC_FloorTurret::C_NPC_FloorTurret()
{
}

C_NPC_FloorTurret::~C_NPC_FloorTurret()
{
}

// Added turret status HUD element
const char* C_NPC_FloorTurret::GetOwnerDisplay( void )
{
	C_Contingency_Player *pPlayer = ToContingencyPlayer( GetOwnerEntity() );
	if ( !pPlayer )
		return "Orphaned Turret";

	if ( pPlayer == C_Contingency_Player::GetLocalContingencyPlayer() )
		return "Your Turret";

	char szOwnerDisplay[256];
	Q_snprintf( szOwnerDisplay, sizeof(szOwnerDisplay), "%s's Turret", pPlayer->GetPlayerName() );
	return szOwnerDisplay;
}

// Added turret status HUD element
const char* C_NPC_FloorTurret::GetHealthCondition( void )
{
	if ( m_iHealth <= 0 )
		return "DESTROYED";

	float ratio = ((float)m_iHealth) / ((float)m_iMaxHealth);
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return "Fully Operational";
		else if ( ratio >= 0.50 )
			return "Damaged";
		else if ( ratio >= 0.25 )
			return "Severely Damaged";
		else if ( ratio < 0.25 )
			return "Critically Damaged";
	}

	return "Condition Unknown";
}

// Added turret status HUD element
Color C_NPC_FloorTurret::GetHealthConditionColor( void )
{
	if ( m_iHealth <= 0 )
		return Color( 204, 0, 0, 255 );	// dark(er) red

	float ratio = ((float)m_iHealth) / ((float)m_iMaxHealth);
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return Color( 0, 255, 0, 255 );	// green
		else if ( ratio >= 0.50 )
			return Color( 255, 204, 0, 255 );	// yellow
		else if ( ratio >= 0.25 )
			return Color( 255, 153, 0, 255 );	// orange
		else if ( ratio < 0.25 )
			return Color( 255, 0, 0, 255 );	// red
	}

	return Color( 255, 255, 255, 255 );	// white
}

// Remember to update CNPC_FloorTurret::ShouldCollide!
bool C_NPC_FloorTurret::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( (collisionGroup == COLLISION_GROUP_PLAYER) ||
		 (collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT) )
		return false;	// prevent collisions with players (NOT their projectiles though!)

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}
