// Added loadout system
// Added loadout menu

#include "cbase.h"
#include "loadoutmenu.h"

#include <vgui_controls/Frame.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CON_COMMAND( OpenLoadoutMenu, "Opens the loadout menu" )
{
	loadoutmenu->RefreshData();
	loadoutmenu->SetVisible( true );
};

extern ConVar contingency_client_preferredprimaryweapon;
extern ConVar contingency_client_preferredsecondaryweapon;
extern ConVar contingency_client_preferredmeleeweapon;
extern ConVar contingency_client_preferredequipment;
extern ConVar contingency_client_updateloadout;

using namespace vgui;

class CLoadoutMenuInterface : public ILoadoutMenu
{
private:
	CLoadoutMenu *LoadoutMenu;

public:
	CLoadoutMenuInterface()
	{
		LoadoutMenu = NULL;
	}

	void Create( VPANEL parent )
	{
		LoadoutMenu = new CLoadoutMenu( parent );
	}

	void Destroy()
	{
		if ( LoadoutMenu )
		{
			LoadoutMenu->SetParent( (Panel *)NULL );
			delete LoadoutMenu;
		}
	}

	void RefreshData()
	{
		LoadoutMenu->Reset();
	}

	void SetVisible( bool state )
	{
		LoadoutMenu->SetVisible( state );
	}
};
static CLoadoutMenuInterface g_LoadoutMenu;
ILoadoutMenu* loadoutmenu = (ILoadoutMenu*)&g_LoadoutMenu;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLoadoutMenu::CLoadoutMenu( VPANEL parent ) : BaseClass( NULL, "loadoutmenu" )
{
	SetParent( parent );
 
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( true );
 
	SetProportional( true );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( true );
	SetVisible( false );
 
	SetScheme( scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme") );
 
	LoadControlSettings( "resource/ui/loadoutmenu.res" );
 
	ivgui()->AddTickSignal( GetVPanel(), 100 );

	Reset();
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

void CLoadoutMenu::Reset()
{
	int i;

	currentPrimaryWeaponIndex = 0;
	for ( i = 0; i < NUM_PRIMARY_WEAPON_TYPES; i++ )
	{
		if ( Q_strcmp(kPrimaryWeaponTypes[i][0], contingency_client_preferredprimaryweapon.GetString()) == 0 )
		{
			currentPrimaryWeaponIndex = i;
			break;
		}
	}
	currentPrimaryWeaponSelected[0] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][0];
	currentPrimaryWeaponSelected[1] = kPrimaryWeaponTypes[currentPrimaryWeaponIndex][1];

	currentSecondaryWeaponIndex = 0;
	for ( i = 0; i < NUM_SECONDARY_WEAPON_TYPES; i++ )
	{
		if ( Q_strcmp(kSecondaryWeaponTypes[i][0], contingency_client_preferredsecondaryweapon.GetString()) == 0 )
		{
			currentSecondaryWeaponIndex = i;
			break;
		}
	}
	currentSecondaryWeaponSelected[0] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][0];
	currentSecondaryWeaponSelected[1] = kSecondaryWeaponTypes[currentSecondaryWeaponIndex][1];

	currentMeleeWeaponIndex = 0;
	for ( i = 0; i < NUM_MELEE_WEAPON_TYPES; i++ )
	{
		if ( Q_strcmp(kMeleeWeaponTypes[i][0], contingency_client_preferredmeleeweapon.GetString()) == 0 )
		{
			currentMeleeWeaponIndex = i;
			break;
		}
	}
	currentMeleeWeaponSelected[0] = kMeleeWeaponTypes[currentMeleeWeaponIndex][0];
	currentMeleeWeaponSelected[1] = kMeleeWeaponTypes[currentMeleeWeaponIndex][1];

	currentEquipmentIndex = 0;
	for ( i = 0; i < NUM_EQUIPMENT_TYPES; i++ )
	{
		if ( Q_strcmp(kEquipmentTypes[i][0], contingency_client_preferredequipment.GetString()) == 0 )
		{
			currentEquipmentIndex = i;
			break;
		}
	}
	currentEquipmentSelected[0] = kEquipmentTypes[currentEquipmentIndex][0];
	currentEquipmentSelected[1] = kEquipmentTypes[currentEquipmentIndex][1];
}

void CLoadoutMenu::OnTick()
{
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

	BaseClass::OnTick();
}

void CLoadoutMenu::OnCommand( const char *command )
{
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
		if ( (currentMeleeWeaponIndex + 1) >= NUM_MELEE_WEAPON_TYPES )
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

		SetVisible( false );
	}
	else
	{
		SetVisible( false );
	}

	BaseClass::OnTick();
}
