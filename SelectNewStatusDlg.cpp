// SelectNewStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "SelectNewStatusDlg.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
   
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CSelectNewStatusDlg dialog


CSelectNewStatusDlg::CSelectNewStatusDlg(CWnd* pParent)
	: CNxDialog(CSelectNewStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectNewStatusDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectNewStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectNewStatusDlg)
	DDX_Control(pDX, IDC_NEW_STATUS_CAP, m_nxstaticNewStatusCap);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectNewStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectNewStatusDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectNewStatusDlg message handlers

BOOL CSelectNewStatusDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pNewStatusList = BindNxDataListCtrl(IDC_NEW_STATUSES, false);
	CString strWhere;
	strWhere.Format("ID <> %li AND ID <> -1", m_nDoomedID);
	m_pNewStatusList->WhereClause = _bstr_t(strWhere);
	m_pNewStatusList->Requery();

	_RecordsetPtr rsDoomedName = CreateRecordset("SELECT Name FROM LadderStatusT WHERE ID = %li", m_nDoomedID);
	CString strCaption;
	strCaption.Format("The status '%s' is in use.  Please select a new status.  All ladders which currently have a status of '%s' will be set to the status you select.", AdoFldString(rsDoomedName, "Name"), AdoFldString(rsDoomedName, "Name"));
	SetDlgItemText(IDC_NEW_STATUS_CAP, strCaption);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSelectNewStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectNewStatusDlg)
	ON_EVENT(CSelectNewStatusDlg, IDC_NEW_STATUSES, 18 /* RequeryFinished */, OnRequeryFinishedNewStatuses, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectNewStatusDlg::OnRequeryFinishedNewStatuses(short nFlags) 
{
	m_pNewStatusList->SetSelByColumn(0, (long)1);
}

void CSelectNewStatusDlg::OnOK() 
{
	m_nNewID = VarLong(m_pNewStatusList->GetValue(m_pNewStatusList->CurSel, 0));

	CDialog::OnOK();
}

void CSelectNewStatusDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
