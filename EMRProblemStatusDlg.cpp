// EMRProblemStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRProblemStatusDlg.h"
#include "globalutils.h"
#include "WellnessDataUtils.h"
#include "DecisionRuleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemStatusDlg dialog

// (j.jones 2008-11-20 16:05) - PLID 28497 - added column enum
enum ProblemStatusColumns {

	pscID = 0,
	pscName,
	pscResolved,
	pscOldResolved, //TES 6/4/2009 - PLID 34371
	pscInactive,
};

CEMRProblemStatusDlg::CEMRProblemStatusDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRProblemStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRProblemStatusDlg)
	//}}AFX_DATA_INIT
	m_bModified = FALSE;
	m_nParentStatusID = -1;
}

long CEMRProblemStatusDlg::GetParentStatusID() const
{
	return m_nParentStatusID;
}

void CEMRProblemStatusDlg::SetParentStatusID(long nParentStatusID)
{
	m_nParentStatusID = nParentStatusID;
}

void CEMRProblemStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRProblemStatusDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_REMOVE_STATUS, m_btnRemove);
	DDX_Control(pDX, IDC_BTN_ADD_STATUS, m_btnAdd);
	DDX_Control(pDX, IDC_NXC_PROBLEM_STATUS, m_nxcTop);
	//}}AFX_DATA_MAP
}

BOOL CEMRProblemStatusDlg::DoesStatusExist(const CString& strStatus)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->GetFirstRow();
	while (NULL != pRow) {
		if (!strStatus.CompareNoCase(VarString(pRow->GetValue(pscName)))) {
			return TRUE;
		}
		pRow = pRow->GetNextRow();
	}
	return FALSE;
}

void CEMRProblemStatusDlg::Save()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->GetFirstRow();
		CString strSqlBatch;
		int i;
		//
		// Build the list of table additions and modifications
		//
		//TES 6/4/2009 - PLID 34371 - Track any statuses whose "Resolved" flag changed, we'll need to update Wellness
		CString strStatusIDs_ChangedResolved;
		while (NULL != pRow) {
			long nStatusID = VarLong(pRow->Value[pscID]);
			if (-2 == nStatusID) {
				// -2 means this is a new status
				// (j.jones 2008-11-20 16:08) - PLID 28497 - added Resolved
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRProblemStatusT (Name, Resolved, Inactive) VALUES ('%s', %li, %li)",
					_Q(VarString(pRow->Value[pscName])), VarBool(pRow->Value[pscResolved]) ? 1 : 0, VarBool(pRow->Value[pscInactive]) ? 1 : 0);
			} else if (nStatusID != 1 && nStatusID != 2) { // 1 is reserved for "Open", and 2 is reserved for "Closed"
				// A positive status ID means it exists in data
				// (c.haag 2007-01-02 10:12) - PLID 22355 - Make it impossible to change the name if the item is in 
				// the EMR problem history or an existing problem (to be comprehensive)
				// (d.moore 2007-10-12) - PLID 27749 - I had to rework the query to allow the Inactive value to be set
				//  all of the time, but the name value to be set only when the name is not in the EMR problem history 
				//  or an existing problem.
				// (j.jones 2008-11-20 16:08) - PLID 28497 - added Resolved, which you are permitted to change on in-use items
				AddStatementToSqlBatch(strSqlBatch, 
					"UPDATE EMRProblemStatusT "
						"SET Inactive = %li, "
						"Resolved = %li, "
							"Name = "
								"CASE WHEN NOT EXISTS "
								"(SELECT StatusID FROM EMRProblemHistoryT WHERE StatusID = %li "
								"UNION "
								"SELECT StatusID FROM EMRProblemsT WHERE StatusID = %li) "
							"THEN '%s' ELSE Name END "
					"WHERE ID = %li", 					
					VarBool(pRow->Value[pscInactive]) ? 1 : 0, 
					VarBool(pRow->Value[pscResolved]) ? 1 : 0,
					nStatusID, nStatusID, 
					_Q(VarString(pRow->Value[pscName])), 
					nStatusID);

				if(VarBool(pRow->Value[pscResolved]) != VarBool(pRow->Value[pscOldResolved])) {
					//TES 6/4/2009 - PLID 34371 - Remember that this status's Resolved flag changed.
					strStatusIDs_ChangedResolved += FormatString("%li,", nStatusID);
				}
			}
			pRow = pRow->GetNextRow();
		}
		//
		// Build the list of row deletions
		//
		for (i=0; i < m_anDeletedStatus.GetSize(); i++) {
			// (c.haag 2007-01-02 10:12) - PLID 22355 - Make it impossible to delete the status if it exists in
			// the EMR problem history or an existing problem (to be comprehensive)
			// (c.haag 2008-07-24 17:07) - PLID 30835 - Do not allow users to delete problems in EmrProblemActionsT
			// (c.haag 2008-08-05 09:16) - PLID 30941 - Added check for EmrProblemActions inactive flag
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRProblemStatusT WHERE ID = %d "
				"AND ID NOT IN (SELECT StatusID FROM EMRProblemHistoryT UNION SELECT StatusID FROM EMRProblemsT UNION SELECT DefaultStatus FROM EmrProblemActionsT WHERE Inactive = 0) ",
				m_anDeletedStatus[i]);
		}
		//
		// Commit the batch
		//
		//TES 11/10/2006 - PLID 23434 - Don't execute a blank statement (for instance, if they just added one status and removed it).
		if(!strSqlBatch.IsEmpty()) {
			// (j.jones 2010-04-19 17:44) - PLID 30852 - convert to a batch execute
			ExecuteSqlBatch(strSqlBatch);

			//TES 10/31/2013 - PLID 59251 - Track what interventions get triggered
			CDWordArray arNewCDSInterventions;

			//TES 6/4/2009 - PLID 34371 - Now, we need to go through all patients who have a problem with a status
			// whose Resolved flag just changed, and have them re-calculate any wellness pre-qualifications based on
			// the fact that they now have a different "active problem list"
			if(!strStatusIDs_ChangedResolved.IsEmpty()) {
				strStatusIDs_ChangedResolved.TrimRight(",");
				ADODB::_RecordsetPtr rsChangedPatIDs = CreateRecordset("SELECT PersonID FROM PatientsT WHERE PersonID IN "
					"(SELECT PatientID FROM EmrProblemsT WHERE EmrProblemsT.StatusID IN (%s))", strStatusIDs_ChangedResolved);
				while(!rsChangedPatIDs->eof) {
					long nPatientID = AdoFldLong(rsChangedPatIDs, "PersonID");
					UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), nPatientID);
					// (c.haag 2010-09-21 11:35) - PLID 40612 - Create todo alarms for decisions
					UpdateDecisionRules(GetRemoteData(), nPatientID, arNewCDSInterventions);
					rsChangedPatIDs->MoveNext();
				}
			}

			//TES 10/31/2013 - PLID 59251 - Notify the user about any interventions that got triggered
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

			// (j.jones 2008-11-20 16:28) - PLID 32119 - we changed something, so send one tablechecker
			CClient::RefreshTable(NetUtils::EMRProblemStatusT);
		}
	} NxCatchAll("Error saving status list");
}

