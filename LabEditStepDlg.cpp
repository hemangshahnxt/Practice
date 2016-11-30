// LabEditStepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LabEditStepDlg.h"
#include "MultiSelectDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "AdministratorRc.h"

using namespace NXDATALIST2Lib;

// CLabEditStepDlg dialog
// (z.manning 2008-10-10 10:32) - PLID 21108 - Created

IMPLEMENT_DYNAMIC(CLabEditStepDlg, CNxDialog)

CLabEditStepDlg::CLabEditStepDlg(long nLabProcedureID, CString strLabProcedureName, CWnd* pParent)
	: CNxDialog(CLabEditStepDlg::IDD, pParent)
{
	m_nLabProcedureID = nLabProcedureID;
	m_strLabProcedureName = strLabProcedureName;
	m_nStepID = -1;
	m_varTodoCategoryID.vt = VT_NULL;
	m_bOtherStepsCompletedByHL7 = FALSE;
	m_bCreateLadder = FALSE;
	m_bCompletedByHL7 = FALSE;
	m_bPrevStepIsCompletedByHL7 = FALSE;
	m_bNextStepIsCreateLadder = FALSE;
	m_bOtherStepsMarkedCompletedBySigning = false; 
}

CLabEditStepDlg::~CLabEditStepDlg()
{
}

void CLabEditStepDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAB_STEP_NAME, m_nxeditStepName);
	DDX_Control(pDX, IDC_LAB_CREATE_TODO, m_nxbtnCreateTodo);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_LAB_CREATE_LADDER, m_nxbtnCreateLadder);
	DDX_Control(pDX, IDC_ASSIGN_TO_LABEL, m_nxlblTodoAssignees);
	DDX_Control(pDX, IDC_COMPLETED_BY_HL7, m_nxbtnCompletedByHL7);
	DDX_Control(pDX, IDC_COMPLETED_BY_SIGNING, m_nxbtnCompletedBySigning); 
}


BEGIN_MESSAGE_MAP(CLabEditStepDlg, CNxDialog)
	ON_BN_CLICKED(IDC_LAB_CREATE_TODO, &CLabEditStepDlg::OnBnClickedLabCreateTodo)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_COMPLETED_BY_HL7, &CLabEditStepDlg::OnCompletedByHl7)
	ON_BN_CLICKED(IDC_LAB_CREATE_LADDER, &CLabEditStepDlg::OnLabCreateLadder)
	ON_BN_CLICKED(IDC_COMPLETED_BY_SIGNING, &CLabEditStepDlg::OnCompletedBySigning)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLabEditStepDlg, CNxDialog)
ON_EVENT(CLabEditStepDlg, IDC_LAB_STEP_USER, 18, OnRequeryFinishedLabStepUser, VTS_I2)
ON_EVENT(CLabEditStepDlg, IDC_LAB_TODO_CATEGORY, 18, OnRequeryFinishedLabTodoCategory, VTS_I2)
ON_EVENT(CLabEditStepDlg, IDC_LAB_STEP_USER, 16, CLabEditStepDlg::SelChosenLabStepUser, VTS_DISPATCH)
END_EVENTSINK_MAP()

// CLabEditStepDlg message handlers

