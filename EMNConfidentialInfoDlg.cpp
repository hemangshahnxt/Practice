// EMNConfidentialInfoDlg.cpp : implementation file
// (d.thompson 2009-05-18) - PLID 29909 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "EMNConfidentialInfoDlg.h"


// CEMNConfidentialInfoDlg dialog

IMPLEMENT_DYNAMIC(CEMNConfidentialInfoDlg, CNxDialog)

CEMNConfidentialInfoDlg::CEMNConfidentialInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMNConfidentialInfoDlg::IDD, pParent)
{
	m_bReadOnly = false;
}

CEMNConfidentialInfoDlg::~CEMNConfidentialInfoDlg()
{
}

void CEMNConfidentialInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CONFIDENTIAL_TEXT, m_nxeditInfo);
}


BEGIN_MESSAGE_MAP(CEMNConfidentialInfoDlg, CNxDialog)
END_MESSAGE_MAP()


// CEMNConfidentialInfoDlg message handlers
BOOL CEMNConfidentialInfoDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//interface niceties
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_nxeditInfo.SetLimitText(1000);

		//load data
		SetDlgItemText(IDC_CONFIDENTIAL_TEXT, m_strConfidentialInfo);

		//If this is set, no changes are allowed
		if(m_bReadOnly) {
			CEdit *pEdit = (CEdit*)GetDlgItem(IDC_CONFIDENTIAL_TEXT);
			pEdit->SetReadOnly(TRUE);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CEMNConfidentialInfoDlg::OnOK()
{
	try {
		//safety check - the OK button should not be accessible
		if(m_bReadOnly) {
			return;
		}

		//Just yank it back
		GetDlgItemText(IDC_CONFIDENTIAL_TEXT, m_strConfidentialInfo);

		CDialog::OnOK();

	} NxCatchAll("Error in OnInitDialog");
}
