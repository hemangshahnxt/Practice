// ConfigureReportSegmentsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "ConfigureReportSegmentsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2008-07-22 10:30) - PLID 30695 - created
/////////////////////////////////////////////////////////////////////////////
// CConfigureReportSegmentsDlg dialog

enum SegmentColumnList {
	slcID = 0,
	slcType = 1,
	slcName = 2,
};

enum AvailColumnList {
	alcID = 0,
	alcName = 1,
};

enum SelectedColumnList {
	selID = 0,
	selName = 1,
};

/*This is in data- cannot be changed!!*/
enum SegmentTypes {
	stCategories = 1,
	stProviders = 2,
	stLocations = 3,
	stApptTypes = 4,
};

CConfigureReportSegmentsDlg::CConfigureReportSegmentsDlg(CWnd* pParent)
	: CNxDialog(CConfigureReportSegmentsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureReportSegmentsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigureReportSegmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureReportSegmentsDlg)
	DDX_Control(pDX, IDC_SEGMENT_TYPE_STATIC, m_stSegmentType);
	DDX_Control(pDX, IDC_SEGMENT_CATEGORIES, m_rdSegmentCategories);
	DDX_Control(pDX, IDC_SEGMENT_PROVIDERS, m_rdSegmentProviders);
	DDX_Control(pDX, IDC_SEGMENT_LOCATIONS, m_rdSegmentLocations);
	DDX_Control(pDX, IDC_SEGMENT_APPT_TYPES, m_rdSegmentApptTypes);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_SEGMENT_RENAME, m_btnSegmentRename);
	DDX_Control(pDX, IDC_SEGMENT_ONE_RIGHT, m_btnSegmentOneRight);
	DDX_Control(pDX, IDC_SEGMENT_ONE_LEFT, m_btnSegmentOneLeft);
	DDX_Control(pDX, IDC_SEGMENT_DELETE, m_btnSegmentDelete);
	DDX_Control(pDX, IDC_SEGMENT_ALL_LEFT, m_btnSegmentAllLeft);
	DDX_Control(pDX, IDC_SEGMENT_ADD, m_btnSegmentAdd);
	DDX_Control(pDX, IDC_SEGEMENT_ALL_RIGHT, m_btnSegmentAllRight);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureReportSegmentsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureReportSegmentsDlg)
	ON_BN_CLICKED(IDC_SEGMENT_CATEGORIES, OnSegmentCategories)
	ON_BN_CLICKED(IDC_SEGMENT_APPT_TYPES, OnSegmentApptTypes)
	ON_BN_CLICKED(IDC_SEGMENT_LOCATIONS, OnSegmentLocations)
	ON_BN_CLICKED(IDC_SEGMENT_PROVIDERS, OnSegmentProviders)
	ON_BN_CLICKED(IDC_SEGEMENT_ALL_RIGHT, OnSegementAllRight)
	ON_BN_CLICKED(IDC_SEGMENT_ALL_LEFT, OnSegmentAllLeft)
	ON_BN_CLICKED(IDC_SEGMENT_ONE_LEFT, OnSegmentOneLeft)
	ON_BN_CLICKED(IDC_SEGMENT_ONE_RIGHT, OnSegmentOneRight)
	ON_BN_CLICKED(IDC_SEGMENT_ADD, OnSegmentAdd)
	ON_BN_CLICKED(IDC_SEGMENT_DELETE, OnSegmentDelete)
	ON_BN_CLICKED(IDC_SEGMENT_RENAME, OnSegmentRename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureReportSegmentsDlg message handlers

void CConfigureReportSegmentsDlg::OnSegmentCategories() 
{
	try {

		if (m_rdSegmentCategories.GetCheck()) {

			//categories is checked
			m_rdSegmentCategories.SetCheck(1);
			m_rdSegmentProviders.SetCheck(0);
			m_rdSegmentLocations.SetCheck(0);
			m_rdSegmentApptTypes.SetCheck(0);

			CString strWhere;
			strWhere.Format("ReportSegmentsT.TypeID = %li", stCategories);
			m_pSegmentList->WhereClause =  _bstr_t(strWhere);
			m_pSegmentList->Requery();
			m_pSegmentList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pAvailList->FromClause = "CategoriesT";
			m_pAvailList->WhereClause = "";
			
			m_pSelectedList->FromClause = "ReportSegmentDetailsT LEFT JOIN CategoriesT ON ReportSegmentDetailsT.ItemID = CategoriesT.ID";

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pSegmentList->GetFirstRow();
			if (pRow) {
				m_pSegmentList->CurSel = pRow;
				OnSelChosenSegmentGroupList(pRow);
			}		
			else {
				m_pAvailList->Clear();
				m_pSelectedList->Clear();
			}
			
		}

		
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentCategories() ");
	
}

void CConfigureReportSegmentsDlg::OnSegmentApptTypes() 
{
	try {

		if (m_rdSegmentApptTypes.GetCheck()) {

			//categories is checked
			m_rdSegmentApptTypes.SetCheck(1);
			m_rdSegmentProviders.SetCheck(0);
			m_rdSegmentLocations.SetCheck(0);
			m_rdSegmentCategories.SetCheck(0);

			CString strWhere;
			strWhere.Format("ReportSegmentsT.TypeID = %li", stApptTypes);
			m_pSegmentList->WhereClause =  _bstr_t(strWhere);
			m_pSegmentList->Requery();
			m_pSegmentList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pAvailList->FromClause = "AptTypeT";
			m_pAvailList->WhereClause = "";
			//leaving inactive's in there since its for reports
			
			m_pSelectedList->FromClause = "ReportSegmentDetailsT LEFT JOIN AptTypeT ON ReportSegmentDetailsT.ItemID = AptTypeT.ID";

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pSegmentList->GetFirstRow();
			if (pRow) {
				m_pSegmentList->CurSel = pRow;
				OnSelChosenSegmentGroupList(pRow);
			}		
			else {
				m_pAvailList->Clear();
				m_pSelectedList->Clear();
			}
			
		}

		
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentApptTypes()  ");
	
}

void CConfigureReportSegmentsDlg::OnSegmentLocations() 
{
	try {

		if (m_rdSegmentLocations.GetCheck()) {

			//categories is checked
			m_rdSegmentLocations.SetCheck(1);
			m_rdSegmentProviders.SetCheck(0);
			m_rdSegmentApptTypes.SetCheck(0);
			m_rdSegmentCategories.SetCheck(0);

			CString strWhere;
			strWhere.Format("ReportSegmentsT.TypeID = %li", stLocations);
			m_pSegmentList->WhereClause =  _bstr_t(strWhere);
			m_pSegmentList->Requery();
			m_pSegmentList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pAvailList->FromClause = "LocationsT";
			m_pAvailList->WhereClause = "Managed = 1";
			//leaving inactive's in there since its for reports
			

			m_pSelectedList->FromClause = "ReportSegmentDetailsT LEFT JOIN LocationsT ON ReportSegmentDetailsT.ItemID = LocationsT.ID";

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pSegmentList->GetFirstRow();
			if (pRow) {
				m_pSegmentList->CurSel = pRow;
				OnSelChosenSegmentGroupList(pRow);
			}
			else {
				m_pAvailList->Clear();
				m_pSelectedList->Clear();
			}
			
		}

		
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentLocations()  ");
	
	
}

void CConfigureReportSegmentsDlg::OnSegmentProviders() 
{
	try {

		if (m_rdSegmentProviders.GetCheck()) {

			//categories is checked
			m_rdSegmentProviders.SetCheck(1);
			m_rdSegmentLocations.SetCheck(0);
			m_rdSegmentApptTypes.SetCheck(0);
			m_rdSegmentCategories.SetCheck(0);

			CString strWhere;
			strWhere.Format("ReportSegmentsT.TypeID = %li", stProviders);
			m_pSegmentList->WhereClause =  _bstr_t(strWhere);
			m_pSegmentList->Requery();
			m_pSegmentList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pAvailList->FromClause = "(SELECT PersonT.ID as ID, First + ' ' + Last as Name FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID)Q";
			m_pAvailList->WhereClause = "";
			//leaving inactive's in there since its for reports
			

			m_pSelectedList->FromClause = "ReportSegmentDetailsT LEFT JOIN (SELECT PersonT.ID as ID, First + ' ' + Last as Name FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID)Q ON ReportSegmentDetailsT.ItemID = Q.ID";

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pSegmentList->GetFirstRow();
			if (pRow) {
				m_pSegmentList->CurSel = pRow;
				OnSelChosenSegmentGroupList(pRow);
			}
			else {
				m_pAvailList->Clear();
				m_pSelectedList->Clear();
			}
			
		}

		
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentProviders()  ");
	
}

void CConfigureReportSegmentsDlg::OnSegementAllRight() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//first get the segmentID
		pRow = m_pSegmentList->CurSel;
		long nSegmentID;
		if (pRow) {
			nSegmentID = VarLong(pRow->GetValue(slcID));

			pRow = m_pAvailList->GetFirstRow();
			CString strSql = BeginSqlBatch();
			BOOL bRunQuery = FALSE;
			while (pRow) {

				long nID = VarLong(pRow->GetValue(alcID));

				AddStatementToSqlBatch(strSql, "INSERT INTO ReportSegmentDetailsT (SegmentID, ItemID) VALUES (%li, %li)", nSegmentID, nID);

				pRow = pRow->GetNextRow();
				bRunQuery = TRUE;			
			}

			
			if (bRunQuery) {
				ExecuteSqlBatch(strSql);
			}

			m_pSelectedList->Requery();
			m_pAvailList->Clear();
			
		}
			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegementAllRight() ");
	
}

void CConfigureReportSegmentsDlg::OnSegmentAllLeft() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//first get the segmentID
		pRow = m_pSegmentList->CurSel;
		long nSegmentID;
		if (pRow) {
			nSegmentID = VarLong(pRow->GetValue(slcID));

			ExecuteParamSql("DELETE FROM ReportSegmentDetailsT WHERE SegmentID = {INT}", nSegmentID);	

			m_pAvailList->Requery();
			m_pSelectedList->Clear();
		}
			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegementAllLeft() ");
	
}

void CConfigureReportSegmentsDlg::MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pSelRow) {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//first get the segmentID
		pRow = m_pSegmentList->CurSel;
		long nSegmentID;
		if (pRow) {
			nSegmentID = VarLong(pRow->GetValue(slcID));

			if (pSelRow) {

				long nID = VarLong(pSelRow->GetValue(selID));

				ExecuteParamSql("DELETE FROM ReportSegmentDetailsT WHERE ItemID = {INT} AND SegmentID = {INT}", nID,nSegmentID);

				m_pAvailList->TakeRowAddSorted(pSelRow);
			}				
			
		}
			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::MoveOneLeft() ");
	
}

void CConfigureReportSegmentsDlg::MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pSelRow) {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//first get the segmentID
		pRow = m_pSegmentList->CurSel;
		long nSegmentID;
		if (pRow) {
			nSegmentID = VarLong(pRow->GetValue(slcID));

			if (pSelRow) {

				long nID = VarLong(pSelRow->GetValue(alcID));

				ExecuteParamSql("INSERT INTO ReportSegmentDetailsT (SegmentID, ItemID) VALUES ({INT}, {INT})", nSegmentID, nID);

				m_pSelectedList->TakeRowAddSorted(pSelRow);

			}
		}
			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::MoveOneRight() ");
}

	

void CConfigureReportSegmentsDlg::OnSegmentOneLeft() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pSelectedList->CurSel;
		if (pRow) {
			MoveOneLeft(pRow);
		}				
			

			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegementOneLeft() ");
	
}

