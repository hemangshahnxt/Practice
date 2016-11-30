#pragma once

// InsuranceBilling.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsuranceBilling dialog

#include "BillingRc.h"
#include "GlobalFinancialUtils.h"
#include "UB04Utils.h"

// (j.jones 2013-08-16 09:28) - PLID 58063 - moved ConditionDateTypeComboColumns to the cpp

// (a.walling 2014-02-25 13:34) - PLID 61024 - Removed ancient Access error 3048 handling and nonexistent datalist Exception event

class CInsuranceBilling : public CNxDialog
{
// Construction
public:
	// Bill description edit box
	CNxEdit* m_peditBillDescription;

	// Bill date edit box
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker* m_peditBillDate;

	CBrush m_brush;

	long m_nPatientID;
	// (a.walling 2008-05-05 13:15) - PLID 29897 - Patient name
	CString GetBillPatientName();

	int m_iActiveColumn;
	BOOL m_HasHCFABeenOpened;
	// (j.jones 2011-01-21 09:58) - PLID 42156 - changed to an enum
	BillingAccessType m_eHasAccess;
	// (r.gonet 07/02/2014) - PLID 62567 - Initialize to NULL
	CWnd* m_pBillingModuleWnd = NULL;

	long m_GuarantorID1;
	long m_GuarantorID2;
	long m_nPendingReferringPhyID;
	long m_nPendingUB92Box44ID;
	BOOL m_boInitialized;

	// (r.gonet 2016-04-07) - NX-100072 - Stores a copy of what is in the Condition Date Type combo
	// and the First Condition Date (or illness, the naming is inconsistent) NxTime control, plus stores
	// the values of the other additional claim dates whose type is not loaded into the Condition Date Type combo currently.
	ClaimDates m_claimDates;

	CTableChecker m_PatInsPartyChecker, m_refphyChecker, m_cptChecker;

	void ChangeAccess();
	
	//Returns SCR_ABORT_SAVE, SCR_NOT_SAVED, or SCR_SAVED
	int SaveChanges();

	void SetAccidentStatus();

	// (j.jones 2008-05-14 17:09) - PLID 30044 - returns the status of the test result controls,
	// so we can properly validate upon saving
	void CheckTestResultFields(BOOL &bEnabled, BOOL &bIsValid);

	// (j.jones 2008-05-14 17:25) - PLID 30044 - returns the value of the FormType setting
	long GetFormTypeID();

	// (c.haag 2009-09-03 14:11) - PLID 34781 - Ensures that the tab is fully loaded; including
	// any defaults there may be for new bills
	void EnsureInitialized();

	// (b.eyers 2015-07-14) - PLID 47739
	void UpdateInsuranceDates();

	// (j.jones 2010-08-17 11:40) - PLID 40135 - If there are two insured parties selected and they
	// have different categories (ie. one is Medical, one is Vision), warn the user and ask if they
	// wish to continue. Return FALSE if they said no.
	BOOL CheckWarnMismatchedInsuranceCategories();

	// (r.gonet 2016-04-07) - NX-100072
	void SetConditionDateType(ConditionDateType eConditionDateType);
	// (r.gonet 2016-04-07) - NX-100072
	void SetFirstConditionDate(COleDateTime dt);
	// (r.gonet 2016-04-07) - NX-100072
	void ClearFirstConditionDate();
	// (r.gonet 2016-04-07) - NX-100072
	void ClearAdditionalClaimDates();

	// (j.jones 2010-08-17 11:58) - PLID 40135 - store the Old values for insurance plans
	long m_nOldInsuranceID1;
	long m_nOldInsuranceID2;

public:
	NXTIMELib::_DNxTimePtr m_pWorkFrom, m_pWorkTo, m_pHospFrom, m_pHospTo, m_pAccident, m_pIllness;
	// (j.jones 2013-06-07 15:29) - PLID 41479 - added admission & discharge times
	NXTIMELib::_DNxTimePtr m_pAdmissionTime, m_pDischargeTime;

	CInsuranceBilling(CWnd* pParent);   // standard constructor
	~CInsuranceBilling();

