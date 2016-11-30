// PhaseStepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PhaseStepDlg.h"
#include "PhaseTracking.h"
#include "GlobalDataUtils.h"
#include "PromptDlg.h"
#include "GlobalDrawingUtils.h"
#include "MultiSelectDlg.h"
#include "MultiTemplateDlg.h"
#include "AdministratorRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
using namespace PhaseTracking;
using namespace NXDATALISTLib;

enum UserListColumns {

	ulcID = 0,
	ulcName,
};

/////////////////////////////////////////////////////////////////////////////
// CPhaseStepDlg dialog

CPhaseStepDlg::CPhaseStepDlg()
	: CNxDialog(CPhaseStepDlg::IDD, NULL)
{
	m_nTemplateID = -1;
	m_bIsTracked = false;
	m_bActionIdsChanged = false;
	//{{AFX_DATA_INIT(CPhaseStepDlg)
	//}}AFX_DATA_INIT
}
/////////////////////////////////////////////////////////////////////////////
void CPhaseStepDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhaseStepDlg)
	DDX_Control(pDX, IDC_ASSIGN_TO_MULTIPLE_LABEL, m_nxstaticAssignToMultiple);
	DDX_Control(pDX, IDC_CHECK_OPEN_PIC_WHEN_SELECTED, m_checkOpenPICWhenSelected);
	DDX_Control(pDX, IDC_IMMEDIATE, m_btnImmediate);
	DDX_Control(pDX, IDC_TIME_AFTER, m_btnTimeAfter);
	DDX_Control(pDX, IDC_ASK, m_btnAsk);
	DDX_Control(pDX, IDC_NEVER, m_btnNever);
	DDX_Control(pDX, IDC_ALLOW_SKIP, m_btnAllowSkip);
	DDX_Control(pDX, IDC_STEP_CREATE_TODO, m_btnCreateTodo);
	DDX_Control(pDX, IDC_TODO_PRIORITY, m_cbPriority);
	DDX_Control(pDX, IDC_INTERVAL_TYPE, m_cbIntervalType);
	DDX_Control(pDX, IDOK, m_okBtn);
	DDX_Control(pDX, IDCANCEL, m_cancelBtn);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_NOTE, m_nxeditNote);
	DDX_Control(pDX, IDC_INTERVAL_NUMBER, m_nxeditIntervalNumber);
	DDX_Control(pDX, IDC_STEP, m_nxeditStep);
	DDX_Control(pDX, IDC_PREV_STEP_LABEL, m_nxstaticPrevStepLabel);
	DDX_Control(pDX, IDC_CRITERIA_LIST, m_nxstaticCriteriaList);
	DDX_Control(pDX, IDC_MERGE_SCOPE_LABEL, m_nxstaticMergeScopeLabel);
	DDX_Control(pDX, IDC_MERGE_SCOPE_HELP, m_nxbMergeScopeHelp);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CPhaseStepDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPhaseStepDlg)
	ON_EN_KILLFOCUS(IDC_STEP, OnKillfocusStep)
	ON_BN_CLICKED(IDC_IMMEDIATE, OnImmediate)
	ON_BN_CLICKED(IDC_TIME_AFTER, OnTimeAfter)
	ON_BN_CLICKED(IDC_ASK, OnAsk)
	ON_BN_CLICKED(IDC_NEVER, OnNever)
	ON_BN_CLICKED(IDC_PROMPT, OnPrompt)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_STEP_CREATE_TODO, OnStepCreateTodo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MERGE_SCOPE_HELP, &CPhaseStepDlg::OnMergeScopeHelp)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPhaseStepDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPhaseStepDlg)
	ON_EVENT(CPhaseStepDlg, IDC_ACTION, 2 /* SelChanged */, OnSelChangedAction, VTS_I4)
	ON_EVENT(CPhaseStepDlg, IDC_ITEM, 16 /* SelChosen */, OnSelChosenItem, VTS_I4)
	ON_EVENT(CPhaseStepDlg, IDC_ASSIGN_USERS, 18 /* RequeryFinished */, OnRequeryFinishedAssignUsers, VTS_I2)
	ON_EVENT(CPhaseStepDlg, IDC_ACTION, 16 /* SelChosen */, OnSelChosenAction, VTS_I4)
	ON_EVENT(CPhaseStepDlg, IDC_TODO_CATEGORY, 18 /* RequeryFinished */, OnRequeryFinishedTodoCategory, VTS_I2)
	ON_EVENT(CPhaseStepDlg, IDC_TODO_CATEGORY, 16 /* SelChosen */, OnSelChosenTodoCategory, VTS_I4)
	ON_EVENT(CPhaseStepDlg, IDC_ASSIGN_USERS, 16, OnSelChosenAssignUsers, VTS_I4)
	//}}AFX_EVENTSINK_MAP	
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPhaseStepDlg message handlers

BOOL CPhaseStepDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_okBtn.AutoSet(NXB_OK);
	m_cancelBtn.AutoSet(NXB_CANCEL);
	
	GetDlgItem(IDC_ITEM)->ModifyStyleEx(0, WS_EX_TRANSPARENT);
	
	// (a.walling 2009-02-04 10:14) - PLID 31956 - No longer used
	//SetRgnBg();
			
	m_action = BindNxDataListCtrl (IDC_ACTION, false);
	m_item = BindNxDataListCtrl (IDC_ITEM, false);

	m_pUserList = BindNxDataListCtrl(IDC_ASSIGN_USERS);

	//TES 7/16/2010 - PLID 39705 - Fill the merge scope options.
	m_pMergeScope = BindNxDataList2Ctrl(IDC_MERGE_SCOPE, false);
	NXDATALIST2Lib::IRowSettingsPtr pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsPatient);
	pScopeRow->PutValue(1, _bstr_t("Patient"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);
	pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsPic);
	pScopeRow->PutValue(1, _bstr_t("PIC"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);
	pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsProcedure);
	pScopeRow->PutValue(1, _bstr_t("Procedure"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);
	pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsMasterProcedure);
	pScopeRow->PutValue(1, _bstr_t("Master Procedure"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);
	pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsDetailProcedure);
	pScopeRow->PutValue(1, _bstr_t("Detail Procedure"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);
	pScopeRow = m_pMergeScope->GetNewRow();
	pScopeRow->PutValue(0, (long)mtsPrescription);
	pScopeRow->PutValue(1, _bstr_t("Prescription"));
	m_pMergeScope->AddRowAtEnd(pScopeRow, NULL);


	m_pTodoCategoryList = BindNxDataListCtrl(IDC_TODO_CATEGORY);
	m_pTodoCategoryList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	IRowSettingsPtr pRow = m_pTodoCategoryList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("{No Category}"));
	m_pTodoCategoryList->InsertRow(pRow, 0);
	m_pTodoCategoryList->CurSel = 0;

	//Find out if we're on a brand-new step or not.
	if(m_nStepTemplateID == -1) {
		m_new = true;
	}
	else {
		// (j.armen 2012-01-26 09:20) - PLID 47711 - Refactored
		m_new = !ReturnsRecordsParam("SELECT ID FROM StepTemplatesT WHERE ID = {INT}", m_nStepTemplateID);
	}

	m_cbPriority.SetCurSel(1);
	GetDlgItem(IDC_TODO_PRIORITY)->EnableWindow(FALSE);
	GetDlgItem(IDC_TODO_CATEGORY)->EnableWindow(FALSE);

	((CNxEdit*)GetDlgItem(IDC_NAME))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_NOTE))->SetLimitText(2000);

	LoadActionList();
	LoadItemList();
	LoadData();
	LoadActivateData();
	ReflectCurrentAction();
	
	return TRUE;
}

void CPhaseStepDlg::OnSelChangedAction(long nNewSel) 
{
	/*TES 7/14/03: This is now in OnSelChosenAction(), where it should have been long ago.
	LoadItemList();	
	m_dwaActionIds.RemoveAll();
	RefreshItemDisplay();
	*/
}

void CPhaseStepDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CPhaseStepDlg::OnOK() 
{	
	if (Save())
		CDialog::OnOK();
}

void CPhaseStepDlg::OnKillfocusStep() 
{
}

/////////////////////////////////////////////////////////////////////////////
void CPhaseStepDlg::LoadAction(long id, _bstr_t action)
{
	IRowSettingsPtr pRow = m_action->GetRow(-1);

	pRow->Value[0] = id;
	pRow->Value[1] = action;
	m_action->AddRow(pRow);

}

