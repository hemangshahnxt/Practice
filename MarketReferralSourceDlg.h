#include "marketutils.h"
#include "MarketingDlg.h"
#include "graphDescript.h"
#include "MarketGraphDlg.h"
#if !defined(AFX_MARKETREFERRALSOURCE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_MARKETREFERRALSOURCE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketEffect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketReferralSourceDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketReferralSourceDlg : public CMarketingDlg
{
// Construction
public:
	CMarketReferralSourceDlg (CWnd* pParent);   // standard constructor
	~CMarketReferralSourceDlg();
	// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
	//(e.lally 2009-09-18) PLID 35300 - Added separate function for setting up referral graph parameters
	void InitializeGraph(GraphDescript &desc, long nRowCount);
	//(e.lally 2009-09-18) PLID 35300 - Combined GraphBy functions into one, removed unused parameters
	void GraphByReferral(GraphDescript &desc, ADODB::_RecordsetPtr rsReferralDetails, ADODB::_RecordsetPtr rsMasterReferralList, long &nCurrentRow, long &nCurrentColumn, BOOL bIncludeNoReferralEntry);
	void GraphReferrals(CString strRefSourSql, CString strRefPhysSql, CString strRefPatSql, GraphDescript &desc, BOOL bIncludeNoReferralEntry = FALSE);

	// (j.jones 2012-08-07 17:28) - PLID 51058 - changed the variable name to clarify that
	// it only reflects the referral we are drilled down to on the graph, it is not the referral
	// selected in the dropdown filter
	long m_nCurrentDrilledDownReferral;

	CString m_strCategory;

	CArray<long, long>	m_arClickableRows;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void MoneyByRefSour();
	void Graph(GraphDescript &desc);
	CString GetCurrentGraphSql();
	void Print(CDC *pDC, CPrintInfo *pInfo);

	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	// (j.jones 2012-06-19 10:16) - PLID 48481 - added strFilter5
	void GetCurrentGraphFilters(CString &strFilter1, CString &strFilter2, CString &strFilter3, CString &strFilter4, CString &strFilter5,
		ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);

	// (j.gruber 2009-06-26 15:27) - PLID 34718 - added referral description
	CString GetReferralFilterDescription();
	//(e.lally 2009-09-28) PLID 35594
	CString GetConsToProcPrimaryReferralFilter();
	CString GetConsToProcMultiReferralFilter();

	// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
	
// Dialog Data
	//{{AFX_DATA(CMarketReferralSourceDlg)
	enum { IDD = IDD_REFERRAL_SOURCE };
	CNxIconButton	m_Go;
	CNxLabel	m_nxlDateLabel;
	CNxIconButton	m_Up;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	CNxColor	m_color1;
	CNxColor	m_color3;
	CNxColor	m_color;
	//CDTPicker	m_FromDate;  // (a.walling 2008-05-28 17:05) - PLID 27591 - These are never used
	//CDTPicker	m_ToDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketReferralSourceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	NXDATALISTLib::_DNxDataListPtr   m_list;

	void ShowConversionRateControls(BOOL bShow);

	// Generated message map functions
	//{{AFX_MSG(CMarketReferralSourceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMoneyByRefSour();
	afx_msg void OnUp();
	afx_msg void OnOnClickColumnGraph(short Row, short Column);
	afx_msg void OnOnMouseMoveColumnGraph(short Row, short Column);
	afx_msg void OnChangeBackButtonPosColumnGraph();
	afx_msg void OnPatsByReferral();
	afx_msg void OnInqsByReferral();
	afx_msg void OnNoShowsByReferral();
	afx_msg void OnConversionRad();
	afx_msg void OnShowInquiries();
	afx_msg void OnInqToCons() ;
	afx_msg void OnProcsClosed(); 
	afx_msg void OnProsToCons() ;
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGo();
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnConfigureApptTypes(); // (z.manning 2009-08-31 17:14) - PLID 35051
	afx_msg void OnShowAllColumns();
	afx_msg void OnShowNumbersOnly();
	afx_msg void OnShowPercentagesOnly();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int				m_patient_count;
	COleCurrency	m_cost;
	COleCurrency	m_total_billed;
	COleCurrency	m_total_collected;
	CString			m_strSql;

	NxButton			m_MoneyByRefSourRad;
	NxButton			m_MoneyByProcRad;
	NxButton			m_ProfitByProcRad;
	NxButton			m_ProfitByPatCoordRad;
	NxButton			m_MoneyByPatCoordRad;
	NxButton			m_MoneyByCategory;
	HCURSOR				m_cursor;
	HCURSOR				m_oldCursor;
	NxButton			m_PatsByReferral;
	NxButton			m_InqsByReferral;
	NxButton			m_NoShowsByReferral;
	NxButton			m_conversionRad;
	NxButton			m_InqToCons;
	NxButton			m_ProsToCons;
	NxButton			m_ProcsClosed;
	NxButton			m_ShowInquiries;
	// (j.gruber 2009-06-23 11:47) - PLID 34227
	CNxLabel			m_nxlReferralLabel;
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;

	void PatsByReferral();
	void InqsByReferral();
	void NoShowsByReferral();
	void ConversionRad();
	void InqToCons();
	void ProcsClosed(); 
	void ProsToCons();

	bool m_bIsLoading;
	bool m_bRenderedOnce;
	bool m_bGraphEmpty;
	BOOL m_bActive;
	int  m_nLastChecked; // ID of last checked dialog item

	// (j.gruber 2009-06-23 11:44) - PLID 34227 - added referral source filter
	NXDATALIST2Lib::_DNxDataListPtr m_pReferralSourceList;
	CString GetReferralNamesFromIDString(CString strIDs);
	BOOL SelectMultiReferrals();	

	// (j.jones 2012-08-07 17:11) - PLID 51058 - renamed these to clarify that they reflect
	// the selection from the dropdown filter, not the graph drilldown data
	CDWordArray m_dwFilteredRefIDList;
	long m_nCurrentFilteredReferralID;
	CString m_strFilteredReferralList;
	
	void OnRequeryFinishedReferralSources(short nFlags);
	void OnSelChosenReferralSourceList(LPDISPATCH lpRow);
	void ShowDescendantWarning();
	void OnSelChangingReferralFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);

	void InvalidateConsToProcRadioButtons(); // (z.manning 2009-09-09 15:31) - PLID 35051

	// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
	void InvalidateShowInquiriesButton();

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:
	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.
};

///{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
