// Added boss system

#ifndef CONTINGENCY_BOSS_SPAWNER_H
#define CONTINGENCY_BOSS_SPAWNER_H
#ifdef _WIN32
#pragma once
#endif

#include "monstermaker.h"

// Add a custom rally point entity for wave spawners
#include "contingency_rallypoint.h"

class CContingencyBossSpawner : public CBaseNPCMaker
{
public:
	DECLARE_CLASS( CContingencyBossSpawner, CBaseNPCMaker );

	CContingencyBossSpawner( void );
	~CContingencyBossSpawner( void );

	DECLARE_DATADESC();

public:
	void Spawn( void );
	void MakeNPC( void );

	// We never actually run out of NPCs to spawn,
	// it's just that the code handles when to spawn them
	// and how many to spawn at any given time
	bool IsDepleted() { return false; }

public:
	string_t m_ChildTargetName;
	string_t m_SquadName;
	string_t m_strHintGroup;

	string_t m_BossNPCType;
	int m_iBossSpawnFrequency;

	// Add a custom rally point entity for wave spawners
	CContingencyRallyPoint*	pRallyPoint;
	string_t rallyPointName;

private:
	bool m_bHasSpawnedBoss;
};

#endif // CONTINGENCY_BOSS_SPAWNER_H
