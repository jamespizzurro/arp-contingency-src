#ifndef CONTINGENCY_PLAYER_SHARED_H
#define CONTINGENCY_PLAYER_SHARED_H
#pragma once

#include "hl2mp_player_shared.h"

#if defined( CLIENT_DLL )
#define CContingency_Player C_Contingency_Player
#endif

// Added loadout system
#define CONTINGENCY_MAX_HEALTH 100
#define	CONTINGENCY_WALK_SPEED (3*CONTINGENCY_NORM_SPEED)/4
#define	CONTINGENCY_NORM_SPEED 200
#define	CONTINGENCY_SPRINT_SPEED (5*CONTINGENCY_NORM_SPEED)/4
#define CONTINGENCY_MINIMUM_SPEED 50

// Added loadout system
static const int NUM_PLAYER_MODELS = 5;
static const char* kContingencyPlayerModels[NUM_PLAYER_MODELS] =
{
	// these used to correspond to particular classes (hence the names),
	// but that system has since been removed, and so we're left with this

	"models/player/soldier.mdl",
	"models/player/shotgun_soldier.mdl",
	"models/player/commander.mdl",
	"models/player/marksman.mdl",
	"models/player/demolitionist.mdl"
};

#endif // CONTINGENCY_PLAYER_SHARED_H
