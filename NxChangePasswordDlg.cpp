// NxChangePasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxChangePasswordDlg.h"
#include "NexwebLoginGeneratePasswordDlg.h"
#include "GlobalNexWebUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxChangePasswordDlg dialog
// (f.gelderloos 2013-08-07 12:20) - PLID 57914 Abstracting common functionality

CNxChangePasswordDlg::CNxChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent, boost::function<NexWebPasswordComplexity*()> fnComplexity)
	: CNxDialog(CNxChangePasswordDlg::IDD, pParent)
{
	m_nPersonID = nPersonID;
	m_strUserName = strUserName;
	// (j.armen 2013-10-03 07:08) - PLID 57914 - Store passwod complexity function
	m_fnComplexity = fnComplexity;
}


void CNxChangePasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxChangePasswordDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEXWEB_NEW_PASSWORD, m_nxeditNexwebNewPassword);
	DDX_Control(pDX, IDC_NEXWEB_CONFIRM_PASSWORD, m_nxeditNexwebConfirmPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxChangePasswordDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxChangePasswordDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GENERATE_NEW_PASSWORD, &CNxChangePasswordDlg::OnBnClickedGenerateNewPassword)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxChangePasswordDlg message handlers

BOOL CNxChangePasswordDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (j.armen 2013-10-03 07:08) - PLID 57914 - Window text is set by the overriding OnInitDialog
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (b.savon 2012-09-05 17:06) - PLID 52464 - Display requirement banner
CString CNxChangePasswordDlg::GetPasswordRequirementBanner()
{
	CString strBanner;
	try{
		// (j.armen 2013-10-03 07:08) - PLID 57914 - Use scoped ptr
		boost::scoped_ptr<NexWebPasswordComplexity> nwpComplexityRules(GetNexWebPasswordRules(GetRemoteData()));

		strBanner += "Minimum Password Length: " + FormatBannerParameter(nwpComplexityRules->nMinLength);
		strBanner += "Minimum Letters (A-Z): " + FormatBannerParameter(nwpComplexityRules->nMinLetters);
		strBanner += "Minimum Numbers (0-9): " + FormatBannerParameter(nwpComplexityRules->nMinNumbers);
		strBanner += "Minimum Uppercase Letters (A-Z): " + FormatBannerParameter(nwpComplexityRules->nMinUpper);
		strBanner += "Minimum Lowercase Letters (a-z): " + FormatBannerParameter(nwpComplexityRules->nMinLower);

	}NxCatchAll(__FUNCTION__);

	return strBanner;
}

CString CNxChangePasswordDlg::FormatBannerParameter(long nValue)
{
	CString strTemp;
	strTemp.Format("%li\r\n", nValue);
	return strTemp;
}

BOOL CNxChangePasswordDlg::ValidateAndSave() {


	try {
		//first make sure that the passwords match
		CString strPass, strConfirm;

		GetDlgItemText(IDC_NEXWEB_NEW_PASSWORD, strPass);
		GetDlgItemText(IDC_NEXWEB_CONFIRM_PASSWORD, strConfirm);

		if (strPass.IsEmpty()) {
			MessageBox("You cannot have an empty password.");
			return FALSE;
		}

		if (strPass.Compare(strConfirm) != 0)  {

			MessageBox("The password and confirm password fields do not match, please correct this.");
			return FALSE;
		}

		// (j.armen 2013-10-03 07:08) - PLID 57914 -- overridden Update Data is responsible for validating as well

		return !!UpdateData(strPass);

	}NxCatchAllCall("Error Saving Password", return FALSE;);
	return TRUE;
}

void CNxChangePasswordDlg::OnOK()
{

	if (ValidateAndSave()) {
		CDialog::OnOK();

	}
	else  {
		return;
	}
}

void CNxChangePasswordDlg::OnCancel()
{
	CDialog::OnCancel();
}

// (j.gruber 2009-01-06 17:49) - PLID 32480 - auto generate a password
void CNxChangePasswordDlg::OnBnClickedGenerateNewPassword()
{
	try {
		// (j.armen 2013-10-03 07:08) - PLID 57914 - Pass our complexity function to the generation dlg
		CNexwebLoginGeneratePasswordDlg dlg(this, m_fnComplexity);

		int nResult = dlg.DoModal();

		if (nResult == IDOK) {
			CString strPassword = dlg.m_strPassword;
			if (!strPassword.IsEmpty()) {
				SetDlgItemText(IDC_NEXWEB_NEW_PASSWORD, strPassword);
				SetDlgItemText(IDC_NEXWEB_CONFIRM_PASSWORD, strPassword);
			}
		}
	}NxCatchAll("Error in CNxChangePasswordDlg::OnBnClickedGenerateNewPassword()");
}