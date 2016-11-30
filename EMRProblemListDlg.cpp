// EMRProblemListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRProblemListDlg.h"
#include "EMRProblemEditDlg.h"
#include "EMNDetail.h"
#include "AuditTrail.h"
#include "globalutils.h"
#include "PatientsRc.h"
#include "DateTimeUtils.h"
#include "EMRTopic.h"
#include "EMN.h"
#include "EMR.h"
#include "EmrItemAdvDlg.h"
#include "InternationalUtils.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "LabRequisitionDlg.h"
#include "WellnessDataUtils.h"
#include "DecisionRuleUtils.h"
#include "EmrColors.h"
#include "EmrUtils.h"
#include "DiagSearchUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

using namespace NXDATALIST2Lib;

// (j.jones 2008-07-18 15:38) - PLID 30773 - converted into an enum, and added a problem ptr column
// (j.jones 2008-10-30 15:28) - PLID 31869 - added plcEMNID and plcDataType
// (a.walling 2009-05-04 12:20) - PLID 28495, 33751 - Added diag code and chronicity
enum ProblemListColumns {
	plcID = 0,
	plcProblemLinkID, // (c.haag 2009-05-21 16:19) - PLID 34298
	plcDetailID,
	plcEMNID,
	plcDataType,
	plcEnteredDate,
	plcOnsetDate,
	plcStatusName,
	// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns
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
	plcProblemLinkPtr,	// (c.haag 2009-05-21 17:15) - PLID 34298
};

// (j.jones 2014-02-26 14:32) - PLID 60764 - added enum for the history list
enum HistoryListColumns {
	hlcStatusID = 0,
	hlcDate,
	hlcDescription,
	hlcUsername,
	hlcStatusName,
	hlcDiagCodeName_ICD9,
	hlcDiagCodeName_ICD10,
	hlcChronicityName,
};

// (j.jones 2008-07-15 15:33) - PLID 30731 - added enums for the filters
// (j.jones 2008-11-20 17:52) - PLID 28497 - added Resolved column
enum StatusComboColumn {

	sccID = 0,
	sccName,
	sccResolved,
};

enum TypeComboColumn {

	tccID = 0,
	tccName,
};

enum DateComboColumn {

	dccID = 0,
	dccName,
};

enum DateFilterTypes {

	dftAll = 0,
	dftEntered,
	dftModified,
};

// (j.jones 2008-11-20 17:32) - PLID 28497 - added enum for "special" problem status filters
enum ProblemStatusSpecialFilters {
	pssfAll = -1,
	pssfResolved = -2,
	pssfUnresolved = -3,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

UINT PopulateDetailValuesThread(LPVOID p)
{
	try {
		// (c.haag 2009-02-09 15:17) - PLID 32976 - We now take in a list of data values rather than reading
		// from the datalist.
		CArray<CEMRProblemListDlg::ProblemListThreadValue*,CEMRProblemListDlg::ProblemListThreadValue*>* papPLTV = 
			(CArray<CEMRProblemListDlg::ProblemListThreadValue*,CEMRProblemListDlg::ProblemListThreadValue*>*)p;

		//
		// Open a new connection based on the existing one
		//
		// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
		ADODB::_ConnectionPtr pCon = GetThreadRemoteData();

		//
		// Go through each row and fill in the detail value column,
		// for detail problems (and list item problems) only
		//

		CArray<CEMN*, CEMN*> aryEMNs;

		for (int nRow=0; nRow < papPLTV->GetSize(); nRow++) {
			// Load the detail
			// (c.haag 2009-02-09 15:43) - PLID 32976 - Load from the array above instead of the datalist directly
			CEMRProblemListDlg::ProblemListThreadValue* pPLTV = papPLTV->GetAt(nRow);
			long nDetailID = pPLTV->nDetailID;

			// (c.haag 2009-02-09 15:39) - PLID 32976 - Don't need to check for this anymore; all
			// detail ID's are not -1
			//if(nDetailID != -1) {

				// (j.jones 2008-10-30 14:56) - PLID 31869 - If we have a Narrative or a Table,
				// we need to load the entire EMN, otherwise our results won't be reliable
				// (narratives won't show their linked items, tables won't show linked details).
				// (c.haag 2009-02-09 15:43) - PLID 32976 - Load from the array above instead of the datalist directly
				long nEMNID = pPLTV->nEMNID;
				EmrInfoType eDataType = pPLTV->DataType;
				
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
					// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
					pEMN = new CEMN(NULL, pCon);
					pEMN->LoadFromEmnID(nEMNID, pCon);
					aryEMNs.Add(pEMN);
				}

				CEMNDetail *pDetail = NULL;
				// (j.jones 2008-10-30 14:55) - PLID 31869 - if we have an EMN, just find the already-loaded detail in it
				if(pEMN) {
					pDetail = pEMN->GetDetailByID(nDetailID);
				}

				//if we have no detail, load it now, which is way faster
				//than always loading the EMN
				BOOL bIsLocal = FALSE;
				if(pDetail == NULL) {
					// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
					pDetail = CEMNDetail::CreateDetail(NULL, "EMR problem list local detail");
					// Load the detail
					// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
					pDetail->LoadFromDetailID(nDetailID, pCon);
					bIsLocal = TRUE;
				}

				CString strSentence;
				// Get the detail value in sentence form
				// (c.haag 2006-07-03 15:44) - Copied from CEmrItemAdvDlg::GetToolTipText()
				// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
				if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
					//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamp" long form.  Note that for the time being, it doesn't
					// actually matter, as both formats are always blank.  However, I entered 37558 to resolve that, which will either make
					// sure the variables are set here, or just replace them with an explicit empty string.
					// (z.manning 2010-07-26 15:22) - PLID 39848 - All tables now use the regular long form
					strSentence = pDetail->m_strLongForm;
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
				// (c.haag 2009-02-09 15:43) - PLID 32976 - Instead of writing to the list directly, have the dialog do it for us
				pPLTV->strDetailValue = strSentence;
				if (IsWindow(pPLTV->pDlg->GetSafeHwnd())) {
					pPLTV->pDlg->PostMessage(NXM_EMR_PROBLEM_LIST_DETAIL, (WPARAM)pPLTV);
					// The dialog will delete this object
				} else {
					// This should never happen, but have a backup plan
					delete pPLTV;
				}
				
				// (j.jones 2008-10-30 15:42) - PLID 31869 - We don't need to delete detail if it was from the EMN,
				// because it would be handled when the EMN is deleted. But we may have created this detail on our
				//own, and if so, we need to delete it now.
				if(bIsLocal) {
					// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
					pDetail->__QuietRelease();
					//delete pDetail;
					pDetail = NULL;
				}

			//} // if(nDetailID != -1) {
		} // for (int nRow=0; nRow < papPLTV->GetSize(); nRow++) {

		// (j.jones 2008-10-30 15:42) - PLID 31869 - we now have to delete any EMNs we loaded
		int i=0;
		for(i=aryEMNs.GetSize() - 1; i>=0; i--) {
			CEMN *pEMN = (CEMN*)aryEMNs.GetAt(i);
			delete pEMN;
		}
		aryEMNs.RemoveAll();

		// (c.haag 2009-02-09 15:48) - PLID 32976 - Cleanup
		delete papPLTV;

		return 0;
	}
	// (a.walling 2007-09-13 14:50) - PLID 26762 - Use exception handling for threads
	NxCatchAllThread("Error populating detail values");
	return -1;
}


/////////////////////////////////////////////////////////////////////////////
// CEMRProblemListDlg dialog

// (j.jones 2008-11-20 16:29) - PLID 32119 - added problem status tablechecker
CEMRProblemListDlg::CEMRProblemListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRProblemListDlg::IDD, pParent),
	m_ProblemStatusChecker(NetUtils::EMRProblemStatusT)
{
	//{{AFX_DATA_INIT(CEMRProblemListDlg)
	m_bShowProblemDesc = FALSE;
	m_bShowDetailData = FALSE;
	m_bShowDetailName = FALSE;
	m_bShowEMNName = FALSE;
	m_bShowEMRName = FALSE;
	m_bShowTopicName = FALSE;
	m_bShowDiagCode = FALSE;
	m_bShowChronicity = FALSE;
	m_nPatientID = -1;
	m_eprtDefaultRegardingType = eprtInvalid;
	m_nDefaultRegardingID = -1;
	// (c.haag 2009-05-21 13:17) - PLID 34298 - I changed the old problem array to a problem link array. This
	// change applies throughout the dialog
	m_papEMRProblemLinks = NULL;
	m_bIsOwnedByMainframe = FALSE;
	m_pMessageWnd = NULL;
	m_bIsRefilteringFromMemory = FALSE;
	//}}AFX_DATA_INIT
	m_pPopulateDetailValuesThread = NULL;	
	m_pdlgLabRequisition = NULL; //TES 11/25/2009 - PLID 36191
	m_bExcludeCurrentUserFromEmnConcurrencyCheck = TRUE; // (z.manning 2009-07-01 16:01) - PLID 34765
	m_bIncludeEmrLevelProblemsInConcurrencyCheck = FALSE;
}


void CEMRProblemListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRProblemListDlg)
	DDX_Control(pDX, IDC_PROBLEM_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_PROBLEM_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_CHECK_SHOW_EMR_NAME, m_btnShowEMRName);
	DDX_Control(pDX, IDC_CHECK_SHOW_EMN_NAME, m_btnShowEMNName);
	DDX_Control(pDX, IDC_CHECK_SHOW_TOPIC_NAME, m_btnShowTopicName);
	DDX_Control(pDX, IDC_CHECK_SHOW_DETAIL_NAME, m_btnShowDetailName);
	DDX_Control(pDX, IDC_CHECK_SHOW_DETAIL_DATA, m_btnShowDetailValue);
	DDX_Control(pDX, IDC_CHECK_SHOW_PROBLEM_DESC, m_btnShowProblemDesc);
	DDX_Control(pDX, IDC_CHECK_SHOW_DIAGNOSIS, m_btnShowDiagCode);
	DDX_Control(pDX, IDC_CHECK_SHOW_CHRONICITY, m_btnShowChronicity); // (a.walling 2009-05-04 12:16) - PLID 33751
	DDX_Check(pDX, IDC_CHECK_SHOW_PROBLEM_DESC, m_bShowProblemDesc);
	DDX_Check(pDX, IDC_CHECK_SHOW_DETAIL_DATA, m_bShowDetailData);
	DDX_Check(pDX, IDC_CHECK_SHOW_DETAIL_NAME, m_bShowDetailName);
	DDX_Check(pDX, IDC_CHECK_SHOW_EMN_NAME, m_bShowEMNName);
	DDX_Check(pDX, IDC_CHECK_SHOW_EMR_NAME, m_bShowEMRName);
	DDX_Check(pDX, IDC_CHECK_SHOW_TOPIC_NAME, m_bShowTopicName);
	DDX_Check(pDX, IDC_CHECK_SHOW_DIAGNOSIS, m_bShowDiagCode);
	DDX_Check(pDX, IDC_CHECK_SHOW_CHRONICITY, m_bShowChronicity); // (a.walling 2009-05-04 12:16) - PLID 33751
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_NEW_PROBLEM, m_btnNewProblem);
	DDX_Control(pDX, IDC_BTN_EDIT_PROBLEM, m_btnEditProblem);
	DDX_Control(pDX, IDC_BTN_DELETE_PROBLEM, m_btnDeleteProblem);
	DDX_Control(pDX, IDC_BTN_PREVIEW_PROBLEM_LIST, m_btnPrintPreview);
	DDX_Control(pDX, IDC_NXC_PROBLEM_LIST, m_nxcTop);	
	//}}AFX_DATA_MAP
}

// (j.jones 2008-07-17 16:18) - PLID 30731 - renamed SetPatientID to SetDefaultFilter,
// so that we can fill in m_eprtDefaultRegardingType and m_nDefaultRegardingID
// (e.lally 2008-07-30) Please note that the default regarding ID is not used when the Problem List is opened from withint the EMR.
// It can be ignored entirely in those cases.
void CEMRProblemListDlg::SetDefaultFilter(long nPatientID, EMRProblemRegardingTypes eprtDefaultRegardingType /*= eprtInvalid*/, long nDefaultRegardingID /*= -1*/, CString strRegardingDesc /*= ""*/)
{
	m_nPatientID = nPatientID;
	m_eprtDefaultRegardingType = eprtDefaultRegardingType;
	m_nDefaultRegardingID = nDefaultRegardingID;
	m_strRegardingDesc = strRegardingDesc;
}

// (j.jones 2008-07-18 14:35) - PLID 30773 - added ability to pass in an array of problem pointers
// (c.haag 2009-05-21 12:40) - PLID 34298 - We now pass in problem links
void CEMRProblemListDlg::LoadFromProblemList(CWnd* pMessageWnd, CArray<CEmrProblemLink*, CEmrProblemLink*> *papEMRProblemLinks)
{
	//it is assumed that the caller will include only the problems that are on
	//or underneath the EMR object that calls this dialog
	//(example, calling from an EMR Topic will only include problems on that Topic
	//or any of its Details and List Items)

	m_papEMRProblemLinks = papEMRProblemLinks;

	//pMessageWnd is the window that will receive messages if problems change
	m_pMessageWnd = pMessageWnd;
}

long CEMRProblemListDlg::GetPatientID()
{
	return m_nPatientID;
}

EMRProblemRegardingTypes CEMRProblemListDlg::GetRegardingType()
{
	return m_eprtDefaultRegardingType;
}

void CEMRProblemListDlg::RequeryHistory(long nProblemID)
{
	try {

		// (j.jones 2008-07-21 14:55) - PLID 30773 - if the problem is new,
		// there is no history yet, so we shouldn't display anything
		if(nProblemID < 0) {
			m_dlHistory->Clear();
			UpdateHistoryListDiagnosisColumns();
			return;
		}

		CString strWhere;
		strWhere.Format("ProblemID = %d", nProblemID);
		m_dlHistory->WhereClause = (LPCTSTR)strWhere;
		m_dlHistory->Requery();
	}
	NxCatchAll("Error requerying problem history");
}

void CEMRProblemListDlg::ResizeColumns()
{
	try {
		
		//
		// Diag Code // (a.walling 2009-05-04 12:19) - PLID 28495		
		//
		{
			// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns,
			// but this setting affects both of them
			IColumnSettingsPtr pCol9 = m_dlProblems->GetColumn(plcDiagCodeName_ICD9);
			IColumnSettingsPtr pCol10 = m_dlProblems->GetColumn(plcDiagCodeName_ICD10);
			bool bShowDiags = (GetRemotePropertyInt("EMRProblemListColumns", 1, 6, GetCurrentUserName()) != 0);
			if(bShowDiags) {
				UpdateProblemListDiagnosisColumns();
			} else {
				//hide the columns, but allow them to be resizeable
				pCol9->ColumnStyle = csVisible;
				pCol9->PutStoredWidth(0);
				pCol10->ColumnStyle = csVisible;
				pCol10->PutStoredWidth(0);
			}
		}
		
		IColumnSettingsPtr pCol;
		//
		// Chronicity // (a.walling 2009-05-04 12:19) - PLID 33751
		//
		pCol = m_dlProblems->GetColumn(plcChronicityName);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 7, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
		//
		// Problem description
		//
		pCol = m_dlProblems->GetColumn(plcDescription);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 0, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
		//
		// Detail data
		//
		pCol = m_dlProblems->GetColumn(plcDetailValue);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 1, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}		
		//
		// Detail name
		//
		pCol = m_dlProblems->GetColumn(plcDetailName);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 2, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
		//
		// Topic name
		//
		pCol = m_dlProblems->GetColumn(plcTopicName);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 3, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
		//
		// EMN name
		//
		pCol = m_dlProblems->GetColumn(plcEMNName);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 4, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
		//
		// EMR name
		//
		pCol = m_dlProblems->GetColumn(plcEMRName);
		if (GetRemotePropertyInt("EMRProblemListColumns", 1, 5, GetCurrentUserName())) {
			pCol->ColumnStyle |= csWidthAuto;
		} else {
			pCol->ColumnStyle &= ~csWidthAuto;
		}
	} NxCatchAll("Error in ResizeColumns");
}

