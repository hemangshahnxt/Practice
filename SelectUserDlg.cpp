// SelectUserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectUserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectUserDlg dialog


CSelectUserDlg::CSelectUserDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectUserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectUserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bAllowNone = true; // PLID 22458 - Whether the (None) option should appear in our dialog.
	// default to true (standard behaviour).
}


void CSelectUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectUserDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_USER_CAPTION, m_nxstaticSelectUserCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectUserDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectUserDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectUserDlg message handlers

BOOL CSelectUserDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	SetDlgItemText(IDC_SELECT_USER_CAPTION, m_strCaption);

	try {
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pUserList = BindNxDataListCtrl(this, IDC_USER_LIST, GetRemoteData(), false);

		CString strWhereClause;
		if(m_nExcludeUser != -1) {
			strWhereClause.Format("PersonT.Archived = 0 AND PersonID <> %li ", m_nExcludeUser);
		}

		if(!m_strUserWhereClause.IsEmpty()) {
			if(strWhereClause.IsEmpty())
				strWhereClause = m_strUserWhereClause;
			else
				strWhereClause += " AND " + m_strUserWhereClause;
		}

		if(!strWhereClause.IsEmpty())
			m_pUserList->WhereClause = _bstr_t(strWhereClause);

		m_pUserList->Requery();
	}NxCatchAll("Error in CSelectUserDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSelectUserDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectUserDlg)
	ON_EVENT(CSelectUserDlg, IDC_USER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUserList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectUserDlg::OnRequeryFinishedUserList(short nFlags) 
{
	if (m_bAllowNone) {
		IRowSettingsPtr pRow = m_pUserList->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{None}"));
		m_pUserList->InsertRow(pRow, 0);
	}
	m_pUserList->CurSel = 0;
}

void CSelectUserDlg::OnOK() 
{
	if(m_pUserList->CurSel == -1) {
		AfxMessageBox("You must make a selection from the list.");
		return;
	}

	m_nSelectedUser = m_pUserList->GetValue(m_pUserList->CurSel, 0);

	CDialog::OnOK();
}
