#include "cbase.h"
#include "contingency_player.h"

#include "team.h"
#include "in_buttons.h"
#include "ai_basenpc.h"
#include "bone_setup.h"
#include "studio.h"

#include "contingency_gamerules.h"

#include "../hl2/weapon_physcannon.h"

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
	// Added credits system
	SendPropInt( SENDINFO(m_iCredits) ),

	// Added spawnable prop system
	SendPropInt( SENDINFO(m_iNumSpawnableProps) ),

	// Added spawnable prop system
	SendPropInt( SENDINFO(m_iDesiredSpawnablePropIndex) ),
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

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = false;

	// Adjust players' max speed based on different factors
	m_flSpeedCheckDelay = 0.0f;

	// Added chat bubble above players' heads while they type in chat
	m_hChatBubble = NULL;

	// Added loadout system
	IsMarkedForLoadoutUpdate( false );

	// Added shout system
	m_bShowShoutMenu = false;
	m_flShoutDelay = 0.0f;

	// Added phase system
	m_bAllowedToSpawn = false;

	// Added a modified version of Valve's floor turret
	m_hDeployedTurret = NULL;

	// Added credits system
	m_iCredits = 0;

	// Added spawnable prop system
	m_pSpawnablePropInFocus = NULL;
	m_iNumSpawnableProps = 0;

	// Added spawnable prop system
	m_iDesiredSpawnablePropIndex = 0;
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
void CContingency_Player::ApplyLoadout( void )
{
	CBaseCombatWeapon *pDeployableTurret = Weapon_OwnsThisType( "weapon_deployableturret" );
	bool hadDeployableTurret = false;
	//int previousDeployableTurretHealth = CONTINGENCY_TURRET_MAX_HEALTH;
	if ( pDeployableTurret )
	{
		hadDeployableTurret = true;
		//previousDeployableTurretHealth = pDeployableTurret->GetHealth();
	}

	// Handle weapons and ammunition

	// Remove any shit we have before we get new shit
	//ContingencyRules()->RemoveSatchelsAndTripmines( this );

	CBaseCombatWeapon *wpn = GetActiveWeapon();

	if ( wpn != NULL ) //only execute weapon methods if the player exists at the end of the phase
	{
		const char* ActiveWeaponName = wpn->GetName();
		if (strcmp(ActiveWeaponName, "weapon_physcannon") == 0) // If at the end of any phase player has the gravity gun out...
		{
			PhysCannonForceDropUnconditional(wpn); //...and is carrying an object with it, drop that object properly
		}
	}

	RemoveAllWeapons();

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = true;

	EquipSuit( false );

	// Supply the player with their weapons
	if ( ContingencyRules()->GetCurrentPhase() == PHASE_INTERIM )
	{
		// During interim phases, we choose what players get, not the players themselves
		GiveNamedItem( "weapon_crowbar" );
		GiveNamedItem( "weapon_wrench" );
		GiveNamedItem( "weapon_physcannon" );
	}
	else
	{

		const char *szPreferredEquipmentClassname = "";

		// During combat phases, players are given their full loadout
		if ( IsBot() )
		{
			// Bots get a randomly chosen loadout
			GiveNamedItem( kPrimaryWeaponTypes[random->RandomInt(0, NUM_PRIMARY_WEAPON_TYPES)][0] );
			GiveNamedItem( kSecondaryWeaponTypes[random->RandomInt(0, NUM_SECONDARY_WEAPON_TYPES)][0] );
			GiveNamedItem( kMeleeWeaponTypes[random->RandomInt(0, NUM_MELEE_WEAPON_TYPES)][0] );

			szPreferredEquipmentClassname = kEquipmentTypes[random->RandomInt(0, NUM_EQUIPMENT_TYPES)][0];
		}
		else
		{
			GiveNamedItem( GetPreferredPrimaryWeaponClassname() );
			GiveNamedItem( GetPreferredSecondaryWeaponClassname() );
			GiveNamedItem( GetPreferredMeleeWeaponClassname() );

			szPreferredEquipmentClassname = GetPreferredEquipmentClassname();
		}

		if ( Q_strcmp(szPreferredEquipmentClassname, "weapon_slam") != 0 )
		{
			// We do not want SLAMs
			ContingencyRules()->RemoveSatchelsAndTripmines( this );	// remove any we have on the map
		}

		// Added a modified version of Valve's floor turret
		CNPC_FloorTurret *pDeployedTurret = GetDeployedTurret();
		if ( Q_strcmp(szPreferredEquipmentClassname, "weapon_deployableturret") == 0 )
		{
			// We want a deployable turret

			if ( pDeployedTurret )
			{
				// We already have a deployed turret, so don't give us another deployable one
				// unless we already had both before all of our weapons were removed at the beginning of this function
			
				// This allows players to pick up turrets that may have been explicitly placed
				// on the map by level designers without fear of them being removed

				if ( hadDeployableTurret )
				{
					CBaseEntity *pDeployableTurret = GiveNamedItem( szPreferredEquipmentClassname );
					if ( pDeployableTurret )
					{
						// TODO: Get the health carry over to work? :s
						//pDeployableTurret->SetHealth( previousDeployableTurretHealth );	// carry over health of previous deployable turret
					}
				}
			}
			else
			{
				// We do not yet have a deployed turret, so give us a deployable one per our request
				GiveNamedItem( szPreferredEquipmentClassname );
			}
		}
		else
		{
			// We do not want a deployable turret

			if ( pDeployedTurret )
				pDeployedTurret->Explode();	// we prefer new equipment but currently have a deployed turret in the world,
											// so explode that turret before giving us our new equipment

			GiveNamedItem( szPreferredEquipmentClassname );
		}
	}

	// Added loadout system
	// Prevent players from being able to pick up weapons that don't belong to their loadout
	// ...unless the game specifically wants us to, of course
	m_bGivingWeapons = false;

	if ( ContingencyRules()->GetCurrentPhase() == PHASE_INTERIM )
	{
		// Switch to something iconic (the gravity gun, of course)
		Weapon_Switch( Weapon_OwnsThisType("weapon_physcannon") );
	}
	else
	{
		// Give players all the ammo they'll ever need
		CBasePlayer::GiveAmmo( 99999, "Pistol", true );
		CBasePlayer::GiveAmmo( 99999, "grenade", true );
		CBasePlayer::GiveAmmo( 99999, "SMG1", true );
		CBasePlayer::GiveAmmo( 99999, "SMG1_Grenade", true );
		CBasePlayer::GiveAmmo( 99999, "Buckshot", true );
		CBasePlayer::GiveAmmo( 99999, "AR2", true );
		CBasePlayer::GiveAmmo( 99999, "AR2AltFire", true );
		CBasePlayer::GiveAmmo( 99999, "XBowBolt", true );
		CBasePlayer::GiveAmmo( 99999, "357", true );
		CBasePlayer::GiveAmmo( 99999, "rpg_round", true );
		CBasePlayer::GiveAmmo( 99999, "slam", true );

		// Make sure we switch to our primary weapon at the end of all this madness
		Weapon_Switch( Weapon_OwnsThisType(GetPreferredPrimaryWeaponClassname()) );

		if ( IsMarkedForLoadoutUpdate() )
		{
			ClientPrint( this, HUD_PRINTTALK, "Your new loadout has been applied." );
			IsMarkedForLoadoutUpdate( false );
		}
	}
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
	for ( int i = 0; i < NUM_PLAYER_MODELS; ++i )
	   	 PrecacheModel( kContingencyPlayerModels[i] );

	PrecacheScriptSound( "Male.Pain" );
	PrecacheScriptSound( "Male.Death" );

	// Added shout system
	PrecacheScriptSound( "Male.Incoming" );
	PrecacheScriptSound( "Male.Run" );
	PrecacheScriptSound( "Male.Go" );
	PrecacheScriptSound( "Male.Lead" );
	PrecacheScriptSound( "Male.Cover" );
	PrecacheScriptSound( "Male.Ready" );
	PrecacheScriptSound( "Male.Headcrabs" );
	PrecacheScriptSound( "Male.Zombies" );
	PrecacheScriptSound( "Male.Combine" );

	// Precaching called by each player (server-side)
	// This is in gamerules because all the precaching done here is really gameplay-dependent
	ContingencyRules()->PrecacheStuff();
}

