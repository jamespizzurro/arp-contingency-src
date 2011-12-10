#ifndef NPC_CONTINGENCY_TARGET_H
#define NPC_CONTINGENCY_TARGET_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_bullseye.h"

class CNPC_Contingency_Target : public CNPC_Bullseye
{
	DECLARE_CLASS( CNPC_Contingency_Target, CNPC_Bullseye );

public:
	CNPC_Contingency_Target( void );
	~CNPC_Contingency_Target();

	Class_T Classify( void );

	bool CanBeSeenBy( CAI_BaseNPC *pNPC );
	bool CanBeAnEnemyOf( CBaseEntity *pEnemy );

protected:
	DECLARE_DATADESC();
};

#endif // NPC_CONTINGENCY_TARGET_H
