// SelectBillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectBillDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectBillDlg dialog


CSelectBillDlg::CSelectBillDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectBillDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectBillDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectBillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectBillDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectBillDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectBillDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectBillDlg message handlers

BOOL CSelectBillDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pBillList = BindNxDataListCtrl(this, IDC_BILL_LIST, GetRemoteData(), false);
	m_pBillList->WhereClause = _bstr_t(m_strWhere);
	m_pBillList->Requery();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectBillDlg::OnOK() 
{
	if(m_pBillList->CurSel == -1) m_nBillID = -1;
	else m_nBillID = VarLong(m_pBillList->GetValue(m_pBillList->CurSel, 0));

	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CSelectBillDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectBillDlg)
	ON_EVENT(CSelectBillDlg, IDC_BILL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedBillList, VTS_I2)
	ON_EVENT(CSelectBillDlg, IDC_BILL_LIST, 16 /* SelChosen */, OnSelChosenBillList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectBillDlg::OnRequeryFinishedBillList(short nFlags) 
{
	m_pBillList->CurSel = 0;
	OnSelChosenBillList(0);
}

void CSelectBillDlg::OnSelChosenBillList(long nRow) 
{
	if(nRow == -1) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	
}
