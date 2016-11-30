#pragma once

#include "PatientsRc.h"

#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <NxSystemUtilitiesLib/NxThread.h>
#include <NxUILib/NxStaticIcon.h>
#include "NxAPI.h"
#include "NexERxSetupDlg.h"
#include "NexERxLicense.h"
#include "PrescriptionUtilsAPI.h"
#include "MultiSelectComboController.h"

class CProgressDialog;

#define PRESCRIPTION_CHANGED	1234

// CPrescriptionQueueDlg dialog

// (j.fouts 2012-11-01 15:10) - PLID 53566 - Created

// (b.savon 2013-01-29 18:19) - PLID 54919
enum EMedicationSearch{
	msWriteRx = 0,
	msCurrentMeds,
};

// (b.savon 2013-02-12 14:31) - PLID 54782
// (b.savon 2013-08-23 15:11) - PLID 58236 - rename
enum ENexFormularySource{
	nfsQuickList = 0,
	nfsWriteRx,
};

// (j.fouts 2012-11-06 12:05) - PLID 53574 - A box that contains controls, can be collapsed and expanded.
class CCollapseableBox
{
public:
	//Location of a control within the box
	enum ControlLocation { clTop, clCenter, clBottom, };

	CCollapseableBox(CWnd* pParent, bool bCollapsed = false);

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Bind all the controls
	//Binds the relative positions of the controls to the box
	void BindControls(CNxColor* pColorBkg, CNxStatic* pLabel = NULL, CNxIconButton* pIconButton = NULL);
	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Controls positions are now relative to the size of the collapsable box
	//Add an additional control to the box
	void AddHeaderControl(int nCtrl);
	void AddBodyControl(int nCtrl);
	void AddFooterControl(int nCtrl);

	//This this box collapsed
	bool IsCollapsed();

	//Repositions all the controls based on the size of the box (rectSize), and if the box is collapsed
	void RepositionControls(CRect &rectSize);
	//Collapse/Expand the box
	void ToggleCollapsed();
	// (j.jones 2016-02-03 16:43) - PLID 68118 - added ForceCollapsed, ForceOpen
	void ForceCollapsed();
	void ForceOpen();
protected:
	//Window containing the box
	CWnd* m_pParent;

	//Current size of the box
	CRect m_rectSize;
	//Is this collapsed
	bool m_bCollapsed;
	
	//Array of the controls
	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Array of all the controls associated with the box
	CArray<int,int> m_aryBodyControls;
	CArray<int,int> m_aryHeaderControls;
	CArray<int,int> m_aryFooterControls;
	//Maps a control to its relative position for resizing
	CMap<int,int,CRect,CRect> m_aryRelativePositions;

	//Optional Label shown when expanded and collapsed
	CNxStatic* m_pLabel;
	//The color, used as the background of the box
	CNxColor* m_pColorBkg;

	//Optional button for expanding and collapsing
	CNxIconButton* m_pIconButton;

	long m_nHeaderHeight;
	long m_nFooterHeight;

	
};
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:46) - PLID 67965
enum ReminderOptions {
	Hour = 0,
	Minute,
	Login,
	Never
};

class CPrescriptionQueueDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPrescriptionQueueDlg)

protected:
	// (r.gonet 2016-01-22) - PLID 67967 - Controller class for the combo/hyperlink Rx prescriber filter.
	class CRxPrescriberFilterController : public CMultiSelectComboController {
	public:
		// (r.gonet 2016-01-22) - PLID 67967 - Creates a new CRxPrescriberFilterController object.
		CRxPrescriberFilterController(CWnd *pParent);
	protected:
		// (r.gonet 2016-01-22) - PLID 67967 - Loads the Rx prescriber combo with the prescriber rows.
		virtual void LoadComboRows();
	};

	// (r.gonet 2016-01-22) - PLID 67973 - Controller class for the combo/hyperlink Renewal prescriber filter.
	class CRenewalPrescriberFilterController : public CMultiSelectComboController {
	public:
		// (r.gonet 2016-01-22) - PLID 67973 - Creates a new CRenewalPrescriberFilterController object.
		CRenewalPrescriberFilterController(CWnd *pParent);
	protected:
		// (r.gonet 2016-01-22) - PLID 67973 - Loads the Renewal prescriber combo with the prescriber rows.
		virtual void LoadComboRows();
	};
