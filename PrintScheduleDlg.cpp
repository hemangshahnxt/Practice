// PrintScheduleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PrintScheduleDlg.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "Reports.h"
#include "ProcessRptDlg.h"
#include "GlobalReportUtils.h"
//#include "NxRecordset.h"
#include "ReportInfo.h"
#include "SchedulerRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrintScheduleDlg dialog

using namespace ADODB;
CPrintScheduleDlg::CPrintScheduleDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrintScheduleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrintScheduleDlg)
	//}}AFX_DATA_INIT
	m_nPurposeSetId = -1;

	m_bShowApptShowState = FALSE;
	m_bShowMonthProcedures = FALSE;
	m_bShowTemplateColors = FALSE;
	m_bClassicShowCancelled = FALSE;
	m_bUseLandscape = TRUE;
	m_bViewedResources = FALSE;

	m_pmPrintMode = CPrintScheduleDlg::pmGraphical;
}


void CPrintScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrintScheduleDlg)
	DDX_Control(pDX, IDC_PREVIEW_BTN, m_btnPreview);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SCHED_PRINT_STYLE_GROUPBOX, m_btnSchedPrintStyleGroupbox);
	DDX_Control(pDX, IDC_ORIENTATION_STATIC, m_btnOrientationStatic);
	DDX_Control(pDX, IDC_PROPERTIES_GROUPBOX, m_btnPropertiesGroupbox);
	DDX_Control(pDX, IDC_GRAPHICAL_RADIO, m_radioGraphical);
	DDX_Control(pDX, IDC_TEXTUAL_RADIO, m_radioTextual);
	DDX_Control(pDX, IDC_PORTRAIT_RADIO, m_radioPortrait);
	DDX_Control(pDX, IDC_LANDSCAPE_RADIO, m_radioLandscape)	;
	DDX_Control(pDX, IDC_CHECK_SHOW_CANCELLED_APPTS, m_checkShowCancelledAppts);
	DDX_Control(pDX, IDC_CHECK_PRINT_PENDING, m_checkPrintPending);
	DDX_Control(pDX, IDC_CHECK_PRINT_TEMPLATE_COLOR, m_checkPrintTemplateColor);
	DDX_Control(pDX, IDC_PRINT_GRID, m_checkPrintGrid);
	DDX_Control(pDX, IDC_CHECK_VIEWED_RESOURCES, m_checkViewedResources);
	DDX_Control(pDX, IDC_CHECK_PRINT_PROCEDURES, m_checkPrintProcedures);
	DDX_Control(pDX, IDC_CHECK_PRINT_TEMPLATES, m_checkPrintTemplates);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrintScheduleDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPrintScheduleDlg)
	ON_BN_CLICKED(IDC_PREVIEW_BTN, OnPreviewBtn)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GRAPHICAL_RADIO, OnGraphicalRadio)
	ON_BN_CLICKED(IDC_TEXTUAL_RADIO, OnTextualRadio)
	ON_BN_CLICKED(IDC_CHECK_PRINT_TEMPLATE_COLOR, OnPrintTemplateClr)
	ON_BN_CLICKED(IDC_CHECK_PRINT_PENDING, OnPrintPending)
	ON_BN_CLICKED(IDC_CHECK_PRINT_PROCEDURES, OnPrintProcedures)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CANCELLED_APPTS, OnShowCancelledAppts)
	ON_BN_CLICKED(IDC_LANDSCAPE_RADIO, OnLandscapeRadio)
	ON_BN_CLICKED(IDC_PORTRAIT_RADIO, OnPortraitRadio)
	ON_BN_CLICKED(IDC_PRINT_GRID, OnPrintGrid)
	ON_BN_CLICKED(IDC_CHECK_VIEWED_RESOURCES, OnCheckViewedResources)
	ON_BN_CLICKED(IDC_CHECK_PRINT_TEMPLATES, OnCheckPrintTemplates)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintScheduleDlg message handlers