void CContingency_Player::Spawn( void )
{
	BaseClass::Spawn();

	// Added chat bubble above players' heads while they type in chat
	KillChatBubble();

	// Handle our player model 
	// At least for now, give us a random one
	SetModel( kContingencyPlayerModels[random->RandomInt(0, NUM_PLAYER_MODELS - 1)] );

	// Added phase system
	if ( IsAllowedToSpawn() )
	{
		ClientPrint( this, HUD_PRINTTALK, "You have been spawned." );

		// Added a non-restorative health system
		const char *steamID = engine->GetPlayerNetworkIDString( edict() );
		int savedPlayerHealth = 0;
		CContingency_Player_Info *pPlayerInfo = ContingencyRules()->FindPlayerInfoBySteamID( steamID );
		if ( pPlayerInfo && !pPlayerInfo->HasBeenAccessed() )
		{
			// We've found a player info entry with our player's steamID in it,
			// so use it to recall our player's health, etc. when they disconnected
			pPlayerInfo->HasBeenAccessed( true );
			savedPlayerHealth = pPlayerInfo->GetHealth();
			
			// Added credits system
			// Restore the player's credits
			SetCredits( pPlayerInfo->GetCredits() );

			// Added spawnable prop system
			// Restore our reference to our spawnable props (via a list)
			SetNumSpawnableProps( pPlayerInfo->GetNumSpawnableProps() );
			m_SpawnablePropList = pPlayerInfo->spawnablePropList;	// NOTE: spawnablePropList is updated even while we're not connected (e.g. see CContingency_SpawnableProp's deconstructor)
			for ( int i = 0; i < m_SpawnablePropList.Count(); i++ )	// actually register us as the owner of our spawnable props again (if possible)
			{
				CContingency_SpawnableProp *pSpawnableProp = m_SpawnablePropList.Element(i);
				if ( !pSpawnableProp )
					continue;

				pSpawnableProp->SetSpawnerPlayer( this );
			}

			// Added a modified version of Valve's floor turret
			// Restore our reference to our deployed turret (if any)
			CNPC_FloorTurret *pDeployedTurret = pPlayerInfo->GetDeployedTurret();
			if ( pDeployedTurret )	// actually register us as the owner of our deployed turret again (if possible)
			{
				SetDeployedTurret( pDeployedTurret );
				pDeployedTurret->SetOwnerEntity( this );
			}

			ClientPrint( this, HUD_PRINTTALK, "Your player state before you disconnected has been restored." );
		}
		else if ( !pPlayerInfo )
		{
			// No player info was found!
			// So, this player hasn't spawned before during the current game

			// Added credits system
			// Give the player some starting credits according to what the current map has defined
			SetCredits( ContingencyRules()->GetMapStartingCredits() );
		}

		// Handle our current and maximum health
		if ( (savedPlayerHealth > 0) && (savedPlayerHealth < CONTINGENCY_MAX_HEALTH) )
			m_iHealth = savedPlayerHealth;
		else
			m_iHealth = CONTINGENCY_MAX_HEALTH;
		m_iMaxHealth = CONTINGENCY_MAX_HEALTH;

		// Prevent suit gloves admiration animation from showing
		CBaseViewModel *pViewModel = GetViewModel();
		if ( pViewModel )
			pViewModel->RemoveEffects( EF_NODRAW );

		// Added loadout system
		ApplyLoadout( );
	}
	else	// by default, players are forced to be observers
	{
		ClientPrint( this, HUD_PRINTTALK, "You are currently an observer. You will spawn at the start of the next phase." );
	
		// Handle our current and maximum health
		m_iHealth = m_iMaxHealth = CONTINGENCY_MAX_HEALTH;

		// Prevent suit gloves admiration animation from showing
		CBaseViewModel *pViewModel = GetViewModel();
		if ( pViewModel )
			pViewModel->AddEffects( EF_NODRAW );

		StartObserverMode( OBS_MODE_ROAMING );
	}
}

