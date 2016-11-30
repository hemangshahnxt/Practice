// AuditSupportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AuditSupportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAuditSupportDlg dialog


CAuditSupportDlg::CAuditSupportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAuditSupportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAuditSupportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAuditSupportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAuditSupportDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_AUDIT_TEXT, m_nxeditAuditText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAuditSupportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAuditSupportDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuditSupportDlg message handlers

void CAuditSupportDlg::OnOK() 
{
	CString str;
	GetDlgItemText(IDC_AUDIT_TEXT, str);

	str.TrimLeft();
	str.TrimRight();

	if(str.IsEmpty()) {
		MsgBox("You may not enter an empty audit string.  Please enter something before pressing 'OK'.");
		return;
	}

	m_strText = str;
	
	CDialog::OnOK();
}

void CAuditSupportDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CAuditSupportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//allow them to start something
	SetDlgItemText(IDC_AUDIT_TEXT, m_strText);

	//set focus to the text list
	GetDlgItem(IDC_AUDIT_TEXT)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
