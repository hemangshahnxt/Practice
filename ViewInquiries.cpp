// ViewInquiries.cpp : implementation file
//

#include "stdafx.h"
#include "ViewInquiries.h"
#include "DateTimeUtils.h"
#include "InquiryDlg.h"
#include "NewPatient.h"
#include "InvUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "Datetimeutils.h"
#include "InternationalUtils.h"
#include "TodoUtils.h"
#include "HL7Utils.h"
#include "AuditTrail.h"
#include <HL7ParseUtils.h>
#include "GlobalNexWebUtils.h"
#include "HL7Utils.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PERSON_ID_COL 0
#define USER_DEF_ID_COL 1
#define LAST_COL 2
#define FIRST_COL 3
#define REFERRAL_ID_COL 5
#define PROCEDURE_COL 7


/////////////////////////////////////////////////////////////////////////////
// CViewInquiries dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker


CViewInquiries::CViewInquiries(CWnd* pParent)
	: CNxDialog(IDD, pParent)

{
	//{{AFX_DATA_INIT(CViewInquiries)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CViewInquiries::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewInquiries)
	DDX_Control(pDX, IDC_PREVIEW_INQUIRY, m_btnPreviewReport);
	DDX_Control(pDX, IDC_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_CONVERT_TO_PATIENT, m_btnConvertToPatient);
	DDX_Control(pDX, IDC_CONVERT_TO_PROSPECT, m_btnConvertToProspect);
	DDX_Control(pDX, IDC_NEW_INQUIRY, m_btnNewInquiry);
	DDX_Control(pDX, IDC_DELETE_INQUIRY, m_btnDeleteInquiry);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CViewInquiries, IDC_FROM_DATE, 2 /* Change */, OnChangeFromDate, VTS_NONE)
//	ON_EVENT(CViewInquiries, IDC_TO_DATE, 2 /* Change */, OnChangeToDate, VTS_NONE)