// Adjust players' max speed based on different factors
void CContingency_Player::SetMaxSpeed( float flMaxSpeed )
{
	float newMaxSpeed = flMaxSpeed;

	// Added weapon weight system
	// Cycle through the player's weapons and apply their respective weights
	for ( int i = 0; i < MAX_WEAPONS; ++i ) 
	{
		CBaseCombatWeapon *pWeapon = GetWeapon(i);
		if ( !pWeapon )
			continue;

		newMaxSpeed = newMaxSpeed - pWeapon->GetWeight();
	}

	if ( ContingencyRules()->IsPlayerPlaying(this) )
		newMaxSpeed = newMaxSpeed - (m_iMaxHealth - m_iHealth);

	if ( newMaxSpeed < CONTINGENCY_MINIMUM_SPEED )
		newMaxSpeed = CONTINGENCY_MINIMUM_SPEED;	// clamp to CONTINGENCY_MINIMUM_SPEED

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

bool CContingency_Player::ClientCommand( const CCommand &args )
{
	return BaseClass::ClientCommand( args );
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
	for ( int i = 0; i < NUM_SPECIAL_WEAPON_TYPES; i++ )
	{
		if ( FClassnameIs(pWeapon, kSpecialWeaponTypes[i][0]) && !Weapon_OwnsThisType(kSpecialWeaponTypes[i][0]) )
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
	for ( int i = 0; i < NUM_SPECIAL_WEAPON_TYPES; i++ )
	{
		pWeapon = Weapon_OwnsThisType( kSpecialWeaponTypes[i][0] );
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
		// Now we wait for our gamerules to respawn us
		// In the meantime, allow us to start roaming around the map as an observer
		StartObserverMode( OBS_MODE_ROAMING );
	}
}

// Added drop system
// Most of this is from base classes...
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
