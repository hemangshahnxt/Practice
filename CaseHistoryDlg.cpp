// CaseHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaseHistoryDlg.h"
#include "DocumentOpener.h"
#include "AuditTrail.h"
#include "CaseHistorySelectDetailDlg.h"
#include "CaseHistorySelectSurgeryDlg.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "ProcessRptDlg.h"
#include "ProductItemsDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "SelectApptDlg.h"
#include "SchedulerView.h"
#include "GlobalReportUtils.h"
#include "GlobalFinancialUtils.h"
#include "BarCode.h"
#include "MultiSelectDlg.h"
#include "DontShowDlg.h"
#include "EditDischargeStatusDlg.h"
#include "InvPatientAllocationDlg.h"
#include "InvAllocationUsageDlg.h"
#include "selectdlg.h"
#include "SelectLinkedProductsDlg.h"
#include "MultiSelectDlg.h"
#include "OHIPUtils.h"
#include "DiagSearchUtils.h"
#include "GlobalStringUtils.h"

using namespace ADODB;
using namespace NXTIMELib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.wilson 02/20/2014) PLID 60774 - enum for diagnosis list columns.
enum DiagnosisCodeListColumns {
	dclcIndex = 0,
	dclcDiagICD9CodeID,
	dclcDiagICD9Code,
	dclcDiagICD9Desc,
	dclcDiagICD10CodeID,
	dclcDiagICD10Code,
	dclcDiagICD10Desc,
};

//const _variant_t g_cvarTrue(VARIANT_TRUE, VT_BOOL);
//const _variant_t g_cvarFalse(VARIANT_FALSE, VT_BOOL);

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum ECaseHistoryDlgProvidersComboColumns
{
	chdpccID = 0,
	chdpccName = 1,
};

enum ECaseHistoryDlgLocationsComboColumns
{
	chdlccID = 0,
	chdlccName = 1,
};

enum ECaseHistoryDetailListColumns
{
	chdlcID = 0,
	chdlcAction,	
	chdlcItemID,
	chdlcItemType,
	chdlcName,
	chdlcQuantity,
	chdlcAmount,
	chdlcVisibleCost,
	chdlcTrueCost,				// (j.jones 2008-07-01 09:49) - PLID 18744
	chdlcBillable,
	// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
	//chdlcPayToPractice,
	chdlcCHDProductItemListID,
	chdlcIsSerializedProduct,	// (j.jones 2008-05-20 17:25) - PLID 29249
	chdlcIsLinkedToProducts,	//TES 7/16/2008 - PLID 27983
};

// (j.jones 2008-02-26 16:10) - PLID 29102 - added allocation combo
enum EInvAllocationComboColumns
{
	iaccID = 0,
	iaccInputDate,
	iaccStatus,
	iaccStatusName,
	iaccCompletionDate,
	iaccFirstProductName,
	iaccTotalQuantity,
};

enum DiagListColumns
{
	dxColID = 0,
	dxColCode = 1,
	dxColDesc = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CCaseHistoryDlg dialog


CCaseHistoryDlg::CCaseHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCaseHistoryDlg::IDD, pParent)
{
	m_bIsScanningBarcode = FALSE;
	//{{AFX_DATA_INIT(CCaseHistoryDlg)
	m_pParent = NULL;
	m_nCaseHistoryID = -1;
	m_bIsNew = TRUE;
	m_nPersonID = -1;
	m_nDefaultProviderID = -1;
	m_nCurLocationID = -1;
	m_nAppointmentID = -1;
	m_bIsModified = FALSE;
	m_bHasProcedureBeenChanged = FALSE;
	m_CurAnesthMinutes = 0;		
	m_CurFacilityMinutes = 0;
	m_nLastSavedProviderID = -1;
	m_nLastSavedLocationID = -1;
	m_nLastSavedPOSID = -1;
	m_nLastSavedAppointmentID = -1;
	m_bLastSavedCompleted = FALSE;
	m_bLastSavedDetailsLoaded = false;
	m_nLastSavedAnesthesiaMinutes = -1;
	m_nLastSavedFacilityMinutes = -1;
	m_nLastSavedDischargeStatusID = -1;
	m_nLastSavedAllocationID = -1;
	m_strLastAnesType = "";
	// (a.wilson 2014-02-24 17:46) - PLID 60774
	m_bPreOpDiagCodesChanged = false;
	m_bPostOpDiagCodesChanged = false;
	//}}AFX_DATA_INIT

	// (a.walling 2007-02-23 11:44) - PLID 24451 - Fixing this so classwizard doesn't bomb
	m_varCompletedDate.vt = VT_NULL;
	m_dtLastSavedSurgeryDate.SetStatus(COleDateTime::invalid);
	m_dtLastSavedPreOpIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedPreOpOut.SetStatus(COleDateTime::invalid);
	m_dtLastSavedSurgeonIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedSurgeonOut.SetStatus(COleDateTime::invalid);
	m_dtLastSavedOpRoomIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedOpRoomOut.SetStatus(COleDateTime::invalid);
	m_dtLastSavedRecoveryIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedRecoveryOut.SetStatus(COleDateTime::invalid);
	m_dtLastSavedAnesthesiaIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedAnesthesiaOut.SetStatus(COleDateTime::invalid);
	m_dtLastSavedFacilityIn.SetStatus(COleDateTime::invalid);
	m_dtLastSavedFacilityOut.SetStatus(COleDateTime::invalid);
	m_dtLastSaved23HourRoomIn.SetStatus(COleDateTime::invalid);
	m_dtLastSaved23HourRoomOut.SetStatus(COleDateTime::invalid);

	// (a.walling 2007-02-27 14:53) - PLID 24944 - Help prevent redundant message boxes
	m_dtLastEnteredAnesthesiaIn.SetStatus(COleDateTime::invalid);
	m_dtLastEnteredFacilityIn.SetStatus(COleDateTime::invalid);
	m_dtLastEnteredAnesthesiaOut.SetStatus(COleDateTime::invalid);
	m_dtLastEnteredFacilityOut.SetStatus(COleDateTime::invalid);


	m_rcMultiProcedureLabel.top = m_rcMultiProcedureLabel.bottom = m_rcMultiProcedureLabel.left = m_rcMultiProcedureLabel.right = 0;
}

CCaseHistoryDlg::~CCaseHistoryDlg() {
	DeleteAllFromCaseHistoryDetailProductItemsArray();
}


void CCaseHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaseHistoryDlg)
	DDX_Control(pDX, IDC_COMPLETED_CHECK, m_btnCompleted);
	DDX_Control(pDX, IDC_BTN_CREATE_ALLOCATION, m_btnCreateInvAllocation);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_SURGEON, m_btnAutoTime_Out_Surgeon);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_RECOVERY, m_btnAutoTime_Out_Recovery);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_PREOP, m_btnAutoTime_Out_PreOp);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_OPROOM, m_btnAutoTime_Out_OpRoom);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_FAC, m_btnAutoTime_Out_Fac);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_ANES, m_btnAutoTime_Out_Anes);
	DDX_Control(pDX, IDC_AUTOTIME_OUT_23, m_btnAutoTime_Out_23);
	DDX_Control(pDX, IDC_AUTOTIME_IN_SURGEON, m_btnAutoTime_In_Surgeon);
	DDX_Control(pDX, IDC_AUTOTIME_IN_RECOVERY, m_btnAutoTime_In_Recovery);
	DDX_Control(pDX, IDC_AUTOTIME_IN_PREOP, m_btnAutoTime_In_PreOp);
	DDX_Control(pDX, IDC_AUTOTIME_IN_OPROOM, m_btnAutoTime_In_OpRoom);
	DDX_Control(pDX, IDC_AUTOTIME_IN_FAC, m_btnAutoTime_In_Fac);
	DDX_Control(pDX, IDC_AUTOTIME_IN_ANES, m_btnAutoTime_In_Anes);
	DDX_Control(pDX, IDC_AUTOTIME_IN_23, m_btnAutoTime_In_23);
	DDX_Control(pDX, IDC_SURGERY_DATE, m_ctrlSurgeryDate);
	DDX_Control(pDX, IDC_NAME_EDIT, m_nxeditNameEdit);
	DDX_Control(pDX, IDC_APPOINTMENT_DESC, m_nxeditAppointmentDesc);
	DDX_Control(pDX, IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, m_nxeditEditTotalCaseAnesthMinutes);
	DDX_Control(pDX, IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, m_nxeditEditTotalCaseFacilityMinutes);
	DDX_Control(pDX, IDC_NOTES_EDIT, m_nxeditNotesEdit);
	DDX_Control(pDX, IDC_MULTI_PROCEDURE_LABEL, m_nxstaticMultiProcedureLabel);
	DDX_Control(pDX, IDC_ALLOCATION_SELECT_LABEL, m_nxstaticAllocationSelectLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_BTN, m_btnPrint);
	DDX_Control(pDX, IDC_BTN_CHANGE_APPT, m_btnChangeAppt);
	DDX_Control(pDX, IDC_BTN_CLEAR_APPT, m_btnClearAppt);
	DDX_Control(pDX, IDC_BTN_GOTO_APPT, m_btnGotoAppt);
	DDX_Control(pDX, IDC_PREOP_LINE, m_btnPreopLine);
	DDX_Control(pDX, IDC_BTN_EDIT_DISCHARGE_STATUS, m_btnDischargeStatus);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CCaseHistoryDlg, IDC_SURGERY_DATE, 2 /* Change */, OnChangeSurgeryDate, VTS_NONE)

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

BEGIN_MESSAGE_MAP(CCaseHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCaseHistoryDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SURGERY_DATE, OnChangeSurgeryDate)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CASEHISTORYDETAILS_ADD, OnCasehistorydetailsAdd)
	ON_COMMAND(ID_CASEHISTORYDETAILS_REMOVE, OnCasehistorydetailsRemove)
	ON_COMMAND(ID_CASEHISTORYDETAILS_EDITPRODITEMS, OnEditProductItems)
	ON_BN_CLICKED(IDC_PRINT_BTN, OnPrintBtn)
	ON_EN_CHANGE(IDC_NOTES_EDIT, OnChangeNotesEdit)
	ON_EN_CHANGE(IDC_NAME_EDIT, OnChangeNameEdit)
	ON_BN_CLICKED(IDC_COMPLETED_CHECK, OnCompletedCheck)
	ON_BN_CLICKED(IDC_BTN_CHANGE_APPT, OnBtnChangeAppt)
	ON_BN_CLICKED(IDC_BTN_CLEAR_APPT, OnBtnClearAppt)
	ON_BN_CLICKED(IDC_BTN_GOTO_APPT, OnBtnGotoAppt)
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, OnChangeEditTotalCaseAnesthMinutes)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, OnChangeEditTotalCaseFacilityMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, OnKillfocusEditTotalCaseAnesthMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, OnKillfocusEditTotalCaseFacilityMinutes)
	ON_BN_CLICKED(IDC_BTN_EDIT_DISCHARGE_STATUS, OnBtnEditDischargeStatus)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_23, OnAutotimeIn23)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_ANES, OnAutotimeInAnes)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_FAC, OnAutotimeInFac)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_OPROOM, OnAutotimeInOproom)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_PREOP, OnAutotimeInPreop)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_RECOVERY, OnAutotimeInRecovery)
	ON_BN_CLICKED(IDC_AUTOTIME_IN_SURGEON, OnAutotimeInSurgeon)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_23, OnAutotimeOut23)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_ANES, OnAutotimeOutAnes)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_FAC, OnAutotimeOutFac)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_OPROOM, OnAutotimeOutOproom)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_PREOP, OnAutotimeOutPreop)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_RECOVERY, OnAutotimeOutRecovery)
	ON_BN_CLICKED(IDC_AUTOTIME_OUT_SURGEON, OnAutotimeOutSurgeon)
	ON_BN_CLICKED(IDC_BTN_CREATE_ALLOCATION, OnBtnCreateAllocation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaseHistoryDlg message handlers

BOOL CCaseHistoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-05-01 10:00) - PLID 29863 - Assign standard CNxIconButton styles
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnPrint.AutoSet(NXB_PRINT_PREV);
		m_btnCreateInvAllocation.AutoSet(NXB_OK);
		m_btnChangeAppt.AutoSet(NXB_MODIFY);
		m_btnClearAppt.AutoSet(NXB_MODIFY);
		m_btnGotoAppt.AutoSet(NXB_OK);

		// (j.jones 2006-05-10 09:11) - PLID 20510 - Load all common Case History properties into the
		// NxPropManager cache
		g_propManager.CachePropertiesInBulk("CaseHistoryDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'CredentialWarnCaseHistories' OR "
			"Name = 'ExpLicenseWarnCaseHistories' OR "
			"Name = 'DefaultASCLicenseWarnDayRange' OR "
			"Name = 'CredentialWarningOnlyTOS2' OR "
			"Name = 'ChargeAllowQtyIncrementCaseHistory' OR "
			"Name = 'ApptAutoOpenResEntry' OR "
			"Name = 'BarcodeAllowQtyIncrementCaseHistory' OR "
			// (j.jones 2009-08-26 10:25) - PLID 34943 - this won't cache in time for
			// OpenNewCase() to benefit from it, but will help when we change appts.
			"Name = 'DefaultCaseHistoryLocation' "
			")",
			_Q(GetCurrentUserName()));

		m_nxtAnesthStart = BindNxTimeCtrl(this, IDC_ANESTHESIA_START_EDIT);
		m_nxtAnesthEnd = BindNxTimeCtrl(this, IDC_ANESTHESIA_STOP_EDIT);
		m_nxtFacilityStart = BindNxTimeCtrl(this, IDC_FACILITY_START_EDIT);
		m_nxtFacilityEnd = BindNxTimeCtrl(this, IDC_FACILITY_STOP_EDIT);

		if(!m_pParent) {
			//if not modeless, remove the minimize box
			ModifyStyle(WS_MINIMIZEBOX,0);
		}

		m_dlProviders = BindNxDataListCtrl(this, IDC_PROVIDER_LIST, GetRemoteData(), true);
		m_dlLocations = BindNxDataListCtrl(this, IDC_CASE_HISTORY_LOCATION_COMBO, GetRemoteData(), true);
		m_dlPOS = BindNxDataListCtrl(this, IDC_CASE_HISTORY_POS_COMBO, GetRemoteData(), true);
		m_dlProcedures = BindNxDataListCtrl(this, IDC_PROCEDURE_LIST, GetRemoteData(), true);
		// (j.jones 2006-11-24 09:24) - PLID 23372 - added discharge status
		m_DischargeStatusCombo = BindNxDataList2Ctrl(this, IDC_DISCHARGE_STATUS_COMBO, GetRemoteData(), true);

		// (j.jones 2008-02-26 13:59) - PLID 29102 - added allocation combo
		m_InvAllocationCombo = BindNxDataList2Ctrl(this, IDC_CASE_ALLOCATION_LIST, GetRemoteData(), false);

		// (j.jones 2008-03-10 14:38) - PLID 29233 - added allocation contents list
		m_InvAllocationContentsList = BindNxDataList2Ctrl(this, IDC_INV_ALLOCATION_CONTENTS_LIST, GetRemoteData(), false);
		
		m_dlCaseHistoryDetails = BindNxDataListCtrl(this, IDC_CASEHISTORY_DETAILS_LIST, GetRemoteData(), false);

		m_dlCaseHistoryDetails->GridVisible = TRUE;
		
		{
			NXDATALISTLib::IColumnSettingsPtr pCol = m_dlCaseHistoryDetails->GetColumn(chdlcAction);
			pCol->PutForeColor(RGB(0, 0, 255));
			pCol->PutBackColorSel(RGB(255, 255, 255));
			pCol->PutForeColorSel(RGB(0, 0, 255));
		}

		m_dlProcedures->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);

		NXDATALISTLib::IRowSettingsPtr pRow = m_dlProcedures->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1," {Multiple Procedures}");	
		m_dlProcedures->InsertRow(pRow, 0);

		pRow = m_dlProcedures->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1," {No Procedure}");	
		m_dlProcedures->InsertRow(pRow, 0);

		// (j.jones 2008-02-26 16:06) - PLID 29102 - hide the allocation combo if they do not have the license
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//disable the "create" button if no permissions
			if(!(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS)) {
				GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->EnableWindow(FALSE);
			}
		}
		else {
			//no license, hide the controls
			GetDlgItem(IDC_ALLOCATION_SELECT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CASE_ALLOCATION_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->ShowWindow(SW_HIDE);
			
			// (j.jones 2008-03-10 13:39) - PLID 29233 - need to hide the allocation list,
			// but also resize the notes control to be wider
			GetDlgItem(IDC_INV_ALLOCATION_CONTENTS_LIST)->ShowWindow(SW_HIDE);
			CRect rcList, rcNotes;
			GetDlgItem(IDC_INV_ALLOCATION_CONTENTS_LIST)->GetWindowRect(&rcList);
			ScreenToClient(rcList);
			GetDlgItem(IDC_NOTES_EDIT)->GetWindowRect(&rcNotes);
			ScreenToClient(rcNotes);
			rcNotes.right = rcList.right;
			GetDlgItem(IDC_NOTES_EDIT)->MoveWindow(&rcNotes);
		}

		// Calculate hyperlink rectangles
		{
			CWnd *pWnd;

			pWnd = GetDlgItem(IDC_MULTI_PROCEDURE_LABEL);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the is hotlinks
				pWnd->GetWindowRect(m_rcMultiProcedureLabel);
				ScreenToClient(&m_rcMultiProcedureLabel);

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
		}

		GetMainFrame()->RegisterForBarcodeScan(this);

		// (j.gruber 2008-07-07 17:27) - PLID 15807 - anesthesia type and diag code drop downs
		m_pAnesType = BindNxDataList2Ctrl(IDC_CASE_HISTORY_ANES_TYPE, true);

		// (a.wilson 02/19/2014) PLID 60774
		m_pPreOpDiagnosisSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_PREOP_DIAGNOSIS_SEARCH, GetRemoteData());
		m_pPostOpDiagnosisSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_POSTOP_DIAGNOSIS_SEARCH, GetRemoteData());
		m_pPreOpDiagnosisList = BindNxDataList2Ctrl(IDC_PREOP_DIAGNOSIS_LIST, false);
		m_pPostOpDiagnosisList = BindNxDataList2Ctrl(IDC_POSTOP_DIAGNOSIS_LIST, false);

		// (j.jones 2006-12-13 16:00) - PLID 23578 - supported ability to create a new
		// case history from within this dialog
		if(!m_bIsNew || m_nCaseHistoryID != -1) {
			//load existing case
			Load(m_nCaseHistoryID);
		}
		else {
			//create new case from a preference card
			// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
			// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
			InitializeFromPreferenceCards(m_arynDefaultPreferenceCardIDs);

			// (j.jones 2008-02-26 16:06) - PLID 29102 - now requery the allocation combo (if they have the license)
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.5
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
				CString strWhere = "";
				if(m_nPersonID != -1) {
					//we'll still run this same filter even if m_nCaseHistoryID is -1
					strWhere.Format("PatientID = %li "
						"AND ID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID <> %li) "
						"AND LocationID = %li", m_nPersonID, m_nCaseHistoryID, m_nCurLocationID);
				}

				if(!strWhere.IsEmpty()) {
					m_InvAllocationCombo->PutWhereClause(_bstr_t(strWhere));
					m_InvAllocationCombo->Requery();

					// (j.jones 2008-03-10 14:40) - PLID 29233 - clear the allocation contents list
					m_InvAllocationContentsList->Clear();
				}
			}
			// (a.wilson 2014-02-21 15:56) - PLID 60774
			for (int i = 1; i <= 4; i++) {
				NXDATALIST2Lib::IRowSettingsPtr pPreOpRow = m_pPreOpDiagnosisList->GetNewRow();
				NXDATALIST2Lib::IRowSettingsPtr pPostOpRow = m_pPostOpDiagnosisList->GetNewRow();
				pPreOpRow->PutValue(dclcIndex, i);
				pPreOpRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
				pPreOpRow->PutValue(dclcDiagICD9Code, g_cvarNull);
				pPreOpRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
				pPreOpRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
				pPreOpRow->PutValue(dclcDiagICD10Code, g_cvarNull);
				pPreOpRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
				pPostOpRow->PutValue(dclcIndex, i);
				pPostOpRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
				pPostOpRow->PutValue(dclcDiagICD9Code, g_cvarNull);
				pPostOpRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
				pPostOpRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
				pPostOpRow->PutValue(dclcDiagICD10Code, g_cvarNull);
				pPostOpRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
				m_pPreOpDiagnosisList->AddRowAtEnd(pPreOpRow, NULL);
				m_pPostOpDiagnosisList->AddRowAtEnd(pPostOpRow, NULL);
			}
			UpdateDiagnosisListColumnSizes();
		}

		const int NUM_AUTOTIMES = 14;
		CNxIconButton* pButtons[NUM_AUTOTIMES] = {
			&m_btnAutoTime_Out_Surgeon,
			&m_btnAutoTime_Out_Recovery,
			&m_btnAutoTime_Out_PreOp,
			&m_btnAutoTime_Out_OpRoom,
			&m_btnAutoTime_Out_Fac,
			&m_btnAutoTime_Out_Anes,
			&m_btnAutoTime_Out_23,
			&m_btnAutoTime_In_Surgeon,
			&m_btnAutoTime_In_Recovery,
			&m_btnAutoTime_In_PreOp,
			&m_btnAutoTime_In_OpRoom,
			&m_btnAutoTime_In_Fac,
			&m_btnAutoTime_In_Anes,
			&m_btnAutoTime_In_23 };


		for (int i = 0; i < NUM_AUTOTIMES; i++) {
			pButtons[i]->AutoSet(NXB_TIME);
			pButtons[i]->SetToolTip("Set to current time");
		}

	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCaseHistoryDlg::SetDlgItemTime(UINT nID, const COleDateTime &dtTime, BOOL bDontSetIfMidnight)
{
	CWnd *pWnd = GetDlgItem(nID);
	if (pWnd) {
		_DNxTimePtr pTime = BindNxTimeCtrl(this, nID);
		if(dtTime.m_status == COleDateTime::invalid) {
			pTime->Clear();
		}
		else {
			// (a.walling 2007-02-27 11:28) - PLID 24944 - Strip seconds when setting the time.
			if (dtTime.m_dt == 0.0) {
				if (bDontSetIfMidnight) {
					// Don't set
					pTime->Clear();
				} else {
					pTime->SetDateTime(StripSeconds(dtTime));
				}
			} else {
				pTime->SetDateTime(StripSeconds(dtTime));
			}
		}
	}
}

void CCaseHistoryDlg::SetDlgItemTime(UINT nID, const CString &strTime, BOOL bDontSetIfMidnight)
{
	COleDateTime dt;
	dt.ParseDateTime(strTime);
	SetDlgItemTime(nID, dt, bDontSetIfMidnight);
}

COleDateTime CCaseHistoryDlg::GetDlgItemTime(UINT nID)
{
	CWnd *pWnd = GetDlgItem(nID);
	if (pWnd) {
		_DNxTimePtr pTime = BindNxTimeCtrl(this, nID);
		if(pTime->GetStatus() == 1) {
			return StripSeconds(pTime->GetDateTime());
		}
		else {
			COleDateTime dt;
			dt.SetStatus(COleDateTime::invalid);
			return StripSeconds(dt);
		}
	} else {
		COleDateTime dt;
		dt.SetStatus(COleDateTime::invalid);
		return dt;
	}
}

// (j.jones 2006-12-13 16:09) - PLID 23578 - load new info. from a surgery
// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
void CCaseHistoryDlg::InitializeFromPreferenceCards(CArray<long, long> &arynPreferenceCardIDs)
{
	if (m_bIsModified) {
		AfxThrowNxException("CCaseHistoryDlg::Load: The current case history has been modified.  A case history cannot be loaded over one that has been modified.");
	}
	if (!m_hWnd) {
		AfxThrowNxException("CCaseHistoryDlg::Load: The dialog does not exist and can't be loaded.");
	}

	//first load the information given to us by the "default" values

	SetDlgItemText(IDC_NAME_EDIT, m_strDefaultName);

	CString strTitle;
	strTitle.Format("Case History for %s",GetExistingPatientName(m_nPersonID));
	SetWindowText(strTitle);

	DisplayAppointmentDesc();

	CheckDlgButton(IDC_COMPLETED_CHECK, 0);

	//do nothing if the provider is inactive
	m_dlProviders->SetSelByColumn(chdpccID, m_nDefaultProviderID);

	//do nothing if the location is inactive
	m_dlLocations->SetSelByColumn(chdlccID, m_nCurLocationID);
	m_dlPOS->SetSelByColumn(chdlccID, m_nCurLocationID);

	m_ctrlSurgeryDate.SetValue(_variant_t(m_dtDefaultSurgeryDate, VT_DATE));

	// Load the procedures from the appointment
	//TES 5/6/2008 - PLID 29931 - Make sure not to load any non-procedural purposes.
	m_arProcedureIDs.RemoveAll();
	// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
	// (c.haag 2008-12-26 11:29) - PLID 32539 - I changed no code here except to make the query
	// not run if m_nAppointmentID is not positive. I want to stress that we must include inactive
	// procedures here. This query tries to build a procedure list from an appointment's purpose 
	// list. If an appointment was assigned a procedure that was later inactivated; then it would
	// not show up here with an inactive procedure filter. This can cause confusion; and worst case,
	// a case history for a scheduled procedure is never made.
	if(m_nAppointmentID > 0) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT "
			"INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
			"WHERE AppointmentID = {INT}", m_nAppointmentID);
		while(!rs->eof) {
			m_arProcedureIDs.Add(AdoFldLong(rs, "PurposeID"));
			rs->MoveNext();
		}
		rs->Close();
	}
	// (j.jones 2009-09-01 10:03) - PLID 35380 - If there is no appointment, then load the procedures
	// from the linked case histories. Even if the appointment had no procedures, we still will only
	// do this when we are not creating from an appointment.
	else {
		//if there is one procedure, silently use it, but if there are multiple procedures,
		//give a multi-select list
		CString strPrefCardIDs;
		for(int i=0; i<arynPreferenceCardIDs.GetSize(); i++) {
			long nPrefCardID = (long)arynPreferenceCardIDs.GetAt(i);
			if(!strPrefCardIDs.IsEmpty()) {
				strPrefCardIDs += ",";
			}
			strPrefCardIDs += AsString(nPrefCardID);
		}
		if(!strPrefCardIDs.IsEmpty()) {
			//cannot be parameterized
			_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID "
				"FROM PreferenceCardProceduresT "
				"INNER JOIN ProcedureT ON PreferenceCardProceduresT.ProcedureID = ProcedureT.ID "
				"WHERE PreferenceCardID IN (%s) "
				"AND ProcedureT.Inactive = 0 "
				"GROUP BY ProcedureID", strPrefCardIDs);
			if(!rs->eof) {
				if(rs->GetRecordCount() == 1) {
					//just use the one procedure we found
					m_arProcedureIDs.Add(AdoFldLong(rs, "ProcedureID"));
				}
				else {
					// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
					CMultiSelectDlg dlgMulti(this, "ProcedureT");
					CString strWhere;
					strWhere.Format("PreferenceCardID IN (%s) AND ProcedureT.Inactive = 0", strPrefCardIDs);
					//they are allowed to select nothing, if they wish
					if(dlgMulti.Open("PreferenceCardProceduresT INNER JOIN ProcedureT ON PreferenceCardProceduresT.ProcedureID = ProcedureT.ID",
						strWhere, "ProcedureT.ID", "ProcedureT.Name", "Select the procedures to be used on this case history:") == IDOK) {
						
						dlgMulti.FillArrayWithIDs(m_arProcedureIDs);
					}
				}
			}
			rs->Close();
		}
	}
	DisplayProcedureInfo();

	//now load the preference card information

	// (j.jones 2007-02-20 11:36) - PLID 23994 - we might not have a surgery ID
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
	for(int i=0; i<arynPreferenceCardIDs.GetSize(); i++) {
		AddPreferenceCard(arynPreferenceCardIDs.GetAt(i));
	}

	//and lastly, add the "add" row to the end

	_variant_t varNull;
	varNull.vt = VT_NULL;

	NXDATALISTLib::IRowSettingsPtr pRow = m_dlCaseHistoryDetails->GetRow(-1);
	pRow->PutValue(chdlcID, (long)MAXLONG);
	pRow->PutValue(chdlcAction, _bstr_t("Add"));
	pRow->PutValue(chdlcQuantity, varNull);
	pRow->PutValue(chdlcItemID, varNull);
	pRow->PutValue(chdlcItemType, varNull);
	pRow->PutValue(chdlcName, varNull);
	pRow->PutValue(chdlcCHDProductItemListID, varNull);
	// (j.jones 2008-05-20 17:26) - PLID 29249 - added the chdlcIsSerializedProduct column
	pRow->PutValue(chdlcIsSerializedProduct, varNull);
	//TES 7/16/2008 - PLID 27983 - Added the IsLinkedToProducts column
	pRow->PutValue(chdlcIsLinkedToProducts, varNull);

	pRow->PutBackColor(PaletteColor(GetSysColor(COLOR_WINDOW)));
	pRow->PutBackColorSel(PaletteColor(GetSysColor(COLOR_WINDOW)));

	m_dlCaseHistoryDetails->InsertRow(pRow, m_dlCaseHistoryDetails->GetRowCount());

	m_bIsModified = TRUE;
}

// (a.walling 2007-02-27 10:14) - PLID 24451 - Strip seconds here when loading, so we don't audit and change
// every time when the dialog is opened.
#define LOAD_LOG_TIME(strFieldName, dtVariable, nCtrlID)	varTime = flds->GetItem(strFieldName)->Value;\
			if(varTime.vt == VT_DATE) dtVariable = StripSeconds(VarDateTime(varTime));\
			SetDlgItemTime(nCtrlID, AsString(varTime), FALSE);

void CCaseHistoryDlg::Load(long nCaseHistoryID)
{
	if (m_bIsModified) {
		AfxThrowNxException("CCaseHistoryDlg::Load: The current case history has been modified.  A case history cannot be loaded over one that has been modified.");
	}
	if (!m_hWnd) {
		AfxThrowNxException("CCaseHistoryDlg::Load: The dialog does not exist and can't be loaded.");
	}

	// Leave this at -1 while we're loading, in case anything throws an exception
	m_nCaseHistoryID = -1;

	// Load
	{
		// Load the case history properties
		{
			// (j.jones 2008-02-27 09:34) - PLID 29102 - added allocation link information
			// (j.gruber 2008-07-07 17:34) - PLID 15807 - added anesthesia type and diagnosiscodes
			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT Name, CompletedDate, ProviderID, LocationID, PlaceOfServiceID, SurgeryDate, "
				"LogPreOpIn, LogPreOpOut, LogOpRoomIn, LogOpRoomOut, "
				"LogRecoveryIn, LogRecoveryOut, LogAnesthesiaIn, LogAnesthesiaOut, "
				"LogFacilityIn, LogFacilityOut, "
				"LogSurgeonIn, LogSurgeonOut, Log23HourRoomIn, Log23HourRoomOut, "
				"AnesthMinutes, FacilityMinutes, "
				"Notes, PersonID, AppointmentID, DischargeStatusID, "
				"CaseHistoryAllocationLinkT.AllocationID, AnesType "
				"FROM CaseHistoryT "
				"LEFT JOIN CaseHistoryAllocationLinkT ON CaseHistoryT.ID = CaseHistoryAllocationLinkT.CaseHistoryID "
				"WHERE CaseHistoryT.ID = {INT}", 
				nCaseHistoryID);

			if(prs->eof) {
				MessageBox("This Case History cannot be found. It is possible it has been deleted.");
				// (a.walling 2008-02-21 10:58) - PLID 29043 - Unregister for barcode scans
				GetMainFrame()->UnregisterForBarcodeScan(this);
				EndDialog(IDCANCEL);
			}

			FieldsPtr flds = prs->GetFields();
			
			// Load the name
			m_strLastSavedName = AdoFldString(flds, "Name");
			SetDlgItemText(IDC_NAME_EDIT, m_strLastSavedName);

			m_nPersonID = AdoFldLong(flds, "PersonID");
			CString strTitle;
			strTitle.Format("Case History for %s",GetExistingPatientName(m_nPersonID));
			SetWindowText(strTitle);
			
			m_nAppointmentID = AdoFldLong(flds, "AppointmentID", -1);
			m_nLastSavedAppointmentID = m_nAppointmentID;
			DisplayAppointmentDesc();

			// Load the completed date
			m_varCompletedDate = flds->GetItem("CompletedDate")->GetValue();
			CheckDlgButton(IDC_COMPLETED_CHECK, (m_varCompletedDate.vt != VT_NULL) ? 1 : 0);
			m_bLastSavedCompleted = m_varCompletedDate.vt != VT_NULL;

			// (j.jones 2008-02-27 16:41) - PLID 29108 - disable the allocation creation button if completed
			if(m_bLastSavedCompleted) {
				GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->EnableWindow(FALSE);
			}
			
			// Load the provider
			m_nLastSavedProviderID = AdoFldLong(flds, "ProviderID");
			if(m_dlProviders->SetSelByColumn(chdpccID, m_nLastSavedProviderID) == -1) {
				//they may have an inactive provider
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", AdoFldLong(flds, "ProviderID", -1));
				if(!rsProv->eof) {
					m_strLastSavedProviderName = AdoFldString(rsProv, "Name", "");
					m_dlProviders->PutComboBoxText(_bstr_t(m_strLastSavedProviderName));
				}
				else 
					m_dlProviders->PutCurSel(-1);
			}
			else {
				m_strLastSavedProviderName = VarString(m_dlProviders->GetValue(m_dlProviders->CurSel, chdpccName));
			}

			// Load the location
			m_nCurLocationID = AdoFldLong(flds, "LocationID");
			m_nLastSavedLocationID = m_nCurLocationID;
			if(m_dlLocations->SetSelByColumn(chdlccID, m_nCurLocationID) == -1) {
				//they must have an inactive location selected.
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nCurLocationID);
				if(!rsLocation->eof) {
					m_strLastSavedLocationName = AdoFldString(rsLocation, "Name", "");
					m_dlLocations->PutComboBoxText(_bstr_t(m_strLastSavedLocationName));
				}
				rsLocation->Close();
			}
			else {
				m_strLastSavedLocationName = VarString(m_dlLocations->GetValue(m_dlLocations->CurSel, chdlccName));
			}

			// Load the POS
			m_nLastSavedPOSID = AdoFldLong(flds, "PlaceOfServiceID");
			if(m_dlPOS->SetSelByColumn(chdlccID, m_nLastSavedPOSID) == -1) {
				//they must have an inactive POS selected.
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", AdoFldLong(flds, "PlaceOfServiceID"));
				if(!rsLocation->eof) {
					m_strLastSavedPOSName = AdoFldString(rsLocation, "Name", "");
					m_dlPOS->PutComboBoxText(_bstr_t(m_strLastSavedPOSName));
				}
				rsLocation->Close();
			}
			else {
				m_strLastSavedPOSName = VarString(m_dlPOS->GetValue(m_dlPOS->CurSel, chdlccName));
			}

			// Load the surgery date
			m_dtLastSavedSurgeryDate = AdoFldDateTime(flds, "SurgeryDate");
			m_ctrlSurgeryDate.SetValue(_variant_t(m_dtLastSavedSurgeryDate, VT_DATE));

			// Load the procedures
			m_arProcedureIDs.RemoveAll();
			m_arLastSavedProcedureIDs.RemoveAll();
			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT ProcedureID FROM CaseHistoryProceduresT WHERE CaseHistoryID = {INT}", nCaseHistoryID);
			while(!rs->eof) {
				long nProcID = AdoFldLong(rs, "ProcedureID");
				m_arProcedureIDs.Add(nProcID);
				m_arLastSavedProcedureIDs.Add(nProcID);
				rs->MoveNext();
			}
			rs->Close();
			DisplayProcedureInfo();
			
			// Load the notes
			m_strLastSavedNotes = AdoFldString(flds, "Notes");
			SetDlgItemText(IDC_NOTES_EDIT, m_strLastSavedNotes);

			// Load the Minutes
			_variant_t var = flds->Item["AnesthMinutes"]->Value;
			if(var.vt == VT_I4 && VarLong(var) > 0) {
				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, VarLong(var));
				m_CurAnesthMinutes = VarLong(var);
			}
			m_nLastSavedAnesthesiaMinutes = m_CurAnesthMinutes;

			var = flds->Item["FacilityMinutes"]->Value;
			if(var.vt == VT_I4 && VarLong(var) > 0) {
				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, VarLong(var));
				m_CurFacilityMinutes = VarLong(var);
			}
			m_nLastSavedFacilityMinutes = m_CurFacilityMinutes;

			// (j.jones 2006-11-24 09:25) - PLID 23372 - supported the Discharge Status
			var = flds->Item["DischargeStatusID"]->Value;
			if(var.vt != VT_NULL) {
				m_nLastSavedDischargeStatusID = VarLong(var);
				m_DischargeStatusCombo->SetSelByColumn(0, var);
				m_strLastSavedDischargeStatus = VarString(m_DischargeStatusCombo->CurSel->GetValue(1)) + " - " + VarString(m_DischargeStatusCombo->CurSel->GetValue(2));
			}
			else {
				m_nLastSavedDischargeStatusID = -1;
				m_strLastSavedDischargeStatus = "";
				m_DischargeStatusCombo->PutCurSel(NULL);
			}

			// (j.jones 2008-02-26 16:06) - PLID 29102 - now requery the allocation combo (if they have the license)
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
				CString strWhere = "";
				strWhere.Format("PatientID IN (SELECT PersonID FROM CaseHistoryT WHERE ID = %li) "
					"AND ID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID <> %li) "
					"AND LocationID = %li", nCaseHistoryID, nCaseHistoryID, m_nCurLocationID);

				m_InvAllocationCombo->PutWhereClause(_bstr_t(strWhere));
				m_InvAllocationCombo->Requery();

				// (j.jones 2008-03-10 14:40) - PLID 29233 - clear the allocation contents list
				m_InvAllocationContentsList->Clear();

				// (j.jones 2008-02-27 09:31) - PLID 29102 - load the allocation link information
				var = flds->Item["AllocationID"]->Value;
				//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
				// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
				if(var.vt != VT_NULL && g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
					m_nLastSavedAllocationID = VarLong(var);
					NXDATALIST2Lib::IRowSettingsPtr pAllocationRow = m_InvAllocationCombo->SetSelByColumn(iaccID, var);
					if(pAllocationRow) {
						m_strLastSavedAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(VarDateTime(pAllocationRow->GetValue(iaccInputDate)), NULL, dtoDate));
					}
					else {
						//why is the allocation not in the list? is it bad data?
						ASSERT(FALSE);
					}
					// (j.jones 2008-03-10 14:40) - PLID 29233 - requery the allocation contents list
					CString strContentsWhere;
					strContentsWhere.Format("AllocationID = %li", m_nLastSavedAllocationID);
					m_InvAllocationContentsList->PutWhereClause((LPCTSTR)strContentsWhere);
					m_InvAllocationContentsList->Requery();
				}
				else {
					m_nLastSavedAllocationID = -1;
					m_strLastSavedAllocationDescription = "";
					m_InvAllocationCombo->PutCurSel(NULL);
				}
			}
			else {
				m_nLastSavedAllocationID = -1;
				m_strLastSavedAllocationDescription = "";
			}

			// Load all the log times
			_variant_t varTime;
			//TES 1/10/2007 - PLID 23575 - Made a macro for loading the LastSaved member variable while setting the display,
			// here's how it's defined (up above this function):

			/*#define LOAD_LOG_TIME(strFieldName, dtVariable, nCtrlID)	varTime = flds->GetItem(strFieldName)->Value;\
			if(varTime.vt == VT_DATE) dtVariable = VarDateTime(varTime);\
			SetDlgItemTime(nCtrlID, AsString(varTime), FALSE);*/

			LOAD_LOG_TIME("LogPreOpIn", m_dtLastSavedPreOpIn, IDC_PREOP_IN_EDIT);
			LOAD_LOG_TIME("LogPreOpOut", m_dtLastSavedPreOpOut, IDC_PREOP_OUT_EDIT);
			LOAD_LOG_TIME("LogSurgeonIn", m_dtLastSavedSurgeonIn, IDC_SURGEON_IN_EDIT);
			LOAD_LOG_TIME("LogSurgeonOut", m_dtLastSavedSurgeonOut, IDC_SURGEON_OUT_EDIT);
			LOAD_LOG_TIME("LogOpRoomIn", m_dtLastSavedOpRoomIn, IDC_OPROOM_IN_EDIT);
			LOAD_LOG_TIME("LogOpRoomOut", m_dtLastSavedOpRoomOut, IDC_OPROOM_OUT_EDIT);
			LOAD_LOG_TIME("LogRecoveryIn", m_dtLastSavedRecoveryIn, IDC_RECOVERY_IN_EDIT);
			LOAD_LOG_TIME("LogRecoveryOut", m_dtLastSavedRecoveryOut, IDC_RECOVERY_OUT_EDIT);
			LOAD_LOG_TIME("LogAnesthesiaIn", m_dtLastSavedAnesthesiaIn, IDC_ANESTHESIA_START_EDIT);
			LOAD_LOG_TIME("LogAnesthesiaOut", m_dtLastSavedAnesthesiaOut, IDC_ANESTHESIA_STOP_EDIT);
			LOAD_LOG_TIME("LogFacilityIn", m_dtLastSavedFacilityIn, IDC_FACILITY_START_EDIT);
			LOAD_LOG_TIME("LogFacilityOut", m_dtLastSavedFacilityOut, IDC_FACILITY_STOP_EDIT);
			LOAD_LOG_TIME("Log23HourRoomIn", m_dtLastSaved23HourRoomIn, IDC_23HOURROOM_IN_EDIT);
			LOAD_LOG_TIME("Log23HourRoomOut", m_dtLastSaved23HourRoomOut, IDC_23HOURROOM_OUT_EDIT);

			m_dtLastEnteredAnesthesiaIn = m_dtLastSavedAnesthesiaIn;
			m_dtLastEnteredFacilityIn = m_dtLastSavedFacilityIn;
			m_dtLastEnteredAnesthesiaOut = m_dtLastSavedAnesthesiaOut;
			m_dtLastEnteredFacilityOut = m_dtLastSavedFacilityOut;

			// (j.gruber 2008-07-07 17:36) - PLID 15807 - load the anesthesia type and diagcodes
			m_strLastAnesType = AdoFldString(flds, "AnesType", "");
			
			// (a.wilson 02/20/2014) PLID 60774 - load existing diagcodes.
			m_pPreOpDiagnosisList->PutFromClause(_bstr_t(
				"( \r\n"
				"	SELECT \r\n"
				"	CaseHistoryT.ID, c.IndexNumber, c.DiagICD9CodeID, Dx9.CodeNumber AS DiagICD9Code, Dx9.CodeDesc AS DiagICD9Desc, \r\n"
				"	c.DiagICD10CodeID, Dx10.CodeNumber AS DiagICD10Code, Dx10.CodeDesc AS DiagICD10Desc \r\n"
				"	FROM CaseHistoryT \r\n"
				"	CROSS APPLY \r\n"
				"	( \r\n"
				"		SELECT 1, PreOpDx1, PreOpDx1ICD10 \r\n"
				"		UNION ALL SELECT 2, PreOpDx2, PreOpDx2ICD10 \r\n"
				"		UNION ALL SELECT 3, PreOpDx3, PreOpDx3ICD10 \r\n"
				"		UNION ALL SELECT 4, PreOpDx4, PreOpDx4ICD10 \r\n"
				"	) c (IndexNumber, DiagICD9CodeID, DiagICD10CodeID) \r\n"
				"	LEFT JOIN DiagCodes Dx9 ON c.DiagICD9CodeID = Dx9.ID \r\n"
				"	LEFT JOIN DiagCodes Dx10 ON c.DiagICD10CodeID = Dx10.ID \r\n"
				") CaseHistoryDiagCodesQ \r\n"));
			m_pPreOpDiagnosisList->PutWhereClause(FormatBstr(
				"CaseHistoryDiagCodesQ.ID = %li \r\n", nCaseHistoryID));

			m_pPostOpDiagnosisList->PutFromClause(_bstr_t(
				"( \r\n"
				"	SELECT \r\n"
				"	CaseHistoryT.ID, c.IndexNumber, c.DiagICD9CodeID, Dx9.CodeNumber AS DiagICD9Code, Dx9.CodeDesc AS DiagICD9Desc, \r\n"
				"	c.DiagICD10CodeID, Dx10.CodeNumber AS DiagICD10Code, Dx10.CodeDesc AS DiagICD10Desc \r\n"
				"	FROM CaseHistoryT \r\n"
				"	CROSS APPLY \r\n"
				"	( \r\n"
				"		SELECT 1, PostOpDx1, PostOpDx1ICD10 \r\n"
				"		UNION ALL SELECT 2, PostOpDx2, PostOpDx2ICD10 \r\n"
				"		UNION ALL SELECT 3, PostOpDx3, PostOpDx3ICD10 \r\n"
				"		UNION ALL SELECT 4, PostOpDx4, PostOpDx4ICD10 \r\n"
				"	) c (IndexNumber, DiagICD9CodeID, DiagICD10CodeID) \r\n"
				"	LEFT JOIN DiagCodes Dx9 ON c.DiagICD9CodeID = Dx9.ID \r\n"
				"	LEFT JOIN DiagCodes Dx10 ON c.DiagICD10CodeID = Dx10.ID \r\n"
				") CaseHistoryDiagCodesQ \r\n"));
			m_pPostOpDiagnosisList->PutWhereClause(FormatBstr(
				"CaseHistoryDiagCodesQ.ID = %li \r\n", nCaseHistoryID));

			m_pPreOpDiagnosisList->Requery();
			m_pPostOpDiagnosisList->Requery();
			m_pPreOpDiagnosisList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pPostOpDiagnosisList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

			UpdateDiagnosisListColumnSizes();
		}

		// Clear the list of details to remove
		m_aryDeleteDetailIDs.RemoveAll();

		// Load the case history details
		CString strWhere;
		strWhere.Format("CaseHistoryID = %li", nCaseHistoryID);
		m_dlCaseHistoryDetails->PutWhereClause((LPCTSTR)strWhere);
		m_dlCaseHistoryDetails->Requery();
	}
	
	// Done loading
	m_nCaseHistoryID = nCaseHistoryID;

	m_bIsModified = FALSE;
}

// Throws _com_error, CException*
BOOL CCaseHistoryDlg::Save()
{
	CString strBigSql;
	//TES 1/8/2007 - PLID 23575 - This audits now.
	long nAuditTransactionID = -1;	
	try {

		CWaitCursor pWait;

		// Build the string that will contain all the execute statements
		if (ValidateAndBuildSaveString(strBigSql, nAuditTransactionID)) {
			// Write to the data
			BEGIN_TRANS("CCaseHistoryDlg::Save") {

				// (j.jones 2006-12-13 17:15) - PLID 23578 - supported creating from within this dialog
				if(m_bIsNew) {
					//create the records

					//this recordset cannot be parameterized
					_RecordsetPtr prsCaseHistoryResults = CreateRecordset(
						"SET NOCOUNT ON \r\n"
						"BEGIN TRAN \r\n"
						"%s "
						"COMMIT TRAN \r\n"
						"SET NOCOUNT OFF \r\n"
						"SELECT @nCaseHistoryID AS CaseHistoryID, @nMailID AS MailID", strBigSql);

					// We now get our answers as part of our above ad-hoc proc, so they are retrieved here.
					long nMailID;
					{
						FieldsPtr pflds = prsCaseHistoryResults->GetFields();
						nMailID = AdoFldLong(pflds, "MailID");
						m_nCaseHistoryID = AdoFldLong(pflds, "CaseHistoryID");
						//TES 1/10/2006 - PLID 23575 - Now that we have an ID, audit the creation.
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						CString strName;
						GetDlgItemText(IDC_NAME_EDIT, strName);
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryCreated, m_nCaseHistoryID, "", strName, aepHigh, aetCreated);

						// (j.jones 2008-02-27 10:05) - PLID 29104 - audit linking to an allocation, if we have one
						{
							long nAllocationID = -1;						
							//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.5
							// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
							if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && m_InvAllocationCombo->GetCurSel() != NULL) {
								nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
								if(nAllocationID != -1) {
									CString strAllocationDescription;
									strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(VarDateTime(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccInputDate)), NULL, dtoDate));

									AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryInvAllocationLinkCreated, m_nCaseHistoryID, m_strName + ", No Linked Allocation", strAllocationDescription, aepMedium, aetCreated);
									AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkCreated, nAllocationID, strAllocationDescription + ", No Linked Case History", "Case History: " + m_strName, aepMedium, aetCreated);
								}
							}
						}

						m_bIsNew = FALSE;
					}

					// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
					CClient::RefreshMailSentTable(m_nPersonID, nMailID);
				}
				else {

					// Run the update

					CString strFinalRecordset;
					strFinalRecordset.Format(
							"SET NOCOUNT ON \r\n"
							"%s "
							"SET NOCOUNT OFF \r\n"
							"SELECT @nMailID AS ChangedMailSentID",
							strBigSql);

					// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
					NxAdo::PushPerformanceWarningLimit ppw(-1);
					_RecordsetPtr rsResult = CreateRecordsetStd(strFinalRecordset,
						adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);
					if(!rsResult->eof) {
						// (j.jones 2011-07-22 15:06) - PLID 21784 - refresh using the MailSentID
						// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
						CClient::RefreshMailSentTable(m_nPersonID, VarLong(rsResult->Fields->Item["ChangedMailSentID"]->Value));
					}
				}

				CommitAuditTransaction(nAuditTransactionID);
			} END_TRANS("CCaseHistoryDlg::Save");

			// (j.jones 2006-10-30 14:22) - PLID 23271 - refresh the product inventory always,
			// because even if not completed, this will update the "available" amount, which is
			// different from "on hand".
			// Note: if this dialog ever gets tracking that detects changed line items and
			// updates accordingly, this code should be changed to support that
			{
				long nLocationID = -1;
				ValidateCurLocationID(nLocationID);
				
				//update each product
				// (j.jones 2008-09-15 13:23) - PLID 31375 - InvUtils::EnsureInventoryTodoAlarms
				// now supports multiple products at once, so build up an array
				CArray<long, long> aryProductIDs;
				long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
				while (p) {
					NXDATALISTLib::IRowSettingsPtr pRow;
					// Get the row object and move to the next enum
					{
						LPDISPATCH lpDisp = NULL;
						m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
						pRow = lpDisp;
						lpDisp->Release();
					}

					long nDetailID = VarLong(pRow->GetValue(chdlcID));
					if (nDetailID != MAXLONG) {
						ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
						if(nItemType == chditProduct) {
							aryProductIDs.Add(VarLong(pRow->GetValue(chdlcItemID)));						
						}
					}
				}

				// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
				// because that's a closer description to what it actually does. Also removed unused quantity parameter.
				// (j.jones 2008-09-15 13:24) - PLID 31375 - support the usage of EnsureInventoryTodoAlarms for more than
				// one product at a time
				if(aryProductIDs.GetSize() > 0) {
					//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
					InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, nLocationID, false);
				}
			}

			// Now that it's saved, we need to reflect the fact that the case history is no longer "modified"
			m_aryDeleteDetailIDs.RemoveAll();
			m_bIsModified = FALSE;

			//now refresh the list, so CaseHistoryDetailIDs are loaded
			DeleteAllFromCaseHistoryDetailProductItemsArray();
			m_dlCaseHistoryDetails->Requery();

			// Success
			return TRUE;
		} else {
			// Did not save
			return FALSE;
		}

	} NxCatchAllCall("Error in CCaseHistoryDlg::Save", RollbackAuditTransaction(nAuditTransactionID));
	return FALSE;
}

BOOL CCaseHistoryDlg::ValidateCurProviderID(OUT long &nProviderID, OUT CString &strProviderName)
{
	long nProvIndex = m_dlProviders->GetCurSel();
	if (nProvIndex != -1) {
		nProviderID = m_dlProviders->GetValue(nProvIndex, chdpccID);
		strProviderName = VarString(m_dlProviders->GetValue(nProvIndex, chdpccName));
		return TRUE;
	} else if(m_dlProviders->IsComboBoxTextInUse) {
		//TES 1/9/2007 - PLID 23575 - Actually, we can now return values, because we know that if this is using the
		// ComboBoxText, it MUST be using the originally loaded provider ID, because there's no way for the user to set
		// the ComboBoxText.  So, that's the ID, and the name is the text.
		/*//do nothing, don't warn, but don't set the index - we handle this in the saving		
		return TRUE;*/

		nProviderID = m_nLastSavedProviderID;
		strProviderName = (LPCTSTR)m_dlProviders->ComboBoxText;
		return TRUE;
	} else {
		// Please select a provider
		MessageBox("Please select a provider for this case.", NULL, MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}
}

BOOL CCaseHistoryDlg::ValidateCurLocationID(OUT long &nLocationID, OPTIONAL OUT CString *pstrLocationName /*= NULL*/)
{
	long nLocIndex = m_dlLocations->GetCurSel();
	if (nLocIndex != -1) {
		nLocationID = m_dlLocations->GetValue(nLocIndex, chdlccID);
		if(pstrLocationName) *pstrLocationName = VarString(m_dlLocations->GetValue(nLocIndex, chdlccName));
		m_nCurLocationID = nLocationID;
		return TRUE;
	} else if(m_dlLocations->IsComboBoxTextInUse) {
		//TES 1/9/2007 - PLID 23575 - Actually, we can now return values, because we know that if this is using the
		// ComboBoxText, it MUST be using the originally loaded location ID, because there's no way for the user to set
		// the ComboBoxText.  So, that's the ID, and the name is the text.
		/*//do nothing, don't warn, but don't set the index - we handle this in the saving		
		return TRUE;*/

		nLocationID = m_nLastSavedLocationID;
		if(pstrLocationName) *pstrLocationName = (LPCTSTR)m_dlLocations->ComboBoxText;
		return TRUE;
	} else {
		// Please select a location
		MessageBox("Please select a location for this case.", NULL, MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}
}

BOOL CCaseHistoryDlg::ValidateCurPOSID(OUT long &nPOSID, OUT CString &strPOSName)
{
	long nLocIndex = m_dlPOS->GetCurSel();
	if (nLocIndex != -1) {
		nPOSID = m_dlPOS->GetValue(nLocIndex, chdlccID);
		strPOSName = VarString(m_dlPOS->GetValue(nLocIndex, chdlccName));
		return TRUE;
	} else if(m_dlPOS->IsComboBoxTextInUse) {
		//TES 1/9/2007 - PLID 23575 - Actually, we can now return values, because we know that if this is using the
		// ComboBoxText, it MUST be using the originally loaded POS ID, because there's no way for the user to set
		// the ComboBoxText.  So, that's the ID, and the name is the text.
		/*//do nothing, don't warn, but don't set the index - we handle this in the saving		
		return TRUE;*/
		
		nPOSID = m_nLastSavedPOSID;
		strPOSName = (LPCTSTR)m_dlPOS->ComboBoxText;
		return TRUE;
	} else {
		// Please select a POS
		MessageBox("Please select a Place Of Service for this case.", NULL, MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}
}

BOOL CCaseHistoryDlg::ValidateCurProcedures()
{
	if(!m_bHasProcedureBeenChanged || m_nAppointmentID == -1)
		return TRUE;

	CString strTemp = ArrayAsString(m_arProcedureIDs, false);

	//first see if the appointment has procedures not on the case history
	//this recordset cannot be parameterized
	_RecordsetPtr rs = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = %li AND PurposeID NOT IN (%s)",m_nAppointmentID, strTemp);
	if(!rs->eof) {
		if(IDNO == MessageBox("The attached appointment has purposes not selected on this case history. Are you sure you wish to save?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	rs->Close();

	//now see if the case history has procedures not on the appointment
	BOOL bAllFound = TRUE;
	for(int i = 0; i < m_arProcedureIDs.GetSize(); i++) {
		// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
		rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT} AND PurposeID = {INT}",m_nAppointmentID, m_arProcedureIDs[i]);
		if(rs->eof) {
			bAllFound = FALSE;
		}
		rs->Close();
	}

	if(!bAllFound) {
		if(IDNO == MessageBox("The case history has purposes not selected on the attached appointment. Are you sure you wish to save?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return FALSE;
		}
	}

	return TRUE;
}

#define AUDIT_LOG_TIME(time)	if(!m_bIsNew && (m_dtLastSaved##time##.GetHour() != dt##time##.GetHour() || m_dtLastSaved##time##.GetMinute() != dt##time##.GetMinute() || m_dtLastSaved##time##.GetSecond() != dt##time##.GetSecond())) {\
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();\
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistory##time##, m_nCaseHistoryID, m_dtLastSaved##time##.GetStatus() == COleDateTime::invalid ? "" : FormatDateTimeForInterface(m_dtLastSaved##time##, DTF_STRIP_SECONDS, dtoTime), dt##time##.GetStatus() == COleDateTime::invalid ? "" : FormatDateTimeForInterface(dt##time##, DTF_STRIP_SECONDS, dtoTime), aepMedium, aetChanged);\
			}

// A helper function used by the save to store the case history details
// Throws _com_error
//TES 1/8/2007 - PLID 23575 - Added auditing.
BOOL CCaseHistoryDlg::ValidateAndBuildSaveString(OUT CString &strSqlString, OUT long &nAuditTransactionID)
{
	// Validate the values on screen and load them into local variables
	// (j.gruber 2008-07-08 09:24) - PLID 15807 - added anesthesiaType and DiagCodes
	CString strCompletedDate, strSurgeryDate, strPreOpIn, strPreOpOut, strOpRoomIn, strOpRoomOut, 
		strRecoveryIn, strRecoveryOut, strAnesthesiaIn, strAnesthesiaOut, strFacilityIn, strFacilityOut, 
		strSurgeonIn, strSurgeonOut, str23HourRoomIn, str23HourRoomOut, strNotes, strAnesType;
	long nAnesthMinutes = 0, nFacilityMinutes = 0;
	long nProviderID = -1;
	long nLocationID = -1;
	long nPOSID = -1;
	long nAllocationID = -1;

	// (a.wilson 2014-02-25 16:14) - PLID 60774 - removed all the old variables.
	CString strPreOpCodes, strPostOpCodes;

	CString strDischargeStatusID = "NULL";
	CString strAnesthMinutes = "NULL";
	CString strFacilityMinutes = "NULL";
	CString strAppointmentID = "NULL";

	CArray<long,long> arAddedProcedureIDs, arRemovedProcedureIDs;
	CStringArray saAddedProcedureNames, saRemovedProcedureNames;
	
	BOOL bRequireAnesthesiaTimes = FALSE;
	BOOL bRequireFacilityFeeTimes = FALSE;

	// Get the POS id
	//TES 1/9/2006 - PLID 23575 - Also get the name, for auditing
	CString strPOSName;
	if (!ValidateCurPOSID(nPOSID, strPOSName)) {
		return FALSE;
	}

	{
		//loop through and validate line items

		CString strServiceIDs;

		long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
		while (p) {
			NXDATALISTLib::IRowSettingsPtr pRow;
			// Get the row object and move to the next enum
			{
				LPDISPATCH lpDisp = NULL;
				m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
			}

			long nDetailID = VarLong(pRow->GetValue(chdlcID));
			if(nDetailID == MAXLONG) {
				continue;
			}

			// (j.jones 2008-05-21 09:08) - PLID 29249 - warn if any product's quantity is a decimal,
			// if it is a serialized product

			BOOL bIsSerializedProduct = VarBool(pRow->GetValue(chdlcIsSerializedProduct), FALSE);
			double dblQuantity = VarDouble(pRow->GetValue(chdlcQuantity), 0.0);
			CString strItemName = VarString(pRow->GetValue(chdlcName), "");
			
			if(bIsSerializedProduct && (long)dblQuantity != dblQuantity) {

				CString strWarn;
				strWarn.Format("The product '%s' is has a quantity of %g, but it must be a whole number as it requires a serial number / expiration date.\n\n"
					"Please correct this quantity before saving.", strItemName, dblQuantity);
				AfxMessageBox(strWarn);
				return FALSE;
			}
			
			ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
			if(nItemType != chditPerson) {

				long nServiceID = VarLong(pRow->GetValue(chdlcItemID));

				// (j.jones 2008-09-15 15:43) - PLID 31375 - track the service ID in a
				// comma delimited list
				if(!strServiceIDs.IsEmpty()) {
					strServiceIDs += ",";
				}
				strServiceIDs += AsString(nServiceID);
			}
		}

		{
			// (j.jones 2008-05-21 16:15) - PLID 29223 - see if the given item is anesthesia or
			// a facility fee, which we will need to know later in this function
			// (j.jones 2008-09-15 15:43) - PLID 31375 - now we can run these recordsets just one time
			if(!strServiceIDs.IsEmpty()) {

				//this cannot be parameterized
				_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 AnesthesiaSetupT.ID "
					"FROM AnesthesiaSetupT "
					"INNER JOIN ServiceT ON AnesthesiaSetupT.ServiceID = ServiceT.ID "
					"WHERE ServiceID IN (%s) AND Anesthesia = 1 AND UseAnesthesiaBilling = 1 "
					"AND LocationID = %li AND AnesthesiaFeeBillType <> 1", strServiceIDs, nPOSID);

				if(!rs->eof) {
					//it is an anesthesia item and the billing type requires the time
					bRequireAnesthesiaTimes = TRUE;
				}
				rs->Close();

				//this cannot be parameterized
				rs = CreateRecordset("SELECT TOP 1 FacilityFeeSetupT.ID "
					"FROM FacilityFeeSetupT "
					"INNER JOIN ServiceT ON FacilityFeeSetupT.ServiceID = ServiceT.ID "
					"WHERE ServiceID IN (%s) AND FacilityFee = 1 AND UseFacilityBilling = 1 "
					"AND LocationID = %li AND FacilityFeeBillType <> 1", strServiceIDs, nPOSID);

				if(!rs->eof) {
					//it is a facility fee item and the billing type requires the time
					bRequireFacilityFeeTimes = TRUE;
				}
				rs->Close();
			}
		}
	}

	{
		// Get the values off the screen
		COleDateTime dtSurgeryDate, dtPreOpIn, dtPreOpOut, dtOpRoomIn, dtOpRoomOut, 
			dtRecoveryIn, dtRecoveryOut, dtAnesthesiaIn, dtAnesthesiaOut, dtFacilityIn, dtFacilityOut, 
			dtSurgeonIn, dtSurgeonOut, dt23HourRoomIn, dt23HourRoomOut;
		{
			// Get the name
			GetDlgItemText(IDC_NAME_EDIT, m_strName);

			m_strName.TrimLeft();
			m_strName.TrimRight();
			if(m_strName.IsEmpty()) {
				// (a.walling 2007-02-26 16:13) - PLID 24943 - Use MessageBox so we know 'this' is the parent.
				MessageBox("You must enter a description for this case history.");
				return FALSE;
			}

			//TES 1/10/2006 - PLID 23575 - Audit the name
			if(!m_bIsNew && m_strLastSavedName != m_strName) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryDescription, m_nCaseHistoryID, m_strLastSavedName, m_strName, aepMedium, aetChanged);
			}

			// Get the provider id
			//TES 1/8/2006 - PLID 23575 - Also get the name, for auditing.
			CString strProviderName;
			if (!ValidateCurProviderID(nProviderID, strProviderName)) {
				return FALSE;
			}

			//Audit the provider id
			if(!m_bIsNew && nProviderID != m_nLastSavedProviderID) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryProvider, m_nCaseHistoryID, m_strLastSavedProviderName, strProviderName, aepMedium, aetChanged);
			}

			// Get the location id
			//TES 1/9/2006 - PLID 23575 - Also get the name, for auditing
			CString strLocationName;
			if (!ValidateCurLocationID(nLocationID, &strLocationName)) {
				return FALSE;
			}

			//Audit the location id
			if(!m_bIsNew && nLocationID != m_nLastSavedLocationID) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryLocation, m_nCaseHistoryID, m_strLastSavedLocationName, strLocationName, aepMedium, aetChanged);
			}

			//Audit the POS id
			if(!m_bIsNew && nPOSID != m_nLastSavedPOSID) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryPOS, m_nCaseHistoryID, m_strLastSavedPOSName, strPOSName, aepMedium, aetChanged);
			}

			if (!ValidateCurProcedures()) {
				return FALSE;
			}

			//Audit the procedures which have been added and removed
			ParseProcedures(arAddedProcedureIDs, saAddedProcedureNames, arRemovedProcedureIDs, saRemovedProcedureNames);
			if(!m_bIsNew) {
				// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
				int i = 0;
				for(i = 0; i < saAddedProcedureNames.GetSize(); i++) {
					if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryProcedureAdded, m_nCaseHistoryID, "", saAddedProcedureNames[i], aepMedium, aetCreated);
				}
				for(i = 0; i < saRemovedProcedureNames.GetSize(); i++) {
					if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryProcedureRemoved, m_nCaseHistoryID, saRemovedProcedureNames[i], "<Deleted>", aepMedium, aetDeleted);
				}
			}

			// (j.jones 2008-05-21 16:20) - PLID 29223 - validate anesthesia and 
			// facility dates only if there are anesthesia or facility fee items
			if (bRequireAnesthesiaTimes && !ValidateAnesthesiaDates()) {
				return FALSE;
			}

			if (bRequireFacilityFeeTimes && !ValidateFacilityDates()) {
				return FALSE;
			}
			
			// Get the surgery date
			dtSurgeryDate = VarDateTime(m_ctrlSurgeryDate.GetValue());

			//Audit the surgery date
			if(!m_bIsNew && dtSurgeryDate != m_dtLastSavedSurgeryDate) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistorySurgeryDate, m_nCaseHistoryID, FormatDateTimeForInterface(m_dtLastSavedSurgeryDate, NULL, dtoDate), FormatDateTimeForInterface(dtSurgeryDate, NULL, dtoDate), aepMedium, aetChanged);
			}

			// Get the notes
			GetDlgItemText(IDC_NOTES_EDIT, strNotes);

			//Audit the notes
			if(!m_bIsNew && m_strLastSavedNotes != strNotes) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryNotes, m_nCaseHistoryID, m_strLastSavedNotes, strNotes, aepMedium, aetChanged);
			}


			// Get the Minutes
			nAnesthMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES);
			nFacilityMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES);
			
			// Get all the times 
			_DNxTimePtr nxtTmp = BindNxTimeCtrl(this, IDC_PREOP_IN_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_PREOP_OUT_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_SURGEON_IN_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_SURGEON_OUT_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_OPROOM_IN_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_OPROOM_OUT_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_RECOVERY_IN_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_RECOVERY_OUT_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			
			if(m_nxtAnesthStart->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			if(m_nxtAnesthEnd->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			if(m_nxtFacilityStart->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			if(m_nxtFacilityEnd->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}

			nxtTmp = BindNxTimeCtrl(this, IDC_23HOURROOM_IN_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}
			nxtTmp = BindNxTimeCtrl(this, IDC_23HOURROOM_OUT_EDIT);
			if(nxtTmp->GetStatus() == 2) {MessageBox("You have entered an invalid time"); return FALSE;}

			// (a.walling 2007-02-27 09:31) - PLID 24451 - The issue with multiple audits is that the time is saved
			// with seconds the first time around, then when it is reloaded the seconds are gone and it is then detected
			// as a change. So we will just strip the seconds when we get the data to save initially
			dtPreOpIn = StripSeconds(GetDlgItemTime(IDC_PREOP_IN_EDIT));
			dtPreOpOut = StripSeconds(GetDlgItemTime(IDC_PREOP_OUT_EDIT));
			dtSurgeonIn = StripSeconds(GetDlgItemTime(IDC_SURGEON_IN_EDIT));
			dtSurgeonOut = StripSeconds(GetDlgItemTime(IDC_SURGEON_OUT_EDIT));
			dtOpRoomIn = StripSeconds(GetDlgItemTime(IDC_OPROOM_IN_EDIT));
			dtOpRoomOut = StripSeconds(GetDlgItemTime(IDC_OPROOM_OUT_EDIT));
			dtRecoveryIn = StripSeconds(GetDlgItemTime(IDC_RECOVERY_IN_EDIT));
			dtRecoveryOut = StripSeconds(GetDlgItemTime(IDC_RECOVERY_OUT_EDIT));
			dtAnesthesiaIn = StripSeconds(GetDlgItemTime(IDC_ANESTHESIA_START_EDIT));
			dtAnesthesiaOut = StripSeconds(GetDlgItemTime(IDC_ANESTHESIA_STOP_EDIT));
			dtFacilityIn = StripSeconds(GetDlgItemTime(IDC_FACILITY_START_EDIT));
			dtFacilityOut = StripSeconds(GetDlgItemTime(IDC_FACILITY_STOP_EDIT));
			dt23HourRoomIn = StripSeconds(GetDlgItemTime(IDC_23HOURROOM_IN_EDIT));
			dt23HourRoomOut = StripSeconds(GetDlgItemTime(IDC_23HOURROOM_OUT_EDIT));

			//TES 1/10/2007 - PLID 23575 - Now, audit the times.  Here's the macro (defined above this function):
			/*#define AUDIT_LOG_TIME(time)	if(!m_bIsNew && (m_dtLastSaved##time##.GetHour() != dt##time##.GetHour() || m_dtLastSaved##time##.GetMinute() != dt##time##.GetMinute() || m_dtLastSaved##time##.GetSecond() != dt##time##.GetSecond())) {\
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();\
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistory##time##, m_nCaseHistoryID, m_dtLastSaved##time##.GetStatus() == COleDateTime::invalid ? "" : FormatDateTimeForInterface(m_dtLastSaved##time##, DTF_STRIP_SECONDS, dtoTime), dt##time##.GetStatus() == COleDateTime::invalid ? "" : FormatDateTimeForInterface(dt##time##, DTF_STRIP_SECONDS, dtoTime), aepMedium, aetChanged);\
			}*/
			AUDIT_LOG_TIME(PreOpIn);
			AUDIT_LOG_TIME(PreOpOut);
			AUDIT_LOG_TIME(SurgeonIn);
			AUDIT_LOG_TIME(SurgeonOut);
			AUDIT_LOG_TIME(OpRoomIn);
			AUDIT_LOG_TIME(OpRoomOut);
			AUDIT_LOG_TIME(RecoveryIn);
			AUDIT_LOG_TIME(RecoveryOut);
			AUDIT_LOG_TIME(AnesthesiaIn);
			AUDIT_LOG_TIME(AnesthesiaOut);
			AUDIT_LOG_TIME(FacilityIn);
			AUDIT_LOG_TIME(FacilityOut);
			AUDIT_LOG_TIME(23HourRoomIn);
			AUDIT_LOG_TIME(23HourRoomOut);

			//run through the "charge" list and make sure everything passes

			//TODO: right now we only validate the credentialing, so we
			//don't loop at all if we don't perform this check.
			//Once we add other checks, we'll need to loop through the list
			//and do the CredentialWarnCaseHistories check inside the loop

			BOOL bCheckAndWarnCredentials = GetRemotePropertyInt("CredentialWarnCaseHistories",0,0,"<None>",true) == 1;
			BOOL bCheckAndWarnLicensing = GetRemotePropertyInt("ExpLicenseWarnCaseHistories", bCheckAndWarnCredentials ? 1 : 0, 0, "<None>",true) == 1;

			if(bCheckAndWarnCredentials || bCheckAndWarnLicensing) {

				long nProviderIDToCheck = nProviderID;
				//TES 1/9/2007 - PLID 23575 - This is now handled in ValidateCurProviderID() above.
				/*if(m_dlProviders->IsComboBoxTextInUse) {
					_RecordsetPtr rs = CreateRecordset("SELECT ProviderID FROM CaseHistoryT WHERE ID = %li",m_nCaseHistoryID);
					if(!rs->eof) {
						nProviderIDToCheck = AdoFldLong(rs, "ProviderID",nProviderID);
					}
					rs->Close();
				}*/

				if(bCheckAndWarnLicensing) {

					CString str;

					ECredentialWarning eCredWarning = CheckPersonCertifications(nProviderIDToCheck);

					if(eCredWarning != ePassedAll) {

						if(eCredWarning == eFailedLicenseExpired) {

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nProviderID);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The selected provider has the following expired licenses:\n\n%s\n"
								"Do you still wish to save this case history?",strLicenses);
						}
						else if(eCredWarning == eFailedLicenseExpiringSoon) {

							//check if a license will expire within the given day range
							long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nProviderID,nLicenseWarnDayRange);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The following licenses are about to expire for the selected provider:\n\n%s\n"
								"Do you still wish to save this case history?",strLicenses);
						}

						if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
							return FALSE;
						}
					}
				}

				BOOL bWarnedAboutNonCredentialedCodes = FALSE;
				BOOL bWarnCredentialingTOS2 = GetRemotePropertyInt("CredentialWarningOnlyTOS2",0,0,"<None>",true) == 1;

				// (j.jones 2008-09-15 15:59) - PLID 31375 - this was previously run per detail on the case history
				_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID FROM CaseHistoryT WHERE ProviderID = {INT} AND ID = {INT}",nProviderIDToCheck,m_nCaseHistoryID);
				BOOL bProviderChanged = FALSE;
				if(rs->eof) {
					bProviderChanged = TRUE;
				}
				rs->Close();

				if(bProviderChanged) {

					long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
					while (p) {
						NXDATALISTLib::IRowSettingsPtr pRow;
						// Get the row object and move to the next enum
						{
							LPDISPATCH lpDisp = NULL;
							m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
							pRow = lpDisp;
							lpDisp->Release();
						}

						long nDetailID = VarLong(pRow->GetValue(chdlcID));
						if(nDetailID != MAXLONG) {
							ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
							if(nItemType == chditCptCode && !bWarnedAboutNonCredentialedCodes) {
								long CPTCodeID = VarLong(pRow->GetValue(chdlcItemID));

								if(bCheckAndWarnCredentials) {

									ECredentialWarning eCredWarning = CheckServiceCodeCredential(nProviderIDToCheck, CPTCodeID);

									if(eCredWarning != ePassedAll) {

										//it's not credentialed. Check the TOS status before we warn though.
										BOOL bWarn = TRUE;
										if(bWarnCredentialingTOS2 && IsRecordsetEmpty("SELECT ID FROM CPTCodeT WHERE TypeOfService = '2' AND ID = %li",CPTCodeID))
											bWarn = FALSE;

										if(bWarn) {

											CString str;

											if(eCredWarning == eFailedCredential) {

												str.Format("You have at least one Service code that your provider is not credentialed for.\n"
													"Are you sure you want to continue saving this case history?");
											}

											if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
												return FALSE;
											}
											bWarnedAboutNonCredentialedCodes = TRUE;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Convert the dates and times into well-formed strings
		{
			// Convert the completed date to string
			strCompletedDate = (m_varCompletedDate.vt != VT_NULL) ? "'" + FormatDateTimeForSql(VarDateTime(m_varCompletedDate)) + "'" : "NULL";

			//Audit the completed date
			BOOL bCompleted = m_varCompletedDate.vt != VT_NULL;
			if(!m_bIsNew && m_bLastSavedCompleted != bCompleted) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryCompleted, m_nCaseHistoryID, m_bLastSavedCompleted?"Completed":"Not Completed", bCompleted ? "Completed" : "Not Completed", aepHigh, aetChanged);
			}
			
			// Convert the surgery date to string
			strSurgeryDate = "'" + FormatDateTimeForSql(dtSurgeryDate, dtoDate) + "'";

			// Convert all the times to string
			strPreOpIn = (dtPreOpIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtPreOpIn)) + "'" : "NULL";
			strPreOpOut = (dtPreOpOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtPreOpOut)) + "'" : "NULL";
			strSurgeonIn = (dtSurgeonIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtSurgeonIn)) + "'" : "NULL";
			strSurgeonOut = (dtSurgeonOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtSurgeonOut)) + "'" : "NULL";
			strOpRoomIn = (dtOpRoomIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtOpRoomIn)) + "'" : "NULL";
			strOpRoomOut = (dtOpRoomOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtOpRoomOut)) + "'" : "NULL";
			strRecoveryIn = (dtRecoveryIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtRecoveryIn)) + "'" : "NULL";
			strRecoveryOut = (dtRecoveryOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtRecoveryOut)) + "'" : "NULL";
			strAnesthesiaIn = (dtAnesthesiaIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtAnesthesiaIn)) + "'" : "NULL";
			strAnesthesiaOut = (dtAnesthesiaOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtAnesthesiaOut)) + "'" : "NULL";
			strFacilityIn = (dtFacilityIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtFacilityIn)) + "'" : "NULL";
			strFacilityOut = (dtFacilityOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dtFacilityOut)) + "'" : "NULL";
			str23HourRoomIn = (dt23HourRoomIn.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dt23HourRoomIn)) + "'" : "NULL";
			str23HourRoomOut = (dt23HourRoomOut.GetStatus() != COleDateTime::invalid) ? "'" + _Q(FormatDateTimeForSql(dt23HourRoomOut)) + "'" : "NULL";
		}

		// (j.jones 2006-11-24 09:27) - PLID 23372 - added Discharge Status
		strDischargeStatusID = "NULL";
		long nDischargeStatusID = -1;
		CString strDischargeStatus;
		if(m_DischargeStatusCombo->GetCurSel() != NULL) {
			nDischargeStatusID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(0), -1);
			if(nDischargeStatusID != -1) {
				strDischargeStatusID.Format("%li", nDischargeStatusID);
				strDischargeStatus = VarString(m_DischargeStatusCombo->GetCurSel()->GetValue(1)) + " - " + VarString(m_DischargeStatusCombo->GetCurSel()->GetValue(2));
			}
		}
		
		//Audit the discharge status.
		if(!m_bIsNew && m_nLastSavedDischargeStatusID != nDischargeStatusID) {
			if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryDischargeStatus, m_nCaseHistoryID, m_strLastSavedDischargeStatus, strDischargeStatus, aepMedium, aetChanged);
		}


		//save the appointment ID, it may have changed, and it may be null
		strAppointmentID = "NULL";
		if(m_nAppointmentID != -1)
			strAppointmentID.Format("%li",m_nAppointmentID);

		//Audit the appointment
		if(!m_bIsNew && m_nLastSavedAppointmentID != m_nAppointmentID) {
			if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			CString strNewApptDesc;
			GetDlgItemText(IDC_APPOINTMENT_DESC, strNewApptDesc);
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryAppointment, m_nCaseHistoryID, m_strLastSavedAppointmentDesc, strNewApptDesc, aepMedium, aetChanged);
		}

		strAnesthMinutes = "NULL";
		if(nAnesthMinutes > 0)
			strAnesthMinutes.Format("%li",nAnesthMinutes);

		//TES 1/10/2007 - PLID 23575 - Audit, if both values are below 1 they count as identical
		if(!m_bIsNew && ((m_nLastSavedAnesthesiaMinutes > 0 || nAnesthMinutes > 0) && m_nLastSavedAnesthesiaMinutes != nAnesthMinutes) ) {
			if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryAnesthesiaMinutes, m_nCaseHistoryID, m_nLastSavedAnesthesiaMinutes < 1 ? "" : AsString(m_nLastSavedAnesthesiaMinutes), nAnesthMinutes < 1 ? "" : AsString(nAnesthMinutes), aepMedium, aetChanged);
		}


		strFacilityMinutes = "NULL";
		if(nFacilityMinutes > 0)
			strFacilityMinutes.Format("%li",nFacilityMinutes);

		//TES 1/10/2007 - PLID 23575 - Audit, if both values are below 1 they count as identical
		if(!m_bIsNew && ((m_nLastSavedFacilityMinutes > 0 || nFacilityMinutes > 0) && m_nLastSavedFacilityMinutes != nFacilityMinutes) ) {
			if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryFacilityMinutes, m_nCaseHistoryID, m_nLastSavedFacilityMinutes < 1 ? "" : AsString(m_nLastSavedFacilityMinutes), nFacilityMinutes < 1 ? "" : AsString(nFacilityMinutes), aepMedium, aetChanged);
		}

		// (j.gruber 2008-07-08 09:25) - PLID 15807 - AnesthesiaType
		strAnesType = "";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAnesType->CurSel;
		if (pRow) {
			strAnesType = VarString(pRow->GetValue(0), "");

			if ((!m_bIsNew) && (m_strLastAnesType != strAnesType)) {
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryAnesType, m_nCaseHistoryID, m_strLastAnesType, strAnesType, aepMedium, aetChanged);
			}
		}

		// (a.wilson 2014-02-24 17:09) - PLID 60774 - Generate Save for Diagnosis Codes.
		//	Code Order will be (9, 10), (9, 10), (9, 10), (9, 10)
		GenerateDiagnosisIDListStrings(strPreOpCodes, strPostOpCodes);

		// (j.jones 2008-02-27 09:04) - PLID 29102 - store the selected allocation ID
		{
			nAllocationID = -1;
			CString strAllocationDescription;
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && m_InvAllocationCombo->GetCurSel() != NULL) {
				nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
				if(nAllocationID != -1) {
					strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(VarDateTime(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccInputDate)), NULL, dtoDate));
				}
			}
			
			// (j.jones 2008-02-27 09:43) - PLID 29104 - audit the allocation link
			if(!m_bIsNew && m_nLastSavedAllocationID != nAllocationID) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//need to audit exactly how it changed, both on the case history and for the allocation
				if(m_nLastSavedAllocationID == -1) {
					//we made a new link
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryInvAllocationLinkCreated, m_nCaseHistoryID, m_strName + ", No Linked Allocation", strAllocationDescription, aepMedium, aetCreated);
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkCreated, nAllocationID, strAllocationDescription + ", No Linked Case History", "Case History: " + m_strName, aepMedium, aetCreated);
				}
				else if(nAllocationID == -1) {
					//we removed the link
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, m_nCaseHistoryID, m_strName + ", " + m_strLastSavedAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, m_strLastSavedAllocationDescription + ", Case History: " + m_strName, "<No Linked Case History>", aepMedium, aetDeleted);
				}
				else {
					//we changed the link
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryInvAllocationLinkChanged, m_nCaseHistoryID, m_strName + ", " + m_strLastSavedAllocationDescription, strAllocationDescription, aepMedium, aetChanged);
					
					//that means we removed from one allocation and linked another
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, m_nLastSavedAllocationID, m_strLastSavedAllocationDescription + ", Case History: " + m_strName, "<No Linked Case History>", aepMedium, aetDeleted);
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkCreated, nAllocationID, strAllocationDescription + ", No Linked Case History", "Case History: " + m_strName, aepMedium, aetCreated);
				}
			}
		}

	}

	// (j.jones 2006-12-13 16:38) - PLID 23578 - altered to support creating case histories
	if(m_bIsNew) {
		//create a new case history

		CString strCaseHistorySql;

		strCaseHistorySql = BeginSqlBatch();

		AddDeclarationToSqlBatch(strCaseHistorySql, "DECLARE @nCaseHistoryID INT");
		AddDeclarationToSqlBatch(strCaseHistorySql, "DECLARE @nCaseHistoryDetailID INT");
		AddDeclarationToSqlBatch(strCaseHistorySql, "DECLARE @nMailID INT");

		{
			AddStatementToSqlBatch(strCaseHistorySql, "SET @nCaseHistoryID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM CaseHistoryT WITH(UPDLOCK, HOLDLOCK))");

			// (j.gruber 2008-07-08 11:19) - PLID 15807 - add Anesthesia Type and DiagCodes
			// (a.wilson 2014-02-25 09:51) - PLID 60774 - add ICD-10 codes to insert query.
			AddStatementToSqlBatch(strCaseHistorySql, 
				"INSERT INTO CaseHistoryT (ID, Name, PersonID, LocationID, PlaceOfServiceID, CompletedDate, "
				"ProviderID, SurgeryDate, TurnOverTime, AppointmentID,"
				"LogPreOpIn, LogPreOpOut, LogOpRoomIn, LogOpRoomOut, "
				"LogRecoveryIn, LogRecoveryOut, LogAnesthesiaIn, LogAnesthesiaOut, "
				"LogFacilityIn, LogFacilityOut, "
				"LogSurgeonIn, LogSurgeonOut, Log23HourRoomIn, Log23HourRoomOut, "
				"AnesthMinutes, FacilityMinutes, "
				"Notes, DischargeStatusID, AnesType, "
				"PreOpDx1, PreOpDx1ICD10, PreOpDx2, PreOpDx2ICD10, PreOpDx3, PreOpDx3ICD10, PreOpDx4, PreOpDx4ICD10, "
				"PostOpDx1, PostOpDx1ICD10, PostOpDx2, PostOpDx2ICD10, PostOpDx3, PostOpDx3ICD10, PostOpDx4, PostOpDx4ICD10) "
				"VALUES (@nCaseHistoryID, '%s', %li, %li, %li, %s, %li, %s, NULL, %s,"
				"%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,"
				"'%s', %s, '%s', %s, %s)", 
				_Q(m_strName), m_nPersonID, nLocationID, nLocationID, strCompletedDate, 
				nProviderID, strSurgeryDate, strAppointmentID,
				strPreOpIn, strPreOpOut, strOpRoomIn, strOpRoomOut, 
				strRecoveryIn, strRecoveryOut, strAnesthesiaIn, strAnesthesiaOut, 
				strFacilityIn, strFacilityOut, 
				strSurgeonIn, strSurgeonOut, str23HourRoomIn, str23HourRoomOut, 
				strAnesthMinutes, strFacilityMinutes, 
				_Q(strNotes), strDischargeStatusID,
				_Q(strAnesType), strPreOpCodes, strPostOpCodes);
		}

		// Generate the string for the details
		{
			if (!ValidateAndBuildSaveDetailsCreateString(strCaseHistorySql, nAuditTransactionID)) {
				return FALSE;
			}
		}

		//now procedures
		{
			for(int i = 0; i < m_arProcedureIDs.GetSize(); i++) {
				AddStatementToSqlBatch(strCaseHistorySql, "INSERT INTO CaseHistoryProceduresT (CaseHistoryID, ProcedureID) VALUES (@nCaseHistoryID, %li)\r\n", m_arProcedureIDs[i]);
			}
		}
		
		// (j.jones 2008-09-04 13:31) - PLID 30288 - supported MailSentNotesT
		// (c.haag 2010-01-27 12:04) - PLID 36271 - Use GetDate() for the service date, not COleDateTime::GetCurrentTime()
		// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
		AddStatementToSqlBatch(strCaseHistorySql, 
			"INSERT INTO MailSent (PersonID, Selection, PathName, Subject, Sender, Date, Location, MailBatchID, InternalRefID, InternalTblName, TemplateID, ServiceDate) "
			"SELECT %li, '%s', '%s', '%s', '%s', GETDATE(), %li, (SELECT COALESCE(MAX(MailBatchID), 0) + 1 FROM MailSent WITH(UPDLOCK, HOLDLOCK)), @nCaseHistoryID, '%s', NULL, GETDATE()", 
			m_nPersonID, _Q(SELECTION_FILE), _Q(PATHNAME_OBJECT_CASEHISTORY), _Q(""), _Q(GetCurrentUserName()), GetCurrentLocation(), _Q("CaseHistoryT"));
		AddStatementToSqlBatch(strCaseHistorySql, "SET @nMailID = SCOPE_IDENTITY()");
		AddStatementToSqlBatch(strCaseHistorySql, "INSERT INTO MailSentNotesT (MailID, Note) VALUES (@nMailID, '%s')", _Q(m_strName));

		// (j.jones 2008-02-27 09:10) - PLID 29102 - added CaseHistoryAllocationLinkT
		{
			if(nAllocationID != -1) {
				AddStatementToSqlBatch(strCaseHistorySql, 
					"INSERT INTO CaseHistoryAllocationLinkT (AllocationID, CaseHistoryID) "
					"VALUES (%li, @nCaseHistoryID) ",nAllocationID);
			}
		}

		strSqlString = strCaseHistorySql;
		
		return TRUE;
	}
	else {
		//update existing case history
	
		// Generate each update string
		// (a.wilson 2014-02-25 10:22) - PLID 60774 - split delimited strings to add to update.
		CStringArray arPreOpCodes, arPostOpCodes;
		CString strCaseHistorySql, strMailSentSql, strDetailsSql, strProcedureSql;
		SplitString(strPreOpCodes, ",", &arPreOpCodes);
		SplitString(strPostOpCodes, ",", &arPostOpCodes);
		// (a.wilson 2014-02-25 15:12) - PLID 61034 - audit the diagnosis changes if its an update.
		AuditDiagnosisListChanges(nAuditTransactionID);

		// Generate the string for the case history properties
		{
			//if there is an inactive provider set, don't overwrite it
			CString strProviderUpdate;
			if(!m_dlProviders->IsComboBoxTextInUse) 
				strProviderUpdate.Format("ProviderID = %li,", nProviderID);

			//if there is an inactive location set, don't overwrite it
			CString strLocationUpdate;
			if(!m_dlLocations->IsComboBoxTextInUse) 
				strLocationUpdate.Format("LocationID = %li,", nLocationID);

			//if there is an inactive POS set, don't overwrite it
			CString strPOSUpdate;
			if(!m_dlPOS->IsComboBoxTextInUse) 
				strPOSUpdate.Format("PlaceOfServiceID = %li,", nPOSID);

			// (j.gruber 2008-07-08 11:23) - PLID 15807 - add Anesthesia Type and DiagCodes
			// (a.wilson 2014-02-25 10:22) - PLID 60774 - modify update query to include ICD-10 codes.
			strCaseHistorySql.Format("UPDATE CaseHistoryT SET "
				"Name = '%s', CompletedDate = %s, %s %s %s SurgeryDate = %s, "
				"LogPreOpIn = %s, LogPreOpOut = %s, LogOpRoomIn = %s, LogOpRoomOut = %s, "
				"LogRecoveryIn = %s, LogRecoveryOut = %s, LogAnesthesiaIn = %s, LogAnesthesiaOut = %s, "
				"LogFacilityIn = %s, LogFacilityOut = %s, "
				"LogSurgeonIn = %s, LogSurgeonOut = %s, Log23HourRoomIn = %s, Log23HourRoomOut = %s, "
				"AnesthMinutes = %s, FacilityMinutes = %s, "
				"Notes = '%s', AppointmentID = %s, DischargeStatusID = %s, AnesType = '%s', "
				"PreOpDx1 = %s, PreOpDx1ICD10 = %s, PreOpDx2 = %s, PreOpDx2ICD10 = %s, PreOpDx3 = %s, PreOpDx3ICD10 = %s, PreOpDx4 = %s, PreOpDx4ICD10 = %s, "
				"PostOpDx1 = %s, PostOpDx1ICD10 = %s, PostOpDx2 = %s, PostOpDx2ICD10 = %s, PostOpDx3 = %s, PostOpDx3ICD10 = %s, PostOpDx4 = %s, PostOpDx4ICD10 = %s "
				"WHERE ID = %li", 
				_Q(m_strName), strCompletedDate, strProviderUpdate, strLocationUpdate, strPOSUpdate, strSurgeryDate, 
				strPreOpIn, strPreOpOut, strOpRoomIn, strOpRoomOut, 
				strRecoveryIn, strRecoveryOut, strAnesthesiaIn, strAnesthesiaOut, 
				strFacilityIn, strFacilityOut, 
				strSurgeonIn, strSurgeonOut, str23HourRoomIn, str23HourRoomOut, 
				strAnesthMinutes, strFacilityMinutes, 
				_Q(strNotes), strAppointmentID, strDischargeStatusID, _Q(strAnesType), 
				arPreOpCodes[0], arPreOpCodes[1], arPreOpCodes[2], arPreOpCodes[3], arPreOpCodes[4], arPreOpCodes[5], arPreOpCodes[6], arPreOpCodes[7], 
				arPostOpCodes[0], arPostOpCodes[1], arPostOpCodes[2], arPostOpCodes[3], arPostOpCodes[4], arPostOpCodes[5], arPostOpCodes[6], arPostOpCodes[7], 
				m_nCaseHistoryID);
		}

		// Generate the string for the MailSent record
		{
			// (j.jones 2008-09-04 13:45) - PLID 30288 - supported MailSentNotesT
			// (j.jones 2011-07-22 15:22) - PLID 21784 - track this as @nMailID
			strMailSentSql.Format("DECLARE @nMailID INT \r\n"
				"SET @nMailID = (SELECT MailID FROM MailSent WHERE PathName = '%s' AND InternalRefID = %li); \r\n"
				"UPDATE MailSentNotesT SET Note = '%s' WHERE MailID = @nMailID;", 
				_Q(PATHNAME_OBJECT_CASEHISTORY), m_nCaseHistoryID, _Q(m_strName));
		}

		// Generate the string for the details
		{
			if (!ValidateAndBuildSaveDetailsUpdateString(strDetailsSql, nAuditTransactionID)) {
				return FALSE;
			}
		}

		// Generate the string for the Procedures
		{
			if(arRemovedProcedureIDs.GetSize()) {
				strProcedureSql.Format("DELETE FROM CaseHistoryProceduresT "
					"WHERE CaseHistoryID = %li AND ProcedureID IN (%s)\r\n",m_nCaseHistoryID, ArrayAsString(arRemovedProcedureIDs,false));
			}
			for(int i = 0; i < arAddedProcedureIDs.GetSize(); i++) {
				CString strInsert;
				strInsert.Format("INSERT INTO CaseHistoryProceduresT (CaseHistoryID, ProcedureID) VALUES (%li, %li)\r\n", m_nCaseHistoryID, arAddedProcedureIDs[i]);
				strProcedureSql += strInsert;
			}
		}

		// (j.jones 2008-02-27 09:13) - PLID 29102 - added CaseHistoryAllocationLinkT
		{
			//did we change the allocation link?
			if(nAllocationID != m_nLastSavedAllocationID) {

				//if we previously had one, we have to remove that link
				if(m_nLastSavedAllocationID != -1) {
					AddStatementToSqlBatch(strCaseHistorySql, 
						"DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID = %li AND CaseHistoryID = %li",
						m_nLastSavedAllocationID, m_nCaseHistoryID);
				}
				
				//if we have a new one, we have to add that link
				if(nAllocationID != -1) {
					AddStatementToSqlBatch(strCaseHistorySql, 
						"INSERT INTO CaseHistoryAllocationLinkT (AllocationID, CaseHistoryID) "
						"VALUES (%li, %li) ",nAllocationID, m_nCaseHistoryID);
				}
			}
		}

		// Combine the individual strings together, and return the result
		strSqlString = strCaseHistorySql + " " + strMailSentSql + " " + strDetailsSql + " " + strProcedureSql;
	}
	return TRUE;
}

// (j.jones 2006-12-13 17:04) - PLID 23578 - made two separate functions for detail saving
// Throws _com_error
//TES 1/8/2007 - PLID 23575 - Added auditing.
BOOL CCaseHistoryDlg::ValidateAndBuildSaveDetailsCreateString(OUT CString &strSqlString, OUT long &nAuditTranactionID)
{
	// Generate the string to insert the new details
	{
		long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
		while (p) {
			NXDATALISTLib::IRowSettingsPtr pRow;
			// Get the row object and move to the next enum
			{
				LPDISPATCH lpDisp = NULL;
				m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
			}
			// Use the row object (guaranteed by the datalist to exist) to generate the update statement
			long nDetailID = VarLong(pRow->GetValue(chdlcID));
			if (nDetailID == MAXLONG) {
				// The "add" row, not a detail at all
			} else if (nDetailID == -1) {
				// A new item
				long nItemID = VarLong(pRow->GetValue(chdlcItemID));
				ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
				double dblQuantity = VarDouble(pRow->GetValue(chdlcQuantity));
				CString strItemAmount = FormatCurrencyForSql(VarCurrency(pRow->GetValue(chdlcAmount),COleCurrency(0,0)));
				// (j.jones 2008-07-01 09:49) - PLID 18744 - use the TrueCost column, which will track costs
				// even if they are not displayed in the visible Cost column, such as on personnel
				CString strItemCost = FormatCurrencyForSql(VarCurrency(pRow->GetValue(chdlcTrueCost),COleCurrency(0,0)));
				BOOL bBillable = VarBool(pRow->GetValue(chdlcBillable),FALSE);
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//BOOL bPayToPractice = VarBool(pRow->GetValue(chdlcPayToPractice),FALSE);

				AddStatementToSqlBatch(strSqlString, "SET @nCaseHistoryDetailID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM CaseHistoryDetailsT WITH(UPDLOCK, HOLDLOCK))");
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				AddStatementToSqlBatch(strSqlString, "INSERT INTO CaseHistoryDetailsT (ID, CaseHistoryID, ItemID, ItemType, Amount, Cost, Quantity, Billable) "
					"SELECT @nCaseHistoryDetailID, @nCaseHistoryID, %li, %li, Convert(money,'%s'), Convert(money,'%s'), %g, %li;",
					nItemID, nItemType, _Q(strItemAmount), _Q(strItemCost), dblQuantity, bBillable?1:0);			

			} else {
				//there shouldn't be already-saved items if this function is called
				ASSERT(FALSE);
			}

			//at this point, the nDetailID is not -1, so we can now link ChargedProductItems to it
			// Generate the string to insert new items to ChargedProductItemsT
			{
				if (nDetailID != MAXLONG) {
					for (int i=0; i<m_aryCaseHistoryDetailProductItems.GetSize(); i++) {
						CaseHistoryDetailProductItemList *addCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
						if(addCHDPI->ID == VarLong(pRow->GetValue(chdlcCHDProductItemListID))) {
							for(int q=0;q<addCHDPI->ProductItemAry.GetSize();q++) {
								if(addCHDPI->ProductItemAry.GetAt(q)->SaveStatus == CHDPI_SAVENEW) {
									AddStatementToSqlBatch(strSqlString, "INSERT INTO ChargedProductItemsT (ProductItemID, CaseHistoryDetailID) VALUES (%li,@nCaseHistoryDetailID);",addCHDPI->ProductItemAry.GetAt(q)->ProductItemID);
									//TES 6/23/2008 - PLID 26152 - Just in case this item was flagged as To Be Returned, it
									// shouldn't be any more.
									AddStatementToSqlBatch(strSqlString, "UPDATE ProductItemsT SET ToBeReturned = 0 WHERE ID = %li", addCHDPI->ProductItemAry.GetAt(q)->ProductItemID);
								}
							}
							break;
						}
					}
				}
			}

		}
	}

	return TRUE;
}

// Throws _com_error
//TES 1/8/2007 - PLID 23575 - Added auditing.
BOOL CCaseHistoryDlg::ValidateAndBuildSaveDetailsUpdateString(OUT CString &strSqlString, OUT long &nAuditTransactionID)
{
	//TODO - the problem with this implementation is that we call NewNumber when we make a new insert statement,
	//but if we batch insert multiple records at once, we get the same number for all of them!
	//we can increment it as a workaround, but what we really need is a SQL function that generates the NewNumber
	//as the batch processes
	long nNewNumber = NewNumber("CaseHistoryDetailsT","ID");

	// Fill these strings
	CString strDeleteChargedProductItems, strDeleteDetails, strUpdateDetails, strInsertDetails, strInsertChargedProductItems;

	// Generate the string to delete any details the user removed
	{
		// Generate a comma-delimited list of IDs to remove
		CString strIDList;
		for (long i=0; i<m_aryDeleteDetailIDs.GetSize(); i++) {
			CString strID;
			//TES 1/10/2007 - PLID 23575 - Audit the deletion.
			long nDetailID = m_aryDeleteDetailIDs.GetAt(i);
			if(nDetailID > 0) {
				//This has a valid ID, meaning it must have been saved at some point.
				CString strName;
				ECaseHistoryDetailItemType ItemType;
				bool bMatched = false;
				for(int nSavedDetail = 0; nSavedDetail < m_arLastSavedDetails.GetSize() && !bMatched; nSavedDetail++) {
					if(m_arLastSavedDetails[nSavedDetail].nCaseHistoryDetailID == nDetailID) {
						bMatched = true;
						strName = m_arLastSavedDetails[nSavedDetail].strName;
						ItemType = m_arLastSavedDetails[nSavedDetail].eType;
					}
				}
				if(bMatched) {
					if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
					AuditEventItems aei;
					switch(ItemType) {
					case chditProduct:
						aei = aeiCaseHistoryProductRemoved;
						break;
					case chditCptCode:
						aei = aeiCaseHistoryServiceCodeRemoved;
						break;
					case chditPerson:
						aei = aeiCaseHistoryPersonnelRemoved;
						break;
					default:
						//What?!
						ASSERT(FALSE);
						AfxThrowNxException(FormatString("Attempted to delete Case History Detail with an invalid type (%i)!", ItemType));
						break;
					}
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aei, m_nCaseHistoryID, strName, "<Deleted>", aepHigh, aetDeleted);
				}
				else {
					//If we get here, that means that somehow we're deleting a detail that we never loaded!
					ASSERT(FALSE);
				}
			}

			strID.Format("%li, ", nDetailID);
			strIDList += strID;
		}
		// If the string isn't empty, generate the sql statement
		if (!strIDList.IsEmpty()) {
			// Since the string isn't empty it's guaranteed to end with an unwanted ", " so drop those two characters
			strIDList.Delete(strIDList.GetLength() - 2, 2);
			// We have our clean list of at least on ID, now generate the sql statement
			strDeleteDetails.Format("DELETE FROM CaseHistoryDetailsT WHERE ID IN (%s);", strIDList);
		}
	}

	// Generate the string to delete anything from ChargedProductItemsT
	{
		CString strIDList;
		for (long i=0; i<m_aryCaseHistoryDetailProductItems.GetSize(); i++) {
			CaseHistoryDetailProductItemList *delCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
			for(int q=0;q<delCHDPI->ProductItemAry.GetSize();q++) {
				if(delCHDPI->ProductItemAry.GetAt(q)->SaveStatus == CHDPI_DELETE) {
					CString strID;
					strID.Format("%li, ", delCHDPI->ProductItemAry.GetAt(q)->ProductItemID);
					strIDList += strID;
				}
			}			
		}

		// If the string isn't empty, generate the sql statement
		if (!strIDList.IsEmpty()) {
			// Since the string isn't empty it's guaranteed to end with an unwanted ", " so drop those two characters
			strIDList.Delete(strIDList.GetLength() - 2, 2);
			// We have our clean list of at least on ID, now generate the sql statement
			strDeleteChargedProductItems.Format("DELETE FROM ChargedProductItemsT WHERE ProductItemID IN (%s);", strIDList);
		}

	}

	// Generate the string to update the quantities on the existing details, as well as the string to insert the new details
	{
		long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
		while (p) {
			NXDATALISTLib::IRowSettingsPtr pRow;
			// Get the row object and move to the next enum
			{
				LPDISPATCH lpDisp = NULL;
				m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
			}
			// Use the row object (guaranteed by the datalist to exist) to generate the update statement
			long nDetailID = VarLong(pRow->GetValue(chdlcID));
			if (nDetailID == MAXLONG) {
				// The "add" row, not a detail at all
			} else if (nDetailID == -1) {
				// A new item
				long nItemID = VarLong(pRow->GetValue(chdlcItemID));
				ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
				double dblQuantity = VarDouble(pRow->GetValue(chdlcQuantity));
				CString strItemAmount = FormatCurrencyForSql(VarCurrency(pRow->GetValue(chdlcAmount),COleCurrency(0,0)));
				// (j.jones 2008-07-01 09:49) - PLID 18744 - use the TrueCost column, which will track costs
				// even if they are not displayed in the visible Cost column, such as on personnel
				CString strItemCost = FormatCurrencyForSql(VarCurrency(pRow->GetValue(chdlcTrueCost),COleCurrency(0,0)));
				BOOL bBillable = VarBool(pRow->GetValue(chdlcBillable),FALSE);
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//BOOL bPayToPractice = VarBool(pRow->GetValue(chdlcPayToPractice),FALSE);

				CString strInsert;
				strInsert.Format(
					"INSERT INTO CaseHistoryDetailsT (ID, CaseHistoryID, ItemID, ItemType, Amount, Cost, Quantity, Billable) "
					"SELECT %li, %li, %li, %li, Convert(money,'%s'), Convert(money,'%s'), %g, %li;", nDetailID = nNewNumber++,
					m_nCaseHistoryID, nItemID, nItemType, _Q(strItemAmount), _Q(strItemCost), dblQuantity, bBillable?1:0);
				// Add this update statement to the string of update statements
				strInsertDetails += strInsert + " ";

				//TES 1/10/2007 - PLID 23575 - Audit this new detail
				if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
				AuditEventItems aei;
				switch(nItemType) {
				case chditProduct:
					aei = aeiCaseHistoryProductAdded;
					break;
				case chditCptCode:
					aei = aeiCaseHistoryServiceCodeAdded;
					break;
				case chditPerson:
					aei = aeiCaseHistoryPersonnelAdded;
					break;
				default:
					//What?!
					ASSERT(FALSE);
					AfxThrowNxException(FormatString("Attempted to save Case History Detail with an invalid type (%i)!", nItemType));
					break;
				}
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aei, m_nCaseHistoryID, "", VarString(pRow->GetValue(chdlcName),""), aepMedium, aetCreated);

			} else {
				// Not a new item, so it's an existing item
				CString strUpdate;
				double dblQuantity = VarDouble(pRow->GetValue(chdlcQuantity));
				COleCurrency cyAmount = VarCurrency(pRow->GetValue(chdlcAmount),COleCurrency(0,0));
				CString strItemAmount = FormatCurrencyForSql(cyAmount);
				// (j.jones 2008-07-01 09:49) - PLID 18744 - use the TrueCost column, which will track costs
				// even if they are not displayed in the visible Cost column, such as on personnel
				COleCurrency cyCost = VarCurrency(pRow->GetValue(chdlcTrueCost),COleCurrency(0,0));
				CString strItemCost = FormatCurrencyForSql(cyCost);
				BOOL bBillable = VarBool(pRow->GetValue(chdlcBillable),FALSE);
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//BOOL bPayToPractice = VarBool(pRow->GetValue(chdlcPayToPractice),FALSE);

				//TES 1/10/2007 - PLID 23575 - Audit changes.
				// First, find the corresponding detail in our "last saved" details.
				SavedCaseHistoryDetail schd;
				bool bMatched = false;
				for(int nSavedDetail = 0; nSavedDetail < m_arLastSavedDetails.GetSize() && !bMatched; nSavedDetail++) {
					if(m_arLastSavedDetails[nSavedDetail].nCaseHistoryDetailID == nDetailID) {
						bMatched = true;
						schd = m_arLastSavedDetails[nSavedDetail];
					}
				}
				if(bMatched) {
					if(schd.dblQuantity != dblQuantity) {
						if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryItemQuantity, m_nCaseHistoryID, schd.strName + " - " + AsString(schd.dblQuantity), schd.strName + " - " + AsString(dblQuantity), aepMedium, aetChanged);
					}
					if(schd.cyAmount != cyAmount) {
						if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryItemAmount, m_nCaseHistoryID, schd.strName + " - " + FormatCurrencyForInterface(schd.cyAmount), schd.strName + " - " + FormatCurrencyForInterface(cyAmount), aepMedium, aetChanged);
					}
					if(schd.cyCost != cyCost) {
						if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryItemCost, m_nCaseHistoryID, schd.strName + " - " + FormatCurrencyForInterface(schd.cyCost), schd.strName + " - " + FormatCurrencyForInterface(cyCost), aepMedium, aetChanged);
					}
					if(schd.bBillable != bBillable) {
						if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryItemBillable, m_nCaseHistoryID, schd.strName + " - " + CString(schd.bBillable ? "Billable" : "Not Billable"), schd.strName + " - " + CString(bBillable ? "Billable" : "Not Billable"), aepMedium, aetChanged);
					}
					// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
					/*
					if(schd.bPayToPractice != bPayToPractice) {
						if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryItemPayToPractice, m_nCaseHistoryID, schd.strName + " - " + CString(schd.bPayToPractice ? "Pay To Practice" : "Do Not Pay To Practice"), schd.strName + " - " + CString(bPayToPractice ? "Pay To Practice" : "Do Not Pay To Practice"), aepMedium, aetChanged);
					}
					*/
				}
				else {
					//If we get here, that means that somehow we're updating a detail that we never loaded!
					ASSERT(FALSE);
				}
				

				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				strUpdate.Format("UPDATE CaseHistoryDetailsT SET Quantity = %g, Amount = Convert(money,'%s'), "
					"Cost = Convert(money,'%s'), Billable = %li WHERE ID = %li;", dblQuantity, _Q(strItemAmount),
					_Q(strItemCost), bBillable?1:0, nDetailID);
				// Add this update statement to the string of update statements
				strUpdateDetails += strUpdate + " ";
			}

			//at this point, the nDetailID is not -1, so we can now link ChargedProductItems to it
			// Generate the string to insert new items to ChargedProductItemsT
			{
				if (nDetailID != MAXLONG) {
					for (int i=0; i<m_aryCaseHistoryDetailProductItems.GetSize(); i++) {
						CaseHistoryDetailProductItemList *addCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
						if(addCHDPI->ID == VarLong(pRow->GetValue(chdlcCHDProductItemListID))) {
							for(int q=0;q<addCHDPI->ProductItemAry.GetSize();q++) {
								if(addCHDPI->ProductItemAry.GetAt(q)->SaveStatus == CHDPI_SAVENEW) {
									CString strInsert;
									strInsert.Format("INSERT INTO ChargedProductItemsT (ProductItemID,CaseHistoryDetailID) VALUES (%li,%li);",addCHDPI->ProductItemAry.GetAt(q)->ProductItemID,nDetailID);
									// Add this update statement to the string of update statements
									strInsertChargedProductItems += strInsert + " ";
									//TES 6/23/2008 - PLID 26152 - Just in case this item was flagged as To Be Returned, it
									// shouldn't be any more.
									strInsert.Format("UPDATE ProductItemsT SET ToBeReturned = 0 WHERE ID = %li", addCHDPI->ProductItemAry.GetAt(q)->ProductItemID);
									strInsertChargedProductItems += strInsert + " ";
									
								}
							}
							break;
						}
					}
				}
			}

		}
		// If the string isn't empty, then it is guaranteed to end in a space, just drop that space because we don't care about it
		if (!strUpdateDetails.IsEmpty()) {
			strUpdateDetails.Delete(strUpdateDetails.GetLength() - 1, 1);
		}
		// If the string isn't empty, then it is guaranteed to end in a space, just drop that space because we don't care about it
		if (!strInsertDetails.IsEmpty()) {
			strInsertDetails.Delete(strInsertDetails.GetLength() - 1, 1);
		}
	}

	if (!strInsertChargedProductItems.IsEmpty()) {
		strInsertChargedProductItems.Delete(strInsertChargedProductItems.GetLength() - 1, 1);
	}

	// Combine all the strings together and return success
	strSqlString = strDeleteChargedProductItems + " " + strDeleteDetails + " " + strUpdateDetails + " " + strInsertDetails + " " + strInsertChargedProductItems;
	return TRUE;
}

// (j.jones 2006-12-13 15:47) - PLID 23578 - created ability to create a new, unsaved, case history
// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
int CCaseHistoryDlg::OpenNewCase(long nPersonID, CArray<long, long> &arynCopyFromPreferenceCardIDs, const CString &strName, const long nProviderID, const COleDateTime &dtSurgeryDate, const long nAppointmentID /*= -1*/)
{
	m_nCaseHistoryID = -1;
	m_nPersonID = nPersonID;

	m_arynDefaultPreferenceCardIDs.RemoveAll();
	m_arynDefaultPreferenceCardIDs.Append(arynCopyFromPreferenceCardIDs);

	m_strDefaultName = strName;
	m_nDefaultProviderID = nProviderID;	
	m_dtDefaultSurgeryDate = dtSurgeryDate;
	m_nAppointmentID = nAppointmentID;
	m_bIsNew = TRUE;

	// (j.jones 2009-08-26 09:53) - PLID 34943 - Use the preference to determine how the location
	// should be defaulted. -1 for current location, -2 for patient location, -3 for appointment location,
	// and a positive number is an actual location ID
	m_nCurLocationID = GetCurrentLocationID();

	long nDefaultCaseHistoryLocation = GetRemotePropertyInt("DefaultCaseHistoryLocation", -1, 0, "<None>", true);
	if(nDefaultCaseHistoryLocation == -2) {
		//use G2 location, confirm it is still active
		_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.ID FROM LocationsT "
			"INNER JOIN PersonT ON LocationsT.ID = PersonT.Location "
			"WHERE PersonT.ID = {INT} "
			"AND LocationsT.Managed = 1 AND LocationsT.TypeID = 1 AND LocationsT.Active = 1", m_nPersonID);
		if(!rs->eof) {
			m_nCurLocationID = AdoFldLong(rs, "ID");
		}
		rs->Close();
	}
	else if(nDefaultCaseHistoryLocation == -3 && m_nAppointmentID != -1) {
		//use appt. location, but only if it is managed
		_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.ID FROM LocationsT "
			"INNER JOIN AppointmentsT ON LocationsT.ID = AppointmentsT.LocationID "
			"WHERE AppointmentsT.ID = {INT} "
			"AND LocationsT.Managed = 1 AND LocationsT.TypeID = 1 AND LocationsT.Active = 1", m_nAppointmentID);
		if(!rs->eof) {
			m_nCurLocationID = AdoFldLong(rs, "ID");
		}
		rs->Close();
	}
	else if(nDefaultCaseHistoryLocation > 0) {
		//ensure the default location is an active, managed location
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM LocationsT "
			"WHERE ID = {INT} AND Managed = 1 AND TypeID = 1 AND Active = 1", nDefaultCaseHistoryLocation);
		if(!rs->eof) {
			m_nCurLocationID = AdoFldLong(rs, "ID");
		}
		rs->Close();
	}

	return CDialog::DoModal();
}

// (j.jones 2006-12-13 15:47) - PLID 23578 - reworked the code used to open an existing case history
int CCaseHistoryDlg::OpenExistingCase(long nCaseHistoryID)
{
	m_nCaseHistoryID = nCaseHistoryID;
	m_bIsNew = FALSE;
	return CDialog::DoModal();
}

// Fills the given parameters with good guesses of what the new IDs would be.
// NOTE: This function does not create any records, therefore it cannot guarantee these IDs will 
//		 still be unique because it can't know how long you will wait to create the records.
// Throws _com_error
// If an exception is thrown, none of the given variables will be written to
void CalcNewCaseHistoryIDs(OUT long &nCaseHistoryID, OUT long &nMailID, OUT long &nMailBatchID)
{
	// Store the values in local variables
	long nLocalCaseHistoryID = NewNumber("CaseHistoryT", "ID");
	_RecordsetPtr prs = CreateRecordset("SELECT MAX(MailID) AS MaxMailID, MAX(MailBatchID) AS MaxMailBatchID FROM MailSent");
	FieldsPtr flds = prs->GetFields();
	long nLocalMailID = AdoFldLong(flds, "MaxMailID", 0) + 1;
	long nLocalMailBatchID = AdoFldLong(flds, "MaxMailBatchID", 0) + 1;

	// Now we have our numbers, so set the OUT parameters
	nCaseHistoryID = nLocalCaseHistoryID;
	nMailID = nLocalMailID;
	nMailBatchID = nLocalMailBatchID;
}

// Deletes the case history
// Throws _com_error, CNxException*
void CCaseHistoryDlg::DeleteCaseHistory(long nID, BOOL bDeleteMailSent /* = TRUE */)
{
	long nAuditTransactionID = -1;

	try {

		// TODO: Make CCaseHistoryDlg::DeleteCaseHistory use the network messages
		BEGIN_TRANS("CCaseHistoryDlg::DeleteCaseHistory") {

			// (j.jones 2007-02-20 17:08) - PLID 24164 - we can now delete case histories linked to bills
			// if the bills have been marked deleted, so in that case, update deleted bills referencing this case
			// (j.jones 2009-08-06 10:22) - PLID 35120 - supported BilledCaseHistoriesT
			ExecuteParamSql("DELETE FROM BilledCaseHistoriesT WHERE CaseHistoryID = {INT}", nID);

			// Delete attached Product Items
			ExecuteSql("DELETE FROM ChargedProductItemsT WHERE CaseHistoryDetailID IN (SELECT ID FROM CaseHistoryDetailsT WHERE CaseHistoryID = %li)", nID);

			// Delete the case history procedures
			ExecuteSql("DELETE FROM CaseHistoryProceduresT WHERE CaseHistoryID = %li", nID);

			// Delete the case history details
			ExecuteSql("DELETE FROM CaseHistoryDetailsT WHERE CaseHistoryID = %li", nID);

			//TES 1/10/2007 - PLID 23575 - Audit this deletion.
			long nPersonID = -1;
			CString strCaseHistoryName;
			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr rsCaseHistoryInfo = CreateParamRecordset("SELECT PersonID, Name FROM CaseHistoryT WHERE ID = {INT}", nID);
			if(rsCaseHistoryInfo->eof) {
				ASSERT(FALSE);
			}
			else {
				nPersonID = AdoFldLong(rsCaseHistoryInfo, "PersonID");
				strCaseHistoryName = AdoFldString(rsCaseHistoryInfo, "Name");
			}
			rsCaseHistoryInfo->Close();

			nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditTransactionID, aeiCaseHistoryDeleted, nID, strCaseHistoryName, "<Deleted>", aepHigh, aetDeleted);

			// (j.jones 2008-02-27 10:30) - PLID 29104 - audit the allocation link deletion			
			_RecordsetPtr rsAllocationInfo = CreateParamRecordset("SELECT PatientInvAllocationsT.ID, InputDate "
				"FROM PatientInvAllocationsT "
				"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
				"WHERE PatientInvAllocationsT.Status <> {INT} "
				"AND CaseHistoryAllocationLinkT.CaseHistoryID = {INT}", InvUtils::iasDeleted, nID);
			//with the current design, there should never be more than one linked allocation, but the 
			//structure allows it, so let's make it a while loop for accuracy
			while(!rsAllocationInfo->eof) {
				long nAllocationID = AdoFldLong(rsAllocationInfo, "ID");
				COleDateTime dtInput = AdoFldDateTime(rsAllocationInfo, "InputDate");

				CString strAllocationDescription;
				strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(dtInput, NULL, dtoDate));

				//need to audit that we removed the link from both the case history and the allocation
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nID, strAllocationDescription, strCaseHistoryName + ", No Linked Allocation", aepMedium, aetDeleted);
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, strAllocationDescription + ", Case History: " + strCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
				rsAllocationInfo->MoveNext();
			}
			rsAllocationInfo->Close();
			
			// (j.jones 2008-02-27 10:05) - PLID 29102 - delete the allocation link
			ExecuteParamSql("DELETE FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID = {INT}", nID);

			// (j.jones 2009-08-06 10:22) - PLID 7397 - unlink from ProcInfoT
			ExecuteParamSql("UPDATE ProcInfoT SET CaseHistoryID = NULL WHERE CaseHistoryID = {INT}", nID);

			// Delete the case history record
			ExecuteSql("DELETE FROM CaseHistoryT WHERE ID = %li", nID);

			// Delete the mailsent record
			if(bDeleteMailSent) {
				// (a.walling 2006-11-29 12:11) - PLID 23700 - Audit this removal from mailsent.
				// (j.jones 2008-09-04 17:43) - PLID 30288 - supported MailSentDetailsT
				_RecordsetPtr prs = CreateParamRecordset("SELECT Note FROM MailSentNotesT WHERE MailID IN (SELECT MailID FROM MailSent WHERE PathName = {STRING} AND InternalRefID = {INT})", PATHNAME_OBJECT_CASEHISTORY, nID);
				CString strOld = "Case History: ";
				CString strPersonName;
				if (prs->eof) {
					ASSERT(FALSE);
				} else {
					strOld += AdoFldString(prs, "Note", "");
					strPersonName = GetExistingPatientName(nPersonID); // this queries PersonT if not found in patient toolbar
					if (strPersonName.GetLength() == 0) {
						ASSERT(FALSE);
					}
				}

				AuditEvent(nPersonID, strPersonName, nAuditTransactionID, aeiPatientDocDetach, nID, strOld, "<Detached>", aepHigh, aetDeleted);

				// (j.jones 2011-07-22 15:32) - PLID 21784 - this should only update one MailSent ID at most - get that ID.
				_RecordsetPtr rs = CreateParamRecordset("SELECT MailID FROM MailSent WHERE PathName = {STRING} AND InternalRefID = {INT}", PATHNAME_OBJECT_CASEHISTORY, nID);
				if(!rs->eof) {
					long nMailSentID = VarLong(rs->Fields->Item["MailID"]->Value);

					CString strSqlBatch = BeginSqlBatch();
					// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
					AddDeleteMailSentQueryToSqlBatch(strSqlBatch, AsString(nMailSentID));
					ExecuteSqlBatch(strSqlBatch);

					// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
					// we have to send those as well.
					// (j.jones 2011-07-22 15:32) - PLID 21784 - update only the ID we changed
					// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
					CClient::RefreshMailSentTable(nPersonID, nMailSentID);
					// We don't support making todos for case histories currently
					//CClient::RefreshTable(NetUtils::TodoList, -1);
				}
				rs->Close();
			}

			//we always have an audit transaction here
			CommitAuditTransaction(nAuditTransactionID);

		} END_TRANS("CCaseHistoryDlg::DeleteCaseHistory");

	//this continues the existing functionality of throwing the exception to the caller,
	//while still cleaning up our audit transaction
	} NxCatchAllSilentCallThrow(
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

BOOL CCaseHistoryDlg::IsCaseHistoryBilled(IN const long nCaseHistoryID)
{
	// (j.jones 2007-02-20 17:08) - PLID 24164 - changed so we ignore deleted bills, which the deletion code will handle
	// (j.jones 2009-08-06 10:22) - PLID 35120 - supported BilledCaseHistoriesT
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM BillsT "
		"INNER JOIN BilledCaseHistoriesT ON BillsT.ID = BilledCaseHistoriesT.BillID "
		"WHERE CaseHistoryID = {INT} AND BillsT.EntryType = 1 AND Deleted = 0", nCaseHistoryID);
	if(rs->eof) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (j.jones 2009-08-06 11:10) - PLID 7397 - added check to see if the case is linked to a PIC
BOOL CCaseHistoryDlg::IsCaseHistoryInProcInfo(IN const long nCaseHistoryID)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM ProcInfoT WHERE CaseHistoryID = {INT}", nCaseHistoryID);
	if(rs->eof) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// Returns the comma-delimited list of procedure names reprsented by the cpt details of the given surgery
// Throws _com_error
// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
CString CCaseHistoryDlg::GuessProcedureListFromPreferenceCard(long nPreferenceCardID)
{
	// Open the recordset
	// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
	// (c.haag 2008-12-26 11:36) - PLID 32539 - I changed no code here. If we have an existing surgery, then
	// we need to pull all procedures; even inactive ones.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT Name "
		"FROM ProcedureT INNER JOIN ("
		"   SELECT DISTINCT ServiceT.ProcedureID, PreferenceCardDetailsT.PreferenceCardID "
		"   FROM PreferenceCardDetailsT LEFT JOIN ServiceT ON PreferenceCardDetailsT.ServiceID = ServiceT.ID "
		"   WHERE ProcedureID IS NOT NULL AND PreferenceCardID = {INT}"
		") CardProceduresQ ON ProcedureT.ID = CardProceduresQ.ProcedureID",
		nPreferenceCardID);

	// Loop through the recordset, generating the string
	CString strAns;
	FieldsPtr flds = prs->GetFields();
	while (!prs->eof) {
		strAns += AdoFldString(flds, "Name") + ", ";
		prs->MoveNext();
	}

	// If there are more than 0 characters, then the last two characters are the annoying ", " so delete them
	if (!strAns.IsEmpty()) {
		strAns.Delete(strAns.GetLength() - 2, 2);
	}

	// Return the list
	return strAns;
}

// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
// (j.jones 2009-08-31 17:54) - PLID 17734 - this optionally takes in an appointment ID
BOOL CCaseHistoryDlg::ChoosePreferenceCards(CArray<long, long> &arynPreferenceCardIDs, OUT CString &strNewCaseHistoryName, OUT long &nProviderID, long nPatientID, long nAppointmentID /*= -1*/)
{
	CCaseHistorySelectSurgeryDlg dlg(NULL);
	// (c.haag 2007-03-09 11:10) - PLID 25138 - Specify the patient ID for use in internal calculations
	dlg.SetPatientID(nPatientID);
	dlg.m_nAppointmentID = nAppointmentID;
	if (dlg.DoModal() == IDOK) {
		strNewCaseHistoryName = dlg.m_strNewCaseHistoryName;
		nProviderID = dlg.m_nProviderID;

		arynPreferenceCardIDs.RemoveAll();
		arynPreferenceCardIDs.Append(dlg.m_arynPreferenceCardIDs);
		return TRUE;
	} else {
		return FALSE;
	}
}

// (j.jones 2008-02-27 17:09) - PLID 29108 - calls Save and closes the dialog
BOOL CCaseHistoryDlg::SaveAndClose()
{
	CWaitCursor pWait;

	if(Save()) {
		GetMainFrame()->UnregisterForBarcodeScan(this);
		CDialog::OnOK();

		//if this is a tracked case history, from the ASC module,
		//update the parent's view
		if(m_pParent) {
			m_pParent->OnClosedCaseHistory(m_nCaseHistoryID);
		}

		return TRUE;
	}
	else {
		return FALSE;
	}

	//any errors would be passed to the calling function	
}


void CCaseHistoryDlg::OnOK() 
{
	try {
		
		// (j.jones 2008-02-27 17:12) - PLID 29108 - SaveAndClose will cleanly
		// exit the dialog if successful
		if(!SaveAndClose()) {
			return;
		}

	} NxCatchAll("CCaseHistoryDlg::OnOK");
}

BEGIN_EVENTSINK_MAP(CCaseHistoryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCaseHistoryDlg)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCasehistoryDetailsList, VTS_I2)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 6 /* RButtonDown */, OnRButtonDownCasehistoryDetailsList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 4 /* LButtonDown */, OnLButtonDownCasehistoryDetailsList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_PROVIDER_LIST, 2 /* SelChanged */, OnSelChangedProviderList, VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_PREOP_IN_EDIT, 1 /* KillFocus */, OnKillFocusPreopInEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_OPROOM_IN_EDIT, 1 /* KillFocus */, OnKillFocusOproomInEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_OPROOM_OUT_EDIT, 1 /* KillFocus */, OnKillFocusOproomOutEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_PREOP_OUT_EDIT, 1 /* KillFocus */, OnKillFocusPreopOutEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_RECOVERY_IN_EDIT, 1 /* KillFocus */, OnKillFocusRecoveryInEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_RECOVERY_OUT_EDIT, 1 /* KillFocus */, OnKillFocusRecoveryOutEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_ANESTHESIA_START_EDIT, 1 /* KillFocus */, OnKillFocusAnesthesiaStartEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_ANESTHESIA_STOP_EDIT, 1 /* KillFocus */, OnKillFocusAnesthesiaStopEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_FACILITY_START_EDIT, 1 /* KillFocus */, OnKillFocusFacilityStartEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_FACILITY_STOP_EDIT, 1 /* KillFocus */, OnKillFocusFacilityStopEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 9 /* EditingFinishing */, OnEditingFinishingCasehistoryDetailsList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 10 /* EditingFinished */, OnEditingFinishedCasehistoryDetailsList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 8 /* EditingStarting */, OnEditingStartingCasehistoryDetailsList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenLocationCombo, VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_POS_COMBO, 2 /* SelChanged */, OnSelChangedPOSCombo, VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 17 /* ColumnClicking */, OnColumnClickingCasehistoryDetailsList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CCaseHistoryDlg, IDC_SURGEON_IN_EDIT, 1 /* KillFocus */, OnKillFocusSurgeonInEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_SURGEON_OUT_EDIT, 1 /* KillFocus */, OnKillFocusSurgeonOutEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_23HOURROOM_IN_EDIT, 1 /* KillFocus */, OnKillFocus23hourroomInEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_23HOURROOM_OUT_EDIT, 1 /* KillFocus */, OnKillFocus23hourroomOutEdit, VTS_NONE)
	ON_EVENT(CCaseHistoryDlg, IDC_PROCEDURE_LIST, 16 /* SelChosen */, OnSelChosenProcedureList, VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_POS_COMBO, 16 /* SelChosen */, OnSelChosenCaseHistoryPosCombo, VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASEHISTORY_DETAILS_LIST, 23 /* ChangeColumnSortFinished */, OnChangeColumnSortFinishedCasehistoryDetailsList, VTS_I2 VTS_BOOL VTS_I2 VTS_BOOL)
	ON_EVENT(CCaseHistoryDlg, IDC_DISCHARGE_STATUS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDischargeStatusCombo, VTS_I2)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_ALLOCATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCaseAllocationList, VTS_I2)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_LOCATION_COMBO, 1 /* SelChanging */, OnSelChangingCaseHistoryLocationCombo, VTS_PI4)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_ALLOCATION_LIST, 1 /* SelChanging */, OnSelChangingCaseAllocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_ALLOCATION_LIST, 16 /* SelChosen */, OnSelChosenCaseAllocationList, VTS_DISPATCH)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_ANES_TYPE, 1 /* SelChanging */, OnSelChangingCaseHistoryAnesType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CCaseHistoryDlg, IDC_CASE_HISTORY_ANES_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedCaseHistoryAnesType, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CCaseHistoryDlg, IDC_PREOP_DIAGNOSIS_SEARCH, 16, CCaseHistoryDlg::SelChosenPreopDiagnosisSearch, VTS_DISPATCH)
	ON_EVENT(CCaseHistoryDlg, IDC_POSTOP_DIAGNOSIS_SEARCH, 16, CCaseHistoryDlg::SelChosenPostopDiagnosisSearch, VTS_DISPATCH)
	ON_EVENT(CCaseHistoryDlg, IDC_PREOP_DIAGNOSIS_LIST, 6, CCaseHistoryDlg::RButtonDownPreopDiagnosisList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCaseHistoryDlg, IDC_POSTOP_DIAGNOSIS_LIST, 6, CCaseHistoryDlg::RButtonDownPostopDiagnosisList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CCaseHistoryDlg::OnEditingFinishingCasehistoryDetailsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	if(nCol == chdlcQuantity && pvarNewValue->vt == VT_R8) {

		double dblNewQuantity = VarDouble(pvarNewValue, 0.0);

		if(dblNewQuantity <= 0.0) {
			*pvarNewValue = varOldValue;
			*pbCommit = FALSE;
			MessageBox("You must have a quantity greater than zero.");
			return;
		}

		// (j.jones 2008-05-21 08:56) - PLID 29249 - do we require a non-decimal quantity?
		if(VarBool(m_dlCaseHistoryDetails->GetValue(nRow, chdlcIsSerializedProduct), FALSE)			
			&& (long)dblNewQuantity != dblNewQuantity) {

			*pvarNewValue = varOldValue;
			*pbCommit = FALSE;

			MessageBox("This product must have a whole number for its quantity, as it requires a serial number / expiration date.");
			return;
		}
	}
}

void CCaseHistoryDlg::OnEditingFinishedCasehistoryDetailsList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow==-1)
		return;

	switch(nCol) {

		case chdlcQuantity: {

			if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {
				double dblQuantity = varNewValue.dblVal;
				if(!ChangeProductItems(nRow,dblQuantity)) {
					//if they didn't change the items, we must set to the old quantity
					m_dlCaseHistoryDetails->PutValue(nRow,chdlcQuantity,varOldValue.dblVal);
				}
			}

			break;
		}

		case chdlcVisibleCost: {
			// (j.jones 2008-07-01 10:44) - PLID 18744 - also copy to the TrueCost column
			m_dlCaseHistoryDetails->PutValue(nRow, chdlcTrueCost, varNewValue);
			break;
		}
	}

	if(nCol == chdlcBillable && varNewValue.vt == VT_BOOL && VarBool(varNewValue)) {

		long nItemType = VarLong(m_dlCaseHistoryDetails->GetValue(nRow, chdlcItemType), -1);

		if(nItemType == -1) {
			long nProductID = VarLong(m_dlCaseHistoryDetails->GetValue(nRow, chdlcItemID), -1);

			// (j.jones 2007-02-21 11:06) - PLID 24846 - if enabling Billable, and the item is a
			// product, warn the user if the product is marked as not billable for the current location
			// (j.jones 2008-09-15 11:08) - PLID 31375 - converted into a parameterized query
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 Billable "
				"FROM ProductLocationInfoT "
				"WHERE Billable = 1 AND ProductID = {INT} AND LocationID = {INT}", nProductID, m_nCurLocationID);
			BOOL bBillableForLocation = FALSE;
			if(rs->eof) {
				//the product is not billable for this location - warn, but allow the change
				// (a.walling 2007-02-26 16:13) - PLID 24943 - Use MessageBox so we know 'this' is the parent.
				MessageBox("Warning: this product is not marked as billable for the currently selected location.\n"
					"The product will not be billed unless corrected in Inventory, or the Case History location is changed.");
			}
			rs->Close();
		}
	}
}

void CCaseHistoryDlg::OnEditProductItems() {
	if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {
		ChangeProductItems(m_dlCaseHistoryDetails->GetCurSel(),-1);
	}
}

void CCaseHistoryDlg::OnRequeryFinishedCasehistoryDetailsList(short nFlags) 
{
	try {

		// (j.jones 2008-07-01 10:02) - PLID 18744 - track this permission rather
		// than checking it during each iteration of the loop
		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

		//Color the rows
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		int i = 0;
		for(i=0;i<m_dlCaseHistoryDetails->GetRowCount();i++) {

			long ItemType = m_dlCaseHistoryDetails->GetValue(i,chdlcItemType).lVal;

			if(ItemType == -3) {
				//color the Person row yellow-orange, like the Contacts module
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));

				//color the Amount, Billable, and PayToPractice columns gray
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutCellBackColor(chdlcAmount,RGB(230,230,230));
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutCellBackColor(chdlcBillable,RGB(230,230,230));
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutCellBackColor(chdlcPayToPractice,RGB(230,230,230));

				// (j.jones 2008-07-01 09:51) - PLID 18744 - the visible cost column loads as null by default for personnel,
				// so we must copy the true cost into the visible cost column, but only if they have read permissions
				if(bCanViewPersonCosts) {
					_variant_t varCost = m_dlCaseHistoryDetails->GetValue(i, chdlcTrueCost);
					m_dlCaseHistoryDetails->PutValue(i, chdlcVisibleCost, varCost);
				}
				else {
					//color the cost column gray
					NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutCellBackColor(chdlcVisibleCost,RGB(230,230,230));
				}
			}
			else if(ItemType == -2) {
				//color the CPT row red, like the Admin. module
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));

				//color the Cost column gray
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutCellBackColor(chdlcVisibleCost,RGB(230,230,230));
			}
			else if(ItemType == -1) {
				//color the Product row blue, like the Inventory module
				NXDATALISTLib::IRowSettingsPtr(m_dlCaseHistoryDetails->GetRow(i))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
			}

			if(!m_bLastSavedDetailsLoaded) {
				//TES 1/10/2007 - PLID 23575 - This must be our first time loading the list, so keep track of the initial
				// details, for auditing later.
				SavedCaseHistoryDetail schd;
				schd.nCaseHistoryDetailID = VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcID));
				schd.eType = (ECaseHistoryDetailItemType)ItemType;
				schd.strName = VarString(m_dlCaseHistoryDetails->GetValue(i,chdlcName),"");
				schd.dblQuantity = VarDouble(m_dlCaseHistoryDetails->GetValue(i,chdlcQuantity),0.0);
				schd.cyAmount = VarCurrency(m_dlCaseHistoryDetails->GetValue(i,chdlcAmount),COleCurrency(0,0));
				// (j.jones 2008-07-01 09:49) - PLID 18744 - use the TrueCost column, which will track costs
				// even if they are not displayed in the visible Cost column, such as on personnel
				schd.cyCost = VarCurrency(m_dlCaseHistoryDetails->GetValue(i,chdlcTrueCost),COleCurrency(0,0));
				schd.bBillable = VarBool(m_dlCaseHistoryDetails->GetValue(i,chdlcBillable),FALSE);
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//schd.bPayToPractice = VarBool(m_dlCaseHistoryDetails->GetValue(i,chdlcPayToPractice),FALSE);
				m_arLastSavedDetails.Add(schd);
			}
		}
		//We have now loaded our m_arLastSavedDetails array.
		m_bLastSavedDetailsLoaded = true;

		_variant_t varNull;
		varNull.vt = VT_NULL;

		NXDATALISTLib::IRowSettingsPtr pRow = m_dlCaseHistoryDetails->GetRow(-1);
		pRow->PutValue(chdlcID, (long)MAXLONG);
		pRow->PutValue(chdlcAction, _bstr_t("Add"));
		pRow->PutValue(chdlcQuantity, varNull);
		pRow->PutValue(chdlcItemID, varNull);
		pRow->PutValue(chdlcItemType, varNull);
		pRow->PutValue(chdlcName, varNull);
		pRow->PutValue(chdlcCHDProductItemListID, varNull);
		// (j.jones 2008-05-20 17:26) - PLID 29249 - added the chdlcIsSerializedProduct column
		pRow->PutValue(chdlcIsSerializedProduct, varNull);
		//TES 7/16/2008 - PLID 27983 - Added the IsSerializedProduct column
		pRow->PutValue(chdlcIsSerializedProduct, varNull);

		pRow->PutBackColor(PaletteColor(GetSysColor(COLOR_WINDOW)));
		pRow->PutBackColorSel(PaletteColor(GetSysColor(COLOR_WINDOW)));

		m_dlCaseHistoryDetails->InsertRow(pRow, m_dlCaseHistoryDetails->GetRowCount());

		//load the ChargedProductItems
		for(i=0;i<m_dlCaseHistoryDetails->GetRowCount()-1;i++) {
			long CaseHistoryDetailID = m_dlCaseHistoryDetails->GetValue(i,chdlcID).lVal;
			long CaseHistoryDetailProductItemListID = m_dlCaseHistoryDetails->GetValue(i,chdlcCHDProductItemListID).lVal;
			if(CaseHistoryDetailProductItemListID == -2) {
				//if -2, it means there are items to load, otherwise leave alone
				CaseHistoryDetailProductItemListID = LoadIntoCaseHistoryDetailProductItemsArray(CaseHistoryDetailID);
				m_dlCaseHistoryDetails->PutValue(i,chdlcCHDProductItemListID,CaseHistoryDetailProductItemListID);
			}			
		}

	} NxCatchAll("CCaseHistoryDlg::OnRequeryFinishedCasehistoryDetailsList");
}

void CCaseHistoryDlg::OnRButtonDownCasehistoryDetailsList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// We want to auto-select the item that's being right-clicked on so the context menu works how we want
		GetDlgItem(IDC_CASEHISTORY_DETAILS_LIST)->SetFocus();
		m_dlCaseHistoryDetails->PutCurSel(nRow);
	} NxCatchAll("CCaseHistoryDlg::OnRButtonDownCasehistoryDetailsList");
}

void CCaseHistoryDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{	
	try {
		// Is the focus on the details list?
		CWnd *pDetailsWnd = GetDlgItem(IDC_CASEHISTORY_DETAILS_LIST);
		if (pWnd->GetSafeHwnd() == pDetailsWnd->GetSafeHwnd()) {
			// Yes, so show the context menu
			CMenu mnu;
			mnu.LoadMenu(IDR_CASE_HISTORY_POPUP);
			CMenu *pmnuSub = mnu.GetSubMenu(0);
			if (pmnuSub) {
				// Make sure we have an appropriate place to pop up the menu
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);
					if (!rc.PtInRect(point)) {
						point.x = rc.left+5;
						point.y = rc.top+5;
					}
				}
				// Hide certain items if we're not on a row
				long nCurSel = m_dlCaseHistoryDetails->GetCurSel();
				if (nCurSel == -1 || nCurSel == (m_dlCaseHistoryDetails->GetRowCount() - 1)) {
					pmnuSub->EnableMenuItem(ID_CASEHISTORYDETAILS_REMOVE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);					
					pmnuSub->EnableMenuItem(ID_CASEHISTORYDETAILS_EDITPRODITEMS, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				}
				else {
					if(VarLong(m_dlCaseHistoryDetails->GetValue(nCurSel,chdlcCHDProductItemListID),-1) == -1)
						pmnuSub->EnableMenuItem(ID_CASEHISTORYDETAILS_EDITPRODITEMS, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				}

				// Show the popup
				pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
			}
		}
	} NxCatchAll("CCaseHistoryDlg::OnContextMenu");
}

// (j.jones 2006-12-13 16:13) - PLID 23578 - added modularity to this functionality
// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
void CCaseHistoryDlg::AddPreferenceCard(long nPreferenceCardID)
{
	BOOL bWarnedLicenses = FALSE;

	BOOL bCheckAndWarnCredentials = GetRemotePropertyInt("CredentialWarnCaseHistories",0,0,"<None>",true) == 1;
	BOOL bCheckAndWarnLicensing = GetRemotePropertyInt("ExpLicenseWarnCaseHistories",bCheckAndWarnCredentials ? 1 : 0,0,"<None>",true) == 1;
	BOOL bWarnCredentialingTOS2 = GetRemotePropertyInt("CredentialWarningOnlyTOS2",0,0,"<None>",true) == 1;

	long nProviderID = -1;
	if (m_dlProviders->GetCurSel() != -1) {
		nProviderID = m_dlProviders->GetValue(m_dlProviders->GetCurSel(), chdpccID);
	}
	if(nProviderID == -1 && m_dlProviders->IsComboBoxTextInUse && m_nCaseHistoryID != -1) {
		// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID FROM CaseHistoryT WHERE ID = {INT}",m_nCaseHistoryID);
		if(!rs->eof) {
			nProviderID = AdoFldLong(rs, "ProviderID",nProviderID);
		}
		rs->Close();
	}

	//add the contents of the surgery
	// (j.jones 2008-09-15 11:27) - PLID 31375 - changed to include the ProductLocationInfoT.Billable status, and
	// the IsSerializedProduct calculation
	// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
	// (j.jones 2009-08-19 16:41) - PLID 35124 - converted to use PreferenceCardsT
	_RecordsetPtr rs = CreateParamRecordset("SELECT PreferenceCardsT.Name AS CardName, ServiceT.Name AS ServiceName, "
		"(CASE WHEN PreferenceCardDetailsT.PersonID Is Null THEN PreferenceCardDetailsT.ServiceID ELSE PreferenceCardDetailsT.PersonID END) AS ItemID, "
		"(CASE WHEN PreferenceCardDetailsT.PersonID Is Not Null THEN -3 WHEN ProductT.ID Is Null THEN -2 ELSE -1 END) AS ItemType, "					
		"PreferenceCardDetailsT.Amount, PreferenceCardDetailsT.Cost, PreferenceCardDetailsT.Quantity, "
		"PreferenceCardDetailsT.Billable, ProductLocationInfoQ.Billable AS BillableForLocation, "
		"Convert(bit, CASE WHEN ProductT.ID Is Not NULL AND (HasSerialNum = 1 OR HasExpDate = 1) THEN 1 ELSE 0 END) AS IsSerializedProduct, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
		"FROM PreferenceCardDetailsT "
		"INNER JOIN PreferenceCardsT ON PreferenceCardDetailsT.PreferenceCardID = PreferenceCardsT.ID "
		"LEFT JOIN ServiceT ON PreferenceCardDetailsT.ServiceID = ServiceT.ID "
		"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN (SELECT Billable, ProductID FROM ProductLocationInfoT WHERE LocationID = {INT}) "
		"	AS ProductLocationInfoQ ON ProductT.ID = ProductLocationInfoQ.ProductID "
		"LEFT JOIN PersonT ON PreferenceCardDetailsT.PersonID = PersonT.ID "
		"WHERE PreferenceCardsT.ID = {INT}", m_nCurLocationID, nPreferenceCardID);
	BOOL bCheckedSurgeryName = FALSE;
	while(!rs->eof) {

		// (j.jones 2007-02-20 12:01) - PLID 24826 - if there is no description, set the surgery name as the description
		// (j.jones 2008-09-15 11:47) - PLID 31375 - moved this check inside the surgery block,
		// for efficiency, and why would we update to the surgery name unless we added items anyways?
		if(!bCheckedSurgeryName) {
			GetDlgItemText(IDC_NAME_EDIT, m_strName);
			m_strName.TrimLeft();
			m_strName.TrimRight();
			if(m_strName.IsEmpty()) {
				CString str = AdoFldString(rs, "CardName","");
				str.TrimLeft();
				str.TrimRight();
				SetDlgItemText(IDC_NAME_EDIT, str);
				m_strName = str;
			}
			bCheckedSurgeryName = TRUE;
		}

		long nItemID = AdoFldLong(rs, "ItemID",-1);
		long nItemType = AdoFldLong(rs, "ItemType",-1);

		BOOL bIsSerializedProduct = FALSE;
		BOOL bIsLinkedToProducts = FALSE;

		CString strName;
		_RecordsetPtr rs2;
		if(nItemType == -3) {
			
			strName = AdoFldString(rs, "PersonName","");
			strName.TrimRight();

			if(bCheckAndWarnLicensing) {

				ECredentialWarning eCredWarning = CheckPersonCertifications(nItemID);

				if(eCredWarning != ePassedAll) {

					CString str;

					if(eCredWarning == eFailedLicenseExpired) {

						CString strLicenses;
						// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nItemID);
						while(!rs2->eof) {
							strLicenses += AdoFldString(rs2, "ExpiredLicense","");
							strLicenses += "\n";
							rs2->MoveNext();
						}
						rs2->Close();

						str.Format("The selected personnel (%s) has the following expired licenses:\n\n%s\n"
							"Do you still wish to add this person to this case history?",strName,strLicenses);
					}
					else if(eCredWarning == eFailedLicenseExpiringSoon) {

						//check if a license will expire within the given day range
						long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

						CString strLicenses;
						// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nItemID,nLicenseWarnDayRange);
						while(!rs2->eof) {
							strLicenses += AdoFldString(rs2, "ExpiredLicense","");
							strLicenses += "\n";
							rs2->MoveNext();
						}
						rs2->Close();

						str.Format("The following licenses are about to expire for the selected personnel (%s):\n\n%s\n"
							"Do you still wish to add this person to this case history?",strName,strLicenses);
					}

					if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
						rs->MoveNext();
						continue;
					}
				}
			}
		}
		else if(nItemType == -2) {

			//TES 7/16/2008 - PLID 27983 - Need to track whether this service code is linked to products.
			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			rs2 = CreateParamRecordset("SELECT Code + ' - ' + Name AS Name, ServiceToProductLinkT.ProductID FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ServiceToProductLinkT ON CptCodeT.ID = ServiceToProductLinkT.CptID WHERE ServiceT.ID = {INT}",nItemID);
			if(!rs2->eof) {
				strName = AdoFldString(rs2, "Name","");
				bIsLinkedToProducts = (rs2->Fields->Item["ProductID"]->Value.vt == VT_I4);
			}
			rs2->Close();

			if(bCheckAndWarnLicensing && !bWarnedLicenses) {

				ECredentialWarning eCredWarning = CheckPersonCertifications(nProviderID);

				if(eCredWarning != ePassedAll) {

					CString str;

					if(eCredWarning == eFailedLicenseExpired) {

						CString strLicenses;
						// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nProviderID);
						while(!rs2->eof) {
							strLicenses += AdoFldString(rs2, "ExpiredLicense","");
							strLicenses += "\n";
							rs2->MoveNext();
						}
						rs2->Close();

						bWarnedLicenses = TRUE;

						str.Format("The selected provider has the following expired licenses:\n\n%s",strLicenses);
					}
					else if(eCredWarning == eFailedLicenseExpiringSoon) {

						//check if a license will expire within the given day range
						long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

						CString strLicenses;
						// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nProviderID,nLicenseWarnDayRange);
						while(!rs2->eof) {
							strLicenses += AdoFldString(rs2, "ExpiredLicense","");
							strLicenses += "\n";
							rs2->MoveNext();
						}
						rs2->Close();

						//set so we do not warn about the licenses multiple times in a row
						bWarnedLicenses = TRUE;

						str.Format("The following licenses are about to expire for the selected provider:\n\n%s",strLicenses);
					}

					MessageBox(str);
				}
			}

			if(bCheckAndWarnCredentials) {

				ECredentialWarning eCredWarning = CheckServiceCodeCredential(nProviderID, nItemID);

				if(eCredWarning != ePassedAll) {

					//it's not credentialed. Check the TOS status before we warn though.
					BOOL bWarn = TRUE;
					if(bWarnCredentialingTOS2 && IsRecordsetEmpty("SELECT ID FROM CPTCodeT WHERE TypeOfService = '2' AND ID = %li",nItemID))
						bWarn = FALSE;

					if(bWarn) {

						CString str;

						if(eCredWarning == eFailedCredential) {

							CString strCodeName;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT Code + ' - ' + Name AS Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = {INT}",nItemID);
							if(!rs2->eof) {
								strCodeName = AdoFldString(rs2, "Name","");
							}
							rs2->Close();

							str.Format("The selected provider is not credentialed for the Service code '%s'\n"
								"Do you still wish to add it to this case history?",strCodeName);
						}

						if(bWarn && IDNO == MessageBox(str,NULL,MB_YESNO)) {
							rs->MoveNext();
							continue;
						}
					}
				}
			}
		}
		else if(nItemType == -1) {
			
			// (j.jones 2008-05-20 17:46) - PLID 29249 - added IsSerializedProduct
			// (j.jones 2008-09-16 09:01) - PLID 31375 - put this in the main recordset			
			strName = AdoFldString(rs, "ServiceName", "");
			bIsSerializedProduct = AdoFldBool(rs, "IsSerializedProduct", FALSE);
		}

		double dblQuantity = AdoFldDouble(rs, "Quantity",1.0);
		COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
		COleCurrency cyCost = AdoFldCurrency(rs, "Cost",COleCurrency(0,0));
		bool bBillable = AdoFldBool(rs, "Billable",TRUE) ? true : false;
		// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
		//bool bPayToPractice = AdoFldBool(rs, "PayToPractice",FALSE) ? true : false;

		// (j.jones 2008-09-15 11:32) - PLID 31375 - passed BillableForLocation to AddDetail
		AddDetail(nItemID, (ECaseHistoryDetailItemType)nItemType, strName, dblQuantity, cyAmount, cyCost,
			bBillable, GetRemotePropertyInt("ChargeAllowQtyIncrementCaseHistory",0,0,"<None>",TRUE)?true:false,
			bIsSerializedProduct, bIsLinkedToProducts, rs->Fields->Item["BillableForLocation"]->Value);

		rs->MoveNext();
	}
	rs->Close();

	//TES 7/16/2008 - PLID 27983 - If this case history is completed, then we may have just added some "placeholder" service
	// codes, which need to be replaced with their linked products.
	if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {
		ReplaceLinkedServices();
	}
}

void CCaseHistoryDlg::OnCasehistorydetailsAdd() 
{
	try {

		BOOL bCheckAndWarnCredentials = GetRemotePropertyInt("CredentialWarnCaseHistories",0,0,"<None>",true) == 1;
		BOOL bCheckAndWarnLicensing = GetRemotePropertyInt("ExpLicenseWarnCaseHistories",bCheckAndWarnCredentials ? 1 : 0,0,"<None>",true) == 1;
		BOOL bWarnCredentialingTOS2 = GetRemotePropertyInt("CredentialWarningOnlyTOS2",0,0,"<None>",true) == 1;

		CCaseHistorySelectDetailDlg dlg(this);
		//TES 7/16/2008 - PLID 27983 - This dialog now needs to know whether it's being called from a completed case history
		dlg.m_bCaseHistoryIsCompleted = IsDlgButtonChecked(IDC_COMPLETED_CHECK) ? true : false;
		if (dlg.DoModal() == IDOK) {

			long nProviderID = -1;
			if (m_dlProviders->GetCurSel() != -1) {
				nProviderID = m_dlProviders->GetValue(m_dlProviders->GetCurSel(), chdpccID);
			}
			if(nProviderID == -1 && m_dlProviders->IsComboBoxTextInUse) {
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID FROM CaseHistoryT WHERE ID = {INT}", m_nCaseHistoryID);
				if(!rs->eof) {
					nProviderID = AdoFldLong(rs, "ProviderID",nProviderID);
				}
				rs->Close();
			}

			// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
			if(dlg.m_nPreferenceCardID != -1) {

				// (j.jones 2006-12-13 16:12) - PLID 23578 - migrated this code to AddSurgery()
				// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
				AddPreferenceCard(dlg.m_nPreferenceCardID);

				// (j.jones 2009-09-01 10:11) - PLID 35380 - add the procedures from this preference card
				// outside of AddPreferenceCard(), only if there is no linked appointment, and
				// the procedures don't already exist on the case history
				if(m_nAppointmentID == -1) {

					//get the existing procedure list
					CString strProcedureIDs = "-1";
					for(int i=0; i<m_arProcedureIDs.GetSize(); i++) {
						long nProcedureID = (long)m_arProcedureIDs.GetAt(i);
						strProcedureIDs += ",";
						strProcedureIDs += AsString(nProcedureID);
					}

					//cannot be parameterized
					_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID "
						"FROM PreferenceCardProceduresT "
						"INNER JOIN ProcedureT ON PreferenceCardProceduresT.ProcedureID = ProcedureT.ID "
						"WHERE PreferenceCardID = %li "
						"AND ProcedureID NOT IN (%s) "
						"AND ProcedureT.Inactive = 0 "
						"GROUP BY ProcedureID", dlg.m_nPreferenceCardID, strProcedureIDs);
					if(!rs->eof) {
						if(rs->GetRecordCount() == 1) {
							//just use the one procedure we found
							m_arProcedureIDs.Add(AdoFldLong(rs, "ProcedureID"));
						}
						else {
							// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
							CMultiSelectDlg dlgMulti(this, "ProcedureT");
							CString strWhere;
							strWhere.Format("PreferenceCardID = %li AND ProcedureID NOT IN (%s) AND ProcedureT.Inactive = 0", dlg.m_nPreferenceCardID, strProcedureIDs);
							//they are allowed to select nothing, if they wish
							if(dlgMulti.Open("PreferenceCardProceduresT INNER JOIN ProcedureT ON PreferenceCardProceduresT.ProcedureID = ProcedureT.ID",
								strWhere, "ProcedureT.ID", "ProcedureT.Name", "Select the procedures to be added to this case history:") == IDOK) {
								
								//we want to append to our procedure list, not replace it
								CArray<long, long> aryNewProcIDs;
								dlgMulti.FillArrayWithIDs(aryNewProcIDs);
								m_arProcedureIDs.Append(aryNewProcIDs);
							}
						}
					}
					rs->Close();

					DisplayProcedureInfo();
				}
			}
			else {

				if((bCheckAndWarnCredentials || bCheckAndWarnLicensing) && dlg.m_nType == chditCptCode) {				
					//make sure the provider is credentialed for this CPT code (if one is selected)
					if(nProviderID != -1) {

						if(bCheckAndWarnLicensing) {

							ECredentialWarning eCredWarning = CheckPersonCertifications(nProviderID);

							if(eCredWarning != ePassedAll) {

								CString str;

								if(eCredWarning == eFailedLicenseExpired) {

									CString strLicenses;
									// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
									_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
										"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nProviderID);
									while(!rs2->eof) {
										strLicenses += AdoFldString(rs2, "ExpiredLicense","");
										strLicenses += "\n";
										rs2->MoveNext();
									}
									rs2->Close();

									str.Format("The selected provider has the following expired licenses:\n\n%s\n"
										"Do you still wish to add this service to this case history?",strLicenses);
								}
								else if(eCredWarning == eFailedLicenseExpiringSoon) {

									//check if a license will expire within the given day range
									long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

									CString strLicenses;
									// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
									_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
										"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nProviderID,nLicenseWarnDayRange);
									while(!rs2->eof) {
										strLicenses += AdoFldString(rs2, "ExpiredLicense","");
										strLicenses += "\n";
										rs2->MoveNext();
									}
									rs2->Close();

									str.Format("The following licenses are about to expire for the selected provider:\n\n%s\n"
										"Do you still wish to add this service to this case history?",strLicenses);
								}

								if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
									return;
								}
							}
						}
						
						if(bCheckAndWarnCredentials) {

							ECredentialWarning eCredWarning = CheckServiceCodeCredential(nProviderID, dlg.m_nID);

							if(eCredWarning != ePassedAll) {

								//it's not credentialed. Check the TOS status before we warn though.
								BOOL bWarn = TRUE;
								if(bWarnCredentialingTOS2 && IsRecordsetEmpty("SELECT ID FROM CPTCodeT WHERE TypeOfService = '2' AND ID = %li",dlg.m_nID))
									bWarn = FALSE;

								if(bWarn) {

									CString str;

									if(eCredWarning == eFailedCredential) {

										CString strCodeName;
										// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
										_RecordsetPtr rs2 = CreateParamRecordset("SELECT Code + ' - ' + Name AS Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = {INT}",dlg.m_nID);
										if(!rs2->eof) {
											strCodeName = AdoFldString(rs2, "Name","");
										}
										rs2->Close();

										str.Format("The selected provider is not credentialed for the Service code '%s'\n"
											"Do you still wish to add it to this case history?",strCodeName);
									}

									if(bWarn && IDNO == MessageBox(str,NULL,MB_YESNO)) {
										return;
									}
								}
							}
						}
					}
				}

				if(bCheckAndWarnLicensing && dlg.m_nType == chditPerson) {

					ECredentialWarning eCredWarning = CheckPersonCertifications(dlg.m_nID);

					if(eCredWarning != ePassedAll) {

						CString str;

						if(eCredWarning == eFailedLicenseExpired) {

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",dlg.m_nID);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The selected personnel (%s) has the following expired licenses:\n\n%s\n"
								"Do you still wish to add this person to this case history?",dlg.m_strName,strLicenses);
						}
						else if(eCredWarning == eFailedLicenseExpiringSoon) {

							//check if a license will expire within the given day range
							long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",dlg.m_nID,nLicenseWarnDayRange);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The following licenses are about to expire for the selected personnel (%s):\n\n%s\n"
								"Do you still wish to add this person to this case history?",dlg.m_strName,strLicenses);
						}

						if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
							return;
						}
					}
				}

				//add the detail that they have selected
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				AddDetail(dlg.m_nID, dlg.m_nType, dlg.m_strName, dlg.m_dblQuantity, dlg.m_cyAmount, dlg.m_cyCost, dlg.m_bBillable, GetRemotePropertyInt("ChargeAllowQtyIncrementCaseHistory",0,0,"<None>",TRUE)?true:false, dlg.m_bIsSerializedProduct, dlg.m_bIsLinkedToProducts, g_cvarNull);
			}
		}
	} NxCatchAll("CCaseHistoryDlg::OnCasehistorydetailsAdd");
}

void CCaseHistoryDlg::OnCasehistorydetailsRemove() 
{
	try {
		if (MessageBox("Are you sure you want to remove the selected entry from this case history?","Practice",MB_ICONQUESTION|MB_YESNO) == IDYES) {
			RemoveDetail(m_dlCaseHistoryDetails->GetCurSel());
		}
	} NxCatchAll("CCaseHistoryDlg::OnCasehistorydetailsRemove");
}

void CCaseHistoryDlg::OnLButtonDownCasehistoryDetailsList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow != -1 && nCol == chdlcAction) {
		CString strAction = VarString(m_dlCaseHistoryDetails->GetValue(nRow, chdlcAction));
		if (strAction == "Add") {
			// The user clicked the "Add" link
			OnCasehistorydetailsAdd();
		} else if (strAction == "Remove") {
			// The user clicked a "Remove" link
			try {
				m_dlCaseHistoryDetails->PutCurSel(nRow);
				OnCasehistorydetailsRemove();
			} NxCatchAll("CCaseHistoryDlg::OnLButtonDownCasehistoryDetailsList");
		} else {
			// This shouldn't be possible
			ASSERT(FALSE);
		}
	}
}

void CCaseHistoryDlg::RemoveDetail(const long nIndex)
{
	try {
		long nDetailID = VarLong(m_dlCaseHistoryDetails->GetValue(nIndex, chdlcID));
		long CaseHistoryDetailProductItemListID = VarLong(m_dlCaseHistoryDetails->GetValue(nIndex, chdlcCHDProductItemListID),-1);
		m_dlCaseHistoryDetails->RemoveRow(nIndex);
		m_aryDeleteDetailIDs.Add(nDetailID);

		//go through the product item array and mark deleted
		MarkCaseHistoryDetailProductItemListDeleted(CaseHistoryDetailProductItemListID);

		m_bIsModified = TRUE;
	} NxCatchAll("CCaseHistoryDlg::RemoveDetail");
}

// (j.jones 2008-05-20 17:44) - PLID 29249 - added bIsSerializedProduct parameter
//TES 7/16/2008 - PLID 27983 - Added bIsLinkedToProducts parameter
// (j.jones 2008-09-15 11:15) - PLID 31375 - added varIsBillableProductForLoc which will be
// TRUE if it is a product that is billable for the current location,
// FALSE if not billable for the location (or not a product)
// and NULL if we didn't check the setting
// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
void CCaseHistoryDlg::AddDetail(const long nItemID, const ECaseHistoryDetailItemType nItemType, const CString &strItemName, const double &dblQuantity, const COleCurrency &cyAmount, const COleCurrency &cyCost, const bool &bBillable, bool bTryToIncrement, BOOL bIsSerializedProduct, BOOL bIsLinkedToProducts, _variant_t varIsBillableProductForLoc)
{
	try {

		double dblNewQuantity = dblQuantity;

		BOOL bAnesthesia = FALSE;
		COleCurrency cyAnesthUnitCost = COleCurrency(0,0);
		double dblAnesthUnits = 0.0;
		long nAnesthMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES);

		BOOL bFacilityFee = FALSE;
		COleCurrency cyFacilityUnitCost = COleCurrency(0,0);

		BOOL bAssistingCode = FALSE;
		COleCurrency cyAssistingCodeUnitCost = COleCurrency(0,0);

		long nFacilityMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES);
		if(m_dlPOS->CurSel == -1)
			m_dlPOS->CurSel = 0;
		long nPlaceOfServiceID = m_dlPOS->GetValue(m_dlPOS->CurSel, 0).lVal;

		if(nItemType == -2) { //CPT Code

			//check and see if this is an anesthesia code or facility code, and already exists in the charge list
			if(!CheckAllowAddAnesthesiaFacilityCharge(nItemID))
				return;

			CString strStartTime = "", strEndTime = "";

			COleDateTime dtAnesthStart;
			if(m_nxtAnesthStart->GetStatus() == 1) {
				dtAnesthStart = StripSeconds(m_nxtAnesthStart->GetDateTime());
			}
			else {
				dtAnesthStart.SetStatus(COleDateTime::invalid);
			}
			if(dtAnesthStart.GetStatus() != COleDateTime::invalid) {
				strStartTime = dtAnesthStart.Format("%H:%M:%S");
			}

			COleDateTime dtAnesthEnd;
			if(m_nxtAnesthEnd->GetStatus() == 1) {
				dtAnesthEnd = StripSeconds(m_nxtAnesthEnd->GetDateTime());
			}
			else {
				dtAnesthEnd.SetStatus(COleDateTime::invalid);
			}
			if(dtAnesthEnd.GetStatus() != COleDateTime::invalid) {
				strEndTime = dtAnesthEnd.Format("%H:%M:%S");
			}

			long nCurAnesthMinutes = nAnesthMinutes;

			// (j.jones 2004-07-07 12:09) - checks to see if this is an anesthesia code and loads the associated info.
			if(!CheckAnesthesia(nItemID,bAnesthesia,cyAnesthUnitCost,dblAnesthUnits,nCurAnesthMinutes,strStartTime,strEndTime,nPlaceOfServiceID))
				return;

			if(bAnesthesia) {

				if(nCurAnesthMinutes > 0)
					nAnesthMinutes = nCurAnesthMinutes;

				dblNewQuantity = dblAnesthUnits;
				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, nAnesthMinutes);
				OnKillfocusEditTotalCaseAnesthMinutes();
				
				COleDateTime dt;
				if(strStartTime != "" && dt.ParseDateTime("1/1/1900 " + strStartTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtAnesthStart->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtAnesthStart->Clear();
				}

				if(strEndTime != "" && dt.ParseDateTime("1/1/1900 " + strEndTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtAnesthEnd->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtAnesthEnd->Clear();
				}
			}

			strStartTime = "", strEndTime = "";

			COleDateTime dtFacilityStart;
			if(m_nxtFacilityStart->GetStatus() == 1) {
				dtFacilityStart = StripSeconds(m_nxtFacilityStart->GetDateTime());
			}
			else {
				dtFacilityStart.SetStatus(COleDateTime::invalid);
			}
			if(dtFacilityStart.GetStatus() != COleDateTime::invalid) {
				strStartTime = dtFacilityStart.Format("%H:%M:%S");
			}

			COleDateTime dtFacilityEnd;
			if(m_nxtFacilityEnd->GetStatus() == 1) {
				dtFacilityEnd = StripSeconds(m_nxtFacilityEnd->GetDateTime());
			}
			else {
				dtFacilityEnd.SetStatus(COleDateTime::invalid);
			}
			if(dtFacilityEnd.GetStatus() != COleDateTime::invalid) {
				strEndTime = dtFacilityEnd.Format("%H:%M:%S");
			}

			long nCurFacilityMinutes = nFacilityMinutes;
			
			// (j.jones 2005-07-01 11:31) - checks to see if this is a facility fee code and calculates the right fee
			if(!CheckFacilityFee(nItemID,bFacilityFee,cyFacilityUnitCost,nCurFacilityMinutes,strStartTime,strEndTime,nPlaceOfServiceID)) {
				return;
			}

			if(bFacilityFee) {

				if(nCurFacilityMinutes > 0)
					nFacilityMinutes = nCurFacilityMinutes;

				COleDateTime dt;
				if(strStartTime != "" && dt.ParseDateTime("1/1/1900 " + strStartTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtFacilityStart->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtFacilityStart->Clear();
				}

				if(strEndTime != "" && dt.ParseDateTime("1/1/1900 " + strEndTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtFacilityEnd->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtFacilityEnd->Clear();
				}

				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, nFacilityMinutes);
				OnKillfocusEditTotalCaseFacilityMinutes();
			}

			// (j.jones 2010-11-23 09:19) - PLID 39602 - if OHIP, and not anesthesia, and not a facility fee,
			// check to see if it is an assisting code
			if(!bAnesthesia && !bFacilityFee && UseOHIP()) {

				// (j.jones 2011-10-31 17:02) - PLID 41558 - we do not store this data on the case history,
				// so pass in dummy data to force a prompt
				long nAssistingMinutes = 0;
				CString strAssistingStartTime = "";
				CString strAssistingEndTime = "";
				if(!CheckAssistingCode(nItemID, bAssistingCode, cyAssistingCodeUnitCost, nAssistingMinutes, strAssistingStartTime, strAssistingEndTime)) {
					return;
				}
			}
		}

		// (j.jones 2008-05-21 08:52) - PLID 29249 - warn if the quantity is a decimal,
		// but it is a serialized product
		if(bIsSerializedProduct && (long)dblNewQuantity != dblNewQuantity) {

			CString strWarn;
			strWarn.Format("The product '%s' is being added with a quantity of %g, but it must be a whole number as it requires a serial number / expiration date.\n\n"
				"The quantity has been reduced to %li", strItemName, dblNewQuantity, (long)dblNewQuantity);
			AfxMessageBox(strWarn);

			//reduce the quantity
			dblNewQuantity = (long)dblNewQuantity;
		}

		if(bTryToIncrement) {
			bool bMatched = false;
			for(int i = 0; i < m_dlCaseHistoryDetails->GetRowCount() && !bMatched; i++) {
				NXDATALISTLib::IRowSettingsPtr pRow = m_dlCaseHistoryDetails->GetRow(i);
				if(VarLong(pRow->GetValue(chdlcItemID),-1) == nItemID && VarLong(pRow->GetValue(chdlcItemType),-1) == nItemType) {
					bMatched = true;
					pRow->PutValue(chdlcQuantity, VarDouble(pRow->GetValue(chdlcQuantity)) + dblNewQuantity);
					m_dlCaseHistoryDetails->EnsureRowVisible(i);
				}
			}
			if(bMatched) {
				return;
			}
		}

		// (j.jones 2007-02-20 12:01) - PLID 24826 - if there is no description, set the item name as the description
		// (except if the item is a person, which of course makes no sense)
		GetDlgItemText(IDC_NAME_EDIT, m_strName);
		m_strName.TrimLeft();
		m_strName.TrimRight();
		if(m_strName.IsEmpty() && nItemType != -3) {
			SetDlgItemText(IDC_NAME_EDIT, strItemName);
		}

		NXDATALISTLib::IRowSettingsPtr pRow = m_dlCaseHistoryDetails->GetRow(-1);
		pRow->PutValue(chdlcID, (long)-1);
		pRow->PutValue(chdlcAction, _bstr_t("Remove"));		
		pRow->PutValue(chdlcItemID, (long)nItemID);
		pRow->PutValue(chdlcItemType, (long)nItemType);
		pRow->PutValue(chdlcName, _bstr_t((LPCTSTR)strItemName));
		pRow->PutValue(chdlcQuantity, (double)dblNewQuantity);
		
		_variant_t varAmount;
		//this will always be null for Personnel
		if(nItemType == -3) {
			varAmount.vt = VT_NULL;
		}
		else {
			varAmount = _variant_t(cyAmount);
		}

		if(bAnesthesia) {
			varAmount = cyAnesthUnitCost;
		}
		else if(bFacilityFee) {
			varAmount = cyFacilityUnitCost;
		}
		// (j.jones 2010-11-23 09:20) - PLID 39602 - use the assisting code cost
		else if(bAssistingCode) {
			varAmount = cyAssistingCodeUnitCost;
		}

		pRow->PutValue(chdlcAmount, varAmount);

		_variant_t varCost;
		//this will always be null for CPT Codes
		if(nItemType == -2) {
			varCost.vt = VT_NULL;
		}
		else {
			varCost = _variant_t(cyCost);
		}

		// (j.jones 2008-07-01 09:51) - PLID 18744 - do not fill the visible cost column for
		// personnel if there are no read permissions
		if(nItemType == -3 && !(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
			pRow->PutValue(chdlcVisibleCost, g_cvarNull);
			//color the background gray
			pRow->PutCellBackColor(chdlcVisibleCost,RGB(230,230,230));
		}
		else {
			pRow->PutValue(chdlcVisibleCost, varCost);
		}

		// (j.jones 2008-07-01 09:49) - PLID 18744 - fill the TrueCost column, which will track costs
		// even if they are not displayed in the visible Cost column, such as on personnel
		pRow->PutValue(chdlcTrueCost, varCost);

		_variant_t varBillable;
		//this will always be null for Personnel
		if(nItemType == -3) {
			varBillable.vt = VT_NULL;
		}
		else {

			// (j.jones 2007-02-21 10:47) - PLID 24846 - set the billable status
			// based on the product's billable status for the location
			// (provided we have been told it is a billable product)
			if(nItemType == -1 && bBillable) {
				// (j.jones 2008-09-15 11:08) - PLID 31375 - converted into a parameterized query,
				// but also used the passed in varIsBillableProductForLoc, so we don't need to run this
				// query unless varIsBillableProductForLoc is NULL				
				if(varIsBillableProductForLoc.vt == VT_BOOL) {
					varBillable = varIsBillableProductForLoc;
				}
				else {
					_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 Billable "
						"FROM ProductLocationInfoT "
						"WHERE Billable = 1 AND ProductID = {INT} AND LocationID = {INT}", nItemID, m_nCurLocationID);
					BOOL bBillableForLocation = FALSE;
					if(!rs->eof) {
						bBillableForLocation = TRUE;
					}
					rs->Close();
					varBillable = bBillableForLocation ? g_cvarTrue : g_cvarFalse;
				}				
			}
			else {
				//otherwise use what was passed in
				varBillable = bBillable ? g_cvarTrue : g_cvarFalse;
			}
		}

		pRow->PutValue(chdlcBillable, varBillable);

		// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
		/*
		_variant_t varPayToPractice;
		//this will always be null for Personnel
		if(nItemType == -3) {
			varPayToPractice.vt = VT_NULL;
		}
		else {
			varPayToPractice = _variant_t(bPayToPractice);
		}
		pRow->PutValue(chdlcPayToPractice, varPayToPractice);
		*/

		if(nItemType == -3) {
			//color the Person row yellow-orange, like the Contacts module
			pRow->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));

			//color the Amount, Billable, and PayToPractice columns gray
			pRow->PutCellBackColor(chdlcAmount,RGB(230,230,230));
			pRow->PutCellBackColor(chdlcBillable,RGB(230,230,230));
			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			//pRow->PutCellBackColor(chdlcPayToPractice,RGB(230,230,230));
		}
		else if(nItemType == -2) {
			//color the CPT row red, like the Admin. module
			pRow->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));

			//color the Cost column gray
			pRow->PutCellBackColor(chdlcVisibleCost,RGB(230,230,230));
		}
		else if(nItemType == -1) {
			//color the Product row blue, like the Inventory module
			pRow->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		}

		long CaseHistoryDetailProductItemListID = -1;		
		if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {
			//TES 4/19/2013 - PLID 56352 - Don't call AddCaseHistoryDetailProductItem() if this isn't a product!
			if(nItemType == chditProduct) {
				if(!AddCaseHistoryDetailProductItem(nItemID,dblNewQuantity,-1,CaseHistoryDetailProductItemListID)) {
					return;
				}
				else {
					pRow->PutValue(chdlcQuantity, (double)dblNewQuantity);
				}
			}
		}
		pRow->PutValue(chdlcCHDProductItemListID, long(CaseHistoryDetailProductItemListID));

		// (j.jones 2008-05-20 17:26) - PLID 29249 - this column needs to be TRUE if
		// a serialized product, even if we aren't tracking items yet
		_variant_t varIsSerializedProduct = g_cvarFalse;
		if(bIsSerializedProduct) {
			varIsSerializedProduct = g_cvarTrue;
		}
		pRow->PutValue(chdlcIsSerializedProduct, varIsSerializedProduct);

		//TES 7/16/2008 - PLID 27983 - Added the IsLinkedToProducts line
		_variant_t varIsLinkedToProducts = g_cvarFalse;
		if(bIsLinkedToProducts) {
			varIsLinkedToProducts = g_cvarTrue;
		}
		pRow->PutValue(chdlcIsLinkedToProducts, varIsLinkedToProducts);

		m_dlCaseHistoryDetails->InsertRow(pRow, m_dlCaseHistoryDetails->GetRowCount() - 1);
		m_dlCaseHistoryDetails->EnsureRowVisible(m_dlCaseHistoryDetails->GetRowCount() - 1);

		m_bIsModified = TRUE;

	} NxCatchAll("CCaseHistoryDlg::AddDetail");
}


BOOL CCaseHistoryDlg::PrintCaseHistory(IN const BOOL bPreview)
{
	// First ensure the case history is actually saved
	try {
		// Make sure the user knows what's going on
		int nResult = MessageBox(
			"Any changes will be saved automatically.  Would you like to proceed?", 
			NULL, MB_OKCANCEL|MB_ICONINFORMATION);
		if (nResult == IDOK) {
			// Ok, let's save
			if (!Save()) {
				// Save was cancelled or failed and a message has already been given to the user
				return FALSE;
			}
		} else {
			// The user doesn't want to save, and therefore is not allowed to print
			return FALSE;
		}
	} NxCatchAllCall("CCaseHistoryDlg::PrintCaseHistory:EnsureSaved", return FALSE);

	// Now do the print or preview
	try {
		// Show immediate feedback by way of the wait cursor
		CWaitCursor wc;

		// Create a copy of the report object
		CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(387)]);

		// Create and show the centered progress dialog
		CProcessRptDlg dlgProgress(this);
		{
			// Create the window without making it visible yet
			dlgProgress.Create(IDD_PROCESSINGDLG, NULL);
			// Find the center of the parent
			CRect rcNew;
			{
				// Get the current rect
				CRect rcCur;
				dlgProgress.GetWindowRect(&rcCur);
				// Get the parent rect
				CRect rcParent;
				GetActiveWindow()->GetWindowRect(&rcParent);
				// Center the new rect inside the parent rect
				rcNew.left = (rcParent.Width() / 2) - (rcCur.Width() / 2);
				rcNew.right = rcNew.left + rcCur.Width();
				rcNew.top = (rcParent.Height() / 2) - (rcCur.Height() / 2);
				rcNew.bottom = rcNew.top + rcCur.Height();
			}
			// Center it and show it at the same time
			dlgProgress.SetWindowPos(&wndTop, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
		}
		
		// Tell the report that we want to filter on this case history
		rep.nExtraID = m_nCaseHistoryID;

		//check to see if there is a default report
		_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 387");
		CString strFileName;

		if (rsDefault->eof) {

			strFileName = "CaseHistoryIndiv";
		}
		else {
			
			long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

			if (nDefaultCustomReport > 0) {

				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsFileName = CreateParamRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 387 AND Number = {INT}", nDefaultCustomReport);

				if (rsFileName->eof) {

					//this should never happen
					MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
				}
				else {
					
					//set the default
					rep.nDefaultCustomReport = nDefaultCustomReport;
					strFileName =  AdoFldString(rsFileName, "FileName");
				}
			}
			else {
				//if this occurs it just means they want the default, which in this case, there is only one
				strFileName = "CaseHistoryIndiv";
				
			}
		}

		// Disable this window, preview the report, and re-enable this window
		BOOL bSuccess = FALSE;
		{
			EnableWindow(FALSE);
			try {
				//Made new function for running reports - JMM 5-28-04
				// (a.walling 2009-07-02 10:45) - PLID 14181 - Send in a print info structure
				CPrintInfo prInfo;
				if (!bPreview) {
					CPrintDialog* dlg;
					dlg = new CPrintDialog(FALSE);
					prInfo.m_bPreview = false;
					prInfo.m_bDirect = false;
					prInfo.m_bDocObject = false;
					if(prInfo.m_pPD != NULL) {
						delete prInfo.m_pPD;
					}
					prInfo.m_pPD = dlg;
				}
				bSuccess = RunReport(&rep, bPreview, (CWnd *)this, "Case History" + m_strName, &prInfo);
			} catch (...) {
				try { EnableWindow(TRUE); } catch (...) { };
				throw;
			}
			EnableWindow(TRUE);
		}

		// Kill the progress window (I think this happens automatically on destruction, but it can't hurt to call it explicitly)
		dlgProgress.DestroyWindow();

		if (bSuccess) {
			// Success, close the case history dialog and return success
			// (a.walling 2008-02-21 10:58) - PLID 29043 - Unregister for barcode scans
			GetMainFrame()->UnregisterForBarcodeScan(this);
			
			// (j.jones 2009-08-07 08:40) - PLID 7397 - track when we close via print previewing
			EndDialog(RETURN_PREVIEW_CASE_HISTORY);

			//if this is a tracked case history, from the ASC module,
			//update the parent's view
			if(m_pParent) {
				m_pParent->OnClosedCaseHistory(m_nCaseHistoryID);
			}

			return TRUE;
		} else {
			// Failure, we need to bring the dialog back
			return FALSE;
		}
	
	} NxCatchAllCall("CCaseHistoryDlg::PrintCaseHistory:Print", {
		// Failure, we need to bring the dialog back
		return FALSE;
	});
}

void CCaseHistoryDlg::OnPrintBtn() 
{
	PrintCaseHistory(TRUE);
}

void CCaseHistoryDlg::OnSelChosenLocationCombo(long nRow)
{
	try {
	
		if(nRow == -1)
			m_dlLocations->SetSelByColumn(chdlccID, GetCurrentLocationID());

		long nLocationID = m_dlLocations->GetValue(m_dlLocations->GetCurSel(),chdlccID).lVal;

		//if they didn't change the selection, then don't bother doing anything
		if(nLocationID == m_nCurLocationID)
			return;

		// (j.jones 2008-02-29 10:20) - PLID 29102 - see if we have an allocation selected, as it will be losing that selection
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			long nAllocationID = -1;					
			if(m_InvAllocationCombo->GetCurSel() != NULL) {
				nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
			}
			if(nAllocationID != -1) {
				if(IDNO == MessageBox("The currently linked allocation will be unlinked if you switch locations.\n"
					"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					
					//reset the selection
					m_dlLocations->TrySetSelByColumn(chdlccID, m_nCurLocationID);
					return;
				}
			}
		}

		m_bIsModified = TRUE;

		m_nCurLocationID = nLocationID;

		// (j.jones 2008-02-28 17:44) - PLID 29102 - requery the allocation list
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {

			CString strWhere = "";
			if(m_nPersonID != -1) {
				//we'll still run this same filter even if m_nCaseHistoryID is -1
				strWhere.Format("PatientID = %li "
					"AND ID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID <> %li) "
					"AND LocationID = %li", m_nPersonID, m_nCaseHistoryID, nLocationID);
			}
			else if(m_nCaseHistoryID != -1) {
				strWhere.Format("PatientID IN (SELECT PersonID FROM CaseHistoryT WHERE ID = %li) "
					"AND ID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID <> %li) "
					"AND LocationID = %li", m_nCaseHistoryID, m_nCaseHistoryID, nLocationID);
			}

			if(!strWhere.IsEmpty()) {
				m_InvAllocationCombo->PutWhereClause(_bstr_t(strWhere));
				m_InvAllocationCombo->Requery();
			}
			else {
				m_InvAllocationCombo->Clear();
			}

			// (j.jones 2008-03-10 14:40) - PLID 29233 - clear the allocation contents list
			m_InvAllocationContentsList->Clear();
		}

		PromptInventoryLocationChanged();

	}NxCatchAll("Error in CCaseHistoryDlg::OnSelChosenLocationCombo");
}

void CCaseHistoryDlg::OnSelChangedProviderList(long nNewSel) { m_bIsModified = TRUE;}
void CCaseHistoryDlg::OnSelChangedPOSCombo(long nNewSel) { m_bIsModified = TRUE;}
void CCaseHistoryDlg::OnChangeSurgeryDate(NMHDR* pNMHDR, LRESULT* pResult) { 	m_bIsModified = TRUE; *pResult = 0; }
void CCaseHistoryDlg::OnKillFocusPreopInEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusPreopOutEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusSurgeonInEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusSurgeonOutEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusOproomInEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusOproomOutEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusRecoveryInEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocusRecoveryOutEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocus23hourroomInEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnKillFocus23hourroomOutEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnChangeNotesEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnChangeNameEdit() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnChangeEditTotalCaseAnesthMinutes() { m_bIsModified = TRUE; }
void CCaseHistoryDlg::OnChangeEditTotalCaseFacilityMinutes() { m_bIsModified = TRUE; }

void CCaseHistoryDlg::OnCompletedCheck() 
{
	CWaitCursor pWait;

	if (IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {
		m_varCompletedDate = _variant_t(COleDateTime::GetCurrentTime(), VT_DATE);

		//TES 7/16/2008 - PLID 27983 - We need to replace any service codes that are linked to products with their associated
		// products.
		ReplaceLinkedServices();

		// (j.jones 2008-02-27 16:34) - PLID 29126 - if we have a linked allocation,
		// and it is uncompleted, try to complete it, and if we don't have one
		// then search for one, try to link one, and complete that
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			if(!TryCompleteInvAllocation()) {
				//if they aborted this, don't complete the case history
				m_varCompletedDate.Clear();
				m_varCompletedDate.vt = VT_NULL;

				CheckDlgButton(IDC_COMPLETED_CHECK, FALSE);

				// (j.jones 2008-02-28 16:12) - PLID 29108 - enable the allocation creation button if uncompleted,
				//but only if they have permissions
				if(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS) {
					GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->EnableWindow(TRUE);
				}
				return;
			}
		}

		PromptForCaseHistoryDetailProductItems();

		// (j.jones 2008-02-27 16:41) - PLID 29108 - disable the allocation creation button if completed
		GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->EnableWindow(FALSE);

	} else {
		//prompt that they are removing serialized products from the detail
		if(!MarkAllDeletedFromCaseHistoryDetailProductItemsArray()) {
			CheckDlgButton(IDC_COMPLETED_CHECK,TRUE);
			return;
		}
		m_varCompletedDate.Clear();
		m_varCompletedDate.vt = VT_NULL;

		// (j.jones 2008-02-27 16:41) - PLID 29108 - enable the allocation creation button if uncompleted,
		//but only if they have permissions
		if(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS) {
			GetDlgItem(IDC_BTN_CREATE_ALLOCATION)->EnableWindow(TRUE);
		}
	}
}

void CCaseHistoryDlg::PromptForCaseHistoryDetailProductItems(BOOL bAllowQuantityChange /*= TRUE*/) {
	
	try {

		//loop through all items and if there needs to be a product item selected, prompt for it
		for(int i=0;i<m_dlCaseHistoryDetails->GetRowCount()-1;i++) {
			//only check products
			if((ECaseHistoryDetailItemType)VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcItemType),0) == chditProduct) {
				if(VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcCHDProductItemListID),-1) == -1) {
					//no product items selected
					long CaseHistoryDetailProductItemListID = -1;
					double dblQuantity = VarDouble(m_dlCaseHistoryDetails->GetValue(i,chdlcQuantity),0.0);
					if(!AddCaseHistoryDetailProductItem(
						VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcItemID),-1),
						dblQuantity,
						VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcID),-1),CaseHistoryDetailProductItemListID,
						bAllowQuantityChange)) {
						RemoveDetail(i);
					}
					else {
						m_dlCaseHistoryDetails->PutValue(i,chdlcCHDProductItemListID,CaseHistoryDetailProductItemListID);
						m_dlCaseHistoryDetails->PutValue(i,chdlcQuantity,dblQuantity);
					}
				}
			}
		}

	}NxCatchAll("Error adding products to the case history item.");
}

BOOL CCaseHistoryDlg::MarkAllDeletedFromCaseHistoryDetailProductItemsArray() {

	try {

		if(m_aryCaseHistoryDetailProductItems.GetSize() > 0) {
			if(IDNO == MessageBox("There are serial numbered / expirable products attached to this case history.\n"
				"If you mark it incompleted, you release these products for use again.\n"
				"Are you sure you wish to do this?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
		}

		for(int j=0;j<m_dlCaseHistoryDetails->GetRowCount()-1;j++)
			m_dlCaseHistoryDetails->PutValue(j,chdlcCHDProductItemListID,(long)-1);

		for(int i=0;i<m_aryCaseHistoryDetailProductItems.GetSize();i++) {
			CaseHistoryDetailProductItemList *delCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
			
			for(int q=0;q<delCHDPI->ProductItemAry.GetSize();q++) {
				delCHDPI->ProductItemAry.GetAt(q)->SaveStatus = CHDPI_DELETE;
			}
		}

	}NxCatchAll("Error removing linked products from the case history item.");

	return TRUE;
}

void CCaseHistoryDlg::DeleteAllFromCaseHistoryDetailProductItemsArray() {

	try {

		for(int i=m_aryCaseHistoryDetailProductItems.GetSize()-1;i>=0;i--) {
			CaseHistoryDetailProductItemList *delCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
			
			for(int q=delCHDPI->ProductItemAry.GetSize()-1;q>=0;q--) {
				delete delCHDPI->ProductItemAry.GetAt(q);
			}
			delCHDPI->ProductItemAry.RemoveAll();

			delete delCHDPI;
			m_aryCaseHistoryDetailProductItems.RemoveAt(i);
		}

	} NxCatchAll("Error clearing linked products from the case history.");
}

CString CCaseHistoryDlg::GetProductItemWhereClause() {

	//returns a query of AND statements so unsaved assigned ProductItems aren't in the list

	try {

		CString strWhere = "";

		for(int i=0;i<m_aryCaseHistoryDetailProductItems.GetSize();i++) {
			CaseHistoryDetailProductItemList *CHDPIlist = m_aryCaseHistoryDetailProductItems.GetAt(i);
			
			for(int j=0;j<CHDPIlist->ProductItemAry.GetSize();j++) {
				CString str;
				if(CHDPIlist->ProductItemAry.GetAt(j)->SaveStatus != CHDPI_DELETE) {
					str.Format("ProductItemsT.ID <> %li",CHDPIlist->ProductItemAry.GetAt(j)->ProductItemID);
					strWhere += str + " AND ";
				}
			}
		}

		strWhere.TrimRight("AND ");

		return strWhere;

	}NxCatchAll("Error determining list of unsaved products.");

	return "";
}

long CCaseHistoryDlg::NewCaseHistoryDetailProductItemListID() {

	long Max = -1;
	if(m_aryCaseHistoryDetailProductItems.GetSize()==0)
		return 1;
	else {
		for(int i=0; i<m_aryCaseHistoryDetailProductItems.GetSize();i++) {
			long ID = m_aryCaseHistoryDetailProductItems.GetAt(i)->ID;
			if(ID > Max)
				Max = ID;
		}
		Max++;
		return Max;
	}
}

void CCaseHistoryDlg::AddToCaseHistoryDetailProductItemsArray(long CaseHistoryDetailID, long CaseHistoryDetailProductItemListID, CDWordArray &adwProductItemIDs) {

	try {

		//initialize the CPI list
		CaseHistoryDetailProductItemList *newCHDPI = new (CaseHistoryDetailProductItemList);
		newCHDPI->CaseHistoryDetailID = CaseHistoryDetailID;
		newCHDPI->ID = CaseHistoryDetailProductItemListID;

		//now add the product items
		for(int i=0;i<adwProductItemIDs.GetSize();i++) {
			CHProductItems *newProdItem = new (CHProductItems);
			newProdItem->ProductItemID = (long)adwProductItemIDs.GetAt(i);
			newProdItem->SaveStatus = CHDPI_SAVENEW;
			newCHDPI->ProductItemAry.Add(newProdItem);
		}
		
		m_aryCaseHistoryDetailProductItems.Add(newCHDPI);
		
	}NxCatchAll("Error linking selected products to the case history item.");
}

BOOL CCaseHistoryDlg::AddCaseHistoryDetailProductItem(long ServiceID, double &dblQuantity, long CaseHistoryDetailID, long &CaseHistoryDetailProductItemListID, BOOL bAllowQuantityChange /*= TRUE*/) {

	try {

		long LocationID = -1;

		if(m_dlLocations->GetCurSel()==-1)
			LocationID = GetCurrentLocationID();
		else 
			LocationID = m_dlLocations->GetValue(m_dlLocations->GetCurSel(),0).lVal;

		// (j.jones 2008-02-29 10:54) - PLID 29125 - see if we have a linked allocation that is completed
		long nAllocationID = -1;
		InvUtils::InventoryAllocationStatus iasStatus = InvUtils::iasActive;
		NXDATALIST2Lib::IRowSettingsPtr pAllocRow = m_InvAllocationCombo->GetCurSel();
		if(pAllocRow) {
			nAllocationID = VarLong(pAllocRow->GetValue(iaccID), -1);
			iasStatus = (InvUtils::InventoryAllocationStatus)VarLong(pAllocRow->GetValue(iaccStatus), InvUtils::iasActive);
			if(iasStatus != InvUtils::iasCompleted) {
				//if not completed, don't use the allocation
				nAllocationID = -1;
			}
		}

		CString strLinkedAllocation = "";
		if(nAllocationID != -1) {
			strLinkedAllocation.Format(" AND AllocationID <> %li ", nAllocationID);
		}

		// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
		if(!IsRecordsetEmpty("SELECT ID FROM ProductItemsT WHERE ProductID = %li "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				%s "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null)",
			ServiceID, InvUtils::iadsActive, InvUtils::iadsUsed, strLinkedAllocation, LocationID)) {

			BOOL bCancel = FALSE;
			BOOL bLoop = TRUE;

			//while we still need to prompt
			while(bLoop) {
				//there are product items available, so lets prompt
				CProductItemsDlg dlg(this);
				dlg.m_EntryType = PI_SELECT_DATA;
				dlg.m_ProductID = ServiceID;
				dlg.m_nLocationID = LocationID;
				dlg.m_CountOfItemsNeeded = (long)dblQuantity;
				dlg.m_strWhere = GetProductItemWhereClause();
				//we will only allow the auto-qty-grow when selecting an individual product
				dlg.m_bAllowQtyGrow = FALSE;
				dlg.m_bDisallowQtyChange = !bAllowQuantityChange;
				// (j.jones 2008-02-29 12:04) - PLID 29125 - pass in our allocation ID
				if(nAllocationID != -1) {
					dlg.m_nLinkedAllocationID = nAllocationID;
				}

				// (j.jones 2007-12-19 14:49) - PLID 28412 - removed this registration code
				/*
				//register the products dlg for barcode messages
				if(GetMainFrame()) {
					if(!GetMainFrame()->RegisterForBarcodeScan(&dlg))
						MessageBox("Error registering for barcode scans.  You may not be able to scan.");
				}
				*/

				//prompt
				if(IDCANCEL == dlg.DoModal()) {
					//if they cancelled, warn them!
					if(IDYES == MessageBox("You have chosen to cancel linking serial numbered / expirable items to this product."
						"\nThe item will be removed from the list if you do not fill in the requested information."
						"\nAre you SURE you wish to cancel?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						//if they wish to not bill the item, stop looping, cancel adding this item
						bLoop = FALSE;
						bCancel = TRUE;
					}
				}
				else {
					//if they selected an item, then stop looping and process
					bLoop = FALSE;

					dblQuantity = (double)dlg.m_CountOfItemsNeeded;
					CaseHistoryDetailProductItemListID = NewCaseHistoryDetailProductItemListID();
					AddToCaseHistoryDetailProductItemsArray(
						CaseHistoryDetailID,
						CaseHistoryDetailProductItemListID,
						dlg.m_adwProductItemIDs);

					return TRUE;
				}

				// (j.jones 2007-12-19 14:49) - PLID 28412 - removed this registration code
				/*
				//unregister products dlg for barcode messages
				if(GetMainFrame()) {
					if(!GetMainFrame()->UnregisterForBarcodeScan(&dlg))
						MessageBox("Error unregistering for barcode scans.");
				}
				*/
			}
			if(bCancel) {
				//if the loop ended with a cancellation, return false
				return FALSE;
			}			
			
		}
		else {
			//if the product doesn't have any ProductItems, see if it requires them, in which case they cannot use it
			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Name, HasSerialNum, HasExpDate FROM ServiceT "
				"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"WHERE ServiceT.ID = {INT} AND (HasSerialNum = 1 OR HasExpDate = 1)", ServiceID);
			if(!rs->eof) {
				CString strName = AdoFldString(rs, "Name","");
				BOOL bHasSerialNum = AdoFldBool(rs, "HasSerialNum",FALSE);
				BOOL bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
				CString str;
				str.Format("The product '%s' requires %s%s%s, but has no items in stock at the selected location.\n"
					"This product cannot be used with no items in stock. It will be removed from the list.",strName,
					bHasSerialNum ? "a serial number" : "",
					(bHasSerialNum && bHasExpDate) ? " and " : "",
					bHasExpDate ? "an expiration date" : "");
				MessageBox(str);
				return FALSE;
			}
			rs->Close();			
		}
		
		return TRUE;

	}NxCatchAll("Error adding serialized product to case history.");

	return TRUE;
}

void CCaseHistoryDlg::MarkCaseHistoryDetailProductItemListDeleted(long CaseHistoryDetailProductItemListID) {
	
	try {

		for(int i=0;i<m_aryCaseHistoryDetailProductItems.GetSize();i++) {
			CaseHistoryDetailProductItemList *delCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
			if(delCHDPI->ID == CaseHistoryDetailProductItemListID) {

				for(int q=0;q<delCHDPI->ProductItemAry.GetSize();q++) {
					delCHDPI->ProductItemAry.GetAt(q)->SaveStatus = CHDPI_DELETE;
				}
				break;
			}
		}

	}NxCatchAll("Error removing linked product from the charge.");
}

long CCaseHistoryDlg::LoadIntoCaseHistoryDetailProductItemsArray(long CaseHistoryDetailID) {

	try {

		long CaseHistoryDetailProductItemListID = -1;
	
		// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProductItemID FROM ChargedProductItemsT WHERE CaseHistoryDetailID = {INT}",CaseHistoryDetailID);

		if(!rs->eof) {

			//initialize the CPI list
			CaseHistoryDetailProductItemList *newCHDPI = new (CaseHistoryDetailProductItemList);
			newCHDPI->CaseHistoryDetailID = CaseHistoryDetailID;
			CaseHistoryDetailProductItemListID = NewCaseHistoryDetailProductItemListID();
			newCHDPI->ID = CaseHistoryDetailProductItemListID;

			//now add the product items
			while(!rs->eof) {
				long ProductItemID = AdoFldLong(rs, "ProductItemID");
				CHProductItems *newProdItem = new (CHProductItems);
				newProdItem->ProductItemID = ProductItemID;
				newProdItem->SaveStatus = CHDPI_NONE;
				newCHDPI->ProductItemAry.Add(newProdItem);

				rs->MoveNext();
			}

			m_aryCaseHistoryDetailProductItems.Add(newCHDPI);
		}
		rs->Close();
		
		return CaseHistoryDetailProductItemListID;

	}NxCatchAll("Error loading products associated with a case history item.");

	return -1;
}

BOOL CCaseHistoryDlg::ChangeProductItems(long nRow, double dblQuantity) {

	//we're editing the serialized item list for the charge

	try {

		long CaseHistoryDetailID = -2;
		long CaseHistoryDetailProductItemListID = -1;
		long ProductID = -1;

		CaseHistoryDetailID = VarLong(m_dlCaseHistoryDetails->GetValue(nRow,chdlcID));
		CaseHistoryDetailProductItemListID = VarLong(m_dlCaseHistoryDetails->GetValue(nRow,chdlcCHDProductItemListID));
		ProductID = VarLong(m_dlCaseHistoryDetails->GetValue(nRow,chdlcItemID));
		dblQuantity = VarDouble(m_dlCaseHistoryDetails->GetValue(nRow,chdlcQuantity));

		long LocationID = -1;

		if(m_dlLocations->GetCurSel()==-1)
			LocationID = GetCurrentLocationID();
		else 
			LocationID = m_dlLocations->GetValue(m_dlLocations->GetCurSel(),0).lVal;

		// (j.jones 2008-02-29 10:54) - PLID 29125 - see if we have a linked allocation that is completed
		long nAllocationID = -1;
		InvUtils::InventoryAllocationStatus iasStatus = InvUtils::iasActive;
		NXDATALIST2Lib::IRowSettingsPtr pAllocRow = m_InvAllocationCombo->GetCurSel();
		if(pAllocRow) {
			nAllocationID = VarLong(pAllocRow->GetValue(iaccID), -1);
			iasStatus = (InvUtils::InventoryAllocationStatus)VarLong(pAllocRow->GetValue(iaccStatus), InvUtils::iasActive);
			if(iasStatus != InvUtils::iasCompleted) {
				//if not completed, don't use the allocation
				nAllocationID = -1;
			}
		}

		//first loop up until the CaseHistoryDetailProductItem list in question
		for(int j=0;j<m_aryCaseHistoryDetailProductItems.GetSize();j++) {
			CaseHistoryDetailProductItemList *CHDPIlist = m_aryCaseHistoryDetailProductItems.GetAt(j);
			if(CHDPIlist->ID == CaseHistoryDetailProductItemListID) {

				if((long)dblQuantity != dblQuantity) {
					MessageBox("This product is being tracked by either a serial number or expiration date,\n"
								"and requires that it is only used in increments of 1.\n"
								"Please enter a whole number for the quantity.");
					return FALSE;
				}

				CProductItemsDlg dlg(this);
				dlg.m_EntryType = PI_SELECT_DATA;
				dlg.m_ProductID = ProductID;
				dlg.m_nLocationID = LocationID;
				dlg.m_CountOfItemsNeeded = (long)dblQuantity;
				dlg.m_strWhere = GetProductItemWhereClause();
				// (j.jones 2008-02-29 12:04) - PLID 29125 - pass in our allocation ID
				if(nAllocationID != -1) {
					dlg.m_nLinkedAllocationID = nAllocationID;
				}

				//fill in the existing values
				for(int z=0;z<CHDPIlist->ProductItemAry.GetSize();z++) {
					long ProductItemID = CHDPIlist->ProductItemAry.GetAt(z)->ProductItemID;
					if(CHDPIlist->ProductItemAry.GetAt(z)->SaveStatus != CHDPI_DELETE)
						dlg.m_adwProductItemIDs.Add(ProductItemID);
				}

				// (j.jones 2007-12-19 14:49) - PLID 28412 - removed this registration code
				/*
				//register the products dlg for barcode messages
				if(GetMainFrame()) {
					if(!GetMainFrame()->RegisterForBarcodeScan(&dlg))
						MessageBox("Error registering for barcode scans.  You may not be able to scan.");
				}
				*/

				if(dlg.DoModal() != IDCANCEL) {
					//in this case, they can cancel, and we only make changes if they did not

					dblQuantity = (double)dlg.m_CountOfItemsNeeded;

					//now loop through each item and see if we removed anything
					for(int i=0;i<CHDPIlist->ProductItemAry.GetSize();i++) {
						long ProductItemID = CHDPIlist->ProductItemAry.GetAt(i)->ProductItemID;
						BOOL bFound = FALSE;
						for(int x=0;x<dlg.m_adwProductItemIDs.GetSize();x++) {								
							if(ProductItemID == (long)dlg.m_adwProductItemIDs.GetAt(x)) {
								bFound = TRUE;
								break;
							}
						}
						if(!bFound)
							CHDPIlist->ProductItemAry.GetAt(i)->SaveStatus = CHDPI_DELETE;
					}

					//now loop to see if we added anything
					//now loop through each item and compare
					for(int q=0;q<dlg.m_adwProductItemIDs.GetSize();q++) {
						long ProductItemID = (long)dlg.m_adwProductItemIDs.GetAt(q);
						BOOL bFound = FALSE;
						for(int x=0;x<CHDPIlist->ProductItemAry.GetSize();x++) {								
							if(ProductItemID == CHDPIlist->ProductItemAry.GetAt(x)->ProductItemID) {
								bFound = TRUE;
								break;
							}
						}
						if(!bFound) {
							//add the item new
							CHProductItems *newProdItem = new (CHProductItems);
							newProdItem->ProductItemID = ProductItemID;
							newProdItem->SaveStatus = CHDPI_SAVENEW;
							CHDPIlist->ProductItemAry.Add(newProdItem);
						}
					}					
				}

				// (j.jones 2007-12-19 14:49) - PLID 28412 - removed this registration code
				/*
				//unregister products dlg for barcode messages
				if(GetMainFrame()) {
					if(!GetMainFrame()->UnregisterForBarcodeScan(&dlg))
						MessageBox("Error unregistering for barcode scans.");
				}
				*/
			}				
		}

		m_dlCaseHistoryDetails->PutValue(nRow,chdlcQuantity,(double)dblQuantity);

		return TRUE;

	}NxCatchAll("Error changing the products attached to the selected item.");

	return FALSE;
}

void CCaseHistoryDlg::OnEditingStartingCasehistoryDetailsList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		if(nRow == -1) {
			*pbContinue = FALSE;
			return;
		}
		else if(VarLong(m_dlCaseHistoryDetails->GetValue(nRow, chdlcID)) == (long)MAXLONG) {
			//This is the bottom "Add" row.
			*pbContinue = FALSE;
			return;
		}

		long ItemType = VarLong(m_dlCaseHistoryDetails->GetValue(nRow,chdlcItemType),0);

		if(ItemType == -3) {
			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			if(nCol == chdlcAmount || nCol == chdlcBillable /*|| nCol == chdlcPayToPractice*/) {
				*pbContinue = FALSE;
			}

			// (j.jones 2008-07-01 09:32) - PLID 18744 - if editing the cost for a person,
			// check and see if they have permissions for that
			if(nCol == chdlcVisibleCost
				&& (!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)
				|| !(GetCurrentUserPermissions(bioContactsDefaultCost) & sptWrite))) {
				*pbContinue = FALSE;
			}
		}
		else if(ItemType == -2) {
			if(nCol == chdlcVisibleCost) {
				*pbContinue = FALSE;
			}
		}

	}NxCatchAll("Error editing case history detail.");
}

void CCaseHistoryDlg::OnColumnClickingCasehistoryDetailsList(short nCol, BOOL FAR* bAllowSort) 
{
	//do nothing, it's now handled in OnChangeColumnSortFinishedCasehistoryDetailsList()
}

void CCaseHistoryDlg::OnCancel() 
{
	// (d.thompson 2010-01-11) - PLID 23267 - Warn before cancelling
	if(AfxMessageBox("Cancelling this case history will lose any changes.  Are you sure you wish to cancel and discard any changes?", MB_YESNO) == IDNO) {
		return;
	}

	GetMainFrame()->UnregisterForBarcodeScan(this);
	CDialog::OnCancel();

	//if this is a tracked case history, from the ASC module,
	//update the parent's view
	if(m_pParent) {
		m_pParent->OnClosedCaseHistory(m_nCaseHistoryID);
	}
}

void CCaseHistoryDlg::OnBtnChangeAppt() 
{
	try {
		
		//if modeless, make sure we're the top window before opening a modal dialog
		if(m_pParent)
			BringWindowToTop();

		//We want all procedural appointments except for the current one
		CString strWhere;
		strWhere.Format("PatientID = %li AND AppointmentsT.ID <> %li AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4)",m_nPersonID,m_nAppointmentID);
		if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE %s", strWhere)) {
			CSelectApptDlg dlg(this);
			dlg.m_strWhere = strWhere;
			dlg.m_bAllowMultiSelect = FALSE;
			dlg.m_strLabel = "Please select an appointment to associate with this case history.";
			int nReturn = dlg.DoModal();
			if(nReturn == IDOK) {
				if(dlg.m_arSelectedIds.GetSize() > 0) {
					m_nAppointmentID = dlg.m_arSelectedIds.GetAt(0);
					DisplayAppointmentDesc();
					
					//compare the dates and location
					COleDateTime dtCurDate = m_ctrlSurgeryDate.GetValue();
					COleDateTime dtApptDate;
					// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
					// (j.jones 2009-08-26 10:23) - PLID 34943 - also check the location, if
					// it is active and managed, etc.
					_RecordsetPtr rs = CreateParamRecordset("SELECT Date, LocationsQ.ID AS LocID "
						"FROM AppointmentsT "
						"LEFT JOIN (SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1) "
						"	AS LocationsQ ON AppointmentsT.LocationID = LocationsQ.ID "
						"WHERE AppointmentsT.ID = {INT} ", m_nAppointmentID);
					if(!rs->eof) {
						dtApptDate = AdoFldDateTime(rs, "Date");
						if(dtCurDate.GetDay() != dtApptDate.GetDay() ||
							dtCurDate.GetMonth() != dtApptDate.GetMonth() ||
							dtCurDate.GetYear() != dtApptDate.GetYear()) {
							if(IDYES == MessageBox("The date on this appointment is different than the surgery date on this case history.\n"
								"Would you like to update the surgery date to reflect the appointment date?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								m_ctrlSurgeryDate.SetValue(_variant_t(dtApptDate));
							}
						}

						// (j.jones 2009-08-26 10:24) - PLID 34943 - ask to use the location IF the preference says
						// to use appt. locations (-3)
						if(-3 == GetRemotePropertyInt("DefaultCaseHistoryLocation", -1, 0, "<None>", true)) {
							long nNewLocID = AdoFldLong(rs, "LocID", -1);
							
							if(nNewLocID != -1 && nNewLocID != m_nCurLocationID) {
								if(IDYES == MessageBox("The location selected on this appointment is different than the location on this case history.\n"
									"Would you like to update the case history location to reflect the appointment location?","Practice",MB_ICONQUESTION|MB_YESNO)) {

									//don't change it yet, just try to fire the selection changed, since there
									//is various processing that must occur before truly changing it
									long nRow = m_dlLocations->FindByColumn(0, (long)nNewLocID, 0, TRUE);
									if(nRow != -1) {
										OnSelChosenLocationCombo(nRow);
									}
									else {
										ASSERT(FALSE);
									}

									//if they changed the location, ask if they also want to change the place of service
									if(m_dlPOS->CurSel != -1) {
										long nPOSID = VarLong(m_dlPOS->GetValue(m_dlPOS->CurSel, 0));
										if(nNewLocID != -1 && nNewLocID != nPOSID) {
											if(IDYES == MessageBox("Would you also like to update the place of service to reflect this new location?","Practice",MB_ICONQUESTION|MB_YESNO)) {

												long nRow = m_dlPOS->FindByColumn(0, (long)nNewLocID, 0, TRUE);
												if(nRow != -1) {
													OnSelChosenCaseHistoryPosCombo(nRow);
												}
												else {
													ASSERT(FALSE);
												}
											}
										}
									}
								}
							}
						}
					}
					rs->Close();

					//compare the procedures
					if(m_arProcedureIDs.GetSize() == 0) {
						//TES 5/6/2008 - PLID 29931 - Make sure not to load any non-procedural purposes.
						// (c.haag 2008-12-26 11:37) - PLID 32539 - For reasons mentioned in a similar query earlier up in this source file, we
						// do not filter out inactive procedures.
						if(ReturnsRecords("SELECT PurposeID FROM AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID WHERE AppointmentID = %li",m_nAppointmentID)) {
							if(IDYES == MessageBox("This appointment has procedures not selected on this case history. Would you like to add the appointment procedures?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
								_RecordsetPtr rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID WHERE AppointmentID = {INT}",m_nAppointmentID);
								while(!rs->eof) {
									m_arProcedureIDs.Add(AdoFldLong(rs, "PurposeID"));
									rs->MoveNext();
								}
								rs->Close();
							}
						}
					}
					else {
						//TES 5/6/2008 - PLID 29931 - Make sure not to load any non-procedural purposes.
						// (c.haag 2008-12-26 11:37) - PLID 32539 - For reasons mentioned in a similar query earlier up in this source file, we
						// do not filter out inactive procedures.
						CString strTemp = ArrayAsString(m_arProcedureIDs, false);
						if(ReturnsRecords("SELECT PurposeID FROM AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID WHERE AppointmentID = %li AND PurposeID NOT IN (%s)",m_nAppointmentID, strTemp)) {
							if(IDYES == MessageBox("This appointment has procedures not selected on this case history. Would you like to add the appointment procedures?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								//this recordset cannot be parameterized
								_RecordsetPtr rs = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID WHERE AppointmentID = %li AND PurposeID NOT IN (%s)",m_nAppointmentID, strTemp);
								while(!rs->eof) {
									m_arProcedureIDs.Add(AdoFldLong(rs, "PurposeID"));
									rs->MoveNext();
								}
								rs->Close();
							}
						}
					}

					DisplayProcedureInfo();
				}
				// (j.gruber 2008-07-09 17:14) - PLID 15807 - Load Anesthesia
				LoadAnesthesia();
			}
		}
		else {
			MessageBox("There are no unattached appointments to associate with this case history.");
		}

	}NxCatchAll("Error changing appointment.");
}


void CCaseHistoryDlg::DisplayAppointmentDesc()
{
	try {

		CString strAppt = "<No Appointment>";

		if(m_nAppointmentID != -1) {

			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 Date, StartTime, AptTypeT.Name, "
				"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, dbo.GetResourceString(AppointmentsT.ID) AS Resource "
				"FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"WHERE AppointmentsT.ID = {INT}",m_nAppointmentID);
			if(!rs->eof) {
				COleDateTime dtDate, dtStartTime;
				CString strType, strPurpose, strResource;

				dtDate = AdoFldDateTime(rs, "Date");
				dtStartTime = AdoFldDateTime(rs, "StartTime");
				strType = AdoFldString(rs, "Name","");
				strPurpose = AdoFldString(rs, "Purpose","");
				strResource = AdoFldString(rs, "Resource","");

				if(strType == "")
					strType = "<No Type>";

				if(strPurpose == "")
					strPurpose = "<No Purpose>";

				strAppt.Format("%s %s, %s - %s, %s",FormatDateTimeForInterface(dtDate,NULL,dtoDate),FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime),
					strType,strPurpose,strResource);
			}
			rs->Close();

			GetDlgItem(IDC_BTN_CLEAR_APPT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_GOTO_APPT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_BTN_CLEAR_APPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_GOTO_APPT)->EnableWindow(FALSE);
		}

		//TES 1/9/2007 - PLID 23575 - If our last appointment description is empty, then we must not have set it yet, because
		// even if there was no appointment it would be "<No Appointment>", not empty.  So, set it.
		if(m_strLastSavedAppointmentDesc.IsEmpty()) {
			m_strLastSavedAppointmentDesc = strAppt;
		}

		SetDlgItemText(IDC_APPOINTMENT_DESC,strAppt);

	}NxCatchAll("Error displaying appointment information");
}

void CCaseHistoryDlg::OnBtnClearAppt() 
{
	if(IDNO == MessageBox("Are you sure you wish to remove the link to this appointment?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
	}

	m_nAppointmentID = -1;
	DisplayAppointmentDesc();
}

void CCaseHistoryDlg::OnBtnGotoAppt() 
{
	try {

		if(m_nAppointmentID == -1)
			return;

		//don't let them leave if they have any other open case histories
		long Count = 0;
		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					Count++;
				}
			}
		}

		//obviously the count will be at least 1... this one! And we will close it.
		if(Count > 1) {
			MessageBox("You will need to close all other open Case Histories before leaving the Surgery Center module.");
			return;
		}


		if(IDNO == MessageBox("This case history will be saved and closed. Are you sure you wish to switch to the patient's appointment?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//we want to close our dialog but can't access m_nAppointmentID once it's closed
		long nApptID = m_nAppointmentID;

		//OnOK will save, close, and decrement the count of open cases
		OnOK();

		//see whether the appointments been cancelled
		if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE Status = 4 AND ID = %li",nApptID)) {
			MessageBox("This appointment has been cancelled");
			return;
		}
				
		CMainFrame  *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {
			if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					((CSchedulerView *)pView)->OpenAppointment(nApptID, GetRemotePropertyInt("ApptAutoOpenResEntry", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE);
				}//end pView
			}
		}//end pMainFrame
		else {
			//MainFrame pointer is null
			MessageBox("Error switching to scheduler.");
		}

	}NxCatchAll("Error switching to appointment.");
}

LRESULT CCaseHistoryDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	CString str;
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string
	long row = -1;
	BOOL bQtyIncrementFailed = TRUE;

	// Put in a mutex wanna-be
	if (m_bIsScanningBarcode) return 0;
	m_bIsScanningBarcode = TRUE;
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	//var.SetString((const char*)bstr);
	_variant_t var(bstr);

	try {
		// (j.gruber 2007-04-12 08:54) - PLID 25597 - fixed query error
		// (j.jones 2008-05-20 17:46) - PLID 29249 - added IsSerializedProduct
		//TES 7/16/2008 - PLID 27983 - Added IsLinkedToProducts
		// (j.jones 2008-09-15 11:27) - PLID 31375 - changed to include the ProductLocationInfoT.Billable status
		//(c.copits 2010-09-17) PLID 40317 - Allow duplicate UPC codes for FramesData certification
		_RecordsetPtr rsServiceID = GetBestUPCProduct(var);
		//_RecordsetPtr rsServiceID = CreateParamRecordset("SELECT ServiceT.ID, Name, Price, "
		//	"CASE WHEN ProductT.ID Is Null THEN 0 ELSE "
		//	"ProductT.LastCostPerUU END AS LastCost, "
		//	"Convert(bit, CASE WHEN ProductT.ID Is Not Null AND (HasSerialNum = 1 OR HasExpDate = 1) THEN 1 ELSE 0 END) AS IsSerializedProduct, "
		//	"CASE WHEN ProductT.ID Is Null THEN 0 ELSE 1 END AS Type, "
		//	"convert(bit,CASE WHEN ServiceT.ID IN (SELECT CptID FROM ServiceToProductLinkT) THEN 1 ELSE 0 END) AS IsLinkedToProducts, "
		//	"ProductLocationInfoQ.Billable AS BillableForLocation "
		//	"FROM ServiceT "
		//	"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		//	"LEFT JOIN (SELECT Billable, ProductID FROM ProductLocationInfoT WHERE LocationID = {INT}) "
		//	"	AS ProductLocationInfoQ ON ProductT.ID = ProductLocationInfoQ.ProductID "
		//	"WHERE Barcode = {STRING}", m_nCurLocationID, VarString(var));
		if(!rsServiceID->eof) {

			BOOL bCheckAndWarnCredentials = GetRemotePropertyInt("CredentialWarnCaseHistories",0,0,"<None>",true) == 1;
			BOOL bCheckAndWarnLicensing = GetRemotePropertyInt("ExpLicenseWarnCaseHistories", bCheckAndWarnCredentials ? 1 : 0, 0, "<None>",true) == 1;
			BOOL bWarnCredentialingTOS2 = GetRemotePropertyInt("CredentialWarningOnlyTOS2",0,0,"<None>",true) == 1;
			long nProvIndex = m_dlProviders->GetCurSel();

			//only warn for Service Codes
			if((bCheckAndWarnCredentials || bCheckAndWarnLicensing) && AdoFldLong(rsServiceID, "Type") == 0 && nProvIndex != -1) {

				long nServiceID = AdoFldLong(rsServiceID, "ID");

				long nProviderID = m_dlProviders->GetValue(nProvIndex, chdpccID);

				if(bCheckAndWarnLicensing) {

					ECredentialWarning eCredWarning = CheckPersonCertifications(nProviderID);

					if(eCredWarning != ePassedAll) {

						CString str;

						if(eCredWarning == eFailedLicenseExpired) {

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nProviderID);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The selected provider has the following expired licenses:\n\n%s\n"
								"Do you still wish to add this service to this case history?",strLicenses);
						}
						else if(eCredWarning == eFailedLicenseExpiringSoon) {

							//check if a license will expire within the given day range
							long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

							CString strLicenses;
							// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
							_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
								"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nProviderID,nLicenseWarnDayRange);
							while(!rs2->eof) {
								strLicenses += AdoFldString(rs2, "ExpiredLicense","");
								strLicenses += "\n";
								rs2->MoveNext();
							}
							rs2->Close();

							str.Format("The following licenses are about to expire for the selected provider:\n\n%s\n"
								"Do you still wish to add this service to this case history?",strLicenses);
						}

						if(IDNO == MessageBox(str,NULL,MB_YESNO)) {
							m_bIsScanningBarcode = FALSE;
							return 0;
						}
					}
				}

				if(bCheckAndWarnCredentials) {

					ECredentialWarning eCredWarning = CheckServiceCodeCredential(nProviderID, nServiceID);

					if(eCredWarning != ePassedAll) {

						//it's not credentialed. Check the TOS status before we warn though.
						BOOL bWarn = TRUE;
						if(bWarnCredentialingTOS2 && IsRecordsetEmpty("SELECT ID FROM CPTCodeT WHERE TypeOfService = '2' AND ID = %li",nServiceID))
							bWarn = FALSE;

						if(bWarn) {

							CString str;

							if(eCredWarning == eFailedCredential) {

								CString strCodeName;
								// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
								_RecordsetPtr rs2 = CreateParamRecordset("SELECT Code + ' - ' + Name AS Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = {INT}",nServiceID);
								if(!rs2->eof) {
									strCodeName = AdoFldString(rs2, "Name","");
								}
								rs2->Close();

								str.Format("The selected provider is not credentialed for the Service code '%s'\n"
									"Do you still wish to add it to this case history?",strCodeName);
							}

							if(bWarn && IDNO == MessageBox(str,NULL,MB_YESNO)) {
								m_bIsScanningBarcode = FALSE;
								return 0;
							}
						}
					}
				}
			}

			// (j.jones 2008-09-15 11:32) - PLID 31375 - passed BillableForLocation to AddDetail
			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			AddDetail(AdoFldLong(rsServiceID, "ID"), AdoFldLong(rsServiceID, "Type")?chditProduct:chditCptCode, 
				AdoFldString(rsServiceID, "Name"), 1, AdoFldCurrency(rsServiceID, "Price"), 
				AdoFldCurrency(rsServiceID, "LastCost", COleCurrency(0,0)), true,
				GetRemotePropertyInt("BarcodeAllowQtyIncrementCaseHistory",1,0,"<None>",TRUE)?true:false,
				AdoFldBool(rsServiceID, "IsSerializedProduct", FALSE),
				AdoFldBool(rsServiceID, "IsLinkedToProducts", FALSE),
				rsServiceID->Fields->Item["BillableForLocation"]->Value);
		}
	}NxCatchAll("Error in CCaseHistoryDlg::OnBarcodeScan()");

	m_bIsScanningBarcode = FALSE;
	return 0;
}

void CCaseHistoryDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawProcedureLabel(&dc);
}

void CCaseHistoryDlg::DrawProcedureLabel(CDC *pdc)
{
	// Draw the procedures
	if(m_bIsProcedureListHidden) {
		// (j.jones 2008-05-01 15:54) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcMultiProcedureLabel, m_strProcedureList, m_bIsProcedureListHidden?dtsHyperlink:dtsDisabledHyperlink, false, DT_LEFT, true, false, 0);
	}
}

BOOL CCaseHistoryDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (m_bIsProcedureListHidden) {
		if (m_rcMultiProcedureLabel.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CCaseHistoryDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDown(nFlags, point);
}

void CCaseHistoryDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CCaseHistoryDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	if (m_bIsProcedureListHidden) {
		if (m_rcMultiProcedureLabel.PtInRect(point)) {
			OnMultiProcedure();
		}	
	}
}

void CCaseHistoryDlg::DisplayProcedureInfo()
{
	try {
		//if we only have 1 item, select it in the datalist, don't bother setting this all up
		if(m_arProcedureIDs.GetSize() <= 1) {
			m_bIsProcedureListHidden = FALSE;
			((CWnd*)GetDlgItem(IDC_PROCEDURE_LIST))->ShowWindow(SW_SHOW);
			((CWnd*)GetDlgItem(IDC_MULTI_PROCEDURE_LABEL))->ShowWindow(SW_HIDE);

			if(m_arProcedureIDs.GetSize() == 1) {
				if (NXDATALISTLib::sriNoRow == m_dlProcedures->SetSelByColumn(0, m_arProcedureIDs[0])) {
					// (c.haag 2009-01-08 12:11) - PLID 32539 - The procedure may be inactive, so we need
					// to know right away if it will appear. If not, we need to add it to the list.
					_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", m_arProcedureIDs[0]);
					if (!prs->eof) {
						NXDATALISTLib::IRowSettingsPtr pRow = m_dlProcedures->GetRow(-1);
						pRow->PutValue(0,m_arProcedureIDs[0]);
						pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
						m_dlProcedures->AddRow(pRow);
						m_dlProcedures->Sort();
						m_dlProcedures->SetSelByColumn(0, m_arProcedureIDs[0]);
					} else {
						// It was deleted. Nothing we can do.
					}
				}
			}
			return;
		}

		m_bIsProcedureListHidden = TRUE;

		//populate the readable string
		m_strProcedureList = GetStringOfProcedures();

		//hide the datalist, the string will paint itself
		((CWnd*)GetDlgItem(IDC_PROCEDURE_LIST))->ShowWindow(SW_HIDE);

		InvalidateRect(m_rcMultiProcedureLabel);
	
	}NxCatchAll("Error displaying Procedure information.");
}

void CCaseHistoryDlg::OnMultiProcedure()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProcedureT");

		//see if we have anything already
		dlg.PreSelect(m_arProcedureIDs);

		dlg.m_strNameColTitle = "Procedure";

		// (c.haag 2008-12-26 11:41) - PLID 32539 - Filter out inactive procedures
		CString strWhere = "Inactive = 0";
		if (m_arProcedureIDs.GetSize() > 0) {
			strWhere += FormatString(" OR ProcedureT.ID IN (%s)", ArrayAsString(m_arProcedureIDs));
		}
		int res = dlg.Open("ProcedureT", strWhere, "ProcedureT.ID", "ProcedureT.Name", "Select Procedures");

		BringWindowToTop();

		if(res == IDCANCEL)
			return;

		//save all our id's for later parsing
		dlg.FillArrayWithIDs(m_arProcedureIDs);

		// (j.gruber 2008-07-09 16:26) - PLID 15807 - load the anesthesia from here
		LoadAnesthesia();
		
		DisplayProcedureInfo();

		m_bHasProcedureBeenChanged = TRUE;

	} NxCatchAll("Error in OnMultiProcedure()");
}

CString CCaseHistoryDlg::GetStringOfProcedures() {

	CString str = "";

	//this recordset cannot be parameterized
	_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ProcedureT WHERE ID IN (%s)",ArrayAsString(m_arProcedureIDs, false));
	while(!rs->eof) {
		str += AdoFldString(rs, "Name","");
		str += ", ";
		rs->MoveNext();
	}
	rs->Close();

	str.TrimRight(", ");

	return str;
}


void CCaseHistoryDlg::LoadAnesthesia() {

	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAnesType->CurSel;

		if (pRow) {
			CString strValue = VarString(pRow->GetValue(0), "");
			if (!strValue.IsEmpty()) {
				//if something is in the box, don't fill it out
				return;
			}
		}	


		CString strIDs;
		for (int i = 0; i < m_arProcedureIDs.GetSize(); i++) {
			strIDs += AsString(m_arProcedureIDs[i]) + "," ;
		}
		strIDs.TrimRight(",");

		if (m_arProcedureIDs.GetSize() > 0) {
			//this recordset cannot be parameterized
			_RecordsetPtr rsAnes = CreateRecordset("SELECT Anesthesia FROM ProcedureT WHERE ID IN (%s) GROUP BY Anesthesia", strIDs);
			CStringArray arystrAnes;
			while (!rsAnes->eof) {
				CString strAnes = AdoFldString(rsAnes, "Anesthesia", "");
				if (!strAnes.IsEmpty()) {
					arystrAnes.Add(strAnes);
				}
				rsAnes->MoveNext();
			}

			if (arystrAnes.GetSize() == 1) {

				//set out list to it
				m_pAnesType->SetSelByColumn(0, _variant_t(arystrAnes.GetAt(0)));
			}
			else if (arystrAnes.GetSize() > 1) {

				//we have to have them select
				//Ask the user to select an anesthesia
				CSelectDlg dlg(this);
				dlg.m_strTitle = "Select an Anesthesia";
				dlg.m_strCaption = "The anesthesia to be used for the combination of these procedures could not be determined.  Please select an anesthesia from the list below.";
				dlg.m_strFromClause = "(SELECT Anesthesia FROM AnesthesiaTypes UNION SELECT '<No Anesthesia Selected>') SubQ";
				dlg.AddColumn("Anesthesia", "Anesthesia", TRUE, FALSE);
				if(dlg.DoModal() == IDOK) {

					CString strAnesthesia = VarString(dlg.m_arSelectedValues[0], "");
					//Set our selected anesthesia
					m_pAnesType->SetSelByColumn(0, _variant_t(strAnesthesia));
					

					//Determine if <No Anesthesia Selected> has been chosen
					if(strAnesthesia == "<No Anesthesia Selected>") {
						m_pAnesType->SetSelByColumn(0, _variant_t(""));				
					}
				}
				else {
					// We pressed cancel, so do nothing
					
				}
			}
			else {
				//there are no procedures, so don't do anything
			}
		}

	}NxCatchAll("Error in CCaseHistoryDlg::LoadAnesthesia()");



}

void CCaseHistoryDlg::OnSelChosenProcedureList(long nRow) 
{
	try {
		//a single item was chosen.  this is disabled if they've done a multi-select, 
		//so we can safely update our id list
		if(m_dlProcedures->GetCurSel() != -1) {
			//if the ID is -2, it's the "multiple" row
			if(VarLong(m_dlProcedures->GetValue(nRow, 0)) == -2) {
				m_dlProcedures->PutCurSel(-1);
				OnMultiProcedure();			
			}
			//if the ID is -1, it's the "none selected" row
			else if(VarLong(m_dlProcedures->GetValue(nRow, 0)) == -1) {
				m_arProcedureIDs.RemoveAll();
			}
			else{
				m_arProcedureIDs.RemoveAll();
				m_arProcedureIDs.Add(VarLong(m_dlProcedures->GetValue(nRow, 0)));

				// (j.gruber 2008-07-09 16:26) - PLID 15807 - load the anesthesia from here
				LoadAnesthesia();
				
			}
		}
		else{
			m_arProcedureIDs.RemoveAll();
		}

		m_bHasProcedureBeenChanged = TRUE;

	} NxCatchAll("Error in OnSelChosenProcedureList()");
}

void CCaseHistoryDlg::OnKillfocusEditTotalCaseAnesthMinutes() 
{
	long nTotalMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES);

	if(nTotalMinutes > 1440) {
		MessageBox("You cannot have more than 1440 anesthesia minutes (24 hours).");
		nTotalMinutes = 1440;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, nTotalMinutes);
	}

	if(m_CurAnesthMinutes != nTotalMinutes) {
		m_CurAnesthMinutes = nTotalMinutes;
		OnAnesthesiaTimeChanged(nTotalMinutes);
	}
}

void CCaseHistoryDlg::OnKillfocusEditTotalCaseFacilityMinutes() 
{
	long nTotalMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES);

	if(nTotalMinutes > 1440) {
		MessageBox("You cannot have more than 1440 facility minutes (24 hours).");
		nTotalMinutes = 1440;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, nTotalMinutes);
	}

	if(m_CurFacilityMinutes != nTotalMinutes) {
		m_CurFacilityMinutes = nTotalMinutes;

		OnFacilityTimeChanged(nTotalMinutes);
	}
}

void CCaseHistoryDlg::OnAnesthesiaTimeChanged(long nTotalMinutes, BOOL bPlaceOfServiceChanged /*= FALSE*/)
{
	//see if there are any anesthesia codes in the list, and if so, change their value

	if(m_dlPOS->CurSel == -1)
		m_dlPOS->CurSel = 0;
	long nPlaceOfServiceID = m_dlPOS->GetValue(m_dlPOS->CurSel, 0).lVal;

	BOOL bPrompted = FALSE;

	for(int i=0; i<m_dlCaseHistoryDetails->GetRowCount()-1; i++) {

		//don't check if it's not a CPT Code
		if(VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemType)) != -2)
			continue;

		long nServiceID = VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemID));

		// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure, which required this to be checked per line item
		// (j.jones 2008-05-21 16:36) - PLID 29223 - streamlined two recordsets into one
		_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthesiaSetupT.ID "
			"FROM AnesthesiaSetupT "
			"INNER JOIN ServiceT ON AnesthesiaSetupT.ServiceID = ServiceT.ID "
			"WHERE ServiceID = {INT} AND Anesthesia = 1 AND UseAnesthesiaBilling = 1 "
			"AND LocationID = {INT} AND AnesthesiaFeeBillType <> 1", nServiceID, nPlaceOfServiceID);
		if(rs->eof) {
			//either not an anesthesia item, or a flat fee, either way we don't care that the time changed
			continue;
		}

		if(!bPrompted) {
			//this is a "don't show me again" prompt, but if they do actually see it,
			//we don't want to try and show them again during this process

			if(bPlaceOfServiceChanged) {
				//they changed the POS
				// (a.walling 2007-02-26 16:07) - PLID 24943 - Use a parent for these so focus doesn't flip around
				DontShowMeAgain(this, "Changing the Place Of Service will change the fee for your Anesthesia charges on this Case History.\n"
					"Be sure to double-check the new amounts before saving.", "CaseHistoryDlgChangeAnesthesiaPOS", "Practice", FALSE, FALSE);
			}
			else {
				//they changed the time
				// (a.walling 2007-02-26 16:07) - PLID 24943 - Use a parent for these so focus doesn't flip around
				DontShowMeAgain(this, "Changing the Anesthesia Time will change the fee for your Anesthesia charges on this Case History.\n"
					"Be sure to double-check the new amounts before saving.", "CaseHistoryDlgChangeAnesthesiaTime", "Practice", FALSE, FALSE);
			}
			bPrompted = TRUE;
		}

		//ok, we're on an anesthesia charge, they've been warned, and it's not a flat fee, so recalculate the fee!

		//default these values to their current values in the list
		COleCurrency cyAmount = VarCurrency(m_dlCaseHistoryDetails->GetValue(i, chdlcAmount), COleCurrency(0,0));
		double dblQuantity = VarDouble(m_dlCaseHistoryDetails->GetValue(i, chdlcQuantity), 0.0);

		if(bPlaceOfServiceChanged) {
			//if the POS Changed, we might not have the data stored on the Additional Info tab

			long nAnesthMinutes = nTotalMinutes;

			CString strStartTime = "", strEndTime = "";

			COleDateTime dtAnesthStart;
			if(m_nxtAnesthStart->GetStatus() == 1) {
				dtAnesthStart = StripSeconds(m_nxtAnesthStart->GetDateTime());
			}
			else {
				dtAnesthStart.SetStatus(COleDateTime::invalid);
			}
			if(dtAnesthStart.GetStatus() != COleDateTime::invalid) {
				strStartTime = dtAnesthStart.Format("%H:%M:%S");
			}

			COleDateTime dtAnesthEnd;
			if(m_nxtAnesthEnd->GetStatus() == 1) {
				dtAnesthEnd = StripSeconds(m_nxtAnesthEnd->GetDateTime());
			}
			else {
				dtAnesthEnd.SetStatus(COleDateTime::invalid);
			}
			if(dtAnesthEnd.GetStatus() != COleDateTime::invalid) {
				strEndTime = dtAnesthEnd.Format("%H:%M:%S");
			}

			long nCurAnesthMinutes = nAnesthMinutes;
			
			BOOL bAnesthesia = FALSE;
			if(CheckAnesthesia(nServiceID,bAnesthesia,cyAmount,dblQuantity,nCurAnesthMinutes,strStartTime,strEndTime,nPlaceOfServiceID,FALSE) && bAnesthesia) {
				
				if(nCurAnesthMinutes > 0)
					nAnesthMinutes = nCurAnesthMinutes;

				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, nAnesthMinutes);
				m_CurAnesthMinutes = nTotalMinutes;
				
				COleDateTime dt;
				if(strStartTime != "" && dt.ParseDateTime("1/1/1900 " + strStartTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtAnesthStart->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtAnesthStart->Clear();
				}

				if(strEndTime != "" && dt.ParseDateTime("1/1/1900 " + strEndTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtAnesthEnd->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtAnesthEnd->Clear();
				}
			}
		}
		else {
			CalcAnesthesia(nServiceID, cyAmount, dblQuantity, nTotalMinutes, nPlaceOfServiceID, FALSE);
		}

		//we have a new amount and new quantity, now put those back in the charge list
		m_dlCaseHistoryDetails->PutValue(i, chdlcAmount, _variant_t(cyAmount));		
		m_dlCaseHistoryDetails->PutValue(i, chdlcQuantity, dblQuantity);
	}
}

void CCaseHistoryDlg::OnFacilityTimeChanged(long nTotalMinutes, BOOL bPlaceOfServiceChanged /*= FALSE*/)
{
	//see if there are any facility codes in the list, and if so, change their value

	if(m_dlPOS->CurSel == -1)
		m_dlPOS->CurSel = 0;
	long nPlaceOfServiceID = m_dlPOS->GetValue(m_dlPOS->CurSel, 0).lVal;

	BOOL bPrompted = FALSE;

	for(int i=0; i<m_dlCaseHistoryDetails->GetRowCount()-1; i++) {

		//don't check if it's not a CPT Code
		if(VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemType)) != -2)
			continue;

		long nServiceID = VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemID));

		// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure, which required this to be checked per line item
		// (j.jones 2008-05-21 16:36) - PLID 29223 - streamlined two recordsets into one
		_RecordsetPtr rs = CreateParamRecordset("SELECT FacilityFeeSetupT.ID "
			"FROM FacilityFeeSetupT "
			"INNER JOIN ServiceT ON FacilityFeeSetupT.ServiceID = ServiceT.ID "
			"WHERE ServiceID = {INT} AND FacilityFee = 1 AND UseFacilityBilling = 1 "
			"AND LocationID = {INT} AND FacilityFeeBillType <> 1", nServiceID, nPlaceOfServiceID);
		if(rs->eof) {
			//either not an facility fee item, or a flat fee, either way we don't care that the time changed
			continue;
		}

		if(!bPrompted) {
			//this is a "don't show me again" prompt, but if they do actually see it,
			//we don't want to try and show them again during this process

			if(bPlaceOfServiceChanged) {
				//they changed the POS
				// (a.walling 2007-02-26 16:07) - PLID 24943 - Use a parent for these so focus doesn't flip around
				DontShowMeAgain(this, "Changing the Place Of Service will change the fee for your Facility charges on this Case History.\n"
					"Be sure to double-check the new amounts before saving.", "CaseHistoryDlgChangeFacilityPOS", "Practice", FALSE, FALSE);
			}
			else {
				//they changed the time
				// (a.walling 2007-02-26 16:07) - PLID 24943 - Use a parent for these so focus doesn't flip around
				DontShowMeAgain(this, "Changing the Facility Time will change the fee for your Facility charges on this Case History.\n"
					"Be sure to double-check the new amounts before saving.", "CaseHistoryDlgChangeFacilityTime", "Practice", FALSE, FALSE);
			}
			bPrompted = TRUE;
		}

		//ok, we're on a facility charge, they've been warned, and it's not a flat fee, so recalculate the fee!

		//default the amounr to the current value in the list
		COleCurrency cyAmount = VarCurrency(m_dlCaseHistoryDetails->GetValue(i, chdlcAmount), COleCurrency(0,0));

		if(bPlaceOfServiceChanged) {
			//if the POS Changed, we might not have the data stored on the Additional Info tab

			long nFacilityMinutes = nTotalMinutes;

			CString strStartTime = "", strEndTime = "";

			COleDateTime dtFacilityStart;
			if(m_nxtFacilityStart->GetStatus() == 1) {
				dtFacilityStart = StripSeconds(m_nxtFacilityStart->GetDateTime());
			}
			else {
				dtFacilityStart.SetStatus(COleDateTime::invalid);
			}
			if(dtFacilityStart.GetStatus() != COleDateTime::invalid) {
				strStartTime = dtFacilityStart.Format("%H:%M:%S");
			}

			COleDateTime dtFacilityEnd;
			if(m_nxtFacilityEnd->GetStatus() == 1) {
				dtFacilityEnd = StripSeconds(m_nxtFacilityEnd->GetDateTime());
			}
			else {
				dtFacilityEnd.SetStatus(COleDateTime::invalid);
			}
			if(dtFacilityEnd.GetStatus() != COleDateTime::invalid) {
				strEndTime = dtFacilityEnd.Format("%H:%M:%S");
			}

			long nCurFacilityMinutes = nFacilityMinutes;
			
			BOOL bFacilityFee = FALSE;
			if(CheckFacilityFee(nServiceID,bFacilityFee,cyAmount,nCurFacilityMinutes,strStartTime,strEndTime,nPlaceOfServiceID) && bFacilityFee) {
				
				if(nCurFacilityMinutes > 0)
					nFacilityMinutes = nCurFacilityMinutes;

				SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, nFacilityMinutes);
				m_CurFacilityMinutes = nTotalMinutes;
				
				COleDateTime dt;
				if(strStartTime != "" && dt.ParseDateTime("1/1/1900 " + strStartTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtFacilityStart->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtFacilityStart->Clear();
				}

				if(strEndTime != "" && dt.ParseDateTime("1/1/1900 " + strEndTime)) {
					// (a.walling 2007-02-27 11:29) - PLID 24944 - Strip seconds when setting the time
					m_nxtFacilityEnd->SetDateTime(StripSeconds(dt));
				}
				else {
					m_nxtFacilityEnd->Clear();
				}
			}
		}
		else {
			CalcFacilityFee(nServiceID, cyAmount, nTotalMinutes, nPlaceOfServiceID);
		}

		//we have a new amount, now put it back in the charge list
		m_dlCaseHistoryDetails->PutValue(i, chdlcAmount, _variant_t(cyAmount));
	}
}

void CCaseHistoryDlg::OnKillFocusAnesthesiaStartEdit() {
	
	m_bIsModified = TRUE;

	if(m_nxtAnesthStart->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtAnesthStart->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthStart->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtStart = StripSeconds(m_nxtAnesthStart->GetDateTime());
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtEnd = StripSeconds(m_nxtAnesthEnd->GetDateTime());
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtStart.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthStart->Clear();
		return;
	}

	//if a valid end time was entered, then update the minutes appropriately
	if(dtEnd.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);*/
		
		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalMinutes = MinuteSpan(dtStart, dtEnd); 

		if (m_dtLastEnteredAnesthesiaIn != dtStart) {
			if (dtEnd < dtStart) {
				// a.walling PLID 24944 - this means that the end time precedes the start time, so prompt to review and/or swap them
				if (IDYES == MessageBox("The 'Anesthesia End Time' precedes the 'Anesthesia Start Time.' \r\nTimes will be treated as different days; please review this before saving.\r\n\r\nWould you like to automatically swap them?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
					// swap the values.
					m_nxtAnesthStart->SetDateTime(dtEnd);
					m_nxtAnesthEnd->SetDateTime(dtStart);
					nTotalMinutes = MinuteSpan(dtEnd, dtStart);
					COleDateTime dt(dtStart); dtStart = dtEnd; dtEnd = dt;
				}
			} else if (dtEnd == dtStart) {
				MessageBox("The 'Anesthesia End Time' is equal to the 'Anesthesia Start Time.'\r\nTimes will be treated as different days; please review this before saving.", "Practice", MB_OK | MB_ICONQUESTION);
			}
		}

		//otherwise, set the total minutes and recalculate
		m_dtLastEnteredAnesthesiaIn = dtStart;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalCaseAnesthMinutes();
	}
}

void CCaseHistoryDlg::OnKillFocusAnesthesiaStopEdit() {
	
	m_bIsModified = TRUE;

	if(m_nxtAnesthEnd->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtAnesthEnd->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthEnd->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtStart = StripSeconds(m_nxtAnesthStart->GetDateTime());
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtEnd = StripSeconds(m_nxtAnesthEnd->GetDateTime());
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}	

	if(dtEnd.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthEnd->Clear();
		return;
	}

	//if a valid start time was entered, then update the minutes appropriately
	if(dtStart.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);*/

		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalMinutes = MinuteSpan(dtStart, dtEnd); 

		if (m_dtLastEnteredAnesthesiaOut != dtEnd) {
			if (dtEnd < dtStart) {
				// a.walling PLID 24944 - this means that the end time precedes the start time, so prompt to review and/or swap them
				if (IDYES == MessageBox("The 'Anesthesia End Time' precedes the 'Anesthesia Start Time.' \r\nTimes will be treated as different days; please review this before saving.\r\n\r\nWould you like to automatically swap them?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
					// swap the values.
					m_nxtAnesthStart->SetDateTime(dtEnd);
					m_nxtAnesthEnd->SetDateTime(dtStart);
					nTotalMinutes = MinuteSpan(dtEnd, dtStart);
					COleDateTime dt(dtStart); dtStart = dtEnd; dtEnd = dt;
				}
			} else if (dtEnd == dtStart) {
				MessageBox("The 'Anesthesia End Time' is equal to the 'Anesthesia Start Time.'\r\nTimes will be treated as different days; please review this before saving.", "Practice", MB_OK | MB_ICONQUESTION);
			}
		}

		//otherwise, set the total minutes and recalculate
		m_dtLastEnteredAnesthesiaOut = dtEnd;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalCaseAnesthMinutes();
	}
}

void CCaseHistoryDlg::OnKillFocusFacilityStartEdit() {
	
	m_bIsModified = TRUE;

	if(m_nxtFacilityStart->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtFacilityStart->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityStart->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtStart = StripSeconds(m_nxtFacilityStart->GetDateTime());
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtEnd = StripSeconds(m_nxtFacilityEnd->GetDateTime());
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtStart.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityStart->Clear();
		return;
	}

	//if a valid end time was entered, then update the minutes appropriately
	if(dtEnd.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);*/

		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalMinutes = MinuteSpan(dtStart, dtEnd); 

		if (m_dtLastEnteredFacilityIn != dtStart) {
			if (dtEnd < dtStart) {
				// a.walling PLID 24944 - this means that the end time precedes the start time, so prompt to review and/or swap them
				if (IDYES == MessageBox("The 'Facility End Time' precedes the 'Facility Start Time.' \r\nTimes will be treated as different days; please review this before saving.\r\n\r\nWould you like to automatically swap them?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
					// swap the values.
					m_nxtFacilityStart->SetDateTime(dtEnd);
					m_nxtFacilityEnd->SetDateTime(dtStart);
					nTotalMinutes = MinuteSpan(dtEnd, dtStart);
					COleDateTime dt(dtStart); dtStart = dtEnd; dtEnd = dt;
				}
			} else if (dtEnd == dtStart) {
				MessageBox("The 'Facility End Time' is equal to the 'Facility Start Time.'\r\nTimes will be treated as different days; please review this before saving.", "Practice", MB_OK | MB_ICONQUESTION);
			}
		}

		//otherwise, set the total minutes and recalculate
		m_dtLastEnteredFacilityIn = dtStart;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalCaseFacilityMinutes();
	}
}

void CCaseHistoryDlg::OnKillFocusFacilityStopEdit() {
	
	m_bIsModified = TRUE;

	if(m_nxtFacilityEnd->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtFacilityEnd->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityEnd->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtStart = StripSeconds(m_nxtFacilityStart->GetDateTime());
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtEnd = StripSeconds(m_nxtFacilityEnd->GetDateTime());
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtEnd.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityEnd->Clear();
		return;
	}

	//if a valid start time was entered, then update the minutes appropriately
	if(dtStart.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);*/

		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalMinutes = MinuteSpan(dtStart, dtEnd); 

		if (m_dtLastEnteredFacilityOut != dtEnd) {
			if (dtEnd < dtStart) {
				// a.walling PLID 24944 - this means that the end time precedes the start time, so prompt to review and/or swap them
				if (IDYES == MessageBox("The 'Facility End Time' precedes the 'Facility Start Time.' \r\nTimes will be treated as different days; please review this before saving.\r\n\r\nWould you like to automatically swap them?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
					// swap the values.
					m_nxtFacilityStart->SetDateTime(dtEnd);
					m_nxtFacilityEnd->SetDateTime(dtStart);
					nTotalMinutes = MinuteSpan(dtEnd, dtStart);
					COleDateTime dt(dtStart); dtStart = dtEnd; dtEnd = dt;
				}
			} else if (dtEnd == dtStart) {
				MessageBox("The 'Facility End Time' is equal to the 'Facility Start Time.'\r\nTimes will be treated as different days; please review this before saving.", "Practice", MB_OK | MB_ICONQUESTION);
			}
		}

		//otherwise, set the total minutes and recalculate
		m_dtLastEnteredFacilityOut = dtEnd;
		SetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalCaseFacilityMinutes();
	}
}

void CCaseHistoryDlg::OnSelChosenCaseHistoryPosCombo(long nRow) 
{
	if(nRow == -1)
		return;

	try {

		long nPOSID = m_dlPOS->GetValue(nRow, 0).lVal;

		OnAnesthesiaTimeChanged(GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES), TRUE);
		OnFacilityTimeChanged(GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES), TRUE);
	}
	NxCatchAll("Error in OnSelChosenCaseHistoryPosCombo");	
}

BOOL CCaseHistoryDlg::CheckAllowAddAnesthesiaFacilityCharge(long nServiceID)
{
	//check and see if nServiceID is an anesthesia code or facility code, and already exists in the charge list
	// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT Anesthesia, FacilityFee, Name, Code FROM ServiceT "
		"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = {INT} AND ((Anesthesia = 1 AND UseAnesthesiaBilling = 1) OR (FacilityFee = 1 AND UseFacilityBilling = 1))", nServiceID);
	if(!rs->eof) {
		//they are adding an anesthesia or facility fee, so see if any matching charge types exist

		BOOL bAnesthesia = AdoFldBool(rs, "Anesthesia",FALSE);
		BOOL bFacilityFee = AdoFldBool(rs, "FacilityFee",FALSE);

		BOOL bWarned = FALSE;

		// (j.jones 2007-10-15 15:12) - PLID 27757 - reworded these warnings to reflect that we now allow multiple service code setups

		for(int i=0; i<m_dlCaseHistoryDetails->GetRowCount()-1 && !bWarned;i++) {
			long nChargeServiceID = VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemID), -1);
			if(bAnesthesia && ReturnsRecords("SELECT ID FROM ServiceT WHERE ID = %li AND Anesthesia = 1 AND UseAnesthesiaBilling = 1",nChargeServiceID)) {

				//there is an anesthesia code

				bWarned = TRUE;
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is an Anesthesia charge.\n"
					"However, there is already an Anesthesia charge on this Case History.\n\n"
					"Are you sure you wish to add another Anesthesia charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
			else if(bFacilityFee && ReturnsRecords("SELECT ID FROM ServiceT WHERE ID = %li AND FacilityFee = 1 AND UseFacilityBilling = 1",nChargeServiceID)) {

				//there is a facility code

				bWarned = TRUE;
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is a Facility charge.\n"
					"However, there is already a Facility charge on this Case History.\n\n"
					"Are you sure you wish to add another Facility charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
		}
	}
	rs->Close();

	return TRUE;
}

void CCaseHistoryDlg::OnChangeColumnSortFinishedCasehistoryDetailsList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending) 
{
	try { // (c.haag 2010-01-04 15:52) - PLID 36596 - Needs a try/catch
		//what we have to do here is make it so the Add row is at the bottom
		
		//find out what row number the add row is
		long nRowNumber = m_dlCaseHistoryDetails->FindByColumn(0, (long)MAXLONG, -1, FALSE);
		// (c.haag 2010-01-04 15:52) - PLID 36596 - If nRowNumber is -1, that means the "Add" row doesn't
		// exist yet because a requery is still in progress. Because it doesn't exist, there's nothing more to do here.
		if (-1 != nRowNumber) {
			NXDATALISTLib::IRowSettingsPtr pRow;
			//get the row for later use
			pRow = m_dlCaseHistoryDetails->GetRow(nRowNumber);	

			//remove the row
			m_dlCaseHistoryDetails->RemoveRow(nRowNumber);

			//add the row again at the bottom of the list
			long nRowCount = m_dlCaseHistoryDetails->GetRowCount();
			m_dlCaseHistoryDetails->InsertRow(pRow, nRowCount);
		}
		else {
			// The "Add" row doesn't exist. Nothing to do.
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CCaseHistoryDlg::PromptInventoryLocationChanged()
{
	try {

		// (j.jones 2007-02-21 11:21) - PLID 24846 - instead of returning immediately
		// when on an uncompleted case, the various portions of this function now are
		// separately dependent on being completed. The "Billable Product" check
		// now fires whether the case is completed or not.
		
		//we do not need to prompt for product items on uncompleted cases
		if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {

			// (j.jones 2005-11-16 14:32) - remove all serialized items and re-prompt to enter them again
			for(int j=0;j<m_dlCaseHistoryDetails->GetRowCount()-1;j++)
				m_dlCaseHistoryDetails->PutValue(j,chdlcCHDProductItemListID,(long)-1);

			for(int i=0;i<m_aryCaseHistoryDetailProductItems.GetSize();i++) {
				CaseHistoryDetailProductItemList *delCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
				
				for(int q=0;q<delCHDPI->ProductItemAry.GetSize();q++) {
					delCHDPI->ProductItemAry.GetAt(q)->SaveStatus = CHDPI_DELETE;
				}
			}

			PromptForCaseHistoryDetailProductItems(FALSE);

		}

		//now serial numbered and expirable products are taken care of, so just warn if any others are out of stock

		CStringArray strInvalidArray;
		CStringArray strOutOfStockArray;
		CArray<double, double> aryOutOfStockOnHandQty;
		CArray<double, double> aryOutOfStockAllocatedQty;
		
		for(int i=0;i<m_dlCaseHistoryDetails->GetRowCount();i++) {
			long nServiceID = -1;
			if((ECaseHistoryDetailItemType)VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemType), -1) == chditProduct) {
				nServiceID = VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemID), -1);
			}

			if(nServiceID == -1)
				continue;

			// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID WHERE ProductT.ID = {INT}",nServiceID);
			if(!rs->eof) {

				CString strName = AdoFldString(rs, "Name","");

				BOOL bBillable = VarBool(m_dlCaseHistoryDetails->GetValue(i,chdlcBillable),FALSE);
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//BOOL bPayToPractice = VarBool(m_dlCaseHistoryDetails->GetValue(i,chdlcPayToPractice),FALSE);

				if(bBillable /*&& bPayToPractice*/) {
					// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 Billable "
						"FROM ProductLocationInfoT "
						"WHERE Billable = 1 AND ProductID = {INT} AND LocationID = {INT}", nServiceID, m_nCurLocationID);
					if(rs->eof) {
						//not billable for this location
						BOOL bFound = FALSE;
						for(int j=0;j<strInvalidArray.GetSize() && !bFound;j++) {
							if(strInvalidArray.GetAt(j) == strName) {
								bFound = TRUE;
							}
						}
						if(!bFound) {
							strInvalidArray.Add(strName);
						}
					}
				}

				//we do not need to check stock on uncompleted cases
				if(IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {

					double dblQuantity = 0.0;
					double dblAllocated = 0.0;

					// (j.jones 2007-12-18 11:25) - PLID 28037 - CalcAmtOnHand changed to return allocation information
					if(InvUtils::CalcAmtOnHand(nServiceID, m_nCurLocationID, dblQuantity, dblAllocated, -GetUnsavedQuantityCount(nServiceID), 0.0) && dblQuantity - dblAllocated <= 0.0) {
						//out of stock
						BOOL bFound = FALSE;
						for(int j=0;j<strOutOfStockArray.GetSize() && !bFound;j++) {
							if(strOutOfStockArray.GetAt(j) == strName)
								bFound = TRUE;
						}
						if(!bFound) {
							strOutOfStockArray.Add(strName);
							aryOutOfStockOnHandQty.Add(dblQuantity);
							aryOutOfStockAllocatedQty.Add(dblAllocated);
						}
					}
				}
			}
			rs->Close();
		}


		if(strInvalidArray.GetSize() > 0 || strOutOfStockArray.GetSize() > 0) {
			//okay, something is invalid, so warn the user

			CString strWarning, strInvalidWarning, strOutOfStockWarning;

			if(strInvalidArray.GetSize() > 0) {
				strInvalidWarning = "The following products are marked as billable to the practice on the Case History, but are not marked as billable for the new location:\n";
				for(int i=0;i<strInvalidArray.GetSize();i++) {
					CString strInvalid;
					strInvalid.Format("    %s\n",strInvalidArray.GetAt(i));
					strInvalidWarning += strInvalid;
				}
				strInvalidWarning += "\n";
			}

			if(strOutOfStockArray.GetSize() > 0) {
				strOutOfStockWarning = "The following products are out of stock at the new location:\n";
				for(int i=0;i<strOutOfStockArray.GetSize();i++) {
					CString strAllocatedWarning = "";
					if(aryOutOfStockAllocatedQty.GetAt(i) > 0.0) {
						strAllocatedWarning.Format(", %g allocated to patients", aryOutOfStockAllocatedQty.GetAt(i));
					}
					CString strOutOfStock;
					strOutOfStock.Format("    %s (%g in stock%s)\n",strOutOfStockArray.GetAt(i), aryOutOfStockOnHandQty.GetAt(i), strAllocatedWarning);
					strOutOfStockWarning += strOutOfStock;
				}
				strOutOfStockWarning.TrimRight("\n");
			}

			strWarning.Format("Changing the location of this case history has triggered the following warnings:\n\n%s%s",strInvalidWarning,strOutOfStockWarning);

			strWarning.TrimRight("\n");

			MessageBox(strWarning);
		}

	}NxCatchAll("Error validating inventory items for the new location.");
}

double CCaseHistoryDlg::GetUnsavedQuantityCount(long nServiceID)
{
	double dblCount = 0.0;
	
	for (int i=0;i<m_dlCaseHistoryDetails->GetRowCount();i++) {
		if((ECaseHistoryDetailItemType)VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemType), -1) == chditProduct
			&& VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcItemID),1) == nServiceID) /* Has same service ID */ {
			long nDetailID = VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcID),-1);
			if(nDetailID == MAXLONG)
				continue;
			if(nDetailID == -1 /* Is a new line item */) {	
				dblCount += VarDouble(m_dlCaseHistoryDetails->GetValue(i,chdlcQuantity),0.0);
			}
			else {
				//get the difference between the current amount and saved amount
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Quantity FROM CaseHistoryDetailsT WHERE ID = {INT}",nDetailID);
				if(!rs->eof) {
					double dblQty = AdoFldDouble(rs, "Quantity",1.0);
					dblCount += (VarDouble(m_dlCaseHistoryDetails->GetValue(i,chdlcQuantity),0.0) - dblQty);
				}
				rs->Close();
			}
		}
	}

	return dblCount;
}

void CCaseHistoryDlg::OnRequeryFinishedDischargeStatusCombo(short nFlags) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DischargeStatusCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "");
		pRow->PutValue(2, "<No Status Selected>");
		m_DischargeStatusCombo->AddRowBefore(pRow, m_DischargeStatusCombo->GetFirstRow());

	}NxCatchAll("CCaseHistoryDlg::OnRequeryFinishedDischargeStatusCombo");
}

void CCaseHistoryDlg::OnBtnEditDischargeStatus() 
{
	try {

		//save the current value

		long nCurID = -1;
		if(m_DischargeStatusCombo->GetCurSel() != NULL) {
			nCurID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(0), -1);
		}

		CEditDischargeStatusDlg dlg(this); // use the dialog as the parent
		dlg.DoModal();

		m_DischargeStatusCombo->Requery();

		//now set the old value, if it exists
		if(nCurID != -1) {
			m_DischargeStatusCombo->SetSelByColumn(0, nCurID);
		}

	}NxCatchAll("CCaseHistoryDlg::OnBtnEditDischargeStatus");
}

void CCaseHistoryDlg::ParseProcedures(OUT CArray<long,long> &arAddedIDs, OUT CStringArray &saAddedNames, OUT CArray<long,long> &arRemovedIDs, OUT CStringArray &saRemovedNames)
{
	//First, go through and find all the procedures that are in our current array that were not in the original array.
	// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
	int i = 0;
	for(i = 0; i < m_arProcedureIDs.GetSize(); i++) {
		bool bFound = false;
		for(int j = 0; j < m_arLastSavedProcedureIDs.GetSize() && !bFound; j++) {
			if(m_arProcedureIDs[i] == m_arLastSavedProcedureIDs[j]) bFound = true;
		}
		if(!bFound) {
			long nID = m_arProcedureIDs[i];
			//Look up the name.
			long nProcIndex = m_dlProcedures->FindByColumn(0, nID, 0, VARIANT_FALSE);
			if(nProcIndex != -1) {
				saAddedNames.Add(VarString(m_dlProcedures->GetValue(nProcIndex, 1)));
				arAddedIDs.Add(nID);
			}
			else {
				//This shouldn't really be possible, the list should have all procedures in it.
				// Nonetheless, we will go ahead and load the name from data.
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsProcName = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nID);
				if(!rsProcName->eof) {
					saAddedNames.Add(AdoFldString(rsProcName, "Name"));
					arAddedIDs.Add(nID);
				}
				else {
					//What?!  We have an invalid procedure ID!  We will NOT include it in our arrays.
					ASSERT(FALSE);
				}
			}
		}
	}

	//Second, go through and find all the procedures that were in our original array that are not in the current array.
	for(i = 0; i < m_arLastSavedProcedureIDs.GetSize(); i++) {
		bool bFound = false;
		for(int j = 0; j < m_arProcedureIDs.GetSize() && !bFound; j++) {
			if(m_arLastSavedProcedureIDs[i] == m_arProcedureIDs[j]) bFound = true;
		}
		if(!bFound) {
			long nID = m_arLastSavedProcedureIDs[i];
			//Look up the name.
			long nProcIndex = m_dlProcedures->FindByColumn(0, nID, 0, VARIANT_FALSE);
			if(nProcIndex != -1) {
				saRemovedNames.Add(VarString(m_dlProcedures->GetValue(nProcIndex, 1)));
				arRemovedIDs.Add(nID);
			}
			else {
				//This shouldn't really be possible, the list should have all procedures in it.
				// Nonetheless, we will go ahead and load the name from data.
				// (j.jones 2008-09-15 11:47) - PLID 31378 - parameterized
				_RecordsetPtr rsProcName = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nID);
				if(!rsProcName->eof) {
					saRemovedNames.Add(AdoFldString(rsProcName, "Name"));
					arRemovedIDs.Add(nID);
				}
				else {
					//What?!  We have an invalid procedure ID!  We will NOT include it in our arrays.
					ASSERT(FALSE);
				}
			}
		}
	}
}

// (a.walling 2007-02-26 16:22) - PLID 24451 - Set the control's time to the current time, prompt if data exists
void CCaseHistoryDlg::SetCurrentTime(IN long nControlID)
{
	// control ID should point to an NxTime control

	NXTIMELib::_DNxTimePtr nxt = BindNxTimeCtrl(this, nControlID);
	if (nxt) {
		if (nxt->GetStatus() == 1) { // valid time
			// there is already a valid time in there!
			COleDateTime dt = GetDlgItemTime(nControlID);
			if (IDYES == MessageBox(FormatString("Are you sure you want to overwrite the time %s with the current time?",
				FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime, false)),
				"Practice", MB_YESNO | MB_ICONQUESTION)) {
				nxt->SetDateTime((DATE)StripSeconds(COleDateTime::GetCurrentTime()));
			} else {
				return;
			}
		} else {
			nxt->SetDateTime((DATE)StripSeconds(COleDateTime::GetCurrentTime()));
		}
	}

	m_bIsModified = TRUE;
}

void CCaseHistoryDlg::OnAutotimeIn23() 
{
	SetCurrentTime(IDC_23HOURROOM_IN_EDIT);
}

void CCaseHistoryDlg::OnAutotimeInAnes() 
{
	SetCurrentTime(IDC_ANESTHESIA_START_EDIT);
	OnKillFocusAnesthesiaStartEdit();
}

void CCaseHistoryDlg::OnAutotimeInFac() 
{
	SetCurrentTime(IDC_FACILITY_START_EDIT);	
	OnKillFocusFacilityStartEdit();
}

void CCaseHistoryDlg::OnAutotimeInOproom() 
{
	SetCurrentTime(IDC_OPROOM_IN_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeInPreop() 
{
	SetCurrentTime(IDC_PREOP_IN_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeInRecovery() 
{
	SetCurrentTime(IDC_RECOVERY_IN_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeInSurgeon() 
{
	SetCurrentTime(IDC_SURGEON_IN_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeOut23() 
{
	SetCurrentTime(IDC_23HOURROOM_OUT_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeOutAnes() 
{
	SetCurrentTime(IDC_ANESTHESIA_STOP_EDIT);
	OnKillFocusAnesthesiaStopEdit();
}

void CCaseHistoryDlg::OnAutotimeOutFac() 
{
	SetCurrentTime(IDC_FACILITY_STOP_EDIT);	
	OnKillFocusFacilityStopEdit();
}

void CCaseHistoryDlg::OnAutotimeOutOproom() 
{
	SetCurrentTime(IDC_OPROOM_OUT_EDIT);
}

void CCaseHistoryDlg::OnAutotimeOutPreop() 
{
	SetCurrentTime(IDC_PREOP_OUT_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeOutRecovery() 
{
	SetCurrentTime(IDC_RECOVERY_OUT_EDIT);	
}

void CCaseHistoryDlg::OnAutotimeOutSurgeon() 
{
	SetCurrentTime(IDC_SURGEON_OUT_EDIT);	
}

// (j.jones 2008-05-21 16:20) - PLID 29223 - we validate anesthesia and 
// facility dates independently, as the checks will only occur if we have
// anesthesia or facility fee items
BOOL CCaseHistoryDlg::ValidateAnesthesiaDates()
{
	//verify that the anesthesia times are valid for anesthesia line items

	COleDateTime dtAnesthStart, dtAnesthEnd;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtAnesthStart = StripSeconds(m_nxtAnesthStart->GetDateTime());
	}
	else {
		dtAnesthStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtAnesthEnd = StripSeconds(m_nxtAnesthEnd->GetDateTime());
	}
	else {
		dtAnesthEnd.SetStatus(COleDateTime::invalid);
	}

	//first see if they entered one date and not the other
	
	if(dtAnesthStart.GetStatus() != COleDateTime::invalid
		&& dtAnesthEnd.GetStatus() == COleDateTime::invalid) {

		AfxMessageBox("The 'Anesthesia Start Time' is filled in, but the corresponding 'Anesthesia End Time' is not.\n"
			"You have at least one anesthesia item on this case history that requires anesthesia times to calculate its value, so you must ensure that both dates are either filled in, or are blank.");
		return FALSE;
	}

	if(dtAnesthStart.GetStatus() == COleDateTime::invalid
		&& dtAnesthEnd.GetStatus() != COleDateTime::invalid) {

		AfxMessageBox("The 'Anesthesia End Time' is filled in, but the corresponding 'Anesthesia Start Time' is not.\n"
			"You have at least one anesthesia item on this case history that requires anesthesia times to calculate its value, so you must ensure that both dates are either filled in, or are blank.");
		return FALSE;
	}

	//now see if the dates match the total minutes

	if(dtAnesthStart.GetStatus() != COleDateTime::invalid
		&& dtAnesthEnd.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtAnesthStart,dtoTime),FormatDateTimeForSql(dtAnesthEnd,dtoTime),
			FormatDateTimeForSql(dtAnesthStart,dtoTime),FormatDateTimeForSql(dtAnesthEnd,dtoTime),
			FormatDateTimeForSql(dtAnesthStart,dtoTime),FormatDateTimeForSql(dtAnesthEnd,dtoTime));
		long nTotalCalcMinutes = AdoFldLong(rs, "Minutes",0);*/

		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalCalcMinutes = MinuteSpan(dtAnesthStart, dtAnesthEnd);

		long nTotalEnteredMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES);

		if(nTotalCalcMinutes != nTotalEnteredMinutes) {
			CString str;
			str.Format("Your Anesthesia Start and End times span %li minutes, but you have manually entered a total Anesthesia time of %li minutes.\n"
				"You have at least one anesthesia item on this case history that requires anesthesia times to calculate its value, so you must correct this discrepancy before saving.",
				nTotalCalcMinutes,nTotalEnteredMinutes);
			AfxMessageBox(str);
			return FALSE;
		}
	}

	long nAnesthMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES);

	CString strAnesthMinutes;

	GetDlgItemText(IDC_EDIT_TOTAL_CASE_ANESTH_MINUTES, strAnesthMinutes);

	if (!strAnesthMinutes.IsEmpty()) {
		if (nAnesthMinutes == 0) {
			AfxMessageBox("You cannot save zero 'Total Anesthesia Minutes', becayse you have at least one anesthesia item on this case history that requires anesthesia times to calculate its value.\n"
				"Please review your times, or clear the minutes, before saving.");
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CCaseHistoryDlg::ValidateFacilityDates()
{
	//verify that the facility times are valid for facility fee line items

	COleDateTime dtFacilityStart, dtFacilityEnd;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtFacilityStart = StripSeconds(m_nxtFacilityStart->GetDateTime());
	}
	else {
		dtFacilityStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtFacilityEnd = StripSeconds(m_nxtFacilityEnd->GetDateTime());
	}
	else {
		dtFacilityEnd.SetStatus(COleDateTime::invalid);
	}

	//first see if they entered one date and not the other
	
	if(dtFacilityStart.GetStatus() != COleDateTime::invalid
		&& dtFacilityEnd.GetStatus() == COleDateTime::invalid) {

		AfxMessageBox("The 'Facility Start Time' is filled in, but the corresponding 'Facility End Time' is not.\n"
			"You have at least one facility fee item on this case history that requires facility times to calculate its value, so you must ensure that both dates are either filled in, or are blank.");
		return FALSE;
	}

	if(dtFacilityStart.GetStatus() == COleDateTime::invalid
		&& dtFacilityEnd.GetStatus() != COleDateTime::invalid) {

		AfxMessageBox("The 'Facility End Time' is filled in, but the corresponding 'Facility Start Time' is not.\n"
			"You have at least one facility fee item on this case history that requires facility times to calculate its value, so you must ensure that both dates are either filled in, or are blank.");
		return FALSE;
	}

	if(dtFacilityStart.GetStatus() != COleDateTime::invalid
		&& dtFacilityEnd.GetStatus() != COleDateTime::invalid) {

		/*_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtFacilityStart,dtoTime),FormatDateTimeForSql(dtFacilityEnd,dtoTime),
			FormatDateTimeForSql(dtFacilityStart,dtoTime),FormatDateTimeForSql(dtFacilityEnd,dtoTime),
			FormatDateTimeForSql(dtFacilityStart,dtoTime),FormatDateTimeForSql(dtFacilityEnd,dtoTime));
		long nTotalCalcMinutes = AdoFldLong(rs, "Minutes",0);*/

		// (a.walling 2007-02-27 09:49) - PLID 24944 - Don't use sql to get the difference between two times.
		long nTotalCalcMinutes = MinuteSpan(dtFacilityStart, dtFacilityEnd);

		long nTotalEnteredMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES);

		if(nTotalCalcMinutes != nTotalEnteredMinutes) {
			CString str;
			str.Format("Your Facility Start and End times span %li minutes, but you have manually entered a total Facility time of %li minutes.\n"
				"You have at least one facility fee item on this case history that requires facility times to calculate its value, so you must correct this discrepancy before saving.",
				nTotalCalcMinutes,nTotalEnteredMinutes);
			AfxMessageBox(str);
			return FALSE;
		}
	}

	long nFacilityMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES);

	CString strFacilityMinutes;

	GetDlgItemText(IDC_EDIT_TOTAL_CASE_FACILITY_MINUTES, strFacilityMinutes);

	if (!strFacilityMinutes.IsEmpty()) {
		if (nFacilityMinutes == 0) {
			AfxMessageBox("You cannot save zero 'Total Facility Minutes', because you have at least one facility fee item on this case history that requires facility times to calculate its value.\n"
				"Please review your times, or clear the minutes, before saving.");
			return FALSE;
		}
	}

	return TRUE;
}

// (a.walling 2007-02-27 14:02) - PLID 24944 - Return the span between the two times in minutes
long CCaseHistoryDlg::MinuteSpan(IN COleDateTime dtStart, IN COleDateTime dtEnd)
{
	if (dtEnd <= dtStart) {
		dtEnd += COleDateTimeSpan(1, 0, 0, 0); // increase to next day
	}

	return long((dtEnd - dtStart).GetTotalMinutes());
}

// (j.jones 2008-02-26 13:52) - PLID 29108 - added ability to create allocations from the case
void CCaseHistoryDlg::OnBtnCreateAllocation() 
{
	try {

		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//shouldn't have been able to click the button
			ASSERT(FALSE);
			return;
		}

		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
			return;
		}

		//do we already have an allocation selected?
		if(m_InvAllocationCombo->GetCurSel() != NULL) {
			long nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
			if(nAllocationID != -1) {
				//an allocation is already selected in the list, so we should warn them
				//that creating a new allocation will not replace the existing one
				if(IDNO == MessageBox("There is already an inventory allocation linked to this case history.\n"
					"Creating a new inventory allocation will not link to this case history unless the existing allocation is unselected.\n\n"
					"Are you sure you wish to create a new allocation?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					return;
				}
			}
		}

		//get everything now from the dialog, as we are about to destroy it
		CString strDescription = m_strName;
		long nPatientID = m_nPersonID;
		long nApptID = m_nAppointmentID;
		long nCaseHistoryID = m_nCaseHistoryID;

		long nLocationID = -1;
		if(!ValidateCurLocationID(nLocationID)) {
			return;
		}

		long nProviderID = -1;
		CString strProviderName;
		if(!ValidateCurProviderID(nProviderID, strProviderName)) {
			return;
		}

		//first save the Case History, call SaveAndClose which,
		//if successful, will cleanly close the case history
		//dialog prior to opening up the allocation dialog
		if(!SaveAndClose()) {
			return;
		}

		//nothing after SaveAndClose() should try to access the dialog
		{
			CWaitCursor pWait;

			CInvPatientAllocationDlg dlg(GetActiveWindow());
			dlg.m_nDefaultPatientID = nPatientID;
			dlg.m_nDefaultAppointmentID = nApptID;
			dlg.m_nDefaultLocationID = nLocationID;
			dlg.m_nDefaultCaseHistoryID = nCaseHistoryID;
			// (j.jones 2008-03-05 12:46) - PLID 29201 - allocations can have providers, so let's send ours
			dlg.m_nDefaultProviderID = nProviderID;
			dlg.m_strDefaultCaseHistoryName = strDescription;
			dlg.DoModal();

			//we don't care if they saved or cancelled the allocation,
			//the case history dialog will already be closed at this point
		}

	}NxCatchAll("Error in CCaseHistoryDlg::OnBtnCreateAllocation");
}

// (j.jones 2008-02-27 16:34) - PLID 29126 - if we have a linked allocation,
// and it is uncompleted, try to complete it, and if we don't have one
// then search for one, try to link one, and complete that
BOOL CCaseHistoryDlg::TryCompleteInvAllocation()
{
	InvUtils::AllocationMasterInfo *pInfo = NULL;

	try {

		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//should never have been called
			ASSERT(FALSE);
			return FALSE;
		}

		long nAllocationID = -1;
		InvUtils::InventoryAllocationStatus iasStatus = InvUtils::iasActive;

		//see if we have an allocation already selected
		{
			NXDATALIST2Lib::IRowSettingsPtr pAllocRow = m_InvAllocationCombo->GetCurSel();
			if(pAllocRow) {
				nAllocationID = VarLong(pAllocRow->GetValue(iaccID), -1);
				iasStatus = (InvUtils::InventoryAllocationStatus)VarLong(pAllocRow->GetValue(iaccStatus), InvUtils::iasActive);				
			}
		}

		if(nAllocationID == -1) {
			//we don't have one linked, so see if we can find an allocation that fits the case at all

			//since we can have several products, not just one, find an allocation that
			//matches the most types of products, that is not linked to a case history

			CString strProductIDs;

			long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
			while (p) {
				NXDATALISTLib::IRowSettingsPtr pRow;
				// Get the row object and move to the next enum
				{
					LPDISPATCH lpDisp = NULL;
					m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
					pRow = lpDisp;
					lpDisp->Release();
				}
				long nDetailID = VarLong(pRow->GetValue(chdlcID));
				if (nDetailID != MAXLONG) {

					// Use the row object (guaranteed by the datalist to exist) to generate the update statement
					ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
					if(nItemType == chditProduct) {

						long nProductID = VarLong(pRow->GetValue(chdlcItemID));
						// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice, we check always now
						//BOOL bPayToPractice = VarBool(pRow->GetValue(chdlcPayToPractice),FALSE);

						//if(bPayToPractice) {
							if(!strProductIDs.IsEmpty()) {
								strProductIDs += ",";
							}
							strProductIDs += AsString(nProductID);
						//}
					}
				}
			}

			if(!strProductIDs.IsEmpty()) {

				//we have products, so now search for relevant allocations that
				//are not already linked to another case history
				//(or this one, incase it is linked in data but they had unselected it)
				
				//TES 7/18/2008 - PLID 29478 - Count "To Be Ordered" details as active.
				CString strSql;
				strSql.Format("SELECT ID, Status, InputDate, Count(ProductID) AS CountOfProducts "
					"FROM ("
					"	SELECT PatientInvAllocationsT.ID, PatientInvAllocationsT.Status, "
					"	PatientInvAllocationsT.InputDate, PatientInvAllocationDetailsT.ProductID "
					"	FROM PatientInvAllocationsT "
					"	INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"	INNER JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
					"	WHERE ((PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status IN (%li,%li) ) "
					"		OR (PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li)) "
					"	AND PatientInvAllocationDetailsT.ProductID IN (%s) "
					"	AND PatientInvAllocationsT.PatientID = %li "
					"	AND PatientInvAllocationsT.LocationID = %li "
					"	AND PatientInvAllocationsT.ID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT WHERE CaseHistoryID <> %li) "
					"	GROUP BY PatientInvAllocationsT.ID, PatientInvAllocationsT.Status, "
					"	PatientInvAllocationsT.InputDate, PatientInvAllocationDetailsT.ProductID "
					") AS AllocationsQ "
					"GROUP BY ID, Status, InputDate "
					"ORDER BY Count(ProductID) DESC, InputDate ASC ",
					InvUtils::iasActive, InvUtils::iadsActive, InvUtils::iadsOrder,
					InvUtils::iasCompleted, InvUtils::iadsUsed,
					strProductIDs,
					m_nPersonID, m_nCurLocationID, m_nCaseHistoryID); //m_nCaseHistoryID is still ok if it is -1
				
				//can't parameterize the above query because of the IN clause, but we can
				//at least send the result as a param query
				_RecordsetPtr rs = CreateParamRecordset(strSql);
				if(!rs->eof) {
					//If we have a result, it means we have some allocation for this patient,
					//that is not linked to a different case history, that matches at least one
					//product in this case history. If multiple exist, the first record will
					//be the allocation that has the most products in common with this case.

					long nRecordsReturned = rs->GetRecordCount();

					COleDateTime dtInput = AdoFldDateTime(rs, "InputDate");

					CString strWarning;
					if(nRecordsReturned == 1) {
						strWarning.Format("There is no inventory allocation linked to this case history, but this patient does have an "
							"allocation from %s that has similar products.\n\n"
							"Do you wish to link this allocation to this case history?", FormatDateTimeForInterface(dtInput, NULL, dtoDate));
					}
					else {
						strWarning.Format("There is no inventory allocation linked to this case history, but this patient does have an "
							"allocation from %s that has similar products.\n\n"
							"This patient also has other allocations using similar products. You may wish to review the allocations "
							"for this patient and decide which one, if any, should be linked to this case history.\n\n"
							"Do you wish to link the %s allocation to this case history?", FormatDateTimeForInterface(dtInput, NULL, dtoDate), FormatDateTimeForInterface(dtInput, NULL, dtoDate));
					}

					if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						//they don't want to use the allocation, do they still wish to complete the case?
						if(IDNO == MessageBox("Do you still wish to complete this case history without linking to an inventory allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
							return FALSE;
						}
					}
					else {
						//they do wish to link, so let's use this allocation now

						nAllocationID = AdoFldLong(rs, "ID");
						iasStatus = (InvUtils::InventoryAllocationStatus)AdoFldLong(rs, "Status");						

						NXDATALIST2Lib::IRowSettingsPtr pAllocationRow = m_InvAllocationCombo->SetSelByColumn(iaccID, nAllocationID);
						//this allocation should exist in the list, if not, requery,
						//and if it fails, throw an exception
						if(pAllocationRow == NULL) {
							m_InvAllocationCombo->Requery();
							pAllocationRow = m_InvAllocationCombo->SetSelByColumn(iaccID, nAllocationID);
							if(pAllocationRow == NULL) {
								ThrowNxException("Could not find allocation in list!");
							}
						}
						// (j.jones 2008-03-10 14:40) - PLID 29233 - requery the allocation contents list
						CString strContentsWhere;
						strContentsWhere.Format("AllocationID = %li", nAllocationID);
						m_InvAllocationContentsList->PutWhereClause((LPCTSTR)strContentsWhere);
						m_InvAllocationContentsList->Requery();
					}
				}
				rs->Close();
			}
		}

		if(nAllocationID != -1) {

			// (j.jones 2008-03-06 09:26) - PLID 29203 - changed the logic to load the allocation
			// info outside the usage dialog, because after the dialog closes we are going to
			// access the data again
			pInfo = NULL;
			InvUtils::PopulateAllocationInfo(nAllocationID, pInfo, FALSE);

			if(pInfo == NULL) {
				ThrowNxException("Tried to load a nonexistent allocation!");
			}
			
			if(iasStatus == InvUtils::iasActive) {
				//we have a linked allocation that is not completed, so try to complete it

				CInvAllocationUsageDlg dlg(this);
				dlg.SetAllocationInfo(pInfo);

				//force the allocation to be completed
				dlg.m_bForceCompletion = TRUE;

				//let the allocation dialog save the changes
				dlg.m_bSaveToData = TRUE;

				//tell the dialog it's being called by the case history
				dlg.m_bIsCompletedByCaseHistory = TRUE;

				if(dlg.DoModal() == IDOK) {
					//we're still going to use the object, so don't let the dlg free its memory
					dlg.m_bFreeInfoObject = FALSE;

					//requery the allocation list to reflect the new status and completion date
					m_InvAllocationCombo->Requery();

					if(m_InvAllocationCombo->SetSelByColumn(iaccID, nAllocationID) == NULL) {
						//should be impossible
						ASSERT(FALSE);
					}

					// (j.jones 2008-03-10 14:40) - PLID 29233 - requery the allocation contents list
					CString strContentsWhere;
					strContentsWhere.Format("AllocationID = %li", nAllocationID);
					m_InvAllocationContentsList->PutWhereClause((LPCTSTR)strContentsWhere);
					m_InvAllocationContentsList->Requery();
				}
				else {					
					//free the memory object
					if(pInfo) {
						InvUtils::FreeAllocationMasterInfoObject(pInfo);
					}

					//they cancelled, so do not complete the case history					
					return FALSE;
				}
			}
			
			if(pInfo) {

				// (j.jones 2008-03-07 12:50) - PLID 29231 - after the allocation is completed, but before
				// adding allocation products to the case, check and see if any products that are on both
				// the case and the allocation have different quantities, if so then warn, and give the
				// option to cancel completing the case history
				if(!CheckWarnAllocationQuantityMismatch(pInfo)) {
					//free the memory object
					if(pInfo) {
						InvUtils::FreeAllocationMasterInfoObject(pInfo);
					}

					//they cancelled, so do not complete the case history					
					return FALSE;
				}

				// (j.jones 2008-03-06 09:22) - PLID 29203 - after it is completed, see if any products
				// on the allocation do not exist on the case, and prompt to use them on the case
				CheckAddProductsFromAllocation(pInfo);

				//now free the memory
				InvUtils::FreeAllocationMasterInfoObject(pInfo);
			}
		}

		return TRUE;

	}NxCatchAllCall("Error in CCaseHistoryDlg::TryCompleteInvAllocation",
		//free the memory object
		if(pInfo) {
			InvUtils::FreeAllocationMasterInfoObject(pInfo);
		}
	);

	return FALSE;
}

// (j.jones 2008-03-07 12:50) - PLID 29231 - compares products in the allocation to the
// case history, will warn if quantities don't match, return FALSE if they wish to cancel
// completing the case history
BOOL CCaseHistoryDlg::CheckWarnAllocationQuantityMismatch(InvUtils::AllocationMasterInfo *pInfo)
{
	//intentionally no error handling so the caller will catch the exception
	
	CArray<int, int> aryAllocProductIDs;	
	CMap<int, int, CString, CString> mapProductNamesToIDs;
	CMap<int, int, double, double> mapAllocProductQtyToIDs;
	
	//first we have to summarize the products on the allocation	
	int i=0;
	for(i=0;i<pInfo->paryAllocationDetailInfo.GetSize();i++) {

		InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)(pInfo->paryAllocationDetailInfo.GetAt(i));
		if(pDetail && pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
			//add to our list
			BOOL bFound = FALSE;
			int j=0;
			for(j=0;j<aryAllocProductIDs.GetSize() && !bFound; j++) {
				if((long)aryAllocProductIDs.GetAt(j) == pDetail->nProductID) {
					bFound = TRUE;

					//update the quantity
					double dblQuantity = 0;
					mapAllocProductQtyToIDs.Lookup(pDetail->nProductID, dblQuantity);
					dblQuantity += pDetail->dblCurQuantity;
					mapAllocProductQtyToIDs.SetAt(pDetail->nProductID, dblQuantity);
				}
			}
			if(!bFound) {
				//add all of the product information
				aryAllocProductIDs.Add(pDetail->nProductID);
				mapProductNamesToIDs.SetAt(pDetail->nProductID, pDetail->strProductName);
				mapAllocProductQtyToIDs.SetAt(pDetail->nProductID, pDetail->dblCurQuantity);
			}
		}
	}

	CArray<int, int> aryMatchingProductIDs;
	CMap<int, int, double, double> mapCaseProductQtyToIDs;

	//now with that information summarized, search the case history
	for(i=0;i<aryAllocProductIDs.GetSize();i++) {
		
		long nProductID = (long)aryAllocProductIDs.GetAt(i);

		//see if this product is on the case history

		BOOL bFound = FALSE;
		long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
		while (p && !bFound) {
			NXDATALISTLib::IRowSettingsPtr pRow;
			// Get the row object and move to the next enum
			{
				LPDISPATCH lpDisp = NULL;
				m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
			}
			long nDetailID = VarLong(pRow->GetValue(chdlcID));
			if (nDetailID != MAXLONG) {

				// Use the row object (guaranteed by the datalist to exist) to generate the update statement
				ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
				if(nItemType == chditProduct
					&& VarLong(pRow->GetValue(chdlcItemID)) == nProductID) {
					
					//the product is on our case
					bFound = TRUE;

					//update the quantity in the case quantity map
					double dblQuantity = 0;
					mapCaseProductQtyToIDs.Lookup(nProductID, dblQuantity);
					dblQuantity += VarDouble(pRow->GetValue(chdlcQuantity), 1.0);
					mapCaseProductQtyToIDs.SetAt(nProductID, dblQuantity);
				}
			}
		}
		
		if(bFound) {
			//the product is on our case history, so add it to our matching list

			//we already summarized the product list so we won't hit this more than once per ID
			aryMatchingProductIDs.Add(nProductID);
		}
	}

	//return TRUE if no matching products, which means
	//this function doesn't have to do anything
	if(aryMatchingProductIDs.GetSize() == 0) {
		return TRUE;
	}

	//now we must check our list of matching product IDs and compare quantities
	CString strProductsToWarn;
	long nCountMismatched = 0;
	for(i=0;i<aryMatchingProductIDs.GetSize();i++) {

		long nProductID = aryMatchingProductIDs.GetAt(i);

		double dblAllocTotal = 0.0;		
		mapAllocProductQtyToIDs.Lookup(nProductID, dblAllocTotal);
		double dblCaseTotal = 0.0;
		mapCaseProductQtyToIDs.Lookup(nProductID, dblCaseTotal);

		//now, are the totals different?
		if(dblAllocTotal != dblCaseTotal) {

			//they are different! we will have to warn the user,
			//so track all the information we have

			nCountMismatched++;

			if(!strProductsToWarn.IsEmpty()) {
				strProductsToWarn += "\n";
			}
			CString strName;
			mapProductNamesToIDs.Lookup(nProductID, strName);

			CString str;
			str.Format("%s (Case History lists %g, Allocation lists %g)", strName, dblCaseTotal, dblAllocTotal);

			strProductsToWarn += str;
		}
	}

	//return TRUE if all matching products have the same quantities
	if(nCountMismatched == 0) {
		return TRUE;
	}

	//now warn the user, but we better have a list of products
	ASSERT(!strProductsToWarn.IsEmpty());

	CString strWarning;
	strWarning.Format("The following product%s exist%s on the linked inventory allocation and the case history but do not have the same quantity in each record:\n\n"
		"%s\n\n"
		"If you continue, the higher value for each product will be deducted from stock.\n\n"
		"Are you sure you wish to complete this case history?",
		nCountMismatched == 1 ? "" : "s", nCountMismatched == 1 ? "s" : "",
		strProductsToWarn);

	if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
		//they don't want to complete the case history, so return FALSE
		return FALSE;
	}

	//they must be ok with this, so continue normally
	return TRUE;
}

// (j.jones 2008-03-06 09:56) - PLID 29203 - compares products in the allocation to the
// case history, will ask to add any that don't exist in the allocation
void CCaseHistoryDlg::CheckAddProductsFromAllocation(InvUtils::AllocationMasterInfo *pInfo)
{
	//intentionally no error handling so the caller will catch the exception

	CArray<int, int> aryAllocProductIDs;
	CMap<int, int, CString, CString> mapProductNamesToIDs;
	CMap<int, int, double, double> mapProductQtyToIDs;
	int i=0;
	for(i=0;i<pInfo->paryAllocationDetailInfo.GetSize();i++) {

		InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)(pInfo->paryAllocationDetailInfo.GetAt(i));
		if(pDetail && pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
			//see if this product is on the case history

			BOOL bFound = FALSE;
			long p = m_dlCaseHistoryDetails->GetFirstRowEnum();
			while (p && !bFound) {
				NXDATALISTLib::IRowSettingsPtr pRow;
				// Get the row object and move to the next enum
				{
					LPDISPATCH lpDisp = NULL;
					m_dlCaseHistoryDetails->GetNextRowEnum(&p, &lpDisp);
					pRow = lpDisp;
					lpDisp->Release();
				}
				long nDetailID = VarLong(pRow->GetValue(chdlcID));
				if (nDetailID != MAXLONG) {

					// Use the row object (guaranteed by the datalist to exist) to generate the update statement
					ECaseHistoryDetailItemType nItemType = (ECaseHistoryDetailItemType)VarLong(pRow->GetValue(chdlcItemType));
					if(nItemType == chditProduct
						&& VarLong(pRow->GetValue(chdlcItemID)) == pDetail->nProductID) {
						
						//the product is on our case
						bFound = TRUE;
					}
				}
			}
			
			if(!bFound) {
				//the product does not exist in the case history,
				//so add it to our list if it isn't already in it
				bFound = FALSE;
				int j=0;
				for(j=0;j<aryAllocProductIDs.GetSize() && !bFound; j++) {
					if((long)aryAllocProductIDs.GetAt(j) == pDetail->nProductID) {
						bFound = TRUE;

						//update the quantity
						double dblQuantity = 0;
						mapProductQtyToIDs.Lookup(pDetail->nProductID, dblQuantity);
						dblQuantity += pDetail->dblCurQuantity;
						mapProductQtyToIDs.SetAt(pDetail->nProductID, dblQuantity);
					}
				}
				if(!bFound) {
					//add all of the product information
					aryAllocProductIDs.Add(pDetail->nProductID);
					mapProductNamesToIDs.SetAt(pDetail->nProductID, pDetail->strProductName);
					mapProductQtyToIDs.SetAt(pDetail->nProductID, pDetail->dblCurQuantity);
				}
			}
		}
	}

	//return if empty
	if(aryAllocProductIDs.GetSize() == 0) {
		return;
	}

	//alright, now if aryAllocProductIDs is not empty, we have
	//products on the allocation that do not exist on the case history

	//prompt the user with the first 8 examples of products and ask if they wish to add them
	CString strProductNames;
	CString strProductIDs;
	for(i=0;i<aryAllocProductIDs.GetSize() && i<8;i++) {

		long nProductID = aryAllocProductIDs.GetAt(i);

		if(!strProductIDs.IsEmpty()) {
			strProductIDs += ",";
		}
		strProductIDs += AsString(nProductID);

		//stop adding names if we already have 8
		if(i<8) {
			if(!strProductNames.IsEmpty()) {
				strProductNames += "\n";
			}
			CString strName;
			mapProductNamesToIDs.Lookup(nProductID, strName);
			strProductNames += strName;
		}
	}
	if(aryAllocProductIDs.GetSize()>8) {
		strProductNames += "\n<More>";
	}
	CString strWarning;
	strWarning.Format("The following product%s exist%s on the linked inventory allocation but do not exist on the case history:\n\n"
		"%s\n\nWould you like to add these products to the case history?",
		aryAllocProductIDs.GetSize() == 1 ? "" : "s", aryAllocProductIDs.GetSize() == 1 ? "s" : "",
		strProductNames);

	if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
		//they don't want to add products, so return
		return;
	}

	//to add the products to the case history, we need their price and cost,
	//so run one recordset and loop through that, can't be fully parameterized
	//since we have an IN clause
	CString strSql;

	// (j.jones 2008-05-20 17:46) - PLID 29249 - added IsSerializedProduct
	// (j.jones 2008-09-15 11:27) - PLID 31375 - changed to include the ProductLocationInfoT.Billable status
	strSql.Format("SELECT ServiceT.ID, Active, Price, "
		"(CASE WHEN UseUU = 0 THEN LastCost ELSE (CASE WHEN Conversion = 0 THEN 0 ELSE Round(Convert(money, LastCost / Conversion),2) END) END) AS LastCost, "
		"Convert(bit, CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 THEN 1 ELSE 0 END) AS IsSerializedProduct, "
		"ProductLocationInfoQ.Billable AS BillableForLocation "
		"FROM ServiceT "
		"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN (SELECT Billable, ProductID FROM ProductLocationInfoT WHERE LocationID = %li) "
		"	AS ProductLocationInfoQ ON ProductT.ID = ProductLocationInfoQ.ProductID "
		"WHERE ServiceT.ID IN (%s)", m_nCurLocationID, strProductIDs);
	CString strInactiveProducts;
	_RecordsetPtr rs = CreateParamRecordset(strSql);
	while(!rs->eof) {
		long nProductID = AdoFldLong(rs, "ID");
		CString strProductName;
		double dblQuantity = 1.0;
		mapProductNamesToIDs.Lookup(nProductID, strProductName);
		mapProductQtyToIDs.Lookup(nProductID, dblQuantity);

		//is it active?
		if(!AdoFldBool(rs, "Active")) {
			if(!strInactiveProducts.IsEmpty()) {
				strInactiveProducts += "\n";
			}
			strInactiveProducts += strProductName;
		}
		else {		
			//add this product to the case history
			COleCurrency cyAmount = AdoFldCurrency(rs, "Price");
			COleCurrency cyCost = AdoFldCurrency(rs, "LastCost");

			//TES 7/16/2008 - PLID 27983 - Pass in FALSE for bIsLinkedToProducts, which is only ever true for CPT Codes.
			// (j.jones 2008-09-15 11:32) - PLID 31375 - passed BillableForLocation to AddDetail
			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			AddDetail(nProductID, chditProduct, strProductName, dblQuantity, cyAmount, cyCost, true,
				GetRemotePropertyInt("ChargeAllowQtyIncrementCaseHistory",0,0,"<None>",TRUE) ? true : false,
				AdoFldBool(rs, "IsSerializedProduct", FALSE),
				FALSE, rs->Fields->Item["BillableForLocation"]->Value);
		}

		rs->MoveNext();
	}
	rs->Close();

	if(!strInactiveProducts.IsEmpty()) {
		strWarning.Format("The following product(s) are inactive and were not added to the case history:\n\n%s",strInactiveProducts);
		AfxMessageBox(strWarning);
	}
}

// (j.jones 2008-02-28 16:44) - PLID 29126 - moved the 'no row' creation to the RequeryFinished
// because I changed the logic to potentially add this row in multiple places in code
void CCaseHistoryDlg::OnRequeryFinishedCaseAllocationList(short nFlags) 
{
	try {

		//add the "none selected" row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_InvAllocationCombo->GetNewRow();
		pRow->PutValue(iaccID, (long)-1);
		pRow->PutValue(iaccInputDate, g_cvarNull);
		pRow->PutValue(iaccStatus, g_cvarNull);
		pRow->PutValue(iaccStatusName, g_cvarNull);
		pRow->PutValue(iaccCompletionDate, g_cvarNull);			
		pRow->PutValue(iaccFirstProductName, _bstr_t("< No Allocation Selected >"));
		pRow->PutValue(iaccTotalQuantity, g_cvarNull);
		m_InvAllocationCombo->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error in CCaseHistoryDlg::OnRequeryFinishedCaseAllocationList");	
}

void CCaseHistoryDlg::OnSelChangingCaseHistoryLocationCombo(long FAR* nNewSel) 
{
	try {

		if(*nNewSel == m_dlLocations->CurSel) {
			//do nothing
			return;
		}

		// (j.jones 2008-02-28 17:41) - PLID 29102 - disallow changing the location
		// if there is a linked allocation and the case history is completed and
		// using the same serialized items

		long nAllocationID = -1;
		if(m_InvAllocationCombo->GetCurSel() != NULL) {
			nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
		}

		//call CanUnlinkAllocation to confirm whether we have to force uncompleting the case
		if(nAllocationID != -1 && IsDlgButtonChecked(IDC_COMPLETED_CHECK)
			&& !CanUnlinkAllocation()) {
			//disallow changing locations
			*nNewSel = m_dlLocations->CurSel;

			//explain why
			AfxMessageBox("Changing locations would unlink the attached inventory allocation from this case history, but the allocation cannot be unlinked while the case history is completed, and the two are sharing the same serial numbered / exp. date items.\n"
				 "You must uncheck the 'Completed' box before you can change locations or unlink this allocation.");
		}

	}NxCatchAll("Error in CCaseHistoryDlg::OnSelChangingCaseHistoryLocationCombo");
}

// (j.jones 2008-02-29 08:49) - PLID 29102 - lets us know if we're allowed to unlink the allocation
BOOL CCaseHistoryDlg::CanUnlinkAllocation()
{
	try {

		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//if no license, don't let this function stop anything
			return TRUE;
		}

		long nAllocationID = -1;						
		if(m_InvAllocationCombo->GetCurSel() != NULL) {
			nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
		}

		if(nAllocationID != -1 && IsDlgButtonChecked(IDC_COMPLETED_CHECK)) {

			//we have a completed case and a linked allocation, so now we have to see if
			//the two are sharing serial numbers

			CString strProductItemIDs;

			for (int i=0; i<m_aryCaseHistoryDetailProductItems.GetSize(); i++) {
				CaseHistoryDetailProductItemList *addCHDPI = m_aryCaseHistoryDetailProductItems.GetAt(i);
				for(int q=0;q<addCHDPI->ProductItemAry.GetSize();q++) {
					if(addCHDPI->ProductItemAry.GetAt(q)->SaveStatus != CHDPI_DELETE) {
						
						if(!strProductItemIDs.IsEmpty()) {
							strProductItemIDs += ",";
						}
						strProductItemIDs += AsString(addCHDPI->ProductItemAry.GetAt(q)->ProductItemID);
					}
				}
			}

			if(!strProductItemIDs.IsEmpty()) {
				//see if the allocation also uses any of these product items
				if(ReturnsRecords("SELECT ID FROM PatientInvAllocationsT WHERE ID = %li "
					"AND ID IN (SELECT AllocationID FROM PatientInvAllocationDetailsT "
					"	WHERE Status IN (%li, %li) AND ProductItemID IN (%s))",
					nAllocationID, InvUtils::iadsActive, InvUtils::iadsUsed, _Q(strProductItemIDs))) {

					//they are indeed sharing at least one product item, and thus cannot
					//be unlinked until the case is uncompleted
					return FALSE;
				}
			}

			//if we get here, we're good to go
			return TRUE;
		}

	}NxCatchAll("Error in CCaseHistoryDlg::CanUnlinkAllocation");

	return FALSE;
}

void CCaseHistoryDlg::OnSelChangingCaseAllocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {

		// (j.jones 2008-02-29 09:00) - PLID 29102 - disallow changing the allocation if 
		// the case history is completed and using the same serialized items

		long nAllocationID = -1;						
		if(m_InvAllocationCombo->GetCurSel() != NULL) {
			nAllocationID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_InvAllocationCombo->GetCurSel())->GetValue(iaccID), -1);
		}

		//call CanUnlinkAllocation to confirm whether we have to force uncompleting the case
		if(nAllocationID != -1 && IsDlgButtonChecked(IDC_COMPLETED_CHECK)
			&& !CanUnlinkAllocation()) {

			//disallow this
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			
			//explain why
			AfxMessageBox("The attached inventory allocation cannot be unlinked from this case history while the case history is completed, and the two are sharing the same serial numbered / exp. date items.\n"
				"You must uncheck the 'Completed' box before you can unlink this allocation.");
		}

	}NxCatchAll("Error in CCaseHistoryDlg::OnSelChangingCaseAllocationList");
}

void CCaseHistoryDlg::OnSelChosenCaseAllocationList(LPDISPATCH lpRow) 
{
	try {

		// (j.jones 2008-03-10 15:40) - PLID 29233 - requery the allocation contents list
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_InvAllocationContentsList->Clear();
			return;
		}

		long nAllocationID = VarLong(pRow->GetValue(iaccID),-1);
		if(nAllocationID == -1) {
			m_InvAllocationContentsList->Clear();
		}
		else {
			CString strContentsWhere;
			strContentsWhere.Format("AllocationID = %li", nAllocationID);
			m_InvAllocationContentsList->PutWhereClause((LPCTSTR)strContentsWhere);
			m_InvAllocationContentsList->Requery();
		}

	}NxCatchAll("Error in CCaseHistoryDlg::OnSelChosenCaseAllocationList");
}


// (j.gruber 2008-07-07 17:13) - PLID 15807 - added anesthesia type, preop diag and postop diag to the case history
void CCaseHistoryDlg::OnSelChangingCaseHistoryAnesType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll("Error in CCaseHistoryDlg::OnSelChangingCaseHistoryAnesType");
	
}

void CCaseHistoryDlg::OnRequeryFinishedCaseHistoryAnesType(short nFlags) 
{
	try {

		//add the no diag code row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAnesType->GetNewRow();

		if (pRow) {
			pRow->PutValue(0, "");
			
			m_pAnesType->AddRowSorted(pRow, NULL);

			if(!m_bIsNew || m_nCaseHistoryID != -1) {
				pRow = m_pAnesType->SetSelByColumn(0, _variant_t(m_strLastAnesType));
			}
			else {
				LoadAnesthesia();
			}
		}
			

	}NxCatchAll("Error in CCaseHistoryDlg::OnRequeryFinishedCaseHistoryAnesType");
	
}

//TES 7/16/2008 - PLID 27983 - Goes through all details, and for any linked service codes, removes them and prompts the user
// for which products to replace them with.
void CCaseHistoryDlg::ReplaceLinkedServices()
{
	for(int i=0;i<m_dlCaseHistoryDetails->GetRowCount()-1;i++) {
		if((ECaseHistoryDetailItemType)VarLong(m_dlCaseHistoryDetails->GetValue(i,chdlcItemType),0) == chditCptCode) {
			if(VarBool(m_dlCaseHistoryDetails->GetValue(i, chdlcIsLinkedToProducts),FALSE)) {
				//TES 7/16/2008 - PLID 27983 - OK, this is one we need to replace.
				CSelectLinkedProductsDlg dlg(this);
				//TES 7/15/2008 - PLID 27983 - Give the dialog the information it needs.
				dlg.m_nCptID = VarLong(m_dlCaseHistoryDetails->GetValue(i, chdlcItemID));
				dlg.m_dDefaultQty = VarDouble(m_dlCaseHistoryDetails->GetValue(i, chdlcQuantity),1.0);
				dlg.m_nLocationID = m_nCurLocationID;
				dlg.m_bIsCaseHistory = true;
				if(IDOK == dlg.DoModal()) {
					//TES 7/15/2008 - PLID 27983 - Add all the products they added to the end of our list.
					for(int nProduct = 0; nProduct < dlg.m_arProductsToBill.GetSize(); nProduct++) {
						ProductToBill ptb = dlg.m_arProductsToBill[nProduct];
						long nQuantity = ptb.arProductItemIDs.GetSize();
						if(nQuantity == 0) nQuantity = ptb.nQty;
						_RecordsetPtr rsProductInfo = CreateParamRecordset("SELECT ServiceT.Name, ServiceT.Price, "
							"ProductT.LastCost, ProductLocationInfoT.Billable, convert(bit,CASE WHEN "
							"ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1 THEN 1 ELSE 0 END) "
							"AS IsSerialized "
							"FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN "
							"ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
							"WHERE ServiceT.ID = {INT} AND ProductLocationInfoT.LocationID = {INT}",
							ptb.nProductID, m_nCurLocationID);
						// (j.jones 2008-09-15 11:32) - PLID 31375 - passed ProductLocationInfoT.Billable to AddDetail
						// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
						AddDetail(ptb.nProductID, chditProduct, AdoFldString(rsProductInfo, "Name"), 
							(double)nQuantity, AdoFldCurrency(rsProductInfo, "Price", COleCurrency(0,0)),
							AdoFldCurrency(rsProductInfo, "LastCost", COleCurrency(0,0)), 
							AdoFldBool(rsProductInfo, "Billable", FALSE)?true:false, 
							GetRemotePropertyInt("ChargeAllowQtyIncrementCaseHistory",0,0,"<None>",TRUE) ? true : false,
							AdoFldBool(rsProductInfo, "IsSerialized", FALSE), FALSE,
							rsProductInfo->Fields->Item["Billable"]->Value);
					}
				}
				//TES 7/16/2008 - PLID 27983 - Now, remove the row, and adjust our index accordingly.
				RemoveDetail(i);
				i--;
			}
		}
	}
}

//(c.copits 2010-09-17) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns a recordset of all matching UPC codes. This recordset may contain
// the IDs of both products and service codes.

_RecordsetPtr CCaseHistoryDlg::GetBestUPCProduct(_variant_t barcode) 
{
	_RecordsetPtr rsServiceID;

	try {
		rsServiceID = CreateParamRecordset("SELECT ServiceT.ID, Name, Price, "
			"CASE WHEN ProductT.ID Is Null THEN 0 ELSE "
			"ProductT.LastCostPerUU END AS LastCost, "
			"Convert(bit, CASE WHEN ProductT.ID Is Not Null AND (HasSerialNum = 1 OR HasExpDate = 1) THEN 1 ELSE 0 END) AS IsSerializedProduct, "
			"CASE WHEN ProductT.ID Is Null THEN 0 ELSE 1 END AS Type, "
			"convert(bit,CASE WHEN ServiceT.ID IN (SELECT CptID FROM ServiceToProductLinkT) THEN 1 ELSE 0 END) AS IsLinkedToProducts, "
			"ProductLocationInfoQ.Billable AS BillableForLocation "
			"FROM ServiceT "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN (SELECT Billable, ProductID FROM ProductLocationInfoT WHERE LocationID = {INT}) "
			"	AS ProductLocationInfoQ ON ProductT.ID = ProductLocationInfoQ.ProductID "
			"WHERE Barcode = {STRING}", m_nCurLocationID, VarString(barcode));
	} NxCatchAll(__FUNCTION__);

	return rsServiceID;
}

// (a.wilson 02/19/2014) PLID 60774 - attempt to add preop from search.
void CCaseHistoryDlg::SelChosenPreopDiagnosisSearch(LPDISPATCH lpRow)
{
	try {
		if (lpRow) {
			CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);
			long nDiag9ID = results.m_ICD9.m_nDiagCodesID, nDiag10ID = results.m_ICD10.m_nDiagCodesID;

			if (nDiag9ID == -1 && nDiag10ID == -1) {
				return;
			}	
			//if we have 4 rows, do not add a 5th
			NXDATALIST2Lib::IRowSettingsPtr pInsertRow = NULL;
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPreOpDiagnosisList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
				//check if row is empty
				if (VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1) == -1 && VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1) == -1) {
					pInsertRow = pRow;
					break;
				//prevent duplicates
				} else if (nDiag9ID == VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1) && nDiag10ID == VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1)) {
					AfxMessageBox("You cannot add duplicate Pre-Op diagnosis codes to a case history.");
					return;
				}
			}
			//if row is null then all our rows are filled with data. therefore, all 4 records and full.
			if(!pInsertRow) {
				AfxMessageBox("No more than four Pre-Op diagnosis codes can be added to a case history.");
				return;
			}

			//add the row ICD-9
			if(nDiag9ID != -1) {
				pInsertRow->PutValue(dclcDiagICD9CodeID, nDiag9ID);
				pInsertRow->PutValue(dclcDiagICD9Code, _bstr_t(results.m_ICD9.m_strCode));
				pInsertRow->PutValue(dclcDiagICD9Desc, _bstr_t(results.m_ICD9.m_strDescription));
			} else {
				pInsertRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD9Code, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
			}
			//ICD-10
			if(nDiag10ID != -1) {
				pInsertRow->PutValue(dclcDiagICD10CodeID, nDiag10ID);
				pInsertRow->PutValue(dclcDiagICD10Code, _bstr_t(results.m_ICD10.m_strCode));
				pInsertRow->PutValue(dclcDiagICD10Desc, _bstr_t(results.m_ICD10.m_strDescription));
			} else {
				pInsertRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD10Code, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
			}
			m_bPreOpDiagCodesChanged = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 02/19/2014) PLID 60774 - attempt to add postop from search.
void CCaseHistoryDlg::SelChosenPostopDiagnosisSearch(LPDISPATCH lpRow)
{
	try {
		if (lpRow) {
			CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);
			long nDiag9ID = results.m_ICD9.m_nDiagCodesID, nDiag10ID = results.m_ICD10.m_nDiagCodesID;
			
			if (nDiag9ID == -1 && nDiag10ID == -1) {
				return;
			}	
			//if we have 4 rows, do not add a 5th
			NXDATALIST2Lib::IRowSettingsPtr pInsertRow = NULL;
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPostOpDiagnosisList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
				//check if row is empty
				if (VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1) == -1 && VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1) == -1) {
					pInsertRow = pRow;
					break;
				//prevent duplicates
				} else if (nDiag9ID == VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1) && nDiag10ID == VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1)) {
					AfxMessageBox("You cannot add duplicate Post-Op diagnosis codes to a case history.");
					return;
				}
			}
			//if row is null then all our rows are filled with data. therefore, all 4 records and full.
			if(!pInsertRow) {
				AfxMessageBox("No more than four Post-Op diagnosis codes can be added to a case history.");
				return;
			}

			//add the row ICD-9
			if(nDiag9ID != -1) {
				pInsertRow->PutValue(dclcDiagICD9CodeID, nDiag9ID);
				pInsertRow->PutValue(dclcDiagICD9Code, _bstr_t(results.m_ICD9.m_strCode));
				pInsertRow->PutValue(dclcDiagICD9Desc, _bstr_t(results.m_ICD9.m_strDescription));
			} else {
				pInsertRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD9Code, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
			}
			//ICD-10
			if(nDiag10ID != -1) {
				pInsertRow->PutValue(dclcDiagICD10CodeID, nDiag10ID);
				pInsertRow->PutValue(dclcDiagICD10Code, _bstr_t(results.m_ICD10.m_strCode));
				pInsertRow->PutValue(dclcDiagICD10Desc, _bstr_t(results.m_ICD10.m_strDescription));
			} else {
				pInsertRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD10Code, g_cvarNull);
				pInsertRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
			}
			m_bPostOpDiagCodesChanged = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-21 15:50) - PLID 60774
void CCaseHistoryDlg::UpdateDiagnosisListColumnSizes(bool bUpdatePreOp /* = true */, bool bUpdatePostOp /* = true */)
{
	try {
		if (bUpdatePreOp) {
			DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_pPreOpDiagnosisList, dclcDiagICD9Code, dclcDiagICD10Code,
				50, 50, "", "", dclcDiagICD9Desc, dclcDiagICD10Desc, true, true, true);
		}
		if (bUpdatePostOp) {
			DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_pPostOpDiagnosisList, dclcDiagICD9Code, dclcDiagICD10Code,
				50, 50, "", "", dclcDiagICD9Desc, dclcDiagICD10Desc, true, true, true);
		}

	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-24 14:10) - PLID 60862 - remove diag code from preop list.
void CCaseHistoryDlg::RButtonDownPreopDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}
		//set focus on row.
		m_pPreOpDiagnosisList->PutCurSel(pRow);

		//if no codes are selected, do nothing
		long nDiagID9 = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
		long nDiagID10 = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);
		if(nDiagID9 == -1 && nDiagID10 == -1) {
			return;
		}

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "&Remove Diagnosis Code");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if (nRet == 1) { //Remove Diagnosis Code
			//remove the code from the list.
			m_pPreOpDiagnosisList->RemoveRow(pRow);
			m_bPreOpDiagCodesChanged = true;
			//update the list indexes.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPreOpDiagnosisList->GetFirstRow();
			for (int i = 1; i <= 4; i++) {
				if (pRow) {
					pRow->PutValue(dclcIndex, i);
				} else {
					pRow = m_pPreOpDiagnosisList->GetNewRow();
					pRow->PutValue(dclcIndex, i);
					pRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
					pRow->PutValue(dclcDiagICD9Code, g_cvarNull);
					pRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
					pRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
					pRow->PutValue(dclcDiagICD10Code, g_cvarNull);
					pRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
					m_pPreOpDiagnosisList->AddRowAtEnd(pRow, NULL);
				}
				pRow = pRow->GetNextRow();
			}
			//update column sizes if extra codes were removed that should be hid based on preference.
			UpdateDiagnosisListColumnSizes(true, false);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-24 14:10) - PLID 60862 - remove diag code from postop list.
void CCaseHistoryDlg::RButtonDownPostopDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}
		//set focus on row.
		m_pPostOpDiagnosisList->PutCurSel(pRow);

		//if no codes are selected, do nothing
		long nDiagID9 = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
		long nDiagID10 = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);
		if(nDiagID9 == -1 && nDiagID10 == -1) {
			return;
		}

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "&Remove Diagnosis Code");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if (nRet == 1) { //Remove Diagnosis Code
			//remove the code from the list.
			m_pPostOpDiagnosisList->RemoveRow(pRow);
			m_bPostOpDiagCodesChanged = true;
			//update the list indexes.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPostOpDiagnosisList->GetFirstRow();
			for (int i = 1; i <= 4; i++) {
				if (pRow) {
					pRow->PutValue(dclcIndex, i);
				} else {
					pRow = m_pPostOpDiagnosisList->GetNewRow();
					pRow->PutValue(dclcIndex, i);
					pRow->PutValue(dclcDiagICD9CodeID, g_cvarNull);
					pRow->PutValue(dclcDiagICD9Code, g_cvarNull);
					pRow->PutValue(dclcDiagICD9Desc, g_cvarNull);
					pRow->PutValue(dclcDiagICD10CodeID, g_cvarNull);
					pRow->PutValue(dclcDiagICD10Code, g_cvarNull);
					pRow->PutValue(dclcDiagICD10Desc, g_cvarNull);
					m_pPostOpDiagnosisList->AddRowAtEnd(pRow, NULL);
				}
				pRow = pRow->GetNextRow();
			}
			//update column sizes if extra codes were removed that should be hid based on preference.
			UpdateDiagnosisListColumnSizes(false, true);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-25 10:49) - PLID 60774 - generates two strings containing a delimited list of the current diagnosis ids.
//	Empty rows and codes will be assigned NULL in string.
//	ID Order will be 9, 10, 9, 10, 9, 10, 9, 10
void CCaseHistoryDlg::GenerateDiagnosisIDListStrings(OUT CString &strPreOpIDs, OUT CString &strPostOpIDs)
{
	//Pre-Op List
	for (NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_pPreOpDiagnosisList->GetFirstRow(); pDiagRow; pDiagRow = pDiagRow->GetNextRow()) {
		strPreOpIDs += FormatString("%s, ", AsString(VarVar(pDiagRow->GetValue(dclcDiagICD9CodeID), _bstr_t("NULL"))));
		strPreOpIDs += FormatString("%s, ", AsString(VarVar(pDiagRow->GetValue(dclcDiagICD10CodeID), _bstr_t("NULL"))));
	}
	//Post-Op List
	for (NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_pPostOpDiagnosisList->GetFirstRow(); pDiagRow; pDiagRow = pDiagRow->GetNextRow()) {
		strPostOpIDs += FormatString("%s, ", AsString(VarVar(pDiagRow->GetValue(dclcDiagICD9CodeID), _bstr_t("NULL"))));
		strPostOpIDs += FormatString("%s, ", AsString(VarVar(pDiagRow->GetValue(dclcDiagICD10CodeID), _bstr_t("NULL"))));
	}
	strPreOpIDs.TrimRight(", ");
	strPostOpIDs.TrimRight(", ");
}

// (a.wilson 2014-02-25 10:49) - PLID 61034 - generates two strings containing a delimited list of the current diagnosis codes.
//	Empty rows and codes will be assigned NULL in string.
//	Code Order will be 9, 10, 9, 10, 9, 10, 9, 10
void CCaseHistoryDlg::GenerateDiagnosisCodeListStrings(OUT CString &strPreOpCodes, OUT CString &strPostOpCodes)
{
	//Pre-Op List
	for (NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_pPreOpDiagnosisList->GetFirstRow(); pDiagRow; pDiagRow = pDiagRow->GetNextRow()) {
		strPreOpCodes += FormatString("%s, ", VarString(pDiagRow->GetValue(dclcDiagICD9Code), "NULL"));
		strPreOpCodes += FormatString("%s, ", VarString(pDiagRow->GetValue(dclcDiagICD10Code), "NULL"));
	}
	//Post-Op List
	for (NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_pPostOpDiagnosisList->GetFirstRow(); pDiagRow; pDiagRow = pDiagRow->GetNextRow()) {
		strPostOpCodes += FormatString("%s, ", VarString(pDiagRow->GetValue(dclcDiagICD9Code), "NULL"));
		strPostOpCodes += FormatString("%s, ", VarString(pDiagRow->GetValue(dclcDiagICD10Code), "NULL"));
	}
	strPreOpCodes.TrimRight(", ");
	strPostOpCodes.TrimRight(", ");
}

// (a.wilson 2014-02-25 15:10) - PLID 61034 - audit the changes to the diagnosis lists if the case history was changed.
void CCaseHistoryDlg::AuditDiagnosisListChanges(long & nAuditTransactionID)
{
	if (!m_bPreOpDiagCodesChanged && !m_bPostOpDiagCodesChanged) {
		return;
	}
	CString strPreOpCodes, strPostOpCodes;
	GenerateDiagnosisCodeListStrings(strPreOpCodes, strPostOpCodes);	

	_RecordsetPtr prs = CreateParamRecordset(
	"SELECT \r\n"
	"c.IndexNumber, c.DiagICD9CodeID, Dx9.CodeNumber AS DiagICD9Code, \r\n"
	"c.DiagICD10CodeID, Dx10.CodeNumber AS DiagICD10Code \r\n"
	"FROM CaseHistoryT \r\n"
	"CROSS APPLY \r\n"
	"( \r\n"
	"	SELECT 1, PreOpDx1, PreOpDx1ICD10 \r\n"
	"	UNION ALL SELECT 2,  PreOpDx2, PreOpDx2ICD10 \r\n"
	"	UNION ALL SELECT 3, PreOpDx3, PreOpDx3ICD10 \r\n"
	"	UNION ALL SELECT 4, PreOpDx4, PreOpDx4ICD10 \r\n"
	") c (IndexNumber, DiagICD9CodeID, DiagICD10CodeID) \r\n"
	"LEFT JOIN DiagCodes Dx9 ON c.DiagICD9CodeID = Dx9.ID \r\n"
	"LEFT JOIN DiagCodes Dx10 ON c.DiagICD10CodeID = Dx10.ID \r\n"
	"WHERE CaseHistoryT.ID = {INT} ORDER BY c.IndexNumber ASC \r\n"
	"SELECT \r\n"
	"c.IndexNumber, c.DiagICD9CodeID, Dx9.CodeNumber AS DiagICD9Code, \r\n"
	"c.DiagICD10CodeID, Dx10.CodeNumber AS DiagICD10Code \r\n"
	"FROM CaseHistoryT \r\n"
	"CROSS APPLY \r\n"
	"( \r\n"
	"	SELECT 1, PostOpDx1, PostOpDx1ICD10 \r\n"
	"	UNION ALL SELECT 2,  PostOpDx2, PostOpDx2ICD10 \r\n"
	"	UNION ALL SELECT 3, PostOpDx3, PostOpDx3ICD10 \r\n"
	"	UNION ALL SELECT 4, PostOpDx4, PostOpDx4ICD10 \r\n"
	") c (IndexNumber, DiagICD9CodeID, DiagICD10CodeID) \r\n"
	"LEFT JOIN DiagCodes Dx9 ON c.DiagICD9CodeID = Dx9.ID \r\n"
	"LEFT JOIN DiagCodes Dx10 ON c.DiagICD10CodeID = Dx10.ID \r\n"
	"WHERE CaseHistoryT.ID = {INT} ORDER BY c.IndexNumber ASC \r\n"
	, m_nCaseHistoryID, m_nCaseHistoryID);

	//Audit Pre-Op
	if (m_bPreOpDiagCodesChanged) {
		CString strOldPreOpCodes;

		for (; !prs->eof; prs->MoveNext()) {
			strOldPreOpCodes += FormatString("%s, ", AdoFldString(prs, "DiagICD9Code", "NULL"));
			strOldPreOpCodes += FormatString("%s, ", AdoFldString(prs, "DiagICD10Code", "NULL"));
		}
		strOldPreOpCodes.Replace("NULL, ", "");
		strOldPreOpCodes.TrimRight(", ");
		strPreOpCodes.Replace("NULL, ", "");
		strPreOpCodes.Replace("NULL", "");
		strPreOpCodes.TrimRight(", ");
		if (strOldPreOpCodes != strPreOpCodes) {
			if (nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryPreOpDiagCodes, m_nCaseHistoryID, 
				strOldPreOpCodes, strPreOpCodes, aepMedium, aetChanged);
		}
	}
	prs = prs->NextRecordset(NULL);
	//Audit Post-Op
	if (m_bPostOpDiagCodesChanged) {
		CString strOldPostOpCodes;

		for (; !prs->eof; prs->MoveNext()) {
			strOldPostOpCodes += FormatString("%s, ", AdoFldString(prs, "DiagICD9Code", "NULL"));
			strOldPostOpCodes += FormatString("%s, ", AdoFldString(prs, "DiagICD10Code", "NULL"));
		}
		strOldPostOpCodes.Replace("NULL, ", "");
		strOldPostOpCodes.TrimRight(", ");
		strPostOpCodes.Replace("NULL, ", "");
		strPostOpCodes.Replace("NULL", "");
		strPostOpCodes.TrimRight(", ");
		if (strOldPostOpCodes != strPostOpCodes) {
			if (nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditTransactionID, aeiCaseHistoryPostOpDiagCodes, m_nCaseHistoryID, 
				strOldPostOpCodes, strPostOpCodes, aepMedium, aetChanged);	
		}
	}
}