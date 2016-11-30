// Purposes.cpp : implementation file
//

#include "stdafx.h"
#include "Purposes.h"
#include "globalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPurposes dialog


CPurposes::CPurposes(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPurposes::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPurposes)
	m_nPCount = 0;
	m_nPUserID = 0;
	m_nParseIndex = 0;
	m_bAppendNoteToDesc = FALSE;
	n_bAllowApptCancelling = FALSE;
	//}}AFX_DATA_INIT
}


void CPurposes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPurposes)
	DDX_Control(pDX, IDC_CHECK_ALLOWDELETE, m_btnAllowDelete);
	DDX_Control(pDX, IDC_CHECK_APPENDNOTETODESC, m_btnAppendNote);
	DDX_Control(pDX, IDC_BUTTON_REMOVE_ALL, m_btnRemoveAll);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_BUTTON_ADD_ALL, m_btnAddAll);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_DAYS, m_editDays);
	DDX_Control(pDX, IDC_LIST_SELECTED, m_listSelected);
	DDX_Control(pDX, IDC_LIST_AVAIL, m_listAvail);
	DDX_Check(pDX, IDC_CHECK_APPENDNOTETODESC, m_bAppendNoteToDesc);
	DDX_Check(pDX, IDC_CHECK_ALLOWDELETE, n_bAllowApptCancelling);
	DDX_Control(pDX, IDC_NOTE_GROUPBOX, m_btnNoteGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPurposes, CNxDialog)
	//{{AFX_MSG_MAP(CPurposes)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_ADD_ALL, OnButtonAddAll)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_ALL, OnButtonRemoveAll)
	ON_LBN_DBLCLK(IDC_LIST_AVAIL, OnDblclkListAvail)
	ON_LBN_DBLCLK(IDC_LIST_SELECTED, OnDblclkListSelected)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPurposes message handlers



BOOL CPurposes::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	m_btnRemoveAll.AutoSet(NXB_LLEFT);
	m_btnRemove.AutoSet(NXB_LEFT);
	m_btnAddAll.AutoSet(NXB_RRIGHT);
	m_btnAdd.AutoSet(NXB_RIGHT);

	// Add "About..." menu item to system menu.
	CoInitialize(NULL);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	
	// TODO: Add extra initialization here
	//m_btnRemove.AutoSet(NXB_LEFT);
	//m_btnRemoveAll.AutoSet(NXB_LLEFT);
	//m_btnAdd.AutoSet(NXB_RIGHT);
	//m_btnAddAll.AutoSet(NXB_RRIGHT);
		
	NxParse();
	PopulateAvail();	
	PopulateSelected();

	// do the days edit box
	_RecordsetPtr rs;
	CString strSQL;
	char strDays[64];
	long nTime;


	strSQL.Format("SELECT TimeSpanInDays, DBAppendNoteToDesc, AllowPracCancel FROM PalmSettingsT WHERE ID = %li", GetUserID());
	try {
		rs = CreateRecordset(strSQL);
		if (rs->eof) {
			nTime = 30;
			m_bAppendNoteToDesc = 0;
			n_bAllowApptCancelling = 0;
			MessageBox("No user found, please add user");
		}
		else {
			nTime = rs->Fields->GetItem("TimeSpanInDays")->Value.lVal;
			m_bAppendNoteToDesc = rs->Fields->GetItem("DBAppendNoteToDesc")->Value.bVal;
			n_bAllowApptCancelling = (rs->Fields->GetItem("AllowPracCancel")->Value.bVal) ? 1 : 0;
		}
	}
	NxCatchAll("SQL Error");

	sprintf(strDays, "%li", nTime); 
	m_editDays.SetWindowText(strDays);

	UpdateData(FALSE);

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CPurposes::PopulateAvail()
{
	// clear out the avail box
	m_listAvail.ResetContent();

	char strSQL[4096];
	long nItem = 0;
	CString strName;

	sprintf(strSQL, "SELECT Name, ID FROM AptTypeT ORDER BY Name");

	try {
		// Open a recordset based on my SQL string.
		m_p = CreateRecordset(strSQL);


		while(!(m_p->eof)) {
			// grab the name
			strName = m_p->Fields->GetItem("Name")->Value.bstrVal;

			// insert it to the list
			m_listAvail.InsertString(nItem, strName);

			// move next
			m_p->MoveNext();
			nItem++;
		}
	}
	NxCatchAll("General SQL Error");
}

