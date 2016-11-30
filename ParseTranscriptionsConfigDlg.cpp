// ParseTranscriptionsConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRc.h"
#include "ParseTranscriptionsConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsConfigDlg dialog
// (a.walling 2008-07-14 15:41) - PLID 30720 - Dialog to configure parsing options

// (a.wilson 2012-07-24 15:32) - PLID 51753 - enum for the template datalist.
enum TemplateListColumns {
	tlcID = 0,
	tlcName = 1,
};

// (j.jones 2013-03-01 09:38) - PLID 55122 - added enum for EMN status columns
enum EMNStatusColumns
{
	escID = 0,
	escStatus,
};

CParseTranscriptionsConfigDlg::CParseTranscriptionsConfigDlg(const CParseTranscriptionsDlg::CParseOptions& opt, CWnd* pParent /*=NULL*/)
	: CNxDialog(CParseTranscriptionsConfigDlg::IDD, pParent), m_opt(opt)
{
	//{{AFX_DATA_INIT(CParseTranscriptionsConfigDlg)
	//}}AFX_DATA_INIT
}


void CParseTranscriptionsConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParseTranscriptionsConfigDlg)
	DDX_Control(pDX, IDC_CHECK_CREATE_WORD_DOCS, m_nxbCreateWordDocs);
	DDX_Control(pDX, IDC_PARSETXCFG_RESET, m_nxibReset);
	DDX_Control(pDX, IDC_REGEX_TITLE, m_editTitle);
	DDX_Control(pDX, IDC_REGEX_CAT, m_editCat);
	DDX_Control(pDX, IDC_PARSETX_TITLE_LABEL, m_lblTitle);
	DDX_Control(pDX, IDC_PARSETX_CAT_LABEL, m_lblCat);
	DDX_Control(pDX, IDC_PARSETX_PATID_LABEL, m_lblPatID);
	DDX_Control(pDX, IDC_PARSETX_GROUP, m_nxbGroup);
	DDX_Control(pDX, IDC_PARSETX_DATE_LABEL, m_lblDate);
	DDX_Control(pDX, IDC_PARSETX_BREAK_LABEL, m_lblBreak);
	DDX_Control(pDX, IDC_REGEX_SECTION, m_editSection);
	DDX_Control(pDX, IDC_REGEX_DATELINE, m_editDateLine);
	DDX_Control(pDX, IDC_REGEX_PATIDLINE, m_editPatIDLine);
	DDX_Control(pDX, IDC_EDIT_INFO, m_editInfo);
	DDX_Control(pDX, IDC_CHK_BREAK_AT_PATIDLINE, m_nxbBreakOnPatID);
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcolor);
	DDX_Control(pDX, IDC_IMPORT_PATIENT_HISTORY, m_radioHistory);
	DDX_Control(pDX, IDC_IMPORT_PATIENT_EMN, m_radioEMN);
	DDX_Control(pDX, IDC_REGEX_PROVIDER, m_editProviderLine);
	DDX_Control(pDX, IDC_IGNORE_SEGMENT_REVIEW, m_nxbIgnoreSegmentReview);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_INACTIVE_PROVIDERS, m_checkIncludeInactiveProviders);
	DDX_Control(pDX, IDC_CHECK_REQUIRE_ERRORS_FIXED, m_checkRequireErrorsFixed);
	DDX_Control(pDX, IDC_CHECK_MOVE_IMPORTED_FILES, m_checkMoveImportedFiles);
	DDX_Control(pDX, IDC_EDIT_DESTINATION_IMPORT_FOLDER, m_editPostImportFolder);
	DDX_Control(pDX, IDC_BTN_SELECT_DESTINATION_IMPORT_FOLDER, m_btnSelectPostImportFolder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParseTranscriptionsConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CParseTranscriptionsConfigDlg)
	ON_BN_CLICKED(IDC_CHK_BREAK_AT_PATIDLINE, OnChkBreakAtPatidline)
	ON_BN_CLICKED(IDC_PARSETXCFG_RESET, OnReset)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_IMPORT_PATIENT_HISTORY, OnImportDestinationChanged)
	ON_BN_CLICKED(IDC_IMPORT_PATIENT_EMN, OnImportDestinationChanged)
	ON_BN_CLICKED(IDC_CHECK_MOVE_IMPORTED_FILES, OnCheckMoveImportedFiles)
	ON_BN_CLICKED(IDC_BTN_SELECT_DESTINATION_IMPORT_FOLDER, OnBtnSelectPostImportFolder)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsConfigDlg message handlers

