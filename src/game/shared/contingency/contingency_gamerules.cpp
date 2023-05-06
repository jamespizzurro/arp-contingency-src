#include "cbase.h"
#include "contingency_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"

// Moved ammo definitions to contingency_gamerules.cpp
#include "hl2_shareddefs.h" //AI Patch Addition.

#ifdef CLIENT_DLL
	#include "c_contingency_player.h"
	#include "c_npc_citizen17.h"
	#include "c_npc_contingency_turret.h"
#else
	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "contingency_player.h"
	#include "weapon_hl2mpbasehlmpcombatweapon.h"
	#include "team.h"
	#include "voice_gamemgr.h"
	#include "hl2mp_gameinterface.h"
	#include "hl2mp_cvars.h"

	// Added spawnable prop system
	#include "contingency_system_propspawning.h"

#ifdef DEBUG	
	#include "hl2mp_bot_temp.h"
#endif

	#include "grenade_satchel.h"
	#include "grenade_tripmine.h"

	#include "npc_citizen17.h"
	#include "npc_contingency_turret.h"

	// Added wave system
	#include "contingency_wave_spawnpoint.h"
#endif

REGISTER_GAMERULES_CLASS( CContingencyRules );

BEGIN_NETWORK_TABLE_NOBASE( CContingencyRules, DT_ContingencyRules )
#ifdef CLIENT_DLL
	// Added phase system
	RecvPropInt( RECVINFO( m_iCurrentPhase ) ),
	RecvPropInt( RECVINFO( m_iInterimPhaseTimeLeft ) ),

	// Added wave system
	RecvPropInt( RECVINFO( m_iWaveNum ) ),
	RecvPropInt( RECVINFO( m_iWaveType ) ),
	RecvPropInt( RECVINFO( m_iPreferredWaveType ) ),
	RecvPropInt( RECVINFO( m_iNumEnemiesRemaining ) ),

	// Added radar display
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	RecvPropBool( RECVINFO( m_bMapAllowsRadars ) ),

	// Added spawnable prop system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	RecvPropInt( RECVINFO( m_iMapMaxPropsPerPlayer ) ),
#else
	// Added phase system
	SendPropInt( SENDINFO( m_iCurrentPhase ) ),
	SendPropInt( SENDINFO( m_iInterimPhaseTimeLeft ) ),

	// Added wave system
	SendPropInt( SENDINFO( m_iWaveNum ) ),
	SendPropInt( SENDINFO( m_iWaveType ) ),
	SendPropInt( SENDINFO( m_iPreferredWaveType ) ),
	SendPropInt( SENDINFO( m_iNumEnemiesRemaining ) ),

	// Added radar display
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	SendPropBool( SENDINFO( m_bMapAllowsRadars ) ),

	// Added spawnable prop system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	SendPropInt( SENDINFO( m_iMapMaxPropsPerPlayer ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( contingency_gamerules, CContingencyRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( ContingencyRulesProxy, DT_ContingencyRulesProxy )

#ifndef CLIENT_DLL
	extern ConVar sv_report_client_settings;
#endif

// ALL CONTINGENCY CONVARS SHOULD GO HERE regardless of where they are used
// This is for organizational purposes as ConVars everywhere can get quite confusing
// ...keep in mind there are always some exceptions to this "rule" though!

