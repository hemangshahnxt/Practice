// LogManagementDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "LogManagementDlg.h"
#include "InternationalUtils.h"
#include "EditLogTime.h"

using namespace ADODB;
using namespace NXDATALISTLib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////
//Columns
#define COL_ID			0
#define COL_USERID		1
#define COL_USERNAME	2
#define COL_CLOCKIN		3
#define COL_CLOCKOUT	4
#define COL_LOCATIONID	5
#define COL_LOCATIONNAME	6

//user list
#define COL_USER_ID		0
#define COL_USER_NAME	1

/////////////////////////////////////////////////////////////////////////////
// CLogManagementDlg dialog
#define ID_DELETE_LOG	WM_USER + 100
#define ID_ADD_LOG		WM_USER + 101
#define ID_EDIT_LOG		WM_USER + 102

CLogManagementDlg::CLogManagementDlg(CWnd* pParent)
	: CNxDialog(CLogManagementDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogManagementDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLogManagementDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogManagementDlg)
	DDX_Control(pDX, IDC_BTN_ADDNEWENTRY, m_btnAddEntry);
	DDX_Control(pDX, IDC_ALLDATES, m_btnAllDates);
	DDX_Control(pDX, IDC_PROMPT_LOG, m_btnPromptLog);
	DDX_Control(pDX, IDC_DATE_RANGE_OPTIONS, m_btnDateRangeOptions);
	DDX_Control(pDX, IDC_FROM, m_dtpFromCtrl);
	DDX_Control(pDX, IDC_TO, m_dtpToCtrl);
	DDX_Control(pDX, IDC_LABEL_FROMDATE, m_nxstaticLabelFromdate);
	DDX_Control(pDX, IDC_LABEL_TODATE, m_nxstaticLabelTodate);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLogManagementDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLogManagementDlg)
	ON_BN_CLICKED(IDC_ALLDATES, OnAlldates)
	ON_BN_CLICKED(IDC_DATE_RANGE_OPTIONS, OnDateRangeOptions)
	ON_BN_CLICKED(IDC_PROMPT_LOG, OnPromptLog)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_BTN_ADDNEWENTRY, OnBtnAddNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogManagementDlg message handlers

BOOL CLogManagementDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnAddEntry.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);

		//setup datalists
		m_pTimeList = BindNxDataListCtrl(IDC_TIME_LIST, false);
		m_pUserList = BindNxDataListCtrl(IDC_USER_LIST);

		//set the "all dates" checkbox and disable the other date fields
		((CButton*)GetDlgItem(IDC_ALLDATES))->SetCheck(true);
		OnAlldates();	//also handles requerying the time list

		//set the DTPickers to today
		m_dtpFromCtrl.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtpToCtrl.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		//if they don't have permission to change prompts, disable the box
		if (!(GetCurrentUserPermissions(bioLogPrompt) & SPT___W_______))
			GetDlgItem(IDC_PROMPT_LOG)->EnableWindow(false);

	} NxCatchAll("Error initializing dialog.");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLogManagementDlg::OnAlldates() 
{
	//disable all the date options
	GetDlgItem(IDC_LABEL_FROMDATE)->EnableWindow(false);
	GetDlgItem(IDC_LABEL_TODATE)->EnableWindow(false);
	GetDlgItem(IDC_FROM)->EnableWindow(false);
	GetDlgItem(IDC_TO)->EnableWindow(false);

	RequeryTime();
}

