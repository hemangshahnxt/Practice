// EMREMChecklistSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMChecklistSetupDlg.h"
#include "EMRVisitTypesDlg.h"
#include "EMREMChecklistElementSetupDlg.h"
#include "EMREMChecklistCodeSetupDlg.h"
#include "AuditTrail.h"
#include "EMChecklistAuditHistoryDlg.h"

// (j.jones 2007-08-16 10:51) - PLID 27055 - created

// (c.haag 2007-09-11 15:55) - PLID 27353 - Changed all message boxes
// and modal dialog invocations to use this dialog as their parent rather
// than the main window

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum VisitTypeColumns {

	vtcID = 0,
	vtcName,
};

enum ColumnListColumns { //2007 nominee for dumbest enumeration name of the year

	clcID = 0,
	clcName,
	clcSortOrder,
};

enum CheckListFixedColumns {

	//We have the following fixed columns:
	clfcCPTCode = 0,	//the CPT code column (hidden)
	clfcCodingLevel,	//the coding level column
	clfcCodingLevelTime,	//the coding level time required (// (j.jones 2007-09-17 16:59) - PLID 27396)

	//Everything else is dynamically added
};

#define NO_RULES_SET_DESC "<No Rules Set>"

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistSetupDlg dialog


CEMREMChecklistSetupDlg::CEMREMChecklistSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMChecklistSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMChecklistSetupDlg)
		m_pChecklistInfo = NULL;
		m_bLoadFirstVisitType = FALSE;
		m_bReadOnly = FALSE;
	//}}AFX_DATA_INIT
	// (c.haag 2007-08-30 13:02) - PLID 27058 - This is a modal window by default
	m_bIsModeless = FALSE;
}

CEMREMChecklistSetupDlg::~CEMREMChecklistSetupDlg()
{
	ClearChecklistInfo();
}

void CEMREMChecklistSetupDlg::SetModeless(BOOL bModeless)
{
	// (c.haag 2007-08-30 13:02) - PLID 27058 - Defines whether this window should be created
	// as a modal or a modeless dialog
	if (IsWindow(GetSafeHwnd())) {
		ASSERT(FALSE); // Never call this when the window already exists
	} else {
		m_bIsModeless = bModeless;
	}
}

BOOL CEMREMChecklistSetupDlg::IsModeless() const
{
	// (c.haag 2007-08-30 13:04) - PLID 27058 - Returns TRUE if this dialog should exist as a
	// modeless dialog
	return m_bIsModeless;
}

void CEMREMChecklistSetupDlg::PreSubclassWindow()
{
	// (c.haag 2007-08-30 13:04) - PLID 27058 - If this dialog is to be modeless, we alter the
	// window style here
	if (IsWindow(GetSafeHwnd())) {
		if (IsModeless()) {
			LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);

			// Remove flags we must no longer have
			lStyle &= ~WS_POPUP;

			// Add flags we must have
			lStyle |= WS_OVERLAPPED | WS_MINIMIZEBOX;

			// Set the new style
			SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
		}
	} else {
		ASSERT(FALSE); // We should never get here
	}

	CDialog::PreSubclassWindow();
}

void CEMREMChecklistSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMChecklistSetupDlg)
	DDX_Control(pDX, IDC_BTN_SHOW_AUDIT_HISTORY, m_btnShowAuditHistory);
	DDX_Control(pDX, IDC_BTN_MOVE_COLUMN_DOWN, m_btnMoveColDown);
	DDX_Control(pDX, IDC_BTN_MOVE_COLUMN_UP, m_btnMoveColUp);
	DDX_Control(pDX, IDC_BUTTON_CREATE_DELETE, m_btnCreateDelete);
	DDX_Control(pDX, IDC_BTN_EDIT_VISIT_TYPES, m_btnEditVisitTypes);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_DELETE_COLUMN, m_btnDeleteColumn);
	DDX_Control(pDX, IDC_BTN_ADD_COLUMN, m_btnAddColumn);
	DDX_Control(pDX, IDC_BTN_ADD_CODING_LEVEL, m_btnAddCodingLevel);
	DDX_Control(pDX, IDC_CHECK_SHOW_INACTIVE_VISIT_TYPES_FOR_CHECKLIST, m_checkShowInactiveVisits);
	DDX_Control(pDX, IDC_COLUMN_LIST_LABEL, m_nxstaticColumnListLabel);
	DDX_Control(pDX, IDC_CHECKLIST_SETUP_LABEL, m_nxstaticChecklistSetupLabel);
	DDX_Control(pDX, IDC_CHECKLIST_APPROVAL_LABEL, m_nxstaticChecklistApprovalLabel);
	DDX_Control(pDX, IDC_NO_CHECKLIST_LABEL, m_nxstaticNoChecklistLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMChecklistSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMChecklistSetupDlg)
	ON_BN_CLICKED(IDC_BTN_EDIT_VISIT_TYPES, OnBtnEditVisitTypes)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INACTIVE_VISIT_TYPES_FOR_CHECKLIST, OnCheckShowInactiveVisitTypesForChecklist)
	ON_BN_CLICKED(IDC_BTN_ADD_COLUMN, OnBtnAddColumn)
	ON_BN_CLICKED(IDC_BTN_DELETE_COLUMN, OnBtnDeleteColumn)
	ON_BN_CLICKED(IDC_BTN_ADD_CODING_LEVEL, OnBtnAddCodingLevel)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_DELETE, OnButtonCreateDelete)
	ON_BN_CLICKED(IDC_BTN_MOVE_COLUMN_UP, OnBtnMoveColumnUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_COLUMN_DOWN, OnBtnMoveColumnDown)
	ON_BN_CLICKED(IDC_BTN_SHOW_AUDIT_HISTORY, OnBtnShowAuditHistory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistSetupDlg message handlers

BOOL CEMREMChecklistSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_btnAddColumn.AutoSet(NXB_NEW);
		m_btnDeleteColumn.AutoSet(NXB_DELETE);

		m_btnMoveColUp.SetIcon(IDI_UARROW);
		m_btnMoveColDown.SetIcon(IDI_DARROW);

		// (c.haag 2008-04-29 17:05) - PLID 29837 - NxIconify other buttons
		m_btnEditVisitTypes.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddCodingLevel.AutoSet(NXB_NEW);

		//we do want to load the first visit type upon requery
		m_bLoadFirstVisitType = TRUE;

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_VisitTypeCombo = BindNxDataList2Ctrl(this, IDC_VISIT_TYPES_COMBO, GetRemoteData(), true);
		m_Checklist = BindNxDataList2Ctrl(this, IDC_EM_CHECKLIST_SETUP_LIST, GetRemoteData(), false);
		m_ColumnList = BindNxDataList2Ctrl(this, IDC_EM_CHECKLIST_COLUMN_LIST, GetRemoteData(), false);

		ToggleChecklistControls(FALSE);

		// (j.jones 2007-08-29 10:48) - PLID 27135 - disable checklist controls if it is read-only
		if(m_bReadOnly) {
			SetChecklistControlsReadOnly();
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMREMChecklistSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMREMChecklistSetupDlg)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_VISIT_TYPES_COMBO, 16 /* SelChosen */, OnSelChosenVisitTypesCombo, VTS_DISPATCH)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_EM_CHECKLIST_SETUP_LIST, 19 /* LeftClick */, OnLeftClickEmChecklistSetupList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_VISIT_TYPES_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedVisitTypesCombo, VTS_I2)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_EM_CHECKLIST_SETUP_LIST, 6 /* RButtonDown */, OnRButtonDownEmChecklistSetupList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_EM_CHECKLIST_COLUMN_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmChecklistColumnList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMREMChecklistSetupDlg, IDC_EM_CHECKLIST_COLUMN_LIST, 10 /* EditingFinished */, OnEditingFinishedEmChecklistColumnList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMREMChecklistSetupDlg::OnSelChosenVisitTypesCombo(LPDISPATCH lpRow) 
{
	try {

		Load();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnSelChosenVisitTypesCombo");
}

void CEMREMChecklistSetupDlg::OnBtnEditVisitTypes() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_VisitTypeCombo->CurSel;

		_variant_t value;
		if(pRow) {
			value = pRow->GetValue(vtcID);

			//we do NOT want to load the first visit type upon requery,
			//as we will set it to the current type ourselves
			m_bLoadFirstVisitType = FALSE;
		}
		else {
			//we do want to load the first visit type upon requery
			m_bLoadFirstVisitType = TRUE;
		}

		CEMRVisitTypesDlg dlg(this);
		dlg.DoModal();

		m_VisitTypeCombo->Requery();

		if(pRow)
			m_VisitTypeCombo->SetSelByColumn(vtcID, value);

		Load();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnEditVisitTypes");
}

