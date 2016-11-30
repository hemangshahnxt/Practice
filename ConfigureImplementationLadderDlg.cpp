// ConfigureImplementationLadderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "ConfigureImplementationLadderDlg.h"
#include "ConfigureImplementationStepDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationLadderDlg dialog

/*
SQL Statements Associated with this Dialog

  Ladder Template Types
  CREATE TABLE ImplementationLadderTemplateTypeT (ID INT PRIMARY KEY, TypeName nVarChar(255))

  INSERT INTO ImplementationLadderTemplateTypeT(ID, TypeName) VALUES (1, 'EMR')
  INSERT INTO ImplementationLadderTemplateTypeT(ID, TypeName) VALUES (2, 'PM')
  INSERT INTO ImplementationLadderTemplateTypeT(ID, TypeName) VALUES (3, 'Sales')

  Ladder Templates
  CREATE TABLE ImplementationLadderTemplateT (ID INT PRIMARY KEY, Name nVarChar(255), Active bit not null default 1, TypeID INT NULL CONSTRAINT ImpLadderTempT_TypeID_ImpLadderTempTypeT_ID REFERENCES ImplementationLadderTemplateTypeT(ID))

  Step Templates
  CREATE TABLE ImplementationStepTemplatesT (ID INT PRIMARY KEY, 
  LadderTemplateID INT CONSTRAINT ImpStepTempsT_LadderTemplateID_ImpLadderTempT_ID REFERENCES ImplementationLadderTemplateT(ID), 
  ActionID INT NOT NULL, SortOrder INT NOT NULL,
  StepName NVarChar(255), Note nVarChar(1000), CreateToDo bit not null default 0, 
  ToDoUserID INT NULL,
  ToDoPriority INT NULL, ToDoCategoryID INT CONSTRAINT ImpStepTempT_ToDoCategoryID_NoteCatsF_ID REFERENCES NoteCatsF(ID),
  IsClientAssigned bit not null default 0, IsWebVisible bit not null default 0,
  DocumentCategoryID INT CONSTRAINT ImpStepTempT_DocumentCategoryID_NoteCatsF_ID REFERENCES NoteCatsF(ID))

  CREATE TABLE ImplementationStepTemplatesCriteriaT (StepTemplateID INT CONSTRAINT ImpStepTempCriteriaT_Step_TemplateID_ImpStepTempT_ID REFERENCES ImplementationStepTemplatesT(ID), 
  ActionItemID INT, ActionItemPath nVarChar(1000))

  ALTER TABLE ImplementationStepTemplatesT ADD AutoActivate bit NOT NULL default 0, ActivateTimeLength INT NULL, ActivateTimeInterval INT NULL


  
  */

enum LadderTemplateColumns {
	ltcID = 0,
	ltcName,
	ltcTypeID,
};

enum LadderTemplateTypeColumns {
	lttcID = 0,
	lttcName,
};

enum StepTemplateColumns {
	stcID =0,
	stcOrder,
	stcName,
	stcNotes,
};

