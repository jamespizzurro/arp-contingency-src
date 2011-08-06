// Added a fully-automated nodegraph generation system

#include "cbase.h"
#include "navgen_display.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>

#include <vgui_controls/Label.h>

#include <game/client/iviewport.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CNavGenDisplay::CNavGenDisplay( IViewPort *pViewPort ) : Frame( NULL, PANEL_NAVGEN_DISPLAY )
{
	// initialize dialog
	m_pViewPort = pViewPort;

	// load the new scheme early!!
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( true );

	// hide the system buttons
	SetTitleBarVisible( false );

	m_pTitle = new Label( this, "TitleLabel", " -= Automatic Nodegraph Generator =- " );
	m_pText = new Label( this, "TextLabel", "Loading, please wait..." );

	LoadControlSettings( "Resource/UI/NavGenDisplay.res" );

	Reset();
}

CNavGenDisplay::~CNavGenDisplay()
{
}

void CNavGenDisplay::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundType( 2 );
}

void CNavGenDisplay::PerformLayout()
{
	BaseClass::PerformLayout();
}

//--------------------------------------------------------------------------------------------------------------
void CNavGenDisplay::Init( const char *title )
{
	m_pText->SetText( title );

	InvalidateLayout();
}

//--------------------------------------------------------------------------------------------------------------
void CNavGenDisplay::SetData( KeyValues *data )
{
	Init( data->GetString("msg") );
}

//--------------------------------------------------------------------------------------------------------------
void CNavGenDisplay::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	m_pViewPort->ShowBackGround( bShow );

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
}

void CNavGenDisplay::Reset( void )
{
}

void CNavGenDisplay::Update( void )
{
}