void CPurposes::PopulateSelected()
{
	// delete the box
	//while (m_listSelected.GetCount() > 0)
	//	m_listSelected.DeleteString(0);

	m_listSelected.ResetContent();

	long i = 0;
	long nIndex;

	// loop through the array
	for (i = 0; i < m_nPCount; i++) {
		// move the item over
		nIndex = m_listAvail.FindString(-1, m_aryPurposes[i]);

		if (nIndex >= 0) {
			// delete it from the avail list
			m_listAvail.DeleteString(nIndex);

			// add it to the selected list
			m_listSelected.AddString(m_aryPurposes[i]);
		}
	}
}

void CPurposes::NxParse()
{
	m_nPCount = 0;
	m_aryPurposes.RemoveAll();

	try {
		_RecordsetPtr rs = CreateRecordset("SELECT AptTypeT.Name FROM PalmSettingsAptTypeT LEFT OUTER JOIN "
			"AptTypeT ON PalmSettingsAptTypeT.AptTypeID = AptTypeT.ID WHERE (PalmSettingsAptTypeT.PalmSettingsTID = %d)",
			GetUserID());
		COleVariant var;

		while (!rs->eof)
		{
			var = rs->Fields->GetItem("Name")->Value;

			// put it into the array
			if (var.vt == VT_NULL || var.vt == VT_EMPTY) {
				rs->MoveNext();
			}
			else {
				m_aryPurposes.Add(VarString(var));
				rs->MoveNext();
				m_nPCount++;
			}
		}
	} NxCatchAll("Error loading appointment types");
}



void CPurposes::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	long nCount = m_listAvail.GetSelCount();
	long nDel = 0;
	CString strItem;

	if (nCount == 0)
		return;

	CArray<int, int> arySelected;
	CArray<CString, int> aryStrings;

	// set the array size
	arySelected.SetSize(nCount);
	aryStrings.SetSize(nCount);

	m_listAvail.GetSelItems(nCount, arySelected.GetData());

	// now cycle through array and delete
	for (int i = 0; i < nCount; i++) {
		// move it
		m_listAvail.GetText(arySelected[i], strItem);

		// add it
		m_listSelected.AddString(strItem);
		
		// add it to the array
		aryStrings[i] = strItem;
	}

	// for loop to delete
	for (i = 0; i < nCount; i++) {
		// get the string
		nDel = m_listAvail.FindString(0, aryStrings[i]);

		// delete
		m_listAvail.DeleteString(nDel);

	}
}

void CPurposes::OnButtonAddAll() 
{
	// TODO: Add your control notification handler code here
	CString strItem;


	// bump all the items over to the selected list
	while (m_listAvail.GetCount() > 0) {
		// iterate through, add all items over to the list
		m_listAvail.GetText(0, strItem);

		// add it to the selected box
		m_listSelected.AddString(strItem);

		// delete it from the avail box
		m_listAvail.DeleteString(0);

	}

	// sort the lists?
	
}

void CPurposes::OnButtonRemove() 
{
	// TODO: Add your control notification handler code here
		// TODO: Add your control notification handler code here
	long nCount = m_listSelected.GetSelCount();
	long nDel = 0;
	CString strItem;

	if (nCount == 0)
		return;

	CArray<int, int> arySelected;
	CArray<CString, int> aryStrings;

	// set the array size
	arySelected.SetSize(nCount);
	aryStrings.SetSize(nCount);

	m_listSelected.GetSelItems(nCount, arySelected.GetData());

	// now cycle through array and delete
	for (int i = 0; i < nCount; i++) {
		// move it
		m_listSelected.GetText(arySelected[i], strItem);

		// add it
		m_listAvail.AddString(strItem);
		
		// add it to the array
		aryStrings[i] = strItem;
	}

	// for loop to delete
	for (i = 0; i < nCount; i++) {
		// get the string
		nDel = m_listSelected.FindString(0, aryStrings[i]);

		// delete
		m_listSelected.DeleteString(nDel);

	}
}

void CPurposes::OnButtonRemoveAll() 
{
	// TODO: Add your control notification handler code here

	CString strItem;


	// to the avail list
	while (m_listSelected.GetCount() > 0) {
		// iterate through, add all items over to the list
		m_listSelected.GetText(0, strItem);

		// add it to the avail box
		m_listAvail.AddString(strItem);

		// delete it from the selected box
		m_listSelected.DeleteString(0);

	}
}



