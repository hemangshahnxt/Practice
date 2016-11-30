// SchedulingMixRulesConfigDLG.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulingMixRulesConfigDLG.h"
#include "AdministratorRc.h"
#include "SchedulingMixRulesDLG.h"
#include "SchedulerMixRulesAddTypes.h"
#include "Color.h"
#include "TodoUtils.h"
#include "CommonSchedUtils.h"
#include "SchedulerView.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;
using namespace std;
// SchedulingMixRulesConfigDLG dialog

IMPLEMENT_DYNAMIC(CSchedulingMixRulesConfigDLG, CNxDialog)
enum LocationList{
	LocationID=0,
	LocationName,
	TypeID,
	Active
};

enum InsuranceList{
	InsuranceID=0,
	InsuranceName,
	Inactive,
};

enum TodoPriorityList{
	PriorityID = 0,
	PriorityName,
};

enum TodoAssignedUserList{
	UserID=0,
	UserName,

};
enum RuleResourceList{
	ResourceID=0,
	ResourceName,
};
enum RuleDetailList{
	RuleDetailID=0,
	DetailresourceID,
	ApptTypeID,
	PurposeID,
	MaxAppts,
};

enum TodoCategoryList{
	CategoryID = 0,
	CategoryName,
};




CSchedulingMixRulesConfigDLG::CSchedulingMixRulesConfigDLG(SchedulingMixRulesConfigMode ConfigMode, CString strRule, COleDateTime dtStartDate, COleDateTime dtEndDate,long nColor, long nRuleID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulingMixRulesConfigDLG::IDD, pParent)
{
	
		m_ConfigurationMode = ConfigMode;
		m_strRuleName = strRule;
		m_nRuleID = nRuleID;
		m_dtStartDate = dtStartDate;
		m_dtEndDate = dtEndDate;
		m_nColor = nColor;
}

CSchedulingMixRulesConfigDLG::~CSchedulingMixRulesConfigDLG()
{
}

void CSchedulingMixRulesConfigDLG::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RULE_TYPE_COLOR, m_typeColor);
	DDX_Control(pDX, IDC_RULE_COLOR_PICKER_CTRL, m_ctrlRuleColorPicker);
	DDX_Control(pDX, IDC_LOCATION_RIGHT, m_btnLocation_Right);
	DDX_Control(pDX, IDC_LOCATION_RIGHT_ALL, m_btnLocation_Right_All);
	DDX_Control(pDX, IDC_LOCATION_LEFT, m_btnLocation_Left);
	DDX_Control(pDX, IDC_LOCATION_LEFT_ALL, m_btnLocation_Left_All);
	DDX_Control(pDX, IDC_INSURANCE_RIGHT, m_btnInsurance_Right);
	DDX_Control(pDX, IDC_INSURANCE_RIGHT_ALL, m_btnInsurance_Right_All);
	DDX_Control(pDX, IDC_INSURANCE_LEFT, m_btnInsurance_Left);
	DDX_Control(pDX, IDC_INSURANCE_LEFT_ALL, m_btnInsurance_Left_All);
	DDX_Control(pDX, IDC_RULE_COPY_TO, m_btnCopyTo);
	DDX_Control(pDX, IDC_RULE_DETAIL_ADD_TYPES, m_btnAddTypes);
	DDX_Control(pDX, IDC_RESOURCE_RIGHT, m_btnResource_Right);
	DDX_Control(pDX, IDC_RESOURCE_LEFT, m_btnResource_Left);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EXPIRATION_EDIT, m_TodoDayExpired);
	DDX_Control(pDX, IDC_SET_DATE_CHECK, m_checkExpiredDate);
	DDX_Control(pDX, IDC_SELECT_COLOR_CHECK, m_checkSelectColor);
	DDX_Control(pDX, IDC_RULE_START_DATE, m_dateRuleStart);
	DDX_Control(pDX, IDC_RULE_END_DATE, m_dateRuleEnd);
	DDX_Control(pDX, IDC_SET_DATE_CHECK, m_checkExpiredDate);
	DDX_Control(pDX, IDC_MULTI_USER_SELECT, m_MultiAssignedTodoUsers);
	DDX_Control(pDX, IDC_RULE_TODO_CHECK, m_checkSetReminder);
}



BEGIN_MESSAGE_MAP(CSchedulingMixRulesConfigDLG, CNxDialog)
	ON_BN_CLICKED(IDC_LOCATION_RIGHT, &CSchedulingMixRulesConfigDLG::OnBnClickedLocationRight)
	ON_BN_CLICKED(IDC_LOCATION_LEFT, &CSchedulingMixRulesConfigDLG::OnBnClickedLocationLeft)
	ON_BN_CLICKED(IDC_INSURANCE_RIGHT, &CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceRight)
	ON_BN_CLICKED(IDC_INSURANCE_LEFT, &CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceLeft)
	ON_BN_CLICKED(IDC_LOCATION_RIGHT_ALL, &CSchedulingMixRulesConfigDLG::OnBnClickedLocationRightAll)
	ON_BN_CLICKED(IDC_LOCATION_LEFT_ALL, &CSchedulingMixRulesConfigDLG::OnBnClickedLocationLeftAll)
	ON_BN_CLICKED(IDC_INSURANCE_RIGHT_ALL, &CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceRightAll)
	ON_BN_CLICKED(IDC_INSURANCE_LEFT_ALL, &CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceLeftAll)
	ON_BN_CLICKED(IDC_RESOURCE_RIGHT, &CSchedulingMixRulesConfigDLG::OnBnClickedResourceRight)
	ON_BN_CLICKED(IDC_RESOURCE_LEFT, &CSchedulingMixRulesConfigDLG::OnBnClickedResourceLeft)
	ON_BN_CLICKED(IDC_RULE_COPY_TO, &CSchedulingMixRulesConfigDLG::OnBnClickedRuleCopyTo)
	ON_BN_CLICKED(IDC_RULE_DETAIL_ADD_TYPES, &CSchedulingMixRulesConfigDLG::OnBnClickedRuleDetailAddTypes)
	ON_BN_CLICKED(IDC_SET_DATE_CHECK, &CSchedulingMixRulesConfigDLG::OnBnClickedSetDateCheck)
	ON_BN_CLICKED(IDOK, &CSchedulingMixRulesConfigDLG::OnBnClickedOk)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_RULE_TODO_CHECK, &CSchedulingMixRulesConfigDLG::OnBnClickedRuleTodoCheck)
	ON_BN_CLICKED(IDC_SELECT_COLOR_CHECK, &CSchedulingMixRulesConfigDLG::OnBnClickedSelectColorCheck)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDCANCEL, &CSchedulingMixRulesConfigDLG::OnBnClickedCancel)
	ON_EN_KILLFOCUS(IDC_EXPIRATION_EDIT, &CSchedulingMixRulesConfigDLG::OnEnKillfocusExpirationEdit)
END_MESSAGE_MAP()


// SchedulingMixRulesConfigDLG message handlers

