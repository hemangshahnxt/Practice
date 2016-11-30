// ProcedureSectionEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "ProcedureSectionEditDlg.h"
#include "SelectSourceProcedureDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "SpellExUtils.h"
#include "NexFormsUtils.h" // (z.manning, 08/03/2007) - PLID 18359 - Move the functions to add & remove the need review text to  NexFormsUtils.

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum ESectionComboColumn {
	psesccName = 0,
	psesccNeedsReviewed = 1, 
	psesccArrayIndex = 2
};

enum EProcedureComboColumn {
	pseProcColID = 0, 
	pseProcColName = 1,
	pseProcColNeedReview = 2
};

#define	SECTION_COUNT	25

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_CUT	50100
#define ID_COPY	50101
#define ID_PASTE	50102

#define IDM_PREVIEW_FIELD 50103
#define IDM_PREVIEW_PROC 50104

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProcedureSectionEditDlg dialog


CProcedureSectionEditDlg::CProcedureSectionEditDlg(CWnd* pParent /*=NULL*/, long nProcedureID /* = -1 */)
	: CNxDialog(CProcedureSectionEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureSectionEditDlg)
	//}}AFX_DATA_INIT
	
	m_nProcedureID = nProcedureID;
	m_nCurrentSection = -1;
	m_arSections = NULL;
	m_bInitializing = true;
	m_bNavigating = false;
	m_bNeedsReviewedWasSet = false;
}

void CProcedureSectionEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureSectionEditDlg)
	DDX_Control(pDX, IDC_PROC_SPELL_CHECK, m_btnSpellCheck);
	DDX_Control(pDX, IDC_PREVIEW_ITEM, m_btnPreviewItem);
	DDX_Control(pDX, IDC_COPY_SECTIONS, m_btnCopySections);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_RENAME_BTN, m_btnRename);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
	DDX_Control(pDX, IDC_PROC_SECTION_ITALIC, m_btnItalic);
	DDX_Control(pDX, IDC_PROC_SECTION_SUPERSCRIPT, m_btnSuperScript);
	DDX_Control(pDX, IDC_PROC_SECTION_SUBSCRIPT, m_btnSubScript);
	DDX_Control(pDX, IDC_PROC_SECTION_BULLETS, m_btnBullets);
	DDX_Control(pDX, IDC_PROC_SECTION_ALIGN_RIGHT, m_btnAlignRight);
	DDX_Control(pDX, IDC_PROC_SECTION_ALIGN_CENTER, m_btnAlignCenter);
	DDX_Control(pDX, IDC_PROC_SECTION_ALIGN_LEFT, m_btnAlignLeft);
	DDX_Control(pDX, IDC_PROC_SECTION_UNDERLINE, m_btnUnderline);
	DDX_Control(pDX, IDC_PROC_SECTION_BOLD, m_btnBold);
	DDX_Control(pDX, IDC_PROC_SECTION_UNDO, m_btnUndo);
	DDX_Control(pDX, IDC_SECTION_EDIT, m_richeditSectionText);
	DDX_Control(pDX, IDC_PROC_SECTION_FONT_HEIGHT, m_nxeditProcSectionFontHeight);
	DDX_Control(pDX, IDC_STATIC_PROCEDURE, m_nxstaticProcedure);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEEDS_REVIEWED, m_btnNeedsReviewed);
	DDX_Control(pDX, IDC_CHECK_FILTER_NEED_REVIEWED, m_btnFilterNeedReviewed);
	DDX_Control(pDX, IDC_CHECK_FILTER_SECTIONS_WITH_CONTENT, m_btnFilterWithContent);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CProcedureSectionEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureSectionEditDlg)
	ON_EN_CHANGE(IDC_SECTION_EDIT, OnChangeSectionEdit)
	ON_BN_CLICKED(IDC_RENAME_BTN, OnRenameBtn)
	ON_BN_CLICKED(IDC_COPY_SECTIONS, OnCopySections)
	ON_NOTIFY(EN_SELCHANGE, IDC_SECTION_EDIT, OnSelchangeSectionEdit)
	ON_BN_CLICKED(IDC_PROC_SECTION_BOLD, OnProcSectionBold)
	ON_BN_CLICKED(IDC_PROC_SECTION_ITALIC, OnProcSectionItalic)
	ON_BN_CLICKED(IDC_PROC_SECTION_UNDERLINE, OnProcSectionUnderline)
	ON_EN_CHANGE(IDC_PROC_SECTION_FONT_HEIGHT, OnChangeProcSectionFontHeight)
	ON_EN_KILLFOCUS(IDC_PROC_SECTION_FONT_HEIGHT, OnKillfocusProcSectionFontHeight)
	ON_NOTIFY(UDN_DELTAPOS, IDC_FONT_HEIGHT_SPIN, OnDeltaposFontHeightSpin)
	ON_BN_CLICKED(IDC_PROC_SECTION_UNDO, OnUndo)
	ON_BN_CLICKED(IDC_PROC_SECTION_ALIGN_CENTER, OnProcSectionAlignCenter)
	ON_BN_CLICKED(IDC_PROC_SECTION_ALIGN_LEFT, OnProcSectionAlignLeft)
	ON_BN_CLICKED(IDC_PROC_SECTION_ALIGN_RIGHT, OnProcSectionAlignRight)
	ON_BN_CLICKED(IDC_PROC_SECTION_BULLETS, OnProcSectionBullets)
	ON_NOTIFY(NM_RCLICK, IDC_SECTION_EDIT, OnRclickSectionEdit)
	ON_COMMAND(ID_CUT, OnCut)
	ON_COMMAND(ID_COPY, OnCopy)
	ON_COMMAND(ID_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_PROC_SECTION_SUBSCRIPT, OnProcSectionSubscript)
	ON_BN_CLICKED(IDC_PROC_SECTION_SUPERSCRIPT, OnProcSectionSuperscript)
	ON_BN_CLICKED(IDC_NEEDS_REVIEWED, OnNeedsReviewed)
	ON_BN_CLICKED(IDC_PREVIEW_ITEM, OnPreviewItem)
	ON_COMMAND(IDM_PREVIEW_FIELD, OnPreviewField)
	ON_COMMAND(IDM_PREVIEW_PROC, OnPreviewProc)
	ON_BN_CLICKED(IDC_BUTTON_BACK, OnButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_CHECK_FILTER_NEED_REVIEWED, OnCheckFilterNeedReviewed)
	ON_BN_CLICKED(IDC_CHECK_FILTER_SECTIONS_WITH_CONTENT, OnCheckFilterSectionsWithContent)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_PROC_SPELL_CHECK, OnProcSpellCheck)
	ON_COMMAND(ID_PROCEDURE_SECTION_MENU_UPDATE_FONT, OnProcedureSectionMenuUpdateFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CProcedureSectionEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureSectionEditDlg)
	ON_EVENT(CProcedureSectionEditDlg, IDC_PROC_SECTION_FONT_LIST, 16 /* SelChosen */, OnSelChosenProcSectionFontList, VTS_I4)
	ON_EVENT(CProcedureSectionEditDlg, IDC_PROC_SECTION_COLOR, 16 /* SelChosen */, OnSelChosenProcSectionColor, VTS_I4)
	ON_EVENT(CProcedureSectionEditDlg, IDC_PROCEDURE_CONTENTS_LIST, 16 /* SelChosen */, OnSelChosenProcedureContentsList, VTS_DISPATCH)
	ON_EVENT(CProcedureSectionEditDlg, IDC_PROCEDURE_CONTENTS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedProcedureContentsList, VTS_I2)
	ON_EVENT(CProcedureSectionEditDlg, IDC_CUSTOM_SECTION_COMBO, 16 /* SelChosen */, OnSelChosenCustomSectionCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

CProcedureSectionEditDlg::~CProcedureSectionEditDlg()
{
	if(m_arSections != NULL)
		delete [] m_arSections;
}

//Callback function usede by EnumFontFamiliesEx
int _stdcall Callback_EnumFontFamEx(const LOGFONT *plf, const TEXTMETRIC *ptm, DWORD dwFontType, LPARAM lParam)
{
	_DNxDataListPtr pFontList = (IDispatch*)lParam;
	//Have we already entered a font for this name?
	if(pFontList->FindByColumn(0, _bstr_t(CString(plf->lfFaceName)), 0, false) == -1) {
		IRowSettingsPtr pRow = pFontList->GetRow(-1);
		pRow->PutValue(0, _bstr_t(CString(plf->lfFaceName)));
		pRow->PutValue(1, (long)plf->lfPitchAndFamily);
		pRow->PutValue(2, (long)plf->lfCharSet);
		pFontList->AddRow(pRow);
	}
	return 1; //Non-zero => keep enumerating
}

BOOL CProcedureSectionEditDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
	
	//First, set up our formatting section.
	// (z.manning, 11/29/2007) - PLID 28232 - These 9 formatting icons were originall regular push-buttons but
	// were later changed to NxIconButtons by 23861. However, NxIconButtons do not support push-buttons which
	// caused many of these buttons to not work, so I am changing them back to regular CButton push-buttons.
	
	// (a.walling 2008-06-03 10:06) - PLID 27686 - Use LR_SHARED for these icons
	m_btnBold.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnItalic.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ITALIC),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnUnderline.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_UNDERLINE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnAlignLeft.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ALIGN_LEFT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnAlignCenter.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ALIGN_CENTER),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnAlignRight.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ALIGN_RIGHT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnBullets.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BULLETS),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnSubScript.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_SUBSCRIPT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	m_btnSuperScript.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_SUPERSCRIPT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));

	m_btnSpellCheck.AutoSet(NXB_SPELLCHECK);
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	//DRT 5/16/2008 - PLID 29469 - We decided to remove the WIZ styles
	m_btnRename.AutoSet(NXB_MODIFY);
	m_btnPreviewItem.AutoSet(NXB_PRINT_PREV);
	
	m_pFontList = BindNxDataListCtrl(IDC_PROC_SECTION_FONT_LIST, false);

	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET; //Will enumerate fonts for all character sets.
	lf.lfFaceName[0] = '\0'; //Will enumerate all fonts.
	// (a.walling 2008-07-03 16:15) - PLID 30623 - Fix GDI leak
	CDC* pDC = GetDC();
	EnumFontFamiliesEx(pDC->GetSafeHdc(), &lf, Callback_EnumFontFamEx, (LPARAM)(IDispatch*)m_pFontList, 0);
	ReleaseDC(pDC);

	m_pColorList = BindNxDataListCtrl(IDC_PROC_SECTION_COLOR, false);
	//Automatic
	IRowSettingsPtr pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Automatic"));
	m_pColorList->AddRow(pRow);
	//Red
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Red"));
	pRow->PutForeColor(RGB(255,0,0));
	pRow->PutForeColorSel(RGB(255,0,0));
	m_pColorList->AddRow(pRow);
	//Green
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Green"));
	pRow->PutForeColor(RGB(0,255,0));
	pRow->PutForeColorSel(RGB(0,255,0));
	m_pColorList->AddRow(pRow);
	//Blue
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Blue"));
	pRow->PutForeColor(RGB(0,0,255));
	pRow->PutForeColorSel(RGB(0,0,255));
	m_pColorList->AddRow(pRow);
	//Yellow
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Yellow"));
	pRow->PutForeColor(RGB(255,255,0));
	pRow->PutForeColorSel(RGB(255,255,0));
	m_pColorList->AddRow(pRow);
	//Cyan
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Cyan"));
	pRow->PutForeColor(RGB(0,255,255));
	pRow->PutForeColorSel(RGB(0,255,255));
	m_pColorList->AddRow(pRow);
	//Magenta
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Magenta"));
	pRow->PutForeColor(RGB(255,0,255));
	pRow->PutForeColorSel(RGB(255,0,255));
	m_pColorList->AddRow(pRow);
	//Black
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Black"));
	pRow->PutForeColor(RGB(0,0,0));
	pRow->PutForeColorSel(RGB(0,0,0));
	m_pColorList->AddRow(pRow);
	//Gray
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Gray"));
	pRow->PutForeColor(RGB(127,127,127));
	pRow->PutForeColorSel(RGB(127,127,127));
	m_pColorList->AddRow(pRow);
	//Custom
	pRow = m_pColorList->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Custom"));
	m_pColorList->AddRow(pRow);

	m_btnUndo.SetIcon(IDI_UNDO);
	m_btnUndo.SetDisabledIcon(IDI_UNDO_DISABLED);

	m_richeditSectionText.SetEventMask(m_richeditSectionText.GetEventMask() | ENM_CHANGE | ENM_SELCHANGE);
	m_richeditSectionText.SetOptions(ECOOP_OR, ECO_SAVESEL);
	m_richeditSectionText.LimitText(0xFFFFFFF);
	
	// (d.moore 2007-06-20 17:12) - PLID 23861 - Some checkboxes were added to be used as a filter.
	//  These all need to default to checked when the dialog opens.
	CheckDlgButton(IDC_CHECK_FILTER_NEED_REVIEWED, TRUE);
	CheckDlgButton(IDC_CHECK_FILTER_SECTIONS_WITH_CONTENT, TRUE);

	// Bind to the datalist
	try {
		m_dlSectionCombo = BindNxDataList2Ctrl(IDC_CUSTOM_SECTION_COMBO, false);
		InitSectionArray();

		// (d.moore 2007-06-19 11:02) - PLID 23861 - Added a procedure dropdown list.
		m_pProcedureList = BindNxDataList2Ctrl(IDC_PROCEDURE_CONTENTS_LIST, false);
		LoadProcedures();

	} NxCatchAllCall("CProcedureSectionEditDlg::OnInitDialog", {
		EndDialog(IDCANCEL); 
		return FALSE;
	});
	
	// (z.manning, 10/11/2007) - PLID 27719 - Only enable the update font option if they have a NexForms license.
	CMenu *pmnu = GetMenu();
	if(pmnu != NULL) {
		if(g_pLicense->CheckForLicense(CLicense::lcNexForms, CLicense::cflrSilent)) {
			pmnu->EnableMenuItem(ID_PROCEDURE_SECTION_MENU_UPDATE_FONT, MF_BYCOMMAND|MF_ENABLED);
		}
		else {
			pmnu->EnableMenuItem(ID_PROCEDURE_SECTION_MENU_UPDATE_FONT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		}
	}
	else {
		// (z.manning, 10/11/2007) - PLID 27719 - Uh, someone must have removed the menu from the resource editor.
		ASSERT(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////////////////////
// CProcedureSectionEditDlg message handlers

#define TEXT_INCREMENT	8000
BOOL CProcedureSectionEditDlg::ValidateAndSave()
{
	try {
		for(int i = 0; i < SECTION_COUNT; i++) {
			if(m_arSections[i].bChanged) {
				if(m_arSections[i].strText.GetLength() > TEXT_INCREMENT) {
					CStringArray arTextSections;
					for(int j = 0; j+TEXT_INCREMENT < m_arSections[i].strText.GetLength(); j += TEXT_INCREMENT) {
						arTextSections.Add(m_arSections[i].strText.Mid(j,TEXT_INCREMENT));
					}
					arTextSections.Add(m_arSections[i].strText.Mid(j));
					ExecuteSql("UPDATE ProcedureT SET %s = '%s', %s = %li WHERE ID = %li", m_arSections[i].strFieldName,
						_Q(arTextSections[0]), m_arSections[i].strReviewFieldName, m_arSections[i].bNeedsReview, m_nProcedureID);
					for(j = 1; j < arTextSections.GetSize(); j++) {
						CString str = arTextSections[j];
						ExecuteSql("DECLARE @ptrval binary(16) "							
							"SELECT @ptrval = TEXTPTR(%s) "
							"FROM ProcedureT "
							"WHERE ProcedureT.ID = %li "
							"UPDATETEXT ProcedureT.%s @ptrval NULL 0 '%s' ",
							m_arSections[i].strFieldName, m_nProcedureID, m_arSections[i].strFieldName, _Q(arTextSections[j]));
						/*ExecuteSql("UPDATE ProcedureT SET %s = %s + '%s' WHERE ID = %li", m_arSections[i].strFieldName, m_arSections[i].strFieldName,
							_Q(arTextSections[j]), m_nProcedureID);*/
					}
				}
				else {
					ExecuteSql("UPDATE ProcedureT SET %s = '%s', %s = %li WHERE ID = %li", m_arSections[i].strFieldName, _Q(m_arSections[i].strText), 
						m_arSections[i].strReviewFieldName, m_arSections[i].bNeedsReview, m_nProcedureID);
				}
				m_arSections[i].bChanged = false;
			}
		}
		return TRUE;
	} NxCatchAll("Error in CProcedureSectionEditDlg::ValidateAndSave()");
	return FALSE;
}

BOOL CProcedureSectionEditDlg::Save()
{
	if (m_nCurrentSection < 0 || m_nCurrentSection >= SECTION_COUNT) {
		return false;
	}

	//PLID 21789 - this was taken from the on OK so it could do everything on did except close the dialog and return a bool
	try {
		CWaitCursor cuWait;
		if(m_arSections[m_nCurrentSection].bChanged) {
			CString strText;
			GetRtfText(&m_richeditSectionText, strText, SF_RTF);
			m_arSections[m_nCurrentSection].strText = strText;
		}

		if (ValidateAndSave()) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}NxCatchAllCall("Error in CProcedureSectionEditDlg::Save()", return FALSE;);
}

void CProcedureSectionEditDlg::OnOK() 
{
	try {
		if (Save()) {
			EndDialog(IDOK);
		}
	} NxCatchAll ("Error In: CProcedureSectionEditDlg::OnOK");
}

// (z.manning, 05/02/2008) - PLID 29867 - Added OnCancel handler
void CProcedureSectionEditDlg::OnCancel()
{
	try
	{
		OnClose();

	}NxCatchAll("CProcedureSectionEditDlg::OnCancel");
}

void CProcedureSectionEditDlg::OnSelChosenCustomSectionCombo(LPDISPATCH lpRow) 
{
	// (d.moore 2007-06-19 16:56) - PLID 23861 - Modified m_dlSectionCombo to NxDataList2.
	try {
		m_bNavigating = true;
		StoreSectionText();
		
		if (lpRow == NULL) {
			// If no section was selected then don't let them do anything else until they pick one.
			m_nCurrentSection = -1;
			EnableControls(FALSE);
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_nCurrentSection = VarLong(pRow->GetValue(psesccArrayIndex ));
		EnableControls(TRUE);
		LoadDataForSection(m_nCurrentSection);
		EnsureNextBackButtonState();
	} NxCatchAll ("Error In: CProcedureSectionEditDlg::OnSelChosenCustomSectionCombo");
}

void CProcedureSectionEditDlg::LoadDataForSection(long nSection)
{
	// (d.moore 2007-06-21 17:04) - PLID 23861 - This was functionality originally in OnSelChosenCustomSectionCombo
	//  but it was being called in an awkward way from several points throughout this class.
	try {
		m_bNavigating = true;

		// (d.moore 2007-07-05 11:16) - PLID 23861 - We need to track when the 'Needs Reviewed' checkbox
		//  is actually set to checked. This is not the same thing as m_arSections[x].bNeedsReview being set.
		m_bNeedsReviewedWasSet = false;
		
		if (nSection < 0 || nSection >= SECTION_COUNT) {
			EnableControls(FALSE);
			return;
		}
		
		// If the row passed in is valid then proceed to load the data.
		CWaitCursor cuWait;
		EnableControls(TRUE);
		
		//Enable/disable the Rename button
		GetDlgItem(IDC_RENAME_BTN)->EnableWindow(m_arSections[nSection].nCustomFieldID != -1);

		//DRT 7/7/2004 - PLID 13349 - Review button
		CheckDlgButton(IDC_NEEDS_REVIEWED, m_arSections[nSection].bNeedsReview);

		//Load the appropriate text.
		SetRtfText(&m_richeditSectionText, (BYTE*)m_arSections[nSection].strText.GetBuffer(m_arSections[nSection].strText.GetLength()), m_arSections[nSection].strText.GetLength(), SF_RTF);
		//TES 5/4/2004: For reasons I don't claim to understand, rich edit will not properly display the scrollbar, nor return
		//the correct values for its format, unless you load the text once, load something else, then load the text again.
		CString strText = "\r\n";
		SetRtfText(&m_richeditSectionText, (BYTE*)strText.GetBuffer(strText.GetLength()), strText.GetLength(), SF_TEXT);
		SetRtfText(&m_richeditSectionText, (BYTE*)m_arSections[nSection].strText.GetBuffer(m_arSections[nSection].strText.GetLength()), m_arSections[nSection].strText.GetLength(), SF_RTF);

		//Set the format controls.
		SyncControlsWithText();
		m_bNavigating = false;
	} NxCatchAll("Error In: CProcedureSectionEditDlg::LoadDataForSection");
}

void CProcedureSectionEditDlg::OnChangeSectionEdit() 
{
	// (d.moore 2007-06-19 16:56) - PLID 23861 - Modified m_dlSectionCombo to NxDataList2.
	try {
		if(!m_bInitializing && !m_bNavigating) {
			// Decide what section is selected
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->CurSel;
			ASSERT(pRow != NULL);
			if (pRow != NULL) {
				// Get the text and save it in the correct string
				//TES 5/14/2004: Due to a bug in MS, getting the text will empty the undo buffer.  So, we won't get the text
				//until the last minute, we'll just flag it for now.  When XP SP2 comes out, maybe we'll restore this code.
				/*CString strDest;
				DWORD dwErr = GetRtfText(m_richeditSectionText, strDest, SF_RTF);
				if(dwErr == 0) {
					m_arSections[nCurSel].strText = strDest;*/
					long nLineIndex = VarLong(pRow->GetValue(psesccArrayIndex));
					m_arSections[nLineIndex].bChanged = true;
				//}
			} else {
				// Do nothing
			}
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnChangeSectionEdit");
}


void CProcedureSectionEditDlg::OnRenameBtn() 
{
	try {
		// (d.moore 2007-06-19 16:56) - PLID 23861 - Modified m_dlSectionCombo to NxDataList2.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->CurSel;
		if(pRow == NULL)
		{
			return;
		}	
		
		// (d.moore 2007-06-19 17:07) - PLID 23861 - Get the index for the Section in the list.
		//  This value is used to reference the m_arSections array.
		long nListIndex = VarLong(pRow->GetValue(psesccArrayIndex));

		CString strNewName;
		int nReturn = InputBoxLimited(this, "Enter the new name for this section:", strNewName, "",50,false,false,NULL);
		if(nReturn == IDOK) {
			ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strNewName), m_arSections[nListIndex].nCustomFieldID);
			// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
			// update the global cache so it won't have to be reloaded in its entirety.
			long nFieldID = m_arSections[nListIndex].nCustomFieldID;
			SetCustomFieldNameCachedValue(nFieldID, strNewName);
			// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
			CClient::RefreshTable_CustomFieldName(nFieldID, strNewName);
			m_arSections[nListIndex].strDisplayName = strNewName;
			LoadSectionList();
			m_nCurrentSection = SetSectionSelection(m_nCurrentSection);
		}
	}NxCatchAll("Error in CProcedureSectionEditDlg::OnRenameBtn()");
}

void CProcedureSectionEditDlg::LoadSectionList()
{
	try {
		if(m_arSections == NULL) {
			return;
		}
		
		m_dlSectionCombo->Clear();
		if (m_nProcedureID <= 0) {
			return;
		}

		CString strTemp = "SELECT ";
		for(int i = 0; i < SECTION_COUNT; i++) {
			strTemp += m_arSections[i].strFieldName + ", ";
			strTemp += m_arSections[i].strReviewFieldName + ", ";
		}
		strTemp = strTemp.Left(strTemp.GetLength()-2);
		strTemp += " FROM ProcedureT";
		CString strQuery;
		strQuery.Format("%s WHERE ID = %li", strTemp, m_nProcedureID);
		_RecordsetPtr rsSections = CreateRecordsetStd(strQuery);
		FieldsPtr fSections = rsSections->Fields;
		for(i = 0; i < SECTION_COUNT; i++) {
			m_arSections[i].strText = AdoFldString(fSections, m_arSections[i].strFieldName, "");
			m_arSections[i].bNeedsReview = AdoFldBool(fSections, m_arSections[i].strReviewFieldName, 0);
			m_arSections[i].bChanged = false;
		}
		
		// (d.moore 2007-06-19 17:55) - PLID 23861 - Modified m_dlSectionCombo to be a NxDataList2
		//  Also added filters for the list.
		BOOL bShowOnlyNeedsReviewed = IsDlgButtonChecked(IDC_CHECK_FILTER_NEED_REVIEWED);
		BOOL bShowOnlySectionsWithContent = IsDlgButtonChecked(IDC_CHECK_FILTER_SECTIONS_WITH_CONTENT);
		BOOL bDontShow, bMarkedReviewed, bHasContent;

		for(i = 0; i < SECTION_COUNT; i++) {
			// Remove any formatting text from the CRichTextCtrl.
			CString sContent = m_arSections[i].strText;
			sContent = ConvertTextFormat(sContent, tfRichText, tfPlainText);
			sContent.TrimLeft();
			
			// Now check to see if the section for content and review status.
			bMarkedReviewed = m_arSections[i].bNeedsReview;
			bHasContent = !sContent.IsEmpty();

			bDontShow = ((bShowOnlyNeedsReviewed && !bMarkedReviewed) 
				|| (bShowOnlySectionsWithContent && !bHasContent));
			
			if (!bDontShow) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->GetNewRow();
				pRow->PutValue(psesccName, _bstr_t(m_arSections[i].strDisplayName));
				pRow->PutValue(psesccNeedsReviewed, (VARIANT_BOOL)m_arSections[i].bNeedsReview);
				pRow->PutValue(psesccArrayIndex, (long)i);
				m_dlSectionCombo->AddRowAtEnd(pRow, NULL);
				ColorSectionRow(pRow);
			}
		}

	}NxCatchAll("Error Loading Section List");
}

void CProcedureSectionEditDlg::InitSectionArray()
{
	try{
		if(m_arSections != NULL) {
			delete m_arSections;
			m_arSections = NULL;
		}

		m_arSections = new ProcSection[SECTION_COUNT];

		//First, everything that's hard-coded.
		//DRT 9/19/2006 - PLID 22599 - Reordered these to be alphabetical.
		m_arSections[0].strDisplayName = "Alternatives";
		m_arSections[0].strFieldName = "Alternatives";
		m_arSections[0].bChanged = false;
		m_arSections[0].nCustomFieldID = -1;
		m_arSections[0].strReviewFieldName = "ReviewAlternatives";

		m_arSections[1].strDisplayName = "Bandages";
		m_arSections[1].strFieldName = "Bandages";
		m_arSections[1].bChanged = false;
		m_arSections[1].nCustomFieldID = -1;
		m_arSections[1].strReviewFieldName = "ReviewBandages";

		m_arSections[2].strDisplayName = "Complications";
		m_arSections[2].strFieldName = "Complications";
		m_arSections[2].bChanged = false;
		m_arSections[2].nCustomFieldID = -1;
		m_arSections[2].strReviewFieldName = "ReviewComplications";

		m_arSections[3].strDisplayName = "Consent";
		m_arSections[3].strFieldName = "Consent";
		m_arSections[3].bChanged = false;
		m_arSections[3].nCustomFieldID = -1;
		m_arSections[3].strReviewFieldName = "ReviewConsent";

		// (a.walling 2006-10-24 11:32) - PLID 22598 - Alternate language consent section
		m_arSections[4].strDisplayName = "Consent - Alt. Language";
		m_arSections[4].strFieldName = "AltConsent";
		m_arSections[4].bChanged = false;
		m_arSections[4].nCustomFieldID = -1;
		m_arSections[4].strReviewFieldName = "ReviewAltConsent";

		m_arSections[5].strDisplayName = "Hospital Stay";
		m_arSections[5].strFieldName = "HospitalStay";
		m_arSections[5].bChanged = false;
		m_arSections[5].nCustomFieldID = -1;
		m_arSections[5].strReviewFieldName = "ReviewHospitalStay";

		m_arSections[6].strDisplayName = "Mini-Description";
		m_arSections[6].strFieldName = "MiniDescription";
		m_arSections[6].bChanged = false;
		m_arSections[6].nCustomFieldID = -1;
		m_arSections[6].strReviewFieldName = "ReviewMini";

		m_arSections[7].strDisplayName = "PostOp";
		m_arSections[7].strFieldName = "PostOp";
		m_arSections[7].bChanged = false;
		m_arSections[7].nCustomFieldID = -1;
		m_arSections[7].strReviewFieldName = "ReviewPostOp";

		m_arSections[8].strDisplayName = "PreOp";
		m_arSections[8].strFieldName = "PreOp";
		m_arSections[8].bChanged = false;
		m_arSections[8].nCustomFieldID = -7;
		m_arSections[8].strReviewFieldName = "ReviewPreOp";

		m_arSections[9].strDisplayName = "Procedure Details";
		m_arSections[9].strFieldName = "ProcDetails"; // (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
		m_arSections[9].bChanged = false;
		m_arSections[9].nCustomFieldID = -1;
		m_arSections[9].strReviewFieldName = "ReviewProcDetails";

		m_arSections[10].strDisplayName = "Recovery";
		m_arSections[10].strFieldName = "Recovery";
		m_arSections[10].bChanged = false;
		m_arSections[10].nCustomFieldID = -1;
		m_arSections[10].strReviewFieldName = "ReviewRecovery";

		m_arSections[11].strDisplayName = "Risks";
		m_arSections[11].strFieldName = "Risks";
		m_arSections[11].bChanged = false;
		m_arSections[11].nCustomFieldID = -1;
		m_arSections[11].strReviewFieldName = "ReviewRisks";

		m_arSections[12].strDisplayName = "Showering";
		m_arSections[12].strFieldName = "Showering";
		m_arSections[12].bChanged = false;
		m_arSections[12].nCustomFieldID = -1;
		m_arSections[12].strReviewFieldName = "ReviewShowering";

		m_arSections[13].strDisplayName = "Special Diet";
		m_arSections[13].strFieldName = "SpecialDiet";
		m_arSections[13].bChanged = false;
		m_arSections[13].nCustomFieldID = -1;
		m_arSections[13].strReviewFieldName = "ReviewDiet";

		m_arSections[14].strDisplayName = "The Day Of";
		m_arSections[14].strFieldName = "TheDayOf";
		m_arSections[14].bChanged = false;
		m_arSections[14].nCustomFieldID = -1;
		m_arSections[14].strReviewFieldName = "ReviewDayOf";

		//Custom
		long nFirstCustomRow = 15;
		m_arSections[15].strFieldName = "CustomSection1";
		m_arSections[15].bChanged = false;
		m_arSections[15].strReviewFieldName = "ReviewCustom1";
		m_arSections[16].strFieldName = "CustomSection2";
		m_arSections[16].bChanged = false;
		m_arSections[16].strReviewFieldName = "ReviewCustom2";
		m_arSections[17].strFieldName = "CustomSection3";
		m_arSections[17].bChanged = false;
		m_arSections[17].strReviewFieldName = "ReviewCustom3";
		m_arSections[18].strFieldName = "CustomSection4";
		m_arSections[18].bChanged = false;
		m_arSections[18].strReviewFieldName = "ReviewCustom4";
		m_arSections[19].strFieldName = "CustomSection5";
		m_arSections[19].bChanged = false;
		m_arSections[19].strReviewFieldName = "ReviewCustom5";
		m_arSections[20].strFieldName = "CustomSection6";
		m_arSections[20].bChanged = false;
		m_arSections[20].strReviewFieldName = "ReviewCustom6";
		m_arSections[21].strFieldName = "CustomSection7";
		m_arSections[21].bChanged = false;
		m_arSections[21].strReviewFieldName = "ReviewCustom7";
		m_arSections[22].strFieldName = "CustomSection8";
		m_arSections[22].bChanged = false;
		m_arSections[22].strReviewFieldName = "ReviewCustom8";
		m_arSections[23].strFieldName = "CustomSection9";
		m_arSections[23].bChanged = false;
		m_arSections[23].strReviewFieldName = "ReviewCustom9";
		m_arSections[24].strFieldName = "CustomSection10";
		m_arSections[24].bChanged = false;
		m_arSections[24].strReviewFieldName = "ReviewCustom10";

		//Finally, the custom field names.
		_RecordsetPtr rsCustomFields = CreateRecordset("SELECT ID, Name FROM CustomFieldsT WHERE ID >= 70 AND ID <= 79 ORDER BY ID");
		FieldsPtr Fields = rsCustomFields->Fields;
		long i = nFirstCustomRow;
		while(!rsCustomFields->eof) {
			m_arSections[i].nCustomFieldID = AdoFldLong(Fields, "ID");
			m_arSections[i].strDisplayName = AdoFldString(Fields, "Name");
			i++;
			rsCustomFields->MoveNext();
		}

	//All done!
	}NxCatchAll("Error 100 in InitSectionArray()");
}

void CProcedureSectionEditDlg::OnCopySections() 
{
	try {
		CSelectSourceProcedureDlg dlg(this);
		if(IDOK == dlg.DoModal()) {
			// (d.moore 2007-06-19 17:55) - PLID 23861 - Modified m_dlSectionCombo to be a NxDataList2
			//  Also added filters for the list.
			m_dlSectionCombo->Clear();
			BOOL bShowOnlyReviewed = IsDlgButtonChecked(IDC_CHECK_FILTER_NEED_REVIEWED);
			BOOL bShowOnlySectionsWithContent = IsDlgButtonChecked(IDC_CHECK_FILTER_SECTIONS_WITH_CONTENT);
			BOOL bDontShow, bMarkedReviewed, bHasContent;

			_RecordsetPtr rsSourceProc = CreateRecordset("SELECT * FROM ProcedureT WHERE ID = %li", dlg.m_nSelectedId);
			FieldsPtr fSourceProc = rsSourceProc->Fields;
			if(!rsSourceProc->eof) {
				for(int i = 0; i < SECTION_COUNT; i++) {
					m_arSections[i].strText = AdoFldString(fSourceProc, m_arSections[i].strFieldName);
					m_arSections[i].bNeedsReview = AdoFldBool(fSourceProc, m_arSections[i].strReviewFieldName, 0);
					m_arSections[i].bChanged = true;
					
					// Remove any formatting text from the CRichTextCtrl.
					CString sContent = m_arSections[i].strText;
					sContent = ConvertTextFormat(sContent, tfRichText, tfPlainText);
					sContent.TrimLeft();
					
					// Now check to see if the section for content and review status.
					bMarkedReviewed = m_arSections[i].bNeedsReview;
					bHasContent = !sContent.IsEmpty();

					bDontShow = ((bShowOnlyReviewed && !bMarkedReviewed) 
						|| (bShowOnlySectionsWithContent && !bHasContent));
					
					if (!bDontShow) {
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->GetNewRow();
						pRow->PutValue(psesccName, _bstr_t(m_arSections[i].strDisplayName));
						pRow->PutValue(psesccNeedsReviewed, (VARIANT_BOOL)m_arSections[i].bNeedsReview);
						pRow->PutValue(psesccArrayIndex, (long)i);
						m_dlSectionCombo->AddRowAtEnd(pRow, NULL);
						ColorSectionRow(pRow);
					}
				}
			}
			//Refresh the screen.
			// (d.moore 2007-06-19 17:55) - PLID 23861 - Modified m_dlSectionCombo to be a NxDataList2
			m_nCurrentSection = SetSectionSelection(m_nCurrentSection);
			if (m_nCurrentSection < 0) {
				m_nCurrentSection = SetSectionSelectionToFirst();
			}
			LoadDataForSection(m_nCurrentSection);
			EnsureNextBackButtonState();
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnCopySections");
}

void CProcedureSectionEditDlg::OnSelchangeSectionEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);

		SyncControlsWithText();

		*pResult = 0;
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnSelchangeSectionEdit");
}

void CProcedureSectionEditDlg::SyncControlsWithText()
{
	try {
		//Get the CharFormat.
		CHARFORMAT2 cf;
		DWORD dwMask = m_richeditSectionText.GetSelectionCharFormat(cf);
		//First, check the bold state.
		if(dwMask & CFM_BOLD) {
			//All selected characters are either bold or not.
			if(cf.dwEffects & CFE_BOLD) {
				m_btnBold.SetCheck(1);
			}
			else {
				m_btnBold.SetCheck(0);
			}
		}
		else {
			//Some selected are bold, some are not.
			m_btnBold.SetCheck(0);
		}
		//Next, the italic state.
		if(dwMask & CFM_ITALIC) {
			//All selected characters are either italic or not.
			if(cf.dwEffects & CFE_ITALIC) {
				m_btnItalic.SetCheck(1);
			}
			else {
				m_btnItalic.SetCheck(0);
			}
		}
		else {
			//Some selected are italic, some are not.
			m_btnItalic.SetCheck(0);
		}
		//Next, the underline state.
		if(dwMask & CFM_UNDERLINE) {
			//All selected characters are either underlined or not.
			if(cf.dwEffects & CFE_UNDERLINE) {
				m_btnUnderline.SetCheck(1);
			}
			else {
				m_btnUnderline.SetCheck(0);
			}
		}
		else {
			//Some selected are underlined, some are not.
			m_btnUnderline.SetCheck(0);
		}
		//Next, the font name.
		if(dwMask & CFM_FACE) {
			//All selected characters are the same font.
			m_pFontList->SetSelByColumn(0, _bstr_t(cf.szFaceName));
		}
		else {
			//Selected characters have different fonts.
			m_pFontList->CurSel = -1;
		}
		//Next, the font height.
		if(dwMask & CFM_SIZE) {
			//All selected characters are the same height.
			double dHeight = (double)cf.yHeight / 20.0;
			CString strHeight;
			strHeight.Format("%f", dHeight);
			//There is surely a way to do this in the Format(), but I couldn't figure it out easily.
			strHeight.TrimRight("0");
			strHeight.TrimRight(".");
			SetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strHeight);
		}
		else {
			//Varying heights.
			SetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, "");
		}
		//Next, the color
		if(dwMask & CFM_COLOR) {
			//All selected characters are the same color.
			if(cf.dwEffects & CFE_AUTOCOLOR) {
				m_pColorList->CurSel = 0;
			}
			else if(cf.crTextColor == RGB(255,0,0)) m_pColorList->CurSel = 1;
			else if(cf.crTextColor == RGB(0,255,0)) m_pColorList->CurSel = 2;
			else if(cf.crTextColor == RGB(0,0,255)) m_pColorList->CurSel = 3;
			else if(cf.crTextColor == RGB(255,255,0)) m_pColorList->CurSel = 4;
			else if(cf.crTextColor == RGB(0,255,255)) m_pColorList->CurSel = 5;
			else if(cf.crTextColor == RGB(255,0,255)) m_pColorList->CurSel = 6;
			else if(cf.crTextColor == RGB(0,0,0)) m_pColorList->CurSel = 7;
			else if(cf.crTextColor == RGB(127,127,127)) m_pColorList->CurSel = 8;
			else {
				//Custom.
				m_pColorList->CurSel = 9;
				((IRowSettingsPtr)m_pColorList->GetRow(9))->PutForeColor(cf.crTextColor);
				((IRowSettingsPtr)m_pColorList->GetRow(9))->PutForeColorSel(cf.crTextColor);
			}
		}
		else {
			//Varying colors.
			m_pColorList->CurSel = -1;
		}

		//Next, the subscript/superscript values.
		if(dwMask & (CFM_OFFSET) ){
			//All selected characters are the same offset.
			m_btnSubScript.SetCheck(cf.yOffset < 0);
			m_btnSuperScript.SetCheck(cf.yOffset > 0);
		}
		else {
			//Different offsets.
			m_btnSubScript.SetCheck(0);
			m_btnSuperScript.SetCheck(0);
		}
		//Now, only allow them to change these if all selected characters are the same height.
		m_btnSubScript.EnableWindow((dwMask & CFM_SIZE) && (dwMask & CFM_OFFSET));
		m_btnSuperScript.EnableWindow((dwMask & CFM_SIZE) && (dwMask & CFM_OFFSET));

		//Now, the paragraph format stuff.
		PARAFORMAT2 pf;
		m_richeditSectionText.GetParaFormat(pf);
		if(pf.dwMask & PFM_ALIGNMENT) {
			//The current selection is one alignment.
			if(pf.wAlignment == PFA_LEFT) {
				m_btnAlignLeft.SetCheck(1);
				m_btnAlignCenter.SetCheck(0);
				m_btnAlignRight.SetCheck(0);
			}
			else if(pf.wAlignment == PFA_CENTER) {
				m_btnAlignLeft.SetCheck(0);
				m_btnAlignCenter.SetCheck(1);
				m_btnAlignRight.SetCheck(0);
			}
			else if(pf.wAlignment == PFA_RIGHT) {
				m_btnAlignLeft.SetCheck(0);
				m_btnAlignCenter.SetCheck(0);
				m_btnAlignRight.SetCheck(1);
			}
		}
		else {
			//Multiple alignments selected.
			m_btnAlignLeft.SetCheck(0);
			m_btnAlignCenter.SetCheck(0);
			m_btnAlignRight.SetCheck(0);
		}
		//Next, the paragraph bulleting.
		if(pf.dwMask & PFM_NUMBERING) {
			//The current section is bulleted or not.
			if(pf.wNumbering == PFN_BULLET) {
				m_btnBullets.SetCheck(1);
			}
			else {
				m_btnBullets.SetCheck(0);
			}
		}
		else {
			//Some bulleted, some not.
			m_btnBullets.SetCheck(0);
		}
		//Finally, check the undo buffer.
		BOOL bCanUndo = m_richeditSectionText.CanUndo();
		m_btnUndo.EnableWindow(bCanUndo);

	}NxCatchAll("Error in CProcedureSectionEditDlg::SyncControlsWithText()");
}

void CProcedureSectionEditDlg::OnProcSectionBold() 
{
	try {
		CHARFORMAT2 cf;
		cf.dwMask = CFM_BOLD;
		cf.dwEffects = m_btnBold.GetCheck() ? CFE_BOLD : 0;
		m_richeditSectionText.SetSelectionCharFormat(cf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionBold");
}

void CProcedureSectionEditDlg::OnProcSectionItalic() 
{
	try {
		CHARFORMAT2 cf;
		cf.dwMask = CFM_ITALIC;
		cf.dwEffects = m_btnItalic.GetCheck() ? CFE_ITALIC : 0;
		m_richeditSectionText.SetSelectionCharFormat(cf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionItalic");
}

void CProcedureSectionEditDlg::OnProcSectionUnderline() 
{
	try {
		CHARFORMAT2 cf;
		cf.dwMask = CFM_UNDERLINE|CFM_UNDERLINETYPE;
		cf.dwEffects = m_btnUnderline.GetCheck() ? CFE_UNDERLINE : 0;
		cf.bUnderlineType = m_btnUnderline.GetCheck() ? CFU_UNDERLINE : CFU_UNDERLINENONE;
		m_richeditSectionText.SetSelectionCharFormat(cf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionUnderline");
}

void CProcedureSectionEditDlg::OnSelChosenProcSectionFontList(long nRow) 
{
	try {
		if(nRow == -1) {
			//That's no good, set it to reflect whatever is correct (may be -1 anyway).
			SyncControlsWithText();
		}
		else {
			CHARFORMAT2 cf;
			cf.dwMask = CFM_FACE|CFM_CHARSET;
			strcpy(cf.szFaceName, VarString(m_pFontList->GetValue(nRow, 0)));
			cf.bPitchAndFamily = (BYTE)VarLong(m_pFontList->GetValue(nRow, 1));
			cf.bCharSet = (BYTE)VarLong(m_pFontList->GetValue(nRow, 2));
			m_richeditSectionText.SetSelectionCharFormat(cf);
		}
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnSelChosenProcSectionFontList");
}

static bool g_bChangingFontHeight = false;//Infinite loop protection.
void CProcedureSectionEditDlg::OnChangeProcSectionFontHeight() 
{
	try {
		if(!g_bChangingFontHeight) {
			g_bChangingFontHeight = true;
			//We only allow numbers, commas (for international people) and decimal points here.
			CString strText;
			GetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strText);
			CString strNumber;
			for(int i = 0; i < strText.GetLength(); i++) {
				if(strText.Mid(i,1).SpanIncluding("1234567890,.").GetLength()) {
					strNumber += strText.GetAt(i);
				}
			}
			if(strNumber != strText) {
				int nStart, nEnd;
				((CNxEdit*)GetDlgItem(IDC_PROC_SECTION_FONT_HEIGHT))->GetSel(nStart, nEnd);
				SetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strNumber);
				((CNxEdit*)GetDlgItem(IDC_PROC_SECTION_FONT_HEIGHT))->SetSel(nStart, nEnd);
			}
			g_bChangingFontHeight = false;
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnChangeProcSectionFontHeight");
}

void CProcedureSectionEditDlg::OnKillfocusProcSectionFontHeight() 
{
	try {
		CString strHeight;
		GetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strHeight);
		if(!strHeight.IsEmpty()) {
			double dHeight = atof(strHeight);

			//DRT 5/11/2004 - PLID 12182 - Max height of 72.
			if(dHeight > 72.0) {
				MsgBox("You cannot set a font height greater than 72.  Your entry will be reset.");
				dHeight = 72.0;
				SetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, "72");
			}

			CHARFORMAT2 cf;
			cf.dwMask = CFM_SIZE;
			cf.yHeight = (long)dHeight * 20;
			m_richeditSectionText.SetSelectionCharFormat(cf);
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnKillfocusProcSectionFontHeight");
}

void CProcedureSectionEditDlg::OnDeltaposFontHeightSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

		int nDifference = pNMUpDown->iDelta;
		CString strNumber;
		GetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strNumber);
		double fNum = atof(strNumber);
		//First, if we are not an integer, go to the nearest integer.
		int nNewNum;
		if(fNum != (double)((int)fNum)) {
			nNewNum = nDifference > 0 ? (int)fNum : (int)fNum + 1;
		}
		else {
			//Just add.
			nNewNum = (int)fNum - nDifference;
		}
		if(nNewNum <= 0) return;
		CString strNewNumber;
		strNewNumber.Format("%i", nNewNum);
		SetDlgItemText(IDC_PROC_SECTION_FONT_HEIGHT, strNewNumber);

		//Now apply that to the selected text.
		OnKillfocusProcSectionFontHeight();
		
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
		*pResult = 0;
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnDeltaposFontHeightSpin");
}

void CProcedureSectionEditDlg::OnSelChosenProcSectionColor(long nRow) 
{
	try {
		if(nRow == -1) {
			//That's no good!
			SyncControlsWithText();
			return;
		}
		else if(nRow == 0) {//Automatic.
			CHARFORMAT2 cf;
			cf.dwMask = CFM_COLOR;
			cf.dwEffects = CFE_AUTOCOLOR;
			m_richeditSectionText.SetSelectionCharFormat(cf);
		}
		else if(nRow == m_pColorList->GetRowCount()-1) {//Custom
			CColorDialog dlg(((IRowSettingsPtr)m_pColorList->GetRow(nRow))->GetForeColor(), CC_FULLOPEN);
			if(dlg.DoModal() == IDOK) {
				COLORREF cNew = dlg.GetColor();
				((IRowSettingsPtr)m_pColorList->GetRow(nRow))->PutForeColor(cNew);
				((IRowSettingsPtr)m_pColorList->GetRow(nRow))->PutForeColorSel(cNew);
				CHARFORMAT2 cf;
				cf.dwMask = CFM_COLOR;
				cf.dwEffects = 0;
				cf.crTextColor = cNew;
				m_richeditSectionText.SetSelectionCharFormat(cf);
			}
			else {
				SyncControlsWithText();
				return;
			}
		}
		else {
			//One of the colors
			CHARFORMAT2 cf;
			cf.dwMask = CFM_COLOR;
			cf.dwEffects = 0;
			cf.crTextColor = ((IRowSettingsPtr)m_pColorList->GetRow(nRow))->GetForeColor();
			m_richeditSectionText.SetSelectionCharFormat(cf);
		}
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnSelChosenProcSectionColor");
}

BOOL CProcedureSectionEditDlg::PreTranslateMessage(MSG* pMsg) 
{
	try {
		if(pMsg->message == WM_KEYDOWN) {
			//All the characters we care about, we only care about them if ctrl is pressed.
			if(GetAsyncKeyState(VK_CONTROL) & 0xE000) {
				switch(pMsg->wParam) {
				case 'B':
					m_btnBold.SetCheck(!m_btnBold.GetCheck());
					OnProcSectionBold();
					return TRUE;
					break;
				case 'I':
					m_btnItalic.SetCheck(!m_btnItalic.GetCheck());
					OnProcSectionItalic();
					return TRUE;
					break;
				case 'U':
					m_btnUnderline.SetCheck(!m_btnUnderline.GetCheck());
					OnProcSectionUnderline();
					return TRUE;
					break;
				case 'Z':
					OnUndo();
					return TRUE;
					break;
				}
			}
		}
		else if(pMsg->message == WM_RBUTTONDOWN && pMsg->hwnd == m_richeditSectionText.GetSafeHwnd()) {
			//The rclick handler isn't working, so we'll do it manually here.
			//First, move the cursor to where they right-clicked.
			m_richeditSectionText.SendMessage(WM_LBUTTONDOWN, pMsg->wParam, pMsg->lParam);
			OnRclickSectionEdit(NULL, NULL);
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::PreTranslateMessage");
	return CDialog::PreTranslateMessage(pMsg);
}

void CProcedureSectionEditDlg::OnUndo()
{
	try {
		if(m_richeditSectionText.CanUndo()) {
			m_richeditSectionText.Undo();
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnUndo");
}

void CProcedureSectionEditDlg::OnProcSectionAlignCenter() 
{
	try {
		m_btnAlignLeft.SetCheck(0);
		m_btnAlignCenter.SetCheck(1);
		m_btnAlignRight.SetCheck(0);
		PARAFORMAT2 pf;
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_CENTER;
		m_richeditSectionText.SetParaFormat(pf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionAlignCenter");
}

void CProcedureSectionEditDlg::OnProcSectionAlignLeft() 
{
	try {
		m_btnAlignLeft.SetCheck(1);
		m_btnAlignCenter.SetCheck(0);
		m_btnAlignRight.SetCheck(0);
		PARAFORMAT2 pf;
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_LEFT;
		m_richeditSectionText.SetParaFormat(pf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionAlignLeft");
}

void CProcedureSectionEditDlg::OnProcSectionAlignRight() 
{
	try {
		m_btnAlignLeft.SetCheck(0);
		m_btnAlignCenter.SetCheck(0);
		m_btnAlignRight.SetCheck(1);
		PARAFORMAT2 pf;
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_RIGHT;
		m_richeditSectionText.SetParaFormat(pf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionAlignRight");
}

void CProcedureSectionEditDlg::OnProcSectionBullets() 
{
	try {
		PARAFORMAT2 pf;
		pf.dwMask = PFM_NUMBERING;
		pf.wNumbering = m_btnBullets.GetCheck() ? PFN_BULLET : 0;
		m_richeditSectionText.SetParaFormat(pf);
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionBullets");
}

void CProcedureSectionEditDlg::OnRclickSectionEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		long nStart, nEnd;
		m_richeditSectionText.GetSel(nStart, nEnd);
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		if(nEnd-nStart == 0) {	
			mnu.InsertMenu(0, MF_BYPOSITION|MF_GRAYED, ID_CUT, "Cut");
			mnu.InsertMenu(1, MF_BYPOSITION|MF_GRAYED, ID_COPY, "Copy");
		}
		else {
			mnu.InsertMenu(0, MF_BYPOSITION, ID_CUT, "Cut");
			mnu.InsertMenu(1, MF_BYPOSITION, ID_COPY, "Copy");
		}
		if(m_richeditSectionText.CanPaste()) {
			mnu.InsertMenu(2, MF_BYPOSITION, ID_PASTE, "Paste");
		}
		else {
			mnu.InsertMenu(2, MF_BYPOSITION|MF_GRAYED, ID_PASTE, "Paste");
		}
		
		CPoint pos;		
		GetCursorPos(&pos);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this, NULL);
		
		if(pResult)
			*pResult = 0;
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnRclickSectionEdit");
}

void CProcedureSectionEditDlg::OnCut()
{
	try {
		m_richeditSectionText.Cut();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnCut");
}

void CProcedureSectionEditDlg::OnCopy()
{
	try {
		m_richeditSectionText.Copy();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnCopy");
}

void CProcedureSectionEditDlg::OnPaste()
{
	try {
		if(m_richeditSectionText.CanPaste()) {
			m_richeditSectionText.Paste();
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnPaste");
}

void CProcedureSectionEditDlg::OnProcSectionSubscript() 
{
	try {
		//We will set the font height to half of its current height, and the offset to 2/3 the current height.
		CHARFORMAT2 cfCurrent;
		m_richeditSectionText.GetSelectionCharFormat(cfCurrent);
		DWORD dwMask = cfCurrent.dwMask;
		if((dwMask & CFM_SIZE) && (dwMask & CFM_OFFSET)) {
			if(m_btnSubScript.GetCheck()) {
				int nCurrentHeight = cfCurrent.yHeight;
				int nCurrentOffset = cfCurrent.yOffset;
				if(nCurrentOffset != 0) {
					//This should only be possible if it was superscripted.  So, reverse the superscriptness.
					nCurrentHeight = nCurrentHeight/2*3;
					nCurrentOffset = 0;
				}
				CHARFORMAT2 cfNew;
				cfNew.dwMask = CFM_SIZE|CFM_OFFSET;
				cfNew.yHeight = nCurrentHeight * 2/3;
				cfNew.yOffset = nCurrentHeight * -2/3;
				m_richeditSectionText.SetSelectionCharFormat(cfNew);
				m_btnSuperScript.SetCheck(0);
			}
			else {
				//Reverse.
				int nCurrentHeight = cfCurrent.yHeight;
				CHARFORMAT2 cfNew;
				cfNew.dwMask = CFM_SIZE|CFM_OFFSET;
				cfNew.yHeight = nCurrentHeight/2*3;
				cfNew.yOffset = 0;
				m_richeditSectionText.SetSelectionCharFormat(cfNew);
			}
		}
		else {
			//This shouldn't be possible, do nothing.
			SyncControlsWithText();
		}
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionSubscript");
}

void CProcedureSectionEditDlg::OnProcSectionSuperscript() 
{
	try {
		//We will set the font height to half of its current height, and the offset to 2/3 the current height.
		CHARFORMAT2 cfCurrent;
		m_richeditSectionText.GetSelectionCharFormat(cfCurrent);
		DWORD dwMask = cfCurrent.dwMask;
		if((dwMask & CFM_SIZE) && (dwMask & CFM_OFFSET)) {
			if(m_btnSuperScript.GetCheck()) {
				int nCurrentHeight = cfCurrent.yHeight;
				int nCurrentOffset = cfCurrent.yOffset;
				if(nCurrentOffset != 0) {
					//This should only be possible if it was subscripted.  So, reverse the subscriptness.
					nCurrentHeight = nCurrentHeight/2*3;
					nCurrentOffset = 0;
				}
				CHARFORMAT2 cfNew;
				cfNew.dwMask = CFM_SIZE|CFM_OFFSET;
				cfNew.yHeight = nCurrentHeight * 2/3;
				cfNew.yOffset = nCurrentHeight * 2/3;
				m_richeditSectionText.SetSelectionCharFormat(cfNew);
				m_btnSubScript.SetCheck(0);
			}
			else {
				//Reverse.
				int nCurrentHeight = cfCurrent.yHeight;
				CHARFORMAT2 cfNew;
				cfNew.dwMask = CFM_SIZE|CFM_OFFSET;
				cfNew.yHeight = nCurrentHeight/2*3;
				cfNew.yOffset = 0;
				m_richeditSectionText.SetSelectionCharFormat(cfNew);
			}
		}
		else {
			//This shouldn't be possible, do nothing.
			SyncControlsWithText();
		}
		//Let them keep typing.
		m_richeditSectionText.SetFocus();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnProcSectionSuperscript");
}

//DRT 7/7/2004 - PLID 13349 - Review button
void CProcedureSectionEditDlg::OnNeedsReviewed() 
{
	// (d.moore 2007-06-25 09:21) - PLID 23861 - Make sure the current selection is not out of bounds.
	try {
		if(m_nCurrentSection < 0 || m_nCurrentSection >= SECTION_COUNT) {
			return;
		}

		//update the datalist and the array with this new value
		BOOL bSet = IsDlgButtonChecked(IDC_NEEDS_REVIEWED);

		// (d.moore 2007-06-29 12:33) - PLID 23863 - Add or remove text that labels the
		//  section as 'Needs Reviewed'.
		if (bSet) {
			InsertNeedsReviewedText(&m_richeditSectionText);
			m_bNeedsReviewedWasSet = true;
			m_richeditSectionText.SetFocus();
		} 
		else {
			RemoveNeedsReviewedText(&m_richeditSectionText);
			m_bNeedsReviewedWasSet = false;
			m_richeditSectionText.SetFocus();
		}

		m_arSections[m_nCurrentSection].bNeedsReview = bSet;
		// (d.moore 2007-06-19 17:45) - PLID 23861 - Modified m_dlSectionCombo to be a NxDataList2
		m_dlSectionCombo->CurSel->PutValue(psesccNeedsReviewed, (VARIANT_BOOL)bSet);
		
		m_arSections[m_nCurrentSection].bChanged = true;
		ColorSectionRow(m_dlSectionCombo->CurSel);
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnNeedsReviewed");
}

void CProcedureSectionEditDlg::ColorSectionRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (d.moore 2007-06-20 09:52) - PLID 23861 - Modified the function to take a row pointer
	//  instead of an integer index value.
	try {
		if(pRow == NULL) {
			return;
		}

		// Get the index value for m_arSections from the row.
		long nIndex = VarLong(pRow->GetValue(psesccArrayIndex));
		
		//if it needs reviewed, color it red, otherwise black
		BOOL bSet = m_arSections[nIndex].bNeedsReview;
			
		if(bSet) {
			pRow->PutForeColor(RGB(255, 0, 0));
			pRow->PutForeColorSel(RGB(255, 0, 0));
		}
		else {
			pRow->PutForeColor(RGB(0, 0, 0));
			pRow->PutForeColorSel(RGB(255, 255, 255));
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::ColorSectionRow");
}

void CProcedureSectionEditDlg::ColorSectionMarkedRows()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->GetFirstRow();
	while (pRow != NULL) {
		ColorSectionRow(pRow);
		pRow = pRow->GetNextRow();
	}
}

void CProcedureSectionEditDlg::OnPreviewItem() 
{
	//we have to make a menu to know whether they want just this field the whole procedure
	try {
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		CString strProcName;
		CString strField;

		_RecordsetPtr rsProcName = CreateRecordset("SELECT Name From ProcedureT WHERE ID = %li", m_nProcedureID);
		if (! rsProcName->eof) {
			strProcName = AdoFldString(rsProcName, "Name");
		}
		strField = VarString(m_dlSectionCombo->CurSel->GetValue(psesccName));
		CString strMsg1, strMsg2;
		strMsg1.Format("P&review only %s for %s", strField, strProcName);
		strMsg2.Format("Pr&eview all fields for %s", strProcName);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_PREVIEW_FIELD, strMsg1);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_PREVIEW_PROC, strMsg2);
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_PREVIEW_ITEM);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_RIGHTALIGN, rc.right, rc.bottom, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_RIGHTALIGN, pt.x, pt.y, this, NULL);
		}	
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnPreviewItem");
}

void CProcedureSectionEditDlg::OnPreviewField() {

	try {
		PreviewNexFormsReport(TRUE);
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnPreviewField");
}

void CProcedureSectionEditDlg::OnPreviewProc() {

	try {
		PreviewNexFormsReport(FALSE);
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnPreviewProc");
}

void CProcedureSectionEditDlg::PreviewNexFormsReport(BOOL bShowCurrentFieldOnly) {

	// (d.moore 2007-06-21 17:35) - PLID 23861 - The dialog used to save and then close at this point.
	//  However, much of the reason for this punchlist item was the goal of keeping the dialog open 
	//  and makeing editing a rapid process. So closing the dialog to see a preview doesn't make much sense.
	
	// Check to see if the user wants to save any recently changed data.
	long nChangedCount = 0;
	for(int i = 0; i < SECTION_COUNT; i++) {
		if(m_arSections[i].bChanged) {
			nChangedCount++;	
		}
	}
	if (nChangedCount > 0) {
		CString strMsg = 
			"Would you like to save your changes before previewing? \n"
			"If you select no then you will preview the content \n"
			"without the most recent changes.";
		if (AfxMessageBox(strMsg, MB_YESNO) == IDYES) {
			Save();
		}
	}

	// Now minimize the window so that the user can see the preview screen.
	ShowWindow(SW_MINIMIZE);


	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(570)];

	//steal the extra values field for the procedure number
	infReport.AddExtraValue(AsString(m_nProcedureID)); 
	
	//Create a parameter to use with suppression formulas for the one field only part
	CRParameterInfo *paramInfo;
	CPtrArray paParams;
	
	if (bShowCurrentFieldOnly) {
		CString strField = VarString(m_dlSectionCombo->CurSel->GetValue(psesccName));
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "TRUE";
		paramInfo->m_Name = "ShowOneFieldOnly";
		paParams.Add(paramInfo);
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strField;
		paramInfo->m_Name = "FieldToShow";
		paParams.Add(paramInfo);
	}
	else {
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "FALSE";
		paramInfo->m_Name = "ShowOneFieldOnly";
		paParams.Add(paramInfo);
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "<All Fields>";
		paramInfo->m_Name = "FieldToShow";
		paParams.Add(paramInfo);
	}

	// (z.manning, 09/20/2007) - PLID 27467 - There's a new parameter for this report to determine whether
	// or not we should show the "cheat sheet" procedure fields. When running the report from the NexForms
	// editor, we do not want to show those fields.
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "FALSE";
	paramInfo->m_Name = "ShowCheatSheetFields";
	paParams.Add(paramInfo);

	//now just run the report
	RunReport(&infReport, &paParams, true, (CWnd *)this, "NexForms Content");
	ClearRPIParameterList(&paParams);	
}

void CProcedureSectionEditDlg::OnButtonBack() 
{
	// (d.moore 2007-06-19 13:14) - PLID 23861 - Move to the previous entry in the Section
	//  list. If the begining of the list has been reached, then move to the previous entry
	//  in the procedure list. Navigating this way automatically saves data.
	try {
		PromptRemoveNeedsReviewed(m_nCurrentSection);
		StoreSectionText();
		
		NXDATALIST2Lib::IRowSettingsPtr pSecRow = m_dlSectionCombo->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pSecFirst = m_dlSectionCombo->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pProcRow = m_pProcedureList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pProcFirst = m_pProcedureList->GetFirstRow();
		
		// It is possible that some of the row pointers above may be NULL.
		//  If so, then try to switch to an appropriate section.

		if (pProcRow == NULL && pProcFirst == NULL) {
			// Can't do anything at all from this point. There are no procedures in the
			//  list, so there isn't anywhere to navigate to.
			EnableControls(false);
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return;
		}
		else if (pProcRow == NULL) {
			// There is content in the procedure list, so move to the first entry
			//  and select the first section for it.
			pProcRow = pProcFirst;
			m_pProcedureList->PutCurSel(pProcRow);
			m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));
			// Load the new set of sections for the procedure.
			LoadSectionList();
			m_nCurrentSection = SetSectionSelectionToFirst();
			LoadDataForSection(m_nCurrentSection);
			// Need to check button state.
			EnsureNextBackButtonState();
			return;
		}

		// It is possible that there may be no sections for the procedure.
		// Attempt to move back in the procedure list.
		if (pSecRow == NULL) {
			// Technically, there may be a valid pSecFirst value that we could try to use,
			//  but for symplicity just treat it like there is no content.
			if (pProcRow->IsSameRow(pProcFirst)) {
				// Can't do anything from here either. But it may be possible to move forward.
				EnableControls(false);
				GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
				return;
			} else {
				// Move back in procedure list.
				pProcRow = pProcRow->GetPreviousRow();
				m_pProcedureList->PutCurSel(pProcRow);
				m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));
				// Load the new set of sections for the procedure.
				LoadSectionList();
				m_nCurrentSection = SetSectionSelectionToLast();
				LoadDataForSection(m_nCurrentSection);
				// Need to check button state.
				EnsureNextBackButtonState();
				return;
			}
		}

		// If we've reached this point, then we should have valid procedure and section selections.
		
		// Check to see if we are moving through the list of Sections.
		if (!pSecRow->IsSameRow(pSecFirst)) {
			// The selection is not at the begining of the section list, so it can just
			//  be moved back in the list.
			pSecRow = pSecRow->GetPreviousRow();
			m_dlSectionCombo->PutCurSel(pSecRow);
			m_nCurrentSection = VarLong(pSecRow->GetValue(psesccArrayIndex ));
			LoadDataForSection(m_nCurrentSection);
			// Check to see if the Back button should be disabled.
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
			if (pSecRow->IsSameRow(pSecFirst)) {				
				if (pProcRow->IsSameRow(pProcFirst)) {
					GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
				}
			}
			return;
		}
		
		if (pProcRow->IsSameRow(pProcFirst)) {
			// The selection is at the begining of the procedure list, and at the begining 
			//  of the sections list. So nothing can really be done. Disable the Back button
			//  so that this can't happen again.
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			return;
		}

		// The procedure selection can simply be moved back in the list.
		//  Need to save the data for the current procedure first though.
		CWaitCursor cuWait;
		if (PromptForSaveOnProcedureChange()) {
			ValidateAndSave();
		}
		
		pProcRow = pProcRow->GetPreviousRow();
		m_pProcedureList->PutCurSel(pProcRow);
		m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));

		// Load the new set of sections for the procedure.
		LoadSectionList();
		m_nCurrentSection = SetSectionSelectionToLast();
		LoadDataForSection(m_nCurrentSection);
		// Need to check button state.
		EnsureNextBackButtonState();

	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnButtonBack");
}

void CProcedureSectionEditDlg::OnButtonNext() 
{
	// (d.moore 2007-06-19 13:14) - PLID 23861 - Move to the next entry in the Section
	//  list. If the end of the list has been reached, then move to the next Procedure.
	//  Navigating this way automatically saves data.
	try {
		PromptRemoveNeedsReviewed(m_nCurrentSection);
		StoreSectionText();
		
		// First try to see if we are moving through the list of Sections.
		NXDATALIST2Lib::IRowSettingsPtr pSecRow = m_dlSectionCombo->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pSecLast = m_dlSectionCombo->GetLastRow();
		NXDATALIST2Lib::IRowSettingsPtr pProcRow = m_pProcedureList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pProcLast = m_pProcedureList->GetLastRow();

		// It is possible that some of the row pointers above may be NULL.
		//  If so, then try to switch to an appropriate section.

		if (pProcRow == NULL && pProcLast == NULL) {
			// Can't do anything at all from this point. There are no procedures in the
			//  list, so there isn't anywhere to navigate to.
			EnableControls(false);
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return;
		}
		else if (pProcRow == NULL) {
			// There is content in the procedure list, so move to the last entry
			//  and select the last section for it.
			pProcRow = pProcLast;
			m_pProcedureList->PutCurSel(pProcRow);
			m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));
			// Load the new set of sections for the procedure.
			LoadSectionList();
			m_nCurrentSection = SetSectionSelectionToLast();
			LoadDataForSection(m_nCurrentSection);
			// Need to check button state.
			EnsureNextBackButtonState();
			return;
		}

		// It is possible that there may be no sections for the procedure.
		// Attempt to move forward in the procedure list.
		if (pSecRow == NULL) {
			// Technically, there may be a valid pSecFirst value that we could try to use,
			//  but for symplicity just treat it like there is no content.
			if (pProcRow->IsSameRow(pProcLast)) {
				// Can't do anything from here either. But it may be possible to move back.
				EnableControls(false);
				GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(TRUE);
				GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
				return;
			} else {
				// Move forward in procedure list.
				pProcRow = pProcRow->GetNextRow();
				m_pProcedureList->PutCurSel(pProcRow);
				m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));
				// Load the new set of sections for the procedure.
				LoadSectionList();
				m_nCurrentSection = SetSectionSelectionToFirst();
				LoadDataForSection(m_nCurrentSection);
				// Need to check button state.
				EnsureNextBackButtonState();
				return;
			}
		}

		// If we've reached this point, then we should have valid procedure and section selections.

		if (!pSecRow->IsSameRow(pSecLast)) {
			// The selection is not at the end of the section list, so it can just
			//  be moved ahead in the list.
			pSecRow = pSecRow->GetNextRow();
			m_dlSectionCombo->PutCurSel(pSecRow);
			m_nCurrentSection = VarLong(pSecRow->GetValue(psesccArrayIndex ));
			LoadDataForSection(m_nCurrentSection);
			// Check to see if the Next button should be disabled.
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(TRUE);
			if (pSecRow->IsSameRow(pSecLast)) {
				if (pProcRow->IsSameRow(pProcLast)) {
					GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
				}
			}
			return;
		}

		if (pProcRow->IsSameRow(pProcLast)) {
			// The selection is at the end of the procedure list, and at the end of the
			//  sections list. So nothing can really be done. Disable the Next button
			//  so that this can't happen again.
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return;
		}

		// The procedure selection can simply be moved ahead in the list.
		//  Need to save the data for the current procedure first though.
		CWaitCursor cuWait;
		if (PromptForSaveOnProcedureChange()) {
			ValidateAndSave();
		}
		pProcRow = pProcRow->GetNextRow();
		m_pProcedureList->PutCurSel(pProcRow);
		m_nProcedureID = VarLong(pProcRow->GetValue(pseProcColID));
		
		// Load the new set of sections for the procedure.
		LoadSectionList();
		m_nCurrentSection = SetSectionSelectionToFirst();
		LoadDataForSection(m_nCurrentSection);
		// Need to check button state.
		EnsureNextBackButtonState();

	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnButtonNext");
}

void CProcedureSectionEditDlg::OnSelChosenProcedureContentsList(LPDISPATCH lpRow) 
{
	// (d.moore 2007-06-19 13:14) - PLID 23861 - Need to update the list of sections
	//  to show the items that need to be reviewed for the selected procedure.
	try {
		// First, we need to try to save whatever was being worked on.
		StoreSectionText();
		if (PromptForSaveOnProcedureChange()) {
			ValidateAndSave();
		}
		
		// If there is no row selected, then deactivate most of the controls.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			// Nothing selected.
			m_nProcedureID = -1;
			m_nCurrentSection = -1;
			// Disable fields that depend on Section content.
			EnableControls(FALSE);
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return; // Can't do anything else until a selection is made.
		}

		// Now we need to get the new procedure ID that was selected.
		m_nProcedureID = VarLong(pRow->GetValue(pseProcColID));
		m_nProcedureID = SetProcedureSelection(m_nProcedureID);
		if (m_nProcedureID <= 0) {
			m_nProcedureID = SetProcedureSelectionToFirst();
		}

		// Since we have switched to a new procedure, also switch to the first section
		//  for that procedure.
		LoadSectionList();
		m_nCurrentSection = SetSectionSelectionToFirst();
		LoadDataForSection(m_nCurrentSection);
		EnsureNextBackButtonState();

	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnSelChosenProcedureContentsList");
}

void CProcedureSectionEditDlg::OnRequeryFinishedProcedureContentsList(short nFlags) 
{
	// (d.moore 2007-06-19 13:14) - PLID 23861 - Added a procedure selection dropdown.
	try {
		// First check to make sure that there is at least one procedure in the list.
		if (m_pProcedureList->GetRowCount() <= 1) {
			// There is nothing in the list. This should be a rare case, but disable most
			//  of the controls in the dialog. There isn't anything you can do without
			//  a procedure selected.
			m_nProcedureID = -1;
			m_nCurrentSection = -1;
			EnableControls(FALSE);
			GetDlgItem(IDC_CUSTOM_SECTION_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
			return;
		}

		// (d.moore 2007-07-31) - PLID 23861 - Remove the 'No Procedures' row.
		// This is part of a work around. If the SQL statement for m_pProcedureList returns
		//  no records then a BOF or EOF error was occuring. The error is not caused by any
		//  function in this class as far as I can tell.
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProcedureList->FindByColumn(pseProcColID, (long)-1, NULL, false);
		m_pProcedureList->RemoveRow(pRow);

		// Color any procedure entry that has sections marked 'Need Reviewed'
		pRow = m_pProcedureList->GetFirstRow();
		while (pRow != NULL) {
			if (VarBool(pRow->GetValue(pseProcColNeedReview), false)) {
				pRow->PutForeColor(RGB(255, 0, 0));
				pRow->PutForeColorSel(RGB(255, 0, 0));
			}
			pRow = pRow->GetNextRow();
		}

		// Set the selection for the Procedure.
		m_nProcedureID = SetProcedureSelection(m_nProcedureID);
		if (m_nProcedureID <= 0) {
			m_nProcedureID = SetProcedureSelectionToFirst();
			m_nCurrentSection = -1;
		}

		// Now attempt to get the list of sections that match the selected procedure.
		GetDlgItem(IDC_CUSTOM_SECTION_COMBO)->EnableWindow(TRUE);
		LoadSectionList();
		if (m_nCurrentSection >= 0) {
			m_nCurrentSection = SetSectionSelection(m_nCurrentSection);
		}
		// Check to make sure something actually got set.
		if (m_nCurrentSection < 0) {
			m_nCurrentSection = SetSectionSelectionToFirst();
		}
		LoadDataForSection(m_nCurrentSection);
		EnsureNextBackButtonState();
		// Initializing starts in the constructor and ends the first time this point is reached.
		m_bInitializing = false;
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnRequeryFinishedProcedureContentsList");
}

void CProcedureSectionEditDlg::OnCheckFilterNeedReviewed() 
{
	// (d.moore 2007-06-21 10:31) - PLID 23861 - Added filtering for procedure and section lists.
	try {
		StoreSectionText();

		bool bChanged = false;
		for (long i = 0; i < SECTION_COUNT; i++) {
			if (m_arSections[i].bChanged) {
				bChanged = true;
				break;
			}
		}
		
		if (bChanged) {
			CString strMessage = 
				"Changing this filter may cause a new procedure to "
				"be selected. Would you like to save the changes you "
				"have made to the current procedure before proceeding?";
			if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
				ValidateAndSave();
			}
		}
		
		LoadProcedures();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnCheckFilterNeedReviewed");
}

void CProcedureSectionEditDlg::OnCheckFilterSectionsWithContent() 
{
	// (d.moore 2007-06-21 10:31) - PLID 23861 - Added filtering for procedure and section lists.
	try {
		StoreSectionText();
		LoadSectionList();
		m_nCurrentSection = SetSectionSelection(m_nCurrentSection);
		if (m_nCurrentSection < 0) {
			m_nCurrentSection = SetSectionSelectionToFirst();
		}
		LoadDataForSection(m_nCurrentSection);
		EnsureNextBackButtonState();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnCheckFilterSectionsWithContent");
}

void CProcedureSectionEditDlg::LoadProcedures()
{
	// (d.moore 2007-06-19 11:02) - PLID 23861 - Added a procedure dropdown list.
	try {
		// Prepare the filter for the query.
		BOOL bShowOnlyReviewed = IsDlgButtonChecked(IDC_CHECK_FILTER_NEED_REVIEWED);

		CString strWhere;
		if (bShowOnlyReviewed) {
			if (!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			
			strWhere += 
				"(ReviewAlternatives = 1 OR ReviewBandages = 1 OR ReviewComplications = 1 OR "
				"ReviewConsent = 1 OR ReviewAltConsent = 1 OR ReviewHospitalStay = 1 OR "
				"ReviewMini = 1 OR ReviewPostOp = 1 OR ReviewPreOp = 1 OR "
				"ReviewProcDetails = 1 OR ReviewRecovery = 1 OR ReviewRisks = 1 OR "
				"ReviewShowering = 1 OR ReviewDiet = 1 OR ReviewDayOf = 1 OR "
				"ReviewCustom1 = 1 OR ReviewCustom2 = 1 OR ReviewCustom3 = 1 OR "
				"ReviewCustom4 = 1 OR ReviewCustom5 = 1 OR ReviewCustom6 = 1 OR "
				"ReviewCustom7 = 1 OR ReviewCustom8 = 1 OR ReviewCustom9 = 1 OR "
				"ReviewCustom10 = 1)";
		}
		
		if (!strWhere.IsEmpty()) {
			strWhere = " WHERE " + strWhere;
		}

		CString strFrom;
		strFrom.Format(
			"(SELECT -1 AS ID, 'No Procedures' AS Name, CONVERT (bit, 0) AS NeedsReview "
			"UNION "
			"SELECT ID, Name, "
			"CONVERT (bit, CASE WHEN (ReviewAltConsent = 1 OR ReviewCustom1 = 1 OR ReviewCustom2 = 1 OR ReviewCustom3 = 1 OR ReviewCustom4 = 1 OR "
			"ReviewCustom5 = 1 OR ReviewCustom6 = 1 OR ReviewCustom7 = 1 OR ReviewCustom8 = 1 OR ReviewCustom9 = 1 OR "
			"ReviewCustom10 = 1 OR ReviewMini = 1 OR ReviewDayOf = 1 OR ReviewRecovery = 1 OR ReviewProcDetails = 1 OR ReviewPreOp = 1 OR "
			"ReviewRisks = 1 OR ReviewPostOp = 1 OR ReviewHospitalStay = 1 OR ReviewConsent = 1 OR ReviewBandages = 1 OR "
			"ReviewShowering = 1 OR ReviewDiet = 1 OR ReviewComplications = 1 OR ReviewAlternatives = 1) THEN 1 ELSE 0 END) AS NeedsReview "
			"FROM ProcedureT %s ) AS ProcedureQ "
			"GROUP BY ProcedureQ.ID, ProcedureQ.Name, ProcedureQ.NeedsReview "
			"ORDER BY ProcedureQ.Name", strWhere);

		m_pProcedureList->FromClause = _bstr_t(strFrom);
		
		m_pProcedureList->Requery();
	} NxCatchAll("Error In: CProcedureSectionEditDlg::LoadCategories");
}

// (d.moore 2007-06-21 14:16) - PLID 13861 - Enable or disable all controls not related to navigation.
void CProcedureSectionEditDlg::EnableControls(BOOL bState)
{
	if (!bState) {
		SetRtfText(&m_richeditSectionText, "", SF_RTF);
		CheckDlgButton(IDC_NEEDS_REVIEWED, false);
	}
	
	GetDlgItem(IDC_NEEDS_REVIEWED)->EnableWindow(bState);
	GetDlgItem(IDC_SECTION_EDIT)->EnableWindow(bState);
	GetDlgItem(IDC_COPY_SECTIONS)->EnableWindow(bState);
	GetDlgItem(IDOK)->EnableWindow(bState);
	GetDlgItem(IDC_PREVIEW_ITEM)->EnableWindow(bState);
	GetDlgItem(IDC_RENAME_BTN)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_SUPERSCRIPT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_SUBSCRIPT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_BULLETS)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_ALIGN_RIGHT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_ALIGN_LEFT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_ALIGN_CENTER)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_UNDO)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_UNDERLINE)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_ITALIC)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_BOLD)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_FONT_LIST)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_COLOR)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_SUPERSCRIPT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_SUBSCRIPT)->EnableWindow(bState);
	GetDlgItem(IDC_PROC_SECTION_FONT_HEIGHT)->EnableWindow(bState);
	GetDlgItem(IDC_FONT_HEIGHT_SPIN)->EnableWindow(bState);
}

// // (d.moore 2007-06-21 16:47) - PLID 23861 - Make sure that next and back buttons are properly enabled.
void CProcedureSectionEditDlg::EnsureNextBackButtonState()
{
	// First check that there are valid procedure and section rows selected.
	NXDATALIST2Lib::IRowSettingsPtr pProcRow = m_pProcedureList->CurSel;
	if (pProcRow == NULL) {
		GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
		return;
	}

	// Now make comparisons to first and last rows.
	NXDATALIST2Lib::IRowSettingsPtr pSecRow = m_dlSectionCombo->CurSel;
	NXDATALIST2Lib::IRowSettingsPtr pProcFirst = m_pProcedureList->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pProcLast = m_pProcedureList->GetLastRow();
	NXDATALIST2Lib::IRowSettingsPtr pSecFirst = m_dlSectionCombo->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pSecLast = m_dlSectionCombo->GetLastRow();
	
	if (pSecRow == NULL) { // Only use procedure rows.
		// Back button: On if the procedure is not the first and section is not the first in the list.
		if (pProcRow->IsSameRow(pProcFirst)) {
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(TRUE);
		}

		// Next button: On if the procedure is not the very last and the section is also the last in the list.
		if (pProcRow->IsSameRow(pProcLast)) {
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
		}
	} else {
		// Back button: On if the procedure is not the first and section is not the first in the list.
		if (pProcRow->IsSameRow(pProcFirst) && pSecRow->IsSameRow(pSecFirst)) {
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(TRUE);
		}

		// Next button: On if the procedure is not the very last and the section is also the last in the list.
		if (pProcRow->IsSameRow(pProcLast) && pSecRow->IsSameRow(pSecLast)) {
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
		}
	}
}

void CProcedureSectionEditDlg::OnClose() 
{
	try {
		bool bChanged = false;
		long i;
		for (i = 0; i < SECTION_COUNT; i++) {
			if (m_arSections[i].bChanged) {
				bChanged = true;
				break;
			}
		}
		
		if (!bChanged) {
			CDialog::OnCancel();
		} else {
			CString strMessage = "The sections for the current procedure have not been saved.\n"
				"Would you like to save them before exiting?";
			
			if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
				if (Save()) {
					EndDialog(IDOK);
				}
			} else {
				CDialog::OnCancel();
			}
		}
		// (d.moore 2007-10-05) - PLID 23861 - We need to reset the array containing the
		//  Change flags before closing.
		for (i = 0; i < SECTION_COUNT; i++) {
			m_arSections[i].bChanged = false;
		}
	} NxCatchAll("Error In: CProcedureSectionEditDlg::OnClose");
}

// (d.moore 2007-06-21 14:16) - PLID 13861 - These functions are just to make selecting the right section
//  or procedure easier and clearer in the code. They return the index for the section of procedure that actually got set.
long CProcedureSectionEditDlg::SetProcedureSelection(long nID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->SetSelByColumn(pseProcColID, nID);
	if (pRow != NULL) {
		return nID;
	}
	m_pProcedureList->PutCurSel(NULL);
	return -1;
}

long CProcedureSectionEditDlg::SetProcedureSelectionToFirst()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->GetFirstRow();
	if (pRow != NULL) {
		m_pProcedureList->PutCurSel(pRow);
		long nID = VarLong(pRow->GetValue(pseProcColID));
		return nID;
	}
	m_pProcedureList->PutCurSel(NULL);
	return -1;
}

long CProcedureSectionEditDlg::SetProcedureSelectionToLast()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->GetLastRow();
	if (pRow != NULL) {
		m_pProcedureList->PutCurSel(pRow);
		long nID = VarLong(pRow->GetValue(pseProcColID));
		return nID;
	}
	m_pProcedureList->PutCurSel(NULL);
	return -1;
}

long CProcedureSectionEditDlg::SetSectionSelection(long nIndex)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->SetSelByColumn(psesccArrayIndex, nIndex);
	if (pRow != NULL) {
		return nIndex;
	} else {
		m_dlSectionCombo->PutCurSel(NULL);
		return -1;
	}
}

long CProcedureSectionEditDlg::SetSectionSelectionToFirst()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->GetFirstRow();
	if (pRow != NULL) {
		m_dlSectionCombo->PutCurSel(pRow);
		return VarLong(pRow->GetValue(psesccArrayIndex));
	} else {
		m_dlSectionCombo->PutCurSel(NULL);
		return -1;
	}
}

long CProcedureSectionEditDlg::SetSectionSelectionToLast()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSectionCombo->GetLastRow();
	if (pRow != NULL) {
		m_dlSectionCombo->PutCurSel(pRow);
		return VarLong(pRow->GetValue(psesccArrayIndex));
	} else {
		m_dlSectionCombo->PutCurSel(NULL);
		return -1;
	}
}

// (d.moore 2007-06-27 17:47) - PLID 13861 - Store the text for a section in the m_arSections.
void CProcedureSectionEditDlg::StoreSectionText()
{
	if (m_nCurrentSection < 0 || m_nCurrentSection >= SECTION_COUNT) {
		return;
	}
	
	if(m_arSections[m_nCurrentSection].bChanged) {
		CString strText;
		GetRtfText(&m_richeditSectionText, strText, SF_RTF);
		m_arSections[m_nCurrentSection].strText = strText;
	}
}

void CProcedureSectionEditDlg::SetProcedure(long nID)
{
	m_nProcedureID = nID;
	if (m_bInitializing) {
		// When the dialog initializes, the procedure will be selected.
		return;
	}
	
	// (d.moore 2007-09-26) - PLID 23861 - 
	// Check to see if there have been changes made that may be lost.
	bool bChanged = false;
	for (long i = 0; i < SECTION_COUNT; i++) {
		if (m_arSections[i].bChanged) {
			bChanged = true;
			break;
		}
	}
	if (bChanged) {
		CString strMessage = 
		"You may be about to switch to a new procedure.\n"
		"Would you like to save the changes you have \n"
		"made to the current procedure?";
		if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
			StoreSectionText();
			ValidateAndSave();
		}
	}
	
	// Attempt to set selection and update the data displayed.
	m_nProcedureID = SetProcedureSelection(m_nProcedureID);
	if (m_nProcedureID <= 0) {
		m_nProcedureID = SetProcedureSelectionToFirst();
		// A prodecure ID was set externally but can not be found in the list.
		//  This most likely means that the filters have screened out the item.
		MessageBox("The currently selected procedure does not match the default filters.\n"
			"The first procedure in the list will be selected instead.");		
	}
	
	// Since we have loaded a new procedure, switch to the first section for that procedure.
	LoadSectionList();
	m_nCurrentSection = SetSectionSelectionToFirst();
	LoadDataForSection(m_nCurrentSection);
	EnsureNextBackButtonState();
}

