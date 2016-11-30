// EMRAnalysisConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRAnalysisConfigDlg.h"
#include "EMRAnalysisDlg.h"
#include "InternationalUtils.h"

// (j.jones 2008-10-14 16:31) - PLID 31692 - created

// CEMRAnalysisConfigDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum TemplateComboColumns
{
	tccID = 0,
	tccName,
};

enum ItemComboColumns
{
	iccID = 0,
	iccName,
	iccDataType,
	iccDataTypeName,
	iccColor,
};

enum ItemListColumns
{
	iloID = 0,
	iloEMRInfoMasterID,
	iloEMRInfoName,
	iloEMRInfoType,	// (j.jones 2009-04-09 16:13) - PLID 33947
	iloEMRInfoTypeName,
	iloDataOperator,
	iloDataFilter,
};

#define START_EDITING_FILTER_LIST	42042

CEMRAnalysisConfigDlg::CEMRAnalysisConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRAnalysisConfigDlg::IDD, pParent)
{
	m_nID = -1;
	m_nSelTemplateID = -1;
}


void CEMRAnalysisConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_ANALYSIS_CONFIG_BKG, m_bkg);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_ALL_EMN_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_EMN_DATE, m_radioEMNDate);
	DDX_Control(pDX, IDC_RADIO_ALL_EMR_ITEMS, m_radioAllItems);
	DDX_Control(pDX, IDC_RADIO_ANY_EMR_ITEMS, m_radioAnyItems);	
	DDX_Control(pDX, IDC_RADIO_FILTER_ON_PATIENT, m_radioFilterOnPatient);
	DDX_Control(pDX, IDC_RADIO_FILTER_ON_EMN, m_radioFilterOnEMN);
	DDX_Control(pDX, IDC_RADIO_FILTER_ON_EMR, m_radioFilterOnEMR);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_PATIENT, m_radioGroupByPatient);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_DATE, m_radioGroupByDate);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_EMN, m_radioGroupByEMN);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_EMR, m_radioGroupByEMR);
	DDX_Control(pDX, IDC_EMN_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_EMN_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_EDIT_CONFIG_NAME, m_editDescription);
	DDX_Control(pDX, IDC_ANY_ALL_LABEL, m_nxstaticAnyAll);	
	DDX_Control(pDX, IDC_RADIO_ONE_COL_PER_ITEM, m_radioColumnGroupByItem);
	DDX_Control(pDX, IDC_RADIO_MINIMAL_COLUMNS,	m_radioColumnGroupByCondensed);
	DDX_Control(pDX, IDC_CHECK_FILTER_BY_SELECTED_TEMPLATE, m_checkFilterOnSelectedTemplate);
	DDX_Control(pDX, IDC_CHECK_SHOW_SPAWNING_INFO, m_checkShowSpawningInfo);
}


BEGIN_MESSAGE_MAP(CEMRAnalysisConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_RADIO_ALL_EMN_DATES, OnRadioAllEmnDates)
	ON_BN_CLICKED(IDC_RADIO_EMN_DATE, OnRadioEmnDate)
	ON_MESSAGE(START_EDITING_FILTER_LIST, OnStartEditingFilterList)
	ON_BN_CLICKED(IDC_RADIO_FILTER_ON_PATIENT, OnRadioFilterOnPatient)
	ON_BN_CLICKED(IDC_RADIO_FILTER_ON_EMN, OnRadioFilterOnEmn)
	ON_BN_CLICKED(IDC_RADIO_FILTER_ON_EMR, OnRadioFilterOnEmr)
	ON_BN_CLICKED(IDC_RADIO_ALL_EMR_ITEMS, OnRadioAllEmrItems)
	ON_BN_CLICKED(IDC_RADIO_ANY_EMR_ITEMS, OnRadioAnyEmrItems)
END_MESSAGE_MAP()


// CEMRAnalysisConfigDlg message handlers