void CConfigureReportSegmentsDlg::OnSegmentOneRight() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pAvailList->CurSel;
		if (pRow) {
			MoveOneRight(pRow);
		}				
			
			
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegementOneRight() ");
	
}

long CConfigureReportSegmentsDlg::GetSegmentType() 
{
	try {
		if (m_rdSegmentCategories.GetCheck() ){
			return stCategories;
		}
		else if (m_rdSegmentProviders.GetCheck() ){
			return stProviders;
		}
		else if (m_rdSegmentLocations.GetCheck() ) {
			return stLocations;
		}else if (m_rdSegmentApptTypes.GetCheck() ) {
			return stApptTypes;
		}
		else {
			ASSERT(FALSE);
			return -1;
		}
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::GetSegmentType() ");
	return -1;
}		

void CConfigureReportSegmentsDlg::OnSegmentAdd() 
{
	try {
		//prompt for a name
		CString strNewName;
		if (IDOK == (InputBoxLimited(this, "Please enter a name for the new segment.", strNewName, "",255,false,false,NULL))) {

			//check to see if it already exists
			if (ReturnsRecords("SELECT ID FROM ReportSegmentsT WHERE Name = '%s'", _Q(strNewName))) {

				MessageBox("There is already a segment with this name, please choose another.");
				return;
			}

			long nNewGroupID = NewNumber("ReportSegmentsT", "ID");

			long nTypeID = GetSegmentType();
			
			//create the new group
			ExecuteParamSql("INSERT INTO ReportSegmentsT (ID, Name, TypeID) VALUES "
				" ({INT}, {STRING}, {INT})", nNewGroupID, strNewName, nTypeID);

			
			//add the group to the list
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSegmentList->GetNewRow();
			pRow->PutValue(slcID, nNewGroupID);
			pRow->PutValue(slcType, nTypeID);
			pRow->PutValue(slcName, _variant_t(strNewName));
			m_pSegmentList->AddRowSorted(pRow, NULL);
			m_pSegmentList->CurSel = pRow;
			OnSelChosenSegmentGroupList(pRow);
		}
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentAdd()");
	
}

void CConfigureReportSegmentsDlg::OnSegmentDelete() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSegmentList->CurSel;
		if (pRow == NULL) {
			return;
		}

		long nValue = VarLong(pRow->GetValue(slcID));


		if (IDYES == MsgBox(MB_YESNO, "This will remove this segment, are you sure you wish to continue?")) {

			//now delete the report group
			ExecuteParamSql("DELETE FROM ReportSegmentDetailsT WHERE SegmentID = {INT}", nValue);
			ExecuteParamSql("DELETE FROM ReportSegmentsT WHERE ID = {INT}", nValue);

			m_pSegmentList->RemoveRow(pRow);
			pRow = m_pSegmentList->GetFirstRow();
			if (pRow) {
				m_pSegmentList->CurSel = pRow;
				OnSelChosenSegmentGroupList(pRow);
			}
			else {
				m_pAvailList->Clear();
				m_pSelectedList->Clear();
			}

		}
	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentDelete() ");
	
	
}

void CConfigureReportSegmentsDlg::OnSegmentRename() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSegmentList->CurSel;

		if (pRow) {
			//get the current segment ID
			long nSegmentID = VarLong(pRow->GetValue(slcID));

			//prompt for new name
			CString strNewName;
			if (InputBoxLimited(this, "New Report Segment Name:", strNewName, "",255,false,false,NULL) == IDOK) {

				//make sure it doesn't already exist
				if (ReturnsRecords("SELECT ID FROM ReportSegmentsT WHERE Name = '%s'", _Q(strNewName))) {

					//output a messagebox
					MsgBox("There is already a report segment with that name, please choose a different name");
					return;
				}

				//we are good to go
				ExecuteParamSql("Update ReportSegmentsT SET Name = {STRING} WHERE ID = {INT}", strNewName, nSegmentID);

				//Now, update the list on screen.
				pRow->PutValue(slcName, _variant_t(strNewName));
			}

		}

	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSegmentRename() ");
	
}