#ifndef CLIENT_DLL
	// Health regeneration system
	ConVar contingency_health_regen( "contingency_health_regen", "1", FCVAR_NOTIFY, "Toggles player health regeneration functionality" );
	ConVar contingency_health_regen_delay( "contingency_health_regen_delay", "1", FCVAR_NOTIFY, "Defines the amount of time (in seconds) before players are granted additional health" );
	ConVar contingency_health_regen_amount( "contingency_health_regen_amount", "1", FCVAR_NOTIFY, "Defines the amount of additional health granted to players upon regeneration" );

	// Added support wave system
	ConVar contingency_wave_support( "contingency_wave_support", "1", FCVAR_NOTIFY, "Toggles the spawning support NPCs during interim phases when server isn't full" );

	// Added wave system
	ConVar contingency_wave_multiplier_arbitrary( "contingency_wave_multiplier_arbitrary", "5", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by based on nothing (i.e. a magic number)" );
	ConVar contingency_wave_multiplier_players( "contingency_wave_multiplier_players", "0.125", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by based on the number of players" );
	//ConVar contingency_wave_multiplier_headcrabs( "contingency_wave_multiplier_headcrabs", "3", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by during headcrab waves" );
	ConVar contingency_wave_multiplier_antlions( "contingency_wave_multiplier_antlions", "1.25", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by during antlion waves" );
	//ConVar contingency_wave_multiplier_zombies( "contingency_wave_multiplier_zombies", "1", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by during zombie waves" );
	ConVar contingency_wave_multiplier_zombies( "contingency_wave_multiplier_zombies", "1.5", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by during zombie waves" ); //increased the number spawned to make room for adding headcrabs to the wave
	ConVar contingency_wave_multiplier_combine( "contingency_wave_multiplier_combine", "0.75", FCVAR_NOTIFY, "Defines the amount to scale the amount of NPCs spawned by during combine waves" );
#else
	ConVar contingency_client_heartbeatsounds( "contingency_client_heartbeatsounds", "1", FCVAR_ARCHIVE, "Toggles heartbeat sounds when health is low" );

	ConVar contingency_client_weaponhints( "contingency_client_weaponhints", "1", FCVAR_ARCHIVE, "Toggles display of weapon hints on HUD" );

	// Added loadout system
	ConVar contingency_client_preferredprimaryweapon( "contingency_client_preferredprimaryweapon", "weapon_smg1", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Defines the classname of the preferred primary weapon to use" );
	ConVar contingency_client_preferredsecondaryweapon( "contingency_client_preferredsecondaryweapon", "weapon_pistol", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Defines the classname of the preferred secondary weapon to use" );
	ConVar contingency_client_preferredmeleeweapon( "contingency_client_preferredmeleeweapon", "weapon_crowbar", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Defines the classname of the preferred melee weapon to use" );
	ConVar contingency_client_preferredequipment( "contingency_client_preferredequipment", "weapon_frag", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Defines the classname of the preferred equipment to use" );
	ConVar contingency_client_updateloadout( "contingency_client_updateloadout", "0", FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE, "Requests a manual update to the preferred loadout" );
#endif

// Added wave system
ConVar contingency_wave_challenge_frequency( "contingency_wave_challenge_frequency", "5", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines the frequency of challenge waves (i.e. defines x, where every x wave is a challenge wave)" );

#ifdef CLIENT_DLL
	void RecvProxy_ContingencyRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CContingencyRules *pRules = ContingencyRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CContingencyRulesProxy, DT_ContingencyRulesProxy )
		RecvPropDataTable( "contingency_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_ContingencyRules ), RecvProxy_ContingencyRules )
	END_RECV_TABLE()
#else
	void* SendProxy_ContingencyRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CContingencyRules *pRules = ContingencyRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CContingencyRulesProxy, DT_ContingencyRulesProxy )
		SendPropDataTable( "contingency_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_ContingencyRules ), SendProxy_ContingencyRules )
	END_SEND_TABLE()
#endif

CContingencyRules::CContingencyRules()
{
#ifndef CLIENT_DLL
	// Move AI relationship tables to CContingencyRules
	InitDefaultAIRelationships();

	ResetPhaseVariables();

	m_iRestartDelay = 0;

	// Added phase system
	m_iInterimPhaseLength = 60;

	// Added radar display
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	m_bMapAllowsRadars = true;

	// Added spawnable prop system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	m_iMapMaxPropsPerPlayer = 50;

	// Added credits system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	m_iMapStartingCredits = 3;

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	m_iMapMaxLivingNPCs = 30;
	m_bMapHeadcrabSupport = true;
	m_bMapAntlionSupport = true;
	m_bMapZombieSupport = true;
	m_bMapCombineSupport = true;

	// Added wave system
	// This information is updated by a contingency_configuration entity (if one exists) when it spawns
	m_flMapHeadcrabWaveMultiplierOffset = 0.0f;
	m_flMapAntlionWaveMultiplierOffset = 0.0f;
	m_flMapZombieWaveMultiplierOffset = 0.0f;
	m_flMapCombineWaveMultiplierOffset = 0.0f;

	// Added wave system
	SetCurrentWaveNPCList( new CUtlVector<CAI_BaseNPC*>() );

	// Added support wave system
	SetCurrentSupportWaveNPCList( new CUtlVector<CAI_BaseNPC*>() );
#endif
}
	
CContingencyRules::~CContingencyRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();

	// Added a non-restorative health system
	m_PlayerInfoList.PurgeAndDeleteElements();

	// Added wave system
	delete m_pCurrentWaveNPCList;

	// Added support wave system
	delete m_pCurrentSupportWaveNPCList;
#endif
}

#ifndef CLIENT_DLL
void CContingencyRules::ResetPhaseVariables( void )
{
	// Added phase system
	SetCurrentPhase( PHASE_WAITING_FOR_PLAYERS );
	SetInterimPhaseTimeLeft( 0 );
	m_flInterimPhaseTime = 0.0f;

	// Added wave system
	SetWaveNumber( 0 );	// wave number is set to 1 after the first interim phase (i.e. this is intentional)
	SetWaveType( WAVE_NONE );
	SetPreferredWaveType( WAVE_NONE );
	SetPreferredNPCType( "" );
	SetNumEnemiesRemaining( 0 );
	SetCalculatedNumEnemies( 0 );
	SetNumEnemiesSpawned( 0 );
	m_bPlayersDefeated = false;
}
#endif

void CContingencyRules::CreateStandardEntities( void )
{
#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "contingency_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

void CContingencyRules::Precache( void )
{
	BaseClass::Precache();
}

#ifndef CLIENT_DLL
// Precaching called by each player (server-side)
// This is in gamerules because all the precaching done here is really gameplay-dependent
void CContingencyRules::PrecacheStuff( void )
{
	// Added rain splash particles to func_precipitation
	PrecacheParticleSystem( "rainsplash" );

	int i;

	// Added wave system
	/*for ( i = 0; i < NUM_HEADCRAB_NPCS; i++ )
		UTIL_PrecacheOther( kWaveHeadcrabsNPCTypes[i] );*/
	for ( i = 0; i < NUM_ANTLION_NPCS; i++ )
		UTIL_PrecacheOther( kWaveAntlionsNPCTypes[i] );
	for ( i = 0; i < NUM_ZOMBIE_NPCS; i++ )
		UTIL_PrecacheOther( kWaveZombiesNPCTypes[i] );
	for ( i = 0; i < NUM_COMBINE_NPCS; i++ )
		UTIL_PrecacheOther( kWaveCombineNPCTypes[i] );

	// Added spawnable prop system
	for ( i = 0; i < NUM_SPAWNABLEPROP_TYPES; i++ )
		CBaseEntity::PrecacheModel( kSpawnablePropTypes[i][3] );
	
	// Added spawnable prop system
	// Because at least some (currently all) of our spawnable props are breakable,
	// we need to precache all the possible pieces of shit that each can break down into
	// NOTE: If additional spawnable props are added that are breakable,
	// this list will likely have to be expanded, so REMEMBER THAT (just check console for warnings)!
	// TODO: Organize this and just generally make it all neater!
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04c.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_splinters01a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_splinters01b.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_splinters01c.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_splinters01d.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_splinters01e.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04d.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04b.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04e.mdl" );
	CBaseEntity::PrecacheModel( "models/props_debris/wood_chunk04f.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunka.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunka4.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunkb2.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunka1.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_shard01.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunka3.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_pallet001a_chunkb3.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furniturechair001a_chunk01.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_shard01.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furniturechair001a_chunk02.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furniturechair001a_chunk03.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk01.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk02.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_shard01.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk03.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk07.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk08.mdl" );
	CBaseEntity::PrecacheModel( "models/gibs/furniture_gibs/furnituretable002a_chunk11.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk01.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk02.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk03.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk04.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk05.mdl" );
	CBaseEntity::PrecacheModel( "models/props_c17/furnituredrawer001a_chunk06.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk01.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk02.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk03.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk04.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk05.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk07.mdl" );
	CBaseEntity::PrecacheModel( "models/props_junk/wood_crate001a_chunk09.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_shard01a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board01a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board05a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board03a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board07a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board09a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board10a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board04a.mdl" );
	CBaseEntity::PrecacheModel( "models/props_wasteland/wood_fence02a_board08a.mdl" );

	// Added support wave system
	for ( i = 0; i < NUM_SUPPORT_NPCS; i++ )
		UTIL_PrecacheOther( kSupportWaveSupportNPCTypes[i] );

	// Added boss system
	// TODO: Move boss NPC classnames over to a dedicated array or something
	// so we don't have to go changing this every time we add/remove bosses
	UTIL_PrecacheOther( "npc_antlionguard" );

	// Added a modified version of Valve's floor turret
	UTIL_PrecacheOther( "npc_turret_floor" );
}
#endif

bool CContingencyRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	// Added shout system
	if ( FStrEq(args[0], "menuselect") )
	{
		CContingency_Player *pPlayer = ToContingencyPlayer( pEdict );
		if ( !pPlayer )
			return true;

		if ( pPlayer->IsShowingShoutMenu() )
		{
			int choice = atoi( args.Arg(1) );
			const char *playerName = pPlayer->GetPlayerName();

			switch ( choice )
			{
			case 1:
				pPlayer->EmitSound( "Male.Incoming" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Incoming!'", playerName );
				break;

			case 2:
				pPlayer->EmitSound( "Male.Run" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Run!'", playerName );
				break;

			case 3:
				pPlayer->EmitSound( "Male.Go" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Let's go!'", playerName );
				break;

			case 4:
				pPlayer->EmitSound( "Male.Lead" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Lead the way!'", playerName );
				break;

			case 5:
				pPlayer->EmitSound( "Male.Cover" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Take cover!'", playerName );
				break;

			case 6:
				pPlayer->EmitSound( "Male.Ready" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Ready!'", playerName );
				break;

			case 7:
				pPlayer->EmitSound( "Male.Headcrabs" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Headcrabs!'", playerName );
				break;

			case 8:
				pPlayer->EmitSound( "Male.Zombies" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Zombies!'", playerName );
				break;

			case 9:
				pPlayer->EmitSound( "Male.Combine" );
				pPlayer->SetShoutDelay( gpGlobals->curtime + 5.0f );
				UTIL_ClientPrintAll( HUD_PRINTTALK, "%s1 says 'Combine!'", playerName );
				break;
			}
		}

		pPlayer->ShouldShowShoutMenu( false );
		return true;
	}
#endif

	if( BaseClass::ClientCommand(pEdict, args) )
		return true;

	return false;
}

// Added loadout system
// Prevent players from being able to change their player models
void CContingencyRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	CContingency_Player *pContingencyPlayer = ToContingencyPlayer( pPlayer );
	if ( !pContingencyPlayer )
		return;

	if ( Q_stricmp(engine->GetClientConVarValue(engine->IndexOfEdict(pContingencyPlayer->edict()), "contingency_client_updateloadout"), "1") == 0 )
	{
		// This player's loadout needs to be updated according to
		// their preferred weapons/equipment, so do so

		// Loadouts are not applied right away, but rather at the start of a combat phase
		// or when the player respawns (whatever comes first)

		ClientPrint( pContingencyPlayer, HUD_PRINTTALK, "Loadout saved. Any changes will be applied at the start of the next combat phase." );

		pContingencyPlayer->IsMarkedForLoadoutUpdate( true );

		// Update successful, so update the value of our ConVar to reflect this
		// This will in turn call this function again, but this block won't because of the first condition
		// Yes, this entire ConVar thing is a bit hacky and dumb, but it's the only one of my solutions that has actually worked so far
		engine->ClientCommand( pContingencyPlayer->edict(), "contingency_client_updateloadout 0" );
	}

	if ( sv_report_client_settings.GetInt() == 1 )
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pContingencyPlayer->GetPlayerName(), engine->GetClientConVarValue(pContingencyPlayer->entindex(), "cl_cmdrate") );

	CTeamplayRules::ClientSettingsChanged( pContingencyPlayer );	// skip over CHL2MPRules::ClientSettingsChanged
#endif
}

void CContingencyRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	// Added a non-restorative health system
	CContingency_Player *pPlayer = ToContingencyPlayer( CBaseEntity::Instance(pClient) );
	if ( pPlayer )
	{
		const char *steamID = engine->GetPlayerNetworkIDString( pClient );
		CContingency_Player_Info *pPlayerInfo = FindPlayerInfoBySteamID( steamID );
		if ( pPlayerInfo )
		{
			// We've found a player info entry with our player's steamID in it,
			// so update that entry with our player's new information
			pPlayerInfo->HasBeenAccessed( false );
			if ( !pPlayer->IsAlive() )
				pPlayerInfo->SetHealth( 0 );
			else
				pPlayerInfo->SetHealth( pPlayer->GetHealth() );
			
			// Added credits system
			// Save player's credits
			pPlayerInfo->SetCredits( pPlayer->GetCredits() );

			// Added spawnable prop system
			// Save a reference to our spawnable props (via a list)
			pPlayerInfo->SetNumSpawnableProps( pPlayer->GetNumSpawnableProps() );
			pPlayerInfo->spawnablePropList = pPlayer->m_SpawnablePropList;

			// Added a modified version of Valve's floor turret
			// Save a reference to our deployed turret (if any)
			pPlayerInfo->SetDeployedTurret( pPlayer->GetDeployedTurret() );
		}
		else
		{
			// We don't have a player info for this player yet,
			// so create one and add it to our list
			pPlayerInfo = new CContingency_Player_Info();
			if ( pPlayerInfo )
			{
				pPlayerInfo->HasBeenAccessed( false );
				pPlayerInfo->SetSteamID( steamID );
				if ( !pPlayer->IsAlive() )
					pPlayerInfo->SetHealth( 0 );
				else
					pPlayerInfo->SetHealth( pPlayer->GetHealth() );

				// Added credits system
				// Save player's credits
				pPlayerInfo->SetCredits( pPlayer->GetCredits() );

				// Added spawnable prop system
				// Save a reference to our spawnable prop list
				pPlayerInfo->spawnablePropList = pPlayer->m_SpawnablePropList;

				// Added a modified version of Valve's floor turret
				// Save a reference to our deployed turret (if any)
				pPlayerInfo->SetDeployedTurret( pPlayer->GetDeployedTurret() );

				m_PlayerInfoList.AddToTail( pPlayerInfo );
			}
		}
	}

	BaseClass::ClientDisconnected( pClient );
#endif
}

// Added wave system
#ifndef CLIENT_DLL
void CContingencyRules::PerformWaveCalculations( void )
{
	// See what wave types our current map supports via our custom contingency_configuration entity
	// (if a map doesn't have one, then we're assuming it supports all wave types)
	if ( /*!DoesMapSupportHeadcrabs() && */!DoesMapSupportAntlions() && !DoesMapSupportZombies() && !DoesMapSupportCombine() )
	{
		// The current map does not appear to support any type of wave
		// This isn't allowed, so just pretend we support all of them

		Warning("The current map is set not to allow any waves. This is not allowed, and to prevent the game from breaking, all waves have been enabled. Please check your contingency_configuration entity flags!\n");

		//DoesMapSupportHeadcrabs( true );
		DoesMapSupportAntlions( true );
		DoesMapSupportZombies( true );
		DoesMapSupportCombine( true );
	}

	int waveSelected = GetPreferredWaveType();	// allow server to specify preferred wave type

	if ( waveSelected == WAVE_NONE )
	{
		waveSelected = random->RandomInt( WAVE_NONE + 1, NUM_WAVES - 1 );
		while ( /*((waveSelected == WAVE_HEADCRABS) && !DoesMapSupportHeadcrabs()) ||*/
				((waveSelected == WAVE_ANTLIONS) && !DoesMapSupportAntlions()) ||
				((waveSelected == WAVE_ZOMBIES) && !DoesMapSupportZombies()) ||
				((waveSelected == WAVE_COMBINE) && !DoesMapSupportCombine()) )
			waveSelected = random->RandomInt( WAVE_NONE + 1, NUM_WAVES - 1 );

		SetWaveType( waveSelected );
	}

	SetWaveType( waveSelected );

	// Start by factoring in the current wave number
	int numEnemiesToSpawnThisWave = contingency_wave_multiplier_arbitrary.GetFloat() * GetWaveNumber();
	
	// Next, we consider the wave type that's been selected
	switch ( waveSelected )
	{
	/*case WAVE_HEADCRABS:
		numEnemiesToSpawnThisWave = numEnemiesToSpawnThisWave *
			(contingency_wave_multiplier_headcrabs.GetFloat() + GetMapHeadcrabWaveMultiplierOffset());
		break;*/
	case WAVE_ANTLIONS:
		numEnemiesToSpawnThisWave *= contingency_wave_multiplier_antlions.GetFloat() + GetMapAntlionWaveMultiplierOffset();
		break;
	case WAVE_ZOMBIES:
		numEnemiesToSpawnThisWave *= contingency_wave_multiplier_zombies.GetFloat() + GetMapZombieWaveMultiplierOffset();
		break;
	case WAVE_COMBINE:
		numEnemiesToSpawnThisWave *= contingency_wave_multiplier_combine.GetFloat() + GetMapCombineWaveMultiplierOffset();
		break;
	}

	// Lastly, we consider how many players there are
	numEnemiesToSpawnThisWave *= 1 - ((GetMaxNumPlayers() - GetTotalNumPlayers()) * contingency_wave_multiplier_players.GetFloat());

	// Handle challenge waves
	if ( IsChallengeWave() )
	{
		// Firstly, we need to figure out exactly what NPCs we can and cannot spawn on this map
		// to prevent us from picking an NPC type for our challenge wave that we
		// cannot actually spawn, which would throw the whole game out of whack

		// We do this by looping through all of the wave spawnpoints on this map,
		// seeing what NPC types each one supports, where NPC types that are not supported
		// by any wave spawnpoints should be ignored with regards to our challenge wave "drawing"

		bool bConsiderHeadcrab = false;
		bool bConsiderHeadcrabFast = false;
		bool bConsiderHeadcrabBlack = false;
		bool bConsiderAntlion = false;
		bool bConsiderZombie = false;
		bool bConsiderZombieTorso = false;
		bool bConsiderZombieFast = false;
		bool bConsiderZombiePoison = false;
		bool bConsiderCombine = false;
		bool bConsiderCombineMetro = false;
		bool bConsiderCombineScanner = false;
		bool bConsiderCombineManhack = false;
		bool bConsiderCombineStalker = false;
		CBaseEntity *pEntity = gEntList.FindEntityByClassname( NULL, "contingency_wave_spawnpoint" );
		while ( pEntity != NULL )
		{
			/*if ( waveSelected == WAVE_HEADCRABS )
			{
				if ( !bConsiderHeadcrab && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB) )
					bConsiderHeadcrab = true;

				if ( !bConsiderHeadcrabFast && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_FAST) )
					bConsiderHeadcrabFast = true;

				if ( !bConsiderHeadcrabBlack && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_BLACK) )
					bConsiderHeadcrabBlack = true;
			}
			else */if ( waveSelected == WAVE_ANTLIONS )
			{
				if ( !bConsiderAntlion && pEntity->HasSpawnFlags(SF_WAVESPAWNER_ANTLION) )
					bConsiderAntlion = true;
			}
			else if ( waveSelected == WAVE_ZOMBIES )
			{
				if ( !bConsiderHeadcrab && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB) )
					bConsiderHeadcrab = true;

				if ( !bConsiderHeadcrabFast && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_FAST) )
					bConsiderHeadcrabFast = true;

				if ( !bConsiderHeadcrabBlack && pEntity->HasSpawnFlags(SF_WAVESPAWNER_HEADCRAB_BLACK) )
					bConsiderHeadcrabBlack = true;

				if ( !bConsiderZombie && pEntity->HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE) )
					bConsiderZombie = true;

				if ( !bConsiderZombieTorso && pEntity->HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_TORSO) )
					bConsiderZombieTorso = true;

				if ( !bConsiderZombieFast && pEntity->HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_FAST) )
					bConsiderZombieFast = true;

				if ( !bConsiderZombiePoison && pEntity->HasSpawnFlags(SF_WAVESPAWNER_ZOMBIE_POISON) )
					bConsiderZombiePoison = true;
			}
			else if ( waveSelected == WAVE_COMBINE )
			{
				if ( !bConsiderCombine && pEntity->HasSpawnFlags(SF_WAVESPAWNER_COMBINE) )
					bConsiderCombine = true;

				if ( !bConsiderCombineMetro && pEntity->HasSpawnFlags(SF_WAVESPAWNER_COMBINE_METRO) )
					bConsiderCombineMetro = true;

				if ( !bConsiderCombineScanner && pEntity->HasSpawnFlags(SF_WAVESPAWNER_COMBINE_SCANNER) )
					bConsiderCombineScanner = true;

				if ( !bConsiderCombineManhack && pEntity->HasSpawnFlags(SF_WAVESPAWNER_COMBINE_MANHACK) )
					bConsiderCombineManhack = true;

				if ( !bConsiderCombineStalker && pEntity->HasSpawnFlags(SF_WAVESPAWNER_COMBINE_STALKER) )
					bConsiderCombineStalker = true;
			}
 
			pEntity = gEntList.FindEntityByClassname( pEntity, "contingency_wave_spawnpoint" );
		}

		// At this point, we implicitly know exactly what NPCs we can and cannot choose
		// to spawn for our challenge wave, we just need to act on that knowledge

		// We do this by mapping each of our boolean variables to an NPC classname,
		// adding those NPC types we are considering to a linked list and finally
		// randomly choosing an NPC classname from that linked list

		CUtlLinkedList<const char*, unsigned short> *pConsiderationList = new CUtlLinkedList<const char*, unsigned short>;
		
		if ( bConsiderHeadcrab )
			pConsiderationList->AddToTail( "npc_headcrab" );
		if ( bConsiderHeadcrabFast )
			pConsiderationList->AddToTail( "npc_headcrab_fast" );
		if ( bConsiderHeadcrabBlack )
			pConsiderationList->AddToTail( "npc_headcrab_black" );
		if ( bConsiderAntlion )
			pConsiderationList->AddToTail( "npc_antlion" );
		if ( bConsiderZombie )
			pConsiderationList->AddToTail( "npc_zombie" );
		if ( bConsiderZombieTorso )
			pConsiderationList->AddToTail( "npc_zombie_torso" );
		if ( bConsiderZombieFast )
			pConsiderationList->AddToTail( "npc_fastzombie" );
		if ( bConsiderZombiePoison )
			pConsiderationList->AddToTail( "npc_poisonzombie" );
		if ( bConsiderCombine )
			pConsiderationList->AddToTail( "npc_combine_s" );
		if ( bConsiderCombineMetro )
			pConsiderationList->AddToTail( "npc_metropolice" );
		if ( bConsiderCombineScanner )
			pConsiderationList->AddToTail( "npc_cscanner" );
		if ( bConsiderCombineManhack )
			pConsiderationList->AddToTail( "npc_manhack" );
		if ( bConsiderCombineStalker )
			pConsiderationList->AddToTail( "npc_stalker" );
		
		SetPreferredNPCType( pConsiderationList->Element(random->RandomInt(0, pConsiderationList->Count() - 1)) );

		delete pConsiderationList;	// we're done with this now

		// All in all, this might not be the cleanest solution, but it works!
		// ...and actually, I think it's pretty good. :D
	}

	// Update wave-specific variables and stuff to reflect the calculations made above
	SetNumEnemiesRemaining( numEnemiesToSpawnThisWave );
	SetCalculatedNumEnemies( numEnemiesToSpawnThisWave );
	SetNumEnemiesSpawned( 0 );
}

// Added wave system
void CContingencyRules::HandleNPCDeath( CAI_BaseNPC *pNPC, const CTakeDamageInfo &info )
{
	if ( !pNPC )
		return;	// just in case, I guess

	// Make sure this NPC is part of our current wave
	if ( ContingencyRules() && GetCurrentWaveNPCList() && (GetCurrentWaveNPCList()->Find(pNPC) != -1) )
	{
		// Give players credit for killing enemy NPCs
		CContingency_Player *pPlayer = ToContingencyPlayer( info.GetAttacker() );
		if ( pPlayer )
		{
			// Added credits system
			// TODO: Move boss NPC classnames over to a dedicated array or something
			// so we don't have to go changing this every time we add/remove bosses
			if ( Q_strcmp(pNPC->GetClassname(), "npc_antlionguard") == 0 )
				pPlayer->AddCredits( 5 );
			else
				pPlayer->AddCredits( 1 );

			pPlayer->IncrementFragCount( 1 );
			GetGlobalTeam( pPlayer->GetTeamNumber() )->AddScore( 1 );
		}
	}
}

void CContingencyRules::HandleNPCRemoval( CAI_BaseNPC *pNPC )
{
	if ( !pNPC )
		return;	// just in case, I guess

	// Make sure this NPC is part of our current wave
	if ( ContingencyRules() && GetCurrentWaveNPCList() && (GetCurrentWaveNPCList()->Find(pNPC) != -1) )
	{
		// ...now players have one less enemy to worry about!
		GetCurrentWaveNPCList()->FindAndRemove( pNPC );
		DecrementNumEnemiesRemaining();
	}
	else if ( ContingencyRules() && GetCurrentSupportWaveNPCList() && (GetCurrentSupportWaveNPCList()->Find(pNPC) != -1) )
	{
		// If this NPC was a member of our support wave,
		// then remove them from our current support NPC list
		// seeing as they're gone now

		GetCurrentSupportWaveNPCList()->FindAndRemove( pNPC );
	}
}

#endif

void CContingencyRules::Think( void )
{
#ifndef CLIENT_DLL
	// Skip over CHL2MPRules::Think()
	CGameRules::Think();

	// Handle end of map level changes
	if ( g_fGameOver )
	{
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
			ChangeLevel();

		return;
	}
	
	// Handle end of map conditions
	// (These probably shouldn't be checked periodically)
	if ( GetMapRemainingTime() < 0 )
	{
		GoToIntermission();
		return;
	}

	// Handle end of map conditions
	// (These probably shouldn't be checked periodically)
	float flFragLimit = fraglimit.GetFloat();
	if ( flFragLimit )
	{
		int i;
		CBasePlayer *pPlayer;
		for ( i = 1; i <= gpGlobals->maxClients; i++ )
		{
			pPlayer = UTIL_PlayerByIndex( i );
			if ( !pPlayer )
				continue;

			if ( pPlayer->FragCount() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}
		}
	}

	// Added phase system
	if ( gpGlobals->curtime >= m_flInterimPhaseTime )
	{
		if ( GetCurrentPhase() == PHASE_INTERIM )
		{
			// At the end of an interim phase, a combat phase begins

			IncrementWaveNumber();
			PerformWaveCalculations();

			if ( IsChallengeWave() )
			{
				DisplayAnnouncement( UTIL_VarArgs("CHALLENGE WAVE %i COMMENCING...", GetWaveNumber()), 5.0f );
				CContingency_System_Music::PlayAnnouncementSound( "Contingency.InterimToCombatChallenge" );
			}
			else
			{
				DisplayAnnouncement( UTIL_VarArgs("WAVE %i COMMENCING...", GetWaveNumber()), 3.0f );
				CContingency_System_Music::PlayAnnouncementSound( "Contingency.InterimToCombat" );
			}

			CContingency_System_Music::PlayBackgroundMusic( BACKGROUND_MUSIC_COMBAT );

			SetCurrentPhase( PHASE_COMBAT );
		}
	}

	// Handle periodic conditions
	if ( gpGlobals->curtime >= m_tmNextPeriodicThink )
	{
		CheckRestartGame();

		// Added phase system
		if ( m_iInterimPhaseTimeLeft > 0 )
			m_iInterimPhaseTimeLeft = m_iInterimPhaseTimeLeft - 1;
		else if ( m_iInterimPhaseTimeLeft < 0 )
			m_iInterimPhaseTimeLeft = 0;	// make sure we don't go lower than zero...ever...
											// ...just in case?

		// Added wave system
		if ( (GetCurrentPhase() == PHASE_COMBAT) && !m_bPlayersDefeated )
		{
			if ( GetNumPlayingPlayers() <= 0 )
			{
				// Awwww...our players have been defeated by this wave...
				// TOO BAD! Schedule a game restart.
				
				DisplayAnnouncement( UTIL_VarArgs("YOUR TEAM WAS WIPED OUT!\nYou made it to wave %i.\nThe game will reset shortly.", GetWaveNumber()) );

				// TODO: Good place for a sound cue of some kind...

				m_iRestartDelay = 10;

				m_bPlayersDefeated = true;
			}
			else if ( GetNumEnemiesRemaining() <= 0 )
			{
				// Our players have managed to kill all of the NPCs
				// associated with this wave, which means it's time
				// for an interm phase and some words of encouragement

				DisplayAnnouncement( UTIL_VarArgs("WAVE %i CLEARED!\nInterim phase is now active for %i seconds.", GetWaveNumber(), GetMapInterimPhaseLength()), 5.0f );

				CContingency_System_Music::PlayAnnouncementSound( "Contingency.CombatToInterim" );
				CContingency_System_Music::PlayBackgroundMusic( BACKGROUND_MUSIC_INTERIM );

				SetCurrentPhase( PHASE_INTERIM );
			}
		}

		m_tmNextPeriodicThink = gpGlobals->curtime + 1.0;
	}

	// Handle game restart conditions
	if ( (m_flRestartGameTime > 0.0f) && (m_flRestartGameTime <= gpGlobals->curtime) )
		RestartGame();

	// I don't know what this does...
	// It was in CHL2MPRules::Think, so I'm keeping it around
	// until we have a specific reason to remove it
	ManageObjectRelocation();
#endif
}

void CContingencyRules::CheckRestartGame( void )
{
	// Added phase system
	// If there are no players on the server, restart the game and switch to our dummy phase
	// If there are players on the server, restart the game and switch out of our dummy phase
#ifndef CLIENT_DLL
	if ( ((GetTotalNumPlayers() <= 0) && (GetCurrentPhase() != PHASE_WAITING_FOR_PLAYERS)) ||
		 ((GetTotalNumPlayers() > 0) && (GetCurrentPhase() == PHASE_WAITING_FOR_PLAYERS)) )
	{
		m_flRestartGameTime = gpGlobals->curtime;	// restart right away
		m_bCompleteReset = true;
		m_iRestartDelay = 0;	// cancel any pending timed restarts in favor of an immediate one
		return;
	}
#endif

	// Restart the game if specified by the server
	if ( m_iRestartDelay > 0 )
	{
		if ( m_iRestartDelay > 60 )
			m_iRestartDelay = 60;	// clamp @ 60 seconds (is this necessary?)

		m_flRestartGameTime = gpGlobals->curtime + m_iRestartDelay;
		m_bCompleteReset = true;
		m_iRestartDelay = 0;
	}
}

#ifndef CLIENT_DLL
void CContingencyRules::RestartGame()
{
	// Out-of-bounds check
	if ( mp_timelimit.GetInt() < 0 )
		mp_timelimit.SetValue( 0 );

	// Out-of-bounds check (?)
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	// Added a non-restorative health system
	m_PlayerInfoList.PurgeAndDeleteElements();	// purge all player infos when the game is restarted

	// Pre-cleanup stuff:

	int i;
	CContingency_Player *pPlayer;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex(i) );
		if ( !pPlayer )
			continue;

		// Added credits system
		pPlayer->ResetCredits();

		// Added prop spawning system
		pPlayer->m_SpawnablePropList.PurgeAndDeleteElements();	// remove all of the player's spawnable props
		pPlayer->SetNumSpawnableProps( 0 );

		if ( pPlayer->IsInAVehicle() )
			pPlayer->LeaveVehicle();

		QAngle angles = pPlayer->GetLocalAngles();

		angles.x = 0;
		angles.z = 0;

		pPlayer->SetLocalAngles( angles );

		CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pPlayer->GetActiveWeapon();
		if ( pWeapon )
		{
			pPlayer->Weapon_Detach( pWeapon );
			UTIL_Remove( pWeapon );
		}

		pPlayer->RemoveAllItems( true );
		pPlayer->ClearActiveWeapon();

		pPlayer->Reset();
	}

	CleanUpMap();

	// Post-cleanup stuff:

	// Added phase system
	// If there are no players on the server, restart the game and switch to our dummy phase
	// If there are players on the server, restart the game and switch out of our dummy phase
	if ( GetTotalNumPlayers() <= 0 )
		ResetPhaseVariables();
	else
		SetCurrentPhase( PHASE_INTERIM );

	// Added wave system
	SetWaveNumber( 0 );	// will be 1 after the first interim phase

	m_flIntermissionEndTime = 0;
	m_flRestartGameTime = 0.0;
	m_bCompleteReset = false;

	IGameEvent *event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt( "fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString( "objective", "CONTINGENCY" );

		gameeventmanager->FireEvent( event );
	}

	// Stop any left-over sounds now before it gets out of hand...
	engine->ServerCommand( "snd_restart\n" );
}
#endif

// Do not allow players to hurt each other
int CContingencyRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	if ( !pPlayer || !pTarget )
		return GR_NOTTEAMMATE;	// null pointer(s), but we have to return something...

	if ( pPlayer == pTarget )
		return GR_NOTTEAMMATE;	// allow things to hurt themselves

	if ( pPlayer->IsPlayer() && pTarget->IsPlayer() )
		return GR_TEAMMATE;	// don't allow players to hurt other players

	// Citizens and turrets are players' friends!
	// TODO: Is this even used?!
#ifdef CLIENT_DLL
		if ( pPlayer->IsPlayer() && (dynamic_cast<C_NPC_Citizen*>(pTarget) || dynamic_cast<C_NPC_FloorTurret*>(pTarget)) )
#else
		if ( pPlayer->IsPlayer() && (dynamic_cast<CNPC_Citizen*>(pTarget) || dynamic_cast<CNPC_FloorTurret*>(pTarget)) )
#endif
			return GR_TEAMMATE;
#ifdef CLIENT_DLL
		if ( pTarget->IsPlayer() && (dynamic_cast<C_NPC_Citizen*>(pPlayer) || dynamic_cast<C_NPC_FloorTurret*>(pPlayer)) )
#else
		if ( pTarget->IsPlayer() && (dynamic_cast<CNPC_Citizen*>(pPlayer) || dynamic_cast<CNPC_FloorTurret*>(pPlayer)) )
#endif
			return GR_TEAMMATE;

	return GR_NOTTEAMMATE;	// everything else is baaaaaad
}

// Do not allow players to hurt each other
bool CContingencyRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if ( !pPlayer || !pAttacker )
		return false;	// null pointer(s), but we have to return something...

	// Whether or not the player can take damage from an attacker
	// should depend strictly on the player and attacker's relationship
	return !PlayerRelationship( pAttacker, pPlayer );
}

bool CContingencyRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
		swap( collisionGroup0, collisionGroup1 );	// swap so that lowest is always first

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		 (collisionGroup1 == COLLISION_GROUP_PLAYER || collisionGroup1 == COLLISION_GROUP_PLAYER_MOVEMENT) )
		 return false;	// prevent player-player collisions

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

// Moved ammo definitions to contingency_gamerules.cpp

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	static bool balanced = true;

	if ( !bInitted )
	{
		bInitted = true;

//===================		
// AI Patch Addition.
//===================

		// Give players unlimited ammo
		// This is merely an illusion, but it works
		// The "99999"s are all the changes I've made in this department

		// Other unrelated changes have probably been made here too...
		// (e.g. reduced the effectiveness of all projectile weapons when wieled by an NPC...most have been restored to their HL2 defaults)
		// Player damage for most weapons have also been restored to their HL2 defaults (we are dealing with NPCs afterall)

		// OBSOLETE: Remember: player damage is defined in weapon scripts, NOT here (hence most if not all values for "plr dmg" below being '0')
//																								plr dmg		npc dmg	max carry	impulse
		if (balanced)
		{
		def.AddAmmoType("AR2",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_ar2",	"sk_npc_dmg_ar2", "sk_max_ar2", BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			"sk_plr_dmg_ar2_altfire", "sk_npc_dmg_ar2_altfire", "sk_max_ar2_altfire", 0, 0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_pistol", "sk_npc_dmg_pistol", "sk_max_pistol", BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_smg1",	"sk_npc_dmg_smg1",	"sk_max_smg1", BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_357", "sk_npc_dmg_357",	"sk_max_357", BULLET_IMPULSE(800, 5000), 0 );
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_crossbow", "sk_npc_dmg_crossbow", "sk_max_crossbow", BULLET_IMPULSE(800, 8000), 0 );
		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			"sk_plr_dmg_buckshot", "sk_npc_dmg_buckshot", "sk_max_buckshot", BULLET_IMPULSE(400, 1200), 0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_rpg_round", "sk_npc_dmg_rpg_round", "sk_max_rpg_round", 0, 0 );
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_smg1_grenade", "sk_npc_dmg_smg1_grenade", "sk_max_smg1_grenade", 0, 0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0 );
		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_satchel", "sk_npc_dmg_satchel", "sk_max_satchel", 0, 0 );
		
		// Added a modified version of Valve's floor turret
		// New ammo type for turrets
		def.AddAmmoType("TURRET",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_turret", "sk_npc_dmg_turret", "sk_max_turret", BULLET_IMPULSE(200, 1225),	0 );	// based on the AR2 ammo type

		def.AddAmmoType("hopwire",			DMG_BLAST,					TRACER_NONE,			0,			0,		1,			0,							0 );

		def.AddAmmoType("AlyxGun",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_alyxgun",		"sk_npc_dmg_alyxgun",		"sk_max_alyxgun",		BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("SniperRound",		DMG_BULLET | DMG_SNIPER,	TRACER_NONE,			"sk_plr_dmg_sniper_round",	"sk_npc_dmg_sniper_round",	"sk_max_sniper_round",	BULLET_IMPULSE(650, 6000), 0 );
		def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE,			"sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE(150, 6000), 0 );
		def.AddAmmoType("Thumper",			DMG_SONIC,					TRACER_NONE,			10, 10, 2, 0, 0 );
		def.AddAmmoType("Gravity",			DMG_CLUB,					TRACER_NONE,			0,	0, 8, 0, 0 );
		def.AddAmmoType("Battery",			DMG_CLUB,					TRACER_NONE,			NULL, NULL, NULL, 0, 0 );
		def.AddAmmoType("GaussEnergy",		DMG_SHOCK,					TRACER_NONE,			"sk_jeep_gauss_damage",		"sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE(650, 8000), 0 ); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType("CombineCannon",	DMG_BULLET,					TRACER_LINE,			"sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5 * 750 * 12, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType("AirboatGun",		DMG_AIRBOAT,				TRACER_LINE,			"sk_plr_dmg_airboat",		"sk_npc_dmg_airboat",		NULL,					BULLET_IMPULSE(10, 600), 0 );
		def.AddAmmoType("StriderMinigun",	DMG_BULLET,					TRACER_LINE,			5, 15,15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("StriderMinigunDirect",	DMG_BULLET,				TRACER_LINE,			2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("HelicopterGun",	DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter",	"sk_max_smg1",	BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );
#ifdef HL2_EPISODIC
		//def.AddAmmoType("Hopwire",			DMG_BLAST,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_hopwire",		0, 0);
		def.AddAmmoType("CombineHeavyCannon",	DMG_BULLET,				TRACER_LINE,			40,	40, NULL, 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType("ammo_proto1",			DMG_BULLET,				TRACER_LINE,			0, 0, 10, 0, 0 );
#endif // HL2_EPISODIC
		}

		else
		{
		def.AddAmmoType("AR2",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			3,		99999,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			0,			50,		1,			0,							0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			3,		99999,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			3,		99999,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			30,		99999,		BULLET_IMPULSE(800, 5000),	0 );
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			0,			10,		99999,		BULLET_IMPULSE(800, 8000),	0 );
		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			0,			3,		99999,		BULLET_IMPULSE(400, 1200),	0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			0,			50,		3,			0,							0 );
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			0,			50,		1,			0,							0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			0,			75,		3,			0,							0 );
		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			0,			50,		3,			0,							0 );
		
		// Added a modified version of Valve's floor turret
		// New ammo type for turrets
		def.AddAmmoType("TURRET",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_turret", "sk_npc_dmg_turret", "sk_max_turret", BULLET_IMPULSE(200, 1225),	0 );	// based on the AR2 ammo type

		def.AddAmmoType("hopwire",			DMG_BLAST,					TRACER_NONE,			0,			0,		1,			0,							0 );

		def.AddAmmoType("AlyxGun",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_alyxgun",		"sk_npc_dmg_alyxgun",		"sk_max_alyxgun",		BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("SniperRound",		DMG_BULLET | DMG_SNIPER,	TRACER_NONE,			"sk_plr_dmg_sniper_round",	"sk_npc_dmg_sniper_round",	"sk_max_sniper_round",	BULLET_IMPULSE(650, 6000), 0 );
		def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE,			"sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE(150, 6000), 0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_grenade",		0, 0);
		def.AddAmmoType("Thumper",			DMG_SONIC,					TRACER_NONE,			10, 10, 2, 0, 0 );
		def.AddAmmoType("Gravity",			DMG_CLUB,					TRACER_NONE,			0,	0, 8, 0, 0 );
		def.AddAmmoType("Battery",			DMG_CLUB,					TRACER_NONE,			NULL, NULL, NULL, 0, 0 );
		def.AddAmmoType("GaussEnergy",		DMG_SHOCK,					TRACER_NONE,			"sk_jeep_gauss_damage",		"sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE(650, 8000), 0 ); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType("CombineCannon",	DMG_BULLET,					TRACER_LINE,			"sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5 * 750 * 12, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType("AirboatGun",		DMG_AIRBOAT,				TRACER_LINE,			"sk_plr_dmg_airboat",		"sk_npc_dmg_airboat",		NULL,					BULLET_IMPULSE(10, 600), 0 );
		def.AddAmmoType("StriderMinigun",	DMG_BULLET,					TRACER_LINE,			5, 15,15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("StriderMinigunDirect",	DMG_BULLET,				TRACER_LINE,			2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("HelicopterGun",	DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter",	"sk_max_smg1",	BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );
#ifdef HL2_EPISODIC
		//def.AddAmmoType("Hopwire",			DMG_BLAST,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_hopwire",		0, 0);
		def.AddAmmoType("CombineHeavyCannon",	DMG_BULLET,				TRACER_LINE,			40,	40, NULL, 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType("ammo_proto1",			DMG_BULLET,				TRACER_LINE,			0, 0, 10, 0, 0 );
#endif // HL2_EPISODIC
		}
		//========== End Of AI Patch =====
	}

	return &def;
}

#ifndef CLIENT_DLL

// Move AI relationship tables to CContingencyRules

//==================
//AI Patch Addition
//==================

void CContingencyRules::InitDefaultAIRelationships( void )
{
	int i, j;

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// --------------------------------------------------------------
	// First initialize table so we can report missing relationships
	// --------------------------------------------------------------
	for (i=0;i<NUM_AI_CLASSES;i++)
	{
		for (j=0;j<NUM_AI_CLASSES;j++)
		{
			// By default all relationships are neutral of priority zero
			CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_NU, 0 );
		}
	}

	// ------------------------------------------------------------
	//	> CLASS_ANTLION
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PROTOSNIPER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ANTLION,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_BARNACLE
	//
	//  In this case, the relationship D_HT indicates which characters
	//  the barnacle will try to eat.
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BARNACLE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_EARTH_FAUNA,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_BULLSEYE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_BULLSQUID
	// ------------------------------------------------------------
	/*
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HEADCRAB,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HOUNDEYE,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
	*/
	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_PASSIVE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_HUNTER,	D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HEADCRAB,			D_FR, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HOUNDEYE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_VORTIGAUNT,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ZOMBIE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PROTOSNIPER,		D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_REBEL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER,			D_NU, 0);			
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_COMBINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_GUNSHIP
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MISSILE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_HUNTER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_CONSCRIPT
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
	
/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_FLARE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_HEADCRAB
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HACKED_ROLLERMINE,D_FR, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_HOUNDEYE
	// ------------------------------------------------------------
	/*
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSQUID,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HEADCRAB,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
	*/

	// ------------------------------------------------------------
	//	> CLASS_MANHACK
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HEADCRAB,			D_HT,-1);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,		CLASS_HOUNDEYE,			D_HT,-1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_METROPOLICE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_MILITARY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_MISSILE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_NONE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSEYE,			D_NU, 0);	
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CONSCRIPT,		D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_PLAYER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSEYE,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_REBEL,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_FLARE,			D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MILITARY,			D_HT, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PROTOSNIPER,		D_HT, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HEADCRAB,			D_FR, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ZOMBIE,			D_FR, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PROTOSNIPER,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_LI, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY_VITAL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BARNACLE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_REBEL,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_HUNTER,	D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_SCANNER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PROTOSNIPER,		D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HACKED_ROLLERMINE,D_LI, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_LI, 0 );

/////

	// ------------------------------------------------------------
    //	> CLASS_SCANNER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,		CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_HUNTER,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,		CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MANHACK,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_METROPOLICE,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MILITARY,			D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_SCANNER,			D_LI, 0);		
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_STALKER,			D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PROTOSNIPER,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_STALKER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,		CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_VORTIGAUNT
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BARNACLE,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,	CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_REBEL,	D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,	CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_SCANNER,			D_HT, 0);		
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HACKED_ROLLERMINE,D_LI, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_LI, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_ZOMBIE
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,		CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,		CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MANHACK,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MILITARY,			D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_PROTOSNIPER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER,			D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,		CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,		CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MISSILE,			D_NU, 5);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_VORTIGAUNT,		D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_EARTH_FAUNA
	//
	// Hates pretty much everything equally except other earth fauna.
	// This will make the critter choose the nearest thing as its enemy.
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_NONE,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,		CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_REBEL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_HUNTER,	D_HT, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CONSCRIPT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_FLARE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,		CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MANHACK,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MISSILE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_SCANNER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_STALKER,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_VORTIGAUNT,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ZOMBIE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PROTOSNIPER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_HT, 0 );

/////

	// ------------------------------------------------------------
	//	> CLASS_HACKED_ROLLERMINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER,			D_LI, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ANTLION,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSQUID,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HEADCRAB,			D_HT, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HOUNDEYE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_METROPOLICE,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MILITARY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_STALKER,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_VORTIGAUNT,		D_LI, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ZOMBIE,			D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_EARTH_FAUNA,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY,		D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

/////

	// Contingency - James
	// Added spawnable prop system

	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_LI, 0 );

/////

/////

	// Contingency - James
	// Added spawnable prop system

	// ------------------------------------------------------------
	//	> CLASS_CONTINGENCY_SPAWNABLE_PROP
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_NONE,				D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_PLAYER,			D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_ANTLION,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_BARNACLE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_BULLSEYE,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_BULLSQUID,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_CITIZEN_REBEL,	D_NU, 0);
    CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_COMBINE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_COMBINE_HUNTER,	D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_CONSCRIPT,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_FLARE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_HEADCRAB,			D_NU, 0);
	//CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_HOUNDEYE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_MANHACK,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_METROPOLICE,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_MILITARY,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_MISSILE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_SCANNER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_STALKER,			D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_VORTIGAUNT,		D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_ZOMBIE,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_PROTOSNIPER,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_EARTH_FAUNA,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_PLAYER_ALLY,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONTINGENCY_SPAWNABLE_PROP,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONTINGENCY_SPAWNABLE_PROP, CLASS_CONTINGENCY_SPAWNABLE_PROP, D_NU, 0 );

/////

}

