#pragma once

// (d.lange 2010-08-31 11:19) - PLID 40309 - created new dialog for Chase CC setup
// (d.thompson 2010-11-15) - PLID 40614 - Reworked to support multiple accounts.  Re-formatted the layout of this file.  Very
//		much copied from CQBMSProcessingSetupDlg implementation, except values are all stored in the datalist instead of a map.

// CChaseProcessingSetupDlg dialog
#include "PracticeRc.h"
#include "ChaseProcessingUtils.h"

class CChaseProcessingSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CChaseProcessingSetupDlg)

public:
	CChaseProcessingSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CChaseProcessingSetupDlg();

// Dialog Data
	enum { IDD = IDD_CHASE_PROCESSING_SETUP_DLG };

protected:
	//Controls
	CNxEdit m_nxeditUsername;
	CNxEdit m_nxeditMerchantID;
	CNxEdit m_nxeditTerminalID;
	CNxEdit m_nxeditDescription;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSetPassword;
	// (d.thompson 2010-11-15) - PLID 40614 - Added add/delete
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	// (d.thompson 2010-11-15) - PLID 40614 - Account list
	NXDATALIST2Lib::_DNxDataListPtr m_pAccountList;

	//Utility members
	long m_nNextNewID;
	//Workaround:  This dialog operates by checking OnSelChanging to save the current interface to memory, and OnSelChosen to display the 
	//	memory to the interface.  But the datalist only fires the latter (without an 'old selection') if you select the same row.  So
	//	this variable will tell us what the currently selected account ID is, and we can use that to avoid overwriting when 
	//	selecting the same row.
	long m_nPreviouslySelectedAcctID;
	//Maintain a list of all deleted accounts
	CDWordArray m_dwaryDeletedAccounts;

	//Utility functions
	void ReflectCurrentAccount();
	void EnableInterface();
	bool SaveCurrentlyDisplayedAccount();
	void UpdatePasswordBtnAppearance();
	void UpdateModeBtnText(BOOL bIsProduction);
	bool DoesNameExist(CString strName, long nIDToSkip);
	void OneTimeDecryptPasswords();

	//Derived functions
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	afx_msg void OnBnClickedSetPasswordBtn();
	afx_msg void OnBnClickedSwitchProdModeBtn();
	DECLARE_EVENTSINK_MAP()
	void SelChangingAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenAccountList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedAddAccount();
	afx_msg void OnBnClickedDeleteAccount();
	afx_msg void OnEnKillfocusAccountDescription();
};
