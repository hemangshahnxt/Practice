#include "stdafx.h"
#include <NxUILib/NxOccManager.h>
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "EMRItemAdvPopupWnd.h"
#include "EMRItemAdvPopupDlg.h"
#include "EMRItemAdvListDlg.h"
#include "EmrUtils.h"
#include "EMNDetail.h"
#include "EMRTopic.h"
#include "EMN.h"
#include "MultiSelectDlg.h"
#include "NxExpression.h"
#include "EmrTableEditCalculatedFieldDlg.h"
#include "Mirror.h"
#include "DontShowdlg.h"
#include "EmrTreeWnd.h"
#include "LabEntryDlg.h"
#include "EmrEditorDlg.h"
#include "PicContainerDlg.h"
#include "EmrTextMacroDlg.h"
#include "SelectStampDlg.h"
#include "StampSearchDlg.h"
#include "TopazSigPad.h"
#include "EMRItemAdvMultiPopupDlg.h"
#include "EMR.h"
#include "NxImageCache.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

extern CPracticeApp theApp;

using namespace ADODB;


// (a.walling 2012-10-03 12:09) - PLID 53002 - Use list-specific fonts for list items

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXINKPICTURELib;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupWnd

CEMRItemAdvPopupWnd::CEMRItemAdvPopupWnd()
{
	m_nCurPenColor = 0;
	m_fltCurPenSize = 53;
	m_bIsEraserState = FALSE;
	m_clrHilightColor = 0;
	m_pDetail = NULL;
	m_pParentDlg = NULL;
	m_bIsActiveCurrentMedicationsTable = FALSE;
	m_bIsActiveAllergiesTable = FALSE;
	m_bReadOnly = FALSE;
	// (c.haag 2007-08-13 16:14) - PLID 25970 - Copied from
	// CEmrItemAdvNarrativeDlg due to Tom copying other functions over
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//m_bAddingMergeFields = FALSE;
	// (c.haag 2007-08-15 16:34) - PLID 27084 - Garbage collection
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//m_pstrRequestFormatRichTextCtrlResult = NULL;
	// (c.haag 2007-08-20 15:14) - PLID 27127 - By default, when we call
	// ReflectTableState, it should populate the datalist.
	m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
	//DRT 7/29/2008 - PLID 30824 - Converted to datalist2
	m_pEditingFinishedRow = NULL;
	m_nEditingFinishedCol = -1;
	m_nTopMargin = 0;
	m_nLeftMargin = 0;
	m_szIdeal = CSize(0,0);
	m_pRealDetail = NULL;
	//DRT 4/30/2008 - PLID 29771 - For setting the background color
	m_brBackground.CreateSolidBrush(GetNxColor(GNC_EMR_ITEM_BG, 0));

	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	m_szRequestFormatBuffer = NULL;

	// (a.walling 2010-05-19 11:19) - PLID 38778 - Option to shrink to fit the display area
	m_bShrinkFitToArea = GetRemotePropertyInt("NxInkPicture_ShrinkToFitArea", TRUE, 0, GetCurrentUserName(), true) ? true : false;
	// (r.gonet 02/14/2013) - PLID 40017 - Record whether we need to save column resizes if we have a table.
	m_bRememberPoppedUpTableColumnWidths = FALSE;
}

CEMRItemAdvPopupWnd::~CEMRItemAdvPopupWnd()
{
	// (c.haag 2008-01-15 17:40) - PLID 17936 - Clear out all dropdown source-related
	// content data
	ClearDropdownSourceInfoMap();

	// (c.haag 2007-08-15 16:32) - PLID 27084 - Garbage collection
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	/*
	if (m_pstrRequestFormatRichTextCtrlResult) {
		delete m_pstrRequestFormatRichTextCtrlResult;
	}
	*/
	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	if (m_szRequestFormatBuffer) {
		delete[] m_szRequestFormatBuffer;
		m_szRequestFormatBuffer = NULL;
	}
}

// (z.manning 2011-10-11 11:14) - PLID 42061 - Added stamp ID
CString CEMRItemAdvPopupWnd::GetDropdownSource(long nColumnID, const long nStampID)
{
	// (z.manning 2011-03-11) - PLID 42778 - Moved this logic to the base class
	// (j.jones 2015-04-02 16:31) - PLID 61625 - the confirmation on whether this is on a real EMN
	// or merely in the "Preview Table" is supposed to compare on m_pRealDetail, not m_pDetail
	return CEmrItemAdvTableBase::GetDropdownSource(nColumnID, m_pDetail, m_pRealDetail != NULL, nStampID);
}

// (a.walling 2007-06-25 10:23) - PLID 22097 - I don't know why the owner of the event sink map is set to
// CEmrItemAdvImageDlg for everything; seems like it should be set to CEMRItemAdvPopupWnd. I made this change
// at least for the narrative events since that is all I am touching at the moment.
//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
BEGIN_EVENTSINK_MAP(CEMRItemAdvPopupWnd, CWnd)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 10 /* EditingFinished */, OnEditingFinishedTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 2 /* Stroke */, OnStrokeInkPicture, VTS_DISPATCH VTS_DISPATCH VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 3 /* Browse */, OnBrowseInkPicture, VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 5 /* TextChanged */, OnTextChangedInkPicture, VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 1 /* TextChanged */, OnTextChangedRichTextCtrl, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 2 /* Link */, OnLinkRichTextCtrl, VTS_BSTR)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 4 /* RequestFormat */, OnRequestFormatRichTextCtrl, VTS_BSTR VTS_BSTR VTS_PBSTR)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 5 /* RightClickField */, OnRightClickFieldRichTextCtrl, VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	//ON_EVENT(CEMRItemAdvPopupWnd, 8000, 6 /* RequestField */, OnRequestFieldRichTextCtrl, VTS_BSTR)
	//ON_EVENT(CEMRItemAdvPopupWnd, 8000, 7 /* RequestAllFields */, OnRequestAllFieldsRichTextCtrl, VTS_NONE)
	//ON_EVENT(CEMRItemAdvPopupWnd, 8000, 8 /* ResolvePendingMergeFieldValue */, OnResolvePendingMergeFieldValue, VTS_PDISPATCH)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 6 /* CustomStampsChanged */, OnCustomStampsChangedInkPicture, VTS_BSTR)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 9 /* RequestLWMergeFieldData */, OnRequestLWMergeFieldData, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 9 /* EditingFinishing */, OnEditingFinishingTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 7 /* ClickedHotSpot */, OnClickedHotSpotInkPicture, VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 8 /* EditingStarting */, OnEditingStartingTable, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 6 /* RButtonDown */, OnRButtonDownTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedTable, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 32 /* ShowContextMenu */, OnShowContextMenuTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, TABLE_IDC, 5 /* LButtonUp */, OnLButtonUpTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 10 /* RequestAllAvailableFields */, OnRequestAllAvailableFields, VTS_BOOL VTS_BOOL VTS_BOOL VTS_PVARIANT)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 13 /* RequestAvailableFieldsVersion */, OnRequestAvailableFieldsVersion, VTS_PI2)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 8 /* OpenStampSetup */, OnOpenStampSetupInkPicture, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 11 /* OpenStampFilterSetup */, OnOpenStampFilterSetupInkPicture, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 12 /* OpenStampSearch */, OnOpenStampSearch, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 31 /* OpenStampFilterSetup */, OnOpenStampFilterSetup3DControl, VTS_NONE)
	// (a.walling 2010-05-19 11:25) - PLID 38778 - Toggle between resized / full mode
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 4 /* FullScreen */, OnFullScreenInkPicture, VTS_NONE)
	// (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
	//ON_EVENT(CEMRItemAdvPopupWnd, 4000, 9 /* ZoomLevelChanged */, OnZoomLevelChangedInkPicture, VTS_R8)
	//ON_EVENT(CEMRItemAdvPopupWnd, 4000, 10 /* ViewportOffsetsChanged */, OnViewportOffsetsChangedInkPicture, VTS_I4 VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 1 /* HotSpotClicked */, OnClickedHotSpot3DControl, VTS_I2)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 19 /* SmartStampAdd */, OnSmartStampAdd3DControl, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 20 /* SmartStampErase */, OnSmartStampErase3DControl, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 23 /* OpenStampSetup */, OnOpenStampSetup3DControl, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 24 /* PreviewModified */, OnPreviewModified3DControl, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, NX3D_IDC, 37 /* OpenStampSearch */, OnOpenStampSearch3D, VTS_NONE) // (j.gruber 2012-08-14 12:23) - PLID 52134
	ON_EVENT_RANGE(CEMRItemAdvPopupWnd, 1000, 1999, DISPID_CLICK /* Click */, OnButtonClickedEvent, VTS_I4)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 14 /* CheckValidIDs */, OnCheckValidIDsRichTextCtrl, VTS_I4 VTS_PBOOL VTS_I4 VTS_PBOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 15 /* RightClickHtmlField */, OnRightClickHtmlFieldRichTextCtrl, VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CEMRItemAdvPopupWnd, 8000, 16 /* RightClickHtml */, OnRightClickHtmlRichTextCtrl, VTS_I4 VTS_I4 VTS_BOOL VTS_BOOL)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 13 /* Topaz Signature */, OnClickedTopazSignature, VTS_NONE)
	// (j.armen 2014-07-22 09:15) - PLID 62836 - Handle Pen Color Changed and Pen Size Changed
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 14 /* CurrentPenColorChanged */, OnCurrentPenColorChanged, VTS_NONE)
	ON_EVENT(CEMRItemAdvPopupWnd, 4000, 15 /* CurrentPenSizeChanged */, OnCurrentPenSizeChanged, VTS_NONE)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CEMRItemAdvPopupWnd, CWnd)
	//{{AFX_MSG_MAP(CEMRItemAdvPopupWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CONTEXTMENU()
	ON_WM_ERASEBKGND()
	ON_CONTROL_RANGE(BN_CLICKED, 1000, 1999, OnButtonClicked)
	ON_EN_CHANGE(500, OnChangeEdit)
	ON_MESSAGE(NXM_EMR_ADD_NEW_DROPDOWN_COLUMN_SELECTION, OnAddNewDropdownColumnSelection)
	ON_MESSAGE(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, OnToggleHotSpot)
	ON_MESSAGE(NXM_START_EDITING_EMR_TABLE, OnStartEditingEMRTable)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(501, OnTextSpellCheck)
	ON_BN_CLICKED(502, OnLabButton)
	ON_MESSAGE(NXM_EMR_ITEM_PASTE_MACRO_TEXT, OnPasteMacroText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupWnd message handlers

int CEMRItemAdvPopupWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

// (a.walling 2008-01-18 16:22) - PLID 14982 - Added pRealDetail parameter
// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
void CEMRItemAdvPopupWnd::Initialize(CWnd *pParentDlg, CWnd *pParent, CRect rcWindow, CEMNDetail *pDetail, CEMNDetail *pRealDetail, BOOL bIsDetailIndependent, CSize szMax, CSize szMin, int nTopMargin /*= 0*/, int nLeftMargin /*= 0*/, CSize szIdeal /*= CSize(0,0)*/, bool bDrawBorder /*= false*/)
{
	m_pParentDlg = pParentDlg;

	m_expandedLabelIDs.clear();

	m_szMax = szMax;
	//TES 1/14/2008 - PLID 24157 - Added these three variables, for when this window is embedded on CEMRItemAdvMultiPopupDlg.
	//TES 2/21/2008 - PLID 28827 - Replaced m_szMin with m_szIdeal.
	m_szIdeal = szIdeal;

	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	m_szMin = szMin;

	m_nTopMargin = nTopMargin;
	m_nLeftMargin = nLeftMargin;

	//Create the window.
	CBrush brBackground;
	brBackground.Attach(GetSysColorBrush(COLOR_BTNFACE));
	
	//TES 1/14/2008 - PLID 24157 - Added the option to have this window have a border.
	BOOL bAns = CreateEx(WS_EX_CONTROLPARENT, 
		AfxRegisterWndClass(CS_PARENTDC|CS_NOCLOSE|CS_VREDRAW|CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW), brBackground), NULL, 
		WS_VISIBLE|WS_CHILD|WS_GROUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|(bDrawBorder?WS_BORDER:0), rcWindow, pParent, IDC_PLACEHOLDER);

	if(bAns) {
		
		// (j.jones 2007-09-26 09:57) - PLID 27510 - *Copied from b.cardillo notes elsewhere, such as EmrTopicWnd.cpp*
		// There is a crazy bug in MFC where if you have a subdialog and that subdialog has exactly one child
		// AND that one child is itself a subdialog AND that one child is invisible, then the _AfxNextControl()
		// MFC function gets into an infinitely recursive loop searching for the next child to send keyboard mnemonics
		// to whenever the user hits the keyboard.  By adding a SECOND control that is STATIC (though still invisible)
		// to the original subdialog we save the _AfxNextControl() function from its infinite recursion.
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		// (a.walling 2012-05-07 09:21) - PLID 48934 - This bug is now fixed via CNxOccManager
		//m_wndInvisibleAnchor.Create("invisible static control", WS_CHILD|WS_GROUP, CRect(0,0,0,0), this);
	}

	m_pDetail = pDetail;
	// (a.walling 2008-01-16 15:52) - PLID 14982
	m_pRealDetail = pRealDetail;

	m_bIsDetailIndependent = bIsDetailIndependent;
	m_pDetail->LoadContent();

	
	// (a.walling 2008-02-07 10:37) - PLID 14982 - Get a list of real selections from the real detail, or the detail if failing that
	{
		CEMNDetail* pActiveDetail = m_pRealDetail ? m_pRealDetail : m_pDetail;
		pActiveDetail->LoadContent();// (a.walling 2008-02-08 17:20) - PLID 14982 - Ensure this is done after we LoadContent

		if ((pActiveDetail != NULL) && (pActiveDetail->m_EMRInfoType == eitMultiList || pActiveDetail->m_EMRInfoType == eitSingleList) ) {
			for (int i = 0; i < pActiveDetail->GetListElementCount(); i++) {
				ListElement le = pActiveDetail->GetListElement(i);

				if (le.bIsSelected) {
					m_dwaRealSelectedDataGroupIDs.Add(le.nDataGroupID);
				}
			}
		}
	}

	// (a.walling 2007-06-21 13:47) - PLID 22097 - Get the readonly status from the detail, then EMN
	m_bReadOnly = m_pDetail->GetReadOnly();

	/*
	if (m_pDetail->m_pParentTopic->GetParentEMN()->GetStatus() == 2)
		ASSERT(TRUE);
	*/ // (a.walling 2008-02-08 17:18) - PLID 14982 - This is obsolete code now anyway; we can popup details when locked.

	if (!m_bReadOnly) {
		// try to get the status of the EMN. If it is locked, set readonly.
		if (m_pDetail->m_pParentTopic && m_pDetail->m_pParentTopic->GetParentEMN()
			&& (m_pDetail->m_pParentTopic->GetParentEMN()->GetStatus() == 2)) {
			// the EMN is locked!
			m_bReadOnly = TRUE;
		}
	}

	//TES 4/6/2007 - PLID 25456 - Only lock spawning if our detail is an independent copy.
	if(m_bIsDetailIndependent) m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->LockSpawning();

	if(m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList) {
		// Destroy the old brush
		if (m_brHilight.m_hObject) {
			m_brHilight.DeleteObject();
		}
		// If the new color is not 0 then create the new brush		
		if (m_clrHilightColor != 0) {
			if (m_clrHilightColor & 0x80000000) {
				m_brHilight.CreateSysColorBrush(m_clrHilightColor & 0x000000FF);
			} else {
				m_brHilight.CreateSolidBrush(PaletteColor(m_clrHilightColor));
			}
		}
	}
	// (z.manning, 04/10/2007) - PLID 25560 - We may have popped up a table that was spawned on another topic
	// and thus does not have a valid EmrItemAdvTableDlg, which means it doesn't know its dropdown contents. 
	// So, let's make sure each column gets its query to load the dropdown data.
	// (j.jones 2008-06-03 17:26) - PLID 27232 - this should not recalculate the combo sql if
	// this is called from the EMR item entry table preview
	else if(m_pDetail->m_EMRInfoType == eitTable && NULL != pRealDetail) {
		m_pDetail->CalcComboSql();
	}

	CreateControls();
}

void CEMRItemAdvPopupWnd::OnDestroy() 
{
	DestroyControls();
	
	CWnd::OnDestroy();
}

void CEMRItemAdvPopupWnd::DestroyControls()
{
	if (IsControlValid(&m_wndLabel)) {
		m_wndLabel.DestroyWindow();
	}

	//TES 4/5/2007 - PLID 25456 - Destroy the richedit window.
	if(m_wndRichEdit.m_hWnd) {
		m_wndRichEdit.DestroyWindow();
	}

	// (a.walling 2008-01-18 13:50) - PLID 14982 - Clear out the table, but don't destroy the window
	/*if (m_wndTable.m_hWnd) {
		m_wndTable.DestroyWindow();
	}*/

	// (c.haag 2008-10-17 10:32) - PLID 31700 - Clear the table control
	ClearTableControl();

	// (a.walling 2008-01-18 13:57) - PLID 14982 - Destroy the slider and its ilk
	if (m_Slider.m_hWnd) {
		m_Slider.DestroyWindow();
	}
	if (m_Caption.m_hWnd) {
		m_Caption.DestroyWindow();
	}
	if (m_MinCaption.m_hWnd) {
		m_MinCaption.DestroyWindow();
	}
	if (m_MaxCaption.m_hWnd) {
		m_MaxCaption.DestroyWindow();
	}

	// (a.walling 2008-01-18 14:02) - PLID 14982 - Destroy the Edit control
	if (m_edit.m_hWnd) {
		m_edit.DestroyWindow();
	}

	// (z.manning 2009-01-19 15:27) - PLID 27682 - Added a spell check button
	if(IsWindow(m_btnSpellCheck.GetSafeHwnd())) {
		m_btnSpellCheck.DestroyWindow();
	}

	// (z.manning 2009-09-21) - PLID 33612 - 
	if(IsWindow(m_btnOpenLab.GetSafeHwnd())) {
		m_btnOpenLab.DestroyWindow();
	}

	for (long i=0; i<m_arypControls.GetSize(); i++) {
		CWnd *pwnd = m_arypControls.GetAt(i);
		if (pwnd) {
			pwnd->DestroyWindow();
			delete pwnd;
		}
	}

	m_arypControls.RemoveAll();
}

