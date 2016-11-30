// NexWebImportDlg.cpp : implementation file
//
 
#include "stdafx.h"
#include "practice.h"
#include "NexWebImportDlg.h"
#include "NexWebEditObjectsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "NexWebUtils.h"
#include "GlobalNexWebUtils.h"
#include "duplicate.h"
#include <afxinet.h>
#include "WellnessDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ID_EDIT_NEXWEB_OBJECT	49101

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CNexWebImportDlg dialog

//(e.lally 2009-04-30) PLID 34123 - Removed references to the depreciated NexWebFTPSettingsDlg

CNexWebImportDlg::CNexWebImportDlg(CWnd* pParent)
	: CNxDialog(CNexWebImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebImportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexWebImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebImportDlg)
	DDX_Control(pDX, IDCANCEL, m_Exit);
	DDX_Control(pDX, IDC_SAVE, m_Save);
	DDX_Control(pDX, IDC_IMPORT, m_ImportAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebImportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebImportDlg)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebImportDlg message handlers



void CNexWebImportDlg::OnImport() 
{
	try {
		// (b.cardillo 2007-05-22 17:01) - PLID 26026 - Safely move all transactions from the live source 
		// table over to our working transaction table.
		ExecuteSqlStd(
			// Lock both the source and the destination tables
			"BEGIN TRAN \r\n"
			"SELECT COUNT(*) FROM NexWebTransactionsT WITH(TABLOCKX) \r\n"
			"SELECT COUNT(*) FROM NexWebTransactionSourceT WITH(TABLOCKX) \r\n"
			"\r\n"
			// Copy everything from the source to the destination, adjusting all negative object IDs and 
			// PersonIDs to be unique in our table
			"DECLARE @nUniqueImaginaryOffset INT \r\n"
			"SET @nUniqueImaginaryOffset = COALESCE((SELECT MIN(ObjectID) FROM NexWebTransactionsT WHERE ObjectID < 0), -100) \r\n"
			"INSERT INTO NexWebTransactionsT (TransType, Field, Value, PersonID, ObjectID) \r\n"
			" SELECT TransType, Field, Value, \r\n"
			"  CASE WHEN PersonID < 0 THEN PersonID + @nUniqueImaginaryOffset ELSE PersonID END AS PersonID, \r\n"
			"  CASE WHEN ObjectID < 0 THEN ObjectID + @nUniqueImaginaryOffset ELSE ObjectID END AS ObjectID \r\n"
			" FROM NexWebTransactionSourceT \r\n"
			"\r\n"
			// Good, we're finished with all source records, we can now delete them.
			"DELETE FROM NexWebTransactionSourceT \r\n"
			"\r\n"
			// Since we just deleted all source records, we should reseed the identity on the source table
			"DBCC CHECKIDENT (NexWebTransactionSourceT, RESEED, 0) \r\n"
			"DBCC CHECKIDENT (NexWebTransactionSourceT, RESEED) \r\n"
			"\r\n"
			// And we're done
			"COMMIT TRAN\r\n");


		// Okay, we now have all the records in our local table so load it on screen
		LoadTransactionRecords();
	} NxCatchAll("CNexWebImportDlg::OnImport");
	
}


void CNexWebImportDlg::LoadTransactionRecords() 
{

	try {
		m_pImportList->Clear();

		//First, load all the records that have just been changed
		long nNewUserDefinedID = NewNumber("PatientsT", "UserDefinedID");
		_RecordsetPtr rsImport = CreateRecordset("SELECT ID, UserDefinedID, First, Last, Middle, 0 AS IsNewPatient FROM PersonT INNER JOIN PatientsT ON "
			" PersonT.ID = PatientsT.PersonID WHERE PersonT.ID IN (SELECT PersonID FROM NexWebTransActionsT) AND PersonT.ID NOT IN (SELECT PersonID FROM NexWebTransactionsT WHERE TransType = 'Added' AND Field = %li)"
			" UNION "
			" SELECT PersonID AS ID, %li AS UserDefinedID, "
			" (SELECT Max(Value) FROM NexWebTransActionsT AS FirstNameT WHERE Field = %li AND FirstNameT.PersonID = NexwebTransactionsT.PersonID ) AS First, "
			" (SELECT Max(Value) FROM NexWebTransActionsT AS LastNameT WHERE Field = %li AND LastNameT.PersonID = NexwebTransactionsT.PersonID ) AS Last,  "
			" (SELECT Max(Value) FROM NexWebTransActionsT AS MiddleNameT WHERE Field = %li AND MiddleNameT.PersonID = NexwebTransactionsT.PersonID ) AS Middle,  "
			" 1 as IsNewPatient "
			" FROM NexWebTransActionsT WHERE TransType = 'Added' AND Field = %li ", 		
			 transTypePatient, nNewUserDefinedID, transTypeFirstName, transTypeLastName, transTypeMiddleName, transTypePatient);

		IRowSettingsPtr pRow;
		//load these in first
		while (! rsImport->eof) {

			pRow = m_pImportList->GetRow(-1);

			pRow->PutValue(0, AdoFldLong(rsImport, "ID"));
			pRow->PutValue(1, nNewUserDefinedID);
			pRow->PutValue(2, _variant_t(AdoFldString(rsImport, "First", "")));
			pRow->PutValue(3, _variant_t(AdoFldString(rsImport, "Middle", "")));
			pRow->PutValue(4, _variant_t(AdoFldString(rsImport, "Last", "")));
			pRow->PutValue(5, _variant_t( "..."));
			pRow->PutValue(6, AdoFldLong(rsImport, "IsNewPatient"));
			pRow->PutCellLinkStyle(5, dlLinkStyleTrue);

			m_pImportList->AddRow(pRow);

			nNewUserDefinedID++;

			rsImport->MoveNext();
		}
		
	}NxCatchAll("Error Loading Records");

}