void CPrintScheduleDlg::OnPreviewBtn() 
{
	//(e.lally 2008-11-18) PLID 32070 - Added try/catch
	try{
		//PLID 20056 - save whether they chose graphical or text
		SetRemotePropertyInt("SchedulerPrintType", GetPrintMode(), 0, GetCurrentUserName());

		switch (GetPrintMode()) {
		case pmGraphical:
			{
				bool bNewDevNames = false, bNewInfoDevMode = false;

				// Set the paper orientation
				ASSERT(m_pInfo && m_pInfo->m_pPD);
				if (m_pInfo && m_pInfo->m_pPD) {

					// First make sure we have the devmode and devnames objects as copies of the app's settings
					if (m_pInfo->m_pPD->m_pd.hDevMode == NULL || m_pInfo->m_pPD->m_pd.hDevNames == NULL) {
						// Get a copy of the app's device settings, we will use these to set the report's device settings after making our adjustments
						DEVMODE *pFinDevMode = NULL;
						LPTSTR strPrinter = NULL, strDriver = NULL, strPort = NULL;
						AllocCopyOfAppDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
						
						// dev MODE
						if (m_pInfo->m_pPD->m_pd.hDevMode == NULL) {
							// The devmode object isn't set, so set it to a copy of the app's
							if(pFinDevMode) {
								m_pInfo->m_pPD->m_pd.hDevMode = AllocDevModeCopy(pFinDevMode);
								bNewInfoDevMode = true;	//drt
							}
						}
						// dev NAMES
						if (m_pInfo->m_pPD->m_pd.hDevNames == NULL) {
							// The devnames object isn't set, so set it to a copy of the app's
							if(strPrinter != NULL && strDriver != NULL && strPort != NULL) {
								m_pInfo->m_pPD->m_pd.hDevNames = AllocDevNamesCopy(strPrinter, strDriver, strPort);
								bNewDevNames = true;	//drt
							}
						}

						if(!bNewDevNames || !bNewDevNames) {
							//printer device copying failed.  Quit out
							AfxMessageBox("Unable to determine printer settings.  Please ensure a printer is installed and able to be accessed and try again.");
							return;
						}

						// We're done with our copies because we've made a second set of copies and stored them in the m_pInfo object
						FreeCopyOfDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
					}

					// Then set the page orientation correctly
					DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(m_pInfo->m_pPD->m_pd.hDevMode);
					ASSERT(pInfoDevMode);
					pInfoDevMode->dmOrientation = m_bUseLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
					GlobalUnlock(m_pInfo->m_pPD->m_pd.hDevMode);
				} else {
					int nResult = AfxMessageBox(
						"Your page orientation selection could not be applied.  If you "
						"continue, the current default orientation will be used.", 
						MB_OKCANCEL|MB_ICONEXCLAMATION);
					if (nResult != IDOK) {
						// They chose not to continue
						return;
					}
				}
				CNxDialog::OnOK();
			}
			break;
		case pmReport:
			{
				// Print the report
				BeginWaitCursor();
				OpenReport();
				EndWaitCursor();
				CNxDialog::OnOK();
			}
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("CPrintScheduleDlg::OnPreviewBtn - Error previewing the scheduler report");
}


void CPrintScheduleDlg::SetCenterDate(COleDateTime dateBase, COleDateTime dateEnd, COleDateTime &dateFrom, COleDateTime &dateTo)
{
	int Month, Year;

	dateFrom.GetCurrentTime();
	dateTo.GetCurrentTime();
    
	switch (m_nPrintExtent) {
	case 0: // daily
	case 3: // resource 

		// Set single day dates
		dateFrom = dateTo = dateBase;
		break;

	case 1: // If Week report

		//TES 5/27/03: Use whatever dates are passed in.
		dateFrom.SetDate (dateBase.GetYear(), dateBase.GetMonth(), dateBase.GetDay() );
		dateTo.SetDate (dateEnd.GetYear(), dateEnd.GetMonth(), dateEnd.GetDay() );

		break;

	case 2: // If Month report

		//TES 5/9/03: We only care about date to, since this is a oneday report
		
		Month = dateBase.GetMonth();
		Year = dateBase.GetYear();

		/*dateFrom.SetDate (Year, Month, 1);

		if (Month == 12) {
			Month = 1;
			Year++;
		}
		else {
			Month++;
		}*/
		//dateTo.SetDate (Year, Month, 1);
		dateTo.SetDate (Year, Month, 1);

		break;

	}

}


BOOL CPrintScheduleDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnPreview.AutoSet(NXB_PRINT_PREV);
	m_btnCancel.AutoSet(NXB_CANCEL);

	Init();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrintScheduleDlg::OpenReport()
{
	COleDateTime	dateTo, 
					dateFrom;
	COleDateTimeSpan span;
	CString			strReportName, 
					filter, 
					ResFilter, 
					currRes,
					Alias;

/*	CNxRecordset	rcset(NULL, &g_dbPractice);
	
	try {
		rcset.Open(dbOpenSnapshot, "SELECT * FROM ReportsT WHERE ID = 17");
		rcset.MoveFirst();

		Alias = rcset["Alias"].pcVal;

		rcset.Close();

	} catch (CDaoException *e) {
		e->ReportError();
		e->Delete();
	}*/

	SetCenterDate (m_dtReportCenterDate, m_dtReportEndDate, dateFrom, dateTo);

	// crystal execution

	CPtrArray paramList;
	CRParameterInfo *item;
	//this is done because the daily schedule has a different query than the other tabs
	long nReportInfoIndex;
	if (m_nPrintExtent == 0)  {
		nReportInfoIndex = 380;
	}
	else if (m_nPrintExtent == 1) {
		//Use the PP version of the weekly schedule.
		nReportInfoIndex = 393;
	}
	else if (m_nPrintExtent == 2) {
		//Use the PP version of the monthly schedule
		nReportInfoIndex = 394;
	}
	else if (m_nPrintExtent == 3) {
		//Use the resource version
		nReportInfoIndex = 416;
	}
	else {
		ASSERT(FALSE);
		//nReportInfoIndex = 239;
	}

	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportInfoIndex)]);
	BeginWaitCursor();
	CProcessRptDlg Progressdlg(this);
	CRect msgRect;
	Progressdlg.Create(IDD_PROCESSINGDLG, NULL);
	Progressdlg.ShowWindow(SW_SHOW);
	GetParent()->GetWindowRect(msgRect);
	msgRect.left = (msgRect.Width() / 2) - 100;
	msgRect.right = msgRect.left + 200;
	msgRect.top = (msgRect.Height() / 2) - 50;
	msgRect.bottom = msgRect.top + 100;
	Progressdlg.SetWindowPos(&wndTop, msgRect.left, msgRect.top, msgRect.Width(), msgRect.Height(), SWP_NOSIZE | SWP_NOZORDER);
	CString ToDate, FromDate;
	long ToYear, ToMonth, ToDay, FromYear, FromMonth, FromDay;

	ToYear = dateTo.GetYear();
	ToMonth = dateTo.GetMonth();
	ToDay = dateTo.GetDay();
	FromYear = dateFrom.GetYear();
	FromMonth = dateFrom.GetMonth();
	FromDay = dateFrom.GetDay();

	Alias = infReport.strRecordSource;


	// Generate the date filters
	if (m_nPrintExtent == 3) {
		//DateTime() function needed only for resource report
		ToDate.Format("DateTime(%li,%li,%li)", ToYear, ToMonth, ToDay); 
		FromDate.Format("DateTime(%li,%li,%li)", FromYear, FromMonth, FromDay);

		filter.Format("%s <= {%s.date} AND %s >= {%s.date}",  FromDate, Alias, ToDate, Alias);
		//filter.Format("%s >= {%s.date} AND %s <= {%s.date}", FromDate, Alias, ToDate, Alias);

	} else {
		//currRes.Format ("%d", GetRemotePropertyInt ("ResCurItemID", -1, 0, GetCurrentUserName()));
		
		//Date() function needed for crystal filter
		ToDate.Format("Date(%li,%li,%li)", ToYear, ToMonth, ToDay); 
		FromDate.Format("Date(%li,%li,%li)", FromYear, FromMonth, FromDay);
		
		filter.Format("{%s.ID} = %d AND %s <= {%s.date} AND %s >= {%s.date}", Alias, GetRemotePropertyInt ("ResCurItemID", -1, 0, GetCurrentUserName()), FromDate, Alias, ToDate, Alias);
		//filter = "{" + Alias + ".ID} = " + currRes + " AND " +  dateFrom >= "{" + Alias + ".date} AND " + dateTo + <= "{" + Alias + ".date};
	}
