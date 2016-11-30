// ToDoAlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ToDoAlarmDlg.h"
#include "TaskEditDlg.h"
#include "GlobalDataUtils.h"
#include "InvOrderDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalReportUtils.h"
#include "AuditTrail.h"
#include "ExportDlg.h"
#include "ContactView.h" // (k.messina 2010-04-12 11:15) - PLID 37957
#include "LabEntryDlg.h"			//(e.lally 2010-05-06) PLID 36567

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code
// (j.armen 2012-05-30 11:50) - PLID 49854 - Removed OnPaint handler for drag handler - now handled by CNexTechDialog

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



using namespace ADODB;
using namespace NXDATALISTLib;

extern CPracticeApp theApp;

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_MODIFY			43041
#define ID_MARKDONE			43042
#define	ID_SNOOZE_1_HOUR	43043
#define	ID_SNOOZE_1_DAY		43044
#define	ID_SNOOZE_1_WEEK	43045

// (j.fouts 2012-04-30 11:50) - PLID 49857 - Make the default larger
#define TODODLG_DEFAULT_WIDTH 958
#define TODODLG_DEFAULT_HEIGHT 600

#define TODODLG_GRIPWIDTH	15
#define TODODLG_GRIPHEIGHT	15

#define TITLE_BASE_STRING		"To Do Alarm"

enum TODO_COLUMN_TYPE
{
	TaskID = 0,
	PersonID,
	Name,
	PreferredContact,	// (j.fouts 2012-04-30 11:06) - PLID 49859 - Consolidated Phone and Email into a PreferredContact
	HomePhone,			// (j.fouts 2012-06-05 11:42) - PLID  49859 - Added Back in
	PreferredPhone,		// (j.fouts 2012-06-05 11:42) - PLID  49859 - Added Back in
	State,
	Email,				// (j.fouts 2012-06-05 11:42) - PLID  49859 - Added Back in
	Notes,
	CategoryID,
//	Category,		//DRT 7/22/03 - Removed - CategoryID is now a combo that lets you edit the field in place.
	Task,
	Priority,
	Deadline,
	Remind,
	Completed,
	EnteredBy,
	RegardingID,
	RegardingType,
	BackColor,
	AssignedIDs   // (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support
}	TODO_COLUMN_TYPE;

enum CATEGORY_FILTER_COLUMNS
{
	cfcID = 0,
	cfcDescription = 1,
};

const long CategoryFilter_AllCategoriesID = -1;

// (b.spivey, August 22, 2014) - PLID 63470 - Enums for radio button filters. 
enum ToDoPatientsFilter
{
	tdpfAllTasks = 0,
	tdpfTodaysAppts,
	tdpfApptsMarkedIn,
};

enum ToDoShowFilter
{
	tdsfIncomplete = 0,
	tdsfComplete,
	tdsfAll,
};

/////////////////////////////////////////////////////////////////////////////
// CToDoAlarmDlg dialog


// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

CToDoAlarmDlg::CToDoAlarmDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CToDoAlarmDlg::IDD, pParent), m_tblCheckTask(NetUtils::TodoList), 
	m_UserChecker(NetUtils::Coordinators)
{
	Init();	
}

// (a.walling 2007-05-04 09:53) - PLID 4850 - Initialize the variables (to allow for re-initializing the window when switching users)
void CToDoAlarmDlg::Init()
{
	// Initialize the variables used for auto-sizing the dialog controls
	m_nListWidthAdj = 0;
	m_nListHeightAdj = 0;
	m_nOkayXOffsetFromRight = 0;
	m_nOkayYOffsetFromTop = 0;
	m_nTitleLabelWidth = 0;
	m_nTitleLabelYFromTop = 0;
	m_rPrev.left = m_rPrev.bottom = m_rPrev.right = m_rPrev.top = 0;

	m_bDateDrop = false;

	m_bListColored = false;
	m_rightClicked = -1;
	m_strWhere = "";

	// (j.jones 2014-08-12 11:49) - PLID 63187 - cache if we need a requery upon OnShowWindow
	m_bNeedsRequery = false;

	// (j.armen 2012-06-06 12:38) - PLID 50830 - Set the min dlg size
	SetMinSize(800, 326);
}

void CToDoAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToDoAlarmDlg)	
	DDX_Control(pDX, IDC_PATIENTS_GROUPBOX, m_btnPatientFilterGroup);
	DDX_Control(pDX, IDC_RADIO_TODO_APPTS_MARKED_IN, m_radioApptsMarkedIn);
	DDX_Control(pDX, IDC_RADIO_TODO_TODAYS_APPTS, m_radioTodaysPatients);
	DDX_Control(pDX, IDC_RADIO_TODO_ALL_PATIENTS, m_radioAllPatients);
	DDX_Control(pDX, IDC_RADIO_TODO_INCOMPLETE, m_btnIncomplete);
	DDX_Control(pDX, IDC_RADIO_TODO_COMPLETE, m_btnComplete);
	DDX_Control(pDX, IDC_RADIO_TODO_ALL, m_btnAll);
	DDX_Control(pDX, IDC_CHECK_TODO_HIGH, m_btnHigh);
	DDX_Control(pDX, IDC_CHECK_TODO_MEDIUM, m_btnMedium);
	DDX_Control(pDX, IDC_CHECK_TODO_LOW, m_btnLow);
	DDX_Control(pDX, IDC_REMEMBER_COLUMN_SETTINGS, m_btnRemember);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_BTN_PRINT_TODOS, m_btnPrintPreview);
	DDX_Control(pDX, IDC_OTHER_H_SPIN, m_hourSpin);
	DDX_Control(pDX, IDC_RADIO_OTHER, m_minuteBtn);
	DDX_Control(pDX, IDC_RADIO_OTHER_H, m_hourBtn);
	DDX_Control(pDX, IDC_RADIO_DONTREMIND, m_neverBtn);
	DDX_Control(pDX, IDC_OTHER_SPIN, m_minuteSpin);
	DDX_Control(pDX, IDC_TODO_DATETO, m_DateTo);
	DDX_Control(pDX, IDC_TODO_DATEFROM, m_DateFrom);
	DDX_Control(pDX, IDC_OTHER_TIME, m_nxeditOtherTime);
	DDX_Control(pDX, IDC_OTHER_H_TIME, m_nxeditOtherHTime);
	DDX_Control(pDX, IDC_LABEL_DATE, m_nxstaticLabelDate);
	DDX_Control(pDX, IDC_REMIND_ME_GROUP, m_btnRemindMeGroup);
	DDX_Control(pDX, IDC_STATUS_GROUPBOX, m_btnStatusGroupbox);
	DDX_Control(pDX, IDC_PRIORITY_GROUPBOX, m_btnPriorityGroupbox);
	DDX_Control(pDX, IDC_START_REMIND_GROUPBOX, m_btnStartRemindGroupbox);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATEFROM, 2 /* Change */, OnChangeTodoDateFrom, VTS_NONE)
//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATETO, 2 /* Change */, OnChangeTodoDateTo, VTS_NONE)
//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATEFROM, 4 /* DropDown */, OnDropDownTodoDateFrom, VTS_NONE)
//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATEFROM, 3 /* CloseUp */, OnCloseUpTodoDateFrom, VTS_NONE)
//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATETO, 4 /* DropDown */, OnDropDownTodoDateTo, VTS_NONE)
//	ON_EVENT(CToDoAlarmDlg, IDC_TODO_DATETO, 3 /* CloseUp */, OnCloseUpTodoDateTo, VTS_NONE)

BEGIN_MESSAGE_MAP(CToDoAlarmDlg, CNxDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_NOTIFY(UDN_DELTAPOS, IDC_OTHER_SPIN, OnChangeMinuteSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_OTHER_H_SPIN, OnChangeHourSpin)
	ON_BN_CLICKED(IDC_RADIO_OTHER, OnClickRadioMinute)
	ON_BN_CLICKED(IDC_RADIO_OTHER_H, OnClickRadioHour)
	ON_BN_CLICKED(IDC_RADIO_DONTREMIND, OnClickRadioDontremind)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PRINT_TODOS, OnBtnPrintTodos)
	ON_BN_CLICKED(IDC_REMEMBER_COLUMN_SETTINGS, OnRememberColumnSettings)
	ON_WM_ACTIVATE()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CHECK_TODO_HIGH, OnCheckTodoHigh)
	ON_BN_CLICKED(IDC_CHECK_TODO_LOW, OnCheckTodoLow)
	ON_BN_CLICKED(IDC_CHECK_TODO_MEDIUM, OnCheckTodoMedium)
	ON_BN_CLICKED(IDC_RADIO_TODO_ALL, OnRadioTodoAll)
	ON_BN_CLICKED(IDC_RADIO_TODO_COMPLETE, OnRadioTodoComplete)
	ON_BN_CLICKED(IDC_RADIO_TODO_INCOMPLETE, OnRadioTodoIncomplete)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TODO_DATEFROM, OnChangeTodoDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TODO_DATETO, OnChangeTodoDateTo)
	ON_NOTIFY(DTN_DROPDOWN, IDC_TODO_DATEFROM, OnDropDownTodoDateFrom)
	ON_NOTIFY(DTN_CLOSEUP, IDC_TODO_DATEFROM, OnCloseUpTodoDateFrom)
	ON_NOTIFY(DTN_DROPDOWN, IDC_TODO_DATETO, OnDropDownTodoDateTo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_TODO_DATETO, OnCloseUpTodoDateTo)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_RADIO_TODO_ALL_PATIENTS, OnRadioTodoAllPatients)
	ON_BN_CLICKED(IDC_RADIO_TODO_APPTS_MARKED_IN, OnRadioTodoApptsMarkedIn)
	ON_BN_CLICKED(IDC_RADIO_TODO_TODAYS_APPTS, OnRadioTodoTodaysAppts)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CToDoAlarmDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CToDoAlarmDlg)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 6 /* RButtonDown */, OnRButtonDownList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 19 /* LeftClick */, OnLeftClickList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 3 /* DblClickCell */, OnDblClickCellList, VTS_I4 VTS_I2)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 8 /* EditingStarting */, OnEditingStartingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 18 /* RequeryFinished */, OnRequeryFinishedList, VTS_I2)
	ON_EVENT(CToDoAlarmDlg, IDC_COMBO, 16 /* SelChosen */, OnSelChosenCombo, VTS_I4)
	ON_EVENT(CToDoAlarmDlg, IDC_LIST, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CToDoAlarmDlg, IDC_TODO_CATEGORY_FILTER, 16, CToDoAlarmDlg::SelChosenTodoCategoryFilter, VTS_I4)
	ON_EVENT(CToDoAlarmDlg, IDC_TODO_CATEGORY_FILTER, 1, CToDoAlarmDlg::SelChangingTodoCategoryFilter, VTS_PI4)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToDoAlarmDlg message handlers

BOOL CToDoAlarmDlg::SelectTodoListUser(long nUserID)
{
	long nRow = m_Combo->FindByColumn(0, nUserID, 0, TRUE);
	if (nRow == -1)
		return FALSE;
	OnSelChosenCombo(nRow);//Update the where clause and requery
	return TRUE;
}

void CToDoAlarmDlg::OnShowWindow(BOOL bShow, UINT nStatus)
//Correct use of OnShowWindow, as we are not a modal dialog
//and we need to do this every time the window is shown
{
	try
	{
		if (bShow) 
		{	
			// (a.walling 2006-10-09 10:52) - PLID 22875 - Handle the appwindow setting if it is changed during a session.
			int nTaskbar = GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true);
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			if (nTaskbar == 1)
				nStyle |= WS_EX_APPWINDOW;
			else
				nStyle &= !WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
			
			COleDateTime dt = COleDateTime::GetCurrentTime();
			CString strLabel = "To-Do Alarm For " + FormatDateTimeForInterface(dt, NULL, dtoDate);
			SetDlgItemText(IDC_LABEL_DATE, strLabel);//in case date has changed

			// (j.jones 2008-11-07 09:01) - OnSelChosenCombo() will try to call m_List->Requery() if
			// GenerateWhereClause() returns TRUE. At first glance, that code looks as though it should
			// only return TRUE, and thus requery, if the filters changed. Actually we need to requery
			// every time we open the ToDo Alarm, if we are not filtering on a fixed date range.
			// Otherwise, we would never find alarms with specific remind times. Just be aware of this
			// when reviewing and modifying this code.

			OnSelChosenCombo(m_Combo->SetSelByColumn(0, (_variant_t)GetCurrentUserID())); //Update the where clause and requery

			if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)))
			{
				MessageBox("You do not have permission to read your own To-Do Items");
			}

			//DRT 4/10/03 - Table checker messages if the user list was changed
			if(m_UserChecker.Changed()) {
				long nCurSel = m_Combo->CurSel;
				long nCurID = -1;
				if(nCurSel > -1)
					nCurID = VarLong(m_Combo->GetValue(nCurSel, 0));
				m_Combo->Requery();

				//try to reset the selection
				OnSelChosenCombo(m_Combo->SetSelByColumn(0,(_variant_t)nCurID));//Update the where clause and requery
			}

			// (j.jones 2014-08-12 11:53) - PLID 63187 - if we need to requery, do so now
			if(m_bNeedsRequery) {
				//if the list is already requerying, don't immediately requery again
				if (!m_List->IsRequerying()) {
					m_List->Requery();
				}
				//clear our needs requery flag
				m_bNeedsRequery = false;
			}

			if(!IsNexTechInternal()) {
				//we're not disabling this for clients, just hiding it by default
				m_List->GetColumn(State)->PutStoredWidth(0);
			}

			if(GetRemotePropertyInt("RememberTODOColumns", 0, 0, GetCurrentUserName(), true) == 1) {
				CheckDlgButton(IDC_REMEMBER_COLUMN_SETTINGS, TRUE);
				OnRememberColumnSettings();
			}
			else
				CheckDlgButton(IDC_REMEMBER_COLUMN_SETTINGS, FALSE);			
		}

		RefreshColors();
	}
	NxCatchAll("Error in creating ToDoAlarm");

	CDialog::OnShowWindow(bShow, nStatus);
}

