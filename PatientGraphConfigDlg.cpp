// PatientGraphConfigDlg.cpp : implementation file
// (d.thompson 2009-05-20) - PLID 28486
//

#include "stdafx.h"
#include "Practice.h"
#include "PatientGraphConfigDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "MsgBox.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CPatientGraphConfigDlg dialog

#define nColListType 3 //we only support text columns
#define nRowListType 2

enum eGraphOptionColumns {
	egocID = 0,
	egocName,
	egocDoNotGraphUnfilled,
};

// (j.gruber 2010-02-09 16:29) - PLID 37291 - added columns
enum eNewItemColumns {
	enicMasterID = 0,
	enicName,
	enicDataType,
	enicIsFlipped,
};


enum eSelItemColumns {
	esicGraphDetailID = 0,
	esicOptionID,
	esicName,
	esicDataType,
	esicMasterID,
	esicColEMRDataGroupID,
	esicRowEMRDataGroupID,
	esicGraphAllRows,
	esicGraphAllCols,
	esicDateEMRDataGroupID,
	esicIsFlipped,
};

// (j.gruber 2010-02-17 10:02) - PLID 37396
enum eStatNormColumns {
	sncID = 0,
	sncName,
};

IMPLEMENT_DYNAMIC(CPatientGraphConfigDlg, CNxDialog)

CPatientGraphConfigDlg::CPatientGraphConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientGraphConfigDlg::IDD, pParent)
{

}

CPatientGraphConfigDlg::~CPatientGraphConfigDlg()
{
}

void CPatientGraphConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_GRAPH_OPTION, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_GRAPH_OPTION, m_btnDelete);
	DDX_Control(pDX, IDC_REMOVE_DETAIL, m_btnRemoveSelected);
	DDX_Control(pDX, IDOK, m_btnPreview);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_GRAPH_DO_NOT_FILL_EMPTY, m_btnDoNotFillEmpty);
	DDX_Control(pDX, IDC_GRAPH_BY_AGE, m_btnGraphAge);
	DDX_Control(pDX, IDC_GRAPH_BY_DATE, m_btnGraphDate);
}


BEGIN_MESSAGE_MAP(CPatientGraphConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_GRAPH_OPTION, &CPatientGraphConfigDlg::OnBnClickedAddGraphOption)
	ON_BN_CLICKED(IDC_DELETE_GRAPH_OPTION, &CPatientGraphConfigDlg::OnBnClickedDeleteGraphOption)
	ON_BN_CLICKED(IDC_REMOVE_DETAIL, &CPatientGraphConfigDlg::OnBnClickedRemoveDetail)
	ON_BN_CLICKED(IDC_GRAPH_DO_NOT_FILL_EMPTY, &CPatientGraphConfigDlg::OnBnClickedGraphDoNotFillEmpty)
	ON_BN_CLICKED(IDC_GRAPH_BY_DATE, &CPatientGraphConfigDlg::OnBnClickedGraphByDate)
	ON_BN_CLICKED(IDC_GRAPH_BY_AGE, &CPatientGraphConfigDlg::OnBnClickedGraphByAge)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientGraphConfigDlg, CNxDialog)
	ON_EVENT(CPatientGraphConfigDlg, IDC_GRAPH_OPTION_LIST, 16, CPatientGraphConfigDlg::SelChosenGraphOptionList, VTS_DISPATCH)
	ON_EVENT(CPatientGraphConfigDlg, IDC_ADD_GRAPH_DETAIL_LIST, 16, CPatientGraphConfigDlg::SelChosenAddGraphDetailList, VTS_DISPATCH)
	ON_EVENT(CPatientGraphConfigDlg, IDC_CHOSEN_GRAPH_DETAIL_LIST, 10, CPatientGraphConfigDlg::EditingFinishedChosenGraphDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientGraphConfigDlg, IDC_GRAPH_OPTION_LIST, 1, CPatientGraphConfigDlg::SelChangingGraphOptionList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPatientGraphConfigDlg, IDC_CHOSEN_GRAPH_DETAIL_LIST, 8, CPatientGraphConfigDlg::EditingStartingChosenGraphDetailList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPatientGraphConfigDlg, IDC_CHOSEN_GRAPH_DETAIL_LIST, 9, CPatientGraphConfigDlg::EditingFinishingChosenGraphDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPatientGraphConfigDlg, IDC_STATISTICAL_NORM_LIST, 18, CPatientGraphConfigDlg::RequeryFinishedStatisticalNormList, VTS_I2)
END_EVENTSINK_MAP()

// CPatientGraphConfigDlg message handlers
BOOL CPatientGraphConfigDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Interface setup
		//Load once, adds and deletes will manually update
		m_pGraphList = BindNxDataList2Ctrl(IDC_GRAPH_OPTION_LIST, true);
		//Load once, this list will never change during the duration of this dialog
		m_pAddDetailList = BindNxDataList2Ctrl(IDC_ADD_GRAPH_DETAIL_LIST, false);
		// (j.gruber 2010-02-17 09:57) - PLID 37396 - add statistical norm drop down
		m_pStatNormList = BindNxDataList2Ctrl(IDC_STATISTICAL_NORM_LIST, true);


		// (j.gruber 2010-02-17 10:14) - PLID 37396 - get their last option
		g_propManager.CachePropertiesInBulk("PatientGraphConfig", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'PatientGraphChoice') ", _Q(GetCurrentUserName()));			
		
		long nLastChoice = GetRemotePropertyInt("PatientGraphChoice", 0, 0, GetCurrentUserName());

		if (nLastChoice == 0 ) {
			m_btnGraphDate.SetCheck(1);
			m_btnGraphAge.SetCheck(0);
			OnBnClickedGraphByDate();
		}
		else {
			m_btnGraphDate.SetCheck(0);
			m_btnGraphAge.SetCheck(1);
			OnBnClickedGraphByAge();
		}

		// (j.gruber 2010-02-09 16:31) - PLID 37291 - added formula cells
		// (j.jones 2010-07-28 13:03) - PLID 39029 - do not show the generic table (DataSubType = 3)
		CString strFromClause;
		strFromClause.Format("(SELECT EMRInfoMasterT.ID, EMRInfoT.Name,  "
			" CASE WHEN EMRInfoT.DataType = 1 THEN 'Text' ELSE CASE WHEN EMRInfoT.DataType = 2 THEN 'Single Select' "
			" ELSE CASE WHEN EMRInfoT.DataType = 5 THEN 'Slider' ELSE CASE WHEN EMRInfoT.DataType = 7 THEN 'Table' END END  END END as DataType, "
			" EMRInfoT.TableRowsAsFields as IsFlipped "
			" FROM EMRInfoMasterT INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEmrInfoID = EMRInfoT.ID "
			" WHERE DataType IN (1, 2, 5, 7) AND EMRInfoMasterT.Inactive = 0 "
			" AND EMRInfoT.DataSubType <> %li) AS Q", eistGenericTable);
		m_pAddDetailList->FromClause = _bstr_t(strFromClause);
			

		m_pAddDetailList->Requery();

		//Do not load until we've chosen a graph option
		m_pChosenDetailList = BindNxDataList2Ctrl(IDC_CHOSEN_GRAPH_DETAIL_LIST, false);

		m_pChosenDetailList->FromClause = " ( SELECT GraphoptionDetailsT.ID as DetailID, OptionID, EMRInfoT.Name,  "
			" CASE WHEN EMRInfoT.DataType = 1 THEN 'Text' ELSE CASE WHEN EMRInfoT.DataType = 2 THEN 'Single Select' "
			" ELSE CASE WHEN EMRInfoT.DataType = 5 THEN 'Slider' ELSE CASE WHEN EMRInfoT.DataType = 7 THEN 'Table' END END  END END as DataType, EMRInfoMasterT.ID,  "
			" ColEMRDataGroupID, RowEMRDataGroupID, GraphAllRows, GraphAllCols, DateEMRDataGroupID, "
			" EMRInfoT.TableRowsAsFields as IsFlipped "
			" FROM GraphOptionDetailsT  "
			" INNER JOIN EMRInfoMasterT ON GraphOptionDetailsT.EMRInfoMasterID = EMRInfoMasterT.ID  "
			" INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEmrInfoID = EMRInfoT.ID "
			" WHERE DataType IN (1, 2, 5, 7)  ) Q ";
		

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnRemoveSelected.AutoSet(NXB_DELETE);
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		m_btnCancel.AutoSet(NXB_CANCEL);


		//Default to the first selection
		m_pGraphList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pGraphList->CurSel = m_pGraphList->GetFirstRow();
		LoadInterfaceForCurrentSelection();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CPatientGraphConfigDlg::SelChosenGraphOptionList(LPDISPATCH lpRow)
{
	try {
		//Just simply load the interface for this selection
		LoadInterfaceForCurrentSelection();

	} NxCatchAll(__FUNCTION__);
}

