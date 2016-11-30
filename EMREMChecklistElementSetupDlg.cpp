// EMREMChecklistElementSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMChecklistElementSetupDlg.h"
#include "EMREMCodeCategorySetupDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SingleSelectDlg.h"
#include "AuditTrail.h"

// (j.jones 2007-08-17 15:31) - PLID 27104 - created

// (j.jones 2013-01-04 14:48) - PLID 28135 - changed all references to say E/M, and not use an ampersand

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



enum ElementDetailListColumn {

	edlcID = 0,
	edlcObjectPtr,
	edlcMinElements,
	edlcCategoryID,
};

enum SavedObjectType {

	sotRule = 1,
	sotRuleDetail,
};

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistElementSetupDlg dialog


CEMREMChecklistElementSetupDlg::CEMREMChecklistElementSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMChecklistElementSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMChecklistElementSetupDlg)
		m_pParentRuleInfo = NULL;
		m_pLocalRuleInfo = NULL;
		m_pChecklist = NULL;
		m_bIsPreLoaded = FALSE;
		m_bShowDescriptionChangedWarning = FALSE;
		m_strCurGeneratedRuleDesc = "";
	//}}AFX_DATA_INIT
}

CEMREMChecklistElementSetupDlg::~CEMREMChecklistElementSetupDlg()
{
	//we are only responsible for clearing the local rule info pointer
	if(m_pLocalRuleInfo) {

		//this will clear and deallocate the details, but not deallocate the info pointer
		ClearRuleContents(m_pLocalRuleInfo);

		delete m_pLocalRuleInfo;
		
		m_pLocalRuleInfo = NULL;
	}
}

void CEMREMChecklistElementSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMChecklistElementSetupDlg)
	DDX_Control(pDX, IDC_RADIO_ALL_DETAILS, m_btnAllDetails);
	DDX_Control(pDX, IDC_RADIO_ANY_DETAILS, m_btnAnyDetails);
	DDX_Control(pDX, IDC_CHECK_ELEMENT_APPROVED, m_btnApproved);
	DDX_Control(pDX, IDC_BTN_EDIT_EM_CATS, m_btnEditEMCategories);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_DELETE_ELEMENT_RULE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_ELEMENT_RULE, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_RULE_DESCRIPTION, m_nxeditEditRuleDescription);
	DDX_Control(pDX, IDC_CATEGORY_COMPLETION_LABEL, m_nxstaticCategoryCompletionLabel);
	DDX_Control(pDX, IDC_RULES_PREFILLED_LABEL, m_nxstaticRulesPrefilledLabel);
	DDX_Control(pDX, IDC_DESCRIPTION_CHANGED_LABEL, m_nxstaticDescriptionChangedLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMChecklistElementSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMChecklistElementSetupDlg)
	ON_EN_CHANGE(IDC_EDIT_RULE_DESCRIPTION, OnChangeEditRuleDescription)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_ELEMENT_RULE, OnBtnAddNewElementRule)
	ON_BN_CLICKED(IDC_BTN_DELETE_ELEMENT_RULE, OnBtnDeleteElementRule)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_EDIT_EM_CATS, OnBtnEditEmCats)
	ON_BN_CLICKED(IDC_CHECK_ELEMENT_APPROVED, OnCheckElementApproved)
	ON_BN_CLICKED(IDC_RADIO_ALL_DETAILS, OnRadioAllDetails)
	ON_BN_CLICKED(IDC_RADIO_ANY_DETAILS, OnRadioAnyDetails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistElementSetupDlg message handlers

BOOL CEMREMChecklistElementSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (c.haag 2008-04-29 17:02) - PLID 29837 - NxIconify additional buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnEditEMCategories.AutoSet(NXB_MODIFY);

		m_DetailList = BindNxDataList2Ctrl(this, IDC_CHECKLIST_ELEMENT_RULE_LIST, GetRemoteData(), false);

		//have to assign the combosource here so the non-requerying datalist will load this data
		m_DetailList->GetColumn(edlcCategoryID)->ComboSource = _bstr_t("SELECT ID, Name FROM EMCodeCategoryT ORDER BY Name");

		//fill out the header label
		CString str;
		str.Format("The following rules must be satisfied for the cell to be marked complete\n"
			"for the %s column and %s coding level.",
			m_pParentRuleInfo ? m_pParentRuleInfo->pColumnInfo->strName : "current",
			m_pParentRuleInfo ? m_pParentRuleInfo->pRowInfo->strCodeNumber : "current");
		SetDlgItemText(IDC_CATEGORY_COMPLETION_LABEL, str);

		//copy our passed-in parent rule into the local rule, such that we only make changes locally
		CopyRuleInfo(m_pParentRuleInfo, m_pLocalRuleInfo);

		//now load details from the local rule
		if(m_pLocalRuleInfo) {

			//fill the detail list
			for(int i=0;i<m_pLocalRuleInfo->paryDetails.GetSize(); i++) {

				ChecklistElementRuleDetailInfo *pInfo = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));
				//don't display deleted details
				if(!pInfo->bDeleted) {
					//create a new row
					IRowSettingsPtr pRow = m_DetailList->GetNewRow();
					pRow->PutValue(edlcID, (long)pInfo->nID);
					pRow->PutValue(edlcObjectPtr, (long)pInfo);
					pRow->PutValue(edlcMinElements, (long)pInfo->nMinElements);
					pRow->PutValue(edlcCategoryID, (long)pInfo->nCategoryID);
					m_DetailList->AddRowAtEnd(pRow, NULL);
				}
			}


			// (j.jones 2007-09-18 14:52) - PLID 27397 - load bRequireAllDetails
			if(m_pLocalRuleInfo->bRequireAllDetails)
				CheckDlgButton(IDC_RADIO_ALL_DETAILS, TRUE);
			else
				CheckDlgButton(IDC_RADIO_ANY_DETAILS, TRUE);

			//if we don't have more than one detail, disable the radio buttons
			if(m_DetailList->GetRowCount() <= 1) {
				GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(FALSE);
				GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(FALSE);
			}

			//set the description
			SetDlgItemText(IDC_EDIT_RULE_DESCRIPTION,m_pLocalRuleInfo->strDescription);

			//fill in the approved status
			if(m_pLocalRuleInfo->bApproved) {
				
				CheckDlgButton(IDC_CHECK_ELEMENT_APPROVED, TRUE);
				
				CString strApproved;
				strApproved.Format("Approved By %s on %s", m_pLocalRuleInfo->strApprovalUserName, FormatDateTimeForInterface(m_pLocalRuleInfo->dtApproved, NULL, dtoDateTime));

				SetDlgItemText(IDC_CHECK_ELEMENT_APPROVED, strApproved);
			}
			else {
				CheckDlgButton(IDC_CHECK_ELEMENT_APPROVED, FALSE);
				SetDlgItemText(IDC_CHECK_ELEMENT_APPROVED, "Approved");
			}
		}

		//if the pre-loaded flag is set, indicate as such on the screen
		if(m_bIsPreLoaded) {
			//show the label, as it is hidden by default
			GetDlgItem(IDC_RULES_PREFILLED_LABEL)->ShowWindow(SW_SHOW);
			
			//OnCtlColor will color it red
		}

		//calculate our description, compare to the displayed text,
		//and show the warning label if different
		CheckShowDescriptionChangedWarning(TRUE);

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMREMChecklistElementSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMREMChecklistElementSetupDlg)
	ON_EVENT(CEMREMChecklistElementSetupDlg, IDC_CHECKLIST_ELEMENT_RULE_LIST, 9 /* EditingFinishing */, OnEditingFinishingChecklistElementRuleList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMREMChecklistElementSetupDlg, IDC_CHECKLIST_ELEMENT_RULE_LIST, 10 /* EditingFinished */, OnEditingFinishedChecklistElementRuleList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMREMChecklistElementSetupDlg::OnEditingFinishingChecklistElementRuleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;

		if(nCol == edlcMinElements) {

			long nMinElements = 0;

			if(pvarNewValue->vt == VT_I4) {
				nMinElements = VarLong(pvarNewValue,0);
			}
			
			if(nMinElements <= 0) {
				//invalid data
				MessageBox("You must enter in a value greater than zero.", "Practice", MB_OK | MB_ICONERROR);
				*pbCommit = FALSE;
				return;
			}
		}
		else if(nCol == edlcCategoryID) {

			long nCategoryID = -1;

			if(pvarNewValue->vt == VT_I4) {
				nCategoryID = VarLong(pvarNewValue, -1);
			}
			
			if(nCategoryID == -1) {
				//invalid data
				MessageBox("You must select an E/M category.", "Practice", MB_OK | MB_ICONERROR);
				*pbCommit = FALSE;
				return;
			}
			else {
				//disallow selecting the same category for multiple details
				IRowSettingsPtr pRowToCheck = m_DetailList->GetFirstRow();
				while(pRowToCheck) {

					if(pRowToCheck != pRow) {

						long nCategoryToCheck = VarLong(pRowToCheck->GetValue(edlcCategoryID), -1);
						if(nCategoryToCheck == nCategoryID) {
							//another detail uses this category
							MessageBox("Another rule is using this category. Two rules cannot use the same category.", "Practice", MB_OK | MB_ICONERROR);
							*pbCommit = FALSE;
							return;
						}
					}

					pRowToCheck = pRowToCheck->GetNextRow();
				}
			}
		}

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnEditingFinishingChecklistElementRuleList");
}