BEGIN_EVENTSINK_MAP(CLogManagementDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLogManagementDlg)
	ON_EVENT(CLogManagementDlg, IDC_USER_LIST, 16 /* SelChosen */, OnSelChosenUserList, VTS_I4)
	ON_EVENT(CLogManagementDlg, IDC_USER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUserList, VTS_I2)
	ON_EVENT(CLogManagementDlg, IDC_TIME_LIST, 7 /* RButtonUp */, OnRButtonUpTimeList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLogManagementDlg, IDC_TIME_LIST, 3 /* DblClickCell */, OnDblClickCellTimeList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CLogManagementDlg::OnOK() 
{
	CDialog::OnOK();
}

void CLogManagementDlg::OnCancel()
{
	CDialog::OnCancel();
}

void CLogManagementDlg::OnSelChosenUserList(long nRow) 
{
	try {
		if(nRow < 0){
			m_pUserList->CurSel=0;
			nRow = 0;
		}
		//requery the time list for the newly chosen user
		m_nUserID = VarLong(m_pUserList->GetValue(nRow, COL_USER_ID));

		RequeryTime();

		CString strUser = VarString(m_pUserList->GetValue(m_pUserList->GetCurSel(), COL_USER_NAME));

		if(GetRemotePropertyInt("LogTimePrompt", 0, 0, strUser, true))
			CheckDlgButton(IDC_PROMPT_LOG, true);
		else
			CheckDlgButton(IDC_PROMPT_LOG, false);

	} NxCatchAll("Error choosing user.")

}

void CLogManagementDlg::OnDateRangeOptions() 
{
	//enable all items related to date
	GetDlgItem(IDC_LABEL_FROMDATE)->EnableWindow(true);
	GetDlgItem(IDC_LABEL_TODATE)->EnableWindow(true);
	GetDlgItem(IDC_FROM)->EnableWindow(true);
	GetDlgItem(IDC_TO)->EnableWindow(true);

	//requery based on the dates in the DTPickers
	RequeryTime();
}

void CLogManagementDlg::OnRequeryFinishedUserList(short nFlags) 
{
	try {
		//select m_nUserID in the user list
		m_pUserList->SetSelByColumn(COL_USER_ID, long(m_nUserID));

		//set the prompt while we're at it
		CString strUser = VarString(m_pUserList->GetValue(m_pUserList->GetCurSel(), COL_USER_NAME));

		if(GetRemotePropertyInt("LogTimePrompt", 0, 0, strUser, true))
			CheckDlgButton(IDC_PROMPT_LOG, true);

	} NxCatchAll("Error setting User selection.");
}

void CLogManagementDlg::OnPromptLog() 
{
	try {
		int nCheck = 0;
		CString strUser = VarString(m_pUserList->GetValue(m_pUserList->GetCurSel(), COL_USER_NAME));

		if(IsDlgButtonChecked(IDC_PROMPT_LOG))
			nCheck = 1;

		SetRemotePropertyInt("LogTimePrompt", nCheck, 0, strUser);

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", m_nUserID);
		if(!rs->eof && !nCheck) {	//they are disabling the prompt, but there is an open item
			int res = AfxMessageBox("There is still an open time log for this user.  Would you like to prompt them one more time at the end of their session to close it?  "
						"Selecting 'No' will not prompt the user when they log out.  Selecting 'Yes' will prompt the user one time to end their logging.", MB_YESNO);
			if(res == IDYES) {
				SetRemotePropertyInt("LogTimePromptOneLastTime", 1, 0, VarString(m_pUserList->GetValue(m_pUserList->GetCurSel(), COL_USER_NAME)));
			}
		}

	} NxCatchAll("Error saving prompt selection.");
}

void CLogManagementDlg::RequeryTime() {

	CString strWhere, str;
	COleDateTime dtFrom, dtTo;	//IDC_FROM and IDC_TO date pickers

	//always filter on current user
	strWhere.Format("UserTimesT.UserID = %li", m_nUserID);

	if(IsDlgButtonChecked(IDC_DATE_RANGE_OPTIONS)) {	//we're filtering on dates
		dtFrom = m_dtpFromCtrl.GetValue();
		dtTo = m_dtpToCtrl.GetValue();
		COleDateTimeSpan dtSpan(1, 0, 0, 0);
		dtTo += dtSpan;	//up it one day for filtering purposes

		str.Format(" AND CheckIn >= '%s' AND CheckIn < '%s'", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		strWhere += str;
	}

	m_pTimeList->PutWhereClause(_bstr_t(strWhere));
	m_pTimeList->Requery();

}

void CLogManagementDlg::OnRButtonUpTimeList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_pTimeList->CurSel = nRow;
}

void CLogManagementDlg::DeleteLog()
{
	try{
		long nCurSel = m_pTimeList->CurSel;
		if(nCurSel != -1) {
			if(CheckCurrentUserPermissions(bioLogManage, sptDelete)) {
				if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this log entry?")) {
					ExecuteSql("DELETE FROM UserTimesT WHERE ID = %li", VarLong(m_pTimeList->GetValue(nCurSel, 0)));
					//(e.lally 2006-08-04) PLID 21788 - If we are deleting the current log entry for the current user,
					//we should reset the global variable so they can start logging again in this practice session.
					//TES 9/9/2010 - PLID 40465 - Rather than checking whether this user is the one whose entry was changed, just
					// fire a tablechecker that will update that user's session wherever they are.
					if(m_pTimeList->GetValue(nCurSel, COL_CLOCKOUT).vt == VT_NULL) {
						CClient::RefreshTable(NetUtils::TimeLogStatus, m_nUserID);
					}

					m_pTimeList->RemoveRow(nCurSel);
					
				}
			}
		}
	}NxCatchAll("Error deleting log entry");
}

