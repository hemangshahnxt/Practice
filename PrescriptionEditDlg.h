#if !defined(AFX_PRESCRIPTIONEDITDLG_H__54C82712_172B_417B_9077_F18D940F7D92__INCLUDED_)
#define AFX_PRESCRIPTIONEDITDLG_H__54C82712_172B_417B_9077_F18D940F7D92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrescriptionEditDlg.h : header file
//

// (j.jones 2008-05-13 11:15) - PLID 29732 - created
// (j.fouts 2012-11-01 15:08) - PLID 53566 - Moved all members and methods related to the Queue into PrescriptionQueueDlg.h

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionEditDlg dialog

// only for MSXML2 namespace definition
#include "SOAPUtils.h"
#include "SureScriptsPractice.h"

#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "NxAPI.h"

#include "NexERxSetupDlg.h"
#include "NexERxLicense.h"
#include "PrescriptionUtilsAPI.h"	// (j.jones 2013-03-27 17:25) - PLID 55920 - we really do need the API version here
#include "AutoPopulatingSig.h"

// (j.jones 2012-11-15 14:39) - PLID 53765 - Added an enum to return to the caller so that they know what this dialog did,
// create a new prescription, edit an existing one, or delete one.
// These use negative values to differentiate from traditional CDialog return values.
// (b.savon 2016-04-19 7:27) - PLID-68647 - I changed epdrvIDCANCEL_Invalid to be a legitimate return value because it is.  I've renamed it
// to epdrvIDCANCEL so that it can be used in a case where the user actually does cancel opening the prescription, currently for taking
// ownership but can be used in other scenarios if they arise in the future.
enum PrescriptionDialogReturnValue {
	//because this enum is an override for EndDialog values, I added these IDOK/IDCANCEL enums
	//just incase some wacky windows functionality causes the dialog to close with them,
	//but that should never, ever happen
	epdrvIDOK_Invalid = 1, //placeholder in the case where IDOK is returned, though this should never happen
	epdrvIDCANCEL = 2, //placeholder in the case where IDCANCEL is returned
	
	//the rest should be the only real return values ever actually used
	epdrvNewRx_Obsolete = -2,	// (j.fouts 2013-03-12 15:09) - PLID 52973 - This is no longer responsible for adding a new rx so this return value is obsolete
	epdrvEditRx = -3,
	epdrvDeleteRx = -4,
	epdrvErrorRx = -5, // (b.savon 2013-05-03 09:58) - PLID 51705 - Error return
};

class CPrescriptionEditDlg : public CNxDialog
{
// Construction
public:
	// (j.jones 2012-10-31 15:05) - PLID 52819 - Added nCurrentlyOpenedEMNID, the ID of the EMN currently being edited, and bIsEMRTemplate.
	// This EMNID may or may not necessarily be the EMNID associated with the prescription we are editing.	
	CPrescriptionEditDlg(CWnd* pParent, long nCurrentlyOpenedEMNID = -1, BOOL bIsEMRTemplate = FALSE);   // standard constructor
	~CPrescriptionEditDlg();

//DoModal Overloads
public:
	//Moved CreateNewPrescription into PrescriptionUtils

	//EditPrescription is called instead of DoModal()
	// (b.savon 2013-03-18 16:06) - PLID 55477 - Pass in a struct
	// (b.savon 2013-01-28 10:01) - PLID 51705 - Give this function the ability to have a hidden dialog too
	// (j.jones 2008-10-13 10:09) - PLID 17235 - this function now requires a patient ID
	// (j.jones 2013-11-25 09:15) - PLID 59772 - added the drug interaction struct, if it is filled then
	// the Rx dialog needs to show drug interactions immediately upon opening
	// (j.jones 2016-01-06 15:29) - PLID 67823 - added bIsNewRx, so the dialog knows whether or not the prescription has just been created
	// (r.farnworth 2016-01-08 15:34) - PLID 58692 - Added bRePrescribe
	int EditPrescription(bool bIsNewRx, PrescriptionInfo rxInformation, DrugInteractionInfo *drugInteractionsToShow = NULL,
		BOOL bOpenReadOnly = FALSE, bool bSilentDialog = false, BOOL bRePrescribe = FALSE);

