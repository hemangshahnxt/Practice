// MoveUpListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerRc.h"
#include "GlobalUtils.h"
#include "GlobalSchedUtils.h"
#include "CommonSchedUtils.h"
#include "MoveUpListDlg.h"
#include "SchedulerView.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "MultiSelectDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "WaitingListEntryDlg.h"
#include "AuditTrail.h"
#include "SharedScheduleUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



# define IDM_GO_TO_APPT   13482
# define IDM_REMOVE_MOVE_UP   13483
# define IDM_GO_TO_WAIT_LIST   13484
# define IDM_REMOVE_FROM_WAIT_LIST   13485

enum EMoveUpColumns {
	mucWaitListID, 
	mucAppointmentID, 
	mucLineItemID, 
	mucPatientID, 
	mucTypeID, 
	mucColor, 
	mucPhone, 
	mucName, 
	mucApptFlag, 
	mucApptStartDateTime, //(e.lally 2009-08-13) PLID 28591
	mucApptType, 
	mucPurpose, 
	mucInputDate,	// (c.haag 2009-08-07 09:50) - PLID 11699 - Input Date
	mucNotes
};

enum EMoveUpPurposeColumns {
	mupcID, 
	mupcName
};

enum EMoveUpAppointmentTypeColumns {
	muatcID, 
	muatcName
};

enum EMoveUpResourceColumns {
	murcID, 
	murcName
};

//(e.lally 2009-08-14) PLID 35226 - columns for the list of available fields to sort by.
enum ESortColumns {
	scColumn,
	scField,
};

/////////////////////////////////////////////////////////////////////////////
// CMoveUpListDlg dialog


CMoveUpListDlg::CMoveUpListDlg(CWnd* pParent)
	: CNxDialog(CMoveUpListDlg::IDD, pParent)
{
	m_bMoveUpRequeryEnabled = TRUE;
	//m_szCancelBtn.cx = m_szCancelBtn.cy = 0;

	// (j.armen 2012-06-06 12:39) - PLID 50830 - Set min size
	SetMinSize(690, 284);
}

// (c.haag 2009-08-07 09:55) - PLID 11699 - This function will see which columns
// the user wants visible, and update the appropriate checkboxes
void CMoveUpListDlg::LoadVisibleColumnDefaults()
{
	// Load the data and set the checkbox on screen appropriately.
	if (GetRemotePropertyInt("MoveUpIncludeInputDate", 1, 0, GetCurrentUserName(), false) != 0) {
		CheckDlgButton(IDC_CHECK_MOVEUP_SHOWINPUTDATE, 1);
	} else {
		CheckDlgButton(IDC_CHECK_MOVEUP_SHOWINPUTDATE, 0);
	}

	//(e.lally 2009-08-13) PLID 28591
	if (GetRemotePropertyInt("MoveUpIncludeApptStartDateTime", 1, 0, GetCurrentUserName(), false) != 0) {
		CheckDlgButton(IDC_CHECK_MOVEUP_SHOW_APPT_START, 1);
	} else {
		CheckDlgButton(IDC_CHECK_MOVEUP_SHOW_APPT_START, 0);
	}

	// Reflect the state of the checkboxes
	EnsureAllVisibleColumnWidths();
}


// (c.haag 2009-08-07 10:02) - PLID 11699 - This function will ensure a single optional column
// is at its appropriate size.
void CMoveUpListDlg::EnsureVisibleColumnWidth(UINT nCheckID, short col, long nWidth)
{
	IColumnSettingsPtr pCol;
	BOOL bIsChecked = IsDlgButtonChecked(nCheckID);
	pCol = m_pMoveUpList->GetColumn(col);
	if ((pCol->StoredWidth == 0L && bIsChecked) || (pCol->StoredWidth > 0L && !bIsChecked))
	{
		if (bIsChecked) {
			pCol->ColumnStyle = csVisible | csWidthData;
			pCol->StoredWidth = nWidth;
		}
		else {
			pCol->ColumnStyle = csVisible | csFixedWidth;
			pCol->StoredWidth = 0;
		}
	}
}

// (c.haag 2009-08-07 09:59) - PLID 11699 - This function will update the visible columns to
// ensure they are the proper sizes.
void CMoveUpListDlg::EnsureAllVisibleColumnWidths()
{
	//(e.lally 2009-08-13) PLID 28591
	EnsureVisibleColumnWidth(IDC_CHECK_MOVEUP_SHOW_APPT_START, mucApptStartDateTime, 105);
	EnsureVisibleColumnWidth(IDC_CHECK_MOVEUP_SHOWINPUTDATE, mucInputDate, 65);
}

void CMoveUpListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoveUpListDlg)
	DDX_Control(pDX, IDC_MULTIPLE_RESOURCE_LABEL, m_nxlMultipleResourceLabel);
	DDX_Control(pDX, IDC_DATE_MOVEUP, m_dtcMoveUp);
	DDX_Control(pDX, IDC_DATE_START, m_dtcStartDate);
	DDX_Control(pDX, IDC_DATE_END, m_dtcEndDate);
	DDX_Control(pDX, IDC_STATIC_MOVEUPDATETIME, m_nxstaticMoveupdatetime);
	DDX_Control(pDX, IDC_ADD_WAIT_LIST_PATIENT, m_btnAddWaitListPatient);
	DDX_Control(pDX, IDC_BTN_MOVEUP, m_btnMoveUp);
	DDX_Control(pDX, IDC_PREVIEW_MOVEUP_LIST, m_btnPreviewMoveUpList);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_CHECK_MOVEUP_SHOWINPUTDATE, m_btnShowInputDate);
	DDX_Control(pDX, IDC_CHECK_MOVEUP_SHOW_APPT_START, m_btnShowApptStart);
	DDX_Control(pDX, IDC_MOVEUP_SORT_ASC, m_radioSortAsc);
	DDX_Control(pDX, IDC_MOVEUP_SORT_DESC, m_radioSortDesc);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CMoveUpListDlg, IDC_DATE_START, 2 /* Change */, OnChangeDateStart, VTS_NONE)
//	ON_EVENT(CMoveUpListDlg, IDC_DATE_END, 2 /* Change */, OnChangeDateEnd, VTS_NONE)

// (a.walling 2008-05-13 14:57) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CMoveUpListDlg, CNxDialog)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_START, OnChangeDateStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_END, OnChangeDateEnd)
	ON_BN_CLICKED(IDC_GOTOAPPOINTMENT, OnGotoappointment)
	ON_BN_CLICKED(IDC_BTN_MOVEUP, OnMoveUp)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_PREVIEW_MOVEUP_LIST, OnPreviewMoveupList)
	ON_BN_CLICKED(IDC_ADD_WAIT_LIST_PATIENT, OnAddWaitListPatient)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CHECK_MOVEUP_SHOWINPUTDATE, OnCheckShowInputDate)
	ON_BN_CLICKED(IDC_CHECK_MOVEUP_SHOW_APPT_START, OnCheckShowApptStart)
	ON_BN_CLICKED(IDC_MOVEUP_SORT_ASC, OnRadioSortMoveUpListAscending)
	ON_BN_CLICKED(IDC_MOVEUP_SORT_DESC, OnRadioSortMoveUpListDescending)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveUpListDlg message handlers

BOOL CMoveUpListDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {

		// (z.manning, 04/30/2008) - PLID 29814 - Set button styles
		m_btnAddWaitListPatient.AutoSet(NXB_NEW);
		m_btnMoveUp.AutoSet(NXB_MODIFY);
		m_btnPreviewMoveUpList.AutoSet(NXB_PRINT_PREV);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_pMoveUpList = BindNxDataList2Ctrl(IDC_MOVEUPLIST, false);
		m_pMoveUpListSort = BindNxDataList2Ctrl(IDC_MOVEUPLIST_SORT, false);
		m_pPurposeList = BindNxDataList2Ctrl(IDC_APPT_PURPOSE, false);
		m_pTypeList = BindNxDataList2Ctrl(IDC_APPT_TYPE_LIST, false);
		m_pResourceList = BindNxDataList2Ctrl(IDC_APPT_RESOURCE, true);
		m_nxtMoveUp = BindNxTimeCtrl(this, IDC_MOVEUP_TIME_BOX);

		// The resource filter should default to being empty
		m_strResourceFilter = "";
		
		//setup the Appointment list
		m_pTypeList->Requery();

		//add the {All Purposes}
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pPurposeList->GetNewRow();
		pRow->PutValue(mupcID, (long) -1);
		pRow->PutValue(mupcName, _variant_t("{All Purposes}"));
		m_pPurposeList->AddRowBefore(pRow, m_pPurposeList->GetFirstRow());
		m_pPurposeList->SetSelByColumn(mupcID, (long) -1);		

		// Assign a default move up date
		m_dtcMoveUp.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
		
		// (d.moore 2007-05-10 17:11) - PLID 4013 - 
		//  Set appropriate start and end times for the date filter.
		COleDateTime dtDateFilterVal = COleDateTime::GetCurrentTime();
		m_dtcStartDate.SetValue(COleVariant(dtDateFilterVal));
		m_dtcEndDate.SetValue(COleVariant(dtDateFilterVal));

		// (d.moore 2007-05-17 17:08) - PLID 4013 - 
		//  Since not every row is an appointment now, it isn't always 
		//  appropriate to let this button be enabled.
		GetDlgItem(IDC_GOTOAPPOINTMENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MOVEUP)->EnableWindow(FALSE);

		// Hyperlink for displaying multiple selected purpose values.
		m_nxlMultipleResourceLabel.SetText("");
		m_nxlMultipleResourceLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_HIDE);
		m_nxlMultipleResourceLabel.SetColor(GetSysColor(COLOR_3DFACE));

		//(e.lally 2009-08-13) PLID 28591 - Bulk cache preferences
		// (j.jones 2014-11-14 14:32) - PLID 64169 - added AutoFillApptInsurance and AutoFillApptInsurance_DefaultCategory
		g_propManager.CachePropertiesInBulk("MoveUpList", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'MoveUpIncludeApptStartDateTime' \r\n"
			"	, 'MoveUpIncludeInputDate' \r\n"
			"	, 'MoveUpLastSortColumn' \r\n"
			"	, 'MoveUpLastSortAscending' \r\n"
			"	, 'AutoFillApptInsurance' "
			"	, 'AutoFillApptInsurance_DefaultCategory' "
			")"
			, _Q(GetCurrentUserName()));

		// (c.haag 2009-08-07 09:54) - PLID 11699 - Load all optional column
		// sizes from data.
		LoadVisibleColumnDefaults();

		//(e.lally 2009-08-14) PLID 35226 - Manually build the list of columns that we can sort by
		LoadSortOptions();

		//(e.lally 2009-08-14) PLID 35226 - Load the last used ascending/descending sort
		BOOL bSortAscending = GetRemotePropertyInt("MoveUpLastSortAscending", 1, 0, GetCurrentUserName()) == 0 ? FALSE: TRUE;
		if(bSortAscending){
			CheckDlgButton(IDC_MOVEUP_SORT_ASC, 1);
		}
		else{
			CheckDlgButton(IDC_MOVEUP_SORT_DESC, 1);
		}

		//  The fill the waiting list, and sets the colors for the rows.
		Requery();
	} NxCatchAll("Error In: CMoveUpListDlg::OnInitDialog");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMoveUpListDlg::OnOK() 
{
	CWnd::DestroyWindow();
}