BOOL CLabEditStepDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning 2008-10-20 17:45) - PLID 31371 - Hide the tracking option if they don't have a tracking
		// license.
		//TES 1/4/2011 - PLID 37877 - I changed this to disable instead of hide, consistent with the button below and our license
		// handling in general.
		BOOL bShowTracking = g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent);
		m_nxbtnCreateLadder.EnableWindow(bShowTracking);

		//TES 1/4/2011 - PLID 37877 - Likewise for the Completed by HL7 option.
		m_nxbtnCompletedByHL7.EnableWindow(g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent));

		m_nxeditStepName.SetLimitText(100);

		m_nxlblTodoAssignees.SetType(dtsHyperlink);

		m_pdlTodoPriority = BindNxDataList2Ctrl(IDC_LAB_TODO_PRIORITY, false);
		// (z.manning 2008-10-10 10:53) - Populate the todo priority list
		IRowSettingsPtr pRow = m_pdlTodoPriority->GetNewRow();
		pRow->PutValue(tplcPriority, _variant_t((BYTE)3));
		pRow->PutValue(tplcPriorityName, "Low");
		m_pdlTodoPriority->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlTodoPriority->GetNewRow();
		pRow->PutValue(tplcPriority, _variant_t((BYTE)2));
		pRow->PutValue(tplcPriorityName, "Medium");
		m_pdlTodoPriority->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlTodoPriority->GetNewRow();
		pRow->PutValue(tplcPriority, _variant_t((BYTE)1));
		pRow->PutValue(tplcPriorityName, "High");
		m_pdlTodoPriority->AddRowAtEnd(pRow, NULL);

		if(m_nStepID != -1) {
			LoadExisting();
		}
		else {
			//TES 1/4/2011 - PLID 37877 - We need to check whether any other steps for the current ladder have CompletedByHL7 checked.
		
			// (b.spivey, March 29, 2013) - PLID 44188 - Reworked this query to support multiple settings, 
			//		instead of a single "ReturnRecords" call
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT SUM(CASE WHEN HL7Steps.CompletedByHL7 = 1 THEN 1 ELSE 0 END) AS OtherStepCompletedByHL7, "
				"SUM(CASE WHEN HL7Steps.CompletedBySigning = 1 THEN 1 ELSE 0 END) AS OtherStepCompletedBySigning "
				"FROM LabProcedureStepsT HL7Steps "
				"WHERE HL7Steps.Inactive = 0 AND HL7Steps.LabProcedureID = {INT} "
				"GROUP BY LabProcedureID ", m_nLabProcedureID); 

			//Assume false unless we find something. 
			if(!prs->eof) {
				m_bOtherStepsCompletedByHL7 = (AdoFldLong(prs->Fields, "OtherStepCompletedByHL7", 0) > 0 ? TRUE : FALSE); 
				m_bOtherStepsMarkedCompletedBySigning = (AdoFldLong(prs->Fields, "OtherStepCompletedBySigning", 0) > 0 ? true : false); 
			}
			else {
				m_bOtherStepsCompletedByHL7 = FALSE;
				m_bOtherStepsMarkedCompletedBySigning = false; 
			}
		}

		m_pdlTodoCategory = BindNxDataList2Ctrl(IDC_LAB_TODO_CATEGORY, true);
		m_pdlUsers = BindNxDataList2Ctrl(IDC_LAB_STEP_USER, true);

		UpdateTodoControls();

	}NxCatchAll("CLabEditStepDlg::OnInitDialog");

	return TRUE;
}