BOOL CToDoAlarmDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 15:27) - PLID 29793 - NxIconified buttons
		m_btnOk.AutoSet(NXB_CLOSE);
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);

		// (z.manning 2008-11-21 10:20) - PLID 31893 - Since this dialog is so common I want it to be
		// as effieient as possible so only cache EMR related preferences if they have EMR.
		CString strEmrPrefs;
		if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
			strEmrPrefs = 
				"Name = 'EmrTodoLinkBehavior' OR ";
		}

		// (a.walling 2007-07-24 13:46) - PLID 26787 - Cache common todo alarm properties
		// (a.walling 2007-09-04 12:36) - PLID 26787 - Added more properties
		// (j.armen 2011-10-25 15:56) - PLID 46139 - GetPracPath references ConfigRT
		// (b.spivey, August 22, 2014) - PLID 63470 - cache all the new properties. 
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		g_propManager.CachePropertiesInBulk("ToDoAlarm-Init", propNumber, "("

			"(Username = '<None>' OR Username = '%s') AND ("
			"%s "
			"Name = 'ToDoColorize' OR "
			"Name = 'ToDoColorComplete' OR "
			"Name = 'ToDoColorIncompleteHigh' OR "
			"Name = 'ToDoColorIncompleteMedium' OR "
			"Name = 'ToDoColorIncompleteLow' OR "
			"Name = 'DisplayTaskbarIcons' OR "
			"Name = 'MyDefaultToDoTab_Patients' OR "
			"Name = 'RememberTODOColumns' OR "
			"Name = 'TodoMinimizeOnPatientSelect' " // (j.jones 2009-03-16 15:21) - PLID 25070
			"OR Name = 'LastOtherTime_User' "		// (d.thompson 2012-06-21) - PLID 51100
			"OR Name = 'DontRemind_User' "			// (d.thompson 2012-06-21) - PLID 51100
			"OR Name = 'LastOtherHourTime_User' "	// (d.thompson 2012-06-21) - PLID 51100
			"OR Name = 'LastTimeOption_User' "		// (d.thompson 2012-06-21) - PLID 51100
			"OR Name = 'ToDoAlarm_PatientsFilter' " 
			"OR Name = 'ToDoAlarm_ShowFilter' "
			"OR Name = 'ToDoAlarm_PriorityLowFilter' " 
			"OR Name = 'ToDoAlarm_PriorityMediumFilter' "
			"OR Name = 'ToDoAlarm_PriorityHighFilter' "
			"OR Name = 'ToDoAlarm_StartRemindingFromFilter' "
			"OR Name = 'ToDoAlarm_StartRemindingToFilter' "
			"OR Name = 'ToDoCategoryFilter' "
			")"

			") OR ("

			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'LastTimeOption' OR "
			"Name = 'DontRemind' OR "
			"Name = 'LastOtherTime' OR "
			"Name = 'LastOtherHourTime' "
			")"

			")", _Q(GetCurrentUserName()), strEmrPrefs, _Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		g_propManager.CachePropertiesInBulk("ToDoAlarm-InitText", propText, "("

			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultTODOColumnSizes' "
			")"

			")", _Q(GetCurrentUserName()));


		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the todolist in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		// (d.thompson 2012-06-20) - PLID 51100 - Move to per-user option
		int minutes = GetRemotePropertyInt("LastTimeOption_User", -1, 0, GetCurrentUserName());
		if(minutes == -1)
		{
			//Legacy per machine/path option
			minutes = GetPropertyInt("LastTimeOption", 10);
		}

		m_List = BindNxDataListCtrl(this, IDC_LIST,GetRemoteData(), false);
		m_Combo = BindNxDataListCtrl(this, IDC_COMBO,GetRemoteData(), false);
		m_CategoryFilter = BindNxDataListCtrl(this, IDC_TODO_CATEGORY_FILTER, GetRemoteData(), true);
		
		CString strPerson;

		//DRT 3/4/2005 - PLID 15849 - New implementation
		//Find out what they're allowed to see
		BOOL bHasSelfRead = TRUE;
		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)))
			bHasSelfRead = FALSE;

		BOOL bHasOtherRead = TRUE;
		if (!(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT__R_________ANDPASS)))
			bHasOtherRead = FALSE;

		//Both self and other
		if(bHasSelfRead && bHasOtherRead) {
			strPerson = "Archived = 0";	//all non-inactive users
		}
		//Does not have permission to self or other
		else if(!bHasSelfRead && !bHasOtherRead) {
			strPerson = "1 = 0";	//noone!
		}
		//self, not other
		else if(bHasSelfRead) {
			strPerson.Format("Archived = 0 AND PersonID = %d", GetCurrentUserID());
		}
		//other, not self
		else if(bHasOtherRead) {
			strPerson.Format("Archived = 0 AND PersonID <> %d", GetCurrentUserID());
		}

		m_Combo->WhereClause = (LPCTSTR)strPerson;
		m_Combo->Requery();

		// (d.thompson 2012-06-20) - PLID 51100 - Move to per-user settings
		long nDontRemind = GetRemotePropertyInt("DontRemind_User", -1, 0, GetCurrentUserName());
		if(nDontRemind == -1) {
			//fall back to legacy per-system/path behavior
			nDontRemind = GetPropertyInt("DontRemind", 0);
		}

		// (d.thompson 2012-06-20) - PLID 51100 - Move to per-user settings
		long nLastOtherTime = GetRemotePropertyInt("LastOtherTime_User", -1, 0, GetCurrentUserName());
		if(nLastOtherTime == -1) {
			//Legacy per machine/path option
			nLastOtherTime = GetPropertyInt("LastOtherTime", 25);
		}

		// (d.thompson 2012-06-20) - PLID 51100 - Move to per-user settings
		long nLastOtherHourTime = GetRemotePropertyInt("LastOtherHourTime_User", -1, 0, GetCurrentUserName());
		if(nLastOtherHourTime == -1) {
			//Legacy per machine/path option
			nLastOtherHourTime = GetPropertyInt("LastOtherHourTime", 1);
		}

		if (nDontRemind){
			m_neverBtn.SetCheck(TRUE);
			m_minuteSpin.EnableWindow(FALSE);
			m_hourSpin.EnableWindow(FALSE);
			SetDlgItemInt(IDC_OTHER_TIME, nLastOtherTime);
			SetDlgItemInt(IDC_OTHER_H_TIME, nLastOtherHourTime);
		}
		else if (minutes % 60 == 0)
		{	SetDlgItemInt(IDC_OTHER_H_TIME, minutes / 60);
			m_hourBtn.SetCheck(TRUE);
			m_minuteSpin.EnableWindow(false);
			SetDlgItemInt(IDC_OTHER_TIME, nLastOtherTime);
		}
		else
		{	SetDlgItemInt(IDC_OTHER_TIME, minutes);
			m_minuteBtn.SetCheck(TRUE);
			m_hourSpin.EnableWindow(false);
			SetDlgItemInt(IDC_OTHER_H_TIME, nLastOtherHourTime);
		}

		GetDlgItem(IDC_LABEL_DATE)->SetFont(&theApp.m_titleFont);

		// Get the dialog size
		CRect rcDialog;
		GetClientRect(&rcDialog);

		// Set the variables used for auto-sizing the dialog controls appropriately
		CWnd *pListWnd = GetDlgItem(IDC_LIST);
		if (pListWnd) {
			CRect rcList;
			pListWnd->GetClientRect(&rcList);
			m_nListWidthAdj = (rcDialog.Width() - rcList.Width()) / 2;  // this was making the right side much fatter than the left
			m_nListHeightAdj = rcDialog.Height() - rcList.Height();
		}
		CWnd *pOkayWnd = GetDlgItem(IDOK);
		if (pOkayWnd) {
			CRect rcOkay;
			pOkayWnd->GetWindowRect(&rcOkay);
			ScreenToClient(&rcOkay);
			m_nOkayXOffsetFromRight = rcDialog.Width() - rcOkay.left;
			m_nOkayYOffsetFromTop = rcOkay.top;
		}
		CWnd *pTitleLabelWnd = GetDlgItem(IDC_LABEL_DATE);
		if (pTitleLabelWnd) {
			CRect rcTitleLabel;
			pTitleLabelWnd->GetWindowRect(&rcTitleLabel);
			ScreenToClient(&rcTitleLabel);
			m_nTitleLabelWidth = rcTitleLabel.Width();
			m_nTitleLabelYFromTop = rcTitleLabel.top;
		}

		// Size the window to the last size it was
		{
			// Get the work area to make sure that wherever we put it, it's accessible
			CRect rcWork;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			// Get the last size and position of the window
			CRect rcDialog;
			CString strBuffer = AfxGetApp()->GetProfileString("Settings", "TodoPopupSize");
			if (strBuffer.IsEmpty() || _stscanf(strBuffer, "%d,%d,%d,%d", &rcDialog.left, &rcDialog.top, &rcDialog.right, &rcDialog.bottom) != 4) {
				// We couldn't get the registry setting for some reason
				CSize ptDlgHalf(TODODLG_DEFAULT_WIDTH/2, TODODLG_DEFAULT_HEIGHT/2);
				CPoint ptScreenCenter(rcWork.CenterPoint());
				rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
			} else {
				// (b.cardillo 2010-03-26 16:31) - PLID 37583
				// Translate from workspace coordinates (which are what we save in the registry) to screen coordinates
				rcDialog.OffsetRect(rcWork.TopLeft());
			}
			// Make sure if we put the dialog at rcDialog it's accessible (we consider 'accessible' 
			// to mean that the dialog title bar is visible vertically, and 1/3 visible horizontally)
			if (rcDialog.top+rcDialog.Height()/8<rcWork.bottom && rcDialog.top>rcWork.top &&
				rcDialog.left<rcWork.right-rcDialog.Width()/3 && rcDialog.right>rcWork.left+rcDialog.Width()/3) {
				// It's accessible so leave it
			} else {
				// It's not accessible so center it
				CSize ptDlgHalf(rcDialog.Width()/2, rcDialog.Height()/2);
				CPoint ptScreenCenter(rcWork.CenterPoint());
				rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
			}
			// Move the window to its new position
			MoveWindow(rcDialog);
		}

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!(GetCurrentUserPermissions(bioPatient) & (SPT___W________ANDPASS)))
		{
			GetDlgItem(IDC_LIST)->EnableWindow(FALSE);
		}
		*/

		//DRT 6/6/03 - Save the sizes of all columns originally so we can reset them if necessary later.
		CString strOrig, str;
		for(int i = 0; i < m_List->GetColumnCount(); i++) {
			IColumnSettingsPtr pCol = m_List->GetColumn(i);
			str.Format("%li,", pCol->GetStoredWidth());

			strOrig += str;
		}

		m_strOriginalColSizes = strOrig;

		//set up today as a default date in the DTPickers.
		m_DateFrom.SetValue((_variant_t)COleDateTime::GetCurrentTime());
		m_DateTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());
					
		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);

		m_DateFrom.SetValue(dtNull);
		m_DateTo.SetValue(dtNull);

		//and select some default values for priorites and status filters

		m_bListColored = false;

		//Reload the category filter
		int nCatFilter = GetRemotePropertyInt("ToDoCategoryFilter", CategoryFilter_AllCategoriesID, 0, GetCurrentUserName(), true);
		m_CategoryFilter->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_CategoryFilter->SetSelByColumn(cfcID, (long)nCatFilter);

		// (b.spivey, August 22, 2014) - PLID 63470 - Now load all the new properties.
		switch (GetRemotePropertyInt("ToDoAlarm_PatientsFilter", tdpfAllTasks, 0, GetCurrentUserName()))
		{

			case tdpfAllTasks:
				// (j.jones 2008-09-30 11:28) - PLID 31331 - default the patient filter to "all"
				// (which actually means all tasks, not all patient-only tasks
				m_radioAllPatients.SetCheck(TRUE);
				break;

			case tdpfTodaysAppts:
				m_radioTodaysPatients.SetCheck(TRUE);
				break;

			case tdpfApptsMarkedIn:
				m_radioApptsMarkedIn.SetCheck(TRUE); 
				break; 

			default:
				//This shouldn't happen. Did someone add a new filter and forgot to update this case?
				ASSERT(FALSE); 
				break; 
		}


		switch (GetRemotePropertyInt("ToDoAlarm_ShowFilter", tdsfIncomplete, 0, GetCurrentUserName()))
		{

			case tdsfIncomplete:
				m_btnIncomplete.SetCheck(TRUE); 
				break;

			case tdsfComplete:
				m_btnComplete.SetCheck(TRUE);
				break;

			case tdsfAll:
				m_btnAll.SetCheck(TRUE);
				break;

			default:
				//This shouldn't happen. Did someone add a new filter and forgot to update this case?
				ASSERT(FALSE);
				break;
		}

		if (GetRemotePropertyInt("ToDoAlarm_PriorityLowFilter", TRUE, 0, GetCurrentUserName())) {
			m_btnLow.SetCheck(TRUE);
		}
		else {
			m_btnLow.SetCheck(FALSE);
		}

		if (GetRemotePropertyInt("ToDoAlarm_PriorityMediumFilter", TRUE, 0, GetCurrentUserName())) {
			m_btnMedium.SetCheck(TRUE);
		}
		else {
			m_btnMedium.SetCheck(FALSE);
		}

		if (GetRemotePropertyInt("ToDoAlarm_PriorityHighFilter", TRUE, 0, GetCurrentUserName())) {
			m_btnHigh.SetCheck(TRUE);
		}
		else {
			m_btnHigh.SetCheck(FALSE);
		}


		long nStartRemindingFrom = GetRemotePropertyInt("ToDoAlarm_StartRemindingFromFilter", FALSE, 0, GetCurrentUserName());

		if (!!nStartRemindingFrom) {
			COleDateTime dtFrom;
			dtFrom = GetRemotePropertyDateTime("ToDoAlarm_StartRemindingFromFilter", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName());

			m_DateFrom.SetValue(dtFrom);
		}
		else {
			m_DateFrom.SetValue((_variant_t)COleDateTime::GetCurrentTime());

			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);

			m_DateFrom.SetValue(dtNull);
		}


		long nStartRemindingTo = GetRemotePropertyInt("ToDoAlarm_StartRemindingToFilter", FALSE, 0, GetCurrentUserName());

		if (!!nStartRemindingTo) {
			COleDateTime dtTo;
			dtTo = GetRemotePropertyDateTime("ToDoAlarm_StartRemindingToFilter", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName());

			m_DateTo.SetValue((_variant_t)dtTo);
		}
		else {
			m_DateTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());

			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);

			m_DateTo.SetValue(dtNull);
		}


		if (GenerateWhereClause()) {
			m_List->Requery();
		}

		RefreshColors();
	}
	NxCatchAll("Error in CToDoAlarmDlg::OnInitDialog");

	return TRUE;
}

void CToDoAlarmDlg::OnClose() 
{
	OnOK();
}

