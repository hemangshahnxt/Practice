// EmrProblemDeleteDlg.cpp : implementation file
//
// (c.haag 2008-07-23 16:30) - PLID 30820 - Initial implementation
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EmrProblemDeleteDlg.h"
#include "EMNDetail.h"
#include "EmrColors.h"
#include "DiagSearchUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (c.haag 2008-08-06 11:24) - PLID 30942 - Enumerations for the problem list
// (j.jones 2008-10-30 15:28) - PLID 31869 - added plcEMNID and plcDataType
// (a.walling 2009-05-04 09:42) - PLID 33751, 28495 - Add diag code and chronicity
// (j.jones 2009-05-29 11:45) - PLID 34301 - added problem link ID and problem link ptr
enum ProblemListColumns {

	plcProblemID = 0,
	plcProblemLinkID,
	plcDetailID,
	plcEMNID,
	plcDataType,
	plcEnteredDate,
	plcOnsetDate,
	plcStatusID,
	// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
	plcDiagCodeName_ICD9,
	plcDiagCodeName_ICD10,
	plcChronicityName,
	plcDescription,
	plcDetailValue,
	plcDetailName,
	plcTopicName,
	plcEMNName,
	plcEMRName,
	plcProblemPtr,
	plcProblemLinkPtr,
};

using namespace NXDATALIST2Lib;

//TES 2/26/2010 - PLID 37463 - Commented out dead code
/*UINT PopulateDeletedProblemValuesThread(LPVOID p)
{
	CEmrProblemDeleteDlg* pDlg = (CEmrProblemDeleteDlg*)p;
	try {
		//
		// Open a new connection based on the existing one
		//
		ADODB::_ConnectionPtr pExistingConn = GetRemoteData();
		ADODB::_ConnectionPtr pCon(__uuidof(ADODB::Connection));
		pCon->PutConnectionTimeout(pExistingConn->GetConnectionTimeout());
		pCon->Open(pExistingConn->ConnectionString, "", "", 0);
		pCon->PutCommandTimeout(pExistingConn->GetCommandTimeout());
		//
		// Go through each row and fill in the detail value column,
		// for detail problems (and list item problems) only
		//

		CArray<CEMN*, CEMN*> aryEMNs;

		NXDATALIST2Lib::IRowSettingsPtr pRow = pDlg->m_dlList->GetFirstRow();
		while (NULL != pRow) {

			// Load the detail
			long nDetailID = VarLong(pRow->GetValue(plcDetailID), -1);

			if(nDetailID != -1) {

				// (j.jones 2008-10-30 14:37) - PLID 31869 - If we have a Narrative or a Table,
				// we need to load the entire EMN, otherwise our results won't be reliable
				// (narratives won't show their linked items, tables won't show linked details).

				long nEMNID = VarLong(pRow->GetValue(plcEMNID), -1);
				EmrInfoType eDataType = (EmrInfoType)VarByte(pRow->GetValue(plcDataType), eitInvalid);
				
				//first see if we already have this EMN
				int i=0;
				BOOL bFound = FALSE;
				CEMN *pEMN = NULL;
				for(i=0; i<aryEMNs.GetSize() && pEMN == NULL; i++) {
					CEMN *pEMNToCheck = (CEMN*)aryEMNs.GetAt(i);
					if(pEMNToCheck->GetID() == nEMNID) {
						pEMN = pEMNToCheck;
					}
				}

				//now if we do not have this EMN already, but this is a Narrative or Table,
				//create the EMN and add it to our array
				if(pEMN == NULL && (eDataType == eitNarrative || eDataType == eitTable)) {
					pEMN = new CEMN(NULL);
					pEMN->LoadFromEmnID(nEMNID);
					aryEMNs.Add(pEMN);
				}

				CEMNDetail *pDetail = NULL;
				// (j.jones 2008-10-30 14:38) - PLID 31869 - if we have an EMN, just find the already-loaded detail in it
				if(pEMN) {
					pDetail = pEMN->GetDetailByID(nDetailID);
				}

				//if we have no detail, load it now, which is way faster
				//than always loading the EMN
				BOOL bIsLocal = FALSE;
				if(pDetail == NULL) {
					// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
					pDetail = CEMNDetail::CreateDetail(NULL, "EMR problem delete local detail");
					// Load the detail
					pDetail->LoadFromDetailID(nDetailID, NULL);
					bIsLocal = TRUE;
				}

				CString strSentence;
				// Get the detail value in sentence form
				// (c.haag 2006-07-03 15:44) - Copied from CEmrItemAdvDlg::GetToolTipText()
				// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
				if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
					strSentence = pDetail->m_bUseLongFormSmartStamp ? pDetail->m_strLongFormSmartStamp : pDetail->m_strLongForm;
				} else {
					CStringArray saDummy;
					// (c.haag 2006-11-14 10:49) - PLID 23543 - If the info type is an image, we may get a debug assertion
					// failure when calling GetSentence. Rather than try to get a sentence, just return a sentinel value.
					if (eitImage == pDetail->m_EMRInfoType) {
						strSentence = "<image>";
					} else {
						// (c.haag 2008-02-22 13:53) - PLID 29064 - GetSentence may access the database when doing calculations on
						// dropdown table columns. Make sure we pass in our connection object so it won't try to use the global one
						// which belongs to the main thread.
						strSentence = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL, pCon);
					}
				}

				// Assign the sentence
				pRow->PutValue(plcDetailValue, (LPCTSTR)strSentence);

				// (j.jones 2008-10-30 14:38) - PLID 31869 - We don't need to delete detail if it was from the EMN,
				// because it would be handled when the EMN is deleted. But we may have created this detail on our
				//own, and if so, we need to delete it now.
				if(bIsLocal) {
					// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
					pDetail->__QuietRelease();
					//delete pDetail;
					pDetail = NULL;
				}
			}			
			pRow = pRow->GetNextRow();
		}

		// (j.jones 2008-10-30 14:39) - PLID 31869 - we now have to delete any EMNs we loaded
		int i=0;
		for(i=aryEMNs.GetSize() - 1; i>=0; i--) {
			CEMN *pEMN = (CEMN*)aryEMNs.GetAt(i);
			delete pEMN;
		}
		aryEMNs.RemoveAll();

		return 0;
	}
	// (a.walling 2007-09-13 14:50) - PLID 26762 - Use exception handling for threads
	NxCatchAllThread("Error populating detail values");
	return -1;
}*/

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemDeleteDlg dialog


