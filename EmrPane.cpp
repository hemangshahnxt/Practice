// EmrPane.cpp : implementation file
//

#include "stdafx.h"
#include "EmrPane.h"
#include "EmrFrameWnd.h"

////////////////////////////////////////////////////////////////

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

// CEmrPane

IMPLEMENT_DYNAMIC(CEmrPane, CNxDockablePane)


BEGIN_MESSAGE_MAP(CEmrPane, CNxDockablePane)
END_MESSAGE_MAP()

CEmrPane::CEmrPane()
{	
}


// CEmrPane message handlers

CEmrFrameWnd* CEmrPane::GetEmrFrameWnd() const
{
	return polymorphic_downcast<CEmrFrameWnd*>(GetTopLevelFrame());
}

BOOL CEmrPane::LoadState(LPCTSTR lpszProfileName /*= NULL*/, int nIndex /*= -1*/, UINT uiID /*= (UINT) -1*/)
{
	return CDockablePane::LoadState(lpszProfileName, nIndex, uiID);
}

BOOL CEmrPane::SaveState(LPCTSTR lpszProfileName /*= NULL*/, int nIndex /*= -1*/, UINT uiID /*= (UINT) -1*/)
{
	return CDockablePane::SaveState(lpszProfileName, nIndex, uiID);
}

// (a.walling 2012-04-17 16:56) - PLID 49750 - Most moved to shared CNxDockablePane

