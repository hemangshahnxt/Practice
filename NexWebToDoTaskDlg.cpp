// NexWebToDoTaskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexWebToDoTaskDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "TodoUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CNexWebToDoTaskDlg dialog

enum ToDoListColumns{
	tdlTaskID = 0,
	tdlPersonID,
	tdlCategory,
	tdlTask,
	tdlPriority,
	tdlNotes,
	tdlAssignedTo,
	tdlDeadline,
	tdlRemindBy,
	tdlIsNew,
};


CNexWebToDoTaskDlg::CNexWebToDoTaskDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebToDoTaskDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebToDoTaskDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CNexWebToDoTaskDlg::~CNexWebToDoTaskDlg()
{

}


void CNexWebToDoTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebToDoTaskDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebToDoTaskDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebToDoTaskDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebToDoTaskDlg message handlers

BOOL CNexWebToDoTaskDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pToDoList = BindNxDataList2Ctrl(this, IDC_NEXWEB_TODO_LIST, GetRemoteData(), true);
	LoadToDoList();	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexWebToDoTaskDlg::SetPersonID(long nPersonID, BOOL bIsNewPatient)
{
	m_nPersonID = nPersonID;
	m_bIsNewPatient = bIsNewPatient;
}

void CNexWebToDoTaskDlg::LoadToDoList()
{
	try {
		//First get all the ToDos that they added for this patient
		_RecordsetPtr rs = CreateRecordset(
			"SELECT ObjectID, (SELECT Top 1 Value FROM NexwebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 4102 ORDER BY ID DESC) AS Method, "
			"  (SELECT Top 1 CONVERT(int, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 4103 ORDER BY ID DESC) AS Priority, "
			" (SELECT Top 1 CONVERT(int, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 4104 ORDER BY ID DESC) AS AssignedTo,  "
			" (SELECT Top 1 CONVERT(datetime, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 4105 ORDER BY ID DESC) AS Deadline, "
			" (SELECT Top 1 CONVERT(datetime, Value)  From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field =  4106 ORDER BY ID DESC) AS RemindBy,  "
			" (SELECT top 1 Value FROM NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND nTrans2.PersonID = NexwebTransactionsT.PersonID AND Field = 4107 ORDER BY ID DESC) AS Notes, "
			" 1 AS IsNew FROM NexWebTransactionsT  "
			" WHERE Field = 4101 AND PersonID = %li ", m_nPersonID);
		CString strNotes, strTaskMethod;
		long nPriority = -1, nAssignedToUserID = -1;
		COleDateTime dtDeadline, dtRemindBy, dtDefault;
		dtDefault = COleDateTime::GetCurrentTime();
		_variant_t varIsNew;
		varIsNew.vt = VT_BOOL;
		varIsNew.boolVal = TRUE;

		//loop through the records and get the other values out that are used in the datalist
		while (!rs->eof) {

			strTaskMethod = AdoFldString(rs, "Method", "E-Mail");
			strNotes = AdoFldString(rs, "Notes", "");

			nPriority = AdoFldLong(rs, "Priority", 0);
			nAssignedToUserID = AdoFldLong(rs, "AssignedTo", -1);

			dtDeadline = AdoFldDateTime(rs, "Deadline", dtDefault);
			dtRemindBy = AdoFldDateTime(rs, "RemindBy", dtDefault);

			//add the record to the datalist
			_variant_t varNull, varDeadline, varRemindBy;
			varNull.vt = VT_NULL;
			IRowSettingsPtr pRow = m_pToDoList->GetNewRow();
			pRow->PutValue(tdlTaskID, varNull);
			pRow->PutValue(tdlPersonID, m_nPersonID);
			pRow->PutValue(tdlCategory, varNull);
			pRow->PutValue(tdlTask, _bstr_t(strTaskMethod));
			pRow->PutValue(tdlPriority, nPriority);
			pRow->PutValue(tdlNotes, _bstr_t(strNotes));
			pRow->PutValue(tdlAssignedTo, nAssignedToUserID);
			varDeadline = dtDeadline;
			varDeadline.vt = VT_DATE;
			pRow->PutValue(tdlDeadline, varDeadline);
			varRemindBy = dtRemindBy;
			varRemindBy.vt = VT_DATE;
			pRow->PutValue(tdlRemindBy, varRemindBy);
			pRow->PutValue(tdlIsNew, varIsNew);

			m_pToDoList->AddRowAtEnd(pRow, NULL);
			rs->MoveNext();
		}

	}NxCatchAll("Error Loading ToDo List");
}


