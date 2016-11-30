#pragma once


// CCareCreditSetupDlg dialog

class CCareCreditSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCareCreditSetupDlg)

public:
	CCareCreditSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCareCreditSetupDlg();

	//Dialog Data
	enum { IDD = IDD_CARECREDIT_SETUP_DLG };

	//Derived Functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	//
	void LoadCurrentAccountInfo();
	bool SaveCurrentlyDisplayedAccount();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_pCCAccountList;
	CNxEdit m_nxeditAccountName;
	CNxEdit m_nxeditMerchantID;
	CNxEdit m_nxeditPassword;
	//Interface niceties 
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	// Array of deleted accounts
	CDWordArray m_dwaryDeletedAccounts;

	long m_nPreviousAccountNum;
	long m_nNextNewID;
	bool DoesNameExist(CString strAccount, long nIDToSkip);
	
	void SelChosenCarecreditAccountList(LPDISPATCH lpRow);
	void SelChangingCarecreditAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedCarecreditAddAccount();
	afx_msg void OnBnClickedCarecreditDeleteAccount();

	
	afx_msg void OnEnKillfocusCarecreditAccountname();
};

