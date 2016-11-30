#pragma once

#include <typeinfo> 
#include <NxAdvancedUILib/NxHtmlMDITemplate.h>
#include <NxAdvancedUILib/NxHtmlMDIDoc.h>
#include <NxUILib/SafeMsgProc.h>
#include <boost/scoped_ptr.hpp>
#include "EMRCustomPreviewLayoutsMDIFrame.h"
#include "EMRCustomPreviewLayoutTemplateTree.h"
#include "EMRCustomPreviewLayoutView.h"

// This is just like an MFC menu bar but without the ability to float, hide or be closed
class CCustomPreviewMFCMenuBar : public CMFCMenuBar
{
public:
	virtual BOOL CanFloat() const { return FALSE; }
	virtual BOOL CanBeClosed() const { return FALSE; }
	virtual BOOL CanAutoHide() const { return FALSE; }
};

// CEMRCustomPreviewLayoutsMDIFrame
// (c.haag 2013-01-16) - PLID 54612 - Initial implementation. This is the
// frame window that contains one or more views.

class CEMRCustomPreviewLayoutsMDIFrame : public SafeMsgProc<CMDIFrameWndEx>
{
	DECLARE_DYNAMIC(CEMRCustomPreviewLayoutsMDIFrame)

public:
	static CEMRCustomPreviewLayoutsMDIFrame* FindOpenTemplate(long nEmrTemplateID);
	static CEMRCustomPreviewLayoutsMDIFrame* LaunchWithTemplate(long nEmrTemplateID, const CString& strEmrTemplateName);

public:
	CEMRCustomPreviewLayoutsMDIFrame(long nEmrTemplateID, const CString& strEmrTemplateName);

protected:
	// The EMR template ID we serve the layouts for
	long m_nEmrTemplateID;
	// The name of the EMR template
	CString m_strEmrTemplateName;
	// The document template
	boost::scoped_ptr<CNxHtmlMDITemplate> m_pDocTemplate;
	// The EMR template tree
	CEMRCustomPreviewLayoutTemplateTree m_TreePane;

protected:
	// Create the panes
	void CreatePanes();

protected:
	// Returns TRUE if there are any modified documents in the frame
	BOOL CheckForModifiedDocuments(CArray<CNxHtmlMDIDoc*,CNxHtmlMDIDoc*>& apModifiedDocuments, CString& strWarning);

protected:
	// Creates and retunrs a new layout document. Set bSilent to FALSE to prompt the user to choose
	// a name for the layout, or TRUE to generate it with the default name for layouts.
	CNxHtmlMDIDoc* CreateNewLayout(BOOL bSilent);

public:
	// Prompts for a layout name. Empty strings are rejected.
	static int PromptForLayoutName(CWnd* pParent, CString &strResult);

// Attributes
public:
	inline int GetTemplateID() const { return m_nEmrTemplateID; }
	inline CNxHtmlMDITemplate *GetDocTemplate() const { return m_pDocTemplate.get(); }

// Operations
public:
	// (c.haag 2013-01-23) - PLID 54739 - Saving functions
	void SaveToDatabase(CEMRCustomPreviewLayoutView *pView);
	void SaveToDatabase(CNxHtmlMDIDoc *pDoc);
	// (c.haag 2013-01-23) - PLID 54739 - Deleting functions
	void DeleteFromDatabase(CEMRCustomPreviewLayoutView *pView);
	void DeleteFromDatabase(CNxHtmlMDIDoc *pDoc);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CEMRCustomPreviewLayoutsMDIFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CCustomPreviewMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//afx_msg void OnWindowManager();
	//afx_msg void OnViewCustomize();
	//afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnNewLayout();
	afx_msg void OnFileSaveAllLayouts();
	afx_msg void OnViewShowHiddenTemplateDetails();
	afx_msg void OnFileSaveAllLayoutsAndExit();
	afx_msg void OnFileExit();
	afx_msg void OnUpdateViewShowHiddenTemplateDetails(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAllLayouts(CCmdUI* pCmdUI);
	afx_msg void OnViewExpandAllTreeItems(); // (z.manning 2013-03-12 15:33) - PLID 55595
	afx_msg void OnViewCollapseAllTreeItems(); // (z.manning 2013-03-12 15:33) - PLID 55595
	DECLARE_MESSAGE_MAP()
};


