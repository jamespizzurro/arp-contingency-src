#include "cbase.h"
#include "c_contingency_player.h"

#include "takedamageinfo.h"
#include "contingency_gamerules.h"

// Don't alias here
#if defined( CContingency_Player )
#undef CContingency_Player	
#endif

LINK_ENTITY_TO_CLASS( player, C_Contingency_Player );

BEGIN_RECV_TABLE_NOBASE( C_Contingency_Player, DT_SOLocalPlayerExclusive )
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_Contingency_Player, DT_SONonLocalPlayerExclusive )
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_Contingency_Player, DT_Contingency_Player, CContingency_Player )
	// Add a custom maximum health variable so that the client can get a player's maximum health
	RecvPropInt( RECVINFO( m_iHealthMax ) ),

	// Added loadout system
	RecvPropInt( RECVINFO( m_iCurrentLoadout ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_Contingency_Player )
END_PREDICTION_DATA()

C_Contingency_Player::C_Contingency_Player()
{
	// Added loadout system
	// Added loadout menu
	m_iSelectedLoadout = -1;
}

C_Contingency_Player::~C_Contingency_Player()
{
}

C_Contingency_Player* C_Contingency_Player::GetLocalContingencyPlayer()
{
	return (C_Contingency_Player*)C_BasePlayer::GetLocalPlayer();
}

void C_Contingency_Player::ClientThink( void )
{
	BaseClass::ClientThink();
}

// Do not allow players to hurt each other
// Refrain from using any blood effects in such instances to reinforce this idea
// Largely the same as C_HL2MP_Player::TraceAttack with a few modifications
void SpawnBlood( Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage );
void C_Contingency_Player::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	Vector vecOrigin = ptr->endpos - vecDir * 4;

	float flDistance = 0.0f;
	
	if ( info.GetAttacker() )
	{
		flDistance = (ptr->endpos - info.GetAttacker()->GetAbsOrigin()).Length();
	}

	if ( m_takedamage )
	{
		AddMultiDamage( info, this );

		int blood = BloodColor();
		
		CBaseEntity *pAttacker = info.GetAttacker();

		if ( pAttacker )
		{
			// This is the important new bit right here:
			if ( !ContingencyRules()->FPlayerCanTakeDamage(this, pAttacker) )
				return;
		}

		if ( blood != DONT_BLEED )
		{
			SpawnBlood( vecOrigin, vecDir, blood, flDistance );// a little surface blood.
			TraceBleed( flDistance, vecDir, ptr, info.GetDamageType() );
		}
	}
}

// Added loadout system
const char *C_Contingency_Player::GetLoadoutName( int loadout )
{
	switch ( loadout )
	{
	case LOADOUT_SOLDIER:
		return "Rebel Soldier";
	case LOADOUT_SHOTGUN_SOLDIER:
		return "Rebel Shotgun Soldier";
	case LOADOUT_COMMANDER:
		return "Rebel Commander";
	case LOADOUT_MARKSMAN:
		return "Rebel Marksman";
	case LOADOUT_DEMOLITIONIST:
		return "Rebel Demolitionist";
	}

	return "";
}

// Added player status HUD element
const char* C_Contingency_Player::GetHealthCondition( void )
{
	// If we're not playing, well, that's easy!
	if ( !ContingencyRules()->IsPlayerPlaying(this) )
		return "DEAD";

	float ratio = ((float)GetHealth()) / ((float)GetMaxHealth());
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return "Healthy";
		else if ( ratio >= 0.50 )
			return "Hurt";
		else if ( ratio >= 0.25 )
			return "Wounded";
		else if ( ratio < 0.25 )
			return "Near Death";
	}

	return "Undetermined";
}

// Added player status HUD element
Color C_Contingency_Player::GetHealthConditionColor( void )
{
	// If we're not playing, well, that's easy!
	if ( !ContingencyRules()->IsPlayerPlaying(this) )
		return Color( 204, 0, 0, 255 );	// dark(er) red

	float ratio = ((float)GetHealth()) / ((float)GetMaxHealth());
	if ( (ratio <= 1.00) && (ratio >= 0.00) )
	{
		if ( ratio >= 0.75 )
			return Color( 0, 255, 0, 255 );	// green
		else if ( ratio >= 0.50 )
			return Color( 255, 204, 0, 255 );	// yellow
		else if ( ratio >= 0.25 )
			return Color( 255, 153, 0, 255 );	// orange
		else if ( ratio < 0.25 )
			return Color( 255, 0, 0, 255 );	// red
	}

	return Color( 255, 255, 255, 255 );	// white
}

// Added sound cue and background music system
void PlayBackgroundMusic_f( void )
{
	ContingencyRules()->PlayBackgroundMusic();
}
ConCommand playbackgroundmusic( "playbackgroundmusic", PlayBackgroundMusic_f, "Plays some random background music", 0 );

// Added sound cue and background music system
void StopPlayingBackgroundMusic_f( void )
{
	ContingencyRules()->StopPlayingBackgroundMusic();
}
ConCommand stopplayingbackgroundmusic( "stopplayingbackgroundmusic", StopPlayingBackgroundMusic_f, "Stops any background music that might be playing", 0 );