void CPurposes::PUpdate()
{
	
	PopulateAvail();
	NxParse();
	PopulateSelected();
}

void CPurposes::OK()
{
	try {
		long nCount = m_listSelected.GetCount();
		CString strType;

		// (c.haag 2004-02-24 15:49) - Flag all appointments to synchronize
		CString strSQL;
		strSQL.Format("SELECT AptTypeID FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d", GetUserID());
		if (GetRecordCount(strSQL) == 0)
		{
			ExecuteSql("INSERT INTO PalmSyncT (PalmSettingsTID, AppointmentID, Audit) SELECT %d, ID, 0 FROM AppointmentsT WHERE StartTime >= DateAdd(d, -(SELECT TimeSpanInDays FROM PalmSettingsT WHERE ID = %d), GetDate()) AND ID IN "
				"(SELECT ID FROM AppointmentResourceT WHERE ResourceID IN (SELECT ResourceID FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d)) AND "
				"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PalmSyncT WHERE PalmSettingsTID = %d) ",
				GetUserID(), GetUserID(), GetUserID(), GetUserID());
		}
		else
		{
			ExecuteSql("INSERT INTO PalmSyncT (PalmSettingsTID, AppointmentID, Audit) SELECT %d, ID, 0 FROM AppointmentsT WHERE StartTime >= DateAdd(d, -(SELECT TimeSpanInDays FROM PalmSettingsT WHERE ID = %d), GetDate()) AND ID IN "
				"(SELECT ID FROM AppointmentResourceT WHERE ResourceID IN (SELECT ResourceID FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d)) AND "
				"AptTypeID IN (SELECT AptTypeID FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d) AND "
				"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PalmSyncT WHERE PalmSettingsTID = %d) ",
				GetUserID(), GetUserID(), GetUserID(), GetUserID(), GetUserID());
		}

		// Clear the list and fill it with entries
		ExecuteSql("DELETE FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d", GetUserID());

		for (int i = 0; i < nCount; i++)
		{
			// grab the purpose name outta the list
			m_listSelected.GetText(i, strType);

			ExecuteSql("INSERT INTO PalmSettingsAptTypeT SELECT %d, ID FROM AptTypeT WHERE Name = '%s' ", GetUserID(), _Q(strType));
		}

		// now do the days
		char strDays[32];
		m_editDays.GetWindowText(strDays, 32);
		long nDays = atoi(strDays);
		UpdateData();
		ExecuteSql("UPDATE PalmSettingsT SET TimeSpanInDays = %li, DBAppendNoteToDesc = %d, AllowPracCancel = %d WHERE (ID = %li)", nDays, m_bAppendNoteToDesc, n_bAllowApptCancelling, GetUserID());
	}
	NxCatchAll("Error saving appointment types");
}


void CPurposes::OnDblclkListAvail() 
{

	// move the thing over, baby
	long nIndex = 0;
	CString nItem;

	nIndex = m_listAvail.GetCurSel();

	m_listAvail.GetText(nIndex, nItem);
	m_listAvail.DeleteString(nIndex);

	m_listSelected.AddString(nItem);

}

void CPurposes::OnDblclkListSelected() 
{
	// move the thing over, baby
	long nIndex = 0;
	CString nItem;

	nIndex = m_listSelected.GetCurSel();

	m_listSelected.GetText(nIndex, nItem);
	m_listSelected.DeleteString(nIndex);

	m_listAvail.AddString(nItem);
}

long CPurposes::GetUserID()
{
	return m_nPUserID;
}

void CPurposes::SetUserID(long ID)
{
	m_nPUserID = ID;
}

void CPurposes::OnOK()
{
	//We should pass this on up to our parent.
	CWnd *pParent = GetParent();
	if(pParent && pParent->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)GetParent())->OK();
	}
	else if(pParent->GetParent() && pParent->GetParent()->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)pParent->GetParent())->OK();
	}
}

void CPurposes::OnCancel()
{
	//We should pass this on up to our parent.
	CWnd *pParent = GetParent();
	if(pParent && pParent->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)GetParent())->Cancel();
	}
	else if(pParent->GetParent() && pParent->GetParent()->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)pParent->GetParent())->Cancel();
	}
}
