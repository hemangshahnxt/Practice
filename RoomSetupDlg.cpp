// RoomSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RoomSetupDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2009-08-03 17:27) - PLID 24600 - converted to an enum
enum RoomListColumns {

	rlcID = 0,
	rlcName,
	rlcLocationID,
	// (j.jones 2010-11-24 10:56) - PLID 41620 - added WaitingRoom option
	rlcWaitingRoom,
	rlcInactive,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CRoomSetupDlg dialog


CRoomSetupDlg::CRoomSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRoomSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoomSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRoomSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomSetupDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_INACTIVE_ROOMS, m_checkShowInactive);
	DDX_Control(pDX, IDC_BTN_INACTIVATE_ROOM, m_btnInactivate);
	DDX_Control(pDX, IDC_BTN_DELETE_ROOM, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_CLOSE_ROOM_SETUP, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_ROOM, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRoomSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_ROOM, OnBtnAddRoom)
	ON_BN_CLICKED(IDC_BTN_DELETE_ROOM, OnBtnDeleteRoom)
	ON_BN_CLICKED(IDC_BTN_INACTIVATE_ROOM, OnBtnInactivateRoom)
	ON_BN_CLICKED(IDC_BTN_CLOSE_ROOM_SETUP, OnBtnCloseRoomSetup)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INACTIVE_ROOMS, OnCheckShowInactiveRooms)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomSetupDlg message handlers

BOOL CRoomSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pRoomList = BindNxDataList2Ctrl(this, IDC_ROOM_LIST, GetRemoteData(), true);

	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	// (z.manning, 04/30/2008) - PLID 29845 - More button styles
	m_btnInactivate.AutoSet(NXB_MODIFY);
	m_btnClose.AutoSet(NXB_CLOSE);

	GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoomSetupDlg::OnCancel() {

	CNxDialog::OnCancel();
}

