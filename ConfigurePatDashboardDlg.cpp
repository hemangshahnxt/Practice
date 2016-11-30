// ConfigurePatDashboardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigurePatDashboardDlg.h"
#include "PatientsRc.h"
#include "AddPatDashboardControlDlg.h"
// (j.gruber 2012-07-02 12:15) - PLID 51210
#include "PatDashDiagramDlg.h"
#include <sstream>

// (j.gruber 2012-04-13 15:46) - PLID 49700 - created for

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum UserListColumns
{
	ulcID = 0,
	ulcTitle,
	ulcType,
	ulcBlock,
	ulcPosition,
	ulcOrder,
	ulcFilters,
};


enum AdminListColumns
{
	alcID = 0,
	alcTitle,	
	alcType,
	alcFilters,
};

/////////////////////////////////////////////////////////////////////////////
// CConfigurePatDashboardDlg dialog

// Removes missing values from dashboard category filters
/* static */ void CConfigurePatDashboardDlg::RemoveMissingFilterValues(PatientDashboardType dashboardType,
	FilterType filterType, const CString& strCatTable, const CString& strCatColumn)
{
	// (c.haag 2016-05-05 15:24) - NX-100441 - Initial implementation
	ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(R"(
SELECT DISTINCT [%s] FROM [%s];

SELECT PatientDashboardControlFiltersT.ControlID, PatientDashboardControlFiltersT.FilterValueText
FROM PatientDashboardControlFiltersT
INNER JOIN PatientDashboardControlsT ON 
	PatientDashboardControlFiltersT.ControlID = PatientDashboardControlsT.ID
	AND PatientDashboardControlsT.TypeID = {CONST_INT}
WHERE
	PatientDashboardControlFiltersT.FilterTypeID = {CONST_INT}
)", strCatColumn, strCatTable)
	, pdtHistoryAttachments
	, HistoryCategory
	);

	// Read in the existing ID's
	std::vector<int> existingIDs;
	while (!prs->eof)
	{
		existingIDs.push_back(AdoFldLong(prs, strCatColumn));
		prs->MoveNext();
	}

	// Read in the dashboard configurations and build the batch statement for updating
	// them all to no longer have missing records
	CParamSqlBatch batch;
	prs = prs->NextRecordset(nullptr);
	while (!prs->eof)
	{
		// Get the existing category ID's as a vector
		CString strFilterValueText = AdoFldString(prs, "FilterValueText", "");
		strFilterValueText.Replace("(", "");
		strFilterValueText.Replace(")", "");
		strFilterValueText.Replace(" ", "");

		std::stringstream ss((LPCTSTR)strFilterValueText);
		std::vector<int> dashboardIDs;
		int i;
		while (ss >> i)
		{
			dashboardIDs.push_back(i);
			if (ss.peek() == ',')
				ss.ignore();
		}

		// Build strNewDashboardIDs to be all the ID's that are in both existingIDs and dashboardIDs
		CString strNewDashboardIDs;
		bool bMissingElements = false;
		for (auto a : dashboardIDs)
		{
			if (std::find(existingIDs.begin(), existingIDs.end(), a) != existingIDs.end())
			{
				strNewDashboardIDs += FormatString("%d, ", a);
			}
			else
			{
				bMissingElements = true;
			}
		}

		// Build the batch statement if we need to remove elements
		if (bMissingElements)
		{
			CString strWhere = FormatString("WHERE ControlID = %d AND FilterTypeID = %d", AdoFldLong(prs, "ControlID"), (int)filterType);
			if (strNewDashboardIDs.IsEmpty())
			{
				batch.AddFormat("DELETE FROM PatientDashboardControlFiltersT %s", strWhere);
			}
			else
			{
				strNewDashboardIDs.TrimRight(", ");
				strNewDashboardIDs = CString("(") + strNewDashboardIDs + ")";
				batch.AddFormat("UPDATE PatientDashboardControlFiltersT SET FilterValueText = '%s' %s"
					, _Q(strNewDashboardIDs), strWhere);
			}
		}

		prs->MoveNext();
	}

	if (!batch.IsEmpty())
	{
		batch.Execute(GetRemoteData());
	}
}

CConfigurePatDashboardDlg::CConfigurePatDashboardDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigurePatDashboardDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigurePatDashboardDlg)
	
	//}}AFX_DATA_INIT
}