BEGIN_EVENTSINK_MAP(CConfigureReportSegmentsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureReportSegmentsDlg)
	ON_EVENT(CConfigureReportSegmentsDlg, IDC_SEGMENT_AVAIL, 3 /* DblClickCell */, OnDblClickCellSegmentAvail, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureReportSegmentsDlg, IDC_SEGMENT_SELECTED, 3 /* DblClickCell */, OnDblClickCellSegmentSelected, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureReportSegmentsDlg, IDC_SEGMENT_GROUP_LIST, 1 /* SelChanging */, OnSelChangingSegmentGroupList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureReportSegmentsDlg, IDC_SEGMENT_GROUP_LIST, 16 /* SelChosen */, OnSelChosenSegmentGroupList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigureReportSegmentsDlg::OnDblClickCellSegmentAvail(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneRight(pRow);
		}

	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnDblClickCellSegmentAvail");
	
}

void CConfigureReportSegmentsDlg::OnDblClickCellSegmentSelected(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneLeft(pRow);
		}

	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnDblClickCellSegmentSelected");
	
}

void CConfigureReportSegmentsDlg::OnSelChangingSegmentGroupList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CConfigureReportSegmentsDlg::OnSelChangingSegmentGroupList");
	
	
}

void CConfigureReportSegmentsDlg::OnSelChosenSegmentGroupList(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {

			CString strWhere, strTemp; 
			
			long nSegmentID = VarLong(pRow->GetValue(slcID));
			long nTypeID = GetSegmentType();

			if (nTypeID == stLocations) {
				strWhere  = "Managed = 1 AND ID NOT IN (SELECT ItemID FROM ReportSegmentDetailsT WHERE SegmentID = " + AsString(nSegmentID) + ")";
			}
			else {
				strWhere  = "ID NOT IN (SELECT ItemID FROM ReportSegmentDetailsT WHERE SegmentID = " + AsString(nSegmentID) + ")";
			}
			m_pAvailList->WhereClause = _bstr_t(strWhere);
			m_pAvailList->Requery();			


			strWhere = "SegmentID = " + AsString(nSegmentID);
			m_pSelectedList->WhereClause = _bstr_t(strWhere);

			m_pSelectedList->Requery();
		}

	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnSelChosenSegmentGroupList");
	
}