BOOL CEMRAnalysisConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		//always set to the patient color
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_editDescription.SetLimitText(255);

		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		m_TemplateCombo = BindNxDataList2Ctrl(IDC_EMR_TEMPLATE_COMBO, GetRemoteDataSnapshot(), true);
		m_ItemCombo = BindNxDataList2Ctrl(IDC_EMR_INFO_ITEM_COMBO, GetRemoteDataSnapshot(), false);
		m_ItemList = BindNxDataList2Ctrl(IDC_EMR_ANALYSIS_CONFIG_LIST, GetRemoteDataSnapshot(), false);

		// (j.jones 2010-05-03 16:24) - PLID 38464 - the From clause for this is now
		// in the cpp file for easier readability
		// (j.jones 2010-06-04 16:55) - PLID 39029 - do not show the generic table (DataSubType = 3)
		CString strFrom;
		strFrom.Format("(SELECT EMRInfoMasterT.ID AS EMRInfoMasterID, "
				"EmrInfoT.Name, EmrInfoT.DataType, EmrInfoT.DataSubType "
				"FROM EmrInfoT WITH(NOLOCK) "
				"INNER JOIN EmrInfoMasterT WITH(NOLOCK) ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
				"WHERE EMRInfoMasterT.Inactive = 0 "
				"AND EMRInfoT.DataSubType <> %li "
				") AS ItemListQ", eistGenericTable);
		m_ItemCombo->PutFromClause(_bstr_t(strFrom));
		
		//update the color field for allergies and medications
		{
			CString strColorField;
			strColorField.Format("CASE WHEN DataSubType = %li OR DataSubType = %li THEN Convert(int, %li) ELSE 0 END",
				eistCurrentMedicationsTable, eistAllergiesTable, (long)RGB(0,0,255));
			IColumnSettingsPtr pColorCol = m_ItemCombo->GetColumn(iccColor);
			pColorCol->PutFieldName((LPCTSTR)strColorField);
			m_ItemCombo->Requery();
		}

		//add an "all" option to the template combo
		{
			IRowSettingsPtr pNewRow = m_TemplateCombo->GetNewRow();
			pNewRow->PutValue(tccID, (long)-1);
			pNewRow->PutValue(tccName, " {All Templates}");
			m_TemplateCombo->AddRowSorted(pNewRow, NULL);
		}

		if(m_nID == -1) {
			//initialize defaults

			m_dtFrom.SetValue(COleDateTime::GetCurrentTime());
			m_dtTo.SetValue(COleDateTime::GetCurrentTime());
			m_radioAllDates.SetCheck(TRUE);
			OnRadioEmnDate();

			m_radioFilterOnPatient.SetCheck(TRUE);
			OnRadioFilterOnPatient();
			m_radioAllItems.SetCheck(TRUE);
			OnRadioAllEmrItems();
			m_radioGroupByPatient.SetCheck(TRUE);

			// (c.haag 2009-02-25 09:20) - PLID 33187 - Default column grouping
			m_radioColumnGroupByCondensed.SetCheck(TRUE);

			// (j.jones 2009-03-30 10:32) - PLID 33703 - disable this filter until a template is selected
			m_checkFilterOnSelectedTemplate.EnableWindow(FALSE);
		}
		else {
			//load from data
			Load();
		}

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRAnalysisConfigDlg::Load()
{
	try {

		CWaitCursor pWait;

		if(m_nID == -1) {
			ASSERT(FALSE);
			ThrowNxException("Attempted to load an EMR Analysis Configuration with no ID!");
			return;
		}

		//load the configuration settings
		// (c.haag 2009-02-24 11:16) - PLID 33187 - Added ColumnGroupBy
		// (j.jones 2009-03-30 09:48) - PLID 33703 - Added FilterByTemplateID
		// (j.jones 2009-04-09 14:24) - PLID 33916 - added IncludeSpawningInfo
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Name, UseDateFilter, "
			"DateFrom, DateTo, FilterOnAllItems, RecordFilter, DisplayGroupBy, "
			"ColumnGroupBy, FilterByTemplateID, IncludeSpawningInfo "
			"FROM EMRAnalysisConfigT "
			"WHERE EMRAnalysisConfigT.ID = {INT}", m_nID);
		if(!rs->eof) {
			CString strName = AdoFldString(rs, "Name", "");
			m_editDescription.SetWindowText(strName);

			BOOL bUseDateFilter = AdoFldBool(rs, "UseDateFilter", FALSE);
			if(bUseDateFilter) {
				m_dtFrom.SetValue(rs->Fields->Item["DateFrom"]->Value);
				m_dtTo.SetValue(rs->Fields->Item["DateTo"]->Value);
			}
			m_radioAllDates.SetCheck(!bUseDateFilter);
			m_radioEMNDate.SetCheck(bUseDateFilter);
			OnRadioEmnDate();

			EMRAnalysisRecordFilterType eRecordFilter = (EMRAnalysisRecordFilterType)AdoFldLong(rs, "RecordFilter", (long)earftPatient);
			m_radioFilterOnPatient.SetCheck(eRecordFilter == earftPatient);
			m_radioFilterOnEMN.SetCheck(eRecordFilter == earftEMN);
			m_radioFilterOnEMR.SetCheck(eRecordFilter == earftEMR);
			OnRadioFilterOnPatient();

			BOOL bFilterOnAllItems = AdoFldBool(rs, "FilterOnAllItems", FALSE);
			m_radioAllItems.SetCheck(bFilterOnAllItems);
			m_radioAnyItems.SetCheck(!bFilterOnAllItems);
			OnRadioAllEmrItems();

			EMRAnalysisGroupByType eDisplayGroupBy = (EMRAnalysisGroupByType)AdoFldLong(rs, "DisplayGroupBy", (long)eagbtPatient);
			m_radioGroupByPatient.SetCheck(eDisplayGroupBy == eagbtPatient);
			m_radioGroupByDate.SetCheck(eDisplayGroupBy == eagbtDate);
			m_radioGroupByEMN.SetCheck(eDisplayGroupBy == eagbtEMN);
			m_radioGroupByEMR.SetCheck(eDisplayGroupBy == eagbtEMR);

			// (c.haag 2009-02-24 11:27) - PLID 33187 - Load ColumnGroupBy
			EMRAnalysisColumnGroupByType eColumnGroupBy = (EMRAnalysisColumnGroupByType)AdoFldLong(rs, "ColumnGroupBy", (long)eacgbtCondensed);
			m_radioColumnGroupByItem.SetCheck(eColumnGroupBy == eacgbtOnePerItem);
			m_radioColumnGroupByCondensed.SetCheck(eColumnGroupBy == eacgbtCondensed);

			// (j.jones 2009-03-30 09:49) - PLID 33703 - added FilterByTemplateID
			long nFilterByTemplateID = AdoFldLong(rs, "FilterByTemplateID", -1);
			if(nFilterByTemplateID != -1) {
				IRowSettingsPtr pTemplateRow = m_TemplateCombo->SetSelByColumn(tccID, nFilterByTemplateID);
				if(pTemplateRow) {
					m_checkFilterOnSelectedTemplate.SetCheck(TRUE);
					m_checkFilterOnSelectedTemplate.EnableWindow(TRUE);

					m_nSelTemplateID = nFilterByTemplateID;

					//filter the item combo
					OnSelChosenEmrTemplateCombo(pTemplateRow);					
				}
				else {
					//disable this filter if no template is selected
					m_checkFilterOnSelectedTemplate.EnableWindow(FALSE);
				}
			}
			else {
				//disable this filter if no template is selected
				m_checkFilterOnSelectedTemplate.EnableWindow(FALSE);
			}

			// (j.jones 2009-04-09 14:24) - PLID 33916 - added IncludeSpawningInfo
			m_checkShowSpawningInfo.SetCheck(AdoFldBool(rs, "IncludeSpawningInfo", FALSE));
		}
		else {
			CString strError;
			strError.Format("Attempted to load an EMR Analysis Configuration with an invalid ID (%li)!", m_nID);
			ThrowNxException(strError);
		}
		rs->Close();

		//load the list - can't be requeried because we're making the iloDataFilter column have
		//a different combo source per item in the list
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection, but leave the nolock hint since SQL2000 may do locking otherwise
		// (a.wilson 2013-05-14 14:07) - PLID 55963 - add new text filter column for the contains operator.
		rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRAnalysisConfigDetailsT.ID, "
			"EmrInfoMasterT.ID AS EMRInfoMasterID, EMRInfoT.Name, DataType, "
			"CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' WHEN DataType = 7 THEN 'Table' END AS DataTypeText, "
			"EMRAnalysisConfigDetailsT.DataOperator, EMRAnalysisConfigDetailsT.EMRDataGroupID, EMRAnalysisConfigDetailsT.TextFilterData "
			"FROM EMRAnalysisConfigDetailsT WITH(NOLOCK) "
			"INNER JOIN EmrInfoMasterT WITH(NOLOCK) ON EMRAnalysisConfigDetailsT.EMRInfoMasterID = EmrInfoMasterT.ID "
			"INNER JOIN EmrInfoT WITH(NOLOCK) ON EmrInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
			"WHERE EMRAnalysisConfigID = {INT}", m_nID);
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID");
			long nEMRInfoMasterID = AdoFldLong(rs, "EMRInfoMasterID");
			CString strName = AdoFldString(rs, "Name", "");
			EmrInfoType eDataType = (EmrInfoType)AdoFldByte(rs, "DataType");
			CString strType = AdoFldString(rs, "DataTypeText", "");
			EMRAnalysisDataOperatorType eOperator = (EMRAnalysisDataOperatorType)AdoFldLong(rs, "DataOperator", (long)eadotExists);
			_variant_t varEMRDataGroupID = rs->Fields->Item["EMRDataGroupID"]->Value;
			// (a.wilson 2013-05-14 14:16) - PLID 55963 - get data from new column.
			CString strTextFilterData = AdoFldString(rs, "TextFilterData", "");

			//this creates the combo source for the operator cell, which means using FormatSettings
			IFormatSettingsPtr pfsOperator = GetOperatorFormatSettings(eDataType);

			//this creates the combo source for the filter cell, which means using FormatSettings
			//IFormatSettingsPtr pfsFilter = GetDataFilterFormatSettings(eDataType, nEMRInfoMasterID);

			//add it to the list
			IRowSettingsPtr pNewRow = m_ItemList->GetNewRow();

			//set our combo sources before setting the data
			pNewRow->PutRefCellFormatOverride(iloDataOperator, pfsOperator);
			
			//pNewRow->PutRefCellFormatOverride(iloDataFilter, pfsFilter);
			// (j.jones 2009-04-09 15:18) - PLID 33947 - renamed to ApplyDataFilterFormatSettings and added several parameters
			TryApplyDataFilterFormatSettings(pNewRow, eOperator, eDataType, nEMRInfoMasterID);

			pNewRow->PutValue(iloID, (long)nID);
			pNewRow->PutValue(iloEMRInfoMasterID, (long)nEMRInfoMasterID);
			pNewRow->PutValue(iloEMRInfoName, _bstr_t(strName));
			pNewRow->PutValue(iloEMRInfoType, (long)eDataType);
			pNewRow->PutValue(iloEMRInfoTypeName, _bstr_t(strType));
			pNewRow->PutValue(iloDataOperator, (long)eOperator);
			// (a.wilson 2013-05-14 14:17) - PLID 55963 - assign based on operator (only contains will use the textfilterdata)
			if (eOperator == eadotContains)
				pNewRow->PutValue(iloDataFilter, _bstr_t(strTextFilterData));
			else	//retain old behavior if its not contains.
				pNewRow->PutValue(iloDataFilter, varEMRDataGroupID);

			m_ItemList->AddRowSorted(pNewRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::Load");
}

