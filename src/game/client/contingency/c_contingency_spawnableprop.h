// Added spawnable prop system

#ifndef C_CONTINGENCY_SPAWNABLEPROP_H
#define C_CONTINGENCY_SPAWNABLEPROP_H
#pragma once

#include "cbase.h"
#include "c_physicsprop.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Contingency_SpawnableProp : public C_PhysicsProp
{
public:
	DECLARE_CLASS( C_Contingency_SpawnableProp, C_PhysicsProp );
	DECLARE_CLIENTCLASS();

	C_Contingency_SpawnableProp();
	virtual	~C_Contingency_SpawnableProp();

	const char* GetOwnerDisplay( void );

	CBasePlayer *GetSpawnerPlayer( void ) { return m_hSpawnerPlayer; }

private:
	CHandle<CBasePlayer> m_hSpawnerPlayer;
	bool m_bIsFrozen;

	C_Contingency_SpawnableProp( const C_Contingency_SpawnableProp & ); // not defined, not accessible
};

extern CUtlVector<C_Contingency_SpawnableProp*> m_SpawnablePropList;

#endif // C_CONTINGENCY_SPAWNABLEPROP_H
