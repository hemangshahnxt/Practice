#pragma once

class CBillingDlg;
class CInsuranceBilling;
class CBilling2Dlg;
class CPatientView;
class CFinancialDlg;

enum BillingAccessType;
enum class EBillStatusType;	// (r.gonet 07/02/2014) - PLID 62567 - Forward declaration
enum BillEntryType { Bill = 1, Quote = 2 };	// (j.armen 2014-08-06 10:06) - PLID 63161 - Use an enum for this now
enum class BillFromType {Other = 0, EMR = 1, HL7Batch = 2, VisitsToBeBilled = 3};  // (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum

class CBillingModuleDlg : public CNxDialog
	{

// Construction
public:
	typedef enum { IR_CANCEL_BILL = 0, IR_SUCCESS = 1, IR_FAIL = 2 } EApplyReferralStatus;

	// (j.gruber 2009-07-10 17:19) - PLID 34724 - take out batched and add manually batched
	//BOOL m_bBatched;
	BOOL m_bManuallyUnbatched;
	BOOL m_bInsurance;
	BOOL m_bPromptForReferral;
	long m_nPatientID;

	NXDATALISTLib::_DNxDataListPtr m_CoordCombo;
	CTableChecker m_coordChecker;	
	// (j.jones 2008-06-13 13:52) - PLID 28782 - renamed the parameter here
	void AddToModifiedList(long nChargeID);
	void SetBillID(long BillID);
	long GetBillID();
	CString m_strExtraDesc;
	BOOL m_boShowAvailablePayments;
	BOOL m_boAskForNewPayment;

	// (j.jones 2011-08-24 08:41) - PLID 44868 - added m_bHasBeenCorrected,
	// which is only TRUE if the bill ID exists in BillCorrectionsT.OriginalBillID
	BOOL m_bHasBeenCorrected;
	
	// (j.jones 2011-08-24 08:41) - PLID 44868 - returns true if any charge
	// is an "original" or "void" charge, and therefore read only
	BOOL HasOriginalOrVoidCharge();
	
	//TES 7/1/2008 - PLID 26143 - Tell the bill to automatically apply the given IDs when saving.
	void ApplyPaymentIDs(const CDWordArray &dwaPaymentIDs);
	//TES 7/3/2008 - PLID 26143 - Force the "Bill A" combo to be set to the given row on the next bill that's opened.
	void SetDefaultBillA(int nBillA);
	// (d.thompson 2009-09-02) - PLID 34694
	// (j.jones 2011-01-21 09:14) - PLID 42156 - I renamed this function for clarity, and added a string for the
	// billing dlg Edit button to use
	BOOL IsBillWriteable_HistoricFinancial(BOOL bSilent, CString *pstrBillEditWarning = NULL);

	BOOL m_boIsNewBill;
	CBillingDlg &m_dlgBilling;
	CBilling2Dlg &m_dlgBilling2;
	CInsuranceBilling &m_dlgInsuranceBilling;	
	COleDateTime m_dtCurrentDate;

	// (j.jones 2011-01-25 15:29) - PLID 42156 - track the patient coordinator ID
	long m_nCurCoordinatorID;

	BOOL m_bUseDefaultDate;
	COleDateTime m_dtDefaultDate;

	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized, so this is now dead code
	//CString GetChargeSql();
	ADODB::_RecordsetPtr GetChargeRecordset();
	
	int m_iSaveType;

	long m_nQuoteEMR;

	// (j.jones 2011-06-30 08:58) - PLID 43770 - added m_bCreatedFromEMR
	// which tells the bill to load differently in anticipation of receiving
	// a NXM_BILL_EMR message after creation
	// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
	BillFromType m_eCreatedFrom;

	// (r.gonet 07/02/2014) - PLID 62567 - Track the current bill status type
	EBillStatusType m_eBillStatusType;
	// (r.gonet 07/01/2014) - PLID 62567 - The prefix we prepend to the bill description when the bill is on hold.
	CString m_strOnHoldPrefix = "ON HOLD: ";

	// (j.jones 2011-06-30 08:57) - PLID 43770 - added bFromEMR, which, if true,
	// means we should be later posting the NXM_BILL_EMR message, and the bill
	// needs to know we're about to do that
	// (d.singleton 2014-02-27 17:16) - PLID 61072 - new preference to open menu when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
	// (j.armen 2014-08-06 10:06) - PLID 63161 - Use an enum
	// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
	void OpenWithBillID(long lBillID, BillEntryType eEntryType, int iSaveType, BillFromType eFromType = BillFromType::Other, long nBillToInsPartyID = -1);
	long m_iBillID; // -1 = New
	BillEntryType m_EntryType;
	// (j.jones 2011-04-27 15:32) - PLID 43405 - this is now a variant, should be VT_I4
	_variant_t m_varSuperbillID;	//this will be inserted into the data
	CBillingModuleDlg(CWnd* pParent, UINT nIDTemplate = IDD_BILLING_MODULE_DLG);   // standard constructor
	~CBillingModuleDlg();

	// (j.jones 2010-05-20 09:15) - PLID 32338 - added an optional default insured party ID,
	// because when opening a bill, if this function is called, the same recordset would otherwise
	// be run twice
	BOOL ApplyInsuranceReferral(bool bForcePrompt = false, bool bIsNewBill = true, long nDefaultInsuredPartyID = -1);
	long m_nInsuranceReferralID;
	CString m_strSelectedAuthNum;

	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate);
	NXDATALISTLib::_DNxDataListPtr m_pQuoteCustomList;

	BOOL CheckHasInsuranceCharges();

	long GetCurrentBillToInsuredPartyID(); // (z.manning 2010-08-16 10:26) - PLID 40120

	// (j.gruber 2007-11-20 11:41) - PLID 28061 - changing how new quotes work
	void SetQuoteReportDropDown(BOOL bIsPackage);

	// (a.walling 2008-05-05 12:53) - PLID 29897 - Patient name
	CString GetBillPatientName(long nPatientID = -1);

	// (d.singleton 2014-02-27 17:16) - PLID 61072 - new preference to open menu when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
	long m_nBillToInsPartyID;

	// (r.farnworth 2014-12-10 14:36) - PLID 64081 - If you cancel a bill from a HL7 charge message (DFT) that was commited, Nextech considers the message as imported.
	long m_nMessageQueueID;

// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CBillingModuleDlg)
	enum { IDD = IDD_BILLING_MODULE_DLG };
	CNxIconButton	m_btnSaveCopy;
	CNxIconButton	m_editTextButton;
	CNxIconButton	m_cancelButton;
	CNxIconButton	m_printPreviewButton;
	CNxIconButton	m_editButton;
	CNxIconButton	m_deleteButton;
	CNxIconButton	m_saveButton;
	CNxIconButton	m_mergeButton;
	CNxEdit	m_editDescription;
	CNxEdit	m_editQuoteNotes;
	CDateTimePicker	m_date;
	CNxEdit	m_nxeditEditId;
	CNxEdit	m_nxeditEditPatientId;
	CNxEdit	m_nxeditEditPatientName;
	CNxStatic	m_nxstaticLabelBillDate;
	CNxStatic	m_nxstaticLabelBillId;
	CNxStatic	m_nxstaticLabelQuoteNote1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBillingModuleDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
//protected:
public:
	void SetActiveTab(short newTab);
	void CloseWindow();
	BOOL Save();

	CPatientView* m_pPatientView;
	//CFinancialDlg* pFinancialDlg;
	CWnd* m_pFinancialDlg;
	CString m_strBarcode;

	//DRT 5/11/2004 - PLID 12071 - Allow the quote to send a message back that it is quitting via preview.  
	//	See comments in billingmoduledlg.h for usage.  This ONLY works for quotes at this point in time.
	CWnd* m_pPostCloseMsgWnd;

	// (a.walling 2007-05-24 08:46) - PLID 26114
	// (j.jones 2013-07-10 15:48) - PLID 57148 - moved these definitions to the cpp where they belonged
	COleCurrency GetTotalRewardPoints();
	COleCurrency GetAdjustedRewardPoints();
	void AddAdjustedRewardPoints(COleCurrency &cy);

	//TES 7/1/2008 - PLID 26143 - Added the ability to add a product to the bill from outside.
	void AddNewProductToBillByServiceID(long nServiceID, double dblQtyDefault = 1.0);

	// (c.haag 2009-08-24 13:12) - PLID 14844 - Sets the bill date. Should only be called when the
	// dialog is initializing as it changes both the date control and m_dtCurrentDate.
	void SetDefaultDate(const COleDateTime& dt);

	// (j.jones 2013-07-11 08:53) - PLID 57148 - Retrieves the first charge provider sorted by LineID.
	// If Charge 1 has no provider, and Charge 2 has a provider, we'll return the provider ID on Charge 2.
	long GetFirstChargeProviderID();

	// (r.gonet 07/02/2014) - PLID 62567 - Sets the bill description edit box value.
	// The intention is to hide the prepending of the status prefix. The opposite of this function
	// is GetBillDescription()
	void SetBillDescription(CString &strBillDescription);
	// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, with the status prefix.
	CString CBillingModuleDlg::GetBillDescriptionWithPrefix();
	// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, without the status prefix.
	// The intention is to hide the prepending of the status prefix. The opposite of this function
	// is SetBillDescription(str)
	CString CBillingModuleDlg::GetBillDescription();
	// (s.tullis 2016-02-24 16:48) - PLID 68319
	void UpdateClaimFormSelection();

	// Generated message map functions
	// (a.walling 2008-05-13 14:57) - PLID 27591 - Use Notify handlers for DateTimePicker
	//{{AFX_MSG(CBillingModuleDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBillDeleteBtn();
	afx_msg void OnBillEditBtn();
	afx_msg void OnBillPreviewBtn();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnEditText();
	virtual void OnCancel();
	afx_msg void OnChangeEditDate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnChangeBillDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSave();
	afx_msg void OnSelectTab(short newTab, short oldTab);
	afx_msg void OnSelChosenCoordinatorCombo(long nRow);
	afx_msg void OnSelChosenCustomReportCombo(long nRow);
	afx_msg void OnBtnSaveCopy();
	afx_msg void OnQuoteMakeDefault();
	afx_msg void OnMergeToWord();
	afx_msg void OnTrySetSelFinishedCoordinatorCombo(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// (j.jones 2011-01-21 09:58) - PLID 42156 - changed to an enum
	BillingAccessType m_eHasAccess;
	BOOL m_bHasEverHadAccess;//Used to suppress some warnings if you've never been able to edit.
	UINT m_nIDTemplate;

	// (a.walling 2009-02-23 10:45) - PLID 11034 - Cached resized value
	DWORD m_dwSize;

	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized, so this is now dead code
	//CString m_strCharges;

	void GetTabRect15(CRect &rect);
	void ActivateInsuranceDlg();
	void ActivateBillingDlg();
	void ActivateBilling2Dlg();
	CDialog* m_pActiveDlg;
	void OnOK();
	BOOL m_bIsPopupAllowed; //Used to ensure that we disable and enable popups exactly once.

	// (r.gonet 07/09/2014) - PLID 62834 - Splits a bill's on hold charges into a separate bill
	void SplitOnHoldChargesIntoNewBill();
	// (r.gonet 07/09/2014) - PLID 62834 - Update the bill in the financial dialog.
	// - nBillID: The bill to refresh in the financial dialog
	// - msg: The message to send to the financial dialog. Defaults to bill edited.
	void RefreshBillInFinancialDialog(long nBillID, UINT msg = NXM_POST_EDIT_BILL);
	BOOL AttemptUpdateScheduler();

	void EnableButtons(BOOL bEnable);

	// (a.walling 2009-12-23 09:26) - PLID 7002 - Since we may be modeless, we need to ensure the OnShowWindow logic is only executed when necessary
	bool m_bVisibleState;

	NxTab::_DNxTabPtr m_tab;

	// (a.walling 2007-05-24 08:45) - PLID 26114
	COleCurrency m_cyTotalPoints;
	
	// (a.walling 2007-05-24 09:05) - PLID 26114
	COleCurrency m_cyAdjustedPoints; // adjustments to the total points for unsaved charges on this bill

	// (a.walling 2008-05-05 12:53) - PLID 29897 - Cached patient name values
	long m_nCachedPatientNameID;
	CString m_strCachedPatientName;
public:
	afx_msg void OnEnKillfocusEditDescription();
};