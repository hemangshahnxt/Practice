#if !defined(AFX_EELIGIBILITYTABDLG_H__1E6EFA44_ECB8_4A7E_BF0B_711CE3B94EA6__INCLUDED_)
#define AFX_EELIGIBILITYTABDLG_H__1E6EFA44_ECB8_4A7E_BF0B_711CE3B94EA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EEligibilityTabDlg.h : header file
//

// (j.jones 2007-05-01 13:51) - PLID 8993 - created E-Eligibility Tab

/////////////////////////////////////////////////////////////////////////////
// CEEligibilityTabDlg dialog

#include "FinancialRc.h"

class CEEligibilityTabDlg : public CNxDialog
{
// Construction
public:
	CEEligibilityTabDlg(CWnd* pParent);   // standard constructor
	virtual ~CEEligibilityTabDlg();
// Dialog Data
	//{{AFX_DATA(CEEligibilityTabDlg)
	enum { IDD = IDD_EELIGIBILITY_TAB_DLG };
	CNxIconButton	m_btnReviewPastRequests;
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnImportResponses;
	CNxIconButton	m_btnANSIProperties;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnbatchUnselected;
	CNxIconButton	m_btnUnbatchAll;
	CNxIconButton	m_btnUnbatchSelected;
	CNxIconButton	m_btnCreateNewRequest;
	CNxStatic	m_nxstaticEligUnselectedLabel;
	CNxStatic	m_nxstaticEligUnselectedTotal;
	CNxStatic	m_nxstaticEligSelectedLabel;
	CNxStatic	m_nxstaticEligSelectedTotal;
	// (j.jones 2009-09-16 10:28) - PLID 26481 - added ability to mass-create requests
	CNxIconButton	m_btnCreateForScheduled;
	// (j.jones 2010-03-25 17:32) - PLID 37905 - added ability to configure filtering how response info. displays
	CNxIconButton	m_btnConfigResponseFiltering;
	// (j.jones 2010-07-02 14:37) - PLID 39506 - added setup for the real-time E-Eligibility settings
	CNxIconButton	m_btnRealTimeSetup;
	NxButton		m_radioProduction;
	NxButton		m_radioTest;
	//}}AFX_DATA

	//needs to be public
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEEligibilityTabDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_FormatCombo;

	void RefreshTotals();
	void UpdateCurrentSelect(int iList);
	int m_iCurrList;  //0 - Unselected, 1 - Selected

	void UpdateEligibilityProductionStatus();

	// Generated message map functions
	//{{AFX_MSG(CEEligibilityTabDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnImportResponses();
	afx_msg void OnEligExport();
	afx_msg void OnBtnCreateNewEligibilityRequest();
	afx_msg void OnConfig();
	afx_msg void OnRemoveAllElig();
	afx_msg void OnRemoveSelectedElig();
	afx_msg void OnRemoveUnselectedElig();
	afx_msg void OnSelectAllElig();
	afx_msg void OnSelectOneElig();	
	afx_msg void OnUnselectAllElig();
	afx_msg void OnUnselectOneElig();
	afx_msg void OnRButtonDownUnselectedEligList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedUnselectedEligList(short nFlags);
	afx_msg void OnDblClickCellUnselectedEligList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonDownSelectedEligList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedSelectedEligList(short nFlags);
	afx_msg void OnDblClickCellSelectedEligList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedAnsiFormat(short nFlags);
	// (j.jones 2007-06-19 14:10) - PLID 26269 - added ability to review (and re-batch) past requests
	afx_msg void OnBtnReviewPastRequests();
	// (j.jones 2009-09-16 10:28) - PLID 26481 - added ability to mass-create requests
	afx_msg void OnBtnCreateRequestsForScheduledPatients();
	// (j.jones 2010-03-25 17:32) - PLID 37905 - added ability to configure filtering how response info. displays
	afx_msg void OnBtnConfigResponseFiltering();
	// (s.dhole 07/23/2012) PLID 48693
	//afx_msg LRESULT OnEligibilityRequestDlgClosing(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	// (j.jones 2010-07-02 14:37) - PLID 39506 - added setup for the real-time E-Eligibility settings
	afx_msg void OnBtnConfigEligibilityRealtimeSetup();
	// (j.jones 2016-05-19 10:29) - NX-100685 - added OnSelChosen
	void OnSelChosenAnsiFormat(LPDISPATCH lpRow);
	afx_msg void OnRadioEligTestBatch();
	afx_msg void OnRadioEligProductionBatch();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EELIGIBILITYTABDLG_H__1E6EFA44_ECB8_4A7E_BF0B_711CE3B94EA6__INCLUDED_)
