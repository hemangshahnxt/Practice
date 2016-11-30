#if !defined(AFX_INVVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_INVVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvView.h : header file
//
// (c.haag 2007-11-15 12:44) - PLID 28094 - Added table checker support

// (j.jones 2013-05-07 12:01) - PLID 53969 - removed the .h files for the child tabs
class CInvEditDlg;
class CInvOrderDlg;
class CInvAllocationsDlg;
class CInvOverviewDlg;
class CInvReportsDlg;
class CInvFramesDlg;
class CInvVisionWebDlg;
class CInvInternalManagementDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CInvView view

class CInvEditDlg;

class CInvView : public CNxTabView
{
public:

protected:
	CInvView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CInvView)

// Attributes
public:
	// (j.jones 2013-05-07 12:02) - PLID 53969 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CInvEditDlg			&m_EditorSheet;
	CInvOrderDlg		&m_OrderSheet;
	CInvAllocationsDlg	&m_AllocationSheet;	// (j.jones 2007-11-06 11:22) - PLID 28003 - added Allocation tab
	CInvOverviewDlg		&m_OverviewSheet;	// (j.jones 2007-11-06 12:11) - PLID 27989 - addded Overview tab
	CInvReportsDlg		&m_ReportsSheet;		// (c.haag 2009-01-12 15:20) - PLID 32683 - Added Reports tab
	CInvFramesDlg		&m_FramesSheet;		// (z.manning 2010-06-17 15:49) - PLID 39222
	CInvVisionWebDlg	&m_GlassesOrderSheet;	// (s.dhole 2010-09-15 16:18) - PLID 40538
	CInvInternalManagementDlg &m_InventoryManagementSheet; //(r.wilson 2011-4-8 ?r.wilson?)

// Operations
public:
	BOOL CheckPermissions();
	void ShowTabs();	// (j.jones 2007-11-13 13:10) - PLID 28004 - required to keep our ordinals accurate

	//TES 11/17/2010 - PLID 41528 - There's a preference node for the Glasses Order tab now.
	int ShowPrefsDlg();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvView)
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewUI(CCmdUI* pCmdUI);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual			~CInvView();
	virtual int	Hotkey (int key);

	virtual void OnSelectTab(short newTab, short oldTab);
#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CInvView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//global stuff that should be local

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
