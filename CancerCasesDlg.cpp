// CancerCasesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "CancerCasesDlg.h"

//TES 4/23/2014 - PLID 61854 - Created
// CCancerCasesDlg dialog

IMPLEMENT_DYNAMIC(CCancerCasesDlg, CNxDialog)

CCancerCasesDlg::CCancerCasesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCancerCasesDlg::IDD, pParent)
{

}

CCancerCasesDlg::~CCancerCasesDlg()
{
}

void CCancerCasesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPORT_SELECTED_CANCERCASES, m_nxbExportSelected);
	DDX_Control(pDX, IDC_FILTER_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_FILTER_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_DISPLAY_RESULTS, m_nxbDisplayResults);
	DDX_Control(pDX, IDC_RESET_FILTERS, m_nxbResetFilters);
	DDX_Control(pDX, IDC_VIEW_ALL_ERRORS, m_nxbViewAllErrors);
}


BEGIN_MESSAGE_MAP(CCancerCasesDlg, CNxDialog)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_DATE_FROM, &CCancerCasesDlg::OnDtnDatetimechangeFilterDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_DATE_TO, &CCancerCasesDlg::OnDtnDatetimechangeFilterDateTo)
	ON_BN_CLICKED(IDC_DISPLAY_RESULTS, &CCancerCasesDlg::OnBnClickedDisplayResults)
	ON_BN_CLICKED(IDC_RESET_FILTERS, &CCancerCasesDlg::OnBnClickedResetFilters)
	ON_BN_CLICKED(IDC_REMEMBER_COLUMN_WIDTHS_CANCERCASES, &CCancerCasesDlg::OnBnClickedRememberColumnWidthsCancercases)
	ON_BN_CLICKED(IDC_EXPORT_SELECTED_CANCERCASES, &CCancerCasesDlg::OnBnClickedExportSelectedCancercases)
	ON_BN_CLICKED(IDC_VIEW_ALL_ERRORS, &CCancerCasesDlg::OnBnClickedViewAllErrors)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;

enum FilterStatuses
{
	fsAll,
	fsExported,
	fsNotExported,
};

enum StatusFilterColumns
{
	sfcEnum = 0,
	sfcName = 1,
};

enum ProviderFilterColumns
{
	pfcID = 0,
	pfcName = 1,
};

enum SecondaryProviderFilterColumns
{
	spfcID = 0,
	spfcName = 1,
};

enum LocationFilterColumns
{
	lfcID = 0,
	lfcName = 1,
};

enum EmnListColumns
{
	elcPicID = 0,
	elcEmnID = 1,
	elcEmnTabChartID = 2,
	elcSend = 3,
	elcPersonID = 4,
	elcUserDefinedID = 5,
	elcPatientName = 6,
	elcProvider = 7,
	elcSecondaryProvider = 8,
	elcEmnDate = 9,
	elcEmnDescription = 10,
	elcLastExportedDate = 11,
	elcErrors = 12,
	elcErrorText = 13, //TES 5/2/2014 - PLID 61855
	elcDiagCodeID = 14, //TES 5/1/2014 - PLID 61916
	elcDiagCodeNumber = 15, //TES 5/1/2014 - PLID 61916
	elcDiagCodeDesc = 16, //TES 5/1/2014 - PLID 61916
	elcModifiedDate = 17, //TES 5/1/2014 - PLID 61916
};

