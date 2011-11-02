// Added prop spawning system

#ifndef CONTINGENCY_SYSTEM_PROPSPAWNING_H
#define CONTINGENCY_SYSTEM_PROPSPAWNING_H
#pragma once

static const int NUM_PROPSPAWNING_PARAMETERS = 6;

static const int NUM_SPAWNABLEPROP_TYPES = 8;
static const char* kSpawnablePropTypes[NUM_SPAWNABLEPROP_TYPES][NUM_PROPSPAWNING_PARAMETERS] =
{
	// Name, cost (in credits), filepath to associated image, filepath to associated mdl, height (y-vector) offset from origin, y-angle offset
	{ "Wooden Bench", "1", "spawnableprops/woodenbench", "models/props_c17/bench01a.mdl", "20", "0" },
	{ "Wooden Pallet", "2", "spawnableprops/woodenpallet", "models/props_junk/wood_pallet001a.mdl", "5", "0" },
	{ "Small Wooden Crate", "3", "spawnableprops/smallwoodencrate", "models/props_junk/wood_crate001a.mdl", "20", "0" },
	{ "Large Wooden Crate", "4", "spawnableprops/largewoodencrate", "models/props_junk/wood_crate002a.mdl", "20", "0" },
	{ "Wooden Fence Beams", "5", "spawnableprops/woodenfencebeams", "models/props_wasteland/wood_fence01c.mdl", "70", "90" },
	{ "Wooden Fence Skeleton", "6", "spawnableprops/woodenfenceskeleton", "models/props_wasteland/wood_fence01b.mdl", "70", "90" },
	{ "Small Wooden Fence", "7", "spawnableprops/smallwoodenfence", "models/props_wasteland/wood_fence02a.mdl", "65", "90" },
	{ "Large Wooden Fence", "8", "spawnableprops/largewoodenfence", "models/props_wasteland/wood_fence01a.mdl", "65", "90" }
};

#endif // CONTINGENCY_SYSTEM_PROPSPAWNING_H