//===== End Of AI Patch ======

#endif

//////////////////////
// HELPER FUNCTIONS //
//////////////////////
// (everything from here on down)

// These are functions that can function (no pun intended?)
// more or less independently and be called anytime without much fuss

bool CContingencyRules::IsPlayerPlaying( CContingency_Player *pPlayer )
{
	if ( !pPlayer )
		return false;	// null pointers? eww...

	if ( !pPlayer->IsAlive() || pPlayer->IsObserver() )
		return false;	// dead players aren't considered playing because they're more like spectators

	// All players are forced to TEAM_PLAYER when they spawn
	// and they should not be able to leave that team,
	// so we shouldn't need to check for that here anymore
	// (originally we did, hence this comment)

	return true;
}

#ifndef CLIENT_DLL
int CContingencyRules::GetTotalNumPlayers( void )
{
	int iNumPlayers = 0;

	int i;
	CBasePlayer *pClient;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pClient = UTIL_PlayerByIndex( i );

		if ( !pClient || !pClient->edict() )
			continue;

		if ( !pClient->IsNetClient() )
			continue;

		iNumPlayers++;
	}

	return iNumPlayers;
}

int CContingencyRules::GetNumPlayingPlayers( void )
{
	int iNumPlayers = 0;

	int i;
	CContingency_Player *pClient;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pClient = ToContingencyPlayer( UTIL_PlayerByIndex(i) );

		if ( !pClient || !pClient->edict() )
			continue;

		if ( !pClient->IsNetClient() )
			continue;

		if ( !IsPlayerPlaying(pClient) )
			continue;

		iNumPlayers++;
	}

	return iNumPlayers;
}