void CEMREMChecklistElementSetupDlg::OnEditingFinishedChecklistElementRuleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL || !bCommit)
			return;

		//figure out which detail this row corresponds to
		ChecklistElementRuleDetailInfo *pInfo = NULL;
		_variant_t var = pRow->GetValue(edlcObjectPtr);
		if(var.vt == VT_I4)
			pInfo = (ChecklistElementRuleDetailInfo*)VarLong(var);
		
		if(pInfo == NULL) {
			//this should not be possible
			ASSERT(FALSE);
			return;
		}

		if(nCol == edlcMinElements) {

			//OnEditingFinishing should make it impossible for the value not to be a long
			if(varNewValue.vt != VT_I4) {
				ASSERT(FALSE);
				return;
			}

			//update our memory object with the change
			pInfo->nMinElements = VarLong(varNewValue);

			//since we've made a change, hide the pre-loaded label if it exists
			HidePreLoadedLabel();

			//update our description
			RecalculateAndApplyNewDescription();

			//undo the approval
			CheckUndoApproval();
		}
		else if(nCol == edlcCategoryID) {

			//OnEditingFinishing should make it impossible for the value not to be a long
			if(varNewValue.vt != VT_I4) {
				ASSERT(FALSE);
				return;
			}

			//update our memory object with the change
			pInfo->nCategoryID = VarLong(varNewValue);

			pInfo->strCategoryName = VarString(pRow->GetOutputValue(edlcCategoryID));

			//since we've made a change, hide the pre-loaded label if it exists
			HidePreLoadedLabel();

			//update our description
			RecalculateAndApplyNewDescription();

			//undo the approval
			CheckUndoApproval();
		}

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnEditingFinishedChecklistElementRuleList");
}