void CMoveUpListDlg::OnCancel() 
{
	CWnd::DestroyWindow();
}


BEGIN_EVENTSINK_MAP(CMoveUpListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMoveUpListDlg)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST, 18 /* RequeryFinished */, OnRequeryFinishedMoveuplist, VTS_I2)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST, 6 /* RButtonDown */, OnRButtonDownMoveuplist, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_TYPE_LIST, 16 /* SelChosen */, OnSelChosenApptTypeList, VTS_DISPATCH)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_PURPOSE, 16 /* SelChosen */, OnSelChosenApptPurpose, VTS_DISPATCH)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_TYPE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedApptTypeList, VTS_I2)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_PURPOSE, 18 /* RequeryFinished */, OnRequeryFinishedApptPurpose, VTS_I2)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_RESOURCE, 16 /* SelChosen */, OnSelChosenApptResource, VTS_DISPATCH)
	ON_EVENT(CMoveUpListDlg, IDC_APPT_RESOURCE, 18 /* RequeryFinished */, OnRequeryFinishedApptResource, VTS_I2)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST, 2 /* SelChanged */, OnSelChangedMoveuplist, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST, 3 /* DblClickCell */, OnDblClickCellMoveuplist, VTS_DISPATCH VTS_I2)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST_SORT, 2 /* SelChanged */,OnSelChangedMoveupListSort, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CMoveUpListDlg, IDC_MOVEUPLIST_SORT, 16 /* SelChosen */, OnSelChosenMoveupListSort, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMoveUpListDlg::OnRequeryFinishedMoveuplist(short nFlags) 
{
	try {
		// (d.moore 2007-10-26) - PLID 26546 - Format phone numbers and add them to the list beside
		//  the patient's name.
		CString strName, strPhone, strFormattedPhone;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->GetFirstRow();
		while (pRow != NULL) {
			strName = VarString(pRow->GetValue(mucName), "");
			strPhone = VarString(pRow->GetValue(mucPhone), "");
			strPhone.TrimRight();
			strFormattedPhone = "";
			if (!strPhone.IsEmpty()) {
				FormatText (strPhone, strFormattedPhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
			}
			strName += " " + strFormattedPhone;
			pRow->PutValue(mucName, (_variant_t)strName);
			pRow->PutValue(mucPhone, (_variant_t)strFormattedPhone);
			pRow = pRow->GetNextRow();
		}


		SetAppointmentColors();
		// Add line items to each waiting list entry.
		QueryLineItemCollection();
	}NxCatchAll("Error in MoveUpDlg::FinishedRequery");	
}


void CMoveUpListDlg::OnRButtonDownMoveuplist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	// (d.moore 2007-05-10 13:23) - PLID 4013 - Change to NxDatalist2
	try {
		if (lpRow != NULL)  {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			//select the row in the datalist
			if (pRow != m_pMoveUpList->CurSel) {
				m_pMoveUpList->CurSel = pRow;
			}
			if (pRow == NULL) {
				return;
			}
			
			// Build a menu popup with the ability to delete the current row
			CMenu  menPopup;
			menPopup.m_hMenu = CreatePopupMenu();
			
			// Determine if the waiting list entry is an actual appointment 
			//  or just a request for one.
			if (VarLong(pRow->GetValue(mucAppointmentID), 0) > 0) {
				// Appointment.
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_GO_TO_APPT, "Go To Appointment");
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_GO_TO_WAIT_LIST, "Go To Wait List Details");
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_REMOVE_MOVE_UP, "Remove Move-Up Status");
			} else {
				// Request (line item).
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_GO_TO_WAIT_LIST, "Go To Wait List Details");
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_REMOVE_FROM_WAIT_LIST, "Remove From Wait List");
			}

			CPoint pt;
			GetCursorPos(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnRButtonDownMoveuplist");
}


BOOL CMoveUpListDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (wParam) {
			
			case IDM_GO_TO_APPT:
				SwitchToAppointment();
			break;

			case IDM_REMOVE_MOVE_UP: {
				if(m_pMoveUpList->CurSel == NULL){
					break;
				}

				// (d.moore 2007-05-10 13:07) - PLID 4013 - The functionality for removing
				//  an item from the waiting list has changed considerably. It has also all
				//  been moved into the RemoveFromWaitList function.
				CString strQuestion = "Are you sure you want to remove this appointment from the waiting list?";
				if (AfxMessageBox(strQuestion, MB_ICONQUESTION | MB_YESNO) == IDYES) {
					RemoveFromWaitList(m_pMoveUpList->CurSel);

					CSchedulerView* pView = (CSchedulerView*)(GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME));
					if (pView) {
						pView->UpdateView();
					}
				}
			}	
			break;

			case IDM_GO_TO_WAIT_LIST: {
				CWaitingListEntryDlg dlg(this);
				dlg.m_WaitListID = m_pMoveUpList->CurSel->GetValue(mucWaitListID);
				if (dlg.DoModal() == IDOK) {
					Requery();
				}
			}
			break;
			
			case IDM_REMOVE_FROM_WAIT_LIST: {
				// (d.moore 2007-10-25) - PLID 4013 - Prompt the user to make sure they want to remove the request
				//  from the waiting list.
				CString strQuestion = "Are you sure you want to remove this request from the waiting list?";
				if (AfxMessageBox(strQuestion, MB_ICONQUESTION | MB_YESNO) == IDYES) {
					RemoveFromWaitList(m_pMoveUpList->CurSel);

					CSchedulerView* pView = (CSchedulerView*)(GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME));
					if (pView) {
						pView->UpdateView();
					}
				}
			}
			break;

			default:
			break;
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnCommand");

	return CNxDialog::OnCommand(wParam, lParam);
}


void CMoveUpListDlg::SwitchToAppointment() {
	
	try {
		EnsureRemoteData();
		//Open the recordset
		//First make sure the they have selected a record
		
		// (d.moore 2007-05-10 13:10) - PLID 4013 - Convert to NxDataList2
		if (m_pMoveUpList->CurSel == NULL) {
			MsgBox("Please select an appointment");
			return;
		}
		
		long nAppointmentID = VarLong(m_pMoveUpList->CurSel->GetValue(mucAppointmentID), -1);
		
		if (nAppointmentID > 0) {
			OpenAppointment(nAppointmentID);
		}
	}NxCatchAll("Error in SwitchToAppointment()");
}