BOOL CSchedulingMixRulesConfigDLG::OnInitDialog(){

	CNxDialog::OnInitDialog();
	ModifyStyle(WS_SYSMENU, 0);

	try{
		
		CString Title = " Scheduling Mix Rules - " + m_strRuleName;
		this->SetWindowText(_T(Title));
		m_btnAddTypes.AutoSet(NXB_NEW);
		m_btnCopyTo.AutoSet(NXB_NEW);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (s.tullis 2014-12-08 17:16) - PLID 64137 - Set Location Arrow Buttons
		m_btnLocation_Left.AutoSet(NXB_LEFT);
		m_btnLocation_Left_All.AutoSet(NXB_LLEFT);
		m_btnLocation_Right.AutoSet(NXB_RIGHT);
		m_btnLocation_Right_All.AutoSet(NXB_RRIGHT);
		m_btnInsurance_Left.AutoSet(NXB_LEFT);
		m_btnInsurance_Left_All.AutoSet(NXB_LLEFT);
		m_btnInsurance_Right.AutoSet(NXB_RIGHT);
		m_btnInsurance_Right_All.AutoSet(NXB_RRIGHT);
		// (s.tullis 2014-12-08 17:16) - PLID 64137 - Set Resource Arrow Button
		m_btnResource_Left.AutoSet(NXB_LEFT);
		m_btnResource_Right.AutoSet(NXB_RIGHT);
		// (s.tullis 2014-12-08 17:41) - PLID 64138 -  Set Multi Todo Users Label
		m_MultiAssignedTodoUsers.SetType(dtsDisabledHyperlink);
		m_MultiAssignedTodoUsers.SetSingleLine(true);

		m_checkExpiredDate.SetCheck(FALSE);
		m_dateRuleEnd.EnableWindow(FALSE);
		// (s.tullis 2014-12-08 17:16) - PLID 64137 - Location DataLists
		m_pSelectedLocations = BindNxDataList2Ctrl(IDC_SELECTED_LOCATIONS, false);
		m_pUnSelectedLocations = BindNxDataList2Ctrl(IDC_UNSELECTED_LOCATIONS, true);
		m_pSelectedInsurance = BindNxDataList2Ctrl(IDC_SELECTED_INSURANCE, false);
		m_pUnSelectedInsurance = BindNxDataList2Ctrl(IDC_UNSELECTED_INSURANCE, true);
		// (s.tullis 2014-12-08 17:16) - PLID 64137 - Rule Detail Datalists
		m_pResourceList = BindNxDataList2Ctrl(IDC_RULE_RESOURCE_LIST, false);
		CString strWhere;
		strWhere.Format(" Inactive=0 OR ID IN (Select ResourceID FROM ScheduleMixRuleDetailsT WHERE ruleID = %li)",m_nRuleID );
		m_pResourceList->PutWhereClause(_bstr_t(strWhere));
		m_pResourceList->Requery();
		m_pRuleDetailList = BindNxDataList2Ctrl(IDC_RULE_DETAIL_LIST, false);
		// (s.tullis 2014-12-08 17:41) - PLID 64138 - Todo DataLists
		m_pTodoCategory = BindNxDataList2Ctrl(IDC_TODO_CATEGORY, true);
		m_pTodoPriority = BindNxDataList2Ctrl(IDC_TODO_PRIORITY, false);
		m_pTodoUserAssigned = BindNxDataList2Ctrl(IDC_TODO_USERS_ASSIGNED, true);

		Load();

		

	}NxCatchAll(__FUNCTION__)

		return TRUE;
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedLocationRight()
{
	try{
		m_pSelectedLocations->TakeCurrentRowAddSorted(m_pUnSelectedLocations, NULL);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedLocationRightAll()
{
	try{
		m_pSelectedLocations->TakeAllRows(m_pUnSelectedLocations);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedLocationLeft()
{
	try{
		m_pUnSelectedLocations->TakeCurrentRowAddSorted(m_pSelectedLocations, NULL);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedLocationLeftAll()
{
	try{
		m_pUnSelectedLocations->TakeAllRows(m_pSelectedLocations);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceRight()
{
	try{
		m_pSelectedInsurance->TakeCurrentRowAddSorted(m_pUnSelectedInsurance, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceLeft()
{
	try{
		m_pUnSelectedInsurance->TakeCurrentRowAddSorted(m_pSelectedInsurance, NULL);
	}NxCatchAll(__FUNCTION__)
}



// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceRightAll()
{
	try{
		m_pSelectedInsurance->TakeAllRows(m_pUnSelectedInsurance);
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Move Support
void CSchedulingMixRulesConfigDLG::OnBnClickedInsuranceLeftAll()
{
	try{
		m_pUnSelectedInsurance->TakeAllRows(m_pSelectedInsurance);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Navigate resource list Right  -->
void CSchedulingMixRulesConfigDLG::OnBnClickedResourceRight()
{
	try{
		CString strWhereClause = "";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		if (pRow){
			pRow = pRow->GetNextRow();
			if (pRow){
				m_pResourceList->SetSelByColumn(RuleResourceList::ResourceID, pRow->GetValue(RuleResourceList::ResourceID));
			}
			else{
				pRow = m_pResourceList->GetFirstRow();
				if (pRow){
					m_pResourceList->SetSelByColumn(RuleResourceList::ResourceID, pRow->GetValue(RuleResourceList::ResourceID));
				}
			}
			ReflectRuleDetailListChange(VarLong(pRow->GetValue(RuleResourceList::ResourceID)));
		}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Navigate resource list left  <--
void CSchedulingMixRulesConfigDLG::OnBnClickedResourceLeft()
{
	try{
		CString strWhereClause = "";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		if (pRow){
			pRow = pRow->GetPreviousRow();
			if (pRow){
				m_pResourceList->SetSelByColumn(RuleResourceList::ResourceID, pRow->GetValue(RuleResourceList::ResourceID));
			}
			else{
				pRow = m_pResourceList->GetLastRow();
				if (pRow){
					m_pResourceList->SetSelByColumn(RuleResourceList::ResourceID, pRow->GetValue(RuleResourceList::ResourceID));
				}
			}

			ReflectRuleDetailListChange(VarLong(pRow->GetValue(RuleResourceList::ResourceID)));
		}
	}NxCatchAll(__FUNCTION__)
}

BEGIN_EVENTSINK_MAP(CSchedulingMixRulesConfigDLG, CNxDialog)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_RULE_RESOURCE_LIST, 2, CSchedulingMixRulesConfigDLG::SelChangedRuleResourceList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_TODO_USERS_ASSIGNED, 2, CSchedulingMixRulesConfigDLG::SelChangedTodoUsersAssigned, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_RULE_DETAIL_LIST, 9, CSchedulingMixRulesConfigDLG::EditingFinishingRuleDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_RULE_DETAIL_LIST, 10, CSchedulingMixRulesConfigDLG::EditingFinishedRuleDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_RULE_DETAIL_LIST, 6, CSchedulingMixRulesConfigDLG::RButtonDownRuleDetailList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_UNSELECTED_LOCATIONS, 3, CSchedulingMixRulesConfigDLG::DblClickCellUnselectedLocations, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_UNSELECTED_INSURANCE, 3, CSchedulingMixRulesConfigDLG::DblClickCellUnselectedInsurance, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_SELECTED_LOCATIONS, 3, CSchedulingMixRulesConfigDLG::DblClickCellSelectedLocations, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_SELECTED_INSURANCE, 3, CSchedulingMixRulesConfigDLG::DblClickCellSelectedInsurance, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSchedulingMixRulesConfigDLG, IDC_RULE_TYPE_COLOR, DISPID_CLICK, CSchedulingMixRulesConfigDLG::ClickRuleTypeColor, VTS_NONE)
END_EVENTSINK_MAP()

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Clear and Load Rule Detail List based on Resource switch
void CSchedulingMixRulesConfigDLG::SelChangedRuleResourceList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{

		if (lpOldSel == lpNewSel){
			return;
		}
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		NXDATALIST2Lib::IFormatSettingsPtr pfsDefault(__uuidof(NXDATALIST2Lib::FormatSettings));
		if (pRow){

			ReflectRuleDetailListChange(VarLong(pRow->GetValue(RuleResourceList::ResourceID)));

		}
		// (s.tullis 2014-12-08 17:16) - PLID 64137 - Default Rule Detail < All Appointment Types >
		if (m_pRuleDetailList->GetRowCount() == 0)
		{
			pRow = m_pRuleDetailList->GetNewRow();
			if (pRow){
				pRow->PutValue(RuleDetailList::RuleDetailID, -2);
				pRow->PutValue(RuleDetailList::ApptTypeID, -1);
				pRow->PutValue(RuleDetailList::PurposeID, -1);

				pfsDefault->PutDataType(VT_I4);
				pfsDefault->PutFieldType(NXDATALIST2Lib::cftComboSimple);
				pfsDefault->PutEditable(VARIANT_FALSE);

				pRow->PutRefCellFormatOverride(RuleDetailList::PurposeID, pfsDefault);

				m_pRuleDetailList->AddRowSorted(pRow, NULL);
			}
		}
	}NxCatchAll(__FUNCTION__)

}

// (s.tullis 2014-12-08 17:16) - PLID 64137 -Copy Rule Detail to one or more resources
void CSchedulingMixRulesConfigDLG::OnBnClickedRuleCopyTo()
{
	try{
		CString strWhereClause = "";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		if (pRow){

			CMultiSelectDlg dlg(this, "SchedulingMixRules");
			CString strWindowText = FormatString("Select the Resources you wish to copy.");
			CString strWhere = "";
			strWhere.Format("Inactive = 0  AND  ID <> %li ", VarLong(pRow->GetValue(RuleResourceList::ResourceID)));
			if (IDOK == dlg.Open("ResourceT", strWhere, "ID", "Item", strWindowText,1)){
				CArray<long, long> arynResourceIDs;
				CVariantArray arystrResourceNames;
				dlg.FillArrayWithIDs(arynResourceIDs);
				dlg.FillArrayWithNames(arystrResourceNames);
				if (arynResourceIDs.GetSize() == 0){ 
					return;
				}
				CString strWarningText = "";
				CString arystrCopyResources = "";
				CString strOriginalResource = VarString(pRow->GetValue(RuleResourceList::ResourceName));
				for (int i = 0; i < arystrResourceNames.GetSize(); i++){

					arystrCopyResources = arystrCopyResources + VarString(arystrResourceNames[i]) + "\r\n";
				}
				strWarningText.Format("You have selected to copy the appointment rules for %s to:\r\n"
					"%s\r\n"
					"This will overwrite any existing data and is unrecoverable. Are you sure you wish to continue?", strOriginalResource, arystrCopyResources);

				if (IDYES == AfxMessageBox(strWarningText, MB_YESNO )){
					CopyToResource(arynResourceIDs);
				}

			}
		}


	}NxCatchAll(__FUNCTION__)
}



// (s.tullis 2014-12-08 17:49) - PLID 64144 - Launch Add types Dialog 
void CSchedulingMixRulesConfigDLG::OnBnClickedRuleDetailAddTypes()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		long nResourceID;
		int nResult = IDC_RULE_DETAIL_SAVE_ADD_ANOTHER;
		RuleDetail *pDetails = NULL;
		if (pRow){
			nResourceID = VarLong(pRow->GetValue(RuleResourceList::ResourceID));
			while (nResult == IDC_RULE_DETAIL_SAVE_ADD_ANOTHER){
				CSchedulerMixRulesAddTypes dlg(m_nRuleID, nResourceID);
				nResult = dlg.DoModal();
				long nRuleDetailID = dlg.m_ruleDetail.nRuleDetailID;
				long nApptType = dlg.m_ruleDetail.nApptTypeID;
				long nAppTpurpose = dlg.m_ruleDetail.nPurposeID;
				long nApptMax = dlg.m_ruleDetail.nMaxappts;
				if (nResult == IDCANCEL || nApptMax == -1 ){
					return;
				}
				
				if (FindDetail(nResourceID, nApptType, nAppTpurpose) == NULL){
					std::map<long, RuleDetail*>::iterator it;
					it = m_mDetailRuleMap.find(nResourceID);
					if (it != m_mDetailRuleMap.end()){
						if (it->second->pDetailNext == NULL){
							it->second->pDetailNext = new RuleDetail(nRuleDetailID, nApptType, nAppTpurpose, nApptMax);
						}
						else{
							pDetails = it->second->pDetailNext;
							while (pDetails->pDetailNext != NULL){
								pDetails = pDetails->pDetailNext;
							}
							if (pDetails != NULL){
								pDetails->pDetailNext = new RuleDetail(nRuleDetailID, nApptType, nAppTpurpose, nApptMax);
							}
						}
					}
					else{
						m_mDetailRuleMap.insert(std::pair<long, RuleDetail*>(nResourceID, new RuleDetail(nRuleDetailID, nApptType, nAppTpurpose, nApptMax)));
					}
					ReflectRuleDetailListChange(nResourceID);
				}
				else{
					MessageBox("You have chosen an Appointment Type / Appointment Purpose combination that already exists for this resource.\r\n"
						"Scheduling Mix Rule Appointment Type / Purpose combinations are required to be unique per Resource.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					nResult = IDC_RULE_DETAIL_SAVE_ADD_ANOTHER;
					continue;
				}
			}
			
		}
	}NxCatchAll(__FUNCTION__)
}

void CSchedulingMixRulesConfigDLG::Save()
{
	try{
		CParamSqlBatch Sql;
		long newTodoTaskID=-1;
		BOOL bTodoDeletedTC = FALSE;
		BOOL bTodoAddedTC = FALSE;
		_variant_t vNull;
		_variant_t vdtEnd;
		 m_dtStartDate = m_dateRuleStart.GetDateTime();// (s.tullis 2014-12-08 16:49) - PLID 64135 - Get start and End date ( if there is one)
		 m_dtStartDate = COleDateTime(m_dtStartDate.GetYear(), m_dtStartDate.GetMonth(), m_dtStartDate.GetDay(), 0,0,0);
		
		 vNull.vt = VT_NULL;
		if (m_checkExpiredDate.GetCheck()){ 
			m_dtEndDate = m_dateRuleEnd.GetDateTime();
			m_dtEndDate = COleDateTime(m_dtEndDate.GetYear(), m_dtEndDate.GetMonth(), m_dtEndDate.GetDay(), 0, 0, 0);
			vdtEnd = _variant_t(m_dtEndDate, VT_DATE);
		}
		else{
			vdtEnd = vNull;
		}
		
		Sql.Add(" DECLARE @RuleID INT ");
		Sql.Add(" DECLARE @RuleColor INT ");
		Sql.Add(" DECLARE @TodoTaskID INT ");

		if (m_checkSetReminder.GetCheck())
		{
			newTodoTaskID = CreateTodo();
			Sql.Add(" SET @TodoTaskID = {INT} ", newTodoTaskID);
		}
		else{
			Sql.Add(" SET @TodoTaskID = NULL ");
		}

		if (m_checkSelectColor.GetCheck()){
			Sql.Add(" SET @RuleColor = {INT} ", m_nColor);
		}
		else{
			Sql.Add(" SET @RuleColor = NULL " );
		}

		// (s.tullis 2014-12-08 16:49) - PLID 64135 - Save Color Start and End Date
		switch (m_ConfigurationMode){
		case SchedulingMixRulesConfigMode::Create:
		case SchedulingMixRulesConfigMode::Copy:
			Sql.Add(" INSERT INTO ScheduleMixRulesT "
				"Values({STRING}, {VT_DATE}, {VT_DATE}, @RuleColor , @TodoTaskID) ", m_strRuleName, _variant_t(m_dtStartDate, VT_DATE), vdtEnd);
			//---------------------------------------------------
			Sql.Add("  SET @RuleID = (Select ID FROM ScheduleMixRulesT WHERE Name = {STRING} ) ", m_strRuleName);
			break;
		case SchedulingMixRulesConfigMode::Edit:
			Sql.Add("  SET @RuleID = {INT} ", m_nRuleID);
			//---------------------------------------------------
			Sql.Add("  UPDATE ScheduleMixRulesT "
				"  SET StartDate = {VT_DATE} , "
				"  EndDate = { VT_DATE } , "
				"  Color = @RuleColor , "
				"  TaskID = @TodoTaskID  "
				"  WHERE ID = @RuleID ", _variant_t(m_dtStartDate, VT_DATE), vdtEnd);

			if (m_nTodoTaskID != -1){
				Sql.Add(" Delete FROM TodoAssignToT "
					" Where TaskID = {INT} ", m_nTodoTaskID);

				Sql.Add("Delete FROM TodoList "
					" Where TaskID = {INT} ", m_nTodoTaskID);

				bTodoDeletedTC = TRUE;
			}
			break;
		default:
			ASSERT(FALSE);
			break;
		}

		if (newTodoTaskID != -1){
			Sql.Add(" Update Todolist "
				" SET RegardingID = @RuleID "
				" WHERE TaskID = @TodoTaskID ");
			bTodoAddedTC = TRUE;

		}

		

		GetRuleDetailsQuery(Sql);
		GetSelectedLocationQuery(Sql);
		GetSelectedInsQuery(Sql);

		Sql.Execute(GetRemoteData());
		// (s.tullis 2015-02-10 10:50) - PLID 64138 - tablechecker support
		if (bTodoDeletedTC)
		{
			CArray < long, long> arrAssignedID;
			ParseDelimitedStringToLongArray(m_strTodoAssignedIDS, " ", arrAssignedID);

			if (arrAssignedID.GetSize() == 1)
				CClient::RefreshTodoTable(m_nTodoTaskID, m_nRuleID, arrAssignedID[0], TableCheckerDetailIndex::tddisDeleted);
			else {// more than one User Assigned send the regular TableChecker
				CClient::RefreshTodoTable(m_nTodoTaskID, m_nRuleID, -1, TableCheckerDetailIndex::tddisDeleted);
			}
		}


		if (bTodoAddedTC)
		{
			if (m_arrAssignedTodoUsers.GetSize() == 1)
				CClient::RefreshTodoTable(newTodoTaskID, m_nRuleID, m_arrAssignedTodoUsers[0], TableCheckerDetailIndex::tddisAdded);
			else {// more than one User Assigned send the regular TableChecker
				CClient::RefreshTodoTable(newTodoTaskID, m_nRuleID, -1, TableCheckerDetailIndex::tddisAdded);
			}
		}

		//TES 2/11/2015 - PLID 64120 - We need to update the stored value to reflect that we definitely have a mix rule now, and tell the scheduler, if it's
		// open, to show the "View Mix Rules" button
		SetDatabaseHasMixRules();
		CNxView *pSchedView = GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
		if (pSchedView) {
			((CSchedulerView*)pSchedView)->ShowMixRulesButton();
		}

	}NxCatchAll(__FUNCTION__)
}


void CSchedulingMixRulesConfigDLG::Load()
{
	try{
		

		CString strWhereClause;

		SetTodoLists();
		
		
		// (s.tullis 2014-12-08 16:49) - PLID 64135 - Set Start And End Date.. Start Date defaults to day of creation
		m_dateRuleStart.SetValue(m_dtStartDate);
		if (m_dtEndDate.GetStatus() == COleDateTime::null){
			m_checkExpiredDate.SetCheck(FALSE);
			m_dateRuleEnd.EnableWindow(FALSE);
			m_checkSetReminder.SetCheck(FALSE);
			SetTodoControls(FALSE);
		}
		else{
			m_checkExpiredDate.SetCheck(TRUE);
			m_dateRuleEnd.EnableWindow(TRUE);
			m_dateRuleEnd.SetValue(m_dtEndDate);
		}
		// (s.tullis 2014-12-08 16:49) - PLID 64135 - Set Color.. defaults to light blue
		if (m_nColor== -1){
			m_checkSelectColor.SetCheck(FALSE);
			m_typeColor.EnableWindow(FALSE);
			m_typeColor.SetColor(16777088);
		}
		else{
			m_checkSelectColor.SetCheck(TRUE);
			m_typeColor.SetColor(m_nColor);
			m_typeColor.EnableWindow(TRUE);
		}
		// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Set Where Clauses for Unselected and Selected Location Lists .. Filter out Labs and Pharmacys and Inactive Locations
		strWhereClause.Format(" LocationsT.ID IN ( SELECT LOCATIONID FROM ScheduleMixRuleLocationsT WHERE RULEID= %li ) ", m_nRuleID);
		m_pSelectedLocations->WhereClause = _bstr_t(strWhereClause);

		strWhereClause.Format(" LocationsT.ID NOT IN ( SELECT LOCATIONID FROM ScheduleMixRuleLocationsT WHERE RULEID= %li ) AND LocationsT.Active = 1 AND TypeID = 1", m_nRuleID);
		m_pUnSelectedLocations->WhereClause = _bstr_t(strWhereClause);

		strWhereClause.Format(" InsuranceCoT.PersonID IN ( Select INSURANCECOID FROM ScheduleMixRuleInsuranceCosT WHERE RULEID= %li ) ", m_nRuleID);
		m_pSelectedInsurance->WhereClause = _bstr_t(strWhereClause);

		strWhereClause.Format(" InsuranceCoT.PersonID NOT IN ( Select INSURANCECOID FROM ScheduleMixRuleInsuranceCosT WHERE RULEID= %li ) AND PersonT.Archived = 0 ", m_nRuleID);
		m_pUnSelectedInsurance->WhereClause = _bstr_t(strWhereClause);

		m_pSelectedInsurance->Requery();
		m_pUnSelectedInsurance->Requery();
		m_pSelectedLocations->Requery();
		m_pUnSelectedLocations->Requery();

		m_pSelectedInsurance->Sort();
		m_pUnSelectedInsurance->Sort();
		m_pSelectedLocations->Sort();
		m_pUnSelectedLocations->Sort();

		LoadRuleDetails();
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 16:49) - PLID 64135 - Enable / Disable End Date pick based on check
void CSchedulingMixRulesConfigDLG::OnBnClickedSetDateCheck()
{
	try{
		if (m_checkExpiredDate.GetCheck() == TRUE){
			m_dateRuleEnd.EnableWindow(TRUE);
		}
		else{
			m_dateRuleEnd.EnableWindow(FALSE);
			m_checkSetReminder.SetCheck(FALSE);
			SetTodoControls(FALSE);
		}

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Get Location Query
void CSchedulingMixRulesConfigDLG::GetSelectedLocationQuery(CParamSqlBatch &batch){
	try{
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedLocations->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pRowNext;
		if (m_ConfigurationMode == SchedulingMixRulesConfigMode::Edit){
			batch.Add(" DELETE ScheduleMixRuleLocationsT FROM ScheduleMixRuleLocationsT "
					  " WHERE  RuleID = @RuleID ");
		}

		while (pRow){
			pRowNext = pRow->GetNextRow();
			batch.Add("Insert into ScheduleMixRuleLocationsT VALUES ( @RuleID,{INT})",  VarLong(pRow->GetValue(LocationList::LocationID)));
			pRow = pRowNext;
		}
	
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Get Ins Query
void CSchedulingMixRulesConfigDLG::GetSelectedInsQuery(CParamSqlBatch &batch){
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedInsurance->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pRowNext;
		if (m_ConfigurationMode == SchedulingMixRulesConfigMode::Edit){
			batch.Add("DELETE ScheduleMixRuleInsuranceCosT FROM ScheduleMixRuleInsuranceCosT "
					  " WHERE  ScheduleMixRuleInsuranceCosT.RuleID = @RuleID ");
		}
		while (pRow){
			pRowNext = pRow->GetNextRow();
			batch.Add(" Insert into ScheduleMixRuleInsuranceCosT VALUES ( @RuleID,{INT}) ",  VarLong(pRow->GetValue(LocationList::LocationID)));
			pRow = pRowNext;
		}

	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-08 17:46) - PLID 64134 - Make sure everything is filled.. save .. clear memory objects
void CSchedulingMixRulesConfigDLG::OnBnClickedOk()
{
	try{
		if (!Validate()){
			return;
		}
		Save();
		ClearAllResources();
		EndDialog(IDOK);
	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Copy one Resources Rule Details to One or more other Resources
void CSchedulingMixRulesConfigDLG::CopyToResource(CArray<long, long> &arrIDs){
	try{
		RuleDetail *pDetail;

		for (int i = 0; i < arrIDs.GetSize(); i++)
		{
			ClearResource(arrIDs[i]);
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleDetailList->GetFirstRow();
			NXDATALIST2Lib::IRowSettingsPtr pRowNext;
			std::map<long, RuleDetail*>::iterator it;
			it = m_mDetailRuleMap.find(arrIDs[i]);
			while (pRow)
			{
				
				pRowNext = pRow->GetNextRow();
				long nRuleDetaiLID = -1;
				long nApptTypeID = VarLong(pRow->GetValue(RuleDetailList::ApptTypeID), -1);
				long nPurposeID = VarLong(pRow->GetValue(RuleDetailList::PurposeID), -1);
				variant_t nMaxAppt =  pRow->GetValue(RuleDetailList::MaxAppts);
				if (nApptTypeID == -1 && nPurposeID == -1 && nMaxAppt.vt == VT_EMPTY)
				{
					ClearResource(arrIDs[i]);
					pRow = pRowNext;
					continue;
				}
				
				
				if (it != m_mDetailRuleMap.end()){
					if (pDetail->pDetailNext == NULL){
						pDetail->pDetailNext = new RuleDetail(nRuleDetaiLID, nApptTypeID, nPurposeID, VarLong(nMaxAppt));
						pDetail = pDetail->pDetailNext;
					}
				}
				else{
					m_mDetailRuleMap.insert(std::pair<long, RuleDetail*>(arrIDs[i], new RuleDetail(nRuleDetaiLID, nApptTypeID, nPurposeID,VarLong(nMaxAppt) )));
				}
				
				if (it == m_mDetailRuleMap.end()){
					it = m_mDetailRuleMap.find(arrIDs[i]);
					if (it != m_mDetailRuleMap.end()){
						pDetail = it->second;
					}
				}

				pRow = pRowNext;
			}

		}
		
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:41) - PLID 64138 -Set Todo List
void CSchedulingMixRulesConfigDLG::SetTodoLists(){
	try{

		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoPriority->GetNewRow();
		if (pRow){
			pRow->PutValue(TodoPriorityList::PriorityID, TodoPriority::ttpHigh);
			pRow->PutValue(TodoPriorityList::PriorityName, " High ");
			m_pTodoPriority->AddRowSorted(pRow, NULL);
		}

		pRow=m_pTodoPriority->GetNewRow();
		if (pRow){
			pRow->PutValue(TodoPriorityList::PriorityID, TodoPriority::ttpMedium);
			pRow->PutValue(TodoPriorityList::PriorityName, " Medium ");
			m_pTodoPriority->AddRowSorted(pRow, NULL);
		}

		pRow = m_pTodoPriority->GetNewRow();
		if (pRow){
			pRow->PutValue(TodoPriorityList::PriorityID, TodoPriority::ttpLow);
			pRow->PutValue(TodoPriorityList::PriorityName, " Low ");
			m_pTodoPriority->AddRowSorted(pRow, NULL);
		}

		pRow = m_pTodoUserAssigned->GetNewRow();
		if (pRow){
			pRow->PutValue(TodoAssignedUserList::UserID, -1);
			pRow->PutValue(TodoAssignedUserList::UserName, "< Multiple Users >");
			m_pTodoUserAssigned->AddRowSorted(pRow, NULL);
		}

		ADODB::_RecordsetPtr pRs = CreateParamRecordset(
			"Select Todolist.TaskID, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, dbo.GetTodoAssignToNamesString(TodoList.TaskID) as AssignedNames ,TodoList.CategoryID, TodoList.Priority, DATEDIFF(DAY,TodoList.Remind,ScheduleMixRulesT.EndDate) As RemindDays  "
			"FROM TodoList "
			"Inner Join ScheduleMixRulesT "
			"ON ScheduleMixRulesT.TaskID = TodoList.TaskID "
			"Where ScheduleMixRulesT.ID = { INT } ", m_nRuleID);

		if (!pRs->eof){
			m_checkSetReminder.SetCheck(TRUE);
			ADODB::FieldsPtr fields = pRs->Fields;
			m_strTodoAssignedIDS = AdoFldString(fields->Item["AssignedToIDs"]);
			m_nTodoTaskID = AdoFldLong(fields->Item["TaskID"]);
			m_pTodoCategory->SetSelByColumn(TodoCategoryList::CategoryID,fields->Item["CategoryID"]->Value);
			// Priority
			COleVariant var = fields->Item["Priority"]->Value;
			if (var.vt == VT_I2 || var.vt == VT_UI1) {
				switch (var.iVal) {
				case 1:
					m_pTodoPriority->SetSelByColumn(TodoPriorityList::PriorityID, TodoPriority::ttpHigh);
					break;
				case 2:
					m_pTodoPriority->SetSelByColumn(TodoPriorityList::PriorityID, TodoPriority::ttpMedium);
					break;
				default:
					m_pTodoPriority->SetSelByColumn(TodoPriorityList::PriorityID, TodoPriority::ttpLow);
					break;
				}
			}
			CString strReminderDays;
			strReminderDays.Format("%li", AdoFldLong(fields->Item["RemindDays"]));
			m_TodoDayExpired.SetWindowText(strReminderDays);
			SetTodoUsers(AdoFldString(fields->Item["AssignedToIDs"]), AdoFldString(fields->Item["AssignedNames"]));
		}
		else{
			m_pTodoPriority->SetSelByColumn(TodoPriorityList::PriorityID, TodoPriority::ttpLow);
			m_nTodoTaskID = -1;
		}
	  
		if (pRs->eof && !m_checkExpiredDate.GetCheck()){
			SetTodoControls(FALSE);
		}
		pRs->Close();
		
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:41) - PLID 64138 - Get Assigned Users 
void CSchedulingMixRulesConfigDLG::SelChangedTodoUsersAssigned(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
			if (pRow){
				if (VarLong(pRow->GetValue(TodoAssignedUserList::UserID)) == -1){// Multiple Selectedions
					HandleMultiUserSel();
				}
				else{
					m_arrAssignedTodoUsers.RemoveAll();
					m_arrAssignedTodoUsers.Add(VarLong(pRow->GetValue(TodoAssignedUserList::UserID)));
				}
			}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:41) - PLID 64138 - Label For Multi Todo Users
LRESULT CSchedulingMixRulesConfigDLG::OnLabelClick(WPARAM wParam, LPARAM lParam){
		try {
			UINT nIdc = (UINT)wParam;
			switch (nIdc)
			{
			case IDC_MULTI_USER_SELECT:
				HandleMultiUserSel();
				break;
			default:
				break;
			}
	}NxCatchAll(__FUNCTION__)
	return 0;
}

// (s.tullis 2014-12-08 17:41) - PLID 64138 - cursor changes to hand when hovering over hypertext
BOOL CSchedulingMixRulesConfigDLG::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try{
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_MultiAssignedTodoUsers.IsWindowVisible() && m_MultiAssignedTodoUsers.IsWindowEnabled()){
			GetDlgItem(IDC_MULTI_USER_SELECT)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__)

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}


// (s.tullis 2014-12-08 17:41) - PLID 64138 - Multi Todo Users Support
void CSchedulingMixRulesConfigDLG::HandleMultiUserSel(){
	try{
		CMultiSelectDlg dlg(this, "SchedulingMixRulesTodoAssignedUsers");
		CString strWindowText = FormatString("Select the User you wish to Assigned to the Todo.");

		
		for (int i = 0; i < m_arrAssignedTodoUsers.GetSize(); i++) {
			dlg.PreSelect(m_arrAssignedTodoUsers[i]);
		}
		
		if (IDOK == dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0 ", "PersonT.Archived = 0", "PersonID", "UserName", strWindowText,1)){
			m_arrAssignedTodoUsers.RemoveAll();
			dlg.FillArrayWithIDs(m_arrAssignedTodoUsers);

			if (m_arrAssignedTodoUsers.GetSize() == 1){
				GetDlgItem(IDC_TODO_USERS_ASSIGNED)->ShowWindow(SW_SHOW);
				m_MultiAssignedTodoUsers.ShowWindow(SW_HIDE);
				m_pTodoUserAssigned->SetSelByColumn(TodoAssignedUserList::UserID, m_arrAssignedTodoUsers[0]);
				return;
			}

			CString strMultiSelectString = dlg.GetMultiSelectString();
			if (strMultiSelectString.GetLength() > 255) {
				strMultiSelectString = strMultiSelectString.Left(255);
			}

			GetDlgItem(IDC_TODO_USERS_ASSIGNED)->ShowWindow(SW_HIDE);
			m_MultiAssignedTodoUsers.ShowWindow(SW_SHOW);
			m_MultiAssignedTodoUsers.SetText(strMultiSelectString);
			m_MultiAssignedTodoUsers.SetType(dtsHyperlink);
			m_MultiAssignedTodoUsers.SetToolTip(dlg.GetMultiSelectString("\r\n"));
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:41) - PLID 64138 - Multi Todo Users Support
void CSchedulingMixRulesConfigDLG::SetTodoUsers(CString strTodoAssigned, CString strTodoAsssignedName){
	try{
		if (strTodoAssigned.IsEmpty()){
			return;
		}

		CStringArray arrAssignedNames;
		ParseDelimitedStringToStringArray(strTodoAsssignedName, ",", arrAssignedNames);
		ParseDelimitedStringToLongArray(strTodoAssigned," ", m_arrAssignedTodoUsers);
		if (m_arrAssignedTodoUsers.GetSize() == 1)
		{
			m_pTodoUserAssigned->SetSelByColumn(TodoAssignedUserList::UserID, m_arrAssignedTodoUsers[0]);
			return;
		}
		else if (m_arrAssignedTodoUsers.GetSize() > 1){

			CString strMultiUsers = GenerateDelimitedListFromStringArray(arrAssignedNames, ",");
			CString strMultiSelToolTip = GenerateDelimitedListFromStringArray(arrAssignedNames, "\r\n");
			GetDlgItem(IDC_TODO_USERS_ASSIGNED)->ShowWindow(SW_HIDE);
			m_MultiAssignedTodoUsers.ShowWindow(SW_SHOW);
			m_MultiAssignedTodoUsers.SetText(strMultiUsers);
			m_MultiAssignedTodoUsers.SetType(dtsHyperlink);
			m_MultiAssignedTodoUsers.SetToolTip(strMultiSelToolTip);
		}

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:41) - PLID 64138 - Function to set todo controls on and off
void CSchedulingMixRulesConfigDLG::SetTodoControls(BOOL bonOFF){
	try{
			m_checkSetReminder.SetCheck(bonOFF);
			m_pTodoPriority->PutEnabled(bonOFF);
			m_pTodoCategory->PutEnabled(bonOFF);
			m_pTodoUserAssigned->PutEnabled(bonOFF);
			m_TodoDayExpired.EnableWindow(bonOFF);
			m_MultiAssignedTodoUsers.EnableWindow(bonOFF);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Load Rule Details in Memory
void CSchedulingMixRulesConfigDLG::LoadRuleDetails()
{
	try{
		long nprevResourceID=-1;
		long nFirstResource = -1;
		RuleDetail *pDetail;
		//nothing to load if creation mode
		if (m_ConfigurationMode == SchedulingMixRulesConfigMode::Create){
			
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetFirstRow();
				if (pRow){
					m_pResourceList->PutCurSel(pRow);
					ReflectRuleDetailListChange(VarLong(pRow->GetValue(RuleResourceList::ResourceID)));
				}
			
				return;
		}
		
		ADODB:: _RecordsetPtr rsRuleDetails = CreateParamRecordset("Select ScheduleMixRuleDetailsT.ID AS DetailID,  "
			"ResourceID, AptPurposeT.ID as PurposeID, (CASE When AptPurposeT.Name IS NULL THEN ' < All Purpose Types > 'else AptPurposeT.Name END) as PurposeName, "
			"AptTypeT.ID AS TypeID, (CASE When AptTypeT.name IS NULL THEN ' < All Appointment Types > 'else AptTypeT.name END) as AptName, MaxAppts, ResourceT.Inactive "
			" FROM ScheduleMixRuleDetailsT LEFT JOIN AptTypeT ON AptTypeT.ID = ScheduleMixRuleDetailsT.AptTypeID "
			"LEFT JOIN AptPurposeT ON AptPurposeT.ID = ScheduleMixRuleDetailsT.AptPurposeID "
			"LEFT JOIN ResourceT ON ResourceT.ID = ScheduleMixRuleDetailsT.ResourceID "
			"WHERE ScheduleMixRuleDetailsT.RuleID = {INT} "
			"ORDER BY ResourceID ", m_nRuleID);


		ADODB::FieldsPtr fields = rsRuleDetails->Fields;
		while (!rsRuleDetails->eof){
			
			long nResourceID = AdoFldLong(fields->Item["ResourceID"]);
			long nRuleDetailID = AdoFldLong(fields->Item["DetailID"]);
			long nApptTypeID = AdoFldLong(fields->Item["TypeID"], -1);
			long nPurposeID = AdoFldLong(fields->Item["PurposeID"], -1);
			long nMaxappts = AdoFldLong(fields->Item["MaxAppts"]);
			BOOL bInactive = AdoFldBool(fields->Item["Inactive"]);

			if (nFirstResource == -1  ){
				nFirstResource = nResourceID;
			}

			if (nResourceID != nprevResourceID){
				
				m_mDetailRuleMap.insert(std::pair<long, RuleDetail*>(nResourceID, new RuleDetail(nRuleDetailID, nApptTypeID,  nPurposeID,  nMaxappts)));
				nprevResourceID = nResourceID;
				pDetail = m_mDetailRuleMap[nResourceID];
			}
			else{
				pDetail->pDetailNext= new RuleDetail(nRuleDetailID, nApptTypeID, nPurposeID, nMaxappts);
				pDetail = pDetail->pDetailNext;
			}
			
		     rsRuleDetails->MoveNext();
		}

		rsRuleDetails->Close();

		if (nFirstResource != -1){
			m_pResourceList->SetSelByColumn(RuleResourceList::ResourceID, nFirstResource);
			ReflectRuleDetailListChange(nFirstResource);
		}
		else{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceList->GetFirstRow();
				if (pRow){
					m_pResourceList->PutCurSel(pRow);
					ReflectRuleDetailListChange(VarLong(pRow->GetValue(RuleResourceList::ResourceID)));
				}
			
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Add Rule Detail to the datalist
// Overide the combo source to make each Row have a Purpose combo based on that rows Appt ID
void CSchedulingMixRulesConfigDLG::AddRuleDetail(RuleDetail &rd){
	try{
		NXDATALIST2Lib::IFormatSettingsPtr pfsApptPurpose(__uuidof(NXDATALIST2Lib::FormatSettings));
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleDetailList->GetNewRow();
		if (pRow){
			pRow->PutValue(RuleDetailList::DetailresourceID, rd.nRuleDetailID);
			pRow->PutValue(RuleDetailList::ApptTypeID, rd.nApptTypeID);
			pRow->PutValue(RuleDetailList::PurposeID, rd.nPurposeID);
			pRow->PutValue(RuleDetailList::MaxAppts, rd.nMaxappts);
			m_pRuleDetailList->AddRowSorted(pRow,NULL);

			pfsApptPurpose->PutDataType(VT_I4);
			pfsApptPurpose->PutFieldType(NXDATALIST2Lib::cftComboSimple);
			// (s.tullis 2014-12-08 17:16) - PLID 64137 - if were adding < All Appt > then disable the purpose edit
			if (rd.nApptTypeID != -1){
				pfsApptPurpose->PutEditable(VARIANT_TRUE);
			}
			else{
				pfsApptPurpose->PutEditable(VARIANT_FALSE);
			}
			pfsApptPurpose->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
			pfsApptPurpose->PutComboSource(_bstr_t(GetApptPurposeComboSource(rd.nApptTypeID, m_nRuleID)));
			pfsApptPurpose->EmbeddedComboDropDownMaxHeight = 200;
			pfsApptPurpose->EmbeddedComboDropDownWidth = 250;
			

			pRow->PutRefCellFormatOverride(RuleDetailList::PurposeID, pfsApptPurpose);
		}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Get Purpose Combo Data .. based on what Appt type is linked 
CString CSchedulingMixRulesConfigDLG::GetApptPurposeComboSource(long nApptTypeID /*=-1*/,long nRuleID /*=-1*/ )
{
		return FormatString(
			"Select * FROM("
				"Select -1 as TypeID, '< All Appointment Purposes >' as Name "
				"Union ALL "
				"Select Distinct AptPurposeT.ID AS TypeID, AptPurposeT.Name as Name FROM AptPurposeT "
				"LEFT JOIN AptPurposeTypeT "
				"ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID "
				"LEFT JOIN ScheduleMixRuleDetailsT "
				"ON AptPurposeT.ID = ScheduleMixRuleDetailsT.AptPurposeID "
				"WHERE AptPurposeTypeT.AptTypeID = %li OR ScheduleMixRuleDetailsT.RuleID = %li "
			") AptPurpose "
			" ORDER BY CASE WHEN TypeID = -1 THEN 0 ELSE 1 END, Name ", nApptTypeID, nRuleID);

}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Get Appt type Combo Data
CString CSchedulingMixRulesConfigDLG::GetApptTypeComboSource()
{
		return "Select * FROM ( "
					 " SELECT -1 AS ID, '< All Appointment Types >' AS Text "
					 " UNION ALL "
					 " SELECT ID, Name AS Text FROM AptTypeT "
				" ) AptType "
				" ORDER BY CASE WHEN ID=-1 THEN 0 ELSE 1 END, Text";
	
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Checkto see that the User is Not setting Max # of appointments to nothing or negative
// Also Check that the changing purpose does not conflict with existing Appt Type - Purpose combination for the resource
void CSchedulingMixRulesConfigDLG::EditingFinishingRuleDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{

		if (*pbCommit == FALSE)
			return;
		CString strUserInput(strUserEntered);
		strUserInput.TrimLeft();
		long nResourceID;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pRowResource = m_pResourceList->GetCurSel();
		if (pRowResource){
			nResourceID = VarLong(pRowResource->GetValue(RuleResourceList::ResourceID));
		}

		switch (nCol) {
		case RuleDetailList::MaxAppts:
			if (VarLong(pvarNewValue) < 0){
				MessageBox("The maximum number of Appointments must be a number greater than or equal to 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
			if (strUserInput.IsEmpty()  && varOldValue.vt != VT_EMPTY ){
				MessageBox("The maximum number of Appointments must be a number greater than or equal to 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
			
		
		break;
		case RuleDetailList::PurposeID:
			if (pRow){
				if (!(FindDetail(nResourceID, VarLong(pRow->GetValue(RuleDetailList::ApptTypeID)), VarLong(pvarNewValue))) == NULL)
				{
					MessageBox("Scheduling Mix Rule Appointment Type and Purpose combinations need to be unique per Resource.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
			break;
			}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Update the field Changed in the datalist
void CSchedulingMixRulesConfigDLG::EditingFinishedRuleDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if (varNewValue.vt == VT_EMPTY){
			return;
		}
		long nResourceID;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pRowResource = m_pResourceList->GetCurSel();
		if (pRowResource){
			nResourceID = VarLong(pRowResource->GetValue(RuleResourceList::ResourceID));
		}
		
		if (pRow){
			switch (nCol)
			{// (s.tullis 2014-12-08 17:16) - PLID 64137 - update Max Appt in memory
			case RuleDetailList::MaxAppts:
				UpdateRuleDetail(pRow, nResourceID);
				break;
			case RuleDetailList::PurposeID:// (s.tullis 2014-12-08 17:16) - PLID 64137 -Update Purpose in Memory 
				UpdateRuleDetail(pRow, nResourceID, VarLong(varOldValue));
			default:
				break;
			}

		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:41) - PLID 64138 -  Reminder Check event.. enable / disable todo controls based on check.. dont't allow reminder if no Rule End Date
void CSchedulingMixRulesConfigDLG::OnBnClickedRuleTodoCheck()
{
	try{
		if (m_checkExpiredDate.GetCheck()){
			SetTodoControls(m_checkSetReminder.GetCheck());
		}
		else{
			MessageBox("A rule end date is required for a rule to have a reminder.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			m_checkSetReminder.SetCheck(FALSE);
			m_checkExpiredDate.SetFocus();
		}
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-08 16:49) - PLID 64135 - Event for Color Check.. default color to light blue
void CSchedulingMixRulesConfigDLG::OnBnClickedSelectColorCheck()
{
	try{
		if (m_nColor == -1){
			m_nColor = 16777088;
		}
		m_typeColor.EnableWindow(m_checkSelectColor.GetCheck());
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:34) - PLID 64138 - Get required data and call TodoUtils::TodoCreate.. return the newly created ID
long CSchedulingMixRulesConfigDLG::CreateTodo()
{
	try{
		long nTodoCategory=-1;
		long nTodoPriority;
		long nNumDaysToRemind;
		CString strWindowText="";
		COleDateTime dtRemind;
		NXDATALIST2Lib::IRowSettingsPtr pRow= m_pTodoCategory->GetCurSel();
		if (pRow){
			nTodoCategory = pRow->GetValue(TodoCategoryList::CategoryID);
		}

		pRow = m_pTodoPriority->GetCurSel();
		if (pRow){
			nTodoPriority = pRow->GetValue(TodoPriorityList::PriorityID);
		}
		GetDlgItem(IDC_EXPIRATION_EDIT)->GetWindowText(strWindowText);
		if (!strWindowText.IsEmpty()){
			nNumDaysToRemind = atol(strWindowText);
			//
			COleDateTime dtBuffer(m_dtEndDate.GetYear(), m_dtEndDate.GetMonth(), m_dtEndDate.GetDay(), 0, 0, 0);
			COleDateTimeSpan ts(nNumDaysToRemind,0,0,0);
			dtRemind = dtBuffer - ts;
		}
		else{
			dtRemind = m_dtEndDate;
		}
		CString Note = "";
		
		Note.Format("Scheduling Mix Rule \"%s\" is expiring on %s.", m_strRuleName, FormatDateTimeForInterface(m_dtEndDate, NULL, dtoDate));
		
		return TodoCreate(
			dtRemind,
			m_dtEndDate,
			m_arrAssignedTodoUsers,
			Note,
			"",
			-1,
			ttScheduleMixRules,
			-1,
			-1,
			TodoPriority(nTodoPriority),
			nTodoCategory);


	}NxCatchAll(__FUNCTION__)

		return -1;
}


// (s.tullis 2014-12-08 17:16) - PLID 64137 - Clear the List and Add the Rule Details attatched to a Resource
void CSchedulingMixRulesConfigDLG::ReflectRuleDetailListChange(long nResourceID)
{
	try{
		m_pRuleDetailList->Clear();
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pRuleDetailList->GetColumn(short(RuleDetailList::PurposeID));
		pCol = m_pRuleDetailList->GetColumn(short(RuleDetailList::ApptTypeID));
		pCol->PutComboSource(_bstr_t(GetApptTypeComboSource()));
		

		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.find(nResourceID);
		if (it != m_mDetailRuleMap.end()){
			RuleDetail *pRuleDetail;
			pRuleDetail = it->second;
			while (pRuleDetail != NULL){
				AddRuleDetail(*pRuleDetail);
				pRuleDetail = pRuleDetail->pDetailNext;
			}
		}
		else{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleDetailList->GetNewRow();
			NXDATALIST2Lib::IFormatSettingsPtr pfsDefault(__uuidof(NXDATALIST2Lib::FormatSettings));
			if (pRow){
				pRow->PutValue(RuleDetailList::RuleDetailID, -2);
				pRow->PutValue(RuleDetailList::ApptTypeID, -1);
				pRow->PutValue(RuleDetailList::PurposeID, -1);

				pfsDefault->PutDataType(VT_I4);
				pfsDefault->PutFieldType(NXDATALIST2Lib::cftComboSimple);
				pfsDefault->PutEditable(VARIANT_FALSE);

				pRow->PutRefCellFormatOverride(RuleDetailList::PurposeID, pfsDefault);

				m_pRuleDetailList->AddRowSorted(pRow, NULL);
			}
		}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Get Rule Details Save Query 
void CSchedulingMixRulesConfigDLG::GetRuleDetailsQuery(CParamSqlBatch &batch)
{
	try{
		if (m_ConfigurationMode == SchedulingMixRulesConfigMode::Edit){
			batch.Add(" DELETE ScheduleMixRuleDetailsT FROM ScheduleMixRuleDetailsT "
				  " WHERE  RuleID = @RuleID ");
		}
		RuleDetail *pDetail;
		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.begin();
		while (it != m_mDetailRuleMap.end())
		{
			pDetail = it->second;
			while (pDetail != NULL ){

					_variant_t vNull;
					vNull.vt = VT_NULL;
					_variant_t vApptType = (pDetail->nApptTypeID > 0) ? _variant_t(pDetail->nApptTypeID) : vNull;
					_variant_t vApptPurpose = (pDetail->nPurposeID > 0) ? _variant_t(pDetail->nPurposeID) : vNull;

					batch.Add(" INSERT INTO ScheduleMixRuleDetailsT "
						" Values (@RuleID,{INT}, {VT_I4},{VT_I4}, {INT})", it->first, vApptType, vApptPurpose, pDetail->nMaxappts);

					pDetail = pDetail->pDetailNext;
			
				}
				++it;
		} 
		}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Clear Resource in Memory
void CSchedulingMixRulesConfigDLG::ClearResource(long nResource)
{
	try{
		RuleDetail *pDetail;
		RuleDetail *pDelete;
		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.find(nResource);
		if (it != m_mDetailRuleMap.end()){
			pDetail = it->second;
			while (pDetail != NULL){
				pDelete = pDetail;
				pDetail = pDetail->pDetailNext;
				delete[] pDelete;
			}
			m_mDetailRuleMap.erase(nResource);
		}
		
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:16) - PLID 64137 - Search for detail in the memory object.. return NULL if not found
RuleDetail* CSchedulingMixRulesConfigDLG::FindDetail(long nResourceID, long nApptTypeID, long nPurposeID)
{
	try{
		RuleDetail *pDetail;
		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.find(nResourceID);
		if (it != m_mDetailRuleMap.end()){
			pDetail = it->second;
			while (pDetail != NULL){
				if (pDetail->nPurposeID == nPurposeID && pDetail->nApptTypeID == nApptTypeID)
				{
					return pDetail;
				}
				pDetail = pDetail->pDetailNext;
			}
		}
	}NxCatchAll(__FUNCTION__)

	return NULL;
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Find and Delete Detail.. based off Resource ,ApptType, Purpose
//  Resource , nApptTypeID, and Purpose are required to be unique
BOOL CSchedulingMixRulesConfigDLG::FindDetailDelete(long nResourceID, long nApptTypeID, long nPurposeID)
{
	try{
		RuleDetail *pDetailDelete;
		RuleDetail *pDetailBefore;
		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.find(nResourceID);
		if (it != m_mDetailRuleMap.end())
		{
			pDetailDelete = it->second;
			pDetailBefore = it->second;
			while (pDetailDelete != NULL)
			{
				if (pDetailDelete->nPurposeID == nPurposeID && pDetailDelete->nApptTypeID == nApptTypeID)
				{
					//  first record.. 
					if (pDetailDelete == pDetailBefore){
						// only one record for this resource
						if (pDetailDelete->pDetailNext == NULL)
						{
							m_mDetailRuleMap.erase(it);
						}
						else// point the map pointer to the next value
						{
							it->second = pDetailDelete->pDetailNext;
						}
					}
					else {// else connect before and after 
						//note pDetailDelete->pDetailNext can be NULL (Last Record) which will just set pDetailBefore->pDetailNext to NULL (New Last Record)
						pDetailBefore->pDetailNext = pDetailDelete->pDetailNext;
					}
					delete[] pDetailDelete;
					return TRUE;
				}
				pDetailBefore = pDetailDelete;
				pDetailDelete = pDetailDelete->pDetailNext;
			}
		}
	}NxCatchAll(__FUNCTION__)
	return FALSE;
}

// (s.tullis 2014-12-08 16:48) - PLID 64134 - right click.. prompt menu to delete RuleDetail from datalist
void CSchedulingMixRulesConfigDLG::RButtonDownRuleDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow){
			// don't prompt for the default value .. can't delete it
			if (VarLong(pRow->GetValue(RuleDetailList::ApptTypeID), -1) == -1 && VarLong(pRow->GetValue(RuleDetailList::PurposeID), -1) && pRow->GetValue(RuleDetailList::MaxAppts).vt == VT_EMPTY)
			{
				return;
			}

			m_pRuleDetailList->PutCurSel(pRow);
			CMenu mnu;
			CPoint pt;
			GetCursorPos(&pt);
			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 1, "&Delete");
			long nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
			mnu.DestroyMenu();
			switch (nMenuChoice)
			{
			case 1:
				DeleteRuleDetailRow(pRow);
				break;
			default:
				break;
			}
		}
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-08 16:48) - PLID 64134 -  Remove RuleDetail from datalist and Memory
BOOL CSchedulingMixRulesConfigDLG::DeleteRuleDetailRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pResourceRow;
		if (pRow){
			long nApptID = VarLong(pRow->GetValue(RuleDetailList::ApptTypeID));
			long nPurposeID = VarLong(pRow->GetValue(RuleDetailList::PurposeID));
			pResourceRow = m_pResourceList->GetCurSel();
			if (pResourceRow)
			{
				long nResourceID = VarLong(pResourceRow->GetValue(RuleResourceList::ResourceID));
				if (FindDetailDelete(nResourceID, nApptID, nPurposeID))
				{
					ReflectRuleDetailListChange(nResourceID);
				}
				
			}
		}
	}NxCatchAll(__FUNCTION__)

		return FALSE;
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Double Click Move Support
void CSchedulingMixRulesConfigDLG::DblClickCellUnselectedLocations(LPDISPATCH lpRow, short nColIndex)
{
	try{
		    m_pSelectedLocations->TakeCurrentRowAddSorted(m_pUnSelectedLocations, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Double Click Move Support
void CSchedulingMixRulesConfigDLG::DblClickCellUnselectedInsurance(LPDISPATCH lpRow, short nColIndex)
{
	try{
			m_pSelectedInsurance->TakeCurrentRowAddSorted(m_pUnSelectedInsurance, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Double Click Move Support
void CSchedulingMixRulesConfigDLG::DblClickCellSelectedLocations(LPDISPATCH lpRow, short nColIndex)
{
	try{
		m_pUnSelectedLocations->TakeCurrentRowAddSorted(m_pSelectedLocations, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-08 17:04) - PLID 64136 -  Double Click Move Support
void CSchedulingMixRulesConfigDLG::DblClickCellSelectedInsurance(LPDISPATCH lpRow, short nColIndex)
{
	try{
		m_pUnSelectedInsurance->TakeCurrentRowAddSorted(m_pSelectedInsurance, NULL);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 16:49) - PLID 64135 - lauch common dialog color picker with current color and get selected color
void CSchedulingMixRulesConfigDLG::ClickRuleTypeColor()
{
	try{
		m_ctrlRuleColorPicker.SetColor(m_nColor);
		m_ctrlRuleColorPicker.ShowColor();
		m_nColor = m_ctrlRuleColorPicker.GetColor();
		m_typeColor.SetColor(m_nColor);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Update Resource When its manually edited in the datalist
void CSchedulingMixRulesConfigDLG::UpdateRuleDetail(NXDATALIST2Lib::IRowSettingsPtr pRow,long nResourceID, long nOldApptPurposeValue)
{
	try{
		if (pRow){//
			RuleDetail *pDetailUpdate;
			//default value
			_variant_t var =pRow->GetValue(RuleDetailList::RuleDetailID);
			if (var.vt != VT_EMPTY){
				if (var.lVal == -2)
				{
					m_mDetailRuleMap.insert(std::pair<long, RuleDetail*>(nResourceID, new RuleDetail(-2, -1, -1, VarLong(pRow->GetValue(RuleDetailList::MaxAppts)))));
					ReflectRuleDetailListChange(nResourceID);
					return;
				}
			}
			//were changing the Max Appointments need current values for detail lookup
			if (nOldApptPurposeValue == -2){
				pDetailUpdate = FindDetail(nResourceID, VarLong(pRow->GetValue(RuleDetailList::ApptTypeID)), VarLong(pRow->GetValue(RuleDetailList::PurposeID)));
			}
			else{// changing the Appt Purpose so we need the old purpose value for the lookup
				pDetailUpdate = FindDetail(nResourceID, VarLong(pRow->GetValue(RuleDetailList::ApptTypeID)), nOldApptPurposeValue);
			}
			

			if (pDetailUpdate != NULL){
				pDetailUpdate->nPurposeID = VarLong(pRow->GetValue(RuleDetailList::PurposeID));
				pDetailUpdate->nMaxappts = VarLong(pRow->GetValue(RuleDetailList::MaxAppts));
			}
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2014-12-08 17:16) - PLID 64137 - Delete All Resource objects.. don't need any memory leaks!
void CSchedulingMixRulesConfigDLG::ClearAllResources()
{
	try{
		std::map<long, RuleDetail*>::iterator it = m_mDetailRuleMap.begin();
		RuleDetail *pDetail;
		RuleDetail *pDetailNext;
		
		while (it != m_mDetailRuleMap.end())
		{
			for (pDetail = it->second; pDetail != NULL; pDetail = pDetailNext)
			{
				pDetailNext = pDetail->pDetailNext;
				delete[] pDetail;
			}
			it++;
		}
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-08 17:39) - PLID 64134 - Clear all Memory Objects and end dialog
void CSchedulingMixRulesConfigDLG::OnBnClickedCancel()
{
	try{
		ClearAllResources();
		EndDialog(IDCANCEL);
	}NxCatchAll(__FUNCTION__)
}

BOOL CSchedulingMixRulesConfigDLG::Validate()
{
	
	try{
		// (s.tullis 2014-12-08 17:40) - PLID 64138 - Can't save without Users Assigned to the Todo
		if (m_checkSetReminder.GetCheck())
		{
			if (m_arrAssignedTodoUsers.IsEmpty())
			{
				MessageBox("A To-Do requires an assigned user. Please assign a user to the To-Do to save the rule.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
			// only allow 4 digt numbers in the days field
			if (!(GetDlgItem(IDC_EXPIRATION_EDIT)->GetWindowTextLength() < 5))
			{
				MessageBox("The number of days to remind before the rule end date must be less than 5 digits. Please change the number of days to remind to save the rule.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				GetDlgItem(IDC_EXPIRATION_EDIT)->SetFocus();
				return FALSE;
			}
			// should not be negative
			CString strWindowText = "";
			m_TodoDayExpired.GetWindowText(strWindowText);
			if (!strWindowText.IsEmpty()){
				long nRemindDays = atol(strWindowText);
				if (nRemindDays < 0){
					MessageBox("The reminder date is required to be before or on the rule expiration date.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					m_TodoDayExpired.SetWindowText("");
					m_TodoDayExpired.SetFocus();
					return FALSE;
				}
			}
			else{
				MessageBox("The reminder date is required to be before or on the rule expiration date.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				m_TodoDayExpired.SetWindowText("");
				m_TodoDayExpired.SetFocus();
				return FALSE;
			}
			
		}
		// (s.tullis 2014-12-08 16:49) - PLID 64135 - End date should be after the start date
		if (m_checkExpiredDate.GetCheck())
		{
			COleDateTime dtStart = m_dateRuleStart.GetDateTime();
			COleDateTime dtEnd = m_dateRuleEnd.GetDateTime();
			if (dtStart > dtEnd){
				MessageBox("The rule end date is required to be after the rule start date.\r\nPlease change the rule end date to save the rule.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
		}
	}NxCatchAll(__FUNCTION__)

		return TRUE;
}
// (s.tullis 2014-12-08 17:41) - PLID 64138 - Reminder Days should not be negative
void CSchedulingMixRulesConfigDLG::OnEnKillfocusExpirationEdit()
{
	try{
		CString strWindowText = "";
		m_TodoDayExpired.GetWindowText(strWindowText);
		if (!strWindowText.IsEmpty()){
			long nRemindDays = atol(strWindowText);
			if (nRemindDays < 0){
				MessageBox("The Number of days to Remind should be greater than or equal to 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				m_TodoDayExpired.SetWindowText("");
				m_TodoDayExpired.SetFocus();
			}
		}

	}NxCatchAll(__FUNCTION__)
}


