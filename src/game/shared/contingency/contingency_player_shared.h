#ifndef CONTINGENCY_PLAYER_SHARED_H
#define CONTINGENCY_PLAYER_SHARED_H
#pragma once

#include "hl2mp_player_shared.h"

#if defined( CLIENT_DLL )
#define CContingency_Player C_Contingency_Player
#endif

// Added loadout system
// Defines max health for each loadout

#define LOADOUT_SOLDIER_MAXHEALTH 100
#define LOADOUT_SHOTGUN_SOLDIER_MAXHEALTH 100
#define LOADOUT_COMMANDER_MAXHEALTH 125
#define LOADOUT_MARKSMAN_MAXHEALTH 75
#define LOADOUT_DEMOLITIONIST_MAXHEALTH 50

// Adjust players' max speed based on different factors
// This is the one place where player speeds are defined now

#define	CONTINGENCY_WALK_SPEED (3*CONTINGENCY_NORM_SPEED)/4
#define	CONTINGENCY_NORM_SPEED 150
#define	CONTINGENCY_SPRINT_SPEED (5*CONTINGENCY_NORM_SPEED)/4

#define	LOADOUT_SOLDIER_WALK_SPEED (3*LOADOUT_SOLDIER_NORM_SPEED)/4
#define	LOADOUT_SOLDIER_NORM_SPEED 200
#define	LOADOUT_SOLDIER_SPRINT_SPEED (5*LOADOUT_SOLDIER_NORM_SPEED)/4

#define	LOADOUT_SHOTGUN_SOLDIER_WALK_SPEED (3*LOADOUT_SHOTGUN_SOLDIER_NORM_SPEED)/4
#define	LOADOUT_SHOTGUN_SOLDIER_NORM_SPEED 200
#define	LOADOUT_SHOTGUN_SOLDIER_SPRINT_SPEED (5*LOADOUT_SHOTGUN_SOLDIER_NORM_SPEED)/4

#define	LOADOUT_COMMANDER_WALK_SPEED (3*LOADOUT_COMMANDER_NORM_SPEED)/4
#define	LOADOUT_COMMANDER_NORM_SPEED 150
#define	LOADOUT_COMMANDER_SPRINT_SPEED (5*LOADOUT_COMMANDER_NORM_SPEED)/4

#define	LOADOUT_MARKSMAN_WALK_SPEED (3*LOADOUT_MARKSMAN_NORM_SPEED)/4
#define	LOADOUT_MARKSMAN_NORM_SPEED 250
#define	LOADOUT_MARKSMAN_SPRINT_SPEED (5*LOADOUT_MARKSMAN_NORM_SPEED)/4

#define	LOADOUT_DEMOLITIONIST_WALK_SPEED (3*LOADOUT_DEMOLITIONIST_NORM_SPEED)/4
#define	LOADOUT_DEMOLITIONIST_NORM_SPEED 200
#define	LOADOUT_DEMOLITIONIST_SPRINT_SPEED (5*LOADOUT_DEMOLITIONIST_NORM_SPEED)/4

// Added loadout system
enum CONTINGENCY_LOADOUT
{
	LOADOUT_SOLDIER,
	LOADOUT_SHOTGUN_SOLDIER,
	LOADOUT_COMMANDER,
	LOADOUT_MARKSMAN,
	LOADOUT_DEMOLITIONIST,

	NUM_LOADOUTS
};

// Added loadout system
// The order corresponds to the order of the CONTINGENCY_LOADOUT enumerator above
static const char* kContingencyPlayerModels[NUM_LOADOUTS] =	// more stages can be added here (remember to change NUM_STAGES too)
{
	"models/player/soldier.mdl",
	"models/player/shotgun_soldier.mdl",
	"models/player/commander.mdl",
	"models/player/marksman.mdl",
	"models/player/demolitionist.mdl"
};

#endif // CONTINGENCY_PLAYER_SHARED_H