void CLabEditStepDlg::LoadExisting()
{
	//TES 1/4/2011 - PLID 37877 - Added CompletedByHL7, as well as OtherCompleteByHL7Steps, which indicates
	// whether or not the current ladder has any other steps flagged as CompletedByHL7, because we want to warn if they select multiple.
	// (b.spivey, March 19, 2013) - PLID 44188 - Reworked this to support returning multiple flags from lab steps. 
	ADODB::_RecordsetPtr prsLoad = CreateParamRecordset(
		"SELECT Name, UserID, CreateTodo, TodoCategoryID, TodoPriority, CreateLadder, CompletedByHL7, CompletedBySigning, \r\n"
		"CONVERT(BIT, CASE WHEN HL7Ladders.OtherStep = 0 THEN 0 ELSE 1 END) AS OtherCompleteByHL7Steps, "
		"CONVERT(BIT, CASE WHEN HL7Ladders.SigningComplete = 0 THEN 0 ELSE 1 END) AS OtherCompletedBySigning "
		"FROM LabProcedureStepsT \r\n"
		"LEFT JOIN LabProcedureStepTodoAssignToT ON LabProcedureStepsT.StepID = LabProcedureStepTodoAssignToT.LabProcedureStepID \r\n"
		"LEFT JOIN (SELECT SUM(CASE WHEN HL7Steps.CompletedByHL7 = 1 THEN 1 ELSE 0 END) AS OtherStep, "
		"	SUM(CASE WHEN HL7Steps.CompletedBySigning = 1 THEN 1 ELSE 0 END) AS SigningComplete, "
		"	LabProcedureID FROM LabProcedureStepsT HL7Steps "
		"	WHERE HL7Steps.StepID <> {INT} AND  HL7Steps.Inactive = 0 "
		"	GROUP BY LabProcedureID) AS HL7Ladders ON LabProcedureStepsT.LabProcedureID = HL7Ladders.LabProcedureID "
		"WHERE StepID = {INT} \r\n"
		, m_nStepID, m_nStepID);
	if(prsLoad->eof) {
		MessageBox("Lab step ID " + AsString(m_nStepID) + " does not exist.");
		EndDialog(IDCANCEL);
		return;
	}

	ADODB::FieldsPtr pflds = prsLoad->Fields;
	m_nxeditStepName.SetWindowText(AdoFldString(pflds, "Name", ""));
	// (r.goldschmidt 2014-08-20 09:25) - PLID 53701 - remember initial value for rename auditing
	m_nxeditStepName.GetWindowText(m_strStepName);
	m_nxbtnCreateTodo.SetCheck(AdoFldBool(pflds, "CreateTodo") ? BST_CHECKED : BST_UNCHECKED);
	m_pdlTodoPriority->SetSelByColumn(tplcPriority, AdoFldByte(pflds, "TodoPriority"));
	// (z.manning 2008-10-20 13:21) - PLID 31371 - Load tracking info
	m_nxbtnCreateLadder.SetCheck(AdoFldBool(pflds, "CreateLadder") ? BST_CHECKED : BST_UNCHECKED);
	// (z.manning 2008-10-10 12:27) - These will be set when the lists are done requerying
	m_varTodoCategoryID = pflds->GetItem("TodoCategoryID")->Value;
	//TES 1/4/2011 - PLID 37877 - Added CompletedByHL7 and OtherCompleteByHL7Steps
	m_nxbtnCompletedByHL7.SetCheck(AdoFldBool(pflds, "CompletedByHL7") ? BST_CHECKED : BST_UNCHECKED);	
	m_bOtherStepsCompletedByHL7 = AdoFldBool(pflds, "OtherCompleteByHL7Steps");
	// (b.spivey, March 18, 2013) - PLID 44188 - Set the check appropriately, 
	// 		make sure we flag if others are marked as completed by signing
	m_nxbtnCompletedBySigning.SetCheck(!!AdoFldBool(pflds, "CompletedBySigning") ? BST_CHECKED : BST_UNCHECKED); 
	m_bOtherStepsMarkedCompletedBySigning = !!AdoFldBool(pflds, "OtherCompletedBySigning"); 

	// (z.manning 2008-10-22 12:23) - We allow multiple assignees for todos
	for(; !prsLoad->eof; prsLoad->MoveNext()) {
		long nUserID = AdoFldLong(prsLoad->GetFields(), "UserID", -1);
		if(nUserID != -1) {
			m_arynTodoUserIDs.Add(nUserID);
		}
	}

	if(m_pdlUsers != NULL) {
		UpdateTodoControls();
	}
}

void CLabEditStepDlg::SetStepID(const long nStepID)
{
	m_nStepID = nStepID;
}

long CLabEditStepDlg::GetStepID()
{
	return m_nStepID;
}

void CLabEditStepDlg::OnOK()
{
	try
	{
		if(Save()) {
			CDialog::OnOK();
		}

	}NxCatchAll("CLabEditStepDlg::OnOK");
}

