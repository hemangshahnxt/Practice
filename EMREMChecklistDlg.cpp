// EMREMChecklistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMChecklistDlg.h"
#include "EMREMChecklistApprovalDlg.h"
#include "EmrTreeWnd.h"
#include "NxAPI.h"
#include "EMNDetail.h"
#include "EMN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-08-27 08:39) - PLID 27056 - created
// (j.jones 2013-01-04 14:48) - PLID 28135 - changed all references to say E/M, and not use an ampersand

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


#define NO_RULES_SET_DESC "<No Rules Set>"

#define CELL_BKG_COLOR_AUTO_COMPLETE		RGB(192, 255, 192)
#define CELL_BKG_COLOR_FORCED_COMPLETE		RGB(255, 253, 170)
#define CELL_BKG_COLOR_INCOMPLETE			RGB(235, 235, 235)

enum CheckListFixedColumns {

	//We have the following fixed columns:
	clfcCodingLevelID = 0,		// (j.jones 2013-04-19 15:26) - PLID 54596 - added column for EMChecklistCodingLevelsT.ID
	clfcCPTCode,				//the CPT code column (hidden)
	clfcCodingLevel,			//the coding level column
	clfcCodingLevelTime,		//the coding level time required (// (j.jones 2007-09-17 16:59) - PLID 27396)
	clfcCodingLevelCheckbox,	//the coding level approval box

	//Everything else is dynamically added
};

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistDlg dialog

CEMREMChecklistDlg::CEMREMChecklistDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMChecklistDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMChecklistDlg)
		m_pChecklistInfo = NULL;
		m_pEMN = NULL;
		m_nChecklistID = -1;
		m_nVisitTypeID = -1;
		m_pCodingLevelToUse = NULL;
		m_bIsReadOnly = FALSE;
	//}}AFX_DATA_INIT
}


void CEMREMChecklistDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMChecklistDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EDIT_CODING_LEVEL, m_nxeditEditCodingLevel);
	DDX_Control(pDX, IDOK, m_btnAddCharge);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMChecklistDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMChecklistDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistDlg message handlers

BOOL CEMREMChecklistDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_Checklist = BindNxDataList2Ctrl(this, IDC_EM_CHECKLIST, GetRemoteData(), false);
		
		// (c.haag 2008-04-25 17:15) - PLID 29796 - NxIconify the buttons
		// (j.jones 2013-04-24 11:25) - PLID 54596 - Renamed the button variables
		// for clarity, because they aren't displayed as OK/Cancel. Also gave them
		// more appropriate icons for what they actually do.
		m_btnAddCharge.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);

		CString strCaption;
		strCaption.Format("E/M Checklist for '%s'", m_strVisitTypeName);
		SetWindowText(strCaption);

		((CNxEdit*)GetDlgItem(IDC_EDIT_CODING_LEVEL))->SetReadOnly(TRUE);

		// (j.jones 2013-04-23 17:01) - PLID 56372 - if the EMN is read only,
		// keep the datalist editable so the hyperlinks work, but disable the add
		// button so that it's clear that you can't do anything on this dialog
		if(m_bIsReadOnly) {
			m_btnAddCharge.EnableWindow(FALSE);
		}

		//if Load returns false, it's because the checklist is invalid, and we should close
		if(!Load()) {
			CDialog::OnCancel();
			return TRUE;
		}

	}NxCatchAll("Error in CEMREMChecklistDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CEMREMChecklistDlg::~CEMREMChecklistDlg()
{
	ClearChecklistInfo();

	// (j.jones 2007-09-28 08:50) - PLID 27547 - empty the tracked category map
	ClearTrackedCategories();
}

BOOL CEMREMChecklistDlg::Load() 
{	
	try {

		//first clear out the checklist
		{
			//clear the interface
			m_Checklist->Clear();
			for (short nCol = m_Checklist->GetColumnCount() - 1; nCol >= 0; nCol--) {
				m_Checklist->RemoveColumn(nCol);
			}

			//clear out our stored data
			ClearChecklistInfo();
		}

		//if there is no checklist ID, do nothing
		if(m_nChecklistID == -1)
			return FALSE;

		CWaitCursor pWait;

		//now create a new checklist object
		m_pChecklistInfo = new ChecklistInfo;

		m_pChecklistInfo->nID = m_nChecklistID;

		m_Checklist->SetRedraw(FALSE);

		// (j.jones 2013-04-16 12:09) - PLID 54596 - this should never have been opened without a saved EMN,
		// so throw exceptions if that were to happen
		if(m_pEMN == NULL) {
			ThrowNxException("Could not load the E/M checklist because no valid EMN was provided.");
		}
		if(m_pEMN->GetID() == -1) {
			ThrowNxException("Could not load the E/M checklist because the EMN has not been saved.");
		}

		//get our element counts
		PopulateTrackedCategories();

		//add the columns to the checklist
		{
			// (j.jones 2013-04-19 15:26) - PLID 54596 - added column for EMChecklistCodingLevelsT.ID
			IColumnSettingsPtr pIDCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevelID, _T("CodingLevelID"), _T("CodingLevelID"), 0, csVisible|csFixedWidth));
			pIDCol->FieldType = cftTextSingleLine;

			//add the CPT code column, a hidden column we can sort by
			IColumnSettingsPtr pCPTCodeCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCPTCode, _T("CPTCode"), _T("CPTCode"), 0, csVisible|csFixedWidth));
			pCPTCodeCol->FieldType = cftTextSingleLine;
			pCPTCodeCol->PutSortPriority(0);

			//add the coding level column
			// (b.spivey, April 25, 2012) - PLID 49583 - This column doesn't need to auto, it's only ever 5 characters long on the default. 
			// (b.spivey, May 14, 2012) - PLID 49583 - Changed the width type. 
			IColumnSettingsPtr pCodeLevelCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevel, _T("CodingLevel"), _T("Coding Level"), -1, csVisible|csWidthData));
			pCodeLevelCol->FieldType = cftTextWordWrap;
			pCodeLevelCol->PutStoredWidth(90);

			// (j.jones 2007-09-17 17:00) - PLID 27396 - add the coding level time column
			IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevelTime, _T("CodingLevelTime"), _T("Time"), 40, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;

			//add the coding level checkbox column
			// (j.jones 2007-09-07 17:39) - PLID 27336 - add a "App." header
			IColumnSettingsPtr pCodingLevelCheckboxCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(clfcCodingLevelCheckbox, _T("CodingLevelCheck"), _T(_bstr_t("App.")), 35, csVisible|csFixedWidth|csEditable));
			pCodingLevelCheckboxCol->FieldType = cftBoolCheckbox;
			pCodingLevelCheckboxCol->DataType = VT_BOOL;

			short nColIndex = clfcCodingLevelCheckbox + 1;

			_RecordsetPtr rsColumns = CreateParamRecordset("SELECT ID, Name, OrderIndex FROM EMChecklistColumnsT WHERE ChecklistID = {INT} ORDER BY OrderIndex", m_nChecklistID);
			while(!rsColumns->eof) {

				long nID = AdoFldLong(rsColumns, "ID");
				CString strName = AdoFldString(rsColumns, "Name");
				long nOrderIndex = AdoFldLong(rsColumns, "OrderIndex");

				//store in our checklist object (using the current nColIndex)
				ChecklistColumnInfo* pInfo = new ChecklistColumnInfo;
				pInfo->nID = nID;
				pInfo->strName = strName;
				pInfo->nOrderIndex = nOrderIndex;
				
				// (j.jones 2007-09-17 15:47) - PLID 27399 - add a dividing column before the other columns
				pInfo->nBorderColumnIndex = nColIndex;
				IColumnSettingsPtr pBorderCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex++, _T(""), _T(""), 1, csVisible|csFixedWidth));
				pBorderCol->FieldType = cftTextSingleLine;
				pBorderCol->PutBackColor(RGB(0,0,0));
				
				//add the column for the rule
				pInfo->nColumnIndex = nColIndex;
				// (b.spivey, February 24, 2012) - PLID 38409 - Hyperlink "effect" -- Word Wrap Link
				IColumnSettingsPtr(m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex++, _T("Category"), _T(_bstr_t(strName)), -1, csVisible|csWidthAuto)))->FieldType = cftTextWordWrapLink;
				
				//and now the column for the checkbox
				// (j.jones 2007-09-07 17:39) - PLID 27336 - add a "App." header
				pInfo->nCheckColumnIndex = nColIndex;
				IColumnSettingsPtr pCheckboxCol = m_Checklist->GetColumn(m_Checklist->InsertColumn(nColIndex++, _T("CategoryCheck"), _T(_bstr_t("App.")), 35, csVisible|csFixedWidth|csEditable));
				pCheckboxCol->FieldType = cftBoolCheckbox;
				pCheckboxCol->DataType = VT_BOOL;

				//now that we've created the column, add the column info to our array
				m_pChecklistInfo->paryColumns.Add(pInfo);

				rsColumns->MoveNext();
			}
			rsColumns->Close();
		}

		//track the total cells so we can compare to the count of rules later
		long nTotalCells = 0;
		//and track the cells we filled in with rules
		long nRuleCells = 0;

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
				m_nChecklistID);

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
					if(!bApproved) {
						//exit right now, we cannot use this checklist
						AfxMessageBox("At least one coding level is not approved for use on a patient's EMN yet.\n"
							"This checklist cannot be used until all coding levels and rules are approved.");
						return FALSE;
					}

					IRowSettingsPtr pRow = m_Checklist->GetNewRow();
					// (j.jones 2013-04-19 15:26) - PLID 54596 - added column for EMChecklistCodingLevelsT.ID
					pRow->PutValue(clfcCodingLevelID, (long)nCodingLevelID);
					pRow->PutValue(clfcCPTCode, _bstr_t(strCode));
					pRow->PutValue(clfcCodingLevel, _bstr_t(strDescription));

					// (j.jones 2007-09-17 17:02) - PLID 27396 - fill the time column
					CString strTime = "N/A";
					if(nMinimumTime > 0)
						strTime.Format("%li", nMinimumTime);
					pRow->PutValue(clfcCodingLevelTime, _bstr_t(strTime));					

					pRow->PutValue(clfcCodingLevelCheckbox, g_cvarFalse);

					//store in our checklist object
					ChecklistCodingLevelInfo* pInfo = new ChecklistCodingLevelInfo;
					pInfo->nID = nCodingLevelID;
					pInfo->nServiceID = nServiceID;
					pInfo->nColumnsRequired = nMinColumns;
					// (j.jones 2007-09-17 10:34) - PLID 27396 - added minimum time
					pInfo->nMinimumTimeRequired = nMinimumTime;
					pInfo->strCodeNumber = strCode;
					pInfo->strDescription = strDescription;
					//approval information is not needed in memory,
					//we re-use these fields for approving checklist usage
					pInfo->bApproved = FALSE;
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					pInfo->dtApproved = dtInvalid;
					pInfo->nApprovalUserID = -1;
					pInfo->strApprovalUserName = "";
					pInfo->pRow = pRow;

					//add to our memory object
					m_pChecklistInfo->paryCodingLevelRows.Add(pInfo);

					//initialize all columns to having no rules, we will change
					//accordingly for cells that have rules later
					for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize(); i++) {
						ChecklistColumnInfo* pInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));
						//set the "no rules" description
						pRow->PutValue(pInfo->nColumnIndex, _bstr_t(NO_RULES_SET_DESC));
						//set such there is no checkbox
						pRow->PutValue(pInfo->nCheckColumnIndex, g_cvarNull);
						//track how many category cells we initialized this way
						nTotalCells++;
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
					if(!bApproved) {
						//exit right now, we cannot use this checklist
						AfxMessageBox("At least one checklist rule is not approved for use on a patient's EMN yet.\n"
							"This checklist cannot be used until all coding levels and rules are approved.");
						return FALSE;
					}

					ChecklistElementRuleInfo *pInfo = new ChecklistElementRuleInfo;
					pInfo->nID = nRuleID;
					pInfo->strDescription = strDescription;
					pInfo->bRequireAllDetails = bRequireAllDetails;
					//approval information is not needed in memory,
					//we re-use these fields for approving checklist usage
					pInfo->bApproved = FALSE;
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					pInfo->dtApproved = dtInvalid;
					pInfo->nApprovalUserID = -1;
					pInfo->strApprovalUserName = "";

					//initialize to false, we will check this later
					pInfo->bPassed = FALSE;
					
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
						//set the rule description
						pInfo->pRowInfo->pRow->PutValue(pInfo->pColumnInfo->nColumnIndex, _bstr_t(pInfo->strDescription));
						//now set the checkbox to false, thus creating it
						pInfo->pRowInfo->pRow->PutValue(pInfo->pColumnInfo->nCheckColumnIndex, g_cvarFalse);
						//track how many cells we filled
						nRuleCells++;
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

		//warn if not all cells have rules
		if(nTotalCells > nRuleCells) {
			AfxMessageBox("This E/M Checklist does not have rules set for all of its cells.\n"
				"You must have a rule configured and approved for each column of each coding level.");
			return FALSE;
		}

		//now go through the list and color-code accordingly
		CalculatePassedRulesAndColorCells();

		m_Checklist->SetRedraw(TRUE);

		//when we get here, the memory structure is populated, the screen is populated,
		//and we're fully ready to use the checklist

		return TRUE;

	}NxCatchAll("Error in CEMREMChecklistDlg::Load()");

	return FALSE;
}

