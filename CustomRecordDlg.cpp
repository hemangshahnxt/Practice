// CustomRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "nxmessagedef.h"
#include "CustomRecordDlg.h"
#include "CustomRecordItemDlg.h"
#include "CustomRecordSetupDlg.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "NxModalParentDlg.h"
#include "DateTimeUtils.h"
#include "EMRChartNote.h"
#include "EMRUtils.h"
#include "MedicationSelectDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "MergeEngine.h"
#include "LetterWriting.h"
#include "HistoryDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "GlobalFinancialUtils.h"
#include "CustomRecordSelectServiceDlg.h"
#include "GlobalReportUtils.h"
#include "TodoUtils.h"
#include "CorrectMedicationQuantitiesDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "BillingModuleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



extern CPracticeApp theApp;

#define ID_REMOVE_ICD9	34928
#define ID_REMOVE_CHARGE 34929
#define	ID_DELETE_PRESCRIPTION	34930

#define ID_LOAD_NEW_EMR	34931

#define CHARGE_COLUMN_ID			0
#define CHARGE_COLUMN_SERVICE_ID	1
#define CHARGE_COLUMN_SUB_CODE		2
#define CHARGE_COLUMN_DESCRIPTION	3
#define CHARGE_COLUMN_QTY			4
#define CHARGE_COLUMN_COST			5

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDlg dialog



// (j.armen 2013-05-08 13:14) - PLID 56603 - Constructor takes in extra info that was being set by all callers
CCustomRecordDlg::CCustomRecordDlg(CWnd* pParent, const long& nPatID, const long& nMasterID, CBillingModuleDlg* pBillingDlg)
	: CNxDialog(CCustomRecordDlg::IDD, pParent),
	m_EMRDetailDlg(*(new CCustomRecordDetailDlg(NULL))),
	m_nPatID(nPatID),
	m_nMasterID(nMasterID),
	m_pBillingDlg(pBillingDlg)
{
	m_bInitialized = FALSE;
	m_bIsSpawning = FALSE;
}

CCustomRecordDlg::~CCustomRecordDlg()
{
	if(!m_EMRDetailDlg) {
		delete (&m_EMRDetailDlg);
	}
}

void CCustomRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordDlg)
	DDX_Control(pDX, IDC_CHECK_PROCEDURE, m_btnProcedure);
	DDX_Control(pDX, IDC_CHECK_FU, m_btnFollowup);
	DDX_Control(pDX, IDC_PRINT_REPORT, m_btnPrintReport);
	DDX_Control(pDX, IDC_BTN_WRITE_PRESCRIPTIONS, m_btnWritePrescriptions);
	DDX_Control(pDX, IDC_BTN_MERGE, m_btnMerge);
	DDX_Control(pDX, IDC_BTN_CREATE_BILL, m_btnCreateBill);
	DDX_Control(pDX, IDC_BTN_ADD_MEDICATION, m_btnAddMedication);
	DDX_Control(pDX, IDC_BTN_ADD_CHARGE, m_btnAddCharge);
	DDX_Control(pDX, IDC_CLEAR_DETAILS, m_btnClearDetails);
	DDX_Control(pDX, IDC_ADD_EMR_ITEMS, m_btnAddEditItems);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_GENDER, m_strGender);
	DDX_Control(pDX, IDC_AGE, m_strAge);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_PATIENT_NAME, m_strPatientName);
	DDX_Control(pDX, IDC_DATE, m_dtPicker);
	DDX_Control(pDX, IDC_EDIT_EMR_NOTES, m_nxeditEditEmrNotes);
	DDX_Control(pDX, IDC_EDIT_FU_NUMBER, m_nxeditEditFuNumber);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CCustomRecordDlg, IDC_DATE, 3 /* CloseUp */, OnCloseUpDate, VTS_NONE)
//	ON_EVENT(CCustomRecordDlg, IDC_DATE, 2 /* Change */, OnChangeDate, VTS_NONE)
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

BEGIN_MESSAGE_MAP(CCustomRecordDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnChangeDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATE, OnCloseUpDate)
	ON_BN_CLICKED(IDC_PRINT, OnPrint)
	ON_BN_CLICKED(IDC_ADD_EMR_ITEMS, OnAddEmrItems)
	ON_BN_CLICKED(IDC_CLEAR_DETAILS, OnClearDetails)
	ON_BN_CLICKED(IDC_BTN_ADD_CHARGE, OnBtnAddCharge)
	ON_BN_CLICKED(IDC_BTN_CREATE_BILL, OnBtnCreateBill)
	ON_BN_CLICKED(IDC_BTN_ADD_MEDICATION, OnBtnAddMedication)
	ON_BN_CLICKED(IDC_BTN_WRITE_PRESCRIPTIONS, OnBtnWritePrescriptions)
	ON_BN_CLICKED(IDC_BTN_MERGE, OnBtnMerge)
	ON_BN_CLICKED(IDC_CHECK_FU, OnCheckFu)
	ON_BN_CLICKED(IDC_CHECK_PROCEDURE, OnCheckProcedure)
	ON_BN_CLICKED(IDC_PRINT_REPORT, OnPrintReport)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDlg message handlers