void CMoveUpListDlg::OnDblClickCellMoveuplist(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		if (lpRow == NULL)  {
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		long nAppointmentID = VarLong(pRow->GetValue(mucAppointmentID), 0);
		if (nAppointmentID > 0) {
			SwitchToAppointment();
		} else {
			CWaitingListEntryDlg dlg(this);
			dlg.m_WaitListID = pRow->GetValue(mucWaitListID);
			if (dlg.DoModal() == IDOK) {
				Requery();
			}
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnDblClickCellMoveuplist");
}

void CMoveUpListDlg::OnGotoappointment() 
{
	try {
		//get the current selection
		long nCurSel = m_pMoveUpList->CurSel;

		if (nCurSel == -1 ) {
			MsgBox("Please select an appointment");
		}
		else {
			SwitchToAppointment();
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnGotoappointment");
}

void CMoveUpListDlg::OnMoveUp()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->CurSel;
		if (pRow == NULL) {
			return;
		}

		// (d.moore 2007-07-10 11:03) - PLID 4013 - 
		//  Check to make sure that a valid time was entered. If nothing
		//  has been entered int the time box then it will return midnight.
		//  It should usually be safe to assume that this is not going to
		//  be a valid selection.
		COleDateTime dtTime = m_nxtMoveUp->GetDateTime();
		if (dtTime.GetHour() == 0 && dtTime.GetMinute() == 0) {
			MessageBox("Please enter a valid time.");
			return;
		}

		// Check that the date is valid. 
		COleDateTime dtDate = m_dtcMoveUp.GetValue();
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		if (CompareDateIgnoreTime(dtDate, dtToday) < 0) {
			MessageBox("The date you have entered is earlier than today's date.");
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
		long nApptID = VarLong(pParentRow->GetValue(mucAppointmentID), 0);
		if (nApptID > 0) {
			// This is an actual appointment.
			MoveUpAppointment();
		} else {
			// This is a waiting list request for an appointment.
			TryScheduleNewAppt();
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnMoveUp");
}

void CMoveUpListDlg::MoveUpAppointment()
{

	COleDateTime dtDate = m_dtcMoveUp.GetValue();
	COleDateTime dtTime = m_nxtMoveUp->GetDateTime();
	COleDateTime dtDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
		dtTime.GetHour(), dtTime.GetMinute(), dtTime.GetSecond());

	if (m_pMoveUpList->CurSel == NULL) {
		MsgBox("Please select an appointment");
		return;
	}
	if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to move up the selected appointment to %s?", FormatDateTimeForInterface(dtDateTime)))
		return;

	try
	{
		// (d.moore 2007-05-10 13:23) - PLID 4013 - Get the appointment ID for the selected
		//  item. If the item is a resource request then get its parent row and get the
		//  ID value from it.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
		long nResID = VarLong(pParentRow->GetValue(mucAppointmentID));

		// (r.gonet 2015-08-26) - PLID 66954 - Parameterized.
		_RecordsetPtr prsAppt = CreateParamRecordset("SELECT PatientID, LocationID, StartTime, EndTime, ArrivalTime, ShowState, [Notes], AptTypeID, Ready, RequestedResourceID FROM AppointmentsT WHERE ID = {INT}", nResID);
		FieldsPtr pFld = prsAppt->Fields;
		if (prsAppt->eof)
		{
			// (c.haag 2003-08-08 13:58) - Only assumes deletion
			MsgBox("This appointment has been deleted by another user, and can not be moved up.");
			if (m_bMoveUpRequeryEnabled)
				m_pMoveUpList->Requery();
			return;
		}
		
		// Calculate the new times of the appointment
		COleDateTime dtStart = AdoFldDateTime(prsAppt, "StartTime");
		COleDateTime dtEnd = AdoFldDateTime(prsAppt, "EndTime");
		COleDateTime dtArrival = AdoFldDateTime(prsAppt, "ArrivalTime");
		COleDateTimeSpan dtsStartEnd, dtsStartArrival;
		if (dtEnd < dtStart)
		{
			MsgBox("The appointment end time is before the start time. Please fix the appointment before moving it up.");
			OnGotoappointment();
			return;
		}
		dtsStartEnd = dtEnd - dtStart;
		dtsStartArrival = dtStart - dtArrival;
		dtStart = dtTime;
		dtEnd = dtTime + dtsStartEnd;
		dtArrival = dtTime - dtsStartArrival;

		// Get the resource and purpose lists
		CDWordArray adwPurposeID, adwResourceID;
		SchedAuditItems items;
		// (r.gonet 2015-08-26) - PLID 66954 - Parameterized. Ordered.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Item, ResourceID FROM AppointmentResourceT LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentID = {INT} ORDER BY ResourceID ASC", nResID);
		while (!rs->eof)
		{
			items.aryOldResource.Add(AdoFldString(rs, "Item"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		// (r.gonet 2015-08-26) - PLID 66954 - Parameterized. Ordered.
		rs = CreateParamRecordset("SELECT Name, PurposeID FROM AppointmentPurposeT LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentID = {INT} ORDER BY PurposeID ASC", nResID);
		while (!rs->eof)
		{
			items.aryOldPurpose.Add(AdoFldString(rs, "Name"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		// Get the view for the scheduler.
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);

		// (r.gonet 2015-08-26) - PLID 66954 - Get the wait list ID so we can look up the associated WaitingListPurposesT records.
		long nWaitListID = VarLong(pRow->GetValue(mucWaitListID));
		// Get the wait list item ID so we can look up whether the wait list detail has all resources from WaitingListItemsT or the individual resources from WaitingListItemResourceT.
		long nLineItemID = VarLong(pRow->GetValue(mucLineItemID));

		// Get Resource IDs.

		// First check to see if all resources are selected.
		bool bAllResources = false;
		bool bMultipleResources = false;
		long nResourceID = -1;
		// (r.gonet 2015-08-26) - PLID 66954 - Parameterized.
		_RecordsetPtr prs = CreateParamRecordset("SELECT AllResources FROM WaitingListItemT WHERE ID = {INT}", nLineItemID);
		if (!prs->eof) {
			bAllResources = !!AdoFldBool(prs, "AllResources");
		}
		prs->Close();

		if (bAllResources) {
			// All resources was selected for the item. So assume that they
			//  probably want to sechedule this for the active resource, not
			//  every resource.
			nResourceID = ((CSchedulerView*)pView)->GetActiveResourceID();
			adwResourceID.Add(nResourceID);

			// (r.gonet 2015-08-26) - PLID 66954 - Get the name of the resource.
			CString strResourceName;
			if (nResourceID != CSchedulerView::sdrUnknownResource) {
				prs = CreateParamRecordset(R"(
SELECT ResourceT.Item
FROM ResourceT 
WHERE ResourceT.ID = {INT}
)", nResourceID);
				if (prs->eof) {
					ThrowNxException("%s : Resource with ID = %li does not exist. This appointment could not be moved up.", __FUNCTION__, nResourceID);
				}
				strResourceName = AdoFldString(prs, "Item");
			} else {
				// (r.gonet 2015-08-26) - PLID 66954 - This was being allowed before and it would produce a FK exception. It is a rare case and frankly you 
				// have to be working hard to get here. You have to have a user permissioned to see the scheduler but not permissioned to see any of the 
				// current views' resources. It is arguable what should happen here and is out of this item's scope.
				MessageBox("The appointment could not be moved up because the resource to use could not be determined.", "Error", MB_ICONERROR | MB_OK);
				return;
			}

			// (r.gonet 2015-08-26) - PLID 66954 - Changed this from (char)nResourceID, an inconsistency which luckily did not cause any bugs, to the actual resource name
			items.aryNewResource.Add(strResourceName);
		} else {
			// Use the selected resource ID values.
			// (r.gonet 2015-08-26) - PLID 66954 - Parameterized.
			prs = CreateParamRecordset(R"(
SELECT ResourceT.ID, ResourceT.Item
FROM WaitingListItemResourceT 
INNER JOIN ResourceT ON WaitingListItemResourceT.ResourceID = ResourceT.ID
WHERE WaitingListItemResourceT.ItemID = {INT}
ORDER BY ResourceT.ID ASC
)", nLineItemID);
			if (prs->GetRecordCount() > 1) {
				// This is used later to help determine which sceduler tab to use.
				bMultipleResources = true;
			}
			while (!prs->eof) {
				nResourceID = AdoFldLong(prs, "ID");
				CString strResourceName = AdoFldString(prs, "Item");

				// (r.gonet 2015-08-26) - PLID 66954 - Previously, the following two array adds used to be conditional on whether nResourceID > 0. 0 being the sentinel value 
				// in case the WaitingListItemResourceT.ResourceID was null. But ResourceID is non-null and has a FK constraint referencing ResourceT.ID, so it 
				// is never null and it is never 0. Thus the comparison was unnecessary and I removed it.
				adwResourceID.Add(nResourceID);
				// (r.gonet 2015-08-26) - PLID 66954 - Changed this from (char)nResourceID, an inconsistency which luckily did not cause any bugs, to the actual resource name.
				items.aryNewResource.Add(strResourceName);
				prs->MoveNext();
			}
		}
		

		// Get Purpose IDs.
		// (r.gonet 2015-08-26) - PLID 66954 - Compare using the wait list ID rather than the wait list item ID. This fixes a bug where we were comparing
		// the wait list item ID to the WaitingListPurposeT.WaitingListID. Parameterized.
		prs = CreateParamRecordset(R"(
SELECT AptPurposeT.ID, AptPurposeT.Name
FROM WaitingListPurposeT
INNER JOIN AptPurposeT ON WaitingListPurposeT.PurposeID = AptPurposeT.ID
WHERE WaitingListPurposeT.WaitingListID = {INT}
ORDER BY AptPurposeT.ID ASC
)", nWaitListID);
		long nPurposeID;
		while (!prs->eof) {
			nPurposeID = AdoFldLong(prs, "ID");
			// (r.gonet 2015-08-26) - PLID 66954 - Get the name of the purpose.
			CString strPurposeName = AdoFldString(prs, "Name");

			// (r.gonet 2015-08-26) - PLID 66954 - Previously, and bizarrly, the following two array adds used to be conditional on whether nResourceID > 0. I think this was a copy paste bug and it
			// should have been nPurposeID > 0, which was the sentinel value in case the WaitingListPurposeT.PurposeID was null. But PurposeID is non-null and has a FK constraint referencing
			// AptPurposeT.ID, so it is never null and it is never 0. Thus the comparison was unnecessary and I removed it.
			adwPurposeID.Add(nPurposeID);
			// (r.gonet 2015-08-26) - PLID 66954 - Changed this from (char)nPurposeID, an inconsistency which was causing a bug in AppointmentModify(), to the actual purpose name.
			items.aryNewPurpose.Add(strPurposeName);
			prs->MoveNext();
		}

		
		if (!AppointmentModify(nResID, VarLong(pParentRow->GetValue(mucPatientID)), 
			adwResourceID, AdoFldLong(pFld, "LocationID"),
			dtDate, dtStart, dtEnd, dtArrival, 0 /* This has not been confirmed yet */,
			false /* No need to make this move up anymore */,
			AdoFldLong(pFld, "ShowState"), VarString(pParentRow->GetValue(mucNotes)),
			VarLong(pParentRow->GetValue(mucTypeID), -1), adwPurposeID, &items,
			AdoFldBool(pFld, "Ready"), AdoFldLong(pFld, "RequestedResourceID", -1)))
		{
			MsgBox("This appointment could not be moved up");
			return;
		}


		// Flip to the resource view. We should always be in the scheduler module
		// at this point.
		if (pView) {
			if (bMultipleResources) {
				// Switch to the resource tab if more than one resource is in use.
				pView->SetActiveTab(SchedulerModule::MultiResourceTab);
			}
			// (r.gonet 2015-08-26) - PLID 66954 - Be sure to check if the resource is actually in the current view before trying to set it as the active resource.
			else if (nResourceID > 0 && ((CSchedulerView*)pView)->IsResourceInCurView(nResourceID)) {
				((CSchedulerView*)pView)->SetActiveResourceID(nResourceID, true);
			}
			((CSchedulerView*)pView)->UpdateView();
			// Go to the day and time, and open the resentry dialog.
			((CSchedulerView *)pView)->OpenAppointment(nResID, TRUE);
		}
	}
	NxCatchAll(FormatString("%s : Error moving up appointment", __FUNCTION__));		
}

void CMoveUpListDlg::TryScheduleNewAppt()
{
	// Check for permissions to create an appointment.
	if (!CheckCurrentUserPermissions(bioAppointment, sptCreate)) {
		return;
	}
	
	// Make sure there is a row selected.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->CurSel;	
	if (pRow == NULL) {
		return;
	}

	try {
		
		// Prompt the user to make sure they want to turn this item into an appointment.
		// Get start and end times for the new appointment.
		COleDateTime dtStartDate = m_dtcMoveUp.GetValue();
		COleDateTime dtStartTime = m_nxtMoveUp->GetDateTime();
		COleDateTime dtDateTime(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay(),
			dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to schedule the selected appointment on %s?", FormatDateTimeForInterface(dtDateTime))) {
			return;
		}

		long nLineItemID = VarLong(pRow->GetValue(mucLineItemID), 0);
		CDWordArray adwResources, adwPurposes;
		long nResID = -1;
		
		// Get the view for the scheduler.
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);

		// Get Resource IDs.

		// First check to see if all resources are selected.
		CString strQuery;
		bool bAllResources = false;
		bool bMultipleResources = false;
		long nResourceID = -1;
		strQuery.Format(
			"SELECT AllResources FROM WaitingListItemT "
			"WHERE ID = %li", nLineItemID);
		_RecordsetPtr prs = CreateRecordsetStd(strQuery);
		if (!prs->eof) {
			bAllResources = (AdoFldBool(prs, "AllResources") > 0)?true:false;
		}

		if (bAllResources) {
			// All resources was selected for the item. So assume that they
			//  probably want to sechedule this for the active resource, not
			//  every resource.
			nResourceID = ((CSchedulerView*)pView)->GetActiveResourceID();
			if (nResourceID == CSchedulerView::sdrUnknownResource) {
				// (r.gonet 2015-08-26) - PLID 66954 - This was being allowed before and it would produce a FK exception. It is a rare case and frankly you 
				// have to be working hard to get here. You have to have a user permissioned to see the scheduler but not permissioned to see any of the 
				// current views' resources. It is arguable what should happen here and is out of this item's scope.
				MessageBox("The new appointment could not be created because the resource to use could not be determined.", "Error", MB_ICONERROR | MB_OK);
				return;
			}
			adwResources.Add(nResourceID);
		} else {
			// Use the selected resource ID values.
			strQuery.Format(
				"SELECT ResourceID FROM WaitingListItemResourceT "
				"WHERE ItemID = %li", nLineItemID);

			prs = CreateRecordsetStd(strQuery);
			if (prs->GetRecordCount() > 1) {
				bMultipleResources = true;
			}
			while (!prs->eof) {
				nResourceID = AdoFldLong(prs, "ResourceID", 0);
				if (nResourceID > 0) {
					adwResources.Add(nResourceID);
				}
				prs->MoveNext();
			}
		}


		// Get Purpose IDs.
		strQuery.Format(
			"SELECT PurposeID FROM WaitingListPurposeT "
			"WHERE WaitingListID = %li", nLineItemID);

		prs = CreateRecordsetStd(strQuery);

		long nPurposeID;
		while (!prs->eof) {
			nPurposeID = AdoFldLong(prs, "PurposeID", 0);
			if (nResourceID > 0) {
				adwPurposes.Add(nPurposeID);
			}
			prs->MoveNext();
		}
		
		// Format the end time for the appointment and check for double bookings permissions.
		COleDateTime dtEndTime = dtStartTime + COleDateTimeSpan(0, 0, 15, 0);
		COleDateTime dtBookingStart, dtBookingEnd;
		dtBookingStart.SetDateTime(
			dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay(), 
			dtStartTime.GetHour(), dtStartTime.GetMinute(), 0);
		dtBookingEnd.SetDateTime(
			dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay(), 
			dtEndTime.GetHour(), dtEndTime.GetMinute(), 0);
		if(!AppointmentValidateDoubleBooking(dtBookingStart, dtBookingEnd, adwResources)){
			return;
		}

		// Get the parent row.
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();

		long nPatientID = VarLong(pParentRow->GetValue(mucPatientID));
		long nTypeID = VarLong(pParentRow->GetValue(mucTypeID));
		CString strNotes = VarString(pParentRow->GetValue(mucNotes));

		// (j.jones 2014-11-14 14:31) - PLID 64169 - try to load the patient's insurance information
		AppointmentInsuranceMap mapInsPlacements;
		::TryAutoFillAppointmentInsurance(nPatientID, mapInsPlacements);

		nResID = AppointmentCreate(nPatientID, adwResources, GetCurrentLocationID(), 
			dtStartDate, dtStartTime, dtEndTime, dtStartTime, 0, 0, 0, strNotes, 
			nTypeID, adwPurposes, 0, -1, TRUE, true, &mapInsPlacements);

		::ClearAppointmentInsuranceMap(mapInsPlacements);
		
		// Try to open the dialog for editing the new appointment.
		if (nResID > -1) {
			// Flip to the resource view. We should always be in the scheduler module
			// at this point.
			if (pView) {
				if (bMultipleResources) {
					pView->SetActiveTab(SchedulerModule::MultiResourceTab);
				}
				// (r.gonet 2015-08-26) - PLID 66954 - Be sure to check if the resource is actually in the current view before trying to set it as the active resource.
				else if (nResourceID > 0 && ((CSchedulerView*)pView)->IsResourceInCurView(nResourceID)) {
					((CSchedulerView*)pView)->SetActiveResourceID(nResourceID, true);
				}
				((CSchedulerView*)pView)->UpdateView();
				// Go to the day and time, and open the resentry dialog.
				((CSchedulerView *)pView)->OpenAppointment(nResID, TRUE);
			}

			RemoveFromWaitList(pParentRow);
		}

	} NxCatchAll(FormatString("%s : Error trying to schedule a new appointment.", __FUNCTION__));
}

void CMoveUpListDlg::SetAppointmentColors()
{

	long nStart, nEnd;
	nStart = 0;
	nEnd = m_pMoveUpList->GetRowCount();
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->GetFirstRow();
	while(pRow != NULL) {
		// (d.moore 2007-05-10 13:23) - PLID 4013 - Change to NxDatalist2
		OLE_COLOR color = pRow->GetValue(mucColor).lVal;
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		color = GetDarkerColorForApptText(color);
		pRow->PutForeColor(color);
		pRow = pRow->GetNextRow();
	}

}


LRESULT CMoveUpListDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		switch (wParam) {
		case NetUtils::AppointmentsT:
			// (j.jones 2014-08-21 17:24) - PLID 63186 - there should never be a non-Ex
			// appointments message anymore
			break;
		}

	} NxCatchAll(__FUNCTION__);

	return 0;

}

// (j.jones 2014-08-21 17:19) - PLID 63186 - added an Ex handler
LRESULT CMoveUpListDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		switch (wParam) {
		case NetUtils::AppointmentsT:

			try {

				//if requerying is disabled, do nothing
				if (!m_bMoveUpRequeryEnabled) {
					return 0;
				}

				//if the list is querying, do nothing
				if (m_pMoveUpList->IsRequerying()) {
					return 0;
				}

				// (d.moore 2007-05-10 13:37) - PLID 4013 - 
				//  An item has been altered or added in the schedule. Check to see if the item
				//   is part of the waiting list and refresh the list if needed.
				// (j.jones 2014-08-21 17:23) - PLID 63186 - moved this from the regular tablechanged function, and converted to use Ex

				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nAppointmentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);

				// (j.jones 2014-08-21 17:26) - PLID 63186 - don't do anything unless we have an ID
				if (nAppointmentID == -1) {
					return 0;
				}

				// (j.jones 2014-08-22 08:44) - PLID 63186 - Most of the MoveUp list filtering is for details of
				// the move up request, not the appointment data, so we cannot exclude based on time or resources.
				// We can only ignore appointments earlier than our filter, and cancelled appointments.
				bool bAppointmentCouldBeInList = true;

				//check to see if the date is earlier than our filter range
				if (bAppointmentCouldBeInList) {
					COleDateTime dtStartTime = VarDateTime(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStartTime), g_cdtInvalid);

					//if today's date, this will be the current time
					COleDateTime dtFilterStartDate = GetCurStartDate();

					//We can only ignore appointments that are from before our date range.
					//This will ignore appointments with a start time in the past, if you're
					//filtering on today's date.
					if (dtStartTime < dtFilterStartDate) {
						bAppointmentCouldBeInList = false;
					}
				}

				//check to see if the appointment is cancelled
				if (bAppointmentCouldBeInList) {

					long nStatus = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStatus), -1);
					if (nStatus == 4) {
						//the appointment is cancelled
						bAppointmentCouldBeInList = false;
					}
				}				

				if (!bAppointmentCouldBeInList) {
					//if the appointment does not match our filter, but exists in the list, remove it now
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->FindByColumn(mucAppointmentID, nAppointmentID, NULL, FALSE);
					if (pRow != NULL) {
						RemoveFromWaitList(pRow);
					}

					//now return, because we do not need to run a recordset on this appointment
					return 0;
				}

				CString strWhereFilter = FormatQueryFilter();

				if (!strWhereFilter.IsEmpty()) {
					strWhereFilter = " AND " + strWhereFilter;
				}

				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT COUNT(WaitingListT.ID) AS NumForAppointment "
					"FROM WaitingListT "
					"INNER JOIN AppointmentsT ON WaitingListT.AppointmentID = AppointmentsT.ID "
					"LEFT JOIN AptTypeT ON WaitingListT.TypeID = AptTypeT.ID "
					"LEFT JOIN PersonT ON WaitingListT.PatientID = PersonT.ID "
					"WHERE WaitingListT.AppointmentID = {INT} {CONST_STRING}",
					nAppointmentID, strWhereFilter);

				if (!prs->eof) {
					long nNumEntriesForAppointment = AdoFldLong(prs, "NumForAppointment", 0);
					
					if (nNumEntriesForAppointment == 0) {
						//this appointment does not need to be in the list,
						//so remove it if it exists
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpList->FindByColumn(mucAppointmentID, nAppointmentID, NULL, FALSE);
						if (pRow != NULL) {
							RemoveFromWaitList(pRow);
						}
					}
					else {
						//this appointment should be in the list, reload so
						//it displays the tree properly
						Requery();
						return 0;
					}
				}

			}NxCatchAll("Error in CMoveUpListDlg::OnTableChangedEx:AppointmentsT");

			break;
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

void CMoveUpListDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	//pop up the right click menu for:
	//the current row
	//column 0 (unused)
	//point.x, point.y
	//no flags
	OnRButtonDownMoveuplist(m_pMoveUpList->GetCurSel(), 0, point.x, point.y, 0);

}

void CMoveUpListDlg::OnSelChosenApptTypeList(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//check to make sure the selection is valid'
		if (pRow == NULL) {
			pRow = m_pTypeList->SetSelByColumn(muatcID, (long) -1);
		}

		long nNewAptTypeID = VarLong(pRow->GetValue(muatcID));

		if (nNewAptTypeID == -1) {

			//set the purpose list to be {All Purposes}
			m_pPurposeList->SetSelByColumn(mupcID, (long) -1);
			//Reset the filter on the purposes.
			m_pPurposeList->WhereClause = _bstr_t("0=1");
			m_pPurposeList->Requery();

			//its the {All Types} so just use the regular where cluse
			Requery();

		}
		else {
		
			//load the purpose list
			// (c.haag 2008-12-18 12:55) - PLID 32376 - Filter out inactive procedures
			CString strPurposeWhere;
			strPurposeWhere.Format("AptPurposeT.ID IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li) "
				"AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ", nNewAptTypeID);
			m_pPurposeList->WhereClause = (LPCTSTR)(strPurposeWhere);
			m_pPurposeList->Requery();

			//now onto the list
			Requery();
		}

	}NxCatchAll("Error CMoveUpListDlg::OnSelChosenApptTypeList");
}

