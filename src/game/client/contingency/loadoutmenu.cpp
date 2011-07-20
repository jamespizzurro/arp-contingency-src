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

extern ConVar contingency_client_preferredprimaryweapon;
extern ConVar contingency_client_preferredsecondaryweapon;
extern ConVar contingency_client_preferredmeleeweapon;
extern ConVar contingency_client_preferredequipment;
extern ConVar contingency_client_updateloadout;

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

	currentPrimaryWeaponIndex = 0;
	currentPrimaryWeaponSelected[0] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][0];
	currentPrimaryWeaponSelected[1] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][1];

	currentSecondaryWeaponIndex = 0;
	currentSecondaryWeaponSelected[0] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][0];
	currentSecondaryWeaponSelected[1] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][1];

	currentMeleeWeaponIndex = 0;
	currentMeleeWeaponSelected[0] = kMeleeWeaponTypes[currentMeleeWeaponIndex][0];
	currentMeleeWeaponSelected[1] = kMeleeWeaponTypes[currentMeleeWeaponIndex][1];

	currentEquipmentIndex = 0;
	currentEquipmentSelected[0] = kEquipmentTypes[currentEquipmentIndex][0];
	currentEquipmentSelected[1] = kEquipmentTypes[currentEquipmentIndex][1];
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

	currentPrimaryWeaponIndex = 0;
	currentPrimaryWeaponSelected[0] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][0];
	currentPrimaryWeaponSelected[1] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][1];

	currentSecondaryWeaponIndex = 0;
	currentSecondaryWeaponSelected[0] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][0];
	currentSecondaryWeaponSelected[1] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][1];

	currentMeleeWeaponIndex = 0;
	currentMeleeWeaponSelected[0] = kMeleeWeaponTypes[currentMeleeWeaponIndex][0];
	currentMeleeWeaponSelected[1] = kMeleeWeaponTypes[currentMeleeWeaponIndex][1];

	currentEquipmentIndex = 0;
	currentEquipmentSelected[0] = kEquipmentTypes[currentEquipmentIndex][0];
	currentEquipmentSelected[1] = kEquipmentTypes[currentEquipmentIndex][1];
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

	for ( int i = 0; i < m_Labels.Count(); i++ )
	{
		Label* pLabel = m_Labels[i];
		if ( !pLabel )
			continue;

		const char *labelName = pLabel->GetName();

		// Update weapon name labels
		if ( Q_strcmp(labelName, "PrimaryWeaponName") == 0 )
			pLabel->SetText(currentPrimaryWeaponSelected[1]);
		if ( Q_strcmp(labelName, "SecondaryWeaponName") == 0 )
			pLabel->SetText(currentSecondaryWeaponSelected[1]);
		if ( Q_strcmp(labelName, "MeleeWeaponName") == 0 )
			pLabel->SetText(currentMeleeWeaponSelected[1]);
		if ( Q_strcmp(labelName, "EquipmentName") == 0 )
			pLabel->SetText(currentEquipmentSelected[1]);
	}

	for ( int i = 0; i < m_ImagePanels.Count(); i++ )
	{
		ImagePanel* pImagePanel = m_ImagePanels[i];
		if ( !pImagePanel )
			continue;

		const char *imagePanelName = pImagePanel->GetName();

		// Update weapon images
		char imagePath[256];
		if ( Q_strcmp(imagePanelName, "PrimaryWeaponImage") == 0 )
		{
			Q_snprintf( imagePath, sizeof(imagePath), "weapons/%s", currentPrimaryWeaponSelected[0] );
			pImagePanel->SetImage( scheme()->GetImage(imagePath, false) );
		}
		if ( Q_strcmp(imagePanelName, "SecondaryWeaponImage") == 0 )
		{
			Q_snprintf( imagePath, sizeof(imagePath), "weapons/%s", currentSecondaryWeaponSelected[0] );
			pImagePanel->SetImage( scheme()->GetImage(imagePath, false) );
		}
		if ( Q_strcmp(imagePanelName, "MeleeWeaponImage") == 0 )
		{
			Q_snprintf( imagePath, sizeof(imagePath), "weapons/%s", currentMeleeWeaponSelected[0] );
			pImagePanel->SetImage( scheme()->GetImage(imagePath, false) );
		}
		if ( Q_strcmp(imagePanelName, "EquipmentImage") == 0 )
		{
			Q_snprintf( imagePath, sizeof(imagePath), "weapons/%s", currentEquipmentSelected[0] );
			pImagePanel->SetImage( scheme()->GetImage(imagePath, false) );
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

	if ( Q_stricmp(command, "nextprimaryweapon") == 0 )
	{
		// Bounds check
		if ( (currentPrimaryWeaponIndex + 1) >= NUM_PRIMARY_WEAPON_TYPES )
			return;

		currentPrimaryWeaponIndex = currentPrimaryWeaponIndex + 1;
		currentPrimaryWeaponSelected[0] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][0];
		currentPrimaryWeaponSelected[1] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "previousprimaryweapon") == 0 )
	{
		// Bounds check
		if ( (currentPrimaryWeaponIndex - 1) < 0 )
			return;

		currentPrimaryWeaponIndex = currentPrimaryWeaponIndex - 1;
		currentPrimaryWeaponSelected[0] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][0];
		currentPrimaryWeaponSelected[1] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "nextsecondaryweapon") == 0 )
	{
		// Bounds check
		if ( (currentSecondaryWeaponIndex + 1) >= NUM_SECONDARY_WEAPON_TYPES )
			return;

		currentSecondaryWeaponIndex = currentSecondaryWeaponIndex + 1;
		currentSecondaryWeaponSelected[0] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][0];
		currentSecondaryWeaponSelected[1] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "previoussecondaryweapon") == 0 )
	{
		// Bounds check
		if ( (currentSecondaryWeaponIndex - 1) < 0 )
			return;

		currentSecondaryWeaponIndex = currentSecondaryWeaponIndex - 1;
		currentSecondaryWeaponSelected[0] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][0];
		currentSecondaryWeaponSelected[1] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "nextmeleeweapon") == 0 )
	{
		// Bounds check
		if ( (currentMeleeWeaponIndex + 1) >= NUM_SECONDARY_WEAPON_TYPES )
			return;

		currentMeleeWeaponIndex = currentMeleeWeaponIndex + 1;
		currentMeleeWeaponSelected[0] = kMeleeWeaponTypes[currentMeleeWeaponIndex][0];
		currentMeleeWeaponSelected[1] = kMeleeWeaponTypes[currentMeleeWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "previousmeleeweapon") == 0 )
	{
		// Bounds check
		if ( (currentMeleeWeaponIndex - 1) < 0 )
			return;

		currentMeleeWeaponIndex = currentMeleeWeaponIndex - 1;
		currentMeleeWeaponSelected[0] = kMeleeWeaponTypes[currentMeleeWeaponIndex][0];
		currentMeleeWeaponSelected[1] = kMeleeWeaponTypes[currentMeleeWeaponIndex][1];
	}
	else if ( Q_stricmp(command, "nextequipment") == 0 )
	{
		// Bounds check
		if ( (currentEquipmentIndex + 1) >= NUM_EQUIPMENT_TYPES )
			return;

		currentEquipmentIndex = currentEquipmentIndex + 1;
		currentEquipmentSelected[0] = kEquipmentTypes[currentEquipmentIndex][0];
		currentEquipmentSelected[1] = kEquipmentTypes[currentEquipmentIndex][1];
	}
	else if ( Q_stricmp(command, "previousequipment") == 0 )
	{
		// Bounds check
		if ( (currentEquipmentIndex - 1) < 0 )
			return;

		currentEquipmentIndex = currentEquipmentIndex - 1;
		currentEquipmentSelected[0] = kEquipmentTypes[currentEquipmentIndex][0];
		currentEquipmentSelected[1] = kEquipmentTypes[currentEquipmentIndex][1];
	}
	else if ( Q_stricmp(command, "saveloadout") == 0 )
	{
		const char *currentPrimaryWeaponSelectedClassname = currentPrimaryWeaponSelected[0];
		const char *currentSecondaryWeaponSelectedClassname = currentSecondaryWeaponSelected[0];
		const char *currentMeleeWeaponSelectedClassname = currentMeleeWeaponSelected[0];
		const char *currentEquipmentSelectedClassname = currentEquipmentSelected[0];

		// Update our preferred weapon/equipment ConVars according to our selection
		contingency_client_preferredprimaryweapon.SetValue( currentPrimaryWeaponSelectedClassname );
		contingency_client_preferredsecondaryweapon.SetValue( currentSecondaryWeaponSelectedClassname );
		contingency_client_preferredmeleeweapon.SetValue( currentMeleeWeaponSelectedClassname );
		contingency_client_preferredequipment.SetValue( currentEquipmentSelectedClassname );
		contingency_client_updateloadout.SetValue( 1 );	// tells the server (gamerules) that our loadout needs to be updated

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
void CLoadoutMenu::ShowPanel(bool bShow)
{
	if ( bShow )
	{
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
