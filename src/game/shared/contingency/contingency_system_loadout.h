// Added loadout system

#ifndef CONTINGENCY_SYSTEM_LOADOUT_H
#define CONTINGENCY_SYSTEM_LOADOUT_H
#pragma once

static const int NUM_WEAPON_TYPE_PARAMETERS = 2;

static const int NUM_PRIMARY_WEAPON_TYPES = 6;
static const char* kPrimaryWeaponTypes[NUM_PRIMARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_smg1", "SMG1" },
	{ "weapon_alyxgun", "Alyx's Gun" },
	{ "weapon_shotgun", "Shotgun" },
	{ "weapon_ar2", "AR2" },
	{ "weapon_crossbow", "Crossbow" },
	{ "weapon_rpg", "RPG" }
};

static const int NUM_SECONDARY_WEAPON_TYPES = 4;
static const char* kSecondaryWeaponTypes[NUM_SECONDARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_pistol", "Pistol" },
	{ "weapon_pistols", "Dual Pistols" },
	{ "weapon_357", "357" },
	{ "weapon_physcannon", "Gravity Gun" }
};

static const int NUM_MELEE_WEAPON_TYPES = 4;
static const char* kMeleeWeaponTypes[NUM_MELEE_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_crowbar", "Crowbar" },
	{ "weapon_axe", "Axe" },
	{ "weapon_stunstick", "Stunstick" },
	{ "weapon_wrench", "Wrench" }
};

static const int NUM_EQUIPMENT_TYPES = 4;
static const char* kEquipmentTypes[NUM_EQUIPMENT_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_frag", "Frag Grenade" },
	{ "weapon_slam", "S.L.A.M." },
	{ "weapon_deployableturret", "Turret" },
	{ "weapon_healthkit", "Health Kit" }
};

// Special weapons are weapons that can be picked up regardless of a player's loadout
// They are also the only types of weapons that can be dropped using Contingency's drop system
// Some special weapons can only be obtained using cheats while others require explicit placement by level designers
static const int NUM_SPECIAL_WEAPON_TYPES = 1;
static const char* kSpecialWeaponTypes[NUM_SPECIAL_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_nyangun", "Nyan Gun" }
};

#endif // CONTINGENCY_SYSTEM_LOADOUT_H