void CMoveUpListDlg::OnSelChosenApptPurpose(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		//check to make sure the the selection is valid 
		if(pRow == NULL) {
			pRow = m_pPurposeList->SetSelByColumn(mupcID, (long) -1);
		}

		//just requery the moveuplist
		Requery();
	}NxCatchAll("Error in CMoveUpListDlg::OnSelChosenApptPurpose");
	
}

void CMoveUpListDlg::OnSelChosenApptResource(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		long nNewResourceID = -1;
		if(pRow != NULL) {
			nNewResourceID = VarLong(pRow->GetValue(murcID), -1);
		}

		if(nNewResourceID > 0) {
			m_strResourceFilter.Format("%li", nNewResourceID);
		}
		else if(nNewResourceID == -2) {
			// Convert the list of resource id values over to a form that 
			//  can be used by the multi select dialog.
			CString strIDs = m_strResourceFilter;
			strIDs.Replace(",", "");
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "ResourceT");
			dlg.PreSelect(strIDs);
			dlg.m_strNameColTitle = "Resource";

			int res = dlg.Open("ResourceT", "", "ResourceT.ID", "ResourceT.Item", "Select Resources.");

			if(res == IDCANCEL)
				return;

			//save all our id's for later parsing
			CDWordArray tgResIDs;
			dlg.FillArrayWithIDs(tgResIDs);
			m_strResourceFilter = dlg.GetMultiSelectIDString(", ");

			if (tgResIDs.GetSize() > 1) {
				// Multiple items were selected from the dialog.
				// Get their ID values.
				
				// Get the names that match the ID values.
				CString strResList = dlg.GetMultiSelectString(", ");

				// Set the Hyperlink.
				m_nxlMultipleResourceLabel.SetText(strResList);
				m_nxlMultipleResourceLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_APPT_RESOURCE, SW_HIDE);
				ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_SHOW);
				InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
			} 
			else if (tgResIDs.GetSize() == 1) {
				// Only a single item was selected from the dialog.
				m_pResourceList->SetSelByColumn(murcID, (long)tgResIDs[0]);
				m_nxlMultipleResourceLabel.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_APPT_RESOURCE, SW_SHOW);
				ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_HIDE);
				InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
			}
		}
		else {
			// All resources selected, or bogus selection.
			m_strResourceFilter = "";
		}

		// (d.moore 2007-10-25) - PLID 4013 - Store the appointment type selected so that we can try to reset it later if needed.
		long nAptTypeSel = -1;
		pRow = m_pTypeList->CurSel;
		if (pRow != NULL) {
			nAptTypeSel = VarLong(pRow->GetValue(muatcID), 0);
		}
		
		// (d.moore 2007-10-25) - PLID 4013 - If resources are selected then filter the types based on 
		//  the resource selection.
		
		CString strWhere = "Inactive = 0";
		CString strResources;
		//DRT 5/17/2004 - PLID 12406 - This needs to be preferenced, otherwise it will hide types
		//	and the user may not have a clue why.
		if(GetRemotePropertyInt("HideEmptyTypes", 0, 0, "<None>", true)) {
			// (c.haag 2004-03-31 12:41) PLID 11690 - Assign the where clause for the type dropdown
			// to reflect only the available types to pull.
			if(!m_strResourceFilter.IsEmpty()) {
				strResources.Format(
					" AND ID IN (SELECT AptTypeId FROM ResourcePurposeTypeT WHERE ResourceId IN (%s))", m_strResourceFilter);
			}
		}
		strWhere += strResources;
		m_pTypeList->WhereClause = (_bstr_t)strWhere;
		// Now requery the apointment type list.
		m_pTypeList->Requery();
		m_pTypeList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		
		if (nAptTypeSel > 0) {
			m_pTypeList->SetSelByColumn(muatcID, nAptTypeSel);
			// Check to see if we were able to reset the value.
			nAptTypeSel = -1;
			pRow = m_pTypeList->CurSel;
			if (pRow != NULL) {
				nAptTypeSel = VarLong(pRow->GetValue(muatcID), 0);
			}
			if (nAptTypeSel <= 0) {
				// Reset to 'All Types'
				nAptTypeSel = VarLong(pRow->GetValue(muatcID), -1);
				m_pPurposeList->Clear();
				m_pPurposeList->Requery();
				MessageBox(
					"The selected appointment type does not match the resources you\n"
					"have chosen. The type and purpose dropdowns have been reset to\n"
					"search for all combinations.");
			}
		}
		
		// Requery the main list.
		Requery();
		
	}NxCatchAll("Error in CMoveUpListDlg::OnSelChosenApptResource");
}

