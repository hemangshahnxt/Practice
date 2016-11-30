// CopyEmployerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CopyEmployerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum company_list {
	clCompany = 0, 
	clAddr1, 
	clAddr2, 
	clCity,
	clState,
	clZip,
} company_list;

/////////////////////////////////////////////////////////////////////////////
// CCopyEmployerDlg dialog


CCopyEmployerDlg::CCopyEmployerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCopyEmployerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyEmployerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCopyEmployerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyEmployerDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyEmployerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCopyEmployerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyEmployerDlg message handlers

void CCopyEmployerDlg::OnOK() 
{
	if(m_listCompany->CurSel == -1) {
		MsgBox("Please select a company.");
		return;
	}

	m_company = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clCompany), "");
	m_addr1 = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clAddr1), "");
	m_addr2  = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clAddr2), "");
	m_city = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clCity), "");
	m_state = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clState), "");
	m_zip = VarString(m_listCompany->GetValue(m_listCompany->CurSel, clZip), "");

	CDialog::OnOK();
}

void CCopyEmployerDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CCopyEmployerDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:53) - PLID 29863 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_listCompany = BindNxDataListCtrl(this, IDC_EMPLOYER_LIST, GetRemoteData(), false);

		if(!m_strFilterOn.IsEmpty()) {
			//they want to filter on something
			CString strWhere;
			strWhere.Format("Company = '%s'", m_strFilterOn);
			m_listCompany->PutWhereClause(_bstr_t(strWhere));
		}

		m_listCompany->Requery();
	}
	NxCatchAll("Error in CCopyEmployerDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CCopyEmployerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCopyEmployerDlg)
	ON_EVENT(CCopyEmployerDlg, IDC_EMPLOYER_LIST, 3 /* DblClickCell */, OnDblClickCellEmployerList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCopyEmployerDlg::OnDblClickCellEmployerList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_listCompany->CurSel = nRowIndex;
	OnOK();
}

void CCopyEmployerDlg::FilterOnCompany(CString strCompany)
{
	m_strFilterOn = strCompany;
}
