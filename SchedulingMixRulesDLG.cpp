// SchedulingMixRulesDLG.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SchedulingMixRulesDLG.h"
#include "AdministratorRc.h"
#include "SchedulingMixRulesConfigDLG.h"
#include "AuditTrail.h"

// SchedulingMixRulesDLG dialog
// (s.tullis 2014-12-08 15:49) - PLID 64125 - created
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CSchedulingMixRulesDLG, CNxDialog)

CSchedulingMixRulesDLG::CSchedulingMixRulesDLG(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulingMixRulesDLG::IDD, pParent)
{

}

CSchedulingMixRulesDLG::~CSchedulingMixRulesDLG()
{
}

void CSchedulingMixRulesDLG::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_MIX_RULE, m_btnAddRule);
	DDX_Control(pDX, IDC_EDIT_MIX_RULE, m_btnEditRule);
	DDX_Control(pDX, IDC_COPY_MIX_RULE, m_btnCopyRule);
	DDX_Control(pDX, IDC_DELETE_MIX_RULE, m_btnDeleteRule);
	DDX_Control(pDX, IDC_HIDE_EXPIRED_CHECK, m_checkHideExpiredRules);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CSchedulingMixRulesDLG, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_MIX_RULE, &CSchedulingMixRulesDLG::OnBnClickedAddRule)
	ON_BN_CLICKED(IDC_COPY_MIX_RULE, &CSchedulingMixRulesDLG::OnBnClickedCopyRule)
	ON_BN_CLICKED(IDC_DELETE_MIX_RULE, &CSchedulingMixRulesDLG::OnBnClickedDeleteRule)
	ON_BN_CLICKED(IDC_HIDE_EXPIRED_CHECK, &CSchedulingMixRulesDLG::OnBnClickedHideExpiredCheck)
	ON_BN_CLICKED(IDC_EDIT_MIX_RULE, &CSchedulingMixRulesDLG::OnBnClickedEditMixRule)
END_MESSAGE_MAP()


// SchedulingMixRulesDLG message handlers
enum RulesList{
	RuleID = 0,
	RuleName,
	RuleStartDate,
	RuleEndDate,
	Color,
};
enum LocationFilter{
	LocationID=0,
	LocationName,
};

enum LocationFilterDefault{
	lfdAll=-1,
};
enum ResourceFilter{
	ResourceID=0,
	ResourceName, 
};

enum ResourceFilterDefault{
	 rfdAll= -1,
};
enum InsuranceFilter{
	InsuranceID = 0,
	InsuranceName,
};

enum InsuranceFilterDefault{
	ifdAll=-3,
	ifdALLInsurance=-2,
	ifdNoInsurance=-1,
};

BOOL CSchedulingMixRulesDLG::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	ModifyStyle(WS_SYSMENU, 0);

	try{
		g_propManager.CachePropertiesInBulk("SchedulingMixRulesDLG", propNumber,
			R"((Username = '<None>' OR Username = '%s') AND (
		Name = 'HideExpiredScheduleMixRules' OR Name = 'ScheduleMixRulesInsuranceFilter'))", _Q(GetCurrentUserName()));
		
		m_btnAddRule.AutoSet(NXB_NEW);// (s.tullis 2014-12-08 16:15) - PLID 64129 -Set Icon
		m_btnCopyRule.AutoSet(NXB_NEW);// (s.tullis 2014-12-08 15:57) - PLID 64127 -Set Icon
		m_btnDeleteRule.AutoSet(NXB_DELETE);// (s.tullis 2014-12-08 15:57) - PLID 64127 -Set Icon
		m_btnEditRule.AutoSet(NXB_MODIFY); // (s.tullis 2014-12-08 16:15) - PLID 64129 -Set Icon
		m_btnClose.AutoSet(NXB_CLOSE);
		// (s.tullis 2014-12-08 15:53) - PLID 64126 - Bind Filters
		m_pResourceList = BindNxDataList2Ctrl(IDC_RULE_RESOURCE_LIST, true);
		m_pLocationList = BindNxDataList2Ctrl(IDC_RULE_LOCATION_LIST, true);
		m_pInsuranceList = BindNxDataList2Ctrl(IDC_RULE_INSURANCE_LIST, false);
		// (s.tullis 2014-12-08 15:49) - PLID 64125 - Bind Rule Datalist
		m_pRulesList = BindNxDataList2Ctrl(IDC_SCHEDULE_RULES_LIST, false);
		// (s.tullis 2014-12-08 15:49) - PLID 64125 - Remember the hide expired check per user default to .. filter main datalist
		BOOL bCheckHideExpiredRulesSetting = GetRemotePropertyInt("HideExpiredScheduleMixRules", TRUE, 0, GetCurrentUserName(), TRUE);
		m_checkHideExpiredRules.SetCheck(bCheckHideExpiredRulesSetting);
		SetRuleFilters(TRUE);
		m_pRulesList->Sort();
		
		

	}NxCatchAll(__FUNCTION__)

	return TRUE;
}


