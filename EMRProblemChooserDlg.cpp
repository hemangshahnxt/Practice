// EMRProblemChooserDlg.cpp : implementation file
//
// (c.haag 2009-05-13 17:38) - PLID 34249 - Initial implementation
//

#include "stdafx.h"
#include "Practice.h"
#include "EMRProblemChooserDlg.h"
#include "EmrColors.h"
#include "DiagSearchUtils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ProblemListColumns {
	plcChecked = 0L,
	plcID,
	plcOwnedByCaller, // 1 if the problem is in memory, 0 if this dialog had to load it from data
	plcEnteredDate,
	plcOnsetDate,
	// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
	plcDiagCodeName_ICD9,
	plcDiagCodeName_ICD10,
	plcChronicityName,
	plcDescription,
	plcProblemPtr,
};

// CEMRProblemChooserDlg dialog

IMPLEMENT_DYNAMIC(CEMRProblemChooserDlg, CNxDialog)

CEMRProblemChooserDlg::CEMRProblemChooserDlg(
	const CArray<CEmrProblem*, CEmrProblem*>& apProblemsInMemory,
	const CArray<CEmrProblem*, CEmrProblem*>& apProblemsToExclude,
	long nPatientID,
	CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRProblemChooserDlg::IDD, pParent)
{
	CWaitCursor wc;
	m_nPatientID = nPatientID;

	// (c.haag 2009-05-27 11:13) - PLID 34249 - The caller gave us two arrays: All the problems
	// in memory, and all the problems currently bound to the EMR object this dialog is
	// invoked for.
	//
	// The first thing we do is fill in apAllProblems with all the problems in memory
	// that are not already assigned to the EMR object.
	//
	for (int i=0; i < apProblemsInMemory.GetSize(); i++) {
		CEmrProblem* p = apProblemsInMemory[i];
		BOOL bCopy = TRUE;
		for (int j=0; j < apProblemsToExclude.GetSize() && bCopy; j++) {
			if (apProblemsToExclude[j] == p) {
				bCopy = FALSE;
			}
		}
		if (bCopy) {
			m_apListProblemsInMemory.Add(p);
		}
	}

	// (c.haag 2009-05-27 11:29) - PLID 34249 - Now we need to query data for all the problems
	// for this patient that are not already in memory, and add them to the available problem
	// list. For this, we will need to create new CEmrProblem objects. This dialog will be responsible
	// for managing those.
	{
		CArray<long,long> anMemoryProblemIDs;
		int i;
		for (i=0; i < apProblemsInMemory.GetSize(); i++) {
			long nProblemID = apProblemsInMemory[i]->m_nID;
			if (nProblemID > -1) {
				BOOL bFound = FALSE;
				for (int j=0; j < anMemoryProblemIDs.GetSize() && !bFound; j++) {
					if (anMemoryProblemIDs[j] == nProblemID) {
						bFound = TRUE;
					}
				}
				if (!bFound) {
					anMemoryProblemIDs.Add(nProblemID);
				}
			}
		}

		// (z.manning 2009-05-27 14:09) - PLID 34297 - Added patient ID
		// (b.spivey, November 11, 2013) - PLID 58677 - Added CodeID
		// (j.jones 2014-02-24 17:15) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
		// (s.tullis 2015-02-23 15:44) - PLID 64723
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
		_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
			"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, "
			"EmrProblemsT.PatientID, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
			"FROM EMRProblemsT "
			"WHERE EMRProblemsT.PatientID = {INT} AND EMRProblemsT.ID NOT IN ({INTARRAY}) " 
			"AND Deleted = 0", m_nPatientID, anMemoryProblemIDs);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			CEmrProblem* p = new CEmrProblem(prs->Fields);
			m_apListProblemsInData.Add(p);
			prs->MoveNext();
		} // while (!prs->eof) {
	}
}

// (c.haag 2009-05-27 12:35) - PLID 34249 - Destruction and cleanup
CEMRProblemChooserDlg::~CEMRProblemChooserDlg()
{
	for (int i=0; i < m_apListProblemsInData.GetSize(); i++) {
		delete m_apListProblemsInData[i];
	}
}

// (c.haag 2009-05-27 12:35) - PLID 34249 - This is the main point of "entry" as far as
// invoking the dialog. Both parameters are outputs; one for selected problems that exist
// in memory (managed by the caller), and one for selected problems that were loaded from
// data by this dialog (for which we return EMR Problem ID's)
int CEMRProblemChooserDlg::Open(OUT CArray<CEmrProblem*, CEmrProblem*>& apSelectionsInMemory,
		OUT CArray<long,long>& anSelectionsInData)
{
	int nResult = DoModal();
	if (IDOK == nResult) {
		apSelectionsInMemory.Copy(m_apMemoryResults);
		anSelectionsInData.Copy(m_apDataResults);
	}
	return nResult;
}

