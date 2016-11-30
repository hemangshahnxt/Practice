// CustomRecordSelectServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CustomRecordSelectServiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSelectServiceDlg dialog


CCustomRecordSelectServiceDlg::CCustomRecordSelectServiceDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCustomRecordSelectServiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomRecordSelectServiceDlg)
		m_ServiceID = -1;
	//}}AFX_DATA_INIT
}


void CCustomRecordSelectServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordSelectServiceDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordSelectServiceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordSelectServiceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSelectServiceDlg message handlers

BOOL CCustomRecordSelectServiceDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_CPTCombo = BindNxDataListCtrl(this,IDC_CPT_CODE_COMBO,GetRemoteData(),true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomRecordSelectServiceDlg::OnOK() 
{
	if(m_CPTCombo->GetCurSel() == -1) {
		AfxMessageBox("Please select a service to add to the bill.");
		return;
	}

	m_ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),0));
	
	CDialog::OnOK();
}

void CCustomRecordSelectServiceDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