BOOL CNexWebImportDlg::OnInitDialog() 
{
	// (e.lally 2009-01-26) PLID 32813 - Added try/catch
	try{
		CNxDialog::OnInitDialog();

		m_Exit.AutoSet(NXB_CLOSE);
		
		m_pImportList = BindNxDataListCtrl(IDC_NEXWEB_IMPORT_LIST, false);

		//(e.lally 2008-02-29) PLID 27379- This is only used with the save data button, but it should be initialized for safety.
		m_bContinueMassImport = TRUE;

		// (e.lally 2009-01-26) PLID 32813 - Bulk cache preferences
		g_propManager.CachePropertiesInBulk("NexWebImport-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AssignNewPatientSecurityCode' OR "
			// (j.jones 2010-01-15 09:03) - PLID 31927 - added NewPatientsDefaultTextMessagePrivacy
			"Name = 'NewPatientsDefaultTextMessagePrivacy' "
			")",
			_Q(GetCurrentUserName()));


		// (b.cardillo 2007-05-22 17:12) - PLID 26026 - Now that we no longer download through ftp, we can just 
		// pull everything in at start-up.
		OnImport();
	}NxCatchAll("Error loading NexWebImport screen")
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CNexWebImportDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebImportDlg)
	ON_EVENT(CNexWebImportDlg, IDC_NEXWEB_IMPORT_LIST, 5 /* LButtonUp */, OnLButtonUpNexwebImportList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexWebImportDlg, IDC_NEXWEB_IMPORT_LIST, 3 /* DblClickCell */, OnDblClickCellNexwebImportList, VTS_I4 VTS_I2)
	ON_EVENT(CNexWebImportDlg, IDC_NEXWEB_IMPORT_LIST, 7 /* RButtonUp */, OnRButtonUpNexwebImportList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebImportDlg::OnLButtonUpNexwebImportList(long nRow, short nCol, long x, long y, long nFlags) 
{
	//(e.lally 2008-02-28) PLID 29142 - Add error handling
	try{
		if (nCol == 5) {

			//Check if no row was passed in.
			if(nRow == sriNoRow ){
				return;
			}

			//(e.lally 2008-02-28) PLID 29142 - Use general function for launching editor.
			long nResult = EditImportListObject(nRow);

			switch (nResult) {
				
				case IDCANCEL:
					//previewing, close this window too
					OnCancel();
				break;

				default:
					//at the moment, just requery the list
					m_pImportList->Clear();
					LoadTransactionRecords();
				break;
			}

		}
	}NxCatchAll("Error editing NexWeb Import object in OnLButtonUpNexwebImportList");
}

//(e.lally 2008-02-28) PLID 29142 - Open the editor from a centralized function
long CNexWebImportDlg::EditImportListObject(long nRow)
{
	//Caller should check for no row selected, but we will assert it here
	ASSERT(nRow != sriNoRow);

	//get the personID out of the datalist to use in the query
	long nPersonID = VarLong(m_pImportList->GetValue(nRow, 0));
	BOOL bIsNewPatient = VarLong(m_pImportList->GetValue(nRow, 6));

	//open up the editing dialog
	CNexWebEditObjectsDlg dlg(nPersonID, bIsNewPatient, this);

	long nResult = dlg.DoModal();

	return nResult;
}


void CNexWebImportDlg::OnCancel() {
	CDialog::OnCancel();
}


void CNexWebImportDlg::RemoveTransactionFromList(long nPersonID) {

	long nRow = m_pImportList->FindByColumn(0, nPersonID, 0, FALSE);

	m_pImportList->RemoveRow(nRow);


}

BOOL CNexWebImportDlg::AppointmentInfoExists() {

	try {
		m_strError = "";
		//loop through the list, check NexWebTransactionsT to see if any appt info exists for the patients
		BOOL bApptsExist = FALSE;
		long p = m_pImportList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		CString strPatName;
		while (p) {
			m_pImportList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			long nPersonID = VarLong(pRow->GetValue(0));
			_RecordsetPtr rs = CreateRecordset("SELECT ID FROM NexwebTransactionsT WHERE PersonID = %li "
				" AND Field > 3000 AND Field < 4000 ", nPersonID);
			if (!rs->eof) {
				//add them to the list
				strPatName.Format("%li - %s %s \n", 
				VarLong(pRow->GetValue(1)),
				VarString(pRow->GetValue(2)),
				VarString(pRow->GetValue(4)));
				m_strError += strPatName;
				bApptsExist = TRUE;
			}
			rs->Close();

		}

		return bApptsExist;
	}NxCatchAll("Error in CNexWebImportDlg::AppointmentInfoExists()");

	//if we got here, we failed, so don't let them import
	return TRUE;

}


BOOL CNexWebImportDlg::PatientNotesExists() {

	try {
		m_strError = "";
		//loop through the list, check NexWebTransactionsT to see if any appt info exists for the patients
		BOOL bNotesExist = FALSE;
		long p = m_pImportList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		CString strPatName;
		while (p) {
			m_pImportList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			long nPersonID = VarLong(pRow->GetValue(0));
			_RecordsetPtr rs = CreateRecordset("SELECT ID FROM NexwebTransactionsT WHERE PersonID = %li "
				" AND Field IN (2042,2043,2044) ", nPersonID);
			if (!rs->eof) {
				//add them to the list
				strPatName.Format("%li - %s %s \n", 
				VarLong(pRow->GetValue(1)),
				VarString(pRow->GetValue(2)),
				VarString(pRow->GetValue(4)));
				m_strError += strPatName;
				bNotesExist = TRUE;
			}
			rs->Close();

		}

		return bNotesExist;
	}NxCatchAll("Error in CNexWebImportDlg::PatientNotesExists()");

	//if we got here, we failed, so don't let them import
	return TRUE;

}

BOOL CNexWebImportDlg::ToDoTaskExists() {

	//(e.lally 2007-05-24) PLID 26014 - check for ToDo alarm records and return the names of patients with those
		//existing transaction records.
	try {
		m_strError = "";
		//loop through the list, check NexWebTransactionsT to see if any ToDo info exists for the patients
		BOOL bToDoExists = FALSE;
		long p = m_pImportList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		CString strPatName, strTemp, strPersonList;
		long nPersonID =-1;
		while (p) {
			m_pImportList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			nPersonID = VarLong(pRow->GetValue(0));
			strTemp.Format("%li,", nPersonID);
			strPersonList += strTemp;
		}
		strPersonList.TrimRight(",");
		_RecordsetPtr rs;
		if(!strPersonList.IsEmpty()){
			rs = CreateRecordset("SELECT PersonID FROM NexwebTransactionsT "
				"WHERE PersonID IN(%s) AND Field >= %li AND Field <= %li "
				"GROUP BY PersonID ",
				strPersonList, transTypeTodoTask, transTypeTodoNotes);
			IRowSettingsPtr pRow;
			long nRow=0;
			while(!rs->eof) {
				nPersonID = AdoFldLong(rs, "PersonID");
				nRow = m_pImportList->FindByColumn(0, nPersonID, 0, FALSE);
				if(nRow >= 0){
					pRow = m_pImportList->GetRow(nRow);
					//add them to the list
					strPatName.Format("%li - %s %s \n", 
						VarLong(pRow->GetValue(1)),
						VarString(pRow->GetValue(2)),
						VarString(pRow->GetValue(4)));
					m_strError += strPatName;
				}
				bToDoExists = TRUE;
				rs->MoveNext();
			}
			rs->Close();
		}

		return bToDoExists;
	}NxCatchAll("Error in CNexWebImportDlg::ToDoAlarmExists()");

	//if we got here, we failed, so don't let them import
	return TRUE;

}

void CNexWebImportDlg::OnSave() 
{
	//(e.lally 2008-02-28) PLID 27379 -  We very rarely want to force a wait, but it is not a good idea to save an incomplete list with sorting
	//subject to future change. I anticipate this to very rarely be an issue, but if it happens to 
	//become a performance problem, we can take it out.
	try{
		if(m_pImportList->IsRequerying()){
			m_pImportList->WaitForRequery(dlPatienceLevelTerminate);
		}
	}NxCatchAll("Error in CNexWebImportDlg::OnSave waiting for the list to load");

	//we need to make them check the appointment information if there are any
	if (AppointmentInfoExists() ) {
		CString strError;
		strError.Format("The following patients have appointment information added or changed through NexWeb.  Please check over the appointment information "
			"and import those patients individually.\n%s", m_strError);
		MsgBox(strError);
		return;
	}

	//we need to make them check the patient note information if there are any
	if (PatientNotesExists() ) {
		CString strError;
		strError.Format("The following patients have patient notes added or changed through NexWeb.  Please check over the patient note information "
			"and import those patients individually.\n%s", m_strError);
		MsgBox(strError);
		return;
	}

	//(e.lally 2007-05-24) PLID 26014 - We need to make them check the ToDo alarm information if there is any.
	if(ToDoTaskExists()){
		CString strError;
		strError.Format("The following patients have ToDo alarms added or changed through NexWeb.  Please check over the ToDo alarm information "
			"and import those patients individually.\n%s", m_strError);
		MsgBox(strError);
		return;
	}

	//Validate all the data that needs it
	if (! ValidateData()) {
		CString strError;
		strError.Format("The following patients in the list have invalid information, please fix this before saving them to the data.  \n"
			"You can view and fix the information on each patient by clicking the ... column for that patient, fixing the invalid data and clicking the import button. \n%s ", m_strError);
		MsgBox(strError);
		return;
	}

	//make sure that they dinfnately want to save all the information to the data

	if (IDNO == MsgBox(MB_YESNO, "This will import all information in this list into the database.  Are you sure you want to do this? \n"
		"To check the data or import one patient at a time, click the ... column on that patient's row in the datalist")) {

		//woo hoo, they said no
		return;
	}

	//they said yes, keep going
	try  {

		//first, clear the audit array
		m_paryAuditEvents.RemoveAll();

		//(e.lally 2008-02-29) PLID 27379 - be sure to reset this flag
		m_bContinueMassImport = TRUE;

		long nRowCount = m_pImportList->GetRowCount();

		for (int i = 0; i < nRowCount; i++) {
			long nPersonID = VarLong(m_pImportList->GetValue(i, 0));
			m_bSkipOverwrites = FALSE;
	
			CString strTransType;
			TransactionType  transField;
			CString strEntry;
			//04-18-05 - JMM - Added union for referral source because they can add more than one from the web
			// also added the order by because we need to add the referral source to list and then the 
			// primary selection, also we want the last primary selection
			// (j.gruber 2006-11-08 15:56) - PLID 23392 - reorder so that adding a patient is first so that auditing works correctly
			_RecordsetPtr rs = CreateRecordset("SELECT Value, Sub.* FROM NexWebTransactionsT INNER JOIN  "
				"  ( "
				" SELECT MAX(ID) AS ID, TransType, Field, 1 as OrderID FROM NexWebTransactionsT WHERE PersonID = %li AND  "
				" FIELD = (2000)  GROUP BY TransType, Field  "
				" UNION "
				" SELECT MAX(ID) AS ID, TransType, Field, 2 as OrderID FROM NexWebTransactionsT WHERE PersonID = %li AND "
				" FIELD NOT IN (2032, 2036, 2000)  GROUP BY TransType, Field  "
				"  UNION  "
				" SELECT ID, TransType, Field, 3 as OrderID FROM NexWebTransactionsT WHERE PersonID = %li AND FIELD IN (2032,2036)   "
				"  ) Sub ON  "
				"  NexWEbTransactionsT.ID = Sub.Id  "
				"  ORDER BY Sub.OrderID, Sub.Field ASC", nPersonID, nPersonID, nPersonID);

			
			CString strPersonUpdate = "UPDATE PersonT SET ";
			CString strPatientUpdate = "UPDATE PatientsT SET ";

			//we need referral sources also to add them to multireferralsT
			CString strRefSources;

			while (! rs->eof) {

				strTransType = AdoFldString(rs, "TransType");
				transField = (TransactionType)AdoFldLong(rs, "Field");
				strEntry = AdoFldString(rs, "Value");

				SaveField(transField, strEntry, strPersonUpdate, strPatientUpdate, strRefSources,  i, m_bSkipOverwrites);

				if(m_bContinueMassImport == FALSE){
					LoadTransactionRecords();
					MessageBox("The Save To Data was stopped before all records were processed. "
						"Try the import again to continue.");
					return;
				}

				rs->MoveNext();

			}

			//the personID would have changed if we added the patient
			long nOldPersonID = nPersonID;
			nPersonID = VarLong(m_pImportList->GetValue(i, 0));
		
			CString strTemp;

			if (strPersonUpdate != "UPDATE PersonT SET ") {
				strPersonUpdate = strPersonUpdate.Left(strPersonUpdate.GetLength() - 2);
				strTemp.Format(" WHERE ID = %li", nPersonID);
				strPersonUpdate += strTemp;
				ExecuteSqlStd(strPersonUpdate);
			}
	

			if (strPatientUpdate != "UPDATE PatientsT SET ") {
				strPatientUpdate = strPatientUpdate.Left(strPatientUpdate.GetLength() - 2);	
				strTemp.Format(" WHERE PersonID = %li", nPersonID);
			
				strPatientUpdate += strTemp;
				ExecuteSqlStd(strPatientUpdate);
			}

		
			//run the referral sources SQL
			if (! strRefSources.IsEmpty()) {
				ExecuteSqlStd(strRefSources);
			}


			//lets audit this patient
			for (int k=0; k < m_paryAuditEvents.GetSize(); k++) {
				long nPersonID = VarLong(m_pImportList->GetValue(i, 0));
				AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(k));
				CString strPersonName = GetExistingPatientName(nPersonID);
				long nAuditID = BeginNewAuditEvent(pAudit->strUserName);
				AuditEvent(nPersonID, strPersonName, nAuditID, pAudit->aeiItem, nPersonID, pAudit->strOldValue, pAudit->strNewValue, pAudit->nPriority, pAudit->nType);
			}

			//remove everything from the array
			// (j.gruber 2006-11-08 16:17) - PLID 23392 - clean up the memory leaks
			long nCount = m_paryAuditEvents.GetSize();
			for (int i = nCount - 1; i >= 0; i--) {
				AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(i));
				delete pAudit;
				m_paryAuditEvents.RemoveAt(i);
			}
			
			
			//delete the data
			ExecuteSql("DELETE FROM NexwebTransactionsT WHERE PersonID = %li", nOldPersonID);

			
			//send messages
			CClient::RefreshTable(NetUtils::PatCombo, nPersonID);
			CClient::RefreshTable(NetUtils::PatG1, nPersonID);
		}
	

		//remove all the rows
		m_pImportList->Clear();
		
		
		MessageBox("Save To Data finished successfully");

	}NxCatchAll("Error importing information");

}


