// Added prop spawning system

#ifndef CONTINGENCY_SYSTEM_PROPSPAWNING_H
#define CONTINGENCY_SYSTEM_PROPSPAWNING_H
#pragma once

static const int NUM_PROPSPAWNING_PARAMETERS = 4;

static const int NUM_SPAWNABLEPROP_TYPES = 5;
static const char* kSpawnablePropTypes[NUM_SPAWNABLEPROP_TYPES][NUM_PROPSPAWNING_PARAMETERS] =
{
	// Name, cost (in credits), filepath to associated image, filepath to associated mdl
	// REMINDER: If you need to convert cost -- which is defined as a string here -- to an integer, use Q_atoi!
	{ "Wooden Bench", "3", "spawnableprops/woodenbench", "models/props_c17/bench01a.mdl" },
	{ "Small Wooden Crate", "5", "spawnableprops/smallwoodencrate", "models/props_junk/wood_crate001a.mdl" },
	{ "Large Wooden Crate", "7", "spawnableprops/largewoodencrate", "models/props_junk/wood_crate002a.mdl" },
	{ "Metal Chair", "9", "spawnableprops/metalchair", "models/props_wasteland/controlroom_chair001a.mdl" },
	{ "Concrete Barrier", "11", "spawnableprops/concretebarrier", "models/props_c17/concrete_barrier001a.mdl" }
};

#endif // CONTINGENCY_SYSTEM_PROPSPAWNING_H
