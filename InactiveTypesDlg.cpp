// InactiveTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "InactiveTypesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInactiveTypesDlg dialog


CInactiveTypesDlg::CInactiveTypesDlg(CWnd* pParent)
	: CNxDialog(CInactiveTypesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactiveTypesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInactiveTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactiveTypesDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInactiveTypesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInactiveTypesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactiveTypesDlg message handlers

BEGIN_EVENTSINK_MAP(CInactiveTypesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInactiveTypesDlg)
	ON_EVENT(CInactiveTypesDlg, IDC_INACTIVE_TYPES, 3 /* DblClickCell */, OnDblClickCellInactiveTypes, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CInactiveTypesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pInactiveTypes = BindNxDataListCtrl(IDC_INACTIVE_TYPES);

	// (z.manning, 05/01/2008) - PLID 29860 - Set button styles
	m_btnClose.AutoSet(NXB_CLOSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInactiveTypesDlg::OnDblClickCellInactiveTypes(long nRowIndex, short nColIndex) 
{
	try {
		if(nRowIndex != -1) {
			if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to restore the type %s to active use?", 
				VarString(m_pInactiveTypes->GetValue(nRowIndex, 1)))) {
				ExecuteSql("UPDATE AptTypeT SET Inactive = 0 WHERE ID = %li", VarLong(m_pInactiveTypes->GetValue(nRowIndex, 0)));
				m_pInactiveTypes->Requery();
			}
		}
	}NxCatchAll("Error in CInactiveTypesDlg::OnDblClickCellInactiveTypes()");
}

void CInactiveTypesDlg::OnOK() 
{
	
	CDialog::OnOK();
}
