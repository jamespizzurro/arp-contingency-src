#include "cbase.h"
#include "contingency_player.h"

#include "team.h"
#include "in_buttons.h"
#include "ai_basenpc.h"
#include "bone_setup.h"
#include "studio.h"

#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Add pain sounds when a player takes damage
// This defines the minimum amount of time between pain sounds
const float MIN_TIME_BETWEEN_PAIN_SOUNDS = 2.0f;

// Reworked spawnpoint system
extern CBaseEntity *g_pLastSpawn;

LINK_ENTITY_TO_CLASS( player, CContingency_Player );

extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

BEGIN_SEND_TABLE_NOBASE( CContingency_Player, DT_SOLocalPlayerExclusive )
	END_SEND_TABLE()

	BEGIN_SEND_TABLE_NOBASE( CContingency_Player, DT_SONonLocalPlayerExclusive )
	END_SEND_TABLE()

	IMPLEMENT_SERVERCLASS_ST( CContingency_Player, DT_Contingency_Player )
	// Add a custom maximum health variable so that the client can get a player's maximum health
	SendPropInt( SENDINFO( m_iHealthMax ) ),

	// Added loadout system
	SendPropInt( SENDINFO( m_iCurrentLoadout ) ),
	END_SEND_TABLE()

	BEGIN_DATADESC( CContingency_Player )
	END_DATADESC()

#pragma warning( disable : 4355 )

// Health regeneration system
extern ConVar contingency_health_regen;
extern ConVar contingency_health_regen_delay;
extern ConVar contingency_health_regen_amount;

CContingency_Player::CContingency_Player()
{
	// Health regeneration system
	m_flHealthRegenDelay = 0.0f;

	// Add pain sounds when a player takes damage
	m_flMinTimeBtwnPainSounds = 0.0f;

	// Added phase system
	// By default, the player is a rebel soldier
	m_iCurrentLoadout = LOADOUT_SOLDIER;

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = false;

	// Adjust players' max speed based on different factors
	m_flSpeedCheckDelay = 0.0f;

	// Added chat bubble above players' heads while they type in chat
	m_hChatBubble = NULL;
}

CContingency_Player::~CContingency_Player( void )
{
}

void CContingency_Player::PickDefaultSpawnTeam( void )
{
	ChangeTeam( TEAM_PLAYER );
}

void CContingency_Player::ChangeTeam( int iTeam )
{
	// We are not allowed to join any other team other than TEAM_PLAYER (TEAM_UNASSIGNED)
	if ( iTeam != TEAM_PLAYER )
		iTeam = TEAM_PLAYER;

	CHL2_Player::ChangeTeam( iTeam );	// skip over CHL2MP_Player::ChangeTeam
}

// Added loadout system
const char *CContingency_Player::GetLoadoutName( int loadout )
{
	switch ( loadout )
	{
	case LOADOUT_SOLDIER:
		return "Rebel Soldier";
	case LOADOUT_SHOTGUN_SOLDIER:
		return "Rebel Shotgun Soldier";
	case LOADOUT_COMMANDER:
		return "Rebel Commander";
	case LOADOUT_MARKSMAN:
		return "Rebel Marksman";
	case LOADOUT_DEMOLITIONIST:
		return "Rebel Demolitionist";
	}

	return "";
}

