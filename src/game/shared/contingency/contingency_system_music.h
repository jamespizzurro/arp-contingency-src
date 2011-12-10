// Added sound cue and background music system

#ifndef CONTINGENCY_SYSTEM_MUSIC_H
#define CONTINGENCY_SYSTEM_MUSIC_H
#pragma once

static const char* BACKGROUND_MUSIC_INTERIM_CMD = "playinterimbackgroundmusic";
static const char* BACKGROUND_MUSIC_COMBAT_CMD = "playcombatbackgroundmusic";
static const char* BACKGROUND_MUSIC_STOP_CMD = "stopplayingbackgroundmusic";

static const int NUM_COMBAT_BACKGROUND_MUSIC = 9;
static const char* kCombatBackgroundMusic[NUM_COMBAT_BACKGROUND_MUSIC] =
{
	"music/HL1_song10_loop.wav",
	"music/HL1_song15_loop.wav",
	"music/HL2_song3_loop.wav",
	"music/HL2_song14_loop.wav",
	"music/HL2_song16_loop.wav",
	"music/HL2_song20_submix0_loop.wav",
	"music/HL2_song20_submix4_loop.wav",
	"music/HL2_song29_loop.wav",
	"music/HL2_song31_loop.wav"
};
static const int NUM_INTERIM_BACKGROUND_MUSIC = 5;
static const char* kInterimBackgroundMusic[NUM_INTERIM_BACKGROUND_MUSIC] =
{
	"music/interim1.wav",
	"music/interim2.wav",
	"music/interim3.wav",
	"music/interim4.wav",
	"music/interim5.wav"
};

enum BACKGROUND_MUSIC_TYPES
{
	BACKGROUND_MUSIC_INTERIM,
	BACKGROUND_MUSIC_COMBAT,

	NUM_BACKGROUND_MUSIC_TYPES
};

class CContingency_System_Music
{
public:
#ifdef CLIENT_DLL
	static void PlayBackgroundMusic( const char *filePathToBackgroundMusic );
	static void StopPlayingBackgroundMusic( void );
#else
	static void PlayBackgroundMusic( BACKGROUND_MUSIC_TYPES backgroundMusicType );
	static void PlayAnnouncementSound( const char *soundName, CBasePlayer *pTargetPlayer = NULL );
#endif
};

#endif // CONTINGENCY_SYSTEM_MUSIC_H