/*
	// Get the appointment type to see if we need to add that filter
	if (m_nPurposeSetId != -1) {
		// Filter on appointment type
		CString strApptTypeFilter;
		strApptTypeFilter.Format("{%s.SetID} = %li", Alias, m_nPurposeSetId);
		filter += " AND " + strApptTypeFilter;
	}
*/
	// Set the parameter information
	item = new CRParameterInfo;
	item->m_Data = GetCurrentUserName();
	item->m_Name = "CurrentUser";
	paramList.Add((void *)item);

	item = new CRParameterInfo;
	CString strLang;
	strLang.Format("%li", GetUserDefaultLangID());
	item->m_Data = strLang;
	item->m_Name = "CurrentLanguage";
	paramList.Add ((void *)item);


	item = new CRParameterInfo;
	item->m_Data = dateFrom.Format("%m/%d/%Y");
	item->m_Name = "DateFrom";
	paramList.Add ((void *)item);

	item = new CRParameterInfo;
	item->m_Data = dateTo.Format("%m/%d/%Y");
	item->m_Name = "DateTo";
	paramList.Add ((void *)item);

	item = new CRParameterInfo;
	item->m_Data = GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) ? "true" : "false";
	item->m_Name = "ShowLocName";
	paramList.Add ((void *)item);

	//date filters
	infReport.nDateRange = 2;
	infReport.DateFrom.SetDate(FromYear, FromMonth, FromDay);
	infReport.DateTo.SetDate(ToYear, ToMonth, ToDay);
	//(a.wilson 2012-2-16) PLID 39893 - passing on the location id.
	// (r.gonet 06/10/2013) - PLID 56503 - Location can now also tell us that we have multiple locations, analogous to how the CReportInfo does the locations.
	if(m_dwLocations.GetSize() == 0) {
		// (r.gonet 06/10/2013) - PLID 56503 - We don't need to set anything for location. By default, the report will be filtered on all locations.
	} else if(m_dwLocations.GetSize() == 1) {
		// (r.gonet 06/10/2013) - PLID 56503 - If infReport.nLocation is positive, then it will be interpretted as the only location ID to filter on.
		infReport.nLocation = m_dwLocations.GetAt(0);
	} else if(m_dwLocations.GetSize() > 1) {
		// (r.gonet 06/10/2013) - PLID 56503 - -3 tells Practice to use a location filter with multiple location IDs found in the m_dwLocations variable.
		infReport.nLocation = -3;
		infReport.m_dwLocations.Copy(m_dwLocations);
	}

	//this will be -1 if all values are to be printed, the ID of the PurposeSet otherwise
	infReport.nExtraID = m_nPurposeSetId;
	if(m_nPurposeSetId != -1) {
		CString strExtraValue;
		strExtraValue.Format("%li", m_nPurposeSetId);
		infReport.SetExtraValue(strExtraValue);
	}
	if(m_nPurposeSetId == -2) {
		//-2 means multiple types.  We should format the strExtraValue parameter to be
		//a comma delimited list of all ids, then the report can filter using that
		//TES 8/22/2005 - We support this now!
		infReport.ClearExtraValues();
		int nLastComma = 0;
		int nNextComma = m_strMultiTypeIds.Find(",");
		while(nNextComma != -1) {
			infReport.AddExtraValue(m_strMultiTypeIds.Mid(nLastComma,nNextComma-nLastComma));
			nLastComma = nNextComma+1;
			nNextComma = m_strMultiTypeIds.Find(",",nLastComma);
		}
		CString strAfterLast = m_strMultiTypeIds.Mid(nLastComma);
		if(!strAfterLast.IsEmpty()) infReport.AddExtraValue(strAfterLast);
	}

	//send the resource via the provider
	if(m_nPrintExtent != 3)
		infReport.nProvider = atoi(m_strActiveItemID);

	//I'm really running out of places to send data through
	//this sends the "show cancelled or not" through the patient id
	//TES 12/19/2003: This is silly, and was causing problems, I just changed the report code to check the ConfigRT setting.
	/*if(((CButton *)GetDlgItem(IDC_CHECK_SHOW_CANCELLED_APPTS))->GetCheck())
		infReport.nPatient = 1;
	else
		infReport.nPatient = -1;*/

	//DRT 2/9/2004 - PLID 10848 - Did the same thing as the above for allowing them to filter on 
	//	the current resource view only, the report checks the configrt setting


	OnOK();


	/*3/20/03 JMM  changed this back to run the different reports so that only the daily report can be editable
	and so that the reports can have different queries*/

	/*if(m_pInfo->m_bPreview)
		infReport.ViewReport("Schedule", "SchAppts", &paramList, true, (CWnd *)this);
	else
		infReport.ViewReport ("Schedule", "SchAppts", &paramList, false, (CWnd *)this);*/

	if (m_nPrintExtent == 0) {  //daily report 

		//Made new function for running reports - JMM 5-28-04
		// (a.walling 2009-07-02 10:34) - PLID 14181 - Need to pass in our printer info!
		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, &paramList, m_pInfo->m_bPreview?true:false, (CWnd *)this, "Schedule", m_pInfo, TRUE);
	}
	else if (m_nPrintExtent == 1) { //weekly report
		//simulate checking the date range button.
		infReport.nDateFilter = 1;

		//Made new function for running reports - JMM 5-28-04
		// (a.walling 2009-07-02 10:34) - PLID 14181 - Need to pass in our printer info!
		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, &paramList, m_pInfo->m_bPreview?true:false, (CWnd *)this, "Schedule", m_pInfo, TRUE);
	
	}
	else if (m_nPrintExtent == 2) { //monthly report
		//simulate checking the date range button.
		infReport.nDateFilter = 1;

		//Made new function for running reports - JMM 5-28-04
		// (a.walling 2009-07-02 10:34) - PLID 14181 - Need to pass in our printer info!
		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, &paramList, m_pInfo->m_bPreview?true:false, (CWnd *)this, "Schedule", m_pInfo, TRUE);
	}
	else if (m_nPrintExtent == 3) { //resource report

		//JMM - Made the resource view report editable
		//check to see if we want to run the custom report
		//Made new function for running reports - JMM 5-28-04
		// (a.walling 2009-07-02 10:34) - PLID 14181 - Need to pass in our printer info!
		// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
		RunReport(&infReport, &paramList, m_pInfo->m_bPreview?true:false, (CWnd *)this, "Schedule", m_pInfo, TRUE);

	}

	Progressdlg.DestroyWindow();
	EndWaitCursor();

	// Free the contents of the array
	ClearRPIParameterList(&paramList);	//DRT - PLID 18085 - Cleanup after ourselves
}

