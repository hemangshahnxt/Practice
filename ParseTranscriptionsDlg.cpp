// ParseTranscriptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRc.h"
#include "ParseTranscriptionsDlg.h"
#include "shlwapi.h"
#include "FileUtils.h"
#include "mergeengine.h"
#include "ParseTranscriptionsConfigDlg.h"
#include "shellapi.h"
#include "MergeEngine.h"
#include "DateTimeUtils.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "EditTextDlg.h"
#include "DontShowDlg.h"
//TES 9/18/2008 - PLID 31413 - GetPatientDocumentPath() moved to HistoryUtils
#include "HistoryUtils.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h> // (a.wilson 2012-08-09 16:38) - PLID 33429 - added for safearray handling

#import "RegEx.tlb" rename_namespace("RegEx")

using namespace ADODB;
using namespace NXDATALIST2Lib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsDlg dialog

// (a.wilson 2012-08-06 09:09) - PLID 33429 - add some text to add emr import information.
long CParseTranscriptionsDlg::m_cnProgressResolution = 1000;
#define DEFAULT_TX_TITLE "Transcription"
#define DEFAULT_TX_INSTRUCTIONS "Drag and drop or click 'Add File(s)' to choose documents to parse. Once you are finished, click 'Parse All' to parse the documents into separate transcription segments.\r\n\r\n" \
			"You may then review the parsed segments. Check 'This segment has been reviewed...' to remove the 'Dictated but not read' disclaimer which is inserted at the end of each segment. " \
			"You may also verify or modify the detected patient, provider, category, title, and date. If no category is set, the default category will be used. If no provider is set for importing into a new EMN then no provider will be assigned.\r\n\r\n" \
			"To change the segment that is being imported, choose 'Select Segment Area Within Document' and select the portion of the document that corresponds to this segment. Then click 'Set To Selection' to set that segment " \
			"to the text that you have just selected. To edit the actual text of the segment, choose 'Edit Segment Text'.\r\n\r\n" \
			"The 'Configure' button will allow you to set the import destination, advanced options for parsing transcriptions, and also choose " \
			"whether to import the segments as Microsoft Office Word documents (default) or plain text files.\r\n\r\n" \
			"Finally, click 'Import Selected!' to import the segments into the patients' history tab or into a new patient EMN."

// (a.wilson 2012-09-10 10:59) - PLID 51901 - define to prevent repetitive strings.
#define NOT_FOUND			"(not found) "
#define DUPLICATE_MATCHES	"(duplicate matches) "

// (j.jones 2013-03-01 09:38) - PLID 55122 - added enum for EMN status columns
enum EMNStatusColumns
{
	escID = 0,
	escStatus,
};

CParseTranscriptionsDlg::CParseTranscriptionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CParseTranscriptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CParseTranscriptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hSubIcon = NULL;
	m_hIconRedX = NULL;
	m_bCheckedForWord = FALSE;
	m_bSettingEditText = FALSE;
	m_nDefaultCategoryID = -1;
	m_bEditSelectionMode = FALSE;
	m_bViewWholeDocument = FALSE;
}


void CParseTranscriptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParseTranscriptionsDlg)
	DDX_Control(pDX, IDC_PARSETXDLG_DEFAULTCAT_LABEL, m_lblDefCat);
	DDX_Control(pDX, IDC_PARSETXDLG_TITLE_LABEL, m_lblTitle);
	DDX_Control(pDX, IDC_PARSETXDLG_PATIENT_LABEL, m_lblPatient);
	DDX_Control(pDX, IDC_PARSETXDLG_DATE_LABEL, m_lblDate);
	DDX_Control(pDX, IDC_PARSETXDLG_CAT_LABEL, m_lblCat);
	DDX_Control(pDX, IDC_PARSETXDLG_DROP_REMIND, m_lblDrop);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_editTitle);
	DDX_Control(pDX, IDCANCEL, m_nxibClose);
	DDX_Control(pDX, IDC_BTN_SET_TO_SELECTION, m_nxibSetToSelection);
	DDX_Control(pDX, IDC_BTN_EDIT_TEXT, m_nxibEditText);
	DDX_Control(pDX, IDC_BTN_PARSE_ALL, m_nxibParseAll);
	DDX_Control(pDX, IDC_BTN_ADD_FILE, m_nxibAddFile);
	DDX_Control(pDX, IDC_BTN_IMPORT, m_nxibImport);
	DDX_Control(pDX, IDC_BTN_CONFIGURE, m_nxibConfigure);
	DDX_Control(pDX, IDC_CHECK_REVIEWED, m_nxbReviewed);
	DDX_Control(pDX, IDC_CHECK_VIEW_WHOLE_DOCUMENT, m_nxbViewWholeDoc);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcolor);
	DDX_Control(pDX, IDC_PARSED_TEXT, m_text);
	DDX_Control(pDX, IDC_NXCOLORCTRL2, m_nxcolorSegment);
	DDX_Control(pDX, IDC_TRANSCRIPTION_ERROR_COUNT_LABEL, m_labelErrorCount);
	DDX_Control(pDX, IDC_BTN_GO_TO_NEXT_TRANSCRIPTION_ERROR, m_btnGoToNextError);
	DDX_Control(pDX, IDC_EMN_STATUS_LIST_LABEL, m_nxstaticEMNStatusLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParseTranscriptionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CParseTranscriptionsDlg)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BTN_ADD_FILE, OnBtnAddFile)
	ON_BN_CLICKED(IDC_BTN_PARSE_ALL, OnBtnParseAll)
	ON_BN_CLICKED(IDC_BTN_SET_TO_SELECTION, OnBtnSetToSelection)
	ON_BN_CLICKED(IDC_CHECK_REVIEWED, OnCheckReviewed)
	ON_BN_CLICKED(IDC_BTN_IMPORT, OnBtnImport)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE, OnBtnConfigure)
	ON_EN_CHANGE(IDC_EDIT_TITLE, OnChangeEditTitle)
	ON_BN_CLICKED(IDC_CHECK_VIEW_WHOLE_DOCUMENT, OnCheckViewWholeDocument)
	ON_BN_CLICKED(IDC_BTN_EDIT_TEXT, OnBtnEditText)
	ON_BN_CLICKED(IDC_BTN_GO_TO_NEXT_TRANSCRIPTION_ERROR, OnBtnGoToNextTranscriptionError)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsDlg message handlers

BOOL CParseTranscriptionsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcolorSegment.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// (a.walling 2008-07-17 14:33) - PLID 30768 - Safety check for license
		// (a.walling 2008-07-30 10:08) - PLID 30768 - 'Use' the license here.
		if (!g_pLicense->HasTranscriptions(CLicense::cflrUse)) {
			::DestroyWindow(GetSafeHwnd());
			return TRUE;
		}

		// (j.jones 2013-02-27 16:18) - PLID 55117 - added an error count label,
		// defaults to being invisible due to lack of text, but will be colored
		// red when text is added later
		m_labelErrorCount.SetWindowText("");
		m_labelErrorCount.SetColor(RGB(255,0,0));

		// (j.jones 2013-02-28 12:19) - PLID 55118 - added button to go to next error,
		// it will not show up until errors exist
		m_btnGoToNextError.ShowWindow(SW_HIDE);

		// (j.jones 2013-02-27 16:35) - PLID 55119 - added error icon
		m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		// (a.wilson 2012-08-23 14:50) - PLID 33429 - cache all the properties being used in parsetranscriptions dialogs.
		// (a.wilson 2012-10-12 16:49) - PLID 53129 - adding new preference to ignore segment review.
		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
		// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
		// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
		g_propManager.BulkCache("ParseTranscriptionsDlg", propbitMemo | propbitNumber, "Username = '<None>' AND Name IN "
			"('ParseOptionsPatID_Memo', 'ParseOptionsDate_Memo', 'ParseOptionsBreak_Memo', 'ParseOptionsCat_Memo', "
			"'ParseOptionsTitle_Memo', 'ParseOptionsBreakOnPatID', 'ParseOptionsCreateWordDocs', 'ParseOptionsImportIntoEMR', "
			"'ParseOptionsTemplateID', 'ParseOptionsProvider_Memo', 'ParseOptionsViewWholeDoc', 'ParseOptionsDefaultEMNCategoryID', "
			"'ParseOptionsDefaultCategoryID', 'ParseOptionsIgnoreSegmentReview', 'ParseOptionsIncludeInactiveProviders', "
			"'ParseOptionsRequireErrorsFixed', 'ParseOptionsDefaultEMNStatus', 'ParseOptionsMoveImportedFiles', 'ParseOptionsPostImportFolder' "
			")");

		m_text.SetLimitText(0);

		m_text.SetWindowText(DEFAULT_TX_INSTRUCTIONS);

		m_nxibClose.AutoSet(NXB_CLOSE);
		m_nxibImport.AutoSet(NXB_NEW); // changed from EXPORT to NEW style, makes more sense.
		// (a.walling 2008-08-14 11:39) - PLID 28838 - Put the NEW style on the Add Files button
		m_nxibAddFile.AutoSet(NXB_NEW); 
		m_nxibSetToSelection.AutoSet(NXB_MODIFY);
		m_nxibEditText.AutoSet(NXB_MODIFY);

		m_nDocumentID = 0;

		m_opt.Load();
		
		m_dl = BindNxDataList2Ctrl(IDC_TREE, false);
		m_dlPat = BindNxDataList2Ctrl(IDC_PATLIST, true);
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		m_dlCat = BindNxDataList2Ctrl(IDC_PARSETX_CATEGORY_LIST, false);
		m_dlDefCat = BindNxDataList2Ctrl(IDC_PARSETX_DEFAULTCATEGORY_LIST, false);
		// (a.wilson 2012-08-02 14:36) - PLID 51932 - need to load categories specific to history OR EMR.
		EnsureCategories();
		// (a.wilson 2012-10-15 15:03) - PLID 53129 - check to see if they want segment reviews to be ignored.
		m_nxbReviewed.ShowWindow(m_opt.bIgnoreSegmentReview ? FALSE : TRUE);

		m_nxTime = BindNxTimeCtrl(this, IDC_PARSETX_DATE);

		// (a.wilson 2012-08-01 14:23) - PLID 51901 - initialize provider combo.
		m_dlProvider = BindNxDataList2Ctrl(IDC_PARSETX_PROVIDER_LIST, false);
		
		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		CString strProviderWhere = "";
		if(!m_opt.bIncludeInactiveProviders) {
			strProviderWhere = "Archived = 0";
		}
		m_dlProvider->PutWhereClause((LPCTSTR)strProviderWhere);
		m_dlProvider->Requery();

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

		m_nxibParseAll.EnableWindow(FALSE);
		m_nxibImport.EnableWindow(FALSE);
		m_nxibSetToSelection.EnableWindow(FALSE);
		m_nxibEditText.EnableWindow(FALSE);

		m_bViewWholeDocument = GetRemotePropertyInt("ParseOptionsViewWholeDoc", FALSE, 0, "<None>", true);
		m_nxbViewWholeDoc.SetCheck(m_bViewWholeDocument ? BST_CHECKED : BST_UNCHECKED);

		m_hSubIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_TOPIC_CLOSED), IMAGE_ICON, 32, 32, 0);
		m_arIcons.Add(m_hSubIcon);

		// (a.wilson 2012-09-10 09:52) - PLID 51901 - check to see if provider needs to hide for import destination.
		EnsureProviderEnabled();

		// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
		EnsureEMNStatus();

		OnCurSelWasSetTree();

		// (a.wilson 2012-07-25 15:31) - PLID 52305 - check their license to see if their allowed to use an EMR configuraton.
		if (m_opt.bImportIntoEMR && g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			//warn them that they cannot import to emr with an expired license or no license at all.
			MessageBox("The transcription parsing configuration is set to import "
				"into a new patient EMN but your license for EMR is expired or inactive."
				"\r\nPlease contact Nextech Techincal Support for assistance or change the "
				"configuration to import into patient history.");
		}
	} NxCatchAll("Error in OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CParseTranscriptionsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CParseTranscriptionsDlg::OnDestroy() 
{
	try {
		// (c.haag 2016-05-10 13:27) - NX-100318 - We no longer maintain a persistent Word application in this object

		IRowSettingsPtr pRow = m_dl->GetFirstRow();

		while (pRow) {
			CTxDoc* pDoc = (CTxDoc*)VarLong(pRow->GetValue(ecObject));

			if (pDoc)
				delete pDoc;

			pRow = pRow->GetNextRow();
		}
		
		for (int i = 0; i < m_arIcons.GetSize(); i++) {
			::DestroyIcon(m_arIcons[i]);
		}
		m_hSubIcon = NULL;

		// (j.jones 2013-02-27 16:35) - PLID 55119 - added error icon
		if(m_hIconRedX) {
			DestroyIcon((HICON)m_hIconRedX);
			m_hIconRedX = NULL;
		}

	} NxCatchAll("Error in OnDestroy");

	CNxDialog::OnDestroy();	
}

void CParseTranscriptionsDlg::OnDropFiles(HDROP hDropInfo) 
{
	try {
		UINT iFile = 0xFFFFFFFF;
		UINT nCount = ::DragQueryFile(hDropInfo, iFile, NULL, NULL);

		for (iFile = 0; iFile < nCount; iFile++) {
			CString str;
			LPTSTR szBuf = str.GetBuffer(MAX_PATH + 1);
			::DragQueryFile(hDropInfo, iFile, szBuf, MAX_PATH);
			str.ReleaseBuffer();

			AddDocumentRow(str);
		}

		::DragFinish(hDropInfo);

		// (j.jones 2013-02-28 11:59) - PLID 55117 - update our error count label
		UpdateErrorCountLabel();

	} NxCatchAll("Error in OnDropFiles");
	
	CNxDialog::OnDropFiles(hDropInfo);
}

BEGIN_EVENTSINK_MAP(CParseTranscriptionsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CParseTranscriptionsDlg)
	ON_EVENT(CParseTranscriptionsDlg, IDC_TREE, 6 /* RButtonDown */, OnRButtonDownTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CParseTranscriptionsDlg, IDC_TREE, 7 /* RButtonUp */, OnRButtonUpTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CParseTranscriptionsDlg, IDC_TREE, 28 /* CurSelWasSet */, OnCurSelWasSetTree, VTS_NONE)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PATLIST, 29 /* SelSet */, OnSelSetPatlist, VTS_DISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_DATE, 2 /* Changed */, OnChangedParsetxDate, VTS_NONE)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_CATEGORY_LIST, 29 /* SelSet */, OnSelSetCategoryList, VTS_DISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_DEFAULTCATEGORY_LIST, 29 /* SelSet */, OnSelSetDefaultCategoryList, VTS_DISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_TREE, 9 /* EditingFinishing */, OnEditingFinishingTree, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PATLIST, 1 /* SelChanging */, OnSelChangingPatlist, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_DEFAULTCATEGORY_LIST, 1 /* SelChanging */, OnSelChangingDefaultCategoryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingCategoryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_DATE, 1 /* KillFocus */, OnKillFocusDate, VTS_NONE)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_PROVIDER_LIST, 1, CParseTranscriptionsDlg::SelChangingProlist, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_PARSETX_PROVIDER_LIST, 29, CParseTranscriptionsDlg::SelSetProlist, VTS_DISPATCH)
	ON_EVENT(CParseTranscriptionsDlg, IDC_TREE, 8, CParseTranscriptionsDlg::OnEditingStartingTree, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CParseTranscriptionsDlg::OnRButtonDownTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_dl->PutCurSel(pRow);
		}
	} NxCatchAll("Error in OnRButtonDownTree");
}

void CParseTranscriptionsDlg::OnRButtonUpTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {
				// do nothing, this is a segment
			} else {
				// this is a document
				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.InsertMenu(1, MF_BYPOSITION, 1, "&Remove");
				CPoint pt;
				GetMessagePos(pt);
				if (1 == mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL)) {
					CTxDoc* pDoc = (CTxDoc*)VarLong(pRow->GetValue(ecObject));
					if (pDoc) {
						delete pDoc;
					}

					// (j.jones 2013-03-04 10:33) - PLID 55120 - prompt to move the file to the post-import folder,
					// they may or may not want to since they didn't import it entirely
					if(pRow != NULL) {
						//this call will prompt the user
						TryMoveImportedTranscriptionFile(VarString(pRow->GetValue(ecFilePath), ""), true);

						//no need to check whether this succeeded, because we're removing the row anyways
					}

					// (a.walling 2008-08-08 12:42) - PLID 28838 - Set to no selection
					m_dl->PutCurSel(NULL);
					m_dl->RemoveRow(pRow);

					long nSegmentCount = 0;
					long nFileCount = 0;
					IRowSettingsPtr pTopRow = m_dl->GetFirstRow();
					while (pTopRow) {
						nFileCount++;
						IRowSettingsPtr pChildRow = pTopRow->GetFirstChildRow();
						while (pChildRow) {
							nSegmentCount++;
							pChildRow = pChildRow->GetNextRow();
						}
						pTopRow = pTopRow->GetNextRow();
					}

					if (nSegmentCount == 0) {
						// no segments, disable the import all button
						m_nxibImport.EnableWindow(FALSE);
					}
					if (nFileCount == 0) {
						// no files, disable the parse all button
						m_nxibParseAll.EnableWindow(FALSE);
					}
				}
				mnu.DestroyMenu();

				// (j.jones 2013-03-04 11:18) - PLID 55117 - We may have removed records that had errors.
				// Update the label now that those records are gone.
				UpdateErrorCountLabel();
			}
		}		
	} NxCatchAll("Error in OnRButtonUpTree");
}

