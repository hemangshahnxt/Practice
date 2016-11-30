// EmrPreviewPane.cpp : implementation file
//

#include "stdafx.h"
#include "EmrPreviewPane.h"
#include "EmrFrameWnd.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

IMPLEMENT_DYNCREATE(CEmrPreviewPane, CEmrPane)

BEGIN_MESSAGE_MAP(CEmrPreviewPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrPreviewPane, CEmrPane)
END_EVENTSINK_MAP()

int CEmrPreviewPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		CRect rcClient;
		GetClientRect(&rcClient);

		// (a.walling 2012-11-05 11:58) - PLID 53588 - CEMRPreviewCtrlDlg now expecting a control ID
		m_EMRPreviewCtrlDlg.Create(rcClient, this, -1);
		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}

BOOL CEmrPreviewPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CEmrPreviewPane::OnSize(UINT nType, int cx, int cy)
{
	try {
		CEmrPane::OnSize(nType, cx, cy);

		if (m_EMRPreviewCtrlDlg.GetSafeHwnd()) {
			CRect rcClient;
			GetClientRect(&rcClient);
			m_EMRPreviewCtrlDlg.MoveWindow(rcClient);
		}
	} NxCatchAll(__FUNCTION__);
}