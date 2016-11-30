// EmrDebugDlg.cpp : implementation file
//
// (c.haag 2007-08-04 10:29) - PLID 26946 - This dialog is used for
// developers to look for discrepancies or data problems in EMN's.
//

#include "stdafx.h"
#include "EmrDebugDlg.h"
#include "EMN.h"
#include "EMRTopic.h"
#include "InternationalUtils.h"
#include "EMNDetail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (c.haag 2007-06-19 10:51) - CSortedDetailArray is a sortable detail array used
// to ensure that detail data is output in sorted order
typedef int (__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
typedef int (__cdecl *COMPAREFN)(const CEMNDetail ** elem1, const CEMNDetail ** elem2);

class CSortedDetailArray : public CArray<CEMNDetail*,CEMNDetail*>
{
public:
	void Sort(COMPAREFN pfnCompare = Compare)
	{
		CEMNDetail** prgdtl = (CEMNDetail**)GetData();
		qsort(prgdtl,GetSize(),sizeof(CEMNDetail*),(GENERICCOMPAREFN)pfnCompare);
	}

protected:
	static int __cdecl Compare(const CEMNDetail** pp1, const CEMNDetail** pp2)
	{
		const CEMNDetail* p1 = *pp1;
		const CEMNDetail* p2 = *pp2;
		ASSERT(p1);
		ASSERT(p2);

		int nStrCmpRes = p1->GetLabelText().CompareNoCase( p2->GetLabelText() );
		if (nStrCmpRes != 0) {
			return nStrCmpRes;
		} else {
			if (p1->m_nEMRTemplateDetailID < p2->m_nEMRTemplateDetailID) {
				return -1;
			} else if (p1->m_nEMRTemplateDetailID > p2->m_nEMRTemplateDetailID) {
				return 1;
			} else {

				if (p1->m_nEMRInfoID < p2->m_nEMRInfoID) {
					return -1;
				} else if (p1->m_nEMRInfoID  > p2->m_nEMRInfoID) {
					return 1;
				} else {

					if (p1->m_nEMRDetailID < p2->m_nEMRDetailID) {
						return -1;
					} else if (p1->m_nEMRDetailID > p2->m_nEMRDetailID) {
						return 1;
					} else {

						return 0;
					
					}
				}
			}
		}
	}

};

/////////////////////////////////////////////////////////////////////////////
// CEmrDebugDlg dialog


CEmrDebugDlg::CEmrDebugDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrDebugDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrDebugDlg)
	m_strDiffFile = _T("");
	m_strDiffApp = _T("");
	m_strFilePath = _T("");
	//}}AFX_DATA_INIT
	m_pEMR = NULL;
	m_pActiveEMN = NULL;
}

void CEmrDebugDlg::SetEMR(CEMR* pEMR)
{
	m_pEMR = pEMR;
}

void CEmrDebugDlg::SetActiveEMN(CEMN* pActiveEMN)
{
	m_pActiveEMN = pActiveEMN;
}

CEmrDebugDlg::EOutputMethod CEmrDebugDlg::GetOutputMethod()
{
	// (c.haag 2007-09-17 10:17) - PLID 27401 - Returns the method in which we report information
	if (IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_CONSOLE)) {
		return eEMRDebugOutput_Console;
	} else if (IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_TEXT)) {
		return eEMRDebugOutput_TextFile;
	} else {
		ASSERT(FALSE); // This should never happen
		return eEMRDebugOutput_Console;
	}
}


void CEmrDebugDlg::Output(const CString& str)
{
	// (c.haag 2007-09-17 11:29) - PLID 27041 - We used to use TRACE to output information,
	// but now that we can output to files, we need this function to deal with the abstraction
#ifdef _DEBUG
	switch (GetOutputMethod()) {
	case eEMRDebugOutput_Console:
		TRACE(str);
		// (c.haag 2007-09-17 10:23) - Give the output window time to handle this.
		// If this code is not in place, I experience a great deal of "output loss"
		// where TRACE lines never make it to the output window.
		Sleep(5);
		break;

	case eEMRDebugOutput_TextFile:
		// (c.haag 2007-09-17 11:28) - PLID 27401 - Added support for logging to text files
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - this is invalid in 2008; just check for a null stream
		if (m_fileOutput.m_pStream) {
			m_fileOutput.WriteString(str);
		}
		break;
	}
#endif
}