BEGIN_MESSAGE_MAP(CViewInquiries, CNxDialog)
	//{{AFX_MSG_MAP(CViewInquiries)
	ON_BN_CLICKED(IDC_CONVERT_TO_PATIENT, OnConvertToPatient)
	ON_BN_CLICKED(IDC_CONVERT_TO_PROSPECT, OnConvertToProspect)
	ON_BN_CLICKED(IDC_NEW_INQUIRY, OnNewInquiry)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM_DATE, OnChangeFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO_DATE, OnChangeToDate)
	ON_BN_CLICKED(IDC_DELETE_INQUIRY, OnDeleteInquiry)
	ON_BN_CLICKED(IDC_PREVIEW_INQUIRY, OnPreviewInquiry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewInquiries message handlers

BOOL CViewInquiries::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:30) - PLID 29866 - NxIconify the buttons
		m_btnConvertToPatient.AutoSet(NXB_OK);
		m_btnConvertToProspect.AutoSet(NXB_OK);
		m_btnNewInquiry.AutoSet(NXB_NEW);
		m_btnDeleteInquiry.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnPreviewReport.AutoSet(NXB_PRINT_PREV);
		
		m_dlInquiryList = BindNxDataListCtrl(IDC_INQUIRY_LIST);
		m_dlReferralFilter = BindNxDataListCtrl(IDC_REFERRAL_FILTER);
		m_dlProcedureFilter = BindNxDataListCtrl(IDC_PROCEDURE_FILTER);
		m_dlLocationFilter = BindNxDataListCtrl(IDC_INQUIRY_LOCATION_FILTER);

		IRowSettingsPtr pRow;
		pRow = m_dlReferralFilter->GetRow(-1);
		_variant_t var;
		var.vt = VT_NULL;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<Show All Referral Sources>");
		m_dlReferralFilter->InsertRow(pRow,0);
		m_dlReferralFilter->PutCurSel(0);

		pRow = m_dlProcedureFilter->GetRow(-1);
		var.vt = VT_NULL;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<Show All Procedures>");
		m_dlProcedureFilter->InsertRow(pRow,0);
		m_dlProcedureFilter->PutCurSel(0);

		pRow = m_dlLocationFilter->GetRow(-1);
		var.vt = VT_NULL;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<Show All Locations>");
		m_dlLocationFilter->InsertRow(pRow,0);
		m_dlLocationFilter->PutCurSel(0);

		COleDateTime dt = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(60,0,0,0);
		dt -= dtSpan;
		m_dtFrom.SetValue(COleVariant(dt));
		m_dtTo.SetValue(COleVariant(COleDateTime::GetCurrentTime()));

		SetFilter();

		//(e.lally 2011-07-08) PLID 43820
		g_propManager.CachePropertiesInBulk("ViewInquiry", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AssignNewPatientSecurityCode' "
			")",
			_Q(GetCurrentUserName()));
	}
	NxCatchAll("Error in CViewInquiries::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CViewInquiries::ConvertToDifferentStatus(long nPersonID, long nNewStatus)
{
	CString strNewStatus, strNewStatusAudit;
	//TES 8/13/2014 - PLID 63194 - Track the status for use in the EX tablechecker
	CClient::PatCombo_StatusType pcstStatusForTC = CClient::pcstUnchanged;
	if(nNewStatus == 1){
		strNewStatus = "patient";
		strNewStatusAudit = "Patient";
		pcstStatusForTC = CClient::pcstPatient;
	}
	else if(nNewStatus == 2){
		strNewStatus = "prospect";
		strNewStatusAudit = "Prospect";
		pcstStatusForTC = CClient::pcstPatientProspect;
	}
	
	
	/*CNewPatient dlg(NULL, IsScreen640x480() ? IDD_NEW_PATIENT_640X480 : IDD_NEW_PATIENT);
	long nUserDefID = VarLong(m_dlInquiryList->GetValue(m_dlInquiryList->GetCurSel(), USER_DEF_ID_COL));
	CString strFirst = VarString(m_dlInquiryList->GetValue(m_dlInquiryList->GetCurSel(), FIRST_COL));
	CString strLast = VarString(m_dlInquiryList->GetValue(m_dlInquiryList->GetCurSel(), LAST_COL));
	long nRefID = VarLong(m_dlInquiryList->GetValue(m_dlInquiryList->GetCurSel(), REFERRAL_ID_COL));

	dlg.m_strIDBox.Format("%li", nUserDefID);
	dlg.m_strFirstNameBox = strFirst;
	dlg.m_strLastNameBox = strLast;
	if(dlg.m_referral){
		dlg.m_referral.Select(nRefID);
	}
	dlg.DoModal(&nPersonID);*/

	CAuditTransaction auditTrans;
	
	CString strMsg;
	strMsg.Format("Are you sure you wish to change the status of this person?");
	if(MessageBox(strMsg, "Patient Inquiries", MB_YESNO) == IDYES){
		long nID = NewNumber("PatientStatusHistoryT", "ID");
		if(nID > 0){
			//(e.lally 2011-07-08) PLID 43820 - Create a security code when changing status
			bool bCreateSecurityCode = false;
			if(g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent)) {
				// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Always (1)
				if(GetRemotePropertyInt("AssignNewPatientSecurityCode", 1, 0, "<None>", true) != 0){
					bCreateSecurityCode = true;
				}
			}
			// change the status
			// enter a record into PatientStatusHistoryT to track when the patient was converted
			// 1 = patient, 2 = prospect, 3 = patient/prospect, 4 = Inquiry
			//(e.lally 2009-03-11) PLID 29895 - Put this in a parameterized query
			ExecuteParamSql("UPDATE PatientsT SET CurrentStatus = {INT} WHERE PersonID = {INT} "
			"INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
			"VALUES ({INT}, {INT}, 4, {INT}, {STRING}, {STRING})", 
			nNewStatus, nPersonID,
			nID, nPersonID, nNewStatus, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());

			//(e.lally 2009-03-11) PLID 29895 - Audit the status change
			AuditEvent(nPersonID, GetExistingPatientName(nPersonID), auditTrans, aeiCurrentStatus, nPersonID, "Inquiry", strNewStatusAudit, aepMedium, aetChanged);

			//(e.lally 2011-07-08) PLID 43820 - Create a security code if the pref and license say to do so for new patients/prospects
			if(bCreateSecurityCode && nNewStatus != 4){
				CString strSecurityCode = EnsurePatientSecurityCode(nPersonID, GetRemoteData());
				// (d.singleton 2011-10-11 11:31) - PLID 42102 - add auditing for security codes.
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), BeginNewAuditEvent(), aeiPatientSecurityCode, nPersonID, "", strSecurityCode, aepHigh, aetChanged);
			}

			//Now send the table checker
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(nPersonID, false, CClient::pcatActive, pcstStatusForTC);

			// (z.manning 2011-08-05 12:10) - PLID 40872 - We need to send this patient to HL7 as well. Since inquries are
			// never sent via HL7 this is a new patient as far as HL7 is concerned.
			UpdateExistingPatientInHL7(nPersonID, FALSE, TRUE);
		}
		else{
			return FALSE;
		}
	}
	else{
		return FALSE;
	}

	// (m.cable 2004-05-27 10:24) we made it this far, if they are no longer an inquiry, then we need to activate their
	// tracking ladders
	// All of the ladders for this person so far should be inactive because when the inquiry was created, 
	// there is code there to inactive the ladders for that person
	_RecordsetPtr rsLadders = CreateRecordset("SELECT ID FROM LaddersT WHERE PersonID = %li", nPersonID);
	// go through all of the ladders and set them to be Active
	while(!rsLadders->eof){
		long nLadderID = AdoFldLong(rsLadders, "ID");
		ExecuteSql("UPDATE LaddersT SET Status = %li WHERE ID = %li", 1 /*Active*/,nLadderID);
		PhaseTracking::SyncTodoWithLadder(nLadderID);
		rsLadders->MoveNext();
	}

	auditTrans.Commit();
	return TRUE;
}

