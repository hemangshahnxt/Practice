#if !defined(AFX_MARKETINTERNALDLG_H__048C20F4_C25F_4F20_8807_A9FFB08516E3__INCLUDED_)
#define AFX_MARKETINTERNALDLG_H__048C20F4_C25F_4F20_8807_A9FFB08516E3__INCLUDED_

#include "MarketUtils.h"
#include "MarketingDlg.h"

#import "nxcolumngraph.tlb" rename_namespace("ColumnGraph")
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ColumnGraph;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketInternalDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketInternalDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketInternalDlg : public CMarketingDlg
{
// Construction
public:
	CMarketInternalDlg(CWnd* pParent);   // standard constructor
	~CMarketInternalDlg();

// Dialog Data
	//{{AFX_DATA(CMarketInternalDlg)
	enum { IDD = IDD_MARKET_INTERNAL };
	CNxIconButton	m_Up;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketInternalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//the graph itself!
	ColumnGraph::_DNxColumnGraphPtr m_graph;

	//radio button graph selections
	NxButton m_nxbIncidentsPerCat;
	NxButton m_nxbIncidentsPerPerson;
	NxButton m_nxbIncidentsPerClient;
	NxButton m_nxbOpenPerWeek;

	//Cursors for the graph
	HCURSOR				m_hCursor;
	HCURSOR				m_hOldCursor;

	//For ability to "drill-down" to subcolumns
	long m_nCurrentCategoryID;
	CArray<long, long>	m_arClickableRows;

	//Graphing functions
	void GraphIncidentsPerCategory();
	void GraphIncidentsPerPerson();
	void GraphIncidentsPerClient();
	void GraphOpenPerWeek();

	//
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// Generated message map functions
	//{{AFX_MSG(CMarketInternalDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMarketingIncidentsPerCat();
	afx_msg void OnIncidentsPerPerson();
	afx_msg void OnIncidentsPerClient();
	afx_msg void OnOnClickColumnInternalGraph(short Row, short Column);
	afx_msg void OnOnMouseMoveColumnInternalGraph(short Row, short Column);
	afx_msg void OnOnChangeBackButtonPosInternalGraph();
	afx_msg void OnUp();
	afx_msg void OnOpenPerWeek();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETINTERNALDLG_H__048C20F4_C25F_4F20_8807_A9FFB08516E3__INCLUDED_)