void CEMREMChecklistDlg::ClearChecklistInfo()
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

	}NxCatchAll("Error in CEMREMChecklistDlg::ClearChecklistInfo()");
}

//given a column index, find and return the matching ChecklistColumnInfo object by the category column
ChecklistColumnInfo* CEMREMChecklistDlg::FindColumnInfoObjectByCategoryColIndex(short nCategoryCol)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize(); i++) {

		ChecklistColumnInfo* pColInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));

		if(pColInfo->nColumnIndex == nCategoryCol) {
			//found it, so return this pointer
			return pColInfo;			
		}
	}

	//it should not be possible to not find an object here
	ASSERT(FALSE);
	return NULL;
}

//given a column index, find and return the matching ChecklistColumnInfo object by the checkbox
ChecklistColumnInfo* CEMREMChecklistDlg::FindColumnInfoObjectByCheckboxColIndex(short nCheckboxCol)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	for(int i=0; i<m_pChecklistInfo->paryColumns.GetSize(); i++) {

		ChecklistColumnInfo* pColInfo = (ChecklistColumnInfo*)(m_pChecklistInfo->paryColumns.GetAt(i));

		if(pColInfo->nCheckColumnIndex == nCheckboxCol) {
			//found it, so return this pointer
			return pColInfo;
		}
	}

	//it should not be possible to not find an object here
	ASSERT(FALSE);
	return NULL;
}

//given a column ID, find and return the matching ChecklistColumnInfo object
ChecklistColumnInfo* CEMREMChecklistDlg::FindColumnInfoObjectByColumnID(long nID)
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
ChecklistCodingLevelInfo* CEMREMChecklistDlg::FindCodingLevelInfoObjectByRowPtr(NXDATALIST2Lib::IRowSettingsPtr pRow)
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

//given a datalist row and a column info ptr, find the rule info object if one exists
ChecklistElementRuleInfo* CEMREMChecklistDlg::FindElementRuleInfoObject(NXDATALIST2Lib::IRowSettingsPtr pRow, ChecklistColumnInfo *pColInfo)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	//try to find a rule that matches this column and row
	for(int i=0; i<m_pChecklistInfo->paryRules.GetSize(); i++) {

		ChecklistElementRuleInfo *pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
		if(pRuleInfo->pColumnInfo == pColInfo && pRuleInfo->pRowInfo->pRow == pRow) {
			//found it, return
			return pRuleInfo;
		}
	}

	//if we got here, we didn't find anything
	return NULL;
}

// (j.jones 2013-04-19 16:04) - PLID PLID 54596 - finds a rule by rule ID
ChecklistElementRuleInfo* CEMREMChecklistDlg::FindElementRuleInfoObjectByID(long nRuleID)
{
	if(m_pChecklistInfo == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return NULL;
	}

	//try to find a rule that matches this ID
	for(int i=0; i<m_pChecklistInfo->paryRules.GetSize(); i++) {

		ChecklistElementRuleInfo *pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
		if(pRuleInfo->nID == nRuleID) {
			return pRuleInfo;
		}
	}

	//if we got here, we didn't find anything
	return NULL;
}

