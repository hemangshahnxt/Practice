// SharedPathConflictDlg.cpp : implementation file
//
// (c.haag 2013-01-15) - PLID 59257 - Initial implementation. This dialog will appear
// if this workstation has multiple dock subkeys with the same shared path.

#include "stdafx.h"
#include "Practice.h"
#include "PracticeRc.h"
#include "SharedPathConflictDlg.h"


// CSharedPathConflictDlg dialog

IMPLEMENT_DYNAMIC(CSharedPathConflictDlg, CNxDialog)

CSharedPathConflictDlg::CSharedPathConflictDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSharedPathConflictDlg::IDD, pParent)
{

}

CSharedPathConflictDlg::~CSharedPathConflictDlg()
{
}

void CSharedPathConflictDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHAREDPATHCONFLICT_1, m_nxs1);
	DDX_Control(pDX, IDC_SHAREDPATHCONFLICT_2, m_nxs2);
	DDX_Control(pDX, IDOK, m_nxbOK);
}


BEGIN_MESSAGE_MAP(CSharedPathConflictDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SHAREDPATHCONFLICT_EMAIL, &CSharedPathConflictDlg::OnBnClickedSharedpathconflictEmail)
END_MESSAGE_MAP()


// CSharedPathConflictDlg message handlers

BOOL CSharedPathConflictDlg::OnInitDialog() 
{
	try
	{
		__super::OnInitDialog();
		m_nxbOK.AutoSet(NXB_CLOSE);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSharedPathConflictDlg::OnBnClickedSharedpathconflictEmail()
{
	try
	{
		ShellExecute(GetSafeHwnd(), NULL, "mailto:allsupport@nextech.com?Subject=Shared Path Conflict Error", NULL, "", SW_MAXIMIZE);
	}
	NxCatchAll(__FUNCTION__);
}