CConfigurePatDashboardDlg::~CConfigurePatDashboardDlg()
{
	try {
		//get rid of all the memory
		POSITION pos = m_controls.GetStartPosition();
		long nID;
		PDControl *pControl = NULL;		

		while(pos) {
			m_controls.GetNextAssoc(pos, nID, pControl);

			//remove it
			m_controls.RemoveKey(nID);
			
			//first delete all the filters
			for (int i = pControl->aryFilters.GetSize()-1; i >= 0; i--) {
				PDFilter *filter = pControl->aryFilters[i];
				pControl->aryFilters.RemoveAt(i);
				if (filter) {
					delete filter;
				}
			}

			//now delete the control
			delete pControl;				

		}

		m_controls.RemoveAll();


		//now do our deleted controls
		pos = m_deletedControls.GetStartPosition();	
		PDControl *pDelControl = NULL;		

		while(pos) {
			m_deletedControls.GetNextAssoc(pos, nID, pDelControl);

			//remove it
			m_deletedControls.RemoveKey(nID);
			
			//first delete all the filters
			for (int i = pDelControl->aryFilters.GetSize()-1; i >= 0; i--) {
				PDFilter *filter = pDelControl->aryFilters[i];
				pDelControl->aryFilters.RemoveAt(i);
				if (filter) {
					delete filter;
				}
			}

			//now delete the control
			delete pDelControl;				

		}
		m_deletedControls.RemoveAll();

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigurePatDashboardDlg)		
	DDX_Control(pDX, IDC_CPD_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_CPD_MOVE_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_CPD_MOVE_LEFT, m_btnMoveLeft);
	DDX_Control(pDX, IDC_CPD_MOVE_RIGHT, m_btnMoveRight);
	DDX_Control(pDX, IDOK, m_btnOK);		
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CPD_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_CPD_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_CPD_EDIT_SELECTED, m_btnEditBottom);
	DDX_Control(pDX, IDC_CPD_REMOVE, m_btnRemove);	
	DDX_Control(pDX, IDC_CONFIGURE_PAT_DASH_BKG, m_bkg);
	DDX_Control(pDX, IDC_COLUMN_DIAGRAM, m_diagramlink);
		//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigurePatDashboardDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigurePatDashboardDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CPD_ADD, &CConfigurePatDashboardDlg::OnBnClickedCpdAdd)
	ON_BN_CLICKED(IDC_CPD_MOVE_RIGHT, &CConfigurePatDashboardDlg::OnBnClickedCpdMoveRight)
	ON_BN_CLICKED(IDC_CPD_MOVE_LEFT, &CConfigurePatDashboardDlg::OnBnClickedCpdMoveLeft)
	ON_BN_CLICKED(IDC_CPD_MOVE_UP, &CConfigurePatDashboardDlg::OnBnClickedCpdMoveUp)
	ON_BN_CLICKED(IDC_CPD_MOVE_DOWN, &CConfigurePatDashboardDlg::OnBnClickedCpdMoveDown)
	ON_BN_CLICKED(IDC_CPD_REMOVE, &CConfigurePatDashboardDlg::OnBnClickedCpdRemove)
	ON_BN_CLICKED(IDC_CPD_EDIT, &CConfigurePatDashboardDlg::OnBnClickedCpdEdit)
	ON_BN_CLICKED(IDC_CPD_EDIT_SELECTED, &CConfigurePatDashboardDlg::OnBnClickedCpdEditBottom)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigurePatDashboardDlg message handlers

BOOL CConfigurePatDashboardDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnMoveDown.AutoSet(NXB_DOWN);
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveLeft.AutoSet(NXB_UP);
		m_btnMoveRight.AutoSet(NXB_DOWN);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnEditBottom.AutoSet(NXB_MODIFY);
		m_btnRemove.AutoSet(NXB_DELETE);

		// (j.gruber 2012-07-02 12:15) - PLID 51210 - diagram link
		m_diagramlink.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_diagramlink.SetText("Click for column diagram");
		m_diagramlink.SetType(dtsHyperlink);
		

		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_pUserList = BindNxDataList2Ctrl(IDC_CPD_USER_LIST, false);
		m_pAdminList = BindNxDataList2Ctrl(IDC_CPD_ADMIN_LIST, false);	

		// (j.gruber 2012-06-26 16:32) - PLID 51210 - changed the column names
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pUserList->GetColumn(ulcPosition);
		pCol->ComboSource ="SELECT 0 as ID, 'Column 1' as Value "
			" UNION SELECT 1, 'Column 2'  "
			" UNION SELECT 2, 'Columns 1 & 2'  "
			" UNION SELECT 3, 'Column 3' ";

		m_nNextControlID = 0;

		Load();
		
	}NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CConfigurePatDashboardDlg::Validate() 
{

	//run through the rows and make sure each one has a position
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->GetFirstRow();
	while (pRow) {
		long nRegion = VarLong(pRow->GetValue(ulcPosition), -1);
		if (((PDRegion)nRegion) != pdrLeft &&
			((PDRegion)nRegion) != pdrRight &&
			((PDRegion)nRegion) != pdrCentered &&
			((PDRegion)nRegion) != pdrFarRight &&
			((PDRegion)nRegion) != pdrNone) {
			MsgBox("You must enter a valid position for all controls before saving");
			return FALSE;
		}
		pRow = pRow->GetNextRow();
	}
	return TRUE;

}

