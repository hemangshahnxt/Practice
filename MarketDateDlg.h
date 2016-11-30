#include "marketutils.h"
#include "MarketingDlg.h"
#include "marketconvrategraphdlg.h"
#include "GraphDescript.h"
#if !defined(AFX_MARKETDATE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_MARKETDATE_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketEffect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketDateDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketDateDlg : public CMarketingDlg
{
// Construction
public:
	CMarketDateDlg (CWnd* pParent);   // standard constructor
	~CMarketDateDlg();
	CArray<long, long>	m_arClickableRows;	
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	NXDATALISTLib::_DNxDataListPtr  m_pApptPurposeList;
	CString m_strPurposeList;
	NXDATALISTLib::_DNxDataListPtr  m_pYearFilter;
	void RefreshConversionDateGraph();
	BOOL SelectMultiPurposes();
	//void CheckDataList(CMultiSelectDlg *dlg);
	void GraphByDate(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2 = "", CString strField2 = "", CString strDesc3 = "", CString strField3 = "" );
	void GraphDate(GraphDescript &desc);
	// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
	long CalcDateRowCount(ADODB::_RecordsetPtr rsDate);
	void Print(CDC *pDC, CPrintInfo *pInfo);
	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	void GetCurrentGraphFilters(CString &strFilter1, CString &strFilter2, CString &strFilter3, CString &strFilter4, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
	// (z.manning 2009-09-08 14:29) - PLID 35051 - Dead code
	//CString GetConversionPrintSql();
	// (j.gruber 2011-05-05 17:51) - PLID 43583 - Added Conv Group Filter
	NXDATALIST2Lib::_DNxDataListPtr m_pConvGroupList;
	void ReplaceGraphSpecificFilters(MarketGraphType mktType, CString &strSql);

	void ConversionDateRad();
	void ProcsClosed();
	void TotalRevenue();
	void Patients();
	void Cancellations();
	void InqToCons();
	void ProsToCons();
	void ApptsToCharges(); // (j.gruber 2011-05-03 16:31) - PLID 38153

	// (j.gruber 2007-03-21 10:04) - PLID 25262 - added function
	CString GetProcedureNamesFromIDString(CString str);
	CString GetPurposeFilterString();
	//(e.lally 2009-09-11) PLID 35521 - Used as the local function to be used to replace the consult procedure filter placeholder
		//in the sql graph query.
	//TODO: this should be replaced with a shared function.
	CString GetConsToProcProcedureFilter();
	// (j.gruber 2011-05-06 15:25) - PLID 43583
	CString GetConvGroupFilter();
	CString GetConvGroupFilterString();

	void InvalidateConsToProcRadioButtons(); // (z.manning 2009-09-09 15:31) - PLID 35051

	// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
	void InvalidateShowInquiriesButton();





// Dialog Data
	//{{AFX_DATA(CMarketDateDlg)
	enum { IDD = IDD_DATE };
	CNxIconButton	m_Go;
	CNxIconButton	m_Up;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	CNxColor	m_color1;
	CNxColor	m_color3;
	CNxColor	m_color;
	//CDTPicker	m_FromDate; // (a.walling 2008-05-28 17:05) - PLID 27591 - These are never used
	//CDTPicker	m_ToDate;
	// (j.gruber 2007-04-19 12:45) - PLID 25288 - added for label
	CNxLabel	m_nxlPurposeLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketDateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitializeControls();

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:

	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.

	bool m_bRenderedOnce;  // a.walling 5/18/06 PLID 20695 Prevent render on first updateview
	// a.walling 5/18/06 PLID 20695 These functions actually render the graphs. Event handlers are for GUI updates.
	BOOL m_bActive;
	int  m_nLastChecked; // ID of last checked dialog item
	bool m_bGraphEmpty;


	// Generated message map functions
	//{{AFX_MSG(CMarketDateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnConversionDateRad();
	afx_msg void OnConfigureApptTypes();
	afx_msg void OnShowAllColumns();
	afx_msg void OnShowNumbersOnly();
	afx_msg void OnShowPercentagesOnly();
	afx_msg void OnSelChosenApptPurposeList(long nRow);
	afx_msg void OnSelChosenYearFilter(long nRow);
	afx_msg void OnProcsClosed();
	afx_msg void OnProcsPerformed();
	afx_msg void OnTotalRevenue();
	afx_msg void OnPatients();
	afx_msg void OnCancellations();
	afx_msg void OnShowInquiries();	
	afx_msg void OnInqToCons();	
	afx_msg void OnProsToCons();
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedApptPurposeList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;
	NxButton			m_ConvDateRad;
	NxButton			m_ProcsPerf;
	NxButton			m_ProcsClosed;
	NxButton			m_TotalRevenue;
	NxButton			m_Cancellations;
	NxButton			m_Patients;
	NxButton			m_ShowInquiries;
	NxButton			m_InqToCons;
	NxButton			m_ProsToCons;
	HCURSOR				m_cursor;
	HCURSOR				m_oldCursor;
	// (j.gruber 2011-05-03 15:36) - PLID 38153
	NxButton			m_ApptsToCharge;
	
	// (j.gruber 2007-04-19 12:44) - PLID 25288 - making a label and cleaning up what this multiselect does
	CDWordArray m_dwPurpIDList;
	long m_nCurrentPurposeID;
	void LoadApptPurposeList(CString strList);
	void EnsurePurposeSelection();
public:
	// (j.gruber 2011-05-06 16:53) - PLID 38153
	afx_msg void OnBnClickedApptToCharge();
	// (j.gruber 2011-05-06 16:53) - PLID 43550
	afx_msg void OnBnClickedConfigureApptToCharge();
	// (j.gruber 2011-05-06 16:53) - PLID 43583
	void SelChosenConversionGroupList(LPDISPATCH lpRow);
	void RequeryFinishedConversionGroupList(short nFlags);
};

///{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
