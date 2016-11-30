// ScheduledPatientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ScheduledPatientsDlg.h"
#include "GlobalSchedUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum EnumScheduledPatientListColumns {
	splcPersonID,
	splcStartTime,
	splcPatientName,
	splcEntryCount,
	splcIdList,
};

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_MARK_IN	49100
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CScheduledPatientsDlg dialog


CScheduledPatientsDlg::CScheduledPatientsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CScheduledPatientsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScheduledPatientsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScheduledPatientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScheduledPatientsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScheduledPatientsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CScheduledPatientsDlg)
	ON_WM_SIZE()
	ON_COMMAND(ID_MARK_IN, OnMarkIn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScheduledPatientsDlg message handlers

BOOL CScheduledPatientsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_dlScheduledPatientList = GetDlgItem(IDC_SCHEDULED_PATIENT_LIST)->GetControlUnknown();

	} NxCatchAll("CScheduledPatientsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Throws _com_error
long CScheduledPatientsDlg::AddAppointment(long nApptID, COleDateTime dtStartTime, long nPatientID, const CString &strPatientName, BOOL bIsPending)
{
	long nAns = 0;

	// (c.haag 2006-03-16 10:12) - PLID 19736 - If the patient ID is invalid, don't add it
	if (nPatientID <= 0) {
		return -1;
	}
	
	// See if the row already exists
	long nRowIndex = m_dlScheduledPatientList->FindByColumn(splcPersonID, nPatientID, 0, VARIANT_FALSE);
	
	// Either update the existing row or add it new if it's not there
	if (nRowIndex >= 0) {
		// The row already exists
		IRowSettingsPtr pRow = m_dlScheduledPatientList->GetRow(nRowIndex);

		// Increment the entry count and write it to the list
		nAns = VarLong(pRow->GetValue(splcEntryCount));
		//TES 7/24/03: I'm not checking whether the id was already added, because the entry count clearly
		//assumes that duplicates aren't being added, so I'll go along with that.
		// (j.jones 2005-10-31 16:40) - bad idea, because it keeps incrementing the entry count for every time
		// that the appointment is edited
		
		CString strIdList = VarString(pRow->GetValue(splcIdList));
		CString strNewId;
		strNewId.Format("%li", nApptID);

		//check if the ID exists
		if(strNewId != strIdList && strIdList.Find(strNewId + ",") == -1 && strIdList.Find(", " + strNewId) == -1) {
			//does not already exist, so increment the count
			nAns++;
			pRow->PutValue(splcEntryCount, nAns);
			strIdList += ", " + strNewId;
			pRow->PutValue(splcIdList, _bstr_t(strIdList));
		}
		
		// Set the foreground color if requested
		if (bIsPending) {
			pRow->PutForeColor(RGB(0, 0, 255));
		}
		else {
			pRow->PutForeColor(RGB(0, 0, 0));
		}

		// If the start time is different than the current time, replace it and re-sort
		COleDateTime dtOldStart;
		_variant_t varOld = pRow->GetValue(splcStartTime);
		if(varOld.vt == VT_DATE)
			dtOldStart = varOld;
		else if(varOld.vt == VT_R8)
			dtOldStart = (DATE)varOld;

		if(dtStartTime != dtOldStart) {
			pRow->PutValue(splcStartTime, _variant_t(dtStartTime));
			m_dlScheduledPatientList->Sort();		
		}
		
	} else {
		// The row didn't exist so add it
		IRowSettingsPtr pRow = m_dlScheduledPatientList->GetRow(-1);
		pRow->PutValue(splcPersonID, nPatientID);
		pRow->PutValue(splcStartTime, _variant_t(dtStartTime));
		pRow->PutValue(splcPatientName, _bstr_t(strPatientName));

		// This is a new row so the entry count must be 1
		nAns = 1;
		pRow->PutValue(splcEntryCount, nAns);
		
		CString strId;
		strId.Format("%li", nApptID);
		pRow->PutValue(splcIdList, _bstr_t(strId));
		
		// Set the foreground color if requested
		if (bIsPending) {
			pRow->PutForeColor(RGB(0, 0, 255));
		}
		else {
			pRow->PutForeColor(RGB(0, 0, 0));
		}
		
		m_dlScheduledPatientList->AddRow(pRow);
	}

	// Return the entry count of this patient
	return nAns;
}

void CScheduledPatientsDlg::DeleteAppointment(long nResID)
{
	for (long i=0; i < m_dlScheduledPatientList->GetRowCount(); i++)
	{
		CString strIdList = VarString(m_dlScheduledPatientList->GetValue(i, splcIdList));
		long j = 0;
		CString strId;
		while(j < strIdList.GetLength()) {
			if(strIdList.Find(", ", j) != -1) {
				strId = strIdList.Mid(j, strIdList.Find(", ", j)-j);
			}
			else {
				strId = strIdList.Mid(j);
			}
			long nId = atol(strId);

			// (c.haag 2003-10-28 14:41) - If we get here, we found the appointment
			if (nId == nResID)
			{
				CString strTmp;
				strTmp.Format("%d", nResID);
				if (strIdList == strTmp)
				{
					// (c.haag 2003-10-28 14:42) - If we get here, its the only appointment
					// for this patient, so lets get rid of it
					m_dlScheduledPatientList->RemoveRow(i);
					return;
				}
				else
				{
					// (c.haag 2003-10-28 14:42) - Ok, so there are other appointments.
					// Remove this one from the column and walk away quietly.
					CString strNewList = strIdList.Left(j);

					if(strIdList.Find(", ", j) != -1) {
						// There are more entries after this
						strNewList += strIdList.Right( strIdList.GetLength() - j - (strId.GetLength() + 2) );
					}
					else
					{
						// There are not more entries after this
						strNewList = strNewList.Left( strNewList.GetLength() - 2 );
					}

					//re-update the list and count
					m_dlScheduledPatientList->PutValue(i, splcIdList, _bstr_t(strNewList));
					long nCount = VarLong(m_dlScheduledPatientList->GetValue(i, splcEntryCount)) - 1;
					m_dlScheduledPatientList->PutValue(i, splcEntryCount, (long)nCount);

					// (j.jones 2005-10-31 16:53) - PLID 17694 - we need to run through these appts. real fast
					// to get the earliest time, for sorting purposes
					if(strNewList.Find(",") == -1 && strNewList != "") {
						_RecordsetPtr rs = CreateRecordset("SELECT StartTime FROM AppointmentsT WHERE ID = %s", strNewList);
						if(!rs->eof) {
							COleDateTime dtStart = AdoFldDateTime(rs, "StartTime");
							m_dlScheduledPatientList->PutValue(i, splcStartTime, _variant_t(dtStart));
						}
						rs->Close();
					}
					else if(strNewList.Find(",") != -1) {

						COleDateTime dtEarliestStart;
						dtEarliestStart.SetStatus(COleDateTime::invalid);

						while(strNewList.Find(",") != -1) {
							CString strID = strNewList.Left(strNewList.Find(","));
							_RecordsetPtr rs = CreateRecordset("SELECT StartTime FROM AppointmentsT WHERE ID = %s", strID);
							if(!rs->eof) {
								COleDateTime dtStart = AdoFldDateTime(rs, "StartTime");
								if(dtEarliestStart.m_status == COleDateTime::invalid || dtStart < dtEarliestStart)
									dtEarliestStart = dtStart;
								m_dlScheduledPatientList->PutValue(i, splcStartTime, _variant_t(dtEarliestStart));
							}
							rs->Close();
							strNewList = strNewList.Right(strNewList.GetLength() - strID.GetLength() - 1);
							strNewList.TrimLeft();
						}

						if(strNewList != "") {
							_RecordsetPtr rs = CreateRecordset("SELECT StartTime FROM AppointmentsT WHERE ID = %s", strNewList);
							if(!rs->eof) {
								COleDateTime dtStart = AdoFldDateTime(rs, "StartTime");
								if(dtEarliestStart.m_status == COleDateTime::invalid || dtStart < dtEarliestStart)
									dtEarliestStart = dtStart;
								m_dlScheduledPatientList->PutValue(i, splcStartTime, _variant_t(dtEarliestStart));
							}
							rs->Close();
						}
					}
					
					m_dlScheduledPatientList->Sort();

					return;
				}
			}
			
			j += strId.GetLength() + 2;
		}
	}
}

// Throws _com_error
void CScheduledPatientsDlg::Clear()
{
	m_dlScheduledPatientList->Clear();
}


void CScheduledPatientsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	GetDlgItem(IDC_SCHEDULED_PATIENT_LIST)->MoveWindow(CRect(0, 0, cx, cy));
}

