// Added spawnable prop system

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "propspawningmenu.h"

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
CPropSpawningMenu::CPropSpawningMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_PROPSPAWNING)
{
	m_pViewPort = pViewPort;
	m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()

	SetTitle("", true);

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	LoadControlSettings("Resource/UI/PropSpawningMenu.res");

	m_pPanel = new EditablePanel( this, PANEL_PROPSPAWNING );

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPropSpawningMenu::CPropSpawningMenu(IViewPort *pViewPort, const char *panelName) : Frame(NULL, panelName)
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
	LoadControlSettings("Resource/UI/PropSpawningMenu.res");

	m_pPanel = new EditablePanel( this, PANEL_PROPSPAWNING );

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CPropSpawningMenu::~CPropSpawningMenu()
{
}

Panel *CPropSpawningMenu::CreateControlByName(const char *controlName)
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
void CPropSpawningMenu::Reset()
{
	C_Contingency_Player *pLocalPlayer = ToContingencyPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return;

	currentSpawnablePropIndex = pLocalPlayer->GetDesiredSpawnablePropIndex();
	currentSpawnablePropSelected[0] = kSpawnablePropTypes[currentSpawnablePropIndex][0];
	currentSpawnablePropSelected[1] = kSpawnablePropTypes[currentSpawnablePropIndex][1];
	currentSpawnablePropSelected[2] = kSpawnablePropTypes[currentSpawnablePropIndex][2];
	currentSpawnablePropSelected[3] = kSpawnablePropTypes[currentSpawnablePropIndex][3];
	currentSpawnablePropSelected[4] = kSpawnablePropTypes[currentSpawnablePropIndex][4];
	currentSpawnablePropSelected[5] = kSpawnablePropTypes[currentSpawnablePropIndex][5];
	currentSpawnablePropSelected[6] = kSpawnablePropTypes[currentSpawnablePropIndex][6];
}

void CPropSpawningMenu::OnThink()
{
	C_Contingency_Player *pLocalPlayer = ToContingencyPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return;

	for ( int i = 0; i < m_Labels.Count(); i++ )
	{
		Label* pLabel = m_Labels[i];
		if ( !pLabel )
			continue;

		const char *labelName = pLabel->GetName();

		// Update prop labels
		if ( Q_strcmp(labelName, "CurrentNumSpawnableProps") == 0 )
		{
			char szCurrentNumSpawnableProps[128];
			Q_snprintf( szCurrentNumSpawnableProps, sizeof(szCurrentNumSpawnableProps), "%i", pLocalPlayer->GetNumSpawnableProps() );
			pLabel->SetText( szCurrentNumSpawnableProps );
		}
		else if ( Q_strcmp(labelName, "PropName") == 0 )
			pLabel->SetText( currentSpawnablePropSelected[0] );
		else if ( Q_strcmp(labelName, "PropCost") == 0 )
		{
			char szPropCost[128];
			Q_snprintf( szPropCost, sizeof(szPropCost), "COST: %s credits\nHP: %s", currentSpawnablePropSelected[1], currentSpawnablePropSelected[6] );
			pLabel->SetText( szPropCost );
		}
	}

	for ( int i = 0; i < m_ImagePanels.Count(); i++ )
	{
		ImagePanel* pImagePanel = m_ImagePanels[i];
		if ( !pImagePanel )
			continue;

		const char *imagePanelName = pImagePanel->GetName();

		// Update prop image
		if ( Q_strcmp(imagePanelName, "PropImage") == 0 )
			pImagePanel->SetImage( scheme()->GetImage(currentSpawnablePropSelected[2], false) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CPropSpawningMenu::OnCommand( const char *command )
{
	C_Contingency_Player *pLocalPlayer = ToContingencyPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !pLocalPlayer )
		return;

	if ( Q_stricmp(command, "nextprop") == 0 )
	{
		// Bounds check
		if ( (currentSpawnablePropIndex + 1) >= NUM_SPAWNABLEPROP_TYPES )
			return;

		currentSpawnablePropIndex = currentSpawnablePropIndex + 1;
		currentSpawnablePropSelected[0] = kSpawnablePropTypes[currentSpawnablePropIndex][0];
		currentSpawnablePropSelected[1] = kSpawnablePropTypes[currentSpawnablePropIndex][1];
		currentSpawnablePropSelected[2] = kSpawnablePropTypes[currentSpawnablePropIndex][2];
		currentSpawnablePropSelected[3] = kSpawnablePropTypes[currentSpawnablePropIndex][3];
		currentSpawnablePropSelected[4] = kSpawnablePropTypes[currentSpawnablePropIndex][4];
		currentSpawnablePropSelected[5] = kSpawnablePropTypes[currentSpawnablePropIndex][5];
		currentSpawnablePropSelected[6] = kSpawnablePropTypes[currentSpawnablePropIndex][6];
	}
	else if ( Q_stricmp(command, "previousprop") == 0 )
	{
		// Bounds check
		if ( (currentSpawnablePropIndex - 1) < 0 )
			return;

		currentSpawnablePropIndex = currentSpawnablePropIndex - 1;
		currentSpawnablePropSelected[0] = kSpawnablePropTypes[currentSpawnablePropIndex][0];
		currentSpawnablePropSelected[1] = kSpawnablePropTypes[currentSpawnablePropIndex][1];
		currentSpawnablePropSelected[2] = kSpawnablePropTypes[currentSpawnablePropIndex][2];
		currentSpawnablePropSelected[3] = kSpawnablePropTypes[currentSpawnablePropIndex][3];
		currentSpawnablePropSelected[4] = kSpawnablePropTypes[currentSpawnablePropIndex][4];
		currentSpawnablePropSelected[5] = kSpawnablePropTypes[currentSpawnablePropIndex][5];
		currentSpawnablePropSelected[6] = kSpawnablePropTypes[currentSpawnablePropIndex][6];
	}
	else if ( Q_stricmp(command, "selectprop") == 0 )
	{
		char szSpawnPropCmd[128];
		Q_snprintf( szSpawnPropCmd, sizeof(szSpawnPropCmd), "selectprop %i\n", currentSpawnablePropIndex );
		engine->ServerCmd( szSpawnPropCmd );

		Close();
		gViewPortInterface->ShowBackGround( false );
	}
	else
	{
		Close();
		gViewPortInterface->ShowBackGround( false );
	}

	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CPropSpawningMenu::ShowPanel(bool bShow)
{
	if ( bShow )
	{
		Reset();
		Activate();
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
	
	m_pViewPort->ShowBackGround( bShow );
}

void CPropSpawningMenu::SetData(KeyValues *data)
{
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CPropSpawningMenu::SetLabelText(const char *textEntryName, const char *text)
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
void CPropSpawningMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetVisible(state);
	}
}

void CPropSpawningMenu::OnKeyCodePressed(KeyCode code)
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
