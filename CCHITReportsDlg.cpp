// CCHITReportsDlg.cpp : implementation file
// (d.thompson 2010-01-18) - PLID 36927 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "CCHITReportsDlg.h"
#include "CCHITReportsConfigOptionsDlg.h"
#include "EmrUtils.h"
#include "MultiSelectDlg.h"
#include "globalreportutils.h"
#include "reports.h"
#include "Reportinfo.h"
#include "CCHITReportInfoListing.h" //(e.lally 2012-02-24) PLID 48266
#include "CCHITReportsLoadData.h" //(e.lally 2012-02-24) PLID 48266
#include "GlobalUtils.h"

using namespace NXDATALIST2Lib;

enum eColumns {
	eInternalName = 0, // (j.gruber 2011-11-04 12:45) - PLID 45692
	eName,
	ePercent,
	eNum,
	eDenom,
	eHelp,
	eHelpGeneral,
	eHelpNum,
	eHelpDenom,
	eConfigure,
	eConfigureType, // (j.gruber 2010-09-10 14:30) - PLID 40487 - added type
	eReportSelect, // (j.gruber 2011-11-08 11:08) - PLID 45689
	eReportNumFrom, // (j.gruber 2011-11-08 11:08) - PLID 45689
	eReportDenomFrom, // (j.gruber 2011-11-08 11:08) - PLID 45689
};

// (j.gruber 2011-05-16 16:47) - PLID 43676 - added provider list
enum ProviderListColumns {
	plcID = 0,
	plcArchived,
	plcName,
};

// (j.gruber 2011-05-18 10:51) - PLID 43758
enum LocationListColumns {
	llcID = 0,
	llcName,
};


// CCCHITReportsDlg dialog
IMPLEMENT_DYNAMIC(CCCHITReportsDlg, CNxDialog)

BEGIN_EVENTSINK_MAP(CCCHITReportsDlg, CNxDialog)
	ON_EVENT(CCCHITReportsDlg, IDC_REPORTS_LIST, 19, CCCHITReportsDlg::LeftClickReportsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCCHITReportsDlg, IDC_CCHIT_PROV_LIST, 16, CCCHITReportsDlg::SelChosenCchitProvList, VTS_DISPATCH)
	ON_EVENT(CCCHITReportsDlg, IDC_CCHIT_PROV_LIST, 18, CCCHITReportsDlg::RequeryFinishedProviderList, VTS_I2)
	ON_EVENT(CCCHITReportsDlg, IDC_CCHIT_REPORTS_LIST, 19, CCCHITReportsDlg::LeftClickCCHITReportsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCCHITReportsDlg, IDC_CCHIT_LOCATION_LIST, 16, CCCHITReportsDlg::SelChosenCchitLocationList, VTS_DISPATCH)
	ON_EVENT(CCCHITReportsDlg, IDC_CCHIT_LOCATION_LIST, 18, CCCHITReportsDlg::RequeryFinishedLocationList, VTS_I2)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CCCHITReportsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CCHIT_LOAD, &CCCHITReportsDlg::OnBnClickedCchitLoad)
	ON_BN_CLICKED(IDC_CCHIT_CLOSE, &CCCHITReportsDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_CCHIT_SHOW_CONFIG, &CCCHITReportsDlg::OnBnClickedCchitShowConfig)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	ON_MESSAGE(NXM_CCHIT_REPORTS_ADD_ROW, OnProcessingAddRow)
	ON_MESSAGE(NXM_CCHIT_REPORTS_PROCESSING_FINISHED, OnProcessingFinished)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MU_PREVIEW, &CCCHITReportsDlg::OnBnClickedMUPreview)
	ON_BN_CLICKED(IDC_CCHIT_EXCLUSIONS, &CCCHITReportsDlg::OnBnClickedCCHITExclusions)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_OPEN_PQRI, &CCCHITReportsDlg::OnOpenPqri)
	ON_BN_CLICKED(IDC_RADIO_STAGE1, &CCCHITReportsDlg::OnBnClickedRadioStage1)
	ON_BN_CLICKED(IDC_RADIO_STAGE2, &CCCHITReportsDlg::OnBnClickedRadioStage2)
	ON_BN_CLICKED(IDC_RADIO_MOD_STAGE2, &CCCHITReportsDlg::OnBnClickedRadioModStage2)
END_MESSAGE_MAP()

CCCHITReportsDlg::CCCHITReportsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCCHITReportsDlg::IDD, pParent)
{
	// (j.gruber 2011-11-02 11:37) - PLID 46222
	m_pThread = NULL;
	m_hStopThread = NULL;
	m_bDialogClosing = FALSE;
	// (j.gruber 2011-11-03 16:47) - PLID 44993
	m_bDoneLoading = false;	

}

CCCHITReportsDlg::~CCCHITReportsDlg()
{
}

void CCCHITReportsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CCHIT_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_CCHIT_FROM, m_from);
	DDX_Control(pDX, IDC_CCHIT_TO, m_to);
	DDX_Control(pDX, IDC_CCHIT_EXCLUSIONS, m_btnExclusions);
	DDX_Control(pDX, IDC_CCHIT_SHOW_CONFIG, m_btnConfigOptions);
	DDX_Control(pDX, IDC_CCHIT_MULTI_PROV_LIST, m_nxlProviderLabel);	
	DDX_Control(pDX, IDC_CCHIT_MULTI_LOC_LIST, m_nxlLocationLabel);	
	DDX_Control(pDX, IDC_OPEN_PQRI, m_nxbOpenPqri);
}

