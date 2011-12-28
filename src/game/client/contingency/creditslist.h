// Added credits list

#ifndef CREDITSLIST_H
#define CREDITSLIST_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IVGui.h>

#include <vgui_controls/Frame.h>

using namespace vgui;

class ICreditsList
{
public:
	virtual void Create( VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void SetVisible( bool state ) = 0;
};
extern ICreditsList* creditslist;

class CCreditsList : public Frame
{
private:
	DECLARE_CLASS_SIMPLE( CCreditsList, Frame );

public:
	CCreditsList( VPANEL parent );
	~CCreditsList() {};

protected:
	void OnCommand( const char *command );
};

#endif // CREDITSLIST_H