void CMoveUpListDlg::OnRequeryFinishedApptTypeList(short nFlags) 
{
	try {
		//add the {All Types}
		NXDATALIST2Lib::IRowSettingsPtr pTypeRow;
		pTypeRow = m_pTypeList->GetNewRow();
		pTypeRow->PutValue(muatcID, (long)-1);
		pTypeRow->PutValue(muatcName, _variant_t("{All Types}"));
		m_pTypeList->AddRowBefore(pTypeRow, m_pTypeList->GetFirstRow());
		
		// (d.moore 2007-10-25) - PLID 4013 - Select 'All Types' as the default value.
		//  If the list has already had something set then use that value.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
		long nAptTypeSel = -1;
		if (pRow != NULL) {
			nAptTypeSel = VarLong(pRow->GetValue(muatcID), 0);
		}


		m_pTypeList->SetSelByColumn(muatcID, (long) nAptTypeSel);
	} NxCatchAll("Error In: CMoveUpListDlg::OnRequeryFinishedApptTypeList");
}

void CMoveUpListDlg::OnRequeryFinishedApptPurpose(short nFlags) 
{
	try {
		//add the {All Purposes}
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pPurposeList->GetNewRow();
		pRow->PutValue(mupcID, (long) -1);
		pRow->PutValue(mupcName, _variant_t("{All Purposes}"));
		m_pPurposeList->AddRowBefore(pRow, m_pPurposeList->GetFirstRow());
		// (d.moore 2007-10-25) - PLID 4013 - Select 'All Purposes' as the default value.
		m_pPurposeList->SetSelByColumn(mupcID, (long) -1);
	} NxCatchAll("Error In: CMoveUpListDlg::OnRequeryFinishedApptPurpose");
}

void CMoveUpListDlg::OnRequeryFinishedApptResource(short nFlags) 
{
	try {
		//add the {All Resources}
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pResourceList->GetNewRow();
		pRow->PutValue(murcID, (long) -2);
		pRow->PutValue(murcName, _variant_t("{Multiple Resources}"));
		m_pResourceList->AddRowBefore(pRow, m_pResourceList->GetFirstRow());
		pRow = m_pResourceList->GetNewRow();
		pRow->PutValue(murcID, (long) -1);
		pRow->PutValue(murcName, _variant_t("{All Resources}"));
		m_pResourceList->AddRowBefore(pRow, m_pResourceList->GetFirstRow());

		if(!m_strResourceFilter.IsEmpty()) {
			if (m_strResourceFilter.Find(",") > 0) {
				// Multiple selections.
				m_pResourceList->SetSelByColumn(murcID,(long)-2);
			} else {
				// Single selection.
				m_pResourceList->SetSelByColumn(murcID,(long)atoi(m_strResourceFilter));
			}
		} else {
			// All resources.
			m_pResourceList->SetSelByColumn(murcID,(long)-1);
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnRequeryFinishedApptResource");
}

void CMoveUpListDlg::SetMoveUpDateTime(const COleDateTime& dt)
{
	COleDateTime dtDateOnly, dtTimeOnly;
	dtDateOnly.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
	dtTimeOnly.SetTime(dt.GetHour(), dt.GetMinute(), dt.GetSecond());
	m_dtcMoveUp.SetValue(COleVariant(dtDateOnly));
	m_nxtMoveUp->SetDateTime(dtTimeOnly);
}

void CMoveUpListDlg::SetDateRangeFilter(COleDateTime dtStart, COleDateTime dtEnd)
{
	m_dtcStartDate.SetValue(COleVariant(dtStart));
	m_dtcEndDate.SetValue(COleVariant(dtEnd));
}

void CMoveUpListDlg::SetResourceFilter(const CDWordArray& adwResourceFilter)
{
	try {
		m_strResourceFilter = "";
		CString strID;
		for (long i = 0; i < adwResourceFilter.GetSize(); i++) {
			strID.Format("%li, ", adwResourceFilter[i]);
			m_strResourceFilter += strID;
		}
		m_strResourceFilter = m_strResourceFilter.Left(m_strResourceFilter.GetLength()-2);
		// (d.moore 2007-05-31 13:52) - PLID 4013 - 
		//  At one point a CDWordArray member variable was used to copy the values of
		//  adwResourceFilter and retain them for later use. However, when elements were
		//  added to the array at this point in the code a seemingly unrelated error would
		//  occur. Switching to saving the data in a CString resolved the issue. This in fact 
		//  was the way the function originally worked anyway.


		if (adwResourceFilter.GetSize() > 1) {
			// Multiple items were selected form the dialog.
			// Get the names that match the ID values.
			CString strResList;
			_RecordsetPtr prs = CreateRecordset(
				"SELECT Item FROM ResourceT WHERE ID IN (%s)", m_strResourceFilter);
			while(!prs->eof) {
				strResList += AdoFldString(prs, "Item") + ", ";
				prs->MoveNext();
			}
			strResList = strResList.Left(strResList.GetLength()-2);

			// Set the Hyperlink.
			m_nxlMultipleResourceLabel.SetText(strResList);
			m_nxlMultipleResourceLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_APPT_RESOURCE, SW_HIDE);
			ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_SHOW);
			InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
		} 
		else if (adwResourceFilter.GetSize() == 1) {
			// Only a single item was selected from the dialog.
			m_pResourceList->SetSelByColumn(murcID, (long)adwResourceFilter[0]);
			m_nxlMultipleResourceLabel.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_APPT_RESOURCE, SW_SHOW);
			ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_HIDE);
			InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
		} else {
			// All Resources
			m_pResourceList->SetSelByColumn(murcID, (long)-1);
		}
	} NxCatchAll("Error in CMoveUpListDlg::SetResourceFilter");
}

// (c.haag 2003-09-26 12:54) - We may want to wait to set the resource filter
// before querying the data.
void CMoveUpListDlg::EnableRequery(BOOL bEnable)
{
	m_bMoveUpRequeryEnabled = bEnable;
}

LRESULT CMoveUpListDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