void CEMRItemAdvPopupWnd::CreateControls()
{
	//load the data

	// (a.wetta 2007-03-09 10:45) - PLID 24757 - *****FYI***** When adding controls make sure that the controls you want the user to be
	// able to tab to have the WS_TABSTOP style.  Also, a control can only have focus if it has this style.  So, at least one control needs
	// to have the tab stop style so that when the pop up dialog comes up, the OK or Cancel button does not have the initial focus.
	
	DestroyControls();

	// (a.walling 2007-06-21 13:13) - PLID 22097 - Style to create disabled if we are readonly
	DWORD dwDisabled = m_bReadOnly ? WS_DISABLED : 0;

	CString strLabel;
	// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Prefix the name with an asterisk if it's required and not filled in
	if (m_pDetail->IsRequired() && !m_pDetail->IsStateSet()) {
		strLabel = _T("* ");
	}
	// Append the detail label text
	strLabel += m_pDetail->GetMergeFieldOverride().IsEmpty()?m_pDetail->GetLabelText():m_pDetail->GetMergeFieldOverride();
	// Clean it for rendering in a label control
	strLabel.Replace("&", "&&");

	// (a.wetta 2007-02-07 14:23) - PLID 24564 - Let's determine if this table is the active Current Medications table
	m_bIsActiveCurrentMedicationsTable = FALSE;
	m_bIsActiveAllergiesTable = FALSE;
	if(m_pDetail->IsCurrentMedicationsTable()) {

		// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR
		long nActiveCurrentMedicationsInfoID = -2;
		//do memory checks
		if(m_pDetail->m_pParentTopic) {
			if(m_pDetail->m_pParentTopic->GetParentEMN()) {
				if(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()) {
					nActiveCurrentMedicationsInfoID = m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
				}
			}
		}

		if(nActiveCurrentMedicationsInfoID == -2) {
			//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
			//but why don't we have an EMR?
			ASSERT(FALSE);
			nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
		}
		
		if(nActiveCurrentMedicationsInfoID == m_pDetail->m_nEMRInfoID) {
			m_bIsActiveCurrentMedicationsTable = TRUE;
		}
	}
	// (c.haag 2007-04-02 15:43) - PLID 25465 - Also determine if this is the active Allergies table
	else if(m_pDetail->IsAllergiesTable()) {

		// (j.jones 2007-07-24 09:27) - PLID 26742 - the allergies info ID is cached in CEMR
		long nActiveCurrentAllergiesInfoID = -2;
		//do memory checks
		if(m_pDetail->m_pParentTopic) {
			if(m_pDetail->m_pParentTopic->GetParentEMN()) {
				if(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()) {
					nActiveCurrentAllergiesInfoID = m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
				}
			}
		}

		if(nActiveCurrentAllergiesInfoID == -2) {
			//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
			//but why don't we have an EMR?
			ASSERT(FALSE);
			nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
		}
		
		if(nActiveCurrentAllergiesInfoID == m_pDetail->m_nEMRInfoID) {
			m_bIsActiveAllergiesTable = TRUE;
		}
	}

	// Add the label
	if (!strLabel.IsEmpty()) {		
		// (a.walling 2007-06-21 13:13) - PLID 22097 - Create disabled if necessary
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_wndLabel.CreateControl(strLabel, WS_VISIBLE | WS_GROUP | dwDisabled, CRect(0, 0, 0, 0), this, 0xffff);
		//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_wndLabel->NativeFont = EmrFonts::GetTitleFont();
	}

	// Create the controls
	if(m_pDetail->m_EMRInfoType == eitText) {
		// (a.walling 2007-06-21 13:15) - PLID 22097 - Create disabled if necessary
		m_edit.CreateEx(
		WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY, _T("EDIT"), NULL, 
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|WS_GROUP|ES_LEFT|ES_MULTILINE|ES_WANTRETURN, 
		CRect(0, 0, 300, 300), this, 500);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_edit.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));

		// (z.manning 2008-12-16 12:55) - PLID 27682 - Added spell check button
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_btnSpellCheck.Create("", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON|BS_CENTER|BS_OWNERDRAW|dwDisabled, CRect(0,0,0,0), this, 501);
		m_btnSpellCheck.SetIcon(IDI_SPELLCHECK_ICON);
		m_btnSpellCheck.SetToolTip("Check Spelling");
		m_btnSpellCheck.EnableWindow(!m_bReadOnly);

		// (a.walling 2007-06-21 12:36) - PLID 22097 - Set readonly
		m_edit.SetReadOnly(m_bReadOnly);
		// cannot disable or we can't scroll!

		if(m_pDetail->IsLabDetail())
		{
			// (z.manning 2009-09-21) - PLID 33612 - Added lab button to popped up text boxes
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_btnOpenLab.Create("", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON|BS_CENTER|BS_OWNERDRAW|dwDisabled, CRect(0,0,0,0), this, 502);
			// (z.manning 2012-07-20 15:01) - PLID 51676 - Use a smaller font for this button's text
			m_btnOpenLab.SetFont(CFont::FromHandle(EmrFonts::GetSmallFont()));
			m_btnOpenLab.SetTextColor(RGB(0,0,255));
			m_btnOpenLab.SetWindowText("Open Lab");
			m_btnSpellCheck.EnableWindow(FALSE);
			m_edit.SetReadOnly(TRUE);
		}

		m_edit.SetWindowText(AsString(m_pDetail->GetState()));
		// (b.cardillo 2007-02-14 15:41) - PLID 24486 - For some reason under certain conditions, setting 
		// focus here was causing a 5 to 30 second freeze of Practice.  It only froze like that on a few 
		// computers, too (see pl item notes).  Oddly, setting the focus here was not effective anyway, 
		// on any computer, even when it didn't cause a freeze.  Same is true of the slider popup, but 
		// that one doesn't cause the freeze so it's not related to this pl item.  I've just removed the 
		// m_edit.SetFocus() that was being called here so as to resolve this pl item, and I made another 
		// pl item (plid 24757) to address the fact that no pop-up details have the focus set properly.
	}
	else if(m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList) {

		EmrListState listState(m_pDetail);
		
		std::vector<long> visited;

		for (long i=0; i< m_pDetail->GetListElementCount() && i < 1000; i++) {
			ListElement le = m_pDetail->GetListElement(i);

			visited.clear();

			if (le.nParentLabelID != -1 && !listState.selected.count(le.nID)) 
			{
				// (a.walling 2012-12-03 12:57) - PLID 53988 - don't show unless we are expanded and all parents are also expanded
				bool bParentsExpanded = true;
				long nParentLabelID = le.nParentLabelID;
				do {
					visited.push_back(nParentLabelID);
					if (!m_expandedLabelIDs.count(nParentLabelID)) {
						bParentsExpanded = false;
						break;
					}
					nParentLabelID = listState.elements[nParentLabelID].nParentLabelID;
					if (find(visited.begin(), visited.end(), nParentLabelID) != visited.end()) {
						TRACE("Loop detected!\r\n");
						ASSERT(FALSE);
						break;
					}
				} while (nParentLabelID != -1);

				if (!bParentsExpanded) {
					continue;
				}
			}

			CString strValue = le.strName;
			// (r.gonet 09/18/2012) - PLID 52713 - We need to escape any ampersands otherwise they will be interpreted as keyboard shortcut definitions.
			strValue.Replace("&", "&&");
			
			if(le.bIsLabel) {
				// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
				// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
				NxWindowlessLib::NxFreeLabelControl* pwnd = new NxWindowlessLib::NxFreeLabelControl;
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				pwnd->CreateControl(strValue, WS_VISIBLE | dwDisabled, CRect(0, 0, 0, 0), this, 1000 + i);
				// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
				(*pwnd)->NativeFont = EmrFonts::GetUnderlineListFont();

				// (a.walling 2012-12-03 12:57) - PLID 53988 - Give them a blue color if clickable, purple if expanded
				if (listState.children.count(le.nID)) {
					if (m_expandedLabelIDs.count(le.nID)) {
						(*pwnd)->ForeColor = HEXRGB(0x800080);
					} else {
						(*pwnd)->ForeColor = HEXRGB(0x0000FF);
					}
					(*pwnd)->Interactive = VARIANT_TRUE;
				}

				m_arypControls.Add(pwnd);
			}
			else {

				// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
				// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
				NxWindowlessLib::NxFreeButtonControl* pwnd = new NxWindowlessLib::NxFreeButtonControl;
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				pwnd->CreateControl(strValue, WS_VISIBLE | WS_TABSTOP | ((i == 0) ? WS_GROUP : 0) | dwDisabled, CRect(0, 0, 0, 0), this, 1000 + i);

				if (m_pDetail->m_EMRInfoType == eitSingleList) {
					(*pwnd)->RadioStyle = VARIANT_TRUE;
				}

				long nActionBold = GetRemotePropertyInt("EMNTemplateActionBold", 1, 0, GetCurrentUserName(), true);
				//1 - only if it spawns details
				//2 - if it spawns anything
				//0 - never bold

				// (j.jones 2007-07-31 08:54) - PLID 26881 - the ListElement's nActionsType will be
				// -1 if there are no actions, 1 if there are EMR item or Mint Item actions, and 2 if there are
				// other actions but no EMR item or Mint Item actions

				if(m_pDetail->m_bIsTemplateDetail && ((nActionBold == 1 && le.nActionsType == 1) ||
					(nActionBold == 2 && le.nActionsType >= 1))) {
						(*pwnd)->ForeColor = RGB(174,0,0);
				}

				if (m_clrHilightColor & 0x80000000) {
					(*pwnd)->BackColor = GetSysColor(m_clrHilightColor & 0x000000FF);
				} else {
					(*pwnd)->BackColor = PaletteColor(m_clrHilightColor);
				}
				
				// (j.jones 2011-04-28 14:39) - PLID 43122 - bold this if IsFloated
				// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
				if(le.bIsFloated) {
					(*pwnd)->NativeFont = EmrFonts::GetBoldListFont();
				} else {
					(*pwnd)->NativeFont = EmrFonts::GetRegularListFont();
				}

				m_arypControls.Add(pwnd);
			}
		}

		ReflectListState();
	}
	//(j.anspach 07-11-2005 10:18 PLID 16440) - Adding support for image popups.
	else if(m_pDetail->m_EMRInfoType == eitImage)
	{		
		// (z.manning 2011-10-25 13:04) - PLID 39401 - Pass in the detail
		CString strStamps = GenerateInkPictureStampInfo(m_pDetail);
		if(m_pDetail->Is3DImage())
		{
			// (z.manning 2011-07-21 15:50) - PLID 44649 - Added support for 3D images
			if(m_wndImage.m_hWnd == NULL) {
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				if(!m_wndImage.CreateControl(__uuidof(Nx3DLib::Nx3D), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0, 0, 0, 0), this, NX3D_IDC)) {
					ASSERT(FALSE);
					AfxThrowNxException("Failed to create wnd for 3D control on popup");
				}
				m_p3DControl = m_wndImage.GetControlUnknown();
			}

			m_p3DControl->SetTextDataVersion(CURRENT_TEXT_STRUCTURE_VERSION);

			m_p3DControl->SetReadOnly(m_bReadOnly ? VARIANT_TRUE : VARIANT_FALSE);

			// (j.armen 2011-09-06 15:44) - PLID 45245 - Implemented BackGround Color
			m_p3DControl->BackColor = GetNxColor(GNC_EMR_ITEM_BG, 0);

			// (j.armen 2011-09-15 14:50) - PLID 45424 - Set the background of the opengl area
			m_p3DControl->GLBackColor = (COLORREF)GetRemotePropertyInt("Nx3DBackgroundColor", GetNxColor(GNC_EMR_ITEM_BG, 0), 0, GetCurrentUserName(), true);

			// (j.armen 2011-09-06 17:18) - PLID 45347 - Implemented Full Screen
			m_p3DControl->IsFullScreen = VARIANT_TRUE;

			m_p3DControl->UseCustomStamps = (GetRemotePropertyInt("EMR_Image_Use_Custom_Stamps", 1, 0, GetCurrentUserName(), TRUE) == 1 ? VARIANT_TRUE : VARIANT_FALSE);
			m_p3DControl->CustomStamps = _bstr_t(strStamps); // (z.manning 2011-09-07 16:56) - PLID 44693

			// (j.armen 2011-10-10 12:43) - PLID 45849
			m_p3DControl->UseCustomViews = VARIANT_TRUE;

			// (j.armen 2011-10-10 16:56) - PLID 45245
			m_p3DControl->IsItemSetup = VARIANT_FALSE;
		}
		else
		{
			//create the NxInkPicture control
			// (a.walling 2008-01-18 14:03) - PLID 14982 - Don't create if it already exists
			if (m_wndImage.m_hWnd == NULL) {
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				m_wndImage.CreateControl(__uuidof(NxInkPicture), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0, 0, 0, 0), this, 4000);
				//link the ctrlpointer to the main control

				m_Image = m_wndImage.GetControlUnknown();
			}
			m_Image->ReadOnly = m_bReadOnly ? VARIANT_TRUE : VARIANT_FALSE; // (a.walling 2007-06-21 12:37) - PLID 22097 - Set readonly
			// (a.walling 2010-05-19 11:23) - PLID 38778 - Toggle between resized / full mode (always enabled now)
			m_Image->IsFullScreen = VARIANT_FALSE;
			// (c.haag 2009-02-18 10:15) - PLID 31327 - Synchronize the hot spot click property with the real detail
			if (NULL != m_pRealDetail && NULL != ((CEmrItemAdvImageDlg*)m_pRealDetail->m_pEmrItemAdvDlg)) {
				m_Image->PutEnableHotSpotClicks( ((CEmrItemAdvImageDlg*)m_pRealDetail->m_pEmrItemAdvDlg)->GetEnableHotSpotClicks() );
			}

			// (j.jones 2010-04-12 09:30) - PLID 16594 - tell the ocx that we are not using a signature date
			m_Image->PutEnableSignatureDateStamp(VARIANT_FALSE);

			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			if (m_pDetail->IsSignatureDetail())
			{
				m_Image->ViewOnly = VARIANT_TRUE;
			}

			// (a.wetta 2007-04-06 09:36) - PLID 24324 - Load the custom stamps
			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			m_Image->UseCustomStamps = m_pDetail->IsSignatureDetail() ? VARIANT_FALSE : (GetRemotePropertyInt("EMR_Image_Use_Custom_Stamps", 1, 0, GetCurrentUserName(), TRUE) == 1 ? VARIANT_TRUE : VARIANT_FALSE);

			// (j.jones 2010-02-10 14:26) - PLID 37312 - tell the ocx that we control stamp setup
			m_Image->PutApplicationHandlesStampSetup(VARIANT_TRUE);

			// (j.jones 2010-02-18 10:01) - PLID 37423 - tell the ocx whether it is a SmartStamp image
			if(m_pRealDetail->GetSmartStampTableDetail()) {
				m_Image->PutIsSmartStampImage(VARIANT_TRUE);
			}
			else {
				m_Image->PutIsSmartStampImage(VARIANT_FALSE);
			}

			// (j.jones 2010-04-08 09:35) - PLID 38097 - tell the control whether the preference is enabled to fire hotspots when stamping on them,
			// this used to be SmartStamp only but now it applies to all stamps, so disregard the SmartStamp ConfigRT naming
			m_Image->PutStampFireHotspotClick(GetRemotePropertyInt("EMR_AddSmartStampOnHotSpot", 0, 0, GetCurrentUserName(), true) == 1 ? VARIANT_TRUE : VARIANT_FALSE);

			// (j.jones 2010-02-10 11:11) - PLID 37224 - stamps are now tracked globally, just call the
			// EmrUtils function to generate the stamp info that is readable by the ink picture			
			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			m_Image->CustomStamps = m_pDetail->IsSignatureDetail() ? "" : _bstr_t(strStamps);
			//DRT 4/30/2008 - PLID 29771 - Set the background color
			m_Image->BackColor = GetNxColor(GNC_EMR_ITEM_BG, 0);

			// (j.jones 2005-08-08 16:12) - don't load the ConfigRT default color, load the EMR image ink color
			// Assign the default pen color
			// (j.armen 2014-07-23 11:19) - PLID 62836 - Also load the current pen size
			m_Image->DefaultPenColor = m_nCurPenColor;
			m_Image->CurrentPenSize = m_fltCurPenSize;

			// (r.gonet 05/06/2011) - PLID 43542 - Set whether text should scale. Since the popup can't scale, we set this to true so that locked EMNs scaling is preserved.
			// (z.manning 2011-11-21 10:26) - PLID 46558 - This now only checks the preference
			m_Image->EnableTextScaling = GetRemotePropertyInt("EnableEMRImageTextScaling", 1, 0, "<None>", true) == 1 ? VARIANT_TRUE : VARIANT_FALSE;


			// (b.spivey, May 02, 2013) - PLID 56542 
			m_Image->SetTopazSigPadConnected(GetMainFrame()->GetIsTopazConnected());


			//TES 2/26/2008 - PLID 27721 - Setting the focus here ensures that the image gets sized properly if it stretches
			// off the edge of the screen; I'm not sure why, apart from it being something to do with changing the order in
			// which various windows/COleControl messages are handled.
			m_wndImage.SetFocus();
		
			// (j.jones 2005-08-08 16:12) - you CANNOT load the image or the ink here,
			// you must size the control first before doing either action
		}
	}
	else if(m_pDetail->m_EMRInfoType == eitSlider) {
		// (a.wetta 2007-03-09 10:11) - PLID 24757 - Make sure that the slider is a tabstop so that it will have the focus
		// when the pop up first comes up.
		// (a.walling 2007-06-21 13:16) - PLID 22097 - Create disabled if necessary
		m_Slider.Create(WS_BORDER|WS_CHILD|WS_TABSTOP|WS_VISIBLE|dwDisabled|WS_GROUP|TBS_HORZ|TBS_AUTOTICKS|TBS_BOTTOM|TBS_LEFT, CRect(0, 0, 300, 300), this, 5000);

		// (a.walling 2007-06-21 13:16) - PLID 22097 - Create disabled if necessary
		m_Caption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|dwDisabled|SS_CENTER, CRect(0,0,300,300), this);
		//m_Caption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
		m_Caption.SetFont(&theApp.m_boldFont);

		// (a.walling 2007-06-21 13:16) - PLID 22097 - Create disabled if necessary
		m_MinCaption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|dwDisabled|SS_RIGHT, CRect(0,0,300,300), this);
		//m_MinCaption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
		m_MinCaption.SetFont(&theApp.m_notboldFont);
		m_MinCaption.SetWindowText(AsString(m_pDetail->GetSliderMin()));

		// (a.walling 2007-06-21 13:16) - PLID 22097 - Create disabled if necessary
		m_MaxCaption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|dwDisabled|SS_RIGHT, CRect(0,0,300,300), this);
		//m_MaxCaption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
		m_MaxCaption.SetFont(&theApp.m_notboldFont);
		m_MaxCaption.SetWindowText(AsString(m_pDetail->GetSliderMax()));

		//Set the slider's parameters.
		double dScale = 1.0 / m_pDetail->GetSliderInc();
		if(m_Slider.GetSafeHwnd()) {
			/*m_Slider.SetRangeMin((int)(m_pDetail->GetSliderMin()*dScale));
			m_Slider.SetRangeMax((int)(m_pDetail->GetSliderMax()*dScale));
			m_Slider.SetTicFreq(1);*/

			// (a.walling 2007-02-28 12:31) - PLID 24914 - Several issues with slider values. I've simplified the way
			// we store this data and calculate between values and positions. Now the slider control's minimum value is
			// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
			// solving the problem with odd increments allowing values below the minimum (and then all the incremented
			// values being off).

			// I don't know why or how this could happen, but might as well check for it.
			if (m_pDetail->GetSliderInc() == 0) {
				ThrowNxException("CEMRItemAdvPopupWnd::CreateControls(): Slider increment value is zero!");
			}

			m_Slider.SetRangeMin(0);
			m_Slider.SetRangeMax((int)floor((m_pDetail->GetSliderMax() - m_pDetail->GetSliderMin()) / m_pDetail->GetSliderInc()));
			m_Slider.SetTicFreq(1);
		}
	}
	else if(m_pDetail->m_EMRInfoType == eitTable) {
		// (c.haag 2008-10-16 10:13) - PLID 31700 - Ensures the table is valid and has all rows and columns populated
		EnsureTableObject(m_pDetail, this, strLabel, m_bReadOnly);
		// (c.haag 2008-10-16 14:35) - PLID 31700 - Now add fields to the table (columns for non-flipped tables; or rows
		// for flipped tables)
		AddTableFields(m_pDetail, TRUE);
		// (c.haag 2008-10-16 11:01) - PLID 31700 - Add content to the table
		AddTableContent(m_pDetail);

		ReflectTableState();

		// (a.walling 2007-06-21 12:38) - PLID 22097 - Set the readonly flag.
		SetTableReadOnly(m_bReadOnly ? VARIANT_TRUE : VARIANT_FALSE);
	}
	else if(m_pDetail->m_EMRInfoType == eitNarrative) {
		//TES 4/5/2007 - PLID 25456 - Narratives can now be popped up.
		if (!m_wndRichEdit.CreateControl(_T("RICHTEXTEDITOR.RichTextEditorCtrl.1"), "EmrItemAdvNarrativeDlg", WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, 8000)) {
			DWORD dwErr = GetLastError();
			return;
		}
		
		//TES 7/6/2012 - PLID 51424 - If this doesnt have the WS_EX_CONTROLPARENT flag set, there's a risk of MFC going into an infinite loop
		// figuring out which window to activate.
		m_wndRichEdit.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

		m_RichEditCtrl = m_wndRichEdit.GetControlUnknown();

		// (a.walling 2009-11-17 13:58) - PLID 36365
		m_RichEditCtrl->UsePushFieldUpdates = VARIANT_TRUE;

		m_RichEditCtrl->DefaultFlags = "s";
		//DRT 4/30/2008 - PLID 29771 - Set the background color
		m_RichEditCtrl->BackColor = GetNxColor(GNC_EMR_ITEM_BG, 0);

		//Fill in any fields that have may have been supplied to us before our window was created.
		// (c.haag 2007-04-03 17:54) - PLID 25488 - We now use methods rather than direct accesses
		// to the merge fields in the detail object
		// (c.haag 2007-05-10 15:02) - PLID 25958 - The methods have changed such that we now use
		// a connectionless recordset object to do the work

		// (a.walling 2007-06-25 09:45) - PLID 22097 - Added chris' comment below, this should take care
		// of popping up narratives which could make locked EMN topics get marked as modified.
		// (c.haag 2007-05-14 10:00) - PLID 25970 - Now that we share the detail's merge field list
		// with the control, there's no need to manually fill in values anymore

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		//m_RichEditCtrl->MergeFieldSet = m_pDetail->GetNarrativeFieldRecordset();

		/*
		m_RichEditCtrl->BeginAddMergeFields();
		ADODB::_RecordsetPtr prsNarrativeFields = m_pDetail->GetNarrativeFieldRecordset();
		if (prsNarrativeFields->GetRecordCount() > 0) {
			ADODB::FieldsPtr f = prsNarrativeFields->Fields;
			prsNarrativeFields->MoveFirst();
			while (!prsNarrativeFields->eof) {
				CString strProperties = "Linkable;";
				if(AdoFldBool(f, "Linkable")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "IsSubField;";
				if(AdoFldBool(f, "IsSubField")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "IsCaseSensitive;";
				if(AdoFldBool(f, "CaseSensitive")) strProperties += "True;"; else strProperties += "False;";
				
				if(AdoFldBool(f, "ValueIsValid")) {
					m_RichEditCtrl->AddFilledMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(GetNarrativeValueField(prsNarrativeFields)), _bstr_t(strProperties));
				}
				else {
					m_RichEditCtrl->AddAvailableMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(strProperties));
				}
				prsNarrativeFields->MoveNext();
			}
		}

		m_RichEditCtrl->EndAddMergeFields();
		*/

		// (a.walling 2007-06-21 16:22) - PLID 22097 - Set the readonly flag
		m_RichEditCtrl->ReadOnly = m_bReadOnly ? VARIANT_TRUE : VARIANT_FALSE;
		m_wndRichEdit.ShowWindow(SW_SHOW);

		m_RichEditCtrl->RichText = _bstr_t(AsString(m_pDetail->GetState()));
		m_RichEditCtrl->PutAllowFieldInsert(TRUE);

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
		m_pDetail->UpdateNarrativeFields(this);
	}
	
	CallRepositionControls();
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionTextControls(IN OUT CSize &szArea)
{
	//TES 2/21/2008 - PLID 28827 - If we have an "ideal" size, then we will try to default to that, rather than to
	// our current size (which is what szArea is).
	if(m_szIdeal != CSize(0,0)) {
		szArea = m_szIdeal;
	}

	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnMinEditWidth = 80;
	const long cnMinEditHeight = 25;

	CClientDC dc(this);

	long nTopMargin = 3;
	long nLabelRight = 0;
	long nIdealWidth = 320;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);

		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;
		if (nIdealWidth < sz.cx) {
			nIdealWidth = sz.cx;
		}

		nLabelRight = 6 + sz.cx;

		{
			CRect rPrior, rLabel;
			GetControlChildRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlChildRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}
	}

	// (z.manning 2008-12-16 14:06) - PLID 27682 - Copied the logic for positioning the spell check
	// button from EmrItemAdvTextDlg
	CRect rcSpellCheckButton;
	if (m_btnSpellCheck.m_hWnd) {
		// Get the size the button needs to be 
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_btnSpellCheck, sz);

		// Now determine the left based on the right of the label, and the top based on the top 
		// of the edit box (i.e. the bottom of the label) minus the height of the button
		// allows (i.e. never < nTopMargin).  If the right side of the button would go off the 
		// right side of szArea, then put the button below the label instead.
		CPoint ptTopLeft;
		if (nLabelRight + 3 + sz.cx <= szArea.cx) {
			ptTopLeft.x = nLabelRight + 3;
			// Make sure there's vertical room for it by adjusting our overall top margin.
			if (nTopMargin < sz.cy + 1) {
				nTopMargin = sz.cy + 1;
			}
			ptTopLeft.y = nTopMargin - sz.cy;
		} else {
			ptTopLeft.x = 6;
			ptTopLeft.y = nTopMargin;
			// Since it's being placed BELOW the label, we have to increase or overall top 
			// margin to fit it.
			nTopMargin += sz.cy + 1;
		}

		// Regardless of what we actually decided our position would be, IDEALLY we would want 
		// to be on the right side of the label, so indicate that in our nIdealWidth check
		if (nIdealWidth < nLabelRight + 3 + sz.cx) {
			nIdealWidth = nLabelRight + 3 + sz.cx;
		}

		// Remember the position and dimentions of the button
		rcSpellCheckButton = CRect(ptTopLeft, sz);
		
		// Make sure our ideal width includes the button
		if (nIdealWidth < rcSpellCheckButton.right) {
			nIdealWidth = rcSpellCheckButton.right;
		}
		
		// And finally move the button to the appropriate spot (if we're not in calc-only mode)
		{
			m_btnSpellCheck.MoveWindow(rcSpellCheckButton, TRUE);
		}

		if(IsWindow(m_btnOpenLab.GetSafeHwnd()))
		{
			// (z.manning 2009-09-21) - PLID 33612 - Move the open lab button if we have one
			CRect rcLabButton(rcSpellCheckButton);
			rcLabButton.left = rcSpellCheckButton.right + 2;
			rcLabButton.right = rcLabButton.left + 60;
			{
				m_btnOpenLab.MoveWindow(rcLabButton, TRUE);
			}
		}
	}

	long nIdealHeight = 150;
	
	// Make sure we're not smaller than the minimum
	long nMinWidth = 20 + cnMinEditWidth;
	long nMinHeight = nTopMargin + 10;
	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (m_edit.m_hWnd) {
		m_edit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - 10);
	}

	BOOL bAns;
	if (szArea.cx >= nIdealWidth && szArea.cy >= nIdealHeight) {
		// Our ideal fits within the given area
		bAns = TRUE;
	} else {
		// Our ideal was too big for the given area
		bAns = FALSE;
	}

	// Return the new area, but adjust back on the border size since the caller wants window area
	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bAns;
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionListControls(IN OUT CSize &szArea)
{
	if (m_arypControls.GetSize() > 0) {
		// The caller is giving us a full window area so we have to adjust off the 
		// border to get what will be our client area.
		CSize szBorder;
		CalcWindowBorderSize(this, &szBorder);

		// Adjust off the border
		szArea.cx -= szBorder.cx;
		szArea.cy -= szBorder.cy;

		// Adjust off the scroll bar
		// (a.walling 2010-06-21 14:56) - PLID 38779 - Always assume scroll bars are visible
		//if (IsScrollBarVisible(this->m_hWnd, SB_HORZ))
			szArea.cy -= (GetSystemMetrics(SM_CYHSCROLL) + 5);
		//if (IsScrollBarVisible(this->m_hWnd, SB_VERT))
			szArea.cx -= (GetSystemMetrics(SM_CXVSCROLL) + 5);

		//TES 2/26/2008 - PLID 28827 - We will also need to adjust for the scroll POSITION.
		int nXScroll = 0, nYScroll = 0;	
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE;
		if(GetScrollInfo(SB_VERT, &si, si.fMask)) {
			nYScroll = si.nPos - si.nMin;
		}
		if(GetScrollInfo(SB_HORZ, &si, si.fMask)) {
			nXScroll = si.nPos - si.nMin;
		}
		CSize szScrollOffset(-1*nXScroll, -1*nYScroll);

		CClientDC dc(this);

		long nTopMargin = 3;
		long nBotMargin = 3;
		long nMinNecessaryWidth = 200;
		CRect rLabel;
		if (IsControlValid(&m_wndLabel)) {
			CSize sz(LONG_MAX, LONG_MAX);
			CalcControlIdealDimensions(&dc, &m_wndLabel, sz);			

			//first see if the label is too wide to be displayed
			if(sz.cx > szArea.cx) {

				//if so, find out how many lines we must create
				int nHeight = 1;
				while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
					nHeight++;
				}
				
				//now increase the height of the item to match
				if((sz.cx / (nHeight - 1)) > szArea.cx) {
					sz.cx = szArea.cx;
					sz.cy *= nHeight;
				}
			}

			nTopMargin += sz.cy;

			if (nMinNecessaryWidth < sz.cx) {
				nMinNecessaryWidth = sz.cx;
			}
			{
				//TES 10/27/2004  - We won't actually move the label yet, because the position of the controls may 
				//cause us to move the label.
				rLabel = CRect(11, 2, 11 + sz.cx, 2 + sz.cy);
				//TES 2/26/2008 - PLID 28827 - Adjust for the scroll position
				rLabel.OffsetRect(szScrollOffset);
			}
		}

		//TES 2/26/2008 - PLID 28827 - Adjust for the scroll position
		long nCurTop = nTopMargin - nYScroll;
		long nCurLeft = 8 - nXScroll;
		long nMaxWidth = 0;
		long nMaxBottom = nCurTop;
		BOOL bAns = TRUE;

		//DRT 8/13/2007 - PLID 27004 - Set a minimum and maximum width for each list element.
		const long MAX_WIDTH = (long)(m_szMax.cx / 3);
		const long MIN_WIDTH = 15;

		for (long i=0; i<m_arypControls.GetSize(); i++) {
			CWnd *pWnd = ((CWnd *)m_arypControls.GetAt(i));
			if (IsControlValid(pWnd)) {
				// Calc the dimensions of the control
				CSize sz(LONG_MAX, LONG_MAX);
				CalcControlIdealDimensions(&dc, pWnd, sz);
				// Decide the rect based on the current location, with the control's ideal dimensions
				CRect rc(CPoint(nCurLeft, nCurTop), sz);

				BOOL bChanged = TRUE;

				//we must do this in a loop because we may increase the height of an object,
				//which may increase the column count, which then may need to increase the height again, etc.
				while(bChanged) {

					bChanged = FALSE;

					{
						//DRT 8/13/2007 - PLID 27004 - This no longer applies.  Previously we tried to do some crazy stuff involving staying
						//	on the screen, but it never worked well.  You could end up with the 3rd column shrunk to fit on screen, but then
						//	have a 4th column off to the right of it anyways!  Or worse yet, the 3rd column might shrink to just a few pixels
						//	if you hit it just right (and we did on client data), so that you can't read any of the labels.  So this whole
						//	thing is now dropped in place of the above min width / max width code below
						/*
						//first see if the field is too wide to be displayed
						if(pWnd->GetDlgCtrlID() < 2000 && rc.left < szArea.cx &&
							rc.right > szArea.cx + 6 + GetSystemMetrics(SM_CXMENUCHECK)) {

							CFont *pFont = pWnd->GetFont();
							long nFontHeightBuffer = GetControlBufferWidth();
							
							//recalculate the ideal dimensions, knowing our width is limited
							sz.cx = szArea.cx - rc.left - 6 - GetSystemMetrics(SM_CXMENUCHECK) - (nFontHeightBuffer * 4);
							sz.cy = LONG_MAX;
							CalcControlIdealDimensions(&dc, pWnd, sz, TRUE);
							long nIdealHeight = sz.cy;
							
							//now resize the control appropriately
							rc.right = rc.left + sz.cx;
							rc.bottom = rc.top + nIdealHeight;
							bChanged = TRUE;
						}*/

						//If it's under the minimum, just increase the size.
						if(rc.Width() < MIN_WIDTH) {
							rc.right = rc.left + MIN_WIDTH;
							bChanged = TRUE;
						}

						//If the width is exceeded, we will increase the height instead.  All the other stuff there (SM_CXMENU..., font buffer, etc) are 
						//	to take into account the extra calculations that CalcControlIdealDimensions does.  This is basically the same code that used to check
						//	to see if we were going off the right side of the screen.
						if(rc.Width() > MAX_WIDTH) {
							long nFontHeightBuffer = GetControlBufferWidth();

							//recalculate the ideal dimensions, knowing our width is limited
							sz.cx = MAX_WIDTH;
							sz.cy = LONG_MAX;
							CalcControlIdealDimensions(&dc, pWnd, sz, TRUE);
							long nIdealHeight = sz.cy;

							//now resize the control appropriately
							rc.right = rc.left + sz.cx;
							rc.bottom = rc.top + nIdealHeight;
							bChanged = TRUE;
						}


						// If that would put the control off the bottom, then move it to the next column
						//TES 2/7/2008 - PLID 28778 - Don't move it to the next column if it's already at the top of
						// this column.  If that is the case, that means that this control is taller by itself then
						// our area, so there's no way we can fit it, and we'll end up in an infinite loop.  So, just 
						// leave it in its own column and move on.
						//TES 2/26/2008 - PLID 28827 - Adjusted these various comparisons to account for the fact that 
						// we may be scrolled down
						if (rc.bottom + nYScroll > szArea.cy - nBotMargin && nMaxWidth > 0 && rc.top != nTopMargin - nYScroll) {
							rc.OffsetRect(nMaxWidth + 8, nTopMargin - rc.top - nYScroll);
							nMaxWidth = 0;
							bAns = FALSE;
							bChanged = TRUE;
						}
						else if(rc.left < szArea.cx && rc.bottom + nYScroll > szArea.cy - nBotMargin && i == 0 && rc.top != nTopMargin - nYScroll) {
							//The very first one goes off the bottom. Let's start a column (and all ensuing columns) 
							//to the right of the label.
							//We also need to offset the label a little bit, to look good.
							rLabel.OffsetRect(0,3);
							
							nTopMargin = 3;
							rc.OffsetRect(nMinNecessaryWidth + 8, nTopMargin - rc.top - nYScroll);
							nMaxWidth = 0;
							bAns = FALSE;
							bChanged = TRUE;
						}
					}

					// Unless we've been asked to just calculate, we want to also move the control.
					{
						// Move it to that spot
						// (b.cardillo 2004-07-07 12:10) - PLID 13344 - Do nothing if the rect hasn't changed.
						CRect rcPrior;

						GetControlChildRect(this, pWnd, &rcPrior);

						if (!rcPrior.EqualRect(rc)) {
							// Move and don't repaint (we don't want to paint until later, but we do need to invalidate 
							// so we do so immediately after moving)
							pWnd->MoveWindow(rc, TRUE);
							// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
							// children, invalidating the child area is not good enough, so we 
							// have to actually tell each child to invalidate itself.  Also, we 
							// used to ask it to erase too, because according to an old comment 
							// related to plid 13344, the "SetRedraw" state might be false at 
							// the moment.  (I'm deleting that comment on this check-in, so to 
							// see it you'll have to look in source history.)  But I think the 
							// "SetRedraw" state no longer can be false, and even if it were I 
							// think now that we're invalidating the control itself, we wouldn't 
							// need to erase here anyway.
							//pWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE);
						}
					}
					// Set our variables so we know where to put the next control
					if (rc.bottom > nMaxBottom) {
						nMaxBottom = rc.bottom;
					}
					nCurTop = rc.bottom + 5;
					nCurLeft = rc.left;

					long nWidth = rc.Width();
					if (nWidth > nMaxWidth) {
						nMaxWidth = nWidth;
					}
				}
			}
		}

		//Now, put the label wherever we decided to put it.
		{
			CRect rPrior;
			GetControlChildRect(this, &m_wndLabel, &rPrior);
			if(!rPrior.EqualRect(rLabel)) {
				m_wndLabel.MoveWindow(rLabel, FALSE);
				rPrior.InflateRect(2,2,2,2);
				rLabel.InflateRect(2,2,2,2);
				InvalidateRect(rPrior, TRUE);
				InvalidateRect(rLabel, TRUE);				
			}
		}
		
		// Store the ideal size
		//TES 2/26/2008 - PLID 28827 - Adjust for the scroll position
		CSize szIdeal(nCurLeft + nMaxWidth + 8 + nXScroll, nMaxBottom + nBotMargin + nYScroll);
		if (szIdeal.cx < nMinNecessaryWidth) {
			szIdeal.cx = nMinNecessaryWidth;
		} else {
			// We're still within the label width so we might as well not complain about 
			// being imperfectly shaped
			bAns = TRUE;
		}
		
		// Return the given rect reflecting the new right and bottom sides
		// Adjust back on the border size since the caller wants window area
		szArea.cx = szIdeal.cx + szBorder.cx;
		szArea.cy = szIdeal.cy + szBorder.cy;

		return bAns;
	} else {
		szArea.cx = 0;
		szArea.cy = 0;
		return TRUE;
	}
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionImageControls(IN OUT CSize &szArea)
{
	// (a.walling 2011-05-19 18:07) - PLID 43843 - This is absolute insanity, and eventually
	// I ended up having to rewrite most of it. Oddly enough I predicted I would be doing so exactly
	// one year before this point.

	const CPoint ptOffset(6, 2);

	//TES 2/26/2008 - PLID 28827 - We will also need to adjust for the scroll POSITION.
	int nXScroll = 0, nYScroll = 0;	
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS | SIF_RANGE;
	if(GetScrollInfo(SB_VERT, &si, si.fMask)) {
		nYScroll = si.nPos - si.nMin;
	}
	if(GetScrollInfo(SB_HORZ, &si, si.fMask)) {
		nXScroll = si.nPos - si.nMin;
	}
	CSize szScrollOffset(-1*nXScroll, -1*nYScroll);

	// the border of the window
	CSize szBorder;
	// The label at the top left
	CSize szLabel(LONG_MAX, LONG_MAX);
	// the minimum content size
	CSize szMinimumContent;
	// The size of the content (the ink image control)
	CSize szContent;
	// the available client area (szArea - szBorder)
	CSize szClient;
	// the best fit in szClient
	CSize szBestFit;
	
	// Adjust off the scroll bar
	// (a.walling 2010-06-21 14:56) - PLID 38779 - Always assume scroll bars are visible
	CSize szScrollBars(GetSystemMetrics(SM_CYHSCROLL) + 5, GetSystemMetrics(SM_CXVSCROLL) + 5);

	szArea.cx -= szScrollBars.cx;
	szArea.cy -= szScrollBars.cy;

	// Get the border size
	CalcWindowBorderSize(this, &szBorder);

	// now get the minimum possible size
	szMinimumContent.SetSize(0, 0);
	if(m_Image != NULL) {
		m_Image->GetMinimumDimensions((long*)&szMinimumContent.cx, (long*)&szMinimumContent.cy);
	}
	else if(m_p3DControl != NULL) {
		m_p3DControl->GetMinimumDimensions(&szMinimumContent.cx, &szMinimumContent.cy);
	}
	
	// Update the client area minus the border
	szClient.SetSize(szArea.cx - szBorder.cx, szArea.cy - szBorder.cy);

	// ensure szClient is at least that minimum
	szClient.SetSize(max(szClient.cx, szMinimumContent.cx), max(szClient.cy, szMinimumContent.cy));

	CClientDC dc(this);

	// now that we have the provisional width so figure out the size of the label
	if (!IsControlValid(&m_wndLabel)) {
		szLabel.SetSize(0, 0);
	} else {
		CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel);

		//check if the label is too wide to be displayed
		if(szLabel.cx > szClient.cx) {
			szLabel.cx = szClient.cx;
			szLabel.cy = 0;
			CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
		}
	}		
	
	szBestFit.SetSize(0, 0);
	if(m_Image != NULL) {
		m_Image->GetIdealDimensions((long*)&szBestFit.cx, (long*)&szBestFit.cy, VARIANT_TRUE);
	}
	else if(m_p3DControl != NULL) {
		m_p3DControl->GetIdealDimensions(&szBestFit.cx, &szBestFit.cy, TRUE);
	}

	szBestFit.SetSize(max(szBestFit.cx, szMinimumContent.cx), max(szBestFit.cy, szMinimumContent.cy));
	
	if (!m_bShrinkFitToArea) {
		// bCalcOnly means get the best fit for the item.
		szContent = szBestFit;

		// of course we may need to adjust the label size again
		if (szContent.cx < szLabel.cx && m_wndLabel.GetSafeHwnd()) {		
			szLabel.cx = szContent.cx;
			szLabel.cy = 0;
			CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
		}
	} else {			
		// Otherwise we simply squeeze into what we are given!
		
		// Update the client area minus the offset
		szClient.cx -= ptOffset.x;
		szClient.cy -= ptOffset.y;
		
		// figure out our available size
		// the size available within szArea for the content
		szContent = szClient;
		szContent.cy -= szLabel.cy;

		szContent.SetSize(max(szContent.cx, 0), max(szContent.cy, 0));

		if(m_Image != NULL) {
			m_Image->GetIdealDimensionsForControl((long*)&szContent.cx, (long*)&szContent.cy);
		}
		else if(m_p3DControl != NULL) {
			m_p3DControl->GetIdealDimensionsForControl(&szContent.cx, &szContent.cy);
		}

		szContent.SetSize(min(szBestFit.cx, szContent.cx), min(szBestFit.cy, szContent.cy));
	}

	// now we have the various sizes we require.
	CPoint currentOffset = ptOffset;
	CRect rcLabel(currentOffset, szLabel);
	currentOffset.y += rcLabel.Height();

	CRect rcContent(currentOffset, szContent);
	currentOffset.y += rcContent.Height();

	CRect rcClient(0, 0, szContent.cx, currentOffset.y);

	// add the borders and padding in to get the final area
	CSize szIdeal(
		rcClient.Width() + (szBorder.cx * 2) + ptOffset.x + szScrollBars.cx,
		rcClient.Height() + (szBorder.cy * 2) + ptOffset.y + szScrollBars.cy
	);

	CSize szMinSize(
		szMinimumContent.cx  + (szBorder.cx * 2) + ptOffset.x + szScrollBars.cx,
		szMinimumContent.cy + (szBorder.cy * 2) + ptOffset.y + szScrollBars.cy
	);

	// update positions
	{
		//TES 2/26/2008 - PLID 28827 - Adjust for the scroll position
		rcLabel.OffsetRect(szScrollOffset);
		rcContent.OffsetRect(szScrollOffset);

		// (a.walling 2011-05-19 18:07) - PLID 43843 - EXACTLY ONE YEAR LATER I will make this thing sane!
		m_wndLabel.MoveWindow(&rcLabel, FALSE);
		m_wndImage.MoveWindow(&rcContent, FALSE);

		// (a.walling 2010-05-19 11:15) - PLID 38778 - For whatever reason, a portion of the stamp buttons are being cut off. For now just fudge the width.
		// FYI this code is absolutely insane. There is a better solution to this, of course, but that would require destabilizing even more parts of this
		// code, so I am leaving that for another day. The issue is that the scrollbar is added after the size calculation.
		// (a.walling 2010-06-21 15:10) - PLID 38779
		//rNewImageRect.right -= 15;

		//TES 2/26/2008 - PLID 27721 - Setting the focus here ensures that the image gets sized properly if it stretches
		// off the edge of the screen; I'm not sure why, apart from it being something to do with changing the order in
		// which various windows/COleControl messages are handled.
		m_wndImage.SetFocus();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
	}
	
	// Determine our return value
	BOOL bAns;
	if (szArea.cx >= szIdeal.cx && szArea.cy >= szIdeal.cy) {
		bAns = TRUE;
	} else {
		bAns = FALSE;
	}

	if (!m_bShrinkFitToArea) {
		szArea = szIdeal;
	} else {
		szArea.SetSize(min(szArea.cx, szIdeal.cx), min(szArea.cy, szIdeal.cy));
	}
	
	szArea.SetSize(max(szMinSize.cx, szArea.cx), max(szMinSize.cy, szArea.cy));

	BringWindowToTop();

	//Reflect the state.
	// (a.walling 2008-01-18 14:05) - PLID 14982 - Call the shared function here
	ReflectImageState();

	return bAns;

	return FALSE;
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionSliderControls(IN OUT CSize &szArea)
{
	//TES 2/21/2008 - PLID 28827 - If we have an "ideal" size, then we will try to default to that, rather than to
	// our current size (which is what szArea is).
	if(m_szIdeal != CSize(0,0)) {
		szArea = m_szIdeal;
	}
	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CSize szMergeBtn;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnSliderSize = 80;

	CClientDC dc(this);

	long nTopMargin = 3;
	long nIdealWidth = 320;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);
		
		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;

		if (nIdealWidth < sz.cx) {
			nIdealWidth = sz.cx;
		}
		{
			CRect rPrior, rLabel;
			GetControlChildRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlChildRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}
	}

	long nIdealHeight = 150;



	{
		if (szArea.cx > nIdealWidth) {
			szArea.cx = nIdealWidth;
		}
		if (szArea.cy > nIdealHeight) {
			szArea.cy = nIdealHeight;
		}
	}

	//Now, are we horizontal or vertical?
	bool bVert = szArea.cy > szArea.cx;
	
	//I'm just guessing at these for now.
	long nMinSliderWidth = bVert ? 20 : 50;
	long nMinSliderHeight = bVert ? 50 : 20;
	long nMinCaptionWidth = 50;
	long nMinCaptionHeight = 20;

	// Make sure we're not smaller than the minimum
	long nMinWidth = nMinSliderWidth + 20 + (bVert?nMinCaptionWidth:0);
	long nMinHeight = nTopMargin + nMinSliderHeight + 10 + (bVert?0:nMinCaptionHeight);

	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (m_Slider.m_hWnd) {
		if(bVert) {
			//Slider: 30 pixels wide, centered in the window, 
			//height = from (top of area + top margin) to (bottom of the area - the merge button margin - the size of the merge button - 10)
			if(!(m_Slider.GetStyle() & TBS_VERT)) {
				m_Slider.ModifyStyle(TBS_HORZ, TBS_VERT,0);
				m_MinCaption.ModifyStyle(SS_LEFT, SS_RIGHT, 0);
				m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
				m_Caption.SetWindowText(AsString(m_pDetail->GetState()));
			}
			CRect rSlider((szArea.cx/2) - 15, nTopMargin, (szArea.cx/2)+15, szArea.cy - nTopMargin - 10);
			m_Slider.MoveWindow(&rSlider);
			//Caption.  20 pixels high, centered vertically.
			//From 10 pixels right of slider to end of window.
			CRect rCaption((szArea.cx/2) + 25, (szArea.cy/2)-10, szArea.cx, (szArea.cy/2)+10);
			m_Caption.MoveWindow(&rCaption);
			m_Caption.RedrawWindow();
			//Minimum label, 20 pixels high, bottom-aligned with slider, from left of screen to 5 pixels right of slider.
			CRect rMinCaption(0, szArea.cy - nTopMargin - 15 - 20, (szArea.cx/2)-20,
				szArea.cy - nTopMargin - 15);
			m_MinCaption.MoveWindow(&rMinCaption);
			m_MinCaption.RedrawWindow();
			//Maximum label, 20 pixels high, top-aligned with slider, from left of screen to 5 pixels right of slider.
			CRect rMaxCaption(0, nTopMargin, (szArea.cx/2)-20, nTopMargin + 20);
			m_MaxCaption.MoveWindow(&rMaxCaption);
			m_MaxCaption.RedrawWindow();
		}
		else {
			//Slider: 30 pixels high, centered vertically, width of window - 30 pixels on each side.
			if(!(m_Slider.GetStyle() & TBS_HORZ)) {
				m_Slider.ModifyStyle(TBS_VERT, TBS_HORZ,0);
				m_MinCaption.ModifyStyle(SS_RIGHT, SS_LEFT, 0);
				m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
				m_Caption.SetWindowText(AsString(m_pDetail->GetState()));
			}
			CRect rSlider(10, (szArea.cy/2)-15, szArea.cx-10, (szArea.cy/2)+15);
			m_Slider.MoveWindow(&rSlider);
			
			//Caption, 20 pixels high, 20 pixels below slider, width of window - 10 pixels on each side.
			CRect rCaption(10, (szArea.cy/2)+35, szArea.cx-10, (szArea.cy/2)+55);
			m_Caption.MoveWindow(&rCaption);
			m_Caption.RedrawWindow();
			
			//Minimum label,20 pixels high, just above slider, left half of screen (-10 pixel margin).
			CRect rMinCaption(10, (szArea.cy/2)-35, szArea.cx/2, (szArea.cy/2)-15);
			m_MinCaption.MoveWindow(&rMinCaption);
			m_MinCaption.RedrawWindow();
			//Minimum label,20 pixels high, just above slider, right half of screen (-10 pixel margin).
			CRect rMaxCaption(szArea.cx/2, (szArea.cy/2)-35, szArea.cx - 10, (szArea.cy/2)-15);
			m_MaxCaption.MoveWindow(&rMaxCaption);
			m_MaxCaption.RedrawWindow();
		}
	}

	BOOL bAns;
	if (szArea.cx >= nIdealWidth && szArea.cy >= nIdealHeight) {
		// Our ideal fits within the given area
		bAns = TRUE;
	} else {
		// Our ideal was too big for the given area
		bAns = FALSE;
	}

	// Return the new area, but adjust back on the border size since the caller wants window area
	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bAns;
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionTableControls(IN OUT CSize &szArea)
{
	if (IsTableControlValid()) {

		SetTableRedraw(FALSE);

		// The caller is giving us a full window area so we have to adjust off the 
		// border to get what will be our client area.
		CSize szBorder;
		CalcWindowBorderSize(this, &szBorder);
		// Adjust off the border
		szArea.cx -= szBorder.cx;
		szArea.cy -= szBorder.cy;

		CClientDC dc(this);

		long nTopMargin = 3;
		long nBotMargin = 3;
		long nMinNecessaryWidth = 0;
		CRect rLabel;
		if (IsControlValid(&m_wndLabel)) {
			CSize sz(LONG_MAX, LONG_MAX);
			CalcControlIdealDimensions(&dc, &m_wndLabel, sz);			

			//first see if the label is too wide to be displayed
			if(sz.cx > szArea.cx) {

				//if so, find out how many lines we must create
				int nHeight = 1;
				while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
					nHeight++;
				}
				
				//now increase the height of the item to match
				if((sz.cx / (nHeight - 1)) > szArea.cx) {
					sz.cx = szArea.cx;
					sz.cy *= nHeight;
				}
			}

			nTopMargin += sz.cy;

			if (nMinNecessaryWidth < sz.cx) {
				nMinNecessaryWidth = sz.cx;
			}
			{
				//TES 10/27/2004  - We won't actually move the label yet, because the position of the controls may 
				//cause us to move the label.
				rLabel = CRect(6, 2, 6+sz.cx, 2+sz.cy);
			}
		} // if (m_wndLabel.m_hWnd) {

		long nCurTop = nTopMargin;
		long nCurLeft = 10;
		long nMaxWidth = 0;
		long nMaxBottom = nCurTop;
		BOOL bAns = TRUE;
		
		// (c.haag 2008-10-21 12:37) - PLID 31700 - We already checked to see if m_Table was valid up above; don't
		// bother to check a second time. None of this code was actually changed short of being indented
		// Calc the dimensions of the control
		//if (m_Table) {
		CSize sz(LONG_MAX, LONG_MAX);
		
		CalculateTableSize(m_pDetail, sz);

		// Decide the rect based on the current location, with the control's ideal dimensions
		CRect rc(CPoint(nCurLeft, nCurTop), sz);

		if (nMinNecessaryWidth < sz.cx) {
			nMinNecessaryWidth = sz.cx + 20;
		}

		{

			//if the list is too wide, shrink it
			if(rc.Width() > szArea.cx - 20) {
				rc.right = rc.left + szArea.cx - 20;
			}

			//if the list is too tall, shrink it
			if(rc.Height() > szArea.cy - 3 - rLabel.Height()) {
				rc.bottom = rc.top + szArea.cy - 3 - rLabel.Height();
			}
		}

		//(e.lally 2012-03-30) PLID 49319 - extend the table to the min size if we are under those dimensions
		if(rc.Width() < m_szMin.cx){
			rc.right += (m_szMin.cx - rc.Width());
		}
		if(rc.Height() < m_szMin.cy){
			rc.bottom += (m_szMin.cy - rc.Height());
		}

		// (c.haag 2011-03-18) - PLID 42969 - Prior to the 9800 release, rc would the variable that
		// defines the table rectangle. Now we store this in m_rcTableAndCommonButtonsArea and
		// size the table control down to make room for the buttons.
		m_rcTableAndCommonButtonsArea = rc;
		// Now adjust the table control rectangle to make room for the buttons. If the button region
		// exceeds m_rcTableAndCommonButtonsArea, then szCommonButtons will be CSize(0,0) and 
		// the buttons will not display.
		CSize szCommonButtons = CalculateCommonButtonRegionSize();
		rc.top += szCommonButtons.cy;

		// Unless we've been asked to just calculate, we want to also move the control.
		{
			// Move it to that spot
			// (b.cardillo 2004-07-07 12:10) - PLID 13344 - Do nothing if the rect hasn't changed.
			CRect rcPrior;
			m_wndTable.GetWindowRect(&rcPrior);
			ScreenToClient(&rcPrior);
			if (!rcPrior.EqualRect(rc)) {
				// Move and don't repaint (we don't want to paint until later, but we do need to invalidate 
				// so we do so immediately after moving)
				m_wndTable.MoveWindow(rc, FALSE);
				// (c.haag 2011-03-18) - PLID 42969 - Move the common list buttons as well
				ResizeCommonListButtons();
				// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
				// children, invalidating the child area is not good enough, so we 
				// have to actually tell each child to invalidate itself.  Also, we 
				// used to ask it to erase too, because according to an old comment 
				// related to plid 13344, the "SetRedraw" state might be false at 
				// the moment.  (I'm deleting that comment on this check-in, so to 
				// see it you'll have to look in source history.)  But I think the 
				// "SetRedraw" state no longer can be false, and even if it were I 
				// think now that we're invalidating the control itself, we wouldn't 
				// need to erase here anyway.
				m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
				// (c.haag 2011-03-18) - PLID 42969 - Redraw the buttons
				for (int i=0; i < m_apCommonListButtons.GetSize(); i++) 
				{
					CNxIconButton* pBtn = m_apCommonListButtons[i];
					pBtn->RedrawWindow(NULL, NULL, RDW_INVALIDATE);
				}

				// (a.walling 2011-04-08) - PLID 42962 - Invalidate the difference between the two rects
				CRgn rgnDiff, rgnNew;
				rgnDiff.CreateRectRgnIndirect(rcPrior);
				rgnNew.CreateRectRgnIndirect(rc);
				if (1 < rgnDiff.CombineRgn(&rgnDiff, &rgnNew, RGN_DIFF)) {
					InvalidateRgn(&rgnDiff, TRUE);
				} else {
					InvalidateRect(rcPrior, TRUE);
					InvalidateRect(rc, TRUE);
				}
			}
		}

		nMaxBottom = rc.bottom;
		nCurLeft = rc.left;

		long nWidth = rc.Width();
		if (nWidth > nMaxWidth) {
			nMaxWidth = nWidth;
		}

		//Now, put the label wherever we decided to put it.
		{
			CRect rPrior;
			GetControlChildRect(this, &m_wndLabel, &rPrior);
			if(!rPrior.EqualRect(rLabel)) {
				m_wndLabel.MoveWindow(rLabel, FALSE);
				rPrior.InflateRect(2,2,2,2);
				rLabel.InflateRect(2,2,2,2);
				InvalidateRect(rPrior, TRUE);
				InvalidateRect(rLabel, TRUE);

				// (a.walling 2008-01-18 13:38) - PLID 14982
				//m_wndLabel.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE);				
			}
		}
		
		// Store the ideal size
		CSize szIdeal(nCurLeft + nMaxWidth + 8, nMaxBottom + nBotMargin);
		if (szIdeal.cx < nMinNecessaryWidth) {
			szIdeal.cx = nMinNecessaryWidth;
		} else {
			// We're still within the label width so we might as well not complain about 
			// being imperfectly shaped
			bAns = TRUE;
		}
		
		// Return the given rect reflecting the new right and bottom sides
		// Adjust back on the border size since the caller wants window area
		szArea.cx = szIdeal.cx + szBorder.cx;
		szArea.cy = szIdeal.cy + szBorder.cy;

		SetTableRedraw(TRUE);

		return bAns;
	} // if (m_Table) {

	return FALSE;
}

// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEMRItemAdvPopupWnd::RepositionNarrativeControls(IN OUT CSize &szArea)
{
	//TES 2/21/2008 - PLID 28827 - If we have an "ideal" size, then we will try to default to that, rather than to
	// our current size (which is what szArea is).
	if(m_szIdeal != CSize(0,0)) {
		szArea = m_szIdeal;
	}
	
	//TES 4/6/2007 - PLID 25456 - Narratives can be popped up now, code ported from CEMRItemAdvNarrativeDlg.

	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnMinEditWidth = 80;
	const long cnMinEditHeight = 25;

	CClientDC dc(this);

	long nTopMargin = 3;
	long nIdealWidth = 668;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);

		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;
		if (nIdealWidth < sz.cx) {
			nIdealWidth = sz.cx;
		}
		{
			CRect rPrior, rLabel;
			GetControlChildRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlChildRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);

			// (a.walling 2008-01-18 13:38) - PLID 14982
			//m_wndLabel.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
		}
	}

	long nIdealHeight = 325;

	// Make sure we're not smaller than the minimum
	long nMinWidth = 20 + cnMinEditWidth;
	long nMinHeight = nTopMargin + cnMinEditHeight + 10;

	//TES 2/21/2008 - PLID 28827 - We no longer take a minimum size.
	//TES 1/18/2008 - PLID 24157 - Also make sure we're not smaller than the minimum that our owner specified.
	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	if(nMinWidth < m_szMin.cx) {
		nMinWidth = m_szMin.cx;
	}
	if(nMinHeight < m_szMin.cy) {
		nMinHeight = m_szMin.cy;
	}

	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (m_wndRichEdit.m_hWnd) {
		m_wndRichEdit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - 10);
	}

	BOOL bAns;
	if (szArea.cx >= nIdealWidth && szArea.cy >= nIdealHeight) {
		// Our ideal fits within the given area
		bAns = TRUE;
	} else {
		// Our ideal was too big for the given area
		bAns = FALSE;
	}

	// Return the new area, but adjust back on the border size since the caller wants window area
	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bAns;
}