void CParseTranscriptionsDlg::OnBtnAddFile() 
{
	try {
		// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
		CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
			"Supported Files (*.txt;*.doc;*.docx;*.docm)|*.txt; *.doc; *.docx; *.docm|All Files (*.*)|*.*||"
			, this
			);

		CString str;
		LPTSTR szBuf = str.GetBuffer(8192);
		{
			BrowseFiles.m_ofn.lpstrFile = szBuf;
			BrowseFiles.m_ofn.nMaxFile = 8191;
			if (BrowseFiles.DoModal() == IDCANCEL) {
				str.ReleaseBuffer();
				return;
			}

			POSITION pos = BrowseFiles.GetStartPosition();
			while (pos) {
				AddDocumentRow(BrowseFiles.GetNextPathName(pos));
			}
		}
		str.ReleaseBuffer();

		// (j.jones 2013-02-28 11:59) - PLID 55117 - update our error count label
		UpdateErrorCountLabel();

	} NxCatchAll("Error in OnBtnAddFile");
}

void CParseTranscriptionsDlg::AddDocumentRow(CString strFilePath)
{	
	IRowSettingsPtr pCheckRow = m_dl->GetFirstRow();
	while (pCheckRow) {
		CTxDoc* pCheckDoc = (CTxDoc*)VarLong(pCheckRow->GetValue(ecObject), NULL);
		if (pCheckDoc) {
			if (pCheckDoc->strPath.CompareNoCase(strFilePath) == 0) {
				// file already exists, just exit
				return;
			}
		}

		pCheckRow = pCheckRow->GetNextRow();
	}

	CString strFileName = FileUtils::GetFileName(strFilePath);
		
	CString strExtension = FileUtils::GetFileExtension(strFilePath);
	strExtension.MakeLower();
	if (strExtension != "doc" && strExtension != "docx" && strExtension != "docm" && strExtension != "txt") {
		if (IDNO == DontShowMeAgain(this, FormatString("The file %s has an unrecognized extension ('%s'). Only Microsoft Office Word documents and text files are supported for parsing.\r\n\r\nIf you choose not to show this message again, files of this type will be added automatically without this prompt. Are you sure you want to add and parse this file?", strFileName, strExtension), CString("ParseTranscriptionsNonstandardFile_") + strExtension, "Unrecognized file type", FALSE, TRUE)) {
			return;
		}
	}

	CTxDoc* pDoc = new CTxDoc;
	pDoc->nID = m_nDocumentID++;
	pDoc->strPath = strFilePath;

	SHFILEINFO fileinfo;
	HICON hIcon = NULL;
	if (0 == SHGetFileInfo(strFilePath, 0, &fileinfo, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_ICON)) {
		hIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_LETTER), IMAGE_ICON, 32, 32, 0);
	} else {
		hIcon = fileinfo.hIcon;
	}

	m_arIcons.Add(hIcon);
	
	long nRowCount = m_dl->GetRowCount();
	if (nRowCount == 0) {
		m_lblDrop.SetWindowText("Click 'Parse All' to parse these documents.");
	}

	IRowSettingsPtr pRow = m_dl->GetNewRow();
	// (j.jones 2013-02-27 16:35) - PLID 55119 - added error columns, and error icon
	pRow->PutValue(ecID, _variant_t(pDoc->nID));
	pRow->PutValue(ecParentID, g_cvarNull);
	pRow->PutValue(ecIcon, (long)hIcon);
	pRow->PutValue(ecObject, _variant_t((long)pDoc));
	pRow->PutValue(ecCommit, g_cvarNull);
	// (j.jones 2013-03-04 10:06) - PLID 55120 - we now track the file path
	pRow->PutValue(ecFilePath, _variant_t(strFilePath));
	pRow->PutValue(ecName, _variant_t(strFileName));
	pRow->PutValue(ecLabel, g_cvarNull);
	pRow->PutValue(ecCategory, g_cvarNull);
	pRow->PutValue(ecNameError, g_cvarFalse);
	pRow->PutValue(ecCategoryError, g_cvarFalse);
	pRow->PutValue(ecProviderError, g_cvarFalse);
	pRow->PutValue(ecErrorIcon, g_cvarNull);
	m_dl->AddRowAtEnd(pRow, NULL);

	m_nxibParseAll.EnableWindow(TRUE);
}
// (a.wilson 2012-08-03 11:41) - PLID 51932 - move this to a new function to handle silent flag and now the button calls this function.
void CParseTranscriptionsDlg::OnBtnParseAll() 
{
	try {
		ParseAll(false);
	} NxCatchAll(__FUNCTION__);
}

void CParseTranscriptionsDlg::OnBtnSetToSelection()
{
	try {
		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {		
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {
				CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);
				CTxSeg* pSeg = NULL;
				if (pDoc && pDoc->psaText) {
					long nSegID = VarLong(pRow->GetValue(ecID));
					pSeg = pDoc->arSegs[nSegID];
				}
				ASSERT(pDoc);
				ASSERT(pSeg);

				if (m_bEditSelectionMode) {
					if (pSeg) {
						int nStart = 0, nEnd = 0;
						m_text.GetSel(nStart, nEnd);
						if (nStart != nEnd && (nStart >= 0 && nEnd > 0)) {
							if (IDNO == MessageBox("This transcription segment's text will be set to the current selection. Are you sure you want to continue?", NULL, MB_YESNO | MB_ICONQUESTION)) {
								m_nxibSetToSelection.SetWindowText("Select Segment Area Within Document");
								m_bEditSelectionMode = FALSE;
								DisplaySegment(pSeg, pDoc);
								return;
							}

							pSeg->nBeginCharIndex = nStart;
							pSeg->nEndCharIndex = nEnd;

							if (pSeg->m_pstrOverrideText) {
								delete pSeg->m_pstrOverrideText;
								pSeg->m_pstrOverrideText = NULL;
							}

							m_nxbReviewed.SetCheck(BST_UNCHECKED);
							OnCheckReviewed();
							
							m_nxibSetToSelection.SetWindowText("Select Segment Area Within Document");
							m_bEditSelectionMode = FALSE;
							DisplaySegment(pSeg, pDoc);
							
							MessageBox("The selected text has now been set as this transcription segment.", NULL, MB_ICONINFORMATION);
						}
					}
				} else {
					if (pSeg) {
						if (pSeg->m_pstrOverrideText) {
							if (IDNO == MessageBox("This segment has been manually edited. By selecting a new portion of the document for this segment, any manual changes will be lost. Are you sure you want to continue?", NULL, MB_YESNO | MB_ICONQUESTION))
								return;
						}
						DontShowMeAgain(this, "Select the portion of the document you wish to set as this segment. When you click this button ('Set To Selection') again, you will have the option to cancel your changes.", "ParseTranscriptionsSetToSelection", "Parse Transcriptions");
						m_nxibSetToSelection.SetWindowText("Set To Selection");
						m_bEditSelectionMode = TRUE;
						DisplaySegment(pSeg, pDoc);
					}
				}
			}
		}
		
	} NxCatchAll("Error in OnBtnSetToSelection");
}

void CParseTranscriptionsDlg::OnCheckReviewed() 
{	
	try {
		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {			
				CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

				if (pDoc) {
					long nIx = VarLong(pRow->GetValue(ecID), -1);
					if (nIx >= 0) {
						CTxSeg* pSeg = pDoc->arSegs[nIx];

						if (pSeg) {
							pSeg->bReviewed = m_nxbReviewed.GetCheck() == BST_CHECKED ? TRUE : FALSE;

							if (pSeg->bReviewed) {
								// (j.jones 2013-02-28 15:53) - PLID 55121 - CanCommitRow will return true
								// if Commit is checkable (it might not be checked) and if the row isn't
								// disallowed from being imported due to errors
								if(CanCommitRow(pRow)) {
									pRow->PutValue(ecCommit, g_cvarTrue);
								}
							}

							long nScrollToLine = m_text.GetFirstVisibleLine();
							DisplaySegment(pSeg, pDoc, nScrollToLine);
						}
					}
				}
			}
		}
		
	} NxCatchAll("Error in OnCheckReviewed");
}
// (a.wilson 2012-08-01 15:40) - PLID 51901 - add provider parsing.
long CParseTranscriptionsDlg::RefreshSegments()
{
	m_dlCat->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	m_dlPat->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	long nBadSegments = 0;
	// (a.walling 2008-08-18 13:24) - PLID 28838 - Return total # of segments
	long nTotalSegments = 0;

	IRowSettingsPtr pCurRow = m_dl->GetFirstRow();
	while (pCurRow) {
		IRowSettingsPtr pChildRow = pCurRow->GetFirstChildRow();
		while (pChildRow) {
			IRowSettingsPtr pChildRowCopy = pChildRow;

			pChildRow = pChildRow->GetNextRow();
			m_dl->RemoveRow(pChildRowCopy);
		}

		CTxDoc* pDoc = (CTxDoc*)VarLong(pCurRow->GetValue(ecObject), NULL);
		ASSERT(pDoc);
		if (pDoc) {
			for (long i = 0; i < pDoc->arSegs.GetSize(); i++) {
				CTxSeg* pSeg = pDoc->arSegs[i];
				IRowSettingsPtr pNewRow = m_dl->GetNewRow();
				// (j.jones 2013-02-27 16:35) - PLID 55119 - added error columns, and error icon
				pNewRow->PutValue(ecID, _variant_t(i));
				pNewRow->PutValue(ecParentID, _variant_t(pDoc->nID));
				pNewRow->PutValue(ecIcon, (long)m_hSubIcon);
				pNewRow->PutValue(ecObject, _variant_t((long)pDoc));
				pNewRow->PutValue(ecLabel, g_cvarNull);
				pNewRow->PutValue(ecFilePath, g_cvarNull);
				//pNewRow->PutValue(ecLabel, pSeg->nUserDefinedID != LONG_MIN ? _variant_t(pSeg->nUserDefinedID) : g_cvarNull);
				pNewRow->PutValue(ecNameError, g_cvarFalse);
				pNewRow->PutValue(ecCategoryError, g_cvarFalse);
				pNewRow->PutValue(ecProviderError, g_cvarFalse);
				pNewRow->PutValue(ecErrorIcon, g_cvarNull);
				
				//PATIENT
				CString strName;
				if (pSeg->nUserDefinedID != LONG_MIN) {
					IRowSettingsPtr pPatRow = m_dlPat->FindByColumn(ecUserDefinedID, pSeg->nUserDefinedID, NULL, VARIANT_FALSE);

					if (pPatRow) {
						strName = VarString(pPatRow->GetValue(ecLast), "") + ", " + VarString(pPatRow->GetValue(ecFirst), "") + " " + VarString(pPatRow->GetValue(ecMiddle), "");
						strName += FormatString("(%li)", pSeg->nUserDefinedID);
						pSeg->nPatientID = VarLong(pPatRow->GetValue(ecPersonID), -1);
						
						pNewRow->PutValue(ecCommit, g_cvarFalse);
					} else {
						// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the name column with an error,
						// this will color both the name & label columns
						UpdateRowErrorStatus(pNewRow, ecName, true);
						
						pNewRow->PutValue(ecCommit, g_cvarNull);
						nBadSegments++;

						strName.Format("Invalid ID <%li>", pSeg->nUserDefinedID);
					}
				} else {
					// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the name column with an error,
					// this will color both the name & label columns
					UpdateRowErrorStatus(pNewRow, ecName, true);
					
					pNewRow->PutValue(ecCommit, g_cvarNull);
					nBadSegments++;
					
					// (a.walling 2008-08-12 17:47) - PLID 28838 - Clarify this text
					strName = "<No Patient Found>";
				}

				pNewRow->PutValue(ecName, _variant_t(strName));

				//CATEGORY
				if (!pSeg->strCat.IsEmpty()) {
					IRowSettingsPtr pCatRow = NULL;
					
					if (pSeg->strCat.CompareNoCase("miscellaneous") == 0) {
						pCatRow = m_dlCat->FindByColumn(0, _variant_t((long)-1), NULL, VARIANT_TRUE);
					} else {
						pCatRow = m_dlCat->FindByColumn(1, _variant_t(pSeg->strCat), NULL, VARIANT_TRUE);
					}

					if (pCatRow == NULL) {
						// no category found

						// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the category column as having an error
						UpdateRowErrorStatus(pNewRow, ecCategory, true);

						pNewRow->PutValue(ecCategory, _variant_t(CString(NOT_FOUND + pSeg->strCat)));
						pSeg->nCategoryID = -1;
					}
					else {
						pSeg->nCategoryID = VarLong(pCatRow->GetValue(0), -1);
						pNewRow->PutValue(ecCategory, _variant_t(pSeg->strCat));
					}
				}
				else {
					pSeg->nCategoryID = -2;
					pNewRow->PutValue(ecCategory, _variant_t(CString("<Default Category>")));
				}
				
				//PROVIDER	- (a.wilson 2012-08-02 14:01) - PLID 51901 - update provider
				// (j.jones 2013-03-04 13:35) - PLID 55119 - don't bother if not an EMR import
				if (!pSeg->strProvider.IsEmpty() && m_opt.bImportIntoEMR) {
					long nProvID;
					if ((nProvID = FindBestMatchingProviderID(pSeg->strProvider)) == -1) { //not found

						// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the provider column as having an error
						UpdateRowErrorStatus(pNewRow, ecProvider, true);

						pNewRow->PutValue(ecProvider, _variant_t(CString(NOT_FOUND + pSeg->strProvider)));
						pSeg->nProviderID = -1;
					}
					else if (nProvID == -2) {	//duplicates

						// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the provider column as having an error
						UpdateRowErrorStatus(pNewRow, ecProvider, true);

						pNewRow->PutValue(ecProvider, _variant_t(CString(DUPLICATE_MATCHES + pSeg->strProvider)));
						pSeg->nProviderID = -2;
					}
					else {	//valid ID
						IRowSettingsPtr pProvRow = m_dlProvider->FindByColumn(epID, nProvID, NULL, VARIANT_TRUE);
						if (pProvRow) {	//valid selection
							pSeg->nProviderID = nProvID;
							pSeg->strProvider = (LPCTSTR)_bstr_t(pProvRow->GetValue(epName));
							pNewRow->PutValue(ecProvider, _variant_t(pProvRow->GetValue(epName)));
						}
						else {	//invalid

							// (j.jones 2013-02-27 16:35) - PLID 55119 - flag the provider column as having an error
							UpdateRowErrorStatus(pNewRow, ecProvider, true);

							pNewRow->PutValue(ecProvider, _variant_t(CString(NOT_FOUND + pSeg->strProvider)));
							pSeg->nProviderID = -1;
						}
					}
				} else {	//no provider mentioned
					pSeg->nProviderID = -1;
					pNewRow->PutValue(ecProvider, _variant_t(CString("<No Provider>")));
				}

				m_dl->AddRowAtEnd(pNewRow, pCurRow);
				nTotalSegments++;
			}

			if (pDoc->arSegs.GetSize() > 0) {
				pCurRow->PutExpanded(VARIANT_TRUE);
			}
		}
		pCurRow = pCurRow->GetNextRow();
	}

	// (j.jones 2013-02-28 11:59) - PLID 55117 - update our error count label
	UpdateErrorCountLabel();

	if (nBadSegments > 0) {
		MessageBox(FormatString("There are %li segments which have not been matched to a patient. Until a patient is selected for these segments, you will be unable to import them. The checkbox will not be available next to the row until a patient is chosen.", nBadSegments), NULL, MB_ICONINFORMATION);
	}

	return nTotalSegments;
}

// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
// (z.manning 2016-06-02 16:10) - NX-100790 - Added output param indicating if the action was canceled
BOOL CParseTranscriptionsDlg::GetDocumentText(const CString& strPath, OUT CStringArray& saText, IN OUT CString& strErrors, CProgressDialog &progressDialog
	, long nCurrent, long nTotal, std::shared_ptr<CGenericWordProcessorApp> pApp, OUT BOOL &bWasCanceled)
{
	try {
		CString str;
		CString strExt = FileUtils::GetFileExtension(strPath);

		CString strErrorBegin;
		strErrorBegin.Format("'%s':\r\n", strPath);

		progressDialog.SetLine(1, "Retrieving document text...");

		if (strExt.CompareNoCase("doc") == 0 || strExt.CompareNoCase("docx") == 0) 
		{
			// word document
			if (EnsureWordApp("Parsing...")) 
			{
				// (c.haag 2016-04-22 10:56) - NX-100275 - There's no longer a need to do all of the exception
				// handling we used to do below since the word processor object does the work for us and we can
				// easily use NxCatch to get all combinations of text. We also don't need the document to persist
				// once we have the text and as a rule we should close unused objects as quickly as possible.
				// We'll just have the word processor object manage the document for us instead.
				CString strFullText, strError;
				BOOL bOpenSuccess = TRUE;
				try
				{
					// (c.haag 2016-05-10 13:27) - NX-100318 - We no longer maintain a persistent Word application in this object.
					// We don't need one either since all we do here is get document text
					// (z.manning 2016-06-02 16:07) - NX-100790 - This now returns a boolean
					bWasCanceled = !(pApp->GetAllDocumentText(strPath, strFullText));
				}
				catch (...)
				{
					// Get the unabridged error message and reset the status
					bOpenSuccess = FALSE;
					strError = NxCatch::GetErrorMessage();
					// Now rethrow so NxCatchAll will release the exeception and save us from the abstraction
					// of having to do it ourselves
					try
					{
						throw;
					}
					NxCatchAllIgnore();
				}
				
				if (bOpenSuccess) 
				{
					if (!strFullText.IsEmpty()) 
					{
						long nLength = strFullText.GetLength();

						long nLastIx = 0;
						int i = 0;
						for (i = 0; i < strFullText.GetLength(); i++) {
							if (strFullText[i] == '\r') {
								CString strLine = strFullText.Mid(nLastIx, i - nLastIx);
								strLine.Replace("\r", "");
								strLine.Replace("\n", "");

								saText.Add(strLine);

								nLastIx = i+1;
							}
							
							progressDialog.SetProgress(nCurrent + ( (i + 1 / nLength) * (m_cnProgressResolution / 2)), nTotal);
							if (progressDialog.HasUserCancelled()) 
							{
								return FALSE;
							}
						}
						if (nLastIx < strFullText.GetLength()) {
							CString strLine = strFullText.Mid(nLastIx, strFullText.GetLength() - nLastIx);
							strLine.Replace("\r", "");
							strLine.Replace("\n", "");

							saText.Add(strLine);
							
							progressDialog.SetProgress(nCurrent + (m_cnProgressResolution / 2), nTotal);
							if (progressDialog.HasUserCancelled())
							{
								return FALSE;
							}
						}
					}
					return TRUE;
				} 
				else 
				{
					strErrors += strErrorBegin + strError + "\r\n";
					return FALSE;
				}
			}

			return FALSE;
		} else {
			// try as a text file
			CStdioFile f;
			if (f.Open(strPath, CFile::modeRead | CFile::shareCompat)) {
				// (a.walling 2008-10-02 10:44) - PLID 31567 - Cast to DWORD
				DWORD dwLength = (DWORD)f.GetLength();
				long nCurPos = 0;
				CString strLine;
				while (f.ReadString(strLine)) {
					nCurPos += strLine.GetLength();
					progressDialog.SetProgress(nCurrent + ( (nCurPos / dwLength) * (m_cnProgressResolution / 2)), nTotal);
					if (progressDialog.HasUserCancelled()) {
						f.Close();
						return FALSE;
					}
					strLine.Replace("\r", "");
					strLine.Replace("\n", "");
					saText.Add(strLine);
				}

				f.Close();

				return TRUE;
			} else {
				strErrors += strErrorBegin + FormatLastError() + "\r\n";
				return FALSE;
			}
		}
	} catch (...) {		
		m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");

		throw;
	}

	return FALSE;
}