void CEmrDebugDlg::DumpEMNTopicsRecurse(BOOL bOnlyInclude8300Fields, BOOL bAllowAddresses, CEMRTopic* pTopic)
{
#ifdef _DEBUG
	// (c.haag 2007-08-04 10:38) - PLID 26946 - Recursively reports topic
	// information to the Output window
	// (c.haag 2007-09-17 13:23) - PLID 27408 - We now have a flag for allowing or suppressing addresses
	pTopic->DebugReportMembers(this, bOnlyInclude8300Fields, bAllowAddresses);
	const int nSubTopics = pTopic->GetSubTopicCount();
	for (int i=0; i < nSubTopics; i++) {
		DumpEMNTopicsRecurse(bOnlyInclude8300Fields, bAllowAddresses, pTopic->GetSubTopic(i));
	}
#endif
}

void CEmrDebugDlg::DumpEMNTreeRecurse(CEMRTopic* pTopic, long nDepth)
{
	// (c.haag 2007-08-07 12:21) - PLID 26946 - This item recursively lists topics and
	// their details to the Output window
	CString str, strTab;
	int i;

	// Generate the identation string
	for (i=0; i < nDepth; i++) {
		strTab += "\t";
	}

	// Print the topic name
	Output(FormatString("%sTopic %s\n", strTab, pTopic->GetName()));

	// Print the topic details in order
	CSortedDetailArray apDetails;
	const int nDetails = pTopic->GetEMNDetailCount();
	for (i=0; i < nDetails; i++) {
		apDetails.Add(pTopic->GetDetailByIndex(i));
	}
	apDetails.Sort();
	for (i=0; i < nDetails; i++) {
		Output(FormatString("\t%sDetail %s\n", strTab, apDetails[i]->GetLabelText()));
	}

	// Recurse
	const int nSubTopics = pTopic->GetSubTopicCount();
	for (i=0; i < nSubTopics; i++) {
		DumpEMNTreeRecurse(pTopic->GetSubTopic(i), nDepth + 1);
	}
}

void CEmrDebugDlg::DumpEMNNarrativeFields()
{
	// (c.haag 2007-11-26 10:40) - PLID 28170 - Report EMR narrative
	// lists. We've already established that m_pActiveEMN is not NULL
	// by this point
	CSortedDetailArray apDetails;
	m_pActiveEMN->GenerateTotalEMNDetailArray(&apDetails);
	apDetails.Sort();

	const long nDetails = apDetails.GetSize();
	for (int i=0; i < nDetails; i++) {
		CEMNDetail* pDetail = apDetails[i];

		// Only include narrative details that have parent topics with valid EMN's
		// (CEMNDetail::GetNarrativeFieldRecordset() doesn't check for those things)
		if (pDetail->m_EMRInfoType == eitNarrative && NULL != pDetail->m_pParentTopic &&
			NULL != pDetail->m_pParentTopic->GetParentEMN()) {

			// Ouput the detail information
			Output(FormatString("Detail %s (Topic %s) Spawn Group ID = %d\n",
				pDetail->GetLabelText(), pDetail->m_pParentTopic->GetName(), pDetail->GetSpawnedGroupID()));

			
			// (a.walling 2009-11-19 15:08) - PLID 36365 - output the NxRichText in its entirety.
			Output(AsString(pDetail->GetState())); // will throw exception if VT_EMPTY

			/*
			// Output the fields
			ADODB::_RecordsetPtr prs = pDetail->GetNarrativeFieldRecordset();
			if (NULL == prs) {
				Output("<null merge field list>");
			} else if (prs->GetRecordCount() == 0) {
				Output("<no merge fields>\n");
			} else {
				prs->MoveFirst();
				while (!prs->eof) {
					CString strField = AdoFldString(prs, "Field", "");
					CString strValue = GetNarrativeValueField(prs);
					Output(FormatString("%s: %s\n", strField, strValue));				
					prs->MoveNext();
				}
			}
			*/
			Output("\n\n");
		}
		
	} // for (i=0; i < nDetails; i++) {
}