BEGIN_EVENTSINK_MAP(CCustomRecordDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCustomRecordDlg)
	ON_EVENT(CCustomRecordDlg, IDC_PROCEDURE_COMBO, 16 /* SelChosen */, OnSelChosenProcedureCombo, VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_EMR_LOCATIONS_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmrLocationsCombo, VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_DIAG_DROPDOWN, 16 /* SelChosen */, OnSelChosenDiagDropdown, VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_DIAGS, 6 /* RButtonDown */, OnRButtonDownDiags, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_BILL, 6 /* RButtonDown */, OnRButtonDownBill, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_BILL, 9 /* EditingFinishing */, OnEditingFinishingBill, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCustomRecordDlg, IDC_BILL, 10 /* EditingFinished */, OnEditingFinishedBill, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCustomRecordDlg, IDC_MEDICATIONS, 6 /* RButtonDown */, OnRButtonDownMedications, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordDlg, IDC_MEDICATIONS, 9 /* EditingFinishing */, OnEditingFinishingMedications, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCustomRecordDlg, IDC_MEDICATIONS, 10 /* EditingFinished */, OnEditingFinishedMedications, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCustomRecordDlg, IDC_EMR_PROVIDER_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmrProviderCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


BOOL CCustomRecordDlg::OnInitDialog() 
{
	CWaitCursor pWait;

	CNxDialog::OnInitDialog();

	try {
		
		m_btnPrintReport.AutoSet(NXB_PRINT_PREV);
		m_btnWritePrescriptions.AutoSet(NXB_MERGE);
		m_btnCreateBill.AutoSet(NXB_NEW);
		m_btnAddMedication.AutoSet(NXB_NEW);
		m_btnAddCharge.AutoSet(NXB_NEW);
		m_btnClearDetails.AutoSet(NXB_DELETE);
		m_btnAddEditItems.AutoSet(NXB_MODIFY);
		m_btnPrint.AutoSet(NXB_PRINT_PREV);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);

		m_dtPicker.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
		m_bdate.m_status = COleDateTime::invalid;
	
		m_ProcedureCombo = BindNxDataListCtrl(IDC_PROCEDURE_COMBO);
		m_ProviderCombo = BindNxDataListCtrl(IDC_EMR_PROVIDER_COMBO);
		m_LocationCombo = BindNxDataListCtrl(IDC_EMR_LOCATIONS_COMBO);
		m_DiagCodeCombo = BindNxDataListCtrl(IDC_DIAG_DROPDOWN);
		m_DiagCodeList = BindNxDataListCtrl(IDC_DIAGS,false);
		m_BillList = BindNxDataListCtrl(IDC_BILL,false);
		m_MedicationList = BindNxDataListCtrl(IDC_MEDICATIONS,false);
		m_TemplateCombo = BindNxDataListCtrl(IDC_TEMPLATE_LIST,false);
		m_FUTimeCombo = BindNxDataListCtrl(IDC_FU_INCREMENT,false);
		m_SchedProcedureCombo = BindNxDataListCtrl(IDC_PROCEDURE_SCH_LIST);

		//fill the follow-up list
		IRowSettingsPtr pRow = m_FUTimeCombo->GetRow(-1);
		pRow->PutValue(0,(long)fiDays);
		pRow->PutValue(1,_bstr_t(GetDisplayName(fiDays)));
		m_FUTimeCombo->AddRow(pRow);
		pRow = m_FUTimeCombo->GetRow(-1);
		pRow->PutValue(0,(long)fiWeeks);
		pRow->PutValue(1,_bstr_t(GetDisplayName(fiWeeks)));
		m_FUTimeCombo->AddRow(pRow);
		pRow = m_FUTimeCombo->GetRow(-1);
		pRow->PutValue(0,(long)fiMonths);
		pRow->PutValue(1,_bstr_t(GetDisplayName(fiMonths)));
		m_FUTimeCombo->AddRow(pRow);

		//prepare to show the EMR detail dialog
		CRect rect;
		//align the top
		GetDlgItem(IDC_BKG)->GetWindowRect(rect);
		ScreenToClient(rect);
		long top = rect.top;
		
		//align the left
		GetDlgItem(IDC_BKG3)->GetWindowRect(rect);
		ScreenToClient(rect);
		long left = rect.left;

		//now create the dialog
		m_EMRDetailDlg.Create(IDD_CUSTOM_RECORD_DETAIL_DLG,this);
		//align it
		m_EMRDetailDlg.GetWindowRect(rect);
		ScreenToClient(rect);
		m_EMRDetailDlg.MoveWindow(left,top,rect.Width(),rect.Height());
		//and show it
		m_EMRDetailDlg.BringWindowToTop();
		m_EMRDetailDlg.ShowWindow(SW_SHOW);

		CString last, first, middle, full;
		_variant_t var;
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT [Last], [First], Middle, Gender, BirthDate, MainPhysician, Location \r\n"
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = {INT}",
			m_nPatID);
		if(!rs->eof) {
			var = rs->Fields->Item["Last"]->Value;
			if(var.vt == VT_BSTR) {
				last = CString(var.bstrVal);
			}
			var = rs->Fields->Item["First"]->Value;
			if(var.vt == VT_BSTR) {
				first = CString(var.bstrVal);
			}
			var = rs->Fields->Item["Middle"]->Value;
			if(var.vt == VT_BSTR) {
				middle = CString(var.bstrVal);
			}						
			full = last + ", " + first + " " + middle;
			m_strPatientName.SetWindowText(full);

			m_strPatientName.SetFont(&theApp.m_subtitleFont);

			var = rs->Fields->Item["BirthDate"]->Value;
			if(var.vt==VT_DATE) {
				m_bdate = var.date;
			}

			/////////////////////////////////////////

			if(m_nMasterID == -1)
			{
				var = rs->Fields->Item["Gender"]->Value;
				if(var.vt==VT_UI1) {
					if(var.bVal==1)
						m_strGender.SetWindowText("Male");
					if(var.bVal==2)
						m_strGender.SetWindowText("Female");
				}

				if(m_bdate.m_status != COleDateTime::invalid)
				{
					COleDateTime dtNow = COleDateTime::GetCurrentTime();

					// Use a temporary value so that GetPatientAge doesn't write to our stored member variable
					COleDateTime dtBirthDateTemporary = m_bdate;
					// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
					//  validation should only be done when bdays are entered/changed
					// (z.manning 2010-01-13 10:57) - PLID 22672 - Age is now a string
					m_strAge.SetWindowText(GetPatientAgeOnDate(dtBirthDateTemporary, dtNow, TRUE));
				}

				var = rs->Fields->Item["MainPhysician"]->Value;
				if(var.vt==VT_I4) {
					if(m_ProviderCombo->TrySetSelByColumn(0,var) == -1) {
						//they may have an inactive provider
						_RecordsetPtr rsProv = CreateParamRecordset(
							"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
							"WHERE ID = (SELECT TOP 1 ProviderID FROM EmrProvidersT WHERE EmrID = {INT} AND Deleted = 0)", m_nMasterID);
						if(!rsProv->eof) {
							m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						}
						else 
							m_ProviderCombo->PutCurSel(-1);
					}
				}


				var = rs->Fields->Item["Location"]->Value;
				if(var.vt==VT_I4) {
					if(m_LocationCombo->TrySetSelByColumn(0,var) == -1) {
						//Must be inactive.
						_RecordsetPtr rsLocName = CreateParamRecordset(
							"SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM PersonT WHERE ID = {INT})",
							m_nPatID);
						if(!rsLocName->eof) {
							m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
						}
					}
				}
			}

			/////////////////////////////////////////
		}
		rs->Close();

		if(m_nMasterID != -1) {

			CString str;

			//load EMR master information
			// (c.haag 2008-12-29 15:05) - PLID 32579 - Parameterized
			rs = CreateParamRecordset("SELECT (SELECT TOP 1 ProviderID FROM EmrProvidersT WHERE EmrProvidersT.Deleted = 0 AND EmrProvidersT.EmrID = EmrMasterT.ID) AS ProviderID, LocationID, Date, PatientAge, PatientGender, AdditionalNotes, NeedsFollowup, "
				"FollowUpNumber, FollowUpUnit, NeedsProcedure, ApptProcedureID, "
				"(SELECT TOP 1 ProcedureID FROM EmrProcedureT WHERE EmrProcedureT.Deleted = 0 AND EmrProcedureT.EmrID = EmrMasterT.ID) AS ProcedureID "
				"FROM EMRMasterT WHERE ID = {INT}", m_nMasterID);
			if(!rs->eof) {
				var = rs->Fields->Item["ProcedureID"]->Value;
				if(var.vt==VT_I4) {
					// (c.haag 2008-12-29 14:57) - PLID 32579 - If the custom record procedure was inactivated at some point in
					// the past, the selection will fail. We must manually add the procedure to it.
					if (sriNoRow == m_ProcedureCombo->SetSelByColumn(0,var)) {
						_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", VarLong(var));
						if (!prs->eof) {
							IRowSettingsPtr pRow = m_ProcedureCombo->GetRow(-1);
							pRow->Value[0L] = var;
							pRow->Value[1L] = prs->Fields->Item["Name"]->Value;
							m_ProcedureCombo->AddRow(pRow);
							m_ProcedureCombo->Sort();
							m_ProcedureCombo->SetSelByColumn(0,var);
						} else {
							// The procedure was deleted? Nothing we can do about that.
						}
					}
					m_EMRDetailDlg.m_ProcedureID = var.lVal;
				}
				var = rs->Fields->Item["ProviderID"]->Value;
				if(var.vt==VT_I4) {
					if(m_ProviderCombo->TrySetSelByColumn(0,var) == -1) {
						//they may have an inactive provider
						_RecordsetPtr rsProv = CreateParamRecordset(
							"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
							"WHERE ID = (SELECT TOP 1 ProviderID FROM EmrProvidersT WHERE EmrID = {INT} AND Deleted = 0)",
							m_nMasterID);
						if(!rsProv->eof) {
							m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						}
						else 
							m_ProviderCombo->PutCurSel(-1);
					}
				}
				var = rs->Fields->Item["LocationID"]->Value;
				if(var.vt==VT_I4) {
					if(m_LocationCombo->TrySetSelByColumn(0,var) == -1) {
						//Must be inactive.
						_RecordsetPtr rsLocName = CreateParamRecordset(
							"SELECT Name FROM LocationsT \r\n"
							"WHERE ID = (SELECT LocationID FROM EMRMasterT WHERE ID = {INT})", m_nMasterID);
						if(!rsLocName->eof) {
							m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
						}
					}
				}
				var = rs->Fields->Item["Date"]->Value;
				if(var.vt==VT_DATE) {
					m_dtPicker.SetValue(var);
				}
				var = rs->Fields->Item["PatientAge"]->Value;
				if(var.vt==VT_I4) {
					str.Format("%li",var.lVal);
					m_strAge.SetWindowText(str);
				}

				//incase the patient's birthday has changed (rare), then the age will change on save,
				//so reflect the impending change on load
				LRESULT dummy = 0;
				OnChangeDate(NULL, &dummy);

				var = rs->Fields->Item["PatientGender"]->Value;
				if(var.vt==VT_UI1) {
					if(var.bVal==1)
						m_strGender.SetWindowText("Male");
					if(var.bVal==2)
						m_strGender.SetWindowText("Female");
				}

				CString strNotes = AdoFldString(rs, "AdditionalNotes");
				SetDlgItemText(IDC_EDIT_EMR_NOTES, strNotes);

				//Load the appointment stuff.
				BOOL bNeedsFollowUp = AdoFldBool(rs, "NeedsFollowUp");
				CheckDlgButton(IDC_CHECK_FU, bNeedsFollowUp);
				OnCheckFu();
				if(bNeedsFollowUp) {
					long nNumber = AdoFldLong(rs, "FollowUpNumber", -1);
					if(nNumber != -1) {
						SetDlgItemInt(IDC_EDIT_FU_NUMBER, nNumber);
					}
					m_FUTimeCombo->TrySetSelByColumn(0, AdoFldLong(rs, "FollowUpUnit", -1));
				}

				BOOL bNeedsProcedure = AdoFldBool(rs, "NeedsProcedure");
				CheckDlgButton(IDC_CHECK_PROCEDURE, bNeedsProcedure);
				OnCheckProcedure();
				if(bNeedsProcedure) {
					m_SchedProcedureCombo->TrySetSelByColumn(0, AdoFldLong(rs, "ApptProcedureID", -1));
				}
			}
			rs->Close();

			//load DiagCodes
			// (r.gonet 03/02/2014) - PLID 61131 - Removed support for ICD-10
			str.Format("EMRDiagCodesT.Deleted = 0 AND EMRDiagCodesT.EMRID = %li AND DiagCodes.ICD10 = 0", m_nMasterID);
			m_DiagCodeList->WhereClause = _bstr_t(str);
			m_DiagCodeList->Requery();
			m_DiagCodeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

			for(int i=0;i<m_DiagCodeList->GetRowCount();i++) {
				m_EMRDetailDlg.AddDiagCodeID(VarLong(m_DiagCodeList->GetValue(i,0)));
			}

			m_EMRDetailDlg.LoadInfoWhereClause();

			//load EMR details

			rs = CreateParamRecordset("SELECT ID FROM EMRDetailsT WHERE Deleted = 0 AND EMRID = {INT}", m_nMasterID);

			while(!rs->eof) {
				m_EMRDetailDlg.AddDetail(-1,rs->Fields->Item["ID"]->Value.lVal, m_nMasterID == -1);
				rs->MoveNext();
			}
			rs->Close();

			//Now, add the blank one at the end.
			m_EMRDetailDlg.TryAddBlankDetail();

			//load charges
			str.Format("EMRChargesT.Deleted = 0 AND EMRChargesT.EMRID = %li", m_nMasterID);
			m_BillList->WhereClause = _bstr_t(str);
			m_BillList->Requery();

			//load medications
			str.Format("PatientMedications.Deleted = 0 AND EmrMedicationsT.EmrID = %li AND EMRMedicationsT.Deleted = 0", m_nMasterID);
			m_MedicationList->WhereClause = _bstr_t(str);
			m_MedicationList->Requery();
			

		}
		else {
			//We have to requery the bill list, otherwise the embedded drop down will not get filled.
			m_BillList->WhereClause = _bstr_t("EMRChargesT.Deleted = 0 AND EMRChargesT.EMRID = -1");
			m_BillList->Requery();

			m_btnPrint.EnableWindow(FALSE);
			m_btnPrintReport.EnableWindow(FALSE);

			CheckDlgButton(IDC_CHECK_FU, BST_UNCHECKED);
			OnCheckFu();
			CheckDlgButton(IDC_CHECK_PROCEDURE, BST_UNCHECKED);
			OnCheckProcedure();
		}


	}NxCatchAll("Error in OnInitDialog");	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomRecordDlg::OnSelChosenProcedureCombo(long nRow) 
{

	//JJ - Here's the deal - when you choose a procedure the first time, it needs to add one blank detail,
	//containing items for that procedure. If you change the procedure, those items are invalid,
	//so they must be cleared out or you can't change the procedure. However, if you change the procedure
	//to the same one, nothing should happen.
	//Also, the only time there should be no details is when you're in a new EMR and have not chosen any procedure yet.
	//So if you clear items from an existing procedure, or change to a different procedure, a blank detail should
	//exist, and that detail must be filtered to the proper procedure.

	try {

		if(nRow == -1) {
			//if we already added details, attempt to set it back to this procedure
			//(if we haven't, this will set it to -1, which is fine)
			m_ProcedureCombo->SetSelByColumn(0,(long)m_EMRDetailDlg.m_ProcedureID);
			return;
		}

		long ProcedureID = m_ProcedureCombo->GetValue(nRow,0).lVal;

		//conveniently the EMR Detail Dlg stores the current procedure ID, which lets us compare
		//this is basically so we don't try to add required items again, or an empty item, 
		//in the event they somehow try to re-select the procedure that is already selected
		if(ProcedureID == m_EMRDetailDlg.m_ProcedureID)
			return;

		//if we have existing details, we will need to clear them, so warn them
		if(m_EMRDetailDlg.m_ProcedureID > 0 && m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize() > 0 &&
			IDNO == MessageBox("If you change this procedure, any record details associated with this procedure will be removed."
			"(Unless they are associated with the new procedure as well.)\n\n"
			"Are you sure you wish to change the procedure?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			m_ProcedureCombo->SetSelByColumn(0,((long)m_EMRDetailDlg.m_ProcedureID));
			return;
		}

		//this will automatically update the list, removing any invalid items
		//JJ - the bonus here is that if they change to a similar procedure that has the same items,
		//those items won't be deleted!
		m_EMRDetailDlg.m_ProcedureID = ProcedureID;
		m_EMRDetailDlg.RefreshAllItems();

		//process any actions such as adding a charge or diagnosis code
		ProcessProcedureActions(ProcedureID);

		/////////////////////////////////////////////

		CWaitCursor pWait;

		//if there is a blank line, remove it
		m_EMRDetailDlg.TryRemoveBlankDetail();

		_RecordsetPtr rs = CreateRecordset("SELECT EMRInfoT.* FROM ProcedureToEMRInfoT INNER JOIN EMRInfoT ON ProcedureToEMRInfoT.EMRInfoID = EMRInfoT.ID WHERE ProcedureID = %li AND Required = 1",ProcedureID);

		if(!rs->eof) {

			while(!rs->eof) {

				long InfoID = rs->Fields->Item["ID"]->Value.lVal;

				if(!m_EMRDetailDlg.IsInfoInList(InfoID)) {
					m_EMRDetailDlg.AddDetail(InfoID,-1);

					//JJ - don't do it here, it gets processed as the Info selection is set, deeper into the code
					//ProcessEMRInfoActions(InfoID);
				}			

				rs->MoveNext();
			}
		}

		rs->Close();

		//create an empty detail item, if we need one	
		m_EMRDetailDlg.TryAddBlankDetail();

		m_btnPrint.EnableWindow(TRUE);
		m_btnPrintReport.EnableWindow(TRUE);

	}NxCatchAll("Error loading required items.");
}

void CCustomRecordDlg::OnOK() 
{
	if(!Save())
		return;
		
	CDialog::OnOK();
}

void CCustomRecordDlg::OnCancel() 
{
	m_dwDeletedMedications.RemoveAll();

	CDialog::OnCancel();
}

BOOL CCustomRecordDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(wParam == ID_REMOVE_ICD9) {
		if(m_DiagCodeList->GetCurSel() != -1) {
			if(DeleteICD9(m_DiagCodeList->GetValue(m_DiagCodeList->GetCurSel(),0).lVal)) {
				m_DiagCodeList->RemoveRow(m_DiagCodeList->GetCurSel());
			}
		}	
	}
	else if(wParam == ID_REMOVE_CHARGE) {
		if(m_BillList->GetCurSel() != -1) {
			//for the time being, all charges are deleted and re-saved from scratch, so nothing else is necessary for now
			m_BillList->RemoveRow(m_BillList->GetCurSel());
		}
	}
	else if(wParam == ID_DELETE_PRESCRIPTION) {
		if(m_MedicationList->GetCurSel() != -1) {
			if (CheckCurrentUserPermissions(bioPatientMedication, sptDelete)) {

				long nMedicationID = m_MedicationList->GetValue(m_MedicationList->GetCurSel(),0).lVal;
				// (j.jones 2012-10-29 14:45) - PLID 53259 - Cannot delete if the prescription status is E-Prescribed.
				if (nMedicationID != -1
					&& ReturnsRecordsParam("SELECT TOP 1 SureScriptsMessagesT.ID "
						"FROM SureScriptsMessagesT "
						"WHERE PatientMedicationID = {INT} "
						"UNION SELECT TOP 1 PatientMedications.ID "
						"FROM PatientMedications "
						"WHERE PatientMedications.ID = {INT} AND PatientMedications.QueueStatus IN ({SQL})",
						nMedicationID, nMedicationID, GetERxStatusFilter())) {

					// (z.manning 2009-08-10 16:01) - PLID 35007 - Prevent deleting if e-prescribed
					// (b.savon 2013-09-23 07:31) - PLID 58486 - Changed the wording
					// (b.eyers 2016-02-05) - PLID 67980 - added dispensed in house
					MessageBox("This prescription cannot be deleted because it has been printed, voided, electronically prescribed, or dispensed in-house", NULL, MB_ICONSTOP);
				}
				else if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to remove this prescription?")) {
					if(nMedicationID != -1) {
						m_dwDeletedMedications.Add(nMedicationID);
					}
					m_MedicationList->RemoveRow(m_MedicationList->GetCurSel());
				}
			}
		}
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CCustomRecordDlg::OnPrint() 
{
	//print chart note

	if(IDYES != MsgBox(MB_YESNO, "Before previewing, this custom record's changes must be saved.  Continue?")) return;
	if(!Save()) return;

	CEmrChartNote ChartNote(m_nMasterID);
	ChartNote.OutputToWord();
}

void CCustomRecordDlg::OnAddEmrItems() 
{
	CCustomRecordSetupDlg emr(this);
	CNxModalParentDlg dlg(this, &emr, CString("Custom Record Items"));
	dlg.DoModal();

	m_EMRDetailDlg.RefreshAllItems();
}

LRESULT CCustomRecordDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (message) {
		case ID_LOAD_NEW_EMR:
			{
				try {
					if(m_nMasterID == -1) {
						_RecordsetPtr rs = CreateParamRecordset(
							"SELECT DefaultDiagID1, DefaultDiagID2, DefaultDiagID3, DefaultDiagID4 "
							"FROM PatientsT WHERE PersonID = {INT}", m_nPatID);
						if(!rs->eof) {
							//add their default diagnosis codes
							long nDefaultDiagID1, nDefaultDiagID2, nDefaultDiagID3, nDefaultDiagID4;
							nDefaultDiagID1 = AdoFldLong(rs, "DefaultDiagID1",-1);
							nDefaultDiagID2 = AdoFldLong(rs, "DefaultDiagID2",-1);
							nDefaultDiagID3 = AdoFldLong(rs, "DefaultDiagID3",-1);
							nDefaultDiagID4 = AdoFldLong(rs, "DefaultDiagID4",-1);
							if(nDefaultDiagID1 != -1 && m_DiagCodeList->FindByColumn(0,(long)nDefaultDiagID1,0,FALSE) == -1) {
								OnSelChosenDiagDropdown(m_DiagCodeCombo->SetSelByColumn(0,nDefaultDiagID1));
							}
							if(nDefaultDiagID2 != -1 && m_DiagCodeList->FindByColumn(0,(long)nDefaultDiagID2,0,FALSE) == -1) {
								OnSelChosenDiagDropdown(m_DiagCodeCombo->SetSelByColumn(0,nDefaultDiagID2));
							}
							if(nDefaultDiagID3 != -1 && m_DiagCodeList->FindByColumn(0,(long)nDefaultDiagID3,0,FALSE) == -1) {
								OnSelChosenDiagDropdown(m_DiagCodeCombo->SetSelByColumn(0,nDefaultDiagID3));
							}
							if(nDefaultDiagID4 != -1 && m_DiagCodeList->FindByColumn(0,(long)nDefaultDiagID4,0,FALSE) == -1) {
								OnSelChosenDiagDropdown(m_DiagCodeCombo->SetSelByColumn(0,nDefaultDiagID4));
							}
						}
						rs->Close();
					}
				} NxCatchAll("Error in CCustomRecordDlg::WindowProc:ID_LOAD_NEW_EMR");
			}
			break;
		case NXM_EMR_ITEM_CHANGED:
			{
				try {
					ProcessEMRInfoActions((long)wParam);
				} NxCatchAll("Error in CCustomRecordDlg::WindowProc:NXM_EMR_ITEM_CHANGED");
			}
			break;
		case NXM_EMR_DATA_CHANGED:
			{
				try {
					ProcessEMRDataActions((long)wParam);
				} NxCatchAll("Error in CCustomRecordDlg::WindowProc:NXM_EMR_DATA_CHANGED");
			}
			break;
		case WM_TABLE_CHANGED:
			{
				try {
					switch(wParam) {
						case NetUtils::PatientBDate:
						{
							try {
								//BDate has changed, update our age accordingly
								_RecordsetPtr rs = CreateParamRecordset(
									"SELECT BirthDate \r\n"
									"FROM PersonT \r\n"
									"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
									"WHERE ID = {INT}", m_nPatID);
								if(!rs->eof) {
									_variant_t var = rs->Fields->Item["BirthDate"]->Value;
									if(var.vt==VT_DATE) {
										m_bdate = var.date;
									}
									else {
										m_bdate.SetStatus(COleDateTime::invalid);
									}

									LRESULT dummy = 0;
									OnChangeDate(NULL, &dummy);
								}
							} NxCatchAll("Error in CCustomRecordDlg::WindowProc:WM_TABLE_CHANGED:PatientBDate");
						}
						break;
					}
				} NxCatchAll("Error in CCustomRecordDlg::WindowProc:WM_TABLE_CHANGED");
			}
			break;
		}

	} NxCatchAll("Error in CCustomRecordDlg::WindowProc");

	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CCustomRecordDlg::ClearDetails()
{
	m_ProcedureCombo->CurSel = -1;
	m_EMRDetailDlg.ClearDetails();
	m_DiagCodeList->Clear();
	m_BillList->Clear();
	m_MedicationList->Clear();
	SetDlgItemText(IDC_EDIT_EMR_NOTES, "");
}