CConfigureImplementationLadderDlg::CConfigureImplementationLadderDlg(CWnd* pParent)
	: CNxDialog(CConfigureImplementationLadderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureImplementationLadderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigureImplementationLadderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureImplementationLadderDlg)
	DDX_Control(pDX, IDC_COPY_IMPLEMENTATION_LADDER_TEMPLATE, m_btnCopy);
	DDX_Control(pDX, IDC_EDIT_IMPLEMENTATION_LADDER_TEMPLATE, m_btnEditLadder);
	DDX_Control(pDX, IDOK, m_btn_OK);
	DDX_Control(pDX, IDC_UP, m_btn_Up);
	DDX_Control(pDX, IDC_NEW_IMPLEMENTATION_STEP_TEMPLATE, m_btn_NewStep);
	DDX_Control(pDX, IDC_NEW_IMPLEMENTATION_LADDER_TEMPLATE, m_btn_AddLadder);
	DDX_Control(pDX, IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE, m_btn_EditStep);
	DDX_Control(pDX, IDC_DOWN, m_btn_Down);
	DDX_Control(pDX, IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE, m_btn_DeleteStep);
	DDX_Control(pDX, IDC_DELETE_IMPLEMENTATION_LADDER_TEMPLATE, m_btn_DeleteLadder);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CConfigureImplementationLadderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureImplementationLadderDlg)
	ON_BN_CLICKED(IDC_NEW_IMPLEMENTATION_LADDER_TEMPLATE, OnNewLadder)
	ON_BN_CLICKED(IDC_DELETE_IMPLEMENTATION_LADDER_TEMPLATE, OnDeleteLadder)
	ON_BN_CLICKED(IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE, OnDeleteStep)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE, OnEditStep)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_NEW_IMPLEMENTATION_STEP_TEMPLATE, OnNewImplementationStepTemplate)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_EDIT_IMPLEMENTATION_LADDER_TEMPLATE, OnEditImplementationLadderTemplate)
	ON_BN_CLICKED(IDC_COPY_IMPLEMENTATION_LADDER_TEMPLATE, OnCopyImplementationLadderTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationLadderDlg message handlers

void CConfigureImplementationLadderDlg::OnNewLadder() 
{
	try {

		//prompt them to enter a name
		CString strName;
		if (IDOK == InputBoxLimited(this, "Enter a new Implementation Ladder Template Name:", strName, "",255,false,false,NULL)) {

			strName.TrimLeft();
			strName.TrimRight();

			//make sure that this item doesn't exist already
			if (ReturnsRecords("SELECT ID FROM ImplementationLadderTemplateT WHERE Name like '%s'", _Q(strName))) {

				MsgBox("This ladder template name already exists.  Please choose a different ladder template name.");
			}
			else {

				strName.TrimLeft();
				strName.TrimRight();
				//make sure they actually entered a value
				if (strName.IsEmpty()) {
					MsgBox("You may not enter blank ladder template names");
				}
				else {
				
					//woo hoo we can keep going
					long nID = NewNumber("ImplementationLadderTemplateT", "ID");
					ExecuteSql("INSERT INTO ImplementationLadderTemplateT (ID, Name, TypeID) "
						" VALUES (%li, '%s', NULL)", nID, _Q(strName));

					//add it to the datalist
					NXDATALIST2Lib::IRowSettingsPtr pRow;
					pRow = m_pLadderTemplateList->GetNewRow();

					pRow->PutValue(ltcID, nID);
					pRow->PutValue(ltcName, _variant_t(strName));
					
					m_pLadderTemplateList->AddRowSorted(pRow, NULL);
					m_pLadderTemplateList->CurSel = pRow;
					OnSelChosenImplementationLadderList(pRow);

				}
			}
		}


	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnNewLadder");
	
}

void CConfigureImplementationLadderDlg::OnDeleteLadder() 
{
	try {

		//get the row
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->GetCurSel();

		if (pRow) {

			long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

			//check to see if there are client ladders with this ladderID
			ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Count(*) as CountLadders FROM ClientImplementationLadderT WHERE MasterLadderTemplateID = {INT}", nLadderTemplateID);
		
			if (! rs->eof) {
				long nCount = AdoFldLong(rs, "CountLadders", 0);

				if (nCount > 0) {
					MsgBox("There are client ladders which refer to this template, you may not delete it.  Please inactivate it instead");
					return;
				}
			
				//we are good to go
				if (IDYES == MsgBox(MB_YESNO, "Are you sure you wish to delete this ladder template?  This action is UNRECOVERABLE")) {

					//first the actionIDs
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID IN "
						" (SELECT ID FROM ImplementationStepTemplatesT WHERE LadderTemplateID = {INT})", nLadderTemplateID);

					//now the steps
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesT  WHERE LadderTemplateID = {INT}", nLadderTemplateID);

					//and finally the ladder
					ExecuteParamSql("DELETE FROM ImplementationLadderTemplateT WHERE ID = {INT}", nLadderTemplateID);
				}

				m_pLadderTemplateList->Requery();
				NXDATALIST2Lib::IRowSettingsPtr pCurSel;
				pCurSel = m_pLadderTemplateList->GetFirstRow();
				if (pCurSel) {
					m_pLadderTemplateList->CurSel = pCurSel;
				}
				Load();
			}
			else {
				ASSERT(FALSE);
			}
		}
	}NxCatchAll("Error In CConfigureImplementationLadderDlg::OnDeleteLadder() ");
	
}

void CConfigureImplementationLadderDlg::OnDeleteStep() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pStepList->CurSel;

		if (pRow) {

			long nStepTemplateID = VarLong(pRow->GetValue(stcID));
			long nSortOrder = VarLong(pRow->GetValue(stcOrder));

			pRow = m_pLadderTemplateList->CurSel;
			
			if (pRow) {

				long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

				if (IDYES == MsgBox(MB_YESNO, "Are you sure you wish to delete this step?  Note: this will only have an affect on new ladders, not existing ladders")) {

					//go ahead and delete it
					//first the actionIDs
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID  = {INT} ",
						nStepTemplateID);

					//now the steps
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesT  WHERE ID = {INT}", nStepTemplateID);

					//reorder
					ExecuteParamSql("UPDATE ImplementationStepTemplatesT SET SortOrder = SortOrder - 1 WHERE LadderTemplateID = {INT} AND SortOrder > {INT}",
						nLadderTemplateID, nSortOrder);

					CheckButtonStatus();

					m_pStepList->Requery();
				}
			}
		}

	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnDeleteStep() ");
	
}