/*this function makes sure all the incoming information is valid data
  currently it only does Fields:
		Patient BirthDate
		Patient Current Illness Date
*/
BOOL CNexWebImportDlg::ValidateData() {


	try {
		m_strError = "";
		COleDateTime dtMin;
		dtMin.SetDate(1793,1,1);
		COleDateTime dtCheck;
		CString strCheck;
		long nFieldID;
		BOOL bValid = TRUE;

		m_strError = "";
		for (int i = 0; i < m_pImportList->GetRowCount(); i++ ) {

			long nPersonID = VarLong(m_pImportList->GetValue(i, 0));

			//just get the date fields for now
			_RecordsetPtr rsCheck = CreateRecordset("SELECT Value, Field FROM NexWebTransactionsT "
				" WHERE Field IN (1015, 2035) "
				" AND PersonID = %li", nPersonID);

			
			while (! rsCheck->eof) {

				nFieldID = AdoFldLong(rsCheck, "Field");

				switch (nFieldID) {

					case transTypeBirthDate:
					case transTypeCurrentIllnessDate:

						strCheck = AdoFldString(rsCheck, "Value");

						if (! strCheck.IsEmpty()) {

							// (j.gruber 2009-10-01 15:58) - PLID 35602 - ParseDateTime doesn't like the miliseconds, so parse them off
							CString strDate;
							strDate.Format("%s-%s-%s",strCheck.Left(4), strCheck.Mid(5,2), strCheck.Mid(8,2));
							dtCheck.ParseDateTime(strDate, VAR_DATEVALUEONLY);

							if (dtCheck.GetStatus() != 0 || dtCheck < dtMin) {
								CString strPatName;
								strPatName.Format("%li - %s %s \n", 
									VarLong(m_pImportList->GetValue(i, 1)),
									VarString(m_pImportList->GetValue(i, 2)),
									VarString(m_pImportList->GetValue(i, 4)));
								m_strError += strPatName;
								bValid = FALSE;
							}
						}
					break;
				}

				rsCheck->MoveNext();
			}
		}
									

		return bValid;
	}NxCatchAllCall("Error in ValidateData", return FALSE;);
}