BOOL CLabEditStepDlg::Save()
{
	// (r.goldschmidt 2014-08-19 18:15) - PLID 53701
	bool bIsNewStep = (m_nStepID == -1);
	CString strOldStepName = m_strStepName;

	// (z.manning 2008-10-10 12:28) - First collect all the data
	m_nxeditStepName.GetWindowText(m_strStepName);
	BOOL bCreateTodo = m_nxbtnCreateTodo.GetCheck() == BST_CHECKED;

	if (m_pdlTodoCategory->GetCurSel() != NULL) {
		m_varTodoCategoryID = m_pdlTodoCategory->GetCurSel()->GetValue(tclcID);
	}
	BYTE nTodoPriority = 2;
	if (m_pdlTodoPriority->GetCurSel() != NULL) {
		nTodoPriority = VarByte(m_pdlTodoPriority->GetCurSel()->GetValue(tplcPriority));
	}

	// (z.manning 2008-10-20 13:26) - PLID 31371 - Ladder info
	m_bCreateLadder = m_nxbtnCreateLadder.GetCheck() == BST_CHECKED;

	//TES 1/4/2011 - PLID 37877 - Added CompletedByHl7
	m_bCompletedByHL7 = m_nxbtnCompletedByHL7.GetCheck() == BST_CHECKED;

	// (b.spivey, March 18, 2013) - PLID 44188 - We're checked off. 
	bool bCompletedBySigning = (m_nxbtnCompletedBySigning.GetCheck() == BST_CHECKED ? true : false);

	// (z.manning 2008-10-10 12:28) - Validate the data
	if (m_strStepName.IsEmpty()) {
		MessageBox("Please enter a step name.");
		return FALSE;
	}

	if (bCreateTodo && m_arynTodoUserIDs.GetSize() == 0) {
		MessageBox("Please select a user.");
		return FALSE;
	}

	CString strSql = BeginSqlBatch();
	CNxParamSqlArray aryParams;
	AddDeclarationToSqlBatch(strSql, "DECLARE @nStepID int");
	if (bIsNewStep)
	{
		// (z.manning 2008-10-10 12:30) - Save a new step
		// (z.manning 2008-10-20 13:28) - PLID 31371 - Added ladder fields
		//TES 1/4/2011 - PLID 37877 - Added CompletedByHL7
		// (r.goldschmidt 2014-08-20 13:15) - PLID 53701 - set nocount on
		AddParamStatementToSqlBatch(strSql, aryParams,
			"SET NOCOUNT ON \r\n"
			"DECLARE @nStepOrder INT \r\n"
			//calculate our next step order
			"SET @nStepOrder = (SELECT COALESCE(MAX(StepOrder), 0) +1 FROM LabProcedureStepsT "
			"   WHERE LabProcedureID = {INT} AND Inactive = 0) \r\n"

			// (b.spivey, March 18, 2013) - PLID 44188 - Save the completed by signing value.
			//Insert the new record
			"INSERT INTO LabProcedureStepsT (LabProcedureID, StepOrder, Inactive, Name, CreateTodo, TodoCategoryID, TodoPriority, CreateLadder, CompletedByHL7, CompletedBySigning) \r\n"
			"VALUES({INT}, @nStepOrder, 0, {STRING}, {BIT}, {VT_I4}, {INT}, {BIT}, {BIT}, {BIT}) \r\n"
			, m_nLabProcedureID, m_nLabProcedureID, m_strStepName, bCreateTodo
			, m_varTodoCategoryID, nTodoPriority, m_bCreateLadder, m_bCompletedByHL7, bCompletedBySigning);
		AddStatementToSqlBatch(strSql, "SET @nStepID = (SELECT SCOPE_IDENTITY())");
	}
	else
	{
		// (z.manning 2008-10-10 12:30) - Save existing step
		// (z.manning 2008-10-20 13:28) - PLID 31371 - Added ladder fields
		//TES 1/4/2011 - PLID 37877 -  Added CompletedByHL7
		// (r.goldschmidt 2014-08-20 13:15) - PLID 53701 - set nocount on
		AddParamStatementToSqlBatch(strSql, aryParams,
			"SET NOCOUNT ON \r\n"
			"SET @nStepID = {INT} \r\n"
			"UPDATE LabProcedureStepsT \r\n"
			"SET Name = {STRING} \r\n"
			"	, CreateTodo = {BIT} \r\n"
			"	, TodoCategoryID = {VT_I4} \r\n"
			"	, TodoPriority = {INT} \r\n"
			"	, CreateLadder = {BIT} \r\n"
			"	, CompletedByHL7 = {BIT} \r\n"
			"	, CompletedBySigning = {BIT} \r\n "
			"WHERE StepID = @nStepID \r\n"
			, m_nStepID, m_strStepName, bCreateTodo, m_varTodoCategoryID, nTodoPriority, m_bCreateLadder, m_bCompletedByHL7, bCompletedBySigning);
		AddStatementToSqlBatch(strSql,
			"DELETE FROM LabProcedureStepTodoAssignToT WHERE LabProcedureStepID = @nStepID");
	}
	for (int nUserIndex = 0; nUserIndex < m_arynTodoUserIDs.GetSize(); nUserIndex++) {
		long nUserID = m_arynTodoUserIDs.GetAt(nUserIndex);
		AddParamStatementToSqlBatch(strSql, aryParams,
			"INSERT INTO LabProcedureStepTodoAssignToT (LabProcedureStepID, UserID) \r\n"
			"VALUES (@nStepID, {INT}) "
			, nUserID);
	}
	// (r.goldschmidt 2014-08-20 13:15) - PLID 53701 - Get New Step ID
	AddStatementToSqlBatch(strSql,
		"SET NOCOUNT OFF \r\n"
		"SELECT StepID, StepOrder FROM LabProcedureStepsT WHERE StepID = @nStepID ");

	// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
	// (r.goldschmidt 2014-08-20 13:15) - PLID 53701 - get recordset instead
	ADODB::_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSql, aryParams);

	if (!prs->eof){

		// (r.goldschmidt 2014-08-19 18:15) - PLID 53701 - need auditing for Adminstrator>Labs tab. (lab procedure step new, rename, and edit)
		long nRecordID = AdoFldLong(prs, "StepID");
		long nStepOrder = AdoFldLong(prs, "StepOrder");
		CString strNewValue;
		strNewValue.Format("%s: (Step %li) %s", m_strLabProcedureName, nStepOrder, m_strStepName);
		long nAuditID = BeginNewAuditEvent();
		if (bIsNewStep){
			AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepCreated, nRecordID, "", strNewValue, aepMedium, aetCreated);
		}
		else {
			// assumes an edit took place rather than checking all the possible changes; technically it does delete and then reapply ToDo Users
			AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepModified, m_nStepID, "", strNewValue + " Was Modified", aepMedium, aetChanged);
			if (strOldStepName.Compare(m_strStepName)){
				CString strOldValue;
				strOldValue.Format("%s: (Step %li) %s", m_strLabProcedureName, nStepOrder, strOldStepName);
				AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepRenamed, nRecordID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
		}
	}

	return TRUE;
}

