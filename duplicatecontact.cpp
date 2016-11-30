// DuplicateContact.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "DuplicateContact.h"
#include "globalutils.h"
#include "pracprops.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
extern CPracticeApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CDuplicateContact dialog


CDuplicateContact::CDuplicateContact(CWnd* pParent /*=NULL*/, LPCTSTR strIgnoreBtnText /*= NULL*/, LPCTSTR strRetryBtnText /*= NULL*/, LPCTSTR strAbortBtnText /*= NULL*/, bool bForceSelectionOnIgnore /*= false*/)
	: CNxDialog(CDuplicateContact::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDuplicate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bForceSelectionOnIgnore = bForceSelectionOnIgnore;
	m_nSelContactId = -1;
	m_strIgnoreBtnText = strIgnoreBtnText ? strIgnoreBtnText : "Save Contact";
	m_strRetryBtnText = strRetryBtnText ? strRetryBtnText : "Change Contact Name";
	m_strAbortBtnText = strAbortBtnText ? strAbortBtnText : GetStringOfResource(IDS_DISCARD_ENTRY);
}


void CDuplicateContact::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDuplicate)
	DDX_Control(pDX, IDC_NAME_LABEL, m_nxstaticNameLabel);
	DDX_Control(pDX, IDC_ADD, m_btnSaveContact);
	DDX_Control(pDX, IDC_BACK, m_btnChangeContactName);
	DDX_Control(pDX, IDC_CANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDuplicateContact, CNxDialog)
	//{{AFX_MSG_MAP(CDuplicate)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_BACK, OnBack)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDuplicateContact, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDuplicate)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
bool CDuplicateContact::FindDuplicates (CString first, CString last, CString middle)
{
	_RecordsetPtr rs;
	CString	value;

	try
	{	
		//JJ - 5/14/2001 - we don't search on middle name anymore, but I will keep it in the parameter list for
		//easy reintegration should we ever use it again
		rs = CreateRecordset("SELECT ID FROM ((SELECT ID, First, Middle, Last FROM ContactsT INNER JOIN PersonT ON ContactsT.PersonID = PersonT.ID) UNION (SELECT ID, First, Middle, Last FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0) UNION (SELECT ID, First, Middle, Last FROM SupplierT INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID) UNION (SELECT ID, First, Middle, Last FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID) UNION (SELECT ID, First, Middle, Last FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID)) AS SubQ WHERE First = '%s' AND Last = '%s'", first, last);
		
		if (rs->eof)
			return false;
		sql = "ID = ";
		while (!rs->eof)
		{	value.Format("%i", AdoFldLong(rs, "ID"));
			sql += value + " OR ID = ";
			rs->MoveNext();
		}
		rs->Close();
		sql.TrimRight (" OR ID = ");
		m_name = first + ' ' + last;
	}NxCatchAllCall("Error 100: CDuplicateContact::FindDuplicates", return false;);
	return true;
}

BOOL CDuplicateContact::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	m_pDupList = BindNxDataListCtrl(this, IDC_DUPLICATES, GetRemoteData(), false);
	m_pDupList->WhereClause = (LPCTSTR)sql;
	m_pDupList->Requery();
	SetDlgItemText(IDC_NAME_LABEL, m_name);
	CWnd *pWnd = GetDlgItem(IDC_NAME_LABEL);
	pWnd->SetFont(&theApp.m_boldFont);
	
	m_btnSaveContact.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnChangeContactName.AutoSet(NXB_MODIFY);

	// Set the button labels
	SetDlgItemText(IDC_ADD, m_strIgnoreBtnText);
	SetDlgItemText(IDC_BACK, m_strRetryBtnText);
	SetDlgItemText(IDC_CANCEL, m_strAbortBtnText);

	// Hide the retry button if it has no text
	if (m_strRetryBtnText.IsEmpty()) {
		GetDlgItem(IDC_BACK)->ShowWindow(SW_HIDE);
	}

	// Select the patient if specified
	if (m_nSelContactId != -1) {
		m_pDupList->SetSelByColumn(0, (long)m_nSelContactId);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

long CDuplicateContact::GetSelContactId()
{
	if(m_pDupList->CurSel == -1) return -1;
	else return VarLong(m_pDupList->GetValue(m_pDupList->CurSel, 0), -1);
}

void CDuplicateContact::OnAdd() 
{
	m_nSelContactId = GetSelContactId();
	if (m_bForceSelectionOnIgnore && m_nSelContactId == -1) {
		MsgBox(MB_OK|MB_ICONINFORMATION, "Please select a contact before proceeding");
		return;
	}

	EndDialog(IDIGNORE);
}

void CDuplicateContact::OnBack() 
{
	m_nSelContactId = GetSelContactId();
	
	EndDialog(IDRETRY);	
}

void CDuplicateContact::OnCancel() 
{
	m_nSelContactId = GetSelContactId();
	
	EndDialog(IDABORT);	
}
