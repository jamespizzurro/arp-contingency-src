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
		return "Prop";

	if ( pPlayer == C_Contingency_Player::GetLocalContingencyPlayer() )
		return "Your Prop";

	char szOwnerDisplay[256];
	Q_snprintf( szOwnerDisplay, sizeof(szOwnerDisplay), "%s's Prop", pPlayer->GetPlayerName() );
	return szOwnerDisplay;
}
