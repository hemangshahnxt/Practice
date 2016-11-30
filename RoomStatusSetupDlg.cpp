// RoomStatusSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RoomStatusSetupDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID	0
#define COLUMN_NAME	1
#define COLUMN_WITH_PERSON	2
#define COLUMN_INACTIVE	3
#define COLUMN_ORDER	4	// (j.jones 2008-05-29 09:55) - PLID 27797 - the sort is by this calculated column

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CRoomStatusSetupDlg dialog


CRoomStatusSetupDlg::CRoomStatusSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRoomStatusSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoomStatusSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRoomStatusSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomStatusSetupDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_INACTIVE_ROOM_STATUSES, m_checkShowInactive);
	DDX_Control(pDX, IDC_BTN_INACTIVATE_ROOM_STATUS, m_btnInactivate);
	DDX_Control(pDX, IDC_BTN_DELETE_ROOM_STATUS, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_CLOSE_ROOM_STATUS_SETUP, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_ROOM_STATUS, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomStatusSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRoomStatusSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_ROOM_STATUS, OnBtnAddRoomStatus)
	ON_BN_CLICKED(IDC_BTN_DELETE_ROOM_STATUS, OnBtnDeleteRoomStatus)
	ON_BN_CLICKED(IDC_BTN_INACTIVATE_ROOM_STATUS, OnBtnInactivateRoomStatus)
	ON_BN_CLICKED(IDC_BTN_CLOSE_ROOM_STATUS_SETUP, OnBtnCloseRoomStatusSetup)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INACTIVE_ROOM_STATUSES, OnCheckShowInactiveRoomStatuses)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomStatusSetupDlg message handlers

BOOL CRoomStatusSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pStatusList = BindNxDataList2Ctrl(this, IDC_ROOM_STATUS_LIST, GetRemoteData(), true);

	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	// (z.manning, 04/30/2008) - PLID 29845 - More button styles
	m_btnInactivate.AutoSet(NXB_MODIFY);
	m_btnClose.AutoSet(NXB_CLOSE);

	GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoomStatusSetupDlg::OnCancel() {

	CNxDialog::OnCancel();
}

