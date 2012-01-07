// Added spawnable prop system

#include "cbase.h"
#include "contingency_spawnableprop.h"
#include "contingency_gamerules.h"

#include "contingency_system_propspawning.h"

#include "props_shared.h"
#include "npc_citizen17.h"

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

Class_T CContingency_SpawnableProp::Classify( void )
{
	return CLASS_CONTINGENCY_SPAWNABLE_PROP;

	// See Class_T enumerator in baseentity.h for
	// a description of CLASS_CONTINGENCY_SPAWNABLE_PROP
}

void CContingency_SpawnableProp::Spawn( void )
{
	Precache();
	// Model should be set by what is spawning us

	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_COLLIDE_WITH_OWNER );
	SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	SetMoveType( MOVETYPE_NONE );
	SetBlocksLOS( true );
	SetAIWalkable( true );
	SetGravity( 0.0 );
	AddEffects( EF_NOSHADOW );	// save some FPS (?)
	SetBloodColor( BLOOD_COLOR_MECH );	// change to DONT_BLEED?

	m_flFieldOfView = 0.0f;

	AddFlag( FL_NPC );

	if ( VPhysicsGetObject() )
	{
		VPhysicsGetObject()->EnableCollisions( false );
		VPhysicsGetObject()->EnableMotion( false );
		VPhysicsGetObject()->EnableDrag( false );
		VPhysicsGetObject()->EnableGravity( false );
	}
	
	m_takedamage = DAMAGE_YES;

	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	// Our initial "health" depends on what type of spawnable prop we are
	// and therefore should be set by whatever is spawning us

	// Fade spawned prop in to the world
	SetRenderColorA( 0 );
	m_nRenderFX = kRenderFxSolidFast;
	m_flSpeed = gpGlobals->curtime + 3.0f;
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
	// Don't let players damage each other's spawned props
	// Players can still damage their own spawned props though
	CBasePlayer *pPlayerAttacker = ToBasePlayer( info.GetAttacker() );
	if ( pPlayerAttacker && (pPlayerAttacker != GetSpawnerPlayer()) )
		return 0;

	if ( info.GetAttacker() && info.GetAttacker()->IsNPC() )
	{
		// Don't let support NPCs do damage to us
		CNPC_Citizen *pCitizenAttacker = dynamic_cast<CNPC_Citizen*>( info.GetAttacker() );
		if ( pCitizenAttacker )
			return 0;

		// Don't let deployed turrets do damage to us
		CNPC_FloorTurret *pTurretAttacker = dynamic_cast<CNPC_FloorTurret*>( info.GetAttacker() );
		if ( pTurretAttacker )
			return 0;
	}

	return BaseClass::OnTakeDamage( info );
}

void CContingency_SpawnableProp::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	if ( !IsDissolving() )	// don't break if we're dissolving
	{
//
// This next block of code is taken directly from CAI_BaseNPC::Break,
// which is a private function of CAI_BaseNPC

	m_takedamage = DAMAGE_NO;

	Vector velocity;
	AngularImpulse angVelocity;
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	Vector origin;
	QAngle angles;
	AddSolidFlags( FSOLID_NOT_SOLID );
	if ( pPhysics )
	{
		pPhysics->GetVelocity( &velocity, &angVelocity );
		pPhysics->GetPosition( &origin, &angles );
		pPhysics->RecheckCollisionFilter();
	}
	else
	{
		velocity = GetAbsVelocity();
		QAngleToAngularImpulse( GetLocalAngularVelocity(), angVelocity );
		origin = GetAbsOrigin();
		angles = GetAbsAngles();
	}

	breakablepropparams_t params( GetAbsOrigin(), GetAbsAngles(), velocity, angVelocity );
	params.impactEnergyScale = m_impactEnergyScale;
	params.defCollisionGroup = GetCollisionGroup();
	if ( params.defCollisionGroup == COLLISION_GROUP_NONE )
	{
		// don't automatically make anything COLLISION_GROUP_NONE or it will
		// collide with debris being ejected by breaking
		params.defCollisionGroup = COLLISION_GROUP_INTERACTIVE;
	}

	// no damage/damage force? set a burst of 100 for some movement
	params.defBurstScale = 100;//pDamageInfo ? 0 : 100;
	PropBreakableCreateAll( GetModelIndex(), pPhysics, params, this, -1, false );

// END of code block from CAI_BaseNPC::Break
//
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
	SetThink( &CBaseEntity::SUB_Remove );
}