void CEMREMChecklistSetupDlg::OnCheckShowInactiveVisitTypesForChecklist() 
{
	try {

		if(!m_checkShowInactiveVisits.GetCheck()) {
			//hide inactive items
			m_VisitTypeCombo->WhereClause = _bstr_t("Inactive = 0");
		}
		else {
			//show all items
			m_VisitTypeCombo->WhereClause = _bstr_t("");
		}

		//store the existing selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_VisitTypeCombo->CurSel;

		_variant_t value;
		if(pRow) {
			value = pRow->GetValue(vtcID);

			//we do NOT want to load the first visit type upon requery,
			//as we will set it to the current type ourselves
			m_bLoadFirstVisitType = FALSE;
		}
		else {
			//we do want to load the first visit type upon requery
			m_bLoadFirstVisitType = TRUE;
		}

		m_VisitTypeCombo->Requery();

		if(pRow)
			m_VisitTypeCombo->SetSelByColumn(vtcID, value);

		Load();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnCheckShowInactiveVisitTypesForChecklist");
}

void CEMREMChecklistSetupDlg::Load(long nChecklistID /*= -1*/)
{
	try {

		//first clear out the checklist
		{
			//clear the interface
			m_ColumnList->Clear();
			m_Checklist->Clear();
			for (short nCol = m_Checklist->GetColumnCount() - 1; nCol >= 0; nCol--) {
				m_Checklist->RemoveColumn(nCol);
			}

			//clear out our stored data
			ClearChecklistInfo();
		}

		IRowSettingsPtr pVisitRow = m_VisitTypeCombo->GetCurSel();
		if(pVisitRow == NULL) {
			ToggleChecklistControls(FALSE);
			return;
		}

		long nVisitID = VarLong(pVisitRow->GetValue(vtcID));

		//if a checklist ID was not passed in, load it from data
		if(nChecklistID == -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM EMChecklistsT WHERE VisitTypeID = {INT}", nVisitID);
			if(!rs->eof) {
				nChecklistID = AdoFldLong(rs, "ID",-1);
			}
			rs->Close();
		}

		//show/hide the controls accordingly
		ToggleChecklistControls(nChecklistID != -1);

		//if there is no checklist, do nothing
		if(nChecklistID == -1)
			return;

		//now create a new checklist object
		m_pChecklistInfo = new ChecklistInfo;

		m_pChecklistInfo->nID = nChecklistID;

		//requery the column list
		CString strWhere;
		strWhere.Format("ChecklistID = %li", nChecklistID);
		m_ColumnList->WhereClause = _bstr_t(strWhere);
		m_ColumnList->Requery();

		m_Checklist->SetRedraw(FALSE);

		//add the columns to the checklist
		{
			//add the CPT code column, a hidden column we can sort by
			IColumnSettingsPtr pCPTCodeCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCPTCode, _T("CPTCode"), _T("CPTCode"), 0, csVisible|csFixedWidth));
			pCPTCodeCol->FieldType = cftTextSingleLine;
			pCPTCodeCol->PutSortPriority(0);

			//add the coding level column
			IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevel, _T("CodingLevel"), _T("Coding Level"), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;

			// (j.jones 2007-09-17 17:00) - PLID 27396 - add the coding level time column
			IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevelTime, _T("CodingLevelTime"), _T("Time"), 35, csVisible|csWidthData)))->FieldType = cftTextSingleLine;

			short nColIndex = clfcCodingLevelTime + 1;

			_RecordsetPtr rsColumns = CreateParamRecordset("SELECT ID, Name, OrderIndex FROM EMChecklistColumnsT WHERE ChecklistID = {INT} ORDER BY OrderIndex", nChecklistID);
			while(!rsColumns->eof) {

				long nID = AdoFldLong(rsColumns, "ID");
				CString strName = AdoFldString(rsColumns, "Name");
				long nOrderIndex = AdoFldLong(rsColumns, "OrderIndex");

				//store in our checklist object (using the current nColIndex)
				ChecklistColumnInfo* pInfo = new ChecklistColumnInfo;
				pInfo->nID = nID;
				pInfo->strName = strName;
				pInfo->nOrderIndex = nOrderIndex;
				pInfo->nColumnIndex = nColIndex;
				pInfo->nCheckColumnIndex = -1; //patient EMNs only ((j.jones 2007-08-28 13:14) - PLID 27056)
				pInfo->nBorderColumnIndex = -1; //patient EMNs only (// (j.jones 2007-09-17 15:58) - PLID 27399)
				
				IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex++, _T("Category"), _T(_bstr_t(strName)), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;

				//now that we've created the column, add the column info to our array
				m_pChecklistInfo->paryColumns.Add(pInfo);

				rsColumns->MoveNext();
			}
			rsColumns->Close();
		}

		//add the rows to the checklist
		{
			//load a row for each code, and all of its rules
			_RecordsetPtr rsDetails = CreateParamRecordset("SELECT "
				//coding level information
				"EMChecklistCodingLevelsT.ID AS CodingLevelID, "
				"EMChecklistCodingLevelsT.ServiceID, EMChecklistCodingLevelsT.MinColumns, "
				// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
				"EMChecklistCodingLevelsT.MinimumTime AS CodingLevelMinimumTime, "
				"EMChecklistCodingLevelsT.Description AS CodingLevelDescription, "
				"EMChecklistCodingLevelsT.Approved AS CodingLevelApproved, "
				"EMChecklistCodingLevelsT.ApprovedBy AS CodingLevelApprovedByID, "
				"CodingLevelUsersT.Username AS CodingLevelApprovedByName, "
				"EMChecklistCodingLevelsT.ApprovedDate AS CodingLevelApprovedDate, "
				"CPTCodeT.Code, "
				//rule information
				"EMChecklistRulesT.ID AS RuleID, EMChecklistRulesT.ColumnID AS RuleColumnID, "
				"EMChecklistRulesT.CodingLevelID AS RuleCodingLevelID, EMChecklistRulesT.Description AS RuleDescription, "
				// (j.jones 2007-09-18 14:15) - PLID 27397 - added RequireAllDetails
				"EMChecklistRulesT.RequireAllDetails AS RuleRequireAllDetails, "
				"EMChecklistRulesT.Approved AS RuleApproved, "
				"EMChecklistRulesT.ApprovedBy AS RuleApprovedByID, RuleUsersT.Username AS RuleApprovedByName, "
				"EMChecklistRulesT.ApprovedDate AS RuleApprovedDate, "
				//rule detail information
				"EMChecklistRuleDetailsT.ID AS RuleDetailID, "
				"EMChecklistRuleDetailsT.CategoryID AS RuleDetailCategoryID, "
				"EMChecklistRuleDetailsT.MinElements AS RuleDetailMinElements, "
				"EMCodeCategoryT.Name AS RuleDetailCategoryName "
				""
				"FROM EMChecklistCodingLevelsT "
				"INNER JOIN CPTCodeT ON EMChecklistCodingLevelsT.ServiceID = CPTCodeT.ID "
				"LEFT JOIN UsersT CodingLevelUsersT ON EMChecklistCodingLevelsT.ApprovedBy = CodingLevelUsersT.PersonID "
				"LEFT JOIN EMChecklistRulesT ON EMChecklistCodingLevelsT.ID = EMChecklistRulesT.CodingLevelID "
				"LEFT JOIN UsersT RuleUsersT ON EMChecklistRulesT.ApprovedBy = RuleUsersT.PersonID "
				"LEFT JOIN EMChecklistRuleDetailsT ON EMChecklistRulesT.ID = EMChecklistRuleDetailsT.RuleID "
				"LEFT JOIN EMCodeCategoryT ON EMChecklistRuleDetailsT.CategoryID = EMCodeCategoryT.ID "
				"WHERE EMChecklistCodingLevelsT.ChecklistID = {INT} "
				"ORDER BY CPTCodeT.Code, EMChecklistCodingLevelsT.ID, EMChecklistRulesT.ID, EMChecklistRuleDetailsT.ID",
				nChecklistID);

			ChecklistCodingLevelInfo* pLastCodingLevel = NULL;
			ChecklistElementRuleInfo* pLastRule = NULL;

			while(!rsDetails->eof) {

				//we're looping through a list of all coding levels, rules, and rule details,
				//so we need to determine when we hit a new coding level or new rule,
				//and handle accordingly

				long nCodingLevelID = AdoFldLong(rsDetails, "CodingLevelID", -1);

				if((pLastCodingLevel == NULL || pLastCodingLevel->nID != nCodingLevelID) && nCodingLevelID != -1) {

					//new coding level row			
					long nServiceID = AdoFldLong(rsDetails, "ServiceID");
					CString strCode = AdoFldString(rsDetails, "Code");
					long nMinColumns = AdoFldLong(rsDetails, "MinColumns",1);
					// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
					long nMinimumTime = AdoFldLong(rsDetails, "CodingLevelMinimumTime",0);
					CString strDescription = AdoFldString(rsDetails, "CodingLevelDescription", strCode);
					BOOL bApproved = AdoFldBool(rsDetails, "CodingLevelApproved", FALSE);
					long nApprovalUserID = AdoFldLong(rsDetails, "CodingLevelApprovedByID", -1);
					CString strApprovalUserName = AdoFldString(rsDetails, "CodingLevelApprovedByName", "");
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					COleDateTime dtApproved = AdoFldDateTime(rsDetails, "CodingLevelApprovedDate", dtInvalid);

					IRowSettingsPtr pRow = m_Checklist->GetNewRow();
					pRow->PutValue(clfcCPTCode, _bstr_t(strCode));
					pRow->PutValue(clfcCodingLevel, _bstr_t(strDescription));

					// (j.jones 2007-09-17 17:02) - PLID 27396 - fill the time column
					CString strTime = "N/A";
					if(nMinimumTime > 0)
						strTime.Format("%li", nMinimumTime);
					pRow->PutValue(clfcCodingLevelTime, _bstr_t(strTime));

					// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
					if(!bApproved)
						pRow->PutCellForeColor(clfcCodingLevel, RGB(255,0,0));
					else
						pRow->PutCellForeColor(clfcCodingLevel, RGB(0,0,0));

					//store in our checklist object
					ChecklistCodingLevelInfo* pInfo = new ChecklistCodingLevelInfo;
					pInfo->nID = nCodingLevelID;
					pInfo->nServiceID = nServiceID;
					pInfo->nColumnsRequired = nMinColumns;
					// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
					pInfo->nMinimumTimeRequired = nMinimumTime;
					pInfo->strCodeNumber = strCode;
					pInfo->strDescription = strDescription;
					pInfo->bApproved = bApproved;
					pInfo->dtApproved = dtApproved;
					pInfo->nApprovalUserID = nApprovalUserID;
					pInfo->strApprovalUserName = strApprovalUserName;
					pInfo->pRow = pRow;

					//add to our memory object
					m_pChecklistInfo->paryCodingLevelRows.Add(pInfo);

					//initialize all columns to having no rules, we will change
					//accordingly for cells that have rules later
					for(int i=clfcCodingLevelTime+1;i<m_Checklist->GetColumnCount();i++) {
						pRow->PutValue(i, _bstr_t(NO_RULES_SET_DESC));

						// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red
						pRow->PutCellForeColor(i, RGB(255,0,0));
					}

					//add to the checklist interface
					m_Checklist->AddRowSorted(pRow, NULL);

					//store as our "last" coding level
					pLastCodingLevel = pInfo;
				}

				//load the rules for this row, if any exist

				long nRuleID = AdoFldLong(rsDetails, "RuleID", -1);
				if((pLastRule == NULL || pLastRule->nID != nRuleID) && nRuleID != -1) {

					//new rule

					long nRuleCodingLevelID = AdoFldLong(rsDetails, "RuleCodingLevelID");
					long nRuleColumnID = AdoFldLong(rsDetails, "RuleColumnID");
					CString strDescription = AdoFldString(rsDetails, "RuleDescription", "");
					// (j.jones 2007-09-18 14:45) - PLID 27397 - added RequireAllDetails
					BOOL bRequireAllDetails = AdoFldBool(rsDetails, "RuleRequireAllDetails", TRUE);

					BOOL bApproved = AdoFldBool(rsDetails, "RuleApproved", FALSE);
					long nApprovalUserID = AdoFldLong(rsDetails, "RuleApprovedByID", -1);
					CString strApprovalUserName = AdoFldString(rsDetails, "RuleApprovedByName", "");
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					COleDateTime dtApproved = AdoFldDateTime(rsDetails, "RuleApprovedDate", dtInvalid);

					ChecklistElementRuleInfo *pInfo = new ChecklistElementRuleInfo;
					pInfo->nID = nRuleID;
					pInfo->strDescription = strDescription;
					pInfo->bRequireAllDetails = bRequireAllDetails;
					pInfo->bApproved = bApproved;
					pInfo->dtApproved = dtApproved;
					pInfo->nApprovalUserID = nApprovalUserID;
					pInfo->strApprovalUserName = strApprovalUserName;

					pInfo->bPassed = FALSE; //patient EMNs only ((j.jones 2007-08-29 12:14) - PLID 27056)
					
					//find the column info object by the column ID, and assign to our rule pointer
					{
						ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByColumnID(nRuleColumnID);
						if(pColInfo == NULL) {
							//should not be possible
							ASSERT(FALSE);
							delete pInfo;
							rsDetails->MoveNext();
							continue;
						}
						else {
							pInfo->pColumnInfo = pColInfo;
						}
					}

					//based on the order we are loading data, the current
					//pLastCodingLevel should be the correct row
					{
						if(pLastCodingLevel == NULL || pLastCodingLevel->nID != nRuleCodingLevelID) {
							//should not be possible
							ASSERT(FALSE);
							delete pInfo;
							rsDetails->MoveNext();
							continue;
						}
						else {
							pInfo->pRowInfo = pLastCodingLevel;
						}
					}

					//if we got here, the rule object is created, so we're ready to
					//add it to the checklist memory object
					m_pChecklistInfo->paryRules.Add(pInfo);

					//update the checklist interface with this description
					if(pInfo->pRowInfo->pRow) {
						pInfo->pRowInfo->pRow->PutValue(pInfo->pColumnInfo->nColumnIndex, _bstr_t(pInfo->strDescription));

						// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
						if(!bApproved)
							pInfo->pRowInfo->pRow->PutCellForeColor(pInfo->pColumnInfo->nColumnIndex, RGB(255,0,0));
						else
							pInfo->pRowInfo->pRow->PutCellForeColor(pInfo->pColumnInfo->nColumnIndex, RGB(0,0,0));
					}
					else
						ASSERT(FALSE);

					//store as our "last" rule
					pLastRule = pInfo;
				}

				//load the rule details, if any exist
				long nRuleDetailID = AdoFldLong(rsDetails, "RuleDetailID", -1);
				if(pLastRule != NULL && pLastRule->nID == nRuleID && nRuleID != -1 && nRuleDetailID != -1) {

					//new rule detail
					long nRuleDetailCategoryID = AdoFldLong(rsDetails, "RuleDetailCategoryID");
					long nRuleDetailMinElements = AdoFldLong(rsDetails, "RuleDetailMinElements");
					CString strCategoryName = AdoFldString(rsDetails, "RuleDetailCategoryName", "");

					ChecklistElementRuleDetailInfo* pInfo = new ChecklistElementRuleDetailInfo;
					pInfo->nID = nRuleDetailID;
					pInfo->nMinElements = nRuleDetailMinElements;
					pInfo->nCategoryID = nRuleDetailCategoryID;
					pInfo->strCategoryName = strCategoryName;
					pInfo->bDeleted = FALSE;
					pLastRule->paryDetails.Add(pInfo);
				}

				rsDetails->MoveNext();
			}
			rsDetails->Close();
		}

		//should be in code order, but sort anyways
		m_Checklist->Sort();

		//show or hide the approval label
		CheckShowApprovalWarning();

		//when we get here, the memory structure is populated, the screen is populated
		//(and will be redrawn next), and we are ready to rock and roll

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::Load()");

	m_Checklist->SetRedraw(TRUE);
}

