// PalmPilotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PalmPilotDlg.h"
#include "SchedulerRc.h"
#include "GlobalUtils.h"
#include "globaldatautils.h"
#include "direct.h"
#include "PracProps.h"
#include "NxStandard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

/////////////////////////////////////////////////////////////////////////////
// CPalmPilotDlg dialog


CPalmPilotDlg::CPalmPilotDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPalmPilotDlg::IDD, pParent)
	, palm_dlg(this)
{
	//{{AFX_DATA_INIT(CPalmPilotDlg)
	m_dwPalmUserID = 0;
	m_bModified = FALSE;
	//}}AFX_DATA_INIT
}


void CPalmPilotDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalmPilotDlg)
	DDX_Control(pDX, IDC_ALL_TO_SELECTED, m_btnSelectAll);
	DDX_Control(pDX, IDC_ALL_TO_AVAIL, m_btnRemoveAll);
	DDX_Control(pDX, IDC_TO_AVAIL, m_btnRemoveOne);
	DDX_Control(pDX, IDC_TO_SELECTED, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECTED_LIST, m_SelectedList);
	DDX_Control(pDX, IDC_AVAILABLE_LIST, m_AvailList);
	DDX_Control(pDX, IDC_PALM_DIALOG_TAB, m_Tab);
	DDX_Control(pDX, IDC_STATIC_AVAILABLE_FIELDS, m_nxstaticAvailableFields);
	DDX_Control(pDX, IDC_STATIC_DISPLAYED_FIELDS, m_nxstaticDisplayedFields);
	DDX_Control(pDX, IDC_STATIC_RESOURCES, m_nxstaticResources);
	DDX_Control(pDX, IDC_PALMPREF_RENAME, m_btnRename);
	DDX_Control(pDX, IDC_PALMPREF_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_PALMPREF_REMOVE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPalmPilotDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPalmPilotDlg)
	ON_BN_CLICKED(IDC_ALL_TO_AVAIL, OnAllToAvail)
	ON_BN_CLICKED(IDC_ALL_TO_SELECTED, OnAllToSelected)
	ON_BN_CLICKED(IDC_TO_AVAIL, OnToAvail)
	ON_BN_CLICKED(IDC_TO_SELECTED, OnToSelected)
	ON_BN_CLICKED(ID_ADVPALM, OnClickAdvPalm)
	ON_BN_CLICKED(IDC_PALMPREF_ADD, OnClickBtnAdd)
	ON_BN_CLICKED(IDC_PALMPREF_REMOVE, OnClickBtnRemove)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PALM_DIALOG_TAB, OnSelchangeTab)
	ON_BN_CLICKED(IDC_PALMPREF_RENAME, OnPalmprefRename)
	ON_NOTIFY(NM_DBLCLK, IDC_AVAILABLE_LIST, OnDblclkAvailableList)
	ON_NOTIFY(NM_DBLCLK, IDC_SELECTED_LIST, OnDblclkSelectedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CPalmPilotDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketGraphDlg)
	ON_EVENT(CPalmPilotDlg, IDC_COMBO_PALMUSERLIST, 2 /* SelChanged */, OnPalmUserChanged, VTS_I4)
	ON_EVENT(CPalmPilotDlg, IDC_PALM_RESOURCE_LIST, 10 /* EditingFinished */, OnEditingFinishedPalmResourceList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPalmPilotDlg message handlers

void CPalmPilotDlg::OnAllToAvail() 
{
	m_bModified = TRUE;
	m_AvailList.DeleteAllItems();
	m_SelectedList.DeleteAllItems();
	PopulateAvail();
}

void CPalmPilotDlg::OnAllToSelected() 
{
	m_bModified = TRUE;
	while(m_AvailList.GetItemCount() > 0)
	{
		m_SelectedList.InsertItem(m_SelectedList.GetItemCount(), m_AvailList.GetItemText(0,0));
		m_AvailList.DeleteItem(0);
	}
}

void CPalmPilotDlg::OnToAvail() 
{
	m_bModified = TRUE;
	m_TextParam.Empty();
	for(int i = 0; i < m_SelectedList.GetItemCount(); i++)
	{
		while(m_SelectedList.GetItemState(i,LVIS_SELECTED)){
			m_AvailList.InsertItem(m_AvailList.GetItemCount(),m_SelectedList.GetItemText(i,0));
			m_SelectedList.DeleteItem(i);
		}
	}
	for(int j = 0; j < m_SelectedList.GetItemCount(); j++)
	{
		m_TextParam += "[" +  m_SelectedList.GetItemText(j,0) +"] ";		
	}
}

void CPalmPilotDlg::OnToSelected() 
{
	m_bModified = TRUE;
	for(int i = 0; i < m_AvailList.GetItemCount(); i++)
	{
		while(m_AvailList.GetItemState(i,LVIS_SELECTED)){
			m_SelectedList.InsertItem(m_SelectedList.GetItemCount(),m_AvailList.GetItemText(i,0));
			m_AvailList.DeleteItem(i);
		}
	}
}

void CPalmPilotDlg::OnCancel() 
{
	//AfxMessageBox("If you have not set this dialog up and clicked OK at LEAST \n"
	//	"once, DO NOT attempt to perform a HotSync operation;\n" 
	//	"without setting up the software first");

	CNxDialog::OnCancel();
}

void CPalmPilotDlg::OnOK() 
{	
	try {
		CString strSQL;
		CWaitCursor wc;
		SavePalmScript();
		SaveResourceCheckboxes();
		UpdateModifiedAppts();
		CNxDialog::OnOK();
	} NxCatchAll("Error saving palm settings");
}

BOOL CPalmPilotDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnSelectAll.AutoSet(NXB_RRIGHT);
	m_btnRemoveAll.AutoSet(NXB_LLEFT);
	m_btnRemoveOne.AutoSet(NXB_LEFT);
	m_btnSelectOne.AutoSet(NXB_RIGHT);
	// (z.manning, 04/30/2008) - PLID 29845 - More button styles
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnRename.AutoSet(NXB_MODIFY);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_Tab.InsertItem(0, "Fields");
	m_Tab.InsertItem(1, "Resources");

	m_mapFields["Resource Name(s)"] = "Resource";
	m_mapFields["Patient ID"] = "Person.ID";
	m_mapFields["First Name"] = "Person.First";
	m_mapFields["Middle Name"] = "Person.Middle";
	m_mapFields["Last Name"] = "Person.Last";
	m_mapFields["Patient Notes"] = "Person.Memo";
	m_mapFields["Home Number"] = "Person.HomePhone";
	m_mapFields["Work Number"] = "Person.WorkPhone";
	m_mapFields["Address"] = "Person.Address";
	m_mapFields["City"] = "Person.City";
	m_mapFields["State"] = "Person.State";
	m_mapFields["Zip"] = "Person.Zip";
	m_mapFields["E-Mail"] = "Person.Email";
	m_mapFields["Birthday"] = "Person.BirthDate";
	m_mapFields["Appointment Type"] = "Appointment.PurposeSet";
	m_mapFields["Appointment Purpose"] = "Appointment.Purpose";
	m_mapFields["Move Up"] = "Appointment.MoveUp";
	m_mapFields["Confirmed"] = "Appointment.Confirmed";
	m_mapFields["Location"] = "Location";

	m_dlPalmUsers = BindNxDataListCtrl(this, IDC_COMBO_PALMUSERLIST, GetRemoteData(), true);
	m_dlResources = BindNxDataListCtrl(this, IDC_PALM_RESOURCE_LIST, GetRemoteData(), true);
	m_dlPalmUsers->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	if (!m_dlPalmUsers->GetRowCount())
	{
		if (IDNO == MsgBox(MB_YESNO, "There are currently no Palm Pilot users in your database. Would you like to add one now?"))
		{
			PostMessage(WM_COMMAND, IDCANCEL);
			return TRUE;			
		}
		OnClickBtnAdd();
		m_dlPalmUsers->Requery();
		m_dlPalmUsers->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		if (!m_dlPalmUsers->GetRowCount())
		{
			PostMessage(WM_COMMAND, IDCANCEL);
			return TRUE;
		}
	}
	m_dlPalmUsers->PutCurSel(0);
	m_dwPalmUserID = VarLong(m_dlPalmUsers->GetValue(0, 0));

	// Fill the resource checkboxes
	m_dlResources->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	FillResourceCheckboxes();
	FillScript(); // Get the palm script
	PopulateAvail(); // Fill the list of available script fields
	PopulateSelected(); // Fill the list of selected script fields
	m_bModified = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPalmPilotDlg::FillScript()
{
	m_TextParam = VarString(m_dlPalmUsers->GetValue(m_dlPalmUsers->GetCurSel(),2));
}

void CPalmPilotDlg::PopulateAvail()
{
	m_AvailList.DeleteAllItems();

	m_AvailList.InsertItem(0,"Resource Name(s)");
	m_AvailList.InsertItem(1,"Location");
	m_AvailList.InsertItem(2,"Patient ID");
	m_AvailList.InsertItem(3,"First Name");
	m_AvailList.InsertItem(4,"Middle Name");
	m_AvailList.InsertItem(5,"Last Name");
	m_AvailList.InsertItem(6,"Patient Notes");
	m_AvailList.InsertItem(7,"Home Number");
	m_AvailList.InsertItem(8,"Work Number");
	m_AvailList.InsertItem(9,"Address");
	m_AvailList.InsertItem(10,"City");
	m_AvailList.InsertItem(11,"State");
	m_AvailList.InsertItem(12,"Zip");
	m_AvailList.InsertItem(13,"E-Mail");
	m_AvailList.InsertItem(14,"Birthday");
	m_AvailList.InsertItem(15,"Appointment Type");
	m_AvailList.InsertItem(16,"Appointment Purpose");
	m_AvailList.InsertItem(17,"Move Up");
	m_AvailList.InsertItem(18,"Confirmed");
}

void CPalmPilotDlg::PopulateSelected()
{
	int pos;
	CString newItem, curStr, tmpStr;

	m_SelectedList.DeleteAllItems();
	while(m_TextParam.GetLength() > 1){
		pos = 0;
		curStr.Empty();
		while(curStr.Right(1) != "]"){
			pos++;
			curStr += m_TextParam.Mid(pos,1);
		}
		newItem = curStr.Mid(0,curStr.GetLength() - 1);
		for(int i = 0; i < m_AvailList.GetItemCount(); i++){
			if(m_mapFields[m_AvailList.GetItemText(i,0)] == newItem){
				m_SelectedList.InsertItem(m_SelectedList.GetItemCount(), m_AvailList.GetItemText(i,0));
				m_AvailList.DeleteItem(i);
			}
		}
		tmpStr = m_TextParam.Mid(curStr.GetLength() + 2,m_TextParam.GetLength());
		m_TextParam = tmpStr;
	}
}
 

void CPalmPilotDlg::OnClickAdvPalm() 
{
	palm_dlg.SetUserID(m_dwPalmUserID);
	if (IDOK == palm_dlg.DoModal())
		m_bModified = TRUE;
}

void CPalmPilotDlg::OnClickBtnAdd()
{
	try {
		long nProfiles = GetRecordCount("SELECT ID FROM PalmSettingsT GROUP BY ID");
		if (nProfiles >= g_pLicense->GetPalmCountAllowed())
		{
			MsgBox("You must purchase additional Palm Pilot licenses to add profiles to the NexTech Practice Palm integration. Please contact your sales representative for more information.");
			return;
		}
	
		CString strNewName;
		if (IDCANCEL == InputBoxLimited(this, "Please enter the name of the Palm user", strNewName, "",255,false,false,NULL))
			return;

		//DRT 2/7/03 - confirm the name is less than 255 (data limit), and prompt them to keep trying until it is
		while(strNewName.GetLength() > 255) {
			AfxMessageBox("The name you have entered is too long.  Please shorten it and try again.");
			if (IDCANCEL == InputBoxLimited(this, "Please enter the name of the Palm user", strNewName, "",255,false,false,NULL))
				return;
		}

		ExecuteSql("INSERT INTO PalmSettingsT (ID, PalmScript, PalmUserName, DoRestore, DupContactBehavior) "
			"VALUES (%d, '[Person.First] [Person.Last] [Person.HomePhone] [Appointment.PurposeSet] [Appointment.Purpose] ', '%s', 0, 1)",
			NewNumber("PalmSettingsT", "ID"), _Q(strNewName));

		//DRT 2/7/03 - moved this code inside the try/catch - if it failed, it would execute this and cause a crash
		m_dlPalmUsers->Requery();
		m_dlPalmUsers->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_dlPalmUsers->SetSelByColumn(1, _variant_t(strNewName));

		// Update the current palm user ID
		m_dwPalmUserID = VarLong(m_dlPalmUsers->GetValue(m_dlPalmUsers->GetCurSel(), 0));

		FillResourceCheckboxes();
		FillScript();
		PopulateAvail();
		PopulateSelected();

		// (c.haag 2004-02-24 17:27) - Force the conduits to synchronize all
		// pertinent appointments and contacts now, so if the user decides to
		// cancel out right away, he can still do a normal hotsync and get
		// everything.
		m_bModified = TRUE;
		UpdateModifiedAppts();
		UpdateModifiedContacts();
		m_bModified = FALSE;

	} NxCatchAll("Failed to create Palm user");

}

void CPalmPilotDlg::OnClickBtnRemove()
{
	if (m_dwPalmUserID == -1)
		return;

	if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to delete this Palm user and all information associated with it?"))
		return;

	BEGIN_TRANS("DeletePalmUser"){		
		ExecuteSql("DELETE FROM PalmRecordT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmSyncT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmDeletedT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmContactsInfoT WHERE PalmUserID = %d", m_dwPalmUserID);
		ExecuteSql("DELETE FROM PalmSettingsT WHERE ID = %d", m_dwPalmUserID);
	}END_TRANS_CATCH_ALL("DeletePalmUser");

	m_dlPalmUsers->Requery();
	m_dlPalmUsers->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	if (m_dlPalmUsers->GetRowCount() == 0)
	{
		MsgBox("Since there are no palm users, this window will now close.");
		PostMessage(WM_COMMAND, IDCANCEL);
		return;
	}
	m_dlPalmUsers->PutCurSel(0);

	// Update the current palm user ID
	m_dwPalmUserID = VarLong(m_dlPalmUsers->GetValue(0, 0));
	FillResourceCheckboxes();
	FillScript();
	PopulateAvail();
	PopulateSelected();
	m_bModified = FALSE;

}

void CPalmPilotDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch (m_Tab.GetCurSel())
	{
	case 0:
		m_SelectedList.ShowWindow(SW_SHOW);
		m_AvailList.ShowWindow(SW_SHOW);		
		GetDlgItem(IDC_TO_SELECTED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ALL_TO_SELECTED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TO_AVAIL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ALL_TO_AVAIL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PALM_RESOURCE_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_RESOURCES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_AVAILABLE_FIELDS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_DISPLAYED_FIELDS)->ShowWindow(SW_SHOW);
		break;

	case 1:
		m_SelectedList.ShowWindow(SW_HIDE);
		m_AvailList.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TO_SELECTED)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ALL_TO_SELECTED)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TO_AVAIL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ALL_TO_AVAIL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PALM_RESOURCE_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_RESOURCES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_AVAILABLE_FIELDS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_DISPLAYED_FIELDS)->ShowWindow(SW_HIDE);
		break;
	}
}