// CCCHITReportsDlg message handlers
BOOL CCCHITReportsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();	

		// (j.gruber 2011-05-13 17:06) - PLID 43695
		g_propManager.CachePropertiesInBulk("CCHITSettings", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("			
			" Name = 'CCHITReportsMeaningfulUseOnly' "						
			")",
			_Q(GetCurrentUserName()));

		// (j.gruber 2011-08-02 09:37) - PLID 44755
		// (d.thompson 2012-10-18) - PLID 48423 - Renamed mu13
		g_propManager.CachePropertiesInBulk("CCHITSettings2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("			
			" Name = 'CCHIT_MU.13_CODES_v3' "						
			")",
			_Q(GetCurrentUserName()));


		// (j.gruber 2011-11-07 11:56) - PLID 45365
		g_propManager.CachePropertiesInBulk("CCHITSettings3", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			" Name = 'MU_ExcludedTemplates' "
			")",
			_Q(GetCurrentUserName()));
		

		m_pList = BindNxDataList2Ctrl(IDC_REPORTS_LIST, false);
		// (j.gruber 2011-05-13 11:55) - PLID 43694
		m_pCCHITList = BindNxDataList2Ctrl(IDC_CCHIT_REPORTS_LIST, false);
		m_btnClose.AutoSet(NXB_CLOSE);
		// (j.gruber 2011-05-13 11:55) - PLID 43676
		m_pProviderList = BindNxDataList2Ctrl(IDC_CCHIT_PROV_LIST);

		// (j.gruber 2011-05-18 10:38) - PLID 43758
		m_pLocationList = BindNxDataList2Ctrl(IDC_CCHIT_LOCATION_LIST);

		// (j.gruber 2011-05-13 11:55) - PLID 43676
		m_nxlProviderLabel.SetColor(0x00C8FFFF);
		m_nxlProviderLabel.SetText("");
		m_nxlProviderLabel.SetType(dtsHyperlink);


		// (j.gruber 2011-05-18 10:38) - PLID 43758
		m_nxlLocationLabel.SetColor(0x00C8FFFF);
		m_nxlLocationLabel.SetText("");
		m_nxlLocationLabel.SetType(dtsHyperlink);

		// (j.gruber 2011-05-13 13:25) - PLID 43695 - move the controls appropriately
		PositionControls();

		//Show hide columns as necessary
		EnsureColumns();

		// (j.gruber 2011-11-07 11:57) - PLID 45365 - Load the Exclusions
		LoadExclusions();

		//Default the date range to the past year
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		m_from.SetValue(dtNow + COleDateTimeSpan(-365, 0, 0, 0));
		m_to.SetValue(dtNow);

		// (j.gruber 2011-11-02 11:08) - PLID 46222
		SetDlgItemText(IDC_STATIC_MEASURE_PROGRESS, "");
		m_nCurrentProviderID = -1; //r.wilson PLID 45510 -> Fixes a small bug that selects a blank row if cancel was hit on the mulit-seledt dialog on the first time.	
	    m_nCurrentLocationID = -1; //r.wilson PLID 45510 -> Fixes a small bug that selects a blank row if cancel was hit on the mulit-seledt dialog on the first time.	

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.gruber 2011-05-18 10:46) - PLID 43758 - consolidated into one function
// (j.gruber 2011-09-27 10:01) - PLID 45616 - added checkbox for secondaries
void CCCHITReportsDlg::PositionLists(long nListID, long nLabelID, long nMultiSelectBoxID, long nPlaceHolderID, long nCheckBoxID /*= -1*/) 
{

	//move the list up to the top
	CRect rctLabel, rctList, rctMultiBox, rctPlaceHolder;
	GetDlgItem(nLabelID)->GetWindowRect(rctLabel);
	ScreenToClient(rctLabel);
	
	GetDlgItem(nListID)->GetWindowRect(rctList);
	ScreenToClient(rctList);

	GetDlgItem(nMultiSelectBoxID)->GetWindowRect(rctMultiBox);
	ScreenToClient(rctMultiBox);

	GetDlgItem(nPlaceHolderID)->GetWindowRect(rctPlaceHolder);
	ScreenToClient(rctPlaceHolder);

	int nDiff = rctList.top - rctLabel.top;
	int nHeight = rctLabel.Height();
	rctList.top = rctPlaceHolder.top;
	rctList.bottom = rctPlaceHolder.bottom;
	GetDlgItem(nListID)->MoveWindow(rctList);

	rctMultiBox.top = rctPlaceHolder.top;
	rctMultiBox.bottom = rctPlaceHolder.bottom;
	GetDlgItem(nMultiSelectBoxID)->MoveWindow(rctMultiBox);

	rctLabel.top = rctPlaceHolder.top - nDiff;
	rctLabel.bottom = rctLabel.top + nHeight;
	GetDlgItem(nLabelID)->MoveWindow(rctLabel);

	// (j.gruber 2011-09-27 10:04) - PLID 45616 - checkbox
	if (nCheckBoxID != -1) {
		CRect rctCheck;
		GetDlgItem(nCheckBoxID)->GetWindowRect(rctCheck);
		ScreenToClient(rctCheck);
		//put it at the same y as the label
		rctCheck.top = rctPlaceHolder.top - nDiff;
		rctCheck.bottom = rctCheck.top + nHeight;
		GetDlgItem(nCheckBoxID)->MoveWindow(rctCheck);
	}

}

		// (j.gruber 2011-05-13 11:55) - PLID 43695
void CCCHITReportsDlg::PositionControls()
{
	if (GetRemotePropertyInt("CCHITReportsMeaningfulUseOnly", 1, 0, "<None>")) {

		
		// (j.gruber 2011-05-18 10:47) - PLID 43758 - call the new function
		// (j.gruber 2011-09-27 10:04) - PLID 45616 - added checkbox
		PositionLists(IDC_CCHIT_PROVIDER_LABEL, IDC_CCHIT_PROV_LIST, IDC_CCHIT_MULTI_PROV_LIST, IDC_CCHIT_PROV_PLACE_HOLDER, IDC_CCHIT_EXCLUDE_SECONDARIES);
		PositionLists(IDC_CCHIT_LOCATION_LABEL, IDC_CCHIT_LOCATION_LIST, IDC_CCHIT_MULTI_LOC_LIST, IDC_CCHIT_LOCATION_PLACE_HOLDER);

		//make the MU reports take up the entire dialog
		CRect rctMUReportList, rctCCHITReportList;
		GetDlgItem(IDC_REPORTS_LIST)->GetWindowRect(rctMUReportList);
		ScreenToClient(rctMUReportList);

		GetDlgItem(IDC_CCHIT_REPORTS_LIST)->GetWindowRect(rctCCHITReportList);
		ScreenToClient(rctCCHITReportList);

		rctMUReportList.top = rctCCHITReportList.top;
		GetDlgItem(IDC_REPORTS_LIST)->MoveWindow(rctMUReportList);

		// Preselect Mod. Stage 2 OnInit
		((CButton*)GetDlgItem(IDC_RADIO_STAGE1))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_STAGE2))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_MOD_STAGE2))->SetCheck(BST_CHECKED);
		m_eMeaningfulUseStage = MU::ModStage2;

		GetDlgItem(IDC_CCHIT_REPORTS_LIST)->ShowWindow(SW_HIDE);		
	}
	else {
		//move the top list up slightly to get rid of provider the space
		CRect rctProvPlaceHolder, rctCCHITReportList;
		GetDlgItem(IDC_CCHIT_PROV_PLACE_HOLDER)->GetWindowRect(rctProvPlaceHolder);
		ScreenToClient(rctProvPlaceHolder);

		GetDlgItem(IDC_CCHIT_REPORTS_LIST)->GetWindowRect(rctCCHITReportList);
		ScreenToClient(rctCCHITReportList);

		rctCCHITReportList.top = rctProvPlaceHolder.top;
		GetDlgItem(IDC_CCHIT_REPORTS_LIST)->MoveWindow(rctCCHITReportList);
	}

}

void CCCHITReportsDlg::LoadAllReportData()
{
	//(e.lally 2012-02-24) PLID 48266 - Moved initialization of report array to CCHITReportInfoListing
	CCHITReportInfoListing cchitReportListing(this);
	// (r.farnworth 2013-10-14 17:27) - PLID 58995 - Clear the array so that it starts fresh when switching stages
	m_aryReports.RemoveAll();
	cchitReportListing.LoadReportListing(m_eMeaningfulUseStage);
	for(int i = 0; i<cchitReportListing.m_aryReports.GetSize(); i++){
		m_aryReports.Add(cchitReportListing.m_aryReports.GetAt(i));
	}
}