BOOL CEMRAnalysisConfigDlg::Save()
{
	try {

		//get the description
		CString strDescription;
		m_editDescription.GetWindowText(strDescription);
		strDescription.TrimLeft();
		strDescription.TrimRight();
		//we may have removed spaces, so reset the displayed text
		m_editDescription.SetWindowText(strDescription);

		//do we have a description?
		if(strDescription.IsEmpty()) {
			AfxMessageBox("You must enter a description for this configuration before saving.");
			return FALSE;
		}

		//make sure the description is not a duplicate of another configuration
		//this works fine if our ID is -1
		_RecordsetPtr rsDupDesc = CreateParamRecordset("SELECT ID FROM EMRAnalysisConfigT "
			"WHERE Name = {STRING} AND ID <> {INT} ",
			strDescription, m_nID);
		if(!rsDupDesc->eof) {
			AfxMessageBox("Another EMR Analysis Configuration has the same description.\n"
				"You must change the description of this configuration before saving.");
			return FALSE;
		}
		rsDupDesc->Close();

		//make sure we have items in the list
		if(m_ItemList->GetRowCount() == 0) {
			AfxMessageBox("You must add at least one EMR Item to the list before saving.");
			return FALSE;
		}

		// (c.haag 2009-02-24 17:29) - PLID 33187 - Don't let the user save an absurdly large
		// number of columns under the non-condensed mode
		else if (m_ItemList->GetRowCount() > MAX_ANALYSIS_DYNAMIC_COLUMNS && !m_radioColumnGroupByCondensed.GetCheck()) {
			AfxMessageBox(FormatString("You may not have more than %d items in your list using the 'One Column Per Item' display setting. "
				"However, you may change the setting to 'Use As Few Columns As Possible', which will use columns only as needed, if you require additional items.", MAX_ANALYSIS_DYNAMIC_COLUMNS));
			return FALSE;
		}

		BOOL bUseDateFilter = m_radioEMNDate.GetCheck();
		COleDateTime dtFrom = g_cdtNull;
		COleDateTime dtTo = g_cdtNull;

		if(bUseDateFilter) {
			//get the date filter and validate it
			dtFrom = VarDateTime(m_dtFrom.GetValue());
			dtTo = VarDateTime(m_dtTo.GetValue());
			if(dtFrom > dtTo) {
				AfxMessageBox("Your date filter has a From date greater than your To date. Please correct this before saving.");
				return FALSE;
			}
		}

		BOOL bFilterOnAllItems = m_radioAllItems.GetCheck();

		EMRAnalysisRecordFilterType eRecordFilter = earftPatient;
		if(m_radioFilterOnEMN.GetCheck()) {
			eRecordFilter = earftEMN;
		}
		else if(m_radioFilterOnEMR.GetCheck()) {
			eRecordFilter = earftEMR;
		}

		EMRAnalysisGroupByType eDisplayGroupBy = eagbtPatient;
		if(m_radioGroupByDate.GetCheck()) {
			eDisplayGroupBy = eagbtDate;
		}
		else if(m_radioGroupByEMN.GetCheck()) {
			eDisplayGroupBy = eagbtEMN;
		}
		else if(m_radioGroupByEMR.GetCheck()) {
			eDisplayGroupBy = eagbtEMR;
		}

		// (c.haag 2009-02-24 11:57) - PLID 33187 - eColumnGroupBy
		EMRAnalysisColumnGroupByType eColumnGroupBy = eacgbtCondensed;
		if(m_radioColumnGroupByItem.GetCheck()) {
			eColumnGroupBy = eacgbtOnePerItem;
		}
		else if (m_radioColumnGroupByCondensed.GetCheck()) {
			eColumnGroupBy = eacgbtCondensed;
		}
		
		//ok, we're ready to save!		
		// (a.wilson 2013-05-14 10:10) - PLID 55963 - convert to paramSqlBatch.
		CParamSqlBatch sqlBatch;

		//delete any items we removed
		for(int i=0; i<m_aryDeletedItems.GetSize(); i++) {
			long nID = (long)m_aryDeletedItems.GetAt(i);
			if(nID != -1) {
				sqlBatch.Add("DELETE FROM EMRAnalysisConfigDetailsT WHERE ID = {INT} ", nID);
			}
		}

		// (j.jones 2009-03-27 11:37) - PLID 33703 - save our filtered template ID
		long nFilterByTemplateID = -1;
		CString strFilterByTemplateID = "NULL";
		if(m_checkFilterOnSelectedTemplate.GetCheck()) {
			IRowSettingsPtr pRow = m_TemplateCombo->GetCurSel();
			if(pRow) {
				nFilterByTemplateID = VarLong(pRow->GetValue(tccID), -1);
			}

			if(nFilterByTemplateID != -1) {
				strFilterByTemplateID.Format("%li", nFilterByTemplateID);
			}
			else {
				//don't let them save
				AfxMessageBox("This analysis configuration is currently set to filter results by the current template, "
					"however there is no template selected in the list. Please correct this before saving.");
				return FALSE;
			}
		}

		// (j.jones 2009-04-09 14:24) - PLID 33916 - added IncludeSpawningInfo
		BOOL bIncludeSpawningInfo = m_checkShowSpawningInfo.GetCheck();

		sqlBatch.Declare("DECLARE @nID INT ");
		
		//save/update the configuration record
		if(m_nID == -1) {
			sqlBatch.Add("SET @nID = Coalesce((SELECT Max(ID) FROM EMRAnalysisConfigT), 0) + 1 ");
			// (c.haag 2009-02-24 11:16) - PLID 33187 - Added ColumnGroupBy
			// (j.jones 2009-03-27 12:00) - PLID 33703 - added FilterByTemplateID
			// (j.jones 2009-04-09 14:25) - PLID 33916 - added IncludeSpawningInfo
			sqlBatch.Add("INSERT INTO EMRAnalysisConfigT "
				"(ID, Name, UseDateFilter, DateFrom, DateTo, FilterOnAllItems, RecordFilter, DisplayGroupBy, ColumnGroupBy, FilterByTemplateID, IncludeSpawningInfo) "
				"VALUES (@nID, {STRING}, {BIT}, (CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, {VT_DATE})))), (CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, {VT_DATE})))), {BIT}, {INT}, {INT}, {INT}, {VT_I4}, {BIT}) ",
				strDescription, (bUseDateFilter ? 1 : 0), (dtFrom == g_cdtNull ? g_cvarNull : _variant_t(dtFrom, VT_DATE)), 
				(dtTo == g_cdtNull ? g_cvarNull : _variant_t(dtTo, VT_DATE)), (bFilterOnAllItems ? 1 : 0), (long)eRecordFilter, 
				(long)eDisplayGroupBy, (long)eColumnGroupBy, (strFilterByTemplateID == "NULL" ? g_cvarNull : _variant_t(atol(strFilterByTemplateID), VT_I4)), 
				(bIncludeSpawningInfo ? 1 : 0));
		}
		else {
			sqlBatch.Add("SET @nID = {INT}", m_nID);
			// (c.haag 2009-02-24 11:16) - PLID 33187 - Added ColumnGroupBy
			// (j.jones 2009-03-27 12:00) - PLID 33703 - added FilterByTemplateID
			// (j.jones 2009-04-09 14:25) - PLID 33916 - added IncludeSpawningInfo
			sqlBatch.Add("UPDATE EMRAnalysisConfigT SET Name = {STRING}, UseDateFilter = {BIT}, "
				"DateFrom = (CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, {VT_DATE})))), "
				"DateTo = (CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, {VT_DATE})))), "
				"FilterOnAllItems = {BIT}, RecordFilter = {INT}, DisplayGroupBy = {INT}, "
				"ColumnGroupBy = {INT}, FilterByTemplateID = {VT_I4}, IncludeSpawningInfo = {BIT} WHERE ID = @nID ",
				strDescription, (bUseDateFilter ? 1 : 0), (dtFrom == g_cdtNull ? g_cvarNull : _variant_t(dtFrom, VT_DATE)), 
				(dtTo == g_cdtNull ? g_cvarNull : _variant_t(dtTo, VT_DATE)), (bFilterOnAllItems ? 1 : 0), 
				(long)eRecordFilter, (long)eDisplayGroupBy, (long)eColumnGroupBy, 
				(strFilterByTemplateID == "NULL" ? g_cvarNull : _variant_t(atol(strFilterByTemplateID), VT_I4)),
				(bIncludeSpawningInfo ? 1 : 0));
		}

		CString strEMRInfoMasterIDs;

		//now add/update all the items in the list
		IRowSettingsPtr pRow = m_ItemList->GetFirstRow();
		while(pRow) {

			long nID = VarLong(pRow->GetValue(iloID));
			long nEMRInfoMasterID = VarLong(pRow->GetValue(iloEMRInfoMasterID));

			// (j.jones 2009-03-27 11:35) - PLID 33703 - track these IDs, we're going to validate them shortly
			if(!strEMRInfoMasterIDs.IsEmpty()) {
				strEMRInfoMasterIDs += ",";
			}
			strEMRInfoMasterIDs += AsString((long)nEMRInfoMasterID);

			EMRAnalysisDataOperatorType eOperator = (EMRAnalysisDataOperatorType)VarLong(pRow->GetValue(iloDataOperator), (long)eadotExists);
			// (a.wilson 2013-05-14 09:43) - PLID 55963 - add saving for new text filter column.
			// (j.jones 2009-04-09 16:35) - PLID 33947 - have to specifically check for VT_I4 now
			// because this can sometimes be VT_EMPTY
			_variant_t varEMRData = pRow->GetValue(iloDataFilter);
			CString strEMRDataGroupID = "NULL", strTextFilterData = "NULL";
			
			//if the operator is contains (which is only used by text details) then do special handling.
			if (eOperator == eadotContains) {
				//get the data.
				strTextFilterData = AsString(varEMRData);
				//remove any leading or trailing spaces.
				strTextFilterData.Trim();
				//if this is empty then we want to assign NULL in data.
				if (strTextFilterData.IsEmpty()) {
					strTextFilterData = "NULL";
				}
			} else {
				long nEMRDataGroupID = -1;
				//make sure its a valid integer value before pulling out data.
				if(varEMRData.vt == VT_I4) {
					nEMRDataGroupID = VarLong(varEMRData);
				}
				if(nEMRDataGroupID != -1) {
					strEMRDataGroupID.Format("%li", nEMRDataGroupID);
				}
				else {
					if(eOperator == eadotHasDataIn || eOperator == eadotHasNoDataIn) {
						//abort this entire operation
						AfxMessageBox("At least one selected EMR Item has the 'Has Data In...' or 'Has No Data In...' Operator, but no Data Filter selected.\n"
							"Please correct this before saving.");
						return FALSE;
					}
				}
			}

			if(nID == -1) {
				sqlBatch.Add("INSERT INTO EMRAnalysisConfigDetailsT "
					"(EMRAnalysisConfigID, EMRInfoMasterID, DataOperator, EMRDataGroupID, TextFilterData) "
					"VALUES (@nID, {INT}, {INT}, {VT_I4}, {VT_BSTR}) ", 
					nEMRInfoMasterID, (long)eOperator, 
					(strEMRDataGroupID == "NULL" ? g_cvarNull : _variant_t(atol(strEMRDataGroupID), VT_I4)), 
					(strTextFilterData == "NULL" ? g_cvarNull : _bstr_t(strTextFilterData)));
			} else {
				sqlBatch.Add("UPDATE EMRAnalysisConfigDetailsT "
					"SET DataOperator = {INT}, EMRDataGroupID = {VT_I4}, TextFilterData = {VT_BSTR} WHERE ID = {INT} ",
					(long)eOperator, 
					(strEMRDataGroupID == "NULL" ? g_cvarNull : _variant_t(atol(strEMRDataGroupID), VT_I4)), 
					(strTextFilterData == "NULL" ? g_cvarNull : _bstr_t(strTextFilterData)), nID);
			}

			pRow = pRow->GetNextRow();
		}

		// (j.jones 2009-03-27 11:36) - PLID 33703 - before we actually execute the batch,
		// warn if any items are not on our filtered template
		if(m_checkFilterOnSelectedTemplate.GetCheck() && nFilterByTemplateID != -1
			&& !strEMRInfoMasterIDs.IsEmpty()) {

			//this query checks that items aren't in the filtered item list, and should
			//always follow similar logic to the from clause generated in OnSelChosenEmrTemplateCombo
			// (j.jones 2010-05-03 16:23) - PLID 38464 - streamlined this query to not use
			// quite as many IN clauses, in order to keep roughly in line with the from clause,
			// which changed to have no IN clauses
			CString strSql;
			strSql.Format("SELECT EMRInfoT.Name "
				"FROM EMRInfoMasterT WITH(NOLOCK) "
				"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
				"WHERE EMRInfoMasterT.ID IN (%s) "
				"AND EMRInfoMasterT.ID NOT IN "
				"	(SELECT EMRInfoMasterT.ID "
				"	FROM EmrInfoT WITH(NOLOCK) "
				"	INNER JOIN EmrInfoMasterT WITH(NOLOCK) ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
				"	LEFT JOIN (SELECT EmrInfoMasterID FROM EMRTemplateDetailsT WITH(NOLOCK) "
				"		WHERE TemplateID = %li) AS IsOnEMRTemplateQ ON EmrInfoMasterT.ID = IsOnEMRTemplateQ.EmrInfoMasterID "
				"	LEFT JOIN (SELECT SourceID, DestID FROM EMRActionsT WITH(NOLOCK) "
				"		INNER JOIN EMRTemplateDetailsT WITH(NOLOCK) ON EMRActionsT.SourceID = EMRTemplateDetailsT.EMRInfoMasterID "
				"		WHERE SourceType = %li AND DestType = %li AND EMRActionsT.Deleted = 0 "
				"		AND EMRTemplateDetailsT.TemplateID = %li "
				"		) AS EMRActionsT1 ON EmrInfoMasterT.ID = EMRActionsT1.DestID "
				"	LEFT JOIN (SELECT SourceID, DestID FROM EMRActionsT WITH(NOLOCK) "
				"		INNER JOIN EMRDataT WITH(NOLOCK) ON EMRActionsT.SourceID = EMRDataT.EmrDataGroupID "
				"		INNER JOIN EMRInfoMasterT WITH(NOLOCK) ON EMRDataT.EMRInfoID = EMRInfoMasterT.ActiveEMRInfoID "
				"		INNER JOIN EMRTemplateDetailsT WITH(NOLOCK) ON EMRInfoMasterT.ID = EMRTemplateDetailsT.EMRInfoMasterID "
				"		WHERE SourceType = %li AND DestType = %li AND EMRActionsT.Deleted = 0 "
				"		AND EMRTemplateDetailsT.TemplateID = %li "
				"		) AS EMRActionsT2 ON EmrInfoMasterT.ID = EMRActionsT2.DestID "
				"	WHERE EMRInfoMasterT.Inactive = 0 "
				"	AND (IsOnEMRTemplateQ.EMRInfoMasterID Is Not Null "
				"		OR EMRActionsT1.SourceID Is Not Null OR EMRActionsT2.SourceID Is Not Null)"
				"	) "
				"ORDER BY EMRInfoT.Name",
				strEMRInfoMasterIDs,
				nFilterByTemplateID, 
				eaoEmrItem, eaoEmrItem, nFilterByTemplateID, 
				eaoEmrDataItem, eaoEmrItem, nFilterByTemplateID);

			//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection, but leave the nolock hint since SQL2000 may do locking otherwise
			_RecordsetPtr rsInvalidItems = CreateParamRecordset(GetRemoteDataSnapshot(), strSql);

			CString strInvalidItems;
			long nCountInvalidItems = 0;
			while(!rsInvalidItems->eof) {

				CString strName = AdoFldString(rsInvalidItems, "Name", "");

				//only show the top 10 items
				if(nCountInvalidItems < 10) {
					strInvalidItems += strName;
					strInvalidItems += "\n";
				}
				else if(nCountInvalidItems == 10) {
					strInvalidItems += "(more...)\n";
				}

				nCountInvalidItems++;

				rsInvalidItems->MoveNext();
			}
			rsInvalidItems->Close();

			if(nCountInvalidItems > 0) {
				CString strWarn;
				// (j.jones 2009-06-04 13:18) - PLID 34486 - added an extra line that explains why this may be fine
				strWarn.Format("This analysis configuration is currently set to filter results by the current template.\n"
					"However, %li EMR item(s) in your filter do not currently exist on the template:\n\n"
					"%s\n"
					"While these items do not exist on your template, it is possible they can be spawned by items that do exist on your template.\n\n"
					"Do you still wish to save this configuration?", nCountInvalidItems, strInvalidItems);
				if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		if(!sqlBatch.IsEmpty()) {
			sqlBatch.Prepend("SET NOCOUNT ON \r\nBEGIN TRAN \r\n");
			sqlBatch.Add("COMMIT TRAN \r\nSET NOCOUNT OFF \r\nSELECT @nID AS ID ");
			//CSqlFragment sqlFrag = FromBatch(sqlBatch, sqlBatch.m_aryParams);
			//CString str = sqlFrag.Flatten();
			_RecordsetPtr rs = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());
			if(!rs->eof) {
				if(m_nID == -1) {
					m_nID = AdoFldLong(rs, "ID");
				}
			}
			rs->Close();
		}

		return TRUE;

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::Save");

	return FALSE;
}

void CEMRAnalysisConfigDlg::OnOk()
{
	try {

		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnOk");
}

BEGIN_EVENTSINK_MAP(CEMRAnalysisConfigDlg, CNxDialog)
	ON_EVENT(CEMRAnalysisConfigDlg, IDC_EMR_INFO_ITEM_COMBO, 16, OnSelChosenEmrInfoItemCombo, VTS_DISPATCH)
	ON_EVENT(CEMRAnalysisConfigDlg, IDC_EMR_ANALYSIS_CONFIG_LIST, 6, OnRButtonDownEmrAnalysisConfigList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRAnalysisConfigDlg, IDC_EMR_ANALYSIS_CONFIG_LIST, 8, OnEditingStartingEmrAnalysisConfigList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMRAnalysisConfigDlg, IDC_EMR_ANALYSIS_CONFIG_LIST, 10, OnEditingFinishedEmrAnalysisConfigList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRAnalysisConfigDlg, IDC_EMR_TEMPLATE_COMBO, 16, OnSelChosenEmrTemplateCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEMRAnalysisConfigDlg::OnSelChosenEmrInfoItemCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//add this item to the list, but not if it already exists
		long nEMRInfoMasterID = VarLong(pRow->GetValue(iccID));
		CString strName = VarString(pRow->GetValue(iccName));
		EmrInfoType eDataType = (EmrInfoType)VarByte(pRow->GetValue(iccDataType));
		CString strType = VarString(pRow->GetValue(iccDataTypeName));

		//does it already exist?
		if(m_ItemList->GetRowCount()) {
			IRowSettingsPtr pFoundRow = m_ItemList->FindByColumn(iloEMRInfoMasterID, (long)nEMRInfoMasterID, m_ItemList->GetFirstRow(), FALSE);
			if(pFoundRow) {
				AfxMessageBox("This EMR Item is already used in this configuration.");
				return;
			}
		}

		//this creates the combo source for the operator cell, which means using FormatSettings
		IFormatSettingsPtr pfsOperator = GetOperatorFormatSettings(eDataType);

		//this creates the combo source for the filter cell, which means using FormatSettings
		//IFormatSettingsPtr pfsFilter = GetDataFilterFormatSettings(eDataType, nEMRInfoMasterID);

		//add it to the list
		IRowSettingsPtr pNewRow = m_ItemList->GetNewRow();

		//set our combo sources before setting the data
		pNewRow->PutRefCellFormatOverride(iloDataOperator, pfsOperator);
		
		//pNewRow->PutRefCellFormatOverride(iloDataFilter, pfsFilter);
		// (j.jones 2009-04-09 15:18) - PLID 33947 - we don't apply a data filter format override yet,
		// because we default to eadotExists, which doesn't need it

		pNewRow->PutValue(iloID, (long)-1);
		pNewRow->PutValue(iloEMRInfoMasterID, (long)nEMRInfoMasterID);
		pNewRow->PutValue(iloEMRInfoName, _bstr_t(strName));
		pNewRow->PutValue(iloEMRInfoType, (long)eDataType);
		pNewRow->PutValue(iloEMRInfoTypeName, _bstr_t(strType));
		pNewRow->PutValue(iloDataOperator, (long)eadotExists);
		pNewRow->PutValue(iloDataFilter, g_cvarNull);
		m_ItemList->AddRowSorted(pNewRow, NULL);

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnSelChosenEmrInfoItemCombo");
}

void CEMRAnalysisConfigDlg::OnRButtonDownEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_ItemList->CurSel = pRow;

		//add an ability to delete this item
		enum {
			eDeleteItem = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eDeleteItem, "&Remove This Item");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eDeleteItem) {

			//grab the ID
			long nID = VarLong(pRow->GetValue(iloID));

			//remove the row from the list
			m_ItemList->RemoveRow(pRow);

			//if not -1, add to our deleted array
			if(nID != -1) {
				m_aryDeletedItems.Add(nID);
			}
		}

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRButtonDownEmrAnalysisConfigList");
}
void CEMRAnalysisConfigDlg::OnRadioAllEmnDates()
{
	try {

		OnRadioEmnDate();

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioAllEmnDates");
}

void CEMRAnalysisConfigDlg::OnRadioEmnDate()
{
	try {

		BOOL bEnabled = m_radioEMNDate.GetCheck();
		GetDlgItem(IDC_EMN_FROM_DATE)->EnableWindow(bEnabled);
		GetDlgItem(IDC_EMN_TO_DATE)->EnableWindow(bEnabled);

	} NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioEmnDate");
}

void CEMRAnalysisConfigDlg::OnEditingStartingEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//the iloDataFilter column should not be editable if the
		//iloDataOperator is not eadotHasDataIn or eadotHasNoDataIn
		// (a.wilson 2013-05-13 17:47) - PLID 55963 - allow editing if the operator is contains.
		if(nCol == iloDataFilter) {
			EMRAnalysisDataOperatorType eFilter = (EMRAnalysisDataOperatorType)VarLong(pRow->GetValue(iloDataOperator), (long)eadotExists);
			if (eFilter != eadotHasDataIn && eFilter != eadotHasNoDataIn && eFilter != eadotContains) {
				*pbContinue = FALSE;
				return;
			}
		}
		
	}NxCatchAll("CEMRAnalysisConfigDlg::OnEditingStartingEmrAnalysisConfigList");
}