void CPhaseStepDlg::LoadActionList()
{
	LoadAction(PA_Manual,				_bstr_t(GetPhaseActionDescription(PA_Manual)));
//	LoadAction(PA_QuoteSurgery,			_bstr_t(GetPhaseActionDescription(PA_QuoteSurgery))); probably can never work
	LoadAction(PA_Quote,				_bstr_t(GetPhaseActionDescription(PA_Quote)));
	LoadAction(PA_QuoteCPT,				_bstr_t(GetPhaseActionDescription(PA_QuoteCPT)));
	LoadAction(PA_QuoteInventory,		_bstr_t(GetPhaseActionDescription(PA_QuoteInventory)));
	LoadAction(PA_Bill,					_bstr_t(GetPhaseActionDescription(PA_Bill)));
//	LoadAction(PA_BillSurgery,			_bstr_t(GetPhaseActionDescription(PA_BillSurgery))); probably can never work
	LoadAction(PA_BillCPT,				_bstr_t(GetPhaseActionDescription(PA_BillCPT)));
	LoadAction(PA_BillInventory,		_bstr_t(GetPhaseActionDescription(PA_BillInventory)));
//	LoadAction(PA_BillQuote,			_bstr_t(GetPhaseActionDescription(PA_BillQuote)));
	LoadAction(PA_WriteTemplate,		_bstr_t(GetPhaseActionDescription(PA_WriteTemplate)));
	LoadAction(PA_WritePacket,			_bstr_t(GetPhaseActionDescription(PA_WritePacket)));

	LoadAction(PA_ScheduleAptCategory,	_bstr_t(GetPhaseActionDescription(PA_ScheduleAptCategory)));
	LoadAction(PA_ScheduleAptType,		_bstr_t(GetPhaseActionDescription(PA_ScheduleAptType)));

	LoadAction(PA_ActualAptCategory,	_bstr_t(GetPhaseActionDescription(PA_ActualAptCategory)));
	LoadAction(PA_ActualAptType,		_bstr_t(GetPhaseActionDescription(PA_ActualAptType)));
	LoadAction(PA_Ladder,				_bstr_t(GetPhaseActionDescription(PA_Ladder)));
	LoadAction(PA_Payment,				_bstr_t(GetPhaseActionDescription(PA_Payment)));
	// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
	// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
	LoadAction(PA_CreateEMRByCollection,		_bstr_t(GetPhaseActionDescription(PA_CreateEMRByCollection)));
	LoadAction(PA_CreateEMRByTemplate,			_bstr_t(GetPhaseActionDescription(PA_CreateEMRByTemplate)));

	m_action->CurSel = 0;
}

void CPhaseStepDlg::LoadItemList()
{
	if (m_action->CurSel == -1)
		return;

	//TES 10/30/2003: Note that I'm using ShowDlgItem rather than GetDlgItem()->ShowWindow.
	//This is a workaround for a bug in ActiveX (really!) that messes up focus.
	switch (VarLong(m_action->Value[m_action->CurSel][0]))
	{	case PA_QuoteSurgery:
		case PA_BillSurgery:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "SurgeriesT";
			m_item->WhereClause = "";
			m_item->Requery();
			break;
		case PA_QuoteCPT:
		case PA_BillCPT:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "(SELECT ServiceT.ID, Code + ' ' + SubCode + ' ' + Name AS Name "
				"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID) AS SubQ";
			m_item->WhereClause = "";
			m_item->Requery();
			break;
		case PA_QuoteInventory:
		case PA_BillInventory:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "(SELECT ServiceT.ID, Name "
				"FROM ServiceT INNER JOIN ProductT On ServiceT.ID = ProductT.ID) AS SubQ";
			m_item->WhereClause = "";
			m_item->Requery();
			break;
		case PA_ActualAptCategory:
		case PA_ScheduleAptCategory:
		{	ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			IRowSettingsPtr pRow;
			m_item->Clear();

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 1L;
			pRow->Value[1] = "Consult";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 2L;
			pRow->Value[1] = "PreOp";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 3L;
			pRow->Value[1] = "Minor Procedure";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 4L;
			pRow->Value[1] = "Surgery";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 5L;
			pRow->Value[1] = "Follow-Up";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 6L;
			pRow->Value[1] = "Other Procedure";
			m_item->AddRow(pRow);

			break;
		}
		case PA_ActualAptType:
		case PA_ScheduleAptType:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "AptTypeT";
			m_item->WhereClause = "Inactive = 0";
			m_item->Requery();
			break;
		case PA_Bill:
		case PA_Quote:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "ProcedureT";
			// (c.haag 2009-01-09 10:39) - PLID 32571 - Don't let users assign inactive procedures to steps. Active steps
			// should not be assigned to any; so don't try to fill in the combo with any existing data (if there is existing,
			// it's bad data)
			m_item->WhereClause = "Inactive = 0";
			m_item->Requery();
			break;
		case PA_WriteTemplate:
			{
				ShowDlgItem(IDC_ITEM, SW_HIDE);
				GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_SHOW);
				// (j.armen 2012-01-26 09:07) - PLID 47711 - Parameratized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ID, Path "
					"FROM MergeTemplatesT "
					"WHERE ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT})", 
					m_nStepTemplateID);
				m_strItemList = "";
				if(prs->eof) {
					m_strItemList = "{No template specified}, ";
				}
				else {
					while(!prs->eof) {
						m_strItemList += "{" + AdoFldString(prs, "Path", "No template specified") + "}, ";
						prs->MoveNext();
					}
				}
				m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);//Trim last ", "
				m_nTemplateID = prs->eof ? -1 : AdoFldLong(prs, "ID", -1);
				m_strItemList.MakeLower();
				CRect rTextBox;
				GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
				ScreenToClient(&rTextBox);
				// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
				DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
			}
			break;
		case PA_WritePacket:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "PacketsT";
			m_item->WhereClause = "Deleted = 0";
			m_item->Requery();
			break;

		case PA_Ladder:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "ProcedureT";
			// (d.moore 2007-06-08 11:25) - PLID 14670 - Modified query to use ProcedureLadderTemplateT
			//  This allows procedures to be associated with multiple ladders.
			// (c.haag 2009-01-09 10:39) - PLID 32571 - Don't let users assign inactive procedures to steps. Active steps
			// should not be assigned to any; so don't try to fill in the combo with any existing data (if there is existing,
			// it's bad data)
			m_item->WhereClause = "Inactive = 0 AND ID IN (SELECT DISTINCT ProcedureID FROM ProcedureLadderTemplateT)";
			m_item->Requery();
			break;

		case PA_Payment:
		{	ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			IRowSettingsPtr pRow;
			m_item->Clear();

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 1L;
			pRow->Value[1] = "Any payment received for this procedure";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 2L;
			pRow->Value[1] = "Total PrePayment amounts equal to the active quote";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 3L;
			pRow->Value[1] = "Payment amounts equal to the balance of the active bill";
			m_item->AddRow(pRow);

			pRow = m_item->GetRow(-1);
			pRow->Value[0] = 4L;
			pRow->Value[1] = "Total PrePayments meet deposit requirements for the active quote";
			m_item->AddRow(pRow);

			m_item->CurSel = 0;
			//For this type only, we don't want None or Multiple.
			return;
			break;
		}
		// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
		// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
		case PhaseTracking::PA_CreateEMRByCollection:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "EMRCollectionT";
			m_item->WhereClause = "Inactive = 0";
			m_item->Requery();
			break;
		case PhaseTracking::PA_CreateEMRByTemplate:
			ShowDlgItem(IDC_ITEM, SW_SHOW);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->FromClause = "EMRTemplateT";
			m_item->WhereClause = "Deleted = 0 AND (CollectionID Is Null OR CollectionID Not In (SELECT ID FROM EMRCollectionT WHERE Inactive = 1))";
			m_item->Requery();
			break;
		case PA_Manual:
		default:
			ShowDlgItem(IDC_ITEM, SW_HIDE);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->Clear();
			break;
		
	}

	//Regardless, let's add a "{None}" row and  a {Multiple} row.
	m_item->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	IRowSettingsPtr pRow = m_item->GetRow(-1);
	pRow->Value[0] = 0L;
	pRow->Value[1] = "{None}";
	m_item->InsertRow(pRow, 0);

	pRow = m_item->GetRow(-1);
	pRow->Value[0] = 0L;
	pRow->Value[1] = "{Multiple}";
	m_item->InsertRow(pRow, 1);
	
}