// CCancerCasesDlg message handlers
BOOL CCancerCasesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_pEmnList = BindNxDataList2Ctrl(IDC_CANCER_CASES_LIST, false);
		
		//TES 4/24/2014 - PLID 61917 - Set up all the filter controls
		m_pProvCombo = GetDlgItemUnknown(IDC_PROVIDER_FILTER_COMBO);
		m_pProvCombo->PutAdoConnection(GetRemoteDataSnapshot());
		CString strLicensedProviders;
		CDWordArray dwaLicensedProviders;
		g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);
		for(int i = 0; i < dwaLicensedProviders.GetSize(); i++) {
			strLicensedProviders += FormatString("%i,",dwaLicensedProviders[i]);
		}
		strLicensedProviders.TrimRight(",");
		if(strLicensedProviders.IsEmpty()) strLicensedProviders = "-1";
		m_pProvCombo->WhereClause = _bstr_t("PersonT.Archived = 0 AND PersonT.ID IN (" + strLicensedProviders + ")");
		m_pProvCombo->Requery();

		m_pSecProvCombo = BindNxDataList2Ctrl(IDC_SEC_PROVIDER_FILTER_COMBO);
		
		m_pLocCombo = BindNxDataList2Ctrl(IDC_LOCATION_FILTER_COMBO);

		m_pStatusCombo = BindNxDataList2Ctrl(IDC_STATUS_FILTER_COMBO, false);
		IRowSettingsPtr pRow = m_pStatusCombo->GetNewRow();
		pRow->PutValue(sfcEnum, (long)fsAll);
		pRow->PutValue(sfcName, _bstr_t("<All Statuses>"));
		m_pStatusCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_pStatusCombo->GetNewRow();
		pRow->PutValue(sfcEnum, (long)fsExported);
		pRow->PutValue(sfcName, _bstr_t("<Exported>"));
		m_pStatusCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_pStatusCombo->GetNewRow();
		pRow->PutValue(sfcEnum, (long)fsNotExported);
		pRow->PutValue(sfcName, _bstr_t("<Not Exported>"));
		m_pStatusCombo->AddRowAtEnd(pRow, NULL);
		m_pStatusCombo->CurSel = pRow;

		pRow = m_pProvCombo->GetNewRow();
		pRow->PutValue(pfcID, (long)-1);
		pRow->PutValue(pfcName, _bstr_t("<All Providers>"));
		m_pProvCombo->AddRowBefore(pRow, m_pProvCombo->GetFirstRow());
		m_pProvCombo->CurSel = pRow;

		pRow = m_pSecProvCombo->GetNewRow();
		pRow->PutValue(spfcID, (long)-1);
		pRow->PutValue(spfcName, _bstr_t("<All Providers>"));
		m_pSecProvCombo->AddRowBefore(pRow, m_pSecProvCombo->GetFirstRow());
		m_pSecProvCombo->CurSel = pRow;

		pRow = m_pLocCombo->GetNewRow();
		pRow->PutValue(lfcID, (long)-1);
		pRow->PutValue(lfcName, _bstr_t("<All Locations>"));
		m_pLocCombo->AddRowBefore(pRow, m_pLocCombo->GetFirstRow());
		m_pLocCombo->CurSel = pRow;

		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		m_dtFrom.SetValue(COleDateTime(dtToday.GetYear(), 1, 1, 0, 0, 0));
		m_dtTo.SetValue(dtToday);

		m_nxbExportSelected.AutoSet(NXB_EXPORT);
		m_nxbDisplayResults.AutoSet(NXB_INSPECT);
		m_nxbResetFilters.AutoSet(NXB_REFRESH);
		//TES 5/19/2014 - PLID 62196 - Added View All Errors
		m_nxbViewAllErrors.AutoSet(NXB_INSPECT);

		// (j.gruber 2016-05-31 15:33) - NX-100777 - pulled from clause out 
		//made made changes to show ICD10s before ICD9's
		m_pEmnList->FromClause = " EmrMasterT INNER JOIN PersonT ON EmrMasterT.PatientID = PersonT.ID "
			" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" INNER JOIN EmrGroupsT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID "
			" INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
			" LEFT JOIN (SELECT PicID, Max(Date) AS LastDate FROM MailSent WHERE Selection = 'BITMAP:CANCERCASE' GROUP BY PicID) AS LastExportedDateQ ON PicT.ID = LastExportedDateQ.PicID "
			" INNER JOIN ("
			    " SELECT EMRDiagCodesT.EMRID, Min(diagCodes.ID) as DiagCodeID, "
				" RANK() OVER(Partition BY EMRID "
				" ORDER BY DiagCodes.ICD10 DESC) as Rank "
				" FROM EMRDiagCodesT INNER JOIN DiagCodes ON EmrDiagCodesT.DiagCodeID = DiagCodes.ID OR EmrDiagCodesT.DiagCodeID_ICD10 = DiagCodes.ID "
				" INNER JOIN CancerDiagCodesT ON DiagCodes.CodeNumber = CancerDiagCodesT.CodeNumber "
				" Where EmrDiagCodesT.Deleted = 0 "
				" GROUP BY EMRDiagCodesT.EMRID, DiagCodes.ICD10 "
			" ) AS CancerEmnsQ ON EmrMasterT.ID = CancerEmnsQ.EmrID AND CancerEmnsQ.Rank = 1 "
			" INNER JOIN DiagCodes ON CancerEmnsQ.DiagCodeID = DiagCodes.ID LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID ";		

		RefreshEmnList();

		g_propManager.CachePropertiesInBulk("CCancerCasesDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'CancerCasesRememberColumns' "
			")"
			, _Q(GetCurrentUserName()));
		g_propManager.CachePropertiesInBulk("CCancerCasesDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'CancerCasesColumnWidths' "
			"	,'CancerCaseExport_LastFolder' " //TES 5/1/2014 - PLID 61916
			")"
			, _Q(GetCurrentUserName()));

		//TES 4/25/2014 - PLID 61859 - Do we need to restore saved column widths?
		BOOL bRememberColumnWidths = GetRemotePropertyInt("CancerCasesRememberColumns", 0, 0, GetCurrentUserName(), true) != 0;
		CheckDlgButton(IDC_REMEMBER_COLUMN_WIDTHS_CANCERCASES, bRememberColumnWidths);
		if (bRememberColumnWidths) {
			//TES 4/25/2014 - PLID 61859 - Yes, we do!
			RestoreColumnWidths();
		}

	} NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CCancerCasesDlg::RefreshEmnList()
{
	// (j.jones 2015-12-22 14:48) - PLID 67720 - removed code that bizarrely filtered out locked EMNs
	CString strWhereClause = "EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL) AND PatientCreatedStatus <> 1";

	//TES 4/25/2014 - PLID 61917 - Apply all the filters
	//Provider
	if(m_pProvCombo->CurSel) {
		long nProvID = VarLong(m_pProvCombo->CurSel->GetValue(pfcID),-1);
		if(nProvID != -1) {
			strWhereClause += FormatString(" AND EmrMasterT.ID IN (SELECT EmrID FROM EmrProvidersT WHERE ProviderID = %li AND Deleted = 0) ", nProvID);
		}
	}
	else {
		ASSERT(FALSE);
	}

	//Sec. Provider
	if(m_pSecProvCombo->CurSel) {
		long nProvID = VarLong(m_pSecProvCombo->CurSel->GetValue(spfcID),-1);
		if(nProvID != -1) {
			strWhereClause += FormatString(" AND EmrMasterT.ID IN (SELECT EmrID FROM EmrSecondaryProvidersT WHERE ProviderID = %li AND Deleted = 0) ", nProvID);
		}
	}
	else {
		ASSERT(FALSE);
	}

	//Location
	if(m_pLocCombo->CurSel) {
		long nLocID = VarLong(m_pLocCombo->CurSel->GetValue(lfcID),-1);
		if(nLocID != -1) {
			strWhereClause += FormatString(" AND EmrMasterT.LocationID = %li ", nLocID);
		}
	}
	else {
		ASSERT(FALSE);
	}

	//Status
	if(m_pStatusCombo->CurSel) {
		FilterStatuses fsStatus = (FilterStatuses)VarLong(m_pStatusCombo->CurSel->GetValue(sfcEnum));
		//TES 5/2/2014 - PLID 61855 - We need to keep track of whether we're hiding exported rows, so we can properly update the list when exporting
		m_bHidingExported = false;
		switch(fsStatus) {
			case fsExported:
				strWhereClause += FormatString(" AND LastExportedDateQ.LastDate Is Not Null ");
				break;
			case fsNotExported:
				m_bHidingExported = true;
				strWhereClause += FormatString(" AND LastExportedDateQ.LastDate Is Null ");
				break;
		}
	}
	else {
		ASSERT(FALSE);
	}

	//Date Range
	COleDateTime dtFrom = m_dtFrom.GetDateTime();
	COleDateTime dtTo = m_dtTo.GetDateTime();
	dtTo += COleDateTimeSpan(1,0,0,0);
	strWhereClause += FormatString(" AND EmrMasterT.Date >= '%s' AND EmrMasterT.Date < '%s' ", 
		FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));

	m_pEmnList->WhereClause = _bstr_t(strWhereClause);

	m_pEmnList->Requery();

	//TES 4/25/2014 - PLID 61917 - Now disable the Display Results button, since the list is up to date
	m_nxbDisplayResults.EnableWindow(FALSE);
	//TES 5/19/2014 - PLID 62196 - Disable the View All Errors button, since any errors just got cleared out.
	m_nxbViewAllErrors.EnableWindow(FALSE);
}