BEGIN_MESSAGE_MAP(CEMRProblemListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRProblemListDlg)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PROBLEM_DESC, OnCheckShowProblemDesc)
	ON_BN_CLICKED(IDC_CHECK_SHOW_DETAIL_DATA, OnCheckShowDetailData)
	ON_BN_CLICKED(IDC_CHECK_SHOW_DETAIL_NAME, OnCheckShowDetailName)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TOPIC_NAME, OnCheckShowTopicName)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMN_NAME, OnCheckShowEmnName)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMR_NAME, OnCheckShowEmrName)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_NEW_PROBLEM, OnBtnNewProblem)
	ON_BN_CLICKED(IDC_BTN_EDIT_PROBLEM, OnBtnEditProblem)
	ON_BN_CLICKED(IDC_BTN_DELETE_PROBLEM, OnBtnDeleteProblem)
	ON_NOTIFY(DTN_CLOSEUP, IDC_PROBLEM_DATE_FROM, OnCloseupProblemFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_PROBLEM_DATE_TO, OnCloseupProblemToDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PROBLEM_DATE_FROM, OnDatetimechangeProblemFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PROBLEM_DATE_TO, OnDatetimechangeProblemToDate)
	ON_BN_CLICKED(IDC_BTN_PREVIEW_PROBLEM_LIST, OnPrintPreview)
	ON_MESSAGE(NXM_EMR_PROBLEM_LIST_DETAIL, OnEMRProblemListDetailFromThread)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_SHOW_DIAGNOSIS, OnBnClickedCheckShowDiagnosis)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CHRONICITY, OnBnClickedCheckShowChronicity)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemListDlg message handlers