void CToDoAlarmDlg::Save()
{
	int minutes;

	//lets discuss if we really need to save this every time they hit ok

	if (m_minuteBtn.GetCheck())
	{	minutes = GetDlgItemInt(IDC_OTHER_TIME);
		if(minutes > 500) {
			MessageBox("You cannot set your remind time for more than 500 minutes.\n"
				"The remind time has been reduced to 500.");
			minutes = 500;
			SetDlgItemInt(IDC_OTHER_TIME,500);
		}
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("LastOtherTime_User", minutes, 0, GetCurrentUserName());
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("DontRemind_User", 0, 0, GetCurrentUserName());
	}
	else if (m_hourBtn.GetCheck())
	{	minutes = GetDlgItemInt(IDC_OTHER_H_TIME);
		if(minutes > 500) {
			MessageBox("You cannot set your remind time for more than 500 hours.\n"
				"The remind time has been reduced to 500.");
			minutes = 500;
			SetDlgItemInt(IDC_OTHER_H_TIME,500);
		}
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("LastOtherHourTime_User", minutes, 0, GetCurrentUserName());
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("DontRemind_User", 0, 0, GetCurrentUserName());
		minutes *= 60;
	}
	else if (m_neverBtn.GetCheck()){
		minutes = 0;
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("DontRemind_User", 1, 0, GetCurrentUserName());
	}
	else
	{	ASSERT(FALSE);
		minutes = 10;
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("DontRemind_User", 0, 0, GetCurrentUserName());
	}

	//Save the category filter
	long nCategoryFilterID = CategoryFilter_AllCategoriesID;
	if (m_CategoryFilter->GetCurSel() != sriNoRow) {
		nCategoryFilterID = VarLong(m_CategoryFilter->GetValue(m_CategoryFilter->GetCurSel(), cfcID));
	}
	SetRemotePropertyInt("ToDoCategoryFilter", nCategoryFilterID, 0, GetCurrentUserName());

	// b.cardillo (2003-04-17 12:11:00) - Notice this code is similar to the code in 
	// CMainFrame::WindowProc() WM_TABLE_CHANGED(NetUtils::TodoList).  Ultimately I think they need to both 
	// call a shared function to produce the same results, but for now we just need to keep the two 
	// functions in sync.

	//DRT - 12/02/02 - Made a big change to the way the timer is setup.  If you enter a time (anything other than midnight) for a
	//			task, then you will be prompted at that task time no matter what.  This means that if the time is wanted to be set
	//			to 1 hour from now, but there is a task with a time of 30 minutes from now, we need to have the timer set 30 mins, 
	//			not 1 hour.  Also, if they turn it off (dont remind), but there is a time, we must set the timer to that time, 
	//			and not turned off.
	//			For now, it is ONLY turned on for Internal use, not for general use

	//DRT 1/30/03 - This is now enabled for ALL users (not just internal usage).  Also, I made it use the remind time, not the deadline (since that isn't what
	//			determines when the popup comes up)

	//firstly, find the next todo for the current user
	//DRT 3/11/03 - ignore midnight times!
	// (c.haag 2008-06-10 09:30) - PLID 11599 - Use TodoAssignToT
	// (a.walling 2013-07-12 08:42) - PLID 57537 - The ToDoList save also checks for the min upcoming 
	// remind time, but ignores > 24hrs, so we can limit this range to prevent scans when the ToDoList becomes 
	// large and there are many tasks with remind times in the future.
	_RecordsetPtr rs = CreateParamRecordset("SELECT TodoAssignToT.AssignTo, Min(Remind) AS MinTime FROM ToDoList "
		"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
		"WHERE TodoAssignToT.AssignTo = {INT} AND Remind >= GetDate() AND Remind < DATEADD(d, 1, GetDate()) AND convert(nvarchar, Remind, 8) > '00:00:00' GROUP BY AssignTo", GetCurrentUserID());

	COleDateTime dtNext;
	int nSeconds = -1;	//number of seconds until the next todo is due
	if(!rs->eof) {
		dtNext = rs->Fields->Item["MinTime"]->Value.date;
		nSeconds = (int)(dtNext - COleDateTime::GetCurrentTime()).GetTotalSeconds();
	}

	//DRT 3/21/03 - There was an issue with the "next time" being too far away, creating values that went over the highest long.
	//		This wrapped around, causing a negative time to get entered for the timer (which pops it up immediately).  Now, if
	//		the time is > 24 hours (86400 seconds), just set it to 0.  They aren't going to be running Practice for days at a time
	//		anyways.
	if(nSeconds > 86400) {
		nSeconds = -1;
	}

	//if our next time is sooner, use it instead of the time in the dialog
	//or if the minutes is set to 0 (dont remind), and we have a next time
	if((nSeconds > -1 && nSeconds < (minutes * 60)) || (minutes == 0 && nSeconds > -1)) {
		GetMainFrame()->SetToDoTimer(nSeconds * 1000);
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("LastTimeOption_User", nSeconds / 60, 0, GetCurrentUserName());
		if(minutes == 0)
			SetRemotePropertyInt("DontRemind_User", 0, 0, GetCurrentUserName());	//undo the dont remind setting
	}
	else {
		GetMainFrame()->SetToDoTimer(minutes * 60 * 1000);
		// (d.thompson 2012-06-21) - PLID 51100 - Change to per-user settings
		SetRemotePropertyInt("LastTimeOption_User", minutes, 0, GetCurrentUserName());
	}


	// (b.spivey, August 22, 2014) - PLID 63470 - Now save all the new properties. 
	ToDoPatientsFilter tdpf = ToDoPatientsFilter::tdpfAllTasks;

	if (m_radioAllPatients.GetCheck()) {
		tdpf = ToDoPatientsFilter::tdpfAllTasks;
	}
	else if (m_radioTodaysPatients.GetCheck()) {
		tdpf = ToDoPatientsFilter::tdpfTodaysAppts;
	}
	else if (m_radioApptsMarkedIn.GetCheck()) {
		tdpf = ToDoPatientsFilter::tdpfApptsMarkedIn;
	}
	else {
		// Looks like a new setting was added to this group and was not added to the save block. Is that right?
		ASSERT(FALSE);
	}

	SetRemotePropertyInt("ToDoAlarm_PatientsFilter", (long)tdpf, 0, GetCurrentUserName()); 


	ToDoShowFilter tdsf = ToDoShowFilter::tdsfAll;

	if (m_btnAll.GetCheck()) {
		tdsf = ToDoShowFilter::tdsfAll;
	}
	else if (m_btnIncomplete.GetCheck()) {
		tdsf = ToDoShowFilter::tdsfIncomplete;
	}
	else if (m_btnComplete.GetCheck()) {
		tdsf = ToDoShowFilter::tdsfComplete;
	}
	else {
		// Looks like a new setting was added to this group and was not added to the save block. Is that right?
		ASSERT(FALSE);
	}

	SetRemotePropertyInt("ToDoAlarm_ShowFilter", (long)tdsf, 0, GetCurrentUserName()); 

	SetRemotePropertyInt("ToDoAlarm_PriorityLowFilter", m_btnLow.GetCheck(), 0, GetCurrentUserName());
	SetRemotePropertyInt("ToDoAlarm_PriorityMediumFilter", m_btnMedium.GetCheck(), 0, GetCurrentUserName());
	SetRemotePropertyInt("ToDoAlarm_PriorityHighFilter", m_btnHigh.GetCheck(), 0, GetCurrentUserName());
	
	COleDateTime dtFrom = VarDateTime(m_DateFrom.GetValue(), g_cdtNull);

	if (dtFrom == g_cdtNull) {
		SetRemotePropertyInt("ToDoAlarm_StartRemindingFromFilter", FALSE, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyInt("ToDoAlarm_StartRemindingFromFilter", TRUE, 0, GetCurrentUserName());
		SetRemotePropertyDateTime("ToDoAlarm_StartRemindingFromFilter", dtFrom, 0, GetCurrentUserName());
	}


	COleDateTime dtTo = VarDateTime(m_DateTo.GetValue(), g_cdtNull);

	if (dtTo == g_cdtNull) {
		SetRemotePropertyInt("ToDoAlarm_StartRemindingToFilter", FALSE, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyInt("ToDoAlarm_StartRemindingToFilter", TRUE, 0, GetCurrentUserName());
		SetRemotePropertyDateTime("ToDoAlarm_StartRemindingToFilter", dtTo, 0, GetCurrentUserName());
	}
	


	// Now save the box size and position
	//TES 5/27/2004: Note that we are using GetWindowPlacement; this gives us the rect for the window when it isn't 
	//minimized, even if at the moment it happens to be minimized.
	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	CString strBuffer;
	
	strBuffer.Format("%d,%d,%d,%d", wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);

	AfxGetApp()->WriteProfileString("Settings", "TodoPopupSize", strBuffer);
}

void CToDoAlarmDlg::OnOK() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		Save();

		//Restore mainframe, in case it's minimized, weird things might happen.
		// (a.walling 2012-04-02 08:32) - PLID 46648 - Don't try to activate the main frame; we have not been a modal window for years
		//CMainFrame* pMain = GetMainFrame();
		//if(pMain) {
		//	pMain->ActivateFrame();
		//	pMain->BringWindowToTop();
		//}

		//TS 5-8-03: DON'T call OnOK() for a modeless dialog; when you do, MainFrame will start handling messages
		//again, even if there's another modal dialog up.
		//CDialog::OnOK();
		ShowWindow(SW_HIDE);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnChangeMinuteSpin(NMHDR* pNMHDR, LRESULT* pResult) 
//wierd function name
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		//change minutes
		NM_UPDOWN* pUpDown = (NM_UPDOWN*)pNMHDR;

		if((int)(GetDlgItemInt(IDC_OTHER_TIME) - pUpDown->iDelta * 5) < 1)
			return;
		SetDlgItemInt(IDC_OTHER_TIME, GetDlgItemInt(IDC_OTHER_TIME) - pUpDown->iDelta * 5);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnChangeHourSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		NM_UPDOWN* pUpDown = (NM_UPDOWN*)pNMHDR;

		_variant_t var;
		var.vt = 17;

		if((int)(GetDlgItemInt(IDC_OTHER_H_TIME) - pUpDown->iDelta * 1) < 1)
			return;
		SetDlgItemInt(IDC_OTHER_H_TIME, GetDlgItemInt(IDC_OTHER_H_TIME) - pUpDown->iDelta * 1);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags)
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		CMenu pMenu, pSubMenu;
		m_rightClicked = nRow;
		m_List->CurSel = nRow;
		GetDlgItem(IDC_LIST)->SetFocus();

		if(nRow == -1)
			return;

		// Build a menu popup with the ability to delete the current row
		pMenu.CreatePopupMenu();
		pSubMenu.CreatePopupMenu();
		pSubMenu.InsertMenu(0, MF_BYPOSITION, ID_SNOOZE_1_HOUR, "For 1 Hour");
		pSubMenu.InsertMenu(1, MF_BYPOSITION, ID_SNOOZE_1_DAY, "For 1 Day");
		pSubMenu.InsertMenu(2, MF_BYPOSITION, ID_SNOOZE_1_WEEK, "For 1 Week");

		pMenu.InsertMenu(0, MF_BYPOSITION, ID_MODIFY, "Modify Task");
		pMenu.InsertMenu(1, MF_BYPOSITION, ID_MARKDONE, "Mark As Completed");
		pMenu.InsertMenu(2, MF_SEPARATOR, 2, "");
		pMenu.InsertMenu(3, MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu.m_hMenu, "'Snooze' Alarm...");

		_variant_t var = m_List->GetValue(nRow, Completed);
		if (var.vt != VT_NULL) {
			pMenu.EnableMenuItem(3, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			pMenu.EnableMenuItem(1, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}NxCatchAll(__FUNCTION__);
}

BOOL CToDoAlarmDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(m_rightClicked==-1)
		return CDialog::OnCommand(wParam, lParam);

	switch (wParam) {
	case ID_MODIFY:
		//(e.lally 2009-11-17) PLID 36304 - Added try/catch
		try{
			// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
			//to edit a todo alarm
			/*
			if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
				break;
			*/
			//TES 6/17/03: Don't let them modify if they don't have access.
			if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
			{
				if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
					break;
			}
			else
			{
				if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
					break;
			}

			LoadTodoListScreen(VarLong(m_List->Value[m_rightClicked][TaskID]));
		}NxCatchAll("CToDoAlarmDlg::OnCommand - Could not modify Todo Alarm");
		break;
	case ID_MARKDONE:
		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			break;
		*/
		try 
		{	
			// CAH 5/5/2003 - Don't let them modify the todo item if they don't have
			// write access
			if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
			{
				if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
					break;
			}
			else
			{
				if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
					break;
			}


			long nTaskID = VarLong(m_List->Value[m_rightClicked][TaskID]);
			ExecuteSql("UPDATE ToDoList SET Done = GetDate() WHERE TaskID = %li", 
				nTaskID);

			CString strTitle;
			strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
			SetWindowText(strTitle);

			//update everything
			PhaseTracking::SyncLadderWithTodo(nTaskID);
			// (z.manning 2008-10-29 12:13) - PLID 31667 - Update labs
			SyncLabWithTodo(nTaskID);
			
			// (s.tullis 2014-09-08 09:10) - PLID 63344 
			long nPersonID = VarLong(m_List->Value[m_rightClicked][PersonID], -1);
			CString strAssignedIDs = VarString(m_List->Value[m_rightClicked][AssignedIDs], "");

			CArray < long, long> arrAssignedID;
			ParseDelimitedStringToLongArray(strAssignedIDs, " ", arrAssignedID);

			if (arrAssignedID.GetSize() == 1){
				CClient::RefreshTodoTable(nTaskID, nPersonID, arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
			}
			else{
				CClient::RefreshTodoTable(nTaskID, nPersonID, -1, TableCheckerDetailIndex::tddisChanged);
			}

			
			
			if (((CButton*)GetDlgItem(IDC_RADIO_TODO_INCOMPLETE))->GetCheck()) {
				// (a.walling 2006-08-03 08:48) - PLID 20419
				// we only need to remove this row if we are only showing incomplete entries
				m_List->RemoveRow(m_rightClicked);
			}
			else {
				IRowSettingsPtr pRow = m_List->GetRow(m_rightClicked);
				if (pRow) {
					pRow->PutValue(Completed, _variant_t(COleDateTime::GetCurrentTime()));
				}
				ColorizeList(m_rightClicked); // (a.walling 2006-08-03 08:34) - PLID 20419 - re-color the item
			}			
			
			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			CString strSQL, strPatientName;
			if(nAuditID != -1) {
				//(e.lally 2007-06-04) PLID 25253 - Fixed the audit query to look at ToDoList and left join on PersonT
				// (a.walling 2008-02-07 17:49) - PLID 28692 - This only needs to use one recordset anyway, and inner join
				_RecordsetPtr rsPatient = CreateParamRecordset("SELECT First, Middle, Last, ToDoList.PersonID, PatientsT.PersonID AS PatientID "
												"FROM ToDoList " 
												"INNER JOIN PersonT ON ToDoList.PersonID = PersonT.ID "
												"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
												"WHERE ToDoList.TaskID = {INT}", nTaskID);
				// (a.walling 2008-01-22 13:12) - PLID 28692 - This should be if (!rsPatient->eof)
				if(!rsPatient->eof){
					strPatientName.Format("%s, %s %s", AdoFldString(rsPatient, "Last", ""), AdoFldString(rsPatient, "First", ""), AdoFldString(rsPatient, "Middle", ""));
					// strSQL.Format("SELECT PersonID FROM PatientsT WHERE PersonID = %li", AdoFldLong(rsPatient, "PersonID"));
					// (a.walling 2008-02-07 17:50) - PLID 28692 - This will be a patient if the join to PatientsT succeeded
					long nPatientID = AdoFldLong(rsPatient, "PatientID", -1);
					if (nPatientID != -1)
						AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientTodoTaskComplete, nTaskID, "Uncompleted", "Completed", aepMedium, aetChanged);
					else
						AuditEvent(-1, strPatientName, nAuditID, aeiContactTodoTaskComplete, nTaskID, "Uncompleted", "Completed", aepMedium, aetChanged);
				}
				else
					AuditEvent(-1, "", nAuditID, aeiContactTodoTaskComplete, nTaskID, "Uncompleted", "Completed", aepMedium, aetChanged);
			}
		}NxCatchAll("Error in ToDoAlarmDlg::OnPopupSelectionList");

		break;
	case ID_SNOOZE_1_HOUR: {
		try {
		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			break;
		*/
		//TES 6/17/03: Don't let them modify if they don't have access.
		if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
				break;
		}
		else
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
				break;
		}

		long nTaskID = VarLong(m_List->Value[m_rightClicked][TaskID]);

		//update the remind time to be this exact time plus one hour
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtOneHour;
		dtOneHour.SetDateTimeSpan(0,1,0,0);
		dtNow += dtOneHour;

		COleDateTime dtDeadline;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Deadline FROM ToDoList WHERE TaskID = {INT}",nTaskID);
		if(!rs->eof) {
			COleDateTime dtCompare;
			dtCompare.SetDateTime(dtNow.GetYear(),dtNow.GetMonth(),dtNow.GetDay(),0,0,0);
			dtDeadline = AdoFldDateTime(rs, "Deadline");
			if(dtDeadline < dtCompare) {
				if(IDNO == MessageBox("The deadline for this task is before the new remind time.\n"
					"If you proceed, the deadline will be changed to be equal to this remind time.\n"
					"Are you sure you wish to change the times on this task?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return CDialog::OnCommand(wParam, lParam);
				}
				ExecuteSql("UPDATE ToDoList SET Deadline = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtCompare,dtoDate),nTaskID);
			}
		}

		ExecuteSql("UPDATE ToDoList SET Remind = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtNow),nTaskID);

		SetWindowText(TITLE_BASE_STRING + CString(" - Loading..."));
		m_List->Requery();
		//clear our needs requery flag
		m_bNeedsRequery = false;

		//auditing
		long nAuditID = BeginNewAuditEvent();
		CString strNew = "", strOld = "", strPersonName, strSQL;
		BOOL bFirst = TRUE;
		if(nAuditID != -1) {				
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate) != FormatDateTimeForInterface(dtNow, 0, dtoDate)) {
				strOld += "Deadline: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate);
				strNew += "Deadline: " + FormatDateTimeForInterface(dtNow, 0, dtoDate);
				bFirst = FALSE;
			}
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime) != FormatDateTimeForInterface(dtNow, 0, dtoTime)) {
				if (bFirst == FALSE) {
					strOld += ", ";
					strNew += ", ";
				}
				strOld += "Remind Time: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime);
				strNew += "Remind Time: " + FormatDateTimeForInterface(dtNow, 0, dtoTime);
			}
			if (strNew != "") {
				long nPersonID = -1;
				_RecordsetPtr rsPerson = CreateParamRecordset("SELECT First, Middle, Last, ToDoList.PersonID "
												"FROM PersonT "
												"LEFT JOIN ToDoList ON ToDoList.PersonID = PersonT.ID "
												"WHERE ToDoList.TaskID = {INT}", nTaskID);
				//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
				if(rsPerson->eof)
					strPersonName = "<No Supplier>";
				else{
					strPersonName.Format("%s, %s %s", AdoFldString(rsPerson, "Last", ""), AdoFldString(rsPerson, "First", ""), AdoFldString(rsPerson, "Middle", ""));
					nPersonID = AdoFldLong(rsPerson, "PersonID", -1);
				}
				// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPersonID))		
					AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
				else 
					AuditEvent(-1, strPersonName, nAuditID, aeiContactToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
			}
		}

		rs->Close();

		}NxCatchAll("Error snoozing alarm for 1 hour.");
		break;
		}
	case ID_SNOOZE_1_DAY: {
		try {
		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			break;
		*/
		//TES 6/17/03: Don't let them modify if they don't have access.
		if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
				break;
		}
		else
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
				break;
		}

		long nTaskID = VarLong(m_List->Value[m_rightClicked][TaskID]);

		//update the remind time to be tomorrow, without a time
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtOneDay;
		dtOneDay.SetDateTimeSpan(1,0,0,0);
		dtNow += dtOneDay;

		COleDateTime dtDeadline;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Deadline FROM ToDoList WHERE TaskID = {INT}",nTaskID);
		if(!rs->eof) {
			COleDateTime dtCompare;
			dtCompare.SetDateTime(dtNow.GetYear(),dtNow.GetMonth(),dtNow.GetDay(),0,0,0);
			dtDeadline = AdoFldDateTime(rs, "Deadline");
			if(dtDeadline < dtCompare) {
				if(IDNO == MessageBox("The deadline for this task is before the new remind time.\n"
					"If you proceed, the deadline will be changed to be equal to this remind time.\n"
					"Are you sure you wish to change the times on this task?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return CDialog::OnCommand(wParam, lParam);
				}
				ExecuteSql("UPDATE ToDoList SET Deadline = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtCompare,dtoDate),nTaskID);
			}
		}

		ExecuteSql("UPDATE ToDoList SET Remind = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtNow,dtoDate),nTaskID);

		SetWindowText(TITLE_BASE_STRING + CString(" - Loading..."));
		m_List->Requery();
		//clear our needs requery flag
		m_bNeedsRequery = false;

		//auditing
		long nAuditID = BeginNewAuditEvent();
		CString strNew = "", strOld = "", strPersonName, strSQL;
		BOOL bFirst = TRUE;
		if(nAuditID != -1) {				
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate) != FormatDateTimeForInterface(dtNow, 0, dtoDate)) {
				strOld += "Deadline: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate);
				strNew += "Deadline: " + FormatDateTimeForInterface(dtNow, 0, dtoDate);
				bFirst = FALSE;
			}
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime) != FormatDateTimeForInterface(dtNow, 0, dtoTime)) {
				if (bFirst == FALSE) {
					strOld += ", ";
					strNew += ", ";
				}
				strOld += "Remind Time: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime);
				strNew += "Remind Time: " + FormatDateTimeForInterface(dtNow, 0, dtoTime);
			}
			if (strNew != "") {
				//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
				long nPersonID = -1;
				_RecordsetPtr rsPerson = CreateParamRecordset("SELECT First, Middle, Last, ToDoList.PersonID "
												"FROM PersonT "
												"LEFT JOIN ToDoList ON ToDoList.PersonID = PersonT.ID "
												"WHERE ToDoList.TaskID = {INT}", nTaskID);
				if(rsPerson->eof)
					strPersonName = "<No Supplier>";
				else{
					strPersonName.Format("%s, %s %s", AdoFldString(rsPerson, "Last", ""), AdoFldString(rsPerson, "First", ""), AdoFldString(rsPerson, "Middle", ""));
					nPersonID = AdoFldLong(rsPerson, "PersonID", -1);
				}
				// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPersonID))		
					AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
				else 
					AuditEvent(-1, strPersonName, nAuditID, aeiContactToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
			}
		}

		rs->Close();

		}NxCatchAll("Error snoozing alarm for 1 day.");
		break;
		}
	case ID_SNOOZE_1_WEEK: {
		try {
		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			break;
		*/
		//TES 6/17/03: Don't let them modify if they don't have access.
		if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
				break;
		}
		else
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
				break;
		}

		long nTaskID = VarLong(m_List->Value[m_rightClicked][TaskID]);

		//update the remind time to be this day next week, without a time
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtOneWeek;
		dtOneWeek.SetDateTimeSpan(7,0,0,0);
		dtNow += dtOneWeek;

		COleDateTime dtDeadline;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Deadline FROM ToDoList WHERE TaskID = {INT}",nTaskID);
		if(!rs->eof) {
			COleDateTime dtCompare;
			dtCompare.SetDateTime(dtNow.GetYear(),dtNow.GetMonth(),dtNow.GetDay(),0,0,0);
			dtDeadline = AdoFldDateTime(rs, "Deadline");
			if(dtDeadline < dtCompare) {
				if(IDNO == MessageBox("The deadline for this task is before the new remind time.\n"
					"If you proceed, the deadline will be changed to be equal to this remind time.\n"
					"Are you sure you wish to change the times on this task?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return CDialog::OnCommand(wParam, lParam);
				}
				ExecuteSql("UPDATE ToDoList SET Deadline = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtCompare,dtoDate),nTaskID);
			}
		}

		ExecuteSql("UPDATE ToDoList SET Remind = Convert(datetime,'%s') WHERE TaskID = %li",FormatDateTimeForSql(dtNow,dtoDate),nTaskID);

		SetWindowText(TITLE_BASE_STRING + CString(" - Loading..."));
		m_List->Requery();
		//clear our needs requery flag
		m_bNeedsRequery = false;

		//auditing
		long nAuditID = BeginNewAuditEvent();
		CString strNew = "", strOld = "", strPersonName, strSQL;
		BOOL bFirst = TRUE;
		if(nAuditID != -1) {				
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate) != FormatDateTimeForInterface(dtNow, 0, dtoDate)) {
				strOld += "Deadline: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoDate);
				strNew += "Deadline: " + FormatDateTimeForInterface(dtNow, 0, dtoDate);
				bFirst = FALSE;
			}
			if (FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime) != FormatDateTimeForInterface(dtNow, 0, dtoTime)) {
				if (bFirst == FALSE) {
					strOld += ", ";
					strNew += ", ";
				}
				strOld += "Remind Time: " + FormatDateTimeForInterface(AdoFldDateTime(rs, "Deadline"), 0, dtoTime);
				strNew += "Remind Time: " + FormatDateTimeForInterface(dtNow, 0, dtoTime);
			}
			if (strNew != "") {
				//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
				long nPersonID = -1;
				_RecordsetPtr rsPerson = CreateParamRecordset("SELECT First, Middle, Last, ToDoList.PersonID "
												"FROM PersonT "
												"LEFT JOIN ToDoList ON ToDoList.PersonID = PersonT.ID "
												"WHERE ToDoList.TaskID = {INT}", nTaskID);
				if(rsPerson->eof)
					strPersonName = "<No Supplier>";
				else{
					strPersonName.Format("%s, %s %s", AdoFldString(rsPerson, "Last", ""), AdoFldString(rsPerson, "First", ""), AdoFldString(rsPerson, "Middle", ""));
					nPersonID = AdoFldLong(rsPerson, "PersonID", -1);
				}
				// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPersonID))		
					AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
				else 
					AuditEvent(-1, strPersonName, nAuditID, aeiContactToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
			}
		}

		rs->Close();

		}NxCatchAll("Error snoozing alarm for 1 week.");
		break;
		}
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// (z.manning 2008-11-21 10:47) - PLID 31893 - Added a parameter for tab. If -1 the user's default is used.
void CToDoAlarmDlg::LoadPatientScreen(long nID, long nTab /* = -1 */)
//only call this if we are sure we have a patient
//lets discuss if these should be a method of CPatientView or CMainFrame
{
	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;

	if (nID != GetActivePatientID()) {
		if(!p->m_patToolBar.DoesPatientExistInList(nID)) {
			if(IDNO == MessageBox("This patient is not in the current lookup. \n"
				"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		//TES 1/7/2010 - PLID 36761 - This function may fail now
		if(!p->m_patToolBar.TrySetActivePatientID(nID)) {
			return;
		}
	}	

	if(p->FlipToModule(PATIENT_MODULE_NAME)) {

		pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
		if (pView) 
		{
			// (m.hancock 2006-05-09 11:16) - PLID 20433 - Clicking a patient name
			//in To-Do should open the user's preferred tab in Patient module.  We
			//used to just set the active tab to General 1 and display it, but now
			//we need to allow the user to set a preferred tab for display.
			
			// Get the user's preferred tab
			// (m.hancock 2006-07-20 10:49) - PLID 20433 - Need to link the tab id from prefs with the tab id used
			// in patientview.h, so call ResolveDefaultTab to get the correct tab id.
			long nPreferredTab;
			if(nTab == -1) {
				nPreferredTab = g_Modules[Modules::Patients]->ResolveDefaultTab((short)GetRemotePropertyInt("MyDefaultToDoTab_Patients", 0, 0, GetCurrentUserName(), true) );
			}
			else {
				// (z.manning 2008-11-21 11:12) - PLID 31893 - The caller passed in a tab, so go to that one
				// instead of the default.
				nPreferredTab = nTab;
			}
			
			// If the active tab != preferred tab, set the tab to the preferred tab
			if(nPreferredTab != pView->GetActiveTab())
				pView->SetActiveTab((short)nPreferredTab);

			// Hide or minize the To-Do tasks
			// (j.jones 2009-03-16 15:20) - PLID 25070 - added preference to minimize the to-do list
			if(GetRemotePropertyInt("TodoMinimizeOnPatientSelect", 1, 0, GetCurrentUserName(), true)) {
				ShowWindow(SW_MINIMIZE);
			}

			// Display the tab
			pView->UpdateView();
		}
	}
}

void CToDoAlarmDlg::LoadContactScreen(long nID)
{
	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;
	// (d.lange 2011-06-03 17:33) - PLID 43253 - Check against a bad ID
	if(nID != -1) {
		if(p->FlipToModule(CONTACT_MODULE_NAME)) {

			if (nID != GetActiveContactID())
			{	p->m_contactToolBar.SetActiveContactID(nID);
			}

			pView = (CNxTabView *)p->GetOpenView(CONTACT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()==0)
					pView->UpdateView();
				else
					pView->SetActiveTab(0);
			}

			// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
			if(IsNexTechInternal()) {
				((CContactView*)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
			}
		}
	}
}

//(e.lally 2010-05-06) PLID 36567 - Go to patient's lab tab and open the lab up.
// (z.manning 2010-05-12 16:28) - PLID 37405 - Added regarding type
void CToDoAlarmDlg::LoadLabEntryScreen(long nPatientID, TodoType eRegardingType, long nRegardingID)
{
	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;

	if (nPatientID != GetActivePatientID()) {
		if(!p->m_patToolBar.DoesPatientExistInList(nPatientID)) {
			if(IDNO == MessageBox("This patient is not in the current lookup. \n"
				"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		//TES 1/7/2010 - PLID 36761 - This function may fail now
		if(!p->m_patToolBar.TrySetActivePatientID(nPatientID)) {
			return;
		}
	}	

	if(p->FlipToModule(PATIENT_MODULE_NAME)) {

		pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
		if (pView) 
		{
			//(e.lally 2010-05-06) PLID 36567 - Go to the Labs tab if they have read permission
			if((GetCurrentUserPermissions(bioPatientLabs) & (sptRead|sptReadWithPass))){
				if(PatientsModule::LabTab != pView->GetActiveTab())
					pView->SetActiveTab(PatientsModule::LabTab);

				// Hide or minize the To-Do tasks
				// (j.jones 2009-03-16 15:20) - PLID 25070 - added preference to minimize the to-do list
				if(GetRemotePropertyInt("TodoMinimizeOnPatientSelect", 1, 0, GetCurrentUserName(), true)) {
					ShowWindow(SW_MINIMIZE);
				}
				// Display the tab
				pView->UpdateView();
			}
		}
	}

	//(e.lally 2010-05-06) PLID 36567 - Tell the user why we aren't opening the lab, if they don't have permission
	if(!(GetCurrentUserPermissions(bioPatientLabs) & (sptWrite|sptWriteWithPass))){
		MessageBox("You do not have permission to edit this Lab.", "Practice", MB_OK | MB_ICONWARNING);
		return;
	}

	//Now enforce the permission with the check.
	if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite)){
		return;
	}

	//(e.lally 2010-05-06) PLID 36567 - The regardingID is actually the stepID of the uncompleted step, 
		//so we'll have to look up what lab it belongs to.
	// (z.manning 2010-05-12 16:29) - PLID 37405 - Support multiple regarding types
	long nLabID = -1;
	switch(eRegardingType)
	{
		case ttLab:
			nLabID = nRegardingID;
			break;

		case ttLabStep:
			_RecordsetPtr rs = CreateParamRecordset("SELECT LabID FROM LabStepsT WHERE StepID = {INT}", nRegardingID);
			if(!rs->eof){
				nLabID = AdoFldLong(rs, "LabID", -1);
			}
			break;
	}

	if(nLabID != -1) {
		// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
		// If the lab changed, a table checker message will be dispatched, and we will handle requeries from
		// there. There should also not be window positioning problems opening reports.
		//TES 8/5/2011 - PLID 44901 - They may not have permission to view this lab, based on its location
		CArray<long,long> arAllowedLocationIDs;
		if(!PollLabLocationPermissions(arAllowedLocationIDs)) {
			//TES 8/5/2011 - PLID 44901 - If that function had returned TRUE, we'd know they had permission to all locations.  Since they
			// don't, we need to look up this lab's location.
			_RecordsetPtr rsLocationID = CreateParamRecordset("SELECT LocationID FROM LabsT WHERE ID = {INT}", nLabID);
			if(!rsLocationID->eof) {
				long nLocationID = AdoFldLong(rsLocationID, "LocationID");
				bool bAllowed = false;
				for(int i = 0; i < arAllowedLocationIDs.GetSize() && !bAllowed; i++) {
					if(arAllowedLocationIDs[i] == nLocationID) bAllowed = true;
				}
				if(!bAllowed) {
					MsgBox("You do not have permission to view this lab, due to the location to which it is assigned.\r\n"
						"Please see your office manager for assistance.");
					return;
				}
			}
		}

		GetMainFrame()->OpenLab(NULL, nPatientID, -1, ltInvalid, nLabID, -1, "", -1, FALSE, FALSE, GetSafeHwnd());
	}
}

// (c.haag 2008-02-07 13:34) - PLID 28853 - Included a parameter for consignment
void CToDoAlarmDlg::LoadInventoryScreen(long nID, BOOL bConsignment)
{
	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;

	if(p->FlipToModule(INVENTORY_MODULE_NAME)) {

		pView = (CNxTabView *)p->GetOpenView(INVENTORY_MODULE_NAME);
		if (pView) 
		{	if(pView->GetActiveTab()==1)
				pView->UpdateView();
			else
				pView->SetActiveTab(InventoryModule::OrderTab);

			if (IDYES == MessageBox("Close todo list and create an order?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
				OnOK();
				// (j.jones 2008-02-07 10:25) - PLID 28851 - Added a second parameter to create
				// auto orders as general or consignment. PLID 28853 will change todo alarms
				// to properly support this.
				((CInvOrderDlg *)pView->GetActiveSheet())->CreateAutoOrder(nID, bConsignment);
			}
		}
	}
}

// (d.thompson 2009-11-16) - PLID 36301 - Moved Export dialog to 'Links' module instead of 'Financial'
void CToDoAlarmDlg::LoadExportScreen(long nExportID)
{
	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;

	if(p->FlipToModule(LINKS_MODULE_NAME)) {

		pView = (CNxTabView *)p->GetOpenView(LINKS_MODULE_NAME);
		if (pView) 
		{	
			if(pView->GetActiveTab() == LinksModule::ExportTab)
				pView->UpdateView();
			else
				pView->SetActiveTab(LinksModule::ExportTab);

			((CExportDlg*)pView->GetActiveSheet())->LoadExport(nExportID);
		}
	}
}

void CToDoAlarmDlg::LoadPersonScreen(long nRow)
{
	try {
		TodoType tt = (TodoType)VarLong(m_List->GetValue(nRow,RegardingType));

		switch(tt) {
		case ttTrackingStep:
		case ttCustomRecord:
		case ttGlobalPeriod:
		case ttDecisionRule: // (j.gruber 2010-02-25 16:43) - PLID 37510
			LoadPatientScreen(VarLong(m_List->GetValue(nRow,PersonID)));
			break;

		// (z.manning 2008-10-13 11:19) - PLID 31667
		//(e.lally 2010-05-06) PLID 36567 - We will now go to this patient and open this lab up.
		case ttLabStep: 
		case ttLab: // (z.manning 2010-05-12 17:32) - PLID 37405
			LoadLabEntryScreen(VarLong(m_List->GetValue(nRow,PersonID)), tt, VarLong(m_List->GetValue(nRow,RegardingID)));
			break;

		case ttPatientContact:
			// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
			// (d.lange 2011-06-03 17:33) - PLID 43253 - Added default value
			if(ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}",VarLong(m_List->GetValue(nRow,PersonID), -1)))  {
				LoadPatientScreen(VarLong(m_List->GetValue(nRow,PersonID), -1));
			}
			else {
				LoadContactScreen(VarLong(m_List->GetValue(nRow,PersonID), -1));
			}
			break;
		// (c.haag 2008-02-07 12:33) - PLID 28853 - Include General and Consignment items
		case ttPurchasedInvItem:
		case ttConsignmentInvItem:
		{
			//(e.lally 2007-05-04) PLID 25253 - Account for null PersonID values
			long nPersonID = VarLong(m_List->GetValue(nRow,PersonID), -1);
			if(nPersonID == -1)
				break;
			// (c.haag 2008-02-07 13:35) - PLID 28853 - Specify the consignment boolean flag
			LoadInventoryScreen(VarLong(m_List->GetValue(nRow,PersonID)), (ttConsignmentInvItem == tt) ? TRUE : FALSE);
			break;
		}
		case ttLicenseCertification:
			LoadContactScreen(VarLong(m_List->GetValue(nRow,PersonID)));
			break;
		case ttExport:
			LoadExportScreen(VarLong(m_List->GetValue(nRow,RegardingID)));
			break;

		case ttEMN:
		case ttEMNDetail:
			HandleEmrTodoLink(tt, VarLong(m_List->GetValue(nRow,RegardingID), -1), VarLong(m_List->GetValue(nRow,PersonID)));
			break;

		// (c.haag 2010-05-24 9:48) - PLID 38731 - Handling for MailSent tasks
		case ttMailSent:
			HandleMailSentTodoLink(VarLong(m_List->GetValue(nRow,RegardingID)), VarLong(m_List->GetValue(nRow,PersonID)));
			break;

		default:
			//unknown type, do nothing
			break;
		}
	} NxCatchAll("Error in ToDoAlarmDlg::LoadPersonScreen");
}

void CToDoAlarmDlg::LoadTodoListScreen(int nID)
{
	CTaskEditDlg dlg(this);

	dlg.m_iTaskID = nID;

	long nRow = m_List->GetCurSel();
	if(nRow == -1)
		return;
	TodoType ttType = (TodoType)VarLong(m_List->GetValue(nRow, RegardingType));

	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	bool bIsPatient = ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", VarLong(m_List->GetValue(nRow,PersonID),-1))?true:false;

	// (a.walling 2008-07-07 17:49) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	dlg.m_nPersonID = VarLong(m_List->GetValue(nRow,PersonID),-1);

	//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields 
	dlg.m_bIsNew = FALSE;

	if (dlg.DoModal(bIsPatient) == IDOK) {
		if(ttType == ttTrackingStep) {
			PhaseTracking::SyncLadderWithTodo(nID);
		}
		else if(ttType == ttLabStep) {
			// (z.manning 2008-10-29 12:13) - PLID 31667 - Update labs
			SyncLabWithTodo(nID);
		}
		SetWindowText(TITLE_BASE_STRING + CString(" - Loading..."));
		m_List->Requery();
		//clear our needs requery flag
		m_bNeedsRequery = false;
	}
}

void CToDoAlarmDlg::OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags) 
{

	try 
	{
		if (nRow < 0)  //they clicked white space, so do nothing
			return;

//Load the person regardless of column
		switch (nCol)
		{	case Name:
				LoadPersonScreen(nRow);
				break;
			case HomePhone:	
			case Email:
			case Notes:
			case CategoryID:
			case Task:
			case Priority:
			case Deadline:
			case EnteredBy:
//				LoadTodoListScreen(VarLong(m_List->Value[nRow][TaskID]));
				break;
		}
	}NxCatchAll("Could not load todo alarm item");
}

void CToDoAlarmDlg::OnDblClickCellList(long nRowIndex, short nColIndex) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		if(nRowIndex == -1)
			return;

		long nCurUserID = m_Combo->GetCurSel();

		if(nCurUserID == -1)
			return;

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			return;
		*/
		
		if (VarLong(m_Combo->GetValue(nCurUserID, 0)) == GetCurrentUserID())
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite))
				return;
		}
		else
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite))
				return;
		}

		LoadTodoListScreen(VarLong(m_List->Value[nRowIndex][TaskID]));
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnClickRadioMinute() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_minuteBtn.SetCheck(1);
		m_hourBtn.SetCheck(0);
		m_neverBtn.SetCheck(0);

		m_minuteSpin.EnableWindow(true);
		m_hourSpin.EnableWindow(false);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnClickRadioHour() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_minuteBtn.SetCheck(0);
		m_hourBtn.SetCheck(1);
		m_neverBtn.SetCheck(0);

		m_minuteSpin.EnableWindow(false);
		m_hourSpin.EnableWindow(true);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnClickRadioDontremind() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_minuteBtn.SetCheck(0);
		m_hourBtn.SetCheck(0);
		m_neverBtn.SetCheck(1);

		m_minuteSpin.EnableWindow(false);
		m_hourSpin.EnableWindow(false);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	switch (nType) {
	// (j.fouts 2012-05-07 10:14) - PLID 49858 - Refresh the positions and sizes on Maximize aswell
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		{
			/* Get the previous position erased */
			// (a.walling 2008-05-23 14:06) - PLID 30099
#ifdef NXDIALOG_NOCLIPCHILDEN
			InvalidateRect( &m_rPrev );
#endif

			/* Calculate the new position of the size grip */
			m_rPrev.right = cx;
			m_rPrev.bottom = cy;
			m_rPrev.top = m_rPrev.bottom - GetSystemMetrics( SM_CYHSCROLL );
			m_rPrev.left = m_rPrev.right - GetSystemMetrics( SM_CXVSCROLL ); 
			
			/* Have the size grip re-painted */
			// (a.walling 2008-05-23 14:06) - PLID 30099
#ifdef NXDIALOG_NOCLIPCHILDEN
			InvalidateRect( &m_rPrev );
#endif

			CRect rc;

			CWnd *pList = GetDlgItem(IDC_LIST);
			if (pList) {
				pList->SetWindowPos(0, 0, 0, cx-m_nListWidthAdj, cy-m_nListHeightAdj, 
					SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
				pList->GetWindowRect(rc);
				ScreenToClient(rc);

				//move remember col settings button
				CWnd *pRememberList = GetDlgItem(IDC_REMEMBER_COLUMN_SETTINGS);
				if (pRememberList) {
					CRect rcRemember;
					pRememberList->GetWindowRect(rcRemember);
					ScreenToClient(rcRemember);
					//pRememberList->SetWindowPos(0, 0, 0, cx-rcRemember.Width(), rc.bottom, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
					pRememberList->SetWindowPos(NULL, rc.right-rcRemember.Width(), rc.bottom + 5, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
				}
			}
			
			CWnd *pOkayButton = GetDlgItem(IDOK);
			if (pOkayButton) {
				pOkayButton->SetWindowPos(0, cx-m_nOkayXOffsetFromRight, m_nOkayYOffsetFromTop, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
			}

			CWnd *pPrintButton = GetDlgItem(IDC_BTN_PRINT_TODOS);
			GetDlgItem(IDOK)->GetWindowRect(rc);
			ScreenToClient(rc);
			if (pPrintButton) {
				pPrintButton->SetWindowPos(0, cx-m_nOkayXOffsetFromRight, rc.bottom + 5, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
			}
			
			CWnd *pTitleLabel = GetDlgItem(IDC_LABEL_DATE);
			if (pTitleLabel) {
				pTitleLabel->SetWindowPos(0, cx/2-m_nTitleLabelWidth/2, m_nTitleLabelYFromTop, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
			}

			
			// (a.walling 2008-05-23 14:06) - PLID 30099
#ifndef NXDIALOG_NOCLIPCHILDEN
			//Invalidate(); // like NxDialog
			// (a.walling 2008-11-21 12:54) - PLID 31956 - Need to erase when invalidating
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
#endif
		}
		break;
	case SIZE_MINIMIZED:
	case SIZE_MAXSHOW:
	case SIZE_MAXHIDE:
	default:
		Save();
		break;
	}
}

void CToDoAlarmDlg::OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		//(c.copits 2011-02-18) PLID 40794 - Permissions for individual todo alarm fields
		if (!CheckIndividualPermissions(nCol)) {
			*pbContinue = FALSE;
		}

		//TES 12/16/2009 - PLID 30960 - Need to check write permissions before modifying an item.
		if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID())
		{
			if(!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT___W________ANDPASS))) {
				*pbContinue = FALSE;
			}
		}
		else
		{
			if(!(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT___W________ANDPASS))) {
				*pbContinue = FALSE;
			}
		}

	}NxCatchAll(__FUNCTION__);
	
}