BEGIN_EVENTSINK_MAP(CCancerCasesDlg, CNxDialog)
ON_EVENT(CCancerCasesDlg, IDC_PROVIDER_FILTER_COMBO, 1, CCancerCasesDlg::SelChangingProviderFilterCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_SEC_PROVIDER_FILTER_COMBO, 1, CCancerCasesDlg::SelChangingSecProviderFilterCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_LOCATION_FILTER_COMBO, 1, CCancerCasesDlg::SelChangingLocationFilterCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_STATUS_FILTER_COMBO, 1, CCancerCasesDlg::SelChangingStatusFilterCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_PROVIDER_FILTER_COMBO, 16, CCancerCasesDlg::SelChosenProviderFilterCombo, VTS_DISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_SEC_PROVIDER_FILTER_COMBO, 16, CCancerCasesDlg::SelChosenSecProviderFilterCombo, VTS_DISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_LOCATION_FILTER_COMBO, 16, CCancerCasesDlg::SelChosenLocationFilterCombo, VTS_DISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_STATUS_FILTER_COMBO, 16, CCancerCasesDlg::SelChosenStatusFilterCombo, VTS_DISPATCH)
ON_EVENT(CCancerCasesDlg, IDC_CANCER_CASES_LIST, 32, CCancerCasesDlg::ShowContextMenuCancerCasesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
ON_EVENT(CCancerCasesDlg, IDC_CANCER_CASES_LIST, 22, CCancerCasesDlg::ColumnSizingFinishedCancerCasesList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
ON_EVENT(CCancerCasesDlg, IDC_CANCER_CASES_LIST, 19, CCancerCasesDlg::LeftClickCancerCasesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CCancerCasesDlg::SelChangingProviderFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Don't let them select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChangingSecProviderFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Don't let them select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChangingLocationFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Don't let them select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChangingStatusFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Don't let them select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChosenProviderFilterCombo(LPDISPATCH lpRow)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChosenSecProviderFilterCombo(LPDISPATCH lpRow)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChosenLocationFilterCombo(LPDISPATCH lpRow)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SelChosenStatusFilterCombo(LPDISPATCH lpRow)
{
	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::OnDtnDatetimechangeFilterDateFrom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);

	*pResult = 0;
}

void CCancerCasesDlg::OnDtnDatetimechangeFilterDateTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	try {
		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);
	}NxCatchAll(__FUNCTION__);

	*pResult = 0;
}

