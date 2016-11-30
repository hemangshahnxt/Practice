// GetNewIDName.cpp : implementation file
//

#include "stdafx.h"
#include "GetNewIDName.h"
#include "globalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetNewIDName dialog


CGetNewIDName::CGetNewIDName(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGetNewIDName::IDD, pParent)
{
	m_strCaption = "Enter a Name for the New Item:";
	m_nMaxLength = -1;
	//{{AFX_DATA_INIT(CGetNewIDName)
	//}}AFX_DATA_INIT
}


void CGetNewIDName::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetNewIDName)
	DDX_Control(pDX, IDC_NEWNAME, m_Name);
	DDX_Control(pDX, IDC_CAPTION, m_nxstaticCaption);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetNewIDName, CNxDialog)
	//{{AFX_MSG_MAP(CGetNewIDName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetNewIDName message handlers

void CGetNewIDName::OnOK() 
{
	CString NewName;
	GetDlgItemText(IDC_NEWNAME, NewName);
	m_pNewName->Format("%s",NewName);

	if(m_nMaxLength != -1 && m_pNewName->GetLength() > m_nMaxLength) {
		MsgBox("This field can be no longer than %li characters.", m_nMaxLength);
		return;
	}
	CDialog::OnOK();
}

BOOL CGetNewIDName::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	SetDlgItemText (IDC_CAPTION, m_strCaption);
	SetDlgItemText (IDC_NEWNAME, *m_pNewName);

	((CNxEdit*)GetDlgItem(IDC_NEWNAME))->LimitText(m_nMaxLength);

	m_Name.SetFocus();
	m_Name.SetSel(0, -1);
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