void CEMRItemAdvPopupWnd::CallRepositionControls()
{
	try {

		CWaitCursor pWait;

		BOOL bReposition = TRUE;

		CSize szMax = m_szMax;

		long nPasses = 0;

		while(bReposition && nPasses < 2) {

			nPasses++;

			bReposition = FALSE;
			
			// (a.walling 2010-06-21 15:07) - PLID 38779 - These are never called with not bCalcOnly
			CSize szNew = szMax;
			if(m_pDetail->m_EMRInfoType == eitText)
				RepositionTextControls(szNew);
			else if(m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList)
				RepositionListControls(szNew);
			else if(m_pDetail->m_EMRInfoType == eitImage)
				RepositionImageControls(szNew);
			else if(m_pDetail->m_EMRInfoType == eitSlider)
				RepositionSliderControls(szNew);
			else if(m_pDetail->m_EMRInfoType == eitTable)
				RepositionTableControls(szNew);
			else if(m_pDetail->m_EMRInfoType == eitNarrative)
				RepositionNarrativeControls(szNew);

			// If the scroll bars are already there, add the buffer space to the area for them
			// (a.walling 2010-06-21 14:56) - PLID 38779 - Always assume scroll bars are visible
			//if (IsScrollBarVisible(this->m_hWnd, SB_HORZ))
				szNew.cy += (GetSystemMetrics(SM_CYHSCROLL) + 5);
			//if (IsScrollBarVisible(this->m_hWnd, SB_VERT))
				szNew.cx += (GetSystemMetrics(SM_CXVSCROLL) + 5);

			// Add the scrolls bars if necessary
			// (a.walling 2010-05-19 11:35) - PLID 38778 - Initialize these variables
			long lVertSize = 0, lHorzSize = 0;
			if(szNew.cx > szMax.cx) {
				if (m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList || m_pDetail->m_EMRInfoType == eitImage) {
					ModifyStyle(0, WS_HSCROLL);
					lHorzSize = szNew.cx;
				}
				bReposition = TRUE;
				szNew.cx = szMax.cx;
			}
			if(szNew.cy > szMax.cy) {	
				if (m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList || m_pDetail->m_EMRInfoType == eitImage) {
					ModifyStyle(0, WS_VSCROLL);
					lVertSize = szNew.cy;
				}
				bReposition = TRUE;
				szNew.cy = szMax.cy;
			}	

			//TES 2/21/2008 - PLID 28827 - We no longer take a minimum size.
			//TES 1/14/2008 - PLID 24157 - Make sure that the window is at least as big as m_szMin.
			// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
			if(szNew.cx < m_szMin.cx) {
				szNew.cx = m_szMin.cx;
			}
			if(szNew.cy < m_szMin.cy) {
				szNew.cy = m_szMin.cy;
			}

			//TES 1/18/2008 - PLID 24157 - We need to adjust off our border, if any.  Otherwise, Windows will simply
			// not draw the scroll bars.
			CSize szBorder;
			CalcWindowBorderSize(this, &szBorder);
			// Adjust off the border
			szNew.cx -= szBorder.cx;
			szNew.cy -= szBorder.cy;

			//TES 1/14/2008 - PLID 24157 - Take into account any margins we were given.
			MoveWindow(m_nLeftMargin, m_nTopMargin, szNew.cx, szNew.cy);

			CRect rcWindow;
			GetWindowRect(&rcWindow);
			ScreenToClient(&rcWindow);

			// Take the horizontal scroll bar's height off
			szNew.cy -= GetSystemMetrics(SM_CYHSCROLL);

			// Take the vertical scroll bar's width off
			// (a.walling 2010-06-21 15:07) - PLID 38779 - shouldn't this be szNew.cx?
			//rcWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
			szNew.cx -= GetSystemMetrics(SM_CXVSCROLL);

			if (IsScrollBarVisible(this->m_hWnd, SB_HORZ)) {
				// Set up the horizontal scroll bar
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_RANGE | SIF_PAGE;
				
				// Set the horz bar
				si.nMin = rcWindow.left;
				si.nMax = lHorzSize;
				si.nPage = szNew.cx;
				SetScrollInfo(SB_HORZ, &si, TRUE);		
			}

			if (IsScrollBarVisible(this->m_hWnd, SB_VERT)) {

				// Set up the vertical scroll bar
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_RANGE | SIF_PAGE;
				
				// Set the vert bar
				si.nMin = rcWindow.top;
				si.nMax = lVertSize;
				si.nPage = szNew.cy;
				SetScrollInfo(SB_VERT, &si, TRUE);	
			}

			// (j.jones 2006-08-09 09:28) - PLID 21859 - re-set the column widths to
			// force the datalist to realize it doesn't need a scrollbar
			// (c.haag 2008-10-17 10:37) - PLID 31700 - Moved to EmrItemAdvTableBase
			if(m_pDetail->m_EMRInfoType == eitTable) {
				ResetTableColumnWidths();
			}

			if(m_pDetail->m_EMRInfoType == eitImage)
			{
				//for images, we have to load the image and ink data AFTER the initial sizing
				//m.hancock - 3/2/2006 - PLID 19521 - Calculate the image source
				CString strFullPath = m_pDetail->GetImagePath(true);
				if(m_Image) {
					//m.hancock - 3/2/2006 - PLID 19521 - Check if the image has been set or changed from a previous selection
					// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - Operator != is ambiguous
					if((m_Image->GetPictureFileName() == _bstr_t("")) || (strFullPath.Compare((LPCTSTR)m_Image->GetPictureFileName()) != 0)) {
						try {
							// (c.haag 2008-09-10 09:24) - PLID 31145 - Don't just blindly set the picture file name. It could be
							// a linked image from Mirror. Now it mimics the code from EmrItemAdvImageDlg
							//m.hancock - 3/2/2006 - PLID 19521 - Pop-up images should have the ... browse button.
							//set the image by "changing" it's name (we're not really changing it, just giving that impression to the system)
							//m_Image->PictureFileName = _bstr_t(strFullPath);
							if (m_pDetail->GetImageType() == itMirrorImage)
							{								
								// (a.walling 2011-05-02 15:41) - PLID 43530 - Cache the mirror image as well (loading duplicate full-res images takes time and a lot of memory)
								/*g_EmrImageCache.GetCachedImage(CString("mirror://") + strFullPath, m_pCachedImage);
								
								HBITMAP hbmp = m_pCachedImage ? m_pCachedImage->GetImage() : NULL;
								m_Image->Image = (OLE_HANDLE)hbmp;

								ASSERT(hbmp);*/
								
								// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
								NxImageLib::ISourceImagePtr pSource = NxImageLib::Cache::OpenSourceImage(CString("mirror://") + strFullPath);
								if (pSource && SUCCEEDED(pSource->raw_Load())) {
									m_Image->Source = (IUnknown*)(pSource);
								} else {
									m_Image->Image = (OLE_HANDLE)NULL;
								}

							}
							// (a.walling 2011-05-02 15:25) - PLID 43530 - We should only do this if we are NOT a mirror image! The 'else' was missing.
							else if (strFullPath.GetLength()) // && FileUtils::DoesFileOrDirExist(strFullPath))
							{
								// (a.walling 2010-10-25 10:04) - PLID 41043 - Get a cached image reference and use that instead
								/*g_EmrImageCache.GetCachedImage(strFullPath, m_pCachedImage);

								HBITMAP hbmp = m_pCachedImage ? m_pCachedImage->GetImage() : NULL;
								m_Image->Image = (OLE_HANDLE)hbmp;
								m_Image->PictureFileName = hbmp ? _bstr_t(strFullPath) : _bstr_t("");*/
								
								// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
								NxImageLib::ISourceImagePtr pSource = NxImageLib::Cache::OpenSourceImage(strFullPath);
								if (pSource && SUCCEEDED(pSource->raw_Load())) {
									m_Image->Source = (IUnknown*)(pSource);
								} else {
									m_Image->Image = (OLE_HANDLE)NULL;
								}
								m_Image->PictureFileName = pSource ? _bstr_t(strFullPath) : _bstr_t("");
							}
							else {
								m_Image->PictureFileName = _bstr_t("");
								m_Image->Image = NULL;
							}

							//now resize to adapt to this new image size
							bReposition = TRUE;
						
						}NxCatchAll("Error in CallRepositionControls() - Setting Image");
					}
					else if(m_Image->GetInkData().vt == VT_NULL
						|| m_Image->GetInkData().vt == VT_EMPTY) {

						try {

							m_Image->PutCurrentPenColor(m_nCurPenColor);
							m_Image->PutCurrentPenSize(m_fltCurPenSize);
							m_Image->PutIsEraserState(m_bIsEraserState);

							// Update the ink strokes
							m_Image->PutInkData(m_pDetail->GetInkData());
							bReposition = TRUE;

						}NxCatchAll("Error in CallRepositionControls() - Setting Ink");
					}
				}
				else if(m_p3DControl != NULL)
				{
					if(!FileUtils::DoesFileOrDirExist(strFullPath)) {
						strFullPath.Empty();
					}
					// (z.manning 2011-07-22 16:53) - PLID 44649 - Load the image file if it changed
					m_p3DControl->PutModelFileName(_bstr_t(strFullPath));
					ReflectCurrentState();
				}
				else {
					//should be impossible...
					ASSERT(FALSE);
					bReposition = TRUE;
				}
			}
		}

		//TES 1/15/2008 - PLID 24157 - This is now a message that can be sent to any window, rather than a function
		// specific to CEMRItemAdvPopupDlg.
		// (a.walling 2009-06-22 10:30) - PLID 34635 - Post this message rather than send it, like everywhere else does.
		m_pParentDlg->PostMessage(NXM_EMR_POPUP_RESIZED, (WPARAM)this);

		// (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
		/*if(m_pDetail->m_EMRInfoType == eitImage) {
			// (r.gonet 05/31/2011) - PLID 43896 
			m_Image->SetLoading(FALSE);
			double dZoomLevel = m_pDetail->GetZoomLevel();
			long nOffsetX = m_pDetail->GetOffsetX();
			long nOffsetY = m_pDetail->GetOffsetY();
			// (r.gonet 05/31/2011) - PLID 43896 - This is the first place where we know it is safe to zoom in on an image. All initial resizing has ceased.
			if(dZoomLevel != -1) {
				m_Image->SetZoomLevel(dZoomLevel);
				m_Image->SetViewportOffsets(nOffsetX, nOffsetY);
			} else {
				m_Image->ZoomToFit(TRUE);
			}
		}*/
	}NxCatchAll("Error in CallRepositionControls()");
}

HBRUSH CEMRItemAdvPopupWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	try {
#pragma TODO("!!!!!!!!!Set color for current meds / allergy table when popped up!")
		//if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetSafeHwnd() && pWnd->GetParent()->GetSafeHwnd() == m_hWnd) {
		//	if (IsControlValid(&m_wndLabel) && pWnd->GetDlgCtrlID() == m_wndLabel.GetDlgCtrlID())
		//	{
		//		// (a.wetta 2007-02-07 17:33) - PLID 24564 - If this detail is an active current medications list, color the details title blue
		//		// (c.haag 2007-04-02 15:42) - PLID 25465 - If this is an active allergy table, then color the detail title blue
		//		if (m_bIsActiveCurrentMedicationsTable || m_bIsActiveAllergiesTable) {
		//			pDC->SetTextColor(RGB(0,0,128));
		//		}

		//		//DRT 4/24/2008 - PLID 29771 - This covers non-buttons in the list... mostly just the "Label" types.
		//		pDC->SetBkColor(GetNxColor(GNC_EMR_ITEM_BG, 0));
		//		hbr = (HBRUSH)m_brBackground;
		//	}
		//}

		if(nCtlColor == CTLCOLOR_DLG /*&& pWnd->GetSafeHwnd() == m_hWnd*/) {
			pDC->SetBkColor(GetNxColor(GNC_EMR_ITEM_BG, 0));
			return (HBRUSH)m_brBackground;
		}

		//DRT 4/24/2008 - PLID 29771 - The static label title should always be given the right background color
		if(nCtlColor == CTLCOLOR_STATIC) {
			pDC->SetBkColor(GetNxColor(GNC_EMR_ITEM_BG, 0));
			return (HBRUSH)m_brBackground;
		}
		
	} NxCatchAllIgnore();

	return hbr;
}

int CEMRItemAdvPopupWnd::ValueToSliderPos(double dValue)
{
	//First, scale by our increment.
	// (b.cardillo 2006-09-18 13:55) - PLID 22215 - Need to round here, not truncate.  For 
	// example, if dValue is 4.2999999998, and the increment is 0.1, then we would get 42 
	// for nPos by truncating, instead of 43 which is obviously what we want.
	/*int nPos = RoundAsLong(dValue / m_pDetail->GetSliderInc());

	//Now, if we're vertical, we need to flip it.
	if(m_Slider.GetStyle() & TBS_VERT) {
		int nDistanceFromEdge = (int)(m_pDetail->GetSliderMax()/m_pDetail->GetSliderInc()) - nPos;
		nPos = (int)(m_pDetail->GetSliderMin()/m_pDetail->GetSliderInc()) + nDistanceFromEdge;
	}

	return nPos;*/

	// (a.walling 2007-02-28 12:35) - PLID 24914 - Several issues with slider values. I've simplified the way
	// we store this data and calculate between values and positions. Now the slider control's minimum value is
	// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
	// solving the problem with odd increments allowing values below the minimum (and then all the incremented
	// values being off).

	// I don't know why or how this could happen, but might as well check for it.
	if (m_pDetail->GetSliderInc() == 0) {
		ThrowNxException("CEMRItemAdvPopupWnd::ValueToSliderPos(%g): Slider increment value is zero!", dValue);
	}

	dValue -= m_pDetail->GetSliderMin();
	dValue /= m_pDetail->GetSliderInc();
	long nSliderPos = RoundAsLong(dValue);

	// flip the slider position if vertical. It is easier to flip the position after being calculated
	// rather than introduce more possible rounding errors. See SliderPosToValue for more info.
	if (m_Slider.GetStyle() & TBS_VERT) {
		nSliderPos = m_Slider.GetRangeMax() - nSliderPos;
	}

	// If you reach these asserts, it most likely means that the detail's state is inconsistent with the
	// actual min and max and inc values for the slider. The only way I can see this happening is either
	// corrupted data or a slider which has just been modified on the fly. A vertical slider will have an
	// invalid negative pos, and a horizontal slider will have an invalid pos > max. This situation arose
	// previously, as well, but was silent since there were no asserts. Regardless, setting the slider's
	// pos to these incorrect values as before will effectively set thumb to the max or min position, and
	// the incorrect slider value will stay until the slider is moved. This behaviour will be investigated
	// in PLID 25002
	// (a.walling 2010-04-26 10:23) - PLID 25002 - This is how it will remain, like everywhere else in the
	// EMN. The out of range value will continue to be saved until the slider is modified.
	//ASSERT(nSliderPos >= 0);
	//ASSERT(nSliderPos <= m_Slider.GetRangeMax());

	return nSliderPos;
}

double CEMRItemAdvPopupWnd::SliderPosToValue(int nSliderPos)
{
	//First, if we're vertical, flip it.
	/*if(m_Slider.GetStyle() & TBS_VERT) {
		int nDistanceFromEdge = (int)(m_pDetail->GetSliderMax()/m_pDetail->GetSliderInc()) - nSliderPos;
		nSliderPos = (int)(m_pDetail->GetSliderMin()/m_pDetail->GetSliderInc()) + nDistanceFromEdge;
	}

	//Now, factor in our increment.
	return (double)nSliderPos * m_pDetail->GetSliderInc();*/

	// (a.walling 2007-02-28 12:34) - PLID 24914 - Several issues with slider values. I've simplified the way
	// we store this data and calculate between values and positions. Now the slider control's minimum value is
	// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
	// solving the problem with odd increments allowing values below the minimum (and then all the incremented
	// values being off).

	// If we are vertical, flip the position. Since the min will internally be zero, it's easy enough
	// to calculate that the opposite is the max minus the current. ie, 0 -> max, 1 -> max - 1, etc.
	if(m_Slider.GetStyle() & TBS_VERT) {
		nSliderPos = m_Slider.GetRangeMax() - nSliderPos;
		ASSERT(nSliderPos >= 0); // see ValueToSliderPos for discussion on related asserts.
	}

	double dblVal = nSliderPos * m_pDetail->GetSliderInc();
	dblVal += m_pDetail->GetSliderMin();

	return dblVal;
}

void CEMRItemAdvPopupWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(m_Slider.GetSafeHwnd()) {
		double dValue = SliderPosToValue(m_Slider.GetPos());
		RequestDetailStateChange(dValue);
		//TES 1/15/2008 - PLID 24157 - Notify our parent.
		m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
		m_Caption.SetWindowText(AsString(m_pDetail->GetState()));
	}
	
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEMRItemAdvPopupWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(m_Slider.GetSafeHwnd()) {
		double dValue = SliderPosToValue(m_Slider.GetPos());
		RequestDetailStateChange(dValue);
		//TES 1/15/2008 - PLID 24157 - Notify our parent.
		m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
		m_Caption.SetWindowText(AsString(m_pDetail->GetState()));
	}
	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CEMRItemAdvPopupWnd::OnChangeEdit() 
{
	CString str;
	GetDlgItemText(500, str);
	RequestDetailStateChange(_bstr_t(str));
	//TES 1/15/2008 - PLID 24157 - Notify our parent.
	m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
}

// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state, and recreate content if necessary due to state
void CEMRItemAdvPopupWnd::ReflectListState()
{
	if (!TryReflectListState()) {
		CreateControls();
		CallRepositionControls();
		TryReflectListState();
	}
}

// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state
bool CEMRItemAdvPopupWnd::TryReflectListState()
{
	// First clear all checkboxes/radiobuttons
	// (b.cardillo 2006-02-23 17:54) - PLID 19376 - Notice we invalidate them all here too now.  
	// They were all being invalidated implicitly before because we used to not clip children.  
	// Now that we do, we have to ask our children to draw themselves.
	{
		for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
			CheckDlgButton(1000 + i, 0);
			CWnd *pwnd = GetDlgItem(1000 + i);
			if (pwnd->GetSafeHwnd()) {
				pwnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
			}
		}
		CheckDlgButton(2000, 0);
		CWnd *pwnd = GetDlgItem(2000);
		if (pwnd->GetSafeHwnd()) {
			pwnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
		}
	}

	bool bCheckedAll = true;

	// Then set whichever ones are in the curstate list
	// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
	if (m_pDetail->GetStateVarType() != VT_NULL && m_pDetail->GetStateVarType() != VT_EMPTY) {
		CString strKeyList = VarString(m_pDetail->GetState());
		CString strValidKeyList;
		long nPos = 0;
		while (nPos < strKeyList.GetLength()) {
			// Get the current key and move the index to the beginning of the next key
			long nLen = strKeyList.Find(";", nPos) - nPos;
			if (nLen < 0) {
				nLen = strKeyList.GetLength() - nPos;
			}
			CString strCurKey = strKeyList.Mid(nPos, nLen);
			nPos = nPos + nLen + 2;
			// Find this key in the list
			long nKeyIndex;
			{
				nKeyIndex = -1;
				for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
					if (AsString(m_pDetail->GetListElement(i).nID) == strCurKey) {
						nKeyIndex = i;
						break;
					}
				}
			}
			// Select that control
			if (nKeyIndex != -1) {
				if (bCheckedAll) {
					CWnd* pButton = GetDlgItem(1000 + nKeyIndex);
					if (!pButton) {
						bCheckedAll = false;
					}
				}

				// (a.walling 2012-12-03 12:57) - PLID 53988 - index of m_arypControls no longer related to the ListElement position in the detail
				if (m_pDetail->m_EMRInfoType == eitSingleList) {
					// Found the item, select it (and only it) and we're done
					CheckRadioButton(1000, 1000 + m_pDetail->GetListElementCount() - 1, 1000 + nKeyIndex);
					return bCheckedAll;
				} else if (m_pDetail->m_EMRInfoType == eitMultiList) {
					// Found this item, select it and break out of the inner loop but keep looping in the outer one
					CheckDlgButton(1000 + nKeyIndex, 1);
				}

				if (strValidKeyList.GetLength())
					strValidKeyList += "; " + strCurKey;
				else
					strValidKeyList = strCurKey;
			} else {
				// Couldn't find the control associated with this key; this could mean the key no longer 
				// exists in data, the owner (usually the EMRDlg) should have prevented this situation.
				//ASSERT(FALSE);
			}			
		}
		// (c.haag 2004-06-21 12:12) - PLID 13121 - If strValidKeyList does not match m_pDetail->m_varState, it means that
		// our state would violate the integrity of m_aryKeys. This can happen if you make a list detail, then select an
		// element from the list, then delete that element from Add/Edit Items. The approach here is to assign a new state
		// to this detail that does not violate data integrity, and then redraw the detail.
		//
		// Though this code helps us in the problem of data validation, this is not always called before a chart
		// is saved in funky or improper Practice setups, so there is not a 100% guarantee that when a
		// chart is saved, you won't get an error about invalid multi-list selections.
		if (VarString(m_pDetail->GetState()) != strValidKeyList)
		{
			m_pDetail->SetState(_bstr_t(strValidKeyList));
			RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
		}
	}

	return bCheckedAll;
}

void CEMRItemAdvPopupWnd::ReflectTableState()
{
	// (c.haag 2008-10-17 11:49) - PLID 31700 - Moved logic to EmrItemAdvTableBase
	CEmrItemAdvTableBase::ReflectTableState(this, m_pDetail, TRUE);
}

void CEMRItemAdvPopupWnd::ReflectImageState()
{
	// (z.manning, 08/10/2007) - PLID 26630 - Update the ink and the image.
	if(m_Image != NULL)
	{
		m_Image->PutInkData(m_pDetail->GetInkData());
		m_Image->PutTextData(m_pDetail->GetImageTextData());
		// (r.gonet 02/14/2012) - PLID 37682 - Also set the filter.
		this->SetImageTextStringFilter(m_pDetail->GetImageTextStringFilter());

		// (z.manning, 02/21/2008) - PLID 28690 - Reload hot spots.
		RefreshHotSpots(m_Image, m_pDetail);
	}
	else if(m_p3DControl != NULL)
	{
		for(int nHotSpotIndex = 0; nHotSpotIndex < m_pDetail->GetHotSpotArray()->GetSize(); nHotSpotIndex++)
		{
			CEMRHotSpot *pSpot(m_pDetail->GetHotSpotArray()->GetAt(nHotSpotIndex));
			m_p3DControl->SetHotSpotState(pSpot->Get3DHotSpotID(), pSpot->GetSelected() ? VARIANT_TRUE : VARIANT_FALSE);
		}

		// (z.manning 2011-09-09 12:25) - PLID 45335 - Update the stamp data
		m_p3DControl->PutTextData(m_pDetail->GetImageTextData());

		// (z.manning 2011-10-05 17:14) - PLID 45842 - Print data
		m_p3DControl->PutViewData(m_pDetail->GetImagePrintData());

		// (r.gonet 02/14/2012) - PLID 37682 - Filter too.
		this->SetImageTextStringFilter(m_pDetail->GetImageTextStringFilter());
	}
}

void CEMRItemAdvPopupWnd::OnButtonClicked(UINT nID)
{
	try {
		BOOL bNewChecked = IsDlgButtonChecked(nID);
		
		// (a.walling 2012-12-03 12:57) - PLID 53988 - Expand or contract parent labels as necessary
		{
			long nIndex = nID - 1000;
			ListElement leClicked = m_pDetail->GetListElement(nIndex);
			if (leClicked.bIsLabel) {				
				if (m_expandedLabelIDs.count(leClicked.nID)) {
					m_expandedLabelIDs.erase(leClicked.nID);
				} else {
					m_expandedLabelIDs.insert(leClicked.nID);
				}

				foreach(const ListElement& l, m_pDetail->GetListElements()) {
					if (l.nParentLabelID == leClicked.nID) {
						CreateControls();
						CallRepositionControls();
						ReflectCurrentState();
											
						if (CWnd* pCur = GetDlgItem(nID)) {
							pCur->SetFocus();
						}

						break;
					}
				}

				return;
			}
		}

		// (a.walling 2012-04-10 17:04) - PLID 46633 - Ensure we reset the value of other list item buttons if single select
		CWaitCursor pWait;
		CString strNewState;
		for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
			if (IsDlgButtonChecked(1000+i)) {
				NxWindowlessLib::NxFreeButtonControl* pButton = dynamic_cast<NxWindowlessLib::NxFreeButtonControl*>(GetDlgItem(1000+i));

				if (bNewChecked && (m_pDetail->m_EMRInfoType == eitSingleList) && (nID != 1000+i)) {

					if (pButton) {
						(*pButton)->Value = BST_UNCHECKED;
					} else {
						ASSERT(FALSE);
					}

				} else {
					if (!strNewState.IsEmpty()) {
						strNewState += "; ";
					}
					strNewState += FormatString("%li",m_pDetail->GetListElement(i).nID);
				}
			}
		}

		// Don't request the state change if nothing's actually changing
		// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
		if (m_pDetail->GetStateVarType() == VT_EMPTY || strNewState.Compare(VarString(m_pDetail->GetState(), "")) != 0) {
			// The state is definitely changing
			RequestDetailStateChange((LPCTSTR)strNewState);
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		}
		Invalidate();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);

		//TES 1/15/2008 - PLID 24157 - Our parent will do this if appropriate when they get the 
		// NXM_EMR_POPUP_POST_STATE_CHANGED message above.
		/*// (z.manning, 07/31/2006) - PLID 21700 - If it's a single select and they have
		// the preference enabled, dismiss this dialog.
		if(m_pDetail->m_EMRInfoType == eitSingleList) {
			if(GetRemotePropertyInt("EmnPopupAutoDismiss", 1, 0, GetCurrentUserName(), true)) {
				if(m_pParentDlg && IsWindow(m_pParentDlg->GetSafeHwnd())) {
					m_pParentDlg->EndDialog(IDOK);
				}
			}
		}*/
	} NxCatchAll("CEMRItemAdvPopupWnd::OnButtonClicked");
}

BOOL CEMRItemAdvPopupWnd::OnButtonClickedEvent(UINT nID)
{
	PostMessage(WM_COMMAND, MAKEWPARAM(nID, BN_CLICKED), 0);
	return TRUE;
}

