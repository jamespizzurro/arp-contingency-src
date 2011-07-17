// Added loading screen tips

#ifndef CONTINGENCY_LOADINGTIPS_H
#define CONTINGENCY_LOADINGTIPS_H
#ifdef _WIN32
#pragma once
#endif

#include "gameeventlistener.h"
#include <vgui_controls/Frame.h>

using namespace vgui;

class CContingencyLoadingTips : public Frame, public CGameEventListener
{
private:
	DECLARE_CLASS_SIMPLE( CContingencyLoadingTips, Frame );

public:
	CContingencyLoadingTips(Panel *parent = NULL);
	~CContingencyLoadingTips();

	void FireGameEvent( IGameEvent * event );

protected:
	void OnTick();
	void PerformLayout();
	void ApplySchemeSettings(IScheme *pScheme);

private:
	Label* m_pMapName;
	Label* m_pTip;
	Label* m_pTipPanel;
	ImagePanel* m_pTipHeader;
	ImagePanel* m_pMapHeader;
	ImagePanel* m_pTipBackground;
	HTML* m_pHTMLPanel;
	ImagePanel* m_pPreviewImage;
	CUtlVector<char*> m_RandomTip;
	int m_iTipIndex;
	bool m_bShownTip;
};

#endif	// CONTINGENCY_LOADINGTIPS_H