void CCustomRecordDlg::OnClearDetails() 
{
	CString strNotes;
	GetDlgItemText(IDC_EDIT_EMR_NOTES, strNotes);
	if(m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize() > 0 || m_ProcedureCombo->CurSel != -1 || m_DiagCodeList->GetRowCount() > 0 ||
		m_BillList->GetRowCount() > 0 || m_MedicationList->GetRowCount() > 0 || strNotes.GetLength()) {
		if (IDNO == MessageBox("This will remove all the details you have added thus far; including purpose, diagnosis codes, service codes, prescriptions and notes.\n"
		"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO))
		return;
	} else {
		return;
	}
		
	ClearDetails();

	if(m_ProcedureCombo->GetCurSel() != -1) {
		//always add one blank detail
		m_EMRDetailDlg.TryAddBlankDetail();
	}
}

void DeleteEMRDetail(long EMRDetailID) {

	try {
		//ExecuteSql("DELETE FROM EmrSelectT WHERE EMRDetailID = %li", EMRDetailID);
		//ExecuteSql("DELETE FROM EmrDetailsT WHERE ID = %li",EMRDetailID);
		ExecuteSql("UPDATE EmrDetailsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), EMRDetailID);
		// (c.haag 2006-10-19 13:00) - PLID 21454 - I don't know if we can assign problems to
		// custom records, but I'll do this for completeness
		// (j.jones 2008-07-16 09:11) - PLID 30739 - You can't, but I'm supporting EMRRegardingType and EMRRegardingID anyways.
		// (c.haag 2009-05-11 17:38) - PLID 28494 - Problem-regarding information now goes into its own table
		// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
		// and if so, delete just our links, otherwise delete the whole problem
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
			"CASE WHEN EMRProblemID IN ("
				"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT ((EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
					"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT})) "
			") "
			"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
			"FROM EMRProblemLinkT WHERE	((EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}))",
			eprtEmrItem, EMRDetailID, eprtEmrDataItem, EMRDetailID,
			eprtEmrItem, EMRDetailID, eprtEmrDataItem, EMRDetailID);

		CString strEMRProblemLinkIDsToDelete, strEMRProblemIDsToDelete;
		while(!rsLinkedProblems->eof) {

			//every record references a link to delete
			long nProblemLinkID = AdoFldLong(rsLinkedProblems, "ID");

			if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
				strEMRProblemLinkIDsToDelete += ",";
			}
			strEMRProblemLinkIDsToDelete += AsString(nProblemLinkID);

			//the problem ID will be -1 if we're not deleting the problem the above link references,
			//will be a real ID if we do need to delete the problem
			long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
			if(nProblemID != -1) {
				if(!strEMRProblemIDsToDelete.IsEmpty()) {
					strEMRProblemIDsToDelete += ",";
				}
				//we might end up having duplicate IDs in this string, but it's just for an IN clause,
				//so it's no big deal
				strEMRProblemIDsToDelete += AsString(nProblemID);
			}

			rsLinkedProblems->MoveNext();
		}
		rsLinkedProblems->Close();

		// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
		if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
			ExecuteSql("DELETE FROM EmrProblemLinkT WHERE ID IN (%s)", strEMRProblemLinkIDsToDelete);
		}
		if(!strEMRProblemIDsToDelete.IsEmpty()) {
			ExecuteSql("UPDATE EmrProblemsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = '%s' WHERE ID IN (%s)", _Q(GetCurrentUserName()), strEMRProblemIDsToDelete);
		}

	}NxCatchAll("Error deleting custom record detail.");
}