BOOL CEMRProblemListDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-28 12:22) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnNewProblem.AutoSet(NXB_NEW); // (c.haag 2008-11-26 12:28) - PLID 28496
		m_btnEditProblem.AutoSet(NXB_MODIFY);
		m_btnDeleteProblem.AutoSet(NXB_DELETE);
		//(e.lally 2008-07-23) PLID 30732 - Add print preview button
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);

		// (c.haag 2008-12-15 17:42) - PLID 28496 - If the following is TRUE, that means the problem list is in memory, meaning we must
		// be inside an EMR as opposed to being on the NexEMR tab. Since we're inside an EMR, the idea of adding a "New (EMR-independent) Problem"
		// doesn't make sense. It will just confuse everyone into thinking it's an EMR problem. It would be slick if we could just have the button
		// add a new EMR problem instead, but we don't always track the EMR object and data. This list may have been invoked from viewing
		// problems via detail, or medication; meaning "New Problem" could mean many things. So, just avert all confusion by hiding
		// the button and rearranging the other buttons so there's not a block of empty space on the corner.
		if (m_papEMRProblemLinks) {
			CRect rc;
			int nOKWidth, nEditWidth, nDeleteWidth, nPrintWidth;
			int nMinX, nMaxX;
			int Y;
			// Gather measurements
			m_btnNewProblem.GetWindowRect(rc); ScreenToClient(rc); nMinX = rc.left; Y = rc.top;
			m_btnEditProblem.GetClientRect(rc); nEditWidth = rc.Width();
			m_btnDeleteProblem.GetClientRect(rc); nDeleteWidth = rc.Width();
			m_btnOK.GetClientRect(rc); nOKWidth = rc.Width();
			m_btnPrintPreview.GetWindowRect(rc); ScreenToClient(rc); nPrintWidth = rc.Width();
			nMaxX = rc.right;
			// Calculate the horizontal space between buttons
			int nCombinedBtnWidths = nOKWidth + nEditWidth + nDeleteWidth + nPrintWidth;
			int nTotalAreaWidth = nMaxX - nMinX;
			int nHSpace = (nTotalAreaWidth - nCombinedBtnWidths) / 3; // 4 buttons, 3 gaps in between
			// Reposition the buttons
			m_btnEditProblem.SetWindowPos(NULL, nMinX, Y, 0,0, SWP_NOZORDER | SWP_NOSIZE);
			m_btnDeleteProblem.SetWindowPos(NULL, nMinX + nEditWidth + nHSpace, Y, 0,0, SWP_NOZORDER | SWP_NOSIZE);
			m_btnOK.SetWindowPos(NULL, nMinX + nEditWidth + nHSpace + nDeleteWidth + nHSpace, Y, 0,0, SWP_NOZORDER | SWP_NOSIZE); 
			m_btnPrintPreview.SetWindowPos(NULL, nMinX + nEditWidth + nHSpace + nDeleteWidth + nHSpace + nOKWidth + nHSpace, Y, 0,0, SWP_NOZORDER | SWP_NOSIZE);
			// Hide the "New Problem" button
			m_btnNewProblem.ShowWindow(SW_HIDE);
		}

		// (j.jones 2008-07-15 14:35) - PLID 30731 - added date controls
		m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		//used to allow being minimized to the taskbar
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		if(m_nPatientID == -1) {
			//we should have always been given a patient ID
			ASSERT(FALSE);

			m_nPatientID = GetActivePatientID();
		}

		m_dlProblems = BindNxDataList2Ctrl(this, IDC_LIST_EMR_PROBLEMS, GetRemoteData(), false);
		m_dlHistory = BindNxDataList2Ctrl(this, IDC_LIST_EMR_PROBLEM_HISTORY, GetRemoteData(), false);

		// (c.haag 2008-07-24 08:43) - PLID 30820 - Moved the From clause to a global utility function
		CString strFrom = GetEmrProblemListFromClause();
		m_dlProblems->PutFromClause(_bstr_t(strFrom));

		// (j.jones 2008-07-15 14:34) - PLID 30731 - added filters
		m_StatusCombo = BindNxDataList2Ctrl(this, IDC_PROBLEM_STATUS_FILTER, GetRemoteData(), true);
		m_TypeCombo = BindNxDataList2Ctrl(this, IDC_PROBLEM_OBJECT_TYPE_FILTER, GetRemoteData(), false);
		m_DateCombo = BindNxDataList2Ctrl(this, IDC_PROBLEM_DATE_FILTER, GetRemoteData(), false);

		// (a.walling 2008-07-16 17:45) - PLID 30586 - Ensure this is cached
		CString strCurrentLocationLogo = GetCurrentLocationLogo();

		{
			IRowSettingsPtr pAllRow = m_StatusCombo->GetNewRow();
			pAllRow->PutValue(sccID, (long)pssfAll);
			pAllRow->PutValue(sccName, _bstr_t(" < Show All >"));
			pAllRow->PutValue(sccResolved, g_cvarNull);
			m_StatusCombo->AddRowSorted(pAllRow, NULL);

			// (j.jones 2008-11-20 17:37) - PLID 28497 - added options for Unresolved/Resolved
			IRowSettingsPtr pResolvedRow = m_StatusCombo->GetNewRow();
			pResolvedRow->PutValue(sccID, (long)pssfResolved);
			pResolvedRow->PutValue(sccName, _bstr_t(" < Show Resolved >"));
			pResolvedRow->PutValue(sccResolved, g_cvarNull);
			m_StatusCombo->AddRowSorted(pResolvedRow, NULL);

			IRowSettingsPtr pUnresolvedRow = m_StatusCombo->GetNewRow();
			pUnresolvedRow->PutValue(sccID, (long)pssfUnresolved);
			pUnresolvedRow->PutValue(sccName, _bstr_t(" < Show Unresolved >"));
			pUnresolvedRow->PutValue(sccResolved, g_cvarNull);
			m_StatusCombo->AddRowSorted(pUnresolvedRow, NULL);

			m_StatusCombo->PutCurSel(pAllRow);
		}

		{
			//call BuildTypeCombo to populate m_TypeCombo
			BuildTypeCombo();

			//select all by default
			m_TypeCombo->SetSelByColumn(tccID, (long)eprtInvalid);
		}

		{
			IRowSettingsPtr pRow = m_DateCombo->GetNewRow();
			pRow->PutValue(dccID, (long)dftAll);
			pRow->PutValue(dccName, _bstr_t(" < All Dates >"));
			m_DateCombo->AddRowAtEnd(pRow, NULL);

			m_DateCombo->PutCurSel(pRow);

			pRow = m_DateCombo->GetNewRow();
			pRow->PutValue(dccID, (long)dftEntered);
			pRow->PutValue(dccName, _bstr_t("Entered Date"));
			m_DateCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_DateCombo->GetNewRow();
			pRow->PutValue(dccID, (long)dftModified);
			pRow->PutValue(dccName, _bstr_t("Modified Date"));
			m_DateCombo->AddRowAtEnd(pRow, NULL);
		}

		//disable the date range by default
		GetDlgItem(IDC_PROBLEM_DATE_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROBLEM_DATE_TO)->EnableWindow(FALSE);

		//
		// Assign the dialog color
		//
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_nxcTop.SetColor(EmrColors::Topic::PatientBackground());
		//
		// Populate the problems list
		
		// (j.jones 2008-07-16 10:43) - PLID 30731 - moved the where clause and requery to RefilterList();
		RefilterList(TRUE);

		//
		// Set the dialog title
		//
		UpdateProblemListWindowText();
		
		//
		// Update the checkboxes
		//
		m_bShowProblemDesc = GetRemotePropertyInt("EMRProblemListColumns", 1, 0, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowDetailData = GetRemotePropertyInt("EMRProblemListColumns", 1, 1, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowDetailName = GetRemotePropertyInt("EMRProblemListColumns", 1, 2, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowTopicName = GetRemotePropertyInt("EMRProblemListColumns", 1, 3, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowEMNName = GetRemotePropertyInt("EMRProblemListColumns", 1, 4, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowEMRName = GetRemotePropertyInt("EMRProblemListColumns", 1, 5, GetCurrentUserName()) ? TRUE : FALSE;		
		// (a.walling 2009-05-04 12:18) - PLID 33751, 28495
		// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns, but this setting affects both of them
		m_bShowDiagCode = GetRemotePropertyInt("EMRProblemListColumns", 1, 6, GetCurrentUserName()) ? TRUE : FALSE;
		m_bShowChronicity = GetRemotePropertyInt("EMRProblemListColumns", 1, 7, GetCurrentUserName()) ? TRUE : FALSE;		
		UpdateData(FALSE);
		//
		// Set button text colors
		// (c.haag 2008-04-28 13:02) - PLID 29806 - Don't use the text colors anymore; we now
		// have a new standard which instead uses icons
		//
		//m_btnDeleteProblem.SetTextColor(RGB(255,0,0));
		//m_btnEditProblem.SetTextColor(RGB(0,127,0));
		//
		// Resize the columns
		//
		ResizeColumns();

		UpdateHistoryListDiagnosisColumns();

		//TES 9/2/2011 - PLID 37633 - We need to make sure that the global narrative dialog used when loading details has been created, because
		// if it hasn't, then maybe our thread will create it, and we don't want our thread creating windows.
		EnsureGlobalEmrItemAdvNarrativeDlg(egiLoading);
	}
	NxCatchAll("Error initializing the EMR problem list dialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.jones 2008-07-17 17:29) - PLID 30731 - dynamically builds the Type combo based on m_eprtDefaultRegardingType
void CEMRProblemListDlg::BuildTypeCombo()
{
	try {

		//depending on m_eprtDefaultRegardingType, the "show all" text will change,
		//some options may be hidden, and the combo may even be disabled

		CString strSelectAllText = " < Show All >";

		BOOL bEnabled = TRUE;

		if(m_eprtDefaultRegardingType == eprtEmrItem) {
			strSelectAllText = " < Show All for this Item>";
		}
		else if(m_eprtDefaultRegardingType == eprtEmrDataItem) {
			strSelectAllText = " < Show All for this List Item>";

			//we'll disable the combo because there are no children to filter on
			bEnabled = FALSE;
		}
		else if(m_eprtDefaultRegardingType == eprtEmrTopic) {
			strSelectAllText = " < Show All for this Topic>";
		}
		else if(m_eprtDefaultRegardingType == eprtEmrEMN) {
			strSelectAllText = " < Show All for this EMN>";
		}
		else if(m_eprtDefaultRegardingType == eprtEmrEMR) {
			strSelectAllText = " < Show All for this EMR>";
		}
		else if(m_eprtDefaultRegardingType == eprtEmrDiag) {
			strSelectAllText = " < Show All for this Diagnosis>";

			//we'll disable the combo because there are no children to filter on
			bEnabled = FALSE;
		}
		else if(m_eprtDefaultRegardingType == eprtEmrCharge) {
			strSelectAllText = " < Show All for this Charge>";

			//we'll disable the combo because there are no children to filter on
			bEnabled = FALSE;
		}
		else if(m_eprtDefaultRegardingType == eprtEmrMedication) {
			strSelectAllText = " < Show All for this Medication>";

			//we'll disable the combo because there are no children to filter on
			bEnabled = FALSE;
		}
		else if(m_eprtDefaultRegardingType == eprtLab) {
			// (z.manning 2009-05-28 09:26) - PLID 34345 - Labs can now have problems
			strSelectAllText = " < Show All for this Lab>";
			bEnabled = FALSE;
		}

		//add the "all" row at all time
		IRowSettingsPtr pRow = m_TypeCombo->GetNewRow();
		pRow->PutValue(tccID, (long)eprtInvalid);
		pRow->PutValue(tccName, _bstr_t(strSelectAllText));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);

		//if m_eprtDefaultRegardingType == eprtInvalid, then this list should reflect all EMRProblemRegardingTypes,
		//otherwise we will only show the m_eprtDefaultRegardingType and any child types

		// (c.haag 2008-12-05 09:48) - PLID 28496 - The (Unassigned) filter is valid if filtering
		// on all problems not associated with any EMR kind of record
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtUnassigned) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtUnassigned);
			pRow->PutValue(tccName, _bstr_t(" < Show EMR-Independent Problems >"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Detail filter is valid if filtering on all, or Items, Topics, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrItem
			|| m_eprtDefaultRegardingType == eprtEmrTopic
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrItem);
			pRow->PutValue(tccName, _bstr_t("EMR Items"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Detail filter is valid if filtering on all, or Data Items, Items, Topics, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrDataItem
			|| m_eprtDefaultRegardingType == eprtEmrItem
			|| m_eprtDefaultRegardingType == eprtEmrTopic
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrDataItem);
			pRow->PutValue(tccName, _bstr_t("EMR List Items"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Topic filter is valid if filtering on all, or Topics, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrTopic
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrTopic);
			pRow->PutValue(tccName, _bstr_t("EMR Topics"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the EMN filter is valid if filtering on all, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrEMN);
			pRow->PutValue(tccName, _bstr_t("EMNs"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the EMN filter is valid if filtering on all, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrEMR);
			pRow->PutValue(tccName, _bstr_t("EMRs"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Diagnosis filter is valid if filtering on all, or Diagnoses, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrDiag
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrDiag);
			pRow->PutValue(tccName, _bstr_t("EMR Diagnosis Codes"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Charges filter is valid if filtering on all, or Charges, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrCharge
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrCharge);
			pRow->PutValue(tccName, _bstr_t("EMR Charges"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		//the Charges filter is valid if filtering on all, or Medications, EMNs, or EMRs
		if(m_eprtDefaultRegardingType == eprtInvalid
			|| m_eprtDefaultRegardingType == eprtEmrMedication
			|| m_eprtDefaultRegardingType == eprtEmrEMN
			|| m_eprtDefaultRegardingType == eprtEmrEMR) {

			pRow = m_TypeCombo->GetNewRow();
			pRow->PutValue(tccID, (long)eprtEmrMedication);
			pRow->PutValue(tccName, _bstr_t("EMR Medications"));
			m_TypeCombo->AddRowAtEnd(pRow, NULL);
		}

		// (z.manning 2009-05-28 09:28) - PLID 34345 - Labs
		if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent))
		{
			if(m_eprtDefaultRegardingType == eprtInvalid
				|| m_eprtDefaultRegardingType == eprtLab)
			{
				pRow = m_TypeCombo->GetNewRow();
				pRow->PutValue(tccID, (long)eprtLab);
				pRow->PutValue(tccName, _bstr_t("Labs"));
				m_TypeCombo->AddRowAtEnd(pRow, NULL);
			}
		}

		//now disable the combo if a type was set to do so
		m_TypeCombo->Enabled = bEnabled;

	}NxCatchAll("Error in CEMRProblemListDlg::BuildTypeCombo");
}

// (j.jones 2008-07-16 10:43) - PLID 30731 - This function will filter the list accordingly and requery.
// bForceRequery is set to TRUE if the caller wants the list to requery even if the filter does not change.
void CEMRProblemListDlg::RefilterList(BOOL bForceRequery /*= FALSE*/)
{
	try {

		// (j.jones 2008-11-20 17:06) - PLID 32119 - added a problem status tablechecker,
		// used predominantly in modal dialogs, because if this window is modeless it
		// will be updated in realtime, and this member will likely never remain marked
		// as Changed() for long
		if(m_ProblemStatusChecker.Changed()) {
				
			RequeryStatusFilter(FALSE);
		}

		// (j.jones 2008-07-18 14:40) - PLID 30773 - call RefilterListFromMemory()
		// if we have a list of problem pointers, otherwise call RefilterListFromSql
		if(m_papEMRProblemLinks) {
			RefilterListFromMemory();
		}
		else {
			RefilterListFromSql(bForceRequery);
		}

	}NxCatchAll("Error in CEMRProblemListDlg::RefilterList");
}

// (j.jones 2008-07-18 14:42) - PLID 30773 - reload the list, exclusively
// from the m_papEMRProblemLinks array
void CEMRProblemListDlg::RefilterListFromMemory()
{
	try {

		if(m_papEMRProblemLinks == NULL) {
			ThrowNxException("RefilterListFromMemory called when m_papEMRProblemLinks is NULL!");
		}

		// (j.jones 2008-08-20 10:39) - PLID 30773 - added boolean so loading from memory doesn't load twice
		if(m_bIsRefilteringFromMemory) {
			return;
		}

		m_bIsRefilteringFromMemory = TRUE;

		m_dlHistory->Clear();
		UpdateHistoryListDiagnosisColumns();

		// (c.haag 2009-02-09 15:06) - PLID 32976 - Calling Clear on the problem list is now done in
		// a utility function
		ClearProblemList();

		if(m_papEMRProblemLinks->GetSize() == 0) {
			//no point in doing any work if we have no problems!
			m_bIsRefilteringFromMemory = FALSE;
			return;
		}

		//first grab our filter information

		//get the status filter
		long nStatusID = (long)pssfAll;
		CString strStatusFilterName = "";
		{
			IRowSettingsPtr pRow = m_StatusCombo->GetCurSel();
			if(pRow) {
				nStatusID = VarLong(pRow->GetValue(sccID), (long)pssfAll);
				strStatusFilterName = VarString(pRow->GetValue(sccName), "");
			}
		}

		//get the type filter
		EMRProblemRegardingTypes eprtTypeID = eprtInvalid;
		{
			IRowSettingsPtr pRow = m_TypeCombo->GetCurSel();
			if(pRow) {
				eprtTypeID = (EMRProblemRegardingTypes)VarLong(pRow->GetValue(tccID), eprtInvalid);
			}
		}

		//get the date filter
		DateFilterTypes dftDateID = dftAll;
		COleDateTime dtFrom = COleDateTime::GetCurrentTime();
		COleDateTime dtTo = COleDateTime::GetCurrentTime();
		{
			IRowSettingsPtr pRow = m_DateCombo->GetCurSel();
			if(pRow) {

				dftDateID = (DateFilterTypes)VarLong(pRow->GetValue(dccID), dftAll);
				if(dftDateID != dftAll) {

					dtFrom = m_dtFrom.GetValue();
					dtTo = m_dtTo.GetValue();

					if(dtFrom.GetStatus() == COleDateTime::invalid || dtTo.GetStatus() == COleDateTime::invalid) {
						dtFrom = COleDateTime::GetCurrentTime();
						dtTo = dtFrom;
						m_dtFrom.SetValue(_variant_t(dtFrom));
						m_dtTo.SetValue(_variant_t(dtTo));
						AfxMessageBox("The date range selected is invalid. It has been reset to today's date.");
					}

					if(dtFrom > dtTo) {
						dtFrom = dtTo;
						m_dtFrom.SetValue(_variant_t(dtFrom));
						AfxMessageBox("The 'From' date is greater than the 'To' date. It has been reset to match the 'To' date.");
					}
				}
			}
		}

		//make our dates be date-only, increase the to date by one day
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(1, 0 ,0, 0);
		dtTo += dtSpan;
		dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);
		dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);

		//now loop through every problem, and see if each problem matches our filters

		// (c.haag 2009-05-21 13:17) - PLID 34298 - We now iterate one problem link for every row (as opposed to one
		// problem per row). If a problem is linked with multiple EMR objects, then the problem will appear once for
		// every EMR object.
		for(int i=0; i<m_papEMRProblemLinks->GetSize(); i++) {
			
			CEmrProblemLink *pLink = m_papEMRProblemLinks->GetAt(i);
			if(pLink == NULL || pLink->IsDeleted()) {
				continue;
			}
			CEmrProblem* pProblem = pLink->GetProblem();
			if (pProblem == NULL || pProblem->m_bIsDeleted) {
				continue;
			}

			//check the status filter			
			if(nStatusID != (long)pssfAll) {

				// (j.jones 2008-11-20 17:41) - PLID 28497 - need to handle resolved/unresolved filters
				if(nStatusID == (long)pssfResolved || nStatusID == (long)pssfUnresolved) {

					//rather than querying, simply find the problem's status in the list, and find the list item's resolved flag
					IRowSettingsPtr pRow = m_StatusCombo->FindByColumn(sccID, pProblem->m_nStatusID, m_StatusCombo->GetFirstRow(), FALSE);
					if(pRow) {
						BOOL bResolved = VarBool(pRow->GetValue(sccResolved), FALSE);

						if((nStatusID == (long)pssfResolved && !bResolved)
							|| (nStatusID == (long)pssfUnresolved && bResolved)) {
							//doesn't match our status filter, so skip it
							continue;
						}
					}
					else {
						//not found? That's weird. ASSERT, and keep the problem included anyways.
						ASSERT(FALSE);
					}
				}
				else if(nStatusID != pProblem->m_nStatusID) {
					//doesn't match our status filter, so skip it
					continue;
				}
			}

			//check the type filter

			//it is assumed that the caller will include only the problems that are on
			//or underneath the EMR object that calls this dialog
			//(example, calling from an EMR Topic will only include problems on that Topic
			//or any of its Details and List Items)

			//as such, m_eprtDefaultRegardingType should be the type of the master item
			//this list was called from, in which case higher-level types would not be
			//included in the list, so selecting "all" requires no special work here

			if(eprtTypeID != eprtInvalid) {

				//first simply check to see if the problem is of that type
				// (c.haag 2009-05-28 10:19) - PLID 34298 - Check the problem link type
				if(pLink->GetType() != eprtTypeID) {
					//doesn't match our type, so skip it
					continue;
				}
			}

			//check the date filter
			if(dftDateID == dftEntered) {

				COleDateTime dtEnteredToCompare;
				dtEnteredToCompare.SetDateTime(pProblem->m_dtEnteredDate.GetYear(), pProblem->m_dtEnteredDate.GetMonth(), pProblem->m_dtEnteredDate.GetDay(), 0, 0, 0);

				if(dtEnteredToCompare < dtFrom || dtEnteredToCompare >= dtTo) {
					//outside our date filter, so skip it
					continue;
				}
			}
			else if(dftDateID == dftModified) {

				COleDateTime dtModifiedToCompare;
				dtModifiedToCompare.SetDateTime(pProblem->m_dtModifiedDate.GetYear(), pProblem->m_dtModifiedDate.GetMonth(), pProblem->m_dtModifiedDate.GetDay(), 0, 0, 0);

				if(dtModifiedToCompare < dtFrom || dtModifiedToCompare >= dtTo) {
					//outside our date filter, so skip it
					continue;
				}
			}

			//if we make it this far, the problem matches our filters, so add it to the list

			long nDetailID = -1;
			long nEMNID = -1;
			EmrInfoType eDataType = eitInvalid;

			// (c.haag 2009-05-21 13:25) - PLID 34298 - We now pull details from the problem link object
			if(pLink->GetDetail()) {
				CEMNDetail* pDetail = pLink->GetDetail();
				nDetailID = pDetail->GetID();

				// (j.jones 2008-10-30 15:41) - PLID 31869 - only if a detail, we need the EMNID and datatype
				if(pDetail->m_pParentTopic) {
					if(pDetail->m_pParentTopic->GetParentEMN()) {
						nEMNID = pDetail->m_pParentTopic->GetParentEMN()->GetID();
					}
				}
				eDataType = pDetail->m_EMRInfoType;
			}

			CString strStatusName = "";
			// (j.jones 2008-11-20 17:40) - PLID 28497 - this logic changes slightly now that we have multiple status
			// types that can return multiple results
			if(nStatusID != (long)pssfAll && nStatusID != (long)pssfResolved && nStatusID != (long)pssfUnresolved) {
				//we're filtering by one specific status, all problems would have the same status name
				strStatusName = strStatusFilterName;
			}
			else {
				//we're filtering on multiple statuses, so we need to grab the names from the datalist
				IRowSettingsPtr pRow = m_StatusCombo->FindByColumn(sccID, pProblem->m_nStatusID, m_StatusCombo->GetFirstRow(), FALSE);
				if(pRow) {
					strStatusName = VarString(pRow->GetValue(sccName), "");
				}
			}

			CString strValue = "";
			CString strDetailName = "";
			CString strTopicName, strEMNName, strEMRName;

			// (c.haag 2009-05-21 13:25) - PLID 34298 - We now pull details from the problem link object
			GetDetailNameAndValue(pLink, strDetailName, strValue, strTopicName, strEMNName, strEMRName);

			pProblem->m_dtEnteredDate.SetDateTime(pProblem->m_dtEnteredDate.GetYear(), pProblem->m_dtEnteredDate.GetMonth(), pProblem->m_dtEnteredDate.GetDay(), 0, 0, 0);

			_variant_t varOnsetDate = g_cvarNull;
			if(pProblem->m_dtOnsetDate.GetStatus() != COleDateTime::invalid) {
				pProblem->m_dtOnsetDate.SetDateTime(pProblem->m_dtOnsetDate.GetYear(), pProblem->m_dtOnsetDate.GetMonth(), pProblem->m_dtOnsetDate.GetDay(), 0, 0, 0);
				varOnsetDate = _variant_t(pProblem->m_dtOnsetDate, VT_DATE);
			}

			// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns
			CString strICD9Name, strICD10Name;
			GetICD9And10Codes(pProblem->m_nDiagICD9CodeID, pProblem->m_nDiagICD10CodeID, strICD9Name, strICD10Name);
			if(strICD9Name.IsEmpty()) {
				strICD9Name = "<None>";
			}
			if(strICD10Name.IsEmpty()) {
				strICD10Name = "<None>";
			}
			
			// (a.walling 2009-05-04 12:18) - PLID 33751, 28495 - This is awful, but we are pressed for time. PLID 34147 would implement
			// a combo which we could use instead of these queries if we don't optimize it before this item is done.
//#pragma TODO("PLID 33751, 28495 - Find a better way to keep track of strings for diagcodes, chronicity statuses")
			CString strChronicityName = "<None>";
			if (pProblem->m_nChronicityID != -1) {
				strChronicityName = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", pProblem->m_nChronicityID), "<None>");
			}

			IRowSettingsPtr pRow = m_dlProblems->GetNewRow();
			pRow->PutValue(plcID, (long)pProblem->m_nID);
			pRow->PutValue(plcDetailID, nDetailID);
			// (j.jones 2008-10-30 15:28) - PLID 31869 - added plcEMNID and plcDataType
			pRow->PutValue(plcEMNID, nEMNID);
			pRow->PutValue(plcDataType, (short)eDataType);
			pRow->PutValue(plcEnteredDate, _variant_t(pProblem->m_dtEnteredDate, VT_DATE));
			pRow->PutValue(plcOnsetDate, varOnsetDate);
			pRow->PutValue(plcStatusName, (LPCSTR)strStatusName);
			// (a.walling 2009-05-04 12:18) - PLID 33751, 28495 - Update the diag code and chronicity
			// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns
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
			// (c.haag 2009-05-21 17:15) - PLID 34298 - Problem link pointer
			pRow->PutValue(plcProblemLinkPtr, (long)pLink);
			m_dlProblems->AddRowSorted(pRow, NULL);
		}

		m_bIsRefilteringFromMemory = FALSE;

		// (j.jones 2014-02-26 13:57) - PLID 60763 - diagnosis codes now can dynamically
		// resize their columns based on content, so we need to try a resize now
		ResizeColumns();

		UpdateHistoryListDiagnosisColumns();

	}NxCatchAll("Error in CEMRProblemListDlg::RefilterListFromMemory");
}

// (j.jones 2008-07-16 10:43) - PLID 30731 - This function will filter the list accordingly and requery.
// bForceRequery is set to TRUE if the caller wants the list to requery even if the filter does not change.
void CEMRProblemListDlg::RefilterListFromSql(BOOL bForceRequery)
{
	try {

		CWaitCursor pWait;

		// (c.haag 2006-10-19 12:07) - PLID 21454 - Consider the deleted flag now
		//
		CString strWhere;
		strWhere.Format("PatientID = %d AND Deleted = 0 ", m_nPatientID);

		//(e.lally 2008-07-23) PLID 30732 - Generate where clauses based on our filters and add to our string
		//check the status filter
		{
			CString strStatusFilter = GetWhereClauseFromStatusFilter();
			if(!strStatusFilter.IsEmpty()){
				strWhere += " AND " + strStatusFilter;
			}

		}

		//check the type filter
		{
			CString strTypeFilter = GetWhereClauseFromTypeFilter();
			if(!strTypeFilter.IsEmpty()){
				strWhere += " AND " + strTypeFilter;
			}
		}

		//check the date filter
		{
			CString strDateFilter = GetWhereClauseFromDateFilter();
			if(!strDateFilter.IsEmpty()){
				strWhere += " AND " + strDateFilter;
			}
		}

		CString strOldWhere = (LPCTSTR)m_dlProblems->WhereClause;
		//reload the list if the filter changed or if the caller specifically tells us to do so
		if(strOldWhere != strWhere || bForceRequery) {

			// (c.haag 2009-02-09 09:12) - PLID 32976 - If a requery is in progress, stop it now.
			// This can happen if a refiltering takes place in response to a user action right
			// before an EMR problem table checker arrives.
			if (m_dlProblems->IsRequerying()) {
				m_dlProblems->CancelRequery();
			}

			m_dlProblems->WhereClause = (LPCTSTR)strWhere;

			//first kill the PopulateDetailValuesThread thread, if it is running
			KillThread();

			m_dlHistory->Clear();
			UpdateHistoryListDiagnosisColumns();

			m_dlProblems->Requery();
		}

	}NxCatchAll("Error in CEMRProblemListDlg::RefilterListFromSql");
}

//(e.lally 2008-07-23) PLID 30732 - return the resulting string for filtering a recordset using the selected Status
CString CEMRProblemListDlg::GetWhereClauseFromStatusFilter()
{
	IRowSettingsPtr pRow = m_StatusCombo->GetCurSel();
	if(pRow) {
		long nStatusID = VarLong(pRow->GetValue(sccID), (long)pssfAll);
		// (j.jones 2008-11-20 17:45) - PLID 28497 - we now have resolved/unresolved filters
		if(nStatusID == (long)pssfAll) {
			return "";
		}
		else if(nStatusID == (long)pssfResolved) {
			return " StatusID IN (SELECT ID FROM EMRProblemStatusT WHERE Resolved = 1) ";
		}
		else if(nStatusID == (long)pssfUnresolved) {
			return " StatusID IN (SELECT ID FROM EMRProblemStatusT WHERE Resolved = 0) ";
		}
		else {
			CString str;
			str.Format(" StatusID = %li ", nStatusID);
			return str;
		}
	}
	return "";
}

//(e.lally 2008-07-23) PLID 30732 - return the resulting string for filtering a recordset using the selected Regarding Type
CString CEMRProblemListDlg::GetWhereClauseFromTypeFilter()
{
	CString str;
	EMRProblemRegardingTypes eprtTypeID = eprtInvalid;
	IRowSettingsPtr pRow = m_TypeCombo->GetCurSel();
	if(pRow) {
		eprtTypeID = (EMRProblemRegardingTypes)VarLong(pRow->GetValue(tccID), eprtInvalid);
	}
	
	if(eprtTypeID != eprtInvalid) {
		//if we have a default type, it means we are based on an EMR object
		//and need to show only problems directly under the umbrella of that object
		if(m_eprtDefaultRegardingType != eprtInvalid) {

			if(m_eprtDefaultRegardingType == eprtTypeID) {
				//filter on only the default object
				str.Format(" EMRRegardingType = %li AND EMRRegardingID = %li ",
					m_eprtDefaultRegardingType, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrItem && eprtTypeID == eprtEmrDataItem) {

				//filter on only the list items for the specified EmrItem
				str.Format(" EMRRegardingType = %li AND EMRRegardingID = %li ",
					eprtEmrDataItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrTopic && eprtTypeID == eprtEmrItem) {

				//filter on only the details for the specified topic
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRTopicID = %li) ",
					eprtEmrItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrTopic && eprtTypeID == eprtEmrDataItem) {

				//filter on only the list items for the specified topic
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRTopicID = %li) ",
					eprtEmrDataItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrTopic) {

				//filter on only the topics for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRTopicsT WHERE EMRID = %li) ",
					eprtEmrTopic, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrItem) {

				//filter on only the details for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRID = %li) ",
					eprtEmrItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrDataItem) {

				//filter on only the list items for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRID = %li) ",
					eprtEmrDataItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrDiag) {

				//filter on only the diagnoses for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDiagCodesT WHERE EMRID = %li) ",
					eprtEmrDiag, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrCharge) {

				//filter on only the charges for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRChargesT WHERE EMRID = %li) ",
					eprtEmrCharge, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN && eprtTypeID == eprtEmrMedication) {

				//filter on only the medications for the specified EMN
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT MedicationID FROM EMRMedicationsT WHERE EMRID = %li) ",
					eprtEmrMedication, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrEMN) {

				//filter on only the EMNs for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li) ",
					eprtEmrEMN, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrTopic) {

				//filter on only the topics for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRTopicsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrTopic, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrItem) {

				//filter on only the details for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrDataItem) {

				//filter on only the list items for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrDataItem, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrDiag) {

				//filter on only the diagnoses for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRDiagCodesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrDiag, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrCharge) {

				//filter on only the charges for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT ID FROM EMRChargesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrCharge, m_nDefaultRegardingID);
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR && eprtTypeID == eprtEmrMedication) {

				//filter on only the medications for the specified EMR
				str.Format(" EMRRegardingType = %li AND EMRRegardingID IN "
					"	(SELECT MedicationID FROM EMRMedicationsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) ",
					eprtEmrMedication, m_nDefaultRegardingID);
			}
			else {

				//What other combinations are there? Filter on the default item as a failsafe,
				//but if you hit this assertion, find out why it's not covered in this list.

				ASSERT(FALSE);

				//filter on only the default object
				str.Format(" EMRRegardingType = %li AND EMRRegardingID = %li ",
					m_eprtDefaultRegardingType, m_nDefaultRegardingID);
			}
		}
		else {
			//no default type, so filter exclusively on the selected type
			str.Format(" EMRRegardingType = %li ", (long)eprtTypeID);
		}

	}else if(m_eprtDefaultRegardingType != eprtInvalid) {
		//"all" is selected, and we have a default type,
		//so show all problems on or beneath that type

		if(m_eprtDefaultRegardingType == eprtEmrItem) {

			//filter on the EmrItem and its List Items
			str.Format(" ("
				"(EMRRegardingType = %li AND EMRRegardingID = %li) "
				"OR (EMRRegardingType = %li AND EMRRegardingID = %li) "
				")",
				eprtEmrItem, m_nDefaultRegardingID,
				eprtEmrDataItem, m_nDefaultRegardingID);
		}
		else if(m_eprtDefaultRegardingType == eprtEmrTopic) {
			
			//filter on the Topic, its EmrItems, and List Items
			str.Format(" ("
				"(EMRRegardingType = %li AND EMRRegardingID = %li) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRTopicID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRTopicID = %li)) "
				")",
				eprtEmrTopic, m_nDefaultRegardingID,
				eprtEmrItem, m_nDefaultRegardingID,
				eprtEmrDataItem, m_nDefaultRegardingID);
		}
		else if(m_eprtDefaultRegardingType == eprtEmrEMN) {
			
			//filter on the EMN, its Topics, EmrItems, and List Items,
			//plus diags, and charges
			str.Format(" ("
				"(EMRRegardingType = %li AND EMRRegardingID = %li) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRTopicsT WHERE EMRID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDiagCodesT WHERE EMRID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRChargesT WHERE EMRID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT MedicationID FROM EMRMedicationsT WHERE EMRID = %li)) "
				")",
				eprtEmrEMN, m_nDefaultRegardingID,
				eprtEmrTopic, m_nDefaultRegardingID,
				eprtEmrItem, m_nDefaultRegardingID,
				eprtEmrDataItem, m_nDefaultRegardingID,
				eprtEmrDiag, m_nDefaultRegardingID,
				eprtEmrCharge, m_nDefaultRegardingID,
				eprtEmrMedication, m_nDefaultRegardingID);
		}
		else if(m_eprtDefaultRegardingType == eprtEmrEMR) {
			
			//filter on the EMR, its EMNs, Topics, EmrItems, and List Items
			//plus diags, and charges
			str.Format(" ("
				"(EMRRegardingType = %li AND EMRRegardingID = %li) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li)) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRTopicsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRDiagCodesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT ID FROM EMRChargesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				"OR (EMRRegardingType = %li AND EMRRegardingID IN "
				"	(SELECT MedicationID FROM EMRMedicationsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li))) "
				")",
				eprtEmrEMR, m_nDefaultRegardingID,
				eprtEmrEMN, m_nDefaultRegardingID,
				eprtEmrTopic, m_nDefaultRegardingID,
				eprtEmrItem, m_nDefaultRegardingID,
				eprtEmrDataItem, m_nDefaultRegardingID,
				eprtEmrDiag, m_nDefaultRegardingID,
				eprtEmrCharge, m_nDefaultRegardingID,
				eprtEmrMedication, m_nDefaultRegardingID);
		}
		else {
			//otherwise filter just on the type and ID, there are no children
			str.Format(" EMRRegardingType = %li AND EMRRegardingID = %li ",
				m_eprtDefaultRegardingType, m_nDefaultRegardingID);
		}
	}
	return str;
}

//(e.lally 2008-07-23) PLID 30732 - return the resulting string for filtering a recordset using the selected dates.
CString CEMRProblemListDlg::GetWhereClauseFromDateFilter()
{
	CString str;
	IRowSettingsPtr pRow = m_DateCombo->GetCurSel();
	if(pRow) {

		DateFilterTypes dftDateID = (DateFilterTypes)VarLong(pRow->GetValue(dccID), dftAll);
		if(dftDateID != dftAll) {

			COleDateTime dtFrom = m_dtFrom.GetValue();
			COleDateTime dtTo = m_dtTo.GetValue();

			if(dtFrom.GetStatus() == COleDateTime::invalid || dtTo.GetStatus() == COleDateTime::invalid) {
				dtFrom = COleDateTime::GetCurrentTime();
				dtTo = dtFrom;
				m_dtFrom.SetValue(_variant_t(dtFrom));
				m_dtTo.SetValue(_variant_t(dtTo));
				AfxMessageBox("The date range selected is invalid. It has been reset to today's date.");
			}

			if(dtFrom > dtTo) {
				dtFrom = dtTo;
				m_dtFrom.SetValue(_variant_t(dtFrom));
				AfxMessageBox("The 'From' date is greater than the 'To' date. It has been reset to match the 'To' date.");
			}

			//(e.lally 2008-07-30) PLID 30732 - The preview report needs to specify where to pull the date from,
				//set it to ProblemsQ so it doesn't conflict with the dialogs from clause.
			if(dftDateID == dftEntered) {
				str.Format(" ProblemsQ.EnteredDate >= '%s' AND ProblemsQ.EnteredDate < DATEADD(day, 1, '%s') ",
					FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
			}
			else if(dftDateID == dftModified) {
				str.Format(" ProblemsQ.ModifiedDate >= '%s' AND ProblemsQ.ModifiedDate < DATEADD(day, 1, '%s') ",
					FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
			}
		}
	}
	return str;
}

BEGIN_EVENTSINK_MAP(CEMRProblemListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRProblemListDlg)
	ON_EVENT(CEMRProblemListDlg, IDC_LIST_EMR_PROBLEMS, 18 /* RequeryFinished */, OnRequeryFinishedListEmrProblems, VTS_I2)
	ON_EVENT(CEMRProblemListDlg, IDC_LIST_EMR_PROBLEMS, 2 /* SelChanged */, OnSelChangedListEmrProblems, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMRProblemListDlg, IDC_LIST_EMR_PROBLEMS, 3 /* DblClickCell */, OnDblClickCellListEmrProblems, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEMRProblemListDlg, IDC_PROBLEM_STATUS_FILTER, 16 /* SelChosen */, OnSelChosenProblemStatusFilter, VTS_DISPATCH)
	ON_EVENT(CEMRProblemListDlg, IDC_PROBLEM_OBJECT_TYPE_FILTER, 16 /* SelChosen */, OnSelChosenProblemObjectTypeFilter, VTS_DISPATCH)
	ON_EVENT(CEMRProblemListDlg, IDC_PROBLEM_DATE_FILTER, 16 /* SelChosen */, OnSelChosenProblemDateFilter, VTS_DISPATCH)
	ON_EVENT(CEMRProblemListDlg, IDC_LIST_EMR_PROBLEM_HISTORY, 18 /* RequeryFinished */, OnRequeryFinishedListEmrProblemHistory, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRProblemListDlg::OnRequeryFinishedListEmrProblems(short nFlags) 
{
	try {
		// (c.haag 2009-02-09 09:16) - PLID 32976 - Look at the result flags
		switch ((ERequeryFinished)nFlags) {
			case dlRequeryFinishedCompleted:
				// This is the expected value. Proceed normally.
				break;
			case dlRequeryFinishedCanceled:
				// The requery was cancelled. We need to return from this
				// function since it was the application's intent to cancel 
				// the entire requery operation.
			case dlRequeryFinishedError:
				// An error occurred during the requery. We should stop further
				// refreshing until the next successful requery.
			default:
				// Unexpected value (should never happen)
				return;
		}

		// (c.haag 2009-02-09 09:20) - PLID 32976 - At this point, m_pPopulateDetailValuesThread should be NULL.
		// However, in the unexpected case it is not (which I have achieved in at least one test that I cannot
		// reproduce), we need to kill it now to, at a minimum, prevent two instances of the thread running at once.
		if (NULL != m_pPopulateDetailValuesThread) {
			KillThread();
		}

		//
		// Requery the history list
		//
		if (m_dlProblems->GetRowCount() > 0) {
			IRowSettingsPtr pRow = m_dlProblems->GetFirstRow();
			m_dlProblems->CurSel = pRow;
			RequeryHistory(VarLong(pRow->Value[0]));
			GetDlgItem(IDC_BTN_EDIT_PROBLEM)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_DELETE_PROBLEM)->EnableWindow(TRUE);
		} else {
			m_dlHistory->Clear();
			UpdateHistoryListDiagnosisColumns();
			GetDlgItem(IDC_BTN_EDIT_PROBLEM)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_DELETE_PROBLEM)->EnableWindow(FALSE);
		}

		// (c.haag 2009-02-09 15:15) - PLID 32976 - Populate the data to pass to the thread
		CArray<ProblemListThreadValue*,ProblemListThreadValue*>* papPLTV = new CArray<ProblemListThreadValue*,ProblemListThreadValue*>;
		{
			IRowSettingsPtr pRow = m_dlProblems->GetFirstRow();
			while (NULL != pRow) {
				if (-1 != VarLong(pRow->GetValue(plcDetailID), -1)) {
					ProblemListThreadValue* p = new ProblemListThreadValue;
					p->pDlg = this;
					// (j.jones 2009-05-28 14:00) - PLID 34298 - supported problem links
					p->nProblemID = VarLong(pRow->GetValue(plcID));
					p->nProblemLinkID = VarLong(pRow->GetValue(plcProblemLinkID));
					p->nDetailID = VarLong(pRow->GetValue(plcDetailID), -1);
					p->nEMNID = VarLong(pRow->GetValue(plcEMNID), -1);
					p->DataType = (EmrInfoType)VarByte(pRow->GetValue(plcDataType), eitInvalid);
					papPLTV->Add(p);
				}
				pRow = pRow->GetNextRow();
			}
		}

		if (papPLTV->GetSize() > 0) { 
			//
			// Populate the problem list with detail values
			//
			m_pPopulateDetailValuesThread = AfxBeginThread(PopulateDetailValuesThread, papPLTV, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
			// (j.jones 2008-10-30 16:53) - PLID 31869 - we need to track the thread ID
			if(GetMainFrame()) {
				//this should never already have an ID because this thread shouldn't be able to be called twice at the same time!
				ASSERT(GetMainFrame()->m_nProblemListDlg_PopulateDetailValues_CurThreadID == -1);
				GetMainFrame()->m_nProblemListDlg_PopulateDetailValues_CurThreadID = m_pPopulateDetailValuesThread->m_nThreadID;
			}
			else {
				//should be impossible
				ASSERT(FALSE);
			}
			m_pPopulateDetailValuesThread->m_bAutoDelete = false;
			m_pPopulateDetailValuesThread->ResumeThread();

		} else {
			// Nothing to do -- no valid details
			delete papPLTV;
		}

		// (j.jones 2014-02-26 13:57) - PLID 60763 - diagnosis codes now can dynamically
		// resize their columns based on content, so we need to try a resize now
		ResizeColumns();
	}
	// (a.walling 2007-07-20 10:56) - PLID 26762 - Need to use the thread-safe exception handling
	// (a.walling 2007-09-05 08:41) - PLID 26762 - Well, this isn't a thread..
	NxCatchAll("Error in OnRequeryFinishedListEmrProblems");
}

void CEMRProblemListDlg::OnSelChangedListEmrProblems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		IRowSettingsPtr pRow = lpNewSel;
		if (NULL != pRow) {
			RequeryHistory(VarLong(pRow->GetValue(plcID)));
			GetDlgItem(IDC_BTN_EDIT_PROBLEM)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_DELETE_PROBLEM)->EnableWindow(TRUE);
		} else {
			m_dlHistory->Clear();
			UpdateHistoryListDiagnosisColumns();
			GetDlgItem(IDC_BTN_EDIT_PROBLEM)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_DELETE_PROBLEM)->EnableWindow(FALSE);
		}
	}
	NxCatchAll("Error in OnSelChangedListEmrProblems");
}


void CEMRProblemListDlg::OnDblClickCellListEmrProblems(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2006-10-17 13:12) - PLID 23116 - Allow the user to edit a problem by double-clicking on it
	try {
		IRowSettingsPtr pRow = lpRow;
		if (NULL != pRow) {
			m_dlProblems->CurSel = pRow;
			OnBtnEditProblem();
		}
	}
	NxCatchAll("Error in OnDblClickCellListEmrProblems");
}


void CEMRProblemListDlg::OnCheckShowProblemDesc() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowProblemDesc) ? 1 : 0, 0, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnCheckShowDetailData() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowDetailData) ? 1 : 0, 1, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnCheckShowDetailName() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowDetailName) ? 1 : 0, 2, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnCheckShowTopicName() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowTopicName) ? 1 : 0, 3, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnCheckShowEmnName() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowEMNName) ? 1 : 0, 4, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnCheckShowEmrName() 
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowEMRName) ? 1 : 0, 5, GetCurrentUserName());
	ResizeColumns();
}

void CEMRProblemListDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();
	
	try {
		// Destroy our thread object
		KillThread();
	}
	NxCatchAll("Error in OnDestroy()");
}

// (c.haag 2008-11-26 12:29) - PLID 28496 - We can now add EMR-less problems
void CEMRProblemListDlg::OnBtnNewProblem()
{
	// (c.haag 2009-05-22 15:01) - PLID 34298 - We now edit problems under
	// the new problem linking structure (Josh made the code changes but
	// I'm still responsible for if this doesn't work)
	try {
		// Check permissions for creating problems
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		// Invoke the problem edit dialog
		CEMRProblemEditDlg dlg(this);
		// We always write to data, no matter where this list is invoked from
		dlg.SetWriteToData(TRUE);
		// New problems are not tied to any records other than the patient's file itself. Set
		// the owner ID to the patient (this is redundant, but the save will fail if the owner
		// ID is -1).
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function
		dlg.AddLinkedObjectInfo(-1, eprtUnassigned, GetExistingPatientName(m_nPatientID), "", m_nPatientID);
		dlg.SetPatientID(m_nPatientID);
		if (dlg.DoModal()) {
			// (c.haag 2009-02-09 16:16) - PLID 32976 - Don't refilter the list; the dialog
			// throws a table checker that gets bounced back to us
			//RefilterList(TRUE); // Refresh the problem list
		}
	}
	NxCatchAll("Error in CEMRProblemListDlg::OnBtnNewProblem");
}