void CEMREMChecklistDlg::PopulateTrackedCategories()
{
	try {

		ClearTrackedCategories();

		// (j.jones 2013-02-13 14:44) - PLID 54668 - the element calculation is now all done in the API
		if(m_pEMN != NULL && m_pEMN->GetID() != -1) {
			NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
			if(pApi == NULL) {
				ThrowNxException("Could not call CalculateEMElementsForEMN due to an invalid API.");
			}

			NexTech_Accessor::_EMCodingChecklistElementResultsPtr pResults = pApi->CalculateEMElementsForEMN(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()));

			//CalculateEMElementsForEMN returns all the categories we had elements for,
			//as well as the information on details that contributed to those categories.
			//It also returns the total time spent on the EMN, but that is not used by
			//Practice, which instead looks the active ever-increasing time clock.

			if(pResults) {
				Nx::SafeArray<IUnknown *> aryResults = Nx::SafeArray<IUnknown *>(pResults->GetResults());
				long nResultCount = aryResults.GetCount();

				for(int i=0; i<nResultCount; i++) {
					NexTech_Accessor::_EMCodingChecklistElementResultPtr pResult = aryResults.GetAt(i);
					long nCategoryID = atoi((LPCTSTR)pResult->GetcategoryID());

					Nx::SafeArray<IUnknown *> aryDetails = Nx::SafeArray<IUnknown *>(pResult->GetDetailInfo());
					long nDetailCount = aryDetails.GetCount();

					for(int j=0; j<nDetailCount; j++) {
						NexTech_Accessor::_EMCodingChecklistElementResultDetailPtr pResultDetail = aryDetails.GetAt(j);
						if(pResultDetail) {
							long nDetailID = atoi((LPCTSTR)pResultDetail->GetdetailID());
							long nCountElementsFound = atoi((LPCTSTR)pResultDetail->GetCountElementsFound());

							CEMNDetail *pDetail = m_pEMN->GetDetailByID(nDetailID);
							if(pDetail == NULL) {
								ThrowNxException("Detail ID %li does not exist on this EMN!", nDetailID);
							}

							AddEMElementsToCategoryArray(nCategoryID, nCountElementsFound, pDetail, m_aryTrackedCategories);
						}
					}
				}
			}
		}

	}NxCatchAll("Error calculating E/M element counts.");
}

// (j.jones 2007-09-28 08:50) - PLID 27547 - empty the tracked category map
void CEMREMChecklistDlg::ClearTrackedCategories()
{
	try {

		for(int i=m_aryTrackedCategories.GetSize()-1;i>=0;i--) {
			ChecklistTrackedCategoryInfo *pCatInfo = (ChecklistTrackedCategoryInfo*)(m_aryTrackedCategories.GetAt(i));

			for(int j=pCatInfo->aryDetailInfo.GetSize()-1;j>=0;j--) {
				ChecklistTrackedCategoryDetailInfo* pDetailInfo = (ChecklistTrackedCategoryDetailInfo*)(pCatInfo->aryDetailInfo.GetAt(j));
				//do NOT delete pDetailInfo->pDetail!
				delete pDetailInfo;
			}
			pCatInfo->aryDetailInfo.RemoveAll();

			delete pCatInfo;

			pCatInfo = NULL;
		}
		m_aryTrackedCategories.RemoveAll();

	}NxCatchAll("Error clearing tracked categories.");
}

//this is the key function that colorizes cells based on E/M elements and the cell rules
void CEMREMChecklistDlg::CalculatePassedRulesAndColorCells()
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2007-08-29 16:26) - PLID 27057 - do not calculate passed rules
		// unless all details are up to date
		if(!VerifyAllDetailsUpToDate()) {

			//color all cells as incomplete
			IRowSettingsPtr pRow = m_Checklist->GetFirstRow();
			while(pRow) {
				pRow->PutBackColor(CELL_BKG_COLOR_INCOMPLETE);

				pRow = pRow->GetNextRow();
			}
			return;
		}

		// (j.jones 2013-04-16 12:09) - PLID 54596 - the API now calculates completion status for us,
		// and also loads previously saved progress, including rules manually marked approved
		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
		if(pApi == NULL) {
			ThrowNxException("Could not calculate E/M rule progress due to an invalid API.");
		}

		//the dialog should have already thrown exceptions in the intial load
		//if the EMN was null or had a -1 ID, but just for safety's sake, let's do it again
		if(m_pEMN == NULL) {
			ThrowNxException("Could not calculate E/M rule progress because no valid EMN was provided.");
		}
		if(m_pEMN->GetID() == -1) {
			ThrowNxException("Could not calculate E/M rule progress because the EMN has not been saved.");
		}

		NexTech_Accessor::_EMCodingChecklistPtr pChecklist = pApi->GetEMChecklistProgress(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()));
		if(pChecklist) {

			//Loop through each coding level, then get the completion status of each column.
			//Colorize each completed column, and then colorize the row if it is completed.
			Nx::SafeArray<IUnknown *> aryCodingLevels = Nx::SafeArray<IUnknown *>(pChecklist->GetChecklistCodingLevels());
			long nCodingLevelCount = aryCodingLevels.GetCount();
			for(int cl=0; cl<nCodingLevelCount; cl++) {
				NexTech_Accessor::_EMCodingChecklistCodingLevelPtr pCodingLevel = aryCodingLevels.GetAt(cl);
				//long nServiceID = atoi((LPCTSTR)pCodingLevel->GetserviceID());
				long nCodingLevelID = atoi((LPCTSTR)pCodingLevel->GetLevelID());
				long nMinColumns = atoi((LPCTSTR)pCodingLevel->GetMinColumns());
				//CString strCodingLevelDesc = (LPCTSTR)pCodingLevel->Getdescription();
				long nMinimumTime = atoi((LPCTSTR)pCodingLevel->GetMinimumTime());
				long nCountColumnsMet = atoi((LPCTSTR)pCodingLevel->GetCountColumnsMet());
				//VARIANT_BOOL bCodingLevelMet = pCodingLevel->GetLevelMet();
				VARIANT_BOOL bCodingLevelManuallyApproved = pCodingLevel->GetManuallyApproved();

				//find the matching coding level row
				IRowSettingsPtr pRow = m_Checklist->FindByColumn(clfcCodingLevelID, (long)nCodingLevelID, m_Checklist->GetFirstRow(), VARIANT_FALSE);
				if(pRow == NULL) {
					ThrowNxException("Could not calculate E/M rule progress because a coding level row could not be found for ID %li.", nCodingLevelID);
				}

				ChecklistCodingLevelInfo *pCodingLevelInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
				if(pCodingLevelInfo == NULL) {
					ThrowNxException("Could not calculate E/M rule progress because the coding level object for ID %li could not be found.", nCodingLevelID);
				}

				Nx::SafeArray<IUnknown *> aryChecklistColumns = Nx::SafeArray<IUnknown *>(pCodingLevel->GetChecklistColumns());
				long nChecklistColumnCount = aryChecklistColumns.GetCount();
				for(int col=0; col<nChecklistColumnCount; col++) {
					NexTech_Accessor::_EMCodingChecklistColumnPtr pChecklistColumn = aryChecklistColumns.GetAt(col);
					//CString strColumnName = (LPCTSTR)pChecklistColumn->GetName();
					//long nOrderIndex = atoi((LPCTSTR)pChecklistColumn->GetOrderIndex());

					NexTech_Accessor::_EMCodingChecklistRulePtr pRule = pChecklistColumn->GetChecklistRule();
					long nRuleID = atoi((LPCTSTR)pRule->GetruleID());
					//CString strRuleDesc = (LPCTSTR)pRule->Getdescription();
					VARIANT_BOOL bRequireAllDetails = pRule->GetRequireAllDetails();
					VARIANT_BOOL bRuleMet = pRule->GetRuleMet();
					VARIANT_BOOL bRuleManuallyApproved = pRule->GetManuallyApproved();

					ChecklistElementRuleInfo *pRuleInfo = FindElementRuleInfoObjectByID(nRuleID);
					if(pRuleInfo == NULL) {
						ThrowNxException("Could not calculate E/M rule progress because a rule object could not be found for ID %li.", nRuleID);
					}
					if(pRuleInfo->pColumnInfo == NULL) {
						ThrowNxException("Could not calculate E/M rule progress because the rule object for ID %li has no column object.", nRuleID);
					}

					// (j.jones 2008-09-30 09:20) - PLID 31536 - we need to track whether
					// "any" or "all" items completed
					BOOL bPartialCompleted = FALSE;
					BOOL bFullyCompleted = TRUE;

					Nx::SafeArray<IUnknown *> aryRuleDetails = Nx::SafeArray<IUnknown *>(pRule->GetChecklistRuleDetails());
					long nRuleDetailCount = aryRuleDetails.GetCount();
					if(nRuleDetailCount == 0) {
						//no details, we can't complete it
						bPartialCompleted = FALSE;
						bFullyCompleted = FALSE;
					}
					else {
						for(int rd=0; rd<nRuleDetailCount; rd++) {
							NexTech_Accessor::_EMCodingChecklistRuleDetailPtr pRuleDetail = aryRuleDetails.GetAt(rd);
							//long nCategoryID = atoi((LPCTSTR)pRuleDetail->GetcategoryID());
							//CString strCategoryName = (LPCTSTR)pRuleDetail->GetCategoryName();
							long nMinElements = atoi((LPCTSTR)pRuleDetail->GetMinElements());
							long nCountElementsFound = atoi((LPCTSTR)pRuleDetail->GetCountElementsFound());
							//VARIANT_BOOL bRuleDetailMet = pRuleDetail->GetRuleMet();

							// (j.jones 2008-09-30 09:21) - PLID 31536 - if the detail isn't satisfied,
							// we know that the rule can't be fully completed, but if it is satisfied,
							// we know that the rule can be partially completed

							//do we have enough elements for this category?
							if(nCountElementsFound < nMinElements) {
								//we do not, so we know it's not fully completed
								bFullyCompleted = FALSE;
							}
							else {
								//we do, so we know it's at least partially completed
								bPartialCompleted = TRUE;
							}

							// (j.jones 2007-09-18 15:52) - PLID 27397 - if completed after the first pass,
							// and the "any" flag is set, then exit out of this loop, as this rule has passed
							// (j.jones 2008-09-30 09:21) - PLID 31536 - forget the first pass nonsense,
							// instead if it is partially completed and we don't require all details, we can
							// break now because we know the rule is satisfied, and conversely if the rule is
							// not fully completed and it does require all details, we can break now because
							// we know the rule cannot be satisfied
							if(bPartialCompleted && !bRequireAllDetails) {
								break;
							}
							else if(!bFullyCompleted && bRequireAllDetails) {
								break;
							}
						}
					}

					pRuleInfo->bPassed = bRuleMet;

					//if they had previously approved, update the memory object and the colors accordingly
					if(bRuleMet || bRuleManuallyApproved) {
						pRuleInfo->bApproved = TRUE;
						pRow->PutValue(pRuleInfo->pColumnInfo->nCheckColumnIndex, g_cvarTrue);

						//we can't fill who approved it and when, the API doesn't tell us that,
						//but we also don't need it, because we won't be auditing this

						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, bRuleMet ? CELL_BKG_COLOR_AUTO_COMPLETE : CELL_BKG_COLOR_FORCED_COMPLETE);
						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, bRuleMet ? CELL_BKG_COLOR_AUTO_COMPLETE : CELL_BKG_COLOR_FORCED_COMPLETE);
					}
					else if((bPartialCompleted && !bRequireAllDetails) || (bFullyCompleted && bRequireAllDetails)) {
						//our rule is satisfied for this cell
						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);
						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);

						//track that this column is completed
						pRuleInfo->bPassed = TRUE;

						//approve this cell
						ApproveCell(pRow, pRuleInfo);
					}
					else {
						//no rule, nothing can be filled
						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_INCOMPLETE);
						pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_INCOMPLETE);
					}
				}

				// (j.jones 2007-09-17 10:34) - PLID 27396 - Added minimum time,
				// which will require a given amount to time spent on an EMN in order
				// to satisfy a coding level. Zero means no requirement, but needs no
				// special handling in this logic.

				double dblMinutesSpentOnEMN = 0.0;
				if(m_pEMN) {
					COleDateTimeSpan dtSpan = m_pEMN->GetTotalTimeSpanOpened();
					//we should have already had an assertion if this is invalid
					if(dtSpan.GetStatus() != COleDateTimeSpan::invalid)
						dblMinutesSpentOnEMN = dtSpan.GetTotalMinutes();
				}
				else {
					//why don't we have an EMN?
					ASSERT(FALSE);
				}

				BOOL bTimeOK = (double)nMinimumTime <= dblMinutesSpentOnEMN;

				//now, do we have enough columns satisfied to also complete the row?
				if(nMinColumns <= nCountColumnsMet && bTimeOK) {
					//hey, we can use this coding level!
					pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_AUTO_COMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_AUTO_COMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_AUTO_COMPLETE);				
				}
				// (b.spivey, February 28, 2012) - PLID 38409 - It can initialize as "yellow" if it meets all but the time requirements.
				else if(nMinColumns <= nCountColumnsMet && !bTimeOK) {
					pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_FORCED_COMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_FORCED_COMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_FORCED_COMPLETE);
				}
				else {
					//insert price is right failure music here
					pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_INCOMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_INCOMPLETE);
					pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_INCOMPLETE);				
				}

				//if they had previously manually approved, update the memory object and the colors accordingly
				if(bCodingLevelManuallyApproved) {
					pCodingLevelInfo->bApproved = TRUE;
					pRow->PutValue(clfcCodingLevelCheckbox, g_cvarTrue);

					//we can't fill who approved it and when, the API doesn't tell us that,
					//but we also don't need it, because we won't be auditing this

					//colorize this coding level cell
					UpdateCodingLevelStatus(pCodingLevelInfo);
				}
			}
		}

		FindAndReflectBestServiceCode();

	}NxCatchAll("Error coloring E/M checklist cells.");
}