void CToDoAlarmDlg::OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		switch(nCol) {
		// (a.walling 2006-11-01 17:05) - PLID 23320 - Prevent the SQL error to begin with by limiting text to 2000 chars.
		case Notes:
			{
				// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
				CString strNewValue(pvarNewValue->bstrVal);
				if(strNewValue.GetLength() > 2000) {
					MsgBox("The text you entered is longer than the maximum amount (2000) and has been shortened.\n"
						"Please double-check your note and make changes as needed.");
					::VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(strNewValue.Left(2000)).Detach();
				}
				break;
			}

		case Deadline:
			{
				COleDateTime dtMin(1900,1,1,0,0,0), dtMax(5000,12,31,0,0,0);
				if(pvarNewValue->vt != VT_DATE || VarDateTime(pvarNewValue) < dtMin || VarDateTime(pvarNewValue) > dtMax) {
					*pbCommit = FALSE;
					MessageBox("You have entered an invalid deadline date.");
				}
				break;
			}

		default:
			break;
		}
	} NxCatchAll("Error in CToDoAlarmDlg::OnEditingFinishingList()");
}

void CToDoAlarmDlg::OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//(e.lally 2009-11-17) PLID 36304 - Moved try to top
	try{
		if(!bCommit)
			return;

		if (varNewValue.vt == VT_EMPTY)
			return;

		// (a.walling 2006-11-01 12:59) - PLID 23309 Although the issue is fixed if we just return if either of these
		// are null, this way we make sure that new values are never prevented from being set.
		VARIANT varSafeOldValue = varOldValue;
		if (varOldValue.vt == VT_EMPTY) {
			varSafeOldValue = g_cvarNull;
		}

		long nAuditID = -1, nPatientID = -1;
		CString strOld, strNew, strSQL, strPatientName;
		_RecordsetPtr rsPatient;
		switch(nCol) {
		case Notes:
			if (varSafeOldValue.vt == VT_EMPTY || varSafeOldValue.vt == VT_NULL || VarString(varSafeOldValue) != VarString(varNewValue)) {
				ExecuteSql("UPDATE ToDoList SET Notes = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), VarLong(m_List->GetValue(nRow, TaskID)));
				//auditing
				nAuditID = BeginNewAuditEvent();
				strOld.Format("Note: %s", VarString(varSafeOldValue));
				strNew.Format("Note: %s", VarString(varNewValue));
				//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
				nPatientID = VarLong(m_List->GetValue(nRow, 1), -1);
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				if(nPatientID == -1)
					strPatientName = "<No Supplier>";
				else{
					rsPatient = CreateParamRecordset("SELECT First, Middle, Last "
											"FROM PersonT "
											"WHERE ID = {INT}", nPatientID);
					strPatientName.Format("%s, %s %s", AdoFldString(rsPatient, "Last", ""), AdoFldString(rsPatient, "First", ""), AdoFldString(rsPatient, "Middle", ""));
				}
				// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPatientID))		
					AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
				else
					AuditEvent(-1, strPatientName, nAuditID, aeiContactToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
			}
			break;

		case CategoryID:
			if (varSafeOldValue.vt == VT_EMPTY || varSafeOldValue.vt == VT_NULL || VarLong(varSafeOldValue) != VarLong(varNewValue)) {
				CString strID;
				if(VarLong(varNewValue) == -1) 
					strID.Format("NULL");
				else
					strID.Format("%li", VarLong(varNewValue));
				ExecuteSql("UPDATE ToDoList SET CategoryID = %s WHERE TaskID = %li", strID, VarLong(m_List->GetValue(nRow, TaskID)));
				//for auditing
				_RecordsetPtr rsTaskTypeNew = CreateParamRecordset("SELECT Description FROM NoteCatsF "
					"WHERE ID = {INT}", VarLong(varNewValue));
				if (!rsTaskTypeNew->eof)
					strNew.Format("Category: %s", AdoFldString(rsTaskTypeNew, "Description", ""));
				else
					strNew = "Category: <None>";
			}
			else
				strNew = "Category: <None>";

			//auditing
			if(varSafeOldValue.vt == VT_I4 && varSafeOldValue.lVal != -1) {
				_RecordsetPtr rsTaskTypeOld = CreateParamRecordset("SELECT Description FROM NoteCatsF "
					"WHERE ID = {INT}", VarLong(varSafeOldValue));
				if (!rsTaskTypeOld->eof)
					strOld.Format("Category: %s", AdoFldString(rsTaskTypeOld, "Description", ""));
			}
			else
				strOld = "Category: <None>";
			nAuditID = BeginNewAuditEvent();
			//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
			nPatientID = VarLong(m_List->GetValue(nRow, 1), -1);
			if(nPatientID == -1){
				strPatientName = "<No Supplier>";
			}
			else{
				rsPatient = CreateParamRecordset("SELECT First, Middle, Last "
										"FROM PersonT "
										"WHERE ID = {INT}", nPatientID);
				//This recordset shouldn't be eof, but we need to put in a check anyways.
				if(rsPatient->eof)
					strPatientName = "<Unknown>";
				strPatientName.Format("%s, %s %s", AdoFldString(rsPatient, "Last", ""), AdoFldString(rsPatient, "First", ""), AdoFldString(rsPatient, "Middle", ""));
			}
			// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
			if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPatientID))		
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
			else
				AuditEvent(-1, strPatientName, nAuditID, aeiContactToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
			break;

		case Task:
			if (varSafeOldValue.vt == VT_EMPTY || varSafeOldValue.vt == VT_NULL || VarString(varSafeOldValue) != VarString(varNewValue)) {
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Get old value for auditing
				_RecordsetPtr rsMethodTypeOld = CreateParamRecordset("SELECT Task FROM ToDoList "
					"WHERE TaskID = {INT}", VarLong(m_List->GetValue(nRow, TaskID)));
				if (!rsMethodTypeOld->eof) {
					strOld.Format("Method: %s", AdoFldString(rsMethodTypeOld, "Task", ""));
					if (Trim(AdoFldString(rsMethodTypeOld, "Task", "")) == "") {
						strOld.Format("Method: <none>");
					}
				}
				else {
					strOld.Format("Method: <none>");
				}

				//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
				// Audit new value
				strNew.Format("Method: %s", _Q(VarString(varNewValue)));
				nPatientID = VarLong(m_List->GetValue(nRow, 1), -1);
				if(nPatientID == -1)
					strPatientName = "<No Supplier>";
				else {
					rsPatient = CreateParamRecordset("SELECT First, Middle, Last "
											"FROM PersonT "
											"WHERE ID = {INT}", nPatientID);
					strPatientName.Format("%s, %s %s", 
						AdoFldString(rsPatient, "Last", ""), 
						AdoFldString(rsPatient, "First", ""), 
						AdoFldString(rsPatient, "Middle", ""));
				}

				nAuditID = BeginNewAuditEvent();
				if (strOld != strNew) {
					AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientToDoChanged, VarLong(m_List->GetValue(nRow, TaskID)), strOld, strNew, aepMedium, aetChanged);
				}

					ExecuteSql("UPDATE ToDoList SET Task = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), VarLong(m_List->GetValue(nRow, TaskID)));
				}
			break;

		case Priority:
			if (varSafeOldValue.vt == VT_EMPTY || varSafeOldValue.vt == VT_NULL || VarShort(varSafeOldValue) != VarShort(varNewValue)) {
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Get old value for auditing
				_RecordsetPtr rsPriorityOld = CreateParamRecordset("SELECT Priority FROM ToDoList "
					"WHERE TaskID = {INT}", VarLong(m_List->GetValue(nRow, TaskID)));
				if (!rsPriorityOld->eof) {
					strOld.Format("Priority: %s", StrPriorityString(AdoFldByte(rsPriorityOld, "Priority", -1)));
				}
				else {
					strOld.Format("Priority: <none>");
				}

				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Audit new value
				strNew.Format("Priority: %s", _Q(StrPriorityString(VarShort(varNewValue))));
				nAuditID = BeginNewAuditEvent();
				nPatientID = VarLong(m_List->GetValue(nRow, 1), -1);
				if(nPatientID == -1)
					strPatientName = "<No Supplier>";
				else {
					rsPatient = CreateParamRecordset("SELECT First, Middle, Last "
											"FROM PersonT "
											"WHERE ID = {INT}", nPatientID);
					strPatientName.Format("%s, %s %s", 
						AdoFldString(rsPatient, "Last", ""), 
						AdoFldString(rsPatient, "First", ""), 
						AdoFldString(rsPatient, "Middle", ""));
				}
				if (strOld != strNew) {
					AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientToDoChanged, VarLong(m_List->GetValue(nRow, TaskID)), strOld, strNew, aepMedium, aetChanged);
				}

				ExecuteSql("UPDATE ToDoList SET Priority = %li WHERE TaskID = %li", VarShort(varNewValue), VarLong(m_List->GetValue(nRow, TaskID)));
			}
			break;

		case Deadline:
			if (varSafeOldValue.vt == VT_EMPTY || varSafeOldValue.vt == VT_NULL || VarDateTime(varSafeOldValue) != VarDateTime(varNewValue)) {
				ExecuteSql("UPDATE ToDoList SET Deadline = '%s' WHERE TaskID = %li", FormatDateTimeForSql(varNewValue, dtoDate), VarLong(m_List->GetValue(nRow, TaskID)));
				//auditing
				nAuditID = BeginNewAuditEvent();
				strOld.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varSafeOldValue), dtoDate));
				strNew.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varNewValue), dtoDate));
			//(e.lally 2007-05-04) PLID 25253 - Allow ToDo to be created when no supplier exists. Account for null person IDs
			nPatientID = VarLong(m_List->GetValue(nRow, 1), -1);
			if(nPatientID == -1)
				strPatientName = "<No Supplier>";
			else{
				rsPatient = CreateParamRecordset("SELECT First, Middle, Last "
										"FROM PersonT "
										"WHERE ID = {INT}", nPatientID);
				strPatientName.Format("%s, %s %s", AdoFldString(rsPatient, "Last", ""), AdoFldString(rsPatient, "First", ""), AdoFldString(rsPatient, "Middle", ""));
			}
			// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
			if (ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", nPatientID))		
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
			else
				AuditEvent(-1, strPatientName, nAuditID, aeiContactToDoChanged, TaskID, strOld, strNew, aepMedium, aetChanged);
			}
			break;
		}

		// (z.manning 2008-11-17 15:24) - PLID 32056 - Only send a table checker if something actually changed!
		if(!(COleVariant(varOldValue) == COleVariant(varNewValue))) {
			// (s.tullis 2014-09-08 09:10) - PLID 63344 
			long nTaskID = VarLong(m_List->GetValue(nRow, TaskID));
			long nPersonID = VarLong(m_List->GetValue(nRow, PersonID), -1);
			CString strAssignedIDs = VarString(m_List->GetValue(nRow, AssignedIDs), "");

			CArray < long, long> arrAssignedID;
			ParseDelimitedStringToLongArray(strAssignedIDs, " ", arrAssignedID);

			if (arrAssignedID.GetSize() == 1){
				CClient::RefreshTodoTable(nTaskID, nPersonID, arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
			}
			else{
				CClient::RefreshTodoTable(nTaskID, nPersonID, -1, TableCheckerDetailIndex::tddisChanged);
			}

		}
	} NxCatchAll("Error in CToDoAlarmDlg::OnEditingFinishedList()");
}