void CPalmPilotDlg::FillResourceCheckboxes()
{
	COleVariant vBool;
	DWORD dwResID;
	_RecordsetPtr rs;

	// Get the palm user ID
	if (m_dwPalmUserID == (DWORD)-1)
		return;
	vBool.vt = VT_BOOL;

	for (long i=0; i < m_dlResources->GetRowCount(); i++)
	{
		// Get the resource ID
		dwResID = VarLong(m_dlResources->GetValue(i,0));

		// See if the selected palm user uses this
		rs = CreateRecordset("SELECT PalmSettingsTID FROM PalmSettingsResourceT WHERE "
			"PalmSettingsTID = %d AND ResourceID = %d", m_dwPalmUserID, dwResID);
		if (!rs->eof)
			vBool.boolVal = 1;
		else
			vBool.boolVal = 0;

		m_dlResources->PutValue(i, 1, vBool);
	}
}

void CPalmPilotDlg::SavePalmScript()
{
	CString strSQL;

	if (m_SelectedList.GetItemCount() > 0) {
		// Build the script to write to the databases
		m_TextParam.Empty();
		for (int j=0; j<m_SelectedList.GetItemCount(); j++) {
			m_TextParam += "[" +  m_mapFields[m_SelectedList.GetItemText(j,0)] +"] ";	
		}

		if (DoesFieldExist("PalmSettingsT", "DoRestore")) {
			// here store the text string in the palmsettingsT table
			strSQL.Format("UPDATE PalmSettingsT SET PalmScript = '%s', DoRestore = 1 WHERE ID = %d", m_TextParam, m_dwPalmUserID); 
		}
		else {
			// no dorestore
			strSQL.Format("UPDATE PalmSettingsT SET PalmScript = '%s' WHERE ID = %d", m_TextParam, m_dwPalmUserID); 
			MsgBox("You do not have the latest version of the Practice Data.  Please run the Data"
				" upgrade utility");
		}

//#ifdef _DEBUG
//		MsgBox(strSQL);
//#endif

		try {
			// NO.  Taken out 12/6 cjs
			// CPalmRestorePromptDlg dlg;
			// (j.jones 2010-04-20 10:00) - PLID 30852 - changed to ExecuteSqlStd
			ExecuteSqlStd(strSQL);

			// dlg.DoModal();
		} NxCatchAll("General Error");	

	}else{
		MsgBox("You must choose at least one field to display in the Palm Pilot.");
		return;
	}
}