// Added loadout system
void CContingencyRules::UpdatePlayerLoadouts( void )
{
	int i;
	CContingency_Player *pPlayer;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer )
			continue;

		if ( !IsPlayerPlaying(pPlayer) )
			continue;	// dead players' loadouts are updated when they respawn (see RespawnDeadPlayers)

		pPlayer->ApplyLoadout();
	}
}

void CContingencyRules::HealPlayers( void )
{
	int i;
	CContingency_Player *pPlayer;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer )
			continue;

		if ( !IsPlayerPlaying(pPlayer) )
			continue;	// dead players are healed when they respawn (duh)

		pPlayer->SetHealth( pPlayer->GetMaxHealth() );
	}
}

void CContingencyRules::RespawnDeadPlayers( void )
{
	int i;
	CContingency_Player *pPlayer;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer )
			continue;

		if ( IsPlayerPlaying(pPlayer) )
			continue;

		// Added phase system
		pPlayer->IsAllowedToSpawn( true );

		pPlayer->Spawn();	// spawning will also update our loadout

		// Added phase system
		pPlayer->IsAllowedToSpawn( false );
	}
}

// Added announcements system
// This function allows the server to display a certain block of text at the center of a particular player's or all players' HUDs
void CContingencyRules::DisplayAnnouncement( const char* announcementText, float timeOnScreen, bool shouldFade, CBasePlayer *pTargetPlayer )
{
	hudtextparms_s tTextParam;
	tTextParam.x			= -1;
	tTextParam.y			= 0.3;
	tTextParam.effect		= 1;
	tTextParam.r1			= 255;
	tTextParam.g1			= 255;
	tTextParam.b1			= 255;
	tTextParam.a1			= 255;
	tTextParam.r2			= 255;
	tTextParam.g2			= 255;
	tTextParam.b2			= 255;
	tTextParam.a2			= 255;
	tTextParam.fadeinTime	= 1.0;
	tTextParam.fadeoutTime	= 1.0;
	tTextParam.holdTime		= timeOnScreen;
	tTextParam.fxTime		= 1.0;
	tTextParam.channel		= 1;

	color32 fadeblack = { 0, 0, 0, 200 };

	if ( pTargetPlayer )
	{
		UTIL_HudMessage( pTargetPlayer, tTextParam, announcementText );

		if ( shouldFade )
			UTIL_ScreenFade( pTargetPlayer, fadeblack, 3.0, timeOnScreen, FFADE_IN );
	}
	else
	{
		UTIL_HudMessageAll( tTextParam, announcementText );

		if ( shouldFade )
			UTIL_ScreenFadeAll( fadeblack, 3.0, timeOnScreen, FFADE_IN );
	}
}

