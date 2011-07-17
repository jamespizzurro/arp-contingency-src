// Add a custom rally point entity for wave spawners

#ifndef CONTINGENCY_RALLYPOINT_H
#define CONTINGENCY_RALLYPOINT_H
#ifdef _WIN32
#pragma once
#endif

class CContingencyRallyPoint : public CBaseAnimating
{
public:
	DECLARE_CLASS( CContingencyRallyPoint, CBaseAnimating );

	CContingencyRallyPoint();

	void Spawn( void );
};

#endif // CONTINGENCY_RALLYPOINT_H