void CEMRProblemListDlg::OnBtnEditProblem() 
{
	try {
		//
		// (c.haag 2006-10-17 12:52) - PLID 23116 - Let the user edit the problem from this dialog
		//
		IRowSettingsPtr pRow = m_dlProblems->CurSel;
		if (NULL != pRow) {

			// (j.jones 2008-07-18 16:06) - PLID 30773 - check and see if we have a problem ptr.,
			// and if so, edit that pointer, and do not commit the changes to data
			CEmrProblem *pProblem = (CEmrProblem*)VarLong(pRow->GetValue(plcProblemPtr), 0);
			if(pProblem) {
				// (c.haag 2009-05-22 15:50) - PLID 34298 - We don't need to pass in the problem
				EditProblemFromMemory(pRow);
			}
			else {
				EditProblemFromData(pRow);
			}

			// (j.jones 2008-11-20 16:29) - PLID 32119 - added a problem status tablechecker,
			// used predominantly in modal dialogs, because if this window is modeless it
			// will be updated in realtime, and this member will likely never remain marked
			// as Changed() for long
			if(m_ProblemStatusChecker.Changed()) {
					
				RequeryStatusFilter(TRUE);
			}

		} else {
			MsgBox("Please select a problem to edit");
		}
	}NxCatchAll("Error in CEMRProblemListDlg::OnBtnEditProblem");
}

// (j.jones 2008-07-18 16:10) - PLID 30773 - Made separate functions for editing
// a problem from a memory object, or simply from the SQL-generated list
// (c.haag 2009-05-22 16:50) - PLID 34298 - No need to pass in a problem or problem
// link object; it's already in the row
void CEMRProblemListDlg::EditProblemFromMemory(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (c.haag 2009-05-22 15:01) - PLID 34298 - We now edit problems under
	// the new problem linking structure.
	//
	// In the old implementation, we just passed in information from a single problem object,
	// pulled the changes out, and updated the problem. Now we pass in an array of problem
	// links (one per-problem per-EMR-object).
	//
	try {
		if(pRow == NULL) {
			return;
		}
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptRead)) {
			return;
		}

		// (j.jones 2008-07-18 16:16) - PLID 30773 - this logic was moved and modified from CEmrItemAdvDlg::EditProblem
		// (c.haag 2006-06-29 15:07) - PLID 19977 - We can now mark a detail as a problem.
		CEMRProblemEditDlg dlg(this);
		CString strDescription = VarString(pRow->GetValue(plcDetailName), "");
		CEmrProblemLink* pSelLink = (CEmrProblemLink*)VarLong(pRow->GetValue(plcProblemLinkPtr), 0);
		CEmrProblem* pProblem = pSelLink->GetProblem();
		ASSERT((long)pProblem == VarLong(pRow->GetValue(plcProblemPtr)));
		dlg.SetProblemID(pProblem->m_nID);
		dlg.SetProblemStatusID(pProblem->m_nStatusID);
		// (a.walling 2009-05-04 15:23) - PLID 33751, 28495
		// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
		dlg.SetProblemDiagCodeIDs(pProblem->m_nDiagICD9CodeID, pProblem->m_nDiagICD10CodeID);
		dlg.SetProblemChronicityID(pProblem->m_nChronicityID);
		dlg.SetProblemDesc(pProblem->m_strDescription);
		// (c.haag 2008-07-24 17:52) - PLID 30727 - Added support for onset date
		dlg.SetOnsetDate(pProblem->m_dtOnsetDate);
		dlg.SetProblemCodeID(pProblem->m_nCodeID); 
		// (s.tullis 2015-02-23 15:44) - PLID 64723 
		dlg.SetProblemDoNotShowOnCCDA(pProblem->m_bDoNotShowOnCCDA);
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Load the dialog's Do Not Show On Problem Prompt value from the in memory object.
		dlg.SetProblemDoNotShowOnProblemPrompt(pProblem->m_bDoNotShowOnProblemPrompt);
		int i;
		{
			// (z.manning 2009-07-01 16:02) - PLID 34765 - Added this check to EditProblemFromMemory too
			if (WarnIfEMNConcurrencyIssuesExist(this, 
					CEmrProblem::GetEMNQueryFromProblemID(pProblem->m_nID, m_bIncludeEmrLevelProblemsInConcurrencyCheck),
					"This problem may viewed, but not modified.",
					m_bExcludeCurrentUserFromEmnConcurrencyCheck)) {					
				dlg.SetReadOnly(TRUE);
			}
		}

		// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function
		// (c.haag 2009-05-22 15:08) - PLID 34298 - We have the problem and one problem link.
		// However, we need to have all the problem's links in memory. The list may not have 
		// all of them (consider forinstance editing a problem on an EMR item that is also 
		// linked with the parent EMR itself). We also don't have the other problem links
		// that we would need from data (consider an EMR problem for both an EMR topic and
		// a lab). So, we will need problem links from data that aren't in memory.
		CArray<CEmrProblemLink*,CEmrProblemLink*> apAllLinks;
		CArray<CEmrProblemLink*,CEmrProblemLink*> apLinksFromData;
		if (NULL == pSelLink) { ThrowNxException("Could not edit non-existent problem link"); }
		// (j.jones 2009-05-29 12:32) - PLID 34301 - this is now in EmrUtils
		PopulateProblemLinkArrays(pProblem, pSelLink->GetEMR(), apAllLinks, apLinksFromData,
			m_dlProblems, plcProblemLinkPtr, plcProblemLinkID, plcDetailName, plcDetailValue, m_pdlgLabRequisition);
		for (i=0; i < apAllLinks.GetSize(); i++) {

			CEmrProblemLink* pCurLink = apAllLinks[i];

			// (j.jones 2009-05-26 18:25) - PLID 34250 - the detail value field is currently only filled on detail objects
			CString strDetailValue;
			if(pCurLink->GetType() == eprtEmrDataItem || pCurLink->GetType() == eprtEmrItem) {
				strDetailValue = pCurLink->m_strDetailValue;
			}
			dlg.AddLinkedObjectInfo(pCurLink->GetID(), pCurLink->GetType(), pCurLink->m_strDetailName, 
				strDetailValue, pCurLink->GetRegardingID(), pCurLink, pCurLink->GetDataID());

			// (c.haag 2009-05-22 15:32) - PLID 34298 - Changed all functions to access the
			// link, not the problem itself.
			// (a.walling 2008-06-12 10:27) - PLID 23138 - Set readonly status
			// we'll get this from the EMN for safety's sake.
			if(pCurLink->GetDetail()) {
				pCurLink->UpdatePointersWithDetail(pCurLink->GetDetail());
			}
			// (j.jones 2008-07-28 09:55) - PLID 30854 - if it's a topic
			// problem type, make sure we have updated the
			// EMN pointer from that topic. If it were a charge, diag, or
			// medication, the creator of that object should have passed in
			// an EMN pointer already.
			if(pCurLink->GetTopic()) {
				pCurLink->UpdatePointersWithTopic(pCurLink->GetTopic());
			}
		}

		// (j.jones 2008-07-28 09:56) - PLID 30854 - by now if it is an
		// EMN-level problem or any problem type underneath it, we should
		// have an EMN pointer - assert if we don't, unless it's an
		// EMR-level problem.
		// (c.haag 2009-05-26 15:40) - PLID 34298 - We now enforce this rule if at least
		// one EMR problem is linked to an EMN without a write token.
		BOOL bIsReadOnly = FALSE;
		for (i=0; i < apAllLinks.GetSize() && !bIsReadOnly; i++) {
			CEmrProblemLink* pCurLink = apAllLinks[i];
			if (pCurLink->GetEMN()) {
				if (!(pCurLink->GetEMN()->IsWritable())) {
					// Warn the user
					MsgBox(MB_ICONINFORMATION, "You will not be able to modify this problem because you do not have write access to the '%s' EMN, which is related to the problem.", pCurLink->GetEMN()->GetDescription() );
					bIsReadOnly = TRUE;
				} else {
					// If we get here, we own a write token to the EMN
				}
			}
		}
		if (bIsReadOnly) {
			dlg.SetReadOnly(TRUE);
		}

		// (c.haag 2009-05-26 15:47) - PLID 34298 - Set the patient ID
		for (i=0; i < apAllLinks.GetSize(); i++) {
			CEmrProblemLink* pCurLink = apAllLinks[i];
			if(pCurLink->GetEMR()) {
				dlg.SetPatientID(pCurLink->GetEMR()->GetPatientID());
				break;
			}
		}

		if (IDOK == dlg.DoModal()) {
			// (c.haag 2006-12-27 11:25) - PLID 23158 - If the problem was deleted, update the
			// detail so that the button is properly updated
			if (dlg.ProblemWasDeleted()) {
				// (j.jones 2009-05-29 09:03) - PLID 34301 - handle problem deleting")
				pProblem->m_bIsDeleted = TRUE;

				pProblem->m_bIsModified = TRUE;

				// (j.jones 2009-05-28 17:16) - PLID 34301 - delete all the links
				for(int i=0; i < apAllLinks.GetSize(); i++) {
					
					//the saving code will be responsible for deleting links
					//that are not actually loaded in memory, but here we will
					//flag all the links we've got
					CEmrProblemLink *pLink = apAllLinks.GetAt(i);
					if(pLink) {
						pLink->SetDeleted();
					}

					// (c.haag 2009-05-29 12:08) - PLID 34298 - All of the link deletions take place from
					// within the EMR save query. Not all of those links may be in the EMR, however. One or
					// more of them could belong to an outside lab or another EMR. So, how do those links
					// get recognized by the EMR save query? We use a very ad-hoc solution: Add those links
					// to the EMR. They will never show up anywhere because they are already deleted. When
					// the user is prompted with a list of links to be deleted, these links will also appear
					// (because they are indeed going to be deleted by the big save batch)
					if (NULL == pLink->GetEMR()) {
						CEMR* pEMR = pSelLink->GetEMR();
						ASSERT(NULL != pEMR);
						pEMR->m_apEmrProblemLinks.Add(pLink);
						pLink->UpdatePointersWithEMR(pEMR);
					}

					//call the function to update the EMR interface properly to reflect this change
					UpdateEMRInterface(m_pMessageWnd, pLink);
				}

				// (j.jones 2009-05-28 17:53) - PLID 34301 - remove all rows with this problem pointer
				IRowSettingsPtr pRowToRemove = m_dlProblems->FindByColumn(plcProblemPtr, (long)pProblem, m_dlProblems->GetFirstRow(), FALSE);
				while(pRowToRemove) {
					m_dlProblems->RemoveRow(pRowToRemove);
					pRowToRemove = m_dlProblems->FindByColumn(plcProblemPtr, (long)pProblem, m_dlProblems->GetFirstRow(), FALSE);
				}

				// (j.jones 2014-02-26 13:57) - PLID 60763 - diagnosis codes now can dynamically
				// resize their columns based on content, so we need to try a resize now
				ResizeColumns();

			} else {
				if (dlg.IsModified()) {
					pProblem->m_strDescription = dlg.GetProblemDesc();
					pProblem->m_nStatusID = dlg.GetProblemStatusID();
					// (a.walling 2009-05-04 15:23) - PLID 33751, 28495
					// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
					dlg.GetProblemDiagCodeIDs(pProblem->m_nDiagICD9CodeID, pProblem->m_nDiagICD10CodeID);
					pProblem->m_nChronicityID = dlg.GetProblemChronicityID();
					pProblem->m_dtModifiedDate = COleDateTime::GetCurrentTime();
					// (c.haag 2008-07-25 11:19) - PLID 30727 - Update the onset date
					pProblem->m_dtOnsetDate = dlg.GetOnsetDate();
					// (c.haag 2006-11-13 15:02) - PLID 22052 - If the EMN is locked, then the dialog
					// will have already saved the changes to data for us. We therefore do not need
					// to flag anything as being unsaved.					
					pProblem->m_bIsModified = TRUE;
					// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
					pProblem->m_nCodeID = dlg.GetProblemCodeID(); 
					// (s.tullis 2015-02-23 15:44) - PLID 64723 
					pProblem->m_bDoNotShowOnCCDA = dlg.GetProblemDoNotShowOnCCDA();
					// (r.gonet 2015-03-09 18:21) - PLID 65008 - Assign the in memory object's Do Not Show On Problem Prompt value from the dialog.
					pProblem->m_bDoNotShowOnProblemPrompt = dlg.GetProblemDoNotShowOnProblemPrompt();
					// (j.jones 2009-05-26 18:06) - PLID 34250 - handle deleted links
					for (i=0; i < dlg.m_arypLinkedObjects.GetSize(); i++) {
						LinkedObjectInfo *pInfo = dlg.m_arypLinkedObjects.GetAt(i);
						// (c.haag 2009-05-29 12:15) - PLID 34298 - This is the problem link to be deleted
						CEmrProblemLink* pDoomedLink = NULL;
						if(pInfo->bDeleted) {
							//do we have a pointer?
							if(pInfo->pEmrProblemLink) {
								pDoomedLink = pInfo->pEmrProblemLink;
							}
							else if(pInfo->nEMRProblemLinkID != -1) {
								//find by ID
								BOOL bFound = FALSE;
								for(int j=0; j < apAllLinks.GetSize() && !bFound; j++) {
									CEmrProblemLink* pLink = apAllLinks[j];
									if(pLink->GetID() == pInfo->nEMRProblemLinkID) {
										//found it
										bFound = TRUE;
										pDoomedLink = pLink;
									}
								}

								//we always should have found a matching link
								ASSERT(bFound);
							}
							else {
								//this shouldn't be possible, we have to have
								//an ID or a pointer
								ASSERT(FALSE);
							}
						}

						// (c.haag 2009-05-29 12:15) - PLID 34298 - Now flag the doomed link as deleted
						if (NULL != pDoomedLink) {
							pDoomedLink->SetDeleted();
							// (c.haag 2009-05-29 12:08) - PLID 34298 - All of the link deletions take place from
							// within the EMR save query. Not all of those links may be in the EMR, however. One or
							// more of them could belong to an outside lab or another EMR. So, how do those links
							// get recognized by the EMR save query? We use a very ad-hoc solution: Add those links
							// to the EMR. They will never show up anywhere because they are already deleted. When
							// the user is prompted with a list of links to be deleted, these links will also appear
							// (because they are indeed going to be deleted by the big save batch)							
							if (NULL == pDoomedLink->GetEMR()) {
								// (j.jones 2009-06-05 10:21) - PLID 34487 - needs special handling if this is opened by the LabEntryDlg
								//TES 11/25/2009 - PLID 36191 - Changed from m_pdlgLabEntry to m_pdlgLabRequisition
								if(m_pdlgLabRequisition) {
									//AddProblemLink will check to see if the link already exists before adding
									m_pdlgLabRequisition->AddProblemLink(pDoomedLink);
								}
								else {
									//opened through an EMR
									CEMR* pEMR = pSelLink->GetEMR();
									ASSERT(NULL != pEMR);
									pEMR->m_apEmrProblemLinks.Add(pDoomedLink);
									pDoomedLink->UpdatePointersWithEMR(pEMR);
								}

								// (c.haag 2009-06-03 17:14) - PLID 34298 - This problem link is now
								// officially connected to pEMR. If it came from data, then its status
								// has been promoted from a "data link" to a "memory link"; and therefore
								// we must remove it from apLinksFromData. If we don't do this, it will get
								// deleted, and the EMR will be stuck with a dangling pointer.
								for (int l=0; l < apLinksFromData.GetSize(); l++) {
									if (apLinksFromData[l] == pDoomedLink) {
										apLinksFromData.RemoveAt(l);
										break;
									}
								} // for (int l=0; l < apLinksFromData.GetSize(); l++) {
							} // if (NULL == pDoomedLink->GetEMR()) {
						} // if (NULL != pDoomedLink) {
					} // for (i=0; i < dlg.m_arypLinkedObjects.GetSize(); i++) {

					//call the function to update the EMR interface properly to reflect this change
					// (c.haag 2009-05-26 15:49) - PLID 34298 - Do this once per link
					for (i=0; i < apAllLinks.GetSize(); i++) {
						CEmrProblemLink* pLink = apAllLinks[i];
						if (pLink->GetEMR()) {
							// Update the interface. The problem was modified; which could mean maybe
							// the link was deleted, or the problem was changed from opened to closed
							UpdateEMRInterface(m_pMessageWnd, pLink);
						} else {
							// This link is with an EMR object not in memory. Do nothing
						}
					}

					//re-filter the problem list
					RefilterList(FALSE);
				}
			}
		} // if (IDOK == dlg.DoModal()) {

		// (c.haag 2009-05-22 15:08) - PLID 34298 - Cleanup
		for (i=0; i < apLinksFromData.GetSize(); i++) {
			delete apLinksFromData[i];
		}

	}NxCatchAll("Error in CEMRProblemListDlg::EditProblemFromMemory");
}