//this function re-colorizes a coding level cell based on the approval status of rules,
//and also potentially unchecks the approval status if rule approvals have changed
void CEMREMChecklistDlg::UpdateCodingLevelStatus(ChecklistCodingLevelInfo *pCodingLevelInfo)
{
	long nCountColumnsForcedComplete = 0;
	long nColumnsApproved = 0;
	long nColumnsPassed = 0;

	for(int i=0;i<m_pChecklistInfo->paryRules.GetSize(); i++) {
		ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
		//if the rule is for this coding level,
		//check the approval status, and the passed status
		if(pRuleInfo->pRowInfo == pCodingLevelInfo) {

			if(pRuleInfo->bApproved) {
				//increment our approved count for this coding level
				nColumnsApproved++;
			}

			if(pRuleInfo->bPassed) {
				//increment our approved count for this coding level
				nColumnsPassed++;
			}

			//was this rule forced complete?
			if(pRuleInfo->bApproved && !pRuleInfo->bPassed) {
				//yes, so track this
				nCountColumnsForcedComplete++;
			}
		}
	}

	// (j.jones 2007-09-17 10:34) - PLID 27396 - Added minimum time,
	// which will require a given amount to time spent on an EMN in order
	// to satisfy a coding level. Zero means no requirement, but needs no
	// special handling in this logic.

	double dblMinutesSpentOnEMN = 0.0;
	if(m_pEMN) {
		COleDateTimeSpan dtSpan = m_pEMN->GetTotalTimeSpanOpened();
		//we should have already had an assertion if this is invalid
		if(dtSpan.GetStatus() != COleDateTimeSpan::invalid)
			dblMinutesSpentOnEMN = dtSpan.GetTotalMinutes();
	}
	else {
		//why don't we have an EMN?
		ASSERT(FALSE);
	}

	BOOL bTimeOK = (double)pCodingLevelInfo->nMinimumTimeRequired <= dblMinutesSpentOnEMN;

	if(pCodingLevelInfo->nColumnsRequired <= nColumnsPassed && bTimeOK) {
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_AUTO_COMPLETE);
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_AUTO_COMPLETE);
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_AUTO_COMPLETE);		
	}
	else if(pCodingLevelInfo->nColumnsRequired <= (nColumnsPassed + nCountColumnsForcedComplete)
		|| (pCodingLevelInfo->nColumnsRequired <= nColumnsPassed && !bTimeOK)) {
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_FORCED_COMPLETE);		
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_FORCED_COMPLETE);
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_FORCED_COMPLETE);
	}
	else {
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevel, CELL_BKG_COLOR_INCOMPLETE);		
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelTime, CELL_BKG_COLOR_INCOMPLETE);
		pCodingLevelInfo->pRow->PutCellBackColor(clfcCodingLevelCheckbox, CELL_BKG_COLOR_INCOMPLETE);
	}

	//now, do we need to revoke the approval?
	// (j.jones 2007-09-18 10:23) - PLID 27396 - do not check time here, it can be overridden
	if(pCodingLevelInfo->bApproved && pCodingLevelInfo->nColumnsRequired > nColumnsApproved) {

		//indeed we do
		pCodingLevelInfo->bApproved = FALSE;
		pCodingLevelInfo->dtApproved.SetStatus(COleDateTime::invalid);
		pCodingLevelInfo->nApprovalUserID = -1;
		pCodingLevelInfo->strApprovalUserName = "";

		//update the datalist
		pCodingLevelInfo->pRow->PutValue(clfcCodingLevelCheckbox, g_cvarFalse);


		// (j.jones 2013-04-22 14:21) - PLID 54596 - save & audit immediately
		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
		if(pApi == NULL) {
			ThrowNxException("Could not save coding level changes due to an invalid API.");
		}
		pApi->SetEMChecklistCodingLevelApproval(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()), (LPCTSTR)AsString(pCodingLevelInfo->nID), VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE);
	}
}

