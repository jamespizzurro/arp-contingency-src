// Added loadout system
// Added loadout menu

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "loadoutmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "cdll_util.h"
#include "IGameUIFuncs.h" // for key bindings

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

#include <game/client/iviewport.h>

#include <stdlib.h> // MAX_PATH define

#include "c_contingency_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLoadoutMenu::CLoadoutMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_LOADOUT)
{
	m_pViewPort = pViewPort;
	m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()

	SetTitle("", true);

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	LoadControlSettings("Resource/UI/LoadoutMenu.res");
	LoadControlSettings("Resource/UI/MainLoadoutMenu.res");

	m_pPanel = new EditablePanel( this, PANEL_LOADOUT );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLoadoutMenu::CLoadoutMenu(IViewPort *pViewPort, const char *panelName) : Frame(NULL, panelName)
{
	m_pViewPort = pViewPort;
	m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()

	SetTitle("", true);

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	// The rest of the configuration is team-specific (see beginning of OnThink method)
	LoadControlSettings("Resource/UI/LoadoutMenu.res");
	LoadControlSettings("Resource/UI/MainLoadoutMenu.res");

	m_pPanel = new EditablePanel( this, PANEL_LOADOUT );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLoadoutMenu::~CLoadoutMenu()
{
}

Panel *CLoadoutMenu::CreateControlByName(const char *controlName)
{
	if ( V_stricmp( controlName, "Label" ) == 0 )
	{
		Label *newLabel = new Label( this, controlName, "" );
		if ( newLabel )
		{
			m_Labels.AddToTail( newLabel );
			return newLabel;
		}
	}

	if ( V_stricmp( controlName, "Button" ) == 0 )
	{
		Button *newButton = new Button( this, controlName, "", this, NULL );
		if ( newButton )
		{
			m_Buttons.AddToTail( newButton );
			return newButton;
		}
	}

	if ( V_stricmp( controlName, "ImagePanel" ) == 0 )
	{
		ImagePanel *newImagePanel = new ImagePanel( this, controlName );
		if ( newImagePanel )
		{
			m_ImagePanels.AddToTail( newImagePanel );
			return newImagePanel;
		}
	}

	return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLoadoutMenu::Reset()
{
}

void CLoadoutMenu::OnThink()
{
	C_Contingency_Player *pLocalPlayer = ToContingencyPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return;

	int numButtonsHighlighted = 0;

	// Cycle through each button and see if any are highlighted
	for ( int i = 0; i < m_Buttons.Count(); i++ )
	{
		Button* pButton = m_Buttons[i];

		if ( !pButton )
			continue;

		if ( pButton->IsCursorOver() )
		{
			const char *buttonName = pButton->GetName();

			if ( strcmp( buttonName, "SoldierButton" ) == 0 )
				pLocalPlayer->SetSelectedLoadout( LOADOUT_SOLDIER );
			else if ( strcmp( buttonName, "ShotgunSoldierButton" ) == 0 )
				pLocalPlayer->SetSelectedLoadout( LOADOUT_SHOTGUN_SOLDIER );
			else if ( strcmp( buttonName, "CommanderButton" ) == 0 )
				pLocalPlayer->SetSelectedLoadout( LOADOUT_COMMANDER );
			else if ( strcmp( buttonName, "MarksmanButton" ) == 0 )
				pLocalPlayer->SetSelectedLoadout( LOADOUT_MARKSMAN );
			else if ( strcmp( buttonName, "DemolitionistButton" ) == 0 )
				pLocalPlayer->SetSelectedLoadout( LOADOUT_DEMOLITIONIST );

			numButtonsHighlighted = numButtonsHighlighted + 1;
		}
	}

	if ( numButtonsHighlighted == 0 )	// when in doubt, we're not interested in any class
		pLocalPlayer->SetSelectedLoadout( -1 );

	// Display a description for the class we're considering
	for ( int i = 0; i < m_Labels.Count(); i++ )
	{
		Label* pLabel = m_Labels[i];
		if ( !pLabel )
			continue;

		const char *labelName = pLabel->GetName();

		if ( strcmp( labelName, "DescriptionArea" ) == 0 )
		{
			switch ( pLocalPlayer->GetSelectedLoadout() )
			{
			case LOADOUT_SOLDIER:
				pLabel->SetText("#Contingency_Soldier_Description");
				break;

			case LOADOUT_SHOTGUN_SOLDIER:
				pLabel->SetText("#Contingency_Shotgun_Soldier_Description");
				break;

			case LOADOUT_COMMANDER:
				pLabel->SetText("#Contingency_Commander_Description");
				break;

			case LOADOUT_MARKSMAN:
				pLabel->SetText("#Contingency_Marksman_Description");
				break;

			case LOADOUT_DEMOLITIONIST:
				pLabel->SetText("#Contingency_Demolitionist_Description");
				break;

			default:
				pLabel->SetText("");	// when in doubt, display nothing
				break;
			}
		}
		else if ( strcmp( labelName, "LoadoutName" ) == 0 )
		{
			char classDisplay[64];
			Q_snprintf( classDisplay, sizeof(classDisplay), "%s", pLocalPlayer->GetLoadoutName(pLocalPlayer->GetSelectedLoadout()) );
			pLabel->SetText( classDisplay );
		}
	}

	// Display an image for the class we're considering
	for ( int i = 0; i < m_ImagePanels.Count(); i++ )
	{
		ImagePanel* pImagePanel = m_ImagePanels[i];
		if ( !pImagePanel )
			continue;

		const char *imagePanelName = pImagePanel->GetName();

		if ( strcmp( imagePanelName, "ImagePreview" ) == 0 )
		{
			switch ( pLocalPlayer->GetSelectedLoadout() )
			{
			case LOADOUT_SOLDIER:
				pImagePanel->SetImage( scheme()->GetImage("playermodels/soldier", false) );
				pImagePanel->SetAlpha(255);
				break;

			case LOADOUT_SHOTGUN_SOLDIER:
				pImagePanel->SetImage( scheme()->GetImage("playermodels/shotgun_soldier", false) );
				pImagePanel->SetAlpha(255);
				break;

			case LOADOUT_COMMANDER:
				pImagePanel->SetImage( scheme()->GetImage("playermodels/commander", false) );
				pImagePanel->SetAlpha(255);
				break;

			case LOADOUT_MARKSMAN:
				pImagePanel->SetImage( scheme()->GetImage("playermodels/marksman", false) );
				pImagePanel->SetAlpha(255);
				break;

			case LOADOUT_DEMOLITIONIST:
				pImagePanel->SetImage( scheme()->GetImage("playermodels/demolitionist", false) );
				pImagePanel->SetAlpha(255);
				break;

			default:
				pImagePanel->SetAlpha(0);	// when in doubt, display nothing
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CLoadoutMenu::OnCommand( const char *command )
{
	C_Contingency_Player *pLocalPlayer = ToContingencyPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return;

	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
	}

	Close();
	gViewPortInterface->ShowBackGround( false );
	pLocalPlayer->SetSelectedLoadout( -1 );

	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CLoadoutMenu::ShowPanel(bool bShow)
{
	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );

		/*// load a default class page
		for ( int i=0; i<m_mouseoverButtons.Count(); ++i )
		{
			if ( i == 0 )
			{
				m_mouseoverButtons[i]->ShowPage();	// Show the first page
			}
			else
			{
				m_mouseoverButtons[i]->HidePage();	// Hide the rest
			}
		}
		
		if ( m_iScoreBoardKey == BUTTON_CODE_INVALID ) 
		{
			m_iScoreBoardKey = gameuifuncs->GetButtonCodeForBind( "showscores" );
		}*/
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
	
	m_pViewPort->ShowBackGround( bShow );
}

void CLoadoutMenu::SetData(KeyValues *data)
{
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CLoadoutMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the visibility of a button by name
//-----------------------------------------------------------------------------
void CLoadoutMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetVisible(state);
	}
}

void CLoadoutMenu::OnKeyCodePressed(KeyCode code)
{
	if ( m_iScoreBoardKey != BUTTON_CODE_INVALID && m_iScoreBoardKey == code )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
		gViewPortInterface->PostMessageToPanel( PANEL_SCOREBOARD, new KeyValues( "PollHideCode", "code", code ) );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}