void CEMREMChecklistSetupDlg::ClearChecklistInfo()
{
	try {

		if(m_pChecklistInfo) {
			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			//first clear out all of our rules
			for(i=m_pChecklistInfo->paryRules.GetSize()-1;i>=0;i--) {
				ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

				for(int j=pRuleInfo->paryDetails.GetSize()-1;j>=0;j--) {
					ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(j));
					delete pDetailInfo;
				}
				pRuleInfo->paryDetails.RemoveAll();

				delete pRuleInfo;

				pRuleInfo = NULL;
			}
			m_pChecklistInfo->paryRules.RemoveAll();

			//now clear the columns
			for(i=m_pChecklistInfo->paryColumns.GetSize()-1;i>=0;i--) {
				ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));
				delete pInfo;
			}
			m_pChecklistInfo->paryColumns.RemoveAll();

			//now clear the rows
			for(i=m_pChecklistInfo->paryCodingLevelRows.GetSize()-1;i>=0;i--) {
				ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));
				delete pInfo;
			}
			m_pChecklistInfo->paryCodingLevelRows.RemoveAll();

			delete m_pChecklistInfo;

			m_pChecklistInfo = NULL;
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::ClearChecklistInfo()");
}

void CEMREMChecklistSetupDlg::OnLeftClickEmChecklistSetupList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL)
			return;

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}
		
		if(nCol == clfcCodingLevel) {
			//the "Coding Type" column

			ChecklistCodingLevelInfo *pRowInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
			if(pRowInfo == NULL) {
				//should not be possible
				ASSERT(FALSE);
				return;
			}

			CEMREMChecklistCodeSetupDlg dlg(this);

			dlg.m_pChecklist = m_pChecklistInfo;	//not used for updating, only searching data

			//populate the dialog with our data in memory
			dlg.m_nCodingLevelID = pRowInfo->nID;
			dlg.m_nServiceID = pRowInfo->nServiceID;
			dlg.m_strServiceCode = pRowInfo->strCodeNumber;
			dlg.m_nColumnsRequired = pRowInfo->nColumnsRequired;
			// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
			dlg.m_nMinimumTimeRequired = pRowInfo->nMinimumTimeRequired;
			dlg.m_strDescription = pRowInfo->strDescription;
			dlg.m_bApproved = pRowInfo->bApproved;		
			dlg.m_dtApproved = pRowInfo->dtApproved;
			dlg.m_nApprovalUserID = pRowInfo->nApprovalUserID;
			dlg.m_strApprovalUserName = pRowInfo->strApprovalUserName;
			
			if(dlg.DoModal() == IDOK) {

				//pull the new data from the dialog and put it back into memory
				pRowInfo->nID = dlg.m_nCodingLevelID;
				pRowInfo->nServiceID = dlg.m_nServiceID;
				pRowInfo->strCodeNumber = dlg.m_strServiceCode;
				pRowInfo->nColumnsRequired = dlg.m_nColumnsRequired;
				// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
				pRowInfo->nMinimumTimeRequired = dlg.m_nMinimumTimeRequired;
				pRowInfo->strDescription = dlg.m_strDescription;
				pRowInfo->bApproved = dlg.m_bApproved;
				pRowInfo->dtApproved = dlg.m_dtApproved;
				pRowInfo->nApprovalUserID = dlg.m_nApprovalUserID;
				pRowInfo->strApprovalUserName = dlg.m_strApprovalUserName;

				//update the coding level cells with the new code & description
				pRow->PutValue(clfcCPTCode, _bstr_t(dlg.m_strServiceCode));
				pRow->PutValue(clfcCodingLevel, _bstr_t(dlg.m_strDescription));

				// (j.jones 2007-09-17 17:02) - PLID 27396 - fill the time column
				CString strTime = "N/A";
				if(dlg.m_nMinimumTimeRequired > 0)
					strTime.Format("%li", dlg.m_nMinimumTimeRequired);
				pRow->PutValue(clfcCodingLevelTime, _bstr_t(strTime));	

				// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
				if(!pRowInfo->bApproved)
					pRow->PutCellForeColor(clfcCodingLevel, RGB(255,0,0));
				else
					pRow->PutCellForeColor(clfcCodingLevel, RGB(0,0,0));

				//and sort accordingly
				m_Checklist->Sort();

				//show or hide the approval label
				CheckShowApprovalWarning();
			}
		}
		else if(nCol > clfcCodingLevelTime) {
			//all columns after the coding level time are those that have element rules

			CEMREMChecklistElementSetupDlg dlg(this);

			ChecklistElementRuleInfo* pInfo = NULL;

			dlg.m_pChecklist = m_pChecklistInfo;	//not used for updating, only searching data

			BOOL bCreatedNewRule = FALSE;	//tracks if we had to create a new rule object

			//we need to find the column header and the coding level's service code
			if(m_pChecklistInfo) {

				BOOL bFound = FALSE;

				for(int i=0; i<m_pChecklistInfo->paryRules.GetSize() && !bFound; i++) {

					pInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

					//find the rule that points to our current column and row
					if(pInfo->pColumnInfo->nColumnIndex == nCol && pInfo->pRowInfo->pRow == pRow) {
						//found it!
						bFound = TRUE;

						//now assign this rule to the dlg
						dlg.m_pParentRuleInfo = pInfo;
					}
				}

				if(!bFound) {
					//if not found, it means we don't have any rule for this cell,
					//and have to create a new one

					bCreatedNewRule = TRUE;

					pInfo = new ChecklistElementRuleInfo;
					pInfo->nID = -1;
					pInfo->strDescription = "";
					// (j.jones 2007-09-18 14:48) - PLID 27397 - added RequireAllDetails
					pInfo->bRequireAllDetails = TRUE;

					pInfo->bApproved = FALSE;
					
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					pInfo->dtApproved = dtInvalid;

					pInfo->nApprovalUserID = -1;
					pInfo->strApprovalUserName = "";

					pInfo->bPassed = FALSE; //patient EMNs only ((j.jones 2007-08-29 12:14) - PLID 27056)
					
					//find the column info object by the column index, and assign to our new rule pointer
					{
						ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByColIndex(nCol);
						if(pColInfo == NULL) {
							//should not be possible
							ASSERT(FALSE);
							delete pInfo;
							return;
						}
						else {
							pInfo->pColumnInfo = pColInfo;
						}
					}

					//find the coding level row by the datalist row pointer, and assign to our new rule pointer
					{
						ChecklistCodingLevelInfo *pRowInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
						if(pRowInfo == NULL) {
							//should not be possible
							ASSERT(FALSE);
							delete pInfo;
							return;
						}
						else {
							pInfo->pRowInfo = pRowInfo;
						}
					}

					//if the row is not the first row, see if the previous row
					//has a rule, and if so copy the details only from that
					//previous rule into the new rule, to pre-fill it
					if(TryPopulateRuleFromPriorRow(pInfo)) {

						//tell the dialog we're pre-loading the new rule with
						//previous rule information
						dlg.m_bIsPreLoaded = TRUE;
					}

					//if we got here, the rule object is created, so we're ready to
					//add it to the checklist memory object and the rule edit dialog
					m_pChecklistInfo->paryRules.Add(pInfo);

					//now assign this rule to the dlg
					dlg.m_pParentRuleInfo = pInfo;
				}
			}
			else {
				//we should not have the ability to get this far
				//without a checklist memory object
				ASSERT(FALSE);
				return;
			}

			//now, confirm we have a rule info object in the dialog
			ASSERT(dlg.m_pParentRuleInfo);

			//and open the dialog
			int nRet = dlg.DoModal();
			if(nRet == IDCANCEL && bCreatedNewRule) {

				//the dialog is responsible for updating the rule pointer accordingly
				//if the user clicked OK, but if we just created this rule object
				//and the user cancelled the dialog, delete the rule object

				RemoveOneChecklistElementRule(pInfo);
				//leave this function now, pInfo is now invalid
				return;
			}
			else if(nRet == IDOK) {

				//the dialog will save the rule to data

				//update the checklist cell contents accordingly
				pRow->PutValue(pInfo->pColumnInfo->nColumnIndex, _bstr_t(pInfo->strDescription));

				// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
				if(!pInfo->bApproved)
					pRow->PutCellForeColor(pInfo->pColumnInfo->nColumnIndex, RGB(255,0,0));
				else
					pRow->PutCellForeColor(pInfo->pColumnInfo->nColumnIndex, RGB(0,0,0));

				//show or hide the approval label
				CheckShowApprovalWarning();
			}
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnLeftClickEmChecklistSetupList()");
}

