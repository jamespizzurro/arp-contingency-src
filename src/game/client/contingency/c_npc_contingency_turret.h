// Contingency - James
// Added a modified version of Valve's floor turret
// Implement client-side turret NPC class
// Not much is commented here, so...

#ifndef C_NPC_CONTINGENCY_TURRET_H
#define C_NPC_CONTINGENCY_TURRET_H
#pragma once

#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_FloorTurret : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_FloorTurret, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	C_NPC_FloorTurret();
	virtual	~C_NPC_FloorTurret();

	// Added turret status HUD element
	const char* GetOwnerDisplay( void );
	const char* GetHealthCondition( void );
	Color GetHealthConditionColor( void );

	virtual	bool ShouldCollide( int collisionGroup, int contentsMask ) const;

private:
	C_NPC_FloorTurret( const C_NPC_FloorTurret & ); // not defined, not accessible
};

#endif // C_NPC_CONTINGENCY_TURRET_H