void CPatientGraphConfigDlg::SelChosenAddGraphDetailList(LPDISPATCH lpRow)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pSelRow(lpRow);
		//Get the InfoMasterID, it's needed to save
		long nInfoMasterID = VarLong(pSelRow->GetValue(enicMasterID));

		//also get the graph
		IRowSettingsPtr pGraphRow = m_pGraphList->CurSel;
		if(pGraphRow == NULL) {
			//shouldn't happen, just quit
			return;
		}
		long nGraphID = VarLong(pGraphRow->GetValue(egocID));


		//First, see if it's already in our list.  We don't need it twice.
		// (j.gruber 2010-02-09 16:42) - PLID 37291 - since we only support graphing one 'thing' at a time, they should never need to graph 
		// multiple cells in the same tables that aren't in the same row or column (ie: they couldn't choose {all} for one)
		if(m_pChosenDetailList->FindByColumn(esicMasterID, (long)nInfoMasterID, NULL, VARIANT_TRUE) != NULL) {
			AfxMessageBox("The selected EMR Item already exists for this graph.  It cannot be added again.");
			return;			
		}		

		CString strDataType = VarString(pSelRow->GetValue(enicDataType), "");
		BOOL bIsTable = FALSE;

		if (strDataType == "Table") {
			bIsTable = TRUE;
		}

		//Add it to data
		// (j.gruber 2010-02-09 16:46) - PLID 37291 - added fields		
		_RecordsetPtr prs; 
		prs = CreateParamRecordset(
			"SET NOCOUNT ON;\r\n"
			"INSERT INTO GraphOptionDetailsT (OptionID, EMRInfoMasterID) values ({INT}, {INT});\r\n"
			"SET NOCOUNT OFF;\r\n"
			"SELECT convert(int, @@identity) AS NewID;\r\n", nGraphID, nInfoMasterID);
	

		long nNewID = AdoFldLong(prs, "NewID");

		//add it to the detail list manually
		IRowSettingsPtr pNewRow = m_pChosenDetailList->GetNewRow();
		pNewRow->PutValue(esicGraphDetailID, (long)nNewID);
		pNewRow->PutValue(esicName, pSelRow->GetValue(enicName));
		pNewRow->PutValue(esicDataType, pSelRow->GetValue(enicDataType));
		pNewRow->PutValue(esicMasterID, (long)nInfoMasterID);
		pNewRow->PutValue(esicColEMRDataGroupID, g_cvarNull);
		pNewRow->PutValue(esicRowEMRDataGroupID, g_cvarNull);
		pNewRow->PutValue(esicGraphAllRows, bIsTable ? g_cvarFalse : g_cvarNull);	
		pNewRow->PutValue(esicGraphAllCols, bIsTable ? g_cvarFalse : g_cvarNull);	
		pNewRow->PutValue(esicDateEMRDataGroupID, g_cvarNull);	
		pNewRow->PutValue(esicIsFlipped, pSelRow->GetValue(enicIsFlipped));	
		m_pChosenDetailList->AddRowSorted(pNewRow, NULL);

		if (bIsTable) {
			LoadColumnValuesForOneRow(pNewRow, nColListType, esicColEMRDataGroupID, TRUE);
			LoadColumnValuesForOneRow(pNewRow, nRowListType, esicRowEMRDataGroupID, TRUE);
		}

		//Clear the selection so its fresh for the next go-round
		m_pAddDetailList->CurSel = NULL;

	} NxCatchAll(__FUNCTION__);
}

