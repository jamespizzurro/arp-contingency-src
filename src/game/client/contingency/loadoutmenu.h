// Added loadout system
// Added loadout menu

#ifndef LOADOUTMENU_H
#define LOADOUTMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <UtlVector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>
#include <vgui_controls/ImagePanel.h>

#include <vgui_controls/Button.h>

#include "contingency_system_loadout.h"

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: Draws the class menu
//-----------------------------------------------------------------------------
class CLoadoutMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CLoadoutMenu, vgui::Frame );

public:
	CLoadoutMenu(IViewPort *pViewPort);
	CLoadoutMenu(IViewPort *pViewPort, const char *panelName );
	virtual ~CLoadoutMenu();

	virtual const char *GetName( void ) { return PANEL_LOADOUT; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	void OnThink();

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:

	virtual vgui::Panel *CreateControlByName(const char *controlName);

	//vgui2 overrides
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	// helper functions
	void SetLabelText(const char *textEntryName, const char *text);
	void SetVisibleButton(const char *textEntryName, bool state);

	// command callbacks
	void OnCommand( const char *command );

	IViewPort	*m_pViewPort;
	ButtonCode_t m_iScoreBoardKey;
	int			m_iTeam;
	vgui::EditablePanel *m_pPanel;

	CUtlVector< vgui::Button * > m_Buttons;
	CUtlVector< vgui::Label * > m_Labels;
	CUtlVector< vgui::ImagePanel * > m_ImagePanels;

	int currentPrimaryWeaponIndex;
	const char *currentPrimaryWeaponSelected[NUM_WEAPON_TYPE_PARAMETERS];
	int currentSecondaryWeaponIndex;
	const char *currentSecondaryWeaponSelected[NUM_WEAPON_TYPE_PARAMETERS];
	int currentMeleeWeaponIndex;
	const char *currentMeleeWeaponSelected[NUM_WEAPON_TYPE_PARAMETERS];
	int currentEquipmentIndex;
	const char *currentEquipmentSelected[NUM_WEAPON_TYPE_PARAMETERS];
};

#endif // LOADOUTMENU_H
