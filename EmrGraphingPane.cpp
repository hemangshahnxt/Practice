#include "stdafx.h"
#include "EmrGraphingPane.h"
#include "EmrGraphingCtrl.h"

// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

IMPLEMENT_DYNCREATE(CEmrGraphingPane, CEmrPane)


CEmrGraphingPane::CEmrGraphingPane()
{
}

CEmrGraphingPane::~CEmrGraphingPane()
{
}

BEGIN_MESSAGE_MAP(CEmrGraphingPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CEmrGraphingPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		m_pGraphingCtrl.reset(new CEmrGraphingCtrl);

		CRect rcClient;
		GetClientRect(&rcClient);

		if (m_pGraphingCtrl->Create(rcClient, this, -1)) {
			return 0;
		}
	} NxCatchAll(__FUNCTION__);

	return -1;
}

BOOL CEmrGraphingPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CEmrGraphingPane::OnSize(UINT nType, int cx, int cy)
{
	try {
		CEmrPane::OnSize(nType, cx, cy);

		if (m_pGraphingCtrl && m_pGraphingCtrl->GetSafeHwnd()) {
			CRect rcClient;
			GetClientRect(&rcClient);
			m_pGraphingCtrl->MoveWindow(rcClient);
		}
	} NxCatchAll(__FUNCTION__);
}
