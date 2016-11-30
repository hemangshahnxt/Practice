// ConfigureImplementationStepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRC.h"
#include "ConfigureImplementationStepDlg.h"
#include "MultiSelectDlg.h"
#include "Fileutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum ColumnListAssignTo {
	claID = 0,
	claName,
};

enum ColumnListAction {
	clacID = 0,
	clacAction,
};

enum ColumnListActionItem {
	claiID = 0,
	claiDescription,
};

enum ColumnListPriority {

	clpID = 0,
	clpName,
};

enum ColumnListToDoCategory {
	cltcID = 0,
	cltcName,
};

enum ColumnListDocumentCategory {
	cldcID = 0,
	cldcName,
};


//this is copied in implementationdlg.cpp
//and is in data
//IT CANNOT BE CHANGED!!!
// (j.gruber 2008-05-22 12:35) - PLID 29445 - adding a custom step
enum ActionTypes {
	atEmailDocument = 1,
	atCompleteEMR,
	atSelectProcedures,
	atCustomizeTemplates,
	atMergeDocument,
	atMergePacket,
	atAptByCategory,
	atAptByType,	
	atCustom,
};


//this is copied in implementationdlg.cpp
//and is in data
//IT CANNOT BE CHANGED!!!
enum TimeInterval {
	tiImmediately = 0,
	tiDays,
	tiWeeks,
	tiMonths,
	tiYears,
};

enum TimeIntervalColumns {
	ticID = 0,
	ticName,
};




/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationStepDlg dialog


CConfigureImplementationStepDlg::CConfigureImplementationStepDlg(long nStepID, long nLadderID, bool bIsTemplate,
	 CWnd* pParent)
	: CNxDialog(CConfigureImplementationStepDlg::IDD, pParent)
{

	m_nStepID = nStepID;
	m_nLadderID = nLadderID;
	m_bIsTemplate = bIsTemplate;
	//{{AFX_DATA_INIT(CConfigureImplementationStepDlg)
	//}}AFX_DATA_INIT
}


void CConfigureImplementationStepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureImplementationStepDlg)
	DDX_Control(pDX, IDC_ACTION_LINK, m_nxlActionLabel);
	DDX_Control(pDX, IDCANCEL, m_btn_Cancel);
	DDX_Control(pDX, IDOK, m_btn_OK);
	DDX_Control(pDX, IDC_STEP, m_nxeditStep);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_NOTE, m_nxeditNote);
	DDX_Control(pDX, IDC_TIME_LENGTH, m_nxeditTimeLength);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CConfigureImplementationStepDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureImplementationStepDlg)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)		
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_AUTO_ACTIVATE, OnAutoActivate)
	ON_BN_CLICKED(IDC_AFTER_X_TIME, OnAfterXTime)
	ON_BN_CLICKED(IDC_ACTIVATE_IMMEDIATELY, OnActivateImmediately)
	ON_BN_CLICKED(IDC_STEP_CREATE_TODO, OnStepCreateTodo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationStepDlg message handlers

BOOL CConfigureImplementationStepDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	try {

		m_btn_OK.AutoSet(NXB_OK);
		m_btn_Cancel.AutoSet(NXB_CANCEL);
	
		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

		m_pPriorityList = BindNxDataList2Ctrl(IDC_TODO_PRIORITY_LIST, true);
		m_pCategoryList = BindNxDataList2Ctrl(IDC_TODO_CATEGORY_LIST, true);
		m_pAssignToList = BindNxDataList2Ctrl(IDC_TODO_USERS_LIST, true);
		m_pActionList = BindNxDataList2Ctrl(IDC_IMPLEMENTATION_STEP_ACTION, false);
		m_pActionItemList = BindNxDataList2Ctrl(IDC_IMPLEMENTATION_ACTION_ITEM_LIST, false);
		m_pDocCategoryList = BindNxDataList2Ctrl(IDC_DOCUMENT_CATEGORY_LIST, true);
		m_pTimeIntervalList = BindNxDataList2Ctrl(IDC_TIME_INTERVAL, false);


		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pAssignToList->GetNewRow();
		pRow->PutValue(claID, (long) -1);
		pRow->PutValue(claName, _variant_t("<EMR Specialist>"));
		m_pAssignToList->AddRowSorted(pRow, NULL);
		

		pRow = m_pAssignToList->GetNewRow();
		pRow->PutValue(claID, (long) -2);
		pRow->PutValue(claName, _variant_t("<Install Incident Owner>"));
		m_pAssignToList->AddRowSorted(pRow, NULL);

		//load the action list
		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atEmailDocument);
		pRow->PutValue(clacAction, _variant_t("<Email Document>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atCompleteEMR);
		pRow->PutValue(clacAction, _variant_t("<Complete EMR>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atMergeDocument);
		pRow->PutValue(clacAction, _variant_t("<Merge Document>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atMergePacket);
		pRow->PutValue(clacAction, _variant_t("<Merge Packet>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atAptByCategory);
		pRow->PutValue(clacAction, _variant_t("<Apt By Category>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atAptByType);
		pRow->PutValue(clacAction, _variant_t("<Apt By Type>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atSelectProcedures);
		pRow->PutValue(clacAction, _variant_t("<Select Procedures>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atCustomizeTemplates);
		pRow->PutValue(clacAction, _variant_t("<Customize Templates>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		// (j.gruber 2008-05-22 12:36) - PLID 29445
		pRow = m_pActionList->GetNewRow();
		pRow->PutValue(clacID, (long) atCustom);
		pRow->PutValue(clacAction, _variant_t("<Custom Step>"));
		m_pActionList->AddRowSorted(pRow, NULL);

		//load the time interval list
		pRow = m_pTimeIntervalList->GetNewRow();
		pRow->PutValue(ticID, (long) tiDays);
		pRow->PutValue(ticName, _variant_t("Day(s)"));
		m_pTimeIntervalList->AddRowSorted(pRow, NULL);

		pRow = m_pTimeIntervalList->GetNewRow();
		pRow->PutValue(ticID, (long) tiWeeks);
		pRow->PutValue(ticName, _variant_t("Weeks(s)"));
		m_pTimeIntervalList->AddRowSorted(pRow, NULL);
		
		pRow = m_pTimeIntervalList->GetNewRow();
		pRow->PutValue(ticID, (long) tiMonths);
		pRow->PutValue(ticName, _variant_t("Month(s)"));
		m_pTimeIntervalList->AddRowSorted(pRow, NULL);

		pRow = m_pTimeIntervalList->GetNewRow();
		pRow->PutValue(ticID, (long) tiYears);
		pRow->PutValue(ticName, _variant_t("Year(s)"));
		m_pTimeIntervalList->AddRowSorted(pRow, NULL);


		m_nxlActionLabel.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxlActionLabel.SetText("Choose Action Here");
		m_nxlActionLabel.SetType(dtsHyperlink);

		Load();
		
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnInitDialog");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


CString CConfigureImplementationStepDlg::GetActionItemText(long nActionID) {
	
	try {

		ADODB::_RecordsetPtr rs;

		CString strIDs;
		for (int i = 0; i < m_dwActionItemIDs.GetSize(); i++) {
			strIDs += AsString((long)m_dwActionItemIDs.GetAt(i)) + ",";
		}
		strIDs.TrimRight(",");

		switch (nActionID) {

			case atCompleteEMR:
				rs = CreateRecordset("SELECT ID, Name as Description FROM EMRTemplateT WHERE ID IN (%s)", strIDs);
			break;

			case atMergePacket:
				rs = CreateRecordset("SELECT ID, Name as Description FROM PacketsT WHERE Deleted = 0 and ProcedureRelated  = 0 AND ID IN (%s)", strIDs);
			break;
			case atAptByCategory:
				rs = CreateRecordset("SELECT * FROM ( "
					" SELECT 1 as ID, 'Consult' as Description  "
					" UNION  "
					" SELECT 2, 'PreOp' "
					" UNION "
					" SELECT 3, 'Minor Procedure' "
					" UNION "
					" SELECT 4, 'Surgery' "
					" UNION  "
					" SELECT 5, 'Follow-up' "
					" UNION  "
					" SELECT 6, 'Other Procedure') Q WHERE ID IN (%s)", strIDs);
			break;
			case atAptByType:
				rs = CreateRecordset("SELECT ID, Name as Description FROM AptTypeT WHERE InActive = 0 AND ID IN (%s)", strIDs);
			break;
		}

		CString strText;
		while (!rs->eof) {

			strText += AdoFldString(rs, "Description") + ", ";

			rs->MoveNext();
		}

		strText.TrimRight(", ");

		return strText;
	}NxCatchAll("Error in CConfigureImplementationStepDlg::GetActionItemText");

	return "";
}


void CConfigureImplementationStepDlg::AddToIDArray(long nIDToAdd) {

	try {
		BOOL bCanAdd = TRUE;

		for (int i = 0; i < m_dwActionItemIDs.GetSize(); i++) {

			if (((long)m_dwActionItemIDs.GetAt(i)) == nIDToAdd) {
				bCanAdd = FALSE;
			}
		}

		if (bCanAdd) {
			m_dwActionItemIDs.Add(nIDToAdd);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::AddToIDArray");
}


void CConfigureImplementationStepDlg::AddToStringArray(CString strStringToAdd) {

	try {
		BOOL bCanAdd = TRUE;

		for (int i = 0; i < m_strActionItemPaths.GetSize(); i++) {

			if (m_strActionItemPaths.GetAt(i).CompareNoCase(strStringToAdd) == 0) {
				bCanAdd = FALSE;
			}
		}

		if (bCanAdd) {
			m_strActionItemPaths.Add(strStringToAdd);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::AddToStringArray");
}


void CConfigureImplementationStepDlg::LoadActionItems(long nActionID) {

	try {

		m_dwActionItemIDs.RemoveAll();
		m_strActionItemPaths.RemoveAll();


		if (m_nStepID) {

			//its an existing step
			CString strIDField, strTable;
			if (m_bIsTemplate) {
				strTable = "ImplementationStepTemplatesCriteriaT";
				strIDField = "StepTemplateID";
			}
			else {
				strTable = "ClientImplementationStepCriteriaT";
				strIDField = "StepID";
			}

			ADODB::_RecordsetPtr rs = CreateRecordset("SELECT ActionItemID, ActionItemPath FROM %s WHERE %s = %li", strTable, strIDField, m_nStepID);
			while (!rs->eof) {

				switch (nActionID) {

					case atCompleteEMR:
					case atMergePacket:
					case atAptByCategory:
					case atAptByType:
						AddToIDArray(AdoFldLong(rs, "ActionItemID"));						
					break;

					case atEmailDocument:
					case atMergeDocument:
						AddToStringArray(AdoFldString(rs, "ActionItemPath"));
					break;
				}
				rs->MoveNext();
			}

			//now we have to load this into the dialog
			if (m_dwActionItemIDs.GetSize() > 0) {

				if (m_dwActionItemIDs.GetSize() > 1) {

					//show the link
					GetDlgItem(IDC_ACTION_LINK)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);

					m_nxlActionLabel.SetText(GetActionItemText(nActionID));
				}
				else {

					//there is only one
					GetDlgItem(IDC_ACTION_LINK)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);

					m_pActionItemList->SetSelByColumn(claiID, (long)m_dwActionItemIDs.GetAt(0));
				}
			}

			if (m_strActionItemPaths.GetSize() > 0) {

				GetDlgItem(IDC_ACTION_LINK)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);

				CString strText;
				for (int i = 0; i < m_strActionItemPaths.GetSize(); i++) {

					strText += m_strActionItemPaths.GetAt(i) + ";";
				}
				//remove the last ";"
				strText = strText.Left(strText.GetLength() - 1);

				m_nxlActionLabel.SetText(strText);
			}					

		}
		else {

			//we are just loading the regular stuff
			LoadActionItemDatalist(nActionID);
		
		}

	}NxCatchAll("Error in CConfigureImplementationStepDlg::LoadActionItems");



}

void CConfigureImplementationStepDlg::Load()  {

	try {

		if (m_nStepID != -1) {
		
			CString strTableName;
			
			if (m_bIsTemplate) {
				strTableName = "ImplementationStepTemplatesT";
			}
			else {
				strTableName = "ClientImplementationStepsT";
			}
			

			CString strSql;
			strSql.Format("SELECT ActionID, SortOrder, StepName, CreateToDo, Note, ToDoUserID, ToDoPriority, ToDoCategoryID, "
				" IsClientAssigned, IsWebVisible, DocumentCategoryID, AutoActivate, ActivateTimeInterval, ActivateTimeLength "
				" FROM %s WHERE ID = {INT}", strTableName);
			
			ADODB::_RecordsetPtr rs = CreateParamRecordset(strSql, m_nStepID);

			m_pActionList->SetSelByColumn(clacID, AdoFldLong(rs, "ActionID", -1));
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			long nActionID = AdoFldLong(rs, "ActionID", -1);
			
			LoadActionItemDatalist(nActionID);
						
			LoadActionItems(nActionID);

			SetDlgItemText(IDC_NAME, AdoFldString(rs, "StepName", ""));
			SetDlgItemInt(IDC_STEP, AdoFldLong(rs, "SortOrder"));
			SetDlgItemText(IDC_NOTE, AdoFldString(rs, "Note", ""));

			
			m_pDocCategoryList->SetSelByColumn(cldcID, AdoFldLong(rs, "DocumentCategoryID", -1));

			if (AdoFldBool(rs, "IsClientAssigned", false)) {
				CheckDlgButton(IDC_IS_CLIENT_ASSIGNED, 1);
			}
			else {
				CheckDlgButton(IDC_IS_CLIENT_ASSIGNED, 0);
			}

			if (AdoFldBool(rs, "CreateToDo", false)) {
				CheckDlgButton(IDC_STEP_CREATE_TODO, 1);
				m_pPriorityList->SetSelByColumn(clpID, AdoFldLong(rs, "ToDoPriority", -1));
				m_pCategoryList->SetSelByColumn(cltcID, AdoFldLong(rs, "ToDoCategoryID", -1));
				m_pAssignToList->SetSelByColumn(claID, AdoFldLong(rs, "ToDoUserID", -1));
			}
			else {
				CheckDlgButton(IDC_STEP_CREATE_TODO, 0);

				GetDlgItem(IDC_TODO_PRIORITY_LIST)->EnableWindow(FALSE);
				GetDlgItem(IDC_TODO_CATEGORY_LIST)->EnableWindow(FALSE);
				GetDlgItem(IDC_TODO_USERS_LIST)->EnableWindow(FALSE);

			}

			if (AdoFldBool(rs, "IsWebVisible", false)) {
				CheckDlgButton(IDC_IS_VISIBLE_ON_WEB, 1);
			}
			else {
				CheckDlgButton(IDC_IS_VISIBLE_ON_WEB, 0);
			}

			if (AdoFldBool(rs, "AutoActivate", false)) {
				CheckDlgButton(IDC_AUTO_ACTIVATE, 1);
				OnAutoActivate();

				long nTimeInterval = AdoFldLong(rs, "ActivateTimeInterval", 0);
				
				switch (nTimeInterval) {

					case tiImmediately:
						CheckDlgButton(IDC_ACTIVATE_IMMEDIATELY, 1);
						OnActivateImmediately();
					break;

					default:
						{
							//everything else
							CheckDlgButton(IDC_AFTER_X_TIME, 1);
							OnAfterXTime();
							long nTimeLength = AdoFldLong(rs, "ActivateTimeLength", 0);
							SetDlgItemInt(IDC_TIME_LENGTH, nTimeLength);

							m_pTimeIntervalList->SetSelByColumn(ticID, nTimeInterval);
						}
					break;
				}
			}
			else {
				CheckDlgButton(IDC_AUTO_ACTIVATE, 0);
				OnAutoActivate();
			}

		}
		else {
			CheckDlgButton(IDC_AUTO_ACTIVATE, 0);
			OnAutoActivate();
		}


	}NxCatchAll("Error in CConfigureImplementationStepDlg::Load");


}

HBRUSH CConfigureImplementationStepDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if ((nCtlColor == CTLCOLOR_STATIC || pWnd->GetDlgCtrlID() == IDC_STATIC) 
		&& pWnd->GetDlgCtrlID() != IDC_STEP && pWnd->GetDlgCtrlID() != IDC_TIME_LENGTH)
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

void CConfigureImplementationStepDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


void CConfigureImplementationStepDlg::SaveActionItems(long nStepID, long nActionID)  
{
	try {

		

		CString strSql;
		strSql = BeginSqlBatch();


		//first we need to delete what is already there
		if (m_nStepID != -1) {
			if (m_bIsTemplate) {
				AddStatementToSqlBatch(strSql, "DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = %li", m_nStepID);
			}
			else {
				AddStatementToSqlBatch(strSql, "DELETE FROM ClientImplementationStepCriteriaT WHERE StepID = %li", m_nStepID);
			}
		}


		switch (nActionID) {

			
			case atCompleteEMR:
			case atMergePacket:
			case atAptByCategory:
			case atAptByType:
				{

				for (int i = 0; i < m_dwActionItemIDs.GetSize(); i++) {
					if (m_bIsTemplate) {
						AddStatementToSqlBatch(strSql, "INSERT INTO ImplementationStepTemplatesCriteriaT (StepTemplateID, ActionItemID) "
							" VALUES (%li, %li)", nStepID, m_dwActionItemIDs.GetAt(i));
					}
					else {
						AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT (StepID, ActionItemID) "
							" VALUES (%li, %li)", nStepID, m_dwActionItemIDs.GetAt(i));
					}
				}
				}


			break;

			case atSelectProcedures:
				if (! m_bIsTemplate) {
					AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT (StepID, ActionItemID) "
						" SELECT %li, ID FROM ImplementationProcedureT ",
						nStepID);
				}
				else {
					
				}
			break;
			case atCustomizeTemplates:	
				if (! m_bIsTemplate) {
					AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT (StepID, ActionItemID) "
						" SELECT %li, ID FROM ImplementationEMRTemplatesT ",
						nStepID);
				}
				else {
					
				}

			break;

			case atEmailDocument:
			case atMergeDocument:
				{
				for (int i = 0; i < m_strActionItemPaths.GetSize(); i++) {
					if (m_bIsTemplate) {
						AddStatementToSqlBatch(strSql, "INSERT INTO ImplementationStepTemplatesCriteriaT (StepTemplateID, ActionItemPath) "
							" VALUES (%li, '%s')", nStepID, _Q(m_strActionItemPaths.GetAt(i)));
					}
					else {
						AddStatementToSqlBatch(strSql, "INSERT INTO ClientImplementationStepCriteriaT (StepID, ActionItemPath) "
							" VALUES (%li, '%s')", nStepID, _Q(m_strActionItemPaths.GetAt(i)));
					}
				}
				}

			break;	
		
		}

		ExecuteSqlBatch(strSql);				

		

		
		
	}NxCatchAll("Error in CConfigureImplementationStepDlg::SaveActionItems");
}

void CConfigureImplementationStepDlg::OnOK() 
{
	try {

		//get all the variables, except action items since they are already taken care of
		long nActionID, nPriority, nToDoCategoryID, nToDoAssignTo, nDocCategoryID;
		CString strName, strNote;
		bool bIsWebVisible, bIsClientAssigned, bCreateToDo;

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pActionList->CurSel;
		if (pRow) {
			nActionID = VarLong(pRow->GetValue(clacID));
		}
		else {
			MsgBox("Please select an action for this step before closing");
			return;
		}


		GetDlgItemText(IDC_NAME, strName);
		GetDlgItemText(IDC_NOTE, strNote);

		
		if (IsDlgButtonChecked(IDC_STEP_CREATE_TODO)) {
			bCreateToDo = true;

			pRow = m_pPriorityList->CurSel;
			if (pRow) {
				nPriority = VarLong(pRow->GetValue(clpID));
			}
			else {
				MsgBox("Please choose a priority or uncheck the create todo box");
			}

			pRow = m_pCategoryList->CurSel;
			if (pRow) {
				nToDoCategoryID = VarLong(pRow->GetValue(cltcID));
			}
			else {
				MsgBox("Please choose a ToDo category or uncheck the create todo box");
			}

			pRow = m_pAssignToList->CurSel;
			if (pRow) {
				nToDoAssignTo = VarLong(pRow->GetValue(claID));
			}
			else {
				MsgBox("Please choose a user to assign the ToDo to or uncheck the create todo box");
			}
		
		}
		else {
			bCreateToDo = false;
		}

		pRow = m_pDocCategoryList->CurSel;
		if (pRow) {
			nDocCategoryID = VarLong(pRow->GetValue(cldcID));
		}
		else {
			nDocCategoryID = -1;
		}
			
		bIsClientAssigned = IsDlgButtonChecked(IDC_IS_CLIENT_ASSIGNED) ? true : false;
		bIsWebVisible = IsDlgButtonChecked(IDC_IS_VISIBLE_ON_WEB) ? true : false;

		long nTimeInterval;
		long nTimeLength;
		BOOL bAutoActivate;
		if (IsDlgButtonChecked(IDC_AUTO_ACTIVATE)) {
			bAutoActivate = TRUE;
			
			if (IsDlgButtonChecked(IDC_ACTIVATE_IMMEDIATELY)) {
				nTimeLength = 0;
				nTimeInterval = tiImmediately;
			}
			else {
				nTimeLength = GetDlgItemInt(IDC_TIME_LENGTH);
				
				NXDATALIST2Lib::IRowSettingsPtr pRowInt;
				pRowInt = m_pTimeIntervalList->CurSel;
				if (pRowInt) {
					nTimeInterval = VarLong(pRowInt->GetValue(ticID));
				}
			}

		}
		else {
			bAutoActivate = FALSE;
		}


		if (m_bIsTemplate) {

			//is this a new step, or are we just editing?
			if (m_nStepID != -1) {

				//we are editing a step template
				ExecuteSql("UPDATE ImplementationStepTemplatesT SET ActionID = %li, StepName = '%s', Note = '%s', "
					" CreateToDo = %li, ToDoUserID = %s, ToDoCategoryID = %s, ToDoPriority = %s, IsClientAssigned = %li, "
					" IsWebVisible = %li, DocumentCategoryID = %s, AutoActivate = %li, ActivateTimeLength = %s, ActivateTimeInterval = %s WHERE ID = %li ", 
					nActionID, _Q(strName), _Q(strNote), 
					bCreateToDo ? 1 : 0,
					bCreateToDo ? AsString(nToDoAssignTo) : "NULL",
					bCreateToDo ? AsString(nToDoCategoryID) : "NULL",
					bCreateToDo ? AsString(nPriority) : "NULL",
					bIsClientAssigned ? 1:0,
					bIsWebVisible ? 1:0,
					nDocCategoryID == -1 ? "NULL" : AsString(nDocCategoryID),
					bAutoActivate,
					bAutoActivate ? AsString(nTimeLength) : "NULL",
					bAutoActivate ? AsString(nTimeInterval) : "NULL", m_nStepID);

				SaveActionItems(m_nStepID, nActionID);				

			}
			else {

				//saving a new step template
				long nStepTemplateID = NewNumber("ImplementationStepTemplatesT", "ID");
				long nStepOrder = NewNumber("ImplementationStepTemplatesT WHERE LadderTemplateID = " + AsString(m_nLadderID), "SortOrder");

				ExecuteSql("INSERT INTO ImplementationStepTemplatesT (ID, LadderTemplateID, ActionID, SortOrder, StepName, Note, "
					" CreateToDo, ToDoUserID, ToDoCategoryID, TodoPriority, IsClientAssigned, "
					" IsWebVisible, DocumentCategoryID, AutoActivate, ActivateTimeLength, ActivateTimeInterval) "
					" VALUES "
					" (%li, %li, %li , %li, '%s', '%s', "
					" %li, %s, %s, %s, %li, "
					" %li, %s, %li, %s, %s) ",
					nStepTemplateID, m_nLadderID, nActionID, nStepOrder, _Q(strName), _Q(strNote), 
					bCreateToDo ? 1 : 0,
					bCreateToDo ? AsString(nToDoAssignTo) : "NULL",
					bCreateToDo ? AsString(nToDoCategoryID) : "NULL",
					bCreateToDo ? AsString(nPriority) : "NULL",
					bIsClientAssigned ? 1:0,
					bIsWebVisible ? 1:0,
					nDocCategoryID == -1 ? "NULL" : AsString(nDocCategoryID),
					bAutoActivate,
					bAutoActivate ? AsString(nTimeLength) : "NULL",
					bAutoActivate ? AsString(nTimeInterval) : "NULL");

				SaveActionItems(nStepTemplateID, nActionID);
			}

		}
		else {

			if (m_nStepID != -1) {

				//we are editing a client step
				ExecuteSql("UPDATE ClientImplementationStepsT SET ActionID = %li, StepName = '%s', Note = '%s', "
				" CreateToDo = %li, ToDoUserID = %s, ToDoCategoryID = %s, ToDoPriority = %s, IsClientAssigned = %li, "
				" IsWebVisible = %li, DocumentCategoryID = %s, AutoActivate = %li, ActivateTimeLength = %s, ActivateTimeInterval = %s  WHERE ID = %li", 
				nActionID, _Q(strName), _Q(strNote), 
				bCreateToDo ? 1 : 0,
				bCreateToDo ? AsString(nToDoAssignTo) : "NULL",
				bCreateToDo ? AsString(nToDoCategoryID) : "NULL",
				bCreateToDo ? AsString(nPriority) : "NULL",
				bIsClientAssigned ? 1:0,
				bIsWebVisible ? 1:0,
				nDocCategoryID == -1 ? "NULL" : AsString(nDocCategoryID), 
				bAutoActivate,
				bAutoActivate ? AsString(nTimeLength) : "NULL",
				bAutoActivate ? AsString(nTimeInterval) : "NULL",
				m_nStepID);

				SaveActionItems(m_nStepID, nActionID);	
			}
			else {
				
				//its a new client step
				//saving a new step template
				long nStepTemplateID = NewNumber("ClientImplementationStepsT", "ID");
				long nStepOrder = NewNumber("ClientImplementationStepsT WHERE LadderID = " + AsString(m_nLadderID), "SortOrder");

				ExecuteSql("INSERT INTO ClientImplementationStepsT (ID, LadderID, ActionID, SortOrder, StepName, Note, "
					" CreateToDo, ToDoUserID, ToDoCategoryID, TodoPriority, IsClientAssigned, "
					" IsWebVisible, DocumentCategoryID, AutoActivate, ActivateTimeLength, ActivateTimeInterval) "
					" VALUES "
					" (%li, %li, %li , %li, '%s', '%s', "
					" %li, %s, %s, %s, %li, "
					" %li, %s, %li, %s, %s) ",
					nStepTemplateID, m_nLadderID, nActionID, nStepOrder, _Q(strName), _Q(strNote), 
					bCreateToDo ? 1 : 0,
					bCreateToDo ? AsString(nToDoAssignTo) : "NULL",
					bCreateToDo ? AsString(nToDoCategoryID) : "NULL",
					bCreateToDo ? AsString(nPriority) : "NULL",
					bIsClientAssigned ? 1:0,
					bIsWebVisible ? 1:0,
					nDocCategoryID == -1 ? "NULL" : AsString(nDocCategoryID), 
					bAutoActivate,
					bAutoActivate ? AsString(nTimeLength) : "NULL",
					bAutoActivate ? AsString(nTimeInterval) : "NULL"
				);

				SaveActionItems(nStepTemplateID, nActionID);
			}



		}
		
		CDialog::OnOK();
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnOK");
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CConfigureImplementationStepDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureImplementationStepDlg)
	ON_EVENT(CConfigureImplementationStepDlg, IDC_IMPLEMENTATION_STEP_ACTION, 16 /* SelChosen */, OnSelChosenImplementationStepAction, VTS_DISPATCH)
	ON_EVENT(CConfigureImplementationStepDlg, IDC_IMPLEMENTATION_ACTION_ITEM_LIST, 16 /* SelChosen */, OnSelChosenImplementationActionItemList, VTS_DISPATCH)
	ON_EVENT(CConfigureImplementationStepDlg, IDC_IMPLEMENTATION_ACTION_ITEM_LIST, 1 /* SelChanging */, OnSelChangingImplementationActionItemList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureImplementationStepDlg, IDC_TIME_INTERVAL, 1 /* SelChanging */, OnSelChangingTimeInterval, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CConfigureImplementationStepDlg::LoadActionItemDatalist(long nActionID) 
{

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		switch (nActionID) {

			case atEmailDocument:
				//they just need to pick documents
				m_nxlActionLabel.ShowWindow(SW_SHOW);
				m_nxlActionLabel.SetText("Choose Document(s)...");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);
			break;

			case atCompleteEMR:
				//they need to pick the template
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				m_nxlActionLabel.SetText("");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);
				m_pActionItemList->FromClause = " (SELECT ID, Name as Description FROM EMRTemplateT) Q ";
				m_pActionItemList->Requery();
				
				pRow = m_pActionItemList->GetNewRow();
				pRow->PutValue(claiID, (long)-1);
				pRow->PutValue(claiDescription, _variant_t("<Multiple>"));
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetFirstRow();
				if (pRow) {
					if (VarString(pRow->GetValue(claiDescription), "") == "<Multiple>") {
						pRow = pRow->GetNextRow();
						if (pRow) {
							m_pActionItemList->CurSel = pRow;
						}
					}
					else {
						m_pActionItemList->CurSel = pRow;
					}
				}
			break;

			case atSelectProcedures:
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				m_nxlActionLabel.SetText("");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);
				/*m_pActionItemList->FromClause = " (SELECT ID, TemplateName as Description FROM ImplementationEMRProceduresT) Q";
				m_pActionItemList->Requery();
									
				pRow = m_pActionItemList->GetNewRow();
				pRow->PutValue(claiID, (long)-1);
				pRow->PutValue(claiDescription, _variant_t("<Multiple>"));
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetFirstRow();
				if (pRow) {
					if (VarString(pRow->GetValue(claiDescription), "") == "<Multiple>") {
						pRow = pRow->GetNextRow();
						if (pRow) {
							m_pActionItemList->CurSel = pRow;
						}
					}
					else {
						m_pActionItemList->CurSel = pRow;
					}
				}*/
			break;

			case atCustomizeTemplates:
				//they just need to pick documents
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				//m_nxlActionLabel.SetText("Choose Templates...");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);

			break;

			case atMergeDocument:
				//they just need to pick documents
				m_nxlActionLabel.ShowWindow(SW_SHOW);
				m_nxlActionLabel.SetText("Choose Document...");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);
			break;

			case atMergePacket:
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				m_nxlActionLabel.SetText("");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);
				m_pActionItemList->FromClause = " (SELECT ID, Name as Description FROM PacketsT WHERE Deleted = 0 and ProcedureRelated  = 0) Q";
				m_pActionItemList->Requery();
				
				pRow = m_pActionItemList->GetNewRow();
				pRow->PutValue(claiID, (long)-1);
				pRow->PutValue(claiDescription, _variant_t("<Multiple>"));
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetFirstRow();
				if (pRow) {
					if (VarString(pRow->GetValue(claiDescription), "") == "<Multiple>") {
						pRow = pRow->GetNextRow();
						if (pRow) {
							m_pActionItemList->CurSel = pRow;
						}
					}
					else {
						m_pActionItemList->CurSel = pRow;
					}
				}
			break;

			case atAptByCategory:{

				m_nxlActionLabel.ShowWindow(SW_HIDE);
				m_nxlActionLabel.SetText("");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 1L;
				pRow->Value[claiDescription] = "Consult";
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 2L;
				pRow->Value[claiDescription] = "PreOp";
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 3L;
				pRow->Value[claiDescription] = "Minor Procedure";
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 4L;
				pRow->Value[claiDescription] = "Surgery";
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 5L;
				pRow->Value[claiDescription] = "Follow-Up";
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetNewRow();
				pRow->Value[claiID] = 6L;
				pRow->Value[claiDescription] = "Other Procedure";
				m_pActionItemList->AddRowSorted(pRow, NULL);

					 }

			break;

			case atAptByType:
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				m_nxlActionLabel.SetText("");
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);
				m_pActionItemList->FromClause = " (SELECT ID, Name as Description FROM AptTypeT WHERE InActive = 0) Q";
				m_pActionItemList->Requery();

				pRow = m_pActionItemList->GetNewRow();
				pRow->PutValue(claiID, (long)-1);
				pRow->PutValue(claiDescription, _variant_t("<Multiple>"));
				m_pActionItemList->AddRowSorted(pRow, NULL);

				pRow = m_pActionItemList->GetFirstRow();
				if (pRow) {
					if (VarString(pRow->GetValue(claiDescription), "") == "<Multiple>") {
						pRow = pRow->GetNextRow();
						if (pRow) {
							m_pActionItemList->CurSel = pRow;
						}
					}
					else {
						m_pActionItemList->CurSel = pRow;
					}
				}
			break;

			case atCustom:
				// (j.gruber 2008-05-22 12:42) - PLID 29445 - hide everything
				m_nxlActionLabel.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);

			break;

			default:
				ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::LoadActionItemDatalist");


}



void CConfigureImplementationStepDlg::OnSelChosenImplementationStepAction(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//first, clear out our arrays
			m_dwActionItemIDs.RemoveAll();
			m_strActionItemPaths.RemoveAll();

			m_pActionItemList->Clear();

			//clear out the existing information
			/*if (m_nStepID != -1) {
				if (m_bIsTemplate) {
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = {INT}", m_nStepID);
				}
				else {
					ExecuteParamSql("DELETE FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", m_nStepID);
				}
			}*/

			long nActionID = VarLong(pRow->GetValue(clacID));

			LoadActionItemDatalist(nActionID);			
		}
		else {
			//kill the link
			m_nxlActionLabel.SetText("");
			m_nxlActionLabel.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnSelChosenImplementationStepAction");
	
}

BOOL CConfigureImplementationStepDlg::SelectMultiPurposes(long nActionID) {

	try {
		CMultiSelectDlg dlg(this, "");
		HRESULT hRes;
		bool bDontFill = false;

		long nResult = 0;

		CString strFrom, strWhere, strIDField, strValueField, strDescription; 

		switch (nActionID) {
			case atCompleteEMR:
				strFrom = "EMRTemplateT";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select the EMR Templates that fulfill this step";
			break;
			case atMergePacket:
				strFrom = "PacketsT";
				strIDField = "ID";
				strWhere = " Deleted = 0 and ProcedureRelated  = 0";
				strValueField = "Name";
				strDescription = "Please select the Packets that fulfill this step";
			break;
			case atAptByType:
				strFrom = "AptTypeT";
				strIDField = "ID";
				strWhere = " Inactive = 0 ";
				strValueField = "Name";
				strDescription = "Please select the Appt. Types that fulfill this step";
			break;

			default:
				return FALSE;
			break;
		}
		
			
		// Fill the dialog with existing selections
		dlg.PreSelect(m_dwActionItemIDs);
		
		dlg.m_strNameColTitle = "Item";

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		dlg.SetSizingConfigRT(strFrom);
		hRes = dlg.Open(strFrom, strWhere, strIDField, strValueField, strDescription, 1);

		//better safe the sorry
		BOOL bReturn = TRUE;	

		// Update our array of procedures with this information
		if (hRes == IDOK) {

			//clear the array
			m_dwActionItemIDs.RemoveAll();

			//clear the data
			/*if (m_nStepID != -1) {
				if (m_bIsTemplate) {
					ExecuteParamSql("DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = {INT}", m_nStepID);
				}
				else {
					ExecuteParamSql("DELETE FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", m_nStepID);
				}
			}*/

			dlg.FillArrayWithIDs(m_dwActionItemIDs);
			//m_strPurposeList = "(" + dlg.GetMultiSelectIDString(",") + ")";
			bReturn = TRUE;

			if(m_dwActionItemIDs.GetSize() > 1) {
				ShowDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST, SW_HIDE);
				CString strNames = dlg.GetMultiSelectString();
				m_nxlActionLabel.SetText(strNames);
				m_nxlActionLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_ACTION_LINK, SW_SHOW);
				m_nxlActionLabel.Invalidate();
			}
			else if(m_dwActionItemIDs.GetSize() == 1) {
				//They selected exactly one.
				ShowDlgItem(IDC_ACTION_LINK, SW_HIDE);
				ShowDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST, SW_SHOW);
				m_pActionItemList->SetSelByColumn(claiID, (long)m_dwActionItemIDs.GetAt(0));
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			bReturn = FALSE;
			//Check if they have "multiple" selected
			if(m_dwActionItemIDs.GetSize() > 1) {
				ShowDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST, SW_HIDE);
				CString strNames = GetActionItemText(nActionID);
				m_nxlActionLabel.SetText(strNames);
				m_nxlActionLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_ACTION_LINK, SW_SHOW);
				InvalidateDlgItem(IDC_ACTION_LINK);
			}
			else {
				ShowDlgItem(IDC_ACTION_LINK, SW_HIDE);
				ShowDlgItem(IDC_IMPLEMENTATION_ACTION_ITEM_LIST, SW_SHOW);
				if (m_dwActionItemIDs.GetSize() > 0) {
					m_pActionItemList->SetSelByColumn(0, (long)m_dwActionItemIDs.GetAt(0));
				}
			}
		}

		return bReturn;
	}NxCatchAll("Error in CConfigureImplementationStepDlg::SelectMultiPurposes");

	return FALSE;
	
}

void CConfigureImplementationStepDlg::OnSelChosenImplementationActionItemList(LPDISPATCH lpRow) 
{
	try { 
		//get the actionID
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pActionList->CurSel;

		if (pRow) {

			long nActionID = VarLong(pRow->GetValue(claID));

			NXDATALIST2Lib::IRowSettingsPtr pRow2(lpRow);

			if (pRow2) {

				long nID = VarLong(pRow2->GetValue(claiID));

				if (nID == -1) {

					//they selected Multiple
					SelectMultiPurposes(nActionID);
				}
				else {

					//clear the current selection
					m_dwActionItemIDs.RemoveAll();

					/*if (m_nStepID != -1) {
						if (m_bIsTemplate) {
							ExecuteParamSql("DELETE FROM ImplementationStepTemplatesCriteriaT WHERE StepTemplateID = {INT}", m_nStepID);
						}
						else {
							ExecuteParamSql("DELETE FROM ClientImplementationStepCriteriaT WHERE StepID = {INT}", m_nStepID);
						}
					}*/

					AddToIDArray(VarLong(nID));
				}
			}
		}
	}NxCatchAll("CConfigureImplementationStepDlg::OnSelChosenImplementationActionItemList");
			

	
}

void CConfigureImplementationStepDlg::OnSelChangingImplementationActionItemList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnSelChangingImplementationActionItemList");

	
}


LRESULT CConfigureImplementationStepDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
		case IDC_ACTION_LINK:
			{

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pActionList->CurSel;

			if (pRow) {
				long nActionID = VarLong(pRow->GetValue(claID));

				switch (nActionID) {

					case atCompleteEMR:
					case atMergePacket:
					case atAptByCategory:
					case atAptByType:
						SelectMultiPurposes(nActionID);
					break;

					case atSelectProcedures:
					case atCustomizeTemplates:			
					case atCustom:
						return 0;
					break;

					case atEmailDocument:
						{
							//pick the documents
							CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, "All Files (*.*)|*.*||");
							// (j.gruber 2007-12-11 12:47) - PLID 28328 - remember the last path chosen
							CString strInitPath = GetRemotePropertyText("ImplementationTabLastFolder", GetSharedPath(), 0, GetCurrentUserName(), TRUE);
							dlg.m_ofn.lpstrInitialDir = strInitPath;
							//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
							char * strFile = new char[5000];
							strFile[0] = 0;
							dlg.m_ofn.nMaxFile = 5000;
							dlg.m_ofn.lpstrFile = strFile;
							CString strLinkText;

							if (dlg.DoModal() == IDOK) {
								m_strActionItemPaths.RemoveAll();
								POSITION p = dlg.GetStartPosition();
								while (p) {
									CString strSelection = SELECTION_FILE;
									CString strFile = dlg.GetNextPathName(p);
									AddToStringArray(strFile);
									strLinkText += strFile + ";";								
								}
								// (j.gruber 2007-12-11 12:56) - PLID 28328 -save the last path accessed
								SetRemotePropertyText("ImplementationTabLastFolder", GetFilePath(strFile), 0, GetCurrentUserName());
								
							}
							delete [] strFile;

							m_nxlActionLabel.SetText(strLinkText);
					
						}
					break;
					case atMergeDocument:
						{
							//pick the documents
							CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "All Files (*.*)|*.*||");
							// (j.gruber 2007-12-11 12:47) - PLID 28328 - remember the last path chosen
							CString strInitPath = GetRemotePropertyText("ImplementationTabLastFolder", GetSharedPath(), 0, GetCurrentUserName(), TRUE);
							dlg.m_ofn.lpstrInitialDir = strInitPath;
							//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
							char * strFile = new char[5000];
							strFile[0] = 0;
							dlg.m_ofn.nMaxFile = 5000;
							dlg.m_ofn.lpstrFile = strFile;
							CString strLinkText;

							if (dlg.DoModal() == IDOK) {
								m_strActionItemPaths.RemoveAll();
								POSITION p = dlg.GetStartPosition();
								while (p) {
									CString strSelection = SELECTION_FILE;
									CString strFile = dlg.GetNextPathName(p);
									AddToStringArray(strFile);
									strLinkText += strFile + ";";								
								}
								// (j.gruber 2007-12-11 12:56) - PLID 28328 -save the last path accessed
								SetRemotePropertyText("ImplementationTabLastFolder", GetFilePath(strFile), 0, GetCurrentUserName());
								
							}
							delete [] strFile;

							m_nxlActionLabel.SetText(strLinkText);
					
						}

					break;	
				
				}

			}
			}
	
		break;

		default:
			ASSERT(FALSE);
		break;
	}

	return 0;
}

BOOL CConfigureImplementationStepDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (m_dwActionItemIDs.GetSize() > 1 || m_strActionItemPaths.GetSize() > 0) 
	{
		CRect rc;
		GetDlgItem(IDC_ACTION_LINK)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CConfigureImplementationStepDlg::OnAutoActivate() 
{
	try {

		if (IsDlgButtonChecked(IDC_AUTO_ACTIVATE)) {

			GetDlgItem(IDC_ACTIVATE_IMMEDIATELY)->EnableWindow(TRUE);
			GetDlgItem(IDC_AFTER_X_TIME)->EnableWindow(TRUE);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(TRUE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(TRUE);
		}
		else {

			CheckDlgButton(IDC_ACTIVATE_IMMEDIATELY, 0);
			CheckDlgButton(IDC_AFTER_X_TIME, 0);
			SetDlgItemText(IDC_TIME_LENGTH, "");
			GetDlgItem(IDC_ACTIVATE_IMMEDIATELY)->EnableWindow(FALSE);
			GetDlgItem(IDC_AFTER_X_TIME)->EnableWindow(FALSE);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(FALSE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnAutoActivate");
	
}

void CConfigureImplementationStepDlg::OnAfterXTime() 
{
	try {
		if (IsDlgButtonChecked(IDC_AFTER_X_TIME)) {
			CheckDlgButton(IDC_ACTIVATE_IMMEDIATELY, 0);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(TRUE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(TRUE);

			NXDATALIST2Lib::IRowSettingsPtr pRow; pRow = m_pTimeIntervalList->GetFirstRow();
			if (pRow) {
				m_pTimeIntervalList->CurSel = pRow;
			}
		}
		else {
			CheckDlgButton(IDC_ACTIVATE_IMMEDIATELY, 1);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(FALSE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnAfterXTime() ");
	
}

void CConfigureImplementationStepDlg::OnActivateImmediately() 
{
	try {

		if (IsDlgButtonChecked(IDC_ACTIVATE_IMMEDIATELY)) {
			CheckDlgButton(IDC_AFTER_X_TIME, 0);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(FALSE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(FALSE);
		}
		else {
			CheckDlgButton(IDC_AFTER_X_TIME, 1);
			GetDlgItem(IDC_TIME_LENGTH)->EnableWindow(TRUE);
			GetDlgItem(IDC_TIME_INTERVAL)->EnableWindow(TRUE);
		}

	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnActivateImmediately() ");
	
}

void CConfigureImplementationStepDlg::OnSelChangingTimeInterval(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnSelChangingTimeInterval");
	
}

void CConfigureImplementationStepDlg::OnStepCreateTodo() 
{
	try {
		if (IsDlgButtonChecked(IDC_STEP_CREATE_TODO)) {

			GetDlgItem(IDC_TODO_USERS_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_TODO_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_TODO_PRIORITY_LIST)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_TODO_USERS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_TODO_CATEGORY_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_TODO_PRIORITY_LIST)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CConfigureImplementationStepDlg::OnStepCreateTodo() ");


	
}