// (j.armen 2013-05-08 13:15) - PLID 56603 - Parameterized many of the functions here while 
//	I was making EMRMasterT and EMRGroupsT utilize the identity columns
BOOL CCustomRecordDlg::Save()
{
	if(m_ProcedureCombo->GetCurSel() == -1) {
		AfxMessageBox("You must have a procedure selected to save a custom record.");
		return FALSE;
	}

	try {
		//get provider ID, null is allowed
		_variant_t vtProviderID = g_cvarNull;
		if(m_ProviderCombo->GetCurSel()!=-1) {
			vtProviderID = m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0);
		}

		//get age and gender
		_variant_t vtAge = g_cvarNull;
		_variant_t vtGender = g_cvarNull;

		COleDateTime dt;
		dt.m_status = COleDateTime::invalid;
		_RecordsetPtr rs = CreateParamRecordset("SELECT BirthDate, Gender FROM PersonT WHERE ID = {INT}",m_nPatID);
		if(!rs->eof) {
			_variant_t var = rs->Fields->Item["BirthDate"]->Value;
			if(var.vt==VT_DATE)
				dt = var.date;

			// (j.jones 2013-09-18 10:00) - PLID 58525 - get the gender in a local BYTE value
			BYTE iGender = VarByte(rs->Fields->Item["Gender"]->Value, 0);
			switch(iGender)
			{
				case 1:
					m_strGender.SetWindowText("Male");
					vtGender = AsLong(iGender);
					break;
				case 2:
					m_strGender.SetWindowText("Female");
					vtGender = AsLong(iGender);
					break;
				// (j.jones 2013-09-18 10:01) - PLID 58525 - if there is no gender,
				// (including if it is 0), ensure the variant is NULL for saving
				default:
					m_strGender.SetWindowText("");
					vtGender = g_cvarNull;
					break;
			}
		}
		else {
			vtAge = g_cvarNull;
			vtGender = g_cvarNull;
		}
		rs->Close();

		if(dt.m_status==COleDateTime::invalid) {
			vtAge = g_cvarNull;
		} else {
			COleDateTime dtEMR = VarDateTime(m_dtPicker.GetValue());
			// Make sure the birthdate is in the past
			if(dt > dtEMR) {
				AfxMessageBox("This custom record is dated to be earlier than the patient's birthdate. Please correct this date.");
				return FALSE;
			}
			
			// calculate the appropriate age (see if they've had a birthday yet)
			// Start by calculating the difference in years
			long nAge = dtEMR.GetYear() - dt.GetYear();
			
			// Now decide whether we need to subtract one
			int nMonths = dtEMR.GetMonth() - dt.GetMonth();
			if (nMonths < 0) {
				// The patient's birth month is greater than (after) the current month so
				// age is simple year difference minus 1 (they haven't had their birthday)
				nAge = nAge - 1;
			} else if (nMonths == 0) {
				// This is their birth month, so compare days
				int nDays = dtEMR.GetDay() - dt.GetDay();
				if (nDays < 0) {
					// The patient's birth day is greater than (after) the current day so
					// age is simple year difference minus 1 (they haven't had their birthday)
					nAge = nAge - 1;
				}
			}

			vtAge = nAge;
			m_strAge.SetWindowText(FormatString("%li", nAge));
		}

		long nLocID = -1;
		if(m_LocationCombo->GetCurSel() != -1 && m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).vt == VT_I4) {
			nLocID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0));
		}
		else if(m_nMasterID == -1) {
			_RecordsetPtr rsLocID = CreateParamRecordset("SELECT Location FROM PersonT WHERE ID = {INT}", m_nPatID);
			nLocID = rsLocID->eof ? -1 : AdoFldLong(rsLocID, "Location", -1);
		}
		else {
			_RecordsetPtr rsLocID = CreateParamRecordset("SELECT LocationID FROM EMRMasterT WHERE ID = {INT}", m_nMasterID);
			nLocID = rsLocID->eof ? -1 : AdoFldLong(rsLocID, "LocationID", -1);
		}
		if(nLocID == -1) nLocID = GetCurrentLocationID();

		CString strNotes;
		GetDlgItemText(IDC_EDIT_EMR_NOTES, strNotes);

		//Appointment stuff.
		BOOL bNeedsFollowUp = IsDlgButtonChecked(IDC_CHECK_FU);
		_variant_t vtFuUnit;
		if(!bNeedsFollowUp) {
			vtFuUnit = g_cvarNull;
		}
		else {
			if(m_FUTimeCombo->CurSel == -1) {
				MsgBox("Please select a valid unit of time for the Follow-Up appointment.");
				return FALSE;
			}
			vtFuUnit = m_FUTimeCombo->GetValue(m_FUTimeCombo->CurSel, 0);
		}

		_variant_t vtFuNumber;
		if(!bNeedsFollowUp) {
			vtFuNumber = g_cvarNull;
		} else {
			CString strFuNumber;
			GetDlgItemText(IDC_EDIT_FU_NUMBER, strFuNumber);
			strFuNumber.TrimLeft();
			if(strFuNumber.IsEmpty()) {
				MsgBox("Please enter a valid number of %s for the Follow-Up appointment.", VarString(m_FUTimeCombo->GetValue(m_FUTimeCombo->CurSel, 1)));
				return FALSE;
			}
			vtFuNumber = strFuNumber;
		}

		BOOL bNeedsProcedure = IsDlgButtonChecked(IDC_CHECK_PROCEDURE);
		_variant_t vtApptProcedureID;
		if(!bNeedsProcedure) {
			vtApptProcedureID = g_cvarNull;
		}
		else {
			if(m_SchedProcedureCombo->CurSel == -1) {
				MsgBox("Please select a valid procedure to schedule.");
				return FALSE;
			}
			vtApptProcedureID = m_SchedProcedureCombo->GetValue(m_SchedProcedureCombo->CurSel, 0);
		}

		BOOL bEmptyData = FALSE;
		//before we save anything, check the item info
		for(int i=0; i<m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize(); i++) {

			long InfoID,DataID;
			int datatype;
			CString text;

			int result = ((CCustomRecordItemDlg*)m_EMRDetailDlg.m_aryEMRItemDlgs.GetAt(i))->GetItemInfo(InfoID,datatype,DataID,text);
			if(result == ITEM_EMPTY)
				bEmptyData = TRUE;
		}

		if(bEmptyData) {
			AfxMessageBox("One or more details have no data entered, please correct this or remove these details before saving.");
			return FALSE;
		}

		//before we save, check to see if there are any required items not present on the EMR
		
		//TODO: check required items for diag codes

		BOOL bReqItemNotFound = FALSE;
		CString strReqItems = "";
		_RecordsetPtr rsReqInfo = CreateParamRecordset(
			"SELECT EMRInfoT.* FROM ProcedureToEMRInfoT \r\n"
			"INNER JOIN EMRInfoT ON ProcedureToEMRInfoT.EMRInfoID = EMRInfoT.ID \r\n"
			"WHERE ProcedureID = {INT} AND Required = 1",
			VarLong(m_ProcedureCombo->GetValue(m_ProcedureCombo->GetCurSel(),0)));
		while(!rsReqInfo->eof) {

			BOOL bFound = FALSE;
			long ReqInfoID = AdoFldLong(rsReqInfo, "ID");

			for(int i=0; i<m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize(); i++) {

				long InfoID,DataID;
				int datatype;
				CString text;

				int result = ((CCustomRecordItemDlg*)m_EMRDetailDlg.m_aryEMRItemDlgs.GetAt(i))->GetItemInfo(InfoID,datatype,DataID,text);
				if(result == ITEM_OK && InfoID == ReqInfoID) {
					bFound = TRUE;
					break;
				}

			}

			if(!bFound) {
				bReqItemNotFound = TRUE;
				strReqItems += AdoFldString(rsReqInfo, "Name","");
				strReqItems += ", ";
			}

			rsReqInfo->MoveNext();
		}
		rsReqInfo->Close();

		if(bReqItemNotFound) {
			strReqItems.TrimRight(", ");
			CString str = FormatString("The following item(s) are marked 'required' but are not in your custom record:\n\n"
				"%s\n\nDo you still wish to save this EMR?", strReqItems);
			if(IDNO==MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO))
				return FALSE;
		}

		if(m_nMasterID == -1) {
			//TES 2/4/2005 - Let's insert a dummy record into EmrGroupsT, there's a dependency there, and anyway if they
			//ever upgrade we'll want that record there.
			CString strDescription = (m_ProcedureCombo->CurSel == -1)
				? "{No Procedure}"
				: VarString(m_ProcedureCombo->GetValue(m_ProcedureCombo->CurSel, 1));

			// (j.armen 2013-05-08 13:16) - PLID 56603 - Let's batch these together and utilize the identity columns
			CParamSqlBatch sql;
				
			sql.Add("SET NOCOUNT ON \r\n");
			
			sql.Add(
				"DECLARE @EMRGroupsT_ID TABLE(ID INT NOT NULL) \r\n"
				"INSERT INTO EMRGroupsT (PatientID, Description, Status, InputDate) \r\n"
				"OUTPUT inserted.ID INTO @EMRGroupsT_ID \r\n"
				"SELECT {INT}, {STRING}, 0, getdate() \r\n",
				m_nPatID, strDescription);

			//TES 9/22/2005 - Let's also insert a dummy record into PicT.
			// (b.cardillo 2005-10-12 13:42) - PLID 17178 - Default IsCommitted to true 
			// because by OUR very existence, our parent must be fully committed.
			sql.Add(
				"INSERT INTO PicT (EmrGroupID, IsCommitted) \r\n"
				"SELECT ID, 1 FROM @EMRGroupsT_ID \r\n");

			//first create the master record
			// (j.jones 2007-07-31 10:20) - PLID 26276 - added CompletionStatus, which is unused in Custom Records
			// but a non-nullable field
			// (j.armen 2013-05-07 15:31) - PLID 55315 - EMRMasterT.ID is now an identity
			sql.Add(
				"DECLARE @EMRMasterT_ID TABLE(ID INT NOT NULL) \r\n"
				"INSERT INTO EMRMasterT ( \r\n"
				"	PatientID, LocationID, Date, InputDate, PatientAge, \r\n"
				"	PatientGender, AdditionalNotes, NeedsFollowUp, FollowUpNumber, FollowUpUnit, \r\n"
				"	NeedsProcedure, ApptProcedureID, EmrGroupID, CompletionStatus) \r\n"
				"OUTPUT inserted.ID INTO @EMRMasterT_ID \r\n"
				"SELECT \r\n"
				"	{INT}, {INT}, dbo.AsDateNoTime({OLEDATETIME}), GetDate(), {VT_I4}, \r\n"
				"	{VT_I4}, {STRING}, {BOOL}, {VT_BSTR}, {VT_I4}, \r\n"
				"	{BOOL}, {VT_I4}, ID, {INT} \r\n"
				"FROM @EMRGroupsT_ID \r\n",
				m_nPatID, nLocID, VarDateTime(m_dtPicker.GetValue()), vtAge, 
				vtGender, strNotes, bNeedsFollowUp ? 1 : 0, vtFuNumber, vtFuUnit, 
				bNeedsProcedure ? 1 : 0, vtApptProcedureID, ecsPartiallyComplete);

			if(m_ProcedureCombo->CurSel != -1) {
				sql.Add(
					"INSERT INTO EmrProcedureT (EmrID, ProcedureID) \r\n"
					"SELECT ID, {INT} FROM @EMRMasterT_ID \r\n",
					VarLong(m_ProcedureCombo->GetValue(m_ProcedureCombo->GetCurSel(),0)));
			}
			if(vtProviderID != g_cvarNull) {
				sql.Add(
					"INSERT INTO EmrProvidersT (EmrID, ProviderID) \r\n"
					"SELECT ID, {INT} FROM @EMRMasterT_ID \r\n",
					VarLong(vtProviderID));
			}

			sql.Add("SET NOCOUNT OFF");

			sql.Add("SELECT ID FROM @EMRMasterT_ID");

			_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());
			m_nMasterID = AdoFldLong(prs, "ID");
		}
		else {

			//TODO: (j.jones 2006-04-26 10:30) - we need to be able to identify deleted procedures,
			//and only delete those, instead of deleting all and recreating. We can't reliably utilize
			//the deleted flag while this behavior exists.

			//TODO: Audit this!

			//first update the master record
			// (j.armen 2013-05-08 13:16) - PLID 56603 - Let's batch these
			CParamSqlBatch sql;

			sql.Add(
				"UPDATE EMRMasterT SET \r\n"
				"	LocationID = {INT}, Date = {OLEDATETIME}, PatientAge = {VT_I4}, \r\n"
				"	AdditionalNotes = {STRING}, NeedsFollowup = {BOOL}, FollowUpNumber = {VT_BSTR}, \r\n"
				"	FollowUpUnit = {VT_I4}, NeedsProcedure = {BOOL}, ApptProcedureID = {VT_I4} \r\n"
				"WHERE ID = {INT} \r\n",
				nLocID, VarDateTime(m_dtPicker.GetValue()), vtAge, 
				strNotes, bNeedsFollowUp ? 1 : 0, vtFuNumber, 
				vtFuUnit, bNeedsProcedure ? 1 : 0, vtApptProcedureID, 
				m_nMasterID);

			sql.Add("DELETE FROM EmrProcedureT WHERE EmrID = {INT}", m_nMasterID);
			if(m_ProcedureCombo->CurSel != -1) {
				sql.Add(
					"INSERT INTO EmrProcedureT (EmrID, ProcedureID) \r\n"
					"SELECT {INT}, {INT} \r\n",
					m_nMasterID, VarLong(m_ProcedureCombo->GetValue(m_ProcedureCombo->GetCurSel(),0)));
			}

			sql.Add("DELETE FROM EmrProvidersT WHERE EmrID = {INT}", m_nMasterID);
			if(vtProviderID != g_cvarNull) {
				sql.Add(
					"INSERT INTO EmrProvidersT (EmrID, ProviderID) \r\n"
					"SELECT {INT}, {INT} \r\n",
					m_nMasterID, VarLong(vtProviderID));
			}

			sql.Execute(GetRemoteData());
		}

		//if there are any deleted items, remove them now
		for(int q=0; q<m_EMRDetailDlg.m_aryDeletedDetails.GetSize(); q++) {
			long DetailID = ((long)m_EMRDetailDlg.m_aryDeletedDetails.GetAt(q));
			if(DetailID != -1) {
				DeleteEMRDetail(DetailID);
			}
		}
		m_EMRDetailDlg.m_aryDeletedDetails.RemoveAll();

		//check for deleted diagnosis codes
		// (r.gonet 03/24/2014) - PLID 61131 - Custom records should filter out cases where DiagCodeID is null. 
		// That is an ICD-10 code, which Custom Records does not support. Note that it is unlikely that we will ever
		// have such a case because how did an ICD-10 code get added to a Custom Record anyhow?
		_RecordsetPtr rsDiags = CreateParamRecordset(
			"SELECT ID, DiagCodeID FROM EMRDiagCodesT \r\n"
			"WHERE EMRDiagCodesT.Deleted = 0 AND EMRID = {INT} AND EMRDiagCodesT.DiagCodeID IS NOT NULL", m_nMasterID);
		while(!rsDiags->eof) {
			long nID = AdoFldLong(rsDiags, "ID");
			long nDiagID = AdoFldLong(rsDiags, "DiagCodeID");
			if(m_DiagCodeList->FindByColumn(0,nDiagID,0,FALSE) == -1) {
				//it's been deleted
				//ExecuteSql("DELETE FROM EMRDiagCodesT WHERE EMRID = %li AND DiagCodeID = %li",m_ID,DiagID);
				ExecuteParamSql(
					"UPDATE EmrDiagCodesT SET \r\n"
					"Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} \r\n"
					"WHERE EMRID = {INT} AND DiagCodeID = {INT}", GetCurrentUserName(), m_nMasterID, nDiagID);
				// (c.haag 2008-07-28 09:42) - PLID 30853 - Delete all problems linked directly with the diagnosis code.
				// (c.haag 2009-05-11 17:38) - PLID 28494 - Problem-regarding information now goes into its own table
				// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
				// and if so, delete just our links, otherwise delete the whole problem
				// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
				_RecordsetPtr rsLinkedProblems = CreateParamRecordset(
					"SELECT ID, \r\n"
					"	CASE WHEN EMRProblemID IN ( \r\n"
					"		SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) \r\n"
					"	) \r\n"
					"	THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete \r\n"
					"FROM EMRProblemLinkT \r\n"
					"WHERE EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT} \r\n",
					eprtEmrDiag, nID,
					eprtEmrDiag, nID);

				CArray<long> aryEMRProblemLinkIDsToDelete;
				CArray<long> aryEMRProblemIDsToDelete;
				while(!rsLinkedProblems->eof) {

					//every record references a link to delete
					aryEMRProblemLinkIDsToDelete.Add(AdoFldLong(rsLinkedProblems, "ID"));

					//the problem ID will be -1 if we're not deleting the problem the above link references,
					//will be a real ID if we do need to delete the problem
					long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
					if(nProblemID != -1) {
						//we might end up having duplicate IDs in this string, but it's just for an IN clause,
						//so it's no big deal
						aryEMRProblemIDsToDelete.Add(nProblemID);
					}

					rsLinkedProblems->MoveNext();
				}
				rsLinkedProblems->Close();

				// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
				if(!aryEMRProblemLinkIDsToDelete.IsEmpty()) {
					ExecuteParamSql("DELETE FROM EmrProblemLinkT WHERE ID IN ({INTARRAY})", aryEMRProblemLinkIDsToDelete);
				}
				if(!aryEMRProblemIDsToDelete.IsEmpty()) {
					ExecuteParamSql(
						"UPDATE EmrProblemsT SET \r\n"
						"	Deleted = 1, DeletedDate = GetDate(), DeletedBy = {STRING} \r\n"
						"WHERE ID IN ({INTARRAY})", GetCurrentUserName(), aryEMRProblemIDsToDelete);
				}
			}
			rsDiags->MoveNext();
		}
		rsDiags->Close();

		//save the new diagnosis codes
		for(int i=0; i< m_DiagCodeList->GetRowCount(); i++) {
			long nDiagID = VarLong(m_DiagCodeList->GetValue(i,0));
			if(!ReturnsRecordsParam(
				"SELECT EMRID FROM EMRDiagCodesT \r\n"
				"WHERE EMRDiagCodesT.Deleted = 0 AND EMRID = {INT} AND DiagCodeID = {INT}", m_nMasterID, nDiagID))
			{
				if(!ReturnsRecordsParam("SELECT ID FROM DiagCodes WHERE ID = {int}", nDiagID)) {
					CString str = FormatString("The ICD-9 code '%s' has been deleted by another user. It will not be saved to this custom record.",
						VarString(m_DiagCodeList->GetValue(i,1),""));
					AfxMessageBox(str);
				}
				else {
					ExecuteParamSql("INSERT INTO EMRDiagCodesT (EMRID, DiagCodeID) VALUES ({INT}, {INT})", m_nMasterID, nDiagID);
				}
			}
		}
		
		SaveMedications();

		//now save the item info
		for(int i=0; i<m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize(); i++) {

			long nInfoID,nDataID;
			int datatype;
			CString strText;

			CCustomRecordItemDlg *pInfo = ((CCustomRecordItemDlg*)m_EMRDetailDlg.m_aryEMRItemDlgs.GetAt(i));

			long nDetailID = pInfo->m_ID;

			if(pInfo->GetItemInfo(nInfoID,datatype,nDataID,strText) == ITEM_OK) {

				if(datatype<=1) {

					if(nDetailID == -1) {
						// (c.haag 2007-06-13 12:31) - PLID 26316 - We now get the generated ID from data
						// (a.walling 2012-07-11 18:02) - PLID 51491 - Set nocount before getting the new identity
						_RecordsetPtr prsID = CreateParamRecordset(
							"SET NOCOUNT ON \r\n"
							"DECLARE @EMRDetailsT_ID TABLE(ID INT NOT NULL) \r\n"
							"INSERT INTO EMRDetailsT (EMRID, EMRInfoID, Text, OrderID) \r\n"
							"OUTPUT inserted.ID INTO @EMRDetailsT_ID \r\n"
							"SELECT {INT}, {INT}, {STRING}, {INT} \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT ID FROM @EMRDetailsT_ID \r\n",
							m_nMasterID, nInfoID, strText, i+1);

						nDetailID = AdoFldLong(prsID, "ID");

						pInfo->m_ID = nDetailID;
					}
					else {
						ExecuteParamSql(
							"UPDATE EMRDetailsT SET \r\n"
							"	EMRInfoID = {INT}, OrderID = {INT}, Text = {STRING} \r\n"
							"WHERE ID = {INT} \r\n",
							nInfoID, i+1, strText, nDetailID);
					}
				}
				else {

					if(nDetailID == -1) {
						// (c.haag 2007-06-13 12:34) - PLID 26316 - We now get the generated ID from data
						// (a.walling 2012-07-11 18:02) - PLID 51491 - Set nocount before getting the new identity
						_RecordsetPtr prsID = CreateParamRecordset(
							"SET NOCOUNT ON\r\n"
							"DECLARE @EMRDetailsT_ID TABLE(ID INT NOT NULL) \r\n"
							"INSERT INTO EMRDetailsT (EMRID, EMRInfoID, OrderID) \r\n"
							"OUTPUT inserted.ID INTO @EMRDetailsT_ID \r\n"
							"SELECT {INT}, {INT}, {INT} \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT ID FROM @EMRDetailsT_ID \r\n",
							m_nMasterID, nInfoID, i+1);

						nDetailID = AdoFldLong(prsID, "ID");

						if(nDataID != -1) {
							ExecuteParamSql(
								"INSERT INTO EmrSelectT (EMRDetailID, EMRDataID) VALUES ({INT}, {INT})", nDetailID, nDataID);
						}
						pInfo->m_ID = nDetailID;
					}
					else {
						//TODO: Audit this!
						// (c.haag 2007-02-16 11:10) - PLID 24785 - The nDataID parameter does not belong here. Furthermore,
						// it makes the DetailID parameter ignored. What that means is saving existing details never worked 
						// since the inception of this bug.
						ExecuteParamSql("UPDATE EMRDetailsT SET EMRInfoID = {INT}, OrderID = {INT} WHERE ID = {INT}",
							nInfoID,/*nDataID,*/i+1, nDetailID);
						ExecuteParamSql("DELETE FROM EmrSelectT WHERE EMRDetailID = {INT}", nDetailID);
						if(nDataID != -1) {
							ExecuteParamSql(
								"INSERT INTO EmrSelectT (EMRDetailID, EMRDataID) \r\n"
								"SELECT {INT}, {INT}",
								nDetailID, nDataID);
						}
					}
				}
			}
		}

		//now save charges
		//TODO - make this cleaner, obviously this is the brute force approach
		// (j.jones 2006-04-26 10:28) - also this way doesn't delete "only deleted" charges,
		// so we can't reliably mark charges as deleted
		// (j.jones 2008-06-04 16:13) - PLID 30255 - delete from EMRQuotedChargesT, although
		// this should be impossible because custom records shouldn't have these entries,
		// and we are not going to attempt to recreate the links
		// (j.dinatale 2012-01-10 10:06) - PLID 47456 - added a new table to handle EMR Charges Responsibilities, EMRChargeRespT, need to delete from there before
		//	deleting from EMRChargesT.
		// (j.jones 2012-01-18 11:19) - PLID 47537 - unlink these from bill charges as well
		ExecuteParamSql("DELETE FROM EMRQuotedChargesT "
			"WHERE EMRChargeID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})", m_nMasterID);
		ExecuteParamSql("DELETE FROM EMRChargeRespT WHERE EMRChargeID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})", m_nMasterID);
		ExecuteParamSql("UPDATE ChargesT SET EMRChargeID = NULL WHERE EMRChargeID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})", m_nMasterID);
		ExecuteParamSql("DELETE FROM EMRChargesT WHERE EMRID = {INT}", m_nMasterID);
		for(long i = 0; i < m_BillList->GetRowCount(); i++)
		{
			// (j.armen 2013-06-27 15:39) - PLID 57354 - Idenitate EMRChargesT
			ExecuteParamSql(
				"INSERT INTO EMRChargesT (EMRID, ServiceID, Description, Quantity, UnitCost) \r\n"
				" VALUES ({INT}, {INT}, {STRING}, {DOUBLE}, {OLECURRENCY}) \r\n",
				m_nMasterID,
				VarLong(m_BillList->GetValue(i,CHARGE_COLUMN_SERVICE_ID)),
				VarString(m_BillList->GetValue(i,CHARGE_COLUMN_DESCRIPTION),""),
				VarDouble(m_BillList->GetValue(i,CHARGE_COLUMN_QTY),1.0),
				VarCurrency(m_BillList->GetValue(i,CHARGE_COLUMN_COST),COleCurrency(0,0)));
		}

		SynchronizeTodoAlarms();

		return TRUE;

	}NxCatchAll("Error creating new custom record.");

	return FALSE;
}

