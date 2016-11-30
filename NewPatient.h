#include "Client.h"
#include "PatientGroupsDlg.h"
#include "doesexist.h"
#include "ReferralSubDlg.h"
#if !defined(AFX_NEWPATIENT_H__1BFDE1A7_D556_11D2_9340_00104B318376__INCLUDED_)
#define AFX_NEWPATIENT_H__1BFDE1A7_D556_11D2_9340_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewPatient.h : header file
//
// (c.haag 2010-10-04 10:21) - PLID 39447 - We're now dependent on this for CNewPatientInsuredParty
#include "NewPatientAddInsuredDlg.h"

class CAddProcedureDlg;
class COPOSMSRDevice;
/////////////////////////////////////////////////////////////////////////////
// CNewPatient dialog

class CNewPatient : public CNxDialog
{
// Construction
public:
	void SaveGroups();
	CArray<int, int> m_arGroupIDs;
	// (r.farnworth 2016-03-03 10:31) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
	CString m_strPreselectFirst;
	CString m_strPreselectLast;
	CString m_strPreselectGender;
	CString m_strPreselectEmail;
	CString m_strPreselectAddress1;
	CString m_strPreselectAddress2;
	CString m_strPreselectCity;
	CString m_strPreselectState;
	CString m_strPreselectZip;
	CString m_strPreselectHomePhone;
	CString m_strPreselectCellPhone;
	COleDateTime m_dtPreselectBirthdate;
	// (j.gruber 2010-01-11 13:13) - PLID 36140 - added referralIDtoselect
	CNewPatient(CWnd* pParent, UINT nIDTemplate /*= IDD_NEW_PATIENT*/, long nReferralIDToSelect = -1);   // standard constructor
	virtual ~CNewPatient();
	NXDATALISTLib::_DNxDataListPtr m_GenderCombo;
	NXDATALISTLib::_DNxDataListPtr m_PrefixCombo;
	NXDATALISTLib::_DNxDataListPtr m_PatientTypeCombo;
	NXDATALISTLib::_DNxDataListPtr m_pProcedureList;
	bool m_bForGC;		//This is for special gift-certificate stuff only.
	bool m_bFromReassign;		// (r.farnworth 2016-03-08 11:39) - PLID 68454
	bool m_bFromResEntry;
	NXDATALISTLib::_DNxDataListPtr		m_pDocList;
	NXDATALISTLib::_DNxDataListPtr		m_pCoordList;
	NXDATALISTLib::_DNxDataListPtr		m_pRefPhysList;
	// (r.gonet 05/16/2012) - PLID 48561 - Allow the user to setup an affiliate physician
	NXDATALIST2Lib::_DNxDataListPtr		m_pAffiliatePhysList;
	NXDATALISTLib::_DNxDataListPtr		m_pRefPatientList;
	NXDATALISTLib::_DNxDataListPtr		m_pRelatedPatientList;
	NXDATALISTLib::_DNxDataListPtr		m_PreferredContactCombo;
	NXDATALIST2Lib::_DNxDataListPtr		m_pMultiReferralList;
	// (c.haag 2010-10-04 11:22) - PLID 39447 - List of new insured parties
	NXDATALIST2Lib::_DNxDataListPtr		m_dlInsuredParties;

	// (s.dhole 2013-06-04 12:59) - PLID 12018
	NXDATALIST2Lib::_DNxDataListPtr  m_pDefaultLocation;

	// (z.manning 2008-12-08 11:58) - PLID 32320 - Added an enum for the ref phys datalist's columns
	// (j.jones 2011-12-01 13:25) - PLID 46771 - added address columns
	enum EReferringPhysicianColumns {
		rpcID = 0,
		rpcName,
		rpcNpi,
		rpcRefPhysID,
		rpcUpin,
		rpcShowProcWarning,
		rpcProcWarning,
		rpcAddress1,
		rpcAddress2,
		rpcCity,
		rpcState,
		rpcZip,
	};

	// (r.gonet 05/16/2012) - PLID 48561 - Added an enum for affiliate physician drop down
	enum EAffiliatePhysicianColumns {
		afphID = 0,
		afphName,
		afphNpi,
		afphAffiliatePhysID,
		afphUpin,
		afphAddress1,
		afphAddress2,
		afphCity,
		afphState,
		afphZip,
	};

	// (v.maida - 2014-08-18 16:21) - PLID 30758 - Enumeration for provider dropdown.
	enum EProviderColumns {
		pcID = 0,
		pcFullName,
	};

