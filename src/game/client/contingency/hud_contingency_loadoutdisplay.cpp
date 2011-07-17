// Added phase system

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

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudContingencyLoadoutDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudContingencyLoadoutDisplay, vgui::Panel );

public:
	CHudContingencyLoadoutDisplay( const char *pElementName );
	void Init( void );
	void Reset();

	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

private:
	vgui::HFont m_hFont;
	Color		m_bgColor;

	vgui::Label *m_pWarmupLabel;	// "Warmup Mode"

	vgui::Label *m_pBackground;		// black box

	CPanelAnimationVarAliasType( int, m_iTextX, "text_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "8", "proportional_int" );

	float m_flUpdateDelay;
};

DECLARE_HUDELEMENT( CHudContingencyLoadoutDisplay );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudContingencyLoadoutDisplay::CHudContingencyLoadoutDisplay( const char *pElementName ) : BaseClass(NULL, "HudContingencyLoadoutDisplay"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	SetAlpha( 255 );

	m_flUpdateDelay = 0.0f;

	m_pBackground = new vgui::Label( this, "Background", "" );

	m_pWarmupLabel = new vgui::Label( this, "RoundState_warmup", "test label" /*g_pVGuiLocalize->Find( "#Clan_warmup_mode" )*/ );
	m_pWarmupLabel->SetPaintBackgroundEnabled( false );
	m_pWarmupLabel->SetPaintBorderEnabled( false );
	m_pWarmupLabel->SizeToContents();
	m_pWarmupLabel->SetContentAlignment( vgui::Label::a_west );
	m_pWarmupLabel->SetFgColor( GetFgColor() );
}

void CHudContingencyLoadoutDisplay::Init()
{
	Reset();
}

void CHudContingencyLoadoutDisplay::Reset( void )
{
	m_flUpdateDelay = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudContingencyLoadoutDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( Color(0,0,0,0) );	//GetSchemeColor("RoundStateFg", pScheme) );
	m_hFont = pScheme->GetFont( "Default", true );

	m_pBackground->SetBgColor( GetSchemeColor("BgColor", pScheme) );
	m_pBackground->SetPaintBackgroundType( 2 );

	SetAlpha( 255 );
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetPaintBackgroundType( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Resizes the label
//-----------------------------------------------------------------------------
void CHudContingencyLoadoutDisplay::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	// find the widest line
	int labelWide = m_pWarmupLabel->GetWide();

	// find the total height
	int fontTall = vgui::surface()->GetFontTall( m_hFont );
	int labelTall = fontTall;

	labelWide += m_iTextX*2;
	labelTall += m_iTextY*2;

	m_pBackground->SetBounds( 0, 0, labelWide, labelTall );

	int xOffset = (labelWide - m_pWarmupLabel->GetWide())/2;
	m_pWarmupLabel->SetPos( 0 + xOffset, 0 + m_iTextY );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudContingencyLoadoutDisplay::OnThink()
{
	if ( gpGlobals->curtime >= m_flUpdateDelay )
	{
		SetVisible( false );

		C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
		if ( pLocalPlayer )
		{
			m_pBackground->SetFgColor( GetFgColor() );
			m_pWarmupLabel->SetFgColor( Color(255, 255, 255, 255) );

			m_pWarmupLabel->SetText( pLocalPlayer->GetCurrentLoadoutName() );
			m_pWarmupLabel->SetVisible( true );
			m_pWarmupLabel->SizeToContents();

			InvalidateLayout();

			SetVisible( true );
		}

		m_flUpdateDelay = gpGlobals->curtime + 0.5f;	// update every half-second
	}
}