void CEMREMChecklistElementSetupDlg::OnChangeEditRuleDescription() 
{
	try {

		//since we've made a change, hide the pre-loaded label if it exists
		HidePreLoadedLabel();

		//likely we'll need to show the warning, so check and see
		CheckShowDescriptionChangedWarning(FALSE);

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnChangeEditRuleDescription");
}

void CEMREMChecklistElementSetupDlg::OnOK() 
{
	try {

		//get the description
		GetDlgItemText(IDC_EDIT_RULE_DESCRIPTION, m_pLocalRuleInfo->strDescription);

		// (j.jones 2007-09-18 14:52) - PLID 27397 - get bRequireAllDetails
		m_pLocalRuleInfo->bRequireAllDetails = IsDlgButtonChecked(IDC_RADIO_ALL_DETAILS);

		//save to data
		if(!Save()) {
			return;
		}

		//now copy our local rule into the parent rule, overwriting it with our changes
		CopyRuleInfo(m_pLocalRuleInfo, m_pParentRuleInfo);
	
		CDialog::OnOK();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnOK");
}

BOOL CEMREMChecklistElementSetupDlg::Save()
{
	long nAuditTransactionID = -1;

	try {

		if(m_pChecklist == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return FALSE;
		}

		//validate the data

		//require a description
		CString strDesc = m_pLocalRuleInfo->strDescription;
		strDesc.TrimLeft();
		strDesc.TrimRight();
		if(strDesc.IsEmpty()) {
			MessageBox("You must enter a description for this rule.", "Practice", MB_OK | MB_ICONERROR);
			return FALSE;

		}

		/* // (j.jones 2007-08-24 15:57) - PLID 27104 - we DO allow saving rules without rule details
		//require at least one rule detail
		if(m_DetailList->GetRowCount() == 0) {
			MessageBox("You must have at least one rule detail entered in order to save this setup.", "Practice", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		*/

		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;

		//require at least 1 element required for rule details
		for(i=0; i<m_pLocalRuleInfo->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));

			if(pDetailInfo->bDeleted)
				continue;

			if(!pDetailInfo->bDeleted && pDetailInfo->nMinElements <= 0) {
				MessageBox("At least one rule has less than zero elements required. Please correct this before saving.", "Practice", MB_OK | MB_ICONERROR);
				return FALSE;
			}

			//double-check that they do not have multiple details for the same category
			for(int j=0; j<m_pLocalRuleInfo->paryDetails.GetSize(); j++) {
				ChecklistElementRuleDetailInfo* pDetailInfoToCompare = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(j));

				//compare the detail to every other detail in the list
				if(!pDetailInfoToCompare->bDeleted
					&& pDetailInfo != pDetailInfoToCompare
					&& pDetailInfo->nCategoryID == pDetailInfoToCompare->nCategoryID) {

					//two details have the same category ID. Bad!
					CString strWarning;
					strWarning.Format("Multiple rule details are referencing the %s category. You may only have one detail per category.", pDetailInfo->strCategoryName);
					MessageBox(strWarning, "Practice", MB_OK | MB_ICONERROR);
					return FALSE;
				}
			}
		}

		//if not approved, warn that it needs to be, but allow saving
		if(!m_pLocalRuleInfo->bApproved
			&& IDNO == MessageBox("This rule must be marked as approved before the checklist can be used on a patient's EMN.\n"
			"Are you sure you wish to save this unapproved rule?","Practice",MB_ICONQUESTION|MB_YESNO))
			return FALSE;

		//save the rule to data

		CString strSqlBatch = BeginSqlBatch();

		CString strApprovedBy = "NULL";
		CString strApprovedDate = "NULL";
		if(m_pLocalRuleInfo->bApproved) {
			strApprovedBy.Format("%li", m_pLocalRuleInfo->nApprovalUserID);
			strApprovedDate.Format("'%s'", FormatDateTimeForSql(m_pLocalRuleInfo->dtApproved));
		}

		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nRuleID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nRuleDetailID INT");

		//used to track newly created IDs
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewObjectsT TABLE (ID INT, Type INT, ObjectPtr INT)");
		
		//save the rule
		if(m_pLocalRuleInfo->nID == -1) {
			//save new rule					

			AddStatementToSqlBatch(strSqlBatch, "SET @nRuleID = (SELECT COALESCE(MAX(ID),0) + 1 AS NewID FROM EMChecklistRulesT)");

			// (j.jones 2007-09-18 14:15) - PLID 27397 - added RequireAllDetails
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMChecklistRulesT (ID, ChecklistID, ColumnID, CodingLevelID, Description, "
				"Approved, ApprovedBy, ApprovedDate, RequireAllDetails) "
				"VALUES (@nRuleID, %li, %li, %li, '%s', %li, %s, %s, %li)", m_pChecklist->nID,
				m_pLocalRuleInfo->pColumnInfo->nID, m_pLocalRuleInfo->pRowInfo->nID, _Q(m_pLocalRuleInfo->strDescription),
				m_pLocalRuleInfo->bApproved ? 1 : 0, strApprovedBy, strApprovedDate, m_pLocalRuleInfo->bRequireAllDetails ? 1 : 0);

			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewObjectsT (ID, Type, ObjectPtr) VALUES (@nRuleID, %li, %li)", sotRule, (long)m_pLocalRuleInfo);

			//audit the creation

			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();

			//replace newlines with spaces
			CString strNewValue = m_pLocalRuleInfo->strDescription;
			strNewValue.Replace("\r\n", "  ");

			AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleCreated, m_pChecklist->nID, "", strNewValue, aepMedium, aetCreated);
		}
		else {
			//update existing rule

			AddStatementToSqlBatch(strSqlBatch, "SET @nRuleID = %li", m_pLocalRuleInfo->nID);
			
			// (j.jones 2007-09-18 14:15) - PLID 27397 - added RequireAllDetails
			AddStatementToSqlBatch(strSqlBatch, "UPDATE EMChecklistRulesT SET Description = '%s', "
				"Approved = %li, ApprovedBy = %s, ApprovedDate = %s, RequireAllDetails = %li "
				"WHERE ID = @nRuleID", _Q(m_pLocalRuleInfo->strDescription),
				m_pLocalRuleInfo->bApproved ? 1 : 0, strApprovedBy, strApprovedDate, m_pLocalRuleInfo->bRequireAllDetails ? 1 : 0);

			//audit changes

			if(m_pLocalRuleInfo->strDescription != m_pParentRuleInfo->strDescription) {

				//changed the description

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldValue = m_pParentRuleInfo->strDescription;
				strOldValue.Replace("\r\n", "  ");
				CString strNewValue = m_pLocalRuleInfo->strDescription;
				strNewValue.Replace("\r\n", "  ");

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDesc, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			// (j.jones 2007-09-18 14:15) - PLID 27397 - audit RequireAllDetails
			if(m_pLocalRuleInfo->bRequireAllDetails != m_pParentRuleInfo->bRequireAllDetails) {

				//changed the description

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldDesc = m_pParentRuleInfo->strDescription;
				strOldDesc.Replace("\r\n", "  ");
				CString strNewDesc = m_pLocalRuleInfo->strDescription;
				strNewDesc.Replace("\r\n", "  ");

				CString strOldValue, strNewValue;
				if(m_pParentRuleInfo->bRequireAllDetails)
					strOldValue.Format("'%s' - requires all details", strOldDesc);
				else
					strOldValue.Format("'%s' - requires any detail", strOldDesc);
				if(m_pLocalRuleInfo->bRequireAllDetails)
					strNewValue.Format("'%s' - requires all details", strNewDesc);
				else
					strNewValue.Format("'%s' - requires any detail", strNewDesc);

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleAnyAll, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			if(m_pLocalRuleInfo->bApproved != m_pParentRuleInfo->bApproved
				|| m_pLocalRuleInfo->nApprovalUserID != m_pParentRuleInfo->nApprovalUserID
				|| m_pLocalRuleInfo->dtApproved != m_pParentRuleInfo->dtApproved) {

				//approval information changed

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldDesc = m_pParentRuleInfo->strDescription;
				strOldDesc.Replace("\r\n", "  ");
				CString strNewDesc = m_pLocalRuleInfo->strDescription;
				strNewDesc.Replace("\r\n", "  ");

				CString strOldValue, strNewValue;
				if(!m_pParentRuleInfo->bApproved) {
					strOldValue.Format("Not Approved (%s)", strOldDesc);
				}
				else {
					strOldValue.Format("Approved by %s on %s (%s)", m_pParentRuleInfo->strApprovalUserName, FormatDateTimeForInterface(m_pParentRuleInfo->dtApproved, NULL, dtoDateTime), strOldDesc);
				}
				if(!m_pLocalRuleInfo->bApproved) {
					strNewValue.Format("Not Approved (%s)", strNewDesc);
				}
				else {
					strNewValue.Format("Approved by %s on %s (%s)", m_pLocalRuleInfo->strApprovalUserName, FormatDateTimeForInterface(m_pLocalRuleInfo->dtApproved, NULL, dtoDateTime), strNewDesc);
				}

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleApproved, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
		}

		//save the details
		for(i=0; i<m_pLocalRuleInfo->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));
			
			if(pDetailInfo->nID == -1) {
				//save new detail

				AddStatementToSqlBatch(strSqlBatch, "SET @nRuleDetailID = (SELECT COALESCE(MAX(ID),0) + 1 AS NewID FROM EMChecklistRuleDetailsT)");

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMChecklistRuleDetailsT (ID, RuleID, CategoryID, MinElements) "
					"VALUES (@nRuleDetailID, @nRuleID, %li, %li)", pDetailInfo->nCategoryID, pDetailInfo->nMinElements);

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewObjectsT (ID, Type, ObjectPtr) VALUES (@nRuleDetailID, %li, %li)", sotRuleDetail, (long)pDetailInfo);

				//audit the creation

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				CString strNewValue;
				strNewValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailCreated, m_pChecklist->nID, "", strNewValue, aepMedium, aetCreated);
			}
			else {
				//existing detail

				if(pDetailInfo->bDeleted) {
					//delete detail

					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMChecklistRuleDetailsT WHERE ID = %li", pDetailInfo->nID);

					//audit

					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();

					CString strOldValue;
					strOldValue.Format("Category: %s, Elements Required: %li", pDetailInfo->strCategoryName, pDetailInfo->nMinElements);
					AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailDeleted, m_pChecklist->nID, strOldValue, "Deleted", aepMedium, aetDeleted);
				}
				else {
					//update existing detail

					AddStatementToSqlBatch(strSqlBatch, "UPDATE EMChecklistRuleDetailsT SET "
						"CategoryID = %li, MinElements = %li WHERE ID = %li",
						pDetailInfo->nCategoryID, pDetailInfo->nMinElements, pDetailInfo->nID);

					//have to find the same detail in the parent info struct
					BOOL bFound = FALSE;
					for(int j=0; j<m_pParentRuleInfo->paryDetails.GetSize() && !bFound;j++) {
						ChecklistElementRuleDetailInfo* pOldDetailInfo = (ChecklistElementRuleDetailInfo*)(m_pParentRuleInfo->paryDetails.GetAt(j));

						if(pOldDetailInfo->nID == pDetailInfo->nID) {
							//found it!
							bFound = TRUE;

							//audit changes

							if(pDetailInfo->nCategoryID != pOldDetailInfo->nCategoryID) {

								//category changed

								if(nAuditTransactionID == -1)
									nAuditTransactionID = BeginAuditTransaction();

								CString strOldValue, strNewValue;
								strOldValue.Format("Category: %s", pOldDetailInfo->strCategoryName);
								strNewValue.Format("Category: %s", pDetailInfo->strCategoryName);

								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailCategory, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
							}

							if(pDetailInfo->nMinElements != pOldDetailInfo->nMinElements) {

								//min. elements changed

								if(nAuditTransactionID == -1)
									nAuditTransactionID = BeginAuditTransaction();

								CString strOldValue, strNewValue;
								strOldValue.Format("Elements Required: %li", pOldDetailInfo->nMinElements);
								strNewValue.Format("Elements Required: %li", pDetailInfo->nMinElements);
								
								AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistRuleDetailMinElements, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
							}
						}
					}
				}
			}
		}

		if(!strSqlBatch.IsEmpty()) {

			//commit the changes
			
			CString strRecordset;
			strRecordset.Format(
					"SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"%s "
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT ID, Type, ObjectPtr FROM @NewObjectsT ",
					strSqlBatch);

			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushPerformanceWarningLimit pmr(-1);
			_RecordsetPtr prsResults = CreateRecordsetStd(strRecordset,
				adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);

			//grab any returned IDs from @NewObjectsT and update the
			//local rule object's IDs, because this structure is being
			//passed back to the parent

			while(!prsResults->eof) {

				long nID = AdoFldLong(prsResults, "ID");
				SavedObjectType sotType = (SavedObjectType)AdoFldLong(prsResults, "Type");
				long nObject = AdoFldLong(prsResults, "ObjectPtr");

				if(sotType == sotRule && m_pLocalRuleInfo == (ChecklistElementRuleInfo*)nObject) {
					//assign the rule ID
					m_pLocalRuleInfo->nID = nID;
				}
				else if(sotType == sotRuleDetail) {
					
					//assign the ID (no need to search, we're pointing to the detail inside m_pLocalRuleInfo)
					ChecklistElementRuleDetailInfo *pInfo = (ChecklistElementRuleDetailInfo*)nObject;
					pInfo->nID = nID;
				}

				prsResults->MoveNext();
			}			
			prsResults->Close();

			// (j.jones 2007-10-01 15:33) - PLID 27104 - now we can remove deleted details from memory
			for(int i=m_pLocalRuleInfo->paryDetails.GetSize()-1;i>=0;i--) {
				ChecklistElementRuleDetailInfo* pDetailInfoToCheck = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));
				if(pDetailInfoToCheck->bDeleted) {
					delete pDetailInfoToCheck;
					m_pLocalRuleInfo->paryDetails.RemoveAt(i);
				}
			}
		}

		if(nAuditTransactionID != -1)
			CommitAuditTransaction(nAuditTransactionID);
		
		return TRUE;

	}NxCatchAllCall("Error in CEMREMChecklistElementSetupDlg::Save",
		// (j.jones 2007-10-01 12:37) - PLID 27104 - rollback the audit transaction if we have one, and had an exception
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)

	return FALSE;
}