	// (j.jones 2012-11-14 11:00) - PLID 52819 - removed functions to create/edit from an EMN

	int ViewRenewalRequest(long nRenewalID);

// Accessor Functions
public:
	// (j.jones 2009-02-19 10:45) - PLID 33165 - added GetPrescriptionID to expose m_nPrescriptionID
	long GetPrescriptionID();

	// (z.manning 2009-05-13 14:15) - PLID 28554 - Use this to set the order set ID if this prescription
	// is part of an order set.
	void SetOrderSetID(const long nOrderSetID);

	// (j.jones 2011-05-02 15:39) - PLID 43450 - added GetPatientExplanation to expose m_strPatientExplanation
	CString GetPatientExplanation();

	// (j.fouts 2012-11-07 16:23) - PLID 53566 - Expose m_bChangesMade
	bool GetChangesMade();
	// (r.farnworth 2016-01-08 15:36) - PLID 58692
	void SaveAll();
	// (r.farnworth 2016-01-18 11:15) - PLID 67750
	void SavePackagingInfo(NexTech_Accessor::_NexERxMedicationPackageInformationPtr pPackage);
	void LoadPackagingInfo();

// Dialog Data
private:
	// (a.walling 2009-04-20 15:55) - PLID 34026 - Checkbox to send electronically
	//{{AFX_DATA(CPrescriptionEditDlg)
	enum { IDD = IDD_PRESCRIPTION_EDIT_DLG };

// Controls
private:
	CNxStatic m_nxstaticDateLabel;
	// (b.savon 2013-04-09 10:19) - PLID 56153
	CNxStatic m_nxstaticInternalDate; 
	CNxStatic m_nxstaticSampleInternalLabel;
	CNxStatic m_nxstaticPriorAuthInternalLabel;
	CNxStatic m_nxstaticFirstDateLabel;
	// (b.savon 2013-05-21 09:17) - PLID 56648
	CNxStatic m_nxstaticStrengthInternalLabel;
	CNxIconButton	m_btnPreview;
	CNxEdit	m_editFirstDate;
	CNxEdit	m_editQuantity;
	CNxEdit	m_editEnglishExplanation;
	CNxEdit	m_editPrescriptionExplanation;
	CNxEdit m_editStrength;
	CNxEdit m_editDaysSupply;
	CNxEdit m_editPriorAuthorization;
	CNxEdit m_editNoteToPharmacist;
	CNxEdit m_editDosageForm;
	CNxEdit m_nxeditStrengthUnits;
	CNxStatic	m_nxstaticMedicationNameLabel;
	CNxStatic	m_nxstaticEnglishExplanationLabel;
	CNxStatic m_nxPatientName, m_nxPatientGender, m_nxPatientAddress, m_nxPatientBirthdate, m_nxPatientPhone;
	CNxIconButton	m_btnClose;
	CDateTimePicker	m_dtPrescriptionDate;
	CDateTimePicker	m_dtReminderDate;
	NxButton m_checkAllowSubstitutions;
	CNxLabel m_nxlabelRefills;
	CNxLabel m_nxlabelPriorAuthLabel;
	// (j.fouts 2012-11-26 15:18) - PLID 51889 - Added a variable for the Note To Pharm label
	CNxStatic m_nxstaticNoteToPharm;
	CNxIconButton m_btnAddDiag;
	CNxStatic m_nxsSampleExpirationLbl;
	// (j.fouts 2012-10-01 17:52) - PLID 52973 - Save, eSubmit, Delete, Hold buttons
	CNxIconButton m_btnSave;
	CNxIconButton m_btnESubmit;
	CNxIconButton m_btnDelete;
	// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
	CNxLabel m_nxlabelDosage;
	// (s.dhole 2012-10-23 ) - PLID 53334
	CNxIconButton m_nxBtnRxError;
	// (j.fouts 2013-01-04 11:25) - PLID 54447 - Added a control to save the default	
	NxButton m_checkSaveAsDefault;
	// (j.fouts 2013-01-04 11:24) - PLID 54448 - Added a control to save to the favorites list
	NxButton m_checkAddToFavorites;
	// (j.fouts 2013-01-10 10:54) - PLID 54526 - Added Dosage quantity list
	NXDATALIST2Lib::_DNxDataListPtr m_pDosageQuantityList;
	// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added dosage route list
	NXDATALIST2Lib::_DNxDataListPtr m_pRouteList;
	// (j.fouts 2013-01-10 10:55) - PLID 54529 - Added dosage frequency list
	NXDATALIST2Lib::_DNxDataListPtr m_pFrequencyList;
	// (j.fouts 2013-01-10 10:52) - PLID 54463 - Refills is now a datalist
	NXDATALIST2Lib::_DNxDataListPtr m_pRefillsList;
	// (r.farnworth 2013-01-25) PLID 54667 - eFax button
	CNxIconButton m_btnCreateTodo, m_btnEFax;
	// (s.dhole 2013-10-19 11:08) - PLID 59084 
	CNxIconButton  m_btnFCovarage;
	// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added a dosage unit list
	NXDATALIST2Lib::_DNxDataListPtr m_pDosageUnitList;
	// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workaround, the checkboxes's text are now labels
	CNxLabel m_nxsAllowSubsText;
	CNxLabel m_nxsSaveToQuickListText;
	CNxLabel m_nxsSaveAsDefaultText;
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;
	// (a.walling 2009-07-01 11:00) - PLID 34052 - Supervisor and Agent support
	NXDATALIST2Lib::_DNxDataListPtr m_AgentCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_SupervisorCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_PharmacyCombo;
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Hyperlink label for the Pharmacy datalist in search list mode.
	CNxLabel m_nxlPharmacySelection;
	NXDATALIST2Lib::_DNxDataListPtr m_pQuantityUnitCombo;
	// (r.farnworth 2016-01-15 12:20) - PLID 67750 - Packaging Info
	NXDATALIST2Lib::_DNxDataListPtr m_pPackagingCombo;