void CEMRAnalysisConfigDlg::OnEditingFinishedEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(!bCommit) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == iloDataOperator) {
			EMRAnalysisDataOperatorType eNewOperator = (EMRAnalysisDataOperatorType)VarLong(varNewValue, (long)eadotExists);

			//if they changed the iloDataOperator column to an option that
			//doesn't have a data filter, clear the iloDataFilter column
			if(eNewOperator != eadotHasDataIn && eNewOperator != eadotHasNoDataIn && eNewOperator != eadotContains) {
				pRow->PutValue(iloDataFilter, g_cvarNull);
			}
			//if they selected an option that does have a data filter, and the current
			//value of that filter is NULL, auto-dropdown that filter list
			// (a.wilson 2013-05-14 14:01) - PLID 55963
			else if(eNewOperator == eadotHasDataIn || eNewOperator == eadotHasNoDataIn || eNewOperator == eadotContains) {

				// (j.jones 2009-04-09 15:59) - PLID 33947 - ensure we've set up our combo
				TryApplyDataFilterFormatSettings(pRow, eNewOperator,
					(EmrInfoType)VarLong(pRow->GetValue(iloEMRInfoType), (long)eitInvalid),
					VarLong(pRow->GetValue(iloEMRInfoMasterID), -1));

				if(pRow->GetValue(iloDataFilter).vt == VT_NULL) {
					//drop it down after this function completes
					PostMessage(START_EDITING_FILTER_LIST, (WPARAM)pRow.Detach());
				}
			}
		}

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnEditingFinishedEmrAnalysisConfigList");
}
void CEMRAnalysisConfigDlg::OnSelChosenEmrTemplateCombo(LPDISPATCH lpRow)
{
	try {

		long nTemplateID = -1;

		//if the row is NULL, we will filter on all templates
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select "all"
			pRow = m_TemplateCombo->FindByColumn(tccID, (long)-1, m_TemplateCombo->GetFirstRow(), TRUE);
			if(pRow == NULL) {
				ThrowNxException("The {All Templates} row does not exist!");
			}
		}

		nTemplateID = VarLong(pRow->GetValue(tccID));

		//If they changed templates while a filtering on a template, warn them.
		//We'd ideally do this in OnSelChanging, but if you fire a MessageBox there,
		//OnSelChosen will never be hit.
		if(m_checkFilterOnSelectedTemplate.GetCheck() && nTemplateID != m_nSelTemplateID) {

			if(IDNO == MessageBox("This analysis configuration is currently set to filter results by a template.\n"
				"If you switch templates, this filter will be unchecked and the analysis will include the selected items regardless of the template that created them.\n\n"
				"Are you sure you wish to change templates?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				pRow = m_TemplateCombo->SetSelByColumn(tccID, (long)m_nSelTemplateID);

				if(pRow == NULL) {
					//should be impossible
					ASSERT(FALSE);
					//select "all"
					pRow = m_TemplateCombo->FindByColumn(tccID, (long)-1, m_TemplateCombo->GetFirstRow(), TRUE);
					if(pRow == NULL) {
						ThrowNxException("The {All Templates} row does not exist!");
					}
				}

				nTemplateID = VarLong(pRow->GetValue(tccID));				
			}
			else {
				m_checkFilterOnSelectedTemplate.SetCheck(FALSE);
			}
		}

		m_nSelTemplateID = nTemplateID;

		// (j.jones 2009-03-30 10:30) - PLID 33703 - disable & clear the template filter if no row selected
		if(nTemplateID == -1) {
			m_checkFilterOnSelectedTemplate.SetCheck(FALSE);
			m_checkFilterOnSelectedTemplate.EnableWindow(FALSE);
		}
		else {
			m_checkFilterOnSelectedTemplate.EnableWindow(TRUE);
		}

		//re-filter the item list, no need to try and re-select anything
		CString strFrom;
		if(nTemplateID == -1) {
			//simply show all items
			// (j.jones 2010-04-27 10:14) - PLID 37693 - images are now permitted
			// (j.jones 2010-06-04 16:55) - PLID 39029 - do not show the generic table (DataSubType = 3)
			strFrom.Format("(SELECT EMRInfoMasterT.ID AS EMRInfoMasterID, "
				"EmrInfoT.Name, EmrInfoT.DataType, EmrInfoT.DataSubType "
				"FROM EmrInfoT WITH(NOLOCK) "
				"INNER JOIN EmrInfoMasterT WITH(NOLOCK) ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
				"WHERE EMRInfoMasterT.Inactive = 0 "
				"AND EMRInfoT.DataSubType <> %li "
				") AS ItemListQ", eistGenericTable);
		}
		else {
			//use the same logic from the EMR Item Entry's 'Used On Template' list,
			//and show items that are on the selected template, or are directly spawned
			//by items on the template
			
			//if you change this query, please change the logic of the rsInvalidItems recordset
			//in the Save() process
			// (j.jones 2010-04-27 10:14) - PLID 37693 - images are now permitted
			// (j.jones 2010-05-03 16:23) - PLID 38464 - streamlined this query to not use IN clauses
			// (j.jones 2010-06-04 16:55) - PLID 39029 - do not show the generic table (DataSubType = 3)
			strFrom.Format("(SELECT EMRInfoMasterT.ID AS EMRInfoMasterID, "
				"EmrInfoT.Name, EmrInfoT.DataType, EmrInfoT.DataSubType "
				"FROM EmrInfoT WITH(NOLOCK) "
				"INNER JOIN EmrInfoMasterT WITH(NOLOCK) ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
				"LEFT JOIN (SELECT EmrInfoMasterID FROM EMRTemplateDetailsT WITH(NOLOCK) "
				"	WHERE TemplateID = %li) AS IsOnEMRTemplateQ ON EmrInfoMasterT.ID = IsOnEMRTemplateQ.EmrInfoMasterID "
				"LEFT JOIN (SELECT SourceID, DestID FROM EMRActionsT WITH(NOLOCK) "
				"	INNER JOIN EMRTemplateDetailsT WITH(NOLOCK) ON EMRActionsT.SourceID = EMRTemplateDetailsT.EMRInfoMasterID "
				"	WHERE SourceType = %li AND DestType = %li AND EMRActionsT.Deleted = 0 "
				"	AND EMRTemplateDetailsT.TemplateID = %li "
				"	) AS EMRActionsT1 ON EmrInfoMasterT.ID = EMRActionsT1.DestID "
				"LEFT JOIN (SELECT SourceID, DestID FROM EMRActionsT WITH(NOLOCK) "
				"	INNER JOIN EMRDataT WITH(NOLOCK) ON EMRActionsT.SourceID = EMRDataT.EmrDataGroupID "
				"	INNER JOIN EMRInfoMasterT WITH(NOLOCK) ON EMRDataT.EMRInfoID = EMRInfoMasterT.ActiveEMRInfoID "
				"	INNER JOIN EMRTemplateDetailsT WITH(NOLOCK) ON EMRInfoMasterT.ID = EMRTemplateDetailsT.EMRInfoMasterID "
				"	WHERE SourceType = %li AND DestType = %li AND EMRActionsT.Deleted = 0 "
				"	AND EMRTemplateDetailsT.TemplateID = %li "
				"	) AS EMRActionsT2 ON EmrInfoMasterT.ID = EMRActionsT2.DestID "
				"WHERE EMRInfoMasterT.Inactive = 0 "
				"AND EMRInfoT.DataSubType <> %li "
				"AND (IsOnEMRTemplateQ.EMRInfoMasterID Is Not Null "
				"	OR EMRActionsT1.SourceID Is Not Null OR EMRActionsT2.SourceID Is Not Null) "
				"GROUP BY EMRInfoMasterT.ID, EmrInfoT.Name, EmrInfoT.DataType, EmrInfoT.DataSubType "
				") AS ItemListQ ",
				nTemplateID, 
				eaoEmrItem, eaoEmrItem, nTemplateID, 
				eaoEmrDataItem, eaoEmrItem, nTemplateID, eistGenericTable);
		}

		CString strOldFromClause = (LPCTSTR)m_ItemCombo->GetFromClause();
		//don't bother requerying this large data set if the from clause didn't change
		if(strOldFromClause != strFrom) {
			m_ItemCombo->PutFromClause(_bstr_t(strFrom));
			m_ItemCombo->Requery();
		}

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnSelChosenEmrTemplateCombo");
}

//returns the format settings for the iloDataOperator column, given an EMRInfoT.DataType
NXDATALIST2Lib::IFormatSettingsPtr CEMRAnalysisConfigDlg::GetOperatorFormatSettings(EmrInfoType eDataType)
{
	try {

		CString strOperatorComboSource;

		if(eDataType == eitSingleList || eDataType == eitMultiList
			|| eDataType == eitTable) {

			//set our combo data to include all five options
			strOperatorComboSource.Format("%li;Exists;"
				"%li;Item Has Data;"
				"%li;Item Has No Data;"
				"%li;Has Data In...;"
				"%li;Has No Data In...;",
				eadotExists, eadotItemHasData, eadotItemHasNoData, eadotHasDataIn, eadotHasNoDataIn);
		}
		// (a.wilson 2013-05-13 17:44) - PLID 55963 - add "Contains" for text details only.
		else if (eDataType == eitText) {
			strOperatorComboSource.Format(
				"%li;Exists;"
				"%li;Contains;"
				"%li;Item Has Data;"
				"%li;Item Has No Data;",
				eadotExists, eadotContains, eadotItemHasData, eadotItemHasNoData);
		} else {
			//set our combo data to only include the first three options
			strOperatorComboSource.Format("%li;Exists;"
				"%li;Item Has Data;"
				"%li;Item Has No Data;",
				eadotExists, eadotItemHasData, eadotItemHasNoData);
		}

		// (j.jones 2009-04-09 15:09) - PLID 33847 - is this already in our map?
		IFormatSettingsPtr pfs(__uuidof(FormatSettings));

		if(m_mapComboSourceToOperatorFormatSettings.Lookup((LPCTSTR)strOperatorComboSource, pfs)) {
			//excellent, get out of here
			return pfs;
		}
		else {
			//create the format settings, add to the map, and return it
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(cftComboSimple);
			pfs->PutEditable(VARIANT_TRUE);
			//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
			pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteDataSnapshot())); //we're going to let this combo use Practice's snapshot connection
			pfs->PutComboSource(_bstr_t(strOperatorComboSource));

			m_mapComboSourceToOperatorFormatSettings.SetAt((LPCTSTR)strOperatorComboSource, pfs);

			return pfs;
		}

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::GetOperatorFormatSettings");

	return NULL;
}

