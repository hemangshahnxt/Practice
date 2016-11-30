// PatientReminderSenthistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PatientReminderSenthistoryDlg.h"
#include "afxdialogex.h"
#include "PatientReminderSenthistoryUtils.h"
#include "NotesDlg.h"
#include "NxModalParentDlg.h"
#include "AuditTrail.h"
#include "HL7Utils.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;
//(s.dhole 8/28/2014 3:20 PM ) - PLID 62751 AAdded new Class
// CPatientReminderSenthistoryDlg dialog

enum SentReminderHst{
	rhcID = 0,
	rhcReminderDate = 1,
	rhcUserID = 2,
	rhcUserName = 3,
	rhcReminderMethodID = 4,
	rhcReminderMethod = 5,
	rhcNoteIcon = 6,
	rhcPatientID=7, // (s.tullis 2015-06-23 14:17) - PLID 66197 - added patientID column
};

#define IDM_DELETE_RECORD			49001
#define IDM_HOME_PHONE				49011
#define IDM_WORK_PHONE				49012
#define IDM_MOBILE_PHONE			49013
#define IDM_PAGER				    49014
#define IDM_OTHER_PHONE				49015
#define IDM_EMAIL				    49016
#define IDM_TEXTMESSAGING			49017
#define IDM_TELEVOX				    49018
#define IDM_LETTERWRITING			49019
#define IDM_RECALL				    49020
#define IDM_INPERSON				49021
IMPLEMENT_DYNAMIC(CPatientReminderSenthistoryDlg, CNxDialog)

CPatientReminderSenthistoryDlg::CPatientReminderSenthistoryDlg(CWnd* pParent /*=NULL*/, long nPatientId, CString sPatientName)
	: CNxDialog(CPatientReminderSenthistoryDlg::IDD, pParent)
{

	m_nPatientId = nPatientId;
	m_sPatientName = sPatientName;
	m_hExtraNotesIcon = NULL;

}

CPatientReminderSenthistoryDlg::~CPatientReminderSenthistoryDlg()
{
	if (m_hExtraNotesIcon) DestroyIcon(m_hExtraNotesIcon);
}


void CPatientReminderSenthistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REMINDER_SENT_DLG_COLOR, m_bkg);
	DDX_Control(pDX, ID_BTN_CLOSE_REMINDER_HISTORY, m_btn_Close);
	DDX_Control(pDX, IDI_BTN_REMINDERSENT, m_btn_New);
	
}


BEGIN_MESSAGE_MAP(CPatientReminderSenthistoryDlg, CNxDialog)
	ON_COMMAND(IDM_DELETE_RECORD, OnDeleteRecord)
	ON_BN_CLICKED(IDI_BTN_REMINDERSENT, &CPatientReminderSenthistoryDlg::OnBnClickedBtnRemindersent)
	ON_COMMAND_RANGE(IDM_HOME_PHONE, IDM_INPERSON, &CPatientReminderSenthistoryDlg::OnMenuAction)
	ON_BN_CLICKED(ID_BTN_CLOSE_REMINDER_HISTORY, &CPatientReminderSenthistoryDlg::OnBnClickedBtnCloseReminderHistory)
END_MESSAGE_MAP()

BOOL CPatientReminderSenthistoryDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		SetWindowText(FormatString("Reminders Sent for %s", m_sPatientName));
		//(s.dhole 8/29/2014 3:37 PM ) - PLID 62746 global property in bulk
		g_propManager.CachePropertiesInBulk("PatientReminderSenthistoryDlg", propNumber,
			"(Username = '<None>') AND ("
			"Name = 'SentReminderNoteDefaultCategory' "
			")");

		m_btn_Close.AutoSet(NXB_CLOSE);
		m_btn_New.AutoSet(NXB_NEW);

		m_pReminderHistoryList = BindNxDataList2Ctrl(IDC_REMINDER_SENT_HISTORY_LIST, false);
		m_hExtraNotesIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16, 16, 0);
		LoadData();
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}
void CPatientReminderSenthistoryDlg::AddRowToList(_RecordsetPtr rsPtr)
{
	IRowSettingsPtr pRow = m_pReminderHistoryList->GetNewRow();
	pRow->PutValue(rhcID, AdoFldLong(rsPtr, "ID"));
	pRow->PutValue(rhcReminderDate, COleVariant(AdoFldDateTime(rsPtr, "ReminderDate")));
	pRow->PutValue(rhcUserID, AdoFldLong(rsPtr, "UserID"));
	pRow->PutValue(rhcUserName, _bstr_t(AdoFldString(rsPtr, "UserName")));
	pRow->PutValue(rhcReminderMethodID, (enumSentReminderHistoryMethod)AdoFldByte(rsPtr, "ReminderMethod"));
	pRow->PutValue(rhcReminderMethod, _bstr_t(GetSentReminderMethodeString((enumSentReminderHistoryMethod)AdoFldByte(rsPtr, "ReminderMethod"))));
	if (AdoFldLong(rsPtr, "IsNOTEExist")>0)
	{
		pRow->PutValue(rhcNoteIcon, (long)m_hExtraNotesIcon);
	}
	else
	{
		pRow->PutValue(rhcNoteIcon, (LPCTSTR)"BITMAP:FILE");
	}
	// (s.tullis 2015-06-23 14:17) - PLID 66197 - Added PatientID column
	pRow->PutValue(rhcPatientID, AdoFldLong(rsPtr, "PatientID"));
	m_pReminderHistoryList->AddRowSorted(pRow, NULL);
}
// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
void CPatientReminderSenthistoryDlg::LoadData()
{
	_RecordsetPtr rsPtr = CreateParamRecordset("SELECT PatientRemindersSentT.ID, PatientRemindersSentT.ReminderDate,  "
		" PatientRemindersSentT.UserID, UsersT.UserName, PatientRemindersSentT.ReminderMethod, "
		" PatientRemindersSentT.PatientID, ISNULL((Select Top 1  PatientRemindersSentID From NoteInfoT WHERE PatientRemindersSentID = PatientRemindersSentT.ID), 0) AS IsNOTEExist "
		" FROM  PatientRemindersSentT INNER JOIN "
		" UsersT ON PatientRemindersSentT.UserID = UsersT.PersonID  WHERE PatientRemindersSentT.PatientID = {INT} AND Deleted = 0 ", m_nPatientId);
	while (!rsPtr->eof) {
		AddRowToList(rsPtr);
		rsPtr->MoveNext();
	}

}

