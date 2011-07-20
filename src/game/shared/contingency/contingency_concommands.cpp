// This is the ONE place where all Contingency-specific ConCommands
// should be defined (purely for organizational purposes)

#include "cbase.h"

#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

// Added sound cue and background music system
void CC_PlayBackgroundMusic( void )
{
	ContingencyRules()->PlayBackgroundMusic();
}
static ConCommand playbackgroundmusic( "playbackgroundmusic", CC_PlayBackgroundMusic, "Plays some random background music", 0 );

// Added sound cue and background music system
void CC_StopPlayingBackgroundMusic( void )
{
	ContingencyRules()->StopPlayingBackgroundMusic();
}
static ConCommand stopplayingbackgroundmusic( "stopplayingbackgroundmusic", CC_StopPlayingBackgroundMusic, "Stops any background music that might be playing", 0 );

#else

// Added drop system
void CC_DropCurrentWeapon( void )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( !pWeapon )
		return;

	// Only allow players to drop special weapons
	for ( int i = 0; i < NUM_SPECIAL_WEAPON_TYPES; i++ )
	{
		if ( FClassnameIs(pWeapon, kSpecialWeaponTypes[i][0]) )
		{
			pPlayer->Weapon_Drop( pWeapon, NULL, NULL );
			return;
		}
	}
}
static ConCommand drop( "drop", CC_DropCurrentWeapon, "Drops your current weapon (if possible; only works for special weapons)" );

// Added loadout menu
void CC_ShowLoadoutMenu( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	// We used to restrict when players were allowed to bring up this menu,
	// but they wanted to be able to change their loadout anytime even if
	// those changes weren't applied right away, so a new system was devised

	pPlayer->ShowViewPortPanel( "loadoutmenu", true, NULL );
}
static ConCommand showloadoutmenu( "showloadoutmenu", CC_ShowLoadoutMenu, "Shows the loadout menu for changing one's loadout (when permitted)" );

#endif
