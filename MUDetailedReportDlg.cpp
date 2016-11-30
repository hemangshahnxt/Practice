// MUDetailedReportDlg.cpp : implementation file
//

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Created.

#include "stdafx.h"
#include "MUDetailedReportDlg.h"
#include "PracticeRc.h"
#include "MsgBox.h"
#include "foreach.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MEASURE_FAILED_COLOR RGB(222,122,122)
#define MEASURE_SUCCESS_COLOR RGB(122,222,122)
#define NUM_ROW_COLOR RGB(160,224,224)
#define DENOM_ROW_COLOR RGB(153,153,51)
#define PERCENT_MET_ROW_COLOR RGB(255,128,128)
#define REQUIRED_ROW_COLOR RGB(255,255,255)
#define DEFICIENT_ROW_COLOR RGB(153,102,102)
#define DEFICIENT_CELL_FORE_COLOR RGB(255,0,0)
#define PATIENT_NEW_ROW RGB(125,125,125)

enum LocationListColumns {
	llcID = 0,
	llcName,
};

namespace MUDtlProvList{
	enum MUDtlProvListCol{
		ID = 0,
		Archived,
		Name,
		Color,
	};
};

using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CMUDetailedReportDlg dialog

CMUDetailedReportDlg::CMUDetailedReportDlg(CWnd* pParent)
	: CNxDialog(CMUDetailedReportDlg::IDD, pParent, "CMUDetailedReportDlg")
{
	m_nCurrentSeenLocationID = -1;
	m_nCurrentLocationID = -1;
	m_nCurrentProvID = -1;
	m_bDialogVisible = false;
}

void CMUDetailedReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMUDetailedReportDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_MU_DTL_EXCLUSIONS, m_btnExclusions);
	DDX_Control(pDX, IDC_LOAD, m_btnLoad);
	DDX_Control(pDX, IDC_MU_DET_PREVIEW, m_btnExport);
	DDX_Control(pDX, IDC_MU_MULTI_LOC_SEEN_LIST, m_nxlLocationSeenLabel);
	DDX_Control(pDX, IDC_MU_MULTI_LOC_LIST, m_nxlLocationLabel);
	DDX_Control(pDX, IDC_MU_PROV_MULTI_LIST, m_nxlProviderLabel);
	DDX_Control(pDX, IDC_MU_FROM, m_MUFrom);
	DDX_Control(pDX, IDC_MU_TO, m_MUTo);
	DDX_Control(pDX, IDC_SEEN_FROM, m_SeenFrom);
	DDX_Control(pDX, IDC_SEEN_TO, m_SeenTo);
	DDX_Control(pDX, IDC_MU_DTL_PROGRESS_STATIC, m_nxlProgressLabel);
	DDX_Control(pDX, IDC_MOVE_LEFT, m_btnLeft);
	DDX_Control(pDX, IDC_MOVE_RIGHT, m_btnRight);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMUDetailedReportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMUDetailedReportDlg)	
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOAD, &CMUDetailedReportDlg::OnBnClickedLoad)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	ON_BN_CLICKED(IDC_MU_DET_PREVIEW, &CMUDetailedReportDlg::OnBnClickedMuDetPreview)
	ON_MESSAGE(WM_ALL_MEASURES_COMPLETE, &CMUDetailedReportDlg::OnAllMeasuresComplete)
	ON_MESSAGE(WM_MEASURE_LOAD_CANCEL, &CMUDetailedReportDlg::OnMeasureLoadCancel)
	ON_MESSAGE(WM_MEASURE_COMPLETE, &CMUDetailedReportDlg::OnMeasureComplete)
	ON_MESSAGE(WM_MEASURE_PRELOAD_COMPLETE, &CMUDetailedReportDlg::OnMeasurePreload)
	ON_BN_CLICKED(IDC_MU_DTL_DISABLE_SEEN_FILTER, &CMUDetailedReportDlg::OnBnClickedMuDtlDisableSeenFilter)
	ON_BN_CLICKED(IDC_MU_DTL_EXCLUSIONS, &CMUDetailedReportDlg::OnBnClickedMuDtlExclusions)
	ON_BN_CLICKED(IDOK, &CMUDetailedReportDlg::OnBnClickedOk)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_DSTAGE1, &CMUDetailedReportDlg::OnBnClickedRadioDstage1)
	ON_BN_CLICKED(IDC_RADIO_DSTAGE2, &CMUDetailedReportDlg::OnBnClickedRadioDstage2)
	ON_BN_CLICKED(IDC_MOVE_LEFT, &CMUDetailedReportDlg::OnBnClickedMoveLeft)
	ON_BN_CLICKED(IDC_RADIO_MOD_DSTAGE2, &CMUDetailedReportDlg::OnBnClickedRadioModDstage2)
	ON_BN_CLICKED(IDC_MOVE_RIGHT, &CMUDetailedReportDlg::OnBnClickedMoveRight)END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMUDetailedReportDlg message handlers