void CMoveUpListDlg::OnPreviewMoveupList() 
{

	try {
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(601)]);

		// (a.walling 2008-05-13 15:15) - PLID 27591 - Don't need VarDateTime anymore
		COleDateTime dtStartDate = GetCurStartDate();
		COleDateTime dtEndDate = GetCurEndDate();
		
		infReport.DateFrom = dtStartDate;
		infReport.DateTo = dtEndDate;
		
		infReport.nDateRange = 1;
		infReport.nDateFilter = 1;
		
		CPtrArray paramList;

		CRParameterInfo piDateFrom;
		piDateFrom.m_Data = FormatDateTimeForInterface(dtStartDate);
		piDateFrom.m_Name = "DateFrom";
		paramList.Add (&piDateFrom);

		CRParameterInfo piDateTo;
		piDateTo.m_Data = FormatDateTimeForInterface(dtEndDate);
		piDateTo.m_Name = "DateTo";
		paramList.Add (&piDateTo);


		CString strWhereFilter;
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		// Appointment type filter.
		if (m_pTypeList->CurSel != NULL) {
			CString strApptTypeFilter;
			pRow = m_pTypeList->CurSel;
			
			long nType = VarLong(pRow->GetValue(muatcID), 0);
			if (nType > 0) {
				strApptTypeFilter.Format("WaitingListT.TypeID = %li ", nType);
				strWhereFilter += strApptTypeFilter;
			}
		}
		
		// Appointment purpose filter.
		if (m_pPurposeList->CurSel != NULL) {
			CString strPurposeFilter;
			pRow = m_pPurposeList->CurSel;

			long nPurpose = VarLong(pRow->GetValue(mupcID), 0);
			if (nPurpose > 0) {
				strPurposeFilter.Format(
					"WaitingListT.ID IN "
						"(SELECT DISTINCT WaitingListID "
						"FROM WaitingListPurposeT "
						"WHERE PurposeID = %li) ", nPurpose);
				if (!strWhereFilter.IsEmpty()) {
					strWhereFilter += " AND ";
				}
				strWhereFilter += strPurposeFilter;
			}
		}

		// Appointment resource filter.
		if (!m_strResourceFilter.IsEmpty()) {
			
			CString strResourceFilter;
			strResourceFilter.Format(
				"(WaitingListItemResourceT.ResourceID IN (%s) "
				"OR WaitingListItemT.AllResources = 1)", m_strResourceFilter);
			if (!strWhereFilter.IsEmpty()) {
				strWhereFilter += " AND ";
			}
			strWhereFilter += strResourceFilter;
		}

		if (!strWhereFilter.IsEmpty()) {
			infReport.strFilterString = strWhereFilter;
		}

		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, &paramList, true, this, "Move Up List", NULL, TRUE);

		//close the dialog
		OnOK();
	}NxCatchAll("Error running report");
		
}

void CMoveUpListDlg::OnAddWaitListPatient() 
{
	try {
		// (d.moore 2007-05-10 13:53) - PLID 4013 - 
		//  This function opens up the dialog for adding a patient to the waiting list
		//  without having to create an appointment.
		CWaitingListEntryDlg dlg(this);
		if (dlg.DoModal() == IDOK) {
			Requery();
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnAddWaitListPatient");
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CMoveUpListDlg::OnChangeDateStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// (d.moore 2007-05-10 13:53) - PLID 4013 - Refill the waiting list NxDataList control.
		// (a.walling 2008-05-13 15:15) - PLID 27591 - Don't need VarDateTime anymore
		COleDateTime dtStartDate = GetCurStartDate();
		COleDateTime dtEndDate = GetCurEndDate();
		if (dtStartDate > dtEndDate) {
			MessageBox("The start date selected is later than the end date.");
		}
		Requery();
	} NxCatchAll("Error In: CMoveUpListDlg::OnChangeDateStart");

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CMoveUpListDlg::OnChangeDateEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// (d.moore 2007-05-10 13:53) - PLID 4013 - Refill the waiting list NxDataList control.
		// (a.walling 2008-05-13 15:15) - PLID 27591 - Don't need VarDateTime anymore
		COleDateTime dtStartDate = GetCurStartDate();
		COleDateTime dtEndDate = GetCurEndDate();
		if (dtStartDate > dtEndDate) {
			MessageBox("The end date selected is earlier than the start date.");
		}
		Requery();
	} NxCatchAll("Error In: CMoveUpListDlg::OnChangeDateEnd");
	
	*pResult = 0;
}

void CMoveUpListDlg::OnSelChangedMoveuplist(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		// (d.moore 2007-05-10 13:53) - PLID 4013 - Now used NxDataList2.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		if (pRow == NULL) {
			// No row selected, so no appointment selected, so deactivate the button.
			GetDlgItem(IDC_GOTOAPPOINTMENT)->EnableWindow(FALSE);
			return;
		}
		
		long nApptID = VarLong(pRow->GetValue(mucAppointmentID), 0);
		long nLineItemID = VarLong(pRow->GetValue(mucLineItemID), 0);
		if (nApptID > 0) {
			GetDlgItem(IDC_GOTOAPPOINTMENT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_MOVEUP)->EnableWindow(FALSE);
		} else if (nLineItemID > 0) {
			GetDlgItem(IDC_BTN_MOVEUP)->EnableWindow(TRUE);
			GetDlgItem(IDC_GOTOAPPOINTMENT)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_GOTOAPPOINTMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_MOVEUP)->EnableWindow(FALSE);
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnSelChangedMoveuplist");
}


void CMoveUpListDlg::Requery()
{
	// (d.moore 2007-05-11 11:24) - PLID 4013 - 
	//  This function just fills the Waiting List NxDataList control and sets the appropriate
	//  colors for each row in the list.
	if (!m_bMoveUpRequeryEnabled) {
		return;
	}

	// Create filters for displaying Waiting List items.
	CString strWhereFilter = FormatQueryFilter();
	if (!strWhereFilter.IsEmpty()) {
		strWhereFilter = " AND " + strWhereFilter;
	}

	// Put the query together.

	//DRT 8/29/2008 - PLID 29240 - Remove appointments which are In or Out status from the available list.
	// (c.haag 2009-08-07 10:20) - PLID 11699 - Added CreatedDate
	// (s.tullis 2013-10-23 17:38) - PLID 43917 - Waitlist to show preferred form of contact instead of home phone.
	//(e.lally 2009-08-13) PLID 28591 - Added Appoinment Start datetime
	CString strFrom;
	strFrom.Format(
		"(SELECT "
			"WaitingListT.ID AS WaitListID, WaitingListT.PatientID, -1 AS LineItemID, "
			"WaitingListT.AppointmentID, WaitingListT.Notes, AppointmentsT.CreatedDate AS ApptCreatedDate, "
			"AppointmentsT.StartTime AS ApptStart, "
			"Convert(nvarchar(10), WaitingListT.CreatedDate, 101) AS CreatedDate, "
			"dbo.GetWaitListPurposeString(WaitingListT.ID) AS Purpose, "
			"CASE WHEN WaitingListT.AppointmentID IS NOT NULL THEN 'Yes' ELSE 'No' END AS ApptFlag, "
			"AptTypeT.NAME AS ApptType, AptTypeT.COLOR, WaitingListT.TypeID, "
			"CASE WHEN (LEN(RTRIM(PersonT.[Last])) > 0 AND LEN(RTRIM(PersonT.[First])) > 0) "
			"THEN PersonT.[Last] + ', ' + PersonT.[First] "
			"ELSE '' END AS PersonName, "
			"CASE WHEN PatientsT.PreferredContact = 1 THEN PersonT.HomePhone ELSE "
 			"CASE WHEN PatientsT.PreferredContact = 2 THEN PersonT.WorkPhone ELSE "
  			"CASE WHEN PatientsT.PreferredContact = 3 THEN PersonT.CellPhone ELSE "
			"CASE WHEN PatientsT.PreferredContact = 4 THEN PersonT.Pager ELSE "
  			"CASE WHEN PatientsT.PreferredContact = 5 THEN PersonT.OtherPhone ELSE "
  			"CASE WHEN PatientsT.PreferredContact = 7 THEN PersonT.CellPhone ELSE "
 			"PersonT.HomePhone END END END END END END AS HomePhone "
		"FROM WaitingListT "
		"LEFT JOIN PatientsT "
		"ON PatientsT.PersonID = WaitingListT.PatientID "
		"LEFT JOIN AptTypeT "
		"ON WaitingListT.TypeID = AptTypeT.ID "
		"LEFT JOIN PersonT "
		"ON WaitingListT.PatientID = PersonT.ID "
		"LEFT JOIN AppointmentsT "
		"ON WaitingListT.AppointmentID = AppointmentsT.ID "
		"WHERE (AppointmentsT.ID IS NULL OR AppointmentsT.ShowState NOT IN (1, 2)) %s) AS WaitListDataQ", 
		strWhereFilter);
	
	
	m_pMoveUpList->FromClause = (_bstr_t)strFrom;

	//(e.lally 2009-08-14) PLID 35226 - set the sort priority for our currently selected options
	RefreshSortPriority();

	m_pMoveUpList->Requery();
}