bool CPrintScheduleDlg::Init()
{
	CString strTemp;
	// Prepare Date info and dialog controls
	//TES 5/27/03: This overload of this function stopped having any effect several years ago, so I took it out.
	//SetCenterDate(m_dtReportCenterDate);

	
	// Hide the "Show Cancelled Appointments" checkbox since it only
	// applies to the classic printout
	GetDlgItem(IDC_CHECK_SHOW_CANCELLED_APPTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_VIEWED_RESOURCES)->ShowWindow(SW_HIDE);

	// Load the print properties
	m_bShowApptShowState = GetRemotePropertyInt("SchedPrintPending", 0, 0, GetCurrentUserName());
	m_bShowMonthProcedures = GetRemotePropertyInt("SchedPrintProcedures", 0, 0, GetCurrentUserName());
	m_bShowTemplateColors = GetRemotePropertyInt("SchedPrintTemplateClr", 0, 0, GetCurrentUserName());
	m_bClassicShowCancelled = GetRemotePropertyInt("ClassicReptShowCancelledAppts", 0, 0, GetCurrentUserName());
	m_bPrintGrid = GetRemotePropertyInt("SchedPrintGrid", 0, 0, GetCurrentUserName());
	m_bViewedResources = GetRemotePropertyInt("SchedViewedResources", 0, 0, GetCurrentUserName());
	// (c.haag 2008-06-18 16:09) - PLID 26135 - Added m_bPrintTemplates
	m_bPrintTemplates = FALSE; // This should never default to on

	// Reflect the properties on screen
	CheckDlgButton(IDC_CHECK_PRINT_PENDING, m_bShowApptShowState);
	CheckDlgButton(IDC_CHECK_PRINT_PROCEDURES, m_bShowMonthProcedures);
	CheckDlgButton(IDC_CHECK_PRINT_TEMPLATE_COLOR, m_bShowTemplateColors);
	CheckDlgButton(IDC_CHECK_SHOW_CANCELLED_APPTS, m_bClassicShowCancelled);
	CheckDlgButton(IDC_PRINT_GRID, m_bPrintGrid);
	CheckDlgButton(IDC_CHECK_VIEWED_RESOURCES, m_bViewedResources);
	// (c.haag 2008-06-18 16:09) - PLID 26135 - Added m_bPrintTemplates
	CheckDlgButton(IDC_CHECK_PRINT_TEMPLATES, m_bPrintTemplates);

///////////////////////////////////////////////////////////////////////////////////////
// Wondering what this is?  See the CheckRadioButton doesn't ensure that the values you 
// give it are legal, so I put the check here manually.  This way if the IDCs ever 
// change what they are defined as and the new definitions are invalid, there will be a 
// compile error.
#if IDC_PORTRAIT_RADIO != IDC_LANDSCAPE_RADIO-1
ERROR______IDC_PORTRAIT_RADIO___and___IDC_LANDSCAPE_RADIO___must___be___consecutive;
#endif
///////////////////////////////////////////////////////////////////////////////////////

	// Load the boolean and select the radio button according to what it was last time
	if (GetRemotePropertyInt("SchedPrintPortrait", 0, 0, GetCurrentUserName())) {
		m_bUseLandscape = FALSE;
		CheckRadioButton(IDC_PORTRAIT_RADIO, IDC_LANDSCAPE_RADIO, IDC_PORTRAIT_RADIO);
	} else {
		m_bUseLandscape = TRUE;
		CheckRadioButton(IDC_PORTRAIT_RADIO, IDC_LANDSCAPE_RADIO, IDC_LANDSCAPE_RADIO);
	}

	
	// Choose to display "Print" or "Preview" on the action button
	if (m_pInfo->m_bPreview) {
		strTemp = "Preview";
	} else {
		strTemp = "Print";
	}
	SetDlgItemText(IDC_PREVIEW_BTN, "&" + strTemp);

	// Display an appropriate dialog title
	if (m_nPrintExtent == 0) {
		SetWindowText(strTemp + " Day Schedule");
	} else if (m_nPrintExtent == 1) {
		SetWindowText(strTemp + " Week Schedule");
	} else if (m_nPrintExtent == 2) {
		SetWindowText(strTemp + " Month Schedule");
	} else if (m_nPrintExtent == 3) {
		//ToDo:  Make this work  (MultiResource report)
		SetWindowText(strTemp);
	}

	//PLID 20056: make it pull from what they chose last
	//do this last so nothing is re-updated afterwards
	long nType = GetRemotePropertyInt("SchedulerPrintType", pmGraphical, 0, GetCurrentUserName(), true);
	if (nType == pmGraphical) {
		((CButton *)GetDlgItem(IDC_GRAPHICAL_RADIO))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_TEXTUAL_RADIO))->SetCheck(0);
		OnGraphicalRadio();
	}
	else if (nType == pmReport) {
		((CButton *)GetDlgItem(IDC_GRAPHICAL_RADIO))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_TEXTUAL_RADIO))->SetCheck(1);
		OnTextualRadio();
	}
	else {
		//thats strange, just set it to what it defaults to now
		((CButton *)GetDlgItem(IDC_GRAPHICAL_RADIO))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_TEXTUAL_RADIO))->SetCheck(0);
		OnGraphicalRadio();
	}
	

	return true;
}