//copies the details and description only from pSourceRule into pDestRule,
//for the purposes of pre-filling data
void CEMREMChecklistSetupDlg::CopyRuleDetails(ChecklistElementRuleInfo *pSourceRule, ChecklistElementRuleInfo *&pDestRule)
{
	try {

		//do not clear out existing details, in fact, they should be blank
		if(pDestRule->paryDetails.GetSize() > 0) {
			ASSERT(FALSE);
			return;
		}

		//now assign the details only of pSourceRule into pDestRule
		pDestRule->strDescription = pSourceRule->strDescription;
		// (j.jones 2007-09-18 14:48) - PLID 27397 - added RequireAllDetails
		pDestRule->bRequireAllDetails = pSourceRule->bRequireAllDetails;
		
		for(int i=0; i<pSourceRule->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pSourceRule->paryDetails.GetAt(i));
			
			//create a new rule detail
			ChecklistElementRuleDetailInfo* pNewInfo = new ChecklistElementRuleDetailInfo;
			pNewInfo->nID = -1;
			pNewInfo->nMinElements = pDetailInfo->nMinElements;
			pNewInfo->nCategoryID = pDetailInfo->nCategoryID;
			pNewInfo->strCategoryName = pDetailInfo->strCategoryName;
			pNewInfo->bDeleted = pDetailInfo->bDeleted;
			pDestRule->paryDetails.Add(pNewInfo);
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::CopyRuleDetails()");
}

//used when creating a new rule, this function will try to auto-populate the rule
//contents with data from a previous rule in the same column
BOOL CEMREMChecklistSetupDlg::TryPopulateRuleFromPriorRow(ChecklistElementRuleInfo *pInfo)
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return FALSE;
		}

		IRowSettingsPtr pPrevRow = pInfo->pRowInfo->pRow->GetPreviousRow();

		//we're going to loop through all prior rows until we find a rule to copy from
		while(pPrevRow != NULL) {
			//we have a previous row, does it have a rule in the same column?

			for(int i=0; i<m_pChecklistInfo->paryRules.GetSize(); i++) {

				ChecklistElementRuleInfo* pRuleToCheck = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
				if(pRuleToCheck->pRowInfo != NULL && pRuleToCheck->pRowInfo->pRow == pPrevRow
					&& pRuleToCheck->pColumnInfo == pInfo->pColumnInfo) {

					//copy the details from this rule into our new rule
					CopyRuleDetails(pRuleToCheck, pInfo);

					return TRUE;
				}
			}

			//if we didn't find a rule (we wouldn't have if we're still in this function),
			//we're going to look at the next previous row
			pPrevRow = pPrevRow->GetPreviousRow();
		}

		//if we didn't find a rule to copy from, no big deal, it just
		//means they have to fill the contents of the new rule from scratch

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::TryPopulateRuleFromPriorRow");

	return FALSE;
}

//given a column index, find and return the matching ChecklistColumnInfo object
ChecklistColumnInfo* CEMREMChecklistSetupDlg::FindColumnInfoObjectByColIndex(short nCol)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize(); i++) {

		ChecklistColumnInfo* pColInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));

		if(pColInfo->nColumnIndex == nCol) {
			//found it, so return this pointer
			return pColInfo;			
		}
	}

	//it should not be possible to not find an object here
	ASSERT(FALSE);
	return NULL;
}

//given a column ID, find and return the matching ChecklistColumnInfo object
ChecklistColumnInfo* CEMREMChecklistSetupDlg::FindColumnInfoObjectByColumnID(long nID)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize(); i++) {

		ChecklistColumnInfo* pColInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));

		if(pColInfo->nID == nID) {
			//found it, so return this pointer
			return pColInfo;		
		}
	}

	//it should not be possible to not find an object here
	ASSERT(FALSE);
	return NULL;
}

//given a datalist row, find and return the matching ChecklistCodingLevelInfo object
ChecklistCodingLevelInfo* CEMREMChecklistSetupDlg::FindCodingLevelInfoObjectByRowPtr(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	for(int i=0; i<m_pChecklistInfo->paryCodingLevelRows.GetSize(); i++) {

		ChecklistCodingLevelInfo* pRowInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));

		if(pRowInfo->pRow == pRow) {
			//found it, so return this pointer
			return pRowInfo;
		}
	}

	//it should not be possible to not find an object here
	ASSERT(FALSE);
	return NULL;
}

//removes one checklist rule, and its details, from memory
void CEMREMChecklistSetupDlg::RemoveOneChecklistElementRule(ChecklistElementRuleInfo *pInfo)
{
	if(pInfo == NULL)
		//well that just doesn't even make sense
		return;

	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return;
	}

	for(int i=m_pChecklistInfo->paryRules.GetSize()-1;i>=0;i--) {
		ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

		//we're only deleting one element
		if(pRuleInfo == pInfo) {

			for(int j=pRuleInfo->paryDetails.GetSize()-1;j>=0;j--) {
				ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(j));
				delete pDetailInfo;
			}
			pRuleInfo->paryDetails.RemoveAll();

			delete pRuleInfo;

			pRuleInfo = NULL;

			m_pChecklistInfo->paryRules.RemoveAt(i);

			//our work here is done
			return;
		}
	}
}

