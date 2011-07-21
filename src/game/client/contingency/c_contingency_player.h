#ifndef CONTINGENCY_PLAYER_H
#define CONTINGENCY_PLAYER_H
#pragma once

#include "c_hl2mp_player.h"
#include "contingency_player_shared.h"

//=============================================================================
// >> Contingency_Player
//=============================================================================
class C_Contingency_Player : public C_HL2MP_Player
{
public:
	DECLARE_CLASS( C_Contingency_Player, C_HL2MP_Player );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_Contingency_Player();
	~C_Contingency_Player( void );

	static C_Contingency_Player* GetLocalContingencyPlayer();

public:
	void ClientThink( void );

	// Do not allow players to hurt each other
	// Refrain from using any blood effects in such instances to reinforce this idea
	// Largely the same as C_HL2MP_Player::TraceAttack with a few modifications
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );

	// Revert to HL2 footsteps
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	// Add a custom maximum health variable so that the client can get a player's maximum health
	int GetMaxHealth( void ) { return m_iHealthMax; }

	// Added player status HUD element
	const char* GetHealthCondition( void );
	Color GetHealthConditionColor( void );

	// Added sound cue and background music system
	int GetAmbientSoundGUID( void ) { return m_iAmbientSoundGUID; }
	void SetAmbientSoundGUID( int newAmbientSoundGUID ) { m_iAmbientSoundGUID = newAmbientSoundGUID; }

private:
	C_Contingency_Player( const C_Contingency_Player & );

	// Add a custom maximum health variable so that the client can get a player's maximum health
	int m_iHealthMax;

	// Added sound cue and background music system
	int m_iAmbientSoundGUID;
};

inline C_Contingency_Player *ToContingencyPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_Contingency_Player*>( pEntity );
}

#endif // CONTINGENCY_PLAYER_H