// Added loadout system
void CContingency_Player::ApplyLoadout( int requestedLoadout, int requestedHealth, bool wasCalledDuringSpawn )
{
	if ( !ContingencyRules()->IsPlayerPlaying(this) )
	{
		if ( !wasCalledDuringSpawn )	// supress "error" messages when we're called via spawn code
			ClientPrint( this, HUD_PRINTTALK, "Only living players can change their loadout." );

		return;
	}

	if ( (ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM) && !wasCalledDuringSpawn )
	{
		if ( !wasCalledDuringSpawn )	// supress "error" messages when we're called via spawn code
			ClientPrint( this, HUD_PRINTTALK, "You can only change your loadout during an interim phase." );

		return;
	}

	// Allow players to do this anyway 'cause it's easy enough to do even with the check below
	/*if ( (GetCurrentLoadout() == requestedLoadout) && !wasCalledDuringSpawn )
	{
		if ( !wasCalledDuringSpawn )	// supress "error" messages when we're called via spawn code
			ClientPrint( this, HUD_PRINTTALK, UTIL_VarArgs("You are already a %s!", GetLoadoutName(requestedLoadout)) );

		return;
	}*/

	ContingencyRules()->RemoveSatchelsAndTripmines(this);

	RemoveAllWeapons();

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = true;

	EquipSuit();

	// All loadouts get the following weapons and ammo:
	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_frag" );
	CBasePlayer::GiveAmmo( 99999, "Pistol" );
	CBasePlayer::GiveAmmo( 99999, "grenade" );

	switch ( requestedLoadout )
	{
	case LOADOUT_SOLDIER:
		SetModel( kContingencyPlayerModels[LOADOUT_SOLDIER] );
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < LOADOUT_SOLDIER_MAXHEALTH) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = LOADOUT_SOLDIER_MAXHEALTH;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = LOADOUT_SOLDIER_MAXHEALTH;
		GiveNamedItem( "weapon_smg1" );
		CBasePlayer::GiveAmmo( 99999, "SMG1" );
		CBasePlayer::GiveAmmo( 99999, "SMG1_Grenade" );
		Weapon_Switch( Weapon_OwnsThisType("weapon_smg1") );
		break;
	case LOADOUT_SHOTGUN_SOLDIER:
		SetModel( kContingencyPlayerModels[LOADOUT_SHOTGUN_SOLDIER] );
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < LOADOUT_SHOTGUN_SOLDIER_MAXHEALTH) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = LOADOUT_SHOTGUN_SOLDIER_MAXHEALTH;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = LOADOUT_SHOTGUN_SOLDIER_MAXHEALTH;
		GiveNamedItem( "weapon_shotgun" );
		CBasePlayer::GiveAmmo( 99999, "Buckshot" );
		Weapon_Switch( Weapon_OwnsThisType("weapon_shotgun") );
		break;
	case LOADOUT_COMMANDER:
		SetModel( kContingencyPlayerModels[LOADOUT_COMMANDER] );
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < LOADOUT_COMMANDER_MAXHEALTH) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = LOADOUT_COMMANDER_MAXHEALTH;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = LOADOUT_COMMANDER_MAXHEALTH;
		GiveNamedItem( "weapon_ar2" );
		CBasePlayer::GiveAmmo( 99999, "AR2" );
		CBasePlayer::GiveAmmo( 99999, "AR2AltFire" );
		Weapon_Switch( Weapon_OwnsThisType("weapon_ar2") );
		break;
	case LOADOUT_MARKSMAN:
		SetModel( kContingencyPlayerModels[LOADOUT_MARKSMAN] );
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < LOADOUT_MARKSMAN_MAXHEALTH) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = LOADOUT_MARKSMAN_MAXHEALTH;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = LOADOUT_MARKSMAN_MAXHEALTH;
		GiveNamedItem( "weapon_crossbow" );
		CBasePlayer::GiveAmmo( 99999, "XBowBolt" );
		Weapon_Switch( Weapon_OwnsThisType("weapon_crossbow") );
		break;
	case LOADOUT_DEMOLITIONIST:
		SetModel( kContingencyPlayerModels[LOADOUT_DEMOLITIONIST] );
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < LOADOUT_DEMOLITIONIST_MAXHEALTH) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = LOADOUT_DEMOLITIONIST_MAXHEALTH;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = LOADOUT_DEMOLITIONIST_MAXHEALTH;
		GiveNamedItem( "weapon_physcannon" );
		GiveNamedItem( "weapon_357" );
		GiveNamedItem( "weapon_rpg" );
		GiveNamedItem( "weapon_slam" );
		CBasePlayer::GiveAmmo( 99999, "357" );
		CBasePlayer::GiveAmmo( 99999, "rpg_round" );
		CBasePlayer::GiveAmmo( 99999, "slam" );
		Weapon_Switch( Weapon_OwnsThisType("weapon_physcannon") );
		break;
	default:
		SetModel( kContingencyPlayerModels[LOADOUT_SOLDIER] );	// ...I guess?
		// Add a custom maximum health variable so that the client can get a player's maximum health
		if ( (requestedHealth > 0)  && (requestedHealth < 100) )
		{
			m_iHealth = requestedHealth;
			m_iHealthMax = m_iMaxHealth = 100;
		}
		else
			m_iHealthMax = m_iMaxHealth = m_iHealth = 100;	// any 'ol health value
		Weapon_Switch( Weapon_OwnsThisType("weapon_crowbar") );
	}

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = false;

	SetCurrentLoadout( requestedLoadout );

	// Added phase system
	// Force players to be observers during combat phases
	if ( ContingencyRules()->GetCurrentPhase() != PHASE_COMBAT )
	{
		if ( wasCalledDuringSpawn )
		{
			ClientPrint( this, HUD_PRINTTALK, UTIL_VarArgs("You have spawned as a %s.", GetLoadoutName(requestedLoadout)) );

			if ( ContingencyRules()->GetCurrentPhase() == PHASE_INTERIM )
				ClientPrint( this, HUD_PRINTTALK, "You can change your loadout during an interim phase by pressing your loadout menu key (M by default)." );
		}
		else
			ClientPrint( this, HUD_PRINTTALK, UTIL_VarArgs("You are now a %s.", GetLoadoutName(requestedLoadout)) );
	}
	else
	{
		StartObserverMode( OBS_MODE_ROAMING );
		ClientPrint( this, HUD_PRINTTALK, "You are currently an observer." );
		ClientPrint( this, HUD_PRINTTALK, "You will spawn at the start of the next interim phase." );
	}
}

