// Added spawnable prop system

#ifndef CONTINGENCY_SPAWNABLEPROP_H
#define CONTINGENCY_SPAWNABLEPROP_H
#pragma once

#include "npc_contingency_target.h"

class CContingency_SpawnableProp : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CContingency_SpawnableProp, CAI_BaseNPC );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CContingency_SpawnableProp( void );
	~CContingency_SpawnableProp( void );

	CBasePlayer *GetSpawnerPlayer( void ) { return m_hSpawnerPlayer; }
	void SetSpawnerPlayer( CBasePlayer *pNewSpawnerPlayer ) { m_hSpawnerPlayer = pNewSpawnerPlayer; }

	int GetSpawnablePropIndex( void ) { return m_iSpawnablePropIndex; }
	void SetSpawnablePropIndex( int iNewSpawnablePropIndex ) { m_iSpawnablePropIndex = iNewSpawnablePropIndex; }

	Class_T Classify( void );
	void Spawn( void );

	int ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION | FCAP_IMPULSE_USE; }
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int OnTakeDamage( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );

private:
	CNetworkHandle( CBasePlayer, m_hSpawnerPlayer );

	int m_iSpawnablePropIndex;
	CNPC_Contingency_Target *pTarget;
};

#endif // CONTINGENCY_SPAWNABLEPROP_H