public:
	// (j.fouts 2012-11-15 10:35) - PLID 53573 - Added bEmbeded
	// (j.jones 2012-11-20 10:16) - PLID 52818 - added optional default provider, location, and date,
	// such that new prescriptions defaulted to these values
	// (j.fouts 2013-01-16 09:42) - PLID 51712 - Added an option to open to renewal dlg
	// (j.jones 2013-03-26 15:30) - PLID 53532 - renamed nEMNID to nCurrentlyOpenedEMNID to reflect what
	// the variable really means, it's the ID of the currently opened EMN, such that creating new prescriptions
	// would use this EMN ID
	CPrescriptionQueueDlg(CWnd* pParent = NULL, 
		long nPatientID = -1, 
		long nCurrentlyOpenedEMNID = -1,
		bool bShowMedsAllergies = true, 
		bool bInMedicationsTab = false,
		long nProviderID = -1, 
		long nLocationID = GetCurrentLocationID(), 
		COleDateTime dtDate = COleDateTime::GetCurrentTime(),
		bool bOpenToRenewals = false);
	virtual ~CPrescriptionQueueDlg();
	// (b.savon 2013-01-25 14:51) - PLID 54854	
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Dialog Data
	enum { IDD = IDD_PRESCRIPTION_QUEUE };

public:
	// (j.fouts 2012-11-01 15:11) - PLID 53566 - Moved these methods from PrescriptionEditDlg.h

	// (j.jones 2012-11-19 11:03) - PLID 53085 - added bShowInteractionChecks, such that if
	// the requery is only due to changing a filter, we don't always have to show the interactions
	void RequeryQueue(bool bShowInteractionChecks = true);

	// (j.jones 2013-04-05 13:38) - PLID 56114 - Added function to reload allergies & meds, will do nothing
	// if the list is hidden due to being embedded in the Medications tab.
	void RequeryAllergiesAndMeds(bool bRequeryAllergies = true, bool bRequeryMeds = true);

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Handle patient change
	void ChangePatient(long nPatientID, bool bForceRefresh = false);
	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Allow the caller to set the color
	void SetColor(OLE_COLOR nNewColor);
	// (j.fouts 2012-11-14 11:42) - PLID 53743 - Moved print into the queue
	void UpdatePrintPreviewText();
	void UpdateShowState();
	// (j.fouts 2013-02-06 09:41) - PLID 54472 - Update the filters to show what needs attention for the current user
	// (j.jones 2016-02-03 16:12) - PLID 68118 - renamed to simply respond to which notification link the user clicked
	// if bShowRenewals is false, show the Rx Queue, else renewals
	void LoadFromNotificationClick(bool bShowRenewals);
	// (a.wilson 2012-11-02 14:35) - PLID - function to generate renewal list.
	// (b.savon 2013-04-22 16:17) - PLID 54472 - Override From filter
	void RequeryRenewalRequests(const COleDateTime &dtFromOverride = g_cdtNull);
	// (j.fouts 2013-01-03 14:46) - PLID 54429 - Update the text on the interactions button
	void UpdateInteractionsButton(long nInteractionCount);
	// (j.fouts 2013-04-22 14:13) - PLID 54719 - Update the hidden/shown columns
	virtual void UpdateView(bool bForceRefresh = true);