// Added loadout system
void CContingency_Player::Replenish( void )
{
	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = true;

	// All loadouts get the following weapons and ammo:
	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_frag" );
	CBasePlayer::GiveAmmo( 99999, "Pistol" );
	CBasePlayer::GiveAmmo( 99999, "grenade" );

	switch ( GetCurrentLoadout() )
	{
	case LOADOUT_SOLDIER:
		GiveNamedItem( "weapon_smg1" );
		CBasePlayer::GiveAmmo( 99999, "SMG1" );
		CBasePlayer::GiveAmmo( 99999, "SMG1_Grenade" );
		break;
	case LOADOUT_SHOTGUN_SOLDIER:
		GiveNamedItem( "weapon_shotgun" );
		CBasePlayer::GiveAmmo( 99999, "Buckshot" );
		break;
	case LOADOUT_COMMANDER:
		GiveNamedItem( "weapon_ar2" );
		CBasePlayer::GiveAmmo( 99999, "AR2" );
		CBasePlayer::GiveAmmo( 99999, "AR2AltFire" );
		break;
	case LOADOUT_MARKSMAN:
		GiveNamedItem( "weapon_crossbow" );
		CBasePlayer::GiveAmmo( 99999, "XBowBolt" );
		break;
	case LOADOUT_DEMOLITIONIST:
		GiveNamedItem( "weapon_physcannon" );
		GiveNamedItem( "weapon_357" );
		GiveNamedItem( "weapon_rpg" );
		GiveNamedItem( "weapon_slam" );
		CBasePlayer::GiveAmmo( 99999, "357" );
		CBasePlayer::GiveAmmo( 99999, "rpg_round" );
		CBasePlayer::GiveAmmo( 99999, "slam" );
		break;
	}

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = false;
}

// Reworked spawnpoint system
CBaseEntity* CContingency_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;

	const char *pSpawnpointName = "info_player_deathmatch";

	pSpot = pLastSpawnPoint;

	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet, so spawn at the first spawn point
	if ( pSpot )
		goto ReturnSpot;

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	g_pLastSpawn = pSpot;

	return pSpot;
}

void CContingency_Player::Precache( void )
{
	BaseClass::Precache();

	// Precache player models
	for ( int i = 0; i < NUM_LOADOUTS; ++i )
	   	 PrecacheModel( kContingencyPlayerModels[i] );

	PrecacheScriptSound( "Male.Pain" );
	PrecacheScriptSound( "Male.Death" );

	// Added wave system
	// Precache all NPC models that are to be used here to ensure
	// players don't experience any annoying loading delays later
	ContingencyRules()->PrecacheWaveNPCs();
}