void CPalmPilotDlg::SaveResourceCheckboxes()
{
	BOOL bUsesResource;
	DWORD dwResID;
	_RecordsetPtr rs;

	if (m_dwPalmUserID == (DWORD)-1)
		return;

	// Do for each resource in the configuration
	for (long i=0; i < m_dlResources->GetRowCount(); i++)
	{
		// Get the resource ID
		dwResID = VarLong(m_dlResources->GetValue(i,0));
		bUsesResource = VarBool(m_dlResources->GetValue(i,1));

		// If the palm user uses this, make sure an entry for it exists in PalmSettingsResourceT.
		if (bUsesResource)
		{			
			try {
				_RecordsetPtr prs = CreateRecordset("SELECT PalmSettingsTID FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d AND ResourceID = %d",
					m_dwPalmUserID, dwResID);
				if (prs->eof)
				{
					ExecuteSql("INSERT INTO PalmSettingsResourceT (PalmSettingsTID, ResourceID) VALUES (%d, %d)",
						m_dwPalmUserID, dwResID);
				}
			}
			NxCatchAll("Error saving palm user configuration");
		}
		else
			ExecuteSql("DELETE FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d AND ResourceID = %d",
				m_dwPalmUserID, dwResID);
	}
}

void CPalmPilotDlg::OnPalmUserChanged(long nNewSel) 
{
	if (nNewSel == -1)
		return;

	try {
		// Save all changes
		SavePalmScript();

		// Update the dropdown with the new script
		m_dlPalmUsers->PutValue(m_dlPalmUsers->FindByColumn(0, COleVariant((long)m_dwPalmUserID), 0, FALSE), 2, COleVariant(m_TextParam));

		SaveResourceCheckboxes();
		UpdateModifiedAppts();

		// Update the current palm user ID
		m_dwPalmUserID = VarLong(m_dlPalmUsers->GetValue(nNewSel, 0));

		FillResourceCheckboxes();
		FillScript();
		PopulateAvail();
		PopulateSelected();
		m_bModified = FALSE;
	} NxCatchAll("Error saving palm settings");
}