	// (v.maida - 2014-08-18 16:21) - PLID 30758 - Enumeration for possible new contact choices that can be made when the New Contact dialog is generated from the new patient dialog. 
	enum EContactChoices {
		providerChoice = 0,
		refPhysChoice,
	};

	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);

	// (z.manning 2009-07-08 15:08) - PLID 27251 - Changed the email required asterisk to an NxLabel
	// (j.gruber 2009-10-13 09:26) - PLID 10723 - added edits for insurance and policy number boxes
	// (c.haag 2010-10-04 10:21) - PLID 39447 - Added insured party buttons
 // Dialog Data
	//{{AFX_DATA(CNewPatient)
	enum { IDD = IDD_NEW_PATIENT };
	NxButton	m_btnMultipleReferrals;
	CNxIconButton	m_editButton;
	CNxIconButton	m_resumeButton;
	CNxIconButton	m_cancelButton;
	CNxIconButton	m_scheduleButton;
	CNxIconButton	m_procedureBtn;
	CNxIconButton	m_addanotherBtn;
	CNxIconButton	m_ffaBtn;
	CNxIconButton	m_EditPtTypesBtn;
	CNxIconButton	m_NewRefPhysBtn;
	CNxIconButton m_NewInsPartyBtn;
	CNxIconButton m_EditInsPartyBtn;
	CNxIconButton m_DeleteInsPartyBtn;
	NxButton				m_inform;
	NxButton				m_mirror;
	NxButton				m_united;
	NxButton				m_hl7;
	NxButton			m_patient;
	NxButton			m_prospect;
	CNxIconButton		m_inquiry;
	CNxIconButton		m_group;
	NxButton			m_homePriv;
	NxButton			m_workPriv;
	CNxEdit	m_nxeditIdBox;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditSsnBox;
	CNxEdit	m_nxeditHomePhoneBox;
	CNxEdit	m_nxeditWorkPhoneBox;
	CNxEdit	m_nxeditExtPhoneBox;
	CNxEdit	m_nxeditCellPhoneBox;
	CNxEdit	m_nxeditEmailBox;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticPrefixReq;
	CNxLabel	m_nxlabelEmailReq;
	CNxStatic	m_nxstaticPatientTypeReq;
	CNxStatic	m_nxstaticFirstNameReq;
	CNxStatic	m_nxstaticLastNameReq;
	CNxStatic	m_nxstaticAddress1Req;
	CNxStatic	m_nxstaticCityReq;
	CNxStatic	m_nxstaticStateReq;
	CNxStatic	m_nxstaticZipReq;
	CNxStatic	m_nxstaticSsnReq;
	CNxStatic	m_nxstaticDobReq;
	CNxStatic	m_nxstaticDoctorReq;
	CNxStatic	m_nxstaticCoordinatorReq;
	CNxStatic	m_nxstaticRefPhysReq;
	// (r.gonet 05/16/2012) - PLID 48561
	CNxStatic	m_nxStaticAffiliatePhysReq;
	CNxStatic	m_nxstaticRefPatReq;
	CNxStatic	m_nxstaticGenderReq;
	CNxStatic	m_nxstaticHomePhoneReq;
	CNxStatic	m_nxstaticWorkPhoneReq;
	CNxStatic	m_nxstaticCellPhoneReq;
	CNxStatic	m_nxstaticNoteReq;
	CNxStatic	m_nxstaticMiddleNameReq;
	CNxStatic	m_nxstaticReferralReq;
	CNxStatic	m_nxstaticProcedureReq;
	NxButton	m_btnNewptReferralArea;
	CNxEdit		m_edtIDForInsurance;
	CNxEdit		m_edtInsPolGrouNum;
	// (j.jones 2010-05-06 14:37) - PLID 38482 - added fixed names for these insurance labels
	CNxStatic	m_nxstaticInsInfoLabel;
	CNxStatic	m_nxstaticIDForInsurance;
	CNxStatic	m_nxstaticPolicyGroupNum;

	// (s.dhole 2013-06-04 13:24) - PLID  12018
	CNxStatic	m_nxStaticLocationReq;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewPatient)
	public:
	virtual int DoModal(long *id);
	/* (v.maida - 2014-02-06 15:15) - PLID 60120 - PreTranslateMessage() removed because it's no longer needed to handle Alt-key combinations.
	   Keeping it in results in it attempting to save patients twice.
	virtual BOOL PreTranslateMessage(MSG* pMsg);*/
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam);	//(a.wilson 2012-1-11) PLID 47485
	//}}AFX_VIRTUAL