void CEMRItemAdvPopupWnd::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	BOOL bHandledContextMenu = FALSE;
	
	// Try to pop up the menu ourselves if we're supposed to
	try {
		CRect rcLabel;
		GetControlWindowRect(this, &m_wndLabel, &rcLabel);

		//TES 3/8/2012 - PLID 45127 - Track if we right-click on a stamp
		long nRightClickedStampIndex = -1;
		long nRightClickedStampID = -1;

		// (a.walling 2008-01-16 15:24) - PLID 14982 - Add Edit to the context menu

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.CreatePopupMenu();
		enum {
			miClearInk = 1,
			// (d.thompson 2009-03-05 12:46) - PLID 32891 - Remove background image
			miRemoveBackgroundImage,
			// (c.haag 2009-02-17 17:52) - PLID 31327 - HotSpot click enabling
			miEnableHotSpotClicks,
			miDisableHotSpotClicks,
			miEditItem,
			// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
			miEditCommonLists,
			//TES 3/8/2012 - PLID 45127 - Copied from CEmrItemAdvDlg.cpp
			//TES 2/24/2012 - PLID 45127 - Option to change existing stamps to a different type
			miChangeStamp,
			//TES 3/8/2012 - PLID 48733 - Option to change all stamps of a given type to a different type
			miChangeAllStamps,
		};

		// (a.walling 2008-01-18 15:48) - PLID 14982 - Silently check for Admin EMR permissions
		DWORD dwEnabled = CheckCurrentUserPermissions(bioAdminEMR, sptRead, FALSE, 0, TRUE, TRUE) ? MF_ENABLED : MF_DISABLED;

		if(m_pDetail->m_EMRInfoType == eitImage)
		{
			CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
			BOOL bIsLockedEMN = pEMN->GetStatus() == 2;		
			if(!bIsLockedEMN && !m_bReadOnly && (m_Image != NULL || m_p3DControl != NULL)) // (a.walling 2007-06-22 12:31) - PLID 22097 - Don't popup image menu if readonly
			{
				//TES 3/8/2012 - PLID 45127 - Copied all this code from CEmrItemAdvDlg
				//TES 2/24/2012 - PLID 45127 - If they have right-clicked on a stamp, give them the option to change its type.
				CPoint ptInk(pos.x,pos.y);
				m_wndImage.ScreenToClient(&ptInk);
				if(m_Image != NULL) {
					m_Image->GetStampFromPoint(ptInk.x, ptInk.y, &nRightClickedStampIndex);
				}
				//TES 3/9/2012 - PLID 45127 - Added the same option for the 3D version
				else if(m_p3DControl != NULL) {
					m_p3DControl->GetStampFromPoint(ptInk.x, ptInk.y, &nRightClickedStampIndex);
				}
				if(nRightClickedStampIndex > -1) {
					mnu.AppendMenu(MF_SEPARATOR);
					CNxInkPictureText nipt;
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(m_pDetail->GetState());
					nipt.LoadFromVariant(ais.m_varTextData);
					// (r.gonet 05/02/2012) - PLID 49949 - We don't allow changes to image stamps because it doesn't make sense to do that.
					if(!nipt.GetTextString(nRightClickedStampIndex).ImageInfo.bHasImage) {
						//TES 2/24/2012 - PLID 45127 - Include the stamp text, in case there are two overlapping stamps, to make it clear
						// which one is about to get changed.
						CString strStamp = nipt.GetStringByIndex(nRightClickedStampIndex);
						//TES 2/24/2012 - PLID 45127 - Trim the dot (if any).
						strStamp.TrimLeft((char)149);
						strStamp.TrimLeft(" ");
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miChangeStamp, "Change Stamp From " + strStamp + " To ...");

						//TES 3/8/2012 - PLID 48733 - See if this stamp is Increase Quantity, and if there are multiple instances of it on this
						// image, and if so give the Change All Stamps option
						nRightClickedStampID = nipt.GetStampIDByIndex(nRightClickedStampIndex);
						EMRImageStamp *pGlobalStamp = GetMainFrame()->GetEMRImageStampByID(nRightClickedStampID);
						//TES 3/30/2012 - PLID 48733 - pGlobalStamp might be NULL, if it's a free-text stamp
						if(pGlobalStamp && pGlobalStamp->eSmartStampTableSpawnRule == esstsrIncreaseQuantity) {
							bool bOneFound = false;
							bool bMultipleFound = false;
							for(int nStamp = 0; nStamp < nipt.GetStringCount() && !bMultipleFound; nStamp++) {
								if(nipt.GetTextString(nStamp).nStampID == nRightClickedStampID) {
									if(!bOneFound) bOneFound = true;
									else bMultipleFound = true;
								}
							}
							if(bMultipleFound) {
								mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miChangeAllStamps, "Change All " + strStamp + " Stamps To ...");
							}
						}

						mnu.AppendMenu(MF_SEPARATOR);
					}
				}
			}
		}

		// (a.walling 2008-01-18 16:16) - PLID 14982 - Real Detail should only be NULL when previewing a table
		// (a.walling 2008-02-11 16:44) - PLID 14982 - Don't edit if read only (ie, locked EMNs!)
		if (!m_bReadOnly && pos.y >= rcLabel.top && pos.y <= rcLabel.bottom && m_pRealDetail != NULL) {
			bHandledContextMenu = TRUE;
			mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, miEditItem, "Edit");
			// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
			if (m_pDetail->IsCurrentMedicationsTable() || m_pDetail->IsAllergiesTable()) 
			{
				mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, miEditCommonLists, "Edit Common &Lists...");
			}
			if (m_pDetail->m_EMRInfoType == eitImage) {
				mnu.AppendMenu(MF_SEPARATOR);
			} else {
				CPoint pt = CalcContextMenuPos(pWnd, pos);
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				switch (nResult) {
					case miEditItem: {
						OnEditItem();
						break;
					// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
					case miEditCommonLists:
						OpenItemEntryDlg(eEmrItemEntryDlgBehavior_EditCommonLists);
						break;
					}
				}
			}
		}

		// (j.jones 2006-09-18 10:24) - PLID 22555 - if an unlocked EMN, allow the user to clear all the ink
		if(m_pDetail->m_EMRInfoType == eitImage)
		{
			CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
			BOOL bIsLockedEMN = pEMN->GetStatus() == 2;		
			if(!bIsLockedEMN && !m_bReadOnly) // (a.walling 2007-06-22 12:31) - PLID 22097 - Don't popup image menu if readonly
			{
				// (z.manning 2011-09-15 09:54) - PLID 45335 - Separate the options for 3D images
				if(m_pDetail->Is3DImage())
				{
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miClearInk, "Erase All Stamps");
				}
				else if(m_Image != NULL)
				{
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miClearInk, "Erase All Ink");

					// (d.thompson 2009-03-04) - PLID 32891 - Let the user remove the image from the detail entirely
					//	if they don't want it anymore.
					// (d.thompson 2009-03-24) - PLID 33643 - Some time later, it was decided this should be controlled
					//	by a preference to enable it.
					if(GetRemotePropertyInt("EMNDetail_AllowRemoveBackgroundImage", 0, 0, GetCurrentUserName(), true) != 0) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miRemoveBackgroundImage, "Remove Back&ground Image");
					}

					// (c.haag 2009-02-18 09:58) - PLID 31327 - Let the user stamp on hotspots
					if (VARIANT_FALSE != m_Image->GetEnableHotSpotClicks()) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miDisableHotSpotClicks, "Enable &Writing on Hotspots");
					} else {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEnableHotSpotClicks, "Disable &Writing on Hotspots");
					}
				}

				bHandledContextMenu = TRUE;
	
				CPoint pt = CalcContextMenuPos(pWnd, pos);
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				switch (nResult) {
				case miClearInk: {
	
					//grab the existing ink
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(m_pDetail->GetState());
	
					// (a.walling 2011-07-22 09:45) - PLID 44612 - Also check for text data
					if(ais.m_varInkData.vt > VT_NULL || ais.m_varTextData.vt > VT_NULL) {
						// (z.manning 2008-12-01 11:59) - PLID 32182 - Warn them before erasing ink.
						CString strInkWord = m_pDetail->Is3DImage() ? "stamps" : "ink";
						CString strDontShowPropName = m_pDetail->Is3DImage() ? "EmrEraseAllInkWarning3D" : "EmrEraseAllInkWarning";
						int nResult = DontShowMeAgain(this, "Are you sure you want to erase all " + strInkWord + " on this image?", strDontShowPropName, "Erase All", FALSE, TRUE);
						if(nResult == IDNO) {
							break;
						}
					}
					
					//clear out the ink
					VariantClear(&ais.m_varInkData);
					m_pDetail->SetInkErased();
					//TES 1/22/2007 - PLID 18159 - Also the text.
					VariantClear(&ais.m_varTextData);
					m_pDetail->SetImageTextRemoved();
	
					//assign the new state
					m_pDetail->SetInkData(ais.m_varInkData);
					m_pDetail->SetImageTextData(ais.m_varTextData);
					if(m_Image != NULL) {
						m_Image->PutInkData(m_pDetail->GetInkData());
						m_Image->PutTextData(m_pDetail->GetImageTextData());
					}
					else if(m_p3DControl != NULL) {
						m_p3DControl->PutTextData(m_pDetail->GetImageTextData());
					}
					break;
				}
				// (d.thompson 2009-03-04) - PLID 32891 - Ability to remove the background image from the detail
				case miRemoveBackgroundImage: 
					{
						//Get the existing state to work from
						CEmrItemAdvImageState ais;
						ais.CreateFromSafeArrayVariant(m_pDetail->GetState());
						//wipe it all from the state
						ais.m_eitImageTypeOverride = itForcedBlank;
						ais.m_strImagePathOverride = "";
	
						//clear out the ink & text if necessary
						if(ais.m_varInkData.vt != VT_EMPTY && ais.m_varInkData.vt != NULL) {
							VariantClear(&ais.m_varInkData);
							m_pDetail->SetInkErased();
						}
						if(ais.m_varTextData.vt != VT_EMPTY && ais.m_varTextData.vt != VT_NULL) {
							VariantClear(&ais.m_varTextData);
							m_pDetail->SetImageTextRemoved();
						}
						//now assign the new state
						RequestDetailStateChange(ais.AsSafeArrayVariant());
	
						m_Image->Image = NULL;
						m_Image->PutPictureFileName("");
						RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
					}
					break;
				//TES 3/8/2012 - PLID 45127 - Copied all this code here from CEmrItemAdvDlg
				case miChangeStamp: 
				case miChangeAllStamps: {
					//TES 2/24/2012 - PLID 45127 - Pop up the dialog to allow them to select a new stamp yet.
					CSelectStampDlg dlg(this);
					//TES 3/28/2012 - PLID 49294 - We need to tell the dialog what image we are on
					dlg.m_nImageEmrInfoMasterID = m_pDetail->m_nEMRInfoMasterID;
					if(IDOK == dlg.DoModal()) {
						//TES 2/24/2012 - PLID 45127 - Actually change the stamp.
						CNxInkPictureText nipt;
						nipt.LoadFromVariant(m_pDetail->GetImageTextData());
						TextString tsSelected = nipt.GetTextString(nRightClickedStampIndex);
						dlg.GetSelectedStamp(tsSelected.nStampID, tsSelected.str, tsSelected.strTypeName, tsSelected.color);
						for(int nStamp = 0; nStamp < nipt.GetStringCount(); nStamp++) {
							//TES 3/8/2012 - PLID 48733 - Replace if this is the stamp we right-clicked on OR it has the same type, if
							// the Change All option was selected
							if(nStamp == nRightClickedStampIndex || (nResult == miChangeAllStamps && nipt.GetStampIDByIndex(nStamp) == nRightClickedStampID)) {
								TextString ts = nipt.GetTextString(nStamp);
								ts.nStampID = tsSelected.nStampID;
								ts.str = tsSelected.str;
								ts.strTypeName = tsSelected.strTypeName;
								ts.color = tsSelected.color;
								nipt.SetTextString(nStamp, ts);
							}
						}
						//TES 3/6/2012 - PLID 45127 - The detail will handle everything when it sees the new text data.
						m_pDetail->SetImageTextData(nipt.GetAsVariant());
						if(m_Image != NULL) {
							m_Image->TextData = nipt.GetAsVariant();
						}
						//TES 3/9/2012 - PLID 45127 - Added the same option for the 3D version
						else if(m_p3DControl != NULL) {
							m_p3DControl->TextData = nipt.GetAsVariant();
						}
						
					}
				}
				break;
				case miEnableHotSpotClicks: // (c.haag 2009-02-18 09:59) - PLID 31327
					{
						m_Image->PutEnableHotSpotClicks(VARIANT_TRUE);
						// Also update the one on the topic. We have to keep them in sync.
						if (NULL != m_pRealDetail && NULL != m_pRealDetail->m_pEmrItemAdvDlg) {
							((CEmrItemAdvImageDlg*)m_pRealDetail->m_pEmrItemAdvDlg)->SetEnableHotSpotClicks(TRUE);
						}
					}
					break;
				case miDisableHotSpotClicks: // (c.haag 2009-02-18 09:59) - PLID 31327
					{
						m_Image->PutEnableHotSpotClicks(VARIANT_FALSE);
						// Also update the one on the topic. We have to keep them in sync.
						if (NULL != m_pRealDetail && NULL != m_pRealDetail->m_pEmrItemAdvDlg) {
							((CEmrItemAdvImageDlg*)m_pRealDetail->m_pEmrItemAdvDlg)->SetEnableHotSpotClicks(FALSE);
						}
					}
					break;
				case miEditItem: {
					OnEditItem();
					break;
				}
				case 0:
					// The user canceled, do nothing
					break;
				}
			}
		}

		if(!bHandledContextMenu)
			bHandledContextMenu = DataElementPopUp(CalcContextMenuPos(pWnd, pos));

		// (a.walling 2008-01-18 15:38) - PLID 14982 - If there was no data element popup, go ahead and pop up
		// our Edit option, assuming there is a real detail (otherwise this is a previewed table)
		// (a.walling 2008-02-11 16:44) - PLID 14982 - Don't edit if read only (ie, locked EMNs!)
		if(!m_bReadOnly && !bHandledContextMenu && m_pRealDetail != NULL) {
			bHandledContextMenu = TRUE;

			mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, miEditItem, "Edit");
			// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
			if (m_pDetail->IsCurrentMedicationsTable() || m_pDetail->IsAllergiesTable()) 
			{
				mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, miEditCommonLists, "Edit Common &Lists...");
			}

			if (m_pDetail->m_EMRInfoType == eitImage) {
				mnu.AppendMenu(MF_SEPARATOR);
			} else {
				CPoint pt = CalcContextMenuPos(pWnd, pos);
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				switch (nResult) {
					case miEditItem: {
						OnEditItem();
						break;
					// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
					case miEditCommonLists:
						OpenItemEntryDlg(eEmrItemEntryDlgBehavior_EditCommonLists);
						break;
					}
				}
			}
		}
	} NxCatchAll("CEmrItemAdvListDlg::OnContextMenu");

	if (!bHandledContextMenu) {
		// Give the base class its chance
		CWnd::OnContextMenu(pWnd, pos);
	}
}

BOOL CEMRItemAdvPopupWnd::DataElementPopUp(CPoint pt)
{
	// (a.walling 2007-06-21 12:34) - PLID 22097 - Don't pop up anything if we are readonly.
	if (m_bReadOnly) return FALSE; // we didn't really handle it, so return FALSE in case a base class wants to do something.

	// Find out which ID was right-clicked on
	long nSelButtonDataID = -1;
	BOOL bAnythingSelected = FALSE;
	{
		// Loop through the whole list to determine both the button the point 
		// is over and whether ANY button is selected.
		nSelButtonDataID = -1;
		bAnythingSelected = FALSE;
		// (a.walling 2012-12-03 12:57) - PLID 53988 - index of m_arypControls does not correlate with the ListElement index of the detail
		CRect rect;
		for (int i = 0; i < m_arypControls.GetSize(); ++i) {
			CWnd *pBtn = m_arypControls.GetAt(i);

			if (!pBtn) {
				continue;
			}

			int nID = pBtn->GetDlgCtrlID();
			int nIndex = nID - 1000;

			if (nID < 1000 || nID > 1999) {
				continue;
			}

			GetControlWindowRect(this, pBtn, &rect);
			
			if (!rect.IsRectEmpty() && rect.PtInRect(pt)) {
				// Got the button, calculate its data ID
				ASSERT(nSelButtonDataID == -1); // There ought to be only one button that the mouse is over.
				nSelButtonDataID = m_pDetail->GetListElement(nIndex).nID;
				// And give it focus because it was just right-clicked upon
				pBtn->SetFocus();
			}

			if (!bAnythingSelected && IsDlgButtonChecked(nID)) {
				// An item is selected, so now we know
				bAnythingSelected = TRUE;
			}
		}
	}

	// If we got a data ID above, then give the user the context menu
	if (nSelButtonDataID != -1) {
		// Just use an enum instead of using const variables or number literals
		enum {
			miUnselect = 1,
		};

		// Create the menu
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.CreatePopupMenu();

		BOOL bShow = FALSE;

		// Only add the unselect menu item if this item is selected
		if (m_pDetail->m_EMRInfoType == eitSingleList) {
			//mnu.AppendMenu(MF_SEPARATOR);
			mnu.AppendMenu((bAnythingSelected ? MF_ENABLED : MF_DISABLED|MF_GRAYED)|MF_STRING|MF_BYPOSITION, miUnselect, "&Unselect");
			bShow = TRUE;
		}

		if(!bShow)
			return FALSE;

		switch (mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this)) {

		case miUnselect:
			// Call the unselect functionality
			{
				// Just ask the detail to change the state to "nothing selected" and then reflect the 
				// current state on our own screen (which we do ourselves because technically it's our 
				// responsibility, even though in most cases the RequestStateChange() function will 
				// call it for us too).
				if (RequestDetailStateChange("")) {
					//TES 1/15/2008 - PLID 24157 - Notify our parent.
					m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
					ReflectListState();
					RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
				}
			}
			break;

		case 0:
			// The user canceled the menu, do nothing
			break;
		default:
			// This should never happen because we only have 1 possible menu items (miUnselect)
			ASSERT(FALSE);
			break;
		}
		// Return TRUE because a menu was popped up (even if the user didn't do anything with it)
		return TRUE;
	} else {
		// Return FALSE because it wasn't our job to pop up a menu
		return FALSE;
	}
}


//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
void CEMRItemAdvPopupWnd::OnEditingFinishedTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// (c.haag 2008-10-20 09:42) - PLID 31700 - Moved to EmrItemAdvTableBase
	HandleEditingFinishedTable(lpRow, nCol, varOldValue, varNewValue, bCommit, this, m_pParentDlg, m_pDetail, TRUE);
}

CString CEMRItemAdvPopupWnd::GenerateNewVarState()
{
	//update the m_varState with the new values

	//Note: We can't just call RecreateStateFromContent, because that tries to look up linked details.  We need to just cache
	//the name.

	// (c.haag 2007-08-20 15:25) - PLID 27127 - Use the table element array to populate the state
	CString strNewState;
	const int nTableElements = m_pDetail->GetTableElementCount();
	for(int i = 0; i < nTableElements; i++) {
		TableElement te;
		m_pDetail->GetTableElementByIndex(i, te);
		// (z.manning 2010-02-18 09:22) - PLID 37427 - Added nEmrDetailImageStampID
		// (z.manning 2011-03-02 14:55) - PLID 42335 - Added nStampID
		// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
		AppendTableStateWithUnformattedElement(strNewState, te.m_pRow->m_ID.nDataID, te.m_pColumn->nID
			, te.GetValueAsString(), te.m_pRow->m_ID.GetDetailImageStampID(), (long)te.m_pRow->m_ID.GetDetailImageStampObject(), te.m_pRow->m_ID.GetImageStampID());
	}

	//TES 1/23/2008 - PLID 24157 - Compare against the old state (I'm not sure why this wasn't infinite recursion
	// before).
	if(strNewState != AsString(m_pDetail->GetState())) {
		RequestDetailStateChange(_bstr_t(strNewState));
	}

	//TES 1/15/2008 - PLID 24157 - Notify our parent.
	m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);

	return AsString(m_pDetail->GetState());
}

void CEMRItemAdvPopupWnd::OnStrokeInkPicture(LPDISPATCH Cursor, LPDISPATCH Stroke, BOOL FAR* Cancel)
{
	m_pDetail->SetInkData(m_Image->InkData);

	UpdateRememberedPenState();

	//Tell our detail.
	m_pDetail->SetInkData(m_Image->InkData);

	//for auditing, track what this stroke was
	if(!m_bIsEraserState)
		m_pDetail->SetInkAdded();
	else
		m_pDetail->SetInkErased();	

	ReflectImageState(); // (z.manning, 08/10/2007) - PLID 26630
}

