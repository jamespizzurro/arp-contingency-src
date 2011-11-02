// This is the ONE place where all Contingency-specific ConCommands
// should be defined (purely for organizational purposes)
// ...keep in mind there are always some exceptions to this "rule" though!

#include "cbase.h"

#include "contingency_gamerules.h"

#ifndef CLIENT_DLL
	#include "contingency_system_propspawning.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
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
static ConCommand showloadoutmenu( "showloadoutmenu", CC_ShowLoadoutMenu, "Shows the loadout menu for changing one's weapon and equipment" );

// Added shout system
void CC_ShowShoutMenu( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( !ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You must be alive to shout!" );
		return;
	}

	if ( gpGlobals->curtime < pPlayer->GetShoutDelay() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You've already shouted recently! Wait a few seconds, then try again." );
		return;
	}

	pPlayer->ShouldShowShoutMenu( true );
	CSingleUserRecipientFilter user( pPlayer );
	user.MakeReliable();

	UserMessageBegin( user, "ShowMenu" );
		WRITE_WORD( (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) );
		WRITE_CHAR( 15 );
		WRITE_BYTE( false );
		WRITE_STRING("-=[ SHOUT MENU ]=-\n \n[1] Incoming!\n[2] Run!\n[3] Let's go!\n[4] Lead the way!\n[5] Take cover!\n[6] Ready!\n[7] Headcrabs!\n[8] Zombies!\n[9] Combine!\n \n[0] EXIT\n");
	MessageEnd();
}
static ConCommand showshoutmenu( "showshoutmenu", CC_ShowShoutMenu, "Shows the shout menu (when permitted)" );

// Added spawnable prop system
void CC_SelectProp( const CCommand &args )
{
	int iSpawnablePropIndex = Q_atoi( args[1] );	// get the specified spawnable prop index
	if ( (iSpawnablePropIndex < 0) || (iSpawnablePropIndex >= NUM_SPAWNABLEPROP_TYPES) )
		return;	// bounds check that index soldier!

	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	pPlayer->SetDesiredSpawnablePropIndex( iSpawnablePropIndex );	// updates the prop the player's wrench can spawn
}
static ConCommand selectprop( "selectprop", CC_SelectProp, "Selects a prop to spawn by the unique spawnable prop index specified" );

// Added spawnable prop system
void CC_RemoveSpawnablePropInFocus( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( !pPlayer->GetSpawnablePropInFocus() )
		return;	// this concommand has been used improperly

	if ( pPlayer->GetSpawnablePropInFocus()->IsDissolving() )
		return;	// props that are already dissolving should be ignored

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be operated during interim phases." );
		return;	// we're only allowed to operate props during interim phases
	}

	// Give the player their credits back that they used to purchase this prop
	pPlayer->AddCredits( Q_atoi(kSpawnablePropTypes[pPlayer->GetSpawnablePropInFocus()->GetSpawnablePropIndex()][1]) );

	pPlayer->GetSpawnablePropInFocus()->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
	pPlayer->SetSpawnablePropInFocus( NULL );
}
static ConCommand removespawnablepropinfocus( "removespawnablepropinfocus", CC_RemoveSpawnablePropInFocus, "Removes the spawnable prop currently in focus" );

// Added spawnable prop system
void CC_ForgetSpawnablePropInFocus( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	pPlayer->SetSpawnablePropInFocus( NULL );
}
static ConCommand forgetspawnablepropinfocus( "forgetspawnablepropinfocus", CC_ForgetSpawnablePropInFocus, "Resets the spawnable prop currently in focus" );

#endif