BEGIN_EVENTSINK_MAP(CEMREMChecklistDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMREMChecklistDlg)
	ON_EVENT(CEMREMChecklistDlg, IDC_EM_CHECKLIST, 10 /* EditingFinished */, OnEditingFinishedEmChecklist, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMREMChecklistDlg, IDC_EM_CHECKLIST, 8 /* EditingStarting */, OnEditingStartingEmChecklist, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEMREMChecklistDlg, IDC_EM_CHECKLIST, 19, CEMREMChecklistDlg::LeftClickEmChecklist, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (j.jones 2007-09-27 16:51) - PLID 27547 - added ability to pop up a dialog when an approval is attempted
void CEMREMChecklistDlg::OnEditingStartingEmChecklist(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		// (j.jones 2013-04-23 16:56) - PLID 56372 - if read only, you cannot edit anything
		if(m_bIsReadOnly) {
			*pbContinue = FALSE;
			return;
		}

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			*pbContinue = FALSE;
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbContinue = FALSE;
			return;
		}

		//find the current value

		BOOL bApproved = FALSE;
		if(pvarValue->vt == VT_BOOL)
			bApproved = VarBool(pvarValue);
		else if(pvarValue->vt == VT_I4)
			bApproved = (VarLong(pvarValue) != 0);

		if(nCol != clfcCodingLevelCheckbox && !bApproved) {

			//we need to find what column this is
			ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByCheckboxColIndex(nCol);
			if(pColInfo == NULL) {
				//would mean we don't have a checkbox column that matches this index
				//since checkboxes are the only editable columns, this should be impossible,
				//but we won't ASSERT
				*pbContinue = FALSE;
				return;
			}

			//find which rule this is
			ChecklistElementRuleInfo *pRuleInfo = FindElementRuleInfoObject(pRow, pColInfo);
			if(pRuleInfo == NULL) {
				//we shouldn't have allowed them into the checklist dialog
				//if there isn't a rule for every cell
				ASSERT(FALSE);
				*pbContinue = FALSE;
				return;
			}
			// (b.spivey, February 28, 2012) - PLID 38409 - If it passed, they just want to approve it. 
			else if (pRuleInfo->bPassed) {
				*pbContinue = TRUE;
				return; 
			}

			// (j.jones 2007-09-28 14:22) - PLID 27547 - now display the approval dialog
			CEMREMChecklistApprovalDlg dlg(this);

			dlg.m_pRuleInfo = pRuleInfo;
			dlg.m_aryTrackedCategories = &m_aryTrackedCategories;

			if(dlg.DoModal() == IDCANCEL) {
				//if they cancelled, don't approve the rule
				*pbContinue = FALSE;
				return;
			}
			else {
				// (b.spivey, February 28, 2012) - PLID 38409 - Removed the checkbox, it's either cancel or approve. 
				//grab the approved status and continue only if they approved the dialog
				*pbContinue = TRUE;
				return;
			}
		}
		else {

			//if this cell is not an unapproved column rule, then continue normally
			return;
		}

	}NxCatchAll("Error attempting checklist approval.");
}

void CEMREMChecklistDlg::OnEditingFinishedEmChecklist(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		// (j.jones 2013-04-23 16:56) - PLID 56372 - if read only, you cannot edit anything
		if(m_bIsReadOnly) {
			return;
		}

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}
		
		if(!bCommit)
			return;

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;

		BOOL bApproved = FALSE;
		if(varNewValue.vt == VT_BOOL)
			bApproved = VarBool(varNewValue);
		else if(varNewValue.vt == VT_I4)
			bApproved = (VarLong(varNewValue) != 0);

		if(nCol == clfcCodingLevelCheckbox) {

			//they're editing the approval for a coding level

			//find the coding level
			ChecklistCodingLevelInfo *pCodingLevelInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
			if(pCodingLevelInfo == NULL) {
				//should be impossible
				ASSERT(FALSE);
				return;
			}

			//for purposes of coloring, we need to track how many approved columns
			//were satisfied by the rules, compared to how many were forced complete
			long nCountColumnsForcedComplete = 0;
			long nColumnsApproved = 0;
			long nColumnsPassed = 0;


			// (b.spivey, March 01, 2012) - PLID 38409 - Moved this above the rule checks, since we can 

			// (j.jones 2007-09-17 10:34) - PLID 27396 - Added minimum time,
			// which will require a given amount to time spent on an EMN in order
			// to satisfy a coding level. Zero means no requirement, but needs no
			// special handling in this logic.

			if(bApproved) {

				double dblMinutesSpentOnEMN = 0.0;
				if(m_pEMN) {
					COleDateTimeSpan dtSpan = m_pEMN->GetTotalTimeSpanOpened();
					//we should have already had an assertion if this is invalid
					if(dtSpan.GetStatus() != COleDateTimeSpan::invalid)
						dblMinutesSpentOnEMN = dtSpan.GetTotalMinutes();
				}
				else {
					//why don't we have an EMN?
					ASSERT(FALSE);
				}

				if((double)pCodingLevelInfo->nMinimumTimeRequired > dblMinutesSpentOnEMN) {
					
					//not enough time, so warn

					CString strWarning;
					strWarning.Format("The %s coding level requires at least %li minutes spent on this EMN before the coding level can be used.\n"
						"You have only been editing this EMN for a total of %li minutes.\n\n"
						"Are you sure you wish to approve the %s coding level?",
						pCodingLevelInfo->strCodeNumber, pCodingLevelInfo->nMinimumTimeRequired, (long)dblMinutesSpentOnEMN, pCodingLevelInfo->strCodeNumber);

					if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

						//reset the checkbox
						pRow->PutValue(clfcCodingLevelCheckbox, g_cvarFalse);

						bApproved = FALSE;
					}
				}
			}

			if(bApproved) {
				//before approving this coding level, verify that enough columns are approved,
				//and if not, force the user to approve enough columns first				

				// (b.spivey, February 28, 2012) - PLID 38409 - We're going to need an iterator, number of columns, and the size of
				//	the checklist. 
				long nChecklistIterator = 0, 
					nChecklistColumns = 0, 
					nChecklistSize = m_pChecklistInfo->paryRules.GetSize(); 
				bool bForceApprove = false, 
					bPromptForApprove = true; 

				// (b.spivey, February 28, 2012) - PLID 38409 - Changed to a while loop. Iterates through the entire checklist. 
				while (nChecklistIterator < nChecklistSize) {
					ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(nChecklistIterator));
					// (b.spivey, February 28, 2012) - PLID 38409 - iterate. 
					nChecklistIterator++; 
					//if the rule is for this coding level,
					//check the approval status, and the passed status
					if(pRuleInfo->pRowInfo == pCodingLevelInfo) {
						// (b.spivey, February 28, 2012) - PLID 38409 - This is a checkbox column. 
						nChecklistColumns++; 
						if(pRuleInfo->bApproved) {
							//increment our approved count for this coding level
							nColumnsApproved++;
						}

						// (b.spivey, February 28, 2012) - PLID 38409 - If it passes, just autoapprove it. Saves clicks. 
						if(pRuleInfo->bPassed && !pRuleInfo->bApproved) {
							//increment our approved count for this coding level
							ApproveCell(pRow, pRuleInfo); 
							nColumnsApproved++; 
						}

						// (b.spivey, February 28, 2012) - PLID 38409 - Force approve the row if we're running the loop again, 
						//	 even though the rules don't technically pass. 
						if(!pRuleInfo->bApproved && bForceApprove) {
							//Force approve entire row. 
							ApproveCell(pRow, pRuleInfo); 
						}

						//was this rule forced complete?
						if(pRuleInfo->bApproved && !pRuleInfo->bPassed) {
							//yes, so track this
							nCountColumnsForcedComplete++;
						}
					}

					// (b.spivey, February 27, 2012) - PLID 38409 - If we're at the end of the loop, we haven't prompted to approve, 
					// and the number approved is less than the number required, lets ask if they want to approve it anyways. 
					if(nChecklistIterator >= nChecklistSize 
						&& bPromptForApprove 
						&& nColumnsApproved < pCodingLevelInfo->nColumnsRequired) {

						//now, warn if not enough columns were approved
						CString strWarning;
						strWarning.Format("The %s coding level requires at least %li column(s) to be approved before the coding level can be used.\n"
							"You have approved %li column(s), and need to approve %li more before this coding level can be approved. \n\n"
							"Do you want to approve this code regardless?",
							pCodingLevelInfo->strCodeNumber, pCodingLevelInfo->nColumnsRequired, nColumnsApproved, pCodingLevelInfo->nColumnsRequired - nColumnsApproved);
						if(MessageBox(strWarning, NULL, MB_YESNO|MB_ICONWARNING) == IDYES) {
							//reset the loop, make sure not to prompt again. 
							nChecklistIterator = 0; 
							nChecklistColumns = 0; 
							bPromptForApprove = false; 
							bForceApprove = true;
							bApproved = TRUE;
						}
						else {
							//reset the checkbox
							pRow->PutValue(clfcCodingLevelCheckbox, g_cvarFalse);
							bApproved = FALSE;
							break; 
						}
					}
					// (b.spivey, February 28, 2012) - PLID 38409 - If we're at the end of our loop, and the columns approved are 
					// less than the columns on the list, run it again with the force approve flag. 
					else if (nChecklistIterator >= nChecklistSize 
						&& nColumnsApproved < nChecklistColumns) {
						nChecklistColumns = 0; 
						nChecklistIterator = 0; 
						bForceApprove = true; 
					}
				}
			}

			//now update the memory object and the colors accordingly
			if(bApproved) {
				//they checked the box

				pCodingLevelInfo->bApproved = TRUE;

				//store the time this occurred
				pCodingLevelInfo->dtApproved = COleDateTime::GetCurrentTime();;

				//and the user that did this
				pCodingLevelInfo->nApprovalUserID = GetCurrentUserID();
				pCodingLevelInfo->strApprovalUserName = GetCurrentUserName();
			}
			else {
				//they unchecked the box

				//update the coding level approval fields
				pCodingLevelInfo->bApproved = FALSE;
				pCodingLevelInfo->dtApproved.SetStatus(COleDateTime::invalid);
				pCodingLevelInfo->nApprovalUserID = -1;
				pCodingLevelInfo->strApprovalUserName = "";
			}

			//colorize this coding level cell
			UpdateCodingLevelStatus(pCodingLevelInfo);

			// (j.jones 2013-04-22 11:00) - PLID 54596 - save & audit immediately
			NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
			if(pApi == NULL) {
				ThrowNxException("Could not save coding level changes due to an invalid API.");
			}
			//do not update charges yet, instead we will do that when they click ok
			pApi->SetEMChecklistCodingLevelApproval(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()), (LPCTSTR)AsString(pCodingLevelInfo->nID), bApproved ? VARIANT_TRUE : VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE);
		}
		else {

			//we need to find what column this is
			ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByCheckboxColIndex(nCol);
			if(pColInfo == NULL) {
				//would mean we don't have a checkbox column that matches this index
				//since checkboxes are the only editable columns, this should be impossible,
				//but we won't ASSERT
				return;
			}

			//find which rule this is
			ChecklistElementRuleInfo *pRuleInfo = FindElementRuleInfoObject(pRow, pColInfo);
			if(pRuleInfo == NULL) {
				//we shouldn't have allowed them into the checklist dialog
				//if there isn't a rule for every cell
				ASSERT(FALSE);
				return;
			}

			// (j.jones 2007-09-27 16:58) - PLID 27547 - the checklist approval dialog called in OnEditingStartingEmChecklist
			// would have given all the necessary prompts before approving, so we need not prompt here

			//now update the memory object and the colors accordingly
			if(bApproved) {
				//they checked the box

				// (b.spivey, February 28, 2012) - PLID 38409 - Moved this logic to a function
				ApproveCell(pRow, pRuleInfo); 
			}
			else {
				//they unchecked the box

				// (b.spivey, February 28, 2012) - PLID 38409 - Moved this logic to a function. 
				//update the rule approval fields
				UnapproveCell(pRow, pRuleInfo); 
			}

			//now update the coding level info. cell color and status appropriately
			// (j.jones 2013-04-22 11:00) - PLID 54596 - if this change causes an approved coding level
			// to no longer be approved, it will save & audit immediately
			UpdateCodingLevelStatus(pRuleInfo->pRowInfo);
		}

		//now, based on our approval changes, re-check for the best service code
		FindAndReflectBestServiceCode();

	}NxCatchAll("Error editing checklist approval.");
}