void CPhaseStepDlg::LoadData()
{
	try
	{
		if(!m_new) {
			//TES 7/16/2010 - PLID 39705 - Added DefaultScope
			// (j.armen 2012-01-26 09:23) - PLID 47711 - Parameratized
			_RecordsetPtr prsTemplate = CreateParamRecordset(
				"SELECT "
				"	StepName, Note, Action, StepOrder, Skippable, "
				"	Todo, ToDoPriority, ToDoCategory, DefaultScope "
				"FROM StepTemplatesT "
				"WHERE ID = {INT};\r\n"

				"SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT};", 
				m_nStepTemplateID, m_nStepTemplateID);

			// (j.armen 2012-01-26 09:30) - PLID 47711 - Parameratized and batched above
			_RecordsetPtr prsActionIDs = prsTemplate->NextRecordset(NULL);

			SetDlgItemText(IDC_NAME, AdoFldString(prsTemplate, "StepName", ""));
			SetDlgItemText(IDC_NOTE, AdoFldString(prsTemplate, "Note"));
			m_action->SetSelByColumn(0, AdoFldLong(prsTemplate, "Action"));
			m_nOrigAction = AdoFldLong(prsTemplate, "Action");
			LoadItemList();
			m_aryActionIDs.RemoveAll();
			
			long nAction = VarLong(m_action->GetValue(m_action->CurSel, 0));
			if(nAction == PA_WriteTemplate) {
				//Always show in text form.
				m_strItemList = "";
				// (j.armen 2012-01-26 10:04) - PLID 47711 - Parameratized
				_RecordsetPtr rsTemplate = CreateParamRecordset(
					"SELECT ID, Path "
					"FROM MergeTemplatesT "
					"WHERE ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT})", 
					m_nStepTemplateID);
				while(!rsTemplate->eof) {
					m_aryActionIDs.Add(AdoFldLong(rsTemplate, "Id"));
					m_strItemList += "{" + AdoFldString(rsTemplate, "Path") + "}, ";
					rsTemplate->MoveNext();
				}
				if(m_strItemList == "") {
					m_strItemList = "{No template specified}";
				}
				else {
					m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);
				}
				//Modify display
				ShowDlgItem(IDC_ITEM, SW_HIDE);
				GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_SHOW);
				CRect rTextBox;
				GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
				ScreenToClient(&rTextBox);
				// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
				DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);

				//TES 7/16/2010 - PLID 39705 - Show the Merge Scope fields
				m_nxstaticMergeScopeLabel.ShowWindow(SW_SHOW);
				m_nxbMergeScopeHelp.ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MERGE_SCOPE)->ShowWindow(SW_SHOW);

				//TES 7/16/2010 - PLID 39705 - Set the scope list to the appropriate value.
				m_pMergeScope->SetSelByColumn(0, AdoFldLong(prsTemplate, "DefaultScope", GetRemotePropertyInt("Tracking_DefaultMergeScope", (long)mtsPic, 0, "<None>")));
			}
			else {
				if(!prsActionIDs->eof) {
					m_strItemList = "";
					long nActionID = AdoFldLong(prsActionIDs, "ActionID");
					long nRow = m_item->FindByColumn(0, nActionID, 0, TRUE);
					if(nRow == -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
						//Looks like this step uses an inactive type.
						// (j.armen 2012-01-26 10:07) - PLID 47711 - Parameratized
						_RecordsetPtr rsType = CreateParamRecordset(
							"SELECT ID, Name FROM AptTypeT WHERE ID = {INT}", nActionID);
						CString strName = AdoFldString(rsType, "Name");
						m_item->PutComboBoxText(_bstr_t(strName));
						m_strItemList = strName + ", ";
						m_aryActionIDs.Add(AdoFldLong(rsType, "ID"));
					}
					else {
						if (nRow != -1)
						{
							m_strItemList = VarString(m_item->GetValue(nRow, 1)) + ", ";
							m_aryActionIDs.Add(AdoFldLong(prsActionIDs, "ActionID"));
						}
						else {
							//They've got an invalid criteria (maybe it was deleted).  Get rid of it immediately, and reload
							//the screen.
							// (j.armen 2012-01-26 10:27) - PLID 47711 - Parameratized
							ExecuteParamSql(
								"DELETE FROM StepCriteriaT WHERE StepTemplateID = {INT} AND ActionID = {INT}", 
								m_nStepTemplateID, nActionID);
							LoadData();
							return;
						}
					}
					prsActionIDs->MoveNext();
					if(!prsActionIDs->eof) {
						//There's multiple criteria here.
						bool bNeedReload = false;
						while(!prsActionIDs->eof) {
							nRow = m_item->FindByColumn(0, prsActionIDs->Fields->GetItem("ActionID")->Value, 0, TRUE);
							if(nRow == -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
								//Looks like this step uses an inactive type.
								// (j.armen 2012-01-26 10:10) - PLID 47711 - Parameratized
								_RecordsetPtr rsType = CreateParamRecordset(
									"SELECT ID, Name FROM AptTypeT WHERE ID = {INT}", AdoFldLong(prsActionIDs, "ActionID"));
								m_strItemList += AdoFldString(rsType, "Name") + ", ";
								m_aryActionIDs.Add(AdoFldLong(rsType, "ID"));
							}
							else {
								if (nRow != -1) {
									m_aryActionIDs.Add(AdoFldLong(prsActionIDs, "ActionID"));
									m_strItemList += VarString(m_item->GetValue(nRow, 1)) + ", ";
								}
								else {
									//They've got an invalid criteria (maybe it was deleted).  Get rid of it immediately, and reload
									//the screen.
									// (j.armen 2012-01-26 10:28) - PLID 47711 - Parameratized
									ExecuteParamSql(
										"DELETE FROM StepCriteriaT WHERE StepTemplateID = {INT} AND ActionID = {INT}", 
										m_nStepTemplateID, nActionID);
									bNeedReload = true;
									return;
								}

							}
							prsActionIDs->MoveNext();
						}
						if(bNeedReload) {
							LoadData();
							return;
						}

						m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);
						//Modify display
						ShowDlgItem(IDC_ITEM, SW_HIDE);
						GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_SHOW);
						CRect rTextBox;
						GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
						ScreenToClient(&rTextBox);
						// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
						DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
					}
					else {
						//This is the only one.  Make sure the display is correct.
						ShowDlgItem(IDC_ITEM, SW_SHOW);
						GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
					}
				}
				else {
					//Make sure the display is correct.
					if(VarLong(m_action->GetValue(m_action->CurSel, 0)) == PA_Manual) {
						ShowDlgItem(IDC_ITEM, SW_HIDE);
					}
					else {
						ShowDlgItem(IDC_ITEM, SW_SHOW);
					}
					GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
					m_item->CurSel = -1;
				}

				//TES 7/16/2010 - PLID 39705 - Hide the Merge Scope fields
				m_nxstaticMergeScopeLabel.ShowWindow(SW_HIDE);
				m_nxbMergeScopeHelp.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MERGE_SCOPE)->ShowWindow(SW_HIDE);
			}

			// (j.armen 2012-01-23 08:59) - PLID 47707 - No need to iterate through, just append
			m_aryOrigActionIDs.Append(m_aryActionIDs);

			SetDlgItemInt(IDC_STEP, AdoFldLong(prsTemplate, "StepOrder"));
			CheckDlgButton(IDC_ALLOW_SKIP, AdoFldBool(prsTemplate, "Skippable") ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_STEP_CREATE_TODO, AdoFldBool(prsTemplate, "Todo") ? BST_CHECKED : BST_UNCHECKED);
			OnStepCreateTodo();

			long nPriority = AdoFldLong(prsTemplate, "ToDoPriority", 0);
			if(nPriority > 0){
				// in data, it's stored as 1 = high, 2 = medium, 3 = low
				m_cbPriority.SetCurSel(nPriority - 1);
			}
			else {
				// default to low
				m_cbPriority.SetCurSel(2);
			}
			m_pTodoCategoryList->SetSelByColumn(0, AdoFldLong(prsTemplate, "ToDoCategory", -1));
		}
		else {
			// (j.armen 2012-01-26 09:05) - PLID 47711 - Parameratized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT Max(StepOrder) + 1 AS NewStep "
				"FROM StepTemplatesT "
				"WHERE LadderTemplateID = {INT} AND Inactive = 0", 
				m_nLadderTemplateID);
			SetDlgItemInt(IDC_STEP, prs->eof ? 1 : AdoFldLong(prs, "NewStep", 1));

			if(GetRemotePropertyInt("LadderTodos", 0, 0, "<None>", true)) {
				CheckDlgButton(IDC_STEP_CREATE_TODO, BST_CHECKED);
				OnStepCreateTodo();
				m_cbPriority.SetCurSel(GetRemotePropertyInt("DefaultLadderTodoPriority", 2, 0, "<None>", true)-1);
				m_pTodoCategoryList->SetSelByColumn(0, (long)-1);
			}

			//TES 7/16/2010 - PLID 39705 - Set the scope list to the default value (it's not visible yet, but it might be later).
			m_pMergeScope->SetSelByColumn(0, GetRemotePropertyInt("Tracking_DefaultMergeScope", (long)mtsPic, 0, "<None>"));
		}
	}
	NxCatchAll("Could not load phase step");
}