void CCancerCasesDlg::OnBnClickedDisplayResults()
{
	try {
		//TES 4/25/2014 - PLID 61917 - Just call RefreshEmnList(), that will check all the filters and everything
		//TES 5/2/2014 - PLID 61855 - If there is error information in the list, it'll get lost in a requery, so double-check with the user.
		bool bErrorsInList = false;
		IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
		while (pRow && !bErrorsInList) {
			CString strText = VarString(pRow->GetValue(elcErrorText), "");
			if (!VarString(pRow->GetValue(elcErrorText),"").IsEmpty()) {
				bErrorsInList = true;
			}
			pRow = pRow->GetNextRow();
		}
		if (bErrorsInList) {
			if (IDYES != MsgBox(MB_YESNO, "There are errors or warnings for EMNs in the currently displayed list. Refreshing the list will clear out that information, are you sure you wish to continue?")) {
				return;
			}
		}
		RefreshEmnList();
	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::OnBnClickedResetFilters()
{
	try {
		//TES 4/25/2014 - PLID 61917 - Set all the filter controls to their defaults
		m_pProvCombo->SetSelByColumn(pfcID, (long)-1);
		m_pSecProvCombo->SetSelByColumn(spfcID, (long)-1);
		m_pLocCombo->SetSelByColumn(lfcID, (long)-1);
		m_pStatusCombo->SetSelByColumn(sfcEnum, (long)fsNotExported);
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		m_dtFrom.SetValue(COleDateTime(dtToday.GetYear(), 1, 1, 0, 0, 0));
		m_dtTo.SetValue(dtToday);

		//TES 4/25/2014 - PLID 61917 - Enable the Display Results button, as a visual indicator 
		// that the list is now out of date.
		m_nxbDisplayResults.EnableWindow(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::ShowContextMenuCancerCasesList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		//TES 4/25/2014 - PLID 61858 - Set the CurSel, just so they can be sure which row they clicked on
		m_pEmnList->CurSel = pRow;

		if(nCol == elcSend) {
			//TES 4/25/2014 - PLID 61858 - Give options to Check All/Uncheck All. First see if any are checked/unchecked
			bool bCheckedFound = false, bUncheckedFound = false;
			IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
			while(pRow && (!bCheckedFound || !bUncheckedFound)) {
				if(AsBool(pRow->GetValue(elcSend))) {
					bCheckedFound = true;
				}
				else {
					bUncheckedFound = true;
				}
				pRow = pRow->GetNextRow();
			}
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			long nCheckAll = 1;
			long nUncheckAll = 2;
			pMenu.InsertMenu(0, MF_BYPOSITION|(bUncheckedFound?MF_ENABLED:MF_DISABLED|MF_GRAYED), nCheckAll, "&Check All");
			pMenu.InsertMenu(1, MF_BYPOSITION|(bCheckedFound?MF_ENABLED:MF_DISABLED|MF_GRAYED), nUncheckAll, "&Uncheck All");

			CPoint pt;
			GetCursorPos(&pt);
			int nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
			if(nMenuCmd == nCheckAll || nMenuCmd == nUncheckAll) {
				_variant_t varCheck = (nMenuCmd == nCheckAll ? g_cvarTrue : g_cvarFalse);
				IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
				while(pRow) {
					pRow->PutValue(elcSend, varCheck);
					pRow = pRow->GetNextRow();
				}
			}
			pMenu.DestroyMenu();					

		}
		else {
			//TES 4/25/2014 - PLID 61858 - All other columns have two options: Go To Patient and Go To EMN
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			long nGoToPatient = 1;
			long nGoToEmn = 2;
			//TES 5/7/2015 - PLID 65968 - Add an Export option
			long nExport = 3;
			pMenu.InsertMenu(0, MF_BYPOSITION|MF_ENABLED, nGoToPatient, "Go To &Patient");
			pMenu.InsertMenu(1, MF_BYPOSITION|MF_ENABLED, nGoToEmn, "Go To &EMN");
			pMenu.InsertMenu(2, MF_BYPOSITION|MF_ENABLED, nExport, "E&xport");
			CPoint pt;
			GetCursorPos(&pt);
			int nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
			if(nMenuCmd == nGoToPatient) {
				//TES 4/25/2014 - PLID 61858 - Set the active patient, flip to the Patients module
				long nPatientID = VarLong(pRow->GetValue(elcPersonID));
				CMainFrame *pMainFrame;
				pMainFrame = GetMainFrame();
				if(pMainFrame != NULL) {

					if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
						if(IDNO == MessageBox("This patient is not in the current lookup. \n"
							"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							return;
						}
					}
					
					if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
						pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					}
				}
				else {
					ThrowNxException("Could not open MainFrame");
				}

			}
			else if(nMenuCmd == nGoToEmn) {
				//TES 4/25/2014 - PLID 61858 - Don't let them open an EMN in a chart they don't have access to
				long nChartID = VarLong(pRow->GetValue(elcEmnTabChartID),-1);
				if(nChartID != -1) {
					if(!CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, nChartID)) {
						return;
					}
				}
				GetMainFrame()->EditEmrRecord(VarLong(pRow->GetValue(elcPicID)), VarLong(pRow->GetValue(elcEmnID)));

			}
			else if (nMenuCmd == nExport) {
				//TES 5/7/2015 - PLID 65968 - If they choose to export, call the function and pass in the ID
				ExportCancerCases(VarLong(pRow->GetValue(elcEmnID)));
			}
			pMenu.DestroyMenu();
		}

	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::OnBnClickedRememberColumnWidthsCancercases()
{
	try {
		//TES 4/25/2014 - PLID 61859 - Set the property.
		BOOL bRememberColumnWidths = IsDlgButtonChecked(IDC_REMEMBER_COLUMN_WIDTHS_CANCERCASES) != 0;
		SetRemotePropertyInt("CancerCasesRememberColumns", bRememberColumnWidths ? 1 : 0, 0, GetCurrentUserName());

		if (bRememberColumnWidths) {
			//TES 4/25/2014 - PLID 61859 - They want to remember, so go ahead and save the widths to data.
			SaveColumnWidths();
		}

	}NxCatchAll(__FUNCTION__);
}

void CCancerCasesDlg::SaveColumnWidths()
{
	//TES 4/25/2014 - PLID 61859 - Code based off of CLabResultsDlg::SaveColumnWidths()
	CString strColumnWidths;
	if (!IsDlgButtonChecked(IDC_REMEMBER_COLUMN_WIDTHS_CANCERCASES)) {
		return;
	}

	//TES 4/25/2014 - PLID 61859 - Start with the version of the column widths; we're still on Version 1
	strColumnWidths += "V1,";

	// Store the columns in a xx,xx,xx,xx format
	for (int i = 0; i < m_pEmnList->ColumnCount; i++)
	{
		IColumnSettingsPtr pCol = m_pEmnList->GetColumn(i);
		CString str;		
		
		str.Format("%d", pCol->StoredWidth);
		
		if (i > 0)
			strColumnWidths += ",";

		strColumnWidths += str;
	}

	SetRemotePropertyText("CancerCasesColumnWidths", strColumnWidths, 0, GetCurrentUserName());
}

void CCancerCasesDlg::RestoreColumnWidths()
{
	//TES 4/25/2014 - PLID 61859 - Code based off of CLabResultsDlg::RestoreColumnWidths()
	CString strColumnWidths = GetRemotePropertyText("CancerCasesColumnWidths", "", 0, GetCurrentUserName(), false);

	CArray<int, int> arWidths;
	//TES 4/25/2014 - PLID 61859 - Start the column widths with the version (prefixed with V).
	int nVersion = 1;
	int tokIndex = strColumnWidths.Find(',');
	if(strColumnWidths.GetAt(0) == 'V') {
		nVersion = atol(strColumnWidths.Mid(1,tokIndex-1));
		strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
		tokIndex = strColumnWidths.Find(',');
	}

	if (tokIndex == -1) {
		//TES 4/25/2014 - PLID 61859 - It is empty or invalid, so rebuild
		SaveColumnWidths();
		return;
	}

	int nColIndex = 0;
	while(tokIndex != -1) {
		CString str = strColumnWidths.Left(tokIndex);
		arWidths.Add(atoi(str));
		strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
		tokIndex = strColumnWidths.Find(',');
		nColIndex++;
	}
	arWidths.Add(atoi(strColumnWidths));

	if (arWidths.GetSize() != m_pEmnList->ColumnCount) {
		//TES 4/25/2014 - PLID 61859 - We have invalid data, so save the current widths.
		SaveColumnWidths();
		return;
	}

	for (int i = 0; i < m_pEmnList->ColumnCount; i++)
	{
		//TES 4/25/2014 - PLID 61859 - Now go through the columns, clear any that are set as Auto or Data, and set their saved width.
		IColumnSettingsPtr pCol = m_pEmnList->GetColumn(i);
		long nStyle = pCol->ColumnStyle;
		nStyle = (nStyle&(~csWidthAuto)&(~csWidthData));
		pCol->ColumnStyle = nStyle;
		pCol->StoredWidth = arWidths[i];
	}
}

void CCancerCasesDlg::ColumnSizingFinishedCancerCasesList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		//TES 4/25/2014 - PLID 61859 - Save the new column size if appropriate
		if(!bCommitted) {
			return;
		}
		if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMN_WIDTHS_CANCERCASES)) {
			return;
		}

		SaveColumnWidths();

	}NxCatchAll(__FUNCTION__);
}


