// CodeCorrectSetupDlg.cpp : implementation file
//
// (d.singleton 2012-04-23 11:08) - PLID 49336 added new dialog

#include "stdafx.h"
#include "Practice.h"
#include "CodeCorrectSetupDlg.h"


// CCodeCorrectSetupDlg dialog

IMPLEMENT_DYNAMIC(CCodeCorrectSetupDlg, CNxDialog)

CCodeCorrectSetupDlg::CCodeCorrectSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCodeCorrectSetupDlg::IDD, pParent)
{
	m_strPassword = "";
}

CCodeCorrectSetupDlg::~CCodeCorrectSetupDlg()
{
}

void CCodeCorrectSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PASSWORD_BUTTON, m_btnSetPassword);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CODE_CORRECT_USERNAME, m_eUsername);
}

BEGIN_MESSAGE_MAP(CCodeCorrectSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CCodeCorrectSetupDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CCodeCorrectSetupDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_PASSWORD_BUTTON, &CCodeCorrectSetupDlg::OnBnClickedPasswordButton)
END_MESSAGE_MAP()

BOOL CCodeCorrectSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSetPassword.AutoSet(NXB_MODIFY);

		//set limit of text box
		m_eUsername.SetLimitText(250);

		//load data if there is already a username and password record
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 * FROM CodeCorrectT");
		if(!prs->eof)
		{
			CString strUsername = AdoFldString(prs, "UserName", "");
			SetDlgItemText(IDC_CODE_CORRECT_USERNAME, strUsername);

			//decrypt password and store in mem variable
			_variant_t varPassword = prs->Fields->Item["Password"]->Value;
			m_strPassword = DecryptStringFromVariant(varPassword);
		}
		UpdateButtonAppearance();
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// CCodeCorrectSetupDlg message handlers

void CCodeCorrectSetupDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CCodeCorrectSetupDlg::OnBnClickedOk()
{
	try {
		CString strUsername;
		GetDlgItemText(IDC_CODE_CORRECT_USERNAME, strUsername);
		strUsername.Trim();
		if(strUsername.IsEmpty()) {
			AfxMessageBox("Please enter a username before closing");
			return;
		}
		if(m_strPassword.IsEmpty()) {
			AfxMessageBox("Please enter a password before closing");
			return;
		}
		
		//encrypt the password
		_variant_t varEncryptedPassword = EncryptStringToVariant(m_strPassword);
		VARIANT varPassword = varEncryptedPassword.Detach();

		if(ReturnsRecords("SELECT * FROM CodeCorrectT")) {
			//already have a record there so just update it
			ExecuteParamSql("UPDATE CodeCorrectT SET Password = {VARBINARY}, Username = {STRING}", varPassword, strUsername);
		}
		else {
			//no records so insert one
			ExecuteParamSql("INSERT INTO CodeCorrectT ( Password, Username ) VALUES ( {VARBINARY}, {STRING} )", varPassword, strUsername);
		}
	}NxCatchAll(__FUNCTION__);
		
	OnOK();
}

void CCodeCorrectSetupDlg::OnBnClickedPasswordButton()
{
	try {
		CString strResult;
		if(IDOK == InputBoxLimited(this, "Please input password", strResult, "", 250, true, false, "Cancel")) {
			m_strPassword = strResult.Trim();
			UpdateButtonAppearance();
		}
	}NxCatchAll(__FUNCTION__);
}

void CCodeCorrectSetupDlg::UpdateButtonAppearance()
{
	if(m_strPassword.IsEmpty()) {
		//Update the button text to 'Set Password'
		m_btnSetPassword.SetWindowText("Set Password");
		m_btnSetPassword.SetTextColor(RGB(255, 0, 0));
	}
	else {
		//Update the button text to 'Change Password'
		m_btnSetPassword.SetWindowTextA("Change Password");
		m_btnSetPassword.SetTextColor(0x008000);
	}
}