// (j.armen 2012-01-23 10:46) - PLID 47707 - Renamed local variables to match naming conventions
bool CPhaseStepDlg::Save()
{
	long nStep, nAction, nPriority;
	CString	strName, strNote;

	nStep = GetDlgItemInt(IDC_STEP);
	GetDlgItemText(IDC_NAME, strName);
	CString strTempName = strName;
	strTempName.TrimLeft();
	strTempName.TrimRight();
	if (strTempName.IsEmpty()) {
		//thats no good
		MsgBox("Please enter a name for this step");
		return FALSE;
	}
	GetDlgItemText(IDC_NOTE, strNote);
	//DRT 9/15/2004 - PLID 14127 - Up size to 2k instead of 255.
	if(strNote.GetLength() > 2000) {
		AfxMessageBox("This note is longer than the supported length (2000). Please revise it.");
		return false;
	}
	// (j.armen 2012-01-26 11:10) - PLID 47707 - Corrected spelling.
	if(strName.GetLength() > 50) {
		MsgBox("This name is longer than the supported length (50). Please revise it.");
		return false;
	}

	nAction = VarLong(m_action->Value[m_action->CurSel][0]);

	// (j.jones 2008-11-17 17:22) - PLID 30926 - added m_checkOpenPICWhenSelected
	BOOL bOpenPIC = m_checkOpenPICWhenSelected.GetCheck();

	//Save the Activation date stuff.
	//0 = Immediate
	//1 = Day Interval
	//2 = Week Interval
	//3 = Month Interval
	//4 = Prompt
	//5 = Appointment Date
	int nActivateType = 0;
	int nActivateData = 0;
	CString strActivatePrompt = "";
	CString strInterval, strTemp;

	switch(GetCheckedRadioButton(IDC_IMMEDIATE, IDC_NEVER)) {
	case IDC_IMMEDIATE:
		nActivateType = 0;
		break;
	case IDC_TIME_AFTER:
		GetDlgItemText(IDC_INTERVAL_TYPE, strInterval);
		if(strInterval == "Days") {
			nActivateType = 1;
		}
		else if(strInterval == "Weeks") {
			nActivateType = 2;
		}
		else if(strInterval == "Months") {
			nActivateType = 3;
		}
		GetDlgItemText(IDC_INTERVAL_NUMBER, strTemp);
		nActivateData = atol(strTemp);
		break;
	case IDC_ASK:
		nActivateType = 4;
		strActivatePrompt = m_strPrompt;
		break;
	case IDC_NEVER:
		nActivateType = 5;
		break;
	}

	BOOL bSkippable = IsDlgButtonChecked(IDC_ALLOW_SKIP);
	BOOL bTodo = IsDlgButtonChecked(IDC_STEP_CREATE_TODO);
	// we want to save whatever the current selection is plus 1 because we want
	// 1 = high, 2 = medium, 3 = low so that it matches the todo alarm priorities
	nPriority = m_cbPriority.GetCurSel() + 1;

	// (j.armen 2012-01-26 11:01) - PLID 47711 - Store value in a variant
	_variant_t vtTodoCategory = m_pTodoCategoryList->GetValue(m_pTodoCategoryList->CurSel,0);
	if(VarLong(vtTodoCategory, -1) == -1)
		vtTodoCategory.vt = VT_NULL;

	//TES 7/16/2010 - PLID 39705 - Get the value of the MergeScope dropdown, if it's visible (which is only true if we're on 
	// a WriteTemplate step).
	// (j.armen 2012-01-26 11:02) - PLID 47711 - Store value in a variant
	_variant_t vtMergeScope;
	if(nAction == PA_WriteTemplate) {
		NXDATALIST2Lib::IRowSettingsPtr pScopeRow = m_pMergeScope->CurSel;
		if(pScopeRow) {
			vtMergeScope = pScopeRow->GetValue(0);
		}
		else {
			AfxMessageBox("Please select a default option for merging templates.");
			return false;
		}
	}
	else {
		vtMergeScope.vt = VT_NULL;
	}

	try {
		//Brand new step?
		if (m_new) {

			// (j.jones 2008-11-26 13:32) - PLID 30830 - converted into a batch save 
			// (j.armen 2012-01-26 10:43) - PLID 47711 - Parameratized
			CParamSqlBatch sqlBatch;
			sqlBatch.Declare("DECLARE @nID INT");
			sqlBatch.Add("UPDATE StepTemplatesT SET StepOrder = StepOrder + 1 "
				"WHERE LadderTemplateID = {INT} AND StepOrder >= {INT} AND Inactive = 0", 
				m_nLadderTemplateID, nStep);

			//insert the item
			sqlBatch.Declare("SET @nID = Coalesce((SELECT Max(ID) FROM StepTemplatesT), 0) + 1");
			// (j.jones 2008-11-17 17:22) - PLID 30926 - added OpenPIC
			//TES 7/16/2010 - PLID 39705 - Added DefaultScope
			sqlBatch.Add("INSERT INTO StepTemplatesT ("
				"ID, LadderTemplateID, StepOrder, StepName, Action, "
				"Note, ActivateType, ActivateInterval, ActivateStringData, Skippable, "
				"Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope)"
				"SELECT @nID, {INT}, {INT}, {STRING}, {INT}, "
				"{STRING}, {INT}, {INT}, {STRING}, {BOOL}, "
				"{BOOL}, {INT}, {VT_I4}, {BOOL}, {VT_I4}",
				m_nLadderTemplateID, nStep, strName, nAction, 
				strNote, nActivateType, nActivateData, strActivatePrompt, bSkippable, 
				bTodo, nPriority, vtTodoCategory, bOpenPIC, vtMergeScope);
			
			for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
				sqlBatch.Add("INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
					"VALUES (@nID, {INT})", m_aryActionIDs[i]);
			}

			// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
			for(int i = 0; i < m_aryAssignToIDs.GetSize(); i++) {
				sqlBatch.Add("INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
					"VALUES (@nID, {INT})", m_aryAssignToIDs[i]);
			}

			sqlBatch.Execute(GetRemoteData());
		}
		else {
			bool bSaveAsNew = false;
			if(m_bIsTracked) {
				//Have we made a major change
				if(nAction != m_nOrigAction || m_bActionIdsChanged || HasUserIDListChanged()) {
					//That's a major change, we definitely can't apply it to existing steps.
					int nReturn = MsgBox(MB_OKCANCEL, "The changes you have made will not be applied to ladders which have already been entered.  They will take effect only on ladders which are entered from now on.");
					if(nReturn == IDOK ) bSaveAsNew = true;
					else return FALSE;
				}
				else {
					//These are relatively minor changes, we can try to apply them to existing steps.
					int nReturn = MsgBox(MB_YESNOCANCEL, "This step is being tracked for some users.  Would you like to keep existing steps unchanged?\n"
						"If you select Yes, these changes will only affect ladders which are entered from now on.\n"
						"If you select No, these changes will be applied to all existing instances of this step, with possibly unpredictable results.");
					if(nReturn == IDNO) bSaveAsNew = false;
					else if(nReturn == IDYES) bSaveAsNew = true;
					else return FALSE;
				}
			}
			if(bSaveAsNew) {

				// (j.jones 2008-11-26 13:32) - PLID 30830 - converted into a batch save 
				// (j.armen 2012-01-26 10:44) - PLID 47711 - Parameratized
				CParamSqlBatch sqlBatch;

				//Mark the existing one inactive.
				sqlBatch.Add("UPDATE StepTemplatesT SET Inactive = 1 WHERE ID = {INT}", m_nStepTemplateID);
				
				//Add the new one.
				sqlBatch.Declare("DECLARE @nID INT");
				sqlBatch.Declare("SET @nID = Coalesce((SELECT Max(ID) FROM StepTemplatesT), 0) + 1");

				// (j.jones 2008-11-17 17:22) - PLID 30926 - added OpenPIC
				//TES 7/16/2010 - PLID 39705 - Added DefaultScope
				sqlBatch.Add("INSERT INTO StepTemplatesT ("
					"ID, LadderTemplateID, StepOrder, StepName, Action, "
					"Note, ActivateType, ActivateInterval, ActivateStringData, Skippable, "
					"Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope) "
					"SELECT @nID, {INT}, {INT}, {STRING}, {INT}, "
					"{STRING}, {INT}, {INT}, {STRING}, {BOOL}, "
					"{BOOL}, {INT}, {VT_I4}, {BOOL}, {VT_I4}",
					m_nLadderTemplateID, nStep, strName, nAction, 
					strNote, nActivateType, nActivateData, strActivatePrompt, bSkippable, 
					bTodo, nPriority, vtTodoCategory, bOpenPIC, vtMergeScope);
				
				for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
					sqlBatch.Add("INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
						"VALUES (@nID, {INT})", m_aryActionIDs[i]);
				}

				// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
				for(int i = 0; i < m_aryAssignToIDs.GetSize(); i++) {
					sqlBatch.Add("INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
						"VALUES (@nID, {INT})", m_aryAssignToIDs[i]);
				}

				// (j.gruber 2007-08-24 08:55) - PLID 27091 - check tracking conversions
				sqlBatch.Add("UPDATE TrackingConversionT SET BeginStepTemplateID = @nID WHERE BeginStepTemplateID = {INT}", m_nStepTemplateID);
				sqlBatch.Add("UPDATE TrackingConversionT SET EndStepTemplateID = @nID WHERE EndStepTemplateID = {INT}", m_nStepTemplateID);

				sqlBatch.Execute(GetRemoteData());

			}
			else {

				// (j.jones 2008-11-26 13:32) - PLID 30830 - converted into a batch save 
				CParamSqlBatch sqlBatch;

				// (j.jones 2008-11-17 17:22) - PLID 30926 - added OpenPIC
				//TES 7/16/2010 - PLID 39705 - Added DefaultScope
				sqlBatch.Add("UPDATE StepTemplatesT SET "
					"StepName = {STRING}, Action = {INT}, Note = {STRING}, ActivateType = {INT}, "
					"ActivateInterval = {INT}, ActivateStringData = {STRING}, Skippable = {BOOL}, Todo = {BOOL}, "
					"ToDoPriority = {INT}, TodoCategory = {VT_I4}, OpenPIC = {BOOL}, DefaultScope = {VT_I4} "
					"WHERE ID = {INT}",
					strName, nAction, strNote, nActivateType, 
					nActivateData, strActivatePrompt, bSkippable, bTodo, 
					nPriority, vtTodoCategory, bOpenPIC, vtMergeScope, 
					m_nStepTemplateID);

				sqlBatch.Add("DELETE FROM StepCriteriaT WHERE StepTemplateID = {INT}", m_nStepTemplateID);
				
				for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
					sqlBatch.Add("INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
						"VALUES ({INT}, {INT})", m_nStepTemplateID, m_aryActionIDs[i]);
				}

				// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
				sqlBatch.Add("DELETE FROM StepTemplatesAssignToT WHERE StepTemplateID = {INT}", m_nStepTemplateID);
				for(int i = 0; i < m_aryAssignToIDs.GetSize(); i++) {
					sqlBatch.Add("INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
						"VALUES ({INT}, {INT})", m_nStepTemplateID, m_aryAssignToIDs[i]);
				}

				sqlBatch.Execute(GetRemoteData());

			}
		}
	}NxCatchAll("Error in CPhaseStepDlg::Save");

	return true;
}


