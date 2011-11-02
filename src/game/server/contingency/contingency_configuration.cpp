#include "cbase.h"

#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
// Added wave system
#define SF_CONFIGURATION_SUPPORTSHEADCRABS 1
#define SF_CONFIGURATION_SUPPORTSANTLIONS 16
#define SF_CONFIGURATION_SUPPORTSZOMBIES 32
#define SF_CONFIGURATION_SUPPORTSCOMBINE 64

class CContingencyConfiguration : public CBaseEntity
{
public:
	DECLARE_CLASS( CContingencyConfiguration, CBaseEntity );
	DECLARE_DATADESC();

public:
	CContingencyConfiguration();
	void Spawn( void );

	// Added phase system
	void GamemodeCheckThink( void );

private:	// map-defined stuff
	// Added phase system
	int m_iInterimPhaseLength;

	// Added radar display
	bool m_bAllowRadars;

	// Added spawnable prop system
	int m_iMaxPropsPerPlayer;

	// Added credits system
	int m_iStartingCredits;

	// Added wave system
	int m_iMaxLivingNPCs;
	float m_flHeadcrabWaveMultiplierOffset;
	float m_flAntlionWaveMultiplierOffset;
	float m_flZombieWaveMultiplierOffset;
	float m_flCombineWaveMultiplierOffset;

	// Added phase system
	COutputEvent m_OnInterimPhaseBegin;
	COutputEvent m_OnCombatPhaseBegin;

private:
	// Added phase system
	int m_iPhaseLastThink;
};

LINK_ENTITY_TO_CLASS( contingency_configuration, CContingencyConfiguration );

BEGIN_DATADESC( CContingencyConfiguration )
	// Added phase system
	DEFINE_KEYFIELD( m_iInterimPhaseLength, FIELD_INTEGER, "InterimPhaseLength" ),

	// Added radar display
	DEFINE_KEYFIELD( m_bAllowRadars, FIELD_BOOLEAN, "AllowRadars" ),

	// Added prop spawning system
	DEFINE_KEYFIELD( m_iMaxPropsPerPlayer, FIELD_INTEGER, "MaxPropsPerPlayer" ),
	
	// Added credits system
	DEFINE_KEYFIELD( m_iStartingCredits, FIELD_INTEGER, "StartingCredits" ),

	// Added wave system
	DEFINE_KEYFIELD( m_iMaxLivingNPCs, FIELD_INTEGER, "MaxLivingNPCs" ),
	DEFINE_KEYFIELD( m_flHeadcrabWaveMultiplierOffset, FIELD_FLOAT, "HeadcrabWaveMultiplierOffset" ),
	DEFINE_KEYFIELD( m_flAntlionWaveMultiplierOffset, FIELD_FLOAT, "AntlionWaveMultiplierOffset" ),
	DEFINE_KEYFIELD( m_flZombieWaveMultiplierOffset, FIELD_FLOAT, "ZombieWaveMultiplierOffset" ),
	DEFINE_KEYFIELD( m_flCombineWaveMultiplierOffset, FIELD_FLOAT, "CombineWaveMultiplierOffset" ),

	// Added phase system
	DEFINE_THINKFUNC( GamemodeCheckThink ),

	// Outputs
	// Added phase system
	DEFINE_OUTPUT( m_OnInterimPhaseBegin, "OnInterimPhaseBegin" ),
	DEFINE_OUTPUT( m_OnCombatPhaseBegin, "OnCombatPhaseBegin" ),
END_DATADESC()

CContingencyConfiguration::CContingencyConfiguration()
{
	// Added phase system
	m_iPhaseLastThink = PHASE_WAITING_FOR_PLAYERS;
}

void CContingencyConfiguration::Spawn( void )
{
	// Added phase system
	ContingencyRules()->SetMapInterimPhaseLength( m_iInterimPhaseLength );

	// Added radar display
	ContingencyRules()->DoesMapAllowRadars( m_bAllowRadars );

	// Added spawnable prop system
	ContingencyRules()->SetMapMaxPropsPerPlayer( m_iMaxPropsPerPlayer );

	// Added credits system
	ContingencyRules()->SetMapStartingCredits( m_iStartingCredits );

	// Added wave system
	// Check to see what types of waves our map supports by its spawnflags
	// and update our gamerules accordingly
	ContingencyRules()->SetMapMaxLivingNPCs( m_iMaxLivingNPCs );
	ContingencyRules()->DoesMapSupportHeadcrabs( HasSpawnFlags(SF_CONFIGURATION_SUPPORTSHEADCRABS) );
	ContingencyRules()->DoesMapSupportAntlions( HasSpawnFlags(SF_CONFIGURATION_SUPPORTSANTLIONS) );
	ContingencyRules()->DoesMapSupportZombies( HasSpawnFlags(SF_CONFIGURATION_SUPPORTSZOMBIES) );
	ContingencyRules()->DoesMapSupportCombine( HasSpawnFlags(SF_CONFIGURATION_SUPPORTSCOMBINE) );

	// Added wave system
	// Send our wave multiplier offsets to our gamerules
	ContingencyRules()->SetMapHeadcrabWaveMultiplierOffset( m_flHeadcrabWaveMultiplierOffset );
	ContingencyRules()->SetMapAntlionWaveMultiplierOffset( m_flAntlionWaveMultiplierOffset );
	ContingencyRules()->SetMapZombieWaveMultiplierOffset( m_flZombieWaveMultiplierOffset );
	ContingencyRules()->SetMapCombineWaveMultiplierOffset( m_flCombineWaveMultiplierOffset );

	// Added phase system
	// Handle periodic thinking
	SetThink( &CContingencyConfiguration::GamemodeCheckThink );
	SetNextThink( gpGlobals->curtime );	// start thinking
}

// Added phase system
void CContingencyConfiguration::GamemodeCheckThink()
{
	int m_iCurrentPhase = ContingencyRules()->GetCurrentPhase();
	if ( m_iCurrentPhase != m_iPhaseLastThink )
	{
		switch ( m_iCurrentPhase )
		{
		case PHASE_INTERIM:
			m_OnInterimPhaseBegin.FireOutput( this, this );
			break;
		case PHASE_COMBAT:
			m_OnCombatPhaseBegin.FireOutput( this, this );
			break;
		}

		m_iPhaseLastThink = m_iCurrentPhase;
	}

	SetNextThink( gpGlobals->curtime + 0.5f );	// think pretty frequently (every half-second)
}