CString CLabEditStepDlg::GetStepName()
{
	return m_strStepName;
}

//TES 1/5/2011 - PLID 37877 - Added
BOOL CLabEditStepDlg::GetCreateLadder()
{
	return m_bCreateLadder;
}

//TES 1/5/2011 - PLID 37877 - Added
BOOL CLabEditStepDlg::GetCompletedByHL7()
{
	return m_bCompletedByHL7;
}

void CLabEditStepDlg::OnRequeryFinishedLabStepUser(short nFlags)
{
	try
	{
		IRowSettingsPtr pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(ulcID, (long)-2);
		pRow->PutValue(ulcUsername, "{ Multiple Users }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());

		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(ulcID, g_cvarNull);
		pRow->PutValue(ulcUsername, "{ No User }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		UpdateTodoControls();

	}NxCatchAll("CLabEditStepDlg::OnRequeryFinishedLabStepUser");
}

void CLabEditStepDlg::OnRequeryFinishedLabTodoCategory(short nFlags)
{
	try
	{
		IRowSettingsPtr pRow = m_pdlTodoCategory->GetNewRow();
		pRow->PutValue(tclcID, g_cvarNull);
		pRow->PutValue(tclcCategory, "{ No Category }");
		m_pdlTodoCategory->AddRowBefore(pRow, m_pdlTodoCategory->GetFirstRow());
		
		m_pdlTodoCategory->SetSelByColumn(tclcID, m_varTodoCategoryID);

	}NxCatchAll("CLabEditStepDlg::OnRequeryFinishedLabTodoCategory");
}

void CLabEditStepDlg::OnBnClickedLabCreateTodo()
{
	try
	{
		UpdateTodoControls();

	}NxCatchAll("CLabEditStepDlg::OnBnClickedLabCreateTodo");
}

void CLabEditStepDlg::UpdateTodoControls()
{
	BOOL bEnable = m_nxbtnCreateTodo.GetCheck() == BST_CHECKED;
	GetDlgItem(IDC_LAB_STEP_USER)->EnableWindow(bEnable);
	GetDlgItem(IDC_LAB_TODO_CATEGORY)->EnableWindow(bEnable);
	GetDlgItem(IDC_LAB_TODO_PRIORITY)->EnableWindow(bEnable);
	m_nxlblTodoAssignees.EnableWindow(bEnable);

	if(m_arynTodoUserIDs.GetSize() > 1) {
		GetDlgItem(IDC_LAB_STEP_USER)->ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_ASSIGN_TO_LABEL, SW_SHOW);
		CString strLabelText;
		for(int nUserIndex = 0; nUserIndex < m_arynTodoUserIDs.GetSize(); nUserIndex++) {
			long nUserID = m_arynTodoUserIDs.GetAt(nUserIndex);
			IRowSettingsPtr pRow = m_pdlUsers->FindByColumn(ulcID, nUserID, NULL, VARIANT_FALSE);
			if(pRow != NULL) {
				strLabelText += VarString(pRow->GetValue(ulcUsername), "") + ", ";
			}
		}
		strLabelText.TrimRight(" ,");
		m_nxlblTodoAssignees.SetText(strLabelText);
	}
	else {
		GetDlgItem(IDC_LAB_STEP_USER)->ShowWindow(SW_SHOW);
		ShowDlgItem(IDC_ASSIGN_TO_LABEL, SW_HIDE);
		m_nxlblTodoAssignees.ShowWindow(SW_HIDE);

		if(m_arynTodoUserIDs.GetSize() == 1) {
			m_pdlUsers->SetSelByColumn(ulcID, m_arynTodoUserIDs.GetAt(0));
		}
		else {
			m_pdlUsers->SetSelByColumn(ulcID, g_cvarNull);
		}
	}
}


void CLabEditStepDlg::SelChosenLabStepUser(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_arynTodoUserIDs.RemoveAll();
			return;
		}

		long nUserID = VarLong(pRow->GetValue(ulcID), -1);
		if(nUserID == -1) {
			m_arynTodoUserIDs.RemoveAll();
		}
		else if(nUserID == -2) {
			HandleTodoUserMutiSelection();
		}
		else {
			m_arynTodoUserIDs.RemoveAll();
			m_arynTodoUserIDs.Add(nUserID);
		}

		UpdateTodoControls();

	}NxCatchAll("CLabEditStepDlg::SelChosenLabStepUser");
}