void CMoveUpListDlg::RemoveFromWaitList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// This function removes the row from the m_pMoveUpList and then removes
	//  all associated records from the database.
	
	if (pRow == NULL) {
		return;
	}

	try {
		long nWaitListID = VarLong(pRow->GetValue(mucWaitListID), 0);
		long nLineItemID = VarLong(pRow->GetValue(mucLineItemID), 0);

		// Make sure that we have the parent row.
		if (nLineItemID > 0) {
			pRow = pRow->GetParentRow();
		}
		
		// Determine if we are removing an appointment or a request.
		CString strAuditAction;
		long nAppointmentID = VarLong(pRow->GetValue(mucAppointmentID), -1);
		if (nAppointmentID > 0) {
			strAuditAction = "Remove Appointment Move Up";
		} else {
			strAuditAction = "Remove Wait List Request";
		}

		m_pMoveUpList->RemoveRow(pRow);

		
		if (nWaitListID <= 0) {
			// The ID is invalid, can't do anything else.
			return;
		}

		// Resource query.
		CString strQuery;
		strQuery.Format(
			"DECLARE @Wait_List_ID INT; \r\n"
			"SET @Wait_List_ID = %li; \r\n", 
			nWaitListID);

		strQuery +=
			// Delete Resources Query.
			"DELETE FROM WaitingListItemResourceT "
			"WHERE ItemID IN "
				"(SELECT ID FROM WaitingListItemT "
				"WHERE WaitingListID = @Wait_List_ID); "
			"\r\n"
			// Delete Days Query.
			"DELETE FROM WaitingListItemDaysT "
			"WHERE ItemID IN "
				"(SELECT ID FROM WaitingListItemT "
				"WHERE WaitingListID = @Wait_List_ID); "
			"\r\n"
			// Delete Line Items Query.
			"DELETE FROM WaitingListItemT "
			"WHERE WaitingListID = @Wait_List_ID; "
			"\r\n"
			// Delete Purpose Query.
			"DELETE FROM WaitingListPurposeT "
			"WHERE WaitingListID = @Wait_List_ID; "
			"\r\n"
			// Delete Waiting List Entry Query.
			"DELETE FROM WaitingListT WHERE ID = @Wait_List_ID;"
			"\r\n";
			
		//Update the modified date for the appointment (if any)
		if (nAppointmentID > 0) {
			CString strApptUpdate;
			strApptUpdate.Format(
				"UPDATE AppointmentsT "
					"SET ModifiedDate = GetDate(), "
					"ModifiedLogin = '%s' "
				"WHERE AppointmentsT.ID IN "
					"(SELECT AppointmentID "
						"FROM WaitingListT "
						"WHERE ID = %li)", 
				_Q(GetCurrentUserName()),
				nWaitListID);
			strQuery += strApptUpdate;
		}
		
		//(e.lally 2009-08-14) This should be using the SqlStd version
		ExecuteSqlStd(strQuery);

		// Do auditing.
		CString strPatientName = VarString(pRow->GetValue(mucName), "");
		CString strPhone = VarString(pRow->GetValue(mucPhone), "");
		strPatientName.Replace(" " + strPhone, "");

		long nPatientID = VarLong(pRow->GetValue(mucPatientID), -1);
		
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptMoveUp, nWaitListID, "", strAuditAction, aepMedium);
		
		
	} NxCatchAll("Error attempting to remove an item from the waiting list.");
}


void CMoveUpListDlg::QueryLineItemCollection()
{	
	
	// (d.moore 2007-05-21 10:55) - PLID 4013 - 
	// This function queries the database for every resource line 
	//  item associated with a list of waiting list entry.
	
	try {
		// Format a query that gets just the ID values for all applicable 
		//  Waiting list entries.
		CString strWhere, strQuery;
		CString strFilter = FormatQueryFilter();
		
		if (!strFilter.IsEmpty()) {
			strWhere.Format(
				"(SELECT WaitingListT.ID "
				"FROM WaitingListT "
				"LEFT JOIN AptTypeT "
				"ON WaitingListT.TypeID = AptTypeT.ID "
				"LEFT JOIN PersonT "
				"ON WaitingListT.PatientID = PersonT.ID "
				"LEFT JOIN AppointmentsT "
				"ON WaitingListT.AppointmentID = AppointmentsT.ID "
				"WHERE %s)", strFilter);
		}
		
		// Get data for every line item.
		strQuery = 
			"SELECT ID, StartDate, EndDate, StartTime, EndTime, AllResources, WaitingListID "
			"FROM WaitingListItemT";
		
		if (!strWhere.IsEmpty()) {
			strQuery += " WHERE WaitingListID IN " + strWhere;
		}

		_RecordsetPtr prs = CreateRecordsetStd(strQuery);

		long nCount = prs->GetRecordCount();
		if (nCount < 1) {
			// Nothing was returned, so exit the function without doing anything else.
			return;
		}
		
		WaitListLineItem *rgLineItems = new WaitListLineItem[nCount];
		FieldsPtr flds = prs->Fields;
		long nIndex = 0;
		while (!prs->eof) {
			rgLineItems[nIndex].nLineItemID = AdoFldLong(flds, "ID");
			rgLineItems[nIndex].bAllResources = (AdoFldBool(flds, "AllResources") > 0)?true:false;
			rgLineItems[nIndex].dtStartDate = AdoFldDateTime(flds, "StartDate");
			rgLineItems[nIndex].dtEndDate = AdoFldDateTime(flds, "EndDate");
			rgLineItems[nIndex].dtStartTime = AdoFldDateTime(flds, "StartTime");
			rgLineItems[nIndex].dtEndTime = AdoFldDateTime(flds, "EndTime");
			rgLineItems[nIndex].nWaitListID = AdoFldLong(flds, "WaitingListID");
			
			nIndex++;
			prs->MoveNext();
		}
		
		// Get data for resources associated with line items.
		strQuery = 
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemResourceT.ResourceID AS ResourceID, "
				"ResourceT.Item AS Name "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemResourceT "
				"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
				"INNER JOIN ResourceT "
				"ON WaitingListItemResourceT.ResourceID = ResourceT.ID";
		
		if (!strWhere.IsEmpty()) {
			strQuery += " WHERE WaitingListT.ID IN " + strWhere;
		}


		prs = CreateRecordsetStd(strQuery);

		long nLineItemID;
		if (!prs->eof) {
			flds = prs->Fields;
		}
		while (!prs->eof) {
			// First try to find a matching entry in the rgLineItems array.
			//  There should only ever be a few entries, so just loop and look.
			nLineItemID = AdoFldLong(flds, "LineItemID", 0);
			for (long i = 0; i < nCount; i++) {
				if (rgLineItems[i].nLineItemID == nLineItemID)
				{
					rgLineItems[i].arResourceNames.Add(AdoFldString(flds, "Name", ""));
					break;
				}
			}
			prs->MoveNext();
		}

		// Get all of the days associated with each line item.
		CString strDayWhere;
		if (!strWhere.IsEmpty()) {
			strDayWhere = " WHERE WaitingListT.ID IN " + strWhere;
		}

		strQuery.Format(
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemDaysT.DayOfWeekNum AS DayID "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemDaysT "
				"ON WaitingListItemT.ID = WaitingListItemDaysT.ItemID "
			"%s "
			"ORDER BY WaitingListItemDaysT.DayOfWeekNum", strDayWhere);
		
		prs = CreateRecordsetStd(strQuery);


		CString strDayName;
		if (!prs->eof) {
			flds = prs->Fields;
		}
		while (!prs->eof) {
			// Again, try to find a match for the line item in the array.
			nLineItemID = AdoFldLong(flds, "LineItemID", 0);
			for (long i = 0; i < nCount; i++) {
				if (rgLineItems[i].nLineItemID == nLineItemID)
				{
					rgLineItems[i].arDayNames.Add(CWaitingListUtils::GetDayName(AdoFldLong(flds, "DayID", 0)));
					break;
				}
			}
			prs->MoveNext();
		}
		
		// Search for items in the MoveUp list that match the line items in the collection.
		NXDATALIST2Lib::IRowSettingsPtr pRow, pNewRow;
		for (long i = 0; i < nCount; i++) {
			
			pRow = m_pMoveUpList->FindByColumn(mucWaitListID, rgLineItems[i].nWaitListID, NULL, FALSE);
			if (pRow != NULL) {
				pNewRow = m_pMoveUpList->GetNewRow();
				pNewRow->PutValue(mucWaitListID, rgLineItems[i].nWaitListID);
				pNewRow->PutValue(mucAppointmentID, (long)-1);
				pNewRow->PutValue(mucLineItemID, rgLineItems[i].nLineItemID);
				pNewRow->PutValue(mucName, (_variant_t)CWaitingListUtils::FormatLineItem(rgLineItems[i]));
				m_pMoveUpList->AddRowAtEnd(pNewRow, pRow);
				pRow->PutExpanded(TRUE);
			}
		}

		// (d.moore 2007-08-31) - PLID 4013 - Clean up the temporary array used to store line item data.
		delete [] rgLineItems;

	} NxCatchAll("Error in CMoveUpListDlg::QueryLineItemCollection");

	return;
}

// (j.jones 2014-08-22 10:53) - PLID 63186 - if the start date filter is today's date,
// it will always filter as of the current time, not the date
COleDateTime CMoveUpListDlg::GetCurStartDate()
{
	COleDateTime dtStartDate = m_dtcStartDate.GetValue();
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	if (AsDateNoTime(dtStartDate) == AsDateNoTime(dtNow)) {
		//the start date is today, so change it to be the current time
		dtStartDate = dtNow;
	}
	else {
		//the start date is not today, so remove the time
		dtStartDate = AsDateNoTime(dtStartDate);
	}
	return dtStartDate;
}

// (j.jones 2014-08-22 10:53) - PLID 63186 - if the end date filter is today's date,
// it will always filter through the end of the day
COleDateTime CMoveUpListDlg::GetCurEndDate()
{
	COleDateTime dtEndDate = m_dtcEndDate.GetValue();
	//force the end date to be at 11:59:59pm
	dtEndDate.SetDateTime(dtEndDate.GetYear(), dtEndDate.GetMonth(), dtEndDate.GetDay(), 23, 59, 59);
	return dtEndDate;
}