CEmrProblemDeleteDlg::CEmrProblemDeleteDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrProblemDeleteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrProblemDeleteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strTopCaption = "The following is a list of problems that will be deleted as a result of EMR items, topics, charts, or other information being deleted.";
	m_strSecondCaption = "Please review this list, and confirm that Practice may delete these problems by pressing the OK button. If there are any problems you do not wish to be deleted, please press the Cancel button to abort saving your changes.";
	m_bCloseButtonOnly = FALSE;
	m_pPopulateDeletedProblemValuesThread = NULL;
}

// Tries to kill the active PopulateDeletedProblemValuesThread thread
void CEmrProblemDeleteDlg::KillThread()
{
	if (m_pPopulateDeletedProblemValuesThread) {
		// Wait for the thread to terminate
		WaitForSingleObject(m_pPopulateDeletedProblemValuesThread->m_hThread, 2000);
		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pPopulateDeletedProblemValuesThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself
			// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
			m_pPopulateDeletedProblemValuesThread->m_bAutoDelete = TRUE;

			PostThreadMessage(m_pPopulateDeletedProblemValuesThread->m_nThreadID, WM_QUIT, 0, 0);
			if (WAIT_TIMEOUT == WaitForSingleObject(m_pPopulateDeletedProblemValuesThread->m_hThread, 2000)) {
				TerminateThread(m_pPopulateDeletedProblemValuesThread->m_hThread, 0);
			}
		} else {
			// delete the thread object. We don't delete if it is running, which means autodelete will be set to true.
			delete m_pPopulateDeletedProblemValuesThread;
		}
		m_pPopulateDeletedProblemValuesThread = NULL;

		// (j.jones 2008-10-31 09:01) - PLID 31869 - clear out the stored thread ID
		if(GetMainFrame()) {
			GetMainFrame()->m_nProblemListDlg_PopulateDetailValues_CurThreadID = -1;
		}
		else {
			//should be impossible
			ASSERT(FALSE);
		}
	}
}