//based on what is approved by the user, reflect the highest CPT code in the interface
void CEMREMChecklistDlg::FindAndReflectBestServiceCode()
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//the rows are in order of CPT code ascending, so search in order of the datalist
		IRowSettingsPtr pRow = m_Checklist->GetFirstRow();

		//revert to not having a coding level selected
		m_pCodingLevelToUse = NULL;

		while(pRow) {

			ChecklistCodingLevelInfo *pCodingLevelInfo = FindCodingLevelInfoObjectByRowPtr(pRow);
			if(pCodingLevelInfo == NULL) {
				//should be impossible
				ASSERT(FALSE);
				return;
			}
			
			if(pCodingLevelInfo->bApproved) {
				//the coding level should not be approved without enough rules approved, but check again anyways

				long nColumnsApproved = 0;
				for(int i=0;i<m_pChecklistInfo->paryRules.GetSize() && nColumnsApproved < pCodingLevelInfo->nColumnsRequired; i++) {
					ChecklistElementRuleInfo* pRuleInfo = (ChecklistElementRuleInfo*)(m_pChecklistInfo->paryRules.GetAt(i));
					//if the rule is for this coding level, check the approval status, and ignore the passed status
					if(pRuleInfo->pRowInfo == pCodingLevelInfo && pRuleInfo->bApproved) {
						//increment our approved count for this coding level
						nColumnsApproved++;
					}
				}

				//is this coding level truly approved?
				if(nColumnsApproved >= pCodingLevelInfo->nColumnsRequired) {
					//it is, so this is now the best coding level
					m_pCodingLevelToUse = pCodingLevelInfo;
				}
				else {
					//We should have never allowed a coding level to be approved
					//if not enough columns are approved. Find out how this happened.
					ASSERT(FALSE);
				}

				// (j.jones 2007-09-17 10:34) - PLID 27396 - don't need to check minimum time here, as it is overrideable
			}

			pRow = pRow->GetNextRow();
		}
		
		//set the "Coding Level To Use" field to show the best code we found
		// (j.jones 2013-04-24 11:30) - PLID 54596 - now the button to add a charge
		// enables/disables based on whether a coding level exists
		if(m_pCodingLevelToUse) {
			//fill the coding level field
			SetDlgItemText(IDC_EDIT_CODING_LEVEL, m_pCodingLevelToUse->strCodeNumber);
			//enable the add button, unless we're read-only, and change
			//the text to reflect the code that will be added
			m_btnAddCharge.EnableWindow(!m_bIsReadOnly);
			CString strText;
			strText.Format("Add Charge For Coding Level %s", m_pCodingLevelToUse->strCodeNumber);
			m_btnAddCharge.SetWindowText(strText);
		}
		else {
			//clear the coding level field
			SetDlgItemText(IDC_EDIT_CODING_LEVEL, "");
			//disable the add button, and revert to its generic text
			m_btnAddCharge.EnableWindow(FALSE);
			m_btnAddCharge.SetWindowText("Add Charge For Current Coding Level");
		}

	}NxCatchAll("Error in CEMREMChecklistDlg::FindAndReflectBestServiceCode");
}

void CEMREMChecklistDlg::OnOK()
{
	try {

		if(m_pChecklistInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2013-04-22 16:19) - PLID 56372 - if the EMN is read only, don't warn
		if(!m_bIsReadOnly) {

			//warn if no CPT code will be used
			if(m_pCodingLevelToUse == NULL) {
				if(IDNO == MessageBox("Warning: No Service Code has been approved through this checklist.\n"
					"If you continue, no new Service Code will be added to your EMN.\n\n"
					"Do you still wish to close the checklist?", "Practice", MB_ICONEXCLAMATION|MB_YESNO))
					return;
			}
			else {

				// (j.jones 2013-04-22 10:57) - PLID 54596 - We now save all rule/coding level approvals as they occur,
				// so this function no longer needs to generate pending audits.
				// We used to also add the checklist audit for the new charge, but that is now the responsibility of
				// the caller as it adds the new charge.			
			}
		}

	}NxCatchAll("Error in CEMREMChecklistDlg::OnOK");
	
	CDialog::OnOK();
}