void CPatientGraphConfigDlg::OnBnClickedAddGraphOption()
{
	try {
		CString strResult;
		if(InputBox(this, "Please enter a new graph option name", strResult, "") == IDOK && strResult.GetLength() > 0) {
			//I am not concerned about duplicate names, there's no harm in them.
			//Trim to 100 character limit
			if(strResult.GetLength() > 100) {
				AfxMessageBox("The name is limited to 100 characters, your name will be truncated.");
				strResult = strResult.Left(100);
			}

			//Now insert to data
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON;\r\n"
				"INSERT INTO GraphOptionsT (Name, DoNotGraphUnfilled) values ({STRING}, 0);\r\n"
				"SET NOCOUNT OFF;\r\n"
				"SELECT convert(int, @@identity) as NewID;\r\n", strResult);

			if(prs->eof) {
				//don't know why this would fail... just refresh
				m_pGraphList->Requery();
				LoadInterfaceForCurrentSelection();
			}
			else {
				long nNewID = AdoFldLong(prs, "NewID");

				//Add to the graph list
				IRowSettingsPtr pRow = m_pGraphList->GetNewRow();
				pRow->PutValue(egocID, (long)nNewID);
				pRow->PutValue(egocName, _bstr_t(strResult));
				pRow->PutValue(egocDoNotGraphUnfilled, g_cvarFalse);
				m_pGraphList->AddRowSorted(pRow, NULL);

				//Make it the current selection
				m_pGraphList->CurSel = pRow;

				//Load
				LoadInterfaceForCurrentSelection();
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CPatientGraphConfigDlg::OnBnClickedDeleteGraphOption()
{
	try {
		IRowSettingsPtr pRow = m_pGraphList->CurSel;
		if(pRow == NULL) {
			return;
		}

		//there are no references to this data, so confirm and wipe
		if(AfxMessageBox("Are you sure you wish to remove this graph configuration?", MB_YESNO) != IDYES) {
			return;
		}

		long nID = VarLong(pRow->GetValue(egocID));
		ExecuteParamSql("DELETE FROM GraphOptionDetailsT WHERE OptionID = {INT};\r\n"
			"DELETE FROM GraphOptionsT WHERE ID = {INT};", nID, nID);

		//remove from the datalist
		m_pGraphList->RemoveRow(pRow);

		//just pick the first row to load
		m_pGraphList->CurSel = m_pGraphList->GetFirstRow();

		//Load the screen
		LoadInterfaceForCurrentSelection();

	} NxCatchAll(__FUNCTION__);
}

void CPatientGraphConfigDlg::OnBnClickedRemoveDetail()
{
	try {
		IRowSettingsPtr pRow = m_pChosenDetailList->CurSel;
		IRowSettingsPtr pGraphRow = m_pGraphList->CurSel;
		if(pRow == NULL || pGraphRow == NULL) {
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you wish to remove this item from being graphed?", MB_YESNO) != IDYES) {
			return;
		}

		//We need the graph ID and the item record ID
		long nGraphID = VarLong(pGraphRow->GetValue(egocID));
		long nDetailID = VarLong(pRow->GetValue(esicGraphDetailID));

		//Wipe it out
		ExecuteParamSql("DELETE FROM GraphOptionDetailsT WHERE OptionID = {INT} AND ID = {INT};", nGraphID, nDetailID);

		//clear from the screen
		m_pChosenDetailList->RemoveRow(pRow);

	} NxCatchAll(__FUNCTION__);
}

BOOL CPatientGraphConfigDlg::VerifyList() {

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChosenDetailList->GetFirstRow();
	BOOL bVerified = TRUE;
	while (pRow) {

		//see if this is a table field
		CString strDataType = VarString(pRow->GetValue(esicDataType), "");
		if (strDataType == "Table") {

			//make sure nothing is empty			
			
			_variant_t varCol = pRow->GetValue(esicColEMRDataGroupID);
			long nColVal = -2;

			if (varCol.vt == VT_I4) {
				nColVal = VarLong(varCol);
			}
			else if (varCol.vt == VT_BSTR) {
				nColVal = atoi(VarString(varCol, "-1"));
			}
			else {
				//not possible
				bVerified = FALSE;
			}

			_variant_t varRow = pRow->GetValue(esicRowEMRDataGroupID);
			long nRowVal = -2;

			if (varRow.vt == VT_I4) {
				nRowVal = VarLong(varRow);
			}
			else if (varRow.vt == VT_BSTR) {
				nRowVal = atoi(VarString(varRow, "-1"));
			}
			else {
				//not possible
				bVerified = FALSE;
			}

			if (nRowVal == -2 || nColVal == -2) {

				//they didn't set one
				MsgBox("You must set values for both the Row Value and Column Values for Table Items.");
				return FALSE;
			}

			if (bVerified) {
				
				if (nColVal == -1) {
					//make sure rowid isn't
					if (nRowVal == -1) {
						bVerified = FALSE;
					}
				}
				
			}

			//see if they checked all rows			
			if (nColVal == -1 || nRowVal == -1) {

				//see if they have a date column picked
				_variant_t varDateID = pRow->GetValue(esicDateEMRDataGroupID);				
				long nDateGroupID = -1;
				if (varDateID.vt == VT_I4) {
					nDateGroupID = VarLong(varDateID, -1);
				}
				else if (varDateID.vt == VT_BSTR) {
					nDateGroupID = atoi(VarString(varDateID, "-1"));
				}

				if (nDateGroupID == -1) {
					bVerified = FALSE;
				}
			}			
		}
		pRow = pRow->GetNextRow();
	}
	
	if (!bVerified) {
		MsgBox("You must pick a date column if you want to graph all rows for a table formula column");
		return bVerified;
	}

	//now loop again checking for duplicates
	//removed since I realized we only graph one 'thing' per graph
	/*pRow = m_pChosenDetailList->GetFirstRow();
	while (pRow) {

		long nDetail = VarLong(pRow->GetValue(esicGraphDetailID));
		
		_variant_t varCol = pRow->GetValue(esicColEMRDataGroupID);
		long nColVal;

		if (varCol.vt == VT_I4) {
			nColVal = VarLong(varCol);
		}
		else if (varCol.vt == VT_BSTR) {
			nColVal = atoi(VarString(varCol, "-1"));
		}	

		_variant_t varRow = pRow->GetValue(esicRowEMRDataGroupID);
		long nRowVal;

		if (varRow.vt == VT_I4) {
			nRowVal = VarLong(varRow);
		}
		else if (varRow.vt == VT_BSTR) {
			nRowVal = atoi(VarString(varRow, "-1"));
		}	

		if (nColVal == -1) {
			//we can't have another row with this row value
			if (pRow->GetNextRow())  {
				if(m_pChosenDetailList->FindByColumn(esicRowEMRDataGroupID, (long)nRowVal, pRow->GetNextRow(), VARIANT_TRUE) != NULL) {
					bVerified = FALSE;
				}
			}
		}		

		
		if (nRowVal == -1) {
			//we can't have another row with this column value
			if (pRow->GetNextRow()) {
				if(m_pChosenDetailList->FindByColumn(esicColEMRDataGroupID, (long)nColVal, pRow->GetNextRow(), VARIANT_TRUE) != NULL) {
					bVerified = FALSE;
				}
			}
		}

		if (bVerified) {

			//loop to check for dups
			NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pChosenDetailList->GetFirstRow();
			while (pCheckRow) {
				long nCheckDetail = VarLong(pCheckRow->GetValue(esicGraphDetailID));

				if (nDetail != nCheckDetail) {
					_variant_t varCheckCol = pCheckRow->GetValue(esicColEMRDataGroupID);
					long nCheckColVal;

					if (varCheckCol.vt == VT_I4) {
						nCheckColVal = VarLong(varCheckCol);
					}
					else if (varCheckCol.vt == VT_BSTR) {
						nCheckColVal = atoi(VarString(varCheckCol, "-1"));
					}	

					_variant_t varCheckRow = pCheckRow->GetValue(esicRowEMRDataGroupID);
					long nCheckRowVal;

					if (varCheckRow.vt == VT_I4) {
						nCheckRowVal = VarLong(varCheckRow);
					}
					else if (varCheckRow.vt == VT_BSTR) {
						nCheckRowVal = atoi(VarString(varCheckRow, "-1"));
					}	

					if (nCheckColVal == nColVal && nCheckRowVal == nRowVal) {
						bVerified = FALSE;
					}

				}
				pCheckRow = pCheckRow->GetNextRow();
			}
		}

		pRow = pRow->GetNextRow();
	}


	if (!bVerified) {
		MsgBox("You cannot have duplicate table cell values.");
	}*/

	return bVerified;
}

void CPatientGraphConfigDlg::OnOK()
{
	try {
		IRowSettingsPtr pGraphRow = m_pGraphList->CurSel;
		if(pGraphRow == NULL) {
			return;
		}

		// (j.gruber 2010-02-09 17:46) - PLID 37291 - verify 
		if (!VerifyList() ) {
			return;
		}

		// (j.gruber 2010-02-17 10:16) - PLID 37396 - save our graphing choice
		long nChoice;
		if (m_btnGraphDate.GetCheck()) {
			nChoice = 0;
		}
		else {
			nChoice = 1;
		}
		SetRemotePropertyInt("PatientGraphChoice", nChoice, 0, GetCurrentUserName());

		// (d.thompson 2009-05-22) - PLID 34322 - Preview the graph data report with the right
		//	parameters.
		// (j.gruber 2010-02-17 11:09) - PLID 37378 - by age graph
		
		//Additionally, we need to send a filter for all options.  These are all aggregated into the
		//	appropriate query and sent via the hijacked strExtraText value.
		CString strOptionsQuery;
		if(m_btnDoNotFillEmpty.GetCheck()) {
			//This filter means that any empty values are dropped from the query.  We'll have to do a case-by-case
			//	filter on the datatype, determing what "not filled" means.

			//For Text options, Unfilled is defined as anything non-numeric.  An empty string is not numeric.  If you do not
			//	have this filter applied, non-numeric data will graph as 0.
			//DRT 6/19/2009 - Note that we add the + 'e0' to confirm it's a floating point convertable number.  IsNumeric
			//	will return true for things like '$10' or '4,200', which are then not convertable to int/float/etc.
			//(e.lally 2012-01-11) PLID 44774 - Created a new SQL function "AsNumericString" that behaves like the EMR table formulas and takes all the leading
				//numeric characters [-+.0-9][.0-9] and returns that substring. An empty string is returned if it is not numeric. This function replaces the IsNumeric check.
			strOptionsQuery += " (CASE WHEN EMRInfoT.Datatype = 1 THEN dbo.AsNumericString(EMRDetailsT.Text) ELSE 'non-empty text' END) <> '' ";
			//For Single select options:
			//	If there is no selection (Data is null), it is considered unfilled
			//	Otherwise, if the data that is selected is not numeric, it is considered unfilled
			strOptionsQuery += " and (CASE WHEN EMRInfoT.Datatype = 2 THEN (CASE WHEN EMRDataT.Data IS NULL THEN 0 ELSE (CASE WHEN dbo.AsNumericString(EMRDataT.Data) = '' THEN 0 ELSE -1 END) END) ELSE -1 END) <> 0 ";
			//For Slider options NULL (no selection) values are unfilled.  This allows selections of 0 to still graph.
			strOptionsQuery += " and (CASE WHEN EMRInfoT.DataType = 5 THEN EMRDetailsT.SliderValue ELSE -1 END) is not null ";
		}
		//More Options here as needed

		// (j.gruber 2010-02-10 11:01) - PLID 37148 - hijacked strExtraText for our huge sql for table formulas
		//also the external filter and extendedsql
		CString strValueTableName, strInfoTableName;

		strInfoTableName.Format("#TempEMRGraphInfo%lu", GetTickCount());
		strValueTableName.Format("#TempEMRGraphValues%lu", GetTickCount());		

		
		SetFormulaGraphSql(strValueTableName, strInfoTableName);	


		//Lastly, generate the graph title.
		CString strTemp = nChoice == 0 ? " by Date" : " by Age";
		CString strTitle = "Graph:  " + VarString(pGraphRow->GetValue(egocName)) + strTemp;
		CRParameterInfo* pParamInfo = new CRParameterInfo;
		pParamInfo->m_Name = "GraphTitle";
		pParamInfo->m_Data = strTitle;
		CPtrArray aryParams;
		aryParams.Add((void*)pParamInfo);
		
		if (nChoice == 0) {
			
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(663)]);		
			infReport.nPatient = GetActivePatientID();

			//Use the saExtraValues parameter to provide all the EMRInfoMasterIDs we need to filter on.
			IRowSettingsPtr pRow = m_pChosenDetailList->GetFirstRow();
			if(pRow == NULL) {
				//We must have at least 1 record to view
				AfxMessageBox("You must first select at least one EMR Item to be graphed.");
				return;
			}

			while(pRow != NULL) {
				long nInfoMasterID = VarLong(pRow->GetValue(esicMasterID));
				//requires a string
				infReport.AddExtraValue(FormatString("%li", nInfoMasterID));

				pRow = pRow->GetNextRow();
			}


			infReport.strExtendedSql = strValueTableName;
			infReport.strExternalFilter = strInfoTableName;

			if(!strOptionsQuery.IsEmpty()) {
				infReport.strExtraText = " AND " + strOptionsQuery;
			}

			//Launch the report with the given parameter and then cleanup
			RunReport(&infReport, &aryParams, TRUE, (CWnd*)this, "Patient Graph Data (PP)");
			ClearRPIParameterList(&aryParams);
		}
		else {

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(693)]);		
			infReport.nPatient = GetActivePatientID();

			//Use the saExtraValues parameter to provide all the EMRInfoMasterIDs we need to filter on.
			IRowSettingsPtr pRow = m_pChosenDetailList->GetFirstRow();
			if(pRow == NULL) {
				//We must have at least 1 record to view
				AfxMessageBox("You must first select at least one EMR Item to be graphed.");
				return;
			}

			while(pRow != NULL) {
				long nInfoMasterID = VarLong(pRow->GetValue(esicMasterID));
				//requires a string
				infReport.AddExtraValue(FormatString("%li", nInfoMasterID));

				pRow = pRow->GetNextRow();
			}


			infReport.strExtendedSql = strValueTableName;
			infReport.strExternalFilter = strInfoTableName;

			if(!strOptionsQuery.IsEmpty()) {
				infReport.strExtraText = " AND " + strOptionsQuery;
			}

			pRow = m_pStatNormList->CurSel;
			CString strStatName;
			if (pRow) {
				infReport.nExtraID = VarLong(pRow->GetValue(sncID));
				strStatName = VarString(pRow->GetValue(sncName), "");
			}
			else {
				infReport.nExtraID = -1;
			}

			long nGender = -1;
			if (infReport.nExtraID != -1) {
				//get the gender for this patient
				_RecordsetPtr rsGender = CreateParamRecordset("SELECT Gender FROM PersonT WHERE ID = {INT}", GetActivePatientID());
				if (!rsGender->eof) {
					nGender = AdoFldByte(rsGender, "Gender", -1);

					if (nGender != 1 && nGender != 2) {
						if (IDNO == MsgBox(MB_YESNO, "The Gender for this patient is not filled out.  This will cause no statistical norm data to populate on the report.\n Are you sure you wish to continue?")) {
							ClearRPIParameterList(&aryParams);
							return;
						}
					}

					infReport.nFilterID = nGender;
				}
				
			}

			pParamInfo = new CRParameterInfo;
			pParamInfo->m_Name = "StatName";
			pParamInfo->m_Data = strStatName;				
			aryParams.Add((void*)pParamInfo);

			pParamInfo = new CRParameterInfo;
			pParamInfo->m_Name = "Gender";
			pParamInfo->m_Data = nGender == 1 ? "Male" : nGender == 2 ? "Female" : "";				
			aryParams.Add((void*)pParamInfo);



			//Launch the report with the given parameter and then cleanup
			RunReport(&infReport, &aryParams, TRUE, (CWnd*)this, "Patient Graph Data By Age (PP)");
			ClearRPIParameterList(&aryParams);

		}

		//Quit the dialog
		CDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CPatientGraphConfigDlg::OnCancel()
{
	try {
		//make sure they have everything set correctly
		if (!VerifyList()) {
			return;
		}
		CDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

//Gets the current selection in the graph list, loads all data appropriately
void CPatientGraphConfigDlg::LoadInterfaceForCurrentSelection()
{
	//First, clear everything in the detail list regardless
	m_pChosenDetailList->Clear();

	IRowSettingsPtr pCurSel = m_pGraphList->CurSel;
	if(pCurSel == NULL) {
		//Nothing is selected, so disable the interface
		EnableInterface(FALSE);
	}
	else {
		//Things are selected, ensure we're enabled
		EnableInterface(TRUE);

		//Additionally, load the data
		long nID = VarLong(pCurSel->GetValue(egocID));
		//a)  Refresh the chosen list with the new info
		CString strWhere;
		strWhere.Format("OptionID = %li", nID);
		//m_pChosenDetailList->WhereClause = _bstr_t(strWhere);
		//m_pChosenDetailList->Requery();

		_RecordsetPtr rsDetails = CreateParamRecordset( " SELECT * FROM "
			" ( SELECT GraphoptionDetailsT.ID as DetailID, OptionID, EMRInfoT.Name,  "
			" CASE WHEN EMRInfoT.DataType = 1 THEN 'Text' ELSE CASE WHEN EMRInfoT.DataType = 2 THEN 'Single Select' "
			" ELSE CASE WHEN EMRInfoT.DataType = 5 THEN 'Slider' ELSE CASE WHEN EMRInfoT.DataType = 7 THEN 'Table' END END  END END as DataType, EMRInfoMasterT.ID,  "
			" CASE WHEN GraphAllCols <> 0 THEN -1 ELSE ColEMRDataGroupID END AS ColEMRDataGroupID, "
			" CASE WHEN GraphAllRows <> 0 THEN -1 ELSE RowEMRDataGroupID END AS RowEMRDataGroupID, "
			" GraphAllRows, GraphAllCols, DateEMRDataGroupID, "
			" EMRInfoT.TableRowsAsFields as IsFlipped "
			" FROM GraphOptionDetailsT  "
			" INNER JOIN EMRInfoMasterT ON GraphOptionDetailsT.EMRInfoMasterID = EMRInfoMasterT.ID  "
			" INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEmrInfoID = EMRInfoT.ID "
			" WHERE DataType IN (1, 2, 5, 7)  AND OptionID = {INT}) Q ", nID);

		while (!rsDetails->eof) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChosenDetailList->GetNewRow();
			if (pRow) {

				BOOL bIsFlipped = AdoFldBool(rsDetails, "IsFlipped");
				BOOL bIsTable = FALSE;
				CString strDataType = AdoFldString(rsDetails, "DataType");
				if (strDataType == "Table") {
					bIsTable = TRUE;
				}

				eSelItemColumns esicColumn, esicRow;
				if (bIsFlipped) {
					esicColumn = esicRowEMRDataGroupID;
					esicRow = esicColEMRDataGroupID;
				}
				else {
					esicColumn = esicColEMRDataGroupID;
					esicRow = esicRowEMRDataGroupID;
				}				

				long nRowVal, nColVal, nDateVal;
				nRowVal = AdoFldLong(rsDetails, "RowEMRDataGroupID", -2);
				nColVal = AdoFldLong(rsDetails, "ColEMRDataGroupID", -2);
				nDateVal = AdoFldLong(rsDetails, "DateEMRDataGroupID", -2);

				pRow->PutValue(esicGraphDetailID, AdoFldLong(rsDetails, "DetailID"));
				pRow->PutValue(esicOptionID, AdoFldLong(rsDetails, "OptionID"));
				pRow->PutValue(esicName, _variant_t(AdoFldString(rsDetails, "Name", "")));
				pRow->PutValue(esicDataType, _variant_t(strDataType));
				pRow->PutValue(esicMasterID, AdoFldLong(rsDetails, "ID"));
				if (bIsTable) {
					pRow->PutValue(esicGraphAllCols, AdoFldBool(rsDetails, "GraphAllCols") ? g_cvarTrue : g_cvarFalse);
					pRow->PutValue(esicGraphAllRows, AdoFldBool(rsDetails, "GraphAllRows") ? g_cvarTrue : g_cvarFalse);
					pRow->PutValue(esicColumn, nColVal == -2 ? g_cvarNull: nColVal);
					pRow->PutValue(esicRow, nRowVal == -2 ? g_cvarNull: nRowVal);
					pRow->PutValue(esicDateEMRDataGroupID, nDateVal == -2 ? g_cvarNull: nDateVal);
					pRow->PutValue(esicIsFlipped, bIsFlipped ? g_cvarTrue : g_cvarFalse);
				}				

				m_pChosenDetailList->AddRowAtEnd(pRow, NULL);
			}

			LoadColumnValuesForAllRows();

			rsDetails->MoveNext();
		}	
		
		

		//b)  Set any options.  These are loaded in the graph option datalist to 
		//	avoid a recordset.
		m_btnDoNotFillEmpty.SetCheck(VarBool(pCurSel->GetValue(egocDoNotGraphUnfilled)));
	}
}


void CPatientGraphConfigDlg::LoadColumnValuesForAllRows() {

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChosenDetailList->GetFirstRow();
	while (pRow) {

		CString strDataType = VarString(pRow->GetValue(esicDataType), "");

		if (strDataType == "Table") {

			LoadColumnValuesForOneRow(pRow, nColListType, esicColEMRDataGroupID, TRUE);		
			LoadColumnValuesForOneRow(pRow, nRowListType, esicRowEMRDataGroupID, TRUE);		
			LoadDateColumnValuesForOneRow(pRow);
		}

		pRow = pRow->GetNextRow();
	}
}


void CPatientGraphConfigDlg::LoadColumnValuesForOneRow(LPDISPATCH lpRow, long nListType, int esic, BOOL bAddAll) {

	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	if (pRow) {

		//get the EMR Info value
		long nEMRInfoMasterID = VarLong(pRow->GetValue(esicMasterID));

		//check if its flipped
		if (VarBool(pRow->GetValue(esicIsFlipped)) && (esic == esicColEMRDataGroupID || esic == esicRowEMRDataGroupID)) {
			if (nListType == nRowListType) {
				nListType = nColListType;				
			}
			else if (nListType == nColListType) {
				nListType = nRowListType;				
			}
		}


		//now get our column values that aren't formulas
		_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDataGroupID, Data, Inactive FROM EMRDataT WHERE EMRInfoID = (SELECT ACtiveEmrInfoID FROM EMRInfoMasterT WHERE ID = {INT}) "
			" AND ListType = {INT} ", nEMRInfoMasterID, nListType);

		NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
		pfs->PutDataType(VT_BSTR);
		pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);								
		pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); 

		CString strComboSource;
		strComboSource = ";;1;;";

		if (bAddAll) {
			strComboSource += "-1;{All};1;";
		}
		
		while (!rs->eof) {

			CString strTemp;
			BOOL bInactive = AdoFldBool(rs, "Inactive");
			strTemp.Format("%li;%s;%li;", AdoFldLong(rs, "EmrDataGroupID"), AdoFldString(rs, "Data", ""), bInactive ? 0 : 1);
			strComboSource += strTemp;

			rs->MoveNext();
		}

		pfs->PutEditable(VARIANT_TRUE);
		pfs->PutComboSource(_bstr_t(strComboSource));
		pRow->PutRefCellFormatOverride(esic, pfs);
	}

}



