// PromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PromptDlg.h"
#include "EnterActiveDateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPromptDlg dialog


CPromptDlg::CPromptDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPromptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPromptDlg)
	DDX_Control(pDX, IDC_PROMPT_TEXT, m_nxeditPromptText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPromptDlg)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPromptDlg message handlers

BOOL CPromptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29860 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	((CNxEdit*)GetDlgItem(IDC_PROMPT_TEXT))->SetLimitText(255);

	SetDlgItemText(IDC_PROMPT_TEXT, m_strPrompt);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPromptDlg::OnOK() 
{
	GetDlgItemText(IDC_PROMPT_TEXT, m_strPrompt);

	if(m_strPrompt.GetLength() > 255) {
		MsgBox("The prompt can be no longer than 255 characters.");
		return;
	}

	CDialog::OnOK();
}

void CPromptDlg::OnPreview() 
{
	CString strTemp;
	GetDlgItemText(IDC_PROMPT_TEXT, strTemp);
	CEnterActiveDateDlg dlg(this);
	dlg.m_strPrompt = strTemp;
	dlg.m_dtDate.m_dt = COleDateTime::GetCurrentTime();
	dlg.DoModal();
}
