// Added wave system

#ifndef CONTINGENCY_WAVE_SPAWNER_H
#define CONTINGENCY_WAVE_SPAWNER_H
#ifdef _WIN32
#pragma once
#endif

#include "monstermaker.h"

// Add a custom rally point entity for wave spawners
#include "contingency_rallypoint.h"

// Spawnflags
#define SF_WAVESPAWNER_HEADCRABS 1024
#define SF_WAVESPAWNER_ANTLIONS 2048
#define SF_WAVESPAWNER_ZOMBIES 4096
#define SF_WAVESPAWNER_COMBINE 8192

class CContingencyWaveSpawner : public CBaseNPCMaker
{
public:
	DECLARE_CLASS( CContingencyWaveSpawner, CBaseNPCMaker );

	CContingencyWaveSpawner( void );
	~CContingencyWaveSpawner( void );

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

	// Add a custom rally point entity for wave spawners
	CContingencyRallyPoint*	pRallyPoint;
	string_t rallyPointName;

	float m_flMaximumDistanceFromNearestPlayer;
};

#endif // CONTINGENCY_WAVE_SPAWNER_H