CPrintScheduleDlg::pmPrintMode CPrintScheduleDlg::GetPrintMode()
{
	return m_pmPrintMode;
}

/*
void CPrintScheduleDlg::OnSelchangePrintChoiceCombo() 
{
	CString strTemp;
	GetDlgItemText(IDC_PRINT_CHOICE_COMBO, strTemp);
	if (strTemp.Left(1) == "G") {
		m_pmPrintMode = CPrintScheduleDlg::pmGraphical;
	} else {
		m_pmPrintMode = CPrintScheduleDlg::pmReport;
	}
}
*/

long CPrintScheduleDlg::ZoomPrint(CPrintInfo* pInfo)
{
	m_pInfo = pInfo;
	return DoModal();
}

void CPrintScheduleDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();
	
}

void CPrintScheduleDlg::OnGraphicalRadio() 
{
	m_pmPrintMode = CPrintScheduleDlg::pmGraphical;

	// Hide all controls that relate to textual only
	GetDlgItem(IDC_CHECK_SHOW_CANCELLED_APPTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_VIEWED_RESOURCES)->ShowWindow(SW_HIDE);

	// If the month view is visible, hide the non-month view-related checkboxes
	GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->ShowWindow(SW_SHOW);
	if (GetMainFrame()->GetActiveView() && GetMainFrame()->GetActiveView()->GetActiveTab() == 3) { 
		GetDlgItem(IDC_CHECK_PRINT_PENDING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_PRINT_TEMPLATE_COLOR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PRINT_GRID)->ShowWindow(SW_HIDE);
		// (c.haag 2008-06-18 16:06) - PLID 26135 - Show the Print Templates box
		GetDlgItem(IDC_CHECK_PRINT_TEMPLATES)->ShowWindow(SW_SHOW);
		// Disable Print Procedures if we're printing templates
		if (m_bPrintTemplates) {
			GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->EnableWindow(TRUE);
		}
	}
	// If not, disable the first checkbox
	else {
		((CButton *)GetDlgItem(IDC_CHECK_PRINT_PROCEDURES))->SetCheck(0); // Having a checked grayed out box can scare the user
		GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_PRINT_PENDING)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHECK_PRINT_TEMPLATE_COLOR)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PRINT_GRID)->ShowWindow(SW_SHOW);
		// (c.haag 2008-06-18 16:06) - PLID 26135 - Hide the Print Templates box
		GetDlgItem(IDC_CHECK_PRINT_TEMPLATES)->ShowWindow(SW_HIDE);
	}

	//
	// Show the orientation stuff
	GetDlgItem(IDC_ORIENTATION_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_PORTRAIT_RADIO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_LANDSCAPE_RADIO)->ShowWindow(SW_SHOW);
}

