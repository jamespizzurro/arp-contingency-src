// Contingency - James

#ifndef C_NPC_CITIZEN17_H
#define C_NPC_CITIZEN17_H
#pragma once

#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Citizen : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Citizen, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	C_NPC_Citizen();
	virtual	~C_NPC_Citizen();

	// Added citizen status HUD element
	const char* GetHealthCondition( void );
	Color GetHealthConditionColor( void );

private:
	C_NPC_Citizen( const C_NPC_Citizen & ); // not defined, not accessible
};

#endif // C_NPC_CITIZEN17_H
