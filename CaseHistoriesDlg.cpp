// CaseHistoriesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaseHistoriesDlg.h"
#include "CaseHistoryDlg.h"
#include "DateTimeUtils.h"
#include "SchedulerView.h"
#include "GlobalDrawingUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "LinkCaseHistoryToBillDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_PRINT_ONE		41097
#define ID_GOTO_PATIENT		41098
#define ID_GOTO_APPOINTMENT	41099
#define ID_LINK_TO_BILL		41100	// (j.jones 2009-08-07 11:36) - PLID 24077

// (j.jones 2009-08-07 11:28) - PLID 24077 - added column enum
enum CaseHistoryListColumns {

	chlcID = 0,
	chlcPatientID,
	chlcPatientName,
	chlcDescription,
	chlcProcedures,
	chlcProviderName,
	chlcLocationName,	// (j.jones 2009-08-26 10:36) - PLID 34943
	chlcSurgeryDate,
	chlcCompletedDate,
	chlcBilled,
};

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCaseHistoriesDlg dialog


CCaseHistoriesDlg::CCaseHistoriesDlg(CWnd* pParent)
	: CNxDialog(CCaseHistoriesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCaseHistoriesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "Surgery_Center/create_a_case_history.htm";
}


void CCaseHistoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaseHistoriesDlg)
	DDX_Control(pDX, IDC_RADIO_CASE_HISTORY_ALL_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_CASE_HISTORY_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_RADIO_COMPLETED, m_radioCompleted);
	DDX_Control(pDX, IDC_RADIO_INCOMPLETE, m_radioIncomplete);
	DDX_Control(pDX, IDC_RADIO_ALL, m_radioAllCompleted);
	DDX_Control(pDX, IDC_DT_CASE_HISTORY_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_DT_CASE_HISTORY_TO, m_dtTo);
	DDX_Control(pDX, IDC_RADIO_UNBILLED_CH, m_radioUnbilled);
	DDX_Control(pDX, IDC_RADIO_BILLED_CH, m_radioBilled);
	DDX_Control(pDX, IDC_RADIO_ALL_BILLED_CH, m_radioAllBilled);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CCaseHistoriesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCaseHistoriesDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_CASE_HISTORY_FROM, OnChangeDtCaseHistoryFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_CASE_HISTORY_TO, OnChangeDtCaseHistoryTo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DT_CASE_HISTORY_FROM, OnCloseUpDtCaseHistoryFrom)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DT_CASE_HISTORY_TO, OnCloseUpDtCaseHistoryTo)
	ON_BN_CLICKED(ID_GOTO_PATIENT, OnGoToPatient)
	ON_BN_CLICKED(ID_GOTO_APPOINTMENT, OnGoToAppointment)
	ON_BN_CLICKED(IDC_RADIO_COMPLETED, OnRadioCompleted)
	ON_BN_CLICKED(IDC_RADIO_INCOMPLETE, OnRadioIncomplete)
	ON_BN_CLICKED(IDC_RADIO_ALL, OnRadioAllCompleted)
	ON_BN_CLICKED(IDC_RADIO_CASE_HISTORY_ALL_DATES, OnRadioCaseHistoryAllDates)
	ON_BN_CLICKED(IDC_RADIO_CASE_HISTORY_DATE_RANGE, OnRadioCaseHistoryDateRange)
	ON_COMMAND(ID_PRINT_ONE, OnPrintOne)
	ON_BN_CLICKED(IDC_RADIO_UNBILLED_CH, OnRadioUnbilled)
	ON_BN_CLICKED(IDC_RADIO_BILLED_CH, OnRadioBilled)
	ON_BN_CLICKED(IDC_RADIO_ALL_BILLED_CH, OnRadioAllBilled)
	ON_BN_CLICKED(ID_LINK_TO_BILL, OnLinkWithExistingBill)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaseHistoriesDlg message handlers

