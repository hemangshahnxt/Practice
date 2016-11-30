#pragma once

// CSupplierStatementDlg dialog

// (j.jones 2009-03-17 11:30) - PLID 32831 - created

class CSupplierStatementDlg : public CNxDialog
{

public:
	CSupplierStatementDlg(CWnd* pParent);   // standard constructor
	virtual ~CSupplierStatementDlg();

// Dialog Data
	enum { IDD = IDD_SUPPLIER_STATEMENT_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDisplayResults;
	NxButton	m_radioOrderFilter;
	NxButton	m_radioDateFilter;
	NxButton	m_radioGroupByOrderDetail;
	NxButton	m_radioGroupByProductItem;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_OrderedProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_ReturnedProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_OrderCombo;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg void OnBtnDisplayStatementResults();
	afx_msg void OnRadioFilterByOrder();
	afx_msg void OnRadioFilterByDates();	
	void OnSelChosenSupplierCombo(LPDISPATCH lpRow);
};
