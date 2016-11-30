#if !defined(AFX_MarketVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_MarketVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketView.h : header file
//

// (j.jones 2013-05-08 08:57) - PLID 56591 - removed the .h files for the child tabs
class CMarketCostDlg;
class CEffectiveness;
class CMarketZipDlg;
class CMarketRetentionGraphDlg;
class CMarketPatCoordDlg;
class CMarketReferralSourceDlg;
class CMarketProcedureDlg;
class CMarketDateDlg;
class CMarketOtherDlg;
class CMarketBaselineDlg;
class CMarketInternalDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CMarketView view



class CMarketView : public CNxTabView
{
protected:
	CMarketView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMarketView)

// Attributes
public:
	// (j.jones 2013-05-08 08:59) - PLID 56591 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CMarketCostDlg			&m_costSheet;
	CEffectiveness			&m_effectSheet;
	CMarketZipDlg			&m_zipSheet;
	CMarketRetentionGraphDlg		&m_RetentionSheet;
	CMarketPatCoordDlg		&m_PatCoordSheet;
	CMarketReferralSourceDlg	&m_RefSourSheet;
	CMarketProcedureDlg		&m_ProcedureSheet;
	CMarketDateDlg		&m_DateSheet;
	CMarketOtherDlg		&m_OtherSheet;
	CMarketBaselineDlg	&m_BaselineSheet;
	CMarketInternalDlg	&m_InternalSheet;
	void ShowTabs();
	long m_nUseRetention;
	long m_nUseMarketing;
	bool m_bFocus; // // (a.walling 2006-06-12 09:19) - PLID 20695 Whether this view is active or not.

	BOOL m_bDefaultTabLoading;

// Operations
public:
	BOOL CheckPermissions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketView)
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnSelectTab(short newTab, short oldTab);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMarketView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual int Hotkey (int key);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMarketView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	virtual void OnPrint(CDC *pDC, CPrintInfo *pInfo);
	LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