void CEMREMChecklistSetupDlg::OnRequeryFinishedVisitTypesCombo(short nFlags) 
{
	try {

		//if true, load the first visit type
		if(m_bLoadFirstVisitType) {

			IRowSettingsPtr pRow = m_VisitTypeCombo->GetFirstRow();
			if(pRow) {
				m_VisitTypeCombo->CurSel = pRow;
				OnSelChosenVisitTypesCombo(pRow);
			}

			//and now disable this setting
			m_bLoadFirstVisitType = FALSE;
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnRequeryFinishedVisitTypesCombo");
}

void CEMREMChecklistSetupDlg::OnRButtonDownEmChecklistSetupList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	long nAuditTransactionID = -1;

	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		CMenu mnu;
		if (mnu.CreatePopupMenu()) {
			
			mnu.InsertMenu(-1, MF_BYPOSITION, 1, "&Delete Coding Level");

			//cached so we know what rule to remove later
			ChecklistElementRuleInfo* pRuleInfo = NULL;
			int nRuleIndex = -1;

			//if a dynamic column, see if there is a rule that exists
			if(nCol > clfcCodingLevelTime) {

				ChecklistElementRuleInfo* pInfo = NULL;

				//we need to find the column header and the coding level's service code

				for(int i=0; i<m_pChecklistInfo->paryRules.GetSize() && nRuleIndex == -1; i++) {

					pInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

					//find the rule that points to our current column and row
					if(pInfo->pColumnInfo->nColumnIndex == nCol && pInfo->pRowInfo->pRow == pRow) {
						//found it! cache these values
						nRuleIndex = i;
						pRuleInfo = pInfo;

						//and give the option to delete
						mnu.InsertMenu(-1, MF_BYPOSITION, 2, "&Delete Rule");
					}
				}
			}

			CPoint pt;
			GetCursorPos(&pt);
			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			if (nResult == 1) {
				//delete the coding level

				//first find the CPT code
				CString strCode = "";
				long nCodingLevelID = -1;
				ChecklistCodingLevelInfo *pRowInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
				if(pRowInfo == NULL) {
					//should not be possible
					ASSERT(FALSE);
					return;
				}
				else {
					strCode = pRowInfo->strCodeNumber;
					nCodingLevelID = pRowInfo->nID;
				}

				//now warn
				CString strWarning1, strWarning2;
				// (j.jones 2013-01-16 11:39) - PLID 54634 - added a warning about in-use checklists on EMNs
				strWarning1.Format("Removing the %s coding level will also delete any rules you have set up on this level.\n\n"
					"In addition, any progress you may have saved for this coding level on any patient EMN will be lost, including manually approved rules.\n\n"
					"Are you sure you wish to delete the %s coding level?", strCode, strCode);
				strWarning2.Format("Are you SURE? This action is not recoverable!");
				if(IDYES == MessageBox(strWarning1, "Practice", MB_ICONEXCLAMATION|MB_YESNO)
					&& IDYES == MessageBox(strWarning2, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

					CWaitCursor pWait;

					//ok, delete the row, which also means deleting all rules for that row

					// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized and simplified this batch
					CString strSqlBatch;
					CNxParamSqlArray aryParams;
					nAuditTransactionID = BeginAuditTransaction();

					// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
					int i = 0;

					for(i=m_pChecklistInfo->paryRules.GetSize()-1;i>=0;i--) {
						ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

						//we're only deleting rules linked to this row
						if(pRuleInfo->pRowInfo->pRow == pRow) {

							for(int j=pRuleInfo->paryDetails.GetSize()-1;j>=0;j--) {
								ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(j));

								//audit the detail
								if(pDetailInfo != NULL) {

									if(pDetailInfo->nID != -1) {
										CString strOldValue;
										strOldValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);
										AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
									}

									delete pDetailInfo;
								}
							}
							pRuleInfo->paryDetails.RemoveAll();


							//now audit the rule
							if(pRuleInfo->nID != -1) {
								//replace newlines with spaces
								CString strOldValue = pRuleInfo->strDescription;
								strOldValue.Replace("\r\n", "  ");

								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
							}

							delete pRuleInfo;

							pRuleInfo = NULL;

							m_pChecklistInfo->paryRules.RemoveAt(i);
						}
					}

					//now clear the row
					for(i=m_pChecklistInfo->paryCodingLevelRows.GetSize()-1;i>=0;i--) {
						ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));
						
						if(pInfo == pRowInfo) {

							//audit the coding level
							if(pInfo->nID != -1) {
								//replace newlines with spaces
								CString strOldValue = pInfo->strDescription;
								strOldValue.Replace("\r\n", "  ");

								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
							}
						
							delete pInfo;

							m_pChecklistInfo->paryCodingLevelRows.RemoveAt(i);

							//and now remove the row
							m_Checklist->RemoveRow(pRow);
						}
					}

					// (j.jones 2013-01-16 10:52) - PLID 54634 - moved all SQL deletion code here and parameterized, as we
					// did not need to delete in the above loops, those were only needed for auditing
					if(nCodingLevelID != -1) {
						// (j.jones 2013-01-16 10:46) - PLID 54634 - clear progress on EMNs too (this is not patient content, remember it's just a worksheet)
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistRuleProgressT WHERE EMChecklistRuleID IN (SELECT ID FROM EMChecklistRulesT WHERE CodingLevelID = {INT})", nCodingLevelID);
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistCodingLevelProgressT WHERE EMChecklistCodingLevelID = {INT}", nCodingLevelID);

						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRuleDetailsT WHERE RuleID IN (SELECT ID FROM EMChecklistRulesT WHERE CodingLevelID = {INT})", nCodingLevelID);
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRulesT WHERE CodingLevelID = {INT}", nCodingLevelID);
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistCodingLevelsT WHERE ID = {INT}", nCodingLevelID);
					}

					//commit our changes
					// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized this batch
					if(!strSqlBatch.IsEmpty()) {
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
					}

					if(nAuditTransactionID != -1)
						CommitAuditTransaction(nAuditTransactionID);

					//show or hide the approval label
					CheckShowApprovalWarning();

					//our work here is done
					CString strConfirm;
					strConfirm.Format("The %s coding level has been deleted.", strCode);
					MessageBox(strConfirm, "Practice", MB_OK | MB_ICONINFORMATION);
					return;
				}
			}
			else if(nResult == 2 && pRuleInfo != NULL && nRuleIndex != -1) {

				//delete the rule
				
				CString strWarning;
				// (j.jones 2013-01-16 11:39) - PLID 54634 - added a warning about in-use checklists on EMNs
				strWarning.Format("Deleting a rule will also clear any manual approvals you may have saved for this rule on any patient EMN.\n\n"
					"Are you sure you wish to delete the following rule?\n\n"
					"%s", pRuleInfo->strDescription);
				if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					return;
				}

				//delete from data if it exists
				if(pRuleInfo->nID != -1) {

					// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized and simplified this batch
					CString strSqlBatch;
					CNxParamSqlArray aryParams;
					nAuditTransactionID = BeginAuditTransaction();

					//delete details
					for(int i=pRuleInfo->paryDetails.GetSize()-1;i>=0;i--) {

						ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(i));
						if(pDetailInfo != NULL) {

							if(pDetailInfo->nID != -1) {
								CString strOldValue;
								strOldValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);
								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
							}

							//and delete the detail object
							delete pDetailInfo;
						}
					}
					pRuleInfo->paryDetails.RemoveAll();

					//and now delete the rule

					// (j.jones 2013-01-16 10:46) - PLID 54634 - clear progress on EMNs too (this is not patient content, remember it's just a worksheet)
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistRuleProgressT WHERE EMChecklistRuleID = {INT}", pRuleInfo->nID);

					// (j.jones 2013-01-16 10:52) - PLID 54634 - moved all SQL deletion code here and parameterized, as we
					// did not need to delete in the above loops, those were only needed for auditing
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRuleDetailsT WHERE RuleID = {INT}", pRuleInfo->nID);
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRulesT WHERE ID = {INT}", pRuleInfo->nID);

					//audit

					//replace newlines with spaces
					CString strOldValue = pRuleInfo->strDescription;
					strOldValue.Replace("\r\n", "  ");

					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);

					//commit our changes
					// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized this batch
					if(!strSqlBatch.IsEmpty()) {
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
					}

					if(nAuditTransactionID != -1)
						CommitAuditTransaction(nAuditTransactionID);
				}

				//delete from memory
				delete pRuleInfo;
				m_pChecklistInfo->paryRules.RemoveAt(nRuleIndex); //see, this is why we cached the index!

				//and update the interface
				pRow->PutValue(nCol, _bstr_t(NO_RULES_SET_DESC));

				// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red
				pRow->PutCellForeColor(nCol, RGB(255,0,0));

				//show or hide the approval label
				CheckShowApprovalWarning();
			}
		}

		return;

	}NxCatchAllCall("Error in CEMREMChecklistSetupDlg::OnRButtonDownEmChecklistSetupList",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

void CEMREMChecklistSetupDlg::OnBtnAddColumn() 
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//get the name
		CString strItem;
		BOOL bValid = FALSE;
		while(!bValid) {
			if (InputBoxLimitedWithParent(this, "Enter a new checklist column name:", strItem, "",255,false,false,NULL) == IDOK) {
				strItem.TrimLeft(); strItem.TrimRight();

				//assume it's fine
				bValid = TRUE;

				if(strItem.IsEmpty()) {
					MessageBox("You may not have a blank column name.", "Practice", MB_OK | MB_ICONERROR);
					bValid = FALSE;
					continue;
				}			

				//does any current column use this name?
				IRowSettingsPtr pRowToCheck = m_ColumnList->GetFirstRow();
				while(pRowToCheck != NULL) {

					CString strNameToCompare = VarString(pRowToCheck->GetValue(clcName), "");
					if(strNameToCompare.CompareNoCase(strItem) == 0) {
						//another column uses this name
						MessageBox("There is already a checklist column using this name. Please enter a new name.", "Practice", MB_OK | MB_ICONERROR);
						bValid = FALSE;
						break;
					}

					pRowToCheck = pRowToCheck->GetNextRow();
				}
			}
			else {
				return;
			}
		}

		//create the column in data

		long nID = NewNumber("EMChecklistColumnsT", "ID");;
		long nOrderIndex = m_ColumnList->GetRowCount() + 1;

		ExecuteSql("INSERT INTO EMChecklistColumnsT (ID, ChecklistID, Name, OrderIndex) VALUES (%li, %li, '%s', %li)", nID, m_pChecklistInfo->nID, _Q(strItem), nOrderIndex);

		//audit the creation
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMChecklistColumnCreated, m_pChecklistInfo->nID, "", strItem, aepMedium, aetCreated);

		//create a row in the column list
		IRowSettingsPtr pRow = m_ColumnList->GetNewRow();
		pRow->PutValue(clcID, nID);
		pRow->PutValue(clcName, _bstr_t(strItem));
		pRow->PutValue(clcSortOrder, (long)nOrderIndex);
		m_ColumnList->AddRowSorted(pRow, NULL);

		//create it in our memory object and the checklist interface

		short nColIndex = m_Checklist->GetColumnCount();

		ChecklistColumnInfo* pInfo = new ChecklistColumnInfo;
		pInfo->nID = nID;
		pInfo->strName = strItem;
		pInfo->nOrderIndex = nOrderIndex;		
		pInfo->nColumnIndex = nColIndex;
		pInfo->nCheckColumnIndex = -1; //patient EMNs only ((j.jones 2007-08-28 13:14) - PLID 27056)
		pInfo->nBorderColumnIndex = -1; //patient EMNs only (// (j.jones 2007-09-17 15:58) - PLID 27399)
		
		IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex, _T("Category"), _T(_bstr_t(strItem)), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;

		long nOldMaxColumns = m_pChecklistInfo->paryColumns.GetSize();

		//now that we've created the column, add the column info to our array
		m_pChecklistInfo->paryColumns.Add(pInfo);

		//before we are done, we have to update each row with "no rules set",
		//but to do that we have to rebuild the rows from memory such that
		//they gain the new column information
		RebuildChecklistRows();

		//we don't need update each row with the "no rules set" description,
		//because RebuildChecklistRows() will have already done so

		//now, check for any coding levels that previously required all the columns filled

		CString strServiceIDs;

		for(int i=0; i<m_pChecklistInfo->paryCodingLevelRows.GetSize(); i++) {
			ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));

			if(pInfo->nColumnsRequired == nOldMaxColumns) {

				//track this ID
				if(!strServiceIDs.IsEmpty())
					strServiceIDs += "\n";
				strServiceIDs += pInfo->strCodeNumber;
			}
		}

		//warn, instead of auto-updating, because if we auto-updated, they would have
		//to re-approve those coding levels anyways
		if(!strServiceIDs.IsEmpty()) {

			CString str;
			str.Format("The following coding levels require %li columns completed, which was previously the total column count.\n\n"
				"%s\n\n"
				"You may wish to re-visit these coding levels and determine if they should be changed to require %li columns.",
				nOldMaxColumns, strServiceIDs, nOldMaxColumns+1);
			MessageBox(str, "Practice", MB_OK | MB_ICONINFORMATION);
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnAddColumn");
}