void CToDoAlarmDlg::OnDestroy() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		Save();
	}NxCatchAll(__FUNCTION__);
	
	CDialog::OnDestroy();
}

void CToDoAlarmDlg::OnRequeryFinishedList(short nFlags) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		switch (nFlags) {
		case dlRequeryFinishedCompleted:
			{
				CString strTitle;
				strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
				SetWindowText(strTitle);
			}
			break;
		case dlRequeryFinishedCanceled:
			SetWindowText(TITLE_BASE_STRING + CString(" - Partial listing"));
			break;
		case dlRequeryFinishedError:
			SetWindowText(TITLE_BASE_STRING + CString(" - Error loading"));
			break;
		}

		SetColumnSizes();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::ColorizeList(OPTIONAL long nRow /* = -1 */)
{
	const int nPriorityAdj = 10;

	try {

		bool bColorize = (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1);

		if (!bColorize && m_bListColored) {
			// color everything white (remove the colour)
			for (int i = 0; i < m_List->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_List->GetRow(i);
				pRow->PutBackColor(RGB(255, 255, 255));
			}
			m_bListColored = false;
			return;
		}
		else if (!bColorize && !m_bListColored) {
			// don't colorize, list has not been colorized, so don't do anything
			return;
		}

		m_bListColored = true;

		if (nRow < 0) { // color the whole list
			for (int i = 0; i < m_List->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_List->GetRow(i);
				if (pRow) {
					ColorizeItem(pRow);
				}
			}
		}
		else // just this row...if 
		{
			IRowSettingsPtr pRow = m_List->GetRow(nRow);
			if (pRow) {
				ColorizeItem(pRow);
			}
		}
	} NxCatchAll("Error in CToDoAlarmDlg::ColorizeList");
}

void CToDoAlarmDlg::ColorizeItem(IRowSettingsPtr &pRow)
{
	try {
		if (pRow) {
			_variant_t varCompleted = pRow->GetValue(Completed);

			if (varCompleted.vt == VT_NULL) {
				int nPriority = VarByte(pRow->GetValue(Priority), 0);

				COLORREF colorAdj;
				switch(nPriority) {
					case 1: colorAdj = m_colorIncompleteHigh; break;
					case 2: colorAdj = m_colorIncompleteMedium; break;
					case 3: default: colorAdj = m_colorIncompleteLow; break;
				}

				pRow->PutBackColor(colorAdj);
			}
			else {
				pRow->PutBackColor(m_colorComplete);
			}
		}
	} NxCatchAll("Error in CToDoAlarmDlg::ColorizeItem");
}

// (j.jones 2008-09-30 14:24) - PLID 31331 - added appt. tablechecker support
LRESULT CToDoAlarmDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {

			case NetUtils::AppointmentsT: {

				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
				long nAppointmentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);
				// (j.jones 2014-08-05 11:05) - PLID 63187 - if we are not provided an ID, do nothing
				if (nAppointmentID != -1) {
					TryUpdateTodoListByAppointmentID(nAppointmentID);
				}
				break;
			}

			// (j.jones 2014-09-02 09:29) - PLID 63187 - added todo ex tablecheckers
			case NetUtils::TodoList: {

				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nTaskID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTaskID), -1);

				//this is handled by a modular function
				HandleTodoTablechecker(nTaskID, pDetails);
				return 0;
				break;
			}			
		}

	} NxCatchAll("Error in CToDoAlarmDlg::OnTableChangedEx");

	return 0;
}

