#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_contingency_player.h"

	// Revert to normal HL2 footsteps
	#include "prediction.h"
#else
	#include "contingency_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Revert to normal HL2 footsteps
extern ConVar sv_footsteps;

// Revert to normal HL2 footsteps
void CContingency_Player::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	if ( (gpGlobals->maxClients > 1) && !sv_footsteps.GetFloat() )
		return;

#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( GetFlags() & FL_DUCKING )
		return;

	CBasePlayer::PlayStepSound( vecOrigin, psurface, fvol, force );
}