void CEMREMChecklistSetupDlg::OnBtnDeleteColumn() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		IRowSettingsPtr pRow = m_ColumnList->CurSel;
		if(pRow == NULL) {
			MessageBox("You must first select an entry from the column list before deleting.", "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		long nColumnID = VarLong(pRow->GetValue(clcID),-1);
		CString strColumnName = VarString(pRow->GetValue(clcName),"");
		long nSortOrder = VarLong(pRow->GetValue(clcSortOrder),-1);

		//warn heavily
		//now warn
		CString strWarning1, strWarning2;
		// (j.jones 2013-01-16 11:39) - PLID 54634 - added a warning about in-use checklists on EMNs
		strWarning1.Format("Removing the %s column will also delete any rules you have set up on this column.\n\n"
			"In addition, any progress you may have saved for this column on any patient EMN will be lost, including manually approved rules.\n\n"
			"Are you sure you wish to delete the %s column?", strColumnName, strColumnName);
		strWarning2.Format("Are you SURE? This action is not recoverable!");
		if(IDYES == MessageBox(strWarning1, "Practice", MB_ICONEXCLAMATION|MB_YESNO)
			&& IDYES == MessageBox(strWarning2, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

			CWaitCursor pWait;

			//ok, delete the column, which also means deleting all rules for that column

			ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByColumnID(nColumnID);
			if(pColInfo == NULL) {
				//should not be possible
				ASSERT(FALSE);

				//remove from the column list
				m_ColumnList->RemoveRow(pRow);
				return;
			}

			long nColumnID = pColInfo->nID;

			// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized and simplified this batch
			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			nAuditTransactionID = BeginAuditTransaction();

			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			for(i=m_pChecklistInfo->paryRules.GetSize()-1;i>=0;i--) {
				ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

				//we're only deleting rules linked to this column
				if(pRuleInfo->pColumnInfo == pColInfo) {

					for(int j=pRuleInfo->paryDetails.GetSize()-1;j>=0;j--) {
						ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(j));

						//audit the detail
						if(pDetailInfo != NULL) {

							if(pDetailInfo->nID != -1) {
								CString strOldValue;
								strOldValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);
								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
							}

							delete pDetailInfo;
						}
					}
					pRuleInfo->paryDetails.RemoveAll();


					//now audit the rule
					if(pRuleInfo->nID != -1) {
						//replace newlines with spaces
						CString strOldValue = pRuleInfo->strDescription;
						strOldValue.Replace("\r\n", "  ");

						AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
					}

					delete pRuleInfo;

					pRuleInfo = NULL;

					m_pChecklistInfo->paryRules.RemoveAt(i);
				}
			}

			// (j.jones 2013-01-16 10:52) - PLID 54634 - moved all SQL deletion code here and parameterized, as we
			// did not need to delete in the above loops, those were only needed for auditing
			if(nColumnID != -1) {
				// (j.jones 2013-01-16 10:46) - PLID 54634 - clear progress on EMNs too (this is not patient content, remember it's just a worksheet)
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistRuleProgressT WHERE EMChecklistRuleID IN (SELECT ID FROM EMChecklistRulesT WHERE ColumnID = {INT})", nColumnID);

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRuleDetailsT WHERE RuleID IN (SELECT ID FROM EMChecklistRulesT WHERE ColumnID = {INT})", nColumnID);
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRulesT WHERE ColumnID = {INT}", nColumnID);
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistColumnsT WHERE ID = {INT}", nColumnID);
			}

			//now clear the column, and update the sort orders of other columns
			for(i=m_pChecklistInfo->paryColumns.GetSize()-1;i>=0;i--) {
				ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));
				
				if(pInfo == pColInfo) {

					//audit the column
					if(pInfo->nID != -1) {
						AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistColumnDeleted, m_pChecklistInfo->nID, pInfo->strName, "Deleted", aepMedium, aetDeleted);
					}
				
					delete pInfo;

					m_pChecklistInfo->paryColumns.RemoveAt(i);

					//we will rebuild the columns later
				}
				else if(pInfo->nOrderIndex > pColInfo->nOrderIndex) {

					//decrement the sort order and save, but don't bother auditing that the order changed,
					//when technically it didn't

					pInfo->nOrderIndex--;
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EMChecklistColumnsT SET OrderIndex = {INT} WHERE ID = {INT}", pInfo->nOrderIndex, pInfo->nID);

					//the next loop will handle updating the column datalist
				}
			}

			//update the new order index in remaining datalist column entries
			IRowSettingsPtr pNextRow = pRow->GetNextRow();
			while(pNextRow) {

				long nOrderIndex = VarLong(pNextRow->GetValue(clcSortOrder));
				nOrderIndex--;
				pNextRow->PutValue(clcSortOrder, nOrderIndex);

				pNextRow = pNextRow->GetNextRow();
			}

			//check for any coding levels that require more columns filled than we have in the list,
			//note that we will decrement them
			long nCodingLevelsChanged = 0;

			for(i=0; i<m_pChecklistInfo->paryCodingLevelRows.GetSize(); i++) {
				ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));

				if(pInfo->nColumnsRequired > m_pChecklistInfo->paryColumns.GetSize()) {

					//this row needs updated
					nCodingLevelsChanged++;

					long nOldColumnsRequired = pInfo->nColumnsRequired;
					long nNewColumnsRequired = m_pChecklistInfo->paryColumns.GetSize();

					//we do not need to disapprove an approved coding level,
					//we're simply normalizing the data

					//update the memory object
					pInfo->nColumnsRequired = nNewColumnsRequired;

					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EMChecklistCodingLevelsT "
						"SET MinColumns = {INT} WHERE ID = {INT}", nNewColumnsRequired, pInfo->nID);

					//audit

					//replace newlines with spaces
					CString strDesc = pInfo->strDescription;
					strDesc.Replace("\r\n", "  ");

					CString strOldValue, strNewValue;
					strOldValue.Format("%li columns (%s)", nOldColumnsRequired, strDesc);
					strNewValue.Format("%li columns (%s)", nNewColumnsRequired, strDesc);

					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelMinColumns, m_pChecklistInfo->nID, strOldValue, strNewValue, aepMedium, aetChanged);
				}
			}

			//commit our changes
			// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized this batch
			if(!strSqlBatch.IsEmpty()) {
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			}

			if(nAuditTransactionID != -1)
				CommitAuditTransaction(nAuditTransactionID);

			//remove from the column list
			m_ColumnList->RemoveRow(pRow);

			//rebuild the checklist
			RebuildChecklistColumns();

			//show or hide the approval label
			CheckShowApprovalWarning();

			//our work here is done
			CString strConfirm;
			if(nCodingLevelsChanged == 0)
				strConfirm.Format("The %s column has been deleted.", strColumnName);
			else 
				strConfirm.Format("The %s column has been deleted.\n"
					"%li Coding Levels had their 'required columns' value reduced to match the new column count.", strColumnName, nCodingLevelsChanged);
			MessageBox(strConfirm, "Practice", MB_OK | MB_ICONINFORMATION);

			return;
		}

	}NxCatchAllCall("Error in CEMREMChecklistSetupDlg::OnBtnDeleteColumn",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

void CEMREMChecklistSetupDlg::OnEditingFinishingEmChecklistColumnList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(nCol == clcName) {

			//warn if blank
			CString strEntered = strUserEntered;
			strEntered.TrimLeft();
			strEntered.TrimRight();

			if(strEntered.IsEmpty()) {
				MessageBox("You may not have a blank column name.", "Practice", MB_OK | MB_ICONERROR);
				*pbCommit = FALSE;
				return;
			}

			//warn if the new name is already in use

			//does any current column use this name?
			IRowSettingsPtr pRowToCheck = m_ColumnList->GetFirstRow();
			while(pRowToCheck != NULL) {

				if(pRowToCheck != pRow) {

					CString strNameToCompare = VarString(pRowToCheck->GetValue(clcName), "");
					if(strNameToCompare.CompareNoCase(strUserEntered) == 0) {
						//another column uses this name
						MessageBox("There is already a checklist column using this name. Please enter a new name.", "Practice", MB_OK | MB_ICONERROR);
						*pbCommit = FALSE;
						return;
					}
				}

				pRowToCheck = pRowToCheck->GetNextRow();
			}
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnEditingFinishingEmChecklistColumnList");
}

void CEMREMChecklistSetupDlg::OnEditingFinishedEmChecklistColumnList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL || !bCommit)
			return;

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(nCol == clcName) {

			//find the column object
			ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByColumnID(VarLong(pRow->GetValue(clcID)));
			if(pColInfo == NULL) {
				//should not be possible
				ASSERT(FALSE);

				//remove from the column list
				m_ColumnList->RemoveRow(pRow);
				return;
			}

			CString strOldName = pColInfo->strName;
			CString strNewName = VarString(varNewValue);

			if(strOldName != strNewName) {

				ExecuteSql("UPDATE EMChecklistColumnsT SET Name = '%s' WHERE ID = %li", _Q(strNewName), pColInfo->nID);

				//don't forget to audit
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiEMChecklistColumnName, m_pChecklistInfo->nID, strOldName, strNewName, aepMedium, aetCreated);

				//update the memory object
				pColInfo->strName = strNewName;

				//update the checklist interface
				IColumnSettingsPtr pCol = m_Checklist->GetColumn(pColInfo->nColumnIndex);
				pCol->ColumnTitle = _bstr_t(strNewName);
			}
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnEditingFinishedEmChecklistColumnList");
}