void CViewInquiries::OnConvertToPatient() 
{
	long nCurrentInquiry = m_dlInquiryList->GetCurSel();
	if(nCurrentInquiry  != sriNoRow){
		long nPatientID = m_dlInquiryList->GetValue(nCurrentInquiry, 0);
		if(ConvertToDifferentStatus(nPatientID, 1)){
			EndDialog(1);
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == AfxMessageBox(
						"The patient was converted correctly, but this patient is not in the current lookup.  If you proceed, Practice will "
						"clear the lookup before switching to the Patients module.\r\n\r\n"
						"Would you like to clear the lookup and proceed?", MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - No need to check for failure, inquiries can't have Security Groups.
				pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID);
				
				pMainFrame->UpdateAllViews();				

				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
			}
		}
	}
}

void CViewInquiries::OnConvertToProspect() 
{
	long nCurrentInquiry = m_dlInquiryList->GetCurSel();
	if(nCurrentInquiry  != sriNoRow){
		long nPatientID = m_dlInquiryList->GetValue(nCurrentInquiry, 0);
		if(ConvertToDifferentStatus(nPatientID, 2)){
			EndDialog(2);
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == AfxMessageBox(
						"The patient was converted correctly, but this patient is not in the current lookup.  If you proceed, Practice will "
						"clear the lookup before switching to the Patients module.\r\n\r\n"
						"Would you like to clear the lookup and proceed?", MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - No need to check for failure, inquiries can't have Security Groups.
				pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID);
				
				pMainFrame->UpdateAllViews();				

				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
			}
		}
	}
}

BEGIN_EVENTSINK_MAP(CViewInquiries, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CViewInquiries)
	ON_EVENT(CViewInquiries, IDC_INQUIRY_LIST, 6 /* RButtonDown */, OnRButtonDownInquiryList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CViewInquiries, IDC_PROCEDURE_FILTER, 16 /* SelChosen */, OnSelChosenProcedureFilter, VTS_I4)
	ON_EVENT(CViewInquiries, IDC_REFERRAL_FILTER, 16 /* SelChosen */, OnSelChosenReferralFilter, VTS_I4)
	ON_EVENT(CViewInquiries, IDC_INQUIRY_LOCATION_FILTER, 16 /* SelChosen */, OnSelChosenInquiryLocationFilter, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CViewInquiries::OnRButtonDownInquiryList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_dlInquiryList->CurSel = nRow;

	if(nRow==-1)
		return;

	try {

		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();

		pMenu->InsertMenu(-1, MF_BYPOSITION, IDC_CONVERT_TO_PATIENT, "Convert to Patient and Edit");
		pMenu->InsertMenu(-1, MF_BYPOSITION, IDC_CONVERT_TO_PROSPECT, "Convert to Prospect and Edit");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	
	}NxCatchAll("Error in OnRButtonDownCaseHistoryList");
}	