LRESULT CLabEditStepDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		HandleTodoUserMutiSelection();

	}NxCatchAll("CLabEditStepDlg::OnLabelClick");

	return S_OK;
}

void CLabEditStepDlg::HandleTodoUserMutiSelection()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "UsersT");
	dlg.PreSelect(m_arynTodoUserIDs);
	if(IDOK == dlg.Open((LPCTSTR)m_pdlUsers->GetFromClause(), (LPCTSTR)m_pdlUsers->GetWhereClause(), "ID", "Username", "Select users to assign to this lab step"))
	{
		m_arynTodoUserIDs.RemoveAll();
		dlg.FillArrayWithIDs(m_arynTodoUserIDs);

		UpdateTodoControls();
	}
}
BOOL CLabEditStepDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try
	{
		// (z.manning 2008-10-22 13:50) - If the todo user label is visible be sure to use
		// the link cursor when hovering over it.
		if(m_nxlblTodoAssignees.IsWindowVisible())
		{
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			m_nxlblTodoAssignees.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	}NxCatchAll("CLabEditStepDlg::OnSetCursor");

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
void CLabEditStepDlg::OnCompletedByHl7()
{
	try {
		//TES 1/4/2011 - PLID 37877 - If they're checking the box, and other steps already have this box checked, warn the user, because
		// that's kind of weird.
		if(IsDlgButtonChecked(IDC_COMPLETED_BY_HL7)) {
			if(m_bOtherStepsCompletedByHL7) {
				if(IDYES != MsgBox(MB_YESNO, "Other steps on this ladder are flagged to be marked as complete when an HL7 result is received.  "
					"If multiple steps have this box checked, then ALL of them will be marked as complete when the first result is "
					"received via HL7.  Are you sure you wish to continue?")) {
						CheckDlgButton(IDC_COMPLETED_BY_HL7, BST_UNCHECKED);
				}
			}
			//TES 1/5/2011 - PLID 37877 - Warn the user if this would cause an invalid setup.
			if(m_bNextStepIsCreateLadder) {
				if(IDYES != MsgBox(MB_YESNO, "The next step on this ladder is flagged to prompt to create a ladder when it becomes active.  "
					"Because ladders cannot be created during HL7 imports, if this step is automatically completed by an HL7 result, the "
					"next step will NOT prompt to create a ladder.\r\n\r\n"
					"Are you sure you wish to continue?")) {
						CheckDlgButton(IDC_COMPLETED_BY_HL7, BST_UNCHECKED);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, March 19, 2013) - PLID 44188 - Warn them if there are other steps with this flag. 
void CLabEditStepDlg::OnCompletedBySigning()
{
	try {
		
		if(IsDlgButtonChecked(IDC_COMPLETED_BY_SIGNING)) {
			if(m_bOtherStepsMarkedCompletedBySigning) {
				// (b.spivey, April 12, 2013) - PLID 44188 - The message box had the wrong text in it.
				if(IDYES != MsgBox(MB_YESNO, "Other steps on this ladder are flagged to be marked as complete when the results are signed. "
					"If multiple steps have this box checked, then ALL of them will be marked as complete when all results have been "
					"signed.  Are you sure you wish to continue?")) {
						CheckDlgButton(IDC_COMPLETED_BY_SIGNING, BST_UNCHECKED);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}


//TES 1/5/2011 - PLID 37877 - Need to call these functions to tell the dialog whether either of these things are true, so that
// it can warn the user appropriately of invalid setups.
void CLabEditStepDlg::SetPrevStepIsCompletedByHL7()
{
	m_bPrevStepIsCompletedByHL7 = TRUE;
}
void CLabEditStepDlg::SetNextStepIsCreateLadder()
{
	m_bNextStepIsCreateLadder = TRUE;
}

void CLabEditStepDlg::OnLabCreateLadder()
{
	try {
		//TES 1/5/2011 - PLID 37877 - Warn the user if this would cause an invalid setup.
		if(IsDlgButtonChecked(IDC_LAB_CREATE_LADDER)) {
			if(m_bPrevStepIsCompletedByHL7) {
				if(IDYES != MsgBox(MB_YESNO, "The previous step on this ladder is flagged to be automatically completed when HL7 results "
					"are received.  Because ladders cannot be created during HL7 imports, if that step is automatically completed by an "
					"HL7 result, this step will NOT prompt to create a ladder.\r\n\r\n"
					"Are you sure you wish to continue?")) {
						CheckDlgButton(IDC_LAB_CREATE_LADDER, BST_UNCHECKED);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}