BOOL CNexWebToDoTaskDlg::ValidateData()
{
	BOOL bIsValid = TRUE;
	try{
		IRowSettingsPtr pRow = m_pToDoList->GetFirstRow();
		CString strError;
		COleDateTime dtMin, dtEmpty;
		dtMin.SetDate(1900, 1, 1);
		dtEmpty.SetDate(1799, 1, 1);
		COleDateTime dtDeadline, dtRemindBy;
		while(pRow){

			if(pRow->GetValue(tdlDeadline).vt == VT_NULL || pRow->GetValue(tdlDeadline).vt == VT_EMPTY){
				strError.Format("Invalid Deadline Date: Empty\n");
				m_strError += strError;
				bIsValid = FALSE;
			}
			else{
				dtDeadline = VarDateTime(pRow->GetValue(tdlDeadline));
				if (dtDeadline < dtMin || dtDeadline.GetStatus() != 0) {
					strError.Format("Invalid Deadline Date: %s\n", FormatDateTimeForInterface(dtDeadline));
					m_strError += strError;
					bIsValid = FALSE;
				}
			}

			if(pRow->GetValue(tdlRemindBy).vt == VT_NULL || pRow->GetValue(tdlRemindBy).vt == VT_EMPTY){
				strError.Format("Invalid Remind By Date: Empty\n");
				m_strError += strError;
				bIsValid = FALSE;
			}
			else{
				dtRemindBy = VarDateTime(pRow->GetValue(tdlRemindBy));
				if (dtRemindBy < dtMin || dtRemindBy.GetStatus() != 0) {
					strError.Format("Invalid Remind By Date: %s\n", FormatDateTimeForInterface(dtRemindBy));
					m_strError += strError;
					bIsValid = FALSE;
				}
			}

			//Check if the remind by date and after the deadline, only if they are both valid
			if(bIsValid == TRUE && dtRemindBy > dtDeadline){
				strError.Format("The Remind By Date must be on or before the Deadline Date\n");
					m_strError += strError;
					bIsValid = FALSE;
			}

			CString strNote = VarString(pRow->GetValue(tdlNotes));
			if (strNote.GetLength() >  2000) {
				strError.Format("Note Length exceeds 2000 characters for note: %s\n", strNote);
				m_strError += strError;
				bIsValid = FALSE;

			}

			pRow = pRow->GetNextRow();
		}
		return bIsValid;
	}NxCatchAll("Error in CNexWebToDoTaskDlg::ValidateDate");
	return FALSE;

}

BOOL CNexWebToDoTaskDlg::SaveInfo(long nPersonID /*= -1*/)
{
	BOOL bSuccess = TRUE;
	try {
		
		if (nPersonID == -1) {
			//that means we are using our personID
			nPersonID = m_nPersonID;
		}

		IRowSettingsPtr pRow = m_pToDoList->GetFirstRow();
		//loop through the ToDo tasks in the datalist and save them
		while(pRow){
			if (VarBool(pRow->GetValue(tdlIsNew))) {
				pRow->PutValue(tdlPersonID,nPersonID);
				CreateToDo(pRow);
			}
			else{
				//Modify the ToDo
			}
			pRow = pRow->GetNextRow();
		}
	}NxCatchAllCall("Error in CNexWebToDoTaskDlg::SaveInfo", bSuccess = FALSE;);
	return bSuccess;
}

void CNexWebToDoTaskDlg::CreateToDo(IRowSettingsPtr pRow)
{
	//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
	CString sql;
	CString strDeadlineDate = "NULL", strCompleteDate = "NULL", strRemindByDate = "NULL";
	COleDateTime dtMin, dtDeadline, dtComplete, dtRemindBy;
	dtMin.m_dt = 0;
	dtDeadline = VarDateTime(pRow->GetValue(tdlDeadline), dtMin);
	dtComplete = dtMin;
	dtRemindBy = VarDateTime(pRow->GetValue(tdlRemindBy), dtMin);

	long nRegardingID = m_nPersonID;
	long nRegardingType = 1;//patient

	long nInputUser = GetCurrentUserID();

	//These can't be null
	strDeadlineDate = "'" + FormatDateTimeForSql(dtDeadline) + "'";
	strRemindByDate = "'" + FormatDateTimeForSql(dtRemindBy) + "'";
	//Check our date for null value versus date value
	if(dtComplete.GetStatus() == COleDateTime::valid && dtComplete > dtMin)
		strCompleteDate = "'" + FormatDateTimeForSql(dtComplete) + "'";

	// (c.haag 2008-06-09 11:11) - PLID 30321 - Use the new global todo creation function
	long nTaskID = TodoCreate(dtRemindBy, dtDeadline, VarLong(pRow->GetValue(tdlAssignedTo)), VarString(pRow->GetValue(tdlNotes)),
		VarString(pRow->GetValue(tdlTask)), nRegardingID, (TodoType)nRegardingType, VarLong(pRow->GetValue(tdlPersonID)), -1,
		(TodoPriority)VarLong(pRow->GetValue(tdlPriority)), -1, dtComplete);

	//Audit
	CString strPersonName, strUser, strOldValue, strNewValue;
	_RecordsetPtr rs = CreateRecordset("SELECT (SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li) AS Name, "
		"(SELECT Username FROM UsersT WHERE PersonID = %li) AS Username ", VarLong(pRow->GetValue(tdlPersonID)), VarLong(pRow->GetValue(tdlAssignedTo), -1));
	if(!rs->eof){
		strPersonName = AdoFldString(rs, "Name", "");
		strUser = AdoFldString(rs, "Username", "");
	}
	strNewValue.Format("Assigned To: %s, Deadline: %s, Method: %s, Note: %s",
		strUser, FormatDateTimeForInterface(dtDeadline), 
		VarString(pRow->GetValue(tdlTask)), VarString(pRow->GetValue(tdlNotes)));
	// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
	long nAuditID = BeginNewAuditEvent(GetCurrentUserName());
	AuditEvent(VarLong(pRow->GetValue(tdlPersonID)), strPersonName, nAuditID, aeiPatientToDoCreated, nTaskID, strOldValue, strNewValue, aepMedium, aetCreated);

	//Do the table checker here, while we have the taskID
	// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
	CClient::RefreshTodoTable(nTaskID,m_nPersonID ,VarLong(pRow->GetValue(tdlAssignedTo)), TableCheckerDetailIndex::tddisAdded);
}