void CContingencyRules::RemoveSatchelsAndTripmines( CContingency_Player *pPlayer )
{
	CBaseEntity *pEntity;
	CSatchelCharge *pSatchel;
	CTripmineGrenade *pTripmine;

	pEntity = NULL;
	pSatchel = NULL;
	while ( (pEntity = gEntList.FindEntityByClassname(pEntity, "npc_satchel")) != NULL )
	{
		pSatchel = dynamic_cast<CSatchelCharge*>( pEntity );
		if ( pSatchel )
		{
			if ( pPlayer && (pSatchel->GetThrower() != pPlayer) )
				continue;

			UTIL_Remove( pSatchel );
		}
	}

	pEntity = NULL;
	pTripmine = NULL;
	while ( (pEntity = gEntList.FindEntityByClassname(pEntity, "npc_tripmine")) != NULL )
	{
		pTripmine = dynamic_cast<CTripmineGrenade*>( pEntity );
		if ( pTripmine )
		{
			if ( pPlayer && (pTripmine->m_hOwner != pPlayer) )
				continue;

			pTripmine->KillBeam();
			UTIL_Remove( pTripmine );
		}
	}
}

// Added a non-restorative health system
CContingency_Player_Info *CContingencyRules::FindPlayerInfoBySteamID( const char *steamID )
{
	// Search through our list of player infos looking for one that
	// corresponds with the specified SteamID
	for ( int i = 0; i < m_PlayerInfoList.Count(); i++ )
	{
		CContingency_Player_Info *pPlayerInfo = dynamic_cast<CContingency_Player_Info*>( m_PlayerInfoList[i] );
		if ( !pPlayerInfo )
			continue;	// this entry isn't valid, so move onto the next one in the list

		if ( pPlayerInfo->GetSteamID() == steamID )
			return m_PlayerInfoList[i];	// we've found something, so stop searching and return it right away!
	}

	return NULL;	// our search returned no results!
}
#endif
