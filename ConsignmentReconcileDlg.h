#pragma once

// CConsignmentReconcileDlg dialog

// (j.jones 2009-03-17 11:35) - PLID 32832 - created

class CConsignmentReconcileDlg : public CNxDialog
{

public:
	CConsignmentReconcileDlg(CWnd* pParent);   // standard constructor
	virtual ~CConsignmentReconcileDlg();

// Dialog Data
	enum { IDD = IDD_CONSIGNMENT_RECONCILE_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDisplayResults;
	NxButton	m_radioUsed;
	NxButton	m_radioAvail;
	NxButton	m_radioAll_UsedAvail;
	NxButton	m_radioUnpaid;
	NxButton	m_radioPaid;
	NxButton	m_radioAll_UnpaidPaid;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_ProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;

	BOOL Save(BOOL bWarnBeforeSaving);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg void OnBtnDisplayConsignResults();	
	void OnEditingFinishedConsignProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingStartingConsignProductList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingConsignProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
