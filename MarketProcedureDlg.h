#include "marketutils.h"
#include "MarketingDlg.h"
#include "marketconvrategraphdlg.h"
#include "GraphDescript.h"
#if !defined(AFX_MARKETPROCEDURE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_MARKETPROCEDURE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketEffect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketProcedureDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketProcedureDlg : public CMarketingDlg
{
// Construction
public:
	CMarketProcedureDlg (CWnd* pParent);   // standard constructor
	~CMarketProcedureDlg();
	CArray<long,long>	m_arClickableRows;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void GraphByProcedure(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2 = "", CString strField2 = "", CString strDesc3 = "", CString strField3 = "");
	void GraphProcedures(GraphDescript &desc);
	int GetCurrentID();
	void MoneyByProc();
	void ProfitByProc();
	void InqToConsByProc();
	void PatsNoShowByProc();
	void ProsToConsByProc();
	void ProsToSurgByProc();
	void CancelByProc();
	void InqByProc();
	void NoShowByProc();
	void ProcsPerformed();
	void ProcsClosed();
	void Patients();
	void ConsToSurg();
	void MoneyByCategory();

	void OnMoneyByCategory();
	void GraphCategory(GraphDescript &desc);
	void GraphByCategory(MarketGraphType mktType, CString strDescription, CString strField);
	CString GetCurrentGraphSql();
	CString m_strSql;
	void Print(CDC *pDC, CPrintInfo *pInfo);

	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	CString GetCurrentGraphFilters(ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);

	// (j.gruber 2009-06-26 13:00) - PLID 34719 - added for printing report
	CString GetProcedureFilterDescription();
	//(e.lally 2009-09-11) PLID 35521 - Used as the local function to be used to replace the consult procedure filter placeholder
		//in the sql graph query.
	//TODO: this should be replaced with a shared function.
	CString GetConsToProcProcedureFilter();
	
	// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
// Dialog Data
	//{{AFX_DATA(CMarketProcedureDlg)
	enum { IDD = IDD_PROCEDURE };
	CNxIconButton	m_Go;
	CNxIconButton	m_Up;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	CNxColor	m_color1;
	CNxColor	m_color3;
	CNxColor	m_color;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketProcedureDlg)
//	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void ShowConversionRateControls(BOOL bShow);
	
	// (j.gruber 2009-06-24 11:11) - PLID 34714 - added procedure filter 
	// Generated message map functions
	//{{AFX_MSG(CMarketProcedureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMoneyByProc();
	afx_msg void OnProfitByProc();
	afx_msg void OnOnClickColumnGraph(short Row, short Column);
	afx_msg void OnOnMouseMoveColumnGraph(short Row, short Column);
	afx_msg void OnChangeBackButtonPosColumnGraph();
	afx_msg void OnInqToConsByProc();
	afx_msg void OnProsToConsByProc();
	afx_msg void OnProsToSurgByProc();
	afx_msg void OnPatsNoShowByProc();
	afx_msg void OnCancelByProc();
	afx_msg void OnInqByProc();
	afx_msg void OnProcsPerformed();
	afx_msg void OnNoShowByProc();
	afx_msg void OnUp();
	afx_msg void OnProcsClosed();
	afx_msg void OnPatients();
	afx_msg void OnShowInquiries();
	afx_msg void OnConsToSurg();
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


public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:
	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.

	int				m_patient_count;
	COleCurrency	m_cost;
	COleCurrency	m_total_billed;
	COleCurrency	m_total_collected;
	CString m_strCategory;

	bool m_bRenderedOnce;
	BOOL m_bActive;
	bool m_bGraphEmpty;
	int  m_nLastChecked; // ID of last checked dialog item

	NxButton			m_MoneyByProcRad;
	NxButton			m_ProfitByProcRad;
	NxButton			m_InqToConsByProc;
	NxButton			m_ProsToConsByProc;
	NxButton			m_ProsToSurgByProc;
	NxButton			m_PatsNoShowByProc;
	NxButton			m_CancelByProc;
	NxButton			m_ProcsClosed;
	NxButton			m_NoShowByProc;
	NxButton			m_InqByProc;
	NxButton			m_ProcsPerformed;
	NxButton			m_MoneyByCategory;
	NxButton			m_Patients;
	NxButton			m_ShowInquiries;
	NxButton			m_ConsToSurg;
	HCURSOR				m_cursor;
	HCURSOR				m_oldCursor;
	// (j.gruber 2009-06-24 11:12) - PLID 34714
	CNxLabel			m_nxlProcedureLabel;
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;

	// (j.gruber 2009-06-24 11:13) - PLID 34714 - added procedure filter
	NXDATALIST2Lib::_DNxDataListPtr m_pProcedureList;
	CString GetProcedureNamesFromIDString(CString strIDs);
	BOOL SelectMultiProcedures();	

	CDWordArray m_dwProcIDList;
	long m_nCurrentProcedureID;
	CString m_strProcedureList;
	
	void OnRequeryFinishedProcedures(short nFlags);
	void OnSelChosenProcedureList(LPDISPATCH lpRow);
	void OnSelChangingProcFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);

	void InvalidateConsToProcRadioButtons(); // (z.manning 2009-09-09 15:31) - PLID 35051
	
	// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
	void InvalidateShowInquiriesButton();
	
};

///{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