BOOL CEMRItemAdvPopupWnd::HasValidImage()
{
	if(m_Image != NULL)
	{
		//TES 10/12/2007 - PLID 23816 - Copied out of CEmrItemAdvImageDlg.
		if(m_Image->Image != NULL) return TRUE;
		else {
			CString strFileName = (LPCTSTR)m_Image->PictureFileName;
			if(!strFileName.IsEmpty() && DoesExist(strFileName)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}
	else if(m_p3DControl != NULL)
	{
		// (z.manning 2011-09-14 12:54) - PLID 44649 - Handle 3D control too
		CString strFileName = (LPCTSTR)m_p3DControl->ModelFileName;
		if(!strFileName.IsEmpty() && DoesExist(strFileName)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	return FALSE;
}
void CEMRItemAdvPopupWnd::OnBrowseInkPicture(BOOL FAR* pbProcessed)
{
	*pbProcessed = TRUE; //We've handled the browsing, so the control doesn't need to.

	//m.hancock - 3/1/2006 - PLID 19521 - Pop-up images should have the ... browse button.
	//This was copied from CEmrTreeWnd::OnEmrItemEditContent().
	try {

		// (b.cardillo 2004-07-23 12:23) - PLID 13594 - For images the adv dlg sends a content 
		// change request but we're actually handling it like a "state" change, NOT a content 
		// change.  Notice how we don't write anything to data, because we're just changing the 
		// current state of this detail.  Thus, we need to prevent it if the user is editing a 
		// template.
		if(m_pDetail->m_bIsTemplateDetail) {
			return;
		}

		// (j.jones 2006-07-12 16:36) - PLID 21421 - if there is already ink on the image,
		// warn the user it will be cleared out, then do so if they agree to proceed

		// To detect the ink status, first create an image state object based on the current detail state
		CEmrItemAdvImageState ais;
		ais.CreateFromSafeArrayVariant(m_pDetail->GetState());

		if((ais.m_varInkData.vt != VT_EMPTY && ais.m_varInkData.vt != VT_NULL) ||
			(ais.m_varTextData.vt != VT_EMPTY && ais.m_varTextData.vt != VT_NULL) ) {
			//there is some ink on this image, so warn the user
			if(IDNO == MessageBox("Selecting a new image will erase the ink you have currently drawn.\n"
				"Are you sure you wish to continue?\n\n"
				"(Ink will not be erased until an image is chosen.)","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		CSelectImageDlg dlg(this);

		dlg.m_nPatientID = m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
		// (j.jones 2013-09-19 15:19) - PLID 58547 - added a pointer to the pic
		CEmrTreeWnd* pTreeWnd = m_pRealDetail->GetParentEMN()->GetInterface();
		if(pTreeWnd) {
			dlg.m_pPicContainer = pTreeWnd->GetPicContainer();
		}
		if (dlg.DoModal() == IDOK) {
			// (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
			//m_Image->SetLoading(TRUE);
			//clear out the ink (use the image state from above)
			VariantClear(&ais.m_varInkData);
			m_pDetail->SetInkErased();
			//TES 1/22/2007 - PLID 18159 - Also the text data.
			VariantClear(&ais.m_varTextData);
			m_pDetail->SetImageTextRemoved();

			//assign the new state
			m_pDetail->SetInkData(ais.m_varInkData);
			m_pDetail->SetImageTextData(ais.m_varTextData);

			//TES 10/12/2007 - PLID 23816 - Copied from CEMRItemAdvImageDlg.cpp.
			// (a.wetta 2007-02-26 17:15) - PLID 23816 - If a new image is dragged to the detail, make sure the new image is sized to the detail
			// if the preference is selected.  But if this is a blank built-in image detail, then we always want it to use the images original size	
			// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
			/*if (m_pDetail->m_nEMRInfoID != EMR_BUILT_IN_INFO__IMAGE || HasValidImage())
				m_pDetail->SetSizeImageToDetail(GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true));*/
			
			// (a.walling 2011-05-25 17:57) - PLID 43847 - Since this is a popup, I don't think it is really necessary to bother with this here.

			//set the image
			m_pDetail->SetImage(dlg.m_strFileName, dlg.m_nImageType);
			
			// (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
			/*m_Image->SetLoading(FALSE);
			m_Image->ZoomToFit(TRUE);
			Invalidate();*/
			CallRepositionControls();
		}
	}NxCatchAll("Error in OnBrowseInkPicture()");
}

LRESULT CEMRItemAdvPopupWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - forward extra messages to windowless controls
	if (HandleExtraWindowlessMessages(m_pCtrlCont, message, wParam, lParam, &lResult)) {
		return lResult;
	}

	switch (message) {
	case WM_VSCROLL:
		if(m_pDetail->m_EMRInfoType != eitSlider) {
			//not a slider, so scroll the window
			OnScroll(SB_VERT, LOWORD(wParam));
			return 0;
		}
		else {
			//if a slider, just pass the windowproc onto the control
			return CWnd::WindowProc(message, wParam, lParam);
		}
		break;
	case WM_HSCROLL:
		if(m_pDetail->m_EMRInfoType != eitSlider) {
			//not a slider, so scroll the window
			OnScroll(SB_HORZ, LOWORD(wParam));
			return 0;
		}
		else {
			//if a slider, just pass the windowproc onto the control
			return CWnd::WindowProc(message, wParam, lParam);
		}
		break;
	case WM_MOUSEWHEEL:
		/*try {
			// If we got the message, then do the scrolling
			short zDelta = HIWORD(wParam);
			if (zDelta > 0) {
				// The user scrolled "forward", or "up", or "away from the user"
				for (long z = 0; z < zDelta; z += WHEEL_DELTA) {
					// Scroll up vertically by three "lines"
					OnScroll(SB_VERT, SB_LINEUP);
					OnScroll(SB_VERT, SB_LINEUP);
					OnScroll(SB_VERT, SB_LINEUP);
				}
			} else if (zDelta < 0) {
				// The user scrolled "back", or "down", or "nearer to the user"
				for (long z = 0; z > zDelta; z -= WHEEL_DELTA) {
					// Scroll down vertically by three "lines"
					OnScroll(SB_VERT, SB_LINEDOWN);
					OnScroll(SB_VERT, SB_LINEDOWN);
					OnScroll(SB_VERT, SB_LINEDOWN);
				}
			}
			// Since we handled it by scrolling, then don't let anyone else handle the scrollwheel message
			return FALSE;
		} NxCatchAll("CEmrTopicWnd::WindowProc:WM_MOUSEWHEEL");
		break;*/

	// (c.haag 2011-03-18) - PLID 42969 - Handle common list button presses
	case WM_COMMAND:
		if (HandleCommonListButtonPress(m_pDetail, wParam))
		{
				// (a.walling 2011-03-21 12:34) - PLID 42962 - This calls RecreateStateFromContent, 
				// which is then passed to RequestStateChange
				CString strNewVarState = GenerateNewVarState();
				if (RequestDetailStateChange((LPCTSTR)strNewVarState)) 
				{
					// (c.haag 2011-03-18) - PLID 42969 - Update the visible table
					ReflectTableState();
					// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
					// children, invalidating the child area is not good enough, so we 
					// have to actually tell each child to invalidate itself.  Also, we 
					// used to ask it to erase too, because according to an old comment 
					// related to plid 13344, the "SetRedraw" state might be false at 
					// the moment.  (I'm deleting that comment on this check-in, so to 
					// see it you'll have to look in source history.)  But I think the 
					// "SetRedraw" state no longer can be false, and even if it were I 
					// think now that we're invalidating the control itself, we wouldn't 
					// need to erase here anyway.
					m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
					//TES 1/15/2008 - PLID 24157 - Notify our parent.
					m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
				}
		}
		break;

	default:
		break;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

void CEMRItemAdvPopupWnd::OnScroll(int nBar, const int nScrollType) {
	try {
		long nDestPos;
		long nScrollAmount;
		{
			// Get the current scroll info
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE | SIF_TRACKPOS;
			if (GetScrollInfo(nBar, &si, si.fMask) && si.nMax != si.nMin) {
				// Decide the new destination position based on what type of scrolling was 
				// requested (i.e. page up, page down, line up, line down, etc.).
				switch (nScrollType) {
				case SB_TOP:
					nDestPos = si.nMin;
					break;
				case SB_BOTTOM:
					nDestPos = si.nMax - si.nPage + 1;
					break;
				case SB_LINEDOWN:
					nDestPos = si.nPos + (si.nPage / 30 + 1);
					break;
				case SB_LINEUP:
					nDestPos = si.nPos - (si.nPage / 30 + 1);
					break;
				case SB_PAGEDOWN:
					nDestPos = si.nPos + si.nPage;
					break;
				case SB_PAGEUP:
					nDestPos = si.nPos - si.nPage;
					break;
				case SB_THUMBTRACK:
					nDestPos = si.nTrackPos;
					break;
				case SB_ENDSCROLL:
					// Scrolling finished, but we don't really need to do anything about that.
					return;
					break;
				case SB_THUMBPOSITION:
					// The user relased the thumb after dragging it.  No need to do anything 
					// because SB_THUMBTRACK actually scrolls us so we're constantly up to date.
					return;
					break;
				default:
					ASSERT(FALSE);
					return;
					break;
				}

				// Make sure we're within scrollable bounds
				long nMaxDestPos = si.nMax - si.nPage + 1;
				if (nDestPos > nMaxDestPos) {
					nDestPos = nMaxDestPos;
				}
				if (nDestPos < si.nMin) {
					nDestPos = si.nMin;
				}

				// Calculate exactly how much that means we're going to be scrolling
				nScrollAmount = si.nPos - nDestPos;
			} else {
				nScrollAmount = 0;
			}
		}

		if (nScrollAmount) {
			// Do the visual scrolling of the window contents
			if (nBar == SB_VERT) {
				ScrollWindow(0, nScrollAmount);
			} else {
				ScrollWindow(nScrollAmount, 0);
			}

			// Set the scrollbar to show the new scrolled-to position
			SetScrollPos(nBar, nDestPos, TRUE);

			//RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
		}
	} NxCatchAll("CEMRItemAdvPopupWnd::OnScroll");
}

BOOL CEMRItemAdvPopupWnd::IsScrollBarVisible(HWND hWndParent, int nBar)
{
	SCROLLBARINFO sbi;
	sbi.cbSize = sizeof(SCROLLBARINFO);
	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - GetScrollBarInfo is now a member of CWnd
	::GetScrollBarInfo(hWndParent, nBar == SB_HORZ ? OBJID_HSCROLL : OBJID_VSCROLL, &sbi);
	return ((sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) == 0);
}

//DRT 4/30/2008 - PLID 29771 - Paint the background appropriately
BOOL CEMRItemAdvPopupWnd::OnEraseBkgnd(CDC* pDC) 
{
	// (a.walling 2012-06-14 15:06) - PLID 51002 - Exclude the windowless controls when erasing the background
	CNxOleControlContainer* pNxCtrlCont = polymorphic_downcast<CNxOleControlContainer*>(GetControlContainer());

	BOOL bTouchedClipRegion = FALSE;

	if (pNxCtrlCont) {		
		bTouchedClipRegion = pNxCtrlCont->ExcludeWindowlessClipRegion(pDC);
	}

	CRect rcClient;
	GetClientRect(&rcClient);

	::FillRect(*pDC, &rcClient, m_brBackground);

	if (bTouchedClipRegion) {
		pDC->SelectClipRgn(NULL);
	}

	return TRUE;
}

// (z.manning 2011-09-12 11:48) - PLID 45335
void CEMRItemAdvPopupWnd::OnSmartStampAdd3DControl()
{
	try
	{
		Handle3DStampChange();
		m_pDetail->SetImageTextAdded();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-09-12 11:48) - PLID 45335
void CEMRItemAdvPopupWnd::OnSmartStampErase3DControl()
{
	try
	{
		Handle3DStampChange();
		m_pDetail->SetImageTextRemoved();
	}
	NxCatchAll(__FUNCTION__);
}

void CEMRItemAdvPopupWnd::Handle3DStampChange()
{
	if(m_p3DControl == NULL) {
		return;
	}

	m_pDetail->SetImageTextData(m_p3DControl->TextData, true);
}

void CEMRItemAdvPopupWnd::OnTextChangedInkPicture(ETextChangeType TextChangeType)
{
	if(!m_Image) {
		// (r.gonet 05/06/2011) - PLID 43542 - The ink picture is not fully created. Ignore it until it is.
		return;
	}

	// (r.gonet 06/10/2011) - PLID 30359 - Ignore text scaling events, we don't need them in the popup
	if(TextChangeType == tctScalingChanged) {
		return;
	}

	//TES 1/19/2007 - PLID 18159 - Just like OnStrokeInkPicture(), we need to remember the current settings.
	UpdateRememberedPenState();

	//Tell our detail.
	m_pDetail->SetImageTextData(m_Image->TextData);

	//for auditing, track what sort of change happened.
	switch(TextChangeType) {
	case tctStringAdded:
		m_pDetail->SetImageTextAdded();
		break;
	case tctStringRemoved:
		m_pDetail->SetImageTextRemoved();
		break;
	case tctStampModified:
		// (r.gonet 05/02/2012) - PLID 49946 - Currently this is only position, size, and rotation.
		m_pDetail->SetImageStampModified();
		break;
	}
}

//TES 4/6/2007 - PLID 25456 - Copied from CEMRItemAdvNarrativeDlg
void CEMRItemAdvPopupWnd::OnTextChangedRichTextCtrl()
{
	try {
		RequestDetailStateChange(m_RichEditCtrl->RichText);
		//TES 1/15/2008 - PLID 24157 - Notify our parent.
		m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnTextChangedRichTextCtrl");
}

//TES 4/6/2007 - PLID 25456 - Copied from CEMRItemAdvNarrativeDlg
void CEMRItemAdvPopupWnd::OnLinkRichTextCtrl(LPCTSTR strMergeField)
{
	// (c.haag 2008-11-25 11:39) - PLID 31693 - Moved functionality to CEmrItemAdvNarrativeBase
	HandleLinkRichTextCtrl(strMergeField, m_pDetail, GetSafeHwnd(), m_pParentDlg);
}

//TES 4/6/2007 - PLID 25456 - Copied from CEMRItemAdvNarrativeDlg
void CEMRItemAdvPopupWnd::OnRequestFormatRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, LPCTSTR *pstrValue)
{
	try {
		// (c.haag 2007-04-03 17:55) - PLID 25488 - Use GetMergeField for faster, cleaner lookups
		// (c.haag 2007-05-10 15:00) - PLID 25958 - GetMergeField has been replaced with SeekToNarrativeField
		// now that we use recordsets instead of a map
		// (c.haag 2007-08-15 16:32) - PLID 27084 - I had hoped to depreciate this function with the new
		// shared recordset logic, but alas, we can't do it for backwards compatibility reasons. Before 
		// we used recordsets, we would pass a string from a struct object into pstrValue. The struct
		// object persisted in memory; so while it was a horrible design, it wasn't crashy. We don't have 
		// persistent string objects any longer, so the solution here is to allocate one string per 
		// request, and do garbage collection with it.

		// (c.haag 2007-08-15 16:32) - PLID 27084 - Clean up the old string if we have one
		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		/*
		if (m_pstrRequestFormatRichTextCtrlResult) {
			delete m_pstrRequestFormatRichTextCtrlResult;
			m_pstrRequestFormatRichTextCtrlResult = NULL;
		}
		*/

		//ADODB::_RecordsetPtr prs;
		// (j.jones 2008-04-04 12:24) - PLID 29547 - added strFlags as a parameter

		
		// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
		if (m_szRequestFormatBuffer) {
			delete[] m_szRequestFormatBuffer;
			m_szRequestFormatBuffer = NULL;
		}
		
		// (a.walling 2009-11-17 16:31) - PLID 36365
		NarrativeField nf;
		nf.strField = strField;
		nf.strFlags = strFlags;
		m_pDetail->GetValueForNarrative(nf);
		//*pstrValue = new TCHAR[(nf.strValue.GetLength() + 1) * sizeof(TCHAR)];
		m_szRequestFormatBuffer = new TCHAR[(nf.strValue.GetLength() + 1) * sizeof(TCHAR)];
		strcpy((LPTSTR)m_szRequestFormatBuffer, (LPCTSTR)nf.strValue);
		*pstrValue = m_szRequestFormatBuffer;

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		/*
		if (NULL != (prs = m_pDetail->SeekToNarrativeField(strField, strFlags))) {
			if(CString(strFlags).Find("s") != -1) {
				//We want the sentence form.
				// (c.haag 2007-03-29 17:07) - PLID 25423 - Calculate the sentence form,
				// but only once (I think strTmp was for debugging in the past)
				// (c.haag 2007-05-15 11:15) - We have to do a crazy memory leak-causing workaround to satisfy this crazy rich text callback that we'll soon get rid of
				CString* pstrTmp = new CString(GetNarrativeSentenceFormField(prs));
				*pstrValue = *pstrTmp;
				// (c.haag 2007-08-15 16:32) - PLID 27084 - Remember our allocated string for
				// garbage collection
				m_pstrRequestFormatRichTextCtrlResult = pstrTmp;
			}
			else {
				// (c.haag 2007-03-29 17:07) - PLID 25423 - Calculate the value,
				// but only once (I think strTmp was for debugging in the past)
				CString* pstrTmp = new CString(GetNarrativeValueField(prs));
				*pstrValue = *pstrTmp;
				// (c.haag 2007-08-15 16:32) - PLID 27084 - Remember our allocated string for
				// garbage collection
				m_pstrRequestFormatRichTextCtrlResult = pstrTmp;
			}
		}
		*/

	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnRequestFormatRichTextCtrl");
}

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_FORMAT_SENTENCE	41000
#define ID_UNFORMAT_SENTENCE	41001
#define ID_REMOVE_FIELD	41002

//TES 7/6/2012 - PLID 51419 - Split this out so the context menu function can be called by both RTF and HTML narratives.
int CEMRItemAdvPopupWnd::GetNarrativeRightClickFieldOption(const CString &strField, const CString &strFlags, long x, long y)
{
	//Track the menu synchronously, because nIndex isn't guaranteed to stay valid.
	// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
	CNxMenu mnu;
	mnu.CreatePopupMenu();
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Show sentence unless it's a generic field
	BOOL bShowSentenceOption = TRUE;
	if (m_pDetail->m_pParentTopic && m_pDetail->m_pParentTopic->GetParentEMN()) {
		CString strValueDummy;
		bool bIsValidDummy;
		if (m_pDetail->m_pParentTopic->GetParentEMN()->GetGenericMergeFieldValue(strField, strValueDummy, bIsValidDummy)) {
			// this is a generic merge field, no sentence form available
			bShowSentenceOption = FALSE;
		}
	}
	// (c.haag 2007-04-03 17:55) - PLID 25488 - Use GetMergeField for faster, cleaner lookups
	// (c.haag 2007-05-10 15:00) - PLID 25958 - GetMergeField has been replaced with SeekToNarrativeField
	// now that we use recordsets instead of a map

	// (j.jones 2008-04-04 12:24) - PLID 29547 - added strFlags as a parameter
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	/*
	ADODB::_RecordsetPtr prs = m_pDetail->SeekToNarrativeField(strField, strFlags);
	if (NULL != prs) {
		bShowSentenceOption = AdoFldBool(prs, "SentenceFormIsValid");
	}
	*/

	CString strNewFlags;
	if(bShowSentenceOption) {
		if(CString(strFlags).Find("s") == -1) {
			mnu.AppendMenu(MF_BYPOSITION, ID_FORMAT_SENTENCE, "Show Sentence Format");
			strNewFlags = CString(strFlags) + "s";
		}
		else {
			mnu.AppendMenu(MF_BYPOSITION, ID_UNFORMAT_SENTENCE, "Hide Sentence Format");
			strNewFlags = CString(strFlags);
			strNewFlags.Remove('s');
		}
	}

	mnu.AppendMenu(MF_BYPOSITION, ID_REMOVE_FIELD, "Remove Field");
	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, this, NULL);
	mnu.DestroyMenu();
	return nCmdId;
}

//TES 4/6/2007 - PLID 25456 - Copied from CEMRItemAdvNarrativeDlg
void CEMRItemAdvPopupWnd::OnRightClickFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, long nIndex)
{
	try {
		int nCmdId = GetNarrativeRightClickFieldOption(strField, strFlags, x, y);
		if(nCmdId == ID_FORMAT_SENTENCE || nCmdId == ID_UNFORMAT_SENTENCE) {
			CString strNewFlags = strFlags;
			if(strNewFlags.Find("s") == -1) {
				strNewFlags += "s";
			}
			else {
				strNewFlags.Remove('s');
			}
			m_RichEditCtrl->SetMergeFieldFlags(nIndex, _bstr_t(strNewFlags));
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			RequestDetailStateChange(m_RichEditCtrl->RichText);
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		}
		else if(nCmdId == ID_REMOVE_FIELD) {
			m_RichEditCtrl->RemoveMergeFieldByIndex(nIndex);
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			RequestDetailStateChange(m_RichEditCtrl->RichText);
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		}
		
	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnRightClickFieldRichTextCtrl");
}

void CEMRItemAdvPopupWnd::OnRightClickHtmlFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, LPCTSTR strNexGUID)
{
	try {
		//TES 7/6/2012 - PLID 51419 - Separate function for HTML narratives, which identify their fields differently.
		int nCmdID = GetNarrativeRightClickFieldOption(strField, strFlags, x, y);
		if(nCmdID == ID_FORMAT_SENTENCE || nCmdID == ID_UNFORMAT_SENTENCE) {
			CString strNewFlags = strFlags;
			if(strNewFlags.Find("s") == -1) {
				strNewFlags += "s";
			}
			else {
				strNewFlags.Remove('s');
			}
			m_RichEditCtrl->SetMergeFieldFlagsByNexGuid(strNexGUID, _bstr_t(strNewFlags));
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			RequestDetailStateChange(m_RichEditCtrl->RichText);
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		}
		else if(nCmdID == ID_REMOVE_FIELD) {
			m_RichEditCtrl->RemoveMergeFieldByNexGuid(strNexGUID);
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			RequestDetailStateChange(m_RichEditCtrl->RichText);
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-10-05 15:04) - PLID 52986 - Handle right clicking HTML narrative, when selection is not a field.
// (j.armen 2013-03-19 11:38) - PLID 53102 - Now also called for RTF Narratives
void CEMRItemAdvPopupWnd::OnRightClickHtmlRichTextCtrl(long x, long y, bool bCanCopy, bool bCanPaste)
{
	try
	{
		enum {cut = 1, copy, paste, macro};
		CNxMenu mnu;
		mnu.CreatePopupMenu();

		// Can cut when we can copy and are not read only
		mnu.AppendMenu((!m_pDetail->GetReadOnly() && bCanCopy) ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), cut, "Cu&t");
		// Can copy when we can copy
		mnu.AppendMenu(bCanCopy ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), copy, "&Copy");
		// Can paste when we can paste and are not read only
		mnu.AppendMenu((!m_pDetail->GetReadOnly() && bCanPaste) ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), paste, "&Paste");
		mnu.AppendMenu(MF_BYPOSITION|MF_MENUBREAK);
		// Can paste macro text when we are not read only
		mnu.AppendMenu(!m_pDetail->GetReadOnly() ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), macro, "Paste &Macro Text...");

		int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, this, NULL);
		mnu.DestroyMenu();
		switch(nCmdID)
		{
			case cut:
				m_RichEditCtrl->Cut();
				break;
			case copy:
				m_RichEditCtrl->Copy();
				break;
			case paste:
				m_RichEditCtrl->Paste();
				break;
			case macro:
				CEmrTextMacroDlg dlg(this);
				if(IDOK == dlg.DoModal())
					m_RichEditCtrl->ReplaceSel(_bstr_t(dlg.m_strResultTextData), g_cvarTrue);
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code
void CEMRItemAdvPopupWnd::OnCustomStampsChangedInkPicture(LPCTSTR strNewCustomStamps)
{
	try {

		// (j.jones 2010-02-10 14:59) - PLID 37224 - we manage stamp changes in this version of Practice,
		// not the NxInkPicture, so this event should never be received

		/*
		// (a.walling 2008-12-19 09:31) - PLID 29800 - If we are a member of the multi popup, make sure the stamps are refreshed
		// both on the EMN and on the multipopup tree
		if (m_pParentDlg && m_pParentDlg->IsKindOf(RUNTIME_CLASS(CEMRItemAdvMultiPopupDlg))) {
			SetPropertyMemo("EMR_Image_Custom_Stamps", strNewCustomStamps, 0);

			CEMNDetail* pDetail = m_pRealDetail ? m_pRealDetail : m_pDetail;

			if (pDetail && pDetail->m_pParentTopic && pDetail->m_pParentTopic->GetParentEMN() && pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR() && pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface()) {
				(pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface())->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
			}

			CEMRItemAdvMultiPopupDlg* pMultiPopupDlg = (CEMRItemAdvMultiPopupDlg*)m_pParentDlg;
			pMultiPopupDlg->RefreshCustomStamps(strNewCustomStamps, this);
		}
		
		// (a.wetta 2007-04-18 12:35) - PLID 24324 - Reposition everything to make sure that all of the custom stamps are showing
		CallRepositionControls();
		*/

	}NxCatchAll("Error in CEMRItemAdvPopupWnd::OnCustomStampsChangedInkPicture");
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (a.walling 2007-08-22 17:07) - PLID 27160 - Much like the other Reflect*, ensure the interface
// matches up with the internal state.
void CEMRItemAdvPopupWnd::ReflectNarrativeState()
{
	if (m_RichEditCtrl != NULL) {
		CString strPriorNarrative = (LPCTSTR)m_RichEditCtrl->RichText;
		CString strNewNarrative = AsString(m_pDetail->GetState());
		if (strNewNarrative != strPriorNarrative) {
			//TES 11/3/2005 - PLID 18222 - The act of setting the text may change the text, if it's of an old version.
			CString strInitialText = AsString(m_pDetail->GetState());
			m_RichEditCtrl->RichText = _bstr_t(AsString(m_pDetail->GetState()));
			if(CString((LPCTSTR)m_RichEditCtrl->RichText) != strInitialText) {
				//TES 7/25/2007 - PLID 26793 - This was RequestStateChange(); however, that had the additional effect
				// of setting m_bModified to TRUE for this detail, and as you can see by looking at the code and comments
				// in CEMNDetail::EnsureEmrItemAdvDlg(), ReflectCurrentState() shouldn't result in m_bModified being set.
				//NOTE: Since m_bModified is not being set, that now means that the new text will not be saved (unless
				// the detail is otherwise modified).  However, that's OK, because the next time this narrative is loaded
				// this code will get called again, so the narrative will be in exactly the state next time it's loaded
				// as it is this time.
				m_pDetail->SetState(m_RichEditCtrl->RichText);
			}
		}
		m_RichEditCtrl->PutAllowFieldInsert(TRUE);
	}
}

void CEMRItemAdvPopupWnd::ReflectNarrativeContent()
{
	//TES 6/3/2008 - PLID 29098 - Copied from CEMRItemAdvNarrativeDlg::ReflectCurrentContent().
	if (NULL != m_RichEditCtrl) {
		// (c.haag 2007-05-14 11:37) - PLID 25970 - Now that we are the sole maintainer of the
		// merge field list, this function is depreciated in the context of adding fields to the
		// form control. However, we need to make sure the visible text is up to date.

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
		//m_RichEditCtrl->ParseMergeFields();
		m_pDetail->UpdateNarrativeFields(this);
	}
}

void CEMRItemAdvPopupWnd::UpdateRichTextAppearance()
{
	//
	// (c.haag 2007-10-04 15:46) - PLID 26858 - This function ensures that the
	// rich text and the narrative merge fields are "in synchronization". You 
	// should call this function if this detail's narrative merge field list is
	// maintained by an EMN (rather than by the detail), and the merge field list
	// was modified.
	//
	/*
	if(IsWindow(GetSafeHwnd())) {
		if(NULL != m_RichEditCtrl) {
			CString strRichTextBeforeRemoval = (LPCTSTR)m_RichEditCtrl->RichText;

			m_RichEditCtrl->ParseMergeFields();

			if (strRichTextBeforeRemoval != CString((LPCTSTR)m_RichEditCtrl->RichText)) {
				m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
				//TES 1/15/2008 - PLID 24157 - Notify our parent.
				m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
			}
		}
	}
	*/

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
	m_pDetail->UpdateNarrativeFields(this);	
}

// (j.jones 2008-01-14 10:06) - PLID 18709 - added OnRequestLWMergeFieldData
void CEMRItemAdvPopupWnd::OnRequestLWMergeFieldData()
{
	try {

		//when we get this message from the richedit control, we can assume
		//that the richedit control has set LWMergeDataNeeded = TRUE in the
		//narrative recordset for at least one record, possibly multiple

		//NarrativeRequestLWMergeFieldData will loop through the recordset,
		//grab all the merge fields we need to load, load them, populate
		//the recordset, then set LWMergeDataNeeded = FALSE

		
		// (a.walling 2009-11-19 15:08) - PLID 36365 - the control may have new fields and etc in its own internal state, so pass in the most up to date state
		NarrativeRequestLWMergeFieldData(m_pDetail, (LPCTSTR)m_RichEditCtrl->RichText);


		// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
		m_pDetail->UpdateNarrativeFields(this, (LPCTSTR)m_RichEditCtrl->RichText);

	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnRequestLWMergeFieldData");
}

void CEMRItemAdvPopupWnd::SetHilightColor(long clrHilight)
{
	//TES 1/15/2008 - PLID 24157 - This function should be called to set the hilight color if the window is already
	// created, it will recreate the hilight brush and redraw the screen using it, if necessary.
	if(clrHilight != m_clrHilightColor) {
		m_clrHilightColor = clrHilight;
		if (m_brHilight.m_hObject) {
			m_brHilight.DeleteObject();
		}
		// If the new color is not 0 then create the new brush		
		if (m_clrHilightColor != 0) {
			if (m_clrHilightColor & 0x80000000) {
				m_brHilight.CreateSysColorBrush(m_clrHilightColor & 0x000000FF);
			} else {
				m_brHilight.CreateSolidBrush(PaletteColor(m_clrHilightColor));
			}
		}
		Invalidate();
	}
}

//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
void CEMRItemAdvPopupWnd::OnEditingFinishingTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	// (c.haag 2008-10-20 12:32) - PLID 31700 - Moved logic to EmrItemAdvTableBase
	HandleEditingFinishingTable(lpRow, nCol, varOldValue, strUserEntered, pvarNewValue, pbCommit, pbContinue, this, m_pDetail);
}

// (a.walling 2008-01-16 15:25) - PLID 14982
// The general idea for this was taken from code in CEmrItemAdvDlg that calls CEmrItemEntryDlg
void CEMRItemAdvPopupWnd::OnEditItem()
{
	try {
		// (c.haag 2008-01-29 12:12) - PLID 28686 - The code to invoke the edit dialog used to be here;
		// now moved to its own utility function
		OpenItemEntryDlg();

	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnEditItem");
}

void CEMRItemAdvPopupWnd::ReflectCurrentState()
{
	//TES 1/23/2008 - PLID 24157 - Added this function, needed a master function for reflecting state changes.
	switch(m_pDetail->m_EMRInfoType) {
	case eitText:
		ReflectTextState();
		break;
	case eitSingleList:
	case eitMultiList:
		ReflectListState();
		break;
	case eitImage:
		ReflectImageState();
		break;
	case eitSlider:
		ReflectSliderState();
		break;
	case eitNarrative:
		ReflectNarrativeState();
		break;
	case eitTable:
		ReflectTableState();
		break;
	}
}

//TES 1/23/2008 - PLID 24157 - Added, to be called by ReflectCurrentState().
void CEMRItemAdvPopupWnd::ReflectTextState()
{
	//TES 1/23/2008 - PLID 24157 - Update our edit box.
	CString strOldText;
	m_edit.GetWindowText(strOldText);
	CString strNewText = AsString(m_pDetail->GetState());
	if(strOldText != strNewText) {
		m_edit.SetWindowText(strNewText);
	}
}

//TES 1/23/2008 - PLID 24157 - Added, to be called by ReflectCurrentState().
void CEMRItemAdvPopupWnd::ReflectSliderState()
{
	//TES 1/23/2008 - PLID 24157 - Update our slider position, and its caption.
	m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
	m_Caption.SetWindowText(AsString(m_pDetail->GetState()));	
}

LRESULT CEMRItemAdvPopupWnd::OnAddNewDropdownColumnSelection(WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2008-01-29 12:09) - PLID 28686 - This function is called when the user elects
	// to add a new selection in the embedded combo of a dropdown column. The Emr Item Edit
	// dialog will open, and take the user straight into the dropdown selection list configuration.
	try {
		CWaitCursor wc;
		const long nDetailCol = (long)wParam;
		TableColumn tc = m_pDetail->GetColumn(nDetailCol);
		OpenItemEntryDlg( eEmrItemEntryDlgBehavior_AddNewDropdownColumnSelection, (LPVOID)tc.nID );
	}
	NxCatchAll("Error in CEMRItemAdvPopupWnd::OnAddNewDropdownColumnSelection");
	return 0;
}

void CEMRItemAdvPopupWnd::OpenItemEntryDlg(EEmrItemEntryDlgBehavior behavior /*= eEmrItemEntryDlgBehavior_Normal */, LPVOID pBehaviorData /* = NULL */)
{
	// (c.haag 2008-01-29 12:14) - PLID 28686 - EMR item entry dialog code has been moved into its own
	// utility function. 14982 is the responsible item for the actual code below.

	if (m_pRealDetail == NULL) {
		ASSERT(FALSE);
		ThrowNxException("CEMRItemAdvPopupWnd::OpenItemEntryDlg where m_pRealDetail is NULL!");
		return;
	}

	// (c.haag 2006-04-04 11:06) - PLID 19890 - We consider permissions now
	if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
		return;
	}

	if(!m_pDetail->IsActiveInfo()) {

		// (j.jones 2010-02-17 17:30) - PLID 37318 - if a detail is part of a SmartStamp image/table connection, you cannot "bring up to date",
		// so even though that's not an option here, don't mislead them into thinking it's allowed at all
		// (d.thompson 2010-08-30) - PLID 40107 - Fixed typo in msg box
		// (z.manning 2011-01-21 09:50) - PLID 42338 - Support multiple images per smart stamp table
		if(m_pDetail->GetSmartStampImageDetails()->GetCount() > 0 || m_pDetail->GetSmartStampTableDetail() != NULL) {
			MessageBox("The detail you are editing is on an outdated version, and cannot be edited, because it has been changed since it was added to this EMN.\n"
				"The ability to bring an item up to date is not allowed on SmartStamp detail items. You will need to remove then re-add the SmartStamp image and table if you wish to use a newer version.", NULL, MB_OK | MB_ICONHAND);
			return;
		}

		MessageBox("The detail you are editing is on an outdated version, and cannot currently be edited, because it has been changed since it was added to this EMN.  "
			"You can bring this item up to date when not popped-up by right clicking on it and choosing 'Bring item up to date.'", NULL, MB_OK | MB_ICONHAND);
		return;
	}

	CEmrItemEntryDlg dlg(this);
	if(m_pRealDetail->m_pParentTopic->GetParentEMN()) {
		// (a.walling 2008-01-16 16:08) - PLID 14982
		dlg.SetCurrentEMN(m_pRealDetail->m_pParentTopic->GetParentEMN());
	}

	// (j.jones 2006-08-28 14:50) - PLID 22220 - if not a template, tell the dialog
	// we're editing from a given detail ID
	if(!m_pRealDetail->m_bIsTemplateDetail) {
		// (z.manning 2011-04-28 15:00) - PLID 37604 - We now pass in the detail pointer.
		dlg.m_pCalledFromDetail = m_pRealDetail;
	}

	dlg.m_bIsEditingOnEMN = !m_pRealDetail->m_bIsTemplateDetail;
	dlg.m_bIsEditingOnTemplate = m_pRealDetail->m_bIsTemplateDetail;

	// (z.manning, 08/29/05, PLID 17249)
	// By populating the current list, we tell the EMR item entry dlg to maintain
	// both the current and admin lists for single and multi selects.
	// List type of 1 corresponds to normal data list element (all other types are table-related).

	// (j.jones 2006-02-09 16:27) - only set this field if we are editing an existing list,
	// because the code will reload values from the admin list, not a current list
	// (a.walling 2008-01-18 09:33) - PLID 14982 - We will get values from the popped up detail rather than the existing
	// one on the chart.
	if(m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList) {
		dlg.m_bMaintainCurrentList = TRUE;

		//TES 2/13/2006 - Track which ones are currently selected.
		CDWordArray dwaSelected;
		m_pDetail->GetSelectedValues(dwaSelected);
		// (c.haag 2006-03-03 12:57) - PLID 19557 - We should have as many resulting records as we
		// have list element ID's. Otherwise, the Current List in EmrItemEntryDlg and the detail on
		// the EMN will look different.
		// (a.walling 2008-02-07 10:52) - PLID 14982 - Also check the data group id so the entry dialog can know if it is selected on the topic (not popped up)
		int nDetailListElements = m_pDetail->GetListElementCount();
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset("SELECT Data, CASE WHEN ID IN ({INTSTRING}) THEN 1 ELSE 0 END AS Selected, EmrDataGroupID "
			"FROM EmrDataT WHERE ID IN ({INTSTRING}) AND ListType = 1", ArrayAsString(dwaSelected), m_pDetail->GetListElementIDsCommaDelimited());
		ASSERT(nDetailListElements == prs->GetRecordCount()); // Warn the developer if they don't match up
		while(!prs->eof) {
			CurrentListItem cli;
			cli.strData = AdoFldString(prs, "Data");
			cli.bSelected = AdoFldLong(prs, "Selected") ? true : false;
			cli.bSelectedOnTopic = IsIDInArray(AdoFldLong(prs, "EmrDataGroupID"), &m_dwaRealSelectedDataGroupIDs);
			dlg.m_aryCurrentList.Add(cli);
			prs->MoveNext();
		}
	} else if (m_pDetail->m_EMRInfoType == eitTable) {
		//
		// (c.haag 2006-03-06 16:46) - PLID 19580 - If we are opening the EmrItemEntryDlg for a
		// table item that's already in an open EMR, we need to get the state for every detail
		// in the entire EMR that has the same EmrInfoID as this item, and store them all in
		// an array that we pass into the EmrItemEntryDlg.
		//
		// The reason for this is that if a user wants to change the data type of a table column,
		// we need to check each existing detail to see if there is data in that column. If there
		// is, the user must be prevented from changing the column type.
		//
		// (a.walling 2008-01-16 16:10) - PLID 14982
		CEMRTopic *pTopic = m_pRealDetail->m_pParentTopic;
		CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
		dlg.m_bMaintainCurrentTable = TRUE;

		if (NULL != pEMR) {
			int nEMNs = pEMR->GetEMNCount();
			for (int i=0; i < nEMNs; i++) {
				pEMN = pEMR->GetEMN(i);
				int nTopics = pEMN->GetTopicCount();
				for (int j=0; j < nTopics; j++) {
					//
					// Go through each topic looking for details with the same EmrInfoID, and then add
					// them to dlg.m_apCurrentTableDetails. This function is recursive since topics
					// can have subtopics.
					//
					BuildCurrentTableStateArray(pEMN->GetTopic(j), m_pDetail->m_nEMRInfoID, dlg.m_apCurrentTableDetails);
				}
			}
		} else {
			ASSERT(FALSE);
		}
		
		// (a.walling 2008-03-05 17:19) - PLID 14982 - We need to also ensure our popped up detail is here
		// so we can limit deletions of dropdown columns and etc if the selections do not match on the popup
		// vs the detail on the topic.
		BOOL bDetailInArray = FALSE;
		for (int i = 0; i < dlg.m_apCurrentTableDetails.GetSize(); i++) {
			if (dlg.m_apCurrentTableDetails[i] == m_pDetail) {
				bDetailInArray = TRUE;
				break;
			}
		}

		if (!bDetailInArray) {
			dlg.m_apCurrentTableDetails.Add(m_pDetail);
		}
	}
	else {
		//DRT 3/3/2008 - PLID 28603 - Can't allow hotspots to be deleted now either.  This is the almost the same code as CEMRItemAdvDlg
		dlg.m_bMaintainCurrentImage = true;

		//Now send it all our details for this info ID.  Works the same as tables above recursively.
		CEMRTopic *pTopic = m_pRealDetail->m_pParentTopic;
		CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
		if (NULL != pEMR) {
			int nEMNs = pEMR->GetEMNCount();
			for (int i=0; i < nEMNs; i++) {
				pEMN = pEMR->GetEMN(i);
				int nTopics = pEMN->GetTopicCount();
				for (int j=0; j < nTopics; j++) {
					//
					// Go through each topic looking for details with the same EmrInfoID, and then add
					// them to dlg.m_aryCurrentImageDetails. This function is recursive since topics
					// can have subtopics.
					//
					BuildCurrentImageArray(pEMN->GetTopic(j), m_pDetail->m_nEMRInfoID, dlg.m_aryCurrentImageDetails);
				}
			}
		} else {
			ASSERT(FALSE);
		}
	}
	dlg.m_bIsCurrentDetailTemplate = FALSE; // (a.walling 2008-01-16 15:37) - PLID 14982 - We are never on a template
	//Remember the current InfoID, so we'll be able to tell if it changed.
	long nInfoID = m_pDetail->m_nEMRInfoID;

	// (a.walling 2008-01-18 13:23) - PLID 14982 - Prevent the user from changing the type
	dlg.m_bPreventTypeChange = TRUE;
	// (z.manning 2010-03-18 14:16) - PLID 37318 - Prevent smart stamp changes on popped up details.
	dlg.m_bPreventSmartStampTableChange = TRUE;

	BOOL bForceSaveTopic = FALSE;

	// (c.haag 2008-01-22 11:23) - PLID 28686 - Include behavior information
	if(IDOK == dlg.OpenWithMasterID(m_pDetail->m_nEMRInfoMasterID, behavior, pBehaviorData)) {

		long nDataType = dlg.GetDataType();

		//RefreshContent on all copies of this EMRInfo item on the entire EMR, not just at a topic or EMN level

		// (a.walling 2008-01-18 09:34) - PLID 14982 - We will never update from the dialog, since we will have to call
		// LoadContent anyway.

		// (j.jones 2006-08-17 15:26) - PLID 22078 - It is possible we edited an item that
		// exists on a saved EMN and a new item was created, so we must see if any items
		// should switch to use the new version of that item.
		// This function will update the info ID if the detail is unsaved OR if this is a template
		if(nInfoID != dlg.GetID()) {
				
			// (j.jones 2006-08-28 14:51) - PLID 22220 - also call UpdateInfoID for this detail only, if saved
			if(!m_pDetail->m_bIsTemplateDetail && m_pDetail->m_nEMRDetailID != -1) {
				//I realize it looks silly to call this for the detail, then pass in the detail ID, but I made
				//UpdateInfoID have a check to absolutely not run unless it is a template detail or unsaved detail,
				//so passing in the ID is a confirmation that we definitely want to update

				m_pDetail->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap(), m_pDetail->m_nEMRDetailID);

				// (a.walling 2008-01-18 09:42) - PLID 14982 - Updates the info ID for the real detail, if m_pDetail was a copy
				if (m_pDetail != m_pRealDetail && m_pRealDetail != NULL) {
					m_pRealDetail->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap(), m_pRealDetail->m_nEMRDetailID);
				}	

				// (j.jones 2010-11-11 17:59) - PLID 29075 - force the topic to save
				bForceSaveTopic = TRUE;
			}

			// (a.walling 2008-01-18 09:43) - PLID 14982 - If m_pDetail is a copy, this has no real effect, but also has no bad side effects.
			m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap());

			// (a.walling 2008-01-18 09:42) - PLID 14982 - Updates the info ID for the entire EMR, if m_pDetail was a copy
			if (m_pDetail != m_pRealDetail && m_pRealDetail != NULL) {
				m_pRealDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap());
			}
		}

		// (c.haag 2006-03-31 11:13) - PLID 19387 - The TRUE parameter means that we want to call
		// SyncContentAndState at the end of LoadContent
		//
		// (j.jones 2006-08-17 15:52) - PLID 22078 - be sure to call RefreshContent on the EMRInfoID
		// now set in the EmrItemEntryDlg. It may be the same InfoID we started with, but if it's not,
		// it would mean we didn't update that InfoID anyways
		// (j.jones 2007-07-26 09:20) - PLID 24686 - converted to a ByInfoID function name
		m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->RefreshContentByInfoID(dlg.GetID(), TRUE);

		// (a.walling 2008-01-18 09:44) - PLID 14982 - Sets flags to refresh content for these info items on the EMR, if m_pDetail was copy
		if (m_pDetail != m_pRealDetail && m_pRealDetail != NULL) {
			m_pRealDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->RefreshContentByInfoID(dlg.GetID(), TRUE);
		}

		// (a.walling 2008-01-29 15:05) - PLID 14982
		//m_pDetail->LoadContent();
		// will be called in PostEmrItemEntryDlgItemSaved. Since we are guaranteed that the item type will not change,
		// it is safe to call the function directly, rather than rely on the message.

		//TES 2/15/2008 - PLID 28942 - It's no good to update the SQL if we're going to keep pulling the dropdown
		// lists from our map, so first clear that map out.
		ClearDropdownSourceInfoMap();

		// pass FALSE to let PostEmrItemEntryDlgItemSaved know we are calling from code, not in response to a message
		PostEmrItemEntryDlgItemSaved(m_pDetail, FALSE);
		if(m_pDetail->m_EMRInfoType == eitTable) {
			// (a.walling 2008-01-29 15:05) - PLID 14982 - Ensure the Sql is up to date for the item before attempting to create controls
			m_pDetail->CalcComboSql();
		}

		// (a.walling 2008-01-18 09:46) - PLID 14982 - Handles post-processing of the modified detail. Only if m_pDetail was a copy.
		if (m_pDetail != m_pRealDetail && m_pRealDetail != NULL) {
			// pass FALSE to let PostEmrItemEntryDlgItemSaved know we are calling from code, not in response to a message
			PostEmrItemEntryDlgItemSaved(m_pRealDetail, FALSE);
			if(m_pRealDetail->m_EMRInfoType == eitTable) {
				// (a.walling 2008-01-29 15:05) - PLID 14982 - Ensure the Sql is up to date for the item before attempting to create controls
				m_pRealDetail->CalcComboSql();
			}
		}

		CreateControls();

		// (j.jones 2010-11-11 17:26) - PLID 29075 - force the topic to save
		if(bForceSaveTopic) {
			CEmrTreeWnd* pTreeWnd = m_pRealDetail->GetParentEMN()->GetInterface();
			if(pTreeWnd) {
				pTreeWnd->SaveEMR(esotTopic, (long)m_pRealDetail->m_pParentTopic, TRUE);
			}
		}

	} else {
		// (a.walling 2009-06-30 16:39) - PLID 34759 - For safety's sake, ensure the IsActiveInfo properties are invalidated
		if (m_pDetail) {
			m_pDetail->RefreshIsActiveInfo();
		}
		if (m_pRealDetail) {
			m_pRealDetail->RefreshIsActiveInfo();
		}
	}
}

// (z.manning, 01/22/2008) - PLID 28690 - Added message handler for clicking on hot spots.
void CEMRItemAdvPopupWnd::OnClickedHotSpotInkPicture(long nHotSpotID)
{
	try
	{
		PostMessage(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, nHotSpotID);

	}NxCatchAll("CEMRItemAdvPopupWnd::OnClickedHotSpotInkPicture");
}

// (z.manning 2011-07-25 12:04) - PLID 44649
void CEMRItemAdvPopupWnd::OnClickedHotSpot3DControl(short n3DHotSpotID)
{
	try
	{
		CEMRHotSpot *pHotSpot = m_pDetail->GetHotSpotArray()->FindBy3DHotSpotID(n3DHotSpotID);
		if(pHotSpot != NULL) {
			PostMessage(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, pHotSpot->GetID());
		}
	}
	NxCatchAll(__FUNCTION__);
}

LRESULT CEMRItemAdvPopupWnd::OnToggleHotSpot(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning 2010-05-05 16:38) - PLID 38503 - We need to remember these before we change the state
		UpdateRememberedPenState();

		// (c.haag 2008-01-24 10:50) - PLID 28690 - Handle image hot spot toggles
		const long nHotSpotID = (const long)wParam;
		m_pDetail->ToggleImageHotSpotSelection(nHotSpotID);

		ReflectImageState();

	}NxCatchAll("CEMRItemAdvPopupWnd::OnToggleHotSpot");
	return 0;
}

void CEMRItemAdvPopupWnd::SetMaxSize(CSize szMax)
{
	//TES 2/21/2008 - PLID 28827 - This can now be changed after initialization.
	m_szMax = szMax;
}

void CEMRItemAdvPopupWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if(bShow) {
		if(m_wndImage.m_hWnd != NULL && IsWindow(m_wndImage.m_hWnd)) {
			//TES 2/26/2008 - PLID 27721 - Setting the focus here ensures that the image gets sized properly if it stretches
			// off the edge of the screen; I'm not sure why, apart from it being something to do with changing the order in
			// which various windows/COleControl messages are handled.
			m_wndImage.SetFocus();
		}
	}

	CWnd::OnShowWindow(bShow, nStatus);

	//TES 3/25/2008 - PLID 24157 - If an invisible control has focus, then MFC will lock up when it gets any input
	// messages (keystrokes, generally), as it tries to route them to the control that has focus.  Therefore, since we
	// have just hidden ourselves, make sure that none of our controls has focus.
	if(!bShow) {
		HWND hwndFocus = ::GetFocus();
		if(::IsWindowDescendent(m_hWnd, hwndFocus)) {
			::SetFocus(NULL);
		}
	}
}


// (j.jones 2008-06-05 10:07) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
// row and column in question
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
//	Also updating comments.  This exists as a PostMessage handler because you cannot call StartEditing on 1 cell
//	from within the OnEditingFinished of another cell, or the datalist doesn't allow you to ever really commit
//	the 2nd edit.
// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
LRESULT CEMRItemAdvPopupWnd::OnStartEditingEMRTable(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2008-10-15 16:25) - PLID 31700 - The core logic is now in EmrItemAdvTableBase
		StartEditingEMRTable(wParam, lParam);
	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnStartEditingEMRTable");
	return 0;
}