void CCustomRecordDlg::OnTrySetSelFinishedEmrLocationsCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//Must be inactive.
		_RecordsetPtr rsLocName;
		if(m_nMasterID == -1) {
			rsLocName = CreateParamRecordset(
				"SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM PersonT WHERE ID = {INT})", m_nPatID);
		}
		else {
			rsLocName = CreateParamRecordset(
				"SELECT Name FROM LocationsT WHERE ID = (SELECT LocationID FROM EMRMasterT WHERE ID = {INT})", m_nMasterID);
		}

		if(!rsLocName->eof) {
			m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
		}
	}
}

void CCustomRecordDlg::OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CalculateAge();

	*pResult = 0;
}

void CCustomRecordDlg::OnCloseUpDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	CalculateAge();

	*pResult = 0;
}

void CCustomRecordDlg::CalculateAge()
{
	try {
		if(((COleDateTime)m_dtPicker.GetValue()).m_dt < m_bdate.m_dt) {
			AfxMessageBox("The service date cannot be before the patient's birthdate.");
			m_dtPicker.SetValue((_variant_t)COleDateTime::GetCurrentTime());
			//return;		//Can't return here, or the birthdate field is then messed up.  We still need to calc the age.
		}

		//always change the age
		if(m_bdate.GetStatus()!=COleDateTime::invalid) {
			// Use a temporary value so that GetPatientAge doesn't write to our stored member variable
			COleDateTime dtBirthDateTemporary = m_bdate;
			// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
			//  validation should only be done when bdays are entered/changed
			// (z.manning 2010-01-13 10:58) - PLID 22672 - Age is now a string
			m_strAge.SetWindowText(GetPatientAgeOnDate(dtBirthDateTemporary, ((COleDateTime)m_dtPicker.GetValue()), TRUE));
		}
		else {
			//invalid status, make sure it is empty
			m_strAge.SetWindowText("");
		}

	}NxCatchAll("Error saving date / age.");
}

void CCustomRecordDlg::OnSelChosenDiagDropdown(long nRow) 
{
	//JJ - Every time they add a diagnosis code, we may be adding EMR items

	try {

		if(nRow == -1) {
			return;
		}

		long DiagID = m_DiagCodeCombo->GetValue(nRow,0).lVal;

		//first ensure it's not already in the list
		if(m_DiagCodeList->FindByColumn(0,(long)DiagID,0,FALSE) >= 0) {
			AfxMessageBox("This diagnosis code has already been selected.");
			return;
		}

		//now add it to the list, it will be saved on OK
		m_DiagCodeList->AddRow(m_DiagCodeCombo->GetRow(nRow));

		m_EMRDetailDlg.AddDiagCodeID(DiagID);
		m_EMRDetailDlg.RefreshAllItems();

		CWaitCursor pWait;

		//if there is a blank line, remove it
		m_EMRDetailDlg.TryRemoveBlankDetail();

		_RecordsetPtr rs = CreateRecordset("SELECT EMRInfoT.* FROM DiagCodeToEMRInfoT INNER JOIN EMRInfoT ON DiagCodeToEMRInfoT.EMRInfoID = EMRInfoT.ID WHERE DiagCodeID = %li AND Required = 1",DiagID);

		if(!rs->eof) {

			while(!rs->eof) {

				long InfoID = rs->Fields->Item["ID"]->Value.lVal;

				if(!m_EMRDetailDlg.IsInfoInList(InfoID)) {
					m_EMRDetailDlg.AddDetail(InfoID,-1);

					//JJ - don't do it here, it gets processed as the Info selection is set, deeper into the code
					//ProcessEMRInfoActions(InfoID);
				}

				rs->MoveNext();
			}
		}

		rs->Close();

		m_DiagCodeCombo->CurSel = -1;

		//create an empty detail item if none exist	
		m_EMRDetailDlg.TryAddBlankDetail();

		m_btnPrint.EnableWindow(TRUE);
		m_btnPrintReport.EnableWindow(TRUE);

	}NxCatchAll("Error loading required items.");
}

void CCustomRecordDlg::OnBtnAddCharge() 
{
	CCustomRecordSelectServiceDlg dlg(this);
	if(dlg.DoModal() == IDOK) {
		AddChargeToBillList(dlg.m_ServiceID);
	}	
}