void CCCHITReportsDlg::LoadReportsToInterface()
{
	CWaitCursor pWait;

	//Get the date range filters
	COleDateTime dtFrom = m_from.GetValue();
	COleDateTime dtTo = m_to.GetValue();

	if(dtTo < dtFrom) {
		//Will yield no results
		AfxMessageBox("Your from date is after your to date, no results can be calculated.  Please correct your date filter and try again.");
		return;
	}

	// (j.gruber 2011-11-07 16:39) - PLID 45365 - check to see how many templates they are excluding
	if (m_dwExcludedTemplateIDs.GetCount() > 10) {
		if (IDNO == MsgBox(MB_YESNO, "You are excluding more than 10 templates from each measure.  The only templates that should be excluded are those where the patient is not seen or consulted with that day, ie: NexWeb templates or phone call message templates.\n\n"
			"Are you sure you wish to continue with so many exclusions?")) {
				return;
		}
	}

	//load our map
	//ClearMessageMap();
	long nRecords = -1;
	BOOL bCCHITHidden = GetRemotePropertyInt("CCHITReportsMeaningfulUseOnly", 1, 0, "<None>");

	// (j.gruber 2011-11-03 16:47) - PLID 44993
	m_bDoneLoading = false;

	// (j.gruber 2011-11-04 16:16) - PLID 46222
	//grey out the button 
	GetDlgItem(IDC_CCHIT_LOAD)->EnableWindow(FALSE);

	// (r.farnworth 2013-12-03 09:29) - PLID 58995 - DISable the Stage buttons
	GetDlgItem(IDC_RADIO_STAGE1)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_STAGE2)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_MOD_STAGE2)->EnableWindow(FALSE);

	//run through the array and get our total
	m_nTotalMeasures = 0;
	m_nCurrentMeasureCount = 0;
	for(int i = 0; i < m_aryReports.GetSize(); i++) {
		
		if (((bCCHITHidden) && m_aryReports.GetAt(i).GetReportType() == crtMU) ||
			(!bCCHITHidden) 
		) {			
			m_nTotalMeasures++;			
		}
	}			
	GetDlgItem(IDC_PROGRESS_CCHIT_MEASURES)->PostMessage(PBM_SETRANGE, 0, MAKELPARAM(0, m_nTotalMeasures-1));		

	// (j.gruber 2011-11-04 10:53) - PLID 44993 - save our variables
	m_dtLastDateToRun = dtTo;
	m_dtLastDateFromRun = dtFrom;
	// (j.gruber 2011-11-09 11:01) - PLID 45689 - changed to be the IDs here
	m_strLastProvsRun = m_strProviderList;
	m_strLastLocsRun = m_strLocationList;

	// (j.gruber 2011-11-09 10:50) - PLID 45689 - need them all
	if (m_strExclusionsNameList.IsEmpty() && !m_strExclusionsList.IsEmpty()) {
		ASSERT(FALSE);
		m_strLastRunExclusionsNameList  = GetTemplateNamesFromIDString(m_strExclusionsList);
	}
	else {
		m_strLastRunExclusionsNameList = m_strExclusionsNameList;
	}
	m_strLastRunExclusionsList = m_strExclusionsList;
	m_bLastRunExcludeSecondaries = IsDlgButtonChecked(IDC_CCHIT_EXCLUDE_SECONDARIES) ? TRUE : FALSE;

	// (j.gruber 2011-11-01 13:16) - PLID 46222 - call our thread
	m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	CCCHITReportsLoadData* pData = new CCCHITReportsLoadData();		
	pData->hwndParent = GetSafeHwnd();	
	pData->bCCHITHidden = bCCHITHidden;
	pData->bExcludeSecondaries = IsDlgButtonChecked(IDC_CCHIT_EXCLUDE_SECONDARIES) ? TRUE : FALSE;
	pData->dtFrom = dtFrom;
	pData->dtTo = dtTo;
	pData->strLocationList = m_strLocationList;
	pData->strProviderList = m_strProviderList;
	// (j.gruber 2011-11-07 12:22) - PLID 45365 - exclusiong list
	pData->strExclusionTemplateList = m_strExclusionsList;
	pData->paryReports = &m_aryReports;
	pData->hStopThread = m_hStopThread;

	//(e.lally 2012-02-24) PLID 48266 - Made the thread creation a method of the load data
	m_pThread = pData->BeginCalculationThread();
}

void CCCHITReportsDlg::OnOK()
{
	//do nothing
}

// (j.gruber 2011-11-03 15:01) - PLID 46219
void CCCHITReportsDlg::CloseWindow()
{
	m_bDialogClosing = TRUE;

	// (j.gruber 2011-11-02 09:56) - PLID 46222 - kill the thread
	KillThread();
	// (j.gruber 2011-11-01 15:17) - PLID 46219 - just clear the lists and hide the window	
	m_pList->Clear();
	m_pCCHITList->Clear();

	//set the progress to none		
	SetDlgItemText(IDC_STATIC_MEASURE_PROGRESS, "");
	GetDlgItem(IDC_PROGRESS_CCHIT_MEASURES)->PostMessage(PBM_SETPOS, 0);		

	//reset our values
	m_nTotalMeasures = 0;
	m_nCurrentMeasureCount = 0;

	// (j.gruber 2011-11-04 11:27) - PLID 44993
	m_bDoneLoading = false;

	ShowWindow(SW_HIDE);
}

void CCCHITReportsDlg::OnBnClickedClose()
{
	CloseWindow();
}


void CCCHITReportsDlg::LeftClickReportsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		switch(nCol) {
			case eHelp:
				{
					//The help data is stored in the list, so pull it out, format it, and display it to the user
					IRowSettingsPtr pRow(lpRow);
					CString strGeneral = VarString(pRow->GetValue(eHelpGeneral));
					CString strNum = VarString(pRow->GetValue(eHelpNum));
					CString strDenom = VarString(pRow->GetValue(eHelpDenom));

					//Simply display it to the user
					AfxMessageBox(strGeneral + "\r\n\r\nNumerator:\t" + strNum + "\r\n\r\nDenominator:\t" + strDenom, MB_ICONINFORMATION);
				}
				break;

			case eConfigure:
				{
					IRowSettingsPtr pRow(lpRow);
					//Launch the config dialog, if it's configurable
					if(VarString(pRow->GetValue(eConfigure)) != "") {
						// (j.gruber 2011-11-04 12:45) - PLID 45692 - change to internal name
						CCCHITReportsConfigOptionsDlg dlg(this, VarString(pRow->GetValue(eInternalName)), (CCHITReportConfigType)VarLong(pRow->GetValue(eConfigureType)));
						if(dlg.DoModal() == IDOK) {
							//Reload reports to handle changes
							// (j.gruber 2011-05-13 17:57) - PLID 43704 - don't do this, just let them click the load button
							//OnBnClickedCchitLoad();
						}
					}
				}
			break;

			// (j.gruber 2011-11-08 11:39) - PLID 45689
			case eNum:
			case eDenom:
			case ePercent:
				{
					IRowSettingsPtr pRow(lpRow);
					if (pRow) {
						PreviewReport(pRow, nCol);
					}
				}				
			break;
			
			
		}
	} NxCatchAll(__FUNCTION__);
}

		// (j.gruber 2011-05-13 11:55) - PLID 43694
void CCCHITReportsDlg::LeftClickCCHITReportsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		switch(nCol) {
			case eHelp:
				{
					//The help data is stored in the list, so pull it out, format it, and display it to the user
					IRowSettingsPtr pRow(lpRow);
					CString strGeneral = VarString(pRow->GetValue(eHelpGeneral));
					CString strNum = VarString(pRow->GetValue(eHelpNum));
					CString strDenom = VarString(pRow->GetValue(eHelpDenom));

					//Simply display it to the user
					AfxMessageBox(strGeneral + "\r\n\r\nNumerator:\t" + strNum + "\r\n\r\nDenominator:\t" + strDenom, MB_ICONINFORMATION);
				}
				break;

			case eConfigure:
				{
					IRowSettingsPtr pRow(lpRow);
					//Launch the config dialog, if it's configurable
					if(VarString(pRow->GetValue(eConfigure)) != "") {
						// (j.gruber 2011-11-04 12:46) - PLID 45692 - change to internal name
						CCCHITReportsConfigOptionsDlg dlg(this, VarString(pRow->GetValue(eInternalName)), (CCHITReportConfigType)VarLong(pRow->GetValue(eConfigureType)));
						if(dlg.DoModal() == IDOK) {
							//Reload reports to handle changes
							// (j.gruber 2011-05-13 17:57) - PLID 43704 - don't do this, just let them click the load button
							//OnBnClickedCchitLoad();
						}
					}
				}
				break;
		}
	} NxCatchAll(__FUNCTION__);
}


void CCCHITReportsDlg::OnBnClickedCchitLoad()
{
	try {
		//Clear the data that exists
		m_pList->Clear();
		// (j.gruber 2011-05-13 11:55) - PLID 43694
		m_pCCHITList->Clear();

		// (r.farnworth 2013-10-14 16:22) - PLID 58995 - Moved here to support 2 stages
		//Please put all code to load new reports in this function
		LoadAllReportData();

		//Now load the reports into the interface
		LoadReportsToInterface();

	} NxCatchAll(__FUNCTION__);
}