void CEMRProblemListDlg::EditProblemFromData(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		if(pRow == NULL) {
			return;
		}

		//
		// (c.haag 2006-10-17 12:52) - PLID 23116 - Let the user edit the problem from this dialog
		//

		//
		// (c.haag 2006-11-01 16:06) - PLID 21453 - Check permissions first.
		//
		// Notice we assume the password is known when we check the write permission; the user
		// will be prompted for their password inside the dialog if they had actually changed
		// anything.
		//
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptWrite, FALSE, 0, TRUE, TRUE)) {
			return;
		}

		// (c.haag 2009-05-22 15:01) - PLID 34298 - We now edit problems under
		// the new problem linking structure.
		//
		// In the old implementation, we just passed in information from a single problem object,
		// pulled the changes out, and updated the problem. Now we pass in an array of problem
		// links (one per-problem per-EMR-object).
		//
		const long nProblemID = VarLong(pRow->GetValue(plcID));
		CEmrProblem* pProblem = NULL;
		int i;
		
		// (c.haag 2008-06-16 12:44) - PLID 30319 - We now factor the names of text macro details
		// (j.jones 2008-07-15 17:21) - PLID 30739 - changed to use EMRRegardingType
		// (c.haag 2008-07-25 12:25) - PLID 30727 - Onset date		
		// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
		// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
		// (c.haag 2009-05-12 12:27) - PLID 34234 - EMR problem linking
		// (z.manning 2009-05-27 11:09) - PLID 34297 - Added patient ID
		// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
		// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
		// (s.tullis 2015-02-23 15:44) - PLID 64723
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt
		_RecordsetPtr prs = CreateParamRecordset("SELECT ID, EnteredDate, ModifiedDate, "
			"OnsetDate, StatusID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, Description, EmrProblemActionID, "
			"EmrProblemsT.PatientID, EmrProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, "
			"EmrProblemsT.DoNotShowOnProblemPrompt "
			"FROM EMRProblemsT "
			"WHERE EMRProblemsT.ID = {INT}", nProblemID);
		if (prs->eof) {
			ASSERT(FALSE); // This should never happen!
			MsgBox("This problem could not be found in the database.");
		} else {
			CEMRProblemEditDlg dlg(this);
			CString strCurrentDesc = AdoFldString(prs, "Description");
			long nCurrentStatusID = AdoFldLong(prs, "StatusID");
			// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
			long nCurrentDiagCodeID_ICD9 = AdoFldLong(prs, "DiagCodeID", -1);
			long nCurrentDiagCodeID_ICD10 = AdoFldLong(prs, "DiagCodeID_ICD10", -1);
			long nCurrentChronicityID = AdoFldLong(prs, "ChronicityID", -1);
			long nCurrentCodeID = AdoFldLong(prs, "CodeID", -1); 
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			BOOL bDoNotShowOnCCDA = AdoFldBool(prs, "DoNotShowOnCCDA", FALSE);
			// (r.gonet 2015-03-09 18:21) - PLID 65008 - Load the DoNotShowOnProblemPrompt flag from the recordset.
			BOOL bDoNotShowOnProblemPrompt = AdoFldBool(prs, "DoNotShowOnProblemPrompt", FALSE);
			CString strDescription = VarString(pRow->GetValue(plcDetailName), "");
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleDateTime dtOnsetDate = AdoFldDateTime(prs, "OnsetDate", dtInvalid);
			dlg.SetProblemID( nProblemID );
			dlg.SetProblemStatusID( nCurrentStatusID );
			dlg.SetProblemDiagCodeIDs(nCurrentDiagCodeID_ICD9, nCurrentDiagCodeID_ICD10);
			dlg.SetProblemChronicityID(nCurrentChronicityID);
			dlg.SetProblemDesc( strCurrentDesc );
			dlg.SetPatientID(m_nPatientID);
			// (b.spivey September 24, 2013) - PLID 58677 - set the ID with a mutator. 
			dlg.SetProblemCodeID(nCurrentCodeID);
			// (c.haag 2008-07-25 12:25) - PLID 30727 - Onset date
			dlg.SetOnsetDate( dtOnsetDate );
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			dlg.SetProblemDoNotShowOnCCDA(bDoNotShowOnCCDA);
			// (r.gonet 2015-03-09 18:21) - PLID 65008 - Assign the DoNotShowOnProblemPrompt flag.
			dlg.SetProblemDoNotShowOnProblemPrompt(bDoNotShowOnProblemPrompt);
			dlg.SetWriteToData( TRUE ); // (c.haag 2006-11-13 13:45) - PLID 22052 - The dialog now contains the saving code
			{
				// (a.walling 2008-07-28 15:10) - PLID 30855 - Check for write access
				// (a.walling 2008-08-27 13:29) - PLID 30855 - Use the common function to detect and warn if concurrency issues exist
				// (z.manning 2009-07-01 16:02) - PLID 34765 - Added m_bExcludeCurrentUserFromEmnConcurrencyCheck
				if (WarnIfEMNConcurrencyIssuesExist(this, 
						CEmrProblem::GetEMNQueryFromProblemID(nProblemID, m_bIncludeEmrLevelProblemsInConcurrencyCheck),
						"This problem may viewed, but not modified.",
						m_bExcludeCurrentUserFromEmnConcurrencyCheck)) {					
					dlg.SetReadOnly(TRUE);
				}
			}
			pProblem = new CEmrProblem(prs->Fields);
			prs->Close();

			// (c.haag 2009-05-22 16:29) - PLID 34298 - Now allocate and add problem links
			CArray<CEmrProblemLink*,CEmrProblemLink*> apAllLinks;
			CArray<CEmrProblemLink*,CEmrProblemLink*> apLinksFromData;
			// (j.jones 2009-05-29 12:32) - PLID 34301 - this is now in EmrUtils
			PopulateProblemLinkArrays(pProblem, NULL, apAllLinks, apLinksFromData,
				m_dlProblems, plcProblemLinkPtr, plcProblemLinkID, plcDetailName, plcDetailValue, m_pdlgLabRequisition);
			BOOL bIsReadOnly = FALSE;
			for (i=0; i < apAllLinks.GetSize(); i++) {
				CEmrProblemLink* pLink = apAllLinks[i];

				//find the link in the list by pointer, it likely won't be there but worth a shot
				IRowSettingsPtr pLinkRow = m_dlProblems->FindByColumn(plcProblemLinkPtr, (long)pLink, m_dlProblems->GetFirstRow(), g_cvarFalse);
				if(pLinkRow == NULL) {
					//search by ID
					pLinkRow = m_dlProblems->FindByColumn(plcProblemLinkID, (long)pLink->GetID(), m_dlProblems->GetFirstRow(), g_cvarFalse);
				}

				if(pLinkRow == NULL) {
					ASSERT(FALSE);

					// (c.haag 2009-05-22 15:08) - PLID 34298 - Cleanup
					for (i=0; i < apLinksFromData.GetSize(); i++) {
						delete apLinksFromData[i];
					}
					delete pProblem;

					ThrowNxException("Problem Link not found in list!");
				}

				// (j.jones 2009-05-26 18:25) - PLID 34250 - the detail value field is currently only filled on detail objects
				CString strDetailValue;
				if(pLink->GetType() == eprtEmrDataItem || pLink->GetType() == eprtEmrItem) {
					strDetailValue = pLink->m_strDetailValue;
				}
				dlg.AddLinkedObjectInfo(pLink->GetID(), pLink->GetType(), pLink->m_strDetailName, 
					strDetailValue, pLink->GetRegardingID(), pLink, pLink->GetDataID());
			}
			if (bIsReadOnly) {
				dlg.SetReadOnly(TRUE);
			}
			if (IDOK == dlg.DoModal()) {

				// (c.haag 2006-11-13 13:47) - PLID 22052 - The saving code is now executed in EMRProblemEditDlg.cpp

				// (j.jones 2014-02-24 17:16) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
				long nDiagCode_ICD9 = -1, nDiagCode_ICD10 = -1;
				dlg.GetProblemDiagCodeIDs(nDiagCode_ICD9, nDiagCode_ICD10);

				// Don't do anything if nothing changed
				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				if (strCurrentDesc != dlg.GetProblemDesc() ||
					nCurrentStatusID != dlg.GetProblemStatusID() || 
					nCurrentDiagCodeID_ICD9 != nDiagCode_ICD9 ||
					nCurrentDiagCodeID_ICD10 != nDiagCode_ICD10 ||
					nCurrentChronicityID != dlg.GetProblemChronicityID())
				{
					CString strNewStatus = VarString(GetTableField("EMRProblemStatusT", "Name", "ID", dlg.GetProblemStatusID()), "");

					// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns
					CString strNewICD9Name, strNewICD10Name;
					GetICD9And10Codes(nDiagCode_ICD9, nDiagCode_ICD10, strNewICD9Name, strNewICD10Name);
					if(strNewICD9Name.IsEmpty()) {
						strNewICD9Name = "<None>";
					}
					if(strNewICD10Name.IsEmpty()) {
						strNewICD10Name = "<None>";
					}

					CString strNewChronicity = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", dlg.GetProblemChronicityID()), "");

					// Update the problem list view
					pRow->PutValue(plcStatusName, (LPCTSTR)strNewStatus);
					// (j.jones 2014-02-26 11:23) - PLID 60763 - ICD-9 and 10 are now two columns
					pRow->PutValue(plcDiagCodeName_ICD9, (LPCTSTR)strNewICD9Name);
					pRow->PutValue(plcDiagCodeName_ICD10, (LPCTSTR)strNewICD10Name);
					pRow->PutValue(plcChronicityName, (LPCTSTR)strNewChronicity);
					pRow->PutValue(plcDescription, (LPCTSTR)dlg.GetProblemDesc());
					// (c.haag 2008-07-25 11:29) - PLID 30727 - Update the onset date
					if (dlg.GetOnsetDate().GetStatus() == COleDateTime::valid) {
						pRow->PutValue(plcOnsetDate, _variant_t(dlg.GetOnsetDate(), VT_DATE));
					} else {
						_variant_t vNull;
						vNull.vt = VT_NULL;
						pRow->PutValue(plcOnsetDate, vNull);
					}

					// (j.jones 2014-02-26 13:57) - PLID 60763 - diagnosis codes now can dynamically
					// resize their columns based on content, so we need to try a resize now
					ResizeColumns();

					// Update the history view
					RequeryHistory(nProblemID);
				}
			} // if (IDOK == dlg.DoModal()) {

			// (c.haag 2009-05-22 15:08) - PLID 34298 - Cleanup
			for (i=0; i < apLinksFromData.GetSize(); i++) {
				delete apLinksFromData[i];
			}
			delete pProblem;

		} // if (prs->eof) {

	}NxCatchAll("Error in CEMRProblemListDlg::EditProblemFromData");
}