BOOL CCaseHistoriesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_dlCaseHistoryList = BindNxDataListCtrl(IDC_CASE_HISTORY_LIST,false);
	m_radioIncomplete.SetCheck(TRUE);
	m_radioDateRange.SetCheck(TRUE);

	// (j.jones 2009-08-07 10:18) - PLID 35140 - added billed filters,
	// default to showing all case histories indepdendent of the billed status
	m_radioAllBilled.SetCheck(TRUE);

	//default the from and to dates to be 7 days from today
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan dtSpan;
	dtSpan.SetDateTimeSpan(7,0,0,0);
	m_dtFrom.SetValue(_variant_t(COleDateTime(dtNow - dtSpan)));
	m_dtTo.SetValue(_variant_t(COleDateTime(dtNow + dtSpan)));

	// (j.jones 2009-08-07 10:25) - PLID 35140 - added the from clause in code for easier reading
	// (j.jones 2009-08-26 09:49) - PLID 34943 - added LocationName
	CString strFrom;
	strFrom = "( "
		"SELECT CaseHistoryT.ID, CaseHistoryT.Name, CaseHistoryT.PersonID, CaseHistoryT.SurgeryDate, CaseHistoryT.CompletedDate, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
		"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "
		"LocationsT.Name AS LocationName, "
		"dbo.GetCaseHistoryProcedures(CaseHistoryT.ID) AS ProcedureNames, "
		"Convert(bit, (CASE WHEN CaseHistoryT.ID IN ( "
		"	SELECT CaseHistoryID FROM BilledCaseHistoriesT "
		"	INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
		"	WHERE BillsT.Deleted = 0 "
		"	) THEN 1 ELSE 0 END)) AS HasBeenBilled "
		"FROM CaseHistoryT "
		"INNER JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
		"LEFT JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
		"LEFT JOIN PersonT ProviderPersonT ON CaseHistoryT.ProviderID = ProviderPersonT.ID "
		") AS CaseHistoryQ ";
	m_dlCaseHistoryList->PutFromClause(_bstr_t(strFrom));

	ChangeFilterType();	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CCaseHistoriesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCaseHistoriesDlg)
	ON_EVENT(CCaseHistoriesDlg, IDC_CASE_HISTORY_LIST, 3 /* DblClickCell */, OnDblClickCellCaseHistoryList, VTS_I4 VTS_I2)
	ON_EVENT(CCaseHistoriesDlg, IDC_CASE_HISTORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCaseHistoryList, VTS_I2)
	ON_EVENT(CCaseHistoriesDlg, IDC_CASE_HISTORY_LIST, 6 /* RButtonDown */, OnRButtonDownCaseHistoryList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCaseHistoriesDlg::OnDblClickCellCaseHistoryList(long nRowIndex, short nColIndex) 
{
	CWaitCursor wc;
	if (nRowIndex != -1) {
		try {
			// Edit the case that was double-clicked on
			long nCaseHistoryID = VarLong(m_dlCaseHistoryList->GetValue(nRowIndex, chlcID));
			
			//don't let them open the same one twice
			CPtrArray &ary = theApp.m_arypCaseHistories;
			try {
				long nSize = ary.GetSize();
				for (long i=0; i<nSize; i++) {
					CCaseHistoryDlg *pItem = (CCaseHistoryDlg *)ary[i];
					if (pItem->GetSafeHwnd() && pItem->IsWindowVisible() && (pItem->m_nCaseHistoryID == nCaseHistoryID)) {
						GetParent()->MessageBox("This case history is already open in another window.", NULL, MB_OK|MB_ICONINFORMATION);
						pItem->BringWindowToTop();
						return;
					}
				}
			} NxCatchAll("Error searching for existing dialog.");
			
			//create the new dialog
			CCaseHistoryDlg *pDlg = new CCaseHistoryDlg(GetDesktopWindow());		
			if (pDlg) {
				// Got the window, init the key variables
				pDlg->m_nCaseHistoryID = nCaseHistoryID;		//set the case history ID
				// (j.jones 2006-12-13 16:00) - PLID 23578 - added m_bIsNew variable
				pDlg->m_bIsNew = FALSE;
				pDlg->m_pParent = this;
				// Create and show the window
				if (pDlg->Create(CCaseHistoryDlg::IDD, GetDesktopWindow())) {
					// And finally show the window
					pDlg->ShowWindow(SW_SHOW);
					theApp.m_arypCaseHistories.Add(pDlg);
				} else {
					// We should always be able to get a new window for the case history
					ThrowNxException("Could not create the window!");
				}
			} else {
				// We should always be able to get a new window for the case history
				ThrowNxException("Could not get new window for this case history!");
			}

		} NxCatchAll("CCaseHistoriesDlg::OnDblClickCellCaseHistoryList");
	}
}

void CCaseHistoriesDlg::OnClosedCaseHistory(long nCaseHistoryID)
{
	try {

		//first update the view to reflect any changes made
		UpdateView();

		//now delete the tracked window
		CPtrArray &aryOfWnds = theApp.m_arypCaseHistories;
		long nSize = aryOfWnds.GetSize();
		for (long i=aryOfWnds.GetSize()-1; i>=0; i--) {
			CCaseHistoryDlg *pTrackedWnd = (CCaseHistoryDlg *)aryOfWnds[i];
			if (pTrackedWnd->GetSafeHwnd()) {				
				if(pTrackedWnd->m_nCaseHistoryID == nCaseHistoryID) {
					pTrackedWnd->EndDialog(IDCANCEL);
					aryOfWnds.RemoveAt(i);
					delete pTrackedWnd;
					return;
				}
			}		
		}

	}NxCatchAll("Error closing case history.");
}

void CCaseHistoriesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		m_dlCaseHistoryList->Requery();
	} NxCatchAll("CCaseHistoriesDlg::UpdateView");
}