//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
void CEMRItemAdvPopupWnd::OnEditingStartingTable(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	// (c.haag 2008-10-20 09:48) - PLID 31700 - The core logic is now in EmrItemAdvTableBase
	HandleEditingStartingTable(lpRow, nCol, pvarValue, pbContinue, m_pDetail);
}

// (r.gonet 02/14/2013) - PLID 40017 - Added to record column resizes since we can now save the popped up table's widths separately.
void CEMRItemAdvPopupWnd::OnColumnSizingFinishedTable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		if(!m_bRememberPoppedUpTableColumnWidths) {
			// (r.gonet 02/14/2013) - PLID 40017 - We don't want to remember the column widths in the popped up table,
			//  so quit now before we change the detail's column widths data structure.
			return;
		}

		if(m_pDetail->m_EMRInfoType != eitTable) {
			return;
		}

		// Update detail member variable
		m_pDetail->SetSaveTableColumnWidths(TRUE);
		// (r.gonet 02/14/2013) - PLID 40017 - We pass TRUE to UpdateColumnStyle to signify that this is a popup so it can save the widths to the right variables.
		// We can't rely on m_pDetail's m_bIsPoppedUp value becasue in multipopups, the contained detail is actually the real detail and not a copy. So its 
		//  m_bIsPopup's value is FALSE even though it is in a popup.
		UpdateColumnStyle(m_pDetail, TRUE);
		// (m.hancock 2006-06-19 14:48) - PLID 20929 - Set the detail and topic as unsaved, 
		// then tell the interface to reflect that.
		// (j.jones 2006-09-01 15:33) - PLID 22380 - only if unlocked
		// (a.walling 2008-08-21 17:20) - PLID 23138 - also only if writable
		// (r.gonet 02/14/2013) - PLID 40017 - In multipopups, we work on the real detail rather than a copy, so save that now.
		if(m_pDetail->m_pParentTopic->GetParentEMN()->GetStatus() != 2 && m_pDetail->m_pParentTopic->GetParentEMN()->IsWritable()) {
			m_pDetail->SetUnsaved();
			m_pDetail->m_pParentTopic->SetUnsaved();
			GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);	
		}
	} NxCatchAll("Error in CEMRItemAdvPopupWnd::OnColumnSizingFinished");
}

// (c.haag 2008-10-21 09:54) - PLID 31700 - This was merged into EmrItemAdvTableBase, but outside callers need to access
// the function as well.
void CEMRItemAdvPopupWnd::UpdateLinkedItemList(CEMNDetail* pDetail)
{
	CEmrItemAdvTableBase::UpdateLinkedItemList(pDetail, TRUE);
}

// (z.manning 2008-12-16 12:57) - PLID 27682
void CEMRItemAdvPopupWnd::OnTextSpellCheck()
{
	try
	{
		SpellCheckEditControl(this, &m_edit);

	}NxCatchAll("CEMRItemAdvPopupWnd::OnTextSpellCheck");
}

// (a.walling 2008-12-19 09:30) - PLID 29800 - Set custom stamps on the image
void CEMRItemAdvPopupWnd::SetCustomStamps(LPCTSTR szNewStamps)
{
	try {
		CEMNDetail* pDetail = m_pRealDetail ? m_pRealDetail : m_pDetail;

		if (pDetail->m_EMRInfoType == eitImage)
		{
			if(m_Image != NULL) {
				m_Image->CustomStamps = _bstr_t(szNewStamps);
				// (a.wetta 2007-04-18 12:35) - PLID 24324 - Reposition everything to make sure that all of the custom stamps are showing
				CallRepositionControls();
			}
			else if(m_p3DControl != NULL) {
				// (z.manning 2011-09-07 16:56) - PLID 44693
				m_p3DControl->CustomStamps = _bstr_t(szNewStamps);
				CallRepositionControls();
			}
		}
	} NxCatchAll("CEMRItemAdvPopupWnd::SetCustomStamps");
}

// (r.gonet 02/14/2012) - PLID 37682 - Sets the image up with a filter
void CEMRItemAdvPopupWnd::SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter)
{
	_variant_t varData;
	if(pTextStringFilter == NULL) {
		// (r.gonet 02/14/2012) - PLID 37682 - Pass an empty to clear out the filter
		varData = GetVariantEmpty();
	} else {
		varData = pTextStringFilter->GetAsVariant();
	}
	if(m_Image != NULL) {
		m_Image->PutTextStringFilterData(varData);
	}
	else if(m_p3DControl != NULL) {
		m_p3DControl->PutTextStringFilterData(varData);
	}
}

// (r.gonet 02/14/2013) - PLID 40017 - Sets the flag to remember the column widths in a popped up table.
void CEMRItemAdvPopupWnd::SetRememberPoppedUpTableColumnWidths(BOOL bRemember)
{
	m_bRememberPoppedUpTableColumnWidths = bRemember;
}

// (z.manning 2009-09-21) - PLID 33612 - Open the lab entry dialog for the lab associated with this detail
// (Copied from CEmrItemAdvTextDlg)
void CEMRItemAdvPopupWnd::OnLabButton()
{
	try
	{
		// (z.manning 2009-09-23 09:54) - PLID 33612 - Send a message to the popup dialog letting
		// it know to close this and open the lab entry dialog.
		m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);

	}NxCatchAll("CEMRItemAdvPopupWnd::OnLabButton");
}

// (j.jones 2009-10-02 11:14) - PLID 35161 - added ability to right-click on a table
void CEMRItemAdvPopupWnd::OnRButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);

		SetTableCurSel(pRow);
	} NxCatchAll(__FUNCTION__)
}