void CPatientGraphConfigDlg::LoadDateColumnValuesForOneRow(LPDISPATCH lpRow) {

	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	if (pRow) {

		//get the EMR Info value
		long nEMRInfoMasterID = VarLong(pRow->GetValue(esicMasterID));

		//are we graphing all rows or all columns?
		BOOL bGraphAllCols = VarBool(pRow->GetValue(esicGraphAllCols), 0);
		BOOL bGraphAllRows = VarBool(pRow->GetValue(esicGraphAllRows), 0);

		long nListType = -1;

		if (bGraphAllCols) {
			nListType = nRowListType;
		}
		else if (bGraphAllRows) {
			nListType = nColListType;
		}
		else {
			//we aren't doing either, so return
			return;
		}

		LoadColumnValuesForOneRow(pRow, nListType, esicDateEMRDataGroupID, FALSE);
	}

}



//Enables or disables the entire interface based on the status of the graph option list.  The graph 
//	option list is never disabled, nor is the cancel button.
void CPatientGraphConfigDlg::EnableInterface(BOOL bEnable)
{
	m_pAddDetailList->PutReadOnly(!bEnable);
	m_pChosenDetailList->PutReadOnly(!bEnable);
	m_btnDelete.EnableWindow(bEnable);
	m_btnRemoveSelected.EnableWindow(bEnable);
	m_btnPreview.EnableWindow(bEnable);
	m_btnDoNotFillEmpty.EnableWindow(bEnable);
}