//(e.lally 2008-03-18) PLID 27379 - Added flag for overwriting existing data vs. updating only if the data is blank
void CNexWebImportDlg::SaveField(TransactionType transField, CString strEntry, CString &strPersonUpdate, CString &strPatientUpdate, CString &strReferralSources, long nRow, BOOL bSkipOverwrites /*= FALSE*/) {

	CString strTemp;
		switch (transField) {

		case transTypePatient:

			{
			CString strFirst, strMiddle, strLast;
			strFirst =  VarString(m_pImportList->GetValue(nRow, 2));
			strMiddle = VarString(m_pImportList->GetValue(nRow, 3), "");
			strLast = VarString(m_pImportList->GetValue(nRow, 4));

			CDuplicate dlgDuplicate(this);
			dlgDuplicate.SetStatusFilter(EStatusFilterTypes(esfPatient|esfProspect|esfPatientProspect));
			

			//(e.lally 2008-02-29) PLID 27379 - Use just the current name parameters for finding duplicates
			if(dlgDuplicate.FindDuplicates(_Q(strFirst), _Q(strLast), _Q(strMiddle))){
				dlgDuplicate.SetUseMergeButton(TRUE);
				dlgDuplicate.EnableGoToPatientBtn(FALSE);

				int iResult = dlgDuplicate.DoModal();
				
				if (iResult == ID_CANCEL_NEW_PATIENT) {
					//cancel the duplicate dlg and stop the import
					m_bContinueMassImport = FALSE;
					return;
				}
				else if (iResult == ID_CREATE_NEW_PATIENT) {
					//Save as new patient
				}
				else if(iResult == ID_MERGE_WITH_SELECTED) {
					//Update existing patient
					// put the value in the 0 field so we can reference it in the rest of the changes
					m_pImportList->PutValue(nRow, 0, dlgDuplicate.m_nSelPatientId);
					//Set the flag to only update blank fields.
					bSkipOverwrites = TRUE;
					m_bSkipOverwrites = TRUE;
					return;
				}
				else{
					ASSERT(FALSE);
					//All other options should be disabled, so if we get here, we should stop the import.

					//cancel the duplicate dlg and stop the import
					m_bContinueMassImport = FALSE;
					return;
				}
			}

			// (j.jones 2010-01-15 09:02) - PLID 31927 - check the default text message privacy field
			long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);
			
			//e.lally 2009-01-26) PLID 32813 - Changed to a sql batch. Did not parameterize it since inserts and updates
			//do not benefit much from that.
			CString strSqlBatch = BeginSqlBatch();
			AddDeclarationToSqlBatch(strSqlBatch,
				"DECLARE @nPersonID INT;\r\n"
				"DECLARE @nUserDefinedID INT;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "SET @nPersonID = (SELECT COALESCE(Max(ID), 0) + 1 FROM PersonT WHERE ID > 0)\r\n"
				"SET @nUserDefinedID = (SELECT COALESCE(Max(UserDefinedID), 0) + 1 FROM PatientsT WHERE UserDefinedID > 0)\r\n");

			AddStatementToSqlBatch(strSqlBatch, 
			// (e.lally 2009-06-30) PLID 34000 - Capture the userID of the user committing this data to the patient list.
			// (j.jones 2010-01-15 09:01) - PLID 31927 - supported defaulting the text message privacy field
			"INSERT INTO PersonT (ID, First, Middle, Last, UserID, TextMessage) VALUES "
			"(@nPersonID, '%s', '%s', '%s', %li, %li) ", _Q(strFirst), _Q(strMiddle), _Q(strLast), GetCurrentUserID(), nTextMessagePrivacy);
			
			// (b.cardillo 2007-06-06 16:00) - PLID 26244 - Made it default the record to a CurrentStatus of 2, or 
			// "prospect", in keeping with the default of NxWeb.dll.
			//(e.lally 2009-01-26) PLID 32813 - Check the preference to auto assign a security code
			//We can assume they have a NexWeb license 
			CString strRandomSecurityCode = "NULL";
			// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Always (1)
			if(GetRemotePropertyInt("AssignNewPatientSecurityCode", 1, 0, "<None>", true) != 0){
				//(e.lally 2009-01-26) PLID 32813 - This gives us a random, unique security code, but does not
				// *guarantee* that it will remain unique before we can store it. The probably of there being 
				//a conflict is super low though.
				strRandomSecurityCode = "'" + GenerateUniquePatientSecurityCode(GetRemoteData()) + "'";
			}
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientsT (PersonID, UserDefinedID, CurrentStatus, SecurityCode) "
				"VALUES (@nPersonID, @nUserDefinedID, 2, %s)", strRandomSecurityCode);

			// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
			AddStatementToSqlBatch(strSqlBatch, 
				"DECLARE @SecurityGroupID INT\r\n"
				"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
				"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
				"BEGIN\r\n"
				"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, @nPersonID)\r\n"
				"END\r\n");

			long nPersonID =0;
			long nUserDefinedID =0;
			CString strFinalRecordset;
			strFinalRecordset.Format(
					"SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"%s "
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT @nPersonID AS NewPersonID, @nUserDefinedID AS NewUserDefinedID ",
					strSqlBatch);
			_RecordsetPtr rs = CreateRecordsetStd(strFinalRecordset);
			if(!rs->eof){
				nPersonID = AdoFldLong(rs, "NewPersonID", 0);
				nUserDefinedID = AdoFldLong(rs, "NewUserDefinedID", 0);
			}

			if(nPersonID <=0 || nUserDefinedID <=0){
				//We've failed to properly create a new patient.
				ASSERT(FALSE);
				AfxThrowNxException("NexWeb import failed to create a new patient.");
				throw;
			}

			// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
			UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), nPersonID);

			// put the value in the 0 field so we can reference it
			m_pImportList->PutValue(nRow, 0, nPersonID);

			//audit this now
			CString strPersonName =  strLast + ", " + strFirst + " " + strMiddle;
			// (j.gruber 2006-11-08 15:03) - PLID 23392 - this needs to be the contact name, not the patient name!
			// we seem to be using nexweb everwhere here, so that's what we'll use
			// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
			long nAuditID = BeginNewAuditEvent(GetCurrentUserName());
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientCreated, nPersonID, "", strPersonName, aepMedium, aetCreated);
			// (d.singleton 2011-10-11 13:46) - PLID 42102 - We should add auditing to the patient security code assignment.
			strRandomSecurityCode.Replace("'", "");
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientSecurityCode, nPersonID, "", strRandomSecurityCode, aepHigh, aetChanged);
			}
			break;
		case transTypeFirstName:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" First = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" First = CASE WHEN First = '' THEN '%s' ELSE First END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

			if (! VarLong(m_pImportList->GetValue(nRow, 6))) {
				// its not a new patient
				AddToAuditArray(strEntry, "First", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
			break;
		case transTypeLastName:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Last = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Last = CASE WHEN Last = '' THEN '%s' ELSE Last END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

			if (! VarLong(m_pImportList->GetValue(nRow, 6))) {
				// its not a new patient
				AddToAuditArray(strEntry, "Last", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
			break;
		case transTypeEmailAddress:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Email = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Email = CASE WHEN Email = '' THEN '%s' ELSE Email END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Email", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypePassword:
			break;
		case transTypeAddress1:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Address1 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Address1 = CASE WHEN Address1 = '' THEN '%s' ELSE Address1 END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Address1", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeAddress2:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Address2 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Address2 = CASE WHEN Address2 = '' THEN '%s' ELSE Address2 END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Address2", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeCity:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" City = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" City = CASE WHEN City = '' THEN '%s' ELSE City END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "City", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeState:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" State = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" State = CASE WHEN State = '' THEN '%s' ELSE State END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "State", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeZipCode:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Zip = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Zip = CASE WHEN Zip = '' THEN '%s' ELSE Zip END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Zip", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeNotes:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Note = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Note = CASE WHEN CONVERT(VARCHAR(4000),Note) = '' THEN '%s' ELSE Note END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Note", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeHomePhone:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" HomePhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" HomePhone = CASE WHEN HomePhone = '' THEN '%s' ELSE HomePhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "HomePhone", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);

			break;
		case transTypeWorkPhone:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" WorkPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" WorkPhone = CASE WHEN WorkPhone = '' THEN '%s' ELSE WorkPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "WorkPhone", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		// (j.gruber 2006-11-08 14:56) - PLID 23380 - handle the exception field
		case transTypeExtension:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Extension = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Extension = CASE WHEN Extension = '' THEN '%s' ELSE Extension END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Extension", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeFax:
			//SetDlgItemText(IDC_NEXWEB_FAX, strEntry);
			break;
		case transTypeSocialSecurity:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" SocialSecurity = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" SocialSecurity = CASE WHEN SocialSecurity = '' THEN '%s' ELSE SocialSecurity END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "SocialSecurity", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeBirthDate:
			{		
			COleDateTime dtMin, dtBDay;

			// (j.gruber 2009-10-01 15:58) - PLID 35602 - ParseDateTime doesn't like the miliseconds, so parse them off
			CString strDate;
			strDate.Format("%s-%s-%s",strEntry.Left(4), strEntry.Mid(5,2), strEntry.Mid(8,2));

			dtBDay.ParseDateTime(strDate, VAR_DATEVALUEONLY);
			dtMin.SetDate(1753,1,1);
			
			if (dtBDay.GetStatus() == 0 && dtBDay > dtMin) {
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" BirthDate = '%s', ", FormatDateTimeForSql(dtBDay, dtoDate));
				}
				else{
					strTemp.Format(" BirthDate = CASE WHEN BirthDate IS NULL THEN '%s' ELSE BirthDate END, ", FormatDateTimeForSql(dtBDay, dtoDate));
				}
				strPersonUpdate += strTemp;
				AddToAuditArray(FormatDateTimeForSql(dtBDay, dtoDate), "Birthdate", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
			}
			break;
		case transTypeGender: 
			{
			//convert it to an int
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Gender = %s, ", strEntry);
			}
			else{
				strTemp.Format(" Gender = CASE WHEN Gender = 0 THEN %s ELSE Gender END, ", strEntry);
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Gender", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
	
			}
		break;
		case transTypeMiddleName:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Middle = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Middle = CASE WHEN Middle = '' THEN '%s' ELSE Middle END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

			if (! VarLong(m_pImportList->GetValue(nRow, 6))) {
				// its not a new patient
				AddToAuditArray(strEntry, "Middle", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
			break;
		case transTypeCellPhone:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" CellPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" CellPhone = CASE WHEN CellPhone = '' THEN '%s' ELSE CellPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Cellphone", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;
		case transTypeLocation:
			{
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" Location = %s, ", strEntry);
				}
				else{// (s.tullis 2016-05-25 17:23) - NX-100760 
					strTemp.Format(" Location = CASE WHEN Location IS NULL THEN %s ELSE Location END, ", strEntry );
				}
				strPersonUpdate += strTemp;		
				AddToAuditArray(strEntry, "Location", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
			break;
		case transTypeInactive:
		break;

		case transTypeID:
			if (ReturnsRecords("SELECT PersonID FROM PatientsT WHERE UserDefinedID = %s", strEntry)) {
				strEntry = AsString(NewNumber("PatientsT", "userdefinedID"));
				strTemp.Format(" UserDefinedID  = '%s', ", _Q(strEntry));
				strPatientUpdate += strTemp;
				AddToAuditArray(strEntry, "UserDefinedID", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
		break;
		case transTypeMaritalStatus: 
			{
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" MaritalStatus = '%s', ", _Q(strEntry));
			}
			else{
				//Oy, this is actually an nvarchar field, but we treat it like an INT most all other places.
				strTemp.Format(" MaritalStatus = CASE WHEN MaritalStatus = '' THEN '%s' ELSE MaritalStatus END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "MaritalStatus", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
			}
			break;
		case transTypeSpouseName:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" SpouseName = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" SpouseName = CASE WHEN SpouseName = '' THEN '%s' ELSE SpouseName END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "SpouseName", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			break;	

		case transTypeManagerFirst:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerFirst = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerFirst = CASE WHEN EmployerFirst = '' THEN '%s' ELSE EmployerFirst END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerFirst", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
		break;
		case transTypeManagerMiddle:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerMiddle = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerMiddle = CASE WHEN EmployerMiddle = '' THEN '%s' ELSE EmployerMiddle END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerMiddle", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
		break;
		case transTypeManagerLast:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerLast = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerLast = CASE WHEN EmployerLast = '' THEN '%s' ELSE EmployerLast END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerLast", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
		break;

		case transTypeCurrentIllnessDate:	 
		{		
			COleDateTime dtMin, dtDate;	
			
			dtDate.ParseDateTime(strEntry, VAR_DATEVALUEONLY);
			dtMin.SetDate(1753,1,1);
			
			if (dtDate.GetStatus() == 0 && dtDate > dtMin) {
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" DefaultInjuryDate = '%s', ", FormatDateTimeForSql(dtDate, dtoDate));
				}
				else{
					strTemp.Format(" DefaultInjuryDate = CASE WHEN DefaultInjuryDate IS NULL THEN '%s' ELSE DefaultInjuryDate END, ", FormatDateTimeForSql(dtDate, dtoDate));
				}
				strPatientUpdate += strTemp;
				AddToAuditArray(strEntry, "DefaultInjuryDate", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
		}
		break;

		case transTypeCompany:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Company = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Company = CASE WHEN Company = '' THEN '%s' ELSE Company END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "Company", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
		break;

		case transTypeOccupation:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Occupation = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Occupation = CASE WHEN Occupation = '' THEN '%s' ELSE Occupation END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "Occupation", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmployerAddress1:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerAddress1 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerAddress1 = CASE WHEN EmployerAddress1 = '' THEN '%s' ELSE EmployerAddress1 END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerAddress1", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmployerAddress2:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerAddress2 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerAddress2 = CASE WHEN EmployerAddress2 = '' THEN '%s' ELSE EmployerAddress2 END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerAddress2", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmployerCity:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerCity = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerCity = CASE WHEN EmployerCity = '' THEN '%s' ELSE EmployerCity END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerCity", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmployerState:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerState = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerState = CASE WHEN EmployerState = '' THEN '%s' ELSE EmployerState END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerState", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmployerZipCode:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerZip = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerZip = CASE WHEN EmployerZip = '' THEN '%s' ELSE EmployerZip END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "EmployerZip", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;

		case transTypeEmergencyContactFirst:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergFirst = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergFirst = CASE WHEN EmergFirst = '' THEN '%s' ELSE EmergFirst END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "EmergFirst", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmergencyContactLast:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergLast = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergLast = CASE WHEN EmergLast = '' THEN '%s' ELSE EmergLast END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "EmergLast", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmergencyContactRelation:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergRelation = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergRelation = CASE WHEN EmergRelation = '' THEN '%s' ELSE EmergRelation END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "EmergRelation", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmergencyContactHomePhone:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergHPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergHPhone = CASE WHEN EmergHPhone = '' THEN '%s' ELSE EmergHPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "EmergHPhone", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;
		case transTypeEmergencyContactWorkPhone:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergWPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergWPhone = CASE WHEN EmergWPhone = '' THEN '%s' ELSE EmergWPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "EmergWPhone", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
		break;
		case transTypePrefix:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" PrefixID = %s, ", strEntry);
			}
			else{
				strTemp.Format(" PrefixID = CASE WHEN PrefixID IS NULL THEN %s ELSE PrefixID END, ", strEntry);
			}
			strPersonUpdate += strTemp;
			AddToAuditArray(strEntry, "PrefixID", "PersonT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
		break;

		case transTypePatientType:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" TypeofPatient = %s, ", strEntry);
			}
			else{
				strTemp.Format(" TypeofPatient = CASE WHEN TypeofPatient IS NULL THEN %s ELSE TypeofPatient END, ", strEntry);
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "TypeofPatient", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
		break;
		
		case transTypePatientReferralSource:
			
			//check to see if its already there before we add it
			if (! ReturnsRecords("SELECT * FROM MultiReferralsT WHERE PatientID = %li AND ReferralID = %s", VarLong(m_pImportList->GetValue(nRow, 0)), strEntry)) {
				//now we need to check to make sure we aren't adding this twice
				CString strToFind;
				strToFind.Format("VALUES (%s, %li,", strEntry, VarLong(m_pImportList->GetValue(nRow, 0)));
				if (strReferralSources.Find(strToFind) == -1) {
					strTemp.Format("INSERT INTO MultiReferralsT(ReferralID, PatientID, Date) VALUES (%s, %li, getDate());", strEntry, VarLong(m_pImportList->GetValue(nRow, 0)));
					strReferralSources += strTemp;
					AddToAuditArray(strEntry, "", "", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
				}
			}
		break;

		// (j.gruber 2008-06-03 10:29) - PLID 30235 - added NickName
		case transTypeNickName:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" NickName = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" NickName = CASE WHEN NickName = '' THEN '%s' ELSE NickName END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "NickName", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);						
		break;

		case transTypePrimaryReferralSourceName:
		{
			//check to see this field is already there
			long nResult = strPatientUpdate.Find("ReferralID = ");
			if (nResult > 0 ) {
				//there is already one there, so we need to remove it
				long nResult2 = strPatientUpdate.Find(",", nResult);

				//now, remove the string
				CString strTempBefore, strTempAfter;
				strTempBefore = strPatientUpdate.Left(nResult);
				strTempAfter = strPatientUpdate.Right(strPatientUpdate.GetLength() - (nResult2 + 1));
				strPatientUpdate = strTempBefore + strTempAfter;
			}
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" ReferralID = %s, ", strEntry);
			}
			else{
				strTemp.Format(" ReferralID = CASE WHEN ReferralID IS NULL THEN %s ELSE ReferralID END, ", strEntry);
			}
			strPatientUpdate += strTemp;
			AddToAuditArray(strEntry, "ReferralID", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);			
		break;
		}

		//(e.lally 2007-05-21) PLID 26017 - Added support for Patient Coordinator
		case transTypePatientCoord:
		{
			long nNewPatCoordID = 0;
			_RecordsetPtr prsPatCoord = CreateRecordset("SELECT PersonID, Last + ', ' + First AS Name FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID WHERE PersonT.ID = %s AND PatientCoordinator = 1 AND Archived = 0", strEntry);
			if(prsPatCoord->eof){
				//No valid user was returned, we could probably skip this, but let's set it to NULL explicitly
				strPatientUpdate += " EmployeeID = NULL, ";
				strEntry ="";
			}
			else {
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" EmployeeID = %li, ", AdoFldLong(prsPatCoord, "PersonID", -1));
				}
				else{
					strTemp.Format(" EmployeeID = CASE WHEN EmployeeID IS NULL THEN %li ELSE EmployeeID END, ", AdoFldLong(prsPatCoord, "PersonID", -1));
				}
				strPatientUpdate += strTemp;
				nNewPatCoordID = atol(strEntry);
				strEntry = AdoFldString(prsPatCoord, "Name", "");
			}
			//(e.lally 2007-06-21) PLID 26252 - Added auditing for Patient Coordinator
			_RecordsetPtr rs = CreateRecordset("SELECT PatientsT.EmployeeID AS PatCoordID "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.EmployeeID = PersonT.ID "
				"WHERE PatientsT.PersonID = %li",VarLong(m_pImportList->GetValue(nRow, 0)));
			long nOldPatCoord = 0;
			if (!rs->eof) {
				nOldPatCoord = AdoFldLong(rs, "PatCoordID");
			}

			//Only add if the patient coordinator changed
			if(nOldPatCoord != nNewPatCoordID){
				AddToAuditArray(strEntry, "EmployeeID", "PatientsT", VarLong(m_pImportList->GetValue(nRow, 0)), transField);
			}
		break;
		}
	}
				
}


// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
#define AUDIT_EVENT(aeiEvent) {\
	if (FieldChanged(strField, strTable, nPersonID, strValue, strOldValue)) { \
		AuditEventStruct *pAudit = new AuditEventStruct; \
		pAudit->aeiItem = aeiEvent; \
		pAudit->strOldValue = strOldValue; \
		pAudit->strNewValue = strValue; \
		pAudit->nPriority = 1; \
		pAudit->nType = 1; \
		pAudit->strUserName = GetCurrentUserName(); \
		m_paryAuditEvents.Add((AuditEventStruct*) pAudit);}	}


void CNexWebImportDlg::AddToAuditArray(CString strValue, CString strField, CString strTable, long nPersonID, TransactionType transType) {

	
	CString strOldValue;

	switch (transType) {
		
		case transTypeFirstName:
			AUDIT_EVENT(aeiPatientPersonFirst);
		break;
		
		case transTypeLastName:
			AUDIT_EVENT(aeiPatientPersonLast);
		break;
		
		case transTypeEmailAddress:
			AUDIT_EVENT(aeiPatientEmail);
		break;
		
		case transTypePassword:
		break;
		
		case transTypeAddress1:
			AUDIT_EVENT(aeiPatientAddress);
		break;
		
		case transTypeAddress2:
			AUDIT_EVENT(aeiPatientAddress2);
		break;
		
		case transTypeCity:
			AUDIT_EVENT(aeiPatientCity);
		break;
		
		case transTypeState:
			AUDIT_EVENT(aeiPatientState);
		break;
		
		case transTypeZipCode:
			AUDIT_EVENT(aeiPatientZip);
		break;
		
		case transTypeNotes:
			AUDIT_EVENT(aeiPatientG1Note);
		break;
		
		case transTypeHomePhone:
			AUDIT_EVENT(aeiPatientHPhone);
		break;
		
		case transTypeWorkPhone:
			AUDIT_EVENT(aeiPatientWPhone);
		break;

		// (j.gruber 2006-11-08 15:18) - PLID 23380 - handle extension field
		case transTypeExtension:
			AUDIT_EVENT(aeiPatientExtension);
		break;
		
		case transTypeFax:
//			AUDIT_EVENT(aeiPatientFax);
		break;
	
		case transTypeSocialSecurity:
			AUDIT_EVENT(aeiPatientSSN);
		break;
		case transTypeBirthDate:
			AUDIT_EVENT(aeiPatientBirthDate);
		break;
		
		case transTypeGender: 
			{
				if (FieldChanged(strField, strTable, nPersonID, strValue, strOldValue)) { 
					AuditEventStruct *pAudit = new AuditEventStruct; 
					pAudit->aeiItem = aeiPatientGender; 
					pAudit->strOldValue = GetGender(strOldValue); 
					pAudit->strNewValue = GetGender(strValue); 
					pAudit->nPriority = 1; 
					pAudit->nType = 1; 
					// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
					pAudit->strUserName = GetCurrentUserName();
					m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
				}	
			}
		break;
		
		case transTypeMiddleName:
			AUDIT_EVENT(aeiPatientPersonMiddle);
		break;
		
		case transTypeCellPhone:
			AUDIT_EVENT(aeiPatientMobilePhone);
		break;
		
		case transTypeLocation:
			{
				if (FieldChanged(strField, strTable, nPersonID, strValue, strOldValue)) { 
					AuditEventStruct *pAudit = new AuditEventStruct; 
					pAudit->aeiItem = aeiPatientLocation; 
					pAudit->strOldValue = GetLocation(strOldValue); 
					pAudit->strNewValue = GetLocation(strValue); 
					pAudit->nPriority = 1; 
					pAudit->nType = 1; 
					// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
					pAudit->strUserName = GetCurrentUserName();
					m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
				}	
			}
		break;
		
		case transTypeInactive:
//			AUDIT_EVENT(aeiPatientPersonStatus);
		break;

		case transTypeID:
			AUDIT_EVENT(aeiPatientUserID);
		break;
		
		case transTypeMaritalStatus: 
			{
				if (FieldChanged(strField, strTable, nPersonID, strValue, strOldValue)) { 
					AuditEventStruct *pAudit = new AuditEventStruct; 
					pAudit->aeiItem = aeiPatientMaritalStatus; 
					pAudit->strOldValue = GetMaritalStatus(strOldValue); 
					pAudit->strNewValue = GetMaritalStatus(strValue); 
					pAudit->nPriority = 1; 
					pAudit->nType = 1; 
					// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
					pAudit->strUserName = GetCurrentUserName();
					m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
				}	
			}
		break;

		case transTypeSpouseName:
			AUDIT_EVENT(aeiPatientSpouseStatus);
		break;	

		case transTypeManagerFirst:
			AUDIT_EVENT(aeiPatientEmployerFirst);
		break;
		
		case transTypeManagerMiddle:
			AUDIT_EVENT(aeiPatientEmployerMiddle);
		break;
		
		case transTypeManagerLast:
			AUDIT_EVENT(aeiPatientEmployerLast);
		break;

		case transTypeCurrentIllnessDate:	 
			AUDIT_EVENT(aeiPatientDefaultInjuryDate);
		break;

		case transTypePatientType:
			{
				if (FieldChanged(strField, strTable, nPersonID, strValue, strOldValue)) { 
					AuditEventStruct *pAudit = new AuditEventStruct; 
					pAudit->aeiItem = aeiPatientType; 
					pAudit->strOldValue = GetPatientType(strOldValue); 
					pAudit->strNewValue = GetPatientType(strValue); 
					pAudit->nPriority = 1; 
					pAudit->nType = 1; 
					// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
					pAudit->strUserName = GetCurrentUserName();
					m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
				}	
			}
		break;

		case transTypeCompany:
			AUDIT_EVENT(aeiPatientCompany);
		break;

		case transTypeOccupation:
			AUDIT_EVENT(aeiPatientOccupation);
		break;
		case transTypeEmployerAddress1:
			AUDIT_EVENT(aeiPatientEmployerAddress1);
		break;
		case transTypeEmployerAddress2:
			AUDIT_EVENT(aeiPatientEmployerAddress2);
		break;
		case transTypeEmployerCity:
			AUDIT_EVENT(aeiPatientEmployerCity);
		break;
		case transTypeEmployerState:
			AUDIT_EVENT(aeiPatientEmployerState);
		break;
		case transTypeEmployerZipCode:
			AUDIT_EVENT(aeiPatientEmployerState);
		break;
		case transTypeEmergencyContactFirst:
			AUDIT_EVENT(aeiPatientEmergFirst);
		break;
		case transTypeEmergencyContactLast:
			AUDIT_EVENT(aeiPatientEmergLast);
		break;
		case transTypeEmergencyContactRelation:
			AUDIT_EVENT(aeiPatientEmergRelation);
		break;
		case transTypeEmergencyContactHomePhone:
			AUDIT_EVENT(aeiPatientEmergHPhone);
		break;
		case transTypeEmergencyContactWorkPhone:
			AUDIT_EVENT(aeiPatientEmergWPhone);
		break;
		case transTypePrefix:
			//Prefixes aren't audited
		break;
		case transTypePrimaryReferralSourceName:
			//Primary status is not audited
		break;
		case transTypePatientReferralSource:
			{
				AuditEventStruct *pAudit = new AuditEventStruct; 
				pAudit->aeiItem = aeiPatientReferralAdd; 
				pAudit->strOldValue = ""; 
				pAudit->strNewValue = GetReferralSource(strValue); 
				pAudit->nPriority = aepMedium; 
				pAudit->nType = aetChanged; 
				// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
				pAudit->strUserName = GetCurrentUserName();
				m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
					
			}
		break;	
		case transTypeNickName:
			AUDIT_EVENT(aeiPatientNickname);
		break;
		case transTypePatientCoord:
		{
			AuditEventStruct *pAudit = new AuditEventStruct; 
			pAudit->aeiItem = aeiPatientCoord; 
			_RecordsetPtr rs = CreateRecordset("SELECT Last + ', ' + First as Field FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.EmployeeID = PersonT.ID "
				"WHERE PatientsT.PersonID = %li",nPersonID);
			CString strField;
			if (!rs->eof) {
				strField = AdoFldString(rs, "Field");
			}
			pAudit->strOldValue = strField; 
			pAudit->strNewValue = strValue; 
			pAudit->nPriority = aepMedium; 
			pAudit->nType = aetChanged; 
			// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
			pAudit->strUserName = GetCurrentUserName();
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		}

	}



}