void CRoomSetupDlg::OnBtnAddRoom() 
{
	if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
		return;

	try {

		CString strItem;
		BOOL bValid = FALSE;
		while(!bValid) {
			if (InputBoxLimited(this, "Enter a new room:", strItem, "",255,false,false,NULL) == IDOK) {
				strItem.TrimLeft(); strItem.TrimRight();

				if(ReturnsRecords("SELECT ID FROM RoomsT WHERE Name = '%s'", _Q(strItem))) {
					AfxMessageBox("There is already a room by that name. Please enter a new name.");					
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

			long nID = NewNumber("RoomsT","ID");
			// (j.jones 2009-08-03 17:47) - PLID 24600 - default the room to the current location
			ExecuteParamSql("INSERT INTO RoomsT (ID, Name, LocationID) VALUES ({INT}, {STRING}, {INT})", nID, strItem, GetCurrentLocationID());

			m_pRoomList->Requery();

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomCreate, nID, "", strItem, aepLow, aetCreated);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CRoomSetupDlg::OnBtnAddRoom");
}

void CRoomSetupDlg::OnBtnDeleteRoom() 
{
	try {

		// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
			return;

		IRowSettingsPtr pCurSel = m_pRoomList->GetCurSel();
		if(pCurSel == NULL) {
			AfxMessageBox("Please select a room before deleting.");
			return;
		}

		long nRoomID = VarLong(pCurSel->GetValue(rlcID));

		if(ReturnsRecords("SELECT ID FROM RoomsT WHERE ID = %li AND ID IN (SELECT RoomID FROM RoomAppointmentsT)",nRoomID)) {
			AfxMessageBox("This room has historical appointment information, and cannot be deleted. However, you may inactivate the room instead.");
			return;
		}

		if(IDNO == MessageBox("Deleted rooms will no longer show up in the Room Manager screen, and cannot be restored later.\n"
			"Are you sure you wish to delete this room?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		// (j.jones 2006-09-26 10:52) - we made a conscious decision to not use a deleted flag,
		// because the only way you can delete a room is if it has never been used
		// (z.manning 2014-09-09 10:19) - PLID 63260 - Handle UserRoomExclusionT
		ExecuteParamSql(R"(
BEGIN TRAN
DELETE FROM UserRoomExclusionT WHERE RoomID = {INT}
DELETE FROM RoomsT WHERE ID = {INT}
COMMIT TRAN
)", nRoomID, nRoomID);

		//audit this
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiRoomDelete, nRoomID, VarString(pCurSel->GetValue(rlcName),""), "Deleted", aepHigh, aetDeleted);

		//remove the row
		m_pRoomList->RemoveRow(pCurSel);
		m_pRoomList->PutCurSel(NULL);

		//send a network message
		CClient::RefreshTable(NetUtils::RoomSetup);

		GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);

	}NxCatchAll("Error in CRoomSetupDlg::OnBtnDeleteRoom");
}

void CRoomSetupDlg::OnBtnInactivateRoom() 
{
	try {

		// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
		if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
			return;

		IRowSettingsPtr pCurSel = m_pRoomList->GetCurSel();
		if(pCurSel == NULL) {
			AfxMessageBox("Please select a room before inactivating.");
			return;
		}

		long nRoomID = VarLong(pCurSel->GetValue(rlcID));

		BOOL bInactive = VarBool(pCurSel->GetValue(rlcInactive),FALSE);
		if(bInactive) {

			//activate the room

			if(IDNO == MessageBox("Reactivating this room will make it appear again in the Room Manager screen.\n"
				"Are you sure you wish to reactivate this room?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			ExecuteSql("UPDATE RoomsT SET Inactive = 0 WHERE ID = %li", nRoomID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomInactive, nRoomID, "Inactive", "Active", aepMedium, aetChanged);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			//if showing inactive rooms, uncheck the box, otherwise requery
			if(m_checkShowInactive.GetCheck()) {
				_variant_t varFalse;
				varFalse.boolVal = 0;
				varFalse.vt = VT_BOOL;
				pCurSel->PutValue(rlcInactive, varFalse);

				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM, "Inactivate Room");
			}
			else {
				//shouldn't really be possible to get here
				m_pRoomList->Requery();

				GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);
			}

		}
		else {

			//inactivate the room

			if(ReturnsRecords("SELECT ID FROM RoomsT WHERE ID = %li AND ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1))",nRoomID)) {
				AfxMessageBox("A patient is marked as currently being in this room. The room cannot be inactivated while it is in use.");
				return;
			}

			if(IDNO == MessageBox("Inactive rooms will no longer show up in the Room Manager screen.\n"
				"Are you sure you wish to inactivate this room?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			ExecuteSql("UPDATE RoomsT SET Inactive = 1 WHERE ID = %li", nRoomID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomInactive, nRoomID, "Active", "Inactive", aepMedium, aetChanged);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			//if showing inactive rooms, check the box, otherwise remove the row
			if(m_checkShowInactive.GetCheck()) {
				_variant_t varTrue;
				varTrue.boolVal = 1;
				varTrue.vt = VT_BOOL;
				pCurSel->PutValue(rlcInactive, varTrue);

				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM, "Activate Room");
			}
			else {
				m_pRoomList->RemoveRow(pCurSel);
				m_pRoomList->PutCurSel(NULL);

				GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);
			}
		}

	}NxCatchAll("Error in CRoomSetupDlg::OnBtnInactivateRoom");
}

void CRoomSetupDlg::OnBtnCloseRoomSetup() 
{
	CNxDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CRoomSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRoomSetupDlg)
	ON_EVENT(CRoomSetupDlg, IDC_ROOM_LIST, 9 /* EditingFinishing */, OnEditingFinishingRoomList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CRoomSetupDlg, IDC_ROOM_LIST, 10 /* EditingFinished */, OnEditingFinishedRoomList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CRoomSetupDlg, IDC_ROOM_LIST, 8 /* EditingStarting */, OnEditingStartingRoomList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CRoomSetupDlg, IDC_ROOM_LIST, 2 /* SelChanged */, OnSelChangedRoomList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRoomSetupDlg::OnEditingStartingRoomList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite)) {
		*pbContinue = FALSE;
		return;
	}
}

void CRoomSetupDlg::OnEditingFinishingRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}
		
		long nRoomID = VarLong(pRow->GetValue(rlcID));

		switch(nCol) {

		case rlcName:

			if(ReturnsRecordsParam("SELECT ID FROM RoomsT WHERE Name = {STRING} AND ID <> {INT}", strUserEntered, nRoomID)) {
				AfxMessageBox("There is already a room by that name.");					
				*pbContinue = FALSE;
				return;
			}
			break;

		// (j.jones 2009-08-03 17:45) - PLID 24600 - warn if a patient is in the room
		case rlcLocationID:

			if(VarLong(*pvarNewValue) != VarLong(varOldValue)) {
				if(ReturnsRecordsParam("SELECT ID FROM RoomsT WHERE ID = {INT} AND ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1))",nRoomID)) {
					AfxMessageBox("A patient is marked as currently being in this room. The room cannot have its location changed while it is in use.");
					*pbCommit = FALSE;
					return;
				}
			}
			break;

		// (j.jones 2010-11-24 11:00) - PLID 41620 - added waiting room flag
		case rlcWaitingRoom:

			//disallow if any patient has ever been checked into the room
			if(ReturnsRecordsParam("SELECT RoomID FROM RoomAppointmentsT WHERE RoomID = {INT}",nRoomID)) {
				AfxMessageBox("Patients have previously been marked as having been in this room. This room cannot have its Waiting Room status changed.");
				*pbCommit = FALSE;
				return;
			}			

			break;
		

		case rlcInactive:

			if(VarBool(*pvarNewValue, FALSE)) {
				//inactivating

				if(ReturnsRecordsParam("SELECT ID FROM RoomsT WHERE ID = {INT} AND ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1))",nRoomID)) {
					AfxMessageBox("A patient is marked as currently being in this room. The room cannot be inactivated while it is in use.");
					*pbContinue = FALSE;
					return;
				}
				
				if(IDNO == MessageBox("Inactive rooms will no longer show up in the Room Manager screen.\n"
					"Are you sure you wish to inactivate this room?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					*pbContinue = FALSE;
					return;
				}
			}
			else {
				//activating
				if(IDNO == MessageBox("Reactivating this room will make it appear again in the Room Manager screen.\n"
					"Are you sure you wish to reactivate this room?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					*pbContinue = FALSE;
					return;
				}
			}

			break;
		}

	}NxCatchAll("Error in CRoomSetupDlg::OnEditingFinishingRoomList");
}

