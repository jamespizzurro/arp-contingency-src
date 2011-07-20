// Added loadout system

#ifndef CONTINGENCY_WEAPON_TYPES_H
#define CONTINGENCY_WEAPON_TYPES_H
#pragma once

// TODO: All of this is in its own header because it stands to be redone
// using external text files and KeyValues for easy editing and general manipulation
// without having to recompile and stuff...

static const int NUM_WEAPON_TYPE_PARAMETERS = 2;

static const int NUM_PRIMARY_WEAPON_TYPES = 6;
static const char* kPrimaryWeaponTypes[NUM_PRIMARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_smg1", "SMG1" },
	{ "weapon_shotgun", "Shotgun" },
	{ "weapon_ar2", "AR2" },
	{ "weapon_crossbow", "Crossbow" },
	{ "weapon_rpg", "RPG" },
	{ "weapon_357", "357" }
};

static const int NUM_SECONDARY_WEAPON_TYPES = 2;
static const char* kSecondaryWeaponTypes[NUM_SECONDARY_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_pistol", "Pistol" },
	{ "weapon_physcannon", "Gravity Gun" }
};

static const int NUM_MELEE_WEAPON_TYPES = 2;
static const char* kMeleeWeaponTypes[NUM_MELEE_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_crowbar", "Crowbar" },
	{ "weapon_stunstick", "Stunstick" }
};

static const int NUM_EQUIPMENT_TYPES = 2;
static const char* kEquipmentTypes[NUM_EQUIPMENT_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_frag", "Frag Grenade" },
	{ "weapon_slam", "S.L.A.M." }
};

// Special weapons are weapons that can be picked up regardless of a player's loadout
// They are also the only types of weapons that can be dropped using Contingency's drop system
// Some special weapons can only be obtained using cheats while others require explicit placement by level designers
static const int NUM_SPECIAL_WEAPON_TYPES = 2;
static const char* kSpecialWeaponTypes[NUM_SPECIAL_WEAPON_TYPES][NUM_WEAPON_TYPE_PARAMETERS] =
{
	{ "weapon_nyangun", "Nyan Gun" },
	{ "weapon_healthkit", "Health Kit" }
};

#endif // CONTINGENCY_WEAPON_TYPES_H