// (c.haag 2009-05-27 12:35) - PLID 34249 - Returns true if there are any problems available to select from.
// This should be called after construction and before invoking the dialog.
BOOL CEMRProblemChooserDlg::HasProblems()
{
	if (m_apListProblemsInData.GetSize() > 0) {
		return TRUE;
	}
	else {
		for (int i=0; i < m_apListProblemsInMemory.GetSize(); i++) {
			if (!m_apListProblemsInMemory[i]->m_bIsDeleted) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

// (c.haag 2009-05-27 12:39) - PLID 34249 - Populates the list with problems
void CEMRProblemChooserDlg::RefilterList(CArray<CEmrProblem*, CEmrProblem*>& aryProblems, BOOL bOwnedByCaller)
{
	try {
		// Loop through every problem
		for(int i=0; i < aryProblems.GetSize(); i++) {
			CEmrProblem* pProblem = aryProblems[i];
			if (!pProblem->m_bIsDeleted) {
				ASSERT(pProblem != NULL);
				ASSERT(!pProblem->m_bIsDeleted);

				pProblem->m_dtEnteredDate.SetDateTime(pProblem->m_dtEnteredDate.GetYear(), pProblem->m_dtEnteredDate.GetMonth(), pProblem->m_dtEnteredDate.GetDay(), 0, 0, 0);

				_variant_t varOnsetDate = g_cvarNull;
				if(pProblem->m_dtOnsetDate.GetStatus() != COleDateTime::invalid) {
					pProblem->m_dtOnsetDate.SetDateTime(pProblem->m_dtOnsetDate.GetYear(), pProblem->m_dtOnsetDate.GetMonth(), pProblem->m_dtOnsetDate.GetDay(), 0, 0, 0);
					varOnsetDate = _variant_t(pProblem->m_dtOnsetDate, VT_DATE);
				}
				
				// (a.walling 2009-05-04 12:18) - PLID 33751, 28495 - This is awful, but we are pressed for time. PLID 34147 would implement
				// a combo which we could use instead of these queries if we don't optimize it before this item is done.
	//#pragma TODO("PLID 33751, 28495 - Find a better way to keep track of strings for diagcodes, chronicity statuses")
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

				IRowSettingsPtr pRow = m_dlProblems->GetNewRow();
				pRow->PutValue(plcChecked, _variant_t(VARIANT_FALSE, VT_BOOL));
				pRow->PutValue(plcID, (long)pProblem->m_nID);
				pRow->PutValue(plcOwnedByCaller, bOwnedByCaller ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
				pRow->PutValue(plcEnteredDate, _variant_t(pProblem->m_dtEnteredDate, VT_DATE));
				pRow->PutValue(plcOnsetDate, varOnsetDate);
				pRow->PutValue(plcDiagCodeName_ICD9, (LPCTSTR)strICD9Name);
				pRow->PutValue(plcDiagCodeName_ICD10, (LPCTSTR)strICD10Name);
				pRow->PutValue(plcChronicityName, (LPCTSTR)strChronicityName);
				pRow->PutValue(plcDescription, (LPCTSTR)pProblem->m_strDescription);
				pRow->PutValue(plcProblemPtr, (long)pProblem);
				m_dlProblems->AddRowSorted(pRow, NULL);
			} // if (!pProblem->m_bIsDeleted) {
		}

		// (j.jones 2014-02-26 15:03) - PLID 61019 - resize the ICD-9 and 10 columns accordingly
		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_dlProblems, plcDiagCodeName_ICD9, plcDiagCodeName_ICD10, 0, 0, "<None>", "<None>");
	}
	NxCatchAll("Error in RefilterList");
}

void CEMRProblemChooserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRProblemChooserDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NXC_PROBLEM_LIST, m_nxcTop);	
	//}}AFX_DATA_MAP
}

// CEMRProblemChooserDlg message handlers

BOOL CEMRProblemChooserDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// This dialog should not be opened if there are no problems to select from
		ASSERT(HasProblems());

		// (c.haag 2008-04-28 12:22) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Bind the problem list
		m_dlProblems = BindNxDataList2Ctrl(this, IDC_LIST_EMR_PROBLEMS, GetRemoteData(), false);

		//
		// Assign the dialog color
		//
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_nxcTop.SetColor(EmrColors::Topic::PatientBackground());

		//
		// Populate the problems list
		//		
		RefilterList(m_apListProblemsInMemory, TRUE);
		RefilterList(m_apListProblemsInData, FALSE);
	}
	NxCatchAll("Error initializing the EMR problem chooser dialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (c.haag 2009-05-27 09:11) - PLID 34249 - Called when the user clicks OK
void CEMRProblemChooserDlg::OnOK()
{
	try {
		// Go through the list and add all the selected problems
		IRowSettingsPtr pRow = m_dlProblems->GetFirstRow();
		BOOL bRowsSelected = FALSE;
		while (NULL != pRow) {
			BOOL bSelected = VarBool(pRow->Value[plcChecked]);
			if (bSelected) {
				BOOL bOwnedByCaller = VarBool(pRow->Value[plcOwnedByCaller]);
				if (bOwnedByCaller) {
					m_apMemoryResults.Add( (CEmrProblem*)VarLong(pRow->Value[plcProblemPtr]) );
				} else {
					m_apDataResults.Add( VarLong(pRow->Value[plcID]) );
				}
				bRowsSelected = TRUE;
			} else {
				// User did not select this row
			}
			pRow = pRow->GetNextRow();
		}

		if (!bRowsSelected) {
			// If we get here, the user clicked OK without selecting anything. Warn them about this.
			AfxMessageBox("Please check off at least one problem before clicking OK.");
		} else {
			CDialog::OnOK();
		}
	}
	NxCatchAll("CEMRProblemChooserDlg::OnBnClickedOk");
}
