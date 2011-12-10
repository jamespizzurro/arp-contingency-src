// Added loadout system
// Added loadout menu

#ifndef LOADOUTMENU_H
#define LOADOUTMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IVGui.h>

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>

#include "contingency_system_loadout.h"

using namespace vgui;

class ILoadoutMenu
{
public:
	virtual void Create( VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void RefreshData( void ) = 0;
	virtual void SetVisible( bool state ) = 0;
};
extern ILoadoutMenu* loadoutmenu;

class CLoadoutMenu : public Frame
{
private:
	DECLARE_CLASS_SIMPLE( CLoadoutMenu, Frame );

public:
	CLoadoutMenu( VPANEL parent );
	~CLoadoutMenu() {};

	void Reset();

protected:
	vgui::Panel *CreateControlByName(const char *controlName);
	void OnTick( void );
	void OnCommand( const char *command );

private:
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