// (a.walling 2012-11-06 11:56) - PLID 53609 - Display context menu during datalist ShowContextMenu event
void CEMRItemAdvPopupWnd::OnShowContextMenuTable(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		SetTableCurSel(pRow);

		//we only want to use the menu if we are right clicking on a cell in a selected row
		if(pRow != NULL && nCol > 0) {

			*pbContinue = VARIANT_FALSE;

			int nMenuCmd = -1;
			long nDetailRow, nDetailCol;
			TablePositionToDetailPosition(m_pDetail, LookupTableRowIndexFromDatalistRow(pRow), nCol, nDetailRow, nDetailCol);
			TableColumn tc = m_pDetail->GetColumn(nDetailCol);
			TableColumn* ptc = m_pDetail->GetColumnPtr(nDetailCol);
			TableRow* ptr = m_pDetail->GetRowPtr(nDetailRow);
			long nColID = tc.nID;

			// (j.jones 2010-05-05 10:55) - PLID 38414 - added the ability to check/uncheck all boxes on a table,
			// this previously existed on table items but not the popup
			if (tc.nType == LIST_TYPE_CHECKBOX) {

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();

				BOOL bUncheck = FALSE;
				long menuCheckAll = 0xad;
				long menuUncheckAll = 0xae;
				int i = 0;
				int nRows = m_pDetail->GetRowCount();

				// we want to inspect the rows to determine what options are valid for the user
				// ensure this is not the medications or allergy item; seems like an extremely dangerous idea
				// to allow them to be mass-updated like this.
				if (!m_pDetail->IsCurrentMedicationsTable() && !m_pDetail->IsAllergiesTable()) {

					if (nRows > 1) {
						// should have at least two rows for this. Now we need to inspect the rows to see how many are checked and unchecked.
						TableElement te;

						long nChecked = 0;
						long nUnchecked = 0;

						const int nTableElements = m_pDetail->GetTableElementCount();
						for(i = 0; i < nTableElements; i++) {
							TableElement teTmp;
							m_pDetail->GetTableElementByIndex(i, teTmp);
							if (teTmp.m_pColumn->nID == tc.nID) { // matches column
								if (teTmp.m_bChecked)
									nChecked++;
							}
						}

						nUnchecked = nRows - nChecked;

						//MF_DISABLED items appear as gray and disabled on Vista,  but they appear normal on XP/2000.
						//So ensure that we are using MF_GRAYED as well.
						DWORD dwEnabledFlags = MF_BYPOSITION|MF_ENABLED;
						DWORD dwDisabledFlags = MF_BYPOSITION|MF_DISABLED|MF_GRAYED;

						pMenu.InsertMenu(0, dwDisabledFlags, menuCheckAll-2, FormatString("%li box%s checked", nChecked, nChecked == 1 ? "" : "es"));
						pMenu.InsertMenu(1, MF_SEPARATOR, menuCheckAll-1, (LPCTSTR)NULL);
						pMenu.InsertMenu(2, nUnchecked == 0 ? dwDisabledFlags : dwEnabledFlags, menuCheckAll, "&Check All");
						pMenu.InsertMenu(3, nChecked == 0 ? dwDisabledFlags : dwEnabledFlags, menuUncheckAll, "&Uncheck All");
						pMenu.AppendMenu(MF_SEPARATOR);
					}
				}

				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				AppendSharedMenuOptions(&pMenu, pRow, nCol, m_pDetail);

				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				
				if(nMenuCmd == menuCheckAll || nMenuCmd == menuUncheckAll)
				{
					pMenu.DestroyMenu();
					CWaitCursor cws;

					if (nMenuCmd == menuUncheckAll) 
						bUncheck = TRUE;
					else bUncheck = FALSE;

					// alright, let's modify the table
					CMap<__int64, __int64, long, long> mapTable;
					const int nTableElements = m_pDetail->GetTableElementCount();
					for(i = 0; i < nTableElements; i++) {
						TableElement teTmp;
						m_pDetail->GetTableElementByIndex(i, teTmp);
						__int64 nKey = (((__int64)teTmp.m_pRow) << 32) + (__int64)teTmp.m_pColumn->nID;
						mapTable[nKey] = i;
						//TRACE("%s", FormatString("adding key %I64i (row %I64i, col %I64i)\n", nKey, __int64(teTmp.m_pRow->nID), __int64(teTmp.m_pColumn->nID)));
					}

					// alright, let's modify the table
					long nCols = m_pDetail->GetColumnCount();

					CArray<TableElement, TableElement> arNewTableElements;

					for (int iRow = 0; iRow < nRows; iRow++)
					{
						TableRow *pTableRow = m_pDetail->GetRowPtr(iRow);

						__int64 nnKey = (__int64(pTableRow) << 32) + __int64(tc.nID);
						//TRACE("%s", FormatString("looking for key %I64i (row %I64i, col %I64i)... ", nnKey, __int64(pTableRow->nID), __int64(tc.nID)));

						long nTableIndex = -1;

						if (mapTable.Lookup(nnKey, nTableIndex))
						{
							//TRACE("found\n");
							TableElement teMod = *(m_pDetail->GetTableElementPtrByIndex(nTableIndex));

							// (z.manning 2011-03-23 09:03) - PLID 30608 - If this was previously unchecked and we are now
							// checking it then we need to update any autofill columns.
							if(!teMod.m_bChecked && !bUncheck) {
								m_pDetail->UpdateAutofillColumns(&teMod);
							}
							//TES 7/22/2011 - PLID 42098 - Don't check labels
							if(!teMod.m_pRow->m_bIsLabel) {
								teMod.m_bChecked = !bUncheck;
							}
							m_pDetail->SetTableElement(teMod, TRUE, FALSE);

							if (teMod.m_bChecked && tc.bIsGrouped) {
								// grouped! we need to check for any others to uncheck.
								for (int iCol = 0; iCol < nCols; iCol++) {
									TableColumn* pTableCol = m_pDetail->GetColumnPtr(iCol);

									if ( (pTableCol->nID != tc.nID) && (pTableCol->bIsGrouped) ) {
										nnKey = (__int64(pTableRow) << 32) + __int64(pTableCol->nID);
										//TRACE("%s", FormatString("grouping key %I64i (row %I64i, col %I64i)... ", nnKey, __int64(pTableRow->nID), __int64(pTableCol->nID)));

										long nGroupedElementIndex;
										if (mapTable.Lookup(nnKey, nGroupedElementIndex)) {
											//TRACE("found\n");
											// we found one, unset it!
											TableElement& teGrouped = *(m_pDetail->GetTableElementPtrByIndex(nGroupedElementIndex));

											teGrouped.LoadValueFromVariant(g_cvarEmpty, NULL);
										} else {
											//TRACE("not found\n");
											// if it was checked, it should exist in the map, so ignore.
										}
									}
								}
							}
						}
						else if (!bUncheck)
						{
							//TRACE("not found\n");
							// if we are unchecking, we don't care if we can't find the element in the map, since if it
							// is not in the map it is already unchecked. However, if we are checking, we need to create
							// the table element.

							// create and add the new checked table element
							TableElement teNew;
							teNew.m_pColumn = const_cast<TableColumn*>(ptc);
							teNew.m_pRow = const_cast<TableRow*>(pTableRow);
							//TES 7/22/2011 - PLID 42098 - Don't check labels
							if(!teNew.m_pRow->m_bIsLabel) {
								teNew.m_bChecked = TRUE;
							}
							arNewTableElements.Add(teNew);

							// (z.manning 2011-03-23 09:03) - PLID 30608 - This was previously unchecked and we are now
							// checking it so we need to update any autofill columns.
							m_pDetail->UpdateAutofillColumns(&teNew);

							// uncheck any grouped items
							if (tc.bIsGrouped) {
								for (int iCol = 0; iCol < nCols; iCol++) {
									TableColumn* pTableCol = m_pDetail->GetColumnPtr(iCol);

									if ( (pTableCol->nID != tc.nID) && (pTableCol->bIsGrouped) ) {
										nnKey = (__int64(pTableRow) << 32) + pTableCol->nID;

										long nGroupedElementIndex;
										if (mapTable.Lookup(nnKey, nGroupedElementIndex)) {
											// we found one, uncheck it!
											TableElement& teGrouped = *(m_pDetail->GetTableElementPtrByIndex(nGroupedElementIndex));

											teGrouped.LoadValueFromVariant(g_cvarEmpty, NULL);
										} else {
											// create the grouped item as well
											TableElement teGroupedNew;
											teGroupedNew.m_pColumn = pTableCol;
											teGroupedNew.m_pRow = const_cast<TableRow*>(pTableRow);
											teGroupedNew.m_bChecked = FALSE;
											arNewTableElements.Add(teGroupedNew);
										}
									}
								}
							}
						} else {
							// element not found in the map, and we are unchecking items.
							//TRACE("not found\n");
						}
					}

					mapTable.RemoveAll();

					// add the new table elements
					for (int iNew = 0; iNew < arNewTableElements.GetSize(); iNew++) {
						m_pDetail->SetTableElement(arNewTableElements[iNew]);
					}
					arNewTableElements.RemoveAll();

					// we are not messing with linked details, so we can preserve the linked details cached flag
					// however we still need to recreate the state
					m_pDetail->RecreateStateFromContent();

					//generate the new state and refresh
					CString strNewVarState = GenerateNewVarState();
					if (RequestDetailStateChange((LPCTSTR)strNewVarState)) {
						CRect rc;
						m_wndTable.GetWindowRect(&rc);
						ScreenToClient(&rc);
						// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
						// children, invalidating the child area is not good enough, so we 
						// have to actually tell each child to invalidate itself.  Also, we 
						// used to ask it to erase too, because according to an old comment 
						// related to plid 13344, the "SetRedraw" state might be false at 
						// the moment.  (I'm deleting that comment on this check-in, so to 
						// see it you'll have to look in source history.)  But I think the 
						// "SetRedraw" state no longer can be false, and even if it were I 
						// think now that we're invalidating the control itself, we wouldn't 
						// need to erase here anyway.
						m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
					}

					ReflectTableState();
				}
			}
			else if(tc.nType == LIST_TYPE_TEXT) {

				// (j.jones 2009-10-02 11:17) - PLID 35161 - added ability to append date information

				BOOL bAllowEdit = TRUE;

				//disabled on read only tables, and templates
				if(m_bReadOnly || m_pDetail->m_bIsTemplateDetail) {
					bAllowEdit = FALSE;
				}

				//make sure we have a patient ID
				long nPatientID = m_pDetail->GetPatientID();
				if(nPatientID == -1) {
					bAllowEdit = FALSE;
				}
				
				// (z.manning 2010-03-02 10:54) - PLID 37230 - Don't do this if the column is read only
				// (z.manning 2010-04-19 16:20) - PLID 38228 - Improved this check
				if(ptr->IsReadOnly() || ptc->IsReadOnly()) {
					bAllowEdit = FALSE;
				}

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();

				CMenu pSubMenu;
				pSubMenu.CreatePopupMenu();
				
				enum {
					eTodaysDate = 1,
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times
					eCurrentDateTime,
					eCurrentTime,
					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					eEMNDate,
					eLastApptDate,
					eLastApptDays,
					eLastProcDate,
					eLastProcDays,
				};

				if(bAllowEdit) {
					pSubMenu.InsertMenu(0, MF_BYPOSITION, eTodaysDate, "Today's &Date");
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times
					pSubMenu.InsertMenu(1, MF_BYPOSITION, eCurrentDateTime, "&Current Date && Time");
					pSubMenu.InsertMenu(2, MF_BYPOSITION, eCurrentTime, "Current &Time");
					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					pSubMenu.InsertMenu(3, MF_BYPOSITION, eEMNDate, "&EMN Date");
					pSubMenu.InsertMenu(4, MF_BYPOSITION, eLastApptDate, "&Last Appointment Date");
					pSubMenu.InsertMenu(5, MF_BYPOSITION, eLastApptDays, "Days Since Last &Appointment");
					pSubMenu.InsertMenu(6, MF_BYPOSITION, eLastProcDate, "Last &Procedure Date");
					pSubMenu.InsertMenu(7, MF_BYPOSITION, eLastProcDays, "Days &Since Last Procedure");

					pMenu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu.m_hMenu, "&Add Field...");
					pMenu.AppendMenu(MF_SEPARATOR);
				}

				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				AppendSharedMenuOptions(&pMenu, pRow, nCol, m_pDetail);

				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

				CString strTextToAppend = "";

				switch(nMenuCmd) {
					case eTodaysDate: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, NULL, dtoDate);
						break;
						}
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times, to the minute
					case eCurrentDateTime: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime);
						break;
						}
					case eCurrentTime: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoTime);
						break;
						}
					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					case eEMNDate: {

						COleDateTime dtEMN = m_pDetail->m_pParentTopic->GetParentEMN()->GetEMNDate();
						strTextToAppend = FormatDateTimeForInterface(dtEMN, NULL, dtoDate);
						break;
						}
					case eLastApptDate:
					case eLastApptDays: {

						//get the last appointment *prior* to today (not on today's date)
						_RecordsetPtr rs = CreateParamRecordset("SELECT Max(Date) AS LastDate, "
							"DateDiff(day, dbo.AsDateNoTime(Max(Date)), dbo.AsDateNoTime(GetDate())) AS Days "
							"FROM AppointmentsT "
							"WHERE PatientID = {INT} AND Status <> 4 "
							"AND dbo.AsDateNoTime(Date) < dbo.AsDateNoTime(GetDate()) "
							"GROUP BY PatientID", nPatientID);
						if(!rs->eof) {
							if(nMenuCmd == eLastApptDate) {
								strTextToAppend = FormatDateTimeForInterface(AdoFldDateTime(rs, "LastDate"), NULL, dtoDate);
							}
							else if(nMenuCmd == eLastApptDays) {
								strTextToAppend.Format("%li days", AdoFldLong(rs, "Days"));
							}
						}
						else {
							AfxMessageBox("This patient has no previous appointment.");
							return;
						}
						rs->Close();

						break;
						}
					case eLastProcDate:
					case eLastProcDays: {

						//get the last procedural appointment *prior* to today (not on today's date)
						_RecordsetPtr rs = CreateParamRecordset("SELECT Max(Date) AS LastDate, "
							"DateDiff(day, dbo.AsDateNoTime(Max(Date)), dbo.AsDateNoTime(GetDate())) AS Days "
							"FROM AppointmentsT "
							"WHERE PatientID = {INT} AND Status <> 4 "
							"AND dbo.AsDateNoTime(Date) < dbo.AsDateNoTime(GetDate()) "
							"AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) "
							"GROUP BY PatientID", nPatientID);
						if(!rs->eof) {
							if(nMenuCmd == eLastProcDate) {
								strTextToAppend = FormatDateTimeForInterface(AdoFldDateTime(rs, "LastDate"), NULL, dtoDate);
							}
							else if(nMenuCmd == eLastProcDays) {
								strTextToAppend.Format("%li days", AdoFldLong(rs, "Days"));
							}
						}
						else {
							AfxMessageBox("This patient has no previous procedure appointment.");
							return;
						}
						rs->Close();

						break;
						}
				}

				if(!strTextToAppend.IsEmpty()) {

					AppendTextToTableCell(pRow, nCol, m_pDetail, strTextToAppend, this, m_pParentDlg, TRUE);
				}
			}
			else {
				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu menu;
				menu.CreatePopupMenu();
				AppendSharedMenuOptions(&menu, pRow, nCol, m_pDetail);
				
				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
			}

			// (z.manning 2010-12-20 10:05) - PLID 41886 - Clear options
			EClearTableType eClearType = cttInvalid;
			if(nMenuCmd == IDM_CLEAR_TABLE_CELL) {
				eClearType = cttCell;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE_ROW) {
				eClearType = cttRow;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE_COLUMN) {
				eClearType = cttColumn;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE) {
				eClearType = cttTable;
			}

			if(eClearType != cttInvalid) {
				ClearTable(eClearType, pRow, nCol, m_pDetail, this, m_pParentDlg, TRUE);
			}

			// (z.manning 2010-12-22 15:25) - PLID 41887 - Copy options
			if( nMenuCmd == IDM_COPY_TABLE_ROW_DOWN || nMenuCmd == IDM_COPY_TABLE_ROW_UP ||
				nMenuCmd == IDM_COPY_TABLE_CELL_DOWN || nMenuCmd == IDM_COPY_TABLE_CELL_UP)
			{
				BOOL bCellOnly, bCopyFoward;
				switch(nMenuCmd)
				{
					case IDM_COPY_TABLE_ROW_DOWN:
						bCellOnly = FALSE;
						bCopyFoward = TRUE;
						break;
					case IDM_COPY_TABLE_ROW_UP:
						bCellOnly = FALSE;
						bCopyFoward = FALSE;
						break;
					case IDM_COPY_TABLE_CELL_DOWN:
						bCellOnly = TRUE;
						bCopyFoward = TRUE;
						break;
					case IDM_COPY_TABLE_CELL_UP:
						bCellOnly = TRUE;
						bCopyFoward = FALSE;
						break;
					default:
						ASSERT(FALSE);
						break;
				}

				CopyAndPasteTableRowToAdjacentRow(pRow, nCol, m_pDetail, bCellOnly, bCopyFoward, this, m_pParentDlg, TRUE);
			}

			switch(nMenuCmd)
			{
				case IDM_TRANSFORM_ROW:
					// (z.manning 2011-05-27 10:42) - PLID 42131
					ApplyTransformation(m_pDetail, pRow, nCol, this, m_pParentDlg, TRUE);
					break;

				case IDM_EDIT_CODING_GROUP: // (z.manning 2011-07-14 10:23) - PLID 44469
					OpenChargePromptDialogForRow(m_pDetail, pRow, nCol);
					break;
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
void CEMRItemAdvPopupWnd::OnRequestAllAvailableFields(VARIANT_BOOL bIncludeLetterWriting, VARIANT_BOOL bIncludeEMR, VARIANT_BOOL bIncludeListItems, VARIANT* psafearrayFields)
{
	try {
		COleSafeArray saFields;
		m_pDetail->PopulateAvailableNarrativeFields(saFields, bIncludeLetterWriting != VARIANT_FALSE ? true : false, bIncludeEMR != VARIANT_FALSE ? true : false, bIncludeListItems != VARIANT_FALSE ? true : false);

		*psafearrayFields = saFields.Detach();
	} NxCatchAll("Error loading available narrative fields for popup");
}

// (z.manning 2011-11-10 10:15) - PLID 46382
void CEMRItemAdvPopupWnd::OnRequestAvailableFieldsVersion(short* pnVersion)
{
	try
	{
		*pnVersion = NARRATIVE_AVAILABLE_FIELD_VERSION;
	}
	NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-11-17 11:38) - PLID 36365 - Push updates to the narrative
// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
void CEMRItemAdvPopupWnd::UpdateNarrativeFields(COleSafeArray& saFieldValues, bool bForceUpdate)
{	
	if(IsWindow(GetSafeHwnd())) {
		if(NULL != m_RichEditCtrl) {
			m_pDetail->InvalidateLinkedDetailCache();

			long nFieldsRemoved = 0;
			long nFieldsUpdated = 0;
			m_RichEditCtrl->PushFieldUpdates(saFieldValues, &nFieldsRemoved, &nFieldsUpdated);

			if (bForceUpdate || (nFieldsRemoved != 0 || nFieldsUpdated != 0)) {
				RequestDetailStateChange(m_RichEditCtrl->RichText);
				//TES 1/15/2008 - PLID 24157 - Notify our parent.
				m_pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)m_pDetail);
			}
		}
	}
}

// (z.manning 2011-09-14 10:18) - PLID 44693
void CEMRItemAdvPopupWnd::OnOpenStampSetup3DControl()
{
	try
		{
		HandleStampSetup();
			}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-10 14:44) - PLID 37312 - handle when the image tries to open the stamp setup
void CEMRItemAdvPopupWnd::OnOpenStampSetupInkPicture()
{
	try
	{
		HandleStampSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-10-25 14:48) - PLID 39401 - Moved common logic to this function
void CEMRItemAdvPopupWnd::HandleStampSetup()
{
		//edit the global list
	// (a.walling 2012-03-12 10:06) - PLID 46648 - Dialogs must set a parent!
	if(!GetMainFrame()->EditEMRImageStamps(this)) {
		//returns FALSE if nothing changed
		return;
	}

	// (z.manning 2011-10-25 15:30) - PLID 39401 - The stamps changed so we need to reload stamp exclusions on the 
	// popup's copy of the detail.
	m_pDetail->ReloadStampExclusions();

		//the following code was moved from OnCustomStampsChangedInkPicture
	// (z.manning 2011-10-25 13:04) - PLID 39401 - Pass in the detail
	CString strStamps = GenerateInkPictureStampInfo(m_pDetail);
		
		// If we are a member of the multi popup, make sure the stamps are refreshed both on the EMN and on the multipopup tree
		if (m_pParentDlg && m_pParentDlg->IsKindOf(RUNTIME_CLASS(CEMRItemAdvMultiPopupDlg))) {
			//SetPropertyMemo("EMR_Image_Custom_Stamps", strNewCustomStamps, 0);

			CEMRItemAdvMultiPopupDlg* pMultiPopupDlg = (CEMRItemAdvMultiPopupDlg*)m_pParentDlg;			
			pMultiPopupDlg->RefreshCustomStamps(strStamps, this);
		}
		
		SetCustomStamps(strStamps);

		CEMNDetail* pDetail = m_pRealDetail ? m_pRealDetail : m_pDetail;

		if (pDetail && pDetail->m_pParentTopic && pDetail->m_pParentTopic->GetParentEMN() && pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR() && pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface()) {
			(pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface())->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
		}
}

// (r.gonet 02/14/2012) - PLID 37682 - User clicked the stamp filter setup, so open the dialog.
void CEMRItemAdvPopupWnd::OnOpenStampFilterSetupInkPicture()
{
	try
	{
		HandleStampFilterSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/14/2012) - PLID 37682 - User clicked the stamp filter setup, so open the dialog.
void CEMRItemAdvPopupWnd::OnOpenStampFilterSetup3DControl()
{
	try
	{
		HandleStampFilterSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/14/2012) - PLID 37682 - Open the stamp filter setup dialog.
void CEMRItemAdvPopupWnd::HandleStampFilterSetup()
{
	BOOL bFilterChanged = TRUE;
	if(m_pDetail->GetImageTextStringFilter() != NULL) {
		// Turn the filter off
		m_pDetail->SetImageTextStringFilter(CTextStringFilterPtr());
		m_pDetail->ReflectImageTextStringFilter();
	} else {
		bFilterChanged = m_pDetail->EditImageStampFilter(this);
	}
	if(bFilterChanged) {
		// (r.gonet 02/14/2012) - PLID 37682 - Reflect the filter on the image.
		this->SetImageTextStringFilter(m_pDetail->GetImageTextStringFilter());
	}
}

/* (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
void CEMRItemAdvPopupWnd::OnZoomLevelChangedInkPicture(double dZoomLevel)
{
	try {
		m_pDetail->SetZoomLevel(dZoomLevel);
	}NxCatchAll(__FUNCTION__);
}

void CEMRItemAdvPopupWnd::OnViewportOffsetsChangedInkPicture(long nOffsetX, long nOffsetY)
{
	try {
		m_pDetail->SetOffsetX(nOffsetX);
		m_pDetail->SetOffsetY(nOffsetY);
	}NxCatchAll(__FUNCTION__);
}
*/

// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
void CEMRItemAdvPopupWnd::UpdateRememberedPenState()
{
	if(m_Image != NULL)
	{
		m_nCurPenColor = m_Image->GetCurrentPenColor();
		m_fltCurPenSize = m_Image->GetCurrentPenSize();
		m_bIsEraserState = m_Image->GetIsEraserState();
	}
}

// (a.walling 2010-05-19 11:26) - PLID 38778 - Toggle between resized / full mode
void CEMRItemAdvPopupWnd::OnFullScreenInkPicture()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		m_bShrinkFitToArea = !m_bShrinkFitToArea;

		// (a.walling 2010-07-19 14:25) - PLID 38778 - If reverting back to shrink to fit area, then discard any scroll information.
		// CallRepositionControls may re-create this if necessary.
		if (m_bShrinkFitToArea) {
			// Set up the vertical scroll bar
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			::ZeroMemory(&si, sizeof(si));
			si.fMask = SIF_ALL;

			SetScrollInfo(SB_HORZ, &si, FALSE);
			SetScrollInfo(SB_VERT, &si, TRUE);
		}

		CallRepositionControls();
	} NxCatchAll("Error in OnFullScreenInkPicture");
}

// (j.jones 2011-03-23 10:15) - PLID 42965 - supported the macro abilities in the popup
LRESULT CEMRItemAdvPopupWnd::OnPasteMacroText(WPARAM wParam, LPARAM lParam)
{
	try {
		CEmrTextMacroDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			// User chose macro text. Replace the current selection with the macro text.
			m_edit.ReplaceSel(dlg.m_strResultTextData);
		} else {
			// User changed their mind
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (z.manning 2011-10-05 13:34) - PLID 45842
void CEMRItemAdvPopupWnd::OnPreviewModified3DControl()
{
	try
	{
		if(m_p3DControl != NULL)
		{
			_variant_t varPrintData = m_p3DControl->GetViewData();
			m_pDetail->SetImagePrintData(varPrintData);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEMRItemAdvPopupWnd::OnPaint()
{
	CPaintDC dc(this);

	if (PaintWindowlessControls(&dc)) {
		return;
	}

	Default();
}

// (z.manning 2012-03-27 16:58) - PLID 33710 - Call this function to request a detail's state change from the popup
// (z.manning 2012-03-28 14:24) - I eneded up removing the extra code I had added here but left the function.
BOOL CEMRItemAdvPopupWnd::RequestDetailStateChange(const _variant_t &varNewState)
{
	BOOL bReturn = m_pDetail->RequestStateChange(varNewState);
	return bReturn;
}

void CEMRItemAdvPopupWnd::OnCheckValidIDsRichTextCtrl(long nDetailID, BOOL *pbDetailIDExists, long nDataID, BOOL *pbDataIDExists)
{
	try {
		//TES 7/3/2012 - PLID 51357 - The control wants to know whether the DetailID and DataID it has actually exist on this EMN.
		CEMNDetail* pDetail = m_pDetail->GetParentEMN()->GetDetailByID(nDetailID);
		if(pDetail) {
			*pbDetailIDExists = TRUE;
			ListElement *ple = pDetail->GetListElementByID(nDataID);
			if(ple) {
				*pbDataIDExists = TRUE;
			}
			else {
				*pbDataIDExists = FALSE;
			}
		}
		else {
			*pbDetailIDExists = FALSE;
			*pbDataIDExists = FALSE;
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMRItemAdvPopupWnd::OnOpenStampSearch()
{
	try
	{		
		// (a.walling 2012-08-28 08:37) - PLID 52321 - Launch the stamp dialog and process the clicked stamp
		CStampSearchDlg dlg(this);
		if (IDOK == dlg.DoModal() && -1 != dlg.GetClickedStampID()) {				
			long nStampID = dlg.GetClickedStampID();

			// (a.walling 2012-08-28 10:23) - PLID 51742 - Get stamp from the cache
			EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
			ENSURE(pStamp);

			// (a.walling 2012-09-04 12:07) - PLID 52331 - Pass image info if available
			_variant_t var;
			if (pStamp->m_ImageInfo.arImageBytes && pStamp->m_ImageInfo.nNumImageBytes) {
				var.vt = VT_UI1 | VT_ARRAY;
				var.parray = Nx::SafeArray<BYTE>(
					pStamp->m_ImageInfo.nNumImageBytes
					, pStamp->m_ImageInfo.arImageBytes
				).Detach();
			}
			m_Image->SetCurrentStampInfoWithImage((LPCTSTR)pStamp->strStampText, pStamp->nTextColor, pStamp->nID, (LPCTSTR)pStamp->strTypeName, pStamp->bShowDot, var);
		}
	}
	NxCatchAll(__FUNCTION__);
}
// (j.gruber 2012-08-14 12:23) - PLID 52134
void CEMRItemAdvPopupWnd::OnOpenStampSearch3D()
{
	try
	{
		if (m_p3DControl != NULL) {
			// (a.walling 2012-08-28 08:37) - PLID 52321 - Launch the stamp dialog and process the clicked stamp
			CStampSearchDlg dlg(this);
			if (IDOK == dlg.DoModal() && -1 != dlg.GetClickedStampID()) {
				long nStampID = dlg.GetClickedStampID();

				// (a.walling 2012-08-28 10:23) - PLID 51742 - Get stamp from the cache
				EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
				ENSURE(pStamp);
				m_p3DControl->SetCurrentStampInfo((LPCTSTR)pStamp->strStampText, pStamp->nTextColor, pStamp->nID, (LPCTSTR)pStamp->strTypeName);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
BOOL CEMRItemAdvPopupWnd::PreTranslateMessage(MSG* pMsg)
{
	if (BOOL bRet = CEmrItemAdvTableBase::HandlePreTranslateMessage(pMsg)) {
		return bRet;
	}

	return __super::PreTranslateMessage(pMsg);
}

// (d.singleton 2013-4-22 14:32) - PLID 56421 get the signature form the topaz sig pad and display on ink control
void CEMRItemAdvPopupWnd::OnClickedTopazSignature()
{
	try {
		// (a.walling 2014-07-28 10:36) - PLID 62825 - Use TopazSigPad::GetSignatureInk
		MSINKAUTLib::IInkDispPtr pInk = TopazSigPad::GetSignatureInk(this);
		if (!pInk) {
			//no ink or failure,  return
			return;
		}
		// (b.spivey, March 27, 2013) - PLID 30035 - Finally, we have created an ink object to add to the control. 
		m_Image->PutInkData(g_cvarEmpty); 
		m_Image->AddStrokesFromInkWithOffset(_variant_t((LPDISPATCH)pInk), (float)(-0.20), (float)(0.0)); 

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-22 09:15) - PLID 62836 - Handle pen color changed
void CEMRItemAdvPopupWnd::OnCurrentPenColorChanged()
{
	try
	{
		m_pDetail->OnCurrentPenColorChanged(m_nCurPenColor = m_Image->CurrentPenColor);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-22 09:15) - PLID 62836 - Handle pen size changed
void CEMRItemAdvPopupWnd::OnCurrentPenSizeChanged()
{
	try
	{
		m_fltCurPenSize = m_Image->CurrentPenSize;
		m_pDetail->OnCurrentPenSizeChanged(m_Image->CurrentPenSizePercent);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-04-21 16:19) - PLID 43994 - User Story 16 - FOI Items -  Requirement 160050 - Popped up tables do not
// open dropdown cells in a single click the way that non-popped up tables do
void CEMRItemAdvPopupWnd::OnLButtonUpTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (b.savon 2014-04-21 16:19) - PLID 43994 - User Story 16 - FOI Items -  Requirement 160050
		// *** NOTE: Lifted from Bob's note in EmrItemAdvTableDlg.cpp ~ CEmrItemAdvTableDlg::OnLButtonUpTable
		//
		// Made it so this datalist starts editing 
		// with a single click by making it call StartEditing on lbutton up.  This has the one 
		// slight drawback that if you lbutton DOWN somewhere else (anywhere else really) and 
		// then move your mouse and lbutton UP here, it still starts editing this cell.  
		// Technically that's incorrect behavior, but I think most of our users prefer it this 
		// way.  The correct way would be to use LeftClick, but that has a much bigger drawback: 
		// when you have an embedded combo dropped down and you click off on another cell in the 
		// datalist, it dismisses the embedded combo but doesn't start editing that other cell 
		// you clicked on, because the embedded combo ate the lbutton DOWN, so the LeftClick is 
		// not fired.  Since this interface is generally designed for tablets, it almost seems 
		// preferable to have the editing start on lbutton up anyway, since people have a 
		// tendency to slide the stylus around on the tablet.
		if(lpRow != NULL && nCol != -1) {
			IRowSettingsPtr pRow(lpRow);
			StartTableEditing(pRow, nCol);
		}
	} NxCatchAll("Error in OnLButtonUpTable");
}