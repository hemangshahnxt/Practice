#if !defined(AFX_MARKETBASELINEDLG_H__F78DBDE8_0E2B_4A44_8696_D11A55842048__INCLUDED_)
#define AFX_MARKETBASELINEDLG_H__F78DBDE8_0E2B_4A44_8696_D11A55842048__INCLUDED_

#include "marketingrc.h"
#include "mschart.h"
#include "NxBaselineStatic.h"
#include "marketutils.h"
#include "MarketingDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketBaselineDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
#import "nxcolumngraph.tlb" rename_namespace("ColumnGraph")
// using namespace ColumnGraph;

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineDlg dialog

enum eChartLines {
 eInquires = 0,
 eConsultations,
 eSurgeries,
 eClosureLength,
 eTrackingNoShow,
 eClosure,
 eAvgProc,
};
#define CHARTLINES 7

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CMarketBaselineDlg : public CMarketingDlg
{
// Construction
public:
	CMarketBaselineDlg(CWnd* pParent);   // standard constructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void OnMultiSelectProviders();
	void OnMultiSelectLocations();
	void OnMultiSelectPatCoords();

	// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
	void GetParameters(CString &prov, CString &loc, CString &strPatCoord, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
	void GetParameters(COleDateTime& dtFrom, COleDateTime& dtTo);

	virtual void PrePrint();
	virtual void Print(CDC *pDC, CPrintInfo *pInfo);

	// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
	CString GetReportSQL(ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);


// Dialog Data
	//{{AFX_DATA(CMarketBaselineDlg)
	enum { IDD = IDD_MARKET_BASELINE };
	CNxIconButton	m_Go;
	CNxBaselineStatic	m_Legend7;
	CNxBaselineStatic	m_Legend6;
	CNxBaselineStatic	m_Legend5;
	CNxBaselineStatic	m_Legend4;
	CNxBaselineStatic	m_Legend3;
	CNxBaselineStatic	m_Legend2;
	CNxBaselineStatic	m_Legend1;
	CNxStatic		m_LegendLabel;
	NxButton	m_btnTrackingNoShow;
	NxButton	m_btnInquires;
	NxButton	m_btnSurgeries;
	NxButton	m_btnConsultations;
	NxButton	m_btnClosureLength;
	NxButton	m_btnClosure;
	NxButton	m_btnAvgProc;
	NxButton	m_btnShowPrevYear;
	CProgressCtrl		m_progress;
	CNxIconButton	m_btnGoals;
	//CNxStatic	m_nxstaticPatientsUnlimited; // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketBaselineDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMSChart		m_chart;
	COleSafeArray m_saGraph; // Graph data
	unsigned short m_nColumns;
	unsigned short m_nRows;
	unsigned short m_nCurColumn;
	CStringArray m_astrRowLabels;
	CStringArray m_astrLegendLabels;
	CStringArray m_astrPropKeys;
	CArray<COLORREF, COLORREF> m_aclrRowLines;
	CArray<long, long> m_aPenStyles;
	CArray<long, long> m_aPropDefaults;
	CFont* m_pFontPU, *m_pLabelFont;
	float m_fGraphMax;
	long m_nIntervals;
	HBITMAP m_hBitmapPrint;
	
	ADODB::_RecordsetPtr pCachedRS[CHARTLINES];
	ADODB::_RecordsetPtr pLastYearCachedRS[CHARTLINES];
	bool m_bRenderedOnce;
	bool m_bDirtyGraph;
	BOOL m_bActive;
	bool m_bLastYear;

public:
	// (c.haag 2007-03-15 16:55) - PLID 24253 - Added support for forced refreshes
	void ResetGraph(OPTIONAL bool bClear = true, OPTIONAL CString strTitle = "", OPTIONAL bool bForceReset = false); // a.walling PLID 20695 5/25/06 set the graph to a blank state

protected:
	void ClearCachedRS();

	BOOL IntersectsOtherLabel(long nLast, const CRect& rc, CRect& rcIntersection);
	BOOL IntersectsOtherPrintLabel(CRect* pRectList, long nLast, const CRect& rc, CRect& rcIntersection);

	unsigned short GetGraphColumnCount();
	void UpdateGraphColumnCount(); // (a.walling 2006-06-08 14:18) - PLID 20695 Updates the column count since we can't rely on checkboxes anymore
	void SetGraphLabels();
	void SetBaselineLabels();
	void ScaleGraph();
	double ScaleGoal(long nGoal);
	// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
	void FilterAppointmentSQL(CString& strSQL, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
	// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
	void FilterInquiresSQL(CString& strSQL, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);

	//void Graph(class CMBGraphInfo* pInfo);
	void Graph(class CMBGraphInfo* pInfo, eChartLines eLine, bool bLastYear = false);
	void GraphInquires(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphConsultations(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphSurgeries(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphClosureRatio(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphClosureLength(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphAvgProceduresPerCase(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);
	void GraphNoShows(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle = 1, bool bLastYear = false);

	// (j.gruber 2007-09-13 16:35) - PLID 27370 - function to check that the dates will come out ok
	BOOL CheckDates(COleDateTime dtTo, COleDateTime dtFrom, long nIntervals);

	// (a.walling 2009-02-04 09:45) - PLID 31956 - Don't override this; just use the IncludeChildPosition virtual function instead
	//int GetControlPositions();
	BOOL IncludeChildPosition(HWND hwnd);
	int SetControlPositions();
	void ResizeLegend();
	//(a.wilson 2011-10-5) PLID 38789
	void HideAndRedraw();

	// Generated message map functions
	//{{AFX_MSG(CMarketBaselineDlg)
	virtual BOOL OnInitDialog();
	//afx_msg void OnStaticPatientsUnlimited(); // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg void OnActivitiesSetmarketingbaselinegoals();
	afx_msg void OnGo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCheckInquiries();
	afx_msg void OnCheckConsultations();
	afx_msg void OnCheckClosure();
	afx_msg void OnCheckSurgeries();
	afx_msg void OnCheckClosureLength();
	afx_msg void OnCheckAvgproc();
	afx_msg void OnCheckTrackingnoshow();
	afx_msg void OnCheckShowprevyear();
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETBASELINEDLG_H__F78DBDE8_0E2B_4A44_8696_D11A55842048__INCLUDED_)