void CRoomStatusSetupDlg::OnBtnAddRoomStatus() 
{
	try {

		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
			return;

		CString strItem;
		BOOL bValid = FALSE;
		while(!bValid) {
			if (InputBoxLimited(this, "Enter a new status:", strItem, "",255,false,false,NULL) == IDOK) {
				strItem.TrimLeft(); strItem.TrimRight();

				if(ReturnsRecords("SELECT ID FROM RoomStatusT WHERE Name = '%s'", _Q(strItem))) {
					AfxMessageBox("There is already a status by that name. Please enter a new name.");					
					continue;
				}
				else {
					bValid = TRUE;
				}
			}
			else {
				return;
			}
		}

		if(bValid) {

			long nWithPerson = 0;

			if(IDYES == MessageBox("A status for a room can be configured to imply the patient is 'with a person', which\n"
								   "can be used to track and identify when a patient is not being seen by an employee.\n\n"
								   "Would you like to mark this new status as signifying the patient is 'with a person'?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				nWithPerson = 1;
			}

			long nID = NewNumber("RoomStatusT","ID");
			ExecuteSql("INSERT INTO RoomStatusT (ID, Name, WithPerson) VALUES (%li, '%s', %li)", nID, _Q(strItem), nWithPerson);

			m_pStatusList->Requery();

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomStatusCreate, nID, "", strItem, aepLow, aetCreated);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnBtnAddRoomStatus");
}

void CRoomStatusSetupDlg::OnBtnDeleteRoomStatus() 
{
	try {

		// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
			return;

		IRowSettingsPtr pCurSel = m_pStatusList->GetCurSel();
		if(pCurSel == NULL) {
			AfxMessageBox("Please select a status before deleting.");
			return;
		}

		long nStatusID = VarLong(pCurSel->GetValue(COLUMN_ID));
		// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
		if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
			AfxMessageBox("You may not delete the 'Checked In', 'Ready To Check Out', or 'Checked Out' statuses.");
			return;
		}

		if(ReturnsRecords("SELECT ID FROM RoomStatusT WHERE ID = %li AND (ID IN (SELECT StatusID FROM RoomAppointmentsT) "
				"OR ID IN (SELECT StatusID FROM RoomAppointmentHistoryT))", nStatusID)) {
			AfxMessageBox("This status is used in historical appointment information, and cannot be deleted. However, you may inactivate the status instead.");
			return;
		}

		if(IDNO == MessageBox("Deleted statuses will no longer be available for selection in the Room Manager screen, and cannot be restored later.\n"
			"Are you sure you wish to delete this status?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		// (j.jones 2006-09-26 10:52) - we made a conscious decision to not use a deleted flag,
		// because the only way you can delete a status is if it has never been used
		ExecuteSql("DELETE FROM RoomStatusT WHERE ID = %li", nStatusID);

		//audit this
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiRoomStatusDelete, nStatusID, VarString(pCurSel->GetValue(COLUMN_NAME),""), "Deleted", aepHigh, aetDeleted);

		//remove the row
		m_pStatusList->RemoveRow(pCurSel);
		m_pStatusList->PutCurSel(NULL);

		//send a network message
		CClient::RefreshTable(NetUtils::RoomSetup);

		GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnBtnDeleteRoomStatus");
}

void CRoomStatusSetupDlg::OnBtnInactivateRoomStatus() 
{
	try {

		// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
			return;

		IRowSettingsPtr pCurSel = m_pStatusList->GetCurSel();
		if(pCurSel == NULL) {
			AfxMessageBox("Please select a status before inactivating.");
			return;
		}

		BOOL bInactive = VarBool(pCurSel->GetValue(COLUMN_INACTIVE),FALSE);
		if(bInactive) {

			//activate the status

			if(IDNO == MessageBox("Reactivating this status will make it available for selection in the Room Manager screen.\n"
				"Are you sure you wish to reactivate this status?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			long nStatusID = VarLong(pCurSel->GetValue(COLUMN_ID));
			ExecuteSql("UPDATE RoomStatusT SET Inactive = 0 WHERE ID = %li", nStatusID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomStatusInactive, nStatusID, "Inactive", "Active", aepMedium, aetChanged);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			//if showing inactive statuses, uncheck the box, otherwise requery
			if(m_checkShowInactive.GetCheck()) {
				_variant_t varFalse;
				varFalse.boolVal = 0;
				varFalse.vt = VT_BOOL;
				pCurSel->PutValue(COLUMN_INACTIVE, varFalse);

				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM_STATUS, "Inactivate Status");
			}
			else {
				//shouldn't really be possible to get here
				m_pStatusList->Requery();

				GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
			}

		}
		else {

			//inactivate the status

			long nStatusID = VarLong(pCurSel->GetValue(COLUMN_ID));
			// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
			if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
				AfxMessageBox("You may not inactivate the 'Checked In', 'Ready To Check Out', or 'Checked Out' statuses.");
				return;
			}

			if(ReturnsRecords("SELECT ID FROM RoomStatusT WHERE ID = %li AND ID IN (SELECT StatusID FROM RoomAppointmentsT)", nStatusID)) {
				AfxMessageBox("A patient currently in a room is marked with this status. The status cannot be inactivated while it is currently in use.");
				return;
			}

			if(IDNO == MessageBox("Inactive statuses will no longer be available for selection in the Room Manager screen.\n"
				"Are you sure you wish to inactivate this status?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			ExecuteSql("UPDATE RoomStatusT SET Inactive = 1 WHERE ID = %li", nStatusID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomStatusInactive, nStatusID, "Active", "Inactive", aepMedium, aetChanged);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			//if showing inactive statuses, check the box, otherwise remove the row
			if(m_checkShowInactive.GetCheck()) {
				_variant_t varTrue;
				varTrue.boolVal = 1;
				varTrue.vt = VT_BOOL;
				pCurSel->PutValue(COLUMN_INACTIVE, varTrue);

				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM_STATUS, "Activate Status");
			}
			else {
				m_pStatusList->RemoveRow(pCurSel);
				m_pStatusList->PutCurSel(NULL);

				GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
			}
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnBtnInactivateRoomStatus");
}

void CRoomStatusSetupDlg::OnBtnCloseRoomStatusSetup() 
{
	CNxDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CRoomStatusSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRoomStatusSetupDlg)
	ON_EVENT(CRoomStatusSetupDlg, IDC_ROOM_STATUS_LIST, 9 /* EditingFinishing */, OnEditingFinishingRoomStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CRoomStatusSetupDlg, IDC_ROOM_STATUS_LIST, 10 /* EditingFinished */, OnEditingFinishedRoomStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CRoomStatusSetupDlg, IDC_ROOM_STATUS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRoomStatusList, VTS_I2)
	ON_EVENT(CRoomStatusSetupDlg, IDC_ROOM_STATUS_LIST, 8 /* EditingStarting */, OnEditingStartingRoomStatusList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CRoomStatusSetupDlg, IDC_ROOM_STATUS_LIST, 2 /* SelChanged */, OnSelChangedRoomStatusList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRoomStatusSetupDlg::OnEditingStartingRoomStatusList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow) {
			long nStatusID = VarLong(pRow->GetValue(COLUMN_ID));
			// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
			if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
				*pbContinue = FALSE;
				return;
			}
		}

		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite)) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnEditingStartingRoomStatusList");
}

void CRoomStatusSetupDlg::OnEditingFinishingRoomStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}
		
		long nStatusID = VarLong(pRow->GetValue(COLUMN_ID));
		// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
		if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
			*pbContinue = FALSE;
		}

		switch(nCol) {

		case COLUMN_NAME:

			if(ReturnsRecords("SELECT ID FROM RoomStatusT WHERE Name = '%s' AND ID <> %li", _Q(strUserEntered), nStatusID)) {
				AfxMessageBox("There is already a status by that name.");					
				*pbContinue = FALSE;
				return;
			}
			break;

		case COLUMN_WITH_PERSON:

			// (j.jones 2006-09-26 17:07) - I currently don't think we need to check anything right now,
			// but we may decide later to not allow this change if the status is in use or has been used in the past
			break;

		case COLUMN_INACTIVE:

			if(VarBool(*pvarNewValue, FALSE)) {
				//inactivating

				if(ReturnsRecords("SELECT ID FROM RoomStatusT WHERE ID = %li AND ID IN (SELECT StatusID FROM RoomAppointmentsT)", nStatusID)) {
					AfxMessageBox("A patient currently in a room is marked with this status. The status cannot be inactivated while it is currently in use.");
					*pbContinue = FALSE;
					return;
				}

				if(IDNO == MessageBox("Inactive statuses will no longer be available for selection in the Room Manager screen.\n"
					"Are you sure you wish to inactivate this status?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					*pbContinue = FALSE;
					return;
				}
			}
			else {
				//activating
				if(IDNO == MessageBox("Reactivating this status will make it available for selection in the Room Manager screen.\n"
					"Are you sure you wish to reactivate this status?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					*pbContinue = FALSE;
					return;
				}
			}

			break;
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnEditingFinishingRoomStatusList");
}

void CRoomStatusSetupDlg::OnEditingFinishedRoomStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}
		
		long nStatusID = VarLong(pRow->GetValue(COLUMN_ID));

		switch(nCol) {

		case COLUMN_NAME:
			{

			//for auditing
			CString strOldValue;
			_RecordsetPtr rs = CreateRecordset("SELECT Name FROM RoomStatusT WHERE ID = %li", nStatusID);
			if(!rs->eof) {
				strOldValue = AdoFldString(rs, "Name","");
			}
			rs->Close();

			ExecuteSql("UPDATE RoomStatusT SET Name = '%s' WHERE ID = %li", _Q(VarString(varNewValue,"")), nStatusID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomStatusName, nStatusID, strOldValue, VarString(varNewValue,""), aepMedium, aetChanged);			

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);
			}
			break;

		case COLUMN_WITH_PERSON:

			if(VarBool(varNewValue, FALSE)) {
				//inactivating
				ExecuteSql("UPDATE RoomStatusT SET WithPerson = 1 WHERE ID = %li", nStatusID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomStatusWithPerson, nStatusID, "Not With A Person", "With A Person", aepMedium, aetChanged);
			}
			else {
				//activating
				ExecuteSql("UPDATE RoomStatusT SET WithPerson = 0 WHERE ID = %li", nStatusID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomStatusWithPerson, nStatusID, "With A Person", "Not With A Person", aepMedium, aetChanged);
			}

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);
			break;

		case COLUMN_INACTIVE:

			if(VarBool(varNewValue, FALSE)) {
				//inactivating
				ExecuteSql("UPDATE RoomStatusT SET Inactive = 1 WHERE ID = %li", nStatusID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomStatusInactive, nStatusID, "Active", "Inactive", aepMedium, aetChanged);
			}
			else {
				//activating
				ExecuteSql("UPDATE RoomStatusT SET Inactive = 0 WHERE ID = %li", nStatusID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomStatusInactive, nStatusID, "Inactive", "Active", aepMedium, aetChanged);
			}

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);
			break;
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnEditingFinishedRoomStatusList");
}

