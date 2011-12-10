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
END_SEND_TABLE()

BEGIN_DATADESC( CContingency_SpawnableProp )
END_DATADESC()

CContingency_SpawnableProp::CContingency_SpawnableProp( void )
{
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

	CContingency_Player *pSpawnerPlayer = ToContingencyPlayer( GetSpawnerPlayer() );
	if ( pSpawnerPlayer && (pSpawnerPlayer->m_SpawnablePropList.Find( this ) != -1) )
	{
		pSpawnerPlayer->m_SpawnablePropList.FindAndRemove( this );	// remove from our spawner's list of spawnable props
		pSpawnerPlayer->SetNumSpawnableProps( pSpawnerPlayer->GetNumSpawnableProps() - 1 );	// we have the 'find' bit exclusively for this
	}
}

void CContingency_SpawnableProp::Spawn( void )
{
	BaseClass::Spawn();

	SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	SetMoveType( MOVETYPE_NONE );
	AddEffects( EF_NOSHADOW );	// save some FPS (?)
	SetBlocksLOS( true );
	SetAIWalkable( true );

	if ( VPhysicsGetObject() )
	{
		VPhysicsGetObject()->EnableCollisions( false );
		VPhysicsGetObject()->EnableMotion( false );
		VPhysicsGetObject()->EnableDrag( false );
		VPhysicsGetObject()->EnableGravity( false );
	}

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

	pPlayer->SetSpawnablePropInFocus( this );
	pPlayer->ShowViewPortPanel( "spawnableprop_deletionmenu", true, NULL );
}