void CContingency_Player::Spawn( void )
{
	BaseClass::Spawn();

	// Added chat bubble above players' heads while they type in chat
	KillChatBubble();

	// Added a non-restorative health system
	const char *steamID = engine->GetPlayerNetworkIDString( edict() );
	int savedCurrentLoadout = GetCurrentLoadout();
	int savedPlayerHealth = 0;
	CContingency_Player_Info *pPlayerInfo = ContingencyRules()->FindPlayerInfoWithSteamID( steamID );
	if ( pPlayerInfo && !pPlayerInfo->HasBeenAccessed() )
	{
		// We've found a player info entry with our player's steamID in it,
		// so use it to recall our player's loadout and health when they disconnected
		pPlayerInfo->HasBeenAccessed( true );
		savedCurrentLoadout = pPlayerInfo->GetLoadout();
		savedPlayerHealth = pPlayerInfo->GetHealth();
	}

	ApplyLoadout( savedCurrentLoadout, savedPlayerHealth, true );
}

// Adjust players' max speed based on different factors
void CContingency_Player::SetMaxSpeed( float flMaxSpeed )
{
	float newMaxSpeed = 0.0f;

	if ( ContingencyRules()->IsPlayerPlaying(this) )
		newMaxSpeed = flMaxSpeed - (m_iHealthMax - m_iHealth);
	else
		newMaxSpeed = flMaxSpeed;

	if ( newMaxSpeed < 50 )
		newMaxSpeed = 50;	// clamp to 50

	m_flMaxspeed = newMaxSpeed;
}

void CContingency_Player::PostThink( void )
{
	BaseClass::PostThink();

	// Adjust players' max speed based on different factors
	// The original SetMaxSpeed function has been overrided to make this all work properly (see player.h)
	if ( gpGlobals->curtime >= m_flSpeedCheckDelay )
	{
		// There's no need for any speed alterations when we're not playing
		if ( ContingencyRules()->IsPlayerPlaying(this) )
		{
			if ( IsWalking() )
				SetMaxSpeed( GetMaxWalkingSpeed() );
			else if ( IsSprinting() )
				SetMaxSpeed( GetMaxSprintingSpeed() );
			else
				SetMaxSpeed( GetMaxNormalSpeed() );
		}

		m_flSpeedCheckDelay = gpGlobals->curtime + 0.1f;	// this might be too frequent...
	}

	// Health regeneration system
	if ( gpGlobals->curtime >= m_flHealthRegenDelay )
	{
		if ( contingency_health_regen.GetBool() )
		{
			// Only regenerate health for players who are playing
			if ( ContingencyRules()->IsPlayerPlaying(this) )
			{
				int currentHealth = GetHealth();
				int newHealth = currentHealth;
				int maxHealth = GetMaxHealth();

				if ( currentHealth != maxHealth )
				{
					// If we're hurt, give us some health
					if ( currentHealth < maxHealth )
						newHealth = currentHealth + contingency_health_regen_amount.GetInt();
					// If we somehow managed to get more health
					// than allowed, set our health back to max
					else if ( currentHealth > maxHealth )
						newHealth = maxHealth;

					SetHealth( newHealth );
				}
			}
		}

		m_flHealthRegenDelay = gpGlobals->curtime + contingency_health_regen_delay.GetFloat();
	}
}

// Do not allow players to change their team
bool CContingency_Player::HandleCommand_JoinTeam( int team )
{
	return false;
}

// Added loadout system
// Prevent players from being able to pick up weapons that don't belong to their loadout
// ...unless the game specifically wants us to, of course
bool CContingency_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	if ( !pWeapon )
		return false;

	// See if the weapon we're bumping into is a special weapon,
	// in which case we should see if we can pick it up
	for ( int i = 0; i < NUM_SPECIAL_WEAPONS; i++ )
	{
		if ( FClassnameIs(pWeapon, kSpecialWeapons[i]) && !Weapon_OwnsThisType(kSpecialWeapons[i]) )
			return BaseClass::BumpWeapon( pWeapon );
	}

	// I'll be honest: this isn't the best system, but it works
	if ( !Weapon_OwnsThisType(pWeapon->GetName()) && !m_bGivingWeapons )
		return false;

	return BaseClass::BumpWeapon( pWeapon );
}

int CContingency_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	int damage = BaseClass::OnTakeDamage( inputInfo );

	// Add pain sounds when a player takes damage
	if ( (damage > 0) &&
		(gpGlobals->curtime >= m_flMinTimeBtwnPainSounds) &&
		ContingencyRules()->IsPlayerPlaying(this) )
		PainSound(inputInfo);

	return damage;
}