//returns the combo source for the iloDataFilter column, given an EMRInfoT.DataType and EMRInfoT.EMRInfoMasterID
// (j.jones 2009-04-09 15:18) - PLID 33947 - renamed to ApplyDataFilterFormatSettings and added several parameters
void CEMRAnalysisConfigDlg::TryApplyDataFilterFormatSettings(NXDATALIST2Lib::IRowSettingsPtr pRow, EMRAnalysisDataOperatorType eOperator,
															 EmrInfoType eDataType, long nEMRInfoMasterID)
{
	try {

		CString strFilterComboSource;
		// (a.wilson 2013-05-14 13:38) - PLID 55963 - prevent contains from getting a dropdown as well.
		if(eOperator != eadotHasDataIn && eOperator != eadotHasNoDataIn && eOperator != eadotContains) {
			//we don't need a dropdown at all
			return;
		}
		//ensure the correct override is selected. if so then no point to reconstruct.
		IFormatSettingsPtr pfsExist = pRow->GetCellFormatOverride(iloDataFilter);
		if(pfsExist) {
			//if its a combo and the operator is 'has data in' or 'has no data in' then return.
			if (pfsExist->GetFieldType() == cftComboSimple && (eOperator == eadotHasDataIn || eOperator == eadotHasNoDataIn))
				return;
			//if its a text and the operator is 'contains' then return.
			if (pfsExist->GetFieldType() == cftTextSingleLine && eOperator == eadotContains)
				return;
		}

		//otherwise make a new one
		IFormatSettingsPtr pfs(__uuidof(FormatSettings));
		if (eOperator == eadotHasDataIn || eOperator == eadotHasNoDataIn) {
			if(eDataType == eitSingleList || eDataType == eitMultiList
					|| eDataType == eitTable) {

				CString strListTypes = " = 1";
				if(eDataType == eitTable) {
					strListTypes = ">= 2";
				}

				strFilterComboSource.Format("SELECT EMRDataGroupsT.ID, "
					"CASE "
					"	WHEN EMRDataT.ListType = 2 THEN "
					"		CASE WHEN EmrInfoT.TableRowsAsFields = 1 THEN 'Column: ' ELSE 'Row: ' END "
					"	WHEN EMRDataT.ListType >= 3 THEN "
					"		CASE WHEN EmrInfoT.TableRowsAsFields = 1 THEN 'Row: ' ELSE 'Column: ' END "
					"	ELSE '' "
					"END + EMRDataT.Data AS Name, "
					"CASE WHEN EMRDataT.Inactive = 0 THEN 1 ELSE 0 END AS IsVisible "
					"FROM EMRInfoMasterT WITH(NOLOCK) "
					"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
					"INNER JOIN EMRDataT WITH(NOLOCK) ON EMRInfoT.ID = EMRDataT.EMRInfoID "
					"INNER JOIN EMRDataGroupsT WITH(NOLOCK) ON EMRDataT.EMRDataGroupID = EMRDataGroupsT.ID "
					"WHERE EMRInfoT.EMRInfoMasterID = %li AND EMRDataT.IsLabel = 0 "
					"AND EMRDataT.ListType %s "
					"ORDER BY CASE WHEN ListType >= 3 THEN 1 ELSE 0 END, EMRDataT.SortOrder", nEMRInfoMasterID, strListTypes);
			}
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(cftComboSimple);
			pfs->PutEditable(VARIANT_TRUE);
			//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection, but leave the nolock hint since SQL2000 may do locking otherwise
			pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteDataSnapshot())); //we're going to let this combo use Practice's snapshot connection
			pfs->PutEmbeddedComboDropDownWidth(200);
			pfs->PutComboSource(_bstr_t(strFilterComboSource));

		} else if (eOperator == eadotContains) {
			pfs->PutDataType(VT_BSTR);
			pfs->PutFieldType(cftTextSingleLine);
			pfs->PutEditable(VARIANT_TRUE);
		}

		pRow->PutRefCellFormatOverride(iloDataFilter, pfs);

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::GetDataFilterFormatSettings");
}

