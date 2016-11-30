// ChangeLocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ChangeLocationDlg.h"
#include "GlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeLocationDlg dialog

using namespace ADODB;

CChangeLocationDlg::CChangeLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChangeLocationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeLocationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChangeLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeLocationDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CURR_LOCATION_NAME, m_nxstaticCurrLocationName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeLocationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChangeLocationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeLocationDlg message handlers

BOOL CChangeLocationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_pLocList = BindNxDataListCtrl(this,IDC_CHANGE_LOCATION_LIST,GetRemoteData(),false);
	CString str;
	str.Format("Managed = 1 AND Active = 1 AND ID IN (SELECT LocationID FROM UserLocationT WHERE PersonID = %li)",GetCurrentUserID());
	m_pLocList->WhereClause = _bstr_t(str);	
	m_pLocList->Requery();
	
	str.Format("Change the location for user '%s' from\n'%s' to:",GetCurrentUserName(),GetCurrentLocationName());
	SetDlgItemText(IDC_CURR_LOCATION_NAME,str);

	long nLocID = -1;
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND ID <> %li AND ID IN (SELECT LocationID FROM UserLocationT WHERE PersonID = %li) ORDER BY Name",GetCurrentLocationID(), GetCurrentUserID());
	if(!rs->eof) {
		nLocID = AdoFldLong(rs, "ID");
	}
	else {
		nLocID = GetCurrentLocationID();
	}
	rs->Close();
	
	m_pLocList->SetSelByColumn(0,nLocID);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeLocationDlg::OnOK() 
{
	if(m_pLocList->CurSel == -1) {
		AfxMessageBox("You must choose a location to continue.");
		return;
	}

	long nLocID = VarLong(m_pLocList->GetValue(m_pLocList->GetCurSel(),0),GetCurrentLocationID());
	SetCurrentLocationID(nLocID);
	SetCurrentLocationName(VarString(m_pLocList->GetValue(m_pLocList->GetCurSel(),1),GetCurrentLocationName()));

	GetMainFrame()->LoadTitleBarText();
	
	CDialog::OnOK();
}
