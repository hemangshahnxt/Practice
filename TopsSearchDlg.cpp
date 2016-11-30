// TopsSearch.cpp : implementation file
//

#include "stdafx.h"
#include "TopsSearchDlg.h"
#include "FinancialRc.h"
#include "SOAPUtils.h"
#include "FileUtils.h"
#include "CCDUtils.h"
#include "MultiSelectDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "TopsFieldPickDlg.h"
#include "ImportAMACodesDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;
// CTopsSearch dialog
//(e.lally 2009-10-07) PLID 35803 - Created

IMPLEMENT_DYNAMIC(CTopsSearchDlg, CNxDialog)


enum TopsSearchListColumns {
	tslcPatientID=0,
	tslcUserDefinedID,
	tslcPatientName,
	tslcProcedureName,
	tslcProcedureDate,
	tslcBillID,
	tslcApptID, //(e.lally 2009-11-24) PLID 36415
	tslcLastSentDate,	// (j.jones 2009-11-03 15:37) - PLID 36137 - added last sent date and color columns
	tslcColor,
};

enum TopsProviderColumns {
	provColID =0,
	provColName,
};

enum TopsProcedureColumns {
	procColID =0,
	procColName,
};

// (f.dinatale 2010-09-08) - PLID 38192 - Location filter enum
enum TopsLocationColumns {
	locaColID =0,
	locaColName,
};

const long cnAllValues = -1;
const long cnMultiSelections = -2;
const long cnSubmittedCaseColor = (long)RGB(127,127,127); //(e.lally 2009-11-24) PLID 36415
const long cnMaxSelRecordBrowsers = 20; //(e.lally 2009-11-25) PLID 35803 - limit the number of web browsers we open at once.

//(e.lally 2010-09-15) PLID 40532 - Added table checkers
CTopsSearchDlg::CTopsSearchDlg(CWnd* pParent)
	: CNxDialog(CTopsSearchDlg::IDD, pParent)
	, m_doctorChecker(NetUtils::Providers)
	, m_resourceChecker(NetUtils::Resources)
	, m_locationChecker(NetUtils::LocationsT)
{
	//Not yet supported
	//m_pBrowserDlg = NULL;
}

CTopsSearchDlg::~CTopsSearchDlg()
{

}


void CTopsSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TOPS_SEARCH_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_TOPS_SEARCH_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_TOPS_REFRESH_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_TOPS_SEND, m_btnSend);
	DDX_Control(pDX, IDC_TOPS_SHOW_SUBMITTED, m_radioShowSubmitted);
	DDX_Control(pDX, IDC_TOPS_SHOW_UNSUBMITTED, m_radioShowUnsubmitted);
	DDX_Control(pDX, IDC_TOPS_SHOW_ALL, m_radioShowAll);
	DDX_Control(pDX, IDC_TOPS_ONLY_MASTER_PROCEDURES, m_checkShowMasterProcs);
	DDX_Control(pDX, IDC_TOPS_SEARCH_COUNT, m_nxstaticSearchResults);
	DDX_Control(pDX, IDC_LABEL_TOPS_MULTI_PROV, m_nxlMultiProviders);
	DDX_Control(pDX, IDC_LABEL_TOPS_MULTI_PROC, m_nxlMultiProcedures);
	// (f.dinatale 2010-09-09) - PLID 38192
	DDX_Control(pDX, IDC_LABEL_TOPS_MULTI_LOC, m_nxlMultiLocations);
	DDX_Control(pDX, IDC_TOPS_USERNAME, m_nxeditUsername);
	DDX_Control(pDX, IDC_TOPS_PASSWORD, m_nxeditPassword);
	DDX_Control(pDX, IDC_TOPS_PROXIED_NAME, m_nxeditProxiedUser);
	DDX_Control(pDX, IDC_RADIO_TOPS_BILLING_SEARCH, m_radioBillingSearch);
	DDX_Control(pDX, IDC_RADIO_TOPS_APPT_SEARCH, m_radioAppointmentSearch);
	DDX_Control(pDX, IDC_TOPS_OPTIONS, m_btnTopsOptions);
}


BEGIN_MESSAGE_MAP(CTopsSearchDlg, CNxDialog)
	ON_BN_CLICKED(IDC_TOPS_REFRESH_SEARCH, OnBnClickedRefreshSearch)
	ON_BN_CLICKED(IDC_TOPS_SEND, OnBnClickedSendToTops)
	ON_BN_CLICKED(IDC_TOPS_ONLY_MASTER_PROCEDURES, OnBnClickedTopsOnlyMasterProcedures)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_TOPS_SHOW_UNSUBMITTED, OnShowUnsubmitted)
	ON_BN_CLICKED(IDC_TOPS_SHOW_SUBMITTED, OnShowSubmitted)
	ON_BN_CLICKED(IDC_TOPS_SHOW_ALL, OnShowAll)
	ON_EN_KILLFOCUS(IDC_TOPS_USERNAME, &CTopsSearchDlg::OnEnKillfocusTopsUsername)
	ON_EN_KILLFOCUS(IDC_TOPS_PROXIED_NAME, &CTopsSearchDlg::OnEnKillfocusTopsProxiedName)
	ON_BN_CLICKED(IDC_RADIO_TOPS_BILLING_SEARCH, OnClickedBillingSearch)
	ON_BN_CLICKED(IDC_RADIO_TOPS_APPT_SEARCH, OnClickedApptSearch)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TOPS_SEARCH_DATE_FROM, OnChangeSearchFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TOPS_SEARCH_DATE_TO, OnChangeSearchToDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_TOPS_SEARCH_DATE_FROM, OnDtnDropdownSearchFromDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_TOPS_SEARCH_DATE_TO, OnDtnDropdownSearchToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_TOPS_SEARCH_DATE_FROM, OnDtnCloseupSearchFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_TOPS_SEARCH_DATE_TO, OnDtnCloseupSearchToDate)
	ON_BN_CLICKED(IDC_TOPS_OPTIONS, OnBnClickedOptions)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CTopsSearchDlg, CNxDialog)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_SEARCH_LIST, 18, OnRequeryFinishedSearchList, VTS_I2)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PROVIDER_LIST, 18, OnRequeryFinishedProviderFilter, VTS_I2)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PROCEDURE_LIST, 18, OnRequeryFinishedProcedureFilter, VTS_I2)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PROVIDER_LIST, 16, OnSelChosenProviderFilter, VTS_DISPATCH)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PROCEDURE_LIST, 16, OnSelChosenProcedureFilter, VTS_DISPATCH)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PLACE_OF_SERVICE_LIST, 18, CTopsSearchDlg::OnRequeryFinishedLocationList, VTS_I2)
	ON_EVENT(CTopsSearchDlg, IDC_TOPS_PLACE_OF_SERVICE_LIST, 16, CTopsSearchDlg::OnSelChosenLocationFilter, VTS_DISPATCH)
END_EVENTSINK_MAP()


// CTopsSearch message handlers

BOOL CTopsSearchDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		//Set the icons on the NxIconButtons
		m_btnSend.AutoSet(NXB_EXPORT);
		m_btnSearch.AutoSet(NXB_INSPECT);
		//(e.lally 2010-02-16) PLID 36849
		m_btnTopsOptions.AutoSet(NXB_MODIFY);

		m_bNotifyOnce = TRUE;
		m_bDateFromDown = FALSE;
		m_bDateToDown = FALSE;

		// (j.jones 2009-11-03 15:43) - PLID 36137 - default the submission filter to unsent records
		m_radioShowUnsubmitted.SetCheck(TRUE);

		//(e.lally 2009-10-20) PLID 36001 - Ensure we are showing the proper controls.
		m_nxlMultiProviders.SetText("");
		m_nxlMultiProviders.SetType(dtsHyperlink);
		m_nxlMultiProcedures.SetText("");
		m_nxlMultiProcedures.SetType(dtsHyperlink);
		// (f.dinatale 2010-09-09) - PLID 38192
		m_nxlMultiLocations.SetText("");
		m_nxlMultiLocations.SetType(dtsHyperlink);
		EnsureControls();

		//Preferences
		g_propManager.CachePropertiesInBulk("TOPSSearch-1", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'TOPS_FormUrl' "
			"OR Name = 'TOPSUsername' "			// (d.thompson 2009-11-04) - PLID 36136
			"OR Name = 'TOPSProxiedUser' "		// (d.thompson 2009-11-06) - PLID 36136
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("TOPSSearch-2", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'TOPS_SubmitMethod' "
			"OR Name = 'TOPSLastSearchType' "
			"OR Name = 'ReportFilters_IncludeInactiveProviders' " //(e.lally 2010-09-15) PLID 40532
			"OR Name = 'ReportFilters_IncludeInactiveLocations' " //(e.lally 2010-09-15) PLID 40532
			")",
			_Q(GetCurrentUserName()));

		//(e.lally 2009-11-24) PLID 36415
		if(GetRemotePropertyInt("TOPSLastSearchType", 0, 0, GetCurrentUserName(), true) == 0){
			//default
			m_radioBillingSearch.SetCheck(TRUE);
		}
		else{
			m_radioAppointmentSearch.SetCheck(TRUE);
		}

		//(e.lally 2010-09-15) PLID 40532
		m_bFilterShowInactiveProviders = GetRemotePropertyInt("ReportFilters_IncludeInactiveProviders", 0, 0, "<None>", true) == 1 ? TRUE : FALSE;
		m_bFilterShowInactiveLocations = GetRemotePropertyInt("ReportFilters_IncludeInactiveLocations", 0, 0, "<None>", true) == 1 ? TRUE : FALSE;

		//Set up the datalists
		m_pTopsSearchList = BindNxDataList2Ctrl(IDC_TOPS_SEARCH_LIST, false);
		m_pProcedureFilter = BindNxDataList2Ctrl(IDC_TOPS_PROCEDURE_LIST, false);
		m_checkShowMasterProcs.SetCheck(TRUE);
		SetProcedureFilterWhere(TRUE);
		m_pProcedureFilter->Requery();
		m_pProviderFilter = BindNxDataList2Ctrl(IDC_TOPS_PROVIDER_LIST, false);
		//(e.lally 2009-11-24) PLID 36415
		RequeryProviderList();

		// (f.dinatale 2010-09-08) - PLID 38192
		m_pLocationFilter = BindNxDataList2Ctrl(IDC_TOPS_PLACE_OF_SERVICE_LIST, false);
		RequeryLocationList();
		/*
		//Not yet supported
		m_pBrowserDlg = new CGenericBrowserDlg(this);
		m_pBrowserDlg->Create(IDD_GENERIC_BROWSER_DLG, this);	
		*/


		// (d.thompson 2009-11-04) - PLID 36136 - Remember the username last used
		m_nxeditUsername.SetLimitText(255);		//Max length of ConfigRT.TextParam
		m_nxeditProxiedUser.SetLimitText(255);	//Max length of ConfigRT.TextParam
		SetDlgItemText(IDC_TOPS_USERNAME, GetRemotePropertyText("TOPSUsername", "", 0, GetCurrentUserName(), true));
		SetDlgItemText(IDC_TOPS_PROXIED_NAME, GetRemotePropertyText("TOPSProxiedUser", "", 0, GetCurrentUserName(), true));

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//(e.lally 2009-10-20) PLID 36001 - Added. Make sure the proper controls are showing (dropdown or hyperlink label e.g.)
void CTopsSearchDlg::EnsureControls()
{
	bool bIsMultiProvider = false;
	if(m_arProviderFilterIDList.GetSize() > 1){
		bIsMultiProvider = true;
	}
	//(e.lally 2009-12-03) PLID 36001 - Use SW_SHOWNA here so we don't automatically activate the control, putting the focus there.
	if(bIsMultiProvider){
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_PROV, SW_SHOWNA);
		ShowDlgItem(IDC_TOPS_PROVIDER_LIST, SW_HIDE);
	}
	else {
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_PROV, SW_HIDE);
		ShowDlgItem(IDC_TOPS_PROVIDER_LIST, SW_SHOWNA);
	}
	//(e.lally 2009-12-03) PLID 36001 - Use the new function for a cleaner redraw
	m_nxlMultiProviders.AskParentToRedrawWindow();
	
	bool bIsMultiProcedure = false;
	if(m_arProcFilterIDList.GetSize() > 1){
		bIsMultiProcedure = true;
	}
	//(e.lally 2009-12-03) PLID 36001 - Use SW_SHOWNA here so we don't automatically activate the control, putting the focus there.
	if(bIsMultiProcedure){
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_PROC, SW_SHOWNA);
		ShowDlgItem(IDC_TOPS_PROCEDURE_LIST, SW_HIDE);
	}
	else {
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_PROC, SW_HIDE);
		ShowDlgItem(IDC_TOPS_PROCEDURE_LIST, SW_SHOWNA);
	}
	//(e.lally 2009-12-03) PLID 36001 - Use the new function for a cleaner redraw
	m_nxlMultiProcedures.AskParentToRedrawWindow();

	// (f.dinatale 2010-09-09) - PLID 38192 - Added a location filter which also has the ability to select multiple
	bool bIsMultiLocation = false;
	if(m_arLocFilterIDList.GetSize() > 1){
		bIsMultiLocation = true;
	}

	if(bIsMultiLocation){
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_LOC, SW_SHOWNA);
		ShowDlgItem(IDC_TOPS_PLACE_OF_SERVICE_LIST, SW_HIDE);
	}
	else {
		ShowDlgItem(IDC_LABEL_TOPS_MULTI_LOC, SW_HIDE);
		ShowDlgItem(IDC_TOPS_PLACE_OF_SERVICE_LIST, SW_SHOWNA);
	}

	m_nxlMultiLocations.AskParentToRedrawWindow();
}