	//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
	NXTIMELib::_DNxTimePtr m_nxtSampleExpirationDate;

	// (b.savon 2013-09-18 14:22) - PLID 58455 - Add a URL link to launch a webbrowser to the IStop web address so that prescribers can access the controlled substance registry
	CLinkCtrl m_linkControlledSubstance;

	//TES 8/22/2013 - PLID 57999 - Formularies
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceCombo;


	// (b.savon 2013-09-24 09:44) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
	CNxLabel m_nxsAddDiagnosisCodes;

	// (j.jones 2013-10-18 09:46) - PLID 58983 - added infobutton abilities
	CNxIconButton m_btnPatientEducation;
	CNxLabel m_nxlabelPatientEducation;

	// (b.savon 2013-09-25 10:53) - PLID 58762 - The prescription returned from the API needs to contain the formulary information
	long GetInsuranceLevelPlacement(const CString &strInsuranceLevel);
	CString GetInsuranceResponsibility(const CString &strInsuranceLevel);
	CString GetInsuranceDisplay(const CString &strPBMID, const CString &strPBMName, const CString &strPlanName, const CString &strPharmacyCoverage);

	//}}AFX_DATA

	//DoModal is protected, such that it cannot be called by outside code,
	//you must call CreateNewPrescription or EditPrescription
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrescriptionEditDlg)
	protected:
	virtual int DoModal();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//// (a.wilson 2013-02-01 12:21) - PLID 53799 - All Variables concerning new Renewal Request Functionality.
	//---------------------------------------------------------------------------------------------------------
	NexTech_Accessor::_ERxRenewalRequestInfoPtr m_pCurrentRenewalRequest;	//pointer containing all renewal data.
	CNxIconButton m_btnViewOriginalPrescription;	//button for viewing the original NewRx.
	CNxIconButton m_btnApprove, m_btnApproveWithChanges, m_btnDeny, m_btnDenyAndRewrite;	//buttons for responding to the renewal request.
	CNxStatic m_nxDispensedMedicationNameLabel;	//text displaying the dispenssed medication name.
	CNxStatic m_nxRenewalWarning;	//text displaying whether the renewal is a: narcotic, supply, or compound drug.
	CNxStatic m_nxUserAuthorizationWarning;	// (a.wilson 2013-05-01 09:29) - PLID 56509 - displays a message to the user if they aren't authorized.
	CNxLabel m_nxMedicationLabel, m_nxDispensedMedicationLabel;	//label for dispensed/prescribed medication and hyperlink.
	CNxLabel m_nxAssignPatientLink;	//hyperlink to assign patient when auto-match failed.
	CNxEdit m_editResponseReason;	//this edit control will contain the response reason post response.
	bool m_bShowDispensedMedication;	//flag for displaying either the dispensed or prescribed medication info.
	bool m_bInitialRenewalDisplayComplete;	//flag for determining whether we already ran through the initial display settings for renewals.
	bool m_bUserAuthorizedPrescriber;	// (a.wilson 2013-05-01 09:29) - PLID 56509 - flag to determine whether the user is authorized for this renewal.
	//---------------------------------------------------------------------------------------------------------
	//These fields are for New Prescriptions tied to DenyNewRx Responses.
	long m_nDenyNewRxResponseID;	//id of the response that the prescription was created for.
	//---------------------------------------------------------------------------------------------------------
	
	// (r.gonet 2016-02-23 00:53) - PLID 68408 - Store the type of record we are displaying. Renewal or Prescription.
	// You could get this by looking at the current prescription object and the renewal request object, but this is more purposeful.
	enum class EDisplayedRecordType {
		Prescription = 0,
		Renewal,
	};
	EDisplayedRecordType m_eDisplayedRecordType = EDisplayedRecordType::Prescription;

	// (b.savon 2013-03-18 16:05) - PLID 55477
	Nx::SafeArray<IUnknown*> m_saryPrescribers;
	Nx::SafeArray<IUnknown*> m_sarySupervisors;
	Nx::SafeArray<IUnknown*> m_saryNurseStaff;
	NexTech_Accessor::ERxUserRole m_erxUserRole;

	// (j.fouts 2012-10-10 15:17) - PLID 52973 - The currently opened prescription
	NexTech_Accessor::_QueuePrescriptionPtr m_pCurrentPrescription;
	bool m_bUnsavedChanges;
	// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
	long m_nFDBID;
	//determines whether the dialog should be read-only
	// (j.fouts 2013-03-12 10:06) - PLID 52907 - Renamed from m_bOpenReadOnly
	BOOL m_bReadOnly;
	// (r.farnworth 2016-01-08 15:36) - PLID 58692
	BOOL m_bRePrescribe;

	// (j.jones 2013-11-25 09:15) - PLID 59772 - added the drug interaction struct, if it is filled then
	// the Rx dialog needs to show drug interactions immediately upon opening
	DrugInteractionInfo m_drugInteractionsToShowOnInit;

	// (j.fouts 2013-01-10 10:52) - PLID 54463 - Track when we should auto generate a sig
	bool m_bDontGenerateSig;

	// (a.walling 2009-11-18 16:30) - PLID 36352 - Verify that a date change is allowed
	bool VerifyPrescriptionDate(bool bMessageBox, OPTIONAL OUT CString* pstrMessage = NULL);
	// (j.fouts 2013-04-16 15:30) - PLID 53146 - The previous pharmacy that was selected
	long m_nPrevPharmacyID;
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Flag indicating if the pharmacy search list is searching.
	// If it is, then the dropdown closing up is expected and not due to the user cancelling it.
	bool m_bIsPharmacyDataListSearching = false;

	long m_nPatientID;
	// (j.jones 2012-10-31 15:05) - PLID 52819 - Added m_nCurrentlyOpenedEMNID, the ID of the EMN currently being edited.
	// This EMNID may or may not necessarily be the EMNID associated with the prescription we are editing.
	long m_nCurrentlyOpenedEMNID;
	
	// (j.jones 2012-11-15 09:54) - PLID 52819 - Added source action info, only used when spawning a new medication.
	SourceActionInfo m_saiSpawnedFromAction;

	// (a.walling 2009-04-15 13:34) - PLID 33897 - request Refill quantity
	CString m_strPatientExplanation;

	//used for handling inactive selections
	long m_nPendingLocationID;
	long m_nPendingPharmacyID;

	//TES 2/12/2009 - PLID 33002 - Changed the Unit to a dropdown
	CString m_strPendingUnit;

	_variant_t m_varOrderSetID; // (z.manning 2009-05-13 14:16) - PLID 28554

	HBITMAP m_imgPrescriptionBack;

	// (j.fouts 2012-11-07 16:23) - PLID - Has the opened prescription had changes made to it
	bool m_bChangesMade;

