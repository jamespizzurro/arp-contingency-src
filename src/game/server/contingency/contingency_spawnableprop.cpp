// Added spawnable prop system

#include "cbase.h"
#include "contingency_spawnableprop.h"
#include "contingency_gamerules.h"

#include "contingency_system_propspawning.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( contingency_spawnableprop, CContingency_SpawnableProp );

IMPLEMENT_SERVERCLASS_ST( CContingency_SpawnableProp, DT_Contingency_SpawnableProp )
	SendPropEHandle( SENDINFO(m_hSpawnerPlayer) ),
	SendPropBool( SENDINFO(m_bIsFrozen) ),
END_SEND_TABLE()

BEGIN_DATADESC( CContingency_SpawnableProp )
END_DATADESC()

CContingency_SpawnableProp::CContingency_SpawnableProp( void )
{
	m_bIsFrozen = false;
	m_hSpawnerPlayer = NULL;
	m_iSpawnablePropIndex = 0;
	pTarget = NULL;
}

CContingency_SpawnableProp::~CContingency_SpawnableProp( void )
{
	if ( pTarget )
	{
		UTIL_Remove( pTarget );
		pTarget = NULL;
	}

	// Remove us from our spawner player's spawnable prop list and stuff
	CContingency_Player *pSpawnerPlayer = ToContingencyPlayer( GetSpawnerPlayer() );
	if ( pSpawnerPlayer && (pSpawnerPlayer->m_SpawnablePropList.Find( this ) != -1) )
	{
		pSpawnerPlayer->m_SpawnablePropList.FindAndRemove( this );	// remove from our spawner's list of spawnable props
		pSpawnerPlayer->SetNumSpawnableProps( pSpawnerPlayer->GetNumSpawnableProps() - 1 );	// we have the 'find' bit exclusively for this
	}

	// Remove us from any player info records we belong to as well
	for ( int i = 0; (ContingencyRules() != NULL) && (i < ContingencyRules()->m_PlayerInfoList.Count()); i++ )
	{
		CContingency_Player_Info *pPlayerInfo = ContingencyRules()->m_PlayerInfoList.Element(i);
		if ( !pPlayerInfo )
			continue;

		if ( pPlayerInfo->spawnablePropList.Find( this ) != -1 )
		{
			// We belong to this particular player info record, but not for long; remove us from it!
			pPlayerInfo->spawnablePropList.FindAndRemove( this );
			pPlayerInfo->SetNumSpawnableProps( pPlayerInfo->GetNumSpawnableProps() - 1 );
		}
	}
}

void CContingency_SpawnableProp::Spawn( void )
{
	BaseClass::Spawn();

	SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	AddEffects( EF_NOSHADOW );	// save some FPS (?)
	AddSolidFlags( FSOLID_COLLIDE_WITH_OWNER );

	// Fade spawned prop in to the world
	SetRenderColorA( 0 );
	m_nRenderFX = kRenderFxSolidFast;
	m_flSpeed = gpGlobals->curtime + 3.0f;

	SetFrozenState( true );	// default to frozen

	pTarget = static_cast<CNPC_Contingency_Target*>( CreateEntityByName("npc_contingency_target") );
	if ( pTarget )
	{
		pTarget->SetAbsOrigin( this->GetAbsOrigin() );
		pTarget->SetAbsAngles( this->GetAbsAngles() );
		pTarget->AddSpawnFlags( SF_BULLSEYE_NONSOLID | SF_BULLSEYE_ENEMYDAMAGEONLY | SF_BULLSEYE_PERFECTACC );
		pTarget->Spawn();
		DispatchSpawn( pTarget );
		pTarget->Activate();
		pTarget->SetParent( this );
		pTarget->SetHealth( this->GetHealth() );
		pTarget->SetPainPartner( this );
	}

	SetNextThink( gpGlobals->curtime );
}

void CContingency_SpawnableProp::SetFrozenState( bool shouldFreeze )
{
	if ( shouldFreeze )
	{
		// Freeze us!

		SetMoveType( MOVETYPE_NONE );
		SetBlocksLOS( true );
		SetAIWalkable( true );

		if ( VPhysicsGetObject() )
		{
			VPhysicsGetObject()->EnableCollisions( false );
			VPhysicsGetObject()->EnableMotion( false );
			VPhysicsGetObject()->EnableDrag( false );
			VPhysicsGetObject()->EnableGravity( false );
		}
	}
	else
	{
		// Unfreeze us!

		SetMoveType( MOVETYPE_VPHYSICS );
		SetBlocksLOS( false );
		SetAIWalkable( false );

		if ( VPhysicsGetObject() )
		{
			VPhysicsGetObject()->EnableCollisions( true );
			VPhysicsGetObject()->EnableMotion( true );
			VPhysicsGetObject()->EnableDrag( true );
			VPhysicsGetObject()->EnableGravity( true );
		}
	}

	m_bIsFrozen = shouldFreeze;
}

void CContingency_SpawnableProp::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( pActivator );
	if ( !pPlayer )
		return;

	if ( ContingencyRules()->GetCurrentPhase() != PHASE_INTERIM )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Props can only be operated during interim phases." );
		return;	// we're only allowed to operate props during interim phases
	}

	if ( pPlayer != GetSpawnerPlayer() )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "This prop is not yours to operate." );
		return;	// only our spawner can operate us
	}

	if ( IsDissolving() )
		return;	// props that are already dissolving should be ignored

	pPlayer->SetSpawnablePropInFocus( this );
	pPlayer->ShowViewPortPanel( "spawnableprop_deletionmenu", true, NULL );
}

int CContingency_SpawnableProp::OnTakeDamage( const CTakeDamageInfo &info )
{
	// Don't let players damage each other's spawnable props
	// Players can still damage their own props though
	CBasePlayer *pPlayerAttacker = dynamic_cast<CBasePlayer*>( info.GetAttacker() );
	if ( pPlayerAttacker && (pPlayerAttacker != GetSpawnerPlayer()) )
		return 0;

	return BaseClass::OnTakeDamage( info );
}

void CContingency_SpawnableProp::Think( void )
{
	BaseClass::Think();

	// This is an absolutely horrible hack that prevents spawnable props from sleeping
	// when unfrozen so that we can still perform tracelines that require them to be awake
	// TODO: For the love of all that is good, come up with a better solution!
	if ( !m_bIsFrozen && VPhysicsGetObject() )
		VPhysicsGetObject()->Wake();

	SetNextThink( gpGlobals->curtime );
}

bool CContingency_SpawnableProp::OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if ( m_bIsFrozen )
		return false;	// frozen props cannot be picked up

	if ( pPhysGunUser != GetSpawnerPlayer() )
		return false;	// props that do not belong to us cannot be picked up

	return true;
}