void CPatientGraphConfigDlg::OnBnClickedGraphDoNotFillEmpty()
{
	try {
		IRowSettingsPtr pRow = m_pGraphList->CurSel;
		if(pRow == NULL) {
			//shouldn't happen, just quit
			return;
		}

		long nID = VarLong(pRow->GetValue(egocID));

		long nNewOption = m_btnDoNotFillEmpty.GetCheck();

		//Update data
		ExecuteParamSql("UPDATE GraphOptionsT SET DoNotGraphUnfilled = {INT} WHERE ID = {INT};", nNewOption, nID);

		//Then update the row selected in the graph list
		pRow->PutValue(egocDoNotGraphUnfilled, nNewOption == 0 ? g_cvarFalse : g_cvarTrue);

	} NxCatchAll(__FUNCTION__);
}


void CPatientGraphConfigDlg::SaveChangesToData(NXDATALIST2Lib::IRowSettingsPtr pRow, long nNewValue, long nOldValue, short esic) 
{
	CString strField, strEMRGroupID;

	long nDetailID = VarLong(pRow->GetValue(esicGraphDetailID));
	short esicAllCol;

	
	if (VarBool(pRow->GetValue(esicIsFlipped))) {
		if (esic == esicColEMRDataGroupID) {
			esic = esicRowEMRDataGroupID;
		}
		else if (esic == esicRowEMRDataGroupID) {
			esic = esicColEMRDataGroupID;
		}
	}

	if (esic == esicColEMRDataGroupID) {
		strField = "GraphAllCols";
		strEMRGroupID = "ColEMRDataGroupID";		
		esicAllCol = esicGraphAllCols;
	}
	else if (esic == esicRowEMRDataGroupID) {
		strField = "GraphAllRows";
		strEMRGroupID = "RowEMRDataGroupID";
		esicAllCol = esicGraphAllRows;
	}

	if (nNewValue == -1) {
		ExecuteParamSql(FormatString("UPDATE GraphOptionDetailsT SET %s = 1, %s = NULL  WHERE ID = {INT}", 
			strField, strEMRGroupID), nDetailID);
		pRow->PutValue(esicAllCol, g_cvarTrue);
		LoadDateColumnValuesForOneRow(pRow);
	} 
	else {
		ExecuteParamSql(FormatString("UPDATE GraphOptionDetailsT SET %s = 0, %s = {INT} WHERE ID = {INT}", 
			strField, strEMRGroupID), nNewValue, nDetailID);
		pRow->PutValue(esicAllCol, g_cvarFalse);

		//check if we need to clear the date column
		if (nOldValue == -1) {
			//clear the date field
			ExecuteParamSql("UPDATE GraphOptionDetailsT SET DateEMRDataGroupID = NULL WHERE ID = {INT}", nDetailID);
			pRow->PutValue(esicDateEMRDataGroupID, g_cvarNull);			
		}								
	}


}