void CCCHITReportsDlg::OnBnClickedCchitShowConfig()
{
	try {
		EnsureColumns();

	} NxCatchAll(__FUNCTION__);
}

void CCCHITReportsDlg::EnsureColumns()
{
	IColumnSettingsPtr pCol = m_pList->GetColumn(eConfigure);
	if(IsDlgButtonChecked(IDC_CCHIT_SHOW_CONFIG)) {
		pCol->PutColumnStyle(csVisible|csWidthData);
	}
	else {
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
	}

	// (j.gruber 2011-05-13 12:06) - PLID 43694 - added another list
	pCol = m_pCCHITList->GetColumn(eConfigure);
	if(IsDlgButtonChecked(IDC_CCHIT_SHOW_CONFIG)) {
		pCol->PutColumnStyle(csVisible|csWidthData);
	}
	else {
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}



// (j.gruber 2011-05-16 12:26) - PLID 43676
void CCCHITReportsDlg::SelChosenCchitProvList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(plcID));
			if (nID == -1) {
				//clear the list
				m_strProviderList = "";
				m_nCurrentProviderID = -1;
				m_dwProvIDList.RemoveAll();
			}
			else if (nID == -2) {
				//multiple selections
				if (!SelectMultiProviders()) {
					//nothing changed
					return;
				}
			}		
			else {
				//just one
				m_strProviderList.Format("(%li)", nID);
				m_nCurrentProviderID = nID;
				m_dwProvIDList.RemoveAll();
				m_dwProvIDList.Add(nID);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-16 12:26) - PLID 43676
BOOL CCCHITReportsDlg::SelectMultiProviders() {

	CString strFrom, strWhere;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProvidersT");
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	
	// Fill the dialog with existing selections	
	dlg.PreSelect(m_dwProvIDList);
	
	dlg.m_strNameColTitle = "Provider";
	
	strFrom = AsString(m_pProviderList->GetFromClause());
	strWhere = AsString(m_pProviderList->GetWhereClause());

	//r.wilson 10/19/2011 PLID 45510 
	//r.wilson (response to returned item) Uncommenting the line below will make it so only active providers will be displayed on the multi-select dialog
    //strWhere = " Archived = 0 ";
	hRes = dlg.Open(strFrom, strWhere, "ProvidersT.PersonID", "PersonT.Last + ',' + PersonT.First + ' ' + PersonT.Middle", "Please select the providers to filter on.", 1);

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of providers with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwProvIDList);
		m_strProviderList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwProvIDList.GetSize() > 1) {
			ShowDlgItem(IDC_CCHIT_PROV_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlProviderLabel.SetText(strNames);
			m_nxlProviderLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_CCHIT_MULTI_PROV_LIST, SW_SHOW);			
			m_nxlProviderLabel.Invalidate();
			m_nCurrentProviderID = -2;
		}
		else if(m_dwProvIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_CCHIT_MULTI_PROV_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_PROV_LIST, SW_SHOW);
			m_pProviderList->SetSelByColumn(plcID, (long)m_dwProvIDList.GetAt(0));
			m_nCurrentProviderID = (long)m_dwProvIDList.GetAt(0);
		}
		else {			
			//they can't get here
			m_strProviderList = "";
			//m_nCurrentPurposeID = -3;
			ShowDlgItem(IDC_CCHIT_MULTI_PROV_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_PROV_LIST, SW_SHOW);
			m_pProviderList->SetSelByColumn(plcID, m_nCurrentProviderID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwProvIDList.GetSize() > 1) {
			ShowDlgItem(IDC_CCHIT_PROV_LIST, SW_HIDE);
			m_nxlProviderLabel.SetText(GetProviderNamesFromIDString(m_strProviderList));
			m_nxlProviderLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_CCHIT_MULTI_PROV_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_CCHIT_MULTI_PROV_LIST);
		}
		else {
			//They selected exactly one. 
			ShowDlgItem(IDC_CCHIT_MULTI_PROV_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_PROV_LIST, SW_SHOW);
			m_pProviderList->SetSelByColumn(0, m_nCurrentProviderID);
		}
	}

	return bReturn;
	
}

	// (j.gruber 2011-05-16 12:26) - PLID 43676
CString CCCHITReportsDlg::GetProviderNamesFromIDString(CString strIDs)
{
	// we have the information in the datalist, let's just get it from there instead of querying the data
	//it should be faster that way
	CString strReturn = "";

	//get the parentheses off
	strIDs.TrimRight(')');
	strIDs.TrimLeft('(');

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {

		//there is only one ID, so find item name
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderList->FindByColumn(plcID, (long)atoi(strIDs), 0, FALSE);
		if (pRow) {
			return VarString(pRow->GetValue(plcName));
		}
		else {
			//hmm
			//ASSERT(FALSE);
			return "<All Providers>";
		}
	}
	else {

		//make a map out of our procedure IDs
		CMap <long, long, long, long> mapIDs;
		
	
		while(nResult != -1) {

			long nID = atoi(strIDs.Left(nResult));
			mapIDs.SetAt(nID, nID);

			//take off this string
			strIDs = strIDs.Right(strIDs.GetLength() - (nResult + 1));

			//trim the string
			strIDs.TrimRight();
			strIDs.TrimLeft();

			//now search again
			nResult = strIDs.Find(",");
		}

		strIDs.TrimRight();
		strIDs.TrimLeft();
		//now add the last one
		mapIDs.SetAt(atoi(strIDs), atoi(strIDs));


		//alrighty, now that we have our map, loop through the datalist and look to see its its an ID we want
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderList->GetFirstRow();
		while (pRow) {						
			long nID = VarLong(pRow->GetValue(plcID));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(plcName)) + ", ";
			}
			pRow = pRow->GetNextRow();
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;

}

	// (j.gruber 2011-05-16 12:26) - PLID 43676
