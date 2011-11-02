// Added wave system

#ifndef CONTINGENCY_WAVE_SPAWNER_H
#define CONTINGENCY_WAVE_SPAWNER_H
#ifdef _WIN32
#pragma once
#endif

#include "monstermaker.h"

// Add a custom rally point entity for wave spawners
#include "contingency_rallypoint.h"

/* SPAWNFLAGS */
// kWaveHeadcrabsNPCTypes
#define SF_WAVESPAWNER_HEADCRAB 1024
#define SF_WAVESPAWNER_HEADCRAB_FAST 2048
#define SF_WAVESPAWNER_HEADCRAB_BLACK 4096
// kWaveAntlionsNPCTypes
#define SF_WAVESPAWNER_ANTLION 8192
// kWaveZombiesNPCTypes
#define SF_WAVESPAWNER_ZOMBIE 16384
#define SF_WAVESPAWNER_ZOMBIE_TORSO 32768
#define SF_WAVESPAWNER_ZOMBIE_FAST 65536
#define SF_WAVESPAWNER_ZOMBIE_POISON 131072
// kWaveCombineNPCTypes
#define SF_WAVESPAWNER_COMBINE 262144
#define SF_WAVESPAWNER_COMBINE_METRO 524288
#define SF_WAVESPAWNER_COMBINE_SCANNER 1048576
#define SF_WAVESPAWNER_COMBINE_MANHACK 2097152
#define SF_WAVESPAWNER_COMBINE_STALKER 4194304

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