void CPatientGraphConfigDlg::EditingFinishedChosenGraphDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (bCommit) {

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow) {
				long nDetailID = VarLong(pRow->GetValue(esicGraphDetailID));
					
				switch (nCol) {

					case esicColEMRDataGroupID:
					case esicRowEMRDataGroupID:
						{
							long nNewVal;
							if (varNewValue.vt == VT_I4) {
								nNewVal = VarLong(varNewValue);
							}							
							else if (varNewValue.vt == VT_BSTR) {
								nNewVal = atoi(VarString(varNewValue));
							}
							else {
								ASSERT(FALSE);
							}		

							
							long nOldVal;
							if (varOldValue.vt == VT_I4) {
								nOldVal = VarLong(varOldValue);
							}							
							else if (varOldValue.vt == VT_BSTR) {
								nOldVal = atoi(VarString(varOldValue));
							}
							else if (varOldValue.vt == VT_EMPTY || varOldValue.vt == VT_NULL) {
								nOldVal = -2;
							}
							else {
								ASSERT(FALSE);
							}
							
							SaveChangesToData(pRow, nNewVal, nOldVal, nCol); 
							
						}
					break;	
						
					
					
					case esicDateEMRDataGroupID: 
						{
							_variant_t varValue = varNewValue;

							if (varValue.vt == VT_I4) {
								ExecuteParamSql("UPDATE GraphOptionDetailsT SET DateEMRDataGroupID = {INT} WHERE ID = {INT}", 
									VarLong(varNewValue), nDetailID);
							}
							else if (varValue.vt == VT_BSTR) {
								ExecuteParamSql("UPDATE GraphOptionDetailsT SET DateEMRDataGroupID = {INT} WHERE ID = {INT}", 
									atoi(VarString(varNewValue)), nDetailID);
							}
							else {
								ASSERT(FALSE);
							}	
						}
					break;
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

}

void CPatientGraphConfigDlg::SelChangingGraphOptionList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		
		BOOL bVerified = VerifyList();

		if (!bVerified) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll(__FUNCTION__);

}