void CConfigurePatDashboardDlg::OnOK() 
{
	try { 

		if (Validate())
		{
			Save();
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CConfigurePatDashboardDlg, CNxDialog)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_ADMIN_LIST, 3, CConfigurePatDashboardDlg::DblClickCellCpdAdminList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_USER_LIST, 3, CConfigurePatDashboardDlg::DblClickCellCpdUserList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_ADMIN_LIST, 2, CConfigurePatDashboardDlg::OnSelChangedAdminList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_USER_LIST, 2, CConfigurePatDashboardDlg::OnSelChangedUserList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_ADMIN_LIST, 32, CConfigurePatDashboardDlg::ShowContextMenuCpdAdminList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CConfigurePatDashboardDlg, IDC_CPD_USER_LIST, 32, CConfigurePatDashboardDlg::ShowContextMenuCpdUserList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
END_EVENTSINK_MAP()


void CConfigurePatDashboardDlg::ConvertStringToIntArray(CString str, CArray<int, int> &values)
{	
	str.TrimLeft("(");
	str.TrimRight(")");
	long nResult = str.Find(',');
	while(nResult != -1) {
		values.Add(atoi(str.Left(nResult)));

		str = str.Right(str.GetLength() - (nResult + 1));
		str.TrimLeft();
		str.TrimRight();
		nResult = str.Find(',');
	}

	//add our last value
	str.TrimLeft();
	str.TrimRight();
	values.Add(atoi(str));
}

void CConfigurePatDashboardDlg::Load()
{
	try {

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT PatientDashboardControlsT.ID, Title, TypeID, EMRInfoMasterID, "
			" Block, Region, [Order], FilterTypeID, FilterValueInt, FilterValueText, PatientDashboardUserConfigT.ID AS UserConfigID "
			" FROM PatientDashboardControlsT LEFT JOIN "
			" (SELECT * FROM PatientDashboardUserConfigT WHERE UserID = {INT}) PatientDashboardUserConfigT "
			" ON PatientDashboardControlsT.ID = PatientDashboardUserConfigT.ControlID "
			" LEFT JOIN PatientDashboardControlFiltersT ON "
			" PatientDashboardControlsT.ID = PatientDashboardControlFiltersT.ControlID "
			" ORDER BY Block asc, Region asc, [Order] asc, ID asc "
			, GetCurrentUserID());

		long nCurrentID = -999;
		long nOldID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
		PDControl *pControl = NULL; 
		
		if (!rs->eof) {
			pControl = new PDControl();
		}	

		while (!rs->eof) 
		{
			nCurrentID = AdoFldLong(rs->Fields, "ID");

			if (nCurrentID != nOldID) {
				
				if (nOldID != -1) {
					//add the control to our array
					PDControl *temp;
					if (m_controls.Lookup(pControl->nID, temp)) {
						ASSERT(FALSE);
					}
					m_controls.SetAt(pControl->nID, pControl);
					pControl = new PDControl();
				}

				nOldID = nCurrentID;
				
				pControl->nID =  nCurrentID;
				pControl->strTitle = AdoFldString(rs->Fields, "Title");
				pControl->pdtType = (PatientDashboardType) AdoFldLong(rs->Fields, "TypeID");
				pControl->nEMRInfoMasterID = AdoFldLong(rs->Fields, "EMRInfoMasterID", -1);

				long nUserConfigID = AdoFldLong(rs->Fields, "UserConfigID", -1);				
				pControl->nBlock = AdoFldLong(rs->Fields, "Block", -1);
				pControl->nOrder = AdoFldLong(rs->Fields, "Order", -1);
				pControl->pdrRegion = (PDRegion) AdoFldLong(rs->Fields, "Region", -1);
				
			}

			//get the filters
			long nFilterTypeID = AdoFldLong(rs->Fields, "FilterTypeID", -1);
			if (nFilterTypeID != -1) {

				PDFilter *pFilter = new PDFilter();
				pFilter->filterType = (FilterType)nFilterTypeID;
				switch (pFilter->filterType) {
					case LastTypeFormat:
					case timeIncrement:
					case includeDiscontinued:
					case includeResolved:
						pFilter->nValue = AdoFldLong(rs->Fields, "FilterValueInt");
					break;

					case ApptType:
						pFilter->strValue = AdoFldString(rs->Fields, "FilterValueText");
						//we have to get the list of types
						if (pFilter->strValue != "") {
							CArray<int, int> values;
							ConvertStringToIntArray(pFilter->strValue, values);
							ADODB::_RecordsetPtr rsTypes = CreateParamRecordset("SELECT Name FROM AptTypeT WHERE ID IN ({INTARRAY})", values);
							CString strTypes;
							while (!rsTypes->eof) {								
								strTypes += AdoFldString(rsTypes->Fields, "Name", "") + ",";
								rsTypes->MoveNext();
							}
							strTypes.TrimRight(",");
							pFilter->strDisplay = strTypes;
						}

					break;
					// (s.tullis 2015-02-25 09:53) - PLID 64740 
					case DoNotShowOnCCDA:
						pFilter->nValue = AdoFldLong(rs->Fields, "FilterValueInt");
						break;
					// (r.gonet 2015-03-17 10:23) - PLID 65020 - Load the "Exclude problems flagged to 'Do not show on problem prompt'" filter value from the saved dashboard filters.
					case excludeDoNotShowOnProblemPrompt:
						pFilter->nValue = AdoFldLong(rs->Fields, "FilterValueInt", (long)FALSE);
						break;
					case HistoryCategory:
						// (c.haag 2015-04-29) - NX-100441
						pFilter->strValue = AdoFldString(rs->Fields, "FilterValueText");
						//we have to get the list of types
						if (pFilter->strValue != "") {
							CArray<int, int> values;
							ConvertStringToIntArray(pFilter->strValue, values);
							ADODB::_RecordsetPtr rsTypes = CreateParamRecordset("SELECT Description FROM NoteCatsF WHERE IsPatientTab <> 0 AND ID IN ({INTARRAY}) ORDER BY Description", values);
							CString strTypes;
							while (!rsTypes->eof) {
								strTypes += AdoFldString(rsTypes->Fields, "Description", "") + ",";
								rsTypes->MoveNext();
							}
							strTypes.TrimRight(",");
							pFilter->strDisplay = strTypes;
						}
						break;
				}
				pControl->aryFilters.Add(pFilter);
			}

			rs->MoveNext();
		}

		if (pControl) {
			m_controls.SetAt(pControl->nID, pControl);
		}

		//fill our lists based on the controls
		POSITION pos = m_controls.GetStartPosition();
		long nID;
		pControl = NULL;		

		while(pos) {
			m_controls.GetNextAssoc(pos, nID, pControl);

			if (pControl->nBlock != -1)
			{
				//this is a user row			
				AddUserRow(pControl, FALSE, TRUE);
			}
			else {
				AddAdminRow(pControl);
			}
		}

		HandleSelChanged();

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck)
{
	long nVal = VarLong(pRowToAdd->GetValue(nColumnToCheck));
	long nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
	while (pCheckRow && nCheckVal < nVal) {				
		pCheckRow = pCheckRow->GetNextRow();
		if (pCheckRow) {
			nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
		}
	}
}

//this one makes sure we stay equal to a val and compares
void CConfigurePatDashboardDlg::GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck, short nColumnToEqual, long nValToEqual)
{
	long nVal = VarLong(pRowToAdd->GetValue(nColumnToCheck));
	long nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
	long nCheckEqual = VarLong(pCheckRow->GetValue(nColumnToEqual));
	while (pCheckRow && nCheckVal < nVal && nCheckEqual == nValToEqual) {
		pCheckRow = pCheckRow->GetNextRow();
		if (pCheckRow) {
			nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
			nCheckEqual = VarLong(pCheckRow->GetValue(nColumnToEqual));
		}
	}
}

//this one makes sure we stay equal to 2 vals and compares
void CConfigurePatDashboardDlg::GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck, short nColumnToEqual, long nValToEqual, short nColumnToEqual2, long nValToEqual2)
{
	long nVal = VarLong(pRowToAdd->GetValue(nColumnToCheck));
	long nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
	long nCheckEqual = VarLong(pCheckRow->GetValue(nColumnToEqual));
	long nCheckEqual2 = VarLong(pCheckRow->GetValue(nColumnToEqual2));
	while (pCheckRow && nCheckVal < nVal && nCheckEqual == nValToEqual && nCheckEqual2 == nValToEqual2) {
		pCheckRow = pCheckRow->GetNextRow();
		if (pCheckRow) {
			nCheckVal = VarLong(pCheckRow->GetValue(nColumnToCheck));
			nCheckEqual = VarLong(pCheckRow->GetValue(nColumnToEqual));
			nCheckEqual2 = VarLong(pCheckRow->GetValue(nColumnToEqual2));
		}
	}
}