// (a.walling 2008-07-14 15:41) - PLID 30720 - Configurable parsing for transcriptions
// (a.wilson 2012-08-01 11:04) - PLID 51901 - add handling for provider parsing.
// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
BOOL CParseTranscriptionsDlg::ParseText(CTxDoc& doc, CParseOptions& opt, CProgressDialog &progressDialog, long nCurrent, long nTotal, std::shared_ptr<CGenericWordProcessorApp> pApp)
{
	long nCurCharPos = 0;

	CStringArray& sa = *(doc.psaText);

	if (sa.GetSize() == 0)
		return TRUE;
	
	progressDialog.SetLine(1, "Parsing...");
	if (progressDialog.HasUserCancelled()) {
		return FALSE;
	}

	// clear out any existing segments!
	doc.ClearSegments();
	
	RegEx::IRegExp2Ptr
		rxPatID,
		rxDate,
		rxBreak,
		rxCat,
		rxTitle,
		rxProvider;

	if (opt.strPatID.GetLength()) {
		rxPatID.CreateInstance(__uuidof(RegEx::RegExp));
		rxPatID->PutIgnoreCase(VARIANT_TRUE);
		rxPatID->PutPattern(AsBstr(opt.strPatID));
		rxPatID->PutGlobal(VARIANT_FALSE);
	}
	if (opt.strDate.GetLength()) {
		rxDate.CreateInstance(__uuidof(RegEx::RegExp));
		rxDate->PutIgnoreCase(VARIANT_TRUE);
		rxDate->PutPattern(AsBstr(opt.strDate));
		rxDate->PutGlobal(VARIANT_FALSE);
	}
	if (opt.strBreak.GetLength()) {
		rxBreak.CreateInstance(__uuidof(RegEx::RegExp));
		rxBreak->PutIgnoreCase(VARIANT_TRUE);
		rxBreak->PutPattern(AsBstr(opt.strBreak));
		rxBreak->PutGlobal(VARIANT_FALSE);
	}
	if (opt.strCat.GetLength()) {
		rxCat.CreateInstance(__uuidof(RegEx::RegExp));
		rxCat->PutIgnoreCase(VARIANT_TRUE);
		rxCat->PutPattern(AsBstr(opt.strCat));
		rxCat->PutGlobal(VARIANT_FALSE);
	}
	if (opt.strTitle.GetLength()) {
		rxTitle.CreateInstance(__uuidof(RegEx::RegExp));
		rxTitle->PutIgnoreCase(VARIANT_TRUE);
		rxTitle->PutPattern(AsBstr(opt.strTitle));
		rxTitle->PutGlobal(VARIANT_FALSE);
	}
	if (opt.strProvider.GetLength()) {	// (a.wilson 2012-08-02 13:59) - PLID 51901
		rxProvider.CreateInstance(__uuidof(RegEx::RegExp));
		rxProvider->PutIgnoreCase(VARIANT_TRUE);
		rxProvider->PutPattern(AsBstr(opt.strProvider));
		rxProvider->PutGlobal(VARIANT_FALSE);
	}

	//TRACE("\nCreating new segment with UserDefinedID %li...\n", -1);
	CTxSeg* pSeg = new CTxSeg;
	pSeg->nBeginCharIndex = 0;
	doc.arSegs.Add(pSeg);

	BOOL bNonEmptyFound = FALSE;

	for (int i = 0; i < sa.GetSize(); i++) {
		CString strLine = sa[i];
		_bstr_t bstrLine = AsBstr(strLine);

		if (!strLine.IsEmpty()) {
			if (strLine == "\f") {
				// new page, new segment.
				pSeg->nEndCharIndex = nCurCharPos - 1 + strLine.GetLength() + 2;
				if (pSeg->nLineCount == 0) {
					delete pSeg;
					doc.arSegs.RemoveAt(doc.arSegs.GetSize() - 1);
				}
				
				pSeg = new CTxSeg;
				pSeg->nBeginCharIndex = nCurCharPos + strLine.GetLength() + 2;
				pSeg->nUserDefinedID = LONG_MIN;
				doc.arSegs.Add(pSeg);
			} else {
				bNonEmptyFound = TRUE;

				if (rxBreak) {
					VARIANT_BOOL bResult = rxBreak->Test(bstrLine);

					if (bResult != VARIANT_FALSE) {
						// Section break
						//TRACE("\nSection break Pattern match - '%s' (%li) matched using pattern '%s'\n", strLine, i, (LPCTSTR)rxBreak->GetPattern());
						pSeg->nEndCharIndex = nCurCharPos;
						if (pSeg->nLineCount == 0) {
							delete pSeg;
							doc.arSegs.RemoveAt(doc.arSegs.GetSize() - 1);
						}

						//TRACE("\nCreating new segment with UserDefinedID %li...\n", LONG_MIN);
						pSeg = new CTxSeg;
						pSeg->nBeginCharIndex = nCurCharPos;
						pSeg->nUserDefinedID = LONG_MIN;
						doc.arSegs.Add(pSeg);
					}
				}

				if (rxPatID) {
					VARIANT_BOOL bResult = rxPatID->Test(bstrLine);

					if (bResult == VARIANT_TRUE) {
						// pattern match! we have a patient ID
						RegEx::IMatchCollection2Ptr pMatches = rxPatID->Execute(bstrLine);

						long nCount = pMatches->GetCount();
						
						if (nCount > 0) {
							long nCurUserDefinedID = LONG_MIN;
							RegEx::IMatch2Ptr pMatch = pMatches->GetItem(0);
							if (pMatch) {
								RegEx::ISubMatchesPtr pSubMatches = pMatch->GetSubMatches();
								if (pSubMatches) {
									long nSubCount = pSubMatches->GetCount();
									/*
									for (int s = 0; s < nSubCount; s++) {
										_variant_t var = pSubMatches->GetItem(s);
										CString strMatchums = var.vt == VT_NULL ? "null" : (var.vt == VT_EMPTY ? "empty" : VarString(var, ""));
										TRACE("\t\tSubMatch %li:\t%s\n", s, strMatchums);
									}
									*/
									if (nSubCount > 0) {
										CString strID = VarString(pSubMatches->GetItem(0), "");
										if (!strID.IsEmpty()) {
											//TRACE("\nPattern sub match - '%s' (%li) matched '%s' using pattern '%s'\n", strLine, i, strID, (LPCTSTR)rxPatID->GetPattern());
											
											nCurUserDefinedID = atoi(strID);
											if (nCurUserDefinedID == 0 && strID != "0") {
												ASSERT(FALSE);
												//TRACE("\tCould not determine ID\n");
												// could not get the user def id!
												nCurUserDefinedID = LONG_MIN;
											}
										}
									}
								}
							}

							if (opt.bBreakOnPatID) {
								pSeg->nEndCharIndex = nCurCharPos;
								if (pSeg->nLineCount == 0) {
									delete pSeg;
									doc.arSegs.RemoveAt(doc.arSegs.GetSize() - 1);
								}

								//TRACE("\nCreating new segment with UserDefinedID %li...\n", nCurUserDefinedID);
								pSeg = new CTxSeg;
								pSeg->nBeginCharIndex = nCurCharPos;
								pSeg->nUserDefinedID = nCurUserDefinedID;
								doc.arSegs.Add(pSeg);
							} else {
								if (pSeg->nUserDefinedID != LONG_MIN && pSeg->nUserDefinedID != nCurUserDefinedID) {
									//TRACE("\tUserDefinedID %li overwritten by %li!\n", pSeg->nUserDefinedID, nCurUserDefinedID);
								}
								pSeg->nUserDefinedID = nCurUserDefinedID;
							}
						}
					}
				}

				if (rxDate) {
					VARIANT_BOOL bResult = rxDate->Test(bstrLine);

					if (bResult == VARIANT_TRUE) {
						// it appears we have a date!
						RegEx::IMatchCollection2Ptr pMatches = rxDate->Execute(bstrLine);

						long nCount = pMatches->GetCount();

						if (nCount > 0) {
							RegEx::IMatch2Ptr pMatch = pMatches->GetItem(0);

							if (pMatch) {
								RegEx::ISubMatchesPtr pSubMatches = pMatch->GetSubMatches();
								if (pSubMatches) {
									long nSubCount = pSubMatches->GetCount();
									/*
									for (int s = 0; s < nSubCount; s++) {
										_variant_t var = pSubMatches->GetItem(s);
										CString strMatchums = var.vt == VT_NULL ? "null" : (var.vt == VT_EMPTY ? "empty" : VarString(var, ""));
										TRACE("\t\tSubMatch %li:\t%s\n", s, strMatchums);
									}
									*/
									if (nSubCount > 0) {
										CString strDate = VarString(pSubMatches->GetItem(0), "");
										if (!strDate.IsEmpty()) {											
											//TRACE("\nDate pattern sub match - '%s' (%li) matched '%s' using pattern '%s'\n", strLine, i, strDate, (LPCTSTR)rxDate->GetPattern());

											COleDateTime dt;
											dt.ParseDateTime(strDate);

											if (dt.GetStatus() != COleDateTime::valid) {
												CleanOrdinalsFromLongDateString(strDate);
												dt.ParseDateTime(strDate);
												if (dt.GetStatus() != COleDateTime::valid) {
													//TRACE("\tUnable to translate '%s' into valid datetime\n", strDate);
												} else {
													pSeg->dtDate = dt;
												}
											} else {
												pSeg->dtDate = dt;
											}
										}
									}
								}
							}
						}
					}
				}

				if (rxCat) {
					VARIANT_BOOL bResult = rxCat->Test(bstrLine);

					if (bResult == VARIANT_TRUE) {
						// it appears we have a category!
						RegEx::IMatchCollection2Ptr pMatches = rxCat->Execute(bstrLine);

						long nCount = pMatches->GetCount();

						if (nCount > 0) {
							RegEx::IMatch2Ptr pMatch = pMatches->GetItem(0);

							if (pMatch) {
								RegEx::ISubMatchesPtr pSubMatches = pMatch->GetSubMatches();
								if (pSubMatches) {
									long nSubCount = pSubMatches->GetCount();
									/*
									for (int s = 0; s < nSubCount; s++) {
										_variant_t var = pSubMatches->GetItem(s);
										CString strMatchums = var.vt == VT_NULL ? "null" : (var.vt == VT_EMPTY ? "empty" : VarString(var, ""));
										TRACE("\t\tSubMatch %li:\t%s\n", s, strMatchums);
									}
									*/
									if (nSubCount > 0) {
										CString strCat = VarString(pSubMatches->GetItem(0), "");
										if (!strCat.IsEmpty()) {											
											//TRACE("\nCategory pattern sub match - '%s' (%li) matched '%s' using pattern '%s'\n", strLine, i, strCat, (LPCTSTR)rxCat->GetPattern());

											pSeg->strCat = strCat;
										}
									}
								}
							}
						}
					}
				}
				// (a.wilson 2012-08-02 13:59) - PLID 51901 - check for provider information.
				if (rxProvider) {
					VARIANT_BOOL bResult = rxProvider->Test(bstrLine);

					if (bResult == VARIANT_TRUE) {

						RegEx::IMatchCollection2Ptr pMatches = rxProvider->Execute(bstrLine);

						long nCount = pMatches->GetCount();

						if (nCount > 0) {
							RegEx::IMatch2Ptr pMatch = pMatches->GetItem(0);

							if (pMatch) {
								RegEx::ISubMatchesPtr pSubMatches = pMatch->GetSubMatches();
								if (pSubMatches) {
									long nSubCount = pSubMatches->GetCount();

									if (nSubCount > 0) {
										CString strProvider = VarString(pSubMatches->GetItem(0), "");

										if (!strProvider.IsEmpty()) {											
											pSeg->strProvider = strProvider;
										}
									}
								}
							}
						}
					}
				}

				if (rxTitle) {
					VARIANT_BOOL bResult = rxTitle->Test(bstrLine);

					if (bResult == VARIANT_TRUE) {
						// it appears we have a title!
						RegEx::IMatchCollection2Ptr pMatches = rxTitle->Execute(bstrLine);

						long nCount = pMatches->GetCount();

						if (nCount > 0) {
							RegEx::IMatch2Ptr pMatch = pMatches->GetItem(0);

							if (pMatch) {
								RegEx::ISubMatchesPtr pSubMatches = pMatch->GetSubMatches();
								if (pSubMatches) {
									long nSubCount = pSubMatches->GetCount();
									/*
									for (int s = 0; s < nSubCount; s++) {
										_variant_t var = pSubMatches->GetItem(s);
										CString strMatchums = var.vt == VT_NULL ? "null" : (var.vt == VT_EMPTY ? "empty" : VarString(var, ""));
										TRACE("\t\tSubMatch %li:\t%s\n", s, strMatchums);
									}
									*/
									if (nSubCount > 0) {
										CString strTitle = VarString(pSubMatches->GetItem(0), "");
										if (!strTitle.IsEmpty()) {											
											//TRACE("\nTitle pattern sub match - '%s' (%li) matched '%s' using pattern '%s'\n", strLine, i, strTitle, (LPCTSTR)rxTitle->GetPattern());

											pSeg->strTitle = strTitle;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		if (bNonEmptyFound) {
			pSeg->nLineCount++;
		}

		nCurCharPos += strLine.GetLength() + 2;

		progressDialog.SetProgress(nCurrent + ( ((i + 1) / sa.GetSize()) * (m_cnProgressResolution / 2)), nTotal);
		if (progressDialog.HasUserCancelled()) {
			return FALSE;
		}
	}

	// (a.walling 2008-08-08 13:30) - PLID 28838 - Whups, ensure we save the last one!
	if (pSeg != NULL && pSeg->nBeginCharIndex != -1 && pSeg->nEndCharIndex == -1) {
		pSeg->nEndCharIndex = nCurCharPos;
		if (pSeg->nLineCount == 0) {
			delete pSeg;
			doc.arSegs.RemoveAt(doc.arSegs.GetSize() - 1);
		}
	}

	return TRUE;
}

void CParseTranscriptionsDlg::OnCurSelWasSetTree() 
{
	try {
		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {
			// get parent row
			IRowSettingsPtr pTopRow = pRow;
			while (pTopRow->GetParentRow()) {
				pTopRow = pTopRow->GetParentRow();
			}

			m_nxibSetToSelection.SetWindowText("Select Segment Area Within Document");
			m_bEditSelectionMode = FALSE;

			CTxDoc* pDoc = (CTxDoc*)VarLong(pTopRow->GetValue(ecObject), NULL);
			ASSERT(pDoc);

			if (pDoc && pDoc->psaText) {
				if (pRow->IsSameRow(pTopRow) == VARIANT_FALSE) {
					long nSegID = VarLong(pRow->GetValue(ecID));
					_ASSERTE(nSegID < pDoc->arSegs.GetSize());
					CTxSeg* pSeg = pDoc->arSegs[nSegID];

					if (pSeg) {
						DisplaySegment(pSeg, pDoc);
					} else {
						ASSERT(FALSE);
						m_text.SetWindowText(DEFAULT_TX_INSTRUCTIONS);
					}
					return;
				} else {
					// whole document was selected
					CString strText = pDoc->GetText();
					
					m_text.SetWindowText(strText);
				}
			} else {
				// (a.walling 2008-08-08 13:07) - PLID 28838 - I would love to put some asynchronous loading code
				// in here for word documents, but I think that's a bit of an investment for such a minor return.
				// Instead let's just tell them what is up.
				m_text.SetWindowText(CString("The entire text of this document will display here after being parsed. Click 'Parse All' to parse all documents.\r\n\r\n") + DEFAULT_TX_INSTRUCTIONS);
			}
		} else {
			// (a.walling 2008-08-08 12:49) - PLID 28838 - No selection, so reset to the info text
			m_text.SetWindowText(DEFAULT_TX_INSTRUCTIONS);
		}

		m_nxbReviewed.SetCheck(BST_UNCHECKED);
		m_nxbReviewed.EnableWindow(FALSE);
		m_nxibSetToSelection.EnableWindow(FALSE);
		m_nxibEditText.AutoSet(NXB_MODIFY, NXIB_TEXTCOLOR);
		m_nxibEditText.EnableWindow(FALSE);
		m_editTitle.EnableWindow(FALSE);
		m_bSettingEditText = TRUE;
		m_editTitle.SetWindowText("");
		m_bSettingEditText = FALSE;
		m_dlPat->PutEnabled(VARIANT_FALSE);
		m_dlPat->PutCurSel(NULL);
		m_dlCat->PutEnabled(VARIANT_FALSE);
		// (a.wilson 2012-08-02 17:08) - PLID 51932 - update based on import destination.
		if (m_opt.bImportIntoEMR)
			m_dlCat->FindByColumn(0, _variant_t((long)-1), NULL, VARIANT_TRUE);
		else
			m_dlCat->FindByColumn(0, _variant_t((long)-2), NULL, VARIANT_TRUE);
		m_dlProvider->PutEnabled(VARIANT_FALSE);	// (a.wilson 2012-08-02 11:09) - PLID 51901 - reset if non-selection.
		m_dlProvider->FindByColumn(epID, (long)-1, NULL, VARIANT_TRUE);
		m_nxTime->PutEnabled(VARIANT_FALSE);
		m_nxTime->Clear();
	} NxCatchAll("Error in CParseTranscriptionsDlg::OnCurSelWasSetTree");
}

void CParseTranscriptionsDlg::OnSelSetPatlist(LPDISPATCH lpSel) 
{
	try {
		IRowSettingsPtr pSelectedRow(lpSel);

		if (pSelectedRow) {
			IRowSettingsPtr pRow = m_dl->GetCurSel();

			if (pRow) {
				IRowSettingsPtr pParentRow = pRow->GetParentRow();

				if (pParentRow) {			
					CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

					if (pDoc) {
						long nIx = VarLong(pRow->GetValue(ecID), -1);
						if (nIx >= 0) {
							CTxSeg* pSeg = pDoc->arSegs[nIx];

							if (pSeg) {
								long nNewUserDefinedID = VarLong(pSelectedRow->GetValue(ecUserDefinedID), LONG_MIN);

								pSeg->nUserDefinedID = nNewUserDefinedID;

								CString strName;
								strName = VarString(pSelectedRow->GetValue(ecLast), "") + ", " + VarString(pSelectedRow->GetValue(ecFirst), "") + " " + VarString(pSelectedRow->GetValue(ecMiddle), "");
								strName += FormatString("(%li)", pSeg->nUserDefinedID);
								pSeg->nPatientID = VarLong(pSelectedRow->GetValue(ecPersonID), -1);

								// (j.jones 2013-02-27 16:35) - PLID 55119 - clear the error on the name/label columns
								UpdateRowErrorStatus(pRow, ecName, false);

								pRow->PutValue(ecName, _variant_t(strName));

								//this won't check the box, it will simply replace the null checkbox
								//with a checkable box
								BOOL bCommit = VarBool(pRow->GetValue(ecCommit), FALSE);
								if(!bCommit) {
									pRow->PutValue(ecCommit, g_cvarFalse);
								}
							}
						}
					}
				}
			}
		}	
		
	} NxCatchAll("Error in OnSelSetPatlist");
}

// (a.walling 2008-07-16 11:23) - PLID 30751 - Now, actually IMPORT everything into Practice's history!
void CParseTranscriptionsDlg::OnBtnImport()
{
	try {
		// (c.haag 2016-06-01 11:48) - NX-100320 - Fail if we could not get a Word instance
		std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
		if (nullptr == pApp)
		{
			return;
		}

		// (a.wilson 2012-07-25 15:31) - PLID 52305 - check their license to see if their allowed to use an EMR configuraton.
		if (m_opt.bImportIntoEMR) {
			//warn them that they cannot import to emr with an expired license or no license at all.
			if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
				MessageBox("The transcription parsing configuration is set to import "
					"into a new patient EMN but your license for EMR is expired or inactive."
					"\r\nPlease contact Nextech Techincal Support for assistance or change the "
					"configuration to import into patient history.");
				return;
			}
			//warn them that they do not have permission to create emns and therefore cannot import into emr.
			if (!CheckHasEMRCreateAndWritePermissions()) {
				MessageBox("The transcription parsing configuration is set to import "
					"into a new patient EMN but you do not have permission to create and write to EMR.");
				return;
			}

			//warn them that they don't have a valid template selected.
			if (!ReturnsRecordsParam("SELECT TOP (1) ID FROM EMRTemplateT WHERE ID = {INT} AND Deleted = 0", m_opt.nTemplateID)) {
				MessageBox("The transcription parsing configuration is set to an invalid template."
					"\r\nPlease select a valid template in the configuration before importing.");
				return;
			}

			//warn them that the selected template does not contain a text detail.
			if (!ReturnsRecordsParam("SELECT TOP 1 EMRI.ID FROM EMRTemplateDetailsT EMRTD "
				"INNER JOIN EMRInfoMasterT EMRIM ON EMRIM.ID = EMRTD.EmrInfoMasterID "
				"INNER JOIN EMRInfoT EMRI ON EMRI.ID = EMRIM.ActiveEMRInfoID "
				"WHERE EMRI.DataType = 1 AND EMRTD.TemplateID = {INT}", m_opt.nTemplateID)) {
				MessageBox("The transcription parsing configurations is set to a template that does not contain a text detail."
					"\r\nPlease select a template which contains atleast one text detail to import the transcription.");
				return;
			}

		}

		IRowSettingsPtr pDefCatRow = m_dlDefCat->GetCurSel();
		if (pDefCatRow) {
			long nDefCatID = VarLong(pDefCatRow->GetValue(0), -1);
			if (nDefCatID != m_nDefaultCategoryID) {
				MessageBox("Please set a default category for transcriptions.", NULL, MB_ICONHAND);
				return;
			}
		} else {		
			MessageBox("Please set a default category for transcriptions.", NULL, MB_ICONHAND);
			return;
		}

		// (j.jones 2013-03-04 10:15) - PLID 55120 - if they want to move transcription files
		// after importing, verify the folder exists
		if(m_opt.bMoveImportedFiles) {
			if(m_opt.strPostImportFolder.IsEmpty()) {
				MessageBox("The transcription parsing configuration is set to move imported transcription files to another folder, "
					"but no folder is selected to move the files into.\n\n"
					"Please correct this in the 'Configure...' screen before importing.", "Practice", MB_ICONHAND);
				return;
			}
			else if(!DoesExist(m_opt.strPostImportFolder)) {
				CString strWarning;
				strWarning.Format("The transcription parsing configuration is set to move imported transcription files to another folder, "
					"but the folder selected to move transcriptions into, %s, does not exist.\n\n"
					"Please correct this in the 'Configure...' screen before importing.", m_opt.strPostImportFolder);
				MessageBox(strWarning, "Practice", MB_ICONHAND);
				return;
			}
		}

		EnableWindow(FALSE);

		// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
		CProgressDialog progressDialog;
		//remarkably, the hwnd has to be NULL for this to work
		progressDialog.Start(NULL, CProgressDialog::AutoTime | CProgressDialog::NoMinimize, 
			"Importing transcription files...", "Attempting to cancel...");

		long nTotal = 0, nCurrent = 0;
		{
			IRowSettingsPtr pCountRow = m_dl->GetFirstRow();
			while (pCountRow) {
				// (j.jones 2013-03-05 12:45) - PLID 55121 - if we cannot import this file,
				// skip it entirely
				if(CanImportFile(pCountRow)) {

					IRowSettingsPtr pChildRow = pCountRow->GetFirstChildRow();
					while (pChildRow) {
						// (j.jones 2013-02-28 15:53) - PLID 55121 - NeedCommitRow will return true
						// if Commit is true and if the row isn't disallowed from being imported due to errors
						if(NeedCommitRow(pChildRow)) {
							nTotal++;
						}

						pChildRow = pChildRow->GetNextRow();
					}
				}

				pCountRow = pCountRow->GetNextRow();
			}
		}

		progressDialog.SetProgress(0, nTotal);

		m_dlPat->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
		//0 is a hard coded value for Open, and is the default value
		long nDefaultEMNStatus = 0;
		if(m_opt.bImportIntoEMR) {
			IRowSettingsPtr pRow = m_EMNStatusCombo->GetCurSel();
			if (pRow) {
				nDefaultEMNStatus = VarLong(pRow->GetValue(escID), 0);
			}

			//make sure this is a valid status (check even if 0, though if 0 doesn't exist they have bad data)
			if(!ReturnsRecordsParam("SELECT ID FROM EMRStatusListT WHERE ID = {INT}", nDefaultEMNStatus)) {
				MessageBox("An invalid default EMN Status is selected. Please select a different default EMN Status before importing.");
				return;
			}
		}

		IRowSettingsPtr pTopRow = m_dl->GetFirstRow();

		long nSuccess = 0, nFailed = 0;
		long nAvailable = 0;
		long nTotalSegmentCount = 0;

		CString strFilesSkippedDueToErrors;

		while (pTopRow) {

			// (j.jones 2013-03-05 12:45) - PLID 55121 - if we cannot import this file,
			// we will skip it entirely, but we'll count segments first
			BOOL bCanImportFile = CanImportFile(pTopRow);
			if(!bCanImportFile) {
				//track the name
				if(!strFilesSkippedDueToErrors.IsEmpty()) {
					strFilesSkippedDueToErrors += "\n";
				}
				strFilesSkippedDueToErrors += VarString(pTopRow->GetValue(ecName));
			}

			CTxDoc* pDoc = (CTxDoc*)VarLong(pTopRow->GetValue(ecObject), NULL);

			bool bHasOneImported = false;

			if (pDoc) {
				IRowSettingsPtr pChildRow = pTopRow->GetFirstChildRow();

				while (pChildRow != NULL && !progressDialog.HasUserCancelled()) {
					nTotalSegmentCount++;

					BOOL bSucceeded = FALSE;
					long nIx = VarLong(pChildRow->GetValue(ecID), -1);
					// (j.jones 2013-02-28 15:53) - PLID 55121 - CanCommitRow will return true
					// if Commit is checkable (it might not be checked) and if the row isn't
					// disallowed from being imported due to errors
					// (j.jones 2013-03-05 12:45) - PLID 55121 - if we cannot import this file,
					// we will skip it entirely
					if(bCanImportFile && CanCommitRow(pChildRow)) {
						nAvailable++;

						// (j.jones 2013-02-28 15:53) - PLID 55121 - NeedCommitRow will return true
						// if Commit is true and if the row isn't disallowed from being imported due to errors
						if(NeedCommitRow(pChildRow)) {
							nCurrent++;
							progressDialog.SetProgress(nCurrent, nTotal);

							if(nIx >= 0) {
								CTxSeg* pSeg = pDoc->arSegs[nIx];

								if (pSeg) {
									// (a.wilson 2012-07-25 17:06) - PLID 33429 Here we will either import to history or a new emn.
									if (m_opt.bImportIntoEMR) { //import into emr
										// (j.jones 2013-03-01 11:15) - PLID 55122 - added default EMN status
										if (SaveToPatientEMN(nDefaultEMNStatus, pDoc, pSeg, progressDialog))
											bSucceeded = TRUE;

									} else { //import into history
										if (SaveSegmentToHistory(pDoc, pSeg, progressDialog, pApp))
											bSucceeded = TRUE;
									}
									//check whether succeeded of failed.
									if (bSucceeded) {
										nSuccess++;
										pChildRow->PutValue(ecCommit, VARIANT_FALSE);
									} else {
										nFailed++;
									}
								}
							}
						}
					}

					if (bSucceeded) {
						IRowSettingsPtr pThisRow = pChildRow;
						pChildRow = pChildRow->GetNextRow();
						m_dl->RemoveRow(pThisRow);

						// (j.jones 2013-03-04 10:37) - PLID 55120 - track that a child row was removed
						// due to a successful import
						bHasOneImported = true;
					} else {
						pChildRow = pChildRow->GetNextRow();
					}
				}

			}

			// (j.jones 2013-03-04 10:33) - PLID 55120 - cache the parent row,
			// then advance to the next row before trying to remove the parent row
			IRowSettingsPtr pParentRow = pTopRow;
			pTopRow = pTopRow->GetNextRow();

			// (j.jones 2013-03-04 10:33) - PLID 55120 - try to remove the parent row if there are no children left
			if(bHasOneImported) {				
				if(pParentRow != NULL && pParentRow->GetFirstChildRow() == NULL) {
					//this call will not prompt the user
					if(TryMoveImportedTranscriptionFile(VarString(pParentRow->GetValue(ecFilePath), ""))) {
						//success, remove the parent row

						CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), 0);
						if (pDoc) {
							delete pDoc;
						}

						m_dl->RemoveRow(pParentRow);
					}
				}
			}
		}
		
		m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");
		
		progressDialog.Stop();

		// (j.jones 2013-03-04 11:18) - PLID 55117 - We may have imported records that had errors.
		// Update the label now that those records are gone.
		UpdateErrorCountLabel();

		EnableWindow(TRUE);

		CString strMessage;
		bool bTotalFailure = false;
		if (nSuccess > 0 || nFailed > 0) {
			if (nSuccess > 0) {
				strMessage += FormatString("Successfully imported %li transcriptions.\r\n", nSuccess);
				
				// (j.jones 2014-08-04 13:31) - PLID 63141 - removed the the tablechecker call here,
				// because if we saved in history then CreateMailSentEntry already sent one for us
			}
			if (nFailed > 0) {
				strMessage += FormatString("%li transcriptions failed to import. These segments will remain checked.\r\n", nFailed);
			}

			strMessage.TrimRight("\r\n");

		} else if (nTotalSegmentCount == 0) {
			strMessage = "There are no segments to import.";
			bTotalFailure = true;
		} else if (nAvailable == 0) {
			strMessage = "There are no available segments to import. Please ensure errors are resolved prior to importing.";
			bTotalFailure = true;
		} else if (nSuccess + nFailed == 0) {
			strMessage = "No available segments are checked. Please check the box next to each segment you wish to import.";
			bTotalFailure = true;
		}

		// (j.jones 2013-03-05 13:13) - PLID 55121 - add the list of files that were skipped, if any
		if(!strFilesSkippedDueToErrors.IsEmpty()) {
			CString strFilesMsg;
			strFilesMsg = "\n\nThe following files were not imported due to having at least one error:\n";
			strFilesMsg += strFilesSkippedDueToErrors;
			strMessage += strFilesMsg;
		}

		MessageBox(strMessage, "Practice", bTotalFailure ? MB_ICONHAND : MB_ICONINFORMATION);

		return;
	} NxCatchAll("Error in OnBtnImport");

	EnableWindow(TRUE);	
	m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");
}
// (a.wilson 2012-08-02 15:07) - PLID 51932 - if they changed the config then we need to change the categories as well.
void CParseTranscriptionsDlg::OnBtnConfigure()
{
	try {
		BOOL bPreChangeImportEMR = m_opt.bImportIntoEMR;
		BOOL bPreChangeIncludeInactiveProviders = m_opt.bIncludeInactiveProviders;
		CParseTranscriptionsConfigDlg dlg(m_opt, this);

		if (IDOK == dlg.DoModal()) {

			long nExisting = 0;
			IRowSettingsPtr pRow = m_dl->GetFirstRow();
			while (pRow) {
				IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();

				while (pChildRow) {
					nExisting++;
					pChildRow = pChildRow->GetNextRow();
				}
				pRow = pRow->GetNextRow();
			}

			// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
			BOOL bPostChangeIncludeInactiveProviders = dlg.GetParseOptions().bIncludeInactiveProviders;
			if(bPostChangeIncludeInactiveProviders != bPreChangeIncludeInactiveProviders) {
				//always requery if the setting changed, it's ok if they already linked
				//inactive providers to records even if we're hiding them now, as those
				//records will continue to use their selected providers
				long nCurSelID = -1;
				CString strComboBoxText = (LPCTSTR)m_dlProvider->GetComboBoxText();
				{
					IRowSettingsPtr pRow = m_dlProvider->GetCurSel();
					if(pRow) {
						nCurSelID = VarLong(pRow->GetValue(ecID),-1);
					}
				}
				CString strProviderWhere = "";
				if(!bPostChangeIncludeInactiveProviders) {
					strProviderWhere = "Archived = 0";
				}
				m_dlProvider->PutWhereClause((LPCTSTR)strProviderWhere);
				m_dlProvider->Requery();

				//set the old selection
				IRowSettingsPtr pRow = m_dlProvider->SetSelByColumn(ecID, (long)nCurSelID);
				if(pRow == NULL) {
					//if none was found, set to <none>
					m_dlProvider->SetSelByColumn(ecID, (long)-1);
				}
				//set the combo box text, if there was a selection before, or if the selection was -1
				if((pRow == NULL || nCurSelID == -1) && !strComboBoxText.IsEmpty()) {
					m_dlProvider->PutComboBoxText((LPCTSTR)strComboBoxText);
				}

				//set the combo box text, if there wasn't a selection before
				if(nCurSelID == -1 && !strComboBoxText.IsEmpty()) {
					m_dlProvider->PutComboBoxText((LPCTSTR)strComboBoxText);
				}
			}

			BOOL bPostChangeImportEMR = dlg.GetParseOptions().bImportIntoEMR;
			//if they changed the import destination while still having parsed files
			//then we need to provide them an option to either revert option changes or reparse the files.
			if (bPreChangeImportEMR != bPostChangeImportEMR) {
				if (nExisting > 0) {
					if (IDYES == MsgBox(MB_YESNO, "You have modified the import destination and currently have %li segments parsed. "
						"In order to ensure valid category selections you may cancel any changes made to the configuration or reparse the current files."
						"\r\n\r\nSelect 'Yes' to reparse the current segments.\r\nSelect 'No' to cancel configuration changes.", nExisting)) {
						m_opt = dlg.GetParseOptions();
						EnsureCategories();
						EnsureProviderEnabled();	// (a.wilson 2012-09-10 09:57) - PLID 51901
						// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
						EnsureEMNStatus();
						ParseAll(true);

						//reset the existing count to zero, because we just re-parsed using the new settings
						nExisting = 0;

						m_opt.Save();
					} else {
						//nothing will change and old parsing options will stay the same.
						return;
					}
				} else {
					m_opt = dlg.GetParseOptions();
					EnsureCategories();
					EnsureProviderEnabled();	// (a.wilson 2012-09-10 09:57) - PLID 51901
					// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
					EnsureEMNStatus();
					m_opt.Save();
				}
			} else {
				m_opt = dlg.GetParseOptions();
				m_opt.Save();
			}
			// (a.wilson 2012-10-15 15:03) - PLID 53129 - check to see if they want segment reviews to be ignored.
			m_nxbReviewed.ShowWindow(m_opt.bIgnoreSegmentReview ? FALSE : TRUE);

			// (j.jones 2013-02-28 13:28) - PLID 55123 - If we included inactive providers,
			// and there are existing segments (this would be reverted to 0 if we already re-parsed),
			// then warn them that they would need to parse again to link to those providers.
			// This warning is not necessary if they are not importing to EMR, because we do not link
			// records to providers unless importing to EMR.
			if(m_opt.bImportIntoEMR && bPostChangeIncludeInactiveProviders && !bPreChangeIncludeInactiveProviders && nExisting > 0) {
				if (IDYES == MsgBox(MB_YESNO, "You have modified the import settings to allow linking to inactive providers, and currently have %li segments parsed. "
					"These existing records will not automatically link to inactive providers until they are reparsed. They can, however, be manually linked to these providers.\n\n"
					"Would you like to reparse the current segments?", nExisting)) {
					ParseAll(true);
				}
			}

			// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
			EnsureEMNStatus();
		}
	} NxCatchAll("Error in OnBtnConfigure");
}

void CParseTranscriptionsDlg::CParseOptions::Load()
{
	// load defaults
	try {
		CParseOptions defaultOptions;
		defaultOptions.ResetToDefaults();

		// (a.walling 2008-08-19 15:29) - PLID 30720 - Changed to memo
		strPatID = GetRemotePropertyMemo("ParseOptionsPatID_Memo", defaultOptions.strPatID, 0, "<None>", true);

		strDate = GetRemotePropertyMemo("ParseOptionsDate_Memo", defaultOptions.strDate, 0, "<None>", true);

		strBreak = GetRemotePropertyMemo("ParseOptionsBreak_Memo", defaultOptions.strBreak, 0, "<None>", true);
		
		strCat = GetRemotePropertyMemo("ParseOptionsCat_Memo", defaultOptions.strCat, 0, "<None>", true);
		
		strTitle = GetRemotePropertyMemo("ParseOptionsTitle_Memo", defaultOptions.strTitle, 0, "<None>", true);

		bBreakOnPatID = GetRemotePropertyInt("ParseOptionsBreakOnPatID", defaultOptions.bBreakOnPatID, 0, "<None>", true);

		// (a.walling 2008-07-17 16:33) - PLID 30774 - Option to create word documents
		bCreateWordDocs = GetRemotePropertyInt("ParseOptionsCreateWordDocs", defaultOptions.bCreateWordDocs, 0, "<None>", true);

		// (a.wilson 2012-07-24 12:09) - PLID 51753 - Add new options.
		bImportIntoEMR = GetRemotePropertyInt("ParseOptionsImportIntoEMR", defaultOptions.bImportIntoEMR, 0, "<None>", true);

		nTemplateID = GetRemotePropertyInt("ParseOptionsTemplateID", defaultOptions.nTemplateID, 0, "<None>", true);

		// (a.wilson 2012-08-01 10:25) - PLID 51901 - load provider options
		strProvider = GetRemotePropertyMemo("ParseOptionsProvider_Memo", defaultOptions.strProvider, 0, "<None>", true);

		// (a.wilson 2012-10-12 16:52) - PLID 53129 - new option to ignore segment review.
		bIgnoreSegmentReview = GetRemotePropertyInt("ParseOptionsIgnoreSegmentReview", defaultOptions.bIgnoreSegmentReview, 0, "<None>", true);

		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		bIncludeInactiveProviders = GetRemotePropertyInt("ParseOptionsIncludeInactiveProviders", defaultOptions.bIncludeInactiveProviders, 0, "<None>", true);

		// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
		bRequireErrorsFixed = GetRemotePropertyInt("ParseOptionsRequireErrorsFixed", defaultOptions.bRequireErrorsFixed, 0, "<None>", true);

		// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
		nEMNStatusID = GetRemotePropertyInt("ParseOptionsDefaultEMNStatus", defaultOptions.nEMNStatusID, 0, "<None>", true);

		// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
		bMoveImportedFiles = GetRemotePropertyInt("ParseOptionsMoveImportedFiles", defaultOptions.bMoveImportedFiles, 0, "<None>", true);
		strPostImportFolder = GetRemotePropertyText("ParseOptionsPostImportFolder", defaultOptions.strPostImportFolder, 0, "<None>", true);

	} NxCatchAllThrow_NoParent("Error loading parse options"); // (a.walling 2014-05-05 13:32) - PLID 61945
}

void CParseTranscriptionsDlg::CParseOptions::Save()
{
	try {
		// save to configrt
		
		// (a.walling 2008-08-19 15:29) - PLID 30720 - Changed to memo
		SetRemotePropertyMemo("ParseOptionsPatID_Memo", strPatID, 0, "<None>");

		SetRemotePropertyMemo("ParseOptionsDate_Memo", strDate, 0, "<None>");

		SetRemotePropertyMemo("ParseOptionsBreak_Memo", strBreak, 0, "<None>");
		
		SetRemotePropertyMemo("ParseOptionsCat_Memo", strCat, 0, "<None>");
		
		SetRemotePropertyMemo("ParseOptionsTitle_Memo", strTitle, 0, "<None>");

		SetRemotePropertyInt("ParseOptionsBreakOnPatID", bBreakOnPatID, 0, "<None>");

		// (a.walling 2008-07-17 16:33) - PLID 30774 - Option to create word documents
		SetRemotePropertyInt("ParseOptionsCreateWordDocs", bCreateWordDocs, 0, "<None>");

		// (a.wilson 2012-07-24 17:32) - PLID 51753 - Save new options for EMR.
		SetRemotePropertyInt("ParseOptionsImportIntoEMR", bImportIntoEMR, 0, "<None>");
		
		SetRemotePropertyInt("ParseOptionsTemplateID", nTemplateID, 0, "<None>");

		// (a.wilson 2012-08-01 10:23) - PLID 51901 - save provider transcription parsing.
		SetRemotePropertyMemo("ParseOptionsProvider_Memo", strProvider, 0, "<None>");

		// (a.wilson 2012-10-12 16:54) - PLID 53129 - new preference to ignore segment review.
		SetRemotePropertyInt("ParseOptionsIgnoreSegmentReview", bIgnoreSegmentReview, 0, "<None>");

		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		SetRemotePropertyInt("ParseOptionsIncludeInactiveProviders", bIncludeInactiveProviders, 0, "<None>");

		// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
		SetRemotePropertyInt("ParseOptionsRequireErrorsFixed", bRequireErrorsFixed, 0, "<None>");

		// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
		SetRemotePropertyInt("ParseOptionsDefaultEMNStatus", nEMNStatusID, 0, "<None>");

		// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
		SetRemotePropertyInt("ParseOptionsMoveImportedFiles", bMoveImportedFiles, 0, "<None>");
		SetRemotePropertyText("ParseOptionsPostImportFolder", strPostImportFolder, 0, "<None>");

	} NxCatchAllThrow_NoParent("Error saving parse options");
}

void CParseTranscriptionsDlg::CParseOptions::ResetToDefaults()
{
	// (a.walling 2009-10-21 17:22) - PLID 36027 - The default regex for patid would not match multiple spaces between the ID: and value
	// \bID:\s?(\d+)\b -- this is the OLD default, it should use a * selector rather than ?	
	// \bID:\s*(\d+)\b
	strPatID = "\\bID:\\s*(\\d+)\\b";

	// \bDate:\s*(((((January|February|March|April|May|June|July|August|September|October|November|December)\s+\d{1,2}(th|st|nd|rd)?,?\s+\d{2,4})|(\d{1,2}/\d{1,2}/\d{2,4}))(\s\d{1,2}:\d{2}\s*[ap]m?)?))
	strDate = "\\bDate:\\s*(((((January|February|March|April|May|June|July|August|September|October|November|December)\\s+\\d{1,2}(th|st|nd|rd)?,?\\s+\\d{2,4})|(\\d{1,2}/\\d{1,2}/\\d{2,4}))(\\s\\d{1,2}:\\d{2}\\s*[ap]m?)?))";

	strBreak = "";

	// \bCategory:\s*(.+)$
	strCat = "\\bCategory:\\s*(.+)$";

	// \bTitle:\s*(.+)$
	strTitle = "\\bTitle:\\s*(.+)$";
	bBreakOnPatID = TRUE;

	// (a.walling 2008-07-17 16:33) - PLID 30774 - Option to create word documents
	bCreateWordDocs = TRUE;

	// (a.wilson 2012-07-24 15:20) - PLID 51753 - reset new variables to defaults.
	bImportIntoEMR = FALSE;
	nTemplateID = -1;

	// (a.wilson 2012-08-01 10:22) - PLID 51901 - default for provider transcription parsing.
	strProvider = "\\bProvider:\\s*(.+)$";

	// (a.wilson 2012-10-12 16:55) - PLID 53129 - default for ignoring the segment review.
	bIgnoreSegmentReview = FALSE;

	// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
	bIncludeInactiveProviders = FALSE;

	// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
	bRequireErrorsFixed = FALSE;

	// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
	nEMNStatusID = 0; //defaults to Open

	// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder,
	// but when resetting, just turn it off, don't clear their folder path
	bMoveImportedFiles = FALSE;
}

void CParseTranscriptionsDlg::OnChangedParsetxDate() 
{
	try {
		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {			
				CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

				if (pDoc) {
					long nIx = VarLong(pRow->GetValue(ecID), -1);
					if (nIx >= 0) {
						CTxSeg* pSeg = pDoc->arSegs[nIx];

						if (pSeg) {		
							if (m_nxTime->GetStatus() == 1) {
								COleDateTime dt = m_nxTime->GetDateTime();
								if (dt.GetStatus() == COleDateTime::invalid)
									dt.SetStatus(COleDateTime::null);
								if (dt.GetStatus() != COleDateTime::null && (DATE)dt == 0)
									dt.SetStatus(COleDateTime::null);
								if ((DATE)dt == 0)
									dt.SetStatus(COleDateTime::null);

								pSeg->dtDate = dt;
							} else {
								COleDateTime dt((DATE)0);
								dt.SetStatus(COleDateTime::null);
								pSeg->dtDate = dt;
							}
						}
					}
				}
			}
		}
	} NxCatchAll("Error in OnChangedParsetxDate");
}

void CParseTranscriptionsDlg::OnSelSetCategoryList(LPDISPATCH lpSel) 
{
	try {
		IRowSettingsPtr pSelectedRow(lpSel);

		if (pSelectedRow) {
			IRowSettingsPtr pRow = m_dl->GetCurSel();

			if (pRow) {
				IRowSettingsPtr pParentRow = pRow->GetParentRow();

				if (pParentRow) {			
					CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

					if (pDoc) {
						long nIx = VarLong(pRow->GetValue(ecID), -1);
						if (nIx >= 0) {
							CTxSeg* pSeg = pDoc->arSegs[nIx];

							if (pSeg) {
								CString strCatName;
								long nNewCatID = VarLong(pSelectedRow->GetValue(0), -1);

								if (nNewCatID == -2) {
									pSeg->strCat.Empty();
									pSeg->nCategoryID = nNewCatID;
									strCatName = "<Default Category>";
								} else if (nNewCatID == -1) {
									pSeg->strCat.Empty();
									pSeg->nCategoryID = nNewCatID;
								} else {
									strCatName = pSeg->strCat = VarString(pSelectedRow->GetValue(1), "");
									pSeg->nCategoryID = nNewCatID;						
								}

								pRow->PutValue(ecCategory, _variant_t(strCatName));

								// (j.jones 2013-02-27 16:35) - PLID 55119 - clear the error on the category column
								UpdateRowErrorStatus(pRow, ecCategory, false);

								m_dlCat->PutComboBoxText(_bstr_t(strCatName));
							}
						}
					}
				}
			}
		}	
	} NxCatchAll("Error in OnSelSetCategoryList");
}

void CParseTranscriptionsDlg::OnChangeEditTitle() 
{
	try {
		if (m_bSettingEditText)
			return;

		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {			
				CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

				if (pDoc) {
					long nIx = VarLong(pRow->GetValue(ecID), -1);
					if (nIx >= 0) {
						CTxSeg* pSeg = pDoc->arSegs[nIx];

						if (pSeg) {		
							m_editTitle.GetWindowText(pSeg->strTitle);
						}
					}
				}
			}
		}	
	} NxCatchAll("Error in OnChangeEditTitle");
}

void CParseTranscriptionsDlg::CleanOrdinalsFromLongDateString(CString& str)
{
	// somewhat brute force, but I am not too concerned.
	// Also makes the string lowercase. This is fine for our purposes since we just
	// want to parse it and get the raisin!
	str.MakeLower();

	for (int i = 1; i < 32; i++) {
		CString strFind = FormatString("%li", i);

		switch(i) {
		case 1:
			strFind += "st"; break;
		case 2:
			strFind += "nd"; break;
		case 3:
			strFind += "rd"; break;
		default:
			strFind += "th"; break;
		}

		str.Replace(strFind, FormatString("%li", i));
	}
}

void CParseTranscriptionsDlg::OnSelSetDefaultCategoryList(LPDISPATCH lpSel) 
{
	try {
		IRowSettingsPtr pRow(lpSel);
		// (a.wilson 2012-08-02 15:17) - PLID 59132 - modify to support category for EMR.
		if (pRow) {
			m_nDefaultCategoryID = VarLong(pRow->GetValue(0));
			if (m_opt.bImportIntoEMR)
				SetRemotePropertyInt("ParseOptionsDefaultEMNCategoryID", m_nDefaultCategoryID, 0, "<None>");
			else
				SetRemotePropertyInt("ParseOptionsDefaultCategoryID", m_nDefaultCategoryID, 0, "<None>");
		}	
	} NxCatchAll("Error in OnSelSetDefaultCategoryList");
}

// (a.walling 2008-07-16 09:14) - PLID 30751 - Save this segment into history
// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
BOOL CParseTranscriptionsDlg::SaveSegmentToHistory(CTxDoc* pDoc, CTxSeg* pSeg, CProgressDialog &progressDialog, std::shared_ptr<CGenericWordProcessorApp> pApp)
{
	try {
		if (pSeg) {
			ASSERT(pSeg->nCategoryID != -3);
			ASSERT(pSeg->nPatientID >= 0);

			if (pSeg->nPatientID < 0) {
				ThrowNxException("Cannot save transcription with no patient!");
			} else if (pSeg->nCategoryID == -3) {
				ThrowNxException("Cannot save transcription with no category!");
			}


			CString strFirst, strLast, strFileName, strFilePath;
			CString strExtension = m_opt.bCreateWordDocs ? "doc" : "txt";
			IRowSettingsPtr pRow = m_dlPat->FindByColumn(ecPersonID, pSeg->nPatientID, NULL, VARIANT_FALSE);
			// (a.walling 2010-04-28 17:38) - PLID 38410 - Pass in a connection to GetPatientDocumentPath
			if (pRow) {
				strFirst = VarString(pRow->GetValue(ecFirst), "");
				strLast = VarString(pRow->GetValue(ecLast), "");
				strFileName = GetPatientDocumentName(GetRemoteData(), pSeg->nPatientID, strFirst, strLast, pSeg->nUserDefinedID, strExtension);
				strFilePath = GetPatientDocumentPath(GetRemoteData(), pSeg->nPatientID, strFirst, strLast, pSeg->nUserDefinedID) ^ strFileName;
			} else {
				strFileName = GetPatientDocumentName(GetRemoteData(), pSeg->nPatientID, strExtension);
				strFilePath = GetPatientDocumentPath(GetRemoteData(), pSeg->nPatientID) ^ strFileName;
			}		
			
			progressDialog.SetLine(2, "Saving to history: %s", strFilePath);

			CString strText = pDoc->GetText();
			
			CString strSection;
			if (pSeg->m_pstrOverrideText) {
				strSection = *(pSeg->m_pstrOverrideText);
			} else {
				strSection = strText.Mid(pSeg->nBeginCharIndex, pSeg->nEndCharIndex - pSeg->nBeginCharIndex);
			}
			// (a.wilson 2012-10-15 15:22) - PLID 53129 - check to see if they want reviews ignored.
			if ((!pSeg->bReviewed) && (!m_opt.bIgnoreSegmentReview)) {
				strSection += "\r\n\r\nDictated but not read.\r\n";
			}

			progressDialog.SetLine(1, "Creating document...");

			BOOL bSuccess = FALSE;

			// (a.walling 2008-07-17 16:58) - PLID 30774 - Create a blank word document if desired.
			if (m_opt.bCreateWordDocs) {
				try {
					if (!EnsureWordApp("Creating documents...")) {						
						m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");
						ThrowNxException("Microsoft Word is unavailable or not properly installed.");
						return FALSE;
					}
					// Eventually we can have this insert into a template
					// (c.haag 2016-01-21) - PLID 68173 - Do all the work from the CWordApp class
					// (c.haag 2016-05-10 13:27) - NX-100318 - We no longer maintain a persistent Word application in this object.
					// We don't need one either since all we do here is silently create a new document with text
					bSuccess = pApp->CreateAndPopulateNewFileWithPlainText(strFilePath, strSection);
				} NxCatchAllThrow("Could not create new word document");
			} else {
				CFile f;
				if (f.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
					f.Write(strSection, strSection.GetLength());
					f.Close();

					bSuccess = TRUE;
				} else {
					ThrowNxException("Could not create transcription output file: %s", FormatLastError());
				}
			}

			if (bSuccess) {
				long nCategoryID = pSeg->nCategoryID;
				if (nCategoryID == -2) {
					nCategoryID = m_nDefaultCategoryID;
				}

				//#pragma TODO("// (a.walling 2008-07-16 10:34) - Must I truncate off the time for the service date column?")
				// (a.walling 2008-07-16 17:38) - According to Josh:
					// "ahh, one of the unanswered questions of life, "Must I truncate off the time for the service date column?"
					// (and yes, ye must)"
					// Unfortunately, now I have a time that is being parsed along with the date, and nothing to use it with.

				progressDialog.SetLine(1, "Saving into history...");

				// (j.jones 2008-09-04 17:05) - PLID 30288 - converted to use CreateNewMailSentEntry,
				// which creates the data in one batch and sends a tablechecker
				// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
				// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
				COleDateTime dtNull;
				dtNull.SetStatus(COleDateTime::null);
				CreateNewMailSentEntry(pSeg->nPatientID, pSeg->strTitle.IsEmpty() ? DEFAULT_TX_TITLE : pSeg->strTitle, m_opt.bCreateWordDocs ? SELECTION_WORDDOC : SELECTION_FILE, strFileName, GetCurrentUserName(), "", GetCurrentLocationID(), pSeg->dtDate.GetStatus() == COleDateTime::valid ? pSeg->dtDate : dtNull, -1, nCategoryID, -1, -1, FALSE, -1, "", ctNone);

				return TRUE;
			}
		}
	} NxCatchAllThrow("Error importing transcription segment into history");

	return FALSE;
}

void CParseTranscriptionsDlg::PostNcDestroy() 
{
	CNxDialog::PostNcDestroy();

	GetMainFrame()->m_pParseTranscriptionsDlg = NULL;
	delete this;
}

void CParseTranscriptionsDlg::OnCancel() 
{
	try {
		long nExisting = 0;
		IRowSettingsPtr pRow = m_dl->GetFirstRow();
		while (pRow) {
			IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();

			while (pChildRow) {
				nExisting++;
				pChildRow = pChildRow->GetNextRow();
			}

			pRow = pRow->GetNextRow();
		}

		if (nExisting > 0) {
			if (IDYES != MessageBox(FormatString("There are %li segments in the list. If you continue to close, those segments will be cleared and documents must be re-parsed. Do you want to continue closing?", nExisting), NULL, MB_YESNO | MB_ICONHAND)) {
				return;
			}
		}	
		
		::DestroyWindow(GetSafeHwnd());
	} NxCatchAll("CParseTranscriptionsDlg::OnCancel");
}

void CParseTranscriptionsDlg::OnOK() 
{
	//::DestroyWindow(GetSafeHwnd());
}

void CParseTranscriptionsDlg::OnEditingFinishingTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{

}

// (a.walling 2008-07-17 16:44) - PLID 30774
BOOL CParseTranscriptionsDlg::EnsureWordApp(const CString &strAfter)
{
	try 
	{
		// (c.haag 2016-05-10 13:27) - NX-100318 - We no longer need a persistent Word object, but this
		// function should still make sure that a Word Processor is installed

		// (a.walling 2008-07-18 13:25) - PLID 30774 - Get the object from this method too
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			
			if (!m_bCheckedForWord) {
				m_bCheckedForWord = TRUE;
				
				MessageBox("Microsoft Office Word is required to parse text from a Word document or create a Word document.", NULL, MB_ICONHAND);
			}

			m_lblDrop.SetWindowText(strAfter);

			return FALSE;
		} else {
			m_bCheckedForWord = TRUE;
		}

		m_lblDrop.SetWindowText(strAfter);

		return TRUE;
	} NxCatchAll("Error ensuring Word availability");

	return FALSE;
}

void CParseTranscriptionsDlg::DisplaySegment(CTxSeg* pSeg, CTxDoc* pDoc, long nScrollToLine)
{
	CWaitCursor cws;
	CString strText = pDoc->GetText();

	if ( (pSeg->m_pstrOverrideText == NULL && m_bViewWholeDocument) || (m_bEditSelectionMode) ) {
		// display the whole text and select the current section
		m_text.SetWindowText(strText);

		m_text.SetSel(pSeg->nBeginCharIndex, pSeg->nEndCharIndex, TRUE);
		m_text.LineScroll(LONG_MIN);
		m_text.LineScroll(m_text.LineFromChar(pSeg->nBeginCharIndex));
	} else {
		// just display this segment
		
		CString strSection;
		if (pSeg->m_pstrOverrideText) {
			strSection = "(Note: This is a manually edited segment. This line will not appear when imported into Practice.)\r\n\r\n";
			strSection += *(pSeg->m_pstrOverrideText);
		} else {
			strSection = strText.Mid(pSeg->nBeginCharIndex, pSeg->nEndCharIndex - pSeg->nBeginCharIndex);
		}
		strSection += "\r\n";
		// (a.wilson 2012-10-15 15:23) - PLID 53129 - check to see if they want reviews ignored.
		if ((!pSeg->bReviewed) && (!m_opt.bIgnoreSegmentReview)) {
			strSection += "\r\n\r\nDictated but not read.\r\n";
		}

		m_text.SetWindowText(strSection);

		if (nScrollToLine != -1) {
			m_text.LineScroll(LONG_MIN);
			m_text.LineScroll(nScrollToLine);
		}
	}
	
	m_nxbReviewed.EnableWindow(TRUE);
	m_nxibSetToSelection.EnableWindow(TRUE);
	if (pSeg->m_pstrOverrideText) {
		m_nxibEditText.SetTextColor(RGB(255, 0, 0));
	} else {
		m_nxibEditText.AutoSet(NXB_MODIFY, NXIB_TEXTCOLOR);
	}
	if (m_bEditSelectionMode) {
		m_nxibEditText.EnableWindow(FALSE);
	} else {
		m_nxibEditText.EnableWindow(TRUE);
	}
	m_nxibEditText.Invalidate();
	m_nxbReviewed.SetCheck(pSeg->bReviewed ? BST_CHECKED : BST_UNCHECKED);

	if (pSeg->nUserDefinedID != LONG_MIN) {
		m_dlPat->SetSelByColumn(ecUserDefinedID, pSeg->nUserDefinedID);
	} else {
		m_dlPat->PutCurSel(NULL);
	}
	
	m_dlPat->PutEnabled(VARIANT_TRUE);
	m_dlCat->PutEnabled(VARIANT_TRUE);
	m_dlProvider->PutEnabled(VARIANT_TRUE);
	m_nxTime->PutEnabled(VARIANT_TRUE);
	if (pSeg->dtDate.GetStatus() == COleDateTime::valid) {
		m_nxTime->SetDateTime(pSeg->dtDate);
	} else {
		m_nxTime->Clear();
	}
			
	m_editTitle.EnableWindow(TRUE);
	m_bSettingEditText = TRUE;
	m_editTitle.SetWindowText(pSeg->strTitle.IsEmpty() ? DEFAULT_TX_TITLE : pSeg->strTitle);
	m_bSettingEditText = FALSE;
	//CATEGORY
	IRowSettingsPtr pCatRow = NULL;
	if (!pSeg->strCat.IsEmpty()) {
		if (pSeg->strCat.CompareNoCase("miscellaneous") == 0) {
			pCatRow = m_dlCat->FindByColumn(0, _variant_t((long)-1), NULL, VARIANT_TRUE);
		} else {
			pCatRow = m_dlCat->FindByColumn(1, _variant_t(pSeg->strCat), NULL, VARIANT_TRUE);
		}
		if (pCatRow == NULL) {
			// not found
			m_dlCat->PutComboBoxText(_bstr_t(CString(NOT_FOUND + pSeg->strCat)));
		}
	} else {
		if (pSeg->nCategoryID == -2) {
			m_dlCat->SetSelByColumn(0, _variant_t((long)-2));
		} else {
			m_dlCat->SetSelByColumn(0, _variant_t((long)-1));
		}
	}
	//PROVIDER
	// (a.wilson 2012-08-02 09:12) - PLID 51901 - display the proper row for provider.
	IRowSettingsPtr pProvRow = NULL;
	if (pSeg->nProviderID == -2) {
		m_dlProvider->PutComboBoxText(_bstr_t(CString(DUPLICATE_MATCHES + pSeg->strProvider)));
	} else if (pSeg->nProviderID == -1) {
		if (pSeg->strProvider.IsEmpty())
			m_dlProvider->FindByColumn(epID, (long)-1, NULL, VARIANT_TRUE);
		else
			m_dlProvider->PutComboBoxText(_bstr_t(CString(NOT_FOUND + pSeg->strProvider)));
	} else {
		pProvRow = m_dlProvider->FindByColumn(epID, (long)pSeg->nProviderID, NULL, VARIANT_TRUE);
		// (j.jones 2013-02-28 15:04) - PLID 55123 - it may be an inactive provider, and perhaps
		// we are no longer showing inactive providers
		_RecordsetPtr rsProvider = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle + ' ' + Title AS Name FROM PersonT "
			"WHERE ID = {INT}", pSeg->nProviderID);
		if(!rsProvider->eof) {
			m_dlProvider->PutComboBoxText(_bstr_t(AdoFldString(rsProvider, "Name", "")));
		}
		else {
			//not found
			m_dlProvider->PutComboBoxText(_bstr_t(CString(NOT_FOUND + pSeg->strProvider)));
		}
		rsProvider->Close();
	}
}

void CParseTranscriptionsDlg::OnCheckViewWholeDocument() 
{
	try {
		m_bViewWholeDocument = m_nxbViewWholeDoc.GetCheck() == BST_CHECKED;
		
		SetRemotePropertyInt("ParseOptionsViewWholeDoc", m_bViewWholeDocument, 0, "<None>");

		OnCurSelWasSetTree();
	} NxCatchAll("CParseTranscriptionsDlg::OnCheckViewWholeDocument");
}

void CParseTranscriptionsDlg::OnBtnEditText() 
{
	try {
		IRowSettingsPtr pRow = m_dl->GetCurSel();

		if (pRow) {		
			IRowSettingsPtr pParentRow = pRow->GetParentRow();

			if (pParentRow) {
				CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);
				CTxSeg* pSeg = NULL;
				if (pDoc && pDoc->psaText) {
					long nSegID = VarLong(pRow->GetValue(ecID));
					pSeg = pDoc->arSegs[nSegID];
				}
				ASSERT(pDoc);
				ASSERT(pSeg);

				if (pSeg) {
					CString strText = pDoc->GetText();
					
					CString strSection;
					if (pSeg->m_pstrOverrideText) {
						strSection = *(pSeg->m_pstrOverrideText);
					} else {
						strSection = strText.Mid(pSeg->nBeginCharIndex, pSeg->nEndCharIndex - pSeg->nBeginCharIndex);
					}

					CEditTextDlg dlg(this, strSection, "Edit Segment Text", GetNxColor(GNC_PATIENT_STATUS, 1));

					if (IDOK == dlg.DoModal()) {
						if (strSection != dlg.GetText()) {
							if (!pSeg->m_pstrOverrideText) {
								pSeg->m_pstrOverrideText = new CString;
							}

							*(pSeg->m_pstrOverrideText) = dlg.GetText();

							DisplaySegment(pSeg, pDoc);
						}
					}
				}				
			}
		}
		
	} NxCatchAll("Error in OnBtnEditText");
}

void CParseTranscriptionsDlg::OnSelChangingPatlist(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (NULL == *lppNewSel) {
		// Undo NULL selections (unless lpOldSel is NULL too, in that case we don't have any choice)
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	} 	
}

void CParseTranscriptionsDlg::OnSelChangingDefaultCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (NULL == *lppNewSel) {
		// Undo NULL selections (unless lpOldSel is NULL too, in that case we don't have any choice)
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	} 	
}

