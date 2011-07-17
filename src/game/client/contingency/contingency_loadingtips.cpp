// Added loading screen tips

#include "cbase.h"

#include "contingency_loadingtips.h"

#include <vgui/IVGui.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <FileSystem.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const char* kTipHeader		= "loading/tip";
static const char* kMapHeader		= "loading/map";
static const char* kPreviewImage	= "loading/background";
static const char* kTipBackground	= "loading/tipbackground";

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContingencyLoadingTips::CContingencyLoadingTips(Panel *parent) : Frame(parent, NULL, parent ? false : true)
{
	SetTitle("", true);
	SetVisible(false);

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible( false );
	SetProportional(true);

	SetPaintBorderEnabled(false);

	m_pMapHeader = new ImagePanel( this, "mapheader" );
	m_pMapHeader->SetImage( kMapHeader );
	m_pMapName = new Label( this, "mapname", "#Contingency_Loading_MapName" );

	m_pTipHeader = new ImagePanel( this, "tipheader" );
	m_pTipHeader->SetImage( kTipHeader );
	m_pTipPanel = new Label( this, "tippanel", "#Contingency_Loading_TipLabel" );

	m_pTip = new Label( m_pTipPanel, "tip", "");
	m_pHTMLPanel = new HTML( this, "htmlpanel", false );

	m_pPreviewImage = new ImagePanel( this, "previewimage" );
	m_pPreviewImage->SetImage( kPreviewImage );

	m_pTipBackground = new ImagePanel( this, "tipbackground" );
	m_pTipBackground->SetImage( kTipBackground );

	m_iTipIndex = 0;
	m_bShownTip = false;

	ivgui()->AddTickSignal(GetVPanel(), 1);

	ListenForGameEvent( "game_newmap" );

	KeyValues *manifest = new KeyValues( "tips" );
	if ( manifest->LoadFromFile( filesystem, "resource/tips.txt", "GAME" ) )
	{
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			if ( !Q_stricmp( sub->GetName(), "tip" ) )
			{
				int length = Q_strlen( sub->GetString() ) + 1;
				char *tip = (char *)malloc(length);

				Q_strcpy(tip, sub->GetString());

				m_RandomTip.AddToTail(tip);
				tip = NULL;
			}
			else
			{
				Error( "Expecting 'file', got %s\n", sub->GetName() );
			}
		}
	}
	manifest->deleteThis();

	// do initial shuffle
	for (int i=0; i<m_RandomTip.Count(); i++)
	{
		swap(m_RandomTip[i],
			m_RandomTip[random->RandomInt( 0, m_RandomTip.Count() - 1 )]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContingencyLoadingTips::~CContingencyLoadingTips()
{
	ivgui()->RemoveTickSignal(GetVPanel());

	for (int i=0; i < m_RandomTip.Count(); i++)
	{
		free(m_RandomTip[i]);
		m_RandomTip[i] = NULL;
	}
	m_RandomTip.Purge();
}

void CContingencyLoadingTips::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(Color( 15,15,15,255 ) ); // make the background black
	SetPaintBorderEnabled(false);

	SetBorder( NULL );

	m_pMapName->SetContentAlignment( Label::a_west );
	m_pMapName->SetBgColor(Color(0,0,0,0));

	m_pTip->SetContentAlignment( Label::a_northwest );
	m_pTip->SetWrap(true);
	m_pTip->SetBgColor( GetBgColor() );

	m_pTipPanel->SetContentAlignment( Label::a_northwest );
	m_pTipPanel->SetBorder( pScheme->GetBorder( "WhiteLineBorder" ) );
	m_pTipPanel->SetBgColor( GetBgColor() );

	m_pHTMLPanel->SetBorder( pScheme->GetBorder( "WhiteLineBorder" ) );
	m_pHTMLPanel->SetBgColor( GetBgColor() );

	m_pMapHeader->SetShouldScaleImage(true);
	m_pTipHeader->SetShouldScaleImage(true);
	m_pPreviewImage->SetShouldScaleImage(true);
	m_pTipBackground->SetShouldScaleImage(true);
}

void CContingencyLoadingTips::PerformLayout()
{
	int w,h;
	GetHudSize(w, h);

	// fill the screen
	SetBounds(0,0,w,h);

	// Background image (takes up the entire screen)
	m_pPreviewImage->SetBounds( 0, 0, w, h );
	m_pPreviewImage->SetZPos( -2 );

	// Position the HTML panel top-center
	m_pHTMLPanel->SetSize( (w/2) - 20, (h/2) - 20 );
	m_pHTMLPanel->SetPos( w/2 - (m_pHTMLPanel->GetWide()/2), 50 );

	m_pMapName->SetText("M");
	int wide, tall;
	m_pMapName->GetContentSize(wide, tall);
	wide *= MAX_MAP_NAME;

	m_pMapName->SetBounds( w/2 - (m_pHTMLPanel->GetWide()/2), 10, wide + 15, tall + 18);
	m_pMapName->SetTextInset(15, 0);
	m_pMapName->SetFgColor( Color(255,255,255,255) );
	m_pMapName->SetText("#Contingency_Loading_MapName");

	m_pTipPanel->GetContentSize(wide, tall);
	tall *= 6;

	// Background for tip area (quite a hack, uses an image since SetBgColor does appear to work with Labels)
	m_pTipBackground->SetBounds( w/2 - (m_pHTMLPanel->GetWide()/2), 50 + m_pHTMLPanel->GetTall() + 50,
		w / 2 - 20, tall + 63);
	m_pTipBackground->SetZPos( -1 );

	m_pTipPanel->SetTextInset(15, 10);
	m_pTipPanel->SetBounds( w/2 - (m_pHTMLPanel->GetWide()/2), 50 + m_pHTMLPanel->GetTall() + 50,
		w / 2 - 20, tall + 63);
	m_pTipPanel->SetFgColor( Color(255,255,255,255) );

	m_pTip->SetBounds(15, 48, m_pTipPanel->GetWide() - 30, tall);
	m_pTip->SetFgColor( Color(255,255,255,255) );

	int x, y;
	m_pTipPanel->GetPos(x, y);

	m_pTipHeader->SetBounds(x+2, y+2, m_pTipPanel->GetWide()-4, 32);

	m_pMapName->GetPos(x, y);
	m_pMapHeader->SetBounds(x, y, m_pMapName->GetWide() / 2, m_pMapName->GetTall());
}

void CContingencyLoadingTips::FireGameEvent( IGameEvent * event )
{	
	const char *type = event->GetName();

	if ( Q_strcmp( type, "game_newmap" ) == 0 )
	{
		char mapname[MAX_MAP_NAME];
		Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );

		char loadingmessage[MAX_MAP_NAME + 32];
		Q_snprintf( loadingmessage, sizeof(loadingmessage), "LOADING MAP: %s", mapname );
		m_pMapName->SetText( loadingmessage );
	}
}

void CContingencyLoadingTips::OnTick( void )
{
	BaseClass::OnTick();

	if (ipanel())
	{
		if (!IsVisible())
		{
			m_pMapName->SetText( "" );

			m_bShownTip = false;		
		}
		else
		{
			if (!m_bShownTip)
			{
				if ((m_iTipIndex += 1) > m_RandomTip.Count())
				{
					// reset index
					m_iTipIndex = 0;

					// reshuffle tips
					for (int i=0; i<m_RandomTip.Count(); i++)
					{
						swap(m_RandomTip[i],
							m_RandomTip[random->RandomInt( 0, m_RandomTip.Count() - 1 )]);
					}
				}

				// set tip
				if (m_iTipIndex < m_RandomTip.Count())
				{
					m_pTip->SetText(m_RandomTip[m_iTipIndex]);
				}

				// set web page
				char* url = "http://www.agentredproductions.com/contingency_loadingtips.htm";
				m_pHTMLPanel->OpenURL(url);

				m_bShownTip = true;
			}
		}
	}
}
