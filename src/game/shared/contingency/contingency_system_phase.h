// Added phase system

#ifndef CONTINGENCY_SYSTEM_PHASE_H
#define CONTINGENCY_SYSTEM_PHASE_H
#pragma once

enum CONTINGENCY_PHASES
{
	PHASE_WAITING_FOR_PLAYERS = -1,	// not quite a phase (more like the absence of one, hence the -1)
	PHASE_INTERIM,
	PHASE_COMBAT,

	NUM_PHASES
};

#endif // CONTINGENCY_SYSTEM_PHASE_H
