// FirstAvailList.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerrc.h"
#include "FirstAvailList.h"
#include "globaldatautils.h"
#include "globalschedutils.h"
#include "commonschedutils.h"
#include "firstavailableappt.h"
#include "InternationalUtils.h"
#include "MultiSelectDlg.h"
#include "SchedulerView.h"
#include "NxSchedulerDlg.h"
#include "foreach.h"
#include <set>
#include "AttendanceUtils.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "CommonFFAUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

using namespace ADODB;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (r.gonet 2014-12-17) - PLID 64465 - Moved PopulateThread and thread data structures into the CFFASearchThread class.
// (d.thompson 2010-11-02) - PLID 41284 - Conversion to dl2
// (j.gruber 2011-05-11 16:09) - PLID 41131 - added template names
// (j.politis 2015-07-09 12:35) - PLID 65629 - Add a new “Location” column in the results list after the “Resources” column.
enum eListColumns {
	elcDate = 0,
	elcDay,
	elcResources,
	elcResourceIDs,
	elcLocationName,
	elcLocationID,
	elcTemplateNames,
	elcPrecisionTemplateIDs,
};

// (r.farnworth 2015-05-18 16:57) - PLID 65633 - Dropdown in FFA results to filter by resource. The dropdown list will contain only resources referenced in any results, alphabetical by name, plus < All Resources > at the top
enum eResourceComboColumns {
	rccID = 0,
	rccName,
};

// (r.farnworth 2015-06-05 11:19) - PLID 65635 - Dropdown in FFA results to filter by location. The dropdown list will contain only locations referenced in any results, alphabetical by name, plus < All Locations > at the top
enum eLocationComboColumns {
	lccID = 0,
	lccName,
};

/////////////////////////////////////////////////////////////////////////////
// CFirstAvailList dialog

CFirstAvailList::CFirstAvailList(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFirstAvailList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFirstAvailList)
	// (r.gonet 2014-12-17) - PLID 64465 - Replaced a whole lot of member variables that were search settings
	// with a CFFASearchSettings object. Load the default settings.
	m_pSearchSettings = make_shared<CFFASearchSettings>();
	//TES 1/7/2015 - PLID 64466
	m_bReturnSlot = false;
	m_bReSchedule = false;
	m_pCallingDlg = NULL;
	// (v.maida 2016-02-24 15:07) - PLID 68385 - Set default (invalid) location and resource selections.
	m_nCurrentLocation = -99;
	m_nCurrentResource = -99;
	m_nNewResID = -1;
	//}}AFX_DATA_INIT
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
CFirstAvailList::~CFirstAvailList()
{
	try {
		// (z.manning, 02/27/2007) - PLID 24969 - Ensure that all memory is cleaned up.
		Clear();
	} catch(...) {
		//Unsafe to do any exception handling here, so just drop it.  Better than crashing.
	}
}