void CConfigurePatDashboardDlg::AddRowSorted(NXDATALIST2Lib::IRowSettingsPtr pRow) 
{
	CString strTemp = VarString(pRow->GetValue(ulcTitle));
	//are there any rows in our list yet?
	if (m_pUserList->GetRowCount() == 0) {
		//add our row
		m_pUserList->AddRowAtEnd(pRow, NULL);
	}
	else {
		//which region are we in?
		PDRegion region = (PDRegion)VarLong(pRow->GetValue(ulcPosition));		
		if (region == pdrFarRight) {
			//we are on the far right, so we are going to the end
			NXDATALIST2Lib::IRowSettingsPtr pRowTemp = m_pUserList->FindByColumn(ulcPosition, pdrFarRight, NULL, FALSE);
			if (pRowTemp) {
				GetRowPosition(pRowTemp, pRow, ulcOrder);
				//if we are here, we either are out of rows or we have where we need to insert
				if (pRowTemp) {
					//add before pTempRow
					m_pUserList->AddRowBefore(pRow, pRowTemp);					
				}
				else {
					//we hit the end, add to the end
					m_pUserList->AddRowAtEnd(pRow, NULL);					
				}
			}
			else {
				//we don't have a Far right region, so add it at the end
				m_pUserList->AddRowAtEnd(pRow, NULL);
			}
		}
		else {
			//take block into account first
			long nBlock = VarLong(pRow->GetValue(ulcBlock));
			NXDATALIST2Lib::IRowSettingsPtr pRowTemp = m_pUserList->FindByColumn(ulcBlock, nBlock, NULL, FALSE);
			if (pRowTemp) {
				GetRowPosition(pRowTemp, pRow, ulcBlock);
				if (pRowTemp) {
					//we have a row, so check its block
					long nBlockTemp = VarLong(pRowTemp->GetValue(ulcBlock));
					if (nBlockTemp > nBlock) {
						//we can add it here
						m_pUserList->AddRowBefore(pRow, pRowTemp);
					}
					else {
						//they are equal, so we need to check region, then order, making sure we stay in the same block
						GetRowPosition(pRowTemp, pRow, ulcPosition, ulcBlock, nBlock);
						if (pRowTemp) {
							//are we in the same block?
							nBlockTemp = VarLong(pRowTemp->GetValue(ulcBlock));
							if (nBlockTemp != nBlock) {
								//we have reached the end of the block, so add it
								m_pUserList->AddRowBefore(pRow, pRowTemp);
							}
							else {
								//we are still in the block, so check the region
								long nRegionTemp = VarLong(pRowTemp->GetValue(ulcPosition));
								if (nRegionTemp > region) {
									//we can add it here
									m_pUserList->AddRowBefore(pRow, pRowTemp);
								}
								else {
									//the regions are equal, so we need to check the order, keeping the block and region the same
									GetRowPosition(pRowTemp, pRow, ulcOrder, ulcBlock, nBlock, ulcPosition, region);
									if (pRowTemp) {
										//we are adding it here
										m_pUserList->AddRowBefore(pRow, pRowTemp);
									}
									else {
										//add it at the end
										m_pUserList->AddRowAtEnd(pRow, NULL);
									}
								}
							}
						}
						else {

							//we hit the end
							m_pUserList->AddRowAtEnd(pRow, NULL);
						}
					}
				}
				else {
					m_pUserList->AddRowAtEnd(pRow, NULL);
				}
			}
			else {
				//we don't have this block, so get the first row and see what block it has
				//first make sure of the region
				NXDATALIST2Lib::IRowSettingsPtr pRowTemp = m_pUserList->GetFirstRow();
				if (pRowTemp) {
					long nRegionTemp = VarLong(pRowTemp->GetValue(ulcPosition));
					if (nRegionTemp == pdrFarRight) {
						//we already know, we aren't there, so add it before
						m_pUserList->AddRowBefore(pRow, pRowTemp);
					}
					else {
						//check the block
						long nBlockTemp = VarLong(pRowTemp->GetValue(ulcBlock));
						if (nBlockTemp > nBlock) {
							//add it before
							m_pUserList->AddRowBefore(pRow, pRowTemp);
						}
						else {
							//its less than the block we have, so run until the end of the block
							GetRowPosition(pRowTemp, pRow, ulcBlock);
							if (pRowTemp) {
								//add here since we know we don't have the same block, this must be above ours
								m_pUserList->AddRowBefore(pRow, pRowTemp);
							}
							else {
								//we hit the end
								m_pUserList->AddRowAtEnd(pRow, NULL);
							}
						}
					}
				}
			}			
		}
	}

	HandleSelChanged();
}