	// (j.jones 2008-05-14 12:12) - PLID 30044 - added test results controls
	// (j.jones 2009-02-11 10:02) - PLID 33035 - added m_checkManualReview
// Dialog Data
	//{{AFX_DATA(CInsuranceBilling)
	enum { IDD = IDD_INSURANCE_BILLING };
	CNxStatic m_nxstaticTestResultLabel;
	CNxStatic m_nxstaticTestResultsIDLabel;
	CNxStatic m_nxstaticTestResultsTypeLabel;
	CNxEdit	m_editTestResults;
	NxButton	m_checkSendTestResults;
	NxButton	m_btnSendPaperwork;
	NxButton	m_checkSendCorrespondence;
	CNxIconButton	m_SelectRefPhysButton;
	CNxIconButton	m_SelectPCPButton;
	NxButton	m_outsideLabCheck;
	NxButton	m_autoAccidentRadio;
	NxButton	m_noAccidentRadio;
	NxButton	m_otherAccidentRadio;
	NxButton	m_unbatchedRadio;
	NxButton	m_paperRadio;
	NxButton	m_electronicRadio;
	NxButton	m_employmentRadio;
	CNxIconButton	m_printFormButton;
	CNxIconButton	m_openFormButton;
	CNxEdit	m_editBox19;
	CNxEdit	m_nxeditEditState;
	CNxEdit	m_nxeditEditMedicaidCode;
	CNxEdit	m_nxeditEditReferenceNumber;
	CNxEdit	m_nxeditEditAuthorizationNumber;
	// (j.jones 2013-08-13 12:34) - PLID 57902 - removed Box 10d
	CNxEdit	m_nxeditEditCharges;
	// (j.jones 2013-08-14 11:22) - PLID 57902 - removed UB boxes 34 - 36
	CNxStatic	m_nxstaticPwkTypeLabel;
	CNxStatic	m_nxstaticPwkTxLabel;
	// (j.jones 2013-08-13 12:34) - PLID 57902 - removed Box 10d
	CNxStatic	m_nxstaticLabelBox19;
	CNxStatic	m_nxstaticLabelUb92Box44Override;
	// (j.jones 2013-08-14 11:22) - PLID 57902 - removed UB boxes 34 - 36
	NxButton	m_checkManualReview;
	// (j.jones 2009-09-14 17:32) - PLID 35284 - added claim type
	CNxStatic	m_nxstaticClaimTypeLabel;
	// (j.jones 2010-06-10 09:49) - PLID 38507 - added HCFA Box 13 config
	CNxStatic	m_nxstaticHCFABox13Label;
	// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
	CNxStatic	m_nxstaticUBBox14Label;
	CNxEdit		m_nxeditEditUBBox14;
	CNxStatic	m_nxstaticUBBox15Label;
	CNxEdit		m_nxeditEditUBBox15;
	// (j.jones 2013-08-07 12:04) - PLID 58042 - added resumission code label
	CNxStatic	m_nxstaticResubmissionCodeLabel;
	// (j.jones 2013-08-13 11:23) - PLID 57902 - added additional screen for obscure claim fields
	CNxIconButton	m_btnClaimFields;
	// (j.jones 2016-05-26 15:36) - NX-100705 - added Original Reference Number label
	CNxStatic	m_nxstaticOriginalRefNoLabel;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_FormTypeCombo;
	NXDATALISTLib::_DNxDataListPtr m_pRefList;
	NXDATALISTLib::_DNxDataListPtr	m_pInsurance1;
	NXDATALISTLib::_DNxDataListPtr	m_pInsurance2;
	NXDATALISTLib::_DNxDataListPtr	m_pUB92Box44List;
	// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
	NXDATALIST2Lib::_DNxDataListPtr	m_ConditionDateTypeCombo;

	// (z.manning 2008-12-08 11:58) - PLID 32320 - Added an enum for the ref phys datalist's columns
	// (j.jones 2011-12-01 13:25) - PLID 46771 - added address columns
	enum EReferringPhysicianColumns {
		rpcID = 0,
		rpcName,
		rpcNpi,
		rpcRefPhysID,
		rpcUpin,
		rpcAddress1,
		rpcAddress2,
		rpcCity,
		rpcState,
		rpcZip,
	};

protected:

	// (a.walling 2007-08-27 11:00) - PLID 27026
	NXDATALIST2Lib::_DNxDataListPtr	m_pPWKType;
	NXDATALIST2Lib::_DNxDataListPtr	m_pPWKTx;

	// (j.jones 2008-05-14 12:12) - PLID 30044 - added test results controls
	NXDATALIST2Lib::_DNxDataListPtr	m_TestResultIDCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_TestResultTypeCombo;

	// (j.jones 2010-10-22 10:16) - PLID 40962 - added m_strTestResultType
	CString m_strTestResultType;

	// (j.jones 2009-09-14 17:32) - PLID 35284 - added claim type combo
	NXDATALIST2Lib::_DNxDataListPtr	m_ClaimTypeCombo;

	// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
	NXDATALIST2Lib::_DNxDataListPtr	m_PriorAuthTypeCombo;

	// (j.jones 2010-06-10 09:49) - PLID 38507 - added HCFA Box 13 config
	NXDATALIST2Lib::_DNxDataListPtr	m_HCFABox13Combo;