void CRoomStatusSetupDlg::OnCheckShowInactiveRoomStatuses() 
{
	try {

		if(m_checkShowInactive.GetCheck()) {			
			m_pStatusList->WhereClause = "";
			m_pStatusList->Requery();
			m_pStatusList->GetColumn(COLUMN_INACTIVE)->PutStoredWidth(90);
		}
		else {			
			m_pStatusList->WhereClause = "Inactive = 0";
			m_pStatusList->Requery();
			m_pStatusList->GetColumn(COLUMN_INACTIVE)->PutStoredWidth(0);
		}

		GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnCheckShowInactiveRoomStatuses");
}

void CRoomStatusSetupDlg::OnRequeryFinishedRoomStatusList(short nFlags) 
{
	try {
		//gray out the hard-coded rows

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStatusList->GetFirstRow();
		while(pRow) {
			long nStatusID = VarLong(pRow->GetValue(COLUMN_ID));
			// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
			if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
				//color the hard-coded rows gray
				pRow->PutForeColor(RGB(192,192,192));
				_variant_t varNull;
				varNull.vt = VT_NULL;
				pRow->PutValue(COLUMN_WITH_PERSON, varNull);
				pRow->PutValue(COLUMN_INACTIVE, varNull);
			}
			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnRequeryFinishedRoomStatusList");
}

void CRoomStatusSetupDlg::OnSelChangedRoomStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		IRowSettingsPtr pCurSel = lpNewSel;
		if(pCurSel != NULL) {

			long nStatusID = VarLong(pCurSel->GetValue(COLUMN_ID));
			// (j.jones 2008-05-29 09:44) - PLID 27797 - added the hard-coded 'Ready To Check Out' status
			if(nStatusID == -1 || nStatusID == 0 || nStatusID == 1) {
				GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
				return;
			}

			GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(TRUE);
			
			if(VarBool(pCurSel->GetValue(COLUMN_INACTIVE),FALSE)) {
				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM_STATUS, "Activate Status");
			}
			else {
				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM_STATUS, "Inactivate Status");
			}
		}
		else {			
			GetDlgItem(IDC_BTN_DELETE_ROOM_STATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM_STATUS)->EnableWindow(FALSE);
			SetDlgItemText(IDC_BTN_INACTIVATE_ROOM_STATUS, "Inactivate Status");
		}

	}NxCatchAll("Error in CRoomStatusSetupDlg::OnSelChangedRoomStatusList");	
}
