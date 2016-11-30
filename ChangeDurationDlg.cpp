// ChangeDurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChangeDurationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeDurationDlg dialog


CChangeDurationDlg::CChangeDurationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChangeDurationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeDurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nDefault = 0;
	m_nMinimum = 0;
}


void CChangeDurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeDurationDlg)
	DDX_Control(pDX, IDC_EDIT_DEFAULT, m_nxeditEditDefault);
	DDX_Control(pDX, IDC_EDIT_MINIMUM, m_nxeditEditMinimum);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeDurationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChangeDurationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeDurationDlg message handlers

BOOL CChangeDurationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	SetDlgItemInt(IDC_EDIT_DEFAULT, m_nDefault);
	SetDlgItemInt(IDC_EDIT_MINIMUM, m_nMinimum);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CChangeDurationDlg::Open(long nDefaultValue, long nMinimumValue)
{
	m_nDefault = nDefaultValue;
	m_nMinimum = nMinimumValue;
	return DoModal();
}

long CChangeDurationDlg::GetDefaultDuration()
{
	return m_nDefault;
}

long CChangeDurationDlg::GetMinimumDuration()
{
	return m_nMinimum;
}

void CChangeDurationDlg::OnOK() 
{
	CString str;
	GetDlgItemText(IDC_EDIT_DEFAULT, str); m_nDefault = atoi(str);
	GetDlgItemText(IDC_EDIT_MINIMUM, str); m_nMinimum = atoi(str);
	CDialog::OnOK();
}
