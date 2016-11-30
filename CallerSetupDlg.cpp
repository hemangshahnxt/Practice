// CallerSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CallerSetupDlg.h"
#include "AutoCaller.h"
#include "GlobalDataUtils.h"
#include "GetNewIDName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CCallerSetupDlg dialog


CCallerSetupDlg::CCallerSetupDlg(CWnd* pParent)
	: CNxDialog(CCallerSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCallerSetupDlg)
	//}}AFX_DATA_INIT
}


void CCallerSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallerSetupDlg)
	DDX_Text(pDX, IDC_START_MSG, m_strStartMsg);
	DDX_Text(pDX, IDC_MSG, m_strMsg);
	DDX_Text(pDX, IDC_USER0, m_strUser0);
	DDX_Text(pDX, IDC_USER1, m_strUser1);
	DDX_Text(pDX, IDC_USER2, m_strUser2);
	DDX_Text(pDX, IDC_USER3, m_strUser3);
	DDX_Text(pDX, IDC_USER4, m_strUser4);
	DDX_Text(pDX, IDC_USER5, m_strUser5);
	DDX_Text(pDX, IDC_USER6, m_strUser6);
	DDX_Text(pDX, IDC_USER7, m_strUser7);
	DDX_Text(pDX, IDC_USER8, m_strUser8);
	DDX_Text(pDX, IDC_USER9, m_strUser9);
	DDX_Control(pDX, IDC_START_MSG, m_nxeditStartMsg);
	DDX_Control(pDX, IDC_MSG, m_nxeditMsg);
	DDX_Control(pDX, IDC_LOCAL_CODE, m_nxeditLocalCode);
	DDX_Control(pDX, IDC_OUTSIDE, m_nxeditOutside);
	DDX_Control(pDX, IDC_USER1, m_nxeditUser1);
	DDX_Control(pDX, IDC_USER2, m_nxeditUser2);
	DDX_Control(pDX, IDC_USER3, m_nxeditUser3);
	DDX_Control(pDX, IDC_USER4, m_nxeditUser4);
	DDX_Control(pDX, IDC_USER5, m_nxeditUser5);
	DDX_Control(pDX, IDC_USER6, m_nxeditUser6);
	DDX_Control(pDX, IDC_USER7, m_nxeditUser7);
	DDX_Control(pDX, IDC_USER8, m_nxeditUser8);
	DDX_Control(pDX, IDC_USER9, m_nxeditUser9);
	DDX_Control(pDX, IDC_USER0, m_nxeditUser0);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_RENAME, m_btnRename);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCallerSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCallerSetupDlg)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCallerSetupDlg message handlers

void CCallerSetupDlg::OnOK()
{
	CDialog::OnOK();
}

void CCallerSetupDlg::OnCancel()
{
	CDialog::OnCancel();
}

BOOL CCallerSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_listGroup = BindNxDataListCtrl(IDC_GROUP_LIST);
	m_listUser = BindNxDataListCtrl(IDC_USER_LIST);
	m_listGroup->CurSel = 0;
	m_listUser->CurSel = 0;
	
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnRename.AutoSet(NXB_MODIFY);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	Load();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCallerSetupDlg::Load()
{

	if (m_listGroup->GetCurSel()== -1)
		return;
	long m_id = m_listGroup->GetValue(m_listGroup->GetCurSel(), 0);
	
	try{
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT ID, GroupName, StartMessage, Rings, Message, User0, User1, User2, "
				"User3, User4, User5, User6, User7, User8, User9, LocalCode, OutsideLine, UserID FROM CallSetupT WHERE ID = %li", m_id);

		m_strStartMsg = AdoFldString(rs, "StartMessage");
		m_strMsg = AdoFldString(rs, "Message");
		m_strRings = AdoFldString(rs, "Rings");
		m_strUser0 = AdoFldString(rs, "User0");
		m_strUser1 = AdoFldString(rs, "User1");
		m_strUser2 = AdoFldString(rs, "User2");
		m_strUser3 = AdoFldString(rs, "User3");
		m_strUser4 = AdoFldString(rs, "User4");
		m_strUser5 = AdoFldString(rs, "User5");
		m_strUser6 = AdoFldString(rs, "User6");
		m_strUser7 = AdoFldString(rs, "User7");
		m_strUser8 = AdoFldString(rs, "User8");
		m_strUser9 = AdoFldString(rs, "User9");
		SetDlgItemText(IDC_LOCAL_CODE, AdoFldString(rs, "LocalCode"));
		SetDlgItemText(IDC_OUTSIDE, AdoFldString(rs, "OutsideLine"));

		m_listUser->SetSelByColumn(0, rs->Fields->Item["UserID"]->Value);
	} NxCatchAll("Error in CCallerSetupDlg::Load()");

	UpdateData(false);
}