// (c.haag 2008-08-05 17:16) - PLID 30942 - Add problems that exist in memory
// (j.jones 2009-05-29 11:30) - PLID 34301 - renamed & reworked the logic to support the new structure
void CEmrProblemDeleteDlg::AddProblemLinksFromMemory()
{
	for (int i=0; i < m_apProblemLinksToBeDeleted.GetSize(); i++) {
		CEmrProblemLink* pProblemLink = m_apProblemLinksToBeDeleted[i];
		CEmrProblem* pProblem = pProblemLink->GetProblem();

		// The following code was copied from the EMR problem list dialog. There is one
		// difference: In this dialog, the status column is a combo column, not a text
		// column.
		CString strValue = "";
		CString strDetailName = "";
		long nDetailID = -1;
		long nEMNID = -1;
		EmrInfoType eDataType = eitInvalid;
		CString strTopicName, strEMNName, strEMRName;

		GetDetailNameAndValue(pProblemLink, strDetailName, strValue, strTopicName, strEMNName, strEMRName);

//#pragma TODO("PLID 33751, 28495 - Find a better way to keep track of strings for diagcodes, chronicity statuses")
		// (a.walling 2009-05-04 09:42) - PLID 33751, 28495 - Add diag code and chronicity
		CString strChronicityName = "<None>";
		if (pProblem->m_nChronicityID != -1) {
			strChronicityName = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", pProblem->m_nChronicityID), "<None>");
		}
		// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
		CString strICD9Name, strICD10Name;
		GetICD9And10Codes(pProblem->m_nDiagICD9CodeID, pProblem->m_nDiagICD10CodeID, strICD9Name, strICD10Name);
		if(strICD9Name.IsEmpty()) {
			strICD9Name = "<None>";
		}
		if(strICD10Name.IsEmpty()) {
			strICD10Name = "<None>";
		}

		IRowSettingsPtr pRow = m_dlList->GetNewRow();
		pRow->PutValue(plcProblemID, (long)pProblem->m_nID);
		pRow->PutValue(plcProblemLinkID, (long)pProblemLink->GetID());
		pRow->PutValue(plcDetailID, nDetailID);
		// (j.jones 2008-10-30 15:28) - PLID 31869 - added plcEMNID and plcDataType
		pRow->PutValue(plcEMNID, nEMNID);
		pRow->PutValue(plcDataType, (short)eDataType);
		COleDateTime dtEntered = pProblem->m_dtEnteredDate;
		dtEntered.SetDate(dtEntered.GetYear(), dtEntered.GetMonth(), dtEntered.GetDay());
		pRow->PutValue(plcEnteredDate, _variant_t(dtEntered, VT_DATE));
		COleDateTime dtOnset = pProblem->m_dtOnsetDate;
		if(dtOnset.GetStatus() != COleDateTime::invalid) {
			dtOnset.SetDate(dtOnset.GetYear(), dtOnset.GetMonth(), dtOnset.GetDay());
			pRow->PutValue(plcOnsetDate, _variant_t(dtOnset, VT_DATE));
		}
		pRow->PutValue(plcStatusID, (long)pProblem->m_nStatusID);
		pRow->PutValue(plcDiagCodeName_ICD9, (LPCTSTR)strICD9Name);
		pRow->PutValue(plcDiagCodeName_ICD10, (LPCTSTR)strICD10Name);
		pRow->PutValue(plcChronicityName, (LPCTSTR)strChronicityName);
		pRow->PutValue(plcDescription, (LPCTSTR)pProblem->m_strDescription);
		pRow->PutValue(plcDetailValue, (LPCTSTR)strValue);
		pRow->PutValue(plcDetailName, (LPCTSTR)strDetailName);
		pRow->PutValue(plcTopicName, (LPCTSTR)strTopicName);
		pRow->PutValue(plcEMNName, (LPCTSTR)strEMNName);
		pRow->PutValue(plcEMRName, (LPCTSTR)strEMRName);
		pRow->PutValue(plcProblemPtr, (long)pProblem);
		pRow->PutValue(plcProblemLinkPtr, (long)pProblemLink);

		// (j.jones 2009-05-29 11:48) - PLID 34301 - if the problem is in our deleted list,
		// color the row red
		for(int j=0; j < m_apProblemsToBeDeleted.GetSize(); j++) {
			if(pProblem == m_apProblemsToBeDeleted.GetAt(j)) {
				pRow->PutForeColor(RGB(255,0,0));
			}
		}

		m_dlList->AddRowSorted(pRow, NULL);
	}

	// (j.jones 2014-02-26 15:03) - PLID 61019 - resize the ICD-9 and 10 columns accordingly
	DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_dlList, plcDiagCodeName_ICD9, plcDiagCodeName_ICD10, 0, 0, "<None>", "<None>");
}


void CEmrProblemDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrProblemDeleteDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_CLOSE_PROBLEM_DELETE_DLG, m_btnClose);
	DDX_Control(pDX, IDC_NXC_PROBLEM_DELETE, m_nxcTop);
	DDX_Control(pDX, IDC_STATIC_TOP_CAPTION, m_staticTopCaption);
	DDX_Control(pDX, IDC_STATIC_SECOND_CAPTION, m_staticSecondCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrProblemDeleteDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrProblemDeleteDlg)
	ON_COMMAND(IDC_BTN_CLOSE_PROBLEM_DELETE_DLG, OnOK)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemDeleteDlg message handlers