void CConfigureImplementationLadderDlg::OnDown() 
{
	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStepList->CurSel;

		if (pRow) {
			if (pRow != m_pStepList->GetLastRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(stcOrder));
				long nNewOrderID = nOrigOrderID + 1;

				NXDATALIST2Lib::IRowSettingsPtr pRowLadder = m_pLadderTemplateList->CurSel;
			
				if (pRowLadder) {
					long nLadderTemplateID = VarLong(pRowLadder->GetValue(ltcID));

					ExecuteParamSql(" UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; "
						,-1, nNewOrderID, nLadderTemplateID,
						nNewOrderID, nOrigOrderID, nLadderTemplateID,
						nOrigOrderID, -1, nLadderTemplateID);
										
					pRow->PutValue(stcOrder, nNewOrderID);

					pRow = pRow->GetNextRow();
				
					if (pRow) {
						pRow->PutValue(stcOrder, nOrigOrderID);
					}

					m_pStepList->Sort();

					CheckButtonStatus();
				}
			}
		}
	}NxCatchAll("CConfigureImplementationLadderDlg::OnDown() ");
	
	
}

void CConfigureImplementationLadderDlg::OnEditStep() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pLadderTemplateList->CurSel;

	if (pRow) {

		long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

		pRow = m_pStepList->CurSel;
	
		if (pRow) {

			long nStepID = VarLong(pRow->GetValue(stcID));
		
			CConfigureImplementationStepDlg dlg(nStepID, nLadderTemplateID, true, this);
			long nResult = dlg.DoModal();

			if (nResult == IDOK) {
				m_pStepList->Requery();
			}
		}
		else {
			MsgBox("Please select a step to edit");
		}
		
	}
	else {
		MsgBox("Please select a ladder before editing a step");
	}
	
}

void CConfigureImplementationLadderDlg::OnUp() 
{
	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStepList->CurSel;

		if (pRow) {
			if (pRow != m_pStepList->GetFirstRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(stcOrder));
				long nNewOrderID = nOrigOrderID - 1;

				NXDATALIST2Lib::IRowSettingsPtr pRowLadder = m_pLadderTemplateList->CurSel;
				
				if (pRowLadder) {
					long nLadderTemplateID = VarLong(pRowLadder->GetValue(ltcID));

					ExecuteParamSql("UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; "
						"UPDATE ImplementationStepTemplatesT SET SortOrder = {INT} WHERE SortOrder = {INT} AND LadderTemplateID = {INT}; ",
						-1, nNewOrderID, nLadderTemplateID,
						nNewOrderID, nOrigOrderID, nLadderTemplateID,
						nOrigOrderID, -1, nLadderTemplateID);
					
					pRow->PutValue(stcOrder, nNewOrderID);

					pRow = pRow->GetPreviousRow();

					if (pRow) {
						pRow->PutValue(stcOrder, nOrigOrderID);
					}
					m_pStepList->Sort();

					CheckButtonStatus();
				}
			}
		}
	}NxCatchAll(" CConfigureImplementationLadderDlg::OnUp()");	
	
}