BOOL CCCHITReportsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_dwProvIDList.GetSize() > 1) 
		{
			CRect rc;
			GetDlgItem(IDC_CCHIT_MULTI_PROV_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.gruber 2011-05-18 10:51) - PLID 43758
		if (m_dwLocIDList.GetSize() > 1) 
		{
			CRect rc;
			GetDlgItem(IDC_CCHIT_MULTI_LOC_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__)
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}


	// (j.gruber 2011-05-16 12:26) - PLID 43676
LRESULT CCCHITReportsDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_CCHIT_MULTI_PROV_LIST:
			if (SelectMultiProviders()) {
			}
			break;
		// (j.gruber 2011-05-18 10:51) - PLID 43758
		case IDC_CCHIT_MULTI_LOC_LIST:
			SelectMultiLocations();
		break;
		
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

	// (j.gruber 2011-05-16 12:26) - PLID 43676
void CCCHITReportsDlg::RequeryFinishedProviderList(short nFlags)
{
	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pProviderList->GetNewRow();
		pRow->PutValue(plcID, (long) -2);
		pRow->PutValue(plcName, _variant_t("<Multiple Providers>"));
		m_pProviderList->AddRowBefore(pRow, m_pProviderList->GetFirstRow());

		pRow = m_pProviderList->GetNewRow();
		pRow->PutValue(plcID, (long) -1);
		pRow->PutValue(plcName, _variant_t("<All Providers>"));
		m_pProviderList->AddRowBefore(pRow, m_pProviderList->GetFirstRow());

		m_pProviderList->CurSel = pRow;		

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-18 10:29) - PLID 43758
void CCCHITReportsDlg::SelChosenCchitLocationList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(llcID));
			if (nID == -1) {
				//clear the list
				m_strLocationList = "";
				m_nCurrentLocationID = -1;
				m_dwLocIDList.RemoveAll();
			}
			else if (nID == -2) {
				//multiple selections
				if (!SelectMultiLocations()) {
					//nothing changed
					return;
				}
			}		
			else {
				//just one
				m_strLocationList.Format("(%li)", nID);
				m_nCurrentLocationID = nID;
				m_dwLocIDList.RemoveAll();
				m_dwLocIDList.Add(nID);
				
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-16 12:26) - PLID 43676
BOOL CCCHITReportsDlg::SelectMultiLocations() {

	CString strFrom, strWhere;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "LocationsT");
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	
	// Fill the dialog with existing selections	
	dlg.PreSelect(m_dwLocIDList);
	
	dlg.m_strNameColTitle = "Location";
	
	strFrom = AsString(m_pLocationList->GetFromClause());
	strWhere = AsString(m_pLocationList->GetWhereClause());

	hRes = dlg.Open(strFrom, strWhere, "LocationsT.ID", "LocationsT.Name", "Please select the locations to filter on.", 1);

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of providers with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwLocIDList);
		m_strLocationList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_CCHIT_LOCATION_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlLocationLabel.SetText(strNames);
			m_nxlLocationLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_CCHIT_MULTI_LOC_LIST, SW_SHOW);			
			m_nxlLocationLabel.Invalidate();
			m_nCurrentLocationID = -2;
		}
		else if(m_dwLocIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_CCHIT_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(llcID, (long)m_dwLocIDList.GetAt(0));
			m_nCurrentLocationID = (long)m_dwLocIDList.GetAt(0);
		}
		else {			
			//they can't get here
			m_strLocationList = "";
			ShowDlgItem(IDC_CCHIT_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(llcID, m_nCurrentLocationID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_CCHIT_LOCATION_LIST, SW_HIDE);
			m_nxlLocationLabel.SetText(GetLocationNamesFromIDString(m_strLocationList));
			m_nxlLocationLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_CCHIT_MULTI_LOC_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_CCHIT_MULTI_LOC_LIST);
		}
		else {
			//They selected exactly one. 
			ShowDlgItem(IDC_CCHIT_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_CCHIT_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(0, m_nCurrentLocationID);
		}
	}

	return bReturn;
	
}

	// (j.gruber 2011-05-18 10:34) - PLID 43758
CString CCCHITReportsDlg::GetLocationNamesFromIDString(CString strIDs)
{
	// we have the information in the datalist, let's just get it from there instead of querying the data
	//it should be faster that way
	CString strReturn = "";

	//get the parentheses off
	strIDs.TrimRight(')');
	strIDs.TrimLeft('(');

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {

		//there is only one ID, so find item name
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationList->FindByColumn(llcID, (long)atoi(strIDs), 0, FALSE);
		if (pRow) {
			return VarString(pRow->GetValue(llcName));
		}
		else {
			//hmm
			//ASSERT(FALSE);
			return "<All Locations>";
		}
	}
	else {

		//make a map out of our procedure IDs
		CMap <long, long, long, long> mapIDs;
		
	
		while(nResult != -1) {

			long nID = atoi(strIDs.Left(nResult));
			mapIDs.SetAt(nID, nID);

			//take off this string
			strIDs = strIDs.Right(strIDs.GetLength() - (nResult + 1));

			//trim the string
			strIDs.TrimRight();
			strIDs.TrimLeft();

			//now search again
			nResult = strIDs.Find(",");
		}

		strIDs.TrimRight();
		strIDs.TrimLeft();
		//now add the last one
		mapIDs.SetAt(atoi(strIDs), atoi(strIDs));


		//alrighty, now that we have our map, loop through the datalist and look to see its its an ID we want
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationList->GetFirstRow();
		while (pRow) {						
			long nID = VarLong(pRow->GetValue(llcID));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(llcName)) + ", ";
			}
			pRow = pRow->GetNextRow();
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;

}