void CCaseHistoriesDlg::OnRadioCompleted() 
{
	ChangeFilterType();	
}

void CCaseHistoriesDlg::OnRadioIncomplete() 
{
	ChangeFilterType();	
}

void CCaseHistoriesDlg::OnRadioAllCompleted() 
{
	ChangeFilterType();
}

void CCaseHistoriesDlg::ChangeFilterType()
{
	try {
		CString strWhere = "";
		if(m_radioCompleted.GetCheck()) {
			strWhere = "CompletedDate IS NOT NULL";
		}
		else if(m_radioIncomplete.GetCheck()) {
			strWhere = "CompletedDate IS NULL";
		}

		// (j.jones 2009-08-07 10:18) - PLID 35140 - added billed filters
		if(m_radioUnbilled.GetCheck()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += "HasBeenBilled = 0";
		}
		else if(m_radioBilled.GetCheck()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += "HasBeenBilled = 1";
		}

		if(m_radioDateRange.GetCheck()) {
			m_dtFrom.EnableWindow(TRUE);
			m_dtTo.EnableWindow(TRUE);
			COleDateTime dtFrom, dtTo;
			COleDateTimeSpan dtOneDay;
			dtOneDay.SetDateTimeSpan(1,0,0,0);
			dtFrom = m_dtFrom.GetValue();
			dtTo = m_dtTo.GetValue();

			//JMJ 5/7/2004 - warning removed because it was annoying
			if(dtTo < dtFrom) {
				//warn them but keep filtering anyways
				//AfxMessageBox("Please ensure your 'from' date is before your 'to' date.");
			}

			dtTo += dtOneDay;			

			CString str;
			str.Format("SurgeryDate >= '%s' AND SurgeryDate < '%s'",FormatDateTimeForSql(dtFrom,dtoDate),FormatDateTimeForSql(dtTo,dtoDate));
			if(strWhere != "")
				strWhere += " AND ";
			strWhere += str;
		}
		else {
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}
		
		m_dlCaseHistoryList->PutWhereClause(_bstr_t(strWhere));
		m_dlCaseHistoryList->Requery();
		
	}NxCatchAll("Error filtering list.");
}

void CCaseHistoriesDlg::OnRequeryFinishedCaseHistoryList(short nFlags) 
{
	try {
		
		for(int i=0;i<m_dlCaseHistoryList->GetRowCount();i++) {
			//color the completed cases
			if(m_dlCaseHistoryList->GetValue(i, chlcCompletedDate).vt == VT_DATE) {
				IRowSettingsPtr(m_dlCaseHistoryList->GetRow(i))->PutForeColor(RGB(0,0,128));

				// (j.jones 2009-08-07 10:34) - PLID 35140 - color the billed column red if not billed,
				// but the case is completed
				if(!VarBool(m_dlCaseHistoryList->GetValue(i, chlcBilled))) {
					IRowSettingsPtr(m_dlCaseHistoryList->GetRow(i))->PutCellForeColor(chlcBilled, RGB(255,0,0));
				}
			}
		}

	}NxCatchAll("Error setting list colors.");
}

void CCaseHistoriesDlg::OnRadioCaseHistoryAllDates() 
{
	ChangeFilterType();
}

void CCaseHistoriesDlg::OnRadioCaseHistoryDateRange() 
{
	ChangeFilterType();
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CCaseHistoriesDlg::OnChangeDtCaseHistoryFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ChangeFilterType();

	*pResult = 0;
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CCaseHistoriesDlg::OnCloseUpDtCaseHistoryFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeFilterType();	

	*pResult = 0;
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CCaseHistoriesDlg::OnChangeDtCaseHistoryTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeFilterType();

	*pResult = 0;
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CCaseHistoriesDlg::OnCloseUpDtCaseHistoryTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeFilterType();

	*pResult = 0;
}