void CEMRProblemListDlg::OnBtnDeleteProblem()
{
	long nAuditTransactionID = -1;

	try {
		// (a.walling 2007-11-28 10:32) - PLID 28044
		if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) return;
		//
		// (c.haag 2006-10-19 11:27) - PLID 21454 - We now allow users to delete problems
		//
		IRowSettingsPtr pRow = m_dlProblems->CurSel;
		if (NULL != pRow) {
			//
			// (c.haag 2006-11-01 16:58) - PLID 21453 - Fail if they do not have delete permission
			//
			if (!CanCurrentUserDeleteEmrProblems()) {
				// (c.haag 2009-09-09 11:37) - PLID 31209 - Tell the user they can't manually delete the problem
				MsgBox(MB_OK | MB_ICONSTOP, "You do not have permission to delete EMR problems.");
				return;
			}

			// (j.jones 2008-07-21 14:33) - PLID 30773 - Check and see if we have a problem ptr.,
			// and if so, we will edit that pointer, and do not commit the changes to data.
			// Also we will commit to data if the EMN is locked.
			CEmrProblem *pProblem = (CEmrProblem*)VarLong(pRow->GetValue(plcProblemPtr), 0);

			// (c.haag 2006-11-13 14:56) - PLID 22052 - If the EMN is locked, we can't edit it. Therefore,
			// any changes made to the problem cannot be pooled in with the mass EMN update query. What
			// we must instead do is save the changes on the fly from the dialog itself
			// (j.jones 2009-05-28 17:26) - PLID 34301 - we don't need to handle this anymore, if we're
			// inside an EMR, the EMR level (not EMN level) will do all the saving

			// (j.jones 2009-05-28 16:53) - PLID 34301 - have a different warning if there are multiple problem links
			CString strWarning = "Are you sure you wish to delete the selected problem?";
			CArray<CEmrProblemLink*,CEmrProblemLink*> apAllLinks;
			CArray<CEmrProblemLink*,CEmrProblemLink*> apLinksFromData;
			CEmrProblem *pTempProblemToDelete = NULL;
			long nProblemID = VarLong(pRow->GetValue(plcID));
			if(pProblem) {
				CEMR *pEMR = NULL;

				//see if we have a link pointer
				CEmrProblemLink* pSelLink = (CEmrProblemLink*)VarLong(pRow->GetValue(plcProblemLinkPtr), 0);

				if(pSelLink != NULL && pSelLink->GetEMR()) {
					pEMR = pSelLink->GetEMR();
				}

				// (j.jones 2009-05-29 12:32) - PLID 34301 - this is now in EmrUtils
				PopulateProblemLinkArrays(pProblem, pEMR, apAllLinks, apLinksFromData,
					m_dlProblems, plcProblemLinkPtr, plcProblemLinkID, plcDetailName, plcDetailValue, m_pdlgLabRequisition);

				int i=0;
				for (i=0; i < apAllLinks.GetSize(); i++) {
					CEmrProblemLink* pCurLink = apAllLinks[i];
					if (pCurLink->GetEMN()) {

						if (!(pCurLink->GetEMN()->IsWritable())) {
							AfxMessageBox("You cannot delete this problem because you do not have access to change an EMN it is linked to.");

							// (j.jones 2009-05-28 17:14) - PLID 34301 - clear the pointers we created
							for(int j=0; j < apLinksFromData.GetSize(); j++) {
								delete apLinksFromData[j];
							}
							return;
						}
					}
				}
			}
			else {
				//make a temporary, fake problem
				// (j.jones 2014-02-24 15:44) - PLID 61019 - EMR problems now have ICD-9 and 10 IDs
				// (s.tullis 2015-03-11 10:15) - PLID 64723 - Added DoNotshowonCCDA
				// (r.gonet 2015-03-09 19:24) - PLID 64723 - Added DoNotShowOnProblemPrompt.
				_RecordsetPtr prs = CreateParamRecordset("SELECT ID, EnteredDate, ModifiedDate, "
					"OnsetDate, StatusID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, Description, EmrProblemActionID, "
					"CodeID, "
					"EmrProblemsT.PatientID, "
					"EmrProblemsT.DoNotShowOnCCDA, "
					"EmrProblemsT.DoNotShowOnProblemPrompt "
					"FROM EMRProblemsT "
					"WHERE EMRProblemsT.ID = {INT}", nProblemID);
				if (prs->eof) {
					ASSERT(FALSE); // This should never happen!
					ThrowNxException("This problem could not be found in the database (%li).", nProblemID);
				} else {
					pTempProblemToDelete = new CEmrProblem(prs->Fields);
				}
				prs->Close();

				if(pTempProblemToDelete == NULL) {
					ThrowNxException("This problem could not be loaded!");
				}

				// (j.jones 2009-05-29 12:32) - PLID 34301 - this is now in EmrUtils
				PopulateProblemLinkArrays(pTempProblemToDelete, NULL, apAllLinks, apLinksFromData,
					m_dlProblems, plcProblemLinkPtr, plcProblemLinkID, plcDetailName, plcDetailValue, m_pdlgLabRequisition);
			}

			// (z.manning 2009-07-01 16:08) - PLID 34765 - Added m_bExcludeCurrentUserFromEmnConcurrencyCheck
			if(WarnIfEMNConcurrencyIssuesExist(this, 
					CEmrProblem::GetEMNQueryFromProblemID(nProblemID, m_bIncludeEmrLevelProblemsInConcurrencyCheck),
					"This problem may not be deleted.",
					m_bExcludeCurrentUserFromEmnConcurrencyCheck)) {	

					// (j.jones 2009-05-28 17:14) - PLID 34301 - clear the pointers we created
					for(int i=0; i < apLinksFromData.GetSize(); i++) {
						delete apLinksFromData[i];
					}
					if(pTempProblemToDelete) {
						delete pTempProblemToDelete;
					}
					return;
			}

			if(apAllLinks.GetSize() > 1) {
				strWarning.Format("There are %li objects linked to this problem. Are you sure you wish to delete the selected problem?", apAllLinks.GetSize());
			}

			if (IDNO == MsgBox(MB_YESNO | MB_ICONEXCLAMATION, strWarning)) {

				// (j.jones 2009-05-28 17:14) - PLID 34301 - clear the pointers we created
				for (int i=0; i < apLinksFromData.GetSize(); i++) {
					delete apLinksFromData[i];
				}
				if(pTempProblemToDelete) {
					delete pTempProblemToDelete;
				}
				return;
			}
			
			if(pProblem == NULL) {
				//if no problem pointer, save immediately

				// (a.walling 2008-08-27 13:29) - PLID 30855 - Use the common function to detect and warn if concurrency issues exist
				// (z.manning 2009-07-01 16:09) - PLID 34765 - Added m_bExcludeCurrentUserFromEmnConcurrencyCheck
				if (WarnIfEMNConcurrencyIssuesExist(this, 
						CEmrProblem::GetEMNQueryFromProblemID(nProblemID, m_bIncludeEmrLevelProblemsInConcurrencyCheck), 
						"This problem may not be deleted.",
						m_bExcludeCurrentUserFromEmnConcurrencyCheck)) {

					// (j.jones 2009-05-28 17:14) - PLID 34301 - clear the pointers we created
					for (int i=0; i < apLinksFromData.GetSize(); i++) {
						delete apLinksFromData[i];
					}
					if(pTempProblemToDelete) {
						delete pTempProblemToDelete;
					}
					return;
				}

				CString strSqlBatch = BeginSqlBatch();
				// (a.walling 2008-08-26 12:36) - PLID 30855 - Ensure no one is holding this EMN currently
				// (j.armen 2013-05-14 11:56) - PLID 56680 - Use the LoginTokenID for user verification
				AddStatementToSqlBatch(strSqlBatch, 
					"IF EXISTS(\r\n"
					"	SELECT 1\r\n"
					"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
					"	WHERE EmnID IN (%s) AND UserLoginTokenID <> %li)\r\n"
					"BEGIN\r\n"
					"	RAISERROR('Problem cannot be deleted; the EMN is being modified by another user.', 16, 43)\r\n"
					"	ROLLBACK TRAN\r\n"
					"	RETURN\r\n"
					"END\r\n",
					CEmrProblem::GetEMNQueryFromProblemID(nProblemID, FALSE).Flatten(),
					GetAPIUserLoginTokenID());

				//(e.lally 2008-04-09) PLID 29359 - Added an _Q() to username
				AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRProblemsT SET Deleted = 1, DeletedBy = '%s', DeletedDate = GetDate() WHERE ID = %d", _Q(GetCurrentUserName()), nProblemID);				

				// (j.gruber 2008-09-17 13:25) - PLID 31396 - fixed bug where this wasn't audting because it was sending -1 for the auditID
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),nAuditTransactionID,aeiEMNProblemDeleted,nProblemID,"","<Deleted>",aepHigh,aetDeleted);

				// (j.jones 2009-05-28 17:16) - PLID 34301 - delete all the links
				for(int i=0; i < apAllLinks.GetSize(); i++) {
						
					CEmrProblemLink *pLink = apAllLinks.GetAt(i);
					if(pLink) {

						long nEMRProblemLinkID = pLink->GetID();
						if(nEMRProblemLinkID != -1) {
							//delete this link
							AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrProblemLinkT WHERE ID = %li", nEMRProblemLinkID);

							//audit this
							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							CString strNewValue;
							// (z.manning 2009-05-27 09:37) - PLID 34297 - Moved this code to a utility function
							CString strOwnerType = GetProblemTypeDescription(pLink->GetType());

							CString strName, strValue;
							pLink->GetDetailNameAndValue(strName, strValue);

							strNewValue.Format("Unlinked from %s: %s%s%s", strOwnerType, strName, strValue.IsEmpty() ? "" : " - ", strValue);
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditTransactionID, aeiEMNProblemLinkDeleted, nProblemID, pTempProblemToDelete->m_strDescription, strNewValue, aepHigh, aetChanged);
						}
					}
				}

				ExecuteSqlBatch(strSqlBatch);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}

				//TES 6/3/2009 - PLID 34371 - Update the Patient Wellness qualifications for this patient.
				UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), m_nPatientID);
				// (c.haag 2010-09-21 11:35) - PLID 40612 - Do not check clinical decision rules when deleting objects.
				//TodoCreateForDecisionRules(GetRemoteData(), m_nPatientID);

				// (c.haag 2006-11-13 15:11) - PLID 23158 - Now that everything is done,
				// we must send a table checker to let everyone know the problem was deleted
				// (j.jones 2008-07-23 16:22) - PLID 30823 - send the patient ID, not the problem ID
				CClient::RefreshTable(NetUtils::EMRProblemsT, m_nPatientID);
			}
			
			if(pProblem != NULL) {
				
				//regardless of the lock status, mark the problem as deleted
				pProblem->m_bIsDeleted = TRUE;
				
				pProblem->m_bIsModified = TRUE;

				// (j.jones 2009-05-28 17:16) - PLID 34301 - delete all the links
				for(int i=0; i < apAllLinks.GetSize(); i++) {
					
					//the saving code will be responsible for deleting links
					//that are not actually loaded in memory, but here we will
					//flag all the links we've got
					CEmrProblemLink *pLink = apAllLinks.GetAt(i);
					if(pLink) {
						pLink->SetDeleted();
					}

					//call the function to update the EMR interface properly to reflect this change
					UpdateEMRInterface(m_pMessageWnd, pLink);
				}
			}

			// Now update the view
			// (j.jones 2009-05-28 17:53) - PLID 34301 - remove all rows with this problem ID
			if(nProblemID != -1) {
				IRowSettingsPtr pRowToRemove = m_dlProblems->FindByColumn(plcID, nProblemID, m_dlProblems->GetFirstRow(), FALSE);
				while(pRowToRemove) {
					m_dlProblems->RemoveRow(pRowToRemove);
					pRowToRemove = m_dlProblems->FindByColumn(plcID, nProblemID, m_dlProblems->GetFirstRow(), FALSE);
				}
			}
			else if(pProblem) {
				// (j.jones 2009-05-28 17:53) - PLID 34301 - remove all rows with this problem pointer
				IRowSettingsPtr pRowToRemove = m_dlProblems->FindByColumn(plcProblemPtr, (long)pProblem, m_dlProblems->GetFirstRow(), FALSE);
				while(pRowToRemove) {
					m_dlProblems->RemoveRow(pRowToRemove);
					pRowToRemove = m_dlProblems->FindByColumn(plcProblemPtr, (long)pProblem, m_dlProblems->GetFirstRow(), FALSE);
				}
			}

			// (c.haag 2008-08-04 15:32) - PLID 30947 - If there is no active selection, disable the buttons and
			// clear the history. Otherwise, leave the buttons alone and requery the history list.
			if (NULL == m_dlProblems->CurSel) {
				GetDlgItem(IDC_BTN_EDIT_PROBLEM)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_DELETE_PROBLEM)->EnableWindow(FALSE);
				m_dlHistory->Clear();
				UpdateHistoryListDiagnosisColumns();
			}
			else {
				if (VarLong(m_dlProblems->CurSel->Value[plcID]) > -1) {
					RequeryHistory(m_dlProblems->CurSel->Value[plcID]);
				} else {
					m_dlHistory->Clear();
					UpdateHistoryListDiagnosisColumns();
				}
			}

			// (j.jones 2014-02-26 13:57) - PLID 60763 - diagnosis codes now can dynamically
			// resize their columns based on content, so we need to try a resize now
			ResizeColumns();

			// (j.jones 2009-05-28 17:14) - PLID 34301 - clear the pointers we created
			for (int i=0; i < apLinksFromData.GetSize(); i++) {
				delete apLinksFromData[i];
			}
			if(pTempProblemToDelete) {
				delete pTempProblemToDelete;
			}
		}
	}
	NxCatchAllCall("Error in CEMRProblemListDlg::OnBtnDeleteProblem",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		});
}

void CEMRProblemListDlg::OnSelChosenProblemStatusFilter(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_StatusCombo->SetSelByColumn(sccID, (long)pssfAll);
		}

		RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnSelChosenProblemStatusFilter");
}

void CEMRProblemListDlg::OnSelChosenProblemObjectTypeFilter(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_TypeCombo->SetSelByColumn(tccID, (long)eprtInvalid);
		}

		RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnSelChosenProblemObjectTypeFilter");
}

void CEMRProblemListDlg::OnSelChosenProblemDateFilter(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_DateCombo->SetSelByColumn(dccID, (long)dftAll);
		}

		if(pRow == NULL || VarLong(pRow->GetValue(dccID), dftAll) == dftAll) {
			GetDlgItem(IDC_PROBLEM_DATE_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_PROBLEM_DATE_TO)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_PROBLEM_DATE_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_PROBLEM_DATE_TO)->EnableWindow(TRUE);
		}

		RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnSelChosenProblemDateFilter");
}

void CEMRProblemListDlg::OnCloseupProblemFromDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		*pResult = 0;

		// (j.jones 2008-08-20 10:36) - PLID 30773 - this is not needed because
		// we also check OnDatetimechange
		//RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnCloseupProblemFromDate");
}

void CEMRProblemListDlg::OnCloseupProblemToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		*pResult = 0;

		// (j.jones 2008-08-20 10:36) - PLID 30773 - this is not needed because
		// we also check OnDatetimechange
		//RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnCloseupProblemToDate");
}

void CEMRProblemListDlg::OnDatetimechangeProblemFromDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		*pResult = 0;

		RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnDatetimechangeProblemFromDate");
}

void CEMRProblemListDlg::OnDatetimechangeProblemToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		*pResult = 0;

		RefilterList();

	}NxCatchAll("Error in CEMRProblemListDlg::OnDatetimechangeProblemToDate");
}


//tries to kill the active PopulateDetailValuesThread thread
void CEMRProblemListDlg::KillThread()
{
	try {

		if (m_pPopulateDetailValuesThread) {
			// Wait for the thread to terminate
			WaitForSingleObject(m_pPopulateDetailValuesThread->m_hThread, 2000);
			// Get the exit code
			DWORD dwExitCode = 0;
			::GetExitCodeThread(m_pPopulateDetailValuesThread->m_hThread, &dwExitCode);
			// See if the thread is still active
			if (dwExitCode == STILL_ACTIVE) {
				// The thread is still going so post a quit message to it and let it delete itself
				// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
				m_pPopulateDetailValuesThread->m_bAutoDelete = TRUE;

				PostThreadMessage(m_pPopulateDetailValuesThread->m_nThreadID, WM_QUIT, 0, 0);
				if (WAIT_TIMEOUT == WaitForSingleObject(m_pPopulateDetailValuesThread->m_hThread, 2000)) {
					TerminateThread(m_pPopulateDetailValuesThread->m_hThread, 0);
				}
			} else {
				// delete the thread object. We don't delete if it is running, which means autodelete will be set to true.
				delete m_pPopulateDetailValuesThread;
			}
			m_pPopulateDetailValuesThread = NULL;

			// (j.jones 2008-10-30 16:54) - PLID 31869 - clear out the stored thread ID
			if(GetMainFrame()) {
				GetMainFrame()->m_nProblemListDlg_PopulateDetailValues_CurThreadID = -1;
			}
			else {
				//should be impossible
				ASSERT(FALSE);
			}
		}

	}NxCatchAll("Error in CEMRProblemListDlg::KillThread");
}

void CEMRProblemListDlg::OnOK() 
{
	try {

		// (j.jones 2008-07-21 15:09) - PLID 30730 - handle closing differently
		// if we are modeless or not
		if(m_bIsOwnedByMainframe) {
			GetMainFrame()->PostMessage(NXM_EMR_PROBLEM_LIST_CLOSED);
		}
		else {
			CNxDialog::OnOK();
		}
	
	}NxCatchAll("Error in CEMRProblemListDlg::OnOK");
}

void CEMRProblemListDlg::OnCancel() 
{
	try {

		// (j.jones 2008-07-21 15:09) - PLID 30730 - handle closing differently
		// if we are modeless or not
		if(m_bIsOwnedByMainframe) {
			GetMainFrame()->PostMessage(NXM_EMR_PROBLEM_LIST_CLOSED);
		}
		else {
			CNxDialog::OnCancel();
		}
	
	}NxCatchAll("Error in CEMRProblemListDlg::OnCancel");
}

// (j.jones 2008-07-17 09:10) - PLID 30730 - added OnTableChanged
LRESULT CEMRProblemListDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {

			// (j.jones 2008-11-20 16:29) - PLID 32119 - I added a problem status tablechecker,
			// but we're already checking this status with a member tablechecker,
			// so requery, reselect the current value, refilter the list if the current value
			// no longer exists, and be sure to reset our member tablechecker.
			case NetUtils::EMRProblemStatusT:
				{
					//reset our member tablechecker
					m_ProblemStatusChecker.Changed();
					
					RequeryStatusFilter(TRUE);

					break;
				}
				
			case NetUtils::EMRProblemsT:
			{
				// (j.jones 2008-07-23 16:22) - PLID 30823 - we now get the patient ID
				// as the lParam, making this code simple
				// (j.jones 2014-08-26 09:13) - PLID 63223 - -1 should never be sent,
				// so we should not check for it
				if(lParam == m_nPatientID) {
					RefilterList(TRUE);
					Flash();
				}
			
				break;
			}

			case NetUtils::PatCombo:
			{
				//only pay attention to messages where lParam is our patient ID,
				//do not interpret ones for all patients, which should really only
				//be when an import occurs
				if(lParam == m_nPatientID) {
					UpdateProblemListWindowText();

					Flash();
				}

				break;
			}

			case NetUtils::EMRMasterT:
			{
				//EMRMasterT tablecheckers send the patient ID
				// (j.jones 2014-08-26 09:13) - PLID 63223 - -1 should never be sent,
				// so we should not check for it
				if(lParam == m_nPatientID) {
					RefilterList(TRUE);
					Flash();
				}

				break;
			}

			// (j.jones 2014-08-26 09:21) - PLID 63223 - there is no need to respond to EmrInfoMasterT messages
			/*
			case NetUtils::EmrInfoMasterT:
			{
				if(lParam != -1) {
					//if we have an ID, see if it is for an item on any EMN for this patient
					_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 EMRMasterT.ID "
						"FROM EMRMasterT "
						"INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
						"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
						"WHERE EMRInfoT.EMRInfoMasterID = {INT} AND EMRMasterT.PatientID = {INT}",
						lParam, m_nPatientID);
					if(!rs->eof) {
						//it is on an EMN for this patient
						RefilterList(TRUE);
						Flash();
					}
					rs->Close();
				}

				break;
			}
			*/
		}

	} NxCatchAll("Error in CEMRProblemListDlg::OnTableChanged");

	return 0;
}