/*void CPhaseStepDlg::OnBrowsetemplates() 
{
	CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Microsoft Word Templates|*.dot||");

	// set up open document dialog
	CString strTemplatePath = GetTemplatePath();
	dlg.m_ofn.lpstrInitialDir = strTemplatePath;
	dlg.m_ofn.lpstrTitle = "Select a merge template";

	if (dlg.DoModal() == IDOK) {	
		
		CString strPath = dlg.GetPathName();
		if(strPath.Left(GetSharedPath().GetLength()).CompareNoCase(GetSharedPath()) == 0)
		{
			//Make sure it starts with exactly one \.
			strPath = "" ^ strPath.Mid(GetSharedPath().GetLength());
		}
		
		//Either way, let's make it all upper case
		strPath.MakeUpper();
		//OK, strPath is now a valid record.
		_RecordsetPtr rsTemplateID = CreateRecordset("SELECT ID FROM MergeTemplatesT WHERE Path = '%s'", _Q(strPath));
		if(rsTemplateID->eof) {
			//OK, we need to store it.
			m_nTemplateID = NewNumber("MergeTemplatesT", "ID");
			ExecuteSql("INSERT INTO MergeTemplatesT (ID, Path) VALUES (%li, '%s')", m_nTemplateID, _Q(strPath));
		}
		else {
			//It already has been stored.
			m_nTemplateID = AdoFldLong(rsTemplateID, "ID");
		}
		strPath = "{" + strPath + "}";
		strPath.MakeLower();
		SetDlgItemText(IDC_CRITERIA_LIST, strPath);
	}
	//No, we DON'T want to change m_nTemplateID if they cancel!!!11
	//else m_nTemplateID = -1;

	InvalidateDlgItem(IDC_CRITERIA_LIST);
}*/


void CPhaseStepDlg::LoadActivateData() 
{
	try {				
		if(m_new) {
			CheckRadioButton(IDC_IMMEDIATE, IDC_NEVER, IDC_IMMEDIATE);
			//Disable the non-relevant controls.
			GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);

			//Initialize things appropriately (just in case)
			SetDlgItemText(IDC_INTERVAL_NUMBER, "1");
			m_cbIntervalType.SelectString(0, "Days");

			// (j.jones 2008-11-17 17:37) - PLID 30926 - added m_checkOpenPICWhenSelected
			m_checkOpenPICWhenSelected.SetCheck(FALSE);

			m_strPrompt = "When should the next Tracking step be activated for this patient?";
		}
		else {
			//Load all this stuff
			// (j.jones 2008-11-17 17:38) - PLID 30926 - added OpenPIC
			_RecordsetPtr rsActivate = CreateParamRecordset("SELECT ActivateType, ActivateInterval, ActivateStringData, OpenPIC FROM StepTemplatesT WHERE ID = {INT}", m_nStepTemplateID);
			ASSERT(!rsActivate->eof); //If this was the case, m_new would be true.

			//OK, first of all, we want to set m_strPrompt to ActivateStringData if there's anything there, because why the heck not?  I mean, we've got it saved, so let's use it.
			m_strPrompt = AdoFldString(rsActivate, "ActivateStringData", "");
			if(m_strPrompt == "") {
				m_strPrompt = "When should the next Tracking step be activated for this patient?";
			}

			// (j.jones 2008-11-17 17:37) - PLID 30926 - added m_checkOpenPICWhenSelected
			m_checkOpenPICWhenSelected.SetCheck(AdoFldBool(rsActivate, "OpenPIC", FALSE));

			//Also, let's initialize the number box to "1", because we could pull the old number, but not what it represented, so it doesn't matter.  It'll be changed down there if necessary.
			SetDlgItemText(IDC_INTERVAL_NUMBER, "1");
			//And we'll initialize the drop-down to days, same reason.
			m_cbIntervalType.SelectString(0, "Days");

			long nActivateType, nActivateData;
			CString strTemp;
			nActivateType = AdoFldLong(rsActivate, "ActivateType");
			switch(nActivateType) {
			case 0:
			case 5:
				CheckRadioButton(IDC_IMMEDIATE, IDC_NEVER, nActivateType == 0 ? IDC_IMMEDIATE : IDC_NEVER);
				//Disable the non-relevant controls.
				GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
				GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
				GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);
				break;

			case 1:
			case 2:
			case 3:
				if(nActivateType == 1) {
					m_cbIntervalType.SelectString(0, "Days");
				}
				else if(nActivateType == 2) {
					m_cbIntervalType.SelectString(0, "Weeks");
				}
				else {
					m_cbIntervalType.SelectString(0, "Months");
				}

				CheckRadioButton(IDC_IMMEDIATE, IDC_NEVER, IDC_TIME_AFTER);
				
				GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(TRUE);
				nActivateData = AdoFldLong(rsActivate, "ActivateInterval");
				strTemp.Format("%li", nActivateData);
				SetDlgItemText(IDC_INTERVAL_NUMBER, strTemp);

				GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(TRUE);

				GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);
				
				break;
			case 4:
				//Don't forget, m_strPrompt is already set
				CheckRadioButton(IDC_IMMEDIATE, IDC_NEVER, IDC_ASK);

				GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
				GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
				GetDlgItem(IDC_ASK)->EnableWindow(TRUE);
				break;
			}
		}
		//That ought to do it.
	}NxCatchAll("Error in CPhaseStepDlg::LoadActivateData");
}


//TODO: Find a good way to modularize all this crap.
void CPhaseStepDlg::OnImmediate() 
{
	GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);
	
}

void CPhaseStepDlg::OnTimeAfter() 
{
	GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(TRUE);
	GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(TRUE);
	GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);	
}

void CPhaseStepDlg::OnAsk() 
{
	GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROMPT)->EnableWindow(TRUE);
}

void CPhaseStepDlg::OnNever() 
{
	GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);
}

void CPhaseStepDlg::OnPrompt() 
{
	CPromptDlg dlg(this);
	dlg.m_strPrompt = m_strPrompt;
	int nReturn = dlg.DoModal();
	if(nReturn == IDOK) {
		m_strPrompt = dlg.m_strPrompt;
	}	
}