// (j.jones 2007-08-29 16:13) - PLID 27057 - added ability to check for out-of-date details
BOOL CEMREMChecklistDlg::VerifyAllDetailsUpToDate()
{
	try {

		//Run through all details on the EMN, check to see that they are up to date.
		//If not, give the user a list of those details, and ask if they would like
		//to bring them up to date. If they decline, do not try to run element/rule
		//calculations, and explain why we are not doing so.

		if(m_pEMN) {

			//use this array to track out-of-date details
			CArray<CEMNDetail*, CEMNDetail*> aryOutOfDateDetails;

			//used later for the out-of-date warning
			CString strTop20Names;

			CString strInfoIDsToLookup;

			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			long nDetails = m_pEMN->GetTotalDetailCount();
			for (i=0; i < nDetails; i++) {
				CEMNDetail* pDetail = m_pEMN->GetDetail(i);

				//DO NOT call IsActiveInfo, because it will almost
				//assuredly run a recordset, instead see if the info is
				//valid, and if so, only then call IsActiveInfo
				if(pDetail->IsActiveInfoValid()) {
					//it is valid, but is it the active info?
					if(pDetail->IsActiveInfo()) {
						//great, it is known to be up to date, so move on
						continue;
					}
					else {
						//it's not up to date, add to our array
						aryOutOfDateDetails.Add(pDetail);

						//only add to the list of names if we are at or below 20 entries
						if(aryOutOfDateDetails.GetSize() <= 20) {
							if(!strTop20Names.IsEmpty())
								strTop20Names += "\n";
							strTop20Names += pDetail->GetLabelText();
						}
					}
				}
				else {
					//we don't know if it is up to date or not, so we need to find out from data
					if(!strInfoIDsToLookup.IsEmpty())
						strInfoIDsToLookup += ",";
					strInfoIDsToLookup += AsString(pDetail->m_nEMRInfoID);
				}
			}

			//if we have any InfoIDs to lookup, and we probably do,
			//we need to find out which InfoIDs are out of date through a recordset
			if(!strInfoIDsToLookup.IsEmpty()) {
				//CreateParamRecordset doesn't work for %s IN clauses, so we have to do it separately
				CString strSql;
				strSql.Format("SELECT ID FROM EMRInfoT "
					"WHERE ID IN (%s) AND ID NOT IN (SELECT ActiveEmrInfoID FROM EmrInfoMasterT)", strInfoIDsToLookup);
				_RecordsetPtr rs = CreateParamRecordset(strSql);

				CDWordArray aryOutOfDateEMRInfoIDs;

				while(!rs->eof) {

					long nEMRInfoID = AdoFldLong(rs, "ID");
					aryOutOfDateEMRInfoIDs.Add((DWORD)nEMRInfoID);										
					rs->MoveNext();
				}
				rs->Close();

				//now that we have our array, loop through the detail list one more time,
				//and handle them appropriately now that we truly know the IsActiveInfo() status
				//Run even if aryOutOfDateEMRInfoIDs is empty, so we can update all details.
				long nDetails = m_pEMN->GetTotalDetailCount();
				// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
				int i = 0;

				for (i=0; i < nDetails; i++) {
					CEMNDetail* pDetail = m_pEMN->GetDetail(i);

					//see if this detail uses an affected EMRInfoID
					BOOL bFound = FALSE;
					for(int j=0; j<aryOutOfDateEMRInfoIDs.GetSize() && !bFound;j++) {
						if(pDetail->m_nEMRInfoID == (long)(aryOutOfDateEMRInfoIDs.GetAt(j))) {
							//found it in our list
							bFound = TRUE;

							//update its active info to be FALSE
							pDetail->SetIsActiveInfo(FALSE);

							//and since it is not up to date, add to our array
							aryOutOfDateDetails.Add(pDetail);

							//only add to the list of names if we are at or below 20 entries
							if(aryOutOfDateDetails.GetSize() <= 20) {
								if(!strTop20Names.IsEmpty())
									strTop20Names += "\n";
								strTop20Names += pDetail->GetLabelText();
							}
						}
					}

					if(!bFound && !pDetail->IsActiveInfoValid()) {
						//if it wasn't found, and the active info status
						//was not previously known, we know now that it
						//IS active, so store that knowledge
						pDetail->SetIsActiveInfo(TRUE);
					}
				}
			}

			//if every detail is up to date, we're good to go
			if(aryOutOfDateDetails.GetSize() == 0)
				return TRUE;

			//Otherwise, we need to warn them which details are out of date.
			//We cut off the detail list at 20 names. If they want to change that
			//many en masse, they aren't going to care to see the full list.

			CString strWarning;
			// (j.jones 2010-11-12 10:51) - PLID 29075 - added warning that bringing up to date will save the EMN
			strWarning.Format("There are %li details on this EMN that are out of date, as they have been changed in Administrator since being added to this patient's EMN.\n"
				"The E/M Checklist will not attempt to calculate rules based on E/M coding elements unless all EMN details are up to date.\n\n"
				"%s:\n\n%s\n\n"
				"Would you like to bring these details up to date now? (You can also do this manually per detail by right-clicking and selecting 'Bring Item Up To Date'.) \n"
				"Bringing items up to date will also immediately save the EMN.",
				aryOutOfDateDetails.GetSize(),
				aryOutOfDateDetails.GetSize() > 20 ? "The following details are a sampling of those that are out of date" : "The following details are out of date",
				strTop20Names);
			
			//warn once to explain that they need to update details,
			//and then warn again if they agree to do so
			// (j.jones 2010-11-12 10:51) - PLID 29075 - added warning, again, that bringing up to date will save the EMN
			if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)
				|| IDNO == MessageBox("Bringing details up to date will apply any Administrative edits (for example, adding or removing options to a list item) to that detail.\n"
				"These changes will take effect immediately and cannot be undone, and may result in changes to the value of these details "
				"(if, for example, a checkbox which is selected on a detail does not exist in the updated version of the detail). "
				"In addition, the EMN will be saved immediately after bringing up to date.\r\n\r\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

				//they declined, so remind them again that we won't try to auto-calculate anything
				AfxMessageBox("The E/M Checklist will not attempt to calculate rules based on E/M coding elements until all EMN details are up to date.\n"
					"You will need to manually override each rule in order to select a service code.");
				return FALSE;
			}
			
			//ok, let's try to bring the items up to date
			
			CString strRecordsets;

			//First, figure out what the new InfoID is for each item, but we need to try to do so en masse
			for (i=0; i < aryOutOfDateDetails.GetSize(); i++) {
				CEMNDetail* pDetail = (CEMNDetail*)(aryOutOfDateDetails.GetAt(i));

				CString str;
				//can't parameterize when we're adding in bulk like this
				str.Format("SELECT %li AS OldInfoID, ActiveEMRInfoID FROM EMRInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ID = EmrInfoT.EmrInfoMasterID WHERE EmrInfoT.ID = %li\r\n", pDetail->m_nEMRInfoID, pDetail->m_nEMRInfoID);
				strRecordsets += str;
			}

			//now let's get those recordsets and find those new IDs,
			//store those IDs, and build a batch update statement

			BOOL bForceSaveEMN = FALSE;

			CMap<long, long, long, long> mapInfoIDs;
			CString strSqlBatch = BeginSqlBatch();

			_RecordsetPtr rs = CreateParamRecordset(strRecordsets);
			for (i=0; i < aryOutOfDateDetails.GetSize(); i++) {
				CEMNDetail* pDetail = (CEMNDetail*)(aryOutOfDateDetails.GetAt(i));

				if(!rs->eof) {

					long nOldInfoID = AdoFldLong(rs, "OldInfoID");
					long nNewInfoID = AdoFldLong(rs, "ActiveEMRInfoID");

					//store the ID
					mapInfoIDs.SetAt(nOldInfoID, nNewInfoID);

					//ensure we're on the right detail record, as we assume we are
					ASSERT(pDetail->m_nEMRInfoID == nOldInfoID);

					if(pDetail->m_nEMRDetailID != -1) {
						AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrDetailsT SET EmrInfoID = %li WHERE ID = %li", nNewInfoID, pDetail->m_nEMRDetailID);
						
						// (j.jones 2010-11-11 17:26) - PLID 29075 - force the EMN to save
						bForceSaveEMN = TRUE;
					}
				}

				//if this is the last iteration, don't get the next recordset
				if(aryOutOfDateDetails.GetSize() != i + 1)
					rs = rs->NextRecordset(NULL);
			}

			//now update the data
			// (c.haag 2009-08-17 10:39) - PLID 35246 - The batch and UpdateInfoID will now go in the same transaction
			{
				CSqlTransaction sqlTrans("CEMREMChecklistDlg_VerifyAllDetailsUpToDate");
				sqlTrans.Begin();

				if(!strSqlBatch.IsEmpty()) {
					ExecuteSqlStd(strSqlBatch);
				}

				//and tell the EMN to update each detail, which means yet another loop
				for (i=0; i < aryOutOfDateDetails.GetSize(); i++) {
					CEMNDetail* pDetail = (CEMNDetail*)(aryOutOfDateDetails.GetAt(i));

					long nNewInfoID = -1;
					mapInfoIDs.Lookup(pDetail->m_nEMRInfoID, nNewInfoID);
					if(nNewInfoID == -1) {
						//should be impossible
						ASSERT(FALSE);
						continue;
					}

					pDetail->UpdateInfoID(pDetail->m_nEMRInfoID, nNewInfoID, NULL, pDetail->m_nEMRDetailID);
					pDetail->SetNeedContentReload();
					pDetail->SetNeedSyncContentAndState();
					//Now, post a message that will call PostEmrItemEntryDlgSaved(), see Chris's note in OpenEmrItemEntryDlg()
					// for why we don't call it directly.
					GetMainFrame()->PostMessage(NXM_EMRITEMADVDLG_ITEM_SAVED, (WPARAM)pDetail);
					//Tell it that it's now up to date.
					pDetail->RefreshIsActiveInfo();
				}

				// Commit the transaction
				sqlTrans.Commit();

				// (j.jones 2010-11-11 17:26) - PLID 29075 - force the EMN to save
				if(bForceSaveEMN) {
					// We used to call CEmrTreeWnd::SaveEMR here. However, that has all sorts of possible prompts
					// that allow the user to potentially cancel the save and if that happens then this detail
					// has bad data that may result in data being lost. Let's call the save function directly
					// to make it as likely as possible that this save happens to ensure data integrity. If the
					// EMN has other changes then it will still require a full save at some point and if it doesn't
					// have changes then we should be fine anyway.
					SaveEMRObject(esotEMN, (long)m_pEMN, TRUE);
				}
			}
		}

		return TRUE;

	}NxCatchAll("Error in CEMREMChecklistDlg::VerifyAllDetailsUpToDate");

	return FALSE;
}