//TES 8/13/2014 - PLID 63519 - Added support for EX tablecheckers
LRESULT CEMRProblemListDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {
		case NetUtils::PatCombo:
			{
				//TES 8/13/2014 - PLID 63519 - Just handle the same way as we do regular tablecheckers
				//only pay attention to messages where lParam is our patient ID,
				//do not interpret ones for all patients, which should really only
				//be when an import occurs
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);
				if (nPatientID == m_nPatientID) {
					UpdateProblemListWindowText();

					Flash();
				}

				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (c.haag 2009-02-09 15:04) - PLID 32976 - Utility wrapper for clearing the problem list
void CEMRProblemListDlg::ClearProblemList()
{
	if (NULL != m_dlProblems) {
		// The thread is moot at this point. Stop it if it's running.
		KillThread();
		// Clear the problem list
		m_dlProblems->Clear();
	}
}

// (j.jones 2008-07-17 11:07) - PLID 30730 - used to flash the window when minimized or inactive
void CEMRProblemListDlg::Flash()
{
	try {

		HWND hwndActive = ::GetActiveWindow();
		HWND hwndDlg = this->GetSafeHwnd();
		if (hwndActive == hwndDlg) {
			return; // do not flash if we are the active window
		}

		//use FlashWindow instead, it flashes once and leaves the task button inverted,
		//which is 1. not annoying, and 2. what I have witnessed other programs do
		FlashWindow(TRUE);

	}NxCatchAll("Error in CEMRProblemListDlg::Flash");
}

// (j.jones 2008-07-17 16:28) - PLID 30730 - added function to set the window text
void CEMRProblemListDlg::UpdateProblemListWindowText()
{
	try {

		CString str;
		str.Format("Problem List For %s", GetExistingPatientName(m_nPatientID));

		//if there's a specific type, say so
		if(m_eprtDefaultRegardingType != eprtInvalid) {
			
			CString strType;

			if(m_eprtDefaultRegardingType == eprtEmrItem) {
				strType = " - EMR Item";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrDataItem) {
				strType = " - EMR List Item";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrTopic) {
				strType = " - EMR Topic";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMN) {
				strType = " - EMN";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrEMR) {
				strType = " - EMR";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrDiag) {
				strType = " - EMR Diagnosis Code";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrCharge) {
				strType = " - EMR Charge";
			}
			else if(m_eprtDefaultRegardingType == eprtEmrMedication) {
				strType = " - EMR Medication";
			}
			// (z.manning 2009-05-28 09:54) - PLID 34345
			else if(m_eprtDefaultRegardingType == eprtLab) {
				strType = " - Lab";
			}

			str += strType;

			if(!m_strRegardingDesc.IsEmpty()) {
				str += ": ";
				str += m_strRegardingDesc;
			}
		}

		SetWindowText(str);

	}NxCatchAll("Error in CEMRProblemListDlg::UpdateProblemListWindowText");
}

void CEMRProblemListDlg::OnPrintPreview() 
{
	//(e.lally 2008-07-23) PLID 30732 - Add print preview button
	try{

		//(e.lally 2008-07-28) PLID 30732 - If we are in the EMR editor, and have at least one problem,
			//check to see if we need to force a save so our report can fully query the data.
		// (c.haag 2009-05-21 14:57) - PLID 34298 - We now go through an array of problem links
		CString strWhere;		
		if(m_papEMRProblemLinks && m_papEMRProblemLinks->GetSize() > 0){

			// (c.haag 2010-06-18 08:39) - PLID 39208 - Quit right away if there are no problems in the list.
			// While there may be problem links in the array, they may all be deleted or filtered out. If we don't
			// do this, then strID's below will be an empty string, and will be used to create a malformed query
			// that wouldn't give us results anyway.
			if (0 == m_dlProblems->GetRowCount())
			{
				AfxMessageBox("There are no problems to report in the list.", MB_OK | MB_ICONHAND);
				return;
			}

			for(int i=0; i<m_papEMRProblemLinks->GetSize(); i++) {
				CEmrProblemLink* pLink = m_papEMRProblemLinks->GetAt(i);
				if(pLink == NULL || pLink->IsDeleted()) {
					continue;
				}
				CEmrProblem* pProblem = pLink->GetProblem();
				if(pProblem == NULL || pProblem->m_bIsDeleted) {
					continue;
				}
				if(pLink->GetEMR() != NULL){
					//Make sure the EMR is saved.
					if(pLink->GetEMR()->IsEMRUnsaved()) {
						if(IDYES != MsgBox(MB_YESNO, "Before previewing the Problem List, the changes you have made to the EMR must be saved.  Would you like to continue?")) {
							return;
						}
						EmrSaveStatus essResult = (EmrSaveStatus)m_pMessageWnd->SendMessage(NXM_EMR_SAVE_ALL, TRUE, NULL);
						if(FAILED(essResult)){
							//Do we need any special messages that it failed? We should return here to stop the report from running.
							ASSERT(FALSE); 
							return;
						}
					}
					//Once we've saved the EMR, we don't have to check any more problems in the array.
					break;
				}
			}
			//Since the problem list was loaded from memory, we need to filter from memory using the problem pointer IDs which after a
			//save should all be valid.
			IRowSettingsPtr pRow = m_dlProblems->GetFirstRow();
			CString strIDs, strTemp;
			CEmrProblem *pProblem = NULL;
			while(pRow != NULL){
				//for all rows in the problem list
				pProblem = (CEmrProblem*)VarLong(pRow->GetValue(plcProblemPtr));
				strTemp.Format("%li,", pProblem->m_nID);
				strIDs += strTemp;
				pRow = pRow->GetNextRow();
			}
			strIDs.TrimRight(",");
			strWhere.Format(" AND ProblemsQ.ProblemID IN(%s) ", strIDs);
		}
		else{
			//The problem list was loaded from data, so use the where clauses to filter on
			//check the status filter
			{
				CString strStatusFilter = GetWhereClauseFromStatusFilter();
				if(!strStatusFilter.IsEmpty()){
					strWhere += " AND " + strStatusFilter;
				}

			}

			//check the type filter
			{
				CString strTypeFilter = GetWhereClauseFromTypeFilter();
				if(!strTypeFilter.IsEmpty()){
					strWhere += " AND " + strTypeFilter;
				}
			}

			//check the date filter
			{
				CString strDateFilter = GetWhereClauseFromDateFilter();
				if(!strDateFilter.IsEmpty()){
					strWhere += " AND " + strDateFilter;
				}
			}
		}

		// (z.manning 2009-07-17 19:22) - PLID 34345 - We need to close the lab entry dialog if it's open.
		BOOL bCloseLabEntryDlg = FALSE;
		//TES 11/25/2009 - PLID 36191 - Changed from a CLabEntryDlg to a CLabRequisitionDlg
		if(m_pdlgLabRequisition != NULL && IsWindow(m_pdlgLabRequisition->GetSafeHwnd()))
		{
			int nResult = MessageBox("You must save and close the lab before continuing. Would you like to do so now?", NULL, MB_YESNO|MB_ICONQUESTION);
			if(nResult != IDYES) {
				return;
			}

			ShowWindow(SW_SHOWMINIMIZED);
			if(m_pdlgLabRequisition->Save()) {
				// (z.manning 2009-07-17 19:22) - PLID 34345 - Save was successful, so make sure we close it later.
				bCloseLabEntryDlg = TRUE;
			}
			else {
				// (z.manning 2009-07-17 19:24) - PLID 34345 - Failed to save lab, so go back to that dialog.
				EndDialog(IDCANCEL);
				return;
			}
			ShowWindow(SW_RESTORE);
		}

		CPtrArray aryParams;
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(652)]);

		//Add parameters for suppressing certain fields.
		// (a.walling 2008-10-02 09:19) - PLID 31564 - Explicitly cast the BOOL as a char
		CRParameterInfo* pParam = new CRParameterInfo;
		pParam->m_Name = "ShowEMRName";
		pParam->m_Data = (char)m_bShowEMRName;
		aryParams.Add((void*)pParam);

		pParam = new CRParameterInfo;
		pParam->m_Name = "ShowEMNName";
		pParam->m_Data = (char)m_bShowEMNName;
		aryParams.Add((void*)pParam);

		pParam = new CRParameterInfo;
		pParam->m_Name = "ShowTopicName";
		pParam->m_Data = (char)m_bShowTopicName;
		aryParams.Add((void*)pParam);

		pParam = new CRParameterInfo;
		pParam->m_Name = "ShowDetailName";
		pParam->m_Data = (char)m_bShowDetailName;
		aryParams.Add((void*)pParam);

		pParam = new CRParameterInfo;
		pParam->m_Name = "ShowProblemDesc";
		pParam->m_Data = (char)m_bShowProblemDesc;
		aryParams.Add((void*)pParam);


		//set the date filter parameter fields
		{

			IRowSettingsPtr pRow = m_DateCombo->GetCurSel();
			if(pRow) {

				DateFilterTypes dftDateID = (DateFilterTypes)VarLong(pRow->GetValue(dccID), dftAll);
				if(dftDateID != dftAll) {

					COleDateTime dtFrom = m_dtFrom.GetValue();
					COleDateTime dtTo = m_dtTo.GetValue();

					pParam = new CRParameterInfo;
					pParam->m_Name = "DateFilterType";
					pParam->m_Data = VarString(pRow->GetValue(dccName), "Date");
					aryParams.Add((void*)pParam);

					pParam = new CRParameterInfo;
					pParam->m_Name = "DateTo";
					pParam->m_Data = FormatDateTimeForInterface(dtTo, NULL, dtoDate);
					aryParams.Add((void*)pParam);

					pParam = new CRParameterInfo;
					pParam->m_Name = "DateFrom";
					pParam->m_Data = FormatDateTimeForInterface(dtFrom, NULL, dtoDate);
					aryParams.Add((void*)pParam);
				}
				else{
					pParam = new CRParameterInfo;
					pParam->m_Name = "DateFilterType";
					pParam->m_Data = "All Dates";
					aryParams.Add((void*)pParam);

					pParam = new CRParameterInfo;
					pParam->m_Name = "DateTo";
					pParam->m_Data = "12/31/5000";
					aryParams.Add((void*)pParam);

					pParam = new CRParameterInfo;
					pParam->m_Name = "DateFrom";
					pParam->m_Data = "01/01/1000";
					aryParams.Add((void*)pParam);
				}
			}
		}

		//Be sure to filter on the patient connected to these problems
		{
			//(e.lally 2008-07-30) PLID 30732 - The preview report now has to use ProblemsQ to stay compatible with shared field
			//field references in the dialog.
			CString strPatient;
			strPatient.Format(" AND ProblemsQ.PatID = %li", m_nPatientID);
			strWhere += strPatient;
		}
		

		//Use the strExtraText for the rest of our where clause
		infReport.strExtraText = strWhere;

		RunReport(&infReport, &aryParams, TRUE, (CWnd *)this, "Problem List Preview", NULL);
		ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done

		//(e.lally 2008-07-30) PLID 30732 - If we have a window to send messages to (we opened from within an EMR), tell it to minimize
		//so we can see the report.
		if(m_pMessageWnd) {
			m_pMessageWnd->SendMessage(NXM_EMR_MINIMIZE_PIC);
		}

		//Now that the report is up, we can close the problem list.
		OnOK();

		// (z.manning 2009-07-17 19:13) - PLID 34345 - Close the lab entry dialog if necessary
		if(bCloseLabEntryDlg && m_pdlgLabRequisition != NULL) {
			//TES 11/25/2009 - PLID 36191 - Tell the requisition dialog to close, it will pass the word on to its parent.
			m_pdlgLabRequisition->Close();
		}

	}NxCatchAll("CEMRProblemListDlg::OnPrintPreview - Error previewing EMR Problem List");
	
}

// (j.jones 2008-11-20 17:05) - PLID 32119 - safely requeries the status filter
void CEMRProblemListDlg::RequeryStatusFilter(BOOL bRefilterList)
{
	try {

		long nCurStatusID = 0; //0 is an invalid value, -1 is pssfAll
		{
			IRowSettingsPtr pRow = m_StatusCombo->GetCurSel();
			if(pRow) {
				nCurStatusID = VarLong(pRow->GetValue(sccID), (long)pssfAll);
			}
		}
		
		m_StatusCombo->Requery();

		IRowSettingsPtr pAllRow = m_StatusCombo->GetNewRow();
		pAllRow->PutValue(sccID, (long)pssfAll);
		pAllRow->PutValue(sccName, _bstr_t(" < Show All >"));
		pAllRow->PutValue(sccResolved, g_cvarNull);
		m_StatusCombo->AddRowSorted(pAllRow, NULL);

		// (j.jones 2008-11-20 17:37) - PLID 28497 - added options for Unresolved/Resolved
		IRowSettingsPtr pResolvedRow = m_StatusCombo->GetNewRow();
		pResolvedRow->PutValue(sccID, (long)pssfResolved);
		pResolvedRow->PutValue(sccName, _bstr_t(" < Show Resolved >"));
		pResolvedRow->PutValue(sccResolved, g_cvarNull);
		m_StatusCombo->AddRowSorted(pResolvedRow, NULL);

		IRowSettingsPtr pUnresolvedRow = m_StatusCombo->GetNewRow();
		pUnresolvedRow->PutValue(sccID, (long)pssfUnresolved);
		pUnresolvedRow->PutValue(sccName, _bstr_t(" < Show Unresolved >"));
		pUnresolvedRow->PutValue(sccResolved, g_cvarNull);
		m_StatusCombo->AddRowSorted(pUnresolvedRow, NULL);

		IRowSettingsPtr pRow = m_StatusCombo->SetSelByColumn(sccID, nCurStatusID);
		if(pRow == NULL) {
			//we removed the row we were filtering on
			m_StatusCombo->PutCurSel(pAllRow);
		}

		//refilter, so the problem statuses update,
		//but if we were called from RefilterList, it's not necessary
		if(bRefilterList) {
			RefilterList(TRUE);
			Flash();
		}

	}NxCatchAll("Error in CEMRProblemListDlg::RequeryStatusFilter");
}

// (c.haag 2009-02-09 15:22) - PLID 32976 - Called when the thread is relaying detail sentence information to the dialog
LRESULT CEMRProblemListDlg::OnEMRProblemListDetailFromThread(WPARAM wParam, LPARAM lParam)
{
	try {
		ProblemListThreadValue* pPLTV = (ProblemListThreadValue*)wParam;
		// Find a row in the list with the ID, and assign it the sentence information
		// (j.jones 2009-05-28 14:00) - PLID 34298 - supported problem links
		IRowSettingsPtr pRow = m_dlProblems->FindByColumn(plcProblemLinkID, pPLTV->nProblemLinkID, NULL, VARIANT_FALSE);
		if (NULL != pRow) {
			pRow->PutValue(plcDetailValue, (LPCTSTR)pPLTV->strDetailValue);
		} else {
			// The list may have been cleared, or the row removed
		}
		delete pPLTV;
	}
	NxCatchAll("Error in CEMRProblemListDlg::OnEMRProblemListDetailFromThread");
	return 0;
}

// (a.walling 2009-05-04 12:18) - PLID 28495
// (j.jones 2014-02-26 11:23) - PLID 60763 - this affects both ICD-9 & 10 columns
void CEMRProblemListDlg::OnBnClickedCheckShowDiagnosis()
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowDiagCode) ? 1 : 0, 6, GetCurrentUserName());
	ResizeColumns();
}

// (a.walling 2009-05-04 12:18) - PLID 33751
void CEMRProblemListDlg::OnBnClickedCheckShowChronicity()
{
	UpdateData(TRUE);
	SetRemotePropertyInt("EMRProblemListColumns", (m_bShowChronicity) ? 1 : 0, 7, GetCurrentUserName());
	ResizeColumns();
}

// (z.manning 2009-05-29 09:59) - PLID 34345
//TES 11/25/2009 - PLID 36191 - Changed from a CLabEntryDlg to a CLabRequisitionDlg
void CEMRProblemListDlg::SetLabRequisitionDlg(CLabRequisitionDlg *pdlgLabRequisition)
{
	m_pdlgLabRequisition = pdlgLabRequisition;
}

// (j.jones 2014-02-26 14:30) - PLID 60764 - requerying the history may need to show/hide diagnosis columns
void CEMRProblemListDlg::OnRequeryFinishedListEmrProblemHistory(short nFlags)
{
	try {

		UpdateHistoryListDiagnosisColumns();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-26 14:30) - PLID 60764 - standalone functions for updating diagnosis columns
void CEMRProblemListDlg::UpdateProblemListDiagnosisColumns()
{
	try {

		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_dlProblems, plcDiagCodeName_ICD9, plcDiagCodeName_ICD10, 0, 0, "<None>", "<None>");

	}NxCatchAll(__FUNCTION__);
}

void CEMRProblemListDlg::UpdateHistoryListDiagnosisColumns()
{
	try {

		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_dlHistory, hlcDiagCodeName_ICD9, hlcDiagCodeName_ICD10,
			50, 50, "<None>", "<None>", -1, -1, false, false);

	}NxCatchAll(__FUNCTION__);
}