void CPhaseStepDlg::OnSelChosenItem(long nRow) 
{
	if(nRow == -1) {
		if(VarLong(m_action->Value[m_action->CurSel][0]) == PA_Payment) m_item->CurSel = 0;
		return;
	}

	if(VarLong(m_action->Value[m_action->CurSel][0]) == PA_Payment &&
		VarLong(m_item->GetValue(nRow, 0)) == 4) {
		//they are choosing to make a step for the deposit amount
		if(GetRemotePropertyInt("QuoteDepositTypeRequired",0,0,"<None>",true) == 0) {
			//no deposit is required!
			AfxMessageBox("No deposit is currently required on quotes. You may configure the deposit requirements\n"
						  "in the Quote Setup screen in the Surgery Tab of the Administrator module.\n\n"
						  "The 'deposit' step will have no effect until these requirements are configured.");
		}
	}

	if(VarLong(m_item->GetValue(nRow, 0)) == 0) {
		if(VarString(m_item->GetValue(nRow, 1)) == "{Multiple}") {
			EditCriteriaList();
			RefreshItemDisplay();			
		}
		else {
			//{None}
			m_item->CurSel = -1;
			m_aryActionIDs.RemoveAll();
			m_bActionIdsChanged = true;
		}
	}
	else {
		m_aryActionIDs.RemoveAll();
		m_aryActionIDs.Add(VarLong(m_item->GetValue(nRow, 0)));
		m_bActionIdsChanged = true;
	}
}

void CPhaseStepDlg::EnableWindow(BOOL bEnable)
{
	GetDlgItem(IDC_STEP)->EnableWindow(bEnable);
	GetDlgItem(IDC_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTE)->EnableWindow(bEnable);
	m_action->Enabled = bEnable;
	m_item->Enabled = bEnable;
	GetDlgItem(IDC_IMMEDIATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEVER)->EnableWindow(bEnable);
	GetDlgItem(IDC_ASK)->EnableWindow(bEnable);
	GetDlgItem(IDC_TIME_AFTER)->EnableWindow(bEnable);	
	GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(bEnable);
	GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(bEnable);
	GetDlgItem(IDC_PROMPT)->EnableWindow(bEnable);
	GetDlgItem(IDC_PREV_STEP_LABEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_OPEN_PIC_WHEN_SELECTED)->EnableWindow(bEnable);
}



void CPhaseStepDlg::OnRequeryFinishedAssignUsers(short nFlags) 
{
	// (z.manning 2008-07-14 12:49) - PLID 14214 - Add rows for patient coordinator 
	// and user who entered ladder.	
	IRowSettingsPtr pRow = m_pUserList->GetRow(-1);
	pRow->PutValue(ulcID, (long)tssaCreatingUser);
	pRow->PutValue(ulcName, _bstr_t("<User who entered ladder>"));
	m_pUserList->InsertRow(pRow, 0);

	pRow = m_pUserList->GetRow(-1);
	pRow->PutValue(ulcID, (long)tssaPatientCoordinator);
	pRow->PutValue(ulcName, _bstr_t("<Patient Coordinator>"));
	m_pUserList->InsertRow(pRow, 0);

	// (j.jones 2008-11-26 11:25) - PLID 30830 - added option for "<Multiple>"
	pRow = m_pUserList->GetRow(-1);
	pRow->PutValue(ulcID, (long)-2);
	pRow->PutValue(ulcName, _bstr_t("<Multiple>"));
	m_pUserList->InsertRow(pRow, 0);

	pRow = m_pUserList->GetRow(-1);
	pRow->PutValue(ulcID, (long)-1);
	pRow->PutValue(ulcName, _bstr_t("<None>"));
	m_pUserList->InsertRow(pRow, 0);
	
	try {

		// (j.jones 2008-11-26 11:29) - PLID 30830 - we now support multiple users
		m_aryOriginalAssignToIDs.RemoveAll();
		m_aryAssignToIDs.RemoveAll();

		if(m_nStepTemplateID != -1) {
			_RecordsetPtr rsUserIDs = CreateParamRecordset("SELECT UserID FROM StepTemplatesAssignToT WHERE StepTemplateID = {INT}", m_nStepTemplateID);
			while(!rsUserIDs->eof) {
				long nUserID = AdoFldLong(rsUserIDs, "UserID");
				m_aryOriginalAssignToIDs.Add(nUserID);
				m_aryAssignToIDs.Add(nUserID);

				rsUserIDs->MoveNext();
			}
			rsUserIDs->Close();
		}

		//displays either the combo or the hyperlink
		RefreshUserCombo();

	}NxCatchAll("Error in CPhaseStepDlg::OnRequeryFinishedAssignUsers()");
}

BOOL CPhaseStepDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);

	if(GetDlgItem(IDC_CRITERIA_LIST)->IsWindowVisible()) {
		CRect rc;
		GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	// (j.jones 2008-11-26 10:49) - PLID 30830 - detect the multi-user hyperlink
	if (m_aryAssignToIDs.GetSize() > 1) {
		if(GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->IsWindowVisible()) {
			CRect rc;
			GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->GetWindowRect(rc);
			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CPhaseStepDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(GetDlgItem(IDC_CRITERIA_LIST)->IsWindowVisible()) {
		CRect rc;
		GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(point)) {
			EditCriteriaList();
			RefreshItemDisplay();
			return;
		}
	}
	
	// (j.jones 2008-11-26 14:46) - PLID 30830 - supported multiple users
	if(m_aryAssignToIDs.GetSize() > 1 && GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->IsWindowVisible()) {
		CRect rc;
		GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(point)) {
			//ensure "multiple" is selected
			m_pUserList->SetSelByColumn(ulcID, (long)-2);
			OnSelChosenAssignUsers(m_pUserList->GetCurSel());
			return;
		}
	}

	CNxDialog::OnLButtonDown(nFlags, point);
}

void CPhaseStepDlg::EditCriteriaList()
{
	long nAction = VarLong(m_action->Value[m_action->CurSel][0]);
	if(nAction == PA_WriteTemplate) {
		CMultiTemplateDlg dlg(this);
		// (j.armen 2012-01-23 08:59) - PLID 47707 - No need to iterate through, just append
		dlg.m_aryTemplateIDs.Append(m_aryActionIDs);
		if(IDOK == dlg.DoModal()) {
			m_aryActionIDs.RemoveAll();
			m_aryActionIDs.Append(dlg.m_aryTemplateIDs);
		}
	}
	else {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "");
		dlg.PreSelect(m_aryActionIDs);
		CString strFrom, strWhere, strIDField, strValueField, strDescription;
		switch (nAction)
		{	case PA_QuoteSurgery:
			case PA_BillSurgery:
				strFrom = "SurgeriesT";
				strWhere = "";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any surgeries which will complete this step.";
				dlg.SetSizingConfigRT("SurgeriesT");
				break;
			case PA_QuoteCPT:
			case PA_BillCPT:
				strFrom = "(SELECT ServiceT.ID, Code + ' ' + SubCode + ' ' + Name AS Name "
					"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID) AS SubQ";
				strWhere = "";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any service codes which will complete this step.";
				dlg.SetSizingConfigRT("CPTCodeT");
				break;
			case PA_QuoteInventory:
			case PA_BillInventory:
				strFrom = "(SELECT ServiceT.ID, Name "
					"FROM ServiceT INNER JOIN ProductT On ServiceT.ID = ProductT.ID) AS SubQ";
				strWhere = "";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any inventory items which will complete this step.";
				dlg.SetSizingConfigRT("ProductT");
				break;
			case PA_ActualAptCategory:
			case PA_ScheduleAptCategory:
			{	
				strFrom = "(SELECT 1 AS ID, 'Consult' AS Name UNION SELECT 2, 'PreOp' UNION SELECT 3, 'Minor Procedure' "
					"UNION SELECT 4, 'Surgery' UNION SELECT 5, 'Follow-Up' UNION SELECT 6, 'Other Procedure' "
					") SubQ";
				strWhere = "";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any appointment categories which will complete this step.";
				dlg.SetSizingConfigRT("AptCategory");
				break;
			}
			case PA_ActualAptType:
			case PA_ScheduleAptType:
				strFrom = "AptTypeT";
				//We want to hide any inactive ones that wouldn't be checked.
				if(m_aryActionIDs.GetSize() == 0) {
					strWhere = "Inactive = 0";
				}
				else {
					CString strPart;
					strWhere = "Inactive = 0 OR ID IN (";
					for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
						strPart.Format("%li, ", m_aryActionIDs[i]);
						strWhere += strPart;
					}
					strWhere = strWhere.Left(strWhere.GetLength()-2);
					strWhere += ")";
				}
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any appointment types which will complete this step.";
				dlg.SetSizingConfigRT("AptTypeT");
				break;
			case PA_Bill:
			case PA_Quote:
				strFrom = "ProcedureT";
				// (c.haag 2009-01-09 10:39) - PLID 32571 - Don't let users assign inactive procedures to steps. Active steps
				// should not be assigned to any; so don't try to fill in the combo with any existing data (if there is existing,
				// it's bad data)
				strWhere = "Inactive = 0";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any procedures which will complete this step.";
				dlg.SetSizingConfigRT("ProcedureT");
				break;
			case PA_WritePacket:
				strFrom = "PacketsT";
				strWhere = "Deleted = 0";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any packets which will complete this step.";
				dlg.SetSizingConfigRT("PacketsT");
				break;
			case PA_Ladder:
				strFrom = "ProcedureT";
				// (d.moore 2007-06-08 11:25) - PLID 14670 - Modified query to use ProcedureLadderTemplateT
				//  This allows procedures to be associated with multiple ladders.
				// (c.haag 2009-01-09 10:39) - PLID 32571 - Don't let users assign inactive procedures to steps. Active steps
				// should not be assigned to any; so don't try to fill in the combo with any existing data (if there is existing,
				// it's bad data)
				strWhere = "Inactive = 0 AND ID IN (SELECT DISTINCT ProcedureID FROM ProcedureLadderTemplateT)";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any procedures which will complete this step.";
				dlg.SetSizingConfigRT("ProcedureT");
				break;
			// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
			// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
			case PhaseTracking::PA_CreateEMRByCollection:
				strFrom = "EmrCollectionT";
				strWhere = "Inactive = 0";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any EMR Collections which will complete this step.";
				dlg.SetSizingConfigRT("EmrCollectionT");
				break;
			case PhaseTracking::PA_CreateEMRByTemplate:
				strFrom = "EMRTemplateT";
				strWhere = "Deleted = 0 AND (CollectionID Is Null OR CollectionID Not In (SELECT ID FROM EMRCollectionT WHERE Inactive = 1))";
				strIDField = "ID";
				strValueField = "Name";
				strDescription = "Please select any EMR Templates which will complete this step.";
				dlg.SetSizingConfigRT("EMRTemplateT");
				break;

			case PA_Manual:
			case PA_Payment:
			default:
				//This shouldn't happen.
				ASSERT(FALSE);
				return;
				break;
		}
		if(IDOK == dlg.Open(strFrom, strWhere, strIDField, strValueField, strDescription)) {
			dlg.FillArrayWithIDs(m_aryActionIDs);
			m_bActionIdsChanged = true;
			if(m_aryActionIDs.GetSize() < 2) {
				GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
				ShowDlgItem(IDC_ITEM, SW_SHOW);
				if(m_aryActionIDs.GetSize()) {
					m_item->SetSelByColumn(0, m_aryActionIDs[0]);
					if(m_item->CurSel == -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
						//Inactive type.
						// (j.armen 2012-01-26 10:31) - PLID 47711 - Parameratized
						_RecordsetPtr rsType = CreateParamRecordset(
							"SELECT Name FROM AptTypeT WHERE ID = {INT}", m_aryActionIDs[0]);
						m_item->PutComboBoxText(_bstr_t(AdoFldString(rsType, "Name")));
					}
				}
				else {
					m_item->CurSel = -1;
				}
			}
			else {
				m_strItemList = "";
				for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
					long nRow = m_item->FindByColumn(0, m_aryActionIDs[i], 0, TRUE);
					if(nRow ==  -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
						//Inactive appt type.
						// (j.armen 2012-01-26 10:14) - PLID 47711 - Parameratized
						_RecordsetPtr rsName = CreateParamRecordset(
							"SELECT Name FROM AptTypeT WHERE ID = {INT}", m_aryActionIDs[i]);
						m_strItemList += AdoFldString(rsName, "Name") + ", ";
					}
					else {
						m_strItemList += VarString(m_item->GetValue(nRow, 1)) + ", ";
					}
				}
				m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);
				CRect rTextBox;
				GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
				ScreenToClient(&rTextBox);
				// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
				DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
			}
		}
	}
}