// (j.gruber 2011-05-18 10:35) - PLID 43758
void CCCHITReportsDlg::RequeryFinishedLocationList(short nFlags)
{
	try {	

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLocationList->GetNewRow();
		pRow->PutValue(llcID, (long) -2);
		pRow->PutValue(llcName, _variant_t("<Multiple Locations>"));
		m_pLocationList->AddRowBefore(pRow, m_pLocationList->GetFirstRow());

		pRow = m_pLocationList->GetNewRow();
		pRow->PutValue(llcID, (long) -1);
		pRow->PutValue(llcName, _variant_t("<All Locations>"));
		m_pLocationList->AddRowBefore(pRow, m_pLocationList->GetFirstRow());

		m_pLocationList->CurSel = pRow;		

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-11-01 13:02) - PLID 46222
LRESULT CCCHITReportsDlg::OnProcessingAddRow(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.gruber 2011-11-04 15:27) - PLID 46222 - if the dialog is closing, don't bother
		if (m_bDialogClosing) {
			return 0;
		}

		long nIndex = (long)wParam;		

		CCCHITReportInfo &riReport = m_aryReports.ElementAt(nIndex);
		
		//Interface row
		// (b.savon 2014-05-14 08:58) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”. Move the measure to the very bottom of the list.
		//Don't add the OBSOLETE rows now, add them at the end
		if (riReport.m_nInternalID != (long)CCHITReportMeasures::eMUElectronicCopyRequest){
			AddRowToInterface(riReport, false, RGB(255, 255, 255));
		}

		//update our progress bar
		m_nCurrentMeasureCount++;
		CString strText;
		strText.Format("Please wait while Practice calculates each measure...(%d / %d)",
		m_nCurrentMeasureCount, m_nTotalMeasures);
		GetDlgItem(IDC_PROGRESS_CCHIT_MEASURES)->PostMessage(PBM_SETPOS, m_nCurrentMeasureCount);		
		SetDlgItemText(IDC_STATIC_MEASURE_PROGRESS, strText);				
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (b.savon 2014-05-14 08:58) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”. Move the measure to the very bottom of the list.
void CCCHITReportsDlg::AddRowToInterface(CCCHITReportInfo &riReport, bool bAtTheEnd, OLE_COLOR color)
{
	IRowSettingsPtr pRow = NULL;

	if (riReport.GetReportType() == crtMU) {
		pRow = m_pList->GetNewRow();
	}
	else {
		pRow = m_pCCHITList->GetNewRow();
	}

	if (pRow) {
		pRow->PutValue(eInternalName, _bstr_t(riReport.m_strInternalName));
		pRow->PutValue(eName, _bstr_t(riReport.m_strDisplayName));
		double dblPercent = riReport.GetPercentage();
		CString strPercent;
		strPercent.Format("%.2f", dblPercent);
		pRow->PutValue(ePercent, _bstr_t(strPercent));
		pRow->PutValue(eNum, (long)riReport.GetNumerator());
		pRow->PutValue(eDenom, (long)riReport.GetDenominator());
		pRow->PutValue(eHelp, _bstr_t("Click for More Info"));
		//Save the help data in the datalist for retrieval when requesting more info
		pRow->PutValue(eHelpGeneral, _bstr_t(riReport.GetHelpGeneral()));
		pRow->PutValue(eHelpNum, _bstr_t(riReport.GetHelpNum()));
		pRow->PutValue(eHelpDenom, _bstr_t(riReport.GetHelpDenom()));
		CString strConfigure = "";
		if (riReport.GetConfigureType() != crctNone) { strConfigure = "<Configure>"; }
		pRow->PutValue(eConfigure, _bstr_t(strConfigure));
		pRow->PutValue(eConfigureType, (long)riReport.GetConfigureType());
		// (j.gruber 2011-11-08 11:17) - PLID 45689 - new reports items
		pRow->PutValue(eReportSelect, _bstr_t(riReport.GetReportSelect()));
		pRow->PutValue(eReportNumFrom, _bstr_t(riReport.GetReportNumFrom()));
		pRow->PutValue(eReportDenomFrom, _bstr_t(riReport.GetReportDenomFrom()));
		pRow->PutBackColor(color);

		//add it
		// (j.gruber 2011-05-13 12:51) - PLID 43694 - check the configuration
		if (riReport.GetReportType() == crtMU) {
			if (!m_bDialogClosing) {
				if (bAtTheEnd){
					m_pList->AddRowAtEnd(pRow, NULL);
				}
				else{
					m_pList->AddRowSorted(pRow, NULL);
				}
			}
		}
		else {
			if (!m_bDialogClosing) {
				//it has to be CCHIT or else we wouldn't be here
				if (bAtTheEnd){
					m_pCCHITList->AddRowAtEnd(pRow, NULL);
				}
				else{
					m_pCCHITList->AddRowSorted(pRow, NULL);
				}
			}
		}
	}
}

// This message is posted from the worker thread to inform this object that the processing is finished
// (j.gruber 2011-11-01 13:02) - PLID 46222
LRESULT CCCHITReportsDlg::OnProcessingFinished(WPARAM wParam, LPARAM lParam)
{
	try {
		SetDlgItemText(IDC_STATIC_MEASURE_PROGRESS, "Done");
		GetDlgItem(IDC_PROGRESS_CCHIT_MEASURES)->PostMessage(PBM_SETPOS, 0);		

		BOOL bEndedEarly = (BOOL)wParam;

		if (!bEndedEarly) {
			m_bDoneLoading = true;
		}

		//re-enable the load button 
		GetDlgItem(IDC_CCHIT_LOAD)->EnableWindow(TRUE);

		// (r.farnworth 2013-12-03 09:29) - PLID 58995 - Re-enable the Stage buttons
		GetDlgItem(IDC_RADIO_STAGE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_STAGE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_MOD_STAGE2)->EnableWindow(TRUE);

		// (b.savon 2014-05-14 08:58) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”. Move the measure to the very bottom of the list.
		for (int i = 0; i < m_aryReports.GetCount(); i++){
			CCCHITReportInfo &riReport = m_aryReports.ElementAt(i);
			if (riReport.m_nInternalID == (long)CCHITReportMeasures::eMUElectronicCopyRequest){
				AddRowToInterface(riReport, true, RGB(225, 225, 225));
				if (riReport.GetReportType() == crtMU) {
					m_pList->PutAllowSort(VARIANT_FALSE);
				}
				else {
					m_pCCHITList->PutAllowSort(VARIANT_FALSE);
				}
				break;
			}
		}

		//delete the thread
		KillThread();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.gruber 2011-11-01 13:25) - PLID 46222
void CCCHITReportsDlg::OnDestroy() 
{
	try {
		KillThread();
	}
	NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}


// (j.gruber 2011-11-02 09:54) - PLID 46222
void CCCHITReportsDlg::KillThread() 
{
	if (m_pThread) 
	{
		// Set the event to break the loop in the thread
		SetEvent(m_hStopThread);

		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so, terminate it
			m_pThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pThread->m_nThreadID, WM_QUIT, 0, 0);			
			WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		}	
		else {
			// The thread is finished, so just delete it
			delete m_pThread;
		}
	
		m_pThread = NULL;
		CloseHandle(m_hStopThread);
		m_hStopThread = NULL;
	}


}
// (j.gruber 2011-11-03 15:03) - PLID 46219
void CCCHITReportsDlg::OnCancel() 
{
	try {
		CloseWindow();
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2011-11-04 12:09) - PLID 44993
void CCCHITReportsDlg::OnBnClickedMUPreview()
{
	try {
		//first we need to make sure we are done loading
		if (!m_bDoneLoading) {
			AfxMessageBox("Please make sure all measures have loaded before running the report");
			return;
		}

		//create the sql
		CString strSql;
		BOOL bCCHITHidden = GetRemotePropertyInt("CCHITReportsMeaningfulUseOnly", 1, 0, "<None>");
		for(int i = 0; i < m_aryReports.GetSize(); i++) {
			
			if (((bCCHITHidden) && m_aryReports.GetAt(i).GetReportType() == crtMU) ||
				(!bCCHITHidden) 
			) {		
				CCCHITReportInfo &riReport = m_aryReports.ElementAt(i);

				CString strTemp;

				CString strPercent;
				double dblPercent = riReport.GetPercentage();
				strPercent.Format("%.2f%%", dblPercent);			

				// (j.gruber 2011-11-04 13:23) - PLID 45692  - changed to display name
				strTemp.Format("SELECT '%s' as Name, %li as Numerator, %li as Denominator, '%s' as Percentage, %li as Type \r\n UNION \r\n",
					riReport.m_strDisplayName, riReport.GetNumerator(), riReport.GetDenominator(), strPercent, 
					riReport.GetReportType() == crtMU ? 1 : 2
				);

				strSql += strTemp;

			}
		}
		// (r.farnworth 2013-10-14 18:01) - PLID 58995 - Ideally, this should never happen, but if a load was successfully finished and no measures were put into the list
		//	an exception would be thrown. This exists as a better error handler in this situation.
		if (strSql.IsEmpty()) {
			AfxMessageBox("Practice could not find any measures to display in the report.\r\nPlease make sure all measures have loaded before running the report");
			return;
		}

		//take off the last union
		strSql.TrimRight();
		strSql.TrimRight("UNION");

		//minimize
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp)) {
			//Check if we are not minimized
			if (!IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				SetWindowPlacement(&wp);
				GetMainFrame()->SetForegroundWindow();
			}
		}

		//declare the report
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(718)]);
		infReport.strListBoxSQL = strSql;

		CPtrArray paParams;
		CRParameterInfo *paramInfo;
		
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetCurrentUserName();
		paramInfo->m_Name = "CurrentUserName";
		paParams.Add(paramInfo);

		COleDateTime dt = COleDateTime::GetCurrentTime();
		CString strDate = dt.Format("%m/%d/%Y");

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = FormatDateTimeForInterface(m_dtLastDateFromRun, 0, dtoDate);
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = FormatDateTimeForInterface(m_dtLastDateToRun, 0, dtoDate);
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetProviderNamesFromIDString(m_strLastProvsRun).Left(225);
		paramInfo->m_Name = "ProvFilter";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetLocationNamesFromIDString(m_strLastLocsRun).Left(225);
		paramInfo->m_Name = "LocFilter";
		paParams.Add((void *)paramInfo);	

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = AsString(m_bLastRunExcludeSecondaries);
		paramInfo->m_Name = "ExcludeSecondaries";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = m_strLastRunExclusionsNameList.Left(225);
		paramInfo->m_Name = "Exclusions";
		paParams.Add((void *)paramInfo);

		RunReport(&infReport, &paParams, true, (CWnd *)this, "Meaningful Use Preview");
		ClearRPIParameterList(&paParams);
	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2011-11-04 15:56) - PLID 46222
void CCCHITReportsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{

	try {
		if (bShow) {
			m_pList->Clear();
			m_pCCHITList->Clear();

			//set the progress to none		
			SetDlgItemText(IDC_STATIC_MEASURE_PROGRESS, "");
			GetDlgItem(IDC_PROGRESS_CCHIT_MEASURES)->PostMessage(PBM_SETPOS, 0);		

			//reset our values
			m_nTotalMeasures = 0;
			m_nCurrentMeasureCount = 0;

			GetDlgItem(IDC_CCHIT_LOAD)->EnableWindow(TRUE);

			// (r.farnworth 2013-12-03 09:29) - PLID 58995
			GetDlgItem(IDC_RADIO_STAGE1)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_STAGE2)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_MOD_STAGE2)->EnableWindow(TRUE);

			m_bDialogClosing = FALSE;
		}
	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2011-11-04 16:38) - PLID 45365