//(e.lally 2009-11-25) PLID 35803 - clears the search results
void CTopsSearchDlg::ClearResults()
{
	m_pTopsSearchList->Clear();
	//Update the row count
	SetDlgItemText(IDC_TOPS_SEARCH_COUNT, "Count: 0");
}

//(e.lally 2009-11-25) PLID 36415 - Build and requery the provider/resource list.
void CTopsSearchDlg::RequeryProviderList()
{
	CString strProvFrom;
	if(m_radioAppointmentSearch.GetCheck()){
		//(e.lally 2010-09-15) PLID 40532 - Use the report preference to filter out inactives from the dropdown display
		strProvFrom = "(SELECT ResourceT.Item AS Name, ResourceT.ID "
			"FROM ResourceT ";
		if(!m_bFilterShowInactiveProviders){
			strProvFrom += "WHERE ResourceT.Inactive = 0 ";
		}
		strProvFrom += ") SubQ ";
	}
	else{
		//(e.lally 2010-09-15) PLID 40532 - Use the report preference to filter out inactives from the dropdown display
		strProvFrom = "(SELECT Last + ', ' + First + ' ' + Middle AS Name, "
			"ProvidersT.PersonID as ID "
			"FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID ";
		if(!m_bFilterShowInactiveProviders){
			strProvFrom += "WHERE PersonT.Archived = 0 ";
		}
		strProvFrom += ") SubQ ";
	}
	m_pProviderFilter->FromClause = _bstr_t(strProvFrom);
	m_pProviderFilter->Requery();
}

void CTopsSearchDlg::OnBnClickedRefreshSearch()
{
	try {
		CString strFrom;
		CString strProcedureFilter, strProviderFilter, strDateFilter, strSubmissionFilter;
		// (f.dinatale 2010-09-09) - PLID 38192
		CString strLocationFiler;

		//(e.lally 2009-11-24) PLID 36415 - get the procedure filter (shared)
		IRowSettingsPtr pProcSel = m_pProcedureFilter->GetCurSel();
		if(pProcSel != NULL){
			long nProcID = VarLong(pProcSel->GetValue(procColID), cnAllValues);
			if(nProcID != cnAllValues){
				CString strIDList, strID;
				//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
				for(int i=0; i < m_arProcFilterIDList.GetSize(); i++) {
					strID.Format("%li,", m_arProcFilterIDList.GetAt(i));
					strIDList += strID;
				}
				strIDList.TrimRight(",");
				//Depending on the filter checkbox, filter using details and master or just details
				if(m_checkShowMasterProcs.GetCheck()){
					strProcedureFilter.Format(" AND (ProcedureT.ID IN(%s) "
						"OR ProcedureT.MasterProcedureID IN(%s) ) ", strIDList, strIDList);
				}
				else{
					strProcedureFilter.Format(" AND ProcedureT.ID IN(%s) ", strIDList);
				}
			}
		}

		// (f.dinatale 2010-09-09) - PLID 38192 - generate that Location filter
		IRowSettingsPtr pLocSel = m_pLocationFilter->GetCurSel();
		if(pLocSel != NULL){
			long nLocID = VarLong(pLocSel->GetValue(locaColID), cnAllValues);
			if(nLocID != cnAllValues){
				CString strLocList, strLoc;

				for(int i = 0; i < m_arLocFilterIDList.GetSize(); i++){
					strLoc.Format("%li,", m_arLocFilterIDList.GetAt(i));
					strLocList += strLoc;
				}
				strLocList.TrimRight(",");
				
				if(m_radioBillingSearch.GetCheck()){
					strLocationFiler.Format(" AND BillsT.Location IN(%s) ", strLocList);
				} else {
					if(m_radioAppointmentSearch.GetCheck()){
						strLocationFiler.Format(" AND AppointmentsT.LocationID IN(%s) ", strLocList);
					}
				}
			}
		}

		// (j.jones 2009-11-03 15:23) - PLID 36137 - include the submission status filter
		//(e.lally 2009-11-24) PLID 36415 - get the submission status filter (shared)
		if(m_radioShowSubmitted.GetCheck()) {
			strSubmissionFilter.Format(" AND TOPSSubmissionHistoryQ.LastSentDate Is Not Null");
		}
		else if(m_radioShowUnsubmitted.GetCheck()) {
			strSubmissionFilter.Format(" AND TOPSSubmissionHistoryQ.LastSentDate Is Null");
		}
		//(e.lally 2009-11-24) PLID 36415 - get the search specific filters and from clause.
		if(!m_radioAppointmentSearch.GetCheck()){
			//(e.lally 2009-10-08) PLID 35803 - Generate the appropriate filters
			strDateFilter.Format(" AND (LineItemT.Date >= '%s' AND LineItemT.Date < '%s')", 
					COleDateTime(m_dtFrom.GetValue()).Format("%Y-%m-%d"), 
					(COleDateTime(m_dtTo.GetValue()) + COleDateTimeSpan(1, 0, 0, 0)).Format("%Y-%m-%d"));
			IRowSettingsPtr pProviderSel = m_pProviderFilter->GetCurSel();
			if(pProviderSel != NULL){
				long nProvID = VarLong(pProviderSel->GetValue(provColID), cnAllValues);
				if(nProvID != cnAllValues){
					//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
					CString strIDList, strID;
					for(int i=0; i < m_arProviderFilterIDList.GetSize(); i++) {
						strID.Format("%li,", m_arProviderFilterIDList.GetAt(i));
						strIDList += strID;
					}
					strIDList.TrimRight(",");
					strProviderFilter.Format(" AND ChargesT.DoctorsProviders IN(%s)", strIDList);
				}
			}

			// (j.jones 2009-11-03 15:38) - PLID 36137 - submitted entries are colored gray
			// (j.gruber 2011-06-28 12:37) - PLID 44906 - take out original and voided charges and bills
			strFrom.Format(
				"(SELECT PersonT.ID AS PatientID, PatientsT.UserDefinedID, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
					//(e.lally 2009-11-25) PLID 36441 - Use the earliest charge date just in case two charges on the same
						//bill are different. I don't think we want to split the records out in that instance unless we
						//hear of a legitimate reason to separate the TOPS "cases".
					"BillsT.ID AS BillID, MIN(LineItemT.Date) as ProcedureDate, "
					//(e.lally 2009-11-25) PLID 36441 - Use new function to get a string of delimited procedure names.
					//Note that the use of this function does not scale well, but until we have a better way of gather such
					//strings in a sql statement, we will have to take the performance hit.
					"dbo.GetBillProcedureNames(BillsT.ID) AS ProcedureNames, "
					//(e.lally 2009-11-24) PLID 36415 - Add a null appointment ID
					"NULL AS ApptID, "
					"TOPSSubmissionHistoryQ.LastSentDate, "
					"CASE WHEN TOPSSubmissionHistoryQ.LastSentDate Is Not Null THEN %li ELSE 0 END AS Color "
				"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"INNER JOIN CptCodeT ON ServiceT.ID = CptCodeT.ID "
					"INNER JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
					"LEFT JOIN (SELECT BillID, Max(SubmissionDate) AS LastSentDate "
						"FROM TOPSSubmissionHistoryT "
						"GROUP BY BillID) AS TOPSSubmissionHistoryQ ON BillsT.ID = TOPSSubmissionHistoryQ.BillID "
					" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
					" LEFT JOIN LineItemCorrectionsT LCOrigT ON ChargesT.ID = LCOrigT.OriginalLineItemID \r\n "
					" LEFT JOIN LineItemCorrectionsT LCVoidT ON ChargesT.ID = LCVoidT.VoidingLineItemID \r\n "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted =0 AND LineItemT.Type = 10 "
				" AND BillCorrectionsT.ID IS NULL AND LCOrigT.ID IS NULL AND LCVoidT.ID IS NULL " 
					" %s %s %s %s %s"
				"GROUP BY PersonT.ID, PatientsT.UserDefinedID, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
					//(e.lally 2009-11-25) PLID 36441 - take out the line item date and put in the function
						//again, using a function here causes a performance hit in scalable situations. 
					"BillsT.ID, dbo.GetBillProcedureNames(BillsT.ID), "
					"TOPSSubmissionHistoryQ.LastSentDate"
				") SubQ ",
				cnSubmittedCaseColor,
				strProcedureFilter, strProviderFilter, strDateFilter,
				strSubmissionFilter, 
				// (f.dinatale 2010-09-09) - PLID 38192 - Added the Location filter
				strLocationFiler);
		}
		else{
			//(e.lally 2009-11-24) PLID 36415 - Generate the appropriate filters
			strDateFilter.Format(" AND (AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s')", 
					COleDateTime(m_dtFrom.GetValue()).Format("%Y-%m-%d"), 
					(COleDateTime(m_dtTo.GetValue()) + COleDateTimeSpan(1, 0, 0, 0)).Format("%Y-%m-%d"));
			IRowSettingsPtr pProviderSel = m_pProviderFilter->GetCurSel();
			if(pProviderSel != NULL){
				long nProvID = VarLong(pProviderSel->GetValue(provColID), cnAllValues);
				if(nProvID != cnAllValues){
					//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
					CString strIDList, strID;
					for(int i=0; i < m_arProviderFilterIDList.GetSize(); i++) {
						strID.Format("%li,", m_arProviderFilterIDList.GetAt(i));
						strIDList += strID;
					}
					strIDList.TrimRight(",");
					strProviderFilter.Format(" AND AppointmentsT.ID IN(SELECT AppointmentID FROM AppointmentResourceT WITH(NOLOCK) "
						"WHERE ResourceID IN(%s)) ", strIDList);
				}
			}
			//(e.lally 2009-11-25) PLID 35803 - We don't really want to directly join on the appointment purposes because of the one
				//to many property that would duplicate our records (optionally, we could have grouped by all the fields returned)
				//instead we'll use the IN clause when filtering on specific procedures.
			CString strProcedureWhereClause="";
			if(!strProcedureFilter.IsEmpty()){
				strProcedureWhereClause.Format(" AND AppointmentsT.ID IN(SELECT AppointmentID "
					"FROM AppointmentPurposeT WITH(NOLOCK) "
					"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
					"WHERE 1=1 %s )\r\n", strProcedureFilter);
			}

			strFrom.Format(
			"(SELECT PersonT.ID AS PatientID, PatientsT.UserDefinedID, \r\n"
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, \r\n"
			"NULL AS BillID, TOPSSubmissionHistoryQ.LastSentDate, \r\n"
			"CASE WHEN TOPSSubmissionHistoryQ.LastSentDate Is Not Null THEN %li ELSE 0 END AS Color, \r\n"
			"AppointmentsT.ID AS ApptID, AppointmentsT.Date as ProcedureDate, \r\n"
			//(e.lally 2009-11-25) PLID 36441 - Changed to procedure names plural
			"dbo.GetPurposeString(AppointmentsT.ID) AS ProcedureNames  \r\n"
							 
			"FROM AppointmentsT WITH(NOLOCK) \r\n"
			"INNER JOIN AptTypeT WITH(NOLOCK) ON AppointmentsT.AptTypeID = AptTypeT.ID \r\n"
			"INNER JOIN PersonT WITH(NOLOCK) ON AppointmentsT.PatientID = PersonT.ID \r\n"
			"INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID \r\n"
			"LEFT JOIN (SELECT AppointmentID, Max(SubmissionDate) AS LastSentDate \r\n"
				"FROM TOPSSubmissionHistoryT \r\n"
				"GROUP BY AppointmentID) TOPSSubmissionHistoryQ ON AppointmentsT.ID = TOPSSubmissionHistoryQ.AppointmentID \r\n"
			"WHERE AppointmentsT.PatientID <> -25 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND AptTypeT.Category IN(3,4) \r\n"
				" %s %s %s %s %s\r\n"
			") SubQ ",
			cnSubmittedCaseColor,
			strProcedureWhereClause, strProviderFilter, strDateFilter,
			strSubmissionFilter, 
			// (f.dinatale 2010-09-09) - PLID 38192 - Added the Location filter
			strLocationFiler);
		}

		m_pTopsSearchList->FromClause = _bstr_t(strFrom);
		m_pTopsSearchList->Requery();
	
	} NxCatchAll(__FUNCTION__);
}

