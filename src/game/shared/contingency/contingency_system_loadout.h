// Added loadout system

#ifndef CONTINGENCY_SYSTEM_LOADOUT_H
#define CONTINGENCY_SYSTEM_LOADOUT_H
#pragma once

static const int NUM_WEAPON_TYPE_PARAMETERS = 2;

static const int NUM_PRIMARY_WEAPON_TYPES = 5;
static const char* kPrimaryWeaponTypes[NUM_PRIMARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_smg1", "4.6 mm SMG + GL" },
	{ "weapon_alyxgun", "9 mm Alyx's Gun" },
	{ "weapon_shotgun", "Shotgun" },
	{ "weapon_ar2", "AR2" },
	{ "weapon_crossbow", "Crossbow" }
};

static const int NUM_SECONDARY_WEAPON_TYPES = 4;
static const char* kSecondaryWeaponTypes[NUM_SECONDARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_pistol", "9 mm Pistol" },
	{ "weapon_pistols", "9 mm Dual Pistols" },
	{ "weapon_357", ".357 Magnum" },
	{ "weapon_physcannon", "Gravity Gun" }
};

static const int NUM_MELEE_WEAPON_TYPES = 3;
static const char* kMeleeWeaponTypes[NUM_MELEE_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_crowbar", "Crowbar" },
	{ "weapon_axe", "Axe" },
	{ "weapon_stunstick", "Stunstick" }
};

static const int NUM_EQUIPMENT_TYPES = 6;
static const char* kEquipmentTypes[NUM_EQUIPMENT_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_frag", "Frag Grenades" },
	{ "weapon_slam", "S.L.A.M.s" },
	{ "weapon_deployableturret", "Turret" },
	{ "weapon_healthkit", "Health Kit" },
	{ "weapon_rpg", "RPG" },
	{ "weapon_hopwire", "Vortex Grenade" }
};

// Special weapons are weapons that can be picked up regardless of a player's loadout
// Currently, they are also the only types of weapons that can be dropped using Contingency's drop system
// They can all be spawned using cheats and be placed by level designers for public use
static const int NUM_SPECIAL_WEAPON_TYPES = 1;
static const char* kSpecialWeaponTypes[NUM_SPECIAL_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_nyangun", "Nyan Gun" }
};

#endif // CONTINGENCY_SYSTEM_LOADOUT_H