void CRoomSetupDlg::OnEditingFinishedRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}
		
		long nRoomID = VarLong(pRow->GetValue(rlcID));

		switch(nCol) {

		case rlcName:
			{

			//for auditing
			CString strOldValue;
			_RecordsetPtr rs = CreateRecordset("SELECT Name FROM RoomsT WHERE ID = %li", nRoomID);
			if(!rs->eof) {
				strOldValue = AdoFldString(rs, "Name","");
			}
			rs->Close();

			ExecuteSql("UPDATE RoomsT SET Name = '%s' WHERE ID = %li", _Q(VarString(varNewValue,"")), nRoomID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomName, nRoomID, strOldValue, VarString(varNewValue,""), aepMedium, aetChanged);			

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			}
			break;

		// (j.jones 2009-08-03 17:31) - PLID 24600 - supported assigning locations to rooms
		case rlcLocationID:
			{

			CString strRoomName = VarString(pRow->GetValue(rlcName));

			//for auditing
			CString strOldValue, strNewValue;
			_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.Name "
				"FROM LocationsT "
				"INNER JOIN RoomsT ON LocationsT.ID = RoomsT.LocationID "
				"WHERE RoomsT.ID = {INT} "
				""
				"SELECT Name "
				"FROM LocationsT "
				"WHERE ID = {INT}", nRoomID, VarLong(varNewValue));
			if(!rs->eof) {
				strOldValue.Format("%s: %s", strRoomName, AdoFldString(rs, "Name",""));
			}

			rs = rs->NextRecordset(NULL);

			if(!rs->eof) {
				strNewValue = AdoFldString(rs, "Name","");
			}
			rs->Close();

			ExecuteParamSql("UPDATE RoomsT SET LocationID = {INT} WHERE ID = {INT}", VarLong(varNewValue), nRoomID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomLocationID, nRoomID, strOldValue, strNewValue, aepMedium, aetChanged);			

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			}
			break;

		// (j.jones 2010-11-24 11:00) - PLID 41620 - added waiting room flag
		case rlcWaitingRoom:
			{

			BOOL bWaitingRoom = VarBool(varNewValue, FALSE);
			
			ExecuteParamSql("UPDATE RoomsT SET WaitingRoom = {INT} WHERE ID = {INT}", bWaitingRoom ? 1 : 0, nRoomID);

			CString strRoomName = VarString(pRow->GetValue(rlcName));
			CString strOld;
			strOld.Format("%s (%s)", strRoomName, bWaitingRoom ? "Regular Room" : "Waiting Room");

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRoomWaitingRoom, nRoomID, strOld, !bWaitingRoom ? "Regular Room" : "Waiting Room", aepMedium, aetChanged);

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);

			}
			break;

		case rlcInactive:

			if(VarBool(varNewValue, FALSE)) {
				//inactivating
				ExecuteSql("UPDATE RoomsT SET Inactive = 1 WHERE ID = %li", nRoomID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomInactive, nRoomID, "Active", "Inactive", aepMedium, aetChanged);
			}
			else {
				//activating
				ExecuteSql("UPDATE RoomsT SET Inactive = 0 WHERE ID = %li", nRoomID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRoomInactive, nRoomID, "Inactive", "Active", aepMedium, aetChanged);
			}

			//send a network message
			CClient::RefreshTable(NetUtils::RoomSetup);
			break;
		}

	}NxCatchAll("Error in CRoomSetupDlg::OnEditingFinishedRoomList");
}

