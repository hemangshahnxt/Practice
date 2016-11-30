#pragma once

#include "EmrPane.h"

class CEmrGraphingCtrl;

// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

class CEmrGraphingPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEmrGraphingPane)
public:
	CEmrGraphingPane();
	~CEmrGraphingPane();
	
	CEmrGraphingCtrl* SafeGetEmrGraphingCtrl()
	{
		if (!this) return NULL;

		return m_pGraphingCtrl.get();
	}

protected:
	scoped_ptr<CEmrGraphingCtrl> m_pGraphingCtrl;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
