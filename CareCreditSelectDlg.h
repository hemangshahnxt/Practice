#pragma once


// CareCreditSelectDlg dialog

class CareCreditSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CareCreditSelectDlg)

public:
	CareCreditSelectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CareCreditSelectDlg();

// Dialog Data
	enum { IDD = IDD_CARECREDIT_SELECT };
	
	//Derived Functions
	virtual BOOL OnInitDialog();

	CString GetMerchantNumber();
	CString GetMerchantPassword();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_pCCAccountList;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	CString m_strLoginMerchantID;
	CString m_strLoginMerchantPassword;


	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingCarecreditAccount(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedCarecreditAccount(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
