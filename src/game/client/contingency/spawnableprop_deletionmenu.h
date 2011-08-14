// Added spawnable prop system

#ifndef SPAWNABLEPROP_DELETIONMENU_H
#define SPAWNABLEPROP_DELETIONMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

using namespace vgui;

class CSpawnableProp_DeletionMenu : public Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CSpawnableProp_DeletionMenu, Frame );

public:
	CSpawnableProp_DeletionMenu( IViewPort *pViewPort );
	virtual ~CSpawnableProp_DeletionMenu();

	virtual const char *GetName( void ) { return PANEL_SPAWNABLEPROP_DELETIONMENU; }
	virtual void SetData( KeyValues *data ) { return; }
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( VPANEL parent ) { BaseClass::SetParent( parent ); }

	virtual void Close( void );
	virtual void OnCommand( const char *command );

public:
	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PerformLayout();

protected:
	IViewPort *m_pViewPort;

	Label *m_pTitle;
	Label *m_pText;
	Label *m_pText2;
	Button *m_pYes;
	Button *m_pNo;
};

#endif // SPAWNABLEPROP_DELETIONMENU_H