void CCaseHistoriesDlg::OnRButtonDownCaseHistoryList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_dlCaseHistoryList->CurSel = nRow;

	if(nRow == -1) {
		return;
	}

	try {

		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();

		long nCaseHistoryID = VarLong(m_dlCaseHistoryList->GetValue(nRow, chlcID));

		BOOL bHasAppt = FALSE;
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM CaseHistoryT WHERE ID = {INT} AND AppointmentID Is Not Null", nCaseHistoryID);
		if(!rs->eof) {
			bHasAppt = TRUE;
		}
		rs->Close();

		BOOL bBilled = VarBool(m_dlCaseHistoryList->GetValue(nRow, chlcBilled));
	
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_PRINT_ONE, "&Print Case History");
		// (j.jones 2009-08-07 11:26) - PLID 24077 - added ability to link to an existing bill
		pMenu->InsertMenu(-1, !bBilled ? MF_BYPOSITION : MF_BYPOSITION|MF_GRAYED, ID_LINK_TO_BILL, "&Link With An Existing Bill");
		pMenu->InsertMenu(-1, MF_BYPOSITION|MF_SEPARATOR);
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_GOTO_PATIENT, "Go To &Patient");
		pMenu->InsertMenu(-1, bHasAppt ? MF_BYPOSITION : MF_BYPOSITION|MF_GRAYED, ID_GOTO_APPOINTMENT, "Go To &Appointment");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	
	}NxCatchAll("Error in OnRButtonDownCaseHistoryList");
}

BOOL CCaseHistoriesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CCaseHistoriesDlg::OnGoToPatient()
{
	if(m_dlCaseHistoryList->GetCurSel()==-1)
		return;

	try {

		//don't let them leave if they have open case histories
		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					MessageBox("Please close all open Case Histories before leaving the Surgery Center module.");
					return;
				}
			}
		}

		long nCaseHistoryID = VarLong(m_dlCaseHistoryList->GetValue(m_dlCaseHistoryList->GetCurSel(), chlcID));
		long nPatientID = -1;
		_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM CaseHistoryT WHERE ID = %li",nCaseHistoryID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PersonID",-1);
		}
		rs->Close();

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
				}

			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CaseHistoriesDlg.cpp: Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID
	}NxCatchAll("Error in OnGoToPatient");
}

void CCaseHistoriesDlg::OnGoToAppointment()
{
	if(m_dlCaseHistoryList->GetCurSel()==-1)
		return;

	try {

		//don't let them leave if they have open case histories
		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					MessageBox("Please close all open Case Histories before leaving the Surgery Center module.");
					return;
				}
			}
		}

		long nCaseHistoryID = VarLong(m_dlCaseHistoryList->GetValue(m_dlCaseHistoryList->GetCurSel(), chlcID));
		long nAppointmentID = -1;
		_RecordsetPtr rs = CreateRecordset("SELECT AppointmentID FROM CaseHistoryT WHERE ID = %li",nCaseHistoryID);
		if(!rs->eof) {
			nAppointmentID = AdoFldLong(rs, "AppointmentID",-1);
		}
		rs->Close();

		if (nAppointmentID != -1) {
			//see whether the appointments been cancelled
			if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE Status = 4 AND ID = %li",nAppointmentID)) {
				MsgBox("This appointment has been cancelled");
				return;
			}
					
			CMainFrame  *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
					CNxTabView *pView = pMainFrame->GetActiveView();
					if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
						((CSchedulerView *)pView)->OpenAppointment(nAppointmentID, GetRemotePropertyInt("ApptAutoOpenResEntry", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE);
					}//end pView
				}
			}//end pMainFrame
			else {
				//MainFrame pointer is null
				MsgBox(RCS(IDS_APPOINTMENT_ERROR_1667));
			}
		}
	}NxCatchAll("Error in OnGoToAppointment");
}