BOOL CMUDetailedReportDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		g_propManager.CachePropertiesInBulk("MUDetailedReportDlg_Memo", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			" Name = 'MU_ExcludedTemplates' "
			")",
			_Q(GetCurrentUserName()));
		
		g_propManager.CachePropertiesInBulk("MUDetailedReportDlg_Dates", propDateTime,
			"(Username = '<None>' OR Username = '%s') AND ("
			" Name = 'MU_Dtl_DateFrom' OR "
			" Name = 'MU_Dtl_DateTo' "
			")",
			_Q(GetCurrentUserName()));

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtMUFrom = GetRemotePropertyDateTime("MU_Dtl_DateFrom", &(dtNow + COleDateTimeSpan(-365, 0, 0, 0)), 0, "<None>", FALSE);
		COleDateTime dtMUTo = GetRemotePropertyDateTime("MU_Dtl_DateTo", &dtNow, 0, "<None>", FALSE);

		m_MUFrom.SetValue(dtMUFrom);
		m_MUTo.SetValue(dtMUTo);
		m_SeenFrom.SetValue(dtNow);
		m_SeenTo.SetValue(dtNow);

		m_pReport = BindNxDataList2Ctrl(IDC_MU_DETAILED_LIST, false);
		m_pSummary = BindNxDataList2Ctrl(IDC_MU_REPORT_SUMMARY, false);	
		m_pLocationList = BindNxDataList2Ctrl(IDC_MU_DTL_LOCATION_LIST);
		m_pProviders = BindNxDataList2Ctrl(IDC_MU_DTL_PROVIDER_LIST);
		m_pSeenLocationList = BindNxDataList2Ctrl(IDC_MU_DTL_SEEN_LOCATION_LIST);
		m_pSeenProviders = BindNxDataList2Ctrl(IDC_MU_DTL_SEEN_PROVIDER_LIST);

		m_nxlLocationSeenLabel.SetColor(0x00C8FFFF);
		m_nxlLocationSeenLabel.SetText("");
		m_nxlLocationSeenLabel.SetType(dtsHyperlink);
		m_nxlLocationSeenLabel.ShowWindow(SW_HIDE);

		m_nxlLocationLabel.SetColor(0x00C8FFFF);
		m_nxlLocationLabel.SetText("");
		m_nxlLocationLabel.SetType(dtsHyperlink);
		m_nxlLocationLabel.ShowWindow(SW_HIDE);

		m_nxlProviderLabel.SetColor(0x00C8FFFF);
		m_nxlProviderLabel.SetText("");
		m_nxlProviderLabel.SetType(dtsHyperlink);
		m_nxlProviderLabel.ShowWindow(SW_HIDE);

		m_nxlProgressLabel.SetWindowText("Waiting...");

		// Preselect Mod. Stage 2 OnInit
		((CButton*)GetDlgItem(IDC_RADIO_DSTAGE1))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_DSTAGE2))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_MOD_DSTAGE2))->SetCheck(BST_CHECKED);
		m_muCalculator.m_eMeaningfulUseStage = MU::ModStage2;

		CString strExcludedIDs = GetRemotePropertyMemo("MU_ExcludedTemplates", 0, 0, "<None>");
		if(!strExcludedIDs.IsEmpty()){
			m_btnExclusions.SetTextColor(RGB(255,0,0));
		}

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnExport.AutoSet(NXB_EXPORT);
		m_btnLoad.SetWindowText("Load");
		m_btnLoad.AutoSet(NXB_REFRESH);
		m_btnLeft.AutoSet(NXB_LEFT);
		m_btnRight.AutoSet(NXB_RIGHT);
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Set the cursor when we roll over our multiselect labels
BOOL CMUDetailedReportDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		if (m_dwSeenLocIDList.GetSize() > 1 && GetDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST)->IsWindowEnabled()){
			CRect rc;
			GetDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		if (m_dwLocIDList.GetSize() > 1 && GetDlgItem(IDC_MU_MULTI_LOC_LIST)->IsWindowEnabled()){
			CRect rc;
			GetDlgItem(IDC_MU_MULTI_LOC_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		if (m_dwProvIDList.GetSize() > 1 && GetDlgItem(IDC_MU_PROV_MULTI_LIST)->IsWindowEnabled()){
			CRect rc;
			GetDlgItem(IDC_MU_PROV_MULTI_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__)
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CMUDetailedReportDlg::EnableSeenFilter(BOOL bEnable)
{
	GetDlgItem(IDC_SEEN_FROM)->EnableWindow(bEnable);
	GetDlgItem(IDC_SEEN_TO)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_SEEN_EXCLUDE_SEC)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_SEEN_PROVIDER_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST)->EnableWindow(bEnable);
}

void CMUDetailedReportDlg::EnableMUFilter(BOOL bEnable)
{
	GetDlgItem(IDC_MU_FROM)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_TO)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_EXCLUDE_SEC)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_PROVIDER_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_LOCATION_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_PROV_MULTI_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_MULTI_LOC_LIST)->EnableWindow(bEnable);
}

