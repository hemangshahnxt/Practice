// MultiUserLogDlg.cpp : implementation file
//PLID 26192	6/6/08	r.galicki	-	Allows for log management of multiple users

#include "stdafx.h"
#include "practice.h"
#include "MultiUserLogDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

enum UserListCols{
	USER_NAME = 0,
	PERSON_ID,
};
/////////////////////////////////////////////////////////////////////////////
// CMultiUserLogDlg dialog


CMultiUserLogDlg::CMultiUserLogDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultiUserLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiUserLogDlg)
	//}}AFX_DATA_INIT
}


void CMultiUserLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiUserLogDlg)
	DDX_Control(pDX, IDOK, m_btnLogInOut);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_USER_PASSWORD, m_edtPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiUserLogDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiUserLogDlg)
	ON_BN_CLICKED(IDOK, OnUpdateLog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiUserLogDlg message handlers

BOOL CMultiUserLogDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnLogInOut.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//setup datalists
		m_pUserList = BindNxDataList2Ctrl(IDC_NXDL_USERS, true);

	} NxCatchAll("Error initializing dialog.");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiUserLogDlg::OnUpdateLog() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		if(pRow == NULL) {
			MessageBox("You must select a user before continuing.");
			return;
		}

		long nUserID = VarLong(pRow->GetValue(PERSON_ID));
		CString strUserName = VarString(pRow->GetValue(USER_NAME), "");
		CString strEnteredPassword;
		// (r.galicki 2008-07-28 14:40) - PLID 26192 - Changed password box to NxEdit
		m_edtPassword.GetWindowText(strEnteredPassword);

		//GetDlgItemText(IDC_USER_PASSWORD, strEnteredPassword);

		//Verify the password.
		if(!CompareSpecificUserPassword(nUserID, strEnteredPassword)) {
			// (j.jones 2008-11-19 15:12) - PLID 28578 - passwords are now case-sensitive so we must warn accordingly
			MessageBox("Invalid password entered.  Please try again.\n\n"
					"Be sure to use the correct uppercase and lowercase characters in the password.", "Invalid Password", MB_OK|MB_ICONERROR);
			return;
		}
		else
		{
			//check selected user's log status, log user in or out depending on status
			CString msg;
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", nUserID);
			if(!rs->eof)
			{
				//end the record for selected user
				long nID = AdoFldLong(rs, "ID");

				// (b.eyers 2016-01-18) - PLID - date/time from server now
				//CString end = FormatDateTimeForSql(COleDateTime::GetCurrentTime());

				//insert the end record
				// (b.eyers 2016-01-18) - PLID 67542 - checkout time is now gotten from the server
				ExecuteParamSql("UPDATE UserTimesT SET Checkout = GETDATE() WHERE ID = {INT}", nID);
				msg.Format("%s has been successfully clocked out.", strUserName);
			}
			else
			{
				//start a record for selected user
				// (j.gruber 2008-06-25 12:40) - PLID 26136 - added location
				// (b.eyers 2016-01-18) - PLID - date/time from server now
				//CString start;
				//start = FormatDateTimeForSql(COleDateTime::GetCurrentTime());
				// (r.galicki 2008-08-25 10:36) - PLID 26192 - Fixed COALESCE statement to prevent exceptions for empty UserTimesT
				// (b.eyers 2016-01-18) - PLID 67542 - checkin time is now gotten from the server
				ExecuteParamSql("DECLARE @NewID int; SET @NewID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM UserTimesT);"
								"INSERT INTO UserTimesT (ID, UserID, Checkin, Checkout, LocationID) values (@NewID, {INT}, GETDATE(), NULL, {INT})",
								nUserID, GetCurrentLocationID());
				msg.Format("%s has been successfully clocked in.", strUserName);
			}
			CClient::RefreshTable(NetUtils::TimeLogStatus, nUserID);
			MessageBox(msg);
		}
		CDialog::OnOK();

	} NxCatchAll("Error in CMultiUserLogDlg::OnUpdateLog");
}

BEGIN_EVENTSINK_MAP(CMultiUserLogDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultiUserLogDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
