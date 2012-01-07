// Added spawnable prop system

#ifndef C_CONTINGENCY_SPAWNABLEPROP_H
#define C_CONTINGENCY_SPAWNABLEPROP_H
#pragma once

#include "c_ai_basenpc.h"

class C_Contingency_SpawnableProp : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_Contingency_SpawnableProp, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	C_Contingency_SpawnableProp();
	virtual	~C_Contingency_SpawnableProp();

	const char* GetOwnerDisplay( void );
	const char* GetHealthCondition( void );
	Color GetHealthConditionColor( void );

	CBasePlayer *GetSpawnerPlayer( void ) { return m_hSpawnerPlayer; }

private:
	CHandle<CBasePlayer> m_hSpawnerPlayer;

	C_Contingency_SpawnableProp( const C_Contingency_SpawnableProp & ); // not defined, not accessible
};

extern CUtlVector<C_Contingency_SpawnableProp*> m_SpawnablePropList;

#endif // C_CONTINGENCY_SPAWNABLEPROP_H
