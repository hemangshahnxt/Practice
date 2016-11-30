// SelectMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "SelectMasterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectMasterDlg dialog


CSelectMasterDlg::CSelectMasterDlg(CWnd* pParent)
	: CNxDialog(CSelectMasterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMasterDlg)
		m_bIsTracked = TRUE;
	//}}AFX_DATA_INIT
}


void CSelectMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMasterDlg)
	DDX_Control(pDX, IDC_PROMPT_TEXT, m_nxstaticPromptText);
	DDX_Control(pDX, IDC_PROC_WARNING, m_nxstaticProcWarning);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMasterDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectMasterDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMasterDlg message handlers

BOOL CSelectMasterDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pMasterProcs = BindNxDataListCtrl(IDC_SELECT_MASTER_PROC, false);
	CString strWhere;
	// (c.haag 2008-12-18 17:14) - PLID 32518 - Suppress inactive procedures
	strWhere.Format("Inactive = 0 AND MasterProcedureID Is Null AND ID <> %li", m_nDetailID);
	m_pMasterProcs->WhereClause = _bstr_t(strWhere);
	m_pMasterProcs->Requery();

	CString strCaption;
	strCaption.Format("Please select the new Master record for procedure %s", m_strName);
	SetDlgItemText(IDC_PROMPT_TEXT, strCaption);

	if(!m_bIsTracked) {
		GetDlgItem(IDC_PROC_WARNING)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_PROC_WARNING)->ShowWindow(SW_SHOW);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectMasterDlg::OnOK() 
{
	if(m_pMasterProcs->CurSel == -1) {
		MsgBox("You must select a procedure");
		return;
	}
	if(m_bIsTracked) {
		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to do this?  It cannot be undone!")) {
			return;
		}
	}

	m_nMasterID = VarLong(m_pMasterProcs->GetValue(m_pMasterProcs->CurSel, 0));

	CDialog::OnOK();
}

void CSelectMasterDlg::OnCancel() 
{
	CDialog::OnCancel();
}
