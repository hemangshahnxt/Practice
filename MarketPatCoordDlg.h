#include "marketutils.h"
#include "MarketingDlg.h"
#include "MultiSelectDlg.h"
#include "MarketGraphDlg.h"
#include "GraphDescript.h"
#if !defined(AFX_CMarketPatCoordDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
#define AFX_CMarketPatCoordDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;
//	using namespace ColumnGraph;
// using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CMarketPatCoordDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketPatCoordDlg : public CMarketingDlg
{
// Construction
public: 
	CMarketPatCoordDlg(CWnd* pParent);   // standard constructor
	virtual ~CMarketPatCoordDlg();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void GraphPatientCoordinators(GraphDescript &desc);
	void GraphByPatientCoordinator(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2 = "", CString strField2 = "");

	void GraphStaff(GraphDescript &desc);
	void GraphByStaff(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2 = "", CString strField2 = "");
	void Print(CDC *pDC, CPrintInfo *pInfo);

	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	void GetCurrentGraphFilters(CString &strFilter1, CString &strFilter2, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);


	void ConsByPatCoord();
	void CancelByPatCoord();
	void ProsToCons();
	void ProcsClosed();
	void Patients();
	void ConsToSurgByPatCoord();
	void MoneyByPatCoord();
	void InqToConsByStaff();

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:

	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.

	void ShowConversionRateControls(BOOL bShow);

	void InvalidateConsToProcRadioButtons(); // (z.manning 2009-09-09 15:31) - PLID 35051

// Dialog Data
	//{{AFX_DATA(CMarketPatCoordDlg)
	enum { IDD = IDD_PATIENT_COORDINATOR };
	CNxIconButton	m_Go;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketPatCoordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Added by Bob.  Brad, what the heck are you checking this kinda crap into my source safe for?  :)
// Added by Brad, Bob, its not done yet
///////////////////////////////////////////////////////////////////////////////////////////////////

	NxButton			m_ConsByPatCoord;
	NxButton			m_CancelByPatCoord;
	NxButton			m_ContactWithProsByPatCoord;
	NxButton			m_ConsToSurgByPatCoord;
	NxButton			m_ProfitByPatCoordRad;
	NxButton			m_MoneyByPatCoordRad;
	NxButton			m_InqToConsByStaff;
	NxButton			m_Patients;
	NxButton			m_ShowInquiries;
	NxButton			m_ProsToCons;
	NxButton			m_ProcsClosed;
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;

	bool m_bRenderedOnce; // a.walling 5/18/06 PLID 20695 Prevent render on first updateview
	// a.walling 5/18/06 PLID 20695 These functions actually render the graphs. Event handlers are for GUI updates.
	BOOL m_bActive; // a.walling 5/31/06 PLID 20695 Has the tab been displayed yet? for switching among tabs/modules
	bool m_bGraphEmpty;
	int m_nLastChecked; // ID of last checked dialog item
	
	// Generated message map functions
	//{{AFX_MSG(CMarketPatCoordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnConsByPatCoord();
	afx_msg void OnCancelByPatCoord();
	afx_msg void OnContactWithProsByPatCoord();
	afx_msg void OnMoneyByPatCoord();
	afx_msg void OnConsToSurgByPatCoord();
	afx_msg void OnInqToConsByStaff();
	afx_msg void OnPatients();
	afx_msg void OnShowInquiries();
	afx_msg void OnCompletedOnly();
	afx_msg void OnProcsClosed();
	afx_msg void OnProsToCons();
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnConfigureApptTypes(); // (z.manning 2009-08-31 17:14) - PLID 35051
	afx_msg void OnShowAllColumns();
	afx_msg void OnShowNumbersOnly();
	afx_msg void OnShowPercentagesOnly();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMarketPatCoordDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
};

////{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMarketPatCoordDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
