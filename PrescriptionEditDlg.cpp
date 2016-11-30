// PrescriptionEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PrescriptionEditDlg.h"
#include "EditMedicationListDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "TaskEditDlg.h"
#include "GlobalUtils.h"
#include "MergeEngine.h"
#include "SelectDlg.h"
#include "GlobalParamUtils.h"
#include "CorrectMedicationQuantitiesDlg.h"
#include "PharmacyDirectorySearchDlg.h"
#include "SureScriptsPractice.h"
#include "PatientsRc.h"
#include "SureScriptsConfirmationDlg.h"
#include "FirstDataBankUtils.h"
#include <foreach.h>
#include "PrescriptionDosingDlg.h"
#include "ImportUtils.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "AssignPatientRenewalDlg.h"
#include "RenewalResponseReasonDlg.h"
#include "WebFaxUtils.h"
#include "FaxSetupDlg.h"
#include "SingleSelectDlg.h"
#include "MedicationSelectDlg.h"
#include "DontShowDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "DrugInteractionDlg.h"			// (j.fouts 2013-04-26 09:55) - PLID 53146 - We need to show this before they can prescribe
#include "PrescriptionDiagnosisCodeDlg.h" // (b.savon 2013-09-24 08:42) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
#include "NexFormularyDlg.h"
#include "MedlinePlusUtils.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "PharmacySearchUtils.h" // (r.gonet 2016-02-23 00:14) - PLID 67988
#include "PharmacySearch_DatalistProvider.h" // (r.gonet 2016-02-23 00:14) - PLID 67988
#include "ErxPrescriptionReviewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionEditDlg dialog

// (j.jones 2008-05-13 11:15) - PLID 29732 - created
// (j.fouts 2012-11-01 15:08) - PLID 53566 - Moved all members and methods related to the Queue into PrescriptionQueueDlg.cpp
// (b.savon 2013-03-08 13:12) - PLID 55518 - Changed everywhere to use EMNSpawnSource, Provider, Supervisor, NurseStaff objects.

using namespace NXDATALIST2Lib;
using namespace ADODB;

#define NXM_POST_SHOWWINDOW	WM_APP+0x100

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

extern CPracticeApp theApp;

#define PRESCRIBED_MEDICATION_TEXT "Prescribed Medication:"
#define DISPENSED_MEDICATION_TEXT "Dispensed Medication:"

// (a.walling 2009-07-01 11:03) - PLID 34052 - This is used for Prescriber Agent and Supervisor now, too.
// (b.savon 2013-04-04 13:59) - PLID 56085 - Display Address and Phone (Removed City, State, Zip because they are now concatenated) 
enum ProviderComboColumn {

	provccID = 0,
	provccName,
	provccAddress,
	provccPhone,
};

enum LocationComboColumn {

	locccID = 0,
	locccName,
};

// (r.farnworth 2016-01-18 08:39) - PLID 67750
enum PackagingColumns {
	pNDC = 0,
	pSize = 1,
	pForm = 2,
	pDescription = 3,
	pSizeFormDescription = 4,
};

enum QuantityUnitColumns {
	qucID = 0,
	qucName = 1,
	qucSureScriptsCode = 2,
	qucNCItCode = 3,
};

// (j.fouts 2013-01-10 10:54) - PLID 54526 - Added Dosage quantity list
enum DosageQuantityListColumns {
	dqlcID = 0,
	dqlcSigText = 1,
	dqlcValue = 2,
};

// (j.fouts 2013-01-10 10:55) - PLID 54529 - Added dosage frequency list
enum FrequencyListColumns {
	flcID = 0,
	flcSigText = 1,
	flcValue = 2,
};

// (j.fouts 2013-02-01 15:09) - PLID 54528 - Added dosage route list
enum DosageRouteListColumns {
	drlcID = 0,
	drlcRoute = 1,
	drlcAction = 2,
};

// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Units list
enum DosageUnitListColumns {
	dulcID = 0,
	dulcUnit = 1,
	dulcUnitPlural = 2,
};

//TES 8/22/2013 - PLID 57999 - Formularies
// (b.savon 2013-09-25 12:38) - PLID 58762 - Use the same columns as the NexFormularyDlg
enum FormularyInsuranceListColumns {
	filcID = 0,
	filcPatientID,
	filcInsuranceCoName,
	filcPBMID, // (s.dhole 2013-09-12 17:26) - PLID 58556
	filcPBMName, // (s.dhole 2013-09-12 17:26) - PLID 58556
	filcInsuranceResponsibilityID,
	filcInsuranceResponsibility,
	filcSureScriptsFormularyID,
	filcSureScriptsAlternativesID,
	filcSureScriptsCoverageID,
	filcSureScriptsCopayID,
	filcPlanName,// (s.dhole 2013-09-12 17:26) - PLID 58556
	filcEligibilityCoverageStatus, // (r.gonet 2016-03-03 12:41) - PLID 68494
	filcPharmacyCoverage,// (s.dhole 2013-09-12 17:26) - PLID 58556
	filcLastSureScriptsRequestDate,// (s.dhole 2013-09-12 17:26) - PLID 58556
	filcDemographicsHaveChanged,
	filcDisplay,// (s.dhole 2013-09-12 17:26) - PLID 58556
	filcIsFailure,// (s.dhole 2013-10-08 16:07) - PLID 58923
	filcActive,// (s.dhole 2013-10-08 16:07) - PLID 58923
};

// (j.jones 2012-10-31 15:05) - PLID 52819 - added nCurrentlyOpenedEMNID, the ID of the EMN currently being edited, and bIsEMRTemplate.
CPrescriptionEditDlg::CPrescriptionEditDlg(CWnd* pParent /*= NULL*/, long nCurrentlyOpenedEMNID /*= -1*/, BOOL bIsEMRTemplate /*= FALSE*/)
	: CNxDialog(CPrescriptionEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrescriptionEditDlg)
		m_nPatientID = -1;
		// (j.jones 2012-10-31 15:05) - PLID 52819 - Added nCurrentlyOpenedEMNID, the ID of the EMN currently being edited, and bIsEMRTemplate.
		// This EMNID may or may not necessarily be the EMNID associated with the prescription we are editing.
		m_nCurrentlyOpenedEMNID = nCurrentlyOpenedEMNID;
		m_bIsEMRTemplate = bIsEMRTemplate;
	
		m_bReadOnly = FALSE;
		m_bRePrescribe = FALSE; // (r.farnworth 2016-01-08 15:33) - PLID 58692

		m_nPendingLocationID = -1;
		m_nPendingPharmacyID = -1;
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Removed dead code about toggling on and off favorite pharmacies.
		m_varOrderSetID.vt = VT_NULL;
		// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
		m_nFDBID = -1;
		// (j.fouts 2012-11-07 16:23) - PLID 53566 - No changes yet
		m_bChangesMade = false;
		// (j.jones 2012-11-16 08:55) - PLID 53765 - track what it was we did in this dialog, default to edit
		m_epdrvAction = epdrvEditRx;
		// (j.jones 2016-01-06 15:33) - PLID 67823 - track if the prescription is new - in most cases it is not
		m_bIsNewRx = false;
		m_bHideDialog = false;
		// (a.wilson 2012-11-16 14:15) - PLID 53799
		m_bShowDispensedMedication = false;
		m_bInitialRenewalDisplayComplete = false;
		// (j.fouts 2013-01-10 10:52) - PLID 54463 - We should track changes to the sig and not generate a sig if they have edited it
		m_bDontGenerateSig = false;
		// (b.savon 2013-01-16 10:39) - PLID 54632 - Default the Prescriber, Supervisor, and Nurse/Staff on the Prescription Edit dlg based on the configured assigned roles.
		m_urtCurrentUserRole = urtNone;
		m_bCachedIsLicensedConfiguredUser = FALSE;
		// (a.wilson 2013-02-05 12:41) - PLID 55014 - defaults for new variables from denynewrx feature.
		// (r.gonet 2016-02-23 00:53) - PLID 68408 - Removed the stored m_nPharmacyID since its usage was removed.
		m_nDenyNewRxResponseID = -1;
		m_nPrevPharmacyID = -1;
		// (a.wilson 2013-05-01 09:30) - PLID 56509 - default to false.
		m_bUserAuthorizedPrescriber = false;

		//TES 8/29/2013 - PLID 57999 - New variables
		m_nFormularyStatus = -2;
		// (s.dhole 2013-10-19 13:46) - PLID 59068
		m_SelectedFInsuranceID =-1;
		// (r.gonet 2016-02-23 12:47) - PLID 67964 - Init the star icon, which is used for Favorite Pharmacies.
		m_hStarIcon = nullptr;
		// (r.gonet 2016-02-24 17:01) - PLID 67988 - Cache the license check.
		m_bHasEPrescribingLicense = false;

	//}}AFX_DATA_INIT
}

CPrescriptionEditDlg::~CPrescriptionEditDlg()
{
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Free the favorite icon.
	if (m_hStarIcon) {
		DestroyIcon(m_hStarIcon);
		m_hStarIcon = nullptr;
	}
}

void CPrescriptionEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrescriptionEditDlg)
	DDX_Control(pDX, IDC_DATE_LABEL, m_nxstaticDateLabel);
	DDX_Control(pDX, IDC_FIRST_DATE_LABEL, m_nxstaticFirstDateLabel);
	DDX_Control(pDX, IDC_BTN_PREVIEW_PRESCRIPTION, m_btnPreview);
	DDX_Control(pDX, IDC_FIRSTDATE, m_editFirstDate);
	DDX_Control(pDX, IDC_EDIT_QUANTITY, m_editQuantity);
	DDX_Control(pDX, IDC_PRESCRIPTION_ENGLISH_EXPLANATION, m_editEnglishExplanation);
	DDX_Control(pDX, IDC_PRESCRIPTION_EXPLANATION, m_editPrescriptionExplanation);
	DDX_Control(pDX, IDC_DOSAGE_FORM, m_editDosageForm);
	DDX_Control(pDX, IDC_MEDICATION_NAME_LABEL, m_nxstaticMedicationNameLabel);
	DDX_Control(pDX, IDC_ENGLISH_EXPLANATION_LABEL, m_nxstaticEnglishExplanationLabel);	
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_PRESCRIPTION_DATE, m_dtPrescriptionDate);
	DDX_Control(pDX, IDC_REMINDER_DATE, m_dtReminderDate);
	DDX_Control(pDX, IDC_STRENGTH, m_editStrength);
	DDX_Control(pDX, IDC_DAYS_SUPPLY, m_editDaysSupply);
	DDX_Control(pDX, IDC_ALLOW_SUBSTITUTIONS, m_checkAllowSubstitutions);
	DDX_Control(pDX, IDC_PRIOR_AUTH, m_editPriorAuthorization);
	DDX_Control(pDX, IDC_PRIOR_AUTH_LABEL, m_nxlabelPriorAuthLabel);
	DDX_Control(pDX, IDC_REFILLS_LABEL, m_nxlabelRefills);
	DDX_Control(pDX, IDC_NOTE_TO_PHARMACIST, m_editNoteToPharmacist);
	DDX_Control(pDX, IDC_STRENGTH_UNIT_DISPLAY, m_nxeditStrengthUnits);
	DDX_Control(pDX, IDC_SAMPLE_EXPIRATION_LBL, m_nxsSampleExpirationLbl);
	DDX_Control(pDX, IDC_ESUBMIT_BUTTON, m_btnESubmit);
	DDX_Control(pDX, IDC_DELETE_PRESCRIPTION, m_btnDelete);
	DDX_Control(pDX, IDC_DOSAGE_LABEL, m_nxlabelDosage);
	DDX_Control(pDX, IDC_BTN_ERx_ERROR, m_nxBtnRxError);
	DDX_Control(pDX, IDC_PRESCRIPTION_PATIENT_NAME, m_nxPatientName);
	DDX_Control(pDX, IDC_PRESCRIPTION_PATIENT_ADDRESS, m_nxPatientAddress);
	DDX_Control(pDX, IDC_PRESCRIPTION_PATIENT_GENDER, m_nxPatientGender);
	DDX_Control(pDX, IDC_PRESCRIPTION_PATIENT_DOB, m_nxPatientBirthdate);
	DDX_Control(pDX, IDC_PRESCRIPTION_PATIENT_PHONE, m_nxPatientPhone);
	DDX_Control(pDX, IDC_APPROVE_RENEWAL, m_btnApprove);
	DDX_Control(pDX, IDC_APPROVE_WITH_CHANGES_RENEWAL, m_btnApproveWithChanges);
	DDX_Control(pDX, IDC_DENY_RENEWAL, m_btnDeny);
	DDX_Control(pDX, IDC_PRESCRIPTION_LABEL, m_nxMedicationLabel);
	DDX_Control(pDX, IDC_PHARM_NOTE, m_nxstaticNoteToPharm);
	DDX_Control(pDX, IDC_ORIGINAL_PRESCRIPTION_BTN, m_btnViewOriginalPrescription);
	DDX_Control(pDX, IDC_ASSIGN_PATIENT_LINK, m_nxAssignPatientLink);
	DDX_Control(pDX, IDC_SAVE_DEFAULT, m_checkSaveAsDefault);
	DDX_Control(pDX, IDC_COPY_TO_PICK_LIST, m_checkAddToFavorites);
	DDX_Control(pDX, IDC_PRESCRIPTION_LABEL_DISPENSED, m_nxDispensedMedicationLabel);
	DDX_Control(pDX, IDC_MEDICATION_NAME_LABEL_DISPENSED, m_nxDispensedMedicationNameLabel);
	DDX_Control(pDX, IDC_DENY_AND_REWRITE_RENEWAL, m_btnDenyAndRewrite);
	DDX_Control(pDX, IDC_BTN_TODO_PRESEDIT, m_btnCreateTodo);
	DDX_Control(pDX, IDC_FAX_BUTTON, m_btnEFax);
	DDX_Control(pDX, IDC_ALLOW_SUBSTITUTIONS_TEXT, m_nxsAllowSubsText);
	DDX_Control(pDX, IDC_SAVE_DEFAULT_TEXT, m_nxsSaveAsDefaultText);
	DDX_Control(pDX, IDC_COPY_TO_PICK_LIST_TEXT, m_nxsSaveToQuickListText);
	DDX_Control(pDX, IDC_RENEWAL_WARNING_LABEL, m_nxRenewalWarning);
	DDX_Control(pDX, IDC_RENEWAL_RESPONSE_REASON, m_editResponseReason);
	DDX_Control(pDX, IDC_STATIC_INTERNAL_DATE, m_nxstaticInternalDate);
	DDX_Control(pDX, IDC_STATIC_INTERNAL_PRIOR, m_nxstaticPriorAuthInternalLabel);
	DDX_Control(pDX, IDC_STATIC_INTERNAL_SAMPLE, m_nxstaticSampleInternalLabel);
	DDX_Control(pDX, IDC_RENEWAL_AUTHORIZATION_WARNING_LABEL, m_nxUserAuthorizationWarning);
	DDX_Control(pDX, IDC_STATIC_STRENGTH_INTERNAL, m_nxstaticStrengthInternalLabel);
	DDX_Control(pDX, IDC_SYSLINK_CS_URL, m_linkControlledSubstance);
	DDX_Control(pDX, IDC_STATIC_RX_DIAG_CODE, m_nxsAddDiagnosisCodes);
	DDX_Control(pDX, IDC_BTN_VIEW_COVERAGE, m_btnFCovarage);
	DDX_Control(pDX, IDC_PT_EDUCATION_LABEL, m_nxlabelPatientEducation);
	DDX_Control(pDX, IDC_BTN_PT_EDUCATION, m_btnPatientEducation);
	DDX_Control(pDX, IDC_PHARMACY_SELECTION_LABEL, m_nxlPharmacySelection);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_DATE, 2 /* Change */, OnChangePrescriptionDate, VTS_NONE)
//	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_DATE, 3 /* CloseUp */, OnCloseUpPrescriptionDate, VTS_NONE)

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
BEGIN_MESSAGE_MAP(CPrescriptionEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPrescriptionEditDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PRESCRIPTION_DATE, OnChangePrescriptionDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_PRESCRIPTION_DATE, OnCloseUpPrescriptionDate)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_PREVIEW_PRESCRIPTION, OnBtnPreviewPrescription)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_PRESCRIPTION_EXPLANATION, &CPrescriptionEditDlg::OnEnKillfocusPrescriptionExplanation)
	ON_EN_KILLFOCUS(IDC_NOTE_TO_PHARMACIST, &CPrescriptionEditDlg::OnEnKillfocusNoteToPharmacist)
	ON_EN_KILLFOCUS(IDC_PRIOR_AUTH, &CPrescriptionEditDlg::OnEnKillfocusPriorAuth)
	ON_EN_KILLFOCUS(IDC_DAYS_SUPPLY, &CPrescriptionEditDlg::OnEnKillfocusDaysSupply)
	ON_EN_KILLFOCUS(IDC_EDIT_QUANTITY, &CPrescriptionEditDlg::OnEnKillfocusEditQuantity)
	ON_BN_CLICKED(IDC_ALLOW_SUBSTITUTIONS, &CPrescriptionEditDlg::OnBnClickedAllowSubstitutions)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_DELETE_PRESCRIPTION, &CPrescriptionEditDlg::OnBnClickedDeletePrescription)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_ERx_ERROR, &CPrescriptionEditDlg::OnBnClickedBtnErxError)
	ON_BN_CLICKED(IDC_ESUBMIT_BUTTON, &CPrescriptionEditDlg::OnBnClickedEsubmitButton)
	ON_BN_CLICKED(IDC_APPROVE_RENEWAL, &CPrescriptionEditDlg::OnBnClickedApproveRenewal)
	ON_BN_CLICKED(IDC_APPROVE_WITH_CHANGES_RENEWAL, &CPrescriptionEditDlg::OnBnClickedApproveWithChangesRenewal)
	ON_BN_CLICKED(IDC_DENY_RENEWAL, &CPrescriptionEditDlg::OnBnClickedDenyRenewal)
	ON_BN_CLICKED(IDC_ORIGINAL_PRESCRIPTION_BTN, &CPrescriptionEditDlg::OnBnClickedOriginalPrescriptionBtn)
	ON_BN_CLICKED(IDC_DENY_AND_REWRITE_RENEWAL, &CPrescriptionEditDlg::OnBnClickedDenyAndRewriteRenewal)
	ON_BN_CLICKED(IDC_BTN_TODO_PRESEDIT, &CPrescriptionEditDlg::OnBnClickedBtnTodoPresedit)
	ON_BN_CLICKED(IDC_FAX_BUTTON, &CPrescriptionEditDlg::OnBnClickedFaxButton)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_CS_URL, &CPrescriptionEditDlg::OnNMClickSyslinkCsUrl)
	ON_BN_CLICKED(IDC_BTN_VIEW_COVERAGE, &CPrescriptionEditDlg::OnBnClickedBtnViewCoverage)
	ON_BN_CLICKED(IDC_BTN_PT_EDUCATION, &CPrescriptionEditDlg::OnBtnPtEducation)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_POST_SHOWWINDOW, OnPostShowWindow)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionEditDlg message handlers

//TES 2/16/2009 - PLID 32080 - These are the two possible options for the label by the "Prior Authorization" field
#define PRIOR_AUTHORIZATION_TEXT	"Prior Authorization #"
#define	SAMPLE_PRESCRIPTION_TEXT	"Sample Prescription #"

#define REFILLS_TEXT				"Refills"
#define REFILL_AS_NEEDED_TEXT		"Refill As Needed"
#define REFILL_AS_NEEDED_VALUE		"As Needed"
// (a.wilson 2013-01-23 11:07) - PLID 53799 - per the specifications we need to elaborate on the refill text.
#define REFILL_RENEWAL_TEXT			"Total Refills Approved (1 + X)"	

BOOL CPrescriptionEditDlg::OnInitDialog() 
{	
	try {
		CNxDialog::OnInitDialog();

		//TES 8/19/2009 - PLID 31284 - Added bulk caching
		// (a.walling 2009-11-18 16:45) - PLID 36352 - Include backdating tolerance for electronic prescriptions
		g_propManager.CachePropertiesInBulk("PrescriptionEditDlg", propNumber,
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ShowEnglishPrescriptionColumn' OR "
			"Name = 'DefaultPharmacy' OR "
			"Name = 'ePrescribeBackDateTolerance' OR "
			"Name = 'ERxFaxConfig' OR "
			"Name = 'DrugListNotesLinkToPharmNotes' OR "
			"Name = 'ShowPatientEducationLinks' OR " // (r.gonet 2014-01-27 15:29) - PLID 59339
			"Name = 'ShowPatientEducationButtons' " // (r.gonet 2014-01-27 15:29) - PLID 59339
			")"
			")",
			_Q(GetCurrentUserName()));

		// (b.savon 2013-09-18 14:26) - PLID 58455 - Add a URL link to launch a webbrowser to the IStop web address so that prescribers can access the controlled substance registry
		g_propManager.CachePropertiesInBulk("PrescriptionEditDlg_Text", propText,
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NexERxControlledSubstanceURL' "
			")"
			")",
			_Q(GetCurrentUserName()));

		// (r.gonet 2016-03-03 13:45) - PLID 68494 - Flag that if true will cause the formulary information to reload later in this function.
		bool bReloadFormularyInformation = false;

		if(m_pCurrentPrescription)
		{
			//Maintain m_nFDBID
			CString strFDBMedID = AsString(m_pCurrentPrescription->Medication->FDBMedID);
			m_nFDBID = strFDBMedID.IsEmpty() ? -1 : atol(strFDBMedID);

			//Lets maintain the readonly flag to include perscription with a readonly status
			switch(m_pCurrentPrescription->status)
			{
			case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
			case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
			case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
			case NexTech_Accessor::PrescriptionStatus_eFaxed:
				// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
			case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
				// (b.savon 2013-09-04 15:11) - PLID 58212 - Add a new 'Void' type for Prescriptions
			case NexTech_Accessor::PrescriptionStatus_Void:
				//Any of these statuses mean the pescription is readonly
				m_bReadOnly = true;
				break;
			default:
				//Otherwise just leave the flag as is, because we may have opened this dlg readonly
				break;
			}

			// (b.savon 2014-12-03 07:51) - PLID 64143 - Move this before the controlled substance URL check
			Gdiplus::Bitmap* pPicture;
			if (!CString((LPCTSTR)m_pCurrentPrescription->NewCropGUID).IsEmpty())
			{
				//This prescription came from NewCrop, it should be readonly
				m_bReadOnly = true;

				// (b.savon 2014-12-03 07:55) - PLID 64143 - Create a "Copy Only" Rx pad to be used for NewCrop prescriptions and display it on the Prescription Edit screen when view NewCrop prescriptions
				pPicture = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PRESCRIPTION_BACK_COPY));
			}
			else{
				// This didn't come from NewCrop, use the regular background
				// (j.fouts 2012-10-02 08:54) - PLID 52973 - Load the prescription back background
				pPicture = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PRESCRIPTION_BACK));
			}
			// (r.gonet 2016-02-23 12:47) - PLID 67964 - Load the favorite icon.
			m_hStarIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STAR_FILLED), IMAGE_ICON, 16, 16, 0);
			//Existing logic, move outside newcrop conditional check after we load the proper PNG into the Bitmap pointer
			Gdiplus::Color clrSolidBkgColor;
			clrSolidBkgColor.SetFromCOLORREF(CNxDialog::GetSolidBackgroundColor());
			if (pPicture) {
				pPicture->GetHBITMAP(clrSolidBkgColor, &m_imgPrescriptionBack);
				delete pPicture;
			}

			// (j.fouts 2013-04-01 12:53) - PLID 53840
			if (!(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS))
			{
				m_bReadOnly = true;
			}

			// (j.jones 2016-02-03 12:03) - PLID 68136 - if not read only, and this prescription
			// has no formulary data linked to it yet, try to update the formulary
			if (!m_bReadOnly && g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts
				&& m_nPatientID != -1) {

				CString strDetailID = (LPCSTR)m_pCurrentPrescription->InsuranceDetailID;
				if (atoi(strDetailID) <= 0) {

					//check the formulary
					long nInsuranceDetailID = CheckExistingFormularyData(this, m_nPatientID);
					//if we got a detail ID back, assign it to this prescription
					if (nInsuranceDetailID > 0) {
						strDetailID = AsString(nInsuranceDetailID);

						m_pCurrentPrescription->InsuranceDetailID = _bstr_t(strDetailID);

						NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
						pCommit->Prescription = m_pCurrentPrescription;

						CArray<NexTech_Accessor::_PrescriptionCommitPtr, NexTech_Accessor::_PrescriptionCommitPtr> aryPrescriptions;
						aryPrescriptions.Add(pCommit);
						Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);

						NexTech_Accessor::_SavePrescriptionResultArrayPtr pResults = GetAPI()->SavePrescription(GetAPISubkey(), GetAPILoginToken(),
							saryPrescriptions);
						Nx::SafeArray<IUnknown *> saryResults = pResults->Results;
						foreach(NexTech_Accessor::_SavePrescriptionResultPtr pResult, saryResults) {
							// (s.dhole 2013-02-11 16:49) - PLID 54063
							m_pCurrentPrescription->ValidationList = pResult->ValidationList;
							m_pCurrentPrescription->EnglishDescription = pResult->EnglishDescription;
							SetDlgItemText(IDC_PRESCRIPTION_ENGLISH_EXPLANATION, pResult->EnglishDescription);
							if (pResult->ValidationList->ErrorStatus == NexTech_Accessor::eRxErrorStatus_eRxError) {
								m_epdrvAction = epdrvErrorRx;
							}
						}
						// (r.gonet 2016-03-03 13:45) - PLID 68494 - Set the flag to trigger a reload of the formulary insurance combo.
						bReloadFormularyInformation = true;
					}
				}
			}

			// (b.savon 2013-09-18 14:26) - PLID 58455 - Add a URL link to launch a webbrowser to the IStop web address so that prescribers can access the controlled substance registry
			// (b.savon 2014-12-03 07:52) - PLID 64143 - Removed bogus conditional check -- Always check to display a web address if it isn't readonly
			if (!m_bReadOnly) {
				CString strConfiguredURL = GetRemotePropertyText("NexERxControlledSubstanceURL", "");
				CString strSchedule = VarString(m_pCurrentPrescription->Medication->DEASchedule, "");
				if (!strConfiguredURL.IsEmpty() && !strSchedule.IsEmpty()) {
					CString strCSLink;
					strCSLink.Format(
						"<a href=\"%s\">This is a controlled substance. Click here to check the registry before prescribing.</a>",
						strConfiguredURL
						);
					m_linkControlledSubstance.SetFont(&theApp.m_boldFont);
					m_linkControlledSubstance.EnableWindow();
					m_linkControlledSubstance.ShowWindow(SW_SHOW);
					m_linkControlledSubstance.SetWindowText(strCSLink);
				}
			}
		}

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnPreview.AutoSet(NXB_MERGE);
		// (s.dhole 2012-10-23 ) - PLID 54890
		m_nxBtnRxError.AutoSet(NXB_WARNING);
		// (j.fouts 2012-09-26 10:32) - PLID 52973 - Added new buttons
		// (a.wilson 2012-10-24 10:46) - PLID 51711 - changed to incorporate renewals
		m_btnSave.SetIcon(IDI_SAVE);
		m_btnESubmit.AutoSet(NXB_ERX);
		m_btnApprove.AutoSet(NXB_OK);
		m_btnApproveWithChanges.AutoSet(NXB_OK);
		m_btnDeny.AutoSet(NXB_CANCEL);
		m_btnDenyAndRewrite.AutoSet(NXB_CANCEL);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnEFax.AutoSet(NXB_FAX);
		m_btnViewOriginalPrescription.AutoSet(NXB_INSPECT);
		m_btnViewOriginalPrescription.SetToolTip("View Original Prescription");

		// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
		//Lets assume enabled and disable it later if we need to
		m_nxlabelDosage.SetText("Dosing");
		m_nxlabelDosage.SetType(dtsHyperlink);

		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);

		// (j.jones 2013-10-18 09:46) - PLID 58983 - added infobutton abilities
		m_btnPatientEducation.SetIcon(IDI_INFO_ICON);
		m_nxlabelPatientEducation.SetText("Pat. Edu.");

		// (r.gonet 2014-01-27 10:21) - PLID 59339 - Have a preference to toggle patient education options off.
		if (!bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education info button
			m_btnPatientEducation.ShowWindow(SW_HIDE);
		}

		if (bShowPatientEducationLink) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Show the patient education link
			m_nxlabelPatientEducation.SetType(dtsHyperlink);
		} else if (!bShowPatientEducationLink && bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We just need the label as a description of the info button.
			m_nxlabelPatientEducation.SetType(dtsText);
		} else if (!bShowPatientEducationLink && !bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We don't need the label as either a link or a description label, so hide it.
			m_nxlabelPatientEducation.ShowWindow(SW_HIDE);
		}

		// (j.fouts 2012-10-22 11:35) - PLID 53156 - Default these fields to empty
		m_nxPatientName.SetWindowText("");
		m_nxPatientGender.SetWindowText("");
		m_nxPatientAddress.SetWindowText("");
		m_nxPatientBirthdate.SetWindowText("");
		m_nxPatientPhone.SetWindowText("");

		// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workaround for the checkboxes, they are now labels.
		m_nxsAllowSubsText.SetText("Allow Substitutions");
		m_nxsSaveAsDefaultText.SetText("Save as default for this medication");
		m_nxsSaveToQuickListText.SetText("Copy medication to Quick List");
		m_nxsAllowSubsText.SetType(dtsHyperlink);
		m_nxsSaveAsDefaultText.SetType(dtsHyperlink);
		m_nxsSaveToQuickListText.SetType(dtsHyperlink);

		// (b.savon 2013-09-24 09:45) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
		m_nxsAddDiagnosisCodes.SetText("Add Diagnosis Codes...");
		m_nxsAddDiagnosisCodes.SetType(dtsHyperlink);





		CFont *fBold = &theApp.m_boldFont;
		GetDlgItem(IDC_MEDICATION_NAME_LABEL)->SetFont(fBold);
		GetDlgItem(IDC_MEDICATION_NAME_LABEL_DISPENSED)->SetFont(fBold);
		// (a.wilson 2013-05-01 10:13) - PLID 56509
		GetDlgItem(IDC_RENEWAL_AUTHORIZATION_WARNING_LABEL)->SetFont(fBold);

		// (b.savon 2013-01-15 14:35) - PLID 54632 - Changed to false
		m_ProviderCombo = BindNxDataList2Ctrl(IDC_MED_PROVIDER_COMBO, false);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_MED_LOCATION_COMBO, true);

		// (r.gonet 2016-02-24 17:01) - PLID 67988 - Cache the license check.
		m_bHasEPrescribingLicense = g_pLicense->CheckForLicense(CLicense::lcePrescribe, CLicense::cflrSilent) ? true : false;
		if (IsPharmacyListADropDown()) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - Renewals still must use the pharmacy combo form
			// since they fill their own results manually.
			m_PharmacyCombo = BindNxDataList2Ctrl(IDC_MED_PHARMACY_COMBO, false);
			PharmacySearchUtils::CreatePharmacyDropDown(m_PharmacyCombo);
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - Selection comes later.
		} else {
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Load the pharmacy combo.
			long nPatientMedicationID;
			if (m_pCurrentPrescription != nullptr && m_pCurrentPrescription->PrescriptionID.length() != 0) {
				nPatientMedicationID = atol(m_pCurrentPrescription->PrescriptionID);
			} else {
				nPatientMedicationID = -1;
			}

			// (r.gonet 2016-02-23 12:47) - PLID 67964 - Pass the favorite icon.
			m_PharmacyCombo = PharmacySearchUtils::BindPharmacySearchListCtrl(this, IDC_MED_PHARMACY_COMBO, GetRemoteDataSnapshot(),
				m_hStarIcon, m_nPatientID, nPatientMedicationID);
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Reflect the current pharmacy selection in the UI.
			UpdateControlsForPharmacySelection(GetSelectedPharmacyID());
		}
		
		// (b.savon 2013-01-15 15:51) - PLID 54632 - Changed to false
		// (a.walling 2009-07-01 11:00) - PLID 34052 - Supervisor and Agent support
		m_AgentCombo = BindNxDataList2Ctrl(IDC_MED_PRESCRIBER_AGENT_COMBO, false);
		m_SupervisorCombo = BindNxDataList2Ctrl(IDC_MED_SUPERVISOR_COMBO, false);

		//TES 2/12/2009 - PLID 33002 - Added DosageForm, changed Unit to a dropdown
		//TES 3/31/2009 - PLID 33750 - Changed DosageForm to a read-only edit box, renamed StrengthUnitCombo to QuantityUnitCombo
		m_pQuantityUnitCombo = BindNxDataList2Ctrl(IDC_QUANTITY_UNIT);

		// (r.farnworth 2016-01-15 12:20) - PLID 67750 - Packaging Info
		m_pPackagingCombo = BindNxDataList2Ctrl(IDC_PACKAGING_INFO, false);
		LoadPackagingInfo();
		
		//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
		m_nxtSampleExpirationDate = BindNxTimeCtrl(this, IDC_SAMPLE_EXPIRATION_DATE);

		// (j.fouts 2013-01-10 10:54) - PLID 54526 - Added Dosage quantity list
		m_pDosageQuantityList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_DOSAGE, true);

		// (j.fouts 2013-02-01 15:09) - PLID 54528 - Added route method list
		m_pRouteList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_METHOD, true);
		// (j.jones 2016-01-06 11:22) - PLID 67823 - added a <Custom Sig> entry which behaves like <None>
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRouteList->GetNewRow();
			pRow->PutValue(drlcID, (long)-1);
			pRow->PutValue(drlcRoute, _bstr_t(" <Custom Sig>"));
			pRow->PutValue(drlcAction, g_cvarNull);
			m_pRouteList->AddRowSorted(pRow, NULL);
		}
				
		// (j.fouts 2013-01-10 10:55) - PLID 54529 - Added dosage frequency list
		m_pFrequencyList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_FREQUENCY, true);

		// (j.fouts 2013-01-10 10:52) - PLID 54463 - Refills is now a datalist
		m_pRefillsList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_REFILLS, false);
		{
			//Fill the list will the values 0 - 12
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			for (long i = 0; i <= 12; i++) {
				pRow = m_pRefillsList->GetNewRow();
				pRow->PutValue(0, i);
				m_pRefillsList->AddRowAtEnd(pRow, NULL);
			}
		}

		//TES 8/22/2013 - PLID 57999 - Load the insurance list for formularies
		m_pInsuranceCombo = BindNxDataList2Ctrl(IDC_FORMULARY_INS_LIST, false);
		// (s.dhole 2013-10-19 16:12) - PLID 59068 remove bold
		//TES 8/22/2013 - PLID 57999 - Formularies
		//GetDlgItem(IDC_FORMULARY_STATUS_TEXT)->SetFont(fBold);
		
		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added a dosage units list
		m_pDosageUnitList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_DOSAGE_UNIT, true);
		// (j.jones 2016-01-06 11:22) - PLID 67823 - added a <Custom Sig> entry which behaves like <None>
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDosageUnitList->GetNewRow();
			pRow->PutValue(dulcID, (long)-1);
			pRow->PutValue(dulcUnit, _bstr_t(" <Custom Sig>"));
			pRow->PutValue(dulcUnitPlural, g_cvarNull);
			m_pDosageUnitList->AddRowSorted(pRow, NULL);
		}

		m_bCachedIsLicensedConfiguredUser = IsLicensedConfiguredUser();

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//add a "no location selected" row
		pRow = m_LocationCombo->GetNewRow();
		pRow->PutValue(locccID, (long)-1);
		pRow->PutValue(locccName, (LPCTSTR)"< No Location Selected >");
		m_LocationCombo->AddRowSorted(pRow, NULL);

		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Removed dead favorite pharmacy toggle code.

		if(m_pCurrentPrescription)
		{
			// (r.farnworth 2016-01-08 15:33) - PLID 58692 - We need to save everything right away if we're resprescribing
			if (m_bRePrescribe)
			{
				SaveAll();
			}

			LoadPrescriptionFields();
		}
		else if (m_pCurrentRenewalRequest)
		{
			OpenRenewalRequest();
		}

		// (r.gonet 2016-03-03 13:45) - PLID 68494 - Reload the formulary information.
		if (bReloadFormularyInformation) {
			ReloadFormularyInformation();
		}

		// (b.savon 2013-02-20 13:10) - PLID 54632 - Inform the user that if they open an Rx written by someone else, they will take
		// ownership of the Rx.  Of course, make this a DontShowMeAgain and only show it when they have nexerx, they are editing an rx,
		// there is a valid prescription loaded, and the status is not sent.
		if( m_lNexERxLicense.HasNexERx() && m_epdrvAction == epdrvEditRx && !m_bReadOnly &&	m_pCurrentPrescription && 
			m_pCurrentPrescription->status != NexTech_Accessor::PrescriptionStatus_eTransmitSuccess &&
			m_pCurrentPrescription->status != NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized &&
			m_pCurrentPrescription->status != NexTech_Accessor::PrescriptionStatus_eTransmitPending && !m_bIsEMRTemplate &&
			IDNO == DontShowMeAgain(this, "\nIf a user that opened an unsent prescription is not the user that wrote the prescription, the author of the \n"
			"prescription will be replaced with the new user. \n\n Are you sure you wish to continue?\n\n", 
			"PrescriptionEditDlg_UserOwnershipPrescription", "Change Prescription Ownership", FALSE, TRUE)) {
			
			// (b.savon 2016-04-19 7:30) - PLID-68647 - they do not want to take ownership, return standard CANCEL value from dialog
			EndDialog((int)epdrvIDCANCEL);
			return TRUE;
		}

		// (b.savon 2013-01-17 08:50) - PLID 54632 - Load the combos and if the current user doesnt have a role
		// load the legacy behavior
		// (b.savon 2013-09-04 16:41) - PLID 58212 - Add a new 'Void' type for Prescriptions
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		if( m_pCurrentRenewalRequest == NULL){
			if(m_pCurrentPrescription && 
				(m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_eTransmitSuccess ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_eTransmitPending ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_Printed ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_eFaxed ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_Legacy ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_Void ||
				m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_DispensedInHouse)){
				m_urtCurrentUserRole = urtNone; // This is only read only.  Setting the user role to none, 
				//will query the author boxes, and assign the load the boxes from the saved rx values.
			}
			LoadNurseStaffCombo();
			LoadSupervisorCombo();
			LoadPrescriberCombo();
			if( m_urtCurrentUserRole == urtNone ){
				LoadPrescriptionPersonsLegacy();
			}
		}

		// (j.jones 2012-11-19 09:45) - PLID 52819 - if told to not show the dialog, end the dialog now
		if(m_bHideDialog) {
			// (b.savon 2013-01-28 10:43) - PLID 51705 - And now optionally editing prescriptions
			//this is not used unless the action was to create a prescription
			//ASSERT(m_epdrvAction == epdrvNewRx);

			EndDialog((int)m_epdrvAction);
			return TRUE;
		}

		//Only enable on prescription
		if(m_pCurrentPrescription)
		{
			m_nxlabelPriorAuthLabel.SetType(dtsHyperlink);
			m_nxlabelRefills.SetType(dtsHyperlink);
		}

		//This will handle hiding, disabling controls for the license, permissions, or any other reason.
		EnforceDisabledControls();

		//(r.wilson 4/8/2013) pl 56117 - Trim leading and trailing spaces of the following text fields for Historial Prescriptions
		if(!m_bReadOnly && m_pCurrentPrescription)
		{			
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));

			TrimSpaces(this, IDC_PRESCRIPTION_EXPLANATION);
			TrimSpaces(this, IDC_NOTE_TO_PHARMACIST);
			TrimSpaces(this, IDC_PRIOR_AUTH);
			TrimSpaces(this, IDC_EDIT_QUANTITY);			
			
			//(r.wilson 4/8/2013) pl 56117 - Set the flags in the pCommit object in order to save the four fields below when the Save(...) function gets called 
			SaveBatchPriorAuthorization(pCommit);
			SaveBatchPharmacistNote(pCommit);
			SaveBatchPatientExplanation(pCommit);
			SaveBatchQuantity(pCommit);
					
			//(r.wilson 4/8/2013) pl 56117 - Only change if one of these fields was updated
			if(pCommit->CommitPriorAuthorization == VARIANT_TRUE ||pCommit->CommitPharmacistNote == VARIANT_TRUE ||
				pCommit->CommitPatientExplanation == TRUE || pCommit->CommitQuantity == VARIANT_TRUE)
			{
				Save(pCommit);
			}
			
		}

	}NxCatchAll("Error in CPrescriptionEditDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//EditPrescription is called instead of DoModal()
// (b.savon 2013-03-19 17:06) - PLID 55477 - Change to take in struct
// (b.savon 2013-01-28 10:44) - PLID 51705 - Added the optional silent dialog flag
// (j.jones 2008-10-13 10:09) - PLID 17235 - this function now requires a patient ID
// (j.jones 2013-11-25 09:15) - PLID 59772 - added the drug interaction struct, if it is filled then
// the Rx dialog needs to show drug interactions immediately upon opening
// (j.jones 2016-01-06 15:29) - PLID 67823 - added bIsNewRx, so the dialog knows whether or not the prescription has just been created
// (r.farnworth 2016-01-08 15:34) - PLID 58692 - Added bRePrescribe
int CPrescriptionEditDlg::EditPrescription(bool bIsNewRx, PrescriptionInfo rxInformation, DrugInteractionInfo *drugInteractionsToShow /*= NULL*/,
										   BOOL bOpenReadOnly /*= FALSE*/, bool bSilentDialog /*= false*/, BOOL bRePrescribe /*=FALSE*/)
{
	try {
		// (r.gonet 2016-02-23 00:53) - PLID 68408 - Remember what type of record we are opening.
		m_eDisplayedRecordType = EDisplayedRecordType::Prescription;

		//Set the current prescription to the passed in value
		m_pCurrentPrescription = rxInformation.pPrescription;
		m_saryPrescribers = rxInformation.saryPrescribers;
		m_sarySupervisors = rxInformation.sarySupervisors;
		m_saryNurseStaff = rxInformation.saryNurseStaff;
		m_urtCurrentUserRole = AccessorERxUserRoleToPracticeEnum(rxInformation.erxUserRole);

		m_bReadOnly = bOpenReadOnly;

		m_bRePrescribe = bRePrescribe;

		// (j.jones 2012-11-16 08:55) - PLID 53765 - track what it was we did in this dialog
		m_epdrvAction = epdrvEditRx;

		// (j.jones 2016-01-06 15:33) - PLID 67823 - track if the prescription is new
		m_bIsNewRx = bIsNewRx;

		// (b.savon 2013-01-28 10:03) - PLID 51705 - Ability to hide this dialog
		m_bHideDialog = bSilentDialog;

		CString strUserDefinedID((LPCTSTR)m_pCurrentPrescription->PatientInformation->PatientUserDefinedID);
		m_nPatientID = GetExistingPatientIDByUserDefinedID(atol(strUserDefinedID));

		// (j.jones 2013-11-25 09:15) - PLID 59772 - added ability to show drug interactions immediately upon opening the dialog
		// (b.savon 2014-01-03 08:18) - PLID 58984 - Only if we have the NexERx License
		if(drugInteractionsToShow != NULL && g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts){
			m_drugInteractionsToShowOnInit = *drugInteractionsToShow;
		}

		return DoModal();

	}NxCatchAll("Error in CPrescriptionEditDlg::EditPrescription");

	return 0;
}

//this overload of DoModal will require that we have the correct information
//needed to open the dialog, incase someone manages to call DoModal inappropriately
int CPrescriptionEditDlg::DoModal()
{
	return CNxDialog::DoModal();
}

//This function will fill all fields with data from m_pCurrentPrescription
// (j.jones 2008-05-20 09:46) - PLID 30079 - renamed this function
// (j.fouts 2013-03-12 16:33) - PLID 52973 - Moved Loading to PrescriptionUtils // (j.fouts 2012-10-10 15:19) - PLID 52973 - Merged LoadNewFromData and LoadExistingFromData into this function
void CPrescriptionEditDlg::LoadPrescriptionFields(bool bCheckQty /*=false*/)
{
	try {
		ASSERT(m_pCurrentPrescription);

		if(!m_pCurrentPrescription->FirstPrescriptionDate->IsNull())
		{
			COleDateTime dtFirstPrescriptionDate(m_pCurrentPrescription->FirstPrescriptionDate->GetValue());

			CString str = FormatDateTimeForInterface(dtFirstPrescriptionDate, NULL, dtoDate);
			SetDlgItemText(IDC_FIRSTDATE, str);
		}
		else {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			SetDlgItemText(IDC_FIRSTDATE, "");
		}

		m_pAutoPopulatingSig.reset(new AutoPopulatingSig);
		// (b.savon 2016-01-19 15:29) - PLID 59879 - Determine based on drug form of the packaging if the packages will be available for editing.
		// This also determines if we are autocalculating quantity field based on dosage, days supply changes.
		NexTech_Accessor::_NexERxMedicationPackageInformationPtr pPackage(__uuidof(NexTech_Accessor::NexERxMedicationPackageInformation));
		Nx::SafeArray<IUnknown*> saryAvailablePackages = m_pCurrentPrescription->AvailablePackages;
		if(saryAvailablePackages.GetCount() > 0){
			pPackage = saryAvailablePackages[0];
			m_pAutoPopulatingSig->SetAutoCalculate(pPackage);
			if (m_pAutoPopulatingSig->GetAutoCalculate()) {
				m_pPackagingCombo->ReadOnly = VARIANT_TRUE;
			}
		}		

		//There is a chance that we will prompt the user to change somethings about this prescription while
		//	we open it. If we do and they change something we will save after opening.
		bool bShouldSave = false;
		NexTech_Accessor::_PrescriptionCommitPtr pCommitPrescription(__uuidof(NexTech_Accessor::PrescriptionCommit));
		NexTech_Accessor::_QueuePrescriptionPtr pQueueRx(__uuidof(NexTech_Accessor::QueuePrescription));
		NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
		pCommitPrescription->Prescription = pQueueRx;
		pCommitPrescription->Prescription->EMRPrescriptionSource = pEMRPrescriptionSource;
		pCommitPrescription->Prescription->PrescriptionID = m_pCurrentPrescription->PrescriptionID;
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommitPrescription->Prescription->EMRPrescriptionSource->IsEMRTemplate = m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate;

		
		// (j.fouts 2012-10-22 11:37) - PLID 53156 - Fill in Patient Information
		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		SetTextAndToolTip(&m_nxPatientName, CString((LPCTSTR)m_pCurrentPrescription->PatientInformation->PatientName));
		SetTextAndToolTip(&m_nxPatientGender, CString((LPCTSTR)m_pCurrentPrescription->PatientInformation->PatientGender));
		SetTextAndToolTip(&m_nxPatientBirthdate, CString((LPCTSTR)m_pCurrentPrescription->PatientInformation->PatientDOB));
		SetTextAndToolTip(&m_nxPatientAddress, FormatAddressForPrescription(VarString(m_pCurrentPrescription->PatientInformation->PatientAddress1), VarString(m_pCurrentPrescription->PatientInformation->PatientAddress2), 
			VarString(m_pCurrentPrescription->PatientInformation->PatientCity), VarString(m_pCurrentPrescription->PatientInformation->PatientState), VarString(m_pCurrentPrescription->PatientInformation->PatientZip)));

		// (j.fouts 2013-01-10 10:54) - PLID 54526 - Select the dosage qty from the list or add it to the list if it is not there
		if(!m_pDosageQuantityList->FindByColumn(dqlcSigText, m_pCurrentPrescription->Dosage->DosageQuantity, NULL, true)
			&& !(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageQuantity).IsEmpty()))
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDosageQuantityList->GetNewRow();
			pRow->PutValue(dqlcID, -1);
			pRow->PutValue(dqlcSigText, m_pCurrentPrescription->Dosage->DosageQuantity);
			pRow->PutValue(dqlcValue, -1.0f);
			m_pDosageQuantityList->CurSel = m_pDosageQuantityList->AddRowAtEnd(pRow, NULL);
		}
		else if(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageQuantity).IsEmpty()) {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pDosageQuantityList->CurSel = NULL;
		}

		if(m_pDosageQuantityList->CurSel)
		{
			m_pAutoPopulatingSig->SetDosageQuantity(
				AsString(m_pDosageQuantityList->CurSel->GetValue(dqlcSigText)),
				(double)VarFloat(m_pDosageQuantityList->CurSel->GetValue(dqlcValue), -1.0f));
		}
		else {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pAutoPopulatingSig->SetDosageQuantity(CString(""), 0);
		}

		// (j.fouts 2013-02-01 15:09) - PLID 54528 - Select the dosage route from the list or add it to the list if it is not there
		m_pRouteList->FindByColumn(drlcID, atol(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageRoute->ID)), NULL, true);
		if(atol(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageRoute->ID)) <= 0) {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pRouteList->CurSel = NULL;
		}

		// (j.jones 2016-01-06 15:20) - PLID 67823 - if editing an existing Rx, select <Custom Sig> instead of blank,
		// but New Rx will be blank
		if (m_pRouteList->CurSel == NULL && !m_bIsNewRx) {
			m_pRouteList->SetSelByColumn(drlcID, (long)-1);
		}

		// (j.jones 2016-01-06 11:39) - PLID 67823 - ensure the <Custom Sig> row isn't used
		if (m_pRouteList->CurSel && VarLong(m_pRouteList->CurSel->GetValue(drlcID), -1) != -1)
		{
			m_pAutoPopulatingSig->SetRoute(AsString(m_pRouteList->CurSel->GetValue(drlcAction)));
		}
		else {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pAutoPopulatingSig->SetRoute(CString(""));
		}

		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Select the dosage unit
		m_pDosageUnitList->FindByColumn(dulcID, atol(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageUnit->ID)), NULL, true);
		if(atol(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageUnit->ID)) <= 0) {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pDosageUnitList->CurSel = NULL;
		}

		// (j.jones 2016-01-06 15:20) - PLID 67823 - if editing an existing Rx, select <Custom Sig> instead of blank,
		// but New Rx will be blank
		if (m_pDosageUnitList->CurSel == NULL && !m_bIsNewRx) {
			m_pDosageUnitList->SetSelByColumn(dulcID, (long)-1);
		}

		// (j.jones 2016-01-06 11:39) - PLID 67823 - ensure the <Custom Sig> row isn't used
		if(m_pDosageUnitList->CurSel && VarLong(m_pDosageUnitList->CurSel->GetValue(dulcID), -1) != -1)
		{
			m_pAutoPopulatingSig->SetUnit(
				AsString(m_pDosageUnitList->CurSel->GetValue(dulcUnit)),
				AsString(m_pDosageUnitList->CurSel->GetValue(dulcUnitPlural)));
		}
		else {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pAutoPopulatingSig->SetUnit(CString(""), CString(""));
		}

		// (j.fouts 2013-01-10 10:55) - PLID 54529 - Select the dosage freq from the list or add it to the list if it is not there
		if(!m_pFrequencyList->FindByColumn(flcSigText, m_pCurrentPrescription->Dosage->DosageFrequency, NULL, true)
			&& !(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageFrequency).IsEmpty()))
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFrequencyList->GetNewRow();
			pRow->PutValue(flcID, -1);
			pRow->PutValue(flcSigText, m_pCurrentPrescription->Dosage->DosageFrequency);
			pRow->PutValue(flcValue, -1.0f);
			m_pFrequencyList->CurSel = m_pFrequencyList->AddRowAtEnd(pRow, NULL);
		}
		else if(CString((LPCTSTR)m_pCurrentPrescription->Dosage->DosageFrequency).IsEmpty()) {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pFrequencyList->CurSel = NULL;
		}

		if(m_pFrequencyList->CurSel)
		{
			m_pAutoPopulatingSig->SetFrequency(
				AsString(m_pFrequencyList->CurSel->GetValue(flcSigText)),
				(double)VarFloat(m_pFrequencyList->CurSel->GetValue(flcValue), -1.0f));
		}
		else {
			//TES 10/10/2013 - PLID 58287 - The field may already be filled
			m_pAutoPopulatingSig->SetFrequency(CString(""), 0);
		}

		if(bCheckQty && !m_bReadOnly)
		{
			// (d.thompson 2009-03-12) - PLID 33482 - If the quantity or qty units are blank, that means they've got
			//	some screwy data, and we really want them to fix it.  Most likely it happened due to the 9100 changes
			//	that moved Unit from free text to a defined field.  Pop up the "correction" dialog, which will update
			//	data and give us back the results.
			// (j.fouts 2013-03-19 12:06) - PLID 55740 - We need to check the value in QuantityUnitID as well as QuantityUnit
			CString strQuantity((LPCTSTR)m_pCurrentPrescription->QuantityValue);
			CString strQuantityUnit((LPCTSTR)m_pCurrentPrescription->QuantityUnit);
			CString strQuantityUnitID((LPCTSTR)m_pCurrentPrescription->QuantityUnitID);

			//We do not want to check this when switching between prescriptions in the queue

			if(strQuantity.IsEmpty() || (strQuantityUnit.IsEmpty() && strQuantityUnitID.IsEmpty())) {
				CCorrectMedicationQuantitiesDlg dlg(this);
				dlg.m_nDrugListID = atol(m_pCurrentPrescription->Medication->DrugListID);
				if(dlg.DoModal() == IDOK) {
					m_pCurrentPrescription->QuantityValue = _bstr_t(dlg.m_strOutputDefQty);
					CString strNewQuantityUnitID;
					strNewQuantityUnitID.Format("%li", dlg.m_nOutputQtyUnitID);
					m_pCurrentPrescription->QuantityUnit = _bstr_t(dlg.m_strOutputQtyUnit);
					m_pCurrentPrescription->QuantityUnitID = _bstr_t(strNewQuantityUnitID);
					bShouldSave = true;
					pCommitPrescription->CommitQuantity = true;
					pCommitPrescription->CommitQuantityUnitID = true;
					pCommitPrescription->Prescription->QuantityValue = m_pCurrentPrescription->QuantityValue;
					pCommitPrescription->Prescription->QuantityUnitID = m_pCurrentPrescription->QuantityUnitID;
				}
				else {
					//We do let them cancel, though we recommend against it.  We won't have anything to update if so
				}
			}
			else {
				//quantity and unit are both filled, there's no need to fix anything
			}
		}

		//All buisness logic should have been taken care of in the API all we have to do it set the Text in the controls
		// (j.fouts 2012-11-27 16:48) - PLID 51889 - Set Note to Pharm text according to the drug type
		switch(m_pCurrentPrescription->Medication->MedicationType)
		{
		case NexTech_Accessor::DrugType_Supply:
			m_nxstaticNoteToPharm.SetWindowText("Supply description - Note to pharmacist (Not shown on bottle):");
			break;
		case NexTech_Accessor::DrugType_Compound:
			m_nxstaticNoteToPharm.SetWindowText("Compound description - Note to pharmacist (Not shown on bottle):");
			break;
		case NexTech_Accessor::DrugType_NDC:
			m_nxstaticNoteToPharm.SetWindowText("Note to pharmacist (Not shown on bottle):");			
			break;
		}

		SetTextAndToolTip(&m_nxstaticMedicationNameLabel, CString((LPCTSTR)m_pCurrentPrescription->Medication->MedicationName));

		// (a.walling 2009-04-06 16:41) - PLID 33870
		// (j.fouts 2013-01-10 10:52) - PLID 54463 - Refills is now a datalist
		if (CString((LPCTSTR)m_pCurrentPrescription->RefillsAllowed).CompareNoCase("As Needed") == 0) {
			ToggleRefillAsNeeded(TRUE);
			m_pRefillsList->CurSel = NULL;
		} else {
			ToggleRefillAsNeeded(FALSE);
			if(CString((LPCTSTR)m_pCurrentPrescription->RefillsAllowed).IsEmpty())
			{
				m_pRefillsList->CurSel = NULL;
			}
			else
			{
				if(!m_pRefillsList->FindByColumn(0, atol(m_pCurrentPrescription->RefillsAllowed), NULL, true))
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRefillsList->GetNewRow();
					pRow->PutValue(0, m_pCurrentPrescription->RefillsAllowed);
					m_pRefillsList->CurSel = m_pRefillsList->AddRowAtEnd(pRow, NULL);
				}
			}
		}

		SetDlgItemText(IDC_NOTE_TO_PHARMACIST, m_pCurrentPrescription->PharmacistNote);

		CString strDosageForm((LPTSTR)m_pCurrentPrescription->Medication->DosageForm);

		SetDlgItemText(IDC_EDIT_QUANTITY, m_pCurrentPrescription->QuantityValue);

		m_pAutoPopulatingSig->SetQuantity(CString((LPCTSTR)m_pCurrentPrescription->QuantityValue));

		// (j.jones 2013-01-08 12:50) - PLID 47303 - ensured this field synced with the member variable
		m_strPatientExplanation = (LPCTSTR)m_pCurrentPrescription->PatientExplanation;
		SetDlgItemText(IDC_PRESCRIPTION_EXPLANATION, m_strPatientExplanation);
		SetDlgItemText(IDC_PRESCRIPTION_ENGLISH_EXPLANATION, GetLatinToEnglishConversion((LPCTSTR)m_pCurrentPrescription->PatientExplanation));

		SetDlgItemText(IDC_STRENGTH, m_pCurrentPrescription->Medication->Strength);
		// (d.thompson 2009-04-02) - PLID 33571
		SetDlgItemText(IDC_STRENGTH_UNIT_DISPLAY, m_pCurrentPrescription->Medication->StrengthUnit);

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		CString strLocationID((LPCTSTR)m_pCurrentPrescription->locationID);
		long nLocationID = strLocationID.IsEmpty() ? -1 : atol(strLocationID);
		long nSel = m_LocationCombo->TrySetSelByColumn_Deprecated(
			locccID, strLocationID.IsEmpty() ? -1 : atol(strLocationID));
		if(nSel == sriNoRow) {
			//maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
			if(!rsLoc->eof) {
				m_nPendingLocationID = nLocationID;
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else {
				//m_LocationCombo->PutCurSel(NULL);
				m_LocationCombo->SetSelByColumn(locccID, (long)-1);
			}
			rsLoc->Close();
		}
		else if(nSel == sriNoRowYet_WillFireEvent) {
			m_nPendingLocationID = nLocationID;
		}

		if(!m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate)
		{
			// (a.wilson 2013-02-05 11:46) - PLID 55014 - If this is a newrx based on a denynewrx response. set the pharmacy for it.
			// (j.fouts 2013-04-22 14:13) - PLID 54719 - PharmacyID is passed in a Pharmacy object
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Load pharmacy current selection in the UI.
			long nPharmacyID = GetSelectedPharmacyID();
			m_nDenyNewRxResponseID = atol(CString((LPCTSTR)m_pCurrentPrescription->GetDenialResponseID()));
			UpdateControlsForPharmacySelection(nPharmacyID);
		}
		else
		{
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Or no selection, in which case we just show the search box.
			UpdateControlsForPharmacySelection(-1);
		}

		// (j.fouts 2013-03-19 12:06) - PLID 55740 - Select by the ID, only use value if this is an old prescription without an ID
		CString strQuantityUnitID((LPCTSTR)m_pCurrentPrescription->QuantityUnitID);
		if(strQuantityUnitID.IsEmpty())
		{
			//This is an old Unit, just insert the string value into the datalist
			CString strQuantityUnit((LPCTSTR)m_pCurrentPrescription->QuantityUnit);
			m_pQuantityUnitCombo->PutComboBoxText(_bstr_t(strQuantityUnit));
			m_strPendingUnit = strQuantityUnit;
		}
		else
		{
			//Select the ID in the list
			m_pQuantityUnitCombo->FindByColumn(qucID, atol(strQuantityUnitID), NULL, VARIANT_TRUE);
		}

		// (r.farnworth 2016-01-15 15:12) - PLID 67750 - Populate the packaging info
		CString strPackagingInfo((LPCTSTR)(m_pCurrentPrescription->SelectedPackage->Size + " " + m_pCurrentPrescription->SelectedPackage->DrugForm + " " + m_pCurrentPrescription->SelectedPackage->description));
		m_pPackagingCombo->PutComboBoxText(_bstr_t(strPackagingInfo.Trim()));
	
		m_editDosageForm.SetWindowText(strDosageForm);

		//TES 2/16/2009 - Added some new fields (which aren't in DrugList) for SureScripts
		if(m_pCurrentPrescription->DaysSupply->IsNull() || VarLong(m_pCurrentPrescription->DaysSupply->GetValue(),-1) == -1) {
			SetDlgItemText(IDC_DAYS_SUPPLY, "");
		}
		else {
			long nValue = VarLong(m_pCurrentPrescription->DaysSupply->GetValue(),-1);
			SetDlgItemInt(IDC_DAYS_SUPPLY, nValue);
			m_pAutoPopulatingSig->SetDaysSupply(FormatString("%li", nValue));
		}
		
		CheckDlgButton(IDC_ALLOW_SUBSTITUTIONS, m_pCurrentPrescription->AllowSubstitutions ? BST_CHECKED : BST_UNCHECKED);

		//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
		if(m_pCurrentPrescription->PriorAuthorizationIsSample)
		{
			m_nxlabelPriorAuthLabel.SetText(SAMPLE_PRESCRIPTION_TEXT);
			m_nxsSampleExpirationLbl.ShowWindow(SW_SHOWNA);
			// (b.savon 2013-04-18 13:57) - PLID 56153
			m_nxstaticSampleInternalLabel.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else
		{
			m_nxlabelPriorAuthLabel.SetText(PRIOR_AUTHORIZATION_TEXT);
			m_nxsSampleExpirationLbl.ShowWindow(SW_HIDE);
			// (b.savon 2013-04-18 13:57) - PLID 56153
			m_nxstaticSampleInternalLabel.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->ShowWindow(SW_HIDE);
		}

		if(!m_pCurrentPrescription->SampleExpirationDate->IsNull())
		{
			//Don't show invalid dates
			COleDateTime dtBad;
			dtBad.ParseDateTime("12/31/1899");

			COleDateTime dtSampleExpirationDate(m_pCurrentPrescription->SampleExpirationDate->GetValue());
			//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
			if(dtSampleExpirationDate.GetStatus() == COleDateTime::valid && dtSampleExpirationDate >= dtBad) {
				m_nxtSampleExpirationDate->SetDateTime(dtSampleExpirationDate);
			}
			else {
				m_nxtSampleExpirationDate->Clear();
			}
		}
		else {
			m_nxtSampleExpirationDate->Clear();
		}

		SetDlgItemText(IDC_PRIOR_AUTH, m_pCurrentPrescription->PriorAuthorization);

		// (j.fouts 2012-10-22 15:01) - PLID 52973 - Properly load the Prescription Date
		if(m_pCurrentPrescription->PrescriptionDate->IsNull())
		{
			COleDateTime dtPrescriptionDate = COleDateTime::GetCurrentTime();
			_variant_t varDate = dtPrescriptionDate;
			m_dtPrescriptionDate.SetValue(varDate);

			if(!m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate) {
				NexTech_Accessor::_NullableDateTimePtr pNullableDateTime(__uuidof(NexTech_Accessor::NullableDateTime));
				pCommitPrescription->Prescription->PrescriptionDate = pNullableDateTime;
				pCommitPrescription->CommitPrescriptionDate  = true;
				pCommitPrescription->Prescription->PrescriptionDate->SetDateTime(dtPrescriptionDate);
				bShouldSave = true;
			}
		}
		else
		{
			_variant_t varDate = m_pCurrentPrescription->PrescriptionDate->GetValue();
			m_dtPrescriptionDate.SetValue(varDate);
		}

		// (b.savon 2016-01-20 10:26) - PLID 67953 - Don't do this -- The sig isn't automatically generated sometimes.
		//m_pAutoPopulatingSig->SetSig(m_strPatientExplanation);

		// (b.savon 2013-09-24 14:52) - PLID 58753 - Return the diagnosis code display in the Prescription object in the API
		// (a.wilson 2014-03-24) PLID 61186 - clean up visual appearance of codes in hyperlink.
		CString strDiagnosisDisplay = (LPCTSTR)m_pCurrentPrescription->DiagnosisCodeDisplay;
		if( strDiagnosisDisplay.IsEmpty() ){
			m_nxsAddDiagnosisCodes.SetText("Add Diagnosis Codes...");
			m_nxsAddDiagnosisCodes.SetToolTip("");
		} else {
			m_nxsAddDiagnosisCodes.SetToolTip(strDiagnosisDisplay);
			if (strDiagnosisDisplay.GetLength() > 50) {
				strDiagnosisDisplay = strDiagnosisDisplay.Left(50);
				strDiagnosisDisplay.Trim();
				strDiagnosisDisplay += ("...");
			}
			m_nxsAddDiagnosisCodes.SetText(strDiagnosisDisplay);
		}

		// (r.gonet 2016-03-03 12:41) - PLID 68494 - Flag indicating whether a "Could Not Process" coverage is in the formulary info.
		bool bCouldNotProcess = false;
		// (b.savon 2013-09-25 10:53) - PLID 58762 - The prescription returned from the API needs to contain the formulary information
		Nx::SafeArray<IUnknown *> saryFormularyInformation = m_pCurrentPrescription->FormularyInformation;
		foreach(NexTech_Accessor::_PrescriptionFormularyInformationPtr formularyInformation, saryFormularyInformation)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsuranceCombo->GetNewRow();
			pRow->PutValue(filcID, AsLong(formularyInformation->detailID));
			pRow->PutValue(filcPatientID, AsLong(formularyInformation->patientID));
			pRow->PutValue(filcInsuranceCoName, formularyInformation->InsuranceCoName);
			pRow->PutValue(filcPBMID, formularyInformation->PBMID);
			pRow->PutValue(filcPBMName, formularyInformation->PBMName);
			pRow->PutValue(filcInsuranceResponsibilityID, GetInsuranceLevelPlacement((LPCTSTR)formularyInformation->InsuranceLevel));
			pRow->PutValue(filcInsuranceResponsibility, AsBstr(GetInsuranceResponsibility((LPCTSTR)formularyInformation->InsuranceLevel)));
			pRow->PutValue(filcSureScriptsFormularyID, formularyInformation->FormularyID);
			pRow->PutValue(filcSureScriptsAlternativesID, formularyInformation->AlternativesID);
			pRow->PutValue(filcSureScriptsCoverageID, formularyInformation->CoverageID);
			pRow->PutValue(filcSureScriptsCopayID, formularyInformation->CopayID);
			pRow->PutValue(filcPlanName, formularyInformation->PlanName);
			pRow->PutValue(filcPharmacyCoverage, formularyInformation->PharmacyCoverage);
			pRow->PutValue(filcLastSureScriptsRequestDate, formularyInformation->SentDateUTC->GetValue());
			pRow->PutValue(filcDemographicsHaveChanged, formularyInformation->DemographicsHaveChanged);
			pRow->PutValue(filcDisplay, AsBstr(
											GetInsuranceDisplay(
												(LPCTSTR)formularyInformation->PBMID, 
												(LPCTSTR)formularyInformation->PBMName, 
												(LPCTSTR)formularyInformation->PlanName,
												(LPCTSTR)formularyInformation->PharmacyCoverage
											)
										)
							);
			// (s.dhole 2013-10-08 16:07) - PLID 58923 Update both flags
			pRow->PutValue(filcIsFailure, AsBool(formularyInformation->IsFailure));
			pRow->PutValue(filcActive, AsBool(formularyInformation->Active));
			pRow->PutValue(filcEligibilityCoverageStatus, AsLong(formularyInformation->Coverage->GetValue()));
			// (s.dhole 2013-10-08 16:07) - PLID 58923 Set back color
			if (AsBool (formularyInformation->IsFailure) !=FALSE)
			{
				pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				pRow->PutBackColorSel(ERX_IMPORTED_OUTOFDATE_COLOR);
				
			}
			else if (AsBool (formularyInformation->Active) ==FALSE)
			{
				pRow->PutBackColor(ERX_NO_RESULTS_COLOR);
				pRow->PutBackColorSel(ERX_NO_RESULTS_COLOR);
			}

			// (r.gonet 2016-03-03 09:50) - PLID 68494 - We were loading coverages with any Coverage status, including Coverage status = 5 (Could Not Process).
			// We have since learned that a coverage with this status is returned in the eligibility response by SureScripts if the patient has
			// no coverages at all. If this gets selected on the prescription (which is does by default if it is the only thing in the list), and
			// we send it in the BenefitCoordination/PayerID and BenefitCoordination/PayerName element in the NewRX message, then SureScripts does not
			// respond with an error. However, they did tell us verbally that we should not be sending PayerID or PayerName in that case.
			// They did not say anything about other Coverage statuses, so we are continuing to allow those to be loaded. We do not allow the user to select
			// a CouldNotProcess row.
			if (formularyInformation->Coverage->IsNull() == VARIANT_FALSE
				&& (EligibilityCoverageStatus)AsLong(formularyInformation->Coverage->GetValue()) == EligibilityCoverageStatus::CouldNotProcess) {
				bCouldNotProcess = true;
				// (r.gonet 2016-03-22 14:59) - PLID 68494 - Hide the invalid coverages instead of removing them.
				pRow->Visible = VARIANT_FALSE;
			}

			m_pInsuranceCombo->AddRowSorted(pRow, NULL);
		}

		// (j.fouts 2013-10-01 09:50) - PLID 58773 - Select the previous ID
		long nInsuranceDetailID = atol(CString((LPCTSTR)m_pCurrentPrescription->InsuranceDetailID));
		long nReplacementInsuranceDetailID = atol(CString((LPCTSTR)m_pCurrentPrescription->ReplacementInsuranceDetailID));

		IRowSettingsPtr pRow = m_pInsuranceCombo->FindByColumn(filcID, nInsuranceDetailID, NULL, VARIANT_FALSE);
		if(pRow == NULL)
		{
			pRow = m_pInsuranceCombo->FindByColumn(filcID, nReplacementInsuranceDetailID, NULL, VARIANT_FALSE);

			if(pRow == NULL)
			{
				nReplacementInsuranceDetailID = -1;
				pCommitPrescription->Prescription->InsuranceDetailID = _bstr_t("-1");
			}
			else
			{
				pCommitPrescription->Prescription->InsuranceDetailID = m_pCurrentPrescription->ReplacementInsuranceDetailID;
			}

			if(nInsuranceDetailID != nReplacementInsuranceDetailID)
			{
				nInsuranceDetailID = nReplacementInsuranceDetailID;
				pCommitPrescription->CommitInsuranceDetailID = VARIANT_TRUE;
				bShouldSave = true;
			}
		}

		long nRealCoverages = 0;
		NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_pInsuranceCombo->GetFirstRow();
		while (pRowIter != nullptr) {
			if (VarLong(pRowIter->GetValue(filcEligibilityCoverageStatus), -1) != (long)EligibilityCoverageStatus::CouldNotProcess) {
				nRealCoverages++;
			}
			pRowIter = pRowIter->GetNextRow();
		}

		if (pRow != nullptr && VarLong(pRow->GetValue(filcEligibilityCoverageStatus), -1) != (long)EligibilityCoverageStatus::CouldNotProcess) {
			m_pInsuranceCombo->CurSel = pRow;
		}
		
		SelChosenFormularyInsList(m_pInsuranceCombo->CurSel);

		//TES 9/25/2013 - PLID 57999 - Give a little extra info if there are no eligible plans
		if(nRealCoverages == 0) {
			if (!bCouldNotProcess) {
				m_pInsuranceCombo->ComboBoxText = _bstr_t("N/A; click Coverage... to request");
			} else {
				// (r.gonet 2016-03-03 09:50) - PLID 68494 - Little more detail if the request was made and succeeded but there are no
				// valid coverages.
				m_pInsuranceCombo->ComboBoxText = _bstr_t("No Coverages Found");
			}
		}
		else if(m_pInsuranceCombo->IsComboBoxTextInUse) {
			m_pInsuranceCombo->ComboBoxText = _bstr_t("");
		}

		//TES 10/3/2013 - PLID 58287 - Don't link to the formulary information if this is a non-FDB drug
		if(m_nFDBID == -1) {
			m_pInsuranceCombo->ReadOnly = VARIANT_TRUE;
			// (s.dhole 2013-10-19 11:08) - PLID 59084 
			m_btnFCovarage.EnableWindow(FALSE);
		}

		if(bShouldSave)
		{
			Save(pCommitPrescription);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-09-25 10:53) - PLID 58762 - The prescription returned from the API needs to contain the formulary information
long CPrescriptionEditDlg::GetInsuranceLevelPlacement(const CString &strInsuranceLevel)
{
	if( strInsuranceLevel.CompareNoCase("Primary Payer") == 0 ){
		return 1;
	}else if( strInsuranceLevel.CompareNoCase("Secondary Payer") == 0 ){
		return 2;
	}else if( strInsuranceLevel.CompareNoCase("Tertiary Payer") == 0 ){
		return 3;
	}else{
		return 4;
	}
}

// (b.savon 2013-09-25 10:53) - PLID 58762 - The prescription returned from the API needs to contain the formulary information
CString CPrescriptionEditDlg::GetInsuranceResponsibility(const CString &strInsuranceLevel)
{
	if( strInsuranceLevel.CompareNoCase("Primary Payer") == 0 ){
		return "Primary";
	}else if( strInsuranceLevel.CompareNoCase("Secondary Payer") == 0 ){
		return "Secondary";
	}else if( strInsuranceLevel.CompareNoCase("Tertiary Payer") == 0 ){
		return "Tertiary";
	}else{
		return strInsuranceLevel;
	}
}

// (b.savon 2013-09-25 10:53) - PLID 58762 - The prescription returned from the API needs to contain the formulary information
CString CPrescriptionEditDlg::GetInsuranceDisplay(const CString &strPBMID, const CString &strPBMName, const CString &strPlanName, const CString &strPharmacyCoverage)
{
	CString strDisplay;
	if( !strPBMID.IsEmpty() ){
		strDisplay += "PBM: " + strPBMName + "; ";
	}

	if( !strPlanName.IsEmpty() ){
		strDisplay += "Plan: " + strPlanName + "; ";
	}

	if( !strPharmacyCoverage.IsEmpty() ){
		strDisplay += "Pharmacy Coverage: " + strPharmacyCoverage;
	}

	return strDisplay;
}

//save the prescription
// (j.fouts 2012-10-17 10:22) - PLID 53146 - Modified to call the API with a PrescriptionCommit
BOOL CPrescriptionEditDlg::Save(NexTech_Accessor::_PrescriptionCommitPtr pPrescription)
{
	try
	{
		//Don't save any read only rx's
		if( m_bReadOnly ){
			return FALSE;
		}
		// (j.fouts 2012-11-07 16:23) - PLID 53566 - We have edited the prescription
		m_bChangesMade = true;

		CArray<NexTech_Accessor::_PrescriptionCommitPtr,NexTech_Accessor::_PrescriptionCommitPtr> aryPrescriptions;

		aryPrescriptions.Add(pPrescription);

		Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);
		// (j.fouts 2012-10-25 16:22) - PLID 53396 - The english description may changed with the save so update the text box
		NexTech_Accessor::_SavePrescriptionResultArrayPtr pResults = GetAPI()->SavePrescription(GetAPISubkey(), GetAPILoginToken(),
			saryPrescriptions);

		Nx::SafeArray<IUnknown *> saryResults = pResults->Results;
		
		foreach(NexTech_Accessor::_SavePrescriptionResultPtr pResult, saryResults){
			// (s.dhole 2013-02-11 16:49) - PLID 54063
			m_pCurrentPrescription->ValidationList = pResult->ValidationList; 
			m_pCurrentPrescription->EnglishDescription = pResult->EnglishDescription;
			SetDlgItemText(IDC_PRESCRIPTION_ENGLISH_EXPLANATION,pResult->EnglishDescription);
			if( pResult->ValidationList->ErrorStatus == NexTech_Accessor::eRxErrorStatus_eRxError ){
				m_epdrvAction = epdrvErrorRx;
			}
		}

		//The prescription has changed we will need to enable/disable controls
		TryEnablePendingControls();
		 	
		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

//this function will fill the 'first prescription date' field, based on m_nMedicationID
void CPrescriptionEditDlg::FillFirstPrescriptionDate()
{
	try {
		if(!m_pCurrentPrescription)
		{
			return;
		}

		// (j.fouts 2012-10-10 16:14) - PLID 52973 - Use the correct MedicationID
		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		long nMedicationID = AsLong(m_pCurrentPrescription->Medication->DrugListID);

		// (c.haag 2009-01-27 16:21) - PLID 32868 - EMR templates don't have first prescription dates
		if (m_bIsEMRTemplate) {
			return;
		}
		else if(nMedicationID == -1) {
			//shouldn't be possible
			ASSERT(FALSE);
			SetDlgItemText(IDC_FIRSTDATE, "");
			return;
		}

		//find the first date not counting the current prescription
		// (j.fouts 2013-02-08 09:06) - PLID 51712 - Changed API IDs to be string
		_RecordsetPtr rsFirstDate = CreateParamRecordset("SELECT TOP 1 PatientMedications.PrescriptionDate "
				"FROM PatientMedications "
				"WHERE PatientMedications.PatientID = {INT} "
				"AND PatientMedications.MedicationID = {INT} "
				"AND PatientMedications.ID <> {INT} "
				"AND PatientMedications.Deleted = 0 "
				"ORDER BY PatientMedications.PrescriptionDate", m_nPatientID, nMedicationID, AsLong(m_pCurrentPrescription->PrescriptionID));
		
		COleDateTime dtFirstDate;
		dtFirstDate.m_status = COleDateTime::invalid;

		if(!rsFirstDate->eof) {
			dtFirstDate = AdoFldDateTime(rsFirstDate, "PrescriptionDate");
		}
		rsFirstDate->Close();

		COleDateTime dtPrescriptionDate = VarDateTime(m_dtPrescriptionDate.GetValue());

		if(dtFirstDate.m_status == COleDateTime::invalid && dtPrescriptionDate.m_status == COleDateTime::invalid) {
			
			//no valid first date
			SetDlgItemText(IDC_FIRSTDATE, "");
		}
		else if(dtFirstDate.m_status == COleDateTime::invalid) {
			if(dtPrescriptionDate.m_status != COleDateTime::invalid) {
				//it is the only prescription, show the current date
				CString str = FormatDateTimeForInterface(dtPrescriptionDate, NULL, dtoDate);
				SetDlgItemText(IDC_FIRSTDATE, str);
			}
			else {
				//no valid first date
				SetDlgItemText(IDC_FIRSTDATE, "");
			}
		}
		else if(dtFirstDate <= dtPrescriptionDate) {
			//show the first date from data
			CString str = FormatDateTimeForInterface(dtFirstDate, NULL, dtoDate);
			SetDlgItemText(IDC_FIRSTDATE, str);
		}
		else {
			//show the current date
			CString str = FormatDateTimeForInterface(dtPrescriptionDate, NULL, dtoDate);
			SetDlgItemText(IDC_FIRSTDATE, str);
		}
		
	}NxCatchAll("Error in CPrescriptionEditDlg::FillFirstPrescriptionDate");
}

//hides the english description and makes the regular description wider
void CPrescriptionEditDlg::HideEnglishDescriptionControls()
{
	try {

		CRect rcEnglishExplanation;
		GetDlgItem(IDC_PRESCRIPTION_ENGLISH_EXPLANATION)->GetWindowRect(&rcEnglishExplanation);
		ScreenToClient(&rcEnglishExplanation);

		CRect rcExplanation;
		GetDlgItem(IDC_PRESCRIPTION_EXPLANATION)->GetWindowRect(&rcExplanation);
		ScreenToClient(&rcExplanation);

		//expand the width of the regular description
		rcExplanation.right = rcEnglishExplanation.right;
		GetDlgItem(IDC_PRESCRIPTION_EXPLANATION)->MoveWindow(&rcExplanation);

		//now hide the english description
		GetDlgItem(IDC_ENGLISH_EXPLANATION_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PRESCRIPTION_ENGLISH_EXPLANATION)->ShowWindow(SW_HIDE);

	}NxCatchAll("Error in CPrescriptionEditDlg::HideEnglishDescriptionControls");
}

//hides the todo controls, they are only shown on new prescriptions
// (j.fouts 2012-10-02 08:55) - PLID 52907 - Added a flag to show rather than hide, also disabled when hidden
void CPrescriptionEditDlg::HideToDoReminderControls(BOOL bShow/* = FALSE*/)
{
	try {
		GetDlgItem(IDC_STATIC_VISIT_TESTS_NOTE)->ShowWindow(bShow? SW_SHOWNA : SW_HIDE);
		GetDlgItem(IDC_STATIC_VISIT_TESTS_NOTE)->EnableWindow(bShow);
		m_btnCreateTodo.ShowWindow(bShow? SW_SHOWNA : SW_HIDE);
		m_btnCreateTodo.EnableWindow(bShow);
		GetDlgItem(IDC_REMINDER_DATE)->ShowWindow(bShow? SW_SHOWNA : SW_HIDE);
		GetDlgItem(IDC_REMINDER_DATE)->EnableWindow(bShow);

	}NxCatchAll("Error in CPrescriptionEditDlg::HideToDoReminderControls");
}

// (j.fouts 2013-05-01 09:31) - PLID 52907
//Disables all controls that are representative of a prescription,
//This does not inculde the Send, Print, Fax, Delete, Close buttons
void CPrescriptionEditDlg::DisablePrescriptionControls(BOOL bEnable/* = FALSE*/)
{
		GetDlgItem(IDC_PRESCRIPTION_DATE)->EnableWindow(bEnable);
		m_editPrescriptionExplanation.SetReadOnly(!bEnable);
		// (j.fouts 2013-01-10 11:01) - PLID 54463 - Refills is now a datalist
		// (r.gonet 2016-02-24 17:01) - PLID 67988 - Although we never call this function with TRUE passed,
		// it wouldn't have worked due to the cast of BOOL to VARIANT_BOOL.
		m_pRefillsList->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_editQuantity.SetReadOnly(!bEnable);
		m_ProviderCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_LocationCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;

		if (IsPharmacyListASearchList()) {
			// (r.gonet 2016-02-24 17:01) - PLID 67988 - Disable the search list. Simply making it read only
			// would not take away the ability to put a blinking cursor in the search edit box.
			m_PharmacyCombo->Enabled = bEnable ? VARIANT_TRUE : VARIANT_FALSE;
			m_PharmacyCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
			if (!bEnable && GetSelectedPharmacyID() == -1) {
				GetDlgItem(IDC_MED_PHARMACY_COMBO)->ShowWindow(SW_HIDE);
				m_nxlPharmacySelection.ShowWindow(SW_SHOW);
				m_nxlPharmacySelection.SetText("< No Pharmacy Selected >");
				m_nxlPharmacySelection.SetType(dtsDisabledHyperlink);
			} else {
				UpdateControlsForPharmacySelection(GetSelectedPharmacyID());
			}
		} else {
			// (r.gonet 2016-02-24 17:05) - PLID 68408 - Make the pharmacy combo read only. You can still click into
			// it to see the dropdown's columns but nothing can be selected.
			m_PharmacyCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		}

		m_SupervisorCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_AgentCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_pQuantityUnitCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_pPackagingCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE; // (r.farnworth 2016-01-18 08:24) - PLID 67750
		m_editDaysSupply.SetReadOnly(!bEnable);
		m_checkAllowSubstitutions.EnableWindow(bEnable);
		m_nxlabelPriorAuthLabel.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);
		m_nxlabelRefills.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);
		m_editPriorAuthorization.SetReadOnly(!bEnable);
		m_editNoteToPharmacist.SetReadOnly(!bEnable);
		
		GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->EnableWindow(bEnable);
		m_btnCreateTodo.EnableWindow(bEnable);
		m_dtReminderDate.EnableWindow(bEnable);
		// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
		//m_nxlabelDosage.SetType(bEnable && dtsDisabledHyperlink);
		// (j.fouts 2013-01-04 11:25) - PLID 54447 - Added a control to save the default	
		m_checkSaveAsDefault.EnableWindow(bEnable);
		// (j.fouts 2013-01-04 11:24) - PLID 54448 - Added a control to save to the favorites list
		m_checkAddToFavorites.EnableWindow(bEnable);
		// (j.fouts 2013-01-10 10:54) - PLID 54526 - Added Dosage quantity list
		m_pDosageQuantityList->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		// (j.fouts 2013-02-01 15:09) - PLID 54528 - Added dosage route list
		m_pRouteList->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		// (j.fouts 2013-01-10 10:55) - PLID 54529 - Added dosage frequency list
		m_pFrequencyList->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workarond, checkboxes text are now labels
		m_nxsAllowSubsText.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);
		m_nxsSaveAsDefaultText.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);
		m_nxsSaveToQuickListText.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);
		m_pDosageUnitList->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;

		// (b.savon 2013-09-24 09:45) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
		m_nxsAddDiagnosisCodes.SetType(bEnable ? dtsHyperlink : dtsDisabledHyperlink);

		m_nxlabelDosage.ShowWindow(bEnable? SW_SHOWNA : SW_HIDE);
		m_nxlabelDosage.SetType(bEnable? dtsHyperlink : dtsDisabledHyperlink);	

		//TES 8/22/2013 - PLID 57999 - Formularies
		m_pInsuranceCombo->ReadOnly = !bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		// (s.dhole 2013-10-19 11:08) - PLID 59084 
		m_btnFCovarage.EnableWindow(bEnable);
}

//disables all the controls on the screen, execpt for the Close button
void CPrescriptionEditDlg::DisableControls(BOOL bEnable/* = FALSE*/)
{
	try {
		DisablePrescriptionControls(bEnable);
		// (r.farnworth 2013-01-29) PLID 54667 Added button disables for following an e-Fax
		m_btnEFax.ShowWindow(bEnable? SW_SHOWNA : SW_HIDE);
		m_btnESubmit.ShowWindow(bEnable? SW_SHOWNA : SW_HIDE);
		m_btnPreview.ShowWindow(bEnable? SW_SHOWNA : SW_HIDE);
		m_btnEFax.EnableWindow(bEnable);
		m_btnPreview.EnableWindow(bEnable);
		// (s.dhole 2013-02-11 16:47) - PLID 54063
		m_btnESubmit.EnableWindow(bEnable);
	}NxCatchAll("Error in CPrescriptionEditDlg::DisableControls");	
}

BEGIN_EVENTSINK_MAP(CPrescriptionEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPrescriptionEditDlg)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_LOCATION_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedMedLocationCombo, VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedMedPharmacyCombo, VTS_I4 VTS_I4)	
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 16, OnSelChosenMedPharmacyCombo, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_PACKAGING_INFO, 16, OnSelChosenPackagingInfo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CPrescriptionEditDlg, IDC_QUANTITY_UNIT, 20, CPrescriptionEditDlg::OnTrySetSelFinishedQuantityUnit, VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_LOCATION_COMBO, 16, CPrescriptionEditDlg::SelChosenMedLocationCombo, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 1, CPrescriptionEditDlg::SelChangingMedPharmacyCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_DOSAGE, 16, CPrescriptionEditDlg::SelChosenPrescriptionDosage, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_SAMPLE_EXPIRATION_DATE, 1, CPrescriptionEditDlg::KillFocusSampleExpirationDate, VTS_NONE)
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_QUANTITY, 16, CPrescriptionEditDlg::SelChosenPrescriptionQuantity, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_METHOD, 16, CPrescriptionEditDlg::SelChosenPrescriptionMethod, VTS_DISPATCH)	
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_FREQUENCY, 16, CPrescriptionEditDlg::SelChosenPrescriptionFrequency, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_QUANTITY_UNIT, 16, CPrescriptionEditDlg::SelChosenQuantityUnit, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PROVIDER_COMBO, 16, CPrescriptionEditDlg::SelChosenMedProviderCombo, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_SUPERVISOR_COMBO, 16, CPrescriptionEditDlg::SelChosenMedSupervisorCombo, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PRESCRIBER_AGENT_COMBO, 16, CPrescriptionEditDlg::SelChosenMedPrescriberAgentCombo, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_REFILLS, 16, CPrescriptionEditDlg::SelChosenPrescriptionRefills, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_PRESCRIPTION_DOSAGE_UNIT, 16, CPrescriptionEditDlg::SelChosenPrescriptionDosageUnit, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_QUANTITY_UNIT, 1, CPrescriptionEditDlg::RevertSelChangingOnNull, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PRESCRIBER_AGENT_COMBO, 1, CPrescriptionEditDlg::RevertSelChangingOnNull, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_SUPERVISOR_COMBO, 1, CPrescriptionEditDlg::RevertSelChangingOnNull, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PROVIDER_COMBO, 1, CPrescriptionEditDlg::RevertSelChangingOnNull, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_LOCATION_COMBO, 1, CPrescriptionEditDlg::RevertSelChangingOnNull, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_FORMULARY_INS_LIST, 16, CPrescriptionEditDlg::SelChosenFormularyInsList, VTS_DISPATCH)
	ON_EVENT(CPrescriptionEditDlg, IDC_FORMULARY_INS_LIST, 18, CPrescriptionEditDlg::RequeryFinishedFormularyInsList, VTS_I2)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 34, CPrescriptionEditDlg::ClosedUpExMedPharmacyCombo, VTS_DISPATCH VTS_I2)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 35, CPrescriptionEditDlg::RequestListDataStartingMedPharmacyCombo, VTS_NONE)
	ON_EVENT(CPrescriptionEditDlg, IDC_MED_PHARMACY_COMBO, 36, CPrescriptionEditDlg::RequestListDataFinishedMedPharmacyCombo, VTS_NONE)
END_EVENTSINK_MAP()


void CPrescriptionEditDlg::OnTrySetSelFinishedMedLocationCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingLocationID);
			if(!rsLoc->eof) {
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else {
				m_LocationCombo->PutCurSel(NULL);
			}
			rsLoc->Close();
		}
		else {
			m_nPendingLocationID = -1;
		}

		TryEnablePendingControls();

	}NxCatchAll("Error in CPrescriptionEditDlg::OnTrySetSelFinishedMedLocationCombo");
}

void CPrescriptionEditDlg::OnTrySetSelFinishedMedPharmacyCombo(long nRowEnum, long nFlags) 
{
	try {
		if (IsPharmacyListASearchList()) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - Prescriptions use a search list rather than a combo.
			return;
		}

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsPharm = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingPharmacyID);
			if(!rsPharm->eof) {
				m_PharmacyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPharm, "Name", "")));
			}
			else {
				m_PharmacyCombo->PutCurSel(NULL);
			}
			rsPharm->Close();
		}
		else {
			m_nPendingPharmacyID = -1;
		}
		
		TryEnablePendingControls();

	}NxCatchAll("Error in CPrescriptionEditDlg::OnTrySetSelFinishedMedPharmacyCombo");
}

void CPrescriptionEditDlg::OnBtnPreviewPrescription() 
{
	try {

		if(!GetWPManager()->CheckWordProcessorInstalled() || !m_pCurrentPrescription) {
			return;
		}

		// (b.savon 2014-12-03 09:39) - PLID 64142 - Display a message to the user if they try to merge a NewCrop prescription to Word from within Nextech
		if (!CString((LPCTSTR)m_pCurrentPrescription->NewCropGUID).IsEmpty()){
			MessageBox("This prescription was written in NewCrop and may not be printed from Nextech.  If you wish to print the NewCrop prescription, please do so from NewCrop.", "Nextech", MB_ICONEXCLAMATION);
			return;
		}

		//(r.farnworth 2013-01-24) PLID 54667 Moved to globalutils
		CString strTemplateName, strTemplatePath;
		
		if(CheckPrescriptionTemplates(strTemplateName, strTemplatePath, *this) == true)
		{
			// (j.fouts 2013-04-23 10:53) - PLID 52907 - Warn when printing
			switch(m_pCurrentPrescription->status)
			{
				case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
				case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
				case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
					if(MessageBox("Printing a prescription that has been sent electronically is not recommended. "
						"Please be sure not to give a patient a printed prescription as well as an electronic copy.\n\r"
						"Are you sure you wish to continue printing this prescription?"
						, "Continue Printing?", MB_ICONQUESTION|MB_YESNO) == IDNO)
					{
						return;
					}
					break;
				case NexTech_Accessor::PrescriptionStatus_Printed:
					if(MessageBox("Printing a prescription that has been printed is not recommended. "
						"Please be sure not to give a patient a duplicate copy of the prescription.\n\r"
						"Are you sure you wish to continue printing this prescription?"
						, "Continue Printing?", MB_ICONQUESTION|MB_YESNO) == IDNO)
					{
						return;
					}
					break;
				case NexTech_Accessor::PrescriptionStatus_eFaxed:
					if(MessageBox("Printing a prescription that has been eFaxed is not recommended. "
						"Please be sure not to give a patient a printed prescription as well as a faxed copy.\n\r"
						"Are you sure you wish to continue printing this prescription?"
						, "Continue Printing?", MB_ICONQUESTION|MB_YESNO) == IDNO)
					{
						return;
					}
					break;
			}

			if(CString((LPCTSTR)m_pCurrentPrescription->NewCropGUID).IsEmpty())
			{
				if(DontShowMeAgain(this, "If you continue printing this prescription you will no longer be able "
					"to edit or delete this prescription.\n\r"
					"Are you sure you wish to continue printing this prescription?"
					, "NexERx_PrintLockWarning", "Warning", FALSE, TRUE) == IDNO)
				{
					return;
				}
			}

			// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
			if(!PromptInteractions())
			{
				return;
			}

			// (j.fouts 2012-11-07 16:23) - PLID 53566 - We have edited the prescription
			m_bChangesMade = true;

			//now merge

			CArray<long, long> aryPrescriptionIDs;
			// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
			aryPrescriptionIDs.Add(AsLong(m_pCurrentPrescription->PrescriptionID));
			// (j.fouts 2013-03-05 08:34) - PLID 55427 - Removed PatientID paramater
			if(MergePrescriptionsToWord(aryPrescriptionIDs, strTemplateName, strTemplatePath))
			{
				// (j.fouts 2013-04-26 09:30) - PLID 53146 - Save the Printed Status
				CArray<_bstr_t,_bstr_t> aryPrescriptionIDString;
				aryPrescriptionIDString.Add(_bstr_t(m_pCurrentPrescription->PrescriptionID));

				Nx::SafeArray<BSTR> saryPrescriptionIDs = Nx::SafeArray<BSTR>::From(aryPrescriptionIDString);
				NexTech_Accessor::_UpdatePrescriptionStatusOutputPtr pOutput = GetAPI()->UpdatePrescriptionStatus(GetAPISubkey(), GetAPILoginToken(),
					saryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Printed);

				Nx::SafeArray<IUnknown *> saryResults(pOutput->Results);
				foreach(NexTech_Accessor::_UpdatePrescriptionStatusResultPtr pResult, saryResults)
				{
					//Update the status on this prescription
					m_pCurrentPrescription->status = pResult->NewPrescriptionQueueStatus;
					break;
				}

				OnCancel();
			}
		} else {
			MsgBox(MB_OK|MB_ICONINFORMATION, 
			"Merging to template failed. Aborting operation");
		return;
		}

	}NxCatchAll("Error in CPrescriptionEditDlg::OnBtnPreviewPrescription");
}

void CPrescriptionEditDlg::OnChangePrescriptionDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		
		// (a.walling 2009-11-18 16:30) - PLID 36352 - Warn if this is not acceptable
		VerifyPrescriptionDate(true);

		//if the date is valid, update the 'first prescription date'
		COleDateTime dtPrescriptionDate = VarDateTime(m_dtPrescriptionDate.GetValue());

		if(dtPrescriptionDate.m_status != COleDateTime::invalid) {
			FillFirstPrescriptionDate();
		}

		// (j.fouts 2012-10-17 10:25) - PLID 53146 - Save the changes
		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitPrescriptionDate = VARIANT_TRUE;

		if(dtPrescriptionDate.m_status == COleDateTime::valid)
		{
			m_pCurrentPrescription->PrescriptionDate->SetDateTime(dtPrescriptionDate);
		}
		else
		{
			m_pCurrentPrescription->PrescriptionDate->SetNull();
		}

		Save(pCommit);

	}NxCatchAll("Error in CPrescriptionEditDlg::OnChangePrescriptionDate");	
	
	*pResult = 0;
}

void CPrescriptionEditDlg::OnCloseUpPrescriptionDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {

		//if the date is valid, update the 'first prescription date'
		COleDateTime dtPrescriptionDate = VarDateTime(m_dtPrescriptionDate.GetValue());

		if(dtPrescriptionDate.m_status != COleDateTime::invalid) {
			FillFirstPrescriptionDate();
		}

	}NxCatchAll("Error in CPrescriptionEditDlg::OnCloseUpPrescriptionDate");	

	*pResult = 0;
}

// (a.walling 2009-11-18 16:30) - PLID 36352 - Verify that a date change is allowed
bool CPrescriptionEditDlg::VerifyPrescriptionDate(bool bMessageBox, OPTIONAL OUT CString* pstrMessage)
{
	// (j.fouts 2013-05-13 13:46) - PLID 56645 - I am removing this becuase we use our ERx validation
	//to determine when a date is valid/invalid. We have also restricted the back date range to 7 days
	//as instructed by SureScripts.
	return true;

	//if (!SureScripts::IsEnabled()) {
	//	return true;
	//}

	//// 7 days / 1 week by default
	//long nDayRange = GetRemotePropertyInt("ePrescribeBackDateTolerance", 7, 0,"<None>",true);

	//COleDateTime dtPrescriptionDate = VarDateTime(m_dtPrescriptionDate.GetValue());
	//COleDateTime dtCurrent = COleDateTime::GetCurrentTime();

	//COleDateTimeSpan dts = dtCurrent - dtPrescriptionDate; // will be negative if in the future!

	//CString strMessage;
	//if (dts.GetTotalDays() > nDayRange) {
	//	if (nDayRange > 0) {
	//		strMessage.Format("Electronic prescriptions may not be dated further than %li day%sbefore the current date.", 
	//			nDayRange, nDayRange > 1 ? "s " : " ");
	//		if (bMessageBox) {
	//			MessageBox(CString("Warning: ") + strMessage, NULL, MB_ICONEXCLAMATION);
	//		}
	//	} else {
	//		strMessage.Format("Electronic prescriptions may not be backdated."); 
	//		if (bMessageBox) {
	//			MessageBox(CString("Warning: ") + strMessage, NULL, MB_ICONEXCLAMATION);
	//		}
	//	}

	//	if (pstrMessage) {
	//		*pstrMessage = strMessage;
	//	}

	//	return false;
	//} else {
	//	return true;
	//}
}

// (j.jones 2008-05-20 11:41) - PLID 30079 - used to hide controls based on EMR usage
// (j.fouts 2012-10-02 10:16) - PLID 52907 - Addded a flag so you can hide emr controls and display the others
void CPrescriptionEditDlg::DisplayControlsForEMR(BOOL bShow/* = TRUE*/)
{
	try {
		//Hide all non emr controls if we were told to show emr
		BOOL bHideNonEMR = bShow;
		//hide all non template controls if we were told to show emr and the template flag is set
		BOOL bHideNonTemplate = bShow && m_bIsEMRTemplate;

		//hide the preview button
		if(bHideNonEMR)
		{
			GetDlgItem(IDC_FAX_BUTTON)->ShowWindow(SW_HIDE);
		}

		//hide the prescription date and the "first prescribed date"			
		GetDlgItem(IDC_DATE_LABEL)->ShowWindow(bHideNonTemplate? SW_HIDE : SW_SHOWNA);
		GetDlgItem(IDC_PRESCRIPTION_DATE)->ShowWindow(bHideNonTemplate? SW_HIDE : SW_SHOWNA);
		GetDlgItem(IDC_FIRST_DATE_LABEL)->ShowWindow(bHideNonTemplate? SW_HIDE : SW_SHOWNA);
		GetDlgItem(IDC_FIRSTDATE)->ShowWindow(bHideNonTemplate? SW_HIDE : SW_SHOWNA);

		//disable the provider, location, and pharmacy controls
		m_ProviderCombo->Enabled = !bHideNonTemplate;
		m_LocationCombo->Enabled = !bHideNonTemplate;
		m_PharmacyCombo->Enabled = !bHideNonTemplate;
		m_SupervisorCombo->Enabled = !bHideNonTemplate;
		m_AgentCombo->Enabled = !bHideNonTemplate;
		
		// (b.savon 2013-09-24 09:59) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
		m_nxsAddDiagnosisCodes.EnableWindow(!bHideNonTemplate);
		GetDlgItem(IDC_STATIC_DIAGNOSIS_CODE_GROUP)->EnableWindow(!bHideNonTemplate);

		// (b.savon 2013-01-16 11:26) - PLID 54632 - Disable if they dont have a user role
		m_btnESubmit.EnableWindow(!IsSubmitButtonEnabled() ? FALSE : !bHideNonTemplate);
		m_btnEFax.EnableWindow(!m_bCachedIsLicensedConfiguredUser ? FALSE : !bHideNonTemplate);
		//TES 2/17/2009 - PLID 33140 - Disable the prior authorization controls
		m_nxlabelPriorAuthLabel.SetType(bHideNonTemplate? dtsDisabledHyperlink : dtsHyperlink);

		GetDlgItem(IDC_PRIOR_AUTH)->EnableWindow(!bHideNonTemplate);

		//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
		GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->EnableWindow(!bHideNonTemplate);

		// (a.walling 2009-04-06 16:43) - PLID 33870
		m_nxlabelRefills.SetType(bHideNonTemplate? dtsDisabledHyperlink : dtsHyperlink);

		if(bHideNonEMR)
		{
			//hide the todo reminder controls
			HideToDoReminderControls();

			// (a.walling 2009-04-23 17:42) - PLID 39948
			//m_btnESubmit.EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CPrescriptionEditDlg::DisplayControlsForEMR");	
}

// (j.jones 2008-05-20 15:35) - PLID 30079 - this function will create a to-do alarm if the controls say so
void CPrescriptionEditDlg::CreateTodoAlarm(COleDateTime dtReminderDate)
{
	try {
		if (m_pCurrentPrescription) {
			if(m_nPatientID == -1) {
				ASSERT(FALSE);
				return;
			}
		}

		//to-do reminder code copied from MedicationsDlg and modified slightly
			
		// (m.hancock 2006-10-26 15:45) - PLID 21730 - We added the medication successfully, now we need to create the
		// reminder To Do.  So, first check if we should create the reminder, then create the reminder based on 
		// the properties specificed.
		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to create a todo alarm
		/*
		if(CheckCurrentUserPermissions(bioPatient, sptWrite)) {
		*/

		//Create the reminder
		CTaskEditDlg dlg(this);
		dlg.m_nPersonID = m_nPatientID; // (a.walling 2008-07-07 17:54) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]

		// (m.hancock 2006-11-27 13:31) - PLID 21730 - Set the reminder date and deadline date
		dlg.SetDeadlineDate(dtReminderDate);
		dlg.SetReminderDate(dtReminderDate);

		//Determine the description to be added to the to-do task
		CString strDescription;
		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		strDescription.Format("Patient requires a prescription refill for %s.  Please see the Medications tab for details.", CString((LPCTSTR)m_pCurrentPrescription->Medication->MedicationName));
		dlg.m_strDefaultNote = strDescription;

		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;

		//Open the to-do
		dlg.DoModal();

		// Added by CH 1/17: Force the next remind
		// time to be 5 minutes if a new task is
		// added so the "Don't remind me again"
		// option will not cause the new task to
		// be forgotten.
		{
			COleDateTime dt = COleDateTime::GetCurrentTime();
			dt += COleDateTimeSpan(0,0,5,0);
			SetPropertyDateTime("TodoTimer", dt);
			// (j.dinatale 2012-10-22 17:59) - PLID 52393 - set our user preference
			SetRemotePropertyInt("LastTimeOption_User", 5, 0, GetCurrentUserName());
			SetTimer(IDT_TODO_TIMER, 5*60*1000, NULL);
		}		

	}NxCatchAll(__FUNCTION__);	
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Removed dead ToggleFavoritePharmacies code.
// (j.jones 2008-10-09 14:49) - PLID 17235 - added OnSelChosenMedPharmacyCombo
void CPrescriptionEditDlg::OnSelChosenMedPharmacyCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (pRow == nullptr) {
			return;
		}

		// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
		long nPharmacyID = VarLong(pRow->GetValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID), -1);
		
		// Special rows are not valid pharmacies to save to the prescription.
		if (nPharmacyID == EPharmacySearchSpecialRow::InstructionsFooter || nPharmacyID == EPharmacySearchSpecialRow::NoResultsFound
			|| nPharmacyID == EPharmacySearchSpecialRow::SpacerFooter) {
			// They have no previously saved pharmacy. Revert to the default selection in the search list, since 
			// search lists always require a selection, but show the search text box instead.
			m_PharmacyCombo->CurSel = GetDefaultPharmacySearchListSelection();
			nPharmacyID = -1;
		}

		// (a.walling 2009-03-31 17:23) - PLID 33573 - Allow adding new pharmacy directly from directory
		if (nPharmacyID == EPharmacySearchSpecialRow::AddFromPharmacyDirectory) {
			CPharmacyDirectorySearchDlg dlg(this);
			dlg.m_bMultiMode = FALSE;

			if (IDOK == dlg.DoModal()) {
				long nSelectedPharmacyID = dlg.m_nSelectedID;

				if (nSelectedPharmacyID != -1) {
					SavePharmacy(nSelectedPharmacyID);
					// (r.gonet 2016-02-23 00:14) - PLID 67988 - Update the selection in the UI.
					UpdateControlsForPharmacySelection(nSelectedPharmacyID);
				}
				else 
				{
					// (j.fouts 2013-04-16 13:19) - PLID 53146 - Reselect the old pharmacy
					// (r.gonet 2016-02-23 00:14) - PLID 67988 - Update the selection in the UI.
					UpdateControlsForPharmacySelection(m_nPrevPharmacyID);
				}
			}
			else
			{
				// (r.gonet 2016-02-23 00:14) - PLID 67988 - Update the selection in the UI.
				UpdateControlsForPharmacySelection(m_nPrevPharmacyID);
			}
		} else {
			// (j.fouts 2013-04-16 13:19) - PLID 53146 - Save
			SavePharmacy(nPharmacyID);
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Update the selection in the UI.
			UpdateControlsForPharmacySelection(nPharmacyID);
		}
	}NxCatchAll("Error in CPrescriptionEditDlg::OnSelChosenMedPharmacyCombo");
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Update the pharmacy selection in the UI.
void CPrescriptionEditDlg::UpdateControlsForPharmacySelection(long nSelectedPharmacyID)
{
	if (IsPharmacyListADropDown()) {
		if(nSelectedPharmacyID > 0) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - Try to select the pharmacy in the datalist.
			// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
			m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID, nSelectedPharmacyID, NULL, VARIANT_TRUE);
		} else {
			m_PharmacyCombo->CurSel = nullptr;
		}
	} else {
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Show or hide the pharmacy datalist depending on if there is a
		// selection or not.
		GetDlgItem(IDC_MED_PHARMACY_COMBO)->ShowWindow(nSelectedPharmacyID > 0 ? SW_HIDE : SW_SHOW);
		if (nSelectedPharmacyID > 0) {
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - There is one, so get its name and put it in the hyperlink.
			_RecordsetPtr prsPharmacy = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LocationsT.Name FROM LocationsT WHERE ID = {INT}", nSelectedPharmacyID);
			if (prsPharmacy->eof) {
				ASSERT(FALSE);
				ThrowNxException("%s : The selected pharmacy location with ID = %li does not exist.", __FUNCTION__, nSelectedPharmacyID);
			}
			CString strPharmacyName = AdoFldString(prsPharmacy->Fields, "Name");

			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Show the hyperlink.
			m_nxlPharmacySelection.ShowWindow(SW_SHOW);
			m_nxlPharmacySelection.SetText(strPharmacyName);
			m_nxlPharmacySelection.SetType(dtsHyperlink);
		} else {
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - Hide the hyperlink. The search list will be blank.
			m_nxlPharmacySelection.ShowWindow(SW_HIDE);
			m_nxlPharmacySelection.SetText("");
			m_nxlPharmacySelection.SetType(dtsDisabledHyperlink);
		}

		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Is this necessary?
		long nPatientMedicationID = m_pCurrentPrescription->PrescriptionID.length() != 0 ? atol(m_pCurrentPrescription->PrescriptionID) : -1;
		// (r.gonet 2016-02-23 12:47) - PLID 67964 - Pass the favorite icon.
		PharmacySearchUtils::UpdatePharmacyResultList(m_PharmacyCombo, GetRemoteDataSnapshot(), m_hStarIcon, m_nPatientID, nPatientMedicationID);

		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Invalidate and redraw the label.
		m_nxlPharmacySelection.AskParentToRedrawWindow();
	}
}

void CPrescriptionEditDlg::OnTrySetSelFinishedQuantityUnit(long nRowEnum, long nFlags)
{
	try {

		//TES 2/12/2009 - PLID 33002 - If we couldn't find it, it must be an old invalid unit, so set the combo text.
		if (nFlags == dlTrySetSelFinishedFailure) {
			m_pQuantityUnitCombo->PutComboBoxText(_bstr_t(m_strPendingUnit));
		}
		else {
			m_strPendingUnit = "";
		}

	}NxCatchAll("Error in CPrescriptionEditDlg::OnTrySetSelFinishedQuantityUnit()");
}

BOOL CPrescriptionEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	//TES 2/17/2009 - PLID 32080 - If they are over our Prior Authorization label, change to a hand
	CRect rc;
	GetDlgItem(IDC_PRIOR_AUTH_LABEL)->GetWindowRect(rc);
	ScreenToClient(&rc);

	// (a.walling 2009-04-06 16:35) - PLID 33870 - Same with refills
	CRect rcRefills;
	GetDlgItem(IDC_REFILLS_LABEL)->GetWindowRect(rcRefills);
	ScreenToClient(&rcRefills);
	// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
	CRect rcDosage;
	GetDlgItem(IDC_DOSAGE_LABEL)->GetWindowRect(rcDosage);
	ScreenToClient(&rcDosage);

	// (a.wilson 2012-11-16 15:18) - PLID 53799 - medication label.
	CRect rcMedicationLabel;
	if (m_bShowDispensedMedication) {
		GetDlgItem(IDC_PRESCRIPTION_LABEL)->GetWindowRect(rcMedicationLabel);
	} else {
		GetDlgItem(IDC_PRESCRIPTION_LABEL_DISPENSED)->GetWindowRect(rcMedicationLabel);
	}
	ScreenToClient(&rcMedicationLabel);


	// (a.wilson 2012-12-31 11:54) - PLID 53784
	CRect rcAssignPatientLink;
	GetDlgItem(IDC_ASSIGN_PATIENT_LINK)->GetWindowRect(rcAssignPatientLink);
	ScreenToClient(&rcAssignPatientLink);

	// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workaround, checkboxes are now labels
	CRect rcAllowSubs;
	GetDlgItem(IDC_ALLOW_SUBSTITUTIONS_TEXT)->GetWindowRect(rcAllowSubs);
	ScreenToClient(&rcAllowSubs);

	CRect rcSaveToQuickList;
	GetDlgItem(IDC_COPY_TO_PICK_LIST_TEXT)->GetWindowRect(rcSaveToQuickList);
	ScreenToClient(&rcSaveToQuickList);

	CRect rcSaveAsDefault;
	GetDlgItem(IDC_SAVE_DEFAULT_TEXT)->GetWindowRect(rcSaveAsDefault);
	ScreenToClient(&rcSaveAsDefault);

	// (b.savon 2013-09-24 09:45) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
	CRect rcAddDiagnosisCodes;
	GetDlgItem(IDC_STATIC_RX_DIAG_CODE)->GetWindowRect(rcAddDiagnosisCodes);
	ScreenToClient(&rcAddDiagnosisCodes);

	// (j.jones 2013-10-18 09:46) - PLID 58983 - added patient education link
	CRect rcPatientEducation;
	GetDlgItem(IDC_PT_EDUCATION_LABEL)->GetWindowRect(rcPatientEducation);
	ScreenToClient(&rcPatientEducation);

	// (r.gonet 2016-02-23 00:14) - PLID 67988 - added the pharmacy link.
	CRect rcPharmacySelectionLabel;
	GetDlgItem(IDC_PHARMACY_SELECTION_LABEL)->GetWindowRect(rcPharmacySelectionLabel);
	ScreenToClient(&rcPharmacySelectionLabel);

	bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - However, the pharmacy link is only sometimes shown.
	bool bShowPharmacySelectionLink = (IsPharmacyListASearchList()
		&& m_pCurrentPrescription != nullptr
		&& m_pCurrentPrescription->Pharmacy != nullptr
		&& m_pCurrentPrescription->Pharmacy->PharmacyID.length() != 0);

	// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
	if ((rc.PtInRect(pt) && m_nxlabelPriorAuthLabel.GetType() == dtsHyperlink) || (rcRefills.PtInRect(pt) &&  m_nxlabelRefills.GetType() == dtsHyperlink)
		|| (rcDosage.PtInRect(pt) &&  m_nxlabelDosage.GetType() == dtsHyperlink) || (rcMedicationLabel.PtInRect(pt) && (m_nxMedicationLabel.GetType() == dtsHyperlink || m_nxDispensedMedicationLabel.GetType() == dtsHyperlink))
		|| (rcAssignPatientLink.PtInRect(pt) && m_nxAssignPatientLink.GetType() == dtsHyperlink) 
		|| (rcAllowSubs.PtInRect(pt) && m_nxsAllowSubsText.GetType() == dtsHyperlink) 
		|| (rcSaveToQuickList.PtInRect(pt) && m_nxsSaveToQuickListText.GetType() == dtsHyperlink) 
		|| (rcSaveAsDefault.PtInRect(pt) && m_nxsSaveAsDefaultText.GetType() == dtsHyperlink)
		|| (rcAddDiagnosisCodes.PtInRect(pt) && m_nxsAddDiagnosisCodes.GetType() == dtsHyperlink)
		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Consider the user's preference to show the link. Only if we are, should we set the cursor
		|| (bShowPatientEducationLink && rcPatientEducation.PtInRect(pt) && m_nxlabelPatientEducation.GetType() == dtsHyperlink)
		|| (bShowPharmacySelectionLink && rcPharmacySelectionLabel.PtInRect(pt) && m_nxlPharmacySelection.GetType() == dtsHyperlink)) {

		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.fouts 2012-10-17 10:25) - PLID 53146 - Calls the API to commit the change
void CPrescriptionEditDlg::CommitPriorAuthIsSample(bool bValue)
{
	NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
	
	//We can just use our current prescription and pass flags for what to save
	// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
	// is a template prescription ID, not a patient prescription ID
	pCommit->Prescription = m_pCurrentPrescription;

	//We want to save the value we have for PriorAuthIsSample
	pCommit->CommitPriorAuthorizationIsSample = VARIANT_TRUE;
	pCommit->CommitSampleExpirationDate = VARIANT_TRUE;

	// (b.savon 2013-03-11 12:35) - PLID 55518 - This isn't nullable. Use different struct
	pCommit->Prescription->PriorAuthorizationIsSample = _variant_t(bValue);
	if(bValue)
	{
		//Prior auth is sample so save the expiration date as well
		COleDateTime dtExpirationDate = m_nxtSampleExpirationDate->GetDateTime();
		if(dtExpirationDate.m_status == COleDateTime::valid
			&& dtExpirationDate.m_dt != NULL
			&& dtExpirationDate.GetYear() >= 1753)
		{
			m_pCurrentPrescription->SampleExpirationDate->SetDateTime(dtExpirationDate);
		}
		else
		{
			//Invalid date so set it to null
			m_pCurrentPrescription->SampleExpirationDate->SetNull();
		}
	}
	else
	{
		//Prior Auth does not use expiration date
		m_pCurrentPrescription->SampleExpirationDate->SetNull();
	}

	Save(pCommit);
}

// (j.fouts 2012-10-17 10:25) - PLID 53146 - Calls the API to commit the change
void CPrescriptionEditDlg::CommitRefillAsNeeded(bool bValue)
{
	NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));

	// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
	// is a template prescription ID, not a patient prescription ID
	pCommit->Prescription = m_pCurrentPrescription;
	pCommit->CommitRefillsAllowed = VARIANT_TRUE;

	if(bValue)
	{
		//Refill as needed
		m_pCurrentPrescription->RefillsAllowed = REFILL_AS_NEEDED_VALUE;
	}
	else
	{
		//Set Number of refills
		CString strValue = "0";
		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
		if(m_pRefillsList->CurSel)
		{
			strValue = AsString(m_pRefillsList->CurSel->GetValue(0));
		}
		m_pCurrentPrescription->RefillsAllowed = _bstr_t(strValue);
	}

	Save(pCommit);
}

LRESULT CPrescriptionEditDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_PRIOR_AUTH_LABEL:
		{
			//TES 2/17/2009 - PLID 32080 - Toggle the text between "Prior Authorization #" and "Sample Prescription #"
			CString strCurrentText = m_nxlabelPriorAuthLabel.GetText();
			if (strCurrentText == PRIOR_AUTHORIZATION_TEXT) {
				m_nxlabelPriorAuthLabel.SetText(SAMPLE_PRESCRIPTION_TEXT);
				//TES 5/8/2009 - PLID 28519 - And show the sample prescription's expiration date
				m_nxsSampleExpirationLbl.ShowWindow(SW_SHOWNA);
				// (b.savon 2013-04-18 13:57) - PLID 56153
				m_nxstaticSampleInternalLabel.ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->ShowWindow(SW_SHOW);
				// (j.fouts 2012-10-17 10:26) - PLID 53146 - Commit changes
				CommitPriorAuthIsSample(true);
				}
				else if(strCurrentText == SAMPLE_PRESCRIPTION_TEXT) {
				m_nxlabelPriorAuthLabel.SetText(PRIOR_AUTHORIZATION_TEXT);
				//TES 5/8/2009 - PLID 28519 - And hide the sample prescription's expiration date
				m_nxsSampleExpirationLbl.ShowWindow(SW_HIDE);
				// (b.savon 2013-04-18 13:57) - PLID 56153
				m_nxstaticSampleInternalLabel.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->ShowWindow(SW_HIDE);
				// (j.fouts 2012-10-17 10:26) - PLID 53146 - Commit changes
				CommitPriorAuthIsSample(false);
				}
				else {
				//Inconceivable!
				ASSERT(FALSE);
				AfxThrowNxException("Invalid text found when clicking on prior authorization label");
			}

		}
		break;
		case IDC_REFILLS_LABEL:
		{
			//TES 2/17/2009 - PLID 32080 - Toggle the text between "Refill" and "Refill As Needed"
			CString strCurrentText = m_nxlabelRefills.GetText();
			if (strCurrentText == REFILLS_TEXT || strCurrentText == REFILL_RENEWAL_TEXT) {
				ToggleRefillAsNeeded(TRUE);
				// (j.fouts 2012-10-17 10:26) - PLID 53146 - Commit changes
				if (m_pCurrentPrescription)
					CommitRefillAsNeeded(true);
				}
				else if(strCurrentText == REFILL_AS_NEEDED_TEXT) {					
				ToggleRefillAsNeeded(FALSE);
				// (j.fouts 2012-10-17 10:26) - PLID 53146 - Commit changes
				if (m_pCurrentPrescription)
					CommitRefillAsNeeded(false);
				}
				else {
				//This is inconceivable too!
				ASSERT(FALSE);
				AfxThrowNxException("Invalid text found when clicking on prior authorization label");
			}
			EnforceRenewalRefillApproval();	// (a.wilson 2013-04-08 17:49) - PLID 56141
		}
		break;
		case IDC_DOSAGE_LABEL:
		{
			// (s.dhole 2013-04-04 11:46) - PLID 51718
			_RecordsetPtr rs = CreateParamRecordset("SELECT DATEDIFF( day, birthdate,getdate()) As AgeInDays , birthdate  FROM personT  WHERE id ={INT}", m_nPatientID);
			if (!rs->eof) {
				_variant_t varSampleExpDate = rs->Fields->GetItem("birthdate")->Value;
				if (varSampleExpDate.vt == VT_DATE) {
					CPrescriptionDosingDlg dlg;
					dlg.m_nMedID = m_nFDBID;
					if (m_bIsEMRTemplate) {
						dlg.m_bkgColor = GetNxColor(GNC_ADMIN, 0);
					} else {
						//set the background color to be the patient color, regardless of the patient's status
						dlg.m_bkgColor = GetNxColor(GNC_PATIENT_STATUS, 1);
					}
					dlg.m_nAgeInDays = AdoFldLong(rs, "AgeInDays", -1);
					CString str = "";
					GetDlgItemText(IDC_MEDICATION_NAME_LABEL, str);
					dlg.m_strMedicationName = str;
					dlg.DoModal();
					}
					else{
					AfxMessageBox("The current patient does not have a valid Birth Date. You must correct this in the General 1 Tab of the Patients Module.");

				}
			}
		}
		break;
		// (a.wilson 2012-11-16 14:17) - PLID 53799 - click the label in order to switch to the dispensed medication information.
		case IDC_PRESCRIPTION_LABEL_DISPENSED:
		case IDC_PRESCRIPTION_LABEL:
		{
			if (m_bShowDispensedMedication) {
				m_bShowDispensedMedication = false;
				m_nxDispensedMedicationLabel.SetType(dtsHyperlink);
				m_nxDispensedMedicationLabel.SetText(DISPENSED_MEDICATION_TEXT);
				m_nxMedicationLabel.SetType(dtsText);
				m_nxMedicationLabel.SetText(PRESCRIBED_MEDICATION_TEXT);
			} else {
				m_bShowDispensedMedication = true;
				m_nxMedicationLabel.SetType(dtsHyperlink);
				m_nxMedicationLabel.SetText(PRESCRIBED_MEDICATION_TEXT);
				m_nxDispensedMedicationLabel.SetType(dtsText);
				m_nxDispensedMedicationLabel.SetText(DISPENSED_MEDICATION_TEXT);
			}
			DisplayCurrentRenewalMedication();
		}
		break;
		// (a.wilson 2013-03-25 12:05) - PLID 53784 - allow the user to assign a patient if we couldn't automatically.
		case IDC_ASSIGN_PATIENT_LINK:
		{
			CAssignPatientRenewalDlg dlg(this, m_pCurrentRenewalRequest->GetPatientInfo());
				if (IDOK == (dlg.DoModal()))
				{
				//update renewal pointer and database.
				long nPatientID = dlg.GetAssignedPatientID();

				NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
					if (pApi && nPatientID != -1)
					{
					pApi->CommitAssignedRenewalPatientID(_bstr_t(GetAPISubkey()), _bstr_t(GetAPILoginToken()),
						// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
						_bstr_t(GetExistingPatientUserDefinedID(nPatientID)), _bstr_t(m_pCurrentRenewalRequest->GetID()));
					m_pCurrentRenewalRequest->GetPatientInfo()->PutID(nPatientID);
					//now with a patient assigned we can check for the original newrx for this renewal.
					//first check if we don't have a newrx link already
					if (m_pCurrentRenewalRequest->GetOriginalPrescriptionID() <= 0) {
						_RecordsetPtr prs = CreateParamRecordset(
							"SELECT	TOP 1	PatMeds.ID "
							"FROM			PatientMedications PatMeds "
							"WHERE			(PatMeds.ID = CONVERT(INT, (CASE WHEN IsNumeric({STRING}) = 1 THEN {STRING} ELSE '-1' END)) "
							"AND			PatMeds.PatientID = {INT} "
							"AND			MedicationID = {INT}) "
							"OR				PatMeds.NexERXMessageID = {STRING} ",
							VarString(m_pCurrentRenewalRequest->GetPrescriberOrderNumber(), "-1"),
							VarString(m_pCurrentRenewalRequest->GetPrescriberOrderNumber(), "-1"),
							nPatientID, m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetID(),
							VarString(m_pCurrentRenewalRequest->GetRelatesToMessageID(), "-1"));

						if (!prs->eof) //if we have a newrx reference then show the button for viewing.
						{
							long nOrigPresID = AdoFldLong(prs, "ID", -1);

							if (nOrigPresID > 0)	//we have a match. display appropriately.
							{
								m_btnViewOriginalPrescription.EnableWindow(TRUE);
								m_btnViewOriginalPrescription.ShowWindow(SW_SHOW);
								m_pCurrentRenewalRequest->PutOriginalPrescriptionID(nOrigPresID);
							}
						} else {	//no matches, then hide and disable the button
							m_btnViewOriginalPrescription.EnableWindow(FALSE);
							m_btnViewOriginalPrescription.ShowWindow(SW_HIDE);
						}
					}
				}
				EnforceDisabledControls();
			}
		}
		break;
		// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workaround, checkboxes are now labels
		case IDC_ALLOW_SUBSTITUTIONS_TEXT:
			m_checkAllowSubstitutions.SetCheck(!m_checkAllowSubstitutions.GetCheck());
			OnBnClickedAllowSubstitutions();
			break;
		case IDC_SAVE_DEFAULT_TEXT:
			m_checkSaveAsDefault.SetCheck(!m_checkSaveAsDefault.GetCheck());
			break;
		case IDC_COPY_TO_PICK_LIST_TEXT:
			m_checkAddToFavorites.SetCheck(!m_checkAddToFavorites.GetCheck());
			break;
			// (b.savon 2013-09-24 10:19) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
		case IDC_STATIC_RX_DIAG_CODE:
		{
			CPrescriptionDiagnosisCodeDlg dlg(AsLong(m_pCurrentPrescription->PrescriptionID));
			if (IDOK == dlg.DoModal() && dlg.CommitChanges()) {
				NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
				pCommit->CommitDiagnosisCodes = VARIANT_TRUE;
				pCommit->Prescription = m_pCurrentPrescription;
				m_pCurrentPrescription->DiagnosisCodes = dlg.GetDiagnosisCodes();
				// (a.wilson 2014-03-24) PLID 61186 - clean up visual appearance of codes in hyperlink.
				CString strDiagnosisDisplay = dlg.GetDiagnosisCodeDisplay();
				if (strDiagnosisDisplay.IsEmpty()) {
					m_nxsAddDiagnosisCodes.SetText("Add Diagnosis Codes...");
					m_nxsAddDiagnosisCodes.SetToolTip("");
				} else {
					m_nxsAddDiagnosisCodes.SetToolTip(strDiagnosisDisplay);
					if (strDiagnosisDisplay.GetLength() > 50) {
						strDiagnosisDisplay = strDiagnosisDisplay.Left(50);
						strDiagnosisDisplay.Trim();
						strDiagnosisDisplay += ("...");
					}
					m_nxsAddDiagnosisCodes.SetText(strDiagnosisDisplay);
				}
				Save(pCommit);
			}
		}
		break;

		// (j.jones 2013-10-18 09:46) - PLID 58983 - added patient education link
		case IDC_PT_EDUCATION_LABEL: {
			if (!m_pCurrentPrescription) {
				//should not be possible
				ASSERT(FALSE);
				ThrowNxException("No valid prescription object exists.");
			}
			long nMedicationID = AsLong(m_pCurrentPrescription->Medication->DrugListID);
			if (nMedicationID == -1) {
				//should not be possible
				ASSERT(FALSE);
				ThrowNxException("No valid medication ID was found.");
			}

			// (r.gonet 10/30/2013) - PLID 58980 - The patient education hyperlink goes to the Medline Plus website
			LookupMedlinePlusInformationViaSearch(this, mlpDrugListID, nMedicationID);
			break;
		}
		case IDC_PHARMACY_SELECTION_LABEL:
		{
			// (r.gonet 2016-02-23 00:14) - PLID 67988 - They clicked the pharmacy hyperlink.
			// Show the search list and auto-search it so it shows the selected pharmacy and the
			// user's favorite pharmacies.
			UpdateControlsForPharmacySelection(-1);
			m_PharmacyCombo->PutSearchStringText("");
			break;
		}
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CPrescriptionEditDlg::OnLabelClick");
	return 0;
}

// (j.jones 2009-02-19 10:45) - PLID 33165 - added GetPrescriptionID to expose m_nPrescriptionID
long CPrescriptionEditDlg::GetPrescriptionID()
{
	try {
		if(m_pCurrentPrescription)
		{
			// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
			return AsLong(m_pCurrentPrescription->PrescriptionID);
		}
	}NxCatchAll("Error in CPrescriptionEditDlg::GetPrescriptionID");

	return -1;
}

// (j.jones 2011-05-02 15:39) - PLID 43450 - added GetPatientExplanation to expose m_strPatientExplanation
CString CPrescriptionEditDlg::GetPatientExplanation()
{
	try {

		return m_strPatientExplanation;

	}NxCatchAll(__FUNCTION__);

	return "";
}  

// (a.walling 2009-04-06 16:44) - PLID 33870 - Toggle interface for 'Refill As Needed'
void CPrescriptionEditDlg::ToggleRefillAsNeeded(BOOL bRefillAsNeeded)
{
	// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
	if (bRefillAsNeeded) {
		m_nxlabelRefills.SetText(REFILL_AS_NEEDED_TEXT);
		GetDlgItem(IDC_PRESCRIPTION_REFILLS)->ShowWindow(SW_HIDE);

	} else {
		if (m_pCurrentPrescription) {
			m_nxlabelRefills.SetText(REFILLS_TEXT);
		// (a.wilson 2013-01-23 11:10) - PLID 53799 - need to display specific text for renewals.
		} else if (m_pCurrentRenewalRequest) {
			m_nxlabelRefills.SetText(REFILL_RENEWAL_TEXT);
		}
		GetDlgItem(IDC_PRESCRIPTION_REFILLS)->ShowWindow(SW_SHOWNOACTIVATE);
	}
}

// (z.manning 2009-05-13 14:18) - PLID 28554
void CPrescriptionEditDlg::SetOrderSetID(const long nOrderSetID)
{
	m_varOrderSetID = nOrderSetID;
}

// (j.fouts 2012-09-26 10:40) - PLID 52906 - Delete a prescription, this will set a deleted flag so it appears to be deleted
void CPrescriptionEditDlg::DeleteSelectedPrescription()
{
	try
	{
		// (j.jones 2012-11-05 17:13) - PLID 53532 - if this is an EMR prescription, disallow if the EMR
		// is locked, or if the EMR is not currently open
		long nEMNID = !m_pCurrentPrescription->EMRPrescriptionSource->emnID->IsNull() ? VarLong(m_pCurrentPrescription->EMRPrescriptionSource->emnID->GetValue()) : -1;
		if(nEMNID != -1) {
			if(m_nCurrentlyOpenedEMNID == -1 || nEMNID != m_nCurrentlyOpenedEMNID) {
				//we did not open the prescription from the same EMN that it's linked to, so they can't delete it
				AfxMessageBox("This medication cannot be deleted because it exists on an EMN for this patient.");
				return;
			}
			else {
				//we are viewing the correct EMN, so they should be able to delete it, provided they currently have writeable access

				// (j.jones 2012-11-21 13:57) - PLID 53818 - check for template access
				if(m_bIsEMRTemplate) {
					// (j.armen 2013-05-14 12:39) - PLID 56683 - EMN Template Access Refactoring
					if(!ReturnsRecordsParam(
						"SELECT EMNID\r\n"
						"FROM EMNTemplateAccessT\r\n"
						"WHERE UserLoginTokenID = {INT} AND EMNID = {INT}", GetAPIUserLoginTokenID(), nEMNID)) {
						AfxMessageBox("This medication cannot be deleted from this template because you do not currently have writeable access to the template.");
						return;
					}
				}
				else {
					long nStatus = -1;
					BOOL bHasAccess = FALSE;
					if(!CanEditEMN(nEMNID, nStatus, bHasAccess)) {
						//if the EMN is locked, say so
						if(nStatus == 2) {
							AfxMessageBox("This medication cannot be deleted from this EMN because the EMN is locked.");
						}
						//otherwise warn if they do not have access to the EMN
						else if(!bHasAccess) {
							AfxMessageBox("This medication cannot be deleted from this EMN because you do not currently have writeable access to the EMN.");
						}
						else {
							//should be impossible
							ThrowNxException("CanEditEMN returned FALSE for unknown reasons.");
						}
						return;
					}
				}

				//if we get here, then the user does currently have writeable access to the EMN,
				//so deletion is allowed
			}
		}
		//Make sure they are cool with us deleting this
		if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to delete this prescription? Removing this prescription will be permanent."))
			return;

		CArray<NexTech_Accessor::_QueuePrescriptionPtr,NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
		NexTech_Accessor::_QueuePrescriptionPtr pPrescription(__uuidof(NexTech_Accessor::QueuePrescription));

		//Send our current prescription to the API
		aryPrescriptions.Add(m_pCurrentPrescription);

		Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);

		NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pRequest(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));

		//Tell it that we want to deny this request
		pRequest->Action = NexTech_Accessor::UpdatePresQueueAction_Delete;
		
		// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
		pRequest->ExcludeMonographInformation = VARIANT_TRUE;

		pRequest->PrescriptionsToDelete = saryPrescriptions;
		
		// (j.fouts 2013-04-24 16:54) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
		GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_nPatientID), pRequest);

		// (j.jones 2012-11-15 15:35) - PLID 53765 - return the enum for epdrvDeleteRx
		m_epdrvAction = epdrvDeleteRx;
		EndDialog((int)m_epdrvAction);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-02 08:56) - PLID 52973 - Draws the prescription pad when drawing the background
BOOL CPrescriptionEditDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);

	//Fill the normal bg color
	pDC->FillRect(rcClient, const_cast<CBrush*>(&m_brBackground));
	//Draw the prescription pad

	// (j.fouts 2012-10-22 11:39) - PLID 53156 - Resized the prescription bg to fit the Patient Information
	// (s.dhole 2013-10-19 11:14) - PLID 59084 resize dialog
	DrawBitmap(pDC, (HBITMAP)m_imgPrescriptionBack, 8, 1, 599, 632);

	//TRUE = No more painting
	return TRUE;
}

// (j.fouts 2012-10-02 08:57) - PLID 52907 - Disables/Enables the appropriate controls for the selected prescription
//This function makes the assumption that if a control is currently disabled or hidden then it should remain disabled or hidden.
//this will only dissable or hide controls that should not currently be shown.
// PLEASE DO NOT ADD ANY CODE OR CALL ANY CODE IN THIS FUNCTION THAT ENABLES OR SHOWS A CONTROL THAT WOULD BREAK THE ASSUMPTION ABOVE
void CPrescriptionEditDlg::EnforceDisabledControls()
{
	//Prescriptions will have a different process for hiding than renewals
	if(m_pCurrentPrescription)
	{
		//Strength should always be readonly
		m_editStrength.SetReadOnly(TRUE);
		//We are only showing phone num on renewals
		GetDlgItem(IDC_PRESCRIPTION_PATIENT_PHONE_LABEL)->ShowWindow(SW_HIDE);

		//We have a prescription so renewals should be hidden
		// (a.wilson 2012-11-15 16:32) - PLID 51711 - Hide all resouces involved in renewals if we have an prescription open.
		HideRenewalControls();

		if(m_bIsEMRTemplate)
		{
			HideEMRTemplateControls();
		}

		//hide the english explanation if the setting says to do so
		if(GetRemotePropertyInt("ShowEnglishPrescriptionColumn", 0, 0, GetCurrentUserName(), true) == 0) {
			HideEnglishDescriptionControls();			
		}

		if(m_bReadOnly)
		{
			//This prescription is readonly we should disable everything other than the close, print, and fax buttons
			DisableControls();
			m_btnDelete.EnableWindow(FALSE);
		}

		/// HIDING CONTROLS FOR E-PRESCRIBING LICENSE TYPE ///
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
		switch(ept)
		{
		case CLicense::eptSureScripts:
			//They have NexERx
			
			// (s.dhole 2012-10-23 ) - PLID 51718   Dosing information
			// (s.dhole 2012-12-06 08:53) - PLID 54067 check Licensesing
			if (m_nFDBID <= 0){
				HideDosage();
			}

			// (b.savon 2013-01-16 11:20) - PLID 54632 - Disable if user role is None
			if(!m_bCachedIsLicensedConfiguredUser)
			{
				//They have NexERx but are not configured
				DisableEFax();
				DisableESubmit();
			}

			break;
		case CLicense::eptNewCrop:
		case CLicense::eptNone:
			//They have don't have NexERx
			HideNexERxControls();
			break;
		}

		/// HIDING CONTROLS STATUS OF THE PRESCRIPTION ///
		switch(m_pCurrentPrescription->status)
		{
		// (b.savon 2013-09-04 15:29) - PLID 58212 - Add a new 'ReadyForDoctorReview' type for Prescriptions
		case NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview:
		case NexTech_Accessor::PrescriptionStatus_Incomplete:
		case NexTech_Accessor::PrescriptionStatus_eTransmitError:
		case NexTech_Accessor::PrescriptionStatus_OnHold:
			//Allow Printing/Sending/Faxing
			// (s.dhole 2013-02-11 16:47) - PLID 54063
			if(!IsSubmitButtonEnabled())
			{
				DisableESubmit();
			}

			if(IsErrorEnabled() == SW_HIDE)
			{
				m_nxBtnRxError.ShowWindow(SW_HIDE);
			}

			if(!ConfigureSubmitBtns())
			{
				DisableEFax();
			}

			break;
		case NexTech_Accessor::PrescriptionStatus_Legacy:
			//Allow Printing, Delete / Disallow Fax, Send
			DisableEFax();
			DisableESubmit();
			m_nxBtnRxError.ShowWindow(SW_HIDE);
			break;
		case NexTech_Accessor::PrescriptionStatus_eFaxed:
		case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
		case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
		case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
		// (b.savon 2013-09-04 15:29) - PLID 58212 - Add a new 'Void' type for Prescriptions
		case NexTech_Accessor::PrescriptionStatus_Void:
			//Allow Printing / Disallow Delete, Fax, Send
			m_btnDelete.EnableWindow(FALSE);
			DisableEFax();
			DisableESubmit();
			m_nxBtnRxError.ShowWindow(SW_HIDE);
			break;
		case NexTech_Accessor::PrescriptionStatus_Printed:
			DisablePrescriptionControls();
			if(!IsSubmitButtonEnabled())
			{
				DisableESubmit();
			}

			if(IsErrorEnabled() == SW_HIDE)
			{
				m_nxBtnRxError.ShowWindow(SW_HIDE);
			}

			if(!ConfigureSubmitBtns())
			{
				DisableEFax();
			}

			m_btnDelete.EnableWindow(FALSE);
			break;
		}

		/// HIDING CONTROLS FOR NEWCROP PRESCRIPTIONS ///
		// (j.fouts 2012-12-28 12:49) - PLID 54340 - If this is from NewCrop, never allow sending, printing, and faxing
		if(!CString((LPCTSTR)m_pCurrentPrescription->NewCropGUID).IsEmpty())
		{
			m_btnDelete.EnableWindow(FALSE);
			DisableEFax();
			DisableESubmit();

			// (s.dhole 2013-03-01 13:00) - PLID 54063
			m_nxBtnRxError.ShowWindow(SW_HIDE);
			m_nxBtnRxError.EnableWindow(FALSE);
		}

		/// HIDING CONTROLS FOR PRESCRIPTIONS FROM A NEW RX TO FOLLOW - RENEWAL REQUEST ///
		// (a.wilson 2013-02-05 12:37) - PLID 55014 - if this is a newrx for a deny we need to make the pharmacy readonly
		//as well as other fields that should not be used on this specific situation.
		if (m_nDenyNewRxResponseID > 0) {
			// (r.gonet 2016-02-24 17:01) - PLID 67988 - Disable the search list. Simply making it read only
			// would not take away the ability to put a blinking cursor in the search edit box.
			m_PharmacyCombo->Enabled = VARIANT_FALSE;
			m_PharmacyCombo->PutReadOnly(VARIANT_TRUE);
			m_btnDelete.EnableWindow(FALSE);
		}

		// (j.fouts 2013-04-01 12:53) - PLID 53840
		if(!(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS))
		{
			DisablePrint();
			DisableEFax();
			DisableESubmit();
		}

		if(!(GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS))
		{
			m_btnDelete.EnableWindow(FALSE);
		}
	}
	// (a.wilson 2012-11-15 16:32) - PLID 51711 - hide and show necessary resources.
	else if (m_pCurrentRenewalRequest) {
		//these controls and setup only need to be done on initial load of the renewal.
		if (!m_bInitialRenewalDisplayComplete) {
			//DISABLE all drug related controls that cannot be modified by a renewal. In order.
			m_nxBtnRxError.EnableWindow(FALSE);//rxerror
			m_nxstaticDateLabel.EnableWindow(FALSE);	//pres. date label
			m_dtPrescriptionDate.EnableWindow(FALSE);	//pres. date
			m_pDosageQuantityList->PutReadOnly(VARIANT_TRUE);//dosage
			m_pDosageUnitList->PutReadOnly(VARIANT_TRUE); //dosage unit
			m_editDosageForm.SetReadOnly(TRUE);//dosage form
			m_pRouteList->PutReadOnly(VARIANT_TRUE);//method
			m_pFrequencyList->PutReadOnly(VARIANT_TRUE);//frequency
			m_editQuantity.SetReadOnly(TRUE);	//quantity
			m_pQuantityUnitCombo->PutReadOnly(VARIANT_TRUE);//quantity unit
			m_pPackagingCombo->PutReadOnly(VARIANT_TRUE);//packaging info
			m_checkAllowSubstitutions.EnableWindow(FALSE); //substitutions
			m_nxsAllowSubsText.EnableWindow(FALSE);	//allow subs text.
			m_editDaysSupply.SetReadOnly(TRUE);	//days supply
			m_editPriorAuthorization.SetReadOnly(TRUE);//prior authorization
			m_nxtSampleExpirationDate->PutEnabled(VARIANT_FALSE); //sample exp.
			m_editStrength.SetReadOnly(TRUE);//strength
			m_nxeditStrengthUnits.SetReadOnly(TRUE);//strength units
			m_editPrescriptionExplanation.SetReadOnly(TRUE);	//sig
			m_editNoteToPharmacist.SetReadOnly(TRUE);	//notes
			m_btnESubmit.EnableWindow(FALSE);//esubmit
			m_btnPreview.EnableWindow(FALSE);//preview
			m_btnEFax.EnableWindow(FALSE);//fax
			m_btnDelete.EnableWindow(FALSE);//delete

			//HIDE all drug related controls that are necessary for renewals. In order.
			m_nxBtnRxError.ShowWindow(SW_HIDE);//rxerror
			m_nxstaticDateLabel.ShowWindow(SW_HIDE);	//pres. date label
			m_dtPrescriptionDate.ShowWindow(SW_HIDE);	//pres. date
			GetDlgItem(IDC_DOSAGE_LABEL)->ShowWindow(SW_HIDE);	//dosing hyperlink.
			GetDlgItem(IDC_DOSAGE_TEXT)->ShowWindow(SW_HIDE);	//dosage text
			GetDlgItem(IDC_PRESCRIPTION_DOSAGE)->ShowWindow(SW_HIDE);	//dosage
			GetDlgItem(IDC_PRESCRIPTION_DOSAGE_UNIT)->ShowWindow(SW_HIDE);	//dosage unit
			GetDlgItem(IDC_METHOD_TEXT)->ShowWindow(SW_HIDE);	//method text
			GetDlgItem(IDC_PRESCRIPTION_METHOD)->ShowWindow(SW_HIDE);	//method
			GetDlgItem(IDC_FREQUENCY_TEXT)->ShowWindow(SW_HIDE);	//frequency text
			GetDlgItem(IDC_PRESCRIPTION_FREQUENCY)->ShowWindow(SW_HIDE);	//frequency

			m_btnESubmit.ShowWindow(SW_HIDE);//esubmit
			m_btnPreview.ShowWindow(SW_HIDE);//preview
			m_btnEFax.ShowWindow(SW_HIDE);//fax
			m_btnDelete.ShowWindow(SW_HIDE);//delete

			//READONLY Controls so that the user can see important information about the selection.
			m_PharmacyCombo->PutReadOnly(VARIANT_TRUE);
			m_ProviderCombo->PutReadOnly(VARIANT_TRUE);
			m_SupervisorCombo->PutReadOnly(VARIANT_TRUE);
			m_AgentCombo->PutReadOnly(VARIANT_TRUE);
			m_LocationCombo->PutReadOnly(VARIANT_TRUE);

			//DISABLE other non drug related controls.
			m_editResponseReason.SetReadOnly(TRUE);	//response reason
			m_checkSaveAsDefault.EnableWindow(FALSE);	//save as default
			m_checkAddToFavorites.EnableWindow(FALSE);	//add favorites

			//HIDE other non drug related controls.
			m_checkSaveAsDefault.ShowWindow(SW_HIDE);	//save as default
			m_checkAddToFavorites.ShowWindow(SW_HIDE);	//add favorites
			m_nxsSaveToQuickListText.ShowWindow(SW_HIDE);	//quick list text
			m_nxsSaveAsDefaultText.ShowWindow(SW_HIDE);	//save as default text

			// (b.savon 2013-09-24 09:48) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
			m_nxsAddDiagnosisCodes.ShowWindow(SW_HIDE);
			m_nxsAddDiagnosisCodes.EnableWindow(FALSE);
			GetDlgItem(IDC_STATIC_DIAGNOSIS_CODE_GROUP)->EnableWindow(FALSE);
			GetDlgItem(IDC_STATIC_DIAGNOSIS_CODE_GROUP)->ShowWindow(SW_HIDE);

			//TES 9/24/2013 - PLID 57999 - Hide the formulary controls if we're on a renewal
			GetDlgItem(IDC_FORMULARY_INS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FORMULARY_STATUS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FORMULARY_STATUS_TEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COPAY_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FORMULARY_COPAY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FORMULARY_LABEL)->ShowWindow(SW_HIDE);
			// (s.dhole 2013-10-19 11:08) - PLID 59084 
			GetDlgItem(IDC_FORMULARY_INS_LABEL)->ShowWindow(SW_HIDE);
			// (s.dhole 2013-10-19 11:14) - PLID 59084
			m_btnFCovarage.ShowWindow(SW_HIDE); 
			

			//remove other unnecessary controls for renewals using available functions
			HideEnglishDescriptionControls();
			HideToDoReminderControls(FALSE);

			////handle specific options concerning renewals.
			//check for original NewRx for Viewing otherwise hide the option.
			if (m_pCurrentRenewalRequest->GetOriginalPrescriptionID() <= 0) {
				m_btnViewOriginalPrescription.EnableWindow(FALSE);
				m_btnViewOriginalPrescription.ShowWindow(SW_HIDE);
			}
			//display the date we recieved the renewal.
			COleDateTime dtReceivedDate(m_pCurrentRenewalRequest->GetReceivedDate());
			if (dtReceivedDate && dtReceivedDate.GetStatus() == COleDateTime::valid) {
				// (b.savon 2013-04-09 10:19) - PLID 56153
				SetDlgItemText(IDC_FIRST_DATE_LABEL, "Received Date (Internal Use Only):");
				m_editFirstDate.SetWindowText(dtReceivedDate.Format("%m/%d/%Y"));
				m_nxstaticInternalDate.ShowWindow(SW_HIDE);
			} else {
				GetDlgItem(IDC_FIRST_DATE_LABEL)->ShowWindow(SW_HIDE);
				m_editFirstDate.ShowWindow(SW_HIDE);
			}
			//set the prior auth. label to the correct text.
			m_nxlabelPriorAuthLabel.SetType(dtsText);
			m_nxlabelPriorAuthLabel.SetText(PRIOR_AUTHORIZATION_TEXT);
			// (b.savon 2013-04-18 12:48) - PLID 56153
			m_nxstaticPriorAuthInternalLabel.ShowWindow(SW_HIDE);
			m_nxstaticPriorAuthInternalLabel.EnableWindow(FALSE);
			m_nxstaticSampleInternalLabel.ShowWindow(SW_HIDE);
			m_nxstaticSampleInternalLabel.EnableWindow(FALSE);
			// (b.savon 2013-05-21 09:18) - PLID 56648
			m_nxstaticStrengthInternalLabel.ShowWindow(SW_HIDE);
			m_nxstaticStrengthInternalLabel.EnableWindow(FALSE);
			//rename the sample exp. control label to written date
			GetDlgItem(IDC_SAMPLE_EXPIRATION_LBL)->SetWindowText("Written Date");
			//rename the diagnosis codes label to renewal response label
			GetDlgItem(IDC_DIAGNOSIS_CODES_LABEL)->EnableWindow();
			GetDlgItem(IDC_DIAGNOSIS_CODES_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DIAGNOSIS_CODES_LABEL)->SetWindowText("Response Reason");
			//remove hyperlink for allow subs
			m_nxsAllowSubsText.SetType(dtsText);

			// (a.wilson 2013-05-01 10:09) - PLID 56509 - display warning if user is not authorized and the renewal is pending.
			if (!m_bUserAuthorizedPrescriber && m_pCurrentRenewalRequest->GetResponseStatus() == NexTech_Accessor::RenewalResponseStatus_Pending) {
				m_nxUserAuthorizationWarning.SetColor(RGB(255, 50, 50));
				m_nxUserAuthorizationWarning.SetWindowText("You are not authorized to prescribe renewal requests for this prescriber!");
			} else {
				m_nxUserAuthorizationWarning.EnableWindow(FALSE);
				m_nxUserAuthorizationWarning.ShowWindow(SW_HIDE);
			}

			//ensure these do not get set again during this dialog session.
			m_bInitialRenewalDisplayComplete = true;
		}
		////incase this function gets run again because a status changed or a patient was assigned.
		//Disable the reponse options if this renewal has already been responded to.
		// (a.wilson 2013-05-01 10:08) - PLID 56509 - disable response buttons if user is not authorized.
		if (m_pCurrentRenewalRequest->GetResponseStatus() != NexTech_Accessor::RenewalResponseStatus_Pending || !m_bUserAuthorizedPrescriber) {
			m_btnApprove.EnableWindow(FALSE);
			m_btnApproveWithChanges.EnableWindow(FALSE);
			m_btnDeny.EnableWindow(FALSE);
			m_btnDenyAndRewrite.EnableWindow(FALSE);
		}
		//Display or hide the ability to assign a patient depending on auto-match failed or succeeded.
		// (a.wilson 2013-04-11 16:53) - PLID 55973 - allow user to reassign patient before response.
		// (a.wilson 2013-05-01 10:08) - PLID 56509 - disable patient assignment if user is not authorized.
		if (m_pCurrentRenewalRequest->GetResponseStatus() == NexTech_Accessor::RenewalResponseStatus_Pending && m_bUserAuthorizedPrescriber) {
			m_nxAssignPatientLink.ShowWindow(SW_SHOW);
			m_nxAssignPatientLink.EnableWindow(TRUE);
			m_nxAssignPatientLink.SetType(dtsHyperlink);
			if (m_pCurrentRenewalRequest->GetPatientInfo()->GetID() <= 0)
				m_nxAssignPatientLink.SetText("<Click here to Assign Patient>");
			else
				m_nxAssignPatientLink.SetText("<Click here to Reassign Patient>");
		} else {
			m_nxAssignPatientLink.ShowWindow(SW_HIDE);
			m_nxAssignPatientLink.EnableWindow(FALSE);
			m_nxAssignPatientLink.SetType(dtsText);
		}
	}
	else
	{
		ClearAllFields();
		DisableControls();
	}

	if(m_bReadOnly)
	{
		m_btnDelete.EnableWindow(FALSE);
		m_btnDelete.ShowWindow(SW_HIDE);
	}
}

// (j.fouts 2012-10-02 09:46) - PLID 52907 - Clear all fields associated with a prescription
void CPrescriptionEditDlg::ClearAllFields()
{
	try
	{
		SetDlgItemText(IDC_FIRSTDATE, "");
		m_dtPrescriptionDate.SetTime();
		m_dtReminderDate.SetTime();
		m_nxlabelPriorAuthLabel.SetWindowText(PRIOR_AUTHORIZATION_TEXT);
		m_nxlabelRefills.SetWindowText(REFILLS_TEXT);
		m_nxstaticMedicationNameLabel.SetWindowText("");
		m_editPrescriptionExplanation.SetWindowText("");
		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
		m_pRefillsList->CurSel = NULL;
		m_editQuantity.SetWindowText("");
		m_editStrength.SetWindowText("");
		m_ProviderCombo->CurSel = NULL;
		m_LocationCombo->CurSel = NULL;
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Clear the pharmacy.
		UpdateControlsForPharmacySelection(-1);
		m_SupervisorCombo->CurSel = NULL;
		m_AgentCombo->CurSel = NULL;
		m_pQuantityUnitCombo->CurSel = NULL;
		m_pPackagingCombo->CurSel = NULL;
		m_editDaysSupply.SetWindowText("");
		m_checkAllowSubstitutions.SetCheck(BST_UNCHECKED);
		m_editPriorAuthorization.SetWindowText("");
		m_editNoteToPharmacist.SetWindowText("");
		m_nxtSampleExpirationDate->SetDateTime(NULL);
		m_nxeditStrengthUnits.SetWindowText("");
		m_editDosageForm.SetWindowTextA("");
		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
		GetDlgItem(IDC_PRESCRIPTION_REFILLS)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->ShowWindow(SW_HIDE);
		m_nxsSampleExpirationLbl.ShowWindow(SW_HIDE);
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-02 09:15) - PLID 52906 - Delete button, deletes that selected prescription
void CPrescriptionEditDlg::OnBnClickedDeletePrescription()
{
	try
	{
		DeleteSelectedPrescription();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Focus was lost, check if they changed anything and save it
void CPrescriptionEditDlg::OnEnKillfocusPrescriptionExplanation()
{
	try
	{
		if (!m_pCurrentPrescription)
			return;

		//(r.wilson 4/8/2013) pl 56117 - Lets Go ahead and remove leading and trailing spaces
		TrimSpaces(this, IDC_PRESCRIPTION_EXPLANATION);
		
		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		//(r.wilson 4/8/2013) pl 56117 - This is done inside the function now
		SaveBatchPatientExplanation(pCommit);

		if(pCommit->CommitPatientExplanation ==  VARIANT_TRUE)
		{
			Save(pCommit);
		}
		
	}
	NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/8/2013) pl 56117 - Set flag in pCommit pointer to include PatientExplanation Member when the Save(..) is called
void CPrescriptionEditDlg::SaveBatchPatientExplanation(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	if(!pCommit){
		//(r.wilson 4/8/2013) pl 56117 - If NULL then do nothing
		return;
	}

		//Read the value from the control
		// (j.jones 2013-01-08 12:50) - PLID 47303 - ensured this field synced with the member variable
		GetDlgItemText(IDC_PRESCRIPTION_EXPLANATION, m_strPatientExplanation);

		if(m_strPatientExplanation.Compare(VarString(m_pCurrentPrescription->GetPatientExplanation(), "")) != 0)
		{
			// (j.fouts 2013-01-10 10:53) - PLID 54463 - Check if the patient explination does not match our generated explination
			//then we want to stop generating the sig
			m_pAutoPopulatingSig->SetSig(m_strPatientExplanation);

			m_pCurrentPrescription->PatientExplanation = _bstr_t(m_strPatientExplanation);
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitPatientExplanation = VARIANT_TRUE;

		}
	
	}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Focus was lost, check if they changed anything and save it
void CPrescriptionEditDlg::OnEnKillfocusNoteToPharmacist()
{
	try
	{
		if (!m_pCurrentPrescription)
			return;

		//(r.wilson 4/8/2013) pl 56117 - Remove all leading and trailing spaces
		TrimSpaces(this, IDC_NOTE_TO_PHARMACIST);

			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
		SaveBatchPharmacistNote(pCommit);

		if(pCommit->CommitPharmacistNote == VARIANT_TRUE)
		{
			Save(pCommit);
		}

	}
	NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/8/2013) pl 56117 - Set flag in pCommit pointer to include Pharmacist Note Member when the Save(..) is called
void CPrescriptionEditDlg::SaveBatchPharmacistNote(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	if(!pCommit){
		//(r.wilson 4/8/2013) pl 56117 - If NULL do nothing
		return;
	}

	CString strText;
	//Read the value from the control
	GetDlgItemText(IDC_NOTE_TO_PHARMACIST, strText);

	if(strText.Compare(VarString(m_pCurrentPrescription->GetPharmacistNote(), "")) != 0)
	{		
		m_pCurrentPrescription->PharmacistNote = _bstr_t(strText);
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitPharmacistNote = VARIANT_TRUE;
	}
	
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Focus was lost, check if they changed anything and save it
void CPrescriptionEditDlg::OnEnKillfocusPriorAuth()
{
	try
	{
		if (!m_pCurrentPrescription)
			return;

		//(r.wilson 4/8/2013) pl 56117 - Remove leading and trailing spaces
		TrimSpaces(this, IDC_PRIOR_AUTH);

			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
		SaveBatchPriorAuthorization(pCommit);

		if(pCommit->CommitPriorAuthorization == VARIANT_TRUE)
		{
			Save(pCommit);
		}
		
	}
	NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/8/2013) pl 56117 - Set flag in pCommit pointer to include Prior Authorization Member when the Save(..) is called
void CPrescriptionEditDlg::SaveBatchPriorAuthorization(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	if(!pCommit || m_bIsEMRTemplate == TRUE){
		//(r.wilson 4/8/2013) pl 56117- If null do nothing
		return;
	}

	CString strText;
	//Read the value from the control
	GetDlgItemText(IDC_PRIOR_AUTH, strText);


	if(strText.Compare(VarString(m_pCurrentPrescription->GetPriorAuthorization(),"")) != 0)
	{		
		m_pCurrentPrescription->PriorAuthorization = _bstr_t(strText);
		SetDlgItemText(IDC_PRIOR_AUTH, strText);

		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitPriorAuthorization = VARIANT_TRUE;
	}
	
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Focus was lost, check if they changed anything and save it
void CPrescriptionEditDlg::OnEnKillfocusDaysSupply()
{
	try
	{
		if (!m_pCurrentPrescription)
			return;

		CString strText;
		//Read the value from the control
		GetDlgItemText(IDC_DAYS_SUPPLY, strText);
		CString strCurrentValue = "";
		if(!m_pCurrentPrescription->DaysSupply->IsNull())
		{
			strCurrentValue.Format("%li", VarLong(m_pCurrentPrescription->DaysSupply->GetValue()));
		}

		if(strCurrentValue.Compare(strText) != 0)
		{
			//The value has changed
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitDaysSupply = VARIANT_TRUE;

			if(strText.IsEmpty())
			{
				m_pCurrentPrescription->DaysSupply->SetNull();
			}
			else
			{
				char* strError;
				long nDaysSupply = strtol(strText, &strError, 10);
				if(strlen(strError) > 0) {
					//Invalid number
					return;
				}

				m_pCurrentPrescription->DaysSupply->SetInt(nDaysSupply);
			}

			m_pAutoPopulatingSig->SetDaysSupply(strText);
			UpdateSig(pCommit);
			Save(pCommit);
		}
	}
	NxCatchAll(__FUNCTION__);
}



// (j.fouts 2012-10-17 10:26) - PLID 53146 - Focus was lost, check if they changed anything and save it
void CPrescriptionEditDlg::OnEnKillfocusEditQuantity()
{
	try
	{
		if (!m_pCurrentPrescription)
			return;

		//(r.wilson 4/8/2013) pl 56117 - Get rid of all leading and trailing spaces
		TrimSpaces(this, IDC_EDIT_QUANTITY);

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		SaveBatchQuantity(pCommit);

		if(pCommit->CommitQuantity == VARIANT_TRUE)
		{
			//(r.wilson 4/8/2013) pl 56117 - Attempt to pull out a valid number from string
			/*
			CString strText;
			GetDlgItemText(IDC_EDIT_QUANTITY, strText);
			char* str_pBadString;
			double dExtractedValue = strtod(strText, &str_pBadString);
			*/

			// (j.fouts 2013-04-15 09:05) - PLID 56155 - Removing this for now, I don't like the idea
			// of need being able to enter a Days Supply that does not match the Qty. Sure 99% of the
			// time they will want it to match but there is a possibility that the Qty Unit and Dosage Unit
			// are not the same. If that is the case then they would certainly want a 'custom' Qty and Days
			// Supply. 
			// Note: if the block below this comment is ever uncommented then the block directlt above needs
			//       to be uncommented as well.

			//(r.wilson 4/8/2013) pl 56117 - Only update the Days Supply if the quantity value is valid
			//if(str_pBadString != NULL && str_pBadString[0] == '\0' && dExtractedValue > 0)
			//{
			//	m_pAutoPopulatingSig->SetQuantity(strText);
			//	UpdateDaysSupply(pCommit);
			//}

			Save(pCommit);
		}
	}
	NxCatchAll(__FUNCTION__);	
	
}

//(r.wilson 4/8/2013) pl 56117 - Set flag in pCommit pointer to include Quantity Member when the Save(..) is called
void CPrescriptionEditDlg::SaveBatchQuantity(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	if(!pCommit){
		//(r.wilson 4/8/2013) pl 56117- If NULL do nothing
		return;
	}

	CString strText;
	CString strTextOld;
		//Read the value from the control
		GetDlgItemText(IDC_EDIT_QUANTITY, strText);
	strTextOld = strText;

		if(m_bReadOnly == FALSE){			
			//(r.wilson 3/6/2013) pl 55478 - 11 is the maximum amount of characters the quantity can have and pass validation with sure scripts 		
			strText = FormatDecimalText(CString(strText),11);
			//(r.wilson 4/8/2013) pl 56117 - 
			strText = RemoveInsignificantZeros(CString(strText));
		}

		if(strText.Compare(VarString(m_pCurrentPrescription->GetQuantityValue(),"")) != 0)
		{
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			//(r.wilson 4/8/2013) pl 56117 -		
			m_pCurrentPrescription->QuantityValue = _bstr_t(strText);

			SetDlgItemText(IDC_EDIT_QUANTITY, strText);
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitQuantity = VARIANT_TRUE;
		}
	else if(strText.Compare(strTextOld) != 0)
	{
		SetDlgItemText(IDC_EDIT_QUANTITY, strText);
	}
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - The allow subs box wha clicked save it's new value
void CPrescriptionEditDlg::OnBnClickedAllowSubstitutions()
{
	try
	{
		CString strText;
		//Store value in a API structure
		bool bAllowSubs = m_checkAllowSubstitutions.GetCheck()? true:false;
		if(m_pCurrentPrescription->AllowSubstitutions != (bAllowSubs ? VARIANT_TRUE : VARIANT_FALSE))
		{
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitAllowSubstitutions = VARIANT_TRUE;

			m_pCurrentPrescription->AllowSubstitutions = bAllowSubs ? VARIANT_TRUE : VARIANT_FALSE;

			Save(pCommit);
		}	
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Changes selection, save the changes
void CPrescriptionEditDlg::SelChosenMedLocationCombo(LPDISPATCH lpRow)
{
	try
	{
		// (r.gonet 2016-02-24 17:07) - PLID 67988 - Its read only. They can't select anything.
		if (m_bReadOnly) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nLocationID = pRow ? VarLong(pRow->GetValue(locccID)) : -1;
 
		if(nLocationID != atol(CString((LPCTSTR)m_pCurrentPrescription->locationID)))
		{
			//There were changes
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			pCommit->Prescription = m_pCurrentPrescription;
			
			// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
			pCommit->CommitLocationID = VARIANT_TRUE;

			m_pCurrentPrescription->locationID = nLocationID < 0 ? _bstr_t("") : _bstr_t(FormatString("%li", nLocationID));

			Save(pCommit);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Changes selection, save the changes
void CPrescriptionEditDlg::SelChangingMedPharmacyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - If they select the no results row, revert to whatever they had before.
		NXDATALIST2Lib::IRowSettingsPtr pOldSel(lpOldSel);
		NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);

		// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
		long nPrevPharmacyID = pOldSel != nullptr ? VarLong(pOldSel->GetValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID), -1) : -1;
		m_nPrevPharmacyID = nPrevPharmacyID;

		// Revert to the previous selection.
		if (pNewSel == nullptr) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::SavePharmacy(long nPharmacyID)
{
	if(m_pCurrentPrescription)
	{
		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;

		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		pCommit->CommitPharmacyID = VARIANT_TRUE;

		// (j.fouts 2013-04-22 14:13) - PLID 54719 - PharmacyID is passed in a Pharmacy object
		if(nPharmacyID != atol(CString((LPCTSTR)m_pCurrentPrescription->Pharmacy->PharmacyID)))
		{
			m_pCurrentPrescription->Pharmacy->PharmacyID = nPharmacyID > 0 ? _bstr_t(FormatString("%li", nPharmacyID)) : _bstr_t("");
			Save(pCommit);
		}	
	}
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Changes selection, save the changes
void CPrescriptionEditDlg::SelChosenQuantityUnit(LPDISPATCH lpRow)
{
	try
	{
		// Don't let them select anything while read only.
		if (m_bReadOnly) {
			return;
		}

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitQuantityUnitID = VARIANT_TRUE;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow)
		{
			//Selection was valid save it
 			m_pCurrentPrescription->QuantityUnit = _bstr_t(AsString(pRow->GetValue(qucName)));
			m_pCurrentPrescription->QuantityUnitID = _bstr_t(AsString(pRow->GetValue(qucID)));
		}
		else
		{
			//No row was selected so set it to null
			m_pCurrentPrescription->QuantityUnit = _bstr_t("");
			m_pCurrentPrescription->QuantityUnitID = _bstr_t("");
		}

		Save(pCommit);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-10-17 10:26) - PLID 53146 - Foccus was lost, check for changes and save
void CPrescriptionEditDlg::KillFocusSampleExpirationDate()
{
	try
	{
		COleDateTime dtExpirationDate = m_nxtSampleExpirationDate->GetDateTime();
		m_pCurrentPrescription->SampleExpirationDate;
		bool bChanged = false;
		bool bDateInvalid = false;


		// (j.fouts 2013-04-26 09:30) - PLID 53146 - Don't allow invalid dates
		if(dtExpirationDate.m_status == COleDateTime::invalid 
			|| dtExpirationDate.m_dt == NULL
			|| dtExpirationDate.GetYear() < 1753)
		{
			bDateInvalid = true;
			m_nxtSampleExpirationDate->Clear();
		}

		if(m_pCurrentPrescription->SampleExpirationDate->IsNull())
		{
			if(!bDateInvalid)
			{
				bChanged = true;
			}
		}
		else
		{
			if(dtExpirationDate != VarDateTime(m_pCurrentPrescription->SampleExpirationDate->GetValue()))
			{
				bChanged = true;
			}
		}

		if(bChanged)
		{
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (b.savon 2013-03-25 12:02) - PLID 55518
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitSampleExpirationDate = VARIANT_TRUE;
			
			if(!bDateInvalid)
			{
				m_pCurrentPrescription->SampleExpirationDate->SetDateTime(dtExpirationDate);
			}
			else
			{
				//Invalid date so set it to null
				m_pCurrentPrescription->SampleExpirationDate->SetNull();
			}

			Save(pCommit);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-10-23 ) - PLID 54890 
void CPrescriptionEditDlg::OnBnClickedBtnErxError()
{
	try {
		// (s.dhole 2012-12-06 08:53) - PLID 54067
			if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent) && m_pCurrentPrescription->ValidationList->ErrorStatus!=NexTech_Accessor::eRxErrorStatus_eRxNoError ) 
			{
					// (s.dhole 2012-12-04 13:24) - PLID 54026 Passing parmeter to format text
					SureScripts::ShowSureScriptValidationMsg((HWND)this,GetCurrentUserName(), (LPCTSTR)m_pCurrentPrescription->PatientInformation->PatientName,(LPCTSTR)m_pCurrentPrescription->Medication->MedicationName,
							FormatDateTimeForInterface(VarDateTime(m_dtPrescriptionDate.GetValue())),SureScripts::LoadSureScriptErrorDesc(m_pCurrentPrescription->ValidationList) ,
							GetPracPath(PracPath::SessionPath) ^ "PrescriptionValidation.txt");
				
			}
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-10-25 10:03) - PLID 51711 - this will fill all the necessary fields with the renewals data.
int CPrescriptionEditDlg::ViewRenewalRequest(long nRenewalID)
{
	try {
		// (r.gonet 2016-02-23 00:53) - PLID 68408 - Store the displayed record type.
		m_eDisplayedRecordType = EDisplayedRecordType::Renewal;

		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();

		if (pApi)
		{
			NexTech_Accessor::_ERxRenewalRequestInfoPtr pRenewalRequest = pApi->GetRenewalRequestInfo(_bstr_t(GetAPISubkey()), 
				_bstr_t(GetAPILoginToken()), _bstr_t(nRenewalID));

			if (pRenewalRequest)
			{
				m_pCurrentRenewalRequest = pRenewalRequest;

				// (j.jones 2012-11-16 08:55) - PLID 53765 - track what it was we did in this dialog
				m_epdrvAction = epdrvEditRx;

				// (j.jones 2016-01-06 15:33) - PLID 67823 - track if the prescription is new - in this case it is not new, it's just a renewal
				m_bIsNewRx = false;

				return DoModal();
			}
		}

	}NxCatchAll(__FUNCTION__);

	return 0;	
}

// (a.wilson 2012-11-08 17:23) - PLID 51711 - this will open the renewal and fill in all the fields on the screen.
void CPrescriptionEditDlg::OpenRenewalRequest()
{
	try {
		//ensure we have a renewal before continuing.
		if (m_pCurrentRenewalRequest)
		{
			//update patient fields.
			if (m_pCurrentRenewalRequest->GetPatientInfo()) {
				NexTech_Accessor::_ERxPatientInfoPtr pPatientInfo = m_pCurrentRenewalRequest->GetPatientInfo();
				SetTextAndToolTip(&m_nxPatientName, FormatString("%s, %s %s", VarString(pPatientInfo->GetLastName()), 
					VarString(pPatientInfo->GetFirstName()), VarString(pPatientInfo->GetMiddleName())));
				if (VarString(pPatientInfo->GetGender(), "") != "") {
					m_nxPatientGender.SetWindowText(GetGenderAsTextFromString((LPCTSTR)pPatientInfo->GetGender()));
				} else {
					GetDlgItem(IDC_PRESCRIPTION_PATIENT_GENDER_LABEL)->ShowWindow(SW_HIDE);
				}
				if (Trim(VarString(pPatientInfo->GetAddress1())) != "") {
					SetTextAndToolTip(&m_nxPatientAddress, FormatAddressForPrescription(VarString(pPatientInfo->GetAddress1()), 
						VarString(pPatientInfo->GetAddress2()), VarString(pPatientInfo->GetCity()), 
						VarString(pPatientInfo->GetState()), VarString(pPatientInfo->GetZip())));
				} else {
					GetDlgItem(IDC_PRESCRIPTION_PATIENT_ADDRESS_LABEL)->ShowWindow(SW_HIDE);
				}
				if (pPatientInfo->GetBirthdate() != 0) {
					SetTextAndToolTip(&m_nxPatientBirthdate, COleDateTime(pPatientInfo->GetBirthdate()).Format("%m/%d/%Y"));
				} else {
					GetDlgItem(IDC_PRESCRIPTION_PATIENT_BIRTHDATE_LABEL)->ShowWindow(SW_HIDE);
				}
				CString strPhoneNumber = "";
				CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
				Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pPatientInfo->GetPhoneNumbers());
				sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
				if (!arRenewalRequestPhoneNumbers.IsEmpty())
				{
					for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
					{
						NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
						if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
						{
							strPhoneNumber = VarString(phoneNumber->GetNumber(), "");
						}
					}
				}
				if (!strPhoneNumber.IsEmpty()) {
					SetTextAndToolTip(&m_nxPatientPhone, FormatPhone(strPhoneNumber, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true)));
				} else {
					GetDlgItem(IDC_PRESCRIPTION_PATIENT_PHONE_LABEL)->ShowWindow(SW_HIDE);
				}
			}
			//update location fields.
			{
				if (m_pCurrentRenewalRequest->GetPrescriberLocationID() > 0)
				{
					m_LocationCombo->FindByColumn(locccID, m_pCurrentRenewalRequest->GetPrescriberLocationID() , NULL, VARIANT_TRUE);

					if (!m_LocationCombo->GetCurSel())
					{
						m_LocationCombo->FindByColumn(locccID, -1, NULL, VARIANT_TRUE);
					}
				} else {
					m_LocationCombo->FindByColumn(locccID, -1, NULL, VARIANT_TRUE);
				}
			}
			//update pharmacy fields.
			if (m_pCurrentRenewalRequest->GetPharmacyInfo()) {
				// (r.gonet 2016-02-23 00:53) - PLID 68408 - That we have to fill rows manually from non-LocationsT fields
				// is dismaying. For this, we needed to retain the combo format for the pharmacies.
				NexTech_Accessor::_ERxPharmacyInfoPtr pPharmacyInfo = m_pCurrentRenewalRequest->GetPharmacyInfo();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_PharmacyCombo->GetNewRow();
				// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID, pPharmacyInfo->GetID());
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Name, pPharmacyInfo->GetName());
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Address, FormatBstr("%s %s", VarString(pPharmacyInfo->GetAddress1()), VarString(pPharmacyInfo->GetAddress2())));
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::City, pPharmacyInfo->GetCity());
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::State, pPharmacyInfo->GetState());
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Zip, pPharmacyInfo->GetZip());
				pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::EPrescribingReady, g_cvarTrue);
				CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
				Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pPharmacyInfo->GetPhoneNumbers());
				sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
				if (!arRenewalRequestPhoneNumbers.IsEmpty())
				{
					for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
					{
						NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
						if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
						{
							// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
							pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Phone, _bstr_t(FormatPhone(VarString(phoneNumber->GetNumber(), ""),
								GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true))));
						}
						else if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "FX")
						{
							// (r.gonet 2016-02-23 12:47) - PLID 67988 - Use the new column enum.
							pRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Fax, _bstr_t(FormatPhone(VarString(phoneNumber->GetNumber(), ""),
								GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true))));
						}
					}
				}
				m_PharmacyCombo->AddRowAtEnd(pRow, NULL);
				m_PharmacyCombo->PutCurSel(pRow);
			}
			//update prescriber fields.
			if (m_pCurrentRenewalRequest->GetPrescriberInfo()) {
				NexTech_Accessor::_ERxPrescriberInfoPtr pPrescriberInfo = m_pCurrentRenewalRequest->GetPrescriberInfo();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProviderCombo->GetNewRow();
				pRow->PutValue(provccID, pPrescriberInfo->GetID());
				pRow->PutValue(provccName, FormatBstr("%s, %s %s", VarString(pPrescriberInfo->GetLastName()), 
					VarString(pPrescriberInfo->GetFirstName()), VarString(pPrescriberInfo->GetMiddleName())));
				// (b.savon 2013-04-09 10:44) - PLID 56085 - The address is now stored in a single column
				CString strAddress2 = VarString(pPrescriberInfo->GetAddress2());
				_bstr_t bstrAddress = FormatBstr("%s%s, %s, %s %s", 
									  VarString(pPrescriberInfo->GetAddress1()), 
									  (strAddress2.IsEmpty() ? "" : " " + strAddress2),
									  VarString(pPrescriberInfo->GetCity()),
									  VarString(pPrescriberInfo->GetState()),
									  VarString(pPrescriberInfo->GetZip()));
				pRow->PutValue(provccAddress, bstrAddress);
				CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
				Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pPrescriberInfo->GetPhoneNumbers());
				sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
				if (!arRenewalRequestPhoneNumbers.IsEmpty())
				{
					for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
					{
						NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
						if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
						{
							pRow->PutValue(provccPhone, _bstr_t(FormatPhone(VarString(phoneNumber->GetNumber(), ""), 
								GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true))));
						}
					}
				}
				m_ProviderCombo->AddRowAtEnd(pRow, NULL);
				m_ProviderCombo->PutCurSel(pRow);

				//Determine if the current user is authorized to work on this renewal based on the prescriber ID.
				NexERxUser erxUser;
				m_lNexERxLicense.GetNexERxUser(erxUser);
				m_bUserAuthorizedPrescriber = (boost::exists(erxUser.aryPrescribingIDs, pPrescriberInfo->GetID()));
			}
			//update supervisor fields
			if (m_pCurrentRenewalRequest->GetSupervisorInfo()) {
				NexTech_Accessor::_ERxSupervisorInfoPtr pSupervisorInfo = m_pCurrentRenewalRequest->GetSupervisorInfo();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_SupervisorCombo->GetNewRow();
				if (VarString(pSupervisorInfo->GetFirstName(), "") != "") {
					pRow->PutValue(provccID, pSupervisorInfo->GetID());
					pRow->PutValue(provccName, FormatBstr("%s, %s %s", VarString(pSupervisorInfo->GetLastName()), VarString(pSupervisorInfo->GetFirstName()), VarString(pSupervisorInfo->GetMiddleName())));
					// (b.savon 2013-04-09 10:44) - PLID 56085 - The address is now stored in a single column
					CString strAddress2 = VarString(pSupervisorInfo->GetAddress2());
					_bstr_t bstrAddress = FormatBstr("%s%s, %s, %s %s", 
										  VarString(pSupervisorInfo->GetAddress1()), 
										  (strAddress2.IsEmpty() ? "" : " " + strAddress2),
										  VarString(pSupervisorInfo->GetCity()),
										  VarString(pSupervisorInfo->GetState()),
										  VarString(pSupervisorInfo->GetZip()));
					pRow->PutValue(provccAddress, bstrAddress);					
					CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
					Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pSupervisorInfo->GetPhoneNumbers());
					sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
					if (!arRenewalRequestPhoneNumbers.IsEmpty())
					{
						for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
						{
							NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
							if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
							{
								pRow->PutValue(provccPhone, _bstr_t(FormatPhone(VarString(phoneNumber->GetNumber(), ""), 
									GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true))));
							}
						}
					}
				} else {
					pRow->PutValue(provccID, pSupervisorInfo->GetID());
					pRow->PutValue(provccName, _bstr_t("<No Supervisor>"));
				}
				m_SupervisorCombo->AddRowAtEnd(pRow, NULL);
				m_SupervisorCombo->PutCurSel(pRow);
			}
			//update agent
			if (m_pCurrentRenewalRequest->GetNurseStaffInfo()) {
				// (b.savon 2013-01-17 09:06) - PLID 54656 - Changed to NurseStaff
				NexTech_Accessor::_ERxNurseStaffInfoPtr pAgentInfo = m_pCurrentRenewalRequest->GetNurseStaffInfo();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_AgentCombo->GetNewRow();
				if (VarString(pAgentInfo->GetFirstName(), "") != "" || VarString(pAgentInfo->GetLastName(), "") != "") {
					pRow->PutValue(provccID, pAgentInfo->GetID());
					pRow->PutValue(provccName, FormatBstr("%s, %s %s", VarString(pAgentInfo->GetLastName()), 
						VarString(pAgentInfo->GetFirstName()), VarString(pAgentInfo->GetMiddleName())));
				} else {
					pRow->PutValue(provccID, pAgentInfo->GetID());
					pRow->PutValue(provccName, _bstr_t("<No Nurse/Staff>"));
				}
				m_AgentCombo->AddRowAtEnd(pRow, NULL);
				m_AgentCombo->PutCurSel(pRow);
			}
			
			//display the prescribed and dispensed medication names.
			m_nxDispensedMedicationLabel.SetType(dtsHyperlink);
			m_nxDispensedMedicationLabel.SetText(DISPENSED_MEDICATION_TEXT);
			m_nxMedicationLabel.SetText(PRESCRIBED_MEDICATION_TEXT);
			SetTextAndToolTip(&m_nxstaticMedicationNameLabel, CString((LPCTSTR)m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->Getdescription()));
			SetTextAndToolTip(&m_nxDispensedMedicationNameLabel, CString((LPCTSTR)m_pCurrentRenewalRequest->GetDispensedMedicationInfo()->Getdescription()));
			
			//determine whether this renewal is a narcotic and if so disallow any approval.
			m_nxRenewalWarning.SetFont(&theApp.m_boldFont);
			CString strNeedsWarned = VarString(m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetDEASchedule(), "");
			if (!strNeedsWarned.IsEmpty()) {
				//display warning.
				m_nxRenewalWarning.SetColor(RGB(255, 50, 50));
				strNeedsWarned = FormatString("This Request is for a %s Controlled Substance!\r\n", strNeedsWarned);
				//disable approval buttons
				m_btnApprove.EnableWindow(FALSE);
				m_btnApproveWithChanges.EnableWindow(FALSE);
			} else {
				m_nxRenewalWarning.SetWindowText("");
			}
			//IsSupply, IsCompound
			if (m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetIsCompoundDrug()) {
				//set display text.
				strNeedsWarned += FormatString("This Request is for a Compound Drug.");
			} else if (m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetIsSupply()) {
				//set display text.
				strNeedsWarned += FormatString("This Request is for a Supply.");
			}
			m_nxRenewalWarning.SetWindowText(strNeedsWarned);

			//update prescribed medication fields.
			DisplayCurrentRenewalMedication();

			//other
			{
				//display renewal status on dialog border text.
				UpdateDialogRenewalStatus(m_pCurrentRenewalRequest->GetQueueStatus(), m_pCurrentRenewalRequest->GetResponseStatus());
				//input response reason into the edit control if we have one.
				CString strResponseReason = Trim(VarString(m_pCurrentRenewalRequest->GetResponseReason(), ""));
				if (!strResponseReason.IsEmpty())
					m_editResponseReason.SetWindowText(strResponseReason);
				else
					m_editResponseReason.SetWindowText("<Pending Response>");
				//remove and show necessary controls.
				EnforceDisabledControls();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-07 16:23) - PLID 53566 - Expose m_bChangesMade
bool CPrescriptionEditDlg::GetChangesMade()
{
	return m_bChangesMade;
}

CString CPrescriptionEditDlg::FormatAddressForPrescription(const CString & Address1, const CString & Address2, const CString & City, 
									 const CString & State, const CString & Zip)
{
	CString strAddress = "";

	if (!Address1.IsEmpty())
	{
		strAddress += FormatString("%s\r\n", Trim(Address1));
	}
	if (!Address2.IsEmpty())
	{
		strAddress += FormatString("%s\r\n", Trim(Address2));
	}
	if (!City.IsEmpty())
	{
		strAddress += FormatString("%s", Trim(City));
	}
	if (!State.IsEmpty())
	{
		if (!City.IsEmpty())
			strAddress += FormatString(", %s ", Trim(State));
		else
			strAddress += FormatString("%s ", Trim(State));
	}
	if (!Zip.IsEmpty())
	{
		strAddress += Trim(Zip);
	}

	return strAddress;
}

// (j.fouts 2013-01-04 11:24) - PLID 54448 - Saves the drug to the pick list if the box is checked
void CPrescriptionEditDlg::SaveToPickList()
{
	//Add to favorites
	if(m_checkAddToFavorites.GetCheck())
	{
		CArray<NexTech_Accessor::_ERxQuickListMedicationPtr, NexTech_Accessor::_ERxQuickListMedicationPtr> aryMedication;
		NexTech_Accessor::_ERxQuickListMedicationPtr pMedication(__uuidof(NexTech_Accessor::ERxQuickListMedication));
		
		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		long nUserID = GetCurrentUserID();
		CString strUserID;
		strUserID.Format("%li", nUserID);
		pMedication->userID = _bstr_t(strUserID);
		pMedication->DrugListID = m_pCurrentPrescription->Medication->DrugListID;
		pMedication->Refills = m_pCurrentPrescription->RefillsAllowed;
		pMedication->Quantity = m_pCurrentPrescription->QuantityValue;
		pMedication->Sig = m_pCurrentPrescription->PatientExplanation;
		pMedication->DosageFrequency = m_pCurrentPrescription->Dosage->DosageFrequency;
		pMedication->DosageQuantity = m_pCurrentPrescription->Dosage->DosageQuantity;
		pMedication->NoteToPharmacist = m_pCurrentPrescription->PharmacistNote;
		pMedication->DosageRoute = NexTech_Accessor::_NexERxDosageRoutePtr(m_pCurrentPrescription->Dosage->DosageRoute);
		pMedication->DosageUnit = NexTech_Accessor::_NexERxDosageUnitPtr(m_pCurrentPrescription->Dosage->DosageUnit);

		aryMedication.Add(pMedication);

		Nx::SafeArray<IUnknown *> saryMedications = Nx::SafeArray<IUnknown *>::From(aryMedication);

		// Call the API
		NexTech_Accessor::_ERxQuickListMedicationArrayPtr pResults = GetAPI()->AddQuickListMedication(GetAPISubkey(), GetAPILoginToken(), saryMedications);
	}
}

// (j.fouts 2013-01-04 11:25) - PLID 54447 - Saves the current fields as the default for the drug if the box is checked
void CPrescriptionEditDlg::SaveAsDefault()
{
	if(m_pCurrentPrescription)
	{
		//Save as default
		if(m_checkSaveAsDefault.GetCheck())
		{
			long nDrugListID = atol(CString((LPCSTR)m_pCurrentPrescription->Medication->DrugListID));
			CString strPatientInstructions((LPCTSTR)m_pCurrentPrescription->PatientExplanation);
			CString strDefaultQuantity((LPCTSTR)m_pCurrentPrescription->QuantityValue);
			CString strNotes((LPCTSTR)m_pCurrentPrescription->PharmacistNote);

			// (j.fouts 2013-03-19 12:06) - PLID 55740 - Added Quantity Unit ID
			CString strQuantityUnitID((LPCTSTR)m_pCurrentPrescription->QuantityUnitID);
			_variant_t varQuantityUnitID = g_cvarNull;
			if(!strQuantityUnitID.IsEmpty())
			{
				varQuantityUnitID = atol(strQuantityUnitID);
			}

			CString strQuantityUnit((LPCTSTR)m_pCurrentPrescription->QuantityUnit);
			
			CString strDosageFrequency = AsString(m_pCurrentPrescription->Dosage->DosageFrequency);
			CString strDosageQuantity = AsString(m_pCurrentPrescription->Dosage->DosageQuantity);
			
			CString strDosageRouteID = AsString(m_pCurrentPrescription->Dosage->DosageRoute->ID);
			CString strDosageUnitID = AsString(m_pCurrentPrescription->Dosage->DosageUnit->ID);
			long nDosageRouteID = atol(strDosageRouteID);
			long nDosageUnitID = atol(strDosageUnitID);

			_variant_t varDosageRouteID = g_cvarNull;
			_variant_t varDosageUnitID = g_cvarNull;

			if(!strDosageRouteID.IsEmpty() || nDosageRouteID > 0)
			{
				varDosageRouteID = nDosageRouteID;
			}

			if(!strDosageUnitID.IsEmpty() || nDosageUnitID > 0)
			{
				varDosageUnitID = nDosageUnitID;
			}

			CString strRefills((LPCTSTR)m_pCurrentPrescription->RefillsAllowed);

			// (j.fouts 2013-03-19 12:06) - PLID 55740 - If QuantityUnitID is null then revert to our old ways
			BOOL bLinkNotesToPharmNotes = GetRemotePropertyInt("DrugListNotesLinkToPharmNotes", 0);

			if(bLinkNotesToPharmNotes)			
			{
				ExecuteParamSql(
				"UPDATE DrugList "
				"SET PatientInstructions = {STRING}, "
				"DefaultQuantity = {STRING}, "
				"Notes = {STRING}, "
				"QuantityUnitID = CASE WHEN {VT_I4} IS NULL THEN (SELECT ID FROM DrugStrengthUnitsT WHERE Name LIKE {STRING}) ELSE {VT_I4} END, "
				"DosageUnitID = {VT_I4}, "
				"DosageRouteID = {VT_I4}, "
				"DosageQuantity = {STRING}, "
				"DosageFrequency = {STRING}, "
				"DefaultRefills = {STRING} "
				"WHERE ID = {INT} ",
				strPatientInstructions, strDefaultQuantity, strNotes, varQuantityUnitID, strQuantityUnit, varQuantityUnitID,
				varDosageUnitID, varDosageRouteID, strDosageQuantity, strDosageFrequency, strRefills, nDrugListID
				);
			}
			else
			{
				ExecuteParamSql(
				"UPDATE DrugList "
				"SET PatientInstructions = {STRING}, "
				"DefaultQuantity = {STRING}, "
				"QuantityUnitID = CASE WHEN {VT_I4} IS NULL THEN (SELECT ID FROM DrugStrengthUnitsT WHERE Name LIKE {STRING}) ELSE {VT_I4} END, "
				"DosageUnitID = {VT_I4}, "
				"DosageRouteID = {VT_I4}, "
				"DosageQuantity = {STRING}, "
				"DosageFrequency = {STRING}, "
				"DefaultRefills = {STRING} "
				"WHERE ID = {INT} ",
				strPatientInstructions, strDefaultQuantity, varQuantityUnitID, strQuantityUnit, varQuantityUnitID,
				varDosageUnitID, varDosageRouteID, strDosageQuantity, strDosageFrequency, strRefills, nDrugListID
				);
			}
		}
	}
}

// (j.jones 2012-11-15 15:38) - PLID 53765 - added OnOk
void CPrescriptionEditDlg::OnOK()
{
	try {

		//CNxDialog::OnOK();
		// (j.jones 2012-11-16 11:26) - PLID 53765 - return the enum for what we did in this dialog,
		// it should be Create, Delete, or Edit (has to be cast as a long though to be used this way)
		EndDialog((long)m_epdrvAction);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-11-15 15:38) - PLID 53765 - added OnCancel
void CPrescriptionEditDlg::OnCancel()
{
	try {
		if (m_pCurrentPrescription) {
			// (j.fouts 2013-01-04 11:25) - PLID 54448 - Save to pick list if it is checked
			SaveToPickList();
			// (j.fouts 2013-01-04 11:25) - PLID 54447 - Save as default if it is checked
			SaveAsDefault();

			if (m_nDenyNewRxResponseID > 0 && m_pCurrentPrescription->status == NexTech_Accessor::PrescriptionStatus_Incomplete)
			{
				if (IDNO == MessageBox("This prescription is a rewrite to a denied renewal request and should be completed as soon as possible."
					"\r\n\r\nAre you sure you want to close the prescription?", "Closing Deny and Rewrite Prescription", MB_YESNO | MB_ICONWARNING))
					return;
			}
		}

		//CNxDialog::OnCancel();
		// (j.jones 2012-11-16 11:26) - PLID 53765 - return the enum for what we did in this dialog,
		// it should be Create, Delete, or Edit (has to be cast as a long though to be used this way)
		EndDialog((long)m_epdrvAction);

	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-11-16 14:18) - PLID 53799 - switches the ui to the dispensed medication information and back when clicked again.
void CPrescriptionEditDlg::DisplayCurrentRenewalMedication()
{
	if (m_pCurrentRenewalRequest) //ensure we still have a valid pointer.
	{
		NexTech_Accessor::_ERxMedicationInfoPtr pMedicationInfo;

		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
		if (m_bShowDispensedMedication) {
			pMedicationInfo = m_pCurrentRenewalRequest->GetDispensedMedicationInfo();
		} else {
			pMedicationInfo = m_pCurrentRenewalRequest->GetPrescribedMedicationInfo(); 
		}

		if (!pMedicationInfo)
			return;
		// (a.wilson 2013-05-01 10:07) - PLID 56509 - disable refills controls if user is not authorized.
		//disable refill controls if the current medication is the dispensed or the response status is anything other than pending.
		if (m_bShowDispensedMedication || !m_bUserAuthorizedPrescriber
			|| m_pCurrentRenewalRequest->GetResponseStatus() != NexTech_Accessor::RenewalResponseStatus_Pending) {
			m_pRefillsList->PutEnabled(VARIANT_FALSE);
			m_nxlabelRefills.SetType(dtsDisabledHyperlink);
		} else {
			m_pRefillsList->PutEnabled(VARIANT_TRUE);
			m_nxlabelRefills.SetType(dtsHyperlink);
		}
		//explanation, notes
		m_editPrescriptionExplanation.SetWindowText(pMedicationInfo->GetDirections());
		m_editEnglishExplanation.SetWindowText(pMedicationInfo->GetDirections());
		m_editNoteToPharmacist.SetWindowText(pMedicationInfo->Getnotes());
		//quantity
		m_editQuantity.SetWindowText(pMedicationInfo->GetQuantity());
		m_pQuantityUnitCombo->SetSelByColumn(qucNCItCode, pMedicationInfo->GetQuantityUnits());
		if (!m_pQuantityUnitCombo->GetCurSel())
		{
			m_pQuantityUnitCombo->SetSelByColumn(qucSureScriptsCode, pMedicationInfo->GetQuantityUnits());
		}
		// (r.farnworth 2016-01-18 09:04) - PLID 67750 - packaging
		m_pPackagingCombo->PutComboBoxText("");
		//strength
		m_editStrength.SetWindowText(pMedicationInfo->GetStrength());
		//strength units
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT Name FROM DrugStrengthUnitsT WHERE NCItCode = {STRING} OR SureScriptsCode = {STRING} "
			"SELECT Name FROM DrugDosageFormsT WHERE NCItCode = {STRING} OR SureScriptsCode = {STRING} ", 
			VarString(pMedicationInfo->GetStrengthUnits(), ""), VarString(pMedicationInfo->GetStrengthUnits(), ""), 
			VarString(pMedicationInfo->GetDosageForm(), ""), VarString(pMedicationInfo->GetDosageForm(), ""));
		if (!prs->eof) {
			m_nxeditStrengthUnits.SetWindowText(AdoFldString(prs, "Name", ""));
		} else {
			m_nxeditStrengthUnits.SetWindowText("");
		}
		//dosage form
		prs = prs->NextRecordset(NULL);
		if (!prs->eof) { 
			m_editDosageForm.SetWindowText(AdoFldString(prs, "Name", ""));
		} else {
			m_editDosageForm.SetWindowText("");
		}
		// (a.wilson 2013-04-08 17:28) - PLID 56141 - rework refills to match new documentation from surescripts.
		//Refills
		{
			CString strApprovedRefills = VarString(m_pCurrentRenewalRequest->GetApprovedRefills(), "0");
			CString strQualifier = VarString(pMedicationInfo->GetRefillQualifier(), "P");
			long nRefills = atol(VarString(pMedicationInfo->GetRefills(), "0"));
			//if the request has a response AND we are not showing the dispensed medication then we want the approved amount of refills.
			if (m_pCurrentRenewalRequest->GetResponseStatus() != NexTech_Accessor::RenewalResponseStatus_Pending && !m_bShowDispensedMedication) {
				if (strApprovedRefills.Compare("PRN") == 0) {
					strQualifier = "PRN";
					nRefills = 0;
				} else {
					strQualifier = "A";
					nRefills = atol(strApprovedRefills);
				}
			}
			//assign the values to the controls.
			if (strQualifier.Compare("PRN") == 0) {	//as needed
				m_pRefillsList->FindByColumn(0, 0, NULL, true);
				ToggleRefillAsNeeded(TRUE);
			} else {	//refill amount
				nRefills = (nRefills > 0 ? (nRefills - 1) : 0);
				if (!m_pRefillsList->FindByColumn(0, nRefills, NULL, true)) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRefillsList->GetNewRow();
					pRow->PutValue(0, nRefills);
					m_pRefillsList->CurSel = m_pRefillsList->AddRowAtEnd(pRow, NULL);
				}
				ToggleRefillAsNeeded(FALSE);
			}
			EnforceRenewalRefillApproval();	// (a.wilson 2013-04-08 17:49) - PLID 56141
		}

		m_editDaysSupply.SetWindowText(pMedicationInfo->GetDaysSupply());

		//substitutions
		if (VarString(pMedicationInfo->GetSubstitution()) == "1") {
			m_checkAllowSubstitutions.SetCheck(BST_UNCHECKED);
		} else {
			m_checkAllowSubstitutions.SetCheck(BST_CHECKED);
		}
		//written date
		COleDateTime dtWrittenDate = pMedicationInfo->GetWrittenDate();
		if (dtWrittenDate && dtWrittenDate.GetStatus() == COleDateTime::valid)
			m_nxtSampleExpirationDate->SetDateTime(dtWrittenDate);
		else
			m_nxtSampleExpirationDate->Clear();
		//priorauthorization
		m_editPriorAuthorization.SetWindowText(VarString(pMedicationInfo->GetPriorAuthorization(), ""));
	}
}

// (b.savon 2012-11-20 15:50) - PLID 53831 - Send prescription
void CPrescriptionEditDlg::OnBnClickedEsubmitButton()
{
	try{
		// (j.fouts 2013-04-23 10:53) - PLID 52907 - Warn when sending
		switch(m_pCurrentPrescription->status)
		{
			case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
			case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
			case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
			case NexTech_Accessor::PrescriptionStatus_eFaxed:
			case NexTech_Accessor::PrescriptionStatus_Legacy:
				//This should never happen, the controls should be disable is this is the case
				//If you hit this then try to determine why the eSubmit button was enabled, even
				//though its status is one that cannot be eSubmitted.
				ASSERT(FALSE);
				MessageBox(FormatString("You cannot eSubmit a prescription with a status of %s.", QueueStatusTextFromID(m_pCurrentPrescription->status)), "Error", MB_ICONERROR|MB_OK);
				return;
			case NexTech_Accessor::PrescriptionStatus_Printed:
				if(MessageBox("This prescription has already been printed. If you have distributed a copy of this prescription "
					"to the patient, please do not send it electronically as well. This can result in the patient having multiple prescriptions of the "
					"medication.\n\rWould you like to continue sending?", "Continue sending?", MB_YESNO|MB_ICONQUESTION) != IDOK)
				{
					return;
				}
				break;
			default:
				if(DontShowMeAgain(this, "If you continue electronically submitting this prescription you will no longer be able "
					"to edit, delete, eFax, or electronically submit this prescription.\n\r"
					"Are you sure you wish to continue electronically submitting this prescription?"
					, "NexERx_SendLockWarning", "Warning", FALSE, TRUE) == IDNO)
				{
					return;
				}
				break;
		}

		// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
		if(!PromptInteractions())
		{
			return;
		}

		// (j.jones 2016-02-22 14:05) - PLID 68354 - display the review screen
		CErxPrescriptionReviewDlg dlgReview(this, GetNxColor(GNC_PATIENT_STATUS, 1));
		dlgReview.m_aryPrescriptionIDs.push_back((long)AsLong((LPCTSTR)m_pCurrentPrescription->PrescriptionID));
		if (IDCANCEL == dlgReview.DoModal()) {
			return;
		}

		// (j.fouts 2012-12-27 16:50) - PLID 53160 - Moved setting the queue status to another method
		CArray<_bstr_t,_bstr_t> aryPrescriptionID;

		// (j.fouts 2013-02-08 09:06) - PLID 51712 - Changed API IDs to be string
		aryPrescriptionID.Add(m_pCurrentPrescription->PrescriptionID);

		Nx::SafeArray<BSTR> saryPrescriptionIDs = Nx::SafeArray<BSTR>::From(aryPrescriptionID);
		NexTech_Accessor::_UpdatePrescriptionStatusOutputPtr pOutput = GetAPI()->UpdatePrescriptionStatus(GetAPISubkey(), GetAPILoginToken(),
			saryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized);

		if( pOutput == NULL || pOutput->Results == NULL ){
			MessageBox("Unable to submit prescription.", "NexTech Practice", MB_ICONERROR);
			return;
		}

		Nx::SafeArray<IUnknown *> saryResults(pOutput->Results);
		foreach(NexTech_Accessor::_UpdatePrescriptionStatusResultPtr pResult, saryResults)
		{
			if(!strcmp(pResult->PrescriptionID, m_pCurrentPrescription->PrescriptionID))
			{
				//Update the status on this prescription
				m_pCurrentPrescription->status = pResult->NewPrescriptionQueueStatus;
				m_bChangesMade = true;
			}
		}
		
		// (j.fouts 2012-12-27 16:50) - PLID 53160 - Hide/Show controls based on the new status
		TryEnablePendingControls();

		OnCancel();
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-11-26 11:45) - PLID 53799 - approval
void CPrescriptionEditDlg::OnBnClickedApproveRenewal()
{
	try {
		//check to ensure that a patient is linked to this renewal.
		if (m_pCurrentRenewalRequest->GetPatientInfo()->GetID() == -1)
		{
			AfxMessageBox("You must link a patient before approving this renewal request.");
			return;
		}
		CString strRefills;
		if (m_nxlabelRefills.GetText() == REFILL_AS_NEEDED_TEXT) {
			strRefills = "PRN";
		} else if (m_pRefillsList->GetCurSel() && AsLong(m_pRefillsList->GetCurSel()->GetValue(0)) >= 0) {
			strRefills = FormatString("%li", (AsLong(m_pRefillsList->GetCurSel()->GetValue(0)) + 1));
		} else {
			AfxMessageBox("You must specify a valid refill count for approvals.");
			return;
		}

		//generate a dialog for the user to free text an eplanation of the changes they would like.
		CRenewalResponseReasonDlg dlg(this);

		if (IDOK == dlg.PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus_Approved))
		{
			int nResponseID = CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized, 
				NexTech_Accessor::RenewalResponseStatus_Approved, strRefills, dlg.m_strDetailedReason, dlg.m_arReasonSelections);
			if (nResponseID <= 0)	//cancel operation if renewal already has a response.
				return;
			else
				EndDialog(0);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-11-26 11:45) - PLID 53799 - approval with changes
void CPrescriptionEditDlg::OnBnClickedApproveWithChangesRenewal()
{
	try {
		//check to ensure that a patient is linked to this renewal.
		if (m_pCurrentRenewalRequest->GetPatientInfo()->GetID() == -1)
		{
			AfxMessageBox("You must link a patient before approving this renewal request.");
			return;
		}
		CString strRefills;
		if (m_nxlabelRefills.GetText() == REFILL_AS_NEEDED_TEXT) {
			strRefills = "PRN";
		} else if (m_pRefillsList->GetCurSel() && AsLong(m_pRefillsList->GetCurSel()->GetValue(0)) >= 0) {
			strRefills = FormatString("%li", (AsLong(m_pRefillsList->GetCurSel()->GetValue(0)) + 1));
		} else {
			AfxMessageBox("You must specify a valid refill count for approvals.");
			return;
		}

		//generate a dialog for the user to free text an eplanation of the changes they would like.
		CRenewalResponseReasonDlg dlg(this);

		if (IDOK == dlg.PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges))
		{
			int nResponseID = CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized, 
				NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges, strRefills, dlg.m_strDetailedReason, dlg.m_arReasonSelections);
			if (nResponseID <= 0)	//cancel operation if renewal already has a response.
				return;
			else
				EndDialog(0);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-11-26 11:45) - PLID 53799 - deny
void CPrescriptionEditDlg::OnBnClickedDenyRenewal()
{
	try {
		//ensure they want to choose this option.
		CRenewalResponseReasonDlg dlg(this);

		if (IDOK == dlg.PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus_Denied)) {
			
			int nResponseID = CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized, 
				NexTech_Accessor::RenewalResponseStatus_Denied, "0", dlg.m_strDetailedReason, dlg.m_arReasonSelections);
			if (nResponseID <= 0)	//cancel operation if renewal already has a response.
				return;
			else
				EndDialog(0);
		}	
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-10 17:43) - PLID 53799 - deny and rewrite response.
void CPrescriptionEditDlg::OnBnClickedDenyAndRewriteRenewal()
{
	try {
		// (a.wilson 2013-02-05 11:25) - PLID 55014 - this option will require a larger amount of work in order to perform.
		//check to ensure that a patient is linked to this renewal.
		if (m_pCurrentRenewalRequest->GetPatientInfo()->GetID() == -1)
		{
			AfxMessageBox("You must link a patient before rewriting this renewal request.");
			return;
		}
		//determine if we need to import the pharmacy into data for the newrx.
		if (m_pCurrentRenewalRequest->GetPharmacyInfo()->GetID() == -1) {
			if (!ImportPharmacyFromRenewal()) {
				MsgBox("Invalid pharmacy identification number.  This renewal request cannot be denied and rewritten.");
				m_btnDenyAndRewrite.EnableWindow(FALSE);
				return;
			}
		}

		//ensure they want to choose this option.
		CRenewalResponseReasonDlg dlg(this);
		if (IDOK == dlg.PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus_DeniedNewRx)) {

			//generate newrx and save to data.
			CMedicationSelectDlg medSelectDlg(m_pParent);
			medSelectDlg.SetPatientID(m_pCurrentRenewalRequest->GetPatientInfo()->GetID());
			medSelectDlg.SetInitialMedicationID(m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetID());
			//medSelectDlg.
			if (IDOK == medSelectDlg.DoModal()) {
				//commit renewal so that we have a responseid for the newrx to reference.
				int nResponseID = CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized, 
					NexTech_Accessor::RenewalResponseStatus_DeniedNewRx, "0", dlg.m_strDetailedReason, dlg.m_arReasonSelections);
				if (nResponseID <= 0)	//cancel operation if renewal already has a response.
					return;
				//create new prescription.
				CPrescriptionEditDlg newRxDlg(this);

				NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = SaveNewPrescription(medSelectDlg.m_nMedicationID, TRUE, 
					NULL, medSelectDlg.GetPatientID(), -1, GetCurrentLocation(), COleDateTime::GetCurrentTime(), -1, FALSE, SourceActionInfo(), 
					m_pCurrentRenewalRequest->GetPharmacyInfo()->GetID(), nResponseID);

				// (b.savon 2013-03-18 17:24) - PLID 55477
				if( pResults->PrescriptionsAdded == NULL ){
					ThrowNxException("Unable to Add Prescription in CPrescriptionEditDlg::OnBnClickedDenyAndRewriteRenewal!");
				}

				Nx::SafeArray<IUnknown*> saryPrescriptions(pResults->PrescriptionsAdded);
				if(saryPrescriptions.GetCount() == 0)
				{
					ThrowNxException("Attempted to create a new prescription, but no prescription was returned."); 
				}

				// (b.savon 2013-03-18 17:25) - PLID 55477 - use Struct
				PrescriptionInfo rxInformation;
				NexTech_Accessor::_QueuePrescriptionPtr pPrescription = saryPrescriptions[0];
				rxInformation.pPrescription = pPrescription;
				rxInformation.erxUserRole = pResults->UserRole;
				rxInformation.saryPrescribers = pResults->Prescriber;
				rxInformation.sarySupervisors = pResults->Supervisor;
				rxInformation.saryNurseStaff = pResults->NurseStaff;

				// (j.jones 2013-11-25 09:55) - PLID 59772 - for new prescriptions we pass in the drug interactions info
				DrugInteractionInfo drugInteractionInfo;
				if(pResults->DrugDrugInteracts) {
					drugInteractionInfo.saryDrugDrugInteracts = pResults->DrugDrugInteracts;
				}
				if(pResults->DrugAllergyInteracts) {
					drugInteractionInfo.saryDrugAllergyInteracts = pResults->DrugAllergyInteracts;
				}
				if(pResults->DrugDiagnosisInteracts) {
					drugInteractionInfo.saryDrugDiagnosisInteracts = pResults->DrugDiagnosisInteracts;
				}

				// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx, true here because we are writing a new revision of the Rx
				m_epdrvAction = (PrescriptionDialogReturnValue)newRxDlg.EditPrescription(true, rxInformation, &drugInteractionInfo);
				EndDialog(m_epdrvAction);
			}
		}	
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-02-04 17:00) - PLID 55014 - need to get the pharmacies locationid in order to lock the destination.
bool CPrescriptionEditDlg::ImportPharmacyFromRenewal()
{
	//api pointer
	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	//create pharmacy object to pass to api
	NexTech_Accessor::_PharmacyImportInputPtr pPharmacy(__uuidof(NexTech_Accessor::PharmacyImportInput));
	pPharmacy->PutNCPDPID(_bstr_t(m_pCurrentRenewalRequest->GetPharmacyInfo()->GetNCPDPID()));
	pPharmacy->PutPharmacyName(_bstr_t(m_pCurrentRenewalRequest->GetPharmacyInfo()->GetName()));
	//pass object to api and get results back
	NexTech_Accessor::_PharmacyImportResultsArrayPtr pPharmaciesArray = pApi->PharmacyImport(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown *>::FromValue(pPharmacy));
	CArray<NexTech_Accessor::_PharmacyImportOutputPtr> arImportedPharmacies;
	Nx::SafeArray<IUnknown *> sarImportedPharmacies(pPharmaciesArray->PharmacyImportResults);
	sarImportedPharmacies.ToArray(arImportedPharmacies);
	//check the locationid to make sure we imported.
	long nLocationID = arImportedPharmacies.GetAt(0)->GetlocationID();
	if (nLocationID <= 0) {
		return false;	//bad id, return false.
	}
	//commit to database.
	m_pCurrentRenewalRequest->GetPharmacyInfo()->PutID(nLocationID);
	ExecuteParamSql("UPDATE RenewalRequestsT SET PharmacyID = {INT} WHERE ID = {INT}", 
		m_pCurrentRenewalRequest->GetPharmacyInfo()->GetID(), m_pCurrentRenewalRequest->GetID());
	//success. return true.
	return true;
}

// (a.wilson 2012-12-04 09:24) - PLID 53799
long CPrescriptionEditDlg::CommitRenewalResponse(NexTech_Accessor::PrescriptionStatus queueStatus, NexTech_Accessor::RenewalResponseStatus responseStatus, 
												 const CString &strRefills, const CString &strDetailedReason, const CArray<_bstr_t> &arReasons)
{
	//send the renewal response to the api.
	NexTech_Accessor::_ERxRenewalResponseCommitPtr pRenewalResponse(__uuidof(NexTech_Accessor::ERxRenewalResponseCommit));
	pRenewalResponse->PutID(m_pCurrentRenewalRequest->GetID());
	pRenewalResponse->PutQueueStatus(queueStatus);
	pRenewalResponse->PutResponseStatus(responseStatus);
	pRenewalResponse->PutRefills(_bstr_t(strRefills));
	pRenewalResponse->PutDetailedReason(_bstr_t(strDetailedReason));
	if (!arReasons.IsEmpty()) 
		pRenewalResponse->PutSureScriptsReasonCodes(Nx::SafeArray<BSTR>::From(arReasons));
	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();

	if (pApi)
	{
		NexTech_Accessor::_ERxCommitRenewalRequestResponseResultPtr pResult = pApi->CommitRenewalRequestResponse(GetAPISubkey(), 
			GetAPILoginToken(), pRenewalResponse);

		if (!pResult)
			ThrowNxException("Unable to Commit Renewal Response in CPrescriptionEditDlg::CommitRenewalResponse!");
		else if (pResult->GetResponseID() <= 0) {
			AfxMessageBox("This renewal request already has a response by another user.");
			UpdateRenewalRequestInfo();
		}
		return pResult->GetResponseID();
	}
	return 0;
}

// (a.wilson 2012-11-30 11:46) - PLID 53799
void CPrescriptionEditDlg::UpdateDialogRenewalStatus(NexTech_Accessor::PrescriptionStatus queueStatus, NexTech_Accessor::RenewalResponseStatus responseStatus)
{
	CString strDialogText = "View Renewal Request  ";

	switch (responseStatus)
	{
		case NexTech_Accessor::RenewalResponseStatus_Pending:
			strDialogText += "(Pending";
			break;
		case NexTech_Accessor::RenewalResponseStatus_Approved:
			strDialogText += "(Approved";
			break;
		case NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges:
			strDialogText += "(Approved With Changes";
			break;
		case NexTech_Accessor::RenewalResponseStatus_Denied:
			strDialogText += "(Denied";
			break;
		case NexTech_Accessor::RenewalResponseStatus_DeniedNewRx:
			strDialogText += "(Denied and Rewrite";
			break;
		default:
			//shouldn't happen
			ASSERT(FALSE);
			break;
	}

	switch (queueStatus)
	{
		case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
			strDialogText += (" - eTransmit Authorized)");
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
			strDialogText += (" - eTransmit Pending)");
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
			strDialogText += (" - eTransmit Success)");
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitError:
			strDialogText += (" - eTransmit Error)");
			break;
		default:
			strDialogText += (")");
			break;
	}

	SetWindowText(strDialogText);
}

// (a.wilson 2012-11-30 11:46) - PLID 53799 - This will open the original prescription if a valid one is available.
void CPrescriptionEditDlg::OnBnClickedOriginalPrescriptionBtn()
{
	try {
		//ensure we do have an ID
		if (m_pCurrentRenewalRequest->GetOriginalPrescriptionID() > 0)
		{
			long nPrescriptionID = m_pCurrentRenewalRequest->GetOriginalPrescriptionID();
			CPrescriptionEditDlg dlg(this);
			// (b.savon 2013-03-18 17:28) - PLID 55477
			PrescriptionInfo rxInformation = LoadFullPrescription(nPrescriptionID);
			// (b.savon 2014-08-19 15:12) - PLID 63403 - This should be NULL; we don't need to popup interactions for past prescriptions.
			// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
			dlg.EditPrescription(false, rxInformation, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-10 10:53) - PLID 54463 - Open the next combobox when the select something to create a easier flow
void CPrescriptionEditDlg::SelChosenPrescriptionQuantity(LPDISPATCH lpRow)
{
	try
	{
		DropDownNextEmptyList(IDC_PRESCRIPTION_DOSAGE_UNIT);
	}
	NxCatchAll(__FUNCTION__);
}
// (j.fouts 2013-01-10 10:54) - PLID 54526 - Save the dosage qty when sel changes
void CPrescriptionEditDlg::SelChosenPrescriptionDosage(LPDISPATCH lpRow)
{
	try
	{
		if (!m_pCurrentPrescription)
		{
			return;
		}

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitDosageQuantity = VARIANT_TRUE;

		CString strDosageQuantity = "";
		double dDosageQuantity = -1.0;

		if(lpRow)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			strDosageQuantity = VarString(pRow->GetValue(dqlcSigText), "");
			dDosageQuantity = VarFloat(pRow->GetValue(dqlcValue), -1.0);
		}

		// (j.fouts 2013-02-06 17:53) - PLID 51712 - Changed ID fields to be passed as strings
		m_pCurrentPrescription->Dosage->DosageQuantity = _bstr_t(strDosageQuantity);

		m_pAutoPopulatingSig->SetDosageQuantity(strDosageQuantity, dDosageQuantity);
		UpdateSig(pCommit);
		Save(pCommit);

		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Open the next combobox when the select something to create a easier flow
		DropDownNextEmptyList(IDC_PRESCRIPTION_DOSAGE_UNIT);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-10 10:55) - PLID 54528 - Save the dosage method when sel changes
void CPrescriptionEditDlg::SelChosenPrescriptionMethod(LPDISPATCH lpRow)
{
	try
	{
		if (!m_pCurrentPrescription)
		{
			return;
		}

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitDosageRouteID = VARIANT_TRUE;

		CString strRouteID = "";
		CString strRoute = "";
		CString strRouteSig = "";

		if(m_pRouteList->CurSel)
		{
			long nDosageRouteID = VarLong(m_pRouteList->CurSel->GetValue(drlcID), -1);
			if(nDosageRouteID > 0)
			{
				strRouteID.Format("%li", nDosageRouteID);
				strRoute = AsString(m_pRouteList->CurSel->GetValue(drlcRoute));
				strRouteSig = AsString(m_pRouteList->CurSel->GetValue(drlcAction));
			}
		}

		m_pCurrentPrescription->Dosage->DosageRoute->ID = _bstr_t(strRouteID);
		m_pCurrentPrescription->Dosage->DosageRoute->Route = _bstr_t(strRoute);
		m_pCurrentPrescription->Dosage->DosageRoute->SigText = _bstr_t(strRouteSig);
	
		//Update the sig before saving
		m_pAutoPopulatingSig->SetRoute(strRouteSig);
		UpdateSig(pCommit);
		Save(pCommit);

		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Open the next combobox when the select something to create a easier flow
		DropDownNextEmptyList(IDC_PRESCRIPTION_FREQUENCY);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-10 10:56) - PLID 54529 - Save the dosage freq when sel changes
void CPrescriptionEditDlg::SelChosenPrescriptionFrequency(LPDISPATCH lpRow)
{
	try
	{
		if (!m_pCurrentPrescription)
		{
			return;
		}

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitDosageFrequency = VARIANT_TRUE;

		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		CString strDosageFrequency = "";
		float fDosageFrequency = -1.0f;
		if(lpRow)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			strDosageFrequency = VarString(pRow->GetValue(flcSigText), "");
			fDosageFrequency = VarFloat(pRow->GetValue(flcValue), -1.0);
		}

		m_pCurrentPrescription->Dosage->DosageFrequency = _bstr_t(strDosageFrequency);

		m_pAutoPopulatingSig->SetFrequency(strDosageFrequency, fDosageFrequency);
		UpdateSig(pCommit);
		Save(pCommit);

		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Open the next combobox when the select something to create a easier flow
		// (r.farnworth 2016-01-18 14:20) - PLID 67750 - Packaging moved to after Frequency
		// (b.savon 2016-01-20 09:39) - PLID 59879 - Go to refills if packaging is disabled.
		if (m_pAutoPopulatingSig->GetAutoCalculate()) {
			DropDownNextEmptyList(IDC_PRESCRIPTION_REFILLS);
		}
		else{
			DropDownNextEmptyList(IDC_PACKAGING_INFO);
		}

	}
	NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::UpdateDaysSupply(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	long nDaysSupply;
	if(m_pAutoPopulatingSig->GetDaysSupply(&nDaysSupply))
	{
		CString strDaysSupply;
		strDaysSupply.Format("%li", nDaysSupply);
		m_editDaysSupply.SetWindowText(strDaysSupply);
		if(m_pCurrentPrescription->DaysSupply->IsNull() ||
			(VarLong(m_pCurrentPrescription->DaysSupply->GetValue(), -1) != nDaysSupply))
		{
			if(pCommit)
			{
				pCommit->CommitDaysSupply = VARIANT_TRUE;
			}
			m_pCurrentPrescription->DaysSupply->SetInt(nDaysSupply);
		}
	}
}

// (j.fouts 2013-01-10 10:53) - PLID 54463 - Updates the sig to the auto generated one only if m_bDontGenerateSig is false
// (j.fouts 2013-02-01 15:06) - PLID 54985 - Modified to also set the save flags/values in a commit ptr when sig/qty change
void CPrescriptionEditDlg::UpdateSig(NexTech_Accessor::_PrescriptionCommitPtr pCommit)
{
	CString strSig;
	if(m_pAutoPopulatingSig->GetSig(strSig))
	{
		m_editPrescriptionExplanation.SetWindowText(strSig);
		if(CString((LPCTSTR)m_pCurrentPrescription->PatientExplanation).Compare(strSig) != 0)
		{
			if(pCommit)
			{
				pCommit->CommitPatientExplanation = VARIANT_TRUE;
			}
			
			m_pCurrentPrescription->PatientExplanation = _bstr_t(strSig);
		}
	}

	double dQuantity;
	if(m_pAutoPopulatingSig->GetQuantity(&dQuantity))
	{
		CString strQuantity;
		// (b.savon 2016-03-16 10:30) - PLID 68600 - Use the proper format specifier for double
		strQuantity.Format("%g", dQuantity);
		m_editQuantity.SetWindowText(strQuantity);
		if(CString((LPCTSTR)m_pCurrentPrescription->QuantityValue).Compare(strQuantity) != 0)
		{
			if(pCommit)
			{
				pCommit->CommitQuantity = VARIANT_TRUE;
			}
			m_pCurrentPrescription->QuantityValue = _bstr_t(strQuantity);
		}
	}
}

// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
// (b.savon 2013-01-17 08:51) - PLID 54632
void CPrescriptionEditDlg::LoadPrescriberCombo()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	// Put the blank row
	pRow = m_ProviderCombo->GetNewRow();	
	pRow->PutValue(provccID, (long)-1);
	pRow->PutValue(provccName, (LPCTSTR)"< No Provider Selected >");
	m_ProviderCombo->AddRowSorted(pRow, NULL);

	switch( m_urtCurrentUserRole ){
		case urtLicensedPrescriber:
		case urtMidlevelPrescriber:	
			{
				// Populate the licensed prescriber as the 'Prescriber' and Save
				pRow = m_ProviderCombo->GetNewRow();		
				// (b.savon 2013-03-18 17:30) - PLID 55477
				NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pPrescriber = m_saryPrescribers[0];
				if( pPrescriber == NULL ){
					MessageBox("Unable to populate prescriber.", "NexTech Practice", MB_ICONERROR);
					return;
				}
				pRow->PutValue(provccID, pPrescriber->personID);
				pRow->PutValue(provccName, pPrescriber->PersonName);
				// (b.savon 2013-04-09 10:44) - PLID 56085 - Store address and phone in dl
				pRow->PutValue(provccAddress, pPrescriber->PersonAddress);
				CString strFormatPhoneString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
				_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(VarString(pPrescriber->PersonPhoneNumber, ""), strFormatPhoneString));
				pRow->PutValue(provccPhone, bstrPhoneNumber);
				m_ProviderCombo->AddRowSorted(pRow, NULL);

				//If we have a provider set on the Rx, do it, otherwise select the configured user role prescriber
				if( m_pCurrentPrescription->Provider->personID != _bstr_t() ){
					// If this Rx was written by a different prescriber and we can't find them in the drop down, 
					// then we take ownership and save it to data.
					if( !m_ProviderCombo->FindByColumn(provccID, m_pCurrentPrescription->Provider->personID, NULL, VARIANT_TRUE) ){
						m_ProviderCombo->SetSelByColumn(provccID, pPrescriber->personID);
						SavePrescriber(atol(CString((LPCTSTR)pPrescriber->personID)));
					}
				}else{
					m_ProviderCombo->SetSelByColumn(provccID, pPrescriber->personID);
					SavePrescriber(atol(CString((LPCTSTR)pPrescriber->personID)));
				}
			}
			break;
		case urtNurseStaff:
			{
				//Populate both the Licensed Prescribers and Midlevels as the 'Prescriber' and Save
				//If there is more than one, don't default the selection.  Once the user selects
				//a prescriber, if it is a midlevel, the supervisor drop down will be populated with all the prescribers
				//the midlevel can prescribe for.  To view this look at the SelChanged event handler for the ProviderCombo
				NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pPrescriber;
				for( int idx = 0; idx < (int)m_saryPrescribers.GetCount(); idx++ ){
					pRow = m_ProviderCombo->GetNewRow();
					pPrescriber = m_saryPrescribers[idx];
					if( pPrescriber == NULL ){
						MessageBox("Unable to populate prescriber.", "NexTech Practice", MB_ICONERROR);
						return;
					}
					pRow->PutValue(provccID, pPrescriber->personID);
					pRow->PutValue(provccName, pPrescriber->PersonName);
					// (b.savon 2013-04-09 10:44) - PLID 56085 - Store address and phone in dl
					pRow->PutValue(provccAddress, pPrescriber->PersonAddress);
					CString strPhoneFormatString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
					_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(VarString(pPrescriber->PersonPhoneNumber, ""), strPhoneFormatString)); 
					pRow->PutValue(provccPhone, bstrPhoneNumber);
					m_ProviderCombo->AddRowSorted(pRow, NULL);
				}

				//If the Nurse/Staff only prescribes for 1 person, default it.  Otherwise, there will be no default.
				if( m_pCurrentPrescription->Provider->personID != _bstr_t() ){
					if( !(pRow = m_ProviderCombo->FindByColumn(provccID, m_pCurrentPrescription->Provider->personID, NULL, VARIANT_TRUE)) ){
						if( (int)m_saryPrescribers.GetCount() == 1 ){
							pRow = m_ProviderCombo->SetSelByColumn(provccID, pPrescriber->personID);
							PrepareInterfaceForSupervisorSelection(pRow);
							SavePrescriber(atol(CString((LPCTSTR)pPrescriber->personID)));
						}else{
							m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
							SavePrescriber(-1);
						}
					}else{
						PrepareInterfaceForSupervisorSelection(pRow);
					}
				}else{
					if( (int)m_saryPrescribers.GetCount() == 1 ){
						m_ProviderCombo->SetSelByColumn(provccID, pPrescriber->personID);
						SavePrescriber(atol(CString((LPCTSTR)pPrescriber->personID)));
					}else{
						m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
					}
				}
			}
			break;
		case urtNone:
		default:
			{
				// They can't e-prescribe - Disable the button and populate legacy
				m_btnESubmit.EnableWindow(FALSE);
				m_btnEFax.EnableWindow(FALSE);
				m_ProviderCombo->PutFromClause(AsBstr("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID"));
				m_ProviderCombo->PutWhereClause(AsBstr("Archived = 0"));
				m_ProviderCombo->Requery();
				// Put the blank row
				pRow = m_ProviderCombo->GetNewRow();	
				pRow->PutValue(provccID, (long)-1);
				pRow->PutValue(provccName, (LPCTSTR)"< No Provider Selected >");
				m_ProviderCombo->AddRowSorted(pRow, NULL);
				m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
			}
	}
}

// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
// (b.savon 2013-01-17 08:54) - PLID 54632
void CPrescriptionEditDlg::LoadSupervisorCombo()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	// Put the blank row
	pRow = m_SupervisorCombo->GetNewRow();	
	pRow->PutValue(provccID, (long)-1);
	pRow->PutValue(provccName, (LPCTSTR)"< No Supervisor Selected >");
	m_SupervisorCombo->AddRowSorted(pRow, NULL);

	switch( m_urtCurrentUserRole ){
		case urtLicensedPrescriber:
			{
				// Licensed Prescribers don't have a supervisor

				GetDlgItem(IDC_STATIC_SUPERVISOR)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_SUPERVISOR)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->EnableWindow(FALSE);
				GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->ShowWindow(SW_HIDE);

				// (b.savon 2013-03-20 17:07) - PLID 55477 - handle the case where we take ownership
				//If Rx was written by a Nurse/Staff or Midlevel originally, but opened by a licensed prescriber.
				//Save the Rx with no Supervisor.	
				if( m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
					SaveSupervisor(-1);
				}
			}
			break;
		case urtMidlevelPrescriber:
			{
				//Populate all the LicensedPrescribers (Supervisors) this Midlevel can prescriber under
				NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pSupervisor;
				for( int idx = 0; idx < (int)m_sarySupervisors.GetCount(); idx++ ){
					pSupervisor = m_sarySupervisors[idx];
					if( pSupervisor == NULL ){
						MessageBox("Unable to populate supervisor.", "NexTech Practice", MB_ICONERROR);
						return;
					}
					pRow = m_SupervisorCombo->GetNewRow();
					pRow->PutValue(provccID, pSupervisor->personID);
					pRow->PutValue(provccName, pSupervisor->PersonName);
					// (b.savon 2013-04-09 10:44) - PLID 56085 - Store address and phone in dl
					pRow->PutValue(provccAddress, pSupervisor->PersonAddress);
					CString strFormatPhoneString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
					_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(VarString(pSupervisor->PersonPhoneNumber, ""), strFormatPhoneString));
					pRow->PutValue(provccPhone, bstrPhoneNumber);
					m_SupervisorCombo->AddRowSorted(pRow, NULL);
				}
				
				//If we have a supervisor set on the Rx, do it, otherwise if there is 1 supervisor in the filter, do it, 
				//or select the < No Supervisor > row.
				if( m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
					// If this Rx had a different supervisor when written and we can't find them in the drop down, 
					// then we take ownership and save it to data.
					if( !m_SupervisorCombo->FindByColumn(provccID, m_pCurrentPrescription->Supervisor->personID, NULL, VARIANT_TRUE) ){
						if( (int)m_sarySupervisors.GetCount() == 1 ){
							m_SupervisorCombo->SetSelByColumn(provccID, pSupervisor->personID);
							SaveSupervisor(atol(CString((LPCTSTR)pSupervisor->personID)));
						}else{
							m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
							SaveSupervisor(-1);
						}
					}
				}else{
					if( (int)m_sarySupervisors.GetCount() == 1 ){
						m_SupervisorCombo->SetSelByColumn(provccID, pSupervisor->personID);
						SaveSupervisor(atol(CString((LPCTSTR)pSupervisor->personID)));
					}else{
						m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
					}
				}
			}
			break;
		case urtNurseStaff:
			{
				//Nurse/Staff have supervisors but they are initially held in another column and if they happen
				//to choose a midlevel, then the supervisor combo will appear and it will be filtered based
				//on the midlevel's supervisors.

				GetDlgItem(IDC_STATIC_SUPERVISOR)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_SUPERVISOR)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->EnableWindow(FALSE);
				GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->ShowWindow(SW_HIDE);
			}
			break;
		case urtNone:
		default:
			{
				// They can't e-prescribe - Disable the button and populate legacy
				m_btnESubmit.EnableWindow(FALSE);
				m_btnEFax.EnableWindow(FALSE);
				m_SupervisorCombo->PutFromClause(AsBstr("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID"));
				m_SupervisorCombo->PutWhereClause(AsBstr("Archived = 0"));
				m_SupervisorCombo->Requery();
				// Put the blank row
				pRow = m_SupervisorCombo->GetNewRow();	
				pRow->PutValue(provccID, (long)-1);
				pRow->PutValue(provccName, (LPCTSTR)"< No Supervisor Selected >");
				m_SupervisorCombo->AddRowSorted(pRow, NULL);
				m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
			}
	}
}

// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
// (b.savon 2013-01-17 08:55) - PLID 54632
void CPrescriptionEditDlg::LoadNurseStaffCombo()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	// Put the blank row
	pRow = m_AgentCombo->GetNewRow();	
	pRow->PutValue(provccID, (long)-1);
	pRow->PutValue(provccName, (LPCTSTR)"< No Nurse/Staff Selected >");
	m_AgentCombo->AddRowSorted(pRow, NULL);	

	switch( m_urtCurrentUserRole ){
		case urtLicensedPrescriber:
		case urtMidlevelPrescriber:
			{
				// Licensed Prescribers don't have a nurse/staff

				//Midlevels don't have a nurse/staff
				
				GetDlgItem(IDC_STATIC_AGENT)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_AGENT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MED_PRESCRIBER_AGENT_COMBO)->EnableWindow(FALSE);
				GetDlgItem(IDC_MED_PRESCRIBER_AGENT_COMBO)->ShowWindow(SW_HIDE);

				// (b.savon 2013-03-20 17:07) - PLID 55477 - handle the case where we take ownership
				//If Rx was written by a Nurse/Staff originally, but opened by someone other than that.
				//Save the Rx with no NurseStaff.
				if( m_pCurrentPrescription->NurseStaff->personID != _bstr_t() ){
					SaveNurseStaff(-1);
				}
			}
			break;
		case urtNurseStaff:
			{
				// This is just the logged in user.
				pRow = m_AgentCombo->GetNewRow();
				NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pNurseStaff = m_saryNurseStaff[0];
				if( pNurseStaff == NULL ){
					MessageBox("Unable to populate nurse/staff.", "NexTech Practice", MB_ICONERROR);
					return;
				}
				pRow->PutValue(provccID, pNurseStaff->personID);
				pRow->PutValue(provccName, pNurseStaff->PersonName);
				// (b.savon 2013-04-09 10:44) - PLID 56085 - Store address and phone in dl
				pRow->PutValue(provccAddress, pNurseStaff->PersonAddress);
				CString strPhoneFormatString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
				_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(VarString(pNurseStaff->PersonPhoneNumber, ""), strPhoneFormatString));
				pRow->PutValue(provccPhone, bstrPhoneNumber);
				m_AgentCombo->AddRowSorted(pRow, NULL);
				
				if( m_pCurrentPrescription->NurseStaff->personID != _bstr_t() ){
					if( !m_AgentCombo->FindByColumn(provccID, m_pCurrentPrescription->NurseStaff->personID, NULL, VARIANT_TRUE) ){
						m_AgentCombo->SetSelByColumn(provccID, pNurseStaff->personID);
						SaveNurseStaff(atol(CString((LPCTSTR)pNurseStaff->personID)));
					}
				}else{
					m_AgentCombo->SetSelByColumn(provccID, pNurseStaff->personID);
					SaveNurseStaff(atol(CString((LPCTSTR)pNurseStaff->personID)));
				}
			}
			break;
		case urtNone:
		default:
			{
				// They can't e-prescribe - Disable the button and populate legacy
				m_btnESubmit.EnableWindow(FALSE);
				m_btnEFax.EnableWindow(FALSE);
				m_AgentCombo->PutFromClause(AsBstr("PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID"));
				m_AgentCombo->PutWhereClause(AsBstr("Archived = 0"));
				m_AgentCombo->Requery();
				// Put the blank row
				pRow = m_AgentCombo->GetNewRow();	
				pRow->PutValue(provccID, (long)-1);
				pRow->PutValue(provccName, (LPCTSTR)"< No Nurse/Staff Selected >");
				m_AgentCombo->AddRowSorted(pRow, NULL);	
				m_AgentCombo->SetSelByColumn(provccID, (long)-1);
			}
	}
}

// (b.savon 2013-01-17 08:56) - PLID 54632 - Utility that takes in a comma delmited ID string and splits into 2 arrays
void CPrescriptionEditDlg::GetNurseStaffPrescriberIDs(CString strIDsIn, CArray<long, long> &arySupervising, CArray<long, long> &aryMidlevel)
{
	CString strSupervising;
	CString strMidlevel;
	GetNurseStaffIdentifiers(strIDsIn, strSupervising, strMidlevel, nsiID);

	// Split and add supervising to array.
	if( !strSupervising.IsEmpty() ){
		StringAsArray((LPCTSTR)strSupervising, arySupervising);
	}

	// Split and add midlevel to array.
	if( !strMidlevel.IsEmpty() ){
		StringAsArray((LPCTSTR)strMidlevel, aryMidlevel);
	}
}

// (b.savon 2013-01-17 08:56) - PLID 54632 - Utility that takes in a delimited name string and splits into 2 arrays
void CPrescriptionEditDlg::GetNurseStaffPrescriberNames(CString strNamesIn, CArray<CString, LPCTSTR> &arySupervising, CArray<CString, LPCTSTR> &aryMidlevel)
{
	CString strSupervising;
	CString strMidlevel;
	GetNurseStaffIdentifiers(strNamesIn, strSupervising, strMidlevel, nsiName);

	// Split and add supervising to array.
	if( !strSupervising.IsEmpty() ){
		SplitNames(strSupervising, arySupervising, ";");
	}

	// Split and add midlevel to array.
	if( !strMidlevel.IsEmpty() ){
		SplitNames(strMidlevel, aryMidlevel, ";");
	}
}

// (b.savon 2013-01-17 08:57) - PLID 54632 - Utility that takes in a string from the database, what identifiers you want,
// and spilts into a string for supervisors and midlevels 
void CPrescriptionEditDlg::GetNurseStaffIdentifiers(CString strIdentifiersIn, CString &strSupervising, CString &strMidlevel, ENurseStaffIdentifier nsiIdentifier)
{
	// There are 4 cases
	// (1)	<NONE>
	// (2)	*S*x, y, z*M*a, b, c
	// (3)	*M*a, b, c
	// (4)	*S*x, y, z

	// Set the correct identifier params
	CString strSupervisorIdentifier;
	CString strMidlevelIdentifier;
	EUserRoleColumns urcDataColumn;
	switch( nsiIdentifier ){
		case nsiID:
			{
				strSupervisorIdentifier = "*S*";
				strMidlevelIdentifier = "*M*";
				urcDataColumn = urcPrescriberIDs;
			}
			break;
		case nsiName:
			{
				strSupervisorIdentifier = "Supervisor(s):";
				strMidlevelIdentifier = "Midlevel(s):";
				urcDataColumn = urcPrescribers;
			}
			break;
		default:
			{
				// Need to handle new identifier
				ASSERT(FALSE);
			}
			break;
	}

	CString strPrescribers = strIdentifiersIn;

	// Case 1 - <NONE>
	// There is no data, get out.
	if( strPrescribers.IsEmpty() ){
		return;
	}

	// Find the supervising and midlevel start positions (if any).
	int nSupervisingStart = strPrescribers.Find(strSupervisorIdentifier);
	int nMidlevelStart = strPrescribers.Find(strMidlevelIdentifier);

	// If we have some midlevel ids, let
	if( nMidlevelStart > -1 ){
		// Case 2 - *S*x, y, z*M*a, b, c
		// If we have some supervisors
		if( nSupervisingStart > -1 ){
			CString strTemp = strPrescribers.Left(nMidlevelStart);
			strTemp.TrimLeft(strSupervisorIdentifier);
			strTemp.Trim();
			strSupervising = strTemp;	
			strTemp = strPrescribers.Right(strPrescribers.GetLength()-nMidlevelStart);
			strTemp.TrimLeft(strMidlevelIdentifier);
			strTemp.Trim();
			strMidlevel = strTemp;
		}else{ // There aren't any supervisors - Case 3 - *M*a, b, c
			strPrescribers.TrimLeft(strMidlevelIdentifier);
			strMidlevel = strPrescribers;
		}
	}else{ // There are just supervisors - Case 4 - *S*x, y, z
		strPrescribers.TrimLeft(strSupervisorIdentifier);
		strSupervising = strPrescribers;
	}
}

// (b.savon 2013-01-17 08:59) - PLID 54632 - Refactored from event handler to Save Prescriber
void CPrescriptionEditDlg::SavePrescriber(long nProviderID)
{
	try{
		NexTech_Accessor::_PrescriptionCommitPtr pPrescription(__uuidof(NexTech_Accessor::PrescriptionCommit));
		NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pProvider(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
		NexTech_Accessor::_QueuePrescriptionPtr pQueueRx(__uuidof(NexTech_Accessor::QueuePrescription));
		NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
		pPrescription->Prescription = pQueueRx;
		pPrescription->Prescription->EMRPrescriptionSource = pEMRPrescriptionSource;

		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		pPrescription->CommitProviderID = true;

		CString strProviderID;
		strProviderID.Format("%li", nProviderID);
		if(nProviderID < 0)
		{			
			pProvider->personID = _bstr_t("");
		}
		else
		{
			pProvider->personID = _bstr_t(strProviderID);
		}
		m_pCurrentPrescription->Provider = pProvider;
		pPrescription->Prescription->Provider = pProvider;
		
		pPrescription->Prescription->PrescriptionID = m_pCurrentPrescription->PrescriptionID;
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pPrescription->Prescription->EMRPrescriptionSource->IsEMRTemplate = m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate;

		Save(pPrescription);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-17 08:59) - PLID 54632 - Refactored from event handler to Save Supervisor
void CPrescriptionEditDlg::SaveSupervisor(long nSupervisorID)
{
	try{
		NexTech_Accessor::_PrescriptionCommitPtr pPrescription(__uuidof(NexTech_Accessor::PrescriptionCommit));
		NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pSupervisor(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
		NexTech_Accessor::_QueuePrescriptionPtr pQueueRx(__uuidof(NexTech_Accessor::QueuePrescription));
		NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
		pPrescription->Prescription = pQueueRx;
		pPrescription->Prescription->EMRPrescriptionSource = pEMRPrescriptionSource;
		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings

		pPrescription->CommitSupervisorID = true;

		if(nSupervisorID < 0)
		{
			//<No Selection> was selected so set it to null
			pSupervisor->personID = _bstr_t("");
		}
		else
		{
			//Selection was valid save it
			CString strSupervisor;
			strSupervisor.Format("%li", nSupervisorID);
			pSupervisor->personID = _bstr_t(strSupervisor);
		}
		m_pCurrentPrescription->Supervisor = pSupervisor;
		pPrescription->Prescription->Supervisor = pSupervisor;

		pPrescription->Prescription->PrescriptionID = m_pCurrentPrescription->PrescriptionID;
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pPrescription->Prescription->EMRPrescriptionSource->IsEMRTemplate = m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate;

		Save(pPrescription);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-17 08:59) - PLID 54632 - Refactored from event handler to Save NurseStaff
void CPrescriptionEditDlg::SaveNurseStaff(long nNurseStaffID)
{
	try{
		NexTech_Accessor::_PrescriptionCommitPtr pPrescription(__uuidof(NexTech_Accessor::PrescriptionCommit));
		NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pNurseStaff(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
		NexTech_Accessor::_QueuePrescriptionPtr pQueueRx(__uuidof(NexTech_Accessor::QueuePrescription));
		NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
		pPrescription->Prescription = pQueueRx;
		pPrescription->Prescription->EMRPrescriptionSource = pEMRPrescriptionSource;

		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		pPrescription->CommitNurseStaffID = true;

		CString strNurseStaffID;
		strNurseStaffID.Format("%li", nNurseStaffID);

		if(nNurseStaffID < 0)
		{
			//<No Selection> was selected so set it to null
			pNurseStaff->personID = _bstr_t("");
		}
		else
		{
			//Selection was valid save it
			pNurseStaff->personID = _bstr_t(strNurseStaffID);
		}
		m_pCurrentPrescription->NurseStaff = pNurseStaff;
		pPrescription->Prescription->NurseStaff = pNurseStaff;

		pPrescription->Prescription->PrescriptionID = m_pCurrentPrescription->PrescriptionID;
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pPrescription->Prescription->EMRPrescriptionSource->IsEMRTemplate = m_pCurrentPrescription->EMRPrescriptionSource->IsEMRTemplate;

		Save(pPrescription);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-17 08:59) - PLID 54632 - Load the legacy combo defaults 
void CPrescriptionEditDlg::LoadPrescriptionPersonsLegacy()
{
	try{
		// (b.savon 2013-01-16 10:37) - PLID 54632 - Changed to have default values and set selections.
		// Only do this now if the current user role is none.
		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		CString strProviderID((LPCTSTR)m_pCurrentPrescription->Provider->personID);
		long nProviderID = strProviderID.IsEmpty()? -1 : atol(strProviderID);
		long nSel = m_ProviderCombo->SetSelByColumn(provccID, nProviderID);
		if(nSel == sriNoRow) {
			//maybe it's inactive?
			// (b.savon 2013-04-09 10:44) - PLID 56085 - grab the address and phone
			_RecordsetPtr rsProv = CreateParamRecordset(
			"SELECT Last + ', ' + First + ' ' + Middle AS Name, "
			"		Address1 + (CASE WHEN (Address2 <> '') THEN ' ' + Address2 ELSE '' END) + CASE WHEN (City <> '') THEN ', ' + City ELSE '' END + CASE WHEN (State <> '') THEN ', ' + State ELSE '' END + CASE WHEN (Zip <> '') THEN ' ' + Zip ELSE '' END AS Address, "
			"		CASE WHEN (WorkPhone <> '') THEN WorkPhone ELSE HomePhone END AS Phone "
			"FROM PersonT "
			"WHERE ID = {INT}", VarLong(nProviderID));
			if(!rsProv->eof) {
				m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
			else {
				m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
			}
			rsProv->Close();
		}

		CString strNurseStaffID((LPCTSTR)m_pCurrentPrescription->NurseStaff->personID);
		long nNurseStaffID = strNurseStaffID.IsEmpty()? -1 : atol(strNurseStaffID);
		// (a.walling 2009-07-01 11:10) - PLID 34052
		nSel = m_AgentCombo->SetSelByColumn(provccID, nNurseStaffID);
		if(nSel == sriNoRow) {
			//maybe it's inactive?
			_RecordsetPtr rsName = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = {INT}", nNurseStaffID);
			if(!rsName->eof) {
				m_AgentCombo->PutComboBoxText(_bstr_t(AdoFldString(rsName, "Name", "")));
			}
			else {
				m_AgentCombo->SetSelByColumn(provccID, (long)-1);
			}
			rsName->Close();
		}

		CString strSupervisorID((LPCTSTR)m_pCurrentPrescription->Supervisor->personID);
		long nSupervisorID = strSupervisorID.IsEmpty()? -1 : atol(strSupervisorID);
		// (a.walling 2009-07-01 11:10) - PLID 34052
		nSel = m_SupervisorCombo->SetSelByColumn(provccID, nSupervisorID);
		if(nSel == sriNoRow) {
			//maybe it's inactive?
			_RecordsetPtr rsName = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = {INT}", nSupervisorID);
			if(!rsName->eof) {
				m_SupervisorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsName, "Name", "")));
			}
			else {
				m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
			}
			rsName->Close();
		}
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::PrepareInterfaceForSupervisorSelection(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bKnownMidlevel /*= FALSE*/)
{
	if( m_urtCurrentUserRole != urtNurseStaff ){
		return;
	}	

	if( pRow == NULL ){
		return;
	}

	//This is easy, just populate this midlevels supervisors
	// (b.savon 2013-04-09 10:44) - PLID 56085 - grab the address and phone
	if( bKnownMidlevel ){
		long nMidlevelID = VarLong(pRow->GetValue(provccID), -1);
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT	SupervisingPersonID, \r\n"
		"		NexERxPrescriberT.LastName + ', ' + NexERxPrescriberT.FirstName AS FullName, \r\n"
		"		NexERxPrescriberT.AddressLine1 + (CASE WHEN (NexERxPrescriberT.AddressLine2 <> '') THEN ' ' + NexERxPrescriberT.AddressLine2 ELSE '' END) + ', ' + NexERxPrescriberT.City + ', ' + NexERxPrescriberT.State + ' ' + NexERxPrescriberT.Zip AS Address, \r\n"
		"		NexERxPrescriberT.PhonePrimary AS Phone \r\n"
		"FROM	NexERxSupervisingProviderT \r\n"
		"		INNER JOIN PersonT ON NexERxSupervisingProviderT.SupervisingPersonID = PersonT.ID \r\n"
		"		INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID \r\n"	
		"		INNER JOIN LocationsT ON LocationsT.ID = {INT} \r\n"
		// (j.fouts 2013-06-10 09:56) - PLID 57047 - SPI root is NOT unique per location, we need to use both location ID and provider ID
		"       INNER JOIN NexERxPrescriberRegistrationT \r\n"
		"			ON NexERxPrescriberRegistrationT.ProviderID = ProvidersT.PersonID \r\n"
		"			AND NexERxPrescriberRegistrationT.LocationID = LocationsT.ID \r\n"
	    "		INNER JOIN NexERxPrescriberT ON NexERxPrescriberT.SPI = NexERxPrescriberRegistrationT.SPI \r\n"
		"WHERE	NexERxSupervisingProviderT.UserID = {INT} AND NexERxSupervisingProviderT.PersonID = {INT} \r\n",
		GetCurrentLocationID(), GetCurrentUserID(), nMidlevelID);

		long nRecordCount = 0;
		if( !prs->eof ){
			nRecordCount = prs->GetRecordCount();
		}else{
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pSupRow;
		m_SupervisorCombo->Clear();
		// Put the blank row
		pSupRow = m_SupervisorCombo->GetNewRow();	
		pSupRow->PutValue(provccID, (long)-1);
		pSupRow->PutValue(provccName, (LPCTSTR)"< No Supervisor Selected >");
		m_SupervisorCombo->AddRowSorted(pSupRow, NULL);
		
		long nSupervisingID = -1;
		while(!prs->eof){
			CString strSupervisingName = AdoFldString(prs->Fields, "FullName");
			nSupervisingID = AdoFldLong(prs->Fields, "SupervisingPersonID");
			CString strSupervisingID;
			strSupervisingID.Format("%li", nSupervisingID);

			pSupRow = m_SupervisorCombo->GetNewRow();
			pSupRow->PutValue(provccID, AsBstr(strSupervisingID));
			pSupRow->PutValue(provccName, AsBstr(strSupervisingName));
			// (b.savon 2013-04-09 10:53) - PLID 56085 - Populate the Address and PHone
			pSupRow->PutValue(provccAddress, AsBstr(AdoFldString(prs->Fields, "Address", "")));
			CString strFormatPhoneString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
			_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(AdoFldString(prs->Fields, "Phone", ""), strFormatPhoneString));
			pSupRow->PutValue(provccPhone, bstrPhoneNumber);
			m_SupervisorCombo->AddRowSorted(pSupRow, NULL);
			
			prs->MoveNext();
		}

		GetDlgItem(IDC_STATIC_SUPERVISOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_SUPERVISOR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->ShowWindow(SW_SHOW);

		// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
		if( nRecordCount == 1 || m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
			if( m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
				m_SupervisorCombo->SetSelByColumn(provccID, m_pCurrentPrescription->Supervisor->personID);	
			}else{
				CString strSupervisingID;
				strSupervisingID.Format("%li", nSupervisingID);
				m_SupervisorCombo->SetSelByColumn(provccID, AsBstr(strSupervisingID));
				SaveSupervisor(nSupervisingID);
			}
		}else{
			m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
			m_SupervisorCombo->PutDropDownState(VARIANT_TRUE);
		}

		return;
	}

	// (b.savon 2013-04-09 10:44) - PLID 56085 - grab the address and phone
	//We don't know if the selected Prescriber is a midlevel
	long nSelectedID = AsLong(pRow->GetValue(provccID));
	ADODB::_RecordsetPtr prs = CreateParamRecordset(
	"SELECT NexERxProviderTypeID FROM ProvidersT WHERE PersonID = {INT} \r\n" 

	"SELECT	SupervisingPersonID, \r\n"
	"		NexERxPrescriberT.LastName + ', ' + NexERxPrescriberT.FirstName AS FullName, \r\n"
	"		NexERxPrescriberT.AddressLine1 + (CASE WHEN (NexERxPrescriberT.AddressLine2 <> '') THEN ' ' + NexERxPrescriberT.AddressLine2 ELSE '' END) + ', ' + NexERxPrescriberT.City + ', ' + NexERxPrescriberT.State + ' ' + NexERxPrescriberT.Zip AS Address, \r\n"
	"		NexERxPrescriberT.PhonePrimary AS Phone \r\n"
	"FROM	NexERxSupervisingProviderT \r\n"
	"		INNER JOIN PersonT ON NexERxSupervisingProviderT.SupervisingPersonID = PersonT.ID \r\n"
	"		INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID \r\n"	
	"		INNER JOIN LocationsT ON LocationsT.ID = {INT} \r\n"
	// (j.fouts 2013-06-10 09:56) - PLID 57047 - SPI root is NOT unique per location, we need to use both location ID and provider ID
	"       INNER JOIN NexERxPrescriberRegistrationT \r\n"
	"			ON NexERxPrescriberRegistrationT.ProviderID = ProvidersT.PersonID \r\n"
	"			AND NexERxPrescriberRegistrationT.LocationID = LocationsT.ID \r\n"
    "		INNER JOIN NexERxPrescriberT ON NexERxPrescriberT.SPI = NexERxPrescriberRegistrationT.SPI \r\n"
	"WHERE  NexERxSupervisingProviderT.PersonID = {INT} \r\n",
	nSelectedID, GetCurrentLocationID(), nSelectedID);

	if( prs->eof ){
		return;
	}
	
	BOOL bMidlevel = FALSE;
	if( !prs->eof ){
		bMidlevel = AdoFldLong(prs->Fields, "NexERxProviderTypeID", prtNone) == prtMidlevelPrescriber ? TRUE : FALSE;
	}

	if( !bMidlevel ){
		GetDlgItem(IDC_STATIC_SUPERVISOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_SUPERVISOR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->ShowWindow(SW_HIDE);
		SaveSupervisor(-1); //Save as NULL
		return;
	}

	prs = prs->NextRecordset(NULL);

	long nRecordCount = 0;
	if( !prs->eof ){
		nRecordCount = prs->GetRecordCount();
	}else{
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pSupRow;
	m_SupervisorCombo->Clear();
	// Put the blank row
	pSupRow = m_SupervisorCombo->GetNewRow();	
	pSupRow->PutValue(provccID, (long)-1);
	pSupRow->PutValue(provccName, (LPCTSTR)"< No Supervisor Selected >");
	m_SupervisorCombo->AddRowSorted(pSupRow, NULL);
	
	long nSupervisingID = -1;
	while(!prs->eof){
		CString strSupervisingName = AdoFldString(prs->Fields, "FullName");
		nSupervisingID = AdoFldLong(prs->Fields, "SupervisingPersonID");
		CString strSupervisingID;
		strSupervisingID.Format("%li", nSupervisingID);

		pSupRow = m_SupervisorCombo->GetNewRow();
		pSupRow->PutValue(provccID, AsBstr(strSupervisingID));
		pSupRow->PutValue(provccName, AsBstr(strSupervisingName));
		// (b.savon 2013-04-09 10:53) - PLID 56085 - Populate the Address and PHone
		pSupRow->PutValue(provccAddress, AsBstr(AdoFldString(prs->Fields, "Address", "")));
		CString strFormatPhoneString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
		_bstr_t bstrPhoneNumber = _bstr_t(FormatPhone(AdoFldString(prs->Fields, "Phone", ""), strFormatPhoneString));
		pSupRow->PutValue(provccPhone, bstrPhoneNumber);
		m_SupervisorCombo->AddRowSorted(pSupRow, NULL);
		
		prs->MoveNext();
	}

	GetDlgItem(IDC_STATIC_SUPERVISOR)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_SUPERVISOR)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_MED_SUPERVISOR_COMBO)->ShowWindow(SW_SHOW);

	// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
	if( nRecordCount == 1 || m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
		if( m_pCurrentPrescription->Supervisor->personID != _bstr_t() ){
			m_SupervisorCombo->SetSelByColumn(provccID, m_pCurrentPrescription->Supervisor->personID);	
		}else{
			CString strSupervisingID;
			strSupervisingID.Format("%li", nSupervisingID);
			m_SupervisorCombo->SetSelByColumn(provccID, AsBstr(strSupervisingID));
			SaveSupervisor(nSupervisingID);
		}
	}else{
		m_SupervisorCombo->SetSelByColumn(provccID, (long)-1);
		m_SupervisorCombo->PutDropDownState(VARIANT_TRUE);
	}
}

void CPrescriptionEditDlg::SelChosenMedProviderCombo(LPDISPATCH lpRow)
{
	try{
		// (b.savon 2013-01-16 11:50) - PLID 54632 - Refactored
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow){
			long nProviderID = AsLong(pRow->GetValue(provccID));
			PrepareInterfaceForSupervisorSelection(pRow);
			SavePrescriber(nProviderID);
		}
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::SelChosenMedSupervisorCombo(LPDISPATCH lpRow)
{
	try
	{
		// (b.savon 2013-01-16 11:50) - PLID 54632 - Refactored
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow){
			long nSupervisorID = AsLong(pRow->GetValue(provccID));
			SaveSupervisor(nSupervisorID);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::SelChosenMedPrescriberAgentCombo(LPDISPATCH lpRow)
{
	try
	{
		// (b.savon 2013-01-16 11:50) - PLID 54632 - Refactored
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow){
			long nAgentID = AsLong(pRow->GetValue(provccID));
			SaveNurseStaff(nAgentID);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-21 14:12) - PLID 54463 - Only drop down the list if it is not yet filled
//This will drop down the next empty list after the passed in control
void CPrescriptionEditDlg::DropDownNextEmptyList(long nCtrlID)
{
	switch(nCtrlID)
	{
	default:
		//Default we will start at the top
	case IDC_PRESCRIPTION_DOSAGE:
		if(!m_pDosageQuantityList->CurSel)
		{
			m_pDosageQuantityList->PutDropDownState(true);
			break;
		}
	case IDC_PRESCRIPTION_DOSAGE_UNIT:
		// (j.jones 2016-01-06 11:39) - PLID 67823 - if the <Custom Sig> row is selected, don't dropdown
		if(!m_pDosageUnitList->CurSel)
		{
			m_pDosageUnitList->PutDropDownState(true);
			break;
		}
	case IDC_PRESCRIPTION_METHOD:
		// (j.jones 2016-01-06 11:39) - PLID 67823 - if the <Custom Sig> row is selected, don't dropdown
		if(!m_pRouteList->CurSel)
		{
			m_pRouteList->PutDropDownState(true);
			break;
		}
	case IDC_PRESCRIPTION_FREQUENCY:
		if(!m_pFrequencyList->CurSel)
		{
			m_pFrequencyList->PutDropDownState(true);
			break;
		}
	case IDC_PRESCRIPTION_REFILLS:
		if(!m_pRefillsList->CurSel || AsLong(m_pRefillsList->CurSel->GetValue(0)) <= 0)
		{
			m_pRefillsList->PutDropDownState(true);
			break;
		}
	case IDC_PACKAGING_INFO:
		// (r.farnworth 2016-01-18 14:23) - PLID 67750 - Added
		// (b.savon 2016-01-19 15:29) - PLID 59879 - Add GetAutoCalculate
		if (!m_pPackagingCombo->CurSel && !m_pAutoPopulatingSig->GetAutoCalculate())
		{
			m_pPackagingCombo->PutDropDownState(true);
			break;
		}
	}
}
// (a.wilson 2013-01-23 11:42) - PLID 53799 - save the refill value to the renewal pointer.
void CPrescriptionEditDlg::SelChosenPrescriptionRefills(LPDISPATCH lpRow)
{
	try {
		long nRefillNumber = 0;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow)
		{
			nRefillNumber = VarLong(pRow->GetValue(0), 0);	
		}
		else
		{
			m_pRefillsList->FindByColumn(0, 0, NULL, VARIANT_TRUE);
		}
		
		if (m_pCurrentPrescription) {

			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitRefillsAllowed = VARIANT_TRUE;

			//Read the value from the control
			// (j.fouts 2013-01-10 10:53) - PLID 54463 - Refills is now a datalist
			m_pCurrentPrescription->RefillsAllowed = _bstr_t(nRefillNumber);

			Save(pCommit);

		}
		EnforceRenewalRefillApproval();	// (a.wilson 2013-04-08 17:49) - PLID 56141

	} NxCatchAll(__FUNCTION__);
}

void CPrescriptionEditDlg::OnBnClickedBtnTodoPresedit()
{
	try
	{
		COleDateTime dtReminderDate = COleDateTime::GetCurrentTime();

		dtReminderDate = VarDateTime(m_dtReminderDate.GetValue());

		if(dtReminderDate.m_status == COleDateTime::invalid || dtReminderDate.m_dt == NULL) {
			AfxMessageBox("If you wish to create a To-Do alarm, you must enter a valid date for the To-Do reminder.");
			return;
		}

		CreateTodoAlarm(dtReminderDate);
	}
	NxCatchAll(__FUNCTION__);
}

//(r.farnworth 2013-01-16) Enable Fax support for Controlled Substances
void CPrescriptionEditDlg::OnBnClickedFaxButton()
{
	try {
		if(!GetWPManager()->CheckWordProcessorInstalled() || !m_pCurrentPrescription) {
			return;
		}

		CString strUser, strPass, strFrom, strResolution, strRecipName, strRecipNum, strTemplateName, strTemplatePath, strPath;
		long nIdCount;
		long nPersonID = -1;

		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Get the selected pharmacy ID in the new pharmacy search list way.
		long PharmSelected = GetSelectedPharmacyID();
		if(PharmSelected == -1)
		{
			if (IsPharmacyListADropDown()) {
				// (r.gonet 2016-02-23 00:53) - PLID 68408 - Renewals use a combo box.
				MsgBox(MB_OK | MB_ICONINFORMATION,
					"You have not selected a pharmacy to recieve this fax.\n"
					"Please go to the Pharmacy dropdown list to choose your destination.");
			} else {
				// (r.gonet 2016-02-23 00:14) - PLID 67988 - Changed wording because prescriptions now uses a search list.
				MsgBox(MB_OK | MB_ICONINFORMATION,
					"You have not selected a pharmacy to receive this fax.\n"
					"Please go to the Pharmacy search to choose your destination.");
			}
			return;
		}		

		long OldConfig = GetRemotePropertyInt("ERxFaxConfig", -1, 0, GetCurrentUserName());
		long FaxConfig;

		_RecordsetPtr prn = CreateParamRecordset("SELECT Count(ID) AS IDNO FROM FaxConfigT WHERE FaxConfigT.UserID = {INT}", GetCurrentUserID());
		if(!prn->eof) {
			FieldsPtr nFlds = prn->Fields;
			nIdCount = AdoFldLong(nFlds, "IDNO");
		}

		if(nIdCount == 0)
		{
			MsgBox(MB_OK|MB_ICONINFORMATION, 
					"You have not set up any fax configurations\n"
					"Please go to Tools -> Faxing -> Configure Outgoing to create one.");
			return;
		}

		//(r.farnworth 2013-01-25) PLID 54667 - Only open the dialog if they have more than one configuration. If not, use their default
		if(nIdCount > 1) {
			CSingleSelectDlg ssd(this);
			ssd.PreSelect(OldConfig);
			if(IDCANCEL == ssd.Open("FaxConfigT", FormatString("FaxConfigT.UserID = %li", GetCurrentUserID()), "Id","ConfigName", "Select a Fax Configuration", true))
				return;
			FaxConfig = ssd.m_nSelectedID;

			if (FaxConfig != OldConfig) 
			{
				if(IDYES == MsgBox(MB_YESNO, "Would you like to set the selected configuration as your default?"))
				{
					SetRemotePropertyInt("ERxFaxConfig",FaxConfig, 0, GetCurrentUserName());
				}
			}
		} else {
			FaxConfig = OldConfig;
		}

		//(r.farnworth 2013-01-16) Get user fax configs and destination information from database.
		CFaxSettings settings;
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Also query the location name and fax since we don't
		// have a datalist row anymore. All we have is a selected pharmacy ID.
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"(
SELECT 
	UserID, 
	Username, 
	Password, 
	FromName, 
	Resolution
FROM FaxConfigT 
WHERE FaxConfigT.ID = {INT};

SELECT 
	LocationsT.Name,
	LocationsT.Fax
FROM LocationsT 
WHERE LocationsT.ID = {INT}
)", FaxConfig, PharmSelected);
		if (!prs->eof) {
			FieldsPtr pFlds = prs->Fields;
			nPersonID = AdoFldLong(pFlds, "UserID");
			settings.m_strUser = AdoFldString(pFlds, "Username");
			settings.m_strPassword = DecryptStringFromVariant(pFlds->Item["Password"]->Value);
			settings.m_strFromName = AdoFldString(pFlds, "FromName");
			settings.m_strResolution = AdoFldString(pFlds, "Resolution");
		}
		// (r.gonet 2016-02-23 00:14) - PLID 67988
		prs = prs->NextRecordset(NULL);
		if(!prs->eof) {
			FieldsPtr pFlds = prs->Fields;
			settings.m_strRecipName = AdoFldString(pFlds, "Name", "");
			settings.m_strRecipNumber = AdoFldString(pFlds, "Fax", "");
		}

		//(r.farnworth 2013-01-24) PLID 54667 Moved to globalutils
		BOOL bMerge;
		bool bCheck = CheckPrescriptionTemplates(strTemplateName, strTemplatePath, *this);
		CArray<long, long> aryPrescriptionIDs;
		CString docPath = GetPatientDocumentPath(m_nPatientID);

		if(!bCheck){
			return;
		} else {
			// (j.fouts 2013-04-23 10:53) - PLID 52907 - Warn when faxing
			switch(m_pCurrentPrescription->status)
			{
				case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
				case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
				case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
				case NexTech_Accessor::PrescriptionStatus_eFaxed:
					//This should never happen, the controls should be disable is this is the case
					//If you hit this then try to determine why the eFax button was enabled, even
					//though its status is one that cannot be faxed.
					ASSERT(FALSE);
					MessageBox(FormatString("You cannot eFax a prescription with a status of %s.", QueueStatusTextFromID(m_pCurrentPrescription->status)), "Error", MB_ICONERROR|MB_OK);
					return;
					break;
				case NexTech_Accessor::PrescriptionStatus_Printed:
					if(MessageBox("This prescription has already been printed. If you have distributed a copy of this prescription "
						"to the patient, please do not send a fax as well. This can result in the patient having multiple prescriptions of the "
						"medication.\n\rWould you like to continue faxing?", "Continue Faxing?", MB_YESNO|MB_ICONQUESTION) != IDOK)
					{
						return;
					}
					break;
				default:
					if(DontShowMeAgain(this, "If you continue eFaxing this prescription you will no longer be able "
						"to edit, delete, eFax, or electronically submit this prescription.\n\r"
						"Are you sure you wish to continue eFaxing this prescription?"
						, "NexERx_FaxLockWarning", "Warning", FALSE, TRUE) == IDNO)
					{
						return;
					}
					break;
			}

			// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
			if(!PromptInteractions())
			{
				return;
			}

			//now merge
			// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
			aryPrescriptionIDs.Add(AsLong(m_pCurrentPrescription->PrescriptionID));
			// (j.fouts 2013-03-05 08:34) - PLID 55427 - Removed PatientID paramater
			bMerge = MergePrescriptionsToWord(aryPrescriptionIDs, strTemplateName, strTemplatePath, strPath, FALSE, TRUE);
		}
		// (a.wilson 2013-04-19 14:40) - PLID 56142 - make this message a bit more friendly since this does not always result from an error.
		if(!bMerge){
			MessageBox("The merge to Microsoft Word could not be completed. The fax will now be cancelled.", 
				"Fax Cancelled", MB_OK | MB_ICONINFORMATION);
			return;
		}

		aryPrescriptionIDs.RemoveAll();
		strPath = docPath ^ strPath;

		//Ensure this path exists
		CFile f;
		if(!f.Open(strPath, CFile::modeRead|CFile::shareDenyNone)) {
			//Failed to load this file
			MsgBox(MB_OK|MB_ICONINFORMATION, 
			"The file '" + strPath + "' cannot be accessed.\n"
			"Aborting fax.");	
			//Abort entirely
			return;
		} else {
			f.Close();
			//Perfectly valid file, add it to our list
			settings.m_aryDocPaths.Add(strPath);
		}

		m_bReadOnly = TRUE;
		m_bChangesMade = true;

		//(r.farnworth 2013-01-16) Send the fax
		CMyFaxSend send;
		if(!send.SendFax(settings, nPersonID, -1)) {
			AfxMessageBox("The fax failed to send.");
			//f.Remove(strPath);
			return;
		} else {
			//(r.farnworth 2013-01-23) PLID 54667 Update the status of the prescription in the datalist to E-Faxed
			try{
				CArray<_bstr_t,_bstr_t> aryPrescriptionID;

				// (j.fouts 2013-02-08 09:06) - PLID 51712 - Changed API IDs to be string
				aryPrescriptionID.Add(m_pCurrentPrescription->PrescriptionID);

				Nx::SafeArray<BSTR> saryPrescriptionIDs = Nx::SafeArray<BSTR>::From(aryPrescriptionID);
				NexTech_Accessor::_UpdatePrescriptionStatusOutputPtr pOutput = GetAPI()->UpdatePrescriptionStatus(GetAPISubkey(), GetAPILoginToken(),
				saryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_eFaxed);
			} NxCatchAll("Error updating prescription status. The fax was sent but the status will not update to 'E-Faxed.'"
							"Please contact NexTech for support.");
			//(r.farnworth 2013-01-29) PLID 54667 - Will now close the window
			OnCancel();
		}
	} NxCatchAll("Error in OnBnClickedFaxButton");
}

//(r.farnworth 2013-01-29) PLID 54667 - Added to configure the buttons correctly in OnInit and selected a new pharmacy
//(r.farnworth 2013-01-30) PLID 54667 - Fixed the e-Submit button re-enabling on dialog changes
BOOL CPrescriptionEditDlg::ConfigureSubmitBtns()
{
	// (r.gonet 2016-02-23 00:14) - PLID 67988 - Get the pharmacy in the new search list way.
	long nPharmacyID = GetSelectedPharmacyID();
	if (nPharmacyID > 0){
		CString strSpecType1, strSpecType2, strSpecType3, strSpecType4, strFax;

		//(r.farnworth 2013-01-22) PLID 54667 -  Upon choosing a pharmacy, we must check if that location supports faxing and e-mail and disable or enable the appropriate buttons
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - We can't depend on a datalist row anymore. We need to query for this information.
		ADODB::_RecordsetPtr prsPharmacy = CreateParamRecordset(GetRemoteDataSnapshot(), R"(
SELECT 
	LocationsT.Fax, 
	NexERxPharmacyT.SpecialtyType1, 
	NexERxPharmacyT.SpecialtyType2, 
	NexERxPharmacyT.SpecialtyType3, 
	NexERxPharmacyT.SpecialtyType4
FROM LocationsT 
LEFT JOIN PharmacyIdT ON LocationsT.ID = PharmacyIdT.LocationID 
LEFT JOIN NexERxPharmacyT ON NexERxPharmacyT.NCPDPID = PharmacyIDT.NCPDPID 
WHERE LocationsT.ID = {INT}
)", nPharmacyID);

		if (prsPharmacy->eof) {
			ThrowNxException("%s : Pharmacy with ID = %li does not exist.", __FUNCTION__, nPharmacyID);
		}

		strFax = AdoFldString(prsPharmacy->Fields, "Fax", "");
		strSpecType1 = AdoFldString(prsPharmacy->Fields, "SpecialtyType1", "");
		strSpecType2 = AdoFldString(prsPharmacy->Fields, "SpecialtyType2", "");
		strSpecType3 = AdoFldString(prsPharmacy->Fields, "SpecialtyType3", "");
		strSpecType4 = AdoFldString(prsPharmacy->Fields, "SpecialtyType4", "");
		
		//(r.farnworth 2013-02-21) PLID 54667 - Now checks for the MyFax license
		if(!g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
			return FALSE;
		} else {
			// The pharmacy doesn't support fax
			if(strSpecType1 == "MailOrder" || strSpecType2 == "MailOrder" || strSpecType3 == "MailOrder" || strSpecType4 == "MailOrder"){
				return FALSE;
			} else {
				// The pharmacy only supports mail
				if(strFax.IsEmpty()){
					return FALSE;
				// The pharmacy supports E-Mail and Fax
				} else {
					if(m_bCachedIsLicensedConfiguredUser)
						return TRUE;
					else
						return FALSE;
				}
			}
		}
	} else {
		return FALSE;
	}
}


// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added a dosage unit list
void CPrescriptionEditDlg::SelChosenPrescriptionDosageUnit(LPDISPATCH lpRow)
{
	try
	{
		if (!m_pCurrentPrescription)
		{
			return;
		}

		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
		// is a template prescription ID, not a patient prescription ID
		pCommit->Prescription = m_pCurrentPrescription;
		pCommit->CommitDosageUnitID = VARIANT_TRUE;

		CString strDosageUnitID = "";
		CString strDosageUnit = "";
		CString strDosageUnitPl = "";

		if(m_pDosageUnitList->CurSel)
		{
			long nDosageUnitID = VarLong(m_pDosageUnitList->CurSel->GetValue(dulcID), -1);
			if(nDosageUnitID > 0)
			{
				// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
				strDosageUnitID.Format("%li", nDosageUnitID);
				strDosageUnit = AsString(m_pDosageUnitList->CurSel->GetValue(dulcUnit));
				strDosageUnitPl = AsString(m_pDosageUnitList->CurSel->GetValue(dulcUnitPlural));
			}
		}

		m_pCurrentPrescription->Dosage->DosageUnit->ID = _bstr_t(strDosageUnitID);
		m_pCurrentPrescription->Dosage->DosageUnit->Unit = _bstr_t(strDosageUnit);
		m_pCurrentPrescription->Dosage->DosageUnit->UnitPlural = _bstr_t(strDosageUnitPl);
	
		//Update the sig before saving
		m_pAutoPopulatingSig->SetUnit(strDosageUnit, strDosageUnitPl);
		UpdateSig(pCommit);
		Save(pCommit);	

		// (j.fouts 2013-01-10 10:53) - PLID 54463 - Open the next combobox when the select something to create a easier flow
		DropDownNextEmptyList(IDC_PRESCRIPTION_METHOD);
	}
	NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-02-05 11:23) - PLID 51705
// (a.wilson 2013-05-01 09:36) - PLID 56509 - need to check for prescriber IDs no matter what for renewals.
BOOL CPrescriptionEditDlg::IsLicensedConfiguredUser()
{
	if (m_pCurrentRenewalRequest)
		return m_lNexERxLicense.IsCurrentUserLicensedForNexERx();
	else
		return m_urtCurrentUserRole != urtNone && m_lNexERxLicense.IsCurrentUserLicensedForNexERx();;
}

// (s.dhole 2013-02-11 16:47) - PLID 54063
BOOL CPrescriptionEditDlg::IsSubmitButtonEnabled()
{
	// (s.dhole 2012-12-06 08:53) - PLID 54067 check License
	if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts &&
			m_pCurrentPrescription->ValidationList->ErrorStatus !=NexTech_Accessor::eRxErrorStatus_eRxNoError ){
			return FALSE;
		}	
	return m_bCachedIsLicensedConfiguredUser;
}

// (s.dhole 2013-02-11 16:47) - PLID 54063
int CPrescriptionEditDlg::IsErrorEnabled()
{
	// (s.dhole 2012-12-06 08:53) - PLID 54067 check License
	if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts &&
			m_pCurrentPrescription->ValidationList->ErrorStatus !=NexTech_Accessor::eRxErrorStatus_eRxNoError ){
			return SW_SHOWNA;
		}	
	else
	{
		return SW_HIDE;
	}
}

// (a.wilson 2013-02-20 15:57) - PLID 53912 - if the renewal was already responded to during a response selection update the dialog.
void CPrescriptionEditDlg::UpdateRenewalRequestInfo()
{
	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();

	if (pApi) {
		NexTech_Accessor::_ERxRenewalRequestInfoPtr pRequest = pApi->GetRenewalRequestInfo(GetAPISubkey(), GetAPILoginToken(), 
			_bstr_t(m_pCurrentRenewalRequest->GetID()));
		//ensure its valid.
		if (pRequest != NULL && pRequest->GetID() == m_pCurrentRenewalRequest->GetID())
		{
			m_pCurrentRenewalRequest = pRequest;
			m_bInitialRenewalDisplayComplete = false;
			OpenRenewalRequest();
		}
	}
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Get the selected pharmacy ID.
long CPrescriptionEditDlg::GetSelectedPharmacyID()
{
	if (!IsPharmacyListADropDown()) {
		// (r.gonet 2016-02-23 00:14) - PLID 67988 - Prescriptions uses the search list, which stores its pharmacy ID in the current prescription object.
		if (m_pCurrentPrescription->Pharmacy->PharmacyID.length() != 0) {
			return atol(m_pCurrentPrescription->Pharmacy->PharmacyID);
		} else {
			return -1;
		}
	} else {
		// (r.gonet 2016-02-23 00:53) - PLID 68408 - Renewals use a combo box format.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_PharmacyCombo->CurSel;
		if (pRow != nullptr) {
			return VarLong(pRow->GetValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID), -1);
		} else {
			return -1;
		}
	}
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Disables/Hides controls that should be hidden for EMR Template Prescriptions
void CPrescriptionEditDlg::HideEMRTemplateControls()
{
	//hide the prescription date and the "first prescribed date"			
	GetDlgItem(IDC_DATE_LABEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PRESCRIPTION_DATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FIRST_DATE_LABEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FIRSTDATE)->ShowWindow(SW_HIDE);

	//disable the provider, location, and pharmacy controls
	m_ProviderCombo->ReadOnly = VARIANT_TRUE;
	m_LocationCombo->ReadOnly = VARIANT_TRUE;
	if (IsPharmacyListASearchList()) {
		// (r.gonet 2016-02-24 17:08) - PLID 67988 - Set the search list and search edit box to disabled.
		// They'll still be able to drop down the search results list but the blinking cursor will not
		// be there.
		m_PharmacyCombo->Enabled = VARIANT_FALSE;
		m_PharmacyCombo->ReadOnly = VARIANT_TRUE;
		if (GetSelectedPharmacyID() == -1) {
			GetDlgItem(IDC_MED_PHARMACY_COMBO)->ShowWindow(SW_HIDE);
			m_nxlPharmacySelection.ShowWindow(SW_SHOW);
			m_nxlPharmacySelection.SetText("< No Pharmacy Selected >");
			m_nxlPharmacySelection.SetType(dtsDisabledHyperlink);
		} else {
			UpdateControlsForPharmacySelection(GetSelectedPharmacyID());
		}
	} else {
		// (r.gonet 2016-02-24 17:08) - PLID 68408 - Set it to read only so it functions the same as the other
		// datalists here.
		m_PharmacyCombo->ReadOnly = VARIANT_TRUE;
	}

	//TES 2/17/2009 - PLID 33140 - Disable the prior authorization controls
	m_nxlabelPriorAuthLabel.SetType(dtsDisabledHyperlink);
	m_editPriorAuthorization.SetReadOnly(TRUE);

	//TES 5/8/2009 - PLID 28519 - Added an expiration date for sample prescriptions
	GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->EnableWindow(FALSE);

	//hide the todo reminder controls
	HideToDoReminderControls();

	m_editPrescriptionExplanation.SetReadOnly(VARIANT_TRUE);		
	m_SupervisorCombo->ReadOnly = VARIANT_TRUE;
	m_AgentCombo->ReadOnly = VARIANT_TRUE;
	m_pQuantityUnitCombo->ReadOnly = VARIANT_TRUE;
	m_pPackagingCombo->ReadOnly = VARIANT_TRUE; // (r.farnworth 2016-01-18 09:05) - PLID 67750

	// (b.savon 2013-09-24 09:59) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
	m_nxsAddDiagnosisCodes.EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_DIAGNOSIS_CODE_GROUP)->EnableWindow(FALSE);
	
	GetDlgItem(IDC_SAMPLE_EXPIRATION_DATE)->EnableWindow(FALSE);
	m_btnCreateTodo.EnableWindow(FALSE);
	m_dtReminderDate.EnableWindow(FALSE);
	// (j.fouts 2013-01-04 11:25) - PLID 54447 - Added a control to save the default	
	m_checkSaveAsDefault.EnableWindow(FALSE);
	// (j.fouts 2013-01-04 11:24) - PLID 54448 - Added a control to save to the favorites list
	m_checkAddToFavorites.EnableWindow(FALSE);
	// (j.fouts 2013-01-10 10:54) - PLID 54526 - Added Dosage quantity list
	m_pDosageQuantityList->ReadOnly = VARIANT_TRUE;
	// (j.fouts 2013-02-01 15:09) - PLID 54528 - Added dosage route list
	m_pRouteList->ReadOnly = VARIANT_TRUE;
	// (j.fouts 2013-01-10 10:55) - PLID 54529 - Added dosage frequency list
	m_pFrequencyList->ReadOnly = VARIANT_TRUE;
	// (r.farnworth 2013-01-29) PLID 54667 Added button disables for following an e-Fax
	m_btnEFax.ShowWindow(SW_HIDE);
	m_btnESubmit.ShowWindow(SW_HIDE);
	m_btnPreview.ShowWindow(SW_HIDE);
	// (s.dhole 2013-02-11 16:47) - PLID 54063
	m_btnESubmit.EnableWindow(FALSE);
	// (j.fouts 2013-02-01 15:11) - PLID 53971 - Workarond, checkboxe's text are now labels
	m_nxsSaveAsDefaultText.SetType(dtsDisabledHyperlink);
	m_nxsSaveToQuickListText.SetType(dtsDisabledHyperlink);
	// (b.savon 2013-09-24 09:48) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog
	m_nxsAddDiagnosisCodes.SetType(dtsDisabledHyperlink);
	m_pDosageUnitList->ReadOnly = VARIANT_TRUE;

	// (s.dhole 2013-10-19 13:45) - PLID 59068 reset combo
	m_pInsuranceCombo->Clear(); 
	//TES 10/2/2013 - PLID 58287 - The formulary dialog sends in a request, and can change the medication, so don't allow it here
	m_pInsuranceCombo->ReadOnly = VARIANT_TRUE;
	// (s.dhole 2013-10-19 11:15) - PLID 59084
	m_btnFCovarage.EnableWindow(FALSE);

}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Disables the Print Button
void CPrescriptionEditDlg::DisablePrint()
{
	m_btnPreview.EnableWindow(FALSE);
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Disables the EFax Button
void CPrescriptionEditDlg::DisableEFax()
{
	m_btnEFax.EnableWindow(FALSE);
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Disables the ESubmit Button
void CPrescriptionEditDlg::DisableESubmit()
{
	m_btnESubmit.EnableWindow(FALSE);
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Hides the Dosage Label
void CPrescriptionEditDlg::HideDosage()
{
	m_nxlabelDosage.ShowWindow(SW_HIDE);
	m_nxlabelDosage.SetType(dtsDisabledHyperlink);	
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Hides controls that are only available with NexERx
void CPrescriptionEditDlg::HideNexERxControls()
{
	m_btnESubmit.ShowWindow(SW_HIDE);
	m_btnESubmit.EnableWindow(FALSE);
	m_nxBtnRxError.ShowWindow(SW_HIDE);
	m_nxBtnRxError.EnableWindow(FALSE);
	// (s.dhole 2013-11-27 14:55) - PLID 59189 
	m_btnFCovarage.EnableWindow(FALSE);
	HideDosage();
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Hides controls that should only be shown for renewals
void CPrescriptionEditDlg::HideRenewalControls()
{
	m_nxUserAuthorizationWarning.EnableWindow(FALSE);
	m_nxUserAuthorizationWarning.ShowWindow(SW_HIDE);
	m_btnApprove.EnableWindow(FALSE);
	m_btnApprove.ShowWindow(SW_HIDE);
	m_btnApproveWithChanges.EnableWindow(FALSE);
	m_btnApproveWithChanges.ShowWindow(SW_HIDE);
	m_btnDeny.EnableWindow(FALSE);
	m_btnDeny.ShowWindow(SW_HIDE);
	m_btnDenyAndRewrite.EnableWindow(FALSE);
	m_btnDenyAndRewrite.ShowWindow(SW_HIDE);
	m_nxMedicationLabel.SetText(PRESCRIBED_MEDICATION_TEXT);
	m_btnViewOriginalPrescription.EnableWindow(FALSE);
	m_btnViewOriginalPrescription.ShowWindow(SW_HIDE);
	m_nxAssignPatientLink.ShowWindow(SW_SHOW);
	m_nxAssignPatientLink.EnableWindow(FALSE);
	m_nxDispensedMedicationLabel.EnableWindow(FALSE);
	m_nxDispensedMedicationLabel.ShowWindow(SW_HIDE);
	m_nxDispensedMedicationNameLabel.EnableWindow(FALSE);
	m_nxDispensedMedicationNameLabel.ShowWindow(SW_HIDE);
	m_editResponseReason.EnableWindow(FALSE);
	m_editResponseReason.ShowWindow(SW_HIDE);
	m_nxRenewalWarning.ShowWindow(SW_HIDE);
}

// (j.fouts 2013-03-14 12:18) - PLID 52907 - Enables any controls that are pending and then enforces disabled controls to hide them when neciessary
//If your are adding a new control that can be enabled due to a change in the prescription's state
//please add the code to enable the control here so that we can still call enforce disable controls
//after so we never enable something that should be disabled.
void CPrescriptionEditDlg::TryEnablePendingControls()
{
	//ESubmit could have been enabled becuase a change made validation pass
	m_btnESubmit.ShowWindow(SW_SHOWNA);
	m_btnESubmit.EnableWindow(TRUE);
	//Errors could have just been made so the Error button could be shown
	m_nxBtnRxError.ShowWindow(SW_SHOWNA);
	m_nxBtnRxError.EnableWindow(TRUE);
	//A valid pharmacy could have been selected that enabled the EFax button
	m_btnEFax.ShowWindow(SW_SHOWNA);
	m_btnEFax.EnableWindow(TRUE);

	//We may have enabled something that shouldn't be so enforce disabled controls
	EnforceDisabledControls();
}

// (j.fouts 2013-04-09 08:56) - PLID 52907 - Sets a text ctrl's tooltip to the text if it exceeds the bounds of the ctrl
void CPrescriptionEditDlg::SetTextAndToolTip(CNxStatic *pTextControl, CString &bstrMessage)
{
	CString strTextOut;
	CString strTextIn((LPCTSTR)bstrMessage);

	pTextControl->SetWindowText(strTextIn);

	CDC *pDC = GetDC();
	CDC dc;
	if(dc.CreateCompatibleDC(pDC))
	{
		CRect rectSize;
		CFont* pBold = &theApp.m_boldFont;
		CFont* pOldFont = dc.SelectObject(pBold);
		dc.DrawText(strTextIn, &rectSize, DT_NOCLIP | DT_CALCRECT);
		dc.SelectObject(pOldFont); //Reset the old font
		
		CRect rectCtrlSize;
		pTextControl->GetWindowRect(&rectCtrlSize);

		if(rectSize.Width() > rectCtrlSize.Width())
		{
			//We do not fit in the control
			pTextControl->SetToolTip(strTextIn);
			pTextControl->EnableToolTips(TRUE);
		}
		else
		{
			//We fit in the control
			pTextControl->EnableToolTips(FALSE);
		}
	}
	else
	{
		//Somthing went wrong, lets assume we don't fit
		pTextControl->SetToolTip(strTextIn);
		pTextControl->EnableToolTips(TRUE);
	}
}


// (a.wilson 2013-04-08 17:51) - PLID 56141 - this function checks the changes made to the refills controls and enables\disables the approval button accordingly.
void CPrescriptionEditDlg::EnforceRenewalRefillApproval()
{
	if (!m_pCurrentRenewalRequest)
		return;	//don't worry about this if it isn't a request.
	// (a.wilson 2013-05-01 11:54) - PLID 56509 - also prevent if the user is not authorized.
	if (m_pCurrentRenewalRequest->GetResponseStatus() != NexTech_Accessor::RenewalResponseStatus_Pending
		|| !VarString(m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetDEASchedule(), "").IsEmpty()
		|| !m_bUserAuthorizedPrescriber)
		return;	//ignore if not pending, or is a controlled substance.
	//pull the values from the renewal request.
	CString strRenewalQualifier = VarString(m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetRefillQualifier(), "P");
	long nRenewalRefills = atol(VarString(m_pCurrentRenewalRequest->GetPrescribedMedicationInfo()->GetRefills(), "0"));
	//check if the renewal has a refill value of 0 or null.
	if (nRenewalRefills == 0 && strRenewalQualifier != "PRN") 
		return;
	//adjust to match UI amount.
	nRenewalRefills = (nRenewalRefills > 0 ? (nRenewalRefills - 1) : 0);
	//pull the neccesary values to compare.
	CString strCurrentQualifier = (m_nxlabelRefills.GetText() == REFILL_AS_NEEDED_TEXT ? "PRN" : "P");
	long nCurrentRefills = VarLong(m_pRefillsList->GetCurSel()->GetValue(0), 0);

	if (strCurrentQualifier == "PRN" && strRenewalQualifier == "PRN") {
		m_btnApprove.EnableWindow(TRUE);
	} else if (nCurrentRefills == nRenewalRefills && strCurrentQualifier == strRenewalQualifier) {
		m_btnApprove.EnableWindow(TRUE);
	} else {
		m_btnApprove.EnableWindow(FALSE);
	}
}

// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
bool CPrescriptionEditDlg::PromptInteractions()
{
	// (j.jones 2013-11-25 13:47) - PLID 59772 - properly handled the modeless dialog
	if (!m_pDrugInteractionDlg) {
		m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, epiPrescriptionEdit));
	}

	if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
	{
		m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
	}

	m_pDrugInteractionDlg->ShowOnInteraction(m_nPatientID, true, false, true);
	// (j.fouts 2013-09-17 16:53) - PLID 58496 - Use filtered count and fixed wording
	if(m_pDrugInteractionDlg->GetFilteredInteractionCount() > 0)
	{
		if(MessageBox("This patient has drug interactions. Please review them.\n\rAre you sure you want to continue?", "Interactions Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;	//There were no interactions
}

void CPrescriptionEditDlg::RevertSelChangingOnNull(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if(*lppNewSel == NULL)
		{
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__)
}

// (b.savon 2013-09-18 14:36) - PLID 58455 - Add a URL link to launch a webbrowser to the IStop web address so that prescribers can access the controlled substance registry
void CPrescriptionEditDlg::OnNMClickSyslinkCsUrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	try{
	
		PNMLINK pNMLink = (PNMLINK) pNMHDR;

		if( pNMLink ){
			::ShellExecute(NULL, "open", CString(pNMLink->item.szUrl), NULL, NULL, SW_SHOWNORMAL);
			long nAuditID = BeginNewAuditEvent();
			CString strOldValue, strNewValue;
			strOldValue = "";
			strNewValue = "Controlled substance registry checked";
			AuditEvent(
				m_nPatientID, 
				GetExistingPatientName(m_nPatientID), 
				nAuditID, 
				aeiPatientMedicationCheckControlledSubstanceRegistry, 
				atol((LPCTSTR)m_pCurrentPrescription->PrescriptionID),
				strOldValue, 
				strNewValue, 
				aepMedium, 
				aetChanged
			);
		}

	}NxCatchAll(__FUNCTION__);

	*pResult = 0;
}


//TES 8/22/2013 - PLID 57999 - Formularies
enum FormularyResultsListColumns
{
	frlcFDBID = 0,
	frlcWrite = 1,
	frlcMedName = 2,
};

void CPrescriptionEditDlg::SelChosenFormularyInsList(LPDISPATCH lpRow)
{
	try {
		if (m_pCurrentPrescription)
		{
			NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
			// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
			// is a template prescription ID, not a patient prescription ID
			pCommit->Prescription = m_pCurrentPrescription;
			pCommit->CommitInsuranceDetailID = VARIANT_TRUE;

			IRowSettingsPtr pRow(lpRow);
			CString strInsuranceDetailID = "-1";
			if (pRow == nullptr) {
				// (r.gonet 2016-03-22 14:59) - PLID 68494 - Warning, kludge ahead! Find the first could not process 
				// eligibilty detail and save that. I'd rather have the eligibility detail for the prescription be 
				// null if there is are no coverages to select, but we've been using the column this gets saved to as a flag
				// that we've checked for eligibility for this medication. A better solution would be to add a mod
				// to add an EligibilityChecked bit flag to PatientMedications and revise the places that check this column
				// as an indicator that eligibility has been checked to check that instead, but since this is at the very 
				// end of the release, I'll settle for doing this instead.
				pRow = m_pInsuranceCombo->FindByColumn(filcEligibilityCoverageStatus, (long)EligibilityCoverageStatus::CouldNotProcess, nullptr, VARIANT_FALSE);
			}
			
			if(pRow) 
			{
				strInsuranceDetailID = AsString(pRow->GetValue(filcID));
			}


			m_pCurrentPrescription->InsuranceDetailID = _bstr_t(strInsuranceDetailID);

			Save(pCommit);	
		}

		//TES 8/29/2013 - PLID 57999 - Update the results list
		LoadFormularyResults();

	}NxCatchAll(__FUNCTION__);
}

//TES 8/27/2013 - PLID 58287 - Update the description to use the medication identified by nFDBID
// (s.dhole 2013-10-19 11:52) - PLID 59068 added insurance ID
void CPrescriptionEditDlg::ChangeMedication(long nFDBID, const CString &strMedName, long nFInsuranceID )
{
	//TES 8/27/2013 - PLID 58287 - If they're changing to the same as we already have, or if we don't have a prescription loaded, do nothing.
	if(m_nFDBID == nFDBID || m_pCurrentPrescription == NULL) {
		return;
	}

	//TES 8/29/2013 - PLID 58287 - Make sure they really want this
	if(IDYES != MsgBox(MB_YESNO, "This will overwrite the existing prescription fields with the default values for %s. Are you sure you wish to continue?", strMedName)) {
		return;
	}

	long nMedicationID = -1;
	_RecordsetPtr rsMedID = CreateParamRecordset("SELECT ID FROM DrugList WHERE FDBID = {INT}", nFDBID);
	if(!rsMedID->eof) {
		nMedicationID = AdoFldLong(rsMedID, "ID");
	}
	else {
		//TES 9/24/2013 - PLID 58287 - We now only get here from the formulary dialog, which already imports from FDB if needed. So we
		// should never reach the branch.
		ASSERT(FALSE);
	}

	CWaitCursor cuWait;

	//TES 8/28/2013 - PLID 58287 - Prepare our input to the prescription queue
	CArray<NexTech_Accessor::_PrescriptionChangeMedicationPtr,NexTech_Accessor::_PrescriptionChangeMedicationPtr> aryPrescriptions;
	NexTech_Accessor::_PrescriptionChangeMedicationPtr pPrescription(__uuidof(NexTech_Accessor::PrescriptionChangeMedication));
	NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));
	NexTech_Accessor::_NexERxDrugPtr pMedications(__uuidof(NexTech_Accessor::NexERxDrug));

	//TES 8/28/2013 - PLID 58287 - We're telling it to change the medication
	pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_ChangeMedication;
	pExpects->RequeryQueue = FALSE;
	if (nFInsuranceID==-1){
		m_pCurrentPrescription->InsuranceDetailID = "";  
	}
	else{
		m_pCurrentPrescription->InsuranceDetailID = AsBstr(AsString(nFInsuranceID))  ;
	}
	pPrescription->Prescription = m_pCurrentPrescription;

	pPrescription->NewMedicationID = nMedicationID;

	// (s.dhole 2013-10-19 11:52) - PLID 
	 
	aryPrescriptions.Add(pPrescription);
	Nx::SafeArray<IUnknown*> saryPrescriptions = Nx::SafeArray<IUnknown*>::From(aryPrescriptions);
	pExpects->PrescriptionsToChangeMedication = saryPrescriptions;

	// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
	pExpects->ExcludeMonographInformation = VARIANT_TRUE;

	//TES 8/28/2013 - PLID 58287 - Now tell it to change the medication
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", m_nPatientID)),
		pExpects);

	//TES 8/28/2013 - PLID 58287 - Validate our results
	if( pResults->PrescriptionsMedicationChanged == NULL ){
		ThrowNxException("Unable to Update Prescription in CPrescriptionEditDlg::ChangeMedication!");
	}

	Nx::SafeArray<IUnknown*> saryNewPrescriptions(pResults->PrescriptionsMedicationChanged);

	if(saryNewPrescriptions.GetCount() == 0)
	{
		ThrowNxException("Attempted to update a prescription, but no prescription was returned."); 
	}

	//TES 8/28/2013 - PLID 58287 - OK, read out our results, and store them in our member variables.
	PrescriptionInfo rxInformation;
	NexTech_Accessor::_QueuePrescriptionPtr pNewPrescription = saryNewPrescriptions[0];
	rxInformation.pPrescription = pNewPrescription;
	rxInformation.erxUserRole = pResults->UserRole;
	rxInformation.saryPrescribers = pResults->Prescriber;
	rxInformation.sarySupervisors = pResults->Supervisor;
	rxInformation.saryNurseStaff = pResults->NurseStaff;

	m_pCurrentPrescription = rxInformation.pPrescription;
	m_saryPrescribers = rxInformation.saryPrescribers;
	m_sarySupervisors = rxInformation.sarySupervisors;
	m_saryNurseStaff = rxInformation.saryNurseStaff;
	m_urtCurrentUserRole = AccessorERxUserRoleToPracticeEnum(rxInformation.erxUserRole);

	//TES 9/25/2013 - PLID 58287 - Now re-save the current prescription, so as to trigger the validation
	NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
	// (j.jones 2012-11-21 12:21) - PLID 53818 - must set the template flag so the API knows this
	// is a template prescription ID, not a patient prescription ID
	pCommit->Prescription = m_pCurrentPrescription;
	//TES 8/28/2013 - PLID 58287 - Update our member variable
	m_nFDBID = nFDBID;
	// (s.dhole 2013-10-19 13:08) - PLID 
	m_pInsuranceCombo->Clear(); 
	//TES 8/28/2013 - PLID 58287 - Reload all the data on screen
	LoadPrescriptionFields();
	//TES 10/10/2013 - PLID 58287 - The Sig in m_pCurrentPrescription will be the default, and thus not include Frequency and so on. So update it before we save.
	UpdateSig(pCommit);
	Save(pCommit);
}

void CPrescriptionEditDlg::LoadFormularyResults()
{
	// (s.dhole 2013-10-19 11:08) - PLID 59084 reset text font color
	m_btnFCovarage.SetTextColor(COLOR_NON_FORMULARY); 
	IRowSettingsPtr pInsRow = m_pInsuranceCombo->CurSel;
	if(pInsRow == NULL) {
		return;
	}

	//TES 8/22/2013 - PLID 57999 - Construct our formulary request, based on our medication and the insurance plans in our list
	NexTech_Accessor::_FormularyDataRequestPtr pRequest(__uuidof(NexTech_Accessor::FormularyDataRequest));
	//PatientID
	pRequest->PutpatientID(AsBstr(FormatString("%li", m_nPatientID)));
	//TES 9/24/2013 - PLID 57999 - Added Location ID and Provider ID
	pRequest->PutlocationID(m_pCurrentPrescription->locationID);
	pRequest->PutproviderID(m_pCurrentPrescription->Provider->personID);
	// (b.savon 2014-01-27 15:37) - PLID 60488 - Restrict alternatives information returned when writing an Rx
	pRequest->PutExcludeAlternativesResults(VARIANT_TRUE);
	//DrugListID
	CArray<_bstr_t,_bstr_t> aryBstrIDs;
	aryBstrIDs.Add(AsBstr(FormatString("%li", m_nFDBID)));
	Nx::SafeArray<BSTR> saryFDBIDs = Nx::SafeArray<BSTR>::From(aryBstrIDs);
	pRequest->PutFDBID(saryFDBIDs);
	//InsurancePlan
	CArray<NexTech_Accessor::_FormularyInsurancePlanPtr, NexTech_Accessor::_FormularyInsurancePlanPtr> aryInsPlan;
	// (b.savon 2014-01-27 13:45) - PLID 60484 - Only request formulary information for the selected 
	// insurance and drug; not all insurances and drug
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsuranceCombo->GetCurSel();
	//TES 8/22/2013 - PLID 57999 - Add all the insurance plans, we'll filter the response based on our selection.
	// (b.savon 2013-09-25 12:38) - PLID 58762 - Use the correct enum
	if (pRow) {
		NexTech_Accessor::_FormularyInsurancePlanPtr fipInsurance(__uuidof(NexTech_Accessor::FormularyInsurancePlan));
		fipInsurance->PutdetailID(AsBstr(FormatString("%li", VarLong(pRow->GetValue(filcID)))));
		fipInsurance->PutInsurancePlacement(AsBstr(FormatString("%li", VarLong(pRow->GetValue(filcInsuranceResponsibilityID)))));
		fipInsurance->PutFormularyPublisher(AsBstr(VarString(pRow->GetValue(filcPBMID), "")));
		fipInsurance->PutFormularyID(AsBstr(VarString(pRow->GetValue(filcSureScriptsFormularyID), "")));
		fipInsurance->PutAlternativesID(AsBstr(VarString(pRow->GetValue(filcSureScriptsAlternativesID), "")));
		fipInsurance->PutCopayID(AsBstr(VarString(pRow->GetValue(filcSureScriptsCopayID), "")));
		fipInsurance->PutCoverageID(AsBstr(VarString(pRow->GetValue(filcSureScriptsCoverageID), "")));
		fipInsurance->PutLastSureScriptsRequestDate(VarDateTime(pRow->GetValue(filcLastSureScriptsRequestDate)));
		aryInsPlan.Add(fipInsurance);
	}
	Nx::SafeArray<IUnknown*> saryInsPlan = Nx::SafeArray<IUnknown*>::From(aryInsPlan);
	pRequest->PutInsurancePlan(saryInsPlan);

	//TES 8/22/2013 - PLID 57999 - Now get the data from the API
	// (b.savon 2013-10-04 10:07) - PLID 57377 - NexFormulary - Practice Integration - Master Item
	GetAPI()->SetTimeoutInMs(180000); //default to 3 mins
	NexTech_Accessor::_FormularyDataResponsePtr pResponse = GetAPI()->GetFormularyData(GetAPISubkey(), GetAPILoginToken(), pRequest);

	//TES 8/22/2013 - PLID 57999 - Find the response that matches our selected insurance plan
	Nx::SafeArray<IUnknown *> saryFormularyInsurance = pResponse->GetInsurance();		
	NexTech_Accessor::_FormularyInsurancePtr formularyInsuranceSelected;
	long nResponseDetailID = -1;
	pRow = m_pInsuranceCombo->CurSel;
	if(pRow) {
		nResponseDetailID = VarLong(pRow->GetValue(filcID), -1);
	}
	foreach(NexTech_Accessor::_FormularyInsurancePtr formularyInsurance, saryFormularyInsurance){
		if(atol(formularyInsurance->ResponseDetailID) == nResponseDetailID) {
			formularyInsuranceSelected = formularyInsurance;
		}
	}
	if(formularyInsuranceSelected == NULL) {
		return;
	}
	//TES 8/22/2013 - PLID 57999 - Now display the results (including error messages if appropriate)
	if( formularyInsuranceSelected->GetResponseStatus() == NexTech_Accessor::FormularyResponseStatus_EligibilityRequestFailure ){
		SetDlgItemText(IDC_FORMULARY_STATUS_TEXT, "Eligibility Request Failed");
	}else if ( formularyInsuranceSelected->GetResponseStatus() == NexTech_Accessor::FormularyResponseStatus_FormularyDataFailure ){
		SetDlgItemText(IDC_FORMULARY_STATUS_TEXT, "Formulary Lookup Failed");
	} else if ( formularyInsuranceSelected->GetResponseStatus() == NexTech_Accessor::FormularyResponseStatus_Success ){
		//TES 8/28/2013 - PLID 57999 - Load the drug information into our edit controls at the top
		Nx::SafeArray<IUnknown *> saryDrugList = formularyInsuranceSelected->GetDrug();
		foreach(NexTech_Accessor::_FormularyDrugPtr  FormularyDrug, saryDrugList){
			m_nFormularyStatus = VarLong(FormularyDrug->GetFormulary()->value,-2);
			SetDlgItemText(IDC_FORMULARY_STATUS_TEXT, VarString (FormularyDrug->GetFormulary()->status,""));
			SetDlgItemText(IDC_FORMULARY_COVERAGE, VarString (FormularyDrug->Coverage,""));
			SetDlgItemText(IDC_FORMULARY_COPAY, GetCopayInformation(FormularyDrug->Copay));
			if (!VarString (FormularyDrug->Coverage,"").IsEmpty()){
				// (s.dhole 2013-10-19 11:08) - PLID 59084 set text font color
				m_btnFCovarage.SetTextColor(RGB(0, 0, 255)); 
			}
			


		}
	}else{/*NexTech_Accessor::FormularyResponseStatus_UnknownFailure*/
		SetDlgItemText(IDC_FORMULARY_STATUS_TEXT, "Unknown Error");
	}
}

HBRUSH CPrescriptionEditDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{
	//TES 8/28/2013 - PLID 57999 - Color our formulary status, based on whether or not we're on formulary
	HBRUSH hbr;
	if  (pWnd->GetDlgCtrlID() ==IDC_FORMULARY_STATUS_TEXT){
    
		if (m_nFormularyStatus >= 2 && m_nFormularyStatus<= 99){
			pDC->SetTextColor(COLOR_ON_FORMULARY);
		}else{
			pDC->SetTextColor(COLOR_NON_FORMULARY);
		}
    }
	
	hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}
void CPrescriptionEditDlg::ReloadFormularyInformation()
{
	// (s.dhole 2013-10-19 13:45) - PLID 59068 save old value
	m_SelectedFInsuranceID =-1;
	if(m_pInsuranceCombo->CurSel != NULL)
	{
		m_SelectedFInsuranceID=m_pInsuranceCombo->CurSel->GetValue(filcID);
	}
	//TES 9/25/2013 - PLID 57999 - Reloads just the formulary information from data. The from clause is copied from CNexFormularyDlg
	m_pInsuranceCombo->PutFromClause(
		 // (s.dhole 2013-10-08 10:58) - PLID 58923  move to PrescriptionUtilsAPI
			AsBstr(GetFormularyInsuranceSQL())
		);
		m_pInsuranceCombo->WhereClause = _bstr_t(FormatString("PatientID = %li", m_nPatientID));
		m_pInsuranceCombo->PutComboBoxText(_bstr_t(""));
		m_pInsuranceCombo->Requery();
}

void CPrescriptionEditDlg::RequeryFinishedFormularyInsList(short nFlags)
{
	try {
		// (r.gonet 2016-03-03 09:50) - PLID 68494 - We were showing coverages with any Coverage status, including Coverage status = 5 (Could Not Process).
		// We have since learned that a coverage with this status is returned in the eligibility response by SureScripts if the patient has
		// no coverages at all. If this gets selected on the prescription (which is does by default if it is the only thing in the list), and
		// we send it in the BenefitCoordination/PayerID and BenefitCoordination/PayerName element in the NewRX message, then SureScripts does not
		// respond with an error. However, they did tell us verbally that we should not be sending PayerID or PayerName in that case.
		// They did not say anything about other Coverage statuses, so we are continuing to allow those to be shown. We do not allow the user to select
		// a CouldNotProcess row.

		bool bCouldNotProcess = false;
		long nRealCoverages = 0;
		NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_pInsuranceCombo->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pFirstRealCoverage = nullptr;
		while (pRowIter != nullptr) {
			if (VarLong(pRowIter->GetValue(filcEligibilityCoverageStatus), -1) != (long)EligibilityCoverageStatus::CouldNotProcess) {
				if (pFirstRealCoverage == nullptr) {
					pFirstRealCoverage = pRowIter;
				}
				nRealCoverages++;
			} else {
				// (r.gonet 2016-03-22 14:59) - PLID 68494 - Hide the invalid coverages instead of removing them.
				pRowIter->Visible = VARIANT_FALSE;
				bCouldNotProcess = true;
			}
			pRowIter = pRowIter->GetNextRow();
		}

		// (s.dhole 2013-10-19 13:45) - PLID 59068 try to load old value
		if (m_SelectedFInsuranceID < 1 || (m_SelectedFInsuranceID > 0 && !m_pInsuranceCombo->FindByColumn(filcID, m_SelectedFInsuranceID,NULL, VARIANT_TRUE))) {
			// (r.gonet 2016-03-22 14:59) - PLID 68494 - Or the first real coverage if we can't.
			m_pInsuranceCombo->CurSel = pFirstRealCoverage;
		}
		
		SelChosenFormularyInsList(m_pInsuranceCombo->CurSel);

		//TES 9/25/2013 - PLID 57999 - Give a little extra information if there are no eligible plans
		if(nRealCoverages == 0) {
			if (!bCouldNotProcess) {
				m_pInsuranceCombo->ComboBoxText = _bstr_t("N/A; click Coverage... to request");
			} else {
				// (r.gonet 2016-03-03 09:50) - PLID 68494 - Little more detail if the request was made and succeeded but there are no
				// valid coverages.
				m_pInsuranceCombo->ComboBoxText = _bstr_t("No Coverages Found");
			}
		}
		else if(m_pInsuranceCombo->IsComboBoxTextInUse) {
			m_pInsuranceCombo->ComboBoxText = _bstr_t("");
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-10-19 11:16) - PLID 59068
void CPrescriptionEditDlg::OnBnClickedBtnViewCoverage()
{
	try {
	//TES 9/24/2013 - PLID 58287 - Link to CNexFormularyDlg
	// (s.dhole 2013-10-19 11:16) - PLID 59068 move this code from existing link to button
		if(m_pCurrentPrescription) {
			NexFormularyDrug nfdDrug;
			nfdDrug.nFDBID = m_nFDBID;
			nfdDrug.strDrugName = CString((LPCTSTR)m_pCurrentPrescription->Medication->MedicationName);
			// (s.dhole 2013-10-28 10:15) - PLID 59084 Load nFInsuranceID
			long nFIncuranceId= -1;
			if(m_pInsuranceCombo->CurSel != NULL){
				nFIncuranceId=m_pInsuranceCombo->CurSel->GetValue(filcID);
			}
			// (s.dhole 2013-10-28 10:15) - PLID 59084 Pass nFInsuranceID
			CNexFormularyDlg dlg(nfsPrescriptionEditDlg, m_nPatientID, nfdDrug, atol(CString((LPCTSTR)m_pCurrentPrescription->Provider->personID)),nFIncuranceId, this);
			if(dlg.DoModal() == IDOK) {
				//TES 9/24/2013 - PLID 58287 - Change the medication to the one they selected
				ChangeMedication(dlg.GetFDBID(), dlg.GetDrugName(),dlg.GetFInsuranceID());
			}
			else {
				//TES 9/25/2013 - PLID 58287 - The formulary information still may have changed (it may have submitted a new request),
				// so reload
				ReloadFormularyInformation();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-10-18 09:46) - PLID 58983 - added infobutton abilities
void CPrescriptionEditDlg::OnBtnPtEducation()
{
	try {

		if(!m_pCurrentPrescription) {
			//should not be possible
			ASSERT(FALSE);
			ThrowNxException("No valid prescription object exists.");
		}
		long nMedicationID = AsLong(m_pCurrentPrescription->Medication->DrugListID);
		if(nMedicationID == -1) {
			//should not be possible
			ASSERT(FALSE);
			ThrowNxException("No valid medication ID was found.");
		}

		//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
		LookupMedlinePlusInformationViaURL(this, mlpDrugListID, nMedicationID);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-11-25 12:07) - PLID 59772 - added OnShowWindow
void CPrescriptionEditDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {

		if(bShow) {

			// (j.jones 2013-11-25 09:15) - PLID 59772 - added ability to show drug interactions immediately upon opening the dialog
			if(m_drugInteractionsToShowOnInit.saryDrugDrugInteracts.GetCount() > 0
				|| m_drugInteractionsToShowOnInit.saryDrugAllergyInteracts.GetCount() > 0
				|| m_drugInteractionsToShowOnInit.saryDrugDiagnosisInteracts.GetCount() > 0) {

				this->PostMessage(NXM_POST_SHOWWINDOW);
			}
		}

	}NxCatchAll(__FUNCTION__);	
}

// (j.jones 2013-11-25 12:07) - PLID 59772 - added OnPostShowWindow
LRESULT CPrescriptionEditDlg::OnPostShowWindow(WPARAM wParam, LPARAM lParam)
{
	try {

		if(m_drugInteractionsToShowOnInit.saryDrugDrugInteracts.GetCount() > 0
			|| m_drugInteractionsToShowOnInit.saryDrugAllergyInteracts.GetCount() > 0
			|| m_drugInteractionsToShowOnInit.saryDrugDiagnosisInteracts.GetCount() > 0) {

			bool bShowInteractions = false;

			// (j.jones 2013-11-25 14:18) - PLID 59772 - we will show the interactions dialog immediately
			// if this drug is in the interactions list, which will handle cases where they are prescribing
			// this drug and we need to tell them right away that there is an interaction
			if(!m_pCurrentPrescription)
			{
				//shouldn't be possible, but if this happens, show the interactions anyways
				ASSERT(FALSE);
				bShowInteractions = true;
			}

			if(!bShowInteractions && m_nFDBID != -1) {
				long nDrugCount = m_drugInteractionsToShowOnInit.saryDrugDrugInteracts.GetCount();
				for(int i=0; i<nDrugCount && !bShowInteractions; i++)
				{
					NexTech_Accessor::_FDBDrugDrugInteractionPtr ptr = m_drugInteractionsToShowOnInit.saryDrugDrugInteracts.GetAt(i);
					long nCurFDBID1 = AsLong(ptr->MedID);
					long nCurFDBID2 = AsLong(ptr->InteractMedID);
					if(nCurFDBID1 == m_nFDBID || nCurFDBID2 == m_nFDBID) {
						bShowInteractions = true;
					}
				}

				long nAllergyCount = m_drugInteractionsToShowOnInit.saryDrugAllergyInteracts.GetCount();
				for(int i=0; i<nAllergyCount && !bShowInteractions; i++)
				{
					NexTech_Accessor::_FDBDrugAllergyInteractionPtr ptr = m_drugInteractionsToShowOnInit.saryDrugAllergyInteracts.GetAt(i);
					long nCurFDBID = AsLong(ptr->MedID);
					if(nCurFDBID == m_nFDBID) {
						bShowInteractions = true;
					}
				}

				long nDiagCount = m_drugInteractionsToShowOnInit.saryDrugDiagnosisInteracts.GetCount();
				for(int i=0; i<nDiagCount && !bShowInteractions; i++)
				{
					NexTech_Accessor::_FDBDrugDiagnosisInteractionPtr ptr = m_drugInteractionsToShowOnInit.saryDrugDiagnosisInteracts.GetAt(i);
					long nCurFDBID = AsLong(ptr->MedID);
					if(nCurFDBID == m_nFDBID) {
						bShowInteractions = true;
					}
				}
			}

			if(bShowInteractions) {

				if (!m_pDrugInteractionDlg) {
					m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, epiPrescriptionEdit));
				}

				if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
				{
					m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
				}

				m_pDrugInteractionDlg->ShowOnInteraction(m_drugInteractionsToShowOnInit.saryDrugDrugInteracts,
					m_drugInteractionsToShowOnInit.saryDrugAllergyInteracts,
					m_drugInteractionsToShowOnInit.saryDrugDiagnosisInteracts, m_nPatientID);
			}
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}
// (r.farnworth 2016-01-08 15:35) - PLID 58692 - We need to save right away if we're represcribing
void CPrescriptionEditDlg::SaveAll()
{
	CString strText;
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
	pCommit->Prescription = m_pCurrentPrescription;

	pCommit->CommitAllowSubstitutions = VARIANT_TRUE;
	pCommit->CommitNurseStaffID = VARIANT_TRUE;
	pCommit->CommitPharmacyID = VARIANT_TRUE;
	pCommit->CommitPrescriptionDate = VARIANT_TRUE;
	pCommit->CommitProviderID = VARIANT_TRUE;
	pCommit->CommitSupervisorID = VARIANT_TRUE;
	pCommit->CommitDaysSupply = VARIANT_TRUE;
	pCommit->CommitDosageFrequency = VARIANT_TRUE;
	pCommit->CommitDosageQuantity = VARIANT_TRUE;
	pCommit->CommitDosageForm = VARIANT_TRUE;
	pCommit->CommitDosageRouteID = VARIANT_TRUE;
	pCommit->CommitDosageUnitID = VARIANT_TRUE;
	pCommit->CommitInsuranceDetailID = VARIANT_TRUE;
	pCommit->CommitLocationID = VARIANT_TRUE;
	pCommit->CommitPatientExplanation = VARIANT_TRUE;
	pCommit->CommitPharmacistNote = VARIANT_TRUE;
	pCommit->CommitPriorAuthorization = VARIANT_TRUE;
	pCommit->CommitPriorAuthorizationIsSample = VARIANT_TRUE;
	pCommit->CommitQuantity = VARIANT_TRUE;
	pCommit->CommitQuantityUnitID = VARIANT_TRUE;
	pCommit->CommitRefillsAllowed = VARIANT_TRUE;
	pCommit->CommitSampleExpirationDate = VARIANT_TRUE;
	pCommit->CommitDiagnosisCodes = VARIANT_TRUE;
	pCommit->CommitSelectedPackage = VARIANT_TRUE; // (r.farnworth 2016-01-18 11:15) - PLID 67750

	Save(pCommit);
}

// (r.farnworth 2016-01-18 11:15) - PLID 67750
void CPrescriptionEditDlg::OnSelChosenPackagingInfo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		m_editQuantity.SetWindowText(VarString(pRow->GetValue(pSize)));

		NexTech_Accessor::_NexERxMedicationPackageInformationPtr pPackage(__uuidof(NexTech_Accessor::NexERxMedicationPackageInformation));

		pPackage->NDC = _bstr_t(VarString(pRow->GetValue(pNDC), ""));
		pPackage->Size = _bstr_t(VarString(pRow->GetValue(pSize), ""));
		pPackage->DrugForm = _bstr_t(VarString(pRow->GetValue(pForm), ""));
		pPackage->description = _bstr_t(VarString(pRow->GetValue(pDescription), ""));

		SavePackagingInfo(pPackage);

		DropDownNextEmptyList(IDC_PRESCRIPTION_REFILLS);
	}NxCatchAll("Error in CPrescriptionEditDlg::OnSelChosenPackagingInfo");
}

// (r.farnworth 2016-01-18 11:15) - PLID 67750
void CPrescriptionEditDlg::SavePackagingInfo(NexTech_Accessor::_NexERxMedicationPackageInformationPtr pPackage)
{
	if (m_pCurrentPrescription)
	{
		NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
		pCommit->Prescription = m_pCurrentPrescription;
		CString strText;
		
		if (pPackage->NDC != m_pCurrentPrescription->SelectedPackage->NDC)
		{
			//Check to see if we updated the Quantity
			GetDlgItemText(IDC_EDIT_QUANTITY, strText);
			if (strText.Compare(VarString(m_pCurrentPrescription->GetQuantityValue(), "")) != 0)
			{
				m_pCurrentPrescription->QuantityValue = _bstr_t(strText);
				pCommit->CommitQuantity = VARIANT_TRUE;
				m_pAutoPopulatingSig->SetQuantity(strText); // (b.savon 2016-01-19 15:29) - PLID 59879
			}

			m_pCurrentPrescription->SelectedPackage = pPackage;
			pCommit->CommitSelectedPackage = VARIANT_TRUE;
			Save(pCommit);
		}
	}
}

// (r.farnworth 2016-01-18 11:15) - PLID 67750
void CPrescriptionEditDlg::LoadPackagingInfo()
{
	if (m_pCurrentPrescription)
	{
		NexTech_Accessor::_NexERxMedicationPackageInformationPtr pPackage(__uuidof(NexTech_Accessor::NexERxMedicationPackageInformation));
		Nx::SafeArray<IUnknown*> saryAvailablePackages = m_pCurrentPrescription->AvailablePackages;

		for (int i = 0; i < (int)saryAvailablePackages.GetCount(); i++)
		{
			pPackage = saryAvailablePackages[i];

			// (r.farnworth 2016-03-07 12:38) - PLID 67750 - Check if the main entry (NDC) is null. If it is, then skip this row.
			if (pPackage == NULL || pPackage->NDC.length() == 0)
			{
				continue;
			}

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPackagingCombo->GetNewRow();
			pRow->PutValue(pNDC, pPackage->NDC);
			// (r.farnworth 2016-03-07 12:38) - PLID 67750 - If NDC was not NULL, but the other packaging rows are (seemingly impossible), just default them to blank
			pRow->PutValue(pSize, (pPackage->Size.length() == 0) ? "" : pPackage->Size);
			pRow->PutValue(pForm, (pPackage->DrugForm.length() == 0) ? "" : pPackage->DrugForm);
			pRow->PutValue(pDescription, (pPackage->description.length() == 0) ? "" : pPackage->description);
			pRow->PutValue(pSizeFormDescription, (LPCTSTR)(pPackage->Size + " " + pPackage->DrugForm + " " + pPackage->description));
			m_pPackagingCombo->AddRowSorted(pRow, NULL);
		}
	}
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Flag when the pharmacy search list has started to populate.
void CPrescriptionEditDlg::RequestListDataStartingMedPharmacyCombo()
{
	try {
		if (IsPharmacyListADropDown()) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - We only care about prescriptions.
			return;
		}

		m_bIsPharmacyDataListSearching = true;
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - If the pharmacy search results list has closed up,
// and the user has not cancelled out of the selection, then revert the selection to the old pharmacy.
void CPrescriptionEditDlg::ClosedUpExMedPharmacyCombo(LPDISPATCH lpSel, short dlClosedUpReason)
{
	try {
		if (IsPharmacyListADropDown()) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - We only care about prescriptions.
			return;
		}

		if (!m_bIsPharmacyDataListSearching) {
			NXDATALIST2Lib::EClosedUpReason eReason = static_cast<NXDATALIST2Lib::EClosedUpReason>(dlClosedUpReason);
			if (eReason != NXDATALIST2Lib::EClosedUpReason::dlClosedUpReasonSelChangedOnPurpose) {
				long nOldPharmacyID = m_pCurrentPrescription->Pharmacy->PharmacyID.length() != 0 ? atol(m_pCurrentPrescription->Pharmacy->PharmacyID) : -1;
				UpdateControlsForPharmacySelection(nOldPharmacyID);
			}
		} else {
			// The pharmacy search results datalist will close up when they type into it, because the results
			// list is requeried. We can't exactly show the link again during that time.
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Gets the appropriate default selection for the pharmacy search list.
// All search lists need a default selection, which is usually the first row. In the pharmacy search list though,
// the first row is the Add From Pharmacy Directory, which isn't a good default selection since selecting it opens
// a dialog.
NXDATALIST2Lib::IRowSettingsPtr CPrescriptionEditDlg::GetDefaultPharmacySearchListSelection() const
{
	// The first non-special row, if there is one, should be the default selection.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_PharmacyCombo->GetFirstRow();
	while (pRow != nullptr) {
		long nID = VarLong(pRow->GetValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID));
		if (nID >= 0) {
			return pRow;
		}
		pRow = pRow->GetNextRow();
	}

	// If there are no results, then the default selection should be the Add From Pharmacy Directory row, which sort of makes sense.
	pRow = m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID,
		EPharmacySearchSpecialRow::AddFromPharmacyDirectory, m_PharmacyCombo->GetFirstRow(), VARIANT_FALSE);
	if (pRow != nullptr) {
		return pRow;
	}

	// If they have no Add From Pharmacy Directory, like they aren't licensed for FDB, then select the no results row.
	pRow = m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID,
		EPharmacySearchSpecialRow::NoResultsFound, m_PharmacyCombo->GetFirstRow(), VARIANT_FALSE);
	if (pRow != nullptr) {
		return pRow;
	}

	// The No Results Found row and the Instructions row are mutually exclusive. If the no results row did not exist, then the instructions row should.
	pRow = m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID,
		EPharmacySearchSpecialRow::InstructionsFooter, m_PharmacyCombo->GetFirstRow(), VARIANT_FALSE);
	if (pRow != nullptr) {
		return pRow;
	}

	// Still need a default selection...
	pRow = m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID,
		EPharmacySearchSpecialRow::SpacerFooter, m_PharmacyCombo->GetFirstRow(), VARIANT_FALSE);
	if (pRow != nullptr) {
		return pRow;
	}

	// Last ditch effort. I find this condition to be impossible, but if we reach here, it should mostly work if null is selected. 
	// The primary reason I've found for requiring a default selection on these search lists is in case tab is pressed.
	return m_PharmacyCombo->GetFirstRow();
}

// (r.gonet 2016-02-23 00:14) - PLID 67988 - Unflag when the pharmacy search list has finished populating.
void CPrescriptionEditDlg::RequestListDataFinishedMedPharmacyCombo()
{
	try {
		if (IsPharmacyListADropDown()) {
			// (r.gonet 2016-02-23 00:53) - PLID 68408 - We only care about prescriptions.
			return;
		}

		m_bIsPharmacyDataListSearching = false;

		// (r.gonet 2016-02-24 17:10) - PLID 67988 - Only add the Add From Pharamcy Directory row if
		// they have e-prescribing and this prescription isn't read only.
		NXDATALIST2Lib::IRowSettingsPtr pAddFromPharmacyDirectoryRow = nullptr;
		if (m_bHasEPrescribingLicense && !m_bReadOnly) {
			pAddFromPharmacyDirectoryRow = m_PharmacyCombo->GetNewRow();
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID, (long)EPharmacySearchSpecialRow::AddFromPharmacyDirectory);
			// (r.gonet 2016-02-23 12:47) - PLID 67964 - Init the leading icon.
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::LeadingIcon, (long)0);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Name, _bstr_t("< Add from Pharmacy Directory >"));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Address, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::City, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::State, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Zip, _bstr_t(""));
			// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::CrossStreet, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::OrderIndex, (long)-1);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Phone, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::NCPDPID, _bstr_t(""));
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Fax, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::EPrescribingReady, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec1, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec2, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec3, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec4, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::SpecAll, g_cvarNull);
			// (r.gonet 2016-02-23 11:02) - PLID 67962 - Added color columns.
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowBackColor, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowBackColorSel, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowForeColor, g_cvarNull);
			pAddFromPharmacyDirectoryRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowForeColorSel, g_cvarNull);
			m_PharmacyCombo->AddRowBefore(pAddFromPharmacyDirectoryRow, m_PharmacyCombo->GetFirstRow());
		}

		bool bReplaceNoResultsRowWithFooter = false;
		// Footer
		NXDATALIST2Lib::IRowSettingsPtr pFooterRow = m_PharmacyCombo->GetNewRow();
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::LeadingIcon, (long)0);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Address, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::City, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::State, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Zip, _bstr_t(""));
		// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::CrossStreet, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::OrderIndex, (long)-1);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Phone, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::NCPDPID, _bstr_t(""));
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Fax, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::EPrescribingReady, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec1, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec2, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec3, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Spec4, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::SpecAll, g_cvarNull);
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Added color columns.
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowBackColor, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowBackColorSel, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowForeColor, g_cvarNull);
		pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::RowForeColorSel, g_cvarNull);
		if (m_PharmacyCombo->GetSearchStringText().length() == 0 && !m_bReadOnly) {
			pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID, (long)EPharmacySearchSpecialRow::InstructionsFooter);
			pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Name, _bstr_t("Type to Find Other Pharmacies"));
			bReplaceNoResultsRowWithFooter = true;
		} else {
			pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::ID, (long)EPharmacySearchSpecialRow::SpacerFooter);
			pFooterRow->PutValue((short)PharmacySearchUtils::EPharmacySearchColumns::Name, _bstr_t(""));
		}
		pFooterRow->PutCellForeColor((short)PharmacySearchUtils::EPharmacySearchColumns::Name, RGB(128, 128, 128));

		if (bReplaceNoResultsRowWithFooter) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_PharmacyCombo->FindByColumn((short)PharmacySearchUtils::EPharmacySearchColumns::ID,
				(long)EPharmacySearchSpecialRow::NoResultsFound, m_PharmacyCombo->GetFirstRow(), VARIANT_FALSE);
			if (pRow != nullptr) {
				m_PharmacyCombo->RemoveRow(pRow);
			}
		}

		m_PharmacyCombo->AddRowAtEnd(pFooterRow, NULL);
		if (!bReplaceNoResultsRowWithFooter) {
			m_PharmacyCombo->RemoveRow(pFooterRow);
		} else {
			// Leave the footer row.
		}

		// Make a default selection since all search lists need one.
		m_PharmacyCombo->CurSel = GetDefaultPharmacySearchListSelection();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-24 17:18) - PLID 68408 - Get the type of pharmacy list to display.
NXDATALIST2Lib::EViewType CPrescriptionEditDlg::GetPharmacyListViewType()
{
	return (m_eDisplayedRecordType == EDisplayedRecordType::Renewal) ? NXDATALIST2Lib::EViewType::vtDropdownList : vtSearchList;
}


// (r.gonet 2016-02-24 17:18) - PLID 68408 - Gets whether the pharmacy list is a search list
bool CPrescriptionEditDlg::IsPharmacyListASearchList()
{
	return GetPharmacyListViewType() == NXDATALIST2Lib::EViewType::vtSearchList;
}

// (r.gonet 2016-02-24 17:18) - PLID 68408 - Gets whether the pharmacy list is a dropdown.
bool CPrescriptionEditDlg::IsPharmacyListADropDown()
{
	return GetPharmacyListViewType() == NXDATALIST2Lib::EViewType::vtDropdownList;
}