void CCaseHistoriesDlg::OnPrintOne()
{
	try {

		long nRow = m_dlCaseHistoryList->GetCurSel();

		if(nRow == -1)
			return;

		// Show immediate feedback by way of the wait cursor
		CWaitCursor wc;

		// Create a copy of the report object
		CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(387)]);
		
		// Tell the report that we want to filter on this case history
		rep.nExtraID = VarLong(m_dlCaseHistoryList->GetValue(nRow, chlcID));

		//check to see if there is a default report
		_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 387");
		CString strFileName;

		if (rsDefault->eof) {

			strFileName = "CaseHistoryIndiv";
		}
		else {
			
			long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

			if (nDefaultCustomReport > 0) {

				_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 387 AND Number = %li", nDefaultCustomReport);

				if (rsFileName->eof) {

					//this should never happen
					MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
				}
				else {
					
					//set the default
					rep.nDefaultCustomReport = nDefaultCustomReport;
					strFileName =  AdoFldString(rsFileName, "FileName");
				}
			}
			else {
				//if this occurs it just means they want the default, which in this case, there is only one
				strFileName = "CaseHistoryIndiv";
				
			}
		}

		//Made new function for running reports - JMM 5-28-04
		RunReport(&rep, TRUE, (CWnd *)this, "Case History");
	
	} NxCatchAll("Error printing case history.");
}

// (j.jones 2009-08-07 10:18) - PLID 35140 - added billed filters
void CCaseHistoriesDlg::OnRadioUnbilled()
{
	ChangeFilterType();
}

void CCaseHistoriesDlg::OnRadioBilled()
{
	ChangeFilterType();
}

void CCaseHistoriesDlg::OnRadioAllBilled()
{
	ChangeFilterType();
}

// (j.jones 2009-08-07 11:26) - PLID 24077 - added ability to link to an existing bill
void CCaseHistoriesDlg::OnLinkWithExistingBill()
{
	try {

		long nRow = m_dlCaseHistoryList->GetCurSel();
		if(nRow == -1) {
			return;
		}

		IRowSettingsPtr pRow = m_dlCaseHistoryList->GetRow(nRow);

		if(VarBool(pRow->GetValue(chlcBilled))) {
			//should not have been called on a case that has already been billed
			ASSERT(FALSE);
			return;
		}

		long nPatientID = VarLong(pRow->GetValue(chlcPatientID));

		if(nPatientID == -1) {	
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//it is debateable whether this is considered "writing to the bill",
		//but check permissions anyways, that's what linking EMNs to bills does too
		if(!CheckCurrentUserPermissions(bioBill,sptWrite)) {
			return;
		}

		//do not open the dialog if they have no bills at all
		// (j.gruber 2011-06-27 14:52) - PLID 44874 - don't show voided bills or charges
		// (b.eyers 2015-02-20) - PLID 44337 - Exclude quotes, dialog was displaying still if patient had quotes only
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.ID FROM BillsT "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidsT ON LineItemT.ID = VoidsT.VoidingLineItemID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillCorrectionsT.ID IS NULL "
			" AND LineItemCorrectionsT.ID IS NULL AND VoidsT.ID IS NULL  "
			"AND BillsT.EntryType = 1 AND BillsT.PatientID = {INT} ", nPatientID);
		if(rs->eof) {
			AfxMessageBox("This patient has no bills on their account.");
			return;
		}
		rs->Close();

		CLinkCaseHistoryToBillDlg dlg(this);
		dlg.m_nPatientID = VarLong(pRow->GetValue(chlcPatientID));
		dlg.m_nCaseHistoryID = VarLong(pRow->GetValue(chlcID));
		dlg.m_strCaseHistoryDesc = VarString(pRow->GetValue(chlcDescription));
		dlg.m_dtCaseHistoryDate = VarDateTime(pRow->GetValue(chlcSurgeryDate));
		dlg.m_strPatientName = VarString(pRow->GetValue(chlcPatientName));
		if(IDOK == dlg.DoModal()) {
			//if the user was able to click OK on this dialog,
			//then we linked the case to a bill, and can now
			//update it in the list

			if(m_radioUnbilled.GetCheck()) {
				//we're filtering on unbilled cases, so remove the row
				m_dlCaseHistoryList->RemoveRow(nRow);
			}
			else {
				//update the row contents
				pRow->PutValue(chlcBilled, g_cvarTrue);
				
				//if the case is completed, the cell needs to be dark blue,
				//otherwise make sure it is black
				if(pRow->GetValue(chlcCompletedDate).vt == VT_DATE) {
					pRow->PutCellForeColor(chlcBilled, RGB(0,0,128));
				}
				else {
					pRow->PutCellForeColor(chlcBilled, RGB(0,0,0));
				}
			}
		}

	}NxCatchAll("Error in CCaseHistoriesDlg::OnLinkWithExistingBill");
}