CString CTopsSearchDlg::FormatTopsHiddenElement(CString strElementName, CString strData)
{
	CString strOutput;
	strOutput.Format("<input type=hidden name=\"%s\" value=\"%s\">\r\n", strElementName, XMLEncode(strData));

	return strOutput;
}
CString CTopsSearchDlg::FormatTopsTextAreaElement(CString strElementName, CString strData)
{
	CString strOutput;
	//In a text area, we should be able to use the raw XML. Not sure if there is a way to hide the field though.
	strOutput.Format("<textarea rows=30 cols=100 name=\"%s\" id=\"%s\">\r\n"
		"%s\r\n</textarea>\r\n", strElementName, strElementName, strData);

	return strOutput;
}


// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GetStartTOPSXMLFromCode(CString strCode) {

	if (strCode == "TOPS_ProcedureDuration") {
		return "<ProcedureDuration>";
	}
	else if (strCode == "TOPS_AnesthesiaProvidedBy") {
		return "<AnesthesiaProvidedBy value=\"";
	}
	else if (strCode == "TOPS_ModeOfAnesthesia") {
		return "     <ModeOfAnesthesia value=\"";
	}
	else if (strCode == "TOPS_TobaccoUse") {
		return "<tp_pro_cli_tobacco value=\"";
	}
	else if (strCode == "TOPS_HasDiabetes") {
		return "<tp_pro_cli_diabetes value=\"";
	}
	else if (strCode == "TOPS_DiabetesMedication") {
		return "<tp_pro_cli_diabetes_med value=\"";
	}
	else if (strCode == "TOPS_PatientASAStatus") {
		return "<tp_pro_cli_asa value=\"";
	}
	//(e.lally 2010-01-22) PLID 36839 - Added 24 more TOPS codes, updated new XML node values to be Outcome's XPath.
	else if (strCode == "TOPS_VTEProphylaxis") {
		return "<VTE value=\"";
	}
	else if (strCode == "TOPS_ProcRelatedWtLoss") {
		return "<Weightloss value=\"";
	}
	else if (strCode == "TOPS_PrevBariatricSurg") {
		return "<Bariatric value=\"";
	}
	//(e.lally 2010-02-24) PLID 36839 - And now we have to use the <tag>value</tag> format here because Outcome changed it on their end,
		//breaking the original method rather than supporting both.
	else if (strCode == "TOPS_LipoplastyVolInfused") {
		return "<LipoplastyVolumn>";
		//return "<LipoplastyVolumn value=\"";
	}
	else if (strCode == "TOPS_LipoplastyIVIntake") {
		return "<LipoplastyIVIntake>";
		//return "<LipoplastyIVIntake value=\"";
	}
	else if (strCode == "TOPS_LipoplastyAspirated") {
		return "<LipoplastyAspirated>";
		//return "<LipoplastyAspirated value=\"";
	}
	else if (strCode == "TOPS_BreastImplantRight") {
		return "<RBreastImplant value=\"";
	}
	else if (strCode == "TOPS_BreastImplantLeft") {
		return "<LBreastImplant value=\"";
	}
	else if (strCode == "TOPS_ImplantMfrRight") {
		return "<RBreastImplantMan value=\"";
	}
	else if (strCode == "TOPS_ImplantMfrLeft") {
		return "<LBreastImplantMan value=\"";
	}
	//(e.lally 2010-02-24) PLID 36839 - And now we have to use the <tag>value</tag> format here because Outcome changed it on their end,
		//breaking the original method rather than supporting both.
	else if (strCode == "TOPS_ImplantSerialRight") {
		return "<RBreastSerialNum>";
		//return "<RBreastSerialNum value=\"";
	}
	else if (strCode == "TOPS_ImplantSerialLeft") {
		return "<LBreastSerialNum>";
		//return "<LBreastSerialNum value=\"";
	}
	else if (strCode == "TOPS_ImplantShellRight") {
		return "<RBreastShell value=\"";
	}
	else if (strCode == "TOPS_ImplantShellLeft") {
		return "<LBreastShell value=\"";
	}
	else if (strCode == "TOPS_ImplantShapeRight") {
		return "<RBreastImplantShape value=\"";
	}
	else if (strCode == "TOPS_ImplantShapeLeft") {
		return "<LBreastImplantShape value=\"";
	}
	else if (strCode == "TOPS_ImplantFillerRight") {
		return "<RBreastFiller value=\"";
	}
	else if (strCode == "TOPS_ImplantFillerLeft") {
		return "<LBreastFiller value=\"";
	}
	else if (strCode == "TOPS_PostOpAdjRight") {
		return "<RBreastAdjustable value=\"";
	}
	else if (strCode == "TOPS_PostOpAdjLeft") {
		return "<LBreastAdjustable value=\"";
	}
	else if (strCode == "TOPS_ImplantPositionRight") {
		return "<RBreastPosition value=\"";
	}
	else if (strCode == "TOPS_ImplantPositionLeft") {
		return "<LBreastPosition value=\"";
	}
	//(e.lally 2010-02-24) PLID 36839 - And now we have to use the <tag>value</tag> format here because Outcome changed it on their end,
		//breaking the original method rather than supporting both.
	else if (strCode == "TOPS_FillerVolumeRight") {
		return "<RBreastFillerVolumn>";
		//return "<RBreastFillerVolumn value=\"";
	}
	else if (strCode == "TOPS_FillerVolumeLeft") {
		return "<LBreastFillerVolumn>";
		//return "<LBreastFillerVolumn value=\"";
	}
	else {
		return "";
	}

}

// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GetEndTOPSXMLFromCode(CString strCode) {

	if (strCode == "TOPS_ProcedureDuration") {
		return "</ProcedureDuration>\r\n";
	}
	//(e.lally 2010-01-22) PLID 36839 - Ideally, we would use the <tag>value</tag> format for fields with unlimited values,
	//but that is not supported at this very moment.
	//(e.lally 2010-02-24) PLID 36839 - And now we have to use the <tag>value</tag> format here because Outcome changed it on their end,
		//breaking the original method rather than supporting both.
	else if (strCode == "TOPS_LipoplastyVolInfused") {
		return "</LipoplastyVolumn>\r\n";
	}
	else if (strCode == "TOPS_LipoplastyIVIntake") {
		return "</LipoplastyIVIntake>\r\n";
	}
	else if (strCode == "TOPS_LipoplastyAspirated") {
		return "</LipoplastyAspirated>\r\n";
	}
	else if (strCode == "TOPS_ImplantSerialRight") {
		return "</RBreastSerialNum>\r\n";
	}
	else if (strCode == "TOPS_ImplantSerialLeft") {
		return "</LBreastSerialNum>\r\n";
	}
	else if (strCode == "TOPS_FillerVolumeRight") {
		return "</RBreastFillerVolumn>\r\n";
	}
	else if (strCode == "TOPS_FillerVolumeLeft") {
		return "</LBreastFillerVolumn>\r\n";
	}
	else if (strCode == "TOPS_AnesthesiaProvidedBy"
		|| strCode == "TOPS_ModeOfAnesthesia"
		|| strCode == "TOPS_TobaccoUse"
	    || strCode == "TOPS_HasDiabetes"
		|| strCode == "TOPS_DiabetesMedication"
		|| strCode == "TOPS_PatientASAStatus"
		//(e.lally 2010-01-14) PLID 36839
		|| strCode == "TOPS_VTEProphylaxis"
		|| strCode == "TOPS_ProcRelatedWtLoss"
		|| strCode == "TOPS_PrevBariatricSurg"
		|| strCode == "TOPS_BreastImplantRight"
		|| strCode == "TOPS_BreastImplantLeft"
		|| strCode == "TOPS_ImplantMfrRight"
		|| strCode == "TOPS_ImplantMfrLeft"
		|| strCode == "TOPS_ImplantShellRight"
		|| strCode == "TOPS_ImplantShellLeft"
		|| strCode == "TOPS_ImplantShapeRight"
		|| strCode == "TOPS_ImplantShapeLeft"
		|| strCode == "TOPS_ImplantFillerRight"
		|| strCode == "TOPS_ImplantFillerLeft"
		|| strCode == "TOPS_PostOpAdjRight"
		|| strCode == "TOPS_PostOpAdjLeft"
		|| strCode == "TOPS_ImplantPositionRight"
		|| strCode == "TOPS_ImplantPositionLeft"
		//(e.lally 2010-01-22) PLID 36839 - Ideally, we would use the <tag>value</tag> format for fields with unlimited values,
			//but that is not supported at this very moment.
		//(e.lally 2010-02-24) PLID 36839 - And now we have to use the <tag>value</tag> format here because Outcome changed it on their end,
			//breaking the original method rather than supporting both.
		/*
		|| strCode == "TOPS_LipoplastyVolInfused"
		|| strCode == "TOPS_LipoplastyIVIntake"
		|| strCode == "TOPS_LipoplastyAspirated"
		|| strCode == "TOPS_ImplantSerialRight"
		|| strCode == "TOPS_ImplantSerialLeft"
		|| strCode == "TOPS_FillerVolumeRight"
		|| strCode == "TOPS_FillerVolumeLeft"
		*/
		) {
			return "\" />\r\n";
	}
	else {
		return "";
	}	

}

// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GetHeaderForMultiItem(CString strCode) {

	if (strCode == "TOPS_ModeOfAnesthesia") {
		return "<AnesthesiaModes>\r\n";
	}
	else {
		return "";
	}
}

// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GetFooterForMultiItem(CString strCode) {

	if (strCode == "TOPS_ModeOfAnesthesia") {
		return "</AnesthesiaModes>\r\n";
	}
	else {
		return "";
	}
}

// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
void CTopsSearchDlg::GetValuesFromArray(CArray<TOPSField*, TOPSField*> *aryTOPS, CStringArray *strAry, long nItemID) {

	for (int i = 0; i < aryTOPS->GetSize(); i++) {

		long nItemIDArray = aryTOPS->GetAt(i)->nItemID;

		if (nItemIDArray == nItemID) {

			for (int j = 0; j < aryTOPS->GetAt(i)->parySelections->GetSize(); j++) {
				strAry->Add(aryTOPS->GetAt(i)->parySelections->GetAt(j)->strSelection);
			}
		}
	}
}


// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GenerateStringFromMemory(CArray<TOPLevel*, TOPLevel*> *aryTopLevel, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapSelectedValues )
{
	CString strReturn;
	try {

		
		for (int i = 0; i < aryTopLevel->GetSize(); i++) {

			CString strCode = aryTopLevel->GetAt(i)->strCode;
			BOOL bAllowMultiples = aryTopLevel->GetAt(i)->bAllowMultiples;
			CArray<TOPSField*, TOPSField*> *aryTOPS = aryTopLevel->GetAt(i)->aryTOPS;
			BOOL bIsCCDField = aryTopLevel->GetAt(i)->bIsCCDField;

			if (!bIsCCDField) {
				CString strValue;

				//see if we have a value
				if (mapSelectedValues)  {
					if (mapSelectedValues->Lookup(strCode, strValue)) {

						if (bAllowMultiples) {

							//the 'value' in the map is just the ItemID, we need to get the Value
							CStringArray aryStr;
							long nItemID = atoi(strValue);
							GetValuesFromArray(aryTOPS, &aryStr, nItemID);
							strReturn += GetHeaderForMultiItem(strCode);
							for (int j = 0; j < aryStr.GetSize(); j++) {
								strReturn += GetStartTOPSXMLFromCode(strCode) + aryStr.GetAt(j) + GetEndTOPSXMLFromCode(strCode);
							}
							strReturn += GetFooterForMultiItem(strCode);
						}
						else {

							//excellent! its in our map, we just have to select it
							strReturn += GetStartTOPSXMLFromCode(strCode) + strValue + GetEndTOPSXMLFromCode(strCode);
						}

					}
					else {

						//it's not in our map, so it should have only one value, if not, don't add it
						
						if (aryTOPS->GetSize() == 1) {

							if (bAllowMultiples) {

								//we have multiple items for our one item
								long nItemID = aryTOPS->GetAt(0)->nItemID;

								CStringArray aryStr;
								GetValuesFromArray(aryTOPS, &aryStr, nItemID);
								strReturn += GetHeaderForMultiItem(strCode);
								for (int j = 0; j < aryStr.GetSize(); j++) {
									strReturn += GetStartTOPSXMLFromCode(strCode) + aryStr.GetAt(j) + GetEndTOPSXMLFromCode(strCode);
								}
								strReturn += GetFooterForMultiItem(strCode);
							}
							else {

								if (aryTOPS->GetAt(0)->parySelections->GetSize() == 1) {

									//we only have 1 item, 1 value 
									strReturn += GetStartTOPSXMLFromCode(strCode) + aryTOPS->GetAt(0)->parySelections->GetAt(0)->strSelection + GetEndTOPSXMLFromCode(strCode);
								}
							}

						}
					}
				}
				else {

					//they either cancelled the selection dialog, or we only have 1 value for each option
					//either way, we can only fill, if there is only 1 value in the list
					if (aryTOPS->GetSize() == 1) {

						if (bAllowMultiples) {

							//we have multiple items for our one item
							long nItemID = aryTOPS->GetAt(0)->nItemID;

							CStringArray aryStr;
							GetValuesFromArray(aryTOPS, &aryStr, nItemID);
							strReturn += GetHeaderForMultiItem(strCode);
							for (int j = 0; j < aryStr.GetSize(); j++) {
								strReturn += GetStartTOPSXMLFromCode(strCode) + aryStr.GetAt(j) + GetEndTOPSXMLFromCode(strCode);
							}
							strReturn += GetFooterForMultiItem(strCode);
						}
						else {

							if (aryTOPS->GetAt(0)->parySelections->GetSize() == 1) {

								//we only have 1 item, 1 value 
								strReturn += GetStartTOPSXMLFromCode(strCode) + aryTOPS->GetAt(0)->parySelections->GetAt(0)->strSelection + GetEndTOPSXMLFromCode(strCode);
							}
						}
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

	return strReturn;

}


void CTopsSearchDlg::GetExcludedCCDFields(CArray<TOPLevel*, TOPLevel*> *aryTopLevel, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapSelectedValues, CString &strExcludedDetails, CString &strExcludedItems ) 
{

	try {

		
		for (int i = 0; i < aryTopLevel->GetSize(); i++) {

			BOOL bIsCCDField = aryTopLevel->GetAt(i)->bIsCCDField;
			if (bIsCCDField) {

				CString strCode = aryTopLevel->GetAt(i)->strCode;
				BOOL bAllowMultiples = aryTopLevel->GetAt(i)->bAllowMultiples;
				CArray<TOPSField*, TOPSField*> *aryTOPS = aryTopLevel->GetAt(i)->aryTOPS;

				CString strValue;

				//see if we have a value
				if (mapSelectedValues)  {

					//they oked the dialog
					if (mapSelectedValues->Lookup(strCode, strValue)) {

						//loop through the Field list and add anything that isn't the value
						long nFoundDetailID = -1;
						for (int j = 0; j < aryTOPS->GetSize(); j++) {

							TOPSField *pField = aryTOPS->GetAt(j);

							if (pField) {

								if (nFoundDetailID == -1) {

									//there can only be one value for CCD fields
									if (pField->parySelections->GetSize() == 1) {

										//compare the value
										CString strItemValue = pField->parySelections->GetAt(0)->strSelection;

										if (strItemValue == strValue) {

											//we found it
											nFoundDetailID = pField->parySelections->GetAt(0)->nSelectID;

											//nothing else is necessary
										}
										else {
											//it doesn't match, add it to our list of exclusions
											strExcludedItems += AsString(pField->nItemID) + ",";
										}
									}
									else {
										//we need to loop to see if any detail matches
										for (int k = 0; k < pField->parySelections->GetSize(); k++) {

											TOPSSelection *pSel = pField->parySelections->GetAt(k);
											if (pSel) {
												if (pSel->strSelection == strValue) {

													nFoundDetailID = pSel->nSelectID;
												}
												else {
													//add it to the detailed excluded list
													strExcludedDetails += AsString(pSel->nSelectID) + ",";
												}
											}
											else {
												ASSERT(FALSE);
											}
										}
									}
								}
								else {

									//we already found our value, so add this as an exclusion
									strExcludedItems += AsString(pField->nItemID) + ",";
								}
							}
							else {
								//this shouldn't be possible
								ASSERT(FALSE);
							}
						}
					}
					else {

						//there is no selection from the dialog, but there is a map, so check if there are multiple values
						if (aryTOPS->GetSize() == 1) {

							//make sure there is only one selection
							if (aryTOPS->GetAt(0)->parySelections->GetSize() == 1) {

								//no reason to exclude it, since its valid
								//the map must be for another field
							}
							else {

								//multiple selections, exclude it
								strExcludedItems += AsString(aryTOPS->GetAt(0)->nItemID) + ",";
							}
						}
						else {

							//multiple selections, so its invalid, exclude all necessary values
							for (int j = 0; j < aryTOPS->GetSize(); j++) {

								strExcludedItems += AsString(aryTOPS->GetAt(j)->nItemID) + ",";
							}
						}
					}
				}
				else {

					//there is no map, so check our value to see if there is only one
					//there is no selection from the dialog, but there is a map, so check if there are multiple values
					if (aryTOPS->GetSize() == 1) {

						//make sure there is only one selection
						if (aryTOPS->GetAt(0)->parySelections->GetSize() == 1) {

							//no reason to exclude it, since its valid
							//the map must be for another field
						}
						else {

							//multiple selections, exclude it
							strExcludedItems += AsString(aryTOPS->GetAt(0)->nItemID) + ",";
						}
					}
					else {

						//multiple selections, so its invalid, exclude all necessary values
						for (int j = 0; j < aryTOPS->GetSize(); j++) {

							strExcludedItems += AsString(aryTOPS->GetAt(j)->nItemID) + ",";
						}
					}
				}
			}
		}

		strExcludedDetails.TrimRight(",");
		strExcludedItems.TrimRight(",");

	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2009-11-23 12:47) - PLID 36139 - getting multiple values
CString CTopsSearchDlg::GenerateTopsFields(long nPatientID, COleDateTime dtProcDate, CString strPatientName, CString &strCCDExcludedItems, CString &strCCDExcludedSelections) {

	try {

		CString strReturn;

		//run our procedure to get all the information we will need
		_RecordsetPtr prs = CreateParamRecordset("SET NOCOUNT ON; \r\n " 
			" DECLARE @nPatientID int; \r\n"
			" SET @nPatientID = {INT}; \r\n "
			
			" DECLARE @tResults TABLE (TOPSCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--tops codes \r\n "
			"	('TOPS_ProcedureDuration','TOPS_AnesthesiaProvidedBy','TOPS_ModeOfAnesthesia','TOPS_TobaccoUse','TOPS_HasDiabetes','TOPS_DiabetesMedication','TOPS_PatientASAStatus', '3141-9', '8302-2', "
			//(e.lally 2010-01-14) PLID 36839
				"'TOPS_VTEProphylaxis', 'TOPS_ProcRelatedWtLoss', 'TOPS_PrevBariatricSurg', 'TOPS_LipoplastyVolInfused', 'TOPS_LipoplastyIVIntake', 'TOPS_LipoplastyAspirated', "
				"'TOPS_BreastImplantRight', 'TOPS_BreastImplantLeft', 'TOPS_ImplantMfrRight', 'TOPS_ImplantMfrLeft', "
				"'TOPS_ImplantSerialRight', 'TOPS_ImplantSerialLeft', 'TOPS_ImplantShellRight', 'TOPS_ImplantShellLeft', "
				"'TOPS_ImplantShapeRight', 'TOPS_ImplantShapeLeft', 'TOPS_ImplantFillerRight', 'TOPS_ImplantFillerLeft', "
				"'TOPS_PostOpAdjRight', 'TOPS_PostOpAdjLeft', 'TOPS_ImplantPositionRight', 'TOPS_ImplantPositionLeft', "
				"'TOPS_FillerVolumeRight', 'TOPS_FillerVolumeLeft' "
				") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (TOPSCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (TOPSCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID		  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
			"	SET NOCOUNT OFF; \r\n "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "			
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ProcedureDuration' and Value <> '';\r\n "

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "			
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_AnesthesiaProvidedBy' and Value <> ''; "

			"	SELECT TResults.*, CONVERT(bit, 1) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ModeOfAnesthesia' and Value <> ''; "

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "			
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_TobaccoUse' and Value <> ''; "

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_HasDiabetes' and Value <> ''; "

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_DiabetesMedication' and Value <> ''; "

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_PatientASAStatus' and Value <> ''; "		
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 1) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = '3141-9' and Value <> ''; "		

			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 1) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = '8302-2' and Value <> ''; "		
			//(e.lally 2010-01-14) PLID 36839
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_VTEProphylaxis' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ProcRelatedWtLoss' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_PrevBariatricSurg' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_LipoplastyVolInfused' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_LipoplastyIVIntake' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_LipoplastyAspirated' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_BreastImplantRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "	
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_BreastImplantLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantMfrRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantMfrLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantSerialRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "	
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantSerialLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantShellRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantShellLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantShapeRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantShapeLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantFillerRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantFillerLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_PostOpAdjRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_PostOpAdjLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantPositionRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_ImplantPositionLeft' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_FillerVolumeRight' and Value <> ''; "
			
			"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples, CONVERT(bit, 0) as IsCCDField \r\n "
			"	FROM @tResults TResults WHERE TopsCode = 'TOPS_FillerVolumeLeft' and Value <> ''; "
			, nPatientID);
		
		CArray<TOPLevel*, TOPLevel*> aryTopLevel;

		BOOL bShowDialog = FALSE;
		BOOL bAllowMultiples = FALSE;
		BOOL bIsCCDField = FALSE;
		
		while (prs) {

			TOPLevel *tpLevel = NULL; 

			FieldsPtr flds = prs->Fields;
						
			CString strOldID = "";

			TOPSField *tpField = NULL;
			CArray<TOPSField*,TOPSField*> *aryTOPS = NULL; 			

			while (! prs->eof) {				

				if (tpLevel == NULL ) {
					tpLevel = new TOPLevel();
				}

				if (aryTOPS == NULL) {
					aryTOPS = new CArray<TOPSField*,TOPSField*>();
				}

				if (tpField == NULL) {
					tpField = new TOPSField();
					tpField->parySelections = new CArray<TOPSSelection*, TOPSSelection*>();
				}

				CString strCode = AdoFldString(flds, "TOPSCode");
				CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
				long nItemID = AdoFldLong(flds, "ItemID");
				CString strItemName = AdoFldString(flds, "ItemName");
				CString strEMNName = AdoFldString(flds, "EMNName");
				COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
				CString strValue = AdoFldString(flds, "Value");				
				bAllowMultiples = AdoFldBool(flds, "AllowMultiples");
				bIsCCDField = AdoFldBool(flds, "IsCCDField");
				long nSelectID = AdoFldLong(flds, "SelectID");
				CString strCurrentID;				

				CString strRow;
				CString strSelection;

				strCurrentID = strCode + "-" + AsString(nItemID);

				tpLevel->strCode = strCode;
				tpLevel->strCodeDesc = strCodeDesc;
				tpLevel->bAllowMultiples = bAllowMultiples;
				tpLevel->bIsCCDField = bIsCCDField;

				if (strOldID != strCurrentID) {

					//add one to our array
					if (strOldID != "") {						
						aryTOPS->Add(tpField);
						tpField = new TOPSField();
						tpField->parySelections = new CArray<TOPSSelection*, TOPSSelection*>();
						if ((!bAllowMultiples) && tpField->parySelections->GetSize() > 1) {
							bShowDialog = TRUE;
						}
					}
					
					strOldID = strCurrentID;					
					tpField->nItemID = nItemID;
					tpField->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
				}

				
				TOPSSelection *pSel = new TOPSSelection();
				pSel->strSelection = strValue;
				pSel->nSelectID = nSelectID;
				tpField->parySelections->Add(pSel);

				prs->MoveNext();
			}		

			//add the last value
			if (tpField) {
				if ((!bAllowMultiples) && tpField->parySelections->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				aryTOPS->Add(tpField);
			}

			if (aryTOPS) {
				if (aryTOPS->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				tpLevel->aryTOPS = aryTOPS;
			}

			//add it to the top most array
			if (tpLevel) {
				aryTopLevel.Add(tpLevel);
			}

			prs = prs->NextRecordset(NULL);
		}		
		
		//now we have to send everything to our dialog if necessary
		if (aryTopLevel.GetSize() != 0) {
			if (bShowDialog) {
				CTopsFieldPickDlg dlg(this);
				CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
				dlg.m_paryTopsList = &aryTopLevel;			
				dlg.m_pmapReturnValues = &mapValuesFromDialog;
				dlg.m_strPatientName = strPatientName;
				dlg.m_dtProcDate = dtProcDate;


				if (dlg.DoModal() == IDCANCEL) {
					strReturn = GenerateStringFromMemory(&aryTopLevel, NULL);
					GetExcludedCCDFields(&aryTopLevel, NULL, strCCDExcludedSelections, strCCDExcludedItems);
				}
				else {
					//we now should have an map of values			
					strReturn = GenerateStringFromMemory(&aryTopLevel, &mapValuesFromDialog);
					GetExcludedCCDFields(&aryTopLevel, &mapValuesFromDialog, strCCDExcludedSelections, strCCDExcludedItems);
				}
			}
			else {
				strReturn = GenerateStringFromMemory(&aryTopLevel, NULL);
				//we have no fields that need to be excluded
				strCCDExcludedSelections = "";
				strCCDExcludedItems = "";
				
			}
		}

		//now clear out all our memory
		for (int i = aryTopLevel.GetSize()-1; i >= 0; i--) {
			TOPLevel *pLevel = aryTopLevel.GetAt(i);

			for (int j = pLevel->aryTOPS->GetSize() - 1; j >= 0; j--) {

				TOPSField *pField = pLevel->aryTOPS->GetAt(j);

				for (int k = pField->parySelections->GetSize() - 1; k >= 0; k--) {
					
					TOPSSelection *pSel = pField->parySelections->GetAt(k);
					delete pSel;
				}				
				delete pField->parySelections;
				delete pField;
			}

			delete pLevel->aryTOPS;

			delete pLevel;
		}
/*
#ifdef _DEBUG
		AfxMessageBox(strReturn);
#endif
*/
		return strReturn;


	}NxCatchAll(__FUNCTION__);

	return "";
}


void CTopsSearchDlg::OnBnClickedSendToTops()
{
	try {
		//(e.lally 2009-10-14) PLID 35913 - Make the form URL configurable, just in case.
		//(e.lally 2010-01-11) PLID 36838 - Fixed the form URL to use the form manager html page.
		CString strUrl = GetRemotePropertyText("TOPS_FormUrl", "https://tops.plasticsurgery.org/osformmanager.html", 0, "<None>");
		//Test site: "https://topstest.plasticsurgery.org/osformmanager.html"
		//(e.lally 2009-10-14) PLID 35913 - The temp html file method was super slow (60-120 seconds!), but the direct post only supports using IE,
		//	since both methods had to be coded for testing, I added a hidden option so we can change it if the default is not working well.
		//	If it turns out that we need to change it for several clients, we can add it to the preferences dlg.
		long nSubmitMethod = GetRemotePropertyInt("TOPS_SubmitMethod", 0, 0, GetCurrentUserName());

		//Get the login information for this provider
		//These are the test values for us to use on the topstest site.
		//strUsername = "nextech";
		//strPW = "n3xtech";
		// (d.thompson 2009-11-04) - PLID 36136 - Pull from the interface
		CString strUsername, strPW, strProxiedUsername, strFormID;
		GetDlgItemText(IDC_TOPS_USERNAME, strUsername);
		GetDlgItemText(IDC_TOPS_PROXIED_NAME, strProxiedUsername);
		GetDlgItemText(IDC_TOPS_PASSWORD, strPW);
		strFormID = "8701";

		//(e.lally 2009-12-08) PLID 35803 - Check for blank credentials before continuing
		if(strUsername.IsEmpty() || strProxiedUsername.IsEmpty() || strPW.IsEmpty()){
			MessageBox("Please enter your usernames and password before submitting");
			return;
		}

		//CCD contents
		//(e.lally 2009-10-12) PLID 35913 - Get selected data
		//(e.lally 2009-10-20) PLID 36001 - Run through this process for multiple selected rows
		IRowSettingsPtr pSearchRow = m_pTopsSearchList->GetFirstSelRow();
		IRowSettingsPtr pPrevRow;
		if(pSearchRow == NULL){
			MessageBox("You must select a record from the search results first.", "Practice", MB_OK);
			return;
		}
		//(e.lally 2009-11-25) PLID 35803 - Warn if they are going to send more than 5 cases at a time, 
			//flat out stop after 20 so that we don't bring their OS to a halt. We are doing this for their own good.
			//We record that the cases was last submitted as soon as we open the web browser, so if they have more windows
			//open than they can process, they won't actually know which ones were saved and which ones they closed.
			//Is there another way other than brute force?
		long nSelectedRowCount = 0;
		while (pSearchRow != NULL){
			nSelectedRowCount++;
			pSearchRow = pSearchRow->GetNextSelRow();
		}
		if(nSelectedRowCount > 5 && nSelectedRowCount <= cnMaxSelRecordBrowsers){
			//(e.lally 2009-11-25) PLID 35803 - regular warning
			if(IDCANCEL == MessageBox("Submitting multiple results at once will open a new web browser for each one.\r\n"
				"Are you sure you wish to continue?", "Practice", MB_OKCANCEL)){
					return;
			}
		}
		else if(nSelectedRowCount > cnMaxSelRecordBrowsers){
			//(e.lally 2009-11-25) PLID 35803 - We really mean it this time, 20 is a ton of windows when they are all open at once.
			CString str;
			str.Format("Submitting multiple results at once will open a new web browser for each one. "
				"Only the first %li selected results will be sent.\r\n"
				"Are you sure you wish to continue?", cnMaxSelRecordBrowsers);
			if(IDCANCEL == MessageBox(str, "Practice", MB_OKCANCEL|MB_ICONEXCLAMATION)){
					return;
			}
		}

		//(e.lally 2009-11-30) PLID 35803 - Get the first row now instead of the first selected row. See notes further down for why.
		pSearchRow = m_pTopsSearchList->GetFirstRow();
		long nSubmittedRecordCount = 0;

		long nPatientID =0, nBillID=0, nApptID=0;
		COleDateTime dtProcDate;
		CString strVisitDateXml, strCCDContents, strPrepopXml;
		//(e.lally 2009-11-30) PLID 35803 - re-traverse the datalist results, but this time do it in the LESS efficient manner
			//by visiting each row and checking if it is selected. This will provide the expected behavior of opening the browsers
			//in the order in which they are displayed, rather than in the order in which they were clicked (which can have odd results anyways).
		while (pSearchRow != NULL && nSubmittedRecordCount < cnMaxSelRecordBrowsers /*=20*/){
			if(pSearchRow->IsHighlighted()){
				nPatientID = VarLong(pSearchRow->GetValue(tslcPatientID));
				nBillID = VarLong(pSearchRow->GetValue(tslcBillID), -1);
				//(e.lally 2009-11-24) PLID 36415 - get the appointment ID, if there is one
				nApptID = VarLong(pSearchRow->GetValue(tslcApptID), -1);
				COleDateTime dtProcDate = VarDateTime(pSearchRow->GetValue(tslcProcedureDate));
				
				// (j.gruber 2009-11-20 16:59) - PLID 36139 - show the patient name on our dialog
				CString strPatientName = VarString(pSearchRow->GetValue(tslcPatientName), "");
				
				//Use format YYYYMMDD with 2 digits for month and day.
				strVisitDateXml.Format("<VisitDate>%s</VisitDate>\r\n", dtProcDate.Format("%Y%m%d"));

				// (j.gruber 2009-11-18 17:44) - PLID 36139 - generate the TOPS fields
				CString strCCDExcludedItems, strCCDExcludedSelections;
				CString strTops = GenerateTopsFields(nPatientID, dtProcDate, strPatientName, strCCDExcludedItems, strCCDExcludedSelections);			

				//(e.lally 2010-02-18) PLID 37438 - Pass in our connection ptr, "minimal" preference, userID and LocationID.
				bool bGenerateOptionalSections = true;
				CCD::ClinicalDocument Document(GetRemoteData(), bGenerateOptionalSections, GetCurrentUserID(), GetCurrentLocationID());
				//(e.lally 2009-10-12) PLID 35913 - Send in the bill ID and procedure parameter
				Document.Generate(nPatientID, nBillID, TRUE, strCCDExcludedItems, strCCDExcludedSelections);
				strCCDContents = Document.GetText();
				//Enable trace to output formatted XML to visual studio's output area.
				//Log("\r\n%s", PrettyPrintXML(Document.GetDocument()));			

				strPrepopXml = 
					"<?xml version=\"1.0\"?>\r\n"
					"<prepopData xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"urn:ihe:iti:rfd:2007\">\r\n"
					"<prepopdata>\r\n"
						+ strCCDContents +
					"</prepopdata>\r\n"
					"<Context>\r\n"
						"<SiteID />\r\n"
						"<StudyID /> \r\n"
						"<SubjectID />\r\n"
						"<UniqueSubjectID />\r\n"
						"<VisitNumber />\r\n"
						+ strVisitDateXml +
						"<TOPS_Fields>\r\n"
						// (j.gruber 2009-11-23 12:47) - PLID 36139 - get TOPS fields
						+ strTops + 
							/*"<ProcedureDuration>134</ProcedureDuration>\r\n"// <!-- given in minutes -->						
							"<AnesthesiaProvidedBy value=\"RN (supervised by procedural Surgeon)\" />\r\n"
						   <!-- Alternate values for AnesthesiaProvidedBy
						   "<AnesthesiaProvidedBy value=\"Anesthesiologist\" />
						   "<AnesthesiaProvidedBy value=\"CRNA (supervised by Anesthesiologist)\" />\r\n"
						   "<AnesthesiaProvidedBy value=\"CRNA (supervised by procedural Surgeon)\" />
						   "<AnesthesiaProvidedBy value=\"Procedural Surgeon\" />
						   "<AnesthesiaProvidedBy value=\"RN (supervised by procedural Surgeon)\" />
						   "<AnesthesiaProvidedBy value=\"Other\" />
						   "<AnesthesiaProvidedBy value=\"None\" />
						   -->
							"<AnesthesiaModes>\r\n"
								//"<ModeOfAnesthesia value=\"Conscious Sedation\" />\r\n"
								//"<ModeOfAnesthesia value=\"General\" />\r\n"
								//"<ModeOfAnesthesia value=\"MAC\"/>\r\n"
								"<ModeOfAnesthesia value=\"Peripheral Block\"/>\r\n"
								//"<ModeOfAnesthesia value=\"Spinal/Epidural\" />\r\n"
								"<ModeOfAnesthesia value=\"Topical/Local\"/>\r\n"
								//"<ModeOfAnesthesia value=\"Tumescent\"/>\r\n"
								//"<ModeOfAnesthesia value=\"Other\"/>\r\n"
								//"<ModeOfAnesthesia value=\"None\"/>\r\n"
							"</AnesthesiaModes>\r\n"
							"<tp_pro_cli_tobacco value=\"Current tobacco use:  Use of a tobacco product within past 30 days.\" />\r\n"
							<!-- Alternate values for tobacco use
							"<tp_pro_cli_tobacco value=\"Former Tobacco Use:  No tobacco product use for > 30 days.\" />\r\n"/*
							"<tp_pro_cli_tobacco value=\"Non-Tobacco User:  never used tobacco products\" />\r\n"
							-->\r\n"

							"<tp_pro_cli_diabetes value=\"Yes\" />\r\n"
							//<!-- Alternate values
							"<tp_pro_cli_diabetes value=\"No\" />\r\n"
							"<tp_pro_cli_diabetes value=\"Unknown\" />\r\n"
							//-->\r\n"

							"<tp_pro_cli_diabetes_med value=\"Insulin\" />\r\n"
							//<!-- Alternate values
							"<tp_pro_cli_diabetes_med value=\"Oral\" />\r\n"
							"<tp_pro_cli_diabetes_med value=\"Diet-Controlled\" />\r\n"
							//-->\r\n"

							"<tp_pro_cli_asa value=\"1: Normal\" />\r\n"
							//<!-- Alternate values
							"<tp_pro_cli_asa value=\"2: Mild systemic\" />\r\n"
							"<tp_pro_cli_asa value=\"3: Severe systemic\" />\r\n"
							"<tp_pro_cli_asa value=\"4: Constant life threat\" />\r\n"
							"<tp_pro_cli_asa value=\"5: Moribund\" />\r\n"
							//-->
							Bariatric			Values: Yes, No, Unknown
							LipoplastyAspirated Values: Decimal
							LipoplastyIVIntake	Values: Decimal
							LipoplastyVolumn	Values: Decimal
							RBreastAdjustable, LBreastAdjustable	Values: Yes, No
							RBreastFiller, LBreastFiller		Values : Saline, Saline/ Silicone Gel, Silicone Gel
							RBreastFillerVolumn, LBreastFillerVolumn	 Values: Decimal
							RBreastImplant, LBreastImplant	Values: Yes, No
							RBreastImplantMan, LBreastImplantMan	Values: Allergan, Mentor, Other
							RBreastImplantShape, LBreastImplantShape	Values: Round, Contour
							RBreastPosition, LBreastPosition		Values: Sub-glandular, Sub-muscular, Subcutaneous
							RBreastSerialNum, LBreastSerialNum	Value: Free text
							RBreastShell, LBreastShell	Values: Smooth, Textured
							VTE		Values:
								VTE prophylaxis ordered or delivered within 24 hours prior to incision or start time or within 24 hours of surgery end time
								VTE prophylaxis NOT ordered for medical reason
								VTE prophylaxis NOT ordered, no reason specified
							Weightloss	Values: Yes, No, Unknown
							
							*/
						"</TOPS_Fields>\r\n"
					"</Context>\r\n"
					"</prepopData>\r\n";

				
				if(nSubmitMethod == 0) {
					//(e.lally 2009-10-14) PLID 35913 - Post the data directly to the form URL. This method help avoid IE 8 from blocking the post with its
						//Cross-site scripting (XSS) filter. The performance is also significantly faster than the temp file method.
					CString strPostData = GenerateRawTopsPostData(strUsername, strPW, strProxiedUsername, strFormID, strPrepopXml);
					IEDirectPostToUrl(strUrl, strPostData);
				}
				else if(nSubmitMethod == 1) {
					//(e.lally 2009-10-12) PLID 35913 - Create a temp html file that auto submits to the TOPS form upon load
					//URL of form to POST to
					
					PostFromTempHtmlFile(strUrl, strUsername, strPW, strProxiedUsername, strFormID, strPrepopXml);
				}
				else{
					//Use a built in browser - not yet supported
					//CString strPostData = GenerateRawTopsPostData(strUsername, strPW, strProxiedUsername, strFormID, strPrepopXml);
					//PostFromBuiltInBrowser(strUrl, strPostData);
				}

				// (j.jones 2009-11-03 15:29) - PLID 36137 - track that we sent this record
				//(e.lally 2009-11-24) PLID 36415 - Check use of billing or scheduler
				if(m_radioBillingSearch.GetCheck()){
					ExecuteParamSql("INSERT INTO TOPSSubmissionHistoryT (BillID) VALUES ({INT})", nBillID);
				}
				else if(m_radioAppointmentSearch.GetCheck()){
					ExecuteParamSql("INSERT INTO TOPSSubmissionHistoryT (AppointmentID) VALUES ({INT})", nApptID);
				}

				//(e.lally 2009-10-20) PLID 36001 - Go to the next row
				pPrevRow = pSearchRow;
				pSearchRow = pSearchRow->GetNextRow();
				//(e.lally 2009-11-25) PLID 35803 - Up the count of records we're submitting at once to limit the number of open web browsers.
				nSubmittedRecordCount++;

				//(e.lally 2009-10-20) PLID 36001 - Remove the submitted row from our search list
				//(e.lally 2009-11-25) PLID 35803 - Handle the type of results being filtered on
				if(m_radioShowUnsubmitted.GetCheck()){
					//We are only showing unsubmitted, so this submitted record is removed from our list
					//Be sure to unselect it first in case the datalist decides to make the next row highlighted for no good reason.
					pPrevRow->PutSelected(VARIANT_FALSE);
					m_pTopsSearchList->RemoveRow(pPrevRow);
				}
				else{
					pPrevRow->ForeColor = cnSubmittedCaseColor;
					pPrevRow->PutValue(tslcColor, (long)cnSubmittedCaseColor);
					//(e.lally 2009-11-25) Unless we want to make a round trip to the SQL Server just to get today's date, use 
						//the Workstation's date to update the interface.
					COleDateTime dtNow = COleDateTime::GetCurrentTime();
					_variant_t varNow(dtNow, VT_DATE);
					pPrevRow->PutValue(tslcLastSentDate, varNow);
				}

			}
			else{
				//(e.lally 2009-11-30) PLID 35803 - Go to the next row
				pPrevRow = pSearchRow;
				pSearchRow = pSearchRow->GetNextRow();
			}
		}
		//(e.lally 2009-10-20) PLID 36001 - Ensure nothing is selected now.
		m_pTopsSearchList->CurSel = NULL;

		//Update the row count
		CString strRowCount;
		strRowCount.Format("Count: %li", m_pTopsSearchList->GetRowCount());
		SetDlgItemText(IDC_TOPS_SEARCH_COUNT, strRowCount);

	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::PostFromTempHtmlFile(CString strUrl, CString strUsername, CString strPW, CString strProxiedUsername, CString strFormID, CString strPrepopXml)
{
	//Instead of forming a URL which contains all the data, 
		//	instead generate a local HTML file with a javascript auto-submit on it to send the data.

		CString strHTML;
		//(e.lally 2009-10-12) PLID 35913 - Borrowed from the EMR preview pane logic. I was told this won't change, so making a shared
			//function to get this string won't be necessary.
		strHTML.Format("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Strict//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
			//Add in the "MarkOfTheWeb" to avoid the blocked script when loaded
			"<!-- saved from url=(0022)http://www.nextech.com -->\r\n" // oddly enough, this is needed to place our content
			// in the internet zone as defined by www.nextech.com, so www.nextech.com needs to be in the trusted zone to run script. This should work
			// by default (local machine files have all script disabled by default)
			"<html><form id=\"form1\" method=POST action=\"%s\">\r\n", strUrl);

		//Now each line will get added looking like:
		//	<input type=hidden name="FN" value="First Name">

		
		//username
		strHTML += FormatTopsHiddenElement("username", strUsername);

		//password
		strHTML += FormatTopsHiddenElement("password", strPW);
		
		//proxy username
		strHTML += FormatTopsHiddenElement("proxied_username", strProxiedUsername);

		//Form ID - should always be the same
		strHTML += FormatTopsHiddenElement("formID", strFormID);

		//Archive URL - unused
		strHTML += FormatTopsHiddenElement("archiveURL", "");


		strHTML += FormatTopsHiddenElement("prepopData", strPrepopXml);

		//(e.lally 2009-10-12) PLID 35913 - Wrap up our HTML - create a javascript that will submit the hidden form contents to the actual website upon loading.
		strHTML += 
			"</form> "
			"<script type=\"text/javascript\"> "
			"function loadform () { "
			"var frm = document.getElementById(\"form1\"); "
			"frm.submit(); "
			"} "
			"window.onload = loadform; "
			"</script>"
			"</html>";

		//
		//Now write the HTML to a temporary file.
		CString strFilename;
		HANDLE hFile = FileUtils::CreateTempFile("", "html", &strFilename, TRUE);
		CFile fOut(hFile);
		fOut.Write(strHTML, strHTML.GetLength());
		fOut.Close();

		//done, let's open the file.  The javascript should automatically take over and submit to the proper
		//	URL then.
		//Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent;
		if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, "", SW_MAXIMIZE) < 32){
			AfxMessageBox("Could not submit data to TOPS.");
		}

}



CString CTopsSearchDlg::CreateFieldValuePairForHttpPost(CString strField, CString strValue)
{
	CString strOut;
	strOut.Format("%s=%s", strField, EncodeURL(strValue));
	return strOut;
}
CString CTopsSearchDlg::GenerateRawTopsPostData(CString strUsername, CString strPW, CString strProxiedUsername, CString strFormID, CString strPrepopXml)
{
	//Build the post data
	CString strPostData = CreateFieldValuePairForHttpPost("username", strUsername);

	//password
	strPostData += "&" + CreateFieldValuePairForHttpPost("password", strPW);
	
	//proxy username
	strPostData += "&" + CreateFieldValuePairForHttpPost("proxied_username", strProxiedUsername);

	//Form ID - should always be the same
	strPostData += "&" + CreateFieldValuePairForHttpPost("formID", strFormID);

	//Archive URL - unused
	strPostData += "&" + CreateFieldValuePairForHttpPost("archiveURL", "");

	//prepop data with CCD contents XML
	strPostData += "&" + CreateFieldValuePairForHttpPost("prepopData", strPrepopXml);
	
	return strPostData;

}
void CTopsSearchDlg::IEDirectPostToUrl(CString strUrl, CString strRawPostData)
{
	//Open an IE browser and post directly to the web form
	{
		HRESULT hr;
		IWebBrowserApp* pWBApp = NULL; // Derived from IWebBrowser
		BSTR bstrURL = NULL, bstrHeaders = NULL;
		VARIANT vFlags = {0},
		vTargetFrameName = {0},
		vPostData = {0},
		vHeaders = {0};

		if (FAILED(hr = CoCreateInstance(CLSID_InternetExplorer,
			NULL,
			CLSCTX_SERVER,
			IID_IWebBrowserApp,
			(LPVOID*)&pWBApp)))
		{
			if (bstrURL) SysFreeString(bstrURL);
			if (bstrHeaders) SysFreeString(bstrHeaders);
			VariantClear(&vPostData);
			if (pWBApp) pWBApp->Release();
			return;
		}

		bstrURL = strUrl.AllocSysString();
		bstrHeaders = SysAllocString(L"Content-Type: application/x-www-form-urlencoded\r\n");
		if (!bstrURL || !bstrHeaders )
		{
			if (bstrURL) SysFreeString(bstrURL);
			if (bstrHeaders) SysFreeString(bstrHeaders);
			VariantClear(&vPostData);
			if (pWBApp) pWBApp->Release();
			return;
		}

		V_VT(&vHeaders) = VT_BSTR;
		V_BSTR(&vHeaders) = bstrHeaders;

		{
			LPSAFEARRAY psa;
			LPCTSTR cszPostData = strRawPostData;

			UINT cElems = lstrlen(cszPostData);
			LPSTR pPostData;

			if (!&vPostData)
			{
				return;// E_POINTER;
			}

			VariantInit(&vPostData);

			psa = SafeArrayCreateVector(VT_UI1, 0, cElems);
			if (!psa)
			{
				return;// E_OUTOFMEMORY;
			}

			hr = SafeArrayAccessData(psa, (LPVOID*)&pPostData);
			memcpy(pPostData, cszPostData, cElems);
			hr = SafeArrayUnaccessData(psa);

			V_VT(&vPostData) = VT_ARRAY | VT_UI1;
			V_ARRAY(&vPostData) = psa;
		}

		hr = pWBApp->Navigate(bstrURL, &vFlags,
		&vTargetFrameName, &vPostData, &vHeaders);
		pWBApp->put_Visible(VARIANT_TRUE);

		//Cleanup
		if (bstrURL) SysFreeString(bstrURL);
		if (bstrHeaders) SysFreeString(bstrHeaders);
		VariantClear(&vPostData);
		if (pWBApp) pWBApp->Release();
	}
}

void CTopsSearchDlg::PostFromBuiltInBrowser(CString strUrl, CString strRawPostData)
{
	/*m_pBrowserDlg->ShowWindow(SW_SHOW);

	COleVariant varUrl(strUrl);

	COleVariant varHeaders("Content-Type: application/x-www-form-urlencoded");
	
	// Fill the variant array
	COleSafeArray sa;
	BYTE* pData = NULL;
	sa.CreateOneDim(VT_UI1, strPostData.GetLength());
	sa.AccessData((LPVOID *)&pData);
	memcpy(pData, (LPCTSTR)strPostData, strPostData.GetLength());
	sa.UnaccessData();

	if (m_pBrowserDlg && m_pBrowserDlg->m_pBrowser) {
		m_pBrowserDlg->m_pBrowser->Navigate2(varUrl, NULL, NULL, sa, varHeaders);
	}
	*/
}

void CTopsSearchDlg::OnRequeryFinishedSearchList(short nFlags)
{
	try{
		//(e.lally 2009-10-08) PLID 35803 - update the count of results
		CString strRowCount;
		strRowCount.Format("Count: %li", m_pTopsSearchList->GetRowCount());
		SetDlgItemText(IDC_TOPS_SEARCH_COUNT, strRowCount);

	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnRequeryFinishedProviderFilter(short nFlags)
{
	try{
		//(e.lally 2009-10-08) PLID 35803 - Add a row for filtering on all providers
		IRowSettingsPtr pRow = m_pProviderFilter->GetNewRow();
		//(e.lally 2009-10-20) PLID 36001 - Added multi provider filtering
		if(pRow != NULL){
			pRow->PutValue(provColID, (long)cnMultiSelections);
			CString strMultiProvLabel = "<Multiple Providers>";
			//(e.lally 2009-11-25) PLID 36415 - Change our labeling for scheduler searches.
			if(m_radioAppointmentSearch.GetCheck()){
				strMultiProvLabel = "<Multiple Resources>";
			}
			pRow->PutValue(provColName, _bstr_t(strMultiProvLabel));
			m_pProviderFilter->AddRowBefore(pRow, m_pProviderFilter->GetFirstRow());
		}
		//All providers
		pRow = m_pProviderFilter->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(provColID, (long)cnAllValues);
			CString strAllProvLabel = "<All Providers>";
			//(e.lally 2009-11-25) PLID 36415 - Change our labeling for scheduler searches.
			if(m_radioAppointmentSearch.GetCheck()){
				strAllProvLabel = "<All Resources>";
			}
			pRow->PutValue(provColName, _bstr_t(strAllProvLabel));
			m_pProviderFilter->AddRowBefore(pRow, m_pProviderFilter->GetFirstRow());
			m_pProviderFilter->CurSel = pRow;

			m_arProviderFilterIDList.RemoveAll();
		}
		//(e.lally 2009-11-30) PLID 35803 - Make sure our controls are showing the correct state.
		EnsureControls();

	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnRequeryFinishedProcedureFilter(short nFlags)
{
	try{
		//(e.lally 2009-10-08) PLID 35803 - Add a row for filtering on all procedures
		IRowSettingsPtr pRow = m_pProcedureFilter->GetNewRow();
		//(e.lally 2009-10-20) PLID 36001 - Added multi procedure filtering
		if(pRow != NULL){
			pRow->PutValue(procColID, (long)cnMultiSelections);
			pRow->PutValue(procColName, "<Multiple Procedures>");
			m_pProcedureFilter->AddRowBefore(pRow, m_pProcedureFilter->GetFirstRow());
		}

		//All procedures
		pRow = m_pProcedureFilter->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(procColID, (long)cnAllValues);
			pRow->PutValue(procColName, "<All Procedures>");
			m_pProcedureFilter->AddRowBefore(pRow, m_pProcedureFilter->GetFirstRow());
			m_pProcedureFilter->CurSel = pRow;

			m_arProcFilterIDList.RemoveAll();
		}
		EnsureControls();
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnBnClickedTopsOnlyMasterProcedures()
{
	try{
		//(e.lally 2009-10-08) PLID 35803 - Update the procedure filter where clause and requery
		if(m_checkShowMasterProcs.GetCheck()){
			SetProcedureFilterWhere(TRUE);
		}
		else{
			SetProcedureFilterWhere(FALSE);
		}
		m_pProcedureFilter->Requery();
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();

	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::SetProcedureFilterWhere(BOOL bShowOnlyMasterProcedures)
{
	//(e.lally 2009-10-08) PLID 35803 - Switch the where clause to either filter on master procedure or all.
	if(bShowOnlyMasterProcedures){
		m_pProcedureFilter->WhereClause = "MasterProcedureID IS NULL";
	}
	else{
		m_pProcedureFilter->WhereClause = "";
	}
}

//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
void CTopsSearchDlg::OnSelChosenProviderFilter(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pProviderFilter->SetSelByColumn(provColID, (long)cnAllValues);
			EnsureControls();
			//(e.lally 2009-11-25) PLID 35803 - clear previous results
			ClearResults();
			return;
		}
		//Get the selected ID value
		long nProviderID = VarLong(pRow->GetValue(provColID), cnAllValues);
		if(nProviderID == cnMultiSelections){
			SelectMultiProviders();
		}
		else if(nProviderID != cnAllValues){
			//Just one specific provider
			m_arProviderFilterIDList.RemoveAll();
			m_arProviderFilterIDList.Add(nProviderID);
		}
		else{
			//all providers
			m_arProviderFilterIDList.RemoveAll();
		}
		
		EnsureControls();
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
void CTopsSearchDlg::OnSelChosenProcedureFilter(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pProcedureFilter->SetSelByColumn(procColID, (long)cnAllValues);
			EnsureControls();
			//(e.lally 2009-11-25) PLID 35803 - clear previous results
			ClearResults();
			return;
		}
		//Get the selected Procedure ID value
		long nProcedureID = VarLong(pRow->GetValue(procColID), cnAllValues);
		if(nProcedureID == cnMultiSelections){
			SelectMultiProcedures();
		}
		else if(nProcedureID != cnAllValues){
			//Just one specific procedure
			m_arProcFilterIDList.RemoveAll();
			m_arProcFilterIDList.Add(nProcedureID);
		}
		else{
			//all procedures
			m_arProcFilterIDList.RemoveAll();
		}
		
		EnsureControls();
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
void CTopsSearchDlg::SelectMultiProviders()
{
	CArray<long,long> arCurrentValue;
	arCurrentValue.Copy(m_arProviderFilterIDList);
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProvidersT");
	dlg.PreSelect(m_arProviderFilterIDList);
	CString strFrom = AsString(m_pProviderFilter->GetFromClause());
	CString strWhere = AsString(m_pProviderFilter->GetWhereClause());
	//(e.lally 2009-11-25) PLID 36415 - Update to handle billing and scheduler based searches.
	//The from clause actually gives the lists the same ID and value field names.
	CString strDescription = "Providers";
	if(m_radioAppointmentSearch.GetCheck()){
		strDescription = "Resources";
	}
	if(dlg.Open(strFrom, strWhere, "ID", "Name", strDescription) == IDOK) {
		dlg.FillArrayWithIDs(m_arProviderFilterIDList);
		CString strNames = dlg.GetMultiSelectString("; ");
		m_nxlMultiProviders.SetText(strNames);
		m_nxlMultiProviders.SetType(dtsHyperlink);
		//(e.lally 2009-11-25) PLID 35803 - clear results
		ClearResults();
	}
	else {
		m_arProviderFilterIDList.Copy(arCurrentValue);
	}
	m_nxlMultiProviders.Invalidate();

	if(m_arProviderFilterIDList.GetSize() == 1) {
		m_pProviderFilter->SetSelByColumn(provColID, m_arProviderFilterIDList[0]);
	}
	else if(m_arProviderFilterIDList.GetSize() == 0) {
		m_pProviderFilter->SetSelByColumn(provColID, (long)cnAllValues);
	}
}

//(e.lally 2009-10-20) PLID 36001 - Added multi selection filtering
void CTopsSearchDlg::SelectMultiProcedures()
{
	CArray<long,long> arCurrentValue;
	arCurrentValue.Copy(m_arProcFilterIDList);
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProcedureT");
	dlg.PreSelect(m_arProcFilterIDList);
	CString strFrom = AsString(m_pProcedureFilter->GetFromClause());
	CString strWhere = AsString(m_pProcedureFilter->GetWhereClause());
	if(dlg.Open(strFrom, strWhere, "ID", "Name", "Procedures") == IDOK) {
		dlg.FillArrayWithIDs(m_arProcFilterIDList);
		CString strNames = dlg.GetMultiSelectString("; ");
		m_nxlMultiProcedures.SetText(strNames);
		m_nxlMultiProcedures.SetType(dtsHyperlink);
		//(e.lally 2009-11-25) PLID 35803 - clear results
		ClearResults();
	}
	else {
		m_arProcFilterIDList.Copy(arCurrentValue);
	}
	m_nxlMultiProcedures.Invalidate();

	if(m_arProcFilterIDList.GetSize() == 1) {
		m_pProcedureFilter->SetSelByColumn(procColID, m_arProcFilterIDList[0]);
	}
	else if(m_arProcFilterIDList.GetSize() == 0) {
		m_pProcedureFilter->SetSelByColumn(procColID, (long)cnAllValues);
	}
}

//(e.lally 2009-10-20) PLID 36001 - Added for multi selection filtering
BOOL CTopsSearchDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try{
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		//(e.lally 2009-10-20) PLID 36001 - If our multi provider filter is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if(m_nxlMultiProviders.IsWindowVisible() && m_nxlMultiProviders.IsWindowEnabled()) {
			m_nxlMultiProviders.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		//(e.lally 2009-10-20) PLID 36001 - Now check the procedure filter
		if(m_nxlMultiProcedures.IsWindowVisible() && m_nxlMultiProcedures.IsWindowEnabled()) {
			m_nxlMultiProcedures.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// (f.dinatale 2010-09-14) - PLID 38192 - Added location filtering
		if(m_nxlMultiLocations.IsWindowVisible() && m_nxlMultiLocations.IsWindowEnabled()) {
			m_nxlMultiLocations.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		if(m_bNotifyOnce){ 
			m_bNotifyOnce = FALSE; 
			try{ throw;}NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

//(e.lally 2009-10-20) PLID 36001
LRESULT CTopsSearchDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_LABEL_TOPS_MULTI_PROV:
			SelectMultiProviders();
			EnsureControls();
			break;
		case IDC_LABEL_TOPS_MULTI_PROC:
			SelectMultiProcedures();
			EnsureControls();
			break;
		// (f.dinatale 2010-09-09) - PLID 38192
		case IDC_LABEL_TOPS_MULTI_LOC:
			SelectMultiLocations();
			EnsureControls();
			break;
		
		default:
			//Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	
	return 0;
}

// (j.jones 2009-11-03 15:07) - PLID 36137 - added submission filters
void CTopsSearchDlg::OnShowUnsubmitted()
{
	try {
		//(e.lally 2009-11-25) PLID 35803 - clear results
		ClearResults();
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnShowSubmitted()
{
	try {
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnShowAll()
{
	try {
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
	}NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-11-04) - PLID 36136
void CTopsSearchDlg::OnEnKillfocusTopsUsername()
{
	try {
		CString strUsername;
		GetDlgItemText(IDC_TOPS_USERNAME, strUsername);

		//Save it for future recall, per user
		SetRemotePropertyText("TOPSUsername", strUsername, 0, GetCurrentUserName());

	} NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnEnKillfocusTopsProxiedName()
{
	try {
		CString strProxiedName;
		GetDlgItemText(IDC_TOPS_PROXIED_NAME, strProxiedName);

		//Save it for future recall, per user
		SetRemotePropertyText("TOPSProxiedUser", strProxiedName, 0, GetCurrentUserName());

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-25) PLID 36415
void CTopsSearchDlg::OnClickedBillingSearch()
{
	try {
		//Save it - billing = 0
		SetRemotePropertyInt("TOPSLastSearchType", 0, 0, GetCurrentUserName());
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
		RequeryProviderList();
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-25) PLID 36415
void CTopsSearchDlg::OnClickedApptSearch()
{
	try {
		//Save it - appointment = 1
		SetRemotePropertyInt("TOPSLastSearchType", 1, 0, GetCurrentUserName());
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
		RequeryProviderList();
	} NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnChangeSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		if(!m_bDateFromDown) {
			EnsureValidSearchDateRange();
			//(e.lally 2009-11-25) PLID 35803 - clear previous results
			ClearResults();
		}
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnChangeSearchToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		if(!m_bDateToDown) {
			EnsureValidSearchDateRange();
			//(e.lally 2009-11-25) PLID 35803 - clear previous results
			ClearResults();
		}
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnDtnDropdownSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		m_bDateFromDown = TRUE;
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnDtnDropdownSearchToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		m_bDateToDown = TRUE;	
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnDtnCloseupSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		m_bDateFromDown = FALSE;
		EnsureValidSearchDateRange();
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::OnDtnCloseupSearchToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		m_bDateToDown = FALSE;
		EnsureValidSearchDateRange();
		//(e.lally 2009-11-25) PLID 35803 - clear previous results
		ClearResults();
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTopsSearchDlg::EnsureValidSearchDateRange()
{
	COleDateTime dtFrom, dtTo;
	dtFrom = COleDateTime(m_dtFrom.GetValue());
	dtTo = COleDateTime(m_dtTo.GetValue());
	if(dtFrom.GetStatus() == COleDateTime::invalid) {
		AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
		m_dtFrom.SetValue(_variant_t(dtTo));
	}
	else if(dtTo.GetStatus() == COleDateTime::invalid) {
		AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
		m_dtTo.SetValue(_variant_t(dtFrom));
	}
	else {
		//if dtFrom > dtTo, update dtTo
		if(dtFrom > dtTo) {
			dtTo = dtFrom;
			m_dtTo.SetValue(_variant_t(dtTo));
		}
	}
}

//(e.lally 2010-02-16) PLID 36849 - Link (procedural) cosmetic service codes to AMA CPT codes
void CTopsSearchDlg::OnBnClickedOptions()
{
	try{
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Configure Service Code Links...");

		//Now show the menu.
		CRect rc;
		m_btnTopsOptions.GetWindowRect(rc);
		//Since we are on the bottom right of the screen, reverse the alignment to be right aligned againt the left edge of the button
		int nCmdId = mnu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_RETURNCMD, rc.left, rc.top, this, NULL);	
		switch(nCmdId){
			case 1:
				ConfigureServiceCodeLinks();
				break;
		}
	
	}NxCatchAll(__FUNCTION__)
}

//(e.lally 2010-02-16) PLID 36849 - Opens the ImportAMACodesDlg with the new mode for linking AMA codes/descriptions with
	//their procedural billing service codes
void CTopsSearchDlg::ConfigureServiceCodeLinks()
{
	
		CImportAMACodesDlg dlg(this);
		dlg.SetType(3);//TOPS cpt codes
		dlg.DoModal();
}

// (f.dinatale 2010-09-08) - PLID 38192 - Function to requery the Location filter
void CTopsSearchDlg::RequeryLocationList()
{
	CString strLocationFrom;

	//(e.lally 2010-09-15) PLID 40532 - Use the report preference to filter out inactives from the dropdown display
	strLocationFrom = "(SELECT LocationsT.Name AS Name, LocationsT.ID AS ID "
		"FROM LocationsT WHERE TypeID = 1 ";
	if(!m_bFilterShowInactiveLocations){
		strLocationFrom += "AND Active = 1";
	}
	strLocationFrom += ") SubQ";

	m_pLocationFilter->FromClause = _bstr_t(strLocationFrom);
	m_pLocationFilter->Requery();
}


// (f.dinatale 2010-09-09) - PLID 38192
void CTopsSearchDlg::OnRequeryFinishedLocationList(short nFlags)
{
	try{
		IRowSettingsPtr pRow = m_pLocationFilter->GetNewRow();

		// Add an option for multiple locations
		if(pRow != NULL){
			pRow->PutValue(locaColID, (long)cnMultiSelections);
			pRow->PutValue(locaColName, "<Multiple Locations>");
			m_pLocationFilter->AddRowBefore(pRow, m_pLocationFilter->GetFirstRow());
		}

		pRow = m_pLocationFilter->GetNewRow();

		// Add an option for all locations
		if(pRow != NULL){
			pRow->PutValue(locaColID, (long)cnAllValues);
			pRow->PutValue(locaColName, "<All Locations>");
			m_pLocationFilter->AddRowBefore(pRow, m_pLocationFilter->GetFirstRow());
			m_pLocationFilter->CurSel = pRow;
		}
	} NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-09-09) - PLID 38192
void CTopsSearchDlg::OnSelChosenLocationFilter(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pLocationFilter->SetSelByColumn(locaColID, (long)cnAllValues);
			EnsureControls();
			ClearResults();
			return;
		}
		long nLocationID = VarLong(pRow->GetValue(locaColID), cnAllValues);
		if(nLocationID == cnMultiSelections){
			SelectMultiLocations();
		}
		else if(nLocationID != cnAllValues){
			m_arLocFilterIDList.RemoveAll();
			m_arLocFilterIDList.Add(nLocationID);
		}
		else{
			m_arLocFilterIDList.RemoveAll();
		}
		
		EnsureControls();
		ClearResults();

	}NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-09-09) - PLID 38192
void CTopsSearchDlg::SelectMultiLocations()
{
	CArray<long,long> arCurrentValue;
	arCurrentValue.Copy(m_arLocFilterIDList);
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "LocationsT");
	dlg.PreSelect(m_arLocFilterIDList);
	
	CString strFrom = AsString(m_pLocationFilter->FromClause);
	
	if(dlg.Open(strFrom, "", "ID", "Name", "Locations") == IDOK){
		dlg.FillArrayWithIDs(m_arLocFilterIDList);
		CString strNames = dlg.GetMultiSelectString("; ");
		m_nxlMultiLocations.SetText(strNames);
		m_nxlMultiLocations.SetType(dtsHyperlink);
		ClearResults();
	} else {
		m_arLocFilterIDList.Copy(arCurrentValue);
	}
	m_nxlMultiLocations.Invalidate();

	if(m_arLocFilterIDList.GetSize() == 1){
		m_pLocationFilter->SetSelByColumn(locaColID, m_arLocFilterIDList[0]);
	} else {
		if(m_arLocFilterIDList.GetSize() == 0){
			m_pLocationFilter->SetSelByColumn(provColID, (long)cnAllValues);
		}
	}
}

//(e.lally 2010-09-15) PLID 40532
void CTopsSearchDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		//(e.lally 2010-09-15) PLID 40532 - reload the report filter preferences, requery the filters if they changed
		BOOL bOldFilterShowInactiveProviders = m_bFilterShowInactiveProviders;
		BOOL bOldFilterShowInactiveLocations = m_bFilterShowInactiveLocations;

		m_bFilterShowInactiveProviders = GetRemotePropertyInt("ReportFilters_IncludeInactiveProviders", 0, 0, "<None>", true) == 1 ? TRUE : FALSE;
		m_bFilterShowInactiveLocations = GetRemotePropertyInt("ReportFilters_IncludeInactiveLocations", 0, 0, "<None>", true) == 1 ? TRUE : FALSE;

		//(e.lally 2010-09-15) PLID 40532 - Requery on table checker changes too.
		if (m_doctorChecker.Changed() || m_resourceChecker.Changed() || bOldFilterShowInactiveProviders != m_bFilterShowInactiveProviders) {
			RequeryProviderList();
		}

		if (m_locationChecker.Changed() || bOldFilterShowInactiveLocations != m_bFilterShowInactiveLocations) {
			RequeryLocationList();
		}
	} NxCatchAll(__FUNCTION__);
}