void CConfigureImplementationLadderDlg::CheckButtonStatus() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStepList->CurSel;

		if (pRow) {
			if (pRow == m_pStepList->GetFirstRow()) {
				GetDlgItem(IDC_UP)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_UP)->EnableWindow(TRUE);
			}

			if (pRow == m_pStepList->GetLastRow()) {
				GetDlgItem(IDC_DOWN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_DOWN)->EnableWindow(TRUE);
			}
		
		}
		else {
			//disable the buttons
			GetDlgItem(IDC_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_DOWN)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CheckButtonStatus");
}


void CConfigureImplementationLadderDlg::OnCancel() 
{

	OnOK();

}

void CConfigureImplementationLadderDlg::OnOK() 
{
	try {
		//check to make sure they have a type on this ladder
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->CurSel;
		if (pRow) {
			pRow = m_pLadderTemplateTypeList->CurSel;

			if (pRow == NULL)  {

				//give a message box
				MsgBox("You must pick a ladder template type for each ladder template");
				return;
			}
		}
		
		CDialog::OnOK();
	}NxCatchAll("Error In CConfigureImplementationLadderDlg::OnOK");
}

BOOL CConfigureImplementationLadderDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
	
		m_btn_AddLadder.AutoSet(NXB_NEW);
		m_btn_DeleteLadder.AutoSet(NXB_DELETE);
		m_btn_Up.AutoSet(NXB_UP);
		m_btn_Down.AutoSet(NXB_DOWN);		
		m_btn_NewStep.AutoSet(NXB_NEW);
		m_btn_DeleteStep.AutoSet(NXB_DELETE);
		m_btnEditLadder.AutoSet(NXB_MODIFY);
		m_btn_OK.AutoSet(NXB_CLOSE);
		m_btn_EditStep.AutoSet(NXB_MODIFY);


		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

		m_pLadderTemplateList = BindNxDataList2Ctrl(IDC_IMPLEMENTATION_LADDER_LIST, true);
		m_pLadderTemplateTypeList = BindNxDataList2Ctrl(IDC_LADDER_TYPE, true);
		m_pStepList = BindNxDataList2Ctrl(IDC_LADDER_STEPS_LIST, true);

		// (j.gruber 2008-05-22 11:05) - PLID 29442 - made the step list not sortable
		m_pStepList->AllowSort = FALSE;
		
	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnInitDialog()");
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureImplementationLadderDlg::OnNewImplementationStepTemplate() 
{
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->CurSel;

		if (pRow) {
				
			long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

			CConfigureImplementationStepDlg dlg(-1, nLadderTemplateID, true, this);
			long nResult = dlg.DoModal();

			if (nResult == IDOK) {

				m_pStepList->Requery();
			}			
		}
		else {
			MsgBox("Please select a ladder before making a new step");
		}
	}NxCatchAll("CConfigureImplementationLadderDlg::OnNewImplementationStepTemplate() ");
	
}

HBRUSH CConfigureImplementationLadderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
		if (nCtlColor == CTLCOLOR_STATIC || pWnd->GetDlgCtrlID() == IDC_STATIC)
	{
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		return m_brush;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CConfigureImplementationLadderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureImplementationLadderDlg)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_IMPLEMENTATION_LADDER_LIST, 1 /* SelChanging */, OnSelChangingImplementationLadderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_IMPLEMENTATION_LADDER_LIST, 16 /* SelChosen */, OnSelChosenImplementationLadderList, VTS_DISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_LADDER_STEPS_LIST, 16 /* SelChosen */, OnSelChosenLadderStepsList, VTS_DISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_LADDER_STEPS_LIST, 1 /* SelChanging */, OnSelChangingLadderStepsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_LADDER_TYPE, 16 /* SelChosen */, OnSelChosenLadderType, VTS_DISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_LADDER_TYPE, 1 /* SelChanging */, OnSelChangingLadderType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_IMPLEMENTATION_LADDER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedImplementationLadderList, VTS_I2)
	ON_EVENT(CConfigureImplementationLadderDlg, IDC_LADDER_STEPS_LIST, 2 /* SelChanged */, OnSelChangedLadderStepsList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigureImplementationLadderDlg::OnSelChangingImplementationLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{

	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}

	//check to make sure they have a type on this ladder
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pLadderTemplateList->CurSel;
	if (pRow) {
		pRow = m_pLadderTemplateTypeList->CurSel;

		if (pRow == NULL)  {

			//give a message box
			MsgBox("You must pick a ladder template type for each ladder template");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}	
	}
	
}