BEGIN_MESSAGE_MAP(CEMRProblemStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRProblemStatusDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_STATUS, OnBtnAddStatus)
	ON_BN_CLICKED(IDC_BTN_REMOVE_STATUS, OnBtnRemoveStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemStatusDlg message handlers

BOOL CEMRProblemStatusDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try 
	{
		// (c.haag 2008-04-29 17:24) - PLID 29840 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		m_dlStatus = BindNxDataList2Ctrl(this, IDC_LIST_MASTER_PROBLEM_STATUS, GetRemoteData(), true);
		//
		// Assign the dialog color
		//
		m_nxcTop.SetColor(GetNxColor(GNC_ADMIN, 0));
	}
	NxCatchAll("Error initializing the EMR problem status configuration dialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRProblemStatusDlg::OnBtnAddStatus() 
{
	try {
		CString strStatus;
		BOOL bRetry = FALSE;
		do {
			if (IDCANCEL == InputBoxLimited(this, "Enter a name for the new status", strStatus, "", 255, false, false, NULL))
				return;
			if (strStatus.IsEmpty()) {
				MsgBox(MB_OK | MB_ICONEXCLAMATION, "You must enter a name for the new status.");
				bRetry = TRUE;
			}
			if (DoesStatusExist(strStatus)) {
				MsgBox(MB_OK | MB_ICONEXCLAMATION, "This status already exists in the list. Please choose another name.");
				bRetry = TRUE;
			}
		} while (bRetry);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->GetNewRow();
		_variant_t vFalse((bool)false);
		pRow->Value[pscID] = -2L;
		pRow->Value[pscName] = (LPCTSTR)strStatus;
		// (j.jones 2008-11-20 16:08) - PLID 28497 - added Resolved
		pRow->Value[pscResolved] = vFalse;
		//TES 6/4/2009 - PLID 34371 - Added a column to track the original value of the Resolved flag
		pRow->Value[pscOldResolved] = vFalse;
		pRow->Value[pscInactive] = vFalse;		
		m_dlStatus->AddRowAtEnd(pRow, NULL);
		m_bModified = TRUE;
	}
	NxCatchAll("Error adding status to the list");
}

void CEMRProblemStatusDlg::OnBtnRemoveStatus() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->CurSel;
		long nStatusID = VarLong(pRow->Value[pscID]);
		if (nStatusID == 1 || nStatusID == 2) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "This is a system status, and cannot be removed.");
			return;
		} else if (nStatusID > 0) {
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if (nStatusID == GetParentStatusID() || ReturnsRecordsParam("SELECT ID FROM EMRProblemsT WHERE StatusID = {INT} UNION SELECT ID FROM EMRProblemHistoryT WHERE StatusID = {INT}", nStatusID, nStatusID))
			{
				if (!VarBool(pRow->Value[pscInactive])) {
					if (IDYES == MsgBox(MB_YESNO | MB_ICONQUESTION, "This status is in use by patient problem records, and cannot be deleted.\n\n"
						"Would you like to mark it as inactive to prevent it from being assigned to future problems?"))
					{
						_variant_t vTrue((bool)true);
						pRow->Value[pscInactive] = vTrue;
					}
				} else {
					MsgBox(MB_OK | MB_ICONEXCLAMATION, "This status is in use by patient problem records, and cannot be deleted.");
				}
				return;
			}
			// (c.haag 2008-07-24 17:07) - PLID 30835 - Do not allow users to delete problems in EmrProblemActionsT
			// (c.haag 2008-08-05 09:16) - PLID 30941 - We now flag problem actions as having been inactive
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			else if (ReturnsRecordsParam("SELECT ID FROM EmrProblemActionsT WHERE DefaultStatus = {INT} AND Inactive = 0", nStatusID)) {
				if (!VarBool(pRow->Value[pscInactive])) {
					if (IDYES == MsgBox(MB_YESNO | MB_ICONQUESTION, "This status is in use by EMR problem actions, and cannot be deleted.\n\n"
						"Would you like to mark it as inactive to prevent it from being manually assigned to future problems?"))
					{
						_variant_t vTrue((bool)true);
						pRow->Value[pscInactive] = vTrue;
					}
				} else {
					MsgBox(MB_OK | MB_ICONEXCLAMATION, "This status is in use by EMR problem actions, and cannot be deleted.");
				}
				return;
			}

			m_anDeletedStatus.Add(nStatusID);
		}
		m_dlStatus->RemoveRow(pRow);
		m_bModified = TRUE;
	}
	NxCatchAll("Error removing status entry");
}

