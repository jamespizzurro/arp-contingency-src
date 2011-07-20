// Added loadout system

#include "cbase.h"

#include "contingency_weapon_types.h"
#include "contingency_gamerules.h"

#ifdef CLIENT_DLL
	#include "c_contingency_player.h"
#else
	#include "contingency_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL
// Added loadout menu
void CC_LOADOUTMENU( const CCommand &args )
{
	CContingency_Player *pPlayer = ToContingencyPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	// We used to restrict when players were allowed to bring up this menu,
	// but that doesn't really make sense, hence this new system

	pPlayer->ShowViewPortPanel( "loadoutmenu", true, NULL );
}
static ConCommand showloadoutmenu( "showloadoutmenu", CC_LOADOUTMENU, "Shows the loadout menu for changing one's loadout (when permitted)" );
#endif