LRESULT CEMRAnalysisConfigDlg::OnStartEditingFilterList(WPARAM wParam, LPARAM lParam)
{
	try {

		LPDISPATCH lpRow = (LPDISPATCH)wParam;
		IRowSettingsPtr pRow(lpRow);

		m_ItemList->CurSel = pRow;
		m_ItemList->StartEditing(pRow, iloDataFilter);
	
	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnStartEditingFilterList");

	return 0;
}
void CEMRAnalysisConfigDlg::OnRadioFilterOnPatient()
{
	try {

		//update the any/all label with our selected record filter
		CString strLabel;
		if(m_radioFilterOnPatient.GetCheck()) {
			strLabel = "Show results when a patient has:";
		}
		else if(m_radioFilterOnEMN.GetCheck()) {
			strLabel = "Show results when an EMN has:";
		}
		else if(m_radioFilterOnEMR.GetCheck()) {
			strLabel = "Show results when an EMR has:";
		}

		m_nxstaticAnyAll.SetWindowText(strLabel);

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioFilterOnPatient");
}

void CEMRAnalysisConfigDlg::OnRadioFilterOnEmn()
{
	try {

		OnRadioFilterOnPatient();

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioFilterOnEmn");
}

void CEMRAnalysisConfigDlg::OnRadioFilterOnEmr()
{
	try {

		OnRadioFilterOnPatient();

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioFilterOnEmr");
}

void CEMRAnalysisConfigDlg::OnRadioAllEmrItems()
{
	try {

		//disable the record filters if "any" is chosen
		BOOL bEnabled = m_radioAllItems.GetCheck();
		m_radioFilterOnPatient.EnableWindow(bEnabled);
		m_radioFilterOnEMN.EnableWindow(bEnabled);
		m_radioFilterOnEMR.EnableWindow(bEnabled);

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioAllEmrItems");
}

void CEMRAnalysisConfigDlg::OnRadioAnyEmrItems()
{
	try {

		OnRadioAllEmrItems();

	}NxCatchAll("Error in CEMRAnalysisConfigDlg::OnRadioAnyEmrItems");
}