private:
	boost::scoped_ptr<AutoPopulatingSig> m_pAutoPopulatingSig;

protected:
	// (a.walling 2009-04-06 16:44) - PLID 33870 - Toggle interface for 'Refill As Needed'
	void ToggleRefillAsNeeded(BOOL bRefillAsNeeded);
	// (a.wilson 2013-04-08 17:49) - PLID 56141 - enforce enabling and disabling the approve button for renewals when the refill value changes.
	void EnforceRenewalRefillApproval();

	//this function will fill the 'first prescription date' field, based on m_nMedicationID
	void FillFirstPrescriptionDate();

	//save the prescription
	// (j.fouts 2012-10-17 14:11) - PLID 53146 - Added a PrescriptionCommit pointer, and reworked this to use the API
	BOOL Save(NexTech_Accessor::_PrescriptionCommitPtr pPrescription);

	// (j.fouts 2012-09-26 10:11) - PLID 52973 - Internal Requery function Requeries the queue
	//void RequeryQueue(Nx::SafeArray<IUnknown *> saryPrescriptions);

	//hides the english description and makes the regular description wider
	void HideEnglishDescriptionControls();

	//hides the todo controls, they are only shown on new prescriptions
	void HideToDoReminderControls(BOOL bShow = FALSE);

	//disables all the controls on the screen, execpt for the Close button
	void DisableControls(BOOL bEnabled = FALSE);

	// (j.fouts 2013-05-01 09:31) - PLID 52907
	//Disables all controls that are representative of a prescription,
	//This does not inculde the Send, Print, Fax, Delete, Close buttons
	void DisablePrescriptionControls(BOOL bEnable = FALSE);

	// (j.jones 2008-05-20 11:41) - PLID 30079 - used to hide controls based on EMR usage
	void DisplayControlsForEMR(BOOL bShow = TRUE);
	BOOL m_bIsEMRTemplate;

	// (j.jones 2008-05-20 15:35) - PLID 30079 - this function will create a to-do alarm if the controls say so
	void CreateTodoAlarm(COleDateTime dtReminderDate);

	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Removed dead code involving toggling on and off favorite pharmacies.
	// (j.fouts 2012-10-02 08:52) - PLID 52973 - Opens an existing prescription, selects it if bSelect, 
	//		and requeries the queue if bRequery, bCheckQty optionaly show a correction dlg if qty is invalid
	// (j.fouts 2013-03-12 10:07) - PLID 52973 - Renamed, because it now just fills fields with the values in m_pPrescription
	void LoadPrescriptionFields(bool bCheckQty = false);
	// (j.fouts 2012-10-02 08:48) - PLID 52906 - Calls the API to delete a prescription
	void DeleteSelectedPrescription();
	// (j.fouts 2012-10-02 08:46) - PLID 52907 - Shows the appropriate fields for the selected prescription
	//Renamed this because it will not enable or show anything
	void EnforceDisabledControls();
	// (j.fouts 2012-10-02 08:47) - PLID 52907 - Clears all the prescription realated controls
	void ClearAllFields();
	// (j.fouts 2012-10-17 14:11) - PLID 53146 - Commits PriorAuthorizationIsSample to data
	void CommitPriorAuthIsSample(bool bValue);
	// (j.fouts 2012-10-17 14:12) - PLID 53146 - Commits RefillsAsNeeded to data
	void CommitRefillAsNeeded(bool bValue);
	// (j.fouts 2013-01-04 11:24) - PLID 54448 - Saves the drug to the pick list if the box is checked
	void SaveToPickList();
	// (j.fouts 2013-01-04 11:25) - PLID 54447 - Saves the current fields as the default for the drug if the box is checked
	void SaveAsDefault();

	// (j.jones 2012-11-16 08:55) - PLID 53765 - track what it was we did in this dialog
	PrescriptionDialogReturnValue m_epdrvAction;

	// (j.jones 2016-01-06 15:33) - PLID 67823 - track if the prescription is new - in most cases it is not
	bool m_bIsNewRx;

	// (j.jones 2012-11-19 09:45) - PLID 52819 - used for the purpose of creating a prescription
	// but hiding the dialog (creating the prescription silently)
	bool m_bHideDialog;

	// (j.fouts 2013-01-10 10:52) - PLID 54463 - Updates the sig to the auto generated sig
	// (j.fouts 2013-02-01 15:05) - PLID 54985 - Updated to fill a prescription commit ptr if sig/qty change
	void UpdateSig(NexTech_Accessor::_PrescriptionCommitPtr pPrescription);

	// (b.savon 2013-01-15 12:03) - PLID 54632
	EUserRoleTypes m_urtCurrentUserRole;
	void LoadPrescriberCombo();
	void LoadSupervisorCombo();
	void LoadNurseStaffCombo();
	void GetNurseStaffPrescriberIDs(CString strIDsIn, CArray<long, long> &arySupervising, CArray<long, long> &aryMidlevel);
	void GetNurseStaffPrescriberNames(CString strNamesIn, CArray<CString, LPCTSTR> &arySupervising, CArray<CString, LPCTSTR> &aryMidlevel);
	void GetNurseStaffIdentifiers(CString strIdentifiersIn, CString &strSupervising, CString &strMidlevel, ENurseStaffIdentifier nsiIdentifier);
	void SavePrescriber(long nProviderID);
	void SaveSupervisor(long nSupervisorID);
	void SaveNurseStaff(long nNurseStaffID);
	void LoadPrescriptionPersonsLegacy();
	void SavePharmacy(long nPharmacyID);

	void PrepareInterfaceForSupervisorSelection(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bKnownMidlevel = FALSE);

	// (b.savon 2013-02-05 11:23) - PLID 51705
	BOOL IsLicensedConfiguredUser();
	CNexERxLicense m_lNexERxLicense;
	BOOL m_bCachedIsLicensedConfiguredUser;

	// (j.fouts 2013-01-21 14:12) - PLID 54463 - Only drop down the list if it is not yet filled
	void DropDownNextEmptyList(long nCtrlID);

	////Disable/Hiding Controls
	// (j.fouts 2013-03-12 10:08) - PLID 52907 - Seperated some of the disabling/hiding logic into reusable functions
	//Disables/Hides all controls that should not be edited on a EMRTemplatePresrciption
	void HideEMRTemplateControls();
	//Disables the Print button
	void DisablePrint();
	//Disables the EFax button
	void DisableEFax();
	//Disables the ESubmit button
	void DisableESubmit();
	//Hides the dosage label
	void HideDosage();
	//Hides all controls associated only with NexERx
	void HideNexERxControls();
	//Hides all controls that should only be visable on a renewal request
	void HideRenewalControls();
	//Enables pending controls (controls that may have been enabled due to a change in the prescription)
	// and then enforces disabled controls to hide them as needed
	void TryEnablePendingControls();

	BOOL CPrescriptionEditDlg::IsSubmitButtonEnabled();
	int  CPrescriptionEditDlg::IsErrorEnabled();

	void OpenRenewalRequest();
	CString FormatAddressForPrescription(const CString & Address1, const CString & Address2, const CString & City, const CString & State, const CString & Zip);
	void DisplayCurrentRenewalMedication();
	long CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus queueStatus, NexTech_Accessor::RenewalResponseStatus responseStatus, 
		const CString &strRefills, const CString & strDetailedReason, const CArray<_bstr_t> &arReasons = CArray<bstr_t>());
	void UpdateDialogRenewalStatus(NexTech_Accessor::PrescriptionStatus queueStatus, NexTech_Accessor::RenewalResponseStatus responseStatus);
	bool ImportPharmacyFromRenewal();
	void UpdateRenewalRequestInfo();
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Gets the selected pharmacy ID.
	long GetSelectedPharmacyID();

	//(r.wilson 3/6/2013) pl 55478
	void SaveBatchQuantity(NexTech_Accessor::_PrescriptionCommitPtr pCommit);
	void SaveBatchPatientExplanation(NexTech_Accessor::_PrescriptionCommitPtr pCommit);
	void SaveBatchPharmacistNote(NexTech_Accessor::_PrescriptionCommitPtr pCommit);
	void SaveBatchPriorAuthorization(NexTech_Accessor::_PrescriptionCommitPtr pCommit);

	//TES 8/27/2013 - PLID 58287 - Update the description to use the medication identified by nFDBID
	// (s.dhole 2013-10-19 11:50) - PLID 59068 Added InsuranceID
	void ChangeMedication(long nFDBID, const CString &strMedName, long nFInsuranceID );
	//TES 8/22/2013 - PLID 57999 - Based on the selected insurance and our medication, fill in the Formulary area of the dialog
	void LoadFormularyResults();
	//TES 8/28/2013 - PLID 57999 - Need this as a member variable so we can color the status text
	long m_nFormularyStatus;
	//TES 9/25/2013 - PLID 57999 - Reloads just the formulary section from data
	void ReloadFormularyInformation();
	// (s.dhole 2013-10-19 13:45) - PLID 59068
	long m_SelectedFInsuranceID;	
	HICON m_hStarIcon;
	// (r.gonet 2016-02-24 16:59) - PLID 67988 - Cache the license check.
	bool m_bHasEPrescribingLicense;

	// (r.gonet 2016-02-24 16:59) - PLID 68408 - The Pharmacy list can be either a search list or dropdown. Gets what it should be.
	NXDATALIST2Lib::EViewType GetPharmacyListViewType();
	// (r.gonet 2016-02-24 16:59) - PLID 68408 - Returns true if the pharmacy list is a search list.
	bool IsPharmacyListASearchList();
	// (r.gonet 2016-02-24 16:59) - PLID 68408 - Returns true if the pharmacy list is a dropdown.
	bool IsPharmacyListADropDown();
	// (r.gonet 2016-02-23 00:14) - PLID 67988
	NXDATALIST2Lib::IRowSettingsPtr GetDefaultPharmacySearchListSelection() const;

	void UpdateDaysSupply(NexTech_Accessor::_PrescriptionCommitPtr pCommit);
	// (j.fouts 2013-04-09 08:56) - PLID 52907 - Sets a text ctrl's tooltip to the text if it exceeds the bounds of the ctrl
	void SetTextAndToolTip(CNxStatic *pTextControl, CString &bstrMessage);
	// (j.fouts 2013-04-26 09:29) - PLID 53146 - Warn about interactions
	bool PromptInteractions();

	// (j.jones 2013-11-25 13:45) - PLID 59772 - added a drug interaction pointer
	scoped_ptr<class CDrugInteractionDlg> m_pDrugInteractionDlg;

	// (j.jones 2008-10-09 14:50) - PLID 17235 - added OnSelChosenMedPharmacyCombo and OnRequeryFinishedMedPharmacyCombo
	// Generated message map functions
	//{{AFX_MSG(CPrescriptionEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTrySetSelFinishedMedLocationCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedMedPharmacyCombo(long nRowEnum, long nFlags);
	afx_msg void OnBtnPreviewPrescription();
	afx_msg void OnChangePrescriptionDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpPrescriptionDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenMedPharmacyCombo(LPDISPATCH lpRow);
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Reflects a pharmacy selection by showing either the
	// pharmacy search list or hyperlink. In renewals mode, it just makes the selection from the 
	// datalist.
	void UpdateControlsForPharmacySelection(long nSelectedPharmacyID);
	afx_msg void OnRequeryFinishedMedPharmacyCombo(short nFlags);
	afx_msg void OnSelChosenPackagingInfo(LPDISPATCH lpRow);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	// (j.jones 2012-11-15 15:38) - PLID 53765 - added OnOk and OnCancel
	virtual void OnOK();
	virtual void OnCancel();
	void OnTrySetSelFinishedQuantityUnit(long nRowEnum, long nFlags);
	afx_msg void OnBnClickedCheckEprescribe();
	afx_msg void OnBnClickedDeletePrescription();
	afx_msg void OnBnClickedExpandFirst();
	afx_msg void OnBnClickedExpandSecond();
	void SelChosenQueueStatusFilter(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusNoteToPharmacist();
	afx_msg void OnEnKillfocusPriorAuth();
	afx_msg void OnEnKillfocusDaysSupply();
	afx_msg void OnEnKillfocusEditQuantity();
	afx_msg void OnBnClickedAllowSubstitutions();
	void SelChosenMedLocationCombo(LPDISPATCH lpRow);
	void SelChangingMedPharmacyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenQuantityUnit(LPDISPATCH lpRow);
	void SelChosenPrescriptionDosage(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusPrescriptionExplanation();
	void KillFocusSampleExpirationDate();
	afx_msg void OnBnClickedBtnErxError();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedApproveRenewal();
	afx_msg void OnBnClickedApproveWithChangesRenewal();
	afx_msg void OnBnClickedDenyRenewal();
	afx_msg void OnBnClickedEsubmitButton();
	afx_msg void OnBnClickedOriginalPrescriptionBtn();
	void SelChosenPrescriptionQuantity(LPDISPATCH lpRow);
	void SelChosenPrescriptionMethod(LPDISPATCH lpRow);
	void SelChosenPrescriptionFrequency(LPDISPATCH lpRow);
	void SelChosenPrescriptionQuantityUnit(LPDISPATCH lpRow);
	afx_msg void OnBnClickedDenyAndRewriteRenewal();
	BOOL ConfigureSubmitBtns();
	void SelChosenMedProviderCombo(LPDISPATCH lpRow);
	void SelChosenMedSupervisorCombo(LPDISPATCH lpRow);
	void SelChosenMedPrescriberAgentCombo(LPDISPATCH lpRow);
	void SelChosenPrescriptionRefills(LPDISPATCH lpRow);
	void SelChosenPrescriptionDosageUnit(LPDISPATCH lpRow);
	afx_msg void OnBnClickedBtnTodoPresedit();
	afx_msg void OnBnClickedFaxButton();
	void RevertSelChangingOnNull(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnNMClickSyslinkCsUrl(NMHDR *pNMHDR, LRESULT *pResult);
	void SelChosenFormularyInsList(LPDISPATCH lpRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void RequeryFinishedFormularyInsList(short nFlags);
	afx_msg void OnBnClickedBtnViewCoverage();
	// (j.jones 2013-10-18 09:46) - PLID 58983 - added infobutton abilities
	afx_msg void OnBtnPtEducation();
	// (j.jones 2013-11-25 09:15) - PLID 59772 - added ability to show drug interactions immediately upon opening the dialog
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnPostShowWindow(WPARAM wParam, LPARAM lParam);	
public:
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Added handler for when the search results list closes up.
	void ClosedUpExMedPharmacyCombo(LPDISPATCH lpSel, short dlClosedUpReason);
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Added handler for when the search results are about to start populating..
	void RequestListDataStartingMedPharmacyCombo();
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Added handler for when the search results have finished populating.
	void RequestListDataFinishedMedPharmacyCombo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESCRIPTIONEDITDLG_H__54C82712_172B_417B_9077_F18D940F7D92__INCLUDED_)