void CPrintScheduleDlg::OnTextualRadio() 
{
	m_pmPrintMode = CPrintScheduleDlg::pmReport;

	// Hide all checkboxes that relate to graphical only
	GetDlgItem(IDC_CHECK_PRINT_PENDING)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_PRINT_TEMPLATE_COLOR)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PRINT_GRID)->ShowWindow(SW_HIDE);
	// (c.haag 2008-06-18 16:06) - PLID 26135 - Print templates box
	GetDlgItem(IDC_CHECK_PRINT_TEMPLATES)->ShowWindow(SW_HIDE);
	
	// Hide the orientation stuff
	GetDlgItem(IDC_ORIENTATION_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PORTRAIT_RADIO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LANDSCAPE_RADIO)->ShowWindow(SW_HIDE);

	// Show all checkboxes that related to textual only
	GetDlgItem(IDC_CHECK_SHOW_CANCELLED_APPTS)->ShowWindow(SW_SHOW);

	//this only shows on the resource view
	// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
	if(GetMainFrame()->GetActiveView() && GetMainFrame()->GetActiveView()->GetActiveTab() == 2) {
		GetDlgItem(IDC_CHECK_VIEWED_RESOURCES)->ShowWindow(SW_SHOW);
	}
}

void CPrintScheduleDlg::OnPrintTemplateClr()
{
	m_bShowTemplateColors = IsDlgButtonChecked(IDC_CHECK_PRINT_TEMPLATE_COLOR);
	SetRemotePropertyInt("SchedPrintTemplateClr", m_bShowTemplateColors, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnPrintPending()
{
	m_bShowApptShowState = IsDlgButtonChecked(IDC_CHECK_PRINT_PENDING);
	SetRemotePropertyInt("SchedPrintPending", m_bShowApptShowState, 0, GetCurrentUserName());	
}

void CPrintScheduleDlg::OnPrintProcedures()
{
	m_bShowMonthProcedures = IsDlgButtonChecked(IDC_CHECK_PRINT_PROCEDURES);
	SetRemotePropertyInt("SchedPrintProcedures", m_bShowMonthProcedures, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnShowCancelledAppts()
{
	m_bClassicShowCancelled = IsDlgButtonChecked(IDC_CHECK_SHOW_CANCELLED_APPTS);
	SetRemotePropertyInt("ClassicReptShowCancelledAppts", m_bClassicShowCancelled, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnLandscapeRadio() 
{
	m_bUseLandscape = TRUE;
	SetRemotePropertyInt("SchedPrintPortrait", 0, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnPortraitRadio() 
{
	m_bUseLandscape = FALSE;
	SetRemotePropertyInt("SchedPrintPortrait", 1, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnPrintGrid() 
{
	m_bPrintGrid = IsDlgButtonChecked(IDC_PRINT_GRID);
	SetRemotePropertyInt("SchedPrintGrid", m_bPrintGrid, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnCheckViewedResources() 
{
	m_bViewedResources = IsDlgButtonChecked(IDC_CHECK_VIEWED_RESOURCES);
	SetRemotePropertyInt("SchedViewedResources", m_bViewedResources, 0, GetCurrentUserName());
}

void CPrintScheduleDlg::OnCheckPrintTemplates() 
{
	// (c.haag 2008-06-18 16:12) - PLID 26135 - Toggle the Print Templates property
	try {
		// Disable Print Procedures if we're printing templates
		if ((m_bPrintTemplates = IsDlgButtonChecked(IDC_CHECK_PRINT_TEMPLATES))) {
			GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_CHECK_PRINT_PROCEDURES)->EnableWindow(TRUE);
		}
		SetRemotePropertyInt("SchedPrintTemplates", m_bPrintTemplates, 0, GetCurrentUserName());
	}
	NxCatchAll("Error in CPrintScheduleDlg::OnCheckPrintTemplates");
}