// (b.spivey, February 28, 2012) - PLID 38409 - Function that uses existing logic to approve the cell 
void CEMREMChecklistDlg::ApproveCell(IRowSettingsPtr pRow, ChecklistElementRuleInfo* &pRuleInfo) 
{
	// (j.jones 2013-04-22 11:00) - PLID 54596 - save & audit immediately
	if(!pRuleInfo->bApproved) {
		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
		if(pApi == NULL) {
			ThrowNxException("Could not save rule changes due to an invalid API.");
		}
		pApi->SetEMChecklistRuleApproval(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()), (LPCTSTR)AsString(pRuleInfo->nID), VARIANT_TRUE, VARIANT_FALSE);
	}

	//grab the approved status and continue only if they approved the dialog
	pRuleInfo->bApproved = TRUE; 

	//store the time this occurred
	pRuleInfo->dtApproved = COleDateTime::GetCurrentTime();

	//and the user that did this
	pRuleInfo->nApprovalUserID = GetCurrentUserID();
	pRuleInfo->strApprovalUserName = GetCurrentUserName();

	pRow->PutValue(pRuleInfo->pColumnInfo->nCheckColumnIndex, g_cvarTrue);

	//update the cell color
	if(pRuleInfo->bPassed) {
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);
	}
	else {
		//not flagged as passed, thus this was forced complete
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_FORCED_COMPLETE);
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_FORCED_COMPLETE);
	}
}

// (b.spivey, February 28, 2012) - PLID 38409 - Function to unapprove the cell. 
void CEMREMChecklistDlg::UnapproveCell(IRowSettingsPtr pRow, ChecklistElementRuleInfo* &pRuleInfo) 
{
	// (j.jones 2013-04-22 11:00) - PLID 54596 - save & audit immediately
	if(pRuleInfo->bApproved) {
		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
		if(pApi == NULL) {
			ThrowNxException("Could not save rule changes due to an invalid API.");
		}
		pApi->SetEMChecklistRuleApproval(GetAPISubkey(), GetAPILoginToken(), (LPCTSTR)AsString(m_pEMN->GetID()), (LPCTSTR)AsString(pRuleInfo->nID), VARIANT_FALSE, VARIANT_FALSE);
	}

	//if they cancelled, don't approve the rule
	pRuleInfo->bApproved = FALSE;
	pRuleInfo->dtApproved.SetStatus(COleDateTime::invalid);
	pRuleInfo->nApprovalUserID = -1;
	pRuleInfo->strApprovalUserName = "";

	_variant_t g_cvarFalse(VARIANT_FALSE, VT_BOOL); 
	pRow->PutValue(pRuleInfo->pColumnInfo->nCheckColumnIndex, g_cvarFalse);

	//update the cell color
	if(pRuleInfo->bPassed) {
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_AUTO_COMPLETE);
	}
	else {
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nColumnIndex, CELL_BKG_COLOR_INCOMPLETE);
		pRow->PutCellBackColor(pRuleInfo->pColumnInfo->nCheckColumnIndex, CELL_BKG_COLOR_INCOMPLETE);
	}
}

// (b.spivey, February 28, 2012) - PLID 38409 - if they click a cell in the checklist, they should get the
//		approval dialog if they want to change the approval or review how the rule is being satisfied. 
void CEMREMChecklistDlg::LeftClickEmChecklist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			return;
		}

		// (b.spivey, February 28, 2012) - PLID 38409 - Only check on rule descriptions. 
		if(nCol >= clfcCodingLevelCheckbox) {
			//we need to find what column this is
			// (b.spivey, February 28, 2012) - PLID 38409 - This is the border column. We just return on this. 
			IColumnSettingsPtr pCol = m_Checklist->GetColumn(nCol); 
			if(pCol->GetStoredWidth() == 1){
				return;
			}
			// (b.spivey, February 28, 2012) - PLID 38409 - The checkbox column is always to the right of the description column. 
			ChecklistColumnInfo *pColInfo = FindColumnInfoObjectByCheckboxColIndex(nCol+1);
			if(pColInfo == NULL) {
				//We got to a border column or something we can't get a checklist column from. Can't do anything here. 
				return;
			}
			
			// (b.spivey, February 28, 2012) - PLID 38409 - Follow the same logic as the OnEditing events.
			ChecklistElementRuleInfo *pRuleInfo = FindElementRuleInfoObject(lpRow, pColInfo);
			if(pRuleInfo == NULL) {
				//we shouldn't have allowed them into the checklist dialog
				//if there isn't a rule for every cell
				ASSERT(FALSE);
				return;
			}

			// (b.spivey, February 28, 2012) - PLID 38409 - Launch the approval dialog 
			CEMREMChecklistApprovalDlg dlg(this); 

			dlg.m_pRuleInfo = pRuleInfo;
			dlg.m_aryTrackedCategories = &m_aryTrackedCategories;
			// (j.jones 2013-04-23 16:30) - PLID 56372 - pass in our read only status
			dlg.m_bIsReadOnly = m_bIsReadOnly;

			bool bApproved = false;
			int nRet = dlg.DoModal();

			// (j.jones 2013-04-23 16:30) - PLID 56372 - if read only, do nothing
			if(m_bIsReadOnly) {
				return;
			}
			
			if(nRet == IDCANCEL) {
				//if they cancelled, don't approve the rule
				UnapproveCell(pRow, pRuleInfo); 
				bApproved = false;
			}
			else {
				//They approved. 
				ApproveCell(pRow, pRuleInfo); 
				bApproved = true;
			}

			// (j.jones 2013-04-22 11:00) - PLID 54596 - if this change causes an approved coding level
			// to no longer be approved, it will save & audit immediately
			UpdateCodingLevelStatus(pRuleInfo->pRowInfo);

			FindAndReflectBestServiceCode(); 
		}	
	}NxCatchAll(__FUNCTION__);
}