// Add pain sounds when a player takes damage
void CContingency_Player::PainSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		return;

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( "Male.Pain", params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );

	m_flMinTimeBtwnPainSounds = gpGlobals->curtime + MIN_TIME_BETWEEN_PAIN_SOUNDS;
}

void CContingency_Player::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	// Added chat bubble above players' heads while they type in chat
	KillChatBubble();

	// Drop all special weapons when a player dies
	CBaseCombatWeapon *pWeapon = NULL;
	for ( int i = 0; i < NUM_SPECIAL_WEAPONS; i++ )
	{
		pWeapon = Weapon_OwnsThisType( kSpecialWeapons[i] );
		if ( !pWeapon )
			continue;

		Weapon_Drop( pWeapon, NULL, NULL );
	}
}

// Change death sounds when a player dies
void CContingency_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		return;

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( "Male.Death", params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

// Rework respawning system
bool CContingency_Player::StartObserverMode( int mode )
{
	VPhysicsDestroyObject();
	return CBasePlayer::StartObserverMode( mode );	// skip over CHL2MP_Player::StartObserverMode
}

// Rework respawning system
// Based on CBasePlayer::PlayerDeathThink
void CContingency_Player::PlayerDeathThink( void )
{
	float flForward;

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( GetFlags() & FL_ONGROUND )
	{
		flForward = GetAbsVelocity().Length() - 20;
		if ( flForward <= 0 )
		{
			SetAbsVelocity( vec3_origin );
		}
		else
		{
			Vector vecNewVelocity = GetAbsVelocity();
			VectorNormalize( vecNewVelocity );
			vecNewVelocity *= flForward;
			SetAbsVelocity( vecNewVelocity );
		}
	}

	// we drop the guns here because weapons that have an area effect and can kill their user
	// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
	// player class sometimes is freed. It's safer to manipulate the weapons once we know
	// we aren't calling into any of their code anymore through the player pointer.
	if ( HasWeapons() )
		PackDeadPlayerItems();

	if ( GetModelIndex() && !IsSequenceFinished() && (m_lifeState == LIFE_DYING) )
	{
		StudioFrameAdvance();

		m_iRespawnFrames++;
		if ( m_iRespawnFrames < 60 )  // animations should be no longer than this
			return;
	}

	if ( m_lifeState == LIFE_DYING )
	{
		m_lifeState = LIFE_DEAD;
		m_flDeathAnimTime = gpGlobals->curtime;
	}

	StopAnimation();

	AddEffects( EF_NOINTERP );
	m_flPlaybackRate = 0.0;

	if ( gpGlobals->curtime >= (m_flDeathTime + DEATH_ANIMATION_TIME) )
	{
		if ( ContingencyRules()->CanPlayersRespawn() )
		{
			m_nButtons = 0;
			m_iRespawnFrames = 0;

			Spawn();	// respawn the player

			SetNextThink( TICK_NEVER_THINK );
		}
		else if ( !IsObserver() )
		{
			// Since we can't respawn, allow us to start roaming around as an observer
			StartObserverMode( OBS_MODE_ROAMING );
		}
	}
}

// Added loadout system
void CC_CHANGELOADOUT( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	const char *loadoutName = args.Arg(1);
	int loadout;
	if ( Q_strcmp(loadoutName, "soldier") == 0 )
		loadout = LOADOUT_SOLDIER;
	else if ( Q_strcmp(loadoutName, "shotgun_soldier") == 0 )
		loadout = LOADOUT_SHOTGUN_SOLDIER;
	else if ( Q_strcmp(loadoutName, "commander") == 0 )
		loadout = LOADOUT_COMMANDER;
	else if ( Q_strcmp(loadoutName, "marksman") == 0 )
		loadout = LOADOUT_MARKSMAN;
	else if ( Q_strcmp(loadoutName, "demolitionist") == 0 )
		loadout = LOADOUT_DEMOLITIONIST;
	else
		return;

	pPlayer->ApplyLoadout( loadout, pPlayer->GetHealth(), false );
}
static ConCommand changeloadout("changeloadout", CC_CHANGELOADOUT, "Used to change one's current loadout when permitted");

// Added loadout system
// Added loadout menu
void CC_LOADOUTMENU( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( !ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Only living players can change their loadout." );
		return;
	}

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "You can only change your loadout during an interim phase." );
		return;
	}

	pPlayer->ShowViewPortPanel( "loadoutmenu", true, NULL );
}
static ConCommand showloadoutmenu("showloadoutmenu", CC_LOADOUTMENU, "Shows the loadout menu for changing one's loadout");