void CEMREMChecklistSetupDlg::OnBtnAddCodingLevel() 
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//do not allow adding a coding level until at least one column exists
		if(m_ColumnList->GetRowCount() == 0) {
			MessageBox("You may not enter a new coding level without at least one checklist column.\n"
				"It is recommended that you create all of your checklist columns prior to creating coding levels.", "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		//prompt for a coding level		
		CEMREMChecklistCodeSetupDlg dlg(this); //this dialog is responsible for disallowing duplicate service codes
		
		dlg.m_pChecklist = m_pChecklistInfo;	//not used for updating, only searching data
		dlg.m_nColumnsRequired = m_pChecklistInfo->paryColumns.GetSize();	//default to the total column count

		if(dlg.DoModal() == IDCANCEL)
			return;

		//this dialog is NOT responsible for updating m_pChecklistInfo,
		//we have to update it ourselves, as it includes a datalist row pointer

		//create a new memory object and add the row to the checklist interface

		IRowSettingsPtr pRow = m_Checklist->GetNewRow();
		pRow->PutValue(clfcCPTCode, _bstr_t(dlg.m_strServiceCode));
		pRow->PutValue(clfcCodingLevel, _bstr_t(dlg.m_strDescription));

		// (j.jones 2007-09-17 17:02) - PLID 27396 - fill the time column
		CString strTime = "N/A";
		if(dlg.m_nMinimumTimeRequired > 0)
			strTime.Format("%li", dlg.m_nMinimumTimeRequired);
		pRow->PutValue(clfcCodingLevelTime, _bstr_t(strTime));	

		// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
		if(!dlg.m_bApproved)
			pRow->PutCellForeColor(clfcCodingLevel, RGB(255,0,0));
		else
			pRow->PutCellForeColor(clfcCodingLevel, RGB(0,0,0));

		//loop through each column
		for(int i=clfcCodingLevelTime+1;i<m_Checklist->GetColumnCount();i++) {
			pRow->PutValue(i, _bstr_t(NO_RULES_SET_DESC));

			// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red
			pRow->PutCellForeColor(i, RGB(255,0,0));
		}

		m_Checklist->AddRowSorted(pRow, NULL);

		//store in our checklist object
		ChecklistCodingLevelInfo* pInfo = new ChecklistCodingLevelInfo;
		pInfo->nID = dlg.m_nCodingLevelID;
		pInfo->nServiceID = dlg.m_nServiceID;
		pInfo->nColumnsRequired = dlg.m_nColumnsRequired;
		// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
		pInfo->nMinimumTimeRequired = dlg.m_nMinimumTimeRequired;
		pInfo->strCodeNumber = dlg.m_strServiceCode;
		pInfo->strDescription = dlg.m_strDescription;
		pInfo->bApproved = dlg.m_bApproved;
		pInfo->dtApproved = dlg.m_dtApproved;
		pInfo->nApprovalUserID = dlg.m_nApprovalUserID;
		pInfo->strApprovalUserName = dlg.m_strApprovalUserName;
		pInfo->pRow = pRow;

		m_pChecklistInfo->paryCodingLevelRows.Add(pInfo);

		//show or hide the approval label
		CheckShowApprovalWarning();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnAddCodingLevel");
}

//will remove all rows, and rebuild them from memory,
//properly storing their pointers in any coding levels that reference the rows
void CEMREMChecklistSetupDlg::RebuildChecklistRows()
{
	try {

		//remove all datalist rows
		m_Checklist->Clear();

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		for(int i=0; i<m_pChecklistInfo->paryCodingLevelRows.GetSize(); i++) {

			ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));

			IRowSettingsPtr pRow = m_Checklist->GetNewRow();
			pRow->PutValue(clfcCPTCode, _bstr_t(pInfo->strCodeNumber));
			pRow->PutValue(clfcCodingLevel, _bstr_t(pInfo->strDescription));

			// (j.jones 2007-09-17 17:02) - PLID 27396 - fill the time column
			CString strTime = "N/A";
			if(pInfo->nMinimumTimeRequired > 0)
				strTime.Format("%li", pInfo->nMinimumTimeRequired);
			pRow->PutValue(clfcCodingLevelTime, _bstr_t(strTime));

			// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
			if(!pInfo->bApproved)
				pRow->PutCellForeColor(clfcCodingLevel, RGB(255,0,0));
			else
				pRow->PutCellForeColor(clfcCodingLevel, RGB(0,0,0));

			//loop through each column, update the description accordingly
			for(int j=clfcCodingLevelTime+1;j<m_Checklist->GetColumnCount();j++) {

				//for each column, see if we have a rule for that column and row,
				//and if so, show the rule description, otherwise note that no rules are set
				BOOL bFoundRule = FALSE;
				for(int k=0; k<m_pChecklistInfo->paryRules.GetSize() && !bFoundRule; k++) {

					ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(k));

					//see if the rule matches our row and column
					if(pRuleInfo->pRowInfo == pInfo && pRuleInfo->pColumnInfo->nColumnIndex == j) {

						//it matches, so update the cell description
						pRow->PutValue(j, _bstr_t(pRuleInfo->strDescription));

						// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red if not approved
						if(!pRuleInfo->bApproved)
							pRow->PutCellForeColor(j, RGB(255,0,0));
						else
							pRow->PutCellForeColor(j, RGB(0,0,0));

						bFoundRule = TRUE;
					}
				}

				if(!bFoundRule) {
					//update to note that no rules were set
					pRow->PutValue(j, _bstr_t(NO_RULES_SET_DESC));

					// (j.jones 2007-09-17 12:08) - PLID 27398 - color the text red
					pRow->PutCellForeColor(j, RGB(255,0,0));
				}
			}

			//update our stored row pointer
			pInfo->pRow = pRow;

			m_Checklist->AddRowSorted(pRow, NULL);
		}

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::RebuildChecklistRows");
}

//will remove all columns, and rebuild them from memory
//properly storing their indexes in the column info objects
void CEMREMChecklistSetupDlg::RebuildChecklistColumns()
{
	try {

		//clear out the datalist
		m_Checklist->Clear();
		for (short nCol = m_Checklist->GetColumnCount() - 1; nCol >= 0; nCol--) {
			m_Checklist->RemoveColumn(nCol);
		}

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//add the CPT code column, a hidden column we can sort by
		IColumnSettingsPtr pCPTCodeCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCPTCode, _T("CPTCode"), _T("CPTCode"), 0, csVisible|csFixedWidth));
		pCPTCodeCol->FieldType = cftTextSingleLine;
		pCPTCodeCol->PutSortPriority(0);

		//add the coding level column
		IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevel, _T("CodingLevel"), _T("Coding Level"), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;

		// (j.jones 2007-10-01 11:53) - PLID 27396 - add the coding level time column
		IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevelTime, _T("CodingLevelTime"), _T("Time"), 35, csVisible|csWidthData)))->FieldType = cftTextSingleLine;

		short nColIndex = clfcCodingLevelTime + 1;

		//and now add all the user-created columns
		for(int i=0;i<m_pChecklistInfo->paryColumns.GetSize();i++) {

			//update the current column index in the checklist object
			ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));				
			pInfo->nColumnIndex = nColIndex;
			pInfo->nCheckColumnIndex = -1; //patient EMNs only ((j.jones 2007-08-28 13:14) - PLID 27056)
			pInfo->nBorderColumnIndex = -1; //patient EMNs only (// (j.jones 2007-09-17 15:58) - PLID 27399)

			//and create the columns				
			IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex++, _T("Category"), _T(_bstr_t(pInfo->strName)), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;
		}

		//and now rebuild the rows
		RebuildChecklistRows();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::RebuildChecklistColumns");
}

void CEMREMChecklistSetupDlg::OnButtonCreateDelete() 
{
	//if we have a checklist in memory, delete it,
	//otherwise create a new one

	if(m_pChecklistInfo == NULL) {

		try {

			// (j.jones 2007-08-29 11:27) - PLID 27135 - added permissions
			if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptCreate))
				return;

			IRowSettingsPtr pVisitRow = m_VisitTypeCombo->GetCurSel();
			if(pVisitRow == NULL) {
				MessageBox("You must first select a visit type for which to create a checklist.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			long nVisitID = VarLong(pVisitRow->GetValue(vtcID));
			CString strVisitName = VarString(pVisitRow->GetValue(vtcName));

			long nID = NewNumber("EMChecklistsT", "ID");
			ExecuteSql("INSERT INTO EMChecklistsT (ID, VisitTypeID) VALUES (%li, %li)", nID, nVisitID);

			//audit the creation
			CString strNewValue;
			strNewValue.Format("Created for visit type '%s'", strVisitName);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiEMChecklistCreated, nID, "", strNewValue, aepMedium, aetCreated);

			//now Load this new, empty checklist
			Load(nID);

		}NxCatchAll("Error creating checklist.");

	}
	else {

		long nAuditTransactionID = -1;

		try {

			// (j.jones 2007-08-29 11:27) - PLID 27135 - added permissions
			if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptDelete))
				return;

			IRowSettingsPtr pVisitRow = m_VisitTypeCombo->GetCurSel();
			if(pVisitRow == NULL) {
				MessageBox("You must first select a visit type before you can delete a checklist.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			long nVisitID = VarLong(pVisitRow->GetValue(vtcID));
			CString strVisitName = VarString(pVisitRow->GetValue(vtcName));

			//warn heavily
			CString strWarning1, strWarning2;
			// (j.jones 2013-01-16 10:46) - PLID 54634 - added a warning about in-use checklists on EMNs
			strWarning1.Format("Removing the checklist for the '%s' visit type will delete everything you have set up for this checklist. "
				"All created columns, coding levels, and rules will be removed and not recoverable. "
				"It is not recommended that you delete a checklist.\n\n"
				"In addition, any progress you may have saved for this checklist on any patient EMN will be lost, including manually approved rules.\n\n"
				"Are you sure you wish to delete the '%s' visit type's checklist?", strVisitName, strVisitName);
			strWarning2.Format("Are you absolutely SURE? This action is not recoverable!");
			if(IDNO == MessageBox(strWarning1, "Practice", MB_ICONEXCLAMATION|MB_YESNO)
				|| IDNO == MessageBox(strWarning2, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				//nope, not deleting
				return;
			}

			//ok, they're deleting it, let's do this thing
			CWaitCursor pWait;

			// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized and simplified this batch
			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			nAuditTransactionID = BeginAuditTransaction();

			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			//delete all rules
			for(i=m_pChecklistInfo->paryRules.GetSize()-1;i>=0;i--) {
				ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));

				for(int j=pRuleInfo->paryDetails.GetSize()-1;j>=0;j--) {
					ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRuleInfo->paryDetails.GetAt(j));

					//audit the detail
					if(pDetailInfo != NULL) {						
						if(pDetailInfo->nID != -1) {
							CString strOldValue;
							strOldValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);
							AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
						}
						delete pDetailInfo;
					}
				}
				pRuleInfo->paryDetails.RemoveAll();


				//audit the rule
				if(pRuleInfo->nID != -1) {
					//replace newlines with spaces
					CString strOldValue = pRuleInfo->strDescription;
					strOldValue.Replace("\r\n", "  ");

					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
				}

				delete pRuleInfo;

				pRuleInfo = NULL;

				m_pChecklistInfo->paryRules.RemoveAt(i);
			}

			//delete all coding levels
			for(i=m_pChecklistInfo->paryCodingLevelRows.GetSize()-1;i>=0;i--) {
				ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));
				
				//audit the coding level
				if(pInfo->nID != -1) {
					//replace newlines with spaces
					CString strOldValue = pInfo->strDescription;
					strOldValue.Replace("\r\n", "  ");

					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelDeleted, m_pChecklistInfo->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
				}
			
				delete pInfo;

				m_pChecklistInfo->paryCodingLevelRows.RemoveAt(i);

				//we'll remove the row later
			}

			//delete all columns
			for(i=m_pChecklistInfo->paryColumns.GetSize()-1;i>=0;i--) {
				ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));
				
				//audit the column
				if(pInfo->nID != -1) {
					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistColumnDeleted, m_pChecklistInfo->nID, pInfo->strName, "Deleted", aepMedium, aetDeleted);
				}
			
				delete pInfo;

				m_pChecklistInfo->paryColumns.RemoveAt(i);

				//we'll remove the column later
			}

			//and now delete the checklist

			// (j.jones 2013-01-16 10:46) - PLID 54634 - clear progress on EMNs too (this is not patient content, remember it's just a worksheet)
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistRuleProgressT WHERE EMChecklistID = {INT}", m_pChecklistInfo->nID);
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMNEMChecklistCodingLevelProgressT WHERE EMChecklistID = {INT}", m_pChecklistInfo->nID);

			// (j.jones 2013-01-16 10:52) - PLID 54634 - moved all SQL deletion code here and parameterized, as we
			// did not need to delete in the above loops, those were only needed for auditing
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRuleDetailsT WHERE RuleID IN (SELECT ID FROM EMChecklistRulesT WHERE ChecklistID = {INT})", m_pChecklistInfo->nID);
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistRulesT WHERE ChecklistID = {INT}", m_pChecklistInfo->nID);
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistCodingLevelsT WHERE ChecklistID = {INT}", m_pChecklistInfo->nID);
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistColumnsT WHERE ChecklistID = {INT}", m_pChecklistInfo->nID);

			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMChecklistsT WHERE ID = {INT}", m_pChecklistInfo->nID);

			//audit
			CString strNewValue;
			strNewValue.Format("Deleted from visit type '%s'", strVisitName);
			AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistDeleted, m_pChecklistInfo->nID, "", strNewValue, aepMedium, aetDeleted);

			//commit our changes
			// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized this batch
			if(!strSqlBatch.IsEmpty()) {
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			}

			if(nAuditTransactionID != -1)
				CommitAuditTransaction(nAuditTransactionID);

			//clear the interface
			m_ColumnList->Clear();
			m_Checklist->Clear();
			for (short nCol = m_Checklist->GetColumnCount() - 1; nCol >= 0; nCol--) {
				m_Checklist->RemoveColumn(nCol);
			}

			//clear out our stored data
			//(we ought to have already cleared out everything but the checklist pointer)
			ClearChecklistInfo();

			//disable the interface
			ToggleChecklistControls(FALSE);

			//our work here is done
			CString strConfirm;
			strConfirm.Format("The checklist for the '%s' visit type has been deleted.", strVisitName);
			MessageBox(strConfirm, "Practice", MB_OK | MB_ICONINFORMATION);
			return;

		}NxCatchAllCall("Error deleting checklist.",
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
		)
	}
}