void CMUDetailedReportDlg::EnableUtilityButtons(BOOL bEnable)
{
	GetDlgItem(IDC_MU_DET_PREVIEW)->EnableWindow(bEnable);
	GetDlgItem(IDC_MU_DTL_EXCLUSIONS)->EnableWindow(bEnable);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Save the MU range that we have selected
void CMUDetailedReportDlg::OnDestroy() 
{
	try {
		COleDateTime dtMUFrom = COleDateTime(m_MUFrom.GetValue());
		COleDateTime dtMUTo = COleDateTime(m_MUTo.GetValue());

		// if we have valid dates, save our current range...
		if(!dtMUFrom.GetStatus() && !dtMUTo.GetStatus()){
			SetRemotePropertyDateTime("MU_Dtl_DateFrom", dtMUFrom, 0, "<None>");
			SetRemotePropertyDateTime("MU_Dtl_DateTo", dtMUTo, 0, "<None>");
		}
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

// (j.dinatale 2012-10-24 14:28) - PLID 53508 - allow our calculator to load information
void CMUDetailedReportDlg::OnBnClickedLoad()
{
	try{
		if(m_muCalculator.IsRunning()){
			m_nxlProgressLabel.SetWindowText("Load Cancelling...");
			m_muCalculator.Interrupt();			
			return;
		}

		if(!m_pProviders->CurSel || !m_pSeenProviders->CurSel){
			this->MessageBox("Please select a provider before running the report.", "Warning", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		if(!m_pLocationList->CurSel || !m_pSeenLocationList->CurSel){
			this->MessageBox("Please select a location before running the report.", "Warning", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		CString strExcludedIDs = GetRemotePropertyMemo("MU_ExcludedTemplates", 0, 0, "<None>");

		// (r.farnworth 2013-10-15 11:59) - PLID 59011 - Check what stage we are running for and pass that number along to the calculator
		if(IsDlgButtonChecked(IDC_RADIO_DSTAGE1)) {
			m_muCalculator.m_eMeaningfulUseStage = MU::Stage1;
		} else if (IsDlgButtonChecked(IDC_RADIO_DSTAGE2)) {
			m_muCalculator.m_eMeaningfulUseStage = MU::Stage2;
		} else if (IsDlgButtonChecked(IDC_RADIO_MOD_DSTAGE2)) {
			m_muCalculator.m_eMeaningfulUseStage = MU::ModStage2;
		}

		m_strCurrProvName = VarString(m_pSeenProviders->CurSel->GetValue(MUDtlProvList::Name), "");
		m_bShowProvCol = !IsDlgButtonChecked(IDC_MU_DTL_DISABLE_SEEN_FILTER);

		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETRANGE, 0, 0);
		m_nxlProgressLabel.SetWindowText("Loading MU Calculator...");

		// clear out our lists
		m_pReport->Clear();
		m_pSummary->Clear();

		long nColCount = m_pReport->GetColumnCount();
		for(int i=0; i < nColCount; i++){
			m_pReport->RemoveColumn(0);
		}

		nColCount = m_pSummary->GetColumnCount();
		for(int i=0; i < nColCount; i++){
			m_pSummary->RemoveColumn(0);
		}

		m_muCalculator.m_filterMURange.m_bUseDates = true;
		m_muCalculator.m_filterMURange.m_dtFromDate = static_cast<DATE>(static_cast<int>(m_MUFrom.GetDateTime().m_dt));
		m_muCalculator.m_filterMURange.m_dtToDate = static_cast<DATE>(static_cast<int>(m_MUTo.GetDateTime().m_dt));
		m_muCalculator.m_filterMURange.m_aryProviderList.Copy(m_dwProvIDList);
		m_muCalculator.m_filterMURange.m_aryLocationList.Copy(m_dwLocIDList);
		m_muCalculator.m_filterMURange.m_strExcludedTemplateIDs = strExcludedIDs;
		m_muCalculator.m_filterMURange.m_bExcludeSecondaries = IsDlgButtonChecked(IDC_MU_DTL_EXCLUDE_SEC);


		if(IsDlgButtonChecked(IDC_MU_DTL_DISABLE_SEEN_FILTER)){
			m_muCalculator.m_filterPatients = MUMeasureFilter();
		}else{
			DWORD dwProvID = (DWORD)VarLong(m_pSeenProviders->CurSel->GetValue(MUDtlProvList::ID));
			m_muCalculator.m_filterPatients.m_bUseDates = true;
			m_muCalculator.m_filterPatients.m_dtFromDate = static_cast<DATE>(static_cast<int>(m_SeenFrom.GetDateTime().m_dt));
			m_muCalculator.m_filterPatients.m_dtToDate = static_cast<DATE>(static_cast<int>(m_SeenTo.GetDateTime().m_dt));
			m_muCalculator.m_filterPatients.m_aryLocationList.Copy(m_dwSeenLocIDList);
			m_muCalculator.m_filterPatients.m_aryProviderList.RemoveAll();
			m_muCalculator.m_filterPatients.m_aryProviderList.Add(dwProvID);
			m_muCalculator.m_filterPatients.m_strExcludedTemplateIDs = strExcludedIDs;
			m_muCalculator.m_filterPatients.m_bExcludeSecondaries = IsDlgButtonChecked(IDC_MU_DTL_SEEN_EXCLUDE_SEC);
		}

		m_muCalculator.Run(this->GetSafeHwnd());

		EnableMUFilter(FALSE);
		EnableSeenFilter(FALSE);
		EnableUtilityButtons(FALSE);

		// (r.farnworth 2014-04-24 11:48) - PLID 59011 - Disable the buttons during a load
		GetDlgItem(IDC_RADIO_DSTAGE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_DSTAGE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_MOD_DSTAGE2)->EnableWindow(FALSE);

		m_btnLoad.SetWindowText("Cancel");
		m_btnLoad.AutoSet(NXB_CANCEL);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple MU provider selections
BOOL CMUDetailedReportDlg::SelectMultiProviders() 
{
	// MU Provider Multiple Selection
	CString strFrom, strWhere;
	CMultiSelectDlg dlg(this, "ProvidersT");
	HRESULT hRes;
	
	// Fill the dialog with existing selections	
	dlg.PreSelect(m_dwProvIDList);
	dlg.m_strNameColTitle = "Provider";
	
	strFrom = AsString(m_pProviders->GetFromClause());
	strWhere = AsString(m_pProviders->GetWhereClause());

	hRes = dlg.Open(strFrom, strWhere, "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more providers to filter on:");

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of providers with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwProvIDList);
		m_strProviderList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwProvIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlProviderLabel.SetText(strNames);
			m_nxlProviderLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_SHOW);			
			m_nxlProviderLabel.Invalidate();
			m_nCurrentProvID = -2;
		}
		else if(m_dwProvIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_SHOW);
			m_pProviders->SetSelByColumn(llcID, (long)m_dwProvIDList.GetAt(0));
			m_nCurrentProvID = (long)m_dwProvIDList.GetAt(0);
		}
		else {			
			//they can't get here
			m_strProviderList = "";
			m_nCurrentProvID = -1;
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_SHOW);
			m_pProviders->SetSelByColumn(llcID, m_nCurrentProvID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwProvIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_HIDE);
			m_nxlProviderLabel.SetText(GetNamesFromIDString(m_strProviderList, m_pProviders, MUDtlProvList::ID, MUDtlProvList::Name));
			m_nxlProviderLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_SHOW);
			m_nxlProviderLabel.Invalidate();
		}
		else {
			//They selected exactly one. 
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_SHOW);
			m_pProviders->SetSelByColumn(0, m_nCurrentProvID);
		}
	}

	return bReturn;
	
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple MU location selections
BOOL CMUDetailedReportDlg::SelectMultiLocations() 
{
	// MU Location Multiple Selection
	CString strFrom, strWhere;
	CMultiSelectDlg dlg(this, "LocationsT");
	HRESULT hRes;
	
	// Fill the dialog with existing selections	
	dlg.PreSelect(m_dwLocIDList);
	dlg.m_strNameColTitle = "Location";
	
	strFrom = AsString(m_pLocationList->GetFromClause());
	strWhere = AsString(m_pLocationList->GetWhereClause());

	hRes = dlg.Open(strFrom, strWhere, "LocationsT.ID", "LocationsT.Name", "Please select the locations to filter on.");

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of providers with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwLocIDList);
		m_strLocationList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlLocationLabel.SetText(strNames);
			m_nxlLocationLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_SHOW);			
			m_nxlLocationLabel.Invalidate();
			m_nCurrentLocationID = -2;
		}
		else if(m_dwLocIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(llcID, (long)m_dwLocIDList.GetAt(0));
			m_nCurrentLocationID = (long)m_dwLocIDList.GetAt(0);
		}
		else {			
			//they can't get here
			m_strLocationList = "";
			m_nCurrentLocationID = -1;
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(llcID, m_nCurrentLocationID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_HIDE);
			m_nxlLocationLabel.SetText(GetNamesFromIDString(m_strLocationList, m_pLocationList, llcID, llcName));
			m_nxlLocationLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_SHOW);
			m_nxlLocationLabel.Invalidate();
		}
		else {
			//They selected exactly one. 
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_SHOW);
			m_pLocationList->SetSelByColumn(0, m_nCurrentLocationID);
		}
	}

	return bReturn;
	
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple Patient Seen Location selections
BOOL CMUDetailedReportDlg::SelectMultiSeenLocations() 
{
	// Patients Seen Multiple Location
	CString strFrom, strWhere;
	CMultiSelectDlg dlg(this, "LocationsT");
	HRESULT hRes;
	
	// Fill the dialog with existing selections	
	dlg.PreSelect(m_dwSeenLocIDList);
	dlg.m_strNameColTitle = "Location";
	
	strFrom = AsString(m_pSeenLocationList->GetFromClause());
	strWhere = AsString(m_pSeenLocationList->GetWhereClause());

	hRes = dlg.Open(strFrom, strWhere, "LocationsT.ID", "LocationsT.Name", "Please select the locations to filter on.");

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of providers with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwSeenLocIDList);
		m_strSeenLocationList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwSeenLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlLocationSeenLabel.SetText(strNames);
			m_nxlLocationSeenLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_SHOW);			
			m_nxlLocationSeenLabel.Invalidate();
			m_nCurrentSeenLocationID = -2;
		}
		else if(m_dwSeenLocIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_SHOW);
			m_pSeenLocationList->SetSelByColumn(llcID, (long)m_dwSeenLocIDList.GetAt(0));
			m_nCurrentSeenLocationID = (long)m_dwSeenLocIDList.GetAt(0);
		}
		else {			
			//they can't get here
			m_strSeenLocationList = "";
			m_nCurrentSeenLocationID = -1;
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_SHOW);
			m_pSeenLocationList->SetSelByColumn(llcID, m_nCurrentSeenLocationID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwSeenLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_HIDE);
			m_nxlLocationSeenLabel.SetText(GetNamesFromIDString(m_strSeenLocationList, m_pSeenLocationList, llcID, llcName));
			m_nxlLocationSeenLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_SHOW);
			m_nxlLocationSeenLabel.Invalidate();
		}
		else {
			//They selected exactly one. 
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_HIDE);
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_SHOW);
			m_pSeenLocationList->SetSelByColumn(0, m_nCurrentSeenLocationID);
		}
	}

	return bReturn;	
}