void CLogManagementDlg::AddLog()
{
	try{
		if(CheckCurrentUserPermissions(bioLogManage, sptWrite)) {
			CEditLogTime dlg(this);
			dlg.SetLocationID(GetCurrentLocationID());
			//TES 9/1/2010 - PLID 39233 - Pass in true, this is a new entry.
			if(dlg.DoModal(true) == IDCANCEL){
				//They cancelled, we can stop here.
				return;
			}

			long nNewID = NewNumber("UserTimesT", "ID"); 
			CString strUser = VarString(m_pUserList->GetValue(m_pUserList->GetCurSel(), COL_USER_NAME));
			CString strCheckinForSql, strCheckoutForSql;
	
			strCheckinForSql = FormatDateTimeForSql(dlg.GetStart());
			_variant_t varStart;
			varStart.vt = VT_DATE;
			varStart = dlg.GetStart();

			//TES 9/1/2010 - PLID 39233 - The end time may be NULL.
			COleDateTime dtEnd = dlg.GetEnd();
			_variant_t varEnd;
			if(dtEnd.GetStatus() == COleDateTime::valid) {
				strCheckoutForSql = "'" + FormatDateTimeForSql(dlg.GetEnd()) + "'";
				varEnd.vt = VT_DATE;
				varEnd = dlg.GetEnd();
			}
			else {
				strCheckoutForSql = "NULL";
				varEnd = g_cvarNull;
			}

			long nLocationID = dlg.GetLocationID();
			CString strLocationName = dlg.GetLocationName();

			CString strLocationID;
			if (nLocationID == -1) {
				strLocationID = "NULL";
			}
			else {
				strLocationID.Format("%li", nLocationID);
			}

			// (j.gruber 2008-06-25 09:53) - PLID 26136 - added location
			ExecuteSql("INSERT INTO UserTimesT (ID, UserID, Checkin, Checkout, LocationID) VALUES (%li, %li, '%s', %s, %s)", 
				nNewID, m_nUserID, strCheckinForSql, strCheckoutForSql, strLocationID);
			IRowSettingsPtr pRow = m_pTimeList->GetRow(sriGetNewRow);
			pRow->PutValue(COL_ID, nNewID);
			pRow->PutValue(COL_USERID, m_nUserID);
			pRow->PutValue(COL_USERNAME, (_bstr_t) strUser);
			pRow->PutValue(COL_CLOCKIN, varStart);
			pRow->PutValue(COL_CLOCKOUT, varEnd);
			pRow->PutValue(COL_LOCATIONID, nLocationID);
			pRow->PutValue(COL_LOCATIONNAME, _variant_t(strLocationName));

			m_pTimeList->AddRow(pRow);
			m_pTimeList->SetSelByColumn(COL_ID, nNewID);

			//TES 9/9/2010 - PLID 40465 - If the end time is NULL, this user is now logging, so send out a tablechecker to update
			// their session (for example, to prompt them when they close).
			if(varEnd.vt == VT_NULL) {
				CClient::RefreshTable(NetUtils::TimeLogStatus, m_nUserID);
			}
		}
	}NxCatchAll("Error creating new log entry");
}

void CLogManagementDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		//See where focus lies...
		CWnd* pWndTimeList = GetDlgItem(IDC_TIME_LIST);

		if(pWnd->GetSafeHwnd() == pWndTimeList->GetSafeHwnd()) {
			//Context menu is for the time list
			CMenu mnuPopup;
			mnuPopup.m_hMenu = CreatePopupMenu();
			BOOL bDeleteEntryExists = FALSE;

			if(GetCurrentUserPermissions(bioLogManage) & (sptWrite|sptWriteWithPass)){
				//They have permission to write
				mnuPopup.AppendMenu(MF_ENABLED, ID_ADD_LOG, "&Add New Entry");
				if(m_pTimeList->CurSel != -1) {
					mnuPopup.AppendMenu(MF_ENABLED, ID_EDIT_LOG, "&Edit Entry");
				}
			}
			else{
				//They don't have permission to write
				mnuPopup.AppendMenu(MF_DISABLED|MF_GRAYED, ID_ADD_LOG, "&Add New Entry");
				if(m_pTimeList->CurSel != -1) {
					mnuPopup.AppendMenu(MF_DISABLED|MF_GRAYED, ID_EDIT_LOG, "&Edit Entry");
				}
			}

			if(m_pTimeList->CurSel != -1) {
				//The cursor is on a row
				if(GetCurrentUserPermissions(bioLogManage) & (sptWrite|sptDeleteWithPass)) {
					//and they have permission to delete
					mnuPopup.AppendMenu(MF_ENABLED, ID_DELETE_LOG, "Delete Entry");
				}
				else{
					//and they don't have permission to delete
					mnuPopup.AppendMenu(MF_DISABLED|MF_GRAYED, ID_DELETE_LOG, "Delete Entry");
				}
				bDeleteEntryExists = TRUE;
			}
			
			

			mnuPopup.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this, NULL);
		}
		else {
			//Context menu is for something else we are not caring about at this point in time
		}
	} NxCatchAll("Error in CLogManagementDlg::OnContextMenu");
	
}

BOOL CLogManagementDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (wParam) {
			case ID_DELETE_LOG:
				DeleteLog();
				break;

			//(e.lally 2006-08-04) PLID 14772 - Provide a way to enter a log entry manually if one of
			// the users forgot to log their time.
			case ID_ADD_LOG:
				AddLog();
				break;

			case ID_EDIT_LOG:
				EditLog();
				break;

			return TRUE;
		}//end switch

	} NxCatchAll("Error in CLogManagementDlg::OnCommand");
	
	return CNxDialog::OnCommand(wParam, lParam);
}