void CEmrDebugDlg::UpdateButtonStates()
{
	// Update the enabled state of the Commit button
	BOOL bEnable = FALSE;
	if (IsDlgButtonChecked(IDC_CHECK_ALL_TOPICS) ||
		IsDlgButtonChecked(IDC_CHECK_ALL_DETAILS) ||
		IsDlgButtonChecked(IDC_CHECK_TREE) ||
		// (c.haag 2007-11-26 10:38) - PLID 28170 - Support for
		// reporting EMR narrative field lists
		IsDlgButtonChecked(IDC_CHECK_NARRATIVE_FIELD_LISTS)) {
		bEnable = TRUE;
	}
	GetDlgItem(IDOK)->EnableWindow(bEnable);

	// (c.haag 2007-09-17 10:45) - PLID 27401 - Update the state of the Path edit box
	GetDlgItem(IDC_EDIT_FILE_PATH)->EnableWindow( IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_TEXT) ? TRUE : FALSE );

	// (c.haag 2007-09-17 12:26) - PLID 27406 - Update the state of the Comparision boxes
	if (IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_TEXT)) {
		GetDlgItem(IDC_CHECK_RUN_DIFF)->EnableWindow(TRUE);
		if (IsDlgButtonChecked(IDC_CHECK_RUN_DIFF)) {
			GetDlgItem(IDC_EDIT_DIFF_FILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_DIFF_APP)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_EDIT_DIFF_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_DIFF_APP)->EnableWindow(FALSE);
		}
	} else {
		GetDlgItem(IDC_CHECK_RUN_DIFF)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DIFF_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DIFF_APP)->EnableWindow(FALSE);
	}
}

void CEmrDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrDebugDlg)
	DDX_Control(pDX, IDC_CHECK_8300, m_btn8300);
	DDX_Control(pDX, IDC_RADIO_EMRDEBUG_CONSOLE, m_btnConsole);
	DDX_Control(pDX, IDC_RADIO_EMRDEBUG_TEXT, m_btnText);
	DDX_Control(pDX, IDC_CHECK_NARRATIVE_FIELD_LISTS, m_btnNarrative);
	DDX_Control(pDX, IDC_CHECK_TREE, m_btnTree);
	DDX_Control(pDX, IDC_CHECK_RUN_DIFF, m_btnDiff);
	DDX_Control(pDX, IDC_CHECK_ALL_DETAILS, m_btnAllDetails);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_ADDRESS, m_btnIncludeMemAddress);
	DDX_Control(pDX, IDC_CHECK_ALL_TOPICS, m_btnAllTopics);
	DDX_Text(pDX, IDC_EDIT_DIFF_FILE, m_strDiffFile);
	DDV_MaxChars(pDX, m_strDiffFile, 255);
	DDX_Text(pDX, IDC_EDIT_DIFF_APP, m_strDiffApp);
	DDV_MaxChars(pDX, m_strDiffApp, 255);
	DDX_Text(pDX, IDC_EDIT_FILE_PATH, m_strFilePath);
	DDV_MaxChars(pDX, m_strFilePath, 255);
	DDX_Control(pDX, IDC_EDIT_EMN_NAME, m_nxeditEditEmnName);
	DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_nxeditEditFilePath);
	DDX_Control(pDX, IDC_EDIT_DIFF_FILE, m_nxeditEditDiffFile);
	DDX_Control(pDX, IDC_EDIT_DIFF_APP, m_nxeditEditDiffApp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_INFO_GROUPBOX, m_btnInfoGroupbox);
	DDX_Control(pDX, IDC_FILTERS_GROUPBOX, m_btnFiltersGroupbox);
	DDX_Control(pDX, IDC_OUTPUT_GROUPBOX, m_btnOutputGroupbox);
	DDX_Control(pDX, IDC_COMPARISONS_GROUPBOX, m_btnComparisonsGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrDebugDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrDebugDlg)
	ON_BN_CLICKED(IDC_CHECK_ALL_DETAILS, OnCheckAllDetails)
	ON_BN_CLICKED(IDC_CHECK_ALL_TOPICS, OnCheckAllTopics)
	ON_BN_CLICKED(IDC_CHECK_TREE, OnCheckTree)
	ON_BN_CLICKED(IDC_RADIO_EMRDEBUG_CONSOLE, OnCheckConsole)
	ON_BN_CLICKED(IDC_RADIO_EMRDEBUG_TEXT, OnCheckTextFile)
	ON_BN_CLICKED(IDC_CHECK_RUN_DIFF, OnCheckRunDiff)
	ON_BN_CLICKED(IDC_CHECK_NARRATIVE_FIELD_LISTS, OnCheckNarrativeFieldLists)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrDebugDlg message handlers