LRESULT CToDoAlarmDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//DRT 10/15/2003 - PLID 6005 - Refresh the list when we are told the users have changed.
		switch(wParam) {
		case NetUtils::Coordinators:
			{
				try {
					long nCurSel = m_Combo->CurSel;
					long nCurID = -1;
					if(nCurSel > -1) {
						nCurID = VarLong(m_Combo->GetValue(nCurSel, 0));
						m_Combo->Requery();

						//try to reset the selection
						long nRow = m_Combo->SetSelByColumn(0, _variant_t(nCurID));
						if(nRow > -1) {
							OnSelChosenCombo(nRow);//Update the where clause and requery
						}
						else {
							//whoever they're currently looking at is gone.  Tell them so and reset to the first person
							MessageBox("The user you were viewing has been removed.  Your selection will reset to the first user in the list.");
							m_Combo->PutCurSel(0);
							OnSelChosenCombo(0);
						}
					}
					else {
						//nothing is selected, so requery and leave nothing selected.  This should really never happen
						m_Combo->Requery();
					}

				} NxCatchAll("Error in CToDoAlarmDlg::OnTableChanged:Coordinators");

				return 0;	//to avoid the below wierdness
			}
			break;

		// (j.jones 2014-08-12 11:03) - PLID 63187 - fixed to actually check for NetUtils::TodoList
		case NetUtils::TodoList:
		{
			//the TaskID is in the lParam, so get that first
			long nTaskID = lParam;

			// (j.jones 2014-09-02 09:34) - PLID 63187 - moved this code to a modular function,
			// sending NULL for tablechecker details as this is a non-EX tablechecker
			HandleTodoTablechecker(nTaskID, NULL);			
			return 0;
			break;
		}
		default:
			break;
		};
	} NxCatchAll("Error in CToDoAlarmDlg::OnTableChanged");

	return 0;		
}

// (j.jones 2014-09-02 09:32) - PLID 63187 - unified todo tablechecker code into one function,
// pDetails will be null for non-EX tablecheckers
void CToDoAlarmDlg::HandleTodoTablechecker(long nTaskID, CTableCheckerDetails* pDetails)
{
	//throw exceptions to the caller

	if (nTaskID == -1) {

		if (m_Combo->CurSel != -1)
		{
			// Find out if our currently selected contact still exists
			long nContactID = VarLong(m_Combo->GetValue(m_Combo->CurSel, 0));
			_RecordsetPtr prsUsers = CreateParamRecordset("SELECT PersonID FROM UsersT WHERE PersonID = {INT}", nContactID);
			if (prsUsers->eof)
			{
				// Nope; we need to warn the user
				MessageBox("The contact you are currently reviewing To-Do reminders for has been deleted. The first available contact in the list will now be selected.");

				// Switch the current selection
				if (m_Combo->CurSel == 0)
				{
					if (m_Combo->GetRowCount() == 1)
						m_Combo->CurSel = -1;
					else
						m_Combo->CurSel = 1;
				}
				else
					m_Combo->CurSel = 0;

				// Delete the contact from the list
				m_Combo->RemoveRow(m_Combo->FindByColumn(0, nContactID, 0, FALSE));

				GenerateWhereClause();
			}
			else
			{
				// Ok, so our contact exists...but we may have added or removed another contact.
				// So, we should requery the contacts combo.
				m_Combo->Requery();
				m_Combo->TrySetSelByColumn(0, nContactID);
			}
		}

		// (j.jones 2014-08-12 11:22) - PLID 63187 - don't requery if we're already requerying
		if (m_List->IsRequerying()) {
			return;
		}

		// (j.jones 2014-08-12 11:49) - PLID 63187 - only do a full requery
		// if our todo list is actually visible (including if it is minimized)
		if (GetSafeHwnd() && IsWindowVisible()) {
			//refresh the whole list
			m_List->Requery();
			//clear our needs requery flag
			m_bNeedsRequery = false;
		}
		else {
			//flag that the todo list needs to be requeried the next time it is shown
			m_bNeedsRequery = true;
		}
	}
	else {

		// (c.haag 2003-07-31 11:35) - Preferred contacts:
		// 1 = Home  2 = Work 3 = Cell

		// (c.haag 2003-09-04 16:01) - For some odd reason, the current selection can be -1 sometimes.
		if (m_Combo->GetCurSel() == -1)
		{
			m_Combo->TrySetSelByColumn(0, GetCurrentUserID());
			if (-1 == m_Combo->CurSel)
			{
				// This should never happen, but we should be safe
				if (m_Combo->GetRowCount() == 0) {
					return;
				}
				m_Combo->CurSel = 0;
			}
		}

		// (a.walling 2006-08-03 09:52) - PLID 21758 - Do not update the list if we are requerying anyway
		// (this could happen two or three times even while the data list is being populated. why? who knows)
		if (m_List->IsRequerying()) {
			return;
		}

		// (j.jones 2014-09-02 09:37) - PLID 63187 - if this was called by an EX tablechecker,
		// find out if the assigned user ID is our filtered user
		if (pDetails) {
			long nAssignedUserID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiAssignedUser), -1);

			long nCurSel = m_Combo->CurSel;
			long nCurUserID = -1;
			if (nCurSel > -1) {
				nCurUserID = VarLong(m_Combo->GetValue(nCurSel, 0));
			}

			//-1 means multiple users, so we only want to check to see if it is
			//one user ID, and not who we are filtering on
			// (s.tullis 2015-02-20 10:28) - PLID 64602 - Reassigning a Todo task to another user sends a tablechecker for the new user only, and is never removed from the old user's To-Do Alarm unless they close and reopen.
			if (nAssignedUserID != -1 && nCurUserID != -1 && nAssignedUserID != nCurUserID) {
				long nTodoListRowIndex = -1;
				// We need to see if the todo exists in the todolist
				nTodoListRowIndex = m_List->FindByColumn(TODO_COLUMN_TYPE::TaskID, nTaskID, -1, FALSE);
				if (nTodoListRowIndex == -1){
					//this todo is for another user, so ignore it
					return;
				}
			}
		}

		// (j.jones 2014-09-02 09:37) - PLID 63187 - if this was called by an EX tablechecker,
		// find out if the todo was deleted, and if so, just remove it now
		if (pDetails) {
			TableCheckerDetailIndex::Todo_Status eTodoStatus
				= (TableCheckerDetailIndex::Todo_Status)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTodoStatus), (long)TableCheckerDetailIndex::Todo_Status::tddisChanged);

			if (eTodoStatus == TableCheckerDetailIndex::Todo_Status::tddisDeleted) {

				//find and remove the row, if it exists
				long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

				if (nRow != -1) {

					//it is there, so remove it
					m_List->RemoveRow(nRow);

					CString strTitle;
					strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
					SetWindowText(strTitle);
				}
			}
		}

		//Get the item out of the data and either update, delete, or add it to the datalist
		// (a.walling 2006-08-03 09:20) - PLID 21758 Fixed EnteredBy and the query in general. this should always
		//  return the same records as the data list, except filtered on the TaskID and AssignTo.
		// (c.haag 2008-06-10 09:32) - PLID 11599 - Use TodoAssignToT
		// (z.manning 2008-11-17 14:13) - PLID 32056 - I changed this query to be consistent with the one
		// in the datalist. The main change was to basically put the entire query into a subquery called
		// TodoListQ so that the where clause will filter on the queries values rather than the actual
		// table values for fields such as Remind where we don't pull the the fields exactly as it is in
		// data.
		// (j.gruber 2010-02-24 13:03) - PLID 37510 - Clinical Decision Support Rules
		CString strWhere;
		if (!m_strWhere.IsEmpty()) {
			strWhere = "WHERE " + m_strWhere;
		}

		// (j.fouts 2012-04-30 11:12) - PLID 49859 - Removed HomePhone and Email and Added PreferredContact
		// (j.fouts 2012-06-05 12:06) - PLID 49859 - Added HomePhone and Email back in
		// (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support need assignedIDs
		_RecordsetPtr rsTask = CreateParamRecordset(
			"SELECT * FROM \r\n"
			"	(SELECT ToDoList.TaskID, PersonT.ID AS PersonID, \r\n"
			"		(CASE WHEN (SupplierT.PersonID IS NULL) THEN [Last] + ', ' + [First] ELSE Company END) AS Name, \r\n"
			"		ToDoList.CategoryID, ToDoList.[Task], ToDoList.Priority, ToDoList.Notes, ToDoList.Deadline, \r\n"
			"		(CASE WHEN(UsersT.UserName Is NULL) THEN CASE WHEN RegardingType = 3 THEN '<Tracking>' WHEN RegardingType = 5 THEN '<Custom Record>' WHEN RegardingType = 6 THEN '<Billing>' WHEN RegardingType = 7 THEN '<Export>' WHEN RegardingType = 14 THEN '<Clinical Decision Support Rules>' ELSE '' END ELSE UsersT.UserName END) AS EnteredBy, \r\n"
			"		CASE WHEN PersonT.HomePhone IS NULL THEN '' ELSE PersonT.HomePhone END AS HomePhone, \r\n"
			"		CASE WHEN PreferredContact = 1 THEN (CASE WHEN HomePhone IS NULL THEN '' ELSE HomePhone END) WHEN PreferredContact = 2 THEN (CASE WHEN WorkPhone IS NULL THEN '' ELSE WorkPhone END) WHEN PreferredContact = 3 THEN (CASE WHEN CellPhone IS NULL THEN '' ELSE CellPhone END) WHEN PReferredContact = 4 THEN (CASE WHEN Pager IS NULL THEN ''ELSE Pager END) WHEN PreferredContact = 5 THEN (CASE WHEN OtherPhone IS NULL THEN ''ELSE OtherPhone END) WHEN PreferredContact = 6 THEN (CASE WHEN Email IS NULL THEN '' ELSE Email END) END AS PreferredPhone, \r\n"
			"		CASE WHEN PersonT.Email IS NULL THEN '' ELSE PersonT.Email END AS Email, \r\n"
			"		(CASE WHEN PatientsT.PreferredContact = 1 "
			"			THEN PersonT.HomePhone + ' (Home)' WHEN PatientsT.PreferredContact = 2 "
			"			THEN PersonT.WorkPhone + ' (Work)' WHEN PatientsT.PreferredContact = 3 "
			"			THEN PersonT.CellPhone + ' (Mobile)' WHEN PatientsT.PreferredContact = 4 "
			"			THEN PersonT.Pager + ' (Pager)' WHEN PatientsT.PreferredContact = 5 "
			"			THEN PersonT.OtherPhone + ' (Other)' WHEN PatientsT.PreferredContact = 6 "
			"			THEN PersonT.Email WHEN PatientsT.PreferredContact = 7 "
			"			THEN PersonT.CellPhone + ' (Text Message)' "
			"			ELSE CASE WHEN PersonT.HomePhone NOT LIKE '' "
			"			THEN PersonT.HomePhone + ' (Home)' WHEN PersonT.WorkPhone NOT LIKE '' "
			"			THEN PersonT.WorkPhone + ' (Work)' WHEN PersonT.CellPhone NOT LIKE '' "
			"			THEN PersonT.CellPhone + ' (Mobile)' WHEN PersonT.Pager NOT LIKE '' "
			"			THEN PersonT.Pager + ' (Pager)' WHEN PersonT.OtherPhone NOT LIKE '' "
			"			THEN PersonT.OtherPhone + ' (Other)' WHEN PersonT.Email NOT LIKE '' "
			"			THEN PersonT.Email END END) AS PreferredContact, \r\n"
			"		ToDoList.RegardingType AS RegardingType, ToDoList.Done AS Done, \r\n"
			"		(CASE WHEN ToDoList.Done IS NULL THEN ToDoList.Remind ELSE NULL END) as Remind, \r\n"
			"		RegardingID, \r\n"
			"		dbo.GetTodoAssignToIDString(TodoList.TaskID) as AssignedUserIDs \r\n"	
			"	FROM ToDoList \r\n"
			"	LEFT JOIN PersonT ON ToDoList.PersonID = PersonT.[ID] \r\n"
			"	LEFT JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID \r\n"
			"	LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID \r\n"
			"	LEFT JOIN PatientsT ON PatientsT.PersonID = PersonT.ID \r\n"
			"	WHERE ToDoList.TaskID Is Not Null AND (CurrentStatus Is Null OR CurrentStatus <> 4) \r\n"
			"		AND (TaskID = {INT})) AS TodoListQ \r\n"
			"{CONST_STRING} \r\n",
			nTaskID, strWhere);

		FieldsPtr fields = rsTask->Fields;

		if (rsTask->eof) {

			//the task is not in the datalist at the moment and shouldn't be, try to remove it if it is there
			long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

			if (nRow != -1) {

				//it is there, so remove it
				m_List->RemoveRow(nRow);

				CString strTitle;
				strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
				SetWindowText(strTitle);

			}
			else {

				//it is not there and it shouldn't be there, so we are all good
			}

		}
		else {

			//there is something in the recordset, so we have to determine whether to add it or just update it
			long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

			if (nRow == -1) {

				//the row is not there, so just add it
				IRowSettingsPtr pRow = m_List->GetRow(-1);

				pRow->PutValue(TaskID, nTaskID);
				pRow->PutValue(PersonID, fields->Item["PersonID"]->Value);
				pRow->PutValue(Name, fields->Item["Name"]->Value);
				// (j.fouts 2012-06-05 12:08) - PLID 49859 - Added These back in
				pRow->PutValue(HomePhone, fields->Item["HomePhone"]->Value);
				pRow->PutValue(PreferredPhone, fields->Item["PreferredPhone"]->Value);
				pRow->PutValue(Email, fields->Item["Email"]->Value);
				// (j.fouts 2012-04-30 11:14) - PLID 49859 - Consolidated Phone and Email into a PreferredContact
				pRow->PutValue(PreferredContact, fields->Item["PreferredContact"]->Value);
				pRow->PutValue(Notes, fields->Item["Notes"]->Value);
				pRow->PutValue(CategoryID, fields->Item["CategoryID"]->Value);
				pRow->PutValue(Task, fields->Item["Task"]->Value);
				pRow->PutValue(Priority, (short)fields->Item["Priority"]->Value);
				pRow->PutValue(Deadline, fields->Item["Deadline"]->Value);
				pRow->PutValue(EnteredBy, fields->Item["EnteredBy"]->Value);
				pRow->PutValue(Completed, fields->Item["Done"]->Value);
				pRow->PutValue(Remind, fields->Item["Remind"]->Value);
				pRow->PutValue(RegardingID, fields->Item["RegardingID"]->Value);
				pRow->PutValue(AssignedIDs, fields->Item["AssignedUserIDs"]->Value); // (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support

				// (a.walling 2006-09-05 10:59) - PLID 22387 - This should never be null or empty!
				if ((fields->Item["RegardingType"]->Value.vt == VT_NULL) || (fields->Item["RegardingType"]->Value.vt == VT_EMPTY))
				{
					ASSERT(false);
					pRow->PutValue(RegardingType, (long)ttPatientContact);
				}
				else {
					pRow->PutValue(RegardingType, fields->Item["RegardingType"]->Value);
				}

				if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) {
					ColorizeItem(pRow);
				}

				//add the row to the datalist
				m_List->AddRow(pRow);

				CString strTitle;
				strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
				SetWindowText(strTitle);
			}
			else {

				//the row is already there, we just have to update the values in it
				m_List->PutValue(nRow, TaskID, nTaskID);
				m_List->PutValue(nRow, PersonID, fields->Item["PersonID"]->Value);
				m_List->PutValue(nRow, Name, fields->Item["Name"]->Value);
				// (j.fouts 2012-06-05 12:08) - PLID 49859 - Added These back in
				m_List->PutValue(nRow, HomePhone, fields->Item["HomePhone"]->Value);
				// (a.walling 2006-10-16 10:32) - PLID 21758 - Update preferred phone
				m_List->PutValue(nRow, PreferredPhone, fields->Item["PreferredPhone"]->Value);
				m_List->PutValue(nRow, Email, fields->Item["Email"]->Value);
				// (j.fouts 2012-04-30 11:14) - PLID 49859 - Consolidated Phone and Email into a PreferredContact
				m_List->PutValue(nRow, PreferredContact, fields->Item["PreferredContact"]->Value);
				m_List->PutValue(nRow, Notes, fields->Item["Notes"]->Value);
				m_List->PutValue(nRow, CategoryID, fields->Item["CategoryID"]->Value);
				m_List->PutValue(nRow, Task, fields->Item["Task"]->Value);
				m_List->PutValue(nRow, Priority, (short)fields->Item["Priority"]->Value);
				m_List->PutValue(nRow, Deadline, fields->Item["Deadline"]->Value);
				m_List->PutValue(nRow, EnteredBy, fields->Item["EnteredBy"]->Value);
				m_List->PutValue(nRow, Completed, fields->Item["Done"]->Value);
				m_List->PutValue(nRow, Remind, fields->Item["Remind"]->Value);
				m_List->PutValue(nRow, AssignedIDs, fields->Item["AssignedUserIDs"]->Value); // (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support

				// (a.walling 2006-09-05 10:59) - PLID 22387 - This should never be null or empty!
				if ((fields->Item["RegardingType"]->Value.vt == VT_NULL) || (fields->Item["RegardingType"]->Value.vt == VT_EMPTY))
				{
					ASSERT(false);
					m_List->PutValue(nRow, RegardingType, (long)ttPatientContact);
				}
				else {
					m_List->PutValue(nRow, RegardingType, fields->Item["RegardingType"]->Value);
				}

				// (a.walling 2006-10-16 10:33) - PLID 21758 - Colorize the row
				if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) {
					IRowSettingsPtr pRow = m_List->GetRow(nRow);
					ColorizeItem(pRow);
				}

			}
		}
	}
}

void CToDoAlarmDlg::OnSelChosenCombo(long nRow) 
{
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll("Error in OnSelChosenCombo");
}