BOOL CEmrProblemDeleteDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-08-05 17:18) - PLID 30942 - Set the top caption
		m_staticTopCaption.SetWindowText(m_strTopCaption);
		m_staticSecondCaption.SetWindowText(m_strSecondCaption);

		// Assign icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnClose.AutoSet(NXB_CLOSE);
	
		// Assign the dialog color
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_nxcTop.SetColor(EmrColors::Topic::PatientBackground());

		// Bind and requery the datalist
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_EMR_DELETING_PROBLEMS, false);
		// Assign the status column source. Note that we don't filter out inactive statii. That is on
		// purpose; we're not actually letting the user change problems from this dialog. If existing
		// problems still carry an inactive status, we need to show it.
		m_dlList->GetColumn(plcStatusID)->ComboSource = "SELECT ID, Name FROM EMRProblemStatusT";

		// (c.haag 2008-08-05 17:16) - PLID 30942 - If m_anProblemIDs is populated, we need
		// to run a query and fill the list with problems from data. If it's not, just fill
		// the list with any problems that exist in memory.
		// (j.jones 2009-05-29 12:44) - PLID 34301 - commented out unused code
		/*
		if (m_anProblemIDs.GetSize() > 0) {
			m_dlList->FromClause = _bstr_t(GetEmrProblemListFromClause());
			CString strWhere = FormatString("ID IN (%s)", ArrayAsString(m_anProblemIDs));
			m_dlList->WhereClause = _bstr_t(strWhere);
			m_dlList->Requery();
		}
		else {
		*/
			AddProblemLinksFromMemory();
			if (!m_bCloseButtonOnly) {
				GetDlgItem(IDOK)->EnableWindow(TRUE);
				GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
			}
		//}

		// (c.haag 2008-08-06 10:02) - PLID 30942 - Show the close button 
		if (m_bCloseButtonOnly) {
			GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
			GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_CLOSE_PROBLEM_DELETE_DLG)->ShowWindow(SW_SHOW);
		}
	}
	NxCatchAll("Error in CEmrProblemDeleteDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEmrProblemDeleteDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrProblemDeleteDlg)
	// (j.jones 2009-05-29 12:44) - PLID 34301 - commented out unused code
	//ON_EVENT(CEmrProblemDeleteDlg, IDC_LIST_EMR_DELETING_PROBLEMS, 18 /* RequeryFinished */, OnRequeryFinishedListEmrDeletingProblems, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (j.jones 2009-05-29 12:44) - PLID 34301 - commented out unused code
/*
void CEmrProblemDeleteDlg::OnRequeryFinishedListEmrDeletingProblems(short nFlags) 
{
	// (c.haag 2008-07-23 16:45) - Keep the OK and Cancel buttons disabled until after the requery is finished.
	// The user could still press the escape key to cancel, but all that does is roll back a save operation, so
	// no big deal.
	try {
		// (c.haag 2008-08-05 17:16) - PLID 30942 - At this point, we should add problems
		// that exist in memory to the list
		AddProblemLinksFromMemory();
		if (!m_bCloseButtonOnly) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
		}
		//
		// Populate the problem list with detail values
		//
		m_pPopulateDeletedProblemValuesThread = AfxBeginThread(PopulateDeletedProblemValuesThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

		// (j.jones 2008-10-31 09:00) - PLID 31869 - we need to track the thread ID
		if(GetMainFrame()) {
			//this should never already have an ID because this thread shouldn't be able to be called twice at the same time!
			ASSERT(GetMainFrame()->m_nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID == -1);
			GetMainFrame()->m_nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID = m_pPopulateDeletedProblemValuesThread->m_nThreadID;
		}
		else {
			//should be impossible
			ASSERT(FALSE);
		}

		m_pPopulateDeletedProblemValuesThread->m_bAutoDelete = false;
		m_pPopulateDeletedProblemValuesThread->ResumeThread();
	}
	NxCatchAll("Error in CEmrProblemDeleteDlg::OnRequeryFinishedListEmrDeletingProblems");
}
*/

void CEmrProblemDeleteDlg::OnDestroy() 
{
	try {
		KillThread();
		CNxDialog::OnDestroy();
	}
	NxCatchAll("Error in CEmrProblemDeleteDlg::OnDestroy()");
}