//clear the contents of passed-in rule info, do not deallocate the rule info!
void CEMREMChecklistElementSetupDlg::ClearRuleContents(ChecklistElementRuleInfo *pRule)
{
	try {

		if(pRule) {

			//clear the fields
			pRule->nID = -1;
			pRule->pColumnInfo = NULL;
			pRule->pRowInfo = NULL;
			pRule->strDescription = "";
			// (j.jones 2007-09-18 14:48) - PLID 27397 - added RequireAllDetails
			pRule->bRequireAllDetails = TRUE;

			pRule->bApproved = FALSE;

			pRule->bPassed = FALSE; //patient EMNs only ((j.jones 2007-08-29 12:14) - PLID 27056)
					
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			pRule->dtApproved = dtInvalid;

			pRule->nApprovalUserID = -1;
			pRule->strApprovalUserName = "";

			//we DO want to deallocate the details though
			for(int i=pRule->paryDetails.GetSize()-1;i>=0;i--) {
				ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pRule->paryDetails.GetAt(i));
				delete pDetailInfo;
			}
			pRule->paryDetails.RemoveAll();
		}

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::ClearRuleContents");
}

//copies the contents of pSourceRule into pDestRule, overwriting pDestRule
void CEMREMChecklistElementSetupDlg::CopyRuleInfo(ChecklistElementRuleInfo *pSourceRule, ChecklistElementRuleInfo *&pDestRule)
{
	try {

		//first ensure pDestRule is empty
		ClearRuleContents(pDestRule);

		//if NULL, create a new rule
		if(pDestRule == NULL)
			pDestRule = new ChecklistElementRuleInfo;

		//now assign the contents of pSourceRule into pDestRule
		pDestRule->nID = pSourceRule->nID;
		pDestRule->pColumnInfo = pSourceRule->pColumnInfo;
		pDestRule->pRowInfo = pSourceRule->pRowInfo;
		pDestRule->strDescription = pSourceRule->strDescription;
		// (j.jones 2007-09-18 14:48) - PLID 27397 - added RequireAllDetails
		pDestRule->bRequireAllDetails = pSourceRule->bRequireAllDetails;
		pDestRule->bApproved = pSourceRule->bApproved;
		pDestRule->dtApproved = pSourceRule->dtApproved;
		pDestRule->nApprovalUserID = pSourceRule->nApprovalUserID;
		pDestRule->strApprovalUserName = pSourceRule->strApprovalUserName;

		pDestRule->bPassed = FALSE; //patient EMNs only ((j.jones 2007-08-29 12:14) - PLID 27056)
		
		for(int i=0; i<pSourceRule->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(pSourceRule->paryDetails.GetAt(i));
			
			//create a new rule detail
			ChecklistElementRuleDetailInfo* pNewInfo = new ChecklistElementRuleDetailInfo;
			pNewInfo->nID = pDetailInfo->nID;
			pNewInfo->nMinElements = pDetailInfo->nMinElements;
			pNewInfo->nCategoryID = pDetailInfo->nCategoryID;
			pNewInfo->strCategoryName = pDetailInfo->strCategoryName;
			pNewInfo->bDeleted = pDetailInfo->bDeleted;
			pDestRule->paryDetails.Add(pNewInfo);
		}

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::ClearRuleInfo");
}

void CEMREMChecklistElementSetupDlg::OnBtnAddNewElementRule() 
{
	try {

		//first prompt for a category

		long nCategoryID = -1;
		CString strCategoryName = "";

		BOOL bContinue = TRUE;
		while(bContinue) {
			bContinue = FALSE;

			CSingleSelectDlg dlg(this);
			// (j.jones 2013-01-04 14:48) - PLID 28135 - changed to say E/M, and not use an ampersand
			if(IDOK == dlg.Open("EMCodeCategoryT", "", "ID", "Name", "Select an E/M Category:")) {
				nCategoryID = dlg.GetSelectedID();

				// (j.jones 2007-08-23 09:20) - PLID 27148 - Created the GetSelectedDisplayValue() function,
				// which returns the variant value displayed in the combo box. In this case, it has to be
				// either VT_BSTR or VT_NULL
				strCategoryName = VarString(dlg.GetSelectedDisplayValue(), "");

				if(nCategoryID == -1) {
					//invalid data
					MessageBox("You must select an E/M category.", "Practice", MB_OK | MB_ICONERROR);
					bContinue = TRUE;
				}
				else {
					//disallow selecting the same category for multiple details
					IRowSettingsPtr pRowToCheck = m_DetailList->GetFirstRow();
					while(pRowToCheck != NULL && !bContinue) {

						long nCategoryToCheck = VarLong(pRowToCheck->GetValue(edlcCategoryID), -1);
						if(nCategoryToCheck == nCategoryID) {
							//another detail uses this category
							nCategoryID = -1;
							strCategoryName = "";
							MessageBox("Another rule is using this category. Two rules cannot use the same category.", "Practice", MB_OK | MB_ICONERROR);
							bContinue = TRUE;
						}

						pRowToCheck = pRowToCheck->GetNextRow();
					}
				}
			}
		}

		//can't continue without a category ID
		if(nCategoryID == -1)
			return;

		//create a new detail object
		ChecklistElementRuleDetailInfo *pInfo = new ChecklistElementRuleDetailInfo;
		pInfo->nID = -1;
		pInfo->nMinElements = 1;
		pInfo->nCategoryID = nCategoryID;
		pInfo->strCategoryName = strCategoryName;
		pInfo->bDeleted = FALSE;

		//add to our list
		m_pLocalRuleInfo->paryDetails.Add(pInfo);

		//create a new row
		IRowSettingsPtr pRow = m_DetailList->GetNewRow();
		pRow->PutValue(edlcID, (long)pInfo->nID);
		pRow->PutValue(edlcObjectPtr, (long)pInfo);
		pRow->PutValue(edlcMinElements, pInfo->nMinElements);
		pRow->PutValue(edlcCategoryID, pInfo->nCategoryID);
		m_DetailList->AddRowAtEnd(pRow, NULL);

		// (j.jones 2007-09-18 16:02) - PLID 27397 - update the radio buttons accordingly, if we have more than one detail
		if(m_DetailList->GetRowCount() <= 1) {
			GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(TRUE);
		}

		//since we've made a change, hide the pre-loaded label if it exists
		HidePreLoadedLabel();

		//update our description
		RecalculateAndApplyNewDescription();

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnBtnAddNewElementRule");
}

void CEMREMChecklistElementSetupDlg::OnBtnDeleteElementRule() 
{
	try {

		IRowSettingsPtr pRow = m_DetailList->GetCurSel();
		
		if(pRow == NULL) {
			MessageBox("You must first select a rule to delete.", "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		//figure out which detail this row corresponds to
		ChecklistElementRuleDetailInfo *pInfo = NULL;
		_variant_t var = pRow->GetValue(edlcObjectPtr);
		if(var.vt == VT_I4)
			pInfo = (ChecklistElementRuleDetailInfo*)VarLong(var);
		
		if(pInfo == NULL) {
			//this should not be possible
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2007-10-01 15:30) - PLID 27104 - if an existing detail, mark as deleted
		if(pInfo->nID != -1) {
			pInfo->bDeleted = TRUE;
		}
		else {
			//otherwise remove the detail from memory
			BOOL bFound = FALSE;
			for(int i=m_pLocalRuleInfo->paryDetails.GetSize()-1;i>=0 && !bFound;i--) {
				ChecklistElementRuleDetailInfo* pDetailInfoToCheck = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));
				if(pDetailInfoToCheck == pInfo) {
					bFound = TRUE;
					delete pDetailInfoToCheck;
					m_pLocalRuleInfo->paryDetails.RemoveAt(i);
				}
			}
		}

		//and remove the row
		m_DetailList->RemoveRow(pRow);

		// (j.jones 2007-09-18 16:02) - PLID 27397 - if we don't have more than one detail, disable the radio buttons
		if(m_DetailList->GetRowCount() <= 1) {
			GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(TRUE);
		}

		//since we've made a change, hide the pre-loaded label if it exists
		HidePreLoadedLabel();

		//update our description
		RecalculateAndApplyNewDescription();

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnBtnDeleteElementRule");
}

HBRUSH CEMREMChecklistElementSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(m_bIsPreLoaded && pWnd->GetDlgCtrlID() == IDC_RULES_PREFILLED_LABEL)
	{
		pDC->SetTextColor(RGB(174,0,0));	//this maroon color works in 256-color TS sessions
	}
	else if(m_bShowDescriptionChangedWarning && pWnd->GetDlgCtrlID() == IDC_DESCRIPTION_CHANGED_LABEL)
	{
		pDC->SetTextColor(RGB(174,0,0));	//this maroon color works in 256-color TS sessions
	}
	
	return hbr;
}

void CEMREMChecklistElementSetupDlg::HidePreLoadedLabel()
{
	//only need to hide if the pre-loaded flag is set
	if(m_bIsPreLoaded) {
		m_bIsPreLoaded = FALSE;
		GetDlgItem(IDC_RULES_PREFILLED_LABEL)->ShowWindow(SW_HIDE);
	}
}

//generates a description of the contents of this rule
void CEMREMChecklistElementSetupDlg::GenerateRuleDescription()
{
	try {

		CString strDescription = "";

		//loop through each detail
		IRowSettingsPtr pRow = m_DetailList->GetFirstRow();
		while(pRow) {

			//grab our information
			long nMinElements = VarLong(pRow->GetValue(edlcMinElements), -1);

			CString strCategoryName = "";
			_variant_t var = pRow->GetOutputValue(edlcCategoryID);
			if(var.vt == VT_BSTR)
				strCategoryName = VarString(var);

			//calculate the detail description
			CString strDetailDesc;
			strDetailDesc.Format("%s (%li)", strCategoryName, nMinElements);

			//add to the description to return
			if(!strDescription.IsEmpty()) {

				// (j.jones 2007-09-18 16:14) - PLID 27397 - if the "use any detail" setting is in use,
				// then add the word "or" before the newline
				if(IsDlgButtonChecked(IDC_RADIO_ANY_DETAILS)) {
					strDescription += " or";
				}
				
				strDescription += "\r\n";
			}

			strDescription += strDetailDesc;

			pRow = pRow->GetNextRow();
		}

		//now assign the completed description to our stored value
		m_strCurGeneratedRuleDesc = strDescription;

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::GenerateRuleDescription");
}

//determines if we show or hide the description warning label
void CEMREMChecklistElementSetupDlg::DisplayDescriptionChangedWarning(BOOL bShow)
{
	try {

		m_bShowDescriptionChangedWarning = bShow;
		GetDlgItem(IDC_DESCRIPTION_CHANGED_LABEL)->ShowWindow(m_bShowDescriptionChangedWarning ? SW_SHOW : SW_HIDE);

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::DisplayDescriptionChangedWarning");
}

//compares our stored description to the displayed description, optionally recalculating the stored description
void CEMREMChecklistElementSetupDlg::CheckShowDescriptionChangedWarning(BOOL bRecalculateRuleDesc)
{
	try {
	
		//see if we need to recalculate the rule description
		if(bRecalculateRuleDesc)
			GenerateRuleDescription();

		CString strGetCurDisplayDesc;
		GetDlgItemText(IDC_EDIT_RULE_DESCRIPTION, strGetCurDisplayDesc);

		//show/hide the warning based on whether the calculated description matches the displayed one
		DisplayDescriptionChangedWarning(!m_strCurGeneratedRuleDesc.IsEmpty() && strGetCurDisplayDesc != m_strCurGeneratedRuleDesc);
	
	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::CheckShowDescriptionChangedWarning");
}

//potentially updates the description with our changes
void CEMREMChecklistElementSetupDlg::RecalculateAndApplyNewDescription()
{
	try {

		//first get the current display desc
		CString strGetCurDisplayDesc;
		GetDlgItemText(IDC_EDIT_RULE_DESCRIPTION, strGetCurDisplayDesc);

		BOOL bOverwrite = FALSE;

		//if the currently displayed description is blank or matches
		//our current generated description, we can replace it
		if(strGetCurDisplayDesc.IsEmpty() || strGetCurDisplayDesc == m_strCurGeneratedRuleDesc) {
			bOverwrite = TRUE;
		}

		//now recalculate the description based on the current data
		GenerateRuleDescription();

		if(bOverwrite) {
			//display on the dialog
			SetDlgItemText(IDC_EDIT_RULE_DESCRIPTION, m_strCurGeneratedRuleDesc);
		}

		//now hide the warning if it is shown (shouldn't be possible),
		//or show the warning if we didn't overwrite
		CheckShowDescriptionChangedWarning(FALSE);

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::RecalculateAndApplyNewDescription");
}

void CEMREMChecklistElementSetupDlg::OnBtnEditEmCats() 
{
	try {

		CEMREMCodeCategorySetupDlg dlg(this);
		dlg.DoModal();

		BOOL bRemovedDetails = FALSE;

		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;

		//we'll need to remove any details that reference a deleted category
		for(i=0; i<dlg.m_dwaryDeletedIDs.GetSize(); i++) {
			long nDeletedID = (long)(dlg.m_dwaryDeletedIDs.GetAt(i));

			BOOL bRemovedThisCategory = FALSE;
			
			//do any of our elements reference this category?
			for(int j=0; j<m_pLocalRuleInfo->paryDetails.GetSize(); j++) {
				ChecklistElementRuleDetailInfo *pDetail = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(j));

				if(!pDetail->bDeleted && pDetail->nCategoryID == nDeletedID) {
					//we are referencing that category, we will need to remove the detail

					//mark as deleted
					pDetail->bDeleted = TRUE;

					//we'll remove the row momentarily

					//note that we deleted something
					bRemovedDetails = TRUE;
					bRemovedThisCategory = TRUE;
				}
			}

			if(bRemovedThisCategory) {
				//now in one loop, remove all rows with this category ID
				IRowSettingsPtr pRowToCheck = m_DetailList->GetFirstRow();
				while(pRowToCheck) {

					if(VarLong(pRowToCheck->GetValue(edlcCategoryID), -1) == nDeletedID) {

						//remove this row, but first assign out so we can properly get the next row
						IRowSettingsPtr pRowToDelete = pRowToCheck;
						pRowToCheck = pRowToCheck->GetNextRow();
						m_DetailList->RemoveRow(pRowToDelete);

						// (j.jones 2007-09-18 16:02) - PLID 27397 - if we don't have more than one detail, disable the radio buttons
						if(m_DetailList->GetRowCount() <= 1) {
							GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(FALSE);
							GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(FALSE);
						}
						else {
							GetDlgItem(IDC_RADIO_ALL_DETAILS)->EnableWindow(TRUE);
							GetDlgItem(IDC_RADIO_ANY_DETAILS)->EnableWindow(TRUE);
						}
					}
					else {
						//move on
						pRowToCheck = pRowToCheck->GetNextRow();
					}
				}
			}
		}

		if(bRemovedDetails) {

			//since we've made a change, hide the pre-loaded label if it exists
			HidePreLoadedLabel();

			//update our description
			RecalculateAndApplyNewDescription();

			//undo the approval
			CheckUndoApproval();

			MessageBox("Rules that referenced the categories you deleted have been removed. Please take note of these changes.", "Practice", MB_OK | MB_ICONINFORMATION);
		}

		//need to update the embedded combo query to force a requery of that data
		m_DetailList->GetColumn(edlcCategoryID)->ComboSource = _bstr_t("SELECT ID, Name FROM EMCodeCategoryT ORDER BY Name");

		//now, update each detail's category name, incase it changed
		for(i=0; i<m_pLocalRuleInfo->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo* pDetailInfo = (ChecklistElementRuleDetailInfo*)(m_pLocalRuleInfo->paryDetails.GetAt(i));

			// (j.jones 2007-10-01 15:21) - PLID 27104 - skip if deleted
			if(pDetailInfo->bDeleted) {
				continue;
			}
			
			//find the row in the list
			IRowSettingsPtr pRow = m_DetailList->FindByColumn(edlcCategoryID, (long)(pDetailInfo->nCategoryID), m_DetailList->GetFirstRow(), FALSE);
			if(pRow) {
				//update the category name
				pDetailInfo->strCategoryName = VarString(pRow->GetOutputValue(edlcCategoryID));
			}
			else {
				//this should be impossible
				ASSERT(FALSE);
			}
		}

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnBtnEditEmCats");
}

void CEMREMChecklistElementSetupDlg::OnCheckElementApproved() 
{
	try {

		// (j.jones 2007-08-29 11:38) - PLID 27135 - added permissions

		if(IsDlgButtonChecked(IDC_CHECK_ELEMENT_APPROVED)) {

			//if they checked the box, check permissions, and uncheck if permission denied

			if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptDynamic0))
				CheckDlgButton(IDC_CHECK_ELEMENT_APPROVED, FALSE);
		}

		// (j.jones 2007-08-29 11:38) - PLID 27135 - moved the interface setup to ReflectElementApprovalChange
		ReflectElementApprovalChange();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnCheckElementApproved");
}

//will uncheck the approved box if checked
void CEMREMChecklistElementSetupDlg::CheckUndoApproval()
{
	if(IsDlgButtonChecked(IDC_CHECK_ELEMENT_APPROVED)) {

		//uncheck the box
		CheckDlgButton(IDC_CHECK_ELEMENT_APPROVED, FALSE);
		ReflectElementApprovalChange();
	}
}

// (j.jones 2007-08-29 11:36) - PLID 27135 - changed the approval code for permission purposes
void CEMREMChecklistElementSetupDlg::ReflectElementApprovalChange()
{
	if(IsDlgButtonChecked(IDC_CHECK_ELEMENT_APPROVED)) {

		//they checked the box

		//update the local info
		m_pLocalRuleInfo->bApproved = TRUE;					
		m_pLocalRuleInfo->dtApproved = COleDateTime::GetCurrentTime();
		m_pLocalRuleInfo->nApprovalUserID = GetCurrentUserID();
		m_pLocalRuleInfo->strApprovalUserName = GetCurrentUserName();

		//update the display
		CString strApproved;
		strApproved.Format("Approved By %s on %s", m_pLocalRuleInfo->strApprovalUserName, FormatDateTimeForInterface(m_pLocalRuleInfo->dtApproved, NULL, dtoDateTime));
		SetDlgItemText(IDC_CHECK_ELEMENT_APPROVED, strApproved);
	}
	else {
		//the unchecked the box

		//update the local info
		m_pLocalRuleInfo->bApproved = FALSE;
				
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		m_pLocalRuleInfo->dtApproved = dtInvalid;

		m_pLocalRuleInfo->nApprovalUserID = -1;
		m_pLocalRuleInfo->strApprovalUserName = "";

		//update the display
		SetDlgItemText(IDC_CHECK_ELEMENT_APPROVED, "Approved");
	}
}

// (j.jones 2007-09-18 16:38) - PLID 27397 - added any/all radio buttons
void CEMREMChecklistElementSetupDlg::OnRadioAllDetails() 
{
	try {

		//since we've made a change, hide the pre-loaded label if it exists
		HidePreLoadedLabel();

		//update our description
		RecalculateAndApplyNewDescription();

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnRadioAllDetails");
}

// (j.jones 2007-09-18 16:38) - PLID 27397 - added any/all radio buttons
void CEMREMChecklistElementSetupDlg::OnRadioAnyDetails() 
{
	try {

		//since we've made a change, hide the pre-loaded label if it exists
		HidePreLoadedLabel();

		//update our description
		RecalculateAndApplyNewDescription();

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistElementSetupDlg::OnRadioAnyDetails");
}
