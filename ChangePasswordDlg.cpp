// ChangePasswordDlg.cpp : implementation file
// 

#include "stdafx.h"
#include "practice.h"
#include "ChangePasswordDlg.h"
#include "NxSecurity.h"
#include "AuditTrail.h"
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CChangePasswordDlg dialog


CChangePasswordDlg::CChangePasswordDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChangePasswordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangePasswordDlg)
	m_nUserID = GetCurrentUserID();
	m_strUserName = GetCurrentUserName();
	m_nLocationID = GetCurrentLocationID(); // (b.savon 2015-12-16 09:29) - PLID 67718
	//}}AFX_DATA_INIT
}


void CChangePasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangePasswordDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_OLD_PASSWORD, m_nxeditOldPassword);
	DDX_Control(pDX, IDC_SELF_PASSWORD, m_nxeditSelfPassword);
	DDX_Control(pDX, IDC_CONFIRM_PASSWORD, m_nxeditConfirmPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangePasswordDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChangePasswordDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangePasswordDlg message handlers

void CChangePasswordDlg::SetUserID(long nID)
{
	m_nUserID = nID;
	m_strUserName = GetCurrentUserName();
}

void CChangePasswordDlg::SetUserName(const CString& strName)
{
	m_strUserName = strName;
}

// (b.savon 2015-12-16 09:29) - PLID 67718
void CChangePasswordDlg::SetLocationID(long nID)
{
	m_nLocationID = nID;
}

CString CChangePasswordDlg::GetNewPassword()
{
	return m_strNewPassword;
}

BOOL CChangePasswordDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	this->SetWindowText("User - " + m_strUserName);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangePasswordDlg::OnOK() 
{
	try {
		CString strOldPassword, strConfirm;
		GetDlgItemText(IDC_SELF_PASSWORD, m_strNewPassword);
		GetDlgItemText(IDC_CONFIRM_PASSWORD, strConfirm);
		GetDlgItemText(IDC_OLD_PASSWORD, strOldPassword);
	
		// (b.savon 2015-12-14 16:46) - PLID 67718 - Change Practice to call our new ChangePassword API method instead of calling SQL from C++ code.
		// Create argument
		NexTech_Accessor::_ChangePasswordPtr pChangePassword(__uuidof(NexTech_Accessor::ChangePassword));
			// Create User member of argument
			NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));
			CString strUserID;
			strUserID.Format("%li", m_nUserID);
			CString strLocationID;
			strLocationID.Format("%li", m_nLocationID);
			pUser->PutID(AsBstr(strUserID));
			pUser->Putusername(AsBstr(m_strUserName));
			pUser->PutlocationID(AsBstr(strLocationID));
			pChangePassword->User = pUser;
		// Fill remaining arugment fields
		pChangePassword->PutCurrentPassword(AsBstr(strOldPassword));
		pChangePassword->PutNewPassword(AsBstr(m_strNewPassword));
		pChangePassword->PutConfirmNewPassword(AsBstr(strConfirm));

		// Change the password
		NexTech_Accessor::_ChangePasswordResultPtr pResult = GetAPI()->ChangePassword(GetAPISubkey(), GetAPILoginToken(), pChangePassword);

		if (pResult == NULL) {
			MessageBox("Unspecified error changing password.  Your password has not been changed.", "Nextech", MB_ICONERROR | MB_OK);
		}
		else {
			if (pResult->Getstatus() != NexTech_Accessor::ChangePasswordStatus_Success) {
				MessageBox(pResult->GetMessageA(), "Nextech", MB_ICONERROR | MB_OK);
			}
			else {
				MessageBox(pResult->GetMessageA(), "Nextech", MB_ICONINFORMATION | MB_OK);
				SetCurrentUserPassword(m_strNewPassword);
				SetCurrentUserPasswordVerified(TRUE);
				CNxDialog::OnOK();
			}
		}

	} NxCatchAll("Error changing password.  Your password has not been changed.");
}
