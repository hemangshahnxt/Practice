#if !defined(AFX_INSURANCEDLG_H__712CC0B4_05E3_11D2_8087_00104B2FE914__INCLUDED_)
#define AFX_INSURANCEDLG_H__712CC0B4_05E3_11D2_8087_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// InsuranceDlg.h : header file
//
#include "PatientDialog.h"
#include "PatientsRc.h"
#include "client.h"

// (j.jones 2009-03-06 10:03) - PLID 28834 - added enum for AccidentType
enum InsuredPartyAccidentType
{
	ipatNone = -1,
	ipatEmployment = 1,
	ipatAutoAcc = 2,
	ipatOtherAcc = 3,
};

/////////////////////////////////////////////////////////////////////////////
// CInsuranceDlg dialog

class CInsuranceDlg : public CPatientDialog
{
public:

	void CopyPatientInfo();
	NXDATALISTLib::_DNxDataListPtr m_RelateCombo;
	NXDATALISTLib::_DNxDataListPtr	m_CompanyCombo;
	NXDATALISTLib::_DNxDataListPtr	m_ContactsCombo;
	NXDATALISTLib::_DNxDataListPtr m_PlanList;
	NXDATALISTLib::_DNxDataListPtr m_GenderCombo;
	NXDATALISTLib::_DNxDataListPtr m_listRespType;
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_CountryList;

	BOOL m_bDropPlanBox;
	long m_CurrentID;

	// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
	CTableChecker m_companyChecker, m_planChecker, m_inscontactChecker;

	virtual int Hotkey(int key);
	// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
	//COleDateTime m_dtDateTmp;
	void UpdateInsuranceCo();
	int AddInsuredPartyRecord();
	
	// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
	//CString m_strNewDataTmp;
	//CString m_strCurDataTmp;
	bool m_bSettingBox;
	OLE_COLOR m_nColor;
	CInsuranceDlg(CWnd* pParent);   // standard constructor
	virtual ~CInsuranceDlg();
	virtual void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void UpdatePlan();
	void FillRadioButtons();
	void Load();
	// (a.walling 2010-10-13 11:08) - PLID 40978
	void LoadInsInfo();
	// (a.walling 2010-10-13 11:08) - PLID 40978
	void HandleTableCheckers();
	void Save(int nID);
	void TransferResponsibilities(int iInsuredParty, CString strSourceResp, CString strDestResp);
	void EnableInsuredInfo(bool bEnable); //Disable all the insured party info if you have no insurance, it doesn't get saved anywhere anyway.

	void AssignResp(long nToRespID);
	// (j.jones 2008-09-11 10:32) - PLID 14002 - Made InactivateResp return TRUE if it succeeded,
	// and FALSE if it failed (or was cancelled by the user). Also added bSkipExpireDateCheck
	// which, if enabled, will tell the function not to validate or alter the ExpireDate.
	BOOL InactivateResp(BOOL bSkipExpireDateCheck = FALSE);

