// Added credits list

#include "cbase.h"
#include "creditslist.h"

#include <vgui_controls/Frame.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CON_COMMAND( OpenCreditsList, "Opens the credits list" )
{
	creditslist->SetVisible( true );
};

using namespace vgui;

class CCreditsListInterface : public ICreditsList
{
private:
	CCreditsList *CreditsList;

public:
	CCreditsListInterface()
	{
		CreditsList = NULL;
	}

	void Create( VPANEL parent )
	{
		CreditsList = new CCreditsList( parent );
	}

	void Destroy()
	{
		if ( CreditsList )
		{
			CreditsList->SetParent( (Panel *)NULL );
			delete CreditsList;
		}
	}

	void SetVisible( bool state )
	{
		CreditsList->SetVisible( state );
	}
};
static CCreditsListInterface g_CreditsList;
ICreditsList* creditslist = (ICreditsList*)&g_CreditsList;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCreditsList::CCreditsList( VPANEL parent ) : BaseClass( NULL, "creditslist" )
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
 
	LoadControlSettings( "resource/ui/creditslist.res" );
 
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}

void CCreditsList::OnCommand( const char *command )
{
	if ( Q_stricmp(command, "vguicancel") == 0 )
		SetVisible( false );

	BaseClass::OnTick();
}