void CCustomRecordDlg::OnBtnCreateBill() 
{
	try {

		if(!CheckCurrentUserPermissions(bioBill,sptCreate)) return;

		//checks for any active global period, and warns accordingly
		// (a.walling 2008-07-07 18:03) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		if(!CheckWarnGlobalPeriod(m_nPatID))
			return;

		// (a.walling 2009-12-22 17:26) - PLID 7002 - Maintain only one instance of a bill
		if (!GetMainFrame()->IsBillingModuleOpen(true)) {
			if(IDNO == MessageBox("Before creating a bill, the custom record will automatically be saved and closed.\n"
				"Are you sure you wish to create a bill at this time?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			if(!Save())
				return;

			//bill this EMR
			m_pBillingDlg->m_pFinancialDlg = this;
			// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
			// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
			m_pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::EMR);
			m_pBillingDlg->PostMessage(NXM_BILL_EMR, m_nMasterID);

			CDialog::OnOK();
		}
	
	}NxCatchAll("Error creating bill.");
}

void CCustomRecordDlg::OnBtnAddMedication() 
{
	CString sql, description;

	
	try {

		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication,sptCreate))
			return;

		long nMedicationID;
		
		
		//Open the Medication Selection Dialog
		CMedicationSelectDlg dlg(this);
		long nResult;
		nResult = dlg.DoModal();

		if (nResult == IDOK) {

			nMedicationID = dlg.m_nMedicationID;

			//Check that the patient isn't allergic to it.
			if(!CheckAllergies(GetActivePatientID(), dlg.m_nMedicationID)) {
				return;
			}
			
			//insert the medication into PatientMedications
			// (c.haag 2007-02-02 18:47) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			// (d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity, Description is now PatientInstructions (and parameterized)
			// (d.thompson 2009-01-15) - PLID 32176 - DrugList.Unit is now DrugStrengthUnitsT.Name, joined from DrugList.StrengthUnitID
			// (d.thompson 2009-03-11) - PLID 33481 - Actually this should use QuantityUnitID, not StrengthUnitID
			_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDataT.Data AS Name, PatientInstructions, DefaultRefills, DefaultQuantity, "
				"COALESCE(DrugStrengthUnitsT.Name, '') AS Unit FROM DrugList "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"LEFT JOIN DrugStrengthUnitsT ON DrugList.QuantityUnitID = DrugStrengthUnitsT.ID "
				"WHERE DrugList.ID = {INT}", nMedicationID);

			
			FieldsPtr fields;
			fields = rs->Fields;

			//Put the variables from the recordset into local variables
			CString strName, strDescription, strUnit, strPills, strRefills;

			strName = VarString(fields->Item["Name"]->Value);
			// (d.thompson 2008-12-01) - PLID 32174 - Description is now PatientInstructions
			strDescription = VarString(fields->Item["PatientInstructions"]->Value);
			strRefills = VarString(fields->Item["DefaultRefills"]->Value);
			// (d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity
			strPills = VarString(fields->Item["DefaultQuantity"]->Value);
			strUnit = AdoFldString(fields, "Unit");			

			// (d.thompson 2009-03-12) - PLID 33482 - If the quantity or qty units are blank, that means they've got
			//	some screwy data, and we really want them to fix it.  Most likely it happened due to the 9100 changes
			//	that moved Unit from free text to a defined field.  Pop up the "correction" dialog, which will update
			//	data and give us back the results.
			if(strPills.IsEmpty() || strUnit.IsEmpty()) {
				CCorrectMedicationQuantitiesDlg dlg(this);
				dlg.m_nDrugListID = nMedicationID;
				if(dlg.DoModal() == IDOK) {
					strPills = dlg.m_strOutputDefQty;
					strUnit = dlg.m_strOutputQtyUnit;
				}
				else {
					//We do let them cancel, though we recommend against it.  We won't have anything to update if so
				}
			}
			else {
				//pills and unit are both filled, there's no need to fix anything
			}

			rs->Close();

			//Add the row to the medication datalist
			IRowSettingsPtr pRow;
			pRow = m_MedicationList->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, (long)nMedicationID);
			pRow->PutValue(2, _variant_t(strName));
			pRow->PutValue(3, _variant_t(strDescription));
			pRow->PutValue(4, _variant_t(strRefills));
			pRow->PutValue(5, _variant_t(strPills));
			pRow->PutValue(6, _variant_t(strUnit));

			m_MedicationList->InsertRow(pRow, -1);

		}
	
	}NxCatchAll("Error writing prescription.");
}

void CCustomRecordDlg::OnBtnWritePrescriptions() 
{
	try {

		if(m_MedicationList->GetRowCount() == 0) {
			MsgBox("This custom record does not have any Prescriptions to be printed.");
			return;
		}

		// Get template to merge to

		// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
		// based on how many prescriptions are printed
		long nCountPrescriptions = m_MedicationList->GetRowCount();
		BOOL bExactCountFound = FALSE;
		BOOL bOtherCountFound = FALSE;
		CString strTemplateName = GetDefaultPrescriptionTemplateByCount(nCountPrescriptions, bExactCountFound, bOtherCountFound);

		if(strTemplateName == "") {
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"There is no default template set up. The prescription cannot be printed.\n"
				"Please go to the Medications tab to set up your default prescription template.");
			return;
		}
		//if no template was found for the exact count, and there are some for other counts,
		//ask if they wish to continue or not (will use the standard default otherwise)
		else if(!bExactCountFound && bOtherCountFound) {
			CString str;
			str.Format("There is no default template configured for use with %li prescription%s, "
				"but there are templates configured for other counts of prescriptions.\n\n"
				"Would you like to continue merging using the standard prescription template?",
				nCountPrescriptions, nCountPrescriptions == 1 ? "" : "s");
			if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				AfxMessageBox("The prescription will not be be printed.\n"
					"Please go to the Medications tab to set up your default prescription template.");
				return;
			}
		}

		//Make sure this path still exists.
		CString strFullPath = GetTemplatePath("Forms", strTemplateName);
		if(!DoesExist(strFullPath)) {
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"The default prescription template no longer exists.  You must reset this in the patient medications tab "
				"before you can print a prescription.");
			return;
		}

		AfxMessageBox("Your changes to this custom record will first be saved before writing prescriptions.");

		if(!Save())
			return;

		CWaitCursor pWait;		

		// At this point, we know we want to do it by word merge, so make sure word exists
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}
		
		/// Generate the temp table
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatID);
		CString strMergeT = CreateTempIDTable(strSql, "ID");
		
		// Merge
		CMergeEngine mi;

		// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
		if(!mi.LoadSenderInfo(TRUE)) {
			return;
		}
		
		//add all prescriptions to the merge
		for(int i=0;i<m_MedicationList->GetRowCount();i++) {
			mi.m_arydwPrescriptionIDs.Add(VarLong(m_MedicationList->GetValue(i,0)));
		}	

		if (g_bMergeAllFields)
			mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

		mi.m_nFlags |= BMS_SAVE_FILE_NO_HISTORY; //save the file, do not save in history

		// Do the merge
		// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
		if (mi.MergeToWord(GetTemplatePath("Forms", strTemplateName), std::vector<CString>(), strMergeT))
		{
			//Update patient history here, because multiple merges per patient ID
			//will screw up the merge engine's method of doing it. But hey,
			//we get to make the description a lot better as a result!

			CString strDescription = "Prescription printed for ";
			// (c.haag 2007-02-02 18:47) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			for (int q = 0; q < m_MedicationList->GetRowCount(); q++) {
				_RecordsetPtr rs = CreateRecordset("SELECT PatientMedications.*, EMRDataT.Data AS Name FROM PatientMedications "
					"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
					"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
					"WHERE PatientMedications.ID = %li", VarLong(m_MedicationList->GetValue(q, 0)));
				if (!rs->eof) {
					CString MedicationName = CString(rs->Fields->Item["Name"]->Value.bstrVal);
					CString strRefills = CString(rs->Fields->Item["RefillsAllowed"]->Value.bstrVal);
					//TES 2/10/2009 - PLID 33002 - Renamed PillsPerBottle to Quantity
					CString strQuantity = CString(rs->Fields->Item["Quantity"]->Value.bstrVal);
					CString strUnit = AdoFldString(rs, "Unit");
					CString strExtra;
					strExtra.Format("%s, Quantity: %s %s, Refills: %s  /  ", MedicationName, strQuantity, strUnit, strRefills);
					strDescription += strExtra;
				}
				rs->Close();
			}

			if (strDescription.Right(5) == "  /  ")
				strDescription = strDescription.Left(strDescription.GetLength() - 5);

			// (j.jones 2008-09-04 14:37) - PLID 30288 - converted to use CreateNewMailSentEntry,
			// which creates the data in one batch and sends a tablechecker
			// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
			CreateNewMailSentEntry(m_nPatID, strDescription, SELECTION_WORDDOC, mi.m_strSavedAs, GetCurrentUserName(), "", GetCurrentLocationID());
		}
		
	}NxCatchAll("Error writing prescriptions.");	
}

void CCustomRecordDlg::OnBtnMerge() 
{
	// TODO: Add your control notification handler code here
	
}

void CCustomRecordDlg::OnCheckFu() 
{
	GetDlgItem(IDC_EDIT_FU_NUMBER)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FU));
	GetDlgItem(IDC_FU_INCREMENT)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_FU));
	if(GetDlgItem(IDC_FU_INCREMENT)->IsWindowEnabled() && m_FUTimeCombo->CurSel == -1) {
		m_FUTimeCombo->CurSel = 0;
	}
	CString strNumber;
	GetDlgItemText(IDC_EDIT_FU_NUMBER, strNumber);
	strNumber.TrimLeft();
	if(strNumber.IsEmpty()) {
		GetDlgItem(IDC_EDIT_FU_NUMBER)->SetFocus();
	}
}

void CCustomRecordDlg::OnCheckProcedure() 
{
	GetDlgItem(IDC_PROCEDURE_SCH_LIST)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_PROCEDURE));
	if(GetDlgItem(IDC_PROCEDURE_SCH_LIST)->IsWindowEnabled() && m_SchedProcedureCombo->CurSel == -1 && m_ProcedureCombo->CurSel != -1) {
		m_SchedProcedureCombo->SetSelByColumn(0, VarLong(m_ProcedureCombo->GetValue(m_ProcedureCombo->CurSel, 0)));
	}
}