	// (j.jones 2009-03-05 09:17) - PLID 28834 - added accident controls
// Dialog Data
	//{{AFX_DATA(CInsuranceDlg)
	enum { IDD = IDD_INSURANCE_DLG };
	CNxIconButton	m_btnSwap;
	//NxButton	m_radioCopayAmount;
	//NxButton	m_radioCopayPercent;
	CNxIconButton	m_btnEditCPTNotes;
	//NxButton	m_checkWarnCoPay;
	CNxIconButton	m_btnEditReferrals;
	CNxIconButton	m_copyEmployerInfoButton;
	CNxIconButton	m_editInsList;
	CNxIconButton	m_prev;
	CNxIconButton	m_next;
	CNxIconButton	m_delete;
	CNxIconButton	m_add;
	CNxIconButton	m_insCountButton;
	CNxIconButton	m_copyPatientInfoButton;
	CNxIconButton m_btnDeductOOP; // (j.gruber 2010-07-30 10:50) - PLID 39727
	CNxColor	m_contactBkg;
	CNxColor	m_patientBkg;
	CNxColor	m_placementBkg;
	CNxColor	m_selectionBkg;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditEmployerSchoolBox;
	CNxEdit	m_nxeditInsPartySsn;
	CNxEdit	m_nxeditPhoneBox;
	CNxEdit	m_nxeditMemoBox;
	CNxEdit	m_nxeditContactAddressBox;
	CNxEdit	m_nxeditContactCityBox;
	CNxEdit	m_nxeditContactStateBox;
	CNxEdit	m_nxeditContactZipBox;
	CNxEdit	m_nxeditContactPhoneBox;
	CNxEdit	m_nxeditContactExtBox;
	CNxEdit	m_nxeditContactFaxBox;
	//CNxEdit	m_nxeditCopay;
	CNxEdit	m_nxeditCoMemo;
	CNxEdit	m_nxeditTypeBox;
	CNxEdit	m_nxeditInsuranceIdBox;
	CNxEdit	m_nxeditFecaBox;
	CNxColor m_accBkg;
	CNxEdit m_editAccState;
	NxButton m_radioEmploymentAcc;
	NxButton m_radioAutoAcc;
	NxButton m_radioOtherAcc;
	NxButton m_radioNoneAcc;
	// (j.jones 2009-06-23 11:39) - PLID 34689 - added ability to submit as primary, when not actually primary
	NxButton m_checkSubmitAsPrimary;
	// (j.jones 2010-05-18 10:01) - PLID 37788 - supported a second ID For Insurance
	CNxStatic m_nxstaticInsPlanNameLabel;
	CNxStatic m_nxstaticInsPlanTypeLabel;
	CNxStatic m_nxstaticIDForInsuranceLabel;
	CNxStatic m_nxstaticPatientIDForInsuranceLabel;
	CNxEdit m_nxeditPatientIDForInsurance;
	// (j.jones 2010-07-19 10:35) - PLID 31082 - added ability to create an eligibility request
	CNxIconButton	m_btnCreateEligibilityRequest;
	// (j.jones 2012-10-24 14:28) - PLID 36305 - added Title
	CNxEdit	m_nxeditTitleBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides	
	//{{AFX_VIRTUAL(CInsuranceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;
	virtual void StoreDetails();
	// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
	//CWnd* m_pGiveFocusToAfterUpdate;
	//CString m_sqlInsurance;
	//long m_lastID;
	long m_lastIns;
	long m_id;
	CString m_strPatientName;
	bool SaveAreaCode(long nID);
	void FillAreaCode(long nPhoneID);
	void SecureControls();
	CString m_strAreaCode;
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;
	NXTIMELib::_DNxTimePtr m_nxtBirthDate, m_nxtEffectiveDate, m_nxtInactiveDate;
	COleDateTime m_dtBirthDate, m_dtEffectiveDate, m_dtInactiveDate;

	// (j.jones 2009-03-05 09:22) - PLID 28834 - added accident date
	NXTIMELib::_DNxTimePtr m_nxtAccidentDate;
	COleDateTime m_dtAccidentDate;
	bool m_bSavingAccidentDate;

	//bool m_bSavingBirthDate;
	bool m_bSavingEffectiveDate, m_bSavingInactiveDate;
	void SetCompanyWithInactive(long nCompanyID);
	long m_nCurrentInsCoID;
	bool m_bInsPlanIsRequerying;
	bool m_bFormattingField;
	bool m_bFormattingAreaCode;

	// (j.gruber 2010-08-02 09:26) - PLID 39729 - remove and add new copay fields
	//void OnRadioCopay();
	//void LoadCopayInfo();

	//TES 6/11/2007 - PLID 26257 - A list of the available "Secondary Reason Codes" (Medicare only).
	NXDATALIST2Lib::_DNxDataListPtr m_pSecondaryReasonCode;

	void UpdatePatientForHL7(); // (z.manning 2009-01-08 17:11) - PLID 32663

	// (j.jones 2009-03-06 10:01) - PLID 28834 - added OnAccidentTypeChanged
	void OnAccidentTypeChanged();

	// (m.hancock 2006-11-02 10:32) - PLID 21274 - Added deductible amount for insurances.
	// (j.gruber 2006-12-29 15:14) - PLID 23972 - take this out
	//COleCurrency m_cyDeductible;