void CViewInquiries::OnOK() 
{
	CDialog::OnOK();
	GetMainFrame()->m_patToolBar.Requery();
}

void CViewInquiries::OnCancel()
{
	CDialog::OnCancel();
	GetMainFrame()->m_patToolBar.Requery();
}

void CViewInquiries::OnNewInquiry() 
{
	CInquiryDlg dlg(this);
	dlg.DoModal();
	m_dlInquiryList->Requery();
	m_dlInquiryList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	
	FillProcedureColumn();

}

// (j.gruber 2008-06-04 12:19) - PLID 25447 - added report
void CViewInquiries::SetFilter()
{
	
	// first find the ID of the new referral filter
	long nRefID = VarLong(m_dlReferralFilter->GetValue(m_dlReferralFilter->GetCurSel(), 0), -1);
	// then the From Date
	COleDateTime dtFrom = m_dtFrom.GetValue();
	// then the To Date
	COleDateTime dtTo = m_dtTo.GetValue();
	// finally the procedure filter
	long nProcID = VarLong(m_dlProcedureFilter->GetValue(m_dlProcedureFilter->GetCurSel(), 0), -1);
	// finally the location filter
	long nLocationID = VarLong(m_dlLocationFilter->GetValue(m_dlLocationFilter->GetCurSel(), 0), -1);
	
	// now set the where clause of the list so that only those with this referral ID are shown
	CString strWhere;
	
	//Date Range
	CString dateFrom, dateTo, dateClause;
	dateFrom = FormatDateTimeForSql(COleDateTime(m_dtFrom.GetValue()), dtoDate);
	dtTo = m_dtTo.GetValue().date;
	dateTo = FormatDateTimeForSql(dtTo, dtoDate);
		
	if(nRefID >= 0){
		// they chose a referral to filter on
		strWhere.Format("CurrentStatus = 4  "
					"AND ReferralID = %li AND (Convert(datetime, Convert(varchar, INputDate, 1)) >= Convert(datetime,'%s')) AND "
					"(Convert(datetime, Convert(varchar, INputDate, 1)) <= Convert(datetime,'%s'))", 
					nRefID, dateFrom, dateTo);
	}
	else{
		// they want to view all referrals
		strWhere.Format("CurrentStatus = 4 "
					"AND (Convert(datetime, Convert(varchar, INputDate, 1)) >= Convert(datetime,'%s')) AND "
					"(Convert(datetime, Convert(varchar, INputDate, 1)) <= Convert(datetime,'%s'))",		
					dateFrom, dateTo);
	}

	if(nProcID >= 0){
		// they chose a procedure to filter on
		CString strWherePart2;
		strWherePart2.Format(" AND (PersonID IN (SELECT PatientID AS PersonID FROM ProcInfoT "
					"LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID "
					"WHERE ProcedureID = %li))", nProcID);
		strWhere += strWherePart2;
	}

	if(nLocationID >= 0){
		// they chose a procedure to filter on
		CString strWherePart3;
		strWherePart3.Format(" AND (Location = %li)", nLocationID);
		strWhere += strWherePart3;
	}

	m_dlInquiryList->PutWhereClause(_bstr_t(strWhere));
	m_dlInquiryList->Requery();
	m_dlInquiryList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	FillProcedureColumn();
}

void CViewInquiries::OnChangeFromDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SetFilter();

	*pResult = 0;
}

void CViewInquiries::OnChangeToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetFilter();
	
	*pResult = 0;
}

