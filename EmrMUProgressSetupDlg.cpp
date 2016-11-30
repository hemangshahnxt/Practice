// EmrMuProgressSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrMUProgressSetupDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEmrMUProgressSetupDlg dialog
//(e.lally 2012-03-21) PLID 48264 - Created

enum MeasureListColumns
{
	mlcNumber = 0,
	mlcSelected,
	mlcDisplayName,
	mlcDescription,
};

enum ProviderListColumn
{
	plcID =0,
	plcName = 1,
};

enum DateOptionListColumns
{
	dolcNum = 0,
	dolcDescription,
};


IMPLEMENT_DYNAMIC(CEmrMUProgressSetupDlg, CNxDialog)

CEmrMUProgressSetupDlg::CEmrMUProgressSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrMUProgressSetupDlg::IDD, pParent), m_cchitReportListing((CWnd*)this)
{
	//(e.lally 2012-04-05) PLID 48264 - Initialize
	m_nInitialProviderID = -1;

}

void CEmrMUProgressSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_EMR_MU_START_DATE, m_dtStart);
}


BEGIN_MESSAGE_MAP(CEmrMUProgressSetupDlg, CNxDialog)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMR_MU_START_DATE, OnChangeStartDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_EMR_MU_START_DATE, OnCloseUpStartDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_EMR_MU_START_DATE, OnDropDownStartDate)
	ON_BN_CLICKED(IDC_EMR_MU_EXCLUDE_SEC_PROV, OnExcludeSecProvider)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrMUProgressSetupDlg, CNxDialog)
	ON_EVENT(CEmrMUProgressSetupDlg, IDC_EMR_MU_PROVIDER_LIST, 16, OnSelChosenProvider, VTS_DISPATCH)
	ON_EVENT(CEmrMUProgressSetupDlg, IDC_EMR_MU_DATE_OPTION_LIST, 16, OnSelChosenDateOption, VTS_DISPATCH)
	ON_EVENT(CEmrMUProgressSetupDlg, IDC_EMR_MU_MEASURES_LIST, 10, OnEditingFinishedMeasures, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrMUProgressSetupDlg, IDC_EMR_MU_PROVIDER_LIST, 18, OnRequeryFinishedProviders, VTS_I2)
END_EVENTSINK_MAP()


