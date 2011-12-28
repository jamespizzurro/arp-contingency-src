// Added spawnable prop system

#include "cbase.h"

#include <vgui/ILocalize.h>

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define THINK_DELAY 0.5f

using namespace vgui;

class CHudNumPropsDisplay : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudNumPropsDisplay, CHudNumericDisplay );

public:
	CHudNumPropsDisplay( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset( void );
	void OnThink();

private:
	float m_flNextThinkTime;
};	

DECLARE_HUDELEMENT( CHudNumPropsDisplay );

CHudNumPropsDisplay::CHudNumPropsDisplay( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudNumPropsDisplay" )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudNumPropsDisplay::Init( void )
{
	Reset();
}

void CHudNumPropsDisplay::VidInit( void )
{
	Reset();
}

void CHudNumPropsDisplay::Reset( void )
{
	m_flNextThinkTime = 0.0f;

	wchar_t *tempString = g_pVGuiLocalize->Find( "#Contingency_HUD_NumPropsDisplay" );
	if ( tempString )
		SetLabelText( tempString );
	else
		SetLabelText( L"PROP COUNT" );

	SetDisplayValue( 0 );
	SetSecondaryValue( 0 );

	SetShouldDisplayValue( true );
	SetShouldDisplaySecondaryValue( true );
}

void CHudNumPropsDisplay::OnThink( void )
{
	if ( gpGlobals->curtime >= m_flNextThinkTime )
	{
		C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
		
		if ( pLocalPlayer )
			SetDisplayValue( pLocalPlayer->GetNumSpawnableProps() );

		if ( ContingencyRules() )
			SetSecondaryValue( ContingencyRules()->GetMapMaxPropsPerPlayer() );

		m_flNextThinkTime = gpGlobals->curtime + THINK_DELAY;	// think periodically
	}
}