BOOL CConfigureReportSegmentsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnSegmentOneRight.AutoSet(NXB_RIGHT);
		m_btnSegmentOneLeft.AutoSet(NXB_LEFT);
		m_btnSegmentAllLeft.AutoSet(NXB_LLEFT);
		m_btnSegmentAllRight.AutoSet(NXB_RRIGHT);
		m_btnSegmentAdd.AutoSet(NXB_NEW);
		m_btnSegmentRename.AutoSet(NXB_MODIFY);
		m_btnSegmentDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);		

		//make all the radio buttons hidden for now until we make reports work with more then just the categories
		//TODO: when these are put in, we need to put the checks in for deletions etc for locations, providers, and appt types
		m_rdSegmentApptTypes.ShowWindow(SW_HIDE);
		m_rdSegmentCategories.ShowWindow(SW_HIDE);
		m_rdSegmentLocations.ShowWindow(SW_HIDE);
		m_rdSegmentProviders.ShowWindow(SW_HIDE);
		m_stSegmentType.ShowWindow(SW_HIDE);
		SetWindowText("Configure Category Segments");


		//bind the lists
		m_pSegmentList = BindNxDataList2Ctrl(IDC_SEGMENT_GROUP_LIST, false);
		m_pAvailList = BindNxDataList2Ctrl(IDC_SEGMENT_AVAIL, false);
		m_pSelectedList = BindNxDataList2Ctrl(IDC_SEGMENT_SELECTED, false);

		//check the categories
		m_rdSegmentCategories.SetCheck(1);
		OnSegmentCategories();		


	}NxCatchAll("Error in CConfigureReportSegmentsDlg::OnInitDialog() ");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