void CViewInquiries::OnDeleteInquiry() 
{
	if(m_dlInquiryList->GetCurSel() == sriNoRow || IDYES != MessageBox("Are you sure you want to delete this inquiry?", "Delete Inquiry", MB_YESNO)){
		return;
	}

	// (c.haag 2008-02-29 15:29) - PLID 29115 - Support for inventory todo alarm transactions.
	int nInvTodoTransactionID = -1;
	
	// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
	try {
		CSqlTransaction trans("Delete Inquiry");
		trans.Begin();

		long ID = m_dlInquiryList->GetValue(m_dlInquiryList->GetCurSel(), PERSON_ID_COL);
		long tempID = -1;

		// (c.haag 2008-02-29 15:29) - PLID 29115 - Gather all the product ID's and location ID's that we can now,
		// because the allocations are actually being deleted rather than being marked as deleted.
		nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
		_RecordsetPtr prsInvTodos = CreateRecordset(
			"SELECT ProductID, LocationID FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"WHERE PatientInvAllocationsT.PatientID = %d",
			ID);
		while (!prsInvTodos->eof) {
			const long nProductID = AdoFldLong(prsInvTodos, "ProductID", -1);
			const long nLocationID = AdoFldLong(prsInvTodos, "LocationID", -1);
			InvUtils::AddProductLocToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, nProductID, nLocationID);
			prsInvTodos->MoveNext();
		}
		prsInvTodos->Close();

		ExecuteSql("UPDATE MailSent SET PicID = NULL WHERE PicID IN (SELECT ID FROM PicT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li) OR EmrGroupID IN (SELECT ID FROM EmrGroupsT WHERE PatientID = %li))", ID, ID);
		// (r.galicki 2008-10-07 14:44) - PLID 31555 - Constraint on LabsT.PicID
		ExecuteSql("UPDATE LabsT SET PicID = NULL WHERE PicID IN (SELECT ID FROM PicT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li) OR EmrGroupID IN (SELECT ID FROM EmrGroupsT WHERE PatientID = %li))", ID, ID);
		// (c.haag 2010-07-20 17:42) - PLID 30894 - Because we updated Lab PicID's, we must fire a table checker
		// (r.gonet 09/02/2014) - PLID 63221 - Send the person ID too.
		CClient::RefreshLabsTable(ID, -1);
		ExecuteSql("DELETE FROM PicT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li) OR EmrGroupID IN (SELECT ID FROM EmrGroupsT WHERE PatientID = %li)", ID, ID);

		ExecuteSql("DELETE FROM EventAppliesT WHERE EventID IN (SELECT ID FROM EventsT WHERE PatientID = %li)", ID);

		ExecuteSql("DELETE FROM EventsT WHERE PatientID = %li", ID);

		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM LaddersT WHERE PersonID = {INT}",ID);
		while(!rs->eof) {
			tempID = rs->Fields->Item["ID"]->Value.lVal;
			// (j.jones 2008-11-26 14:27) - PLID 30830 - delete from StepsAssignToT
			ExecuteParamSql("DELETE FROM StepsAssignToT WHERE StepID IN (SELECT ID FROM StepsT WHERE LadderID = {INT})", tempID);
			ExecuteParamSql("DELETE FROM StepsT WHERE LadderID = {INT}",tempID);
			rs->MoveNext();
		}			
		rs->Close();

		ExecuteSql("DELETE FROM LaddersT WHERE PersonID = %li",ID);
		ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		ExecuteSql("DELETE FROM ProcInfoAppointmentsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		ExecuteSql("DELETE FROM ProcInfoMedicationsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		ExecuteSql("DELETE FROM ProcInfoQuotesT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		ExecuteSql("DELETE FROM ProcInfoPaymentsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		// (j.dinatale 2012-07-11 09:42) - PLID 51474 - handle ProcInfoBillsT when deleting an inquiry
		ExecuteSql("DELETE FROM ProcInfoBillsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %li)", ID);
		ExecuteSql("DELETE FROM ProcInfoT WHERE PatientID = %li", ID);
		ExecuteSql("DELETE FROM MultiReferralsT WHERE PatientID = %li", ID);

		// (j.jones 2007-11-07 10:09) - PLID 27987 - it should be impossible
		// for an inventory allocation to be linked to an inquiry, but better safe than sorry

		// (j.jones 2008-02-27 10:20) - PLID 29102 - delete case history links too
		ExecuteParamSql("DELETE FROM CaseHistoryAllocationLinkT WHERE "
			"AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE PatientID = {INT})", ID);

		// (j.jones 2008-03-24 16:55) - PLID 29388 - don't need to send tablecheckers for appointments
		// since the patient and appointments are being deleted
		ExecuteParamSql("DELETE FROM PatientInvAllocationDetailsT WHERE AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE PatientID = {INT})", ID);
		ExecuteParamSql("DELETE FROM PatientInvAllocationsT WHERE PatientID = {INT}", ID);
		
		// (j.gruber 2014-12-16 11:03) - PLID 64393 - Rescheduling Queue - moved this up to take care of notes before appointments
		// (r.gonet 07/21/2014) - PLID 62525 - Clear the reference a bill can have to a note for the Bill Status Note. Don't think it is possible for inquiries to have bills anyway.
		ExecuteParamSql("UPDATE BillsT SET StatusNoteID = NULL WHERE BillsT.PatientID = {INT}", ID);

		// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
		ExecuteSql("DELETE FROM NoteDataT WHERE PersonID = %li", ID);

		_RecordsetPtr rsAppts = CreateRecordset("SELECT ID FROM AppointmentsT WHERE PatientID = %li", ID);
		while(!rsAppts->eof){
			// MSC 5-12-2004 - As of now this shouldn't happen, if we allow inquiries to have appts in the future
			// then we can take this assert out
			ASSERT(FALSE);
			long nCurApptID = AdoFldLong(rsAppts, "ID");

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54108 - Changed to use refactored send function.
			SendCancelAppointmentHL7Message(nCurApptID);

			ExecuteSql("DELETE FROM RoomAppointmentHistoryT WHERE RoomAppointmentID IN (SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = %li)", nCurApptID);
			ExecuteSql("DELETE FROM RoomAppointmentsT WHERE AppointmentID = %li", nCurApptID);
			ExecuteSql("DELETE FROM AppointmentPurposeT WHERE AppointmentID = %li", nCurApptID);			
			ExecuteSql("DELETE FROM AppointmentResourceT WHERE AppointmentID = %li", nCurApptID);
			// (j.gruber 2012-08-07 13:25) - PLID 51869 - remove insurances
			ExecuteParamSql("DELETE FROM AppointmentInsuredPartyT WHERE AppointmentID = {INT}", nCurApptID);
			ExecuteSql("DELETE FROM AptLinkT WHERE AppointmentID = %li", nCurApptID);
			ExecuteSql("DELETE FROM AptShowStateHistoryT WHERE AppointmentID = %li", nCurApptID);
			// (j.jones 2008-03-18 15:48) - PLID 29309 - delete links to orders
			// (j.jones 2008-03-19 12:21) - PLID 29316 - no need to audit the link deletion when the inquiry is deleted (this isn't possible anyways)
			ExecuteParamSql("DELETE FROM OrderAppointmentsT WHERE AppointmentID = {INT}", nCurApptID);
			// (c.haag 2009-10-12 13:08) - PLID 35722 - Ensure there are no references to MailSentProcedureT (not that there should be any at all for an inquiry)
			ExecuteParamSql("UPDATE MailSentProcedureT SET AppointmentsID = NULL WHERE AppointmentsID = {INT}", nCurApptID);
			// (j.jones 2009-12-09 13:22) - PLID 36137 - delete from the TOPS history
			ExecuteParamSql("DELETE FROM TOPSSubmissionHistoryT WHERE AppointmentID = {INT}", nCurApptID);
			// (z.manning 2010-04-05 09:31) - PLID 24607 - AppointmentRemindersT 
			ExecuteParamSql("DELETE FROM AppointmentRemindersT WHERE AppointmentID = {INT}", nCurApptID);
			// (j.jones 2011-10-07 15:52) - PLID 37659 - clear references in EligibilityRequestsT, though no inquiries should have any
			ExecuteParamSql("UPDATE EligibilityRequestsT SET AppointmentID = NULL WHERE AppointmentID = {INT}", nCurApptID);
			// (z.manning 2010-07-14 10:37) - PLID 39422 - Delete an hl7 appt links
			ExecuteSql("DELETE FROM HL7CodeLinkT WHERE Type = %li AND PracticeID = %li"
				, hclrtAppointment, nCurApptID);			
			// (j.jones 2014-11-26 16:09) - PLID 64272 - remove entries from AppointmentMixRuleOverridesT
			ExecuteParamSql("DELETE FROM AppointmentMixRuleOverridesT WHERE AppointmentID = {INT}", nCurApptID);
			ExecuteParamSql("DELETE FROM AppointmentsT WHERE ID = {INT}", nCurApptID);
			// (a.walling 2015-02-06 11:38) - PLID 64364 - Rescheduling Queue, delete here why not though it is not supposed to ever happen
			ExecuteParamSql("DELETE FROM ReschedulingQueueT WHERE AppointmentID = {INT}", nCurApptID);
			rsAppts->MoveNext();
		}

		
	
		//delete all items for this patient from the ToDO List
		// (c.haag 2008-06-09 17:16) - PLID 30328 - Use global utilities to delete todos
		TodoDelete(FormatString("PersonID = %li", ID));
		// (j.jones 2006-12-13 14:19) - PLID 23854 - fixed bug where the inquiry may be in a group
		ExecuteSql("DELETE FROM GroupDetailsT WHERE PersonID = %li", ID);

		// (c.haag 2009-10-12 14:56) - PLID 35923 - Delete patient wellness information. I don't think it was expected
		// that this code needed to be called, but at a minimum, WellnessPatientQualificationT can be filled in.
		ExecuteParamSql("DELETE FROM WellnessPatientQualificationT WHERE PatientID = {INT}", ID);
		ExecuteParamSql("DELETE FROM PatientWellnessCompletionItemT WHERE PatientWellnessID IN "
			"(SELECT ID FROM PatientWellnessT WHERE PatientID = {INT})", ID);
		ExecuteParamSql("DELETE FROM PatientWellnessCriterionT WHERE PatientWellnessID IN "
			"(SELECT ID FROM PatientWellnessT WHERE PatientID = {INT})", ID);
		ExecuteParamSql("DELETE FROM PatientWellnessT WHERE PatientID = {INT}", ID);
		ExecuteParamSql("DELETE FROM WellnessTemplateCompletionItemT WHERE WellnessTemplateID IN "
			"(SELECT ID FROM WellnessTemplateT WHERE SpecificToPatientID = {INT})", ID);
		ExecuteParamSql("DELETE FROM WellnessTemplateCriterionT WHERE WellnessTemplateID IN "
			"(SELECT ID FROM WellnessTemplateT WHERE SpecificToPatientID = {INT})", ID);
		ExecuteParamSql("DELETE FROM WellnessTemplateT WHERE SpecificToPatientID = {INT}", ID);

		// (b.spivey, June 10, 2013) - PLID 55937 - delete patient races. 
		ExecuteParamSql("DELETE FROM PersonRaceT WHERE PersonID = {INT}", ID); 

		DeletePerson(ID,DP_ALL);

		CClient::RefreshTable(NetUtils::TodoList, -1);

		m_dlInquiryList->Requery();

		// (c.haag 2008-02-29 15:29) - PLID 29115 - Update inventory todo alarms.
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, true);

		trans.Commit();
	}NxCatchAllCall(__FUNCTION__, 
		if (-1 != nInvTodoTransactionID) { 
			InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
		}
	);
}

