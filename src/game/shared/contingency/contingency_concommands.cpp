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

void CC_Teleport( void )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	// Determine whether or not we are actually stuck inside something
	// We can only teleport back to spawn if we are
	Ray_t ray;
	ray.Init( pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin(), pPlayer->GetPlayerMins(), pPlayer->GetPlayerMaxs() );
	trace_t trace;
	UTIL_TraceRay( ray, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );
	if ( (trace.contents & MASK_PLAYERSOLID) && trace.m_pEnt )
	{
		// We are in fact stuck, so we are allowed to teleport!

		// Find all spawnpoints on the map
		CUtlVector<CBaseEntity*> *spawnpointsList = new CUtlVector<CBaseEntity*>();
		CBaseEntity *pEntity = gEntList.FindEntityByClassname( NULL, "info_player_deathmatch" );
		while ( pEntity != NULL )
		{
			spawnpointsList->AddToTail( pEntity );
			pEntity = gEntList.FindEntityByClassname( pEntity, "info_player_deathmatch" );
		}

		// Choose a random spawnpoint to use
		pEntity = spawnpointsList->Element( random->RandomInt(0, spawnpointsList->Count() - 1) );

		delete spawnpointsList;	// we don't need this anymore

		// Actually teleport the player
		pPlayer->SetAbsVelocity( Vector(0, 0, 0) );	// make sure we don't kill the player from fall damage or something when teleporting them
		pPlayer->SetAbsOrigin( pEntity->GetAbsOrigin() );
		pPlayer->SetAbsAngles( pEntity->GetAbsAngles() );
	}
	else
		ClientPrint( pPlayer, HUD_PRINTTALK, "You must be stuck to teleport back to spawn!" );
}
static ConCommand teleport( "teleport", CC_Teleport, "Teleport to spawn (only when stuck)" );

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

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be operated during interim phases." );
		return;	// we're only allowed to operate props during interim phases
	}

	CContingency_SpawnableProp *pProp = pPlayer->GetSpawnablePropInFocus();
	if ( !pProp )
		return;	// this concommand has been used improperly

	if ( pPlayer != pProp->GetSpawnerPlayer() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "This prop is not yours to operate." );
		return;	// only our spawner can operate us
	}

	if ( pProp->IsDissolving() )
		return;	// props that are already dissolving should be ignored

	// Give the player their credits back that they used to purchase this prop
	pPlayer->AddCredits( Q_atoi(kSpawnablePropTypes[pProp->GetSpawnablePropIndex()][1]) );

	pProp->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
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

void CC_CheatSetWaveNumber( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( (args.ArgC() < 1) || (Q_strcmp(args.Arg(1), "") == 0) || (atoi(args.Arg(1)) <= 0) )
	{
		Msg( "Description: sets the current wave number to the specified positive integer\n" );
		Msg( "Usage: contingency_cheat_setwavenumber <positive integer>\n" );
		return;
	}

	ContingencyRules()->SetWaveNumber( atoi(args.Arg(1)) );
}
static ConCommand contingency_cheat_setwavenumber( "contingency_cheat_setwavenumber", CC_CheatSetWaveNumber, "CHEAT: Sets the current wave number to the specified positive integer", FCVAR_CHEAT );

void CC_CheatSetWaveType( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( (args.ArgC() < 1) || (Q_strcmp(args.Arg(1), "") == 0) || (atoi(args.Arg(1)) < WAVE_NONE) || (atoi(args.Arg(1)) >= NUM_WAVES) )
	{
		Msg( "Description: sets the next wave type according to the integer specified (-1 for no preference, 0 for antlion, 1 for zombie, 2 for combine)\n" );
		Msg( "Usage: contingency_cheat_setwavetype <integer between -1 and 2 (inclusive)>\n" );
		return;
	}

	ContingencyRules()->SetPreferredWaveType( atoi(args.Arg(1)) );
}
static ConCommand contingency_cheat_setwavetype( "contingency_cheat_setwavetype", CC_CheatSetWaveType, "CHEAT: Sets the next wave type according to the integer specified (-1 for no preference, 0 for antlion, 1 for zombie, 2 for combine)", FCVAR_CHEAT );

void CC_CheatSetCredits( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( (args.ArgC() < 1) || (Q_strcmp(args.Arg(1), "") == 0) || (atoi(args.Arg(1)) < 0) )
	{
		Msg( "Description: sets the player's credits to the positive integer specified\n" );
		Msg( "Usage: contingency_cheat_setcredits <positive integer>\n" );
		return;
	}

	pPlayer->SetCredits( atoi(args.Arg(1)) );
}
static ConCommand contingency_cheat_setcredits( "contingency_cheat_setcredits", CC_CheatSetCredits, "CHEAT: Sets the player's credits to the positive integer specified", FCVAR_CHEAT );

#endif