void CProcedureSectionEditDlg::OnProcSpellCheck() 
{
	try {
		short nResult = PerformSpellCheck(this, m_richeditSectionText.GetSafeHwnd());
		if (nResult == 0) {
			MessageBox("Spell check completed successfully.", NULL, MB_OK|MB_ICONINFORMATION);
		} else {
			CString strErr;
			strErr.Format("ERROR %hi was reported while trying to check spelling!", nResult);
			MessageBox(strErr, NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll ("Error In: CProcedureSectionEditDlg::OnProcSpellCheck");
}

// (d.moore 2007-07-05 11:23) - PLID 23863 - Check to see if the user wants to remove the 'Needs Reviewed' setting for the section.
void CProcedureSectionEditDlg::PromptRemoveNeedsReviewed(long nSectionIndex)
{
	// If the section matching nSectionIndex is marked 'Needs Reviewed' and the user did
	//  not click on the checkbox to set it, then prompt the user to see if they want the flag
	//  removed. This function should usually be called only when transitioning from one section
	//  to another.

	// Check to make sure the nSectionIndex is in proper bounds.
	if (nSectionIndex < 0 || nSectionIndex >= SECTION_COUNT) {
		return;
	}

	if (!m_bNeedsReviewedWasSet && m_arSections[nSectionIndex].bNeedsReview) {
		CString strMessage = "Would you like to mark this section as Reviewed?";
		if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
			m_arSections[nSectionIndex].bNeedsReview = false;
			// (j.jones 2011-08-12 15:33) - PLID 38268 - need to flag this section as changed
			m_arSections[nSectionIndex].bChanged = true;
			RemoveNeedsReviewedText(&m_richeditSectionText);
			m_richeditSectionText.SetFocus();
		}
	}
}


// (d.moore 2007-09-26) - PLID 23863 - When switching between Procedures lets prompt them that the data is about
//  to be saved. This should eliminate some confusion.
BOOL CProcedureSectionEditDlg::PromptForSaveOnProcedureChange()
{
	bool bChanged = false;
	for (long i = 0; i < SECTION_COUNT; i++) {
		if (m_arSections[i].bChanged) {
			bChanged = true;
			break;
		}
	}
	if (!bChanged) {
		// Nothing changed, so we don't really need to do anything here.
		return FALSE;
	}
	
	CString strMessage = 
		"You are about to switch to a new procedure.\n"
		"Would you like to save the changes you have \n"
		"made to the current procedure?";
	if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
		return TRUE;
	}
	return FALSE;
}

// (z.manning, 10/11/2007) - PLID 27719 - This function will attempt to update the font for every NexForms field
// of every procedure.
void CProcedureSectionEditDlg::OnProcedureSectionMenuUpdateFont()
{
	try
	{
		// (z.manning, 10/11/2007) - PLID 27719 - Well, I really would have liked to just get the IDs from the
		// datalist here. Unfortunately, the procedure list here doesn't load all procedures, so we're gonna
		// have to get the IDs from data.
		CArray<long,long> arynProcedureIDs;
		_RecordsetPtr prsAllProcedureIDs = CreateParamRecordset(
			"SELECT ID FROM ProcedureT WHERE COALESCE(NexFormsFontVersion, 0) < {INT}", NEXFORMS_FONT_VERSION);
		for(; !prsAllProcedureIDs->eof; prsAllProcedureIDs->MoveNext()) {
			arynProcedureIDs.Add(AdoFldLong(prsAllProcedureIDs, "ID"));
		}

		if(arynProcedureIDs.GetSize() > 0)
		{
			CString strMessage = "You must save everything first? Would you like to save now?";
			if(MessageBox(strMessage, "Save Content", MB_YESNO) != IDYES) {
				return;
			}
			StoreSectionText();
			if(!ValidateAndSave()) {
				return;
			}

			// (z.manning, 10/11/2007) - PLID 27719 - Warn them about what they are about to do.
			strMessage = 
				"The NexForms content you are importing is designed to work best with content that uses the 'Tahoma' font. "
				"Would you like to have your content automatically re-formatted now?\r\n\r\n"
				"If you say 'Yes', all your existing NexForms content fields will be modified to use the Tahoma font.  Only the formatting of the text will be affected, the text itself will be unchanged.\r\n"
				"If you say 'No', your existing content will not be modified, which may cause formatting issues if that content is merged to one of the Word templates being imported.";
			if(MessageBox(strMessage, "Update Font", MB_YESNO) != IDYES) {
				return;
			}

			// (z.manning, 10/11/2007) - PLID 27719 - We are going to be updating a ton of data here, so let's
			// confirm one more time;
			strMessage = "This change can not be undone. Are you sure you want to update the font for every procedure?";
			if(MessageBox(strMessage, "Update Font", MB_YESNO) != IDYES) {
				return;
			}

			// (z.manning, 10/11/2007) - PLID 27719 - Ok, it's go time. Note: this function has its own progress bar.
			UpdateFontForExistingProcedures(arynProcedureIDs);
			
			LoadProcedures();
			LoadDataForSection(m_nCurrentSection);

			MessageBox("Finished", "Finished");
		}
		else {
			MessageBox("All of your procedures have already been updated with the correct font.");
		}

	}NxCatchAll("CProcedureSectionEditDlg::OnProcedureSectionMenuUpdateFont");
}
