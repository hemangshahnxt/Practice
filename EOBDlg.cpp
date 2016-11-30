// EOBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EOBDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "EditComboBox.h"
#include "Ebilling.h"
#include "AdjustmentCodeSettingsDlg.h"
#include "OHIPUtils.h"
#include "ERemitImportFilteringDlg.h"
#include "AlbertaHLINKUtils.h"
#include "NotesDlg.h"
#include "NxModalParentDlg.h"
#include "ERemitZeroPaysDlg.h"
#include "FileUtils.h"
#include "FinancialCorrection.h"
#include <set>	// (j.dinatale 2012-11-07 11:18) - PLID 50792
#include "PayCatdlg.h" // (j.gruber 2012-11-15 13:58) - PLID 53752

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2012-05-25 13:21) - PLID 44367 - All calls to AfxMessageBox were changed to MessageBox
// to make the modeless dialog behave better.

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.jones 2011-06-16 08:51) - PLID 39747 - the default insurance combo filter for when we aren't showing
// specific insurance companies in the list
#define DEFAULT_INSURANCE_FILTER	"Archived = 0"

// (j.jones 2011-06-16 09:27) - PLID 39747 - added defines for showing all / filtered companies
#define SHOW_ALL_INSCOS			-2
#define SHOW_FILTERED_INSCOS	-3

// (j.jones 2008-12-16 10:15) - PLID 32317 - switched to an enum, but I didn't change the naming
// because it would change too much in this file
enum EOBListColumns {
	COLUMN_EOB_INDEX = 0,		//the index of which EOB the charge is on
	COLUMN_PATIENT_ID,			//The PersonID of the patient
	COLUMN_PATIENT_USERDEFINED_ID,//The userdefinedID of the patient // (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	COLUMN_PATIENT_NAME,		//The display name of the patient
	COLUMN_INSURED_PARTY_ID,	//The patient's insured party ID corresponding to the current insurance co.
// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
	COLUMN_RESP_TYPE_ID,		//The patient's insured party's RespTypeID
// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
	COLUMN_SUBMIT_AS_PRIMARY,
	COLUMN_INS_CO_ID,			//The patient's insured party's insurance company ID
	COLUMN_INS_CO_NAME,			//The patient's insured party's insurance company name
	COLUMN_HCFA_GROUP_ID,		//The patient's insured party's insurance company's HCFA Group ID
	COLUMN_HCFA_GROUP_NAME,		//The patient's insured party's insurance company's HCFA Group name
	COLUMN_BILL_ID,				//The bill ID of the charge being applied to
	COLUMN_CHARGE_ID,			//The charge ID being applied to
// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
	COLUMN_BIRTHDATE,			//The birthdate of the patient
	COLUMN_OHIP_HEALTH_NUM,		//The OHIP Health Card Number for the patient
	COLUMN_OHIP_VERSION_CODE,	//The OHIP Version Code for the patient
	COLUMN_DUPLICATE,			//Tracks if the charge/bill was identified as a duplicate (already processed)
	COLUMN_REVERSAL,			// (j.jones 2011-02-09 11:24) - PLID 42391 - tracks if this claim is a reversal
	COLUMN_BILL_DATE,			//The service date of the bill
	COLUMN_CHARGE_DATE,			//The service date of the charge
	COLUMN_CPT_CODE,			//The CPT Code of the charge
	COLUMN_NOTE_ICON,			// (j.jones 2011-01-04 14:36) - PLID 16423 - added billing notes
	COLUMN_CHARGE_AMOUNT,		//The total charge amount
	COLUMN_PAYMENT,				//The insurance payment made on this charge	
	COLUMN_FEE_SCHED_ALLOWABLE,	// (j.jones 2011-03-18 09:32) - PLID 42157 - added fee sched allowable (hidden)
	COLUMN_EOB_ALLOWABLE,		// (j.jones 2011-03-18 09:32) - PLID 42157 - added EOB allowable (shown)
	// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
	// (j.jones 2016-04-19 13:01) - NX-100161 - the old column, original adjustment, is obsolete now, this is now a hidden column
	// of the total of all positive adjustments on the charge
	COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, 
	// (j.jones 2016-04-19 10:09) - NX-100161 - renamed to "total adjustments", this is also no longer editable
	COLUMN_TOTAL_ADJUSTMENTS,	//The insurance adjustment made on this charge
	COLUMN_ADJ_GROUP_CODE,		//The insurance adjustment group code, according to the EOB
	COLUMN_ADJ_REASON_CODE,		//The insurance adjustment reason code, according to the EOB
	COLUMN_ADJ_REASON,			//The insurance adjustment reason, according to the EOB
	// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from COLUMN_PAT_RESP to COLUMN_OTHER_RESP, because
	// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
	COLUMN_OTHER_RESP,			//The EOB's intended patient responsibility on this charge (NOT the current responsibility in Practice)
	COLUMN_PAT_APPLIES,			//The total of current patient applies made on this charge
	COLUMN_PAT_BALANCE,			//The anticipated patient balance after this EOB is applied
	COLUMN_INS_BALANCE,			//The anticipated insurance balance after this EOB is applied
	COLUMN_SKIPPED,				//Tracks if the charge will be skipped when posting
	// (j.jones 2009-03-05 11:40) - PLID 33235 - renamed these columns to reflect that it is a generic pointer,
	// as we now track OHIP pointers here as well
	COLUMN_CLAIM_PTR,			// (j.jones 2008-11-17 15:12) - PLID 32045 - tracks the EOBClaimInfo pointer from the ANSI835Parser
	COLUMN_CHARGE_PTR,			// (j.jones 2008-11-17 15:12) - PLID 32045 - tracks the EOBLineItemInfo pointer from the ANSI835Parser
	
};

// (j.jones 2008-12-19 09:28) - PLID 32519 - added to auto-load remit files
#define IDT_INIT_LOAD	WM_USER + 10000

// (r.farnworth 2014-01-28 15:48) - PLID 52596 - Need a command to handle reverting the adjustment
// (j.jones 2016-04-19 13:14) - NX-100161 - obsolete
//#define ID_REVERT_ADJUSTMENT	55200

// (j.jones 2008-07-11 16:33) - PLID 28756 - added options for the default pay/adj. descriptions & categories
enum InsuranceCompanyColumns {

	iccID = 0,
	iccName,
	iccAddress,
	iccDefaultPayDesc,
	iccDefaultPayCategoryID,
	iccDefaultAdjDesc,
	iccDefaultAdjCategoryID,
};

enum PayCatColumns {

	pccID = 0,
	pccCategory,
};

enum AdjCatColumns {

	accID = 0,
	accCategory,
};

using namespace ADODB;

// (j.jones 2007-01-16 16:25) - PLID 23913 - used for file manipulation later on
enum Attribute 
{
   normal = 0x00, //a normal file
   readOnly = 0x01, //read-only, can't write into the file
   hidden = 0x02, //likes to hide..
   sys = 0x04, //system file
   volume = 0x08, //seems to be a harddisk after MSDN help
   directory = 0x10, //guess..
   archive = 0x20 //don't know what Windows means by that.. 
}; 


/////////////////////////////////////////////////////////////////////////////
// CEOBDlg dialog
// Passing in an empty string for the strSizeandPositionConfigRT argument now because of a issue
// with the dialog not maximizing correctly because strSizeandPositionConfigRT will automatically 
// maximize the dialog if the dialog does not have the maximize ability disabled
// this is causing the dialog to be maximized and set to a smaller size later.
CEOBDlg::CEOBDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEOBDlg::IDD, pParent, "")
{
	//{{AFX_DATA_INIT(CEOBDlg)
		m_bIsLoading = FALSE;
		m_bWarningsCreated = FALSE;
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		m_ERemitType = ertAnsi;
		m_bEnablePeekAndPump = TRUE;
		m_hNotes = NULL;
		m_bOHIPIgnoreMissingPatients = FALSE;
		m_bUnbatchMA18orN89 = FALSE;
	//}}AFX_DATA_INIT
}

CEOBDlg::~CEOBDlg()
{
}

// (j.jones 2012-10-12 10:23) - PLID 53149 - added OnDestroy for cleanup
void CEOBDlg::OnDestroy()
{
	try {

		if(m_hNotes) {
			DestroyIcon((HICON)m_hNotes);
		}

		CNxDialog::OnDestroy();

		//tell mainframe that this closed,  it will handle clearing the tracked pointer
		GetMainFrame()->PostMessage(NXM_EOB_CLOSED);

	}NxCatchAll(__FUNCTION__);
}

void CEOBDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEOBDlg)
	DDX_Control(pDX, IDC_BTN_ADJUSTMENT_CODE_SETTINGS, m_btnAdjustmentCodeSettings);
	DDX_Control(pDX, IDC_BTN_IMPORT_OHIP_FILE, m_btnImportOHIP);
	DDX_Control(pDX, IDC_CHECK_BATCH_CLAIMS, m_btnAutoShift);
	DDX_Control(pDX, IDC_CHECK_SHIFT_POSTED_AMOUNTS, m_checkOnlyShiftPostedCharges);
	DDX_Control(pDX, IDC_EDIT_ADJ_CAT, m_btnEditAdjCat);
	DDX_Control(pDX, IDC_EDIT_PAY_CAT, m_btnEditPayCat);
	DDX_Control(pDX, IDC_EDIT_ADJ_DESC, m_btnEditAdjDesc);
	DDX_Control(pDX, IDC_EDIT_PAY_DESC, m_btnEditPayDesc);
	DDX_Control(pDX, IDC_PRINT_EOB, m_btnPrint);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_BTN_IMPORT_835_FILE, m_btnImport835);
	DDX_Control(pDX, IDC_PROGRESS_BAR_STATUS, m_progressStatus);
	DDX_Control(pDX, IDC_PAY_DESC, m_nxeditPayDesc);
	DDX_Control(pDX, IDC_ADJ_DESC, m_nxeditAdjDesc);
	DDX_Control(pDX, IDC_INSCO_NAME_LABEL, m_nxstaticInscoNameLabel);
	DDX_Control(pDX, IDC_PROVIDER_LABEL, m_nxstaticProviderLabel);
	DDX_Control(pDX, IDC_PROVIDER_NAME_LABEL, m_nxstaticProviderNameLabel);
	DDX_Control(pDX, IDC_PAYMENT_LABEL, m_nxstaticPaymentLabel);
	DDX_Control(pDX, IDC_TOTAL_PAYMENTS_LABEL, m_nxstaticTotalPaymentsLabel);
	DDX_Control(pDX, IDC_PAY_DESC_LABEL, m_nxstaticPayDescLabel);
	DDX_Control(pDX, IDC_PAY_CAT_LABEL, m_nxstaticPayCatLabel);
	DDX_Control(pDX, IDC_ADJ_DESCRIPTION_LABEL, m_nxstaticAdjDescriptionLabel);
	DDX_Control(pDX, IDC_ADJ_CATEGORY_LABEL, m_nxstaticAdjCategoryLabel);
	DDX_Control(pDX, IDC_SHIFT_BALANCES_LABEL, m_nxstaticShiftBalancesLabel);
	DDX_Control(pDX, IDC_INSCO_LABEL, m_nxstaticInsCoLabel);
	DDX_Control(pDX, IDC_INSCO_DROPDOWN_LABEL, m_nxstaticInsCoDropdownLabel);
	DDX_Control(pDX, IDC_PROVIDER_DROPDOWN_LABEL, m_nxstaticProviderDropdownLabel);
	DDX_Control(pDX, IDC_LOCATION_DROPDOWN_LABEL, m_nxstaticLocationDropdownLabel);
	DDX_Control(pDX, IDC_BTN_AUTO_SKIP_DUPLICATES, m_btnAutoSkipDuplicates);
	DDX_Control(pDX, IDC_ESTIMATED_OVERPAYMENT_LABEL, m_nxstaticEstimatedOverpaymentUnderpaymentLabel);	
	DDX_Control(pDX, IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING, m_btnConfigImportFilters);
	DDX_Control(pDX, IDC_EOB_PAY_DATE, m_dtPayDate);
	DDX_Control(pDX, IDC_EOB_ADJ_DATE, m_dtAdjDate);
	DDX_Control(pDX, IDC_CHECK_ENABLE_PAY_DATE, m_checkEnablePayDate);
	DDX_Control(pDX, IDC_CHECK_ENABLE_ADJ_DATE, m_checkEnableAdjDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEOBDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEOBDlg)
	ON_BN_CLICKED(IDC_BTN_IMPORT_835_FILE, OnBtnImport835File)
	ON_BN_CLICKED(IDC_PRINT_EOB, OnPrintEob)
	ON_BN_CLICKED(IDC_EDIT_PAY_DESC, OnEditPayDesc)
	ON_BN_CLICKED(IDC_EDIT_ADJ_DESC, OnEditAdjDesc)
	ON_BN_CLICKED(IDC_EDIT_PAY_CAT, OnEditPayCat)
	ON_BN_CLICKED(IDC_EDIT_ADJ_CAT, OnEditAdjCat)
	ON_BN_CLICKED(IDC_BTN_IMPORT_OHIP_FILE, OnBtnImportOhipFile)
	ON_BN_CLICKED(IDC_BTN_ADJUSTMENT_CODE_SETTINGS, OnBtnAdjustmentCodeSettings)
	ON_BN_CLICKED(IDC_BTN_AUTO_SKIP_DUPLICATES, OnBtnAutoSkipDuplicates)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING, OnBtnConfigureEremitImportFiltering)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_PAY_DATE, OnCheckEnablePayDate)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_ADJ_DATE, OnCheckEnableAdjDate)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEOBDlg message handlers

BOOL CEOBDlg::OnInitDialog() 
{
	try {
				
		CNxDialog::SetMinSize(914, 458); // (r.goldschmidt 2014-10-07 10:03) - PLID 62646 - Enforce a minimum size (default is 924x579)
		CNxDialog::OnInitDialog();
		ShowWindow(SW_SHOW); // PLID 62646 - Just show dialog before modal activities have concluded (restoring a maximized window necessitates this step)

		// (j.jones 2008-05-07 15:50) - PLID 29854 - set nxiconbutton styles for modernization
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_MODIFY); //this is the process button
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnImport835.AutoSet(NXB_MODIFY); //doesn't change data, but does reload the screen
		// (j.jones 2008-06-16 17:24) - PLID 21921 - added OHIP support
		m_btnImportOHIP.AutoSet(NXB_MODIFY); //doesn't change data, but does reload the screen
		// (j.jones 2008-11-24 09:39) - PLID 32075 - added m_btnConfigAdjCodesToSkip
		// (r.gonet 2016-04-18) - NX-100162 - Renamed the button.
		m_btnAdjustmentCodeSettings.AutoSet(NXB_MODIFY);
		// (j.jones 2009-06-09 12:10) - PLID 33863 - doesn't really modify saved data, but does change
		// how the EOB posting will behave
		m_btnAutoSkipDuplicates.AutoSet(NXB_MODIFY);
		// (j.jones 2010-02-08 15:33) - PLID 37174 - added ability to configure import filtering
		m_btnConfigImportFilters.AutoSet(NXB_MODIFY);

		// (j.jones 2009-07-01 16:27) - PLID 34778 - clear and hide the overpayment label
		m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
		m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);

		m_greenbrush.CreateSolidBrush(PaletteColor(0x009CC294));
		m_bluebrush.CreateSolidBrush(PaletteColor(0x00CCCC00));

		// (j.jones 2011-01-04 14:40) - PLID 16423 - added billing notes
		m_hNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);

		// (j.jones 2008-02-20 11:24) - PLID 29007 - Load all common preferences into the NxPropManager cache
		g_propManager.CachePropertiesInBulk("CEOBDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefEOBShiftAction' OR "
			"Name = 'DefEOBShiftPartialPay' OR "
			"Name = 'DefEOBBatchShiftedClaims' OR "
			"Name = 'DefaultERemitPayCategory' OR "
			"Name = 'DefaultERemitAdjCategory' OR "
			"Name = 'EOBFileManipulation' OR "
			"Name = 'UseEOBAdjustmentReasons' OR "
			// (j.jones 2008-03-26 10:11) - PLID 29407 - added ERemitReduceAdjustments preference
			"Name = 'ERemitReduceAdjustments' OR "
			// (j.jones 2010-02-08 11:46) - PLID 37182 - changed into ERemitPrimaryReduceAdjustments and ERemitSecondaryReduceAdjustments,
			// which continue to use ERemitReduceAdjustments as their default value
			"Name = 'ERemitPrimaryReduceAdjustments' OR "
			"Name = 'ERemitSecondaryReduceAdjustments' OR "
			// (j.jones 2008-06-17 09:10) - PLID 21921 - check the Ebilling format style, to possibly use OHIP e-remits
			"Name = 'EbillingFormatType' OR "
			// (j.jones 2008-11-21 10:51) - PLID 32076 - added preference to allow $0.00 adjustments
			"Name = 'ERemitAllowZeroDollarAdjustments' OR "
			// (j.jones 2009-03-05 14:53) - PLID 33220 - added preference to control shifting to patient responsibility by default
			"Name = 'ERemitAutoShiftPatientResp' OR "
			// (j.jones 2010-09-02 11:01) - PLID 38787 - split the above preference into primary/secondary, which default to the old value
			"Name = 'ERemitPrimaryAutoShiftPatientResp' OR "
			"Name = 'ERemitSecondaryAutoShiftPatientResp' OR "
			// (j.jones 2009-06-04 11:13) - PLID 34341 - added preference to use the charge provider on applied pays/adjs
			"Name = 'ERemitUseChargeProviderOnApplies' OR "
			// (j.jones 2009-06-26 12:36) - PLID 34736 - added EOBSupportOriginalCode
			"Name = 'EOBSupportOriginalCode' OR "
			// (j.jones 2009-10-06 10:56) - PLID 21304 - added ability to switch patient applies between charges
			"Name = 'ERemitShiftPatientPays' OR "
			// (j.jones 2010-03-15 10:54) - PLID 32184 - hidden setting to disable PeekAndPump usage
			"Name = 'EOB_EnablePeekAndPump' OR "
			// (j.jones 2010-11-30 09:52) - PLID 39434 - added preference to default EOB payment/adjustment dates to today
			"Name = 'DefaultEOBDate' OR "
			// (j.jones 2010-12-16 09:59) - PLID 41176 - added ability to skip posting adjustments and no payments
			"Name = 'ERemitSkipZeroPays' OR "
			// (j.jones 2011-01-06 16:54) - PLID 41785 - added ability to append patient responsibility reasons to a charge's billing note
			"Name = 'ERemit_AddCASPR_BillingNote' OR "
			"Name = 'ERemit_AddCASPR_BillingNote_ShowOnStatement' OR "
			"Name = 'ERemit_AddCASPR_BillingNote_DefCategory' OR "
			// (j.jones 2011-01-07 14:21) - PLID 41980 - added ability to append detailed dollar amounts to the charge's billing note
			"Name = 'ERemit_AddDetailedInfo_BillingNote' OR "
			"Name = 'ERemit_AddDetailedInfo_ShowOnStatement' OR "
			"Name = 'ERemit_AddDetailedInfo_DefCategory' OR "
			// (j.jones 2011-04-04 15:42) - PLID 42571 - added ability to ignore all secondary adjustments
			"Name = 'ERemit_IgnoreSecondaryAdjs' OR "
			// (j.jones 2011-07-14 16:11) - PLID 41440 - added ability to use charge location
			"Name = 'ERemitUseChargeLocationOnApplies' OR "
			// (b.spivey, October 01, 2012) - PLID 52666 - OHIP field -- So we can properly match OHIP claims. 
			"Name = 'OHIP_HealthNumberCustomField' OR "
			// (j.jones 2012-10-04 15:26) - PLID 52929 - added ability to ignore missing OHIP patients
			"Name = 'OHIPIgnoreMissingPatients' OR "
			// (j.dinatale 2012-11-06 12:04) - PLID 50792 - Auto Unbatch MA18 claims
			"Name = 'ERemit_UnbatchMA18orNA89' OR "
			// (b.spivey, October 31, 2012) - PLID 49943 - Cache property. 
			"Name = 'EOBUpdateBillOriginaRefNo' OR "
			// (s.dhole 2013-04-24 17:38) - PLID 56122
			"Name = 'ERemit_UnbatchMA18orNA89_MarkForwardToSecondary' "
			// (b.spivey, July 10, 2013) - PLID 56825 - CACHE. 
			"	OR Name = 'ERemitPostAdjustmentNoteWhenZeroPays' "
			"	OR Name = 'ERemitPostAdjustmentNoteWhenZeroPays_DefaultNoteCategory' "
			// (j.jones 2015-03-20 11:57) - PLID 65400 - added a preference to always void and correct when the system auto-unapplies a payment
			"OR Name = 'UnapplyFromCharge_VoidAndCorrect' "
			// (j.jones 2015-07-16 09:37) - PLID 60207 - added ability to control whether we respect the Processed As flag
			// (j.jones 2016-04-13 16:13) - NX-100184 - removed this preference
			//"OR Name = 'EOB_ProcessedAs' "
			")",
			_Q(GetCurrentUserName()));
		
		// (j.dinatale 2012-11-06 12:04) - PLID 50792 - Auto Unbatch MA18 claims
		m_bUnbatchMA18orN89 = GetRemotePropertyInt("ERemit_UnbatchMA18orNA89", 1, 0, "<None>", true);

		// (b.spivey, November 07, 2012) - PLID 49943 - cache in the dialog so this stays consistent. 
		m_nEOBUpdateBillOriginaRefNo = GetRemotePropertyInt("EOBUpdateBillOriginaRefNo", 1, 0, "<None>", true); 

		// (j.jones 2012-10-04 15:28) - PLID 52929 - added cache for the OHIPIgnoreMissingPatients preference
		m_bOHIPIgnoreMissingPatients = (GetRemotePropertyInt("OHIPIgnoreMissingPatients", 0, 0, "<None>", true) == 1);

		// (j.jones 2012-05-25 10:38) - PLID 44367 - we now allow minimizing now, but we do not
		// need to check the DisplayTaskbarIcons preference nor use WS_EX_APPWINDOW because this
		// dialog is always using the desktop as its parent, and always shows in the taskbar

		m_InsuranceCombo = BindNxDataListCtrl(this,IDC_EOB_INSURANCE_COMBO,GetRemoteData(), false);
		// (j.jones 2011-06-16 08:51) - PLID 39747 - the default insurance combo filter for when we aren't showing
		// specific insurance companies in the list, should just be "Archived = 0"
		m_strCurInsuranceComboFilter = DEFAULT_INSURANCE_FILTER;
		m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
		m_InsuranceCombo->Requery();
		// (s.tullis 2016-04-15 16:00) - NX-100211
		m_EOBList = BindNxDataList2Ctrl(this,IDC_EOB_DETAIL_LIST,GetRemoteData(),false);
		m_ProviderCombo = BindNxDataListCtrl(this,IDC_EOB_PROVIDER_COMBO,GetRemoteData(),true);
		m_LocationCombo = BindNxDataListCtrl(this,IDC_EOB_LOCATION_COMBO,GetRemoteData(),true);
		m_ShiftRespCombo = BindNxDataListCtrl(this,IDC_SHIFT_RESP_COMBO,GetRemoteData(),false);

		m_PayDescCombo = BindNxDataList2Ctrl(this,IDC_PAY_DESCRIPTION_COMBO,GetRemoteData(),true);
		m_PayCatCombo = BindNxDataList2Ctrl(this,IDC_PAY_CAT_COMBO,GetRemoteData(),true);
		m_AdjDescCombo = BindNxDataList2Ctrl(this,IDC_ADJ_DESCRIPTION_COMBO,GetRemoteData(),true);
		m_AdjCatCombo = BindNxDataList2Ctrl(this,IDC_ADJ_CAT_COMBO,GetRemoteData(),true);

		// (j.jones 2010-04-09 12:00) - PLID 31309 - added date controls,
		// disabled by default
		// (j.jones 2010-11-30 09:53) - PLID 39434 - check the DefaultEOBDate preference,
		// 0 - use EOB date, 1 - use today's date
		// enable & check the dates if using today's date, act as though it is an override
		BOOL bUseEOBDate = (GetRemotePropertyInt("DefaultEOBDate", 0, 0, "<None>", true) == 0);
		m_dtPayDate.SetValue(COleDateTime::GetCurrentTime());
		m_dtAdjDate.SetValue(COleDateTime::GetCurrentTime());
		m_checkEnablePayDate.SetCheck(!bUseEOBDate);
		m_checkEnableAdjDate.SetCheck(!bUseEOBDate);
		GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(!bUseEOBDate);
		GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(!bUseEOBDate);

		// (j.jones 2011-02-08 14:52) - PLID 42392 - added default insurance category for takeback adjustments,
		// check to make sure the category ID is valid
		// (j.jones 2012-02-13 10:54) - PLID 48084 - this is now obsolete, we no longer make
		// adjustments instead of payments
		/*
		m_nAdjOverageTakebackCategoryID = GetRemotePropertyInt("ERemit_AdjOverageTakebackCategoryID", -1, 0, "<None>", true);
		if(m_nAdjOverageTakebackCategoryID != -1 && !ReturnsRecordsParam("SELECT ID FROM PaymentGroupsT WHERE ID = {INT}", m_nAdjOverageTakebackCategoryID)) {
			//this is no longer a valid category
			m_nAdjOverageTakebackCategoryID = -1;
		}
		*/

		// (j.jones 2008-06-17 09:11) - PLID 21921 - if the FormatStyle is OHIP, then show the OHIP
		// import button, otherwise show the regular ANSI import button
		// (j.jones 2008-12-02 09:00) - PLID 32291 - changed the default to be ANSI
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		long nFormatType = GetRemotePropertyInt("EbillingFormatType", ANSI, 0, "<None>", TRUE);
		if (nFormatType == OHIP) {
			m_ERemitType = ertOhip;
		}
		else if (UseAlbertaHLINK()) {
			//TES 7/30/2014 - PLID 62580 - Added support for Alberta Assessment files
			m_ERemitType = ertAlberta;
		}
		else {
			m_ERemitType = ertAnsi;
		}
		if (m_ERemitType == ertOhip) {
			//show the OHIP button
			GetDlgItem(IDC_BTN_IMPORT_835_FILE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_IMPORT_OHIP_FILE)->ShowWindow(SW_SHOW);
			//hide the insurance company name label, there isn't one in an OHIP file
			GetDlgItem(IDC_INSCO_NAME_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_INSCO_LABEL)->ShowWindow(SW_HIDE);
			// (j.jones 2008-11-24 15:01) - PLID 32075 - hide the "ignore adjustment codes" button
			// (r.gonet 2016-04-18) - NX-100162 - Renamed the button.
			GetDlgItem(IDC_BTN_ADJUSTMENT_CODE_SETTINGS)->ShowWindow(SW_HIDE);
			// (j.jones 2010-02-08 15:18) - PLID 37174 - hide the "EOB filtering" button
			GetDlgItem(IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING)->ShowWindow(SW_HIDE);
			// (s.tullis 2016-04-15 16:00) - NX-100211
			// (j.jones 2011-03-18 09:35) - PLID 42157 - hide the EOB allowable column
			NXDATALIST2Lib::IColumnSettingsPtr pEOBCol = m_EOBList->GetColumn(COLUMN_EOB_ALLOWABLE);
			pEOBCol->PutColumnStyle(NXDATALIST2Lib::csVisible);
			pEOBCol->PutStoredWidth(0);
		}
		else {
			//show the ANSI button
			//TES 7/30/2014 - PLID 62580 - Change the text for Alberta files (we'll check m_ERemitType when they click it).
			if (m_ERemitType == ertAlberta) {
				SetDlgItemText(IDC_BTN_IMPORT_835_FILE, "Import Alberta Assessment File");
				//TES 10/20/2014 - PLID 62777 - We don't support adjustments for Alberta, so let's hide those columns
				NXDATALIST2Lib::IColumnSettingsPtr pAdjCol = m_EOBList->GetColumn(COLUMN_TOTAL_ADJUSTMENTS);
				pAdjCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
				pAdjCol->PutStoredWidth(0);
				pAdjCol = m_EOBList->GetColumn(COLUMN_ADJ_GROUP_CODE);
				pAdjCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
				pAdjCol->PutStoredWidth(0);
				pAdjCol = m_EOBList->GetColumn(COLUMN_ADJ_REASON_CODE);
				pAdjCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
				pAdjCol->PutStoredWidth(0);
				pAdjCol = m_EOBList->GetColumn(COLUMN_ADJ_REASON);
				pAdjCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
				pAdjCol->PutStoredWidth(0);
			}
			else {
				SetDlgItemText(IDC_BTN_IMPORT_835_FILE, "Import Electronic Remittance File");
			}
			GetDlgItem(IDC_BTN_IMPORT_835_FILE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_IMPORT_OHIP_FILE)->ShowWindow(SW_HIDE);
			//show the insurance company name label
			GetDlgItem(IDC_INSCO_NAME_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_INSCO_LABEL)->ShowWindow(SW_SHOW);
			
			// (j.jones 2010-02-08 15:18) - PLID 37174 - show the "EOB filtering" button
			// (j.jones 2008-11-24 15:01) - PLID 32075 - show the "ignore adjustment codes" button
			//TES 8/18/2014 - PLID 62580 - Hide this for Alberta
			if (m_ERemitType == ertAlberta) {
				GetDlgItem(IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING)->ShowWindow(SW_HIDE);
				// (r.gonet 2016-04-18) - NX-100162 - Renamed the button.
				GetDlgItem(IDC_BTN_ADJUSTMENT_CODE_SETTINGS)->ShowWindow(SW_HIDE);
			}
			else {
				GetDlgItem(IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING)->ShowWindow(SW_SHOW);
				// (r.gonet 2016-04-18) - NX-100162 - Renamed the button.
				GetDlgItem(IDC_BTN_ADJUSTMENT_CODE_SETTINGS)->ShowWindow(SW_SHOW);
		}
		}

		// (j.jones 2010-03-15 10:54) - PLID 32184 - added a hidden setting to disable PeekAndPump usage
		m_bEnablePeekAndPump = (GetRemotePropertyInt("EOB_EnablePeekAndPump", 1, 0, "<None>", true) == 1);
		m_EOBParser.m_bEnablePeekAndPump = m_bEnablePeekAndPump;
		m_OHIPParser.m_bEnablePeekAndPump = m_bEnablePeekAndPump;
		if(m_bEnablePeekAndPump) {
			Log("CEOBDlg: PeekAndPump Enabled");
		}
		else {
			Log("CEOBDlg: PeekAndPump Disabled");
		}

		m_LocationCombo->SetSelByColumn(0,GetCurrentLocationID());

		NXDATALISTLib::IRowSettingsPtr pRow = m_ShiftRespCombo->GetRow(-1);
		pRow->PutValue(0,long(0));
		pRow->PutValue(1,_bstr_t("<Do Not Shift>"));
		m_ShiftRespCombo->InsertRow(pRow,0);
		pRow = m_ShiftRespCombo->GetRow(-1);
		pRow->PutValue(0,long(1));
		pRow->PutValue(1,_bstr_t("Patient Responsibility"));
		m_ShiftRespCombo->InsertRow(pRow,1);
		pRow = m_ShiftRespCombo->GetRow(-1);
		pRow->PutValue(0,long(2));
		pRow->PutValue(1,_bstr_t("Next Highest Responsibility, or Patient"));
		m_ShiftRespCombo->InsertRow(pRow,2);
		pRow = m_ShiftRespCombo->GetRow(-1);

		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_PayCatCombo->GetNewRow();
		pRow2->PutValue(pccID,long(-1));
		pRow2->PutValue(pccCategory,_bstr_t(" <No Category Selected>"));
		m_PayCatCombo->AddRowSorted(pRow2, NULL);

		pRow2 = m_AdjCatCombo->GetNewRow();
		pRow2->PutValue(accID,long(-1));
		pRow2->PutValue(accCategory,_bstr_t(" <No Category Selected>"));
		m_AdjCatCombo->AddRowSorted(pRow2, NULL);

		m_ShiftRespCombo->SetSelByColumn(0,(long)GetRemotePropertyInt("DefEOBShiftAction",2,0,"<None>",true));
		m_checkOnlyShiftPostedCharges.SetCheck(GetRemotePropertyInt("DefEOBShiftPartialPay",0,0,"<None>",true) == 1);
		CheckDlgButton(IDC_CHECK_BATCH_CLAIMS,GetRemotePropertyInt("DefEOBBatchShiftedClaims",0,0,"<None>",true));

		long nDefaultPaymentCategoryID = GetRemotePropertyInt("DefaultERemitPayCategory", -1, 0, "<None>", true);
		long nDefaultAdjustmentCategoryID = GetRemotePropertyInt("DefaultERemitAdjCategory", -1, 0, "<None>", true);

		//ensure the default categories are valid
		// (j.armen 2012-02-20 11:05) - PLID 34344 - Parameratized
		if(nDefaultPaymentCategoryID != -1 && !ReturnsRecordsParam("SELECT ID FROM PaymentGroupsT WHERE ID = {INT}", nDefaultPaymentCategoryID)) {
			//not valid, so clear out the preference
			nDefaultPaymentCategoryID = 0;
			SetRemotePropertyInt("DefaultERemitPayCategory", -1, 0, "<None>");
		}

		// (j.armen 2012-02-20 11:06) - PLID 34344 - Parameratized
		if(nDefaultAdjustmentCategoryID != -1 && !ReturnsRecordsParam("SELECT ID FROM PaymentGroupsT WHERE ID = {INT}", nDefaultAdjustmentCategoryID)) {
			//not valid, so clear out the preference
			nDefaultAdjustmentCategoryID = 0;
			SetRemotePropertyInt("DefaultERemitAdjCategory", -1, 0, "<None>");
		}

		if(nDefaultPaymentCategoryID == -1) {
			nDefaultPaymentCategoryID = 0;
		}
		if(nDefaultAdjustmentCategoryID == -1) {
			nDefaultAdjustmentCategoryID = 0;
		}

		// (j.jones 2008-02-20 11:25) - PLID 29007 - rather than always using the default categories,
		// now just set the dropdowns to those categories
		// Note: there is a default category for payments, indepdendent of EOBs, but we are intentionally
		// ignoring that preference and exclusively using the EOB-only preference instead
		if(nDefaultPaymentCategoryID > 0) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_PayCatCombo->TrySetSelByColumn_Deprecated(pccID, nDefaultPaymentCategoryID);
		}
		if(nDefaultAdjustmentCategoryID > 0) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_AdjCatCombo->TrySetSelByColumn_Deprecated(accID, nDefaultAdjustmentCategoryID);
		}

		// (j.jones 2008-02-20 11:35) - PLID 29007 - set the descriptions to default values
		SetDlgItemText(IDC_PAY_DESC, "E-Remittance Insurance Payment");
		SetDlgItemText(IDC_ADJ_DESC, "E-Remittance Insurance Adjustment");

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Ready to import.");

		// Moved to the Dialaog Loading Code
		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		SetWindowPos(NULL, 0, 0, 994, 738, 0);
		// (s.tullis 2016-04-21 10:17) - NX-100211 - Show Maximized
		ShowWindow(SW_SHOWMAXIMIZED);

		// (b.spivey - July 22nd, 2014) - PLID 62917 - Automatically prompt for a file if an autoload path is not selected. 
		if (m_strAutoOpenFilePath.IsEmpty() || !DoesExist(m_strAutoOpenFilePath)) {

			//browse for a file
			CFileDialog BrowseFiles(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, NULL, this);
			BrowseFiles.DoModal();
			//If they selected one, great. If not, the auto load was going to fail either way. 
			m_strAutoOpenFilePath = BrowseFiles.GetPathName();
		}

		// (j.jones 2008-12-19 08:46) - PLID 32519 - added ability to auto-open a file
		if(!m_strAutoOpenFilePath.IsEmpty() && DoesExist(m_strAutoOpenFilePath)) {
			//set a timer to load after OnInitDialog completes
			SetTimer(IDT_INIT_LOAD, 250, NULL);
		}
		else {
			//clear the path, incase it wasn't valid
			m_strAutoOpenFilePath = "";
		}
	
	}NxCatchAll("Error in CEOBDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEOBDlg::OnBtnImport835File() 
{
	try {

		//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
		if (m_ERemitType == ertAlberta) {
			ImportAlbertaFile();
			return;
		}

		// (j.jones 2007-06-29 08:53) - PLID 23951 - fire a use of the license
		if(!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrUse)) {
			return;
		}

		if(m_EOBList->GetRowCount() > 0) {
			if(IDNO == MessageBox("You have already imported a remittance file. If you continue, no data will be posted.\n\n"
				"Do you wish to clear out the current data and import a new file?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}
			else {

				// (j.jones 2009-07-01 16:27) - PLID 34778 - clear and hide the overpayment label
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);

				m_EOBList->Clear();

				// (j.jones 2011-02-09 11:34) - PLID 42391 - hide the reversal column
				NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
				pReversalCol->PutStoredWidth(0);

				// (j.jones 2011-06-16 08:51) - PLID 39747 - the default insurance combo filter for when we aren't showing
				// specific insurance companies in the list, should just be "Archived = 0"
				m_strCurInsuranceComboFilter = DEFAULT_INSURANCE_FILTER;
				m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
				m_InsuranceCombo->Requery();
				SetDlgItemText(IDC_INSCO_NAME_LABEL, "<None Specified>");
				SetDlgItemText(IDC_PROVIDER_LABEL, "Provider:");
				SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL,FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
				// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
				m_EOBParser.ClearAllEOBs();
			}
		}

		//since we pump messages during the processing, make sure that all buttons are disabled
		//during the processing. We re-enable them when the processing is complete.
		EnableButtons(FALSE);

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Importing file...");

		m_EOBParser.m_ptrProgressBar = &m_progress;
		m_EOBParser.m_ptrProgressStatus = &m_progressStatus;

		COleCurrency cyTotalPayments = COleCurrency(0,0);

		BOOL bWarnMissingInsuredParties = FALSE;
		BOOL bWarnMissingBillIDs = FALSE;
		BOOL bHasUnpostableReversals = FALSE;
		// (b.spivey, January 17, 2012) - PLID 47594 - Initializing this to TRUE just ended up throwing the warning every single time.
		BOOL bWarnInvalidPayTotals = FALSE;

		// (j.jones 2008-12-19 08:46) - PLID 32519 - added ability to auto-open a file
		if(!m_strAutoOpenFilePath.IsEmpty() && DoesExist(m_strAutoOpenFilePath)) {
			//tell the parser to use this default file
			m_EOBParser.m_strFileName = m_strAutoOpenFilePath;			
		}
		else {
			//ensure the parser has no default
			m_EOBParser.m_strFileName = "";
		}
		//clear our default file now
		m_strAutoOpenFilePath = "";

		// (j.jones 2012-05-25 13:59) - PLID 44367 - pass this dialog as the parent
		if(m_EOBParser.ParseFile(this)) {

			if(m_EOBParser.m_CountOfEOBs == 1)
				SetDlgItemText(IDC_PAYMENT_LABEL,"Total Payment:");
			else {
				CString str;
				str.Format("Total Payments (%li):",m_EOBParser.m_CountOfEOBs);
				SetDlgItemText(IDC_PAYMENT_LABEL,str);
			}

			// (j.jones 2010-04-09 12:00) - PLID 31309 - added date controls,
			// disable the date if they enabled it and fill with the first check date
			// (j.jones 2010-11-30 09:53) - PLID 39434 - check the DefaultEOBDate preference,
			// 0 - use EOB date, 1 - use today's date
			// we would have set to today's date when the dialog opened, so if the preference is
			// set to 1, do nothing here
			BOOL bUseEOBDate = (GetRemotePropertyInt("DefaultEOBDate", 0, 0, "<None>", true) == 0);
			if(bUseEOBDate) {
				m_checkEnablePayDate.SetCheck(FALSE);
				m_checkEnableAdjDate.SetCheck(FALSE);
				GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(FALSE);
				GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(FALSE);
				COleDateTime dtPayDate = COleDateTime::GetCurrentTime();
				if(m_EOBParser.m_arypEOBInfo.GetSize() > 0) {
					const EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[0];
					if(pEOB->dtCheckDate.GetStatus() != COleDateTime::invalid) {
						dtPayDate = pEOB->dtCheckDate;
					}
				}
				m_dtPayDate.SetValue(dtPayDate);
				m_dtAdjDate.SetValue(dtPayDate);
			}

			// (j.jones 2011-04-04 15:50) - PLID 42571 - calculate the insured party IDs
			// prior to calling CheckAdjustmentCodesToIgnore
			for(int a=0;a<m_EOBParser.m_arypEOBInfo.GetSize();a++) {	

				EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];

				m_progress.SetPos(0);

				CString str;
				if(m_EOBParser.m_CountOfEOBs == 1)
					str = "Searching financial data...";
				else
					str.Format("Searching financial data (EOB %li)...",pEOB->nIndex+1);
				m_progressStatus.SetWindowText(str);

				//calculate the IDs of the insurance co, bills, and charges
				m_EOBParser.CalcInternalIDs(pEOB);

				for(int i=0;i<pEOB->arypEOBClaimInfo.GetSize();i++) {
					EOBClaimInfo *pClaim = pEOB->arypEOBClaimInfo[i];

					for(int j=0;j<pClaim->arypEOBLineItemInfo.GetSize();j++) {

						EOBLineItemInfo *pCharge = pClaim->arypEOBLineItemInfo[j];

						// (j.jones 2011-04-04 15:56) - I don't like that we do this per charge, but I don't want
						// to change it now, because filtering per charge means we pick the most accurate insured
						// party per charge, and if we were wrong, we'd only be wrong on that charge rather than
						// the whole bill
						// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct	
						// (j.jones 2012-05-01 15:02) - PLID 47477 - supported the ProcessedAs flag
						// (j.jones 2016-04-13 16:20) - NX-100184 - added EOB pointer, removed redundant parameters that already existed in pClaim and pEOB
						EInsuredPartyInfo eInsuredInfo = CalcIndivPatientInsuredPartyID(pEOB, pClaim, pCharge->nChargeID);
						pCharge->nInsuredPartyID = eInsuredInfo.nInsuredPartyID;
						pCharge->bIsSecondaryResp = (eInsuredInfo.nCategoryPlacement == 2);

						// (j.jones 2012-04-20 16:43) - PLID 49846 - If this claim is tagged as a reversal,
						// try to find the reversed payment and adjustment IDs.
						// (j.dinatale 2013-01-10 09:49) - PLID 54544 - CalcReversedPaymentAndAdjustmentIDs will check if the current charge is a reversal
						CalcReversedPaymentAndAdjustmentIDs(pClaim, pCharge);
					}
				}
			}

			// (j.jones 2008-11-24 15:02) - PLID 32075 - before populating the list, check the
			// adjustment codes that should be ignored
			m_EOBParser.CheckAdjustmentCodesToIgnore();

			CStringArray arystrInvalidClaims;

			// (j.jones 2010-02-09 15:01) - PLID 37254 - if we are filtering on the claim level,
			// we may have excluded claims and needed to reduce the batch payment amount,
			// or even worse excluded all claims on given EOB and need to remove it completely
			if(m_EOBParser.m_arystrClaimFilteredIDs.GetSize() > 0) {
				//run through and make corrections
				for(int a=m_EOBParser.m_arypEOBInfo.GetSize()-1; a>=0; a--) {

					EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];
					if(pEOB->arypEOBClaimInfo.GetSize() == 0) {
						//this EOB is empty, clear it
						m_EOBParser.ClearEOB(pEOB);
						continue;
					}
					else {
						COleCurrency cyTotalClaimPayments = COleCurrency(0,0);
						for(int i=0;i<pEOB->arypEOBClaimInfo.GetSize();i++) {
							const EOBClaimInfo *pClaim = pEOB->arypEOBClaimInfo[i];
							cyTotalClaimPayments += pClaim->cyClaimPaymentAmt;
						}

						if(cyTotalClaimPayments < pEOB->cyTotalPaymentAmt) {
							//the EOB total has been reduced!
							pEOB->cyTotalPaymentAmt = cyTotalClaimPayments;
						}
					}
				}
			}

			//for each EOB
			for(int a=0;a<m_EOBParser.m_arypEOBInfo.GetSize();a++) {
			
				const EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];

				m_progress.SetPos(0);
				m_progress.SetRange(0,pEOB->arypEOBClaimInfo.GetSize());
				m_progress.SetStep(1);

				CString str;

				if(m_EOBParser.m_CountOfEOBs == 1)
					str = "Populating list...";
				else
					str.Format("Populating list (EOB %li)...",pEOB->nIndex+1);
				m_progressStatus.SetWindowText(str);

				//now propagate the list

				if(pEOB->strInsuranceCoName != "")
					SetDlgItemText(IDC_INSCO_NAME_LABEL, pEOB->strInsuranceCoName);
				else
					SetDlgItemText(IDC_INSCO_NAME_LABEL, "<None Specified>");

				if(pEOB->strProviderName != "") {
					SetDlgItemText(IDC_PROVIDER_LABEL, "Provider:");
					SetDlgItemText(IDC_PROVIDER_NAME_LABEL, pEOB->strProviderName);
				}
				else {
					SetDlgItemText(IDC_PROVIDER_LABEL, "Payee:");
					if(pEOB->strPayeeName != "") {
						SetDlgItemText(IDC_PROVIDER_LABEL, "Payee:");
						SetDlgItemText(IDC_PROVIDER_NAME_LABEL, pEOB->strPayeeName);
					}
					else {
						SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
					}
				}

				cyTotalPayments += pEOB->cyTotalPaymentAmt;
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL,FormatCurrencyForInterface(cyTotalPayments, TRUE, TRUE));

				for(int i=0;i<pEOB->arypEOBClaimInfo.GetSize();i++) {
					const EOBClaimInfo *pClaim = pEOB->arypEOBClaimInfo[i];

					// (j.jones 2011-09-28 14:20) - PLID 45486 - I removed a bunch of dead, commented out code
					// for handling the quirky differences between claim pays/adjustments and charge pays/adjustments.
					// Each claim has a payment total and an adjustment total, but so does each charge. We only
					// post the charge amounts. If the amounts per charge don't match the amounts per claim...
					// ...then we have no idea what we're doing. Code that used to do stupid things to try and
					// handle it was formerly here. Now we will just call HasInvalidPayTotals() later and skip
					// the claim if the totals do not match.

					// (j.jones 2010-01-27 11:48) - PLID 36998 - as we populate the screen, check and see if we have
					// any invalid claim files, and warn accordingly
					if(pClaim->arypEOBLineItemInfo.GetSize() == 0) {
						//This claim is invalid, as we received no services under it.
						//We would have already logged this in the EOB.txt, but we need
						//to warn the user before they post this EOB.
						CString strClaimWarn;
						if(pClaim->bBillDatePresent) {
							strClaimWarn.Format("%s, %s %s - Bill Date: %s - Amount: %s\r\n", FixCapitalization(pClaim->strPatientLast), FixCapitalization(pClaim->strPatientFirst), FixCapitalization(pClaim->strPatientMiddle),
								FormatDateTimeForInterface(pClaim->dtBillDate, NULL, dtoDate), FormatCurrencyForInterface(pClaim->cyClaimChargeAmt));
						}
						else {
							strClaimWarn.Format("%s, %s %s - Amount: %s\r\n", FixCapitalization(pClaim->strPatientLast), FixCapitalization(pClaim->strPatientFirst), FixCapitalization(pClaim->strPatientMiddle),
								FormatCurrencyForInterface(pClaim->cyClaimChargeAmt));
						}
						arystrInvalidClaims.Add(strClaimWarn);
					}

					for(int j=0;j<pClaim->arypEOBLineItemInfo.GetSize();j++) {
						// (s.tullis 2016-04-15 16:00) - NX-100211
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

						const EOBLineItemInfo *pCharge = pClaim->arypEOBLineItemInfo[j];

						pRow->PutValue(COLUMN_EOB_INDEX, (long)pEOB->nIndex);
						pRow->PutValue(COLUMN_PATIENT_ID, (long)pClaim->nPatientID);
						// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
						pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pClaim->nUserDefinedID);
						
						pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pClaim->nPatientID)));
						pRow->PutValue(COLUMN_BILL_ID, (long)pClaim->nBillID);
						pRow->PutValue(COLUMN_CHARGE_ID, (long)pCharge->nChargeID);
						
						// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
						_variant_t varBirthDate = g_cvarNull;
						COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pClaim->nPatientID);
						if(dtBirthDate.GetStatus() != COleDateTime::invalid) {
							varBirthDate.vt = VT_DATE;
							varBirthDate.date = dtBirthDate;
						}
						pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);
						pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, ""); //this isn't an OHIP import
						pRow->PutValue(COLUMN_OHIP_VERSION_CODE, "");

						pRow->PutValue(COLUMN_DUPLICATE, (pClaim->bDuplicateClaim ? (long)1 : (long)0));

						// (j.jones 2011-02-09 11:24) - PLID 42391 - track if this claim is a reversal
						pRow->PutValue(COLUMN_REVERSAL, (pClaim->bIsReversedClaim || pClaim->bIsRepostedClaim ? g_cvarTrue : g_cvarFalse));

						// (j.jones 2011-02-09 11:34) - PLID 42391 - if a reversal, show the column,
						// track that we need to warn, and color the line blue (later code may change
						// the color to red)
						// (s.tullis 2016-04-15 16:00) - NX-100211
						if(pClaim->bIsReversedClaim || pClaim->bIsRepostedClaim) {
							NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
							pReversalCol->PutStoredWidth(58);
							pRow->PutForeColor(RGB(0,0,150));

							// (j.jones 2012-04-19 15:53) - PLID 35306 - if this reversal is unpostable, flag it as such,
							// and color the line red
							if(IsUnpostableReversalCharge(pCharge, pClaim)) {
								bHasUnpostableReversals = TRUE;
								pRow->PutForeColor(RGB(192,0,0));
							}
						}
						
						// (j.jones 2011-09-28 14:03) - PLID 45486 - If we have invalid payment totals,
						// just color the line gray for now, we won't show a column for this.
						// This color would override the reversal color.
						if(m_EOBParser.HasInvalidPayTotals(pClaim)) {

							bWarnInvalidPayTotals = TRUE;
							pRow->PutForeColor(RGB(127, 127, 127));
						}

						if(pClaim->nBillID == -1) {
							bWarnMissingBillIDs = TRUE;
							pRow->PutForeColor(RGB(192,0,0));
						}

						//try to set the provider ID
						// (j.jones 2008-06-18 14:25) - PLID 21921 - added efficiency by caching this value
						if(m_ProviderCombo->CurSel == -1) {
							long nProvID = pCharge->nProviderID;
							if(nProvID != -1) {
								m_ProviderCombo->SetSelByColumn(0,nProvID);
							}
						}

						_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}", pClaim->nBillID);
						if(!rs->eof) {
							pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
						}
						else
							pRow->PutValue(COLUMN_BILL_DATE, g_cvarNull);
						rs->Close();

						// (j.jones 2011-03-18 09:46) - PLID 42157 - moved the insurance info. calculations
						// earlier in this function
						long nInsuranceCoID = -1;

						// (j.jones 2005-04-27 16:11) - PLID 16324 - We need to be able to apply an EOB payment
						// to multiple insurance companies from the same HCFA group. This means we need to narrow
						// down to one HCFA group, and on a per-patient level specifically find each individual insured party.
						// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
						// (j.jones 2011-04-04 15:49) - PLID 42571 - this is now calculated earlier in the process and stored in pClaim
						long nInsuredPartyID = pCharge->nInsuredPartyID;
						// (s.tullis 2016-04-20 9:50) - NX-100185
						pRow->PutValue(COLUMN_INSURED_PARTY_ID,(long)nInsuredPartyID);
						pRow->PutRefCellFormatOverride(COLUMN_INSURED_PARTY_ID, GetInsuredPartyColumnCombo((long)pClaim->nPatientID));
						rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID, InsuranceCoT.Name, "
							"HCFASetupT.ID AS HCFASetupID, HCFASetupT.Name AS HCFASetupName, InsuredPartyT.RespTypeID, "
							"InsuredPartyT.SubmitAsPrimary "
							"FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
							"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
						if(!rs->eof) {
							nInsuranceCoID = AdoFldLong(rs, "PersonID", -1);
							pRow->PutValue(COLUMN_INS_CO_ID, nInsuranceCoID);
							pRow->PutValue(COLUMN_INS_CO_NAME,_bstr_t(AdoFldString(rs, "Name", "")));
							// (j.armen 2012-02-20 11:04) - PLID 48239 - Fill correctly with the HCFASetupID
							pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(rs, "HCFASetupID", -1));
							pRow->PutValue(COLUMN_HCFA_GROUP_NAME,_bstr_t(AdoFldString(rs, "HCFASetupName", "")));
							// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
							pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(rs, "RespTypeID", -1));
							// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
							pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, rs->Fields->Item["SubmitAsPrimary"]->Value);

						}
						else {
							pRow->PutValue(COLUMN_INS_CO_ID, (long)-1);
							pRow->PutValue(COLUMN_INS_CO_NAME, "");
							pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);
							pRow->PutValue(COLUMN_HCFA_GROUP_NAME, "");
							pRow->PutValue(COLUMN_RESP_TYPE_ID, (long)-1);
							pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, g_cvarFalse);
						}
						rs->Close();

						if(nInsuredPartyID == -1) {
							bWarnMissingInsuredParties = TRUE;
							pRow->PutForeColor(RGB(192,0,0));
						}

						// (j.jones 2011-03-18 09:41) - PLID 42157 - added FeeSchedAllowable
						// (d.lange 2015-11-30 14:58) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
						rs = CreateParamRecordset("SELECT Date, ItemCode, "
							"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
							"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes, "
							"Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, AllowableInsuredQ.InsuranceCoID) * ChargesT.Quantity * "
							"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
							"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
							"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
							"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) ), 2) AS FeeSchedAllowableQty "
							"FROM ChargesT "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
							"LEFT JOIN ( "
							"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
							"	FROM InsuredPartyT "
							"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
							") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
							"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0", pCharge->nChargeID);

						COleCurrency cyChargeAmt = COleCurrency(0,0);

						if(!rs->eof) {

							pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);
							
							//should be equivalent to pCharge->strServiceID
							pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							_variant_t varNoteIcon;
							if(!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
								//load from the datalist's icon for no notes
								varNoteIcon = (LPCTSTR)"BITMAP:FILE";
							}
							else {
								//load our icon for having notes
								varNoteIcon = (long)m_hNotes;
							}
							pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

							cyChargeAmt = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));

							// (j.jones 2011-03-18 09:32) - PLID 42157 - added FeeSchedAllowable, which multiplies by
							// quantity and modifiers
							pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, rs->Fields->Item["FeeSchedAllowableQty"]->Value);
						}
						else {
							pRow->PutValue(COLUMN_CHARGE_DATE, g_cvarNull);
							pRow->PutValue(COLUMN_CPT_CODE, g_cvarNull);
							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							pRow->PutValue(COLUMN_NOTE_ICON, g_cvarNull);
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, g_cvarNull);
							pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);
						}
						rs->Close();

						//Note: There are two kinds of adjustments, a claim adjustment and a charge adjustment.
						//The claim adjustment applies to the claim as a whole, the charge adjustment
						//applies to individual line items. They are two distinct, separate adjustments.
						//Neither is a subset of the other.

						//add up the total charge adjustments
						COleCurrency cyTotalChargeAdjustment = COleCurrency(0,0);
						COleCurrency cyTotalPositiveAdjustment = COleCurrency(0, 0);
						COleCurrency cyTotalPatientResp = COleCurrency(0,0);
						CString strAdjustmentDescription;
						CString strAdjustmentGroupCode;
						CString strAdjustmentReasonCode;
						// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
						for(int z=0;z<pCharge->arypEOBAdjustmentInfo.GetSize();z++) {
							const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[z];
							if(pAdj->bPatResp) {
								//reduces the balance by shifting to patient
								cyTotalPatientResp += pAdj->cyAdjustmentAmt;
							}
							else {
								//reduces the balance with a Practice adjustment
								cyTotalChargeAdjustment += pAdj->cyAdjustmentAmt;

								// (j.jones 2016-04-19 13:14) - NX-100161 - track positive adjustments only in a separate variable
								if (pAdj->cyAdjustmentAmt > COleCurrency(0, 0)) {
									cyTotalPositiveAdjustment += pAdj->cyAdjustmentAmt;
								}

								//save the description
								if(!pAdj->strReason.IsEmpty()) {
									if(!strAdjustmentDescription.IsEmpty()) {
										strAdjustmentDescription += "; ";
									}
									strAdjustmentDescription += pAdj->strReason;									
								}

								// (j.jones 2006-11-16 10:41) - PLID 23551 - we now store the
								// group code and reason code on each adjustment
								if(!pAdj->strGroupCode.IsEmpty()) {
									// (j.jones 2008-11-17 13:45) - PLID 32045 - append additional codes
									if(!strAdjustmentGroupCode.IsEmpty()) {
										strAdjustmentGroupCode += "; ";
									}									
									strAdjustmentGroupCode += pAdj->strGroupCode;
								}
								if(!pAdj->strReasonCode.IsEmpty()) {
									// (j.jones 2008-11-17 13:45) - PLID 32045 - append additional codes
									if(!strAdjustmentReasonCode.IsEmpty()) {
										strAdjustmentReasonCode += "; ";
									}									
									strAdjustmentReasonCode += pAdj->strReasonCode;
								}
							}
						}

						pRow->PutValue(COLUMN_PAYMENT, _variant_t(COleCurrency(pCharge->cyLineItemPaymentAmt)));

						// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
						// (j.jones 2016-04-19 13:14) - NX-100161 - old column removed, now this is the total of positive adjustments only
						pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(cyTotalPositiveAdjustment)));

						pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(cyTotalChargeAdjustment)));

						// (j.jones 2011-03-18 09:32) - PLID 42157 - added EOB allowable, can be invalid if
						// none was listed in the EOB
						if(pCharge->cyChargeAllowedAmt.GetStatus() == COleCurrency::invalid) {
							pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);
						}
						else {
							pRow->PutValue(COLUMN_EOB_ALLOWABLE, _variant_t(COleCurrency(pCharge->cyChargeAllowedAmt)));
						}

						pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _variant_t(_bstr_t(strAdjustmentGroupCode)));

						pRow->PutValue(COLUMN_ADJ_REASON_CODE, _variant_t(_bstr_t(strAdjustmentReasonCode)));

						pRow->PutValue(COLUMN_ADJ_REASON, _variant_t(_bstr_t(strAdjustmentDescription)));

						pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(cyTotalPatientResp));

						//get the total of current patient applies made on this charge
						COleCurrency cyPatApplies = COleCurrency(0,0);
						COleCurrency cyPatientResp = GetChargePatientResp(pCharge->nChargeID);
						COleCurrency cyPatientBalance = GetChargePatientBalance(pCharge->nChargeID, pClaim->nPatientID);
						cyPatApplies = cyPatientResp - cyPatientBalance;
						pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));
						
						//calculate the anticipated patient balance after this EOB is applied
						COleCurrency cyPatBalance = COleCurrency(0,0);
						//expected patient resp - existing patient applies
						cyPatBalance = (cyPatientResp < cyTotalPatientResp ? cyTotalPatientResp : cyPatientResp) - cyPatApplies;
						pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

						//calculate the anticipated insurance balance after this EOB is applied
						COleCurrency cyInsBalance = COleCurrency(0,0);
						cyInsBalance = cyChargeAmt - cyTotalPatientResp - pCharge->cyLineItemPaymentAmt - cyTotalChargeAdjustment;
						pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

						// (j.jones 2011-02-09 12:06) - PLID 42391 - auto-skip if a reversal
						// (j.jones 2011-09-28 14:03) - PLID 45486 - auto-skip if we have invalid payment totals
						// (j.jones 2012-04-18 15:59) - PLID 35306 - we support posting reversals now, so we won't auto-skip those anymore
						pRow->PutValue(COLUMN_SKIPPED, m_EOBParser.HasInvalidPayTotals(pClaim) ? g_cvarTrue : g_cvarFalse);

						// (j.jones 2008-11-17 15:23) - PLID 32045 - now we track the claim & charge pointers in the datalist
						pRow->PutValue(COLUMN_CLAIM_PTR, (long)pClaim);
						pRow->PutValue(COLUMN_CHARGE_PTR, (long)pCharge);
						// (s.tullis 2016-04-15 16:00) - NX-100211
						m_EOBList->AddRowAtEnd(pRow,NULL);

						// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
						// function, as we may have disabled this ability
						PeekAndPump_EOBDlg();
					}

					//now we have only processed the charges specified in the remittance file, there could be more
					// (j.jones 2009-08-21 11:31) - PLID 35303 - only add charges if we have already added charges from this bill,
					// just because we have a pClaim entry with that nBillID does not mean we added charges from it
					// (j.jones 2011-03-18 15:03) - PLID 42905 - we already have the charge IDs in our memory objects,
					// what we need to do is just get a list of all charges for the same patient that we are going to 
					// post to, because maybe we just haven't added their rows yet, but will later
					CArray<long,long> aryUsedCharges;
					m_EOBParser.GetChargeIDsByPatientID(pClaim->nPatientID, aryUsedCharges);
					if(aryUsedCharges.GetSize() > 0) {
						// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
						_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID "
							"FROM ChargesT "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"WHERE LineItemT.Deleted = 0 "
							"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"AND BillID = {INT} "
							"AND ChargesT.ID NOT IN ({INTARRAY}) "
							"AND BillID IN (SELECT BillID FROM ChargesT WHERE ID IN ({INTARRAY}))", pClaim->nBillID, aryUsedCharges, aryUsedCharges);
						while(!rsCharges->eof) {
							// (s.tullis 2016-04-15 16:00) - NX-100211
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

							long nChargeID = AdoFldLong(rsCharges, "ID");

							pRow->PutValue(COLUMN_EOB_INDEX, (long)pEOB->nIndex);
							pRow->PutValue(COLUMN_PATIENT_ID, (long)pClaim->nPatientID);
							// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
							pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pClaim->nUserDefinedID);
							pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pClaim->nPatientID)));
							pRow->PutValue(COLUMN_BILL_ID, (long)pClaim->nBillID);
							pRow->PutValue(COLUMN_CHARGE_ID, (long)nChargeID);

							// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
							_variant_t varBirthDate = g_cvarNull;
							COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pClaim->nPatientID);
							if(dtBirthDate.GetStatus() != COleDateTime::invalid) {
								varBirthDate.vt = VT_DATE;
								varBirthDate.date = dtBirthDate;
							}
							pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);
							pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, ""); //this isn't an OHIP import
							pRow->PutValue(COLUMN_OHIP_VERSION_CODE, "");

							pRow->PutValue(COLUMN_DUPLICATE, (pClaim->bDuplicateClaim ? (long)1 : (long)0));

							// (j.jones 2011-02-09 11:24) - PLID 42391 - track if this claim is a reversal
							pRow->PutValue(COLUMN_REVERSAL, (pClaim->bIsReversedClaim || pClaim->bIsRepostedClaim ? g_cvarTrue : g_cvarFalse));

							// (j.jones 2011-02-09 11:34) - PLID 42391 - if a reversal, show the column,
							// track that we need to warn, and color the line blue (later code may change
							// the color to red)
							// (s.tullis 2016-04-15 16:00) - NX-100211
							if(pClaim->bIsReversedClaim || pClaim->bIsRepostedClaim) {
								NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
								pReversalCol->PutStoredWidth(58);
								pRow->PutForeColor(RGB(0,0,150));

								// (j.jones 2012-04-19 15:53) - PLID 35306 - since this is a manually added charge that
								// is unreferenced by the EOB, we do not need to check if it is unpostable
							}

							_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",pClaim->nBillID);
							if(!rs->eof) {
								pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
							}
							else
								pRow->PutValue(COLUMN_BILL_DATE, g_cvarNull);
							rs->Close();

							// (j.jones 2011-03-18 09:46) - PLID 42157 - moved the insurance info. calculations
							// earlier in this function
							long nInsuranceCoID = -1;

							// (j.jones 2005-04-27 16:11) - PLID 16324 - We need to be able to apply an EOB payment
							// to multiple insurance companies from the same HCFA group. This means we need to narrow
							// down to one HCFA group, and on a per-patient level specifically find each individual insured party.
							// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
							// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct	
							// (j.jones 2012-05-01 15:02) - PLID 47477 - supported the ProcessedAs flag
							// (j.jones 2016-04-13 16:20) - NX-100184 - added EOB pointer, removed redundant parameters that already existed in pClaim and pEOB
							EInsuredPartyInfo eInsuredInfo = CalcIndivPatientInsuredPartyID(pEOB, pClaim, nChargeID);
							// (s.tullis 2016-04-20 9:50) - NX-100185 - 
							pRow->PutRefCellFormatOverride(COLUMN_INSURED_PARTY_ID, GetInsuredPartyColumnCombo((long)pClaim->nPatientID));
							pRow->PutValue(COLUMN_INSURED_PARTY_ID,(long)eInsuredInfo.nInsuredPartyID);
							rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID, InsuranceCoT.Name, "
								"HCFASetupT.ID AS HCFASetupID, HCFASetupT.Name AS HCFASetupName, InsuredPartyT.RespTypeID, "
								"InsuredPartyT.SubmitAsPrimary "
								"FROM InsuranceCoT "
								"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
								"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
								"WHERE InsuredPartyT.PersonID = {INT}", eInsuredInfo.nInsuredPartyID);
							if(!rs->eof) {
								nInsuranceCoID = AdoFldLong(rs, "PersonID", -1);
								pRow->PutValue(COLUMN_INS_CO_ID, nInsuranceCoID);
								pRow->PutValue(COLUMN_INS_CO_NAME,_bstr_t(AdoFldString(rs, "Name", "")));
								pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(rs, "HCFASetupID", -1));
								pRow->PutValue(COLUMN_HCFA_GROUP_NAME,_bstr_t(AdoFldString(rs, "HCFASetupName", "")));
								// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
								pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(rs, "RespTypeID", -1));
								// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
								pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, rs->Fields->Item["SubmitAsPrimary"]->Value);
							}
							else {
								pRow->PutValue(COLUMN_INS_CO_ID, (long)-1);
								pRow->PutValue(COLUMN_INS_CO_NAME, "");
								pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);
								pRow->PutValue(COLUMN_HCFA_GROUP_NAME, "");
								pRow->PutValue(COLUMN_RESP_TYPE_ID, (long)-1);
								pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, g_cvarFalse);
							}
							rs->Close();

							if(eInsuredInfo.nInsuredPartyID == -1) {
								bWarnMissingInsuredParties = TRUE;
								pRow->PutForeColor(RGB(192,0,0));
							}

							// (j.jones 2011-03-18 09:41) - PLID 42157 - added FeeSchedAllowable
							// (d.lange 2015-11-30 14:59) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
							rs = CreateParamRecordset("SELECT Date, ItemCode, "
								"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
								"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes, "
								"Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, AllowableInsuredQ.InsuranceCoID) * ChargesT.Quantity * "
								"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
								"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
								"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
								"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) ), 2) AS FeeSchedAllowableQty "
								"FROM ChargesT "
								"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
								"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
								"LEFT JOIN ( "
								"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
								"	FROM InsuredPartyT "
								"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
								") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
								"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0", nChargeID);

							COleCurrency cyChargeAmt = COleCurrency(0,0);
							COleCurrency cyPaymentAmt = COleCurrency(0,0);
							COleCurrency cyPatientAmt = COleCurrency(0,0);
							CString strAdjustmentDescription = "";
							CString strAdjustmentGroupCode = "";
							CString strAdjustmentReasonCode = "";

							if(!rs->eof) {

								pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);
								
								//should be equivalent to pCharge->strServiceID
								pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

								// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
								_variant_t varNoteIcon;
								if(!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
									//load from the datalist's icon for no notes
									varNoteIcon = (LPCTSTR)"BITMAP:FILE";
								}
								else {
									//load our icon for having notes
									varNoteIcon = (long)m_hNotes;
								}
								pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

								// (j.jones 2011-04-25 11:12) - PLID 43406 - handle NULLs, just incase
								cyChargeAmt = AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
								pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));

								// (j.jones 2011-03-18 09:32) - PLID 42157 - added FeeSchedAllowable, which multiplies by
								// quantity and modifiers
								pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, rs->Fields->Item["FeeSchedAllowableQty"]->Value);
							}
							else {
								pRow->PutValue(COLUMN_CHARGE_DATE, g_cvarNull);
								pRow->PutValue(COLUMN_CPT_CODE, g_cvarNull);
								// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
								pRow->PutValue(COLUMN_NOTE_ICON, g_cvarNull);
								pRow->PutValue(COLUMN_CHARGE_AMOUNT, g_cvarNull);
								pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);
							}
							rs->Close();

							// (j.jones 2011-09-28 14:03) - PLID 45486 - If we have invalid payment totals,
							// just color the line gray for now, we won't show a column for this.
							// This color would override the reversal color.
							if(m_EOBParser.HasInvalidPayTotals(pClaim)) {	

								bWarnInvalidPayTotals = TRUE;
								pRow->PutForeColor(RGB(127, 127, 127));
							}

							pRow->PutValue(COLUMN_PAYMENT, _variant_t(cyPaymentAmt));

							// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
							// (j.jones 2016-04-19 13:14) - NX-100161 - old column removed, now this is the total of positive adjustments only,
							// but because this charge was not in the EOB, it's always zero
							pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

							pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

							// (j.jones 2011-03-18 09:32) - PLID 42157 - added EOB allowable, but it is
							// always null here because this charge doesn't exist on the EOB
							pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);

							pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _variant_t(_bstr_t(strAdjustmentGroupCode)));

							pRow->PutValue(COLUMN_ADJ_REASON_CODE, _variant_t(_bstr_t(strAdjustmentReasonCode)));

							pRow->PutValue(COLUMN_ADJ_REASON, _variant_t(_bstr_t(strAdjustmentDescription)));
							
							pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(cyPatientAmt));

							//get the total of current patient applies made on this charge
							COleCurrency cyPatApplies = COleCurrency(0,0);
							COleCurrency cyPatientResp = GetChargePatientResp(nChargeID);
							COleCurrency cyPatientBalance = GetChargePatientBalance(nChargeID, pClaim->nPatientID);
							cyPatApplies = cyPatientResp - cyPatientBalance;
							pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));					

							//calculate the anticipated patient balance after this EOB is applied
							COleCurrency cyPatBalance = COleCurrency(0,0);
							//expected patient resp - existing patient applies
							cyPatBalance = (cyPatientResp < cyPatientAmt ? cyPatientAmt : cyPatientResp) - cyPatApplies;
							pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

							//calculate the anticipated insurance balance after this EOB is applied
							COleCurrency cyInsBalance = COleCurrency(0,0);
							cyInsBalance = cyChargeAmt - cyPatientAmt - cyPaymentAmt /*- cyAdjustmentAmt (no adjustments in this code block)*/;
							pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

							// (j.jones 2011-02-09 12:06) - PLID 42391 - auto-skip if a reversal
							// (j.jones 2011-09-28 14:03) - PLID 45486 - auto-skip if we have invalid payment totals
							// (j.jones 2012-04-18 15:59) - PLID 35306 - we support posting reversals now, so we won't auto-skip those anymore
							pRow->PutValue(COLUMN_SKIPPED, m_EOBParser.HasInvalidPayTotals(pClaim) ? g_cvarTrue : g_cvarFalse);

							// (j.jones 2008-11-17 15:23) - PLID 32045 - now we track the claim & charge pointers in the datalist
							pRow->PutValue(COLUMN_CLAIM_PTR, (long)pClaim);
							pRow->PutValue(COLUMN_CHARGE_PTR, g_cvarNull);	//there is no tracked charge in this branch of code
							// (s.tullis 2016-04-15 16:00) - NX-100211
							m_EOBList->AddRowAtEnd(pRow,NULL);

							rsCharges->MoveNext();

							// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
							// function, as we may have disabled this ability
							PeekAndPump_EOBDlg();
						}
						rsCharges->Close();
					}

					m_progress.StepIt();

					// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
					// function, as we may have disabled this ability
					PeekAndPump_EOBDlg();
				}
			}

			m_progress.SetPos(0);
			m_progressStatus.SetWindowText("Import complete.");
			EnableButtons(TRUE);

			// (j.jones 2010-01-27 11:48) - PLID 36998 - if we had any invalid claim files, warn now
			if(arystrInvalidClaims.GetSize() > 0) {
				//warn the first 10 invalid claims
				CString strClaimWarn = "The EOB did not report any services for the following claim(s):\r\n\r\n";
				for(int i=0; i<arystrInvalidClaims.GetSize() && i<10;i++) {
					strClaimWarn += arystrInvalidClaims.GetAt(i);
				}
				if(arystrInvalidClaims.GetSize() > 10) {
					strClaimWarn += "<More...>\r\n";
				}
				strClaimWarn += "\r\nYou must contact your clearinghouse to receive a corrected EOB.\r\n"
					"No posting will be made for the listed claim(s) without a corrected EOB.";
				MessageBox(strClaimWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}

			//warn the user if data is missing
			CString strWarning;
			if(bWarnMissingInsuredParties) {
				strWarning = "Practice could not match up an insured party with at least one patient on this EOB.\n"
					"The patient(s) with no matching insured party have been highlighted in red.\n"
					"Possible reasons for an insured party to not be found are:\n"
					"     - The Insurance Company name does not match the name of the Insurance Company on this EOB.\n"
					"     - The Insurance Company Payer ID does not match the Payer ID of the Insurance Company on this EOB.\n"
					"     - The policy number returned on this EOB does not match either the policy or group number for the patient.\n"
					"     - The Insurance Company is not in the same HCFA Group as the Insurance Company on this EOB.\n";
			}

			if(bWarnMissingBillIDs) {
				if(!strWarning.IsEmpty()) {
					strWarning += "\nIn addition, ";
				}
				strWarning += "Practice could not match up a bill with at least one patient on this EOB.\n"
					"The patient(s) with no matching bill have been highlighted in red, and will have no charge amount listed.\n"
					"Possible reasons for a bill to not be found are:\n"
					"     - The Bill date does not match the date of the claim on this EOB.\n"
					"     - The Bill total does not match the claim total on this EOB.\n"
					"     - The Service Codes returned on this EOB do not match the Service Codes on any bill for the patient.\n";
			}

			if(bWarnMissingInsuredParties || bWarnMissingBillIDs) {
				strWarning += "\nPractice will not be able to apply to the highlighted patient(s) until these mismatches are resolved.";

				MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}

			// (j.jones 2011-02-09 11:38) - PLID 42391 - if there are reversals, warn separately
			// (j.jones 2012-04-19 15:55) - PLID 35306 - we don't warn anymore for all reversals, but we will warn
			// if we can't process a reversal due to being unable to find the payments/adjustments to undo
			if(bHasUnpostableReversals) {
				// (s.tullis 2016-04-15 16:00) - NX-100211
				//first force all the reversals and their siblings to be skipped
				for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow(); pRow != nullptr; pRow = pRow->GetNextRow()) {
					EOBClaimInfo *pClaim = NULL;
					

					_variant_t var = pRow->GetValue(COLUMN_CLAIM_PTR);
					if(var.vt == VT_I4) {
						pClaim = (EOBClaimInfo*)VarLong(var);
					}

					EOBLineItemInfo *pCharge = NULL;				
					var = pRow->GetValue(COLUMN_CHARGE_PTR);
					if(var.vt == VT_I4) {
						pCharge = (EOBLineItemInfo*)VarLong(var);
					}

					// (j.jones 2012-09-06 14:02) - PLID 52500 - For IsUnpostableReversalCharge, I removed a
					// check for pCharge and pClaim to be non-null, because IsUnpostableReversalCharge already
					// returns false if they are.
					// Then I added a check for HasUnpostableReversalSibling, only if pClaim exists and is
					// a reversal.
					long nChargeID = -1;
					if(pCharge) {
						nChargeID = pCharge->nChargeID;
					}
					// (s.tullis 2016-04-15 16:00) - NX-100211
					if(IsUnpostableReversalCharge(pCharge, pClaim)
						|| (pClaim != NULL && (pClaim->bIsReversedClaim || pClaim->bIsRepostedClaim) && HasUnpostableReversalSibling(pRow, nChargeID, pClaim, pCharge))) {
						//mark this charge skipped
						pRow->PutValue(COLUMN_SKIPPED, g_cvarTrue);
						pRow->PutForeColor(RGB(192,0,0));
						//toggle its reversed sibling as skipped as well, and color it red
						ToggleSiblingRowSkipped(pRow, TRUE, TRUE);
					}
					
				}

				//now warn
				strWarning = "This EOB has reported at least one claim that has a status of \"Reversal of Previous Payment\", "
					"but the payment and adjustment(s) to reverse could not be found on the patient's account.\n\n"
					"The reversal and any corresponding new posting will be skipped, and will need to be manually addressed after the EOB posting is complete. "
					"These claims will be included in the EOB Warnings list at the end of posting.";

				MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);

			}

			// (j.jones 2011-09-28 14:03) - PLID 45486 - if we have invalid payment totals, warn
			if(bWarnInvalidPayTotals) {
				
				strWarning = "This EOB has reported at least one claim where the claim payment field does not match the total amount applied to the claim's charges.\n\n"
					"These claims will be skipped when posting, and will need to be manually addressed after the EOB posting is complete. "
					"These claims will be included in the EOB Warnings list at the end of posting.";

				MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}

			// (j.jones 2008-11-25 16:32) - PLID 32133 - CheckWarnPaymentOverage will report to the user how much of the payment
			// may be converted to adjustments. This has to be called after the EOB is loaded onto the screen because claim level
			// adjustments and other changes may alter the payment amounts that were parsed from the EOB.
			// (j.jones 2012-02-13 10:34) - PLID 48084 - this now just warns of paying past zero, they would be payments now, not adjustments
			m_EOBParser.CheckWarnPaymentOverage();

			// (j.jones 2016-04-14 11:08) - NX-100184 - I streamlined the way we load the
			// insurance dropdown to simply be the selected HCFAGroupID, which might be -1,
			// and any companies selected on patients, which may or may not be in that HCFA
			// group.
			{
				m_InsuranceCombo->Clear();
								
				//get the HCFAGroupIDs & likely insurance company IDs from all EOBs
				std::set<long> aryHCFAGroupIDs;
				std::set<long> aryLikelyInsCoIDs;
				CString strFirstPayerID;
				for (int i = 0; i < m_EOBParser.m_arypEOBInfo.GetSize(); i++) {
					EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[i];
					if (pEOB) {
						aryHCFAGroupIDs.insert(pEOB->nHCFAGroupID);
						aryLikelyInsCoIDs.insert(pEOB->nLikelyInsuranceCoID);

						if (strFirstPayerID.IsEmpty()) {
							strFirstPayerID = pEOB->strPayerID;
							strFirstPayerID.Trim();
						}
					}
				}
				//ensure aryHCFAGroupIDs is not empty, aryLikelyInsCoIDs is fine
				if (aryHCFAGroupIDs.size() == 0) {
					aryHCFAGroupIDs.insert(-1);
				}

				//get the insurance company IDs from all patients
				std::set<long> aryInsCoIDs;
				// (s.tullis 2016-04-15 16:00) - NX-100211
				for(NXDATALIST2Lib::IRowSettingsPtr pTraversRowPtr = m_EOBList->GetFirstRow();pTraversRowPtr != nullptr;pTraversRowPtr = pTraversRowPtr->GetNextRow()) {
					long nInsuranceCoID = VarLong(pTraversRowPtr->GetValue(COLUMN_INS_CO_ID), -1);
					aryInsCoIDs.insert(nInsuranceCoID);
				}
				//ensure this is not empty
				if (aryInsCoIDs.size() == 0) {
					aryInsCoIDs.insert(-1);
				}

				CSqlFragment sql("Archived = 0 AND (InsuranceCoT.HCFASetupGroupID IN ({INTSET}) OR InsuranceCoT.PersonID IN ({INTSET}))",
					aryHCFAGroupIDs, aryInsCoIDs);

				m_strCurInsuranceComboFilter = sql.Flatten();
				m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
				m_InsuranceCombo->Requery();
				
				// (j.jones 2016-04-22 16:06) - NX-100184 - find all companies that use this payer ID
				std::set<long> aryMatchedPayerInsCoIDs;
				{
					_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID AS InsCoID "
						"FROM InsuranceCoT "
						"LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
						"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID "
						"LEFT JOIN EbillingInsCoIDs LocationPayerIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = LocationPayerIDs.ID "
						"WHERE (EbillingInsCoIDs.EbillingID = {STRING} OR LocationPayerIDs.EbillingID = {STRING}) "
						"GROUP BY InsuranceCoT.PersonID, InsuranceCoT.Name "
						"ORDER BY InsuranceCoT.Name",
						strFirstPayerID, strFirstPayerID);
					while (!rs->eof) {
						aryMatchedPayerInsCoIDs.insert(VarLong(rs->Fields->Item["InsCoID"]->Value));
						rs->MoveNext();
					}
					rs->Close();
				}

				long nCurSel = -1;
				if (aryLikelyInsCoIDs.size() > 0 && aryMatchedPayerInsCoIDs.size() > 0) {
					for each (long nInsCoID1 in aryMatchedPayerInsCoIDs)
					{
						if (nCurSel == -1) {
							for each (long nInsCoID2 in aryLikelyInsCoIDs)
							{
								if (nInsCoID1 == nInsCoID2 && nCurSel == -1 && nInsCoID1 != -1) {
									//perfect match, a likely company has the right payer ID
									nCurSel = m_InsuranceCombo->SetSelByColumn(iccID, nInsCoID1);
									break;
								}
							}
						}
					}
				}

				//still no match? match by payer ID only
				if (nCurSel == -1 && aryMatchedPayerInsCoIDs.size() > 0) {
					for each (long nInsCoID in aryMatchedPayerInsCoIDs)
					{
						if (nCurSel == -1 && nInsCoID != -1) {
							nCurSel = m_InsuranceCombo->SetSelByColumn(iccID, nInsCoID);
						}
					}
				}

				//still no match? match by likely ID only
				if (nCurSel == -1 && aryLikelyInsCoIDs.size() > 0) {
					for each (long nInsCoID in aryLikelyInsCoIDs)
					{
						if (nCurSel == -1 && nInsCoID != -1) {
							nCurSel = m_InsuranceCombo->SetSelByColumn(iccID, nInsCoID);
						}
					}
				}

				if (nCurSel != -1) {
					OnSelChosenEobInsuranceCombo(nCurSel);
				}

				// (j.jones 2008-07-11 16:38) - PLID 28756 - now try to set our default category and description
				TrySetDefaultInsuranceDescriptions();
			}

			// (j.jones 2016-04-14 11:39) - NX-100184 - this function is misleading,
			// it actually just warns if any patients have no insured party or have
			// an insured party that mismatches the EOB's HCFA Group, and as such
			// should always be called
			CalcWarnNoInsuranceCompanyChosen();

			// (j.jones 2010-02-09 10:23) - PLID 37174 - check the EOB list
			// (j.jones 2010-02-09 10:43) - PLID 37254 - also check if they filtered out claim providers
			if(m_EOBList->GetRowCount() == 0 && (m_EOBParser.m_arystrEOBFilteredIDs.GetSize() > 0 || m_EOBParser.m_arystrClaimFilteredIDs.GetSize() > 0)) {
				MessageBox("No claims were included in the imported EOB file. "
					"Please verify that your filter settings in the 'Configure EOB Import Filters' screen are accurate.",
					"Practice", MB_ICONINFORMATION|MB_OK);
			}

			return;
		}

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Ready to import.");
		EnableButtons(TRUE);
		return;

	}NxCatchAll("Error importing electronic remittance file.");

	// (j.jones 2010-02-05 13:08) - PLID 32184 - added more logging
	Log("OnBtnImport835File() - Exception Occurred");

	m_progress.SetPos(0);
	m_progressStatus.SetWindowText("Error importing remittance file.");
	EnableButtons(TRUE);
	return;
}

void CEOBDlg::OnOK() 
{
	try {

		//remember the last setting for the claim batching
		SetRemotePropertyInt("DefEOBBatchShiftedClaims", IsDlgButtonChecked(IDC_CHECK_BATCH_CLAIMS) ? 1 : 0, 0, "<None>");

		if(!TryProcessEOB())
			return;

		// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
		m_EOBParser.ClearAllEOBs();
		m_OHIPParser.ClearEOB();
		//TES 9/15/2014 - PLID 62580 - Added Alberta support
		m_AlbertaParser.ClearAllEOBs();

		// (j.jones 2012-10-12 10:20) - PLID 53149 - restored OnOK, OnDestroy will tell MainFrame
		// to release its pointer for this dialog
		CNxDialog::OnOK();
		DestroyWindow();

	}NxCatchAll(__FUNCTION__);
}

void CEOBDlg::OnCancel() 
{
	try {

		if(m_bIsLoading)
			return;

		if(m_EOBList->GetRowCount() > 0) {
			if(IDNO == MessageBox("You have already imported a remittance file. Cancelling will abort this process and will not post any data.\n\n"
				"Are you SURE you wish to cancel?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}
		}

		// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
		m_EOBParser.ClearAllEOBs();
		m_OHIPParser.ClearEOB();
		//TES 9/15/2014 - PLID 62580 - Added Alberta support
		m_AlbertaParser.ClearAllEOBs();

		// (j.jones 2012-10-12 10:20) - PLID 53149 - restored OnCancel, OnDestroy will tell MainFrame
		// to release its pointer for this dialog
		CNxDialog::OnCancel();
		DestroyWindow();

	}NxCatchAll(__FUNCTION__);
}

class ChargeInfo;

// (j.jones 2013-03-18 16:00) - PLID 55570 - added ShiftPatientPaymentInfo,
// which will be used in an array in ChargeInfo to support shifting this
// charge's applied patient payments to multiple other charges, in partial amounts
struct ShiftPatientPaymentInfo
{
	//amount of patient applies to move
	COleCurrency cyPatPaysToShift;
	//charge to re-apply this specified amount to
	ChargeInfo *pShiftPatPaysToCharge;
};

// (j.jones 2013-03-18 16:04) - PLID 55570 - renamed to ChargeInfo, from ChargeStruct,
// and turned into a class that handles deleting its child objects
class ChargeInfo {
public:
	long ID;
	long PatientID;
	long InsuredPartyID;
	// (j.jones 2010-02-08 12:22) - PLID 37182 - added nRespTypeID
	long nRespTypeID;
	// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
	BOOL bSubmitAsPrimary;
	COleCurrency cyCharge;
	COleCurrency cyTotalPays;	//the total payments associated with this charge
	COleCurrency cyPaysToApply;	//the payment amount that we want to apply to this charge, could be less if negative adjustments are present
	// (j.jones 2016-04-19 09:49) - NX-100161 - renamed to cyPositiveAdjustments,
	// because this is only used for detecting if we have enough resp on the charge,
	// and only positive adjustments are applied to charges
	COleCurrency cyPositiveAdjustments;
	// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from cyPatResp to cyOtherResp, because
	// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
	COleCurrency cyOtherResp;
	//BOOL bDuplicate;	// (j.jones 2009-04-07 15:03) - PLID 33862 - removed
	// (j.jones 2009-10-05 13:40) - PLID 21304 - added fields necessary for shifting patient payments
	COleCurrency cyPatPays;
	long nBillID;
	COleDateTime dtChargeDate;
	// (j.jones 2013-03-18 16:00) - PLID 55570 - added an array of ShiftPatientPaymentInfo objects,
	// which will support shifting this charge's applied patient payments to multiple other charges, in partial amounts
	CArray<ShiftPatientPaymentInfo*, ShiftPatientPaymentInfo*> arypShiftPatientPaymentInfo;
	CString strServiceCode;
	// (j.jones 2011-03-21 11:12) - PLID 42099 - track a readable list of adjustment reasons
	CString strAdjReasons;
	// (j.jones 2011-06-02 09:51) - PLID 43931 - added pEOBLineItemInfo
	EOBLineItemInfo *pEOBLineItemInfo;
	OHIPEOBLineItemInfo *pOHIPEOBLineItemInfo;
	//TES 9/15/2014 - PLID 62777 - Added Alberta support
	AlbertaAssessments::ChargeInfoPtr pAlbertaCharge;
	// (j.jones 2012-04-24 17:15) - PLID 35206 - added cyReversedAmt, which tracks the total amount to be reversed on this charge
	COleCurrency cyReversedAmt;

	ChargeInfo() {
		ID = -1;
		PatientID = -1;
		InsuredPartyID = -1;
		nRespTypeID = -1;
		bSubmitAsPrimary = FALSE;
		cyCharge = COleCurrency(0,0);
		cyTotalPays = COleCurrency(0,0);
		cyPaysToApply = COleCurrency(0,0);
		cyPositiveAdjustments = COleCurrency(0,0);
		cyOtherResp = COleCurrency(0,0);
		cyPatPays = COleCurrency(0,0);
		nBillID = -1;
		dtChargeDate = g_cdtInvalid;
		pEOBLineItemInfo = NULL;
		pOHIPEOBLineItemInfo = NULL;
		pAlbertaCharge = NULL;
		cyReversedAmt = COleCurrency(0,0);
	}

	~ChargeInfo() {
		// (j.jones 2013-03-18 16:03) - PLID 55570 - clear our ShiftPatientPaymentInfo array
		for(int i=arypShiftPatientPaymentInfo.GetSize()-1;i>=0;i--) {
			ShiftPatientPaymentInfo *pInfo = (ShiftPatientPaymentInfo*)arypShiftPatientPaymentInfo.GetAt(i);
			//the ShiftPatientPaymentInfo object has a pointer to another charge, but it is not the owner,
			//and is not responsible for ensuring that charge pointer is deleted
			delete pInfo;
			pInfo = NULL;
		}
	}
};

// (j.jones 2008-06-18 13:53) - PLID 21921 - renamed the old ProcessEOB function into TryProcessEOB,
BOOL CEOBDlg::TryProcessEOB() {

	//process our payment and adjustment

	if(m_EOBList->GetRowCount() == 0) {
		MessageBox("You have not imported a remittance file that applies to any existing bills.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	if(m_InsuranceCombo->CurSel == -1) {
		MessageBox("You must select an insurance company from the list before posting.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	if(m_ProviderCombo->CurSel == -1) {
		MessageBox("You must select a provider from the list before posting.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	if(m_LocationCombo->CurSel == -1) {
		MessageBox("You must select a location from the list before posting.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}
	// (s.tullis 2016-04-25 14:41) - NX-100185
	if (!EnsureNoInvalidInsuredParties())
	{
		return FALSE;
	}
	//warn the user of the sheer amount of changes we are about to perform
	if(IDNO == MessageBox("Processing this EOB will:\n"
		"    - Create a Batch Payment for the amount of this check.\n"
		"    - Auto-apply the Batch Payment in the amount of the 'Ins. Payment' column for each charge.\n"
		"    - Auto-create and apply adjustments in the amount of the 'Adjustment' column for each charge.\n"
		"    - Transfer any patient responsibility to match the amount in the 'Pat. Resp.' column for each charge.\n\n"
		"This process is irreversible without manually editing the details of each patient account.\n\n"
		"Are you sure you are ready to process this EOB?","Practice",MB_YESNO|MB_ICONQUESTION)) {
		return FALSE;
	}

	CWaitCursor pWait;

	EnableButtons(FALSE);

	try {

		//Step 0. Before we begin ANY processing, we have to first ensure we will be able to apply our payments.
		//We must check for:
		//	1. Whether or not insurance payments have already been made
		//	2. Whether or not we need to move responsibility to insurance
		//	3. Whether or not patient payments have been made that will interfere with 2.
		//Number 1 can happen if they try to process the same EOB twice.
		//Number 2 can happen if they keep the responsibility as patient until insurance pays.
		//Number 3 can happen if they do #2 and make the patient pay the full amount first.

		m_progressStatus.SetWindowText("Processing...");

		m_bWarningsCreated = FALSE;

		long nInsuranceCompanyID = VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(), iccID));
		CString strInsuranceCompanyName = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(), iccName));
		long nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0));
		long nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0));

		// (j.jones 2008-02-20 11:28) - PLID 29007 - store the selected category IDs
		long nPaymentCategoryID = 0;
		long nAdjustmentCategoryID = 0;

		NXDATALIST2Lib::IRowSettingsPtr pPayRow = m_PayCatCombo->GetCurSel();
		if(pPayRow) {
			nPaymentCategoryID = VarLong(pPayRow->GetValue(pccID), 0);
		}

		NXDATALIST2Lib::IRowSettingsPtr pAdjRow = m_AdjCatCombo->GetCurSel();
		if(pAdjRow) {
			nAdjustmentCategoryID = VarLong(pAdjRow->GetValue(accID), 0);
		}

		// (j.jones 2008-02-20 11:34) - PLID 29007 - use the descriptions from this screen
		CString strPayDescription;
		GetDlgItemText(IDC_PAY_DESC, strPayDescription);
		strPayDescription.TrimLeft();
		strPayDescription.TrimRight();
		if(strPayDescription.IsEmpty()) {
			strPayDescription = "(No description)";
		}
		CString strAdjDescription;
		GetDlgItemText(IDC_ADJ_DESC, strAdjDescription);
		strAdjDescription.TrimLeft();
		strAdjDescription.TrimRight();
		if(strAdjDescription.IsEmpty()) {
			strAdjDescription = "(No description)";
		}

		//verify responsibilities for each claim
		if(!CheckEnsureResponsibilities()) {
			m_progress.SetPos(0);
			m_progressStatus.SetWindowText("");

			EnableButtons(TRUE);

			//open the warning log, if warnings were created
			if(m_bWarningsCreated) {
				if(OpenWarningLog()) {
					MessageBox("A log of the warnings that occurred has been created and automatically opened for your review.", "Practice", MB_ICONINFORMATION|MB_OK);
				}
			}

			return FALSE;
		}

		//Now we can process our EOB safely.

		//Step 1. Make a batch payment for the amount in question

		m_progress.SetPos(0);
		m_progress.SetRange(0,(short)m_EOBList->GetRowCount());
		m_progress.SetStep(1);

		m_progressStatus.SetWindowText("Creating batch payment...");

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_EOBDlg();

		long nEOBID = NewNumber("ERemittanceHistoryT","EOBID");

		CString strFileName;
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		switch (m_ERemitType)
		{
			case ertOhip:
			{
			//OHIP E-Remit

			strFileName = m_OHIPParser.m_strFileName;
			// (b.spivey, October 9th, 2014) PLID 62701 - get the stored file location. 
			CString strParsedFile = m_OHIPParser.GetStoredParsedFilePath();

			for(int a=0;a<m_OHIPParser.m_paryOHIPEOBInfo.GetSize();a++) {

				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(a);

				// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate,
				// which default to the check date for this EOB
				COleDateTime dtPayDate = pEOB->dtPaymentDate;
				if(dtPayDate.GetStatus() == COleDateTime::invalid) {
					dtPayDate = COleDateTime::GetCurrentTime();
				}
				COleDateTime dtAdjDate = dtPayDate;

				//now, if they entered an override date, use that date
				if(m_checkEnablePayDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtPayDate.GetValue(), dtPayDate);
					if(dt.GetStatus() != COleDateTime::invalid) {
						dtPayDate = dt;
					}
				}

				if(m_checkEnableAdjDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtAdjDate.GetValue(), dtAdjDate);
					if(dt.GetStatus() != COleDateTime::invalid) {
						dtAdjDate = dt;
					}
				}

				// (j.jones 2008-06-18 13:53) - PLID 21921 - split the ProcessEOB() functionality into its own sub-function
				// (j.jones 2009-09-25 11:03) - PLID 34453 - added parameters for the EOB pointers				
				ProcessEOB(strFileName, nEOBID, pEOB->nIndex, nInsuranceCompanyID, strInsuranceCompanyName,
					pEOB->cyTotalPaymentAmt, strPayDescription, nPaymentCategoryID, nLocationID, nProviderID,
					nAdjustmentCategoryID, strAdjDescription, pEOB->strCheckNumber, "", "", "",
					dtPayDate, dtAdjDate,
					NULL, pEOB, strParsedFile);
			}
			break;
		}
			case ertAnsi:
			{
			
			//normal ANSI E-Remit

			strFileName = m_EOBParser.m_strFileName;
			// (b.spivey, October 9th, 2014) PLID 62701 - get the stored file location. 
			CString strParsedFile = m_EOBParser.GetStoredParsedFilePath(); 

			for(int a=0;a<m_EOBParser.m_arypEOBInfo.GetSize();a++) {

				EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];

				// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate,
				// which default to the check date for this EOB
				COleDateTime dtPayDate = pEOB->dtCheckDate;
				if(dtPayDate.GetStatus() == COleDateTime::invalid) {
					dtPayDate = COleDateTime::GetCurrentTime();
				}
				COleDateTime dtAdjDate = dtPayDate;

				//now, if they entered an override date, use that date
				if(m_checkEnablePayDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtPayDate.GetValue(), dtPayDate);
					if(dt.GetStatus() != COleDateTime::invalid) {
						dtPayDate = dt;
					}
				}

				if(m_checkEnableAdjDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtAdjDate.GetValue(), dtAdjDate);
					if(dt.GetStatus() != COleDateTime::invalid) {
						dtAdjDate = dt;
					}
				}

				// (j.jones 2008-06-18 13:53) - PLID 21921 - split the ProcessEOB() functionality into its own sub-function
				// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate
				ProcessEOB(m_EOBParser.m_strFileName, nEOBID, pEOB->nIndex, nInsuranceCompanyID, strInsuranceCompanyName,
					pEOB->cyTotalPaymentAmt, strPayDescription, nPaymentCategoryID, nLocationID, nProviderID,
					nAdjustmentCategoryID, strAdjDescription, pEOB->strCheckNumber, pEOB->strPayBankRoutingNo,
					pEOB->strPayBankName, pEOB->strPayAccount, dtPayDate, dtAdjDate,
					pEOB, NULL, strParsedFile);
			}
		}	
			break;
			//TES 7/30/2014 - PLID 62580 - Added support for Alberta Assessment files
			case ertAlberta:
			{
				//Alberta E-Remit
				//TES 9/15/2014 - PLID 62777 - Added Alberta support. We only ever have one EOB for Alberta, so there's no loop here.

				strFileName = m_AlbertaParser.m_strFileName;
				// (b.spivey, October 9th, 2014) PLID 62701 - get the stored file location. 
				CString strParsedFile = m_AlbertaParser.GetStoredParsedFilePath(); 

				// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate,
				// which default to the check date for this EOB
				COleDateTime dtPayDate = m_AlbertaParser.m_dtFirstExpectedPaymentDate;
				if (dtPayDate.GetStatus() == COleDateTime::invalid) {
					dtPayDate = COleDateTime::GetCurrentTime();
				}
				COleDateTime dtAdjDate = dtPayDate;

				//now, if they entered an override date, use that date
				if (m_checkEnablePayDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtPayDate.GetValue(), dtPayDate);
					if (dt.GetStatus() != COleDateTime::invalid) {
						dtPayDate = dt;
					}
				}

				if (m_checkEnableAdjDate.GetCheck()) {
					COleDateTime dt = VarDateTime(m_dtAdjDate.GetValue(), dtAdjDate);
					if (dt.GetStatus() != COleDateTime::invalid) {
						dtAdjDate = dt;
					}
				}

				// (j.jones 2008-06-18 13:53) - PLID 21921 - split the ProcessEOB() functionality into its own sub-function
				// (j.jones 2009-09-25 11:03) - PLID 34453 - added parameters for the EOB pointers				
				//TES 9/15/2014 - PLID 62777 - Passing in NULL for both pointers at the end tells it to just use m_AlbertaParser, which holds the one and only EOB
				ProcessEOB(strFileName, nEOBID, 0, nInsuranceCompanyID, strInsuranceCompanyName,
					m_AlbertaParser.m_cyTotalAssessedAmount, strPayDescription, nPaymentCategoryID, nLocationID, nProviderID,
					nAdjustmentCategoryID, strAdjDescription, "", "", "", "",
					dtPayDate, dtAdjDate,
					NULL, NULL, strParsedFile);
			}
			break;
		}

		//Step 4. Shift balances (if needed)
		//this has to wait until the end, to handle the case where there are multiple payments for the charge,
		//we don't want to shift the balances until all payments have been applied
		
		// (j.jones 2013-07-09 15:56) - PLID 55573 - Renamed to reflect that this will now will also verify
		// balances are what we expected them to be. Even if shifting is disabled, this function needs to be called.
		CheckShiftAndVerifyBalances();

		//Step 5. Try to unbatch $0.00 balance claims.
		// (j.jones 2012-08-23 12:44) - PLID 42438 - if any claims now have a $0.00 balance,
		// this function will unbatch them if they are still batched (and not warn about it)
		CheckUnbatchClaims();

		// Step 6. Unbatch any claims that had a Remark Code of MA18, or if the primary
		// insurance is configured to not batch to secondary
		// (j.dinatale 2012-11-06 11:01) - PLID 50792 - need to handle MA18
		// (j.jones 2013-07-18 11:26) - PLID 57616 - renamed to be generic
        // (a.levy 2013-11-17 11:04) - PLID - 59546 - No need to unbatchCrossoverClaims for OHIP
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType, we don't need this for Alberta
		if (m_ERemitType == ertAnsi) { 
            UnbatchCrossoverClaims(); 
        }

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Posting complete.");

		CString strSuccess = "This EOB has successfully been posted to each of the patients' accounts.";

		if(m_bWarningsCreated) {
			if(OpenWarningLog(nEOBID)) {
				strSuccess += " A log of the warnings you received has been created and automatically opened for your review.";				
			}
		}

		strSuccess += "\n\nWould you like to preview a copy of this EOB prior to closing?";

		if(IDYES == MessageBox(strSuccess,"Practice",MB_ICONINFORMATION|MB_YESNO)) {
			m_progressStatus.SetWindowText("Posting complete - Previewing Electronic Remittance Report...");
			Print(nEOBID);
			m_progressStatus.SetWindowText("Posting complete.");
		}

		// (j.jones 2011-03-16 10:21) - PLID 21559 - detect if any of our payments were under the allowed amount
		m_progressStatus.SetWindowText("Posting complete - Checking for payments under the allowed amount...");
		CheckForPaymentsUnderAllowedAmount(nEOBID);
		m_progressStatus.SetWindowText("Posting complete.");

		// (j.jones 2006-12-19 15:18) - PLID 23913 - follow preferences to decide how to
		// handle the now-imported EOB file
		long nEOBFileManipulation = GetRemotePropertyInt("EOBFileManipulation",1,0,"<None>",true);
		switch(nEOBFileManipulation) {
			case 0:	//do nothing
				break;
			case 1: { //rename file

				//add _IMPORTED_<date>
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				CString strNewName;
				CString strOldName = strFileName;
				CString strExtension;
				long nExt = strOldName.ReverseFind('.');
				if(nExt != -1) {
					strExtension = strOldName.Right(strOldName.GetLength() - nExt);
					strOldName = strOldName.Left(nExt);
				}
				strNewName.Format("%s_IMPORTED_%02li-%02li-%04li%s", strOldName,
					dtNow.GetMonth(), dtNow.GetDay(), dtNow.GetYear(), strExtension);

				//ensure the name we're trying to rename to doesn't exist
				long nCount = 0;
				while(DoesExist(strNewName)) {
					nCount++;
					strNewName.Format("%s_IMPORTED_%02li-%02li-%04li_%li%s", strOldName,
					dtNow.GetMonth(), dtNow.GetDay(), dtNow.GetYear(), nCount, strExtension);
				}

				CFileStatus fsStatus;
				if (DoesExist(strFileName)
					&& CFile::GetStatus(strFileName, fsStatus)
					&& !(fsStatus.m_attribute & readOnly)) {

					CFile::Rename(strFileName, strNewName);
					
					//log it
					Log("User '%s' renamed EOB file '%s' to '%s'.", GetCurrentUserName(), strFileName, strNewName);

					// (j.jones 2008-12-18 11:52) - PLID 32489 - if OHIP, update the OHIPReportHistoryT record to change
					// the old filename to the new one. We act like that's a primary key, it should be unique.
					//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
					if(m_ERemitType == ertOhip) {
						CString strOldFileNameOnly;
						int nSlash = strFileName.ReverseFind('\\');
						if(nSlash == -1) {
							strOldFileNameOnly = strFileName;
						}
						else {
							strOldFileNameOnly = strFileName.Right(strFileName.GetLength() - nSlash - 1);
						}
						CString strNewFileNameOnly;
						nSlash = strNewName.ReverseFind('\\');
						if(nSlash == -1) {
							strNewFileNameOnly = strNewName;
						}
						else {
							strNewFileNameOnly = strNewName.Right(strNewName.GetLength() - nSlash - 1);
						}
						ExecuteParamSql("UPDATE OHIPReportHistoryT SET FileName = {STRING} "
							"WHERE FileName = {STRING}", strNewFileNameOnly, strOldFileNameOnly);
					}
				}
				else {
					//log that we couldn't find the EOB file!
					Log("User '%s' attempted to rename the EOB file '%s' to '%s', however the source file could not be modified. (Either not found or read only.)", GetCurrentUserName(), strFileName, strNewName);
				}
				}								
				break;
			case 2: //prompt to delete

				if(IDYES == MessageBox("Would you like to delete the EOB remittance file now?.\n\n"
						  "If you choose to delete the EOB file, you will need to contact the insurance company\n"
						  "or Ebilling Clearinghouse in order to have it rebatched for download.\n"
						  "NexTech Support will be unable to reclaim the deleted file.\n\n"
						  "It is not recommended that you delete a file unless you are have been successfully\n"
						  "posting EOBs on a regular basis and are certain you will not need the file again.",
						  "Practice",MB_ICONQUESTION|MB_YESNO)) {

					//okay, they want to do it, so delete
					CFileStatus fsStatus;
					if (DoesExist(strFileName)
						&& CFile::GetStatus(strFileName, fsStatus)
						&& !(fsStatus.m_attribute & readOnly)) {

						CFile::Remove(strFileName);

						//log it
						Log("User '%s' deleted EOB file '%s'.", GetCurrentUserName(), strFileName);
					}
					else {
						//log that we couldn't find the EOB file!
						Log("User '%s' attempted to delete the EOB file '%s', however the source file could not be modified. (Either not found or read only.)", GetCurrentUserName(), strFileName);
					}
				}
				break;
			case 3: { //delete the file

				//they want to auto-delete...
				CFileStatus fsStatus;
				if (DoesExist(strFileName)
					&& CFile::GetStatus(strFileName, fsStatus)
					&& !(fsStatus.m_attribute & readOnly)) {

					CFile::Remove(strFileName);

					//log it
					Log("User '%s' auto-deleted EOB file '%s'.", GetCurrentUserName(), strFileName);
				}
				else {
					//log that we couldn't find the EOB file!
					Log("User '%s' attempted to auto-delete the EOB file '%s', however the source file could not be modified. (Either not found or read only.)", GetCurrentUserName(), strFileName);
				}
				}				
				break;
		}
		
		EnableButtons(TRUE);

		return TRUE;

	}NxCatchAll("Error processing EOB");

	if(m_bWarningsCreated) {
		// (j.jones 2011-03-21 13:24) - PLID 42917 - log that an exception occurred in the warnings
		CString strLog = "***Exception Occurred While Processing***";
		m_fileLogWarnings.Write(strLog,strLog.GetLength());
		
		// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up the log,
		// and is the only permissible way to close the file
		CloseWarningLogAndCopyToServer();
	}

	m_progress.SetPos(0);
	m_progressStatus.SetWindowText("");

	// (j.jones 2010-02-05 13:08) - PLID 32184 - added more logging
	Log("TryProcessEOB() - Exception Occurred");

	EnableButtons(TRUE);

	return FALSE;
}

// (j.jones 2008-06-18 13:53) - PLID 21921 - renamed the old ProcessEOB function into TryProcessEOB,
// and split the ProcessEOB() functionality into its own sub-function
// (j.jones 2009-06-23 17:43) - PLID 32184 - changed the return value to be void
// (j.jones 2009-07-07 15:32) - PLID 34805 - renamed the cyTotalPaymentAmt variable to cyInitialBatchPayAmt
// for clarity, since the batch payment that is created may end up with a different amount
// (j.jones 2009-09-25 11:03) - PLID 34453 - added parameters for the EOB pointers
// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate
void CEOBDlg::ProcessEOB(CString strFileName, const long nEOBID, const long nIndex, long nInsuranceCompanyID, CString strInsCoName,
						 COleCurrency cyInitialBatchPayAmt, CString strPayDescription, long nPaymentCategoryID,
						 long nLocationID, long nProviderID, long nAdjustmentCategoryID, CString strAdjDescription,
						 CString strCheckNumber, CString strPayBankRoutingNo, CString strPayBankName, CString strPayAccount,
						 COleDateTime dtPayDate, COleDateTime dtAdjDate,
						 EOBInfo *pEOBInfo, OHIPEOBInfo *pOHIPEOBInfo, CString strParsedFile)
{
	CString strOriginalAmt = "NULL";

	// (j.jones 2009-09-25 11:06) - PLID 34453 - verify we were given the correct EOB pointer
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if(m_ERemitType == ertOhip) {
		ASSERT(pOHIPEOBInfo != NULL);
	}
	else if (m_ERemitType == ertAlberta) {
		//TES 9/15/2014 - PLID 62777 - For Alberta, we just use m_AlbertaParser, there are never multiple EOBs
		ASSERT(m_AlbertaParser.m_pChargeList != NULL);
	}
	else {
		ASSERT(pEOBInfo != NULL);
	}

	// (j.jones 2009-07-07 15:30) - PLID 34805 - OHIP batch payments are now auto-incremented to cover all the payments
	// in the list, whether they are going to be applied or not
	COleCurrency cyAppliedPaymentTotal = COleCurrency(0,0);
	// (s.tullis 2016-04-15 16:00) - NX-100211
	for(NXDATALIST2Lib::IRowSettingsPtr pTraverseRowPtr = m_EOBList->GetFirstRow();pTraverseRowPtr != nullptr;pTraverseRowPtr = pTraverseRowPtr->GetNextRow()) {

		if(nIndex == VarLong(pTraverseRowPtr->GetValue(COLUMN_EOB_INDEX))) {
			COleCurrency cyPayment = VarCurrency(pTraverseRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));
			cyAppliedPaymentTotal += cyPayment;
		}
	}

	COleCurrency cyActualBatchPaymentAmount = cyInitialBatchPayAmt;
	_variant_t vtOriginalAmount = g_cvarNull;
	// (j.jones 2012-10-05 08:37) - PLID 52929 - if the setting to ignore missing payments is on,
	// we will reduce the batch payment total to match the amount to be applied
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if(m_ERemitType == ertOhip &&
		((cyAppliedPaymentTotal > cyActualBatchPaymentAmount) || (m_bOHIPIgnoreMissingPatients && cyAppliedPaymentTotal < cyActualBatchPaymentAmount))) {

		CString strWarn;
		
		if(m_bOHIPIgnoreMissingPatients && cyAppliedPaymentTotal < cyActualBatchPaymentAmount) {
			// (j.jones 2012-10-05 08:37) - PLID 52929 - the user wants the batch payment reduced to match the amount to be applied
			strWarn.Format("The payment originally for %s was decreased to %s to reflect the amount applied to patients.",
				FormatCurrencyForInterface(cyInitialBatchPayAmt), FormatCurrencyForInterface(cyAppliedPaymentTotal));
		}
		else if(cyAppliedPaymentTotal > cyActualBatchPaymentAmount) {
			//the OHIP EOB states that more money should be applied than the check was actually for,
			//so we will create the batch payment with that total amount, not the initial check amount

			strWarn.Format("The payment originally for %s was increased to %s to cover the overage applied to patients.",
				FormatCurrencyForInterface(cyInitialBatchPayAmt), FormatCurrencyForInterface(cyAppliedPaymentTotal));
		}	

		//strOriginalAmount.Format("'%s'", _Q(FormatCurrencyForSql(cyInitialBatchPayAmt)));
		// (a.walling 2009-11-04 15:13) - PLID 36198 - We have to pass unquoted 'currency' strings to SQL for it to work
		// with SQL 2000; otherwise we will get an implicit conversion error. You can work around this by using CONVERT
		// within the statement, but since this can be currency or NULL, we don't necessary want to CONVERT, so we'll just
		// use it without the quotes.
		vtOriginalAmount.cyVal = cyInitialBatchPayAmt;

		// (j.jones 2009-07-01 18:13) - PLID 33856 - log that this occurred both in the EOB warning log,
		// and our own internal log
		
		//log in NxLog
		Log(strWarn);

		//add to the log the user will see
		AddWarningToLog(strWarn, "");

		//now update our actual payment amount
		cyActualBatchPaymentAmount = cyAppliedPaymentTotal;
	}

	COleCurrency cyBatchPayBalance = cyActualBatchPaymentAmount;

	long nBatchPaymentID = NewNumber("BatchPaymentsT","ID");

	
	// (j.jones 2009-06-26 11:28) - PLID 33856 - added OriginalAmount field in BatchPaymentsT,
	// to track the original amount the check was supposed to be for
	// (j.jones 2010-04-09 12:10) - PLID 31309 - use the provided date for the service date
	// (j.armen 2012-02-20 11:06) - PLID 34344 - Parameratized
	// (j.jones 2014-06-27 08:40) - PLID 62546 - added PayType, always a Medical Payment
	// (s.tullis 2014-08-14 17:26) - PLID 60686 - since where importing EOB through the EOB import dialog we need to flag this in batchpayments
	// (b.spivey, October 9th, 2014) PLID 62701 - save the parsed file path too so we can open this later. 
	ExecuteParamSql(
		R"(INSERT INTO BatchPaymentsT (ID, InsuranceCoID, Description, Amount, Date, 
										InputDate, UserID, PayCatID, Location, ProviderID, 
										CheckNo, BankRoutingNum, BankName, CheckAcctNo, OriginalAmount, 
										PayType, ERemittance, EOBFilePath) 
		VALUES ({INT}, {INT}, {STRING}, {OLECURRENCY}, {OLEDATETIME}, 
										GetDate(), {INT}, {INT}, {INT}, {INT}, 
										{STRING}, {STRING}, {STRING}, {STRING}, {VT_CY}, 
										{CONST_INT}, 1, {STRING}) )",
		nBatchPaymentID, nInsuranceCompanyID, strPayDescription, cyActualBatchPaymentAmount,
		dtPayDate, GetCurrentUserID(), nPaymentCategoryID, nLocationID, nProviderID, 
		strCheckNumber, strPayBankRoutingNo, strPayBankName, strPayAccount, vtOriginalAmount, 
		(long)eMedicalPayment, strParsedFile);
	
	// (j.jones 2009-03-05 10:13) - PLID 33076 - If OHIP, we might increase the batch payment amount,
	// and if we do, we will append the original price to the description, one time only. Since we
	// know that now, we can save a recordset later by calculating what that description would be.
	// (j.jones 2009-06-26 11:28) - PLID 33856 - we later changed this to be a field in BatchPaymentsT,
	// and no longer update the description of the batch payment
	// (j.jones 2009-07-07 15:44) - PLID 34805 - and now all of this work is done up above, during the
	// initial BatchPaymentsT creation
	/*
	BOOL bUseOriginalAmount = FALSE;
	COleCurrency cyOriginalAmount = cyTotalPaymentAmt;
	*/

	long AuditID = -1;
	AuditID = BeginNewAuditEvent();
	if(AuditID != -1) {
		// (j.jones 2011-03-21 17:04) - PLID 24273 - changed to use a unique E-Remit audit
		AuditEvent(-1, strInsCoName,AuditID,aeiBatchPaymentCreatedByERemit,nBatchPaymentID,"",strInsCoName,aepHigh,aetCreated);
	}

	// (j.jones 2009-09-25 11:08) - PLID 34453 - write our OHIP transaction messages to data, if we have any
	// (j.armen 2012-02-20 11:07) - PLID 34344 - Parameratized
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if(m_ERemitType == ertOhip) {
		CParamSqlBatch sqlBatch;
		for(int nListIndex=0; nListIndex<pOHIPEOBInfo->paryOHIPEOBAccountingTransaction.GetSize(); nListIndex++) {
			const OHIPEOBAccountingTransaction *pTxInfo = pOHIPEOBInfo->paryOHIPEOBAccountingTransaction[nListIndex];
			sqlBatch.Add("INSERT INTO BatchPaymentOHIPMessagesT (BatchPaymentID, Date, Amount, ReasonCode, Reason, Message) "
				"VALUES ({INT}, dbo.AsDateNoTime({OLEDATETIME}), {OLECURRENCY}, {STRING}, {STRING}, {STRING})",
				nBatchPaymentID, pTxInfo->dtDate, pTxInfo->cyAmount,
				pTxInfo->strReasonCode, pTxInfo->strReasonDesc, pTxInfo->strMessage);
		}

		if(!sqlBatch.IsEmpty()) {
			sqlBatch.Execute(GetRemoteData());
		}
	}

	// (j.jones 2008-11-21 10:51) - PLID 32076 - added preference to allow $0.00 adjustments
	BOOL bAllowZeroDollarAdjustments = GetRemotePropertyInt("ERemitAllowZeroDollarAdjustments", 0, 0, "<None>", true) == 1;

	// (j.jones 2012-04-24 09:56) - PLID 35306 - PostEOBLineItems will iterate through the list and post the EOB.
	// We first make one pass to post reversals, and then a second pass to post normal payments.
	PostEOBLineItems(TRUE, nEOBID, nIndex, nBatchPaymentID, cyBatchPayBalance, cyActualBatchPaymentAmount,
		bAllowZeroDollarAdjustments, dtAdjDate, nAdjustmentCategoryID, strAdjDescription);
	PostEOBLineItems(FALSE, nEOBID, nIndex, nBatchPaymentID, cyBatchPayBalance, cyActualBatchPaymentAmount,
		bAllowZeroDollarAdjustments, dtAdjDate, nAdjustmentCategoryID, strAdjDescription);

	// (j.jones 2008-12-18 11:45) - PLID 32489 - if OHIP, update the OHIPReportHistoryT record now
	// by file name - we act like that's a primary key
	//update the LastImportDate, and also update the FirstImportDate if it is NULL
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType == ertOhip) {
		CString strFileNameOnly;
		int nSlash = strFileName.ReverseFind('\\');
		if(nSlash == -1) {
			strFileNameOnly = strFileName;
		}
		else {
			strFileNameOnly = strFileName.Right(strFileName.GetLength() - nSlash - 1);
		}
		ExecuteParamSql("UPDATE OHIPReportHistoryT SET LastImportDate = GetDate(), "
			"FirstImportDate = CASE WHEN FirstImportDate Is Null THEN GetDate() ELSE FirstImportDate END "
			"WHERE FileName = {STRING}", strFileNameOnly);
	}

	// (j.jones 2008-11-25 17:29) - PLID 32133 - now create an adjustment from the overage,
	// only if we actually have an overage amount (this is unrelated to the bAllowZeroDollarAdjustments option)			
	// (j.jones 2012-02-13 10:55) - PLID 48084 - we now apply payments instead of adjustments, and intentionally
	// leave the batch payment with a negative balance
	if(cyBatchPayBalance < COleCurrency(0,0)) {
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		if (m_ERemitType == ertOhip) {
			//this used to be an assertion, but it should be an exception, because our code
			//above should be increasing the batch payment amount so there's never a negative balance
			ThrowNxException("A negative batch payment balance was found.");
		}

		//log this
		Log("EOB applied a %s batch payment with a final balance of %s.", FormatCurrencyForInterface(cyActualBatchPaymentAmount), FormatCurrencyForInterface(cyBatchPayBalance));

		CString strWarn;
		strWarn.Format("The batch payment for %s has has more payments applied than the initial payment amount, leaving a final balance of %s.", FormatCurrencyForInterface(cyActualBatchPaymentAmount), FormatCurrencyForInterface(cyBatchPayBalance));
		AddWarningToLog(strWarn,"");
	}
}

// (j.jones 2012-04-24 09:56) - PLID 35306 - Called by ProcessEOB, this will iterate through the list
// and post the EOB. It's a separate function so that it can make one pass to post reversals, and
// a second pass to post normal payments.
void CEOBDlg::PostEOBLineItems(const BOOL bOnlyPostReversedClaims, const long nEOBID, const long nIndex,
							   const long nBatchPaymentID, IN OUT COleCurrency &cyBatchPayBalance, IN OUT COleCurrency &cyActualBatchPaymentAmount,
							   const BOOL bAllowZeroDollarAdjustments, const COleDateTime dtAdjDate, const long nAdjustmentCategoryID, const CString strAdjDescription)
{
	//throw exceptions to the caller

	CString strBatchPaymentCheckNo = "", strBatchPaymentDate = "";
	BOOL bNeedLoadBatchPayInfo = TRUE;

	// (b.spivey, July 03, 2013) - PLID 56825 - Check this preference, as we may be wanting to post notes to charges even though
	//		we skipped over them because there was nothing to post but an adjustment. 
	// (b.spivey, July 30, 2013) - PLID 56825 - Default to on...
	bool bPostAdjustmentNoteToBill = 
		(GetRemotePropertyInt("ERemitPostAdjustmentNoteWhenZeroPays", 1, 0, "<None>", true) ? true : false);

	//TES 10/1/2014 - PLID 62782 - Track the highest-priority status we find for each bill ID
	CMap<long, long, AlbertaBillingStatus, AlbertaBillingStatus> mapBillStatuses;
	// (s.tullis 2016-04-15 16:00) - NX-100211
	//iterate through each charge, we will only post those specified by bOnlyPostReversedClaims
	for(NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr = m_EOBList->GetFirstRow(); pLoopRowPtr != nullptr; pLoopRowPtr = pLoopRowPtr->GetNextRow()) {

		//ignore the skipped rows, obviously!
		if(VarBool(pLoopRowPtr->GetValue(COLUMN_SKIPPED))) { 

			// (b.spivey, July 03, 2013) - PLID 56825 - We need to make sure these don't pass 
			//		a certain subset of qualifiers to post the note to the charge anyways. 
			COleCurrency cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP));

			// (j.jones 2016-04-26 14:08) - NX-100327 - the adjustment column is total adjustments, and is now
			// for UI purposes only - we need to pull the real adjustments from the EOB info
			COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
			COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
			COleCurrency cyTotalAdjAmt = COleCurrency(0, 0);
			GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjAmt);

			//if the total is zero, but the total positive is not, we must have negative adjustments balancing it out (not common)
			bool bHasNonZeroAdjustments = (cyTotalPositiveAdj > COleCurrency(0, 0) || cyTotalAdjAmt != COleCurrency(0, 0));

			COleCurrency cyPayAmt = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT));

			//TES 9/18/2014 - PLID 62777 - Alberta doesn't create adjustments
			// (j.jones 2016-04-19 13:30) - NX-100161 - negative adjustments are allowed to post notes,
			// so just post if we have adjustments of any kind
			if (!bOnlyPostReversedClaims && bPostAdjustmentNoteToBill 
				&& cyOtherResp == g_ccyZero && cyPayAmt == g_ccyZero
				&& bHasNonZeroAdjustments
				&& m_ERemitType != ertAlberta) {

				long nPatientID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_ID));
				long nChargeID = VarLong(pLoopRowPtr->GetValue(COLUMN_CHARGE_ID));

				// (j.jones 2013-09-04 11:50) - PLID 58330 - don't save a note if we don't have
				// a valid charge ID or patient ID
				if(nChargeID != -1 && nPatientID != -1) {

					EOBLineItemInfo *pCharge = NULL;
					_variant_t var = pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR);

					if(var.vt == VT_I4) {
						pCharge = (EOBLineItemInfo*)VarLong(var);
					}

					CreateBillingNoteFromAdjustmentReasons(nPatientID, nChargeID, pCharge, true);
				}
			}
			continue;
		}

		// (j.jones 2011-02-09 13:28) - PLID 42391 - ignore reversals as well, though they should
		// also be flagged as skipped
		// (j.jones 2012-04-18 16:41) - PLID 35306 - we now allow reversals, but this function now
		// posts either all reversals, or all non-reversals (as it's called twice)
		BOOL bIsReversedClaim = FALSE;
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		if(m_ERemitType != ertOhip && VarBool(pLoopRowPtr->GetValue(COLUMN_REVERSAL), FALSE)) {
			if (m_ERemitType == ertAnsi) {
			EOBClaimInfo *pClaim = NULL;
			_variant_t var = pLoopRowPtr->GetValue( COLUMN_CLAIM_PTR);
			if(var.vt == VT_I4) {
				pClaim = (EOBClaimInfo*)VarLong(var);
			}

			if(pClaim != NULL) {
				//track this for later
				bIsReversedClaim = pClaim->bIsReversedClaim;
			}

			if(bOnlyPostReversedClaims) {
				//skip anything that is not reversed

				//if pClaim is NULL, it's not a reversal
				if(pClaim == NULL || !pClaim->bIsReversedClaim) {
					continue;
				}
			}
			else {
				//skip reversed claims (but not re-posted claims)
				if(pClaim != NULL && pClaim->bIsReversedClaim) {
					continue;
				}
			}
		}
			else {
				//TES 9/26/2014 - PLID 62536 - Remember that this is reversed
				ASSERT(m_ERemitType == ertAlberta);
				_variant_t var = pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR);
				if (var.vt == VT_I4) {
					AlbertaAssessments::ChargeInfoPtr pCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
					//TES 9/30/2014 - PLID 62536 - In certain cases, a "Reversal" line won't actually reverse an existing payment
					bIsReversedClaim = (pCharge->nReversedPaymentID != -1);
					if (bOnlyPostReversedClaims) {
						if (pCharge == NULL || pCharge->nReversedPaymentID == -1) {
							continue;
						}
					}
					else {
						if (pCharge != NULL && pCharge->nReversedPaymentID != -1) {
							continue;
						}
					}
				}
			}

		}
		else if(bOnlyPostReversedClaims) {
			//this is not a reversed claim, and that's all we want, so skip it
			continue;
		}

		//make sure we're applying the right batch payment to the right charge! (for multiple batch pays)
		if(nIndex == VarLong(pLoopRowPtr->GetValue(COLUMN_EOB_INDEX))) {

			long nPatientID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_ID));
            // (a.levy 2013-11-14 15:10) - PLID - 59445 - Fix for NULL VALUE
			// (s.dhole 2013-06-24 16:20) - PLID 42275 userdefinedid
            long nPatientUserDefinedID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_USERDEFINED_ID), -1);
			CString strPatientName = VarString(pLoopRowPtr->GetValue( COLUMN_PATIENT_NAME),"");
			long nInsuredPartyID = VarLong(pLoopRowPtr->GetValue(COLUMN_INSURED_PARTY_ID));
			// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
			long nRespTypeID = VarLong(pLoopRowPtr->GetValue(COLUMN_RESP_TYPE_ID));
			// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
			BOOL bSubmitAsPrimary = VarBool(pLoopRowPtr->GetValue( COLUMN_SUBMIT_AS_PRIMARY), FALSE);
			long nBillID = VarLong(pLoopRowPtr->GetValue(COLUMN_BILL_ID));
			long nChargeID = VarLong(pLoopRowPtr->GetValue(COLUMN_CHARGE_ID));
			BOOL bDuplicate = VarLong(pLoopRowPtr->GetValue(COLUMN_DUPLICATE)) ? TRUE : FALSE;
			// (j.jones 2011-01-07 09:51) - PLID 41785 - grab the insurance company name
			CString strInsuranceCoName = VarString(pLoopRowPtr->GetValue(COLUMN_INS_CO_NAME), "");
			//TES 10/8/2014 - PLID 62581 - We'll also need the service code, date, and total
			CString strServiceCode = VarString(pLoopRowPtr->GetValue( COLUMN_CPT_CODE), "");
			_variant_t varChargeDate = pLoopRowPtr->GetValue(COLUMN_CHARGE_DATE);
			COleDateTime dtChargeDate;
			if (varChargeDate.vt == VT_DATE) {
				dtChargeDate = VarDateTime(varChargeDate);
			}
			COleCurrency cyChargeAmount = VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0));

			m_progress.StepIt();
			CString str;
			str.Format("Posting payments for patient '%s'...",GetExistingPatientName(nPatientID));
			Log(str);
			m_progressStatus.SetWindowText(str);
			
			// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
			// function, as we may have disabled this ability
			PeekAndPump_EOBDlg();

			if(nPatientID == -1) {
				Log("ProcessEOB - Skipped -1 patient ID");
				continue;
			}

			//Step 1. Shift balances as needed to ensure the patient responsibility is at least what the insurance requests.

			// (j.jones 2008-11-17 11:54) - PLID 32007 - If the insurance specified a given patient responsibility,
			// shift enough money to pat. resp. *before* trying to apply so the patient has that much resp. Keep in
			// mind if may be possible that they already have that resp. and it's already paid off, such as a copay.
			// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
			// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company".
			// However with this preference, we will still shift it directly to patient.
			COleCurrency cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP));
			
			// (j.jones 2009-03-05 14:53) - PLID 33220 - added preference to control shifting to patient responsibility by default
			// (pretty much a way to turn PLID 32007 on or off)
			// (j.jones 2010-09-02 11:01) - PLID 38787 - split the preference into primary/secondary, which default to the old value
			BOOL bShiftPatientResp = FALSE;

			// (j.jones 2010-09-02 11:13) - PLID 38787 - this needs to acknowledge the "submit as primary" setting
			BOOL bIsPrimaryIns = (nRespTypeID == 1 || bSubmitAsPrimary);

			if(bIsPrimaryIns) {
				//primary defaults to the old value, 0 if it doesn't exist
				bShiftPatientResp = GetRemotePropertyInt("ERemitPrimaryAutoShiftPatientResp", GetRemotePropertyInt("ERemitAutoShiftPatientResp", 0, 0, "<None>", false), 0, "<None>", true);
			}
			else {
				//secondary or other non-primary defaults to on
				bShiftPatientResp = GetRemotePropertyInt("ERemitSecondaryAutoShiftPatientResp", 1, 0, "<None>", true);
			}
			
			if(nChargeID != -1 && bShiftPatientResp) {
				COleCurrency cyAmtShiftedToPat = COleCurrency(0,0); //this isn't used here, but it is a parameter needed for TryShiftRespToPatient
				TryShiftRespToPatient(nPatientID, nChargeID, nInsuredPartyID, cyOtherResp, FALSE, cyAmtShiftedToPat);
			}

			//Step 2. Apply the payment to each paid charge of each patient in the list.
								
			COleCurrency cyTotalPaymentAmt = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));
			COleCurrency cyPaymentAmtToApply = cyTotalPaymentAmt;

			// (j.jones 2016-04-19 16:20) - NX-100246 - cache the new payment ID
			long nNewPaymentID = -1;
			
			// (j.jones 2016-04-26 14:08) - NX-100327 - the adjustment column is total adjustments, and is now
			// for UI purposes only - we need to pull the real adjustments from the EOB info
			COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
			COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
			COleCurrency cyTotalAdjustmentAmt = COleCurrency(0, 0);
			GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjustmentAmt);

			//if the total is zero, but the total positive is not, we must have negative adjustments balancing it out (not common)
			bool bHasNonZeroAdjustments = (cyTotalPositiveAdj > COleCurrency(0, 0) || cyTotalAdjustmentAmt != COleCurrency(0, 0));

			//reduce the payment apply amount by the total of negative adjustments
			if (cyTotalNegativeAdj < COleCurrency(0, 0)) {
				cyPaymentAmtToApply += cyTotalNegativeAdj;
			}

			// (j.jones 2012-02-13 10:55) - PLID 48084 - we now apply payments instead of adjustments, and intentionally
			// leave the batch payment with a negative balance, which means a lot of the old logic has been removed

			//reduce the cyBatchPayBalance
			if(cyBatchPayBalance >= cyTotalPaymentAmt) {
				//normal case, we have enough money to apply, so just reduce this amount
				cyBatchPayBalance -= cyTotalPaymentAmt;
			}
			else if(cyBatchPayBalance <= COleCurrency(0,0)) {

				// (j.jones 2009-03-05 10:03) - PLID 33076 - if OHIP, we will not apply overages,
				//but instead increase the batch payment amount
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip) {

					// (j.jones 2009-07-07 15:44) - PLID 34805 - now the overage is calculated at the beginning
					// of this process, not on the fly, so this should be impossible for an OHIP EOB

					ASSERT(FALSE);

					//log this
					Log("Increased Batch Payment by %s as an overage to support Patient '%s', PatientID: %li, ChargeID: %li, InsuredPartyID: %li", FormatCurrencyForInterface(cyTotalPaymentAmt), strPatientName, nPatientID, nChargeID, nInsuredPartyID);

					IncreaseBatchPaymentBalance(nBatchPaymentID, cyTotalPaymentAmt);
					cyActualBatchPaymentAmount += cyTotalPaymentAmt;
					cyBatchPayBalance = COleCurrency(0,0);	
				}
				else {
					// (j.jones 2012-02-13 11:09) - PLID 48084 - just decrement the batch payment balance to be negative
					cyBatchPayBalance -= cyTotalPaymentAmt;
				}
			}
			else if(cyBatchPayBalance < cyTotalPaymentAmt) {

				// (j.jones 2009-03-05 10:03) - PLID 33076 - if OHIP, we will not apply overages,
				//but instead increase the batch payment amount
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip) {

					// (j.jones 2009-07-07 15:44) - PLID 34805 - now the overage is calculated at the beginning
					// of this process, not on the fly, so this should be impossible for an OHIP EOB

					ASSERT(FALSE);

					//log this
					Log("Increased Batch Payment by %s as an overage to support Patient '%s', PatientID: %li, ChargeID: %li, InsuredPartyID: %li", FormatCurrencyForInterface(cyTotalPaymentAmt - cyBatchPayBalance), strPatientName, nPatientID, nChargeID, nInsuredPartyID);

					IncreaseBatchPaymentBalance(nBatchPaymentID, (cyTotalPaymentAmt - cyBatchPayBalance));
					cyActualBatchPaymentAmount += (cyTotalPaymentAmt - cyBatchPayBalance);
					cyBatchPayBalance = COleCurrency(0,0);
				}
				else {
					// (j.jones 2012-02-13 11:09) - PLID 48084 - just decrement the batch payment balance to be negative
					cyBatchPayBalance -= cyTotalPaymentAmt;
				}
			}

			// (j.jones 2012-04-20 09:31) - PLID 49847 - if the payment is negative, take it back (US only)
			//TES 7/30/2014 - PLID 62536 - Changed m_bIsOhip to m_ERemitType, Alberta does have reversals
			//TES 9/26/2014 - PLID 62536 - Alberta reversals aren't negative (they are $0, usually)
			if(m_ERemitType != ertOhip && bIsReversedClaim) {

				long nReversedPaymentID = -1;
				if (m_ERemitType == ertAnsi && cyTotalPaymentAmt < COleCurrency(0,0)) {

					EOBClaimInfo *pClaim = NULL;
					_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
					if(var.vt == VT_I4) {
						pClaim = (EOBClaimInfo*)VarLong(var);
					}
					EOBLineItemInfo *pCharge = NULL;
					var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
					if(var.vt == VT_I4) {
						pCharge = (EOBLineItemInfo*)VarLong(var);
					}

					//throw exceptions in impossible states, we shouldn't get to this point if any of the following is true,
					//as other code should have forced this line to be skipped, and apparently failed to do so
					if(pClaim == NULL || pCharge == NULL) {
						ThrowNxException("A negative payment was found with incomplete EOB information.");
					}
					else if(!pClaim->bIsReversedClaim) {
						ThrowNxException("A negative payment was found on a charge that is not flagged as a reversal.");
					}
					else if(pCharge->nReversedPaymentID == -1) {
						ThrowNxException("A negative payment was found on a charge, but no matching payment was found on the patient's account.");
					}

					nReversedPaymentID = pCharge->nReversedPaymentID;
				}
				else if(m_ERemitType == ertAlberta) {
					//TES 7/30/2014 - PLID 62536 - Support Alberta reversals
					AlbertaAssessments::ChargeInfoPtr pCharge = NULL;
					_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
					if (var.vt == VT_I4) {
						pCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
					}
					if (pCharge == NULL) {
						ThrowNxException("A reversal was found with incomplete EOB information.");
					}
					else if (pCharge->nReversedPaymentID == -1) {
						ThrowNxException("A reversal was found on a charge, but no matching payment was found on the patient's account.");
					}
					nReversedPaymentID = pCharge->nReversedPaymentID;
				}

				//it is possible, albeit unlikely, that this payment ID has already been voided,
				//if so, recursively find the newest payment ID that is not voided
				long nOldReversedPaymentID = nReversedPaymentID;
				nReversedPaymentID = FindNewestCorrectedLineItemID(nReversedPaymentID);
				if (nReversedPaymentID != nOldReversedPaymentID) {
					if (m_ERemitType == ertAnsi) {
						EOBLineItemInfo *pCharge = (EOBLineItemInfo*)VarLong(pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR));
						pCharge->nReversedPaymentID = nReversedPaymentID;
					}
					else {
						//TES 7/30/2014 - PLID 62536 - Support Alberta reversals
						AlbertaAssessments::ChargeInfoPtr pCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR)));
						pCharge->nReversedPaymentID = nReversedPaymentID;
					}
				}

				if(nReversedPaymentID == -1) {
					//cancel posting this line item
					Log("Payment ID %li had been reversed, but had previously been voided, with no corrected payment created. Reversal will be skipped.", nReversedPaymentID);
				}
				else {				
					//log this
					if(nReversedPaymentID != nOldReversedPaymentID) {
						Log("Payment ID %li has been reversed, will be voided. This is the newest corrected version of original payment ID %li.", nReversedPaymentID, nOldReversedPaymentID);
					}
					else {
						Log("Payment ID %li has been reversed, will be voided.", nReversedPaymentID);
					}

					CFinancialCorrection finCor;

					CString strUsername = GetCurrentUserName();
					long nCurrUserID = GetCurrentUserID();

					// (j.jones 2012-10-02 09:15) - PLID 52529 - load the takeback batch payment check number and date
					if(bNeedLoadBatchPayInfo) {
						_RecordsetPtr rsBatchPay = CreateParamRecordset("SELECT CheckNo, Date FROM BatchPaymentsT WHERE ID = {INT}", nBatchPaymentID);
						if(!rsBatchPay->eof) {
							strBatchPaymentCheckNo = VarString(rsBatchPay->Fields->Item["CheckNo"]->Value, "");
							strBatchPaymentCheckNo.TrimLeft(); strBatchPaymentCheckNo.TrimRight();
							strBatchPaymentDate = FormatDateTimeForInterface(VarDateTime(rsBatchPay->Fields->Item["Date"]->Value), NULL, dtoDate);
						}
						rsBatchPay->Close();

						//set the flag such that we only run this recordset one time
						bNeedLoadBatchPayInfo = FALSE;
					}

					//void, but do not correct, and track this batch payment ID so the voided amount is credited towards the batch payment balance
					// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date
					//TES 7/30/2014 - PLID 62536 - Use calculated reversed payment ID (ANSI or Alberta)
					finCor.AddCorrection(ctPayment, nReversedPaymentID, strUsername, nCurrUserID, FALSE,
						nBatchPaymentID, strBatchPaymentCheckNo, strBatchPaymentDate);

					// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
					// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.
					//_RecordsetPtr rsCorrectedApplies = CreateParamRecordset("SELECT SourceID FROM AppliesT WHERE DestID = {INT} GROUP BY SourceID", pCharge->nReversedPaymentID);
					//while(!rsCorrectedApplies->eof){
					//	long nAppliedItemID = AdoFldLong(rsCorrectedApplies, "SourceID");
					//	//void, but do not correct, and do not credit towards the batch payment balance
					//	finCor.AddCorrection(ctPayment, nAppliedItemID, strUsername, nCurrUserID, FALSE);

					//	rsCorrectedApplies->MoveNext();
					//}
					//rsCorrectedApplies->Close();

					finCor.ExecuteCorrections(TRUE);
				}
			}
			// (j.jones 2006-08-03 14:21) - PLID 21603 - only make $0.00 payments if we have a non-zero adjustment
			// (j.jones 2008-11-21 10:51) - PLID 32076 - or if $0.00 adjustments are allowed
			// (j.armen 2012-04-24 17:33) - PLID 46531 - We actually want to post all zero and positive payments.
			//	We do this especially in the case of a secondary insurance paying $0.00 and not adjusting
			else if(cyTotalPaymentAmt >= COleCurrency(0,0) && !bIsReversedClaim) {

				CString strBillingNote = "";
				// (j.jones 2009-03-05 11:44) - PLID 33235 - supported adding billing notes, though this only
				// happens in OHIP right now
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip) {
					OHIPEOBLineItemInfo *pCharge = NULL;
					_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
					if(var.vt == VT_I4) {
						pCharge = (OHIPEOBLineItemInfo*)VarLong(var);
						if(pCharge != NULL && !pCharge->strExplanatoryCode.IsEmpty()
							&& !pCharge->strExplanatoryDescription.IsEmpty()) {

							//build our billing note
							strBillingNote.Format("OHIP Explanatory Code for Payment: %s - %s", pCharge->strExplanatoryCode, pCharge->strExplanatoryDescription);
						}
					}
				}
				
				// (d.singleton 2014-10-15 14:50) - PLID 62698 - added claimnumber
				CString strClaimNumber = "";
				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
				if (var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
				}
				// only care about usa
				if (pClaim && m_ERemitType == ertAnsi) {
					strClaimNumber = pClaim->strOriginalRefNum;
				}

				// (s.dhole 2013-06-24 16:20) - PLID 42275 userdefinedid
				// (j.jones 2016-04-19 17:33) - NX-100246 - cache the new payment ID
				nNewPaymentID = ApplyPayment(nBatchPaymentID, nPatientID, nPatientUserDefinedID, nChargeID, nInsuredPartyID, cyTotalPaymentAmt, cyPaymentAmtToApply, strBillingNote, strClaimNumber);

				// (j.jones 2009-03-05 15:51) - PLID 33086 - if OHIP, and the payment is less than the current insurance resp.,
				// we need to auto-adjust the remaining insurance resp.
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip && nChargeID != -1) {

					//find our balance
					COleCurrency cyInsBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);
					
					//if we are applying adjustments already (highly unlikely), decrement them from this balance
					if(cyTotalAdjustmentAmt > COleCurrency(0,0)) {
						cyInsBalance -= cyTotalAdjustmentAmt;
					}

					//if we still have a balance, add it to our adjustment amount
					if(cyInsBalance > COleCurrency(0,0)) {
						cyTotalAdjustmentAmt += cyInsBalance;
						
						// (j.jones 2016-04-19 13:39) - NX-100161 - also update the total positive adjustments
						cyTotalPositiveAdj += cyInsBalance;
					}
				}
			}

			//Step 3. Apply an adjustment to each adjusted charge of each patient in the list.
			
			// (j.jones 2012-04-20 09:31) - PLID 49847 - if the adjustment is negative, take it back (US only)
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType, Alberta does have reversals
			// (j.jones 2016-04-19 13:44) - NX-100161 - now we look at the adjustment pointers only
			if (bIsReversedClaim && bHasNonZeroAdjustments && m_ERemitType != ertOhip && m_ERemitType != ertAlberta) {

				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
				if(var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
				}

				EOBLineItemInfo *pCharge = NULL;
				var = pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR);
				if(var.vt == VT_I4) {
					pCharge = (EOBLineItemInfo*)VarLong(var);
				}

				//throw exceptions in impossible states, we shouldn't get to this point if any of the following is true
				if(pClaim == NULL || pCharge == NULL) {
					ThrowNxException("A reversal was found with incomplete EOB information.");
				}
				else if(!pClaim->bIsReversedClaim) {
					ThrowNxException("A reversal was found on a charge that is not flagged as a reversal.");
				}

				BOOL bExecuteCorrections = FALSE;
				CFinancialCorrection finCor;
				CString strUsername = GetCurrentUserName();
				long nCurrUserID = GetCurrentUserID();

				for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

					EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];
					if(pAdj->bPatResp) {
						//we don't reverse patient resp. info
						continue;
					}

					// (j.dinatale 2013-03-25 17:39) - PLID 55825 - the user already decided to skip these adjustments, so just continue
					// (j.jones 2016-04-19 16:09) - NX-100161 - do not skip positive (reversed negative) adjustments
					/*
					if(pAdj->cyAdjustmentAmt >= g_ccyZero){
						continue;
					}
					*/

					if(pAdj->nReversedAdjustmentID == -1) {
						//we didn't find an adjustment, is that ok?
						if(pAdj->bMissingReversedAdjustmentOK) {
							//it's safe to skip this adjustment
							continue;
						}
						else {
							//the code should not have been permitted posting this charge
							ThrowNxException("A negative adjustment was found on a charge, but no matching adjustment was found on the patient's account.");
						}
					}
					
					//it is possible, albeit unlikely, that this adjustment ID has already been voided,
					//if so, recursively find the newest adjustment ID that is not voided
					long nOldReversedAdjustmentID = pAdj->nReversedAdjustmentID;
					pAdj->nReversedAdjustmentID = FindNewestCorrectedLineItemID(pAdj->nReversedAdjustmentID);

					if(pAdj->nReversedAdjustmentID == -1) {
						//cancel posting this line item
						Log("Adjustment ID %li had been reversed, but had previously been voided, with no corrected adjustment created. Reversal will be skipped.", pAdj->nReversedAdjustmentID);
					}
					else {				
						//log this
						if(pAdj->nReversedAdjustmentID != nOldReversedAdjustmentID) {
							Log("Adjustment ID %li has been reversed, will be voided. This is the newest corrected version of original adjustment ID %li.", pAdj->nReversedAdjustmentID, nOldReversedAdjustmentID);
						}
						else {
							Log("Adjustment ID %li has been reversed, will be voided.", pAdj->nReversedAdjustmentID);
						}

						//void, but do not correct, and link to this batch payment (it will not credit towards the batch payment balance
						// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date
						finCor.AddCorrection(ctPayment, pAdj->nReversedAdjustmentID, strUsername, nCurrUserID, FALSE,
							nBatchPaymentID, strBatchPaymentCheckNo, strBatchPaymentDate);
						bExecuteCorrections = TRUE;

						// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
						// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.
						//_RecordsetPtr rsCorrectedApplies = CreateParamRecordset("SELECT SourceID FROM AppliesT WHERE DestID = {INT} GROUP BY SourceID", pAdj->nReversedAdjustmentID);
						//while(!rsCorrectedApplies->eof){
						//	long nAppliedItemID = AdoFldLong(rsCorrectedApplies, "SourceID");
						//	//void, but do not correct, and do not credit towards the batch payment balance
						//	finCor.AddCorrection(ctPayment, nAppliedItemID, strUsername, nCurrUserID, FALSE);

						//	rsCorrectedApplies->MoveNext();
						//}
						//rsCorrectedApplies->Close();
					}
				}

				if(bExecuteCorrections) {
					finCor.ExecuteCorrections(TRUE);
				}
			}			
			// (j.jones 2016-04-19 13:44) - NX-100161 - now we look at the adjustment pointers only
			else if((bHasNonZeroAdjustments || bAllowZeroDollarAdjustments) && !bIsReversedClaim) {
				
				// (j.jones 2009-03-05 11:51) - PLID 33235 - this advanced adjustment code is not used for OHIP
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType, excluded Alberta from this branch
				if (m_ERemitType == ertAnsi) {

					EOBClaimInfo *pClaim = NULL;
					_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
					if(var.vt == VT_I4) {
						pClaim = (EOBClaimInfo*)VarLong(var);
					}

					EOBLineItemInfo *pCharge = NULL;				
					var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
					if(var.vt == VT_I4) {
						pCharge = (EOBLineItemInfo*)VarLong(var);
					}

					// (j.jones 2008-11-17 13:48) - PLID 32045 - Do not apply just one lump adjustment,
					// instead attempt to break it down by what was sent in the EOB, in the order it was sent
					// (which is the order of the array). For example, if we have a $50 CO*2 adjustment and
					// a $50 OA*42 adjustment, but we only apply $60, create a $50 CO*2 adjustment and a $10
					// OA*42 adjustment. If we only apply $20, create a $20 CO*2 adjustment.

					// (j.jones 2008-11-21 10:51) - PLID 32076 - when the preference is enabled to allow $0.00
					// adjustments, we will create every adjustment the EOB reported, to track the group & reason codes

					//try to apply charge adjustments BEFORE claim adjustments, they have higher priority
					if(pCharge != NULL) {
						for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

							const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];

							// (j.jones 2016-04-26 14:08) - NX-100327 - turn the adjustment to zero if secondary,
							// and the preference says to ignore secondary
							COleCurrency cyAdjAmount = pAdj->cyAdjustmentAmt;
							//this is for the hardcoded Secondary Medical only
							if (nRespTypeID == 2 && (GetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", 1, 0, "<None>", true) == 1)) {
								cyAdjAmount = COleCurrency(0, 0);
							}

							// (j.jones 2016-04-19 14:03) - NX-100161 - we now support negative adjustments
							if(!pAdj->bPatResp	//insurance adjustments only
								&& (bAllowZeroDollarAdjustments || cyAdjAmount != COleCurrency(0,0))) {

								// (j.jones 2010-04-09 12:14) - PLID 31309 - added dtAdjDate
								// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid

								// (d.singleton 2014-10-15 14:50) - PLID 62698 - added claimnumber
								CString strClaimNumber = "";
								EOBClaimInfo *pClaim = NULL;
								_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
								if (var.vt == VT_I4) {
									pClaim = (EOBClaimInfo*)VarLong(var);
								}
								// only care about usa
								if (pClaim && m_ERemitType == ertAnsi) {
									strClaimNumber = pClaim->strOriginalRefNum;
								}

								ApplyAdjustment(nBatchPaymentID, nPatientID, nPatientUserDefinedID, nChargeID, nInsuredPartyID, cyAdjAmount, dtAdjDate, pAdj->strGroupCode, pAdj->strReasonCode, pAdj->strReason, nAdjustmentCategoryID, strAdjDescription, strClaimNumber, nNewPaymentID);
							}
						}
					}

					//now try claim adjustments
					if(pClaim != NULL) {
						for(int nAdjIndex=0; nAdjIndex<pClaim->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

							const EOBAdjustmentInfo *pAdj = pClaim->arypEOBAdjustmentInfo[nAdjIndex];

							// (j.jones 2016-04-26 14:08) - NX-100327 - turn the adjustment to zero if secondary,
							// and the preference says to ignore secondary
							COleCurrency cyAdjAmount = pAdj->cyAdjustmentAmt;
							//this is for the hardcoded Secondary Medical only
							if (nRespTypeID == 2 && (GetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", 1, 0, "<None>", true) == 1)) {
								cyAdjAmount = COleCurrency(0, 0);
							}

							// (j.jones 2016-04-19 14:03) - NX-100161 - we now support negative adjustments
							if(!pAdj->bPatResp	//insurance adjustments only
								&& (bAllowZeroDollarAdjustments || cyAdjAmount != COleCurrency(0,0))) {
								
								// (j.jones 2008-02-20 11:30) - PLID 29007 - added nAdjustmentCategoryID and strGlobalAdjustmentDescription								
								// (j.jones 2010-04-09 12:14) - PLID 31309 - added dtAdjDate
								// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid

								// (d.singleton 2014-10-15 14:50) - PLID 62698 - added claimnumber
								CString strClaimNumber = "";
								EOBClaimInfo *pClaim = NULL;
								_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
								if (var.vt == VT_I4) {
									pClaim = (EOBClaimInfo*)VarLong(var);
								}
								// only care about usa
								if (pClaim && m_ERemitType == ertAnsi) {
									strClaimNumber = pClaim->strOriginalRefNum;
								}
									
								ApplyAdjustment(nBatchPaymentID, nPatientID, nPatientUserDefinedID, nChargeID, nInsuredPartyID, cyAdjAmount, dtAdjDate, pAdj->strGroupCode, pAdj->strReasonCode, pAdj->strReason, nAdjustmentCategoryID, strAdjDescription, strClaimNumber, nNewPaymentID);
							}
						}
					}
				}
				
				// (j.jones 2016-04-22 14:12) - NX-100161 - OHIP does not have adjustments, removed all that code
			}

			// (j.jones 2011-04-26 17:06) - PLID 42705 - re-worked the if statements for adding billing notes,
			// because they duplicated code unnecessarily
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType; exclude Alberta for the moment, it will have billing notes for denied/partial payments, but
			// the logic may be different.
			if (m_ERemitType == ertAnsi && nChargeID != -1) {
				
				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
				if(var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
				}

				EOBLineItemInfo *pCharge = NULL;				
				var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
				if(var.vt == VT_I4) {
					pCharge = (EOBLineItemInfo*)VarLong(var);
				}

				//it is possible for nChargeID to be -1 and pCharge to be NULL if
				//the charge was one we added ourselves that was not listed by the EOB

				// (j.jones 2012-04-24 15:47) - PLID 35306 - don't track notes, coinsurance, or allowables from reversed claims
				if(pClaim != NULL && pCharge != NULL && !pClaim->bIsReversedClaim) {

					// (j.jones 2011-01-06 15:45) - PLID 41785 - if the preference is enabled to append PR adjustment
					// reasons to billing notes, do so now, but only if there is a specified patient resp
					// (j.jones 2011-11-08 14:57) - PLID 46240 - this variable has been renamed to cyOtherResp, but it's
					// still the PR adjustment, and we still need to add a note for it
					if(cyOtherResp > COleCurrency(0,0)
						&& GetRemotePropertyInt("ERemit_AddCASPR_BillingNote", 1, 0, "<None>", true) != 0) {
						
						CreateBillingNoteFromPatientRespReasons(nPatientID, nChargeID, strInsuranceCoName, pClaim, pCharge, cyOtherResp);
					}

					// (j.jones 2011-01-07 14:25) - PLID 41980 - if the preference is enabled to append detailed info
					// to billing notes, do so now
					if(GetRemotePropertyInt("ERemit_AddDetailedInfo_BillingNote", 0, 0, "<None>", true) != 0) {

						CreateBillingNoteWithDetailedInfo(nPatientID, nChargeID, strInsuranceCoName, pClaim, pCharge);
					}

					// (j.jones 2011-12-19 14:28) - PLID 43925 - if there's a deductible or co-insurance on this charge,
					// save it to ChargeCoinsuranceT
					TrySaveCoinsurance(nPatientID, strPatientName, nChargeID, nInsuredPartyID, pCharge);

					// (j.jones 2011-04-26 17:04) - PLID 42705 - store the allowable in ChargeAllowablesT,
					// even if it is zero, but not if it is invalid
					if(pCharge->cyChargeAllowedAmt.GetStatus() != COleCurrency::invalid
						&& pCharge->cyChargeAllowedAmt >= COleCurrency(0,0)) {

						//this function will update ChargeAllowablesT appropriately, and audit
						//if we are changing an existing value
						SaveChargeAllowable(nPatientID, strPatientName, nChargeID, nInsuredPartyID, pCharge->cyChargeAllowedAmt, caemERemittance);
					}
				}
			}
			else if (m_ERemitType == ertAlberta) {
				//TES 9/18/2014 - PLID 62581 - Check for billing notes we might want to create
				AlbertaAssessments::ChargeInfoPtr pCharge = NULL;
				_variant_t var = pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR);
				if (var.vt == VT_I4) {
					pCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
				}

				if (pCharge != NULL) {
					//TES 9/18/2014 - PLID 62582 - First, check if it's refused
					if (pCharge->eAssessmentResultAction_Main == aramRefused) {
						//TES 9/18/2014 - PLID 62582 - Create a note with the refusal reason(s)
						//TES 10/8/2014 - PLID 62582 - Add charge information
						CreateBillingNoteFromRefusal(nPatientID, nBillID, pCharge, strServiceCode, dtChargeDate, cyChargeAmount);
						//TES 10/1/2014 - PLID 62783 - Rejected is the highest priority status, so remember this for this bill
						mapBillStatuses.SetAt(nBillID, eRejected);
					}
					//TES 10/2/2014 - PLID 63821 - Set the status note on Held claims
					else if (pCharge->eAssessmentResultAction_Main == aramHeld) {
						SetBillStatusNoteFromHoldReason(nPatientID, nBillID, pCharge);
					}
					//TES 9/18/2014 - PLID 62581 - Otherwise, check if they paid less than the claimed amount
					//TES 9/30/2014 - PLID 62536 - But not if this was a reversal (which may add a partial pay to an existing partial pay, for example)
					else if (pCharge->cyFinalAssessedAmount < pCharge->cyClaimedAmount && pCharge->eAssessmentResultAction_Additional != araaReversal) {
						//TES 9/18/2014 - PLID 62581 - Create a note with the reason(s) for the payment reduction
						//TES 10/8/2014 - PLID 62581 - Add charge information
						CreateBillingNoteFromPartialPayment(nPatientID, nBillID, pCharge, strServiceCode, dtChargeDate, cyChargeAmount);
						//TES 10/1/2014 - PLID 62782 - Unless this bill is already rejected, set it to Partially Paid
						AlbertaBillingStatus abs = ePosted;
						mapBillStatuses.Lookup(nBillID, abs);
						if (abs != eRejected) {
							mapBillStatuses.SetAt(nBillID, ePartiallyPaid);
						}
					}
					else if (pCharge->eAssessmentResultAction_Additional == araaReversal) {
						//TES 9/26/2014 - PLID 62536 - Create a note when reversing a payment on a charge
						//TES 10/8/2014 - PLID 62536 - Add charge information
						CreateBillingNoteFromReversalReason(nPatientID, nBillID, pCharge, strServiceCode, dtChargeDate, cyChargeAmount);
					}
				}
			}
			// (b.spivey, November 01, 2012) - PLID 49943 - Use the preference to figure out if we should update or not.
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType, exclude Alberta from this logic
			if ((m_nEOBUpdateBillOriginaRefNo == 1 || m_nEOBUpdateBillOriginaRefNo == 2) && m_ERemitType == ertAnsi && nChargeID != -1) {
				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
				if(var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
				}

				//update the bill's reference number from the EOB. 
				UpdateBillRefNumber(nBillID, pClaim);
			}

			//TES 9/25/2014 - PLID 62781 - Now set the bill status to "Posted" (unless we already set it to "Rejected" or "Partially Paid")
			if (m_ERemitType == ertAlberta) {
				AlbertaBillingStatus abs = ePosted;
				mapBillStatuses.Lookup(nBillID, abs);
				if (abs != eRejected && abs != ePartiallyPaid) {
					mapBillStatuses.SetAt(nBillID, ePosted);
				}
			}

			// (j.jones 2009-04-07 16:40) - ERemittanceHistoryT does not support -1 charge IDs, we may support it in the future.
			if(nChargeID != -1) {
				long nShiftType = 0;
				if(m_ShiftRespCombo->CurSel != -1)
					nShiftType = VarLong(m_ShiftRespCombo->GetValue(m_ShiftRespCombo->GetCurSel(),0));

				// (j.jones 2011-03-18 09:19) - PLID 42157 - added FeeSchedAllowable and EOBAllowable, both nullable,
				// and parametized this query as well
				// (j.jones 2011-07-18 16:35) - PLID 44610 - store the cyOriginalPaymentAmount, which might not be what was paid
				// if the payment was converted to an adjustment due to a takeback
				ExecuteParamSql("INSERT INTO ERemittanceHistoryT (ID, EOBID, PostingDate, BatchPayID, PatientID, InsuredPartyID, ChargeID, "
					"ChargeAmount, PaymentAmount, AdjustmentAmount, PatResp, PatApplies, PatBalance, InsBalance, ShiftType, "
					"FeeSchedAllowable, EOBAllowable) "
					"VALUES ({INT}, {INT}, GetDate(), {INT}, {INT}, {INT}, {INT},"
					"{OLECURRENCY}, {OLECURRENCY}, {OLECURRENCY}, {OLECURRENCY},"
					"{OLECURRENCY}, {OLECURRENCY}, {OLECURRENCY}, "
					"{INT}, {VT_CY}, {VT_CY})",
					NewNumber("ERemittanceHistoryT","ID"), nEOBID, nBatchPaymentID, nPatientID, nInsuredPartyID, nChargeID,
					VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0)), cyTotalPaymentAmt, cyTotalAdjustmentAmt,
					VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0)), VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_APPLIES),COleCurrency(0,0)),
					VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_BALANCE),COleCurrency(0,0)), VarCurrency(pLoopRowPtr->GetValue(COLUMN_INS_BALANCE),COleCurrency(0,0)),
					nShiftType, pLoopRowPtr->GetValue(COLUMN_FEE_SCHED_ALLOWABLE), pLoopRowPtr->GetValue(COLUMN_EOB_ALLOWABLE));
			}
		}
	}

	//TES 10/1/2014 - PLID 62782 - Now go through each of the bills we tracked an Alberta status for, and set the status on the bill
	POSITION pos = mapBillStatuses.GetStartPosition();
	while (pos != NULL) {
		long nBillID = -1;
		AlbertaBillingStatus abs = ePosted;
		mapBillStatuses.GetNextAssoc(pos, nBillID, abs);
		TrySetAlbertaStatus(nBillID, abs);
}
}

// (j.dinatale 2013-03-25 17:39) - PLID 55825 - its possible to have invalid reversals... AWESOME!
// (j.jones 2016-04-19 16:09) - NX-100161 - obsolete now
//BOOL CEOBDlg::CheckForInvalidReversals()

BOOL CEOBDlg::CheckEnsureResponsibilities()
{
	BOOL bBlankPatients = FALSE;

	m_progress.SetPos(0);
	m_progress.SetRange(0,(short)m_EOBList->GetRowCount());
	m_progress.SetStep(1);

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);

	CString str = "Preparing to validate responsibilities...";
	m_progressStatus.SetWindowText(str);

	// (j.jones 2010-12-16 09:59) - PLID 41176 - added ability to skip posting adjustments and no payments
	BOOL bAutoSkipZeroPays = (GetRemotePropertyInt("ERemitSkipZeroPays", 0, 0, "<None>", true) == 1);
	// (b.spivey, July 03, 2013) - PLID 56825 - Check this preference, as we may be wanting to post notes to charges even though
	//		we skipped over them because there was nothing to post but an adjustment. 
	// (b.spivey, July 30, 2013) - PLID 56825 - Default to on...
	bool bPostAdjustmentNoteToBill = 
		(GetRemotePropertyInt("ERemitPostAdjustmentNoteWhenZeroPays", 1, 0, "<None>", true) ? true : false);

	//We want to handle the case where there are multiple payments for one charge,
	//which means we need to make sure the total amount is available.
	NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr;
	//Store the charge ID, charge amount, and total payment, adjustment, pat. resp
	CPtrArray aryCharges;
	for (pLoopRowPtr = m_EOBList->GetFirstRow(); pLoopRowPtr != nullptr; pLoopRowPtr = pLoopRowPtr->GetNextRow()) {

		long nChargeID = VarLong(pLoopRowPtr->GetValue(COLUMN_CHARGE_ID), -1);
		CString strPatientName = VarString(pLoopRowPtr->GetValue(COLUMN_PATIENT_NAME),"");
        // (a.levy 2013-11-13 13:04) - PLID - 59445- Fixed for Value of NULL.
		// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
        long nPatientUserDefinedID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_USERDEFINED_ID), -1);
		
		COleDateTime dtChargeDate = VarDateTime(pLoopRowPtr->GetValue(COLUMN_CHARGE_DATE), COleDateTime::GetCurrentTime());
		COleCurrency cyCharges = VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0));
		COleCurrency cyInsPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));

		// (j.jones 2016-04-26 14:08) - NX-100327 - the adjustment column is total adjustments, and is now
		// for UI purposes only - we need to pull the real adjustments from the EOB info
		COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
		COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
		COleCurrency cyTotalAdjustments = COleCurrency(0, 0);
		GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjustments);

		//if the total is zero, but the total positive is not, we must have negative adjustments balancing it out (not common)
		bool bHasNonZeroAdjustments = (cyTotalPositiveAdj > COleCurrency(0, 0) || cyTotalNegativeAdj < COleCurrency(0, 0));

		// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
		// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
		COleCurrency cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0));

		BOOL bSkipped = VarBool(pLoopRowPtr->GetValue(COLUMN_SKIPPED), FALSE);
		BOOL bIsReversal = VarBool(pLoopRowPtr->GetValue(COLUMN_REVERSAL), FALSE);

		// (j.jones 2011-02-09 13:28) - PLID 42391 - warn about reversals before the skip warning,
		// since currently all reversals are skipped
		// (j.jones 2012-04-18 16:02) - PLID 35306 - we now allow posting reversals, so now always log this,
		// just tweak the log content if we are skipping it
		if(bIsReversal) {
			//TES 9/26/2014 - PLID 62536 - Separate logic for ANSI and Alberta
			if (m_ERemitType == ertAnsi) {
			//log the reversal information
			CString strWarn;

			EOBClaimInfo *pClaim = NULL;
			_variant_t var = pLoopRowPtr->GetValue( COLUMN_CLAIM_PTR);
			if(var.vt == VT_I4) {
				pClaim = (EOBClaimInfo*)VarLong(var);
			}

			EOBLineItemInfo *pCharge = NULL;				
			var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
			if(var.vt == VT_I4) {
				pCharge = (EOBLineItemInfo*)VarLong(var);
			}

			if(cyTotalAdjustments < COleCurrency(0,0) || cyInsPays < COleCurrency(0,0)) {
				//this line merely undid the previous posting

				// (j.jones 2012-04-18 16:11) - PLID 35306 - note if it was skipped
				CString strSkipInfo;
				if(bSkipped && IsUnpostableReversalCharge(pCharge, pClaim)) {
					strSkipInfo = "This charge was skipped because the reversed payment and adjustments could not be found. Please review this patient's account and remove the following prior payments and adjustments from this company:";
				}
				else if(bSkipped) {
					strSkipInfo = "This charge was manually skipped. Please review this patient's account and remove the following prior payments and adjustments from this company:";
				}
				else {
					strSkipInfo = "Practice will void the following prior payments and adjustments from this company:";
				}

				// (j.jones 2011-09-15 11:23) - PLID 45493 - include the amounts that were reversed
				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				strWarn.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s has had its prior posting reversed by the insurance company.\n"
					"%s\n"
					" - %s in payments.\n"
					" - %s in adjustments.",
					strPatientName,nPatientUserDefinedID , FormatDateTimeForInterface(dtChargeDate, dtoDate), FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
					strSkipInfo,
					FormatCurrencyForInterface(-cyInsPays, TRUE, TRUE), FormatCurrencyForInterface(-cyTotalAdjustments, TRUE, TRUE));
			}
			else {
				//this line is the new information

				// (j.jones 2012-04-18 16:11) - PLID 35306 - note if it was skipped
				CString strSkipInfo;
				if(bSkipped && HasUnpostableReversalSibling(pLoopRowPtr, nChargeID, pClaim, pCharge)) {
					strSkipInfo = "This charge was skipped because the reversed payment and adjustments could not be found. Please review this patient's account and correct the payments and adjustments from this company.";
				}
				else if(bSkipped) {
					strSkipInfo = "This charge was manually skipped. Please review this patient's account and correct the payments and adjustments from this company.";
				}
				else {
					strSkipInfo = "Practice will post the new payments and adjustments from this company.";
				}

				// (j.jones 2011-11-08 14:57) - PLID 46240 - the cyPatResp variable has been renamed to cyOtherResp, but it
				// should still be labeled as patient resp. in this message
				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				strWarn.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s has been reversed and re-posted by the insurance company.\n"
					"The EOB now reports the following new posting information:\n"
					"- %s in payments.\n"
					"- %s in adjustments.\n"
					"- %s in patient responsibility.\n\n"
					"%s",
					strPatientName,nPatientUserDefinedID, FormatDateTimeForInterface(dtChargeDate, dtoDate), FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
					FormatCurrencyForInterface(cyInsPays, TRUE, TRUE), FormatCurrencyForInterface(cyTotalAdjustments, TRUE, TRUE), FormatCurrencyForInterface(cyOtherResp, TRUE, TRUE),
					strSkipInfo);
			}
			
			AddWarningToLog(strWarn,"");

			//the bSkipped check later would continue anyways, but we might as well continue now
			//since we already made the log
			if(bSkipped) {
				continue;
			}
		}
			else {
				//TES 9/26/2014 - PLID 62536 - Handle Alberta reversals
				ASSERT(m_ERemitType == ertAlberta);
				//log the reversal information
				CString strWarn;

				AlbertaAssessments::ChargeInfoPtr pCharge = NULL;
				_variant_t var = pLoopRowPtr->GetValue( COLUMN_CHARGE_PTR);
				if (var.vt == VT_I4) {
					pCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
				}

				
				//this line is the new information

				// (j.jones 2012-04-18 16:11) - PLID 35306 - note if it was skipped
				CString strSkipInfo;
				if (bSkipped) {
					strSkipInfo = "This charge was manually skipped. Please review this patient's account and correct the payments and adjustments from this company.";
				}
				else {
					strSkipInfo = "Practice will post the new payments from this company.";
				}

				// (j.jones 2011-11-08 14:57) - PLID 46240 - the cyPatResp variable has been renamed to cyOtherResp, but it
				// should still be labeled as patient resp. in this message
				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				//TES 9/30/2014 - PLID 62536 - Alberta never has adjustments
				strWarn.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s has been reversed and re-posted by the insurance company.\n"
					"The EOB now reports the following new posting information:\n"
					"- %s in payments.\n"
					"- %s in patient responsibility.\n\n"
					"%s",
					strPatientName, nPatientUserDefinedID, FormatDateTimeForInterface(dtChargeDate, dtoDate), FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
					FormatCurrencyForInterface(cyInsPays, TRUE, TRUE), FormatCurrencyForInterface(cyOtherResp, TRUE, TRUE),
					strSkipInfo);

				AddWarningToLog(strWarn, "");

				//the bSkipped check later would continue anyways, but we might as well continue now
				//since we already made the log
				if (bSkipped) {
					continue;
				}
			}
		}

		// (j.jones 2011-09-28 14:43) - PLID 45486 - if this claim has invalid payment totals,
		// we cannot post it, so warn the user (US only)
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		//TES 9/26/2014 - PLID 62536 - Alberta can't have invalid pay totals
		if(m_ERemitType == ertAnsi) {
			EOBClaimInfo *pClaim = NULL;
			_variant_t var = pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR);
			if(var.vt == VT_I4) {
				pClaim = (EOBClaimInfo*)VarLong(var);
				if(pClaim != NULL && m_EOBParser.HasInvalidPayTotals(pClaim)) {
					CString strWarning;

					// (j.jones 2011-11-08 14:57) - PLID 46240 - the cyPatResp variable has been renamed to cyOtherResp, but it
					// should still be labeled as patient resp. in this message
					// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
					strWarning.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s is for a claim with invalid payment totals listed in the EOB.\n"
						"The claim payment field in this EOB shows %s, but the total payment amount applied to the claim's charges adds up to %s.\n"
						"The EOB reports the following posting information for this charge:\n"
						"- %s in payments.\n"
						"- %s in adjustments.\n"
						"- %s in patient responsibility.\n\n"
						"The payments and adjustments for this charge will need to be posted manually.",
						strPatientName,nPatientUserDefinedID, FormatDateTimeForInterface(dtChargeDate, dtoDate), FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
						FormatCurrencyForInterface(pClaim->cyClaimPaymentAmt), FormatCurrencyForInterface(m_EOBParser.GetPaymentTotalsForCharges(pClaim)),
						FormatCurrencyForInterface(cyInsPays, TRUE, TRUE), FormatCurrencyForInterface(cyTotalAdjustments, TRUE, TRUE), FormatCurrencyForInterface(cyOtherResp, TRUE, TRUE));
					
					AddWarningToLog(strWarning,"");
					continue;
				}
			}
		}

		//ignore the skipped rows, obviously!
		if(bSkipped) {

			// (j.jones 2012-04-18 16:09) - PLID 35306 - if we skipped a reversal, we already logged it above (and should have continued, actually)


			if(!bIsReversal) { 

				// (b.spivey, July 09, 2013) - PLID 56825 - Under a very tight set of circumstances, we may not assume 
				//		this was manually skipped. 
				BOOL bZeroPayCharge = (cyInsPays == COleCurrency(0,0) && bHasNonZeroAdjustments && cyOtherResp == COleCurrency(0,0));
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType, excluded Alberta from this branch.
				if (bZeroPayCharge && bPostAdjustmentNoteToBill && m_ERemitType == ertAnsi) {
					
					/*
					// (b.spivey, July 09, 2013) - PLID 56825:
					This code is bits and pieces of other code that was already handling certain aspects 
					of what I wanted to do. The only problem is I wanted to do it before that code did, but didn't want to 
					hold all this stuff in memory ahead of time, because they'd need to skip a large number of claims
					for this to be a huge performance hit. */
					CString strWarningToLog = "", strAdjReasons = "", strServiceCode = "";
					strAdjReasons = VarString(pLoopRowPtr->GetValue(COLUMN_ADJ_REASON), "");
					strServiceCode = VarString(pLoopRowPtr->GetValue(COLUMN_CPT_CODE), "");


					EOBLineItemInfo *pSkippedCharge = NULL;				
					_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
					if(var.vt == VT_I4) {
						pSkippedCharge = (EOBLineItemInfo*)VarLong(var);
					}

					// (j.jones 2016-04-19 14:17) - NX-100161 - just report the total adjustments here, yes it
					// will look weird if they balance to zero, but that is extremely unlikely
					if(nChargeID == -1) {
						CString strServiceID, strChargeDate, strChargeAmt;
						// (z.manning 2013-07-31 14:27) - PLID 57810 - Added null check to be safe
						if(pSkippedCharge != NULL) {
							strServiceID = pSkippedCharge->strServiceID;
							strChargeDate = FormatDateTimeForInterface(pSkippedCharge->dtChargeDate, NULL, dtoDate);
							strChargeAmt = FormatCurrencyForInterface(pSkippedCharge->cyLineItemChargeAmt);
						}
						// (s.dhole 2013-07-23 11:46) - PLID 42275 Added userdefinedid
						strWarningToLog.Format("For patient '%s' [Patient ID: %li], the EOB has indicated that no insurance payments\n"
							"will be applied to a charge '%s' (Date Of Service: %s, Charge Amount: %s)\n"
							"that does not exist in the system, and also states that a %s adjustment\n"
							"should be applied with reason of:\n"
							"%s",
							strPatientName, nPatientUserDefinedID, strServiceID,
							strChargeDate,
							strChargeAmt,
							FormatCurrencyForInterface(cyTotalAdjustments),
							strAdjReasons);
					}
					else {
						// (s.dhole 2013-07-23 11:46) - PLID 42275 Added userdefinedid
						strWarningToLog.Format("For patient '%s' [Patient ID: %li], the EOB has indicated that no insurance payments\n"
							"will be applied to the charge '%s' (Date Of Service: %s, Charge Amount: %s),\n"
							"and also states that a %s adjustment should be applied with reason of:\n"
							"%s",
							strPatientName, nPatientUserDefinedID,strServiceCode,
							FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate),
							FormatCurrencyForInterface(cyCharges),
							FormatCurrencyForInterface(cyTotalAdjustments),
							strAdjReasons);
					}


					AddWarningToLog(strWarningToLog, "Skipped"); 
					
				}
				else {
					//put in the log that these patients were skipped
					// (s.dhole 2013-07-23 11:46) - PLID 42275 Added userdefinedid
					CString strWarn;
					strWarn.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s was manually skipped.",
						strPatientName,nPatientUserDefinedID, FormatDateTimeForInterface(dtChargeDate, dtoDate), FormatCurrencyForInterface(cyCharges, TRUE, TRUE));
					
					AddWarningToLog(strWarn,"");
				}

			}

			continue;
		}
		
		m_progress.StepIt();

		BOOL bFound = FALSE;
		long nPatientID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_ID));

		if(nPatientID == -1) {
			bBlankPatients = TRUE;
			continue;
		}

		long nInsuredPartyID = VarLong(pLoopRowPtr->GetValue(COLUMN_INSURED_PARTY_ID), -1);
		// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
		long nRespTypeID = VarLong(pLoopRowPtr->GetValue(COLUMN_RESP_TYPE_ID));
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		BOOL bSubmitAsPrimary = VarBool(pLoopRowPtr->GetValue(COLUMN_SUBMIT_AS_PRIMARY), FALSE);

		if(nInsuredPartyID == -1) {
			str.Format("The insured party for patient '%s' could not be found.\n"
				"Please double-check to make sure this patient's insurance company has not been changed or deleted.",GetExistingPatientName(nPatientID));
			MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
			return FALSE;
		}

		BOOL bDuplicate = VarLong(pLoopRowPtr->GetValue(COLUMN_DUPLICATE)) ? TRUE : FALSE;

		// (j.jones 2011-09-26 17:32) - PLID 45489 - get these values for warnings later
		CString strServiceCode = VarString(pLoopRowPtr->GetValue(COLUMN_CPT_CODE), "");
		CString strAdjReasons = VarString(pLoopRowPtr->GetValue(COLUMN_ADJ_REASON), "");
		EOBLineItemInfo *pEOBLineItemInfo = NULL;
		OHIPEOBLineItemInfo *pOHIPEOBLineItemInfo = NULL;
		//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
		AlbertaAssessments::ChargeInfoPtr pAlbertaCharge = NULL;
		_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
		if(var.vt == VT_I4) {
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (m_ERemitType == ertOhip) {
				pOHIPEOBLineItemInfo = (OHIPEOBLineItemInfo*)VarLong(var);
			}
			else if(m_ERemitType == ertAnsi) {
				pEOBLineItemInfo = (EOBLineItemInfo*)VarLong(var);
			}
			else {
				ASSERT(m_ERemitType == ertAlberta);
				pAlbertaCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
		}
		}

		// (j.jones 2011-09-26 17:24) - PLID 45489 - split the "zero pay charge" calculation into its own boolean
		BOOL bZeroPayCharge = (cyInsPays == COleCurrency(0,0) && bHasNonZeroAdjustments && cyOtherResp == COleCurrency(0,0));
		if(bZeroPayCharge) {
			//TES 9/18/2014 - PLID 62777 - Alberta doesn't have adjustments, so it should never hit this branch
			ASSERT(m_ERemitType != ertAlberta);

			// (j.jones 2010-12-16 10:13) - PLID 41176 - The duplicate status is per charge line returned in the EOB,
			// but when the same charge exists more than once (rare), we group the total pays/adjustments for the charge
			// and post those totals. If bAutoSkipZeroPays is TRUE, we would ideally want to not bother warning about a duplicate
			// when we are skipping the charge anyways. So only skip the duplicate warning for this line item if this
			// line item's payment is zero and adjustments are non-zero and a patient resp. does not exist.
			BOOL bSkipPostingThisCharge = (bAutoSkipZeroPays && bZeroPayCharge);

			// (j.jones 2009-04-07 14:45) - PLID 33862 - prompt now if it is a duplicate, before EnsureValidResponsibilities
			if(bDuplicate && !bSkipPostingThisCharge) {
				CString strWarningToLog;
				CString strWarning;
				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				// (j.jones 2016-04-19 14:17) - NX-100161 - just report the total adjustments here, yes it
				// will look weird if they balance to zero, but that is extremely unlikely
				strWarningToLog.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s is marked as being a duplicate claim.\n"
					"The EOB has indicated a payment of %s and an adjustment of %s.", strPatientName,nPatientUserDefinedID, 
					FormatDateTimeForInterface(dtChargeDate, dtoDate),
					FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
					FormatCurrencyForInterface(cyInsPays, TRUE, TRUE),
					FormatCurrencyForInterface(cyTotalAdjustments, TRUE, TRUE));
				strWarning = strWarningToLog + "\n\nWould you like to skip this duplicate claim?";
				int nRet = MessageBox(strWarning,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
				if(nRet == IDYES) {
					AddWarningToLog(strWarningToLog, "Skipped");
					pLoopRowPtr->PutValue(COLUMN_SKIPPED, g_cvarTrue);

					// (j.jones 2012-04-18 17:11) - PLID 35306 - if a reversal,
					// toggle its reversed sibling as skipped as well
					BOOL bIsReversal = VarBool(pLoopRowPtr->GetValue(COLUMN_REVERSAL), FALSE);
					//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
					if (bIsReversal && m_ERemitType != ertOhip) {
						ToggleSiblingRowSkipped(pLoopRowPtr, TRUE);
					}

					// (j.jones 2009-07-01 16:55) - PLID 34778 - since this charge has been skipped,
					// we must now update the overpayment label
					// (j.jones 2009-07-07 15:20) - PLID 34805 - no we don't, we will not change
					// the overage amount based on what is skipped
					//UpdateOverpaymentLabel();

					continue;
				}
				else if(nRet == IDNO) {
					AddWarningToLog(strWarningToLog, "Processed");
					//continue normally
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					for(int z=aryCharges.GetSize()-1;z>=0;z--) {
						delete (ChargeInfo*)aryCharges.GetAt(z);
						aryCharges.RemoveAt(z);
					}
					return FALSE;
				}
			}
			// (j.jones 2011-09-26 17:23) - PLID 45489 - this check for unpaid charges was previously after
			// we summed up payments for the same charge, but it should be before the summation, in the (rare)
			// event that one claim was both paid and not paid (due to being a duplicate) in the same EOB.
			// We would want to skip the unpaid charge.
			else {
				// (j.jones 2010-12-16 10:42) - PLID 41176 - If bAutoSkipZeroPays is TRUE, and there is no payment,
				// no patient resp., but there is an adjustment, skip this charge, and log that we did so.
				// (j.jones 2011-03-18 17:35) - PLID 42099 - now we always check this, and if bAutoSkipZeroPays is false,
				// we prompt and ask them if they want to skip

				BOOL bSkipThisCharge = bAutoSkipZeroPays;

				CString strWarningToLog;
				// (j.jones 2011-06-02 09:08) - PLID 43931 - we should still do this if the charge ID is -1, because we won't be posting
				// anything to this charge anyways, but the warning needs to be different
				// (j.jones 2016-04-19 14:17) - NX-100161 - just report the total adjustments here, yes it
				// will look weird if they balance to zero, but that is extremely unlikely
				if(nChargeID == -1) {
					// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
					strWarningToLog.Format("For patient '%s' [Patient ID: %li], the EOB has indicated that no insurance payments\n"
						"will be applied to a charge '%s' (Date Of Service: %s, Charge Amount: %s)\n"
						"that does not exist in the system, and also states that a %s adjustment\n"
						"should be applied with reason of:\n"
						"%s",
						strPatientName, nPatientUserDefinedID, pEOBLineItemInfo->strServiceID,
						FormatDateTimeForInterface(pEOBLineItemInfo->dtChargeDate, NULL, dtoDate),
						FormatCurrencyForInterface(pEOBLineItemInfo->cyLineItemChargeAmt),
						FormatCurrencyForInterface(cyTotalAdjustments),
						strAdjReasons);
				}
				else {
					// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
					strWarningToLog.Format("For patient '%s' [Patient ID: %li], the EOB has indicated that no insurance payments\n"
						"will be applied to the charge '%s' (Date Of Service: %s, Charge Amount: %s),\n"
						"and also states that a %s adjustment should be applied with reason of:\n"
						"%s",
						strPatientName,nPatientUserDefinedID, strServiceCode,
						FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate),
						FormatCurrencyForInterface(cyCharges),
						FormatCurrencyForInterface(cyTotalAdjustments),
						strAdjReasons);
				}

				if(bAutoSkipZeroPays) {
					//don't prompt, just skip it
					bSkipThisCharge = TRUE;

					//log that it was automatically skipped
					strWarningToLog += "\n\nYour preferences are set to automatically skip posting these adjustments.\n"
						"Please review the patient's claim and correct as needed.";
					AddWarningToLog(strWarningToLog, "Automatically Skipped");
						}
				else {
					// (j.jones 2011-03-18 17:36) - PLID 42099 - if the auto-skip preference is off, prompt the user using our
					// special dialog for zero pays
					CERemitZeroPaysDlg dlg(this);
					ERemitZeroPays_ReturnValues erzprvReturnValue = erzprv_Skip;
					// (j.jones 2011-06-02 09:08) - PLID 43931 - we should still do this if the charge ID is -1, because we won't be posting
					// anything to this charge anyways, but the warning needs to be different
					if(nChargeID == -1) {
						erzprvReturnValue = dlg.DoModal(strPatientName,
							pEOBLineItemInfo->strServiceID, pEOBLineItemInfo->dtChargeDate,
							pEOBLineItemInfo->cyLineItemChargeAmt,
							cyTotalAdjustments, strAdjReasons);
					}
					else {
						erzprvReturnValue = dlg.DoModal(strPatientName,
							strServiceCode, dtChargeDate, cyCharges,
							cyTotalAdjustments, strAdjReasons);
					}

					//this log will be slightly misleading in that it will list what the prompt *would* have been if
					//we didn't change this to a special dialog, but its meaning is clear enough
					strWarningToLog += "\nIt is recommended that you skip posting these adjustments. Do you wish to skip this charge?";

					if(erzprvReturnValue == erzprv_Skip) {
						AddWarningToLog(strWarningToLog, "Skipped");
						
						//find the row for this charge
						// (s.tullis 2016-04-15 16:00) - NX-100211
						// (j.jones 2011-06-02 09:47) - PLID 43931 - converted to use FindChargeInList
						//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge (though it should always be NULL here, as asserted above).
						NXDATALIST2Lib::IRowSettingsPtr pSearchRow = FindChargeInList(nChargeID, pEOBLineItemInfo, pOHIPEOBLineItemInfo, pAlbertaCharge);
						if(pSearchRow) {
							//mark this charge as skipped
							pSearchRow->PutValue(COLUMN_SKIPPED, g_cvarTrue);

							// (j.jones 2012-04-18 17:11) - PLID 35306 - if a reversal,
							// toggle its reversed sibling as skipped as well
							BOOL bIsReversal = VarBool(pSearchRow->GetValue( COLUMN_REVERSAL), FALSE);
							//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
							if (bIsReversal && m_ERemitType != ertOhip) {
								ToggleSiblingRowSkipped(pSearchRow, TRUE);
							}
						}
						continue;
					}
					else if(erzprvReturnValue == erzprv_Post) {
						AddWarningToLog(strWarningToLog, "Processed - Adjustments Applied");
						//continue normally
					}
					else {
						AddWarningToLog(strWarningToLog, "Posting Cancelled");
						for(int z=aryCharges.GetSize()-1;z>=0;z--) {
							delete (ChargeInfo*)aryCharges.GetAt(z);
							aryCharges.RemoveAt(z);
						}
						return FALSE;
					}
				}

				if(bSkipThisCharge) {
					//mark this charge as skipped
					// (j.jones 2011-06-02 09:47) - PLID 43931 - converted to use FindChargeInList
					//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge (though it should always be NULL here, as asserted above).
					NXDATALIST2Lib::IRowSettingsPtr pSearchRow = FindChargeInList(nChargeID, pEOBLineItemInfo, pOHIPEOBLineItemInfo, pAlbertaCharge);
					if(pSearchRow) {				
						pSearchRow->PutValue( COLUMN_SKIPPED, g_cvarTrue);

						// (j.jones 2012-04-18 17:11) - PLID 35306 - if a reversal,
						// toggle its reversed sibling as skipped as well
						BOOL bIsReversal = VarBool(pSearchRow->GetValue(COLUMN_REVERSAL), FALSE);
						//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
						if (bIsReversal && m_ERemitType != ertOhip) {
							ToggleSiblingRowSkipped(pSearchRow, TRUE);
						}
					}
					continue;
				}
			}
		}

		for(int j=0;j<aryCharges.GetSize();j++) {
			ChargeInfo *pCharge = (ChargeInfo*)aryCharges.GetAt(j);
			// (j.jones 2009-04-07 16:30) - PLID 33893 - in the rare case that nChargeID is -1,
			// split by PatientID, which we will ignore in the end if it too is -1
			// (j.jones 2011-06-02 10:02) - PLID 43931 - really we should never add these together 
			// if the nChargeID is -1, because as far as we know, they may really be separate charges
			// (NULL != NULL, etc.)
			if(nChargeID != -1 && pCharge->ID == nChargeID && pCharge->PatientID == nPatientID) {

				// (j.jones 2012-04-25 09:55) - PLID 35306 - don't add negative payments or adjustments
				if(cyInsPays > COleCurrency(0,0)) {
					pCharge->cyTotalPays += cyInsPays;
					pCharge->cyPaysToApply += cyInsPays;
				}

				// (j.jones 2016-04-19 09:49) - NX-100161 - we only need to track positive adjustments
				// when detecting if we have enough resp on the charge, as only positive adjustments
				// are actually applied to charges
				if(cyTotalPositiveAdj > COleCurrency(0, 0)) {
					pCharge->cyPositiveAdjustments += cyTotalPositiveAdj;
				}
				else if (cyTotalNegativeAdj < COleCurrency(0, 0)) {
					//we don't need to track the total negative adjustments,
					//we only need to reduce our apply amounts by them
					pCharge->cyPaysToApply += cyTotalNegativeAdj;
				}

				// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
				// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
				pCharge->cyOtherResp += VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP));
				//unlikely, but if it has now been determined to be a duplicate but wasn't before, update the flag
				// (j.jones 2009-04-07 14:55) - PLID 33862 - yeah "unlikely"... we removed this altogether,
				// if they didn't skip the duplicate, then just sum it up normally
				//if(!pCharge->bDuplicate)
					//pCharge->bDuplicate = bDuplicate;

				// (j.jones 2011-03-21 11:12) - PLID 42099 - track a list of adjustment reasons
				if(pCharge->strAdjReasons.Find(strAdjReasons) == -1) {
					pCharge->strAdjReasons += "\n";
					pCharge->strAdjReasons += strAdjReasons;
				}

				// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
				// to the given insurance resp. that we're about to void
				//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
				pCharge->cyReversedAmt += CalcAmountToBeReversed(nChargeID, nInsuredPartyID, pEOBLineItemInfo, pAlbertaCharge);

				bFound = TRUE;
			}
		}

		if(!bFound) {
			ChargeInfo *pCharge = new ChargeInfo;
			pCharge->ID = nChargeID;
			pCharge->PatientID = nPatientID;
			pCharge->InsuredPartyID = nInsuredPartyID;
			pCharge->nRespTypeID = nRespTypeID;
			// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
			pCharge->bSubmitAsPrimary = bSubmitAsPrimary;
			pCharge->cyCharge = VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0));
			pCharge->cyTotalPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));
			//initialize the apply amount to the total pay amount (they'll almost always end up identical)
			pCharge->cyPaysToApply = pCharge->cyTotalPays;
			
			// (j.jones 2016-04-19 09:49) - NX-100161 - we only need to track positive adjustments
			// when detecting if we have enough resp on the charge, as only positive adjustments
			// are actually applied to charges
			COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
			COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
			COleCurrency cyTotalAdjAmt = COleCurrency(0, 0);
			GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjAmt);
			pCharge->cyPositiveAdjustments = cyTotalPositiveAdj;

			// (j.jones 2012-04-25 09:55) - PLID 35306 - don't add negative payments
			if(pCharge->cyTotalPays < COleCurrency(0,0)) {
				pCharge->cyTotalPays = COleCurrency(0,0);
			}
			if (pCharge->cyPaysToApply < COleCurrency(0, 0)) {
				pCharge->cyPaysToApply = COleCurrency(0, 0);
			}
			if(pCharge->cyPositiveAdjustments < COleCurrency(0,0)) {
				//this should not be possible - come on, look at the variable name!
				ASSERT(FALSE);
				pCharge->cyPositiveAdjustments = COleCurrency(0,0);
			}
			if (cyTotalNegativeAdj > COleCurrency(0, 0)) {
				//this should also not be possible
				ASSERT(FALSE);
				cyTotalNegativeAdj = COleCurrency(0, 0);
			}

			//reduce the payment apply amount by the total of negative adjustments
			if (cyTotalNegativeAdj < COleCurrency(0, 0)) {
				pCharge->cyPaysToApply += cyTotalNegativeAdj;
			}

			// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
			// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
			pCharge->cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0));
			//pCharge->bDuplicate = bDuplicate;	// (j.jones 2009-04-07 15:04) - PLID 33862 - removed
			
			// (j.jones 2009-10-05 13:40) - PLID 21304 - grab information we need for potential
			// patient payment shifting (not necessary in the above code, this information does not stack)
			pCharge->cyPatPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_APPLIES), COleCurrency(0,0));
			pCharge->nBillID = VarLong(pLoopRowPtr->GetValue(COLUMN_BILL_ID), -1);
			pCharge->dtChargeDate = VarDateTime(pLoopRowPtr->GetValue(COLUMN_CHARGE_DATE), dtInvalid);
			pCharge->strServiceCode = strServiceCode;

			// (j.jones 2011-03-21 11:12) - PLID 42099 - track a list of adjustment reasons
			pCharge->strAdjReasons = strAdjReasons;

			// (j.jones 2011-06-02 09:51) - PLID 43931 - added pEOBLineItemInfo
			pCharge->pEOBLineItemInfo = pEOBLineItemInfo;
			pCharge->pOHIPEOBLineItemInfo = pOHIPEOBLineItemInfo;
			//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
			pCharge->pAlbertaCharge = pAlbertaCharge;

			// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
			// to the given insurance resp. that we're about to void
			//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
			pCharge->cyReversedAmt = CalcAmountToBeReversed(nChargeID, nInsuredPartyID, pEOBLineItemInfo, pAlbertaCharge);
			
			aryCharges.Add(pCharge);
		}
	}

	m_progress.SetPos(0);
	m_progress.SetRange(0,aryCharges.GetSize());
	m_progress.SetStep(1);

	int j=0;

	// (j.jones 2009-10-05 16:48) - PLID 21304 - try and rearrange patient payments
	// based on where the insurance company seems to think they should be applied
	BOOL bShiftPatientPays = GetRemotePropertyInt("ERemitShiftPatientPays", 1, 0, "<None>", true) == 1;

	if(bShiftPatientPays) {

		for(j=0;j<aryCharges.GetSize();j++) {

			ChargeInfo *pCharge = (ChargeInfo*)aryCharges.GetAt(j);

			// (j.jones 2010-12-16 10:42) - PLID 41176 - If bAutoSkipZeroPays is TRUE, and there is no payment,
			// no patient resp., but there is an adjustment, we will later skip this charge entirely, so don't
			// bother spending time trying to calculate patient payments on this ignored charge
			// (j.jones 2011-03-21 09:37) - PLID 42099 - removed this call because it is needless,
			// we won't auto-skip unless there is a $0.00 patient resp, and this code isn't fired unless
			// there is a patient resp > $0.00
			/*
			if(bAutoSkipZeroPays && pCharge->cyPays == COleCurrency(0,0) && pCharge->cyAdjs > COleCurrency(0,0) && pCharge->cyPatResp == COleCurrency(0,0)) {
				continue;
			}
			*/
			
			//calculate the EOB's "assumed balance" based on what they
			//are paying and adjusting
			COleCurrency cyAssumedBalance = pCharge->cyCharge - pCharge->cyPaysToApply - pCharge->cyPositiveAdjustments;

			// (j.jones 2012-04-19 10:13) - PLID 35306 - for takebacks, the result may be negative,
			// so handle that accordingly
			if(cyAssumedBalance < COleCurrency(0,0)) {
				cyAssumedBalance = COleCurrency(0,0);
			}
			else if(cyAssumedBalance > pCharge->cyCharge) {
				cyAssumedBalance = pCharge->cyCharge;
			}
			
			if(pCharge->cyPatPays > COleCurrency(0,0) && pCharge->cyPatPays > cyAssumedBalance) {
				
				//the current applies need to be reduced, but can they be moved to another charge?

				COleCurrency cyPatientPaymentOverage = pCharge->cyPatPays - cyAssumedBalance;
								
				// (j.jones 2013-04-26 10:26) - PLID 55570 - Loop until cyPatientPaymentOverage has been
				// fully reassigned to other charges. If we go through all charges and still have an overage
				// left, that's fine, it means we just can't move that remaining amount.
				for(int k=0; k<aryCharges.GetSize() && cyPatientPaymentOverage > COleCurrency(0,0); k++) {
					
					ChargeInfo *pChargeToCompare = (ChargeInfo*)aryCharges.GetAt(k);
					
					if(pChargeToCompare->ID != pCharge->ID
						&& pChargeToCompare->PatientID == pCharge->PatientID
						&& pChargeToCompare->nBillID == pCharge->nBillID) {

						//this is another charge on the same bill, can we shift the current patient payments to it?
						COleCurrency cyAssumedBalanceToCompare = pChargeToCompare->cyCharge - pChargeToCompare->cyPaysToApply - pChargeToCompare->cyPositiveAdjustments;

						// (j.jones 2012-04-19 10:13) - PLID 35306 - for takebacks, the result may be negative,
						// so handle that accordingly
						if(cyAssumedBalanceToCompare < COleCurrency(0,0)) {
							cyAssumedBalanceToCompare = COleCurrency(0,0);
						}
						else if(cyAssumedBalanceToCompare > pCharge->cyCharge) {
							cyAssumedBalanceToCompare = pCharge->cyCharge;
						}

						//now subtract patient pays, provided they are positive
						if(pChargeToCompare->cyPatPays > COleCurrency(0,0)) {
							cyAssumedBalanceToCompare -= pChargeToCompare->cyPatPays;
						}

						if(cyAssumedBalanceToCompare >= COleCurrency(0,0)) {

							//We have a winner! Before we state that we will move these payments,
							//check and make sure there's a balance available, of any responsibility.
							COleCurrency cyBalance = GetChargeTotalBalance(pChargeToCompare->ID);

							// (j.jones 2013-03-18 16:00) - PLID 55570 - Charges can now move patient payments
							// in partial amounts, instead of always moving them in their entirety.
							// However, if our patient pays is exactly what is needed on the other charge,
							// move the payment in its entirety, even if there is still some patient resp.
							// left on the current charge.
							COleCurrency cyPaysToShift = COleCurrency(0,0);
							if(pCharge->cyPatPays == cyAssumedBalanceToCompare) {
								//we will move the existing pays in their entirety
								cyPaysToShift = pCharge->cyPatPays;
							}
							else if(cyPatientPaymentOverage >= cyAssumedBalanceToCompare) {
								//we can only move some of our overage to this charge
								cyPaysToShift = cyAssumedBalanceToCompare;
							}
							else {
								//move the remaining overage
								cyPaysToShift = cyPatientPaymentOverage;
							}

							//we know how much we would like to move, but we can only move
							//what the destination charge has enough balance to accept
							if(cyBalance < cyPaysToShift) {
								cyPaysToShift = cyBalance;
							}

							if(cyPaysToShift > COleCurrency(0,0)) {
								//We can apply these payments (we will acquire responsibility later),
								//so mark that these payments need re-applied to this charge
								pChargeToCompare->cyPatPays += cyPaysToShift;
								pCharge->cyPatPays -= cyPaysToShift;

								ShiftPatientPaymentInfo *pShiftInfo = new ShiftPatientPaymentInfo;
								pShiftInfo->cyPatPaysToShift = cyPaysToShift;
								pShiftInfo->pShiftPatPaysToCharge = pChargeToCompare;
								pCharge->arypShiftPatientPaymentInfo.Add(pShiftInfo);
								
								//Reduce our overage by the amount shifted.
								//If there is still some balance left, we'll keep looping
								//through the rest of the charges to see if we can move the
								//remaining overage to another charge
								cyPatientPaymentOverage -= cyPaysToShift;
							}
						}
					}
				}
			}
		}
	}

	//now that we have the data loaded and combined, check it
	for(j=0;j<aryCharges.GetSize();j++) {

		ChargeInfo *pCharge = (ChargeInfo*)aryCharges.GetAt(j);

		long nPatientID = pCharge->PatientID;
		long nInsuredPartyID = pCharge->InsuredPartyID;
		long nRespTypeID = pCharge->nRespTypeID;
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		BOOL bSubmitAsPrimary = pCharge->bSubmitAsPrimary;
		long nChargeID = pCharge->ID;
		// (b.eyers 2016-04-15 12:11) - NX-100187 - get the name of the insurance company
		NXDATALIST2Lib::IRowSettingsPtr pSearchRow = FindChargeInList(nChargeID, pCharge->pEOBLineItemInfo, pCharge->pOHIPEOBLineItemInfo, pCharge->pAlbertaCharge);
		CString strPatInsCoName = VarString(pSearchRow->GetValue(COLUMN_INS_CO_NAME), "");

		CString strPatientName = GetExistingPatientName(pCharge->PatientID);
		long nPatientUserDefinedID = GetExistingPatientUserDefinedID(pCharge->PatientID); // (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid

		m_progress.StepIt();
		CString str;
		str.Format("Validating responsibilities for patient '%s'...", strPatientName);
		m_progressStatus.SetWindowText(str);

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_EOBDlg();

		if(nPatientID <= 0) {
			bBlankPatients = TRUE;
			continue;
		}

		COleCurrency cyPaymentAmtToApply = pCharge->cyPaysToApply;
		COleCurrency cyPositiveAdjustmentAmt = pCharge->cyPositiveAdjustments;
		COleCurrency cyOtherResp = pCharge->cyOtherResp;
		COleCurrency cyReversedAmt = pCharge->cyReversedAmt;

		// (j.jones 2009-10-05 17:40) - PLID 21304 - silently shift patient payments as directed,
		// but log that we did so
		if(bShiftPatientPays) {	

			// (j.jones 2013-03-18 16:34) - PLID 55570 - now we can move payments to multiple destination charges
			for(int c=0;c<pCharge->arypShiftPatientPaymentInfo.GetSize();c++) {

				ShiftPatientPaymentInfo *pShiftPaymentInfo = (ShiftPatientPaymentInfo*)pCharge->arypShiftPatientPaymentInfo.GetAt(c);
				if(pShiftPaymentInfo == NULL) {
					//wouldn't this be bad data?
					ASSERT(FALSE);
					continue;
				}

				long nShiftPaysToChargeID = pShiftPaymentInfo->pShiftPatPaysToCharge->ID;
				long nShiftPaysPatientID = pShiftPaymentInfo->pShiftPatPaysToCharge->PatientID;
				COleCurrency cyPatPaysToShift = pShiftPaymentInfo->cyPatPaysToShift;

				//this function will unapply enough responsibility as we require, correcting closed payments if needed
				CArray<UnappliedAmount, UnappliedAmount> aryUnappliedPatPaymentIDs;
				UnapplyFromCharge(pCharge->ID, -1, FALSE, cyPatPaysToShift, &aryUnappliedPatPaymentIDs, TRUE);

				//make sure we have enough patient balance on the destination charge
				COleCurrency cyPatientBalance = GetChargePatientBalance(nShiftPaysToChargeID, nShiftPaysPatientID);

				if(cyPatientBalance < cyPatPaysToShift) {
					
					COleCurrency cyBalanceToAcquire = cyPatPaysToShift - cyPatientBalance;

					//we know that there is enough balance to support cyBalanceToAcquire, 
					//but it can be spread across several responsibilities.
					//It's time to go hunting.
					
					//first loop through the insurance resps, in order
					_RecordsetPtr rsResps = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT "
						"WHERE PatientID = {INT} AND RespTypeID <> -1 "
						"ORDER BY RespTypeID; "
						"SELECT PersonID FROM InsuredPartyT "
						"WHERE PatientID = {INT} AND RespTypeID = -1; ",
						nShiftPaysPatientID, nShiftPaysPatientID);

					while(!rsResps->eof && cyBalanceToAcquire > COleCurrency(0,0)) {

						long nCurInsuredPartyID = AdoFldLong(rsResps, "PersonID",-1);
						COleCurrency cyCurRespBalance = GetChargeInsBalance(nShiftPaysToChargeID, nShiftPaysPatientID, nCurInsuredPartyID);
						if(cyCurRespBalance > COleCurrency(0,0)) {

							// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
							// E-Remittance because it also uses a unique audit type

							//there is a balance, let's steal it
							if(cyCurRespBalance >= cyBalanceToAcquire) {
								//we have enough, so take what we need
								ShiftInsuranceResponsibility(nShiftPaysToChargeID, nShiftPaysPatientID,
									nCurInsuredPartyID, -1, "Charge", cyBalanceToAcquire,
									"in order to move an existing patient payment",
									COleDateTime::GetCurrentTime(), -1, -1, TRUE);
								cyBalanceToAcquire = COleCurrency(0,0);
							}
							else {
								//we don't have enough, so take all the balance
								ShiftInsuranceResponsibility(nShiftPaysToChargeID, nShiftPaysPatientID,
									nCurInsuredPartyID, -1, "Charge", cyCurRespBalance, 
									"in order to move an existing patient payment",
									COleDateTime::GetCurrentTime(), -1, -1, TRUE);
								cyBalanceToAcquire -= cyCurRespBalance;
							}
						}

						rsResps->MoveNext();
					}
					rsResps->Close();
				}

				COleCurrency cyRemainingPaysToShift = cyPatPaysToShift;
				for(int u=aryUnappliedPatPaymentIDs.GetSize()-1;u>=0 && cyRemainingPaysToShift > COleCurrency(0,0);u--) {
					UnappliedAmount ua = (UnappliedAmount)aryUnappliedPatPaymentIDs.GetAt(u);

					if(ua.cyAmtUnapplied <= COleCurrency(0,0)) {
						aryUnappliedPatPaymentIDs.RemoveAt(u);
						continue;
					}

					COleCurrency cy = COleCurrency(0,0);
					long nReappliedPaymentID = ua.nPaymentID;

					if(ua.cyAmtUnapplied > cyRemainingPaysToShift) {
						cy = cyRemainingPaysToShift;
						ua.cyAmtUnapplied -= cy;
					}
					else {
						cy = ua.cyAmtUnapplied;
						//this payment is now used up, remove it
						aryUnappliedPatPaymentIDs.RemoveAt(u);
					}
						
					//apply this amount to the assigned charge ID (this function will audit)
					// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables
					// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
					ApplyPayToBill(nReappliedPaymentID, nShiftPaysPatientID, cy, "Charge", nShiftPaysToChargeID, -1, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE);

					//decrease the amt. we need to shift
					cyRemainingPaysToShift -= cy;
				}

				//this should be zero now
				ASSERT(cyRemainingPaysToShift == COleCurrency(0,0));

				//shift the amount we unapplied from the old charge to the insurance we're about to pay
				if(pCharge->InsuredPartyID != -1) {
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(pCharge->ID, pCharge->PatientID,
						-1, pCharge->InsuredPartyID, "Charge", cyPatPaysToShift,
						"after moving an existing patient payment",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
				}

				//log this for each apply
				CString strDestServiceCode = pShiftPaymentInfo->pShiftPatPaysToCharge->strServiceCode;
				COleDateTime dtDestChargeDate = pShiftPaymentInfo->pShiftPatPaysToCharge->dtChargeDate;
				COleCurrency cyDestChargeAmount = pShiftPaymentInfo->pShiftPatPaysToCharge->cyCharge;
				CString strWarningToLog;
				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				strWarningToLog.Format("For patient '%s' [Patient ID: %li], the EOB has indicated that %s in patient payments applied\n"
					"to the charge '%s' (Date Of Service: %s, Charge Amount: %s)\n"
					"should be applied to the charge '%s' (Date Of Service: %s, Charge Amount: %s) instead.\n"
					"Practice has reapplied %s in patient payments to the charge for '%s'.",
					GetExistingPatientName(pCharge->PatientID),nPatientUserDefinedID, FormatCurrencyForInterface(cyPatPaysToShift),
					pCharge->strServiceCode, FormatDateTimeForInterface(pCharge->dtChargeDate, NULL, dtoDate), FormatCurrencyForInterface(pCharge->cyCharge),
					strDestServiceCode, FormatDateTimeForInterface(dtDestChargeDate, NULL, dtoDate), FormatCurrencyForInterface(cyDestChargeAmount),
					FormatCurrencyForInterface(cyPatPaysToShift), strDestServiceCode);

				AddWarningToLog(strWarningToLog, "");

			} //end pCharge->arypShiftPatientPaymentInfo loop
		}

		//EnsureValidResponsibilities will check for 1, 2, and 3, prompt the user,
		//and set up correct responsibility balances for our processing to go through

		//returns ePassed, eFailed, or eSkipped
		// (j.jones 2009-04-07 15:04) - PLID 33862 - removed bDuplicate
		// (j.jones 2010-02-08 11:53) - PLID 37182 - added bIsPrimaryIns
		// (j.jones 2010-09-02 11:12) - PLID 38787 - this needs to acknowledge the "submit as primary" setting
		BOOL bIsPrimaryIns = (nRespTypeID == 1 || bSubmitAsPrimary);

		// (j.jones 2012-04-24 15:51) - PLID 35306 - Added reversed amount, so we know how much is currently applied to the
		// given insurance resp. that we're about to void. This is a positive value, not negative.
		// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
		ERespStatus eRespStatus = EnsureValidResponsibilities(nPatientID, nPatientUserDefinedID, nChargeID, nInsuredPartyID, bIsPrimaryIns, cyPaymentAmtToApply, cyPositiveAdjustmentAmt, cyOtherResp, cyReversedAmt, strPatInsCoName);
		if(eRespStatus == eFailed) {
			for(int z=aryCharges.GetSize()-1;z>=0;z--) {
				delete (ChargeInfo*)aryCharges.GetAt(z);
				aryCharges.RemoveAt(z);
			}
			return FALSE;
		}
		else if(eRespStatus == eSkipped) {
			// (s.tullis 2016-04-15 16:00) - NX-100211
			//find the row for this charge
			// (j.jones 2011-06-02 09:47) - PLID 43931 - converted to use FindChargeInList
			//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
			NXDATALIST2Lib::IRowSettingsPtr pSearchRow = FindChargeInList(nChargeID, pCharge->pEOBLineItemInfo, pCharge->pOHIPEOBLineItemInfo, pCharge->pAlbertaCharge);
			if(pSearchRow) {
				//mark this charge as skipped
				pSearchRow->PutValue(COLUMN_SKIPPED, g_cvarTrue);

				// (j.jones 2012-04-18 17:11) - PLID 35306 - if a reversal,
				// toggle its reversed sibling as skipped as well
				BOOL bIsReversal = VarBool(pSearchRow->GetValue(COLUMN_REVERSAL), FALSE);
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (bIsReversal && m_ERemitType != ertOhip) {
					ToggleSiblingRowSkipped(pSearchRow, TRUE);
				}

				// (j.jones 2009-07-01 16:55) - PLID 34778 - since this charge has been skipped,
				// we must now update the overpayment label
				// (j.jones 2009-07-07 15:20) - PLID 34805 - no we don't, we will not change
				// the overage amount based on what is skipped
				//UpdateOverpaymentLabel();
			}
		}
	}

	if(bBlankPatients) {
		if(IDNO == MessageBox("There is at least one record in this EOB that does not reconcile with a patient in the system.\n"
			"Please compare with the generated EOB.txt file in order to identify this discrepancy.\n"
			"If you continue, the lines without patients will be skipped, which may cause the payment to not balance out.\n"
			"Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

			for(int z=aryCharges.GetSize()-1;z>=0;z--) {
				delete (ChargeInfo*)aryCharges.GetAt(z);
				aryCharges.RemoveAt(z);
			}

			return FALSE;
		}
	}

	for(int z=aryCharges.GetSize()-1;z>=0;z--) {
		delete (ChargeInfo*)aryCharges.GetAt(z);
		aryCharges.RemoveAt(z);
	}

	return TRUE;
}


// (j.jones 2013-07-09 15:56) - PLID 55573 - Renamed to reflect that this will now will also verify
// balances are what we expected them to be.
void CEOBDlg::CheckShiftAndVerifyBalances()
{

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);

	// (j.jones 2013-07-09 16:04) - PLID 55573 - We now do a check in this function to ensure
	// the balances are what we expected them to be, which means we always need to loop
	// through the charges even if the shifting combo box says not to shift anything.
	
	long nShiftMethod = 0; //0 means do not shift
	if(m_ShiftRespCombo->CurSel != -1) {
		nShiftMethod = VarLong(m_ShiftRespCombo->GetValue(m_ShiftRespCombo->CurSel,0),0);
	}

	m_progress.SetPos(0);
	m_progress.SetRange(0,(short)m_EOBList->GetRowCount());

	//now, for each charge...

	CString str = "Preparing to shift balances...";
	if(nShiftMethod == 0) {
		// (j.jones 2013-07-09 15:56) - PLID 55573 - if not shifting, give a more accurate description
		// of what shenanigans we're currently up to
		str = "Preparing to verify balances...";
	}
	Log(str);
	m_progressStatus.SetWindowText(str);

	//Store the charge ID, charge amount, and total payment, adjustment, pat. resp
	CPtrArray aryCharges;
	// (s.tullis 2016-04-15 16:00) - NX-100211
	NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr;
	for(pLoopRowPtr = m_EOBList->GetFirstRow();pLoopRowPtr!= nullptr;pLoopRowPtr = pLoopRowPtr->GetNextRow()) {

		long nChargeID = VarLong(pLoopRowPtr->GetValue(COLUMN_CHARGE_ID));

		// (j.jones 2010-01-12 11:38) - PLID 36664 - if we could not find a charge,
		// then we shouldn't be trying to shift it
		if(nChargeID == -1) {
			continue;
		}
		
		//ignore the skipped rows, obviously!
		if(VarBool(pLoopRowPtr->GetValue(COLUMN_SKIPPED)))
			continue;

		BOOL bFound = FALSE;
		long nPatientID = VarLong(pLoopRowPtr->GetValue(COLUMN_PATIENT_ID));
		long nInsuredPartyID = VarLong(pLoopRowPtr->GetValue(COLUMN_INSURED_PARTY_ID));
		// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
		long nRespTypeID = VarLong(pLoopRowPtr->GetValue(COLUMN_RESP_TYPE_ID));
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		BOOL bSubmitAsPrimary = VarBool(pLoopRowPtr->GetValue( COLUMN_SUBMIT_AS_PRIMARY), FALSE);
		BOOL bDuplicate = VarLong(pLoopRowPtr->GetValue(COLUMN_DUPLICATE)) ? TRUE : FALSE;

		for(int j=0;j<aryCharges.GetSize();j++) {
			ChargeInfo *pCharge = (ChargeInfo*)aryCharges.GetAt(j);
			// (j.jones 2011-06-02 10:02) - PLID 43931 - really we should never add these together 
			// if the nChargeID is -1, because as far as we know, they may really be separate charges
			// (NULL != NULL, etc.)
			if(nChargeID != -1 && pCharge->ID == nChargeID) {
				
				// (j.jones 2012-04-25 09:55) - PLID 35306 - don't add negative payments or adjustments
				COleCurrency cyInsPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));
				if(cyInsPays > COleCurrency(0,0)) {
					pCharge->cyTotalPays += cyInsPays;
					pCharge->cyPaysToApply += cyInsPays;
				}
				// (j.jones 2016-04-19 09:49) - NX-100161 - we only need to track positive adjustments
				// when detecting if we have enough resp on the charge, as only positive adjustments
				// are actually applied to charges
				COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
				COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
				COleCurrency cyTotalAdjAmt = COleCurrency(0, 0);
				GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjAmt);

				if(cyTotalPositiveAdj > COleCurrency(0,0)) {
					pCharge->cyPositiveAdjustments += cyTotalPositiveAdj;
				}

				// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
				// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
				pCharge->cyOtherResp += VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0));

				// (j.jones 2011-03-21 11:12) - PLID 42099 - track a list of adjustment reasons
				CString strAdjReason = VarString(pLoopRowPtr->GetValue(COLUMN_ADJ_REASON), "");
				if(pCharge->strAdjReasons.Find(strAdjReason) == -1) {
					pCharge->strAdjReasons += "\n";
					pCharge->strAdjReasons += strAdjReason;
				}

				// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
				// to the given insurance resp. that we're about to void
				_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
				//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
				EOBLineItemInfo *pEOBLineItemInfo = NULL;
				AlbertaAssessments::ChargeInfoPtr pAlbertaCharge = NULL;
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertAnsi && var.vt == VT_I4) {
					pEOBLineItemInfo = (EOBLineItemInfo*)VarLong(var);
				}
				else if (m_ERemitType == ertAlberta && var.vt == VT_I4) {
					pAlbertaCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
				}
				//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
				pCharge->cyReversedAmt += CalcAmountToBeReversed(nChargeID, nInsuredPartyID, pEOBLineItemInfo, pAlbertaCharge);

				bFound = TRUE;
			}
		}

		if(!bFound) {
			ChargeInfo *pCharge = new ChargeInfo;
			pCharge->ID = nChargeID;
			pCharge->PatientID = nPatientID;
			pCharge->InsuredPartyID = nInsuredPartyID;
			pCharge->cyCharge = VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0));
			
			pCharge->cyTotalPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));
			//initialize the apply amount to the total pay amount (they'll almost always end up identical)
			pCharge->cyPaysToApply = pCharge->cyTotalPays;
			
			// (j.jones 2016-04-19 09:49) - NX-100161 - we only need to track positive adjustments
			// when detecting if we have enough resp on the charge, as only positive adjustments
			// are actually applied to charges
			COleCurrency cyTotalPositiveAdj = COleCurrency(0, 0);
			COleCurrency cyTotalNegativeAdj = COleCurrency(0, 0);
			COleCurrency cyTotalAdjAmt = COleCurrency(0, 0);
			GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjAmt);

			pCharge->cyPositiveAdjustments = cyTotalPositiveAdj;

			// (j.jones 2012-04-25 09:55) - PLID 35306 - don't add negative payments or adjustments
			if(pCharge->cyTotalPays < COleCurrency(0,0)) {
				pCharge->cyTotalPays = COleCurrency(0,0);
			}
			if (pCharge->cyPaysToApply < COleCurrency(0, 0)) {
				pCharge->cyPaysToApply = COleCurrency(0, 0);
			}
			if(pCharge->cyPositiveAdjustments < COleCurrency(0,0)) {
				//this should not be possible - come on, look at the variable name!
				ASSERT(FALSE);
				pCharge->cyPositiveAdjustments = COleCurrency(0,0);
			}
			if (cyTotalNegativeAdj > COleCurrency(0, 0)) {
				//this should also not be possible
				ASSERT(FALSE);
				cyTotalNegativeAdj = COleCurrency(0, 0);
			}
			
			//reduce the payment apply amount by the total of negative adjustments
			if (cyTotalNegativeAdj < COleCurrency(0, 0)) {
				pCharge->cyPaysToApply += cyTotalNegativeAdj;
			}

			// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
			// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
			pCharge->cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0));
			//pCharge->bDuplicate = bDuplicate; // (j.jones 2009-04-07 15:04) - PLID 33862 - removed
			
			// (j.jones 2009-10-05 16:57) - PLID 21304 - new fields that are not needed for this code
			pCharge->cyPatPays = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_APPLIES), COleCurrency(0,0));
			pCharge->nBillID = VarLong(pLoopRowPtr->GetValue(COLUMN_BILL_ID), -1);
			pCharge->dtChargeDate = VarDateTime(pLoopRowPtr->GetValue(COLUMN_CHARGE_DATE), dtInvalid);
			pCharge->strServiceCode = VarString(pLoopRowPtr->GetValue(COLUMN_CPT_CODE), "");

			// (j.jones 2011-03-21 11:12) - PLID 42099 - track a list of adjustment reasons
			pCharge->strAdjReasons = VarString(pLoopRowPtr->GetValue( COLUMN_ADJ_REASON), "");

			// (j.jones 2011-06-02 09:51) - PLID 43931 - added pEOBLineItemInfo
			pCharge->pEOBLineItemInfo = NULL;
			pCharge->pOHIPEOBLineItemInfo = NULL;
			_variant_t var = pLoopRowPtr->GetValue(COLUMN_CHARGE_PTR);
			if(var.vt == VT_I4) {
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip) {
					pCharge->pOHIPEOBLineItemInfo = (OHIPEOBLineItemInfo*)VarLong(var);
				}
				else if (m_ERemitType == ertAnsi) {
					pCharge->pEOBLineItemInfo = (EOBLineItemInfo*)VarLong(var);
				}
				else {
					//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
					ASSERT(m_ERemitType == ertAlberta);
					pCharge->pAlbertaCharge = m_AlbertaParser.GetChargeFromRawPointer((AlbertaAssessments::ChargeInfo*)VarLong(var));
			}
			}

			// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
			// to the given insurance resp. that we're about to void
			//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
			pCharge->cyReversedAmt = CalcAmountToBeReversed(nChargeID, nInsuredPartyID, pCharge->pEOBLineItemInfo, pCharge->pAlbertaCharge);

			aryCharges.Add(pCharge);
		}
	}

	for(int j=0;j<aryCharges.GetSize();j++) {

		ChargeInfo *pCharge = (ChargeInfo*)aryCharges.GetAt(j);

		m_progress.StepIt();

		CString strPatientName = GetExistingPatientName(pCharge->PatientID);
		long nPatientUserDefinedID = GetExistingPatientUserDefinedID(pCharge->PatientID); // (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid

		if(nShiftMethod != 0) {	//0 means do not shift
			str.Format("Shifting balances for patient '%s'...", strPatientName);
		}
		else {
			// (j.jones 2013-07-09 15:56) - PLID 55573 - if not shifting, give a more accurate description
			// of what shenanigans we're currently up to
			str.Format("Verifying balances for patient '%s'...", strPatientName);
		}
		Log(str);
		m_progressStatus.SetWindowText(str);
		
		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_EOBDlg();
		
		if(nShiftMethod != 0) {	//0 means do not shift
			// (j.jones 2013-03-18 13:51) - PLID 55574 - We no longer pass in paid/adjusted amounts,
			// instead it is the caller's responsibility to calculate bChargeWasPosted, which is true if
			// we did post a payment > $0.00, or an adjustment > $0.00, or if the reported patient resp
			//('other' resp) is > $0.00. TryShiftBalances uses this information alongside the checkbox
			//to 'only shift posted charges'.
			
			//This looks at the total pays, not applied pays, because it still means the charge was 'paid'.
			BOOL bChargeWasPosted = (pCharge->cyTotalPays > COleCurrency(0,0) || pCharge->cyPositiveAdjustments > COleCurrency(0,0) || pCharge->cyOtherResp > COleCurrency(0,0));
			TryShiftBalances(pCharge->PatientID, pCharge->nBillID, pCharge->ID, pCharge->InsuredPartyID, bChargeWasPosted);
		}

		// (j.jones 2013-07-09 15:56) - PLID 55573 - Verify that the amount stated to be 'patient resp'
		// in the EOB exists as a resp. for an insurance other than this one. Remember, "patient resp"
		// is tracked as "other resp" as this translates to "money this payer is not going to cover".
		// If they didn't shift, then they're likely to get this warning on most charges.
		if(pCharge->cyOtherResp > COleCurrency(0,0)) {
			//This function gets the current responsibility (not the balance) of patient resp. and
			//other insurance resps. aside from the one that is currently paying.
			//The total should ideally never be less than this charge's "other" resp.
			COleCurrency cyTotalPatientAndOtherResps = GetChargePatientAndOtherRespTotals(pCharge->ID, pCharge->InsuredPartyID);
			//NOTE: if we ever decide to change this to be != instead of <, make sure we skip the warning
			//if the reported amount is < than the total other resp but == the real patient resp.
			if(cyTotalPatientAndOtherResps < pCharge->cyOtherResp) {
				CString strItemDesc;
				_RecordsetPtr rs = CreateParamRecordset("SELECT ItemCode, Description FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.ID = {INT}", pCharge->ID);
				if(!rs->eof) {
					strItemDesc.Format("%s - %s", AdoFldString(rs, "ItemCode",""), AdoFldString(rs, "Description",""));
				}
				rs->Close();

				CString strLog;
				strLog.Format("The charge for patient '%s' [Patient ID: %li], Date Of Service: %s, Charge Amount: %s\n"
					"for %s\n"
					"now has a total of %s in patient and/or other insurance responsibilities.\n"
					"However, the EOB has indicated that there should be %s in patient/other responsibilities on this charge.\n\n"
					"Please review this patient's account to ensure that the remaining responsibilities are correct.",
					strPatientName,nPatientUserDefinedID, // (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
					FormatDateTimeForInterface(pCharge->dtChargeDate, dtoDate), FormatCurrencyForInterface(pCharge->cyCharge, TRUE, TRUE),
					strItemDesc,
					FormatCurrencyForInterface(cyTotalPatientAndOtherResps, TRUE, TRUE),
					FormatCurrencyForInterface(pCharge->cyOtherResp, TRUE, TRUE));
				AddWarningToLog(strLog, "");
			}
		}
	}

	for(int z=aryCharges.GetSize()-1;z>=0;z--) {
		delete (ChargeInfo*)aryCharges.GetAt(z);
		aryCharges.RemoveAt(z);
	}
}

// (j.jones 2009-03-05 11:32) - PLID 33235 - supported adding Billing Notes
// (j.armen 2012-02-20 11:09) - PLID 48242 - Parameratized
// (j.jones 2016-04-19 17:33) - NX-100246 - we now return the new payment ID
// we now specify a separate amount to apply, to cover the case where the total payment we make is not fully applied,
// such as when negative adjustments are present
long CEOBDlg::ApplyPayment(const long &nBatchPaymentID, const long &nPatientID, const long &nPatientUserDefinedID,
	const long &nChargeID, const long &nInsuredPartyID,
	const COleCurrency &cyTotalPaymentAmt, const COleCurrency &cyPaymentAmtToApply,
	const CString &strBillingNote, const CString &strClaimNumber)
{
	////////////////////////////
	//Step 1. Create the Payment

	long nLocationID;
	CString strDescription, strCheckNo;

	_variant_t vtProviderID = g_cvarNull;
	COleDateTime dtPaymentDate;
	long nPaymentGroupID;

	_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM BatchPaymentsT WHERE ID = {INT}", nBatchPaymentID);
	if(!rs->eof) {
		nLocationID = AdoFldLong(rs, "Location");
		nPaymentGroupID = AdoFldLong(rs, "PayCatID",-1);
		strDescription = AdoFldString(rs, "Description","(no description)");

		// (j.jones 2014-04-21 09:11) - PLID 61761 - assign the ProviderID directly to the variant,
		// so it can properly be NULL or a long
		vtProviderID = rs->Fields->Item["ProviderID"]->Value;

		strCheckNo = AdoFldString(rs, "CheckNo","");
		dtPaymentDate = AdoFldDateTime(rs, "Date");
	}

	CParamSqlBatch sqlBatch;

	sqlBatch.Declare("SET NOCOUNT ON");
	sqlBatch.Declare("BEGIN TRAN");

	// (j.jones 2010-05-18 14:26) - PLID 37752 - before we do anything else, create hold locks on everything
	//TES 1/31/2014 - PLID 60578 - Took out all these locks, except for Notes since its still not identitated.
	if(!strBillingNote.IsEmpty()) {
		sqlBatch.Declare(
			"SELECT TOP 0 * FROM Notes WITH (TABLOCK, HOLDLOCK)");
	}
	
	sqlBatch.Declare("DECLARE @nLineItemID INT");
	sqlBatch.Declare("DECLARE @nPaymentUniqueID INT");
	sqlBatch.Declare("DECLARE @nProviderID INT");
	sqlBatch.Declare("DECLARE @nLocationID INT");
	if(!strBillingNote.IsEmpty()) {
		sqlBatch.Declare("DECLARE @newNoteID INT");
	}

	// (j.jones 2009-06-04 11:07) - PLID 34341 - if we are applying to a charge and our preference says to use the charge provider
	// on the payment, then assign the charge provider ID now
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	if(nChargeID != -1 && GetRemotePropertyInt("ERemitUseChargeProviderOnApplies", 1, 0, "<None>", true) == 1) {
		//TES 1/31/2014 - PLID 60578 - Took out unnecessary locks
		sqlBatch.Add("SET @nProviderID = (SELECT DoctorsProviders FROM ChargesT WHERE ID = {INT})", nChargeID);
		//an unselected provider on a charge is -1, but on a payment it is NULL
		sqlBatch.Declare("IF @nProviderID = -1 BEGIN SET @nProviderID = NULL END");
	}
	else {
		//otherwise use the provider ID from the batch payment, which we have already loaded
		sqlBatch.Add("SET @nProviderID = {VT_I4}", vtProviderID);
	}

	// (j.jones 2011-07-14 16:11) - PLID 41440 - if we are applying to a charge and our preference says to use the charge location
	// on the payment, then assign the charge location ID now
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	if(nChargeID != -1 && GetRemotePropertyInt("ERemitUseChargeLocationOnApplies", 1, 0, "<None>", true) == 1) {
		//TES 1/31/2014 - PLID 60578 - Took out unnecessary locks
		sqlBatch.Add("SET @nLocationID = (SELECT LocationID FROM LineItemT WHERE ID = {INT})", nChargeID);
	}
	else {
		//otherwise use the location ID from the batch payment, which we have already loaded
		sqlBatch.Add("SET @nLocationID = {INT}", nLocationID);
	}

	// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
	sqlBatch.Add( 
		"INSERT INTO LineItemT (\r\n"
		"	PatientID, LocationID, Type, Date, InputName,\r\n"
		"	InputDate, Amount, Description\r\n"
		") VALUES (\r\n"
		"	{INT}, @nLocationID, 1, dbo.AsDateNoTime({OLEDATETIME}), {STRING},\r\n"
		"	GetDate(), {OLECURRENCY}, {STRING})", 
		nPatientID, dtPaymentDate, GetCurrentUserName(), 
		cyTotalPaymentAmt, strDescription);

	sqlBatch.Declare(
		"SET @nLineItemID = SCOPE_IDENTITY() ");

	// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
	sqlBatch.Declare(
		"INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
		"SET @nPaymentUniqueID = SCOPE_IDENTITY()");

	// (j.jones 2007-03-28 10:58) - PLID 25385 - supported CashReceived
	// (d.singleton 2014-10-15 14:35) - PLID 62698 - added claimnumber
	sqlBatch.Add(
		"INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, BatchPaymentID, PaymentUniqueID, CashReceived, ClaimNumber) VALUES "
		"(@nLineItemID, {INT}, @nProviderID, {INT}, 2, {INT},@nPaymentUniqueID, {OLECURRENCY}, {STRING})", 
		nInsuredPartyID, nPaymentGroupID, nBatchPaymentID, cyTotalPaymentAmt, strClaimNumber);
	sqlBatch.Add("INSERT INTO PaymentPlansT (ID, CheckNo) VALUES (@nLineItemID, {STRING})", strCheckNo);

	// (j.jones 2009-03-05 11:32) - PLID 33235 - supported creating billing notes
	// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
	// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
	if(!strBillingNote.IsEmpty()) {
		sqlBatch.Add( 
			"INSERT INTO Notes (PersonID, Date, UserID, Note) VALUES "
			"({INT}, GetDate(), {INT}, {STRING}) ",
			nPatientID, GetCurrentUserID(), strBillingNote);
		sqlBatch.Declare("SET @newNoteID = SCOPE_IDENTITY()");
		sqlBatch.Declare("INSERT INTO NoteInfoT (NoteID, LineItemID) VALUES (@newNoteID, @nLineItemID)");
	}

	sqlBatch.Declare("COMMIT TRAN");
	sqlBatch.Declare("SET NOCOUNT OFF");
	sqlBatch.Declare("SELECT @nLineItemID AS NewPaymentID");

	_RecordsetPtr prsPayment = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());

	long nLineItemID = -1;

	while(prsPayment->GetState() == adStateClosed || prsPayment->eof) {

		_variant_t varNull;
		prsPayment = prsPayment->NextRecordset(&varNull);
	}

	if(!prsPayment->eof) {
		nLineItemID = AdoFldLong(prsPayment, "NewPaymentID",-1);
	}
	prsPayment->Close();

	long nAuditID = BeginNewAuditEvent();
	CString strAuditDesc;
	strAuditDesc.Format("%s Payment", FormatCurrencyForInterface(cyTotalPaymentAmt, TRUE, TRUE));
	// (j.jones 2011-03-21 17:04) - PLID 24273 - changed to use a unique E-Remit audit
	AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPaymentCreatedByERemit, nLineItemID, "", strAuditDesc, aepHigh, aetCreated);

	AutoUpdateBatchPaymentDepositDates(nBatchPaymentID);

	///////////////////////////
	//Step 2. Apply the Payment

	// (j.jones 2009-02-13 16:44) - PLID 33081 - skip trying to apply if the balance is completely zero
	// and our payment is not zero
	// (j.jones 2009-04-07 16:34) - PLID 33893 - it is possible, in rare circumstances, to not have a charge ID,
	// in which case we simply don't apply it
	if(nChargeID != -1) {
		COleCurrency cyInsBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);
		//this is a passed in value to apply, which is often the full payment amount,
		//but can be less if there are negative adjustments
		if(cyInsBalance > COleCurrency(0,0) || cyPaymentAmtToApply == COleCurrency(0,0)) {
			// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables
			// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
			ApplyPayToBill(nLineItemID, nPatientID, cyPaymentAmtToApply, "Charge", nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE);
		}
	}
	else {
		// (j.jones 2011-06-02 09:28) - PLID 43931 - warn that this payment is left unapplied
		CString strLog;
		// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
		strLog.Format("For patient '%s' [Patient ID: %li], Practice will post %s in insurance payments\n"
			"unapplied on the patient's account, because no charge was found for this posting.",
			GetExistingPatientName(nPatientID),nPatientUserDefinedID, FormatCurrencyForInterface(cyTotalPaymentAmt, TRUE, TRUE));

		AddWarningToLog(strLog, "Payment Posted, Not Applied");
	}

	// (j.jones 2016-04-19 17:33) - NX-100246 - we now return the new payment ID
	return nLineItemID;
}

// (j.jones 2006-11-16 12:04) - PLID 23551 - added parameters for GroupCode and ReasonCode
// (j.jones 2008-02-20 11:30) - PLID 29007 - added nAdjustmentCategoryID and strGlobalAdjustmentDescription
// (j.jones 2009-06-09 14:35) - PLID 34539 - required the BatchPaymentID
// (j.jones 2010-04-09 12:10) - PLID 31309 - added dtAdjDate
// (j.armen 2012-02-20 11:10) - PLID 48243 - Parameratized
// (d.singleton 2014-10-15 14:35) - PLID 62698 - added claimnumber
// (j.jones 2016-04-19 17:33) - NX-100246 - added nNewPaymentID, the ID from ApplyPayment, which negative adjustments will apply to
void CEOBDlg::ApplyAdjustment(const long& nBatchPaymentID, const long& nPatientID,  const long&  nPatientUserDefinedID, const long& nChargeID, const long& nInsuredPartyID, 
	const COleCurrency& cyAdjustmentAmt, const COleDateTime& dtAdjDate, const CString& strGroupCode, 
	const CString& strReasonCode, const CString& strIndivAdjustmentDescription, const long& nAdjustmentCategoryID, 
	const CString& strGlobalAdjustmentDescription, const CString& strClaimNumber,
	const long& nNewPaymentID) {

	///////////////////////////////
	//Step 1. Create the Adjustment

	// (j.jones 2008-02-20 11:40) - PLID 29007 - we now pass in an adjustment description,
	// which may then potentially have strIndivAdjustmentDescription appended to it
	CString strDescription = strGlobalAdjustmentDescription;

	if(GetRemotePropertyInt("UseEOBAdjustmentReasons",1,0,"<None>",true) == 1
		&& !strIndivAdjustmentDescription.IsEmpty()) {

		strDescription += " - " + strIndivAdjustmentDescription;
	}

	// (j.jones 2009-03-30 11:50) - PLID 33739 - ensure we do not save a description that is too long
	if(strDescription.GetLength() > 255) {
		strDescription = strDescription.Left(252);
		strDescription += "...";
	}

	const long nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0));
	const long nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0));

	// (j.jones 2010-09-23 16:16) - PLID 40653 - group and reason codes are now IDs,
	// we have to load the IDs before saving
	long nGroupCodeID = -1, nReasonCodeID = -1;
	if(!strGroupCode.IsEmpty() || !strReasonCode.IsEmpty()) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM AdjustmentCodesT WHERE Type = 1 AND Code = {STRING}\n "
			"SELECT ID FROM AdjustmentCodesT WHERE Type = 2 AND Code = {STRING}", strGroupCode, strReasonCode);
		if(!rs->eof && !strGroupCode.IsEmpty()) {
			nGroupCodeID = AdoFldLong(rs, "ID");
		}
		rs = rs->NextRecordset(NULL);
		if(!rs->eof && !strReasonCode.IsEmpty()) {
			nReasonCodeID = AdoFldLong(rs, "ID");
		}
	}

	const _variant_t vtGroupCodeID = (nGroupCodeID == -1 ? g_cvarNull : nGroupCodeID);
	const _variant_t vtReasonCodeID = (nReasonCodeID == -1 ? g_cvarNull : nReasonCodeID);

	CParamSqlBatch sqlBatch;

	sqlBatch.Declare("SET NOCOUNT ON");
	sqlBatch.Declare("BEGIN TRAN");

	//TES 1/31/2014 - PLID 60578 - Took out unnecessary locks
	sqlBatch.Declare("DECLARE @nLineItemID INT");
	sqlBatch.Declare("DECLARE @nProviderID INT");
	sqlBatch.Declare("DECLARE @nLocationID INT");

	// (j.jones 2009-06-04 11:07) - PLID 34341 - if we are applying to a charge and our preference says to use the charge provider
	// on the adjustment, then assign the charge provider ID now
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	if(nChargeID != -1 && GetRemotePropertyInt("ERemitUseChargeProviderOnApplies", 1, 0, "<None>", true) == 1) {
		//TES 1/31/2014 - PLID 60578 - Took out unnecessary locks
		sqlBatch.Add("SET @nProviderID = (SELECT DoctorsProviders FROM ChargesT WHERE ID = {INT})", nChargeID);
		//an unselected provider on a charge is -1, but on a payment it is NULL
		sqlBatch.Declare("IF @nProviderID = -1 BEGIN SET @nProviderID = NULL END");
	}
	else {
		//otherwise use the provider ID we loaded from the dialog
		sqlBatch.Add("SET @nProviderID = {INT}", nProviderID);
	}

	// (j.jones 2011-07-14 16:11) - PLID 41440 - if we are applying to a charge and our preference says to use the charge location
	// on the adjustment, then assign the charge location ID now
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	if(nChargeID != -1 && GetRemotePropertyInt("ERemitUseChargeLocationOnApplies", 1, 0, "<None>", true) == 1) {
		//TES 1/31/2014 - PLID 60578 - Took out unnecessary locks
		sqlBatch.Add("SET @nLocationID = (SELECT LocationID FROM LineItemT WHERE ID = {INT})", nChargeID);
	}
	else {
		//otherwise use the location ID from the batch payment, which we have already loaded
		sqlBatch.Add("SET @nLocationID = {INT}", nLocationID);
	}

	// (j.jones 2010-04-09 12:10) - PLID 31309 - use the provided date for the service date
	// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
	sqlBatch.Add(
		"INSERT INTO LineItemT (\r\n"
		"	PatientID, LocationID, Type, Date, InputName,\r\n"
		"	InputDate, Amount, Description\r\n"
		") VALUES (\r\n"
		"	{INT}, @nLocationID, 2, {OLEDATETIME}, {STRING},\r\n"
		"	GetDate(), {OLECURRENCY}, {STRING})", 
		nPatientID,	dtAdjDate, GetCurrentUserName(), 
		cyAdjustmentAmt, strDescription);

	sqlBatch.Declare(
		"SET @nLineItemID = SCOPE_IDENTITY() ");

	// (j.jones 2006-11-16 12:04) - PLID 23551 - supported GroupCode and ReasonCode
	// (j.jones 2009-06-09 14:35) - PLID 34549 - supported tracking the BatchPaymentID	
	// (j.jones 2010-09-23 16:16) - PLID 40653 - group and reason codes are now nullable IDs
	// (d.singleton 2014-10-15 14:35) - PLID 62698 - added claimnumber
	sqlBatch.Add(
		"INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, GroupCodeID, ReasonCodeID, BatchPaymentID, ClaimNumber) "
		"VALUES (@nLineItemID, {INT}, @nProviderID, {INT}, 0, {VT_I4}, {VT_I4}, {INT}, {STRING})", 
		nInsuredPartyID, nAdjustmentCategoryID, vtGroupCodeID, vtReasonCodeID, nBatchPaymentID, strClaimNumber);
	sqlBatch.Declare("INSERT INTO PaymentPlansT (ID) VALUES (@nLineItemID)");

	sqlBatch.Declare("COMMIT TRAN");
	sqlBatch.Declare("SET NOCOUNT OFF");
	sqlBatch.Declare("SELECT @nLineItemID AS NewAdjustmentID");

	_RecordsetPtr prsAdjustment = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());

	long nLineItemID = -1;
	
	while(prsAdjustment->GetState() == adStateClosed || prsAdjustment->eof) {

		_variant_t varNull;
		prsAdjustment = prsAdjustment->NextRecordset(&varNull);
	}

	if(!prsAdjustment->eof) {
		nLineItemID = AdoFldLong(prsAdjustment, "NewAdjustmentID",-1);
	}
	prsAdjustment->Close();

	long nAuditID = BeginNewAuditEvent();
	CString strAuditDesc;
	strAuditDesc.Format("%s Adjustment", FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE));
	// (j.jones 2011-03-21 17:04) - PLID 24273 - changed to use a unique E-Remit audit
	AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiAdjustmentCreatedByERemit, nLineItemID, "", strAuditDesc, aepHigh, aetCreated);

	//////////////////////////////
	//Step 2. Apply the Adjustment

	// (j.jones 2009-02-13 16:44) - PLID 33081 - skip trying to apply if the balance is completely zero
	// and our adjustment is not zero
	// (j.jones 2009-04-07 16:34) - PLID 33893 - it is possible, in rare circumstances, to not have a charge ID,
	// in which case we simply don't apply it
	if(nChargeID != -1) {
		COleCurrency cyInsBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);
		// (j.jones 2016-04-19 17:33) - NX-100246 - do not try to apply negative adjustments
		if((cyAdjustmentAmt > COleCurrency(0,0) && cyInsBalance > COleCurrency(0,0)) || cyAdjustmentAmt == COleCurrency(0,0)) {
			// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables
			// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
			AutoApplyPayToBill(nLineItemID,nPatientID,"Charge",nChargeID, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE);
		}
		// (j.jones 2012-06-27 10:41) - PLID 51236 - if we made an adjustment but didn't apply it, log that info.
		else if(nLineItemID != -1) {

			CString strLog;

			// (j.jones 2016-04-19 17:33) - NX-100246 - handle negative adjustments
			if (cyAdjustmentAmt < COleCurrency(0, 0)) {

				bool bAppliedToPayment = false;

				//if we have a payment ID, we should apply to it,
				//but we shouldn't bother if the payment isn't enough to fully
				//accept our negative adjustment
				COleCurrency cyPaymentAmount, cyOutgoing, cyIncoming;
				if (nNewPaymentID != -1
					&& GetPayAdjRefTotals(nNewPaymentID, nPatientID, cyPaymentAmount, cyOutgoing, cyIncoming)
					&& cyPaymentAmount >= -cyAdjustmentAmt) {

					//check the payment's balance
					COleCurrency cyPaymentBalance = cyPaymentAmount - cyOutgoing + cyIncoming;
					if (cyPaymentBalance < -cyAdjustmentAmt) {

						//Technically this should not happen, because ApplyPayment should know exactly
						//how much of the payment should have been left unapplied.
						//Getting to this code means that logic failed.
						//However, we can still continue on and make it right.
						ASSERT(FALSE);

						COleCurrency cyAmtToUnapply = (-cyAdjustmentAmt) - cyPaymentBalance;
						COleCurrency cyAmtUnapplied = UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyAmtToUnapply);

						Log("***ApplyAdjustment unapplied %s from the $s payment for ChargeID %li, PatientID %li, in order to post a %s adjustment.",
							FormatCurrencyForInterface(cyAmtUnapplied), FormatCurrencyForInterface(cyPaymentAmount), nChargeID, nPatientID, FormatCurrencyForInterface(cyAdjustmentAmt));

						//did that work?
						if (cyAmtUnapplied + cyPaymentBalance >= -cyAdjustmentAmt) {
							//it did, we have a new balance
							cyPaymentBalance += cyAmtUnapplied;
						}
						else {
							//not sure what would cause the above to fail - find out!
							ASSERT(FALSE);
						}
					}

					//now, can we apply it?
					if (cyPaymentBalance >= -cyAdjustmentAmt) {
						//we sure can!
						AutoApplyPayToPay(nLineItemID, nPatientID, "Payment", nNewPaymentID);
						bAppliedToPayment = true;
					}
				}

				//log accordingly based on if we succeeded or failed
				if(bAppliedToPayment) {
					strLog.Format("For patient '%s' [Patient ID: %li], Practice posted %s in negative insurance adjustments\n"
						"on the patient's account, applied to this EOB's %s payment. Negative adjustments are never applied to charges, only to payments.",
						GetExistingPatientName(nPatientID), nPatientUserDefinedID, FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE), FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE));
				}
				else {
					//we don't have a payment to apply to, or we do and it isn't big enough to fully absorb our negative adjustment
					strLog.Format("For patient '%s' [Patient ID: %li], Practice will post %s in negative insurance adjustments\n"
						"unapplied on the patient's account. Negative adjustments are never applied to charges.",
						GetExistingPatientName(nPatientID), nPatientUserDefinedID, FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE));
				}
			}
			else {

				// (j.jones 2012-06-27 10:42) - If we are supposed to be reducing adjustments (and we usually are),
				// then I am not aware of any valid case where this should happen.
				// We've run into this from time to time when we miscalculated adjustment amounts and 
				// patient responsibility. For example we want to adjust $50, but we want to shift $50 to patient,
				// we think we can apply $50 in adjustments because the $50 exists as ins. resp., but we shift that
				// $50 to patient before applying the adjustment. This is wrong.

				// Ideally we would assert here if we were supposed to reduce adjustments, but that calculation was made
				// in a different section of code that we no longer have access to.

				// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
				strLog.Format("For patient '%s' [Patient ID: %li], Practice will post %s in insurance adjustments\n"
					"unapplied on the patient's account, because not enough insurance responsibility remains on the charge.",
					GetExistingPatientName(nPatientID), nPatientUserDefinedID, FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE));
			}

			AddWarningToLog(strLog, "Adjustment Posted, Not Applied To A Charge");
		}
	}
	else {
		// (j.jones 2011-06-02 09:28) - PLID 43931 - warn that this adjustment is left unapplied
		CString strLog;
		// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
		strLog.Format("For patient '%s' [Patient ID: %li], Practice will post %s in insurance adjustments\n"
			"unapplied on the patient's account, because no charge was found for this posting.",
			GetExistingPatientName(nPatientID),nPatientUserDefinedID, FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE));

		AddWarningToLog(strLog, "Adjustment Posted, Not Applied To A Charge");
	}
}

// (j.jones 2008-11-17 12:07) - PLID 32007 - added bCalcOnly and cyInsAmtToShift, so we can call this function only to find out
// how much will be shifted, or we can call it to actually perform the shifting
void CEOBDlg::TryShiftRespToPatient(long nPatientID, long nChargeID, long nInsuredPartyID, COleCurrency cyPatResp, BOOL bCalcOnly, COleCurrency &cyInsAmtToShift) {

	//Step 1. Calculate the current patient responsibility and balance

	//1-1: patient responsibility
	COleCurrency cyCurrPatResp = GetChargePatientResp(nChargeID);	

	//1-2: patient balance
	COleCurrency cyCurrPatBal = GetChargePatientBalance(nChargeID, nPatientID);

	//Step 2. Calculate the current insurance responsibility and balance

	//2-1: insurance responsibility
	COleCurrency cyCurrInsResp = GetChargeInsResp(nChargeID, nInsuredPartyID);

	//2-2: insurance balance
	COleCurrency cyCurrInsBal = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);

	//Step 3. Try to shift responsibility to make the patient responsibility equal cyPatResp

	//cyPatResp is what the Insurance Company thinks the patient responsibility should be,
	//but this includes their co-pay amounts or deductibles, so if those have already been paid that might
	//not necessarily be their balance.

	if(cyCurrPatResp == cyPatResp) {
		//it already is correct
		return;
	}
	else if(cyCurrPatResp < cyPatResp) {		
		//responsibility needs to be switched to the patient

		COleCurrency cyAmtToShift = cyPatResp - cyCurrPatResp;

		if(cyCurrInsResp == COleCurrency(0,0) || cyCurrInsBal == COleCurrency(0,0)) {
			//if the current ins. responsiblity or balance is zero, then we can't shift to patient
			//this means that either we screwed up, the insurance company is misguided,
			//or unexpected shifting or applies have been made.

			//since we can't do anything about it
			cyInsAmtToShift = COleCurrency(0,0);
			return;
		}
		
		if(cyCurrInsBal >= cyAmtToShift) {
			//this is the ideal case, in that we are able to shift the full amount to balance the charges

			//shift the necessary amount
			// (j.jones 2008-11-17 12:39) - PLID 32007 - if bCalcOnly is true, we won't actually shift now
			cyInsAmtToShift = cyAmtToShift;
			if(!bCalcOnly) {
				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
				// E-Remittance because it also uses a unique audit type
				ShiftInsuranceResponsibility(nChargeID, nPatientID, nInsuredPartyID, -1, "Charge", cyAmtToShift,
					"following the preference to always shift patient responsibilities",
					COleDateTime::GetCurrentTime(), -1, -1, TRUE);
			}
		}
		else {
			//if we get here, then we are expected to shift more than is available to shift.
			//we can still balance the claim, but it suggests that previous unexpected shifting
			//or applies have been made

			//shift all that we can
			// (j.jones 2008-11-17 12:39) - PLID 32007 - if bCalcOnly is true, we won't actually shift now
			cyInsAmtToShift = cyCurrInsBal;
			if(!bCalcOnly) {
				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
				// E-Remittance because it also uses a unique audit type
				ShiftInsuranceResponsibility(nChargeID, nPatientID, nInsuredPartyID, -1, "Charge", cyCurrInsBal,
					"following the preference to always shift patient responsibilities",
					COleDateTime::GetCurrentTime(), -1, -1, TRUE);
			}
		}
	}
	else {
		//if we get here, the patient responsibility is greater than the predicted responsibility
		//for the time being, we shouldn't need to do anything
		cyInsAmtToShift = COleCurrency(0,0);
		return;
	}
}

// (j.jones 2011-11-08 15:18) - PLID 46240 - removed cyPatResp as a parameter, it was unused
// (j.jones 2012-08-23 12:49) - PLID 42438 - added nBillID
// (j.jones 2013-03-18 13:51) - PLID 55574 - Removed cyPaymentAmount, for one it was inaccurately named
// because it included adjustments, but it also wasn't needed. Added bChargeWasPosted, which is true if
// we did post a payment > $0.00, or an adjustment > $0.00, or if the reported patient resp ('other' resp) is > $0.00.
void CEOBDlg::TryShiftBalances(long nPatientID, long nBillID, long nChargeID, long nInsuredPartyID, BOOL bChargeWasPosted) {

	if(m_ShiftRespCombo->CurSel != -1) {

		long nShiftMethod = VarLong(m_ShiftRespCombo->GetValue(m_ShiftRespCombo->CurSel,0),0);

		if(nShiftMethod != 0) {	//0 means do not shift

			BOOL bShiftPostedCharges = m_checkOnlyShiftPostedCharges.GetCheck();
			BOOL bBatchShiftedClaims = IsDlgButtonChecked(IDC_CHECK_BATCH_CLAIMS);

			COleCurrency cyCurrInsBal = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);

			// (j.jones 2013-03-18 13:48) - PLID 55574 - The option to 'only shift amounts that were paid'
			// needs to also respect charges with a specified patient resp. In high-deductible plans, they
			// may pay $0.00, and might even adjust $0.00, but they will shift to patient, meaning that the
			// insurance really did post this charge, and the $0.00 payment is legitimate.
			// The caller now does this calculation and sets bChargeWasPosted to TRUE if any of these happened.
			if(cyCurrInsBal > COleCurrency(0,0) && (!bShiftPostedCharges || bChargeWasPosted)) {

				//we will shift the balance

				//first calculate where we are shifting it to
				long nDestInsuredPartyID = -1; //default to patient
				if(nShiftMethod == 2) {
					// (j.jones 2014-06-24 11:47) - PLID 60349 - moved this logic to a modular function
					nDestInsuredPartyID = GetNextInsuredPartyIDByPriority(nPatientID, nInsuredPartyID);
				}

				// (j.jones 2013-07-19 10:21) - PLID 57616 - track that we shifted to this insured party
				
				//shift insurance balance
				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, it doesn't need to say
				// E-Remittance because it also uses a unique audit type, but there's not much else to say here
				ShiftInsuranceResponsibility(nChargeID,nPatientID,nInsuredPartyID,nDestInsuredPartyID,"Charge",cyCurrInsBal,
					"after posting an EOB",
					COleDateTime::GetCurrentTime(), -1, -1, TRUE);

				//if the nDestInsuredPartyID is not patient, we're switching from insurance to insurance,
				//so we should swap the insurance company placement on the bill, and perhaps batch the claim
				if(nInsuredPartyID > -1 && nDestInsuredPartyID > -1) {
					//swap the company placement
					SwapInsuranceCompanies(nBillID, nInsuredPartyID, nDestInsuredPartyID, FALSE);
				}

				//batch if the user said to
				if(bBatchShiftedClaims) {
					// (j.jones 2012-01-17 11:59) - PLID 47510 - do not try to force paper,
					// instead just accept the default return value
					// (j.jones 2012-03-13 11:03) - PLID 48687 - always batch despite the
					// return value, so that we unbatch claims if requested
					// (j.jones 2012-08-23 12:50) - PLID 42438 - now unbatch (0) if patient resp.
					int iBatch = 0;
					if(nDestInsuredPartyID > -1) {
						iBatch = FindDefaultHCFABatch(nDestInsuredPartyID);
					}
					BatchBill(nBillID, iBatch);
				}
			}
			return;
		}		
	}

	//if they did not override the shifting, then shift to patient as specified by the EOB
	// (j.jones 2008-11-17 11:56) - PLID 32007 - this is now done prior to applying payments
	//TryShiftRespToPatient(nPatientID, nChargeID, nInsuredPartyID, cyPatResp);
}

// (j.jones 2008-11-17 12:24) - PLID 32007 - added cyPatientResp
// (j.jones 2009-04-07 15:04) - PLID 33862 - removed bDuplicate
// (j.jones 2010-02-08 11:53) - PLID 37182 - added bIsPrimaryIns
// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientResp to cyOtherResp, because even though the
// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
// (j.jones 2012-04-24 15:51) - PLID 35306 - Added reversed amount, so we know how much is currently applied to the
// given insurance resp. that we're about to void. This is a positive value, not negative.
// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
// (j.jones 2016-04-19 14:33) - NX-100161 - renamed the adjustment amount to cyPositiveAdjustmentAmount
// to clarify that we are only looking at the total of positive adjustments we are trying to apply to the charge
ERespStatus CEOBDlg::EnsureValidResponsibilities(long nPatientID,long nPatientUserDefinedID ,long nChargeID, long nInsuredPartyID, BOOL bIsPrimaryIns,
		COleCurrency cyPaymentAmount, COleCurrency &cyPositiveAdjustmentAmount, const COleCurrency cyOtherResp, COleCurrency cyReversedAmount, CString strPatInsCoName)
{
	// (j.jones 2012-04-24 16:12) - PLID 35306 - this value should never be negative, the calculations
	// below depend on it being zero if nothing reversed, or a positive value if something is reversed
	if(cyReversedAmount < COleCurrency(0,0)) {
		ThrowNxException("EnsureValidResponsibilities called with a negative cyReversedAmount for charge ID %li!", nChargeID);
	}

	// (j.jones 2016-04-19 14:33) - NX-100161 - renamed the adjustment amount to cyPositiveAdjustmentAmount,
	// so, you know, it should actually be positive!
	if (cyPositiveAdjustmentAmount < COleCurrency(0, 0)) {
		ThrowNxException("EnsureValidResponsibilities called with a negative cyPositiveAdjustmentAmount for charge ID %li!", nChargeID);
	}

	// (j.jones 2011-06-02 09:37) - PLID 43931 - if there is no charge ID, there is no responsibility,
	// so just return ePassed, because we are later going to post these payments unapplied
	if(nChargeID == -1) {
		return ePassed;
	}

	// (j.jones 2016-04-19 14:33) - NX-100161 - renamed the adjustment amount to cyPositiveAdjustmentAmount
	COleCurrency cyInsBalanceNeeded = cyPaymentAmount + cyPositiveAdjustmentAmount;

	// (j.jones 2012-04-19 10:16) - PLID 35306 - if this balance is negative, do nothing
	// as it means we would need to perform a takeback and we won't be applying more money
	if(cyInsBalanceNeeded < COleCurrency(0,0)) {
		return ePassed;
	}

	// (j.jones 2008-03-26 10:11) - PLID 29407 - added ERemitReduceAdjustments preference
	// (j.jones 2011-11-02 08:34) - PLID 46232 - this is no longer a boolean, instead it's the minimum
	// amount we can reduce the adjustment to
	// (j.jones 2016-04-19 14:33) - NX-100161 - renamed the adjustment amount to cyPositiveAdjustmentAmount
	COleCurrency cyMinimumAdjustmentAmount = cyPositiveAdjustmentAmount;

	// (j.jones 2010-02-08 11:46) - PLID 37182 - changed into ERemitPrimaryReduceAdjustments and ERemitSecondaryReduceAdjustments,
	// which continue to use ERemitReduceAdjustments as their default value, but if it doesn't exist then the primary option
	// defaults to 0 and the secondary option defaults to 1

	BOOL bReduceAdjustmentsPref = FALSE;
	BOOL bCanReduceAdjustments = FALSE;
	if(bIsPrimaryIns) {
		//this is the patient's primary, so use the primary preference
		bReduceAdjustmentsPref = GetRemotePropertyInt("ERemitPrimaryReduceAdjustments", GetRemotePropertyInt("ERemitReduceAdjustments", 0, 0, "<None>", false), 0, "<None>", true) != 0;;
	}
	else {
		//it is not their primary insurance, so use the secondary preference
		//(which is not specifically secondary, in this context we simply mean non-primary)
		bReduceAdjustmentsPref = GetRemotePropertyInt("ERemitSecondaryReduceAdjustments", GetRemotePropertyInt("ERemitReduceAdjustments", 1, 0, "<None>", false), 0, "<None>", true) != 0;;
	}

	// (j.jones 2011-11-02 08:43) - PLID 46232 - if the preference to reduce adjustments is enabled, then the
	// adjustment can be reduced as far as to $0.00, and we only need to enforce that the balance required
	// is the payment amount
	if(bReduceAdjustmentsPref) {
		//if the preference is enabled, we actually want to only ensure we have the payment amount available,
		//we don't have to have the adjustment amount available
		cyInsBalanceNeeded = cyPaymentAmount;
		cyMinimumAdjustmentAmount = COleCurrency(0,0);
		bCanReduceAdjustments = TRUE;
	}

	//Step 1. Ensure there is enough insurance responsibility to match cyInsBalanceNeeded
	
	//Step 1a. Check the current balance, if enough then we're done

	COleCurrency cyInsBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);

	// (j.jones 2012-04-24 16:10) - PLID 35306 - add cyReversedAmount to the balance,
	// as the balance will include this amount once the voiding is finished
	cyInsBalance += cyReversedAmount;

	// (j.jones 2008-11-17 12:10) - PLID 32007 - If the insurance specified a given patient responsibility,
	// we have to shift enough money to pat. resp. *before* trying to apply so the patient has that much resp.
	// It may be possible that they already have that resp. and it's already paid off, such as a copay, so we need
	// to call TryShiftRespToPatient with bCalcOnly set to TRUE to find out how much we would shift.
	
	// (j.jones 2009-03-05 14:53) - PLID 33220 - added preference to control shifting to patient responsibility by default
	// (pretty much a way to turn PLID 32007 on or off)
	// (j.jones 2010-09-02 11:01) - PLID 38787 - split the preference into primary/secondary, which default to the old value
	BOOL bShiftPatientResp = FALSE;
	if(bIsPrimaryIns) {
		//primary defaults to the old value, 0 if it doesn't exist
		bShiftPatientResp = GetRemotePropertyInt("ERemitPrimaryAutoShiftPatientResp", GetRemotePropertyInt("ERemitAutoShiftPatientResp", 0, 0, "<None>", false), 0, "<None>", true);
	}
	else {
		//secondary or other non-primary defaults to on
		bShiftPatientResp = GetRemotePropertyInt("ERemitSecondaryAutoShiftPatientResp", 1, 0, "<None>", true);
	}

	//TES 4/13/2012 - PLID 49706 - This calculation isn't that helpful, because our insurance balance may be higher by the time we actually
	// start shifting, because we may have shifted some (for example) from primary to secondary.  So what we need to do is calculate not
	// how much we are currently able to shift, but rather how much we will eventually need to shift, and then add that to cyInsBalanceNeeded.
	COleCurrency cyAmtToShiftToPat = COleCurrency(0,0);
	if(bShiftPatientResp && cyOtherResp > COleCurrency(0,0)) {
		COleCurrency cyCurrPatResp = GetChargePatientResp(nChargeID);
		cyAmtToShiftToPat = cyOtherResp - cyCurrPatResp;
		if(cyAmtToShiftToPat <= COleCurrency(0,0)) {
			//TES 5/2/2012 - PLID 49706 - We already have enough patient responsibility, we won't need to shift anything to it.
			cyAmtToShiftToPat = COleCurrency(0,0);
		}
		cyInsBalanceNeeded += cyAmtToShiftToPat;
		// (j.jones 2011-11-08 15:48) - PLID 46240 - cyOtherResp is what the EOB states is patient resp.,
		// this is the amount we are going to shift to patient
		/*TryShiftRespToPatient(nPatientID, nChargeID, nInsuredPartyID, cyOtherResp, TRUE, cyAmtToShiftToPat);
		if(cyAmtToShiftToPat > COleCurrency(0,0)) {
			cyInsBalance -= cyAmtToShiftToPat;
		}*/
	}

	// (j.jones 2011-03-07 17:21) - PLID 41877 - If the reduce adjustment preference is on,
	// it may be reduced further in ReduceAdjustmentValue() to account for cyPatientResp,
	// but if the preference is off, we still need to make sure that the adjustment won't take
	// money that needs to remain in order to cover cyPatientResp. However we do not need to
	// do this if we already forcibly shifted resp. to patient.
	// (j.jones 2011-11-08 16:22) - PLID 46240 - cyPatientResp has been renamed to cyOtherResp,
	// because while the EOB calls it patient resp., it actually means "resp. that isn't this insurance company",
	// so all we have to do is ensure it exists somewhere else on this charge.
	if(!bReduceAdjustmentsPref && !bShiftPatientResp && cyOtherResp > COleCurrency(0,0)) {

		//We need to ensure that the amount in cyOtherResp will still exist
		//as a total of the Practice patient resp. + other resp. + remaining insurance balance.
		//Calculate this as cyAvailOtherResp.
		COleCurrency cyAvailOtherResp = GetChargeInsBalanceAndOtherResps(nChargeID, nInsuredPartyID);

		// (j.jones 2012-04-24 16:10) - PLID 35306 - add cyReversedAmount to the balance,
		// as the balance will include this amount once the voiding is finished
		cyAvailOtherResp += cyReversedAmount;

		COleCurrency cyRemain = (cyAvailOtherResp - cyPaymentAmount - cyPositiveAdjustmentAmount);
		COleCurrency cyDiff = cyOtherResp - cyRemain;
		if(cyDiff > COleCurrency(0,0)) {
			//the remaining insurance balance of the charge after applying this payment and adjustment,
			//plus the existing patient resp, will not be enough for cyPatientResp, so we need
			//to take away from the amount we need
			
			//if we can take the difference out of the adjustment, do so
			if(cyPositiveAdjustmentAmount > cyDiff) {
				cyInsBalanceNeeded -= cyDiff;
				// (j.jones 2011-11-02 08:52) - PLID 46232 - reduce our minimum adjustment 
				// amount, since we can no longer post it in its entirety
				cyMinimumAdjustmentAmount -= cyDiff;

				if(cyMinimumAdjustmentAmount < COleCurrency(0,0)) {
					// (j.jones 2011-11-02 10:37) - I haven't found a case where this is possible,
					// but maybe the amount needed cannot actually be acquired, so we would apply
					// what we can, and reduce the adjusment to zero
					cyMinimumAdjustmentAmount = COleCurrency(0,0);
				}
			}
			//if we can't take the full difference out of the adjustment,
			//just reduce the adjustment needs to zero
			else if(cyPositiveAdjustmentAmount <= cyDiff) {
				cyInsBalanceNeeded -= cyPositiveAdjustmentAmount;
				// (j.jones 2011-11-02 08:52) - PLID 46232 - reduce our minimum adjustment 
				// amount to zero, since we can no longer post it at all
				cyMinimumAdjustmentAmount = COleCurrency(0,0);
			}

			bCanReduceAdjustments = TRUE;
		}
	}

	if(cyInsBalance >= cyInsBalanceNeeded) {
		
		// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
		// the payment amount, and the adjustment amount may be larger, so we need to reduce the
		// adjustment amount if it is more than the new balance
		// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
		// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
		if(bCanReduceAdjustments) {
			ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID,nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
		}
		return ePassed;
	}

	//Step 1b. Check the current responsibility

	COleCurrency cyInsResp = GetChargeInsResp(nChargeID, nInsuredPartyID);

	//Step 1c. If there is enough responsibility but not enough balance, prompt to unapply, and do so if permitted

	if(cyInsResp >= cyInsBalanceNeeded) {

		//we have enough responsibility, perhaps we already processed this EOB?

		//Check the payment/adjustment amounts.
		//If the payment amount is $0.00 and the adjustment amount is non-zero,
		//then we can assume it is also likely to be a duplicate claim.

		// (j.jones 2011-03-21 11:49) - PLID 42099 - if a patient resp. is specified, it's probably not a duplicate
		// (j.jones 2011-11-08 16:57) - PLID 46240 - cyPatientResp has been renamed to cyOtherResp, but this code should remain
		// unchanged, as it still represents what the EOB states should be patient resp
		if(cyPaymentAmount == COleCurrency(0,0) && cyPositiveAdjustmentAmount > COleCurrency(0,0) && cyOtherResp == COleCurrency(0,0)) {
			//warn that is could be a duplicate, ask if they still want to process the charge
			//'yes' processes, 'no' skips, 'cancel' quits processing altogether
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_PossibleDuplicateWithNotEnoughResp(nChargeID, nPatientID,nPatientUserDefinedID, TRUE, FALSE, cyPaymentAmount,
				cyPositiveAdjustmentAmount, cyInsBalanceNeeded - cyInsBalance, COleCurrency(0,0), strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply the needed amount of insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsBalanceNeeded - cyInsBalance, NULL, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID,nPatientUserDefinedID, nPatientID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
		else {
			//most common case - could happen if they are applying the same EOB twice by accident
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_NotEnoughResp(nChargeID, nPatientID, nPatientUserDefinedID, TRUE, FALSE, cyPaymentAmount, cyPositiveAdjustmentAmount, cyInsBalanceNeeded - cyInsBalance, COleCurrency(0,0), strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply the needed amount of insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsBalanceNeeded - cyInsBalance, NULL, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID, nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
	}

	//Step 2. We don't have enough insurance responsibility for THIS resp, so try others

	//calculate the total balance of all responsibilities that aren't this one
	COleCurrency cyTotalOthrBalance = GetChargePatientBalance(nChargeID, nPatientID);
	//loop through the insurance resps, in order, skipping the current resp
	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT "
		"WHERE PatientID = {INT} AND RespTypeID <> -1 AND PersonID <> {INT} "
		"ORDER BY RespTypeID; "
		"SELECT PersonID FROM InsuredPartyT "
		"WHERE PatientID = {INT} AND RespTypeID = -1 AND PersonID <> {INT}; ",
		nPatientID, nInsuredPartyID, nPatientID, nInsuredPartyID);

	while(!rs->eof) {
		cyTotalOthrBalance += GetChargeInsBalance(nChargeID, nPatientID, AdoFldLong(rs, "PersonID",-1));

		rs->MoveNext();
	}
	rs->Close();

	//our total "other" balance is now all responsibilities except this one, counting patient responsibility

	if((cyTotalOthrBalance + cyInsBalance) >= cyInsBalanceNeeded) {

		//ok, we can acquire enough responsibility from other insurances/patient
		CString strWarningToLog;
		BOOL bAddWarningToLog = FALSE;
				
		//Without warning the user, shift the needed amounts from other insurances and then patient

		COleCurrency cyBalanceToAcquire = cyInsBalanceNeeded - cyInsBalance;

		//we know that there is enough balance to support cyBalanceToAcquire, 
		//but it can be spread across several responsibilities.
		//It's time to go hunting.
		
		//first loop through the insurance resps, in order, skipping the current resp
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT "
			"WHERE PatientID = {INT} AND RespTypeID <> -1 AND PersonID <> {INT} "
			"ORDER BY RespTypeID; "
			"SELECT PersonID FROM InsuredPartyT "
			"WHERE PatientID = {INT} AND RespTypeID = -1 AND PersonID <> {INT}; ",
			nPatientID, nInsuredPartyID, nPatientID, nInsuredPartyID);

		while(!rs->eof && cyBalanceToAcquire > COleCurrency(0,0)) {

			long nCurInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
			COleCurrency cyCurRespBalance = GetChargeInsBalance(nChargeID, nPatientID, nCurInsuredPartyID);
			if(cyCurRespBalance > COleCurrency(0,0)) {

				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
				// E-Remittance because it also uses a unique audit type

				//there is a balance, let's steal it
				if(cyCurRespBalance >= cyBalanceToAcquire) {
					//we have enough, so take what we need
					ShiftInsuranceResponsibility(nChargeID,nPatientID,nCurInsuredPartyID,nInsuredPartyID,"Charge",cyBalanceToAcquire,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					cyBalanceToAcquire = COleCurrency(0,0);
				}
				else {
					//we don't have enough, so take all the balance
					ShiftInsuranceResponsibility(nChargeID,nPatientID,nCurInsuredPartyID,nInsuredPartyID,"Charge",cyCurRespBalance,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					cyBalanceToAcquire -= cyCurRespBalance;
				}
			}

			rs->MoveNext();
		}
		rs->Close();

		if(cyBalanceToAcquire > COleCurrency(0,0)) {
			//we still have resp. left over, so now try shifting from patient
			COleCurrency cyCurPatBalance = GetChargePatientBalance(nChargeID, nPatientID);

			// (j.jones 2008-11-17 12:13) - PLID 32007 - Remember, we are going to shift to patient balance,
			// but haven't done it yet, so we need to use that calculated amount here to determine how much
			// we can really take. cyAvailPatBalance should represent the current balance, plus how much we
			// intend to shift, minus how much we need to stay as patient resp.

			// (j.jones 2011-11-08 17:02) - PLID 46240 - Reworked this code, as cyOtherResp has replaced cyPatientResp.
			// This still represents what the EOB states is patient resp., but technically is permitted to be on any
			// other resp. as well.
			COleCurrency cyTotalPatientAndOtherResps = GetChargePatientAndOtherRespTotals(nChargeID, nInsuredPartyID);

			//first calculate the available balance as the real balance, plus the amount we're intending to shift to it
			COleCurrency cyAvailPatBalance = (cyCurPatBalance + cyAmtToShiftToPat);

			//Example of the following calculations: If the current available patient balance (cyAvailPatBalance) is $20,
			//but the EOB states that $30 needs to remain untouched (cyOtherResp), we need to first find out how much
			//of that $30 is sitting in patient resp.
			//If our cyTotalPatientAndOtherResps is $50, and cyCurrentPatientResp is $40 and cyCurrentOtherInsResp is $10,
			//then we have to make sure we do not reduce cyCurrentPatientResp below $20, because when we are all done,
			//cyCurrentPatientResp + cyCurrentOtherInsResp has to be at or above the EOB stated value of $30. (Whew!)

			if(cyAvailPatBalance > COleCurrency(0,0) && cyOtherResp > COleCurrency(0,0)) {
				//we have a balance, but we might need it to stay put in order to satisfy cyOtherResp,
				//because the EOB says that amount cannot be used towards this insurance resp.

				//find out how much of cyTotalPatientAndOtherResps is for other insurances
				COleCurrency cyCurrentPatientResp = GetChargePatientResp(nChargeID);
				COleCurrency cyCurrentOtherInsResp = cyTotalPatientAndOtherResps - cyCurrentPatientResp;
				if(cyCurrentOtherInsResp < cyOtherResp) {
					//the amount required to remain in "other" resp is not fully represented by
					//another insurance, which means that some of the patient resp. has to stay
					//in order to satisfy cyOtherResp
					COleCurrency cyPatRespToRemain = cyOtherResp - cyCurrentOtherInsResp;
					cyAvailPatBalance -= cyPatRespToRemain;

					if(cyAvailPatBalance <= COleCurrency(0,0)) {

						//Seems like this would only happen if the EOB designates more money to go to patient
						//resp ("other" resp) than we actually have left on the charge. If so, there's nothing
						//we can do. Hitting this ASSERT probably isn't an error state but we should review
						//the data to make sure we're handling this case properly.
						
						//This is fine. This tends to happen in cases where we can't apply an adjustment,
						//we want to take money from patient to do so, but there isn't enough balance,
						//possibly because the patient resp. has already been paid.
						//ASSERT(FALSE);

						cyAvailPatBalance = COleCurrency(0,0);
					}
				}
			}

			if(cyAvailPatBalance > COleCurrency(0,0)) {

				// (j.jones 2008-11-17 12:34) - It seems to me that if we are shifting
				// anything to patient, then it would mean we wouldn't have any patient
				// balance available to shift back, so how could this be zero?
				// Could be a bad EOB. But it also could be a flaw in our design.
				// If this assertion is hit, investigate.
				ASSERT(cyAmtToShiftToPat == COleCurrency(0,0));

				//there is a balance, let's steal it
				if(cyAvailPatBalance >= cyBalanceToAcquire) {
					//we have enough, so take what we need
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyBalanceToAcquire,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					cyBalanceToAcquire = COleCurrency(0,0);
				}
				else {
					// (j.jones 2011-11-02 11:01) - This would happen if enough patient resp. is available, but we can't shift it
					// because the EOB says that a certain amount has to be there. This means the payment is colliding
					// with the designated patient amount. The remaining payment will be left unapplied.
					// (j.jones 2011-11-08 17:07) - PLID 46240 - The only known case of this being hit has been fixed,
					// though if you hit this, the above statement is probably still true.
					ASSERT(FALSE);

					Log("***EnsureValidResponsibilities cannot apply the entire payment to charge ID %li, patient ID %li - Not enough Patient Resp. Available!", nChargeID, nPatientID);
					//we don't have enough, so take all the balance
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyAvailPatBalance,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					cyBalanceToAcquire -= cyCurPatBalance;
				}
			}
		}

		if(cyBalanceToAcquire > COleCurrency(0,0)) {
			// (j.jones 2011-09-27 11:28) - I found a way for this to be possible. If we do have enough resp,
			// but it is patient resp specified by the EOB, then we can't shift it for use by an insurance payment.
			/*
			//this should be impossible because we previously checked that there should be enough total balance
			ASSERT(FALSE);
			Log("***Error in EnsureValidResponsibilities - Not enough resp. Available!");
			*/
		}
		else {

			if(bAddWarningToLog)
				AddWarningToLog(strWarningToLog, "Processed");

			// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
			// the payment amount, and the adjustment amount may be larger, so we need to reduce the
			// adjustment amount if it is more than the new balance
			// In this case, we moved the exact amount we needed, so there would be no balance, so send in the payment amount.
			// (j.jones 2011-11-02 08:59) - PLID 46232 - Now we check bCanReduceAdjustments, which is true
			// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments.
			if(bCanReduceAdjustments) {
				ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID, nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
			}
			return ePassed;
		}

		//technically, getting here should be impossible, and you will have had two ASSERTs and two logs of errors,
		//but if you do get here, DO NOT return, and instead go on down the line - you'll get the generic
		//"can't acquire responsibility" message

		// (j.jones 2011-09-27 11:28) - I found a way for this to be possible. If we do have enough resp,
		// but it is patient resp specified by the EOB, then we can't shift it for use by an insurance payment.
		// So continue, and provide the normal generic warnings.
	}

	// (j.jones 2005-11-04 11:55) - 3b is questionable on second thought, but blessedly rarely (never?) happens
	// we should consider removing it. Otherwise, at some point we should revise it to look at all responsibilities,
	// instead of just patient responsibility

	//Step 3b. Calculate if the insurance responsibility plus patient balance will be enough.
	//If so, prompt to unapply insurance payments and do so if permitted.

	COleCurrency cyPatBalance = GetChargePatientBalance(nChargeID, nPatientID);
	if((cyPatBalance + cyInsResp) >= cyInsBalanceNeeded) {

		//it's possible they processed this EOB already, or they keep the responsibility as patient
	
		//Check the payment/adjustment amounts.
		//If the payment amount is $0.00 and the adjustment amount is non-zero,
		//then we can assume it is also likely to be a duplicate claim.

		// (j.jones 2011-03-21 11:49) - PLID 42099 - if a patient resp. is specified, it's probably not a duplicate
		// (j.jones 2011-11-08 16:57) - PLID 46240 - cyPatientResp has been renamed to cyOtherResp, but this code should remain
		// unchanged, as it still represents what the EOB states should be patient resp
		if(cyPaymentAmount == COleCurrency(0,0) && cyPositiveAdjustmentAmount > COleCurrency(0,0) && cyOtherResp == COleCurrency(0,0)) {
			//warn that is could be a duplicate, ask if they still want to process the charge
			//'yes' processes, 'no' skips, 'cancel' quits processing altogether
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_PossibleDuplicateWithNotEnoughResp(nChargeID, nPatientID,nPatientUserDefinedID, TRUE, FALSE, cyPaymentAmount, cyPositiveAdjustmentAmount, cyInsBalanceNeeded - cyInsResp, COleCurrency(0,0), strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply all the insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsResp - cyInsBalance, NULL, TRUE);
					//shift the patient balance
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyInsBalanceNeeded - cyInsResp,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// In this case, we moved the exact amount we needed, so there would be no balance, so send in the payment amount.
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID, nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
		else {
			//most common case - could happen if they are applying the same EOB twice by accident
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_NotEnoughResp(nChargeID, nPatientID, nPatientUserDefinedID, TRUE, FALSE, cyPaymentAmount, cyPositiveAdjustmentAmount, cyInsBalanceNeeded - cyInsResp, COleCurrency(0,0), strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply all the insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsResp - cyInsBalance, NULL, TRUE);
					//shift the patient balance
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyInsBalanceNeeded - cyInsResp,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// In this case, we moved the exact amount we needed, so there would be no balance, so send in the payment amount.
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID,nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
	}

	//Step 3c. Prompt to unapply insurance payments AND patient payments, and do so if permitted.

	// (j.jones 2005-11-04 11:55) - 3c is questionable on second thought, but blessedly rarely (never?) happens
	// we should consider removing it. Otherwise, at some point we should revise it to look at all responsibilities,
	// instead of just patient responsibility

	COleCurrency cyPatResp = GetChargePatientResp(nChargeID);

	if((cyPatResp + cyInsResp) >= cyInsBalanceNeeded) {

		//it's possible they processed this EOB already, or they keep the responsibility as patient and make the patient pay

		//Check the payment/adjustment amounts.
		//If the payment amount is $0.00 and the adjustment amount is non-zero,
		//then we can assume it is also likely to be a duplicate claim.

		// (j.jones 2011-03-21 11:49) - PLID 42099 - if a patient resp. is specified, it's probably not a duplicate
		// (j.jones 2011-11-08 16:57) - PLID 46240 - cyPatientResp has been renamed to cyOtherResp, but this code should remain
		// unchanged, as it still represents what the EOB states should be patient resp
		if(cyPaymentAmount == COleCurrency(0,0) && cyPositiveAdjustmentAmount > COleCurrency(0,0) && cyOtherResp == COleCurrency(0,0)) {
			//warn that is could be a duplicate, ask if they still want to process the charge
			//'yes' processes, 'no' skips, 'cancel' quits processing altogether
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_PossibleDuplicateWithNotEnoughResp(nChargeID, nPatientID,nPatientUserDefinedID,
				(cyInsResp - cyInsBalance) != COleCurrency(0,0),
				(cyPatResp - (cyInsBalanceNeeded - cyInsResp)) != COleCurrency(0,0),
				cyPaymentAmount, cyPositiveAdjustmentAmount,
				cyInsResp - cyInsBalance,
				cyPatResp - (cyInsBalanceNeeded - cyInsResp),
				strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply all the insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsResp - cyInsBalance, NULL, TRUE);
					//unapply all the patient payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, FALSE, cyPatResp - (cyInsBalanceNeeded - cyInsResp), NULL, TRUE);
					//shift the patient balance
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyInsBalanceNeeded - cyInsResp,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// In this case, we moved the exact amount we needed, so there would be no balance, so send in the payment amount.
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID,nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
		else {
			//most common case - could happen if they are applying the same EOB twice by accident
			CString strWarningToLog;
			// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
			// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
			UnapplyWarningResult eResult = GenerateUnapplyWarning_NotEnoughResp(nChargeID, nPatientID,nPatientUserDefinedID,
				(cyInsResp - cyInsBalance) != COleCurrency(0,0),
				(cyInsBalanceNeeded - cyInsResp) > COleCurrency(0,0),
				cyPaymentAmount, cyPositiveAdjustmentAmount,
				cyInsResp - cyInsBalance,
				(cyInsBalanceNeeded - cyInsResp),
				strWarningToLog, bCanReduceAdjustments, strPatInsCoName);
			if (eResult == UnapplyWarningResult::eProcess) {
				if(IDYES == MessageBox("Are you SURE you wish to unapply these items?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					//unapply all the insurance payments
					UnapplyFromCharge(nChargeID, nInsuredPartyID, TRUE, cyInsResp - cyInsBalance, NULL, TRUE);
					//unapply all the patient payments
					// (j.jones 2009-04-02 10:59) - PLID 33821 - fixed to unapply only our balance needed minus the ins. resp
					UnapplyFromCharge(nChargeID, nInsuredPartyID, FALSE, cyInsBalanceNeeded - cyInsResp, NULL, TRUE);
					//shift the patient balance
					// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string, doesn't need to say
					// E-Remittance because it also uses a unique audit type
					ShiftInsuranceResponsibility(nChargeID,nPatientID,-1,nInsuredPartyID,"Charge",cyInsBalanceNeeded - cyInsResp,
						"in order to post the EOB",
						COleDateTime::GetCurrentTime(), -1, -1, TRUE);
					AddWarningToLog(strWarningToLog, "Processed");
					// (j.jones 2008-03-26 10:25) - PLID 29407 - if bReduceAdjustments is true, we only checked
					// the payment amount, and the adjustment amount may be larger, so we need to reduce the
					// adjustment amount if it is more than the new balance
					// In this case, we moved the exact amount we needed, so there would be no balance, so send in the payment amount.
					// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
					// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
					if(bCanReduceAdjustments) {
						ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyPaymentAmount, nInsuredPartyID, nPatientID,nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
					}
					return ePassed;
				}
				else {
					AddWarningToLog(strWarningToLog, "Posting Cancelled");
					return eFailed;
				}
			}
			else if (eResult == UnapplyWarningResult::eSkip) {
				AddWarningToLog(strWarningToLog, "Skipped");
				return eSkipped;
			}
			else {
				AddWarningToLog(strWarningToLog, "Posting Cancelled");
				return eFailed;
			}
		}
	}

	//if we have no patient, quit now silently
	if(nPatientID == -1) {
		return ePassed;
	}

	// (j.jones 2009-02-13 13:20) - PLID 33081 - now rather than giving up, we can post the payment and leave the overage unapplied,
	// so tell the user about this problem, and allow them to continue
	CString strWarningToLog;
	// (j.jones 2009-04-07 15:04) - PLID 33862 - removed bDuplicate
	// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
	// (j.jones 2016-04-20 17:16) - NX-100251 - this now reutrns an enum for a result
	UnapplyWarningResult eResult = GenerateOverpaymentWarning(nChargeID, nPatientID,nPatientUserDefinedID,
		cyInsBalance, cyPaymentAmount, cyPositiveAdjustmentAmount,
		strWarningToLog, bCanReduceAdjustments, strPatInsCoName);

	if (eResult == UnapplyWarningResult::eProcess) {
		AddWarningToLog(strWarningToLog, "Processed");
		// (j.jones 2011-11-02 08:59) - PLID 46232 - now we check bCanReduceAdjustments, which is true
		// if the reduce adjustment preference is on, or the presence of patient resp. forced us to reduce adjustments
		if(bCanReduceAdjustments) {
			//ReduceAdjustmentValue wants to know how much of the payment we applied, so we need to calculate that
			COleCurrency cyAppliedPaymentAmount = cyPaymentAmount;
			if(cyPaymentAmount > cyInsBalance) {
				cyAppliedPaymentAmount = cyInsBalance;
			}
			ReduceAdjustmentValue(cyPositiveAdjustmentAmount, cyMinimumAdjustmentAmount, cyAppliedPaymentAmount, nInsuredPartyID, nPatientID,nPatientUserDefinedID, nChargeID, cyOtherResp, bShiftPatientResp);
		}
		return ePassed;
	}
	else if (eResult == UnapplyWarningResult::eSkip) {
		AddWarningToLog(strWarningToLog, "Skipped");
		return eSkipped;
	}
	else {
		AddWarningToLog(strWarningToLog, "Posting Cancelled");
		return eFailed;
	}

	return eFailed;
}

// (j.jones 2008-03-26 10:11) - PLID 29407 - this function now takes in the bReduceAdjustments parameter
// (j.jones 2011-11-02 09:01) - PLID 46232 - renamed bReduceAdjustments to bCanReduceAdjustments, so we know which boolean it represents from EnsureValidResponsibilities
// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
// (j.jones 2016-04-20 17:00) - NX-100251 - this now creates its own messagebox and returns an enum for what the user chose to do
CEOBDlg::UnapplyWarningResult CEOBDlg::GenerateUnapplyWarning_PossibleDuplicateWithNotEnoughResp(long nChargeID, long nPatientID,long nPatientUserDefinedID, BOOL bInsurance, BOOL bPatient, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, COleCurrency cyInsuranceTotalToUnapply, COleCurrency cyPatientTotalToUnapply, CString &strWarningToLog, BOOL bCanReduceAdjustments, CString strPatInsCoName)
{
	CString strWarningHeader, strWarningBody, str, strUnapply;

	CString strPatientName = GetExistingPatientName(nPatientID);
	CString strItemDesc;

	//COleCurrency cyInsBalanceNeeded = cyPaymentAmount + cyAdjustmentAmount;

	// (b.eyers 2016-04-21 12:11) - NX-100187
	_RecordsetPtr rs = CreateParamRecordset("SELECT ItemCode FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.ID = {INT}", nChargeID);
	if (!rs->eof) {
		strItemDesc.Format("%s", AdoFldString(rs, "ItemCode", ""));
	}
	rs->Close();

	// (b.eyers 2016-04-15 12:11) - NX-100187 - grab the insurance company name from the dropdown
	CString strEOBInsCoName = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(), iccName));
	// (b.eyers 2016-04-21 12:11) - NX-100187
	strWarningHeader.Format("Warning for %s (%li) for %s", strPatientName, nPatientUserDefinedID, strEOBInsCoName);

	// (b.eyers 2016-04-21 12:11) - NX-100187
	if (bInsurance && bPatient) {
		strUnapply.Format("both %s and the patient", strPatInsCoName);
	}
	else if (bInsurance) {
		strUnapply.Format("%s", strPatInsCoName);
	}
	else if (bPatient) {
		strUnapply.Format("the patient");
	}

	// (j.jones 2008-03-26 10:28) - PLID 29407 - include the words "up to" when referencing the adjustment amount,
	// if bReduceAdjustments is true
	// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
	// (b.eyers 2016-04-15 12:11) - NX-100187 - Add name of the eob check ins co and name of ins co the $ is being applied to
	str.Format("This EOB states to post %s in payments from %s and %s%s in adjustments to the charge: %s, "
		"but there are existing payments and/or adjustments applied to this responsibility from %s. \n\n"
		"For this to post correctly: \n",
		FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE), strPatInsCoName,
		bCanReduceAdjustments ? "up to " : "", FormatCurrencyForInterface(cyAdjustmentAmount, TRUE, TRUE),
		strItemDesc, strUnapply);

	strWarningBody += str;

	if(bInsurance) {
		// (b.eyers 2016-04-15 12:11) - NX-100187 
		str.Format("  - %s of Insurance Payments/Adjustments need to be unapplied from this charge.\n",FormatCurrencyForInterface(cyInsuranceTotalToUnapply,TRUE,TRUE)); 
		strWarningBody += str;
	}

	if(bPatient) {
		str.Format("  - %s of Patient Payments/Adjustments need to be unapplied from this charge.\n"
				   "    (This amount will then be transferred to insurance responsibility.)\n",FormatCurrencyForInterface(cyPatientTotalToUnapply,TRUE,TRUE));   
		strWarningBody += str;
	}

	strWarningBody += "\n";

	strWarningBody += "This may be a duplicate claim that was already paid on a prior EOB. \n"
		"Do you wish to still post to this charge?";

	strWarningToLog = strWarningHeader + "\n" + strWarningBody;

	// (b.eyers 2016-04-21 12:11) - NX-100187
	if (bInsurance && bPatient) {
		strUnapply.Format("%s and patient", strPatInsCoName);
	}
	else if (bInsurance) {
		strUnapply.Format("%s", strPatInsCoName);
	}
	else if (bPatient) {
		strUnapply.Format("patient");
	}

	// (j.jones 2016-04-20 17:16) - NX-100251 - this is now an NxTaskDialog
	NxTaskDialog dlgWarning(this);
	dlgWarning.Config()
		.WarningIcon()
		.ZeroButtons()
		.MainInstructionText(strWarningHeader)
		.ContentText(strWarningBody)
		.AddCommand(UnapplyWarningResult::eSkip, "Skip This Charge\nSkip this charge and continue to process the rest of the EOB")
		.AddCommand(UnapplyWarningResult::eProcess, FormatString("Post Payments / Adjustments\nThis will unapply existing %s credits and post the above payments/adjustments", strUnapply))
		.AddCommand(UnapplyWarningResult::eCancel, "Cancel\nStop processing this EOB")
		.DefaultButton(UnapplyWarningResult::eSkip);
	return (UnapplyWarningResult)dlgWarning.DoModal();
}

// (j.jones 2008-03-26 10:11) - PLID 29407 - this function now takes in the bReduceAdjustments parameter
// (j.jones 2011-11-02 09:01) - PLID 46232 - renamed bReduceAdjustments to bCanReduceAdjustments, so we know which boolean it represents from EnsureValidResponsibilities
// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
// (j.jones 2016-04-20 17:00) - NX-100251 - this now creates its own messagebox and returns an enum for what the user chose to do
CEOBDlg::UnapplyWarningResult CEOBDlg::GenerateUnapplyWarning_NotEnoughResp(long nChargeID, long nPatientID, long nPatientUserDefinedID, BOOL bInsurance, BOOL bPatient, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, COleCurrency cyInsuranceTotalToUnapply, COleCurrency cyPatientTotalToUnapply, CString &strWarningToLog, BOOL bCanReduceAdjustments, CString strPatInsCoName)
{
	CString strWarningHeader, strWarningBody, str, strUnapply;

	CString strPatientName = GetExistingPatientName(nPatientID);
	CString strItemDesc;

	//COleCurrency cyInsBalanceNeeded = cyPaymentAmount + cyAdjustmentAmount;

	// (b.eyers 2016-04-21 12:11) - NX-100187
	_RecordsetPtr rs = CreateParamRecordset("SELECT ItemCode FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.ID = {INT}",nChargeID);
	if(!rs->eof) {
		strItemDesc.Format("%s",AdoFldString(rs, "ItemCode",""));
	}
	rs->Close();

	// (b.eyers 2016-04-15 12:11) - NX-100187 - grab the insurance company name from the dropdown
	CString strEOBInsCoName = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(), iccName));
	// (b.eyers 2016-04-21 12:11) - NX-100187
	strWarningHeader.Format("Warning for %s (%li) for %s", strPatientName, nPatientUserDefinedID, strEOBInsCoName);

	// (b.eyers 2016-04-21 12:11) - NX-100187
	if (bInsurance && bPatient) {
		strUnapply.Format("both %s and the patient", strPatInsCoName);
	}
	else if (bInsurance) {
		strUnapply.Format("%s", strPatInsCoName);
	}
	else if (bPatient) {
		strUnapply.Format("the patient");
	}

	// (j.jones 2008-03-26 10:28) - PLID 29407 - include the words "up to" when referencing the adjustment amount,
	// if bReduceAdjustments is true
	// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
	// (b.eyers 2016-04-15 12:11) - NX-100187 - Add name of the eob check ins co and name of ins co the $ is being applied to
	str.Format("This EOB states to post %s in payments from %s and %s%s in adjustments to the charge: %s, "
		"but there are existing payments and/or adjustments applied to this responsibility from %s. \n\n"
		"For this to post correctly: \n",
		FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE), strPatInsCoName, 
		bCanReduceAdjustments ? "up to " : "", FormatCurrencyForInterface(cyAdjustmentAmount, TRUE, TRUE), 
		strItemDesc, strUnapply); 

	strWarningBody += str;

	if(bInsurance) {
		// (b.eyers 2016-04-15 12:11) - NX-100187 
		str.Format("  - %s of Insurance Payments/Adjustments need to be unapplied from this charge.\n",FormatCurrencyForInterface(cyInsuranceTotalToUnapply,TRUE,TRUE));
		strWarningBody += str;
	}

	if(bPatient) {
		str.Format("  - %s of Patient Payments/Adjustments need to be unapplied from this charge.\n"
				   "    (This amount will then be transferred to insurance responsibility.)\n",FormatCurrencyForInterface(cyPatientTotalToUnapply,TRUE,TRUE)); 
		strWarningBody += str;
	}

	strWarningBody += "\n";

	strWarningBody += "Do you wish to still post to this charge?";

	strWarningToLog = strWarningHeader + "\n" + strWarningBody;

	// (b.eyers 2016-04-21 12:11) - NX-100187
	if (bInsurance && bPatient) {
		strUnapply.Format("%s and patient", strPatInsCoName);
	}
	else if (bInsurance) {
		strUnapply.Format("%s", strPatInsCoName);
	}
	else if (bPatient) {
		strUnapply.Format("patient");
	}

	// (j.jones 2016-04-20 17:16) - NX-100251 - this is now an NxTaskDialog
	NxTaskDialog dlgWarning(this);
	dlgWarning.Config()
		.WarningIcon()
		.ZeroButtons()
		.MainInstructionText(strWarningHeader)
		.ContentText(strWarningBody)
		.AddCommand(UnapplyWarningResult::eSkip, "Skip This Charge\nSkip this charge and continue to process the rest of the EOB")
		.AddCommand(UnapplyWarningResult::eProcess, FormatString("Post Payments / Adjustments\nThis will unapply existing %s credits and post the above payments/adjustments", strUnapply))
		.AddCommand(UnapplyWarningResult::eCancel, "Cancel\nStop processing this EOB")
		.DefaultButton(UnapplyWarningResult::eSkip);
	return (UnapplyWarningResult)dlgWarning.DoModal();
}

// (j.jones 2009-02-13 13:23) - PLID 33081 - added ability to allow applying overpayments, with the remainder unapplied
// (j.jones 2009-04-07 15:04) - PLID 33862 - removed bDuplicate
// (j.jones 2011-11-02 09:01) - PLID 46232 - renamed bReduceAdjustments to bCanReduceAdjustments, so we know which boolean it represents from EnsureValidResponsibilities
// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
// (j.jones 2016-04-20 17:00) - NX-100251 - this now creates its own messagebox and returns an enum for what the user chose to do
CEOBDlg::UnapplyWarningResult CEOBDlg::GenerateOverpaymentWarning(long nChargeID, long nPatientID,long nPatientUserDefinedID, COleCurrency cyInsBalance, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, CString &strWarningToLog, BOOL bCanReduceAdjustments, CString strPatInsCoName) {

	CString strWarningHeader, strWarningBody, str;

	CString strPatientName = GetExistingPatientName(nPatientID);
	CString strItemDesc;

	//COleCurrency cyInsBalanceNeeded = cyPaymentAmount + cyAdjustmentAmount;

	// (b.eyers 2016-04-21 12:11) - NX-100187
	_RecordsetPtr rs = CreateParamRecordset("SELECT ItemCode FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.ID = {INT}", nChargeID);
	if (!rs->eof) {
		strItemDesc.Format("%s", AdoFldString(rs, "ItemCode", ""));
	}
	rs->Close();

	// (b.eyers 2016-04-15 12:11) - NX-100187 - grab the insurance company name from the dropdown
	CString strEOBInsCoName = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(), iccName));
	// (b.eyers 2016-04-21 12:11) - NX-100187
	strWarningHeader.Format("Warning for %s (%li) for %s", strPatientName, nPatientUserDefinedID, strEOBInsCoName);

	// (j.jones 2008-03-26 10:28) - PLID 29407 - include the words "up to" when referencing the adjustment amount,
	// if bReduceAdjustments is true
	// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
	// (b.eyers 2016-04-15 12:11) - NX-100187 - Add name of the eob check ins co and name of ins co the $ is being applied to
	str.Format("This EOB states to post %s in payments from %s and %s%s in adjustments%s "
		"but Nextech was unable to acquire enough insurance responsibility to make the necessary applies. \n\n"
		"For this to post correctly: \n",
		FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE), strPatInsCoName, 
		bCanReduceAdjustments ? "up to " : "", FormatCurrencyForInterface(cyAdjustmentAmount, TRUE, TRUE),
		nChargeID == -1 ? "," : " to the charge: " + strItemDesc + ",");
	
	strWarningBody += str;

	// (b.eyers 2016-04-15 12:11) - NX-100187 
	str.Format("  - %s of the Insurance Payments/Adjustments will be applied to this charge.\n", 
			   FormatCurrencyForInterface(cyInsBalance,TRUE,TRUE), strPatInsCoName);
	strWarningBody += str;

	COleCurrency cyRemainingPayment = cyPaymentAmount - cyInsBalance;

	if(cyRemainingPayment > COleCurrency(0,0)) {
		str.Format("  - %s of the Insurance Payments will be left unapplied on the patient's account.\n", 
				   FormatCurrencyForInterface(cyRemainingPayment,TRUE,TRUE));
		strWarningBody += str;
	}

	if(!bCanReduceAdjustments) {

		COleCurrency cyRemainingAdjustment = COleCurrency(0,0);
		if(cyRemainingPayment > COleCurrency(0,0)) {
			cyRemainingAdjustment = cyAdjustmentAmount;
		}
		else {
			cyRemainingAdjustment = cyAdjustmentAmount - (cyInsBalance - cyPaymentAmount);
		}

		if(cyRemainingAdjustment > COleCurrency(0,0)) {
			str.Format("  - %s of the Insurance Adjustments will be left unapplied on the patient's account.\n", 
					   FormatCurrencyForInterface(cyRemainingAdjustment,TRUE,TRUE));
			strWarningBody += str;
		}
	}

	strWarningBody += "\n";

	strWarningBody += "Do you wish to still post to this charge?";

	strWarningToLog = strWarningHeader + "\n" + strWarningBody;

	// (j.jones 2016-04-20 17:16) - NX-100251 - this is now an NxTaskDialog
	NxTaskDialog dlgWarning(this);
	dlgWarning.Config()
		.WarningIcon()
		.ZeroButtons()
		.MainInstructionText(strWarningHeader)
		.ContentText(strWarningBody)
		.AddCommand(UnapplyWarningResult::eSkip, "Skip This Charge\nSkip this charge and continue to process the rest of the EOB")
		.AddCommand(UnapplyWarningResult::eProcess, "Post Payments / Adjustments\nThis will post the above payments/adjustments, leaving an overpayment unapplied")
		.AddCommand(UnapplyWarningResult::eCancel, "Cancel\nStop processing this EOB")
		.DefaultButton(UnapplyWarningResult::eSkip);
	return (UnapplyWarningResult)dlgWarning.DoModal();
}

void CEOBDlg::EnableButtons(BOOL bEnabled) {

	m_bIsLoading = !bEnabled;

	GetDlgItem(IDC_BTN_IMPORT_835_FILE)->EnableWindow(bEnabled);
	GetDlgItem(IDOK)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PRINT_EOB)->EnableWindow(bEnabled);
	GetDlgItem(IDCANCEL)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EOB_PROVIDER_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EOB_LOCATION_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SHIFT_RESP_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHECK_SHIFT_POSTED_AMOUNTS)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHECK_BATCH_CLAIMS)->EnableWindow(bEnabled);
	// (j.jones 2008-12-19 09:49) - PLID 32527 - added more buttons to be disabled
	GetDlgItem(IDC_BTN_IMPORT_OHIP_FILE)->EnableWindow(bEnabled);	
	// (r.gonet 2016-04-18) - NX-100162 - Renamed the button.
	GetDlgItem(IDC_BTN_ADJUSTMENT_CODE_SETTINGS)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_ADJ_CAT)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_PAY_CAT)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_ADJ_DESC)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_PAY_DESC)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PAY_DESC)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ADJ_DESC)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PAY_DESCRIPTION_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PAY_CAT_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ADJ_DESCRIPTION_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ADJ_CAT_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EOB_INSURANCE_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_BTN_AUTO_SKIP_DUPLICATES)->EnableWindow(bEnabled);
	// (j.jones 2010-02-09 08:53) - PLID 37174 - added ability to configure import filtering
	GetDlgItem(IDC_BTN_CONFIGURE_EREMIT_IMPORT_FILTERING)->EnableWindow(bEnabled);

	// (j.jones 2010-11-30 10:18) - PLID 41655 - enable the date controls accordingly
	m_checkEnablePayDate.EnableWindow(bEnabled);
	m_checkEnableAdjDate.EnableWindow(bEnabled);
	GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(bEnabled && m_checkEnablePayDate.GetCheck());
	GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(bEnabled && m_checkEnableAdjDate.GetCheck());
}

//DRT 4/10/2007 - PLID 25564 - Removed PeekAndPump in favor of a global version.

BEGIN_EVENTSINK_MAP(CEOBDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEOBDlg)
	ON_EVENT(CEOBDlg, IDC_PAY_DESCRIPTION_COMBO, 16 /* SelChosen */, OnSelChosenPayDescriptionCombo, VTS_DISPATCH)
	ON_EVENT(CEOBDlg, IDC_ADJ_DESCRIPTION_COMBO, 16 /* SelChosen */, OnSelChosenAdjDescriptionCombo, VTS_DISPATCH)
	ON_EVENT(CEOBDlg, IDC_EOB_INSURANCE_COMBO, 16 /* SelChosen */, OnSelChosenEobInsuranceCombo, VTS_I4)
	ON_EVENT(CEOBDlg, IDC_EOB_INSURANCE_COMBO, 18, CEOBDlg::OnRequeryFinishedEobInsuranceCombo, VTS_I2)
	ON_EVENT(CEOBDlg, IDC_EOB_DETAIL_LIST, 9, CEOBDlg::EditingFinishingEobDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEOBDlg, IDC_EOB_DETAIL_LIST, 10, CEOBDlg::EditingFinishedEobDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEOBDlg, IDC_EOB_DETAIL_LIST, 8, CEOBDlg::EditingStartingEobDetailList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEOBDlg, IDC_EOB_DETAIL_LIST, 6, CEOBDlg::RButtonDownEobDetailList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEOBDlg, IDC_EOB_DETAIL_LIST, 19, CEOBDlg::LeftClickEobDetailList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()



void CEOBDlg::Print(long EOBID) 
{
	try {

		CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(537)];
		infReport.nExtraID = EOBID;

		//check to see if there is a default report
		// (j.armen 2012-02-20 11:10) - PLID 34344 - Parameratized
		_RecordsetPtr rsDefault = CreateParamRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = {INT}", 537);
		CString strFileName;

		if (rsDefault->eof) {
			strFileName = "EOB";
		}
		else {
			
			long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

			if (nDefaultCustomReport > 0) {

				_RecordsetPtr rsFileName = CreateParamRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 537 AND Number = {INT}", nDefaultCustomReport);

				if (rsFileName->eof) {

					//this should never happen
					MsgBox("Practice could not find the custom report.  Please contact NexTech for assistance");
				}
				else {
					
					//set the default
					infReport.nDefaultCustomReport = nDefaultCustomReport;
					strFileName =  AdoFldString(rsFileName, "FileName");
				}
			}
			else {
				strFileName = "EOB";
			}
		}			

		////////////////////////////////////////////////////////////////////////////////////////////

		//we processed the EOB, so run from the data
		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, (CWnd *)this, "Electronic Remittance");
		
	}NxCatchAll("Error printing EOB.");
}

void CEOBDlg::OnPrintEob() 
{
	try {

		if(m_EOBList->GetRowCount() == 0) {
			MessageBox("You have not imported a remittance file that applies to any existing bills.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		/*
		if(IDNO == MessageBox("This will print the contents of this EOB - no data will be posted.\r"
			"If you choose to print this file, it is recommended that you set your printer to landscape mode.\r\r"
			"Are you ready to preview the EOB now?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}
		*/

		CWaitCursor pWait;

		CString strSql, strInsCoName, strPayee, strTotalPayment, strOverageWarning;

		GetDlgItemText(IDC_INSCO_NAME_LABEL, strInsCoName);
		GetDlgItemText(IDC_PROVIDER_NAME_LABEL, strPayee);
		GetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL, strTotalPayment);
		// (j.jones 2009-07-06 17:15) - PLID 34776 - supported OverageWarning
		GetDlgItemText(IDC_ESTIMATED_OVERPAYMENT_LABEL, strOverageWarning);

		if(strOverageWarning.GetLength() > 255) {
			//limit this to 255 characters, but assert incase this ever actually happened
			ASSERT(FALSE);
			strOverageWarning = strOverageWarning.Left(255);
		}
		// (s.tullis 2016-04-15 16:00) - NX-100211
		NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr;
		for(pLoopRowPtr=m_EOBList->GetFirstRow();pLoopRowPtr!=nullptr;pLoopRowPtr = pLoopRowPtr->GetNextRow()) {			

			CString strPatientName, strCPTCode;
			COleDateTime dtChargeDate, dtBirthDate, dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleCurrency cyInvalid;
			cyInvalid.SetStatus(COleCurrency::invalid);
			COleCurrency cyCharge, cyPayment, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjustment, cyOtherResp, cyPatApplies, cyPatBal, cyInsBal, cyFeeSchedAllowable, cyEOBAllowable;

			strPatientName = VarString(pLoopRowPtr->GetValue(COLUMN_PATIENT_NAME),"");
			dtChargeDate = VarDateTime(pLoopRowPtr->GetValue(COLUMN_CHARGE_DATE), COleDateTime::GetCurrentTime());
			strCPTCode = VarString(pLoopRowPtr->GetValue(COLUMN_CPT_CODE),"");
			cyCharge = VarCurrency(pLoopRowPtr->GetValue(COLUMN_CHARGE_AMOUNT),COleCurrency(0,0));
			cyPayment = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));

			GetTotalChargeAdjustments(pLoopRowPtr, cyTotalPositiveAdj, cyTotalNegativeAdj, cyTotalAdjustment);

			// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed this from patient resp. to "other" resp., because
			// even though the EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
			cyOtherResp = VarCurrency(pLoopRowPtr->GetValue(COLUMN_OTHER_RESP),COleCurrency(0,0));
			cyPatApplies = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_APPLIES),COleCurrency(0,0));
			cyPatBal = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAT_BALANCE),COleCurrency(0,0));
			cyInsBal = VarCurrency(pLoopRowPtr->GetValue(COLUMN_INS_BALANCE), COleCurrency(0, 0));
			// (j.jones 2011-03-18 10:29) - PLID 42157 - added FeeSchedAllowable and EOBAllowable
			cyFeeSchedAllowable = VarCurrency(pLoopRowPtr->GetValue(COLUMN_FEE_SCHED_ALLOWABLE), cyInvalid);
			cyEOBAllowable = VarCurrency(pLoopRowPtr->GetValue(COLUMN_EOB_ALLOWABLE), cyInvalid);

			// (j.jones 2008-12-16 10:14) - PLID 32317 - added columns for patient birthdate and OHIP Health Card Number and Version Code			
			dtBirthDate = VarDateTime(pLoopRowPtr->GetValue(COLUMN_BIRTHDATE), dtInvalid);
			CString strOHIPHealthCardNum = VarString(pLoopRowPtr->GetValue(COLUMN_OHIP_HEALTH_NUM),"");
			CString strOHIPVersionCode = VarString(pLoopRowPtr->GetValue(COLUMN_OHIP_VERSION_CODE),"");

			//DRT 6/18/2004 - PLID 13055 - If you are selecting things that have a possibility of being the same
			//	(as these do), then you must use 'UNION ALL', which allows duplicates between sections.  The
			//	default UNION behavior will remove all duplicates.
			// (j.jones 2008-12-16 10:07) - PLID 32317 - added birthdate, OHIP health card number, and OHIP
			// version code to the query
			// (j.jones 2009-07-06 17:15) - PLID 34776 - supported OverageWarning
			// (j.jones 2011-03-18 10:29) - PLID 42157 - added FeeSchedAllowable and EOBAllowable
			// (j.jones 2011-11-08 14:57) - PLID 46240 - the cyPatResp variable has been renamed to cyOtherResp, but it
			// should still be in the patient resp. field in this report
			CString strSql2;
			strSql2.Format("UNION ALL SELECT '%s' AS PatientName, '%s' AS ChargeDate, '%s' AS ServiceCode, "
				"'%s' AS ChargeAmt, '%s' AS PayAmt, '%s' AS AdjAmt, '%s' AS PatResp, '%s' AS PatApplies, "
				"'%s' AS PatBal, '%s' AS InsBal, '%s' AS InsCoName, '%s' AS Payee, '%s' AS TotalPayment, "
				"'%s' AS BirthDate, '%s' AS OHIPHealthCardNum, '%s' AS OHIPVersionCode, '%s' AS OverageWarning, "
				"'%s' AS FeeSchedAllowable, '%s' AS EOBAllowable ",
				_Q(strPatientName), _Q(FormatDateTimeForInterface(dtChargeDate,dtoDate)),
				_Q(strCPTCode), _Q(FormatCurrencyForInterface(cyCharge)), _Q(FormatCurrencyForInterface(cyPayment)),
				_Q(FormatCurrencyForInterface(cyTotalAdjustment)), _Q(FormatCurrencyForInterface(cyOtherResp)),
				_Q(FormatCurrencyForInterface(cyPatApplies)), _Q(FormatCurrencyForInterface(cyPatBal)),
				_Q(FormatCurrencyForInterface(cyInsBal)), _Q(strInsCoName), _Q(strPayee), _Q(strTotalPayment),
				_Q(dtBirthDate.GetStatus() == COleDateTime::invalid ? "" : FormatDateTimeForInterface(dtBirthDate,dtoDate)),
				_Q(strOHIPHealthCardNum), _Q(strOHIPVersionCode), _Q(strOverageWarning),
				_Q(cyFeeSchedAllowable.GetStatus() == COleCurrency::invalid ? "" : FormatCurrencyForInterface(cyFeeSchedAllowable)),
				_Q(cyEOBAllowable.GetStatus() == COleCurrency::invalid ? "" : FormatCurrencyForInterface(cyEOBAllowable)));

			strSql += strSql2;
		}

		strSql.TrimLeft("UNION ALL ");		

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(541)]);
		infReport.strExtendedSql = strSql;	//pass in the subquery through the extended filter

		//send directly to the printer
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		CPrintInfo prInfo;
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if (prInfo.m_pPD) delete prInfo.m_pPD;
		prInfo.m_pPD = dlg;

		RunReport(&infReport, false, this, "EOB Preview", &prInfo);

	}NxCatchAll("Error printing EOB.");	
}

// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct
// (j.armen 2012-02-20 11:11) - PLID 48244 - Parameratized
// (j.jones 2012-05-01 15:02) - PLID 47477 - supported the ProcessedAs flag
// (j.jones 2013-07-10 11:20) - PLID 57263 - replaced the ProcessedAs variable with the claim pointer
// (j.jones 2016-04-13 16:20) - NX-100184 - added EOB pointer, removed redundant parameters that already existed in pClaim and pEOB,
EInsuredPartyInfo CEOBDlg::CalcIndivPatientInsuredPartyID(const EOBInfo* pEOB, const EOBClaimInfo *pClaim, const long& nChargeID)
{
	if (pEOB == NULL || pClaim == NULL) {
		//this should not be possible
		ASSERT(FALSE);
		ThrowNxException("CalcIndivPatientInsuredPartyID called with invalid claim pointers.");
	}

	// (j.jones 2016-04-21 15:09) - NX-100184 - first see if this patient has one and only one insured
	// party with the EOB's payer ID, and if so, return it
	CString strPayerIDTrim = pEOB->strPayerID;
	strPayerIDTrim.Trim();
	if (m_ERemitType == ertAnsi && strPayerIDTrim != "") {
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON; "
			"DECLARE @InsCoIDs TABLE (ID INT); "
			"DELETE FROM @InsCoIDs; "
			""
			"INSERT INTO @InsCoIDs(ID) "
			"SELECT DISTINCT InsuranceCoT.PersonID AS InsCoID "
			"FROM InsuranceCoT "
			"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
			"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID "
			"LEFT JOIN EbillingInsCoIDs LocationPayerIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = LocationPayerIDs.ID "
			"WHERE InsuredPartyT.PatientID = {INT} "
			"AND (EbillingInsCoIDs.EbillingID = {STRING} OR LocationPayerIDs.EbillingID = {STRING}); "
			""
			"DECLARE @CountIDs INT; "
			"SET @CountIDs = (SELECT Count(ID) FROM @InsCoIDs);"
			""
			"SET NOCOUNT OFF; "
			"SELECT PersonID, RespTypeID, CategoryPlacement "
			"FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN @InsCoIDs I ON InsuredPartyT.InsuranceCoID = I.ID "
			"WHERE PatientID = {INT} AND @CountIDs = 1; ",
			pClaim->nPatientID, strPayerIDTrim, strPayerIDTrim,
			pClaim->nPatientID);
		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//great, we found one match, so use that insured party
				EInsuredPartyInfo eInsuredInfo;
				eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
				eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

				Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding one match by payer ID.", eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
				return eInsuredInfo;
			}
		}
	}

	// (j.jones 2016-04-13 16:20) - NX-100184 - Previously all this logic could be called with or without
	// respecting the "Processed As" flag in determining the insured party. Now we will run through the logic
	// twice. Once with "Processed As" respected, once without.
	// If either result suggests a non-Primary payer, use that non-Primary payer.

	EInsuredPartyInfo eInsuredInfo_PolicyInfoOnly = CalcIndivPatientInsuredPartyID_ByPolicyInfo(pEOB, pClaim, nChargeID, false);
	EInsuredPartyInfo eInsuredInfo_WithProcessedAs = CalcIndivPatientInsuredPartyID_ByPolicyInfo(pEOB, pClaim, nChargeID, true);

	if (eInsuredInfo_PolicyInfoOnly.nInsuredPartyID == -1 && eInsuredInfo_WithProcessedAs.nInsuredPartyID == -1) {
		//yikes, neither found a match!

		Log("CalcIndivPatientInsuredPartyID: Found no insured party for Patient ID (%li) by either policy info or Processed As flag.", pClaim->nPatientID);

		//this is effectively returning nothing
		return eInsuredInfo_WithProcessedAs;
	}
	else if (eInsuredInfo_PolicyInfoOnly.nInsuredPartyID == -1 && eInsuredInfo_WithProcessedAs.nInsuredPartyID != -1) {
		//we found a match only by using the ProcessedAs flag

		Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
			"finding one match only when using the Processed As flag.",
			eInsuredInfo_WithProcessedAs.nInsuredPartyID, pClaim->nPatientID);
		return eInsuredInfo_WithProcessedAs;
	}
	else if (eInsuredInfo_PolicyInfoOnly.nInsuredPartyID != -1 && eInsuredInfo_WithProcessedAs.nInsuredPartyID == -1) {
		//we found a match only by using the Policy Info and not ProcessedAs

		Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
			"finding one match only when ignoring the Processed As flag.",
			eInsuredInfo_PolicyInfoOnly.nInsuredPartyID, pClaim->nPatientID);
		return eInsuredInfo_PolicyInfoOnly;
	}
	
	//now all remaining logic assumes we have two valid insured parties

	if (eInsuredInfo_PolicyInfoOnly.nInsuredPartyID != -1 && eInsuredInfo_WithProcessedAs.nInsuredPartyID != -1) {

		//are they the same insured party?
		if (eInsuredInfo_PolicyInfoOnly.nInsuredPartyID == eInsuredInfo_WithProcessedAs.nInsuredPartyID) {
			//this is the ideal case: both logical approaches found the same insured party

			Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"finding the same match both with and without the Processed As flag.",
				eInsuredInfo_PolicyInfoOnly.nInsuredPartyID, pClaim->nPatientID);
			return eInsuredInfo_PolicyInfoOnly;
		}

		//If we get here, now we are in a dilemma - calculating with and without the "Processed As" flag
		//calculated different insured parties.

		//of the two calculated payers, does only one match our payer ID?
		if (m_ERemitType == ertAnsi && strPayerIDTrim != "") {
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, RespTypeT.CategoryPlacement "
				"FROM InsuredPartyT "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
				"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID "
				"LEFT JOIN EbillingInsCoIDs LocationPayerIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = LocationPayerIDs.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"AND (EbillingInsCoIDs.EbillingID = {STRING} OR LocationPayerIDs.EbillingID = {STRING}) "
				"AND InsuredPartyT.PersonID IN ({INT},{INT})",
				pClaim->nPatientID,
				strPayerIDTrim, strPayerIDTrim,
				eInsuredInfo_PolicyInfoOnly.nInsuredPartyID, eInsuredInfo_WithProcessedAs.nInsuredPartyID);
			if (!rs->eof) {
				if (rs->GetRecordCount() == 1) {
					//great, we found one match, so use that insured party
					EInsuredPartyInfo eInsuredInfo;
					eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
					eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

					Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
						"matching two insured parties with & without the Processed As flag, and choosing the result from one match by payer ID.", eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
					return eInsuredInfo;
				}
			}
		}

		//neither match by payer ID! Now defer to whichever one, if any, is Secondary.

		if (eInsuredInfo_PolicyInfoOnly.nCategoryPlacement == 1 && eInsuredInfo_WithProcessedAs.nCategoryPlacement == 2) {
			//the Processed As logic found a Secondary, use it

			Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"matching to a Secondary payer using the Processed As flag.",
				eInsuredInfo_WithProcessedAs.nInsuredPartyID, pClaim->nPatientID);
			return eInsuredInfo_WithProcessedAs;
		}
		else if (eInsuredInfo_PolicyInfoOnly.nCategoryPlacement == 2 && eInsuredInfo_WithProcessedAs.nCategoryPlacement == 1) {
			//the non-Processed As logic found a Secondary, use it

			Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"matching to a Secondary payer without using the Processed As flag.",
				eInsuredInfo_PolicyInfoOnly.nInsuredPartyID, pClaim->nPatientID);
			return eInsuredInfo_PolicyInfoOnly;
		}

		//if we get here we have a problem, because one of the following cases occurred:
		// - an inactive or tertiary resp was found, which has no CategoryPlacement
		// - the CategoryPlacements are identical, which means payers for two different categories

		//Look for whoever has the largest balance, and if neither have a balance,
		//CalcInsuredPartyIDByResp will pick the first record in the recordset,
		//which we now need to define.

		CString strProcessedAsSort = "ASC";
		//if ProcessedAs found a secondary category placement, give priority to that calculation's result
		//otherwise give priority to the PolicyInfo calculation's result
		if (eInsuredInfo_WithProcessedAs.nCategoryPlacement == 2) {
			strProcessedAsSort = "DESC";
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, CategoryPlacement FROM "
			"("
			"SELECT InsuredPartyT.PersonID, RespTypeT.CategoryPlacement, 0 AS ProcessedAs "
			"FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PersonID = {INT} "
			"UNION "
			"SELECT InsuredPartyT.PersonID, RespTypeT.CategoryPlacement, 1 AS ProcessedAs "
			"FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PersonID = {INT} "
			") AS RespsQ "
			"ORDER BY ProcessedAs {CONST_STR}",
			eInsuredInfo_PolicyInfoOnly.nInsuredPartyID, eInsuredInfo_WithProcessedAs.nInsuredPartyID,
			strProcessedAsSort);
		
		//we only want the highest balance, NOT highest resp if no balance is found
		EInsuredPartyInfo eInsuredInfoByResp = CalcInsuredPartyIDByResp(rs, nChargeID, pClaim->nPatientID, true);
		//this is either the insured party with the highest balance, or the ProcessedAs party,
		//chosen arbritrarily if neither has a higher balance
		Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
			"matching two insured parties with & without the Processed As flag, and choosing the result from CalcInsuredPartyIDByResp.",
			eInsuredInfoByResp.nInsuredPartyID, pClaim->nPatientID);
		return eInsuredInfoByResp;
	}

	//it should be logically impossible to get here,
	//if you do, add a case to handle it correctly
	ASSERT(FALSE);
	//use the ProcessedAs result
	Log("CalcIndivPatientInsuredPartyID: Calculated Insured Party ID (%li) for Patient ID (%li) by "
		"the catch-all unknown case result.",
		eInsuredInfo_WithProcessedAs.nInsuredPartyID, pClaim->nPatientID);
	return eInsuredInfo_WithProcessedAs;
}

// (j.jones 2016-04-13 16:20) - NX-100184 - made a new version of this function which contains all the old
// policy matching logic with optional usage of the ProcessedAs flag
EInsuredPartyInfo CEOBDlg::CalcIndivPatientInsuredPartyID_ByPolicyInfo(const EOBInfo* pEOB, const EOBClaimInfo *pClaim, const long& nChargeID, bool bUseProcessedAsFlag)
{
	if (pEOB == NULL || pClaim == NULL) {
		//this should not be possible
		ASSERT(FALSE);
		ThrowNxException("CalcIndivPatientInsuredPartyID_ByPolicyInfo called with invalid claim pointers.");
	}

	EInsuredPartyInfo eInsuredInfo;
	eInsuredInfo.nInsuredPartyID = -1;
	eInsuredInfo.nCategoryPlacement = -1;

	// (j.jones 2016-04-13 16:55) - NX-100184 - use the sqlExcludeRespTypeFilter only when filtering using the ProcessedAs info
	CSqlFragment sqlExcludeRespTypeFilter("(1=1)");
	if (bUseProcessedAsFlag && pClaim->nProcessedAs > 1) {
		//if ProcessedAs is Primary, we don't alter our filters, we do NOT want to exclude any payers when filtering by Primary
		//if ProcessedAs is Secondary or Tertiary, just filter out Primary insured parties, do not try to filter by RespType.Priority
		sqlExcludeRespTypeFilter = CSqlFragment("(RespTypeT.CategoryPlacement <> 1 AND InsuredPartyT.SubmitAsPrimary <> 1)");
	}

	CString strPayerIDTrim = pEOB->strPayerID;
	strPayerIDTrim.Trim();

	// (j.jones 2005-04-20 15:49) - It is possible that a patient may have the same insurance company
	// for both their primary and secondary, so once again we need to try to find out which one
	// corresponds to the policy or group numbers.
	
	CSqlFragment sqlHCFAGroupFilter("AND InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = {INT})", pEOB->nHCFAGroupID);

	CStringArray arystrInsuranceIDs;

	if (pClaim->strOriginalPatientInsuranceID.GetLength() > 0) {

		//also search for the group number
		if (!pClaim->strGroupOrPolicyNum.IsEmpty()) {
			arystrInsuranceIDs.Add(pClaim->strGroupOrPolicyNum);
		}

		// (j.jones 2011-03-16 16:36) - PLID 42866 - also search for strCorrectedInsuranceID
		if (!pClaim->strCorrectedPatientInsuranceID.IsEmpty()) {
			if (pClaim->strCorrectedPatientInsuranceID.Find(" ") != -1) {
				CString strInsuranceID1, strInsuranceID2;
				strInsuranceID1 = pClaim->strCorrectedPatientInsuranceID.Left(pClaim->strCorrectedPatientInsuranceID.Find(" "));
				strInsuranceID1.TrimRight();
				strInsuranceID2 = pClaim->strCorrectedPatientInsuranceID.Right(pClaim->strCorrectedPatientInsuranceID.GetLength() - pClaim->strCorrectedPatientInsuranceID.Find(" "));
				strInsuranceID2.TrimLeft();

				arystrInsuranceIDs.Add(pClaim->strCorrectedPatientInsuranceID);
				arystrInsuranceIDs.Add(strInsuranceID1);
				arystrInsuranceIDs.Add(strInsuranceID2);
			}
			else {
				arystrInsuranceIDs.Add(pClaim->strCorrectedPatientInsuranceID);
			}
		}

		if (pClaim->strOriginalPatientInsuranceID.Find(" ") != -1) {
			// (j.jones 2005-04-15 14:23) - we could potentially have multiple IDs to test with,
			// we should test with the whole string and with the string broken at the space
			// incase they sent both IDs on one line.
			CString strInsuranceID1 = pClaim->strOriginalPatientInsuranceID.Left(pClaim->strOriginalPatientInsuranceID.Find(" "));
			strInsuranceID1.TrimRight();
			CString strInsuranceID2 = pClaim->strOriginalPatientInsuranceID.Right(pClaim->strOriginalPatientInsuranceID.GetLength() - pClaim->strOriginalPatientInsuranceID.Find(" "));
			strInsuranceID2.TrimLeft();

			arystrInsuranceIDs.Add(pClaim->strOriginalPatientInsuranceID);
			arystrInsuranceIDs.Add(strInsuranceID1);
			arystrInsuranceIDs.Add(strInsuranceID2);
		}
		else {
			// (j.jones 2005-04-15 14:23) - we just have one ID to test with
			arystrInsuranceIDs.Add(pClaim->strOriginalPatientInsuranceID);
		}

		// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT PersonID, RespTypeID, CategoryPlacement "
			"FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE PatientID = {INT} AND {SQL} "
			"	AND (IDForInsurance IN ({STRINGARRAY}) OR PolicyGroupNum IN ({STRINGARRAY})) "
			"	{SQL}",
			pClaim->nPatientID, sqlExcludeRespTypeFilter,
			arystrInsuranceIDs, arystrInsuranceIDs, sqlHCFAGroupFilter);
		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//great, we found one match, so use that insured party
				// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
				eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
				eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

				Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding one match by policy number.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
				return eInsuredInfo;
			}
			else {
				//more than one match? that can't possibly be good
				//this would mean that they have multiple insured parties that use the same policy number and HCFA group

				//so try to find the insured party that has a balance for this charge, or at least responsibility
				// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
				eInsuredInfo = CalcInsuredPartyIDByResp(rs, nChargeID, pClaim->nPatientID);

				Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding multiple matches by policy number and filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
				return eInsuredInfo;
			}
		}
		rs->Close();

		// (j.jones 2012-05-01 14:55) - PLID 47477 - if we get here, we should try to use the "processed as" flag,
		// because sometimes "processed as secondary" is reported but with the primary ID for insurance, in cases
		// where primary auto-forwarded to secondary
		// (j.jones 2016-04-13 16:55) - NX-100184 - only do this if we're being asked to respect the "Processed As" flag
		if (bUseProcessedAsFlag && pClaim->nProcessedAs != -1) {
			eInsuredInfo = CalcInsuredPartyIDByProcessedAs(nChargeID, pClaim->nPatientID, pClaim->nProcessedAs, pEOB->nHCFAGroupID);
			if (eInsuredInfo.nInsuredPartyID != -1) {
				//this succeeded at finding an insured party
				return eInsuredInfo;
			}
		}

		//If we get here, we have a policy number but didn't find a company in the given HCFA Group,
		//so we're going to try something a little shady - find just the policy number for a company OUTSIDE the HCFA Group.
		//It's a semi-plausible scenario, maybe they have different HCFA Settings for one BCBS variant than another, but
		//one check pays both companies. Uncommon, but could potentially happen. So...
		// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
		rs = CreateParamRecordset("SELECT PersonID, RespTypeID, CategoryPlacement "
			"FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE PatientID = {INT} AND {SQL} "
			"	AND (IDForInsurance IN ({STRINGARRAY}) OR PolicyGroupNum IN ({STRINGARRAY}))",
			pClaim->nPatientID, sqlExcludeRespTypeFilter,
			arystrInsuranceIDs, arystrInsuranceIDs);
		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//okay, we found one match, so we were right to check this. Use that insured party.
				// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
				eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
				eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

				Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding one match by policy number OUTSIDE of the specified HCFA Group of (%li).", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
				return eInsuredInfo;
			}
			else {
				//you have got to be kidding me!
				//this would mean that they have multiple insured parties that use the same policy number
				//AND not under the main HCFA Group we are using

				//so try to find the insured party that has a balance for this charge, or at least responsibility
				eInsuredInfo = CalcInsuredPartyIDByResp(rs, nChargeID, pClaim->nPatientID);

				Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding multiple matches by policy number OUTSIDE of the specified HCFA Group of (%li), and further filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
				return eInsuredInfo;
			}
		}
		rs->Close();
	}

	//if we get here, it means we could not determine an insured party based on policy number, which will likely happen a lot
	// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
	_RecordsetPtr rs = CreateParamRecordset(
		"SELECT PersonID, RespTypeID, CategoryPlacement "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE PatientID = {INT} AND {SQL} "
		"	{SQL}",
		pClaim->nPatientID, sqlExcludeRespTypeFilter,
		sqlHCFAGroupFilter);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//we found one match for this patient for this HCFA Group, which is excellent. Use it.
			// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
			eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
			eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

			Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"finding one matching company for the specified HCFA Group of (%li).", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
			return eInsuredInfo;
		}
		else {
			//the downfall of using HCFA Groups - we found multiple companies that use this HCFA Group

			// (j.jones 2012-05-01 14:55) - PLID 47477 - if we get here, we should try to use the "processed as" flag,
			// because sometimes "processed as secondary" is reported but with the primary ID for insurance, in cases
			// where primary auto-forwarded to secondary
			// (j.jones 2016-04-13 16:55) - NX-100184 - only do this if we're being asked to respect the "Processed As" flag
			if (bUseProcessedAsFlag && pClaim->nProcessedAs != -1) {
				eInsuredInfo = CalcInsuredPartyIDByProcessedAs(nChargeID, pClaim->nPatientID, pClaim->nProcessedAs, pEOB->nHCFAGroupID);
				if (eInsuredInfo.nInsuredPartyID != -1) {
					//this succeeded at finding an insured party
					return eInsuredInfo;
				}
			}

			//if we get here, try to filter by company name
			// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
			_RecordsetPtr rs2 = CreateParamRecordset(
				"SELECT PersonID, RespTypeID, CategoryPlacement "
				"FROM InsuredPartyT "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND {SQL} "
				"	AND InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE Name = {STRING}) "
				"	{SQL}",
				pClaim->nPatientID, sqlExcludeRespTypeFilter,
				pEOB->strInsuranceCoName, sqlHCFAGroupFilter);
			if (!rs2->eof) {
				if (rs2->GetRecordCount() == 1) {
					//we found one match for this patient with the given company name. Use it.
					// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
					eInsuredInfo.nInsuredPartyID = AdoFldLong(rs2, "PersonID");
					eInsuredInfo.nCategoryPlacement = AdoFldLong(rs2, "CategoryPlacement", -1);

					Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
						"finding one matching company for the specified HCFA Group of (%li) and company name.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
					return eInsuredInfo;
				}
				else {
					//we found multiple companies of the same name				
					//so try to find the insured party that has a balance for this charge, or at least responsibility
					eInsuredInfo = CalcInsuredPartyIDByResp(rs2, nChargeID, pClaim->nPatientID);

					Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
						"finding multiple matching companies for the specified HCFA Group of (%li) and company name, and further filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
					return eInsuredInfo;
				}
			}
			rs2->Close();

			//try to filter by payer ID
			if (!strPayerIDTrim.IsEmpty()) {
				// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
				// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
				rs2 = CreateParamRecordset(
					"SELECT PersonID, RespTypeID, CategoryPlacement "
					"FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND {SQL} "
					"	AND InsuranceCoID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
					"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
					"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
					"	{SQL}",
					pClaim->nPatientID, sqlExcludeRespTypeFilter,
					strPayerIDTrim, sqlHCFAGroupFilter);
				if (!rs2->eof) {
					if (rs2->GetRecordCount() == 1) {
						//we found one match for this patient with the given payer ID. Use it.
						// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
						eInsuredInfo.nInsuredPartyID = AdoFldLong(rs2, "PersonID");
						eInsuredInfo.nCategoryPlacement = AdoFldLong(rs2, "CategoryPlacement", -1);

						Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
							"finding one matching company for the specified HCFA Group of (%li) and Payer ID.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
						return eInsuredInfo;
					}
					else {
						//we found multiple companies of the same name				
						//so try to find the insured party that has a balance for this charge, or at least responsibility
						eInsuredInfo = CalcInsuredPartyIDByResp(rs2, nChargeID, pClaim->nPatientID);

						Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
							"finding multiple matching companies for the specified HCFA Group of (%li) and Payer ID, and further filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
						return eInsuredInfo;
					}
				}
				rs2->Close();
			}

			//if we get here, we didn't reduce the list by name or payer ID matches		
			//so try to find the insured party that has a balance for this charge, or at least responsibility
			eInsuredInfo = CalcInsuredPartyIDByResp(rs, nChargeID, pClaim->nPatientID);

			Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"finding multiple matching companies for the specified HCFA Group of (%li), and further filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID, pEOB->nHCFAGroupID);
			return eInsuredInfo;
		}
	}
	rs->Close();

	//If we get here, we didn't find a company in the given HCFA Group, so now we just need to find one company
	// (j.jones 2013-07-10 10:29) - PLID 57263 - DON'T include the resp. type exclusion filter here,
	// if we find multiple companies we will filter by resp type.
	rs = CreateParamRecordset(
		"SELECT PersonID, RespTypeID, CategoryPlacement "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE PatientID = {INT} ",
		pClaim->nPatientID);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//we found one company for this patient, which is all we can do
			// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
			eInsuredInfo.nInsuredPartyID = AdoFldLong(rs, "PersonID");
			eInsuredInfo.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

			Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"finding only one company for this patient.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
			return eInsuredInfo;
		}
		else {
			//we found multiple companies, none of which use this HCFA Group

			// (j.jones 2012-05-01 14:55) - PLID 47477 - if we get here, we should try to use the "processed as" flag,
			// because sometimes "processed as secondary" is reported but with the primary ID for insurance, in cases
			// where primary auto-forwarded to secondary
			// (j.jones 2016-04-13 16:55) - NX-100184 - only do this if we're being asked to respect the "Processed As" flag
			if (bUseProcessedAsFlag && pClaim->nProcessedAs != -1) {
				eInsuredInfo = CalcInsuredPartyIDByProcessedAs(nChargeID, pClaim->nPatientID, pClaim->nProcessedAs, -1);
				if (eInsuredInfo.nInsuredPartyID != -1) {
					//this succeeded at finding an insured party
					return eInsuredInfo;
				}
			}

			//try to filter by company name
			_RecordsetPtr rs2 = CreateParamRecordset(
				"SELECT PersonID, RespTypeID, CategoryPlacement "
				"FROM InsuredPartyT "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} "
				"	AND InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE Name = {STRING})",
				pClaim->nPatientID, pEOB->strInsuranceCoName);
			if (!rs2->eof) {
				if (rs2->GetRecordCount() == 1) {
					//we found one match for this patient with the given company name. Use it.
					// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
					eInsuredInfo.nInsuredPartyID = AdoFldLong(rs2, "PersonID");
					eInsuredInfo.nCategoryPlacement = AdoFldLong(rs2, "CategoryPlacement", -1);

					Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
						"finding only one company for this patient that matches the company name.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
					return eInsuredInfo;
				}
				else {
					//we found multiple companies of the same name				
					//so try to find the insured party that has a balance for this charge, or at least responsibility
					eInsuredInfo = CalcInsuredPartyIDByResp(rs2, nChargeID, pClaim->nPatientID);

					Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
						"finding multiple companies for this patient that matches the company name, further filtered by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
					return eInsuredInfo;
				}
			}
			rs2->Close();

			//try to filter by payer ID
			if (!strPayerIDTrim.IsEmpty()) {
				// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
				rs2 = CreateParamRecordset(
					"SELECT PersonID, RespTypeID, CategoryPlacement "
					"FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} "
					"	AND InsuranceCoID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
					"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
					"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) ",
					pClaim->nPatientID, strPayerIDTrim);
				if (!rs2->eof) {
					if (rs2->GetRecordCount() == 1) {
						//we found one match for this patient with the given company name. Use it.
						// (j.jones 2011-04-04 16:37) - PLID 42571 - changed the return value
						eInsuredInfo.nInsuredPartyID = AdoFldLong(rs2, "PersonID");
						eInsuredInfo.nCategoryPlacement = AdoFldLong(rs2, "CategoryPlacement", -1);

						Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
							"finding only one company for this patient that matches the Payer ID.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
						return eInsuredInfo;
					}
					else {
						//we found multiple companies of the same name				
						//so try to find the insured party that has a balance for this charge, or at least responsibility
						eInsuredInfo = CalcInsuredPartyIDByResp(rs2, nChargeID, pClaim->nPatientID);

						Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
							"finding multiple companies for this patient that matches the Payer ID, further filtered by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
						return eInsuredInfo;
					}
				}
				rs2->Close();
			}

			//if we get here, we didn't reduce the list by name or payer ID matches	

			//so try to find the insured party that has a balance for this charge, or at least responsibility
			eInsuredInfo = CalcInsuredPartyIDByResp(rs, nChargeID, pClaim->nPatientID);

			Log("CalcIndivPatientInsuredPartyID_ByPolicyInfo (bUseProcessedAsFlag = %li): Calculated Insured Party ID (%li) for Patient ID (%li) by "
				"finding multiple companies for this patient, and further filtering by responsibility.", bUseProcessedAsFlag ? 1 : 0, eInsuredInfo.nInsuredPartyID, pClaim->nPatientID);
			return eInsuredInfo;
		}
	}
	rs->Close();

	return eInsuredInfo;
}

// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct
// (j.jones 2016-04-14 13:30) - NX-100184 - added an option to force checking balance only, and ignore
// the highest resp, will return arbitrary first result if no resp has a balance
EInsuredPartyInfo CEOBDlg::CalcInsuredPartyIDByResp(ADODB::_RecordsetPtr rs, long nChargeID, long nPatientID, bool bBalanceCheckOnly /*= false*/)
{
	EInsuredPartyInfo eBlankInsuredInfo;
	eBlankInsuredInfo.nInsuredPartyID = -1;
	eBlankInsuredInfo.nCategoryPlacement = -1;

	EInsuredPartyInfo eFirstInsuredParty = eBlankInsuredInfo;
	
	EInsuredPartyInfo eInsuredWithMostResp = eBlankInsuredInfo;
	COleCurrency cyMostInsuredResp = COleCurrency(0,0);

	EInsuredPartyInfo eInsuredWithMostBalance = eBlankInsuredInfo;
	COleCurrency cyMostInsuredBalance = COleCurrency(0,0);

	while(!rs->eof) {
		
		// (j.jones 2011-04-04 16:40) - PLID 42571 - supported the new struct
		long nCurInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
		long nCurInsuredPartyPlacement = AdoFldLong(rs, "CategoryPlacement",-1);

		if(eFirstInsuredParty.nInsuredPartyID == -1) {
			eFirstInsuredParty.nInsuredPartyID = nCurInsuredPartyID;
			eFirstInsuredParty.nCategoryPlacement = nCurInsuredPartyPlacement;
		}

		COleCurrency cyCharges = COleCurrency(0,0);
		COleCurrency cyPayments = COleCurrency(0,0);
		COleCurrency cyAdjustments = COleCurrency(0,0);
		COleCurrency cyRefunds = COleCurrency(0,0);

		if(GetChargeInsuranceTotals(nChargeID,nPatientID,nCurInsuredPartyID,&cyCharges,&cyPayments,&cyAdjustments,&cyRefunds)) {
			COleCurrency cyBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;

			if(cyCharges > cyMostInsuredResp) {
				cyMostInsuredResp = cyCharges;
				eInsuredWithMostResp.nInsuredPartyID = nCurInsuredPartyID;
				eInsuredWithMostResp.nCategoryPlacement = nCurInsuredPartyPlacement;
			}

			if(cyBalance > cyMostInsuredBalance) {
				cyMostInsuredBalance = cyBalance;
				eInsuredWithMostBalance.nInsuredPartyID = nCurInsuredPartyID;
				eInsuredWithMostBalance.nCategoryPlacement = nCurInsuredPartyPlacement;
			}
		}

		rs->MoveNext();
	}

	//use the insured party with the greatest open balance
	if(eInsuredWithMostBalance.nInsuredPartyID != -1)
		return eInsuredWithMostBalance;

	//if none have a balance, use the insured party with the greatest responsibility

	// (j.jones 2016-04-14 13:31) - NX-100184 - do not do this if the caller only wanted
	// the resp with the greatest balance
	if (!bBalanceCheckOnly && eInsuredWithMostResp.nInsuredPartyID != -1) {
		return eInsuredWithMostResp;
	}
	
	//if none have any responsibility, use the first insured party we came across,
	//it's up to the caller to determine any desired sort order in the recordset
	return eFirstInsuredParty;
}

void CEOBDlg::AddWarningToLog(CString strWarning, CString strAction)
{
	if(!m_bWarningsCreated) {

		m_strWarningOutputFile = GetNxTempPath() ^ "EOBWarnings.txt";

		if(!m_fileLogWarnings.Open(m_strWarningOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			MessageBox("The warning output file could not be created.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		// (j.jones 2012-09-25 16:50) - PLID 50942 - insert the batch payment information to the top of the file,
		// remember that there could potentially be multiple payments in one file (though that is not common)
		CString strHeader;
		strHeader = "*************************************************************\r\n\r\n";
		m_fileLogWarnings.Write(strHeader, strHeader.GetLength());

		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		if (m_ERemitType == ertOhip) {

			//OHIP E-Remit

			for(int a=0;a<m_OHIPParser.m_paryOHIPEOBInfo.GetSize();a++) {

				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(a);
				if(pEOB) {
					CString strInfo;
					strInfo.Format("Batch Payment Amount: %s\r\n\r\n"
						"Payment Date: %s\t\tCheque Number: %s",
						FormatCurrencyForInterface(pEOB->cyTotalPaymentAmt),
						FormatDateTimeForInterface(pEOB->dtPaymentDate, NULL, dtoDate),
						pEOB->strCheckNumber == "99999999" ? "<Direct Deposit>" : pEOB->strCheckNumber);
					strInfo += "\r\n\r\n*************************************************************\r\n\r\n";

					m_fileLogWarnings.Write(strInfo ,strInfo.GetLength());
				}
			}
		}
		else if (m_ERemitType == ertAlberta) {
			//TES 10/3/2014 - PLID 62580 - Log the amount and date
			CString strInfo;
			strInfo.Format("Batch Payment Amount: %s\r\n\r\n"
				"Payment Date: %s",
				FormatCurrencyForInterface(m_AlbertaParser.m_cyTotalAssessedAmount),
				FormatDateTimeForInterface(m_AlbertaParser.m_dtFirstExpectedPaymentDate, NULL, dtoDate));
			strInfo += "\r\n\r\n*************************************************************\r\n\r\n";

			m_fileLogWarnings.Write(strInfo, strInfo.GetLength());

		}
		else {

			//US E-Remit

			for(int a=0;a<m_EOBParser.m_arypEOBInfo.GetSize();a++) {

				EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];
				if(pEOB) {
					CString strInfo;
					strInfo.Format("Batch Payment Amount: %s\t\tPayer: %s\r\n\r\n"
						"Check Issue / EFT Effective Date: %s\r\n"
						"Check / EFT Trace Number: %s",
						FormatCurrencyForInterface(pEOB->cyTotalPaymentAmt), pEOB->strInsuranceCoName,
						FormatDateTimeForInterface(pEOB->dtCheckDate, NULL, dtoDate), pEOB->strCheckNumber);
					strInfo += "\r\n\r\n*************************************************************\r\n\r\n";

					m_fileLogWarnings.Write(strInfo ,strInfo.GetLength());
				}
			}
		}

		m_bWarningsCreated = TRUE;
	}

	CString OutputString;

	strWarning.Replace("\n","\r\n");

	OutputString = strWarning;
	if(!strAction.IsEmpty()) {
		CString strActionNote;
		strActionNote.Format("\r\n\r\nAction taken: %s",strAction);
		OutputString += strActionNote;
	}

	OutputString += "\r\n\r\n*************************************************************\r\n\r\n";

	m_fileLogWarnings.Write(OutputString,OutputString.GetLength());
}

// (j.jones 2011-03-21 13:29) - PLID 42917 - added EOBID, can be -1
// if the log is opened prior to anything actually being posted
BOOL CEOBDlg::OpenWarningLog(long nEOBID /*= -1*/)
{
	if(!m_bWarningsCreated)
		return FALSE;

	// (j.jones 2011-03-21 13:24) - PLID 42917 - log the batch payment information,
	// if we posted batch payments for this EOB
	if(nEOBID != -1) {
		CString strLog, str;		
		_RecordsetPtr rs = CreateParamRecordset("SELECT BatchPaymentsT.ID, InsuranceCoT.Name, "
			"BatchPaymentsT.Amount, BatchPaymentsT.Date "
			"FROM BatchPaymentsT "
			"INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE BatchPaymentsT.ID IN ("
			"	SELECT BatchPayID FROM ERemittanceHistoryT WHERE EOBID = {INT}"
			") "
			"ORDER BY BatchPaymentsT.ID ASC", nEOBID);
		while(!rs->eof) {
			CString str;
			str.Format("Batch Payment ID %li posted %s to %s (Date: %s)\r\n",
				VarLong(rs->Fields->Item["ID"]->Value),				
				FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["Amount"]->Value)),
				VarString(rs->Fields->Item["Name"]->Value),
				FormatDateTimeForInterface(VarDateTime(rs->Fields->Item["Date"]->Value), NULL, dtoDate));

			strLog += str;

			rs->MoveNext();
		}
		rs->Close();

		if(!strLog.IsEmpty()) {
			strLog = "\r\n\r\n***Posting Summary*******************************************\r\n\r\n" + strLog;
			m_fileLogWarnings.Write(strLog,strLog.GetLength());
		}
	}

	// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up the log,
	// and is the only permissible way to close the file
	CloseWarningLogAndCopyToServer();

	m_bWarningsCreated = FALSE;
	
	//open the finished file in notepad

	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	// (j.jones 2011-01-11 14:11) - PLID 42074 - replaced ShellExecute with ShellExecuteEx to fix a VS2008 debug crash

	// Create the shell-execute info object
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

	CString strParameter;
	strParameter.Format("'%s'",m_strWarningOutputFile);

	// Init the info object according to the parameters given
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	//sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd =(HWND)GetDesktopWindow();
	sei.lpFile = "notepad.exe";
	sei.lpParameters = strParameter;
	sei.nShow = SW_SHOW;
	sei.hInstApp = NULL;
	
	// Run ShellExecute
	BOOL bSuccess = ShellExecuteEx(&sei);

	return bSuccess;
}

// (j.armen 2012-02-20 11:13) - PLID 47323 - Parameratized
void CEOBDlg::CalcWarnNoInsuranceCompanyChosen()
{
	try {

		//we need to look at all the insured parties that are returned,
		//and warn the user about errant patients

		CArray<long,long> aryInsuredPartyIDs;

		CArray<long,long> aryPatientsWithoutAnyInsuredParty;
		// (s.tullis 2016-04-15 16:00) - NX-100211
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow();pRow != nullptr;pRow=pRow->GetNextRow()) {
			long nInsuredPartyID = VarLong(pRow->GetValue(COLUMN_INSURED_PARTY_ID), -1);

			//if -1, we couldn't match up any insurance at all for this patient
			if(nInsuredPartyID == -1) {

				long nPatientID = VarLong(pRow->GetValue( COLUMN_PATIENT_ID), -1);

				BOOL bFound = FALSE;
				for(int j=0;j<aryPatientsWithoutAnyInsuredParty.GetSize() && !bFound;j++) {
					if(aryPatientsWithoutAnyInsuredParty.GetAt(j) == nPatientID) {
						bFound = TRUE;
					}
				}

				if(!bFound) {
					aryPatientsWithoutAnyInsuredParty.Add(nPatientID);
				}
				continue;
			}

			//if we got here, we have an insured party ID		
			BOOL bFound = FALSE;
			for(int j=0;j<aryInsuredPartyIDs.GetSize() && !bFound;j++) {
				if(aryInsuredPartyIDs.GetAt(j) == nInsuredPartyID) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				aryInsuredPartyIDs.Add(nInsuredPartyID);
			}
		}

		//now we should have a unique list of InsuredPartyIDs,
		//as well as a unique list of PatientIDs without any insured party found

		//now we need to calculate which insured party IDs are "correct" and which ones aren't

		CString strWarning;

		if(aryPatientsWithoutAnyInsuredParty.GetSize() > 0) {
			if(!aryPatientsWithoutAnyInsuredParty.IsEmpty()) {

				_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatientName FROM PersonT "
					"WHERE ID IN ({INTARRAY})", aryPatientsWithoutAnyInsuredParty);

				if(!rs->eof) {

					if(!strWarning.IsEmpty()) {
						strWarning += "\n";
					}
					strWarning += "The following patients have not been linked to any Insurance Company:\n\n";
				}

				while(!rs->eof) {

					CString strPatientName = AdoFldString(rs, "PatientName","");

					CString str;
					str.Format("%s\n",strPatientName);
					strWarning += str;

					rs->MoveNext();
				}
				rs->Close();
			}
		}

		if(!strWarning.IsEmpty()) {
			MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

	}NxCatchAll("Error in CEOBDlg::CalcWarnNoInsuranceCompanyChosen");
}

// (j.jones 2008-02-20 10:19) - PLID 29007 - added payment description
void CEOBDlg::OnEditPayDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_PayDescCombo, "Edit Combo Box").DoModal();

		//also requery the adjustment combo
		m_AdjDescCombo->Requery();

	}NxCatchAll("Error in CEOBDlg::OnEditPayDesc");
}

// (j.jones 2008-02-20 10:19) - PLID 29007 - added adjustment description
void CEOBDlg::OnEditAdjDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_AdjDescCombo, "Edit Combo Box").DoModal();
		//also requery the payment combo
		m_PayDescCombo->Requery();

	}NxCatchAll("Error in CEOBDlg::OnEditAdjDesc");
}

void CEOBDlg::OnEditPayCat() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pPayRow = m_PayCatCombo->GetCurSel();
		long nCurPayID = -1;
		if(pPayRow) {
			nCurPayID = VarLong(pPayRow->GetValue(pccID), -1);
		}

		NXDATALIST2Lib::IRowSettingsPtr pAdjRow = m_AdjCatCombo->GetCurSel();
		long nCurAdjID = -1;
		if(pAdjRow) {
			nCurAdjID = VarLong(pAdjRow->GetValue(accID), -1);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:57) - PLID 53752 - change the dialog
		//CEditComboBox(this, 3, m_PayCatCombo, "Edit Combo Box").DoModal();
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			m_PayCatCombo->Requery();
			m_PayCatCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

			//also requery the adjustment combo
			m_AdjCatCombo->Requery();
			m_AdjCatCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}		

		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_PayCatCombo->GetNewRow();
		pRow2->PutValue(pccID,long(-1));
		pRow2->PutValue(pccCategory,_bstr_t(" <No Category Selected>"));
		m_PayCatCombo->AddRowSorted(pRow2, NULL);

		pRow2 = m_AdjCatCombo->GetNewRow();
		pRow2->PutValue(accID,long(-1));
		pRow2->PutValue(accCategory,_bstr_t(" <No Category Selected>"));
		m_AdjCatCombo->AddRowSorted(pRow2, NULL);

		// set the selections back to what they were before
		if(nCurPayID != -1) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_PayCatCombo->TrySetSelByColumn_Deprecated(pccID, nCurPayID);
		}
		if(nCurAdjID != -1) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_AdjCatCombo->TrySetSelByColumn_Deprecated(accID, nCurAdjID);
		}

	}NxCatchAll("Error in CEOBDlg::OnEditPayCat");
}

void CEOBDlg::OnEditAdjCat() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pPayRow = m_PayCatCombo->GetCurSel();
		long nCurPayID = -1;
		if(pPayRow) {
			nCurPayID = VarLong(pPayRow->GetValue(pccID), -1);
		}

		NXDATALIST2Lib::IRowSettingsPtr pAdjRow = m_AdjCatCombo->GetCurSel();
		long nCurAdjID = -1;
		if(pAdjRow) {
			nCurAdjID = VarLong(pAdjRow->GetValue(accID), -1);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:58) - PLID 53752 - change the dialog
		//CEditComboBox(this, 3, m_AdjCatCombo, "Edit Combo Box").DoModal();
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			m_AdjCatCombo->Requery();
			m_AdjCatCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);			

			//also requery the payment combo
			m_PayCatCombo->Requery();
			m_PayCatCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);		
		}			

		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_PayCatCombo->GetNewRow();
		pRow2->PutValue(pccID,long(-1));
		pRow2->PutValue(pccCategory,_bstr_t(" <No Category Selected>"));
		m_PayCatCombo->AddRowSorted(pRow2, NULL);

		pRow2 = m_AdjCatCombo->GetNewRow();
		pRow2->PutValue(accID,long(-1));
		pRow2->PutValue(accCategory,_bstr_t(" <No Category Selected>"));
		m_AdjCatCombo->AddRowSorted(pRow2, NULL);

		// set the selections back to what they were before
		if(nCurPayID != -1) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_PayCatCombo->TrySetSelByColumn_Deprecated(pccID, nCurPayID);
		}
		if(nCurAdjID != -1) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_AdjCatCombo->TrySetSelByColumn_Deprecated(accID, nCurAdjID);
		}

	}NxCatchAll("Error in CEOBDlg::OnEditAdjCat");
}

void CEOBDlg::OnSelChosenPayDescriptionCombo(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		CString strDesc = VarString(pRow->GetValue(0));
		m_PayDescCombo->PutComboBoxText("");
		GetDlgItem(IDC_PAY_DESC)->SetWindowText(strDesc);

	}NxCatchAll("Error in CEOBDlg::OnSelChosenPayDescriptionCombo");
}

void CEOBDlg::OnSelChosenAdjDescriptionCombo(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		CString strDesc = VarString(pRow->GetValue(0));
		m_AdjDescCombo->PutComboBoxText("");
		GetDlgItem(IDC_ADJ_DESC)->SetWindowText(strDesc);

	}NxCatchAll("Error in CEOBDlg::OnSelChosenAdjDescriptionCombo");
}

// (j.jones 2008-03-26 10:19) - PLID 29407 - given an adjustment, payment, and a balance, find what the new balance would be
// after the payment is applied, and if the adjustment is greater than that value, reduce the adjustment amount
// (j.jones 2011-03-07 16:09) - PLID 41877 - Added cyPatientRespNeeded, representing the amount the EOB says should
// be a balance of the charge (for any resp, really), and we reduced the adjustment to account for it.
// (j.jones 2011-11-02 08:54) - PLID 46232 - We now have a minimum adjustment amount, which is the most we can reduce the adjustment to.
// I also removed cyAvailableBalance, because this shouldn't be called until after we've forced the ins. balance to be whatever we needed.
// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientRespNeeded to cyOtherRespNeeded, because even though the
	// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
// (j.jones 2012-06-27 13:33) - PLID 51236 - added bWeAreShiftingPatientResp, so the reduction logic knows how to calculate the required "other resp" needed
void CEOBDlg::ReduceAdjustmentValue(IN OUT COleCurrency &cyCurrentAdjustmentAmount, COleCurrency cyMinimumAdjustmentAmount,
									COleCurrency cyPaymentAmount, long nInsuredPartyID,
									long nPatientID, long nPatientUserDefinedID, long nChargeID, const COleCurrency cyOtherRespNeeded, const BOOL bWeAreShiftingPatientResp)
{
	if(cyCurrentAdjustmentAmount < COleCurrency(0,0)) {
		// (j.jones 2011-11-02 10:37) - I ran into this when the amount we need
		// can't actually be acquired, so we're applying what we can, and
		// reducing the adjusment to zero
		cyCurrentAdjustmentAmount = COleCurrency(0,0);

		// (j.jones 2016-04-19 14:33) - NX-100161 - I do not believe this is actually possible anymore.
		ASSERT(FALSE);
	}

	// (j.jones 2011-11-02 09:50) - PLID 46232 - cyAvailableBalance should not have been passed in,
	// it should be the current insurance balance, always. The caller should have ensured that we
	// already shifted all the money we need.
	COleCurrency cyAvailableBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);

	//do not have a try/catch clause, we want the calling function to intercept it

	COleCurrency cyOrigAdjustment = cyCurrentAdjustmentAmount;

	COleCurrency cyNewBalance = cyAvailableBalance - cyPaymentAmount;
	if(cyAvailableBalance < COleCurrency(0,0)) {
		//the payment is more than the balance, how did we even allow this?
		ASSERT(FALSE);
		cyCurrentAdjustmentAmount = COleCurrency(0,0);
	}
	else if(cyAvailableBalance == COleCurrency(0,0)) {
		//the payment was the entire balance, so we have a $0.00 adjustment
		cyCurrentAdjustmentAmount = COleCurrency(0,0);
	}
	else if(cyCurrentAdjustmentAmount > cyNewBalance) {
		//our adjustment was too big, so reduce it to match the new balance
		cyCurrentAdjustmentAmount = cyNewBalance;
	}
	else {
		//do nothing, our adjustment amount is potentially fine
	}

	if(cyCurrentAdjustmentAmount < COleCurrency(0,0)) {
		// (j.jones 2011-11-02 10:37) - I ran into this when the amount we need
		// can't actually be acquired, so we're applying what we can, and
		// reducing the adjusment to zero
		cyCurrentAdjustmentAmount = COleCurrency(0,0);
	}

	// (j.jones 2011-03-08 08:54) - PLID 41877 - If cyPatientRespNeeded is > 0, it means that
	// the EOB states that the patient should owe that much money. When posting is finished,
	// the total patient resp. of the charge plus any insurance balances needs to be this
	// cyPatientRespNeeded value at a minimum. We do not reduce payment information, we only
	// reduce adjustment information in order to achieve this goal.
	// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientRespNeeded to cyOtherRespNeeded, because even though the
	// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company".
	// I'm going to keep this boolean name though, because later we'll call it patient resp. in a message box,
	// which is still what we intend to say.
	BOOL bReducedForPatResp = FALSE;

	//DO NOT check for cyCurrentAdjustmentAmount being zero here! It may seem that this
	//code is unnecessary if cyCurrentAdjustmentAmount is zero, but we may have already
	//reduced it to zero just for this reason. This code is still necessary so
	//the EOB warnings reflect whether the pat. resp also needs to be reported.
	if(cyOtherRespNeeded > COleCurrency(0,0)) {

		COleCurrency cyRemain = COleCurrency(0,0);

		// (j.jones 2012-06-27 12:15) - PLID 51236 - If we are going to shift cyOtherRespNeeded to patient,
		// then we have to enforce that it will exist as a balance. At all times, we enforce that cyOtherRespNeeded
		// exists as an "other" insurance responsibility. This means we have to check both cases.

		//Step 1. Only if we're shifting to patient, we need to ensure that the amount we will shift is reduced
		//from the current balance, and reduce the adjustment accordingly to handle that lower balance.
		if(bWeAreShiftingPatientResp) {
			COleCurrency cyAmtToShiftToPat = COleCurrency(0,0);
			//this calculates the amount that we will later shift to patient
			TryShiftRespToPatient(nPatientID, nChargeID, nInsuredPartyID, cyOtherRespNeeded, TRUE, cyAmtToShiftToPat);

			if(cyAmtToShiftToPat > COleCurrency(0,0)) {
				COleCurrency cyCurInsBalance = GetChargeInsBalance(nChargeID, nPatientID, nInsuredPartyID);
				COleCurrency cyAmtNeeded = cyPaymentAmount + cyCurrentAdjustmentAmount + cyAmtToShiftToPat;
				COleCurrency cyDiff = cyAmtNeeded - cyCurInsBalance;
				if(cyDiff > COleCurrency(0,0)) {
					//after shifting patient resp., the remaining insurance balance of the charge will not
					//be enough to satisfy both the payment and the adjustment, so we need to reduce the adjustment
					
					//if we can take the difference out of the adjustment, do so
					if(cyCurrentAdjustmentAmount > cyDiff) {
						cyCurrentAdjustmentAmount -= cyDiff;
						
						if(cyCurrentAdjustmentAmount < COleCurrency(0,0)) {
							// This may happen if the amount we need can't actually be acquired.
							// So we would apply what we can of the payment, and reduce the adjusment to zero.
							cyCurrentAdjustmentAmount = COleCurrency(0,0);
						}
					}
					//If the difference equals the adjustment amount, reduce it to zero.
					//This is likely if a secondary payer made an adjustment for the same amount
					//as the expected patient responsibility. And yes, that's stupid.
					else if(cyCurrentAdjustmentAmount == cyDiff) {
						//this is the same logic as the next else block, only separated for debugging purposes
						cyCurrentAdjustmentAmount = COleCurrency(0,0);
					}
					//If we can't take the full difference out of the adjustment,
					//just reduce the adjustment needs to zero. We will apply as much as
					//we can of the payment and leave the remainder unapplied if needed.
					else if(cyCurrentAdjustmentAmount <= cyDiff) {
						//this is the same logic as the prior else block, only separated for debugging purposes
						cyCurrentAdjustmentAmount = COleCurrency(0,0);
					}
					
					//enable the flag that we reduced the adjustment further
					bReducedForPatResp = TRUE;
				}
			}
		}
		
		//Step 2: We need to ensure that the amount in cyOtherRespNeeded will still exist
		//as a total of the Practice patient resp. + other resp. + remaining insurance balance.
		//Calculate this as cyAvailOtherResp.
		{
			COleCurrency cyAvailOtherResp = GetChargeInsBalanceAndOtherResps(nChargeID, nInsuredPartyID);
			COleCurrency cyRemain = (cyAvailOtherResp - cyPaymentAmount - cyCurrentAdjustmentAmount);
			COleCurrency cyDiff = cyOtherRespNeeded - cyRemain;
			if(cyDiff > COleCurrency(0,0)) {
				//the remaining insurance balance of the charge after applying this payment and adjustment,
				//plus the existing patient resp, will not be enough for cyPatientResp, so we need
				//to take away from the amount we need
				
				//if we can take the difference out of the adjustment, do so
				if(cyCurrentAdjustmentAmount > cyDiff) {
					cyCurrentAdjustmentAmount -= cyDiff;
					
					if(cyCurrentAdjustmentAmount < COleCurrency(0,0)) {
						// (j.jones 2011-11-02 10:37) - I ran into this when the amount we need
						// can't actually be acquired, so we're applying what we can, and
						// reducing the adjusment to zero
						cyCurrentAdjustmentAmount = COleCurrency(0,0);
					}
				}
				//if we can't take the full difference out of the adjustment,
				//just reduce the adjustment needs to zero
				else if(cyCurrentAdjustmentAmount <= cyDiff) {
					cyCurrentAdjustmentAmount = COleCurrency(0,0);
				}
				
				//enable the flag that we reduced the adjustment further
				bReducedForPatResp = TRUE;
			}
		}
	}

	// (j.jones 2011-11-02 09:11) - PLID 46232 - We should not have to force the cyMinimumAdjustmentAmount,
	// because the calling code should have ensured that cyAvailableBalance was large enough to accommodate
	// the cyMinimumAdjustmentAmount. If we hit this assert, the calling code failed.
	// We can't force the cyCurrentAdjustmentAmount to equal the cyMinimumAdjustmentAmount, because
	// it would have nothing to apply to.
	if(cyCurrentAdjustmentAmount < cyMinimumAdjustmentAmount) {
		ASSERT(FALSE);

		//In theory, if this failed we could assign cyMinimumAdjustmentAmount to cyCurrentAdjustmentAmount,
		//and applies would presumably apply what they could, leaving the rest unapplied.
		//However, our warnings would likely be misleading. I'm not going to add that code until I find
		//an EOB file that legitimately can get to this point.
	}

	//update the screen, and log, if we changed anything
	if(cyCurrentAdjustmentAmount < cyOrigAdjustment) {

		//update the screen, remember it may be across multiple charges!
		COleCurrency cyAdjDiff = cyOrigAdjustment - cyCurrentAdjustmentAmount;
		// (s.tullis 2016-04-15 16:00) - NX-100211
		for(NXDATALIST2Lib::IRowSettingsPtr pLoopRow = m_EOBList->GetFirstRow(); pLoopRow != nullptr && cyAdjDiff > COleCurrency(0,0); pLoopRow = pLoopRow->GetNextRow()) {

			long nChargeIDToCheck = VarLong(pLoopRow->GetValue(COLUMN_CHARGE_ID));			
			if(nChargeIDToCheck == nChargeID) {
				//this is our charge, let's reduce the adjustment

				// (j.jones 2016-04-19 10:09) - NX-100161 - we now have total positive adjustments, and total adjustments,
				// both would need updated
				COleCurrency cyExistingTotalPositiveAdj = COleCurrency(0, 0);
				COleCurrency cyExistingTotalNegativeAdj = COleCurrency(0, 0);
				COleCurrency cyExistingTotalAdj = COleCurrency(0, 0);
				GetTotalChargeAdjustments(pLoopRow, cyExistingTotalPositiveAdj, cyExistingTotalNegativeAdj, cyExistingTotalAdj);

				COleCurrency cyNewPosAdj = cyExistingTotalPositiveAdj;
				COleCurrency cyNewTotalAdj = cyExistingTotalAdj;
				if(cyNewPosAdj >= cyAdjDiff) {
					cyNewPosAdj -= cyAdjDiff;
					cyNewTotalAdj -= cyAdjDiff;
					cyAdjDiff = COleCurrency(0,0);
				}
				else {
					//the existing adjustment on this line item is less than the total,
					//so zero it out and reduce our amount
					cyAdjDiff -= cyNewPosAdj;
					cyNewPosAdj = COleCurrency(0,0);
					cyNewTotalAdj = COleCurrency(0,0);
				}

				//we're not done yet - we now have to update the individual positive adjustments per charge/claim
				{
					COleCurrency cyTotalAdjToReduce = cyExistingTotalPositiveAdj - cyNewPosAdj;

					EOBLineItemInfo *pCharge = NULL;
					_variant_t var = pLoopRow->GetValue(COLUMN_CHARGE_PTR);
					if (var.vt == VT_I4) {
						pCharge = (EOBLineItemInfo*)VarLong(var);
					}
					EOBClaimInfo *pClaim = NULL;
					var = pLoopRow->GetValue(COLUMN_CLAIM_PTR);
					if (var.vt == VT_I4) {
						pClaim = (EOBClaimInfo*)VarLong(var);
					}

					//first try to reduce charge adjustments (this is normal)
					if (pCharge != NULL && cyTotalAdjToReduce > COleCurrency(0, 0)) {
						for (int iAdjIndex = 0; iAdjIndex < pCharge->arypEOBAdjustmentInfo.GetSize() && cyTotalAdjToReduce > COleCurrency(0, 0); iAdjIndex++) {
							EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[iAdjIndex];
							//skip patient resp. adjustments (which are not real line items),
							//and skip negative adjustments
							if (!pAdj->bPatResp && pAdj->cyAdjustmentAmt > COleCurrency(0, 0)) {
								if (pAdj->cyAdjustmentAmt >= cyTotalAdjToReduce) {
									pAdj->cyAdjustmentAmt -= cyTotalAdjToReduce;
									cyTotalAdjToReduce = COleCurrency(0, 0);
								}
								else {
									cyTotalAdjToReduce -= pAdj->cyAdjustmentAmt;
									pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
								}
							}
						}
					}

					//if we have adjustments left to reduce, check the claim level adjustments
					//(this is not common)
					if (pClaim != NULL && cyTotalAdjToReduce > COleCurrency(0, 0)) {
						for (int iAdjIndex = 0; iAdjIndex < pClaim->arypEOBAdjustmentInfo.GetSize() && cyTotalAdjToReduce > COleCurrency(0, 0); iAdjIndex++) {
							EOBAdjustmentInfo *pAdj = pClaim->arypEOBAdjustmentInfo[iAdjIndex];
							//skip patient resp. adjustments (which are not real line items),
							//and skip negative adjustments
							if (!pAdj->bPatResp && pAdj->cyAdjustmentAmt > COleCurrency(0, 0)) {
								if (pAdj->cyAdjustmentAmt >= cyTotalAdjToReduce) {
									pAdj->cyAdjustmentAmt -= cyTotalAdjToReduce;
									cyTotalAdjToReduce = COleCurrency(0, 0);
								}
								else {
									cyTotalAdjToReduce -= pAdj->cyAdjustmentAmt;
									pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
								}
							}
						}
					}

					//if this is non-zero, something has gone very wrong,
					//and our displayed adjustment totals won't reflect what will really be posted!
					ASSERT(cyTotalAdjToReduce == COleCurrency(0, 0));
				}

				//now update the list with our new totals
				pLoopRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(cyNewPosAdj)));
				pLoopRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(cyNewTotalAdj)));

				//calculate the anticipated insurance balance after this EOB is applied
				COleCurrency cyNewInsBalance = VarCurrency(pLoopRow->GetValue(COLUMN_INS_BALANCE), COleCurrency(0,0)) + cyExistingTotalAdj - cyNewTotalAdj;
				pLoopRow->PutValue(COLUMN_INS_BALANCE, _variant_t(COleCurrency(cyNewInsBalance)));
			}
		}

		//some strange problems here if it is somehow not fully reduced
		ASSERT(cyAdjDiff == COleCurrency(0,0));

		//now log this
		CString strPatientName = GetExistingPatientName(nPatientID);

		CString strItemDesc = "";
		_RecordsetPtr rs = CreateParamRecordset("SELECT ItemCode, Description FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.ID = {INT}", nChargeID);
		if(!rs->eof) {
			strItemDesc.Format("%s - %s",AdoFldString(rs, "ItemCode",""),AdoFldString(rs, "Description",""));
		}
		rs->Close();

		CString strLog;
		// (s.dhole 2013-06-24 16:20) - PLID 42275 Added userdefinedid
		strLog.Format("For patient '%s' [Patient ID: %li], Practice will apply %s in insurance payments, and %s in insurance adjustments\n"
			"to the charge: '%s'\n\n"
			"The adjustment amount was reduced from the original EOB adjustment value of %s, which can not be applied\n"
			"because the available balance of this charge prior to these applies was only %s.",
			strPatientName,nPatientUserDefinedID, FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE),
			FormatCurrencyForInterface(cyCurrentAdjustmentAmount, TRUE, TRUE),
			strItemDesc,
			FormatCurrencyForInterface(cyOrigAdjustment, TRUE, TRUE),
			FormatCurrencyForInterface(cyAvailableBalance, TRUE, TRUE));

		// (j.jones 2011-03-08 08:59) - PLID 41877 - if we reduced further due to patient resp,
		// announce that we did so
		// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientRespNeeded to cyOtherRespNeeded, because even though the
		// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company".
		// However, we still mean to call it patient resp. in this warning, so the wording is unchanged.
		if(bReducedForPatResp) {
			CString str;
			str.Format("\nIn addition, the EOB indicates there should be a %s patient responsibility on this charge.",
				FormatCurrencyForInterface(cyOtherRespNeeded, TRUE, TRUE));
			strLog += str;
		}

		AddWarningToLog(strLog, "Adjustment Reduced");
	}
}

// (j.jones 2008-06-16 17:24) - PLID 21921 - added OHIP support
void CEOBDlg::OnBtnImportOhipFile() 
{
	try {

		if(!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrUse)) {
			return;
		}

		if(m_EOBList->GetRowCount() > 0) {
			if(IDNO == MessageBox("You have already imported a remittance file. If you continue, no data will be posted.\n\n"
				"Do you wish to clear out the current data and import a new file?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}
			else {

				// (j.jones 2009-07-01 16:27) - PLID 34778 - clear and hide the overpayment label
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);

				m_EOBList->Clear();	

				// (j.jones 2011-02-09 11:34) - PLID 42391 - hide the reversal column,
				// although it really should have never been shown in OHIP
				NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
				pReversalCol->PutStoredWidth(0);

				// (j.jones 2011-06-16 08:51) - PLID 39747 - the default insurance combo filter for when we aren't showing
				// specific insurance companies in the list, should just be "Archived = 0"
				m_strCurInsuranceComboFilter = DEFAULT_INSURANCE_FILTER;
				m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
				m_InsuranceCombo->Requery();
				SetDlgItemText(IDC_PROVIDER_LABEL, "Provider:");
				SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL,FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
			}
		}

		m_OHIPParser.ClearEOB();

		//since we pump messages during the processing, make sure that all buttons are disabled
		//during the processing. We re-enable them when the processing is complete.
		EnableButtons(FALSE);

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Importing file...");

		m_OHIPParser.m_ptrProgressBar = &m_progress;
		m_OHIPParser.m_ptrProgressStatus = &m_progressStatus;

		COleCurrency cyTotalPayments = COleCurrency(0,0);

		BOOL bWarnMissingInsuredParties = FALSE;
		BOOL bWarnMissingBillIDs = FALSE;

		// (j.jones 2008-12-19 08:46) - PLID 32519 - added ability to auto-open a file
		if(!m_strAutoOpenFilePath.IsEmpty() && DoesExist(m_strAutoOpenFilePath)) {
			//tell the parser to use this default file
			m_OHIPParser.m_strFileName = m_strAutoOpenFilePath;			
		}
		else {
			//ensure the parser has no default
			m_OHIPParser.m_strFileName = "";
		}
		//clear our default file now
		m_strAutoOpenFilePath = "";

		// (j.jones 2008-06-17 10:48) - PLID 30413 - parse the file
		// (j.jones 2012-05-25 13:59) - PLID 44367 - pass this dialog as the parent
		if(m_OHIPParser.ParseFile(this)) {

			// (j.jones 2010-04-09 12:00) - PLID 31309 - added date controls,
			// disable the date if they enabled it and fill with the first check date
			// (j.jones 2010-11-30 09:53) - PLID 39434 - check the DefaultEOBDate preference,
			// 0 - use EOB date, 1 - use today's date
			// we would have set to today's date when the dialog opened, so if the preference is
			// set to 1, do nothing here
			BOOL bUseEOBDate = (GetRemotePropertyInt("DefaultEOBDate", 0, 0, "<None>", true) == 0);
			if(bUseEOBDate) {
				m_checkEnablePayDate.SetCheck(FALSE);
				m_checkEnableAdjDate.SetCheck(FALSE);
				GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(FALSE);
				GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(FALSE);
				COleDateTime dtPayDate = COleDateTime::GetCurrentTime();
				if(m_OHIPParser.m_paryOHIPEOBInfo.GetSize() > 0) {
					OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(0);
					if(pEOB->dtPaymentDate.GetStatus() != COleDateTime::invalid) {
						dtPayDate = pEOB->dtPaymentDate;
					}
				}
				m_dtPayDate.SetValue(dtPayDate);
				m_dtAdjDate.SetValue(dtPayDate);
			}

			// (j.jones 2008-12-18 12:01) - PLID 32489 - add this file to our report history if it is not already there,
			// ensure the folder path is the currently used path if the filename is there
			if(m_OHIPParser.m_paryOHIPEOBInfo.GetSize() > 0) {
				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(0);
				CString strFileName, strFolderPath;
				int nSlash = m_OHIPParser.m_strFileName.ReverseFind('\\');
				if(nSlash == -1) {
					strFileName = m_OHIPParser.m_strFileName;
					strFolderPath = "";
				}
				else {
					strFileName = m_OHIPParser.m_strFileName.Right(m_OHIPParser.m_strFileName.GetLength() - nSlash - 1);
					strFolderPath = m_OHIPParser.m_strFileName.Left(nSlash);
				}
				//use a Report Type of 3 for E-Remits
				ExecuteParamSql("IF NOT EXISTS (SELECT ID FROM OHIPReportHistoryT WHERE FileName = {STRING}) \r\n"
					"BEGIN \r\n"
					"INSERT INTO OHIPReportHistoryT (FileName, FilePath, ReportType, ReportInternalDate) "
					"VALUES ({STRING}, {STRING}, 3, {STRING}) \r\n"
					"END \r\n"
					"ELSE \r\n"
					"BEGIN \r\n"
					"UPDATE OHIPReportHistoryT SET FilePath = {STRING} WHERE FileName = {STRING} \r\n"
					"END \r\n", strFileName, strFileName, strFolderPath, FormatDateTimeForSql(pEOB->dtPaymentDate),
					strFolderPath, strFileName);
			}

			if(m_OHIPParser.m_CountOfEOBs == 1)
				SetDlgItemText(IDC_PAYMENT_LABEL,"Total Payment:");
			else {
				CString str;
				str.Format("Total Payments (%li):",m_OHIPParser.m_CountOfEOBs);
				SetDlgItemText(IDC_PAYMENT_LABEL,str);
			}

			CStringArray arystrInvalidClaims;

			//loop through each EOB, though for OHIP there ought to only be one
			for(int a=0;a<m_OHIPParser.m_paryOHIPEOBInfo.GetSize();a++) {
				_variant_t varNull;
				varNull.vt = VT_NULL;
			
				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(a);

				CString str;
				if(m_OHIPParser.m_CountOfEOBs == 1) {
					str = "Searching financial data...";
				}
				else {
					str.Format("Searching financial data (EOB %li)...",pEOB->nIndex+1);
				}
				m_progressStatus.SetWindowText(str);

				//calculate the IDs of the insurance co and charges
				// (j.jones 2008-12-15 12:30) - PLID 32322 - this function used to calculate the
				// bill and patient IDs, but now it is done via CalcBillAndPatientID(), which is
				// now called during the parsing process
				m_OHIPParser.CalcInternalIDs(pEOB);

				m_progress.SetPos(0);
				m_progress.SetRange(0,pEOB->paryEOBClaimInfo.GetSize());
				m_progress.SetStep(1);

				if(m_OHIPParser.m_CountOfEOBs == 1) {
					str = "Populating list...";
				}
				else {
					str.Format("Populating list (EOB %li)...",pEOB->nIndex+1);
				}
				m_progressStatus.SetWindowText(str);

				//now propagate the list

				SetDlgItemText(IDC_PROVIDER_LABEL, "Payee:");
				if(pEOB->strPayeeName != "") {
					SetDlgItemText(IDC_PROVIDER_NAME_LABEL, pEOB->strPayeeName);
				}
				else {
					SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
				}

				cyTotalPayments += pEOB->cyTotalPaymentAmt;
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL,FormatCurrencyForInterface(cyTotalPayments, TRUE, TRUE));

				m_InsuranceCombo->SetSelByColumn(iccID, pEOB->nLikelyInsuranceCoID);

				// (j.jones 2008-07-11 16:38) - PLID 28756 - now try to set our default category and description
				TrySetDefaultInsuranceDescriptions();

				for(int i=0;i<pEOB->paryEOBClaimInfo.GetSize();i++) {
					OHIPEOBClaimInfo *pClaim = (OHIPEOBClaimInfo*)pEOB->paryEOBClaimInfo.GetAt(i);

					// (j.jones 2010-01-27 11:48) - PLID 36998 - as we populate the screen, check and see if we have
					// any invalid claim files, and warn accordingly
					//(copied from the ANSI logic, has not been proven to happen yet in OHIP)
					if(pClaim->paryOHIPEOBLineItemInfo.GetSize() == 0) {
						//This claim is invalid, as we received no services under it.
						//We would have already logged this in the EOB.txt, but we need
						//to warn the user before they post this EOB.
						CString strClaimWarn;
						if(pClaim->nPatientID != -1) {
							//can't get nPatientID without nBillID
							_RecordsetPtr rsBill = CreateParamRecordset("SELECT Date, dbo.GetBillTotal(ID) AS TotalAmt FROM BillsT WHERE ID = {INT}", pClaim->nBillID);
							if(rsBill->eof) {
								strClaimWarn.Format("%s\r\n", GetExistingPatientName(pClaim->nPatientID));
							}
							else {
								strClaimWarn.Format("%s - Bill Date: %s - Amount: %s\r\n", GetExistingPatientName(pClaim->nPatientID),
									FormatDateTimeForInterface(AdoFldDateTime(rsBill, "Date"), NULL, dtoDate),
									FormatCurrencyForInterface(AdoFldCurrency(rsBill, "TotalAmt", COleCurrency(0,0))));
							}
							rsBill->Close();
						}
						else {
							//we have no idea who this is
							strClaimWarn += "Unknown Patient, Unknown Bill (Check the EOB.txt)\r\n";
						}
						arystrInvalidClaims.Add(strClaimWarn);
					}

					// (j.jones 2009-09-15 11:14) - PLID 35444 - Before adding into the list, we need to do a little
					// work on the payments. If a charge is overpaid and there is another charge in the same claim
					// for the same service that is NOT paid, move the overage to the other charge
					int j=0;
					for(j=0;j<pClaim->paryOHIPEOBLineItemInfo.GetSize();j++) {

						OHIPEOBLineItemInfo *pPaidCharge = (OHIPEOBLineItemInfo*)pClaim->paryOHIPEOBLineItemInfo.GetAt(j);
						COleCurrency cyOverage = pPaidCharge->cyLineItemPaymentAmt - pPaidCharge->cyLineItemChargeAmt;
						if(cyOverage > COleCurrency(0,0)) {

							//the payment is more than the amount charged, so now let's find a duplicate charge
							for(int k=0; k<pClaim->paryOHIPEOBLineItemInfo.GetSize() && cyOverage > COleCurrency(0,0);k++) {

								OHIPEOBLineItemInfo *pChargeToCheck = (OHIPEOBLineItemInfo*)pClaim->paryOHIPEOBLineItemInfo.GetAt(k);
								if(pChargeToCheck != pPaidCharge
									&& pChargeToCheck->strServiceCode == pPaidCharge->strServiceCode
									&& pChargeToCheck->cyLineItemPaymentAmt == COleCurrency(0,0)
									&& pChargeToCheck->cyLineItemChargeAmt > COleCurrency(0,0)) {

									//we just found a charge with the same service code and no payment,
									//so let's move the overage to this charge

									if(cyOverage > pChargeToCheck->cyLineItemChargeAmt) {
										//we can only apply some of this money

										//log this
										Log("Moved %s in payments between charges with the same service code %s for patient %s.",
											FormatCurrencyForInterface(pChargeToCheck->cyLineItemChargeAmt),
											pChargeToCheck->strServiceCode, GetExistingPatientName((long)pClaim->nPatientID));

										pChargeToCheck->cyLineItemPaymentAmt = pChargeToCheck->cyLineItemChargeAmt;
										pPaidCharge->cyLineItemPaymentAmt -= pChargeToCheck->cyLineItemPaymentAmt;
										cyOverage -= pChargeToCheck->cyLineItemPaymentAmt;
									}
									else {
										//apply the rest of the overage

										//log this
										Log("Moved %s in payments between charges with the same service code %s for patient %s.",
											FormatCurrencyForInterface(cyOverage),
											pChargeToCheck->strServiceCode, GetExistingPatientName((long)pClaim->nPatientID));

										pChargeToCheck->cyLineItemPaymentAmt = cyOverage;
										pPaidCharge->cyLineItemPaymentAmt -= cyOverage;
										cyOverage = COleCurrency(0,0);
									}									
								}
							}
						}
					}

					for(j=0;j<pClaim->paryOHIPEOBLineItemInfo.GetSize();j++) {

						NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

						OHIPEOBLineItemInfo *pCharge = (OHIPEOBLineItemInfo*)pClaim->paryOHIPEOBLineItemInfo.GetAt(j);

						pRow->PutValue(COLUMN_EOB_INDEX, (long)pEOB->nIndex);
						pRow->PutValue(COLUMN_PATIENT_ID, (long)pClaim->nPatientID);
						pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pClaim->nPatientID)));
                        // (a.levy 2013-11-14 15:10) - PLID - 59445 - need to add userDefinedID here 
						pClaim->nUserDefinedID = GetExistingPatientUserDefinedID(pClaim->nPatientID);
                        pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pClaim->nUserDefinedID);
						pRow->PutValue(COLUMN_BILL_ID, (long)pClaim->nBillID);
						pRow->PutValue(COLUMN_CHARGE_ID, (long)pCharge->nChargeID);
						
						// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
						_variant_t varBirthDate = g_cvarNull;
						COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pClaim->nPatientID);
						if(dtBirthDate.GetStatus() != COleDateTime::invalid) {
							varBirthDate.vt = VT_DATE;
							varBirthDate.date = dtBirthDate;
						}
						pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);

						pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, _bstr_t(pClaim->strHealthRegistrationNumber));
						pRow->PutValue(COLUMN_OHIP_VERSION_CODE, _bstr_t(pClaim->strVersionCode));

						// (j.jones 2008-12-15 16:50) - PLID 32329 - supported detecting duplicate charges
						pRow->PutValue(COLUMN_DUPLICATE, (pCharge->bDuplicateCharge ? (long)1 : (long)0));

						// (j.jones 2011-02-09 11:24) - PLID 42391 - OHIP does not have reversals
						pRow->PutValue(COLUMN_REVERSAL, g_cvarFalse);

						if(pClaim->nBillID == -1) {
							bWarnMissingBillIDs = TRUE;
							pRow->PutForeColor(RGB(192,0,0));
						}
						//try to set the provider ID
						if(m_ProviderCombo->CurSel == -1) {
							long nProvID = pCharge->nProviderID;
							if(nProvID != -1) {
								m_ProviderCombo->SetSelByColumn(0,nProvID);
							}
						}

						_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",pClaim->nBillID);
						if(!rs->eof) {
							pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
						}
						else
							pRow->PutValue(COLUMN_BILL_DATE, varNull);
						rs->Close();

						rs = CreateParamRecordset("SELECT Date, ItemCode, "
							"dbo.GetChargeTotal(ChargesT.ID) AS Amount , "
							"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes "
							"FROM ChargesT "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
							"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0",pCharge->nChargeID);

						COleCurrency cyChargeAmt = COleCurrency(0,0);

						if(!rs->eof) {

							pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);
							
							//should be equivalent to pCharge->strServiceID
							pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							_variant_t varNoteIcon;
							if(!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
								//load from the datalist's icon for no notes
								varNoteIcon = (LPCTSTR)"BITMAP:FILE";
							}
							else {
								//load our icon for having notes
								varNoteIcon = (long)m_hNotes;
							}
							pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

							cyChargeAmt = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));					
						}
						else {
							pRow->PutValue(COLUMN_CHARGE_DATE, varNull);
							pRow->PutValue(COLUMN_CPT_CODE, varNull);
							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							pRow->PutValue(COLUMN_NOTE_ICON, varNull);
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, varNull);
						}
						rs->Close();

						CString strAdjustmentDescription;

						pRow->PutValue(COLUMN_PAYMENT, _variant_t(COleCurrency(pCharge->cyLineItemPaymentAmt)));

						// (j.jones 2016-04-22 14:12) - NX-100161 - OHIP does not have adjustments
						pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(0, 0)));
						pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(0, 0)));

						// (j.jones 2011-03-18 09:32) - PLID 42157 - added fee sched allowable and EOB allowable,
						// but these are not supported in OHIP
						pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);

						pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);

						pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _bstr_t(""));

						pRow->PutValue(COLUMN_ADJ_REASON_CODE, _bstr_t(""));

						pRow->PutValue(COLUMN_ADJ_REASON, _variant_t(_bstr_t(strAdjustmentDescription)));

						// (j.gruber 2009-12-18 15:45) - PLID 32936 - this always needs to be 0
						pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(COleCurrency(0,0)));
						// (s.tullis 2016-04-20 9:50) - NX-100185 
						pRow->PutRefCellFormatOverride(COLUMN_INSURED_PARTY_ID, GetInsuredPartyColumnCombo((long)pClaim->nPatientID));
						pRow->PutValue(COLUMN_INSURED_PARTY_ID, pClaim->nInsuredPartyID);

						// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
						pRow->PutValue(COLUMN_RESP_TYPE_ID, pClaim->nRespTypeID);

						// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
						pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, pClaim->bSubmitAsPrimary ? g_cvarTrue : g_cvarFalse);

						pRow->PutValue(COLUMN_INS_CO_ID, pClaim->nInsuranceCoID);

						pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(pClaim->strInsuranceCoName));

						pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);

						pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(""));

						if(pClaim->nInsuredPartyID == -1) {
							bWarnMissingInsuredParties = TRUE;
							pRow->PutForeColor(RGB(192,0,0));
						}

						//get the total of current patient applies made on this charge
						COleCurrency cyPatApplies = COleCurrency(0,0);
						COleCurrency cyPatientResp = GetChargePatientResp(pCharge->nChargeID);
						COleCurrency cyPatientBalance = GetChargePatientBalance(pCharge->nChargeID, pClaim->nPatientID);
						cyPatApplies = cyPatientResp - cyPatientBalance;
						pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));
						
						//calculate the anticipated patient balance after this EOB is applied
						COleCurrency cyPatBalance = COleCurrency(0,0);
						//expected patient resp - existing patient applies
						// (j.gruber 2009-12-18 15:47) - PLID 32936 - get rid of cyTotalPatientResp variable
						cyPatBalance = cyPatientResp - cyPatApplies;
						pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

						//calculate the anticipated insurance balance after this EOB is applied
						COleCurrency cyInsBalance = COleCurrency(0,0);
						// (j.gruber 2009-12-18 15:48) - PLID 32936 - get rid of cyTotalPatientResp variable
						// (j.jones 2016-04-22 14:12) - NX-100161 - OHIP does not have adjustments
						cyInsBalance = cyChargeAmt - cyPatientResp - pCharge->cyLineItemPaymentAmt /*- cyTotalChargeAdjustment*/;
						pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

						pRow->PutValue(COLUMN_SKIPPED, g_cvarFalse);

						// (j.jones 2009-03-05 11:41) - PLID 33235 - now we track the OHIP claim & charge pointers in the datalist
						pRow->PutValue(COLUMN_CLAIM_PTR, (long)pClaim);
						pRow->PutValue(COLUMN_CHARGE_PTR, (long)pCharge);

						m_EOBList->AddRowAtEnd(pRow,NULL);

						// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
						// function, as we may have disabled this ability
						PeekAndPump_EOBDlg();
					}

					//now we have only processed the charges specified in the remittance file, there could be more
					// (j.jones 2009-08-21 11:31) - PLID 35303 - only add charges if we have already added charges from this bill,
					// just because we have a pClaim entry with that nBillID does not mean we added charges from it
					// (j.jones 2011-03-18 15:03) - PLID 42905 - we already have the charge IDs in our memory objects,
					// what we need to do is just get a list of all charges for the same patient that we are going to 
					// post to, because maybe we just haven't added their rows yet, but will later
					CArray<long,long> aryUsedCharges;
					m_OHIPParser.GetChargeIDsByPatientID(pClaim->nPatientID, aryUsedCharges);
					if(aryUsedCharges.GetSize() > 0) {
						// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
						_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID "
							"FROM ChargesT "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"WHERE LineItemT.Deleted = 0 "
							"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"AND BillID = {INT} "
							"AND ChargesT.ID NOT IN ({INTARRAY}) "
							"AND BillID IN (SELECT BillID FROM ChargesT WHERE ID IN ({INTARRAY}))", pClaim->nBillID, aryUsedCharges, aryUsedCharges);
						while(!rsCharges->eof) {
							// (s.tullis 2016-04-15 16:00) - NX-100211
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

							long nChargeID = AdoFldLong(rsCharges, "ID");

							pRow->PutValue(COLUMN_EOB_INDEX, (long)pEOB->nIndex);
							pRow->PutValue(COLUMN_PATIENT_ID, (long)pClaim->nPatientID);
							pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pClaim->nPatientID)));
                            // (a.levy 2013-11-14 15:10) - PLID - 59445 - need to add userDefinedID here
							pClaim->nUserDefinedID = GetExistingPatientUserDefinedID(pClaim->nPatientID);
                            pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pClaim->nUserDefinedID);
							pRow->PutValue(COLUMN_BILL_ID, (long)pClaim->nBillID);
							pRow->PutValue(COLUMN_CHARGE_ID, (long)nChargeID);
							
							// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
							_variant_t varBirthDate = g_cvarNull;
							COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pClaim->nPatientID);
							if(dtBirthDate.GetStatus() != COleDateTime::invalid) {
								varBirthDate.vt = VT_DATE;
								varBirthDate.date = dtBirthDate;
							}
							pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);
							pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, _bstr_t(pClaim->strHealthRegistrationNumber));
							pRow->PutValue(COLUMN_OHIP_VERSION_CODE, _bstr_t(pClaim->strVersionCode));

							pRow->PutValue(COLUMN_DUPLICATE, (long)0);

							// (j.jones 2011-02-09 11:24) - PLID 42391 - track if this claim is a reversal
							pRow->PutValue(COLUMN_REVERSAL, g_cvarFalse);

							_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",pClaim->nBillID);
							if(!rs->eof) {
								pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
							}
							else
								pRow->PutValue(COLUMN_BILL_DATE, varNull);
							rs->Close();

							rs = CreateParamRecordset("SELECT Date, ItemCode, "
								"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
								"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes "
								"FROM ChargesT "
								"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
								"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
								"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0",nChargeID);

							COleCurrency cyChargeAmt = COleCurrency(0,0);
							COleCurrency cyPaymentAmt = COleCurrency(0,0);
							COleCurrency cyPatientAmt = COleCurrency(0,0);

							if(!rs->eof) {

								pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);
								
								//should be equivalent to pCharge->strServiceID
								pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

								// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
								_variant_t varNoteIcon;
								if(!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
									//load from the datalist's icon for no notes
									varNoteIcon = (LPCTSTR)"BITMAP:FILE";
								}
								else {
									//load our icon for having notes
									varNoteIcon = (long)m_hNotes;
								}
								pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

								// (j.jones 2011-04-25 11:12) - PLID 43406 - handle NULLs, just incase
								cyChargeAmt = AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
								pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));
							}
							else {
								pRow->PutValue(COLUMN_CHARGE_DATE, varNull);
								pRow->PutValue(COLUMN_CPT_CODE, varNull);
								// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
								pRow->PutValue(COLUMN_NOTE_ICON, varNull);
								pRow->PutValue(COLUMN_CHARGE_AMOUNT, varNull);
							}
							rs->Close();

							pRow->PutValue(COLUMN_PAYMENT, _variant_t(cyPaymentAmt));

							// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
							// (j.jones 2016-04-19 13:14) - NX-100161 - old column removed, now this is the total of positive adjustments only,
							// but it's always zero in this case
							pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

							pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

							// (j.jones 2011-03-18 09:32) - PLID 42157 - added fee sched allowable and EOB allowable,
							// but these are not supported in OHIP
							pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);

							pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);

							pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _bstr_t(""));

							pRow->PutValue(COLUMN_ADJ_REASON_CODE, _bstr_t(""));

							pRow->PutValue(COLUMN_ADJ_REASON, _bstr_t(""));
							
							pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(cyPatientAmt));
							// (s.tullis 2016-04-20 9:50) - NX-100185 
							pRow->PutRefCellFormatOverride(COLUMN_INSURED_PARTY_ID, GetInsuredPartyColumnCombo((long)pClaim->nPatientID));
							pRow->PutValue(COLUMN_INSURED_PARTY_ID, pClaim->nInsuredPartyID);

							// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
							pRow->PutValue(COLUMN_RESP_TYPE_ID, pClaim->nRespTypeID);

							// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
							pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, pClaim->bSubmitAsPrimary ? g_cvarTrue : g_cvarFalse);

							pRow->PutValue(COLUMN_INS_CO_ID, pClaim->nInsuranceCoID);

							pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(pClaim->strInsuranceCoName));

							pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);

							pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(""));

							if(pClaim->nInsuredPartyID == -1) {
								bWarnMissingInsuredParties = TRUE;
								pRow->PutForeColor(RGB(192,0,0));
							}

							//get the total of current patient applies made on this charge
							COleCurrency cyPatApplies = COleCurrency(0,0);
							COleCurrency cyPatientResp = GetChargePatientResp(nChargeID);
							COleCurrency cyPatientBalance = GetChargePatientBalance(nChargeID, pClaim->nPatientID);
							cyPatApplies = cyPatientResp - cyPatientBalance;
							pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));					

							//calculate the anticipated patient balance after this EOB is applied
							COleCurrency cyPatBalance = COleCurrency(0,0);
							//expected patient resp - existing patient applies
							cyPatBalance = (cyPatientResp < cyPatientAmt ? cyPatientAmt : cyPatientResp) - cyPatApplies;
							pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

							//calculate the anticipated insurance balance after this EOB is applied
							COleCurrency cyInsBalance = COleCurrency(0,0);
							cyInsBalance = cyChargeAmt - cyPatientAmt - cyPaymentAmt /*- cyAdjustmentAmt (Alberta doesn't have adjustments)*/;
							pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

							pRow->PutValue(COLUMN_SKIPPED, g_cvarFalse);

							// (j.jones 2009-03-05 11:41) - PLID 33235 - now we track the OHIP claim & charge pointers in the datalist
							pRow->PutValue(COLUMN_CLAIM_PTR, (long)pClaim);
							pRow->PutValue(COLUMN_CHARGE_PTR, g_cvarNull);	//there is no tracked charge in this branch of code
							// (s.tullis 2016-04-15 16:00) - NX-100211
							m_EOBList->AddRowAtEnd(pRow,NULL);

							rsCharges->MoveNext();

							// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
							// function, as we may have disabled this ability
							PeekAndPump_EOBDlg();
						}
						rsCharges->Close();
					}

					m_progress.StepIt();

					// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
					// function, as we may have disabled this ability
					PeekAndPump_EOBDlg();
				}
			}

			// (j.jones 2009-07-01 16:55) - PLID 34778 - UpdateOverpaymentLabel will check and see whether
			// the EOB, when fully posted, will potentially create a batch payment that is more
			// than the original EOB check amount was for. If so, the label will be updated.
			// (j.jones 2012-10-05 10:38) - PLID 52929 - Renamed to reflect that this now handles underpayments too,
			// for when the user wishes to skip payments and reduce the batch payment total.
			UpdateOverpaymentUnderpaymentLabel();

			m_progress.SetPos(0);
			m_progressStatus.SetWindowText("Import complete.");
			EnableButtons(TRUE);

			// (j.jones 2010-01-27 11:48) - PLID 36998 - if we had any invalid claim files, warn now
			//(copied from the ANSI logic, has not been proven to happen yet in OHIP)
			if(arystrInvalidClaims.GetSize() > 0) {
				//warn the first 10 invalid claims
				CString strClaimWarn = "The remit file did not report any services for the following claim(s):\r\n\r\n";
				for(int i=0; i<arystrInvalidClaims.GetSize() && i<10;i++) {
					strClaimWarn += arystrInvalidClaims.GetAt(i);
				}
				if(arystrInvalidClaims.GetSize() > 10) {
					strClaimWarn += "<More...>\r\n";
				}
				strClaimWarn += "\r\nYou must contact OHIP to receive a corrected remit file.\r\n"
					"No posting will be made for the listed claim(s) without a corrected remit file.";
				MessageBox(strClaimWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}

			//warn the user if data is missing

			CString strWarning;
			if(bWarnMissingInsuredParties) {

				// (j.jones 2008-06-18 11:51) - PLID 21921 - change the warning if this is OHIP
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType; Alberta uses more or less the same matching logic
				//TES 10/3/2014 - PLID 62580 - Actually, there's no need to check m_ERemitType here, this is an OHIP-specific function.
				ASSERT(m_ERemitType == ertOhip);
					strWarning = "Practice could not match up an insured party with at least one patient on this EOB.\n"
						"The patient(s) with no matching insured party have been highlighted in red.\n"
						"     - Please ensure that each patient has an insurance company, ideally one Insurance Company used by all patients.\n";
				}

			if(bWarnMissingBillIDs) {
				if(!strWarning.IsEmpty()) {
					strWarning += "\nIn addition, ";
				}
				strWarning += "Practice could not match up a bill with at least one patient on this EOB.\n"
					"The patient(s) with no matching bill have been highlighted in red, and will have no charge amount listed.\n"
					"Possible reasons for a bill to not be found are:\n"
					"     - The Bill date does not match the date of the claim on this EOB.\n"
					"     - The Bill total does not match the claim total on this EOB.\n"
					"     - The Service Codes returned on this EOB do not match the Service Codes on any bill for the patient.\n";
			}

			if(bWarnMissingInsuredParties || bWarnMissingBillIDs) {
				strWarning += "\nPractice will not be able to apply to the highlighted patient(s) until these mismatches are resolved.";

				MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}

			// wait for the insurance list to finish, and if empty, populate with insurance companies
			m_InsuranceCombo->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			// (j.jones 2012-05-02 10:16) - PLID 50138 - the combo is considered empty if
			// there is only one row and it's a built-in "show all" / "show filtered" row,
			// IsInsuranceComboEmpty will check for that state
			if(IsInsuranceComboEmpty()) {
				//this reflects an error state, because we should have a match, but we should at least
				//populate the list with the insurance companies we know the patients have
				Log("The Insurance Company dropdown list is empty! Filtering on patient insurances now.");
				// (s.tullis 2016-04-15 16:00) - NX-100211
				CString strIDList;
				for (NXDATALIST2Lib::IRowSettingsPtr pTraverseRow = m_EOBList->GetFirstRow(); pTraverseRow != nullptr; pTraverseRow = pTraverseRow->GetNextRow()) {
					if (pTraverseRow) {
						long nInsuranceCoID = VarLong(pTraverseRow->GetValue(COLUMN_INS_CO_ID), -1);
						CString str;
						str.Format("%li,", nInsuranceCoID);
						strIDList += str;
					}
				}
				strIDList.TrimRight(",");

				if(!strIDList.IsEmpty()) {
					// (j.jones 2011-06-16 08:51) - PLID 39747 - track our insurance combo filter
					m_strCurInsuranceComboFilter.Format("Archived = 0 AND InsuranceCoT.PersonID IN (%s)",strIDList);
					m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
					m_InsuranceCombo->Requery();
				}

				//while that is happening, let's warn the user about this
				CalcWarnNoInsuranceCompanyChosen();
			}

			return;
		}

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Ready to import.");
		EnableButtons(TRUE);
		return;

	}NxCatchAll("Error importing OHIP electronic remittance file.");

	// (j.jones 2010-02-05 13:08) - PLID 32184 - added more logging
	Log("OnBtnImportOhipFile() - Exception Occurred");

	m_progress.SetPos(0);
	m_progressStatus.SetWindowText("Error importing remittance file.");
	EnableButtons(TRUE);
	return;
}

void CEOBDlg::OnSelChosenEobInsuranceCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			return;
		}

		// (j.jones 2011-06-16 09:41) - PLID 39747 - handle if they try to change the filter
		long nID = VarLong(m_InsuranceCombo->GetValue(nRow, iccID), -1);
		if(nID == SHOW_ALL_INSCOS) {
			//they are looking at a filtered list, and need to see all companies,
			//so revert to our default filter and do NOT cache this as our stored filter
			m_InsuranceCombo->PutWhereClause(_bstr_t(DEFAULT_INSURANCE_FILTER));
			m_InsuranceCombo->Requery();

			//do not try to reselect anything, since they are clearly wanting to change
			//the selection themselves
			return;
		}
		else if(nID == SHOW_FILTERED_INSCOS) {
			//they are looking at a list of all companies, but we have a stored filter,
			//and they want to revert to the stored filter
			m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
			m_InsuranceCombo->Requery();

			//do not try to reselect anything, since they are clearly wanting to change
			//the selection themselves
			return;
		}

		// (j.jones 2008-07-11 16:38) - PLID 28756 - try to set our default category and description
		TrySetDefaultInsuranceDescriptions();
		// (s.tullis 2016-04-20 9:48) - NX-100186 
		EnsureWarnCorrectInsuredParties();

	}NxCatchAll("Error in CEOBDlg::OnSelChosenEobInsuranceCombo");
}


// (j.jones 2008-07-11 16:35) - PLID 28756 - this function will check
// which insurance company is selected, and override the 
// payment description and payment category 
void CEOBDlg::TrySetDefaultInsuranceDescriptions()
{
	try {

		//return if no insurance is selected
		if(m_InsuranceCombo->CurSel == -1) {
			return;
		}

		CString strDefaultPayDesc = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, iccDefaultPayDesc), "");
		long nDefaultPayCategoryID = VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, iccDefaultPayCategoryID), -1);

		CString strDefaultAdjDesc = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, iccDefaultAdjDesc), "");
		long nDefaultAdjCategoryID = VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, iccDefaultAdjCategoryID), -1);
				
		if(nDefaultPayCategoryID != -1) {
			m_PayCatCombo->SetSelByColumn(0, (long)nDefaultPayCategoryID);
			//normally when we manually select a category, it fills the description, but we do NOT do this on defaults
		}

		if(!strDefaultPayDesc.IsEmpty()) {
			m_nxeditPayDesc.SetWindowText(strDefaultPayDesc);
		}

		if(nDefaultAdjCategoryID != -1) {
			m_AdjCatCombo->SetSelByColumn(0, (long)nDefaultAdjCategoryID);
			//normally when we manually select a category, it fills the description, but we do NOT do this on defaults
		}

		if(!strDefaultAdjDesc.IsEmpty()) {
			m_nxeditAdjDesc.SetWindowText(strDefaultAdjDesc);
		}

	}NxCatchAll("Error in CEOBDlg::TrySetDefaultInsuranceDescriptions");
}

// (j.jones 2008-11-24 09:36) - PLID 32075 - added OnBtnConfigEremitAdjCodesToSkip
// (r.gonet 2016-04-18) - NX-100162 - Renamed the function because this dialog is not
// just for ignored adjustment codes anymore.
void CEOBDlg::OnBtnAdjustmentCodeSettings()
{
	try {
		// (r.gonet 2016-04-18) - NX-100162 - Renamed the dialog class because this dialog is not
		// just for ignored adjustment codes anymore.
		CAdjustmentCodeSettingsDlg dlg(this);
		if(dlg.DoModal() == IDOK && dlg.m_bInfoChanged && m_EOBList->GetRowCount() > 0) {
			// (j.jones 2008-11-24 10:19) - PLID 32075 - if anything changed in the setup,
			// and an EOB is loaded, tell the user they need to reload to incorporate the
			// changes they made
			MessageBox("Changes made to the adjustment code settings will take effect the next time an EOB is imported.\n"
				"If you wish to have these changes apply to the current EOB, you must re-import the remittance file.", "Practice", MB_ICONINFORMATION|MB_OK);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2008-12-19 08:59) - PLID 32519 - added OnTimer
void CEOBDlg::OnTimer(UINT nIDEvent) 
{
	try {

		switch(nIDEvent) {		
			case IDT_INIT_LOAD:

				KillTimer(IDT_INIT_LOAD);
				//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
				if (m_ERemitType == ertOhip) {
					OnBtnImportOhipFile();
				}
				else {
					OnBtnImport835File();
				}
				break;
		}

	}NxCatchAll("Error in CEOBDlg::OnTimer");

	CNxDialog::OnTimer(nIDEvent);
}

// (j.jones 2009-03-05 10:06) - PLID 33076 - Added IncreaseBatchPaymentBalance,
// which should only be used in OHIP postings, and is called when OHIP pays more
// on the EOB than the check was for, in which case we increment the batch payment.
void CEOBDlg::IncreaseBatchPaymentBalance(long nBatchPaymentID, COleCurrency cyAmtToIncrease)
{
	//throw exceptions to the caller
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	ASSERT(m_ERemitType == ertOhip);

	ExecuteParamSql("UPDATE BatchPaymentsT SET Amount = Amount + Convert(money, {STRING}) WHERE ID = {INT}",
		FormatCurrencyForSql(cyAmtToIncrease), nBatchPaymentID);
}

// (j.jones 2009-06-09 12:08) - PLID 33863 - this function will simply mark all "duplicate" charges as skipped
void CEOBDlg::OnBtnAutoSkipDuplicates()
{
	try {

		if(m_EOBList->GetRowCount() == 0) {
			MessageBox("You have not imported a remittance file, there are no charges to skip.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		long nCount = 0;
		// (s.tullis 2016-04-15 16:00) - NX-100211
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow(); pRow != nullptr; pRow = pRow->GetNextRow()) {

			if(pRow){
				BOOL bDuplicate = VarLong(pRow->GetValue(COLUMN_DUPLICATE)) ? TRUE : FALSE;
				if (bDuplicate) {
					pRow->PutValue(COLUMN_SKIPPED, g_cvarTrue);
					nCount++;

					// (j.jones 2012-04-18 17:11) - PLID 35306 - if a reversal (unlikely),
					// toggle its reversed sibling as skipped as well
					BOOL bIsReversal = VarBool(pRow->GetValue(COLUMN_REVERSAL), FALSE);
					//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
					if (bIsReversal && m_ERemitType != ertOhip) {
						ToggleSiblingRowSkipped(pRow, TRUE);
					}
				}
			}
		}

		if(nCount == 0) {
			MessageBox("This remittance file has no duplicate claims. No charges were skipped.", "Practice", MB_ICONINFORMATION|MB_OK);
		}
		else {

			// (j.jones 2009-07-01 16:55) - PLID 34778 - since some charges have been skipped,
			// we must now update the overpayment label
			// (j.jones 2009-07-07 15:20) - PLID 34805 - no we don't, we will not change
			// the overage amount based on what is skipped
			//UpdateOverpaymentLabel();

			CString strResult;
			strResult.Format("%li charges are flagged as being duplicates, and have been marked as skipped.", nCount);
			MessageBox(strResult, "Practice", MB_ICONINFORMATION|MB_OK);
		}

	}NxCatchAll("Error in CEOBDlg::OnBtnAutoSkipDuplicates");
}


// (j.jones 2009-07-01 16:55) - PLID 34778 - UpdateOverpaymentLabel will check and see whether
// the EOB, when fully posted, will potentially create a batch payment that is more
// than the original EOB check amount was for. If so, the label will be updated.
// (j.jones 2009-07-07 15:20) - PLID 34805 - now the batch payment will be created for
// the total of all payments in the file, it will not change dynamically
// (j.jones 2012-10-05 10:38) - PLID 52929 - renamed to reflect that this now handles underpayments too
void CEOBDlg::UpdateOverpaymentUnderpaymentLabel()
{
	try {

		//this feature currently only exists for OHIP postings
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		if (m_ERemitType != ertOhip || m_OHIPParser.m_paryOHIPEOBInfo.GetSize() == 0) {
			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);
			return;
		}

		//we have to process through each batch payment, there could be more than one

		COleCurrency cyTotalCheckAmt = COleCurrency(0,0);
		COleCurrency cyTotalChange = COleCurrency(0,0);

		for(int i=0;i<m_OHIPParser.m_paryOHIPEOBInfo.GetSize();i++) {

			OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(i);

			//for this EOB only, figure out the total payment amount for all non-skipped charges
			COleCurrency cyTotalPayments = COleCurrency(0,0);
			// (s.tullis 2016-04-15 16:00) - NX-100211
			for(NXDATALIST2Lib::IRowSettingsPtr pLoopRow = m_EOBList->GetFirstRow();pLoopRow != nullptr ;pLoopRow = pLoopRow->GetNextRow()) {

				if(pEOB->nIndex == VarLong(pLoopRow->GetValue(COLUMN_EOB_INDEX))) {

					// (j.jones 2009-07-07 15:20) - PLID 34805 - do not ignore anything,
					// the batch payment will be created for the total of all payments in the file

					//ignore the skipped rows					
					/*
					if(VarBool(m_EOBList->GetValue(j,COLUMN_SKIPPED))) {
						continue;
					}

					//if we find a -1 patient ID, we definitely cannot post, so don't include it in the calculation
					if(VarLong(m_EOBList->GetValue(j,COLUMN_PATIENT_ID)) == -1) {
						continue;
					}
					*/

					COleCurrency cyPayment = VarCurrency(pLoopRow->GetValue(COLUMN_PAYMENT),COleCurrency(0,0));

					cyTotalPayments += cyPayment;
				}
			}

			//now, do the total payments exceed the EOB check total?

			// (j.jones 2012-10-05 08:37) - PLID 52929 - if the setting to ignore missing payments is on,
			// we may reduce the batch payment total to match the amount to be applied
			if((cyTotalPayments > pEOB->cyTotalPaymentAmt) || (m_bOHIPIgnoreMissingPatients && cyTotalPayments < pEOB->cyTotalPaymentAmt)) {

				//it does, so update our total amount
				cyTotalChange += (cyTotalPayments - pEOB->cyTotalPaymentAmt);
			}

			cyTotalCheckAmt += pEOB->cyTotalPaymentAmt;
		}

		if(cyTotalChange != COleCurrency(0,0)) {

			//we believe there will be an overpayment or underpayment, so warn the user via the label

			//Do we have multiple batch payments? If not, we can give a pretty specific label,
			//but if we do, it will be a little bit more generic.

			CString strWarn;

			// (j.jones 2009-07-07 15:20) - PLID 34805 - now the batch payment will be created for
			// the total of all payments in the file, it will not change dynamically, so these labels
			// have changed slightly to reflect that

			if(m_OHIPParser.m_paryOHIPEOBInfo.GetSize() > 1) {
				//more than one batch payment, so simply reflect the overpayment amount
				if(cyTotalChange > COleCurrency(0,0)) {
					strWarn.Format("Some EOBs in this file contain overpayments that are more than their original payments are for, totalling %s in overage.",
						FormatCurrencyForInterface(cyTotalChange));
				}
				else if(cyTotalChange < COleCurrency(0,0)) {
					// (j.jones 2012-10-05 10:45) - PLID 52929 - handle underpayments
					strWarn.Format("Some EOBs in this file have payments that will be skipped, reducing the total payment amount to %s.",
						FormatCurrencyForInterface(cyTotalCheckAmt + cyTotalChange));
				}
			}
			else {

				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(0);

				//just one batch payment
				if(cyTotalChange > COleCurrency(0,0)) {
					strWarn.Format("This EOB contains %s in payments, which is %s more than the original payment is for.",
						FormatCurrencyForInterface(pEOB->cyTotalPaymentAmt + cyTotalChange), FormatCurrencyForInterface(cyTotalChange));
				}
				else if(cyTotalChange < COleCurrency(0,0)) {
					// (j.jones 2012-10-05 10:45) - PLID 52929 - handle underpayments
					strWarn.Format("This EOB contains payments that will be skipped, reducing the total payment amount to %s.",
						FormatCurrencyForInterface(pEOB->cyTotalPaymentAmt + cyTotalChange));
				}
			}

			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText(strWarn);
			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_SHOW);
		}
		else {
			//there is no overpayment estimated, so hide the label
			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
			m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-08 15:33) - PLID 37174 - added ability to configure import filtering
void CEOBDlg::OnBtnConfigureEremitImportFiltering()
{
	try {

		//not supported in OHIP (the button should not be available)
		//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
		if (m_ERemitType != ertAnsi) {
			return;
		}

		if(m_EOBList->GetRowCount() > 0) {
			MessageBox("You have already imported a remittance file. "
				"Any changes you make to EOB filtering will not take effect until you import your next EOB file.", "Practice", MB_ICONINFORMATION|MB_OK);
		}

		CERemitImportFilteringDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
// can optionally disable PeekAndPump usage for the posting process
void CEOBDlg::PeekAndPump_EOBDlg()
{
	if(m_bEnablePeekAndPump) {
		PeekAndPump();
	}
}

// (j.jones 2010-04-09 13:03) - PLID 31309 - added date controls
void CEOBDlg::OnCheckEnablePayDate()
{
	try {

		GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(m_checkEnablePayDate.GetCheck());

		// (j.jones 2010-11-30 10:00) - PLID 39434 - if they unchecked the date,
		// we should revert the EOB date, always
		if(!m_checkEnablePayDate.GetCheck()) {
			COleDateTime dtPayDate = COleDateTime::GetCurrentTime();
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (m_ERemitType == ertOhip && m_OHIPParser.m_paryOHIPEOBInfo.GetSize() > 0) {
				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(0);
				if(pEOB->dtPaymentDate.GetStatus() != COleDateTime::invalid) {
					dtPayDate = pEOB->dtPaymentDate;
				}
			}
			else if(m_ERemitType == ertAnsi && m_EOBParser.m_arypEOBInfo.GetSize() > 0) {
				const EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[0];
				if(pEOB->dtCheckDate.GetStatus() != COleDateTime::invalid) {
					dtPayDate = pEOB->dtCheckDate;
				}
			}
			else if (m_ERemitType == ertAlberta && m_AlbertaParser.m_pChargeList->size() > 0) {
				dtPayDate = m_AlbertaParser.m_dtFirstExpectedPaymentDate;
			}
			m_dtPayDate.SetValue(dtPayDate);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-04-09 13:03) - PLID 31309 - added date controls
void CEOBDlg::OnCheckEnableAdjDate()
{
	try {

		GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(m_checkEnableAdjDate.GetCheck());

		// (j.jones 2010-11-30 10:00) - PLID 39434 - if they unchecked the date,
		// we should revert the EOB date, always
		if(!m_checkEnableAdjDate.GetCheck()) {
			COleDateTime dtAdjDate = COleDateTime::GetCurrentTime();
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (m_ERemitType == ertOhip && m_OHIPParser.m_paryOHIPEOBInfo.GetSize() > 0) {
				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_OHIPParser.m_paryOHIPEOBInfo.GetAt(0);
				if(pEOB->dtPaymentDate.GetStatus() != COleDateTime::invalid) {
					dtAdjDate = pEOB->dtPaymentDate;
				}
			}
			else if(m_ERemitType == ertAnsi && m_EOBParser.m_arypEOBInfo.GetSize() > 0) {
				const EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[0];
				if(pEOB->dtCheckDate.GetStatus() != COleDateTime::invalid) {
					dtAdjDate = pEOB->dtCheckDate;
				}
			}
			else if (m_ERemitType == ertAlberta && m_AlbertaParser.m_pChargeList->size() > 0) {
				//TES 10/3/2014 - PLID 62580 - Alberta doesn't have adjustments on the claim (though they could have manually entered some), so use the payment date
				dtAdjDate = m_AlbertaParser.m_dtFirstExpectedPaymentDate;
			}
			m_dtAdjDate.SetValue(dtAdjDate);
		}

	}NxCatchAll(__FUNCTION__);
}


// (j.jones 2011-01-06 16:57) - PLID 41785 - added ability to create a billing note from patient
// responsibility reasons & a patient responsibility amount
void CEOBDlg::CreateBillingNoteFromPatientRespReasons(long nPatientID, long nChargeID, CString strInsuranceCoName,
													  EOBClaimInfo *pClaim, EOBLineItemInfo *pCharge, COleCurrency cyPatResp)
{
	try {

		if(pClaim == NULL && pCharge == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(cyPatResp == COleCurrency(0,0)) {
			//no resp., nothing to report
			return;
		}

		CStringArray arystrPatientRespReasons;

		//append each unique reason we have, starting with the claim adjustments

		// (j.jones 2012-04-24 15:47) - PLID 35306 - don't make a note from reversed claims
		if(pClaim != NULL && !pClaim->bIsReversedClaim) {
			for(int nAdjIndex=0; nAdjIndex<pClaim->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

				const EOBAdjustmentInfo *pAdj = pClaim->arypEOBAdjustmentInfo[nAdjIndex];
				if(pAdj->bPatResp) {
					if(!pAdj->strReason.IsEmpty()) {
						//add the reason if it does not already exist
						BOOL bFound = FALSE;
						for(int nReasonIndex = 0; nReasonIndex < arystrPatientRespReasons.GetSize() && !bFound; nReasonIndex++) {
							if(arystrPatientRespReasons.GetAt(nReasonIndex).CompareNoCase(pAdj->strReason) == 0) {
								bFound = TRUE;
							}
						}
						if(!bFound) {
							arystrPatientRespReasons.Add(pAdj->strReason);
						}
					}
					else {
						// (j.jones 2011-01-07 10:10) - it should be impossible to be empty
						// unless we do not have the latest reason codes in the database
						ASSERT(FALSE);
					}
				}
			}
		}

		if(pCharge != NULL) {
			for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

				const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];
				if(pAdj->bPatResp) {
					if(!pAdj->strReason.IsEmpty()) {
						//add the reason if it does not already exist
						BOOL bFound = FALSE;
						for(int nReasonIndex = 0; nReasonIndex < arystrPatientRespReasons.GetSize() && !bFound; nReasonIndex++) {
							if(arystrPatientRespReasons.GetAt(nReasonIndex).CompareNoCase(pAdj->strReason) == 0) {
								bFound = TRUE;
							}
						}
						if(!bFound) {
							arystrPatientRespReasons.Add(pAdj->strReason);
						}
					}
					else {
						// (j.jones 2011-01-07 10:10) - it should be impossible to be empty
						// unless we do not have the latest reason codes in the database
						ASSERT(FALSE);
					}
				}
			}
		}

		long nCategoryID = GetRemotePropertyInt("ERemit_AddCASPR_BillingNote_DefCategory", -1, 0, "<None>", true);
		//don't waste a recordset validating that the category still exists, we will do so in the execute

		BOOL bShowOnStatement = (GetRemotePropertyInt("ERemit_AddCASPR_BillingNote_ShowOnStatement", 0, 0, "<None>", true) != 0);

		//now build the note from our cyPatResp and our reasons, as it is possible (albeit not common)
		//for the EOB to report multiple reasons, and we have to show all of them

		CString strReasons;
		for(int nReasonIndex = 0; nReasonIndex < arystrPatientRespReasons.GetSize(); nReasonIndex++) {
			if(!strReasons.IsEmpty()) {
				strReasons += ", ";
			}
			strReasons += arystrPatientRespReasons.GetAt(nReasonIndex);
		}

		//format it nicely
		if(!strReasons.IsEmpty()) {
			strReasons = " (" + strReasons + ")";
		}
		
		CString strNote;
		// (j.jones 2011-03-22 14:39) - PLID 42946 - reworded this sentence
		strNote.Format("%s states patient owes %s%s.", strInsuranceCoName, FormatCurrencyForInterface(cyPatResp), strReasons);

		//now save the note
		CreateBillingNote(nPatientID, nChargeID, strNote, nCategoryID, bShowOnStatement);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-14 15:21) - PLID 50285 - moved CreateBillingNote to globalfinancialutils

// (j.jones 2011-01-07 14:25) - PLID 41980 - added ability to create a billing note with detailed
// patient resp. reasons and allowable information
void CEOBDlg::CreateBillingNoteWithDetailedInfo(long nPatientID, long nChargeID, CString strInsuranceCoName,
									   EOBClaimInfo *pClaim, EOBLineItemInfo *pCharge)
{
	try {

		if(pClaim == NULL && pCharge == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		CString strDetails;
		CString strAllowable;

		//append each unique amount and reason we have, starting with the claim adjustments
		if(pClaim != NULL) {
			for(int nAdjIndex=0; nAdjIndex<pClaim->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

				const EOBAdjustmentInfo *pAdj = pClaim->arypEOBAdjustmentInfo[nAdjIndex];
				if(pAdj->bPatResp && pAdj->cyAdjustmentAmt > COleCurrency(0,0)) {
					if(!pAdj->strReason.IsEmpty()) {

						if(!strDetails.IsEmpty()) {
							strDetails += ", ";
						}

						CString str;
						str.Format("%s: %s", pAdj->strReason, FormatCurrencyForInterface(pAdj->cyAdjustmentAmt));
						strDetails += str;
					}
					else {
						// (j.jones 2011-01-07 10:10) - it should be impossible to be empty
						// unless we do not have the latest reason codes in the database
						ASSERT(FALSE);
					}
				}
			}
		}

		if(pCharge != NULL) {

			//check charge allowables
			if(pCharge->cyChargeAllowedAmt.GetStatus() != COleCurrency::invalid
				&& pCharge->cyChargeAllowedAmt > COleCurrency(0,0)) {
				strAllowable.Format("Allowable of %s", FormatCurrencyForInterface(pCharge->cyChargeAllowedAmt));
			}

			for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

				const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];
				if(pAdj->bPatResp && pAdj->cyAdjustmentAmt > COleCurrency(0,0)) {
					if(!pAdj->strReason.IsEmpty()) {

						if(!strDetails.IsEmpty()) {
							strDetails += ", ";
						}

						CString str;
						str.Format("%s of %s", pAdj->strReason, FormatCurrencyForInterface(pAdj->cyAdjustmentAmt));
						strDetails += str;
					}
					else {
						// (j.jones 2011-01-07 10:10) - it should be impossible to be empty
						// unless we do not have the latest reason codes in the database
						ASSERT(FALSE);
					}
				}
			}
		}

		//now add the allowable, if we have one
		if(!strAllowable.IsEmpty()) {
			if(!strDetails.IsEmpty()) {
				strDetails += ", ";
			}

			strDetails += strAllowable;
		}

		//if our note is empty, leave
		if(strDetails.IsEmpty()) {
			return;
		}

		long nCategoryID = GetRemotePropertyInt("ERemit_AddDetailedInfo_DefCategory", -1, 0, "<None>", true);
		//don't waste a recordset validating that the category still exists, we will do so in the execute

		BOOL bShowOnStatement = (GetRemotePropertyInt("ERemit_AddDetailedInfo_ShowOnStatement", 0, 0, "<None>", true) != 0);

		//now build the note to send, creatively phrasing it with "a" or "an" if it starts with a vowel
		//(fun fact: technically it is if the spoken word, not the written word, sounds like a vowel,
		//but we're not going to try to get that precise, sorry)
		CString strNote, strAn = "a";
		if(strDetails.Left(1).FindOneOf("aeiouAEIOU") != -1) {
			strAn = "an";
		}

		// (j.jones 2011-03-22 14:39) - PLID 42946 - reworded this sentence
		strNote.Format("%s states %s %s", strInsuranceCoName, strAn, strDetails);

		//now save the note
		CreateBillingNote(nPatientID, nChargeID, strNote, nCategoryID, bShowOnStatement);

	}NxCatchAll(__FUNCTION__);
}

//TES 9/18/2014 - PLID 62581 - Creates a note on the charge explaining why Alberta reduced a payment
//TES 10/8/2014 - PLID 62581 - This creates a bill note now, so I removed ChargeID and added some information about the charge
void CEOBDlg::CreateBillingNoteFromPartialPayment(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount)
{
	CString strPartialNote;
	//TES 9/18/2014 - PLID 62581 - Go through the explanation codes and build the note
	foreach(AssessmentExplanation exp, pCharge->aryExplanationCodes) {
		if (!strPartialNote.IsEmpty()) strPartialNote += "\r\n\r\n";
		strPartialNote += exp.strCode + " - " + exp.strDescription;
	}
	//TES 9/18/2014 - PLID 62581 - If there's no explanation code, don't post a note at all.
	if (strPartialNote.IsEmpty()) {
		return;
	}

	//TES 9/18/2014 - PLID 62581 - Now save the note
	//TES 10/8/2014 - PLID 62581 - Add charge information, tell CreateBillingNote() to create a bill note rather than a charge note
	CString strNote;
	strNote.Format("PARTIAL: Service Code: %s; Service Date: %s, Charge Total: %s\r\n%s", strServiceCode, FormatDateTimeForInterface(dtChargeDate), FormatCurrencyForInterface(cyChargeAmount), strPartialNote);
	CreateBillingNote(nPatientID, nBillID, strNote, -1, FALSE, TRUE);
}

//TES 9/18/2014 - PLID 62582 - Creates a note on the charge explaining why Alberta refused a claimm
//TES 10/8/2014 - PLID 62582 - This creates a bill note now, so I removed ChargeID and added some information about the charge
void CEOBDlg::CreateBillingNoteFromRefusal(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount)
{
	CString strRefusalNote;
	//TES 9/18/2014 - PLID 62582 - Go through the explanation codes and build the note
	foreach(AssessmentExplanation exp, pCharge->aryExplanationCodes) {
		if (!strRefusalNote.IsEmpty()) strRefusalNote += "\r\n\r\n";
		strRefusalNote += exp.strCode + " - " + exp.strDescription;
	}
	//TES 9/18/2014 - PLID 62582 - If there's no explanation code, don't post a note at all.
	if (strRefusalNote.IsEmpty()) {
		return;
	}

	//TES 9/18/2014 - PLID 62582 - Now save the note
	//TES 10/8/2014 - PLID 62582 - Add charge information, tell CreateBillingNote() to create a bill note rather than a charge note
	CString strNote;
	strNote.Format("REFUSAL: Service Code: %s; Service Date: %s, Charge Total: %s\r\n%s", strServiceCode, FormatDateTimeForInterface(dtChargeDate), FormatCurrencyForInterface(cyChargeAmount), strRefusalNote);
	CreateBillingNote(nPatientID, nBillID, strNote, -1, FALSE, TRUE);
}

//TES 9/26/2014 - PLID 62536 - Create a note when reversing a payment on a charge
//TES 10/8/2014 - PLID 62536 - This creates a bill note now, so I removed ChargeID and added some information about the charge
void CEOBDlg::CreateBillingNoteFromReversalReason(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount)
{
	CString strReversalNote;
	//TES 9/26/2014 - PLID 62536 - Go through the explanation codes and build the note
	foreach(AssessmentExplanation exp, pCharge->aryExplanationCodes) {
		if (!strReversalNote.IsEmpty()) strReversalNote += "\r\n\r\n";
		strReversalNote += exp.strCode + " - " + exp.strDescription;
	}
	//TES 9/26/2014 - PLID 62536 - If there's no explanation code, don't post a note at all.
	if (strReversalNote.IsEmpty()) {
		return;
	}

	//TES 9/26/2014 - PLID 62536 - Now save the note
	//TES 10/8/2014 - PLID 62536 - Add charge information, tell CreateBillingNote() to create a bill note rather than a charge note
	CString strNote;
	strNote.Format("REVERSAL: Service Code: %s; Service Date: %s, Charge Total: %s\r\n%s", strServiceCode, FormatDateTimeForInterface(dtChargeDate), FormatCurrencyForInterface(cyChargeAmount), strReversalNote);
	CreateBillingNote(nPatientID, nBillID, strNote, -1, FALSE, TRUE);
}

//TES 10/2/2014 - PLID 63821 - Sets the Bill Status Note to indicate that the bill is held for review, including any explanation codes
void CEOBDlg::SetBillStatusNoteFromHoldReason(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge)
{
	CString strHoldNote;
	//TES 10/2/2014 - PLID 63821 - Go through the explanation codes and build the note
	foreach(AssessmentExplanation exp, pCharge->aryExplanationCodes) {
		if (!strHoldNote.IsEmpty()) strHoldNote += "\r\n\r\n";
		strHoldNote += exp.strCode + " - " + exp.strDescription;
	}

	CString strNote = "Held for review by Alberta Health";
	//TES 10/2/2014 - PLID 63821 - I'm not actually sure whether there will ever be Explanatory Codes on held claims
	if (!strHoldNote.IsEmpty()) {
		strNote += ": " + strHoldNote;
	}
	SetBillStatusNote(nBillID, nPatientID, strNote);
}

// (j.jones 2011-02-09 12:08) - PLID 42391 - added to prevent un-skipping reversals

// (j.jones 2011-03-16 10:21) - PLID 21559 - detects if any of our payments were under the allowed amount,
// and offers to run the report to show them
void CEOBDlg::CheckForPaymentsUnderAllowedAmount(long nEOBID)
{
	try {

		CWaitCursor pWait;

		// (j.jones 2011-03-18 10:21) - PLID 42157 - Since we have the fee schedule allowable already loaded
		// (and multiplied by quantity & modifiers), we can skip the report query if no allowable is filled in
		// or if the payment is not less than this value for any patient. This may find false positives that
		// the report query will weed out (because it adds in other data to the payment amount), but it's a
		// quick way to decide to not run that query to begin with.

		BOOL bShouldRunReport = FALSE;
		// (s.tullis 2016-04-15 16:00) - NX-100211
		for(NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr = m_EOBList->GetFirstRow(); pLoopRowPtr != nullptr && !bShouldRunReport; pLoopRowPtr= pLoopRowPtr->GetNextRow()) {
			COleCurrency cyAllowableQty = VarCurrency(pLoopRowPtr->GetValue(COLUMN_FEE_SCHED_ALLOWABLE), COleCurrency(0,0));
			COleCurrency cyPayment = VarCurrency(pLoopRowPtr->GetValue(COLUMN_PAYMENT), COleCurrency(0,0));
			if(cyAllowableQty > COleCurrency(0,0) && cyPayment < cyAllowableQty) {
				//there is at least one allowable filled in, and the payment is less than that amount,
				//so we must run the report query to validate it further
				bShouldRunReport = TRUE;
			}
		}

		if(!bShouldRunReport) {
			return;
		}

		CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(366)];
		infReport.nExtraID = nEOBID;

		// (j.jones 2011-03-16 10:21) - PLID 21559 - detect if any of our payments were under the allowed amount
		_RecordsetPtr rsReportInfo = infReport.GetRecordset(0, 0, FALSE);
		
		bShouldRunReport = FALSE;
		if(!rsReportInfo->eof) {
			bShouldRunReport = (IDYES == MessageBox("This EOB posted payments that were under the fee schedule's allowed amount. "
				"Would you like to view a report of these payments now?", "Practice", MB_ICONQUESTION|MB_YESNO));
		}
		rsReportInfo->Close();
		
		if(!bShouldRunReport) {
			return;
		}
		
		CPtrArray params;
		CRParameterInfo *tmpParam;

		tmpParam = new CRParameterInfo;
		tmpParam->m_Data = GetCurrentUserName();
		tmpParam->m_Name = "CurrentUserName";
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateTo";
		tmpParam->m_Data = "12/31/5000";
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateFrom";
		tmpParam->m_Data = "01/01/1000";
		params.Add((void *)tmpParam);

		infReport.strReportFile += "ServiceDtld";

		RunReport(&infReport, &params, true, (CWnd *)this, "Payments Under Allowed Amount (By Insurance)");

		ClearRPIParameterList(&params);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up the warning
// log to the server's NexTech\EOBWarningLogs path, and also ensures that files
// > 30 days old are deleted
void CEOBDlg::CloseWarningLogAndCopyToServer()
{
	try {

		//first close the log file
		m_fileLogWarnings.Close();

		//calculate the server's log path
		CString strServerPath = GetNxEOBWarningLogsPath();

		//remove all EOBWarning logs that are > 30 days old (or 365 days if OHIP)
		{
			CTime tm;
			//this assumes that at least the system's date is accurate
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(m_ERemitType == ertOhip ? 365 : 30, 0, 0, 0);

			CFileFind finder;
			BOOL bWorking = finder.FindFile(strServerPath ^ "EOBWarnings*.txt");	
			while(bWorking) {
				bWorking = finder.FindNextFile();

				CString strFilePath = finder.GetFilePath();
				tm = FileUtils::GetFileModifiedTime(strFilePath); 
				if(tm < tmMin) {
					DeleteFile(strFilePath);
				}
			}
		}
	
		//now copy our warning log to the server
		{
			COleDateTime dtServer = GetRemoteServerTime();
			CString strNewFileName;
			strNewFileName.Format("%s\\EOBWarnings_%s_%s.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));
			
			//it's very unlikely that this file will exist, but handle the case anyways
			int nCount = 0;
			while(DoesExist(strNewFileName)) {				
				
				//try adding an index to the end
				nCount++;

				if(nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy log to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\EOBWarnings_%s_%s_%li.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if(!CopyFile(m_strWarningOutputFile, strNewFileName, TRUE)) {
				//failed
				ThrowNxException("Cannot copy log to server, filename: %s", strNewFileName);
			}
		}

	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2016-04-15 16:00) - NX-100211- Changed Return value to row pointer
// (j.jones 2011-06-02 09:48) - PLID 43931 - added FindChargeInList, which searches by ChargeID
// or by charge pointer, and returns the row index, -1 if not found
//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
NXDATALIST2Lib::IRowSettingsPtr CEOBDlg::FindChargeInList(long nChargeID, EOBLineItemInfo *pCharge, OHIPEOBLineItemInfo *pOHIPCharge, AlbertaAssessments::ChargeInfoPtr pAlbertaCharge)
{
	// (j.jones 2011-09-27 10:55) - PLID 45489 - if we have a pointer, search by that as first
	// priority, to handle the rare case where the same charge ID exists more than once
	NXDATALIST2Lib::IRowSettingsPtr pSearchResults;

	if(pCharge != NULL) {
		pSearchResults = m_EOBList->FindByColumn(COLUMN_CHARGE_PTR, (long)pCharge, m_EOBList->GetFirstRow(), FALSE);
		//return whatever we found
		return pSearchResults;
	}

	if(pOHIPCharge != NULL) {
		pSearchResults =	m_EOBList->FindByColumn(COLUMN_CHARGE_PTR, (long)pOHIPCharge, m_EOBList->GetFirstRow(), FALSE);
		//return whatever we found
		return pSearchResults;
	}

	//TES 9/18/2014 - PLID 62777 - Added pAlbertaCharge
	if (pAlbertaCharge != NULL) {
		pSearchResults = m_EOBList->FindByColumn(COLUMN_CHARGE_PTR, (long)pAlbertaCharge.get(), m_EOBList->GetFirstRow(), FALSE);
		return pSearchResults;
	}

	if(nChargeID != -1) {
		pSearchResults =	m_EOBList->FindByColumn(COLUMN_CHARGE_ID, nChargeID, m_EOBList->GetFirstRow(), FALSE);
		//return whatever we found
		return pSearchResults;
	}

	//we had nothing to search on
	return nullptr;
}

// (j.jones 2011-06-15 17:50) - PLID 39747 - added OnRequeryFinished to add a show more/less row
void CEOBDlg::OnRequeryFinishedEobInsuranceCombo(short nFlags)
{
	try {

		if(m_strCurInsuranceComboFilter == DEFAULT_INSURANCE_FILTER) {
			//do nothing, we are currently showing all companies, unfiltered
			return;
		}
		else {
			//m_strCurInsuranceComboFilter might not really be the current filter,
			//if we are already expanding a previously filtered list to "show all companies"

			CString strCurrentWhere = (LPCTSTR)m_InsuranceCombo->GetWhereClause();
			if(strCurrentWhere == m_strCurInsuranceComboFilter) {
				//it is currently filtered, so add "show all companies"

				NXDATALISTLib::IRowSettingsPtr pRow = m_InsuranceCombo->GetRow(-1);
				pRow->PutValue(iccID, SHOW_ALL_INSCOS);
				pRow->PutValue(iccName, " <Show All Companies>");
				pRow->PutValue(iccAddress, "");
				pRow->PutValue(iccDefaultPayDesc, g_cvarNull);
				pRow->PutValue(iccDefaultPayCategoryID, g_cvarNull);
				pRow->PutValue(iccDefaultAdjDesc, g_cvarNull);
				pRow->PutValue(iccDefaultAdjCategoryID, g_cvarNull);
				m_InsuranceCombo->AddRow(pRow);
			}
			else {
				//it is currently showing all companies, but there is a stored filter,
				//so add "show filtered companies"

				NXDATALISTLib::IRowSettingsPtr pRow = m_InsuranceCombo->GetRow(-1);
				pRow->PutValue(iccID, SHOW_FILTERED_INSCOS);
				pRow->PutValue(iccName, " <Show Filtered Companies>");
				pRow->PutValue(iccAddress, "");
				pRow->PutValue(iccDefaultPayDesc, g_cvarNull);
				pRow->PutValue(iccDefaultPayCategoryID, g_cvarNull);
				pRow->PutValue(iccDefaultAdjDesc, g_cvarNull);
				pRow->PutValue(iccDefaultAdjCategoryID, g_cvarNull);
				m_InsuranceCombo->AddRow(pRow);
			}
		}

		//if filtered by a setup group ID, add "show all companies"
		//if not filtered, add "show fewer companies", but ONLY if we have
		//a filtered group ID to use, make sure it is -1 if we have no loaded EOB!

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-03 10:34) - PLID 45785 - I moved HasInvalidPayTotals and GetPaymentTotalsForCharges
// into the parser class, and changed all references to call the parser's functions for these.

// (j.jones 2011-12-19 15:52) - PLID 43925 - added ability to save deductible and coinsurance information per charge
void CEOBDlg::TrySaveCoinsurance(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID, EOBLineItemInfo *pCharge)
{
	try {

		if(pCharge == NULL) {
			ThrowNxException("Invalid charge pointer information given.");
		}

		COleCurrency cyDeductible = COleCurrency(0,0);
		COleCurrency cyCoinsurance = COleCurrency(0,0);
		COleCurrency cyCopay = COleCurrency(0,0);

		//search all charge adjustments for patient adjustments (PR)
		//for deductible (1) or coinsurance (2)
		for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {

			const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];
			if(pAdj->bPatResp && pAdj->cyAdjustmentAmt > COleCurrency(0,0)) {
				if(pAdj->strReasonCode == "1") {
					//deductible is code 1
					cyDeductible += pAdj->cyAdjustmentAmt;
				}
				else if(pAdj->strReasonCode == "2") {
					//coinsurance is code 2
					cyCoinsurance += pAdj->cyAdjustmentAmt;
				}
				// (j.jones 2013-08-27 11:53) - PLID 57398 - added copay
				else if(pAdj->strReasonCode == "3") {
					//copay is code 3
					cyCopay += pAdj->cyAdjustmentAmt;
				}
			}
		}

		//this will save a record even if all values are zero, indicating that we received an EOB,
		//and the charge had neither a deductible, a coinsurance, nor a copay
		SaveChargeCoinsurance(nPatientID, strPatientName, nChargeID, nInsuredPartyID, cyDeductible, cyCoinsurance, cyCopay, ccemERemittance);

	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2016-04-15 16:00) - NX-100211 - changed return to row pointer
// (j.jones 2012-04-18 16:44) - PLID 35306 - Added FindReversedSiblingRow, which searches by ChargeID/charge pointer,
// and returns the row for the reversed charge.
NXDATALIST2Lib::IRowSettingsPtr CEOBDlg::FindReversedSiblingRow(NXDATALIST2Lib::IRowSettingsPtr pSiblingRow, long nSiblingChargeID, const EOBClaimInfo *pSiblingClaim, const EOBLineItemInfo *pSiblingCharge)
{
	//throw exceptions to the caller

	//we can't search for a sibling if the caller isn't matched to a valid charge
	if(nSiblingChargeID == -1 || pSiblingCharge == NULL || pSiblingClaim == NULL) {
		return -1;
	}

	//this is a US-only function
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType == ertOhip) {
		return -1;
	}
	//find the same charge ID with the opposite reported amount
	for (NXDATALIST2Lib::IRowSettingsPtr pSearchRow = m_EOBList->GetFirstRow(); pSearchRow != nullptr; pSearchRow = pSearchRow->GetNextRow()) {
		//skip the calling row
		if(pSearchRow == pSiblingRow) {
			continue;
		}

		long nChargeID = VarLong(pSearchRow->GetValue(COLUMN_CHARGE_ID), -1);

		EOBLineItemInfo *pCharge = NULL;
		_variant_t var = pSearchRow->GetValue(COLUMN_CHARGE_PTR);
		if(var.vt == VT_I4) {
			pCharge = (EOBLineItemInfo*)VarLong(var);
		}
		EOBClaimInfo *pClaim = NULL;
		var = pSearchRow->GetValue(COLUMN_CLAIM_PTR);
		if(var.vt == VT_I4) {
			pClaim = (EOBClaimInfo*)VarLong(var);
		}
		
		//if either pointer is invalid, we can't check this row
		if(pClaim == NULL || pCharge == NULL) {
			//This isn't unusual, could be a charge we added manually from a bill
			//that just wasn't mentioned on the EOB. If so, it's not being posted to.
			continue;
		}

		//if the pointers are the same, continue
		if(pCharge == pSiblingCharge) {
			continue;
		}

		//if the charge IDs are the same and they are both reversals
		//and the pointer's charge amounts are opposites, that's our match

		// (j.jones 2012-09-06 18:02) - PLID 52500 - added a check for non-zero charges, as we cannot possibly
		// compare a negative and a positive charge if they are both zero
		if(nChargeID == nSiblingChargeID && pCharge->cyLineItemChargeAmt != COleCurrency(0,0) && pCharge->cyLineItemChargeAmt == -pSiblingCharge->cyLineItemChargeAmt) {
			//Found a matched row!

			if(pClaim->bIsRepostedClaim && pSiblingClaim->bIsReversedClaim
				&& pClaim->pReversedSibling == pSiblingClaim
				&& pSiblingClaim->pRepostedSibling == pClaim) {

				//this is a matched row, this claim is the reposted claim,
				//the calling sibling claim is the reversed claim
				return pSearchRow;
			}
			else if(pClaim->bIsReversedClaim && pSiblingClaim->bIsRepostedClaim
				&& pClaim->pRepostedSibling == pSiblingClaim
				&& pSiblingClaim->pReversedSibling == pClaim) {

				//this is a matched row, this claim is the reversed claim,
				//the calling sibling claim is the reposted claim
				return pSearchRow;
			}
			else {
				//don't fail, this may be ok, but if we hit a row that matches perfectly
				//but is not marked as a reversal, we should really look into it to
				//make sure we are properly flagging these as reversals to begin with
				ASSERT(FALSE);
				continue;
			}
		}
	}

	//no sibling found
	return nullptr;	
}

// (j.jones 2012-04-18 16:44) - PLID 35306 - Added ToggleSiblingRowSkipped, which will set the reversed sibling
// of our given row to be skipped, per our passed-in parameter. Can also optionally color the row red.
void CEOBDlg::ToggleSiblingRowSkipped(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bSkipped, BOOL bColorRowRed /*= FALSE*/)
{
	//throw exceptions to the caller

	//must have a valid row
	if(pRow == nullptr) {
		return;
	}

	//this is a US-only function
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType == ertOhip) {
		return;
	}

	long nChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID));

	EOBClaimInfo *pClaim = NULL;
	_variant_t var = pRow->GetValue(COLUMN_CLAIM_PTR);
	if(var.vt == VT_I4) {
		pClaim = (EOBClaimInfo*)VarLong(var);
	}

	EOBLineItemInfo *pCharge = NULL;				
	var = pRow->GetValue(COLUMN_CHARGE_PTR);
	if(var.vt == VT_I4) {
		pCharge = (EOBLineItemInfo*)VarLong(var);
	}

	BOOL bIsReversal = VarBool(pRow->GetValue(COLUMN_REVERSAL), FALSE);
	// (s.tullis 2016-04-15 16:00) - NX-100211
	//if the current charge is a reversal, and has a valid charge ID and pointer,
	//find its opposite sibling, and set its skipped status to match bSkipped
	if(bIsReversal && nChargeID != -1 && pCharge != NULL && pClaim != NULL) {
		NXDATALIST2Lib::IRowSettingsPtr pMatchedSiblingRow = FindReversedSiblingRow(pRow, nChargeID, pClaim, pCharge);
		if(pMatchedSiblingRow) {
			//we found a matched sibling row, toggle its skipped status to match this one
			pMatchedSiblingRow->PutValue(COLUMN_SKIPPED, bSkipped ? g_cvarTrue : g_cvarFalse);

			//if requested, color the row red
			if(bColorRowRed) {
				pMatchedSiblingRow->PutForeColor(RGB(192,0,0));
			}
		}
	}
}

// (j.dinatale 2013-01-10 08:46) - PLID 54544 - need a special function to determine if a charge is a reversal
BOOL CEOBDlg::IsReversal(const EOBLineItemInfo *pCharge, const EOBClaimInfo *pClaim)
{
	if(!pCharge || !pClaim){
		// no charge or claim, so its not a reversal
		return FALSE;
	}

	// is this a reversed claim?
	if(!pClaim->bIsReversedClaim){
		return FALSE;
	}

	// if the charge amount is below zero, this is def a reversal
	if(pCharge->cyLineItemChargeAmt < g_ccyZero){
		return TRUE;
	}

	// if for some reason the charge amount is zero, it is still possible to have negative payments or negative adjustments
	if(pCharge->cyLineItemChargeAmt == g_ccyZero){
		// do we have a negative payment?
		if(pCharge->cyLineItemPaymentAmt < g_ccyZero){
			return TRUE;
		}

		// do we have at least one negative adjustment?
		for(int i=0; i<pCharge->arypEOBAdjustmentInfo.GetSize(); i++) {
			EOBAdjustmentInfo *pAdj = (EOBAdjustmentInfo*)pCharge->arypEOBAdjustmentInfo.GetAt(i);

			//remember bPatResp isn't posted as a real adjustment
			// (j.jones 2016-04-22 16:28) - NX-100161 - Decided to check for negatives here
			// such that if the EOB says it's a reversal, $0.00 charge, no reversed payment,
			// and a negative adjustment, sure, why not, it's a reversal.
			if(pAdj && !pAdj->bPatResp && pAdj->cyAdjustmentAmt != g_ccyZero) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

// (j.jones 2012-04-19 15:38) - PLID 35306 - Returns TRUE if the claim is a reversal,
// the and the payment/adjustment is negative and we could not match a payment/adjustment to void.
BOOL CEOBDlg::IsUnpostableReversalCharge(const EOBLineItemInfo *pCharge, const EOBClaimInfo *pClaim)
{
	if(pClaim == NULL || pCharge == NULL) {
		//was not a charge generated by the EOB
		return FALSE;
	}

	// (j.dinatale 2013-01-10 09:48) - PLID 54544 - new handy dandy function to determine if this is a reversal
	//is the claim a reversal?
	if(IsReversal(pCharge, pClaim)) {
		//this is a reversed charge
		
		//if there is a reversed payment, make sure we found it in Practice
		if(pCharge->cyLineItemPaymentAmt < COleCurrency(0,0) && pCharge->nReversedPaymentID == -1) {
			//there is a reversed payment, but we could not find it
			return TRUE;
		}

		//make sure we found each adjustment to reverse (if any)
		for(int i=0; i<pCharge->arypEOBAdjustmentInfo.GetSize(); i++) {
			EOBAdjustmentInfo *pAdj = (EOBAdjustmentInfo*)pCharge->arypEOBAdjustmentInfo.GetAt(i);

			//remember bPatResp isn't posted as a real adjustment
			// (j.jones 2016-04-22 16:28) - NX-100161 - now just check for nonzero adjustments
			if(pAdj != NULL && !pAdj->bPatResp && pAdj->cyAdjustmentAmt != COleCurrency(0,0) && pAdj->nReversedAdjustmentID == -1) {
				//there is a reversed adjustment, but we could not find it - is that ok?
				if(!pAdj->bMissingReversedAdjustmentOK) {
					//we can't reverse this adjustment
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

// (j.jones 2012-04-19 17:57) - PLID 35306 - Finds the given reversed RowID's sibling,
// and returns the value of IsUnpostableReversalCharge for that sibling.
BOOL CEOBDlg::HasUnpostableReversalSibling(NXDATALIST2Lib::IRowSettingsPtr pSiblingRow, long nSiblingChargeID, const EOBClaimInfo *pSiblingClaim, const EOBLineItemInfo *pSiblingCharge)
{
	//throw exceptions to the caller

	//we can't search for a sibling if the caller isn't matched to a valid charge
	if(nSiblingChargeID == -1 || pSiblingCharge == NULL || pSiblingClaim == NULL) {
		return TRUE;
	}

	//this is a US-only function
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType == ertOhip) {
		return TRUE;
	}
	// (s.tullis 2016-04-15 16:00) - NX-100211
	NXDATALIST2Lib::IRowSettingsPtr pRow = FindReversedSiblingRow(pSiblingRow, nSiblingChargeID, pSiblingClaim, pSiblingCharge);
	if(pRow) {

		EOBClaimInfo *pClaim = NULL;
		_variant_t var = pRow->GetValue(COLUMN_CLAIM_PTR);
		if(var.vt == VT_I4) {
			pClaim = (EOBClaimInfo*)VarLong(var);
		}

		EOBLineItemInfo *pCharge = NULL;				
		var = pRow->GetValue(COLUMN_CHARGE_PTR);
		if(var.vt == VT_I4) {
			pCharge = (EOBLineItemInfo*)VarLong(var);
		}

		return IsUnpostableReversalCharge(pCharge, pClaim);
	}

	return FALSE;
}

// (j.jones 2012-04-20 16:40) - PLID 49846 - CalcReversedPaymentAndAdjustmentIDs will fill the
// nReversedPaymentID and nReversedAdjustmentID for reversed line items, if any.
void CEOBDlg::CalcReversedPaymentAndAdjustmentIDs(IN OUT EOBClaimInfo *pClaim, IN OUT EOBLineItemInfo *pCharge)
{
	//throw exceptions to the caller

	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType != ertAnsi) {
		//canadian remits should never have had this function called, they use CalcReversedPaymentIDs() instead
		return;
	}

	if(pClaim == NULL || pCharge == NULL) {
		//can't do anything without this information
		//it's possible these are legitimately null sometimes,
		//though this function shouldn't have been called if so
		return;
	}

	// (j.dinatale 2013-01-10 09:48) - PLID 54544 - new handy dandy function to determine if this is a reversal
	if(!IsReversal(pCharge, pClaim)){
		return;
	}

	if(pCharge->nChargeID == -1) {
		//can't calculate the reversed information without a charge ID
		return;
	}
	
	if(pCharge->nInsuredPartyID == -1) {
		//can't calculate the reversed information without an insured party ID
		return;
	}

	//Now we know we're on a reversed charge, and we have the information we need
	//to try and find the posted payments and adjustments to reverse,
	//so populate the payment ID to reverse and the adjustment ID to reverse.

	//track the BatchPaymentID of the payment to reverse
	long nReversedBatchPaymentID = -1;

	//if there is a reversed payment, its amount will be negative
	if(pCharge->cyLineItemPaymentAmt < COleCurrency(0,0)) {
		//Find the payment ID by matching on any payment by this insurance company for the
		//reversed payment amount. It has to be applied, but the apply amount does not need
		//to match the reversal amount (though it really should). E-Remittance would have
		//only made one payment for the amount originally intended to be applied.
		//The ORDER BY is fairly meaningless as we don't expect to actually find multiple results.
		_RecordsetPtr rsReversedPay = CreateParamRecordset("SELECT TOP 1 PaymentsT.ID, PaymentsT.BatchPaymentID "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND AppliesT.DestID = {INT} AND PaymentsT.InsuredPartyID = {INT} AND LineItemT.Amount = {OLECURRENCY} "
			"ORDER BY LineItemT.InputDate DESC",
			pCharge->nChargeID, pCharge->nInsuredPartyID, -pCharge->cyLineItemPaymentAmt);
		if(rsReversedPay->eof) {
			//we could not find a payment to reverse

			//this should have already initialized to -1, but re-affirm it
			pCharge->nReversedPaymentID = -1;
			//log the failure
			Log("CalcInternalIDs: Failed to find a reversed payment for charge ID %li, insured party ID %li, amount of %s.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pCharge->cyLineItemPaymentAmt));
		}
		else {
			//we successfully found a payment to reverse
			long nReversedPaymentID = VarLong(rsReversedPay->Fields->Item["ID"]->Value);

			//do not use this payment ID if we are already using it elsewhere,
			//which should be impossible unless they foolishly listed the same takeback twice
			//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
			if(IsReversedPaymentInUse(nReversedPaymentID, false)) {
				pCharge->nReversedPaymentID = -1;
				//log the failure
				Log("CalcInternalIDs: Found reversed payment ID %li for charge ID %li, insured party ID %li, amount of %s. Cannot use this payment because another takeback is already using it.", nReversedPaymentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pCharge->cyLineItemPaymentAmt));
			}
			else {
				//assign the payment ID
				pCharge->nReversedPaymentID = nReversedPaymentID;

				//track the BatchPaymentID of the payment to reverse (could be -1, but not likely)
				nReversedBatchPaymentID = VarLong(rsReversedPay->Fields->Item["ID"]->Value, -1);

				//log the success
				Log("CalcInternalIDs: Successfully found reversed payment ID %li for charge ID %li, insured party ID %li, amount of %s.", nReversedPaymentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pCharge->cyLineItemPaymentAmt));
			}
		}
		rsReversedPay->Close();
	}

	//even if we didn't find a reversed payment ID, still look up reversed adjustment IDs, 
	//it may help us in the future when debugging

	//find all adjustments to reverse
	for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {						
		EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo.GetAt(nAdjIndex);
		
		//if there is a reversed adjustment, its amount will be negative
		// (j.jones 2016-04-22 16:28) - NX-100161 - no longer true, this just needs to be nonzero
		if(!pAdj->bPatResp && pAdj->cyAdjustmentAmt != COleCurrency(0,0)) {
			//Find the adjustment ID by matching on any adjustment by this insurance company for the
			//reversed adjustment amount. It has to be applied, but the apply amount does not need
			//to match the reversal amount (though it really should). E-Remittance would have
			//only made one adjustment for the amount originally intended to be applied.

			//On takeback remit files, adjustments typically have the group code CR for corrections & reversals,
			//which means we cannot match by group code, only reason code.

			//The ORDER BY will sort our different ways of finding adjustments in order of accuracy,
			//such that if we do find multiple potential matches (unlikely), we pick the most likely
			//candidate. The priority is to match by batch payment ID and:
			//1. match by amount and reason code
			//2. match by amount
			//3. match by reason code
			//Then match all three again without a batch payment ID.

			_RecordsetPtr rsReversedAdj = CreateParamRecordset("SET NOCOUNT ON "
				"DECLARE @batchPaymentID INT "
				"DECLARE @adjustmentAmount MONEY "
				"DECLARE @reasonCode nvarchar(255) "
				"SET @batchPaymentID = {INT} "
				"SET @adjustmentAmount = {OLECURRENCY} "
				"SET @reasonCode = {STRING} "
				"SET NOCOUNT OFF "
				""
				"SELECT TOP 1 PaymentsT.ID "
				"FROM LineItemT "
				"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND AppliesT.DestID = {INT} AND PaymentsT.InsuredPartyID = {INT} "
				"AND ("
				"	LineItemT.Amount = @adjustmentAmount "
				"	OR AdjustmentReasonCodesT.Code = @reasonCode "
				"	OR PaymentsT.BatchPaymentID = @batchPaymentID "
				") "
				"ORDER BY "
				"CASE WHEN PaymentsT.BatchPaymentID = @batchPaymentID AND LineItemT.Amount = @adjustmentAmount AND AdjustmentReasonCodesT.Code = @reasonCode THEN 1 ELSE 0 END DESC, "
				"CASE WHEN PaymentsT.BatchPaymentID = @batchPaymentID AND LineItemT.Amount = @adjustmentAmount THEN 1 ELSE 0 END DESC, "
				"CASE WHEN PaymentsT.BatchPaymentID = @batchPaymentID AND AdjustmentReasonCodesT.Code = @reasonCode THEN 1 ELSE 0 END DESC, "
				"CASE WHEN LineItemT.Amount = @adjustmentAmount AND AdjustmentReasonCodesT.Code = @reasonCode THEN 1 ELSE 0 END DESC, "
				"CASE WHEN LineItemT.Amount = @adjustmentAmount THEN 1 ELSE 0 END DESC, "
				"CASE WHEN AdjustmentReasonCodesT.Code = @reasonCode THEN 1 ELSE 0 END DESC, "
				"LineItemT.InputDate DESC",
				nReversedBatchPaymentID, -pAdj->cyAdjustmentAmt, pAdj->strReasonCode,
				pCharge->nChargeID, pCharge->nInsuredPartyID);
			if(rsReversedAdj->eof) {
				//we could not find an adjustment to reverse, but that might be ok

				//these should have already initialized, but re-affirm the values,
				//and let the following code decide if it's ok
				pAdj->nReversedAdjustmentID = -1;
				pAdj->bMissingReversedAdjustmentOK = FALSE;

				//if the reversed payment was $0.00, then the user probably didn't post the original
				//adjustment, and thus it's ok to continue, this new re-posting will just post normally
				if(pCharge->cyLineItemPaymentAmt == COleCurrency(0,0)) {
					pAdj->bMissingReversedAdjustmentOK = TRUE;
					Log("CalcInternalIDs: Failed to find a reversed adjustment for charge ID %li, insured party ID %li, amount of %s, reason code %s. Posting is permitted due to having no reversed payment on this charge.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
				}

				//If still not OK, check to see if the adjustment code is currently ignored,
				//which means it was probably skipped in the past.
				//However, this setup includes group and reason code, and we only get the reason code of the reversed adjustment.
				//We can still match by it for two reasons:
				//	1. Odds are very high an ignored reason code is always meant to be ignored.
				//	2. Currently we permit all missing adjustments if we find a payment to reverse anyways.
				if(!pAdj->bMissingReversedAdjustmentOK) {
					if(ReturnsRecordsParam("SELECT TOP 1 ERemitIgnoredAdjCodesT.ID "
						"FROM ERemitIgnoredAdjCodesT "
						"INNER JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON ERemitIgnoredAdjCodesT.GroupCodeID = AdjustmentGroupCodesT.ID "
						"INNER JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON ERemitIgnoredAdjCodesT.ReasonCodeID = AdjustmentReasonCodesT.ID "
						"WHERE AdjustmentReasonCodesT.Code = {STRING}", pAdj->strReasonCode)) {

						//This is a skipped adjustment code, which is ok. Remember, we technically only matched by reason code, not group code.
						pAdj->bMissingReversedAdjustmentOK = TRUE;
						Log("CalcInternalIDs: Failed to find a reversed adjustment for charge ID %li, insured party ID %li, amount of %s, reason code %s. Posting is permitted due to the group & reason code being ignored.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
					}
				}

				//it is possible the adjustment could have been deleted, but we are not going to support matching that way

				//finally, allow skipping adjustments if we have a payment
				if(pCharge->cyLineItemPaymentAmt < COleCurrency(0,0) && pCharge->nReversedPaymentID != -1) {
					//This should be the catch all case that allows all reversed payments to post even if
					//we don't find the adjustments. Remove this in the future if we choose to disallow
					//posting takebacks if we can't find the adjustments.
					pAdj->bMissingReversedAdjustmentOK = TRUE;
					Log("CalcInternalIDs: Failed to find a reversed adjustment for charge ID %li, insured party ID %li, amount of %s, reason code %s. Posting is permitted due to successfully finding the reversed payment.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
				}

				//if we didn't declare this to be ok, log the failure
				if(!pAdj->bMissingReversedAdjustmentOK) {
					Log("CalcInternalIDs: Failed to find a reversed adjustment for charge ID %li, insured party ID %li, amount of %s, reason code %s.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
				}
			}
			else {
				//we successfully found an adjustment to reverse
				long nReversedAdjustmentID = VarLong(rsReversedAdj->Fields->Item["ID"]->Value);
				
				//do not use this adjustment ID if we are already using it elsewhere,
				//which should be impossible unless they foolishly listed the same takeback twice
				if(IsReversedAdjustmentInUse(nReversedAdjustmentID)) {
					pAdj->nReversedAdjustmentID = -1;
					//log the failure
					Log("CalcInternalIDs: Found reversed adjustment ID %li for charge ID %li, insured party ID %li, amount of %s, reason code %s. Cannot use this adjustment because another takeback is already using it.", nReversedAdjustmentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
				}
				else {
					//assign the adjustment ID
					pAdj->nReversedAdjustmentID = nReversedAdjustmentID;
					//log the success
					Log("CalcInternalIDs: Successfully found reversed adjustment ID %li for charge ID %li, insured party ID %li, amount of %s, reason code %s.", nReversedAdjustmentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(-pAdj->cyAdjustmentAmt), pAdj->strReasonCode);
				}
			}
			rsReversedAdj->Close();
		}
	}
}

// (j.jones 2012-04-24 08:47) - PLID 49846 - searches all currently loaded EOBs to see if a reversed payment ID already exists
//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
BOOL CEOBDlg::IsReversedPaymentInUse(long nReversedPaymentID, bool bCheckAlberta)
{
	//this shouldn't be called with -1, so ASSERT to yell at the developer,
	//then return TRUE to ensure that the -1 ID can't be used
	if(nReversedPaymentID == -1) {
		ASSERT(FALSE);
		return TRUE;
	}

	if (bCheckAlberta) {
		//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
		foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_AlbertaParser.m_pChargeList)) {
			if (pCharge->nReversedPaymentID == nReversedPaymentID) {
				return TRUE;
			}
		}
		return FALSE;
	}
	else {
	for(int a=0; a<m_EOBParser.m_arypEOBInfo.GetSize(); a++) {
		EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];

		for(int b=0;b<pEOB->arypEOBClaimInfo.GetSize();b++) {
			EOBClaimInfo *pClaim = pEOB->arypEOBClaimInfo[b];

			//only reversed claims have these IDs in use
			if(!pClaim->bIsReversedClaim) {
				continue;
			}

			for(int c=0;c<pClaim->arypEOBLineItemInfo.GetSize();c++) {

				EOBLineItemInfo *pCharge = pClaim->arypEOBLineItemInfo[c];

				if(pCharge->nReversedPaymentID == nReversedPaymentID) {
					//found it, return true
					return TRUE;
				}
			}
		}
	}

	//if we get here, the given payment ID is not in use
	return FALSE;
}
}

// (j.jones 2012-04-24 08:47) - PLID 49846 - searches all currently loaded EOBs to see if a reversed adjustment ID already exists
BOOL CEOBDlg::IsReversedAdjustmentInUse(long nReversedAdjustmentID)
{
	//this shouldn't be called with -1, so ASSERT to yell at the developer,
	//then return TRUE to ensure that the -1 ID can't be used
	if(nReversedAdjustmentID == -1) {
		ASSERT(FALSE);
		return TRUE;
	}

	for(int a=0; a<m_EOBParser.m_arypEOBInfo.GetSize(); a++) {
		EOBInfo *pEOB = m_EOBParser.m_arypEOBInfo[a];

		for(int b=0;b<pEOB->arypEOBClaimInfo.GetSize();b++) {
			EOBClaimInfo *pClaim = pEOB->arypEOBClaimInfo[b];

			//only reversed claims have these IDs in use
			if(!pClaim->bIsReversedClaim) {
				continue;
			}

			for(int c=0;c<pClaim->arypEOBLineItemInfo.GetSize();c++) {

				EOBLineItemInfo *pCharge = pClaim->arypEOBLineItemInfo[c];
				for(int d=0; d<pCharge->arypEOBAdjustmentInfo.GetSize(); d++) {
					EOBAdjustmentInfo *pAdj = (EOBAdjustmentInfo*)pCharge->arypEOBAdjustmentInfo.GetAt(d);
					if(pAdj->nReversedAdjustmentID == nReversedAdjustmentID) {
						//found it, return true
						return TRUE;
					}
				}
			}
		}
	}

	//if we get here, the given adjustment ID is not in use
	return FALSE;
}

// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
// to the given insurance resp. that we're about to void
//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
COleCurrency CEOBDlg::CalcAmountToBeReversed(long nChargeID, long nInsuredPartyID, const EOBLineItemInfo *pCharge, AlbertaAssessments::ChargeInfoPtr pAlbertaCharge)
{
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
	if (m_ERemitType == ertOhip || (pCharge == NULL && pAlbertaCharge == NULL)) {
		return COleCurrency(0,0);
	}

	if(nChargeID == -1 || nInsuredPartyID == -1) {
		return COleCurrency(0,0);
	}

	//if the charge has a reversed payment and/or reversed adjustments,
	//find out the total amount applied to the charge
	
	CArray<int, int> aryReversedIDs;

	if (pCharge == NULL) {
		ASSERT(pAlbertaCharge != NULL);
		if (pAlbertaCharge->nReversedPaymentID != -1) {
			aryReversedIDs.Add(pAlbertaCharge->nReversedPaymentID);
		}
	}
	else {
	if(pCharge->nReversedPaymentID != -1) {
		aryReversedIDs.Add(pCharge->nReversedPaymentID);
	}

	for(int nAdjIndex=0; nAdjIndex<pCharge->arypEOBAdjustmentInfo.GetSize(); nAdjIndex++) {
		EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[nAdjIndex];
		if(!pAdj->bPatResp && pAdj->nReversedAdjustmentID != -1) {
			aryReversedIDs.Add(pAdj->nReversedAdjustmentID);
		}
	}
	}

	if(aryReversedIDs.GetSize() > 0) {
		//now calculate the total amount applied
		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(AppliesT.Amount) AS TotalApplied "
			"FROM AppliesT "
			"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.ID IN ({INTARRAY}) "
			"AND AppliesT.DestID = {INT} "
			"AND AppliesT.RespID IN (SELECT ID FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID = {INT})",
			aryReversedIDs, nChargeID, nChargeID, nInsuredPartyID);
		if(!rs->eof) {
			return VarCurrency(rs->Fields->Item["TotalApplied"]->Value, COleCurrency(0,0));
		}
		rs->Close();
	}

	//if we're still here, nothing is being reversed
	return COleCurrency(0,0);
}

// (j.jones 2012-05-01 15:36) - PLID 47477 - added function to filter by just the Processed As flag and HCFA group ID
EInsuredPartyInfo CEOBDlg::CalcInsuredPartyIDByProcessedAs(long nChargeID, long nPatientID, long nProcessedAs, long nHCFAGroupID)
{
	EInsuredPartyInfo eBlankInsuredInfo;
	eBlankInsuredInfo.nInsuredPartyID = -1;
	eBlankInsuredInfo.nCategoryPlacement = -1;

	EInsuredPartyInfo eFoundInsuredParty = eBlankInsuredInfo;

	if(nProcessedAs == -1) {
		//shouldn't have called this function, it won't help you

		//assert, because the developer should never call this if -1
		ASSERT(FALSE);

		//return an empty result, the caller should handle that
		return eBlankInsuredInfo;
	}

	CSqlFragment sqlHCFAGroupFilter("");
	//unlike the calling function, we will not try to filter by HCFA group if the ID is -1
	if(nHCFAGroupID != -1) {
		sqlHCFAGroupFilter = CSqlFragment("AND InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = {INT})", nHCFAGroupID);
	}

	CSqlFragment sqlPriorityWhere, sqlOrderBy;
	if(nProcessedAs == 1) {
		//if primary, filter on primary or SubmitAsPrimary, order by Priority, inactive last
		sqlPriorityWhere = CSqlFragment("AND (RespTypeT.Priority = 1 OR InsuredPartyT.SubmitAsPrimary = 1)");
		sqlOrderBy = CSqlFragment("(CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END) ASC, "
			"RespTypeT.Priority ASC");
	}
	else if(nProcessedAs == 2) {
		//if secondary, filter on non-primary, order by Priority 2 as the first result, order by Priority, inactive last
		sqlPriorityWhere = CSqlFragment("AND RespTypeT.Priority <> 1 AND InsuredPartyT.SubmitAsPrimary <> 1");
		sqlOrderBy = CSqlFragment("(CASE WHEN RespTypeID = 2 THEN 0 ELSE 1 END) ASC, "
			"(CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END) ASC, "
			"RespTypeT.Priority ASC");
	}
	else if(nProcessedAs == 3) {
		//if tertiary, filter on non-primary, order by Priority 3 as the first result, order by Priority, inactive last
		sqlPriorityWhere = CSqlFragment("AND RespTypeT.Priority <> 1 AND InsuredPartyT.SubmitAsPrimary <> 1");
		sqlOrderBy = CSqlFragment("(CASE WHEN RespTypeID = 3 THEN 0 ELSE 1 END) ASC, "
			"(CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END) ASC, "
			"RespTypeT.Priority ASC");
	}

	//In all cases, filter on medical resp. only., and filter on the HCFA Group.
	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, RespTypeID, CategoryPlacement "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE PatientID = {INT} AND RespTypeT.CategoryType = 1 "
		"{SQL} "
		"{SQL} "
		"ORDER BY {SQL}",
		nPatientID, sqlPriorityWhere, sqlHCFAGroupFilter, sqlOrderBy);
	if(!rs->eof) {
		if(rs->GetRecordCount() == 1) {
			//we found one match, use this insured party
			eFoundInsuredParty.nInsuredPartyID = AdoFldLong(rs, "PersonID");
			eFoundInsuredParty.nCategoryPlacement = AdoFldLong(rs, "CategoryPlacement", -1);

			//make a different log if no HCFA Group was given
			if(nHCFAGroupID == -1) {
				Log("CalcInsuredPartyIDByProcessedAs: Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding one match for this patient using the Processed As flag (%li).", eFoundInsuredParty.nInsuredPartyID, nPatientID, nProcessedAs);
			}
			else {
				//most likely scenario
				Log("CalcInsuredPartyIDByProcessedAs: Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding one match in the specified HCFA Group of (%li) using the Processed As flag (%li).", eFoundInsuredParty.nInsuredPartyID, nPatientID, nHCFAGroupID, nProcessedAs);
			}
			return eFoundInsuredParty;
		}
		else {
			//This means that, as expected, "Processed As" isn't as reliable as we would like.
			//CalcInsuredPartyIDByResp will try to filter by responsibility, but with precedence towards the first ordered result.
			eFoundInsuredParty = CalcInsuredPartyIDByResp(rs, nChargeID, nPatientID);

			//make a different log if no HCFA Group was given
			if(nHCFAGroupID == -1) {
				Log("CalcInsuredPartyIDByProcessedAs: Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding multiple matches for this patient using the Processed As flag (%li), and further filtering by responsibility.", eFoundInsuredParty.nInsuredPartyID, nPatientID, nProcessedAs);
			}
			else {
				Log("CalcInsuredPartyIDByProcessedAs: Calculated Insured Party ID (%li) for Patient ID (%li) by "
					"finding multiple matches in the specified HCFA Group of (%li) using the Processed As flag (%li), and further filtering by responsibility.", eFoundInsuredParty.nInsuredPartyID, nPatientID, nHCFAGroupID, nProcessedAs);
			}
			return eFoundInsuredParty;
		}
	}
	rs->Close();

	//if we get here, we found nothing
	return eBlankInsuredInfo;
}

// (j.jones 2012-05-02 10:09) - PLID 50138 - returns true if m_InsuranceCombo has no rows, or it has one
// row and that row is one of our built-in "show all" / "show filtered" rows
BOOL CEOBDlg::IsInsuranceComboEmpty()
{
	//throw exceptions to the caller

	long nRowCount = m_InsuranceCombo->GetRowCount();
	if(nRowCount == 0) {
		//the combo is completely empty
		return true;
	}
	else if(nRowCount == 1) {
		//The combo only has one row. It is probably a built-in row,
		//but confirm the sentinel value exists first.
		long nID = VarLong(m_InsuranceCombo->GetValue(0, iccID), -1);
		if(nID == SHOW_ALL_INSCOS || nID == SHOW_FILTERED_INSCOS) {
			//as expected, the single row is not real data, but instead
			//one of our built-in rows
			return true;
		}
		else {
			//This is real data, and we did not add a built-in row.
			//I believe this should be impossible, so we'll assert.

			// (j.jones 2012-10-04 14:38) - this is possible if the insurance company list
			// really does only have one company, which is common in Canada
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			ASSERT(m_ERemitType == ertOhip || m_ERemitType == ertAlberta);

			//Remove the assert if this really is possible, and replace with
			//notes explaining how this is valid... because I'm pretty sure it's not.

			return false;
		}
	}
	else {
		//the combo has more than 1 row, which means even if it
		//has a built-in row, there is at least 1 row of content
		return false;
	}
}

// (j.jones 2012-08-23 12:44) - PLID 42438 - if any claims now have a $0.00 balance,
// this function will unbatch them if they are still batched (and not warn about it)
void CEOBDlg::CheckUnbatchClaims()
{
	try {
		// (s.tullis 2016-04-15 16:00) - NX-100211
		//get a list of all unique bill IDs so we don't run unnecessary recordsets
		CArray<long, long> aryBillIDs;
		for (NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr = m_EOBList->GetFirstRow();pLoopRowPtr != nullptr ; pLoopRowPtr = pLoopRowPtr->GetNextRow()) {
			long nBillID = VarLong(pLoopRowPtr->GetValue(COLUMN_BILL_ID), -1);
			if(nBillID != -1) {
				BOOL bFound = FALSE;
				for(int j=0;j<aryBillIDs.GetSize() && !bFound; j++) {
					if(aryBillIDs.GetAt(j) == nBillID) {
						//skip this duplicate bill
						bFound = TRUE;
					}
				}
				if(!bFound) {
					aryBillIDs.Add(nBillID);
				}
			}
		}

		//call CheckUnbatchClaim on each bill, and tell it not to warn
		for(int i=0;i<aryBillIDs.GetSize();i++) {
			CheckUnbatchClaim(aryBillIDs.GetAt(i), TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, November 01, 2012) - PLID 49943 - function to update the bill reference number if strICN is not empty and we have a bill.
void CEOBDlg::UpdateBillRefNumber(long nBillID, EOBClaimInfo* pClaim) 
{
	//If, for whatever reason, this is zero then we should not execute this function at all.
	if (m_nEOBUpdateBillOriginaRefNo == 0) {
		return;
	};


	CString strICN = pClaim->strOriginalRefNum;
	CString strPrevOrigRefNo = "";
	long nPatientID = pClaim->nPatientID;
	
	//Assuming all other things passed, get the current refno and update. 
	// (b.spivey, November 08, 2012) - PLID 49943 - Lack of errors let me miss this statement with extra unnecessary parameters. 
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT OriginalRefNo FROM BillsT WHERE BillsT.ID = {INT} ", 
		nBillID);

	if (!prs->eof) {
		strPrevOrigRefNo = AdoFldString(prs->GetFields(), "OriginalRefNo", "");
	}


	//Don't update if this is blank, also don't update if the icns match.
	if (strICN.IsEmpty() || strICN.Compare(strPrevOrigRefNo) == 0) {
		return;
	}

	//If this is update when blank only, and the previous refNo is not blank, then we need to bail out.
	if (m_nEOBUpdateBillOriginaRefNo == 1 && !strPrevOrigRefNo.IsEmpty()) {
		return;
	}
	
	//Throw an exception if greater than 50. CPL07 has a max length of 50, so if this ever gets
	//  thrown, that means that either the specs changed or there is something wrong with this file.
	if (strICN.GetLength() > 50) {
		ThrowNxException("CEOBDlg : Attempted to update an OriginalRefNo with a value length greater than 50! "); 
		return;
	}

	//assuming all this passed, update. 	
	// (b.spivey, November 08, 2012) - PLID 49943 - SQL is awfully case insensitive today. 
	ExecuteParamSql("UPDATE BillsT "
		"SET OriginalRefNo = {STRING} "
		"WHERE BillsT.ID = {INT} ",
		strICN, nBillID);

	//audit
	long nAuditID = BeginNewAuditEvent();
	if(nAuditID != -1){
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiBillOriginalRefNoByERemitPosting, nBillID, strPrevOrigRefNo, strICN, aepMedium, aetChanged);
	}
}

// (j.dinatale 2012-11-06 11:00) - PLID 50792 - need to unbatch MA18 claims
// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to unbatch N89 claims also
// (j.jones 2013-07-18 11:26) - PLID 57616 - renamed to be generic
void CEOBDlg::UnbatchCrossoverClaims()
{
	// (s.dhole 2013-04-24 17:40) - PLID 56122
	// (j.jones 2013-07-19 10:38) - PLID 57616 - This ConfigRT name is misleading, it actually just means that
	// if we do unbatch a crossed over claim (either by MA18 or the manual setup), claim history will only include
	// batched charges. If false, then claim history includes all charges.
	bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

	std::set<long> setSkipBillIDs;	// keep track of what we know we don't have to check again
	// (s.tullis 2016-04-15 16:00) - NX-100211
	for(NXDATALIST2Lib::IRowSettingsPtr pLoopRowPtr = m_EOBList->GetFirstRow(); pLoopRowPtr != nullptr; pLoopRowPtr = pLoopRowPtr->GetNextRow()) {
		EOBClaimInfo* pClaimInfo = (EOBClaimInfo*)VarLong(pLoopRowPtr->GetValue(COLUMN_CLAIM_PTR), NULL);
		if(pClaimInfo == NULL) {
			//shouldn't be possible
			continue;
		}
		long nBillID = pClaimInfo->nBillID;
		if(nBillID != -1 && !setSkipBillIDs.count(nBillID)) {
			//if the MA18/N89 pref. is enabled, and this claim has those reason codes,
			//auto-unbatch the crossed-over claims
			if(m_bUnbatchMA18orN89 && pClaimInfo->bHasMA18orN89) {			
				setSkipBillIDs.insert(nBillID);

				// (j.jones 2013-07-18 13:00) - PLID 57616 - moved all this code to a modular function, we pass in
				// the insured party ID who was the OthrInsuredParty on the bill (prior to swapping, which would have
				// already occurred by now) as the insurer who the claim crossed over into
								
				long nOthrInsuredPartyID = pClaimInfo->nBillOthrInsuredPartyID;
				// (j.jones 2014-06-24 11:41) - PLID 60349 - It's highly unlikely that the other insured party ID
				// will be -1, but it is possible if the user messed with the bill after submitting the claim.
				// If this happens, use the insured party we would shift to next.
				// Remember we might not have actually shifted.
				if (nOthrInsuredPartyID == -1) {
					nOthrInsuredPartyID = GetNextInsuredPartyIDByPriority(pClaimInfo->nPatientID, pClaimInfo->nBillInsuredPartyID);
					//it is still possible this ID may be -1 now
				}

				UnbatchCrossoverClaim(pClaimInfo->nPatientID, nBillID, nOthrInsuredPartyID, pClaimInfo->dtCheckDate,
					bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByEOBProcessing, "Batched", "Unbatched due to MA18 or N89");
			}
			else {
				// (j.jones 2013-07-18 11:24) - PLID 57616 - We now have a setup to mimic the MA18/N89 behavior
				// if the payer is primary and is configured to always not batch to secondary.
				long nInsuredPartyID = -1;
				for(int i=0; i<pClaimInfo->arypEOBLineItemInfo.GetSize() && nInsuredPartyID == -1; i++) {
					nInsuredPartyID = pClaimInfo->arypEOBLineItemInfo.GetAt(i)->nInsuredPartyID;
				}
				if(nInsuredPartyID != -1) {
					//Since posting has completed, this will assume that the bill's current insured party ID
					//is the "secondary" insured party we crossed over to, and the insured party who paid was primary.
					//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
					CheckUnbatchCrossoverClaim(pClaimInfo->nPatientID, nBillID, nInsuredPartyID, pClaimInfo->dtCheckDate,
						bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");

					//CheckUnbatchCrossoverClaim would return false if it didn't do anything,
					//but we can still track that we already checked this bill once because
					//the decision won't change the next time either.
					setSkipBillIDs.insert(nBillID);
				}
			}
		}
	}
}

// (b.spivey, July 03, 2013) - PLID 56825 - This function takes a charge pointer and creates a note that is the list of adjustment 
//		reasons, typically on a skipped, zeropay, nonzero adjustment line item. 
CString CEOBDlg::CreateBillingNoteFromAdjustmentReasons(long nPatientID, long nChargeID, EOBLineItemInfo *pCharge, bool bCreateOnCharge /* = false */)
{
	if(pCharge == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return "";
	}

	CString strNote = "";

	long nEOBAdjustInfoSize = pCharge->arypEOBAdjustmentInfo.GetSize(); 

	for (int i = 0; i < nEOBAdjustInfoSize; i++) {
		CString strReason = "";
		strReason = pCharge->arypEOBAdjustmentInfo.GetAt(i)->strReason;
		strNote += (strReason + (strNote.IsEmpty() ? "" : " \r\n")); 
	}

	//if our note is empty, leave
	if(strNote.IsEmpty()) {
		return "";
	}

	strNote = "Insurance states the following Adjustment Reason(s): \r\n" + strNote;
	
	// (b.spivey, July 03, 2013) - PLID 
	long nCategoryID = GetRemotePropertyInt("ERemitPostAdjustmentNoteWhenZeroPays_DefaultNoteCategory", -1, 0, "<None>", true);

	if(bCreateOnCharge) {
		CreateBillingNote(nPatientID, nChargeID, strNote, nCategoryID, FALSE);
	}

	return strNote; 
}


// (r.farnworth 2014-01-28 15:53) - PLID 52596 - Revert the adjustment amount
// (j.jones 2016-04-19 13:14) - NX-100161 - obsolete
//void CEOBDlg::OnRevertAdjustment()

//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files, code copied and modified from the other parsers
void CEOBDlg::ImportAlbertaFile()
{
	try {
		// (j.jones 2007-06-29 08:53) - PLID 23951 - fire a use of the license
		if (!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrUse)) {
			return;
		}

		if (m_EOBList->GetRowCount() > 0) {
			if (IDNO == MessageBox("You have already imported a remittance file. If you continue, no data will be posted.\n\n"
				"Do you wish to clear out the current data and import a new file?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
				return;
			}
			else {

				// (j.jones 2009-07-01 16:27) - PLID 34778 - clear and hide the overpayment label
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.SetWindowText("");
				m_nxstaticEstimatedOverpaymentUnderpaymentLabel.ShowWindow(SW_HIDE);

				m_EOBList->Clear();

				// (j.jones 2011-02-09 11:34) - PLID 42391 - hide the reversal column
				NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
				pReversalCol->PutStoredWidth(0);

				// (j.jones 2011-06-16 08:51) - PLID 39747 - the default insurance combo filter for when we aren't showing
				// specific insurance companies in the list, should just be "Archived = 0"
				m_strCurInsuranceComboFilter = DEFAULT_INSURANCE_FILTER;
				m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
				m_InsuranceCombo->Requery();
				SetDlgItemText(IDC_INSCO_NAME_LABEL, "<None Specified>");
				SetDlgItemText(IDC_PROVIDER_LABEL, "Provider:");
				SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL, FormatCurrencyForInterface(COleCurrency(0, 0), TRUE, TRUE));
				// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
				m_AlbertaParser.ClearAllEOBs();
			}
		}

		//since we pump messages during the processing, make sure that all buttons are disabled
		//during the processing. We re-enable them when the processing is complete.
		EnableButtons(FALSE);

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Importing file...");

		COleCurrency cyTotalPayments = COleCurrency(0, 0);

		BOOL bWarnMissingInsuredParties = FALSE;
		BOOL bWarnMissingBillIDs = FALSE;
		BOOL bHasUnpostableReversals = FALSE;
		// (b.spivey, January 17, 2012) - PLID 47594 - Initializing this to TRUE just ended up throwing the warning every single time.
		BOOL bWarnInvalidPayTotals = FALSE;

		// (j.jones 2008-12-19 08:46) - PLID 32519 - added ability to auto-open a file
		if (!m_strAutoOpenFilePath.IsEmpty() && DoesExist(m_strAutoOpenFilePath)) {
			//tell the parser to use this default file
			m_AlbertaParser.m_strFileName = m_strAutoOpenFilePath;
		}
		else {
			//ensure the parser has no default
			m_AlbertaParser.m_strFileName = "";
		}
		//clear our default file now
		m_strAutoOpenFilePath = "";

		// (j.jones 2012-05-25 13:59) - PLID 44367 - pass this dialog as the parent
		if (m_AlbertaParser.ParseFile(this)) {
			std::size_t nSize = m_AlbertaParser.m_pChargeList->size();
			//TES 9/30/2014 - PLID 62580 - Alberta only has one payment per file
			SetDlgItemText(IDC_PAYMENT_LABEL, "Total Payment:");
			

			// (j.jones 2010-04-09 12:00) - PLID 31309 - added date controls,
			// disable the date if they enabled it and fill with the first check date
			// (j.jones 2010-11-30 09:53) - PLID 39434 - check the DefaultEOBDate preference,
			// 0 - use EOB date, 1 - use today's date
			// we would have set to today's date when the dialog opened, so if the preference is
			// set to 1, do nothing here
			BOOL bUseEOBDate = (GetRemotePropertyInt("DefaultEOBDate", 0, 0, "<None>", true) == 0);
			if (bUseEOBDate) {
				m_checkEnablePayDate.SetCheck(FALSE);
				m_checkEnableAdjDate.SetCheck(FALSE);
				GetDlgItem(IDC_EOB_PAY_DATE)->EnableWindow(FALSE);
				GetDlgItem(IDC_EOB_ADJ_DATE)->EnableWindow(FALSE);
				COleDateTime dtPayDate = COleDateTime::GetCurrentTime();
				if (m_AlbertaParser.m_dtFirstExpectedPaymentDate.GetStatus() != COleDateTime::invalid) {
					dtPayDate = m_AlbertaParser.m_dtFirstExpectedPaymentDate;
				}
				m_dtPayDate.SetValue(dtPayDate);
				m_dtAdjDate.SetValue(dtPayDate);
			}

			// (j.jones 2011-04-04 15:50) - PLID 42571 - calculate the insured party IDs
			// prior to calling CheckAdjustmentCodesToIgnore
			foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_AlbertaParser.m_pChargeList)) {
				m_progress.SetPos(0);

				CString str;
				if (nSize == 1)
					str = "Searching financial data...";
				else
					str.Format("Searching financial data...");
				m_progressStatus.SetWindowText(str);

				CalcReversedPaymentIDs(pCharge);

				if (nSize == 1)
					str = "Populating list...";
				else
					str.Format("Populating list...");
				m_progressStatus.SetWindowText(str);

				//now propagate the list

				if (pCharge->strInsuranceCoName != "")
					SetDlgItemText(IDC_INSCO_NAME_LABEL, pCharge->strInsuranceCoName);
				else
					SetDlgItemText(IDC_INSCO_NAME_LABEL, "<None Specified>");

				if (pCharge->strProviderFullName != "") {
					SetDlgItemText(IDC_PROVIDER_LABEL, "Provider:");
					SetDlgItemText(IDC_PROVIDER_NAME_LABEL, pCharge->strProviderFullName);
				}
				else {
					SetDlgItemText(IDC_PROVIDER_NAME_LABEL, "<None Specified>");
				}

				cyTotalPayments += (pCharge->eAssessmentResultAction_Additional == araaReversal ? -1 : 1)*pCharge->cyFinalAssessedAmount;
				SetDlgItemText(IDC_TOTAL_PAYMENTS_LABEL, FormatCurrencyForInterface(cyTotalPayments, TRUE, TRUE));
				// (s.tullis 2016-04-15 16:00) - NX-100211
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

				pRow->PutValue(COLUMN_EOB_INDEX, 0);
				pRow->PutValue(COLUMN_PATIENT_ID, (long)pCharge->nPatientID);
				// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
				pCharge->nUserDefinedID = GetExistingPatientUserDefinedID(pCharge->nPatientID);
				pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pCharge->nUserDefinedID);

				pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pCharge->nPatientID)));
				pRow->PutValue(COLUMN_BILL_ID, (long)pCharge->nBillID);
				pRow->PutValue(COLUMN_CHARGE_ID, (long)pCharge->nChargeID);

				// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
				_variant_t varBirthDate = g_cvarNull;
				COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pCharge->nPatientID);
				if (dtBirthDate.GetStatus() != COleDateTime::invalid) {
					varBirthDate.vt = VT_DATE;
					varBirthDate.date = dtBirthDate;
				}
				pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);
				pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, ""); //this isn't an OHIP import
				pRow->PutValue(COLUMN_OHIP_VERSION_CODE, "");

				pRow->PutValue(COLUMN_DUPLICATE, (long)0);

				// (j.jones 2011-02-09 11:24) - PLID 42391 - track if this claim is a reversal
				pRow->PutValue(COLUMN_REVERSAL, (pCharge->eAssessmentResultAction_Additional == araaReversal ? g_cvarTrue : g_cvarFalse));
				// (s.tullis 2016-04-15 16:00) - NX-100211
				// (j.jones 2011-02-09 11:34) - PLID 42391 - if a reversal, show the column,
				// track that we need to warn, and color the line blue (later code may change
				// the color to red)
				if (pCharge->eAssessmentResultAction_Additional == araaReversal) {
					NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
					pReversalCol->PutStoredWidth(58);
					pRow->PutForeColor(RGB(0, 0, 150));

					// (j.jones 2012-04-19 15:53) - PLID 35306 - if this reversal is unpostable, flag it as such,
					// and color the line red
					if (pCharge->nReversedPaymentID == -1) {
						bHasUnpostableReversals = TRUE;
						pRow->PutForeColor(RGB(192, 0, 0));
					}
				}

				if (pCharge->nBillID == -1) {
					bWarnMissingBillIDs = TRUE;
					pRow->PutForeColor(RGB(192, 0, 0));
				}

				//try to set the Insurance Company ID
				if (m_InsuranceCombo->CurSel == -1) {
					// (j.jones 2011-06-16 08:51) - PLID 39747 - track our insurance combo filter
					// (j.jones 2012-05-02 10:17) - PLID 50138 - don't requery if the where clause didn't change!
					CString strNewFilter;
					strNewFilter.Format("Archived = 0 AND HCFASetupGroupID = (SELECT HCFASetupGroupID FROM InsuranceCoT WHERE InsuranceCoT.PersonID = %li)", pCharge->nInsuranceCoID);
					if (m_strCurInsuranceComboFilter != strNewFilter) {
						m_strCurInsuranceComboFilter = strNewFilter;
						m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
						m_InsuranceCombo->Requery();
					}
					m_InsuranceCombo->SetSelByColumn(iccID, m_AlbertaParser.m_nLikelyInsuranceCoID);

					// (j.jones 2008-07-11 16:38) - PLID 28756 - now try to set our default category and description
					TrySetDefaultInsuranceDescriptions();
				}

				//try to set the provider ID
				// (j.jones 2008-06-18 14:25) - PLID 21921 - added efficiency by caching this value
				if (m_ProviderCombo->CurSel == -1) {
					long nProvID = pCharge->nProviderID;
					if (nProvID != -1) {
						m_ProviderCombo->SetSelByColumn(0, nProvID);
					}
				}

				_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}", pCharge->nBillID);
				if (!rs->eof) {
					pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
				}
				else
					pRow->PutValue(COLUMN_BILL_DATE, g_cvarNull);
				rs->Close();

				// (j.jones 2011-03-18 09:46) - PLID 42157 - moved the insurance info. calculations
				// earlier in this function
				long nInsuranceCoID = -1;

				// (j.jones 2005-04-27 16:11) - PLID 16324 - We need to be able to apply an EOB payment
				// to multiple insurance companies from the same HCFA group. This means we need to narrow
				// down to one HCFA group, and on a per-patient level specifically find each individual insured party.
				// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
				// (j.jones 2011-04-04 15:49) - PLID 42571 - this is now calculated earlier in the process and stored in pClaim
				long nInsuredPartyID = pCharge->nInsuredPartyID;
				// (s.tullis 2016-04-20 9:50) - NX-100185 
				pRow->PutRefCellFormatOverride(COLUMN_INSURED_PARTY_ID, GetInsuredPartyColumnCombo((long)pCharge->nPatientID));
				pRow->PutValue(COLUMN_INSURED_PARTY_ID, (long)nInsuredPartyID);
				rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID, InsuranceCoT.Name, "
					"HCFASetupT.ID AS HCFASetupID, HCFASetupT.Name AS HCFASetupName, InsuredPartyT.RespTypeID, "
					"InsuredPartyT.SubmitAsPrimary "
					"FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
					"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
				if (!rs->eof) {
					nInsuranceCoID = AdoFldLong(rs, "PersonID", -1);
					pRow->PutValue(COLUMN_INS_CO_ID, nInsuranceCoID);
					pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(AdoFldString(rs, "Name", "")));
					// (j.armen 2012-02-20 11:04) - PLID 48239 - Fill correctly with the HCFASetupID
					pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(rs, "HCFASetupID", -1));
					pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(AdoFldString(rs, "HCFASetupName", "")));
					// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
					pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(rs, "RespTypeID", -1));
					// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
					pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, rs->Fields->Item["SubmitAsPrimary"]->Value);
				}
				else {
					pRow->PutValue(COLUMN_INS_CO_ID, (long)-1);
					pRow->PutValue(COLUMN_INS_CO_NAME, "");
					pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);
					pRow->PutValue(COLUMN_HCFA_GROUP_NAME, "");
					pRow->PutValue(COLUMN_RESP_TYPE_ID, (long)-1);
					pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, g_cvarFalse);
				}
				rs->Close();

				if (nInsuredPartyID == -1) {
					bWarnMissingInsuredParties = TRUE;
					pRow->PutForeColor(RGB(192, 0, 0));
				}

				// (j.jones 2011-03-18 09:41) - PLID 42157 - added FeeSchedAllowable
				rs = CreateParamRecordset("SELECT Date, ItemCode, "
					"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
					"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes, "
					"Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, AllowableInsuredQ.InsuranceCoID) * ChargesT.Quantity * "
					"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
					"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
					"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
					"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) ), 2) AS FeeSchedAllowableQty "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
					"LEFT JOIN ( "
					"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
					"	FROM InsuredPartyT "
					"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
					"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0", pCharge->nChargeID);

				COleCurrency cyChargeAmt = COleCurrency(0, 0);

				if (!rs->eof) {

					pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);

					//should be equivalent to pCharge->strServiceID
					pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

					// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
					_variant_t varNoteIcon;
					if (!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
						//load from the datalist's icon for no notes
						varNoteIcon = (LPCTSTR)"BITMAP:FILE";
					}
					else {
						//load our icon for having notes
						varNoteIcon = (long)m_hNotes;
					}
					pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

					cyChargeAmt = AdoFldCurrency(rs, "Amount", COleCurrency(0, 0));
					pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));

					// (j.jones 2011-03-18 09:32) - PLID 42157 - added FeeSchedAllowable, which multiplies by
					// quantity and modifiers
					pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, rs->Fields->Item["FeeSchedAllowableQty"]->Value);
				}
				else {
					pRow->PutValue(COLUMN_CHARGE_DATE, g_cvarNull);
					pRow->PutValue(COLUMN_CPT_CODE, g_cvarNull);
					// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
					pRow->PutValue(COLUMN_NOTE_ICON, g_cvarNull);
					pRow->PutValue(COLUMN_CHARGE_AMOUNT, g_cvarNull);
					pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);
				}
				rs->Close();

				pRow->PutValue(COLUMN_PAYMENT, _variant_t(COleCurrency(pCharge->cyFinalAssessedAmount)));

				// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
				// (j.jones 2016-04-19 13:14) - NX-100161 - old column removed, now this is the total of positive adjustments only,
				// but it's always zero for Alberta
				pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(0, 0)));

				pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(0, 0)));

				pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);

				pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _variant_t(_bstr_t("")));

				pRow->PutValue(COLUMN_ADJ_REASON_CODE, _variant_t(_bstr_t("")));

				pRow->PutValue(COLUMN_ADJ_REASON, _variant_t(_bstr_t("")));

				pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(COleCurrency(0, 0)));

				//get the total of current patient applies made on this charge
				COleCurrency cyPatApplies = COleCurrency(0, 0);
				COleCurrency cyPatientResp = GetChargePatientResp(pCharge->nChargeID);
				COleCurrency cyPatientBalance = GetChargePatientBalance(pCharge->nChargeID, pCharge->nPatientID);
				cyPatApplies = cyPatientResp - cyPatientBalance;
				pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));

				//calculate the anticipated patient balance after this EOB is applied
				COleCurrency cyPatBalance = COleCurrency(0, 0);
				//expected patient resp - existing patient applies
				cyPatBalance = cyPatientResp - cyPatApplies;
				pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

				//calculate the anticipated insurance balance after this EOB is applied
				COleCurrency cyInsBalance = COleCurrency(0, 0);
				cyInsBalance = cyChargeAmt - pCharge->cyFinalAssessedAmount;
				pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

				// (j.jones 2011-02-09 12:06) - PLID 42391 - auto-skip if a reversal
				// (j.jones 2011-09-28 14:03) - PLID 45486 - auto-skip if we have invalid payment totals
				// (j.jones 2012-04-18 15:59) - PLID 35306 - we support posting reversals now, so we won't auto-skip those anymore
				pRow->PutValue(COLUMN_SKIPPED, g_cvarFalse);

				// (j.jones 2008-11-17 15:23) - PLID 32045 - now we track the claim & charge pointers in the datalist
				pRow->PutValue(COLUMN_CLAIM_PTR, g_cvarNull);
				//TES 9/18/2014 - PLID 62580 - Store the raw pointer
				pRow->PutValue(COLUMN_CHARGE_PTR, (long)pCharge.get());
				// (s.tullis 2016-04-15 16:00) - NX-100211
				m_EOBList->AddRowAtEnd(pRow,NULL);

				// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
				// function, as we may have disabled this ability
				PeekAndPump_EOBDlg();

				//now we have only processed the charges specified in the remittance file, there could be more
				// (j.jones 2009-08-21 11:31) - PLID 35303 - only add charges if we have already added charges from this bill,
				// just because we have a pClaim entry with that nBillID does not mean we added charges from it
				// (j.jones 2011-03-18 15:03) - PLID 42905 - we already have the charge IDs in our memory objects,
				// what we need to do is just get a list of all charges for the same patient that we are going to 
				// post to, because maybe we just haven't added their rows yet, but will later
				CArray<long, long> aryUsedCharges;
				m_AlbertaParser.GetChargeIDsByPatientID(pCharge->nPatientID, aryUsedCharges);
				if (aryUsedCharges.GetSize() > 0) {
					// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
					_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID "
						"FROM ChargesT "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"AND BillID = {INT} "
						"AND ChargesT.ID NOT IN ({INTARRAY}) "
						"AND BillID IN (SELECT BillID FROM ChargesT WHERE ID IN ({INTARRAY}))", pCharge->nBillID, aryUsedCharges, aryUsedCharges);
					while (!rsCharges->eof) {
						// (s.tullis 2016-04-15 16:00) - NX-100211
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetNewRow();

						long nChargeID = AdoFldLong(rsCharges, "ID");

						pRow->PutValue(COLUMN_EOB_INDEX, 0);
						pRow->PutValue(COLUMN_PATIENT_ID, (long)pCharge->nPatientID);
						// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
						pCharge->nUserDefinedID = GetExistingPatientUserDefinedID(pCharge->nPatientID);
						pRow->PutValue(COLUMN_PATIENT_USERDEFINED_ID, (long)pCharge->nUserDefinedID);
						pRow->PutValue(COLUMN_PATIENT_NAME, _bstr_t(GetExistingPatientName((long)pCharge->nPatientID)));
						pRow->PutValue(COLUMN_BILL_ID, (long)pCharge->nBillID);
						pRow->PutValue(COLUMN_CHARGE_ID, (long)nChargeID);

						// (j.jones 2008-12-16 10:14) - PLID 32317 - added hidden columns for patient birthdate and OHIP Health Card Number and Version Code
						_variant_t varBirthDate = g_cvarNull;
						COleDateTime dtBirthDate = GetExistingPatientBirthDate((long)pCharge->nPatientID);
						if (dtBirthDate.GetStatus() != COleDateTime::invalid) {
							varBirthDate.vt = VT_DATE;
							varBirthDate.date = dtBirthDate;
						}
						pRow->PutValue(COLUMN_BIRTHDATE, varBirthDate);
						pRow->PutValue(COLUMN_OHIP_HEALTH_NUM, ""); //this isn't an OHIP import
						pRow->PutValue(COLUMN_OHIP_VERSION_CODE, "");

						pRow->PutValue(COLUMN_DUPLICATE, (long)0);

						// (j.jones 2011-02-09 11:24) - PLID 42391 - track if this claim is a reversal
						pRow->PutValue(COLUMN_REVERSAL, (pCharge->eAssessmentResultAction_Additional == araaReversal ? g_cvarTrue : g_cvarFalse));
						// (s.tullis 2016-04-15 16:00) - NX-100211
						// (j.jones 2011-02-09 11:34) - PLID 42391 - if a reversal, show the column,
						// track that we need to warn, and color the line blue (later code may change
						// the color to red)
						if (pCharge->eAssessmentResultAction_Additional == araaReversal) {
							NXDATALIST2Lib::IColumnSettingsPtr pReversalCol = m_EOBList->GetColumn(COLUMN_REVERSAL);
							pReversalCol->PutStoredWidth(58);
							pRow->PutForeColor(RGB(0, 0, 150));

							// (j.jones 2012-04-19 15:53) - PLID 35306 - since this is a manually added charge that
							// is unreferenced by the EOB, we do not need to check if it is unpostable
						}

						_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}", pCharge->nBillID);
						if (!rs->eof) {
							pRow->PutValue(COLUMN_BILL_DATE, rs->Fields->Item["Date"]->Value);
						}
						else
							pRow->PutValue(COLUMN_BILL_DATE, g_cvarNull);
						rs->Close();

						// (j.jones 2011-03-18 09:46) - PLID 42157 - moved the insurance info. calculations
						// earlier in this function
						long nInsuranceCoID = -1;

						// (j.jones 2005-04-27 16:11) - PLID 16324 - We need to be able to apply an EOB payment
						// to multiple insurance companies from the same HCFA group. This means we need to narrow
						// down to one HCFA group, and on a per-patient level specifically find each individual insured party.
						// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
						// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct	
						// (j.jones 2012-05-01 15:02) - PLID 47477 - supported the ProcessedAs flag
						EInsuredPartyInfo eInsuredInfo; 
						eInsuredInfo.nInsuredPartyID = pCharge->nInsuredPartyID;
						eInsuredInfo.nCategoryPlacement = pCharge->nRespTypeID;
						pRow->PutValue(COLUMN_INSURED_PARTY_ID, (long)pCharge->nInsuredPartyID);
						rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID, InsuranceCoT.Name, "
							"HCFASetupT.ID AS HCFASetupID, HCFASetupT.Name AS HCFASetupName, InsuredPartyT.RespTypeID, "
							"InsuredPartyT.SubmitAsPrimary "
							"FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
							"WHERE InsuredPartyT.PersonID = {INT}", eInsuredInfo.nInsuredPartyID);
						if (!rs->eof) {
							nInsuranceCoID = AdoFldLong(rs, "PersonID", -1);
							pRow->PutValue(COLUMN_INS_CO_ID, nInsuranceCoID);
							pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(AdoFldString(rs, "Name", "")));
							pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(rs, "HCFASetupID", -1));
							pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(AdoFldString(rs, "HCFASetupName", "")));
							// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
							pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(rs, "RespTypeID", -1));
							// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
							pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, rs->Fields->Item["SubmitAsPrimary"]->Value);
						}
						else {
							pRow->PutValue(COLUMN_INS_CO_ID, (long)-1);
							pRow->PutValue(COLUMN_INS_CO_NAME, "");
							pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);
							pRow->PutValue(COLUMN_HCFA_GROUP_NAME, "");
							pRow->PutValue(COLUMN_RESP_TYPE_ID, (long)-1);
							pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, g_cvarFalse);
						}
						rs->Close();

						if (eInsuredInfo.nInsuredPartyID == -1) {
							bWarnMissingInsuredParties = TRUE;
							pRow->PutForeColor(RGB(192, 0, 0));
						}

						// (j.jones 2011-03-18 09:41) - PLID 42157 - added FeeSchedAllowable
						// (d.lange 2015-11-30 15:01) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
						rs = CreateParamRecordset("SELECT Date, ItemCode, "
							"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
							"Convert(bit, CASE WHEN NotesQ.LineItemID Is Not Null THEN 1 ELSE 0 END) AS HasNotes, "
							"Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, AllowableInsuredQ.InsuranceCoID) * ChargesT.Quantity * "
							"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
							"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
							"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
							"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) ), 2) AS FeeSchedAllowableQty "
							"FROM ChargesT "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT DISTINCT LineItemID FROM Notes) AS NotesQ ON LineItemT.ID = NotesQ.LineItemID "
							"LEFT JOIN ( "
							"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
							"	FROM InsuredPartyT "
							"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
							") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
							"WHERE ChargesT.ID = {INT} AND LineItemT.Deleted = 0", nChargeID);

						COleCurrency cyChargeAmt = COleCurrency(0, 0);
						COleCurrency cyPaymentAmt = COleCurrency(0, 0);
						COleCurrency cyPatientAmt = COleCurrency(0, 0);
						CString strAdjustmentDescription = "";
						CString strAdjustmentGroupCode = "";
						CString strAdjustmentReasonCode = "";

						if (!rs->eof) {

							pRow->PutValue(COLUMN_CHARGE_DATE, rs->Fields->Item["Date"]->Value);

							//should be equivalent to pCharge->strServiceID
							pRow->PutValue(COLUMN_CPT_CODE, rs->Fields->Item["ItemCode"]->Value);

							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							_variant_t varNoteIcon;
							if (!VarBool(rs->Fields->Item["HasNotes"]->Value, FALSE)) {
								//load from the datalist's icon for no notes
								varNoteIcon = (LPCTSTR)"BITMAP:FILE";
							}
							else {
								//load our icon for having notes
								varNoteIcon = (long)m_hNotes;
							}
							pRow->PutValue(COLUMN_NOTE_ICON, varNoteIcon);

							// (j.jones 2011-04-25 11:12) - PLID 43406 - handle NULLs, just incase
							cyChargeAmt = AdoFldCurrency(rs, "Amount", COleCurrency(0, 0));
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, _variant_t(cyChargeAmt));

							// (j.jones 2011-03-18 09:32) - PLID 42157 - added FeeSchedAllowable, which multiplies by
							// quantity and modifiers
							pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, rs->Fields->Item["FeeSchedAllowableQty"]->Value);
						}
						else {
							pRow->PutValue(COLUMN_CHARGE_DATE, g_cvarNull);
							pRow->PutValue(COLUMN_CPT_CODE, g_cvarNull);
							// (j.jones 2011-01-04 14:37) - PLID 16423 - supported billing notes
							pRow->PutValue(COLUMN_NOTE_ICON, g_cvarNull);
							pRow->PutValue(COLUMN_CHARGE_AMOUNT, g_cvarNull);
							pRow->PutValue(COLUMN_FEE_SCHED_ALLOWABLE, g_cvarNull);
						}
						rs->Close();

						pRow->PutValue(COLUMN_PAYMENT, _variant_t(cyPaymentAmt));

						// (r.farnworth 2014-01-28 11:49) - PLID 52596 - we want to cache the original adjustment so that we can roll back to it
						// (j.jones 2016-04-19 13:14) - NX-100161 - old column removed, now this is the total of positive adjustments only,
						// but it's always zero for Alberta
						pRow->PutValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

						pRow->PutValue(COLUMN_TOTAL_ADJUSTMENTS, _variant_t(COleCurrency(0,0)));

						// (j.jones 2011-03-18 09:32) - PLID 42157 - added EOB allowable, but it is
						// always null here because this charge doesn't exist on the EOB
						pRow->PutValue(COLUMN_EOB_ALLOWABLE, g_cvarNull);

						pRow->PutValue(COLUMN_ADJ_GROUP_CODE, _variant_t(_bstr_t(strAdjustmentGroupCode)));

						pRow->PutValue(COLUMN_ADJ_REASON_CODE, _variant_t(_bstr_t(strAdjustmentReasonCode)));

						pRow->PutValue(COLUMN_ADJ_REASON, _variant_t(_bstr_t(strAdjustmentDescription)));

						pRow->PutValue(COLUMN_OTHER_RESP, _variant_t(cyPatientAmt));

						//get the total of current patient applies made on this charge
						COleCurrency cyPatApplies = COleCurrency(0, 0);
						COleCurrency cyPatientResp = GetChargePatientResp(nChargeID);
						COleCurrency cyPatientBalance = GetChargePatientBalance(nChargeID, pCharge->nPatientID);
						cyPatApplies = cyPatientResp - cyPatientBalance;
						pRow->PutValue(COLUMN_PAT_APPLIES, _variant_t(cyPatApplies));

						//calculate the anticipated patient balance after this EOB is applied
						COleCurrency cyPatBalance = COleCurrency(0, 0);
						//expected patient resp - existing patient applies
						cyPatBalance = (cyPatientResp < cyPatientAmt ? cyPatientAmt : cyPatientResp) - cyPatApplies;
						pRow->PutValue(COLUMN_PAT_BALANCE, _variant_t(cyPatBalance));

						//calculate the anticipated insurance balance after this EOB is applied
						COleCurrency cyInsBalance = COleCurrency(0, 0);
						cyInsBalance = cyChargeAmt - cyPatientAmt - cyPaymentAmt /*- cyAdjustmentAmt (Alberta doesn't have adjustments)*/;
						pRow->PutValue(COLUMN_INS_BALANCE, _variant_t(cyInsBalance));

						// (j.jones 2011-02-09 12:06) - PLID 42391 - auto-skip if a reversal
						// (j.jones 2011-09-28 14:03) - PLID 45486 - auto-skip if we have invalid payment totals
						// (j.jones 2012-04-18 15:59) - PLID 35306 - we support posting reversals now, so we won't auto-skip those anymore
						pRow->PutValue(COLUMN_SKIPPED, g_cvarFalse);

						// (j.jones 2008-11-17 15:23) - PLID 32045 - now we track the claim & charge pointers in the datalist
						pRow->PutValue(COLUMN_CLAIM_PTR, g_cvarNull);
						pRow->PutValue(COLUMN_CHARGE_PTR, g_cvarNull);	//there is no tracked charge in this branch of code
						// (s.tullis 2016-04-15 16:00) - NX-100211
						m_EOBList->AddRowAtEnd(pRow,NULL);

						rsCharges->MoveNext();

						// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
						// function, as we may have disabled this ability
						PeekAndPump_EOBDlg();
					}
					rsCharges->Close();
				}

				m_progress.StepIt();

				// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
				// function, as we may have disabled this ability
				PeekAndPump_EOBDlg();
			}
			// (j.jones 2009-07-01 16:55) - PLID 34778 - UpdateOverpaymentLabel will check and see whether
			// the EOB, when fully posted, will potentially create a batch payment that is more
			// than the original EOB check amount was for. If so, the label will be updated.
			// (j.jones 2012-10-05 10:38) - PLID 52929 - Renamed to reflect that this now handles underpayments too,
			// for when the user wishes to skip payments and reduce the batch payment total.
			UpdateOverpaymentUnderpaymentLabel();

			m_progress.SetPos(0);
			m_progressStatus.SetWindowText("Import complete.");
			EnableButtons(TRUE);

			//warn the user if data is missing

			CString strWarning;
			if (bWarnMissingInsuredParties) {

				strWarning = "Practice could not match up an insured party with at least one patient on this EOB.\n"
					"The patient(s) with no matching insured party have been highlighted in red.\n"
					"     - Please ensure that each patient has an insurance company, ideally one Insurance Company used by all patients.\n";
			}

			if (bWarnMissingBillIDs) {
				if (!strWarning.IsEmpty()) {
					strWarning += "\nIn addition, ";
				}
				strWarning += "Practice could not match up a bill with at least one patient on this EOB.\n"
					"The patient(s) with no matching bill have been highlighted in red, and will have no charge amount listed.\n"
					"Possible reasons for a bill to not be found are:\n"
					"     - The Bill date does not match the date of the claim on this EOB.\n"
					"     - The Bill total does not match the claim total on this EOB.\n"
					"     - The Service Codes returned on this EOB do not match the Service Codes on any bill for the patient.\n";
			}

			if (bWarnMissingInsuredParties || bWarnMissingBillIDs) {
				strWarning += "\nPractice will not be able to apply to the highlighted patient(s) until these mismatches are resolved.";

				MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION | MB_OK);
			}

			// wait for the insurance list to finish, and if empty, populate with insurance companies
			m_InsuranceCombo->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			// (j.jones 2012-05-02 10:16) - PLID 50138 - the combo is considered empty if
			// there is only one row and it's a built-in "show all" / "show filtered" row,
			// IsInsuranceComboEmpty will check for that state
			if (IsInsuranceComboEmpty()) {
				//this reflects an error state, because we should have a match, but we should at least
				//populate the list with the insurance companies we know the patients have
				Log("The Insurance Company dropdown list is empty! Filtering on patient insurances now.");
				// (s.tullis 2016-04-15 16:00) - NX-100211
				CString strIDList;
				for (NXDATALIST2Lib::IRowSettingsPtr pTraverseRowPtr = m_EOBList->GetFirstRow(); pTraverseRowPtr != nullptr; pTraverseRowPtr = pTraverseRowPtr->GetNextRow()) {
					if (pTraverseRowPtr) {
						long nInsuranceCoID = VarLong(pTraverseRowPtr->GetValue(COLUMN_INS_CO_ID), -1);
						CString str;
						str.Format("%li,", nInsuranceCoID);
						strIDList += str;
					}
				}
				strIDList.TrimRight(",");

				if (!strIDList.IsEmpty()) {
					// (j.jones 2011-06-16 08:51) - PLID 39747 - track our insurance combo filter
					m_strCurInsuranceComboFilter.Format("Archived = 0 AND InsuranceCoT.PersonID IN (%s)", strIDList);
					m_InsuranceCombo->PutWhereClause(_bstr_t(m_strCurInsuranceComboFilter));
					m_InsuranceCombo->Requery();
				}

				//while that is happening, let's warn the user about this
				CalcWarnNoInsuranceCompanyChosen();
			}

			return;
		}

		m_progress.SetPos(0);
		m_progressStatus.SetWindowText("Ready to import.");
		EnableButtons(TRUE);
		return;

	}NxCatchAll("Error importing Alberta electronic remittance file.");

	// (j.jones 2010-02-05 13:08) - PLID 32184 - added more logging
	Log("ImportAlbertaFile() - Exception Occurred");

	m_progress.SetPos(0);
	m_progressStatus.SetWindowText("Error importing remittance file.");
	EnableButtons(TRUE);
	return;

}

//Alberta doesn't create adjustments
void CEOBDlg::CalcReversedPaymentIDs(AlbertaAssessments::ChargeInfoPtr pCharge)
{
	//throw exceptions to the caller

	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	if (m_ERemitType != ertAlberta) {
		return;
	}

	if (pCharge->eAssessmentResultAction_Additional != araaReversal){
		return;
	}

	//TES 9/30/2014 - PLID 62536 - Per the Alberta documentation: "If the Assessment Outcome is A (applied) and the Transaction Action Code is A (add) 
	// or the Assessment Outcome is R (refused) or H (held), only a current assessment result record will be created."
	// So, in that case, we don't want to reverse the existing payment
	if ((pCharge->eAssessmentResultAction_Main == aramHeld || pCharge->eAssessmentResultAction_Main == aramRefused)
		|| (pCharge->eAssessmentResultAction_Main == aramApplied && pCharge->eTransactionActionCode == tacAdd)) {
		return;
	}

	//Now we know we're on a reversed charge, and we have the information we need
	//to try and find the posted payments and adjustments to reverse,
	//so populate the payment ID to reverse and the adjustment ID to reverse.

	//track the BatchPaymentID of the payment to reverse
	long nReversedBatchPaymentID = -1;

	//Find the payment ID by matching on any payment by this insurance company for the
	//reversed payment amount. It has to be applied, but the apply amount does not need
	//to match the reversal amount (though it really should). E-Remittance would have
	//only made one payment for the amount originally intended to be applied.
	//The ORDER BY is fairly meaningless as we don't expect to actually find multiple results.
	//TES 9/30/2014 - PLID 62536 - Neither of the amounts we have on this claim necessarily match up to the amount of the payment being voided, so I took that 
	// qualifier out, we'll just look for the ChargeID and InsuredPartyID
	_RecordsetPtr rsReversedPay = CreateParamRecordset("SELECT TOP 1 PaymentsT.ID, PaymentsT.BatchPaymentID "
		"FROM LineItemT "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND AppliesT.DestID = {INT} AND PaymentsT.InsuredPartyID = {INT} "
		"ORDER BY LineItemT.InputDate DESC",
		pCharge->nChargeID, pCharge->nInsuredPartyID);
	if (rsReversedPay->eof) {
		//we could not find a payment to reverse

		//this should have already initialized to -1, but re-affirm it
		pCharge->nReversedPaymentID = -1;
		//log the failure
		Log("CalcInternalIDs: Failed to find a reversed payment for charge ID %li, insured party ID %li, amount of %s.", pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(pCharge->cyFinalAssessedAmount));
	}
	else {
		//we successfully found a payment to reverse
		long nReversedPaymentID = VarLong(rsReversedPay->Fields->Item["ID"]->Value);

		//do not use this payment ID if we are already using it elsewhere,
		//which should be impossible unless they foolishly listed the same takeback twice
		if (IsReversedPaymentInUse(nReversedPaymentID, true)) {
			pCharge->nReversedPaymentID = -1;
				//log the failure
			Log("CalcInternalIDs: Found reversed payment ID %li for charge ID %li, insured party ID %li, amount of %s. Cannot use this payment because another takeback is already using it.", nReversedPaymentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(pCharge->cyFinalAssessedAmount));
		}
		else {
			//assign the payment ID
			pCharge->nReversedPaymentID = nReversedPaymentID;

			//track the BatchPaymentID of the payment to reverse (could be -1, but not likely)
			nReversedBatchPaymentID = VarLong(rsReversedPay->Fields->Item["ID"]->Value, -1);

			//log the success
			Log("CalcInternalIDs: Successfully found reversed payment ID %li for charge ID %li, insured party ID %li, amount of %s.", nReversedPaymentID, pCharge->nChargeID, pCharge->nInsuredPartyID, FormatCurrencyForInterface(pCharge->cyFinalAssessedAmount));
		}
	}
	rsReversedPay->Close();
}
// (s.tullis 2016-04-15 16:00) - NX-100211
void CEOBDlg::EditingFinishingEobDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		// (s.tullis 2016-04-20 9:50) - NX-100185 
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow ==  nullptr || nCol == COLUMN_SKIPPED || nCol == COLUMN_INSURED_PARTY_ID)
			return;

		// (j.jones 2016-04-19 10:09) - NX-100161 - you can no longer edit adjustments

	}NxCatchAll(__FUNCTION__);
}

#define YELLOW_BACK_COLOR RGB(255, 255, 0)
#define BLACK_TEXT_COLOR RGB(0,0,0)

// (s.tullis 2016-04-15 16:00) - NX-100211
void CEOBDlg::EditingFinishedEobDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
	BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

	try {


		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == nullptr) {
			return;
		}

		// (j.jones 2011-07-15 14:21) - PLID 41579 - ensured we looked at bCommit
		if (!bCommit) {
			return;
		}
		// (s.tullis 2016-04-20 9:48) - NX-100185 - Update the other columns that depend on this column if we change
		if (nCol == COLUMN_INSURED_PARTY_ID)
		{
			if (VarLong(varOldValue) != VarLong(varNewValue)) {

				bool bColorRowYellow = false;

				long nCurInsSel = m_InsuranceCombo->GetCurSel();
				long nSelInsuranceCoID = -1;
				if (nCurInsSel != -1) {
					nSelInsuranceCoID = VarLong(m_InsuranceCombo->GetValue(nCurInsSel, iccID), -1);
				}

				//first, always fill the other columns with the new insured party's information
				long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID), -1);
				_RecordsetPtr prs = CreateParamRecordset(R"(
SELECT InsuredPartyT.PatientID,
	   InsuranceCoT.PersonID as InsCoID,
	   InsuranceCoT.Name as InsCoName,
	   InsuranceCoT.HCFASetupGroupID,
	   InsuranceCoT.HCFAPayerID as PayerID,
	   InsuredPartyT.RespTypeID,
	   InsuredPartyT.SubmitAsPrimary,
	   HCFASetupT.Name AS HCFASetupName	
FROM InsuredPartyT
INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID
LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID
	  WHERE InsuredPartyT.PatientID = {INT} AND InsuredPartyT.PersonID = {INT})"
					, nPatientID, VarLong(varNewValue));
				if (!prs->eof) {
					//for now, assume the row is yellow if the companies do not match
					bColorRowYellow = (nSelInsuranceCoID != AdoFldLong(prs, "InsCoID",-1));
					pRow->PutValue(COLUMN_INS_CO_ID, AdoFldLong(prs, "InsCoID",-1));
					pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(prs, "HCFASetupGroupID",-1));
					pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(prs, "RespTypeID",-1));
					pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, prs->Fields->Item["SubmitAsPrimary"]->Value);
					pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(AdoFldString(prs, "InsCoName", "")));
					pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(AdoFldString(prs, "HCFASetupName", "")));
				}
				else {
					pRow->PutValue(COLUMN_INS_CO_ID, (long)-1);
					pRow->PutValue(COLUMN_INS_CO_NAME, "");
					pRow->PutValue(COLUMN_HCFA_GROUP_ID, (long)-1);
					pRow->PutValue(COLUMN_HCFA_GROUP_NAME, "");
					pRow->PutValue(COLUMN_RESP_TYPE_ID, (long)-1);
					pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, g_cvarFalse);
				}
				prs->Close();

				//now we need to decide if the row should be yellow or white

				long nChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID));

				if (nChargeID != -1 && nSelInsuranceCoID != -1) {
					// (j.jones 2016-04-25 12:29) - NX-100186 - added a query to use for payer ID matching,
					// returns all valid insured parties for a given charge & selected insurance company
					std::vector<long> aryChargeIDs;
					aryChargeIDs.push_back(nChargeID);

					bool bInsuredPartyMatch = false;
					long nCountCurActiveResps = 0;

					_RecordsetPtr prs = GetPayerIDMatchingRecordset(aryChargeIDs, nSelInsuranceCoID);
					while (!prs->eof) {
						long nCurInsuredPartyID = AdoFldLong(prs, "InsuredPartyID");
						if (VarLong(varNewValue) == nCurInsuredPartyID) {
							bInsuredPartyMatch = true;
						}
						if (AdoFldLong(prs, "RespTypeID") != -1) {
							nCountCurActiveResps++;
						}
						prs->MoveNext();
					}
					prs->Close();

					//if the insured party matches AND there are not multiple active resps,
					//we can color the row white
					bColorRowYellow = (!bInsuredPartyMatch || nCountCurActiveResps > 1);
				}

				pRow->PutCellBackColor(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? YELLOW_BACK_COLOR : NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellBackColorSel(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? YELLOW_BACK_COLOR : NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellForeColorSel(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? BLACK_TEXT_COLOR : NXDATALIST2Lib::dlColorNotSet);				
			}
		}
		else if (nCol == COLUMN_SKIPPED) {
			// (j.jones 2009-07-01 16:55) - PLID 34778 - since this charge has been skipped (or un-skipped),
			// we must now update the overpayment label
			// (j.jones 2009-07-07 15:20) - PLID 34805 - no we don't, we will not change
			// the overage amount based on what is skipped
			//UpdateOverpaymentLabel();

			// (j.jones 2012-04-18 16:14) - PLID 35306 - You can't skip the negative payment in a reversal
			// and not the positive payment, or vice versa. Find the opposite row (if it exists) and
			// toggle it to the same skip value.
			BOOL bIsReversal = VarBool(pRow->GetValue(COLUMN_REVERSAL), FALSE);
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (bIsReversal && m_ERemitType != ertOhip) {
				ToggleSiblingRowSkipped(pRow, VarBool(varNewValue));
			}

			//leave now, since the remaining code is irrelevant to this change
			return;
		}

		// (j.jones 2016-04-19 10:09) - NX-100161 - you can no longer edit adjustments

	}NxCatchAll(__FUNCTION__);
}


// (s.tullis 2016-04-15 16:00) - NX-100211
// (j.jones 2011-02-09 12:08) - PLID 42391 - added to prevent un-skipping reversals
void CEOBDlg::EditingStartingEobDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == nullptr) {
			return;
		}

		if (nCol == COLUMN_SKIPPED) {

			// (j.jones 2012-09-06 14:04) - PLID 52500 - For all checks for the skipped column, we're trying
			// to stop them from unchecking the Skipped box in cases where we force a line to be skipped.
			// If they try to uncheck, we stop them and warn why.
			// But in the off chance that the skipped box is somehow unchecked, and they are checking it,
			// just let them do it. There's no point in yelling at them for marking it skipped when that's
			// how it was expected to be in the first place.

			// (j.jones 2011-02-09 12:10) - PLID 42391 - if a reversal, disallow from
			// being un-skipped
			// (j.jones 2012-04-18 16:14) - PLID 35306 - Reversals are no longer forcibly skipped all times,
			// but if the reversal is unpostable, it is always skipped.			
			BOOL bIsReversal = VarBool(pRow->GetValue(COLUMN_REVERSAL), FALSE);
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (bIsReversal && m_ERemitType != ertOhip) {

				long nChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID), -1);

				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pRow->GetValue(COLUMN_CLAIM_PTR);
				if (var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
				}

				EOBLineItemInfo *pCharge = NULL;
				var = pRow->GetValue(COLUMN_CHARGE_PTR);
				if (var.vt == VT_I4) {
					pCharge = (EOBLineItemInfo*)VarLong(var);
				}

				if (pClaim != NULL && pCharge != NULL) {
					if (IsUnpostableReversalCharge(pCharge, pClaim)) {
						// (j.jones 2012-09-06 14:04) - PLID 52500 - Only warn if they are trying to uncheck the box.
						// Otherwise, let them mark it skipped.
						if (VarBool(pvarValue)) {
							*pbContinue = FALSE;
							MessageBox("The posting for this claim is a reversal, and that payment and/or adjustment to reverse could not be found.\n"
								"This posting must be skipped. You will need to manually resolve this claim after the EOB has posted.", "Practice", MB_ICONEXCLAMATION | MB_OK);
							return;
						}
						else {
							// (j.jones 2012-09-06 14:12) - If you hit this, investigate why it wasn't marked
							// as skipped to begin with. We will silently let the user continue and mark it skipped,
							// as it should be, but technically this should never happen.
							ASSERT(FALSE);
						}
					}
					//make sure the sibling is not unpostable
					else if (HasUnpostableReversalSibling(pRow, nChargeID, pClaim, pCharge)) {
						// (j.jones 2012-09-06 14:04) - PLID 52500 - Only warn if they are trying to uncheck the box.
						// Otherwise, let them mark it skipped.
						if (VarBool(pvarValue)) {
							*pbContinue = FALSE;
							MessageBox("The posting for this claim is part of a reversal, and that payment and/or adjustment to reverse could not be found.\n"
								"This posting must be skipped. You will need to manually resolve this claim after the EOB has posted.", "Practice", MB_ICONEXCLAMATION | MB_OK);
							return;
						}
						else {
							// (j.jones 2012-09-06 14:12) - If you hit this, investigate why it wasn't marked
							// as skipped to begin with. We will silently let the user continue and mark it skipped,
							// as it should be, but technically this should never happen.
							ASSERT(FALSE);
						}
					}
				}
			}
			
			// (j.jones 2011-09-28 14:43) - PLID 45486 - if this claim has invalid payment totals,
			// we cannot uncheck the skipped column (US only)
			//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
			if (m_ERemitType == ertAnsi) {
				EOBClaimInfo *pClaim = NULL;
				_variant_t var = pRow->GetValue(COLUMN_CLAIM_PTR);
				if (var.vt == VT_I4) {
					pClaim = (EOBClaimInfo*)VarLong(var);
					if (pClaim != NULL && m_EOBParser.HasInvalidPayTotals(pClaim)) {
						// (j.jones 2012-09-06 14:04) - PLID 52500 - Only warn if they are trying to uncheck the box.
						// Otherwise, let them mark it skipped.
						if (VarBool(pvarValue)) {
							*pbContinue = FALSE;
							CString strWarning;

							strWarning.Format("The EOB reports a claim payment field of %s for this claim, "
								"but the total payment amount applied to the claim's charges adds up to %s.\n\n"
								"You will need to manually resolve this claim after the EOB has posted.",
								FormatCurrencyForInterface(pClaim->cyClaimPaymentAmt),
								FormatCurrencyForInterface(m_EOBParser.GetPaymentTotalsForCharges(pClaim)));
							MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION | MB_OK);
							return;
						}
						else {
							// (j.jones 2012-09-06 14:12) - If you hit this, investigate why it wasn't marked
							// as skipped to begin with. We will silently let the user continue and mark it skipped,
							// as it should be, but technically this should never happen.
							ASSERT(FALSE);
						}
					}
				}
			}
		}

		// (j.jones 2016-04-19 10:16) - NX-100161 - you can no longer edit adjustments

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-04-15 16:00) - NX-100211
// (r.farnworth 2014-01-28 12:04) - PLID 52596 - Need a right-click menu option to house the 'Reset' option
void CEOBDlg::RButtonDownEobDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == nullptr)
		{
			return;
		}

		if (m_bIsLoading)
		{
			return;
		}

		// (j.jones 2016-04-19 13:14) - NX-100161 - we no longer revert adjustments, nothing
		// currently needs a right click handler anymore

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-04-15 16:00) - NX-100211 
// (j.jones 2011-01-04 14:40) - PLID 16423 - added billing notes
void CEOBDlg::LeftClickEobDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == nullptr) {
			return;
		}

		if (nCol == COLUMN_NOTE_ICON) {

			//open the billing notes for this charge, if we have one			
			long nChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID));
			if (nChargeID == -1) {
				//ignore this click, this isn't a real charge row
				return;
			}

			long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));

			CNotesDlg dlgNotes(this);
			dlgNotes.m_bIsBillingNote = true;
			// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
			dlgNotes.m_bntBillingNoteType = bntCharge;
			dlgNotes.m_nLineItemID = nChargeID;
			dlgNotes.SetPersonID(nPatientID);
			CNxModalParentDlg dlg(this, &dlgNotes, CString("Charge Notes"));
			dlg.DoModal();

			BOOL bHasNotes = FALSE;

			//refresh the icon
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 LineItemID FROM Notes WHERE Notes.LineItemID = {INT}", nChargeID);
			if (!rs->eof) {
				bHasNotes = TRUE;
			}
			rs->Close();

			//it's uncommon, but a charge could potentially be in this file multiple times,
			//so we need to update all entries
			// (s.tullis 2016-04-20 9:53) - NX-100211 
			for (pRow = m_EOBList->GetFirstRow(); pRow != nullptr; pRow = pRow->GetNextRow()) {

				long nRowChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID));
				if (nRowChargeID == nChargeID) {
					if (!bHasNotes) {
						pRow->PutValue(COLUMN_NOTE_ICON, (LPCTSTR)"BITMAP:FILE");
					}
					else {
						pRow->PutValue(COLUMN_NOTE_ICON, (long)m_hNotes);
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-04-20 9:50) - NX-100185 - Get our Column Format Override 
NXDATALIST2Lib::IFormatSettingsPtr  CEOBDlg::GetInsuredPartyColumnCombo(long nPatientID)
{
	NXDATALIST2Lib::IFormatSettingsPtr pfsInsuredParties(__uuidof(NXDATALIST2Lib::FormatSettings));
	try {
		pfsInsuredParties->PutDataType(VT_I4);
		pfsInsuredParties->PutFieldType(NXDATALIST2Lib::cftComboSimple);
		pfsInsuredParties->PutEditable(VARIANT_TRUE);
		pfsInsuredParties->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
		pfsInsuredParties->PutComboSource(_bstr_t(GetCellInsuredPartyFormatSQL(nPatientID)));
		pfsInsuredParties->EmbeddedComboDropDownMaxHeight = 300;
		pfsInsuredParties->EmbeddedComboDropDownWidth = 200;
	}NxCatchAll(__FUNCTION__)
		return pfsInsuredParties;
}

// (s.tullis 2016-04-20 9:50) - NX-100185 - SQL for our combo override
CString CEOBDlg::GetCellInsuredPartyFormatSQL(long nPatientID)
{
	return FormatString(R"(SELECT InsParties.ID
	,InsParties.Name
FROM (
	SELECT - 1 AS ID
		,'< No Insured Party >' AS Name
		,1 AS Priority
	
	UNION
	
	SELECT InsuredPartyT.PersonID AS ID
		,InsuranceCoT.Name + ' (' + RespTypeT.TypeName + ')' AS Name
		,RespTypeT.Priority
	FROM InsuranceCoT
	INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID
	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID
	WHERE InsuredPartyT.PatientID = %li
	) InsParties
ORDER BY (
		CASE 
			WHEN InsParties.Priority = - 1
				THEN 1
			ELSE 0
			END
		) ASC
	,InsParties.Priority ASC
      )", nPatientID);
}

// (s.tullis 2016-04-20 9:48) - NX-100186 
void CEOBDlg::EnsureWarnCorrectInsuredParties()
{
	try {

		long nCurInsSel = m_InsuranceCombo->GetCurSel();
		if (nCurInsSel == -1)
			return;
		
		long nSelInsuranceCoID = VarLong(m_InsuranceCombo->GetValue(nCurInsSel, iccID),-1);
		std::vector<long> arrCheckChargeIDs;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow(); pRow != nullptr; pRow = pRow->GetNextRow())
		{
			//first compare by insurance company, if it mismatches, color yellow
			long nClaimItemInsuranceCoID = VarLong(pRow->GetValue(COLUMN_INS_CO_ID), -1);
			if (nClaimItemInsuranceCoID != nSelInsuranceCoID)
			{	//Put the backcolor yellow
				pRow->PutCellBackColor(COLUMN_INSURED_PARTY_ID, YELLOW_BACK_COLOR);
				pRow->PutCellBackColorSel(COLUMN_INSURED_PARTY_ID, YELLOW_BACK_COLOR);
				pRow->PutCellForeColorSel(COLUMN_INSURED_PARTY_ID, BLACK_TEXT_COLOR);
			}	
			// if we changed it before and it's correct now make sure it's updated
			else {
				pRow->PutCellBackColor(COLUMN_INSURED_PARTY_ID, NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellBackColorSel(COLUMN_INSURED_PARTY_ID, NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellForeColorSel(COLUMN_INSURED_PARTY_ID, NXDATALIST2Lib::dlColorNotSet);
			}

			//we will do a more thorough check by charge ID next
			long nChargeID = VarLong(pRow->GetValue(COLUMN_CHARGE_ID), -1);
			if (nChargeID != -1) {
				arrCheckChargeIDs.push_back(nChargeID);
			}
		}	
		
		// (s.tullis 2016-04-15 16:00) - NX-100299
		if (arrCheckChargeIDs.size() > 0) {
			TryUpdateInsuredParties(arrCheckChargeIDs, nSelInsuranceCoID);
		}

		//are any rows still yellow?
		bool bYellowRowFound = false;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow(); pRow != nullptr && !bYellowRowFound; pRow = pRow->GetNextRow()) {
			if (pRow->GetCellBackColor(COLUMN_INSURED_PARTY_ID) == YELLOW_BACK_COLOR) {
				bYellowRowFound = true;
				//set this selection
				m_EOBList->PutCurSel(pRow);

				CString strWarning = "Please check the yellow highlighted insurance companies below to confirm that they match the source of this payment.\n"
					"If it is not the correct insurance company, check the “skip” checkbox on that line item and post the payment manually.";

				MessageBox(strWarning, "Practice", MB_OK | MB_ICONQUESTION);
				break;
			}
		}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-04-15 16:00) - NX-100299
void CEOBDlg::TryUpdateInsuredParties(std::vector<long> &aryChargeIDs, const long &nInsuranceCoID)
{
	try {

		if (aryChargeIDs.size() == 0) {
			//this function should not have been called
			ASSERT(FALSE);
		}

		NXDATALIST2Lib::IRowSettingsPtr pFirstYellowRow = NULL;

		// (j.jones - added a query to use for payer ID matching,
		// returns all valid insured parties for a given charge & selected insurance company
		_RecordsetPtr prs = GetPayerIDMatchingRecordset(aryChargeIDs, nInsuranceCoID);		

		//this recordset can return multiple insured parties per charge,
		//need to cache which charge we're on, the last insured party returned,
		//and track the highest balance for each insured party
		long nLastChargeID = -1;
		long nLastInsuredPartyID = -1;
		COleCurrency cyHighestChargeBalance = g_ccyInvalid;

		//also track how many matching active insured parties exist for this charge
		long nCountCurActiveResps = 0;

		//these track the current insured party ID in the EOB list
		//such that if multiple qualifying matches are found,
		//and one of them is already selected, we might not change it
		//if it has the same balance as a competitor
		long nOriginallySelectedInsuredPartyID = -1;

		bool bColorRowYellow = false;

		while (!prs->eof)
		{
			bool bUseThisInsuredParty = true;

			long nCurChargeID = AdoFldLong(prs, "ChargeID");
			long nCurPatientID = AdoFldLong(prs, "PatientID");
			long nCurInsuredPartyID = AdoFldLong(prs, "InsuredPartyID");
			long nCurRespTypeID = AdoFldLong(prs, "RespTypeID");

			//find the row
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->FindByColumn(COLUMN_CHARGE_ID, nCurChargeID, m_EOBList->GetFirstRow(), VARIANT_FALSE);
			if (pRow) {

				if (nLastChargeID == nCurChargeID && nLastInsuredPartyID != -1
					&& nLastInsuredPartyID != nCurInsuredPartyID) {
					//if the same charge comes back twice it means we have more than one
					//possible insured party that matched by payer ID, so we need to use
					//the one with the largest balance on this charge

					if (AdoFldLong(prs, "RespTypeID") != -1) {
						nCountCurActiveResps++;
					}

					//If the same charge has multiple matching active resps,
					//we will always color this row yellow. We will not do so
					//if it's one active and one or more inactive.
					if (nCountCurActiveResps > 1) {
						bColorRowYellow = true;
					}

					//perform balance checks
					//do NOT skip this if we already found & used the original insured party,
					//we may yet move it to a party of a higher balance
					
					//if cyHighestChargeBalance is invalid, we need to get the balance of that
					//insured party first
					long nInsuredPartyToUse = -1;
					if (cyHighestChargeBalance.GetStatus() == COleCurrency::invalid) {

						COleCurrency cyCharges = COleCurrency(0, 0);
						COleCurrency cyPayments = COleCurrency(0, 0);
						COleCurrency cyAdjustments = COleCurrency(0, 0);
						COleCurrency cyRefunds = COleCurrency(0, 0);

						if (GetChargeInsuranceTotals(nCurChargeID, nCurPatientID, nLastInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
							cyHighestChargeBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
						}
						else {
							//this should be impossible, use zero if so
							ASSERT(FALSE);
							cyHighestChargeBalance = COleCurrency(0, 0);
						}
					}

					//now get the balance for the current insured party

					COleCurrency cyCharges = COleCurrency(0, 0);
					COleCurrency cyPayments = COleCurrency(0, 0);
					COleCurrency cyAdjustments = COleCurrency(0, 0);
					COleCurrency cyRefunds = COleCurrency(0, 0);

					if (GetChargeInsuranceTotals(nCurChargeID, nCurPatientID, nCurInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
						COleCurrency cyBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
						if (cyBalance > cyHighestChargeBalance) {
							//new high balance, use this insured party
							cyHighestChargeBalance = cyBalance;
							bUseThisInsuredParty = true;
						}
						//if the balance is the same and the current insured party is the one
						//that was already selected, keep using it
						else if (cyBalance == cyHighestChargeBalance && nOriginallySelectedInsuredPartyID == nCurInsuredPartyID) {
							bUseThisInsuredParty = true;
						}	
						//the balance is lower or the same and not previously selected
						else {
							bUseThisInsuredParty = false;
						}
					}
					else {
						//this should be impossible, ignore this insurance if so
						ASSERT(FALSE);
						bUseThisInsuredParty = false;
					}
				}
				else if (nLastChargeID != nCurChargeID) {
					//new charge, reset all our variables
					nLastChargeID = nCurChargeID;
					nLastInsuredPartyID = nCurInsuredPartyID;

					//reset the count to 0, increment if it is active
					nCountCurActiveResps = 0;
					if (AdoFldLong(prs, "RespTypeID") != -1) {
						nCountCurActiveResps++;
					}
					cyHighestChargeBalance = g_ccyInvalid;
				
					//cache the originally selected insured party ID
					nOriginallySelectedInsuredPartyID = VarLong(pRow->GetValue(COLUMN_INSURED_PARTY_ID), -1);

					bColorRowYellow = false;
				}

				if (bUseThisInsuredParty) {
					long nNewInsuredPartyID = AdoFldLong(prs, "InsuredPartyID");
					pRow->PutValue(COLUMN_INSURED_PARTY_ID, nNewInsuredPartyID);
					pRow->PutValue(COLUMN_INS_CO_ID, AdoFldLong(prs, "InsCoID",-1));
					pRow->PutValue(COLUMN_HCFA_GROUP_ID, AdoFldLong(prs, "HCFASetupGroupID",-1));
					pRow->PutValue(COLUMN_RESP_TYPE_ID, AdoFldLong(prs, "RespTypeID",-1));
					pRow->PutValue(COLUMN_SUBMIT_AS_PRIMARY, prs->Fields->Item["SubmitAsPrimary"]->Value);
					pRow->PutValue(COLUMN_INS_CO_NAME, _bstr_t(AdoFldString(prs, "InsCoName", "")));
					pRow->PutValue(COLUMN_HCFA_GROUP_NAME, _bstr_t(AdoFldString(prs, "HCFASetupName", "")));
				}

				//we may not be using this insured party but we still may be coloring the row
				pRow->PutCellBackColor(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? YELLOW_BACK_COLOR : NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellBackColorSel(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? YELLOW_BACK_COLOR : NXDATALIST2Lib::dlColorNotSet);
				pRow->PutCellForeColorSel(COLUMN_INSURED_PARTY_ID, bColorRowYellow ? BLACK_TEXT_COLOR : NXDATALIST2Lib::dlColorNotSet);
			}

			//always ensure these are up to date
			nLastChargeID = nCurChargeID;
			nLastInsuredPartyID = nCurInsuredPartyID;

			prs->MoveNext();
		}
		prs->Close();

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-04-25 13:55) - NX-100299 - added a query to use for payer ID matching,
// returns all valid insured parties for a given charge & selected insurance company
ADODB::_RecordsetPtr CEOBDlg::GetPayerIDMatchingRecordset(std::vector<long> &aryChargeIDs, const long &nInsuranceCoID)
{
	return CreateParamRecordset(R"(
SELECT InsuredPartyT.PatientID,
		InsuredPartyT.PersonID AS InsuredPartyID,	
		InsuranceCoT.PersonID as InsCoID,
		InsuranceCoT.Name as InsCoName,
		InsuranceCoT.HCFASetupGroupID,
		(CASE WHEN ClaimPayerIDsT.ID Is Not Null THEN ClaimPayerIDsT.EbillingID ELSE EbillingInsCoIDs.EBillingID END) as PayerID,
		InsuredPartyT.RespTypeID,
		InsuredPartyT.SubmitAsPrimary,
		HCFASetupT.Name AS HCFASetupName,
		ChargesT.ID AS ChargeID
FROM ChargesT
INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
INNER JOIN InsuredPartyT ON LineItemT.PatientID = InsuredPartyT.PatientID
INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID
INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID
LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID
LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID 
LEFT JOIN (SELECT InsuranceCoID, LocationID, ClaimPayerID 
	FROM InsuranceLocationPayerIDsT 
) AS InsuranceLocationPayerIDsQ ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsQ.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsQ.LocationID
LEFT JOIN EbillingInsCoIDs AS ClaimPayerIDsT ON InsuranceLocationPayerIDsQ.ClaimPayerID = ClaimPayerIDsT.ID
LEFT JOIN (
	SELECT InsuranceCoT.PersonID, EbillingInsCoIDs.EbillingID AS PayerID1,
	InsuranceLocationPayerIDsT.LocationID, LocationPayerIDs.EbillingID AS PayerID2
	FROM InsuranceCoT
	LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID 
	LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID 
	LEFT JOIN EbillingInsCoIDs LocationPayerIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = LocationPayerIDs.ID 
	WHERE InsuranceCoT.PersonID = {INT}
) AS SelectedInsuranceCoQ ON (
	(LineItemT.LocationID = SelectedInsuranceCoQ.LocationID AND ClaimPayerIDsT.EbillingID = SelectedInsuranceCoQ.PayerID2)
	OR
	EbillingInsCoIDs.EBillingID = SelectedInsuranceCoQ.PayerID1
)
WHERE ChargesT.ID IN ({INTVECTOR}) 
AND (InsuranceCoT.PersonID = {INT} OR SelectedInsuranceCoQ.PersonID Is Not Null)
GROUP BY InsuredPartyT.PatientID, InsuredPartyT.PersonID, InsuranceCoT.PersonID,
		InsuranceCoT.Name, InsuranceCoT.HCFASetupGroupID,
		CASE WHEN ClaimPayerIDsT.ID Is Not Null THEN ClaimPayerIDsT.EbillingID ELSE EbillingInsCoIDs.EBillingID END,
		InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, HCFASetupT.Name,
		RespTypeT.Priority, ChargesT.ID
ORDER BY InsuredPartyT.PatientID, ChargesT.ID,
(CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC
)",
nInsuranceCoID, aryChargeIDs, nInsuranceCoID);
}

// (s.tullis 2016-04-25 14:15) - NX-100185 
// Now that they can change the insured party in the datalist we need to ensure they can't post to invalid insured parties
bool CEOBDlg::EnsureNoInvalidInsuredParties()
{
	bool bNoInvalidInsuredParties = true;
	try {
		CString strPatients;
		long nCount = 0;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_EOBList->GetFirstRow(); pRow != nullptr && nCount <=10; pRow = pRow->GetNextRow()) {
			if (VarLong(pRow->GetValue(COLUMN_PATIENT_ID), -1) != -1 &&
				VarLong(pRow->GetValue(COLUMN_INSURED_PARTY_ID),-1) == -1
				&& !VarBool(pRow->GetValue(COLUMN_SKIPPED))) {
				bNoInvalidInsuredParties = false;
				nCount++;
				if (nCount > 10) {
					strPatients += "<more...>";
					break;
				}
				strPatients += VarString(pRow->GetValue(COLUMN_PATIENT_NAME)) + "\n" ;
			}
		}

		if (!bNoInvalidInsuredParties) {
			CString strWarning = "The following patients have no insured party selected to post to. Please ensure there is a valid insured party selected.\n"
				"If the patient does not have a valid insured party, check the “skip” checkbox on that line item and post the payment manually.\n\n" + strPatients;
			MessageBox(strWarning, "Practice", MB_OK | MB_ICONQUESTION);
		}
		
	}NxCatchAll(__FUNCTION__)
		return bNoInvalidInsuredParties;
}
	
// (j.jones 2016-04-26 14:08) - NX-100327 - added function to get the total adjustments on a charge,
// which may skip secondary adjustments
void CEOBDlg::GetTotalChargeAdjustments(NXDATALIST2Lib::IRowSettingsPtr pCurRow,
	OUT COleCurrency &cyTotalPositiveAdjustments, 
	OUT COleCurrency &cyTotalNegativeAdjustments, 
	OUT COleCurrency &cyTotalAdjustments)
{
	cyTotalPositiveAdjustments = COleCurrency(0, 0);
	cyTotalNegativeAdjustments = COleCurrency(0, 0);
	cyTotalAdjustments = COleCurrency(0, 0);
	
	if (pCurRow == NULL) {
		//this function should not have been called
		ASSERT(FALSE);
		return;
	}

	if (m_ERemitType != ertAnsi) {
		//for Canada, just return the datalist values
		cyTotalPositiveAdjustments = VarCurrency(pCurRow->GetValue(COLUMN_TOTAL_POSITIVE_ADJUSTMENTS), COleCurrency(0, 0));
		cyTotalAdjustments = VarCurrency(pCurRow->GetValue(COLUMN_TOTAL_ADJUSTMENTS), COleCurrency(0, 0));
		cyTotalNegativeAdjustments = cyTotalPositiveAdjustments - cyTotalAdjustments;
		return;
	}

	EOBLineItemInfo *pCharge = NULL;
	_variant_t var = pCurRow->GetValue(COLUMN_CHARGE_PTR);

	if (var.vt == VT_I4) {
		pCharge = (EOBLineItemInfo*)VarLong(var);
	}	

	//first check if we need to skip all adjustments due to being secondary
	BOOL bIgnoreSecondaryAdjs = (GetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", 1, 0, "<None>", true) == 1);
	//this is for the hardcoded Secondary Medical only
	long nRespTypeID = VarLong(pCurRow->GetValue(COLUMN_RESP_TYPE_ID));
	if (bIgnoreSecondaryAdjs && nRespTypeID == 2) {
		//skip all adjustments, this is secondary
		cyTotalPositiveAdjustments = COleCurrency(0, 0);
		cyTotalNegativeAdjustments = COleCurrency(0, 0);
		cyTotalAdjustments = COleCurrency(0, 0);
		return;
	}

	if (pCharge) {
		for (int i = 0; i < pCharge->arypEOBAdjustmentInfo.GetSize(); i++) {
			const EOBAdjustmentInfo *pAdj = pCharge->arypEOBAdjustmentInfo[i];
			if (!pAdj->bPatResp) {

				cyTotalAdjustments += pAdj->cyAdjustmentAmt;

				if (pAdj->cyAdjustmentAmt > COleCurrency(0, 0)) {
					cyTotalPositiveAdjustments += pAdj->cyAdjustmentAmt;
				}
				else if (pAdj->cyAdjustmentAmt < COleCurrency(0, 0)) {
					cyTotalNegativeAdjustments += pAdj->cyAdjustmentAmt;
				}
			}
		}
	}
}