// CPatientReminderSenthistoryDlg message handlers
BEGIN_EVENTSINK_MAP(CPatientReminderSenthistoryDlg, CNxDialog)
	ON_EVENT(CPatientReminderSenthistoryDlg, IDC_REMINDER_SENT_HISTORY_LIST, 7, CPatientReminderSenthistoryDlg::RButtonUpReminderSentHistoryList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientReminderSenthistoryDlg, IDC_REMINDER_SENT_HISTORY_LIST, 4, CPatientReminderSenthistoryDlg::LButtonDownReminderSentHistoryList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


void CPatientReminderSenthistoryDlg::RButtonUpReminderSentHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{


	IRowSettingsPtr pRow(lpRow);
	if (!pRow)
	{
		return;
	}
	try{
		m_pReminderHistoryList->CurSel = pRow;
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		mnu.InsertMenu(1, MF_BYPOSITION, IDM_DELETE_RECORD, "Delete");
		CRect rc;
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 8/29/2014 3:55 PM ) - PLID 63516 
void CPatientReminderSenthistoryDlg::LButtonDownReminderSentHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	IRowSettingsPtr pRow(lpRow);
	if (!pRow)
	{
		return;
	}
	try
	{ 
	// TODO: Add your message handler code here
		if (nCol == rhcNoteIcon)
		{
			m_pReminderHistoryList->CurSel = pRow;
			long nPatientRemindersSentID = VarLong(pRow->GetValue(rhcID));
			CNotesDlg dlgNotes(this);
			dlgNotes.SetPersonID(m_nPatientId);
			dlgNotes.m_bIsPatientRemindersSent = true;
			dlgNotes.m_clrHistoryNote = m_bkg.GetColor(); 
			//(s.dhole 8/29/2014 3:37 PM ) - PLID 62746 Load defult category
			dlgNotes.m_nCategoryIDOverride = GetRemotePropertyInt("SentReminderNoteDefaultCategory", -1, 0, "<None>", true);
			//(s.dhole 8/29/2014 3:55 PM ) - PLID 63516 load default note text
			dlgNotes.m_sPatientRemindersPrefix = FormatString("Reminder: %s - %s", FormatDateTimeForInterface(pRow->GetValue(rhcReminderDate), DTF_STRIP_SECONDS, dtoDateTime), GetSentReminderMethodeString((enumSentReminderHistoryMethod)VarLong(pRow->GetValue(rhcReminderMethodID))));
			dlgNotes.m_nPatientRemindersSentID = nPatientRemindersSentID;
			CNxModalParentDlg dlg(this, &dlgNotes, CString("Reminder Item Notes"));
			dlg.DoModal();
			// now refresh icon
			if (ReturnsRecordsParam("SELECT 1 FROM Notes WHERE PatientRemindersSentID = {INT}", nPatientRemindersSentID))
			{
				pRow->PutValue(rhcNoteIcon, (long)m_hExtraNotesIcon);
			}
			else
			{
				pRow->PutValue(rhcNoteIcon, (LPCTSTR)"BITMAP:FILE");
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// Delete record
void CPatientReminderSenthistoryDlg::OnDeleteRecord()
{
	try{
		//(s.dhole 9/4/2014 12:00 PM ) - PLID 62752 Check permission
		if (CheckCurrentUserPermissions(bioPatientSentReminder, sptDelete))
		{
			IRowSettingsPtr pRow = m_pReminderHistoryList->CurSel;
			if (pRow && (MessageBox("Are you sure you wish to delete this Reminder?", "Delete?", MB_YESNO) != IDNO))
			{
				m_pReminderHistoryList->CurSel = pRow;
				long nPatientRemindersSentID = VarLong(pRow->GetValue(rhcID));
				// (s.tullis 2015-06-23 14:17) - PLID 66197 - Need a patient ID to send Hl7 Patient Reminder message
				long nPatientID = VarLong(pRow->GetValue(rhcPatientID));
				CString strSqlBatch = BeginSqlBatch();
				CNxParamSqlArray args;
				AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PatientRemindersSentID int;\r\n");
				AddParamStatementToSqlBatch(strSqlBatch, args, "SET @PatientRemindersSentID  = {INT};\r\n", nPatientRemindersSentID);
				AddStatementToSqlBatch(strSqlBatch, "UPDATE NoteInfoT SET PatientRemindersSentID = NULL WHERE PatientRemindersSentID =  @PatientRemindersSentID");
				// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientRemindersSentT SET Deleted = 1 WHERE ID =  @PatientRemindersSentID");
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
				// (s.tullis 2015-06-23 14:17) - PLID 66197 - Send Patient Reminder Hl7 on deletion
				SendPatientReminderHL7Message(nPatientID, nPatientRemindersSentID);
				m_pReminderHistoryList->RemoveRow(pRow);
				//(s.dhole 9/3/2014 3:06 PM ) - PLID 62801 Audit delete action
				CString strOldVal = FormatString("%s [Sent Date: %s ]", VarString(pRow->GetValue(rhcReminderMethod)), FormatDateTimeForInterface(pRow->GetValue(rhcReminderDate), DTF_STRIP_SECONDS, dtoDateTime));
				AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiReminderSentHistoryDelete, GetActivePatientID(), strOldVal, "<Deleted>", aepHigh, aetDeleted);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// return enum value
enumSentReminderHistoryMethod GetSentActionEnum(long nId)
{
	return (enumSentReminderHistoryMethod)(nId - 49010);
}


// load new methode menu
void CPatientReminderSenthistoryDlg::OnBnClickedBtnRemindersent()
{
	try{
		//(s.dhole 9/4/2014 12:00 PM ) - PLID 62752 Check permission
		if (CheckCurrentUserPermissions(bioPatientSentReminder, sptCreate))
		{
			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;
			DWORD dwWritePermitted = MF_BYPOSITION;
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_HOME_PHONE, GetSentReminderMethodeString(GetSentActionEnum(IDM_HOME_PHONE)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_WORK_PHONE, GetSentReminderMethodeString(GetSentActionEnum(IDM_WORK_PHONE)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_MOBILE_PHONE, GetSentReminderMethodeString(GetSentActionEnum(IDM_MOBILE_PHONE)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_PAGER, GetSentReminderMethodeString(GetSentActionEnum(IDM_PAGER)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_OTHER_PHONE, GetSentReminderMethodeString(GetSentActionEnum(IDM_OTHER_PHONE)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_EMAIL, GetSentReminderMethodeString(GetSentActionEnum(IDM_EMAIL)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_TEXTMESSAGING, GetSentReminderMethodeString(GetSentActionEnum(IDM_TEXTMESSAGING)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_TELEVOX, GetSentReminderMethodeString(GetSentActionEnum(IDM_TELEVOX)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_LETTERWRITING, GetSentReminderMethodeString(GetSentActionEnum(IDM_LETTERWRITING)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_RECALL, GetSentReminderMethodeString(GetSentActionEnum(IDM_RECALL)));
			mnu.InsertMenu(nIndex++, dwWritePermitted, IDM_INPERSON, GetSentReminderMethodeString(GetSentActionEnum(IDM_INPERSON)));
			CRect rc;
			CWnd *pWnd = GetDlgItem(IDI_BTN_REMINDERSENT);
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
			}
		}
	}NxCatchAll(__FUNCTION__);
}


// based on menu selection add record
void CPatientReminderSenthistoryDlg::OnMenuAction(UINT nID)
{
	try {
		
		long   nReminderMethod =	GetSentActionEnum(nID);
		// check if it is valid menu id
		if (nReminderMethod > 0 && nReminderMethod < 12)
		{
			_RecordsetPtr rsPtr = CreateParamRecordset(" SET NOCOUNT ON; "
				"INSERT INTO PatientRemindersSentT(PatientID, UserID, ReminderMethod)  "
				" VALUES ({INT},{INT},{INT} );"
				" SET NOCOUNT OFF; "
				"SELECT PatientRemindersSentT.ID, PatientRemindersSentT.ReminderDate, "
				" PatientRemindersSentT.UserID, UsersT.UserName, PatientRemindersSentT.ReminderMethod, "
				" PatientRemindersSentT.PatientID, ISNULL((Select Top 1  PatientRemindersSentID From NoteInfoT WHERE PatientRemindersSentID = PatientRemindersSentT.ID), 0) AS IsNOTEExist "
				" FROM  PatientRemindersSentT INNER JOIN "
				" UsersT ON PatientRemindersSentT.UserID = UsersT.PersonID  WHERE PatientRemindersSentT.ID =SCOPE_IDENTITY() "
				, m_nPatientId, GetCurrentUserID(), nReminderMethod);
			if (!rsPtr->eof)
			{
				AddRowToList(rsPtr);
				// (s.tullis 2015-06-23 14:17) - PLID 66197 - Send patient reminder HL7 Patient Reminder creation
				SendPatientReminderHL7Message(AdoFldLong(rsPtr, "PatientID"), AdoFldLong(rsPtr, "ID"));
			}
		}
	} NxCatchAll(__FUNCTION__);
}



void CPatientReminderSenthistoryDlg::OnBnClickedBtnCloseReminderHistory()
{
	try {
		CNxDialog::OnCancel();
	} NxCatchAll(__FUNCTION__);
}
