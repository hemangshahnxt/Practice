// EmrPreviewPane.cpp : implementation file
//

#include "stdafx.h"
#include "EmrTreePane.h"
#include "EmrTemplateEditorDlg.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

// (a.walling 2012-02-29 06:42) - PLID 46644 - TreePane sets m_bIsTemplate in constructor; moved some construction logic to derived classes

IMPLEMENT_DYNAMIC(CEmrTreePane, CEmrPane)

BEGIN_MESSAGE_MAP(CEmrTreePane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrTreePane, CEmrPane)
END_EVENTSINK_MAP()

int CEmrTreePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		CRect rcClient;
		GetClientRect(&rcClient);

		CWnd* pEditor = GetEmrEditorBase();

		pEditor->CreateEx(WS_EX_CONTROLPARENT, NULL, "", WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, rcClient, this, 1010);
		pEditor->MoveWindow(rcClient);

		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}

BOOL CEmrTreePane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CEmrTreePane::OnSize(UINT nType, int cx, int cy)
{
	try {
		CEmrPane::OnSize(nType, cx, cy);

		if (m_pEditorBase && m_pEditorBase->GetSafeHwnd()) {
			CRect rcClient;
			GetClientRect(&rcClient);
			m_pEditorBase->MoveWindow(rcClient);
		}
	} NxCatchAll(__FUNCTION__);
}

CEmrEditorBase* CEmrTreePane::GetEmrEditorBase()
{
	if (!m_pEditorBase) {
		if (m_bIsTemplate) {
			m_pEditorBase.reset(new CEmrTemplateEditorDlg);
		} else {
			m_pEditorBase.reset(new CEmrEditorDlg);
		}
	}
	return m_pEditorBase.get();
}
