// Added spawnable prop system

#include "cbase.h"
#include "c_contingency_spawnableprop.h"

#include "c_contingency_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_Contingency_SpawnableProp, DT_Contingency_SpawnableProp, CContingency_SpawnableProp )
	RecvPropEHandle( RECVINFO(m_hSpawnerPlayer) ),
END_RECV_TABLE()

CUtlVector<C_Contingency_SpawnableProp*> m_SpawnablePropList = NULL;

C_Contingency_SpawnableProp::C_Contingency_SpawnableProp()
{
	if ( m_SpawnablePropList.Find(this) == -1 )
		m_SpawnablePropList.AddToTail( this );
}

C_Contingency_SpawnableProp::~C_Contingency_SpawnableProp()
{
	m_SpawnablePropList.FindAndRemove( this );
}

const char* C_Contingency_SpawnableProp::GetOwnerDisplay( void )
{
	C_Contingency_Player *pPlayer = ToContingencyPlayer( GetSpawnerPlayer() );
	if ( !pPlayer )
		return "Orphaned Prop";

	if ( pPlayer == C_Contingency_Player::GetLocalContingencyPlayer() )
		return "Your Prop";

	char szOwnerDisplay[256];
	Q_snprintf( szOwnerDisplay, sizeof(szOwnerDisplay), "%s's Prop", pPlayer->GetPlayerName() );
	return szOwnerDisplay;
}

const char* C_Contingency_SpawnableProp::GetHealthCondition( void )
{
	if ( m_iHealth <= 0 )
		return "DESTROYED";

	float ratio = ((float)m_iHealth) / ((float)m_iMaxHealth);
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return "Structurally Sound";
		else if ( ratio >= 0.50 )
			return "Damaged";
		else if ( ratio >= 0.25 )
			return "Severely Damaged";
		else if ( ratio < 0.25 )
			return "Critically Damaged";
	}

	return "Condition Unknown";
}

Color C_Contingency_SpawnableProp::GetHealthConditionColor( void )
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