void CEMREMChecklistSetupDlg::OnBtnMoveColumnUp() 
{

	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}
		
		IRowSettingsPtr pRow1 = m_ColumnList->CurSel;
		//if no row selected, do nothing
		if(pRow1 == NULL) {
			return;
		}

		IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
		//if there is no previous row, do nothing
		if(pRow2 == NULL)
			return;

		//this will do all the swapping, display updating, and saving
		SwapColumnOrders(pRow1, pRow2);

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnMoveColumnUp");
}

void CEMREMChecklistSetupDlg::OnBtnMoveColumnDown() 
{

	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		IRowSettingsPtr pRow1 = m_ColumnList->CurSel;
		//if no row selected, do nothing
		if(pRow1 == NULL) {
			return;
		}

		IRowSettingsPtr pRow2 = pRow1->GetNextRow();
		//if there is no next row, do nothing
		if(pRow2 == NULL)
			return;

		//this will do all the swapping, display updating, and saving
		SwapColumnOrders(pRow1, pRow2);

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnMoveColumnDown");
}

void CEMREMChecklistSetupDlg::SwapColumnOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2)
{
	long nAuditTransactionID = -1;

	try {
		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(pRow1 == NULL || pRow2 == NULL) {
			ASSERT(FALSE);
			return;
		}

		long nID1 = VarLong(pRow1->GetValue(clcID));
		long nID2 = VarLong(pRow2->GetValue(clcID));
		ChecklistColumnInfo* pInfo1 = NULL;
		ChecklistColumnInfo* pInfo2 = NULL;
		long nArrayIndex1 = -1;
		long nArrayIndex2 = -1;	

		//find the array indexes and info objects
		for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize() && (pInfo1 == NULL || pInfo2 == NULL); i++) {

			ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));

			if(pInfo->nID == nID1) {
				pInfo1 = pInfo;
				nArrayIndex1 = i;
			}
			else if(pInfo->nID == nID2) {
				pInfo2 = pInfo;
				nArrayIndex2 = i;
			}
		}

		long nSortOrder1 = pInfo1->nOrderIndex;
		long nSortOrder2 = pInfo2->nOrderIndex;

		// Swap the sort orders in the array, and in the datalist
		{
			// Swap the sort orders in the array
			{
				// Swap the sort order values themselves
				pInfo1->nOrderIndex = nSortOrder2;
				pInfo2->nOrderIndex = nSortOrder1;
				// Swap the order of the elements in the array
				m_pChecklistInfo->paryColumns.SetAt(nArrayIndex1, pInfo2);
				m_pChecklistInfo->paryColumns.SetAt(nArrayIndex2, pInfo1);

				//rebuild the checklist
				RebuildChecklistColumns();
			}

			// Swap the sort orders in the on-screen datalist
			{
				// Swap the sort order values themselves
				pRow1->PutValue(clcSortOrder, nSortOrder2);
				pRow2->PutValue(clcSortOrder, nSortOrder1);
				// And re-sort
				m_ColumnList->Sort();
			}
		}

		// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized and simplified this batch
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		nAuditTransactionID = BeginAuditTransaction();

		//save the changes
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EMChecklistColumnsT SET OrderIndex = {INT} WHERE ID = {INT}", pInfo1->nOrderIndex, pInfo1->nID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EMChecklistColumnsT SET OrderIndex = {INT} WHERE ID = {INT}", pInfo2->nOrderIndex, pInfo2->nID);

		//audit the changes	
		CString strOldValue, strNewValue;	
		strOldValue.Format("%s (Order: %li)", pInfo1->strName, pInfo2->nOrderIndex);
		strNewValue.Format("%s (Order: %li)", pInfo1->strName, pInfo1->nOrderIndex);
		AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistColumnOrder, m_pChecklistInfo->nID, strOldValue, strNewValue, aepMedium, aetChanged);

		strOldValue.Format("%s (Order: %li)", pInfo2->strName, pInfo1->nOrderIndex);
		strNewValue.Format("%s (Order: %li)", pInfo2->strName, pInfo2->nOrderIndex);
		AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistColumnOrder, m_pChecklistInfo->nID, strOldValue, strNewValue, aepMedium, aetChanged);

		// (j.jones 2013-01-16 10:50) - PLID 54634 - parameterized this batch
		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		if(nAuditTransactionID != -1)
			CommitAuditTransaction(nAuditTransactionID);

		return;

	}NxCatchAllCall("Error in CEMREMChecklistSetupDlg::SwapColumnOrders",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

//shows/hides checklist editing controls
void CEMREMChecklistSetupDlg::ToggleChecklistControls(BOOL bShowChecklist)
{
	//show the "no checklist" label only when we're not showing a checklist AND a visit is selected
	GetDlgItem(IDC_NO_CHECKLIST_LABEL)->ShowWindow(bShowChecklist || m_VisitTypeCombo->GetCurSel() == NULL ? SW_HIDE : SW_SHOW);

	//alter the create/delete button appropriately
	if(bShowChecklist) {
		SetDlgItemText(IDC_BUTTON_CREATE_DELETE, "Delete Checklist");
		m_btnCreateDelete.AutoSet(NXB_DELETE);
	}
	else {
		SetDlgItemText(IDC_BUTTON_CREATE_DELETE, "Create Checklist");
		m_btnCreateDelete.AutoSet(NXB_NEW);
	}

	CRect rect;
	GetDlgItem(IDC_BUTTON_CREATE_DELETE)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);

	//now show or hide the checklist controls
	GetDlgItem(IDC_COLUMN_LIST_LABEL)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CHECKLIST_SETUP_LABEL)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_ADD_COLUMN)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_DELETE_COLUMN)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_ADD_CODING_LEVEL)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EM_CHECKLIST_COLUMN_LIST)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_MOVE_COLUMN_UP)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_MOVE_COLUMN_DOWN)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EM_CHECKLIST_SETUP_LIST)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_SHOW_AUDIT_HISTORY)->ShowWindow(bShowChecklist ? SW_SHOW : SW_HIDE);	

	//always hide the warning label, when we load we'll show it again if needed	
	GetDlgItem(IDC_CHECKLIST_APPROVAL_LABEL)->ShowWindow(SW_HIDE);
}

// (j.jones 2007-08-29 10:48) - PLID 27135 - disable checklist controls if it is read-only
void CEMREMChecklistSetupDlg::SetChecklistControlsReadOnly()
{
	//leave the button enabled, clicking it will check the appropriate permissions
	//GetDlgItem(IDC_BUTTON_CREATE_DELETE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ADD_COLUMN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DELETE_COLUMN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ADD_CODING_LEVEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_MOVE_COLUMN_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_MOVE_COLUMN_DOWN)->EnableWindow(FALSE);
	m_Checklist->Enabled = FALSE;
	m_ColumnList->Enabled = FALSE;
}

//will scan all elements, and if any are unapproved, will show the warning label
void CEMREMChecklistSetupDlg::CheckShowApprovalWarning()
{
	try {

		//start out assuming we're all approved
		BOOL bApproved = TRUE;

		if(m_pChecklistInfo) {
			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			//first check our rules - we'll stop whenever one is not approved
			for(i=0; i<m_pChecklistInfo->paryRules.GetSize() && bApproved; i++) {

				ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
				if(!pRuleInfo->bApproved)
					bApproved = FALSE;
			}

			//now check our coding levels - we'll stop whenever one is not approved
			for(i=0; i<m_pChecklistInfo->paryCodingLevelRows.GetSize() && bApproved; i++) {

				ChecklistCodingLevelInfo* pInfo = (ChecklistCodingLevelInfo*)(m_pChecklistInfo->paryCodingLevelRows.GetAt(i));
				if(!pInfo->bApproved)
					bApproved = FALSE;
			}
		}

		//now show or hide the label
		GetDlgItem(IDC_CHECKLIST_APPROVAL_LABEL)->ShowWindow(bApproved ? SW_HIDE : SW_SHOW);

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::CheckShowApprovalWarning");
}

// (j.jones 2007-08-24 11:03) - PLID 27152 - added ability to view the checklist history
void CEMREMChecklistSetupDlg::OnBtnShowAuditHistory() 
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		CEMChecklistAuditHistoryDlg dlg(this);
		dlg.m_nChecklistID = m_pChecklistInfo->nID;
		dlg.DoModal();

	}NxCatchAll("Error in CEMREMChecklistSetupDlg::OnBtnShowAuditHistory");
}

void CEMREMChecklistSetupDlg::OnOK()
{
	// (c.haag 2007-08-30 12:43) - PLID 27058 - If this is in modeless form,
	// then we just hide. The parent is responsible for destroying this dialog
	// when it's modeless.
	try {
		if (IsModeless()) {
			SetWindowPos(NULL,0,0,0,0,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_HIDEWINDOW);
		} else {
			CDialog::OnOK();
		}
	}
	NxCatchAll("Error in CEMREMChecklistSetupDlg::OnOK");
}

void CEMREMChecklistSetupDlg::OnCancel()
{
	// (c.haag 2007-08-30 12:43) - PLID 27058 - If this is in modeless form,
	// then we just hide. The parent is responsible for destroying this dialog
	// when it's modeless.
	try {
		if (IsModeless()) {
			SetWindowPos(NULL,0,0,0,0,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_HIDEWINDOW);
		} else {
			CDialog::OnCancel();
		}
	}
	NxCatchAll("Error in CEMREMChecklistSetupDlg::OnCancel");
}