void CLogManagementDlg::EditLog()
{
	try{
		if(CheckCurrentUserPermissions(bioLogManage, sptWrite)) {
			CEditLogTime dlg(this);
			COleDateTime dtStart, dtEnd;
			long nCurSel = m_pTimeList->GetCurSel();
			if(nCurSel < 0)
				return;
			//If they are trying to edit the log entry that has a start time and no end time, we should at least
				//warn them that it is a bad idea.
			//The only time this should really come up is if someone forgot to log out and needs someone to log them out.
			//Doing so while that user is still in Practice will cause the log management menu to become out of sync.
			_variant_t varTempClockout;
			varTempClockout = m_pTimeList->GetValue(nCurSel, COL_CLOCKOUT);
			if(varTempClockout.vt == VT_NULL){
				if(IDNO == MessageBox("This log entry is currently in use. Are you sure you want to edit it?", NULL, MB_YESNO)){
					return;
				}
			}
			dtStart = VarDateTime(m_pTimeList->GetValue(nCurSel, COL_CLOCKIN),COleDateTime::GetCurrentTime());
			COleDateTime dtInvalid;
			//TES 9/8/2009 - While we should still go ahead and warn them if the slip is open, there's still no reason
			// to force an end time to be entered.  Pass in an invalid COleDateTime if the slip is open, then they
			// can choose whether or not to enter one.
			dtInvalid.SetStatus(COleDateTime::invalid);
			dtEnd = VarDateTime(m_pTimeList->GetValue(nCurSel, COL_CLOCKOUT),dtInvalid);
			dlg.SetStart(dtStart);
			dlg.SetEnd(dtEnd);
			long nLocationID = VarLong(m_pTimeList->GetValue(nCurSel, COL_LOCATIONID), GetCurrentLocationID());
			dlg.SetLocationID(nLocationID);
			//TES 9/1/2010 - PLID 39233 - Pass in false, this is not a new entry
			if(dlg.DoModal(false) == IDCANCEL){
				//they cancelled, we can stop here.
				return;
			}

			long nID = VarLong(m_pTimeList->GetValue(nCurSel, COL_ID));
			CString strCheckinForSql, strCheckinForInterface, strCheckoutForSql, strCheckoutForInterface;
			
			strCheckinForSql = FormatDateTimeForSql(dlg.GetStart());
			_variant_t varStart;
			varStart.vt = VT_DATE;
			varStart = dlg.GetStart();
			//TES 9/8/2009 - PLID 26888 - The end time may be NULL
			_variant_t varEnd;
			dtEnd = dlg.GetEnd();
			if(dtEnd.GetStatus() == COleDateTime::valid) {
				varEnd.vt = VT_DATE;
				varEnd = dtEnd;
				strCheckoutForSql = "'" + FormatDateTimeForSql(dlg.GetEnd()) + "'";
			}
			else {
				varEnd.vt = VT_NULL;
				strCheckoutForSql = "NULL";
			}

			nLocationID = dlg.GetLocationID();
			CString strLocation;
			if (nLocationID == -1) {
				//add null
				strLocation = "NULL";
			}
			else {
				strLocation.Format("%li", nLocationID);
			}

			if (nLocationID == -2) {
				//it was inactive and they didn't change it, so don't update
				ExecuteSql("Update UserTimesT Set CheckIn = '%s', Checkout = %s WHERE ID = %li", strCheckinForSql, strCheckoutForSql, nID);
			}
			else {
				ExecuteSql("Update UserTimesT Set CheckIn = '%s', Checkout = %s, LocationID =%s WHERE ID = %li", strCheckinForSql, strCheckoutForSql, strLocation, nID);
			}

			CString strLocationName = dlg.GetLocationName();

			m_pTimeList->PutValue(nCurSel, COL_CLOCKIN, varStart);
			m_pTimeList->PutValue(nCurSel, COL_CLOCKOUT, varEnd);
			if (nLocationID != -2) {
				m_pTimeList->PutValue(nCurSel, COL_LOCATIONID, _variant_t(nLocationID));
				m_pTimeList->PutValue(nCurSel, COL_LOCATIONNAME, _variant_t(strLocationName));
			}
			m_pTimeList->Sort();
			
		}
	}NxCatchAll("Error editing log time");

}

void CLogManagementDlg::OnDblClickCellTimeList(long nRowIndex, short nColIndex) 
{
	try{
		if(nRowIndex >=0 ){
			m_pTimeList->CurSel = nRowIndex;
			EditLog();
		}
	}NxCatchAll("Error while attempting to edit from double click");
	
}

//PLID 26192  R.G.  6/4/08	-	Added button for "Add New Entry"
void CLogManagementDlg::OnBtnAddNew() 
{
	try{
		AddLog();
	}NxCatchAll("Error in CLogManagementDlg::OnBtnAddNew");
}