void CToDoAlarmDlg::OnBtnPrintTodos() 
{
	try {

		if(m_Combo->GetCurSel() == -1) {
			MessageBox("Please select a user.");
			return;
		}

		OnOK();

		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(453)]);
		infReport.nExtraID = m_Combo->GetValue(m_Combo->GetCurSel(),0).lVal;
		COleDateTime dtMin, dtNow;
		dtMin.ParseDateTime("1/1/1753");
		dtNow = COleDateTime::GetCurrentTime();
		infReport.DateFrom = dtMin;
		infReport.DateTo = dtNow;
		infReport.nDateRange = 2;
		infReport.nDateFilter = 2;

		//d.thompson 2016-08-22 - Since this report is already jam packed with all the usual custom filters in use, I have
		//	to get creative.  Since to-do's don't have locations, I'm going to steal the location filter field and
		//	put my category there.
		if (m_CategoryFilter->GetCurSel() != sriNoRow) {
			int nID = VarLong(m_CategoryFilter->GetValue(m_CategoryFilter->GetCurSel(), cfcID));
			if (nID != CategoryFilter_AllCategoriesID) {
				infReport.nLocation = nID;
			}
			else {
				//All categories, don't filter anything
			}
		}


		//Made new function for running reports - JMM 5-28-04
		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, true, this, "To-Do Preview", NULL, TRUE);
		
	}NxCatchAll("Error previewing To-Do report.");
}

void CToDoAlarmDlg::OnCancel()
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		//TES 5-8-03: Do not call CDialog::OnCancel for a modeless window!
		ShowWindow(SW_HIDE);
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnRememberColumnSettings() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		//save the setting
		long nRemember = 0;	//default off
		if(IsDlgButtonChecked(IDC_REMEMBER_COLUMN_SETTINGS))
			nRemember = 1;
		else
			nRemember = 0;
		SetRemotePropertyInt("RememberTODOColumns", nRemember, 0, GetCurrentUserName());

		//size the datalist appropriately
		if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMN_SETTINGS)) {
			ResetColumnSizes();
		}
		else {
			SetColumnSizes();
		}
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnColumnSizingFinishedList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		//DRT 6/6/03 - Saves the sizes of all columns if bCommitted is set and the checkbox is set

		//uncommitted
		if(!bCommitted)
			return;

		//don't want to remember
		if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMN_SETTINGS))
			return;

		//save width of each column
		IColumnSettingsPtr pCol;
		CString str, strList;

		for(int i = 0; i < m_List->GetColumnCount(); i++) {
			pCol = m_List->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultTODOColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-06-05 12:08) - PLID 49859 - Created this to take in a CSV formated string and insert a number into it
//	nLocation is the 0 based index of where to insert, strList is the CSV, strNewVal is what will be inserted
//	nIfLength condition to only add if the CSV length is equal to nIfLength, Use -1 to always add
CString InsertInCSVList(long nLocation, CString strList, CString strNewVal, long nIfLength = -1)
{
	int nCommaCount = 0;
	int nLastFind = strList.Find(",");
	int nSearchCommaLoc = -1;

	//Count our commas and find the and find the one we are looking for
	while (nLastFind > 0)
	{			
		nCommaCount++;
		if(nCommaCount == 3)
		{
			//Save the 3rd location, we may need to change it
			nSearchCommaLoc = nLastFind;
		}
		nLastFind = strList.Find(",", nLastFind+1);
	}

	if(nIfLength > 0 && nCommaCount == nIfLength)
	{
		//We could not find the location to insert
		if(nSearchCommaLoc > 0)
		{
			//Insert our default size
			return strList.Left(nSearchCommaLoc) + "," + strNewVal + "," + strList.Right(strList.GetLength() - (nSearchCommaLoc+1));
		}
	}

	return strList;
}


void CToDoAlarmDlg::SetColumnSizes()
{
	//DRT - 6/6/03 - This function takes the saved column sizes out of ConfigRT
	//		IF the box is checked.

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMN_SETTINGS)) {
		return;
	}

	CString strCols = GetRemotePropertyText("DefaultTODOColumnSizes", "", 0, GetCurrentUserName(), false);

	if(!strCols.IsEmpty()) {
		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		// (j.fouts 2012-06-05 11:32) - PLID 49859 - If they only have 18 Columns add our new column in defaulted at 100
		strCols = InsertInCSVList(3, strCols, "100", 18);

		//parse the columns out and set them
		int nComma = strCols.Find(",");

		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_List->GetColumn(i);
			if(pCol)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}
}

void CToDoAlarmDlg::ResetColumnSizes()
{
	//DRT - 6/6/03 - This function takes the saved m_strOriginalColSizes and
	//		resets all columns to those sizes.

	if(m_strOriginalColSizes.IsEmpty()) {
		//not sure why we wouldn't have any, but better to leave them as is
		//than to set to empty
		return;
	}

	CString strCols = m_strOriginalColSizes;
	int nWidth = 0, i = 0;

	//parse the columns out and set them
	int nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_List->GetColumn(i);
		if(pCol) {
			if(!IsNexTechInternal() && i == State) {
				//we're not disabling this for clients, just hiding it by default
				pCol->PutStoredWidth(0);
			}
			else {
				pCol->PutStoredWidth(nWidth);
			}
		}

		i++;
		nComma = strCols.Find(",");
	}
}

void CToDoAlarmDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		CDialog::OnActivate(nState, pWndOther, bMinimized);
		
		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation
		if (nState == WA_ACTIVE) {
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - OnNcHitTest should return an LRESULT
LRESULT CToDoAlarmDlg::OnNcHitTest(CPoint point) 
{
	/* Calculate the new position of the size grip */
	CRect rc;
	GetWindowRect(&rc);
	rc.top = rc.bottom - GetSystemMetrics( SM_CYHSCROLL );
	rc.left = rc.right - GetSystemMetrics( SM_CXVSCROLL ); 

	if (rc.PtInRect(point)) {
		return HTBOTTOMRIGHT;
	}
	
	return CDialog::OnNcHitTest(point);
}

LRESULT CToDoAlarmDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CToDoAlarmDlg::OnCheckTodoHigh() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnCheckTodoLow() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnCheckTodoMedium() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnRadioTodoAll() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnRadioTodoComplete() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnRadioTodoIncomplete() 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2008-11-07 08:33) - This function needs to return true if the where clause changed at all.
bool CToDoAlarmDlg::GenerateWhereClause()
{
	CString strWhere;
	BOOL bDateFilterChanged = FALSE;

	// choose a user filter if necessary

	long nID, nCurSel = m_Combo->GetCurSel() ;
	// if nothing is selected, then just set the where clause to look for things assigned to -1
	// that way nothing will be returned
	if(nCurSel == sriNoRow){
		nID = nCurSel;
	}
	else{
		nID = m_Combo->GetValue(m_Combo->GetCurSel(),0).lVal;
	}

	// (c.haag 2008-06-10 12:34) - PLID 11599 - Use TodoAssignToT
	CString strUser;
	strUser.Format("ToDoListQ.TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = %li)",nID);

	// Now include the filters
	CString strPriorities;

	int nExcluded = 0;
	long nExcludeID = -1;
	long nIncludeID = -1;

	CString strExclude, strInclude;

	if (!((CButton*)GetDlgItem(IDC_CHECK_TODO_HIGH))->GetCheck()) {
		nExcluded++;
		nExcludeID = 1;
	}
	else {
		nIncludeID = 1;
	}

	if (!((CButton*)GetDlgItem(IDC_CHECK_TODO_MEDIUM))->GetCheck()) {
		nExcluded++;
		nExcludeID = 2;
	}
	else {
		nIncludeID = 2;
	}

	if (!((CButton*)GetDlgItem(IDC_CHECK_TODO_LOW))->GetCheck()) {
		nExcluded++;
		nExcludeID = 3;
	}
	else {
		nIncludeID = 3;
	}

	if (nExcluded == 3) { // show nothing
		strPriorities = "1 = 0";

		// (a.walling 2015-08-14 10:09) - PLID 65660 - fixes CToDoAlarmDlg::TryUpdateTodoListByAppointmentID Incorrect syntax near the keyword 'THEN'
		// which occurs when m_strWhere is empty

		// just allow normal execution to continue; the 1=0 will prevent anything from being returned.
	}
	else if (nExcluded == 2) { // only show one
		strPriorities.Format("Priority = %li", nIncludeID);
	}
	else if (nExcluded == 1) { // only show two (exclude one)
		strPriorities.Format("Priority <> %li", nExcludeID);
	}
	//otherwise show all, don't bother putting anything in the where clause

	// (j.jones 2008-09-30 11:50) - PLID 31331 - filter on today's patients, if needed
	CString strPatientAppts;
	if(m_radioTodaysPatients.GetCheck()) {
		//exclude cancelled and no show patients
		strPatientAppts = "ToDoListQ.PersonID IN (SELECT PatientID FROM AppointmentsT "
			"WHERE Status <> 4 AND ShowState <> 3 "
			// (a.walling 2013-02-08 13:04) - PLID 55084 - dbo.AsDateNoTime(Date) ends up requring a full scan of the index
			// dbo.AsDateNoTime(Date) causing full scan of index
			"AND StartTime >= dbo.AsDateNoTime(GetDate()) AND StartTime < DATEADD(day, 1, dbo.AsDateNoTime(GetDate())) "
			")";
	}
	else if(m_radioApptsMarkedIn.GetCheck()) {
		//We say it's appts. marked "In", but it also includes appts. marked "Received"
		strPatientAppts = "ToDoListQ.PersonID IN (SELECT PatientID FROM AppointmentsT "
			"WHERE Status <> 4 AND (ShowState = 1 OR ShowState = 4) "
			// (a.walling 2013-02-08 13:04) - PLID 55084 - dbo.AsDateNoTime(Date) ends up requring a full scan of the index
			"AND StartTime >= dbo.AsDateNoTime(GetDate()) AND StartTime < DATEADD(day, 1, dbo.AsDateNoTime(GetDate())) "
			")";
	}
	//otherwise we show all tasks, not filtered by patient


	//Add filters on category, if selected
	CString strCatFilter = GetCategoryFilterText();


	//and now the active filters...
	CString strStatus;

	if (((CButton*)GetDlgItem(IDC_RADIO_TODO_INCOMPLETE))->GetCheck())
		strStatus = "Done IS NULL";
	else if (((CButton*)GetDlgItem(IDC_RADIO_TODO_COMPLETE))->GetCheck())
		strStatus = "Done IS NOT NULL";
	//else show everything, no need to clutter up the where clause.


	//and finally the date filters
	CString strFrom, strTo;

	_variant_t varFrom = m_DateFrom.GetValue();
	_variant_t varTo = m_DateTo.GetValue();

	// swap them if from > to
	if ( (varFrom.vt != VT_NULL) && (varTo.vt != VT_NULL) ) {
		if (VarDateTime(varFrom) > VarDateTime(varTo)) {
			_variant_t varTemp;
			varTemp = varTo;
			varTo = varFrom;
			varFrom = varTemp;
			m_DateFrom.SetValue(varFrom);
			m_DateTo.SetValue(varTo);
		}
	}

	if ((varFrom.vt != VT_NULL)) {
		strFrom.Format("( (Remind >= '%s') OR (Remind IS NULL) )", FormatDateTimeForSql(varFrom, dtoDate));
	}
	if ((varTo.vt != VT_NULL)) {
		COleDateTime dtTo = VarDateTime(varTo);
		//(e.lally 2007-01-02) PLID 24039 - We cannot just add one to the day if it is the last day of the month.
			//We can use a dateTimeSpan to properly increase our date.
		// set the "To Date" to midnight on the next day and filter for anything less than that.
		dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);
		dtTo += COleDateTimeSpan(1,0,0,0); //add 1 day
		strTo.Format("( (Remind < '%s') OR (Remind IS NULL) )", FormatDateTimeForSql(dtTo, dtoDate));
	}
	else {
		if (varFrom.vt == VT_NULL) { // only limit to NOW if a date is not manually specified
			// (z.manning 2008-10-28 16:16) - PLID 31841 - We now use the SQL function get date here
			// rather than passing in the current time because table checkers may be sent before we update
			// the where clause again.
			strTo.Format("( (Remind <= GetDate()) OR (Remind IS NULL) )");
			// (z.manning 2008-11-07 08:34) - PLID 31841 - We need to mark the where clause as changed
			// since even though the actual text isn't changing, we're have a new time.
			bDateFilterChanged = TRUE;
		}
		else {
			strTo = "";
		}
	}

	// now we must combine them all..


	if (strUser.GetLength() > 0) {
		strWhere += strUser;
	}

	if (strCatFilter.GetLength() > 0) {
		if (strWhere.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strCatFilter;
	}

	if (strPriorities.GetLength() > 0) {
		if (strWhere.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strPriorities;
	}

	// (j.jones 2008-09-30 11:50) - PLID 31331 - add the patient appts. filter, if needed
	if(strPatientAppts.GetLength() > 0) {
		if(strPatientAppts.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strPatientAppts;
	}

	if (strStatus.GetLength() > 0) {
		if (strWhere.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strStatus;
	}

	if (strFrom.GetLength() > 0) {
		if (strWhere.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strFrom;
	}

	if (strTo.GetLength() > 0) {
		if (strWhere.GetLength() > 0) {
			strWhere += " AND ";
		}
		strWhere += strTo;
	}

	// (j.jones 2008-11-07 09:01) - This code looks like it should only requery if
	// the filters changed, but actually we need to call m_List->Requery() every
	// time we open the ToDo Alarm, if we are not filtering on a fixed date range.
	// Otherwise, we would never find alarms with specific remind times.
	// Be aware of this when reviewing and modifying this code.

	if (m_strWhere.CompareNoCase(strWhere) == 0 && !bDateFilterChanged) {
		// the strings are identical! no use refreshing.
		return false;
	}
	else {
		m_strWhere = strWhere;
		m_List->WhereClause = (LPCTSTR)m_strWhere;
		return true;
	}
}

CString CToDoAlarmDlg::GetCategoryFilterText()
{
	CString strFilter = "";

	int nSel = m_CategoryFilter->GetCurSel();
	if (nSel != sriNoRow) {
		//We have a row selected, get its value
		int nID = VarLong(m_CategoryFilter->GetValue(nSel, cfcID));
		if (nID == CategoryFilter_AllCategoriesID) {
			//{All} Categories filter.  Do nothing, just leave the filter off.
		}
		else {
			//Specific category filter
			strFilter = FormatString("CategoryID = %li", nID);
		}
	}
	else {
		//No row selected, ignore the filter - treat as 'All'
	}
	return strFilter;
}

void CToDoAlarmDlg::OnChangeTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		if (!m_bDateDrop) {
			// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
			TryRequeryList();
		}

		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnChangeTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		if (!m_bDateDrop) {
			// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
			TryRequeryList();
		}	

		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnDropDownTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_bDateDrop = true; // the date has dropped, so set this to prevent refreshing as they scroll through

		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnCloseUpTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_bDateDrop = false; // date has closed up, so we can refresh now
		
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();

		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnDropDownTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		m_bDateDrop = true; // the date has dropped, so set this to prevent refreshing as they scroll through	
	}NxCatchAll(__FUNCTION__);
}

void CToDoAlarmDlg::OnCloseUpTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-17) PLID 36304 - Added try/catch
	try{
		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();

		m_bDateDrop = false; // date has closed up, so we can refresh now

		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

bool CToDoAlarmDlg::RefreshColors() // return true if colors have changed
{
	bool bColorize = (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1);

	COLORREF colorComplete = m_colorComplete,
		colorIncompleteHigh = m_colorIncompleteHigh,
		colorIncompleteMedium = m_colorIncompleteMedium,
		colorIncompleteLow = m_colorIncompleteLow;

	//Get the color values from preferences...
	m_colorComplete = (COLORREF)GetRemotePropertyInt("ToDoColorComplete", RGB(210, 255, 210), 0, GetCurrentUserName(), true);
	m_colorIncompleteHigh = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteHigh", RGB(240, 200, 200), 0, GetCurrentUserName(), true);
	m_colorIncompleteMedium = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteMedium", RGB(240, 210, 210), 0, GetCurrentUserName(), true);
	m_colorIncompleteLow = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteLow", RGB(240, 220, 220), 0, GetCurrentUserName(), true);

	UpdateColorField();

	bool bChanged =  !(	(m_colorComplete == colorComplete) &&
				(m_colorIncompleteHigh == colorIncompleteHigh) &&
				(m_colorIncompleteMedium == colorIncompleteMedium) &&
				(m_colorIncompleteLow == colorIncompleteLow) &&
				(bColorize && m_bListColored) );
	
	return bChanged;
}

void CToDoAlarmDlg::RecolorList()
{
	if (RefreshColors()) { // reload the colors
		ColorizeList();		// and recolorize the list if they have changed
	}
}

// sets BackColor field to a SQL Case statement that returns the appropriate backcolor of the row
void CToDoAlarmDlg::UpdateColorField()
{
	try {
		CString strField;
		bool bColorize = (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1);

		if (!bColorize) {
			strField = "16777215"; // this is 0xffffff (white)
		}
		else {
			CString strComplete, strIncomHigh, strIncomMed, strIncomLow;

			strComplete.Format("%li", m_colorComplete);
			strIncomHigh.Format("%li", m_colorIncompleteHigh);
			strIncomMed.Format("%li", m_colorIncompleteMedium);
			strIncomLow.Format("%li", m_colorIncompleteLow);

			strField.Format("CASE WHEN ToDoListQ.Done IS NOT NULL THEN %s ELSE (CASE WHEN ToDoListQ.Priority = 1 THEN %s WHEN ToDoListQ.Priority = 2 THEN %s WHEN ToDoListQ.Priority = 3 THEN %s ELSE %s END) END",
				strComplete, strIncomHigh, strIncomMed, strIncomLow, strIncomLow);
		}

		IColumnSettingsPtr pCol = m_List->GetColumn(BackColor);
		if (pCol) {
			pCol->PutFieldName((LPCTSTR)strField);
		}
	}NxCatchAll("Error updating color field");
}

// (j.jones 2008-09-30 11:17) - PLID 31331 - added functions for the patient appt. filter options
void CToDoAlarmDlg::OnRadioTodoAllPatients() 
{
	try {

		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();

	}NxCatchAll("Error in CToDoAlarmDlg::OnRadioTodoAllPatients");
}

void CToDoAlarmDlg::OnRadioTodoApptsMarkedIn() 
{
	try {

		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();

	}NxCatchAll("Error in CToDoAlarmDlg::OnRadioTodoApptsMarkedIn");
}

void CToDoAlarmDlg::OnRadioTodoTodaysAppts() 
{
	try {

		// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
		TryRequeryList();

	}NxCatchAll("Error in CToDoAlarmDlg::OnRadioTodoTodaysAppts");
}

// (j.jones 2008-09-30 14:34) - PLID 31331 - added TryUpdateTodoListByAppointmentID, which will
// see if the patient associated with the appointment has any active todos that need to be displayed
// in the list, given that the appt. qualifies in the current filter setup. If so, it will ensure
// the todos are displayed, either manually or through a requery.
void CToDoAlarmDlg::TryUpdateTodoListByAppointmentID(long nAppointmentID)
{
	try {

		if (nAppointmentID == -1) {
			// (j.jones 2014-08-05 11:06) - PLID 63187 - if the ID is -1, do nothing
			return;
		}

		//before we check anything, are we even filtering on appointments?

		BOOL bTodaysPatients = m_radioTodaysPatients.GetCheck();
		BOOL bApptsMarkedIn = m_radioApptsMarkedIn.GetCheck();

		if(!bTodaysPatients && !bApptsMarkedIn) {
			//we aren't filtering on either appt. option,
			//which means we don't care if an appt. changed
			return;
		}

		//if the list is already requerying, don't try to do anything here
		if(m_List->IsRequerying()) {
			return;
		}

		//Ok, we're filtering on appointments, and we have an appointment ID,
		//so let's see if the appt. qualifies for our filter, and if it is
		//for a patient that has to-do alarms of any kind

		//find all the tasks for the patient linked to this appointment,
		//that would match our current filtering
		//if it matches and doesn't exist, add it
		//if it doesn't match and does exist, remove it

		//this cannot be parameterized as it uses m_strWhere
		// (j.gruber 2010-02-24 13:04) - PLID 37510 - Clinical Decision Support Rules
		// (j.jones 2014-08-05 11:05) - PLID 63187 - parameterized
		// (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support added Assigned IDs
		// (a.walling 2015-07-27 17:14) - PLID 66435 - This was always failing because PreferredContact did not exist as a field
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"Convert(bit, CASE WHEN {CONST_STR} THEN 1 ELSE 0 END) AS IsDisplayed, "
				"ToDoListQ.TaskID, PersonT.ID AS PersonID, "
				"(CASE WHEN (SupplierT.PersonID IS NULL) THEN [Last] + ', ' + [First] ELSE Company END) AS Name, "
				"ToDoListQ.CategoryID, ToDoListQ.[Task], ToDoListQ.Priority, ToDoListQ.Notes, ToDoListQ.Deadline, "
				"(CASE WHEN(UsersT.UserName Is NULL) THEN CASE WHEN RegardingType = 3 THEN '<Tracking>' WHEN RegardingType = 5 THEN '<Custom Record>' WHEN RegardingType = 6 THEN '<Billing>' WHEN RegardingType = 7 THEN '<Export>' WHEN RegardingType = 14 THEN '<Clinical Decision Support Rules>' ELSE '' END ELSE UsersT.UserName END) AS EnteredBy, "
				"CASE WHEN PersonT.HomePhone IS NULL THEN '' ELSE PersonT.HomePhone END AS HomePhone, "
				"CASE WHEN PreferredContact = 1 THEN (CASE WHEN HomePhone IS NULL THEN '' ELSE HomePhone END) WHEN PreferredContact = 2 THEN (CASE WHEN WorkPhone IS NULL THEN '' ELSE WorkPhone END) WHEN PreferredContact = 3 THEN (CASE WHEN CellPhone IS NULL THEN '' ELSE CellPhone END) WHEN PReferredContact = 4 THEN (CASE WHEN Pager IS NULL THEN ''ELSE Pager END) WHEN PreferredContact = 5 THEN (CASE WHEN OtherPhone IS NULL THEN ''ELSE OtherPhone END) WHEN PreferredContact = 6 THEN (CASE WHEN Email IS NULL THEN '' ELSE Email END) END AS PreferredPhone, "
				"CASE WHEN PersonT.Email IS NULL THEN '' ELSE PersonT.Email END AS Email, "
				"(CASE WHEN PatientsT.PreferredContact = 1 "
				"	THEN PersonT.HomePhone + ' (Home)' WHEN PatientsT.PreferredContact = 2 "
				"	THEN PersonT.WorkPhone + ' (Work)' WHEN PatientsT.PreferredContact = 3 "
				"	THEN PersonT.CellPhone + ' (Mobile)' WHEN PatientsT.PreferredContact = 4 "
				"	THEN PersonT.Pager + ' (Pager)' WHEN PatientsT.PreferredContact = 5 "
				"	THEN PersonT.OtherPhone + ' (Other)' WHEN PatientsT.PreferredContact = 6 "
				"	THEN PersonT.Email WHEN PatientsT.PreferredContact = 7 "
				"	THEN PersonT.CellPhone + ' (Text Message)' "
				"	ELSE CASE WHEN PersonT.HomePhone NOT LIKE '' "
				"	THEN PersonT.HomePhone + ' (Home)' WHEN PersonT.WorkPhone NOT LIKE '' "
				"	THEN PersonT.WorkPhone + ' (Work)' WHEN PersonT.CellPhone NOT LIKE '' "
				"	THEN PersonT.CellPhone + ' (Mobile)' WHEN PersonT.Pager NOT LIKE '' "
				"	THEN PersonT.Pager + ' (Pager)' WHEN PersonT.OtherPhone NOT LIKE '' "
				"	THEN PersonT.OtherPhone + ' (Other)' WHEN PersonT.Email NOT LIKE '' "
				"	THEN PersonT.Email END END) AS PreferredContact, \r\n"
				"ToDoListQ.RegardingType AS RegardingType, RegardingID, "
				"ToDoListQ.Done AS Done, (CASE WHEN ToDoListQ.Done IS NULL THEN ToDoListQ.Remind ELSE NULL END) AS Remind, "
				"dbo.GetTodoAssignToIDString(ToDoListQ.TaskID) as AssignedUserIDs "
				"FROM ToDoList ToDoListQ "
				"INNER JOIN PersonT ON ToDoListQ.PersonID = PersonT.ID "
				"LEFT JOIN UsersT ON ToDoListQ.EnteredBy = UsersT.PersonID "
				"LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID LEFT JOIN PatientsT ON PatientsT.PersonID = PersonT.ID "
				"INNER JOIN AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
				"WHERE AppointmentsT.ID = {INT}", m_strWhere, nAppointmentID);

		BOOL bChangesMade = FALSE;

		FieldsPtr fields = rs->Fields;

		while(!rs->eof) {

			BOOL bIsDisplayed = VarBool(fields->Item["IsDisplayed"]->Value, FALSE);
			long nTaskID = VarLong(fields->Item["TaskID"]->Value);

			//whatever we do, we need to know if this task is in the list
			long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

			if(!bIsDisplayed) {
				//this task should not be shown, so if it is in the list,
				//then remove it from the list

				if(nRow != -1) {

					//it is there, so remove it
					m_List->RemoveRow(nRow);

					bChangesMade = TRUE;
				}
				else {
					//it's not in the list, and shouldn't be in the list,
					//so we do not need to do anything
				}
			}
			else {

				//if it is not in the list, add it
				if(nRow == -1) {

					//the row is not there, so just add it
					IRowSettingsPtr pRow = m_List->GetRow(-1);

					pRow->PutValue(TaskID, nTaskID);
					pRow->PutValue(PersonID, fields->Item["PersonID"]->Value);
					pRow->PutValue(Name, fields->Item["Name"]->Value);
					// (j.fouts 2012-06-05 12:13) - PLID 49859 - Added these back in
					pRow->PutValue(HomePhone, fields->Item["HomePhone"]->Value);
					pRow->PutValue(PreferredPhone, fields->Item["PreferredPhone"]->Value);
					pRow->PutValue(Email, fields->Item["Email"]->Value);
					// (j.fouts 2012-04-30 11:14) - PLID 49859 - Consolidated Phone and Email into a PreferredContact
					pRow->PutValue(PreferredContact, fields->Item["PreferredContact"]->Value);
					pRow->PutValue(Notes, fields->Item["Notes"]->Value);
					pRow->PutValue(CategoryID, fields->Item["CategoryID"]->Value);
					pRow->PutValue(Task, fields->Item["Task"]->Value);
					pRow->PutValue(Priority, (short)fields->Item["Priority"]->Value);
					pRow->PutValue(AssignedIDs, fields->Item["AssignedUserIDs"]->Value); // (s.tullis 2014-10-01 15:54) - PLID 63344 -TableChecker EX support
					/*_variant_t varPriority = fields->Item["Priority"]->Value;
					switch(VarByte(varPriority)) {
						case 1: 
							pRow->PutValue(Priority,_variant_t("High") );
						break;
						case 2:
							pRow->PutValue(Priority, _variant_t("Medium"));
						break;
						case 3:
							pRow->PutValue(Priority, _variant_t("Low"));
					}*/
					pRow->PutValue(Deadline, fields->Item["Deadline"]->Value);
					pRow->PutValue(EnteredBy, fields->Item["EnteredBy"]->Value);
					pRow->PutValue(Completed, fields->Item["Done"]->Value);
					pRow->PutValue(Remind, fields->Item["Remind"]->Value);
					pRow->PutValue(RegardingID, fields->Item["RegardingID"]->Value);

					// (a.walling 2006-09-05 10:59) - PLID 22387 - This should never be null or empty!
					if ( (fields->Item["RegardingType"]->Value.vt == VT_NULL) || (fields->Item["RegardingType"]->Value.vt == VT_EMPTY) )
					{
						ASSERT(false);
						pRow->PutValue(RegardingType, (long)ttPatientContact);
					}
					else {
						pRow->PutValue(RegardingType, fields->Item["RegardingType"]->Value);
					}

					if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) {
						ColorizeItem(pRow);
					}

					//add the row to the datalist
					m_List->AddRow(pRow);

					bChangesMade = TRUE;
				}
				else {
					//it's already in the list, and it is not our responsibility
					//to change it, that's the ToDo tablechecker's responsibility
				}			
			}

			rs->MoveNext();
		}
		rs->Close();

		if(bChangesMade) {
			//update our window text
			CString strTitle;
			strTitle.Format("%s - %li items", TITLE_BASE_STRING, m_List->GetRowCount());
			SetWindowText(strTitle);
		}
		

	}NxCatchAll("Error in CToDoAlarmDlg::TryUpdateTodoListByAppointmentID");
}

