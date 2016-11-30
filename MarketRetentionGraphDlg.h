#if !defined(AFX_MARKETRETENTIONGRAPHDLG_H__81D8669F_DF54_4CE0_ACEE_7CF55F3DD9CC__INCLUDED_)
#define AFX_MARKETRETENTIONGRAPHDLG_H__81D8669F_DF54_4CE0_ACEE_7CF55F3DD9CC__INCLUDED_

#include "MarketingDlg.h"
#include "marketutils.h"
#include "MarketPieGraphWnd.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketRetentionGraphDlg.h : header file
//
// (c.haag 2008-04-18 14:02) - PLID 29715 - Replaced the MSChart control with the NexTech
// pie chart window

/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionGraphDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketRetentionGraphDlg : public CMarketingDlg
{
// Construction
public:
	CMarketRetentionGraphDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pProcList;
	// (d.moore 2007-09-04) - PLID 14670 - Procedures can have multiple ladders.
	NXDATALIST2Lib::_DNxDataListPtr m_pLadderList;
	NXDATALISTLib::_DNxDataListPtr m_pStepList;
	NXDATALISTLib::_DNxDataListPtr m_pRangeList;
	// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
	//NXDATALISTLib::_DNxDataListPtr m_pMergeRangeList;
	NXDATALISTLib::_DNxDataListPtr m_pPurposeList;
	// (b.spivey, October 31, 2011) - PLID 38861 - Merge options 
	NXDATALIST2Lib::_DNxDataListPtr m_pMergeOptionList;

	void Refresh();
	void RefreshTab(bool bRefreshGraph = true);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	CBrush m_brush;
	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	CString GetPrintSql(BOOL bExcludeAppts, CString strPurposeIDs, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
	BOOL CheckBoxes();

	virtual void Print(CDC *pDC, CPrintInfo *pInfo);

	CString GenerateRetained(CString strProcedures, long nStepTempForCompletionID, COleDateTime dtPivot, CString strFilter);
	CString GenerateUnRetained(CString strProcedures, long nStepTempForCompletionID, COleDateTime dtPivot, CString strFilter);

	CString GetProcedureNamesFromIDs(CString strIDs);

	CString m_strMultiPurposeIds;
	CString m_strMultiProcedureIDs;

	CString GetUnretainedTitle();
	CString GetUseTracking();

	BOOL OnMultiProc();

	// (j.gruber 2007-03-21 10:04) - PLID 25260 - added function
	CString GetProcedureFilter();
		
	
	
// Dialog Data
	// (a.walling 2008-04-02 15:52) - PLID 29497 - Use NxButtons
	//{{AFX_DATA(CMarketRetentionGraphDlg)
	enum { IDD = IDD_MARKET_RETENTION_PERCENTAGE };
	NxButton	m_btnExcludeAppts;
	NxButton	m_btnUseScheduler;
	NxButton	m_btnUseTracking;
	CNxIconButton	m_Go;
	CNxIconButton	m_btnCreateMergeGroup;
	CNxLabel	m_nxlMultiProcLabel;
	CDateTimePicker	m_dtPivot;
	CNxStatic	m_nxstaticRetentionBasedOn;
	CNxStatic	m_nxstaticRetentionLabel1;
	CNxStatic	m_nxstaticRetentionLabel5;
	CNxStatic	m_nxstaticRetentionLabel2;
	CNxStatic	m_nxstaticRetentionLabel3;
	CNxStatic	m_nxstaticRetentionLabel4;
	NxButton	m_btnPieGraphRegion;
	// (j.jones 2010-01-26 10:30) - PLID 34354 - added checkbox to exclude
	// unretained patients from the merge group
	// (b.spivey, November 04, 2011) - PLID 46267 - A new filter for filtering age old unretained patients.
	CNxEdit		m_nxeditExcludeUnretainedRange; 
	NxButton	m_checkExcludeUnretained; 
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketRetentionGraphDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMarketPieGraphWnd m_wndPieGraph;

protected:

	void CMarketRetentionGraphDlg::InitalizeLists();

	bool m_bRenderedOnce;
	BOOL m_bActive;
	// (b.spivey, November 04, 2011) - PLID 46267 - We'll need this in order to update the report filters. 
	int m_nExcludeUnretainedRange; 

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:


	// (j.gruber 2008-05-28 12:39) - PLID 27440  - fixed the drop down lists so that you can't unselect them
	// Generated message map functions
	//{{AFX_MSG(CMarketRetentionGraphDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenProcedureList(long nRow);
	afx_msg void OnChangePivotDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditingFinishedRangeList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenRetentionStepList(long nRow);
	afx_msg void OnRequeryFinishedRetentionProcedureList(short nFlags);
	afx_msg void OnCreateMergeGroup();
	afx_msg void OnExcludeAppts();
	afx_msg void OnRequeryFinishedRetentionPurposeList(short nFlags);
	afx_msg void OnSelChangedRetentionPurposeList(long nNewSel);
	afx_msg void OnSelChosenRetentionPurposeList(long nRow);
	afx_msg void OnUseSchedule();
	afx_msg void OnUseTracking();
	afx_msg void OnRetentionRangeSetup();
	afx_msg void OnRequeryFinishedMergeRangeList(short nFlags);
	afx_msg void OnRequeryFinishedRangeList(short nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelChosenRetentionLadderList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedRetentionLadderList(short nFlags);
	afx_msg void OnSelChangingRetentionLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingRetentionPurposeList(long FAR* nNewSel);
	afx_msg void OnSelChangingRetentionProcedureList(long FAR* nNewSel);
	afx_msg void OnSelChangingRetentionStepList(long FAR* nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEnableExcludeUnretainedRange();
	afx_msg void OnEnKillfocusExcludeUnretainedRange(); 
	afx_msg void OnEnChangeExcludeUnretainedRange();
	void SelChangingMergeGroupOptions(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnNMKillfocusPivotDate(NMHDR *pNMHDR, LRESULT *pResult);

	// (b.spivey, November 10, 2011) - PLID 46267 - Function made to check valid date range in multiple places. 
	bool CheckValidExcludeDateRange(); 
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETRETENTIONGRAPHDLG_H__81D8669F_DF54_4CE0_ACEE_7CF55F3DD9CC__INCLUDED_)