BOOL CNexWebImportDlg::FieldChanged(CString strField, CString strTable, long nPersonID, CString strNewValue, CString &strOldValue) {

   	//get the value out of the table
	_RecordsetPtr rs;
	if (strTable == "PersonT") {
		rs = CreateRecordset("SELECT %s AS Field FROM %s WHERE ID = %li", strField, strTable, nPersonID);
	}
	else if (strTable == "PatientsT") {
		rs = CreateRecordset("SELECT %s AS Field FROM %s WHERE PersonID  = %li", strField, strTable, nPersonID);
	}

	if (! rs->eof) {
		
		CString strTemp = AsString(rs->Fields->Item["Field"]->Value);

		if (strTemp.CompareNoCase(strNewValue) != 0) {
			
			strOldValue = strTemp;
			return TRUE;			
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}


void CNexWebImportDlg::OnDblClickCellNexwebImportList(long nRowIndex, short nColIndex) 
{
	try{
		//check if no row was selected
		if(nRowIndex == sriNoRow ){
			return;
		}
		//(e.lally 2008-02-28) PLID 29142 - Open the editor when double clicking an entry
		long nResult = EditImportListObject(nRowIndex);

		switch (nResult) {
			
			case IDCANCEL:
				//previewing, close this window too
				OnCancel();
			break;

			default:
				//at the moment, just requery the list
				m_pImportList->Clear();
				LoadTransactionRecords();
			break;
		}
		
	}NxCatchAll("Error in CNexWebImportDlg::OnDblClickCellNexwebImportList");
}

void CNexWebImportDlg::OnRButtonUpNexwebImportList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_pImportList->CurSel = nRow;
	try{
		if(nRow == sriNoRow){
			return;
		}

		enum {
			miEditObject = -1,
		};

		// Build a popup menu to change object
		CMenu Popup;
		Popup.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		Popup.InsertMenu(nIndex++, MF_BYPOSITION, miEditObject, "Edit");
	
		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* pwnd = GetDlgItem(IDC_NEXWEB_IMPORT_LIST);
		int nResult =0;
		if (pwnd != NULL) {
			pwnd->ClientToScreen(&pt);
			nResult = Popup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}

		switch(nResult) {
			case miEditObject:
			{
				try {
					if(m_pImportList != NULL){
						long nRow = m_pImportList->GetCurSel();
						if(nRow != sriNoRow){
							//(e.lally 2008-02-28) PLID 29142 - Open the editor when right clicking and choosing
							//to 'Edit'
							long nReturnVal = EditImportListObject(nRow);
							switch (nReturnVal) {
								case IDCANCEL:
									//previewing, close this window too
									OnCancel();
								break;

								default:
									//at the moment, just requery the list
									m_pImportList->Clear();
									LoadTransactionRecords();
								break;
							}
						}
					}
				}NxCatchAll("Error editing NexWeb Object.");
			}
				break;
			default:
				break;
		}
	}NxCatchAll("Error in CNexWebImportDlg::OnRButtonDownNexwebImportList");
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CNexWebImportDlg::OnOK()
{
	//Eat the message
}