	// (j.gruber 2009-10-08 09:56) - PLID 35826 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	// (j.gruber 2010-08-02 14:01) - PLID 39729
	NXDATALIST2Lib::_DNxDataListPtr m_pPayGroupsList;

	// (j.jones 2009-03-05 09:33) - PLID 28834 - added accident functions
	// Generated message map functions
	//{{AFX_MSG(CInsuranceDlg)
	afx_msg void OnReturnCompanyCombo();
	// (d.thompson 2012-08-28) - PLID 52333 - Changed from SelChanged to SelChosen
	afx_msg void OnSelChosenInsurancePlan(long nRow);
	afx_msg void OnReturnInsurancePlanBox();
	afx_msg void OnBtnNextParty();
	afx_msg void OnBtnPrevParty();
	afx_msg void OnBtnAddInsurance();
	afx_msg void OnBtnDeleteInsurance();
	afx_msg void OnSelChosenCompanyCombo(long nRow);
	afx_msg void OnRequeryFinishedCompanyCombo(short nFlags);
	afx_msg void OnRequeryFinishedInsurancePlanBox(short nFlags);
	afx_msg void OnCopyPatientInfo();
	afx_msg void OnEditInsuranceList();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnCopyEmpInfo();
	afx_msg void OnClosedUpGenderList(long nSelRow);
	afx_msg void OnSelChosenContactsCombo(long nRow);
	afx_msg void OnRequeryFinishedContactsCombo(short nFlags);
	afx_msg void OnEditReferrals();
	afx_msg void OnSwap();
	afx_msg void OnSelChosenRespTypeCombo(long nRow);
	afx_msg void OnEditInsRespTypes();
	afx_msg void OnKillFocusBirthDateBox();
	afx_msg void OnKillFocusEffectiveDate();
	afx_msg void OnTrySetSelFinishedCompanyCombo(long nRowEnum, long nFlags);
	//afx_msg void OnCheckWarnCopay();
	afx_msg void OnBtnEditCptNotes();
	afx_msg void OnKillFocusInactiveDate();
	afx_msg void OnKillfocusSsn();
	afx_msg void OnSelChosenRelateCombo(long nRow);
	//afx_msg void OnRadioCopayAmount();
	//afx_msg void OnRadioCopayPercent();
	afx_msg void OnSelChangingInsurancePlanBox(long FAR* nNewSel);
	afx_msg void OnSelChangingCompanyCombo(long FAR* nNewSel);
	afx_msg void OnEditingFinishedGenderList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedGenderList(long nNewSel);
	afx_msg void OnFocusLostGenderList();
	afx_msg void OnSelChosenSecondaryReasonCode(LPDISPATCH lpRow);
	afx_msg void OnKillFocusCurAccDate();
	afx_msg void OnRadioEmployment();
	afx_msg void OnRadioAutoAccident();
	afx_msg void OnRadioOtherAccident();
	afx_msg void OnRadioNone();
	// (j.jones 2009-06-23 11:39) - PLID 34689 - added ability to submit as primary, when not actually primary
	afx_msg void OnCheckSubmitAsPrimary();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

	// (j.jones 2012-01-26 09:03) - PLID 47787 - UpdateDeductibleOOPLabel can
	// color the button red if there is content in use
	void UpdateDeductibleOOPLabel();

	BOOL IsDateValid(NXTIMELib::_DNxTimePtr& pdt);
	// (j.jones 2008-09-11 10:50) - PLID 14002 - added varExpireDate
	void ReactivateInsurance(int iInsType, _variant_t varExpireDate);
	// (j.jones 2010-07-19 10:35) - PLID 31082 - added ability to create an eligibility request
	afx_msg void OnBtnCreateInsEligibilityRequest();
	// (j.gruber 2010-07-30 10:50) - PLID 39727 - added deductible/oop
	afx_msg void OnBnClickedInsEditDeductOop();
	
	void EditingStartingPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	void OnSelChangingCountryList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenCountryList(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSURANCEDLG_H__712CC0B4_05E3_11D2_8087_00104B2FE914__INCLUDED_)
