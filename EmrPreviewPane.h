#pragma once

#include "EmrPreviewCtrlDlg.h"
#include "EmrPane.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

class CEmrPreviewPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEmrPreviewPane)
public:
	CEmrPreviewPane()
	{
	}
	
	CEMRPreviewCtrlDlg* SafeGetEmrPreviewCtrl()
	{
		if (!GetSafeHwnd() || !m_EMRPreviewCtrlDlg.GetSafeHwnd()) return NULL;

		return &m_EMRPreviewCtrlDlg;
	}

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CEMRPreviewCtrlDlg m_EMRPreviewCtrlDlg;

	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
};
