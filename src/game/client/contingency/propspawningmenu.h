// Added spawnable prop system

#ifndef PROPSPAWNINGMENU_H
#define PROPSPAWNINGMENU_H
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

#include "contingency_system_propspawning.h"

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: Draws the class menu
//-----------------------------------------------------------------------------
class CPropSpawningMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CPropSpawningMenu, vgui::Frame );

public:
	CPropSpawningMenu(IViewPort *pViewPort);
	CPropSpawningMenu(IViewPort *pViewPort, const char *panelName );
	virtual ~CPropSpawningMenu();

	virtual const char *GetName( void ) { return PANEL_PROPSPAWNING; }
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

	int currentSpawnablePropIndex;
	const char *currentSpawnablePropSelected[NUM_PROPSPAWNING_PARAMETERS];
};

#endif // PROPSPAWNINGMENU_H
