// Added wave system

#ifndef CONTINGENCY_SYSTEM_WAVE_H
#define CONTINGENCY_SYSTEM_WAVE_H
#pragma once

enum CONTINGENCY_WAVES
{
	WAVE_NONE = -1,
	WAVE_HEADCRABS,
	WAVE_ANTLIONS,
	WAVE_ZOMBIES,
	WAVE_COMBINE,

	NUM_WAVES
};

// Wave types
static const int NUM_HEADCRAB_NPCS = 3;
static const char* kWaveHeadcrabsNPCTypes[NUM_HEADCRAB_NPCS] =
{
	"npc_headcrab",
	"npc_headcrab_fast",
	"npc_headcrab_black"
};
static const int NUM_ANTLION_NPCS = 1;
static const char* kWaveAntlionsNPCTypes[NUM_ANTLION_NPCS] =
{
	"npc_antlion"
};
static const int NUM_ZOMBIE_NPCS = 4;
static const char* kWaveZombiesNPCTypes[NUM_ZOMBIE_NPCS] =
{
	"npc_zombie",
	"npc_zombie_torso",
	"npc_fastzombie",
	"npc_poisonzombie"
};
static const int NUM_COMBINE_NPCS = 5;
static const char* kWaveCombineNPCTypes[NUM_COMBINE_NPCS] =
{
	"npc_combine_s",
	"npc_metropolice",
	"npc_cscanner",
	"npc_manhack",
	"npc_stalker"
};
static const int NUM_COMBINE_S_WEAPONS = 3;
static const char* kWaveCombineSWeaponTypes[NUM_COMBINE_S_WEAPONS] =
{
	"weapon_shotgun",
	"weapon_smg1",
	"weapon_ar2"
};
static const int NUM_METROPOLICE_WEAPONS = 2;
static const char* kWaveMetropoliceWeaponTypes[NUM_METROPOLICE_WEAPONS] =
{
	"weapon_pistol",
	"weapon_smg1"
};

// Support wave types
static const int NUM_SUPPORT_NPCS = 1;
static const char* kSupportWaveSupportNPCTypes[NUM_SUPPORT_NPCS] =
{
	"npc_citizen"
};
static const int NUM_CITIZEN_WEAPONS = 3;
static const char* kSupportWaveCitizenWeaponTypes[NUM_CITIZEN_WEAPONS] =
{
	"weapon_shotgun",
	"weapon_smg1",
	"weapon_ar2"
};

#endif // CONTINGENCY_SYSTEM_WAVE_H