void CRoomSetupDlg::OnCheckShowInactiveRooms() 
{
	try {

		if(m_checkShowInactive.GetCheck()) {			
			m_pRoomList->WhereClause = "";
			m_pRoomList->Requery();
			m_pRoomList->GetColumn(rlcInactive)->PutStoredWidth(90);
		}
		else {			
			m_pRoomList->WhereClause = "Inactive = 0";
			m_pRoomList->Requery();
			m_pRoomList->GetColumn(rlcInactive)->PutStoredWidth(0);
		}

		GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);

	}NxCatchAll("Error in CRoomSetupDlg::OnCheckShowInactiveRooms");
}

void CRoomSetupDlg::OnSelChangedRoomList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		IRowSettingsPtr pCurSel = lpNewSel;
		if(pCurSel != NULL) {

			GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(TRUE);
			
			if(VarBool(pCurSel->GetValue(rlcInactive),FALSE)) {
				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM, "Activate Room");
			}
			else {
				SetDlgItemText(IDC_BTN_INACTIVATE_ROOM, "Inactivate Room");
			}
		}
		else {
			GetDlgItem(IDC_BTN_DELETE_ROOM)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_INACTIVATE_ROOM)->EnableWindow(FALSE);
			SetDlgItemText(IDC_BTN_INACTIVATE_ROOM, "Inactivate Room");
		}

	}NxCatchAll("Error in CRoomSetupDlg::OnSelChangedRoomList");
}