void CPhaseStepDlg::RefreshItemDisplay()
{
	try {
		long nAction = VarLong(m_action->Value[m_action->CurSel][0]);
		if(nAction == PA_Manual) {
			ShowDlgItem(IDC_ITEM, SW_HIDE);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
			m_item->CurSel = -1;

			//TES 7/16/2010 - PLID 39705 - Hide the Merge Scope fields
			m_nxstaticMergeScopeLabel.ShowWindow(SW_HIDE);
			m_nxbMergeScopeHelp.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MERGE_SCOPE)->ShowWindow(SW_HIDE);
		}
		else if(nAction == PA_WriteTemplate) {
			//Always show in text form.
			m_strItemList = "";
			for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
				// (j.armen 2012-01-26 09:17) - PLID 47711 - Parameratized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT Path FROM MergeTemplatesT WHERE ID = {INT}", m_aryActionIDs[i]);
				if(!prs->eof) m_strItemList += "{" + AdoFldString(prs, "Path") + "}, ";
			}
			if(m_strItemList == "") {
				m_strItemList = "{No template specified}";
			}
			else {
				m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);
			}
			//Modify display
			ShowDlgItem(IDC_ITEM, SW_HIDE);
			GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_SHOW);
			CRect rTextBox;
			GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
			ScreenToClient(&rTextBox);
			// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
			DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
			InvalidateRect(rTextBox);

			//TES 7/16/2010 - PLID 39705 - Show the Merge Scope fields
			m_nxstaticMergeScopeLabel.ShowWindow(SW_SHOW);
			m_nxbMergeScopeHelp.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MERGE_SCOPE)->ShowWindow(SW_SHOW);
		}
		else {
			//TES 7/16/2010 - PLID 39705 - Hide the Merge Scope fields
			m_nxstaticMergeScopeLabel.ShowWindow(SW_HIDE);
			m_nxbMergeScopeHelp.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MERGE_SCOPE)->ShowWindow(SW_HIDE);

			if(m_aryActionIDs.GetSize() > 1) {
				m_strItemList = "";
				for(int i = 0; i < m_aryActionIDs.GetSize(); i++) {
					long nRow = m_item->FindByColumn(0, m_aryActionIDs[i], 0, FALSE);
					if(nRow ==  -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
						//Inactive appt type.
						// (j.armen 2012-01-26 10:13) - PLID 47711 - Parameratized
						_RecordsetPtr rsName = CreateParamRecordset(
							"SELECT Name FROM AptTypeT WHERE ID = {INT}", m_aryActionIDs[i]);
						m_strItemList += AdoFldString(rsName, "Name") + ", ";
					}
					else {
						m_strItemList += VarString(m_item->GetValue(nRow, 1)) + ", ";
					}
				}
				m_strItemList = m_strItemList.Left(m_strItemList.GetLength()-2);
				//Modify display
				ShowDlgItem(IDC_ITEM, SW_HIDE);
				GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_SHOW);
				CRect rTextBox;
				GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
				ScreenToClient(&rTextBox);
				// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
				DrawTextOnDialog(this, GetDC(), rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
			}
			else {
				//Show the drop-down list.
				ShowDlgItem(IDC_ITEM, SW_SHOW);
				GetDlgItem(IDC_CRITERIA_LIST)->ShowWindow(SW_HIDE);
				if(m_aryActionIDs.GetSize()) {
					m_item->SetSelByColumn(0, m_aryActionIDs[0]);
					if(m_item->CurSel == -1 && (nAction == PA_ScheduleAptType || nAction == PA_ActualAptType)) {
						//Inactive type.
						// (j.armen 2012-01-26 10:28) - PLID 47711 - Parameratized
						_RecordsetPtr rsType = CreateParamRecordset(
							"SELECT Name FROM AptTypeT WHERE ID = {INT}", m_aryActionIDs[0]);
						m_item->PutComboBoxText(_bstr_t(AdoFldString(rsType, "Name")));
					}
				}
				else {
					if(nAction == PA_Payment) {
						//They have to choose one or the other.
						m_item->CurSel = 0;
						OnSelChosenItem(0);
					}
					else {
						m_item->CurSel = -1;
					}
				}

			}
		}
	}NxCatchAll("Error in CPhaseStepDlg::RefreshItemDisplay()");

}

void CPhaseStepDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if(GetDlgItem(IDC_CRITERIA_LIST)->IsWindowVisible()) {
		CRect rTextBox;
		GetDlgItem(IDC_CRITERIA_LIST)->GetWindowRect(rTextBox);
		ScreenToClient(&rTextBox);
		// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, &dc, rTextBox, m_strItemList, dtsHyperlink, false, DT_LEFT, true, false, 0);
	}

	// (j.jones 2008-11-26 14:49) - PLID 30830 - supported multiple users
	if(GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->IsWindowVisible()) {
		CRect rcUserLink;
		GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->GetWindowRect(rcUserLink);
		ScreenToClient(&rcUserLink);
		DrawTextOnDialog(this, &dc, rcUserLink, m_strUserLinkText, dtsHyperlink, false, DT_LEFT, true, false, 0);
	}

	// Do not call CNxDialog::OnPaint() for painting messages
}

void CPhaseStepDlg::OnSelChosenAction(long nRow) 
{
	CString strName;
	GetDlgItemText(IDC_NAME, strName);
	if(strName == "" && nRow != -1) {
		SetDlgItemText(IDC_NAME, VarString(m_action->GetValue(nRow, 1)));
	}

	LoadItemList();	
	m_aryActionIDs.RemoveAll();
	RefreshItemDisplay();
	ReflectCurrentAction();
}

