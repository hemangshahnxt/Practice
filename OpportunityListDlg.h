#pragma once

// OpportunityListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpportunityListDlg dialog

// (j.armen 2012-06-06 12:39) - PLID 50830 - Removed GetMinMaxInfo
class COpportunityListDlg : public CNxDialog
{
// Construction
public:
	COpportunityListDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpportunityListDlg)
	enum { IDD = IDD_OPPORTUNITY_LIST_DLG };
	CNxLabel	m_labelOppListCount;
	CNxLabel	m_labelOppCountLabel;
	CNxLabel	m_labelShowOppLabel;
	CNxLabel	m_labelNewSales;
	CNxLabel	m_labelAddOns;
	CNxLabel	m_30DayLabel;
	CNxLabel	m_30DayText;
	CNxLabel	m_60DayLabel;
	CNxLabel	m_60DayText;
	CNxLabel	m_90DayLabel;
	CNxLabel	m_90DayText;
	CNxLabel	m_30DayLabelAddOn;
	CNxLabel	m_30DayTextAddOn;
	CNxLabel	m_60DayLabelAddOn;
	CNxLabel	m_60DayTextAddOn;
	CNxLabel	m_90DayLabelAddOn;
	CNxLabel	m_90DayTextAddOn;
	CNxLabel	m_labelDays;
	NxButton	m_btnLost;
	NxButton	m_btnClosed;
	NxButton	m_btnForecast;
	NxButton	m_btnCollections;
	NxButton	m_btnShowInactive;
	NxButton	m_btnDayAll;
	NxButton	m_btnDay30;
	NxButton	m_btnDay60;
	NxButton	m_btnDay90;
	NxButton	m_btnNewSale;		// (d.lange 2010-11-11 12:10) - PLID 41441 - Added New Sale checkbox
	NxButton	m_btnAddOn;			// (d.lange 2010-11-11 13:48) - PLID 41441 - Added Add On checkbox
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAddOpp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpportunityListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//Functionality
	void ReloadFromFilters();
	// (d.lange 2010-12-01 09:53) - PLID  41336 - Color the text on specific rows
	void SetRowColors();

	//Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pCoordList;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	// (z.manning, 11/05/2007) - PLID 27971 - Returns true if we are showing inactive opportunites and false if we're not.
	BOOL IsShowingInactive();

	// Generated message map functions
	//{{AFX_MSG(COpportunityListDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadCollections();
	afx_msg void OnRadForecast();
	afx_msg void OnRadClosed();
	afx_msg void OnRadLost();
	afx_msg void OnAddOpportunity();
	afx_msg void OnSelChosenCoordList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedCoordList(short nFlags);
	afx_msg void OnRequeryFinishedOpportunityList(short nFlags);
	afx_msg void OnDblClickCellOpportunityList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnLButtonDownOpportunityList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnOppShowInactive();
	afx_msg void OnRButtonDownOpportunityList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRadDayAll();
	afx_msg void OnRadDay30();
	afx_msg void OnRadDay60();
	afx_msg void OnRadDay90();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// (d.lange 2010-11-11 12:10) - PLID 41441 - Added New Sale checkbox
	afx_msg void OnBnClickedShowNewsale();
	// (d.lange 2010-11-11 12:10) - PLID 41441 - Added AddOn checkbox
	afx_msg void OnBnClickedShowAddon();
};