// CEmrMUProgressSetupDlg message handlers
BOOL CEmrMUProgressSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_bIsStartDateDroppedDown = false;


		m_pProviderList = BindNxDataList2Ctrl(IDC_EMR_MU_PROVIDER_LIST, GetRemoteDataSnapshot(), true);
		m_pMeasureList = BindNxDataList2Ctrl(IDC_EMR_MU_MEASURES_LIST, false);
		//(e.lally 2012-04-03) PLID 48264 - List of date options
		m_pDateOptionList = BindNxDataList2Ctrl(IDC_EMR_MU_DATE_OPTION_LIST, false);
		{
			//Load date filter options
			NXDATALIST2Lib::IRowSettingsPtr pRow;

			pRow = m_pDateOptionList->GetNewRow();
			pRow->PutValue(0, (long)mudvThisMonthToDate);
			pRow->PutValue(1, _variant_t("This Month To Date"));
			m_pDateOptionList->AddRowAtEnd(pRow, NULL);

			pRow = m_pDateOptionList->GetNewRow();
			pRow->PutValue(0, (long)mudvThisQuarterToDate);
			pRow->PutValue(1, _variant_t("This Quarter To Date"));
			m_pDateOptionList->AddRowAtEnd(pRow, NULL);

			pRow = m_pDateOptionList->GetNewRow();
			pRow->PutValue(0, (long)mudvThisYearToDate);
			pRow->PutValue(1, _variant_t("This Year To Date"));
			m_pDateOptionList->AddRowAtEnd(pRow, NULL);

			pRow = m_pDateOptionList->GetNewRow();
			pRow->PutValue(0, (long)mudvCustom);
			pRow->PutValue(1, _variant_t("Custom"));
			m_pDateOptionList->AddRowAtEnd(pRow, NULL);
		}

		m_cchitReportListing.LoadReportListing(MU::Stage1);

		{
			CCCHITReportInfo report;
			NXDATALIST2Lib::IRowSettingsPtr pRow;

			//(e.lally 2012-03-21) PLID 48264 - Loop through our report options and display them to the user
			for(int i =0; i< m_cchitReportListing.m_aryReports.GetSize(); i++){
				report = m_cchitReportListing.m_aryReports[i];
				if(report.GetReportType() == crtMU){
					pRow = m_pMeasureList->GetNewRow();

					pRow->PutValue(mlcNumber, report.m_nInternalID);
					pRow->PutValue(mlcSelected, VARIANT_FALSE);
					pRow->PutValue(mlcDisplayName, _bstr_t(report.m_strDisplayName));
					pRow->PutValue(mlcDescription, _bstr_t(report.GetHelpGeneral()));

					m_pMeasureList->AddRowSorted(pRow, NULL);
				}
			}
		}

		//(e.lally 2012-04-05) PLID 48264 - See if we were initialized with a provider.
		if(m_nInitialProviderID == -1){
			//(e.lally 2012-04-03) PLID 48264 - Load based on provider
			IRowSettingsPtr pRow = m_pProviderList->GetFirstRow();
			if(pRow != NULL){
				long nFirstProv = VarLong(pRow->GetValue(plcID));
				Load(nFirstProv);
			}
			else {
				EnsureControls();
			}
		}
		else {
			Load(m_nInitialProviderID);
			m_nInitialProviderID = -1;
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//(e.lally 2012-04-05) PLID 48264 - Flags which provider we should load when initializing the dialog
void CEmrMUProgressSetupDlg::SetInitialProviderID(long nProviderID)
{
	m_nInitialProviderID = nProviderID;
}

//(e.lally 2012-04-03) PLID 48264
long CEmrMUProgressSetupDlg::GetCurrentProviderID()
{
	IRowSettingsPtr pRow = m_pProviderList->GetCurSel();
	if(pRow == NULL){
		return -1;
	}
	return VarLong(pRow->GetValue(plcID), -1);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::Load(long nProviderID)
{
	//(e.lally 2012-04-03) PLID 48264 - Clear the checkboxes then reload
	ResetMeasureList();
	_RecordsetPtr rs = CreateParamRecordset("SELECT MeasureNum, Selected "
		"FROM ProviderMUMeasureSelectionT "
		"WHERE ProviderID = {INT} "
		"\r\n"
		"SELECT DateOptionNum, StartDate, ExcludeSecondaryProv "
		"FROM ProviderMUMeasureOptionT "
		"WHERE ProviderID = {INT} ", nProviderID, nProviderID);
	IRowSettingsPtr pRow = NULL;
	while(!rs->eof){

		long nMeasure = VarLong(rs->Fields->Item["MeasureNum"]->Value);
		BOOL bSelected = AsBool(rs->Fields->Item["Selected"]->Value);

		pRow = m_pMeasureList->FindByColumn(0 /*mlcNumber*/, nMeasure, m_pMeasureList->GetFirstRow(), VARIANT_FALSE);
		if(pRow != NULL){
			pRow->PutValue(mlcSelected, (bSelected == FALSE ? VARIANT_FALSE : VARIANT_TRUE));
		}

		rs->MoveNext();
	}

	//(e.lally 2012-04-03) PLID 48264 - Now get the provider options
	rs = rs->NextRecordset(NULL);
	long nDateOption = mudvThisYearToDate;
	COleDateTime dtStart = g_cdtInvalid;
	BOOL bExcludeSecondaryProv = FALSE;

	if(!rs->eof){
		nDateOption = VarLong(rs->Fields->Item["DateOptionNum"]->Value);
		dtStart = VarDateTime(rs->Fields->Item["StartDate"]->Value, g_cdtInvalid);
		bExcludeSecondaryProv = AsBool(rs->Fields->Item["ExcludeSecondaryProv"]->Value);
	}
	else {
		_variant_t varStartDate = g_cvarNull;
		if(dtStart.m_status == COleDateTime::valid && dtStart.m_dt > 0){
			varStartDate = dtStart;
		}
		ExecuteParamSql("INSERT INTO ProviderMUMeasureOptionT (ProviderID, DateOptionNum, StartDate, ExcludeSecondaryProv) "
			"VALUES({INT}, {INT}, {VT_DATE}, {BIT} ) ", nProviderID, nDateOption, varStartDate, bExcludeSecondaryProv);
	}
	rs->Close();

	m_pDateOptionList->SetSelByColumn(0, nDateOption);
	if(dtStart.m_dt > 0 && dtStart.m_status == COleDateTime::valid){
		m_dtStart.SetValue(dtStart);
	}
	else {
		//Invalid/null, so set to today
		COleDateTime dt = COleDateTime::GetCurrentTime();
		dtStart.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
		m_dtStart.SetValue(dtStart);
	}

	if(nDateOption == mudvCustom){
		GetDlgItem(IDC_EMR_MU_START_DATE)->ShowWindow(SW_SHOWNA);
	}
	else {
		GetDlgItem(IDC_EMR_MU_START_DATE)->ShowWindow(SW_HIDE);
	}


	CheckDlgButton(IDC_EMR_MU_EXCLUDE_SEC_PROV, bExcludeSecondaryProv);

	m_pProviderList->SetSelByColumn(plcID, nProviderID);

	EnsureControls();
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnSelChosenProvider(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//Default to the first one in the list
			pRow = m_pProviderList->GetFirstRow();
			m_pProviderList->PutCurSel(pRow);
			if(pRow != NULL){
				Load(VarLong(pRow->GetValue(plcID)));
			}
			else {
				EnsureControls();
			}
			return;
		}

		long nProviderID = VarLong(pRow->GetValue(0));
		Load(nProviderID);
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnSelChosenDateOption(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			return;
		}
		long nProviderID = GetCurrentProviderID();
		if(nProviderID == -1){
			return;
		}
		EMuDateValues eDateOption = (EMuDateValues)VarLong(pRow->GetValue(dolcNum));
		COleDateTime dtStart = m_dtStart.GetDateTime();
		_variant_t varStartDate = g_cvarNull;
		if(dtStart.m_dt > 0 && dtStart.m_status == COleDateTime::valid){
			varStartDate = dtStart;
		}
		else {
			//We're about to blank out the date, are you sure?
			ASSERT(FALSE);
		}

		if(eDateOption == mudvCustom){
			GetDlgItem(IDC_EMR_MU_START_DATE)->ShowWindow(SW_SHOWNA);
		}
		else {
			GetDlgItem(IDC_EMR_MU_START_DATE)->ShowWindow(SW_HIDE);
		}
		ExecuteParamSql("UPDATE ProviderMUMeasureOptionT SET DateOptionNum = {INT}, StartDate = {OLEDATETIME} WHERE ProviderID = {INT} ",
			eDateOption, dtStart, nProviderID);
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnEditingFinishedMeasures(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		if(bCommit == FALSE || lpRow == NULL){
			return;
		}
		switch(nCol) {
			case mlcSelected:
			{
				IRowSettingsPtr pRow(lpRow);
				if(pRow == NULL){
					return;
				}
				long nProviderID = GetCurrentProviderID();
				if(nProviderID == -1){
					return;
				}

				long nMeasure = VarLong(pRow->GetValue(mlcNumber));

				ExecuteParamSql(
				"IF NOT EXISTS("
					"SELECT ID FROM ProviderMUMeasureSelectionT "
					"WHERE ProviderID = {INT} AND MeasureNum = {INT}) \r\n"
				"BEGIN \r\n"
					"INSERT INTO ProviderMUMeasureSelectionT (ProviderID, MeasureNum, Selected) "
					"VALUES( {INT}, {INT}, {BIT} )\r\n"
				"END "
				"ELSE BEGIN \r\n"
					"UPDATE ProviderMUMeasureSelectionT SET Selected = {BIT} WHERE ProviderID = {INT} AND MeasureNum = {INT} \r\n"
				"END ", nProviderID, nMeasure,
				nProviderID, nMeasure, AsBool(varNewValue),
				AsBool(varNewValue), nProviderID, nMeasure);
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnChangeStartDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		if (!m_bIsStartDateDroppedDown) {
			//The date is not dropped down, so we can save
			SaveStartDate();
		}

		*pResult = 0;

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnCloseUpStartDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Let everyone be able to check and discover the date combo is not dropped down anymore
		m_bIsStartDateDroppedDown = false;

		SaveStartDate();

		*pResult = 0;
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnDropDownStartDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Let everyone be able to check and discover the date combo is dropped down now
		m_bIsStartDateDroppedDown = true;

		*pResult = 0;
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::SaveStartDate()
{
	long nProviderID = GetCurrentProviderID();
	if(nProviderID == -1){
		return;
	}
	COleDateTime dtNew = m_dtStart.GetDateTime();
	if(dtNew.m_dt <= 0 || dtNew.m_status != COleDateTime::valid){
		return;
	}

	ExecuteParamSql("UPDATE ProviderMUMeasureOptionT SET StartDate = {OLEDATETIME} WHERE ProviderID = {INT} ",
			dtNew, nProviderID);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::EnsureControls()
{
	BOOL bEnable = TRUE;
	if(GetCurrentProviderID() == -1){
		bEnable = FALSE;
	}
	GetDlgItem(IDC_EMR_MU_DATE_OPTION_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_EMR_MU_MEASURES_LIST)->EnableWindow(bEnable);
	m_dtStart.EnableWindow(bEnable);
	GetDlgItem(IDC_EMR_MU_EXCLUDE_SEC_PROV)->EnableWindow(bEnable);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::ResetMeasureList()
{
	IRowSettingsPtr pRow = m_pMeasureList->GetFirstRow();
	while(pRow != NULL){
		pRow->PutValue(mlcSelected, VARIANT_FALSE);
		pRow = pRow->GetNextRow();
	}
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnExcludeSecProvider()
{
	try {
		long nProviderID = GetCurrentProviderID();
		if(nProviderID == -1){
			return;
		}
		BOOL bExcludeSecondaryProv = IsDlgButtonChecked(IDC_EMR_MU_EXCLUDE_SEC_PROV) == FALSE ? FALSE : TRUE;

		ExecuteParamSql(
			"UPDATE ProviderMUMeasureOptionT SET ExcludeSecondaryProv = {BIT} WHERE ProviderID = {INT} \r\n"
			, bExcludeSecondaryProv, nProviderID);
				
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-03) PLID 48264
void CEmrMUProgressSetupDlg::OnRequeryFinishedProviders(short nFlags)
{
	try {
		long nProviderID = GetCurrentProviderID();
		if(nProviderID == -1){
			IRowSettingsPtr pRow = m_pProviderList->GetFirstRow();
			if(pRow != NULL){
				long nFirstProv = VarLong(pRow->GetValue(plcID));
				Load(nFirstProv);
			}
			else {
				EnsureControls();
			}
		}
	}NxCatchAll(__FUNCTION__);
}