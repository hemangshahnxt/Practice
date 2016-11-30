#if !defined(AFX_INVOVERVIEWDLG_H__FB1B9095_ED9B_48DE_B87A_D4705B418ADE__INCLUDED_)
#define AFX_INVOVERVIEWDLG_H__FB1B9095_ED9B_48DE_B87A_D4705B418ADE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvOverviewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvOverviewDlg dialog

// (j.jones 2007-11-06 12:02) - PLID 27989 - created

//DRT 11/27/2007 - PLID 27990 - These numbers are hardcoded into the report, please don't change them
// (j.jones 2009-03-18 17:53) - PLID 33579 - split consignment into three filter options, and moved into the header
enum OverviewProductType {
	optAll = 0,
	optPurchased,
	optConsignmentAll,
	optConsignmentPaid,
	optConsignmentUnpaid,
};

class CInvOverviewDlg : public CNxDialog
{
// Construction
public:
	CInvOverviewDlg(CWnd* pParent);   // standard constructor

	virtual void Refresh();

	// (j.jones 2008-03-05 17:13) - PLID 29202 - added provider filters
	//TES 9/3/2008 - PLID 31237 - Added parameters for which date they're filtering on, left unchanged if bUseDateFilter is false.
	// (j.jones 2009-02-09 16:05) - PLID 32873 - added OrderID filter
	void GetCurrentFilters(long &nProductID, long &nSupplierID, long &nLocationID, long &nType, long &nProviderID,
										   bool &bUseDateFilter, short &nDateFilter, CString &strDateFilter, COleDateTime &dtFrom, COleDateTime &dtTo,
										   long &nCategoryID, long &nProductType, long &nOrderID);


// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CInvOverviewDlg)
	enum { IDD = IDD_INV_OVERVIEW_DLG };
	NxButton	m_radioAllDates;
	NxButton	m_radioDateRange;
	CDateTimePicker	m_DateFrom;
	CDateTimePicker	m_DateTo;
	CNxStatic	m_nxstaticTotalOverviewRowsLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvOverviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;	//used for label painting

	NXDATALIST2Lib::_DNxDataListPtr m_OverviewList;		//the list of result items
	NXDATALIST2Lib::_DNxDataListPtr m_TypeCombo;		//the dropdown list of all product types (general, consignment, etc.)
	NXDATALIST2Lib::_DNxDataListPtr m_StatusCombo;		//the dropdown list of all product statuses (allocated, used, etc.)
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;	//the dropdown list of all categories that are in use by any serialized product
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;		//the dropdown list of all active serializeable products
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;	//the dropdown list of all suppliers
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;	//the dropdown list of all active, managed locations
	// (j.jones 2008-03-05 16:09) - PLID 29202 - added provider filter
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;	//the dropdown list of all providers
	// (j.jones 2009-02-09 16:06) - PLID 32873 - added order filter
	NXDATALIST2Lib::_DNxDataListPtr m_OrderCombo;		//the dropdown list of all received orders

	//TES 9/3/2008 - PLID 31237 - Added options for which date to filter on.
	NXDATALIST2Lib::_DNxDataListPtr m_pDateFilterTypes;

	// (j.jones 2009-02-09 16:07) - PLID 32873 - added order tablechecker
	CTableChecker m_CategoryChecker, m_ProductChecker, m_SupplierChecker, m_LocationChecker, m_ProviderChecker, m_OrderChecker;

	//re-filters the overview list with the user's selections
	void ReFilterOverviewList();

	// (j.jones 2009-01-12 17:10) - PLID 32703 - added function to build the status combo
	void BuildStatusCombo();

	// (j.jones 2009-01-12 17:34) - PLID 32703 - added function to build the from clause
	void BuildFromClause();

	// Generated message map functions
	// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
	// (j.jones 2009-02-09 16:28) - PLID 32873 - added OnSelChosenOverviewOrderCombo
	//{{AFX_MSG(CInvOverviewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenOverviewProductCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewSupplierCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewLocationCombo(LPDISPATCH lpRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRadioAllOverviewDates();
	afx_msg void OnRadioOverviewDateRange();
	afx_msg void OnChangeDtOverviewFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDtOverviewTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequeryFinishedOverviewList(short nFlags);
	afx_msg void OnSelChosenOverviewCategoryCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewProductTypeCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewStatusCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewProviderCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingDateFilterTypes(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenDateFilterTypes(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOverviewOrderCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVOVERVIEWDLG_H__FB1B9095_ED9B_48DE_B87A_D4705B418ADE__INCLUDED_)
