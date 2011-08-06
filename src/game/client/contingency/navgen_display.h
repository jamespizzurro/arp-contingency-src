// Added a fully-automated nodegraph generation system

#ifndef NAVGEN_DISPLAY_H
#define NAVGEN_DISPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

class CNavGenDisplay : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CNavGenDisplay, vgui::Frame );

public:
	CNavGenDisplay( IViewPort *pViewPort );
	virtual ~CNavGenDisplay();

	virtual const char *GetName( void ) { return PANEL_NAVGEN_DISPLAY; }
	virtual void SetData( KeyValues *data );
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

public:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	void Init( const char *title );

protected:
	IViewPort *m_pViewPort;

	vgui::Label *m_pTitle;
	vgui::Label *m_pText;
};

#endif // NAVGEN_DISPLAY_H