void CParseTranscriptionsDlg::OnSelChangingCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (NULL == *lppNewSel) {
		// Undo NULL selections (unless lpOldSel is NULL too, in that case we don't have any choice)
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	} 	
}

void CParseTranscriptionsDlg::OnKillFocusDate() 
{
	if (m_nxTime->GetStatus() != 1 || (COleDateTime(m_nxTime->GetDateTime()).GetStatus() != COleDateTime::valid) || (m_nxTime->GetDateTime() == 0) ) {
		m_nxTime->Clear();
		OnChangedParsetxDate();
	} else {
		COleDateTime dtMin, dtMax;
		dtMin.SetDate(1753,1,1);

		COleDateTime dt = m_nxTime->GetDateTime();

		if (dt <= dtMin) {
			m_nxTime->Clear();
			OnChangedParsetxDate();
		}
	}
}

// (a.wilson 2012-07-25 17:17) - PLID 33429 - a function to import parsed patient info into a new patient emn and insert into textbox detail.
// (j.jones 2013-03-01 11:15) - PLID 55122 - added default EMN status
// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
BOOL CParseTranscriptionsDlg::SaveToPatientEMN(long nEMNStatus, CTxDoc* pDoc, CTxSeg* pSeg, CProgressDialog &progressDialog)
{
	try {
		long nTemplateID = m_opt.nTemplateID;	//this is the EMN template we generate
		long nPatientUID = pSeg->nUserDefinedID;//this is the patient we generate the EMN for
		long nProviderID = pSeg->nProviderID;	//this is the provider we attach to the EMN
		long nCategoryID = pSeg->nCategoryID;	//this is the category of the EMN
		COleDateTime dtDate = pSeg->dtDate;			//this is the date of the EMN
		CString strDescription = pSeg->strTitle;	//this is the EMN description
		CString strSection;							//this will be the text for our certain EMN
		
		//STEP 1:  Ensure we have all necessary data

		// (j.jones 2013-04-08 17:33) - PLID 56149 - we never had a worthwhile progress note here, now we do
		progressDialog.SetLine(1, "Ensuring Data...");
		progressDialog.SetLine(2, "Creating EMN for %s", GetExistingPatientName(pSeg->nPatientID));

		//ensure Patient and Template are valid (Required Fields)
		if (nPatientUID <= 0 || nTemplateID <= 0)
			return FALSE;

		//collect the necessary text for the transcription.
		if (pSeg->m_pstrOverrideText) {
			strSection = *(pSeg->m_pstrOverrideText);
		} else {
			strSection = pDoc->GetText().Mid(pSeg->nBeginCharIndex, pSeg->nEndCharIndex - pSeg->nBeginCharIndex);
		}
		// (a.wilson 2012-10-15 16:17) - PLID 53129 - check whether segment review is being ignored.
		if ((!pSeg->bReviewed) && (!m_opt.bIgnoreSegmentReview)) {
			strSection += "\r\n\r\nDictated but not read.\r\n";
		}

		//STEP 2:  Create API Object to Import Transcription
		
		//get api pointer
		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();

		if (!pApi) {
			ThrowNxException("CParseTranscriptionsDlg::SaveToPatientEMN There is an issue with communication between the API.");
		}

		NexTech_Accessor::_ImportEMRTranscriptionCommitPtr pImport(__uuidof(NexTech_Accessor::ImportEMRTranscriptionCommit));
		{
			//templateID
			pImport->PutTemplateID(_bstr_t(nTemplateID));

			//patient ID
			pImport->PutpatientID(_bstr_t(nPatientUID));

			//description
			if (!strDescription.IsEmpty()) {
				pImport->PutEMNDescription(_bstr_t(strDescription));
			} else {
				pImport->PutEMNDescription(_bstr_t(DEFAULT_TX_TITLE));
			}

			//category ID
			if (nCategoryID > 0 || (nCategoryID == -2 && m_nDefaultCategoryID > 0)) {
				//if set to default then save default to EMN
				if (nCategoryID == -2) {
					nCategoryID = m_nDefaultCategoryID;
				}
				pImport->PutcategoryID(_bstr_t(nCategoryID));
			}

			// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
			pImport->PutemnStatusID(_bstr_t(nEMNStatus));

			//provider ID
			if (nProviderID > 0) {
				//check whether the provider is licensed for emr or not.
				if (g_pLicense && g_pLicense->IsProviderLicensed(nProviderID)) {
					pImport->PutPrimaryProviderIDs(Nx::SafeArray<BSTR>::FromValue(_bstr_t(nProviderID)));
				} else {
					pImport->PutSecondaryProviderIDs(Nx::SafeArray<BSTR>::FromValue(_bstr_t(nProviderID)));
				}
			}

			//date
			NexTech_Accessor::_NullableDateTimePtr pEMNDate(__uuidof(NexTech_Accessor::NullableDateTime));
			if (dtDate.GetStatus() == COleDateTime::valid) {
				pEMNDate->SetDateTime(dtDate);
			} else {
				pEMNDate->SetDateTime(COleDateTime::GetCurrentTime());
			}
			pImport->PutRefEMNDate(pEMNDate);

			//detail text
			pImport->PutDetailText(_bstr_t(strSection));

			progressDialog.SetLine(1, "Importing into EMR...");

			//send to api
			pApi->ImportEMRTranscription(_bstr_t(GetAPISubkey()), _bstr_t(GetAPILoginToken()), pImport);

			return TRUE;
		}
		
	} NxCatchAllThrow("CParseTranscriptionsDlg::SaveToPatientEMN Error importing transcription segment into new patient EMN.");

	return FALSE;
}