void CCustomRecordDlg::OnRButtonDownDiags(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;
	m_DiagCodeList->CurSel = nRow;
	
	if(nRow != -1) {
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_ICD9, "Remove ICD-9 Code");
		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

BOOL CCustomRecordDlg::DeleteICD9(long DiagID)
{
	try {

		if(m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize() > 0 && 
			IDNO == MessageBox("If you delete this ICD-9 code, any custom record details associated with this code will be removed as well.\n\n"
			"Are you sure you wish to remove this code?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return FALSE;
		}

		//clean up the detail list, returning TRUE will let the calling function remove the diag code
		m_EMRDetailDlg.RemoveDiagCodeID(DiagID);
		m_EMRDetailDlg.RefreshAllItems();

		return TRUE;

	}NxCatchAll("Error removing diagnosis code.");

	return FALSE;
}

void CCustomRecordDlg::OnRButtonDownBill(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;
	m_BillList->CurSel = nRow;
	
	if(nRow != -1) {
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_CHARGE, "Remove Charge");
		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

void CCustomRecordDlg::OnEditingFinishingBill(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	// TODO: Add your control notification handler code here
	
}

void CCustomRecordDlg::OnEditingFinishedBill(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(nRow == -1) return;
		if(!bCommit) return;

		switch(nCol) {
		case CHARGE_COLUMN_SERVICE_ID:
			{
				//They have selected a new CPT Code.
				if(_variant_t(varOldValue) == _variant_t(varNewValue)) return;
				
				//Fill in the rest of the columns with their default values.
				_RecordsetPtr rsService = CreateRecordset("SELECT SubCode, Name, Price FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID WHERE ServiceT.ID = %li", VarLong(varNewValue));
				if(rsService->eof) {
					ASSERT(FALSE);
					return;
				}
				IRowSettingsPtr pRow = m_BillList->GetRow(nRow);
				pRow->PutValue(CHARGE_COLUMN_SUB_CODE, rsService->Fields->Item["SubCode"]->Value);
				pRow->PutValue(CHARGE_COLUMN_DESCRIPTION, rsService->Fields->Item["Name"]->Value);
				pRow->PutValue(CHARGE_COLUMN_COST, rsService->Fields->Item["Price"]->Value);
			}
		break;
		}
	}NxCatchAll("Error in CCustomRecordDlg::OnEditingFinishedBill()");
}

void CCustomRecordDlg::AddChargeToBillList(long ServiceID)
{
	try {

		_RecordsetPtr rs = CreateRecordset("SELECT ServiceT.ID, Code, SubCode, Name, Price FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID WHERE ServiceT.ID = %li",ServiceID);
		if(rs->eof)
			return;
		else {
			IRowSettingsPtr pRow = m_BillList->GetRow(-1);
			pRow->PutValue(CHARGE_COLUMN_ID,(long)-1);
			pRow->PutValue(CHARGE_COLUMN_SERVICE_ID,AdoFldLong(rs, "ID"));
			pRow->PutValue(CHARGE_COLUMN_SUB_CODE,_bstr_t(AdoFldString(rs, "SubCode","")));
			pRow->PutValue(CHARGE_COLUMN_DESCRIPTION,_bstr_t(AdoFldString(rs, "Name","")));
			pRow->PutValue(CHARGE_COLUMN_QTY,(double)1.0);
			pRow->PutValue(CHARGE_COLUMN_COST,_variant_t(AdoFldCurrency(rs, "Price",COleCurrency(0,0))));
			m_BillList->AddRow(pRow);
		}
		rs->Close();


	}NxCatchAll("Error adding charge.");
}

void CCustomRecordDlg::ProcessProcedureActions(long ProcedureID)
{
	//could potentially add a charge or diagnosis code
	ProcessEMRActions(eaoProcedure, ProcedureID);	
}

void CCustomRecordDlg::ProcessEMRInfoActions(long EMRInfoID)
{
	//could potentially add a charge or diagnosis code
	ProcessEMRActions(eaoEmrItem, EMRInfoID);
}

void CCustomRecordDlg::ProcessEMRDataActions(long EMRDataID)
{
	//could potentially add a charge or diagnosis code
	ProcessEMRActions(eaoEmrDataItem, EMRDataID);
}

void CCustomRecordDlg::ProcessEMRActions(EmrActionObject eaoSourceType, long nSourceID)
{
	// (j.jones 2005-05-19 09:42) - track the parent of the recursive spawning
	BOOL bAmITheSpawnParent = FALSE;
	if(!m_bIsSpawning) {
		bAmITheSpawnParent = TRUE;
		m_bIsSpawning = TRUE;
	}

	try {

		_RecordsetPtr rs = CreateRecordset("SELECT DestType, DestID, DiagCodeID_ICD9 FROM EMRActionsT LEFT JOIN EmrActionDiagnosisDataT ON EMRActionsT.ID = EmrActionDiagnosisDataT.EmrActionID WHERE SourceType = %li AND SourceID = %li AND Deleted = 0", eaoSourceType, nSourceID);
		while(!rs->eof) {

			EmrActionObject eaoDestType = (EmrActionObject)AdoFldLong(rs, "DestType");
			long DestID = AdoFldLong(rs, "DestID");

			switch(eaoDestType) {

				case eaoCpt: {
					AddChargeToBillList(DestID);
				}
				break;

				// (b.savon 2014-07-14 13:24) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
				//case eaoDiag: {
				case eaoDiagnosis: {
					//if it isn't already in the list, auto-add it
					long nDiagCodeID = AdoFldLong(rs, "DiagCodeID_ICD9", -1);
					if(m_DiagCodeList->FindByColumn(0,(long)nDiagCodeID,0,FALSE) < 0)
						OnSelChosenDiagDropdown(m_DiagCodeCombo->SetSelByColumn(0,(long)nDiagCodeID));
				}
				break;

				case eaoEmrItem: {
					//not supported
				}
				break;

				case eaoEmrDataItem: {
					//not supported
				}
				break;
				
				case eaoProcedure: {
					//DEFINITELY not supported
				}
				break;
			}

			rs->MoveNext();
		}
		rs->Close();

		//Now, let's also do the office visit stuff.
		if(eaoSourceType == eaoProcedure || eaoSourceType == eaoCpt || eaoSourceType == eaoEmrItem || eaoSourceType == eaoEmrDataItem) {
			TryToIncrementOfficeVisit();
		}

		// (j.jones 2005-05-19 10:07) - if we are done spawning, warn
		// if any of the office visit codes were upgraded
		if(bAmITheSpawnParent && m_dwaryOldOfficeVisitServiceIDs.GetSize() > 0) {
			ASSERT(m_dwaryOldOfficeVisitServiceIDs.GetSize() == m_dwaryNewOfficeVisitServiceIDs.GetSize());
			CString strWarning = "The following Office Visit codes have been upgraded:";
			for(int i=0; i<m_dwaryOldOfficeVisitServiceIDs.GetSize(); i++) {
				long nOldServiceID = m_dwaryOldOfficeVisitServiceIDs.GetAt(i);
				long nNewServiceID = m_dwaryNewOfficeVisitServiceIDs.GetAt(i);
				CString strOldCode, strNewCode, strOldDesc, strNewDesc;
				_RecordsetPtr rs = CreateRecordset("SELECT Code, Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = %li", nOldServiceID);
				if(!rs->eof) {
					strOldCode = AdoFldString(rs, "Code","");
					strOldDesc = AdoFldString(rs, "Name","");
				}
				rs->Close();
				rs = CreateRecordset("SELECT Code, Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = %li", nNewServiceID);
				if(!rs->eof) {
					strNewCode = AdoFldString(rs, "Code","");
					strNewDesc = AdoFldString(rs, "Name","");
				}
				rs->Close();
				CString str;
				str.Format("\n\n\"(%s) %s\" has been updated to \"(%s) %s\".",strOldCode,strOldDesc,strNewCode,strNewDesc);
				strWarning += str;
			}
			AfxMessageBox(strWarning);

			m_dwaryOldOfficeVisitServiceIDs.RemoveAll();
			m_dwaryNewOfficeVisitServiceIDs.RemoveAll();
		}

	}NxCatchAll("Error processing Custom Record Actions for this procedure.");

	// (j.jones 2005-05-19 09:44) - signal that the parent of the recursive spawning is done
	if(bAmITheSpawnParent) {
		m_bIsSpawning = FALSE;
	}
}

void CCustomRecordDlg::OnRButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;
	m_MedicationList->CurSel = nRow;
	
	if(nRow != -1) {
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_PRESCRIPTION, "Delete Prescription");
		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

void CCustomRecordDlg::OnEditingFinishingMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit == FALSE)
		return;

	if(!CheckCurrentUserPermissions(bioPatientMedication,sptWrite)) {
		// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
		VariantClear(pvarNewValue);
		*pvarNewValue = _variant_t(varOldValue).Detach();
		*pbCommit = FALSE;
		return;
	}

	switch(nCol) {

	case 4:	//number of refills 
	case 5:	//number of pills
		{
			CString strTemp = strUserEntered;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			int testNumber = atoi(strTemp);
			if(strTemp.IsEmpty()) {
				//Don't commit.
				MsgBox("You cannot enter a blank amount.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
			else if (testNumber < 0) {
				//they may have entered text, but then testNumber would be 0
				//testNumber would only be less than 0 if they entered a negative numeric number
				AfxMessageBox("Please enter a positive number.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		}
			
		
	break;

	case 6:
		//Unit
		{
			CString strTemp = strUserEntered;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(strTemp.IsEmpty()) {
				//Don't commit.
				MsgBox("You cannot enter a blank unit description.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
			if(strTemp.GetLength() > 50) {
				MsgBox("The unit description cannot have more than 50 characters.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		}
		break;	
	}
}

void CCustomRecordDlg::OnEditingFinishedMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// TODO: Add your control notification handler code here
	
}

void CCustomRecordDlg::SaveMedications()
{
	try {

		long nLocID = -1; //nProvID = -1;
		_variant_t varProvID = g_cvarNull;// (a.vengrofski 2009-12-18 18:28) - PLID <34890> - changed from long to var for the param query
		CString strTimeToDisplay = FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDate);
		CString strTimeToInsert = _Q(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate));

		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT MainPhysician, Location FROM PatientsT \r\n"
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n"
			"WHERE PatientsT.PersonID = {INT}",
			m_nPatID);

		_variant_t var;
		if(!rs->eof) {
			varProvID = rs->Fields->Item["MainPhysician"]->Value;// (a.vengrofski 2009-12-18 18:29) - PLID <34890> - Change for varProvID

			var = rs->Fields->Item["Location"]->Value;
			if(var.vt == VT_I4)
				nLocID = var.lVal;
		}
		rs->Close();

		if(nLocID == -1)
			nLocID = GetCurrentLocationID();

		// (a.vengrofski 2009-12-18 18:30) - PLID <34890> - change for varProvID
		if((varProvID.vt == VT_NULL) && m_ProviderCombo->GetCurSel() != -1) {
			varProvID = m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0);
		}

		// (a.vengrofski 2009-12-18 18:32) - PLID <34890> - changed for varProvID
		if(varProvID.vt == VT_NULL) {
			rs = CreateParamRecordset("SELECT DefaultProviderID FROM LocationsT WHERE ID = {INT}", GetCurrentLocationID());
			if(!rs->eof) {
				varProvID = rs->Fields->Item["DefaultProviderID"]->Value;
			}
			rs->Close();
		}

		//now run through all the prescriptions, update or insert as needed
		for(int i=0; i<m_MedicationList->GetRowCount(); i++) {

			IRowSettingsPtr pRow = m_MedicationList->GetRow(i);

			long nPrescriptionID = VarLong(pRow->GetValue(0));
			long nMedicationID = VarLong(pRow->GetValue(1));
			CString strName = VarString(pRow->GetValue(2),"");
			CString strPatientExplanation = VarString(pRow->GetValue(3),"");
			CString strRefills = VarString(pRow->GetValue(4));
			CString strQuantity = VarString(pRow->GetValue(5));
			CString strUnit = VarString(pRow->GetValue(6),"");

			if(nPrescriptionID == -1) {
				//new
				//TES 2/10/2009 - PLID 33002 - Renamed PillsPerBottle to Quantity, Description to PatientExplanation
				// (j.jones 2010-01-22 11:31) - PLID 37016 - supported InputByUserID
				// (j.armen 2013-05-24 15:21) - PLID 56863 - Patient Medications has an identity 
				CParamSqlBatch sql;

				sql.Add("SET NOCOUNT ON");
				sql.Add("DECLARE @PatientMedications_ID TABLE(ID INT NOT NULL)");

				sql.Add("INSERT INTO PatientMedications (\r\n"
						"	PatientID, MedicationID, PatientExplanation, PrescriptionDate, RefillsAllowed, \r\n"
						"	Quantity, ProviderID, LocationID, Unit, InputByUserID)\r\n"
						"OUTPUT inserted.ID INTO @PatientMedications_ID\r\n"
						"SELECT\r\n"
						"	{INT}, {INT}, {STRING}, {STRING}, {STRING},\r\n"
						"	{STRING}, {VT_I4}, {INT}, {STRING}, {INT}",
						m_nPatID, nMedicationID, strPatientExplanation, strTimeToInsert, strRefills, 
						strQuantity, varProvID, nLocID, strUnit, GetCurrentUserID());
					
				sql.Add("INSERT INTO EmrMedicationsT (EmrID, MedicationID)\r\n"
						"SELECT {INT}, ID FROM @PatientMedications_ID",
						m_nMasterID);

				sql.Add("SET NOCOUNT OFF");

				sql.Add("SELECT ID FROM @PatientMedications_ID");

				_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());

				nPrescriptionID = AdoFldLong(prs, "ID");

				pRow->PutValue(0, nPrescriptionID);

				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientPrescriptionCreated, m_nPatID, "", strName, aepMedium, aetCreated);
			}
			else {
				//update
				//TES 2/10/2009 - PLID 33002 - Renamed PillsPerBottle to Quantity, Description to PatientExplanation
				ExecuteParamSql(
					"UPDATE PatientMedications SET \r\n"
					"	PatientExplanation = {STRING}, RefillsAllowed = {STRING}, Quantity = {STRING}, Unit = {STRING} \r\n"
					"WHERE ID = {INT}",
					strPatientExplanation, strRefills, strQuantity, strUnit, nPrescriptionID);
			}
		}

		//now run through the list of deleted prescriptions and mark them as such
		for(i=0; i<m_dwDeletedMedications.GetSize(); i++) {

			long nMedicationID = m_dwDeletedMedications.GetAt(i);

			CString strName;

			// (c.haag 2007-02-02 18:47) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT EMRDataT.Data AS Name FROM DrugList \r\n"
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
				"INNER JOIN PatientMedications ON DrugList.ID = PatientMedications.MedicationID \r\n"
				"WHERE PatientMedications.ID = {INT}", nMedicationID);
			if(!rs->eof) {
				strName = AdoFldString(rs, "Name","");
			}
			rs->Close();

			//ExecuteSql("DELETE FROM EmrMedicationsT WHERE MedicationID = %li", nMedicationID);
			ExecuteParamSql(
				"UPDATE EmrMedicationsT SET \r\n"
				"Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} \r\n"
				"WHERE MedicationID = {INT}", GetCurrentUserName(), nMedicationID);

			ExecuteParamSql("UPDATE PatientMedications SET Deleted = 1 WHERE ID = {INT}", nMedicationID);
			// (c.haag 2008-07-28 09:59) - PLID 30853 - Delete problems related to the medication
			// (c.haag 2009-05-11 17:38) - PLID 28494 - Problem-regarding information now goes into its own table
			// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
			// and if so, delete just our links, otherwise delete the whole problem
			// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
			_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
				"CASE WHEN EMRProblemID IN ("
				"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
				") "
				"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
				"FROM EMRProblemLinkT WHERE	EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}",
				eprtEmrMedication, nMedicationID,
				eprtEmrMedication, nMedicationID);

			CArray<long> aryEMRProblemLinkIDsToDelete;
			CArray<long> aryEMRProblemIDsToDelete;
			while(!rsLinkedProblems->eof) {

				//every record references a link to delete
				long nProblemLinkID = AdoFldLong(rsLinkedProblems, "ID");
				aryEMRProblemLinkIDsToDelete.Add(nProblemLinkID);

				//the problem ID will be -1 if we're not deleting the problem the above link references,
				//will be a real ID if we do need to delete the problem
				long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
				if(nProblemID != -1) {
					//we might end up having duplicate IDs in this string, but it's just for an IN clause,
					//so it's no big deal
					aryEMRProblemIDsToDelete.Add(nProblemID);
				}

				rsLinkedProblems->MoveNext();
			}
			rsLinkedProblems->Close();

			// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
			if(!aryEMRProblemLinkIDsToDelete.IsEmpty()) {
				ExecuteParamSql("DELETE FROM EmrProblemLinkT WHERE ID IN ({INTARRAY})", aryEMRProblemLinkIDsToDelete);
			}
			if(!aryEMRProblemIDsToDelete.IsEmpty()) {
				ExecuteParamSql(
					"UPDATE EmrProblemsT SET \r\n"
					"	Deleted = 1, DeletedDate = GetDate(), DeletedBy = {STRING} \r\n"
					"WHERE ID IN ({INTARRAY})", 
					GetCurrentUserName(), aryEMRProblemIDsToDelete);
			}

			//auditing
			long nAuditID= -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientPrescDelete, GetActivePatientID(), strName, "<Deleted>", aepMedium, aetDeleted);
		}

		m_dwDeletedMedications.RemoveAll();

	}NxCatchAll("Error saving Medications.");
}

struct EmrItem
{
	bool bDefaultSelected;
	long nCategoryID;
};