protected:

	// (j.jones 2013-04-05 14:18) - PLID 56114 - added tablechecker variables
	// (r.gonet 09/20/2013) - PLID 58396 - And now a medication history response checker
	CTableChecker m_CurrentMedsChecker, m_PatientAllergiesChecker, m_MedicationHistoryResponseChecker;

	// (r.gonet 2016-01-22) - PLID 67967 - Multi combo box controller for the Rx Prescriber filter dropdown.
	CRxPrescriberFilterController m_rxPrescriberController;
	// (r.gonet 2016-01-22) - PLID 67973 - Multi combo box controller for the Renewal Prescriber filter dropdown.
	CRenewalPrescriberFilterController m_renewalPrescriberController;

	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	bool m_bHasSureScriptsLicense;

	// (j.fouts 2012-11-01 15:11) - PLID 53566 - Moved these methods from PrescriptionEditDlg.h
	// (s.dhole 2012-10-26 ) - PLID 53421
	void LoadSureScriptError(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void OpenRenewalRequest();
	// (j.fouts 2012-12-03 14:09) - PLID 53743 - Add bCheckPrint
	// (j.fouts 2012-12-03 14:10) - PLID 53745 - Add bCheckSend
	// (j.fouts 2013-01-14 10:54) - PLID 54464 - Added Pharmacies combo source
	void InsertPrescriptionIntoQueue(NexTech_Accessor::_QueuePrescriptionPtr pPrescription, bool bCheckSend = false, bool bCheckPrint = false);
	
	// (j.jones 2016-01-22 14:14) - PLID 63732 - creates a progress bar for use prior to loading the data from the API,
	// ends after the screen has finished updating
	void BeginProgressDialog(CProgressDialog &progressDialog);

	// (j.fouts 2013-01-14 10:54) - PLID 54464 - Added Pharmacies
	// (j.jones 2016-01-22 14:18) - PLID 63732 - now takes in a progress dialog
	void RefreshQueueFromArray(Nx::SafeArray<IUnknown *> saryPrescriptions, Nx::SafeArray<IUnknown *> saryPharmacies, CProgressDialog &progressDialog);

	NexTech_Accessor::_QueuePrescriptionPtr LoadPrescription(NexTech_Accessor::_QueuePrescriptionPtr pPrescription);
	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Hides and disables the Current Meds/Allergies controls
	void HideDisableMedsAllergies();
	// (j.fouts 2012-11-14 11:42) - PLID 53744 - Move merge templates to the queue
	void RequeryWordTemplateList();
	void EnforceControlSize();
	// (j.fouts 2012-11-20 14:23) - PLID 53840 - Check permissions and disable controls
	void SecureControls();
	// (j.fouts 2012-11-20 14:23) - PLID 53840 - Disable/Hide all controls that require SureScripts
	void HideSureScriptsControls(bool bHideSendButton = true);
	// (j.fouts 2012-11-20 14:23) - PLID 53840 - Disables/Hides all controls that use FDB including SureScripts controls
	// (j.fouts 2013-05-30 10:37) - PLID 56807 - Drug Interactions is tied to NexERx now so this has no purpose any longer
	//void HideFirstDataBankControls();
	// (j.fouts 2012-11-20 14:23) - PLID 53840 - Hide controls and set hide flags for controls that they don't have a license for
	void UpdateControlsForLicense();

	void RefreshSingleQueueRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (s.Dhole 2012-10-29 ) - PLID 54890 - Warning ICON
	HANDLE  m_hIconHasSureScriptError;

	// (b.savon 2012-11-28 16:51) - PLID 51705 - Based on the statusId, tells if the prescription status is a surescripts response. Return formatted message
	BOOL IsSureScriptsResponse(long nStatusID);
	CString GetFormattedSureScriptsResponseMessage(const CString& strMessageID, const CString& strMessage, const COleDateTime& dtSentTime, const COleDateTime& dtSentResponseTime);

	// (j.fouts 2012-11-30 10:09) - PLID 53954 - Moved code to disable send/preview buttons into a function
	void UpdateSendButton();
	void UpdatePreviewButton();
	
	// (j.fouts 2013-01-03 12:27) - PLID 54429 - Tell the interactions dlg to check for interactions
	void CheckInteractions();
	// (j.fouts 2013-01-04 13:31) - PLID 54456 - Moved opening the Prescription Edit dlg into a function as not to duplicate code
	// (s.dhole 2013-10-18 13:02) - PLID 
	void WritePrescription(long nMedicationID, long nPatientID = -1 , long nFInsuranceID = -1);
	// (r.farnworth 2016-01-06 14:31) - PLID 58692
	PrescriptionInfo CopyPrescription(long nPrescriptionID, long nMedicationID, long nPatientID = -1, long nFInsuranceID = -1, bool bIsTemplate = false);
	// (j.fouts 2013-01-25 13:36) - PLID 53574 - Save/audit allergies reviewed and update the controls
	void UpdateAllergyReviewStatus(bool bReviewedAllergies);
	// (j.fouts 2013-01-25 13:37) - PLID 53574 - Update the allergies reviewed controls
	void UpdateAllergyReviewCtrls(bool bReviewedAllergies, COleDateTime& dtReviewedOn, CString& strUserName);
	// (j.fouts 2013-01-28 16:34) - PLID 53025 - Create a function to set the status and update the queue with the results
	void SetPrescriptionStatus(CArray<CString,CString&> &aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus psStatus);
	// (j.fouts 2013-02-04 10:01) - PLID 53954 - Moved this into a utility function
	Nx::SafeArray<IUnknown *> GetQueueFilters();
	void RefreshInteractionsFromArray(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
		Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
		Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts
		);

	// (j.fouts 2013-04-25 12:45) - PLID 53146 - Generate a combo source string for a set of pharmacies ignoring another set of pharmacies
	CString GeneratePharmacySourceString(Nx::SafeArray<IUnknown*> saryPharmacies, std::vector<int>* aryIgnoreList = NULL);
	// (j.fouts 2013-04-25 12:45) - PLID 53146 - Generate the full combo source string for pharmacies
	CString GeneratePharmacySourceStringFull(Nx::SafeArray<IUnknown*> saryFavoritePharmacies);
	// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
	// (j.fouts 2013-09-17 16:53) - PLID 58496 - Handle the case when we are in needing attention, also added paramaters
	bool PromptInteractions(long nPatientID, const CString &strPatientName = "");

	// (b.savon 2013-05-21 09:00) - PLID 56795
	void ShowNewCropPrescriptionButton();
	
protected:
	// (j.fouts 2012-11-01 15:11) - PLID 53566 - Moved these members from PrescriptionEditDlg.h
	///Controls
	// (j.fouts 2012-10-02 10:36) - PLID 52973 - NxColors for the queue and hold datalists
	// (a.wilson 2012-10-24 11:13) - PLID 51711 - add renewals.
	NxThread m_UpdateRenewalsThread;
	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Add meds and allergies
	CNxColor m_queueBkg, m_renewalBkg, m_medsAllergiesBkg;
	// (j.fouts 2012-09-26 10:11) - PLID 52973 - The Queue's Datalist
	// (a.wilson 2012-10-24 11:14) - PLID 51711 - renewal list.
	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Add meds and allergies
	NXDATALIST2Lib::_DNxDataListPtr m_pPresQueueList, m_pRenewalList, m_pMedsList, m_pAllergiesList, 
		m_pRenewalFilterPrescriberList, m_pRenewalFilterResponseList, m_pRenewalFilterTransmitList;
	CDateTimePicker m_dtpRenewalFilterFrom, m_dtpRenewalFilterTo;
	bool m_bRenewalFilterTimeChanged;
	// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed the Default Filtered Prescriber checkbox.
	// (j.fouts 2012-10-03 16:37) - PLID 53009 - QueueStatus Filter List
	NXDATALIST2Lib::_DNxDataListPtr m_pQueueStatusFilter;
	// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates to the queue
	NXDATALISTLib::_DNxDataListPtr   m_pTemplateList;
	// (a.wilson 2012-10-24 10:44) - PLID 51711 - changed to incorporate renewals
	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Add meds and allergies
	CNxIconButton m_btnExpandRenewals;
	CNxStatic m_nxstaticPendingText, m_nxstaticRenewalText, m_nxstaticMedsAllergiesText;
	NxButton m_checkSendToPrinter;
	// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved configuring templates into the queue
	CNxIconButton m_btnConfigureTemplates;
	CNxIconButton m_btnEditTemplate;
	// (j.fouts 2012-11-14 11:42) - PLID 53573 - Moved interactions into the queue
	CNxIconButton m_btnShowInteractions;
	// (j.fouts 2012-11-14 11:42) - PLID 53745 - Added an icon
	CNxIconButton m_btnESubmit;
	// (j.fouts 2012-11-14 11:42) - PLID 53743 - Added an icon
	CNxIconButton m_btnPreview;
	// (b.savon 2013-07-16 12:42) - PLID 57377 - Added a button
	CNxIconButton m_btnNexFormulary;
	HICON m_hIconNexFormulary;
	// (j.jones 2012-11-16 16:03) - PLID 53085 - added EMN filter
	NxButton	m_radioFilterOnEMNMeds;
	NxButton	m_radioFilterOnPatientMeds;
	// (j.fouts 2013-01-04 13:31) - PLID 54456 - Added a new button to write from the pick list
	CNxIconButton m_btnWritePickList;
	// (b.savon 2013-01-07 09:23) - PLID 54461
	CNxIconButton	m_btnEditFavoritePharmacies;
	// (b.savon 2013-01-10 16:06) - PLID 54567
	CNxIconButton	m_btnRxNeedingAttention;
	// (b.savon 2013-01-18 09:57) - PLID 54678
	CNxIconButton m_btnWriteMoreMeds;
	// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
	NxButton		m_checkHideDiscontinuedAllergies;
	NxButton		m_checkHideDiscontinuedMedications;
	// (j.jones 2016-01-21 16:59) - PLID 67993 - added option to include free text meds in the prescription search
	NxButton		m_checkIncludeFreeTextRx;
	// (j.jones 2016-01-21 17:06) - PLID 67996 - added option to include free text meds in the current meds search
	NxButton		m_checkIncludeFreeTextCurMeds;
	// (j.jones 2016-01-21 17:06) - PLID 67997 - added option to include free text allergies in the allergy search
	NxButton		m_checkIncludeFreeTextAllergies;
	// (b.eyers 2016-01-21) - PLID 67966
	CDateTimePicker	m_DateFrom;
	CDateTimePicker	m_DateTo;
	NxButton	m_chkUseDateFilter;
	// (r.gonet 2016-01-22) - PLID 67967 - Added a hyperlink label for the < Multiple > selection in the Rx prescriber filter.
	CNxLabel m_nxlMultiRxPrescribers;
	// (r.gonet 2016-01-22) - PLID 67973 - Added a hyperlink label for the < Multiple > selection in the Renewal prescriber filter.
	CNxLabel m_nxlMultiRenewalPrescribers;
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
	CNxStaticIcon m_icoAboutRxMedicationColors;
	CNxStaticIcon m_icoAboutCurrentMedicationColors;
	CNxStaticIcon m_icoAboutCurrentAllergiesColors;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlWritePrescriptionResults;
	
	void OnClickWritePrescriptionFromSearchResults(NXDATALIST2Lib::IRowSettingsPtr pRow);
	
		
	// (b.savon 2013-01-18 14:41) - PLID 54691
	CNxIconButton m_btnEditMeds;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlCurrentMedicationResults;
	
	void AddMedToCurrentMedicationsList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (b.savon 2013-01-21 09:38) - PLID 54704
	CNxEdit m_editCurrentAllergy;
	CNxIconButton m_btnEditAllergies;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlCurrentAllergyResults;
	void AddAllergyToCurrentAllergyList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void SetInteractionCount(long nInteractionCount);
	// (b.savon 2013-01-21 12:22) - PLID 54720
	EUserRoleTypes m_urtCurrentuser;
	// (b.savon 2013-01-23 12:33) - PLID 54782
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlQuickList;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlQuickListSupervisor;
	void SetQuickListResultsSize(BOOL bShow, long nResults, long nHeight);
	void SetQuickListSupervisorResultsSize(BOOL bShow, long nResults, long nHeight);
	void CreateAndSavePrescriptions(CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> &arySelectedRows);
	void PopulateSupervisorPopout(NXDATALIST2Lib::IRowSettingsPtr pRow);
	CArray<Nx::SafeArray<IUnknown *>, Nx::SafeArray<IUnknown *>> m_arySupervisorMeds;
	NXDATALIST2Lib::IRowSettingsPtr InsertWritePrescriptionRow(NXDATALIST2Lib::_DNxDataListPtr nxdlQuickList);
	NXDATALIST2Lib::IRowSettingsPtr InsertConfigureRow(NXDATALIST2Lib::_DNxDataListPtr nxdlQuickList);
	CString GetDisplayName(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication);
	// (b.savon 2013-01-24 18:47) - PLID 54846
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlPrescriberFilter;
	// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no meds/allergies/and reviewed check boxes
	NxButton m_checkReviewedAllergies;
	NxButton m_checkHasNoAllergies;
	NxButton m_checkHasNoMeds;


	typedef struct {
		ReminderOptions m_selection;
		ReminderOptions m_Oldselection;
		CSpinButtonCtrl	m_hourSpin;
		NxButton m_minuteBtn;
		NxButton m_hourBtn;
		NxButton m_neverBtn;
		NxButton m_remindLoginBtn;
		CSpinButtonCtrl	m_minuteSpin;
		CNxEdit	m_nxeditRemindMinuteTime;
		CNxEdit	m_nxeditRemindHourTime;
		CNxStatic m_nxstaticHour;
		CNxStatic m_nxstaticMinutes;
		NxButton  m_btnRemindMeGroup;
		bool m_bChanged = false;
		int m_noldMinuteValue = 0;
		int m_noldHourValue = 0;
	} SReminderControls;
	// (s.tullis 2016-01-28 17:46) - PLID 68090 
	SReminderControls m_PrescriptionNeedingAttention;
	// (s.tullis 2016-01-28 17:45) - PLID 67965 
	SReminderControls m_Renewals;
	bool m_bLoadingReminders = false;

	// (r.gonet 09/20/2013) - PLID 58416 - Button that opens the patient's medication history.
	CNxIconButton m_btnMedicationHistory;
	// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
	HICON m_hInfoButtonIcon;
	// (b.savon 2013-08-23 13:36) - PLID 58236 - Use icon and modularize
	HICON m_hIconArrow;
	void InvokeNexFormularyDlg(NXDATALIST2Lib::IRowSettingsPtr pRow, long nFDBID, const CString &strDrugName, ENexFormularySource nfsSource);

	// (b.savon 2013-01-30 09:52) - PLID 54922
	VARIANT_BOOL ImportMedication(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewDrugListID);
	// (b.savon 2013-01-30 11:57) - PLID 54927
	VARIANT_BOOL ImportAllergy(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewAllergyID);

	// (b.savon 2013-02-05 11:23) - PLID 51705
	BOOL IsLicensedConfiguredUser();
	CNexERxLicense m_lNexERxLicense;
	BOOL m_bCachedIsLicensedConfiguredUser;
	// (a.wilson 2013-04-30 16:45) - PLID 56509 - create member of the queue.
	NexERxUser m_nexerxUser;
	
	//Members
	// (a.wilson 2012-10-24 10:45) - PLID 51711 - changed to incorporate renewals
	long m_nPendingState, m_nRenewalState;
	long m_nPatientID;
	// (j.jones 2013-03-26 15:30) - PLID 53532 - renamed to reflect what the variable really means,
	// it's the ID of the currently opened EMN, such that creating new prescriptions would use this EMN ID
	long m_nCurrentlyOpenedEMNID;
	bool m_bReadOnly;
	// (j.fouts 2012-11-20 15:23) - PLID 53840 - Added a way to hide renewals and sending
	bool m_bShowRenewals;
	bool m_bShowSend;
	bool m_bOpenToRenewals;
	// (j.jones 2012-11-19 17:52) - PLID 52819 - added provider ID, location ID, and date, used for defaults for new prescriptions
	long m_nDefaultProviderID;
	long m_nDefaultLocationID;
	COleDateTime m_dtDefaultPrescriptionDate;

	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Array of collapsable boxes
	CArray<CCollapseableBox*,CCollapseableBox*> m_aryCollapseableBoxes;
	// (j.fouts 2012-11-06 12:15) - PLID 53574 - Add meds and allergies
	CCollapseableBox m_pendingBox, m_renewalsBox, m_medsAllergiesBox;
	bool m_bShowMedsAllergies;
	// (r.gonet 08/19/2013) - PLID 58416 - Hide or show the medication history button.
	bool m_bShowMedHistory;
	// (b.savon 2014-01-03 07:44) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr) 
	// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
	bool m_bShowNexFormulary;
	void HideNexFormularyColumn(NXDATALIST2Lib::_DNxDataListPtr nxdlDatalist, const short column);
	// (j.fouts 2012-11-16 11:02) - PLID 53156 - Show/Hide interactions button
	bool m_bShowInteractions;
	// (j.fouts 2012-11-16 11:02) - PLID 53156 - Show/Hide write prescription button
	bool m_bShowWrite;
	long m_nPrescriptionID;
	// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates into the queue
	long m_nDefTemplateRowIndex;

	// (j.fouts 2012-11-08 13:32) - PLID 53574 - Added no meds/allergies as well as reviewed status
	CString m_strAllergiesReviewed;
	BOOL m_bHasNoAllergies;
	BOOL m_bHasNoMeds;

	// (j.fouts 2012-11-15 10:35) - PLID 53573 - Added a interaction dlg for when this is not embeded
	scoped_ptr<class CDrugInteractionDlg> m_pDrugInteractionDlg;

	// (j.fouts 2012-11-15 10:35) - PLID 53573 - Flag for if this is embeded or not
	bool m_bInMedicationsTab;

	// (j.fouts 2012-11-14 11:42) - PLID 53743 - Number of print boxes checked
	long m_nCountPrintSelected;
	// (j.fouts 2012-11-14 11:42) - PLID 53745 - Number of send boxes checked
	long m_nCountSendSelected;

	Nx::SafeArray<IUnknown*> m_saryAllPharmacies;

	// (r.gonet 2016-01-22) - PLID 67973 - With OnSetCursor, if it throws an exception, it will flood the user
	// with exceptions if we let it. Only throw an exception once. Silently discard any more.
	bool m_bNotifyOnce = true;

		// (s.dhole 2013-07-08 14:17) - PLID 56931
	void ColumnSizingFinishedpatallergies(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	
	long GetColumnWidth(const CString& strColWidths, short nColumn);
	// (s.dhole 2013-07-08 14:17) - PLID 56931
	CString  ReadColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pList);
	void ResizeCurrentMedicationColumns(const CString strColWidths );
	void ResizeAllergyColumns(const CString strColWidths );
	// (s.dhole 2013-11-19 09:36) - PLID  56926
	void  UpdateCurrentMedRecord(NXDATALIST2Lib::IRowSettingsPtr pRow,short nColumn,const VARIANT&  varValue );

	// (j.jones 2016-01-22 08:42) - PLID 67993 - resets the prescription search provider based
	// on the value of the prescription' 'include free text' checkbox
	void ResetPrescriptionSearchProvider();

	// (j.jones 2016-01-22 09:22) - PLID 67996 - resets the current meds search provider based
	// on the value of the current meds' 'include free text' checkbox
	void ResetCurrentMedsSearchProvider();

	// (j.jones 2016-01-22 09:58) - PLID 67997 - resets the allergy search provider based
	// on the value of the allergies' 'include free text' checkbox
	void ResetAllergiesSearchProvider();

	// (b.eyers 2016-01-21) - PLID 67966
	BOOL m_bDateFromDown, m_bDateToDown;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	virtual BOOL OnInitDialog();
	virtual BOOL IncludeChildPosition(HWND hwnd);

	void LeftClickRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickPrescriptionQueueList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChosenQueueStatusFilter(LPDISPATCH lpRow);

	afx_msg void OnBnClickedUpdateRenewals();
	afx_msg void OnBnClickedExpandFirst();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void DblClickCellPrescriptionQueueList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedMedsAllergiesBtn();
	void SelChangedPrescriptionQueueList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedCurrentMedsList(short nFlags);
	void RequeryFinishedAllergiesList(short nFlags);
	afx_msg void OnBnClickedPrintAllPresctiptions();
	afx_msg void OnBnClickedSendToPrinter();
	void EditingFinishedPrescriptionQueueList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedBtnConfigureTemplates();
	afx_msg void OnBnClickedBtnEditPrescriptionTemplate();
	afx_msg void OnBnClickedInteractionsButton();
	virtual int SetControlPositions();
	afx_msg void OnRadioEmnMeds();
	afx_msg void OnRadioPatientMeds();
	afx_msg LRESULT OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam);
	void EnforceDisabledControls();
	// (r.gonet 2016-01-22) - PLID 67967 - Encapsulates the filter loading for prescriptions.
	void InitializeRxFilters();
	void InitializeRenewalFilters();
	afx_msg LRESULT OnInteractionsChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedEsubmitAll();
	void RButtonUpPrescriptionQueueList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangedFilterRenewalResponse(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangedFilterRenewalTransmit(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDtnDatetimechangeFilterRenewalFrom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeFilterRenewalTo(NMHDR *pNMHDR, LRESULT *pResult);
	void SelChangedFilterRenewalProvider(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnNMKillfocusFilterRenewalFrom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocusFilterRenewalTo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedWritePickList();
	afx_msg void OnBnClickedBtnEditFavoritePharmacies();
	afx_msg void OnDestroy();
	// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed OnBnClickedRenewalDefaultPrescriber.
	afx_msg void OnBnClickedBtnRxNeedingAttention();
	void RButtonUpRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnEditMedsQueue();
	void LeftClickNxdlQuickListPopout(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void FocusLostNxdlQuickListPopout();
	void FocusLostNxdlQuickListSupervisorPopout();
	void FocusGainedNxdlQuickListSupervisorPopout();
	void FocusGainedNxdlQuickListPopout();
	void SelChosenNxdlQueuePrescriberFilter(LPDISPATCH lpRow);
	void RButtonUpCurrentMedsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonUpAllergiesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedNoMedsCheckQueue();
	afx_msg void OnBnClickedNoAllergyQueue();
	afx_msg void OnBnClickedAllergiesReviewedQueue();
	void LeftClickNxdlQuickListSupervisorPopout(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedBtnMoreMeds();
	afx_msg void OnBnClickedBtnEditAllergiesQueue();
	// (j.fouts 2013-04-22 16:01) - PLID 54719 - Update View when we show
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void EditingFinishedCurrentMedsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingStartingCurrentMedsList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishingAllergiesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedAllergiesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedBtnNexformulary();
	// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
	afx_msg void OnCheckHideDiscontinuedAllergiesQueue();
	afx_msg void OnCheckHideDiscontinuedMedsQueue();
	// (r.gonet 09/20/2013) - PLID 58416 - Handler for the med history button
	afx_msg void OnBnClickedRxQueueMedHistoryBtn();
	// (r.gonet 09/20/2013) - PLID 58396 - Handler for the refresh med history button  message.
	LRESULT OnBackgroundMedHistoryRequestComplete(WPARAM wParam, LPARAM lParam);
	// (s.dhole 2013-10-25 17:24) - PLID 59189 Check if folrmuary information exist 
	long CheckExistingFormularyData();
	// (j.jones 2013-10-17 16:13) - PLID 58983 - added left click handler
	void OnLeftClickCurrentMedsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (b.eyers 2016-01-21) - PLID 67966
	afx_msg void OnChangeRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPrescriptionUseDate();
	// (r.gonet 2016-01-22) - PLID 67973 - Handles when the user clicks a label control.
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// (r.gonet 2016-01-22) - PLID 67973 - Handles when the cursor is set.
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
public:
	void ColumnSizingFinishedCurrentMedsList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	// (s.dhole 2013-11-21 11:46) - PLID 56931 save column size
	void ColumnSizingFinishedAllergiesList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void EditingFinishingCurrentMedsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
	void SelChosenNxdlMedSearchResultsQueue(LPDISPATCH lpRow);
	void LButtonDownNxdlWriteRx(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChosenNxdlWriteRx(LPDISPATCH lpRow);
	void SelChosenNxdlAllergySearchResultsQueue(LPDISPATCH lpRow);
	// (j.jones 2016-01-21 16:59) - PLID 67993 - added option to include free text meds in the prescription search
	afx_msg void OnCheckIncludeFreeTextRx();
	// (j.jones 2016-01-21 17:06) - PLID 67996 - added option to include free text meds in the current meds search
	afx_msg void OnCheckIncludeFreeTextMeds();
	// (j.jones 2016-01-21 17:06) - PLID 67997 - added option to include free text allergies in the allergy search
	afx_msg void OnCheckIncludeFreeTextAllergies();

	// (s.tullis 2016-01-28 17:46) - PLID 68090
	afx_msg void OnEnChangeErxMinuteTime();
	afx_msg void OnEnChangeErxHourTime();
	afx_msg void OnBnClickedRadioErxDontremind2();
	afx_msg void OnBnClickedRadioErxRemindlogin();
	afx_msg void OnBnClickedRadioErxHour();
	afx_msg void OnBnClickedErxRadioMinutes();
	afx_msg void OnDeltaposErxMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposErxHourSpin(NMHDR *pNMHDR, LRESULT *pResult);
	// (s.tullis 2016-01-28 17:45) - PLID 67965 
	afx_msg void OnEnChangeRenewalMinuteTime();
	afx_msg void OnEnChangeRenewalHourTime();
	afx_msg void OnBnClickedRenewalsRadioMinutes();
	afx_msg void OnBnClickedRenewalsRadioHour();
	afx_msg void OnBnClickedRenewalsRadioLogin();
	afx_msg void OnBnClickedRenewalsRadioDontRemind();
	afx_msg void OnBnClickedRenewalsMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRenewalsHourSpin(NMHDR *pNMHDR, LRESULT *pResult);
	void OnBnClickedRadioDontremind(SReminderControls &sControls);
	void OnBnClickedRadioRemindlogin(SReminderControls &sControls);
	void OnBnClickedRadioHour(SReminderControls &sControls);
	void OnBnClickedRadioMinutes(SReminderControls &sControls);
	void OnDeltaposMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult, SReminderControls &sControls);
	void OnDeltaposHourSpin(NMHDR *pNMHDR, LRESULT *pResult, SReminderControls &sControls);
	void ShowHideReminderControls(BOOL bShow);
	void DisableEnableReminderControls(BOOL bEnable);
	void SaveReminder(SReminderControls &sControls,bool bPrescriptionNeedingAttention = true);
	void LoadReminder(SReminderControls &sControls, bool bPrescriptionNeedingAttention = true);
	BOOL CheckWarnInvalidInput(SReminderControls &sControls);

};
