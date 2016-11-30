// PhaseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PhaseTracking.h"
#include "PhaseDlg.h"
#include "PhaseStepDlg.h"
#include "GlobalDataUtils.h"
#include "GetNewIDName.h"
#include "PhaseStatusDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace PhaseTracking;
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CPhaseDlg dialog

CPhaseDlg::CPhaseDlg(CWnd* pParent)
	: CNxDialog(CPhaseDlg::IDD, pParent),
	m_ladderChecker(NetUtils::Ladders),
	m_eventsChecker(NetUtils::TrackableEvents)
{
	//{{AFX_DATA_INIT(CPhaseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/NexTrack_Setup/NexTrack_Setup_with_Sample_Tracking_Ladder.htm";

	m_nLadderID = -1;
}


/////////////////////////////////////////////////////////////////////////////
void CPhaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhaseDlg)
	DDX_Control(pDX, IDC_AUTO_CREATE_LADDER, m_btnAutoCreate);
	DDX_Control(pDX, IDC_EDIT_STEP, m_btnEditStep);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatusBtn);
	DDX_Control(pDX, IDC_NEW_LADDER, m_newLadderBtn);
	DDX_Control(pDX, IDC_DELETE_LADDER, m_deleteLadderBtn);
	DDX_Control(pDX, IDC_DELETE_STEP, m_deleteBtn);
	DDX_Control(pDX, IDC_NEW_STEP, m_newBtn);
	DDX_Control(pDX, IDC_DOWN, m_downBtn);
	DDX_Control(pDX, IDC_UP, m_upBtn);
	DDX_Control(pDX, IDC_NEW_COPY, m_newCopyBtn);
	DDX_Control(pDX, IDC_RENAME, m_btnRename);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPhaseDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPhaseDlg)
	ON_BN_CLICKED(IDC_NEW_STEP, OnNewStep)
	ON_BN_CLICKED(IDC_DELETE_STEP, OnDeleteStep)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_NEW_LADDER, OnNewLadder)
	ON_BN_CLICKED(IDC_DELETE_LADDER, OnDeleteLadder)
	ON_BN_CLICKED(IDC_EDIT_STATUS, OnEditStatus)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_EDIT_STEP, OnEditStep)
	ON_BN_CLICKED(IDC_NEW_COPY, OnNewCopy)
	ON_BN_CLICKED(IDC_AUTO_CREATE_LADDER, OnAutoCreateLadder)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPhaseDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPhaseDlg)
	ON_EVENT(CPhaseDlg, IDC_LIST, 3 /* DblClickCell */, OnDblClickCellList, VTS_I4 VTS_I2)
	ON_EVENT(CPhaseDlg, IDC_LIST, 14 /* DragEnd */, OnDragEndList, VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CPhaseDlg, IDC_LIST, 18 /* RequeryFinished */, OnRequeryFinishedList, VTS_I2)
	ON_EVENT(CPhaseDlg, IDC_LIST, 19 /* LeftClick */, OnLeftClickList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPhaseDlg, IDC_LADDER, 16 /* SelChosen */, OnSelChosenLadder, VTS_I4)
	ON_EVENT(CPhaseDlg, IDC_LADDER, 18 /* RequeryFinished */, OnRequeryFinishedLadder, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhaseDlg message handlers

BOOL CPhaseDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_newBtn.AutoSet(NXB_NEW);
	m_deleteBtn.AutoSet(NXB_DELETE);
	m_upBtn.AutoSet(NXB_UP);
	m_downBtn.AutoSet(NXB_DOWN);
	m_editStatusBtn.AutoSet(NXB_MODIFY);
	m_newLadderBtn.AutoSet(NXB_NEW);
	m_deleteLadderBtn.AutoSet(NXB_DELETE);
	m_btnEditStep.AutoSet(NXB_MODIFY);
	m_newCopyBtn.AutoSet(NXB_NEW);
	m_btnRename.AutoSet(NXB_MODIFY); // (z.manning, 04/16/2008) - PLID 29566 - Set button styles

	m_ladder = BindNxDataListCtrl(IDC_LADDER);
	m_list = BindNxDataListCtrl(IDC_LIST, false);

	//DRT 9/13/2005 - PLID 17490 - This is called by Refresh() anyways, and at this point m_list has
	//	not been loaded, so this code was resetting the status of the "auto start ladder when first
	//	step happens" setting.
//	EnsureCheckBox();
	return TRUE;
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnNewStep() 
{
	try {
		long nLadderID;

		if (!GetLadder(nLadderID))
			return;

		if(ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.LadderTemplateID = {INT}", nLadderID)) {
			int nReturn = MsgBox(MB_OKCANCEL, "This ladder is already being tracked for some patients.  These patients' ladders will not be changed.  The new step will be included in future ladders.");
			if(nReturn == IDCANCEL) return;
		}
		CPhaseStepDlg dlg;
		dlg.m_nStepTemplateID = -1;
		dlg.m_nLadderTemplateID = nLadderID;
		
		if (IDOK == dlg.DoModal())
			m_list->Requery();

	}NxCatchAll("Error in CPhaseDlg::OnNewStep");
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnDeleteStep() 
{
	long nLadderID, nStepOrder;

	if (!GetLadder(nLadderID) || !GetStepOrder(nStepOrder))
		return;

	
	if(MsgBox(MB_YESNO, "Are you sure you want to delete this step?") == IDYES) {
		if(ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.LadderTemplateID = {INT}", nLadderID)) {
			int nReturn = MsgBox(MB_OKCANCEL, "This ladder is already being tracked for some patients.  These patients' ladders will not be changed.  The deleted step will not be included in future ladders.");
			if(nReturn == IDCANCEL) return;
		}

		long nStepID;
		long nCurSel = m_list->CurSel;
		if (nCurSel != -1) {
			nStepID = VarLong(m_list->GetValue(nCurSel, 4));
		
			// (j.gruber 2007-08-17 09:56) - PLID 27091 - get rid of any tracking conversion phases that use this step
			ExecuteParamSql("DELETE FROM TrackingConversionT WHERE LadderTemplateID = {INT} AND (BeginStepTemplateID = {INT} OR EndStepTemplateID = {INT})",
				nLadderID, nStepID, nStepID);

			//now we need to fix the order
			long nCount = 1;
			_RecordsetPtr rsFixSteps = CreateParamRecordset("SELECT LadderTemplateID, StepOrder FROM TrackingConversionT WHERE LadderTemplateID = {INT} ORDER BY StepOrder ASC", nLadderID);
			CParamSqlBatch sqlBatch;
			while (!rsFixSteps->eof) {
				long nStepOrder = AdoFldLong(rsFixSteps, "StepOrder");
				if (nStepOrder != nCount) {
					sqlBatch.Add("UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}",
						nCount, nStepOrder, nLadderID);
				}
				nCount++;
				rsFixSteps->MoveNext();
			}

			if(!sqlBatch.IsEmpty())
				sqlBatch.Execute(GetRemoteData());
		}

		ExecuteParamSql("UPDATE StepTemplatesT SET Inactive = 1 WHERE LadderTemplateID = {INT} AND StepOrder = {INT}", nLadderID, nStepOrder);
		CleanStepOrder(nLadderID);
		m_list->Requery();
		long RowCount = m_list->GetRowCount();
	}
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnUp() 
{
	long nStepOrder, nLadderID;
	
	if (!GetStepOrder(nStepOrder) ||!GetLadder(nLadderID) || nStepOrder == 1) //cannot move first step up
		return;

	BOOL bIsTracked1 = FALSE;
	BOOL bIsTracked2 = FALSE;
	//TES 3/12/2004: Only give this message if both steps are being tracked.
	long nStepID1 = m_list->GetValue(m_list->CurSel, 4);
	long nStepID2 = m_list->GetValue(m_list->CurSel-1, 4);
	bIsTracked1 = ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.ID = {INT}", nStepID1);
	bIsTracked2 = ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.ID = {INT}", nStepID2);
	if(bIsTracked1 && bIsTracked2) {
		int nReturn = MsgBox(MB_OKCANCEL, "This ladder is already being tracked for some patients.  These patients' ladders will not be changed.  The new order will be reflected in future ladders.");
		if(nReturn == IDCANCEL) return;
	}

	MoveStep(nLadderID, nStepOrder - 1, nStepOrder, bIsTracked1 || bIsTracked2);
	CleanStepOrder(nLadderID);
	m_list->Requery();
	m_list->SetSelByColumn(0, nStepOrder - 1L);
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnDown() 
{
	long nStepOrder, nLadderID;
	
	if (!GetStepOrder(nStepOrder) ||!GetLadder(nLadderID) || nStepOrder == GetMaxStep()) //cannot move last step down
		return;

	BOOL bIsTracked1 = FALSE;
	BOOL bIsTracked2 = FALSE;
	//TES 3/12/2004: Only give this message if both steps are being tracked.
	long nStepID1 = m_list->GetValue(m_list->CurSel, 4);
	long nStepID2 = m_list->GetValue(m_list->CurSel+1, 4);
	bIsTracked1 = ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.ID = {INT}", nStepID1);
	bIsTracked2 = ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.ID = {INT}", nStepID2);
	if(bIsTracked1 && bIsTracked2) {
		int nReturn = MsgBox(MB_OKCANCEL, "This ladder is already being tracked for some patients.  These patients' ladders will not be changed.  The new order will be reflected in future ladders.");
		if(nReturn == IDCANCEL) return;
	}

	MoveStep(nLadderID, nStepOrder + 1, nStepOrder, bIsTracked1 || bIsTracked2);
	CleanStepOrder(nLadderID);
	m_list->Requery();
	m_list->SetSelByColumn(0, nStepOrder + 1L);
}

void CPhaseDlg::OnDblClickCellList(long nRowIndex, short nColIndex) 
{
	OnEditStep();
	/*CPhaseStepDlg dlg;
	long step, ladder;

	if (!GetStep(step) ||!GetLadder(ladder))
		return;

	CString strSql;
	strSql.Format("SELECT LadderID FROM LadderProceduresT WHERE ProcedureID IN (SELECT ProcedureT.ID FROM ProcedureT WHERE LadderTemplateID = %li)", VarLong(m_ladder->Value[m_ladder->CurSel][0], -1));
	if(ReturnsRecords(strSql)) {
		int nReturn = MessageBox("This ladder is being tracked for at least one patient. You will be unable to make changes to this step.", NULL, MB_OK);
		dlg.m_bSaveChanges = false;
	}

	if (IDOK == dlg.DoModal(ladder, step, GetMaxStep()))
		m_list->Requery();*/
}

/////////////////////////////////////////////////////////////////////////////
long CPhaseDlg::GetMaxStep()
{
	return m_list->GetRowCount();
}

bool CPhaseDlg::GetStepOrder(long &nStepOrder)
{
	long curSel = m_list->CurSel;
	if (curSel == -1)
		return false;
	nStepOrder = VarLong(m_list->Value[curSel][0], -1);
	if (nStepOrder == -1)
		return false;
	else return true;
}

bool CPhaseDlg::GetLadder(long &nLadderID)
{
	long curSel = m_ladder->CurSel;
	if (curSel == -1)
		return false;
	nLadderID = VarLong(m_ladder->Value[curSel][0], -1);
	if (nLadderID == -1)
		return false;
	else return true;
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnNewLadder() 
{
	CGetNewIDName dlg(this);
	CString strName;
	long nLadderID;
	
	dlg.m_pNewName = &strName;
	dlg.m_nMaxLength = 50;

	if (dlg.DoModal() == IDOK) 
	{	try
		{	
			//first check for this ladder already
			strName.TrimRight();

			if(ReturnsRecordsParam("SELECT ID FROM LadderTemplatesT WHERE Name = {STRING}", strName)) {
				MessageBox("There is already a ladder with this name. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}
			if(strName == "") {
				MessageBox("You cannot enter a blank name for a ladder.");
				return;
			}

			nLadderID = NewNumber("LadderTemplatesT", "ID");
			ExecuteParamSql("INSERT INTO LadderTemplatesT(ID, NAME) SELECT {INT}, {STRING}", nLadderID, strName); 			
			
			// m.carlson - 4/22/2005 - PL 15646 - instead of refreshing the whole ladder list, just add the row
			IRowSettingsPtr pRow;
			pRow = m_ladder->GetRow(-1);
			pRow->PutValue(0,nLadderID);
			pRow->PutValue(1,_bstr_t(strName));
			m_ladder->AddRow(pRow);
			m_ladder->SetSelByColumn(0, nLadderID);
			// m.carlson - Set the m_list to look to our newly-selected ladder for steps, and refresh it
			OnSelChosenLadder(nLadderID);

			GetDlgItem(IDC_DELETE_LADDER)->EnableWindow(TRUE);
			GetDlgItem(IDC_NEW_COPY)->EnableWindow(TRUE);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiLadderCreated, nLadderID, "", strName, aepMedium, aetCreated);

			m_ladderChecker.Refresh();	
		}
		NxCatchAll("Could not add ladder");
	}
}

bool IsSuperUser();

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnDeleteLadder() 
{

//////normal deletion
	try
	{
		long nLadderID;
		
		if (!GetLadder(nLadderID))
			return;

		CString strOld = CString(m_ladder->GetValue(m_ladder->CurSel, 1).bstrVal);
		
		// (z.manning, 08/30/2007) - PLID 18359 - Moved the logic of determining whether or not it was safe to 
		// delete a ladder to a utility function in the PhaseTracking namespace.
		if(!CanDeleteLadderTemplate(nLadderID, FALSE, this)) {
			return;
		}

		if (IDYES == AfxMessageBox("Do you want to delete this ladder?", MB_YESNO))
		{	
			CParamSqlBatch sqlBatch;

			// (j.gruber 2007-08-17 11:20) - PLID 27091 - delete from trackingconversions
			sqlBatch.Add("DELETE FROM TrackingConversionT WHERE LadderTemplateID = {INT}", nLadderID);
			// (j.jones 2008-11-26 13:44) - PLID 30830 - supported multi-users
			sqlBatch.Add("DELETE FROM StepTemplatesAssignToT WHERE StepTemplateID IN (SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = {INT})", nLadderID);
			sqlBatch.Add("DELETE FROM StepCriteriaT WHERE StepTemplateID IN (SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = {INT})", nLadderID);
			sqlBatch.Add("DELETE FROM StepTemplatesT WHERE LadderTemplateID = {INT}", nLadderID);
			sqlBatch.Add("DELETE FROM LadderTemplatesT WHERE ID = {INT}", nLadderID);

			sqlBatch.Execute(GetRemoteData());

			if(m_ladder->GetRowCount() == 0) {
				GetDlgItem(IDC_DELETE_LADDER)->EnableWindow(FALSE); 
				GetDlgItem(IDC_NEW_COPY)->EnableWindow(FALSE); 
			}
			
			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiLadderDeleted, nLadderID, strOld, "<Deleted>", aepMedium, aetDeleted);

			m_ladderChecker.Refresh();

			UpdateView();
		}
	}NxCatchAll("Could not delete ladder");
}

void CPhaseDlg::OnEditStatus() 
{
	CPhaseStatusDlg	dlg(this);
	dlg.DoModal();
}

void CPhaseDlg::Refresh()
{
	// (z.manning, 9/2/2008) - PLID 26886 - Remember the current selection
	GetLadder(m_nLadderID);

	m_ladder->Requery();
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnRename() 
{
	CGetNewIDName dlg(this);
	CString strName;
	long nLadderID;
	
	dlg.m_pNewName = &strName;
	dlg.m_nMaxLength = 50;

	if (!GetLadder(nLadderID))
		return;

	if (dlg.DoModal() == IDOK) 
	{	try
		{
			strName.TrimRight();
			
			if(strName == ""){
				MessageBox("You cannot enter a blank name for a ladder.");
				return;
			}
			
			//first check for this ladder already
			if(ReturnsRecordsParam("SELECT ID FROM LadderTemplatesT WHERE Name = {STRING} AND ID <> {INT}", strName, nLadderID)) {
				MessageBox("There is already a ladder with this name. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			ExecuteParamSql("UPDATE LadderTemplatesT SET NAME = {STRING} WHERE ID = {INT}", strName, nLadderID); 
			m_ladder->Value[m_ladder->CurSel][1] = _bstr_t(strName);

			if (m_ladderChecker.Changed())
				UpdateView();
			m_ladderChecker.Refresh();
		}
		NxCatchAll("Could not rename ladder");
	}	
}


// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnEditStep() 
{
	CPhaseStepDlg dlg;
	int nMaxStep = GetMaxStep();
	long nLadder;

	if (!GetLadder(nLadder))
		return;

	if(m_list->CurSel == -1) return;
	long nStepTemplateID = VarLong(m_list->GetValue(m_list->CurSel, 4));


	if(ReturnsRecordsParam("SELECT ID FROM StepsT WHERE StepTemplateID = {INT}", nStepTemplateID)) {
		dlg.m_bIsTracked = true;
	}
	dlg.m_nLadderTemplateID = nLadder;
	dlg.m_nStepTemplateID = nStepTemplateID;

	if (IDOK == dlg.DoModal()) {
		//TES 1/5/2010 - PLID 36412 - If we just edited the first step of a launchable ladder, then it's possible that the cached list of
		// launchable events has changed, so we'll need to re-calculate them.
		if(IsDlgButtonChecked(IDC_AUTO_CREATE_LADDER)) {
			if(m_list->CurSel == 0) {
				m_eventsChecker.Refresh();
			}
		}
		m_list->Requery();
	}
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::CleanStepOrder(long nLadderID) 
{
	try {
		_RecordsetPtr rsStepTemplates = CreateParamRecordset("SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = {INT} AND Inactive = 0 ORDER BY StepOrder ASC", nLadderID);
		int nRealOrder = 1;
		while(!rsStepTemplates->eof) {
			ExecuteParamSql("UPDATE StepTemplatesT SET StepOrder = {INT} WHERE ID = {INT}", nRealOrder, AdoFldLong(rsStepTemplates, "ID"));
			nRealOrder++;
			rsStepTemplates->MoveNext();
		}
		if(IsDlgButtonChecked(IDC_AUTO_CREATE_LADDER)) {
			m_eventsChecker.Refresh();
		}
	}NxCatchAll("Error in CPhaseDlg::CleanStepOrder()");
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnNewCopy() 
{
	try {
		if(m_ladder->CurSel == -1) {
			MsgBox("Please select a ladder to copy");
			return;
		}

		long nSourceID;
		if(!GetLadder(nSourceID)) {
			MsgBox("Please select a valid ladder to copy");
			return;
		}

		CGetNewIDName dlg(this);
		CString strName;
		long nNewId;
		
		dlg.m_pNewName = &strName;
		dlg.m_nMaxLength = 50;

		if (dlg.DoModal() == IDOK) 
		{
			//first check for this ladder already
			strName.TrimRight();

			if(ReturnsRecordsParam("SELECT ID FROM LadderTemplatesT WHERE Name = {STRING}", strName)) {
				MessageBox("There is already a ladder with this name. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}
			if(strName == "") {
				MessageBox("You cannot enter a blank name for a ladder.");
				return;
			}

			nNewId = NewNumber("LadderTemplatesT", "ID");
			ExecuteParamSql("INSERT INTO LadderTemplatesT(ID, NAME) VALUES ({INT}, {STRING})", nNewId, strName);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiLadderCreated, nNewId, "", strName, aepMedium, aetCreated);

			//Now, go through all the active steps for the ladder we're copying, and insert them into this new ladder.
			_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM StepTemplatesT WHERE LadderTemplateID = {INT} AND Inactive = 0", nSourceID);

			while(!prs->eof) {
				_variant_t vtActivateInterval = prs->Fields->GetItem("ActivateInterval")->Value;

				// (j.jones 2008-11-26 13:32) - PLID 30830 - converted into a batch save 
				CParamSqlBatch sqlBatch;

				sqlBatch.Declare("DECLARE @nID INT");
				sqlBatch.Declare("SET @nID = Coalesce((SELECT Max(ID) FROM StepTemplatesT), 0) + 1 ");

				// (a.walling 2010-03-25 20:17) - PLID 32739 - Todo priority, todo category, and the todo flag were never copied previously!
				_variant_t vtTodoPriority = prs->Fields->GetItem("ToDoPriority")->Value;
				_variant_t vtTodoCategory = prs->Fields->GetItem("ToDoCategory")->Value;

				//TES 8/6/2010 - PLID 39705 - Added default scope.
				_variant_t vtDefaultScope = prs->Fields->GetItem("DefaultScope")->Value;

				// (j.jones 2008-11-17 17:27) - PLID 30926 - added OpenPIC
				sqlBatch.Add("INSERT INTO StepTemplatesT ("
					"ID, LadderTemplateID, StepOrder, StepName, Action, "
					"Note, ActivateType, ActivateInterval, ActivateStringData, Inactive, "
					"Skippable, OpenPIC, ToDoPriority, Todo, TodoCategory, "
					"DefaultScope) "
					"VALUES ("
					"@nID, {INT}, {INT}, {STRING}, {INT}, "
					"{STRING}, {INT}, {VT_I4}, {STRING}, {BOOL}, "
					"{BOOL}, {BOOL}, {VT_I4}, {BOOL}, {VT_I4}, "
					"{VT_I4}"
					")", 
					nNewId, AdoFldLong(prs, "StepOrder"), AdoFldString(prs, "StepName", ""), AdoFldLong(prs, "Action"), 
					AdoFldString(prs, "Note"), AdoFldLong(prs, "ActivateType"), vtActivateInterval, AdoFldString(prs, "ActivateStringData", ""), AdoFldBool(prs, "Inactive"), 
					AdoFldBool(prs, "Skippable"), AdoFldBool(prs, "OpenPIC"), vtTodoPriority, AdoFldBool(prs, "Todo"), vtTodoCategory, vtDefaultScope);
				sqlBatch.Add("INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
					"SELECT @nID, ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT}", AdoFldLong(prs, "ID"));
				// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
				sqlBatch.Add("INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
					"SELECT @nID, UserID FROM StepTemplatesAssignToT WHERE StepTemplateID = {INT}", AdoFldLong(prs, "ID"));

				sqlBatch.Execute(GetRemoteData());

				prs->MoveNext();
			}

			//OK, everything's been added, let's show the user what they've accomplished.

			//Put in the new row.
			IRowSettingsPtr pRow = m_ladder->GetRow(-1);
			pRow->PutValue(0, (long) nNewId);
			pRow->PutValue(1, _bstr_t(strName));
			m_ladder->AddRow(pRow);
			m_ladder->SetSelByColumn(0, (long) nNewId);
			OnSelChosenLadder(m_ladder->CurSel);
		}
	}NxCatchAll("Error in CPhaseDlg::OnNewCopy()");

}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnDragEndList(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags) 
{
	if(nRow != -1 && nFromRow != -1 && nRow != nFromRow) {
		try {
			long nLadderId;
		
			if (!GetLadder(nLadderId)) //cannot move last step down
				return;

			//First, tell them what's about to happen.
			CString strStepName = VarString(m_list->GetValue(nFromRow, 1));
			long nOldStepOrder = VarLong(m_list->GetValue(nFromRow, 0));
			long nNewStepOrder = VarLong(m_list->GetValue(nRow, 0));
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to move the step %s from postion %li to %li in the ladder?", strStepName, nOldStepOrder, nNewStepOrder)) {
				return;
			}

			bool bIsTracked = false;
			if(ReturnsRecordsParam("SELECT StepsT.ID FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepTemplatesT.LadderTemplateID = {INT}", nLadderId)) {
				bIsTracked = true;
				int nReturn = MsgBox(MB_OKCANCEL, "This ladder is already being tracked for some patients.  These patients' ladders will not be changed.  The new order will be reflected in future ladders.");
				if(nReturn == IDCANCEL) return;
			}

			MoveStep(nLadderId, nNewStepOrder, nOldStepOrder, bIsTracked);
			CleanStepOrder(nLadderId);
			m_list->Requery();
			m_list->SetSelByColumn(0, nNewStepOrder);
		}NxCatchAll("Error in CPhaseDlg::OnDragEndList()");
	}
}

void CPhaseDlg::OnRequeryFinishedList(short nFlags) 
{
	if(m_list->GetRowCount() <= 0) {
		GetDlgItem(IDC_DELETE_STEP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_STEP)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE_STEP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_STEP)->EnableWindow(TRUE);
	}
	EnsureCheckBox();

}

void CPhaseDlg::OnRequeryFinishedLadder(short nFlags) 
{
	try
	{
		if(m_ladder->GetRowCount() == 0) {
			GetDlgItem(IDC_DELETE_LADDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_COPY)->EnableWindow(FALSE);
		}

		if(m_list->GetRowCount() == 0) {
			GetDlgItem(IDC_DELETE_STEP)->EnableWindow(FALSE);
		}

		// (z.manning, 9/2/2008) - PLID 26886 - Preserve the previously selected ladder.
		long nRow = m_ladder->FindByColumn(0, m_nLadderID, 0, VARIANT_FALSE);
		if(nRow == -1) {
			if(m_ladder->GetRowCount() > 0) {
				nRow = 0;
			}
			else {
				m_list->Clear();
				return;
			}
		}

		m_ladder->PutCurSel(nRow);
		OnSelChosenLadder(nRow);

	}NxCatchAll("CPhaseDlg::OnRequeryFinishedLadder");
}

void CPhaseDlg::OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == sriNoRow) {
		GetDlgItem(IDC_DELETE_STEP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_STEP)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE_STEP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_STEP)->EnableWindow(TRUE);
	}	
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::EnsureCheckBox()
{
	try {
		long nLadderID;
		if(!GetLadder(nLadderID)) return;
		if(ReturnsRecordsParam("SELECT AutoCreate FROM LadderTemplatesT WHERE ID = {INT} AND AutoCreate = 1", nLadderID)) {
			CheckDlgButton(IDC_AUTO_CREATE_LADDER, BST_CHECKED);
		}
		else {
			CheckDlgButton(IDC_AUTO_CREATE_LADDER, BST_UNCHECKED);
		}
		//Get the type of the first step.
		PhaseAction pa = PA_Manual;
		long nStepID = -1;
		if(m_list->GetRowCount() > 0) {
			pa = (PhaseAction)VarLong(m_list->GetValue(0, 2), PA_Manual);
			nStepID = VarLong(m_list->GetValue(0,4), -1);
		}

		//CHANGES TO THE LIST OF INVALID EVENTS SHOULD ALSO BE REFLECTED IN PhaseTracking.cpp, IsEventLaunchable()

		//We don't allow this for custom steps (obviously), ladder steps (because it could be an infinite loop), and payment
		//steps, (because the payment can't be part of a ladder until the ladder exists).
		// (j.jones 2011-04-07 10:29) - PLID 42354 - also disabled this for merging templates or packets
		if(pa == PA_Manual || pa == PA_Ladder || pa == PA_Payment
			|| pa == PA_WriteTemplate || pa == PA_WritePacket) {			
			SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place.");
			GetDlgItem(IDC_AUTO_CREATE_LADDER)->EnableWindow(FALSE);
			//Also, make sure the property is off, if it's not allowed.
			ExecuteParamSql("UPDATE LadderTemplatesT SET AutoCreate = {BOOL} WHERE ID = {INT}", false, nLadderID);
			CheckDlgButton(IDC_AUTO_CREATE_LADDER, BST_UNCHECKED);
		}
		else {
			GetDlgItem(IDC_AUTO_CREATE_LADDER)->EnableWindow(TRUE);
			long nActionID = -1;
			bool bMultipleCriteria = false;
			_RecordsetPtr rsActionID = CreateParamRecordset("SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT}", nStepID);
			if(!rsActionID->eof) {
				nActionID = AdoFldLong(rsActionID, "ActionID", -1);
				rsActionID->MoveNext();
				if(!rsActionID->eof) bMultipleCriteria = true;
			}
			rsActionID->Close();
			switch(pa) {
			case PA_Quote:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a quote is entered).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified procedures is quoted).");
				}
				else {
					_RecordsetPtr rsProc = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nActionID);
					if(rsProc->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a quote is entered).");
					}
					else {
						CString strProc = AdoFldString(rsProc, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strProc + " is quoted).");
					}
				}
				break;
			case PA_QuoteCPT:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a Service Code is quoted).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified Service Codes is quoted).");
				}
				else {
					_RecordsetPtr rsCPT = CreateParamRecordset("SELECT Code FROM CPTCodeT WHERE ID = {INT}", nActionID);
					if(rsCPT->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a Service Code is quoted).");
					}
					else {
						CString strCPT = AdoFldString(rsCPT, "Code");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when " + strCPT + " is quoted).");
					}
				}
				break;
			case PA_QuoteInventory:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a product is quoted).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified products is quoted).");
				}
				else {
					_RecordsetPtr rsService = CreateParamRecordset("SELECT Name FROM ServiceT WHERE ID = {INT}", nActionID);
					if(rsService->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a product is quoted).");
					}
					else {
						CString strProduct = AdoFldString(rsService, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when " + strProduct + " is quoted).");
					}
				}
				break;
			case PA_Bill:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a bill is entered).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified procedures is billed).");
				}
				else {
					_RecordsetPtr rsProc = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nActionID);
					if(rsProc->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a bill is entered).");
					}
					else {
						CString strProc = AdoFldString(rsProc, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strProc + " is billed).");
					}
				}
				break;
			case PA_BillCPT:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a Service Code is billed).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified Service Codes is billed).");
				}
				else {
					_RecordsetPtr rsCPT = CreateParamRecordset("SELECT Code FROM CPTCodeT WHERE ID = {INT}", nActionID);
					if(rsCPT->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a Service Code is billed).");
					}
					else {
						CString strCPT = AdoFldString(rsCPT, "Code");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when " + strCPT + " is billed).");
					}
				}
				break;
			case PA_BillInventory:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a product is billed).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified products is billed).");
				}
				else {
					_RecordsetPtr rsService = CreateParamRecordset("SELECT Name FROM ServiceT WHERE ID = {INT}", nActionID);
					if(rsService->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a product is billed).");
					}
					else {
						CString strProduct = AdoFldString(rsService, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when " + strProduct + " is billed).");
					}
				}
				break;
			case PA_ScheduleAptCategory:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment is scheduled).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified categories is scheduled).");
				}
				else {
					CString strCategory;
					switch(nActionID) {
					case AC_NON_PROCEDURAL:
						strCategory = "Non Procedural";
						break;
					case AC_CONSULT:
						strCategory = "Consult";
						break;
					case AC_PREOP:
						strCategory = "PreOp";
						break;
					case AC_MINOR:
						strCategory = "Minor Procedure";
						break;
					case AC_SURGERY:
						strCategory = "Surgery";
						break;
					case AC_FOLLOW_UP:
						strCategory = "Follow-Up";
						break;
					case AC_OTHER:
						strCategory = "Other Procedural";
						break;
					case AC_BLOCK_TIME:
						strCategory = "Block Time";
						break;
					}
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strCategory + " is scheduled).");
				}
				break;

			case PA_ScheduleAptType:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment is scheduled).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified types is scheduled).");
				}
				else {
					_RecordsetPtr rsType = CreateParamRecordset("SELECT Name FROM AptTypeT WHERE ID = {INT}", nActionID);
					if(rsType->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment is scheduled).");
					}
					else {
						CString strType = AdoFldString(rsType, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strType + " is scheduled).");
					}
				}
				break;

			case PA_ActualAptCategory:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment takes place).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified categories takes place).");
				}
				else {
					CString strCategory;
					switch(nActionID) {
					case AC_NON_PROCEDURAL:
						strCategory = "Non Procedural";
						break;
					case AC_CONSULT:
						strCategory = "Consult";
						break;
					case AC_PREOP:
						strCategory = "PreOp";
						break;
					case AC_MINOR:
						strCategory = "Minor Procedure";
						break;
					case AC_SURGERY:
						strCategory = "Surgery";
						break;
					case AC_FOLLOW_UP:
						strCategory = "Follow-Up";
						break;
					case AC_OTHER:
						strCategory = "Other Procedural";
						break;
					case AC_BLOCK_TIME:
						strCategory = "Block Time";
						break;
					}
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strCategory + " takes place).");
				}
				break;

			case PA_ActualAptType:
				if(nActionID == -1) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment takes place).");
				}
				else if(bMultipleCriteria) {
					SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when one of the specified types takes place).");
				}
				else {
					_RecordsetPtr rsType = CreateParamRecordset("SELECT Name FROM AptTypeT WHERE ID = {INT}", nActionID);
					if(rsType->eof) {
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when an appointment takes place).");
					}
					else {
						CString strType = AdoFldString(rsType, "Name");
						SetDlgItemText(IDC_AUTO_CREATE_LADDER, "Automatically start this ladder when the first action takes place (i.e., when a " + strType + " takes place).");
					}
				}
				break;

			}
		}

		CRect rc;
		GetDlgItem(IDC_AUTO_CREATE_LADDER)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

	}NxCatchAll("Error in CPhaseDlg::EnsureCheckbox()");
}

