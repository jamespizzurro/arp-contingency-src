// Contingency - James

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "c_npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Citizen, DT_NPC_Citizen, CNPC_Citizen )
END_RECV_TABLE()

C_NPC_Citizen::C_NPC_Citizen()
{
}

C_NPC_Citizen::~C_NPC_Citizen()
{
}

// Added citizen status HUD element
const char* C_NPC_Citizen::GetHealthCondition( void )
{
	if ( m_iHealth <= 0 )
		return "DEAD";

	float ratio = ((float)m_iHealth) / ((float)m_iMaxHealth);
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return "Healthy";
		else if ( ratio >= 0.50 )
			return "Hurt";
		else if ( ratio >= 0.25 )
			return "Wounded";
		else if ( ratio < 0.25 )
			return "Near Death";
	}

	return "Condition Unknown";
}

// Added citizen status HUD element
Color C_NPC_Citizen::GetHealthConditionColor( void )
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