void CConfigurePatDashboardDlg::AddUserRow(PDControl *pControl, BOOL bStartEditing /*=FALSE*/, BOOL bAddRowSorted /*=FALSE*/)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->GetNewRow();

	if (pRow) {
		UpdateUserRowWithControl(pRow, pControl);

		if (bAddRowSorted) {
			AddRowSorted(pRow);
		}
		else {
			m_pUserList->AddRowAtEnd(pRow, NULL);
		}		

		if (bStartEditing) {
			m_pUserList->CurSel = pRow;
			HandleSelChanged();
			m_pUserList->StartEditing(pRow, ulcPosition);
		}

		HandleSelChanged();
	}
}

void CConfigurePatDashboardDlg::UpdateUserRowWithControl(NXDATALIST2Lib::IRowSettingsPtr pRow, PDControl *pControl)
{
	pRow->PutValue(ulcID, pControl->nID);
	pRow->PutValue(ulcTitle, _variant_t(pControl->strTitle));
	pRow->PutValue(ulcType, _variant_t(GetTypeNameFromID(pControl)));
	if (pControl->pdrRegion != pdrFarRight) {
		pRow->PutValue(ulcBlock, pControl->nBlock);
	}
	else {
		//load 99999 becasue we want it to be at the end			
		pRow->PutValue(ulcBlock, 99999);
	}

	pRow->PutValue(ulcPosition, pControl->pdrRegion);
	pRow->PutValue(ulcOrder, pControl->nOrder);
	pRow->PutValue(ulcFilters, _variant_t(FormatFilters(pControl)));
}

void CConfigurePatDashboardDlg::AddAdminRow(PDControl *pControl, BOOL bSetCurSel /*= FALSE*/)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAdminList->GetNewRow();

	if (pRow) {
		UpdateAdminRowWithControl(pRow, pControl);

		pRow = m_pAdminList->AddRowSorted(pRow, NULL);

		if (bSetCurSel) {
			m_pAdminList->CurSel = pRow;
			m_pAdminList->EnsureRowInView(pRow);
			HandleSelChanged();
		}

		HandleSelChanged();
	}
}

void CConfigurePatDashboardDlg::UpdateAdminRowWithControl(NXDATALIST2Lib::IRowSettingsPtr pRow, PDControl *pControl)
{
	pRow->PutValue(alcID, pControl->nID);
	pRow->PutValue(alcTitle, _variant_t(pControl->strTitle));
	pRow->PutValue(alcType, _variant_t(GetTypeNameFromID(pControl)));
	pRow->PutValue(alcFilters, _variant_t(FormatFilters(pControl)));
}
	
