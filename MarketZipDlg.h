#if !defined(AFX_MARKETZIPDLG_H__14933D81_2739_11D4_84D9_00010243175D__INCLUDED_)
#define AFX_MARKETZIPDLG_H__14933D81_2739_11D4_84D9_00010243175D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketZipDlg.h : header file
//
// (c.haag 2008-04-18 14:53) - PLID 29713 - Replaced the MSChart control with the NexTech
// pie chart window

#include "MarketingDlg.h"
#include "marketutils.h"
#include "ReferralSubDlg.h"
#include "MarketPieGraphWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CMarketZipDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketZipDlg : public CMarketingDlg
{
// Construction
public:
	CMarketZipDlg(CWnd* pParent);   // standard constructor
	~CMarketZipDlg();
	virtual void Print(CDC *pDC, CPrintInfo *pInfo);

// Dialog Data
	//{{AFX_DATA(CMarketZipDlg)
	enum { IDD = IDD_MARKET_ZIP };
	CNxIconButton	m_Go;
	NxButton m_btnCityState;
	NxButton m_btnFirstThree;
	NxButton	m_btnMktzipReferralArea;
	NxButton	m_btnPieGraphRegion;
	//}}AFX_DATA
	virtual void Refresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketZipDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMarketPieGraphWnd m_wndPieGraph;

protected:
	NxButton	m_btnAllSources;
	NxButton	m_btnSelSources;

	BOOL m_bActive;

	virtual void	SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true); // a.walling 5/22/06 PLID 20695 Override refresh() in CNxDialog::updateview

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state
 												 //     and update the status of any controls
protected:
	bool LastChecked(int nID); // a.walling PLID 20695 5/31/06 prevent resetting the filters and clearing the graph when same button is checked.

	CReferralSubDlg* m_pReferralSubDlg;

	// Generated message map functions
	//{{AFX_MSG(CMarketZipDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAllReferralsCheck();
	afx_msg void OnSelReferralsCheck();
	afx_msg void OnCityState();
	afx_msg void OnFirstThree();
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETZIPDLG_H__14933D81_2739_11D4_84D9_00010243175D__INCLUDED_)
