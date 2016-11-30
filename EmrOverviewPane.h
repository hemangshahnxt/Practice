#pragma once

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes
#include "EmrPane.h"

class CEmrOverviewPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEmrOverviewPane)
public:
	CEmrOverviewPane() {};

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
};