void CConfigurePatDashboardDlg::Save()
{
	try {
		
		CSqlFragment sql(" SET NOCOUNT ON; \r\n "
			" SET XACT_ABORT ON \r\n"
			" BEGIN TRAN \r\n" 
			" DECLARE @DeletedControlID INT; "
			" DELETE FROM PatientDashboardUserConfigT WHERE UserID = {INT} ", GetCurrentUserID()
			);

		//first let's loop through our deleted and remove them
		
		POSITION pos = m_deletedControls.GetStartPosition();
		long nID;
		PDControl *pDelControl = NULL;		

		while(pos) {
			m_deletedControls.GetNextAssoc(pos, nID, pDelControl);			
		
			sql +=  CSqlFragment(" SET @DeletedControlID = {INT}; \r\n"
				" DELETE FROM PatientDashboardUserConfigT WHERE ControlID = @DeletedControlID ; "
				" DELETE FROM PatientDashboardControlFiltersT WHERE ControlID = @DeletedControlID ; "
				" DELETE FROM PatientDashboardControlsT WHERE ID = @DeletedControlID ; ", pDelControl->nID);
		}
						

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->GetFirstRow();
		long nBlock = 1;
		long nLeftOrder, nRightOrder;
		nLeftOrder = nRightOrder = 0;
		long nFarRightOrder = 0;
		long nCurrentOrder = -1;
		BOOL bFirstRow = TRUE;
		BOOL bNeedsNewBlock = FALSE;
		while (pRow) {

			PDRegion region = (PDRegion)VarLong(pRow->GetValue(ulcPosition));
			ASSERT((long)region >= 0 && (long)region <= 4);
			if (region == pdrCentered && !bFirstRow) {
				bNeedsNewBlock = TRUE;
			}

			//far right has no blocks
			if (region != pdrFarRight) {
				if (bNeedsNewBlock) {
					nBlock++;
					nLeftOrder = 0;
					nRightOrder = 0;
					bNeedsNewBlock = FALSE;						
				}				
				if (region == pdrLeft) {
					nLeftOrder++;
					nCurrentOrder = nLeftOrder;
				}
				else if (region == pdrRight) {
					nRightOrder++;
					nCurrentOrder = nRightOrder;
				}
				else {
					//we can get here if centered is the first position
					nCurrentOrder = 1;
				}
				
			}
			else{
				nFarRightOrder++;
				nCurrentOrder = nFarRightOrder;
			}


			sql += CSqlFragment( "INSERT INTO PatientDashboardUserConfigT ( "
				" UserID, ControlID, Block, Region, [Order]) VALUES "
				" ({INT}, {INT}, {INT}, {INT}, {INT}) ",
				GetCurrentUserID(), 
				VarLong(pRow->GetValue(ulcID)),
				nBlock, region, nCurrentOrder);

			bFirstRow = FALSE;

			if (region == pdrCentered) {
				bNeedsNewBlock = TRUE;
			}

			pRow = pRow->GetNextRow();
		}

		sql += "COMMIT TRAN";
		ExecuteParamSql(sql);
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::InitializeNewControl(PDControl *pControl) {

	m_nNextControlID--;

	pControl->nID = m_nNextControlID;
	pControl->nBlock = -1;
	pControl->nOrder = -1;
}


void CConfigurePatDashboardDlg::OnBnClickedCpdAdd()
{
	try {
		PDControl *pControl = new PDControl();
		InitializeNewControl(pControl);
		CAddPatDashboardControlDlg dlg(pControl, TRUE);
		if (IDOK == dlg.DoModal()) {

			//add our new control
			m_controls.SetAt(pControl->nID, pControl);

			//add it to the admin side
			AddAdminRow(pControl, TRUE);
		}
		else {
			//get rid of the memory
			delete pControl;
			pControl = NULL;
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CConfigurePatDashboardDlg::MoveLeftAfterWarning(PDControl *pControl)
{
	//we really only care about EMNTemplate Items and EMRSummary Items
	PatientDashboardType pdType  = pControl->pdtType;
	CString strName;
	BOOL bCheck = FALSE;
	if (pdType == pdtCreateEMNs) {
		if (IDYES == MsgBox(MB_YESNO, "If you move the EMN Templates control out of the user list, you will not be able to create EMNs from the Patient Dashboard.\n\nWould you like to keep this in the user list?"))
		{
			return FALSE;
		}
	}
	else if (pdType == pdtEMNSummary) {
		
		if (IDYES == MsgBox(MB_YESNO, "If you move the Review Past EMNs control out of the user list, you will not be able to review or open past EMNs from the Patient Dashboard.\n\nWould you like to keep this in the user list?"))
		{
			return FALSE;
		}
	}	

	//if we got here, we are OK
	return TRUE;
}

BOOL CConfigurePatDashboardDlg::CanMoveRight(PDControl *pControl) {

	//we really only care about EMNTemplate Items and EMRSummary Items
	PatientDashboardType pdType = pControl->pdtType;
	CString strName;
	BOOL bCheck = FALSE;
	if (pdType == pdtCreateEMNs) {
		strName = "EMN Templates";
		bCheck = TRUE;
	}
	else if (pdType == pdtEMNSummary) {
		strName = "Review Past EMNs";
		bCheck = TRUE;
	}

	
	if (bCheck) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->GetFirstRow();
		while (pRow) {
			long nID = VarLong(pRow->GetValue(ulcID));
			PDControl *pControl;
			if (m_controls.Lookup(nID, pControl)) {
				if (pControl->pdtType == pdType) {
					MsgBox("There is already a %s control selected, you may only have one of this type selected.", strName);
					return FALSE;
				}
			}
			pRow = pRow->GetNextRow();
		}
	}

	//if we got here, we are OK
	return TRUE;

}

void CConfigurePatDashboardDlg::OnBnClickedCpdMoveRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAdminList->CurSel;
		if (pRow) {
			long nID = VarLong(pRow->GetValue(alcID));
			PDControl *pControl = NULL;
			if (m_controls.Lookup(nID, pControl)) {		
				if (CanMoveRight(pControl)) {
					m_pAdminList->RemoveRow(pRow);
					AddUserRow(pControl, TRUE);					
					HandleSelChanged();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::OnBnClickedCpdMoveLeft()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		if (pRow) {
			long nID = VarLong(pRow->GetValue(ulcID));
			PDControl *pControl = NULL;
			if (m_controls.Lookup(nID, pControl)) {
				if (MoveLeftAfterWarning(pControl)) {
					m_pUserList->RemoveRow(pRow);
					AddAdminRow(pControl, TRUE);
					HandleSelChanged();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::OnBnClickedCpdMoveUp()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowAbove = pRow->GetPreviousRow();
			if (pRowAbove) {
				//switch them
				m_pUserList->RemoveRow(pRow);
				m_pUserList->AddRowBefore(pRow, pRowAbove);
				m_pUserList->CurSel = pRow;
				HandleSelChanged();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::OnBnClickedCpdMoveDown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowBelow = pRow->GetNextRow();
			if (pRowBelow) {
				//have to get the next one
				NXDATALIST2Lib::IRowSettingsPtr pRow2Below = pRowBelow->GetNextRow();
				if (pRow2Below) {
					//switch them
					m_pUserList->RemoveRow(pRow);
					m_pUserList->AddRowBefore(pRow, pRow2Below);		
					m_pUserList->CurSel = pRow;
				}
				else {
					//must be at the end
					m_pUserList->RemoveRow(pRow);
					m_pUserList->AddRowAtEnd(pRow, NULL);	
					m_pUserList->CurSel = pRow;
				}
				HandleSelChanged();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

CString CConfigurePatDashboardDlg::GetTypeNameFromID(PDControl *pControl)
{
	return GetDashboardTypeNameFromID(pControl->pdtType);
}

// (z.manning 2015-05-07 11:23) - NX-100439 - Moved this logic to a global function
CString GetDashboardTypeNameFromID(const PatientDashboardType type)
{
	switch(type)
	{
		case pdtAllergies:
			return "Allergies";
		break;
		case pdtCurrentMeds:
			return "Current Medications";
		break;
		case pdtPrescriptions:
			return "Prescriptions";
		break;
		case pdtHistoryImages:
			// (z.manning 2015-05-07 11:13) - NX-100439 - Renamed 'images' to 'photos'
			return "History Photos";
		break;
		case pdtEMNSummary:
			return "Review Past EMNs";
		break;
		case pdtProcedures:
			return "EMR Procedures";
		break;
		case pdtDiagCodes:
			return "Diagnosis Codes";
		break;
		case pdtCreateEMNs:
			return "EMN Templates";
		break;
		case pdtProblems:
			return "EMR Problems";
		break;
		case pdtLabs:
			return "Labs";
		break;
		case pdtEMNItems:
			return "EMN Item History";
		break;
		case pdtAppointments:
			return "Appointments";
		break;
		case pdtBills:
			return "Bills";
		break;
		// (c.haag 2015-04-29) - NX-100441
		case pdtHistoryAttachments:
			return "History Attachments";
	}
	return "";

}

CString CConfigurePatDashboardDlg::FormatFilters(PDControl *pControl)
{
	CString strFilters = "";

	if (pControl->aryFilters.GetSize() > 0) {

		CString strTimeInterval = "";
		CString strTimeIncrement = "";

		for (int i = 0; i < pControl->aryFilters.GetSize(); i++) {

			PDFilter *pFilter = pControl->aryFilters[i];
			switch(pFilter->filterType) {
				case LastTypeFormat:
					switch ((LastTimeFormat)pFilter->nValue) {
					case ltfAll:
						strTimeInterval = "All";
						break;
					case ltfDay:
						strTimeInterval = "Day(s)";
						break;
					case ltfMonth:
						strTimeInterval = "Month(s)";
						break;
					case ltfYear:
						strTimeInterval = "Year(s)";
						break;
					case lftRecord: // (z.manning 2015-04-15 11:20) - NX-100388
						strTimeInterval = "Record(s)";
						break;
					}
				break;
				case timeIncrement:
					strTimeIncrement = AsString(pFilter->nValue);
				break;
				case ApptType:
					{
						strFilters += "Include ApptTypes: " + pFilter->strDisplay + "; ";
					}
				break;
				case includeDiscontinued:
					if (pFilter->nValue == 0) {
						strFilters += "Not Including Discontinued ";
					}
					else {
						strFilters += "Including Discontinued; ";
					}
				break;
				case includeResolved:
					if (pFilter->nValue == 0) {
						strFilters += "Not Including Resolved; ";
					}
					else {
						strFilters += "Including Resolved; ";
					}					
				break;
				// (s.tullis 2015-03-04 11:48) - PLID 64740 - 
				case DoNotShowOnCCDA:
					if (pFilter->nValue == 0){
						strFilters += "Include problems flagged to 'Do not show on CCDA'; ";
					}
					else{
						strFilters += "Exclude problems flagged to 'Do not show on CCDA'; ";
					}
					break;
				// (r.gonet 2015-03-17 10:23) - PLID 65020 - Describe the filter's value tied to the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox.
				case excludeDoNotShowOnProblemPrompt:
					if ((BOOL)pFilter->nValue == FALSE) {
						// (r.gonet 2015-03-17 10:23) - PLID 65020 - This branch will never be executed under normal circumstances,
						// since there will just be no filter object.
						// (r.gonet 2015-03-19) - PLID 65020 - I don't think there should be an Include version here because by default, these kinds of problems are included, and there is no verbiage
						// about including problems flagged to ... by default. I'm going to follow the lead of every other filter type though.
						strFilters += "Include problems flagged to 'Do not show on problem prompt'; ";
					} else {
						strFilters += "Exclude problems flagged to 'Do not show on problem prompt'; ";
					}
				case HistoryCategory:
					// (c.haag 2015-04-29) - NX-100441
					strFilters += "Include Categories: " + pFilter->strDisplay + "; ";
					break;
			}
		}
		if (!strTimeInterval.IsEmpty()) {
			if (strTimeInterval != "All") {
				strFilters = "Showing last " + strTimeIncrement + " " + strTimeInterval + "; " + strFilters;
			}
			else {
				strFilters = "Showing All; " + strFilters;
			}
		}
	}
	return strFilters;
}
void CConfigurePatDashboardDlg::OnBnClickedCpdRemove()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAdminList->CurSel;
		if (pRow) {
			long nID = VarLong(pRow->GetValue(alcID));		

			PDControl *pControl = NULL;
			if (m_controls.Lookup(nID, pControl)) {						
					
				//see if this item exists on any other user's board
				ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT UserName FROM "
					" PatientDashboardUserConfigT INNER JOIN UsersT ON PatientDashboardUserConfigT.UserID = UsersT.PersonID "
					" WHERE PatientDashboardUserConfigT.ControlID = {INT} AND UsersT.PersonID <> {INT} ",nID, GetCurrentUserID());
				if (rs->eof) {

					if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to remove this control?")) {
						m_controls.RemoveKey(nID);
						m_deletedControls.SetAt(nID, pControl);
					
						//there aren't any other users, we can remove it
						m_pAdminList->RemoveRow(pRow);

						HandleSelChanged();
					}
				}
				else {
					CString strUserNames = "";
					while (!rs->eof) {
						strUserNames += AdoFldString(rs->Fields, "UserName") + "\r\n";
						rs->MoveNext();
					}

					CString strMessage;
					strMessage.Format("The following users have this control showing on their dashboard:\n%sYou cannot remove this control until it is removed from all user's dashboards", strUserNames);
					MessageBox(strMessage);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-28 15:26) - NX-100396 - Moved edit logic here
void CConfigurePatDashboardDlg::EditControl(NXDATALIST2Lib::IRowSettingsPtr pRow, const BOOL bAdminRow)
{
	if (pRow == NULL) {
		return;
	}

	//do we have this control
	PDControl *pExistingControl = NULL;
	long nControlID = VarLong(pRow->GetValue(bAdminRow ? alcID : ulcID));
	if (m_controls.Lookup(nControlID, pExistingControl))
	{
		//see if any other user has this item configured on their dashboard
		ADODB::_RecordsetPtr rs = CreateParamRecordset(R"(
SELECT UserName
FROM PatientDashboardUserConfigT
INNER JOIN UsersT ON PatientDashboardUserConfigT.UserID = UsersT.PersonID
WHERE PatientDashboardUserConfigT.ControlID = {INT} AND UsersT.PersonID <> {INT}
)"
, nControlID, GetCurrentUserID());
		if (!rs->eof) {
			// (z.manning 2015-04-28 16:59) - NX-100397 - I changed this logic quite a bit. We used to not let them edit in-use controls
			// and would make them copy it. Now we let them though we still prompt.
			int nResult = MessageBox("At least one other user has this control selected on their dashboard. Continuing will edit this control for all dashboards using it. Would you like to continue?"
				, NULL, MB_YESNO | MB_ICONQUESTION);
			if (nResult != IDYES) {
				return;
			}
		}

		//remove it from our map
		m_controls.RemoveKey(nControlID);

		//let them edit it
		CAddPatDashboardControlDlg dlg(pExistingControl, TRUE);
		dlg.DoModal();

		//no matter whether they saved or cancelled, we have to add it back to our map
		m_controls.SetAt(pExistingControl->nID, pExistingControl);

		//update the row
		// (z.manning 2015-04-28 15:58) - NX-100396 - Now possible to edit user rows as well
		if (bAdminRow) {
			UpdateAdminRowWithControl(pRow, pExistingControl);
			m_pAdminList->CurSel = pRow;
		}
		else {
			UpdateUserRowWithControl(pRow, pExistingControl);
			m_pUserList->CurSel = pRow;
		}
		HandleSelChanged();
	}
	else {
		ThrowNxException("Could not find dashboard control %li", nControlID);
	}
}

void CConfigurePatDashboardDlg::OnBnClickedCpdEdit()
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAdminList->CurSel;
		EditControl(pRow, TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-28 15:42) - NX-100396
void CConfigurePatDashboardDlg::OnBnClickedCpdEditBottom()
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		EditControl(pRow, FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::DblClickCellCpdAdminList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(alcID));
			PDControl *pControl = NULL;
			if (m_controls.Lookup(nID, pControl)) {		
				if (CanMoveRight(pControl)) {
					m_pAdminList->RemoveRow(pRow);
					AddUserRow(pControl, TRUE);					
					HandleSelChanged();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePatDashboardDlg::DblClickCellCpdUserList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nID = VarLong(pRow->GetValue(ulcID));
			PDControl *pControl = NULL;
			if (m_controls.Lookup(nID, pControl)) {
				if (MoveLeftAfterWarning(pControl)) {
					m_pUserList->RemoveRow(pRow);
					AddAdminRow(pControl, TRUE);
					HandleSelChanged();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-07-02 12:15) - PLID 51210 - diagram link
LRESULT CConfigurePatDashboardDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		//(e.lally 2008-09-05) PLID 6780 - Add option for locations
		switch(nIdc) {
		case IDC_COLUMN_DIAGRAM:
			CPatDashDiagramDlg dlg(this);
			dlg.DoModal();
			break;

		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.gruber 2012-07-02 12:15) - PLID 51210 - diagram link
BOOL CConfigurePatDashboardDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);


		GetDlgItem(IDC_COLUMN_DIAGRAM)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (z.manning 2015-04-28 13:37) - NX-100396
void CConfigurePatDashboardDlg::OnSelChangedAdminList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		HandleSelChanged();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-28 13:37) - NX-100396
void CConfigurePatDashboardDlg::OnSelChangedUserList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		HandleSelChanged();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-28 13:47) - NX-100396
void CConfigurePatDashboardDlg::HandleSelChanged()
{
	NXDATALIST2Lib::IRowSettingsPtr pAdminRow = m_pAdminList->CurSel;
	BOOL bAdminEnabled = (pAdminRow != NULL);
	NXDATALIST2Lib::IRowSettingsPtr pUserRow = m_pUserList->CurSel;
	BOOL bUserEnabled = (pUserRow != NULL);

	GetDlgItem(IDC_CPD_REMOVE)->EnableWindow(bAdminEnabled);
	GetDlgItem(IDC_CPD_EDIT)->EnableWindow(bAdminEnabled);

	GetDlgItem(IDC_CPD_EDIT_SELECTED)->EnableWindow(bUserEnabled);
}

// (z.manning 2015-05-04 15:23) - NX-100401
void CConfigurePatDashboardDlg::ShowContextMenuCpdAdminList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue)
{
	try
	{
		DoContextMenu(lpRow, TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-05-04 15:23) - NX-100401
void CConfigurePatDashboardDlg::ShowContextMenuCpdUserList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue)
{
	try
	{
		DoContextMenu(lpRow, FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-05-04 15:27) - NX-100401
void CConfigurePatDashboardDlg::DoContextMenu(NXDATALIST2Lib::IRowSettingsPtr pRow, const BOOL bAdminRow)
{
	if (pRow == NULL) {
		return;
	}

	if (bAdminRow) {
		m_pAdminList->CurSel = pRow;
	}
	else {
		m_pUserList->CurSel = pRow;
	}
	HandleSelChanged();

	enum MenuOptions {
		moEdit = 1,
		moDelete,
	};

	CMenu mnu;
	mnu.CreatePopupMenu();
	mnu.AppendMenu(MF_ENABLED, moEdit, "&Edit");
	if (bAdminRow) {
		mnu.AppendMenu(MF_ENABLED, moDelete, "&Delete");
	}

	CPoint pt;
	GetCursorPos(&pt);

	switch (mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN, pt.x, pt.y, this, NULL))
	{
	case moEdit:
		EditControl(pRow, bAdminRow);
		break;

	case moDelete:
		OnBnClickedCpdRemove();
		break;
	}
}
