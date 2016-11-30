#if !defined(AFX_SALESSETUPDLG_H__AE7F9286_111A_4699_AA1C_9CDBE2727CBA__INCLUDED_)
#define AFX_SALESSETUPDLG_H__AE7F9286_111A_4699_AA1C_9CDBE2727CBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SalesSetupDlg.h : header file
//

// (a.wetta 2007-05-07 15:11) - PLID 15998 - Created the dialog

enum SalesListFields {
	slfID = 0,
	slfName,
	slfStartDate,
	slfEndDate,
	slfDiscountCategory,
};

/////////////////////////////////////////////////////////////////////////////
// CSalesSetupDlg dialog

class CSalesSetupDlg : public CNxDialog
{
// Construction
public:
	CSalesSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSalesSetupDlg)
	enum { IDD = IDD_SALES_SETUP_DLG };
	NxButton	m_btnShowOutdatedSales;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	CNxStatic	m_nxstaticServiceItemSalesTitle;
	//}}AFX_DATA

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSalesSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pSalesList;

	void RefreshSalesList();

	BOOL VerifyDateRange(long nSaleID, COleDateTime dtStartDate, COleDateTime dtEndDate, CString &strSaleConflicts);

	// Generated message map functions
	//{{AFX_MSG(CSalesSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAddSale();
	afx_msg void OnRemoveSale();
	afx_msg void OnEditSale();
	afx_msg void OnShowOutdatedSales();
	afx_msg void OnDblClickCellSaleList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnEditingFinishingSaleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedSaleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedSaleList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingStartingSaleList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SALESSETUPDLG_H__AE7F9286_111A_4699_AA1C_9CDBE2727CBA__INCLUDED_)