// Added drop system
void CC_Player_Drop( void )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( !pWeapon )
		return;

	// Only allow players to drop special weapons
	for ( int i = 0; i < NUM_SPECIAL_WEAPONS; i++ )
	{
		if ( FClassnameIs(pWeapon, kSpecialWeapons[i]) )
		{
			pPlayer->Weapon_Drop( pWeapon, NULL, NULL );
			return;
		}
	}
}
static ConCommand drop( "drop", CC_Player_Drop, "Drops your current weapon (if possible; only works for special weapons)" );

// Added drop system
void CContingency_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	if ( !pWeapon )
		return;

	bool bWasActiveWeapon = false;
	if ( pWeapon == GetActiveWeapon() )
		bWasActiveWeapon = true;

	if ( bWasActiveWeapon )
		pWeapon->SendWeaponAnim( ACT_VM_IDLE );

	// If I'm an NPC, fill the weapon with ammo before I drop it.
	if ( GetFlags() & FL_NPC )
	{
		if ( pWeapon->UsesClipsForAmmo1() )
			pWeapon->m_iClip1 = pWeapon->GetDefaultClip1();
		if ( pWeapon->UsesClipsForAmmo2() )
			pWeapon->m_iClip2 = pWeapon->GetDefaultClip2();
	}

	if ( IsPlayer() )
	{
		Vector vThrowPos = Weapon_ShootPosition() - Vector(0,0,12);

		if( UTIL_PointContents(vThrowPos) & CONTENTS_SOLID )
			Msg("Weapon spawning in solid!\n");

		pWeapon->SetAbsOrigin( vThrowPos );

		QAngle gunAngles;
		VectorAngles( BodyDirection2D(), gunAngles );
		pWeapon->SetAbsAngles( gunAngles );
	}
	else
	{
		int iBIndex = -1;
		int iWeaponBoneIndex = -1;

		CStudioHdr *hdr = pWeapon->GetModelPtr();
		// If I have a hand, set the weapon position to my hand bone position.
		if ( hdr && hdr->numbones() > 0 )
		{
			// Assume bone zero is the root
			for ( iWeaponBoneIndex = 0; iWeaponBoneIndex < hdr->numbones(); ++iWeaponBoneIndex )
			{
				iBIndex = LookupBone( hdr->pBone( iWeaponBoneIndex )->pszName() );
				// Found one!
				if ( iBIndex != -1 )
					break;
			}

			if ( iBIndex == -1 )
				iBIndex = LookupBone( "ValveBiped.Weapon_bone" );
		}
		else
		{
			iBIndex = LookupBone( "ValveBiped.Weapon_bone" );
		}

		if ( iBIndex != -1)  
		{
			Vector origin;
			QAngle angles;
			matrix3x4_t transform;

			// Get the transform for the weapon bonetoworldspace in the NPC
			GetBoneTransform( iBIndex, transform );

			// find offset of root bone from origin in local space
			// Make sure we're detached from hierarchy before doing this!!!
			pWeapon->StopFollowingEntity();
			pWeapon->SetAbsOrigin( Vector( 0, 0, 0 ) );
			pWeapon->SetAbsAngles( QAngle( 0, 0, 0 ) );
			pWeapon->InvalidateBoneCache();
			matrix3x4_t rootLocal;
			pWeapon->GetBoneTransform( iWeaponBoneIndex, rootLocal );

			// invert it
			matrix3x4_t rootInvLocal;
			MatrixInvert( rootLocal, rootInvLocal );

			matrix3x4_t weaponMatrix;
			ConcatTransforms( transform, rootInvLocal, weaponMatrix );
			MatrixAngles( weaponMatrix, angles, origin );
			
			pWeapon->Teleport( &origin, &angles, NULL );
		}
		// Otherwise just set in front of me.
		else 
		{
			Vector vFacingDir = BodyDirection2D();
			vFacingDir = vFacingDir * 10.0; 
			pWeapon->SetAbsOrigin( Weapon_ShootPosition() + vFacingDir );
		}
	}

	Vector vecThrow;
	if (pvecTarget)
	{
		// I've been told to throw it somewhere specific.
		vecThrow = VecCheckToss( this, pWeapon->GetAbsOrigin(), *pvecTarget, 0.2, 1.0, false );
	}
	else
	{
		if ( pVelocity )
		{
			vecThrow = *pVelocity;
			float flLen = vecThrow.Length();
			if (flLen > 400)
			{
				VectorNormalize(vecThrow);
				vecThrow *= 400;
			}
		}
		else
		{
			// Nowhere in particular; just drop it.
			float throwForce = ( IsPlayer() ) ? 400.0f : random->RandomInt( 64, 128 );
			vecThrow = BodyDirection3D() * throwForce;
		}
	}

	pWeapon->Drop( vecThrow );
	Weapon_Detach( pWeapon );

	if ( HasSpawnFlags( SF_NPC_NO_WEAPON_DROP ) )
	{
		// Don't drop weapons when the super physgun is happening.
		UTIL_Remove( pWeapon );
	}

	if ( bWasActiveWeapon )
	{
		if (!SwitchToNextBestWeapon( NULL ))
		{
			CBaseViewModel *vm = GetViewModel();
			if ( vm )
				vm->AddEffects( EF_NODRAW );
		}
	}

	// Make weapons without owners blink so that they are easier to see
	pWeapon->AddEffects( EF_ITEM_BLINK );
}

