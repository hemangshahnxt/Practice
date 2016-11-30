// EnterActiveDateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EnterActiveDateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnterActiveDateDlg dialog


CEnterActiveDateDlg::CEnterActiveDateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEnterActiveDateDlg::IDD, pParent)
{
	m_bAllowCancel = false;
	m_bAllowPastDate = true;
	m_strWindowTitle = "Enter Activation Date";
	//{{AFX_DATA_INIT(CEnterActiveDateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEnterActiveDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterActiveDateDlg)
	DDX_Control(pDX, IDC_ACTIVE_DATE, m_dtcActiveDate);
	DDX_Control(pDX, IDC_PROMPT_LABEL, m_nxstaticPromptLabel);
	DDX_Control(pDX, ID_EAD_OK, m_btnEadOk);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterActiveDateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEnterActiveDateDlg)
	ON_BN_CLICKED(ID_EAD_OK, OnEadOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnterActiveDateDlg message handlers

BOOL CEnterActiveDateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.SetWindowText("Close");
	m_btnOk.AutoSet(NXB_CLOSE);
	m_btnEadOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	// (z.manning, 02/11/2008) - PLID 28885 - You can now specify the window's title.
	SetWindowText(m_strWindowTitle);
	SetDlgItemText(IDC_PROMPT_LABEL, m_strPrompt);

	m_dtcActiveDate.SetTime(m_dtDate);

	if(m_bAllowCancel) {
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(ID_EAD_OK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(ID_EAD_OK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEnterActiveDateDlg::OnOK() 
{
	m_dtcActiveDate.GetTime(m_dtDate); //This wouldn't compile without the .m_dt

	if(!m_bAllowPastDate) {
		//this includes today (since the start of today is really "past")
		if(m_dtDate < COleDateTime::GetCurrentTime()) {
			//not allowed
			MsgBox("Past dates are not allowed to be entered here.");
			return;
		}
	}

	CNxDialog::OnOK();
}

void CEnterActiveDateDlg::OnEadOk() 
{
	OnOK();
}

void CEnterActiveDateDlg::OnCancel() 
{	
	if(m_bAllowCancel) CNxDialog::OnCancel();
}