// (s.tullis 2014-12-08 15:53) - PLID 64126 - Sets Options for filters
void CSchedulingMixRulesDLG::SetRuleFilters(BOOL bLoad)
{
	try{
		
		_variant_t nInsSelID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		// We are refreshing.. need to keep the user's filter selections if their is any
		if (!bLoad){
			pRow = m_pInsuranceList->GetCurSel();
			if (pRow){
				nInsSelID = pRow->GetValue(InsuranceFilter::InsuranceID);
			}
		}

		if (m_checkHideExpiredRules.GetCheck())
		{
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			CString strWhereClause;
			strWhereClause.Format(" InsuranceCoT.PersonID IN "
				" (Select ScheduleMixRuleInsuranceCosT.InsuranceCoID "
				" FROM ScheduleMixRuleInsuranceCosT "
				" Inner Join ScheduleMixRulesT "
				" ON ScheduleMixRulesT.ID = ScheduleMixRuleInsuranceCosT.RuleID "
				" WHERE ScheduleMixRulesT.EndDate >= '%s' OR ScheduleMixRulesT.EndDate IS NULL  ) "
				, FormatDateTimeForSql(dtToday,dtoDate));
			m_pInsuranceList->PutWhereClause(_bstr_t(strWhereClause));
		}
		else{
			m_pInsuranceList->PutWhereClause(" InsuranceCoT.PersonID IN "
				"(Select ScheduleMixRuleInsuranceCosT.InsuranceCoID "
				" FROM ScheduleMixRuleInsuranceCosT ) ");
		}

		m_pInsuranceList->Requery();
		pRow = m_pInsuranceList->GetNewRow();
		if (pRow){
			pRow->PutValue(InsuranceFilter::InsuranceID, InsuranceFilterDefault::ifdAll);
			pRow->PutValue(InsuranceFilter::InsuranceName, "< All >");
			m_pInsuranceList->AddRowSorted(pRow, NULL);
			
		}

		pRow = m_pInsuranceList->GetNewRow();
		if (pRow){
			pRow->PutValue(InsuranceFilter::InsuranceID, InsuranceFilterDefault::ifdALLInsurance);
			pRow->PutValue(InsuranceFilter::InsuranceName, "< All Insurance Companies >");
			m_pInsuranceList->AddRowSorted(pRow, NULL);
		}

		pRow = m_pInsuranceList->GetNewRow();
		if (pRow){
			pRow->PutValue(InsuranceFilter::InsuranceID, InsuranceFilterDefault::ifdNoInsurance);
			pRow->PutValue(InsuranceFilter::InsuranceName, "< No Insurance Company >");
			m_pInsuranceList->AddRowSorted(pRow, NULL);
		}

		if (!bLoad){
			//set to the old filter select
			// else set to default selection if the old does not exist

			pRow = m_pInsuranceList->SetSelByColumn((short)InsuranceFilter::InsuranceID, nInsSelID);
			if (pRow){
				m_pInsuranceList->PutCurSel(pRow);
			}
			else{
				m_pInsuranceList->SetSelByColumn((short)InsuranceFilter::InsuranceID, InsuranceFilterDefault::ifdAll /*ALL*/);
			}

			
		}
		else{// loading set to defaults
			// (s.tullis 2015-01-07 16:30) - PLID 64126 - Need to Remember User INS Selection
			pRow = m_pResourceList->GetNewRow();
			if (pRow){
				pRow->PutValue(ResourceFilter::ResourceID, ResourceFilterDefault::rfdAll);
				pRow->PutValue(ResourceFilter::ResourceName, "< All >");
				m_pResourceList->AddRowSorted(pRow, NULL);
			}
			pRow = m_pLocationList->GetNewRow();
			if (pRow){
				pRow->PutValue(LocationFilter::LocationID, LocationFilterDefault::lfdAll);
				pRow->PutValue(LocationFilter::LocationName, "< All >");
				m_pLocationList->AddRowSorted(pRow, NULL);
			}
			long nInsRemotePropSel = GetRemotePropertyInt("ScheduleMixRulesInsuranceFilter", InsuranceFilterDefault::ifdAll, 0, GetCurrentUserName(), TRUE);
			pRow = m_pInsuranceList->SetSelByColumn((short)InsuranceFilter::InsuranceID, nInsRemotePropSel);
			if (pRow){
				m_pInsuranceList->PutCurSel(pRow);
			}
			else{
				m_pInsuranceList->SetSelByColumn((short)InsuranceFilter::InsuranceID, InsuranceFilterDefault::ifdAll /*ALL*/);
			}
			m_pResourceList->SetSelByColumn((short)ResourceFilter::ResourceID, ResourceFilterDefault::rfdAll);
			m_pLocationList->SetSelByColumn((short)LocationFilter::LocationID, LocationFilterDefault::lfdAll);
			
		}
		FilterRuleList();
	}NxCatchAll(__FUNCTION__)
}

