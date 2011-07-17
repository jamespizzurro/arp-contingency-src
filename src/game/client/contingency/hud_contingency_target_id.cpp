// Added a modified target ID HUD element (based on hl2mp_hud_target_id.cpp)

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_playerresource.h"
#include "vgui_EntityPanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "contingency_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_ID_STRING 256
#define PLAYER_HINT_DISTANCE 150
#define PLAYER_HINT_DISTANCE_SQ (PLAYER_HINT_DISTANCE * PLAYER_HINT_DISTANCE)

class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CTargetID, vgui::Panel );

public:
	CTargetID( const char *pElementName );
	void Init( void );
	void ApplySchemeSettings( vgui::IScheme *scheme );
	void Paint( void );
	void VidInit( void );

private:
	Color			GetColorForTargetTeam( int iTeamNumber );

private:
	vgui::HFont		m_hFont;
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;
};

DECLARE_HUDELEMENT( CTargetID );

using namespace vgui;

CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

void CTargetID::Init( void )
{
};

void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont( "TargetID", IsProportional() );

	SetPaintBackgroundEnabled( false );
}

void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

Color CTargetID::GetColorForTargetTeam( int iTeamNumber )
{
	return GameResources()->GetTeamColor( iTeamNumber );
} 

void CTargetID::Paint()
{
	C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pLocalPlayer )
		return;

	// Get our target's ent index
	int iEntIndex = pLocalPlayer->GetIDTarget();
	// Didn't find one?
	if ( !iEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) )
		{
			m_flLastChangeTime = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
		m_flLastChangeTime = gpGlobals->curtime;

	// Is this an entindex sent by the server?
	if ( iEntIndex )
	{
		C_Contingency_Player *pTargetPlayer = ToContingencyPlayer( cl_entitylist->GetEnt(iEntIndex) );
		if ( !pTargetPlayer )
			return;	// as of right now, we only generate target IDs for players, not NPCs or other stuff

		int wide, tall;

		wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
		g_pVGuiLocalize->ConvertANSIToUnicode( pTargetPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
		vgui::surface()->GetTextSize( m_hFont, wszPlayerName, wide, tall );
		vgui::surface()->DrawSetTextFont( m_hFont );
		vgui::surface()->DrawSetTextPos( (ScreenWidth() - wide) / 2, YRES(260) );
		vgui::surface()->DrawSetTextColor( GetColorForTargetTeam(pTargetPlayer->GetTeamNumber()) );
		vgui::surface()->DrawPrintText( wszPlayerName, wcslen(wszPlayerName) );

		wchar_t wszHealthConditionText[ MAX_PLAYER_NAME_LENGTH ];
		g_pVGuiLocalize->ConvertANSIToUnicode( pTargetPlayer->GetHealthCondition(), wszHealthConditionText, sizeof(wszHealthConditionText) );
		vgui::surface()->GetTextSize( m_hFont, wszHealthConditionText, wide, tall );
		vgui::surface()->DrawSetTextFont( m_hFont );
		vgui::surface()->DrawSetTextPos( (ScreenWidth() - wide) / 2, YRES(280) );
		vgui::surface()->DrawSetTextColor( pTargetPlayer->GetHealthConditionColor() );
		vgui::surface()->DrawPrintText( wszHealthConditionText, wcslen(wszHealthConditionText) );
	}
}