void CCancerCasesDlg::OnBnClickedExportSelectedCancercases()
{
	try {
		//TES 5/7/2015 - PLID 65968 - Split the code into a separate function
		ExportCancerCases();
	}NxCatchAll(__FUNCTION__);
}

//TES 5/7/2015 - PLID 65968 - Split out the export code into a separate function
void CCancerCasesDlg::ExportCancerCases(long nOverrideEmnID /*= -1*/)
{
	//TES 5/1/2014 - PLID 61916 - First, make sure at least one row is checked.
	bool bOneRowChecked = false;
	//TES 5/7/2015 - PLID 65968 - If we have an override EMN, we don't check the checkboxes
	if (nOverrideEmnID != -1) {
		bOneRowChecked = true;
	}
	else {
		IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
		while (pRow && !bOneRowChecked) {
			if (AsBool(pRow->GetValue(elcSend))) {
				bOneRowChecked = true;
			}
			pRow = pRow->GetNextRow();
		}
	}
	if (!bOneRowChecked) {
		MsgBox("Please select one or more EMNs to generate cancer case submissions for.");
		return;
	}

	//TES 5/1/2014 - PLID 61916 - Prompt them to select a folder to export to
	CString strCancerCaseFolder;
	CString strLastExportFolder = GetRemotePropertyMemo("CancerCaseExport_LastFolder", "", 0, GetCurrentUserName());
	if (strLastExportFolder.IsEmpty() || !DoesExist(strLastExportFolder)) {
		strLastExportFolder = GetSharedPath() ^ "CancerCases";
		FileUtils::EnsureDirectory(strLastExportFolder);
	}
	if (!BrowseToFolder(&strCancerCaseFolder, "Select Cancer Case export folder (files will also be saved in individual patient histories)", GetSafeHwnd(), NULL, strLastExportFolder)) {
		return;
	}
	SetRemotePropertyMemo("CancerCaseExport_LastFolder", strCancerCaseFolder, 0, GetCurrentUserName());

	//TES 5/1/2014 - PLID 61916 - Go through each selected row, call the utility function to create the cancer case document
	IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
	//TES 5/2/2014 - PLID 61855 - Track success vs. warning vs. error, and also track any rows we need to remove (because they were successfully exported)
	bool bOneSucceeded = false, bOneWarningFound = false, bOneErrorFound = false;
	CArray<IRowSettingsPtr, IRowSettingsPtr&> arRowsToRemove;
	while (pRow) {
		//TES 5/7/2015 - PLID 65968 - if we have an nOverrideEmnID, export if this is it, otherwise export if it's checked.
		if ((nOverrideEmnID == -1 && AsBool(pRow->GetValue(elcSend))) || (nOverrideEmnID != -1 && VarLong(pRow->GetValue(elcEmnID)) == nOverrideEmnID)) {
			CancerCaseFailureInfo ccfi;
			//TES 5/15/2015 - PLID 65968 - If we have an nOverrideEmnID, pass in a message parent, and not the CancerCaseFailureInfo
			// (r.goldschmidt 2016-06-02 17:08) - NX-100799 - fix variable that is passed as the diagnosis code
			CString strFilePath = CreateNewCancerCaseDocument(VarLong(pRow->GetValue(elcPersonID)), VarLong(pRow->GetValue(elcPicID)), VarLong(pRow->GetValue(elcEmnID)), VarLong(pRow->GetValue(elcDiagCodeID)),
				VarString(pRow->GetValue(elcDiagCodeNumber)), VarString(pRow->GetValue(elcDiagCodeDesc)), VarDateTime(pRow->GetValue(elcModifiedDate)), VarDateTime(pRow->GetValue(elcEmnDate)), nOverrideEmnID == -1 ? NULL : this, &ccfi,
				VarString(pRow->GetValue(elcPatientName)) + " - " + VarString(pRow->GetValue(elcEmnDescription)));
			if (!strFilePath.IsEmpty()) {
				//get the patient demographic information so we know what to replace
				CString strFirst, strLast;
				long nUserDefinedID;
				GetPatientHistoryDemographicInfo(GetRemoteData(), VarLong(pRow->GetValue(elcPersonID)), &strFirst, &strLast, &nUserDefinedID);
				CString strReplaceValue = strLast + "_" + strFirst + "_";
				CString strFileName = strFilePath.Mid(strFilePath.ReverseFind('\\') + 1);
				strFileName.Replace(strReplaceValue, "");
				CopyFile(strFilePath, strCancerCaseFolder ^ strFileName, TRUE);
				//TES 5/2/2014 - PLID 61855 - Note that we've succeeded, and either flag the row for removal, or update its last exported date
				bOneSucceeded = true;
				if (m_bHidingExported) {
					arRowsToRemove.Add(pRow);
				}
				else {
					pRow->PutValue(elcLastExportedDate, _variant_t(COleDateTime::GetCurrentTime(), VT_DATE));
					pRow->PutValue(elcSend, g_cvarFalse);
					//TES 5/29/2014 - PLID 61855 - Clear out the error fields
					pRow->PutCellLinkStyle(elcErrors, dlLinkStyleFalse);
					pRow->PutCellBackColor(elcErrors, RGB(255, 255, 255));
					pRow->PutValue(elcErrors, g_cvarNull);
					pRow->PutValue(elcErrorText, g_cvarNull);
				}
			}
			else {
				//TES 5/2/2014 - PLID 61855 - Fill in the error columns for this row
				if (ccfi.ccft == ccftError) {
					pRow->PutCellLinkStyle(elcErrors, dlLinkStyleTrue);
					pRow->PutValue(elcErrors, _bstr_t("Error Encountered"));
					pRow->PutCellBackColor(elcErrors, RGB(255, 0, 0));
					pRow->PutValue(elcErrorText, _bstr_t(ccfi.strDisplayText));
					pRow->PutValue(elcSend, g_cvarFalse);
					bOneErrorFound = true;
				}
				else if (ccfi.ccft == ccftWarning) {
					pRow->PutCellLinkStyle(elcErrors, dlLinkStyleTrue);
					pRow->PutValue(elcErrors, _bstr_t("Warning Encountered"));
					pRow->PutCellBackColor(elcErrors, RGB(255, 255, 0));
					pRow->PutValue(elcErrorText, _bstr_t(ccfi.strDisplayText));
					pRow->PutValue(elcSend, g_cvarFalse);
					bOneWarningFound = true;
				}
			}
		}
		pRow = pRow->GetNextRow();
	}
	//TES 5/2/2014 - PLID 61855 - Now remove all the rows we flagged
	for (int i = 0; i < arRowsToRemove.GetSize(); i++) {
		m_pEmnList->RemoveRow(arRowsToRemove[i]);
	}

	//TES 5/2/2014 - PLID 61855 - Give different messages based on the results of the export
	CString strMessage;
	if (bOneErrorFound || bOneWarningFound) {
		//TES 5/19/2014 - PLID 61855 = Reworked the message to indicate if some cases succeeded.
		if (bOneSucceeded) {
			strMessage = "Some Cancer Cases were successfully exported, but at least one error or warning was encountered while generating other Cancer Case submissions. You can find the details in the error column.";
		}
		else {
			//TES 5/7/2015 - PLID 65968 - If we have an override, use singular grammar
			if (nOverrideEmnID == -1) {
				strMessage = "At least one error or warning was encountered while generating the Cancer Case submissions. You can find the details in the error column.";
			}
			else {
				strMessage = "At least one error or warning was encountered while generating the Cancer Case submission. You can find the details in the error column.";
			}
		}
		//TES 5/19/2014 - PLID 62196 - Enable the View All Errors button
		m_nxbViewAllErrors.EnableWindow(TRUE);
	}
	else {
		//TES 5/7/2015 - PLID 65968 - If we have an override, use singular grammar
		if (nOverrideEmnID == -1) {
			strMessage = "All cancer case submissions have been generated.";
		}
		else {
			strMessage = "The cancer case submission has been generated.";
		}
	}
	MsgBox("%s", strMessage);

}

