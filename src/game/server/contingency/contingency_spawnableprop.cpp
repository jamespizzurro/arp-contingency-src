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
}

CContingency_SpawnableProp::~CContingency_SpawnableProp( void )
{
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

	SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );	// collide with everything players collide with
	SetMoveType( MOVETYPE_NONE );	// only move due to gravity, nothing else
	AddEffects( EF_NOSHADOW );	// save some FPS (?)

	if ( VPhysicsGetObject() )
	{
		VPhysicsGetObject()->EnableCollisions( false );
		VPhysicsGetObject()->EnableMotion( false );
		VPhysicsGetObject()->EnableDrag( false );
		VPhysicsGetObject()->EnableGravity( false );
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