void CPhaseStepDlg::OnStepCreateTodo() 
{
	if(IsDlgButtonChecked(IDC_STEP_CREATE_TODO)) {
		GetDlgItem(IDC_TODO_PRIORITY)->EnableWindow(true);
		GetDlgItem(IDC_TODO_CATEGORY)->EnableWindow(true);
	}
	else{
		GetDlgItem(IDC_TODO_PRIORITY)->EnableWindow(false);
		GetDlgItem(IDC_TODO_CATEGORY)->EnableWindow(false);
	}
}

void CPhaseStepDlg::ReflectCurrentAction()
{
	//TES 2/14/2005 - We may change this later, but for now, Create Ladder steps always just happen, so there's no point
	//in putting them on hold.
	if(m_action->CurSel != -1 && VarLong(m_action->GetValue(m_action->CurSel,0)) == PA_Ladder) {
		CheckRadioButton(IDC_IMMEDIATE, IDC_NEVER, IDC_IMMEDIATE);
		GetDlgItem(IDC_IMMEDIATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIME_AFTER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_INTERVAL_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_INTERVAL_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROMPT)->EnableWindow(FALSE);
		return;
	}
	else {
		GetDlgItem(IDC_IMMEDIATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_TIME_AFTER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEVER)->EnableWindow(TRUE);
	}
}

void CPhaseStepDlg::OnRequeryFinishedTodoCategory(short nFlags) 
{
	
}

void CPhaseStepDlg::OnSelChosenTodoCategory(long nRow) 
{
	if(nRow == -1) {
		m_pTodoCategoryList->CurSel = 0;
	}
}

// (j.jones 2008-11-26 11:17) - PLID 30830 - RefreshUserCombo will display or hide
// the user combo box or hyperlink, based on how many users this step is assigned to
void CPhaseStepDlg::RefreshUserCombo()
{
	try {

		if(m_aryAssignToIDs.GetSize() == 0) {
			m_pUserList->CurSel = -1;
			GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ASSIGN_USERS)->ShowWindow(SW_SHOW);
		}
		else if(m_aryAssignToIDs.GetSize() == 1) {
			m_pUserList->TrySetSelByColumn(ulcID, m_aryAssignToIDs.GetAt(0));
			GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ASSIGN_USERS)->ShowWindow(SW_SHOW);
		}
		else {
			// Calculate the hyperlink text from data based on the selected values
			CArray<long, long> aryUserIDs;
			m_strUserLinkText = "";
			for(int i = 0; i < m_aryAssignToIDs.GetSize(); i++) {
				long nID = m_aryAssignToIDs[i];
				if(nID == tssaCreatingUser) {
					if(!m_strUserLinkText.IsEmpty()) {
						m_strUserLinkText += ",";
					}
					m_strUserLinkText += "<User who entered ladder>";
				}
				else if(nID == (long)tssaPatientCoordinator) {
					if(!m_strUserLinkText.IsEmpty()) {
						m_strUserLinkText += ",";
					}
					m_strUserLinkText += "<Patient Coordinator>";
				}
				else if(nID != -1) {
					aryUserIDs.Add(nID);
				}
			}
			if(!aryUserIDs.IsEmpty()) {
				// (j.armen 2012-01-26 09:38) - PLID 47711 - Parameratized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT Username "
					"FROM UsersT "
					"WHERE PersonID IN ({INTARRAY}) "
					"ORDER BY Username", aryUserIDs);
				while(!prs->eof) {
					if(!m_strUserLinkText.IsEmpty()) {
						m_strUserLinkText += ",";
					}
					m_strUserLinkText += AdoFldString(prs, "UserName");
					prs->MoveNext();
				}
				prs->Close();
			}

			//set the <multiple> row
			m_pUserList->SetSelByColumn(ulcID, (long)-2);
			GetDlgItem(IDC_ASSIGN_USERS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->ShowWindow(SW_SHOW);
			CRect rcUserLink;
			GetDlgItem(IDC_ASSIGN_TO_MULTIPLE_LABEL)->GetWindowRect(rcUserLink);
			ScreenToClient(&rcUserLink);
			DrawTextOnDialog(this, GetDC(), rcUserLink, m_strUserLinkText, dtsHyperlink, false, DT_LEFT, true, false, 0);
			InvalidateRect(rcUserLink);
		}

	}NxCatchAll("Error in CPhaseStepDlg::RefreshUserCombo");
}

// (j.jones 2008-11-26 11:30) - PLID 30830 - detects if m_aryAssignToIDs matches m_aryOriginalAssignToIDs
BOOL CPhaseStepDlg::HasUserIDListChanged()
{
	//throw exceptions to the caller

	//if the sizes are different, it changed
	if(m_aryAssignToIDs.GetSize() != m_aryOriginalAssignToIDs.GetSize()) {
		return TRUE;
	}

	//if the sizes are zero, it didn't change
	if(m_aryAssignToIDs.GetSize() == 0) {
		return FALSE;
	}

	//now we need to compare each value
	int i=0, j=0;
	for(i=0; i<m_aryAssignToIDs.GetSize(); i++) {
		BOOL bFound = FALSE;
		for(j=0; j<m_aryOriginalAssignToIDs.GetSize() && !bFound; j++) {
			if((long)m_aryAssignToIDs.GetAt(i) == (long)m_aryOriginalAssignToIDs.GetAt(j)) {
				bFound = TRUE;
			}
		}

		if(!bFound) {
			//couldn't find one ID in the other list, which means
			//that they have different contents
			return TRUE;
		}
	}

	//all IDs matched, so nothing changed
	return FALSE;
}

// (j.jones 2008-11-26 11:37) - PLID 30830 - added OnSelChosenAssignUsers
void CPhaseStepDlg::OnSelChosenAssignUsers(long nRow)
{
	try {

		if(nRow == -1) {
			m_aryAssignToIDs.RemoveAll();
		}
		else {
			long nUserID = VarLong(m_pUserList->GetValue(nRow, ulcID), -1);
			if(nUserID == -1) {
				//no user
				m_aryAssignToIDs.RemoveAll();		
			}
			else if(nUserID == -2) {
				//they picked "<Multiple>"
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "UsersT");

				//pre-select the currently selected IDs
				int i=0;
				for(i=0; i<m_aryAssignToIDs.GetSize(); i++) {
					dlg.PreSelect(m_aryAssignToIDs[i]);
				}
				dlg.m_strNameColTitle = "Username";

				CString strFrom;
				strFrom.Format("("
					"SELECT PersonT.ID, UsersT.UserName "
					"FROM PersonT "
					"INNER JOIN UsersT ON UsersT.PersonID = PersonT.ID "
					"WHERE PersonT.Archived = 0 AND PersonT.ID > 0 "
					""
					"UNION SELECT %li AS ID, '<User who entered ladder>' AS UserName "
					""
					"UNION SELECT %li AS ID, '<Patient Coordinator>' AS UserName "
					") AS UsersQ",
					tssaCreatingUser, tssaPatientCoordinator);
				if(IDOK == dlg.Open(strFrom, "", "ID", "Username", "Please choose one or more users from the list.", 1)) {
					m_aryAssignToIDs.RemoveAll();
					dlg.FillArrayWithIDs(m_aryAssignToIDs);
				}
				//if they cancelled, m_aryAssignToIDs shouldn't change
			}
			else {
				//they picked only one option
				m_aryAssignToIDs.RemoveAll();
				m_aryAssignToIDs.Add(nUserID);
			}
		}

		//displays either the combo or the hyperlink
		RefreshUserCombo();
		
	}NxCatchAll("Error in CPhaseStepDlg::OnSelChosenAssignUsers");
}
void CPhaseStepDlg::OnMergeScopeHelp()
{
	try {
		//TES 7/16/2010 - PLID 39705 - Just pop up a message box explaining what this means
		AfxMessageBox("When a template is merged from the Letter Writing module, it creates one record for each patient in the merge.  "
			"However, when you merge from tracking, you may choose to have the merged document contain one record for each of various types "
			"of data.  The options are as follows:\r\n\r\n"
			"Patient: Merge one record for the patient, with no procedure information.\r\n"
			"PIC: Merge one record for the PIC, with procedure information included.\r\n"
			"Procedure: Merge one record for each procedure (master procedures and selected detail procedures) on the PIC.\r\n"
			"Master Procedure: Merge one record for each master procedure on the PIC.\r\n"
			"Detail Procedure: Merge one record for each detail procedure on the PIC.\r\n"
			"Prescription: Merge one record for each prescription associated with the PIC.\r\n\r\n"
			"NOTE: When using any option except 'Patient' in the Batch Merge From Tracking dialog, a separate document will be created for each step.\r\n", MB_ICONQUESTION);

	}NxCatchAll(__FUNCTION__);
}