BOOL CParseTranscriptionsConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {

		// (a.wilson 2012-07-24 11:45) - PLID 51753
		m_pTemplateCombo = BindNxDataList2Ctrl(IDC_IMPORT_PATIENT_TEMPLATE, true);

		m_EMNStatusCombo = BindNxDataList2Ctrl(IDC_PARSETX_EMN_STATUS_LIST, false);

		// (j.jones 2013-03-01 09:38) - PLID 55122 - add the EMN statuses in the same order we do in EMR,
		// but do NOT add Locked as a choice
		_RecordsetPtr rsStatus = CreateRecordset(GetRemoteDataSnapshot(), "SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID IN (0,1) ORDER BY ID "
			""
			"SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID NOT IN (0,1,2) ORDER BY Name");
		while(!rsStatus->eof) {
			IRowSettingsPtr pRow = m_EMNStatusCombo->GetNewRow();
			pRow->PutValue(escID, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(escStatus, rsStatus->Fields->Item["Name"]->Value);
			m_EMNStatusCombo->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus = rsStatus->NextRecordset(NULL);
		while(!rsStatus->eof) {
			IRowSettingsPtr pRow = m_EMNStatusCombo->GetNewRow();
			pRow->PutValue(escID, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(escStatus, rsStatus->Fields->Item["Name"]->Value);
			m_EMNStatusCombo->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus->Close();

		// (a.walling 2008-08-19 15:31) - PLID 30720 - Limit possible text
		m_editPatIDLine.SetLimitText(3500);
		m_editDateLine.SetLimitText(3500);
		m_editSection.SetLimitText(3500);
		m_editTitle.SetLimitText(3500);
		m_editCat.SetLimitText(3500);
		// (a.wilson 2012-08-01 10:13) - PLID 51901 - provider parsing.
		m_editProviderLine.SetLimitText(3500);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);
		m_nxibReset.AutoSet(NXB_MODIFY);
		
		// (a.walling 2008-08-08 13:12) - PLID 28838 - Modified the text slightly.
		// (a.wilson 2012-08-03 14:36) - PLID 51901 - add information on provider parsing.
		m_editInfo.SetWindowText(
			"Advanced options for parsing transcriptions may be set up here. The default settings handle "
			"the format of 'Identifier: value'. Since the Provider, Category and Title may be an arbitrary "
			"number of words, they must be at the end of a line. Patient IDs and dates are much more "
			"discrete, and may appear anywhere in the line, even adjacent. By default, the Patient ID "
			"signals the start of a section. Therefore it should be the very first line. For example, the following "
			"segments will all be parsed and handled correctly:\r\n"
			"\r\n"
			"\tPatient ID: 1729 Date: 12/21/2009 3:00 PM Category: Clinical Notes\r\n"
			"\tTitle: Transcription Note\r\n"
			"\r\n (OR)"
			"\tPatient ID: 1729 Date: December 2nd, 2009 12:00P\r\n"
			"\tTitle: Transcription Note\r\n"
			"\tCategory: Clinical Notes\r\n"
			"\r\n (OR)"
			"\tPatient ID: 1729\r\n"
			"\tDate: August 4, 2009 9:00A\r\n"
			"\tTitle: Transcription Note\r\n"
			"\tCategory: Clinical Notes\r\n"
			"\tProvider: {First} {Last}\r\n"
			"\r\n (OR)"
			"\tPatient ID: 1729\r\n"
			"\tDate: February 1st, 2009\r\n"
			"\tCategory: Clinical Notes\r\n"
			"\tTitle: Transcription Note\r\n"
			"\tProvider: {Last}, {First} {Middle}\r\n"
			"\r\n\r\n"
			"These settings can be adapted to other formats, but are best left alone. It is much easier to "
			"provide instructions to your transcription agency. For more help or information, please contact NexTech "
			"Technical Support. However, if you feel comfortable modifying these yourself, a brief reference is provided:"
			"\r\n"
			"\r\n"
			"^\t\tOnly match the beginning of a string.\r\n"
			"$\t\tOnly match the ending of a string.\r\n"
			"\\b\t\tMatches any word boundary.\r\n"
			"\\B\t\tMatches any non-word boundary. \r\n"
			"\r\n"
			"[xyz]\t\tMatch any one character enclosed in the character set.\r\n"
			"[^xyz]\t\tMatch any one character not enclosed in the character set.\r\n"
			"\r\n"
			"\\t\t\tMatches a tab.  \r\n"
			"\r\n"
			".\t\tMatch any character except a new line.  \r\n"
			"\\w\t\tMatch any word character. Equivalent to [a-zA-Z_0-9].  \r\n"
			"\\W\t\tMatch any non-word character. Equivalent to [^a-zA-Z_0-9].  \r\n"
			"\\d\t\tMatch any digit. Equivalent to [0-9].  \r\n"
			"\\D\t\tMatch any non-digit. Equivalent to [^0-9].  \r\n"
			"\\s\t\tMatch any space character. Equivalent to [ \\t].  \r\n"
			"\\S\t\tMatch any non-space character. Equivalent to [^ \\t].  \r\n"
			"\r\n"
			"?\t\tMatch zero or one occurrences.\r\n"
			"*\t\tMatch zero or more occurrences.\r\n"
			"+\t\tMatch one or more occurrences.\r\n"
			"{x}\t\tMatch exactly x occurrences.\r\n"
			"{x,}\t\tMatch x or more occurrences.\r\n"
			"{x,y}\t\tMatches x to y number of occurrences.\r\n"
			"\r\n"
			"()\t\tGroups clauses to create a clause.\r\n"
			"|\t\tCombines multiple clauses into one, then matches any of the clauses.\r\n"
			);

		Load();
	} NxCatchAll("Error in CParseTranscriptionsConfigDlg::OnInitDialog");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CParseTranscriptionsConfigDlg::OnOK() 
{
	try {
		m_editPatIDLine.GetWindowText(m_opt.strPatID);
		m_editDateLine.GetWindowText(m_opt.strDate);
		m_editSection.GetWindowText(m_opt.strBreak);
		m_editTitle.GetWindowText(m_opt.strTitle);
		m_editCat.GetWindowText(m_opt.strCat);
		// (a.wilson 2012-08-01 10:29) - PLID 51901 - get provdier parsing options
		m_editProviderLine.GetWindowText(m_opt.strProvider);

		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		m_opt.bIncludeInactiveProviders = m_checkIncludeInactiveProviders.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		m_opt.bBreakOnPatID = m_nxbBreakOnPatID.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		// (a.walling 2008-07-17 16:34) - PLID 30774 - Option to create word documents
		m_opt.bCreateWordDocs = m_nxbCreateWordDocs.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		// (a.wilson 2012-07-24 15:24) - PLID 51753 Collect new options
		m_opt.bImportIntoEMR = m_radioEMN.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		{
			IRowSettingsPtr pRow = m_pTemplateCombo->GetCurSel();
			if (pRow) {
				m_opt.nTemplateID = VarLong(pRow->GetValue(tlcID), -1);
			} else {
				m_opt.nTemplateID = -1;
			}
		}

		// (j.jones 2013-03-01 11:00) - PLID 55122 - added default EMN Status
		{
			IRowSettingsPtr pRow = m_EMNStatusCombo->GetCurSel();
			//0 is a hard coded value for Open, and is the default value
			if (pRow) {
				m_opt.nEMNStatusID = VarLong(pRow->GetValue(escID), 0);
			} else {
				m_opt.nEMNStatusID = 0;
			}
		}

		// (a.wilson 2012-10-15 14:21) - PLID 53129 - save new option to ignore segment reviews.
		m_opt.bIgnoreSegmentReview = m_nxbIgnoreSegmentReview.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
		m_opt.bRequireErrorsFixed = m_checkRequireErrorsFixed.GetCheck() == BST_CHECKED ? TRUE : FALSE;

		CString strErrors;

		if (m_opt.strPatID.IsEmpty()) {
			strErrors += "A patient ID pattern must be specified.\r\n";
		}

		if (!m_opt.bBreakOnPatID && m_opt.strBreak.IsEmpty()) {
			strErrors += "A section break must be specified if the patient ID line does not act as one.\r\n";
		}
		// (a.wilson 2012-07-24 15:31) - PLID 51753 - add a check to ensure they pick a template if they want to import into EMR.
		if (m_opt.bImportIntoEMR == TRUE && m_opt.nTemplateID == -1) {
			strErrors += "A template must be specified if importing into a new patient EMN.\r\n";
		// (a.wilson 2012-08-23 12:56) - PLID 51753 - add a check to ensure they pick a template with a text detail to import into EMR.
		} else if (
			m_opt.bImportIntoEMR == TRUE && 
			!ReturnsRecordsParam("SELECT TOP 1 EMRI.ID FROM EMRTemplateDetailsT EMRTD "
				"INNER JOIN EMRInfoMasterT EMRIM ON EMRIM.ID = EMRTD.EmrInfoMasterID "
				"INNER JOIN EMRInfoT EMRI ON EMRI.ID = EMRIM.ActiveEMRInfoID "
				"WHERE EMRI.DataType = 1 AND EMRTD.TemplateID = {INT}", m_opt.nTemplateID)) {
			strErrors += "A template with a text detail must be used if importing into a new patient EMN.\r\n";
		}
		// (a.wilson 2012-07-24 17:25) - PLID 52305 - add a check to ensure they dont have emr checked with no license.
		if (m_opt.bImportIntoEMR == TRUE && !(g_pLicense && g_pLicense->HasEMR(CLicense::cflrSilent) == 2)) {
			strErrors += "The import destination must be set to patient history when an EMR license has expired.\r\n";
		}

		// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
		BOOL bMoveImportedFiles = m_checkMoveImportedFiles.GetCheck();
		CString strPostImportFolder;
		m_editPostImportFolder.GetWindowText(strPostImportFolder);
		if(bMoveImportedFiles) {
			if(strPostImportFolder.IsEmpty()) {
				strErrors += "No folder is selected to move imported transcription files into.\r\n";
			}
			else if(!DoesExist(strPostImportFolder)) {
				strErrors += "The folder selected to move imported transcription files into does not exist.\r\n";
			}
		}

		strErrors.TrimRight("\r\n");

		if (!strErrors.IsEmpty()) {
			MessageBox(CString("Please review the following warnings and correct any errors:\r\n\r\n") + strErrors);
			return;
		}

		// (j.jones 2013-03-01 12:56) - PLID 55120 - if we didn't return, our post-import fields can be committed
		m_opt.bMoveImportedFiles = bMoveImportedFiles;
		m_opt.strPostImportFolder = strPostImportFolder;
		
		CNxDialog::OnOK();

	} NxCatchAll("Error in CParseTranscriptionsConfigDlg::OnOK");
}

void CParseTranscriptionsConfigDlg::OnCancel() 
{	
	CNxDialog::OnCancel();
}

CParseTranscriptionsDlg::CParseOptions& CParseTranscriptionsConfigDlg::GetParseOptions()
{
	return m_opt;
}

void CParseTranscriptionsConfigDlg::OnChkBreakAtPatidline() 
{
	try {
		if (m_nxbBreakOnPatID.GetCheck() != BST_CHECKED) {
			MessageBox("If a new section does not begin at the patient ID line, you MUST specify a section break.");
		}
	} NxCatchAll("CParseTranscriptionsConfigDlg::OnChkBreakAtPatidline");
}

void CParseTranscriptionsConfigDlg::OnReset() 
{
	try {
		if (IDYES == MessageBox("Your current settings will be overwritten with the defaults. If needed, you can cancel without saving. Do you want to continue?", NULL, MB_YESNO)) {
			m_opt.ResetToDefaults();
			Load();
		}
	} NxCatchAll("CParseTranscriptionsConfigDlg::OnReset");
}

void CParseTranscriptionsConfigDlg::Load() 
{
	m_editPatIDLine.SetWindowText(m_opt.strPatID);
	m_editDateLine.SetWindowText(m_opt.strDate);
	m_editSection.SetWindowText(m_opt.strBreak);
	m_editTitle.SetWindowText(m_opt.strTitle);
	m_editCat.SetWindowText(m_opt.strCat);
	// (a.wilson 2012-08-01 10:28) - PLID 51901 - provider parsing setup into edit control
	m_editProviderLine.SetWindowText(m_opt.strProvider);

	// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
	m_checkIncludeInactiveProviders.SetCheck(m_opt.bIncludeInactiveProviders ? BST_CHECKED : BST_UNCHECKED);

	m_nxbBreakOnPatID.SetCheck(m_opt.bBreakOnPatID ? BST_CHECKED : BST_UNCHECKED);

	// (a.walling 2008-07-17 16:34) - PLID 30774 - Option to create word documents
	m_nxbCreateWordDocs.SetCheck(m_opt.bCreateWordDocs ? BST_CHECKED : BST_UNCHECKED);

	// (a.wilson 2012-07-24 14:07) - PLID 51753 - option to import to emr.
	m_pTemplateCombo->FindByColumn(tlcID, m_opt.nTemplateID, NULL, VARIANT_TRUE);

	// (j.jones 2013-03-01 11:00) - PLID 55122 - added default EMN Status
	if(m_EMNStatusCombo->SetSelByColumn(escID, m_opt.nEMNStatusID) == NULL) {
		//the status is invalid, force it to be Open
		m_EMNStatusCombo->SetSelByColumn(escID, (long)0);
		m_opt.nEMNStatusID = 0;
	}

	// (a.wilson 2012-10-15 14:29) - PLID 53129 - load saved options for ignoring segment review
	m_nxbIgnoreSegmentReview.SetCheck(m_opt.bIgnoreSegmentReview ? BST_CHECKED : BST_UNCHECKED);

	// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
	m_checkRequireErrorsFixed.SetCheck(m_opt.bRequireErrorsFixed ? BST_CHECKED : BST_UNCHECKED);

	if (!m_opt.bImportIntoEMR) {
		m_radioEMN.SetCheck(BST_UNCHECKED);
		m_radioHistory.SetCheck(BST_CHECKED);
		m_pTemplateCombo->Enabled = VARIANT_FALSE;
		m_nxbCreateWordDocs.EnableWindow(TRUE);
		// (j.jones 2013-02-28 13:05) - PLID 55123 - the inactive provider option is only available for EMR imports
		m_checkIncludeInactiveProviders.EnableWindow(FALSE);
		// (j.jones 2013-03-01 09:38) - PLID 55122 - EMN Status is only available for EMR imports
		m_EMNStatusCombo->Enabled = VARIANT_FALSE;
	} else {
		m_radioEMN.SetCheck(BST_CHECKED);
		m_radioHistory.SetCheck(BST_UNCHECKED);
		m_nxbCreateWordDocs.EnableWindow(FALSE);
		m_pTemplateCombo->Enabled = VARIANT_TRUE;
		// (j.jones 2013-02-28 13:05) - PLID 55123 - the inactive provider option is only available for EMR imports
		m_checkIncludeInactiveProviders.EnableWindow(TRUE);
		// (j.jones 2013-03-01 09:38) - PLID 55122 - EMN Status is only available for EMR imports
		m_EMNStatusCombo->Enabled = VARIANT_TRUE;
	}

	// (a.wilson 2012-07-24 17:15) - PLID 51753 - disable emr button if they dont have licensing for EMR.
	bool bHasEMR = (g_pLicense && g_pLicense->HasEMR(CLicense::cflrSilent) == 2);

	if (!bHasEMR) {
		m_radioEMN.EnableWindow(FALSE);
		m_pTemplateCombo->Enabled = VARIANT_FALSE;
		// (j.jones 2013-02-28 13:05) - PLID 55123 - the inactive provider option is only available for EMR imports
		m_checkIncludeInactiveProviders.EnableWindow(FALSE);
		// (j.jones 2013-03-01 09:38) - PLID 55122 - EMN Status is only available for EMR imports
		m_EMNStatusCombo->Enabled = VARIANT_FALSE;
	}

	// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
	m_checkMoveImportedFiles.SetCheck(m_opt.bMoveImportedFiles ? BST_CHECKED : BST_UNCHECKED);
	m_editPostImportFolder.SetWindowText(m_opt.strPostImportFolder);
	m_editPostImportFolder.EnableWindow(m_opt.bMoveImportedFiles);
	m_btnSelectPostImportFolder.EnableWindow(m_opt.bMoveImportedFiles);

}
// (a.wilson 2012-07-24 14:52) - PLID 51753  - update based on the radio button change.
void CParseTranscriptionsConfigDlg::OnImportDestinationChanged()
{
	try {
		if (m_radioHistory.GetCheck() == BST_CHECKED) {
			m_pTemplateCombo->Enabled = VARIANT_FALSE;
			m_nxbCreateWordDocs.EnableWindow(TRUE);
			m_radioEMN.SetCheck(BST_UNCHECKED);
			// (j.jones 2013-02-28 13:05) - PLID 55123 - the inactive provider option is only available for EMR imports
			m_checkIncludeInactiveProviders.EnableWindow(FALSE);
			// (j.jones 2013-03-01 09:38) - PLID 55122 - EMN Status is only available for EMR imports
			m_EMNStatusCombo->Enabled = VARIANT_FALSE;
		}
		else if (m_radioEMN.GetCheck() == BST_CHECKED) {
			m_pTemplateCombo->Enabled = VARIANT_TRUE;
			m_nxbCreateWordDocs.EnableWindow(FALSE);
			m_radioHistory.SetCheck(BST_UNCHECKED);
			// (j.jones 2013-02-28 13:05) - PLID 55123 - the inactive provider option is only available for EMR imports
			m_checkIncludeInactiveProviders.EnableWindow(TRUE);
			// (j.jones 2013-03-01 09:38) - PLID 55122 - EMN Status is only available for EMR imports
			m_EMNStatusCombo->Enabled = VARIANT_TRUE;
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
void CParseTranscriptionsConfigDlg::OnCheckMoveImportedFiles()
{
	try {

		BOOL bEnabled = m_checkMoveImportedFiles.GetCheck();
		m_editPostImportFolder.EnableWindow(bEnabled);
		m_btnSelectPostImportFolder.EnableWindow(bEnabled);

		//if initially enabling this feature, and no folder is selected,
		//prompt to select a folder
		if(bEnabled) {
			CString strPostImportFolder;
			m_editPostImportFolder.GetWindowText(strPostImportFolder);
			if(strPostImportFolder.IsEmpty()) {
				OnBtnSelectPostImportFolder();
			}
		}

	} NxCatchAll(__FUNCTION__);
}


// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
void CParseTranscriptionsConfigDlg::OnBtnSelectPostImportFolder()
{
	try {

		CString strPostImportFolder;
		m_editPostImportFolder.GetWindowText(strPostImportFolder);

		CString strInitialPath;
		//if no folder is selected, or the folder that is selected doesn't exist,
		//default to the shared path on the server
		if(strPostImportFolder.IsEmpty() || !DoesExist(strPostImportFolder)) {
			strInitialPath = GetSharedPath();
		}
		else {
			strInitialPath = strPostImportFolder;
		}

		if(!BrowseToFolder(&strPostImportFolder, "Select a folder in which to move imported transcription files into:", GetSafeHwnd(), "", strInitialPath)) {
			//if they cancelled and the folder is empty, uncheck the checkbox
			if(strPostImportFolder.IsEmpty()) {
				m_checkMoveImportedFiles.SetCheck(FALSE);
			}
			return;
		}

		//if the folder is empty, uncheck the checkbox
		if(strPostImportFolder.IsEmpty()) {
			m_checkMoveImportedFiles.SetCheck(FALSE);
		}

		m_editPostImportFolder.SetWindowText(strPostImportFolder);

	} NxCatchAll(__FUNCTION__);
}