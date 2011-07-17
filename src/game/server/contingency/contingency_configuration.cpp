// Added wave system

#include "cbase.h"

#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CContingencyConfiguration : public CBaseEntity
{
public:
	DECLARE_CLASS( CContingencyConfiguration, CBaseEntity );
	DECLARE_DATADESC();

public:
	CContingencyConfiguration();
	void Spawn( void );
	void GamemodeCheckThink( void );

private:	// map-defined stuff
	int m_iMapPreferredWaveType;

	COutputEvent m_OnInterimPhaseBegin;
	COutputEvent m_OnCombatPhaseBegin;

private:
	int m_iPhaseLastThink;
};

LINK_ENTITY_TO_CLASS( contingency_configuration, CContingencyConfiguration );

BEGIN_DATADESC( CContingencyConfiguration )
	DEFINE_KEYFIELD( m_iMapPreferredWaveType, FIELD_INTEGER, "PreferredWaveType" ),

	DEFINE_THINKFUNC( GamemodeCheckThink ),

	// Outputs
	DEFINE_OUTPUT( m_OnInterimPhaseBegin, "OnInterimPhaseBegin" ),
	DEFINE_OUTPUT( m_OnCombatPhaseBegin, "OnCombatPhaseBegin" ),
END_DATADESC()

CContingencyConfiguration::CContingencyConfiguration()
{
	m_iPhaseLastThink = PHASE_WAITING_FOR_PLAYERS;
}

void CContingencyConfiguration::Spawn( void )
{
	// Set our map's preferred wave type
	ContingencyRules()->SetMapPreferredWaveType( m_iMapPreferredWaveType );

	SetThink( &CContingencyConfiguration::GamemodeCheckThink );
	SetNextThink( gpGlobals->curtime );	// start thinking
}

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
