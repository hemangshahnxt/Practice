#pragma once


// CQBMSProcessingSetupDlg dialog
// (d.thompson 2009-07-06) - PLID 34690 - Created
// (d.thompson 2010-11-05) - PLID 40628 - Heavily redone to support multiple accounts
#include "PracticeRc.h"
#include "QBMSProcessingUtils.h"


class CQBMSProcessingSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CQBMSProcessingSetupDlg)

public:
	CQBMSProcessingSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CQBMSProcessingSetupDlg();

// Dialog Data
	enum { IDD = IDD_QBMS_PROCESSING_SETUP_DLG };

	//Interface helper functions
protected:
	void LoadAllAccountsFromData();
	void ReflectAccountsInDatalist();
	void ReflectCurrentAccount();
	void EnableInterface();
	CQBMSProcessingAccount GetAccountByID(long nIDToFind);
	bool SaveCurrentStringsToMap();
	bool DoesNameExist(CString strName, long nIDToSkip);


protected:
	void EnsureModeButtonText(bool bProduction);

	// (d.thompson 2010-11-04) - PLID 40628 - Track the possible accounts in maps
	CMap<long, long, CQBMSProcessingAccount, CQBMSProcessingAccount&> m_mapLiveAccounts;
	CMap<long, long, CQBMSProcessingAccount, CQBMSProcessingAccount&> m_mapDeletedAccounts;

	//Controls
	CNxEdit m_nxeditTicket;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	NXDATALIST2Lib::_DNxDataListPtr m_pAccountList;
	NxButton m_btnActive;

	//Utility members
	long m_nNextNewID;
	//Workaround:  This dialog operates by checking OnSelChanging to save the current interface to memory, and OnSelChosen to display the 
	//	memory to the interface.  But the datalist only fires the latter (without an 'old selection') if you select the same row.  So
	//	this variable will tell us what the currently selected account ID is, and we can use that to avoid overwriting when 
	//	selecting the same row.
	long m_nPreviouslySelectedAcctID;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSwitchMode();
	afx_msg void OnBnClickedGetConnectionTicket();
public:
	afx_msg void OnBnClickedQbmsAccountAdd();
	afx_msg void OnBnClickedQbmsAccountDelete();
	afx_msg void OnBnClickedQbmsAccountActive();
	afx_msg void OnEnKillfocusQbmsAccountName();
	DECLARE_EVENTSINK_MAP()
	void SelChosenQbmsAccountList(LPDISPATCH lpRow);
	void SelChangingQbmsAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