BEGIN_EVENTSINK_MAP(CEMRProblemStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRProblemStatusDlg)
	ON_EVENT(CEMRProblemStatusDlg, IDC_LIST_MASTER_PROBLEM_STATUS, 2 /* SelChanged */, OnSelChangedListMasterProblemStatus, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMRProblemStatusDlg, IDC_LIST_MASTER_PROBLEM_STATUS, 10 /* EditingFinished */, OnEditingFinishedListMasterProblemStatus, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRProblemStatusDlg, IDC_LIST_MASTER_PROBLEM_STATUS, 18 /* RequeryFinished */, OnRequeryFinishedListMasterProblemStatus, VTS_I2)
	ON_EVENT(CEMRProblemStatusDlg, IDC_LIST_MASTER_PROBLEM_STATUS, 8 /* EditingStarting */, OnEditingStartingListMasterProblemStatus, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRProblemStatusDlg::OnSelChangedListMasterProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	GetDlgItem(IDC_BTN_REMOVE_STATUS)->EnableWindow((NULL == lpNewSel) ? FALSE : TRUE);
}

void CEMRProblemStatusDlg::OnEditingFinishedListMasterProblemStatus(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	m_bModified = TRUE;
}

void CEMRProblemStatusDlg::OnRequeryFinishedListMasterProblemStatus(short nFlags) 
{
	try {
		// (c.haag 2006-06-30 15:34) - PLID 19977 - Visually disable the Open and
		// Closed status items. The user is not permitted to change these.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->GetFirstRow();
		while (NULL != pRow) {
			long nStatusID = VarLong(pRow->Value[pscID]);
			if (nStatusID == 1 || nStatusID == 2) {
				pRow->ForeColor = RGB(192,192,192);
			}
			pRow = pRow->GetNextRow();
		}
	} NxCatchAll("Error in OnRequeryFinishedListMasterProblemStatus"); 
}

void CEMRProblemStatusDlg::OnOK() 
{
	if (m_bModified) Save();
	CNxDialog::OnOK();
}

void CEMRProblemStatusDlg::OnEditingStartingListMasterProblemStatus(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2006-06-30 15:36) - PLID 19977 - Don't let the user edit the
		// Open and Closed status items.
		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		long nStatusID = VarLong(pRow->Value[pscID]);
		if (nStatusID == 1 || nStatusID == 2) {
			*pbContinue = FALSE;
		}
		// (c.haag 2007-01-02 10:31) - PLID 22355 - Don't let the user edit a status name if the status is in use
		if (pscName == nCol && nStatusID > 0) {
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if (ReturnsRecordsParam("SELECT ID FROM EMRProblemsT WHERE StatusID = {INT} UNION SELECT ID FROM EMRProblemHistoryT WHERE StatusID = {INT}", nStatusID, nStatusID)) {
				MsgBox("You may not change the name of this status because it is use by EMR-related records.");
				*pbContinue = FALSE;
			}
		}
	} NxCatchAll("Error in OnEditingStartingListMasterProblemStatus"); 
}
