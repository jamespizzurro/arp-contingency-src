// Added credits system
// Added credits display

#include "cbase.h"

#include <vgui/ILocalize.h>

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "c_contingency_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define THINK_DELAY 0.5f

using namespace vgui;

class CHudCreditsDisplay : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudCreditsDisplay, CHudNumericDisplay );

public:
	CHudCreditsDisplay( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset( void );
	void OnThink();

private:
	float m_flNextThinkTime;
	int m_iCredits;
};	

DECLARE_HUDELEMENT( CHudCreditsDisplay );

CHudCreditsDisplay::CHudCreditsDisplay( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudCreditsDisplay" )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudCreditsDisplay::Init( void )
{
	Reset();
}

void CHudCreditsDisplay::VidInit( void )
{
	Reset();
}

void CHudCreditsDisplay::Reset( void )
{
	m_flNextThinkTime = 0.0f;
	m_iCredits = 0;

	wchar_t *tempString = g_pVGuiLocalize->Find( "#Contingency_HUD_CreditsDisplay" );
	if (tempString)
		SetLabelText( tempString );
	else
		SetLabelText( L"CREDITS" );

	SetDisplayValue( m_iCredits );
}

void CHudCreditsDisplay::OnThink( void )
{
	if ( gpGlobals->curtime >= m_flNextThinkTime )
	{
		C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
		if ( !pLocalPlayer )
			return;

		SetDisplayValue( pLocalPlayer->GetCredits() );

		m_flNextThinkTime = gpGlobals->curtime + THINK_DELAY;	// think periodically
	}
}