void CCustomRecordDlg::TryToIncrementOfficeVisit()
{
	try {

		if(GetRemotePropertyInt("EnableEMROfficeVisitIncrementing", 0, 0, "<None>", true) != 1)
			return;

		//Loop through our list of codes, see if any of them are Office Visits.
		long p = m_BillList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		
		//We'll be using this to store all the currently visible items.
		CArray<EmrItem, EmrItem&> arItems;
		bool bArrayFilled = false;
					
		while (p) {
			m_BillList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			long nCurrentServiceID = VarLong(pRow->GetValue(CHARGE_COLUMN_SERVICE_ID));
			_RecordsetPtr rsCode = CreateRecordset("SELECT Code FROM CPTCodeT WHERE ID = %li", nCurrentServiceID);
			if(!rsCode->eof) {
				CString strCode = AdoFldString(rsCode, "Code");
				if(strCode == "99201" || strCode == "99202" || strCode == "99203" || strCode == "99204" || strCode == "99205" ||
					strCode == "99211" || strCode == "99212" || strCode == "99213" || strCode == "99214" || strCode == "99215") {
					//This is an office visit code.
					int nLevel = atoi(strCode.Mid(4,1));
					int nDefaultLevel = GetRemotePropertyInt("DefaultOfficeVisitLevel", 2, 0, "<None>", true);
					int nNewLevel = -1;
					//Starting with five, and working our way back, have we met the criteria?
					for(int i = 5; i > nLevel && i >= nDefaultLevel && nNewLevel == -1; i--) {
						if(i == nDefaultLevel) {
							//Well, if we're at the default, ALL EMRs meet the criteria.
							nNewLevel = i;
						}
						else {
							if(!bArrayFilled) {
								//Fill the array.
								for(int j=0; j<m_EMRDetailDlg.m_aryEMRItemDlgs.GetSize(); j++) {

									long nInfoID, nDataID;
									int nDatatype;
									CString strText;
									
									CCustomRecordItemDlg *pInfo = ((CCustomRecordItemDlg*)m_EMRDetailDlg.m_aryEMRItemDlgs.GetAt(j));
									if(pInfo->GetItemInfo(nInfoID,nDatatype,nDataID,strText) == ITEM_OK) {
										_RecordsetPtr rsItem = CreateRecordset("SELECT EmrInfoT.CategoryID, EmrInfoDefaultsT.EmrDataID AS DefaultData "
											"FROM EmrInfoT LEFT JOIN EmrInfoDefaultsT ON EmrInfoT.ID = EmrInfoDefaultsT.EmrInfoID "
											"WHERE EmrInfoT.ID = %li", nInfoID);
										EmrItem item;
										item.nCategoryID = AdoFldLong(rsItem, "CategoryID");
										if(nDatatype == 1) {
											item.bDefaultSelected = false;
										}
										else {
											item.bDefaultSelected = nDataID == AdoFldLong(rsItem, "DefaultData", -1);
										}
										arItems.Add(item);
									}
								}
								bArrayFilled = true;
							}
							
							int nNumberNeeded = GetRemotePropertyInt("NumberOfItems", 2*(i-1), i, "<None>", true);
							BOOL bAllowDefault = GetRemotePropertyInt("ItemType", 0, i, "<None>", true);
							
							CArray<int, int> arCategories;
							GetRemotePropertyArray("EmrCategories", arCategories, i, "<None>");
							int nValidItemsFound = 0;
							if(arCategories.GetSize() == 0) {
								//They want all categories
								for(int j = 0; j < arItems.GetSize(); j++) {
									if(!arItems.GetAt(j).bDefaultSelected || bAllowDefault) {
										nValidItemsFound++;
									}
								}
							}
							else {
								//They want specific categories.
								for(int j = 0; j < arItems.GetSize(); j++) {
									bool bValidItemFound = false;
									for(int k = 0; k < arCategories.GetSize(); k++) {
										if(arItems.GetAt(j).nCategoryID == arCategories.GetAt(k) &&
											(!arItems.GetAt(j).bDefaultSelected || bAllowDefault)) {
											bValidItemFound = true;
										}
									}
									if(bValidItemFound) nValidItemsFound++;
								}
							}
							if(nValidItemsFound >= nNumberNeeded) {
								//This EMR meets the criteria for this level!
								nNewLevel = i;
							}
						}
					}
					if(nNewLevel != -1 && nNewLevel != nLevel) {
						//Let's act just like they dropped down and selected the new code.
						CString strNewCode;
						strNewCode.Format("%s%li", strCode.Left(4), nNewLevel);
						_RecordsetPtr rsNewService = CreateRecordset("SELECT TOP 1 ServiceT.ID FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID WHERE ServiceT.Active = 1 AND CPTCodeT.COde = '%s'", _Q(strNewCode));
						if(!rsNewService->eof) {
							long nNewServiceID = AdoFldLong(rsNewService, "ID");
							pRow->PutValue(CHARGE_COLUMN_SERVICE_ID, nNewServiceID);
							OnEditingFinishedBill(pRow->GetIndex(), CHARGE_COLUMN_SERVICE_ID, _variant_t(nCurrentServiceID), _variant_t(nNewServiceID), TRUE);

							// (j.jones 2005-05-19 10:01) - PLID 16965 - save the IDs
							// but first see if perhaps we're upgrading an already upgraded code
							BOOL bFound = FALSE;
							for(int j=0;j<m_dwaryNewOfficeVisitServiceIDs.GetSize() && !bFound;j++) {
								if((long)m_dwaryNewOfficeVisitServiceIDs.GetAt(j) == nCurrentServiceID) {
									//we found it, so simply upgrade the code instead of adding new
									bFound = TRUE;
									m_dwaryNewOfficeVisitServiceIDs.SetAt(j, nNewServiceID);
								}
							}

							if(!bFound) {
								m_dwaryOldOfficeVisitServiceIDs.Add(nCurrentServiceID);
								m_dwaryNewOfficeVisitServiceIDs.Add(nNewServiceID);
							}
						}
					}
				}//End if this is an office visit
			}//End if this is a valid code
		}//End looping through rows.
	}NxCatchAll("Error in CCustomRecordDlg::TryToIncrementOfficeVisit()");

}

void CCustomRecordDlg::SynchronizeTodoAlarms()
{
	try {
		_RecordsetPtr rsApptInfo = CreateParamRecordset(
			"SELECT \r\n"
			"	NeedsFollowUp, FollowUpNumber, FollowUpUnit, FollowUpTaskID, NeedsProcedure, \r\n"
			"	ApptProcedureID, ProcedureTaskID, PatientsT.EmployeeID, PatientID, ProcedureT.Name, \r\n"
			"	EmrMasterT.Date \r\n"
			"FROM EMRMasterT \r\n"
			"INNER JOIN PatientsT ON EMRMAsterT.PatientID = PatientsT.PersonID \r\n"
			"LEFT JOIN ProcedureT ON EMRMasterT.ApptProcedureID = ProcedureT.ID \r\n"
			"WHERE EMRMasterT.ID = {INT}", m_nMasterID);
		
		long nUserID = -1;
		
		if(AdoFldBool(rsApptInfo, "NeedsFollowUp")) {
			if(rsApptInfo->Fields->GetItem("FollowUpTaskID")->Value.vt == VT_I4) {
				//They need a follow up, there is a valid ToDo alarm for it, we're good.
			}
			else {
				//We need to create a ToDo alarm for this.
				//It will be assigned to the patient's coordinator if they have one.
				_variant_t varCoordId = rsApptInfo->Fields->GetItem("EmployeeID")->Value;
				if(varCoordId.vt == VT_I4) {
					nUserID = VarLong(varCoordId);
				}
				else {
					//Well, we'll just assign this to whoever's logged in.
					nUserID = GetCurrentUserID();
				}
				CString strNotes = FormatString("Schedule Follow-up for %li %s from %s.",
					AdoFldLong(rsApptInfo, "FollowUpNumber"), 
					GetDisplayName((FollowupIncrement)AdoFldLong(rsApptInfo, "FollowUpUnit")), 
					FormatDateTimeForInterface(AdoFldDateTime(rsApptInfo, "Date"), NULL, dtoDate, true));

				// (c.haag 2008-06-09 10:40) - PLID 30321 - Use a utility function to create the todo
				long nTaskID = TodoCreate(COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), nUserID, strNotes, "", m_nMasterID, ttCustomRecord, AdoFldLong(rsApptInfo, "PatientID")); 
				ExecuteParamSql("UPDATE EMRMasterT SET FollowUpTaskID = {INT} WHERE ID = {INT}", nTaskID, m_nMasterID);
			}
		}
		else {
			if(rsApptInfo->Fields->GetItem("FollowUpTaskID")->Value.vt == VT_I4) {
				//They shouldn't have this item anymore
				long nTaskID = AdoFldLong(rsApptInfo, "FollowUpTaskID");
				ExecuteParamSql("UPDATE EMRMasterT SET FollowUpTaskID = NULL WHERE ID = {INT}", m_nMasterID);
				// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
				TodoDelete(nTaskID);
			}
			else {
				//They don't need a ToDo alarm, and they don't have one.  Fantastic.
			}
		}

		//Now, do the same thing for the procedure stuff.
		if(AdoFldBool(rsApptInfo, "NeedsProcedure")) {
			if(rsApptInfo->Fields->GetItem("ProcedureTaskID")->Value.vt == VT_I4) {
				//They need a procedure, there is a valid ToDo alarm for it, we're good.
			}
			else {
				//We need to create a ToDo alarm for this.
				if(nUserID == -1) {
					//It will be assigned to the patient's coordinator if they have one.
					_variant_t varCoordId = rsApptInfo->Fields->GetItem("EmployeeID")->Value;
					if(varCoordId.vt == VT_I4) {
						nUserID = VarLong(varCoordId);
					}
					else {
						//Well, we'll just assign this to whoever's logged in.
						nUserID = GetCurrentUserID();
					}
				}
				CString strNotes = FormatString("Schedule %s procedure.", AdoFldString(rsApptInfo, "Name"));
				// (c.haag 2008-06-09 10:40) - PLID 30321 - Use a utility function to create the todo
				long nTaskID = TodoCreate(COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), nUserID, strNotes, "", m_nMasterID, ttCustomRecord, AdoFldLong(rsApptInfo, "PatientID"));
				ExecuteParamSql("UPDATE EMRMasterT SET ProcedureTaskID = {INT} WHERE ID = {INT}", nTaskID, m_nMasterID);
			}
		}
		else {
			if(rsApptInfo->Fields->GetItem("ProcedureTaskID")->Value.vt == VT_I4) {
				//They shouldn't have this item anymore
				long nTaskID = AdoFldLong(rsApptInfo, "ProcedureTaskID");
				ExecuteParamSql("UPDATE EMRMasterT SET ProcedureTaskID = NULL WHERE ID = {INT}", m_nMasterID);
				// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
				TodoDelete(nTaskID);
			}
			else {
				//They don't need a ToDo alarm, and they don't have one.  Fantastic.
			}
		}
	}NxCatchAll("Error in CCustomRecordDlg::SynchronizeTodoAlarms()");
}

void CCustomRecordDlg::OnPrintReport() 
{
	AfxMessageBox("Before previewing, this custom record will be saved and closed.");

	if(!Save())
		return;

	CDialog::OnOK();

	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(257)];
	infReport.nExtraID = m_nMasterID;

	//check to see if there is a default report
	_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 257");
	CString strFileName;

	if (rsDefault->eof) {
		strFileName = "EMRDataByPatientIndiv";
	}
	else {
		
		long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

		if (nDefaultCustomReport > 0) {

			_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 257 AND Number = %li", nDefaultCustomReport);

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
			strFileName = "EMRDataByPatientIndiv";
		}
	}			

	////////////////////////////////////////////////////////////////////////////////////////////
	RunReport(&infReport, true, (CWnd *)this, "EMR / Op Report");
}

void CCustomRecordDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	
	if(bShow && !m_bInitialized) {
		PostMessage(ID_LOAD_NEW_EMR);
		m_bInitialized = TRUE;
	}
}


void CCustomRecordDlg::OnTrySetSelFinishedEmrProviderCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//Must be inactive.

		_RecordsetPtr rsProv = CreateParamRecordset(
			"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
			"WHERE ID = (SELECT TOP 1 ProviderID FROM EmrProvidersT WHERE EmrID = {INT} AND Deleted = 0)", m_nMasterID);

		if(!rsProv->eof) {
			m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
		} else {
			m_ProviderCombo->PutCurSel(-1);
		}

		_RecordsetPtr rsProvName;
		if(m_nMasterID == -1) {
			rsProvName = CreateParamRecordset(
				"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
				"WHERE ID = (SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT})", m_nPatID);
		}
		else {
			rsProvName = CreateParamRecordset(
				"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
				"WHERE ID = (SELECT TOP 1 ProviderID FROM EmrProvidersT WHERE EmrID = {INT} AND Deleted = 0)", m_nMasterID);
		}

		if(!rsProvName->eof) {
			m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProvName, "Name", "")));
		}
	}
}