BEGIN_EVENTSINK_MAP(CSchedulingMixRulesDLG, CNxDialog)
ON_EVENT(CSchedulingMixRulesDLG, IDC_SCHEDULE_RULES_LIST, 6, CSchedulingMixRulesDLG::RButtonDownScheduleRulesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CSchedulingMixRulesDLG, IDC_SCHEDULE_RULES_LIST, 2, CSchedulingMixRulesDLG::SelChangedScheduleRulesList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_SCHEDULE_RULES_LIST, 3, CSchedulingMixRulesDLG::DblClickCellScheduleRulesList, VTS_DISPATCH VTS_I2)
ON_EVENT(CSchedulingMixRulesDLG, IDC_SCHEDULE_RULES_LIST, 18, CSchedulingMixRulesDLG::RequeryFinishedScheduleRulesList, VTS_I2)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_RESOURCE_LIST, 1, CSchedulingMixRulesDLG::SelChangingRuleResourceList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_LOCATION_LIST, 1, CSchedulingMixRulesDLG::SelChangingRuleLocationList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_INSURANCE_LIST, 1, CSchedulingMixRulesDLG::SelChangingRuleInsuranceList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_SCHEDULE_RULES_LIST, 1, CSchedulingMixRulesDLG::SelChangingScheduleRulesList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_INSURANCE_LIST, 16, CSchedulingMixRulesDLG::SelChosenRuleInsuranceList, VTS_DISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_LOCATION_LIST, 16, CSchedulingMixRulesDLG::SelChosenRuleLocationList, VTS_DISPATCH)
ON_EVENT(CSchedulingMixRulesDLG, IDC_RULE_RESOURCE_LIST, 16, CSchedulingMixRulesDLG::SelChosenRuleResourceList, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (s.tullis 2014-12-08 15:49) - PLID 64125 -Right Click the datalist prompt a menu to Edit, Rename, Copy, and delete options
void CSchedulingMixRulesDLG::RButtonDownScheduleRulesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// Build the menu for the current row
		if (pRow){
			m_pRulesList->PutCurSel(pRow);
			SetControls(TRUE);
			CMenu mnu;
			CPoint pt;
			GetCursorPos(&pt);

			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 1, "&Edit Rule");
			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 2, "&Rename Rule");
			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 3, "&Copy Rule");
			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 4, "&Delete Rule");

			long nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
			mnu.DestroyMenu();

			switch (nMenuChoice)
			{
			case 1:
				// (s.tullis 2014-12-08 15:57) - PLID 64127 -Edit Rule
				EditRule(pRow);
				break;
			case 2:
				// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Rename
				RenameRule(pRow);
				break;
			case 3:
				// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Copy
				CopyRule(pRow);
				break;
			case 4:
				// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127- Delete
				DeleteRule(pRow);
				break;
			default:
				break;
			}
		}
	
		
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 16:15) - PLID 64129 - Get Rule Name to Create
// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127  - Get Rule Name to Rename
BOOL CSchedulingMixRulesDLG::GetStrRuleFromInput(CString &rule)
{
	BOOL bGetInput = TRUE;
	CString strOldRule = "";
	CString strNewRule = "";
	if (!rule.IsEmpty())
	{
		strOldRule = rule;
	}

	try{
	 
		while (bGetInput)
		{
			// only able to the user to enter a rule of 255 charactes or less
			int nResult = InputBoxLimited(this, "Enter a name for the new rule", rule, "", 255, false, false, NULL);
			if (nResult == IDOK)
			{
				strNewRule = rule;

				rule.TrimLeft(); rule.TrimRight();
				
				if (rule == "")
				{
					MessageBox("A scheduling mix rule is required to have a name.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					continue;
				}

				if (strNewRule.MakeLower() == strOldRule.MakeLower())
				{
					return TRUE;
				}

				ADODB::_RecordsetPtr prs = CreateParamRecordset(" SELECT ID FROM ScheduleMixRulesT WHERE NAME = {STRING} ", rule) ;
		        if (!prs->eof){
					MessageBox("A rule already exists with the name: " + rule + "\r\nRule names are required to be unique.", "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				else{
					return TRUE;
				}

			}
			else{
				bGetInput= FALSE;
			}
		}


	}NxCatchAll(__FUNCTION__)
		
		return FALSE;
}
// (s.tullis 2014-12-08 16:15) - PLID 64129 - Create Rule
void CSchedulingMixRulesDLG::OnBnClickedAddRule()
{
	try{
		CString strRuleName = "";
		if (!GetStrRuleFromInput(strRuleName)){
			return;
		}

		CreateRule(strRuleName);

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Copy Rule
void CSchedulingMixRulesDLG::OnBnClickedCopyRule()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRulesList->GetCurSel();
		if (pRow){
			CopyRule(pRow);
		}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Delete Rule
void CSchedulingMixRulesDLG::OnBnClickedDeleteRule()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRulesList->GetCurSel();
		if (pRow){
			DeleteRule(pRow);
		}
	   }NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:49) - PLID 64125 - Set the Buttons Disabled /Enabled based on datalist cur sel..
// The user must have a selection in the datalist to access the copy, edit, delete buttons
void CSchedulingMixRulesDLG::SelChangedScheduleRulesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		if ( lpNewSel == NULL){
			SetControls(FALSE);
		}
		else if (lpNewSel != NULL){
			SetControls(TRUE);
		}
		
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:57) - PLID 64127 - Open in Edit Mode when Rule is double clicked
void CSchedulingMixRulesDLG::DblClickCellScheduleRulesList(LPDISPATCH lpRow, short nColIndex)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow){
			EditRule(pRow);
		}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:49) - PLID 64125 - When Hide Expired is check .. Filter Datalist on Expired Rules
