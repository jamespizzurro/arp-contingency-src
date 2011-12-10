// Added sound cue and background music system

#include "cbase.h"

#include "contingency_system_music.h"

#ifdef CLIENT_DLL
	#include "c_contingency_player.h"
	#include "engine/ienginesound.h"
#else
	#include "contingency_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
	ConVar contingency_client_backgroundmusic(
		"contingency_client_backgroundmusic",
		"1",
		FCVAR_ARCHIVE,
		"Toggles the automatic playing of background music during combat and interim phases" );

	ConVar contingency_client_backgroundmusic_volume(
		"contingency_client_backgroundmusic_volume",
		"0.5",
		FCVAR_ARCHIVE,
		"Defines the normalized loudness of background music (0.0 to 1.0)" );
#endif

#ifdef CLIENT_DLL
void CContingency_System_Music::PlayBackgroundMusic( const char *filePathToBackgroundMusic )
{
	C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pLocalPlayer )
		return;

	// Stop any background music that might currently be playing
	if ( pLocalPlayer->GetAmbientSoundGUID() != -1 )
		enginesound->StopSoundByGuid( pLocalPlayer->GetAmbientSoundGUID() );

	if ( !contingency_client_backgroundmusic.GetBool() )
		return;	// this client does not want us playing background music for them, so don't

	// Actually play the specified backgroung music
	// WARNING/TODO: This solution can sometimes interfere with map sounds!
	enginesound->EmitAmbientSound( filePathToBackgroundMusic, contingency_client_backgroundmusic_volume.GetFloat() );
	pLocalPlayer->SetAmbientSoundGUID( enginesound->GetGuidForLastSoundEmitted() );
}

// Added sound cue and background music system
void CContingency_System_Music::StopPlayingBackgroundMusic( void )
{
	C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pLocalPlayer )
		return;

	// Stop any background music that might currently be playing
	// WARNING/TODO: This solution can sometimes interfere with map sounds!
	if ( pLocalPlayer->GetAmbientSoundGUID() != -1 )
		enginesound->StopSoundByGuid( pLocalPlayer->GetAmbientSoundGUID() );
}
#else
void CContingency_System_Music::PlayBackgroundMusic( BACKGROUND_MUSIC_TYPES backgroundMusicType )
{
	const char *szCmd;

	switch ( backgroundMusicType )
	{
	case BACKGROUND_MUSIC_INTERIM:
		szCmd = BACKGROUND_MUSIC_INTERIM_CMD;
		break;
	case BACKGROUND_MUSIC_COMBAT:
		szCmd = BACKGROUND_MUSIC_COMBAT_CMD;
		break;
	default:
		return;
		break;
	}

	int i;
	CContingency_Player *pPlayer;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex(i) );
		if ( !pPlayer )
			continue;

		engine->ClientCommand( pPlayer->edict(), szCmd );
	}
}

void CContingency_System_Music::PlayAnnouncementSound( const char *soundName, CBasePlayer *pTargetPlayer )
{
	CRecipientFilter filter;

	if ( pTargetPlayer )	// target player defined, play sound only for specified player
		filter.AddRecipient( pTargetPlayer );
	else	// no target player defined, play sound for all players
		filter.AddAllPlayers();

	filter.MakeReliable();

	UserMessageBegin( filter, "SendAudio" );
	WRITE_STRING( soundName );
	MessageEnd();
}
#endif

#ifdef CLIENT_DLL
void CC_PlayCombatBackgroundMusic( void )
{
	CContingency_System_Music::PlayBackgroundMusic( kCombatBackgroundMusic[random->RandomInt(0, NUM_COMBAT_BACKGROUND_MUSIC - 1)] );
}
static ConCommand playcombatbackgroundmusic( BACKGROUND_MUSIC_COMBAT_CMD, CC_PlayCombatBackgroundMusic, "Plays some random combat background music", 0 );

void CC_PlayInterimBackgroundMusic( void )
{
	CContingency_System_Music::PlayBackgroundMusic( kInterimBackgroundMusic[random->RandomInt(0, NUM_INTERIM_BACKGROUND_MUSIC - 1)] );
}
static ConCommand playinterimbackgroundmusic( BACKGROUND_MUSIC_INTERIM_CMD, CC_PlayInterimBackgroundMusic, "Plays some random interim background music", 0 );

void CC_StopPlayingBackgroundMusic( void )
{
	CContingency_System_Music::StopPlayingBackgroundMusic();
}
static ConCommand stopplayingbackgroundmusic( BACKGROUND_MUSIC_STOP_CMD, CC_StopPlayingBackgroundMusic, "Stops any background music that might be playing", 0 );
#endif