BEGIN_EVENTSINK_MAP(CScheduledPatientsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CScheduledPatientsDlg)
	ON_EVENT(CScheduledPatientsDlg, IDC_SCHEDULED_PATIENT_LIST, 6 /* RButtonDown */, OnRButtonDownScheduledPatientList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CScheduledPatientsDlg::OnRButtonDownScheduledPatientList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow != -1) {
		m_dlScheduledPatientList->CurSel = nRow;
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_STRING, ID_MARK_IN, "Mark Appointment as In");
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
	}
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
void CScheduledPatientsDlg::OnMarkIn()
{
	struct PendingAppointment{
		long nID;
		long nPatientID;
		long nStatus;
		long nShowState;
		long nLocationID;
		COleDateTime dtStart, dtEnd;
		CString strResourceIDs;
	};
	std::vector<PendingAppointment> vectorPendingAppointments;
	try {
		if(m_dlScheduledPatientList->CurSel != -1) {
			//How many pending appointments are entered for this patient?
			CString strIdList = VarString(m_dlScheduledPatientList->GetValue(m_dlScheduledPatientList->CurSel, splcIdList));
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, PatientID, Status, ShowState, LocationID, StartTime, EndTime, dbo.GetResourceIDString(ID) AS ResourceIDs "
				"FROM AppointmentsT WHERE ID IN ({INTSTRING}) AND ShowState = 0", strIdList);
			long nCount = 0;
			while (!rs->eof) {
				PendingAppointment pa;
				pa.nID = AdoFldLong(rs, "ID");
				pa.nPatientID = AdoFldLong(rs, "PatientID");
				pa.nLocationID = AdoFldLong(rs, "LocationID");
				pa.nStatus = (long)AdoFldByte(rs, "Status");
				pa.nShowState = AdoFldLong(rs, "ShowState");
				pa.dtStart = AdoFldDateTime(rs, "StartTime");
				pa.dtEnd = AdoFldDateTime(rs, "EndTime");
				pa.strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
				vectorPendingAppointments.push_back(pa);
				nCount++;
				rs->MoveNext();
			}
			if (nCount == 0) {
				MsgBox("There are no pending appointments for this patient in the current view.");
				return;
			}
			else if (nCount > 1) {
				if (IDYES != MsgBox(MB_YESNO, "There are %li pending appointments for this patient in the "
					"current view.  Would you like to mark ALL of them as In?", nCount)) {
					return;
				}
			}
			// (a.wilson 2014-08-13 07:28) - PLID 63199 - changed to use the ex table checker and also simplified the loop code.
			//Either nCount is 1, or they said yes to our message.  Let's rock and roll.
			foreach(PendingAppointment pa, vectorPendingAppointments)
			{
				AppointmentMarkIn(pa.nID);
				CClient::RefreshAppointmentTable(pa.nID, pa.nPatientID, pa.dtStart, pa.dtEnd, pa.nStatus, pa.nShowState, pa.nLocationID, pa.strResourceIDs);
			}
		}
	}NxCatchAll("Error in CScheduledPatientsDlg::OnMarkIn()");
}

LRESULT CScheduledPatientsDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2003-11-13 09:15) - If this window is deactivated,
	// we want to reset the internal timer that tracks how long
	// a view has been inactive (because it must be inactive this moment).
	// This way, if a client is distracted with managing their todos, they
	// won't have to enter a password to get back to the view they were
	// working in.
	if (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)
	{
		if (GetMainFrame() && GetMainFrame()->GetSafeHwnd())
		{
			CNxTabView* pView = GetMainFrame()->GetActiveView();

			// Reset the timer before the view gets its activation message
			if (pView && pView->GetSafeHwnd()) pView->ResetSecurityTimer();
		}
	}
	
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CScheduledPatientsDlg::OnOK()
{
	CDialog::OnOK();
}

void CScheduledPatientsDlg::OnCancel()
{
	CDialog::OnCancel();
}
