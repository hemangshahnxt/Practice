// SelectContactDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectContactDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum ContactColumns {
	ccID = 0, 
	ccLast = 1, 
	ccFirst = 2,
	ccMiddle = 3, 
	ccCompany = 4,
	ccTitle = 5,
};

/////////////////////////////////////////////////////////////////////////////
// CSelectContactDlg dialog


CSelectContactDlg::CSelectContactDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectContactDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectContactDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectContactDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectContactDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectContactDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectContactDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectContactDlg message handlers

void CSelectContactDlg::OnOK() 
{
	long nCurSel = m_pList->GetCurSel();

	if(nCurSel == -1) {
		MsgBox("You must select a contact before continuing.");
		return;
	}

	m_nID = VarLong(m_pList->GetValue(nCurSel, ccID));
	m_strLast = VarString(m_pList->GetValue(nCurSel, ccLast));
	m_strFirst = VarString(m_pList->GetValue(nCurSel, ccFirst));
	m_strMiddle = VarString(m_pList->GetValue(nCurSel, ccMiddle));
	m_strCompany = VarString(m_pList->GetValue(nCurSel, ccCompany));
	m_strTitle = VarString(m_pList->GetValue(nCurSel, ccTitle));

	m_strLast.TrimRight();
	m_strFirst.TrimRight();
	m_strMiddle.TrimRight();
	m_strCompany.TrimRight();
	m_strTitle.TrimRight();

	CDialog::OnOK();
}

BOOL CSelectContactDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-04-29 12:49) - PLID 29824 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pList = BindNxDataListCtrl(this, IDC_CONTACT_LIST, GetRemoteData(), true);
	}
	NxCatchAll("Error in CSelectContactDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
