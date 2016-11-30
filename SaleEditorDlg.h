#if !defined(AFX_SALEEDITORDLG_H__5C1D733E_D3E6_40DF_A8FC_BCC656242589__INCLUDED_)
#define AFX_SALEEDITORDLG_H__5C1D733E_D3E6_40DF_A8FC_BCC656242589__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaleEditorDlg.h : header file
//

// (a.wetta 2007-05-07 15:11) - PLID 15998 - Created the dialog

enum DiscountTypes {
	dtMoney = 0,
	dtPercent,
};

enum ItemTypes {
	itServiceCode = 0,
	itInventoryItem,
	itGiftCertificate,
};

enum DiscountListFields {
	dlfID = 0,
	dlfType,
	dlfCategory,
	dlfCode,
	dlfName,
	dlfPercent,
	dlfOldPercent,
	dlfMoney,
	dlfOldMoney,
};

/////////////////////////////////////////////////////////////////////////////
// CSaleEditorDlg dialog

class CSaleEditorDlg : public CNxDialog
{
// Construction
public:
	CSaleEditorDlg(CWnd* pParent);   // standard constructor

	
	// (a.walling 2008-05-28 11:32) - PLID 27591 - Use CDateTimePicker
// Dialog Data
	//{{AFX_DATA(CSaleEditorDlg)
	enum { IDD = IDD_SALE_EDITOR_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnApplyQuickDiscount;
	CDateTimePicker	m_ctrlEndDate;
	CDateTimePicker	m_ctrlStartDate;
	CNxEdit	m_nxeditSaleName;
	CNxEdit	m_nxeditQuickDiscountAmount;
	CNxStatic	m_nxstaticSupplierSaleText;
	//}}AFX_DATA

	long m_nSaleID;
	CString m_strSaleName;
	COleDateTime m_dtStartDate;
	COleDateTime m_dtEndDate;
	long m_nDiscountCategory;
	CString m_strDiscountCategoryName;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaleEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pItemTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSupplierList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountCategoryList;

	BOOL Save();

	BOOL m_bStartDateCalandarOpen;
	BOOL m_bEndDateCalandarOpen;

	BOOL m_bChanged;
	BOOL m_bLoading;

	COleDateTime m_dtCurStartDate;
	COleDateTime m_dtCurEndDate;

	// (a.walling 2008-05-28 11:32) - PLID 27591 - Use Notify handlers for DateTimePicker
	// Generated message map functions
	//{{AFX_MSG(CSaleEditorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangedSaleDiscountTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChangedSaleItemTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishedSaleDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnApplyQuickDiscount();
	afx_msg void OnEditingFinishingSaleDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnKillfocusQuickDiscountAmount();
	afx_msg void OnChangeSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeSaleName();
	afx_msg void OnSelChangingSaleDiscountCatCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangedSaleDiscountCatCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChangingSaleSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnTrySetSelFinishedSaleDiscountCatCombo(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SALEEDITORDLG_H__5C1D733E_D3E6_40DF_A8FC_BCC656242589__INCLUDED_)