void CConfigureImplementationLadderDlg::Load() {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->CurSel;

		if (pRow) {

			long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

			CString strWhere;
			strWhere.Format(" ImplementationStepTemplatesT.LadderTemplateID = %li", nLadderTemplateID);

			m_pStepList->WhereClause = _bstr_t(strWhere);
			m_pStepList->Requery();

			//now set the type
			_variant_t varType = pRow->GetValue(ltcTypeID);
			if (varType.vt == VT_I4) {
				long nTypeID = VarLong(pRow->GetValue(ltcTypeID), -1);
				m_pLadderTemplateTypeList->SetSelByColumn(lttcID, nTypeID);
			}
			else {
				//set it to nothing
				m_pLadderTemplateTypeList->CurSel = NULL;
			}

			
		}

	}NxCatchAll("Error in Load");
}

void CConfigureImplementationLadderDlg::OnSelChosenImplementationLadderList(LPDISPATCH lpRow) 
{
	try {
		Load();
	}NxCatchAll("CConfigureImplementationLadderDlg::OnSelChosenImplementationLadderList");
	
}

void CConfigureImplementationLadderDlg::OnSelChosenLadderStepsList(LPDISPATCH lpRow) 
{
	try {
		CheckButtonStatus();
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pStepList->CurSel;

		if (pRow) {
			GetDlgItem(IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(FALSE);
		}
	}NxCatchAll("CConfigureImplementationLadderDlg::OnSelChosenLadderStepsList(LPDISPATCH lpRow) ");
	
}

void CConfigureImplementationLadderDlg::OnSelChangingLadderStepsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CConfigureImplementationLadderDlg::OnSelChosenLadderType(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->CurSel;

		if (pRow) {

			long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

			NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_pLadderTemplateTypeList->CurSel;
			
			if (pRow2) {

				long nTypeID = VarLong(pRow2->GetValue(lttcID));

				ExecuteParamSql("UPDATE ImplementationLadderTemplateT SET TypeID = {INT} WHERE ID = {INT}",
					nTypeID, nLadderTemplateID);

				pRow->PutValue(ltcTypeID, nTypeID);
			}
		}
	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnSelChosenLadderType");
	
}

void CConfigureImplementationLadderDlg::OnSelChangingLadderType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnSelChangingLadderType");
	
}

void CConfigureImplementationLadderDlg::OnRequeryFinishedImplementationLadderList(short nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->GetFirstRow();
		if (pRow) {
			m_pLadderTemplateList->CurSel = pRow;
		}
		Load();
	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnRequeryFinishedImplementationLadderList");
	
}

void CConfigureImplementationLadderDlg::OnSelChangedLadderStepsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		CheckButtonStatus();
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pStepList->CurSel;

		if (pRow) {
			GetDlgItem(IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_DELETE_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_IMPLEMENTATION_STEP_TEMPLATE)->EnableWindow(FALSE);
		}


	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnSelChangedLadderStepsList");
	
}

