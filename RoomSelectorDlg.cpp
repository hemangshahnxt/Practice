// RoomSelectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RoomSelectorDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalSchedUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define COLUMN_ROOM_ID		0
#define COLUMN_ROOM_NAME	1
#define COLUMN_WAITING_ROOM	2	// (j.jones 2010-12-01 10:12) - PLID 38597
#define COLUMN_ROOM_IN_USE	3

/////////////////////////////////////////////////////////////////////////////
// CRoomSelectorDlg dialog


CRoomSelectorDlg::CRoomSelectorDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRoomSelectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoomSelectorDlg)
		m_nAppointmentID = -1;
		m_nPatientID = -1;
		m_nRoomID = -1;
		// (j.jones 2010-12-01 09:55) - PLID 38597 - default both to true,
		// as in most cases we want to have both lists
		m_bShowWaitingRooms = TRUE;
		m_bShowRegularRooms = TRUE;
	//}}AFX_DATA_INIT
}


void CRoomSelectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomSelectorDlg)
	DDX_Control(pDX, IDC_APPT_LABEL, m_nxstaticApptLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomSelectorDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRoomSelectorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomSelectorDlg message handlers

BEGIN_EVENTSINK_MAP(CRoomSelectorDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRoomSelectorDlg)
	ON_EVENT(CRoomSelectorDlg, IDC_ROOM_SELECTION_COMBO, 16 /* SelChosen */, OnSelChosenRoomSelectionCombo, VTS_DISPATCH)
	ON_EVENT(CRoomSelectorDlg, IDC_ROOM_SELECTION_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedRoomSelectionCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CRoomSelectorDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2010-11-24 14:59) - PLID 38597 - at least one of these should be marked TRUE,
	// enforce that at least one is TRUE
	if(!m_bShowWaitingRooms && !m_bShowRegularRooms) {
		//should not happen unless the calling code is flawed
		ASSERT(FALSE);
		m_bShowRegularRooms = TRUE;
	}

	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_pRoomCombo = BindNxDataList2Ctrl(this, IDC_ROOM_SELECTION_COMBO, GetRemoteData(), false);
	
	// (j.jones 2009-08-03 16:39) - PLID 24600 - we need to filter the room list by appt. location
	_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, LocationID, "
		"(CASE WHEN StartTime = EndTime AND DATEPART(hh,StartTime) = 0 THEN NULL ELSE StartTime END) AS StartTime "
		"FROM AppointmentsT WHERE ID = {INT}", m_nAppointmentID);
	if(!rs->eof) {
		m_nPatientID = AdoFldLong(rs, "PatientID");
		CString strPatientName = m_nPatientID == -25 ? "" : GetExistingPatientName(m_nPatientID);
		
		_variant_t varStartTime = rs->Fields->Item["StartTime"]->Value;
		CString strStartTime = "";
		if(varStartTime.vt == VT_DATE) {
			strStartTime.Format("(%s)", FormatDateTimeForInterface(VarDateTime(varStartTime), DTF_STRIP_SECONDS, dtoTime));
		}

		CString strLabel;
		// (j.jones 2010-11-24 14:59) - PLID 38597 - if we are only displaying waiting rooms, say so,
		// otherwise just label it as a "room"
		if(m_bShowWaitingRooms && !m_bShowRegularRooms) {
			strLabel.Format("Please select a waiting room for patient '%s' %s", strPatientName, strStartTime);
		}
		else {
			strLabel.Format("Please select a room for patient '%s' %s", strPatientName, strStartTime);
		}
		SetDlgItemText(IDC_APPT_LABEL, strLabel);

		long nLocationID = AdoFldLong(rs, "LocationID");
		CString strWhere;		
		strWhere.Format("Inactive = 0 AND LocationID = %li", nLocationID);
		
		// (j.jones 2010-11-24 14:59) - PLID 38597 - filter on whether we are showing waiting rooms, regular rooms,
		// or both, which would be no filter at all
		if(m_bShowWaitingRooms && !m_bShowRegularRooms) {
			strWhere += " AND WaitingRoom = 1";
		}
		else if(!m_bShowWaitingRooms && m_bShowRegularRooms) {
			strWhere += " AND WaitingRoom = 0";
		}

		m_pRoomCombo->PutWhereClause(_bstr_t(strWhere));
		m_pRoomCombo->Requery();
	}
	else {
		//how was this dialog called without a valid appointment?
		ASSERT(FALSE);
	}
	rs->Close();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoomSelectorDlg::OnOK() 
{	
	try {

		//assign to a room

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRoomCombo->GetCurSel();
		if(pRow == NULL) {

			AfxMessageBox("You must select a room before continuing.");
			return;
		}

		long nRoomID = VarLong(pRow->GetValue(COLUMN_ROOM_ID),-1);

		//ensure the room is not in use - check the current state, not
		//the state as it was when we loaded, just to make sure a
		//previously available room has not been filled

		// (j.jones 2009-06-25 14:57) - PLID 34728 - we support multiple people in the same room now
		// (j.jones 2010-12-01 09:59) - PLID 38597 - do not flag waiting rooms as In Use
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Inactive, RoomsT.WaitingRoom, "
			"Convert(bit, CASE WHEN WaitingRoom = 0 AND ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) THEN 1 ELSE 0 END) AS InUse "
			"FROM RoomsT WHERE ID = {INT}", nRoomID);

		BOOL bIsBeingAssignedToWaitingRoom = FALSE;

		if(rs->eof) {
			//must have been deleted

			//update the room list to reflect the change
			m_pRoomCombo->Requery();
			AfxMessageBox("This room is no longer available to use, please assign the patient to a different room.");			
			return;
		}
		else {

			bIsBeingAssignedToWaitingRoom = AdoFldBool(rs, "WaitingRoom", FALSE);

			if(AdoFldBool(rs, "Inactive", FALSE)) {
				//the room was inactivated

				//update the room list to reflect the change
				m_pRoomCombo->Requery();
				AfxMessageBox("This room is no longer available to use, please assign the patient to a different room.");			
				return;
			}
			else if(AdoFldBool(rs, "InUse", FALSE)) {

				//the room is in use, but do we already know that?
				BOOL bInUse = VarBool(pRow->GetValue(COLUMN_ROOM_IN_USE), FALSE);

				if(!bInUse) {
					//it wasn't in use, now it is, so tell them
					CString strName = VarString(pRow->GetValue(COLUMN_ROOM_NAME), "");
						
					if(IDNO == MsgBox(MB_YESNO, "The room '%s' has since had another patient assigned to it, are you sure you want to assign another patient to this room?", strName)) {

						//update the room list to reflect the change
						m_pRoomCombo->Requery();
						return;
					}
				}
			}
		}
		rs->Close();

		//ensure the appt. is not cancelled or deleted
		if(IsRecordsetEmpty("SELECT ID FROM AppointmentsT WHERE Status <> 4 AND ID = %li", m_nAppointmentID)) {
			
			//no longer available
			AfxMessageBox("This appointment has either been cancelled or deleted. The patient cannot be moved into the room.");
			CNxDialog::OnCancel();
			return;
		}

		//see if the appt. is in another room
		// (j.jones 2010-12-01 10:16) - PLID 38597 - ignore waiting rooms
		if(!bIsBeingAssignedToWaitingRoom) {
			rs = CreateParamRecordset("SELECT TOP 1 StatusID FROM RoomAppointmentsT "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"WHERE AppointmentID = {INT} AND RoomsT.WaitingRoom = 0 "
				"ORDER BY RoomAppointmentsT.ID DESC", m_nAppointmentID);
			if(!rs->eof) {

				long nStatus = AdoFldLong(rs, "StatusID", -1);
				if(nStatus != -1) {

					//the patient is in a room already
					AfxMessageBox("This patient has already been assigned to a room. You cannot assign the patient to another room.");
					CNxDialog::OnCancel();
					return;				
				}
				else {
					
					//the apt. has already been scheduled in a room in the past, so prompt

					if(IDNO == MessageBox("This patient's appointment has already been assigned to, and checked out of, a room.\n\n"
						"Are you sure you wish to move the patient back into another room?","Practice", MB_ICONQUESTION|MB_YESNO)) {
						CNxDialog::OnCancel();
						return;
					}
				}			
			}
			rs->Close();
		}

		//ok, now assign to the room!
		
		CString strSql = BeginSqlBatch();

		//we will use the hard-coded status of 1 for checked-in,
		//and allow the times to use their default of GetDate()

		// (a.walling 2013-06-07 09:46) - PLID 57078 - Parameterized
		CParamSqlBatch batch;
		batch.Declare(
			"SET NOCOUNT ON \r\n"
			"DECLARE @nRoomAppointmentID INT; \r\n"
			"\r\n"
		);
				
		// (a.walling 2013-06-07 09:50) - PLID 57079 - RoomAppointmentsT.ID now an identity; no more NewNumber
		batch.Add("INSERT INTO RoomAppointmentsT (RoomID, AppointmentID, LastUpdateUserID, StatusID) "
			"VALUES ({INT}, {INT}, {INT}, 1)", nRoomID, m_nAppointmentID, GetCurrentUserID());
		batch.Add("SET @nRoomAppointmentID = SCOPE_IDENTITY()");
		
		//and log this in the history		
		// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
		batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
			"VALUES (@nRoomAppointmentID, {INT}, 1)", GetCurrentUserID());

		// (a.walling 2013-06-07 09:50) - PLID 57079 - return new RoomAppointmentsT.ID
		batch.Add("SELECT @nRoomAppointmentID AS RoomAppointmentID");

		batch.Add("SET NOCOUNT OFF");

		rs = batch.CreateRecordset(GetRemoteData());
		long nRoomAppointmentID = -1;
		if (!rs->eof) {
			nRoomAppointmentID = AdoFldLong(rs, "RoomAppointmentID", -1);
		}
		rs->Close();

		//for auditing
		// (a.walling 2013-06-07 09:46) - PLID 57078 - Parameterized
		rs = CreateParamRecordset("SELECT PatientID, StartTime, Name, "
			"CASE WHEN Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
			"AND StartTime = EndTime AND DatePart(hh, StartTime) = 0 THEN 1 "
			"ELSE 0 END AS IsEvent "
			"FROM RoomAppointmentsT "
			"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
			"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"WHERE RoomAppointmentsT.ID = {INT}", nRoomAppointmentID);
		if(!rs->eof) {

			long nPatientID = AdoFldLong(rs, "PatientID",-1);
			CString strRoomName = AdoFldString(rs, "Name","");
			long nIsEvent = AdoFldLong(rs, "IsEvent",0);
			COleDateTime dtStartTime = AdoFldDateTime(rs, "StartTime");

			CString strOldValue, strNewValue;

			strOldValue.Format("Appt. Time: %s", nIsEvent == 1 ? "<Event>" : FormatDateTimeForInterface(dtStartTime, NULL, dtoDateTime));
			strNewValue.Format("Assigned to '%s'", strRoomName);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(nPatientID == -25 ? -1 : nPatientID, nPatientID == -25 ? "" : GetExistingPatientName(nPatientID), nAuditID, aeiRoomApptAssign, m_nAppointmentID, strOldValue, strNewValue, aepMedium, aetCreated);
		}
		rs->Close();

		//store to our public variable
		m_nRoomID = nRoomID;

		//and send a network message
		CClient::RefreshRoomAppointmentTable(nRoomAppointmentID);

		//if the preference says to mark the appointment 'In', do so
		if(GetRemotePropertyInt("MarkApptInOnRoomAssign", 0, 0, GetCurrentUserName(), true) == 1) {

			// (j.jones 2006-10-23 15:15) - PLID 23174 - also mark in if a no show appt,
			
			//but only if the appointment is currently 'Pending' (or 'No Show')
			if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE ID = %li AND (ShowState = 0 OR ShowState = 3)",m_nAppointmentID)) {

				//the appointment is 'Pending', so mark it 'In'
				AppointmentMarkIn(m_nAppointmentID);
			}
		}

	}NxCatchAll("CRoomSelectorDlg::OnOK");

	CNxDialog::OnOK();
}

