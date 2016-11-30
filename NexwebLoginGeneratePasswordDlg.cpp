// NexwebLoginGeneratePasswordDlg.cpp : implementation file
//
// (j.gruber 2009-01-06 17:48) - PLID 32480 - created for
#include "stdafx.h"
#include "Practice.h"
#include "NexwebLoginGeneratePasswordDlg.h"
#include "NxChangePasswordDlg.h"

#include "GlobalNexWebUtils.h"

// CNexwebLoginGeneratePasswordDlg dialog

IMPLEMENT_DYNAMIC(CNexwebLoginGeneratePasswordDlg, CNxDialog)

// (j.armen 2013-10-03 07:08) - PLID 57914 - Save a function that will generate password complexity instead of an enum
CNexwebLoginGeneratePasswordDlg::CNexwebLoginGeneratePasswordDlg(CWnd* pParent, boost::function<NexWebPasswordComplexity*()> fnComplexity)
: CNxDialog(CNexwebLoginGeneratePasswordDlg::IDD, pParent)
{
	m_fnComplexity = fnComplexity;
}

CNexwebLoginGeneratePasswordDlg::~CNexwebLoginGeneratePasswordDlg()
{
}

void CNexwebLoginGeneratePasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_PASSWORD, m_btnSelectPassword);
	DDX_Control(pDX, IDC_GENERATE_NEW_PASSWORD, m_btnNewPassword);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_GENERATED_PASSWORD, m_nxstaticGeneratedPassword);
}


BEGIN_MESSAGE_MAP(CNexwebLoginGeneratePasswordDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SELECT_PASSWORD, &CNexwebLoginGeneratePasswordDlg::OnBnClickedSelectPassword)
	ON_BN_CLICKED(IDC_GENERATE_NEW_PASSWORD, &CNexwebLoginGeneratePasswordDlg::OnBnClickedGenerateNewPassword)
	ON_BN_CLICKED(IDCANCEL, &CNexwebLoginGeneratePasswordDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CNexwebLoginGeneratePasswordDlg message handlers

void CNexwebLoginGeneratePasswordDlg::OnBnClickedSelectPassword()
{
	try {
		GetDlgItemText(IDC_GENERATED_PASSWORD, m_strPassword);
		CDialog::OnOK();
	}NxCatchAll("Error in CNexwebLoginGeneratePasswordDlg::OnBnClickedSelectPassword()");
}

void CNexwebLoginGeneratePasswordDlg::OnBnClickedGenerateNewPassword()
{
	try {
		GeneratePassword();
	}NxCatchAll("Error in CNexwebLoginGeneratePasswordDlg::OnBnClickedGenerateNewPassword()");
}

void CNexwebLoginGeneratePasswordDlg::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CNexwebLoginGeneratePasswordDlg::GeneratePassword()
{
	try{
		CString strPassword = GeneratePasswordText();
		SetDlgItemText(IDC_GENERATED_PASSWORD, strPassword);
	}NxCatchAll(__FUNCTION__);
}

// (f.gelderloos 2013-08-14 14:33) - PLID 57914
CString CNexwebLoginGeneratePasswordDlg::GeneratePasswordText()
{
	// (b.savon 2012-09-05 17:56) - PLID 52463 - Generate Complexity Password
	// (f.gelderloos 2013-08-14 14:33) - PLID 57914
	// (j.armen 2013-10-03 07:08) - PLID 57914 - Use the stored function to generate complexity
	boost::scoped_ptr<NexWebPasswordComplexity> complexity(m_fnComplexity());

	CString strPassword = GetRuleAbidingPassword(*complexity);


	if (strPassword.IsEmpty()){
		ADODB::_RecordsetPtr rsPassword = CreateRecordset("GeneratePassword");
		if (!rsPassword->eof) {
			strPassword = AdoFldString(rsPassword, "GeneratedPassword", "");
		}
	}
	return strPassword;
}

BOOL CNexwebLoginGeneratePasswordDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnNewPassword.AutoSet(NXB_MODIFY);
		m_btnSelectPassword.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//set the font bigger
		extern CPracticeApp theApp;
		GetDlgItem(IDC_GENERATED_PASSWORD)->SetFont(&theApp.m_titleFont);
		//generate a new password right off the bat
		GeneratePassword();


	}NxCatchAll("CNexwebLoginGeneratePasswordDlg::OnInitDialog");

	return TRUE;
}