void CViewInquiries::FillProcedureColumn()
{
	CString strCurWhere = VarString(m_dlInquiryList->GetWhereClause());
	// (d.moore 2007-06-11 16:54) - PLID 14670 - Altered the query to use the 
	//  ProcedureLadderTemplateT table, this makes it possible to look for procedures
	//  with multiple ladders associated with them.
	CString strSql;
	strSql.Format("SELECT PatientID, Name FROM ProcInfoT "
		"LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID "
		"LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID "
		"LEFT JOIN PersonT ON PersonT.ID = ProcInfoT.PatientID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE MasterProcedureID Is Null AND "
		"ProcedureT.ID IN "
			"(SELECT DISTINCT ProcedureID FROM ProcedureLadderTemplateT "
			"INNER JOIN LadderTemplatesT ON ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID) "
		"AND (%s)", strCurWhere);

	_RecordsetPtr rsAllProcedures = CreateRecordset(strSql);
		
	long nPtID;
		
	while(!rsAllProcedures->eof){
		nPtID = AdoFldLong(rsAllProcedures, "PatientID");
		long nRow = m_dlInquiryList->FindByColumn(PERSON_ID_COL, _variant_t(nPtID), -1, FALSE);
		if(nRow >= 0){
			// these are the existing procedures
			CString strProcList = VarString(m_dlInquiryList->GetValue(nRow, PROCEDURE_COL), "");
			if(strProcList != ""){
				strProcList += ", ";
			}
			// this is the next record we're adding
			strProcList += AdoFldString(rsAllProcedures, "Name") ;
			m_dlInquiryList->PutValue(nRow, PROCEDURE_COL, _variant_t(strProcList));
		}
		rsAllProcedures->MoveNext();
	}
}

