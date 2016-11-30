#if !defined(AFX_FINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_FINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FinView.h : header file
//

// (j.jones 2013-05-06 16:55) - PLID 53969 - removed the .h files for the child tabs
class CEbillingFormDlg;
class CBatchPrintDlg;
class PracBanking;
class CBatchPayments;
class CBillingFollowUpDlg;
class CEEligibilityTabDlg;
class CReviewCCTransactionsDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CFinView view

class CFinView : public CNxTabView
{
protected:
	CFinView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFinView)

// Attributes
public:

	// (j.jones 2013-05-07 11:52) - PLID 53969 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CEbillingFormDlg	&m_EBilling;
	CBatchPrintDlg		&m_PaperBatch;
	PracBanking			&m_PBanking;
	CBatchPayments		&m_BatchPayments;
	CBillingFollowUpDlg	&m_BillingFollowUpDlg;
	// (j.jones 2007-05-01 15:13) - PLID 8993 - added E-Eligibility Tab
	CEEligibilityTabDlg	&m_EEligibilityTabDlg;
	// (d.thompson 2009-06-30) - PLID 34745 - Added a tab for credit card processing
	// (d.lange 2010-09-02 10:25) - PLID 40311 - ReviewCCTransactionDlg replaced QBMS_ReviewTransactionsDlg, added ReviewCCTransactionsDlg object
	CReviewCCTransactionsDlg &m_ReviewCCDlg;

// Operations
public:
	BOOL CheckPermissions();
	void ShowTabs();
	int ShowPrefsDlg();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnSelectTab(short newTab, short oldTab);//used for the new NxTab
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-07 14:22) - PLID 63179 - added an Ex handler
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual			~CFinView();
	virtual int	Hotkey (int key);
	// Generated message map functions
protected:
	//{{AFX_MSG(CFinView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//global stuff that should be local

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