void CCCHITReportsDlg::OnBnClickedCCHITExclusions()
{
	try {

		CString strFrom, strWhere;
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "EMRTemplateT");
		HRESULT hRes;
		bool bDontFill = false;

		long nResult = 0;
		
		// Fill the dialog with existing selections			
		dlg.PreSelect(m_dwExcludedTemplateIDs);
		
		dlg.m_strNameColTitle = "Name";
		
		strFrom = "EMRTemplateT";
		strWhere = "Deleted = 0 ";
				
		hRes = dlg.Open(strFrom, strWhere, "EMRTemplateT.ID", "EMRTemplateT.Name", "Please select the EMR templates to exclude from the measures. These should be templates where the doctor did not actually see or consult with the patient, for example NexWeb templates or phone messages. ");		
		
		// Update our array of providers with this information
		if (hRes == IDOK)
		{
			// (j.gruber 2011-11-16 11:33) - PLID 45689
			m_strExclusionsNameList = dlg.GetMultiSelectString();

			//dlg.FillArrayWithIDs(m_dwExcludedTemplateIDs);
			m_strExclusionsList = "(" + dlg.GetMultiSelectIDString(",") + ")";
			if (m_strExclusionsList == "()") {
				m_dwExcludedTemplateIDs.RemoveAll();
				m_strExclusionsList = "";
				m_btnExclusions.SetTextColor(RGB(0,0,0));
				SetRemotePropertyMemo("MU_ExcludedTemplates", m_strExclusionsList, 0, "<None>");
			}
			else {
				dlg.FillArrayWithIDs(m_dwExcludedTemplateIDs);
				m_btnExclusions.SetTextColor(RGB(255,0,0));
				SetRemotePropertyMemo("MU_ExcludedTemplates", m_strExclusionsList, 0, "<None>");
			}			
		}		


	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2011-11-07 12:03) - PLID 45365
void CCCHITReportsDlg::LoadExclusions()
{	

	CString strIDs = GetRemotePropertyMemo("MU_ExcludedTemplates", 0, 0, "<None>");
	m_strExclusionsList = strIDs;

	m_dwExcludedTemplateIDs.RemoveAll();

	if (strIDs.IsEmpty()) {
		//make the text in the box black
		m_btnExclusions.SetTextColor(RGB(0,0,0));
	}
	else {
		//make it red
		m_btnExclusions.SetTextColor(RGB(255,0,0));
	}

	//get the parentheses off
	strIDs.TrimRight(')');
	strIDs.TrimLeft('(');

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {		
		m_dwExcludedTemplateIDs.Add(atoi(strIDs));		
	}
	else {		
	
		while(nResult != -1) {

			long nID = atoi(strIDs.Left(nResult));
			m_dwExcludedTemplateIDs.Add(nID);

			//take off this string
			strIDs = strIDs.Right(strIDs.GetLength() - (nResult + 1));

			//trim the string
			strIDs.TrimRight();
			strIDs.TrimLeft();

			//now search again
			nResult = strIDs.Find(",");
		}

		strIDs.TrimRight();
		strIDs.TrimLeft();
		//now add the last one
		m_dwExcludedTemplateIDs.Add(atoi(strIDs));		
	}

	// (j.gruber 2011-11-16 11:41) - PLID 45689
	m_strExclusionsNameList = GetTemplateNamesFromIDString(m_strExclusionsList);
}

// (j.gruber 2011-11-08 11:39) - PLID 45689
long CCCHITReportsDlg::FindIndexFromName(CString strInternalMeasureName) 
{

	for(int i = 0; i < m_aryReports.GetSize(); i++) {
		if (m_aryReports[i].m_strInternalName == strInternalMeasureName) {
			return i;
		}
	}

	// if we got here it means that we can't find the name which should be impossible
	ThrowNxException("Error in FindIndexFromName::Could not find internal name");
}


