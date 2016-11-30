#if !defined(AFX_COST_H__3EE09E6E_7337_11D2_B386_00001B4B970B__INCLUDED_)
#define AFX_COST_H__3EE09E6E_7337_11D2_B386_00001B4B970B__INCLUDED_

#include "MarketingDlg.h"
#include "marketutils.h"
#include "MarketCostEntry.h"

//{{AFX_INCLUDES()
//}}AFX_INCLUDES

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketCost.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketCostDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketCostDlg : public CMarketingDlg
{
// Construction
public:
	CMarketCostDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMarketCostDlg)
	enum { IDD = IDD_MARKET_COST };
	CNxIconButton	m_addButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketCostDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMarketCostEntry mceDlg;
	void OnModify(int id);
	void OnDelete(int id);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void Refresh();
	int m_rightClicked;

	void UpdateFilters(); // a.walling PLID 20695 6/05/06 Updates Docbar filters
	BOOL m_bActive; // a.walling PLID 20695 6/05/06 whether this tab is active and not just being switched to

	NXDATALISTLib::_DNxDataListPtr   m_list;
	// Generated message map functions
	//{{AFX_MSG(CMarketCostDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRButtonUp(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickNxdatalistctrl(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellNxdatalistctrl(long nRowIndex, short nColIndex);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_COST_H__3EE09E6E_7337_11D2_B386_00001B4B970B__INCLUDED_)

