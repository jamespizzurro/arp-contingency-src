// Added phase system

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_playerresource.h"
#include "vgui_EntityPanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "contingency_gamerules.h"

#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudContingencyWaveDisplay : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudContingencyWaveDisplay, Panel );

public:
	CHudContingencyWaveDisplay( const char *pElementName );

	void Init( void );
	void Reset();
	void PerformLayout();

protected:
	void ApplySchemeSettings( IScheme *pScheme );
	void OnThink();

private:
	Label *m_pBackground;	// black box
	ImagePanel* m_pWaveTypeImagePanel;

	float m_flUpdateDelay;
	const char *imagePath;
};

DECLARE_HUDELEMENT( CHudContingencyWaveDisplay );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudContingencyWaveDisplay::CHudContingencyWaveDisplay( const char *pElementName ) : BaseClass(NULL, "HudContingencyWaveDisplay"), CHudElement( pElementName )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetProportional( true );
	SetVisible( false );
	SetAlpha( 0 );

	m_flUpdateDelay = 0.0f;

	m_pBackground = new Label( this, "Background", "" );
	m_pBackground->SetProportional( true );

	m_pWaveTypeImagePanel = new ImagePanel( this, "WaveTypeImage" );
	m_pWaveTypeImagePanel->SetProportional( true );
	m_pWaveTypeImagePanel->SetPos( 0, 0 );
	m_pWaveTypeImagePanel->SetVisible( false );
	m_pWaveTypeImagePanel->SetAlpha( 0 );
}

void CHudContingencyWaveDisplay::Init()
{
	Reset();
}

void CHudContingencyWaveDisplay::Reset( void )
{
	m_flUpdateDelay = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudContingencyWaveDisplay::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( Color(0, 0, 0, 0) );
	SetBgColor( Color(0, 0, 0, 0) );
	SetPaintBackgroundType( 0 );

	m_pBackground->SetBgColor( GetSchemeColor("BgColor", pScheme) );
	m_pBackground->SetPaintBackgroundType( 2 );
	m_pBackground->SetFgColor( GetFgColor() );
}

//-----------------------------------------------------------------------------
// Purpose: Resizes the label
//-----------------------------------------------------------------------------
void CHudContingencyWaveDisplay::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pBackground->SetBounds( 0, 0, GetWide(), GetTall() );	// fill
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudContingencyWaveDisplay::OnThink()
{
	if ( gpGlobals->curtime >= m_flUpdateDelay )
	{
		// Initially, image is hidden (we must pass our checks to display it!)
		m_pWaveTypeImagePanel->SetVisible( false );
		m_pWaveTypeImagePanel->SetAlpha( 0 );
		m_pBackground->SetVisible( true );
		m_pBackground->SetAlpha( 255 );
		SetVisible( true );

		if ( ContingencyRules() )	// make sure gamerules has been loaded to prevent nasty crashes
		{
			if ( ContingencyRules()->GetCurrentPhase() == PHASE_COMBAT )
			{
				// Determine what (if any) image we should show according to the current wave type
				imagePath = "";
				if ( ContingencyRules()->IsChallengeWave() )
					imagePath = "HUDicons/icon_wave_challenge";
				else
				{
					switch ( ContingencyRules()->GetWaveType() )
					{
					/*case WAVE_HEADCRABS:
						imagePath = "HUDicons/icon_wave_headcrab";
						break;*/
					case WAVE_ANTLIONS:
						imagePath = "HUDicons/icon_wave_antlion";
						break;
					case WAVE_ZOMBIES:
						imagePath = "HUDicons/icon_wave_zombie";
						break;
					case WAVE_COMBINE:
						imagePath = "HUDicons/icon_wave_combine";
						break;
					}
				}

				if ( Q_strcmp(imagePath, "") != 0 )
				{
					// Update our image
					m_pWaveTypeImagePanel->SetImage( scheme()->GetImage(imagePath, false) );
					m_pWaveTypeImagePanel->SetSize( GetWide(), GetTall() );
					m_pWaveTypeImagePanel->SetShouldScaleImage( true );

					// Show the result
					m_pWaveTypeImagePanel->SetVisible( true );
					m_pWaveTypeImagePanel->SetAlpha( 255 );
				}
			}
		}

		m_flUpdateDelay = gpGlobals->curtime + 0.5f;	// update every half-second
	}
}