// (a.wilson 2012-08-01 16:25) - PLID 51901 - a function to determine the best possible choice for provider
//not found = -1, duplicate matches = -2
long CParseTranscriptionsDlg::FindBestMatchingProviderID(const CString & strProvider)
{
	try {
		//seperate all the sections of the provider's name.
		CArray<CiString, CiString> tokens;
		CString strToken;
		int tokenPos = 0;
		static const char tokenSeparators[] = (" \t,.");

		strToken = strProvider.Tokenize(tokenSeparators, tokenPos);

		while (!strToken.IsEmpty()) {
			tokens.Add(strToken);
			strToken = strProvider.Tokenize(tokenSeparators, tokenPos);
		}
		
		//get the best count match and add the providers to a map for comparison.
		long nBestCount = 0;
		CMap<long, long, long, long> mapProviderIDToMatches;

		for (IRowSettingsPtr pRow = m_dlProvider->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long nCurrentCount = 0;
			for (int i = 0; i < tokens.GetCount(); i++)
			{
				//iterate when there is a match.
				CiString str = tokens.GetAt(i);
				if (str == CiString(pRow->GetValue(epFirst)))
					nCurrentCount++;
				else if (str == CiString(pRow->GetValue(epLast)))
					nCurrentCount++;
				else if (str == CiString(pRow->GetValue(epMiddle)))
					nCurrentCount++;
			}
			mapProviderIDToMatches.SetAt(VarLong(pRow->GetValue(epID)), nCurrentCount);

			if (nCurrentCount > nBestCount)
				nBestCount = nCurrentCount;
		}

		//check to ensure there were no duplicate provider matches.
		long nDuplicates = 0, nBestMatchID = -1;
		POSITION pos = mapProviderIDToMatches.GetStartPosition();

		while(pos) {
			long nProviderID;
			long nCount;
			mapProviderIDToMatches.GetNextAssoc(pos, nProviderID, nCount);
			//check to see if the current provider count is equal to the best count from the last loop.
			if (nCount == nBestCount && nCount != 0) {
				nBestMatchID = nProviderID;
				nDuplicates++;
			}
			//if duplicates, warn the user.
			if (nDuplicates > 1)
				return -2;
		}

		return nBestMatchID;
	} NxCatchAll(__FUNCTION__);

	return -1;	//no valid selection could be found or an error occured.
}
// (a.wilson 2012-08-02 11:12) - PLID 51901 - ensure valid selection for provider
void CParseTranscriptionsDlg::SelChangingProlist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (NULL == *lppNewSel) {
			// Undo NULL selections (unless lpOldSel is NULL too, in that case we don't have any choice)
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} 
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2012-08-02 14:05) - PLID 51901 - modify the provider if a different one is selected in the drop down.
void CParseTranscriptionsDlg::SelSetProlist(LPDISPATCH lpSel)
{
	try {
		IRowSettingsPtr pSelectedRow(lpSel);

		if (pSelectedRow) {
			IRowSettingsPtr pRow = m_dl->GetCurSel();

			if (pRow) {
				IRowSettingsPtr pParentRow = pRow->GetParentRow();

				if (pParentRow) {			
					CTxDoc* pDoc = (CTxDoc*)VarLong(pParentRow->GetValue(ecObject), NULL);

					if (pDoc) {
						long nIx = VarLong(pRow->GetValue(ecID), -1);
						if (nIx >= 0) {
							CTxSeg* pSeg = pDoc->arSegs[nIx];

							if (pSeg) {
								CString strProvider;
								long nNewProvID = VarLong(pSelectedRow->GetValue(epID), -1);

								if (nNewProvID == -1) {
									pSeg->strProvider.Empty();
									pSeg->nProviderID = -1;
									strProvider = "<No Provider>";
								} else {
									strProvider = pSeg->strProvider = VarString(pSelectedRow->GetValue(epName), "");
									pSeg->nProviderID = nNewProvID;
								}
								pRow->PutValue(ecProvider, _bstr_t(strProvider));
								
								// (j.jones 2013-02-27 16:35) - PLID 55119 - clear the error on the provider column
								UpdateRowErrorStatus(pRow, ecProvider, false);

								m_dlProvider->PutComboBoxText(_bstr_t(strProvider));
							}
						}
					}
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2012-08-02 15:00) - PLID 51932 - if the import destination was changed then change the category combos.
void CParseTranscriptionsDlg::EnsureCategories()
{
	//generate the categories for the neccessary config.
	if (m_opt.bImportIntoEMR) {
		m_dlCat->PutFromClause("(SELECT ID, Description FROM EMNTabCategoriesT "
			"UNION SELECT -1 AS ID, '<No Category>' AS Description UNION SELECT -2 AS ID, '<Default Category>' AS Description) SubQ");
		m_dlDefCat->PutFromClause("(SELECT ID, Description FROM EMNTabCategoriesT "
			"UNION SELECT -1 AS ID, '<No Category>' AS Description) SubQ");
	} else {
		m_dlCat->FromClause = _bstr_t("(SELECT ID, Description FROM NoteCatsF WHERE IsPatientTab = 1 AND " + GetAllowedCategoryClause("ID") + 
			" UNION SELECT -1 AS ID, ' <Miscellaneous>' AS Description UNION SELECT -2 AS ID, ' <Default Category>' AS Description) SubQ");
		m_dlDefCat->FromClause = _bstr_t("(SELECT ID, Description FROM NoteCatsF WHERE IsPatientTab = 1 AND " + GetAllowedCategoryClause("ID") + 
			" UNION SELECT -1 AS ID, ' <Miscellaneous>' AS Description) SubQ");
	}
	m_dlCat->Requery();
	m_dlDefCat->Requery();
	m_dlDefCat->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	//get the default category based on the import destination.
	if (m_opt.bImportIntoEMR) {
		m_nDefaultCategoryID = GetRemotePropertyInt("ParseOptionsDefaultEMNCategoryID", -1, 0, "<None>", true);
		m_dlCat->FindByColumn(0, _variant_t((long)-1), NULL, VARIANT_TRUE);
	} else {
		m_nDefaultCategoryID = GetRemotePropertyInt("ParseOptionsDefaultCategoryID", -1, 0, "<None>", true);
		m_dlCat->FindByColumn(0, _variant_t((long)-2), NULL, VARIANT_TRUE);
	}	
	m_dlDefCat->SetSelByColumn(0, _variant_t(m_nDefaultCategoryID));
}
// (a.wilson 2012-08-02 17:15) - PLID 51932 - needed a silent flag to skip the message of reparsing already parsed files.
void CParseTranscriptionsDlg::ParseAll(bool bSilent /* = false */)
{
	long nFailed = 0;

	try {
		// (c.haag 2016-06-01 11:48) - NX-100320 - Fail if we have any Word documents in the queue and could not get a Word 
		// instance. Failure will make a dialog appear regardless of the silent flag
		std::shared_ptr<CGenericWordProcessorApp> pApp = nullptr;
		for (IRowSettingsPtr p = m_dl->GetFirstRow(); nullptr != p; p = p->GetNextRow())
		{
			CTxDoc* pDoc = (CTxDoc*)VarLong(p->GetValue(ecObject), NULL);
			CString strExt = FileUtils::GetFileExtension(pDoc->strPath);
			if (strExt.CompareNoCase("doc") == 0 || strExt.CompareNoCase("docx") == 0)
			{
				pApp = GetWPManager()->GetAppInstance();
				if (nullptr == pApp)
				{
					return;
				}
			}
		}
		
		if (!bSilent) {
			long nExisting = 0;
			IRowSettingsPtr pRow = m_dl->GetFirstRow();
			while (pRow) {
				IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();

				while (pChildRow) {
					nExisting++;
					pChildRow = pChildRow->GetNextRow();
				}
				pRow = pRow->GetNextRow();
			}

			if (nExisting > 0) {
				if (IDYES != MessageBox(FormatString("There are already %li existing segments. If you continue parsing, those segments (and all changes to them) will be cleared and all documents will be re-parsed. Do you want to continue?", nExisting), NULL, MB_YESNO | MB_ICONHAND)) {
					return;
				}
			}
		}
		if (m_opt.strPatID.IsEmpty()) {
			MessageBox("The parsing configuration for transcripts is invalid. Please ensure the configuration is correct.", NULL, MB_ICONINFORMATION);
			return;
		}
		
		EnableWindow(FALSE);

		// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
		CProgressDialog progressDialog;
		//remarkably, the hwnd has to be NULL for this to work
		progressDialog.Start(NULL, CProgressDialog::AutoTime | CProgressDialog::NoMinimize, 
			"Parsing transcription files...", "Attempting to cancel...");
		progressDialog.SetLine(1, "Parsing files...");

		IRowSettingsPtr pRow = m_dl->GetFirstRow();
		IRowSettingsPtr pTopRow = pRow;

		long nTopLevelRowCount = 0;
		while (pTopRow) {
			nTopLevelRowCount++;
			pTopRow = pTopRow->GetNextRow();
		}

		long nTotal = (nTopLevelRowCount * m_cnProgressResolution) + 1;
		long nCurrent = 1;
		long nTotalLines = 0;
		long nTotalSuccessfulDocuments = 0;

		CString strErrors;

		progressDialog.SetProgress(0, nTotal);

		while (pRow && !progressDialog.HasUserCancelled()) {

			progressDialog.SetProgress(nCurrent, nTotal);

			CTxDoc* pDoc = (CTxDoc*)VarLong(pRow->GetValue(ecObject), NULL);
			ASSERT(pDoc);
			if (pDoc) {
				progressDialog.SetLine(2, pDoc->strPath);

				BOOL bWasCanceled = FALSE;
				if (pDoc->psaText == NULL) {
					pDoc->psaText = new CStringArray;
					if (!GetDocumentText(pDoc->strPath, *(pDoc->psaText), strErrors, progressDialog, nCurrent, nTotal, pApp, bWasCanceled)) {
						nFailed++;
						nCurrent += m_cnProgressResolution / 2;
						pDoc->Clear();
						if(bWasCanceled || progressDialog.HasUserCancelled()) {
							break;
						}
					} else {
						if (bWasCanceled) {
							break;
						}
						nCurrent += m_cnProgressResolution / 2;
						if (!ParseText(*pDoc, m_opt, progressDialog, nCurrent, nTotal, pApp)) {
							pDoc->ClearSegments();
							if(progressDialog.HasUserCancelled()) {
								break;
							}
						} else {
							nTotalSuccessfulDocuments++;
						}
					}
					nCurrent += m_cnProgressResolution / 2;
				} else {
					CTxDoc* pNewDoc = new CTxDoc;
					pNewDoc->nID = pDoc->nID;
					pNewDoc->strPath = pDoc->strPath;
					delete pDoc;
					pDoc = pNewDoc;
					pRow->PutValue(ecObject, _variant_t((long)pNewDoc));
					pNewDoc->psaText = new CStringArray;
					if (!GetDocumentText(pNewDoc->strPath, *(pNewDoc->psaText), strErrors, progressDialog, nCurrent, nTotal, pApp, bWasCanceled)) {
						nFailed++;
						nCurrent += m_cnProgressResolution / 2;
						pNewDoc->Clear();
						if(bWasCanceled || progressDialog.HasUserCancelled()) {
							break;
						}
					} else {
						if (bWasCanceled) {
							break;
						}
						nCurrent += m_cnProgressResolution / 2;
						if (!ParseText(*pNewDoc, m_opt, progressDialog, nCurrent, nTotal, pApp)) {
							pNewDoc->ClearSegments();
							if(progressDialog.HasUserCancelled()) {
								break;
							}
						} else {
							nTotalSuccessfulDocuments++;
						}
					}
					nCurrent += m_cnProgressResolution / 2;
				}
			}

			if (pDoc != NULL && pDoc->psaText != NULL) {
				nTotalLines += pDoc->psaText->GetSize();
			}

			pRow = pRow->GetNextRow();
		}

		progressDialog.Stop();
		
		EnableWindow(TRUE);

		if (!strErrors.IsEmpty()) {
			CString strFullError = FormatString("%li files failed to open. These will not be parsed:\r\n\r\n", nFailed);

			if (nFailed < 12) {
				MessageBox(strFullError + strErrors, NULL, MB_OK | MB_ICONEXCLAMATION);
			} else {
				strFullError = FormatString("%li files failed to open. Only some of these errors will be displayed. Please check the log for a full listing, try again, or contact NexTech Technical support if you need assistance. These will not be parsed:\r\n\r\n", nFailed);
				strFullError += strErrors;
				LogDetail("%s", strFullError);
				strFullError = strFullError.Left(2000);
				strFullError += "...";

				MessageBox(strFullError, NULL, MB_OK | MB_ICONEXCLAMATION);
			}
		}
		
		long nTotalSegments = RefreshSegments();

		// (a.walling 2008-08-18 13:26) - PLID 28838 - Only enable if some segments exist
		m_nxibImport.EnableWindow(nTotalSegments > 0 ? TRUE : FALSE);

		m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");

		// (a.walling 2008-08-08 13:16) - PLID 28838 - This calls CurSelWasSet with whatever the current selection is.
		// I think this seems cleaner than calling the message handler directly.
		m_dl->PutCurSel(m_dl->GetCurSel());

		if (nTotalLines == 0) {
			MessageBox("No lines of text were parsed!", NULL, MB_ICONEXCLAMATION);
		} else {
			DontShowMeAgain(this, FormatString("Successfully parsed %li lines in %li document(s).", nTotalLines, nTotalSuccessfulDocuments), "ParseTranscriptions_ParseSummary", "Practice", FALSE);
		}
		
		return;

	} NxCatchAll("Error in OnBtnParseAll");

	EnableWindow(TRUE);
	m_lblDrop.SetWindowText("Check the box next to each segment you wish to import.");
}

// (a.wilson 2012-09-10 09:53) - PLID 51901 - hide or show provider content based on current settings.
void CParseTranscriptionsDlg::EnsureProviderEnabled()
{
	//if emr is the import destination, show the provider content.
	if (m_opt.bImportIntoEMR) {
		
		m_dlProvider->PutEnabled(VARIANT_TRUE);
		GetDlgItem(IDC_PARSETX_PROVIDER_LIST)->ShowWindow(TRUE);
		GetDlgItem(IDC_PROVIDER_LIST_LABEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROVIDER_LIST_LABEL)->ShowWindow(TRUE);

		IColumnSettingsPtr pCol = m_dl->GetColumn(ecProvider);
		pCol->PutColumnStyle(csWidthPercent | csVisible);
		pCol->PutStoredWidth(25);

	} else {	//if history is the import destination, hide the provider content.
		m_dlProvider->PutEnabled(VARIANT_FALSE);
		GetDlgItem(IDC_PARSETX_PROVIDER_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_PROVIDER_LIST_LABEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROVIDER_LIST_LABEL)->ShowWindow(FALSE);

		IColumnSettingsPtr pCol = m_dl->GetColumn(ecProvider);
		pCol->PutColumnStyle(csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
void CParseTranscriptionsDlg::EnsureEMNStatus()
{
	//always set the default
	if(m_EMNStatusCombo->SetSelByColumn(escID, m_opt.nEMNStatusID) == NULL) {
		//the status is invalid, force it to be Open
		m_EMNStatusCombo->SetSelByColumn(escID, (long)0);
	}

	//if emr is the import destination, show the EMN status combo
	if (m_opt.bImportIntoEMR) {		
		m_EMNStatusCombo->PutEnabled(VARIANT_TRUE);
		GetDlgItem(IDC_PARSETX_EMN_STATUS_LIST)->ShowWindow(TRUE);
		GetDlgItem(IDC_EMN_STATUS_LIST_LABEL)->ShowWindow(TRUE);
	}
	else {	//if history is the import destination, hide the EMN Status
		m_EMNStatusCombo->PutEnabled(VARIANT_FALSE);
		GetDlgItem(IDC_PARSETX_EMN_STATUS_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_EMN_STATUS_LIST_LABEL)->ShowWindow(FALSE);
	}
}

// (j.jones 2013-02-27 16:18) - PLID 55117 - added an error count label
void CParseTranscriptionsDlg::UpdateErrorCountLabel()
{
	try {

		//count the rows with errors
		IRowSettingsPtr pParentRow = m_dl->GetFirstRow();

		long nCountErrors = 0;
		while (pParentRow) {
			//only child rows have errors, so iterate through the children
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while (pChildRow) {
				if(RowHasError(pChildRow)) {
					nCountErrors++;
				}
				pChildRow = pChildRow->GetNextRow();
			}

			//get the next parent row
			pParentRow = pParentRow->GetNextRow();
		}

		CString strErrors = "";
		if(nCountErrors == 1) {
			strErrors = "1 record has errors.";
		}
		else if(nCountErrors > 1) {
			strErrors.Format("%li records have errors.", nCountErrors);
		}
		m_labelErrorCount.SetWindowText(strErrors);

		// (j.jones 2013-02-28 12:19) - PLID 55118 - if errors exist, show the
		// 'go to next error' button
		if(nCountErrors > 0) {
			m_btnGoToNextError.ShowWindow(SW_SHOW);
		}
		else {
			m_btnGoToNextError.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-02-27 16:35) - PLID 55119 - This function will add or remove an error color to a given column,
// and show the red X icon if the row has errors when we're done.
// Takes in a row, a column to track or clear an error for, and a boolean to determine if we're adding an error or removing it.
void CParseTranscriptionsDlg::UpdateRowErrorStatus(NXDATALIST2Lib::IRowSettingsPtr pRow, EParseListColumns eColumn, bool bHasError)
{
	try {

		if(pRow == NULL || eColumn == -1) {
			//this function should have never been called
			ASSERT(FALSE);
			return;
		}

		//find the corresponding error tracking column for the column in question
		EParseListColumns eErrorColumn = ecNameError;
		if(eColumn == ecName) {
			eErrorColumn = ecNameError;
		}
		else if(eColumn == ecCategory) {
			eErrorColumn = ecCategoryError;
		}
		else if(eColumn == ecProvider) {
			eErrorColumn = ecProviderError;
		}
		else {
			//we were given a column that has no corresponding error tracking column
			ThrowNxException("Column %li has no error tracking column.", (long)eColumn);
		}

		//first update the column to reflect the presense or absence of an error
		if(bHasError) {
			//track that there is an error
			pRow->PutValue(eErrorColumn, g_cvarTrue);

			//set the text colors - will be red both normally and when highlighted
			COLORREF clrRed = RGB(255,25,25);
			UpdateCellColor(pRow, eColumn, clrRed, clrRed);
		}
		else {
			//clear the error
			pRow->PutValue(eErrorColumn, g_cvarFalse);

			//clear the text colors
			UpdateCellColor(pRow, eColumn, dlColorNotSet, dlColorNotSet);
		}

		//now see if any errors remain
		if(RowHasError(pRow)) {
			//color the row background yellow and show the red X icon
			COLORREF clrYellow = RGB(255,255,0);
			pRow->PutBackColor(clrYellow);
			pRow->PutValue(ecErrorIcon, (long)m_hIconRedX);
		}
		else {
			//clear the background color, and remove the error icon
			pRow->PutBackColor(dlColorNotSet);
			_variant_t varOldIcon = pRow->GetValue(ecErrorIcon);
			pRow->PutValue(ecErrorIcon, g_cvarNull);

			if(varOldIcon.vt != VT_NULL) {
				// (j.jones 2013-02-28 11:59) - PLID 55117 - We just removed an error,
				// so update the error count on the bottom of the screen.
				// We don't do this when adding errors because the RefreshSegments()
				// function calls this function after all segments are added, but
				// removing errors are always done one at a time.
				UpdateErrorCountLabel();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-02-28 10:26) - PLID 55119 - updates a given column with colors
void CParseTranscriptionsDlg::UpdateCellColor(NXDATALIST2Lib::IRowSettingsPtr pRow, EParseListColumns eColumn, OLE_COLOR clrText, OLE_COLOR clrHighlightedText)
{
	//throw exceptions to the caller
	if(pRow == NULL || eColumn == -1) {
		//this function should have never been called
		ASSERT(FALSE);
		return;
	}

	pRow->PutCellForeColor(eColumn, clrText);
	pRow->PutCellForeColorSel(eColumn, clrHighlightedText);
	if(eColumn == ecName) {
		//if clearing the name color, also update the label color
		pRow->PutCellForeColor(ecLabel, clrText);
		pRow->PutCellForeColorSel(ecLabel, clrHighlightedText);
	}
}

// (j.jones 2013-02-28 11:15) - PLID 55119 - Returns true if the row has at least one unresolved error.
bool CParseTranscriptionsDlg::RowHasError(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//throw exceptions to the caller

	if(pRow != NULL &&
		(VarBool(pRow->GetValue(ecNameError), FALSE)
			|| VarBool(pRow->GetValue(ecCategoryError), FALSE)
			// (j.jones 2013-03-04 13:35) - PLID 55119 - ignore provider errors if not an EMR import
			|| (VarBool(pRow->GetValue(ecProviderError), FALSE) && m_opt.bImportIntoEMR))) {

		return true;
	}

	return false;
}

// (j.jones 2013-03-05 12:37) - PLID 55121 - takes in a parent-level row representing a file,
// returns true if RowHasError returns true on at least one child record
bool CParseTranscriptionsDlg::FileHasError(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	if(pParentRow) {
		IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
		while(pRow) {
			if(RowHasError(pRow)) {
				//at least one row has an error, return true
				return true;
			}
			pRow = pRow->GetNextRow();
		}
	}

	return false;
}

// (j.jones 2013-03-05 12:37) - PLID 55121 - Takes in a parent-level row representing a file,
// returns false if m_opt.bRequireErrorsFixed is true and FileHasError returns true.
// Always returns true if m_opt.bRequireErrorsFixed is false.
bool CParseTranscriptionsDlg::CanImportFile(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	if(m_opt.bRequireErrorsFixed) {
		//if bRequireErrorsFixed is enabled, then return true if FileHasError is false
		return !FileHasError(pParentRow);
	}
	else {
		//if bRequireErrorsFixed is not enabled, nothing restricts file-level imports
		return true;
	}
}

// (j.jones 2013-02-28 12:19) - PLID 55118 - added button to go to next error
void CParseTranscriptionsDlg::OnBtnGoToNextTranscriptionError()
{
	try {

		//try to start with the first selected row, if any
		IRowSettingsPtr pSelRow = m_dl->GetFirstSelRow();
		IRowSettingsPtr pParentRow = NULL;
		
		if(pSelRow) {
			pParentRow = pSelRow->GetParentRow();
			//if pParentRow is not null, pSelRow is a child, so iterate through the rest of the children
			if(pParentRow) {
				//start at the next child, iterate until the end
				IRowSettingsPtr pChildRow = pSelRow->GetNextRow();
				while(pChildRow) {
					if(RowHasError(pChildRow)) {
						//select this row, and return
						m_dl->PutCurSel(pChildRow);
						return;
					}
					pChildRow = pChildRow->GetNextRow();
				}

				//when done, set the parent to the next parent in the list
				pParentRow = pParentRow->GetNextRow();
			}
			else {
				//pSelRow is the parent row
				pParentRow = pSelRow;
			}

			//iterate through all remaining parents & children

			while (pParentRow) {
				IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
				while (pChildRow) {
					if(RowHasError(pChildRow)) {
						//select this row, and return
						m_dl->PutCurSel(pChildRow);
						return;
					}
					pChildRow = pChildRow->GetNextRow();
				}
				pParentRow = pParentRow->GetNextRow();
			}
		}

		//If we reach the end of the list and didn't find an error,
		//but started at a selection rather than at the top of the list,
		//restart from the top of the list.
		//If pSelRow is the only row with an error, we will simply
		//re-select that row once we reach it.
		pParentRow = m_dl->GetFirstRow();
		while (pParentRow) {
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while (pChildRow) {
				if(RowHasError(pChildRow)) {
					//select this row, and return
					m_dl->PutCurSel(pChildRow);
					return;
				}
				pChildRow = pChildRow->GetNextRow();
			}
			pParentRow = pParentRow->GetNextRow();
		}

		//if we didn't find an error, this function should never have
		//been called in the first place, it would mean we displayed
		//the button when no errors actually existed
		ASSERT(FALSE);
		//recalculate the errors such that this button is disabled
		UpdateErrorCountLabel();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-02-28 15:53) - PLID 55121 - returns true if Commit is true and if the row is permitted to be imported,
// it might not be if it still has errors
BOOL CParseTranscriptionsDlg::NeedCommitRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	_variant_t varCommit = pRow->GetValue(ecCommit);
	BOOL bCommit = VarBool(varCommit, FALSE);
	if(bCommit && m_opt.bRequireErrorsFixed && RowHasError(pRow)) {
		bCommit = FALSE;

		//set commit to false, if it's non-null
		if(varCommit.vt == VT_BOOL) {
			pRow->PutValue(ecCommit, g_cvarFalse);
		}
	}

	return bCommit;
}

// (j.jones 2013-02-28 15:53) - PLID 55121 - returns true if Commit is checkable (it might not actually be checked)
// and if the row is permitted to be imported, it might not be if it still has errors
BOOL CParseTranscriptionsDlg::CanCommitRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	BOOL bCanCommit = FALSE;
	_variant_t varCommit = pRow->GetValue(ecCommit);
	if (varCommit.vt == VT_BOOL) {
		//doesn't matter if varCommit is true or false,
		//if the checkbox is non-null, they *could* commit it
		bCanCommit = TRUE;
	
		//can't commit if there are still errors
		if(m_opt.bRequireErrorsFixed && RowHasError(pRow)) {
			bCanCommit = FALSE;

			//if commit is true, set it to false
			if(VarBool(varCommit)) {
				pRow->PutValue(ecCommit, g_cvarFalse);
			}
		}
	}

	return bCanCommit;
}

// (j.jones 2013-02-28 16:30) - PLID 55121 - added so we can prevent committing records with errors
void CParseTranscriptionsDlg::OnEditingStartingTree(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2013-02-28 16:30) - PLID 55121 - The commit box is invisible unless it's VT_BOOL,
		// and pvarValue is the current value. So if it is FALSE, it means they are about to check it.
		// Confirm that they are allowed to, and aren't restricted from importing a row with errors.
		if(nCol == ecCommit && pvarValue->vt == VT_BOOL && !VarBool(pvarValue) && !CanCommitRow(pRow)) {
			*pbContinue = FALSE;
			MessageBox("This record cannot be imported until its errors have been fixed.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-03-04 09:46) - PLID 55120 - Checks the setting to move the transcription file
// to a folder. Returns true if succeeded, in which case the caller needs to remove the parent row.
bool CParseTranscriptionsDlg::TryMoveImportedTranscriptionFile(CString strFilePath, bool bPrompt /*= false*/)
{
	try {

		if(strFilePath.IsEmpty()) {
			//we were given invalid content
			return false;
		}

		//is this setting turned on?
		if(!m_opt.bMoveImportedFiles) {
			return false;
		}

		//is the path filled in?
		if(m_opt.strPostImportFolder.IsEmpty()) {
			//this should be impossible because we disallow them
			//from saving this bad data
			ASSERT(FALSE);
			return false;
		}

		if(bPrompt) {
			CString strPrompt;
			strPrompt.Format("Would you like to move this file, %s, to the imported transcriptions folder?", strFilePath);
			if(IDNO == MessageBox(strPrompt, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return false;
			}
		}

		CString strFileName = GetFileName(strFilePath);
		CString strExtension = FileUtils::GetFileExtension(strFilePath);
		CString strFileNameNoExt = "";
		int nDot = strFileName.ReverseFind('.');
		if(nDot != -1) {
			strFileNameNoExt = strFileName.Left(nDot);
		}

		COleDateTime dtServer = GetRemoteServerTime();
		CString strNewFilePath;
		strNewFilePath.Format("%s\\%s_%s_%s%s%s", m_opt.strPostImportFolder, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), strExtension.IsEmpty() ? "" : ".", strExtension);

		//in the unlikely case that a file with this exact name exists
		int nCount = 0;
		while(DoesExist(strNewFilePath)) {

			//try adding an index to the end
			nCount++;

			if(nCount > 100) {
				//something is seriously wrong if the same file & timestamp exists this many times,
				//so abort before we get into an infinite loop
				ThrowNxException("Cannot move transcription file, too many files with the name like: %s", strNewFilePath);
			}

			strNewFilePath.Format("%s\\%s_%s_%s_%li%s%s", m_opt.strPostImportFolder, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount, strExtension.IsEmpty() ? "" : ".", strExtension);
		}

		//We won't call DoesExist on the source or destination folder.
		//It should be impossible to get here if the source path didn't exist,
		//and if we're importing (as opposed to manually removing) we wouldn't
		//have allowed the import to continue if the destination folder didn't exist.
		//Instead, just complain if the moving failed.

		if(!MoveFile(strFilePath, strNewFilePath)) {
			CString strError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();

			CString strWarning;
			strWarning.Format("The file %s could not be moved to the imported transcription folder %s.\n\n"
				"Error: %s", strFilePath, strNewFilePath, strError);
			MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			return false;
		}
		else {
			return true;
		}

	}NxCatchAll(__FUNCTION__);

	return false;
}