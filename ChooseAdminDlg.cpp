// ChooseAdminDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ChooseAdminDlg.h"
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CChooseAdminDlg dialog


CChooseAdminDlg::CChooseAdminDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseAdminDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseAdminDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseAdminDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseAdminDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseAdminDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseAdminDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseAdminDlg message handlers

BOOL CChooseAdminDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_UserList = BindNxDataListCtrl(this,IDC_USER_LIST,GetRemoteData(),TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseAdminDlg::OnOK() 
{
	if(m_UserList->GetCurSel() == -1) {
		AfxMessageBox("Please select a user to be an Administrator.\n\n"
			"If you cannot make a selection at this time, you may cancel this screen and still log in normally.");
		return;
	}

	if(!SaveAdministrator())
		return;
	
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CChooseAdminDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CChooseAdminDlg)
	ON_EVENT(CChooseAdminDlg, IDC_USER_LIST, 3 /* DblClickCell */, OnDblClickCellUserList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CChooseAdminDlg::OnDblClickCellUserList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;
	
	//if the save works, close the dialog
	if(SaveAdministrator())
		CDialog::OnOK();
}

BOOL CChooseAdminDlg::SaveAdministrator() {

	try {
		
		long UserID = m_UserList->GetValue(m_UserList->GetCurSel(),0).lVal;

		// (b.savon 2015-12-18 10:00) - PLID 67705 - Change Practice to call our new VerifyPassword API method instead of calling SQL from within C++ code.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Username FROM UsersT WHERE PersonID = {INT}",UserID);

		CString strUser = AdoFldString(rs, "Username");

		CString strEnteredPassword, str;		
		str.Format("To enable the Administrator status for the username '%s',\n"
			"you must first enter the password for '%s'.",strUser,strUser);
		AfxMessageBox(str);

		str.Format("Password for '%s':",strUser);
		InputBox(this, str, strEnteredPassword, "", true);
		CWaitCursor cwait;
		if (theApp.InitializeAPI(strEnteredPassword, this) == FALSE) {
			return FALSE;
		}

		// Validate the password, look at any location
		NexTech_Accessor::_ValidateLoginCredentialsResultPtr pResult = GetAPI()->ValidateLoginCredentials(GetAPISubkey(), AsBstr(strUser), AsBstr(strEnteredPassword));

		if (pResult == NULL) {
			return FALSE;
		}
		else {
			if (pResult->GetValid() == VARIANT_FALSE) {
				// (j.jones 2008-11-19 12:46) - PLID 28578 - passwords are now case-sensitive so we must warn accordingly
				MsgBox("Invalid password. You must enter the user's password in order to enable their Administrator status.\n\n"
					"Be sure to use the correct uppercase and lowercase characters in the password.");
				return FALSE;
			}
			else {
				ExecuteParamSql("UPDATE UsersT SET Administrator = 1 WHERE PersonID = {INT}", UserID);

				return TRUE;
			}
		}

	}NxCatchAll("Error setting Administrator status.");

	return FALSE;
}