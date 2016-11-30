#pragma once

#include "EmrFrameWnd.h"

// (a.walling 2011-12-11 13:25) - PLID 46644 - Template editor
// (a.walling 2012-02-29 06:42) - PLID 46644 - Moved some CEmrTemplateEditorDlg logic in here

// CEmrTemplateFrameWnd frame

class CEmrTemplateFrameWnd : public CEmrFrameWnd
{
	friend class CEmrTemplateEditorDlg;
	friend std::auto_ptr<CEmrTemplateFrameWnd>;

	DECLARE_DYNAMIC(CEmrTemplateFrameWnd)

protected:
	// (a.walling 2012-03-02 11:52) - PLID 48469 - Constructor takes template and collection ids
	CEmrTemplateFrameWnd(long nEmrTemplateID, long nCollectionID = -1);
	virtual ~CEmrTemplateFrameWnd();

public:
	// (a.walling 2012-03-02 11:52) - PLID 48469 - Made these static members which return a pointer to the new instance
	//Edit an existing template (or activate an existing window)
	static CEmrTemplateFrameWnd* LaunchWithTemplate(long nEmrTemplateID);
	//Edit a new template
	static CEmrTemplateFrameWnd* CreateNewTemplate(long nCollectionID);
	// (a.walling 2012-04-10 15:59) - PLID 48469 - Find any open windows for the same template ID
	static CEmrTemplateFrameWnd* FindOpenTemplate(long nEmrTemplateID);

	long GetTemplateID();

	static void HandleDatabaseReconnect();	// (j.armen 2012-06-11 17:13) - PLID 48808

protected:
	long m_nEmrTemplateID;
	long m_nNewTemplateCollectionID;
	
	// (a.walling 2012-02-28 14:53) - PLID 48451 - implement CEmrFrameWnd abstract members
	virtual void InitializeRibbon();
	virtual void InitializeStatusBar();
	virtual void InitializePanes();
	
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	void InitializeRibbon_MainPanel();
	void InitializeRibbon_View();
	void InitializeRibbon_Edit();
	void InitializeRibbon_Template();
	void InitializeRibbon_MoreInfo();
	void InitializeRibbon_QAT();

	// (a.walling 2012-03-02 16:30) - PLID 48598 - return our layout section name
	virtual CString GetLayoutSectionName();

	virtual CString GenerateTitleBarText();

	virtual void OnDatabaseReconnect();	// (j.armen 2012-06-11 17:13) - PLID 48808

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// (a.walling 2012-03-05 12:33) - PLID 46644 - Handle closing and saving / prompting re the template
	afx_msg void OnClose();

	////
	/// UI state
	
	afx_msg void OnUpdateEmChecklist(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePositionTopics(CCmdUI* pCmdUI);

	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
	afx_msg void OnUpdateShowTemplateAuditHistory(CCmdUI* pCmdUI);

	///
	/// UI Commands

	afx_msg void OnEmChecklist();
	afx_msg void OnPositionTopics();

	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
	afx_msg void OnShowTemplateAuditHistory();

	
	// (a.walling 2012-04-06 13:19) - PLID 48990 - Ensure the most common lists are cached
	virtual void EnsureCachedData();

	DECLARE_MESSAGE_MAP()
};