// (z.manning 2008-11-21 10:25) - PLID 31893 - Function to handle clicking on links for EMR to-dos
void CToDoAlarmDlg::HandleEmrTodoLink(const TodoType eType, const long nRegardingID, const long nPersonID)
{
	// (z.manning 2008-11-21 10:46) - PLID 31893 - If they somehow have an EMR to-do but no
	// EMR license, don't do anything.
	if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		return;
	}

	switch(GetRemotePropertyInt("EmrTodoLinkBehavior", 1, 0, GetCurrentUserName()))
	{
		case 1: // (z.manning 2008-11-21 10:32) - PLID 31893 - Open the EMN
		{
			if(nRegardingID == -1) {
				// (z.manning 2008-11-21 15:13) - PLID 31893 - Moments ago I learned that when creating
				// to-dos from an EMN it does not fill out the regarding ID for the to-do until the EMN
				// is saved. This certainly makes sense for new EMNs, however, this is also the case for
				// creating to-dos on existing EMNs for only reason c.haag can explain. I entered 32154
				// to look into that.
				//
				// In the meantime, if we don't have a valid regarding ID, then no point to running the
				// below queries and isntead we'll simply go to the NexEMR tab and be done with it.
				//
				// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
				LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
				return;
			}

			ADODB::_RecordsetPtr prs;
			// (z.manning 2008-11-21 11:17) - PLID 31893 - We need to know the EMN ID and PIC ID before
			// we can open the EMR.
			long nEmnID = -1, nPicID = -1, nChartID = -1;
			
			switch(eType)
			{
				case ttEMN:
					nEmnID = nRegardingID;
					prs = CreateParamRecordset(
						"SELECT PicT.ID AS PicID, EmnTabChartsLinkT.EmnTabChartID \r\n"
						"FROM EmrMasterT \r\n"
						"INNER JOIN EmrGroupsT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID \r\n"
						"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID \r\n"
						"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
						"WHERE EmrMasterT.ID = {INT} AND EmrMasterT.Deleted = 0 \r\n"
						, nRegardingID);
					if(prs->eof) {
						// (z.manning 2008-11-21 10:52) - PLID 31893 - The EMR was likely deleted. Just go to the
						// EMR tab.
						// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
						LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
						return;
					}
					else {
						nPicID = AdoFldLong(prs->GetFields(), "PicID");
						nChartID = AdoFldLong(prs, "EmnTabChartID", -1);
					}
					break;

				case ttEMNDetail:
					prs = CreateParamRecordset(
						"SELECT EmrMasterT.ID AS EmnID, PicT.ID AS PicID, EmnTabChartsLinkT.EmnTabChartID \r\n"
						"FROM EmrDetailsT \r\n"
						"INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
						"INNER JOIN EmrGroupsT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID \r\n"
						"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID \r\n"
						"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
						"WHERE EmrDetailsT.ID = {INT} AND EmrMasterT.Deleted = 0 \r\n"
						, nRegardingID);
					if(prs->eof) {
						// (z.manning 2008-11-21 10:52) - PLID 31893 - The EMR was likely deleted. Just go to the
						// EMR tab.
						// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
						LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
						return;
					}
					else {
						nEmnID = AdoFldLong(prs->GetFields(), "EmnID");
						nPicID = AdoFldLong(prs->GetFields(), "PicID");
						nChartID = AdoFldLong(prs, "EmnTabChartID", -1);
					}
					break;

				default:
					ASSERT(FALSE);
					// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
					LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
					return;
					break;
			}

			// (z.manning 2008-11-21 11:18) - PLID 31893 - Assuming we loaded valid values for the PIC and EMN
			// IDs then go ahead and open the corresponding EMR. Note that EditEmrRecord will go to the NexEMR tab.
			// (z.manning 2011-05-24 11:18) - PLID 33114 - Check chart permissions before attempting to open the EMN.
			BOOL bCanViewChart = (nChartID == -1 || CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, nChartID, TRUE));
			if(nPicID != -1 && nEmnID != -1 && bCanViewChart) {
				GetMainFrame()->EditEmrRecord(nPicID, nEmnID);
			}
			else {
				// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
				LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
			}
		}
		break;

		case 2: // (z.manning 2008-11-21 10:32) - PLID 31893 - Go to this patients NexEMR tab
		{
			// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
			LoadPatientScreen(nPersonID, GetMainFrame()->GetPrimaryEmrTab());
		}
		break;
	}
}

// (c.haag 2010-05-24 9:48) - PLID 38731 - Handling for MailSent tasks
void CToDoAlarmDlg::HandleMailSentTodoLink(const long nRegardingID, const long nPersonID)
{
	// First, open the History tab
	LoadPatientScreen(nPersonID, PatientsModule::HistoryTab);

	// Then post a message to the history tab to open the document
	CNxTabView* pView = (CNxTabView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
	if (NULL != pView) {
		CNxDialog *pSheet = pView->GetActiveSheet();
		if (NULL != pSheet) {
			// This should be the history tab. If it's not, then nothing will happen because no
			// other sheet in Practice handles this message.
			pSheet->PostMessage(NXM_HISTORY_OPEN_DOCUMENT, nRegardingID);
		}
	}
}

//(c.copits 2011-02-18) PLID 40794 - Permissions for individual todo alarm fields
BOOL CToDoAlarmDlg::CheckIndividualPermissions(short nCol)
{
	BOOL bSelfAssign = FALSE;
	BOOL bRemoteAssign = FALSE;
	BOOL bValid = TRUE;

	// Method column
	const int nMethod = 9;

	if (VarLong(m_Combo->GetValue(m_Combo->GetCurSel(),0)) == GetCurrentUserID()) {
		bSelfAssign = TRUE;
	}
	else {
		bRemoteAssign = TRUE;
	}

	// Permissions if assigned to self and have self write permissions		
	if (bSelfAssign && CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == CategoryID) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == nMethod) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Priority) {
					bValid = FALSE;
				}
			}

			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Notes) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Deadline) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsRemindTime, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Remind) {
					bValid = FALSE;
				}
			}

		}
	// Permission if assigned to other and have other write permissions
	if (bRemoteAssign && CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == CategoryID) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == nMethod) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Priority) {
					bValid = FALSE;
				}
			}

			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Notes) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Deadline) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsRemindTime, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == Remind) {
					bValid = FALSE;
				}
			}
	}

	return bValid;
}

// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
void CToDoAlarmDlg::TryRequeryList()
{
	if (GenerateWhereClause()) {
		SetWindowText(TITLE_BASE_STRING + CString(" - Loading..."));
		m_List->Requery();

		// (j.jones 2014-08-12 11:49) - PLID 63187 - clear our needs requery flag
		m_bNeedsRequery = false;
	}
}

void CToDoAlarmDlg::SelChosenTodoCategoryFilter(long nRow)
{
	try {
		TryRequeryList();
	} NxCatchAll(__FUNCTION__);
}


void CToDoAlarmDlg::SelChangingTodoCategoryFilter(long* nNewSel)
{
	try {
		if (*nNewSel == sriNoRow) {
			*nNewSel = 0;	//reset sel to first row
		}
	} NxCatchAll(__FUNCTION__);
}