void CFirstAvailList::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFirstAvailList)
	DDX_Control(pDX, IDC_FILTER_RESOURCE_LEFT, m_resourceLeftButton);
	DDX_Control(pDX, IDC_FILTER_RESOURCE_RIGHT, m_resourceRightButton);
	DDX_Control(pDX, IDC_FILTER_LOCATION_LEFT, m_locationLeftButton);
	DDX_Control(pDX, IDC_FILTER_LOCATION_RIGHT, m_locationRightButton);
	DDX_Control(pDX, IDC_BTN_SCHEDULE_EDIT, m_btnScheduleEdit);
	DDX_Control(pDX, IDC_BTN_SCHEDULE, m_btnSchedule);
	DDX_Control(pDX, IDC_BTN_SCHEDULE_KEEP_OPEN, m_btnScheduleKeepOpen); // (z.manning, 02/21/2007) - PLID 23906
	DDX_Control(pDX, IDC_BTN_NEW_FFA_SEARCH, m_btnNewSearch);
	DDX_Control(pDX, IDC_BTN_GO_SELECTED_DAY, m_btnGoSelectedDay);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_GO_SELECTED_DAY, m_checkGoSelectedDay);
	DDX_Control(pDX, IDC_FFA_PROGRESS_BAR, m_ctrlProgressBar);
	DDX_Control(pDX, IDC_FFA_PROGRESS_TEXT, m_nxstaticProgressText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFirstAvailList, CNxDialog)
	//{{AFX_MSG_MAP(CFirstAvailList)
	ON_BN_CLICKED(IDC_FILTER_RESOURCE_LEFT, OnMoveResourceLeft)
	ON_BN_DOUBLECLICKED(IDC_FILTER_RESOURCE_LEFT, OnDoubleClickResourceLeft)
	ON_BN_CLICKED(IDC_FILTER_RESOURCE_RIGHT, OnMoveResourceRight)
	ON_BN_DOUBLECLICKED(IDC_FILTER_RESOURCE_RIGHT, OnDoubleClickResourceRight)
	ON_BN_CLICKED(IDC_FILTER_LOCATION_LEFT, OnMoveLocationLeft)
	ON_BN_DOUBLECLICKED(IDC_FILTER_LOCATION_LEFT, OnDoubleClickLocationLeft)
	ON_BN_CLICKED(IDC_FILTER_LOCATION_RIGHT, OnMoveLocationRight)
	ON_BN_DOUBLECLICKED(IDC_FILTER_LOCATION_RIGHT, OnDoubleClickLocationRight)
	ON_COMMAND(IDC_BTN_GO_SELECTED_DAY, OnGoSelectedDay)
	ON_COMMAND(IDC_BTN_NEW_FFA_SEARCH, OnNewSearch)
	ON_COMMAND(IDC_BTN_SCHEDULE, OnBtnSchedule)
	ON_COMMAND(IDC_BTN_SCHEDULE_KEEP_OPEN, OnBtnScheduleKeepOpen) // (z.manning, 02/21/2007) - PLID 23906
	ON_COMMAND(IDC_BTN_SCHEDULE_EDIT, OnBtnScheduleEdit)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHECK_GO_SELECTED_DAY, OnCheckGoSelectedDay)
	ON_MESSAGE(NXM_FFA_LIST_REQUERY_FINISHED, OnFFAListRequeryFinished)
	ON_MESSAGE(NXM_FFA_LIST_PROCESS_DATA, OnFFAListProcessData) // (z.manning 2010-11-01 15:52) - PLID 41272
	ON_MESSAGE(NXM_FFA_SET_PROGRESS_MIN_MAX, OnSetFFAProgressMinMax)
	ON_MESSAGE(NXM_FFA_SET_PROGRESS_POSITION, OnSetFFAProgressPosition)
	ON_MESSAGE(NXM_FFA_SET_PROGRESS_TEXT, OnSetFFAProgressText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFirstAvailList message handlers
// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
BOOL CFirstAvailList::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-24 14:16) - PLID 29776 - NxIconize the buttons
		m_btnScheduleEdit.AutoSet(NXB_OK);
		m_btnSchedule.AutoSet(NXB_OK);
		m_btnScheduleKeepOpen.AutoSet(NXB_OK);
		m_btnNewSearch.AutoSet(NXB_CANCEL);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (r.farnworth 2015-05-18 12:10) - PLID 65634 - Left/right buttons to filter results by resource in FFA results.
		m_resourceLeftButton.AutoSet(NXB_LEFT);
		m_resourceRightButton.AutoSet(NXB_RIGHT);
		// (r.farnworth 2015-06-05 12:49) - PLID 65636 - Left/right buttons to filter results by location in FFA results.
		m_locationLeftButton.AutoSet(NXB_LEFT);
		m_locationRightButton.AutoSet(NXB_RIGHT);
		// (r.farnworth 2015-05-18 15:57) - PLID 65633 - Dropdown in FFA results to filter by resource. The dropdown list will contain only resources referenced in any results, alphabetical by name, plus < All Resources > at the top
		m_FilterResourceCombo = BindNxDataList2Ctrl(this, IDC_FILTER_RESOURCE_COMBO, GetRemoteData(), false);
		// (r.farnworth 2015-06-05 11:22) - PLID 65635 - Dropdown in FFA results to filter by location. The dropdown list will contain only locations referenced in any results, alphabetical by name, plus < All Locations > at the top
		m_FilterLocationCombo = BindNxDataList2Ctrl(this, IDC_FILTER_LOCATIONS_COMBO, GetRemoteData(), false);

		// (c.haag 2008-04-24 14:43) - PLID 29776 - Don't use old colors
		//m_btnSchedule.SetTextColor(0xC08000);
		//m_btnScheduleKeepOpen.SetTextColor(0xC08000); // (z.manning, 02/21/2007) - PLID 23096
		//m_btnScheduleEdit.SetTextColor(0x008000);
		//m_btnNewSearch.SetTextColor(0xFF0000);
		//m_btnCancel.SetTextColor(0x0000FF);

		// (d.thompson 2010-11-02) - PLID 41284 - dl2 switch
		m_pResults = BindNxDataList2Ctrl(this, IDC_FFA_RESULTS_LIST, GetRemoteData(), false);

		// (c.haag 2006-11-27 11:25) - PLID 20772 - Initial clear
		Clear();

		//TES 1/7/2015 - PLID 64513 - Moved the "Move to selected day" logic to OnShowWindow()
		
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2007-05-04 09:55) - PLID 4850 - Called when the user switches. Refresh user settings.
// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnUserChanged()
{
	try {
		// (c.haag 2006-11-27 14:54) - PLID 20772 - Set the default value for the "Automatically
		// go to the selected day..." checkbox
		//TES 1/7/2015 - PLID 64513 - Don't check this box if we are set to return a slot
		if (GetRemotePropertyInt("AutoGotoApptInFFAListSearch", 1, 0, GetCurrentUserName(), TRUE) && !m_bReturnSlot) {
			((CButton*)GetDlgItem(IDC_CHECK_GO_SELECTED_DAY))->SetCheck(1);
		} else {
			((CButton*)GetDlgItem(IDC_CHECK_GO_SELECTED_DAY))->SetCheck(0);
		}
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-11-02 09:51) - PLID 41272 - Moved the logic to stop the thread out of Clear into its own function.
void CFirstAvailList::StopPopulateThread()
{
	// (r.gonet 2014-12-17) - PLID 64465 - Stop the thread if it is running.
	if (m_pFFASearch) {
		m_pFFASearch->Interrupt();
		m_pFFASearch->Join();
		// (r.gonet 2014-12-17) - PLID 64465 - Do not delete the search since we still may need to reference its data later.
		// (r.gonet 2015-01-08) - PLID 64465 - Pump any remaining process data messages so their handlers don't try to access deleted opening objects. The other option would be to use a mutex but this is easier.
		PeekAndPump();
	}
}

// (r.gonet 2014-12-17) - PLID 64465 - Initializes the search settings.
void CFirstAvailList::SetSearchSettings(CFFASearchSettingsPtr pSearchSettings)
{
	if (pSearchSettings == NULL) {
		ThrowNxException("%s : pSearchSettings is NULL.", __FUNCTION__);
	}

	m_pSearchSettings = pSearchSettings;
}

void CFirstAvailList::Clear()
{
	CWaitCursor wc;

	StopPopulateThread();

	// (v.maida 2016-02-24 15:07) - PLID 68385 - Reset current resource and location selections to their default (invalid) values.
	m_nCurrentResource = -99;
	m_nCurrentLocation = -99;

	//
	// (c.haag 2006-11-27 11:23) - PLID 20772 - Now clear everything else.
	// Most of these will be overwritten by a successive search, but it will
	// help us find out what information is not filled in that should be when
	// debugging
	//
	// (r.gonet 2014-12-17) - PLID 64465 - Revert the search settings to the default settings.
	m_pSearchSettings = make_shared<CFFASearchSettings>();
}

//TES 1/7/2015 - PLID 64466 - Added bReturnSlot, in which case this will fill m_UserSelectedSlot rather than actually scheduling the appointment
long CFirstAvailList::TryScheduleAppt(bool bReturnSlot /*= false*/)
{
	// (c.haag 2006-11-27 12:41) - PLID 20772 - This function tries to schedule
	// an appointment. Returns -1 on failure, or the appointment ID on success.
	// This code has existed for a long time in another function.

	// (b.cardillo 2005-04-28 16:18) - PLID 16368 - Don't let the user create appointments 
	// if he doesn't have permissions to do so.
	if (!CheckCurrentUserPermissions(bioAppointment, sptCreate)) {
		return -1;
	}

	//(e.lally 2005-12-29) PLID 18651 - Double clicking the schedule button causes a low level exception
	//We can just immediately disable it to prevent a quick double click
	GetDlgItem(IDC_BTN_SCHEDULE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_SCHEDULE_KEEP_OPEN)->EnableWindow(FALSE); // (z.manning, 02/21/2007) - PLID 23906
	GetDlgItem(IDC_BTN_SCHEDULE_EDIT)->EnableWindow(FALSE);

	// (d.thompson 2010-11-02) - PLID 41284 - dl2 switch
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResults->GetCurSel();
	CDWordArray adwResources, adwPurposes;
	long nResID = -1;

	if (pRow) {
		COleDateTime dt = pRow->GetValue(elcDate).date; // Fill the date
		COleDateTime tm, tmEnd;
		CString strResources = pRow->GetValue(elcResourceIDs).bstrVal;
		CString strWhere;
		CWaitCursor wc;

		// (r.farnworth 2016-02-02 09:27) - PLID 68116 - Default to the currently logged in location
		long nLocationID = VarLong(pRow->GetValue(elcLocationID), 0);
		if (nLocationID <= 0)
		{
			nLocationID = GetCurrentLocationID();
		}

		// Just an extra precaution
		if (strResources.GetLength())
		{
			
			// Parse the resource list
			char* sz = new char[strResources.GetLength() + 1];
			strcpy(sz, strResources);
			char* psz = strtok(sz, " ");
			while (psz)
			{
				adwResources.Add(atoi(psz));
				psz = strtok(NULL, " ");
			}
			// (j.gruber 2007-01-05 08:39) - PLID 24103 - fix this memory leak
			psz = NULL;
			delete sz;
			 

			// (c.haag 2003-09-03 15:57) - Allow the user to choose the resources
			// for this appointment if there is more than one.
			if (adwResources.GetSize() > 1)
			{
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "ResourceT");
				CString strRes;
				dlg.m_strNameColTitle = "Resources";
				strWhere.Format("ID = %d", adwResources[0]);
				dlg.PreSelect(adwResources[0]);
				for (long i=1; i < adwResources.GetSize(); i++)
				{
					CString str;
					dlg.PreSelect(adwResources[i]);
					str.Format(" OR ID = %d", adwResources[i]);
					strWhere += str;
				}

				// (j.gruber 2006-12-21 09:15) - PLID 23952 - make sure we have at least one resource selected before we let them close
				if (IDCANCEL == dlg.Open("ResourceT", strWhere, "ID", "Item", "Please select the resources to associate with this appointment.", 1))
				{
					//(e.lally 2005-12-29) PLID 18651 - Double clicking the schedule button causes a low level exception
					//We need to re-enable the IDOK button if they cancel the list of resources 
					//	and still have an appointment selected.
					// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion
					if(m_pResults->GetCurSel() != NULL) {
						UpdateButtons();
					}
					return -1;
				}
				strRes = dlg.GetMultiSelectIDString();

				for (i=0; i < adwResources.GetSize(); i++)
				{
					CString str;
					str.Format("%d", adwResources[i]);
					if (-1 == strRes.Find(str))
						adwResources.RemoveAt(i--);
				}
			}

			// Fill the purpose list (only 1 purpose)
			//TS 4/22/03: Don't add -1 as a purpose!
			// (r.gonet 2014-12-17) - PLID 64465 - I'm not going to comment for each of these places changed but the
			// old member variable search settings are now part of the CFFASearchSettings object, m_pSearchSettings.
			if (m_pSearchSettings->m_nAptPurpose != -1) {
				adwPurposes.Add(m_pSearchSettings->m_nAptPurpose);
			}

			//hold on to the full startDateTime, endDateTime so we don't have to recalculate it
			COleDateTime dtStart, dtEnd;
			dtStart = dt;
			dtEnd = dt + COleDateTimeSpan(0, m_pSearchSettings->m_nHours, m_pSearchSettings->m_nMinutes, 0);

			// Fill the start time and end time
			tm.SetTime(dt.GetHour(), dt.GetMinute(), dt.GetSecond());
			tmEnd = tm + COleDateTimeSpan(0, m_pSearchSettings->m_nHours, m_pSearchSettings->m_nMinutes, 0);
			dt.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());

			// Fill the arrival time
			COleDateTime dtArrival = tm - COleDateTimeSpan(0, 0, m_pSearchSettings->m_nArrivalMins, 0);
			// Don't let it go earlier than midnight
			if(dtArrival.m_dt < 0) {
				dtArrival.SetTime(0, 0, 0);
			}


			// (e.lally 2006-12-18) - PLID 21543 - If the user doesn't have permission to double-book,
			// we need to check if we would be double-booking against another appointment.
			// Rather than repeat the section of code as what was in AppointmentValidateByPermissions,
			// I pulled it out into its own function.

			if(!AppointmentValidateDoubleBooking(dtStart, dtEnd, adwResources)){
				UpdateButtons();
				return -1;
			}

			//validate this appt. against all rules
			if (!AppointmentValidateByRules(m_pSearchSettings->m_dwPatientID, adwResources, nLocationID, dt, tm, tmEnd, m_pSearchSettings->m_nAptType, adwPurposes, -1, FALSE, FALSE)) {
				UpdateButtons();
				return -1;
			}

			// (r.gonet 2014-11-19) - PLID 64174 - Make the selected insured party from the FFA dialog
			// the default primary insurance in the new appointment.
			//TES 1/7/2015 - PLID 64466 - We can skip all this insurance stuff if we're just returning a slot
			AppointmentInsuranceMap mapInsPlacements;
			if (!bReturnSlot && m_pSearchSettings->m_pInsurance != NULL && m_pSearchSettings->m_pInsurance->m_nInsuredPartyID != -1) {
				SetAppointmentInsuranceMap(mapInsPlacements, 1, m_pSearchSettings->m_pInsurance->m_nInsuredPartyID, m_pSearchSettings->m_pInsurance->m_strInsuranceCoName, m_pSearchSettings->m_pInsurance->m_strRespType);

				// Find the secondary insurance to fill along with primary
				//this query finds all primary and secondary insured parties for the patient,
				//returning the desired category first, other categories afterwards in order of priority
				_RecordsetPtr rsInsurance = CreateParamRecordset(R"(
SELECT TOP 1 InsuredPartyT.PersonID AS InsuredPartyID, RespTypeT.TypeName, InsuranceCoT.Name AS InsCoName 
FROM InsuredPartyT 
INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID 
INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
WHERE InsuredPartyT.PatientID = {INT} AND RespTypeT.CategoryPlacement = 2 
AND RespTypeT.CategoryType = {INT} 
)",
				m_pSearchSettings->m_dwPatientID, m_pSearchSettings->m_pInsurance->m_nCategoryID);
				if (!rsInsurance->eof) {
					long nSecondaryInsuredPartyID = AdoFldLong(rsInsurance->Fields, "InsuredPartyID");
					CString strSecondaryInsCoName = AdoFldString(rsInsurance->Fields, "InsCoName", "");
					CString strSecondaryRespType = AdoFldString(rsInsurance->Fields, "TypeName", "");
					SetAppointmentInsuranceMap(mapInsPlacements, 2, nSecondaryInsuredPartyID, strSecondaryInsCoName, strSecondaryRespType);
				}
				rsInsurance->Close();
			} else {
				// (r.gonet 2014-11-19) - PLID 64174 - They didn't select an insurance. Don't fill in anything according to Product Management.
			}
			//TES 1/7/2015 - PLID 64466 - If we're returning a slot, just go ahead and fill the values in our member variable, rather than calling AppointmentCreate()
			if (bReturnSlot && !m_bReSchedule) {
				m_UserSelectedSlot->dtStart = dtStart;
				m_UserSelectedSlot->dtEnd = dtStart + COleDateTimeSpan(0, m_pSearchSettings->m_nHours, m_pSearchSettings->m_nMinutes, 0);
				m_UserSelectedSlot->dtArrival = COleDateTime(dtStart.GetYear(), dtStart.GetMonth(), dtStart.GetDay(), dtArrival.GetHour(), dtArrival.GetMinute(), dtArrival.GetSecond());
				m_UserSelectedSlot->dwaResourceIDs.Copy(adwResources);
				m_UserSelectedSlot->nLocationID = nLocationID; // (r.farnworth 2016-02-02 09:27) - PLID 68116
			}
			else {
				// (r.gonet 2014-12-10) - PLID 64174 - We now create the appointment with the insurance selected in the FFA search.
				// (r.farnworth 2016-02-02 09:27) - PLID 68116 - Added nLocationID
				// (r.farnworth 2016-02-18 08:46) - PLID 68247 - Changed m_dwAptType to m_nAptType
				nResID = AppointmentCreate(m_pSearchSettings->m_dwPatientID, adwResources, nLocationID, dt, tm, tmEnd, dtArrival, 0, 0, 0, "", m_pSearchSettings->m_nAptType, adwPurposes, 0, -1, 1, true, &mapInsPlacements);
				// (r.gonet 2014-11-19) - PLID 64174 - Free up space occupied by the insurance placement map's elements.
				ClearAppointmentInsuranceMap(mapInsPlacements);

				// (d.moore 2007-10-16) - PLID 26546 - We need to check to see if the new appointment 
				//  might satisfy a request in the waiting list.
				if (nResID > 0) {
					CheckAppointmentAgainstWaitingList(nResID, m_pSearchSettings->m_dwPatientID, adwResources, dt, tm);
				}
			}			
			

		} // if (strResources.GetLength())
	} // if (pRow) {
	UpdateButtons();
	return nResID;
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnGoSelectedDay()
{
	try {
		// (c.haag 2006-11-27 15:01) - PLID 20772 - This function will go to the selected day
		// in the scheduler
		// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResults->GetCurSel();
		if (pRow == NULL) {
			return;
		}

		COleDateTime dt = VarDateTime(pRow->GetValue(elcDate));
		dt.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
		// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
		g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MultiResourceTab);
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
		if (pView) {
			CNxSchedulerDlg* pSheet = (CNxSchedulerDlg*)pView->GetActiveSheet();
			if (pSheet->GetActiveDate() != dt) {
				pSheet->SetActiveDate(dt);
				pView->UpdateView();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnNewSearch()
{
	try {
		//(e.lally 2011-05-16) PLID 41013 - Set the preselect values prior to hiding the window since that clears out all these values
		//(a.wilson 2012-3-28 PLID 49245 - use new default override struct instead of global bool.
		//also cleaned up the ffa reference from getmainframe so its a little more pleasing to the eyes.
		CFirstAvailableAppt *pFFA = &GetMainFrame()->m_FirstAvailAppt;
		pFFA->m_bDefaultOverrides.SetAllTo(true);
		// (r.gonet 2014-12-17) - PLID 64465 - I'm not going to comment for each of these places changed but the
		// old member variable search settings are now part of the CFFASearchSettings object, m_pSearchSettings.
		pFFA->m_nPreselectPatientID = (long)m_pSearchSettings->m_dwPatientID;
		pFFA->m_nPreselectApptTypeID = m_pSearchSettings->m_nAptType;
		pFFA->m_nPreselectApptPurposeID = m_pSearchSettings->m_nAptPurpose;
		// (r.gonet 2014-11-19) - PLID 64174 - Remember the old insured party so we can recall it when the dialog reloads.
		if (m_pSearchSettings->m_pInsurance != NULL) {
			pFFA->m_nPreselectInsuredPartyID = m_pSearchSettings->m_pInsurance->m_nInsuredPartyID;
		} else {
			pFFA->m_nPreselectInsuredPartyID = -1;
		}
		pFFA->m_bPreselectUserEditedDuration = m_pSearchSettings->m_bUserEditedDuration;
		pFFA->m_nPreselectDurHours = m_pSearchSettings->m_nHours;
		pFFA->m_nPreselectDurMins = m_pSearchSettings->m_nMinutes;
		pFFA->m_nPreselectIntervalMins = m_pSearchSettings->m_nSearchIntervalMinutes;
		pFFA->m_aryPreselectResources.Copy(m_pSearchSettings->m_arynResourceIDs);
		pFFA->m_aryPreselectWeekDays.Copy(m_pSearchSettings->m_arynWeekDayPrefList);
		//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
		pFFA->m_nPreselectResourceTypeSelection = m_pSearchSettings->m_nResourceTypeSelection;
		// (r.farnworth 2015-06-08 16:49) - PLID 65639
		pFFA->m_aryPreselectLocations.Copy(m_pSearchSettings->m_arynAptLocations);

		// (j.politis 2015-06-25 14:48) - PLID 65642 - Extend the time preferences listbox to go from midnight to midnight.
		long nLength = sizeof(m_pSearchSettings->m_anTimePrefList) / sizeof(long);
		for(int i=0; i < nLength; i++){
			if (m_pSearchSettings->m_anTimePrefList[i] != 0)
			{
				pFFA->m_aryPreselectTimes.Add(i);
			}
		}
		pFFA->m_bPreselectAnyResource = m_pSearchSettings->m_bAnyOpenResource;
		pFFA->m_bPreselectOfficeHoursOnly = m_pSearchSettings->m_bSearchOfficeHoursOnly;
		pFFA->m_dtPreselectStart = m_pSearchSettings->m_dtStartDate;
		pFFA->m_bPreselectStartDateRadio = m_pSearchSettings->m_bStartDateRadioChecked;


		// (c.haag 2006-11-27 12:45) - PLID 20772 - This function is called when
		// the user wants to start a new search. We hide this window and tell
		// the mainframe to start it
		ShowWindow(SW_HIDE);
		//TES 1/7/2015 - PLID 64513 - If we're returning a slot, then the FFA dialog will still be visible, so no need to post this message
		if (!m_bReturnSlot) {
			GetMainFrame()->PostMessage(WM_COMMAND, ID_FIRST_AVAILABLE_APPT);
		}
		else {
			//TES 1/12/2015 - PLID 64513 - Let our caller know that the window was closed
			m_pCallingDlg->PostMessage(NXM_ON_CLOSE_FIRST_AVAIL_LIST, (WPARAM)this, falcaNewSearch);
		}
	} NxCatchAll(__FUNCTION__);
}

void CFirstAvailList::OnBtnSchedule()
{
	// (c.haag 2006-11-27 12:45) - PLID 20772 - This function is called when
	// the user tries to schedule an appointment
	try {
#ifdef _DEBUG
		// Do this for easier debugging if any errors happen here
		SetWindowPos(&wndBottom, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
#endif
		//TES 1/7/2015 - PLID 64466 - Pass in whether we're returning a slot
		long nResID = TryScheduleAppt(m_bReturnSlot);
		m_nNewResID = nResID;

		//TES 1/7/2015 - PLID 64513 - Don't do any flipping if we're returning a slot
		if (!m_bReturnSlot) {
			// (c.haag 2004-01-29 10:26) - Open to the day and time of the appointment
			if (nResID > -1) {
				// Flip to the resource view. We should always be in the scheduler module
				// at this point.
				// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
				g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MultiResourceTab);
				CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
				if (pView) {
					((CSchedulerView*)pView)->UpdateView();
					// Go to the day and time, but don't open the resentry dialog.
					((CSchedulerView *)pView)->OpenAppointment(nResID, FALSE);
				}
				
			}
		}
		else {
			//TES 1/7/2015 - PLID 64466 - Let our parent know that the slot information has been filled in now
			//TES 1/9/2015 - PLID 64513 - Pass falcaOK for the LPARAM to let our caller know a slot was selected
			m_pCallingDlg->PostMessage(NXM_ON_CLOSE_FIRST_AVAIL_LIST, (WPARAM)this, falcaOK);
		}
		//
		// (c.haag 2006-11-27 11:08) - PLID 20772 - Now hide this dialog (rather than
		// close it) because it must persist as a floating overlapped window
		//
		//(e.lally 2007-05-04) PLID 23905 - Only hide this window if a new appointment was created.
		ShowWindow(SW_HIDE);

	}NxCatchAll("CFirstAvailList::OnBtnSchedule");
}

// (z.manning, 02/21/2007) - PLID 23906 - We now have a separate buttons for whether or not the user
// wants to close the first available list after scheduling an appt.
void CFirstAvailList::OnBtnScheduleKeepOpen()
{
	try {
		//TES 1/7/2015 - PLID 64466 - This should be disabled if we're returning a slot
		ASSERT(!m_bReturnSlot);

		long nResID = TryScheduleAppt();

		// (c.haag 2004-01-29 10:26) - Open to the day and time of the appointment
		if (nResID > -1) {
			// Flip to the resource view. We should always be in the scheduler module
			// at this point.
			// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
			g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MultiResourceTab);
			CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if (pView) {
				((CSchedulerView*)pView)->UpdateView();
				// Go to the day and time, but don't open the resentry dialog.
				((CSchedulerView *)pView)->OpenAppointment(nResID, FALSE);
			}
			
			// (z.manning, 02/22/2007) - PLID 23906 - The user scheduled an appt but wants to keep the FFA
			// screen open, so we need to search again to avoid conflicts with the appt they just scheduled.
			//
			//(e.lally 2007-05-04) PLID 23905 - Only re-run the search if a new appointment was created.
			GetDlgItem(IDC_BTN_SCHEDULE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_SCHEDULE_KEEP_OPEN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_SCHEDULE_EDIT)->EnableWindow(FALSE);
			// (z.manning 2010-11-02 09:05) - PLID 41272 - Make sure we stop the previous search first.
			StopPopulateThread();
			PopulateList();
		}

		

	}NxCatchAll("CFirstAvailList::OnBtnScheduleKeepOpen");	
}

void CFirstAvailList::OnBtnScheduleEdit()
{
	// (c.haag 2006-11-27 12:45) - PLID 20772 - This function is called when
	// the user tries to schedule an appointment
	try {
		//TES 1/7/2015 - PLID 64466 - This should be disabled if we're returning a slot
		ASSERT(!m_bReturnSlot);
#ifdef _DEBUG
		// Do this for easier debugging if any errors happen here
		SetWindowPos(&wndBottom, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
#endif
		long nResID = TryScheduleAppt();

		// (c.haag 2004-01-29 10:26) - Open to the day and time of the appointment
		if (nResID > -1) {
			// Flip to the resource view. We should always be in the scheduler module
			// at this point.
			// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
			g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MultiResourceTab);
			CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if (pView) {
				((CSchedulerView*)pView)->UpdateView();
				//(e.lally 2007-01-02) PLID 23812 - Hide the window right before we open the 
				//apointment to ensure that the appointment stays open when we make practice the
				//top position in the zorder.
				ShowWindow(SW_HIDE);
				// Go to the day and time, and open the resentry dialog.
				((CSchedulerView *)pView)->OpenAppointment(nResID, TRUE);
				//We can return so we don't hide the window again.
				return;
			}
			//
			// (c.haag 2006-11-27 11:08) - PLID 20772 - Now hide this dialog (rather than
			// close it) because it must persist as a floating overlapped window
			//
			//(e.lally 2007-05-04) PLID 23905 - Only hide this window if a new appointment was created.
			ShowWindow(SW_HIDE);
		}

		
	}
	NxCatchAll("Error saving appointment");	
}

void CFirstAvailList::OnOK() 
{
	// (c.haag 2006-11-27 11:16) - PLID 20772 - There is no OK button anymore; just
	// a schedule button
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnCancel() 
{
	try {
		//
		// (c.haag 2006-11-27 11:16) - PLID 20772 - Hide this dialog (rather than
		// close it) because it must persist as a floating overlapped window
		//
		ShowWindow(SW_HIDE);

		if (m_bReturnSlot) {
			//TES 1/9/2015 - PLID 64513 - Let our caller know that the window was closed
			m_pCallingDlg->PostMessage(NXM_ON_CLOSE_FIRST_AVAIL_LIST, (WPARAM)this, falcaCancel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnCheckGoSelectedDay() 
{
	try {
		//TES 1/7/2015 - PLID 64513 - The box should be disabled if we're returning a slot
		ASSERT(!m_bReturnSlot);
		if (IsDlgButtonChecked(IDC_CHECK_GO_SELECTED_DAY)) {
			SetRemotePropertyInt("AutoGotoApptInFFAListSearch", 1, 0, GetCurrentUserName());
		} else {
			SetRemotePropertyInt("AutoGotoApptInFFAListSearch", 0, 0, GetCurrentUserName());
		}
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-11-01 16:00) - PLID 41272 - This is now a member of CFirstAvailList
// (j.politis 2015-07-09 11:51) - PLID 65629 - Add a new “Location” column in the results list after the “Resources” column.
// (b.cardillo 2016-01-31 14:08) - PLID 65630 - We don't try to use location color anymore.
BOOL CFirstAvailList::TryAddApptToList(COleDateTime dt, const CString &strResourceIDs, const CString &strResourceNames, const CString &strLocationName, long nLocationID, const CString &strTemplateNames)
{
	//JMJ - make sure we don't add an entry for a time prior to now
	//(which could happen if we're trying to add entries for today,
	//but the times have already passed)
	if(dt < (COleDateTime::GetCurrentTime())) {
		return FALSE;
	}

	// Prepare to add the slot to the list
	//Because of bad floating point math, we need to re-calculate the date based on integer values
	dt.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), dt.GetHour(), dt.GetMinute(), dt.GetSecond());
	_variant_t vTime = dt;
	vTime.vt = VT_DATE;

	//when adding all resources at once, check for no mapping
	// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion in all 3 groups below
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResults->GetNewRow();
	pRow->PutValue(elcDate, vTime);
	CString strDay = FormatDateTimeForInterface(vTime.date, "%a");
	pRow->PutValue(elcDay, _variant_t(_bstr_t(strDay)));
	pRow->PutValue(elcResources, _variant_t(strResourceNames));
	pRow->PutValue(elcResourceIDs, _variant_t(strResourceIDs));
	// (j.politis 2015-07-09 11:51) - PLID 65629 - Add a new “Location” column in the results list after the “Resources” column.
	if (nLocationID >= 0)
	{
		pRow->PutValue(elcLocationName, _variant_t(strLocationName));
		pRow->PutValue(elcLocationID, _variant_t(nLocationID));
	}
	else
	{
		pRow->PutValue(elcLocationName, g_cvarNull);
		pRow->PutValue(elcLocationID, g_cvarNull);
	}
	pRow->PutValue(elcTemplateNames, _variant_t(strTemplateNames));
	pRow->PutValue(elcPrecisionTemplateIDs, "");

	// (r.farnworth 2015-05-21 11:39) - PLID 65633 - Check if the associated resources need to be added to the dropdown and do so accordingly
	CString strSearchResource, strDropdownResource;
	long nSearchResourceID, nRowResourceID;
	NXDATALIST2Lib::IRowSettingsPtr pIterRow;
	bool bAlreadyExists;

	CDWordArray aryRowResources;
	LoadResourceIDStringIntoArray(strResourceIDs, aryRowResources);

	//Take the Resource IDs of the search criteria and break them down. See if any resource for this appt matches
	for (long i = 0; i < m_pSearchSettings->m_arynResourceIDs.GetCount(); i++)
	{
		nSearchResourceID = m_pSearchSettings->m_arynResourceIDs.GetAt(i);
		strSearchResource = m_pSearchSettings->m_aryResourceNames.GetAt(i);
		bAlreadyExists = false;

		//Go through the list of Resources associated with this appt
		for (long j = 0; j < aryRowResources.GetCount(); j++)
		{
			//Do any of the resources associated with this appt match the search criteria resource we are examining?
			nRowResourceID = aryRowResources.GetAt(j);
			if (nRowResourceID == nSearchResourceID) {
				
				// do we already have this resource in our dropdown?
				if (m_FilterResourceCombo->FindByColumn(rccID, nSearchResourceID, m_FilterResourceCombo->GetFirstRow(), VARIANT_FALSE) != NULL) {
					bAlreadyExists = true;
				}

				if (!bAlreadyExists)
				{
					NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_FilterResourceCombo->GetNewRow();
					pNewRow->PutValue(rccID, _variant_t(nSearchResourceID));
					pNewRow->PutValue(rccName, _variant_t(strSearchResource));
					m_FilterResourceCombo->AddRowAtEnd(pNewRow, NULL);
					break;
				}
			}
		}
	}

	m_pResults->AddRowSorted(pRow, NULL);

	return TRUE;
}

//DRT 1/10/2007 - (no PLID) - Moved LooseCompareDouble to GlobalUtils

// (r.gonet 2014-12-17) - PLID 64465 - PopulateThread is now a member function of CFFASearchThread.
void CFirstAvailList::PopulateList()
{	
	// (r.gonet 2014-12-17) - PLID 64465 - Removed g_hStopPopulating since the new CFFASearchThread class uses an NxThread, which has Interrupt().
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	// (c.haag 2006-11-27 13:38) - PLID 20772 - We now only clear the list here
	// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion
	m_pResults->Clear();

	// (r.gonet 2014-12-17) - PLID 64465 - CFFASearchThread now takes care of the thread creation and passing data to it.
	m_pFFASearch = make_shared<CFFASearchThread>();
	m_pFFASearch->Run(GetSafeHwnd(), m_pSearchSettings);
}

BEGIN_EVENTSINK_MAP(CFirstAvailList, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFirstAvailList)
	ON_EVENT(CFirstAvailList, IDC_FFA_RESULTS_LIST, 2 /* SelChanged */, OnSelChangedFFAResultsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CFirstAvailList, IDC_FILTER_RESOURCE_COMBO, 16, CFirstAvailList::OnSelChosenResourceListCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailList, IDC_FILTER_RESOURCE_COMBO, 1, CFirstAvailList::OnSelChangingUnselectedResource, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFirstAvailList, IDC_FILTER_LOCATIONS_COMBO, 16, CFirstAvailList::OnSelChosenLocationListCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailList, IDC_FILTER_LOCATIONS_COMBO, 1, CFirstAvailList::OnSelChangingUnselectedLocation, VTS_DISPATCH VTS_PDISPATCH)


	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion
void CFirstAvailList::OnSelChangedFFAResultsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		// (c.haag 2006-11-27 14:55) - PLID 20772 - Update the enabled status of the
		// schedule buttons
		UpdateButtons();

		// (c.haag 2006-11-27 14:56) - PLID 20772 - If the preference is set, take the user
		// to the selected day in the resource view
		if (lpNewSel != NULL && IsDlgButtonChecked(IDC_CHECK_GO_SELECTED_DAY)) {
			OnGoSelectedDay();
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-02) - PLID 41287 - Exception handling
void CFirstAvailList::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		// (c.haag 2006-11-27 13:34) - PLID 20772 - This function is called when
		// the window is displayed or hidden
		CNxDialog::OnShowWindow(bShow, nStatus);
		if (bShow) {
			// (v.maida 2016-02-23 13:24) - PLID 68385 - If this ShowWindow() function has been called due to Practice maximizing, then don't bother
			// calling the below code, because the above CNxDialog::OnShowWindow(bShow, nStatus) call at the beginning of this function will already have
			// called the below code, and appropriately initialized everything, with an nStatus value of 0, which is != SW_PARENTOPENING. 
			
			// A secondary issue is that OnShowWindow runs twice when maximing Practice with FFA open. The first time with nStatus of 3 and then 0. If the below
			// initialization code runs while the FFA Results query is still running it can cause Practice to crash/freeze. Now we check to also see if the thread
			// is finished running before running the below code.
			
			bool bIsFinished = m_pFFASearch == NULL ? true : m_pFFASearch->IsFinished();

			if (nStatus != SW_PARENTOPENING && bIsFinished)
			{
				//TES 1/7/2015 - PLID 64466 - If we're returning a slot, change "Schedule And Close" to "Use Selected Slot"
				//TES 1/7/2015 - PLID 64513 - If we're returning a slot, disable the Go To Selected Day functionality
				if (m_bReturnSlot) {
					SetDlgItemText(IDC_BTN_SCHEDULE, "Use Selected Slot");
					GetDlgItem(IDC_CHECK_GO_SELECTED_DAY)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_GO_SELECTED_DAY)->EnableWindow(FALSE);
				}
				else {
					SetDlgItemText(IDC_BTN_SCHEDULE, "Schedule And Close");
					GetDlgItem(IDC_CHECK_GO_SELECTED_DAY)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_GO_SELECTED_DAY)->EnableWindow(TRUE);
				}
				// (c.haag 2006-11-27 14:54) - PLID 20772 - Set the default value for the "Automatically
				// go to the selected day..." checkbox
				//TES 1/7/2015 - PLID 64513 - Don't check the box if we're returning a slot
				if (GetRemotePropertyInt("AutoGotoApptInFFAListSearch", 1, 0, GetCurrentUserName(), TRUE) && !m_bReturnSlot) {
					((CButton*)GetDlgItem(IDC_CHECK_GO_SELECTED_DAY))->SetCheck(1);
				}
				else {
					((CButton*)GetDlgItem(IDC_CHECK_GO_SELECTED_DAY))->SetCheck(0);
				}

				//(e.lally 2007-01-02) PLID 23812 - By changing the way the first avail list is created, we lost
				//the centered position of that window. I arbitrarily picked the search criteria window as the position
				//we are going to re-center off of. I feel this is acceptable because if for some reason the user moves 
				//that window to the side this window could cover up what they were trying to see behind it. It is 
				//otherwise assumed that the default position for the search criteria window is centered on the screen anyways.

				CRect rcCurListWnd, rcNewListWnd, rcActiveWnd;
				//It is assumed that the search criteria window is still active at this point
				GetActiveWindow()->GetWindowRect(&rcActiveWnd);
				GetWindowRect(&rcCurListWnd);
				// Center the dlgAvailList relative to the firstAvailAppt search criteria window
				rcNewListWnd.left = rcActiveWnd.left + ((rcActiveWnd.Width() / 2) - (rcCurListWnd.Width() / 2));
				rcNewListWnd.right = rcNewListWnd.left + rcCurListWnd.Width();
				rcNewListWnd.top = rcActiveWnd.top + ((rcActiveWnd.Height() / 2) - (rcCurListWnd.Height() / 2));
				rcNewListWnd.bottom = rcNewListWnd.top + rcCurListWnd.Height();

				//Set the new window position to the top, not the topMost as that would place it above all other
				//programs that are currently running and we just want it to be the top practice window.
				SetWindowPos(&wndTop, rcNewListWnd.left, rcNewListWnd.top, 0, 0, SWP_NOSIZE);

				//Get our result list window ready and active before we start the search
				OnDisplayWindow();
			}
		} else {
			//(e.lally 2007-01-02) PLID 23812 - Ensure that hiding this window will activate practice and not
			//a 3rd party application.
			// (v.maida 2016-02-23 13:24) - PLID 68385 - If this ShowWindow() function has been called due to Practice minimizing, then don't bother
			// calling the below code, because the CNxDialog::OnShowWindow(bShow, nStatus) call at the beginning of this function will already have 
			// called the below code, and appropriately handled everything, with an nStatus value of 0, which is != SW_PARENTCLOSING.
			if (nStatus != SW_PARENTCLOSING)
			{
				GetMainFrame()->BringWindowToTop();
				OnHideWindow();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CFirstAvailList::OnDisplayWindow()
{
	GetDlgItem(IDC_BTN_SCHEDULE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_SCHEDULE_KEEP_OPEN)->EnableWindow(FALSE); // (z.manning, 02/21/2007) - PLID 23906
	GetDlgItem(IDC_BTN_SCHEDULE_EDIT)->EnableWindow(FALSE);

	// (r.farnworth 2015-05-18 17:22) - PLID 65633 - Since the dialog never closes we need to clear and repopulate the dropdown on each iteration
	// (r.farnworth 2015-06-05 11:26) - PLID 65635 - We will treat the locations dropdown the same as the resource
	// (v.maida 2016-03-07 11:09) - PLID 68385 - If this function has been called due to Practice maximizing, after being previously minimized, then we
	// don't want to clear out our resource dropdown selections. Check whether we have a current resource selection not set, which would be the case in
	// every situation in which this dialog is displayed, except for when Practice was minimized and them maximized.
	if (m_nCurrentResource == -99)
	{
		m_FilterResourceCombo->Clear();
	}
	m_FilterLocationCombo->Clear();
	m_FilterLocationCombo->Requery();

	// (v.maida 2016-03-07 11:09) - PLID 68385 - Only add the All Resources option if the resource dropdown has been cleared / there is no previously held
	// resource selection.
	NXDATALIST2Lib::IRowSettingsPtr pNewRow;
	if (m_nCurrentResource == -99)
	{
		pNewRow = m_FilterResourceCombo->GetNewRow();
		pNewRow->PutValue(rccID, _variant_t((long)-1));
		pNewRow->PutValue(rccName, _variant_t("{ All Resources }"));
		m_FilterResourceCombo->AddRowSorted(pNewRow, NULL);
	}

	// (r.farnworth 2016-02-19 12:23) - PLID 65635 - In order to ensure  All and Multiple go to the top, we need to wait until the requery is finished before adding them
	m_FilterLocationCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// (v.maida 2016-02-18 10:17) - PLID 68368 - Added multiple location option.
	if (m_pSearchSettings->m_arynAptLocations.GetCount() > 1)
	{
		// there are multiple locations, so we need to add a special row consisting of all of the locations' names
		CString strMultiLocationNames = "{ ";

		for (int i = 0; i < m_pSearchSettings->m_arynAptLocations.GetCount(); i++)
		{
			// find all of the location names for all of the selected locations
			NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_FilterLocationCombo->FindByColumn(lccID, _variant_t(m_pSearchSettings->m_arynAptLocations[i]), NULL, VARIANT_FALSE);
			if (pLocRow != NULL)
			{
				// we've found the row corresponding to the location within the m_arynAptLocations array, so now grab that location's name
				strMultiLocationNames += VarString(pLocRow->GetValue(lccName)) + ", ";

			}
		}

		strMultiLocationNames = strMultiLocationNames.Trim(", ") + " }";
		pNewRow = m_FilterLocationCombo->GetNewRow();
		pNewRow->PutValue(lccID, _variant_t((long)-2));
		pNewRow->PutValue(lccName, _variant_t(strMultiLocationNames));
		// (r.farnworth 2016-02-19 12:23) - PLID 65635 - changed to AddRowBefore
		m_FilterLocationCombo->AddRowBefore(pNewRow, m_FilterLocationCombo->GetTopRow());
	}

	// (r.farnworth 2016-02-19 12:23) - PLID 65635 - After adding sortability to the dropdown, {All} was no longer being set to the top
	pNewRow = m_FilterLocationCombo->GetNewRow();
	pNewRow->PutValue(lccID, _variant_t((long)-1));
	pNewRow->PutValue(lccName, _variant_t("{ All Locations }"));
	m_FilterLocationCombo->AddRowBefore(pNewRow, m_FilterLocationCombo->GetTopRow());

	// (v.maida 2016-02-24 15:22) - PLID 68385 - If there is not a current resource value filled in, then just use the All Resources option, otherwise use that resource.
	m_FilterResourceCombo->SetSelByColumn(rccID, m_nCurrentResource == -99 ? -1 : m_nCurrentResource);
	// (r.farnworth 2015-06-08 10:23) - PLID 65635 - We need to default to the location selected on the intial FFA dialog
	// (v.maida 2016-02-18 10:17) - PLID 68368 - If there are multiple locations, then select the multiple location entry.
	// (v.maida 2016-02-24 15:22) - PLID 68385 - If there is not a current location value filled in, then just use the location info in the search 
	// settings, otherwise use the currently selected location.
	if (m_nCurrentLocation == -99)
	{
		m_FilterLocationCombo->SetSelByColumn(lccID, (m_pSearchSettings->m_arynAptLocations.GetCount() > 1 ? -2 : m_pSearchSettings->m_arynAptLocations[0]));
	}
	else 
	{
		m_FilterLocationCombo->SetSelByColumn(lccID, m_nCurrentLocation);
	}

	PopulateList();
}

void CFirstAvailList::OnHideWindow()
{	
	// (c.haag 2006-11-27 13:34) - PLID 20772 - This function clears everything
	// except the list control. This is because when this is hidden, and Practice
	// redraws, the user would see that the list is empty for a few seconds. It 
	// looks awkward.

	// (v.maida 2016-02-19 16:34) - PLID 68385 - If we've got here because Practice has been minimized, then don't clear the window, because it's still
	// open and may still be used when Practice is maximized, otherwise the window has been closed normally, so clear it.
	BOOL bClear = !GetMainFrame()->m_bPracticeMinimized;
	if (bClear) {
		Clear();
	}
}

void CFirstAvailList::UpdateButtons()
{
	//
	// (c.haag 2006-08-01 12:18) - PLID 21542 - Keep the schedule button disabled if we are not allowed
	// to schedule appointments due to the absence of types or purposes
	//
	BOOL bEnabled;
	// (d.thompson 2010-11-02) - PLID 41284 - dl2 conversion
	if (NULL == m_pResults->GetCurSel()) {
		bEnabled = FALSE;
	} else {
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes for both
		// (r.gonet 2014-12-17) - PLID 64465 - Search settings are now part of the new CFFASearchSettings object.
		// (r.farnworth 2016-02-18 08:41) - PLID 68247 - Remove traces of unneeded DWORD objects
		if (!((-1 == m_pSearchSettings->m_nAptType && GetRemotePropertyInt("ResEntryRequireAptType", 1, 0, "<None>", true)) ||
			(-1 == m_pSearchSettings->m_nAptPurpose && GetRemotePropertyInt("ResEntryRequireAptPurpose", 1, 0, "<None>", true)))) {
			bEnabled = TRUE;
		} else {
			bEnabled = FALSE;
			
		}
	}
	GetDlgItem(IDC_BTN_SCHEDULE)->EnableWindow(bEnabled);
	//TES 1/7/2015 - PLID 64466 - Disable the "Schedule and Keep Open" and "Schedule and Edit Further" buttons if we're returning a slot
	GetDlgItem(IDC_BTN_SCHEDULE_KEEP_OPEN)->EnableWindow(bEnabled && !m_bReturnSlot); // (z.manning, 02/21/2007) - PLID 23906
	GetDlgItem(IDC_BTN_SCHEDULE_EDIT)->EnableWindow(bEnabled && !m_bReturnSlot);
}

LRESULT CFirstAvailList::OnFFAListRequeryFinished(WPARAM wParam, LPARAM lParam)
{
	try
	{
		//
		// (c.haag 2006-11-27 13:46) - PLID 20772 - These functions that used to be called in the
		// worker thread, which was evil. I added the Invalidate call because sometimes the datalist
		// does not redraw after the progress window is hidden (it may have been because the
		// functions were called in a thread, but I'm adding it anyway)
		//
		// (d.thompson 2010-11-01) - PLID 41274 - Progress bar no longer in a window, it's on this dialog.
		//m_dlgProgress.ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_AVAILABLE_LIST)->Invalidate();

		ConfigureResourceControls(); // (r.farnworth 2015-05-21 10:58) - PLID 65633
		GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
		// (z.manning 2010-11-03 17:18) - PLID 41272 - Don't call SetFocus if this window is no longer visible.
		if(IsWindowVisible()) {
			SetFocus();
		}

	}NxCatchAll("CFirstAvailList::OnFFAListRequeryFinished");
	return 0;
}

// (z.manning 2010-11-01 15:53) - PLID 41272
LRESULT CFirstAvailList::OnFFAListProcessData(WPARAM wParam, LPARAM lParam)
{
	try
	{
		FirstAvailListThreadData *pFFAData = (FirstAvailListThreadData*)wParam;
		foreach(FFAOpening *pOpening, pFFAData->arypOpenings)
		{
			// (j.politis 2015-07-09 11:51) - PLID 65629 - Add a new “Location” column in the results list after the “Resources” column.
			TryAddApptToList(pOpening->dt, pOpening->strResourceIDs, pOpening->strResourceNames, pOpening->strLocationName, pOpening->nLocationID, pOpening->strTemplateNames);
		}

		pFFAData->CleanUp();
		delete pFFAData;
		// (r.farnworth 2015-07-14 12:28) - PLID 65635 - We need to filter based on default selections of both dropdowns
		// (j.politis 2015-07-14 17:23) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
		FilterResultsList();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (d.thompson 2010-11-01) - PLID 41274 - wParam = Min, lparam = Max.  Both need to be able to fit in a 'short' integer.
LRESULT CFirstAvailList::OnSetFFAProgressMinMax(WPARAM wParam, LPARAM lParam)
{
	try {
		//Constrain the max value to a 'short'
		long nMax = (long)lParam;
		if(nMax > (long)MAXSHORT) {
			nMax = MAXSHORT;
		}

		m_ctrlProgressBar.SetRange((short)wParam, (short)nMax);
		//Default the range to the min
		m_ctrlProgressBar.SetPos(0);

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (d.thompson 2010-11-01) - PLID 41274 - wParam is a 'short' integer for the current position, from the range given
LRESULT CFirstAvailList::OnSetFFAProgressPosition(WPARAM wParam, LPARAM lParam)
{
	try {
		long nNewPos = (long)wParam;
		//Constrain the new value to a 'short'
		if(nNewPos > (long)MAXSHORT) {
			nNewPos = MAXSHORT;
		}

		m_ctrlProgressBar.SetPos((short)nNewPos);

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (d.thompson 2010-11-01) - PLID 41274 - wParam is a BSTR, allocated by the Post'er.  This function will free the string.
LRESULT CFirstAvailList::OnSetFFAProgressText(WPARAM wParam, LPARAM lParam)
{
	try {
		//Memory management:  The BSTR was allocated by the caller, and must be freed by us.
		BSTR bstr = (BSTR)wParam;
		CString strText(bstr);

		SetDlgItemText(IDC_FFA_PROGRESS_TEXT, strText);
		SysFreeString(bstr);

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (r.farnworth 2015-05-18 17:25) - PLID 65633 - Break off setting-up the dropdown and buttons into its own function
void CFirstAvailList::ConfigureResourceControls()
{
	// (r.farnworth 2015-05-18 14:16) - PLID 65634 - If the user selects multiple resources, and specifies that all of the resources must be available during the time slot, this filter should be disabled. 
	// (r.farnworth 2015-05-18 16:06) - PLID 65633 - Disable dropdown as well
	// (r.farnworth 2015-05-18 16:06) - PLID 65633 - Disable dropdown if Any is the only option to be selected
	long nResourceCount = m_pSearchSettings->m_nNumResources;
	long nDropdownRowCount = m_FilterResourceCombo->GetRowCount();
	BOOL bAnyOpenResource = m_pSearchSettings->m_bAnyOpenResource;
	if ((nResourceCount > 1 && !bAnyOpenResource) || nDropdownRowCount <= 1)
	{
		GetDlgItem(IDC_FILTER_RESOURCE_LEFT)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILTER_RESOURCE_RIGHT)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILTER_RESOURCE_COMBO)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_FILTER_RESOURCE_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILTER_RESOURCE_LEFT)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILTER_RESOURCE_RIGHT)->EnableWindow(TRUE);
	}

}

// (r.farnworth 2015-05-18 12:36) - PLID 65634 - Left/right buttons to filter results by resource in FFA results.
void CFirstAvailList::OnMoveResourceLeft()
{
	//Move the current selection of the dropdown UP by one
	//Refresh the list to filter on this new results
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterResourceCombo->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pUpRow;

	if (pRow == NULL) 
	{
		ASSERT(FALSE);
		return;
	}

	if (pRow == m_FilterResourceCombo->GetFirstRow())
	{
		pUpRow = m_FilterResourceCombo->GetLastRow();
	}
	else 
	{
		pUpRow = pRow->GetPreviousRow();
	}

	if (pUpRow != NULL) 
	{	
		m_FilterResourceCombo->PutCurSel(pUpRow);
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current resource selection.
		m_nCurrentResource = VarLong(pUpRow->GetValue(rccID));
	}
	else 
	{
		ASSERT(FALSE);
		return;
	}

	// (r.farnworth 2015-07-14 12:44) - PLID 65635 - Added Location Dropdown Row
	// (j.politis 2015-07-14 17:23) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
	FilterResultsList();
}

// (r.farnworth 2015-05-18 12:36) - PLID 65634 - Left/right buttons to filter results by resource in FFA results.
void CFirstAvailList::OnMoveResourceRight()
{
	//Move the current selection of the dropdown DOWN by one
	//Refresh the list to filter on this new results
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterResourceCombo->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pDownRow;

	if (pRow == NULL) 
	{
		//should not be possible
		ASSERT(FALSE);
		return;
	}

	if (pRow == m_FilterResourceCombo->GetLastRow())
	{
		pDownRow = m_FilterResourceCombo->GetFirstRow();
	}
	else
	{
		pDownRow = pRow->GetNextRow();
	}

	if (pDownRow != NULL)
	{
		m_FilterResourceCombo->PutCurSel(pDownRow);
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current resource selection.
		m_nCurrentResource = VarLong(pDownRow->GetValue(rccID));
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	// (r.farnworth 2015-07-14 12:44) - PLID 65635 - Added Location Dropdown Row
	// (j.politis 2015-07-14 17:23) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
	FilterResultsList();
}

// (r.farnworth 2015-05-18 16:01) - PLID 65633 - Dropdown in FFA results to filter by resource. The dropdown list will contain only resources referenced in any results, alphabetical by name, plus < All Resources > at the top
void CFirstAvailList::OnSelChosenResourceListCombo(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if (pRow)
	{
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current resource selection.
		m_nCurrentResource = VarLong(pRow->GetValue(rccID));
		// (j.politis 2015-07-13 17:20) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
		//Filter the results list based on our selection 
		FilterResultsList();
	}
}

// (r.farnworth 2015-05-19 10:58) - PLID 65633 - Dropdown in FFA results to filter by resource. The dropdown list will contain only resources referenced in any results, alphabetical by name, plus < All Resources > at the top
// (j.politis 2015-07-14 17:23) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
void CFirstAvailList::FilterResultsList()
{
	//iterate through the list and hide any row whose resource does not match that of strDropDownResource
	NXDATALIST2Lib::IRowSettingsPtr pListRow = m_pResults->GetFirstRow();

	//Get resource dropdown id
	long nSelResourceID = VarLong(m_FilterResourceCombo->GetFirstSelRow()->GetValue(rccID));
	long nSelLocationID = VarLong(m_FilterLocationCombo->GetFirstSelRow()->GetValue(lccID));

	if (pListRow)
	{

		//Iterate through every row in the results list
		while (pListRow != NULL)
		{
			//If the selected resource is present in the list of resources for a time slot, or the selected resource is < All Resources >, then show the row.
			CDWordArray aryDataListIDs;
			LoadResourceIDStringIntoArray(pListRow->GetValue(elcResourceIDs), aryDataListIDs);

			pListRow->PutVisible
				(
					ShouldRowBeVisible
					(
						nSelResourceID
						, aryDataListIDs
						, nSelLocationID
						, pListRow->GetValue(elcLocationID)
					) ? VARIANT_TRUE : VARIANT_FALSE
				);

			pListRow = pListRow->GetNextRow();
		}

		m_pResults->SetRedraw(VARIANT_TRUE);
		//Snap back to the top of the list
		m_pResults->EnsureRowInView(m_pResults->FindAbsoluteFirstRow(VARIANT_TRUE));
	}
}

// (r.farnworth 2015-06-05 11:37) - PLID 65635 - Dropdown in FFA results to filter by location. The dropdown list will contain only locations referenced in any results, alphabetical by name, plus < All Locations > at the top
void CFirstAvailList::OnSelChosenLocationListCombo(LPDISPATCH lpRow)
{
	//Filter the locations list based on our selection 
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if (pRow)
	{
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current location selection.
		m_nCurrentLocation = VarLong(pRow->GetValue(lccID));
		// (j.politis 2015-07-13 17:20) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
		FilterResultsList();
	}
}

// (r.farnworth 2015-06-05 12:58) - PLID 65636 - Left/right buttons to filter results by location in FFA results.
void CFirstAvailList::OnMoveLocationLeft()
{
	//Move the current selection of the dropdown UP by one
	//Refresh the list to filter on this new results
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterLocationCombo->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pUpRow;

	if (pRow == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pRow == m_FilterLocationCombo->GetFirstRow())
	{
		pUpRow = m_FilterLocationCombo->GetLastRow();
	}
	else
	{
		pUpRow = pRow->GetPreviousRow();
	}

	if (pUpRow != NULL)
	{
		m_FilterLocationCombo->PutCurSel(pUpRow);
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current location selection.
		m_nCurrentLocation = VarLong(pUpRow->GetValue(lccID));
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	// (j.politis 2015-07-14 17:26) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
	FilterResultsList();
}

// (r.farnworth 2015-06-05 13:01) - PLID 65636 - Left/right buttons to filter results by location in FFA results.
void CFirstAvailList::OnMoveLocationRight()
{
	//Move the current selection of the dropdown DOWN by one
	//Refresh the list to filter on this new results
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterLocationCombo->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pDownRow;

	if (pRow == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pRow == m_FilterLocationCombo->GetLastRow())
	{
		pDownRow = m_FilterLocationCombo->GetFirstRow();
	}
	else
	{
		pDownRow = pRow->GetNextRow();
	}

	if (pDownRow != NULL)
	{
		m_FilterLocationCombo->PutCurSel(pDownRow);
		// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current location selection.
		m_nCurrentLocation = VarLong(pDownRow->GetValue(lccID));
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	// (j.politis 2015-07-14 17:26) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
	FilterResultsList();
}

// (r.farnworth 2015-05-18 12:36) - PLID 65634
void CFirstAvailList::OnDoubleClickResourceLeft()
{
	try {
		OnMoveResourceLeft();
	} NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2015-05-18 12:36) - PLID 65634
void CFirstAvailList::OnDoubleClickResourceRight()
{
	try {
		OnMoveResourceRight();
	} NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-02-01 10:20) - PLID 65636
void CFirstAvailList::OnDoubleClickLocationLeft()
{
	try {
		OnMoveLocationLeft();
	} NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-02-01 10:20) - PLID 65636
void CFirstAvailList::OnDoubleClickLocationRight()
{
	try {
		OnMoveLocationRight();
	} NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-02-02 09:46) - PLID 65633 - Need to snap back to the previous selection when they accidentally selected NULL
void CFirstAvailList::OnSelChangingUnselectedResource(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Check your pointer...
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);
		if (!pRow) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll("Error checking OnSelChangingUnselectedResource.");
}

// (r.farnworth 2016-02-02 09:47) - PLID 65635 - Need to snap back to the previous selection when they accidentally selected NULL
void CFirstAvailList::OnSelChangingUnselectedLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Check your pointer...
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);
		if (!pRow) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll("Error checking OnSelChangingUnselectedLocation.");
}

// (j.politis 2015-07-13 17:20) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
// (v.maida 2016-02-18 10:17) - PLID 68368 - Made this function a part of the FirstAvailList class.
bool CFirstAvailList::ShouldRowBeVisible_ByResource(long nSelectedResourceID, const CDWordArray &aryDataListIDs)
{
	//If we have  {All Resources} selected, everything should be visible
	if (nSelectedResourceID == -1)
	{
		return true;
	}

	//Go through the list of resources associated with this row. If any of them match our dropdown choice, we need to make the row visible.
	for (int i = 0; i < aryDataListIDs.GetCount(); i++)
	{
		if (aryDataListIDs.GetAt(i) == nSelectedResourceID) {
			return true;
		}
	}

	return false;
}

// (j.politis 2015-07-13 17:20) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
// (v.maida 2016-02-18 10:17) - PLID 68368 - Made this function a part of the FirstAvailList class.
// (v.maida 2016-03-14 17:03) - PLID 68448 - Removed bOnlyWithLocation parameter, since the API now handles filtering or not filtering location-less results.
bool CFirstAvailList::ShouldRowBeVisible_ByLocation(long nSelectedLocationID, const _variant_t &varLocationID)
{
	// (r.farnworth 2016-01-26 14:49) - PLID 65637 - If the time slot has a location, only display the row if that location or {All} is selected in the dropdown
	// (v.maida 2016-03-14 17:03) - PLID 68448 - The API now handles filtering out NULL / location-less slots, so we can allow them hear, but we won't
	// actually have any to allow unless the preference check on the API side lets them through.
	if ((varLocationID.vt != VT_NULL && (VarLong(varLocationID) == nSelectedLocationID || nSelectedLocationID == -1))
		|| (varLocationID.vt == VT_NULL) )
	{
		return true;
	}
	// (v.maida 2016-02-18 10:17) - PLID 68368 - If the multiple location option is selected, then only show those rows related to the selected multiple locations.
	else if (varLocationID.vt != VT_NULL && nSelectedLocationID == -2)
	{
		for (int i = 0; i < m_pSearchSettings->m_arynAptLocations.GetCount(); i++)
		{
			// iterate through all locations, and see if the current location ID matches any of them, return true if so.
			if (VarLong(varLocationID) == m_pSearchSettings->m_arynAptLocations[i])
			{
				// this row's location ID matches one of the multiple selected locations
				return true;
			}
		}
		return false;
	}
	else
	{
		return false;
	}
}

// (j.politis 2015-07-13 17:20) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
// (v.maida 2016-02-18 10:17) - PLID 68368 - Made this function a part of the FirstAvailList class.
// (v.maida 2016-03-14 17:03) - PLID 68448 - Removed bOnlyWithLocation parameter, since the API now handles filtering or not filtering location-less results.
bool CFirstAvailList::ShouldRowBeVisible(long nSelectedResourceID, const CDWordArray &aryDataListIDs, long nSelectedLocationID, const _variant_t &varLocationID)
{
	return ShouldRowBeVisible_ByLocation(nSelectedLocationID, varLocationID)
		&& ShouldRowBeVisible_ByResource(nSelectedResourceID, aryDataListIDs);
}