// (j.gruber 2010-02-16 11:53) - PLID 37148
void CPatientGraphConfigDlg::SetFormulaGraphSql(CString strInfoTableName, CString strValuesTableName) 
{
	long nPatientID = GetActivePatientID();
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGraphList->CurSel;

	if (pRow) {
		
		long nGraphID = VarLong(pRow->GetValue(egocID));

		CString strSql;

		// (z.manning 2011-05-31 17:18) - PLID 43851 - Changed all converstions to float to now convert to decimal(38, 10)
		// so that the modulo functions won't cause errors.
		//(e.lally 2012-01-11) PLID 44774 - Created a new SQL function "AsNumericString" that behaves like the EMR table formulas and takes all the leading
			//numeric characters [-+.0-9][.0-9] and returns that substring. An empty string is returned if it is not numeric. This function replaces the IsNumeric check.
			//This function is not internationally compliant in the use of strings with commas for decimal points.
			/*Examples: 
			'12.99' -> '12.99'
			'$12.00' -> ''
			'55.88cm' -> '55.88'
			'+5.2' -> '+5.2'
			'.88' -> '.88'
			'-22x.77' -> '-22'
			'10,000' -> '10'
			'1/2/2000' -> '1'
			*/
			//Also fixed some bugs where the wrong column/row was being evaluated and a single empty cell was causing the whole formula to be considered unfilled.
		// (z.manning 2012-04-06 13:35) - PLID 43856 - I got rid of all of the formula evaluation stuff in here as formulas are
		// now stored in data so we no longer need to do anything special.
		strSql.Format("  CREATE TABLE %s (ID int PRIMARY KEY, Date datetime, PlotValue float(8))  \r\n"
			"  CREATE TABLE %s (ID int PRIMARY KEY, EMNID int, PatID int, LocID int, IDate datetime, TDate datetime, PatName nVarchar(200),  \r\n"
			"   	DetailName nvarChar(200), EMRInfoMasterID int, DecimalPlaces int)   \r\n"
			"  \r\n"
			"   SET NOCOUNT ON  \r\n"
			"  \r\n"
			"   DECLARE @nPatientID INT  \r\n" 
			"   SET @nPatientID = %li  \r\n"
			"  \r\n"
			"   DECLARE @nGraphOptionsID INT  \r\n"
			"   SET @nGraphOptionsID = %li  \r\n"
			"  \r\n"
			"	DECLARE @nFormulaForTransformFlag INT \r\n"
			"	SET @nFormulaForTransformFlag = %li \r\n"
			"   DECLARE @bDoNotGraphUnfilledData BIT  \r\n"
			"   DECLARE @nEMRInfoMasterID INT  \r\n"			
			"   DECLARE @bGraphAllRows BIT  \r\n"
			"   DECLARE @bGraphAllCols BIT \r\n"
			"   DECLARE @nDateDataGroupID INT  \r\n"
			"  \r\n"
			"   DECLARE @nRowDataGroupID INT \r\n"
			"   DECLARE @nColDataGroupID INT \r\n"
			" 		 \r\n"
			"  \r\n"
			"   DECLARE @nUniqueID INT  \r\n"
			"   SET @nUniqueID = 1  \r\n"
			"  \r\n"
			"   DECLARE rsGraphOptions CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n"
			"   	SELECT DoNotGraphUnfilled, EMRInfoMasterID, GraphAllRows, GraphAllCols, ColEMRDataGroupID, RowEMRDataGroupID, DateEMRDataGroupID  \r\n"
			"   	FROM GraphOptionsT INNER JOIN GraphOptionDetailsT ON GraphOptionsT.ID = GraphOptionDetailsT.OptionID  \r\n"
			"   	WHERE GraphOptionsT.ID = @nGraphOptionsID  \r\n"
			"   OPEN rsGraphOptions  \r\n"
			"   FETCH FROM rsGraphOptions INTO @bDoNotGraphUnfilledData, @nEMRInfoMasterID, @bGraphAllRows, @bGraphAllCols, @nColDataGroupID, @nRowDataGroupID, @nDateDataGroupID  \r\n"
			"   WHILE @@FETCH_STATUS = 0 BEGIN  \r\n"
			"     \r\n"
			"   	DECLARE @nActiveEMRInfoID INT  \r\n"
			"   	SET @nActiveEMRInfoID = (SELECT ActiveEMRInfoID FROM EMRInfoMasterT WHERE ID = @nEMRInfoMasterID)  \r\n"
			"     \r\n"
			"   	DECLARE @nEMRDataID_Y INT  \r\n"
			"   	DECLARE @nEMRDataID_X INT  \r\n"
			"     \r\n"
			"   	DECLARE @nDecimalPlaces INT  \r\n"
			"		SET @nDecimalPlaces = 2 \r\n"
			"     \r\n"
			"   	DECLARE @DateVal datetime  \r\n"
			"     \r\n"
			"   	DECLARE @bDateSet bit  \r\n"
			"   	SET @bDateSet = 0  \r\n"
			"     \r\n"
			"   	DECLARE @bNeedsFloatConversion BIT  \r\n"
			"     \r\n"
			"   	DECLARE @strMainFormula nVarchar(255)  \r\n"
			"     \r\n"
			"   	DECLARE @nEMRDetailID INT  \r\n"
			"   	DECLARE @dtEMRDate datetime  \r\n"
			"   	DECLARE @nEMRDetailsInfoID INT  \r\n"
			"   	DECLARE @nLocID INT  \r\n"
			"   	DECLARE @TDate datetime  \r\n"
			"   	DECLARE @IDate datetime  \r\n"
			"   	DECLARE @strPatName nvarchar(200)  \r\n"
			"   	DECLARE @strDetailName nVarchar(200)  \r\n"
			"   	DECLARE @nEMNID int	  \r\n"
			"     \r\n"
			"   	--loop through all the items of this type for this patient  \r\n"
			"   	DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n"
			"   		SELECT EMRDetailsT.ID, EMRMasterT.Date, EMRInfoT.ID, EMRMasterT.LocationID, EMRMasterT.Date, EMRMasterT.InputDate,   \r\n"
			"   				PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatName,    \r\n"
			"   				EMRInfoT.Name as DetailName, EMRMasterT.ID   \r\n"
			"   				FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID  \r\n"
			"   				LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID   \r\n"
			"   				LEFT JOIN PersonT AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID    \r\n"
			"   				WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType = 7 AND EMRMasterT.PatientID = @nPatientID AND EMRInfoT.EMRInfoMasterID = @nEMRInfoMasterID  \r\n"
			"   	OPEN rsItems  \r\n"
			"   	FETCH FROM rsItems INTO @nEMRDetailID, @dtEMRDate,@nEMRDetailsInfoID, @nLocID, @TDate, @IDate, @strPatName, @strDetailName, @nEMNID   \r\n"
			"   	WHILE @@FETCH_STATUS = 0 BEGIN  \r\n"
			"\r\n"
			" 		--first see if they want to graph all rows or columns \r\n"
			" 		IF (@bGraphAllRows = 0 AND @bGraphAllCols = 0) BEGIN \r\n"
			" 			--we just need to get one value from our cell \r\n"
			" 			DECLARE @TableCellVal decimal(38,10) \r\n"
			//(e.lally 2012-01-11) PLID 44774 - Use the AsNumericString function now to more accurately reflect how the EMR formulas work
			" 			SET @TableCellVal = (SELECT CONVERT(decimal(38,10), dbo.AsNumericString(Data)) FROM EMRDetailTableDataT   \r\n"
			"   									WHERE EMRDetailID = @nEMRDetailID AND EmrDataID_X = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nRowDataGroupID) \r\n"
			"   									AND dbo.AsNumericString(Data) <> ''  \r\n"
			"   									AND EmrDataID_Y = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nColDataGroupID))	 \r\n"
			"  \r\n"
			" 			IF @TableCellVal IS NULL BEGIN \r\n"
			" 				--this is unfilled data \r\n"
			" 				IF @bDoNotGraphUnfilledData = 0 BEGIN \r\n"
			" 					INSERT INTO %s (ID, Date, PlotValue) VALUES (@nUniqueID, @TDate, 0) \r\n"
			" 					 \r\n"
			" 					INSERT INTO %s (ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   						VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"
			"				SET @nUniqueID = @nUniqueID + 1 \r\n "
			" 				END \r\n"
			" 			END \r\n"
			" 			ELSE BEGIN \r\n"
			" 				--it's got something in it, graph it \r\n"
			" 				INSERT INTO %s(ID, Date, PlotValue) VALUES (@nUniqueID, @TDate, @TableCellVal) \r\n"
			" 				 \r\n"
			" 				INSERT INTO %s(ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   					VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"
			"				SET @nUniqueID = @nUniqueID + 1 \r\n "
			" 			END	 \r\n"
			" 			GOTO END_LOOP		 \r\n"
			" 		END \r\n"
			" 		 \r\n"
			" 		IF @bGraphAllRows = 1 BEGIN \r\n"
			" 			 \r\n"
			" 			DECLARE @TableDateValue datetime; \r\n"
			"   			DECLARE rsRowsNoForm CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR   \r\n"
			"   				SELECT ID FROM EMRDataT WHERE Inactive = 0 AND ListType = 2 AND EMRInfoID = @nEMRDetailsInfoID ORDER BY SortOrder ASC  \r\n"
			"   			OPEN rsRowsNoForm  \r\n"
			"   			FETCH FROM rsRowsNoForm INTO @nEMRDataID_X \r\n"
			"   			WHILE @@FETCH_STATUS = 0 BEGIN  \r\n"
			//(e.lally 2012-01-11) PLID 44774 - Use the AsNumericString function now to more accurately reflect how the EMR formulas work
			" 	  			SET @TableCellVal = (SELECT CONVERT(decimal(38,10), dbo.AsNumericString(Data)) FROM EMRDetailTableDataT   \r\n"
			"   									WHERE EMRDetailID = @nEMRDetailID AND EmrDataID_X = @nEMRDataID_X \r\n"
			" 									AND dbo.AsNumericString(Data) <> ''  \r\n"
			"  									AND EmrDataID_Y = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nColDataGroupID))	 \r\n"
			"  \r\n"
			" 				SET @TableDateValue = (SELECT CONVERT(dateTime,Data) FROM EMRDetailTableDataT   \r\n"
			" 									   WHERE EMRDetailID = @nEMRDetailID AND EmrDataID_X = @nEMRDataID_X  										 \r\n"
			"										AND IsDate(CONVERT(nVarChar, Data)) = 1 \r\n "
			"  				   					   AND EmrDataID_Y = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nDateDataGroupID))	 \r\n"
			" 				 \r\n"
			" 				IF @TableDateValue IS NULL BEGIN \r\n"
			" 					SET @TableDateValue = @TDate \r\n"
			" 				END \r\n"
			"  \r\n"
			" 				IF @TableCellVal IS NULL BEGIN \r\n"
			" 					--this is unfilled data \r\n"
			" 					IF @bDoNotGraphUnfilledData = 0 BEGIN \r\n"
			" 						INSERT INTO %s (ID, Date, PlotValue) VALUES (@nUniqueID, @TableDateValue, 0) \r\n"
			" 						 \r\n"
			" 						INSERT INTO %s(ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   							VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"			
			" 					END \r\n"
			" 				END \r\n"
			" 				ELSE BEGIN \r\n"
			" 					--it's got something in it, graph it \r\n"
			" 					INSERT INTO %s(ID, Date, PlotValue) VALUES (@nUniqueID, @TableDateValue, @TableCellVal) \r\n"
			"  \r\n"
			" 					INSERT INTO %s(ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   						VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"
			" 				END							 \r\n"
			" 				 \r\n"
			" 				SET @nUniqueID = @nUniqueID + 1 \r\n"
			" 				FETCH FROM rsRowsNoForm INTO @nEMRDataID_X \r\n"
			" 			END		 \r\n"
			"			DEALLOCATE rsRowsNoForm \r\n"
			" 			GOTO END_LOOP	 \r\n"
			" 		END \r\n"
			" 			 \r\n"
			" 		IF @bGraphAllCols = 1 BEGIN		 \r\n"
			" 				 \r\n"
			"   			DECLARE rsColsNoForm CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR   \r\n"
			"   			SELECT ID FROM EMRDataT WHERE Inactive = 0 AND ListType = 3 AND EMRInfoID = @nEMRDetailsInfoID ORDER BY SortOrder ASC  \r\n"
			"   			OPEN rsColsNoForm  \r\n"
			"   			FETCH FROM rsColsNoForm INTO @nEMRDataID_Y \r\n"
			"   			WHILE @@FETCH_STATUS = 0 BEGIN  \r\n"
			" 	  		 \r\n"
			//(e.lally 2012-01-11) PLID 44774 - Use the AsNumericString function now to more accurately reflect how the EMR formulas work
			" 					SET @TableCellVal = (SELECT CONVERT(decimal(38,10), dbo.AsNumericString(Data)) FROM EMRDetailTableDataT   \r\n"
			"   									WHERE EMRDetailID = @nEMRDetailID AND EmrDataID_Y = @nEMRDataID_Y \r\n"
			"   									AND dbo.AsNumericString(Data) <> '' \r\n"
			"   									AND EmrDataID_X = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nRowDataGroupID))	 \r\n"
			"  \r\n"
			" 					SET @TableDateValue = (SELECT CONVERT(dateTime,Data) FROM EMRDetailTableDataT   \r\n"
			"   										WHERE EMRDetailID = @nEMRDetailID AND EmrDataID_Y = @nEMRDataID_Y  										 \r\n"
			"											AND IsDate(CONVERT(nVarChar, Data)) = 1 \r\n "
			"   										AND EmrDataID_X = (SELECT ID FROM EMRDataT WHERE EMRInfoID = @nEMRDetailsInfoID AND EMRDataGroupID = @nDateDataGroupID))	 \r\n"
			"  \r\n"
			" 					IF @TableDateValue IS NULL BEGIN \r\n"
			" 						SET @TableDateValue = @TDate \r\n"
			" 					END \r\n"
			"  \r\n"
			" 					IF @TableCellVal IS NULL BEGIN \r\n"
			" 						--this is unfilled data \r\n"
			" 						IF @bDoNotGraphUnfilledData = 0 BEGIN \r\n"
			" 							INSERT INTO %s(ID, Date, PlotValue) VALUES (@nUniqueID, @TableDateValue, 0) \r\n"
			" 		 \r\n"
			" 							INSERT INTO %s(ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   								VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"
			" 						END \r\n"
			"					END  \r\n"
			" 					ELSE BEGIN \r\n"
			" 						--it's got something in it, graph it \r\n"
			" 						INSERT INTO %s(ID, Date, PlotValue) VALUES (@nUniqueID, @TableDateValue, @TableCellVal) \r\n"
			"  \r\n"
			" 						INSERT INTO %s(ID, EMNID, PatID, LocID, IDate, TDate, PatName,	DetailName, EMRInfoMasterID, DecimalPlaces)  \r\n"
			"   								VALUES (@nUniqueID, @nEMNID, @nPatientID, @nLocID, @IDate, @TDate, @strPatName, @strDetailName, @nEMRInfoMasterID, @nDecimalPlaces)  \r\n"
			" 					END							 \r\n"
			" 					 \r\n"						
			"				SET @nUniqueID = @nUniqueID + 1 \r\n"
			"				FETCH FROM rsColsNoForm INTO @nEMRDataID_Y \r\n"
			" 			END \r\n"
			"			DEALLOCATE rsColsNoForm \r\n"
			" 		END \r\n"
			"  \r\n"
			"  END_LOOP: \r\n"
			"   		FETCH FROM rsItems INTO @nEMRDetailID, @dtEMRDate,@nEMRDetailsInfoID, @nLocID, @TDate, @IDate, @strPatName, @strDetailName, @nEMNID  \r\n"
			"   	END  \r\n"
			"   	DEALLOCATE rsItems  \r\n"
			"     \r\n"
			"   	FETCH FROM rsGraphOptions INTO @bDoNotGraphUnfilledData, @nEMRInfoMasterID, @bGraphAllRows, @bGraphAllCols, @nColDataGroupID, @nRowDataGroupID, @nDateDataGroupID  \r\n"
			"   END  \r\n"
			"   DEALLOCATE rsGraphOptions  \r\n"
			"  SET NOCOUNT OFF \r\n"
			"  \r\n ", strInfoTableName, strValuesTableName, nPatientID, nGraphID, edfFormulaForTransform,
			strInfoTableName, strValuesTableName, strInfoTableName, strValuesTableName, strInfoTableName, strValuesTableName,
			strInfoTableName, strValuesTableName, strInfoTableName, strValuesTableName, strInfoTableName, strValuesTableName,
			strInfoTableName, strInfoTableName, strInfoTableName, strInfoTableName, strValuesTableName,
			strInfoTableName, strInfoTableName, strInfoTableName, strInfoTableName, strValuesTableName);	

#ifdef _DEBUG
			CMsgBox dlg(this);
			dlg.msg = strSql;
			dlg.DoModal();
#endif
			ExecuteSqlStd(GetRemoteDataSnapshot(), strSql);	

		}
		
			
} 