// (j.armen 2012-01-26 14:55) - PLID 47811 - Parameratized Queries
void CPhaseDlg::OnAutoCreateLadder() 
{
	try {
		long nLadderID;
		if(!GetLadder(nLadderID)) return;

		ExecuteParamSql("UPDATE LadderTemplatesT SET AutoCreate = {BOOL} WHERE ID = {INT}", IsDlgButtonChecked(IDC_AUTO_CREATE_LADDER), nLadderID);

		m_eventsChecker.Refresh();
	}NxCatchAll("Error in CPhaseDlg::OnAutoCreateLadder()");
}

void CPhaseDlg::OnSelChosenLadder(long nRow) 
{
	try {

		CString where;
		long nLadderID;

		// (j.jones 2011-09-16 15:22) - PLID 44945 - re-enable these buttons if they selected something
		GetDlgItem(IDC_DELETE_LADDER)->EnableWindow(nRow != -1); 
		GetDlgItem(IDC_NEW_COPY)->EnableWindow(nRow != -1);

		if (!GetLadder(nLadderID))
			return;

		m_nLadderID = nLadderID;
		where.Format("LadderTemplateID = %li AND Inactive = 0", nLadderID);
		m_list->WhereClause = _bstr_t(where);
		m_list->Requery();
	
	}NxCatchAll(__FUNCTION__);
}

LRESULT CPhaseDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::Ladders:
			case NetUtils::TrackableEvents: {
				try {
					UpdateView();
				} NxCatchAll("Error in CPhaseDlg::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CPhaseDlg::OnTableChanged");

	return 0;
}