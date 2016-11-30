// SelectSenderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectSenderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectSenderDlg dialog


CSelectSenderDlg::CSelectSenderDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectSenderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectSenderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectSenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectSenderDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SUBJECT_MATTER, m_nxeditSubjectMatter);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectSenderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectSenderDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectSenderDlg message handlers

void CSelectSenderDlg::OnOK() 
{
	//make sure they selected something
	long nCurSel = m_pUserList->GetCurSel();
	if(nCurSel == -1) {
		MsgBox("You must choose a user before continuing.");
		return;
	}

	//setup the return variables
	m_strLast = VarString(m_pUserList->GetValue(nCurSel, 1), "");
	m_strFirst = VarString(m_pUserList->GetValue(nCurSel, 2), "");
	m_strMiddle = VarString(m_pUserList->GetValue(nCurSel, 3), "");
	m_strEmail = VarString(m_pUserList->GetValue(nCurSel, 4), "");
	m_strTitle = VarString(m_pUserList->GetValue(nCurSel, 5), "");

	GetDlgItemText(IDC_SUBJECT_MATTER, m_strSubjectMatter);

	CNxDialog::OnOK();
}

BOOL CSelectSenderDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/25/2008) - PLID 29795 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_pUserList = BindNxDataListCtrl(this, IDC_USERS_LIST, GetRemoteData(), true);

	if(m_pUserList->SetSelByColumn(0, (long)GetCurrentUserID()) == -1) {
		//failed, set the first user in the list
		m_pUserList->PutCurSel(0);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectSenderDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}