/////

	// Added chat bubble above players' heads while they type in chat

#define CHAT_BUBBLE_MODEL "models/extras/info_speech.mdl"
class CChatBubble : public CBaseAnimating
{
public:
	DECLARE_CLASS( CChatBubble, CBaseAnimating );

	CChatBubble::CChatBubble() { UseClientSideAnimation(); }

	void Precache()
	{
		PrecacheModel( CHAT_BUBBLE_MODEL );
	}

	void Spawn()
	{
		SetModel( CHAT_BUBBLE_MODEL );

		SetSolid( SOLID_NONE );	// is not solid
		SetMoveType( MOVETYPE_NONE );	// cannot move on its own
		AddEffects( EF_NOSHADOW );	// doesn't make shadows
		AddEffects( EF_NORECEIVESHADOW );	// doesn't receive shadows

		BaseClass::Spawn();

		SetThink( &CChatBubble::BubbleThink );
		SetNextThink( gpGlobals->curtime + 0.01f );
	}

	void BubbleThink( void )
	{
		// Rotate the chat bubble
		QAngle aRotations = GetAbsAngles();
		aRotations[1] = aRotations[1] + 2;
		SetAbsAngles( aRotations );

		// Update our position relative to the position of our player
		if ( GetOwnerEntity() )
			SetAbsOrigin( GetOwnerEntity()->GetAbsOrigin() + Vector(0, 0, 85) );

		SetNextThink( gpGlobals->curtime + 0.01f );
	}
};

LINK_ENTITY_TO_CLASS( chat_bubble, CChatBubble );
PRECACHE_REGISTER( chat_bubble );

void CContingency_Player::MakeChatBubble( int chatbubble )
{
	// Make sure players don't have chat bubbles when they're not supposed to
	if ( !ContingencyRules()->IsPlayerPlaying(this) )
	{
		KillChatBubble();
		return;
	}

	if ( m_hChatBubble.Get() )
		return;	// player already has a chat bubble, so don't make another one

	CChatBubble *pBubble = (CChatBubble*) CBaseEntity::CreateNoSpawn( "chat_bubble", GetAbsOrigin() + Vector(0, 0, 85), QAngle(0, 0, 0), this );
	if ( !pBubble )
		return;

	pBubble->Spawn();
	pBubble->SetOwnerEntity( this );	// set owner to player
	m_hChatBubble = pBubble;	// assign to player
}

void CContingency_Player::KillChatBubble()
{
	if ( !m_hChatBubble.Get() )
		return;	// this player doesn't have a chat bubble to kill

	m_hChatBubble.Get()->FollowEntity( NULL );
	m_hChatBubble.Get()->SetThink( &CBaseEntity::Remove );
	m_hChatBubble.Get()->SetNextThink( gpGlobals->curtime + 0.001f );
	m_hChatBubble = NULL;
}

void CContingency_Player::CheckChatBubble( CUserCmd *cmd )
{
	if ( !cmd )
		return;

	if ( cmd->chatbubble )
		MakeChatBubble( cmd->chatbubble );
	else
		KillChatBubble();
}

/////
