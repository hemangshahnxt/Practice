#pragma once


// CLockBoxPaymentImportDlg dialog

// (d.singleton 2014-07-11 11:13) - PLID 62862 - create new dialog that will import a lockbox payment file

namespace LockBox
{
	struct ApplyInfo
	{
		long nChargeID;
		COleCurrency cyApplyAmount;
	};

	struct PaymentInfo
	{
		long nProviderID;
		long nPersonID;
		long nLocationID;
		long nPaymentID;
		CString strDescription;
		CString strCheckNum;
		CString strAccountNum;
		CString strRoutingNum;
		COleCurrency cyPaymentAmount;
		COleDateTime dtPaymentDate;
		BOOL bManual;

		std::vector<ApplyInfo> aryApplies;

		void RemoveApplies()
		{
			aryApplies.clear();
		}
	};

	struct PatientInfo
	{
		long nPersonID = -1;
		long nUserdefinedID = -1;
		CString strPatientName = "";
		CString strLockboxPatientID = "";
	};
}

class CLockBoxPaymentImportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLockBoxPaymentImportDlg)

public:
	CLockBoxPaymentImportDlg(CWnd* pParent = NULL, long nBatchID = -1);   // standard constructor
	virtual ~CLockBoxPaymentImportDlg();

// Dialog Data
	enum { IDD = IDD_LOCKBOX_IMPORT_DLG };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	//datalists
	NXDATALIST2Lib::_DNxDataListPtr m_dlPaymentList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProviderList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlLocationList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlDescriptionList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlCategoryList;
	//buttons
	CNxIconButton m_nxbImport;
	CNxIconButton m_nxbDescriptionEdit;
	CNxIconButton m_nxbCategoryEdit;
	CNxIconButton m_nxbCancel;
	CNxIconButton m_nxbPost;
	//checkboxes
	NxButton m_nxbPaymentDate;
	NxButton m_nxbCheckNumber;
	NxButton m_nxbAccountNumber;
	NxButton m_nxbRoutingNumber;
	NxButton m_nxbMarkAsDeposited;
	//date time picker
	CDateTimePicker m_dtpPaymentDate;
	//edit boxes
	CEdit m_eBankInstitution;
	CEdit m_ePostedAmount;
	CEdit m_ePaymentDescText;
	CEdit m_eProgressText;
	CEdit m_eDepositAmount;
	//progress bar
	CProgressCtrl m_pcProgressBar;

	long m_nPaymentCatID;
	long m_nBatchID;
	CString m_strAnsi823FilePath;
	COleCurrency m_cyPostedAmount;
	COLORREF Grey = RGB(190, 190, 190);
	COLORREF White = RGB(255, 255, 255);
	COLORREF Red = RGB(255, 0, 0);
	COLORREF Black = RGB(0, 0, 0);
	
	void EnableButtons(BOOL bEnable);
	void LoadFromBatchID();
	// (d.singleton 2014-10-01 13:45) - PLID 62918
	void ParseAndLoadAnsi823File();
	// (d.singleton 2014-10-03 08:39) - PLID 62943 - way to manually add a patient
	void FindPatientInPractice(LockBox::PatientInfo& patient);
	COleCurrency FindPatientBalance(const long& nPersonID);
	// (d.singleton 2014-10-01 13:28) - PLID 62922 - Have a location drop down menu on the lockbox payment dlg,  use this as the location of all the posted lockbox payments.
	void ChoosePatient(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	// (d.singleton 2014-10-01 13:02) - PLID 63333 - need ability to split a lockbox payment into multiple payments
	void SplitPayment(NXDATALIST2Lib::IRowSettingsPtr& pRow);	
	void GetAppliesForPatientPayment(LockBox::PaymentInfo& payInfo);
	// (d.singleton 2014-08-11 14:15) - PLID 63257 - Need the ability to post manually on a line item for lockbox payment
	void PostManually(NXDATALIST2Lib::IRowSettingsPtr& pRow);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	// (d.singleton 2014-09-25 13:47) - PLID 63259 - needs to change when a user edits a payment line item.  make font red if the deposit total doesnt equal the file total	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedImportPaymentFile();
	afx_msg void OnBnClickedPostPayment();
	afx_msg void OnBnClickedPaymentCategoryEdit();
	afx_msg void OnBnClickedPaymentDescriptionEdit();
	// (d.singleton 2014-08-05 10:06) - PLID 62951
	afx_msg void OnBnClickedMarkAsDeposited();
	afx_msg void OnBnClickedCheckNumber();
	afx_msg void OnBnClickedAccountNumber();
	afx_msg void OnBnClickedRoutingNumber();
	afx_msg void OnBnClickedPaymentDate();

	DECLARE_EVENTSINK_MAP()
	void SelChosenPaymentDescription(LPDISPATCH lpRow);
	void SelChosenPaymentCategory(LPDISPATCH lpRow);
	void SelChangingPaymentDescription(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingPaymentCategory(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingLockboxProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingLockboxLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RButtonDownPaymentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickPaymentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishedPaymentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingStartingPaymentList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};