void CViewInquiries::OnSelChosenProcedureFilter(long nRow) 
{
	SetFilter();	
}

void CViewInquiries::OnSelChosenReferralFilter(long nRow) 
{
	SetFilter();
	
}

void CViewInquiries::OnSelChosenInquiryLocationFilter(long nRow) 
{
	SetFilter();	
}

CString CViewInquiries::GetFilter() 
{
	try {

		// first find the ID of the new referral filter
		long nRefID = VarLong(m_dlReferralFilter->GetValue(m_dlReferralFilter->GetCurSel(), 0), -1);
		// then the From Date
		COleDateTime dtFrom = m_dtFrom.GetValue();
		// then the To Date
		COleDateTime dtTo = m_dtTo.GetValue();
		// finally the procedure filter
		long nProcID = VarLong(m_dlProcedureFilter->GetValue(m_dlProcedureFilter->GetCurSel(), 0), -1);
		// finally the location filter
		long nLocationID = VarLong(m_dlLocationFilter->GetValue(m_dlLocationFilter->GetCurSel(), 0), -1);
		
		// now set the where clause of the list so that only those with this referral ID are shown
		CString strWhere;
		
		//Date Range
		CString dateFrom, dateTo, dateClause;
		dateFrom = FormatDateTimeForSql(COleDateTime(m_dtFrom.GetValue()), dtoDate);
		dtTo = m_dtTo.GetValue().date;
		dateTo = FormatDateTimeForSql(dtTo, dtoDate);
			
		if(nRefID >= 0){
			// they chose a referral to filter on
			strWhere.Format("CurrentStatus = 4  "
						"AND ReferralID = %li AND (Convert(datetime, Convert(varchar, INputDate, 1)) >= Convert(datetime,'%s')) AND "
						"(Convert(datetime, Convert(varchar, INputDate, 1)) <= Convert(datetime,'%s'))", 
						nRefID, dateFrom, dateTo);
		}
		else{
			// they want to view all referrals
			strWhere.Format("CurrentStatus = 4 "
						"AND (Convert(datetime, Convert(varchar, INputDate, 1)) >= Convert(datetime,'%s')) AND "
						"(Convert(datetime, Convert(varchar, INputDate, 1)) <= Convert(datetime,'%s'))",		
						dateFrom, dateTo);
		}

		if(nProcID >= 0){
			// they chose a procedure to filter on
			CString strWherePart2;
			strWherePart2.Format(" AND (PatientsT.PersonID IN (SELECT PatientID AS PersonID FROM ProcInfoT "
						"LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID "
						"WHERE ProcedureID = %li))", nProcID);
			strWhere += strWherePart2;
		}

		if(nLocationID >= 0){
			// they chose a procedure to filter on
			CString strWherePart3;
			strWherePart3.Format(" AND (PersonT.Location = %li)", nLocationID);
			strWhere += strWherePart3;
		}

			

		return strWhere;


	}NxCatchAll("Error In CViewInquiries::GetFilter");

	return "";

}


void CViewInquiries::OnPreviewInquiry() 
{
	try {

		CString strFilter = GetFilter();

		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(627)]);

		CPtrArray paParams;
		CRParameterInfo *paramInfo;
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = FormatDateTimeForInterface(m_dtFrom.GetValue(), NULL, dtoDate, FALSE);
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = FormatDateTimeForInterface(m_dtTo.GetValue(), NULL, dtoDate, FALSE);
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo);


		infReport.strExtraField = " AND " + strFilter;

		RunReport(&infReport, &paParams, TRUE, this, "Inquiries Report");
		ClearRPIParameterList(&paParams);

		OnOK();

	}NxCatchAll("Error Previewing Inquiry Report");
	
	
}