void CPalmPilotDlg::OnPalmprefRename() 
{
	// Get the palm user ID
	if (m_dwPalmUserID == (DWORD)-1)
		return;
	try {
		CString strNewName;
		if (IDCANCEL == InputBoxLimited(this, "Please enter the name of the Palm user", strNewName, "",255,false,false,NULL))
			return;
		while(strNewName.GetLength() > 255) {
			AfxMessageBox("The name you have entered is too long.  Please shorten it and try again.");
			if (IDCANCEL == InputBoxLimited(this, "Please enter the name of the Palm user", strNewName, "",255,false,false,NULL))
				return;
		}
		// Change the name in the data
		ExecuteSql("UPDATE PalmSettingsT SET PalmUserName = '%s' WHERE ID = %d",
			_Q(strNewName), m_dwPalmUserID);
		// Change the name in the datalist
		m_dlPalmUsers->PutValue(m_dlPalmUsers->FindByColumn(0, COleVariant((long)m_dwPalmUserID), 0, FALSE), 1, COleVariant(strNewName));
		m_dlPalmUsers->PutCurSel( m_dlPalmUsers->CurSel );

	} NxCatchAll("Failed to rename Palm user");
}

int CPalmPilotDlg::DoModal()
{
	if (!g_pLicense->GetPalmCountAllowed())
	{
		MsgBox("You must purchase a Palm Pilot license to use NexTech Practice Palm integration. Please contact your sales representative for more information.");
		return IDCANCEL;
	}
	return CNxDialog::DoModal();
}

