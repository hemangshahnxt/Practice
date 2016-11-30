// YakGroupEdit.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "YakGroupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CYakGroupEdit dialog


CYakGroupEdit::CYakGroupEdit(CWnd* pParent)
	: CNxDialog(CYakGroupEdit::IDD, pParent)
{
	// initialize the groupID and name
	m_nGroupID = -25;
	m_strGroupName= "";
	//{{AFX_DATA_INIT(CYakGroupEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CYakGroupEdit::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYakGroupEdit)
	DDX_Control(pDX, IDC_RRIGHT, m_buRRight);
	DDX_Control(pDX, IDC_RIGHT, m_buRight);
	DDX_Control(pDX, IDC_LLEFT, m_buLLeft);
	DDX_Control(pDX, IDC_LEFT, m_buLeft);
	DDX_Control(pDX, IDC_AVAILABLE_USERS, m_nxstaticAvailableUsers);
	DDX_Control(pDX, IDC_SELECTED_USERS, m_nxstaticSelectedUsers);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CYakGroupEdit, CNxDialog)
	//{{AFX_MSG_MAP(CYakGroupEdit)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_RRIGHT, OnRright)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_LLEFT, OnLleft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYakGroupEdit message handlers
int CYakGroupEdit::DoModal(long nGroupID, CString strGroupName) 
{
	// set the member variables for the groupID and GroupName
	m_nGroupID = nGroupID;
	m_strGroupName= strGroupName;
		
	return CNxDialog::DoModal();
}


BOOL CYakGroupEdit::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_dlAvail = BindNxDataListCtrl(IDC_AVAILABLE);
	m_dlSelected = BindNxDataListCtrl(IDC_SELECTED);
	
	// now set the appropriate where clauses
	m_dlAvail->GetCurSel();
	CString strFrom = VarString(m_dlAvail->GetFromClause());
	
	
	CString strAvailWhere;
	strAvailWhere.Format("PersonT.Archived = 0 AND PersonID NOT IN (SELECT PersonID FROM YakGroupDetailsT WHERE GroupID = %li)", m_nGroupID);
	CString strSelectedWhere;
	strSelectedWhere.Format("PersonT.Archived = 0 AND PersonID IN (SELECT PersonID FROM YakGroupDetailsT WHERE GroupID = %li)", m_nGroupID);

	
	m_dlAvail->PutWhereClause(_bstr_t(strAvailWhere));
	m_dlSelected->PutWhereClause(_bstr_t(strSelectedWhere));
	m_dlAvail->Requery();
	m_dlSelected->Requery();

	m_buLeft.AutoSet(NXB_LEFT);
	m_buRight.AutoSet(NXB_RIGHT);
	m_buLLeft.AutoSet(NXB_LLEFT);
	m_buRRight.AutoSet(NXB_RRIGHT);

	this->SetWindowText("Edit Yak Group - " + m_strGroupName);

	// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CYakGroupEdit::OnOK() 
{
	try {
		if(IsRecordsetEmpty("SELECT * FROM YakGroupsT WHERE ID = %li", m_nGroupID)){
		// this is a new group so add it
			CString strSql;
			strSql.Format("INSERT INTO YakGroupsT (ID, GroupName) VALUES (%li, '%s')", m_nGroupID, _Q(m_strGroupName));
			ExecuteSql("%s", strSql);
		}
		
		//First, "uncheck" everything.
		ExecuteSql("DELETE FROM YakGroupDetailsT WHERE GroupID = %li", m_nGroupID);
		//Now, add in all the checked ones.
		for(int i = 0; i <= m_dlSelected->GetRowCount() - 1; i++) {
			CString strSql;
			strSql.Format("INSERT INTO YakGroupDetailsT (GroupID, PersonID) VALUES (%li, %li)", m_nGroupID, VarLong(m_dlSelected->GetValue(i, 0)));
			ExecuteSql(strSql);
		}
		
	}NxCatchAll("Error saving Yak group.");
	
	CDialog::OnOK();
}

void CYakGroupEdit::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CYakGroupEdit, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CYakGroupEdit)
	ON_EVENT(CYakGroupEdit, IDC_AVAILABLE, 3 /* DblClickCell */, OnDblClickCellAvailable, VTS_I4 VTS_I2)
	ON_EVENT(CYakGroupEdit, IDC_SELECTED, 3 /* DblClickCell */, OnDblClickCellSelected, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CYakGroupEdit::OnRight() 
{
	if(m_dlAvail->CurSel != -1){
		IRowSettingsPtr pRow = m_dlSelected->Row[-1];
		pRow->Value[0] = m_dlAvail->GetValue(m_dlAvail->CurSel,0);
		pRow->Value[1] = m_dlAvail->GetValue(m_dlAvail->CurSel,1);
		m_dlSelected->AddRow(pRow);
		m_dlAvail->RemoveRow(m_dlAvail->CurSel);
	}
}

void CYakGroupEdit::OnRright() 
{
	m_dlSelected->TakeAllRows(m_dlAvail);
}

void CYakGroupEdit::OnLeft() 
{
	if(m_dlSelected->CurSel != -1){
		IRowSettingsPtr pRow = m_dlAvail->Row[-1];
		pRow->Value[0] = m_dlSelected->GetValue(m_dlSelected->CurSel,0);
		pRow->Value[1] = m_dlSelected->GetValue(m_dlSelected->CurSel,1);
		m_dlAvail->AddRow(pRow);
		m_dlSelected->RemoveRow(m_dlSelected->CurSel);
	}
}

void CYakGroupEdit::OnLleft() 
{
	m_dlAvail->TakeAllRows(m_dlSelected);
	
}

void CYakGroupEdit::OnDblClickCellAvailable(long nRowIndex, short nColIndex) 
{
	if(m_dlAvail->CurSel != -1){
		IRowSettingsPtr pRow = m_dlSelected->Row[-1];
		pRow->Value[0] = m_dlAvail->GetValue(m_dlAvail->CurSel,0);
		pRow->Value[1] = m_dlAvail->GetValue(m_dlAvail->CurSel,1);
		m_dlSelected->AddRow(pRow);
		m_dlAvail->RemoveRow(m_dlAvail->CurSel);
	}	
}

void CYakGroupEdit::OnDblClickCellSelected(long nRowIndex, short nColIndex) 
{
	if(m_dlSelected->CurSel != -1){
		IRowSettingsPtr pRow = m_dlAvail->Row[-1];
		pRow->Value[0] = m_dlSelected->GetValue(m_dlSelected->CurSel,0);
		pRow->Value[1] = m_dlSelected->GetValue(m_dlSelected->CurSel,1);
		m_dlAvail->AddRow(pRow);
		m_dlSelected->RemoveRow(m_dlSelected->CurSel);
	}	
}