void CRoomSelectorDlg::OnCancel() 
{
	//warn that the patient will not be assigned to a room
	
	if(IDNO == MessageBox("If you cancel, the patient will not be assigned to a room.\n"
		"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
		return;
	}

	CNxDialog::OnCancel();
}

void CRoomSelectorDlg::OnSelChosenRoomSelectionCombo(LPDISPATCH lpRow) 
{
	try {

		//disallow selection of in-use rooms

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow != NULL) {

			BOOL bInUse = VarBool(pRow->GetValue(COLUMN_ROOM_IN_USE), FALSE);

			// (j.jones 2009-06-25 14:57) - PLID 34728 - we support multiple people in the same room now

			//if in use, warn, and clear the selection if they cancel
			if(bInUse) {

				CString strName = VarString(pRow->GetValue(COLUMN_ROOM_NAME), "");
				
				if(IDNO == MsgBox(MB_YESNO, "The room '%s' is already in use, are you sure you want to assign another patient to this room?", strName)) {

					m_pRoomCombo->PutCurSel(NULL);
				}
			}
		}

	}NxCatchAll("CRoomSelectorDlg::OnSelChosenRoomSelectionCombo");
}

void CRoomSelectorDlg::OnRequeryFinishedRoomSelectionCombo(short nFlags) 
{
	try {

		//gray out in-use rooms

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRoomCombo->GetFirstRow();
		while(pRow) {
			
			BOOL bInUse = VarBool(pRow->GetValue(COLUMN_ROOM_IN_USE), FALSE);

			//if in use, gray out the row
			if(bInUse) {
				pRow->PutForeColor(RGB(192,192,192));
			}

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("CRoomSelectorDlg::OnRequeryFinishedRoomSelectionCombo");
}
