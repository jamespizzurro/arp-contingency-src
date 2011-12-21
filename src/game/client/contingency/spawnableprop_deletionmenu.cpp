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

#include "c_contingency_player.h"

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

	m_pTitle = new Label( this, "TitleLabel", "" );
	m_pText = new Label( this, "TextLabel", "" );
	m_pDeleteButton = new Button( this, "DeleteButton", "" );
	m_pToggleFrozenButton = new Button( this, "ToggleFrozenButton", "");
	m_pCancelButton = new Button( this, "CancelButton", "" );

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

	if ( Q_stricmp(command, "togglefrozenspawnablepropinfocus") == 0 )
		engine->ServerCmd( "togglefrozenspawnablepropinfocus\n" );

	Close();
	gViewPortInterface->ShowBackGround( false );

	BaseClass::OnCommand( command );
}
