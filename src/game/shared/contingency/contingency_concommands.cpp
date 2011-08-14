// This is the ONE place where all Contingency-specific ConCommands
// should be defined (purely for organizational purposes)

#include "cbase.h"

#include "contingency_gamerules.h"

// Added prop spawning system
#ifndef CLIENT_DLL
	#include "contingency_system_propspawning.h"
	#include "contingency_spawnableprop.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Added prop spawning system
#ifndef CLIENT_DLL
	extern ConVar contingency_props_maxperplayer;
#endif

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

// Added prop spawning system
void CC_ShowPropSpawningMenu( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	pPlayer->ShowViewPortPanel( "propspawningmenu", true, NULL );
}
static ConCommand showpropspawningmenu( "showpropspawningmenu", CC_ShowPropSpawningMenu, "Shows the prop spawning menu for spawning props" );

// Added prop spawning system
void CC_SpawnProp( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( !ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Only living players can spawn props." );
		return;
	}

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You can only spawn props during interim phases." );
		return;
	}

	int iSpawnablePropIndex = Q_atoi( args[1] );	// get the specified spawnable prop index
	if ( (iSpawnablePropIndex < 0) || (iSpawnablePropIndex >= NUM_SPAWNABLEPROP_TYPES) )
		return;	// bounds check that index soldier!

	if ( !pPlayer->HasCredits(Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1])) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You do not have enough credits to spawn that prop." );
		return;
	}

	if ( pPlayer->m_SpawnablePropList.Count() >= contingency_props_maxperplayer.GetInt() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You have hit the server's maximum spawnable prop limit! Remove at least one of your existing spawnable props, then try again." );
		return;
	}

	// TODO: Do this precaching sooner!
	for ( int i = 0; i < NUM_SPAWNABLEPROP_TYPES; i++ )
		pPlayer->PrecacheModel( kSpawnablePropTypes[i][3] );

	CContingency_SpawnableProp *pSpawnableProp = dynamic_cast<CContingency_SpawnableProp*>( CreateEntityByName("contingency_spawnableprop") );
	if ( pSpawnableProp )
	{
		pSpawnableProp->SetModel( kSpawnablePropTypes[iSpawnablePropIndex][3] );
		pSpawnableProp->SetAbsOrigin( Vector(pPlayer->GetAbsOrigin().x, pPlayer->GetAbsOrigin().y, pPlayer->GetAbsOrigin().z + 32.0f) );
		pSpawnableProp->SetAbsAngles( pPlayer->GetAbsAngles() );

		pSpawnableProp->SetSpawnerPlayer( pPlayer );
		if ( pPlayer->m_SpawnablePropList.Find( pSpawnableProp ) == -1 )
		{
			pPlayer->m_SpawnablePropList.AddToTail( pSpawnableProp );	// add to our spawner's list of spawnable props
			pPlayer->SetNumSpawnableProps( pPlayer->GetNumSpawnableProps() + 1 );
		}

		// Actually spawn it and stuff
		pSpawnableProp->Precache();
		DispatchSpawn( pSpawnableProp );
		pSpawnableProp->Activate();

		Warning( kSpawnablePropTypes[iSpawnablePropIndex][0] );
		pPlayer->UseCredits( Q_atoi(kSpawnablePropTypes[iSpawnablePropIndex][1]) );	// spawned, so use up some of the player's credits
	}
}
static ConCommand spawnprop( "spawnprop", CC_SpawnProp, "Spawns props by the unique spawnable prop index specified" );

// Added spawnable prop system
void CC_RemoveSpawnablePropInFocus( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( !pPlayer->GetSpawnablePropInFocus() )
		return;	// this concommand has been used improperly

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be operated during interim phases." );
		return;	// we're only allowed to operate props during interim phases
	}

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
