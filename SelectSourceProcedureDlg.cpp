// SelectSourceProcedureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "SelectSourceProcedureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectSourceProcedureDlg dialog


CSelectSourceProcedureDlg::CSelectSourceProcedureDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectSourceProcedureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectSourceProcedureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectSourceProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectSourceProcedureDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectSourceProcedureDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectSourceProcedureDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectSourceProcedureDlg message handlers

BOOL CSelectSourceProcedureDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pProcList = BindNxDataListCtrl(this, IDC_SOURCE_PROC, GetRemoteData(), true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectSourceProcedureDlg::OnOK() 
{
	if(m_pProcList->CurSel == -1) {
		MsgBox("You must select a procedure to copy from.");
		return;
	}
	if(IDYES != MsgBox(MB_YESNO, "Are you absolutely sure you wish to overwrite all fields in the current procedure with the corresponding fields from this procedure (%s)?", VarString(m_pProcList->GetValue(m_pProcList->CurSel, 1)))) {
		return;
	}
	m_nSelectedId = VarLong(m_pProcList->GetValue(m_pProcList->CurSel, 0));
	CDialog::OnOK();
}
