// UserVerifyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UserVerifyDlg.h"

// CUserVerifyDlg dialog

// (j.jones 2013-08-08 09:23) - PLID 57915 - created

IMPLEMENT_DYNAMIC(CUserVerifyDlg, CNxDialog)

using namespace NXDATALIST2Lib;

enum UserNameComboColumns {
	unccUserID = 0,
	unccTrueUserName,
	unccDisplayUserName,
};

CUserVerifyDlg::CUserVerifyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUserVerifyDlg::IDD, pParent)
{
	m_strDialogTitle = "Please Select A User";
	m_strLabelText = "Select your user name and enter a password:";
	m_strUserWhereClause = "PersonT.Archived = 0";
	m_nDefaultUserID = -1;

	m_nSelectedUserID = -1;
	m_strSelectedUserName = "";
}

CUserVerifyDlg::~CUserVerifyDlg()
{
}

void CUserVerifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_USER_VERIFY_LABEL, m_staticUserVerifyLabel);
	DDX_Control(pDX, IDC_USER_VERIFY_PASSWORD_BOX, m_editPasswordBox);
}

BEGIN_MESSAGE_MAP(CUserVerifyDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
END_MESSAGE_MAP()

// CUserVerifyDlg message handlers

int CUserVerifyDlg::DoModal(CString strDialogTitle /*= "Please Select A User"*/, CString strLabelText /*= "Select your user name and enter a password:"*/,
							long nDefaultUserID /*= -1*/, CString strUserWhereClause /*= "PersonT.Archived = 0"*/)
{
	m_strDialogTitle = strDialogTitle;
	m_strLabelText = strLabelText;
	m_strUserWhereClause = strUserWhereClause;
	m_nDefaultUserID = nDefaultUserID;
	
	return CNxDialog::DoModal();
}

BOOL CUserVerifyDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CUserVerifyDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DisplayLoginUsersInUpperAndLowerCases' "
			")",
			_Q(GetCurrentUserName()));
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetWindowText(m_strDialogTitle);
		m_staticUserVerifyLabel.SetWindowText(m_strLabelText);

		m_UserCombo = BindNxDataList2Ctrl(IDC_USER_VERIFY_COMBO, false);
		m_UserCombo->PutWhereClause(_bstr_t(m_strUserWhereClause));
		
		// Follow the login logic that we did for CCHIT, though it is nonsense.
		// If the DisplayLoginUsersInUpperAndLowerCases setting is enabled
		// (which can only be done through a query), then show each username once in all uppercase,
		// and once in all lowercase, ignoring whether or not the actual name is a mixed case.
		if (GetRemotePropertyInt("DisplayLoginUsersInUpperAndLowerCases", 0, 0, "<None>", true) == 1) {
			//we've enabled this silly preference, so rather than requerying, load up the results
			//in a recordset so we can manually manipulate the usernames and add rows
			CString strFromClause = (LPCTSTR)m_UserCombo->FromClause;
			CString strWhereClause = (LPCTSTR)m_UserCombo->WhereClause;
			strFromClause.TrimRight(" ");
			strWhereClause.TrimRight(" ");
			if (strFromClause.IsEmpty() || strWhereClause.IsEmpty()) {
				//safety check, should be impossible
				ASSERT(FALSE);
				//simply requery
				m_UserCombo->Requery();
			}
			else {
				CString strSql;
				strSql.Format("SELECT UsersT.PersonID, UsersT.UserName "
					"FROM %s WHERE %s", strFromClause, strWhereClause);
				ADODB::_RecordsetPtr rs = CreateRecordsetStd(strSql);

				m_UserCombo->Clear();

				while (!rs->eof) {
					long nPersonID = AdoFldLong(rs, "PersonID");
					CString strUserName = AdoFldString(rs, "UserName", "");

					//add the default case
					{
						IRowSettingsPtr pRow = m_UserCombo->GetNewRow();
						pRow->PutValue(unccUserID, nPersonID);
						pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
						pRow->PutValue(unccDisplayUserName, (LPCTSTR)strUserName);
						m_UserCombo->AddRowSorted(pRow, NULL);
					}

					//add with lowercase name
					{
						CString strLower = strUserName;
						strLower.MakeLower();
						//don't add if this case is exactly the same as our regular case
						if (strLower != strUserName) {
							IRowSettingsPtr pRow = m_UserCombo->GetNewRow();
							pRow->PutValue(unccUserID, nPersonID);
							pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
							pRow->PutValue(unccDisplayUserName, (LPCTSTR)strLower);
							m_UserCombo->AddRowSorted(pRow, NULL);
						}
					}

					//add with uppercase name
					{
						CString strUpper = strUserName;
						strUpper.MakeUpper();
						// don't add if this case is exactly the same as our regular case
						if (strUpper != strUserName) {
							IRowSettingsPtr pRow = m_UserCombo->GetNewRow();
							pRow->PutValue(unccUserID, nPersonID);
							pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
							pRow->PutValue(unccDisplayUserName, (LPCTSTR)strUpper);
							m_UserCombo->AddRowSorted(pRow, NULL);
						}
					}

					rs->MoveNext();
				}
				rs->Close();

				if (m_nDefaultUserID != -1) {
					m_UserCombo->SetSelByColumn(unccUserID, m_nDefaultUserID);
				}
			}
		}
		else {
			//normal requery
			m_UserCombo->Requery();
		}

		//if nothing was selected, select the first row
		if(m_UserCombo->GetCurSel() == NULL) {
			m_UserCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			m_UserCombo->PutCurSel(m_UserCombo->GetFirstRow());
		}

		//never auto-remember a password here

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserVerifyDlg::OnOk()
{
	try {

		//get the selected user
		IRowSettingsPtr pRow = m_UserCombo->GetCurSel();
		if(pRow == NULL) {
			MsgBox("Invalid username. Please choose one from the drop-down list.");
			CWnd *pWnd = GetDlgItem(IDC_USER_VERIFY_COMBO);
			pWnd->SetFocus();
			return;
		}

		long nUserID = VarLong(pRow->GetValue(unccUserID));
		CString strUserName = VarString(pRow->GetValue(unccTrueUserName));

		//get the password they entered
		CString strEnteredPassword;
		GetDlgItemText(IDC_USER_VERIFY_PASSWORD_BOX, strEnteredPassword);

		BOOL bLockedOut = FALSE;
		if(CompareSpecificUserPassword(nUserID, strEnteredPassword, &bLockedOut)) {
			// Success, we got the valid password
			m_nSelectedUserID = nUserID;
			m_strSelectedUserName = strUserName;
			CNxDialog::OnOK();
			return;			
		}
		else {
			// (z.manning 2016-04-19 14:51) - NX-100244 - Give a different message if they're locked out altogether
			if (bLockedOut) {
				MessageBox("This account has exceeded the maximum number of login attempts, and has been locked. Please contact an administrator in order to unlock this account."
				, "Locked Out", MB_OK | MB_ICONERROR);
			}
			else {
				//do not indicate why it failed, only that it did fail
				MessageBox("Failed to validate the credentials for this user", "Invalid Credentials", MB_OK | MB_ICONERROR);
			}
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CUserVerifyDlg, CNxDialog)
ON_EVENT(CUserVerifyDlg, IDC_USER_VERIFY_COMBO, 1, OnSelChangingUserVerifyCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CUserVerifyDlg, IDC_USER_VERIFY_COMBO, 18, CUserVerifyDlg::RequeryFinishedUserVerifyCombo, VTS_I2)
ON_EVENT(CUserVerifyDlg, IDC_USER_VERIFY_COMBO, 16, CUserVerifyDlg::SelChosenUserVerifyCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CUserVerifyDlg::OnSelChangingUserVerifyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2016-04-19 9:13) - NX-100244 - Created
void CUserVerifyDlg::RequeryFinishedUserVerifyCombo(short nFlags)
{
	try
	{
		// (z.manning 2016-04-19 9:14) - NX-100244 - Moved this to requery finished to avoid race condition
		if (m_nDefaultUserID != -1) {
			m_UserCombo->SetSelByColumn(unccUserID, m_nDefaultUserID);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2016-04-20 9:02) - NX-100244 - Created
void CUserVerifyDlg::SelChosenUserVerifyCombo(LPDISPATCH lpRow)
{
	try
	{
		// (z.manning 2016-04-20 9:02) - NX-100244 - Clear the password when a user is selected
		m_editPasswordBox.SetWindowText("");
		m_editPasswordBox.SetFocus();
	}
	NxCatchAll(__FUNCTION__);
}