// Implementation
protected:
	CReferralSubDlg* m_pdlgReferralSubDlg;
	NXDATALIST2Lib::_DNxDataListPtr m_pCountryList;			// v.arth 2009-06-01 PLID 34386

	UINT m_nIDTemplate;
	BOOL Is640x480Resource();
	bool TestRequiredField(UINT id, long nFlag, const CString& strFieldName);
	bool MissingRequiredFields();
	bool Save (void);
	long *m_id;
	long m_nOriginalUserDefinedID;
	CAddProcedureDlg *m_pAddProDlg;
	CTableChecker m_groupChecker;
	void CNewPatient::FillAreaCode(long nID);
	bool CNewPatient::SaveAreaCode(long nID);
	CString m_strAreaCode;
	CPatientGroupsDlg m_dlgGroups;
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;
	long m_lRequiredFields;
	//(e.lally 2007-06-21) PLID 24334 - Long value of the addional field options
	long m_lAdditionalFields;
	CDoesExist m_deInform;
	CDoesExist m_deMirror;
	CDoesExist m_deUnited;
	NXTIMELib::_DNxTimePtr m_nxtBirthDate;
	long m_nPrimaryMultiReferralID;
	void AddMultiReferral(long nReferralID, BOOL bSilent);
	//(e.lally 2007-06-21) PLID 24334 - Check if the additional requirements are fulfilled
	bool MissingAdditionalField();
	// (d.moore 2007-08-15) - PLID 25455 - Check to see if there is an inquiry entry that
	//  the user would like to use when creating a new patient. The function returns the
	//  ID value of the selected inquiry person. Returns -1 if an inquiry was not selected,
	//  or if there were no matches in the inquiry list to choose from.
	long CheckForInquiryConversion(const CString &strFirstName, const CString &strLastName, const CString &strEmail);
	//(e.lally 2008-08-29) PLID 16395 - Copied from General 1
	CString GetPreferredContactTypeStringFromID(long nID);

	// (z.manning 2009-07-08 15:52) - PLID 27251 - Need to be able to decline email address if it's
	// set as a required field.
	BOOL m_bEmailDeclined;
	void PromptDeclineEmail();

	// (j.gruber 2009-10-08 09:56) - PLID 35826 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	// (j.gruber 2010-01-11 13:23) - PLID 36140 - referralID
	long m_nDefaultReferralID;

	// (j.jones 2010-05-06 14:41) - PLID 38482 - used to cache the custom field ID for OHIP Data
	long m_nHealthNumberCustomField;
	long m_nVersionCodeCustomField;

	// (c.haag 2010-10-04 10:21) - PLID 39447 - List of new insured parties
	CArray<CNewPatientInsuredParty,CNewPatientInsuredParty&> m_aInsuredParties;

	// (c.haag 2010-10-04 15:20) - PLID 39447 - Moved validation into its own function. This is called early
	// in the Save event because there's no need to be running all those queries when we find a problem in memory.
	BOOL InsuranceInfoIsInvalid();

	//r.wilson PLID 39357 10/28/2011
	//BOOL bUserCanWrite;
	//BOOL bUserCanWriteOnlyWithPass;

	// (c.haag 2010-10-13 10:26) - PLID 39447 - Edit an insured party
	void DoInsuredPartyEdit(NXDATALIST2Lib::IRowSettingsPtr& pRow);

	// (j.jones 2010-05-06 14:49) - PLID 38482 - added custom fields, used by OHIP
	void SaveCustomInfo(long nPatientID, long nFieldID, CString strValue);

	// (c.haag 2010-10-04 10:21) - PLID 39447 - Call this function to fill the demographic
	// section of a CNewPatientInsuredParty object with the patient demographics in this
	// window.
	void FillPatientInfo(CNewPatientInsuredParty& patient);

	//r.wilson  PLID 39357 10/26/2011
	BOOL CNewPatient::IsIdInvalid(int id, CString& strErrors );

	// (r.goldschmidt 2014-07-17 12:07) - PLID 62774 - Check if birth date valid. Fix if set to future date.
	bool ValidateDateOfBirth();
	bool ValidateDateOfBirth(CString& dob);

	// (r.goldschmidt 2014-07-17 12:21) - PLID 62774 - In New Patient Dialog, check against possible duplicates once first name, last name, and date of birth are entered.
	bool ReadyToCheckForDuplicatePatients();
	bool CheckForDuplicatePatients();

	// (v.maida - 2014-08-18 16:30) - PLID 30758 - Determine what type of person was added into the New Contact dialog, and return an enumeration indicating 
	// the choice.
	int GetContactChoice(long contactID);

	// (v.maida 2014-08-18 16:14) - PLID 30758 - Added a single function to handle choosing referring physicians and/or providers.
	void HandleContactChoice(EContactChoices eContactChoice);

	// (r.farnworth 2016-03-03 12:16) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
	void PopulateWithPreselects();

	// (j.gruber 2009-10-07 17:16) - PLID 35826 - added onkillfocuscity
	// Generated message map functions
	//{{AFX_MSG(CNewPatient)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnChangeExtPhoneBox();
	afx_msg void OnChangeHomePhoneBox();
	afx_msg void OnChangeWorkPhoneBox();
	afx_msg void OnChangeZipBox();
	afx_msg void OnKillfocusCityBox();
	afx_msg void OnKillfocusZipBox();
	afx_msg void OnKillfocusSsnBox();
	afx_msg void OnChangeFirstNameBox();
	afx_msg void OnChangeMiddleNameBox();
	afx_msg void OnChangeLastNameBox();
	afx_msg void OnChangeAddress2Box();
	afx_msg void OnChangeAddress1Box();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnChangeSSN();
	afx_msg void OnChangeBirthDate();
	afx_msg void OnChangeCellPhoneBox();
	afx_msg void OnProcedures();
	afx_msg void OnSaveAndSchedule();
	afx_msg void OnSaveAndEdit();
	afx_msg void OnSaveAndResume();
	afx_msg void OnGroups();
	afx_msg void OnSelChosenPrefix(long nRow);
	afx_msg void OnSelChosenGender(long nRow);
	afx_msg void OnBtnEditPatientTypes();
	afx_msg void OnSaveAndAddAnother();
	afx_msg void OnSaveAndFFA();
	afx_msg void OnClickInquiry();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChosenRefPatientList(long nRow);
	afx_msg void OnSelChosenRefPhysList(long nRow);
	afx_msg void OnNewRefPhys();
	afx_msg void OnRequeryFinishedRefPhysList(short nFlags);
	afx_msg void OnRequeryFinishedPatientTypeCombo(short nFlags);
	afx_msg void OnRequeryFinishedDoclist(short nFlags);
	afx_msg void OnRequeryFinishedCoordlist(short nFlags);
	afx_msg void OnRequeryFinishedPtPrefixList(short nFlags);
	afx_msg void OnRequeryFinishedRefPatientList(short nFlags);
	afx_msg void OnCheckMultipleReferralSources();
	afx_msg void OnNewptAddReferral();
	afx_msg void OnNewptMakePrimaryReferral();
	afx_msg void OnNewptRemoveReferral();
	afx_msg void OnSelChangedNewptReferralList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChangingRelatedPat(long FAR* nNewSel);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblClickCellInsList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString str, m_informPath, m_mirrorPath, m_unitedPath;
	// (r.goldschmidt 2014-07-17 12:21) - PLID 62774 - In New Patient Dialog, check against possible duplicates once first name, last name, and date of birth are entered.
	CString m_strCurrentFirstName, m_strCurrentMiddleName, m_strCurrentLastName;
	COleDateTime m_dtCurrentBirthDate;
	bool m_bTestForDuplicates;
