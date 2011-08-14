// Added spawnable prop system

#include "cbase.h"
#include "spawnableprop_deletionmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#include <game/client/iviewport.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CSpawnableProp_DeletionMenu::CSpawnableProp_DeletionMenu( IViewPort *pViewPort ) : Frame( NULL, PANEL_SPAWNABLEPROP_DELETIONMENU )
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

	m_pTitle = new Label( this, "TitleLabel", "Prop Removal" );
	m_pText = new Label( this, "TextLabel", "Would you like to remove this prop of yours from the world?" );
	m_pText2 = new Label( this, "TextLabel2", "WARNING: This cannot be undone and you will not be refunded any credits!" );
	m_pYes = new Button( this, "YesButton", "YES" );
	m_pNo = new Button( this, "NoButton", "NO" );

	LoadControlSettings( "Resource/UI/SpawnableProp_DeletionMenu.res" );

	Reset();
}

CSpawnableProp_DeletionMenu::~CSpawnableProp_DeletionMenu()
{
}

void CSpawnableProp_DeletionMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundType( 2 );
}

void CSpawnableProp_DeletionMenu::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CSpawnableProp_DeletionMenu::ShowPanel( bool bShow )
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

void CSpawnableProp_DeletionMenu::Reset( void )
{
}

void CSpawnableProp_DeletionMenu::Update( void )
{
}

void CSpawnableProp_DeletionMenu::Close( void )
{
	engine->ServerCmd( "forgetspawnablepropinfocus\n" );

	BaseClass::Close();
}

void CSpawnableProp_DeletionMenu::OnCommand( const char *command )
{
	if ( Q_stricmp(command, "removespawnablepropinfocus") == 0 )
		engine->ServerCmd( "removespawnablepropinfocus\n" );

	Close();
	gViewPortInterface->ShowBackGround( false );

	BaseClass::OnCommand( command );
}