void CSchedulingMixRulesDLG::OnBnClickedHideExpiredCheck()
{
	try{
		if (m_checkHideExpiredRules.GetCheck() == FALSE){
				SetRemotePropertyInt("HideExpiredScheduleMixRules", FALSE, 0, GetCurrentUserName());
		}
		else if (m_checkHideExpiredRules.GetCheck() == TRUE){
				SetRemotePropertyInt("HideExpiredScheduleMixRules", TRUE, 0, GetCurrentUserName());
		}
		SetRuleFilters();
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Delete Rule 
void CSchedulingMixRulesDLG::DeleteRule( NXDATALIST2Lib::IRowSettingsPtr pRow){

	try{
		if (pRow){
			CString strMessage = "";
			long nRuleID = VarLong(pRow->GetValue(RulesList::RuleID));
			CString strRuleName = VarString(pRow->GetValue(RulesList::RuleName));

			strMessage.Format("You have chosen to delete the rule ‘%s’. This action is unrecoverable! \r\n"
				"\r\n"
				"Are you sure you wish to continue?", strRuleName);

			if (AfxMessageBox(strMessage, MB_YESNO) == IDYES) {

				// (j.jones 2014-12-15 15:13) - PLID 64272 - warn if there are tracked overrides
				if (ReturnsRecordsParam("SELECT TOP 1 RuleID FROM AppointmentMixRuleOverridesT WHERE RuleID = {INT}", nRuleID)) {
					if (MessageBox("This rule has already been overridden on an existing appointment. "
						"If you delete this rule, the override history will also be deleted.\n\n"
						"Are you sure you still wish to delete this rule?", "Practice", MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {

						return;
					}
				}

				ADODB::_RecordsetPtr prs = CreateParamRecordset(" SET NOCOUNT ON "
					"DECLARE @RULEID INT ={INT} "
					"Delete FROM ScheduleMixRuleLocationsT WHERE RULEID =@RULEID " 
					"Delete FROM ScheduleMixRuleInsuranceCosT WHERE RULEID= @RULEID "
					"Delete FROM ScheduleMixRuleDetailsT WHERE RULEID =@RULEID "
					"Delete FROM AppointmentMixRuleOverridesT WHERE RuleID =@RULEID "
					"Delete FROM ScheduleMixRulesT WHERE ID = @RULEID "
					"DECLARE @TodoID INT "
					"      DECLARE @T TABLE( TASKID INT, ASSIGNEDUSERSIDS nvarchar(1000))"
					"      INSERT INTO @T (TASKID, ASSIGNEDUSERSIDS)"
					"      SELECT TaskID = T.Taskid , ASSIGNEDUSERSIDS = dbo.GetTodoAssignToIDString(T.TaskID) FROM TodoList T "
					"      WHERE T.RegardingID =@RULEID AND T.RegardingType= {INT} "
					"      SELECT @TodoID = TASKID FROM @T "
					"      IF @TodoID IS NOT NULL                                                   "
					"      BEGIN DELETE FROM TodoAssignToT WHERE TodoAssignToT.TaskID = @TodoID     "
					"      DELETE FROM TodoList WHERE TASKID = @TodoID  END"
					"      SET NOCOUNT OFF "
					"      SELECT * FROM @T  ", nRuleID, ttScheduleMixRules);
				
				
				// remove it from our list
				m_pRulesList->RemoveRow(pRow);
				// (s.tullis 2015-02-10 10:22) - PLID 64138 - Need a todo tablechecker to be sent
				if (!prs->eof)
				{
					ADODB::FieldsPtr fields= prs->Fields;
					long nTaskID = AdoFldLong(fields->Item["TASKID"]);
					CString strAssignedIDs = AdoFldString(fields->Item["ASSIGNEDUSERSIDS"]);
					CArray < long, long> arrAssignedID;
					ParseDelimitedStringToLongArray(strAssignedIDs, " ", arrAssignedID);

					if (arrAssignedID.GetSize() == 1)
						CClient::RefreshTodoTable(nTaskID, nRuleID, arrAssignedID[0], TableCheckerDetailIndex::tddisDeleted);
					else {// more than one User Assigned send the regular TableChecker
						CClient::RefreshTodoTable(nTaskID, nRuleID, -1, TableCheckerDetailIndex::tddisDeleted);
					}

				}
				

				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, strRuleName, nAuditID, aeiSchedulingMixRuleDeleted, nRuleID, strRuleName, "<Deleted>", aepHigh, aetDeleted);
				//end auditing
				SetRuleFilters();
			}
			else{
				return;
			}
		}

	}NxCatchAll(__FUNCTION__)

}
// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127 - Rename Rule 
void CSchedulingMixRulesDLG::RenameRule( NXDATALIST2Lib::IRowSettingsPtr pRow){
	try{
		if (pRow){
			long nRuleID = VarLong(pRow->GetValue(RulesList::RuleID));
			CString Rule = VarString(pRow->GetValue(RulesList::RuleName));
			// this will return false if they don't actually change the name 
			if (!GetStrRuleFromInput(Rule)){
				return;
			}
			ExecuteParamSql(" UPDATE SCHEDULEMIXRULEST SET NAME = {STRING} WHERE SCHEDULEMIXRULEST.ID = {INT}  ", Rule, nRuleID);
			pRow->PutValue(RulesList::RuleName, _variant_t(Rule));
		}
		
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:57) - PLID 64127 -  Copy Rule- Opens up a Rule in edit mode with new rulename .. then saves new rule
void CSchedulingMixRulesDLG::CopyRule(NXDATALIST2Lib::IRowSettingsPtr pRow){
	try{
		if (pRow){
			CString strRuleName = "";
			if (!GetStrRuleFromInput(strRuleName)){
				return;
			}
			COleDateTime dtStartDate = VarDateTime(pRow->GetValue(RulesList::RuleStartDate));
			COleDateTime dtEndDate;
			if (pRow->GetValue(RulesList::RuleEndDate).vt == VT_NULL){
				dtEndDate.SetStatus(COleDateTime::null);
			}
			else{
				dtEndDate = VarDateTime(pRow->GetValue(RulesList::RuleEndDate));
			}
			long nColor = VarLong(pRow->GetValue(RulesList::Color), -1);
			long nRuleID = VarLong(pRow->GetValue(RulesList::RuleID), -1);
			CSchedulingMixRulesConfigDLG dlg(SchedulingMixRulesConfigMode::Copy, strRuleName, dtStartDate, dtEndDate, nColor, nRuleID);
			if (dlg.DoModal() == IDOK){
				SetRuleFilters();

				ADODB::_RecordsetPtr prs = CreateParamRecordset("Select ScheduleMixRulesT.ID FROM ScheduleMixRulesT WHERE ScheduleMixRulesT.Name = {STRING} ", strRuleName);
				ADODB::FieldsPtr fields = prs->Fields;
				if (!prs->eof){
					//auditing
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(-1, strRuleName, nAuditID, aeiSchedulingMixRuleCreated, AdoFldLong(fields->Item["ID"]), strRuleName, "<Created>", aepHigh, aetCreated);
					//end auditing
				}
			}

		
		}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:57) - PLID 64127 - Function that opens the Config Dialog in Edit Mode
void CSchedulingMixRulesDLG::EditRule(NXDATALIST2Lib::IRowSettingsPtr pRow){
	try{
		
		if (pRow){
			COleDateTime dtStartDate = VarDateTime(pRow->GetValue(RulesList::RuleStartDate), COleDateTime::GetCurrentTime());
			COleDateTime dtEndDate; 
			if (pRow->GetValue(RulesList::RuleEndDate).vt == VT_NULL){
				dtEndDate.SetStatus(COleDateTime::null);
			}
			else{
				dtEndDate = VarDateTime(pRow->GetValue(RulesList::RuleEndDate));
			}
			long nRuleID = VarLong(pRow->GetValue(RulesList::RuleID),-1);
			long nColor = VarLong(pRow->GetValue(RulesList::Color),-1);
			CSchedulingMixRulesConfigDLG dlg(SchedulingMixRulesConfigMode::Edit, VarString(pRow->GetValue(RulesList::RuleName)), dtStartDate, dtEndDate,nColor ,nRuleID);
			long nResult=dlg.DoModal();
			
			if (nResult == IDOK){
				SetRuleFilters();
			}

			
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 16:15) - PLID 64129 - Open the Config Dialog in Creation Mode
void CSchedulingMixRulesDLG::CreateRule(CString strRuleName){
	try{
		COleDateTime dtEndDate;
		dtEndDate.SetStatus(COleDateTime::null);
		COleDateTime dtStartDate = COleDateTime::GetCurrentTime();
		CSchedulingMixRulesConfigDLG dlg(SchedulingMixRulesConfigMode::Create, strRuleName , dtStartDate, dtEndDate);
		if (dlg.DoModal() == IDOK){
			
			m_pRulesList->Requery();

			
			
			ADODB::_RecordsetPtr prs = CreateParamRecordset("Select ScheduleMixRulesT.ID FROM ScheduleMixRulesT WHERE ScheduleMixRulesT.Name = {STRING} ", strRuleName);
			ADODB::FieldsPtr fields = prs->Fields;
			if (!prs->eof){
				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, strRuleName, nAuditID, aeiSchedulingMixRuleCreated, AdoFldLong(fields->Item["ID"]), strRuleName, "<Created>", aepHigh, aetCreated);
				//end auditing
			}
			SetRuleFilters();
		}

		SetControls(FALSE);

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 16:15) - PLID 64129 - Edit Selected Rule
void CSchedulingMixRulesDLG::OnBnClickedEditMixRule()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRulesList->GetCurSel();
		if (pRow){
			EditRule(pRow);
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 15:53) - PLID 64126 -  Gets the Filter Query and requeries the Rule DataList
void CSchedulingMixRulesDLG::FilterRuleList()
{
	try{
		CString strFROM = "";
		CString strWhere=" WHERE ";
		CString strHaving = " HAVING ";
		CString strHideExpired = "";
		CString buffer="";
		CArray<CString, CString> arrWhere;
		long nSelID;
		
		if (m_checkHideExpiredRules.GetCheck()){
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			strHideExpired.Format(" SMRQ.EndDate >= '%s' OR SMRQ.EndDate IS NULL ", FormatDateTimeForSql(dtToday,dtoDate));
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationList->GetCurSel();
		if (pRow){
			nSelID = VarLong(pRow->GetValue(LocationFilter::LocationID));
			if (nSelID != -1){
				buffer.Format(" ScheduleMixRuleLocationsT.LocationID = %li ", nSelID);
				arrWhere.Add(buffer);
			}
	
		}
		pRow = m_pInsuranceList->GetCurSel();
		if (pRow){
			nSelID = VarLong(pRow->GetValue(InsuranceFilter::InsuranceID));

			switch (nSelID)
			{
			case -1:
				buffer.Format(" ScheduleMixRuleInsuranceCosT.InsuranceCoID IS NULL ");
				arrWhere.Add(buffer);
				break;
			case -2:
				strHaving = strHaving +" Count(DISTINCT ScheduleMixRuleInsuranceCosT.InsuranceCoID) > 0 ";
				break;
			case -3:
				//nothing
				break;
			default:
				buffer.Format(" ScheduleMixRuleInsuranceCosT.InsuranceCoID = %li ", nSelID);
				arrWhere.Add(buffer);
				break;
			}
			
		}
		pRow = m_pResourceList->GetCurSel();
		if (pRow){
			nSelID = VarLong(pRow->GetValue(ResourceFilter::ResourceID));
			if (nSelID != -1){
				buffer.Format(" ScheduleMixRuleDetailsT.ResourceID = %li ", nSelID);
				arrWhere.Add(buffer);
			}
		}

		for (int i = 0; i < arrWhere.GetSize(); i++)
		{
			if (i != arrWhere.GetSize() - 1){
				strWhere = strWhere + arrWhere[i] + " AND ";
			}
			else{
				strWhere = strWhere + arrWhere[i];
			}
		}
		if (strWhere == " WHERE "){
			strWhere = "";
		}

		if (strHaving == " HAVING "){
			strHaving = "";
		}

		if (strHideExpired == ""){
			m_pRulesList->PutWhereClause("");
		}
		else{
			m_pRulesList->PutWhereClause(_bstr_t(strHideExpired));
		}

		strFROM.Format("(Select DISTINCT ScheduleMixRulesT.ID, ScheduleMixRulesT .Name, ScheduleMixRulesT .StartDate, ScheduleMixRulesT.EndDate,ScheduleMixRulesT.Color,Count( DISTINCT ScheduleMixRuleInsuranceCosT.InsuranceCoID) As InsuranceCount "
			"FROM ScheduleMixRulesT  "
			"LEFT JOIN ScheduleMixRuleInsuranceCosT ON ScheduleMixRulesT.ID= ScheduleMixRuleInsuranceCosT.RuleID "
			"LEFT JOIN ScheduleMixRuleLocationsT ON ScheduleMixRuleLocationsT.RuleID = ScheduleMixRulesT.ID "
			"LEFT JOIN ScheduleMixRuleDetailsT ON ScheduleMixRuleDetailsT.RuleID= ScheduleMixRulesT.ID %s "
			"Group BY ScheduleMixRulesT.ID, ScheduleMixRulesT .Name, ScheduleMixRulesT .StartDate, ScheduleMixRulesT.EndDate,ScheduleMixRulesT.Color %s )  as SMRQ ", strWhere, strHaving);

		
		int nRuleIDSel = -1;
		BOOL bSetControls = FALSE;
		pRow = m_pRulesList->GetCurSel();
		if (pRow)
		{
			nRuleIDSel = VarLong(pRow->GetValue(RulesList::RuleID));
		}
		m_pRulesList->PutFromClause(_bstr_t(strFROM));
		m_pRulesList->Requery();
		
		if (nRuleIDSel != -1){
			pRow = m_pRulesList->SetSelByColumn(RulesList::RuleID, nRuleIDSel);
			if (pRow)
			{
				m_pRulesList->PutCurSel(pRow);
				bSetControls = TRUE;
			}
		}
		SetControls(bSetControls);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 15:49) - PLID 64125 - Set Expired Rules to grey 
void CSchedulingMixRulesDLG::SetExpiredRuleRowsToGrey()
{
	try{
		OLE_COLOR colorGrayed = RGB(205, 201, 201);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRulesList->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pRowNext;
		COleDateTime dtCurTime = COleDateTime::GetCurrentTime();
		COleDateTime dtToday(dtCurTime.GetYear(), dtCurTime.GetMonth(), dtCurTime.GetDay(), 0, 0, 0);
		COleDateTime dtExpire;
		while(pRow){
			pRowNext = pRow->GetNextRow();
			if (pRow->GetValue(RulesList::RuleEndDate).vt != VT_NULL){
				dtExpire = VarDateTime(pRow->GetValue(RulesList::RuleEndDate));
				if (dtExpire < dtToday)
				{
					pRow->PutBackColor(colorGrayed);
				}
			}
			pRow = pRowNext;
		}


	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:49) - PLID 64125 - Check to see if the Hide expired button is check then set expired to grey .. or do nothing
void CSchedulingMixRulesDLG::RequeryFinishedScheduleRulesList(short nFlags)
{
	try{
		if (!m_checkHideExpiredRules.GetCheck()){
			SetExpiredRuleRowsToGrey();
		}
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-22 15:40) - PLID 64125 - Set controls on or off
void CSchedulingMixRulesDLG::SetControls(BOOL onOFF)
{
	try{
		GetDlgItem(IDC_EDIT_MIX_RULE)->EnableWindow(onOFF);
		GetDlgItem(IDC_COPY_MIX_RULE)->EnableWindow(onOFF);
		GetDlgItem(IDC_DELETE_MIX_RULE)->EnableWindow(onOFF);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-22 17:25) - PLID 64126 - check for nulls
void CSchedulingMixRulesDLG::SelChangingRuleResourceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-22 17:25) - PLID 64126 - check for nulls
void CSchedulingMixRulesDLG::SelChangingRuleLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-22 17:25) - PLID 64126 - check for nulls
void CSchedulingMixRulesDLG::SelChangingRuleInsuranceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-22 17:25) - PLID 64125 - check for nulls
void CSchedulingMixRulesDLG::SelChangingScheduleRulesList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-01-07 16:30) - PLID 64126 - Need to Remember User Selection
void CSchedulingMixRulesDLG::SelChosenRuleInsuranceList(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow){
				long nInsuranceSelID = VarLong(pRow->GetValue(InsuranceFilter::InsuranceID));
				SetRemotePropertyInt("ScheduleMixRulesInsuranceFilter", VarLong(pRow->GetValue(InsuranceFilter::InsuranceID)), 0, GetCurrentUserName());
				FilterRuleList();
			}
	  }NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:53) - PLID 64126 - react to changing locations
void CSchedulingMixRulesDLG::SelChosenRuleLocationList(LPDISPATCH lpRow)
{
	try{
		FilterRuleList();
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 15:53) - PLID 64126 - React to Resource Change
void CSchedulingMixRulesDLG::SelChosenRuleResourceList(LPDISPATCH lpRow)
{
	try{
		FilterRuleList();
	}NxCatchAll(__FUNCTION__)
}