protected:
	afx_msg void OnBnClickedListInsAdd();
	afx_msg void OnBnClickedListInsEdit();
	afx_msg void OnBnClickedListInsDelete();
	// (j.jones 2011-06-24 16:57) - PLID 31005 - supported OnEnKillfocusNewpatInsIdBox
	afx_msg void OnEnKillfocusNewpatInsIdBox();
	// (j.jones 2011-06-24 16:57) - PLID 31005 - supported OnEnKillfocusNewpatInsFecaBox
	afx_msg void OnEnKillfocusNewpatInsFecaBox();
	//r.wilson  PLID 39357 10/26/2011
	afx_msg void OnKillFocusID();
	afx_msg void OnNewProvider();
public:
	// (r.gonet 05/16/2012) - PLID 48561
	void RequeryFinishedNpAffiliatePhysList(short nFlags);
	// (s.dhole 2013-06-04 13:24) - PLID  12018
	void SelChangingNewPatientLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void KillFocusDobBox(); // (r.goldschmidt 2014-07-17 17:50) - PLID 62774

	//TES 8/13/2014 - PLID 63194 - Added tracking for the prospect status, in case our caller is curious
protected:
	bool m_bIsProspect;
public:
	bool GetIsProspect()
	{
		return m_bIsProspect;
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPATIENT_H__1BFDE1A7_D556_11D2_9340_00104B318376__INCLUDED_)
