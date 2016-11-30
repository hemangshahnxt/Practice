#include "marketutils.h"
#include "MarketingDlg.h"
#include "marketgraphdlg.h"
#include "GraphDescript.h"
#if !defined(AFX_MARKETOTHER_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_MARKETOTHER_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketEffect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketOtherDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketOtherDlg : public CMarketingDlg
{
// Construction
public:
	CMarketOtherDlg (CWnd* pParent);   // standard constructor
	~CMarketOtherDlg();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void GraphReasons(GraphDescript &desc, CString strTableName);
	void GraphByReason(MarketGraphType mktType, CString strDescription, CString strField, long nType);
	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	CString GetCurrentGraphFilters(ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
	void Print(CDC *pDC, CPrintInfo *pInfo);

	void CancelByReason();
	void NoShowByReason();

	// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace

// Dialog Data
	//{{AFX_DATA(CMarketOtherDlg)
	enum { IDD = IDD_MARKET_OTHER };
	CNxIconButton	m_Go;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	CNxColor	m_color1;
	CNxColor	m_color3;
	CNxColor	m_color;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketOtherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bRenderedOnce;// a.walling 5/18/06 PLID 20695 Prevent render on first updateview
	// a.walling 5/18/06 PLID 20695 These functions actually render the graphs. Event handlers are for GUI updates.
	BOOL m_bActive;
	bool m_bGraphEmpty;
	int  m_nLastChecked; // ID of last checked dialog item

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
													 //     and update the status of any controls
protected:
	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.


	// Generated message map functions
	//{{AFX_MSG(CMarketOtherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNoShowByReason();
	afx_msg void OnCancelByReason();
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	NxButton			m_NoShowByReason;
	NxButton			m_CancelByReason;
	
};

///{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