BEGIN_EVENTSINK_MAP(CMUDetailedReportDlg, CNxDialog)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_LOCATION_LIST, 18, CMUDetailedReportDlg::RequeryFinishedMuLocationList, VTS_I2)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_LOCATION_LIST, 1, CMUDetailedReportDlg::SelChangingMuLocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_LOCATION_LIST, 16, CMUDetailedReportDlg::SelChosenMuLocationList, VTS_DISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_PROVIDER_LIST, 18, CMUDetailedReportDlg::RequeryFinishedMuDtlProviderList, VTS_I2)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_PROVIDER_LIST, 18, CMUDetailedReportDlg::RequeryFinishedMuDtlSeenProviderList, VTS_I2)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_LOCATION_LIST, 18, CMUDetailedReportDlg::RequeryFinishedMuDtlLocationList, VTS_I2)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_LOCATION_LIST, 16, CMUDetailedReportDlg::SelChosenMuDtlLocationList, VTS_DISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_LOCATION_LIST, 1, CMUDetailedReportDlg::SelChangingMuDtlLocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_PROVIDER_LIST, 1, CMUDetailedReportDlg::SelChangingMuDtlProviderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_PROVIDER_LIST, 1, CMUDetailedReportDlg::SelChangingMuDtlSeenProviderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_SEEN_PROVIDER_LIST, 16, CMUDetailedReportDlg::SelChosenMuDtlSeenProviderList, VTS_DISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DTL_PROVIDER_LIST, 16, CMUDetailedReportDlg::SelChosenMuDtlProviderList, VTS_DISPATCH)
	ON_EVENT(CMUDetailedReportDlg, IDC_MU_DETAILED_LIST, 6, CMUDetailedReportDlg::OnRButtonDownReportList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple MU location selections
void CMUDetailedReportDlg::SelChosenMuDtlLocationList(LPDISPATCH lpRow)
{
	// MU Location Filter
	try{
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

void CMUDetailedReportDlg::SelChangingMuDtlLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// MU Location Filter
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::SelChangingMuDtlProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// MU Provider Filter
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::SelChangingMuLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// Patients Seen Location Filter
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::SelChangingMuDtlSeenProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// Patients Seen Provider Filter
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::SelChosenMuDtlSeenProviderList(LPDISPATCH lpRow)
{
	// Patients Seen Provider Filter
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple MU provider selections
void CMUDetailedReportDlg::SelChosenMuDtlProviderList(LPDISPATCH lpRow)
{
	// MU Provider Filter
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(llcID));
			if (nID == -1) {
				//clear the list
				m_strProviderList = "";
				m_nCurrentProvID = -1;
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
				m_nCurrentProvID = nID;
				m_dwProvIDList.RemoveAll();
				m_dwProvIDList.Add(nID);
				
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Allow for multiple MU location selections
void CMUDetailedReportDlg::SelChosenMuLocationList(LPDISPATCH lpRow)
{
	// Patients Seen Location Filter
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(llcID));
			if (nID == -1) {
				//clear the list
				m_strSeenLocationList = "";
				m_nCurrentSeenLocationID = -1;
				m_dwSeenLocIDList.RemoveAll();
			}
			else if (nID == -2) {
				//multiple selections
				if (!SelectMultiSeenLocations()) {
					//nothing changed
					return;
				}
			}		
			else {
				//just one
				m_strSeenLocationList.Format("(%li)", nID);
				m_nCurrentSeenLocationID = nID;
				m_dwSeenLocIDList.RemoveAll();
				m_dwSeenLocIDList.Add(nID);
				
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - util function to get names from the ID string
CString CMUDetailedReportDlg::GetNamesFromIDString(CString strIDs, NXDATALIST2Lib::_DNxDataListPtr pList, short nIDCol, short nNameCol, std::vector<std::pair<long, CString>> & IDsAndNames)
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
		NXDATALIST2Lib::IRowSettingsPtr pRow = pList->FindByColumn(nIDCol, (long)atoi(strIDs), 0, FALSE);
		if (pRow) {
			// (v.maida - 2014-05-08 11:20) - PLID 61904 - Add a provider ID,name pair into a vector for later processing.
			IDsAndNames.push_back(std::make_pair(VarLong(pRow->GetValue(nIDCol)), VarString(pRow->GetValue(nNameCol))));
			return VarString(pRow->GetValue(nNameCol));
		}
		else {
			//hmm
			//ASSERT(FALSE);
			return "<All>";
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
		NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetFirstRow();
		while (pRow) {						
			long nID = VarLong(pRow->GetValue(nIDCol));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(nNameCol)) + ", ";
				// (v.maida - 2014-05-08 11:20) - PLID 61904 - Add a provider ID,name pair into a vector for later processing.
				IDsAndNames.push_back(std::make_pair(nID, VarString(pRow->GetValue(nNameCol))));
			}
			pRow = pRow->GetNextRow();
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;

}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Handle label clicks for our multi selection lists
LRESULT CMUDetailedReportDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
			case IDC_MU_MULTI_LOC_SEEN_LIST:
				SelectMultiSeenLocations();
				break;

			case IDC_MU_MULTI_LOC_LIST:
				SelectMultiLocations();
				break;

			case IDC_MU_PROV_MULTI_LIST:
				SelectMultiProviders();
				break;

			default:
				//What?  Some strange NxLabel is posting messages to us?
				ASSERT(FALSE);
				break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.dinatale 2012-10-24 14:27) - PLID 53498 - allow for our detailed list to be exported
void CMUDetailedReportDlg::OnBnClickedMuDetPreview()
{
	// Export the detailed/report list to a file
	using namespace NXDATALIST2Lib;
	try{
		if(!m_pReport->GetRowCount()){
			MessageBox("There is currently no data to export. Please run the report before attempting to export.", "Warning", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		CFileDialog dlgSaveFile(FALSE, "csv", "MU_Detailed_Export.csv", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*)|*.*||");
		if (dlgSaveFile.DoModal() == IDCANCEL) {
			return;
		}

		CFile fOutputFile;

		//open the file for reading
		if(!fOutputFile.Open(dlgSaveFile.GetPathName(),CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			MessageBox("The output file could not be opened for writing. Ensure this file is not already open by another program.", "Error", MB_OK | MB_ICONERROR);
			return;
		}

		CString strOutput = "";
		for(int i = 0; i < m_pReport->ColumnCount; i++){
			CString strOut = (LPCSTR) m_pReport->GetColumn(i)->ColumnTitle;

			if (strOut.Find(',') != -1 || strOut.Find("\r") != -1 || strOut.Find("\n") != -1 || strOut.Find('\"') != -1) {
				// We have to put the string in quotes
				strOut = "\"" + ConvertToQuotableString(strOut, true) + "\"";
			}

			strOutput += (strOut + ",");
		}

		strOutput.TrimRight(",");
		strOutput += "\r\n";
		fOutputFile.Write(strOutput, strOutput.GetLength());

		strOutput = (LPCSTR)m_pReport->CalcExportText(efCommaDelimited, FALSE, g_cvarEmpty, g_cvarEmpty);
		fOutputFile.Write(strOutput, strOutput.GetLength());
		fOutputFile.Close();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 12:09) - PLID 53494 - Need to dynamically generate our list columns.
MeasureMap CMUDetailedReportDlg::SetUpLists(std::vector<MU::MeasureData> MeasureDatum)
{
	// set up columns first, construct a map to measure data points
	MeasureMap mapMeasureDataPts;
	short nColIndex = 0;

	// summary list has the title column for rows
	m_pSummary->InsertColumn(nColIndex, _bstr_t(""), _bstr_t(""), 200, csVisible);
	m_pReport->InsertColumn(nColIndex, _bstr_t(""), _bstr_t("Provider"), !m_bShowProvCol ? 0 : 100, !m_bShowProvCol ? csVisible|csFixedWidth : csVisible);

	nColIndex++;

	// for each measure data, we need to create a map of maps
	// first map is measure ID to map of datapoint type info
	// second map is data point type to column index
	foreach(MU::MeasureData muData, MeasureDatum){
		DataTypeMap mapDataPointToColIndex;

		foreach(MU::DataPointInfo dpi, muData.DataPointInfo){
			m_pReport->InsertColumn(nColIndex, _bstr_t(""), _bstr_t(dpi.strName), 100, csVisible);
			m_pSummary->InsertColumn(nColIndex, _bstr_t(""), _bstr_t(dpi.strName), 
				muData.MeasureID == MU::MU_DEM_00 ? 0 : 100, 
				muData.MeasureID == MU::MU_DEM_00 ? csVisible|csFixedWidth : csVisible);

			mapDataPointToColIndex.insert(DataTypeMapValue(dpi.DataType, nColIndex++));
		}

		mapMeasureDataPts.insert(MeasureMapValue(muData.MeasureID, mapDataPointToColIndex));
	}

	return mapMeasureDataPts;
}

// (j.dinatale 2012-10-24 12:09) - PLID 53494 - Need to dynamically generate our lists after data is done loading.
//		We aren't overly concerned with performance right now since this appears to be fast and doesn't cause freezing.
//		If performance becomes an issue, we will have to do something different.
LRESULT CMUDetailedReportDlg::OnAllMeasuresComplete(WPARAM wParam, LPARAM lParam)
{
	try{
		CWaitCursor wc;

		m_pReport->SetRedraw(FALSE);
		m_pSummary->SetRedraw(FALSE);

		long nCurrProgress = (long)wParam;
		long nTotal = (long)lParam;

		CString strText;
		strText.Format("Measures Loaded.", nCurrProgress, nTotal);
		m_nxlProgressLabel.SetWindowText(strText);
		m_nxlProgressLabel.Invalidate();
		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETPOS, nCurrProgress);

		MeasureMap mapMeasureDataPts = SetUpLists(m_muCalculator.m_MeasureData);
		CMap<long, long, IRowSettingsPtr, IRowSettingsPtr> mapPatientToRow;

		// Add our Summary rows
		// Numerator
		IRowSettingsPtr pNumRow = m_pSummary->GetNewRow();
		pNumRow->PutValue(0, "Numerator");
		pNumRow->PutBackColor(NUM_ROW_COLOR);
		m_pSummary->AddRowAtEnd(pNumRow, NULL);

		// Denominator
		IRowSettingsPtr pDenomRow = m_pSummary->GetNewRow();
		pDenomRow->PutValue(0, "Denominator");
		pDenomRow->PutBackColor(DENOM_ROW_COLOR);
		m_pSummary->AddRowAtEnd(pDenomRow, NULL);

		// Percent Met
		IRowSettingsPtr pPerMetRow = m_pSummary->GetNewRow();
		pPerMetRow->PutValue(0, "Percent Met");
		pPerMetRow->PutBackColor(PERCENT_MET_ROW_COLOR);
		m_pSummary->AddRowAtEnd(pPerMetRow, NULL);

		// Required Percent
		IRowSettingsPtr pReqRow = m_pSummary->GetNewRow();
		pReqRow->PutValue(0, "Percent Required");
		pReqRow->PutBackColor(REQUIRED_ROW_COLOR);
		m_pSummary->AddRowAtEnd(pReqRow, NULL);

		// Deficiency
		IRowSettingsPtr pDefRow = m_pSummary->GetNewRow();
		pDefRow->PutValue(0, "Deficiency");
		pDefRow->PutBackColor(DEFICIENT_ROW_COLOR);
		m_pSummary->AddRowAtEnd(pDefRow, NULL);

		foreach(MU::MeasureData muData, m_muCalculator.m_MeasureData){
			// process each completed measure
			if(!mapMeasureDataPts.count(muData.MeasureID)){
				// throw exception here?.. something is def wrong..
				ASSERT(FALSE);
			}

			DataTypeMap mapDataTypeInfo = mapMeasureDataPts.find(muData.MeasureID)->second;

			foreach(MU::PersonMeasureData muPersonData, muData.MeasureInfo){
				// try and obtain this person's row from our map...
				IRowSettingsPtr pRow;
				if(!mapPatientToRow.Lookup(muPersonData.nPersonID, pRow)){
					
					pRow = m_pReport->GetNewRow();
					pRow->PutBackColor(PATIENT_NEW_ROW);

					if(m_bShowProvCol){
						pRow->PutValue(0, _bstr_t(m_strCurrProvName));
						pRow->PutCellBackColor(0, RGB(255, 255, 255));
					}

					m_pReport->AddRowAtEnd(pRow, NULL);
					mapPatientToRow.SetAt(muPersonData.nPersonID, pRow);
				}

				// for each person's data
				foreach(MU::DataPoint muDataPoint, muPersonData.DataPoints){
					if(!mapDataTypeInfo.count(muDataPoint.DataType)){
						// throw exception here?.. something is def wrong..
						ASSERT(FALSE);
					}

					short nColIndex = mapDataTypeInfo.find(muDataPoint.DataType)->second;
					pRow->PutValue(nColIndex, _bstr_t(muDataPoint.strVisibleData));

					// if we are filling info for the DEM 00 measure, we want to have a white backcolor
					if(muData.MeasureID == MU::MU_DEM_00){
						pRow->PutCellBackColor(nColIndex, RGB(255, 255, 255));
					}else{
						//	otherwise, if the patient met the required percent, 
						//	they are green, otherwise they are red
						if(muDataPoint.GetPercentage() < muData.dblRequiredPercent){
							pRow->PutCellBackColor(nColIndex, MEASURE_FAILED_COLOR);
						}else{
							pRow->PutCellBackColor(nColIndex, MEASURE_SUCCESS_COLOR);
						}
					}
				}
			}

			// fill in our summary row columns
			foreach(MU::DataPointInfo dataPtInfo, muData.DataPointInfo){
				if(!mapDataTypeInfo.count(dataPtInfo.DataType)){
					// throw exception here?.. something is def wrong..
					ASSERT(FALSE);
				}

				short nColIndex = mapDataTypeInfo.find(dataPtInfo.DataType)->second;
				pNumRow->PutValue(nColIndex, dataPtInfo.nNumerator);
				pDenomRow->PutValue(nColIndex, dataPtInfo.nDenominator);

				CString strTemp;
				double dblReqPer = muData.dblRequiredPercent;
				double dblCurrPer = dataPtInfo.GetPercentage();

				strTemp.Format("%0.2f%%", dblReqPer);
				pReqRow->PutValue(nColIndex, _bstr_t(strTemp));

				strTemp.Format("%0.2f%%", dblCurrPer);
				pPerMetRow->PutValue(nColIndex, _bstr_t(strTemp));

				strTemp.Format("%0.2f%%", (dblCurrPer - dblReqPer));
				pDefRow->PutValue(nColIndex, _bstr_t(strTemp));
				if(dblCurrPer < dblReqPer){
					pDefRow->PutCellForeColor(nColIndex, DEFICIENT_CELL_FORE_COLOR);
				}
			}
		}

		m_pReport->SetRedraw(TRUE);
		m_pSummary->SetRedraw(TRUE);

		EnableMUFilter(TRUE);
		EnableSeenFilter(TRUE);
		EnableUtilityButtons(TRUE);

		m_btnLoad.SetWindowText("Load");
		m_btnLoad.AutoSet(NXB_REFRESH);

		// (r.farnworth 2014-04-24 11:50) - PLID 59011 - Re-enable the radio buttons.
		GetDlgItem(IDC_RADIO_DSTAGE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_DSTAGE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_MOD_DSTAGE2)->EnableWindow(TRUE);

		ResetMUCalculator();
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.dinatale 2012-10-24 14:22) - PLID 53508 - handle measure preloading and updating the progress bar accordingly
LRESULT CMUDetailedReportDlg::OnMeasurePreload(WPARAM wParam, LPARAM lParam)
{
	try{
		long nTotal = (long)lParam;
		long nCurrProgress = (long)wParam;

		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETRANGE, 0, MAKELPARAM(0, nTotal));

		CString strText;
		strText.Format("Calculating Measures...(%d / %d)", nCurrProgress, nTotal);

		m_nxlProgressLabel.SetWindowText(strText);
		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETPOS, nCurrProgress);
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.dinatale 2012-10-24 14:22) - PLID 53508 - update progress when measures are complete
LRESULT CMUDetailedReportDlg::OnMeasureComplete(WPARAM wParam, LPARAM lParam)
{
	try{
		long nCurrProgress = (long)wParam;
		long nTotal = (long)lParam;

		CString strText;
		strText.Format("Calculating Measures...(%d / %d)", nCurrProgress, nTotal);

		m_nxlProgressLabel.SetWindowText(strText);
		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETPOS, nCurrProgress);
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.dinatale 2012-10-24 14:22) - PLID 53508 - Handle when the calculator is cancelled
LRESULT CMUDetailedReportDlg::OnMeasureLoadCancel(WPARAM wParam, LPARAM lParam)
{
	try{
		ResetMUCalculator();

		m_nxlProgressLabel.SetWindowText("Load Cancelled.");
		GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETRANGE, 0, 0);

		EnableMUFilter(TRUE);
		EnableSeenFilter(TRUE);
		EnableUtilityButtons(TRUE);

		// (r.farnworth 2014-06-19 11:50) - PLID 59011 - Re-enable the radio buttons.
		GetDlgItem(IDC_RADIO_DSTAGE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_DSTAGE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_MOD_DSTAGE2)->EnableWindow(TRUE);

		m_btnLoad.SetWindowText("Load");
		m_btnLoad.AutoSet(NXB_REFRESH);
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Add multiple and all selection options for MU providers
void CMUDetailedReportDlg::RequeryFinishedMuDtlProviderList(short nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProviders->GetNewRow();
		pRow->PutValue(0, (long) -2);
		pRow->PutValue(2, _variant_t("<Multiple Providers>"));
		m_pProviders->AddRowBefore(pRow, m_pProviders->GetFirstRow());

		pRow = m_pProviders->GetNewRow();
		pRow->PutValue(0, (long) -1);
		pRow->PutValue(2, _variant_t("<All Providers>"));
		m_pProviders->AddRowBefore(pRow, m_pProviders->GetFirstRow());

		m_pProviders->CurSel = pRow;
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::RequeryFinishedMuDtlSeenProviderList(short nFlags)
{
	try{
		m_pSeenProviders->CurSel = m_pSeenProviders->GetFirstRow();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Add multiple and all selection options for patient seen locations
void CMUDetailedReportDlg::RequeryFinishedMuDtlLocationList(short nFlags)
{
	try {	
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pSeenLocationList->GetNewRow();
		pRow->PutValue(llcID, (long) -2);
		pRow->PutValue(llcName, _variant_t("<Multiple Locations>"));
		m_pSeenLocationList->AddRowBefore(pRow, m_pSeenLocationList->GetFirstRow());

		pRow = m_pSeenLocationList->GetNewRow();
		pRow->PutValue(llcID, (long) -1);
		pRow->PutValue(llcName, _variant_t("<All Locations>"));
		m_pSeenLocationList->AddRowBefore(pRow, m_pSeenLocationList->GetFirstRow());

		m_pSeenLocationList->CurSel = pRow;	
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Add multiple and all selection options for MU locations
void CMUDetailedReportDlg::RequeryFinishedMuLocationList(short nFlags)
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

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - we can handle disabling the seen filter, but we currently dont allow this
void CMUDetailedReportDlg::OnBnClickedMuDtlDisableSeenFilter()
{
	try{
		BOOL bEnable = !IsDlgButtonChecked(IDC_MU_DTL_DISABLE_SEEN_FILTER);
		EnableSeenFilter(bEnable);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-24 12:34) - PLID 53497 - need to be able to modify the exclusions from our new detailed report
void CMUDetailedReportDlg::OnBnClickedMuDtlExclusions()
{
	try {
		// we are going to multiselect from EMR Templates
		CString strFrom, strWhere;
		CMultiSelectDlg dlg(this, "EMRTemplateT");
		HRESULT hRes;
		
		// get the current exclusion set
		CString strExcludedIDs = GetRemotePropertyMemo("MU_ExcludedTemplates", 0, 0, "<None>");
		strExcludedIDs.TrimLeft("(");
		strExcludedIDs.TrimRight(")");

		// parse out the IDs
		CDWordArray dwExcludedTemplateIDs;
		GetIDsFromCommaDelimitedString(&dwExcludedTemplateIDs, strExcludedIDs);

		// Fill the dialog with existing selections			
		dlg.PreSelect(dwExcludedTemplateIDs);
		dlg.m_strNameColTitle = "Name";
		
		strFrom = "EMRTemplateT";
		strWhere = "Deleted = 0 ";
				
		hRes = dlg.Open(strFrom, strWhere, "EMRTemplateT.ID", "EMRTemplateT.Name", "Please select the EMR templates to exclude from the measures. These should be templates where the doctor did not actually see or consult with the patient, for example NexWeb templates or phone messages. ");		
		
		// Update the excluded templates configRT entry and color our button accordingly
		if (hRes == IDOK) {
			CString strExclusionsList = dlg.GetMultiSelectIDString(",");
			if (strExclusionsList.IsEmpty()) {
				m_btnExclusions.SetTextColor(RGB(0,0,0));
				SetRemotePropertyMemo("MU_ExcludedTemplates", "", 0, "<None>");
			} else {
				m_btnExclusions.SetTextColor(RGB(255,0,0));
				SetRemotePropertyMemo("MU_ExcludedTemplates", ("(" + strExclusionsList + ")"), 0, "<None>");
			}			
		}		
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::OnBnClickedOk()
{
	try{
		// (j.dinatale 2012-10-24 14:28) - PLID 53508 - halt our calculator
		m_muCalculator.Interrupt();
		m_muCalculator.Join();

		ResetMUCalculator();

		ShowWindow(SW_HIDE);
		m_bDialogVisible = false;
	}NxCatchAll(__FUNCTION__);

	//CNxDialog::OnOK();
}

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - clear our lists when the dialog is shown
void CMUDetailedReportDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try{
		if(bShow){
			if(!m_bDialogVisible){
				GetDlgItem(IDC_MU_DETAIL_PROGRESS)->PostMessage(PBM_SETRANGE, 0, 0);
				m_nxlProgressLabel.SetWindowText("Waiting...");

				// (r.farnworth 2013-12-19 13:12) - PLID 59011 - This feature is intended for 11100 so we should disable this option for now.
				// (r.farnworth 2014-04-24 11:10) - PLID 59011 - Undershot that release by a bit, but we can finally enable these radio buttons.
				// Moved here to ensure these were always enabled upon opening the window
				GetDlgItem(IDC_RADIO_DSTAGE1)->EnableWindow(TRUE);
				GetDlgItem(IDC_RADIO_DSTAGE2)->EnableWindow(TRUE);
				GetDlgItem(IDC_RADIO_MOD_DSTAGE2)->EnableWindow(TRUE);

				if(m_pSummary){
					m_pSummary->Clear();

					long nColCount = m_pSummary->GetColumnCount();
					for(int i=0; i < nColCount; i++){
						m_pSummary->RemoveColumn(0);
					}
				}

				if(m_pReport){
					m_pReport->Clear();

					long nColCount = m_pReport->GetColumnCount();
					for(int i=0; i < nColCount; i++){
						m_pReport->RemoveColumn(0);
					}
				}		
			}

			m_bDialogVisible = true;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-04-24 12:22) - PLID 59011 - Clear the search results whenever a radio button is selected.
void CMUDetailedReportDlg::OnBnClickedRadioDstage1()
{
	try {
		HandleStageChange(MU::Stage1);
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-04-24 12:22) - PLID 59011 - Clear the search results whenever a radio button is selected.
void CMUDetailedReportDlg::OnBnClickedRadioDstage2()
{
	try {
		HandleStageChange(MU::Stage2);
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::OnBnClickedRadioModDstage2()
{
	try {
		HandleStageChange(MU::ModStage2);
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::HandleStageChange(MU::Stage eStage)
{
	// only set the meaningful use stage if it's actually changed
	if (m_muCalculator.m_eMeaningfulUseStage != eStage)
	{
		m_muCalculator.m_eMeaningfulUseStage = eStage;
		ClearMeasuresDatalists();
	}
}

void CMUDetailedReportDlg::ClearMeasuresDatalists()
{
	m_pReport->Clear();
	m_pSummary->Clear();
}

// (v.maida - 2014-04-28 09:20) - PLID 61902 - Add "Go to Patient" option for main reports datalist.
void CMUDetailedReportDlg::OnRButtonDownReportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}
		m_pReport->PutCurSel(pRow);
		CMenu mnu;
		if (mnu.CreatePopupMenu())
		{
			enum MenuItems
			{
				miGotoPatient = 1,
			};
		
			mnu.AppendMenu(MF_ENABLED, miGotoPatient, "Go to Patient");
			CPoint pt;
			GetCursorPos(&pt);

			int nChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
			switch(nChoice)
			{
				case miGotoPatient:
					{
							CString strPatientID = VarString(pRow->GetValue(1), ""); // 1 = Patient ID column
								if (strPatientID != "" && strPatientID != "-1") {
									// get internal patient ID
									long nPatientID = GetExistingPatientIDByUserDefinedID(atoi(strPatientID));
									// switch over to the patients module
									CMainFrame *pMainFrame;
									pMainFrame = GetMainFrame();
									if (pMainFrame != NULL) {
										pMainFrame->GotoPatient(nPatientID);
										// minimize detailed reporting dialog before switching over
										ShowWindow(SW_MINIMIZE);
									}
								}
						
						break;
					}
				default:
					break;
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (v.maida - 2014-04-28 11:20) - PLID 61904 - Move right-hand filter settings over to the left.
void CMUDetailedReportDlg::OnBnClickedMoveLeft()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr locRow = m_pSeenLocationList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr provRow = m_pSeenProviders->GetCurSel();
		// handle dates
		m_MUFrom.SetValue(m_SeenFrom.GetValue());
		m_MUTo.SetValue(m_SeenTo.GetValue());
		// handle locations
		//Check if multiple locations are selected on the right-hand side
		if (m_dwSeenLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_HIDE);
			m_nxlLocationLabel.SetText(GetNamesFromIDString(m_strSeenLocationList, m_pSeenLocationList, llcID, llcName));
			m_nxlLocationLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_SHOW);
			m_nxlLocationLabel.Invalidate();
			m_nCurrentLocationID = -2;
			m_dwLocIDList.RemoveAll();
			m_dwLocIDList.Copy(m_dwSeenLocIDList);
			m_strLocationList = m_strSeenLocationList;
		}
		else { // there is one location (or "<All Providers>") on the right, to be moved to the left
			if (m_dwLocIDList.GetSize() > 1) {
				// left-hand side has multiple locations selected, so change it to one
				ShowDlgItem(IDC_MU_DTL_LOCATION_LIST, SW_SHOW);
				ShowDlgItem(IDC_MU_MULTI_LOC_LIST, SW_HIDE);
				m_nxlLocationLabel.Invalidate();
				m_pLocationList->SetSelByColumn(llcID, locRow->GetValue(llcID));
				m_nCurrentLocationID = m_nCurrentSeenLocationID;
				m_dwLocIDList.RemoveAll();
				if (VarLong(locRow->GetValue(llcID)) != -1) { // check for "<All Providers>" option, because nothing will be added to the id array if that was chosen
					m_dwLocIDList.Add((DWORD)m_nCurrentSeenLocationID); // add the single selection as the sole ID array entry
				}
				m_strLocationList = m_strSeenLocationList;
			}
			else { // left-hand side has one location
				m_pLocationList->SetSelByColumn(llcID, locRow->GetValue(llcID));
				m_nCurrentLocationID = m_nCurrentSeenLocationID;
				m_dwLocIDList.RemoveAll();
				if (VarLong(locRow->GetValue(llcID)) != -1) { // check for "<All Providers>" option, because nothing will be added to the id array if that was chosen
					m_dwLocIDList.Add((DWORD)m_nCurrentSeenLocationID); 
				}
				m_strLocationList = m_strSeenLocationList;
			}
		}
		// handle providers
		if (m_dwProvIDList.GetSize() > 1) { 
			// left-hand side has multiple providers selected
			ShowDlgItem(IDC_MU_DTL_PROVIDER_LIST, SW_SHOW);
			ShowDlgItem(IDC_MU_PROV_MULTI_LIST, SW_HIDE);
			m_pProviders->SetSelByColumn(MUDtlProvList::ID, provRow->GetValue(MUDtlProvList::ID));
			m_nCurrentProvID = VarLong(provRow->GetValue(MUDtlProvList::ID));
			m_dwProvIDList.RemoveAll();
			m_dwProvIDList.Add((DWORD)m_nCurrentProvID);
			m_strProviderList = "("+FormatString("%li", VarLong(provRow->GetValue(MUDtlProvList::ID)))+")";
		}
		else { // only one provider on the left-hand side
			m_pProviders->SetSelByColumn(MUDtlProvList::ID, provRow->GetValue(MUDtlProvList::ID));
			m_nCurrentProvID = VarLong(provRow->GetValue(MUDtlProvList::ID));
			m_dwProvIDList.RemoveAll();
			m_dwProvIDList.Add((DWORD)m_nCurrentProvID);
			m_strProviderList = "(" + FormatString("%li", VarLong(provRow->GetValue(MUDtlProvList::ID))) + ")";
		}
		
		if (IsDlgButtonChecked(IDC_MU_DTL_SEEN_EXCLUDE_SEC)) {
			CheckDlgButton(IDC_MU_DTL_EXCLUDE_SEC, BST_CHECKED);
		}
		else {
			CheckDlgButton(IDC_MU_DTL_EXCLUDE_SEC, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

// (v.maida - 2014-04-28 11:21) - PLID 61904 - Move left-hand filter settings over to the right.
void CMUDetailedReportDlg::OnBnClickedMoveRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr locRow = m_pLocationList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr provRow = m_pProviders->GetCurSel();
		// handle dates
		m_SeenFrom.SetValue(m_MUFrom.GetValue());
		m_SeenTo.SetValue(m_MUTo.GetValue());
		CString provName = VarString(provRow->GetValue(MUDtlProvList::Name), "");
		// handle providers
		if (provName != "") {
			if (provName == "<All Providers>") { // just show the first provider, since there is no multiple providers option for patients seen.
				provRow = m_pSeenProviders->GetFirstRow();
				m_pSeenProviders->SetSelByColumn(MUDtlProvList::ID, provRow->GetValue(MUDtlProvList::ID));
			}
			else if (provName == "<Multiple Providers>") { // just show the (alphabetic) first provider out of the list of multiple providers, since there is no multiple providers option for patients seen.
				// vector to hold provider IDs, and the corresponding name for each
				std::vector<std::pair<long, CString>> IDsAndNames = std::vector<std::pair<long, CString>>();
				// GetNamesFromIDString() will fill up the vector of pairs with id,name pairs
				GetNamesFromIDString(m_strProviderList, m_pProviders, MUDtlProvList::ID, MUDtlProvList::Name, IDsAndNames);
				// the returned vector will be sorted alphabetically, so just set the datalist to the id corresponding to the first element in the vector
				m_pSeenProviders->SetSelByColumn(MUDtlProvList::ID, _variant_t((long)IDsAndNames[0].first, VT_I4));
			}
			else { // a regular, single provider name has been chosen
				m_pSeenProviders->SetSelByColumn(MUDtlProvList::ID, provRow->GetValue(MUDtlProvList::ID));
			}
		}
		// handle locations
		// check if multiple locations were selected on the left-hand side
		if (m_dwLocIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_HIDE);
			m_nxlLocationSeenLabel.SetText(GetNamesFromIDString(m_strLocationList, m_pLocationList, llcID, llcName));
			m_nxlLocationSeenLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_SHOW);
			m_nxlLocationSeenLabel.Invalidate();
			m_nCurrentSeenLocationID = -2;
			m_dwSeenLocIDList.RemoveAll();
			m_dwSeenLocIDList.Copy(m_dwLocIDList);
			m_strSeenLocationList = m_strLocationList;
		}
		else { // one location (or "<All Providers>") on the left, to be moved to the right
			if (m_dwSeenLocIDList.GetSize() > 1) { // right-hand side has multiple locations selected, so change it to one
				ShowDlgItem(IDC_MU_DTL_SEEN_LOCATION_LIST, SW_SHOW);
				ShowDlgItem(IDC_MU_MULTI_LOC_SEEN_LIST, SW_HIDE);
				m_nxlLocationSeenLabel.Invalidate();
				m_pSeenLocationList->SetSelByColumn(llcID, locRow->GetValue(llcID));
				m_nCurrentSeenLocationID = m_nCurrentLocationID;
				m_dwSeenLocIDList.RemoveAll();
				if (VarLong(locRow->GetValue(llcID)) != -1) { // check for "<All Providers>" option, because nothing will be added to the id array if that was chosen
					m_dwSeenLocIDList.Add((DWORD)m_nCurrentLocationID); // add the single selection as the sole ID array entry
				}
				m_strSeenLocationList = m_strLocationList;
			}
			else { // right-hand side has one location
				m_pSeenLocationList->SetSelByColumn(llcID, locRow->GetValue(llcID));
				m_nCurrentSeenLocationID = m_nCurrentLocationID;
				m_dwSeenLocIDList.RemoveAll();
				if (VarLong(locRow->GetValue(llcID)) != -1) { // check for "<All Providers>" option, because nothing will be added to the id array if that was chosen
					m_dwSeenLocIDList.Add((DWORD)m_nCurrentLocationID); 
				}
				m_strSeenLocationList = m_strLocationList;
			}
		}
		if (IsDlgButtonChecked(IDC_MU_DTL_EXCLUDE_SEC)) {
			CheckDlgButton(IDC_MU_DTL_SEEN_EXCLUDE_SEC, BST_CHECKED);
		}
		else {
			CheckDlgButton(IDC_MU_DTL_SEEN_EXCLUDE_SEC, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMUDetailedReportDlg::ResetMUCalculator()
{
	// get rid of our current calculator, and give the new one the currently selected stage
	m_muCalculator = MU::CCalculator();
	if (IsDlgButtonChecked(IDC_RADIO_DSTAGE1)) {
		m_muCalculator.m_eMeaningfulUseStage = MU::Stage1;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_DSTAGE2)) {
		m_muCalculator.m_eMeaningfulUseStage = MU::Stage2;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_MOD_DSTAGE2)) {
		m_muCalculator.m_eMeaningfulUseStage = MU::ModStage2;
	}
}