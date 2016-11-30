// HistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HistoryDlg.h"
#include "LetterWriting.h"
#include "SuperBill.h"
#include "MergeEngine.h"
#include "DontShowDlg.h"
#include "GlobalDataUtils.h"
#include "MsgBox.h"
#include "AuditTrail.h"
#include "CaseHistoryDlg.h"
#include "nxtwain.h"
#include "SelectPacketDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "nxsecurity.h"
#include "nxrapi.h"
#include "nxrapibrowsedlg.h"
#include "SelectSenderDlg.h"
#include "NoteCategories.h"
#include "MultiSelectDlg.h"
#include "InternationalUtils.h"
#include "dynuxtheme.h"
#import "NxShellExtSendTo.tlb"
#include "RegUtils.h"
#include "GlobalUtils.h"
#include "PicContainerDlg.h"
#include "RenameFileDlg.h"
#include "nxstandard.h"
#include "PatientView.h"
#include "HistoryTabsDropSource.h"
#include "EligibilityRequestDetailDlg.h"
#include "FaxSendDlg.h"
#include "FaxChooseRecipientDlg.h"
#include "ScanMultiDocDlg.h"
#include "FileUtils.h"
#include "NxWIA.h"
#include <mmsystem.h>
#include "SOAPUtils.h"
#include "GenericBrowserDlg.h"
#include "CCDUtils.h"
#include "CCDInterface.h" //(e.lally 2010-02-18) PLID 37438
#include "Selectdlg.h"
#include "historyutils.h"
#include "TaskEditDlg.h"
#include "NxXMLUtils.h"
#include "NxModalParentDlg.h"
#include "EMRPreviewPopupDlg.h"
#include "PatientDocumentStorageWarningDlg.h"
#include "DirectMessageSendDlg.h"// (j.camacho 2013-10-10 15:03) - PLID 58929
#include "FinancialRC.h"
#include "Nxapi.h"
// (s.dhole 2013-11-01 12:19) - PLID 59278
#include "ReconciliationDlg.h"
#include "NxAPIUtils.h"
#include "NotesDlg.h"
#include "SelectCCDAInfoDlg.h"  // (b.savon 2014-05-01 16:57) - PLID 61909
#include "CCDAUtils.h"
#include "HL7Utils.h"
#include "DocumentOpener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
// m_bInPICContainer replaced with Set / GetPicContainer so as to avoid GetParent stuff


#define IDT_DRAG_TIMER			1000

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_NEW_DOCUMENT		49989
#define IDM_ATTACH_DOCUMENT		49990
#define	IDM_MERGE_DOCUMENT		49991
#define	IDM_IUI					49994
#define	IDM_MEDEVAL				49995
#define IDM_ATTACH_FOLDER		49996
#define IDM_NEW_CASE_HISTORY	49997 // (b.cardillo 2002-04-25 19:23) - I have no idea why these aren't resources, but to be consistent I'm putting my define here
#define IDM_ACQUIRE				49998 // (c.haag 2002-08-19 18:07) - Ditto
#define IDM_MERGE_PACKET		49999 // (t.schneider 2002-08-23 11:18) - Ditto.  Well, I have an idea: Shoddy worksmanship!
#define IDM_IMPORT_FROM_PDA		50000
#define IDM_IMPORT_AND_ATTACH	50001 // (c.haag 2003-07-14 16:39) - Copies and attaches files
#define IDM_HX_PARSE_TRANSCRIPTIONS	50002 // (a.walling 2008-07-17 15:11) - PLID 30751 - Launches transcription parser

// (a.walling 2008-09-05 12:42) - PLID 31282 - For scanning into PDF files
#define IDM_ACQUIRE_PDF			50003
#define IDM_ACQUIRE_MULTI_PDF	50004
// (a.walling 2008-09-08 09:39) - PLID 31282 - Use the scan multi doc dialog
#define IDM_ACQUIRE_MULTIDOC	50005
// (a.walling 2008-09-09 13:30) - PLID 30389 - WIA acquire
#define IDM_ACQUIRE_WIA	50006
#define IDM_WIA_OPTIONS_AUTOACQUIRE	50007
// (r.galicki 2008-09-23 10:51) - PLID 31407 - Select Input Souce
#define IDM_SELECT_TWAIN_SOURCE 50008
// (z.manning 2009-01-29 10:45) - PLID 32885 - Added options for merging EMR standard templates and packets
#define IDM_MERGE_EMR_STD_DOCUMENT	50009
#define IDM_MERGE_EMR_STD_PACKET	50010

#define IDM_IMPORT_CCD			51011 // (a.walling 2009-05-05 16:50) - PLID 34176
#define IDM_VIEW_XML			51012 // (a.walling 2009-05-06 10:36) - PLID 34176
#define IDM_VALIDATE_CCD		51013 // (a.walling 2009-05-06 10:36) - PLID 34176
#define IDM_CREATE_CCD			51014 // (a.walling 2009-05-05 16:50) - PLID 34176


#define IDM_CREATE_CCDA			51015 // (j.gruber 2013-11-08 08:30) - PLID 59375
#define IDM_CREATE_CCDA_CUSTOM	51016 // (b.savon 2014-05-01 16:50) - PLID 61909

#define IDM_SET_PRIMARY_PICTURE			49000 // (c.haag 2002-10-30 09:10) - ....yup
#define	IDM_ATTACH_FILE					49001 //	DRT - Message for menu - attaching files
#define IDM_EMAIL_FILE					49003
#define IDM_RESET_PRIMARY_PICTURE		49002
#define IDM_PRINT_FILE					49004
#define IDM_SEND_TO_SERVER				49005 // (b.cardillo 2003-09-29 11:32) - Uses NxShellExtSendTo code to send files to the remote server.  (NOTE: For now this is only available if you have a NexTechInternal license because we don't release NxSourceLinkServer to our users.)
#define IDM_DETACH_DOCUMENT				49006
#define IDM_DETACH_AND_DELETE_DOCUMENT	49007
#define IDM_TOGGLE_PHOTO_STATUS			49008 // (a.wetta 2007-07-09 10:14) - PLID 17467 - Menu check to determine if a document appears in the photos tab
#define IDM_SEND_FAX					49009	//DRT 7/2/2008 - PLID 30601 - Online faxing.
#define IDM_CHANGE_PATIENT				49010  // (j.gruber 2010-01-05 11:28) - PLID 22958 - change patient on an image
#define IDM_ATTACH_TO_CURRENT_PIC		49011  // (z.manning 2010-02-02 11:55) - PLID 34287
#define IDM_NEW_TODO		49012 // (c.haag 2010-05-21 13:33) - PLID 38731
#define IDM_DEVICE_IMPORT				49013	// (d.lange 2010-06-21 16:37) - PLID 39202 - Launches the Device Import dialog
#define IDM_DIRECTMESSAGE_SEND			49014  // (j.camacho 2013-11-05 15:21) - PLID 59303

#define IDM_CCDA_RECONCILIATION_MEDICATION			49015 // (s.dhole 2013-11-01 11:56) - PLID 59278 Medical reconciliation
#define IDM_CCDA_RECONCILIATION_ALLERGY			49016 // (s.dhole 2013-11-01 11:56) - PLID 59278 allergy reconciliation
#define IDM_CCDA_RECONCILIATION_PROBLEM			49017 // (s.dhole 2013-11-01 11:56) - PLID 59278 Problems reconciliation


#define ID_MULTIPLE_CATS	-3
#define ID_ALL_CATS		-2
#define ID_NO_CAT		-1

// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
//#define TAB_HIGHLIGHT_COLOR	RGB(235,235,235)

using namespace ADODB;
using namespace NXDATALISTLib;
using namespace NXTWAINlib;

extern CPracticeApp theApp;

// Column indexes for the datalist referenced by m_pDocuments
enum EPatientsHistoryTabDocumentListColumns
{
	phtdlcMailID = 0,
	phtdlcIcon,				// (a.walling 2008-09-15 16:44) - PLID 23904 - this column is the actual icon
	phtdlcPath,
	phtdlcFilePath,
	phtdlcFilename,
	phtdlcNotes,
	phtdlcExtraNotesIcon,	// (c.haag 2010-07-01 15:59) - PLID 39473
	phtdlcStaff,
	phtdlcDate,
	phtdlcServiceDate,		//DRT 9/11/03
	phtdlcCategory,			//DRT 11/11/2003
	phtdlcSelection,		// (a.walling 2006-11-29 10:26) - PLID 23681 - Best way to determine if item is a message (statement run, hcfa sent, etc) In data as "BITMAP:FILE", 'Superbill', etc
	phtdlcIsPhoto,			// (a.wetta 2007-07-09 10:03) - PLID 17467 - Used to determine if a picture file will appear on the Photos 
	phtdlcEmnID,			// (z.manning 2008-07-01 12:38) - PLID 25574
	phtdlcName,				// (a.walling 2008-09-15 16:43) - PLID 23904 - the "Name" column has been moved
	phtdlcPicID,			// (z.manning 2010-02-02 11:34) - PLID 34287 - Added PicID column
	phtdlcHasExtraNotes, // (c.haag 2010-07-01 15:59) - PLID 39473
	phtdlcEmnModifiedDate,	// (z.manning 2012-09-10 14:28) - PLID 52543
};

BOOL FindFileRecursively(const CString &strPath, const CString &strFileName, CString &strFoundInPath);

// (a.walling 2008-09-15 17:29) - PLID 23904
HICON CHistoryDlg::m_hiconPacket = NULL; 


/////////////////////////////////////////////////////////////////////////////
// CHistoryDlg dialog

CHistoryDlg::CHistoryDlg(CWnd* pParent)
	: CPatientDialog(CHistoryDlg::IDD, pParent),
	m_NoteChecker(NetUtils::NoteCatsF),
	m_PatChecker(NetUtils::PatCombo)
	, m_pPicContainer(NULL)
{
	//{{AFX_DATA_INIT(CHistoryDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/History/view_patients_document_history.htm";
	m_nRowSel = -1;
	m_nCurrentFilterID = -1;
	m_nTopRow = -1;
	m_bInPatientsModule = true;
	m_bPhotoViewerDetachable = false;
	m_bThemeOpen = FALSE;
	m_bDragging = FALSE;
	m_bCaseHistoryIsOpen = FALSE;
	m_pdlgEmrPreview = NULL;
	m_hiconEmn = NULL;
	m_bWIAAutoAcquire = FALSE;
	m_bDateFromDown = FALSE;
	m_bDateToDown = FALSE;
	m_hExtraNotesIcon = NULL; // (c.haag 2010-08-26 16:06) - PLID 39473

	// (a.walling 2008-09-15 17:29) - PLID 23904 - Load the packet HICON
	if (!m_hiconPacket) {
		m_hiconPacket = theApp.LoadIcon(IDI_PACKET_16);
	}

	// (a.walling 2010-10-14 13:51) - PLID 40978
	m_id = -1;
}


CHistoryDlg::~CHistoryDlg()
{
	// (c.haag 2010-07-01 16:06) - PLID 39473 - Destroy the icons we made
	if (m_hExtraNotesIcon) DestroyIcon(m_hExtraNotesIcon);
}

void CHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHistoryDlg)
	DDX_Control(pDX, IDC_EDIT_HISTORY_CATS, m_btnEditCategories);
	DDX_Control(pDX, IDC_DETACH, m_btnDetach);
	DDX_Control(pDX, IDC_NEW, m_btnNew);
	DDX_Control(pDX, IDC_CHECK_FILTER_ON_EMR, m_checkFilterOnEMR);
	DDX_Control(pDX, IDC_REMEMBER_HISTORY_COLUMNS, m_chkRememberColumns);
	DDX_Control(pDX, IDC_ENABLE_UNATTACHED, m_checkShowUnattached);
	DDX_Control(pDX, IDC_USE_HISTORY_FILTER, m_checkFilterOnCategory);
	DDX_Control(pDX, IDC_PHOTOS_SORT_DESCENDING_CHECK, m_checkPhotosSortDescending);
	DDX_Control(pDX, IDC_HISTORY_BKG, m_bkg1);
	DDX_Control(pDX, IDC_HISTORY_BKG2, m_bkg2);
	DDX_Control(pDX, IDC_PHOTO_VIEWER, m_PhotoViewer);
	DDX_Control(pDX, IDC_PHOTOS_SORT_BY_LABEL, m_nxstaticPhotosSortByLabel);
	DDX_Control(pDX, IDC_SHOW_EMNS, m_btnShowEmns);
	DDX_Control(pDX, IDC_BTN_ACQUIRE, m_btnAcquire);
	DDX_Control(pDX, IDC_BTN_RECORD_AUDIO, m_btnRecordAudio);
	DDX_Control(pDX, IDC_RADIO_HISTORY_ALL_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_HISTORY_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_HISTORY_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_HISTORY_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_HISTORY_SHOW_GRIDLINES, m_btnShowGridlines);
	DDX_Control(pDX, IDC_PHOTOS_PRINT_NOTES_CHECK, m_checkPhotosPrintNotes);
	DDX_Control(pDX, IDC_BTN_DEVICEIMPORT, m_btnDeviceImport);

	//}}AFX_DATA_MAP
}

// (a.walling 2008-09-05 12:59) - PLID 31282 - Add handlers for acquiring PDF 
// (a.walling 2008-09-08 09:52) - PLID 31282 - Add handler for scanning multiple documents
// (a.walling 2008-09-10 15:06) - PLID 31334 - WIA event handling
BEGIN_MESSAGE_MAP(CHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHistoryDlg)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(IDM_NEW_DOCUMENT, OnClickNew)
	ON_COMMAND(IDM_MERGE_DOCUMENT, OnClickMerge)
	ON_COMMAND(IDM_ATTACH_DOCUMENT, OnClickAttach)
	ON_COMMAND(IDM_ATTACH_FOLDER, OnClickAttachFolder)
	ON_COMMAND(IDM_MERGE_PACKET, OnClickMergePacket)
	ON_COMMAND(IDM_IUI,OnClickIUI)
	ON_COMMAND(IDM_MEDEVAL,OnClickMedEval)
	ON_COMMAND(IDM_NEW_CASE_HISTORY, OnNewCaseHistory)
	ON_COMMAND(IDM_ACQUIRE, OnAcquire)
	ON_COMMAND(IDM_ACQUIRE_PDF, OnAcquirePDF)
	ON_COMMAND(IDM_ACQUIRE_MULTI_PDF, OnAcquireMultiPDF)
	ON_COMMAND(IDM_ACQUIRE_MULTIDOC, OnAcquireMultiDoc)
	ON_COMMAND(IDM_ACQUIRE_WIA, OnAcquireWIA)
	ON_COMMAND(IDM_WIA_OPTIONS_AUTOACQUIRE, OnWIAOptionsAutoAcquire)
	ON_COMMAND(IDM_SELECT_TWAIN_SOURCE, OnSelectSource)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DETACH, OnDetach)
	ON_COMMAND(IDM_DETACH_DOCUMENT, OnDetachDocument)
	ON_COMMAND(IDM_DETACH_AND_DELETE_DOCUMENT, OnDetachAndDelete)
	ON_BN_CLICKED(IDC_CURRENT_FOLDER, OnCurrentFolder)
	ON_BN_CLICKED(IDC_ENABLE_UNATTACHED, OnEnableUnattached)
	ON_COMMAND(IDM_SET_PRIMARY_PICTURE, OnSetPrimaryPicture)
	ON_COMMAND(IDM_RESET_PRIMARY_PICTURE, OnResetPrimaryPicture)
	ON_COMMAND(IDM_ATTACH_FILE, OnAttachFile)
	ON_COMMAND(IDM_EMAIL_FILE, OnEmailFile)
	ON_COMMAND(IDM_SEND_FAX, OnSendFax)
	ON_COMMAND(IDM_IMPORT_FROM_PDA, OnClickImportFromPDA)
	ON_COMMAND(IDM_IMPORT_AND_ATTACH, OnClickImportAndAttach)
	ON_COMMAND(IDM_PRINT_FILE, OnPrintFile)
	ON_COMMAND(IDM_HX_PARSE_TRANSCRIPTIONS, OnParseTranscriptions)
	ON_COMMAND(IDM_IMPORT_CCD, OnImportCCD)
	ON_COMMAND(IDM_VIEW_XML, OnViewXML)	
	ON_COMMAND(IDM_VALIDATE_CCD, OnValidateCCD)
	ON_COMMAND(IDM_CREATE_CCD, OnCreateCCD)
	ON_WM_DESTROY()
	ON_COMMAND(IDM_SEND_TO_SERVER, OnSendToServer)
	ON_BN_CLICKED(IDC_EDIT_HISTORY_CATS, OnEditHistoryCats)
	ON_BN_CLICKED(IDC_USE_HISTORY_FILTER, OnUseHistoryFilter)
	ON_BN_CLICKED(IDC_REMEMBER_HISTORY_COLUMNS, OnRememberColumns)
	ON_MESSAGE(NXM_HISTORY_TAB_CHANGED, OnHistoryTabChanged)
	ON_MESSAGE(NXM_DROP_FILES, OnDropFiles)
	ON_MESSAGE(NXM_PHOTO_VIEWER_DETACHABLE, OnPhotoViewerDetachable)
	ON_BN_CLICKED(IDC_PHOTOS_SORT_DESCENDING_CHECK, OnPhotosSortDescendingCheck)
	ON_BN_CLICKED(IDC_CHECK_FILTER_ON_EMR, OnCheckFilterOnEmr)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_COMMAND(IDM_TOGGLE_PHOTO_STATUS, OnTogglePhotoStatus)
	ON_COMMAND(IDC_SHOW_EMNS, OnShowEmns)
	ON_WM_SIZE()
	ON_MESSAGE(NXM_WIA_EVENT, OnWIAEvent)
	ON_BN_CLICKED(IDC_BTN_ACQUIRE, OnBtnAcquire)
	ON_COMMAND(IDC_BTN_RECORD_AUDIO, OnRecordAudio)
	ON_COMMAND(IDM_MERGE_EMR_STD_DOCUMENT, OnMergeEmrStdDocument)
	ON_COMMAND(IDM_MERGE_EMR_STD_PACKET, OnMergeEmrStdPacket)
	ON_BN_CLICKED(IDC_RADIO_HISTORY_ALL_DATES, OnBnClickedRadioHistoryAllDates)
	ON_BN_CLICKED(IDC_RADIO_HISTORY_DATE_RANGE, OnBnClickedRadioHistoryDateRange)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_HISTORY_FROM_DATE, OnChangeHistoryFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_HISTORY_TO_DATE, OnChangeHistoryToDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_HISTORY_FROM_DATE, OnDtnDropdownHistoryFromDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_HISTORY_TO_DATE, OnDtnDropdownHistoryToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_HISTORY_FROM_DATE, OnDtnCloseupHistoryFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_HISTORY_TO_DATE, OnDtnCloseupHistoryToDate)
	ON_COMMAND(IDM_CHANGE_PATIENT, OnChangePatient)
	ON_COMMAND(IDM_DIRECTMESSAGE_SEND, OnDirectMessage)
	ON_COMMAND(IDM_ATTACH_TO_CURRENT_PIC, OnAttachToCurrentPic)
	ON_COMMAND(IDM_NEW_TODO, OnNewTodo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HISTORY_SHOW_GRIDLINES, &CHistoryDlg::OnBnClickedHistoryShowGridlines)
	ON_BN_CLICKED(IDC_PHOTOS_PRINT_NOTES_CHECK, OnBnClickedPrintPhotoNotes)
	ON_MESSAGE(NXM_HISTORY_OPEN_DOCUMENT, OnOpenDocument)
	ON_COMMAND(IDM_DEVICE_IMPORT, OnDeviceImport)
	ON_BN_CLICKED(IDC_BTN_DEVICEIMPORT, OnDeviceImport)
	ON_MESSAGE(NXM_DEVICE_IMPORT_STATUS, OnUpdateDeviceImportButton)
	ON_COMMAND(IDM_CREATE_CCDA, OnCreateCCDA)
	ON_COMMAND(IDM_CREATE_CCDA_CUSTOM, OnCreateCCDACustomized)
	ON_COMMAND(IDM_CCDA_RECONCILIATION_MEDICATION, OnReconcileMedication)
	ON_COMMAND(IDM_CCDA_RECONCILIATION_ALLERGY, OnReconcileAllergy)
	ON_COMMAND(IDM_CCDA_RECONCILIATION_PROBLEM, OnReconcileProblem)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHistoryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHistoryDlg)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 10 /* EditingFinished */, OnEditingFinishedDocuments, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 9 /* EditingFinishing */, OnEditingFinishingDocuments, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 18 /* RequeryFinished */, OnRequeryFinishedDocuments, VTS_I2)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 6 /* RButtonDown */, OnRButtonDownDocuments, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 19 /* LeftClick */, OnLeftClickDocuments, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 2 /* SelChanged */, OnSelChangedDocuments, VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 20 /* TrySetSelFinished */, OnTrySetSelFinishedDocuments, VTS_I4 VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_HISTORY_FILTER_LIST, 16 /* SelChosen */, OnSelChosenHistoryFilterList, VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_HISTORY_FILTER_LIST, 1 /* SelChanging */, OnSelChangingHistoryFilterList, VTS_PI4)
	ON_EVENT(CHistoryDlg, IDC_CATEGORY_TAB, 1 /* SelectTab */, OnSelectTabDocTabs, VTS_I2 VTS_I2)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedDocuments, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 3 /* DblClickCell */, OnDblClickCellDocuments, VTS_I4 VTS_I2)
	ON_EVENT(CHistoryDlg, IDC_PHOTOS_SORT_CRITERION_COMBO, 16 /* SelChosen */, OnSelChosenPhotosSortCriterionCombo, VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 12 /* DragBegin */, OnDragBeginDocuments, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CHistoryDlg, IDC_DOCUMENTS, 8 /* EditingStarting */, OnEditingStartingDocuments, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

CString CHistoryDlg::GetMultipleWhere(CDWordArray& dw)
{
	CString str;
	if(dw.GetSize() == 0)
		return "";

	str = "(";

	for(int i = 0; i < dw.GetSize(); i++) {
		CString tmp;
		tmp.Format("CategoryID = %li OR ", dw.GetAt(i));
		str += tmp;
	}

	//trim off the end 'and'
	str.TrimRight("OR ");
	str += ")";

	return str;
}

void CHistoryDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try{
		// (a.wilson 2014-08-11 08:28) - PLID 63246 - check and update category filter.
		EnsureUpdatedCategoryFilter();

		// (z.manning 2010-01-08 10:18) - PLID 12186 - Added option to show gridlines
		if(GetRemotePropertyInt("HistoryShowGrid", 0, 0, GetCurrentUserName()) == 1) {
			CheckDlgButton(IDC_HISTORY_SHOW_GRIDLINES, BST_CHECKED);
			m_pDocuments->PutGridVisible(VARIANT_TRUE);
		}
		else {
			CheckDlgButton(IDC_HISTORY_SHOW_GRIDLINES, BST_UNCHECKED);
			m_pDocuments->PutGridVisible(VARIANT_FALSE);
		}

		//(e.lally 2010-01-12) PLID 25808 - Added quick button for photo note printing preference
		BOOL bPrintPhotoNotes = GetRemotePropertyInt("PhotoPreviewNotes", 0, 0, GetCurrentUserName(), true)==0 ? FALSE:TRUE;
		CheckDlgButton(IDC_PHOTOS_PRINT_NOTES_CHECK, bPrintPhotoNotes);

		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePersonID();
		
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (m_PatChecker.PeekChanged()) {
			m_ForceRefresh = true;
		}

		// (a.walling 2010-10-14 13:43) - PLID 40978 - Moved refreshing code to Refresh		
		if (bForceRefresh || m_ForceRefresh) {	
			Refresh();
		}
		m_ForceRefresh = false;

		EnableAppropriateButtons();

		// (d.lange 2010-10-27 08:55) - PLID 41088 - Ensure we are displaying the proper Device Import icon
		// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
		OnUpdateDeviceImportButton((WPARAM)(GetMainFrame()->GetDeviceImportRecordCount() > 0 ? TRUE : FALSE), 0);

		//TS 5-14-03: Don't call this here; it depends on the requery being finished, so it's in OnRequeryFinished().
		//FillPathAndFile();

		//e.lally - return so that we don't clear the list below
		return;
	}
	catch(CNxPersonDocumentException *pException) {
		// (d.thompson 2013-07-01) - PLID 13764 - Specially handle exceptions regarding the person documents
		CPatientDocumentStorageWarningDlg dlg(pException->m_strPath);
		//No longer need the exception, clean it up
		pException->Delete();

		//Inform the user
		dlg.DoModal();

	} NxCatchAll("Error in UpdateView");
	try {
		//(e.lally 2006-07-24) PLID 21545 - We weren't able to properly update the view so we'd better clear the list
			//rather than show someone else's documents, a partial list or something else.
		m_pDocuments->Clear();
	}NxCatchAll("Error clearing the documents list");
}

// (a.walling 2010-10-14 13:43) - PLID 40978
void CHistoryDlg::Refresh()
{
	//save the current top row
	m_nTopRow = m_pDocuments->GetTopRowIndex();

	CString strWhere, strPhotoWhere, strEmnWhere;

	// (z.manning 2008-07-01 12:52) - PLID 25574 - Update the EMR preview dialog if we have one.
	if(m_pdlgEmrPreview != NULL) {

		// (j.jones 2009-09-22 11:55) - PLID 31620 - Send an empty list of EMNs now,
		// OnRequeryFinishedDocuments will fill the preview pane once the EMNs
		// are loaded into the list.
		CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
		m_pdlgEmrPreview->SetPatientID(m_id, aryEMNs);
	}

	//reset the cached document path if we switched patients or the name changed
	// (a.walling 2010-10-14 13:45) - PLID 40978 - Do this if forcing a refresh
	//if(m_id != nLastID || m_PatChecker.Changed())
	if(m_PatChecker.Changed() || m_ForceRefresh) {
		// (a.walling 2011-02-11 15:13) - PLID 42441 - If there is a failure, we need to make sure we are no longer
		// using an incorrect path!
		m_strCurPatientDocumentPath.Empty();
		m_strCurPatientDocumentPath = GetCurPatientDocumentPath();
	}

	//determine the filter where clause
	if(IsDlgButtonChecked(IDC_USE_HISTORY_FILTER) && GetDlgItem(IDC_USE_HISTORY_FILTER)->IsWindowEnabled()) {
		if(m_nCurrentFilterID == ID_NO_CAT) {
			strWhere.Format("WHERE MailSent.CategoryID IS NULL AND MailSent.PersonID = %li", m_id);
			strPhotoWhere.Format("MailSent.CategoryID IS NULL ");
		}
		else if(m_nCurrentFilterID == ID_ALL_CATS) {
			//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
			strWhere.Format("WHERE MailSent.PersonID = %li AND %s", m_id, GetAllowedCategoryClause("MailSent.CategoryID"));	//nothing, show all items
			strPhotoWhere = GetAllowedCategoryClause("MailSent.CategoryID");
		}
		else if(m_nCurrentFilterID == ID_MULTIPLE_CATS) {
			strWhere.Format("WHERE MailSent.PersonID = %li AND %s", m_id, GetMultipleWhere(m_aryFilter));
			strPhotoWhere = GetMultipleWhere(m_aryFilter);
			if(m_aryFilter.GetSize() > 0) {
				// (c.haag 2008-09-22 15:00) - PLID 31447 - Removed the "WHERE"; its presence was causing errors in the way it was used
				strEmnWhere.Format("EmnTabCategoryID IN (SELECT EmnTabCategoryID FROM EmrHistoryCategoryLinkT WHERE NoteCategoryID IN (%s)) ", ArrayAsString(m_aryFilter, false));
			}
		}
		else {
			//valid category
			strWhere.Format("WHERE MailSent.CategoryID = %li AND MailSent.PersonID = %li", m_nCurrentFilterID, m_id);
			strPhotoWhere.Format("MailSent.CategoryID = %li ", m_nCurrentFilterID);
			// (c.haag 2008-09-22 15:00) - PLID 31447 - Removed the "WHERE"; its presence was causing errors in the way it was used
			strEmnWhere.Format("EmnTabCategoryID IN (SELECT EmnTabCategoryID FROM EmrHistoryCategoryLinkT WHERE NoteCategoryID = %li) ", m_nCurrentFilterID);
		}
	}
	else {
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		strWhere.Format("WHERE MailSent.PersonID = %li AND %s ", m_id, GetAllowedCategoryClause("MailSent.CategoryID"));
		strPhotoWhere = GetAllowedCategoryClause("MailSent.CategoryID");
	}

	// (j.jones 2009-10-27 09:03) - PLID 15385 - added date filter
	if(m_radioDateRange.GetCheck()) {
		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_dtFrom.GetValue());
		dtTo = COleDateTime(m_dtTo.GetValue());

		if(dtFrom.GetStatus() != COleDateTime::invalid && dtTo.GetStatus() != COleDateTime::invalid) {
			CString strWhere2;
			strWhere2.Format(" AND (MailSent.ServiceDate Is Not Null AND MailSent.ServiceDate >= '%s' AND MailSent.ServiceDate < DATEADD(day, 1, '%s')) ",
				FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
			strWhere += strWhere2;
			strPhotoWhere += strWhere2;
		}
	}

	//TES 8/10/2011 - PLID 44966 - If they don't have permission to view photos, filter them out.
	// (e.frazier 2016-05-19 10:31) - This permission check doesn't take place in the Contacts module
	if (m_bInPatientsModule) 
	{
		if (!(GetCurrentUserPermissions(bioPatientHistory) & (sptDynamic1))) {
			strWhere += " AND (dbo.IsImageFile(PathName) = 0 OR MailSent.IsPhoto = 0)";
			//TES 8/10/2011 - PLID 44966 - Might as well clear out the photos filter, although it doesn't really matter since we'll have 
			// hidden the tab.
			strPhotoWhere = " 1=0 ";
		}
	}
	

	// (a.wetta 2007-07-09 13:36) - PLID 17467 - Make sure the pictures shown on the photo viewer are marked as photos
	strPhotoWhere += "AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1)";
	strPhotoWhere.TrimLeft();
	strPhotoWhere.TrimLeft("AND");

	m_PhotoViewer.SetAdditionalFilter(strPhotoWhere);

	// (c.haag 2004-11-11 10:26) - PLID 14074 - Filter on procedure information / EMRs
	if (GetPicContainer() && m_checkFilterOnEMR.GetCheck())
	{
		//TES 1/20/2005 - Use an OR filter, then if one of these is -1 it doesn't matter.
		long nPicID = GetPicID();
		
		CString str;
		str.Format(" AND (PicID = %d) ", nPicID);
		strWhere += str;

		m_PhotoViewer.SetPicID(nPicID);
		
	}
	else {
		//TES 4/29/2008 - PLID 26711 - If we're NOT filtering on a PIC, the photo viewer needs to know that, too.
		m_PhotoViewer.SetPicID(-1);
	}

	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to No (1 - which means to attach)
	if(GetRemotePropertyInt("DoNotAttachSuperbills",1,0,"<None>",TRUE) == 1) {
		CString str;
		str.Format(" MailBatchID NOT IN (SELECT MailBatchID FROM PrintedSuperbillsT)", m_id);
		strWhere += " AND " + str;
	}

	
	CString strTempWhere;
	long nCurTabSel = m_tab->CurSel;
	if (nCurTabSel != m_tab->GetSize()-1 && nCurTabSel != 0) {
		strTempWhere.Format(" AND CategoryID = %li", m_arTabs.GetAt(nCurTabSel).nCatID);
		if(!strEmnWhere.IsEmpty()) {
			strEmnWhere += " AND ";
		}
		// (z.manning 2008-10-01 10:14) - PLID 31553 - We need to filter on the EMN category as well.
		strEmnWhere += FormatString(" EmnTabCategoryID IN (SELECT EmnTabCategoryID FROM EmrHistoryCategoryLinkT WHERE NoteCategoryID = %li) ", m_arTabs.GetAt(nCurTabSel).nCatID);
	}
	else {
		strTempWhere = "";
	}

	strWhere += strTempWhere;

	// (z.manning 2008-07-01 16:23) - PLID 25574 - We can now also show EMNs on the history tab
	CString strEmnFrom;
	if(m_btnShowEmns.GetCheck() == BST_CHECKED) {
		// (z.manning 2010-02-02 11:38) - PLID 34287 - Added PicID
		// (a.walling 2010-08-03 14:31) - PLID 39867 - Some fixes for the extra notes
		// (z.manning 2011-05-20 11:24) - PLID 33114 - This now filters on EMR charting permissions for the current user
		// (z.manning 2012-09-11 13:26) - PLID 52543 - Added modified date
		strEmnFrom.Format("\r\nUNION \r\n"
			"SELECT -1 AS MailID, NULL AS PacketName, 1 AS SeparateDocuments, '<EMN>' AS Selection, "
			"	'<EMN>' AS PathName, Description AS Note, '' AS Sender, InputDate AS Date, Date AS ServiceDate, "
			"	-25 AS CategoryID, 0 AS IsPhoto, NULL AS MergedPacketID, EmrMasterT.ID AS EmnID, NULL AS PicID, "
			"	CAST(0 AS BIT) AS HasExtraNotes, EmrMasterT.ModifiedDate AS EmnModifiedDate "
			"FROM EmrMasterT "
			"LEFT JOIN EmnTabCategoriesLinkT ON EmrMasterT.ID = EmnTabCategoriesLinkT.EmnID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE PatientID = %li AND Deleted = 0 %s \r\n"
			, m_id, GetEmrChartPermissionFilter().Flatten());
		if(!strEmnWhere.IsEmpty()) {
			strEmnFrom += " AND " + strEmnWhere;
		}
	}

	//TES 9/2/2008 - PLID 6484 - If they're blocking multi-patient documents, don't give them a Word icon (this will have
	// the additional effect of making it impossible to open the document).  The logic for determining whether it's a multi-
	// patient document is the same that the globalutils OpenDocument() function uses.
	CString strSelection = "Selection";
	if(GetRemotePropertyInt("HistoryBlockMultiPatDocs", 1, 0, "<None>")) {
		strSelection = "CASE WHEN PathName LIKE '%MultiPatDoc%' THEN '' ELSE Selection END AS Selection";
	}

	// (j.jones 2008-09-04 17:30) - PLID 30288 - supported MailSentNotesT
	// (z.manning 2010-02-02 11:39) - PLID 34287 - Added PicID
	// (a.walling 2010-08-03 14:31) - PLID 39867 - Some fixes for the extra notes
	// (z.manning 2012-09-11 13:27) - PLID 52543 - Added EMN modified date
	m_strFromClause = FormatString("( "
		"SELECT MailSent.MailID, PacketsT.Name AS PacketName, SeparateDocuments, %s, "
		"	PathName, MailSentNotesT.Note, Sender, Date, ServiceDate, CategoryID AS HistoryCategoryID, "
		"	IsPhoto, MergedPacketID, NULL AS EmnID, PicID, CAST(CASE WHEN NoteInfoT.MailID IS NULL THEN 0 ELSE 1 END AS BIT) AS HasExtraNotes, "
		"	NULL AS EmnModifiedDate "
		"FROM MailSent "
		"LEFT JOIN (MergedPacketsT INNER JOIN PacketsT ON MergedPacketsT.PacketID = PacketsT.ID) ON MailSent.MergedPacketID = MergedPacketsT.ID "
		"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		"LEFT JOIN NoteInfoT ON MailSent.MailID = NoteInfoT.MailID "
		" %s %s"
		")"
		, strSelection, strWhere, strEmnFrom);

	m_pDocuments->PutFromClause(_bstr_t(m_strFromClause + " AS HistoryQ"));
	m_pDocuments->Requery();

	//JMM 2/23/04 Revised for remembering column widths
	//DRT 7/19/02

	if (! IsDlgButtonChecked(IDC_REMEMBER_HISTORY_COLUMNS)) {
		if(IsDlgButtonChecked(IDC_ENABLE_UNATTACHED)) {
			//DRT 11/11/2003 - For now, if we choose to filter on a category, we will NOT be showing unattached items - no matter what.
			// (j.jones 2009-10-27 09:27) - PLID 15385 - same for date filter
			if(!IsDlgButtonChecked(IDC_USE_HISTORY_FILTER) && !m_radioDateRange.GetCheck()) {
				LoadUnattachedFiles();
			}

			m_pDocuments->GetColumn(phtdlcFilePath)->PutStoredWidth(100);
			m_pDocuments->GetColumn(phtdlcFilename)->PutStoredWidth(100);
			m_pDocuments->GetColumn(phtdlcNotes)->PutStoredWidth(150);
		}
		else {
			m_pDocuments->GetColumn(phtdlcFilePath)->PutStoredWidth(0);
			m_pDocuments->GetColumn(phtdlcFilename)->PutStoredWidth(0);
			m_pDocuments->GetColumn(phtdlcNotes)->PutStoredWidth(200);
		}
	}
	else {
		SetColumnSizes();

		if(IsDlgButtonChecked(IDC_ENABLE_UNATTACHED)) {
			// (j.jones 2009-10-27 09:27) - PLID 15385 - do not load unattached files if a filter is in use
			if(!IsDlgButtonChecked(IDC_USE_HISTORY_FILTER) && !m_radioDateRange.GetCheck()) {
				LoadUnattachedFiles();
			}
		}
	}

	m_PhotoViewer.SetPersonID(m_id);
	// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
	m_PhotoViewer.SetPicContainer(GetPicContainer());
	m_PhotoViewer.Refresh();

	m_ForceRefresh = false;
}

void CHistoryDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg1.SetColor(nNewColor);
	// (j.jones 2016-04-15 13:35) - NX-100214 - added a second colored background
	m_bkg2.SetColor(nNewColor);
	// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
	//m_tab->BackColor = nNewColor;
	CPatientDialog::SetColor(nNewColor);
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryDlg message handlers

void CHistoryDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	EnableAppropriateButtons();
	CNxDialog::OnShowWindow(bShow, nStatus);
	
}

void CHistoryDlg::OnClickMerge()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// (z.manning 2009-01-29 11:50) - PLID 32885 - Moved this code to MergeNewDocument
		MergeNewDocument(GetTemplatePath());
		
	} NxCatchAll("Error in CHistoryDlg::OnClickMerge");
}



void CHistoryDlg::OnClickNew() 
{
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		CWaitCursor wc;
		CString fileName = GetPatientDocumentName(m_id), sql;

		CString path = GetCurPatientDocumentPath();

		if(!DoesExist(path)) {
			//doesn't exist? try clearing out the cached value and trying again
			m_strCurPatientDocumentPath = "";
			path = GetCurPatientDocumentPath();

			//if it still doesn't exist, leave
			if(!DoesExist(path)) {
				// This shouldn't be possible because either GetPatientDocumentPath should return a 
				// valid path to a folder that exists, or it should throw an exception
				ASSERT(FALSE);
				AfxThrowNxException("Person document folder '%s' could not be found", path);
				return;
			}
		}

		//assign a category ID based on preferences
		// (j.gruber 2010-01-22 13:56) - PLID 23696 - added isImage check
		long nCategoryID = CheckAssignCategoryID(IsImageFile(fileName));

		// (c.haag 2004-12-16 16:37) - PLID 14987 - Assign the EMR Group ID
		
		long nPicID = GetPicID();
		
		// Create the word app and add a document to it
		// (a.walling 2012-08-16 14:09) - PLID 52137 - WordApplication is no longer used; use CWordApp instead
		// (z.manning 2016-02-11 16:24) - PLID 68230 - No longer Word-specific
		GetWPManager()->CreateAndOpenNewFile(path ^ fileName);

		// Store a record of the document's path
		// (j.jones 2008-09-04 15:46) - PLID 30288 - converted to use CreateNewMailSentEntry,
		// which creates the data in one batch and sends a tablechecker
		// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
		// (d.singleton 2013-11-15 11:18) - PLID 59513 - need to insert the CCDAType when generating a CCDA
		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);
		CreateNewMailSentEntry(m_id, "", SELECTION_WORDDOC, fileName, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, -1, nCategoryID, nPicID, -1, FALSE, -1, "", ctNone);

		// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if in a PIC that has an EMR
		if(GetPicContainer() && GetPicContainer()->GetCurrentEMRGroupID() > 0) {
			//refresh the EMN to update the Word icon
			CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePersonID());
		}

		// Requery the on-screen document listing
		UpdateView();

	} NxCatchAll("Error in CHistoryDlg::OnClickNew");
}

BOOL CHistoryDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2010-07-01 16:03) - PLID 39473 - Load icons
		m_hExtraNotesIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);

		//DRT 5/18/2006 - PLID 16736 - Bulk cache a number of properties ahead of time so we don't have to open
		//	7 different recordsets to load them.  All but the last of these are used in OnInitDialog()
		// (a.walling 2008-09-15 15:58) - PLID 23904 - Added HistoryShowShellIcons pref
		// (a.walling 2008-09-18 14:01) - PLID 30389 - Added WIA preferences
		// (c.haag 2009-09-02 13:17) - PLID 35119 - PatientHistoryDefaultCategory
		g_propManager.CachePropertiesInBulk("HistoryDialog", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PhotoViewerSortCriteria' OR "
			"Name = 'PhotoViewerSortCriteriaOrder' OR "
			"Name = 'EnableUnattachedDocuments' OR "
			"Name = 'HistorySortColumn' OR "
			"Name = 'HistorySortAsc' OR "
			"Name = 'RememberHistoryColumns' OR "
			"Name = 'HistoryShowShellIcons' OR "
			"Name = 'WIA_CameraDevicesOnly' OR "
			"Name = 'WIA_AutoLaunchWizardOnConnect' OR "
			"Name = 'WIA_AutoImportOnCreate' OR "
			"Name = 'PatientHistoryDefaultCategory' OR "
			"Name = 'DoNotAttachSuperbills' "		//This is in UpdateView()
			" OR Name = 'HistoryShowGrid' " // (z.manning 2010-01-08 10:10) - PLID 12186
			" OR Name = 'PhotoPreviewNotes'"//(e.lally 2010-01-12) PLID 25808
			" OR Name = 'PatientHistoryDefaultImageCategory' " // (j.gruber 2010-01-27 09:04) - PLID 23693
			" OR Name = 'SortHistoryTabsAlphabetically' " // (j.jones 2012-04-17 16:44) - PLID 13109
			" OR Name = 'LastSelectedAdminUserID' " // (z.manning 2016-04-19 11:53) - NX-100244
			")",
			_Q(GetCurrentUserName()));


		//DRT 5/27/2004 - PLID 12564 - We need to test to see if we are in XP mode.  Since we're
		//	not actually drawing, I just open this open, and set a member variable.
		{
			UXTheme* pTheme = new UXTheme();
			pTheme->OpenThemeData(GetDlgItem(IDC_NEW)->GetSafeHwnd(), "Button");	//this doesn't matter

			if(pTheme->IsOpen())
				m_bThemeOpen = TRUE;
			else
				m_bThemeOpen = FALSE;

			//cleanup
			pTheme->CloseThemeData();
			delete pTheme;
		}

		//TES 9/1/2006 - PLID 22377 - Tell the photo viewer to get ready.
		m_PhotoViewer.PrepareWindow();

		m_hiconEmn = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMN), IMAGE_ICON, 16,16, 0);

		if (GetPicContainer())
		{
			CPicContainerDlg* pDlg = GetPicContainer();
			m_bkg1.SetColor(pDlg->m_nColor);
			m_bkg2.SetColor(pDlg->m_nColor);

			m_checkFilterOnEMR.ShowWindow(SW_SHOW);

			// (j.jones 2011-06-24 12:05) - PLID 36488 - added a default option for this filter
			BOOL bFilterOnEMR = (GetRemotePropertyInt("PicHistoryTabDefaultFilterByPIC", 1, 0, GetCurrentUserName(), true) == 1);
			m_checkFilterOnEMR.SetCheck(bFilterOnEMR);

			m_PhotoViewer.SetPicID(GetPicID());

			// (a.walling 2008-06-11 09:00) - PLID 30351 - Move the nxcolor control at minimum when in the pic
			CWnd* pBG = GetDlgItem(IDC_HISTORY_BKG);
			CRect rcBG, rcClient;
			GetClientRect(rcClient);
			pBG->GetWindowRect(rcBG);
			ScreenToClient(rcBG);

			rcBG.left = 3;
			rcBG.right = rcClient.right;
			rcBG.top = rcClient.top;

			pBG->MoveWindow(rcBG, FALSE);

			// (a.walling 2008-09-03 12:20) - PLID 19638 - Register for table checker messages
			GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());
		}
		else if(!m_bInPatientsModule) {
			m_bkg1.SetColor(GetNxColor(GNC_CONTACT, 0));
			m_bkg2.SetColor(GetNxColor(GNC_CONTACT, 0));

			//TES 4/29/2008 - PLID 26711 - If we're NOT filtering on a PIC, the photo viewer needs to know that, too.
			m_PhotoViewer.SetPicID(-1);
		}

		//DRT 4/11/2008 - PLID 29636 - NxIconify
		m_btnNew.AutoSet(NXB_NEW);
		m_btnDetach.AutoSet(NXB_DELETE);
		m_btnEditCategories.AutoSet(NXB_MODIFY);
		// (c.haag 2008-09-12 16:50) - PLID 31369 - Set the icon for the acquire button
		m_btnAcquire.SetIcon(IDR_PNG_ACQUIRE, 0, FALSE, TRUE);
		// (c.haag 2008-09-19 09:27) - PLID 31368 - Set the icon for the record button
		m_btnRecordAudio.SetIcon(IDI_GO_RECORD_DLG);

		// (d.lange 2010-10-26 12:24) - PLID 40030 - Make sure we have Device Import license before showing
		if (g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
			//We need to check the status of the Device Import dialog and update the icon on the button
			// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
			OnUpdateDeviceImportButton((WPARAM)(GetMainFrame()->GetDeviceImportRecordCount() > 0 ? TRUE : FALSE), 0);
			
		}else {
			//License is not activated for Device Import, so hide the button
			m_btnDeviceImport.ShowWindow(SW_HIDE);
		}

		m_pDocuments = BindNxDataListCtrl(IDC_DOCUMENTS,false);
		IColumnSettingsPtr pCol = m_pDocuments->GetColumn(phtdlcCategory);
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF WHERE " + GetAllowedCategoryClause("ID") + " UNION SELECT -1, '{No Category}' ORDER BY Description"));

		// (z.manning 2008-07-03 11:54) - PLID 25574 - Make sure the have EMR license and permission before showing
		// the Show EMNs option.
		if((m_bInPatientsModule || GetPicContainer()) && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2 && CheckCurrentUserPermissions(bioPatientEMR, sptRead, FALSE, 0, TRUE, TRUE)) {
			m_btnShowEmns.ShowWindow(SW_SHOW);
			m_btnShowEmns.SetCheck(GetRemotePropertyInt("HistoryTabShowEmns", BST_UNCHECKED, 0, GetCurrentUserName(), true));
		}

		GetDlgItem(IDC_DOCUMENTS)->DragAcceptFiles(TRUE);
		GetDlgItem(IDC_PHOTO_VIEWER)->DragAcceptFiles(TRUE);

		// Bind and load the sort criterion combo (no loading from data, everything is hard coded)
		m_dlPhotosSortCriterion = BindNxDataListCtrl(IDC_PHOTOS_SORT_CRITERION_COMBO, false);
		{
			//(e.lally 2012-04-16) PLID 39543 - Made list alphabetical
			IRowSettingsPtr pRow;
			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscAttachDate);
			pRow->PutValue(1, "Attach Date");
			m_dlPhotosSortCriterion->AddRow(pRow);
			
			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscCategory);
			pRow->PutValue(1, "Category");
			m_dlPhotosSortCriterion->AddRow(pRow);

			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscNote);
			pRow->PutValue(1, "Note");
			m_dlPhotosSortCriterion->AddRow(pRow);

			// (b.cardillo 2005-07-15 15:04) - PLID 16454 - Due to a design flaw in the photo viewer, 
			// we cannot allow the user to put the procedure as anything but the primary sort criteria.  
			// So right now it's hard-coded as primary in the photo viewer (see my comment in 
			// CompareImageInfoSingle() explaining this) which means we should not add it to this combo 
			// because doing so will only confuse the user (because selecting it will have no effect).  
			// Once PLID 17037 is done, we can comment this code back in to restore the capability.
			// (b.cardillo 2005-07-21 12:50) - PLID 17078 - Commented this back in now that PLID 17037 
			// is done.
			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscProcedureName);
			pRow->PutValue(1, "Procedure Name");
			m_dlPhotosSortCriterion->AddRow(pRow);

			//(e.lally 2012-04-16) PLID 39543 - Added service date
			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscServiceDate);
			pRow->PutValue(1, "Service Date");
			m_dlPhotosSortCriterion->AddRow(pRow);


			pRow = (m_dlPhotosSortCriterion->GetRow(sriGetNewRow));
			pRow->PutValue(0, (long)CPhotoViewerCtrl::dscStaff);
			pRow->PutValue(1, "Staff");
			m_dlPhotosSortCriterion->AddRow(pRow);
		}

		//DRT 8/1/2005 - PLID 17124 - Recall saved settings
		long nPhotoSort = GetRemotePropertyInt("PhotoViewerSortCriteria", (long)CPhotoViewerCtrl::dscProcedureName, 0, GetCurrentUserName(), true);
		long nPhotoSortOrder = GetRemotePropertyInt("PhotoViewerSortCriteriaOrder", 0, 0, GetCurrentUserName(), true);	//0 = asc, 1 = desc

		//the "secondary" sort will be attach date, the primary sort whatever they have saved
		m_PhotoViewer.SetPrimarySort(CPhotoViewerCtrl::dscAttachDate, FALSE);

		// Set the controls for the sort criterion and sort order
		long nPhotoRow = m_dlPhotosSortCriterion->SetSelByColumn(0, (long)nPhotoSort);
		OnSelChosenPhotosSortCriterionCombo(nPhotoRow);	//handle any extra stuff we need to do
		CheckDlgButton(IDC_PHOTOS_SORT_DESCENDING_CHECK, nPhotoSortOrder);
		OnPhotosSortDescendingCheck();			//handle anything extra

		//(e.lally 2010-01-12) PLID 25808 - Added quick button for photo note printing preference
		BOOL bPrintPhotoNotes = GetRemotePropertyInt("PhotoPreviewNotes", 0, 0, GetCurrentUserName(), true)==0 ? FALSE:TRUE;
		CheckDlgButton(IDC_PHOTOS_PRINT_NOTES_CHECK, bPrintPhotoNotes);

		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		m_pFilter = BindNxDataListCtrl(IDC_HISTORY_FILTER_LIST, false);
		m_pFilter->WhereClause = _bstr_t(GetAllowedCategoryClause("ID"));
		m_pFilter->Requery();

		// (a.walling 2010-10-14 13:52) - PLID 40978 - Update the patient ID in UpdateView

		/*
		m_id = GetActivePersonID();
		try{
			//(e.lally 2006-07-24) PLID 21545 - This function is known to cause exceptions when 
			//the user does not have permissions to access the folder, path does not exist etc...
			//I am moving it into its own try catch so that the rest of the OnInit code will execute.
			// (a.walling 2011-02-11 15:13) - PLID 42441 - If there is a failure, we need to make sure we are no longer
			// using an incorrect path!
			m_strCurPatientDocumentPath.Empty();
			m_strCurPatientDocumentPath = GetCurPatientDocumentPath();
		}NxCatchAll("Error getting the patient's document path");

		m_PhotoViewer.SetPersonID(m_id);
		*/

		//DRT 5/28/2004 - PLID 12656 - Changed from default 0 to default 1
		CheckDlgButton(IDC_ENABLE_UNATTACHED, GetRemotePropertyInt("EnableUnattachedDocuments", 1, 0, GetCurrentUserName(), true));

		//DRT 7/14/03 - Set the sort that has been saved
		long nColumn = GetRemotePropertyInt("HistorySortColumn", -1, 0, GetCurrentUserName(), false);
		long nAsc = GetRemotePropertyInt("HistorySortAsc", -1, 0, GetCurrentUserName(), false);

		if(nColumn != -1) {
			m_pDocuments->GetColumn((short)nColumn)->PutSortPriority(0);
			if(nAsc == 1)
				m_pDocuments->GetColumn((short)nColumn)->PutSortAscending(VARIANT_TRUE);
			else
				m_pDocuments->GetColumn((short)nColumn)->PutSortAscending(VARIANT_FALSE);
		}

		((CWnd*)GetDlgItem(IDC_HISTORY_FILTER_LIST))->EnableWindow(FALSE);

		//TES 8/4/2011 - PLID 44857 - Check whether they have permission to open the default folder
		// (e.frazier 2016-05-19 10:31) - This permission check doesn't take place in the Contacts module
		if (m_bInPatientsModule)
		{
			if (!(GetCurrentUserPermissions(bioPatientHistory) & (sptDynamic0 | sptDynamic0WithPass))) {
				GetDlgItem(IDC_CURRENT_FOLDER)->EnableWindow(FALSE);
			}
		}
			
		// (j.jones 2009-10-27 09:03) - PLID 15385 - added date filter
		m_radioAllDates.SetCheck(TRUE);
		GetDlgItem(IDC_HISTORY_FROM_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_HISTORY_TO_DATE)->EnableWindow(FALSE);
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		m_dtFrom.SetValue(_variant_t(dtNow));
		m_dtTo.SetValue(_variant_t(dtNow));
		
		//initialize the tab control
		m_tab = GetDlgItemUnknown(IDC_CATEGORY_TAB);
		if (m_tab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return 0;
		}
		else {
			RefreshTabs();
		}

		// (c.haag 2009-09-02 12:54) - PLID 35119 - Select the category based on preference
		{
			const long nDefaultTabID = GetRemotePropertyInt("PatientHistoryDefaultCategory", -1, 0, GetCurrentUserName(), true);
			BOOL bFound = FALSE;

			if (-1 == nDefaultTabID) {				
				m_tab->CurSel = m_tab->GetSize()-1; // Misc. tab
			}
			else if (-2 == nDefaultTabID) {
				m_tab->CurSel = 0; // Photo tab
			}
			else {
				for (int i=0; i < m_arTabs.GetSize() && !bFound; i++) {
					if (m_arTabs[i].nCatID == nDefaultTabID) {
						m_tab->CurSel = i;
						bFound = TRUE;
					}
				}
				if (!bFound) {
					// If we couldn't find the category by ID, defer to the Misc. tab
					m_tab->CurSel = m_tab->GetSize()-1;
				}
			}
		}
			
		// (b.cardillo 2005-07-15 15:22) - PLID 16454 - Hide the controls because we're starting out on 
		// the miscellaneous tab, not the photos tab.
		// (c.haag 2009-09-02 13:07) - PLID 35119 - This may no longer be true based on the user's preference;
		// but continue with legacy behavior.
		GetDlgItem(IDC_PHOTOS_SORT_DESCENDING_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_CRITERION_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_BY_LABEL)->ShowWindow(SW_HIDE);
		//(e.lally 2010-01-12) PLID 25808
		GetDlgItem(IDC_PHOTOS_PRINT_NOTES_CHECK)->ShowWindow(SW_HIDE);

		if (GetRemotePropertyInt("RememberHistoryColumns", 0, 0, GetCurrentUserName(), TRUE)) {

			//check the checkbox
			CheckDlgButton(IDC_REMEMBER_HISTORY_COLUMNS, 1);
			//(e.lally 2006-07-24) PLID 21589 - We want to just call Remember Columns without the update view
				//as that update will already get called as soon as the On Init finishes.
			RememberColumns();
		}
		else {
			CheckDlgButton(IDC_REMEMBER_HISTORY_COLUMNS, 0);
		}

		//DRT 6/22/2004 - PLID 13107 - Need to color the tab if we're in the contacts module - the
		//	patients module calls "SetColor()", and gets handled, but if we're on contacts, no such
		//	luck.
		// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
		/*
		if(!m_bInPatientsModule)
			m_tab->BackColor = GetNxColor(GNC_CONTACT, 0);
		*/

		m_DropTargetTabs.Register(GetDlgItem(IDC_CATEGORY_TAB));
		m_DropTargetTabs.m_nTargetType = TT_TABS;
		// (a.walling 2013-09-03 16:47) - PLID 58039 - Document drop target created on demand
		
		GetControlPositions();

		// (c.haag 2009-09-02 13:07) - PLID 35119 - If we aren't starting on the Misc. tab, then we need
		// to handle the tab change here
		if (m_tab->CurSel != m_tab->GetSize()-1) {
			OnSelectTabDocTabs(m_tab->CurSel, m_tab->GetSize()-1);
		}

	}NxCatchAll("Error initializing history tab.");

	return TRUE;
}

void CHistoryDlg::OnClickAttach()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		CString path;
		//DRT 10/28/2003 - PLID 9921 - Added a row for "Commonly Attached" which contains *.doc, *.xls, *.jpg, 
		//*.bmp, *.pcx, *.tiff, *.wav.  This is the default selected item, so that users can quickly add things
		//without changing the file type.
		//	Also changed the description of the other items to include the types they show
		//PLID 18882 - added PDF's to the commonly selected files and their own type
		// (a.walling 2007-07-19 11:28) - PLID 26748 - Added Office 2007 files, also .jpeg
		// (a.walling 2007-09-17 16:11) - PLID 26748 - Also need PowerPoint 2007 files.
		// (a.walling 2007-10-12 14:50) - PLID 26342 - Moved to a shared string, and also include other formats
		// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, szCommonlyAttached, this);
		CString strInitPath = GetCurPatientDocumentPath();

		if(!DoesExist(strInitPath)) {
			//doesn't exist? try clearing out the cached value and trying again
			m_strCurPatientDocumentPath = "";
			strInitPath = GetCurPatientDocumentPath();

			//if it still doesn't exist, leave
			if(!DoesExist(strInitPath)) {
				// This shouldn't be possible because either GetPatientDocumentPath should return a 
				// valid path to a folder that exists, or it should throw an exception
				ASSERT(FALSE);
				AfxThrowNxException("Person document folder '%s' could not be found", path);
				return;
			}
		}

		dlg.m_ofn.lpstrInitialDir = strInitPath;
		//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
		char * strFile = new char[5000];
		strFile[0] = 0;
		dlg.m_ofn.nMaxFile = 5000;
		dlg.m_ofn.lpstrFile = strFile;

		if (dlg.DoModal() == IDOK) {			
			POSITION p = dlg.GetStartPosition();
			while (p) {
				// (c.haag 2006-10-09 09:32) - PLID 22679 - If the file has a .wav extension, then import it with the audio icon
				CString strSelection = SELECTION_FILE;
				CString strFile = dlg.GetNextPathName(p);
				if (IsWaveFile(strFile)) {
					strSelection = SELECTION_AUDIO;
				}
				// (j.gruber 2010-01-22 13:41) - PLID 23693 - moved to support checking whether its an image
				long nCategoryID = CheckAssignCategoryID(IsImageFile(strFile));
				AttachFileToHistory(strFile, GetActivePersonID(), GetSafeHwnd(), nCategoryID, strSelection);
			}
			UpdateView();
		}
		delete [] strFile;
	} NxCatchAll("Error in CHistoryDlg::OnClickAttach");
}

void CHistoryDlg::OnClickAttachFolder()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		CString path;
		// (a.walling 2008-01-29 14:33) - PLID 28716 - "Attach History Path" preference is now deprecated.
		if (Browse(m_hWnd, "", path, false)) {
			// (j.gruber 2010-01-22 13:42) - PLID 23693 - will always be false for folders
			AttachFileToHistory(path, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(FALSE), SELECTION_FOLDER);
			if(IsDlgButtonChecked(IDC_ENABLE_UNATTACHED))
				UpdateView();	//update the view if we're showing things in directories so everything in the one we just added shows up
		}
	} NxCatchAll(__FUNCTION__);
}

// From RAC, implemented by CAH. TODO: Put this in GlobalUtils or NxStandard.
BOOL FindFileRecursively(const CString &strPath, const CString &strFileName, CString &strFoundInPath)
{
 // First see if we can find it in this directory
 CFileFind ff;
 if (ff.FindFile(strPath ^ strFileName)) {
  strFoundInPath = strPath;
  return TRUE;
 }
 // We couldn't find it so search all subfolders
 BOOL bContinue = ff.FindFile(strPath ^ "*.*");
 while (bContinue) {
  bContinue = ff.FindNextFile();
  if (ff.IsDirectory()) {
   if (!ff.IsDots()) {
    // This is a valid directory so search inside it to queue all the files in there
    if (FindFileRecursively(strPath ^ ff.GetFileName(), strFileName, strFoundInPath))
     return TRUE;
    }
   }
 }
 
 // If we made it here it wasn't found in our directory, or any of our subdirectories
 return FALSE;
}

// (c.haag 2010-06-08 10:12) - PLID 38731 - This code is called either when the user left-clicks 
// on the document list, or when another event wants to simulate a left-click.
void CHistoryDlg::HandleDocumentAction(long nRow, short nCol)
{
	switch (nCol)
	{	
	case phtdlcExtraNotesIcon:
		// (c.haag 2010-07-01 16:19) - PLID 39473 - Extra notes handling
		{
			// If the cell is empty, do nothing
			_variant_t v = m_pDocuments->GetValue(nRow, nCol);
			if (v.vt == VT_NULL || v.vt == VT_EMPTY) {
				// If we get here, it means the row cannot have notes assigned to it (it most likely does not have a MailID)
				return;
			}
			long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);
			if (-1 == nMailID) {
				// We should never get into this state, because it means a history item has a notes icon but no ID in the
				// history table! That means the note cannot possibly be tied to the history item. We should throw an exception.
				ThrowNxException("Attempted to add a note to an item that is not in the history table!");
			}

			// Let the user add extra notes to this history item
			CNotesDlg dlgNotes(this);			
			// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
			dlgNotes.SetPersonID(m_id);
			//dlgNotes.m_nHistoryNotePersonID = m_id; // (c.haag 2010-08-26 11:26) - PLID 39473
			dlgNotes.m_bIsHistoryNote = true;
			dlgNotes.m_bIsForPatient = m_bInPatientsModule || GetPicContainer(); // (a.walling 2010-09-09 12:20) - PLID 40267
			dlgNotes.m_clrHistoryNote = m_bkg1.GetColor(); // (c.haag 2010-08-26 14:49) - PLID 39473
			dlgNotes.m_nMailID = nMailID;
			CNxModalParentDlg dlg(this, &dlgNotes, CString("History Item Notes"));
			dlg.DoModal();

			// Now update the icon
			// (a.walling 2010-08-03 14:31) - PLID 39867 - Some fixes for the extra notes
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT CAST(CASE WHEN NoteInfoT.MailID IS NULL THEN 0 ELSE 1 END AS BIT) AS HasExtraNotes "
				"FROM MailSent LEFT JOIN NoteInfoT ON MailSent.MailID = NoteInfoT.MailID WHERE MailSent.MailID = {INT}", nMailID);
			if (prs->eof) {
				// If we get here, someone probably deleted the history item from another terminal.
				AfxMessageBox("The history item could not be found; it may have been deleted by another user. Your screen will now refresh.", MB_OK | MB_ICONWARNING);
				UpdateView();
			}
			else {
				BOOL bHasNotes = AdoFldBool(prs, "HasExtraNotes");
				if (bHasNotes) {
					m_pDocuments->PutValue(nRow,phtdlcExtraNotesIcon,(long)m_hExtraNotesIcon);
				} else {
					m_pDocuments->PutValue(nRow,phtdlcExtraNotesIcon,(LPCTSTR)"BITMAP:FILE");
				}
			}
		}
		break;

		// (a.walling 2008-09-15 16:48) - PLID 23904 - use Icon also
	case phtdlcIcon:  // Icon
	case phtdlcName:  // Name
		{	

			CWaitCursor pWait;

			long nEmnID = VarLong(m_pDocuments->GetValue(nRow, phtdlcEmnID), -1);
			if(nEmnID != -1) {
				// (z.manning 2008-07-01 14:04) - PLID 25574 - This is an EMN row so open the EMR preview dialog
				OpenEmrPreviewDialog(nEmnID);
				return;
			}

			_variant_t tmpVar = m_pDocuments->GetValue(nRow,phtdlcMailID);
			_variant_t varName = m_pDocuments->GetValue(nRow, phtdlcName);
			CString name = "";
			bool bIsPacket = false;
			if(varName.vt == VT_I4) {
				bIsPacket = true;
			}
			else {
				name = VarString(varName);
			}
			CString filename = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));

			// (j.jones 2009-12-18 08:31) - PLID 34606 - the previous permission check here did not support
			// passwords, and there is no reason why not to
			// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
			if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
				return;
			}

			if(filename.IsEmpty()) {
				//one of the unattached documents
				filename = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal) + "\\" + CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);
			}

			// (c.haag 2015-05-06) - NX-100442 - If this is a packet, then open it as such. Otherwise defer to the DocumentOpener
			// object which inherits all of the previous business logic here so that it may be shared with other classes.
			if (bIsPacket)
			{
				//TES 2003-1-12: I'm changing this because a.) it's wrong, and b.) it's not modular.
				_RecordsetPtr rsPacket = CreateParamRecordset("SELECT MergedPacketID FROM MailSent WHERE MailID = {INT}", VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID)));
				// (c.haag 2015-05-04) - NX-100442 - This specific code can't be refactored because other Practice objects call CHistoryDlg::OpenPacket
				// and part of the business logic includes altering the history datalist selection.
				OpenPacket(AdoFldLong(rsPacket, "MergedPacketID"));
			}
			else
			{
				if (CDocumentOpener::OpenHistoryAttachment(filename, m_id, VarLong(tmpVar), this, m_bCaseHistoryIsOpen, name).bNeedUpdateView)
				{
					UpdateView();
				}
			}
		}
		break;
	} // end switch	
}

void CHistoryDlg::OnLeftClickDocuments(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2010-06-08 10:12) - PLID 38731 - All the code for this event is now in a utility function
		HandleDocumentAction(nRow, nCol);
	} NxCatchAll("Error opening document.");
}

void CHistoryDlg::OnEditingFinishedDocuments(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{

	try{
		if (!bCommit)
			return;

		switch(nCol){
		case phtdlcNotes:
			{
				long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);
				BOOL nCaseHistoryID = -1;

				//check for unattached items
				bool bNeedRefresh = false;
				if(nMailID == -1) {
					if(!bCommit)
						return;
					
					//we've commited to attaching this document, so let's do so
					CString strPath, strFile, strSel;
					strPath = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal);
					strFile = CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);
					strSel = CString(m_pDocuments->GetValue(nRow, phtdlcName).bstrVal);

					// Attach it
					{
						// Attach the file, which gives us the new ID
						// (j.gruber 2010-01-22 13:43) - PLID 23693 - added ImageFile check
						long nNewMailSentID = AttachFileToHistory(strPath + "\\" + strFile, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strFile)), strSel);
						// (b.cardillo 2010-08-27 13:31) - PLID 31923 - Pull the new id from the save operation 
						// itself instead of trying to guess it afterward
						if (nNewMailSentID > 0) {
							// It was attached successfully, so remember the new ID for updating below
							nMailID = nNewMailSentID;
							bNeedRefresh = true;
						}
					}
				}

				else {
					// See if this is a case history object
					if (VarString(m_pDocuments->GetValue(nRow, phtdlcPath)).CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) == 0) {
						// This is a case history object so changing the mailsent note is the same thing changing the case history name
						if (IsSurgeryCenter(true)) {
							// Get the case history ID out of the mailsent record (TODO: This should be in the datalist)
							_RecordsetPtr prs = CreateRecordset(
								"SELECT InternalRefID FROM MailSent WHERE MailID = %li", 
								VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID)));
							nCaseHistoryID = AdoFldLong(prs->GetFields(), "InternalRefID");
						} else {
							// TODO: Give license message
						}
					}
				}

				// Write to the database
				CString strNewValue = VarString(varNewValue);
				BEGIN_TRANS("CHistoryDlg::OnEditingFinishedDocuments:Note") {
					if (nCaseHistoryID != -1) {
						// We have to update the name of the casehistory object as well
						ExecuteSql("UPDATE CaseHistoryT SET Name = '%s' WHERE ID = %li", _Q(strNewValue), nCaseHistoryID);
						//TES 1/10/2006 - PLID 23575 - Audit the change
						// (c.haag 2007-03-09 10:52) - PLID 25138 - If we're in the PIC, get the patient name from there
						AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiCaseHistoryDescription, nCaseHistoryID, VarString(varOldValue), strNewValue, aepMedium, aetChanged);
					}
					// No matter what, we always want to update the notes field
					// (j.jones 2008-09-04 13:45) - PLID 30288 - supported MailSentNotesT
					ExecuteSql("UPDATE MailSentNotesT SET Note = '%s' WHERE MailID = %li", _Q(strNewValue), nMailID);
					
					// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
					CClient::RefreshMailSentTable(m_id, nMailID, GetRowPhotoStatus(nRow));

				} END_TRANS("CHistoryDlg::OnEditingFinishedDocuments:Note");

					// (b.cardillo 2010-08-27 17:20) - PLID 31923 - Only refresh the whole view if necessary (in most 
					// cases we've only just written to data something that is already set on screen, so there's no 
					// need to refresh at all).
				if (bNeedRefresh) {
					UpdateView();
				}
			}
			break;
		case phtdlcStaff:
			{
				if(bCommit) {
					long nMailID = VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID));
					bool bNeedRefresh = false;
					if(nMailID == -1) {
						//we've commited to attaching this document, so let's do so
						CString strPath, strFile, strSel;
						strPath = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal);
						strFile = CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);
						strSel = CString(m_pDocuments->GetValue(nRow, phtdlcName).bstrVal);

						// Attach it
						{
							// Attach the file, which gives us the new ID
							// (j.gruber 2010-01-22 13:43) - PLID 23693 - added IsImage to Category check
							long nNewMailSentID = AttachFileToHistory(strPath + "\\" + strFile, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strFile)), strSel);
							// (b.cardillo 2010-08-27 13:31) - PLID 31923 - Pull the new id from the save operation 
							// itself instead of trying to guess it afterward
							if (nNewMailSentID > 0) {
								// It was attached successfully, so remember the new ID for updating below
								nMailID = nNewMailSentID;
								bNeedRefresh = true;
							}
						}
					}
					CString strNewValue = VarString(varNewValue);
					ExecuteSql("UPDATE MailSent SET Sender = '%s' WHERE MailID = %li", _Q(strNewValue), nMailID);
					
					// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
					CClient::RefreshMailSentTable(m_id, nMailID, GetRowPhotoStatus(nRow));
					
					// (b.cardillo 2010-08-27 17:20) - PLID 31923 - Only refresh the whole view if necessary (in most 
					// cases we've only just written to data something that is already set on screen, so there's no 
					// need to refresh at all).
					if (bNeedRefresh) {
						UpdateView();
					}
				}
			}
			break;
		case phtdlcFilename:
			{	//user editing the filename, so let's rename the files they've got

				if(bCommit) {
					CString strOldname, strNewname, strPath, str;
					long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);

					strOldname = CString(varOldValue.bstrVal);
					// (b.spivey, February 18, 2015) PLID 56539 - trim spaces off-- we shouldn't allow leading/trailing spaces
					strNewname = CString(varNewValue.bstrVal).Trim();

					//if we're here, let's rename the file
					strPath = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal);

					if(strPath.Right(1) != "\\")
						strPath += "\\";

					if(rename(strPath + strOldname, strPath + strNewname)) {
						//DRT 2/9/2004 - PLID 10902 - If it is a multi patient document, it may well not be there, 
						//	we need to check in the multi pat path to rename it
						CString strMultiPath = GetSharedPath() ^ "Documents\\---25\\";
						if(rename(strMultiPath + strOldname, strMultiPath + strNewname)) {
							AfxMessageBox("The file could not be renamed. It is possible another file with the same name exists, or the file you are renaming has been deleted.");
							m_pDocuments->PutValue(nRow, phtdlcFilename, varOldValue);
							return;
						}
						else {
							//success!  but now we need to update MailSent to be fixed for all the other people using 
							//this document							

							// (j.jones 2011-07-22 15:41) - PLID 21784 - get a list of the MailIDs affected (rarely more than one)
							CArray<long, long> aryMailIDs;
							CArray<long, long> aryPatientIDs;
							// (j.jones 2014-08-04 17:25) - PLID 63159 - need to load the patient IDs too
							ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT MailID, PersonID FROM MailSent WHERE PathName = {STRING}", strOldname);
							while(!rs->eof) {
								aryMailIDs.Add(VarLong(rs->Fields->Item["MailID"]->Value));
								aryPatientIDs.Add(VarLong(rs->Fields->Item["PersonID"]->Value));
								rs->MoveNext();
							}
							rs->Close();

							ExecuteParamSql("UPDATE MailSent SET PathName = {STRING} WHERE MailID IN ({INTARRAY})", strNewname, aryMailIDs);
							
							// (j.jones 2011-07-22 15:43) - PLID 21784 - send a tablechecker for each file changed, usually it is only one
							ASSERT(aryPatientIDs.GetSize() == aryMailIDs.GetSize());
							for(int i=0; i<aryMailIDs.GetSize(); i++) {
								// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
								CClient::RefreshMailSentTable(aryPatientIDs.GetAt(i), aryMailIDs.GetAt(i), GetRowPhotoStatus(nRow));
							}
						}
					}

					//otherwise, we need to fix the MailSent table
					try {
						CString strUpdate, temp;

						//find out if it's in the default folder or not
						temp = GetCurPatientDocumentPath() + "\\";

						if(!stricmp(strPath, temp))
							strUpdate = strNewname;
						else
							strUpdate = strPath + strNewname;

						//(e.lally 2008-04-16) PLID 28296 - Added the _Q() for filenames with apostrophes.
						ExecuteSql("UPDATE MailSent SET PathName = '%s' WHERE MailID = %li", _Q(strUpdate), nMailID);
						
						// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
						CClient::RefreshMailSentTable(m_id, nMailID, GetRowPhotoStatus(nRow));

						//DRT 2/2/2004 - PLID 10404 - Must also update the phtdlcPath field, when you click an icon to open the
						//	file, it reads from that field.
						m_pDocuments->PutValue(nRow, phtdlcPath, _bstr_t(strUpdate));

					} NxCatchAll("Error updating MailSent in OnEditingFinishedDocuments()");

				}
			}
			break;
		case phtdlcServiceDate:
			{	//editing the service date

				try {
					//EditingFinishing handles any error states.  If we got here just save it
					COleDateTime dt = VarDateTime(varNewValue);
					long nMailID = VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID));
					
					CArray<long, long> aryMailIDs;

					//DRT 9/11/03 - Damn merged packets.  They're all grouped together in the headings, so if the date changes
					//	for one, we need to update them all.
					// (j.jones 2014-08-04 17:50) - PLID 63159 - I reworked this so we just get the MailIDs for all entries
					// from the same packet, if it happens to be a packet
					_RecordsetPtr prs = CreateParamRecordset("SELECT MailID FROM MailSent WHERE PersonID = {INT} "
						"AND (MailID = {INT} OR MergedPacketID IN (SELECT MergedPacketID FROM MailSent WHERE MailID = {INT}))",
						m_id, nMailID, nMailID);
					while(!prs->eof) {
						aryMailIDs.Add(AdoFldLong(prs, "MailID"));
						prs->MoveNext();
					}
					prs->Close();

					if (aryMailIDs.GetSize() == 0) {
						//this should be impossible
						ASSERT(FALSE);
						aryMailIDs.Add(nMailID);
					}

					ExecuteParamSql("UPDATE MailSent SET ServiceDate = {OLEDATETIME} WHERE PersonID = {INT} AND MailID IN ({INTARRAY})", dt, m_id, aryMailIDs);
					
					// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
					for (int i = 0; i < aryMailIDs.GetSize(); i++) {
						CClient::RefreshMailSentTable(m_id, aryMailIDs.GetAt(i), GetRowPhotoStatus(nRow));
					}

				} NxCatchAll("Error saving ServiceDate");
			}
			break;
		case phtdlcCategory:
			{	//editing the category
				long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);
				CString strFilename = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal) + "\\" + CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);
				CString strFileNameOnly = CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);
				
				long nCatID;
				if (varNewValue.vt == VT_EMPTY) 
					nCatID = -1;
				else
					nCatID = VarLong(varNewValue, -1);
				
				BOOL bIsPacket = FALSE;
				_variant_t varName = m_pDocuments->GetValue(nRow, phtdlcName);
				if(varName.vt == VT_I4)
					bIsPacket = TRUE;
			
				// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variant
				if (!ChangeDocumentCategory(nMailID, strFilename, strFileNameOnly, nCatID, bIsPacket, m_pDocuments->GetValue(nRow, phtdlcIsPhoto)))
					return;
			}
			break;
		case phtdlcIsPhoto:
			{   // (a.wetta 2007-07-09 12:41) - PLID 17467 - Editing the photo status of the item
				IRowSettingsPtr pRow = m_pDocuments->GetRow(nRow);

				SetPhotoStatus(pRow, VarBool(varNewValue));
			}
			break;
		}

	}NxCatchAll("Error 100: CHistoryDlg::OnEditingFinishedDocuments");
}

// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variant
BOOL CHistoryDlg::ChangeDocumentCategory(long nMailID, CString strFilename, CString strFileNameOnly, long nNewCatID, BOOL bIsPacket, _variant_t varIsPhoto) {
	try { 
		bool bRefresh = false;

		if(nMailID == -1) {
			//this item isn't attached! 
			if(AfxMessageBox("This item is not currently attached.  Would you like to attach it?", MB_YESNO) == IDNO)
				return TRUE;

			// (j.jones 2009-12-18 08:31) - PLID 34606 - currently everywhere that calls this function
			// checks permissions first
			//if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
			//	return TRUE;
			//}

			//do the attaching

			// (c.haag 2006-10-09 09:32) - PLID 22679 - If the file has a .wav extension, then import it with the audio icon
			CString strSelection = SELECTION_FILE;
			if (IsWaveFile(strFilename)) {
				strSelection = SELECTION_AUDIO;
			}
			// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file
			nMailID = AttachFileToHistory(strFilename, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strFileNameOnly)), strSelection);

			if(nMailID == 0)
			{
				MsgBox("Failed to set category after attaching document.");
				return FALSE;
			}

			bRefresh = true;
		}

		CString strCat;
		if(nNewCatID == -1)
			strCat = "NULL";
		else
			strCat.Format("%li", nNewCatID);

		if(bIsPacket) {
			//is a packet
			ExecuteSql("UPDATE MailSent SET CategoryID = %s WHERE MergedPacketID IN (SELECT MergedPacketID FROM MailSent WHERE MailID = %li)", strCat, nMailID);
		}
		else {
			ExecuteSql("UPDATE MailSent SET CategoryID = %s WHERE MailID = %li", strCat, nMailID);
		}

		// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker
		CClient::RefreshMailSentTable(m_id, nMailID, varIsPhoto);

		if(bRefresh)
			UpdateView();

	}NxCatchAll("Error in CHistoryDlg::ChangeDocumentCategory");

	return TRUE;
}

void CHistoryDlg::OnEditingFinishingDocuments(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{	
	// (a.walling 2007-02-20 12:10) - PLID 24133 - If they are cancelling, we don't need to go through any of this logic.
	if (*pbCommit == FALSE)
		return;

	//If this is the notes column
	if(nCol == phtdlcNotes){
		long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);

		// (j.jones 2008-09-04 12:17) - PLID 30288 - I increased the length limit to 4000.
		long nMaxChars = 4000;
		// (a.walling 2007-02-26 12:02) - PLID 24133 - We need to see if this is a packet that was not merged seperately.
		// if so, we are just seeing the name of the packet. Although if we edit it, it will use min(mailid) and update
		// THAT record, which we will never see since the packet displays as a single item.

		if (nMailID != -1) {
			_RecordsetPtr prs = CreateRecordset("SELECT SeparateDocuments FROM MailSent LEFT JOIN MergedPacketsT ON MailSent.MergedPacketID = MergedPacketsT.ID WHERE MailID = %li", nMailID);
			if (prs->eof) {
				// what? there is no mailid?
				ASSERT(FALSE);
			} else {
				_variant_t var = prs->Fields->Item["SeparateDocuments"]->Value;

				if (var.vt == VT_BOOL) {
					// this is a packet.
					if (VarBool(var)) {
						// seperate documents! We don't need to do anything special.
					} else {
						// aha, the documents are all appearing as a single line. We'll need to warn the user.
						MessageBox("Unless a packet is merged as separate documents, you can not modify the note. The current note is the name of the packet.", "Practice", MB_OK | MB_ICONHAND);
						*pbCommit = FALSE;
						*pbContinue = TRUE;
						return;
					}
				}
			}
		}


		// (a.walling 2007-02-26 12:01) - PLID 24936 - CaseHistoryT.Name only allows 512 chars as compared to the default 4000 in MailSent
		// See if this is a case history object
		_variant_t vtPath = m_pDocuments->GetValue(nRow, phtdlcPath);
		if ((vtPath.vt == VT_BSTR) && VarString(vtPath).CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) == 0) {
			// This is a case history object so changing the mailsent note is the same thing changing the case history name
			// (a.walling 2007-03-19 09:01) - PLID 24936 - This was silly, regardless of whether or not they have an ASC
			// license, we should still be enforcing the maximum number of characters.
			nMaxChars = 512; // CaseHistoryT.Name

			if (!IsSurgeryCenter(true)) {
				// TODO: Give license message
			}
		}

		//If this note is longer than the maximum characters.
		int nLength = strlen(strUserEntered);
		if(nLength > nMaxChars){
			*pbCommit = FALSE;
			//That's no good, let's ask them how committed they are to the last n-nMaxChars characters of this note
			// (a.walling 2007-02-20 12:11) - PLID 24133 - Added better message handling here.
			// (a.walling 2007-02-26 12:01) - PLID 24936 - Included variable for different limits.
			int nID = MessageBox(FormatString("Notes longer than %li characters cannot be stored in this field.  Would you like to continue?  If you answer yes, the note will be truncated to %li characters", nMaxChars, nMaxChars), "Invalid Field Length", MB_YESNOCANCEL);
			
			//If they care nothing for those characters, and are willing for them to be cast onto the ash heap of history
			if(nID == IDYES){
				//Let's truncate the field, like we warned them they would
				// (a.walling 2007-02-26 12:01) - PLID 24936
				//Using CString's member functions instead since we may have a variable length here.
				CString strNewNote = CString(strUserEntered).Left(nMaxChars);
				*pvarNewValue = _variant_t(strNewNote).Detach();
				*pbCommit = TRUE;
				*pbContinue = TRUE;
			}
			//they care for those characters like their own children, and we must leave them to make the emotionally wrenching but inescapable choice as to which are the lucky nMaxChars characters that will actually get written to data.
			else if (nID == IDNO) {
				//Don't let them go on
				*pbContinue = FALSE;
				//Select the offending characters
				m_pDocuments->SetEditingHighlight(nMaxChars, nLength, FALSE); // see PLID 24827 since this is having issues.
			} else if (nID == IDCANCEL) {
				// (when in rome...) apparently the user is more comfortable abandoning all of their bytechildren rather than choosing some over the other.
				*pbContinue = TRUE;
			} else {ASSERT(FALSE);}
		}

		if(nMailID == -1) {
			CString str;

			str.Format("You are attempting to make a note for an unattached file, would you like to attach this file with this note?");
			if(MessageBox(str, "Confirm", MB_YESNO) == IDNO) {
				*pbCommit = false;
				return;
			}
		}
	}

	//If they're changing the user.
	if(nCol == phtdlcStaff) {
		
		//(c.copits 2011-07-12) PLID 39361 - The 'Staff' column should have a permission to NOT allow users to change who scanned in the document.
		// (e.frazier 2016-05-19 10:31) - This permission check doesn't take place in the Contacts module
		if (m_bInPatientsModule) 
		{
			if (!CheckCurrentUserPermissions(bioPatientHistoryChangeStaff, sptWrite)) {
				*pbCommit = false;
				return;
			}
		}
		
		long nMailID = m_pDocuments->GetValue(nRow, phtdlcMailID);
		if(nMailID == -1) {
			CString str;

			str.Format("You are attempting to assign a user to an unattached file, would you like to attach this file with this user?");
			if(MessageBox(str, "Confirm", MB_YESNO) == IDNO) {
				*pbCommit = false;
				return;
			}
		}
	}


	//if they're changing the filename
	if(nCol == phtdlcFilename) {
		//the following cases cause nothing to happen
		CString strOld, strNew, str;

		strOld = CString(varOldValue.bstrVal);
		// (b.spivey, February 18, 2015) PLID 56539 - trim spaces off-- we shouldn't allow leading/trailing spaces
		strNew = CString(pvarNewValue->bstrVal).Trim();

		//if they didn't change the name, or the old one was blank (directory), or the new one is blank (bad), don't save
		if(strOld == strNew || strOld == "" || strNew == "") {
			*pbCommit = false;
			return;
		}

		//setup a warning to make sure they want to do this
		str.Format("Are you sure you wish to rename the file from %s to %s?", strOld, strNew);
		if(MessageBox(str, "Confirm", MB_YESNO) == IDNO) {
			*pbCommit = false;
			return;
		}
	}

	if(nCol == phtdlcServiceDate)
	{	//editing the service date
		if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899) {
			MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
			*pbCommit = FALSE;
		}
	}

	if(nCol == phtdlcIsPhoto)
	{   // (a.wetta 2007-07-09 12:41) - PLID 17467 - Editing the photo status of the item
		IRowSettingsPtr pRow = m_pDocuments->GetRow(nRow);
		CString strPath = (LPCTSTR)(_bstr_t(pRow->GetValue(phtdlcPath)));

		// (a.walling 2007-07-25 12:36) - PLID 17467 - Non-image files should have a NULL value here, meaning no checkbox.
		if (IsImageFile(strPath)) {
			if (VarLong(pRow->GetValue(phtdlcMailID)) == -1) {
				// This file isn't attached, so it can't be shown in the photos tab
				MessageBox("This document cannot be marked as a photo because it is not attached.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbCommit = FALSE;
			}
			//TES 8/10/2011 - PLID 44966 - If they don't have the "View Photos" permission, then they can't assign Photo status to a file.
			// (e.frazier 2016-05-19 10:31) - No permission check if we are in the Contacts module
			else if (m_bInPatientsModule) 
			{
				if (!CheckCurrentUserPermissions(bioPatientHistory, sptDynamic1)) {
					*pbCommit = FALSE;
				}
			}
		} else {
			*pbCommit = FALSE;
		}
		/*
		else if (!IsImageFile(strPath)) {	
			// The file wasn't an image file, so it can't be marked as a photo
			MessageBox("This document cannot be marked as a photo because it is not an image file.", NULL, MB_OK|MB_ICONEXCLAMATION);
			*pbCommit = false;
		}*/
	}

	//Otherwise, we don't need to do anything, the OnEditingFinished will take it from here.

}

void CHistoryDlg::OnNew() 
{
	// (e.lally 2009-06-11) PLID 34600 - Get permissions and disable menu options accordingly
	// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
	DWORD dwWritePermitted = (MF_DISABLED | MF_GRAYED);
	if(GetCurrentUserPermissions(GetHistoryPermissionObject()) & (sptWrite|sptWriteWithPass)){
		dwWritePermitted = MF_ENABLED;
	}

	CMenu mnu;
	mnu.CreatePopupMenu();
	long nIndex = 0;
	// (z.manning 2009-01-29 10:53) - PLID 32885 - Added options for EMR standard merging if they're
	// licensed for it.
	if(g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent)) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_MERGE_EMR_STD_DOCUMENT, "Merge &EMR Standard Template");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_MERGE_EMR_STD_PACKET, "Merge EM&R Standard Packet");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
	}

	// (d.thompson 2010-01-13) - PLID 30949 - Added a preference to hide this option, only allowing users to Import & Attach
	// (d.thompson 2010-03-23) - PLID 37850 - m.clark changed default to 'hide', not 'show'
	if(GetRemotePropertyInt("HistoryTabHideAttach", 1, 0, "<None>", true) == 0) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_ATTACH_DOCUMENT, "&Attach Existing File");
	}
	mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_ATTACH_FOLDER, "Attach Existing &Folder");
	mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_IMPORT_AND_ATTACH, "&Import and Attach Existing File");

	// (a.walling 2008-09-05 12:40) - PLID 31282 - Prepare popup submenu
	CMenu submenu;
	submenu.CreatePopupMenu();
	// (c.haag 2008-09-12 17:05) - PLID 31369 - The menu is now populated here.
	PopulateAcquireMenu(submenu);
	mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_POPUP|dwWritePermitted, (UINT_PTR)submenu.GetSafeHmenu(), "Import from &Scanner/Camera");
	if (g_pLicense->GetPPCCountAllowed() > 0)
	{
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_IMPORT_FROM_PDA, "Import from &PDA");
	}

	// (d.lange 2010-06-21 16:40) - PLID 39202 - Adding the Device Import dialog menu item
	// (c.haag 2010-06-30 15:27) - PLID 39424 - Added license checking
	if (g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_DEVICE_IMPORT, "Import from De&vice");
	}

	// (a.walling 2009-05-05 16:53) - PLID 34176 - Menu option to impor a CCD document
	if ( (m_bInPatientsModule || GetPicContainer()) && (g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) ) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
		// (j.gruber 2013-11-08 09:59) - PLID 59376 - added ability to import ccda
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_IMPORT_CCD, "Import CC&DA / CCD / CCR...");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_CREATE_CCD, "Create CCD S&ummary...");
		// (j.gruber 2013-11-08 08:32) - PLID 59375 - create ccda
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		// (b.savon 2014-05-01 16:47) - PLID 61909 - Patients Module > History Tab > New Add a pop-out menu to the “Create Summary of Care…” option from the menu. It should give you 2 choices: 1. Generate 2. Customize…
		CMenu submenuCCDA;
		submenuCCDA.CreatePopupMenu();
		long nSubIndex = 0;
		submenuCCDA.InsertMenu(nSubIndex++, MF_BYPOSITION | dwWritePermitted, IDM_CREATE_CCDA, "&Generate");
		submenuCCDA.InsertMenu(nSubIndex++, MF_BYPOSITION | dwWritePermitted, IDM_CREATE_CCDA_CUSTOM, "Customi&ze...");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_POPUP | dwWritePermitted, (UINT_PTR)submenuCCDA.GetSafeHmenu(), "Create Summary &of Care...");	
	}
	mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
	mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_NEW_DOCUMENT, "&Create New Document");

	//DRT 3/17/03 - Per a conversation with Meikin regarding some related things, these should not
	//		be enabled if the office does not have a license for letter writing.  So change the nFlags
	//		parameter to included MF_DISABLED if they do not.
	int nFlags = MF_BYPOSITION;
	if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent))
		nFlags |= MF_DISABLED | MF_GRAYED;
	mnu.InsertMenu(nIndex++, nFlags|dwWritePermitted, IDM_MERGE_DOCUMENT, "&Merge New Document");
	mnu.InsertMenu(nIndex++, nFlags|dwWritePermitted, IDM_MERGE_PACKET, "Merge New &Packet");
	
	if(IsReproductive() && m_bInPatientsModule) {
		// (d.thompson 2014-02-20) - PLID 60924 - Removed IVF
	}

	if (IsSurgeryCenter(false) && m_bInPatientsModule) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR); 
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_NEW_CASE_HISTORY, "New Case &History");
	}

	// (a.walling 2008-07-17 15:19) - PLID 30768 - Check for the license
	if (m_bInPatientsModule && g_pLicense->HasTranscriptions()) {
		// (a.walling 2008-07-17 15:18) - PLID 30751 - Launch the transcription parser
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|dwWritePermitted, IDM_HX_PARSE_TRANSCRIPTIONS, "Parse &Transcription File...");
	}

	
	CRect rc;
	CWnd *pWnd = GetDlgItem(IDC_NEW);
	if (pWnd) {
		pWnd->GetWindowRect(&rc);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
	} else {
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}	
}

// TODO: This function needs to be analyzed and fixed so that each set of dependent statements are inside their own transaction
void CHistoryDlg::OnDetach() 
{
	long nHighlightedRows = 0;
	for (long i=0; i < m_pDocuments->GetRowCount(); i++)
	{
		IRowSettingsPtr p = m_pDocuments->GetRow(i);
		// (z.manning 2008-07-03 15:38) - PLID 25574 - Skip EMN rows
		if (p->IsHighlighted() && VarLong(p->GetValue(phtdlcEmnID), -1) == -1)
			nHighlightedRows++;
	}
	
	if (!nHighlightedRows && !(m_PhotoViewer.IsWindowVisible() && m_bPhotoViewerDetachable))
	{
		MsgBox("You must select at least one document to detach.");
		return;
	}

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	long nIndex = 0;

	if (nHighlightedRows == 1)
	{
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_DETACH_DOCUMENT, "&Detach File");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_DETACH_AND_DELETE_DOCUMENT, "Detach &And Delete File");
	}
	else
	{
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_DETACH_DOCUMENT, "&Detach Selected Files");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_DETACH_AND_DELETE_DOCUMENT, "Detach &And Delete Selected Files");
	}

	CRect rc;
	CWnd *pWnd = GetDlgItem(IDC_DETACH);
	if (pWnd) {
		pWnd->GetWindowRect(&rc);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.left, rc.bottom, this, NULL);
	} else {
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}	

}



void CHistoryDlg::DetachSelectedDocuments(BOOL bDelete)
{
	long nHighlightedRows = 0;

	// Find out how many rows are highlighted
	// (m.hancock 2006-06-30 15:23) - PLID 21307 - Added code to assemble a list of MailIDs to check for association with Lab records
	CString strMailIDs;
	for (long i=0; i < m_pDocuments->GetRowCount(); i++)
	{
		IRowSettingsPtr p = m_pDocuments->GetRow(i);
		// (z.manning 2008-07-03 15:29) - Ignore EMNs
		long nEmnID = VarLong(p->GetValue(phtdlcEmnID), -1);
		if (p->IsHighlighted() && nEmnID == -1) {
			nHighlightedRows++;
			// (m.hancock 2006-06-30 14:09) - PLID 21307 - assemble a list of MailIDs
			CString strTemp;
			strTemp.Format("%li", VarLong(m_pDocuments->GetValue(i, phtdlcMailID)));
			if(strMailIDs.IsEmpty())
				strMailIDs = strTemp;
			else
				strMailIDs += ", " + strTemp;
		}
	}

	// (m.hancock 2006-11-21 13:05) - PLID 21496 - Keep track of whether we should delete a document or not.
	BOOL bFileDeleted = FALSE;

	EnsureRemoteData();	
	
	// (e.frazier 2016-05-20 16:52) - PLID-34501 - Check delete permissions corresponding to what module the user is in
	if (CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptDelete))
	{
		// (m.hancock 2006-06-30 15:38) - PLID 21307 - Compare MailIDs against MailSent for associated Lab records and display message if necessary
		CString strPromptSingleDelete = "Are you absolutely sure you wish to detach and delete this file? This is an unrecoverable operation!";
		CString strPromptSingleDetach = "Are you absolutely sure you wish to detach this file?";
		CString strPromptMultipleDelete = "Are you absolutely sure you wish to detach and delete all selected files? This is an unrecoverable operation!";
		CString strPromptMultipleDetach = "Are you absolutely sure you wish to detach all selected files?";
		BOOL bIsLabAttached = IsLabAttachment(strMailIDs);
		if(bIsLabAttached) {
			
			strPromptSingleDelete = "This file is associated with a lab record or lab result(s).\n"
				"If you detach and delete the selected file, the association with the lab will also be lost.\n\n"
				+ strPromptSingleDelete;
			
			strPromptSingleDetach = "This file is associated with a lab record or lab result(s).\n"
				"If you detach the selected file, the association with the lab will also be lost.\n\n"
				+ strPromptSingleDetach;
			
			strPromptMultipleDelete = "At least one of the selected files is associated with a lab record or lab result(s).\n"
				"If you detach and delete the selected files, the association with the labs will also be lost.\n\n"
				+ strPromptMultipleDelete;
			
			strPromptMultipleDetach = "At least one of the selected files is associated with a lab record or lab result(s).\n"
				"If you detach the selected files, the association with the labs will also be lost.\n\n"
				+ strPromptMultipleDetach;
		}


		//DRT 1/16/03 - Confirm detachment
		if (nHighlightedRows == 1)
		{
			if (bDelete)
			{
				if(AfxMessageBox(strPromptSingleDelete, MB_YESNO) == IDNO)
					return;
			}
			else
			{
				if(AfxMessageBox(strPromptSingleDetach, MB_YESNO) == IDNO)
					return;
			}
		}
		else
		{
			if (bDelete)
			{
				if(AfxMessageBox(strPromptMultipleDelete, MB_YESNO) == IDNO)
					return;
			}
			else
			{
				if(AfxMessageBox(strPromptMultipleDetach, MB_YESNO) == IDNO)
					return;
			}
		}

		try 
		{
			CStringArray astrMessages; // Error messages
			CDWordArray adwIDs;
			CWaitCursor wc;

			// (c.haag 2004-01-28 12:38) - We have to precalc all the highlighted items first
			// because the datalist likes to highlight items automatically, so our job becomes
			// unnecessarily harder.
			for (long iCurRow=0; iCurRow < m_pDocuments->GetRowCount(); iCurRow++)
			{
				IRowSettingsPtr p = m_pDocuments->GetRow(iCurRow);
				// (z.manning 2008-07-03 15:37) - PLID 25574 - Ignore EMN rows
				if (p->IsHighlighted() && VarLong(p->GetValue(phtdlcEmnID),-1) == -1)
					adwIDs.Add(VarLong(m_pDocuments->GetValue(iCurRow, phtdlcMailID)));
			}

			for (long nElement=0; nElement < adwIDs.GetSize(); nElement++)
			{
				iCurRow = m_pDocuments->FindByColumn(phtdlcMailID, (long)adwIDs[nElement], 0, FALSE);
				if (iCurRow == -1)
				{
					ASSERT(FALSE);
					continue;
				}
				
				_variant_t tmpVar = m_pDocuments->GetValue(iCurRow,phtdlcMailID);
				if (tmpVar.vt != VT_I4)
					continue;

				//TES 2/23/2004: Don't let them detach it if it's already detached.
				_variant_t var = m_pDocuments->GetValue(iCurRow, phtdlcStaff);
				//if the staff value is filled in, then it must have been attached by that person.  If it
				//is not, then it's an unattached item
				if(var.vt == VT_NULL || var.vt == VT_EMPTY) 
					continue;

				BOOL bDeleteMultiPat = FALSE;
				CString filename = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcPath)));

				// (a.walling 2006-11-29 12:28) - PLID 23700 - Audit something useful when removing Message-based entries, etc.
				CString strNote = VarString(m_pDocuments->GetValue(iCurRow, phtdlcNotes), "");

				IRowSettingsPtr p = m_pDocuments->GetRow(iCurRow);
				if (!p->IsHighlighted())
					continue;

				CString strFullFileName;
				if(!filename.IsEmpty()) {
					if (filename.Find('\\') == -1) {
						if(filename.Left(11) == "MultiPatDoc") 
							strFullFileName = GetPatientDocumentPath(-25) ^ filename;
						else
							strFullFileName = GetCurPatientDocumentPath() ^ filename;
					}
					else
						strFullFileName = filename;
				}

				if (bDelete && filename.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE) &&
					filename.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) &&
					filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_REQUEST) &&
					filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) &&					
					m_pDocuments->GetValue(iCurRow, phtdlcName).vt != VT_I4 && !filename.IsEmpty())
				{
					// (j.jones 2011-07-15 12:55) - PLID 42111 - disallow deleting if the file is an image on an EMR,
					// which sometimes - but not always - has a \ in front of it
					// (j.jones 2013-09-20 09:59) - PLID 58547 - This was somewhat inaccurate, as if the same image name
					// was attached to multiple patients, it could errantly warn about being in use. If this is just
					// a patient-specific image (and not a contact), we should filter on that patient.
					bool bFileNameIsPerPatient = true;
					if(!m_bInPatientsModule || filename.Find('\\') != -1 || filename.Find("MultiPatDoc") != -1) {
						//the file name is a full path that is not specific to this patient
						bFileNameIsPerPatient = false;
					}

					//If the image is local to a patient documents folder, filter by patient, else look at all EMNs.
					//This code intentionally does not exclude deleted EMNs or details.
					// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading History dialog
					if((bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
						"INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
						"WHERE EMRMasterT.PatientID = {INT} "
						"AND (EMRDetailsT.InkImagePathOverride = {STRING} OR EMRDetailsT.InkImagePathOverride = '\\' + {STRING})",
						m_id, filename, filename))
						||
						(!bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
						"WHERE EMRDetailsT.InkImagePathOverride = {STRING}", filename))
						) {

						CString strWarn;
						strWarn.Format("The document '%s' is in use in an image item on at least one EMN. It cannot be deleted.",
								(LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
						MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
						continue;
					}

					// (j.jones 2011-07-15 13:44) - PLID 42111 - check to see if it is use in this EMR,
					// but perhaps not saved yet
					if(GetPicContainer()) {
						CPicContainerDlg* pDlg = GetPicContainer();
						if(pDlg->IsImageFileInUseOnEMR(filename)) {
							CString strWarn;
							strWarn.Format("The document '%s' is in use in an image item in this EMR. It cannot be deleted.",
									(LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
							MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
							continue;
						}
					}
				}

				// (j.jones 2011-07-15 14:01) - PLID 42111 - warn if the file is attached to a PIC that is not the one we are currently in
				long nPicID = -1;
				if (GetPicContainer()) {
					nPicID = GetPicID();
				}
				if(ReturnsRecordsParam("SELECT MailID FROM MailSent WHERE MailID = {INT} "
					"AND ((PicID Is Not Null AND PicID <> {INT}) OR EMNID Is Not Null)", VarLong(tmpVar), nPicID)) {
					CString strWarn;
					strWarn.Format("The document '%s' is attached to the History of at least one PIC or EMN. Are you sure you wish to detach it?",
							(LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
					if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						continue;
					}
				}

				if (bDelete && filename.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE) &&
					filename.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) &&
					filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_REQUEST) &&
					filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) &&					
					m_pDocuments->GetValue(iCurRow, phtdlcName).vt != VT_I4 && !filename.IsEmpty())
				{
					// (c.haag 2004-01-28 11:29) - First make sure nobody else is attached to this file
					// (as is the case for multi-patient documents).
					//TES 3/1/2004: A file is attached to multiple patients if a.) it has a \ in the name (meaning it's not
					//relative to the patient's folder), or b.) it has "MultiPat" in the name (which, astonishingly, is the
					//only way to identify a file as belonging to the multi-patient folder).
					if(filename.Find('\\') != -1 || filename.Find("MultiPatDoc") != -1) {
					_RecordsetPtr prs = CreateRecordset("SELECT TOP 21 Last, First, Middle FROM MailSent INNER JOIN PersonT ON PersonT.ID = MailSent.PersonID WHERE PathName = '%s' AND PersonID <> %d",
							_Q(filename), GetActivePersonID());
						if (!prs->eof)
						{
							CString str, strNames;
							long nMaxNames = 20;
							strNames.Format("The document '%s' is also attached to the following patients:\n\n",
								(LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
							while (!prs->eof && nMaxNames > 0)
							{
								str.Format("%s, %s %s\n", AdoFldString(prs, "Last"),
									 AdoFldString(prs, "First"),
									 AdoFldString(prs, "Middle"));
								strNames += str;
								nMaxNames--;
								prs->MoveNext();
							}
							if (!prs->eof && nMaxNames == 0)
							{
								strNames += "<MORE NAMES OMITTED>\n";
							}
							prs->Close();
							strNames += "\n\nIf you proceed with detaching and deleting this document, it will be detached from all the other patients, and there will be no way to determine the names of those patients after the document is deleted! Are you ABSOLUTELY SURE you wish to detach and delete this document?";
							// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
							if (IDNO == MessageBox(strNames, NULL, MB_YESNO))
								continue;
							bDeleteMultiPat = TRUE;
						}
					}

					// (m.hancock 2006-08-04 16:16) - PLID 21496 - Attempt to delete the file.  Error messages will reside in astrMessages afterwards.
					bFileDeleted = DeleteSingleFile(strFullFileName, VarString(m_pDocuments->GetValue(iCurRow, phtdlcNotes)), astrMessages);
				}

				// (m.hancock 2006-08-04 15:38) - PLID 21496 - Previously, detaching and deleting a packet was not actually
				// deleting the files.  We need to determine if this row is a packet and delete each file if it is.
				if(m_pDocuments->GetValue(iCurRow, phtdlcName).vt == VT_I4) {

					//Find the packet ID
					_RecordsetPtr rsMergedPacketID = CreateParamRecordset("SELECT MergedPacketID FROM MailSent WHERE MailID = {INT}", VarLong(tmpVar));
					if(!rsMergedPacketID->eof) {
						long nMergedPacketID = AdoFldLong(rsMergedPacketID, "MergedPacketID");

						// (m.hancock 2006-08-07 10:52) - PLID 21496 - First, determine if we're dealing with a multi-patient 
						// document.  We use the filename of the selected document to determine this.
						//TES 3/1/2004: A file is attached to multiple patients if a.) it has a \ in the name (meaning it's not
						//relative to the patient's folder), or b.) it has "MultiPat" in the name (which, astonishingly, is the
						//only way to identify a file as belonging to the multi-patient folder).
						if(filename.Find('\\') != -1 || filename.Find("MultiPatDoc") != -1) {
							// (m.hancock 2006-08-07 10:53) - This is a multi-patient document, so now we need to prompt the user
							// to determine if they wish to proceed with deleting the document.
							_RecordsetPtr prsNames = CreateRecordset("SELECT TOP 21 Last, First, Middle FROM MailSent INNER JOIN PersonT ON PersonT.ID = MailSent.PersonID WHERE PathName = '%s' AND PersonID <> %d",
								_Q(filename), GetActivePersonID());
							if (!prsNames->eof)
							{
								CString str, strNames;
								long nMaxNames = 20;
								strNames.Format("The document '%s' is also attached to the following patients:\n\n",
									(LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
								while (!prsNames->eof && nMaxNames > 0)
								{
									str.Format("%s, %s %s\n", AdoFldString(prsNames, "Last"),
										 AdoFldString(prsNames, "First"),
										 AdoFldString(prsNames, "Middle"));
									strNames += str;
									nMaxNames--;
									prsNames->MoveNext();
								}
								if (!prsNames->eof && nMaxNames == 0)
								{
									strNames += "<MORE NAMES OMITTED>\n";
								}
								prsNames->Close();

								//DRT 12/5/2006 - PLID 21496 - Cleanup message whether we're detaching or deleting.
								CString strTmp, strTmp2;
								strTmp = (bDelete ? " and deleting" : "");
								strTmp2 = (bDelete ? " and delete" : "");
								strNames += "\n\nIf you proceed with detaching" + strTmp + " this document, it will be detached from all the other patients, and there will be no way to determine the names of those patients after the document is removed! Are you ABSOLUTELY SURE you wish to detach" + strTmp2 + " this document?";
								// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
								if (IDNO == MessageBox(strNames, NULL, MB_YESNO))
									//DRT 12/5/2006 - PLID 21496 - If no, continue looping -- there may be multiple files selected, we still want to try
									//	detaching those files.
									continue;
								else {
									//DRT 12/4/2006 - PLID 21496 - Packets are an interesting creature... if we try (and fail) to delete some of the files, 
									//	but others suceed, where does that leave us?  We can't do a transaction on physical files, so we'll go with this
									//	approach:  If any file fails to delete, keep trying.  If at least 1 succeeds, the packet is no longer valid, and so 
									//	we delete the record from the database.  If all files fail, then we will skip the deletion.
									//Additionally, we are choosing to continue to ignore the link between file tracking steps and deletion.  If we fail
									//	to delete a single file, we will still unapply any tracking events on the assumption that the user intends
									//	to remove the file manually.  This continues the previous unwritten rule of behavior.  I added PLID 23761 to
									//	further consider this behavior.
									BOOL bOneSucceeded = FALSE;

									// (m.hancock 2006-08-07 12:43) - PLID 21496 - Find all of the files for this multi-patient packet
									// (j.jones 2014-08-04 17:45) - PLID 63159 - track the mail IDs and patient IDs
									CArray<long, long> aryMailIDs;
									CArray<long, long> aryPatientIDs;
									_RecordsetPtr rsFiles = CreateParamRecordset("SELECT MailID, PersonID, PathName FROM MailSent WHERE MergedPacketID = {INT}", nMergedPacketID);
									while(!rsFiles->eof) {
										CString strPathName = AdoFldString(rsFiles, "PathName");
										long nPersonID = AdoFldLong(rsFiles, "PersonID");
										aryPatientIDs.Add(nPersonID);
										long nMailID = AdoFldLong(rsFiles, "MailID");
										aryMailIDs.Add(nMailID);

										//For each file, unapply the event from tracking
										PhaseTracking::UnapplyEvent(nPersonID, PhaseTracking::ET_TemplateSent, nMailID);

										// (j.jones 2014-08-04 13:31) - PLID 63159 - removed the tablecheckers from this loop,
										// they need to wait until MailSent has actually been updated

										//Delete the file.  We need to specify the full path here and since we know this is a multi-pat packet, that is easy.
										if(bDelete) {
											CString strDocFullFileName = GetPatientDocumentPath(-25) ^ strPathName;
											bFileDeleted = DeleteSingleFile(strDocFullFileName, strDocFullFileName, astrMessages, TRUE);
											if(bFileDeleted)
												bOneSucceeded = TRUE;
										}

										rsFiles->MoveNext();
									}
									rsFiles->Close();

									//Remove the database records for this packet if either one of our deletes succeeded, or
									//	if we chose not to delete at all.
									if(bOneSucceeded || !bDelete) {

										// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
										// (j.gruber 2008-10-16 12:49) - PLID 31432 - LabResultsT
										CString strSqlBatch = BeginSqlBatch();
										long nAuditTransactionID = -1;
										if (bIsLabAttached) {
											CString strWhere;
											strWhere.Format("MailID IN (SELECT MailID FROM MailSent WHERE MergedPacketID = %li)", nMergedPacketID);
											nAuditTransactionID = HandleLabResultAudit(strWhere, GetActivePersonName(), GetActivePersonID());
										}
										// (c.haag 2009-05-06 16:15) - PLID 33789 - Reset the Units field too
										AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (SELECT MailID FROM MailSent WHERE MergedPacketID = %li)", nMergedPacketID);

										// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
										AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE MergedPacketID = %li", nMergedPacketID));
										AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MergedPacketsT WHERE ID = %li", nMergedPacketID);
										try {
											ExecuteSqlBatch(strSqlBatch);
											if (nAuditTransactionID != -1) {
												CommitAuditTransaction(nAuditTransactionID);
											}
										}NxCatchAllSilentCallThrow(
											if (nAuditTransactionID != -1) {
												RollbackAuditTransaction(nAuditTransactionID);
												nAuditTransactionID = -1;
											}
										);

										// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
										// we have to send those as well.
										ASSERT(aryPatientIDs.GetSize() == aryMailIDs.GetSize());
										for (int i = 0; i<aryMailIDs.GetSize(); i++) {
											// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
											CClient::RefreshMailSentTable(aryPatientIDs.GetAt(i), aryMailIDs.GetAt(i), GetRowPhotoStatus(iCurRow));
										}

										RefreshTable(NetUtils::TodoList, -1);
										// (r.gonet 08/25/2014) - We no longer send a labs table checker because the mailsent table checker has enough detail to it now.
										
										// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if in a PIC that has an EMR
										if(GetPicContainer() && GetPicContainer()->GetCurrentEMRGroupID() > 0) {
											//refresh the EMN to update the Word icon
											CClient::RefreshTable(NetUtils::EMRMasterT, m_id);
										}

										PhaseTracking::UnapplyEvent(m_id, PhaseTracking::ET_PacketSent, nMergedPacketID);
					
										//TES 2/20/2007 - PLID 23638 - Audit.
										AuditEvent(m_bInPatientsModule ? GetActivePersonID() : -1, GetActivePersonName(), BeginNewAuditEvent(), m_bInPatientsModule ? aeiPatientPacketDetach : aeiContactPacketDetach, m_id, strNote, "<Detached>", aepHigh, aetDeleted);
									}
									else {
										//Failed to delete!
										astrMessages.Add("No files were deleted, therefore the packet was not detached from any patient.");
									}

									//Remove the row from the interface
									m_pDocuments->RemoveRow(iCurRow);
								}
							}
						}

						// (m.hancock 2006-08-07 11:23) - PLID 21496 - Otherwise, we're dealing with a packet that is just for this patient.
						else {
							//Find the documents associated with this packet and attempt to delete them
							_RecordsetPtr prsMailID = CreateRecordset("SELECT PersonID, MailID, PathName FROM MailSent WHERE MergedPacketID = %li", nMergedPacketID);
							while (!prsMailID->eof)
							{
								long nPersonID = AdoFldLong(prsMailID, "PersonID");
								long nMailID = AdoFldLong(prsMailID, "MailID");
								CString docFilename = AdoFldString(prsMailID, "PathName");							

								//Find the file path for this MailID
								CString strDocFullFileName;
								if(!docFilename.IsEmpty()) {
									if (docFilename.Find('\\') == -1)
										strDocFullFileName = GetCurPatientDocumentPath() ^ docFilename;
									else
										strDocFullFileName = docFilename;
								}

								// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always false here
								CClient::RefreshMailSentTable(nPersonID, nMailID);

								// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if in a PIC that has an EMR
								if(GetPicContainer() && GetPicContainer()->GetCurrentEMRGroupID() > 0) {
									//refresh the EMN to update the Word icon
									CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePersonID());
								}

								// (m.hancock 2006-12-04 09:37) - PLID 21496 - Delete the document for this MailID only if we're deleting
								if(bDelete)
									bFileDeleted = DeleteSingleFile(strDocFullFileName, strDocFullFileName, astrMessages);

								// (m.hancock 2006-12-04 09:25) - PLID 21496 - Also need to unapply this packet from any tracking ladders.
								PhaseTracking::UnapplyEvent(nPersonID, PhaseTracking::ET_TemplateSent, nMailID);

								prsMailID->MoveNext();
							}
							prsMailID->Close();

							//Remove the database records for the documents tied to this packet and the packet.
							// (j.gruber 2008-10-16 12:50) - PLID 31432 - LabResultsT
							CString strSqlBatch = BeginSqlBatch();
							long nAuditTransactionID = -1;
							if (bIsLabAttached) {
								CString strWhere;
								strWhere.Format("MailID IN (SELECT MailID FROM MailSent WHERE MergedPacketID = %li)", nMergedPacketID);
								nAuditTransactionID = HandleLabResultAudit(strWhere, GetActivePersonName(), GetActivePersonID());
							}
							// (c.haag 2009-05-06 16:15) - PLID 33789 - Reset the Units field too
							AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (SELECT MailID FROM MailSent WHERE MergedPacketID = %li)", nMergedPacketID);
							// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
							AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE MergedPacketID = %li", nMergedPacketID));
							AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MergedPacketsT WHERE ID = %li", nMergedPacketID);
							try {
								ExecuteSqlBatch(strSqlBatch);
								if (nAuditTransactionID != -1) {
									CommitAuditTransaction(nAuditTransactionID);
								}
							}NxCatchAllSilentCallThrow(
								if (nAuditTransactionID != -1) {
									RollbackAuditTransaction(nAuditTransactionID);
									nAuditTransactionID = -1;
								}
							);

							// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
							// we have to send those as well.
							RefreshTable(NetUtils::TodoList, -1);			
							// (r.gonet 09/02/2014) - PLID 63221 - Don't send a labs tablechecker here. The mailsent one will cover it.
							
							// (m.hancock 2006-12-04 09:25) - PLID 21496 - Also need to unapply this packet from any tracking ladders.
							PhaseTracking::UnapplyEvent(m_id, PhaseTracking::ET_PacketSent, nMergedPacketID);

							//TES 2/20/2007 - PLID 23638 - Audit.
							AuditEvent(m_bInPatientsModule ? GetActivePersonID() : -1, GetActivePersonName(), BeginNewAuditEvent(), m_bInPatientsModule ? aeiPatientPacketDetach : aeiContactPacketDetach, m_id, strNote, "<Detached>", aepHigh, aetDeleted);

							//Remove the row from the interface
							m_pDocuments->RemoveRow(iCurRow);
						}
					}
					rsMergedPacketID->Close();
					continue;
				}

				// (a.walling 2006-11-29 10:58) - PLID 23681 - Store the selection type
				//CString strSelectionType = VarString(m_pDocuments->GetValue(iCurRow, phtdlcSelection), "");
				// (j.jones 2007-03-15 16:12) - PLID 25228 - selection type is not what we want, it is sometimes
				// used for the "message based" entries. Instead, use the path name.
				CString strPathName = VarString(m_pDocuments->GetValue(iCurRow, phtdlcFilePath), "");
				bool bMessage = false; // whether the record is a message-based entry

				if (filename.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE) == 0) {
					if(IsReproductive()) {
						if(IDNO==MsgBox(MB_ICONQUESTION|MB_YESNO, "Are you sure you wish to permanently delete the reproductive record '%s'?", (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))))) {
							continue;
						}
						else {
							_RecordsetPtr rs = CreateRecordset("SELECT InternalTblName, InternalRefID FROM MailSent WHERE MailID = %li",tmpVar.lVal);
							if(!rs->eof) {
								_variant_t varTblName = rs->Fields->Item["InternalTblName"]->Value;
								_variant_t varRefID = rs->Fields->Item["InternalRefID"]->Value;							
								if(varTblName.vt == VT_BSTR && CString(varTblName.bstrVal) == "IVFT") {
									if(varRefID.vt == VT_I4) {
										ExecuteSql("DELETE FROM CycleDayInfoT WHERE FormID = %li",varRefID.lVal);
										ExecuteSql("DELETE FROM IVFT WHERE ID = %li",varRefID.lVal);
										filename.Empty(); // empty the filename so auditing will use the note
									}
								}
								else if(varTblName.vt == VT_BSTR && CString(varTblName.bstrVal) == "PostIVFT") {
									if(varRefID.vt == VT_I4) {
										ExecuteSql("DELETE FROM PostIVFT WHERE ID = %li",varRefID.lVal);
										filename.Empty(); // empty the filename so auditing will use the note
									}
								}
								else {
									//TODO: check for all Repro. forms
								}
							}
						}
					} else {
						// They don't have a reproductive license, so they can't do anything with reproductive items in the patient history
						CString str;
						str.Format("The document '%s' can not be detached because you are not licensed for use of the Reproductive Components.", (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(iCurRow, phtdlcNotes))));
						astrMessages.Add(str);
						continue;
					}
				} else if (filename.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) == 0) {
					
					CString strCaseName = VarString(m_pDocuments->GetValue(iCurRow, phtdlcNotes), "");

					if (IsSurgeryCenter(true)) {
						// Get the case history that we're being asked to delete
						long nCaseHistoryID;
						{
							_RecordsetPtr prs = CreateRecordset("SELECT InternalRefID FROM MailSent WHERE MailID = %li", tmpVar.lVal);
							FieldsPtr flds = prs->GetFields();
							nCaseHistoryID = AdoFldLong(flds, "InternalRefID");
						}

						if(theApp.m_arypCaseHistories.GetSize() > 0) {
							long nSize = theApp.m_arypCaseHistories.GetSize();
							for (long i=0; i<nSize; i++) {
								CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
								if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
									MessageBox("Please close all open Case Histories before continuing.");
									return;
								}
							}
						}

						// Make sure it hasn't been billed
						if (!CCaseHistoryDlg::IsCaseHistoryBilled(nCaseHistoryID)) {

							// (j.jones 2009-08-06 11:10) - PLID 7397 - added check to see if the case is linked to a PIC
							CString strMessage;							

							if(CCaseHistoryDlg::IsCaseHistoryInProcInfo(nCaseHistoryID)) {
								//specify that it is linked to a PIC
								strMessage.Format("This case history is linked to a Procedure Information Center. Are you sure you wish to permanently delete the case history '%s'?", strCaseName);
							}
							else {
								//normal deletion message
								strMessage.Format("Are you sure you wish to permanently delete the case history '%s'?", strCaseName);
							}

							// Good, now let the user know that if they delete it, it will be permanent
							int nResult = MsgBox(MB_ICONQUESTION|MB_YESNO, strMessage);
							if (nResult == IDYES) {
								// Call the delete for the case history
								// (a.walling 2006-11-28 17:46) - PLID 23681 - We want to delete from mailsent!
								//	so, send TRUE into the function. Auditing should be taken care of there as well.
								CCaseHistoryDlg::DeleteCaseHistory(nCaseHistoryID, TRUE);
							} else {
								// The user changed her mind
								continue;
							}
						} else {
							//MsgBox(MB_ICONINFORMATION|MB_OK, "You may not delete the selected case history because it has already been billed.");
							CString str;
							str.Format("You may not delete the case history '%s' because it has already been billed.", strCaseName);
							astrMessages.Add(str);
							continue;
						}
					} else {
						// They don't have a surgery center license, so they can't do anything with surgery center items in the patient history
						//MsgBox(MB_ICONINFORMATION|MB_OK, 
						//	"You may not detach this case history because you are not licensed for use of the Surgery Center Components.\n\n"
						//	"If you would like to purchase a license to use these features or you feel this message is in error, please contact NexTech at 1-888-417-8464");
						CString str;
						str.Format("You may not detach the case history '%s' because you are not licensed for use of the Surgery Center Components.", strCaseName);
						astrMessages.Add(str);
						return;
					}
				} else if (strPathName == "") {
					// (a.walling 2006-11-29 10:59) - PLID 23681 - No selection type? this only happens for message-based mailsent entries,
					//		such as Patient Statement Sent, E-Statement Exported, HCFA Sent to Clearinghouse, etc.
					//		This happens for other situations too, such as IVF/Reproductive, but that is taken care of in the above logic.
					// (j.jones 2007-03-15 16:12) - PLID 25228 - selection type is not what we want, it is sometimes
					// used for the "message based" entries. Instead, use the path name.
					bMessage = true;

					// (j.gruber 2009-10-28 16:17) - PLID 36045 - my hunch is that somehow a race condition is making the pathname be blank and thus clearing the pathname, 
					//so let's log if we get here with a multi-pat doc
					if (bDeleteMultiPat) {
						Log("PathName is blank for a multipat doc");
						//we dont' need to error out here because we are doing that down below
					}
					filename.Empty(); // empty the filename so auditing will use the note (should already be empty anyway);
				}

				// (m.hancock 2006-11-22 10:12) - PLID 21496 - This code handles detaching; deleted should have already happened
				// and the status of the delete should be in bFileDeleted.  We need to execute the detaching code if we deleted 
				// the file or if we are only detaching (which is indicated by bDelete being FALSE).
				// (a.walling 2006-11-29 11:02) - PLID 23681 - Detach also if it is message based (this would happen anyway if bDelete wasn't specified).
				if(bFileDeleted || !bDelete || bMessage) { 
					//Stop tracking this letter.
					PhaseTracking::UnapplyEvent(m_id, PhaseTracking::ET_TemplateSent, VarLong(tmpVar));

					ExecuteSql("UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PatPrimaryHistImage = %d", VarLong(tmpVar));
					m_PhotoViewer.SetPrimaryImage(-1);
					//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
					// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
					// this shared function will handle sending accordingly
					// (e.frazier 2016-06-24 10:28) - NX-100827 - We need to make sure we are in the Patients Module before calling this function

					if (m_bInPatientsModule)
					{
						SendPatientPrimaryPhotoHL7Message(m_id);
					}
					
					// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
					// (j.gruber 2008-10-16 12:50) - PLID 31432 - LabResultsT
					CString strSqlBatch = BeginSqlBatch();
					long nAuditTransactionID = -1;
					if (bIsLabAttached) {
						CString strWhere;
						strWhere.Format("MailID = %li", VarLong(tmpVar));
						nAuditTransactionID = HandleLabResultAudit(strWhere, GetActivePersonName(), GetActivePersonID());
					}
					// (c.haag 2009-05-06 16:15) - PLID 33789 - Reset the Units field too
					AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID = %li", VarLong(tmpVar));
					// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
					AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("%li", VarLong(tmpVar)));
					try {
						ExecuteSqlBatch(strSqlBatch);
						if (nAuditTransactionID != -1) {
							CommitAuditTransaction(nAuditTransactionID);
						}
					}NxCatchAllSilentCallThrow(
						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
							nAuditTransactionID = -1;
						}
					);
					
					// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
					CClient::RefreshMailSentTable(m_id, VarLong(tmpVar), GetRowPhotoStatus(iCurRow));

					// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
					// we have to send those as well.
					if (CanCreateTodoForDocument(iCurRow)) {
						RefreshTable(NetUtils::TodoList, -1);
					}
					// (r.gonet 09/02/2014) - PLID 63221 - Don't send a labs table checker here. The mailsent one will cover it.
					
					// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if in a PIC that has an EMR
					BOOL bSendEMRChecker = FALSE;
					if(GetPicContainer() && GetPicContainer()->GetCurrentEMRGroupID() > 0) {
						//refresh in just a moment
						bSendEMRChecker = TRUE;						
					}

					if (bDeleteMultiPat)
					{ 
						// (j.gruber 2009-10-28 16:15) - PLID 36045 - make sure we have a filename
						if (filename.IsEmpty()) {
							//this is an error condition, it should not be able to be blank if its a multipatdoc because it wasn't blank at the beginning of this function
							ThrowNxException("An error occurred while detaching the file from other patients, please attempt the process again.");
						}

						//TES 9/28/2015 - PLID 66192 - Track which patients are having their primary image modified, so we can update them in HL7
						CArray<long, long> arPatientsToUpdate;
						_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, MailSent.PersonID, CASE WHEN PatPrimaryHistImage Is Not Null THEN 1 ELSE 0 END AS IsPrimaryImage "
							"FROM MailSent LEFT JOIN PatientsT ON MailSent.MailID = PatientsT.PatPrimaryHistImage WHERE PathName = '%s'", _Q(filename));

						while(!rsMailSent->eof) {
							PhaseTracking::UnapplyEvent(AdoFldLong(rsMailSent, "PersonID"), PhaseTracking::ET_TemplateSent, AdoFldLong(rsMailSent, "MailID"));
							if (AsBool(rsMailSent->Fields->GetItem("IsPrimaryImage")->Value)) {
								arPatientsToUpdate.Add(AdoFldLong(rsMailSent, "PersonID"));
							}
							rsMailSent->MoveNext();
						}

						// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
						// (j.gruber 2008-10-16 12:51) - PLID 31432 - LabResultsT
						CString strSqlBatch = BeginSqlBatch();
						long nAuditTransactionID = -1;
						if (bIsLabAttached) {
							CString strWhere;
							strWhere.Format("MailID IN (SELECT MailID FROM MailSent WHERE PathName = '%s')", _Q(filename));
							nAuditTransactionID = HandleLabResultAudit(strWhere, GetActivePersonName(), GetActivePersonID());
						}

						// (j.jones 2013-09-19 12:07) - PLID 58547 - audit for every entry we are about to delete
						_RecordsetPtr rsMultiPat = CreateParamRecordset("SELECT MailSent.PersonID, "
							"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
							"MailSentNotesT.Note, PatientsT.UserDefinedID "
							"FROM MailSent "
							"INNER JOIN PersonT ON MailSent.PersonID = PersonT.ID "
							"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
							"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
							"WHERE MailSent.PathName = {STRING}", filename);
						while(!rsMultiPat->eof) {

							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}

							CString strMultiPersonName = VarString(rsMultiPat->Fields->Item["FullName"]->Value, "");
							long nMultiPersonID = VarLong(rsMultiPat->Fields->Item["PersonID"]->Value, -1);
							long nUserDefinedID = VarLong(rsMultiPat->Fields->Item["UserDefinedID"]->Value, -1);
							CString strMultiPersonNote = VarString(rsMultiPat->Fields->Item["Note"]->Value, "");

							//audit the note if the filename is empty
							CString strAudit;
							if (filename.IsEmpty()) {
								strAudit = strMultiPersonNote;
							} else {
								strAudit = filename;
							}

							//the new value is the same regardless of whether it is detached, or also deleted
							AuditEvent(nMultiPersonID, strMultiPersonName, nAuditTransactionID, nUserDefinedID != -1 ? aeiPatientDocDetach : aeiContactDocDetach, nMultiPersonID, strAudit, "<Detached>", aepHigh, aetDeleted);

							rsMultiPat->MoveNext();
						}
						rsMultiPat->Close();

						AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PatPrimaryHistImage IN (SELECT MailID FROM MailSent WHERE PathName = '%s')", _Q(filename));
						// (c.haag 2009-05-06 16:15) - PLID 33789 - Reset the Units field too
						AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (SELECT MailID FROM MailSent WHERE PathName = '%s')", _Q(filename));
						// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
						AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE PathName = '%s'", _Q(filename)));
						try {
							ExecuteSqlBatch(strSqlBatch);
							if (nAuditTransactionID != -1) {
								CommitAuditTransaction(nAuditTransactionID);
							}
						}NxCatchAllSilentCallThrow(
							if (nAuditTransactionID != -1) {
								RollbackAuditTransaction(nAuditTransactionID);
								nAuditTransactionID = -1;
							}
						);
						
						// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
						CClient::RefreshMailSentTable(m_id, VarLong(tmpVar), GetRowPhotoStatus(iCurRow));

						// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
						// we have to send those as well.
						RefreshTable(NetUtils::TodoList, -1);
						// (r.gonet 08/25/2014) - PLID 63221 - We no longer send a labs table checker because the mailsent table checker has enough detail to it now.

						// (j.jones 2007-09-13 09:04) - PLID 27371 - refresh all EMRs
						CClient::RefreshTable(NetUtils::EMRMasterT, -1);

						//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
						foreach(long nPatID, arPatientsToUpdate) {
							// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
							// this shared function will handle sending accordingly
							SendPatientPrimaryPhotoHL7Message(nPatID);
						}
					}
					else {
						// (j.jones 2007-09-13 09:04) - PLID 27371 - refresh just this EMR
						CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePersonID());
					}

					// (c.haag 2004-03-22 11:56) - Refreshing the view in the middle of this iteration causes problems.
					// For example, iCurRow would no longer be valid. The view is refreshed en total after this function
					// is called. One other note: We suppress the reception of MailSent table checkers inside this function
					// (but the controlling logic is actually in the callers).
					// m.carlson 2/11/2004 PL 10039
					/*if (strFullFileName.Find(GetCurPatientDocumentPath()) != 0 || !IsDlgButtonChecked(IDC_ENABLE_UNATTACHED))
						m_pDocuments->RemoveRow(iCurRow);
					else if (IsDlgButtonChecked(IDC_ENABLE_UNATTACHED)) // Refresh, or we'll still be showing the
						UpdateView();						// attach date and such, and the sort order will be wrong
					*/

					//auditing
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();

					// (a.walling 2006-11-29 12:32) - PLID 23700 - Audit the note if the filename is empty
					CString strAudit;
					if (filename.IsEmpty()) {
						strAudit = strNote;
					} else {
						strAudit = filename;
					}

					// (a.walling 2006-08-14 17:39) - PLID 21970 - correctly audit for the contacts module
					if(nAuditID != -1)
						AuditEvent(m_bInPatientsModule ? GetActivePersonID() : -1, GetActivePersonName(), nAuditID, m_bInPatientsModule ? aeiPatientDocDetach : aeiContactDocDetach, m_id, strAudit, "<Detached>", aepHigh, aetDeleted);
				}

			} // for (long nElement=0; nElement < adwIDs.GetSize(); nElement++)

			EnableAppropriateButtons();

			if (astrMessages.GetSize() > 0)
			{
				CString str = "The operation completed with the following messages:\n\n";
				for (long i=0; i < astrMessages.GetSize(); i++)
					str += astrMessages[i] + "\n";
				// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
				MessageBox(str, NULL, MB_ICONINFORMATION);
			}
		}
		NxCatchAll("Error in CHistoryDlg::OnDetach");
	}
}

void CHistoryDlg::OnDetachDocument()
{
	// (e.lally 2009-06-11) PLID 34600 - Added try/catch
	try{
		if(m_PhotoViewer.IsWindowVisible()) {
			//We'll let the photo control handle it.
			m_PhotoViewer.SendMessage(NXM_DETACH);
		}
		else {
			DetachSelectedDocuments(FALSE);
		}
		UpdateView(); // Update our view in case we missed any table checkers that we didn't send ourselves
	}NxCatchAll(__FUNCTION__);
}

void CHistoryDlg::OnDetachAndDelete()
{
	// (e.lally 2009-06-11) PLID 34600 - Added try/catch
	try {
		if(m_PhotoViewer.IsWindowVisible()) {
			//We'll let the photo control handle it.
			m_PhotoViewer.SendMessage(NXM_DETACH_AND_DELETE);
		}
		else {
			DetachSelectedDocuments(TRUE);
		}
		UpdateView(); // Update our view in case we missed any table checkers that we didn't send ourselves
	}NxCatchAll(__FUNCTION__);
}

void CHistoryDlg::OnClickIUI()
{
	try {

	} NxCatchAll("Error creating IUI form.");
}

void CHistoryDlg::OnClickMedEval()
{
	try {

	} NxCatchAll("Error creating Medical Evaluation form.");
}

void CHistoryDlg::OpenDocumentByID(long nMailID)
{
	//Asynchronously set the selection to this row.
	m_pDocuments->TrySetSelByColumn(phtdlcMailID, nMailID);
	EnableAppropriateButtons();

	//While that's going, let's try and open it.

	_RecordsetPtr rsMailSent = CreateRecordset("SELECT Selection, PathName, PersonID FROM MailSent WHERE MailID = %li", nMailID);
	if(rsMailSent->eof || AdoFldLong(rsMailSent, "PersonID") != m_id) {
		MessageBox("Document not found.");
		return;
	}

	CString name = AdoFldString(rsMailSent, "Selection");
	CString filename = AdoFldString(rsMailSent, "PathName");

	if (strnicmp(name, SELECTION_WORDDOC, 13) == 0 || strnicmp(name, SELECTION_FILE, 11) == 0 ||
		strnicmp(name, SELECTION_FOLDER, 13) == 0 || strnicmp(name, SELECTION_AUDIO, 12) == 0)
	{
		if(!OpenDocument(filename, m_id)) {
			MsgBox(RCS(IDS_NO_FILE_OPEN));
		}
		return;
	}
}

void CHistoryDlg::OpenPacket(long nMergedPacketID)
{
	try {
		//While that's going, let's try and open it.
		_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, Selection, PathName, PersonID FROM MailSent WHERE MergedPacketID = %li AND PersonID = %li ORDER BY MailID ASC", nMergedPacketID, m_id);
		if(rsMailSent->eof) {
			MsgBox("Packet not found.");
			return;
		}
		//Asynchronously set the selection to this row.
		m_pDocuments->TrySetSelByColumn(phtdlcMailID, AdoFldLong(rsMailSent, "MailID"));
		EnableAppropriateButtons();

		while(!rsMailSent->eof) {
			CString strTemplate = AdoFldString(rsMailSent, "PathName");
			if(!OpenDocument(strTemplate, m_id)) {
				MsgBox("Could not open template %s", strTemplate);
			}
			rsMailSent->MoveNext();
		}
	}NxCatchAll("Error in CHistoryDlg::OpenPacket()");
}

void CHistoryDlg::OnNewCaseHistory()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// Have the user choose the preference card upon which this case history is to be based
		CString strCaseHistoryName;
		long nProviderID; //we used to call CalcDefaultCaseHistoryProvider, but is now in the ChooseSurgery
		// (c.haag 2007-03-09 11:12) - PLID 25138 - Pass in the patient ID as well
		// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
		// (j.jones 2009-08-31 15:07) - PLID 35378 - we now allow multiple preference cards to be chosen
		CArray<long, long> arynPreferenceCardIDs;
		if(CCaseHistoryDlg::ChoosePreferenceCards(arynPreferenceCardIDs, strCaseHistoryName, nProviderID, m_id)) {

			// (j.jones 2006-12-18 15:08) - PLID 23578 - I changed the code so we do not create a case
			// in data first, we just open it on the screen and only save then they close it, like
			// any other sensible dialog

			// open a new the case history for this patient based on the given surgery
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());

			CCaseHistoryDlg dlg(this);
			// (j.jones 2009-08-07 08:34) - PLID 7397 - the case can now return more than just ok/cancel
			// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
			if(dlg.OpenNewCase(m_id, arynPreferenceCardIDs, strCaseHistoryName, nProviderID, dtToday) != IDCANCEL) {
				// Reflect the new case history in this patient's document history
				// TODO: The Case History will eventually use NxServer messages, when it does, this call won't be necessary.
				UpdateView();
			}
		}

	} NxCatchAll("CHistoryDlg::OnNewCaseHistory");

}

// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
void WINAPI CALLBACK CHistoryDlg::OnNxTwainPreCompress(const LPBITMAPINFO pDib, /* The image that was scanned */
		BOOL& bAttach,
		BOOL& bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon)
{
	nChecksum = BitmapChecksum(pDib);
	//See if this image has been attached already, by generating a checksum, and checking for its existence.
	// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	if(ReturnsRecordsParam(lpCon, "SELECT MailID FROM MailSent WHERE Checksum = {INT}", nChecksum)) {
		bAttach = FALSE;
	}
	else {
		bAttach = TRUE;
		bAttachChecksum = TRUE;
	}

}

void CHistoryDlg::OnAcquire()
{
	try {
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before starting a new one.");
			return;
		}

		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// (a.walling 2008-09-03 11:36) - PLID 19638 - Send the PicID to the scan engine
		long nPicID = -1;
		if (GetPicContainer()) {
			nPicID = GetPicID();
		}
		// (a.walling 2009-12-11 13:24) - PLID 36518 - use eScanToImage
		// (j.gruber 2010-01-22 13:44) - PLID 23693 - it'll always be an image here
		NXTWAINlib::Acquire(m_id, GetCurPatientDocumentPath(), NULL, OnNxTwainPreCompress, NULL, "", CheckAssignCategoryID(TRUE), -1, nPicID, NXTWAINlib::eScanToImage);
	} NxCatchAll("CHistoryDlg::OnAcquire");
}

// (a.walling 2008-09-05 12:58) - PLID 31282 - Acquire as a single-page PDF
void CHistoryDlg::OnAcquirePDF()
{
	try {
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before starting a new one.");
			return;
		}

		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// (a.walling 2008-09-03 11:36) - PLID 19638 - Send the PicID to the scan engine
		long nPicID = -1;
		if (GetPicContainer()) {
			nPicID = GetPicID();
		}
		// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file, it'll always be false here
		NXTWAINlib::Acquire(m_id, GetCurPatientDocumentPath(), NULL, OnNxTwainPreCompress, NULL, "", CheckAssignCategoryID(FALSE), -1, nPicID, NXTWAINlib::eScanToPDF);
	} NxCatchAll("CHistoryDlg::OnAcquirePDF");
}

// (a.walling 2008-09-05 12:58) - PLID 31282 - Acquire as a multi-page PDF
void CHistoryDlg::OnAcquireMultiPDF()
{
	try {
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before starting a new one.");
			return;
		}

		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// (a.walling 2008-09-03 11:36) - PLID 19638 - Send the PicID to the scan engine
		long nPicID = -1;
		if (GetPicContainer()) {
			nPicID = GetPicID();
		}
		// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file, it'll always by false here
		NXTWAINlib::Acquire(m_id, GetCurPatientDocumentPath(), NULL, OnNxTwainPreCompress, NULL, "", CheckAssignCategoryID(FALSE), -1, nPicID, NXTWAINlib::eScanToMultiPDF);
	} NxCatchAll("CHistoryDlg::OnAcquireMultiPDF");
}

// (a.walling 2008-09-08 09:42) - PLID 31282 - Invoke the scan multi doc dialog
void CHistoryDlg::OnAcquireMultiDoc()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		CScanMultiDocDlg dlg(this);
		// (r.galicki 2008-09-05 11:52) - PLID 31242 - Added support to load current patient in the patient list
		dlg.m_nPatientID = GetActivePersonID();
		dlg.m_nPIC = GetPicID(); //PIC ID too
		dlg.DoModal();
	} NxCatchAll("CHistoryDlg::OnAcquireMultiDoc");
}

// (a.walling 2008-09-09 13:32) - PLID 30389 - Capture items from WIA
void CHistoryDlg::OnAcquireWIA()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		// (a.walling 2008-09-18 14:00) - PLID 30389 - Use the preference for whether to show cameras only or not
		WIA::IDevicePtr pDevice = NxWIA::GetDefaultDevice(GetRemotePropertyInt("WIA_CameraDevicesOnly", TRUE, 0, GetCurrentUserName(), true));
		
		if (pDevice) {
			AcquireFromWIA(pDevice);
		}
	} NxCatchAll("CHistoryDlg::OnAcquireWIA");
}

// (a.walling 2008-09-10 15:57) - PLID 31334
void CHistoryDlg::AcquireFromWIA(WIA::IDevicePtr pDevice)
{
	WIA::IItemsPtr pItems = NxWIA::GetSelectedItems(pDevice);
	if (pItems) {

		CStringArray saTempFiles;

		WIA::ICommonDialogPtr pCommonDialog = NxWIA::GetCommonDialog();

		int i = 0;
		for (i = 1; i <= pItems->Count; i++) {
			WIA::IItemPtr pItem = pItems->Item[i];

			if (pItem) {
				AcquireItemFromWIA(saTempFiles, pItem, pCommonDialog);
			}
		}
		
		AttachTempFilesToMailSent(saTempFiles);
	}
}

void CHistoryDlg::AcquireItemFromWIA(CStringArray& saTempFiles, WIA::IItemPtr pItem, WIA::ICommonDialogPtr pCommonDialog)
{
	WIA::IImageFilePtr pImage;
	// (a.walling 2008-09-18 13:50) - PLID 30389 - wiaFormatJPEG is just a suggestion. If the device does not support that,
	// then we will get the device's preferred file format. This also implies that we now support acquiring videos and,
	// well, anything else that the WIA device supports, such as holograms, memory records, genotypes, etc...
	if (pCommonDialog) {
		pImage = pCommonDialog->ShowTransfer(pItem, WIA::wiaFormatJPEG, VARIANT_FALSE);
	} else {
		pImage = pItem->Transfer(WIA::wiaFormatJPEG);
	}

	if (pImage) {
		CString strFileName = GetNxTempPath() ^ FormatString("ScannedWIATempFile%lu.%s", GetUniqueSessionNum(), (LPCTSTR)pImage->GetFileExtension());
		
		pImage->SaveFile(_bstr_t(strFileName));
		saTempFiles.Add(strFileName);
	}
}

void CHistoryDlg::AttachTempFilesToMailSent(CStringArray& saTempFiles)
{
	long nPicID = -1;
	if (GetPicContainer()) {
		nPicID = GetPicID();
	}

	CString strErrors;
	for (int i = 0; i < saTempFiles.GetSize(); i++) {
		CString strFileName = saTempFiles[i];
		CString strExt = FileUtils::GetFileExtension(saTempFiles[i]);

		
		if (GetRemotePropertyInt("TWAINScanToDocumentFolder",1,0,"<None>",TRUE)) {
			CString strTargetName = GetCurPatientDocumentPath() ^ GetPatientDocumentName(GetActivePersonID(), strExt);
			if (CopyFile(strFileName, strTargetName, TRUE)) {
				// (a.walling 2008-10-27 15:05) - PLID 30389 - This will use the filename rather than the entire path as the note
				// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file
				AttachFileToHistory(strTargetName, GetActivePatientID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strTargetName)), SELECTION_FILE);
			} else {
				strErrors += FormatLastError("Moving %s to %s: \r\n", strFileName, strTargetName);
			}
		}

		if (GetRemotePropertyInt("TWAINScanToRemoteFolder",0,0,"<None>",TRUE)) {
			// (a.walling 2010-01-28 14:15) - PLID 28806 - Requires a connection pointer now
			CString strUserDefinedFilename = NXTWAINlib::GetUserDefinedOutputFilename(GetRemoteData(), GetActivePersonID(), strExt);
			if (CopyFile(strFileName, strUserDefinedFilename, FALSE)) {								
				// (a.walling 2008-10-27 15:05) - PLID 30389 - This will use the filename rather than the entire path as the note
				// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file
				AttachFileToHistory(strUserDefinedFilename, GetActivePatientID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strUserDefinedFilename)), SELECTION_FILE);
			} else {
				strErrors += FormatLastError("Moving %s to %s: \r\n", strFileName, strUserDefinedFilename);
			}
		}
	}

	if (!strErrors.IsEmpty()) {
		MessageBox(FormatString("Errors occurred while attaching to history:\r\n\r\n%s", strErrors), NULL, MB_OK|MB_ICONEXCLAMATION);
	}
}

void CHistoryDlg::OnCurrentFolder() 
{
	try {
		//TES 8/4/2011 - PLID 44857 - Check their permissions
		// (e.frazier 2016-05-19 10:31) - No permission check if we are in the Contacts module
		if (m_bInPatientsModule)
		{
			if (!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptDynamic0)) {
				return;
			}
		}
		//open up this patients folder in windows explorer
		//if the folder doesn't exist, we should create it - apparently windows does this automatically

		//path that we want to open
		CString strPath = GetCurPatientDocumentPath();

		if(!DoesExist(strPath)) {
			//doesn't exist? try clearing out the cached value and trying again
			m_strCurPatientDocumentPath = "";
			strPath = GetCurPatientDocumentPath();

			//if it still doesn't exist, leave
			if(!DoesExist(strPath)) {
				// This shouldn't be possible because either GetPatientDocumentPath should return a 
				// valid path to a folder that exists, or it should throw an exception
				ASSERT(FALSE);
				AfxThrowNxException("Person document folder '%s' could not be found", strPath);
				return;
			}
		}

		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		ShellExecute(GetSafeHwnd(), NULL, strPath, NULL, NULL, SW_SHOWNORMAL);
	} NxCatchAll("Error in OnCurrentFolder()");
}

void CHistoryDlg::LoadUnattachedFiles() {

	try {

		//DRT - 7/19/02 - We're now going to show all documents in the default folder and in attached folders that are not currently showing in this tab
		CString str;

		//we need to find all the files in the default folder
		CString defFolder = GetCurPatientDocumentPath();

		//we're going to trick the recordset into thinking the default folder is in the MailSent table.  This also works even
		//if it is currently in the MailSent table because of the way unions work

		// (j.jones 2014-08-15 09:29) - PLID 63374 - this now takes in a set of files, will be filled only once
		boost::container::flat_set<CiString> aryPatFiles;
		bool bHasLoadedPatFiles = false;

		// (j.jones 2014-08-12 16:16) - PLID 32474 - parameterized this
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, PathName FROM MailSent "
			"WHERE PersonID = {INT} AND Selection = 'BITMAP:FOLDER' "
			"UNION SELECT {INT}, {STRING}", m_id, m_id, defFolder);
		while(!rs->eof) {

			// (z.manning 2008-11-12 12:32) - PLID 31180 - Pass in the tick count so that we can timeout
			// if this directory takes too long to load all of its files.
			CString strFolder = VarString(rs->Fields->Item["PathName"]->Value, "");
			if (!strFolder.IsEmpty()) {
				LoadFilesFromDir(strFolder, FALSE, GetTickCount(), aryPatFiles, bHasLoadedPatFiles);
			}

			rs->MoveNext();
		}//end rs while
		rs->Close();

		// (j.jones 2014-08-15 09:29) - PLID 63374 - if this was filled, clear it now
		if (bHasLoadedPatFiles) {
			aryPatFiles.clear();
		}

	} NxCatchAll("Error in LoadUnattachedFiles()");
}

// (z.manning 2008-11-12 12:06) - PLID 31180 - Added the start tick parameter so we can timeout here
// in case someone attaches a directory with a massive amount of files (e.g. an entire drive).
// (j.jones 2014-08-14 17:22) - PLID 63374 - this now loads the patient's file list one time and
// passes it into recursive calls
void CHistoryDlg::LoadFilesFromDir(CString strDir, const BOOL bIsSubDirectory, const DWORD dwStartTick, boost::container::flat_set<CiString> &aryPatFiles, bool &bHasLoadedPatFiles)
{
	CString sql;	
	CFileFind finder;

	strDir = strDir ^ "\\*.*";

	int nContinue = finder.FindFile(strDir);

	// (z.manning 2008-11-12 12:10) - PLID 31180 - I'm setting a timeout for this function at 15 seconds.
	// If the program hasn't finished loading all the files from a directory by then I'm declaring that
	// directory to have an unreasonable amount of files.
	const DWORD dwTimeoutMs = 15000;

	while(nContinue) {

		nContinue = finder.FindNextFile();
		CString strFile = finder.GetFileName();
		CString strPathFile = finder.GetFilePath();

		//DRT 5/28/03 - Only showing files that are not hidden and not temp files.  Also hid system files after consulting with Bob.
		//DRT 5/18/2006 - PLID 16736 - I also filtered out the IsDots files, there's no reason to query for them, they are not real files.
		if (!finder.IsTemporary() && !finder.IsHidden() && !finder.IsSystem() && !finder.IsDots() && strFile != "." && strFile != "..") {
			
			//see if this file exists, as either just the filename, or as the whole path
			
			// (j.jones 2014-08-14 17:30) - PLID 63374 - we now load this one time only, on demand,
			// to cache all the patient's files and compare to that, as opposed to one recordset per file
			if (!bHasLoadedPatFiles) {

				ASSERT(aryPatFiles.size() == 0);

				//now load all unique path names
				_RecordsetPtr rsFiles = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PathName FROM MailSent "
					"WHERE PersonID = {INT} AND PathName <> '' "
					"GROUP BY PathName", m_id);
				long nCount = rsFiles->GetRecordCount();
				if (nCount > 0) {
					//reserve the right amount of records
					aryPatFiles.reserve(nCount);
				}
				while (!rsFiles->eof) {
					aryPatFiles.insert((CiString)VarString(rsFiles->Fields->Item["PathName"]->Value, ""));
					rsFiles->MoveNext();
				}
				rsFiles->Close();

				bHasLoadedPatFiles = true;
			}

			bool bFound = false;

			//CiString is case insensitive
			CiString strFile_Insensitive = strFile;
			CiString strPathFile_Insensitive = strPathFile;

			//if either lookup (case-insentive) returns a value, this means the file
			//is already in the patient's MailSent history, so we will not add it
			if (aryPatFiles.count(strFile_Insensitive) > 0 || aryPatFiles.count(strPathFile_Insensitive) > 0) {
				bFound = true;
			}

			if(!bFound) {
			
				//the file doesnt exist in the patient's history, so add the row

				IRowSettingsPtr pRow;
				pRow = m_pDocuments->GetRow(-1);
				_variant_t var;
				var.vt = VT_I4;
				var.lVal = -1;
				pRow->PutValue(phtdlcMailID, var);
				pRow->PutValue(phtdlcFilePath, _bstr_t(strPathFile));	//the full path is loaded into the Path column, but is later replaced manually
				_variant_t varNull;
				varNull.vt = VT_NULL;
				pRow->PutValue(phtdlcFilename, varNull);
				pRow->Value[phtdlcExtraNotesIcon] = g_cvarNull; // (c.haag 2010-07-01 16:08) - PLID 39473
				pRow->Value[phtdlcHasExtraNotes] = g_cvarFalse; // (c.haag 2010-07-01 16:08) - PLID 39473

				// (a.walling 2008-09-15 16:51) - PLID 23904 - Mirror these settings with the icon column
				if(finder.IsDirectory()) {
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:FOLDER"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:FOLDER"));
				} else if (finder.GetFileName().Right(4).CompareNoCase(".doc") == 0) {
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:MSWORD"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:MSWORD"));
				// (a.walling 2007-07-19 11:19) - PLID 26748 - Support .docx files as well, and fix for non-lowercase extensions
				} else if (finder.GetFileName().Right(5).CompareNoCase(".docx") == 0) {
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:MSWORD"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:MSWORD"));
				// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
				} else if (finder.GetFileName().Right(5).CompareNoCase(".docm") == 0) {
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:MSWORD"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:MSWORD"));
				} else if (IsWaveFile(finder.GetFileName()))  { // (c.haag 2006-10-09 09:32) - PLID 22679 - We now display wave icons
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:AUDIO"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:AUDIO"));
				} else {
					pRow->PutValue(phtdlcName, _bstr_t("BITMAP:FILE"));
					pRow->PutValue(phtdlcIcon, _bstr_t("BITMAP:FILE"));
				} 

				pRow->PutValue(phtdlcNotes, _bstr_t(strPathFile));

				//null the rest
				var.vt = VT_NULL;
				pRow->PutValue(phtdlcStaff, var);	//sender
				pRow->PutValue(phtdlcDate, var);	//date
				pRow->PutValue(phtdlcEmnID, var);	//EMN ID
				pRow->PutValue(phtdlcPicID, var); // (z.manning 2010-02-02 11:42) - PLID 34287 - PicID
				pRow->PutValue(phtdlcEmnModifiedDate, var); // (z.manning 2012-09-10 14:29) - PLID 52543

				//insert into the datalist
				m_pDocuments->AddRow(pRow);

				if(finder.IsDirectory()) {
					//if it's a directory, load all the subdirs

					// (j.jones 2014-08-15 09:29) - PLID 63374 - pass in our file array
					LoadFilesFromDir(strPathFile, TRUE, dwStartTick, aryPatFiles, bHasLoadedPatFiles);
				}
			}
		}

		// (z.manning 2008-11-12 12:18) - PLID 31180 - Check and see if we've timed out loading this directory.
		if(GetTickCount() - dwStartTick > dwTimeoutMs) {
			if(!bIsSubDirectory) {
				MessageBox("The folder '" + strDir + "' has timed out trying load all of its files.", NULL, MB_OK|MB_ICONERROR);
			}
			break;
		}
	}//end finder while
}

// (a.walling 2008-09-15 15:59) - PLID 23904
void CHistoryDlg::FillPathAndFile(NXDATALISTLib::IRowSettingsPtr pRow) 
{
	try {

		//for(int i = 0; i < m_pDocuments->GetRowCount(); i++) {

			// (z.manning 2008-07-01 14:59) - PLID 25574 - Skip EMN rows
			if(VarLong(pRow->GetValue(phtdlcEmnID), -1) != -1) {
				return;
			}

			CString path, file, str;
			//it automatically loads the path of the file into the datalist, we just need to parse that out and set these 2 fields up correctly
			str = CString(pRow->GetValue(phtdlcFilePath).bstrVal);

			if(str != "") {
				//TES 1/20/2005 - If the filename has a value, that means that something else (probably a tablechecker message),
				//has already snuck in and parsed out this row.  So don't reparse.
				CString strCurrentFilename = VarString(pRow->GetValue(phtdlcFilename),"");
				if(strCurrentFilename == "") {

					int loc = str.ReverseFind('\\');
					//files w/o a full path
					if(loc == -1) {		//no backslash, must just be a file
						file = str;
						path = GetCurPatientDocumentPath();
					}
					//directories
					else if(pRow->GetValue(phtdlcName).vt == VT_BSTR && VarString(pRow->GetValue(phtdlcName),"") == "BITMAP:FOLDER") {
						file = "";
						path = str;
					}
					//everything else
					else {

						file = str.Right(str.GetLength() - loc - 1);
						path = str.Left(loc);
					}

					pRow->PutValue(phtdlcFilePath, _bstr_t(path));
					pRow->PutValue(phtdlcFilename, _bstr_t(file));		
				}
			}
			else {
				//This is a printed statement, or something else without a real "path", so...
				pRow->PutValue(phtdlcFilePath, _bstr_t(""));
				pRow->PutValue(phtdlcFilename, _bstr_t(""));
			}
		//}

	}NxCatchAllCallIgnore({LogDetail("Error in FillPathAndFile");});
}

void CHistoryDlg::OnEnableUnattached() 
{
	//save the checkbox in ConfigRT
	SetRemotePropertyInt("EnableUnattachedDocuments", IsDlgButtonChecked(IDC_ENABLE_UNATTACHED) ? 1 : 0, 0, GetCurrentUserName());
	
	UpdateView();
}

void CHistoryDlg::OnClickMergePacket()
{
  try {
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}
	// (e.lally 2009-06-11) PLID 34600 - Check write permissions
	// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
	if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
		return;
	}

	// Get packet to merge to
	CSelectPacketDlg dlg(this);
	int nReturn = dlg.DoModal();
	if(nReturn == IDOK) {
		// (e.lally 2009-06-11) PLID 34600 - Moved to the top
		//try {
		
			//Let's start mergin!
			CStringArray arystrPaths;

			// (a.walling) 5/1/06 PLID 18343 
			long nPacketCategoryID = CMergeEngine::GetPacketCategory(dlg.m_nPacketID);

			// Loop through each template, adding the full path of each one to the array
			_RecordsetPtr rsTemplates = CreateRecordset("SELECT Path FROM MergeTemplatesT INNER JOIN PacketComponentsT ON MergeTemplatesT.ID = PacketComponentsT.MergeTemplateID WHERE PacketComponentsT.PacketID = %li ORDER BY ComponentOrder %s", dlg.m_nPacketID, dlg.m_bReverseMerge ? "DESC" : "");
			CStringArray saPaths;
			while(!rsTemplates->eof) {
				saPaths.Add(AdoFldString(rsTemplates, "Path"));
				rsTemplates->MoveNext();
			}
			rsTemplates->Close();

			BOOL bAtLeastOneMerged = FALSE;
			if(dlg.m_bSeparateDocuments) {
				CSelectSenderDlg dlgSender(this);
				if(dlgSender.DoModal() == IDCANCEL)
					return;

				for(int nTemplate = 0; nTemplate < saPaths.GetSize(); nTemplate++) {
					CMergeEngine mi;
					mi.m_nPicID = GetPicID();
				
					CString strPath = saPaths.GetAt(nTemplate);;
					
					//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
					if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
						strPath = GetSharedPath() ^ strPath;
					}


					// PLID  15371: make sure the path exists and if it doesn't, output a warning
					if (! DoesExist(strPath)) {
						CString strMessage;
						strMessage.Format("The file %s in the packet does not exist.\nPlease check the path of the template. This packet cannot be merged.", strPath);
						// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
						MessageBox(strMessage, NULL, MB_ICONINFORMATION);
						return;
					}
					
					/// Generate the temp table
					CString strSql;
					strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_id);
					CString strMergeTo = CreateTempIDTable(strSql, "ID");
					
					// Merge
					if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
					mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

					mi.m_strSubjectMatter = dlgSender.m_strSubjectMatter;

					mi.m_strSender = dlgSender.m_strFirst + (dlgSender.m_strMiddle.IsEmpty() ? "" : (" "+ dlgSender.m_strMiddle)) + " " + dlgSender.m_strLast;
					mi.m_strSenderFirst = dlgSender.m_strFirst;
					mi.m_strSenderMiddle = dlgSender.m_strMiddle;
					mi.m_strSenderLast = dlgSender.m_strLast;
					mi.m_strSenderEmail = dlgSender.m_strEmail;
					mi.m_strSenderTitle = dlgSender.m_strTitle;

					//assign a category ID based on preferences
					// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file, always FALSE here
					long nCategoryID = CheckAssignCategoryID(FALSE);
					if(nCategoryID != -1) {
						mi.m_nCategoryID = nCategoryID;
					}

					// Do the merge
					//TES 8/10/2004: We still want to store that the packet was merged.
					long nMergedPacketID = NewNumber("MergedPacketsT", "ID");
					ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID, SeparateDocuments) VALUES (%li, %li, 1)", nMergedPacketID, dlg.m_nPacketID);

					CString strCurrentTemplateFilePath;
					// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object

					// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
					if (mi.MergeToWord(strPath, std::vector<CString>(), strMergeTo, "", nMergedPacketID, nPacketCategoryID, true, FormatString("Template %i of %i", nTemplate + 1, saPaths.GetSize()))) {
						bAtLeastOneMerged = TRUE;
						PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, GetActivePersonID(), COleDateTime::GetCurrentTime(), nMergedPacketID);
					}
					else {
						break;
					}
				}			
				
				// If the merge was successful, refresh the history tab
				if (bAtLeastOneMerged) {
					UpdateView();
				}
			}
			else {
				for(int nTemplate = 0; nTemplate < saPaths.GetSize(); nTemplate++) {
					
					CString strPath = saPaths.GetAt(nTemplate);
					
					//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
					if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
						strPath = GetSharedPath() ^ strPath;
					}


					// PLID  15371: make sure the path exists and if it doesn't, output a warning
					if (! DoesExist(strPath)) {
						CString strMessage;
						strMessage.Format("The file %s in the packet does not exist.\nPlease check the path of the template. This packet cannot be merged.", strPath);
						// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
						MessageBox(strMessage, NULL, MB_ICONINFORMATION);
						return;
					}


					// Add the full path to the array
					arystrPaths.Add(strPath);
				}
				
				/// Generate the temp table
				CString strSql;
				strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_id);
				CString strMergeTo = CreateTempIDTable(strSql, "ID");
				
				// Merge
				CMergeEngine mi;
				mi.m_nPicID = GetPicID();

				if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
				mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

				CSelectSenderDlg dlgSender(this);
				if(dlgSender.DoModal() == IDCANCEL)
					return;

				mi.m_strSubjectMatter = dlgSender.m_strSubjectMatter;

				mi.m_strSender = dlgSender.m_strFirst + (dlgSender.m_strMiddle.IsEmpty() ? "" : (" "+ dlgSender.m_strMiddle)) + " " + dlgSender.m_strLast;
				mi.m_strSenderFirst = dlgSender.m_strFirst;
				mi.m_strSenderMiddle = dlgSender.m_strMiddle;
				mi.m_strSenderLast = dlgSender.m_strLast;
				mi.m_strSenderEmail = dlgSender.m_strEmail;
				mi.m_strSenderTitle = dlgSender.m_strTitle;

				//assign a category ID based on preferences
				// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file, always FALSE here
				long nCategoryID = CheckAssignCategoryID(FALSE);
				if(nCategoryID != -1) {
					mi.m_nCategoryID = nCategoryID;
				}

				// Do the merge
				long nMergedPacketID = NewNumber("MergedPacketsT", "ID");
				ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID, SeparateDocuments) VALUES (%li, %li, 0)", nMergedPacketID, dlg.m_nPacketID);

				CString strCurrentTemplateFilePath;
				
				// Loop through each element in the list of paths and run the merge for each
				long nCount = arystrPaths.GetSize();
				for (long i=0; i<nCount; i++) {
					// Merge for this element
					strCurrentTemplateFilePath = arystrPaths.GetAt(i);
					// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
					// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
					if (mi.MergeToWord(strCurrentTemplateFilePath, std::vector<CString>(), strMergeTo, "", nMergedPacketID, nPacketCategoryID, false, FormatString("Template %i of %i", i + 1, nCount))) {
						bAtLeastOneMerged = TRUE;
					}
					else {
						break;
					}
				}
				
				if (bAtLeastOneMerged) {
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, GetActivePersonID(), COleDateTime::GetCurrentTime(), nMergedPacketID);

					// If the merge was successful, refresh the history tab
					UpdateView();
				}
			} 
		
	}
  }NxCatchAll("CHistoryDlg::OnClickMergePacket");
}

void CHistoryDlg::OnRequeryFinishedDocuments(short nFlags) 
{
	try {
		// (a.walling 2008-09-15 17:29) - PLID 23904 - Made this more modular by putting most work within a single loop

		long p = m_pDocuments->GetFirstRowEnum();
		
		LPDISPATCH pDisp = NULL;

		while (p)
		{
			m_pDocuments->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);
			// (a.walling 2010-08-06 14:21) - PLID 40007 - Memory leak -- the IDispatch interface from GetNextRowEnum must be released.
			if (pDisp) {
				pDisp->Release();
			}
			
			FillPathAndFile(pRow);

			EnsureRowIcon(pRow);	
		}

		//GetDlgItem(IDC_DOCUMENTS)->SetRedraw(FALSE);
		//GetDlgItem(IDC_DOCUMENTS)->SetRedraw(TRUE);
		UpdateListColors();

		//set the scroll to where it used to be, if it's been set
		if(m_nTopRow != -1) {
			m_pDocuments->PutTopRowIndex(m_nTopRow);
		}

		if(m_strOriginalColSizes.IsEmpty()) {
			//First, remember the original settings
			CString strOrig, str;
			for(int i = 0; i < m_pDocuments->GetColumnCount(); i++) {
				IColumnSettingsPtr pCol = m_pDocuments->GetColumn(i);
				str.Format("%li,", pCol->GetStoredWidth());

				strOrig += str;
			}

			m_strOriginalColSizes = strOrig;

			if(GetRemotePropertyInt("RememberHistoryColumns", 0, 0, GetCurrentUserName(), true) == 1) {
				CheckDlgButton(IDC_REMEMBER_HISTORY_COLUMNS, TRUE);
				//(e.lally 2006-07-24) PLID 21589 - We want to call Remember Columns without the
					//update view and do any refreshing outside of this handler, because we don't need to
					//update the view if that is what we just got done doing.
				RememberColumns();
			}
			else {
				CheckDlgButton(IDC_REMEMBER_HISTORY_COLUMNS, FALSE);
			}
		}
		if(GetDlgItem(IDC_DOCUMENTS)->IsWindowVisible()) {
			m_pDocuments->SetRedraw(TRUE);
		}

		// (j.jones 2009-09-22 12:54) - PLID 31620 - if the EMN preview is open, fill it
		// with the EMNs displayed in the list
		if(m_pdlgEmrPreview != NULL && m_pdlgEmrPreview->IsWindowVisible())
		{
			// (z.manning 2012-09-10 14:31) - PLID 52543 - Use the new EmnPreviewPopup object here
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;

			//find all EMNIDs in the list
			for(int i = 0; i < m_pDocuments->GetRowCount(); i++) {
				long nEMNID = VarLong(m_pDocuments->GetValue(i, phtdlcEmnID), -1);
				if(nEMNID != -1) {
					COleDateTime dtEmnModifiedDate = VarDateTime(m_pDocuments->GetValue(i, phtdlcEmnModifiedDate));
					aryEMNs.Add(EmnPreviewPopup(nEMNID, dtEmnModifiedDate));
				}
			}
			
			//(j.camacho 2015-07-10 4:31pm) - PLID 66500 - Refresh and make sure not to lose what index they are on
			m_pdlgEmrPreview->PreviewEMN(aryEMNs, m_pdlgEmrPreview->getCurIndex());

		}

	}NxCatchAll("Error in CHistoryDlg::OnRequeryFinishedDocuments()");
}

// (a.walling 2008-09-15 16:57) - PLID 23904 - Ensure the row has an appropriate icon
void CHistoryDlg::EnsureRowIcon(NXDATALISTLib::IRowSettingsPtr pRow)
{
	_variant_t varName = pRow->GetValue(phtdlcName);
			
	// (z.manning 2008-07-03 09:13) - PLID 25574 - Show the EMN icon on EMN rows
	long nEmnID = VarLong(pRow->GetValue(phtdlcEmnID), -1);
	if(nEmnID != -1) {
		pRow->PutValue(phtdlcName, (long)m_hiconEmn);
		// (a.walling 2008-09-15 16:53) - PLID 23904 - Also Icon
		pRow->PutValue(phtdlcIcon, (long)m_hiconEmn);
	}
	else if(varName.vt == VT_NULL) {
		//TES 9/22/2008 - PLID 6484 - If this is a multi-patient packet, and we have the preference on to block them from opening
		// multi-patient packets, then don't show the icon.
		CString strPathName = VarString(pRow->GetValue(phtdlcFilename),"");
		if(strPathName.Find("MultiPatDoc") == -1 || GetRemotePropertyInt("HistoryBlockMultiPatDocs", 1, 0, "<None>") == 0) {
			pRow->PutValue(phtdlcName, (long)m_hiconPacket);
			// (a.walling 2008-09-15 16:53) - PLID 23904 - Also Icon
			pRow->PutValue(phtdlcIcon, (long)m_hiconPacket);
		}
		else {
			//TES 9/22/2008 - PLID 6484 - Also, we want to fill this cell with an empty string, various places expect this 
			// cell to be non-null.
			pRow->PutValue(phtdlcName, _bstr_t(""));
		}
	} else if ( (varName.vt == VT_BSTR) && (GetRemotePropertyInt("HistoryShowShellIcons", TRUE, 0, GetCurrentUserName(), true)) ) {
		// (a.walling 2008-09-15 16:54) - PLID 23904 - Override the display icon with one from the shell
		CString filename = VarString(pRow->GetValue(phtdlcFilePath), "") ^ VarString(pRow->GetValue(phtdlcFilename), "");

		if(!filename.IsEmpty()) {
			// (a.walling 2013-10-02 09:46) - PLID 58847 - Get cached, small, generic icon
			if (HICON hIcon = FileUtils::GetCachedSmallIcon(filename)) {
				pRow->PutValue(phtdlcIcon, (long)hIcon);
			}
		}
	}

	// (c.haag 2010-07-01 15:59) - PLID 39473 - Also ensure the proper icon for the Extra Notes field
	long nMailID = VarLong(pRow->GetValue(phtdlcMailID), -1);
	BOOL bHasExtraNotes = VarBool(pRow->GetValue(phtdlcHasExtraNotes));
	if (-1 == nMailID) {
		// History items with no mail ID cannot have extra notes
		pRow->PutValue(phtdlcExtraNotesIcon,g_cvarNull);
	} else if (bHasExtraNotes) {
		pRow->PutValue(phtdlcExtraNotesIcon,(long)m_hExtraNotesIcon);
	} else {
		pRow->PutValue(phtdlcExtraNotesIcon,(LPCTSTR)"BITMAP:FILE");
	}
}

void CHistoryDlg::OnRButtonDownDocuments(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {

		//DRT 7/14/03 - Made a few changes:  There is now the option to quickly 'Attach file'.  This is useful if you are 
		//		showing all unattached documents, and perhaps you have a number of images in that patients documents
		//		folder, this allows them to easily and quickly add those images, without going to New -> Attach existing file.

		if (nRow == -1)
			return;

		long nEmnID = VarLong(m_pDocuments->GetValue(nRow, phtdlcEmnID), -1);
		if(nEmnID != -1) {
			// (z.manning 2008-07-03 09:21) - PLID 25574 - No menu for EMN rows
			return;
		}

		_variant_t varPicID = m_pDocuments->GetValue(nRow, phtdlcPicID);

		// Build a popup menu to change task characteristics
		bool bUseMenu = false;
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		//select this row as the current one
		//TES 8/6/2004: If this row is one of the highlighted rows, don't change the selection.
		bool bClickedRowSelected = false;
		bool bUnattachedFound = false;
		long p = m_pDocuments->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		int nCountSelected = 0;
		while (p)
		{	
			nCountSelected++;
			m_pDocuments->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			if(pRow->GetIndex() == nRow) bClickedRowSelected = true;
			if(pRow->GetValue(phtdlcStaff).vt == VT_NULL || pRow->GetValue(phtdlcStaff).vt == VT_EMPTY) bUnattachedFound = true;
			pDisp->Release();
		}
		if(!bClickedRowSelected) {
			m_pDocuments->PutCurSel(nRow);
			nCountSelected = 1;
			bUnattachedFound = (m_pDocuments->GetValue(nRow, phtdlcStaff).vt == VT_NULL || m_pDocuments->GetValue(nRow, phtdlcStaff).vt == VT_EMPTY);
		}
		m_nRowSel = nRow;	
		EnableAppropriateButtons();

		//TES 9/8/2008 - PLID 6484 - If this is a multi-patient document, but it doesn't have the Word icon, then it is blocked
		// by preferences, meaning we shouldn't offer certain options in the right-click menu (such as Print).
		bool bIsBlockedFile = false;
		CString strPathName = VarString(m_pDocuments->GetValue(nRow, phtdlcFilename),"");
		if(strPathName.Find("MultiPatDoc") != -1) {
			_variant_t varName = m_pDocuments->GetValue(nRow, phtdlcName);
			//TES 9/22/2008 - PLID 6484 - If it's not a string, then it's an icon, so definitely not blocked.
			if(varName.vt == VT_BSTR) {
				CString strName = VarString(varName,"");
				if(strName.IsEmpty()) {
					bIsBlockedFile = true;
				}
			}
		}

		// (a.wetta 2007-07-10 13:59) - PLID 17467 - Make sure that the file is also attached
		if(m_bInPatientsModule && nCountSelected == 1&& !bUnattachedFound) {
			//This is the code for handling whether to show 'Mark as Primary Picture' or not.

			CString str = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));
			if (str.IsEmpty())
				str = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal) + "\\" + CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);

			if (IsImageFile(str))
			{
				IRowSettingsPtr pRow = m_pDocuments->GetRow(nRow);

				// (a.wetta 2007-07-09 10:12) - PLID 17467 - Determine if this document has been marked as a Photo or not
				BOOL bIsPhoto;
				if (pRow->GetValue(phtdlcIsPhoto).vt == VT_EMPTY || pRow->GetValue(phtdlcIsPhoto).vt == VT_NULL) {
					// This document was attached before this option existed, so always assume it's a photo
					bIsPhoto = TRUE;
				}
				else {
					bIsPhoto = (BOOL)VarLong(pRow->GetValue(phtdlcIsPhoto));
				}

				// (a.wetta 2007-07-09 10:42) - PLID 17467 - Only show this option if this is a photo
				if (bIsPhoto) {
					if (pRow->ForeColor == RGB(64,64,255))
					{
						mnu.InsertMenu(0, MF_BYPOSITION, IDM_RESET_PRIMARY_PICTURE, "&Reset primary picture");
					}
					else
					{
						mnu.InsertMenu(0, MF_BYPOSITION, IDM_SET_PRIMARY_PICTURE, "&Mark as primary picture");
					}
				}

				// (a.wetta 2007-07-09 10:43) - PLID 17467 - Add a menu item to change the photo status
				if (bIsPhoto) {
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_TOGGLE_PHOTO_STATUS, "Mark as non-&photo");
				}
				else {
					//TES 8/12/2011 - PLID 44966 - Don't let them mark as photo if they don't have permission
					// (e.frazier 2016-05-19 10:31) - This permission check doesn't take place in the Contacts module
					if (m_bInPatientsModule)
					{
						if (GetCurrentUserPermissions(bioPatientHistory)&sptDynamic1) {
						mnu.InsertMenu(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED, IDM_TOGGLE_PHOTO_STATUS, "Mark as &photo");
						}
						else {
							mnu.InsertMenu(0, MF_BYPOSITION, IDM_TOGGLE_PHOTO_STATUS, "Mark as &photo");
						}
					}
					
				}

				bUseMenu = true;
			}
			// (a.walling 2007-07-19 11:21) - PLID 26748 - support .docx as well
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			else if (IsFileType(str, ".doc") || IsFileType(str, ".docx") || IsFileType(str, ".docm"))
			{
				if(!bIsBlockedFile) {
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_PRINT_FILE, "&Print...");
					bUseMenu = true;
				}
			}
		}
		
	

		{
			//Decide whether to show 'Attach file' or not
			if(bUnattachedFound) {
				mnu.InsertMenu(1, MF_BYPOSITION, IDM_ATTACH_FILE, "&Attach File(s)");
				bUseMenu = true;
			}
		}

		{
			//Decide whether or not to show 'Email File to Person' or not
			CString str = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));
			if (str.IsEmpty())
				str = CString(m_pDocuments->GetValue(nRow, phtdlcFilePath).bstrVal) + "\\" + CString(m_pDocuments->GetValue(nRow, phtdlcFilename).bstrVal);

			str.MakeUpper();
			if (str.GetLength() >= 4) {

				// (m.hancock 2006-05-19 13:14) - PLID 20728 - Do not display the email option if the 
				// document is a case history or if the filename is empty.
				CString filename = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));
				if ((filename.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) != 0)
					&& (filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_REQUEST) != 0)
					&& (filename.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) != 0)
					&& (!filename.IsEmpty()) 
					&& (!bIsBlockedFile)) {

					//DRT 7/2/2008 - PLID 30601 - Hijack this portion to also check if we want to be able to send a fax.
					//DRT 8/1/2008 - PLID 30915 - Added licensing
					if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
						mnu.InsertMenu(2, MF_BYPOSITION, IDM_SEND_FAX, "Send &Fax...");
						bUseMenu = true;
					}

					_RecordsetPtr rs = CreateRecordset("SELECT Email FROM PersonT WHERE Email <> '' AND ID = %li",m_id);
					if(!rs->eof) {
						CString strEmailAddress = AdoFldString(rs, "Email","");
						if(strEmailAddress != "") {
							mnu.InsertMenu(2, MF_BYPOSITION, IDM_EMAIL_FILE, "&Email File To Person");
							bUseMenu = true;
						}
					}
					rs->Close();
				}
			}
		}

		// (a.walling 2009-05-06 10:33) - PLID 34176 - Allow right-click view of possible CCD documents
		if (nCountSelected == 1 && (m_bInPatientsModule || GetPicContainer()) && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			// Get the path (just to make sure there IS one)
			CString strPath = 
				VarString(m_pDocuments->GetValue(nRow, phtdlcFilePath)) ^ 
				VarString(m_pDocuments->GetValue(nRow, phtdlcFilename));

			BOOL bIsCCD = FALSE;

			_variant_t varSelection = m_pDocuments->GetValue(nRow, phtdlcSelection);
			if (varSelection.vt == VT_BSTR) {
				CString strSelection = VarString(varSelection);
				if (strSelection == SELECTION_CCD) {
					bIsCCD = TRUE;
				}
			}
			// (j.jones 2010-06-30 11:12) - PLID 38031 - changed to differentiate between CCD and generic XML,
			// though they now call the same function in IDM_VIEW_XML
			// (j.gruber 2013-11-08 11:18) - PLID 59376 - nothing needed here since used in isXmlFileExtension
			BOOL bIsXMLFileType = bIsCCD || NxXMLUtils::IsXMLFileExtension(strPath);
			if(bIsXMLFileType && NxXMLUtils::IsValidXMLFile(strPath)) {
				NxXMLUtils::EDocumentType documentType = NxXMLUtils::GetXMLDocumentType(strPath);
				if(documentType == NxXMLUtils::CCD_Document || documentType == NxXMLUtils::CCR_Document) {
					//(e.lally 2010-08-12) PLID 40104 - Only need separator if this isn't the first option
					if (mnu.GetMenuItemCount() > 0) {
						mnu.InsertMenu(0, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					}
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_VIEW_XML, "&View CCD / CCR Document");
#ifdef _DEBUG
					// (a.walling 2010-01-19 14:09) - PLID 36972 - Validation via NIST web service removed (service out of date)
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_VALIDATE_CCD, "Open NIST CDA Guideline Validation Website");
#endif
				}
				// (b.spivey - November 13th, 2013) - PLID 57964 - Allow them to open this just like the CCD did.
				else if(documentType == NxXMLUtils::CCDA_Document) 
				{
					if (mnu.GetMenuItemCount() > 0) {
						mnu.InsertMenu(0, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					}
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_VIEW_XML, "&View CCDA Document");
				}
				else {
					//(e.lally 2010-08-12) PLID 40104 - Only need separator if this isn't the first option
					if (mnu.GetMenuItemCount() > 0) {
						mnu.InsertMenu(0, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					}
					mnu.InsertMenu(0, MF_BYPOSITION, IDM_VIEW_XML, "&View Document");
				}
				//(e.lally 2010-08-12) PLID 40104 - turn on the popup menu
				bUseMenu = true;
			}
		}

		// (z.manning 2010-02-02 11:57) - PLID 34287 - Option to attach to current PIC assuming we're in a PIC
		// and this document is not already associated with another PIC.
		// (j.dinatale 2012-07-02 17:24) - PLID 51326 - dont show the option if the mailID is not valid
		long nMailID = VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID), -1);
		if (nMailID > 0 && nCountSelected == 1 && GetPicContainer() && varPicID.vt == VT_NULL) {
			bUseMenu = true;
			mnu.InsertMenu(0, MF_BYPOSITION, IDM_ATTACH_TO_CURRENT_PIC, "A&ttach to current PIC");
		}

		// (c.haag 2010-05-21 13:33) - PLID 38731 - If we're in the patients module, and the file is attached,
		// let the user add a new todo alarm and attach it to the document
		if (m_bInPatientsModule && nCountSelected == 1 && !bUnattachedFound) {
			if (CanCreateTodoForDocument(nRow)) {
				bUseMenu = true;
				if (mnu.GetMenuItemCount() > 0) {
					mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION|MF_SEPARATOR, 0, "");
				}
				mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_NEW_TODO, "Create To-Do Tas&k For This Document");
			}
		}



		{
			CString strPath = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));

			// (j.gruber 2010-01-05 12:33) - PLID 22958 - make a change patient function for all images and pdfs
			//only allow in the patients module and if they only have one item selected and its attached
			if(m_bInPatientsModule && nCountSelected == 1&& !bUnattachedFound) {
				if (IsImageFile(strPath) || IsFileType(strPath, ".pdf")) {
					//only attached items
					//(e.lally 2010-08-12) PLID 40104 - Only need separator if this isn't the first option
					if (mnu.GetMenuItemCount() > 0) {
						mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION|MF_SEPARATOR, 0, "");
					}
					mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_CHANGE_PATIENT, "&Change Patient");	
					//(e.lally 2010-08-12) PLID 40104 - turn on the popup menu
					bUseMenu = true;
				}
			}
		}

		{
		
			// (j.camacho 2013-11-04 15:32) - PLID 59303 
			// (b.spivey - November 27th, 2013) - PLID 59590 - License changes for Direct Messaging
			// (j.camacho 2014-02-05 18:12) - PLID 60188 - Do not allow unattached items to be sent.
			if (g_pLicense->CheckForLicense(CLicense::lcDirectMessage, CLicense::cflrSilent) && !bUnattachedFound) {
				mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_DIRECTMESSAGE_SEND, "Attach and Send Direct Message");
			}
		}

		// Decide whether to include the "send to server" option
		if (IsNexTechInternal()) {
			// Get the path (just to make sure there IS one)
			CString strPath = 
				VarString(m_pDocuments->GetValue(nRow, phtdlcFilePath)) ^ 
				VarString(m_pDocuments->GetValue(nRow, phtdlcFilename));
			if (!strPath.IsEmpty()) {
				// Get the shared path
				CString strShared = GetSharedPath();
				long nLenSharedPath = strShared.GetLength();
				// See if this entry's full path BEGINS with the shared path
				if (strnicmp(strShared, strPath, nLenSharedPath) == 0) {
					// The path is good, send the file
					CString strRemoteServerIP = NxRegUtils::ReadString(_T("HKEY_LOCAL_MACHINE\\Software\\NexTech\\RemoteServerIP"));
					CString strRemoteSharedPath = NxRegUtils::ReadString(_T("HKEY_LOCAL_MACHINE\\Software\\NexTech\\RemoteServerSharedPath"));
					if (!strRemoteServerIP.IsEmpty() && !strRemoteSharedPath.IsEmpty()) {
						// We're allowed to send the file remotely so add the menu item for it
						if (mnu.GetMenuItemCount() > 0) {
							mnu.InsertMenu(-1, MF_BYPOSITION|MF_SEPARATOR, 0, "");
						}
						mnu.InsertMenu(-1, MF_BYPOSITION, IDM_SEND_TO_SERVER, "&Send to " + strRemoteServerIP);
						bUseMenu = true;
					}
				}
			}
		}

	// (s.dhole 2013-11-01 12:08) - PLID 59278 Load only if they have valid license
		if (m_bInPatientsModule && nCountSelected == 1 && !bUnattachedFound) {
			if (IsValidCCDADocumentImport(GetFileFullPath())){
				bUseMenu = true;
				
				if (mnu.GetMenuItemCount() > 0) {
					mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION|MF_SEPARATOR, 0, "");
				}
				// (s.dhole 2014-01-13 12:42) - PLID 59643  enabled/desabled based on permission
				mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_CCDA_RECONCILIATION_MEDICATION,"CCDA - Medication Reconciliation");
				mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_CCDA_RECONCILIATION_ALLERGY,	"CCDA - Allergy Reconciliation");
				// (s.dhole 2014-01-13 12:42) - PLID 59643  Load only if they have EMR license
				if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2){
					mnu.InsertMenu(mnu.GetMenuItemCount(), MF_BYPOSITION, IDM_CCDA_RECONCILIATION_PROBLEM,	"CCDA - Problem Reconciliation");
				}
			}
		}
		//Now decide whether we want to show the menu or not
		if(bUseMenu) {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}

		EnableAppropriateButtons();

	}NxCatchAll("CHistoryDlg::OnRButtonDownDocuments");
}

void CHistoryDlg::OnSetPrimaryPicture()
{
	long nOldPrimaryPictureID = -1;
	if (m_nRowSel == -1)
		return;
	
	try
	{
		// Get the ID of the current primary picture; we need this for later.
		_RecordsetPtr prs = CreateRecordset("SELECT PatPrimaryHistImage FROM PatientsT WHERE PersonID = %d AND PatPrimaryHistImage IS NOT NULL",
			m_id);
		if (!prs->eof)
			nOldPrimaryPictureID = AdoFldLong(prs, "PatPrimaryHistImage");
		prs->Close();

		// Update the patients table with the new primary image
		if (m_pDocuments->GetValue(m_nRowSel, phtdlcMailID).vt == VT_NULL ||
			m_pDocuments->GetValue(m_nRowSel, phtdlcMailID).vt == VT_EMPTY ||
			VarLong(m_pDocuments->GetValue(m_nRowSel, phtdlcMailID)) == -1)
		{			
			if (IDYES == MsgBox(MB_YESNO, "This image cannot be marked as the primary image because is not attached to the patient's history file. Would you like to attach and mark it as the primary picture now?"))
			{
				// (j.jones 2009-12-18 08:31) - PLID 34606 - check permissions after the warning,
				// otherwise they might not know what the permission warning was for
				// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
				if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
					return;
				}

				CString strFilename = CString(m_pDocuments->GetValue(m_nRowSel, phtdlcFilePath).bstrVal) + "\\" + CString(m_pDocuments->GetValue(m_nRowSel, phtdlcFilename).bstrVal);
				CString strFileNameOnly = CString(m_pDocuments->GetValue(m_nRowSel, phtdlcFilename).bstrVal);
				// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file, always TRUE here
				long nNewPrimaryPictureID = AttachFileToHistory(strFilename, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(strFileNameOnly)));

				m_pDocuments->WaitForRequery(dlPatienceLevelWaitIndefinitely);
				if (-1 == (m_nRowSel = m_pDocuments->FindByColumn(phtdlcMailID, nNewPrimaryPictureID, 0, TRUE)))
				{
					MsgBox("Practice failed to mark the image as the primary image, but the document was attached. Please try to assign the primary image again.");
					return;
				}
			}
			else
				return;
		}
	
	//(s.dhole 10/8/2014 3:59 PM ) - PLID  37718 Try to update 		PatientsT.PatPrimaryHistImage
		_RecordsetPtr pPrimaryrs = CreateParamRecordset("SET NOCOUNT ON "
			" DECLARE @PersonID  int  "
			" DECLARE @MailID  int  "
			" SET @PersonID = {INT}  "
			" SET @MailID = {INT}  "
			" UPDATE PatientsQ SET PatPrimaryHistImage = MailSent.MailID FROM  PatientsT PatientsQ INNER JOIN  MailSent  ON PatientsQ.PersonID = MailSent.PersonID Where MailSent.PersonID = @PersonID AND MailSent.MailID = @MailID "
			" SET NOCOUNT OFF  "
			" SELECT  PersonID FROM PatientsT WHERE   PersonID = @PersonID AND PatPrimaryHistImage = @MailID", m_id, VarLong(m_pDocuments->GetValue(m_nRowSel, phtdlcMailID)));
		// return records mean, we could update PatPrimaryHistImage 
		if (!pPrimaryrs->eof)
		{
			m_PhotoViewer.SetPrimaryImage(VarLong(m_pDocuments->GetValue(m_nRowSel, phtdlcMailID)));

			// Reset the currently blue primary picture record to black
			//(e.lally 2009-01-23) PLID 32847 - We can only access rows 0 and above. Otherwise, we can ignore it.
			long nRow = m_pDocuments->FindByColumn(phtdlcMailID, nOldPrimaryPictureID, 0, FALSE);
			if (nRow >= 0)
			{
				IRowSettingsPtr pRow = m_pDocuments->GetRow(nRow);
				ASSERT(VarLong(pRow->Value[0]) == nOldPrimaryPictureID);
				pRow->ForeColor = RGB(0, 0, 0);
			}

			// Now assign the primary picture color in blue
			UpdateListColors();

			//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
			// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
			// this shared function will handle sending accordingly
			SendPatientPrimaryPhotoHL7Message(m_id);
		}
		else
		{
			//No return record , we fail to update PatPrimaryHistImage  and photo is removed, shoud call refresh
			CClient::RefreshMailSentTable(m_id, VarLong(m_pDocuments->GetValue(m_nRowSel, phtdlcMailID)), TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);
		}


	}
	NxCatchAll("Error setting list colors");
}

void CHistoryDlg::OnResetPrimaryPicture()
{
	if (m_nRowSel == -1)
		return;

	try {
		ExecuteSql("UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PersonID = %d", m_id);
		m_PhotoViewer.SetPrimaryImage(-1);
		IRowSettingsPtr pRow = m_pDocuments->GetRow(m_nRowSel);
		pRow->ForeColor = RGB(0,0,0);
		//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
		// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
		// this shared function will handle sending accordingly
		SendPatientPrimaryPhotoHL7Message(m_id);
	} NxCatchAll("Error resetting primary picture");
}

void CHistoryDlg::UpdateListColors()
{
	try
	{
		_RecordsetPtr prs = CreateRecordset("SELECT PatPrimaryHistImage FROM PatientsT WHERE PersonID = %d AND PatPrimaryHistImage IS NOT NULL",
			m_id);
		if (!prs->eof)
		{
			long lMailSentID = AdoFldLong(prs, "PatPrimaryHistImage");
			m_PhotoViewer.SetPrimaryImage(lMailSentID);
			long nRow = m_pDocuments->FindByColumn(phtdlcMailID, lMailSentID, 0, FALSE);
			//(e.lally 2009-01-23) PLID 32847 - We can only access rows 0 and above. On requeryFinished will try again.
			if (nRow >=0)
			{
				IRowSettingsPtr pRow = m_pDocuments->GetRow(nRow);
				ASSERT(VarLong(pRow->Value[0]) == lMailSentID);
				pRow->ForeColor = RGB(64,64,255);
			}
		}
	}
	NxCatchAll("Error setting list colors");
}

void CHistoryDlg::OnClickImportFromPDA()
{
  // (e.lally 2009-06-11) PLID 34600 - Moved try up to the top
  try {
	// (e.lally 2009-06-11) PLID 34600 - Check write permissions
	// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
	if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
		return;
	}
	CNxRAPI* pRAPI = new CNxRAPI;
	if (!pRAPI->IsValid())
	{
		MsgBox("Practice was unable to interface with the PDA. Please have your hardware administrator make sure your PDA related software is properly installed.");
		return;
	}

	CNxRAPIBrowseDlg dlg(pRAPI, TRUE);
	if (IDOK == dlg.DoModal())
	{
		CString strPDAFilename = dlg.GetPathName();
		
		// Since this version does not support file conversions, we
		// need to let the user know, if applicable.
		if (strPDAFilename.GetLength() >= 4)
		{
			if (strPDAFilename.Right(4) == ".pwd")
			{
				if (IDNO == MsgBox(MB_YESNO, "The file you are attempting to import is in Pocket Word format. Practice does not support converting Pocket Word documents to Microsoft Word documents. Do you still wish to import the file as is?"))
					return;
			}
			if (strPDAFilename.Right(4) == ".pxl")
			{
				if (IDNO == MsgBox(MB_YESNO, "The file you are attempting to import is in Pocket Excel format. Practice does not support converting Pocket Word documents to Microsoft Excel documents. Do you still wish to import the file as is?"))
					return;
			}
			if (strPDAFilename.Right(4) == ".2bp")
			{
				if (IDNO == MsgBox(MB_YESNO, "The file you are attempting to import is in a CE 4-color bitmap format. Practice does not support converting CE 4-color bitmaps to Windows Bitmap (bmp) files. Do you still wish to import the file as is?"))
					return;
			}
		}

		// Calculate the incoming filename
		CString fileName = strPDAFilename;
		long nSlashLoc = fileName.ReverseFind('\\');
		BOOL bAlreadyExists = FALSE;
		if (nSlashLoc != -1)
			fileName = fileName.Right(fileName.GetLength() - nSlashLoc - 1);		
		
		fileName = GetCurPatientDocumentPath() ^ fileName;

		//DRT 7/2/03 - We need to make sure this file doesn't already exist before just blindly importing it.
		if(DoesExist(fileName)) {
			//it does!
			if(MsgBox(MB_YESNO, "This file (%s) already exists in the import location.  Are you sure you wish to overwrite it?", fileName) == IDNO)
				return;
			bAlreadyExists = TRUE;
		}

		// (e.lally 2009-06-11) PLID 34600 - Moved to top
		//try {
			// Attach the import to the history			
			BOOL bSuccess = FALSE;
			// Commit the import
			if(pRAPI->CopyCEToPC(strPDAFilename, fileName) != S_OK) {
				//some failure copying the file
				return;
			}
			if (!bAlreadyExists)
			{
				CString strSelection = SELECTION_FILE;

				// (c.haag 2006-10-09 09:32) - PLID 22679 - If the file has a .wav extension, then import it with the audio icon
				if (IsWaveFile(strPDAFilename)) {
					strSelection = SELECTION_AUDIO;
				}

				// (j.gruber 2010-01-22 13:44) - PLID 23693 - check if its an image file
				if (AttachFileToHistory(fileName, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(IsImageFile(fileName)), strSelection, strPDAFilename) > 0)
				{
					bSuccess = TRUE;
				}
			}
			else // It's already attached and we copied over the file, so all is well
				bSuccess = TRUE;

			// Tell the user the import is done
			if (bSuccess)
			{
				MsgBox("The file '%s' was successfully imported to your computer under the filename '%s'",
					strPDAFilename, fileName);
			}
		}
	} 
	NxCatchAll("Error importing a file from the PDA");
}

// (d.lange 2010-06-21 16:50) - PLID 39202 - Launch the Device Import dialog
void CHistoryDlg::OnDeviceImport()
{
	try{
		//Let's show the Device Import dialog
		GetMainFrame()->ShowDevicePluginImportDlg();

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-05-13 15:42) - PLID 34243 - Allow calling from outside
void CHistoryDlg::ImportCCDs()
{
	OnImportCCD();
}

// (a.walling 2009-05-05 17:12) - PLID 34176 - Attach a CCD document
void CHistoryDlg::OnImportCCD()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
		// (j.gruber 2013-11-08 10:04) - PLID 59376 - and ccda
		const char* szCCDCCRExtensionFilter = 
			"CCDA / CCD / CCR Files (*.xml, *.ccda *.ccd *.cda *.ccr)|*.xml;*.ccda;*.ccd;*.cda;*.ccr|"
			"All Files (*.*)|*.*|"
			"|";

		CStringArray saFiles;
		CArray<long, long> arMailIDs;
		CArray<long, long> arPatientIDs;
		long nSwitchToPatientID = -1;
	

		// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
		ImportAndAttach(SELECTION_CCD, szCCDCCRExtensionFilter, &saFiles, &arMailIDs, &arPatientIDs, &nSwitchToPatientID);

		// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
		UpdateCCDInformation(saFiles, arMailIDs, arPatientIDs);

		if (nSwitchToPatientID != -1 && m_bInPatientsModule && !GetPicContainer()) {
			if (GetMainFrame()) {
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(GetMainFrame()->m_patToolBar.TrySetActivePatientID(nSwitchToPatientID)) {
					CNxView* pView = GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
					if (pView) {
						((CPatientView*)pView)->UpdateView();
					}
				}
			}
		}
	} NxCatchAll("CHistoryDlg::OnImportCCD");
}

// (a.walling 2009-06-05 13:14) - PLID 34176 - Create a CCD document
void CHistoryDlg::OnCreateCCD()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		GetMainFrame()->CreateCCD(GetActivePersonID(), this);
	} NxCatchAll("CHistoryDlg::OnCreateCCD");
}

// (j.gruber 2013-11-08 08:41) - PLID 59375 - create CCDA
void CHistoryDlg::OnCreateCCDA()
{
	try {		
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		long nCurrentPersonID = GetActivePatientID();

		// (a.walling 2014-05-13 14:43) - PLID 61788 - Just call the shared function to generate
		CCDAUtils::GenerateSummaryOfCare(nCurrentPersonID, -1, *this);
		
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-05-01 16:56) - PLID 61909 - Patients Module > History Tab > New Add a pop-out menu to the “Create Summary of Care…” 2. Customize…
void CHistoryDlg::OnCreateCCDACustomized()
{
	try{
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if (!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		// (a.walling 2014-05-09 10:20) - PLID 61788
		// (r.gonet 05/08/2014) - PLID 61805 - Generating the clinical summary from the history tab means it is neither related to an EMN nor a PIC.
		CSelectCCDAInfoDlg dlg(ctSummaryOfCare, -1, -1, GetActivePersonID(), this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHistoryDlg::OnClickImportAndAttach()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		//JMJ 12/4/2003 - Added a row for "Commonly Attached" which contains *.doc, *.xls, *.jpg, 
		//*.bmp, *.pcx, *.tiff, *.wav.  This is the default selected item, so that users can quickly add things
		//without changing the file type.
		//	Also changed the description of the other items to include the types they show
		//PLID 18882 - added pdf's to commonly attached and on their own line
		// (a.walling 2007-07-19 11:22) - PLID 26748 - Added office 2007 files (docx, xlsx) also jpeg
		// (a.walling 2007-09-17 16:11) - PLID 26748 - Also need PowerPoint 2007 files.
		// (a.walling 2007-10-12 14:50) - PLID 26342 - Moved to a shared string, and also include other formats
		ImportAndAttach("", szCommonlyAttached);
	} NxCatchAll("Error in CHistoryDlg::OnClickImportAndAttach");
}

// (a.walling 2009-05-05 17:04) - PLID 34176 - Generic import and attach function
// (a.walling 2009-05-07 10:31) - PLID 34179 - Return files, mailIDs
// (a.walling 2009-05-13 16:41) - PLID 34243 - support switching from other patients
// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
void CHistoryDlg::ImportAndAttach(const CString& strSelectionOverride, const char* szExtensionFilter, CStringArray* psaFiles, CArray<long, long>* parMailIDs, CArray<long, long>* parPatientIDs, long* pnSwitchToPatientID)
{
	if (pnSwitchToPatientID) {
		*pnSwitchToPatientID = -1;
	}
	CString path;

	// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, szExtensionFilter, this);
	// (a.walling 2008-01-29 14:33) - PLID 28716 - "Attach History Path" preference is now deprecated.
	dlg.m_ofn.lpstrInitialDir = NULL;
	//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.

	//char * strFile = new char[5000]; // bad!
	char strFile[5000];
	strFile[0] = 0;
	dlg.m_ofn.nMaxFile = 5000;
	dlg.m_ofn.lpstrFile = strFile;
	if (dlg.DoModal() == IDOK)
	{
		CString strMessage;		
		POSITION p = dlg.GetStartPosition();
		CString strSelection = strSelectionOverride.IsEmpty() ? SELECTION_FILE : strSelectionOverride;
		CMap<long, long, CString, CString&> mapIDsToPaths;
		while (p) {
			CString strSourcePath = dlg.GetNextPathName(p);

			long nPatientID = -1;
			CString strDstPath;
			// (j.gruber 2010-01-22 13:49) - PLID 23693 - moved to check pathname
			long nCategoryID = CheckAssignCategoryID(IsImageFile(strSourcePath));
			if (strSelectionOverride == SELECTION_CCD) {
				try {

					// (j.gruber 2013-11-08 10:07) - PLID 59376 - this may be a ccda instead of a ccd
					if (NxXMLUtils::IsCCDAFile(strSourcePath))
					{
						//change the selection, but let the rest go through
						strSelection = SELECTION_CCDA;						
					}
					//TES 11/14/2013 - PLID 57415 - Could also be a Cancer Case submission
					else if(NxXMLUtils::IsCancerCaseFile(strSourcePath))
					{
						strSelection = SELECTION_CANCERCASE;
					}
					else {

						// (j.jones 2010-06-30 12:12) - PLID 38031 - added clean validation for IsCCDFile(),
						// this used to throw an exception if you imported something generic
						if(!NxXMLUtils::IsCCDFile(strSourcePath)) {
							AfxMessageBox("This is not a valid CCD, CDA, or CCR document.");
							return;
						}
					}
					nPatientID = CCD::ReviewAndMatchPatient(strSourcePath, m_id, this);
					
				} NxCatchAllCall("Error reviewing CCD/CCR document", { break; });
				if (nPatientID == -1) {
					// cancel out of everything
					break;
				}

				if (!mapIDsToPaths.Lookup(nPatientID, strDstPath)) {
					strDstPath = GetPatientDocumentPath(nPatientID) ^ GetFileName(strSourcePath);
					mapIDsToPaths.SetAt(nPatientID, strDstPath);
				}
			} else {
				nPatientID = m_id;
				strDstPath = GetCurPatientDocumentPath() ^ GetFileName(strSourcePath);
			}			

			if (IsWaveFile(strSourcePath))
			{
				// (c.haag 2006-10-09 09:32) - PLID 22679 - If the file has a .wav extension, then import it with the audio icon
				strSelection = SELECTION_AUDIO;
			}
			if (CopyFile(strSourcePath, strDstPath, TRUE)) {
				long nNewMailID = AttachFileToHistory(strDstPath, nPatientID, GetSafeHwnd(), nCategoryID, strSelection, NULL);
				if (psaFiles) {
					psaFiles->Add(strDstPath);
				}
				if (parMailIDs) {
					parMailIDs->Add(nNewMailID);
				}
				// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
				if (parPatientIDs) {
					parPatientIDs->Add(nPatientID);
				}
				if (pnSwitchToPatientID != NULL && nPatientID != -1 && nPatientID != m_id) {
					*pnSwitchToPatientID = nPatientID;
				}
			}
			else {
				//If the file already exists here, prompt for rename/cancel
				DWORD dwLastError = GetLastError();
				if(dwLastError == ERROR_FILE_EXISTS) {
					strDstPath = GetCurPatientDocumentPath();
					CRenameFileDlg dlgRename(strSourcePath, strDstPath, this);
					try {
						if(dlgRename.DoModal() == IDOK) {
							strDstPath = dlgRename.m_strDstFullPath;
							if(CopyFile(strSourcePath, strDstPath, TRUE)) {
								long nNewMailID = AttachFileToHistory(strDstPath, nPatientID, GetSafeHwnd(), nCategoryID, strSelection, NULL);
								if (psaFiles) {
									psaFiles->Add(strDstPath);
								}
								if (parMailIDs) {
									parMailIDs->Add(nNewMailID);
								}
								// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
								if (parPatientIDs) {
									parPatientIDs->Add(nPatientID);
								}
								if (pnSwitchToPatientID != NULL && nPatientID != -1 && nPatientID != m_id) {
									*pnSwitchToPatientID = nPatientID;
								}
							}
						}
						else {
							if(p) {
								//There are more files to import, see if they want to keep going.
								if(IDYES != MsgBox(MB_YESNO, "Would you like to contine with the import?")) {
									break;
								}
							}
						}
						dwLastError = GetLastError();
					}NxCatchAll("GlobalUtils::ImportAndAttachFolder:RenamingFile");
				}
				//there's an error other than the file already existing
				if(dwLastError != ERROR_SUCCESS) {
					CString strError;
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
					if(IDYES != MsgBox(MB_YESNO, "The file '%s' could not be imported into the patient's documents folder. Windows returned the following error message:\r\n\r\n"
						"%s\r\n"
						"Would you like to continue?", GetFileName(strSourcePath), strError)) {
							strError.ReleaseBuffer(); // fixed memory leak
						break;
					} else {
						strError.ReleaseBuffer(); // fixed memory leak
					}
				}
			}
		}
	}
	//delete [] strFile;
}

void SendFileToRemoteServer(const CString &strFromPath, const CString &strToRemoteServerIP, const CString &strToPathRelativeToRemoteServer)
{
	NxShellExtSendToLib::INxShellExtSendToContextMenuPtr pnsestcm(__uuidof(NxShellExtSendToLib::NxShellExtSendToContextMenu));
	// We rely on NxShellExtSendTo to know what the server IP is
	pnsestcm->SendFile(1, (LPCTSTR)strFromPath, (LPCTSTR)strToRemoteServerIP, (LPCTSTR)strToPathRelativeToRemoteServer, (LPCTSTR)(GetFileName(strToPathRelativeToRemoteServer) + " on the remote server"), "");
}

void CHistoryDlg::OnSendToServer()
{
	// Only allowed if we're in a nextech internal license mode
	if (IsNexTechInternal()) {
		long nCurSel = m_pDocuments->GetCurSel();
		if (nCurSel != sriNoRow) {
			// Get this entry's full path
			CString strPath = 
				VarString(m_pDocuments->GetValue(nCurSel, phtdlcFilePath)) ^ 
				VarString(m_pDocuments->GetValue(nCurSel, phtdlcFilename));
			// Get the shared path
			CString strShared = GetSharedPath();
			long nLenSharedPath = strShared.GetLength();
			// See if this entry's full path BEGINS with the shared path
			if (strnicmp(strShared, strPath, nLenSharedPath) == 0) {
				// The path is good, send the file
				CString strRemoteServerIP = NxRegUtils::ReadString(_T("HKEY_LOCAL_MACHINE\\Software\\NexTech\\RemoteServerIP"));
				CString strRemoteSharedPath = NxRegUtils::ReadString(_T("HKEY_LOCAL_MACHINE\\Software\\NexTech\\RemoteServerSharedPath"));
				if (!strRemoteServerIP.IsEmpty() && !strRemoteSharedPath.IsEmpty()) {
					SendFileToRemoteServer(strPath, strRemoteServerIP, GetFilePath(strRemoteSharedPath ^ strPath.Mid(nLenSharedPath)));
				} else {
					// Our registry's not set up properly.  We MUST have a shared path on the remote server.
					MessageBox(
						"Files cannot be transferred from this machine to the remote server because the "
						"remote server shared path is unknown.  Please set the 'RemoteServerSharedPath' "
						"registry value and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
				}
			} else {
				// It's not a relative path, so right now these files can't be sent because the absolute 
				// path may not even exist on the remote server (like let's say this was a file in the 
				// local computer's F: drive and the server doesn't have an F: drive?)
				MessageBox(
					"The file you are trying to send has a path that is absolute on this machine, so it "
					"cannot be transferred to the remote server.  You may send the file manually, or you may "
					"detach the file from its absolute path and re-attach it in this patient's official "
					"documents folder and then send the file again.", NULL, MB_OK|MB_ICONEXCLAMATION);
			}
		} else {
			// There is no current selection
			MessageBox("You have chosen to send a file to the server but no file is currently selected.  Please select a file you would like to send and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} else {
		// There is no current selection
		MessageBox("Remote file transfer is only available under NexTech Internal license mode.  Please ensure you are logged in correctly using the official shared license file and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
	}
}

//Attaches the currently selected row
void CHistoryDlg::OnAttachFile()
{
	try {
		//This is called from the menu, and is only valid if the item is not already attached
		long nCurSel = m_pDocuments->GetCurSel();
		if(nCurSel == -1)
			return;

		// (j.jones 2009-12-18 08:31) - PLID 34606 - check permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		long p = m_pDocuments->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		CList<long,long> listRowsToRemove;		
		while (p)
		{	
			m_pDocuments->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			//this should never fail, but just in case...
			_variant_t var = pRow->GetValue(phtdlcStaff);

			//if the staff value is filled in, then it must have been attached by that person.  If it
			//is not, then it's an unattached item

			if(var.vt == VT_NULL || var.vt == VT_EMPTY) {

				CString strFile;
				CString strSelection = SELECTION_FILE;
				var = pRow->GetValue(phtdlcFilePath);

				strFile = VarString(var);				

				var = pRow->GetValue(phtdlcFilename);

				strFile = strFile ^ VarString(var);

				// (j.gruber 2010-01-22 13:50) - PLID 23693 - moved to take filename into account
				long nCategoryID = CheckAssignCategoryID(IsImageFile(strFile));

				// (c.haag 2006-10-09 09:32) - PLID 22679 - If the file has a .wav extension, then import it with the audio icon
				if (IsWaveFile(strFile)) {
					strSelection = SELECTION_AUDIO;
				}

				if(AttachFileToHistory(strFile, GetActivePersonID(), GetSafeHwnd(), nCategoryID, strSelection)) {
					//This row will have been added at the end of the list.
					bool bAdded = false;
					POSITION pos = listRowsToRemove.GetHeadPosition();
					POSITION pPrev = NULL;
					while(pos && !bAdded) {
						pPrev = pos;
						long nRowIndex = listRowsToRemove.GetNext(pos);
						if(nRowIndex < pRow->GetIndex()) {
							listRowsToRemove.InsertBefore(pPrev, pRow->GetIndex());
							bAdded = true;
						}
					}
					if(!bAdded) {
						listRowsToRemove.AddTail(pRow->GetIndex());
					}

				}
			}
		}
		POSITION pos = listRowsToRemove.GetHeadPosition();
		while(pos) {
			long nRowIndex = listRowsToRemove.GetNext(pos);
			m_pDocuments->RemoveRow(nRowIndex);
		}


	} NxCatchAll("Error in CHistoryDlg::OnAttachFile()");
}

void CHistoryDlg::OnDestroy() 
{
	try {

		if (GetPicContainer()) {
			// (a.walling 2008-09-03 12:20) - PLID 19638 - Un-register for table checker messages
			GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		}

		//we want to save the primary sort column, and whether it is ascending or descending
		for(int i = 0; i < m_pDocuments->GetColumnCount(); i++) {
			IColumnSettingsPtr pCol = m_pDocuments->GetColumn(i);
			short nPri = VarShort(pCol->GetSortPriority());
			if(nPri == 0)
				break;
		}

		if(i < m_pDocuments->GetColumnCount()) {
			//save this column, and this asc/desc
			BOOL bAscending = FALSE;
			if(m_pDocuments->GetColumn(i)->GetSortAscending())
				bAscending = TRUE;

			//now save these 2 things
			SetRemotePropertyInt("HistorySortColumn", i, 0, GetCurrentUserName());
			long nAsc;
			if(bAscending)
				nAsc = 1;
			else
				nAsc = 0;

			SetRemotePropertyInt("HistorySortAsc", nAsc, 0, GetCurrentUserName());

		}

		m_DropTargetTabs.Revoke();

		// (z.manning 2008-07-01 14:45) - PLID 25574 - Destory the EMR popup dialog
		if(m_pdlgEmrPreview != NULL) {
			m_pdlgEmrPreview->DestroyWindow();
			delete m_pdlgEmrPreview;
			m_pdlgEmrPreview = NULL;
		}

		DestroyIcon(m_hiconEmn);
		m_hiconEmn = NULL;

		// (a.walling 2013-10-02 10:02) - PLID 58847 - map of icons no longer necessary, FileUtils maintains a cache by extension
	} NxCatchAll("Error in CHistoryDlg::OnDestroy() ");

	CNxDialog::OnDestroy();
}

//DRT 7/2/2008 - PLID 30601 - I pulled this code out of OnEmailFile so others could share it.  I didn't write
//	anything here, though I did slightly clean it up and add some comments.
void CHistoryDlg::GetSelectedFiles(CStringArray& aryFileNames, CStringArray& aryFilePaths)
{
	// (a.walling 2015-04-02 11:22) - PLID 62227 - If no selection, it was lost while a menu or messagebox was up. Get out of here.
	if (m_pDocuments->CurSel == -1) {
		MessageBox("No files are currently selected.");
		return;
	}

	// (m.hancock 2006-05-23 13:53) - PLID 20736 - Emailing packets from History does not send all files in the packet.
	// We need to check if a packet has been selected for emailing.
	if((m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcName)).vt == VT_I4) {
		
		// Now we need to retrieve all the document records for the packet so we can assemble a list of attached files
		_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, Selection, PathName, PersonID FROM MailSent WHERE MergedPacketID = (SELECT MergedPacketID FROM MailSent WHERE MailID = %li) AND PersonID = %li ORDER BY MailID ASC",
			VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID)), m_id);
		
		// If no records were returned, we do not have a valid packet, so display an error.
		if(rsMailSent->eof) {
			MsgBox("Packet not found.");
			return;
		}

		// Now we need to assemble a listing of attachments for the email.
		while(!rsMailSent->eof) {

			CString strFile = AdoFldString(rsMailSent, "PathName");
			if (strFile.Find('\\') == -1) {
				// The filename doesn't have a backslash, so it's just a file, but we need the full path.
				// So, we need to add the patient's shared documents path to the file's location.
				// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
				strFile = GetCurPatientDocumentPath() ^ strFile;
			}

			// Add the file's location to our listing of attachments and move to the next attachment
			aryFilePaths.Add(strFile);
			rsMailSent->MoveNext();
		}
	}

	// (m.hancock 2006-05-23 15:01) - PLID 20736 - Added an else block to handle all other cases when we're not
	// emailing a packet.
	else {
		long pSel = m_pDocuments->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		while(pSel) {
			m_pDocuments->GetNextSelEnum(&pSel, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			aryFileNames.Add((LPCTSTR)(_bstr_t(pRow->GetValue(phtdlcPath))));
			aryFilePaths.Add(CString(pRow->GetValue(phtdlcFilePath).bstrVal) + "\\" + CString(pRow->GetValue(phtdlcFilename).bstrVal));

			//TES 11/6/2007 - PLID 27981 - VS2008 - I can only assume the below code, meant to check whether the 
			// string's length was less than 4.  However, there's no reason that a filename should be 4 or more
			// characters (the confusion may have arisen from other lines of code, that check that a string is
			// at least 4 characters long, but in those cases they are checking for a specific file extension).
			// So, I'm just commenting this block out; that shouldn't change the behavior anyway.
			/*if (strFilePaths.GetAt(strFilePaths.GetSize()-1) < 4) {
				MsgBox("There appears to be no valid file attached to this record (" 
					+ strFileNames.GetAt(strFileNames.GetSize()-1) +"). The email will not be sent.");
				return;
			}*/
			if(!DoesExist(aryFilePaths.GetAt(aryFilePaths.GetSize()-1))) {
				// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
				MessageBox("The attached file '" + aryFileNames.GetAt(aryFileNames.GetSize()-1) 
					+ "' cannot be found. It either no longer exists, or you do not have access to the network location in which it is saved.\n"
					"The document will not be sent.", NULL, MB_ICONINFORMATION);
				return;
			}

			pDisp->Release();
		}
	}
}

// (c.haag 2010-10-21 12:36) - PLID 27633 - Returns TRUE if any files IN astrFilePaths could not be found
BOOL CHistoryDlg::AreAnyFilesMissing(const CStringArray& astrFilePaths)
{
	CString strMsg = "The following files could not be found on your network:\r\n\r\n";
	long nFilesMissing = 0;

	// Build a list of up to twenty missing files; put the full paths into a message box.
	// (Full paths are much more helpful for debugging than just file names)
	for (int i=0; i < astrFilePaths.GetSize(); i++) {
		CString strPath = astrFilePaths[i];
		if (!FileUtils::DoesFileOrDirExist(strPath)) 
		{
			if (nFilesMissing >= 20) {
				strMsg += "<more>\r\n";
				break;
			}
			else {
				strMsg += strPath + "\r\n";
				nFilesMissing++;
			}
		}
	}

	if (nFilesMissing > 0) {
		strMsg += "\r\nThe operation cannot continue until this is resolved.";
		MessageBox(strMsg, "Practice", MB_OK | MB_ICONSTOP);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void CHistoryDlg::OnEmailFile()
{
	try {

		if(m_pDocuments->CurSel == -1)
			return;

		CStringArray strFileNames;
		CStringArray strFilePaths;

		//DRT 7/2/2008 - PLID 30601 - Moved the block of code here into its own function to be used by others.
		GetSelectedFiles(strFileNames, strFilePaths);
		// (c.haag 2015-11-03) - PLID 67512 - Test that both arrays are empty because in the case of packets,
		// strFileNames is not populated.
		if (strFileNames.IsEmpty() && strFilePaths.IsEmpty())
		{
			return;
		}

		// (c.haag 2010-10-21 12:36) - PLID 27633 - Quit if any files are missing
		if (AreAnyFilesMissing(strFilePaths))
			return;

		_RecordsetPtr rs = CreateRecordset("SELECT Email, PrivEmail FROM PersonT WHERE Email <> '' AND PersonT.ID = %li",m_id);
		if(!rs->eof) {
			CString strEmailAddress = AdoFldString(rs, "Email","");
			if(strEmailAddress != "") {

				if(AdoFldBool(rs, "PrivEmail")) {
					if(MsgBox(MB_YESNO, "This patient is set for privacy on their email.  Are you sure you wish to send this file?") == IDNO)
						return;
				}
				if (SendEmail(this, strEmailAddress, "Patient documents", "", strFilePaths)) {
					// (c.haag 2009-03-10 12:58) - PLID 30019 - Have a record of the fact the user sent the e-mail
					CString strNote;
					COleDateTime dtNow = COleDateTime::GetCurrentTime();
					strNote.Format("Email created for '%s' on %s.  Subject:  '%s'", strEmailAddress, FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime, false), "Patient documents");
					// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
					// Obviously this will contradict strNote if the server/workstation times are different, but if a new MailSent date is to be "now", then it
					// needs to be the server's "now". If the note becomes an issue, we can address it in a future item.
					CreateNewMailSentEntry(GetActivePatientID(), strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID());
				}
			}
		}
		rs->Close();

	}NxCatchAll("Error emailing file to patient.");
}

//DRT 7/2/2008 - PLID 30601 - Right click menu handler for sending a fax with the documents.
void CHistoryDlg::OnSendFax()
{
	try {
		//DRT 8/1/2008 - PLID 30915 - Added licensing
		if(!g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrUse)) {
			return;
		}

		CStringArray aryFileNames;
		CStringArray aryFilePaths;

		//Retrieve all the currently selected files.
		GetSelectedFiles(aryFileNames, aryFilePaths);
		// (c.haag 2015-11-03) - PLID 67512 - Only test aryFilePaths because aryFileNames is not used
		if (aryFilePaths.IsEmpty())
		{
			return;
		}

		// (c.haag 2010-10-21 12:36) - PLID 27633 - Quit if any files are missing
		if (AreAnyFilesMissing(aryFilePaths))
			return;

		CFaxChooseRecipientDlg dlgRecip(this);
		dlgRecip.m_nPersonID = m_id;
		if(dlgRecip.DoModal() != IDOK) {
			//They cancelled the fax
			return;
		}

		//(e.lally 2011-10-31) PLID 41195 - Pass in the person ID for any mailsent entries
		CFaxSendDlg dlg(this);
		dlg.BeginFax(dlgRecip.m_strNumber, dlgRecip.m_strName, aryFilePaths, m_id);

	} NxCatchAll("Error in OnSendFax");
}

void CHistoryDlg::OnPrintFile()
{
	if(m_pDocuments->CurSel == -1)
		return;

	try {
		_variant_t varName = m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcName);
		if(varName.vt == VT_I4) {
			//DRT 8/3/2005 - PLID 17153 - This is a packet, we need to print each document, not just the first
			//find the path of all the documents in this packet
			_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, PathName FROM MailSent WHERE MergedPacketID = (SELECT MergedPacketID FROM MailSent WHERE MailID = %li) ORDER BY MailID ASC", VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID)));
			while(!rsMailSent->eof) {
					
				//PLID 18609: check to see if it is a multipatdoc
				CString strFile = AdoFldString(rsMailSent, "PathName");
				CString strFilePath = VarString(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcFilePath), "");
					
				//check to see if the path exists
				if (! DoesExist(strFilePath ^ strFile)) {

					//see if it is a multipatdoc
					if (strFile.Find("MultiPatDoc") != -1) {
						//change the path to be the --25 path
						strFilePath = GetSharedPath() ^ "Documents\\---25\\";
					}
				}
				if(!PrintFile(strFilePath ^ strFile)) 
					//If we had a failure on any one document, quit the whole thing
					return;
				rsMailSent->MoveNext();
			}
		}
		else {
			//This is your standard document to be printed
				
			//PLID 18609: check to see if it is a multipatdoc
			CString strFilePath = VarString(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcFilePath));
			CString strDocName = VarString(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcFilename));
			//see if it exists in the patient file
			if (! DoesExist(strFilePath ^ strDocName)) {

				//see if it is a multipatdoc
				if (strDocName.Find("MultiPatDoc") != -1) {
					//change the path to be the --25 path
					strFilePath = GetSharedPath() ^ "Documents\\---25\\";
				}
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
			PrintFile( strFilePath ^ strDocName);
		}
	}
	NxCatchAll("Error printing file");
}

void CHistoryDlg::EnableAppropriateButtons()
{
	try {
		//Detach button
		BOOL bEnable = FALSE;
		// (e.lally 2009-06-11) PLID 34600 - Get delete permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(GetCurrentUserPermissions(GetHistoryPermissionObject()) & (sptDelete|sptDeleteWithPass)){
			if(m_PhotoViewer.IsWindowVisible()) {
				bEnable = m_bPhotoViewerDetachable;
			}
			else {
				//TES 2/23/2004: If there is a single highlighted row with a valid value in "Staff" (meaning the file is attached), 
				//then enable this button.
				for(int i = 0; i < m_pDocuments->GetRowCount() && !bEnable; i++) {
					// (z.manning 2008-07-03 15:33) - PLID 25574 - Don't count EMN rows
					if(((IRowSettingsPtr)m_pDocuments->GetRow(i))->IsHighlighted() && m_pDocuments->GetValue(i,phtdlcStaff).vt != VT_EMPTY && m_pDocuments->GetValue(i,phtdlcStaff).vt != VT_NULL && VarLong(m_pDocuments->GetValue(i,phtdlcEmnID),-1) == -1) {
						bEnable = TRUE;
					}
				}
			}
		}
		GetDlgItem(IDC_DETACH)->EnableWindow(bEnable);

		
		//Record Audio button
		bEnable = FALSE;
		// (e.lally 2009-06-11) PLID 34600 - Get write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if (GetCurrentUserPermissions(GetHistoryPermissionObject()) & (sptWrite | sptWriteWithPass)) {
			bEnable = TRUE;
		}
		GetDlgItem(IDC_BTN_RECORD_AUDIO)->EnableWindow(bEnable);
	} NxCatchAll(__FUNCTION__);
}

void CHistoryDlg::OnSelChangedDocuments(long nNewSel) 
{
	try
	{
		EnableAppropriateButtons();	

		if(m_pdlgEmrPreview != NULL && m_pdlgEmrPreview->IsWindowVisible()) {

			// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
			// of all available EMN IDs, and the index of the array representing the EMN
			// we wish to view
			// (z.manning 2012-09-10 14:33) - PLID 52543 - Use the new EmnPreviewPopup struct
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
			long nIndex = -1;

			//find all EMNIDs in the list
			for(int i = 0; i < m_pDocuments->GetRowCount(); i++) {
				long nEMNID = VarLong(m_pDocuments->GetValue(i, phtdlcEmnID), -1);
				if(nEMNID != -1) {
					aryEMNs.Add(EmnPreviewPopup(nEMNID, VarDateTime(m_pDocuments->GetValue(i, phtdlcEmnModifiedDate))));
					//if this is the row we just clicked on, set that as the index
					if(nNewSel == i) {
						nIndex = aryEMNs.GetSize() - 1;
					}
				}
			}
			
			m_pdlgEmrPreview->PreviewEMN(aryEMNs, nIndex);
		}

	}NxCatchAll("CHistoryDlg::OnSelChangedDocuments");
}

void CHistoryDlg::OnTrySetSelFinishedDocuments(long nRowEnum, long nFlags) 
{
	EnableAppropriateButtons();
}

void CHistoryDlg::OnEditHistoryCats() 
{
	try {

		CNoteCategories	dlg(this);
		dlg.m_bEditingHistoryCat = true;
		dlg.DoModal();

		RefreshTabs();
		//have to requery the list in case something was added
		UpdateView();
	
	}NxCatchAll("Error in OnEditHistoryCats");
}

void CHistoryDlg::OnUseHistoryFilter() 
{
	try {
		if(IsDlgButtonChecked(IDC_USE_HISTORY_FILTER)) {
			m_PhotoViewer.SetUsingCategoryFilter(true);

			GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(TRUE);
			long nRow = m_pFilter->SetSelByColumn(0, (long)ID_ALL_CATS);
			if(nRow > -1)
				OnSelChosenHistoryFilterList(nRow);
			else {
				//shouldn't be possible, but if we somehow get no rows, we're going to be confused
				ASSERT(FALSE);
			}
		}
		else {
			m_PhotoViewer.SetUsingCategoryFilter(false);
			GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(FALSE);
			UpdateView();
		}
	} NxCatchAll("Error in CHistoryDlg::OnUseHistoryFilter()");
}

void CHistoryDlg::OnSelChangingHistoryFilterList(long FAR* nNewSel) 
{
	//reset to the first row if they try to select nothing
	if(*nNewSel == -1) {
		*nNewSel = 0;
	}
}

void CHistoryDlg::OnSelChosenHistoryFilterList(long nRow) 
{
	if(nRow == -1) {
		//shouldnt be able to happen, but just in case....
		ASSERT(FALSE);
		m_pFilter->PutCurSel(0);
		nRow = 0;
	}

	//get the value to see if it's a multiple
	if(VarLong(m_pFilter->GetValue(nRow, 0)) == ID_MULTIPLE_CATS) {
		//do the multiple thing
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "NoteCatsF");
		dlg.PreSelect(m_aryFilter);

		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		if(dlg.Open("NoteCatsF", GetAllowedCategoryClause("ID"), "ID", "Description", "Select a category:") == IDOK) {
			//we need to setup a filter for it
			dlg.FillArrayWithIDs(m_aryFilter);

			// Adjust our selection based on the array contents
			if (m_aryFilter.GetSize() == 0)
				m_nCurrentFilterID = ID_ALL_CATS;
			else if (m_aryFilter.GetSize() == 1)
			{
				m_nCurrentFilterID = m_aryFilter[0];
				m_aryFilter.RemoveAll();
			}
			else
				m_nCurrentFilterID = ID_MULTIPLE_CATS;
		}
		else {
			//cancelled it, we don't want multiple selection
			if(m_nCurrentFilterID != ID_MULTIPLE_CATS) {
				//we hit cancel, and are going to set the sel to something other than multiple, so 
				//wipe out the array
				m_aryFilter.RemoveAll();
			}
		}
		//reset the selection
		m_pFilter->SetSelByColumn(0, (long)m_nCurrentFilterID);
	}
	else
	{
		//save the current selection
		m_nCurrentFilterID = VarLong(m_pFilter->GetValue(m_pFilter->GetCurSel(), 0));
	}

	//update the view
	UpdateView();
}

void CHistoryDlg::OnSelectTabDocTabs(short newTab, short oldTab) 
{
	if (m_bDragging) {
		LockWindowUpdate();
	}

	if(newTab == 0) {
		// (b.cardillo 2006-04-10 17:41) - PLID 20072 - Hide one window and show the other at the same time
		ShowAndHideWindows(1, 1, GetDlgItem(IDC_PHOTO_VIEWER)->GetSafeHwnd(), GetDlgItem(IDC_DOCUMENTS)->GetSafeHwnd());

		//DRT 5/25/2004 - PLID 12568 - If we're showing the photo tab, disable the "show unattached..."
		GetDlgItem(IDC_ENABLE_UNATTACHED)->EnableWindow(FALSE);
		GetDlgItem(IDC_USE_HISTORY_FILTER)->EnableWindow(TRUE);
		if(IsDlgButtonChecked(IDC_USE_HISTORY_FILTER))
			GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMEMBER_HISTORY_COLUMNS)->EnableWindow(FALSE);

		// (b.cardillo 2005-06-28 17:32) - PLID 16454 - And show the photo sort controls
		GetDlgItem(IDC_PHOTOS_SORT_DESCENDING_CHECK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PHOTOS_SORT_CRITERION_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PHOTOS_SORT_BY_LABEL)->ShowWindow(SW_SHOW);
		//(e.lally 2010-01-12) PLID 25808 - Added quick button for photo note printing preference, just on photos subtab
		GetDlgItem(IDC_PHOTOS_PRINT_NOTES_CHECK)->ShowWindow(SW_SHOW);
	}
	else if(newTab == m_tab->GetSize()-1) {
		// (b.cardillo 2006-04-10 17:41) - PLID 20072 - Hide one window and show the other at the same time
		ShowAndHideWindows(1, 1, GetDlgItem(IDC_DOCUMENTS)->GetSafeHwnd(), GetDlgItem(IDC_PHOTO_VIEWER)->GetSafeHwnd());

		GetDlgItem(IDC_ENABLE_UNATTACHED)->EnableWindow(TRUE);
		GetDlgItem(IDC_USE_HISTORY_FILTER)->EnableWindow(TRUE);
		if(IsDlgButtonChecked(IDC_USE_HISTORY_FILTER))
			GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMEMBER_HISTORY_COLUMNS)->EnableWindow(TRUE);

		// (b.cardillo 2005-06-28 17:32) - PLID 16454 - And hide the photo sort controls
		GetDlgItem(IDC_PHOTOS_SORT_DESCENDING_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_CRITERION_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_BY_LABEL)->ShowWindow(SW_HIDE);
		//(e.lally 2010-01-12) PLID 25808
		GetDlgItem(IDC_PHOTOS_PRINT_NOTES_CHECK)->ShowWindow(SW_HIDE);
	}
	else {
		//ACW 6/21/04 - PLID 13124 - If we're not showing the Miscellaneous tab, then the filter option 
		//should be disabled.
		// (b.cardillo 2006-04-10 17:41) - PLID 20072 - Hide one window and show the other at the same time
		ShowAndHideWindows(1, 1, GetDlgItem(IDC_DOCUMENTS)->GetSafeHwnd(), GetDlgItem(IDC_PHOTO_VIEWER)->GetSafeHwnd());

		GetDlgItem(IDC_ENABLE_UNATTACHED)->EnableWindow(TRUE);
		GetDlgItem(IDC_USE_HISTORY_FILTER)->EnableWindow(FALSE);
		GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMEMBER_HISTORY_COLUMNS)->EnableWindow(TRUE);

		// (b.cardillo 2005-06-28 17:32) - PLID 16454 - And hide the photo sort controls
		GetDlgItem(IDC_PHOTOS_SORT_DESCENDING_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_CRITERION_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PHOTOS_SORT_BY_LABEL)->ShowWindow(SW_HIDE);
		//(e.lally 2010-01-12) PLID 25808
		GetDlgItem(IDC_PHOTOS_PRINT_NOTES_CHECK)->ShowWindow(SW_HIDE);
	}
	UpdateView();

	if(m_bThemeOpen) {
		//Windows XP Theme mode.  For now, we're not going to do anything different here
	}
	else {
		//DRT 5/27/2004 - PLID 12564
		//Windows Classic mode - Draw the text highlighted, otherwise it's too hard
		//	to see what tab we are currently on

		//reset the color of the previous tab
		// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
		//m_tab->ResetTabBackColor(oldTab);
		//and set the new selection to white
		//m_tab->SetTabBackColor(m_tab->GetCurSel(), TAB_HIGHLIGHT_COLOR);
	}

	// (a.wetta 2006-09-28 09:50) - If we're dragging and dropping, we need to refresh the screen
	if (m_bDragging) {
		m_bDragDropNeedToRefresh = TRUE;
		UnlockWindowUpdate();
	}
}
void CHistoryDlg::ResetColumnSizes()
{
	//JMM 2/23/04 Copied from AppointmentsDlg.cpp and modified slightly to reflect the unattached checkbox
	//TES 2/9/04 - Plagiarized from ToDoAlarm.
	//DRT - 6/6/03 - This function takes the saved m_strOriginalColSizes and
	//		resets all columns to those sizes.

	if(m_strOriginalColSizes.IsEmpty()) {
		//not sure why we wouldn't have any, but better to leave them as is
		//than to set to empty
		return;
	}

	CString strOrig, str;
	for(int j = 0; j < m_pDocuments->GetColumnCount(); j++) {
		IColumnSettingsPtr pCol = m_pDocuments->GetColumn(j);
		str.Format("%li,", pCol->GetStoredWidth());

		strOrig += str;
	}

	CString strCols = strOrig;
	int nWidth = 0, i = 0;

	//parse the columns out and set them
	int nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pDocuments->GetColumn(i);
		if(pCol) {
			pCol->PutStoredWidth(nWidth);
		}

		i++;
		nComma = strCols.Find(",");
	}

}

void CHistoryDlg::SetColumnSizes()
{
	//JMM 2/23/04 Copied from AppointmentsDlg.cpp and modified slightly to reflect the unattached checkbox
	//TES 2/9/04: Plagiarized from ToDoAlarm
	//DRT - 6/6/03 - This function takes the saved column sizes out of ConfigRT
	//		IF the box is checked.

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_HISTORY_COLUMNS)) {
		return;
	}

	CString strCols;
	// (a.walling 2006-11-29 10:23) - PLID 23681 - Added another column, and added 0 to the end of the default sizes.
	//		this should not interfere with preexisting saved sizes at all.
	// (a.walling 2008-09-15 16:40) - PLID 23904 - Added another column, 0 at the end, this is the actual Name column, whereas we now have a new
	// Icon column in its place (and inheriting its size)
	if (IsDlgButtonChecked(IDC_ENABLE_UNATTACHED)) {
		strCols = GetRemotePropertyText("DefaultHistoryUnAttachedColumnSizes", "0,80,0,100,100,150,125,125,125,371,0,0,", 0, GetCurrentUserName(), TRUE);
	}
	else {
		strCols = GetRemotePropertyText("DefaultHistoryAttachedColumnSizes", "0,80,0,0,0,370,125,125,125,371,0,0,", 0, GetCurrentUserName(), TRUE);
	}

	if(strCols.IsEmpty()) strCols = m_strOriginalColSizes;

	if(!strCols.IsEmpty()) {
		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_pDocuments->GetColumn(i);
			if(pCol)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}
}

void CHistoryDlg::OnRememberColumns() 
{
	//(e.lally 2006-07-24) PLID 21589 - We need to move this code into its own Remember_Columns function
	//so that we can call that function in other places in code without
	//doing the update view.
	RememberColumns();
	UpdateView();
}

void CHistoryDlg::RememberColumns()
{
	//save the setting
	long nRemember = 0;	//default off
	if(IsDlgButtonChecked(IDC_REMEMBER_HISTORY_COLUMNS))
		nRemember = 1;
	else
		nRemember = 0;
	SetRemotePropertyInt("RememberHistoryColumns", nRemember, 0, GetCurrentUserName());

	//size the datalist appropriately
	if(!IsDlgButtonChecked(IDC_REMEMBER_HISTORY_COLUMNS)) {
		ResetColumnSizes();
		m_pDocuments->SetRedraw(TRUE);
	}
	else {
		if(m_tab->CurSel != 0)
			m_pDocuments->SetRedraw(FALSE);
		//Make sure all the styles are fixed-width type styles.
		for (short i=0; i < m_pDocuments->ColumnCount; i++)
		{
			long nStyle = m_pDocuments->GetColumn(i)->ColumnStyle;
			nStyle &= ~(csWidthPercent | csWidthAuto);
			m_pDocuments->GetColumn(i)->ColumnStyle = nStyle;
		}
		SetColumnSizes();
		if(m_tab->CurSel != 0)
			m_pDocuments->SetRedraw(TRUE);
	}
}

void CHistoryDlg::OnColumnSizingFinishedDocuments(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//JMM 2/23/04 Copied from AppointmentsDlg.cpp and modified slightly to reflect the unattached checkbox
	//TES 2/9/2004: Plagiarized from ToDoAlarm
	//DRT 6/6/03 - Saves the sizes of all columns if bCommitted is set and the checkbox is set

	//uncommitted
	if(!bCommitted)
		return;

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_HISTORY_COLUMNS))
		return;

	
	
	//save width of each column
	IColumnSettingsPtr pCol;
	CString str, strList;

	for(int i = 0; i < m_pDocuments->GetColumnCount(); i++) {
		pCol = m_pDocuments->GetColumn(i);
		if(pCol)
			str.Format("%li,", pCol->GetStoredWidth());

		strList += str;
	}

	//write it to ConfigRT
	if (IsDlgButtonChecked(IDC_ENABLE_UNATTACHED)) {
		SetRemotePropertyText("DefaultHistoryUnAttachedColumnSizes", strList, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyText("DefaultHistoryAttachedColumnSizes", strList, 0, GetCurrentUserName());
	}


	SetColumnSizes();
	
}

BOOL CHistoryDlg::PreTranslateMessage(MSG *pMsg) 
{
	if( (pMsg->hwnd == GetDlgItem(IDC_DOCUMENTS)->GetSafeHwnd() || pMsg->hwnd == GetDlgItem(IDC_PHOTO_VIEWER)->GetSafeHwnd())
		&& pMsg->message == WM_DROPFILES) {
		// (j.jones 2009-12-18 09:30) - PLID 34606 - post a message to drop the file, don't do it inside this message loop
		PostMessage(NXM_DROP_FILES, pMsg->wParam, pMsg->lParam);
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}

// (m.cable 6/24/2004 16:41) - this function searches the datalist for the particular nMailID
// if it finds it, then it will compare the values on that row to the values passed in
// if all of the values match, then return true;
// if the nMailID isn't found or something doesn't match, then return false
bool CHistoryDlg::DoesRowMatch(long nMailID, CString strPath, CString strNote, CString strStaff, COleDateTime dtServiceDate, long nCatID, CString strFile)
{
	long nRow = m_pDocuments->FindByColumn(phtdlcMailID, nMailID, 0, false);
	if(nRow == sriNoRow){
		// we didn't find the row
		return false;
	}
	else if
		(VarString(m_pDocuments->GetValue(nRow, phtdlcPath)) != strPath || 
		VarString(m_pDocuments->GetValue(nRow, phtdlcNotes)) != strNote || 
		VarString(m_pDocuments->GetValue(nRow, phtdlcStaff)) != strStaff || 
		VarLong(m_pDocuments->GetValue(nRow, phtdlcCategory)) != nCatID || 
		m_pDocuments->GetValue(nRow, phtdlcServiceDate) != _variant_t(dtServiceDate)){
		// if any of the fields don't match what is showing on the screen, then return false
			return false;
	}
	// looks like it's up to date
	return true;
}


LRESULT CHistoryDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		// (a.wilson 2014-08-11 08:25) - PLID 63246 - check and update note categories.
		if (wParam == NetUtils::NoteCatsF) {
			EnsureUpdatedCategoryFilter((long)lParam);
		}
		else if (wParam == NetUtils::MailSent) {
			// (j.jones 2014-08-04 14:56) - PLID 32474 - MailSent should only be sent in EX form,
			// if a regular version is sent we will not respond to it
		}
		else if(wParam == NetUtils::EMRMasterT) {
			try {
				// (z.manning 2008-07-01 16:05) - PLID 25574 - If we're showing EMNs and an EMN for this patient
				// was updated, then refresh.
				// (j.jones 2014-08-08 16:51) - PLID 32474 - ignore -1 record IDs
				if (m_btnShowEmns.GetCheck() && lParam == m_id) {
					UpdateView();
				}
			} NxCatchAll("CHistoryDlg::OnTableChanged:EMRMasterT");
		}
	}NxCatchAll("Error in CHistoryDlg::OnTableChanged");

	return 0;
}

// (j.jones 2014-08-04 14:59) - PLID 32474 - added Ex handling
LRESULT CHistoryDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {
		switch (wParam) {

		case NetUtils::MailSent:

			try {

				// (j.jones 2014-08-04 15:00) - PLID 32474 - the Ex message now gives us the MailSentID,
				// the PatientID, and IsNonPhoto. IsNonPhoto is only 1 when we know it is not a photo.
				// It would be 0 for files where the photo status is unknown, or definite photos.
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPatientID), -1);

				//if not for this patient, leave now
				if (m_id != nPatientID) {
					return 0;
				}

				//if the list is requerying, leave now
				if (m_pDocuments->IsRequerying()) {
					return 0;
				}

				long nMailSentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiMailSentID), -1);
				TableCheckerDetailIndex::MailSent_PhotoStatus ePhotoStatus = (TableCheckerDetailIndex::MailSent_PhotoStatus)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPhotoStatus), (long)TableCheckerDetailIndex::MailSent_PhotoStatus::mspsUnknown);
								
				CString strGroupBy = (LPCTSTR)m_pDocuments->GroupByClause;
				long nRow = m_pDocuments->FindByColumn(0, nMailSentID, 0, false);
				if (nRow == -2) {
					//This means the list is requerying.  So all this stuff is unnecessary.
					return 0;
				}

				//TES 9/2/2008 - PLID 6484 - m_strFromClause already handles the preference to block multi-patient 
				// documents, so this query doesn't need to change.
				// (z.manning 2010-02-02 11:39) - PLID 34287 - Added PicID
				// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading History dialog
				// (j.jones 2014-08-04 15:08) - PLID 32474 - parameterized
				_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT CASE WHEN PacketName Is Null THEN Min(Selection) ELSE NULL END AS Name, "
					"Min(PathName) AS Path, Min(PathName) AS PathName, '' AS FileName, "
					"CASE WHEN PacketName Is Null THEN Min(Note) ELSE Min(PacketName) END AS [Note], "
					"Min(Sender) AS Staff, Min(Date) AS AttachDate, Min(ServiceDate) AS ServiceDate, "
					"CASE WHEN HistoryCategoryID IS NULL THEN -1 ELSE HistoryCategoryID END AS Category, "
					"Min(Selection) AS Selection, " // (a.walling 2007-01-15 13:08) - PLID 23681 - Return the selection for auditing when deleting non-file objects
					// (a.wetta 2007-07-10 09:56) - PLID 17467 - Add the IsPhoto field
					// (a.walling 2007-07-19 17:30) - PLID 17467 - This was skipping over JPEGs
					"(CASE WHEN dbo.IsImageFile(min(PathName)) = 1 THEN (CASE WHEN IsPhoto IS NULL THEN 1 ELSE IsPhoto END) ELSE NULL END) AS IsPhoto, "
					"PicID "
					"FROM ({CONST_STRING}) AS HistoryQ "
					"WHERE MailID = {INT} "
					"GROUP BY {CONST_STRING}",
					m_strFromClause, nMailSentID, strGroupBy);

				if (prs->eof)
				{
					// The record was deleted; make sure this row doesn't exist
					if (nRow > -1) {
						m_pDocuments->RemoveRow(nRow);

						// (j.jones 2014-08-04 15:06) - PLID 32474 - if we know the file was not a photo,
						// don't refresh the photo viewer, otherwise refresh if it is definitely or
						// potentially a photo
						if (ePhotoStatus != TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto) {
							m_PhotoViewer.Refresh();
						}
						return 0;
					}
				}
				else
				{
					IRowSettingsPtr pRow;
					if (nRow == -1)
					{
						// The record was added
						pRow = m_pDocuments->GetRow(-1);
					}
					else
					{
						// The record was modified
						pRow = m_pDocuments->GetRow(nRow);
					}

					// store these in local variables so we only access the recordset once
					CString strPath = AdoFldString(prs, "Path");
					CString strNote = AdoFldString(prs, "Note");
					CString strStaff = AdoFldString(prs, "Staff");
					COleDateTime dtServiceDate = prs->Fields->Item["ServiceDate"]->Value.date;
					long nCatID = AdoFldLong(prs, "Category");

					// fill the details of the row
					pRow->Value[phtdlcMailID] = (long)(nMailSentID);
					pRow->Value[phtdlcName] = prs->Fields->Item["Name"]->Value;
					pRow->Value[phtdlcPath] = _bstr_t(strPath);
					pRow->Value[phtdlcNotes] = _bstr_t(strNote);
					pRow->Value[phtdlcExtraNotesIcon] = g_cvarNull; // (c.haag 2010-07-01 16:08) - PLID 39473
					pRow->Value[phtdlcHasExtraNotes] = g_cvarFalse; // (c.haag 2010-07-01 16:08) - PLID 39473
					pRow->Value[phtdlcStaff] = _bstr_t(strStaff);
					pRow->Value[phtdlcDate] = prs->Fields->Item["AttachDate"]->Value;
					pRow->Value[phtdlcServiceDate] = prs->Fields->Item["ServiceDate"]->Value;
					pRow->Value[phtdlcCategory] = nCatID;
					pRow->Value[phtdlcSelection] = prs->Fields->Item["Selection"]->Value;
					
					// (a.wetta 2007-07-10 09:57) - PLID 17467 - Set the IsPhoto field
					pRow->Value[phtdlcIsPhoto] = prs->Fields->Item["IsPhoto"]->Value;

					// (j.jones 2014-08-04 15:11) - PLID 32474 - if our photo status is unknown,
					// try to update it with this value, which might be more accurate
					if (ePhotoStatus == TableCheckerDetailIndex::MailSent_PhotoStatus::mspsUnknown) {
						_variant_t varIsPhoto = prs->Fields->Item["IsPhoto"]->Value;
						if (varIsPhoto.vt == VT_BOOL) {
							//non-null means we know for sure what type it is, so update our enum
							if (VarBool(varIsPhoto)) {
								ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto;
							}
							else {
								ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto;
							}
						}
						//see if it is an image file extension
						else if (IsImageFile(strPath)) {
							ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto;
						}
					}

					// (z.manning 2008-07-11 10:30) - PLID 25574 - NULLify the EMN ID row
					pRow->Value[phtdlcEmnID] = g_cvarNull;
					// (z.manning 2010-02-02 11:40) - PLID 34287 - PicID
					pRow->Value[phtdlcPicID] = prs->GetFields()->GetItem("PicID")->GetValue();
					pRow->Value[phtdlcEmnModifiedDate] = g_cvarNull;

					// parse the filename out of the path
					CString file, path;
					CString str = AdoFldString(prs, "PathName");
					if (str != "") {

						int loc = str.ReverseFind('\\');
						//files w/o a full path
						if (loc == -1) {		//no backslash, must just be a file
							file = str;
							path = GetCurPatientDocumentPath();
						}
						//directories
						else if (AdoFldString(prs, "Name") == "BITMAP:FOLDER") {
							file = "";
							path = str;
						}
						//everything else
						else {

							file = str.Right(str.GetLength() - loc - 1);
							path = str.Left(loc);
						}

						pRow->Value[phtdlcFilePath] = _bstr_t(path);
						pRow->Value[phtdlcFilename] = _bstr_t(file);
					}
					else {
						//This is a printed statement, or something else without a real "path", so...
						pRow->Value[phtdlcFilePath] = _bstr_t("");
						pRow->Value[phtdlcFilename] = _bstr_t("");
					}

					// (a.walling 2008-09-17 14:54) - PLID 23904
					EnsureRowIcon(pRow);

					// check to see if anythign has changed between what's showing and what is in data
					if (nRow == -1 && !DoesRowMatch(nMailSentID, strPath, strNote, strStaff, dtServiceDate, nCatID, file))
					{
						m_pDocuments->AddRow(pRow);
					}

					// (j.jones 2014-08-04 15:06) - PLID 32474 - if we know the file was not a photo,
					// don't refresh the photo viewer, otherwise refresh if it is definitely or
					// potentially a photo
					if (ePhotoStatus != TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto) {
						m_PhotoViewer.Refresh();
					}
				}
			} NxCatchAll("Error in CHistoryDlg::OnTableChangedEx:MailSent");
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CHistoryDlg::RefreshTabs()
{
	try {

		//Store the current selected id.
		long nSelID;
		if(m_tab->CurSel == -1 || m_tab->CurSel >= m_arTabs.GetSize()) nSelID = -2;
		else nSelID = m_arTabs.GetAt(m_tab->CurSel).nCatID;

		//Load up our tab array.
		m_arTabs.RemoveAll();
		HistoryTabInfo htiPhotos;
		htiPhotos.nCatID = -1;
		htiPhotos.strDescription = "Photos";
		m_arTabs.Add(htiPhotos);

		// (j.jones 2012-04-17 17:12) - PLID 13109 - We now either sort alphabetically, or by the stored sort order.
		// I also parameterized the recordset using SqlFragments.
		BOOL bSortAlpha = (GetRemotePropertyInt("SortHistoryTabsAlphabetically", 1, 0, "<None>", true) == 1);
		CSqlFragment sqlOrderByClause("Description ASC");
		if(!bSortAlpha) {
			sqlOrderByClause = CSqlFragment("SortOrder ASC");
		}

		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view		
		CSqlFragment sqlAllowedCategoryClause(GetAllowedCategoryClause("ID"));
		
		CSqlFragment sqlTabFilter("IsPatientTab = 1");		
		if(!m_bInPatientsModule) {
			sqlTabFilter = CSqlFragment("IsContactTab = 1");
		}
		
		_RecordsetPtr rsTabs = CreateParamRecordset("SELECT ID, Description "
			"FROM NoteCatsF WHERE {SQL} AND {SQL} "
			"ORDER BY {SQL}", sqlTabFilter, sqlAllowedCategoryClause, sqlOrderByClause);
		while(!rsTabs->eof) {
			HistoryTabInfo htiNew;
			htiNew.nCatID = AdoFldLong(rsTabs, "ID");
			htiNew.strDescription = ConvertToControlText(AdoFldString(rsTabs, "Description"));
			m_arTabs.Add(htiNew);
			rsTabs->MoveNext();
		}
		rsTabs->Close();

		m_tab->BottomTabs = true;		
		
		// (j.jones 2016-04-20 11:01) - NX-100214 - Set HeaderMode to false, which will
		// use a slightly different theme than the module tabs use.
		// A HeaderMode of false looks nicer when the tab is next to a datalist.
		m_tab->HeaderMode = false;

		m_tab->PutTabWidth(m_arTabs.GetSize()+1);
		m_tab->PutSize(m_arTabs.GetSize()+1);
		m_tab->PutLabel(0, "Photos");//The first tab is always the photos tab.
		//TES 8/10/2011 - PLID 44966 - If they don't have permission to view images, hide the Photos tab
		// (e.frazier 2016-05-19 10:31) - this permission check shouldn't take place in the Contacts module
		if (m_bInPatientsModule)
		{
			if (!(GetCurrentUserPermissions(bioPatientHistory) & (sptDynamic1))) {
				m_tab->PutShowTab(0, g_cvarFalse);
			}
		}
		
		for(int i = 0; i < m_arTabs.GetSize(); i++) {
			m_tab->PutLabel(i, _bstr_t(m_arTabs.GetAt(i).strDescription)) ;
			//DRT 6/21/2004 - PLID 13108 - Reset the back color for all tabs, otherwise
			//	if we pull things in and then put them back, we'll end up with multiple 
			//	highlighted items.
			// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
			//m_tab->ResetTabBackColor(i);
		}		
		
		//Last tab is always "Miscellaneous."
		m_tab->PutLabel(m_tab->GetSize()-1, "Miscellaneous");
		// (j.jones 2016-04-26 15:26) - NX-100214 - added a ShortLabel version
		m_tab->PutShortLabel(m_tab->GetSize()-1, "Misc.");

		//Make sure the right tab is selected.
		if(nSelID == -2) m_tab->CurSel = m_tab->GetSize()-1;
		else {
			// (b.cardillo 2006-11-29 16:22) - PLID 22359 - We used to just loop through and hope 
			// for the best.  Now we detect if we succeeded or not by way of the bFoundMatch 
			// variable.  Also, as a nice side-effect we get out of the loop early when possible.
			BOOL bFoundMatch = FALSE;
			for(int i = 0; !bFoundMatch && i < m_arTabs.GetSize(); i++) {
				if(m_arTabs.GetAt(i).nCatID == nSelID) {
					m_tab->CurSel = i;
					bFoundMatch = TRUE;
				}
			}
			if (!bFoundMatch) {
				// We still didn't find the one that was selected before, which means the user must 
				// have hidden that category.  So switch to the default, the miscellaneous tab.
				m_tab->PutCurSel(m_tab->GetSize() - 1);
			}
		}

		if(m_bThemeOpen) {
			//Windows XP Theme mode.  For now, we're not going to do anything different here
		}
		else {
			//DRT 5/27/2004 - PLID 12564
			//Windows Classic mode - Draw the text highlighted, otherwise it's too hard
			//	to see what tab we are currently on

			//Change the text color of the currently selected tab
			// (j.jones 2016-04-15 11:44) - NX-100214 - we no longer change the tab colors
			//if(m_tab->GetCurSel() > 0)
			//	m_tab->SetTabBackColor(m_tab->GetCurSel(), TAB_HIGHLIGHT_COLOR);
		}

	}NxCatchAll("Error in RefreshTabs");
}

LRESULT CHistoryDlg::OnHistoryTabChanged(WPARAM wParam, LPARAM lParam)
{
	OnSelectTabDocTabs((short)wParam, (short)lParam);
	return 0;
}

BOOL CHistoryDlg::ImportAndAttachFolder(FileFolder &Folder)
{
	// (j.gruber 2010-01-22 13:50) - PLID 23693 - always return false here and we'll check down further when we know the filename
	// (j.gruber 2010-01-27 08:58) - PLID 23693 - added check for if in Pats module
	BOOL bCheckForCat = FALSE;
	if (m_bInPatientsModule) {
		bCheckForCat = TRUE;
	}
	// (j.jones 2010-12-29 16:04) - PLID 41943 - forced the pstrNewFileName to be NULL, it is not supported when attaching folders
	return ::ImportAndAttachFolder(Folder, GetActivePersonID(), GetSafeHwnd(), CheckAssignCategoryID(FALSE), "", GetPicID(), bCheckForCat, "", NULL);	
}

LRESULT CHistoryDlg::OnDropFiles(WPARAM wParam, LPARAM lParam)
{
	GetMainFrame()->SetForegroundWindow();

	// (j.jones 2009-12-18 08:31) - PLID 34606 - check permissions, doesn't matter
	// if they are dropping a new file or moving existing files between tabs, if
	// you don't have write permission, you can't do either one
	// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
	if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
		return FALSE;
	}

	if (m_bDragging) {
		int nCurTabCatID;
		if(m_tab->CurSel == -1 || m_tab->CurSel >= m_arTabs.GetSize())
			nCurTabCatID = -1;
		else
			nCurTabCatID = m_arTabs.GetAt(m_tab->CurSel).nCatID;
		
		// Make sure we actually changed tabs and didn't just drag across the same tab
		if (m_DragDocInfo.nOriginalTabCatID != nCurTabCatID) {
			for (int i = 0; i < m_DragDocInfo.arynMailID.GetSize(); i++) {
				// Only change the category if the category is different
				if (m_DragDocInfo.arynOriginalCatID.GetAt(i) != nCurTabCatID) {

					// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variant
					ChangeDocumentCategory(m_DragDocInfo.arynMailID.GetAt(i), 
											m_DragDocInfo.arystrFilename.GetAt(i), 
											m_DragDocInfo.arystrFileNameOnly.GetAt(i), 
											nCurTabCatID, 
											m_DragDocInfo.arybIsPacket.GetAt(i),
											m_DragDocInfo.aryvIsPhoto.GetAt(i));	
				}
			}

			if (m_DragDocInfo.arynMailID.GetSize() > 1) {
				m_pDocuments->PutCurSel(-1);
				m_nRowSel = -1;
			}
		}
	
		m_bDragging = FALSE;
		m_DragDocInfo.arybIsPacket.RemoveAll();
		// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variants
		m_DragDocInfo.aryvIsPhoto.RemoveAll();
		m_DragDocInfo.arynMailID.RemoveAll();
		m_DragDocInfo.arystrFilename.RemoveAll();
		m_DragDocInfo.arystrFileNameOnly.RemoveAll();
		m_DragDocInfo.arynOriginalCatID.RemoveAll();
	}
	else {
		//OK, they've dragged some files on here.  Let's gather some info about them.
		HDROP hDrop = (HDROP)wParam;
		CString strFileName;
		int nFileCount = DragQueryFile(hDrop, -1, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
		strFileName.ReleaseBuffer();
		
		//OK, first, go through and load our folder structure.
		FileFolder fRoot;
		fRoot.strFolderName = "";

		for(int i = 0; i < nFileCount; i++) {
			DragQueryFile(hDrop, i, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
			strFileName.ReleaseBuffer();
			//Is it a directory?
			CFileFind ff;
			if(ff.FindFile(strFileName)) {
				ff.FindNextFile();
				if(ff.IsDirectory()) {
					fRoot.arSubFolders.Add(LoadFolder(strFileName));
				}
				else {
					fRoot.saFileNames.Add(strFileName);
				}
			}
		}

		//OK, let's make a message box out of it.
		CString strFileNames;
		int nFilesAdded = 0;
		GetFileNameList(fRoot, strFileNames, "", nFilesAdded, 10);

		CString strMessage = "Are you sure you want to import the following files into this patient's default documents folder?"
			"\r\n"
			"\r\n"
			+ strFileNames;

		if(IDYES == MsgBox(MB_YESNO, "%s", strMessage)) {
			CWaitCursor cuWait;
			ImportAndAttachFolder(fRoot);
		}
		DragFinish(hDrop);

		UpdateView();
	}
	return 0;
}

LRESULT CHistoryDlg::OnPhotoViewerDetachable(WPARAM wParam, LPARAM lParam)
{
	m_bPhotoViewerDetachable = wParam ? true : false;
	EnableAppropriateButtons();
	return TRUE;
}

void CHistoryDlg::TWAINAcquireFromDevice(const CString& strDeviceName)
{
	// (j.gruber 2010-01-22 13:55) - PLID 23693 - added isImage check, always true here
	NXTWAINlib::Acquire(m_id, GetCurPatientDocumentPath(), NULL, OnNxTwainPreCompress, NULL, strDeviceName, CheckAssignCategoryID(TRUE));
}

// (a.walling 2008-10-28 13:05) - PLID 31334 - Launched from WIA auto-start
void CHistoryDlg::WIAAcquireFromDevice(const CString& strDeviceName)
{
	if (!IsInModalLoop()) {

		// (j.jones 2009-12-18 08:31) - PLID 34606 - check permissions before allowing the scan
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		WIA::IDevicePtr pDevice = NULL;
		if (strDeviceName.IsEmpty()) {
			pDevice = NxWIA::GetDefaultDevice(FALSE); // explicitly allow all devices
		} else {
			pDevice = NxWIA::GetDeviceFromID(strDeviceName);

			if (pDevice == NULL) {
				pDevice = NxWIA::GetDefaultDevice(FALSE); // explicitly allow all devices
			}
		}
		
		if (pDevice) {
			AcquireFromWIA(pDevice);
		} else {
			MessageBox("No WIA device is connected");
		}
	} else {
		LogDetail("Could not acquire from WIA -- application is in a modal state.");
	}
}

//DRT 5/28/2004 - PLID 12658 - Add double click functionality to do what clicking the icon does.
void CHistoryDlg::OnDblClickCellDocuments(long nRowIndex, short nColIndex) 
{
	try {
		// (a.walling 2008-09-15 15:58) - PLID 23904 - Handle the Icon column as well as the deprecated Name column
		//If they double click anywhere except the icon, open the document.
		if(nColIndex == phtdlcName || nColIndex == phtdlcIcon) {
			//This is the icon, don't do anything
			return;
		}

		//There is a massive amount of work done when attempting to open a file - verifying it
		//	making sure data matches up, etc, so we'll just pretend like they left clicked
		//	on the icon
		CPoint pt;
		GetCursorPos(&pt);
		OnLeftClickDocuments(nRowIndex, phtdlcIcon, pt.x, pt.y, 0);

	} NxCatchAll("Error in OnDblClickCellDocuments()");
}

// (j.gruber 2010-01-22 13:37) - PLID 23693 - added parameter for if its an image
long CHistoryDlg::CheckAssignCategoryID(BOOL bIsImageFile) {

	try {
		// (j.jones 2004-09-27 15:26) - determine if we want to assign the category for the currently selected tab, 
		//provided that the currently selected tab is a category in the first place

		// (j.gruber 2010-01-22 13:15) - PLID 23693 - added a preference that defaults image files 
		long nDefaultImageCategory = -1;
		if (bIsImageFile && m_bInPatientsModule) {
			nDefaultImageCategory = GetRemotePropertyInt("PatientHistoryDefaultImageCategory", -1, 0, GetCurrentUserName());
			//TES 8/2/2011 - PLID 44814 - Check whether they're allowed to use the default category
			if(nDefaultImageCategory != -1 && !CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nDefaultImageCategory, TRUE)) {
				nDefaultImageCategory = -1;
			}
		}

		short nTab = m_tab->CurSel;
		//tab 0 is Photos, and the last tab is Misc.

		//DRT 9/26/2005 - PLID 17546 - This is now in use for photos tab, with slighly different action
		if(nTab != m_tab->GetSize() - 1) {

			long nUseCategory = GetRemotePropertyInt("HistoryMergeToCategory", 2, 0, GetCurrentUserName(), true);
			//0 - always assign the currently selected category
			//1 - never assign a category
			//2 - prompt to assign

			bool bAttemptPrompt = false;
			CString strCatName = "";
			long nCategoryID = -1;

			if(nTab == 0) {
				//On Photos tab - We have 4 possible scenarios here:
				//1) If you are not filtering, or filtering on all categories, nothing will happen differently.  No category will be assigned to your image(s).
				//2) If you are filtering on no categories, no prompt will be made, and your item will be added with no category.
				//3) If you are filtering on multiple categories, it will prompt and let you know that due to your filters, the newly added image(s) will not be viewable.  No categories will be assigned.
				//4) If you are filtering on 1 category, it will prompt you and ask if you want to assign all images to that category.  If you say no, your filter will remain and your images will not be visible.

				//Then the preference is applied:
				//Never:  Do #3.
				//Always:  Do #3, #4 (auto-save, no prompt).
				//Prompt:  Do #3, #4 (prompt).

				//Case #1a
				if(!IsDlgButtonChecked(IDC_USE_HISTORY_FILTER)) {
					
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
					else {
						//do nothing
						return -1;
					}
				}

				//Bad case?
				long nCurSel = m_pFilter->GetCurSel();
				if(nCurSel == -1) {
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
					else {
						return -1;
					}
				}

				//Case #1b
				long nCurrentFilter = VarLong(m_pFilter->GetValue(nCurSel, 0));
				if(nCurrentFilter == ID_ALL_CATS) {
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
					else {
						//All categories, do nothing
					}
				}
				//Case #2 - 
				else if(nCurrentFilter == ID_NO_CAT) {
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
					else {
						//do nothing
					}
				}
				//Case #3
				else if(nCurrentFilter == ID_MULTIPLE_CATS) {
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
					else {					
						AfxMessageBox("Due to your multiple category filter, the image(s) you are importing will not be visible.  Change your filter to 'All Categories' to see the "
							"newly imported image(s).");
					}
				}
				//Case #4
				else {
					//We are filtering on a single category, follow the preference
					bAttemptPrompt = true;
					strCatName = VarString(m_pFilter->GetValue(nCurSel, 1), "");
					nCategoryID = nCurrentFilter;
				}
			}
			else {
				//Normal tab
				bAttemptPrompt = true;
				strCatName = (LPCTSTR)(m_tab->GetLabel(nTab));
				nCategoryID = m_arTabs.GetAt(nTab).nCatID;
			}

			if(bAttemptPrompt) {
				//Either using a normal tab, or on photos and looking at a single category.  Setup the prompt to ask
				//	the user if they wish to categorize their new image(s).
				CString str;

				str.Format("Would you like to assign this new object to the '%s' category?",strCatName);
				if(nUseCategory == 0 || 
					(nUseCategory == 2 && IDYES == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO))) {

					return nCategoryID;
				}
				else {
					// (j.gruber 2010-01-22 13:33) - PLID 23693 - see if we are setting an image
					if (nDefaultImageCategory != -1) {
						return nDefaultImageCategory;
					}
				}
			}
		}
		else {

			//miscellaneous tab is now in play
			if (nDefaultImageCategory != -1) {
				return nDefaultImageCategory;
			}
		}




	} NxCatchAll("Error in CHistoryDlg::CheckAssignCategoryID");

	return -1;
}

long CHistoryDlg::AttachFileToHistory(const CString &path, long nPatientID, HWND hwndMessageParent, long nCategoryID, LPCTSTR strSelection /*= SELECTION_FILE*/, LPCTSTR strPDAFilename /* = NULL*/)
{
	// (c.haag 2004-11-11 11:46) - PLID 14074 - Assign procinfoid and emrgroupid's to the
	// resultant mailsent record. TODO: Perhaps consdering sandwiching everything into a
	// transaction
	long nMailSentID = ::AttachFileToHistory(path, nPatientID, hwndMessageParent, nCategoryID, strSelection, strPDAFilename);
	if (nMailSentID > 0 && GetPicContainer())
	{
		long nPicID = GetPicID();
		CPicContainerDlg* pDlg = GetPicContainer();
		ExecuteSql("UPDATE MailSent SET PicID = CASE WHEN %d < 0 THEN NULL ELSE %d END WHERE MailID = %d",
			nPicID, nPicID, nMailSentID);

		// (c.haag 2004-12-17 09:19) - PLID 14987 - We need to fire a table checker because
		// we modified the record
		// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
		CClient::RefreshMailSentTable(m_id, nMailSentID);

		// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if in a PIC that has an EMR
		if(GetPicContainer() && GetPicContainer()->GetCurrentEMRGroupID() > 0) {
			//refresh the EMN to update the Word icon
			CClient::RefreshTable(NetUtils::EMRMasterT, IsImageFile(path) ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);
		}
	}

	//TES 8/12/2011 - PLID 44966 - If they just attached a photo, but don't have permission to view photos, let them know why they can't
	// see it.
	// (e.frazier 2016-05-19 10:31) - this permission check shouldn't take place in the Contacts module
	if (m_bInPatientsModule) {
		if (nMailSentID > 0 && IsImageFile(path) && !(GetCurrentUserPermissions(bioPatientHistory)&sptDynamic1)) {
			MsgBox("The file was successfully attached.  However, it will not display in the list because you do not have permission to view Photos.");
		}
	}
	return nMailSentID;
}

// (c.haag 2004-12-16 15:35) - PLID 14392 - We should have done this a LONG time ago
long CHistoryDlg::GetActivePersonID()
{
	// (c.haag 2007-03-09 10:17) - PLID 25138 - If we're in the PIC, get the
	// patient ID from the PIC
	if (GetPicContainer()) {
		return GetPicContainer()->GetPatientID();
	} else if(m_bInPatientsModule)	//we're in the patients module
		return ::GetActivePatientID();
	return ::GetActiveContactID(); // we must be in the contacts module
}

// (a.walling 2006-08-14 17:30) - PLID 21970 - return the correct person name depending on which module we are in.
CString CHistoryDlg::GetActivePersonName()
{
	// (c.haag 2007-03-09 10:17) - PLID 25138 - If we're in the PIC, get the
	// patient name from the PIC
	if (GetPicContainer()) {
		return GetPicContainer()->GetPatientName();
	} else if(m_bInPatientsModule)	//we're in the patients module
		return ::GetActivePatientName();
	return ::GetActiveContactName(); // we must be in the contacts module
}

// (a.walling 2014-06-09 10:58) - PLID 62355 - Use the appropriate patient ID/name based on whether we are embedded in the PIC or not
long CHistoryDlg::GetActivePatientID()
{
	ASSERT(GetPicContainer() || m_bInPatientsModule);
	return GetActivePersonID();
}

CString CHistoryDlg::GetActivePatientName()
{
	ASSERT(GetPicContainer() || m_bInPatientsModule);
	return GetActivePersonName();
}

//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
void CHistoryDlg::RefreshTable(NetUtils::ERefreshTable table, DWORD id /*= -1*/)
{
	CClient::RefreshTable(table, id);

	// (c.haag 2004-12-17 11:15) - PLID 14987 - We don't get table checkers if we are in the PIC
	// because of how message pumps work in modal dialogs. Some of the code here counts on getting
	// a return table checker.
	// (a.walling 2008-09-03 12:19) - PLID 19638 - The PIC is now modeless
	/*if (GetPicContainer()) {
		PostMessage(WM_TABLE_CHANGED, (WPARAM)table, (LPARAM)id);
	}*/
}

long CHistoryDlg::GetPicID()
{
	if (GetPicContainer()) {
		return GetPicContainer()->GetCurrentPicID();
	}
	else {
		//Whaaa???
		return -1;
	}
}

void CHistoryDlg::OnSelChosenPhotosSortCriterionCombo(long nRow)
{
	try {
		// Change the sort criterion in the photo control
		BOOL bNeedRefresh;
		{
			CPhotoViewerCtrl::EDisplaySortCriteria edsc;

			long nCurSel = m_dlPhotosSortCriterion->GetCurSel();
			if (nCurSel != sriNoRow) {
				edsc = (CPhotoViewerCtrl::EDisplaySortCriteria)VarLong(m_dlPhotosSortCriterion->GetValue(nCurSel, 0));
			} else {
				edsc = CPhotoViewerCtrl::dscAttachDate;
			}

			bNeedRefresh = m_PhotoViewer.SetPrimarySortCriterion(edsc);

			//DRT 8/1/2005 - PLID 17124 - Remember the status of this field so we can pull it up next time
			SetRemotePropertyInt("PhotoViewerSortCriteria", (long)edsc, 0, GetCurrentUserName());
		}

		// Make sure our order checkbox is consistent with the sort order of the photo control
		{
			CheckDlgButton(IDC_PHOTOS_SORT_DESCENDING_CHECK, m_PhotoViewer.GetPrimarySortOrder() ? 0 : 1);
			OnPhotosSortDescendingCheck();
		}
		
		// And refresh it so the new sorting takes effect (but only if it's visible)
		if (bNeedRefresh && m_PhotoViewer.IsWindowVisible()) {
			m_PhotoViewer.Invalidate();
		}

	} NxCatchAll("CHistoryDlg::OnSelChosenPhotosSortCriterionCombo");
}

void CHistoryDlg::OnPhotosSortDescendingCheck() 
{
	try {
		// Change the sort order in the photo control
		BOOL bNeedRefresh = m_PhotoViewer.SetPrimarySortOrder(IsDlgButtonChecked(IDC_PHOTOS_SORT_DESCENDING_CHECK) ? FALSE : TRUE);

		//DRT 8/1/2005 - PLID 17124 - Remember the status of this field so we can pull it up next time
		SetRemotePropertyInt("PhotoViewerSortCriteriaOrder", IsDlgButtonChecked(IDC_PHOTOS_SORT_DESCENDING_CHECK) ? 1 : 0, 0, GetCurrentUserName());
	
		// And refresh it so the new sorting takes effect (but only if it's visible)
		if (bNeedRefresh && m_PhotoViewer.IsWindowVisible()) {
			m_PhotoViewer.Invalidate();
		}
	} NxCatchAll("CHistoryDlg::OnPhotosSortDescendingCheck");
}

void CHistoryDlg::OnCheckFilterOnEmr() 
{
	UpdateView();
}

// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
CString CHistoryDlg::GetCurPatientDocumentPath()
{
	if(m_strCurPatientDocumentPath.IsEmpty()) {
		m_strCurPatientDocumentPath = GetPatientDocumentPath(m_id);
	}

	return m_strCurPatientDocumentPath;
}

void CHistoryDlg::BeginDocumentsDrag()
{
	try {
		// Get the information for the document to be dragged
		int nCurSel = m_pDocuments->GetCurSel();
		if (nCurSel == -1) {
			m_bDragging = FALSE;
			return;
		}

		// (z.manning 2008-07-03 09:23) - PLID 25574 - No dragging EMNs
		long nEmnID = VarLong(m_pDocuments->GetValue(nCurSel, phtdlcEmnID), -1);
		if(nEmnID != -1) {
			m_bDragging = FALSE;
			return;
		}

		// Make sure stored drag info is cleared out
		m_DragDocInfo.arybIsPacket.RemoveAll();
		// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variants
		m_DragDocInfo.aryvIsPhoto.RemoveAll();
		m_DragDocInfo.arynMailID.RemoveAll();
		m_DragDocInfo.arystrFilename.RemoveAll();
		m_DragDocInfo.arystrFileNameOnly.RemoveAll();
		m_DragDocInfo.arynOriginalCatID.RemoveAll();
		m_bDragDropNeedToRefresh = FALSE;

		// Now get the selected items information
		long p = m_pDocuments->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		int nNumDocs = 0;
		while (p)
		{	
			m_pDocuments->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			// (a.wetta 2006-09-12 09:40) - Make sure this isn't an unattached document
			if (VarLong(pRow->GetValue(phtdlcMailID)) != -1) {
				nNumDocs++;
				m_DragDocInfo.arynMailID.Add(VarLong(pRow->GetValue(phtdlcMailID)));
				m_DragDocInfo.arystrFilename.Add(VarString(pRow->GetValue(phtdlcFilePath)) + "\\" + CString(pRow->GetValue(phtdlcFilename).bstrVal));
				m_DragDocInfo.arystrFileNameOnly.Add(VarString(pRow->GetValue(phtdlcFilename).bstrVal));
				_variant_t varName = pRow->GetValue(phtdlcName);
				if(varName.vt == VT_I4)
					m_DragDocInfo.arybIsPacket.Add(TRUE);
				else
					m_DragDocInfo.arybIsPacket.Add(FALSE);
				// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variants
				_variant_t varIsPhoto = pRow->GetValue(phtdlcIsPhoto);
				m_DragDocInfo.aryvIsPhoto.Add(varIsPhoto);
				m_DragDocInfo.arynOriginalCatID.Add(VarLong(pRow->GetValue(phtdlcCategory)));
			}
			
			pDisp->Release();
		}
		if (nNumDocs < 1) {
			m_bDragging = FALSE;
			return;
		}
			
		CString strDragMessage = "NxHistoryDlgDragDrop";
		HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, 
			strDragMessage.GetLength());

		int nCurTabCatID;
		if(m_tab->CurSel == -1 || m_tab->CurSel >= m_arTabs.GetSize())
			nCurTabCatID = -1;
		else
			nCurTabCatID = m_arTabs.GetAt(m_tab->CurSel).nCatID;
		m_DragDocInfo.nOriginalTabCatID = nCurTabCatID;

		if (hData == NULL) {
			m_bDragging = FALSE;
			return;
		}

		LPTSTR strNew = (LPTSTR)GlobalLock(hData); 
		memcpy(strNew, strDragMessage.GetBuffer(strDragMessage.GetLength()), 
			strDragMessage.GetLength()); 
		GlobalUnlock(hData); 

		COleDataSource ods;
		ods.CacheGlobalData(RegisterClipboardFormat(CF_NXHISTDD), hData);

		CHistoryTabsDropSource *htds = new CHistoryTabsDropSource();
		htds->m_wndParentHistoryDlg = this;

		if (m_DragDocInfo.arynMailID.GetSize() > 1) {
			htds->m_bIsPacket = TRUE;
			htds->m_strDragFilename = "";
		}
		else {
			htds->m_bIsPacket = m_DragDocInfo.arybIsPacket.GetAt(0);
			htds->m_strDragFilename = m_DragDocInfo.arystrFilename.GetAt(0);
			// (a.walling 2008-09-22 15:03) - PLID 31471 - Pass in the large HICON if we can get it
			SHFILEINFO fileinfo;
			if (SHGetFileInfo(htds->m_strDragFilename, 0, &fileinfo, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_ICON)) {
				if(fileinfo.hIcon) {
					htds->m_hIcon = fileinfo.hIcon;
				}
			}
		}

		POINT ptCursorPos;
		GetCursorPos(&ptCursorPos);
		ClientToScreen(&ptCursorPos);

		// (a.walling 2013-09-03 16:47) - PLID 58039 - Document drop target created on demand
		CHistoryTabsDropTarget dropTargetDocuments;
		dropTargetDocuments.Register(GetDlgItem(IDC_DOCUMENTS));
		dropTargetDocuments.m_nTargetType = TT_DOCUMENTS;
		dropTargetDocuments.m_nInitialTab = m_tab->CurSel;
		dropTargetDocuments.m_ptrTabs = m_tab;

		CRect rcRectStartDrag(0,0,5,5);

		DROPEFFECT de = ods.DoDragDrop(DROPEFFECT_MOVE, rcRectStartDrag, htds);
		delete htds;

		dropTargetDocuments.Revoke();

		GetDlgItem(IDC_CATEGORY_TAB)->Invalidate();

		m_bDragging = FALSE;
	}NxCatchAll("Error in CHistoryDlg::BeginDocumentsDrag");
}

// (m.hancock 2006-11-22 09:51) - PLID 21496 - This function will determine if a file can be deleted and return 
// the boolean for that value.  If the file cannot be deleted, the errors encountered are returned in astrMessages.
BOOL CHistoryDlg::DeleteSingleFile(CString strFullFileName, CString strNote, CStringArray& astrMessages, BOOL bIgnoreFileErrors /*= FALSE*/)
{
	try {
		// (m.hancock 2006-08-04 16:12) - PLID 21496 - Moved this code to its own function so it can be called without code duplication.

		// (c.haag 2004-01-28 11:21) - Second, try to delete the file. We don't know if this will
		// work. If it doesn't work, we won't detach the file.
		if (!DeleteFile(strFullFileName))
		{
			long nLastError = GetLastError();

			if(nLastError == ERROR_FILE_NOT_FOUND)
			{
				if(!bIgnoreFileErrors) {
					CString str;
					str.Format("The document '%s' was detached, but could not be deleted because it doesn't exist.", strNote);
					astrMessages.Add(str);
				}
				// (m.hancock 2006-11-22 09:46) - PLID 21496 - We need to return true here because the file needs to be detached
				// and will be treated the same as detaching and deleting the file.
				return TRUE;
			}
			else if(nLastError == ERROR_ACCESS_DENIED)
			{
				// (z.manning, 06/01/2007) - PLID 25086 - If we can't delete a file because the user lacks permission,
				// then add a message indicating as much, but return true so that we still detach the file.
				if(!bIgnoreFileErrors) {
					astrMessages.Add(FormatString("The document '%s' was detached, but could not be deleted because you lack sufficient Windows permissions.", strNote));
				}
				return TRUE;
			}
			else
			{
				if (astrMessages.GetSize() < 20)
				{
					CString str;
					LPVOID lpMsgBuf;
					FormatMessage( 
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						nLastError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL 
					);
					str.Format("The document '%s' could not be deleted, and was also not detached. Reason: %s", strNote, (LPCTSTR)lpMsgBuf);
					astrMessages.Add(str);
					LocalFree(lpMsgBuf);
				}
				return FALSE;
			}
		}
		else
			return TRUE;
	} NxCatchAll("CHistoryDlg::DeleteFile");
	return FALSE;  //If we get here, then we have a problem.
}

void CHistoryDlg::OnDragBeginDocuments(BOOL FAR* pbShowDrag, long nRow, short nCol, long nFlags) 
{
	m_bDragging = TRUE;
	BeginDocumentsDrag();
}

// (a.wetta 2007-07-09 13:33) - PLID 17467 - This function handles the mark as photo command menu option
void CHistoryDlg::OnTogglePhotoStatus()
{
	try {
		if (m_nRowSel == -1)
			return;

		IRowSettingsPtr pRow = m_pDocuments->GetRow(m_nRowSel);

		if (pRow) {
			// Determine if this document has been marked as a photo or not
			BOOL bIsPhoto;
			if (pRow->GetValue(phtdlcIsPhoto).vt == VT_EMPTY || pRow->GetValue(phtdlcIsPhoto).vt == VT_NULL) {
				// This document was attached before this option existed, so always assume it's a photo
				bIsPhoto = TRUE;
			}
			else {
				bIsPhoto = (BOOL)VarLong(pRow->GetValue(phtdlcIsPhoto));
			}

			SetPhotoStatus(pRow, bIsPhoto ? FALSE : TRUE);
		}		
	}NxCatchAll("Error in OnTogglePhotoStatus");
}

// (a.wetta 2007-07-09 13:32) - PLID 17467 - This function sets the photo status of a document.  It returns FALSE if the document was not an image file.
BOOL CHistoryDlg::SetPhotoStatus(NXDATALISTLib::IRowSettingsPtr pRow, BOOL bIsPhoto)
{
	BOOL bReturn = TRUE;

	try {
		// Get the path name extension
		CString strPath = (LPCTSTR)(_bstr_t(pRow->GetValue(phtdlcPath)));

		if (!IsImageFile(strPath)) {
			// This doesn't appear to be an image file, so it can't be marked as a photo
			bIsPhoto = FALSE;
			bReturn = FALSE;
		}

		long nMailID = pRow->GetValue(phtdlcMailID);
		if (bIsPhoto) {
			// We need to set this as a photo
			ExecuteSql("UPDATE MailSent SET IsPhoto = 1 WHERE MailID = %li", nMailID);
		}
		else {
			// We need to mark this as a non-photo
			ExecuteSql("UPDATE MailSent SET IsPhoto = 0 WHERE MailID = %li", nMailID);

			// If this is a primary photo, then we need to reset the primary photo
			if (pRow->ForeColor == RGB(64,64,255)) {
				ExecuteSql("UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PersonID = %d", m_id);
				m_PhotoViewer.SetPrimaryImage(-1);
				pRow->ForeColor = RGB(0,0,0);
				//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
				// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
				// this shared function will handle sending accordingly
				SendPatientPrimaryPhotoHL7Message(m_id);
			}
		}
		
		// Update the row correctly
		pRow->PutValue(phtdlcIsPhoto, (long)bIsPhoto);

		// (j.dinatale 2012-07-02 13:46) - PLID 51282 - fire a table checker
		// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
		CClient::RefreshMailSentTable(m_id, nMailID, bIsPhoto ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);

	}NxCatchAll("Error in CHistoryDlg::SetPhotoStatus");

	return bReturn;
}

// (z.manning, 09/18/2007) - PLID 27425 - We now redraw the window when the size changes to avoid
// drawing problems that can come up on NxDialogs.
void CHistoryDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);
		SetControlPositions();

		// (a.walling 2008-06-11 09:21) - PLID 30099 - Send ALLCHILDREN
#ifndef NXDIALOG_NOCLIPCHILDEN
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
#else
		RedrawWindow();
#endif

	}NxCatchAll("CHistoryDlg::OnSize");
}

void CHistoryDlg::OnShowEmns()
{
	try
	{
		// (z.manning 2008-07-01 16:30) - PLID 25574 - Remember the selection and reload the list
		SetRemotePropertyInt("HistoryTabShowEmns", m_btnShowEmns.GetCheck(), 0, GetCurrentUserName());
		UpdateView();

	}NxCatchAll("CHistoryDlg::OnShowEmns");
}

void CHistoryDlg::EnsureEmrPreviewDialog()
{
	if(m_pdlgEmrPreview != NULL) {
		return;
	}

	// (z.manning 2008-07-01 16:31) - PLID 25574 - The EMR preview dialog does not exist yet, so create it.
	m_pdlgEmrPreview = new CEMRPreviewPopupDlg(this);	
	m_pdlgEmrPreview->Create(IDD_EMR_PREVIEW_POPUP, this);

	// (a.walling 2010-01-11 15:11) - PLID 31482 - Restore size
	m_pdlgEmrPreview->RestoreSize("History");
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
	m_pdlgEmrPreview->SetPatientID(m_id, aryEMNs);
}

void CHistoryDlg::OpenEmrPreviewDialog(long nEmnID)
{
	// (z.manning 2008-07-01 16:31) - PLID 25574 - Show the EMR preview dialog.
	EnsureEmrPreviewDialog();

	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, and the index of the array representing the EMN
	// we wish to view
	// (z.manning 2012-09-10 14:35) - PLID 52543 - Use the new EmnPreviewPopup object
	CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
	long nIndex = -1;

	//find all EMNIDs in the list
	for(int i = 0; i < m_pDocuments->GetRowCount(); i++) {
		long nAddEMNID = VarLong(m_pDocuments->GetValue(i, phtdlcEmnID), -1);
		if(nAddEMNID != -1) {
			aryEMNs.Add(EmnPreviewPopup(nAddEMNID, VarDateTime(m_pDocuments->GetValue(i, phtdlcEmnModifiedDate))));
			//if this is the EMN we want to display, set that as the index
			if(nAddEMNID == nEmnID) {
				nIndex = aryEMNs.GetSize() - 1;
			}
		}
	}
	
	m_pdlgEmrPreview->PreviewEMN(aryEMNs, nIndex);

	m_pdlgEmrPreview->ShowWindow(SW_SHOWNA);
}

void CHistoryDlg::OnEditingStartingDocuments(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	try
	{
		if(nRow == -1) {
			return;
		}

		// (j.jones 2009-12-18 08:31) - PLID 34606 - check permissions, if they can't edit
		// history, they can't change any of these fields
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			*pbContinue = FALSE;
			return;
		}

		// (z.manning 2008-07-01 15:38) - PLID 25574 - If this is an EMN row, don't let them edit anything;
		long nEmnID = VarLong(m_pDocuments->GetValue(nRow, phtdlcEmnID), -1);
		if(nEmnID != -1) {
			*pbContinue = FALSE;
			return;
		}

		// (j.jones 2014-01-14 11:34) - PLID 58750 - disallow editing if the image is on an EMN detail
		if(nCol == phtdlcFilename) {
			CString strFilePath = VarString(m_pDocuments->GetValue(nRow, phtdlcPath), "");

			bool bFileNameIsPerPatient = true;
			if(!m_bInPatientsModule || strFilePath.Find('\\') != -1 || strFilePath.Find("MultiPatDoc") != -1) {
				//the file name is a full path that is not specific to this patient
				bFileNameIsPerPatient = false;
			}
			if((bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
				"INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
				"WHERE EMRMasterT.PatientID = {INT} "
				"AND (EMRDetailsT.InkImagePathOverride = {STRING} OR EMRDetailsT.InkImagePathOverride = '\\' + {STRING})",
				m_id, strFilePath, strFilePath))
				||
				(!bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
				"WHERE EMRDetailsT.InkImagePathOverride = {STRING}", strFilePath))
				) {

				//cannot warn in EditingStarting
				*pbContinue = FALSE;
				return;
			}

			// check to see if it is use in this EMR, but perhaps not saved yet
			if(GetPicContainer()) {
				CPicContainerDlg* pDlg = GetPicContainer();
				if(pDlg->IsImageFileInUseOnEMR(strFilePath)) {
					//cannot warn in EditingStarting
					*pbContinue = FALSE;
					return;
				}
			}
		}

	}NxCatchAll("CHistoryDlg::OnEditingStartingDocuments");
}

// (a.walling 2008-07-17 15:17) - PLID 30751 - Launch the transcription parser
void CHistoryDlg::OnParseTranscriptions()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Mainframe will check permissions
		if (GetMainFrame()) {
			GetMainFrame()->OnParseTranscriptions();
		}
	}NxCatchAll("CHistoryDlg::OnParseTranscriptions");
}

// (a.walling 2008-09-10 15:06) - PLID 31334 - Recieved WIA event!
LRESULT CHistoryDlg::OnWIAEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		NxWIA::CWIAEvent* pEvent = (NxWIA::CWIAEvent*)wParam;

		if (pEvent) {
			if (IsInModalLoop()) {
				return (LRESULT)-2;
			}
			if (pEvent->EventID == WIA::wiaEventDeviceConnected) {
				if (GetRemotePropertyInt("WIA_AutoLaunchWizardOnConnect", TRUE, 0, GetCurrentUserName(), true)) {
					WIA::IDevicePtr pDevice = NxWIA::GetDeviceFromID(pEvent->DeviceID);
					if (pDevice != NULL && pDevice->Type == WIA::CameraDeviceType || pDevice->Type == WIA::UnspecifiedDeviceType) {
						AcquireFromWIA(pDevice);
					}
				} else return 0;
			} else if (pEvent->EventID == WIA::wiaEventItemCreated) {	
				if (GetRemotePropertyInt("WIA_AutoImportOnCreate", TRUE, 0, GetCurrentUserName(), true)) {
					WIA::IDevicePtr pDevice = NxWIA::GetDeviceFromID(pEvent->DeviceID);
					if (pDevice != NULL && pDevice->Type == WIA::CameraDeviceType || pDevice->Type == WIA::UnspecifiedDeviceType) {
						WIA::IItemPtr pItem = pDevice->GetItem(_bstr_t(pEvent->ItemID));

						if (pItem) {
							CStringArray saTempFiles;						
							WIA::ICommonDialogPtr pCommonDialog = NxWIA::GetCommonDialog();
							AcquireItemFromWIA(saTempFiles, pItem, pCommonDialog);
							if (saTempFiles.GetSize() > 0) {
								ASSERT(saTempFiles.GetSize() == 1);
								if (m_bWIAAutoAcquire) {
									AttachTempFilesToMailSent(saTempFiles);
									return 0;
								} else {
									CScannedImageViewerDlg dlgPreview(saTempFiles[0], this);
									dlgPreview.m_bPromptMode = TRUE;
									dlgPreview.m_strPrompt = FormatString("This image will be attached to history for '%s.' To attach without prompting, check the 'Acquire New Images Automatically' menu item.", GetActivePersonName());
									if (IDOK == dlgPreview.DoModal()) {
										AttachTempFilesToMailSent(saTempFiles);
										return 0;
									} else {
										for (int i = 0; i < saTempFiles.GetSize(); i++) {
											DeleteFileWhenPossible(saTempFiles[i]);
										}
									}
								}
							}
						} else {
							AcquireFromWIA(pDevice);
							return 0;
						}
					}
				}
			} /*else if (pEvent->EventID == WIA::wiaEventScanImage) {

			}*/
		}
	} NxCatchAll("CHistoryDlg::OnWIAEvent");

	return (LRESULT)-1;
}

// (a.walling 2008-09-11 10:57) - PLID 31334 - Toggle flag to auto acquire WIA images.
// this flag is purposefully not retained among sessions. It should only be set manually!
void CHistoryDlg::OnWIAOptionsAutoAcquire()
{
	try {
		m_bWIAAutoAcquire = !m_bWIAAutoAcquire;
		
		if (m_bWIAAutoAcquire) {
			MessageBox("Any new items created on a connected camera device will be automatically imported into the current patient's history. This option will reset when the Patients Module has closed (or when Practice has closed).\r\n\r\n"
				"Please note that not all devices support this functionality.", NULL, MB_OK|MB_ICONINFORMATION);
		}
	} NxCatchAll("CHistoryDlg::OnWIAOptionsAutoAcquire");
}

// (r.galicki 2008-09-26 15:18) - PLID 31407 - Select TWAIN source menu function
void CHistoryDlg::OnSelectSource() {
	try {
		NXTWAINlib::SelectSource();
	} NxCatchAll("CHistoryDlg::OnSelectSource");
}

// (a.walling 2008-09-11 16:13) - PLID 31334 - Are we in a modal loop?
BOOL CHistoryDlg::IsInModalLoop()
{
	// If only there were a better way. This will still not catch all situations,
	// such as when tracking a popup menu, etc
	BOOL bModal = FALSE;

	if (GetPicContainer()) {
		CWnd* pParent = GetParent();
		if (pParent) {
			CWnd* pGrandParent = pParent->GetParent();

			if (pGrandParent != NULL && pGrandParent->IsWindowEnabled()) {
				bModal = FALSE;
			} else {
				bModal = TRUE;
			}
		}
	} else {
		// we are in a view in the mainframe (i hope)

		if (GetMainFrame()->IsWindowEnabled()) {
			bModal = FALSE;
		} else {
			bModal = TRUE;
		}
	}

	return bModal;
}

// (c.haag 2008-09-12 17:02) - PLID 31369 - We now have a stand-alone acquire button for pulling images from TWAIN
// or other sources
void CHistoryDlg::OnBtnAcquire()
{
	try {
		CMenu mnu;
		CPoint pt;
		mnu.CreatePopupMenu();
		GetCursorPos(&pt);
		PopulateAcquireMenu(mnu);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	NxCatchAll("Error in CHistoryDlg::OnBtnAcquire");
}

// (c.haag 2008-09-12 17:04) - PLID 31369 - Populates a menu with image acquision opptions. I moved Adam's code here.
void CHistoryDlg::PopulateAcquireMenu(CMenu& mnu)
{
	long nSubIndex = 0;
	// (a.walling 2008-09-08 12:54) - PLID 31293 - Changed property name
	// (e.lally 2009-06-11) PLID 34600 - Get permissions and disable menu options accordingly
	DWORD dwPermitted = (MF_DISABLED | MF_GRAYED);
	// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
	if(GetCurrentUserPermissions(GetHistoryPermissionObject()) & (sptWrite|sptWriteWithPass)){
		dwPermitted = MF_ENABLED;
	}
	long nScanDefaultPDF = GetRemotePropertyInt("PDF_Default", FALSE, 0, "<None>", true);
	if (nScanDefaultPDF) {
		// (r.galicki 2008-09-18 15:04) - PLID 31409 - "Import as..." changed to "Scan as..."
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_ACQUIRE_PDF,	"Scan as &PDF...");
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_ACQUIRE_MULTI_PDF,	"Scan as &Multi-Page PDF...");
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_SEPARATOR);
		// (a.walling 2009-12-11 11:35) - PLID 36518 - Changed to Scan as Image since it will be saved in the most appropriate format
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_DEFAULT|dwPermitted, IDM_ACQUIRE,	"Scan as &Image...");
	} else {
		// (r.galicki 2008-09-18 15:04) - PLID 31409 - "Import as..." changed to "Scan as..."
		// (a.walling 2009-12-11 11:35) - PLID 36518 - Changed to Scan as Image since it will be saved in the most appropriate format
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_DEFAULT|dwPermitted, IDM_ACQUIRE,	"Scan as &Image...");
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_SEPARATOR);
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_ACQUIRE_PDF,	"Scan as &PDF...");
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_ACQUIRE_MULTI_PDF,	"Scan as &Multi-Page PDF...");
	}
	
	// (a.walling 2008-09-08 09:40) - PLID 31282 - Invoke the scan multi doc dialog
	if (m_bInPatientsModule || GetPicContainer()) {
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_SEPARATOR);
		mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_ACQUIRE_MULTIDOC, "&Scan Multiple Documents...");
	}

	// (r.galicki 2008-09-23 10:50) - PLID 31407 - Added Select Input Source (TWAIN) menu option
	mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_SEPARATOR);
	mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwPermitted, IDM_SELECT_TWAIN_SOURCE,	"Select TWAIN &Input Source...");

	// (a.walling 2008-09-23 12:54) - PLID 31486 - Pass in the camera only property here too
	DWORD dwWIAFlags = NxWIA::IsWIAAvailable(GetRemotePropertyInt("WIA_CameraDevicesOnly", TRUE, 0, GetCurrentUserName(), true)) ? 0 : MF_GRAYED|MF_DISABLED;

	mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|MF_SEPARATOR);
	mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|dwWIAFlags|dwPermitted, IDM_ACQUIRE_WIA, "&Import from Scanner/Camera (WIA)");
	// (a.walling 2008-09-11 10:54) - PLID 31334 - Allow automatic acquisition without prompts
	// (a.walling 2008-10-29 09:13) - PLID 31334 - Disabled for now
	//mnu.InsertMenu(nSubIndex++, MF_BYPOSITION|(m_bWIAAutoAcquire ? MF_CHECKED : MF_UNCHECKED), IDM_WIA_OPTIONS_AUTOACQUIRE, "&Acquire New Images Automatically (WIA)");

	mnu.SetDefaultItem(nScanDefaultPDF ? IDM_ACQUIRE_PDF : IDM_ACQUIRE);
}

// (c.haag 2008-09-19 09:28) - PLID 31368 - Now invoke the audio record dialog.
void CHistoryDlg::OnRecordAudio()
{
	try {
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		//CAudioRecordDlg dlg;

		// Fail if there are no recording devices on the computer
		if (0 == waveInGetNumDevs()) {
			AfxMessageBox("NexTech Practice could not detect any recording devices on your computer. Please contact your hardware administrator for assistance.", MB_ICONERROR | MB_OK);
			return;
		}

		/*
		// Prepare the dialog for recording. We need to be careful here because this history tab
		// can be invoked from three places:
		//
		// 1: PIC container. We need to set the PIC ID and the patient ID.
		//
		if (GetPicContainer()) {
			dlg.SetPatientID(m_id);
			dlg.SetPicID(GetPicID());
			if (IDOK == dlg.DoModal()) {
				UpdateView();
				AfxMessageBox("The recording has been saved and attached to this patient's history.", MB_ICONINFORMATION | MB_OK);
			}
		}
		//
		// 2 and 3: Contacts module, patients module: All we have to do is set the person ID. After the recording is done,
		// it will then be attached to the history tab. The audio record dialog uses AttachFileToHistory to attach the recording
		// to the history tab. I noticed that AttachFileToHistory doesn't seem to care whether it's called on a patient or on a
		// contact. So, we can do both cases here.
		//
		else {
			dlg.SetPatientID(m_id);
			if (IDOK == dlg.DoModal()) {
				UpdateView();
				if (m_bInPatientsModule) {
					AfxMessageBox("The recording has been saved and attached to this patient's history.", MB_ICONINFORMATION | MB_OK);
				} else {
					AfxMessageBox("The recording has been saved and attached to this contact's history.", MB_ICONINFORMATION | MB_OK);
				}
			}
		}
		*/

		// (a.walling 2010-04-13 14:18) - PLID 36821 - Modeless dialog is all handled through here now.
		// (a.walling 2010-07-30 12:37) - PLID 39433 - Allow attaching audio to a specific EMN rather than to a general EMR.
		CAudioRecordDlg::DoAudioRecord(this, m_id, m_bInPatientsModule ? true : false, GetPicContainer() ? GetPicID() : -1, NULL);
	}
	NxCatchAll("Error in CHistoryDlg::OnRecordAudio");
}

// (z.manning 2009-01-29 11:02) - PLID 32885
void CHistoryDlg::OnMergeEmrStdDocument()
{
	try
	{
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		// (z.manning 2009-01-29 11:03) - PLID 32885 - Merge a new template and use the 
		// EMR Standard Templates folder as the default place to pull a template.
		MergeNewDocument(GetTemplatePath("Forms\\EMR Standard Templates"));

	}NxCatchAll("CHistoryDlg::OnMergeEmrStdDocument");
}

// (z.manning 2009-01-29 11:02) - PLID 32885
void CHistoryDlg::OnMergeEmrStdPacket()
{
	try
	{
		// (e.lally 2009-06-11) PLID 34600 - Check write permissions
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}
		// (z.manning 2009-01-29 11:03) - PLID 32885 - Beavhior is 100% identical to 
		// the merge new packet option.
		OnClickMergePacket();

	}NxCatchAll("CHistoryDlg::OnMergeEmrStdPacket");
}

// (z.manning 2009-01-29 11:47) - PLID 32885 - Added a utility function for merging a new document.
// strTemplatePath is the path that the file browser will default to
// This function was basically moved here from CHistoryDlg::OnClickMerge
void CHistoryDlg::MergeNewDocument(CString strTemplatePath)
{
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	// Get template to merge to
	// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	CString strFilter;

	// Always support Word 2007 templates
	strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

	// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
	CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this);
	if(strTemplatePath.IsEmpty() || !DoesExist(strTemplatePath)) {
		strTemplatePath = GetTemplatePath();
	}
	dlg.m_ofn.lpstrInitialDir = strTemplatePath;
	if (dlg.DoModal() == IDOK) {
		CMergeEngine mi;
		mi.m_nPicID = GetPicID();
		
		// If the user clicked OK do the merge
		CWaitCursor wc;
		CString strTemplateName = dlg.GetPathName();
		
		/// Generate the temp table
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_id);
		CString strMergeT = CreateTempIDTable(strSql, "ID");
		
		// Merge
		if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
		mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
		try {

			//DRT 11/24/2003 - PLID 5551 - Due to complaints and suggestions, this has been updated:
			//1)  There is now a preference (defaults to off) for the whole thing to show up.
			//2)  The dialog defaults to the currently logged in user.
			// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
			if(!mi.LoadSenderInfo(TRUE)) {
				return;
			}

			//assign a category ID based on preferences
			// (j.gruber 2010-01-22 13:55) - PLID 23693 - added isImageCheck, always FALSE here
			long nCategoryID = CheckAssignCategoryID(FALSE);
			if(nCategoryID != -1) {
				mi.m_nCategoryID = nCategoryID;
			}
			
			// Do the merge
			// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
			if (mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT)) {
				// If the merge was successful, refresh the history tab
				UpdateView();
			}
		} NxCatchAll("CHistoryDlg::MergeNewDocument - Merge");
	}
}

// (a.walling 2009-05-07 10:34) - PLID 34179 - Update descriptions from CCD
// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
void CHistoryDlg::UpdateCCDInformation(CStringArray& saFiles, CArray<long, long>& arMailIDs, CArray<long, long>& arPatientIDs)
{
	for (int i = 0; i < saFiles.GetSize() && i < arMailIDs.GetSize(); i++) {
		if (arMailIDs[i] != -1 && !saFiles[i].IsEmpty()) {
			try {
				// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs a patient ID
				CCD::UpdateCCDInformation(saFiles[i], arMailIDs[i], arPatientIDs[i]);
			} NxCatchAllThread("Error updating CCD Descriptions");
		}
	}
}

// (a.walling 2009-05-06 10:45) - PLID 34176 - Context menu view CCD
// (j.jones 2010-06-30 11:09) - PLID 38031 - renamed to view any generic XML document
void CHistoryDlg::OnViewXML()
{
	try {
		CStringArray saFileNames, saFilePaths;
		GetSelectedFiles(saFileNames, saFilePaths);

		if (saFileNames.GetSize() > 0 && saFilePaths.GetSize() > 0) {
			CString strFile = saFilePaths[0];

			if (!strFile.IsEmpty()) {
				// (c.haag 2015-05-04) - NX-100442 - Moved the business logic of opening the object into DocumentOpener 
				IRowSettingsPtr pRow = m_pDocuments->GetRow(m_pDocuments->GetCurSel());
				long nMailSentID = -1;
				if (pRow) {
					nMailSentID = VarLong(pRow->GetValue(phtdlcMailID), -1);
				}
				if (CDocumentOpener::OpenXMLDocument(strFile, nMailSentID, this).bNeedUpdateView)
				{
					UpdateView();
				}
			}
		}

		// only handle the first one
	} NxCatchAll("CHistoryDlg::OnViewXML");
}

// (a.walling 2009-05-06 10:45) - PLID 34176 - Context menu view CCD
void CHistoryDlg::OnValidateCCD()
{
#ifdef _DEBUG
	try {
		// (a.walling 2010-01-19 14:09) - PLID 36972 - Validation via NIST web service removed (service out of date)
		ShellExecute(NULL, NULL, "http://xreg2.nist.gov/cda-validation/validation.html", NULL, NULL, SW_SHOW);
	} NxCatchAll("CHistoryDlg::OnValidateCCD");
#endif
}

// (j.jones 2009-10-26 17:24) - PLID 15385 - added date filter
void CHistoryDlg::OnBnClickedRadioHistoryAllDates()
{
	try {

		BOOL bUseDateRange = m_radioDateRange.GetCheck();

		GetDlgItem(IDC_HISTORY_FROM_DATE)->EnableWindow(bUseDateRange);
		GetDlgItem(IDC_HISTORY_TO_DATE)->EnableWindow(bUseDateRange);

		UpdateView();

	}NxCatchAll("CHistoryDlg::OnBnClickedRadioHistoryAllDates");
}

void CHistoryDlg::OnBnClickedRadioHistoryDateRange()
{
	try {

		OnBnClickedRadioHistoryAllDates();

	}NxCatchAll("CHistoryDlg::OnBnClickedRadioHistoryDateRange");
}

void CHistoryDlg::OnChangeHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

		if(!m_bDateFromDown) {

			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_dtFrom.GetValue());
			dtTo = COleDateTime(m_dtTo.GetValue());
			if(dtFrom.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
				m_dtFrom.SetValue(_variant_t(dtTo));
			}
			else {

				//if dtFrom > dtTo, update dtTo
				if(dtFrom > dtTo) {
					dtTo = dtFrom;
					m_dtTo.SetValue(_variant_t(dtTo));
				}

				UpdateView();
			}
		}

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnChangeHistoryFromDate");
}

void CHistoryDlg::OnChangeHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
		if(!m_bDateToDown) {

			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_dtFrom.GetValue());
			dtTo = COleDateTime(m_dtTo.GetValue());
			if(dtTo.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
				m_dtTo.SetValue(_variant_t(dtFrom));
			}
			else {

				//if dtFrom > dtTo, update dtFrom
				if(dtFrom > dtTo) {
					dtFrom = dtTo;
					m_dtFrom.SetValue(_variant_t(dtFrom));
				}
				
				UpdateView();
			}
		}

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnChangeHistoryToDate");
}

void CHistoryDlg::OnDtnDropdownHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateFromDown = TRUE;

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnDtnDropdownHistoryFromDate");
}

void CHistoryDlg::OnDtnDropdownHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateToDown = TRUE;	

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnDtnDropdownHistoryToDate");
}

void CHistoryDlg::OnDtnCloseupHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateFromDown = FALSE;

		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_dtFrom.GetValue());
		dtTo = COleDateTime(m_dtTo.GetValue());
		if(dtFrom.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
			m_dtFrom.SetValue(_variant_t(dtTo));
		}
		else {

			//if dtFrom > dtTo, update dtTo
			if(dtFrom > dtTo) {
				dtTo = dtFrom;
				m_dtTo.SetValue(_variant_t(dtTo));
			}

			UpdateView();
		}

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnDtnCloseupHistoryFromDate");
}

void CHistoryDlg::OnDtnCloseupHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateToDown = FALSE;

		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_dtFrom.GetValue());
		dtTo = COleDateTime(m_dtTo.GetValue());
		if(dtTo.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
			m_dtTo.SetValue(_variant_t(dtFrom));
		}
		else {

			//if dtFrom > dtTo, update dtFrom
			if(dtFrom > dtTo) {
				dtFrom = dtTo;
				m_dtFrom.SetValue(_variant_t(dtFrom));
			}
			
			UpdateView();
		}

		*pResult = 0;

	}NxCatchAll("CHistoryDlg::OnDtnCloseupHistoryToDate");
}

// (j.gruber 2010-01-05 12:38) - PLID 22958 - right click handler to change a patient on a scanned document
void CHistoryDlg::OnChangePatient()
{
	long nAuditTransID = -1;

	try {
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}


		//get the information about the selection that we'll need
		long nCurSel = m_pDocuments->GetCurSel();

		IRowSettingsPtr pRow = m_pDocuments->GetRow(nCurSel);

		if (pRow) {

			//get the mailID
			long nMailID = VarLong(pRow->GetValue(phtdlcMailID), -1);
			CString strFileNameFromList = VarString(pRow->GetValue(phtdlcFilename), "");
			CString strPathFromList = VarString(pRow->GetValue(phtdlcPath), "");

			//check to see if this file is attached to an EMN or a lab
			long nEMNID = VarLong(pRow->GetValue(phtdlcEmnID), -1);

			if (nEMNID != -1) {
				//don't let them move it
				MsgBox("This document is associated with an EMN and cannot be moved.");
				return;
			}

			// (j.jones 2014-01-14 11:34) - PLID 58750 - the image might be used in an EMN still
			bool bFileNameIsPerPatient = true;
			if(!m_bInPatientsModule || strPathFromList.Find('\\') != -1 || strPathFromList.Find("MultiPatDoc") != -1) {
				//the file name is a full path that is not specific to this patient
				bFileNameIsPerPatient = false;
			}
			if((bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
				"INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
				"WHERE EMRMasterT.PatientID = {INT} "
				"AND (EMRDetailsT.InkImagePathOverride = {STRING} OR EMRDetailsT.InkImagePathOverride = '\\' + {STRING})",
				m_id, strPathFromList, strPathFromList))
				||
				(!bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
				"WHERE EMRDetailsT.InkImagePathOverride = {STRING}", strPathFromList))
				) {

				CString strWarn;
				strWarn.Format("The document '%s' is in use in an image item on at least one EMN. It cannot be deleted.", VarString(pRow->GetValue(phtdlcNotes), ""));
				MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
				return;
			}

			// check to see if it is use in this EMR, but perhaps not saved yet
			if(GetPicContainer()) {
				CPicContainerDlg* pDlg = GetPicContainer();
				if(pDlg->IsImageFileInUseOnEMR(strPathFromList)) {
					CString strWarn;
					strWarn.Format("The document '%s' is in use in an image item in this EMR. It cannot be deleted.", VarString(pRow->GetValue(phtdlcNotes), ""));
					MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
					return;
				}
			}

			//now a lab
			_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ResultID FROM LabResultsT WHERE MailID = {INT}", nMailID);
			if (!rsCheck->eof) {

				//don't let them move it
				MsgBox("This document is associated with a Lab result and cannot be moved.");
				return;
			}

			//pop up the patient selection dialog
			//open the list o' patients
			CSelectDlg dlg(this);
			dlg.m_strTitle = "Select a patient";
			dlg.m_strCaption = "Select a patient to move this document to";
			dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
			// Exclude the -25 built-in patient, and inquiries
			CString strWhere;
			strWhere.Format(" PersonT.ID <> %li AND PersonT.ID >= 0 AND CurrentStatus <> 4", m_id);
			dlg.m_strWhereClause = strWhere;
			
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			dlg.AddColumn("Last", "Last", TRUE, FALSE, TRUE);
			dlg.AddColumn("First", "First", TRUE, FALSE, TRUE);
			dlg.AddColumn("Middle", "Middle", TRUE, FALSE, TRUE);
			dlg.AddColumn("UserDefinedID", "PatientID", TRUE, FALSE, TRUE);
			dlg.AddColumn("BirthDate", "Birth Date", TRUE, FALSE, TRUE);
			dlg.AddColumn("(CASE WHEN Gender = 2 THEN 'F' WHEN Gender = 1 THEN 'M' END)", "Gender", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address1", "Address1", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address2", "Address2", TRUE, FALSE, TRUE);
			dlg.AddColumn("City", "City", TRUE, FALSE, TRUE);
			dlg.AddColumn("State", "State", TRUE, FALSE, TRUE);
			dlg.AddColumn("Zip", "Zip", TRUE, FALSE, TRUE);
			
			if(dlg.DoModal() == IDOK) {
				long nNewPatientID = VarLong(dlg.m_arSelectedValues[0], -1);	
				if(nNewPatientID != -1) {								

					
					CString strOriginalFileName = strFileNameFromList;
					CString strOriginalFilePath;
					CString strNewFilePath;
					CString strNewFileName = strFileNameFromList;

					BOOL bMoveFile = TRUE;
					BOOL bIsPatientPath = FALSE;
					if (strPathFromList != strFileNameFromList) {
						//don't move the file if its attached somewhere other then the current patient's folder
						bMoveFile = FALSE;
						//we need to change where this is currently, just in case they rename it below
						strOriginalFilePath = GetFilePath(strPathFromList);
						strNewFilePath = GetFilePath(strPathFromList);
						bIsPatientPath = FALSE;
					}		
					else {
						//they are using the patient's folder

						// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
						strOriginalFilePath = GetCurPatientDocumentPath();
						strNewFilePath = GetPatientDocumentPath(nNewPatientID);
						bIsPatientPath = TRUE;
					}

					//pop up an input box to have them change the name if necessary
					CString strChange, strTemp;
					strChange = strFileNameFromList;
					strTemp = strChange;
					if (InputBox(this, "This file may contain patient information, such as patient name, please update the filename if necessary", strChange, "")) {
						if (strChange != strTemp) {
							if (strChange.IsEmpty()) {
								MsgBox("The filename cannot be blank.");
								return;
							}
							else {
								bMoveFile = TRUE;
								strNewFileName = MakeValidFolderName(strChange);							
							}
						}
					}

					
					CString strSql = BeginSqlBatch();

					BOOL bWarned = FALSE;

					if (bMoveFile) {
						//try to copy the file
						BOOL bFileMoved = FALSE;
						CString strNewFileNameChanged;
						CString strDstPath = strNewFileName;
						DWORD dwLastError = -1;
						if (!MoveFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName))
						{
							
							//does the filename already exist?
							dwLastError = GetLastError();
							if(dwLastError == ERROR_FILE_EXISTS || dwLastError == ERROR_ALREADY_EXISTS) {												
								CRenameFileDlg dlgRename(strNewFilePath ^ strNewFileName, strNewFilePath, this);
								if(dlgRename.DoModal() == IDOK) {
									strDstPath = dlgRename.m_strDstFullPath;				
									strNewFileName = GetFileName(strDstPath);
									if(!MoveFile(strOriginalFilePath ^ strOriginalFileName, strDstPath)) {
										dwLastError = GetLastError();
										bFileMoved = FALSE;
									}
									else {
										bFileMoved = TRUE;
									}
								}							
								else {
									bFileMoved = FALSE;
								}
							}
							else {
								bFileMoved = FALSE;
							}		
						}
						else {
							bFileMoved = TRUE;
						}

						if (!bFileMoved) {
							CString strError;						
							FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
							strError.ReleaseBuffer();
							MsgBox("Could not move the document from '%s' to '%s'. \n\nError: %s",
								strOriginalFilePath ^ strOriginalFileName, strDstPath, strError);
							return;
						}
						

						//if we got here, we succeded in moving the file, now update the data						
						if ((strOriginalFileName != strNewFileName)) {

							CString strUpdateFile; 
							if (bIsPatientPath) {
								strUpdateFile = strNewFileName;
							}
							else {
								strUpdateFile = strNewFilePath ^ strNewFileName;
							}

							if (IDNO == MsgBox(MB_YESNO, "This will move the document from patient %s (%s) to patient %s (%s)\r\nAre you sure you want to continue?", GetActivePatientName(),  strOriginalFilePath ^ strOriginalFileName, GetExistingPatientName(nNewPatientID), strNewFilePath ^ strNewFileName)) {
								return;
							}

							bWarned = TRUE;

							//the file name got changed along the way
							AddStatementToSqlBatch(strSql, "UPDATE MailSent set PathName = '%s' WHERE MailID = %li", strUpdateFile, nMailID);

							//we don't audit file name changes, so no need to audit here							
						}

					}

					if (!bWarned) {
						//output the message again
						if (IDNO == MsgBox(MB_YESNO, "This will move the document from patient %s (%s) to patient %s (%s)\r\nAre you sure you want to continue?", GetActivePatientName(),  strOriginalFilePath ^ strOriginalFileName, GetExistingPatientName(nNewPatientID), strNewFilePath ^ strNewFileName)) {
							return;
						}

					}


					AddStatementToSqlBatch(strSql, "UPDATE MailSent SET PersonID = %li WHERE MailID = %li", nNewPatientID, nMailID);
					// (c.haag 2010-08-27 13:29) - PLID 39473 - We also need to move notes that are assigned to the MailSent record
					AddStatementToSqlBatch(strSql, "UPDATE Notes SET PersonID = %li WHERE MailID = %li", nNewPatientID, nMailID);

					ExecuteSqlBatch(strSql);

					//now audit
					if (nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}

					AuditEvent(m_bInPatientsModule ? GetActivePersonID() : -1, GetActivePersonName(), nAuditTransID, aeiPatientDocDetach, nMailID, strOriginalFileName, "", aepMedium, aetDeleted); 
					AuditEvent(nNewPatientID, GetExistingPatientName(nNewPatientID), nAuditTransID, aeiPatientDocumentAttach, nMailID, "", strNewFileName, aepMedium, aetCreated);					

					if (nAuditTransID != -1) {
						CommitAuditTransaction(nAuditTransID);
					}

					//send a messsage
					// (j.jones 2011-07-22 16:04) - PLID 21784 - this never used the available nMailID before
					// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker for both patients
					CClient::RefreshMailSentTable(m_id, nMailID, GetRowPhotoStatus(nCurSel));
					CClient::RefreshMailSentTable(nNewPatientID, nMailID, GetRowPhotoStatus(nCurSel));
				}
			}
		}

	}NxCatchAllCall(__FUNCTION__, 
		if (nAuditTransID > -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);
}

// (z.manning 2010-01-08 10:13) - PLID 12186
void CHistoryDlg::OnBnClickedHistoryShowGridlines()
{
	try
	{
		if(IsDlgButtonChecked(IDC_HISTORY_SHOW_GRIDLINES) == BST_CHECKED) {
			SetRemotePropertyInt("HistoryShowGrid", 1, 0, GetCurrentUserName());
			m_pDocuments->PutGridVisible(VARIANT_TRUE);
		}
		else {
			SetRemotePropertyInt("HistoryShowGrid", 0, 0, GetCurrentUserName());
			m_pDocuments->PutGridVisible(VARIANT_FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-01-12) PLID 25808 - Added quick button for photo note printing preference
void CHistoryDlg::OnBnClickedPrintPhotoNotes()
{
	try
	{
		if(IsDlgButtonChecked(IDC_PHOTOS_PRINT_NOTES_CHECK)) {
			SetRemotePropertyInt("PhotoPreviewNotes", 1, 0, GetCurrentUserName());
		}
		else {
			SetRemotePropertyInt("PhotoPreviewNotes", 0, 0, GetCurrentUserName());
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-02-02 12:09) - PLID 34287
void CHistoryDlg::OnAttachToCurrentPic()
{
	try
	{
		long nRow = m_pDocuments->GetCurSel();
		if(nRow == -1) {
			return;
		}

		if(GetPicID() == -1) {
			// (z.manning 2010-02-02 15:32) - PLID 34287 - This should not be called outside the PIC
			ASSERT(FALSE);
			return;
		}

		_variant_t varPicID = m_pDocuments->GetValue(nRow, phtdlcPicID);
		if(varPicID.vt != VT_NULL) {
			// (z.manning 2010-02-02 15:36) - PLID 34287 - This file is already associated with a PIC so
			// they should not have been allowed to assign it to this one.
			ASSERT(FALSE);
			return;
		}

		// (z.manning 2010-02-02 15:08) - PLID 34287 - Make sure they have history write permission
		// (e.frazier 2016-05-19 10:31) - Use the permission object corresponding to the module we are in (Patients or Contacts)
		if(!CheckCurrentUserPermissions(GetHistoryPermissionObject(), sptWrite)){
			return;
		}

		long nMailID = VarLong(m_pDocuments->GetValue(nRow, phtdlcMailID));
		ExecuteParamSql("UPDATE MailSent SET PicID = {INT} WHERE MailID = {INT}", GetPicID(), nMailID);
		m_pDocuments->PutValue(nRow, phtdlcPicID, GetPicID());

	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-05-21 13:39) - PLID 38731 - This function returns TRUE if the row can be associated with a
// todo alarm. (Todo Regarding ID = Document ID)
BOOL CHistoryDlg::CanCreateTodoForDocument(long nRow)
{
	CString strPath = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nRow, phtdlcPath)));

	// Do not let users create tasks for Case Histories or Reproductive forms. Everything else is fair game,
	// including eligibility forms. We may tweak this later on.
	if (!strPath.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) ||
		!strPath.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE)
		) 
	{
		return FALSE;
	}
	return TRUE;
}

// (c.haag 2010-05-21 13:39) - PLID 38731 - Assign a todo task to this document.
void CHistoryDlg::OnNewTodo()
{
	try {
		// We don't need to check permissions here; it's done in TaskEditDlg.
		long nCurSel = m_pDocuments->GetCurSel();
		if (nCurSel != sriNoRow) {
			CTaskEditDlg dlg(this);
			dlg.m_nPersonID = m_id;
			dlg.SetNewTaskRegardingOverrides( VarLong(m_pDocuments->GetValue(nCurSel, phtdlcMailID)), ttMailSent );
			//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
			dlg.m_bIsNew = TRUE;
			dlg.DoModal();
		}

	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-05-24 9:48) - PLID 38731 - This function opens a history document. This
// message is actually posted from outside the history tab.
LRESULT CHistoryDlg::OnOpenDocument(WPARAM wParam, LPARAM lParam)
{
	try {
		if (NULL != m_pDocuments) {
			long nMailSentID = (long)wParam;

			// Now find the row corresponding to the document. This will wait for the requery to finish if necessary.
			long nRow = m_pDocuments->FindByColumn(phtdlcMailID, nMailSentID, 0, VARIANT_FALSE);

			// Treat this message like a user left-clicked on the icon column of the row.
			
			if (-1 == nRow) {
				// (j.jones 2012-10-08 11:24) - PLID 46400 - if not found, switch to the Misc. tab
				// and remove all filters, to make sure we're showing all documents
				//switch to the misc. tab, if not already on it
				short nCurTabSel = m_tab->CurSel;
				short nMiscTab = m_tab->GetSize()-1;
				if (nCurTabSel != nMiscTab) {
					//not on the misc. tab, so switch to it
					m_tab->PutCurSel(nMiscTab);
					OnSelectTabDocTabs(nMiscTab, nCurTabSel);

					//try to find the row now
					nRow = m_pDocuments->FindByColumn(phtdlcMailID, nMailSentID, 0, VARIANT_FALSE);
				}

				if (-1 == nRow) {
					//still not shown? remove all filters.
					BOOL bRequery = FALSE;
					if(m_radioDateRange.GetCheck()) {
						m_radioDateRange.SetCheck(FALSE);
						GetDlgItem(IDC_HISTORY_FROM_DATE)->EnableWindow(FALSE);
						GetDlgItem(IDC_HISTORY_TO_DATE)->EnableWindow(FALSE);
						bRequery = TRUE;
					}

					if(IsDlgButtonChecked(IDC_USE_HISTORY_FILTER)) {
						m_checkFilterOnCategory.SetCheck(FALSE);
						m_PhotoViewer.SetUsingCategoryFilter(false);
						GetDlgItem(IDC_HISTORY_FILTER_LIST)->EnableWindow(FALSE);
						bRequery = TRUE;
					}

					if(bRequery) {
						UpdateView();
						//try to find the row now
						nRow = m_pDocuments->FindByColumn(phtdlcMailID, nMailSentID, 0, VARIANT_FALSE);
					}
				}
			}

			if (-1 == nRow) {
				// Hmm, still no row. Perhaps it was deleted.
				AfxMessageBox("The document could not be found. It may have been deleted by another user.", MB_OK | MB_ICONHAND);
			} else {
				HandleDocumentAction(nRow, phtdlcIcon);
			}
		}
		else {
			// We somehow don't have a documents list (a sign of much bigger problems).
			ThrowNxException("Attempted to open a document without a valid document datalist object.");
		}
		
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (d.lange 2010-10-26 16:54) - PLID 41088 - Receiving function for message NXM_DEVICE_IMPORT_STATUS, update Device Import button
LRESULT CHistoryDlg::OnUpdateDeviceImportButton(WPARAM wParam, LPARAM lParam)
{
	try {
		BOOL bImportStatus = (BOOL)wParam;
		
		if(bImportStatus) {
			m_btnDeviceImport.AutoSet(NXB_IMPORTPENDING);
		}else {
			m_btnDeviceImport.AutoSet(NXB_IMPORTBOX);
		}

		m_btnDeviceImport.RedrawWindow();

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.camacho 2013-11-05 15:22) - PLID 59303
void CHistoryDlg::OnDirectMessage()
{	
	
	CDirectMessageSendDlg sendDlg;
	// (b.spivey, May 13th, 2014) - PLID 61804 - set the patient ID so we know if we can attach from a patient's record. 
	sendDlg.SetPatientID(GetActivePersonID());
		
	if(m_pDocuments->CurSel == -1)
		return;

	try {
		// (d.singleton 2014-04-23 17:13) - PLID 61806 - Allow selecting multiple attachments on a patient account and attaching them all to send on a direct message.
		// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		long p = m_pDocuments->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;
		while(p) {
			m_pDocuments->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);
			long nCurRow = pRow->GetIndex();

			if(pDisp) {
				pDisp->Release();
			}					

			_variant_t varName = m_pDocuments->GetValue(nCurRow, phtdlcName);
			_variant_t varMailID = m_pDocuments->GetValue(nCurRow, phtdlcMailID);
			if(varName.vt == VT_I4) {
				//DRT 8/3/2005 - PLID 17153 - This is a packet, we need to print each document, not just the first
				//find the path of all the documents in this packet
				_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, PathName FROM MailSent WHERE MergedPacketID = (SELECT MergedPacketID FROM MailSent WHERE MailID = %li) ORDER BY MailID ASC", VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID)));
				while(!rsMailSent->eof) {

					//PLID 18609: check to see if it is a multipatdoc
					CString strFile = AdoFldString(rsMailSent, "PathName");
					CString strFilePath = VarString(m_pDocuments->GetValue(nCurRow, phtdlcFilePath), "");

					//check to see if the path exists
					if (! DoesExist(strFilePath ^ strFile)) {

						//see if it is a multipatdoc
						if (strFile.Find("MultiPatDoc") != -1) {
							//change the path to be the --25 path
							strFilePath = GetSharedPath() ^ "Documents\\---25\\";
						}
					}
					//add to attachment list
					sendDlg.AddToAttachments(strFilePath, strFile, VarLong(varMailID));
					rsMailSent->MoveNext();
				}


			}
			else {
				//This is your standard document to be printed

				//PLID 18609: check to see if it is a multipatdoc
				CString strFilePath = VarString(m_pDocuments->GetValue(nCurRow, phtdlcFilePath));
				CString strDocName = VarString(m_pDocuments->GetValue(nCurRow, phtdlcFilename));
				long nMailID = VarLong(m_pDocuments->GetValue(nCurRow, phtdlcMailID));
				//see if it exists in the patient file
				if (! DoesExist(strFilePath ^ strDocName)) {

					//see if it is a multipatdoc
					if (strDocName.Find("MultiPatDoc") != -1) {
						//change the path to be the --25 path
						strFilePath = GetSharedPath() ^ "Documents\\---25\\";
					}
				}
				//update attachment list and open direct message dialog
				sendDlg.AddToAttachments(strFilePath ,strDocName, nMailID);

				//is this a CCDA?
				// (j.gruber 2013-11-11 11:51) - PLID 59403 - send the pdf with the xml
				if (NxXMLUtils::IsCCDAFile(strFilePath^strDocName))
				{
					CString strMailSentID = AsString(nMailID);

					//we need to add both the xml and a pdf, so get the PDF file now
					NexTech_Accessor::_HistoryEntryPDFResultPtr pResult= GetAPI()->GetHistoryEntryPDF(GetAPISubkey(), GetAPILoginToken(),_bstr_t(strMailSentID));
					CString strExt = FileUtils::GetFileExtension(strDocName);
					CString strPDFFileName = strDocName;
					strPDFFileName.Replace(strExt, "pdf");						
					Nx::SafeArray<BYTE> fileBytes = pResult->PDFFile;
					sendDlg.AddToAttachments(fileBytes,  strPDFFileName);
				}
			}
		}
		sendDlg.DoModal();
	}
	NxCatchAll("CHistoryDlg::OnDirectMessage");


}

// (s.dhole 2013-11-01 12:16) - PLID 59278 Call Medicatio  reconciliation
void CHistoryDlg::OnReconcileMedication()
{
try
	{
		if (!CheckCurrentUserPermissions(bioPatientMedication, sptRead)){
			return;
		}
		CWaitCursor cwait;
		long nMailID = VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID));
		CReconciliationDlg dlg(GetActivePatientID(),nMailID ,this);
		dlg.SetBackColor(  GetNxColor(GNC_PATIENT_STATUS, 1));
		dlg.m_nReconciliationType= CReconciliationDlg::erMedication;
		if(dlg.DoModal() == IDOK) {
			CString strMsg = dlg.m_strResultText;
			if (!strMsg.IsEmpty()){
				//(s.dhole 2013-01-10 12:16) - PLID 60259  replace MsgBox with   AfxMessageBox , which causing issue with "%"
				AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION); 
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-11-01 12:16) - PLID 59278 Call Allergy  reconciliation
void CHistoryDlg::OnReconcileAllergy()
{
try
	{
		if (!CheckCurrentUserPermissions(bioPatientMedication, sptRead)){
			return;
		}
		CWaitCursor cwait;
		long nMailID = VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID));
		CReconciliationDlg dlg(GetActivePatientID(),nMailID, this);
		dlg.SetBackColor(  GetNxColor(GNC_PATIENT_STATUS, 1));
		dlg.m_nReconciliationType= CReconciliationDlg::erAllergy;
		if(dlg.DoModal() == IDOK) {
			CString strMsg = dlg.m_strResultText;
			if (!strMsg.IsEmpty()){
				//(s.dhole 2013-01-10 12:16) - PLID 60259  replace MsgBox with   AfxMessageBox , which causing issue with "%"
				AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION); 
			}
		}

	} NxCatchAll(__FUNCTION__);
}


// (s.dhole 2013-11-01 12:16) - PLID 59278 Call Problem reconciliation
void CHistoryDlg::OnReconcileProblem()
{
try
	{
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptRead)){
			return;
		}
		CWaitCursor cwait;
		long nMailID = VarLong(m_pDocuments->GetValue(m_pDocuments->CurSel, phtdlcMailID));
		CReconciliationDlg dlg(GetActivePatientID(),nMailID , this);
		dlg.SetBackColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		dlg.m_nReconciliationType= CReconciliationDlg::erProblem;
		if(dlg.DoModal() == IDOK) {
			CString strMsg = dlg.m_strResultText;
			if (!strMsg.IsEmpty()){
				//(s.dhole 2013-01-10 12:16) - PLID 60259  replace MsgBox with   AfxMessageBox , which causing issue with "%"
				AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION); 
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-11-01 12:16) - PLID 59278  Check if selected file is valid CCDA document 
BOOL CHistoryDlg::IsValidCCDADocumentImport(CString strFileFullPath)
{

	if (NxXMLUtils::IsCCDAFile(strFileFullPath)) {
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


// (s.dhole 2013-11-01 12:16) - PLID 59278  Check if selected file is valid CCDA document 
CString  CHistoryDlg::GetFileFullPath()
{
	long nCurSel = m_pDocuments->GetCurSel();
	CString strFullPath ="";

	if (nCurSel != sriNoRow) {
		CString filename = (LPCTSTR)(_bstr_t(m_pDocuments->GetValue(nCurSel, phtdlcPath)));
		if (filename.Find('\\') == -1) {
			// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
			// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
			strFullPath = GetCurPatientDocumentPath() ^ filename;
		}
		else {
		//use the path that was provided
			strFullPath = filename;
		}
	}
	return strFullPath;
}

// (j.jones 2014-08-04 17:15) - PLID 63159 - helper function to decide if a file is a photo
// based on the IsPhoto status and the path name
bool CHistoryDlg::IsRowAPhoto(long nRow)
{
	if (nRow == -1) {
		//this function should not have been called
		ASSERT(FALSE);
		return false;
	}

	_variant_t varIsPhoto = m_pDocuments->GetValue(nRow, phtdlcIsPhoto);
	if (varIsPhoto.vt == VT_BOOL) {
		//return the true IsPhoto value
		if (VarBool(varIsPhoto)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (varIsPhoto.vt == VT_I4) {
		//return the true IsPhoto value
		if (VarLong(varIsPhoto) == 1) {
			return true;
		}
		else {
			return false;
		}
	}

	//if we're still here, calculate by the image file extension
	CString strFileName = VarString(m_pDocuments->GetValue(nRow, phtdlcFilename), "");
	return IsImageFile(strFileName) ? true : false;
}

// (j.jones 2014-08-04 17:15) - PLID 63159 - helper function to decide if a file is a photo
// based on the IsPhoto status and the path name
TableCheckerDetailIndex::MailSent_PhotoStatus CHistoryDlg::GetRowPhotoStatus(long nRow)
{
	if (nRow == -1) {
		//this function should not have been called
		ASSERT(FALSE);
		return TableCheckerDetailIndex::MailSent_PhotoStatus::mspsUnknown;
	}

	if (IsRowAPhoto(nRow)) {
		return TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto;
	}
	else {
		return TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto;
	}
}

// (a.wilson 2014-08-11 08:20) - PLID 63246 - ensure the category filter is uptodate.
void CHistoryDlg::EnsureUpdatedCategoryFilter(const long & nID /* = -1 */)
{
	if (m_NoteChecker.Changed()) {
		try {
			//requery the combo source
			IColumnSettingsPtr pCol = m_pDocuments->GetColumn(phtdlcCategory);
			pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT -1, '{No Category}' ORDER BY Description"));

			//From OnTableChanged()
			if (nID > -1) {
				long nRow = m_pFilter->CurSel;
				long nCurrentID = -4;
				if (nRow != sriNoRow) {
					nCurrentID = VarLong(m_pFilter->GetValue(nRow, 0), -4); // -4 is not used as an id within the categories.
				}

				_RecordsetPtr rs = CreateParamRecordset("SELECT Description FROM NoteCatsF WHERE ID = {INT}", nID);

				//Remove the row so that we can either keep it removed or reinsert its updated state.
				m_pFilter->RemoveRow(m_pFilter->FindByColumn(0, nID, 0, VARIANT_FALSE));
				if (!rs->eof) {
					IRowSettingsPtr pRow = m_pFilter->GetRow(sriGetNewRow);
					pRow->PutValue(0, nID);
					pRow->PutValue(1, _bstr_t(AdoFldString(rs, "Description")));
					m_pFilter->AddRow(pRow);
				}
				//attempt to reselect the previous option if it was the row we updated.
				if (nRow != sriNoRow && nCurrentID > -4 && nCurrentID == nID) {
					m_pFilter->SetSelByColumn(0, nCurrentID);
					if (m_pFilter->CurSel == sriNoRow) {
						//if the currentid was wiped out then set to {all}
						m_pFilter->SetSelByColumn(0, -2);
					}
				}
			}
			//From UpdateView()
			else {
				long nCurrentID = -4; // -4 is not used as an id within the categories.
				if (m_pFilter->CurSel != sriNoRow) {
					nCurrentID = VarLong(m_pFilter->GetValue(m_pFilter->CurSel, 0), -4);
				}
				m_pFilter->Requery();

				if (nCurrentID > -4) {
					m_pFilter->SetSelByColumn(0, nCurrentID);
					if (m_pFilter->CurSel == sriNoRow) {
						//if the currentid was wiped out then set to {all}
						m_pFilter->SetSelByColumn(0, -2);
					}
				}
			}
		} NxCatchAll(__FUNCTION__);
	}
}

// (e.frazier 2016-05-18 16:23) - PLID-34501 - Return the permission object corresponding to the module we are in (Patients or Contacts)
EBuiltInObjectIDs CHistoryDlg::GetHistoryPermissionObject()
{
	return (m_bInPatientsModule) ? bioPatientHistory : bioContactHistory;
}