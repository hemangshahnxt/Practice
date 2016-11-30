#pragma once

#include "EmrEditorDlg.h"
#include "EmrTreeWnd.h"
#include "EmrPane.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

// (a.walling 2012-02-29 06:42) - PLID 46644 - TreePane sets m_bIsTemplate in constructor; moved some construction logic to derived classes

class CEmrTreePane : public CEmrPane
{
	DECLARE_DYNAMIC(CEmrTreePane)
public:
	CEmrTreePane(bool bIsTemplate)
		: m_bIsTemplate(bIsTemplate)
	{
	}

	CEmrEditorDlg* GetEmrEditor()
	{
		return polymorphic_downcast<CEmrEditorDlg*>(GetEmrEditorBase());
	}

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor
	CEmrEditorBase* GetEmrEditorBase();

	CEmrTreeWnd* GetEmrTreeWnd()
	{
		return GetEmrEditorBase()->GetEmrTreeWnd();
	}

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	scoped_ptr<CEmrEditorBase> m_pEditorBase;

	bool m_bIsTemplate;

	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
};
