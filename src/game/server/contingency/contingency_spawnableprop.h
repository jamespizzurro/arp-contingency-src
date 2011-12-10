// Added spawnable prop system

#ifndef CONTINGENCY_SPAWNABLEPROP_H
#define CONTINGENCY_SPAWNABLEPROP_H
#pragma once

#include "props.h"
#include "npc_contingency_target.h"

class CContingency_SpawnableProp : public CPhysicsProp
{
public:
	DECLARE_CLASS( CContingency_SpawnableProp, CPhysicsProp );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CContingency_SpawnableProp( void );
	~CContingency_SpawnableProp( void );

	CBasePlayer *GetSpawnerPlayer( void ) { return m_hSpawnerPlayer; }
	void SetSpawnerPlayer( CBasePlayer *pNewSpawnerPlayer ) { m_hSpawnerPlayer = pNewSpawnerPlayer; }

	int GetSpawnablePropIndex( void ) { return m_iSpawnablePropIndex; }
	void SetSpawnablePropIndex( int iNewSpawnablePropIndex ) { m_iSpawnablePropIndex = iNewSpawnablePropIndex; }

	void Spawn( void );

	int ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION | FCAP_IMPULSE_USE; }
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
	CNetworkHandle( CBasePlayer, m_hSpawnerPlayer );

	int m_iSpawnablePropIndex;
	CNPC_Contingency_Target *pTarget;
};

#endif // CONTINGENCY_SPAWNABLEPROP_H
