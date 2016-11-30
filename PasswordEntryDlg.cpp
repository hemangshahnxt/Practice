// PasswordEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PasswordEntryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPasswordEntryDlg dialog


CPasswordEntryDlg::CPasswordEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPasswordEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPasswordEntryDlg)
	m_EnteredPassword = _T("");
	//}}AFX_DATA_INIT
}


void CPasswordEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasswordEntryDlg)
	DDX_Text(pDX, IDC_PASSWORD_BOX, m_EnteredPassword);
	DDX_Control(pDX, IDC_PASSWORD_BOX, m_nxeditPasswordBox);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPasswordEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPasswordEntryDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPasswordEntryDlg message handlers


bool CPasswordEntryDlg::OpenPassword(CString &strPass, const CString& strWindowText /* = "Please Enter Password"*/)
{
	m_strWindowText = strWindowText;
	if (DoModal() == IDOK)
	{
		strPass = m_EnteredPassword;
		return true;
	}
	else
	{
		strPass = "";
		return false;
	}
}

BOOL CPasswordEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	SetWindowText(m_strWindowText);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
