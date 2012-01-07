// Added prop spawning system

#ifndef CONTINGENCY_SYSTEM_PROPSPAWNING_H
#define CONTINGENCY_SYSTEM_PROPSPAWNING_H
#pragma once

static const int NUM_PROPSPAWNING_PARAMETERS = 7;

static const int NUM_SPAWNABLEPROP_TYPES = 13;
static const char* kSpawnablePropTypes[NUM_SPAWNABLEPROP_TYPES][NUM_PROPSPAWNING_PARAMETERS] =
{
	// Name, cost (in credits), filepath to associated image, filepath to associated mdl, height (y-vector) offset from origin, y-angle offset, initial "health" (HP)
	{ "Wooden Board", "1", "spawnableprops/woodenboard", "models/props_debris/wood_board04a.mdl", "32", "0", "10" },
	{ "Wooden Chair", "2", "spawnableprops/woodenchair", "models/props_c17/FurnitureChair001a.mdl", "19", "180", "20" },
	{ "Wooden Table", "2", "spawnableprops/woodentable", "models/props_c17/FurnitureTable002a.mdl", "18", "0", "20" },
	{ "Wooden Pallet", "2", "spawnableprops/woodenpallet", "models/props_junk/wood_pallet001a.mdl", "5", "0", "30" },
	{ "Wooden Bench", "2", "spawnableprops/woodenbench", "models/props_c17/bench01a.mdl", "20", "0", "30" },
	{ "Wooden Drawer", "2", "spawnableprops/woodendrawer", "models/props_c17/FurnitureDrawer001a.mdl", "20", "180", "40" },
	{ "Wooden Shelf", "3", "spawnableprops/woodenshelf", "models/props_interiors/Furniture_shelf01a.mdl", "43", "180", "40" },
	{ "Small Wooden Crate", "3", "spawnableprops/smallwoodencrate", "models/props_junk/wood_crate001a.mdl", "20", "0", "50" },
	{ "Large Wooden Crate", "4", "spawnableprops/largewoodencrate", "models/props_junk/wood_crate002a.mdl", "20", "0", "50" },
	{ "Wooden Fence Skeleton", "4", "spawnableprops/woodenfenceskeleton", "models/props_wasteland/wood_fence01b.mdl", "70", "90", "50" },
	{ "Small Wooden Fence", "4", "spawnableprops/smallwoodenfence", "models/props_wasteland/wood_fence02a.mdl", "65", "90", "75" },
	{ "Large Wooden Fence", "5", "spawnableprops/largewoodenfence", "models/props_wasteland/wood_fence01a.mdl", "65", "90", "100" },
	{ "Concrete Barrier", "10", "spawnableprops/concretebarrier", "models/props_c17/concrete_barrier001a.mdl", "0", "0", "200" }
};

// *** IMPORTANT INFO REGARDING ADDING/REMOVING PROP TYPES FROM ABOVE LIST ***
// See CContingencyRules::PrecacheStuff regarding some precaching I've done for
// some breakable props to prevent console warnings and possibly other issues

#endif // CONTINGENCY_SYSTEM_PROPSPAWNING_H