BOOL CEmrDebugDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-28 11:52) - PLID 29806 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (c.haag 2007-08-04 10:34) - PLID 26946 - Assign names at the top of the dialog
		if (NULL == m_pActiveEMN) {
			SetDlgItemText(IDC_EDIT_EMN_NAME, "<none>");
		} else {
			SetDlgItemText(IDC_EDIT_EMN_NAME, FormatString("[%s] %s", FormatDateTimeForInterface(m_pActiveEMN->GetEMNDate(), NULL, dtoDate), m_pActiveEMN->GetDescription()));
		}

		// (c.haag 2007-09-17 11:39) - PLID 27402 - Assign default checkbox and text selections
		g_propManager.CachePropertiesInBulk("EmrDebugDlg-1", propNumber,
			"(Username = '%s') AND ("
			"Name = 'EmrDebugDlg_OutputTopicObjects' OR "
			"Name = 'EmrDebugDlg_OutputDetailObjects' OR "
			"Name = 'EmrDebugDlg_OutputTree' OR "
			"Name = 'EmrDebugDlg_Include8300FieldsOnly' OR "
			"Name = 'EmrDebugDlg_IncludeMemAddress' OR "
			"Name = 'EmrDebugDlg_OutputMethod' OR "
			"Name = 'EmrDebugDlg_DoDiff' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("EmrDebugDlg-2", propText,
			"(Username = '%s') AND ("
			"Name = 'EmrDebugDlg_TextOutputPath' OR "
			"Name = 'EmrDebugDlg_DiffPath' OR "
			"Name = 'EmrDebugDlg_DiffApp' "
			")",
			_Q(GetCurrentUserName()));

		SetDlgItemCheck(IDC_CHECK_ALL_TOPICS, GetRemotePropertyInt("EmrDebugDlg_OutputTopicObjects",0,0,GetCurrentUserName()) ? 1 : 0);
		SetDlgItemCheck(IDC_CHECK_ALL_DETAILS, GetRemotePropertyInt("EmrDebugDlg_OutputDetailObjects",0,0,GetCurrentUserName()) ? 1 : 0);
		SetDlgItemCheck(IDC_CHECK_TREE, GetRemotePropertyInt("EmrDebugDlg_OutputTree",0,0,GetCurrentUserName()) ? 1 : 0);
		// (c.haag 2007-11-26 10:51) - PLID 28170 - Do the same for narrative lists
		SetDlgItemCheck(IDC_CHECK_NARRATIVE_FIELD_LISTS, GetRemotePropertyInt("EmrDebugDlg_NarrativeFieldLists",0,0,GetCurrentUserName()) ? 1 : 0);

		SetDlgItemCheck(IDC_CHECK_8300, GetRemotePropertyInt("EmrDebugDlg_Include8300FieldsOnly",0,0,GetCurrentUserName()) ? 1 : 0);
		SetDlgItemCheck(IDC_CHECK_INCLUDE_ADDRESS, GetRemotePropertyInt("EmrDebugDlg_IncludeMemAddress",0,0,GetCurrentUserName()) ? 1 : 0);

		if (0 == GetRemotePropertyInt("EmrDebugDlg_OutputMethod",0,0,GetCurrentUserName())) {
			SetDlgItemCheck(IDC_RADIO_EMRDEBUG_CONSOLE, 1);
		} else {
			SetDlgItemCheck(IDC_RADIO_EMRDEBUG_TEXT, 1);
		}

		SetDlgItemText(IDC_EDIT_FILE_PATH, GetRemotePropertyText("EmrDebugDlg_TextOutputPath", GetNxTempPath() ^ "EMRDebugInfoOutput.txt",0,GetCurrentUserName()));

		SetDlgItemCheck(IDC_CHECK_RUN_DIFF, GetRemotePropertyInt("EmrDebugDlg_DoDiff",0,0,GetCurrentUserName()) ? 1 : 0);
		SetDlgItemText(IDC_EDIT_DIFF_FILE, GetRemotePropertyText("EmrDebugDlg_DiffPath", "",0,GetCurrentUserName()));
		SetDlgItemText(IDC_EDIT_DIFF_APP, GetRemotePropertyText("EmrDebugDlg_DiffApp", "C:\\Program Files\\Microsoft Visual Studio\\COMMON\\Tools\\WINDIFF.EXE",0,GetCurrentUserName()));

		// (c.haag 2007-09-17 11:54) - PLID 27402 - Ensure the button states are up to date based on our defaults
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrDebugDlg::OnOK() 
{
#ifdef _DEBUG // Enable compiling in release mode
	try {
		/////////////////////////////////////////////////////////
		// Validation
		/////////////////////////////////////////////////////////
		CWaitCursor wc;
		CString strFilePath, strDiffPath, strAppPath;
		GetDlgItemText(IDC_EDIT_FILE_PATH, strFilePath);
		GetDlgItemText(IDC_EDIT_DIFF_FILE, strDiffPath);
		GetDlgItemText(IDC_EDIT_DIFF_APP, strAppPath);
		BOOL bOnlyInclude8300Fields = (IsDlgButtonChecked(IDC_CHECK_8300) ? TRUE : FALSE);
		// (c.haag 2007-09-17 13:25) - PLID 27408 - We now let the user toggle memory address inclusion
		BOOL bAllowAddresses = (IsDlgButtonChecked(IDC_CHECK_INCLUDE_ADDRESS) ? TRUE : FALSE);

		if (NULL == m_pActiveEMN) {
			MessageBox("You must have an active EMN to output debug information for.", "Practice", MB_OK | MB_ICONSTOP);
			return;
		}

		if (eEMRDebugOutput_TextFile == GetOutputMethod()) {
			if (strFilePath.IsEmpty()) {
				MessageBox("Please enter an output file path.", "Practice", MB_OK | MB_ICONSTOP);
				GetDlgItem(IDC_EDIT_FILE_PATH)->SetFocus();
				return;
			}
			if (IsDlgButtonChecked(IDC_CHECK_RUN_DIFF)) {
				// (c.haag 2007-09-17 15:58) - PLID 27406 - Warn the user if they have nothing filled in
				// for parameter comparisons
				if (strDiffPath.IsEmpty()) {
					MessageBox("Please enter the 'Other file path'.", "Practice", MB_OK | MB_ICONSTOP);
					GetDlgItem(IDC_EDIT_DIFF_FILE)->SetFocus();
					return;
				}
				if (strAppPath.IsEmpty()) {
					MessageBox("Please enter the full executable path to the WinDiff utility'.", "Practice", MB_OK | MB_ICONSTOP);
					GetDlgItem(IDC_EDIT_DIFF_APP)->SetFocus();
					return;
				}
			}
		}

		// (c.haag 2007-09-17 13:14) - PLID 27408 - Yell at the user if they want to run a "diff" while including address information
		if (bAllowAddresses && eEMRDebugOutput_TextFile == GetOutputMethod() && IsDlgButtonChecked(IDC_CHECK_RUN_DIFF)) {
			if (IDNO == MessageBox("You have chosen to do a comparsion on debug output files which potentially include memory address information. "
				"With this configuration, the file comparsion will almost always report that there are differences between two files from different sessions of Practice.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONWARNING | MB_YESNO)) {
				return;
			}
		}

		/////////////////////////////////////////////////////////
		// Execution
		/////////////////////////////////////////////////////////
		// (c.haag 2007-09-17 10:49) - If we are outputting to a file, create the file here
		if (eEMRDebugOutput_TextFile == GetOutputMethod()) {
			if (!m_fileOutput.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat)) {
				MessageBox(CString("Error opening output file for writing - ") + FormatLastError(), "Practice", MB_OK | MB_ICONSTOP);
				return;
			}
		}

		// (c.haag 2007-08-04 10:38) - PLID 26946 - Dump all the topics
		if (IsDlgButtonChecked(IDC_CHECK_ALL_TOPICS)) {
			const int nTopics = m_pActiveEMN->GetTopicCount();
			for (int i=0; i < nTopics; i++) {
				DumpEMNTopicsRecurse(bOnlyInclude8300Fields, bAllowAddresses, m_pActiveEMN->GetTopic(i));
			}
		}

		// (c.haag 2007-08-04 10:38) - PLID 26946 - Dump all the details
		if (IsDlgButtonChecked(IDC_CHECK_ALL_DETAILS)) {
			CSortedDetailArray apDetails;
			m_pActiveEMN->GenerateTotalEMNDetailArray(&apDetails);
			apDetails.Sort();
			const int nDetails = apDetails.GetSize();
			for (int i=0; i < nDetails; i++) {
				apDetails[i]->DebugReportMembers(this, bOnlyInclude8300Fields, bAllowAddresses);
			}
		}

		// (c.haag 2007-11-26 10:46) - PLID 28170 - Dump narrative field lists
		if (IsDlgButtonChecked(IDC_CHECK_NARRATIVE_FIELD_LISTS)) {
			DumpEMNNarrativeFields();
		}

		// (c.haag 2007-08-04 10:38) - PLID 26946 - Dump the EMN tree
		if (IsDlgButtonChecked(IDC_CHECK_TREE)) {
			const int nTopics = m_pActiveEMN->GetTopicCount();
			for (int i=0; i < nTopics; i++) {
				DumpEMNTreeRecurse(m_pActiveEMN->GetTopic(i), 1);
			}
		}

		// (c.haag 2007-09-17 10:54) - PLID 27401 - Cleanup
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - this is invalid in 2008; just check for a null stream
		if (m_fileOutput.m_pStream) {
			m_fileOutput.Close();
		}
		// If we wrote to a text file, open it now
		if (eEMRDebugOutput_TextFile == GetOutputMethod()) {
			// (c.haag 2007-09-17 12:30) - PLID 27406 - If we want to compare the text against another
			// file, do so here
			if (IsDlgButtonChecked(IDC_CHECK_RUN_DIFF)) {
				// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
				HINSTANCE hRes = ShellExecute ((HWND)this, NULL, strAppPath, FormatString("\"%s\" \"%s\"", strFilePath, strDiffPath), NULL, SW_SHOW);
				if ((int)hRes < 32) {
					MessageBox(FormatString("An error occured while trying to compare files:\n\n%s", FormatError((int)hRes)), "Practice", MB_OK | MB_ICONSTOP);
				}

			} else {
				// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
				HINSTANCE hRes = ShellExecute ((HWND)this, NULL, "notepad.exe", ("'" + strFilePath + "'"), NULL, SW_SHOW);
				if ((int)hRes < 32) {
					MessageBox(FormatString("An error occured while trying to load '%s' into notepad:\n\n%s", strFilePath, FormatError((int)hRes)), "Practice", MB_OK | MB_ICONSTOP);
				}
			}
		}

		// (c.haag 2007-09-17 11:48) - PLID 27402 - Save our debug info selections for next time
		SetRemotePropertyInt("EmrDebugDlg_OutputTopicObjects", IsDlgButtonChecked(IDC_CHECK_ALL_TOPICS) ? 1 : 0, 0, _Q(GetCurrentUserName()));
		SetRemotePropertyInt("EmrDebugDlg_OutputDetailObjects", IsDlgButtonChecked(IDC_CHECK_ALL_DETAILS) ? 1 : 0, 0, _Q(GetCurrentUserName()));
		SetRemotePropertyInt("EmrDebugDlg_OutputTree", IsDlgButtonChecked(IDC_CHECK_TREE) ? 1 : 0, 0, _Q(GetCurrentUserName()));
		// (c.haag 2007-11-26 10:51) - PLID 28170 - Do the same for narrative lists
		SetRemotePropertyInt("EmrDebugDlg_NarrativeFieldLists", IsDlgButtonChecked(IDC_CHECK_NARRATIVE_FIELD_LISTS) ? 1 : 0, 0, _Q(GetCurrentUserName()));

		SetRemotePropertyInt("EmrDebugDlg_Include8300FieldsOnly", IsDlgButtonChecked(IDC_CHECK_8300) ? 1 : 0, 0, _Q(GetCurrentUserName()));
		SetRemotePropertyInt("EmrDebugDlg_IncludeMemAddress", IsDlgButtonChecked(IDC_CHECK_INCLUDE_ADDRESS) ? 1 : 0, 0, _Q(GetCurrentUserName()));

		if (IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_CONSOLE)) {
			SetRemotePropertyInt("EmrDebugDlg_OutputMethod", 0, 0, _Q(GetCurrentUserName()));
		} else if (IsDlgButtonChecked(IDC_RADIO_EMRDEBUG_TEXT)) {
			SetRemotePropertyInt("EmrDebugDlg_OutputMethod", 1, 0, _Q(GetCurrentUserName()));
		} else {
			ASSERT(FALSE); // This should never happen
			SetRemotePropertyInt("EmrDebugDlg_OutputMethod", 0, 0, _Q(GetCurrentUserName()));
		}

		SetRemotePropertyText("EmrDebugDlg_TextOutputPath", strFilePath,0,GetCurrentUserName());
		SetRemotePropertyInt("EmrDebugDlg_DoDiff", IsDlgButtonChecked(IDC_CHECK_RUN_DIFF) ? 1 : 0, 0, _Q(GetCurrentUserName()));
		SetRemotePropertyText("EmrDebugDlg_DiffPath", strDiffPath,0,GetCurrentUserName());
		SetRemotePropertyText("EmrDebugDlg_DiffApp", strAppPath,0,GetCurrentUserName());

		CDialog::OnOK();
	}
	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - this is invalid in 2008; just check for a null stream
	NxCatchAllCall("Error in CEmrDebugDlg::OnOK()", if (m_fileOutput.m_pStream) { m_fileOutput.Close(); });
#endif
}

void CEmrDebugDlg::OnCheckAllDetails() 
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckAllDetails()");
}

void CEmrDebugDlg::OnCheckAllTopics() 
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckAllTopics()");
}

void CEmrDebugDlg::OnCheckTree() 
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckTree()");
}

void CEmrDebugDlg::OnCheckConsole()
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckConsole()");
}

void CEmrDebugDlg::OnCheckTextFile()
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckTextFile()");
}

void CEmrDebugDlg::OnCheckRunDiff()
{
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckRunDiff()");
}

void CEmrDebugDlg::OnCheckNarrativeFieldLists() 
{
	// (c.haag 2007-11-26 10:37) - PLID 28170 - Support for
	// reporting EMR narrative field lists
	try {
		UpdateButtonStates();
	}
	NxCatchAll("Error in CEmrDebugDlg::OnCheckNarrativeFieldLists()");
}

HBRUSH CEmrDebugDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_EDIT_EMN_NAME:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}