void CPatientGraphConfigDlg::EditingStartingChosenGraphDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRowEdit(lpRow);

			if (pRowEdit) {
				CString strDataType = VarString(pRowEdit->GetValue(esicDataType), "");
		
				switch (nCol) {				

					case esicRowEMRDataGroupID:
					case esicColEMRDataGroupID:
					case esicDateEMRDataGroupID:
						{
							if (strDataType != "Table") {
								*pbContinue = FALSE;
								return;
							}
							if (nCol == esicDateEMRDataGroupID) {
								BOOL bValCol = VarBool(pRowEdit->GetValue(esicGraphAllCols), FALSE);
								BOOL bValRow = VarBool(pRowEdit->GetValue(esicGraphAllRows), FALSE);
								if ((!bValCol) && (!bValRow)) {
									*pbContinue = FALSE;								
								}							
							}
						}
					break;
				}
			}

	}NxCatchAll(__FUNCTION__);
}

BOOL CPatientGraphConfigDlg::CanEdit(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol, long nNewValue) {

	BOOL bIsFlipped = VarBool(pRow->GetValue(esicIsFlipped));

	short nGraphAllField;

	if (bIsFlipped) {
		if (nCol == esicColEMRDataGroupID) {
			nGraphAllField = esicGraphAllCols;		
		}
		else if (nCol == esicRowEMRDataGroupID) {
			nGraphAllField = esicGraphAllRows;
		}
	}
	else {

		if (nCol == esicColEMRDataGroupID) {
			nGraphAllField = esicGraphAllRows;		
		}
		else if (nCol == esicRowEMRDataGroupID) {
			nGraphAllField = esicGraphAllCols;
		}
	}

	BOOL bGraphAll = VarBool(pRow->GetValue(nGraphAllField), FALSE);

	if (bGraphAll) {

		if (nNewValue == -1) {
			//nope, can't have all rows and all cols
			MsgBox("You cannot graph all rows and all columns.");
			return FALSE;
		}
	}	
	return TRUE;
}

void CPatientGraphConfigDlg::EditingFinishingChosenGraphDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		//make sure we don't have both the column and the row set to all
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			switch (nCol) {

				case esicColEMRDataGroupID:
				case esicRowEMRDataGroupID:
					{
					
						if (*pbCommit) {
							//check to see what our new value is
							_variant_t varNewVal =  *pvarNewValue;
							long nNewValue = -2;
								
							if (varNewVal.vt == VT_I4) {
								nNewValue = VarLong(varNewVal);
							}
							else if (varNewVal.vt == VT_BSTR) {
								nNewValue =  atoi(VarString(varNewVal));
							}
							else {
								//ASSERT(FALSE);
								*pbCommit = FALSE;
								return;
							}

							if (!CanEdit(pRow, nCol, nNewValue)) {
								*pbCommit = FALSE;
							}						
						}
					}

				break;		
					
			}

		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-02-17 10:01) - PLID 37396
void CPatientGraphConfigDlg::RequeryFinishedStatisticalNormList(short nFlags)
{
	try {

		//set the selection to None
		m_pStatNormList->SetSelByColumn(sncID, (long)-1);		

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-02-17 10:05) - PLID 37396
void CPatientGraphConfigDlg::OnBnClickedGraphByDate()
{
	try {

		if (IsDlgButtonChecked(IDC_GRAPH_BY_DATE)) {
			GetDlgItem(IDC_STATISTICAL_NORM_LIST)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_STATISTICAL_NORM_LIST)->EnableWindow(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-02-17 10:05) - PLID 37396
void CPatientGraphConfigDlg::OnBnClickedGraphByAge()
{
	try {
		if (IsDlgButtonChecked(IDC_GRAPH_BY_AGE)) {
			GetDlgItem(IDC_STATISTICAL_NORM_LIST)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_STATISTICAL_NORM_LIST)->EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}