// (j.gruber 2011-11-08 11:39) - PLID 45689
void CCCHITReportsDlg::PreviewReport(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol)
{
	
	//first get our query values out
	CString strSelect = VarString(pRow->GetValue(eReportSelect));
	CString strNumFrom = VarString(pRow->GetValue(eReportNumFrom));
	CString strDenomFrom = VarString(pRow->GetValue(eReportDenomFrom));	

	CString strSql;	
	
	CString strMasterNumSelect, strMasterDenomSelect;

	long nIndex = FindIndexFromName(VarString(pRow->GetValue(eInternalName)));
	CCCHITReportInfo riReport = m_aryReports[nIndex];
	
	strMasterNumSelect.Format(" SELECT Q.PatientID, Q.UserDefinedID, Q.First, Q.Middle, Q.Last, Q.Address1, Q.Address2, \r\n"
		" Q.City, Q.State, Q.Zip, Q.Birthdate, Q.HomePhone, Q.WorkPhone,  \r\n "
		" Q.ItemDescriptionLine, Q.ItemDate, Q.ItemDescription, Q.ItemProvider, Q.ItemSecProvider, Q.ItemLocation, \r\n"
		" Q.MiscDesc, Q.ItemMisc, Q.Misc2Desc, Q.ItemMisc2, \r\n"
		" Q.Misc3Desc, Q.ItemMisc3, Q.Misc4Desc, Q.ItemMisc4, \r\n "
		" CASE WHEN Q.ItemSecProvider IS NULL OR Q.ItemSecProvider = '' THEN 0 ELSE 1 END As HasSecondary, \r\n "		
		" %li as Type, \r\n "
		" '%s' as NumExplanation, '%s' as DenomExplanation, '%s' as ReportExplanation \r\n"
		" FROM (%s %s) Q", 
		1/*nCol == eNum ? 1 : nCol == eDenom ? 2 : 3*/, 
		_Q(riReport.GetHelpNum()), _Q(riReport.GetHelpDenom()), _Q(riReport.GetHelpGeneral()),
		strSelect, strNumFrom);

	strMasterDenomSelect.Format(" SELECT Q.PatientID, Q.UserDefinedID, Q.First, Q.Middle, Q.Last, Q.Address1, Q.Address2, \r\n"
		" Q.City, Q.State, Q.Zip, Q.Birthdate, Q.HomePhone, Q.WorkPhone,  \r\n "
		" Q.ItemDescriptionLine, Q.ItemDate, Q.ItemDescription, Q.ItemProvider, Q.ItemSecProvider, Q.ItemLocation, \r\n"
		" Q.MiscDesc, Q.ItemMisc, Q.Misc2Desc, Q.ItemMisc2, \r\n"
		" Q.Misc3Desc, Q.ItemMisc3, Q.Misc4Desc, Q.ItemMisc4, \r\n "
		" CASE WHEN Q.ItemSecProvider IS NULL OR Q.ItemSecProvider = '' THEN 0 ELSE 1 END As HasSecondary, \r\n "
		" %li as Type, \r\n "
		" '%s' as NumExplanation, '%s' as DenomExplanation, '%s' as ReportExplanation \r\n"
		" FROM (%s %s) Q", 
		2/*nCol == eNum ? 1 : nCol == eDenom ? 2 : 3*/, 
		_Q(riReport.GetHelpNum()), _Q(riReport.GetHelpDenom()), _Q(riReport.GetHelpGeneral()),
		strSelect, strDenomFrom);

	

	CString strExtraDecs;
	if (riReport.m_strInternalName.Find("MU.12") != -1) {

		//we need to append the 4 business days
		//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
		strExtraDecs = ""
		" SET @4BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @DateTo) = 'Sunday' THEN 5 ELSE "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Saturday' THEN 4 ELSE  "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Friday' THEN 4 ELSE "
		"	  6 END END END), @DateTo));\r\n ";
	}
	else if (riReport.m_strInternalName.Find("MU.14") != -1) {
		strExtraDecs = 	" SET @DateToUse = DateAdd(dd,-1,@DateTo);  \r\n";
	}

	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	CString strDeclarations = riReport.GetFilterDeclarations();
	// (r.gonet 06/12/2013) - PLID 55151 - Get the initialization sql that must be run before the num and denom queries.
	// (c.haag 2015-08-31) - PLID 65056 - Apply filters to the initialization sql
	CString strInitializationSql = riReport.GetInitializationSql(m_dtLastDateFromRun, m_dtLastDateToRun, m_strLastProvsRun, m_strLastLocsRun, m_strLastRunExclusionsList, m_bLastRunExcludeSecondaries);
	CString strMasterPercentSelect = riReport.ApplyFilters(strExtraDecs + strMasterNumSelect + " UNION ALL " + strMasterDenomSelect, m_dtLastDateFromRun, m_dtLastDateToRun, m_strLastProvsRun, m_strLastLocsRun, m_strLastRunExclusionsList, m_bLastRunExcludeSecondaries);
	strMasterNumSelect = riReport.ApplyFilters(strExtraDecs + strMasterNumSelect, m_dtLastDateFromRun, m_dtLastDateToRun, m_strLastProvsRun, m_strLastLocsRun, m_strLastRunExclusionsList, m_bLastRunExcludeSecondaries);
	strMasterDenomSelect = riReport.ApplyFilters(strExtraDecs + strMasterDenomSelect, m_dtLastDateFromRun, m_dtLastDateToRun, m_strLastProvsRun, m_strLastLocsRun, m_strLastRunExclusionsList, m_bLastRunExcludeSecondaries);
		
	//declare our report
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(720)]);
	long nType = -1;

	//(e.lally 2012-04-24) PLID 48266 - Prepend the declarations
	// (r.gonet 06/12/2013) - PLID 55151 - Add the initializaton sql
	infReport.strListBoxSQL = strDeclarations + "\r\n" + strInitializationSql + "\r\n";
	if (nCol == eNum) {
		infReport.strListBoxSQL += strMasterNumSelect;
		nType = 1;
	}
	else if (nCol == eDenom) {
		infReport.strListBoxSQL += strMasterDenomSelect;
		nType = 2;
	}
	else {
		infReport.strListBoxSQL += strMasterPercentSelect;
		nType = 3;
	}
	// (r.gonet 06/12/2013) - PLID 55151 - Add the cleanup sql if necessary.
	CString strCleanupSql = riReport.GetCleanupSql().Flatten();
	if(strCleanupSql != "") {
		infReport.strListBoxSQL += "\r\n" + strCleanupSql;
	}
	
	CPtrArray paParams;
	CRParameterInfo *paramInfo;
	
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = GetCurrentUserName();
	paramInfo->m_Name = "CurrentUserName";
	paParams.Add(paramInfo);	

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = VarString(pRow->GetValue(eName));
	paramInfo->m_Name = "Title";
	paParams.Add((void *)paramInfo);

	long nCountDetails = 0;
	long nMiscDetails = 0;
	if (riReport.m_strInternalName.Find("MU.13") != -1
		|| riReport.m_strInternalName.Find("MU.08") != -1
		|| riReport.m_strInternalName.Find("MU.11") != -1
		|| riReport.m_strInternalName.Find("MU.15") != -1
		|| riReport.m_strInternalName.Find("MU.16") != -1
	)
	{
		nCountDetails = 1;
		nMiscDetails = 1;
	}

	long nHideDetails = 0;
	//this option hides the first line of the detail, but if the nMiscDetails is set to 1, will show the misc details of the detail
	//so, for MU.12 we need this because we need to show the visit date, but since it just groups
	//on patient and date, we don't want to show any particular EMN
	if (riReport.m_strInternalName.Find("MU.13") != -1) {
		nHideDetails = 1;
	}

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = AsString(nType);
	paramInfo->m_Name = "ReportType";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = AsString(nHideDetails);
	paramInfo->m_Name = "HideDetail";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = AsString(nMiscDetails);
	paramInfo->m_Name = "MiscDetails";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = AsString(nCountDetails);
	paramInfo->m_Name = "CountDetails";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = FormatDateTimeForInterface(m_dtLastDateFromRun, 0, dtoDate);
	paramInfo->m_Name = "DateFrom";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = FormatDateTimeForInterface(m_dtLastDateToRun, 0, dtoDate);
	paramInfo->m_Name = "DateTo";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = GetProviderNamesFromIDString(m_strLastProvsRun).Left(225);
	paramInfo->m_Name = "ProvFilter";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = GetLocationNamesFromIDString(m_strLastLocsRun).Left(225);
	paramInfo->m_Name = "LocFilter";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = AsString(m_bLastRunExcludeSecondaries);
	paramInfo->m_Name = "ExcludeSecondaries";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = m_strLastRunExclusionsNameList.Left(225);
	paramInfo->m_Name = "Exclusions";
	paParams.Add((void *)paramInfo);

	CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);

	RunReport(&infReport, &paParams, true, (CWnd *)this, "Meaningful Use Measure Preview");
	ClearRPIParameterList(&paParams);

	//minimize
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp)) {
		//Check if we are not minimized
		if (!IsIconic()) {
			wp.showCmd = SW_MINIMIZE;
			SetWindowPlacement(&wp);
			GetMainFrame()->SetForegroundWindow();
		}
	}


}

// (j.gruber 2011-11-16 11:23) - PLID 45689
CString CCCHITReportsDlg::GetTemplateNamesFromIDString(CString strIDs)
{	

		//have to run a recordset
	CString strReturn = "";
	CArray<long, long> ary;


	//get the parentheses off
	strIDs.TrimRight(')');
	strIDs.TrimLeft('(');

	strIDs.TrimLeft();

	if (strIDs.IsEmpty()) {
		return "";
	}

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {

		//there is only one
		ary.Add(atoi(strIDs));
	}
	else {
		
		while(nResult != -1) {

			long nID = atoi(strIDs.Left(nResult));
			ary.Add(nID);

			//take off this string
			strIDs = strIDs.Right(strIDs.GetLength() - (nResult + 1));

			//trim the string
			strIDs.TrimRight();
			strIDs.TrimLeft();

			//now search again
			nResult = strIDs.Find(",");
		}

		strIDs.TrimRight();
		strIDs.TrimLeft();
		//now add the last one
		ary.Add(atoi(strIDs));
	}
		
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM EMRTemplateT WHERE ID IN ({INTARRAY})", ary);
	while (! rs->eof) {
		strReturn +=  AdoFldString(rs, "Name", "") + ", ";

		rs->MoveNext();
	}

	//remove the last
	strReturn.TrimRight(", ");

	return strReturn;

}
void CCCHITReportsDlg::OnOpenPqri()
{
	try {
		// (b.eyers 2016-04-29 14:59) - NX-100350 
		OpenPQRSExporter(GetSubRegistryKey());
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2013-10-14 15:33) - PLID 58995 - Allow the user to specifiy Stage 1 or 2 in the Meaningful Use Reporing dialog.
void CCCHITReportsDlg::OnBnClickedRadioStage1()
{
	try {
		HandleStageChange(MU::Stage1);
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2013-10-14 15:33) - PLID 58995 - Allow the user to specifiy Stage 1 or 2 in the Meaningful Use Reporing dialog.
void CCCHITReportsDlg::OnBnClickedRadioStage2()
{
	try {
		HandleStageChange(MU::Stage2);
	}NxCatchAll(__FUNCTION__);
}


void CCCHITReportsDlg::OnBnClickedRadioModStage2()
{
	try {
		HandleStageChange(MU::ModStage2);
	}NxCatchAll(__FUNCTION__);
}

void CCCHITReportsDlg::HandleStageChange(MU::Stage eStage)
{
	// only set the meaningful use stage if it's actually changed
	if (m_eMeaningfulUseStage != eStage)
	{
		m_eMeaningfulUseStage = eStage;

		if (m_bDoneLoading) {
			m_pList->Clear();
		}
	}
}
