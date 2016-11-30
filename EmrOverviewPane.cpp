// EmrOverviewPane.cpp : implementation file
//

#include "stdafx.h"
#include "EmrOverviewPane.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

IMPLEMENT_DYNCREATE(CEmrOverviewPane, CEmrPane)

BEGIN_MESSAGE_MAP(CEmrOverviewPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrOverviewPane, CEmrPane)
END_EVENTSINK_MAP()

int CEmrOverviewPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}

BOOL CEmrOverviewPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CEmrOverviewPane::OnSize(UINT nType, int cx, int cy)
{
	CEmrPane::OnSize(nType, cx, cy);
}
