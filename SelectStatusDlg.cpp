// SelectStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectStatusDlg.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectStatusDlg dialog


CSelectStatusDlg::CSelectStatusDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectStatusDlg)
		m_nStatusID = -1;
	//}}AFX_DATA_INIT
}


void CSelectStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectStatusDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectStatusDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectStatusDlg message handlers

BOOL CSelectStatusDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:41) - PLID 29866 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_pStatusList = BindNxDataListCtrl(this, IDC_SELECT_STATUS_LIST, GetRemoteData(), true);
		m_nStatusID = -1;
	}
	NxCatchAll("Error in CSelectStatusDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectStatusDlg::OnOK() 
{
	if(m_pStatusList->CurSel != -1) {
		m_nStatusID = VarLong(m_pStatusList->GetValue(m_pStatusList->CurSel, 0));
		CNxDialog::OnOK();
	}
	else {
		MsgBox("You must select a status!");
	}
}

BEGIN_EVENTSINK_MAP(CSelectStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectStatusDlg)
	ON_EVENT(CSelectStatusDlg, IDC_SELECT_STATUS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectStatusList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectStatusDlg::OnDblClickCellSelectStatusList(long nRowIndex, short nColIndex) 
{
	OnOK();
}