void CPalmPilotDlg::UpdateModifiedAppts()
{
	if (!m_bModified)
		return;

	// (c.haag 2004-02-24 15:49) - Flag all appointments to synchronize
	CString strSQL;
	strSQL.Format("SELECT AptTypeID FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d", m_dwPalmUserID);
	if (GetRecordCount(strSQL) == 0)
	{
		ExecuteSql("INSERT INTO PalmSyncT (PalmSettingsTID, AppointmentID, Audit) SELECT %d, ID, 0 FROM AppointmentsT WHERE StartTime >= DateAdd(d, -(SELECT TimeSpanInDays FROM PalmSettingsT WHERE ID = %d), GetDate()) AND ID IN "
			"(SELECT ID FROM AppointmentResourceT WHERE ResourceID IN (SELECT ResourceID FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d)) AND "
			"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PalmSyncT WHERE PalmSettingsTID = %d) ",
			m_dwPalmUserID, m_dwPalmUserID, m_dwPalmUserID, m_dwPalmUserID);
	}
	else
	{
		ExecuteSql("INSERT INTO PalmSyncT (PalmSettingsTID, AppointmentID, Audit) SELECT %d, ID, 0 FROM AppointmentsT WHERE StartTime >= DateAdd(d, -(SELECT TimeSpanInDays FROM PalmSettingsT WHERE ID = %d), GetDate()) AND ID IN "
			"(SELECT ID FROM AppointmentResourceT WHERE ResourceID IN (SELECT ResourceID FROM PalmSettingsResourceT WHERE PalmSettingsTID = %d)) AND "
			"AptTypeID IN (SELECT AptTypeID FROM PalmSettingsAptTypeT WHERE PalmSettingsTID = %d) AND "
			"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PalmSyncT WHERE PalmSettingsTID = %d) ",
			m_dwPalmUserID, m_dwPalmUserID, m_dwPalmUserID, m_dwPalmUserID, m_dwPalmUserID);
	}
}

void CPalmPilotDlg::UpdateModifiedContacts()
{
	// (c.haag 2004-02-25 12:34) - Flag all contacts to synchronize
	ExecuteSql("UPDATE PalmSettingsT SET CategoriesChanged = 1 WHERE ID = %d", m_dwPalmUserID);
}

void CPalmPilotDlg::OnEditingFinishedPalmResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	m_bModified = TRUE;	
}

void CPalmPilotDlg::OnDblclkAvailableList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnToSelected();
	*pResult = 0;
}

void CPalmPilotDlg::OnDblclkSelectedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnToAvail();
	*pResult = 0;
}