void CCallerSetupDlg::Save(int nID)
{
	if(m_listGroup->GetCurSel() == -1)
		return;
	
	UpdateData(true);
	CString field;

	switch(nID)
	{
	case IDC_START_MSG:
		field = "StartMessage";
		break;
	case IDC_MSG:
		field = "Message";
		break;
	case IDC_USER0:
		field = "User0";
		break;
	case IDC_USER1:
		field = "User1";
		break;
	case IDC_USER2:
		field = "User2";
		break;
	case IDC_USER3:
		field = "User3";
		break;
	case IDC_USER4:
		field = "User4";
		break;
	case IDC_USER5:
		field = "User5";
		break;
	case IDC_USER6:
		field = "User6";
		break;
	case IDC_USER7:
		field = "User7";
		break;
	case IDC_USER8:
		field = "User8";
		break;
	case IDC_USER9:
		field = "User9";
		break;
	case IDC_LOCAL_CODE:
		field = "LocalCode";
		break;
	case IDC_OUTSIDE:
		field = "OutsideLine";
		break;
	default:
		return;
	}

	try
	{
		CString str;
		GetDlgItemText(nID, str);

		ExecuteSql("UPDATE CallSetupT SET %s = '%s' WHERE CallSetupT.ID = %li", field, str, VarLong(m_listGroup->GetValue(m_listGroup->GetCurSel(), 0)));
	}NxCatchAll("Error saving field in CCallerSetupDlg::Save()");

}

BOOL CCallerSetupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	switch(HIWORD(wParam))
	{
	case EN_CHANGE:
		m_changed = true;
		break;
	
	case EN_KILLFOCUS:
		if(m_changed){
			Save (LOWORD(wParam));
			m_changed = false;
		}

		break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CCallerSetupDlg::OnRename() 
{
	CGetNewIDName dlg(this);
	CString NewName;
	_variant_t var;
	long nID;

	var = m_listGroup->GetValue(m_listGroup->CurSel,0);
	if (var.vt == VT_I4)
		nID = VarLong(var);
	else 
		return;//no item selected or error

	NewName = VarString(m_listGroup->GetValue(m_listGroup->CurSel,1));
	dlg.m_pNewName = &NewName;
	if (dlg.DoModal() == IDOK) 
	{	try 
		{
			ExecuteSql("UPDATE CallSetupT SET GroupName = '%s' WHERE ID = %li", _Q(NewName), nID);

		}NxCatchAll("Error in CCallerSetupDlg::OnRename()");
	}
	else
		return;
	
	m_listGroup->Requery();
	m_listGroup->SetSelByColumn(0, var);
}

void CCallerSetupDlg::OnAdd() 
{
	CGetNewIDName dlg(this);
	CString NewName;
	long nID;

	dlg.m_pNewName = &NewName;
	if (dlg.DoModal() == IDOK) {
		try{
			if(ExistsInTable("CallSetupT", "GroupName = '%s'", _Q(NewName))){
				MessageBox("A group exists with this name, please choose another name.", "Create Group", MB_OK);
				return;
			}
			nID = NewNumber ("CallSetupT", "ID");
			ExecuteSql("INSERT INTO CallSetupT (ID, GroupName) VALUES (%li, '%s');", nID, _Q(NewName));

		}NxCatchAll("Error in CCallerSetupDlg::OnAdd()");
	}
	else
		return;

	m_listGroup->Requery();
	m_listGroup->SetSelByColumn(1, _bstr_t(NewName));

	Load();
	
}

BEGIN_EVENTSINK_MAP(CCallerSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCallerSetupDlg)
	ON_EVENT(CCallerSetupDlg, IDC_GROUP_LIST, 2 /* SelChanged */, OnSelChangedGroupList, VTS_I4)
	ON_EVENT(CCallerSetupDlg, IDC_USER_LIST, 16 /* SelChosen */, OnSelChosenUserList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCallerSetupDlg::OnSelChangedGroupList(long nNewSel) 
{
	Load();	
}

void CCallerSetupDlg::OnSelChosenUserList(long nRow) 
{
	try{
		long nUserID, nSetupID;
		nUserID = VarLong(m_listUser->GetValue(nRow, 0));
		nSetupID = VarLong(m_listGroup->GetValue(m_listGroup->GetCurSel(), 0));

		ExecuteSql("UPDATE CallSetupT SET UserID = %li WHERE CallSetupT.ID = %li", nUserID, nSetupID);
	}NxCatchAll("Error in CCallerSetupDlg::Save()");
	
}

void CCallerSetupDlg::OnDelete() 
{
	try{
		ExecuteSql("DELETE FROM CallSetupT WHERE CallSetupT.ID = %li", VarLong(m_listGroup->GetValue(m_listGroup->GetCurSel(), 0)));
	} NxCatchAll("Error in CCallerSetupDlg::OnDelete()");

	m_listGroup->RemoveRow(m_listGroup->GetCurSel());
}