CString CMoveUpListDlg::FormatQueryFilter()
{
	CString strWhereFilter; // Used to filter on type and purpose.

	// Get the start and end dates for the filter and format them
	//  for the SQL query.
	// (a.walling 2008-05-13 15:15) - PLID 27591 - Don't need VarDateTime anymore
	COleDateTime dtStartDate = GetCurStartDate();
	COleDateTime dtEndDate = GetCurEndDate();
	if (dtStartDate > dtEndDate) {
		dtEndDate = dtStartDate;
		// (j.jones 2014-08-22 08:34) - PLID 63186 - update the date in the interface
		m_dtcEndDate.SetValue(COleVariant(dtEndDate));
	}

	// (c.haag 2008-10-31 09:14) - PLID 31856 - Use international-compliant formatting	
	// (j.jones 2014-08-22 11:12) - PLID 63186 - ignore appointments that have
	// a start time earlier than our filter start time
	CString strTimeFilter;
	strTimeFilter.Format(
		"WaitingListT.ID IN "
		"(SELECT DISTINCT(WaitingListID) "
		"FROM WaitingListItemT "
		"WHERE StartDate <= '%s' AND EndDate >= '%s') "
		"AND "
		"(AppointmentsT.ID Is Null OR AppointmentsT.StartTime >= '%s') ", 
		FormatDateTimeForSql(dtEndDate, dtoDate), FormatDateTimeForSql(dtStartDate, dtoDate),	//these only need dtoDate
		FormatDateTimeForSql(dtStartDate, dtoDateTime));	//this needs dtoDateTime

	if (!strWhereFilter.IsEmpty()) {
		strWhereFilter += " AND ";
	}
	strWhereFilter += strTimeFilter;

	NXDATALIST2Lib::IRowSettingsPtr pRow;

	// Appointment type filter.
	if (m_pTypeList->CurSel != NULL) {
		CString strApptTypeFilter;
		pRow = m_pTypeList->CurSel;
		
		long nType = VarLong(pRow->GetValue(muatcID), 0);
		if (nType > 0) {
			strApptTypeFilter.Format("WaitingListT.TypeID = %li ", nType);
		
			if (!strWhereFilter.IsEmpty()) {
				strWhereFilter += " AND ";
			}
			strWhereFilter += strApptTypeFilter;
		}
	}
	
	// Appointment purpose filter.
	if (m_pPurposeList->CurSel != NULL) {
		CString strPurposeFilter;
		pRow = m_pPurposeList->CurSel;

		long nPurpose = VarLong(pRow->GetValue(mupcID), 0);
		if (nPurpose > 0) {
			strPurposeFilter.Format(
				"WaitingListT.ID IN "
					"(SELECT WaitingListID "
					"FROM WaitingListPurposeT "
					"WHERE PurposeID = %li) ", nPurpose);
		
			if (!strWhereFilter.IsEmpty()) {
				strWhereFilter += " AND ";
			}
			strWhereFilter += strPurposeFilter;
		}
	}

	// Appointment resource filter.
	if (!m_strResourceFilter.IsEmpty()) {
		if (!strWhereFilter.IsEmpty()) {
			strWhereFilter += " AND ";
		}
		
		CString strResourceFilter;
		strResourceFilter.Format(
			"WaitingListT.ID IN "
				"(SELECT WaitingListItemT.WaitingListID "
				"FROM WaitingListItemT LEFT JOIN WaitingListItemResourceT "
				"ON WaitingListItemResourceT.ItemID = WaitingListItemT.ID "
				"WHERE WaitingListItemResourceT.ResourceID IN (%s) "
					"OR WaitingListItemT.AllResources = 1)", m_strResourceFilter);
		
		strWhereFilter += strResourceFilter;
	}

	// (d.moore 2007-10-18) - PLID 26546 - Added a filter to prevent canceled appointments from being selected.
	if (!strWhereFilter.IsEmpty()) {
		strWhereFilter += " AND ";
	}
	strWhereFilter += "(AppointmentsT.Status IS NULL OR AppointmentsT.Status <> 4)";

	return strWhereFilter;
}

LRESULT CMoveUpListDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MULTIPLE_RESOURCE_LABEL:
			OpenResourceMultiList();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnLabelClick");
	return 0;
}

BOOL CMoveUpListDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		if (GetDlgItem(IDC_MULTIPLE_RESOURCE_LABEL)->IsWindowVisible()) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			
			CRect rc;
			GetDlgItem(IDC_MULTIPLE_RESOURCE_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	} NxCatchAll("Error In: CMoveUpListDlg::OnSetCursor");
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CMoveUpListDlg::OpenResourceMultiList()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ResourceT");

		CString strIDs = m_strResourceFilter;
		strIDs.Replace(",", "");
		dlg.PreSelect(strIDs);

		dlg.m_strNameColTitle = "Resource";
		int nResult = dlg.Open(
			"ResourceT", 
			"", 
			"ResourceT.ID", 
			"ResourceT.Item", 
			"Select Resources.");
		
		if (nResult == IDOK) {
			m_strResourceFilter = dlg.GetMultiSelectIDString(", ");
			CDWordArray rgResIDs;
			dlg.FillArrayWithIDs(rgResIDs);
			
			
			// Display the selected items in a hyperlink.
			if (rgResIDs.GetSize() > 1) {
				// Multiple items were selected form the dialog.
				
				// Get the names that match the ID values.
				CString strResList = dlg.GetMultiSelectString(", ");

				m_nxlMultipleResourceLabel.SetText(strResList);
				m_nxlMultipleResourceLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_APPT_RESOURCE, SW_HIDE);
				ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_SHOW);
				InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
			} 
			else if (rgResIDs.GetSize() == 1) {
				// Only a single item was selected from the dialog.
				m_pResourceList->SetSelByColumn(murcID, (long)rgResIDs[0]);
				m_nxlMultipleResourceLabel.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_APPT_RESOURCE, SW_SHOW);
				ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_HIDE);
				InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
			} else {
				// No selection made.
				m_pResourceList->SetSelByColumn(murcID, (long)-1);
				m_nxlMultipleResourceLabel.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_APPT_RESOURCE, SW_SHOW);
				ShowDlgItem(IDC_MULTIPLE_RESOURCE_LABEL, SW_HIDE);
				InvalidateDlgItem(IDC_MULTIPLE_RESOURCE_LABEL);
			}
		}
	} NxCatchAll("Error in CMoveUpListDlg::OpenResourceMultiList");
}

// (c.haag 2009-08-07 10:07) - PLID 11699 - Called when the user checks the Show Input Date box
void CMoveUpListDlg::OnCheckShowInputDate()
{
	try {
		SetRemotePropertyInt("MoveUpIncludeInputDate", IsDlgButtonChecked(IDC_CHECK_MOVEUP_SHOWINPUTDATE) ? 1 : 0, 0, GetCurrentUserName());
		EnsureAllVisibleColumnWidths();
	}
	NxCatchAll("Error in CMoveUpListDlg::OnCheckShowInputDate");
}
//(e.lally 2009-08-13) PLID 28591 - Called when the user checks the Show Appointment Date box
void CMoveUpListDlg::OnCheckShowApptStart()
{
	try {
		SetRemotePropertyInt("MoveUpIncludeApptStartDateTime", IsDlgButtonChecked(IDC_CHECK_MOVEUP_SHOW_APPT_START) ? 1 : 0, 0, GetCurrentUserName());
		EnsureAllVisibleColumnWidths();
	}
	NxCatchAll("Error in CMoveUpListDlg::OnCheckShowApptStart");
}

//(e.lally 2009-08-14) PLID 35226 - Manually build the list of columns available to sort by since the datalist does not inherently
//	support the sorting of the tree style.
void CMoveUpListDlg::LoadSortOptions()
{
	try{
		for(int i=0, nCols = m_pMoveUpList->GetColumnCount(); i<nCols; i++){
			switch(i){
				case mucWaitListID:
				case mucAppointmentID:
				case mucLineItemID:
				case mucPatientID:
				case mucTypeID:
					//Now allow sort by phone since it is not it's own visible column.
				case mucPhone:
				case mucColor:
				case mucNotes:
					//Do not allow sorting by these columns
					break;
				default: {
					//Add column as a sort option
					IRowSettingsPtr pSortRow = m_pMoveUpListSort->GetNewRow();
					IColumnSettingsPtr pMoveUpCol = m_pMoveUpList->GetColumn(i);
					if(pSortRow != NULL && pMoveUpCol != NULL){
						pSortRow->PutValue(scColumn, (long) i);
						pSortRow->PutValue(scField, _variant_t(pMoveUpCol->ColumnTitle));
						m_pMoveUpListSort->AddRowAtEnd(pSortRow, NULL);
					}
					else{
						ASSERT(FALSE);
					}
					break;
				}
			}
		}

		m_pMoveUpListSort->Sort();

		//Set the cur sel to the first row so *something* is selected
		m_pMoveUpListSort->CurSel = m_pMoveUpListSort->GetFirstRow();
		//Get the column this user last sorted by - default to the patient name column
		long nLastSort = GetRemotePropertyInt("MoveUpLastSortColumn", mucName, 0, GetCurrentUserName());
		m_pMoveUpListSort->SetSelByColumn(0, nLastSort);

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-08-14) PLID 35226 - Used to gather the selected sorting options since the datalist does not inherently
//	support the sorting of the tree style.
void CMoveUpListDlg::RefreshSortPriority()
{
	int nSortCol = 0;
	IRowSettingsPtr pSortRow = m_pMoveUpListSort->GetCurSel();
	if(pSortRow != NULL){
		nSortCol = VarLong(pSortRow->GetValue(scColumn));
	}
	else{
		ASSERT(FALSE);
	}

	IColumnSettingsPtr pCol = m_pMoveUpList->GetColumn(nSortCol);
	if(pCol != NULL){
		pCol->PutSortPriority(0);
		VARIANT_BOOL varbAscending = VARIANT_TRUE;
		if(IsDlgButtonChecked(IDC_MOVEUP_SORT_DESC)){
			varbAscending = VARIANT_FALSE;
		}
		pCol->PutSortAscending(varbAscending);
	}
	else{
		ASSERT(FALSE);
	}
}

//(e.lally 2009-08-14) PLID 35226 - Used to set the ascending sorting option since the datalist does not inherently
//	support the sorting of the tree style.
void CMoveUpListDlg::OnRadioSortMoveUpListAscending()
{
	try {
		SetRemotePropertyInt("MoveUpLastSortAscending", TRUE, 0, GetCurrentUserName());
		RefreshSortPriority();
		//(e.lally 2009-08-14) PLID 35226 - Unfortunately, we have to requery here because the datalist does not
		//	support the sort method for the tree style.
		m_pMoveUpList->Requery();
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-08-14) PLID 35226 - Used to set the descending sorting option since the datalist does not inherently
//	support the sorting of the tree style.
void CMoveUpListDlg::OnRadioSortMoveUpListDescending()
{
	try {
		SetRemotePropertyInt("MoveUpLastSortAscending", FALSE, 0, GetCurrentUserName());
		RefreshSortPriority();
		//(e.lally 2009-08-14) PLID 35226 - Unfortunately, we have to requery here because the datalist does not
		//	support the sort method for the tree style.
		m_pMoveUpList->Requery();
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-08-14) PLID 35226 - Used to set the column sorting option since the datalist does not inherently
//	support the sorting of tree structures.
void CMoveUpListDlg::OnSelChangedMoveupListSort(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//Try to stop a non-row from being selected
		if(lpNewSel == NULL){
			lpNewSel = lpOldSel;
			m_pMoveUpListSort->CurSel = IRowSettingsPtr(lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-08-14) PLID 35226 - Used to set the column sorting option since the datalist does not inherently
//	support the sorting of tree structures.
void CMoveUpListDlg::OnSelChosenMoveupListSort(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			long nCol = VarLong(pRow->GetValue(scColumn));
			SetRemotePropertyInt("MoveUpLastSortColumn", nCol, 0, GetCurrentUserName());
			RefreshSortPriority();
			//(e.lally 2009-08-14) PLID 35226 - Unfortunately, we have to requery here because the datalist does not
			//	support the sort method for the tree style.
			m_pMoveUpList->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}