void CCancerCasesDlg::LeftClickCancerCasesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		//TES 5/2/2014 - PLID 61855 - If this is a row with an error or warning, display it to the user
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}
		if (nCol == elcErrors) {
			CString strErrorText = VarString(pRow->GetValue(elcErrorText), "");
			if (!strErrorText.IsEmpty()) {
				MsgBox("The following issue(s) were reported while exporting this Cancer Case:\r\n%s", strErrorText);
			}
		}
	}NxCatchAll(__FUNCTION__);
}


void CCancerCasesDlg::OnBnClickedViewAllErrors()
{
	try {
		//TES 5/19/2014 - PLID 62196 - Go through all the rows that have error text, and compile them into one string.
		CString strErrors;
		IRowSettingsPtr pRow = m_pEmnList->GetFirstRow();
		while (pRow) {
			CString strErrorText = VarString(pRow->GetValue(elcErrorText), "");
			if (!strErrorText.IsEmpty()) {
				//TES 5/19/2014 - PLID 62196 - Identify the row, then add the error text
				CString strEmnInfo;
				strEmnInfo.Format("Patient: %s (%s); EMN: %s - %s", AsString(pRow->GetValue(elcPatientName)), AsString(pRow->GetValue(elcUserDefinedID)),
					AsString(pRow->GetValue(elcEmnDate)), AsString(pRow->GetValue(elcEmnDescription)));
				strErrors += strEmnInfo + "\r\nErrors:\r\n" + strErrorText + "\r\n\r\n";
			}
			pRow = pRow->GetNextRow();
		}
		//TES 5/19/2014 - PLID 62196 - OK, now write that to a .txt file, and open it in Notepad.
		CStdioFile fOut;
		CString strFileName;
		strFileName.Format("CancerCaseErrors%s.txt", COleDateTime::GetCurrentTime().Format("%Y%m%d_%H%M"));
		if (fOut.Open(GetPracPath(PracPath::SessionPath) ^ strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
			fOut.WriteString(strErrors);
			fOut.Close();

			int nResult = (int)ShellExecute((HWND)this, NULL, "notepad.exe", (CString("'") + GetPracPath(PracPath::SessionPath) ^ strFileName + CString("'")), NULL, SW_SHOW);
		}
		else {
			ThrowNxException("Failed to create output file " + GetPracPath(PracPath::SessionPath) ^ strFileName);
		}
	}NxCatchAll(__FUNCTION__);
}
