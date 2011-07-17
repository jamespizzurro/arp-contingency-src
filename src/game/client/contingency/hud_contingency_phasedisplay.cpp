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
class CHudContingencyPhaseDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudContingencyPhaseDisplay, vgui::Panel );

public:
	CHudContingencyPhaseDisplay( const char *pElementName );
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

	char text[256];
};

DECLARE_HUDELEMENT( CHudContingencyPhaseDisplay );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudContingencyPhaseDisplay::CHudContingencyPhaseDisplay( const char *pElementName ) : BaseClass(NULL, "HudContingencyPhaseDisplay"), CHudElement( pElementName )
{
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

void CHudContingencyPhaseDisplay::Init()
{
	Reset();
}

void CHudContingencyPhaseDisplay::Reset( void )
{
	m_flUpdateDelay = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudContingencyPhaseDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
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
void CHudContingencyPhaseDisplay::PerformLayout()
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

	m_pBackground->SetBounds( 0, 0, labelWide, labelTall + 16 );	// the +16 bit is a blatent hack
																	// to factor in the '\n' escape character

	int xOffset = (labelWide - m_pWarmupLabel->GetWide())/2;
	m_pWarmupLabel->SetPos( 0 + xOffset, 0 + m_iTextY );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudContingencyPhaseDisplay::OnThink()
{
	if ( gpGlobals->curtime >= m_flUpdateDelay )
	{
		SetVisible( false );

		if ( ContingencyRules() )
		{
			m_pBackground->SetFgColor( GetFgColor() );
			m_pWarmupLabel->SetFgColor( Color(255, 255, 255, 255) );

			if ( ContingencyRules()->GetCurrentPhase() == PHASE_INTERIM )
				Q_snprintf( text, sizeof(text), "%s:\n%i seconds remaining before wave %i",
				ContingencyRules()->GetCurrentPhaseName(),
				ContingencyRules()->GetInterimPhaseTimeLeft(),
				ContingencyRules()->GetWaveNumber() + 1 );
			else
				Q_snprintf( text, sizeof(text), "%s:\nWave %i (%i enemies remaining)",
				ContingencyRules()->GetCurrentPhaseName(),
				ContingencyRules()->GetWaveNumber(),
				ContingencyRules()->GetNumEnemiesRemaining() );

			m_pWarmupLabel->SetText( text );
			m_pWarmupLabel->SetVisible( true );
			m_pWarmupLabel->SizeToContents();

			InvalidateLayout();

			SetVisible( true );
		}

		m_flUpdateDelay = gpGlobals->curtime + 0.5f;	// update every half-second
	}
}