	// (j.jones 2010-06-14 10:59) - PLID 38507 - cache the old HCFA Box 13 value
	enum HCFABox13Over m_eOldHCFABox13Over;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsuranceBilling)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	// (j.jones 2010-06-14 13:39) - PLID 38507 - tells the caller whether we need to warn
	// that HCFA Box 13 setting is in use, and provides the value to warn about
	BOOL NeedWarnHCFABox13(HCFABox13Over &hb13_CurValue);

	// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
	//toggle the fields that need to change for the new/old HCFA form
	void UpdateDisplayedHCFAFields(long nInsuredPartyID, long nBillID);

	// (j.jones 2008-05-14 14:14) - PLID 30044 - OnSendTestResultCheck
	// Generated message map functions
	//{{AFX_MSG(CInsuranceBilling)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnClickBtnFormProperties();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedInsplan1(short nFlags);
	afx_msg void OnRequeryFinishedInsplan2(short nFlags);
	afx_msg void OnOpenForm();
	afx_msg void OnPrintForm();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelchangeMscomboFormType();
	afx_msg void OnSelChosenFormTypeCombo(long nRow);
	afx_msg void OnBtnEditPriorAuthNum();
	afx_msg void OnKillfocusEditAuthorizationNumber();
	afx_msg void OnSelChosenInsplan1(long nRow);
	afx_msg void OnRadioPaperBatch();
	afx_msg void OnRadioElectronicBatch();
	afx_msg void OnSelectPcp();
	afx_msg void OnSelectRefPhys();
	afx_msg void OnKillFocusEditCurrentDate();
	afx_msg void OnKillFocusEditFirstDate();
	afx_msg void OnKillFocusEditUnableToWorkFrom();
	afx_msg void OnKillFocusEditUnableToWorkTo();
	afx_msg void OnKillFocusEditHospitalizedFrom();
	afx_msg void OnKillFocusEditHospitalizedTo();
	afx_msg void OnTrySetSelFinishedRefPhys(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedUb92Box44List(long nRowEnum, long nFlags);
	afx_msg void OnSendPaperworkCheck();
	afx_msg void OnSendTestResultCheck();
	afx_msg void OnBtnEditClaimDates(); // (b.eyers 2016-04-07) - NX-100071 
	long GetPrimaryInsuredPartyID();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:	
	COleVariant m_varBoundItem;
	long GetBillID();
	void SetBillID(long BillID);
	void ReadBillData();
	void OnOK();
	void OnCancel();
	void PostFormTypeChanged();

	// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, with the status prefix.
	CString GetBillDescriptionWithPrefix();
//	void CalculateLineTotal(COleCurrency cyInsCoUnitFee, int iQuantity, int iMultiplier, double dblTax);

	// (j.jones 2013-08-13 13:54) - PLID 57902 - added member variables for fields that have
	// been moved to the BillClaimFieldsDlg
	CString m_strHCFABox8;
	CString m_strHCFABox9b;
	CString m_strHCFABox9c;
	CString m_strHCFABox10d;
	CString m_strHCFABox11bQual;
	CString m_strHCFABox11b;
	CString m_strUB92Box79;
	// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
	
	// (a.walling 2016-03-09 10:48) - PLID 68555 - UB04 Enhancements - Insert default occurrence code on save or on dialog creation to defer calculated default date that might depend on bill information
	CString m_ub04DefaultOccurrenceCodeToApply;
	UB04::ClaimInfo m_ub04ClaimInfo;
	UB04::ClaimInfo m_ub04ClaimInfoOld; // (a.walling 2016-03-07 08:53) - PLID 68499 - to detect changes for auditing

	// (r.goldschmidt 2016-03-07 12:32) - PLID 68541 - get default dates for UB04 additional fields dialog
	bool m_bUseConditionDate;
	void CalculateUB04DefaultDates(COleDateTime& dtDefaultDate, COleDateTime& dtDefaultFrom, COleDateTime& dtDefaultThrough);
	
	// (r.goldschmidt 2016-03-11 16:16) - PLID 68585 - UB04 Enhancements - Get box 80 defaults
	CString m_ub04DefaultRemarksToApply;

	// (j.jones 2016-05-24 14:39) - NX-100704 - added a function to repopulate the claim type
	// combo based on the current form type
	void RepopulateClaimTypeCombo();
	// (j.jones 2016-05-24 14:44) - NX-100704 - helper function to build the claim type combo
	void AddRowToClaimTypeCombo(long nFormType, ANSI_ClaimTypeCode ctcType);

	// (j.jones 2008-05-14 16:26) - PLID 30044 - this will enable/disable the rest result controls,
	// and is called after the user or the program checks/unchecks the 'send test results' box
	void PostSendTestResultsCheck();
	// (j.gruber 2009-07-13 09:22) - PLID 34724 - make sure we unbatch when manually set
	afx_msg void OnBnClickedRadioUnbatched();
	// (j.jones 2009-09-15 09:20) - PLID 35284 - added claim type combo
	void OnSelChosenClaimTypeCombo(LPDISPATCH lpRow);
	// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
	void OnSelChosenPriorAuthTypeCombo(LPDISPATCH lpRow);
	// (j.jones 2010-06-10 09:49) - PLID 38507 - added HCFA Box 13 config
	void OnSelChosenHCFABox13Combo(LPDISPATCH lpRow);
	// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
	void OnSelChangingConditionDateCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenConditionDateCombo(LPDISPATCH lpRow);
	// (j.jones 2013-06-07 15:29) - PLID 41479 - added admission & discharge times
	void OnKillFocusEditAdmissionTime();
	void OnKillFocusEditDischargeTime();
	// (j.jones 2013-08-13 11:23) - PLID 57902 - added additional screen for obscure claim fields
	afx_msg void OnBtnClaimFields();
};