// (j.gruber 2007-12-11 11:12) - PLID 28324 - add ability to edit ladder name
void CConfigureImplementationLadderDlg::OnEditImplementationLadderTemplate() 
{
	try {
		CString strName;

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->CurSel;

		if (pRow) {

			long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

			if (IDOK == InputBoxLimited(this, "Enter the new Implementation Ladder Template Name:", strName, "",255,false,false,NULL)) {

				ExecuteParamSql("UPDATE ImplementationLadderTemplateT SET Name = {STRING} WHERE ID = {INT}", strName, nLadderTemplateID);

				pRow->PutValue(ltcName, _variant_t(strName));
			}
		}


	}NxCatchAll("Error in CConfigureImplementationLadderDlg::OnEditImplementationLadderTemplate() ");

}

// (j.gruber 2008-05-22 10:39) - PLID 28611 - added ability to copy a ladder template
void CConfigureImplementationLadderDlg::OnCopyImplementationLadderTemplate() 
{

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderTemplateList->GetCurSel();

		if (pRow) {

			//find out what name they would like to name this
			CString strName;
			if (IDOK == InputBoxLimited(this, "Enter the new Implementation Ladder Template Name:", strName, "",255,false,false,NULL)) {

				strName.TrimLeft();
				strName.TrimRight();
				
				if (ReturnsRecords("SELECT ID FROM ImplementationLadderTemplateT WHERE Name like '%s'", _Q(strName))) {

					MsgBox("This ladder template name already exists.  Please choose a different ladder template name.");
				}
				else {

					long nLadderTemplateID = VarLong(pRow->GetValue(ltcID));

					//now make the new record in ImplementationLadderTemplateT
					long nNewLadderTemplateID = NewNumber("ImplementationLadderTemplateT", "ID");
					ExecuteParamSql("INSERT INTO ImplementationLadderTemplateT (ID, Name, Active, TypeID)  "
						" SELECT {INT}, {STRING}, Active, TypeID FROM ImplementationLadderTemplateT WHERE ID = {INT}",
						nNewLadderTemplateID, strName, nLadderTemplateID);

					//note the Steps
					ADODB::_RecordsetPtr rsSteps = CreateParamRecordset("SELECT ID FROM ImplementationStepTemplatesT WHERE LadderTemplateID = {INT}",
						nLadderTemplateID);

					while (! rsSteps->eof) {
							
						long nStepID = AdoFldLong(rsSteps, "ID");
						long nNewStepID = NewNumber("ImplementationStepTemplatesT", "ID");

						ExecuteParamSql("INSERT INTO ImplementationStepTemplatesT (ID, LadderTemplateID, ActionID, SortOrder, StepName, "
							" Note, CreateToDo, ToDoUserID, ToDoPriority, ToDoCategoryID, IsClientAssigned, IsWebVisible, DocumentCategoryID,  "
							" AutoActivate, ActivateTimeLength, ActivateTimeInterval) "
							" SELECT {INT},{INT}, ActionID, SortOrder, StepName, "
							" Note, CreateToDo, ToDoUserID, ToDoPriority, ToDoCategoryID, IsClientAssigned, IsWebVisible, DocumentCategoryID,  "
							" AutoActivate, ActivateTimeLength, ActivateTimeInterval "
							" FROM ImplementationStepTemplatesT WHERE ID = {INT}",
							nNewStepID, nNewLadderTemplateID, nStepID);

						ExecuteParamSql("INSERT INTO ImplementationStepTemplatesCriteriaT (StepTemplateID, ActionItemID, ActionItemPath) "
							" SELECT {INT}, ActionItemID, ActionItemPath FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = {INT}",
							nNewStepID, nStepID);

						rsSteps->MoveNext();
					}

					NXDATALIST2Lib::IRowSettingsPtr pNewRow;
					pNewRow = m_pLadderTemplateList->GetNewRow();
					pNewRow->PutValue(ltcID, nNewLadderTemplateID);
					pNewRow->PutValue(ltcName, _variant_t(strName));
					pNewRow->PutValue(ltcTypeID, pRow->GetValue(ltcTypeID));
					m_pLadderTemplateList->AddRowSorted(pNewRow, NULL);
					m_pLadderTemplateList->CurSel = pNewRow;
					OnSelChosenImplementationLadderList(pNewRow);
				}
			}
		}
	}NxCatchAll("Error In  CConfigureImplementationLadderDlg::OnCopyImplementationLadderTemplate() ");	
}
