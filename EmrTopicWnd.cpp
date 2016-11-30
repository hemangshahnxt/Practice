// EmrTopicWnd.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EmrTopicWnd.h"
#include "EMRTopic.h"
#include "EMN.h"
#include "EmrItemAdvDlg.h"
#include "EmrItemAdvNarrativeDlg.h"
#include "EmrItemAdvTableDlg.h"
#include "EmrItemAdvListDlg.h"
#include "EmrEditorDlg.h"
#include "EMNLoader.h"
#include "EMNUnspawner.h"
#include "EMRTextMacroDlg.h"
#include "SignatureDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "EmrTopicView.h"
#include "EmrColors.h"
#include "EmrFrameWnd.h"
#include "EMR.h"
#include "EMNDetail.h"
#include "UserVerifyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEmrTopicWnd


// (a.walling 2008-09-15 17:08) - PLID 26781 - The ChildDetailsInOrder collection has been changed to a list

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
// storing a pointer to the 'parent' to avoid GetParent stuff since that is no longer the same with the facelift

// (a.walling 2008-09-15 17:07) - PLID 26781 - Limit for when a topic's resource usage may be considered dangerous
// (a.walling 2008-09-18 13:11) - PLID 26781 - Upped to 1000 from 400.

// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Got rid of a lot of scrolling code

CEmrTopicWnd::CEmrTopicWnd()
	: m_pEmrTreeWnd(NULL)
	, m_szContentSize(0,0)
	, m_nMinContentHeight(128)
{
	m_pTopic = NULL;
	m_bIsTemplate = FALSE;
	m_bIsReadOnly = FALSE;
	m_bAllowEdit = FALSE;
	m_colorHighlight = 0;

	// (a.walling 2011-03-19 12:23) - PLID 42694 - This stuff is mostly handled in NxScrollAnchor now
	//m_bDragging = FALSE;
	//m_ptInitialCursorPos = m_ptLastCursorPos = CPoint(0,0);
}

CEmrTopicWnd::~CEmrTopicWnd()
{
	// (a.walling 2011-10-20 21:22) - PLID 46075 - Ensure the interface window ptr is not associated with our now-destroyed window
	try {
		if (m_pTopic && m_pTopic->GetTopicWndRaw() == this) {
			m_pTopic->ResetTopicWnd();
		}
		m_pTopic = NULL;
	} NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CEmrTopicWnd, CWnd)
	//{{AFX_MSG_MAP(CEmrTopicWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()

	// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward these messages to the tree wnd

	//OnEmrItemChanged is really just "this item changed",
	//and the parent needs to know it for display purposes
	//the parent topic to the detail should already have been marked unsaved
	ON_MESSAGE(NXM_EMR_ITEM_CHANGED,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMR_ITEM_STATECHANGED,		ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_PRE_DELETE_EMR_ITEM,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_MERGE_OVERRIDE_CHANGED,		ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMN_DETAIL_DRAG_BEGIN,		ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMN_DETAIL_DRAG_END,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMR_PROBLEM_CHANGED,			ForwardCurrentMessageToTreeWnd)

	// (a.walling 2012-03-22 16:50) - PLID 49141 - All more info changed messages go to the EMN's interface now
	
	// (j.jones 2008-07-28 10:44) - This code was added because
	// CEmrTopicWnd can sometimes be passed into the problem list,
	// which can then sometimes send this message back. However, it
	// is not currently possible to send this message to the TopicWnd
	// because only detail problems are loaded - for now - when
	// opening the problem list from a detail. This is just for
	// future-proofing.
	ON_MESSAGE(NXM_EMN_CHANGED,					ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMR_TOPIC_CHANGED,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMN_REFRESH_CHARGES,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMN_REFRESH_PRESCRIPTIONS,	ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMN_REFRESH_DIAG_CODES,		ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMR_MINIMIZE_PIC,			ForwardCurrentMessageToTreeWnd)
	ON_MESSAGE(NXM_EMR_SAVE_ALL,				ForwardCurrentMessageToTreeWnd)

	/// These actually do something:
	
	// from treewnd
	ON_MESSAGE(NXM_EMR_ITEM_ADDED, OnEmrItemAdded)
	
	// always requires a topicwnd
	ON_MESSAGE(NXM_EMR_ITEM_REMOVE, OnEmrItemRemove)
	ON_MESSAGE(NXM_CONVERT_RECT_FOR_DATA, OnConvertRectForData)
	ON_MESSAGE(NXM_INSERT_STOCK_EMR_ITEM, OnInsertStockEmrItem)
	ON_MESSAGE(NXM_ADD_IMAGE_TO_EMR, OnAddImageToEMR)
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	ON_MESSAGE(NXM_ADD_GENERIC_TABLE_TO_EMR, OnAddGenericTableToEMR)
	// (a.walling 2011-03-19 12:23) - PLID 42694 - Added handlers for WM_SETCURSOR and the various mouse messages
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CEmrTopicWnd, CWnd);

// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward message to the tree wnd
LRESULT CEmrTopicWnd::ForwardMessageToTreeWnd(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_pEmrTreeWnd->GetSafeHwnd() && IsWindow(m_pEmrTreeWnd->GetSafeHwnd())) {
			return m_pEmrTreeWnd->SendMessage(nMsg, wParam, lParam);
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward current message to the tree wnd
LRESULT CEmrTopicWnd::ForwardCurrentMessageToTreeWnd(WPARAM wParam, LPARAM lParam)
{
	UINT nMsg = AfxGetThreadState()->m_lastSentMsg.message;
	return ForwardMessageToTreeWnd(nMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CEmrTopicWnd message handlers
// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
void CEmrTopicWnd::Initialize(CEMRTopic *pTopic, CWnd* pParent, CEmrTreeWnd *pTreeWnd, CRect rcWindow, BOOL bIsTemplate, BOOL bIsReadOnly, BOOL bAllowEdit)
{
	//take in the CEMRTopic, and create all its CEMNDetail dialogs
	m_pTopic = pTopic;

	// (a.walling 2012-07-10 15:02) - PLID 49353 - check for spawned topics
	if (bIsTemplate && m_pTopic->GetSourceActionID() != -1) {
		bIsReadOnly = TRUE;
	}

	if (bIsReadOnly) {
		bAllowEdit = FALSE;
	}

	//give the topic a pointer to this window
	m_pTopic->SetTopicWnd(shared_from_this());

	m_pEmrTreeWnd = pTreeWnd;

	m_bIsTemplate = bIsTemplate;
	m_bIsReadOnly = bIsReadOnly;
	m_bAllowEdit = bAllowEdit;

	//if the passed in topic is NULL, exit now - we shouldn't be here without a topic object
	if(!m_pTopic) {
		ASSERT(FALSE);
		return;
	}

	//TES 12/13/2005 - Sometimes (like after clicking "Add Empty Topic"), topics legitimately don't have any details.
	/*//The EMRTreeWnd is responsible for only calling this function
	//for Topics that have details, and not for Topics that only have sub-topics
	long nDetailCount = m_pTopic->GetEMNDetailCount();
	if(nDetailCount == 0) {
		ASSERT(FALSE);
		return;
	}*/

	//Create the window.

	// (b.cardillo 2006-02-21 17:01) - PLID 19385 - We want the window to auto-repaint when it's 
	// sized, so we give the window class the CS_VREDRAW and CS_HREDRAW flags.
	// (a.walling 2011-03-19 12:23) - PLID 42694 - Get rid of CS_VREDRAW and CS_HREDRAW -- too much flickering.
	// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
	LPCTSTR szWndClass = AfxRegisterWndClass(CS_PARENTDC|CS_NOCLOSE, NULL, EmrColors::Topic::Background(!!bIsTemplate));
	BOOL bAns = CreateEx(
		WS_EX_CONTROLPARENT, 
		szWndClass, NULL, 
		WS_CHILD|WS_GROUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, rcWindow, pParent, IDC_TOPIC_AREA);
	if(bAns) {
		/*// (b.cardillo 2006-02-21 13:19) - PLID 19404 - The window class now owns the background 
		// brush, so we detach ours before letting it go out of scope, otherwise the handle the 
		// window uses won't be valid, because the CBrush destructor will have destroyed it.  
		// Also notice that this means we no longer need the OnPaint() handler, because it was 
		// only there because the background eraser wasn't working.  AND now that we no longer 
		// need the OnPaint(), we also no longer need the m_colorBackground member variable to 
		// store our background color, so now it's just a local variable in this function.
		brBackground.Detach();*/

		// (b.cardillo 2004-05-19 13:13) - PLID 12410 - There is a crazy bug in MFC where if you have a subdialog and that 
		// subdialog has exactly one child AND that one child is itself a subdialog AND that one child is invisible, then 
		// the _AfxNextControl() MFC function gets into an infinitely recursive loop searching for the next child to send 
		// keyboard mnemonics to whenever the user hits the keyboard.  By adding a SECOND control that is STATIC (though 
		// still invisible) to the original subdialog we save the _AfxNextControl() function from its infinite recursion.

		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect

		// (a.walling 2012-05-07 09:21) - PLID 48934 - This bug is now fixed via CNxOccManager
		//m_wndInvisibleAnchor.Create("invisible static control", WS_CHILD|WS_GROUP, CRect(0,0,0,0), this);

		// that was a problem with the OLE control container, which was fixed in VC9
	}

	//now we can create all the EMNDetail dialogs (this will also set the read-only/allow-edit state)
	// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
	RepositionDetailsInTopicByInfoID(-1, FALSE);

	// (a.walling 2012-06-18 16:19) - PLID 49353 - Ensure we start off with the proper modified state and back color etc
	GetEmrTopicView()->GetEmrTreeWnd()->EnsureTopicModifiedState(m_pTopic);
	GetEmrTopicView()->GetEmrTreeWnd()->EnsureTopicBackColor(m_pTopic, NULL, FALSE, FALSE);
}

void CEmrTopicWnd::OnActiveTopicChanged()
{
	if (!::IsWindow(GetSafeHwnd())) return;
	if (!m_pTopic) return;

	CEmrTopicHeaderWndPtr pHeader = m_pTopic->GetTopicHeaderWnd();
	if (!pHeader) return;

	pHeader->OnActiveTopicChanged();
}

void CEmrTopicWnd::SetHighlightColor(COLORREF colorHighlight)
{
	m_colorHighlight = colorHighlight;

	if(!m_pTopic) {
		ASSERT(FALSE);
		return;
	}
	
	for (long i=0; i<m_pTopic->GetEMNDetailCount(); i++) {
		CEMNDetail *pEMNDetail = m_pTopic->GetDetailByIndex(i);
		if (pEMNDetail) {
				pEMNDetail->ChangeHilightColor(colorHighlight);
		}
	}
}

void CEmrTopicWnd::SetReadOnly(BOOL bIsReadOnly)
{
	m_bIsReadOnly = bIsReadOnly;

	//set each detail to our read-only style
	for (long i=0; i<m_pTopic->GetEMNDetailCount(); i++) {
		CEMNDetail *pEMNDetail = m_pTopic->GetDetailByIndex(i);
		ASSERT(pEMNDetail);
		if (pEMNDetail) {
			pEMNDetail->SetReadOnly(m_bIsReadOnly);
		}
	}
}

void CEmrTopicWnd::SetAllowEdit(BOOL bAllowEdit)
{
	m_bAllowEdit = bAllowEdit;

	//set each detail to our allow-edit style
	for (long i=0; i<m_pTopic->GetEMNDetailCount(); i++) {
		CEMNDetail *pEMNDetail = m_pTopic->GetDetailByIndex(i);
		ASSERT(pEMNDetail);
		if (pEMNDetail) {
			pEMNDetail->SetAllowEdit(m_bAllowEdit);
		}
	}
}

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RepositionDetailsInTopic into two functions,
// accepting an InfoID or a MasterID
// (c.haag 2011-11-01) - PLID 46223 - Added pDetailToPreventLoadContentAndState
void CEmrTopicWnd::RepositionDetailsInTopicByInfoID(const long nReloadContentForInfoID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail /*= NULL*/, OPTIONAL IN LPCRECT prcOverrideDetailRect /*= NULL*/, BOOL bMaintainImagesSize /*= FALSE*/,
													CEMNDetail* pDetailToPreventLoadContentAndState /* = NULL */)
{
	RepositionDetailsInTopicByInfoOrMasterID(nReloadContentForInfoID, -1, bSetRegionAndInvalidate, pOverrideDetail, prcOverrideDetailRect, bMaintainImagesSize, pDetailToPreventLoadContentAndState);
}

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RepositionDetailsInTopic into two functions,
// accepting an InfoID or a MasterID
// (c.haag 2011-11-01) - PLID 46223 - Added pDetailToPreventLoadContentAndState
void CEmrTopicWnd::RepositionDetailsInTopicByInfoMasterID(const long nReloadContentForInfoMasterID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail /*= NULL*/, OPTIONAL IN LPCRECT prcOverrideDetailRect /*= NULL*/, BOOL bMaintainImagesSize /*= FALSE*/,
													CEMNDetail* pDetailToPreventLoadContentAndState /* = NULL */)
{
	RepositionDetailsInTopicByInfoOrMasterID(-1, nReloadContentForInfoMasterID, bSetRegionAndInvalidate, pOverrideDetail, prcOverrideDetailRect, bMaintainImagesSize, pDetailToPreventLoadContentAndState);
}

// (j.jones 2007-07-26 09:28) - PLID 24686 - created non-public function to accept an InfoID or InfoMasterID
// (c.haag 2011-11-01) - PLID 46223 - Added pDetailToPreventLoadContentAndState
void CEmrTopicWnd::RepositionDetailsInTopicByInfoOrMasterID(const long nReloadContentForInfoID, const long nReloadContentForInfoMasterID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail /*= NULL*/, OPTIONAL IN LPCRECT prcOverrideDetailRect /*= NULL*/, BOOL bMaintainImagesSize /*= FALSE*/,
													CEMNDetail* pDetailToPreventLoadContentAndState /* = NULL */)
{
	// (c.haag 2007-05-07 16:13) - PLID 25928 - Wait until after the load is complete before
	// repositioning details
	if (!m_pTopic->IsPostLoadComplete() && (m_pTopic->GetParentEMN() && m_pTopic->GetParentEMN()->IsLoading())) {
		m_pTopic->SetNeedsToRepositionDetailsInPostLoad(TRUE);
		return;
	}

	try {
		CRect rcTotalArea(0, 0, 0, 0);

		// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Use our parent's client area at infinite height
		// Get the client rect for the subdialog (in case it's changed from last time)
		CRect rcOrigArea(0, 0, 928, 32767);
		if (GetParent()) {
			GetParent()->GetClientRect(&rcOrigArea);
		}

		// Create an initially rectangular region that we'll subtract from as we decide where to put each detail
		CRgn rgn;
		CRect rcCurArea(rcOrigArea);

		// Deflate it to ensure we don't extend past our margins
		// (b.cardillo 2005-11-09 18:16) - PLID 13319 - Made it only deflate the border width from 
		// the top and left, not the bottom and right, because we don't want the scrollbars to be 
		// visible unless absolutely necessary (if we subtracted the border, it would result in a 
		// small buffer at the bottom and right, and as soon as you started eating into that buffer, 
		// the scrollbars would appear).
		rcCurArea.DeflateRect(gc_nStaticBorderWidth, gc_nStaticBorderWidth, 0, 0);

		//DRT 6/11/2008 - PLID 29979 - It is possible, especially now with the preview pane, that there is no client width
		//	(we encourage some users to use the preview pane at full width).  When this happens, the code that checks for 
		//	scrollbars and borders above can possibly make the right or bottom sides smaller than the left or top.  This
		//	throws off the positioning below, and you end up with details partially shoved off the screen.  We just want
		//	to constraint our rect so that right is always >= left and bottom is always >= top before doing the positioning.
		if(rcCurArea.right < rcCurArea.left) {
			rcCurArea.right = rcCurArea.left;
		}
		if(rcCurArea.bottom < rcCurArea.top) {
			rcCurArea.bottom = rcCurArea.top;
		}

		rgn.CreateRectRgnIndirect(rcCurArea);

		try {
			for (long i=0; i<m_pTopic->GetEMNDetailCount(); i++) {
				//
				CEMNDetail *pEMNDetail = m_pTopic->GetDetailByIndex(i);
				// Whoever adds the tab, should also add an entry to the map corresponding 
				// to that tab.  That way this ASSERT should never fire.
				ASSERT(pEMNDetail);
				if (pEMNDetail) {

					// (a.wetta 2007-04-09 16:42) - PLID 25532 - Make sure that if this is an image it won't be resized
					// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
					/*BOOL bOldSizeImageToDetail = pEMNDetail->GetSizeImageToDetail();
					pEMNDetail->SetSizeImageToDetail(bOldSizeImageToDetail | bMaintainImagesSize);*/

					// (j.jones 2005-12-02 10:04) - re-assert this wnd as being the topic's parent
					// it is possible the parent could be NULL because the detail (such as tables
					// and narratives) may have been created before the topicwnd was created
					if(pEMNDetail->m_pEmrItemAdvDlg) {
						if(pEMNDetail->m_pEmrItemAdvDlg->GetParent() != this)
							pEMNDetail->m_pEmrItemAdvDlg->SetParent(this);
					}

					// (c.haag 2004-06-01 10:39) - PL12289 - Now we check if the control needs refreshing. It used to only be
					// if nReloadContentForItemID matched the item ID. Now, we also check the dirty flag of m_bNeedContentRefresh. 
					// (j.jones 2007-07-26 09:33) - PLID 24686 - now we compare that we match the InfoID or InfoMasterID
					if (pEMNDetail->GetNeedContentReload() ||
						(
						(nReloadContentForInfoID != -1 && pEMNDetail->m_nEMRInfoID == nReloadContentForInfoID)
						||
						(nReloadContentForInfoMasterID != -1 && pEMNDetail->m_nEMRInfoMasterID == nReloadContentForInfoMasterID))
						) {
						try {
							CSize szDelta(0,0);

							// (c.haag 2004-06-05 20:27) - PLID 12400 - We may want to recreate the m_pEmrItemAdvDlg object
							// because the content refresh may be a result of the user changing the type of the object.
							CString strNewName;
							short nNewDataType;
							BYTE nNewDataSubType;

							BOOL bItemFound = FALSE;

							// (j.jones 2007-07-25 17:34) - PLID 26810 - we don't need to try and reload the name
							// and item type if we're only just now creating the detail, and not in the middle
							// of a forced reload
							if(!pEMNDetail->GetIsForcedReload()) {
								strNewName = pEMNDetail->GetLabelText();
								// (a.walling 2008-03-17 10:01) - PLID 28857 - Use the pending info type as the NewDataType if it exists
								nNewDataType = pEMNDetail->m_EMRInfoType;
								nNewDataSubType = pEMNDetail->m_EMRInfoSubType;
								bItemFound = TRUE;
							}
							
							if (!bItemFound) {
								// (j.jones 2007-07-26 10:51) - PLID 24686 - if on a template, compare to the master info ID,
								// as our reload may be changing the current InfoID
								_RecordsetPtr prs;
								// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
								// (z.manning 2011-10-21 15:13) - PLID 44649 - Added data sub type
								if(m_bIsTemplate) {
									prs = CreateParamRecordset("SELECT DataType, DataSubType, Name FROM EMRInfoT "
									"INNER JOIN EMRInfoMasterT ON EMRInfoT.ID = EMRInfoMasterT.ActiveEmrInfoID "
									"WHERE EMRInfoMasterT.ID = {INT}", pEMNDetail->m_nEMRInfoMasterID);
								}
								else {
									prs = CreateParamRecordset("SELECT DataType, DataSubType, Name FROM EMRInfoT WHERE ID = {INT}", pEMNDetail->m_nEMRInfoID);
								}
								if (!prs->eof)
								{
									strNewName = AdoFldString(prs, "Name","");
									nNewDataType = AdoFldByte(prs, "DataType");
									nNewDataSubType = AdoFldByte(prs, "DataSubType");
									bItemFound = TRUE;
								}
							}

							if (bItemFound) {
								EmrInfoType eOldInfoType = pEMNDetail->m_EMRInfoType;
								EmrInfoSubType eOldInfoSubType = pEMNDetail->m_EMRInfoSubType;

								// (z.manning 2011-10-21 15:43) - PLID 44649 - We need to check subtype here too.
								BOOL bInfoTypeChanged = !IsSameEmrInfoType(eOldInfoType, eOldInfoSubType, (EmrInfoType)nNewDataType, (EmrInfoSubType)nNewDataSubType);

								// (a.walling 2008-03-24 16:03) - PLID 28811 - If this is a type change, ensure the adv dialog is created.
								if (bInfoTypeChanged && !IsWindow(pEMNDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
									pEMNDetail->EnsureEmrItemAdvDlg(this);
								}

								if (IsWindow(pEMNDetail->m_pEmrItemAdvDlg->GetSafeHwnd()) && bInfoTypeChanged)
								{
									//TES 2/13/2006 - Remove this item from any narratives, we'll re-add it once it's recreated.
									//TES 1/23/2008 - PLID 24157 - Renamed.
									pEMNDetail->m_pParentTopic->GetParentEMN()->HandleDetailChange(pEMNDetail, TRUE);
									// (c.haag 2004-07-09 10:52) - PLID 13373 - Remember where the dialog was
									CRect rcCur;
									pEMNDetail->m_pEmrItemAdvDlg->GetWindowRect(&rcCur);
									pEMNDetail->m_pEmrItemAdvDlg->GetParent()->ScreenToClient(&rcCur);
									pEMNDetail->m_EMRInfoType = (EmrInfoType)nNewDataType;
									pEMNDetail->m_EMRInfoSubType = (EmrInfoSubType)nNewDataSubType;
									//TES 8/12/2004 - Also, the state is no longer valid if the data type changed.
									// (z.manning, 07/26/2007) - PLID 26574 - That's not necessarily true. There are certain cases
									// (generally involving changing from single to multi or vice versa) where we don't need to 
									// reset the state.
									// (z.manning 2011-10-21 15:12) - PLID 44649 - This now checks on subtype too.
									if(InfoTypeChangeRequiresDetailStateReset(eOldInfoType, eOldInfoSubType, (EmrInfoType)nNewDataType, (EmrInfoSubType)nNewDataSubType, pEMNDetail)) {
										// (z.manning 2011-10-21 17:13) - PLID 44649 - When loading the blank set set the image type
										// to undefined because that will ensure we use the image file from content (rather than the
										// one from state, which will be nothing since we're loading a blank state).
										pEMNDetail->SetState(LoadEMRDetailStateBlank(pEMNDetail->m_EMRInfoType, TRUE));
									}
									pEMNDetail->ReloadEMRItemAdvDlg();
									pEMNDetail->ReflectCurrentState();
									//TES 2/13/2006 - Now we re-add to the narratives.
									//TES 1/23/2008 - PLID 24157 - Renamed.
									pEMNDetail->m_pParentTopic->GetParentEMN()->HandleDetailChange(pEMNDetail);

									// (c.haag 2004-07-09 10:24) - We don't want to reset the position
									// other neighboring items; and the overlap we may get from resizing
									// this detail is already handled below (refer to c.haag 7-1-04 9:33 pm)
									/*
									// Now make sure all of the items after this one in the same
									// tab are repositioned. This is done assuming that the
									// array is ordered by tab.
									for (long j=i+1; j < m_arypEMNDetails.GetSize(); j++)
									{
										CEMNDetail *pInfo = (CEMNDetail *)m_arypEMNDetails.GetAt(j);
										if (pEMNDetail->m_nTabIndex == pInfo->m_nTabIndex)
										{
											pInfo->m_bNeedReposition = TRUE;
										}
									}*/
									// (c.haag 2004-07-09 10:57) - Instead, we will ensure the reposition
									// flag is not set, and then restore its position to where it was before
									// it was re-created.
									CRect rcNew;
									pEMNDetail->m_bNeedReposition = FALSE;
									pEMNDetail->m_pEmrItemAdvDlg->SetWindowPos(NULL, rcCur.left,rcCur.top,0,0,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE);
									pEMNDetail->m_pEmrItemAdvDlg->GetClientRect(&rcNew);
									szDelta.cx = rcNew.Width() - rcCur.Width();
									szDelta.cy = rcNew.Height() - rcCur.Height();

									// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
									// that link may now be invalid. If so, this function will fix it.
									pEMNDetail->EnsureLinkedSmartStampTableValid();
								}								

								// (c.haag 2004-07-23 09:32) - PLID 13589 - Make sure our name matches with that of the
								// EMR item. If it changed, we need to reset the merge field too.
								if (pEMNDetail->GetLabelText() != strNewName)
								{
									// Change the item name on the detail
									pEMNDetail->SetLabelText(strNewName);
									pEMNDetail->SetMergeFieldOverride("");

								}					
							}

							BOOL bPreventContentLoad = FALSE;
							if(pOverrideDetail) {
								bPreventContentLoad = pOverrideDetail->m_pEmrItemAdvDlg->m_bPreventContentReload;
							}
							if (IsWindow(pEMNDetail->m_pEmrItemAdvDlg->GetSafeHwnd()) && !bPreventContentLoad)
							{
								// (c.haag 2011-11-01) - PLID 46223 - Only call the loading functions if we're allowed to. 
								// If pDetailToPreventLoadContentAndState is not null, that means the caller already did
								// all this work. However, we still need to resize the detail.
								if (pEMNDetail != pDetailToPreventLoadContentAndState) 
								{
								pEMNDetail->LoadContent();
								pEMNDetail->ReflectCurrentContent();
								pEMNDetail->ReflectCurrentState();
								}
								
								CRect rc;
								pEMNDetail->m_pEmrItemAdvDlg->GetWindowRect(&rc);


								// (c.haag 7-1-04 9:33 pm) - PLID 13241 - The act of refreshing content may cause a difference in the amount
								// of area required by the controls within this detail. We need to make sure the detail dialog can contain
								// all of it, and shift all the controls in the same tab accordingly.
								CSize szOldArea(rc.Width(), rc.Height());
								CSize szNewArea(rc.Width(), rc.Height());
								pEMNDetail->m_pEmrItemAdvDlg->RepositionControls(szNewArea, FALSE);

								// (j.jones 2004-11-16 13:25) - PLID 14706 - don't do this on a template, where it is far more likely that
								// they want to keep the size they have
								//TES 3/8/2006 - PLID 19617 - Also, don't move anything if this detail is about to be repositioned.
								//TES 12/14/2007 - PLID 28377 - Additionally, we don't ever want to resize images, because
								// images already size themselves to fit in the detail, so it can't hurt to leave them the same size,
								// but it COULD hurt to resize them, they might overlap other items or be too big to fit
								// on scrren.
								if(!m_bIsTemplate && !pEMNDetail->m_bNeedReposition && pEMNDetail->m_EMRInfoType != eitImage) {
									if (szNewArea != szOldArea) {
										szDelta = szNewArea - szOldArea;
									}
									// I'm intentionally only doing the resizing if more space is required, as opposed to less
									// space, because we don't know if the user intended for the empty space to be there. We do
									// know that if there's not enough space, they certainly intend to increase the size.
									// Also notice that we indiscrimantly move items; we don't care if an item is not obstructed
									// after the resize. That's by intent so that we don't have to combat selective overlapping,
									// and it looks cleaner and consistent.
									// (c.haag 2004-07-09 09:53) - On the other hand, it really is much cleaner to automatically
									// shrink the dialog. We will be revisiting this in the future anyway.
									if (szDelta.cx != 0 || szDelta.cy != 0) {
										pEMNDetail->m_pEmrItemAdvDlg->SetWindowPos(NULL, 0,0,szNewArea.cx,szNewArea.cy,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOMOVE);
										// Now shift all the other controls that would be affected by these deltas
										for (long j=0; j < m_pTopic->GetEMNDetailCount(); j++) {
											CEMNDetail *pInfo = m_pTopic->GetDetailByIndex(j);
											if (pInfo == pEMNDetail || pInfo->m_pParentTopic != pEMNDetail->m_pParentTopic) continue;
											if (IsWindow(pInfo->m_pEmrItemAdvDlg->GetSafeHwnd())) {
												CRect rcCur;
												pInfo->m_pEmrItemAdvDlg->GetWindowRect(&rcCur);
												if (szDelta.cx > 0 && rcCur.left > rc.left && !(rcCur.top > rc.bottom || rcCur.bottom < rc.top))
													rcCur.left += szDelta.cx;
												if (szDelta.cy > 0 && rcCur.top > rc.top && !(rcCur.left > rc.right || rcCur.right < rc.left))
													rcCur.top += szDelta.cy;
												pInfo->m_pEmrItemAdvDlg->GetParent()->ScreenToClient(&rcCur);
												pInfo->m_pEmrItemAdvDlg->SetWindowPos(NULL, rcCur.left,rcCur.top,0,0,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE);
											}
										}
									}
								}
							}
						} NxCatchAllCall("Error 150 in RepositionDetailsInTopic()", return;);
					}

					pEMNDetail->SetReadOnly(m_bIsReadOnly);
					pEMNDetail->SetAllowEdit(m_bAllowEdit);

					// (a.wetta 2007-04-10 11:17) - PLID 25532 - Set the variable back to its original value
					//pEMNDetail->SetSizeImageToDetail(bOldSizeImageToDetail);
				}
			}
		} NxCatchAllCall("Error 100 in RepositionDetailsInTopic()", return;);


		// (c.haag 2006-03-22 09:10) - PLID 19786 - We are breaking out the repositioning logic
		// from all the other logic that involves changing an actual item's content. We have to
		// do this because of the following scenario where detail #0 is a narrative and detail #1 is
		// a multi-select list; both of which already exist on the topic:
		//
		// 1. Narrative content is reloaded
		// 2. Narrative indirectly causes list detail to reload itself due to calling CEMN::LoadNarrativeMergeFields
		// 3. List detail destroys and recreates controls; no window positioning is done.
		// 4. List detail controls are never resized through the remainder of processing.
		//
		// If the list detail was detail #0, it would never have an opportunity to properly reposition
		// itself unless we did the repositioning in a second loop. Because we have logic that actually
		// causes the internal content of detail A to change if detail B is being loaded, it really should
		// be:
		//
		// DO FOR ALL ITEMS
		//		EnsureContent
		// END
		//
		// DO FOR ALL ITEMS
		//		UpdateView
		// END
		//
		// This is not a speed hit, unless you consider GetDetailByIndex() and i++ costly operations
		//
		try {
			//TES 4/17/2007 - PLID 25607 - If we have some details that have a stored position, and some that don't, and
			// we process ones that don't before some that do, then we run the risk of placing an unpositioned detail in a
			// spot which will overlap with one of the positioned ones that hasn't been positioned yet.  So what we need
			// to do is first process all the details which have stored positions; then, once all those rectangles have
			// been subtracted from our region, we can process all the unpositioned ones, confident that they will not
			// overlap anything.

			//So, first build two lists, one of the positioned details and one of the unpositioned details.
			// (b.savon 2012-06-06 12:29) - PLID 49144 - Our DI settings obj
			DeviceImportSettings disSettings = DeviceImportSettings(gc_nStaticBorderWidth, gc_nStaticBorderWidth);
			long nTop = 0;

			CArray<CEMNDetail*,CEMNDetail*> arPositionedDetails;
			CArray<CEMNDetail*,CEMNDetail*> arUnpositionedDetails;
			for(long i = 0; i < m_pTopic->GetEMNDetailCount(); i++) {
				CEMNDetail *pDetail = m_pTopic->GetDetailByIndex(i);

				if(pDetail->m_rcDefaultClientArea.IsRectNull()) {
					arUnpositionedDetails.Add(pDetail);
				}
				else {
					arPositionedDetails.Add(pDetail);

					// (b.savon 2012-06-07 14:51) - PLID 49144 - Keep track of our positioning 
					//Get the client rect
					CRect rect = pDetail->GetClientArea();

					//Find the lowest detail
					if( rect.bottom > nTop ){
						disSettings.m_nTop = nTop = rect.bottom;
					}

					//If this detail is a device import, save the bounds
					if( pDetail->IsDeviceImport() ){
						disSettings.m_nDeviceTop = rect.top;
						disSettings.m_nLeft = rect.right;
					}
				}
			}
			//Now, collate them into one list, with the positioned details first.
			CArray<CEMNDetail*,CEMNDetail*> arDetails;
			for(i = 0; i < arPositionedDetails.GetSize(); i++) arDetails.Add(arPositionedDetails[i]);
			for(i = 0; i < arUnpositionedDetails.GetSize(); i++) arDetails.Add(arUnpositionedDetails[i]);
			
			//Now, process that collated list in order.
			for (i = 0; i < arDetails.GetSize(); i++) {
				//
				CEMNDetail *pEMNDetail = arDetails[i];
				// Show the subdialog in the right spot
				CRect rc;
				CRect *prcOverride = NULL;
				if(pEMNDetail == pOverrideDetail) {
					prcOverride = new CRect;
					*prcOverride = *prcOverrideDetailRect;
				}
				else if(m_pTopic->GetParentEMN()->IsLockedAndSaved()) {
					//TES 2/12/2007 - PLID 24724 - If we are on a locked EMN, then we need to tell the detail not to 
					// actually change its position, even if its position is not valid.  So, pass the detail's current rect 
					// into the function.
					prcOverride = new CRect;
					*prcOverride = pEMNDetail->GetClientArea();
				}

				pEMNDetail->ShowItemDialogInRgn(this, rgn, rcCurArea, &rc, bSetRegionAndInvalidate, prcOverride, disSettings);
				if(prcOverride) {
					delete prcOverride;
					prcOverride = NULL;
				}
				CRect rcItem = rc;

				// Adjust for the border, thus normalizing placement regardless of whether we're in "edit mode" in this emr right now or not.

				ASSERT(IsWindow(pEMNDetail->m_pEmrItemAdvDlg->GetSafeHwnd()));
				if (IsWindow(pEMNDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
					// (c.haag 2004-07-02 10:08) - PLID 13003 - Ensure the merge info icon button is visible
					// if it needs to be.
					if (pEMNDetail->m_bNeedRefreshMergeButton ||
						pEMNDetail->m_pEmrItemAdvDlg->GetNeedToRepositionControls()) {
						CRect rcWindow;
						pEMNDetail->m_pEmrItemAdvDlg->GetWindowRect(&rcWindow);
						pEMNDetail->m_pEmrItemAdvDlg->RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);
						pEMNDetail->m_bNeedRefreshMergeButton = FALSE;
					}

					// (j.jones 2005-08-26 11:41) - PLID 16475 - this is needed when switching tabs
					// that each have an image, to ensure the proper image will be drawn on
					// (a.walling 2012-06-14 14:36) - PLID 49297 - Don't do this, we don't have tabs in the same sense any longer
					/*if(pEMNDetail->m_pEmrItemAdvDlg->m_pDetail->m_EMRInfoType == eitImage) {
						if(pEMNDetail->m_pEmrItemAdvDlg)
							pEMNDetail->m_pEmrItemAdvDlg->BringWindowToTop();
					}*/

					// First adjust off the whole border, big or small
					CalcClientFromWindow(pEMNDetail->m_pEmrItemAdvDlg, &rc);
					// Then adjust back on the small border
					rc.InflateRect(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));
				}
				rc.InflateRect(10, 10);
				//TES 10/28/2004 - Don't alter the region for hidden items.
				if(pEMNDetail->GetVisible()) {					

					if (CEmrItemAdvDlg::g_pMovingDlg && CEmrItemAdvDlg::g_pMovingDlg->m_pDetail == pEMNDetail) {
						continue;
					}

					if (rcTotalArea.IsRectEmpty()) {
						rcTotalArea.SetRect(0, 0, 1, 1);
					}
					rcTotalArea |= rc;

					CRgn rgnUsed;
					rgnUsed.CreateRectRgnIndirect(rc);
					CRgn rgnNew;
					rgnNew.CreateRectRgn(0,0,1,1);
					rgnNew.CombineRgn(&rgn, &rgnUsed, RGN_DIFF);
					rgn.DeleteObject();
					rgn.Attach(rgnNew.Detach());
				}
			}
		} NxCatchAllCall("Error 110 in RepositionDetailsInTopic()", return;);

		// Our cur area may extend upwards, but our orig is always 0-based, so adjust the cur rect to be 0-based as well.
		// (b.cardillo 2005-11-09 18:16) - PLID 13319 - Since we only deflated the border width from 
		// the top and left (see above), we need to only re-inflate the top and left.
		rcCurArea.InflateRect(gc_nStaticBorderWidth, gc_nStaticBorderWidth, 0, 0);

		// It's possible for there to be a slight (up to 5 pixels) extension to the left or to the top, this is a 
		// result of child windows having thick borders and going off the top and left.  Since we're sure the child 
		// CLIENT AREAs aren't going off the top left, we just ignore these overlapped pixels by forcibly setting 
		// the .left and .top members to 0.  If we don't do this, then in the case where something does extend off 
		// the left or top we get a confusing scrollbar when it doesn't appear to be necessary.
		ASSERT(rcCurArea.left >= -(5+gc_nStaticBorderWidth) && rcCurArea.left <= 0 && rcCurArea.top >= -(5+gc_nStaticBorderWidth) && rcCurArea.top <= 0);
		rcCurArea.left = rcCurArea.top = 0;

		CRect rcOldArea;
		GetWindowRect(&rcOldArea);
		
		//if (0 == rcTotalArea.Height()) {
		//	rcTotalArea.bottom += 128;
		//}
		
		// (a.walling 2012-04-03 16:27) - PLID 49377 - Prevent sizing down when dragging
		//if (NULL != CEmrItemAdvDlg::g_pMovingDlg && 0 != m_szContentSize.cy) {
		//	rcTotalArea = CRect(rcTotalArea.TopLeft(), m_szContentSize);
		//} else {
		//	m_szLastCalcContentSize = rcTotalArea.Size();
		//}

		CSize szContentSize = rcTotalArea.Size();

		if (CEmrItemAdvDlg::g_pSizingDlg) {
			szContentSize.cy = max(szContentSize.cy, m_szContentSize.cy);
			szContentSize.cx = max(szContentSize.cx, m_szContentSize.cx);
		}
	
		m_szContentSize = szContentSize;

		rcTotalArea = CRect(rcTotalArea.TopLeft(), GetDisplayContentSize());

		// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Reposition and notify
		if (rcOldArea.Size() != rcTotalArea.Size()) {
			SetWindowPos(NULL, 0, 0, rcTotalArea.Width(), rcTotalArea.Height(), SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOACTIVATE);
			
			GetEmrTopicView()->ResizeTopicWnd(shared_from_this());
		}
	} NxCatchAll("Error in RepositionDetailsInTopic");
}

LRESULT CEmrTopicWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CWnd::WindowProc(message, wParam, lParam);
}

void CEmrTopicWnd::OnSize(UINT nType, int cx, int cy)
{
	try {
		CWnd::OnSize(nType, cx, cy);
	} NxCatchAll("Error in OnSize()");
}

//BOOL CEmrTopicWnd::OnEraseBkgnd(CDC* pDC) 
//{
//	CEmrTopicView* pTopicView = GetEmrTopicView();
//
//	if (pTopicView) {
//		CRect rcClient;
//		GetClientRect(&rcClient);
//		pDC->FillRect(&rcClient, &pTopicView->GetBackgroundBrush());
//		return TRUE;
//	}
//
//	return CWnd::OnEraseBkgnd(pDC);
//}

LRESULT CEmrTopicWnd::OnEmrItemRemove(WPARAM wParam, LPARAM lParam)
{
	CEMNDetail *pEMNDetail = (CEMNDetail*)wParam;
	if (!pEMNDetail) return FALSE;
	try {

		if (pEMNDetail && m_pTopic) {
			
			//Could this cause the topic to "disappear"?
			BOOL bShowIfEmptyChanged = FALSE;
			if(!m_bIsTemplate && !m_pTopic->ShowIfEmpty()) {
				//We don't actually want this to disappear, because they manually removed the detail.
				m_pTopic->SetShowIfEmpty(TRUE);
				bShowIfEmptyChanged = TRUE;
			}

			// (j.jones 2008-07-29 09:52) - PLID 30729 - track if this topic has any problems
			BOOL bHasProblems = pEMNDetail->HasProblems();

			// This is the normal case
			DeleteItem(pEMNDetail);

			if(bShowIfEmptyChanged) {
				// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
				if(!m_pTopic->IsEmpty(pEMNDetail, FALSE)) {
					//We set it to ShowIfEmpty, but this detail has been removed and there are still details on there, so
					//when they get removed, it should still hide the topic.
					m_pTopic->SetShowIfEmpty(FALSE);
				}
			}

			CEmrItemValueChangedInfo eivci;
			eivci.varOldState = pEMNDetail->GetState();
			eivci.varNewState = pEMNDetail->GetState();
			eivci.bDeleted = TRUE; // (a.walling 2007-07-11 15:25) - PLID 26261 - We are actually deleting the detail

			//Notify our interface.
			ForwardMessageToTreeWnd(NXM_EMR_ITEM_STATECHANGED, (WPARAM)pEMNDetail, (LPARAM)&eivci);

			// (j.jones 2008-07-29 09:52) - PLID 30729 - if we had problems, tell the parent that
			// problems have changed
			if(bHasProblems) {
				ForwardMessageToTreeWnd(NXM_EMR_PROBLEM_CHANGED, 0, 0);
			}

			return TRUE;
		} else {
			// This should never happen
			ASSERT(FALSE);
			return FALSE;
		}
	} NxCatchAllCall("CEmrTopicWnd::OnEmrItemRemove", {
		return FALSE;
	});
}

void CEmrTopicWnd::DeleteItem(CEMNDetail *pItemToDelete)
{
	ASSERT(pItemToDelete);

	//Tell our parent we are about to delete this item
	//DRT 7/30/2007 - PLID 26876 - The pre-delete message is now sent by the EMRTopic::RemoveDetail function.
	//ForwardMessageToTreeWnd(NXM_PRE_DELETE_EMR_ITEM, (WPARAM)pItemToDelete, 0);

	//DRT 8/3/2007 - PLID 26931 - Use the new unspawning utility to revoke things en masse.
	CEMNUnspawner eu(m_pTopic->GetParentEMN());
	CEMNDetailArray aryDetails;
	aryDetails.Add(pItemToDelete);
	// (z.manning 2010-03-10 17:45) - PLID 37571 - Also add linked smart stamp image and table as they
	// will be removed too.
	// (z.manning 2011-01-21 09:42) - PLID 42338 - Support multiple images per smart stamp table
	aryDetails.Append(*(pItemToDelete->GetSmartStampImageDetails()));
	if(pItemToDelete->GetSmartStampTableDetail() != NULL) {
		// (z.manning 2011-01-27 17:17) - PLID 42336 - If this is a smart stamp image we may not be removing
		// the smart stamp table because the table could be linked to more than one image now.
		if(pItemToDelete->GetSmartStampTableDetail()->GetSmartStampImageDetails()->GetCount() == 1) {
			aryDetails.Add(pItemToDelete->GetSmartStampTableDetail());
		}
		else {
			// (z.manning 2011-01-27 17:23) - PLID 42336 - If this assert fires it means the table linked to
			// this image has no smart stamp images which obviously shouldn't be possible since the image
			// we're deleting is linked to that table.
			ASSERT(pItemToDelete->GetSmartStampTableDetail()->GetSmartStampImageDetails()->GetCount() > 1);
		}
	}
	eu.RemoveActionsByDetails(&aryDetails);

	//now delete the item
	m_pTopic->RemoveDetail(pItemToDelete);
}

void CEmrTopicWnd::AddDetail(CEMNDetail *pDetail) 
{
	//Set the topic's position.
	// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
	RepositionDetailsInTopicByInfoID(pDetail->m_nEMRInfoID, FALSE);

	// (a.walling 2007-09-28 12:38) - PLID 25549 - We need to know where the topic is
	// before sending this message, otherwise the preview has no idea where to place it
	// relative to other details
	//Tell our parent.
	// (a.walling 2007-04-10 09:42) - PLID 25549
	m_pEmrTreeWnd->SendMessage(NXM_POST_ADD_EMR_ITEM, (WPARAM)pDetail, (LPARAM)FALSE);
}

void CEmrTopicWnd::ShowDetail(CEMNDetail *pDetail, BOOL bIsInitialLoad)
{
	pDetail->SetVisible(TRUE, TRUE, bIsInitialLoad);
	if(pDetail->m_pParentTopic->GetParentEMN()) {
		pDetail->m_pParentTopic->GetParentEMN()->UpdateMergeConflicts(pDetail->GetMergeFieldName(TRUE));
	}
	//This was just 'added' to the emn
	// (a.walling 2007-04-10 09:41) - PLID 25549
	// (a.walling 2007-08-08 10:42) - PLID 27017 - Some things have changed, we'll wait to send this message
	// until everything has been reposistioned and created (that is, we'll send it in OnEmrItemAdded)
	// m_pEmrTreeWnd->SendMessage(NXM_POST_ADD_EMR_ITEM, (WPARAM)pDetail, (LPARAM)bIsInitialLoad);
}

// (a.walling 2012-03-29 08:04) - PLID 49297 - These inlines make it a bit easier to check what sides are being used in a size operation
inline bool SizingLeftBorder(UINT hit) {
	return hit == HTLEFT || hit == HTTOPLEFT || hit == HTBOTTOMLEFT;
}

inline bool SizingRightBorder(UINT hit) {
	return hit == HTRIGHT || hit == HTTOPRIGHT || hit == HTBOTTOMRIGHT;
}

inline bool SizingTopBorder(UINT hit) {
	return hit == HTTOP || hit == HTTOPLEFT || hit == HTTOPRIGHT;
}

inline bool SizingBottomBorder(UINT hit) {
	return hit == HTBOTTOM || hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT;
}

// (a.walling 2012-03-29 08:04) - PLID 49297 - Handle the new rect of an item that is being sized
// (a.walling 2012-04-02 08:29) - PLID 49304 - Unified size/move handler
void CEmrTopicWnd::HandleItemSizeMove(CEmrItemAdvDlg* pWnd, UINT nSide, LPRECT prcNewArea)
{
	try {
		CRect rcClient;
		GetClientRect(&rcClient);	

		//CSize szAdjust;
		//{
		//	CRect rcAdjust(0,0,0,0);
		//	pWnd->CalcWindowRect(&rcAdjust);
		//	szAdjust.SetSize(rcAdjust.Width() / 2, rcAdjust.Height() / 2);
		//}
		
		CRect rcNew(*prcNewArea);
		ScreenToClient(&rcNew);
		
		//CRect rcClientMax(rcClient.TopLeft(), m_szLastCalcContentSize);
		//if (IsKeyDown(VK_SHIFT)) {
		//	// shift means they know what they are doing, I suppose -- so no limit
		//	rcClientMax.right = LONG_MAX;
		//	rcClientMax.bottom = LONG_MAX;
		//} else {
		//	rcClientMax.bottom += ((rcNew.Height() + SIZE_EMR_SNAP_TO_GRID_BLOCK - 1) / SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK;
		//	rcClientMax.bottom += 64;
		//}

		//rcNew.DeflateRect(szAdjust);		

		if (nSide == 0) {
			if (rcClient.left > rcNew.left) {
				rcNew.OffsetRect(rcClient.left - rcNew.left, 0);
			}
			if (rcClient.top > rcNew.top) {
				rcNew.OffsetRect(0, rcClient.top - rcNew.top);
			}
		}
		
		// (b.cardillo 2006-06-20 12:47) - PLID 13897 - Snap to grid if the shift key is not held 
		// down.  NOTE: If we ever make the snapping to the grid optional or make it so the user 
		// can specify how big the blocks of the grid are, we will need to move this code and put 
		// it in the CEmrTopicWnd class instead, because there it can be made aware of user 
		// preferences and such.  Here, we are meant to be more universal, and so it is the 
		// appropriate place for this code right now.
		if (GetRemotePropertyInt("EMRAutoAlignToGrid", 1, 0, GetCurrentUserName(), TRUE) == 1 && !IsKeyDown(VK_SHIFT)) {
			if (0 == nSide) {
				// Currently we snap to a SIZE_EMR_SNAP_TO_GRID_BLOCK-pixel grid.  Maybe someday we can 
				// make this user-defined too.
				CPoint snapOffset(
					((rcNew.left / SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK) - rcNew.left
					, ((rcNew.top / SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK) - rcNew.top 
				);
				rcNew.OffsetRect(snapOffset.x, snapOffset.y);
			} else {
				if (SizingLeftBorder(nSide)) {
					rcNew.left = (rcNew.left / SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK;
				}
				if (SizingTopBorder(nSide)) {
					rcNew.top = (rcNew.top / SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK;
				}				
				if (SizingRightBorder(nSide)) {
					rcNew.right = rcNew.right + (rcNew.right % SIZE_EMR_SNAP_TO_GRID_BLOCK);
				}				
				if (SizingBottomBorder(nSide)) {
					rcNew.bottom = rcNew.bottom + (rcNew.bottom % SIZE_EMR_SNAP_TO_GRID_BLOCK);
				}
			}
		}

		//if (rcNew.bottom > rcClientMax.bottom) {
		//	rcNew.OffsetRect(0, rcClientMax.bottom - rcNew.bottom);
		//}
			
		//rcNew.DeflateRect(-szAdjust);

		*prcNewArea = rcNew;
		ClientToScreen(prcNewArea);

		if (!CEmrItemAdvDlg::g_pMovingDlg) {
			GetEmrTopicView()->ForceRefreshTopicWindowPositions();
			//CSize szDisplayContentSize = GetDisplayContentSize();
			//CSize szNewContentSize(max(m_szContentSize.cx, rcNew.right + 7), max(m_szContentSize.cy, rcNew.bottom + 7));

			//if (szNewContentSize != m_szContentSize) {
			//	m_szContentSize = szNewContentSize;
			//	GetEmrTopicView()->ResizeTopicWnd(shared_from_this());
			//}
		}

		GetEmrTreeWnd()->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)GetTopic(), 0);

	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicWnd::NotifyItemPosChanged(class CEmrItemAdvDlg* pWnd)
{
	GetEmrTopicView()->SetForceNextRefreshTopicWindowPositions();
}

LRESULT CEmrTopicWnd::OnEmrItemAdded(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEMNDetail *pDetail = (CEMNDetail*)wParam;
		ShowDetail(pDetail, (BOOL)lParam);
		// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
		RepositionDetailsInTopicByInfoID(pDetail->m_nEMRInfoID, FALSE);
		//This was just 'added' to the emn
		// (a.walling 2007-04-10 09:41) - PLID 25549
		// (a.walling 2007-08-08 10:42) - PLID 27017 - Let the treewnd (and preview control) know this item has been added.
		m_pEmrTreeWnd->SendMessage(NXM_POST_ADD_EMR_ITEM, (WPARAM)pDetail, (LPARAM)lParam);
	} NxCatchAll("Error in OnEmrItemAdded");

	return 0;
}

LRESULT CEmrTopicWnd::OnConvertRectForData(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		LPRECT pRect = (LPRECT)wParam;
		ScreenToClient(pRect);
	} NxCatchAll("Error in OnConvertRectForData");

	return 0;
}

// (b.savon 2012-06-06 12:26) - PLID 49144 - Added optional device import flag
// (j.jones 2010-04-01 10:57) - PLID 37980 - Added optional overrides to default an image file and a label
void CEmrTopicWnd::AddImage(OPTIONAL CString strImageFile /*= ""*/, OPTIONAL CString strLabel /*= ""*/, OPTIONAL BOOL bDeviceImport /*= FALSE*/)
{
	//Create the detail.
	// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(m_pTopic, "EMRTopicWnd AddImage");
	//Load it.
	//TES 10/8/2007 - PLID 27660 - This function can only be called in response to user action, therefore it's never part
	// of the initial load
	SourceActionInfo saiBlank; // (z.manning 2009-03-11 10:46) - PLID 33338
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromInfoID(EMR_BUILT_IN_INFO__IMAGE, m_bIsTemplate, FALSE, saiBlank);

	// (j.jones 2010-04-01 11:06) - PLID 37980 - if an image was provided, use it
	if(!strImageFile.IsEmpty()) {
		if(strImageFile.Find("\\") == -1) {
			pDetail->SetImage(strImageFile, itPatientDocument);
		}
		else {
			pDetail->SetImage(strImageFile, itAbsolutePath);
		}
	}
	if(!strLabel.IsEmpty()) {
		pDetail->SetLabelText(strLabel);
		pDetail->SetMergeFieldOverride(strLabel);
	}

	// (b.savon 2012-06-06 12:26) - PLID 49144 - If were importing a device image detail, then set our flag
	if( bDeviceImport ){
		pDetail->SetDeviceImport(bDeviceImport);
	}

	//Add to the memory object.
	// (c.haag 2007-05-22 12:07) - PLID 26095 - Don't increment the reference;
	// we are ignoring the detail when this function loses scope. The topic will
	// own the only reference.
	m_pTopic->AddDetail(pDetail, FALSE, FALSE);
	//Update the screen.
	// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
	RepositionDetailsInTopicByInfoID(EMR_BUILT_IN_INFO__IMAGE, FALSE);

	// (a.walling 2007-10-15 14:41) - PLID 25548 - Tell our parent to update the position of this detail 
	m_pEmrTreeWnd->SendMessage(NXM_EMN_DETAIL_UPDATE_PREVIEW_POSITION, WPARAM(pDetail), LPARAM(m_pTopic->GetParentEMN()));
}

// (z.manning 2008-10-17 10:19) - PLID 23110 - Adds a sigature image detail to the current topic.
void CEmrTopicWnd::AddSignatureImage()
{
	CSignatureDlg dlgSignature(this);
	// (z.manning 2008-10-17 10:38) - PLID 23110
	dlgSignature.m_bRequireSignature = TRUE;
	dlgSignature.m_bAutoCommitIfSignature = TRUE;
	// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
	dlgSignature.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordEMR", 1, 0, GetCurrentUserName()) == 1);
	if(dlgSignature.DoModal() == IDOK)
	{
		AddSignatureImage(&dlgSignature);
	}
}

// (z.manning 2011-10-31 14:05) - PLID 44594 - Added an overload that takes a signature dialog
// (j.jones 2013-08-08 13:35) - PLID 42958 - added optional user ID and username, for cases when
// the signature is being created by a user who is not the logged-in user
void CEmrTopicWnd::AddSignatureImage(CSignatureDlg *pdlgSignature,
									 OPTIONAL long nSignatureUserID /*= GetCurrentUserID()*/,
									 OPTIONAL CString strSignatureUserName /*= GetCurrentUserName()*/)
{
	// (j.jones 2013-08-08 13:48) - PLID 42958 - if the provided user is not the current user,
	// bulk cache the relevant preferences for the user who is about to sign
	// (b.savon 2014-07-14 07:41) - PLID 62743 - Added GenerateClinicalSummaryAfterSignature
	if(strSignatureUserName != GetCurrentUserName()) {
		CString strBulkCacheName = "CEmrTopicWnd_AddSignatureImage_" + strSignatureUserName;
		g_propManager.CachePropertiesInBulk(strBulkCacheName, propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'HideEMNDefaultSignatureTitle' "
			"OR Name = 'EmrPostSignatureInsertStatus' "
			"OR Name = 'AutoPromptForDefaultSignature' "
			"OR Name = 'DefaultEMRImagePenColor' "
			"OR Name = 'GenerateClinicalSummaryAfterSignature' "
			")",
			_Q(strSignatureUserName));
	}

	//Create the detail.
	// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(m_pTopic, "EMRTopicWnd AddSignatureImage");
	//Load it.
	SourceActionInfo saiBlank; // (z.manning 2009-03-11 10:46) - PLID 33338
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromInfoID(EMR_BUILT_IN_INFO__IMAGE, m_bIsTemplate, FALSE, saiBlank);
	const CString strLabel = "Signature";
	pDetail->SetLabelText(strLabel);

	// (b.spivey, February 26, 2013) - PLID 39697 - If this preference is set, automatically hide default signature. 
	// (j.jones 2013-08-08 13:48) - PLID 42958 - get the preference for the signing user, who might not be the logged-in user
	if (GetRemotePropertyInt("HideEMNDefaultSignatureTitle", TRUE, 0, strSignatureUserName, true)) {
		CString str = GetPreviewFlagsDescriptionForAudit(pDetail->GetPreviewFlags()); 
		pDetail->SetPreviewFlags(pDetail->GetPreviewFlags() ^ epfHideTitle, FALSE, TRUE); 
	}

	pDetail->SetMergeFieldOverride(strLabel);
	pDetail->SetImage(pdlgSignature->GetSignatureFileName(), itSignature);
	_variant_t varInk = pdlgSignature->GetSignatureInkData();
	if(varInk.vt != VT_EMPTY && varInk.vt != VT_NULL) {
		if(!m_bIsTemplate) {
			pDetail->SetInkData(varInk);
			pDetail->SetInkAdded();
		}
	}
	// (j.jones 2010-04-12 15:17) - PLID 16594 - load the text from the signature
	_variant_t varInkText = pdlgSignature->GetSignatureTextData();
	if(varInkText.vt != VT_EMPTY && varInkText.vt != VT_NULL) {
		if(!m_bIsTemplate) {

			//the loaded text is the words "Date/Time", we need to replace it with the actual date/time
			CNxInkPictureText nipt;
			nipt.LoadFromVariant(varInkText);

			COleDateTime dtSign = COleDateTime::GetCurrentTime();
			_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
			if(!rs->eof){
				dtSign = AdoFldDateTime(rs, "CurDateTime");
			}
			rs->Close();

			CString strDate;
		if(pdlgSignature->GetSignatureDateOnly()) {
				strDate = FormatDateTimeForInterface(dtSign, NULL, dtoDate);					
			}
			else {
				strDate = FormatDateTimeForInterface(dtSign, DTF_STRIP_SECONDS, dtoDateTime);					
			}

			//technically, there should only be one SIGNATURE_DATE_STAMP_ID in the array,
			//and currently we don't support more than one stamp in the signature
			//anyways, but just incase, replace all instances of the SIGNATURE_DATE_STAMP_ID with
			//the current date/time
			for(int i=0; i<nipt.GetStringCount(); i++) {
				if(nipt.GetStampIDByIndex(i) == SIGNATURE_DATE_STAMP_ID) {
					nipt.SetStringByIndex(i, strDate);
				}
			}
			varInkText = nipt.GetAsVariant();

			pDetail->SetImageTextData(varInkText);
			pDetail->SetImageTextAdded();
		}
	}

	// (z.manning 2011-09-23 14:45) - PLID 42648 - We may want to scale the size of this detail's dialog.
	pDetail->SetItemAdvDlgScalingFactor(pdlgSignature->GetSignatureEmrScaleFactor());

	// (j.jones 2013-08-08 14:08) - PLID 42958 - If the signing user is not the logged-in user, 
	// track that this detail was added by another user, and who it was.
	// Later, auditing will use this information for a unique audit.
	if(nSignatureUserID != GetCurrentUserID()) {
		pDetail->SetSignatureAddedByAnotherUser(true, strSignatureUserName);
	}

	//Add to the memory object.
	// (c.haag 2007-05-22 12:07) - PLID 26095 - Don't increment the reference;
	// we are ignoring the detail when this function loses scope. The topic will
	// own the only reference.
	m_pTopic->AddDetail(pDetail, FALSE, FALSE);
	//Update the screen.
	RepositionDetailsInTopicByInfoID(EMR_BUILT_IN_INFO__IMAGE, FALSE);

	if(m_pEmrTreeWnd != NULL)
	{
		// (a.walling 2007-10-15 14:41) - PLID 25548 - Tell our parent to update the position of this detail 
		m_pEmrTreeWnd->SendMessage(NXM_EMN_DETAIL_UPDATE_PREVIEW_POSITION, WPARAM(pDetail), LPARAM(m_pTopic->GetParentEMN()));

		// (z.manning 2011-09-02 16:53) - PLID 32123 - Check the preference to see if we should update the status
		// (j.jones 2013-08-08 13:48) - PLID 42958 - get the preference for the signing user, who might not be the logged-in user
		long nStatusPref = GetRemotePropertyInt("EmrPostSignatureInsertStatus", -1, 0, strSignatureUserName);
		if(nStatusPref != -1)
		{
			EMNStatus emnNewStatus;
			emnNewStatus.nID = nStatusPref;
			if(GetEmnStatusName(nStatusPref, emnNewStatus.strName)) {
				m_pEmrTreeWnd->SendMessage(NXM_EMN_STATUS_CHANGING, (WPARAM)&emnNewStatus, (LPARAM)m_pTopic->GetParentEMN());
			}
		}
	}

	// (b.savon 2014-07-14 07:47) - PLID 62743 - Respect the clinical summary generation preference when adding a signature to the patient's EMN
	if (m_bIsTemplate == FALSE){
		CString strUser = strSignatureUserName.IsEmpty() ? GetCurrentUserName() : strSignatureUserName;
		if (strUser.IsEmpty()){
			//This shouldn't happen because there has got to be a user signing this note; but if it does, default to 'Never' by using the <None> identifier
			ASSERT(FALSE);
			strUser = "<None>";
		}

		ClinicalSummaryAfterSignature csasGenerateClinicalSummary = (ClinicalSummaryAfterSignature)GetRemotePropertyInt("GenerateClinicalSummaryAfterSignature", 0, 0, strUser);
		switch (csasGenerateClinicalSummary){
			case csasNever: /*Do Nothing but break*/ break;
			case csasAlways:
			{
				m_pEmrTreeWnd->GenerateClinicalSummary();
			}
			break;
			case csasPrompt:
			{
				m_pEmrTreeWnd->GenerateClinicalSummary(true);
			}
			break;
		}
	}
}

// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
void CEmrTopicWnd::AddSignatureImageForAnotherUser()
{
	//get the last user they inserted a signature for
	long nDefaultUserID = GetRemotePropertyInt("EMRLastOtherUserSignatureID", -1, 0, GetCurrentUserName(), true);

	//prompt for a user & password
	CUserVerifyDlg dlgUser;
	if(IDOK != dlgUser.DoModal("Add Another User's Signature", "Select the user to enter a signature for, and enter their password:", nDefaultUserID)) {
		//return silently
		return;
	}

	long nSignatureUserID = dlgUser.m_nSelectedUserID;
	CString strSignatureUserName = dlgUser.m_strSelectedUserName;
	if(nSignatureUserID == -1) {
		//the dialog should not have returned IDOK without a valid user ID
		ASSERT(FALSE);
		//since they didn't click cancel, we need to explain why this won't work
		MessageBox("No valid user was selected. A signature will not be added.");
		return;
	}

	CSignatureDlg dlgSignature(this);
	dlgSignature.m_bRequireSignature = TRUE;
	dlgSignature.m_bAutoCommitIfSignature = TRUE;
	//do not check the SignatureCheckPasswordEMR preference, and do not
	//set m_bCheckPasswordOnLoad to true, because the user just entered their password already

	//tell the signature dialog that this is not for the logged-in user
	if(dlgSignature.DoModal(nSignatureUserID, strSignatureUserName) == IDOK)
	{
		AddSignatureImage(&dlgSignature, nSignatureUserID, strSignatureUserName);

		//save this user as the last one they inserted a signature for
		SetRemotePropertyInt("EMRLastOtherUserSignatureID", nSignatureUserID, 0, GetCurrentUserName());
	}
}

// (c.haag 2008-06-05 17:41) - PLID 30319 - Add a detail from a text macro
void CEmrTopicWnd::AddTextMacro()
{
	if(NULL != m_pEmrTreeWnd) {

		CEmrTextMacroDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			// Instantiate the detail

			// Create the detail.
			// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
			CEMNDetail *pDetail = CEMNDetail::CreateDetail(m_pTopic, "EMRTopicWnd AddTextMacro");
			// Load it
			SourceActionInfo saiBlank; // (z.manning 2009-03-11 10:46) - PLID 33338
			// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
			pDetail->LoadFromInfoID(EMR_BUILT_IN_INFO__TEXT_MACRO, m_bIsTemplate, FALSE, saiBlank);
			// Update the detail information for this macro
			// (z.manning 2009-06-17 10:46) - PLID 34650 - Call RequestStateChange instead of SetState so
			// that we handle everything we need to by adding a new detail that may already having a state,
			// including topic completion status.
			pDetail->RequestStateChange(_bstr_t(dlg.m_strResultTextData));
			pDetail->SetLabelText(dlg.m_strDetailName);
			// Add to the memory object.
			m_pTopic->AddDetail(pDetail, FALSE, FALSE);
			// Update the screen.
			RepositionDetailsInTopicByInfoID(EMR_BUILT_IN_INFO__TEXT_MACRO, FALSE);
			// Tell our parent to update the position of this detail 
			m_pEmrTreeWnd->SendMessage(NXM_EMN_DETAIL_UPDATE_PREVIEW_POSITION, WPARAM(pDetail), LPARAM(m_pTopic->GetParentEMN()));

		} // if (IDOK == dlg.DoModal()) {

	} // if(NULL != pTreeWnd) {
}

void CEmrTopicWnd::OnDestroy()
{
	//DeleteObject(m_brBackground);

	try {
		// (a.walling 2011-10-20 21:22) - PLID 46075 - Ensure the interface window ptr is not associated with our now-destroyed window
		// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
		if (m_pTopic && m_pTopic->GetTopicWndRaw() == this) {
			m_pTopic->ResetTopicWnd();
		}

		m_pTopic = NULL;

		/*
		CEmrTopicView* pView = polymorphic_downcast<CEmrTopicView*>(GetParent());
		*/
		
		CWnd::OnDestroy();
	} NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

void CEmrTopicWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	//
	// (c.haag 2006-07-06 11:55) - PLID 21296 - Post a message to all the details
	// to let them know that the visibility of this topic has changed
	//
	for (int i=0; i < m_pTopic->GetEMNDetailCount(); i++) {
		CEMNDetail* pDetail = m_pTopic->GetDetailByIndex(i);
		if (pDetail && pDetail->m_pEmrItemAdvDlg && IsWindow(pDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
			pDetail->m_pEmrItemAdvDlg->SendMessage(NXM_EMN_ON_SHOW_TOPIC, bShow);
		}
	}

}

// (a.walling 2012-03-22 16:50) - PLID 49141 - All more info changed messages go to the EMN's interface now

void CEmrTopicWnd::HandleProblemChange(CEmrProblem *pChangedProblem)
{
	//TES 10/30/2008 - PLID 31269 - Just pass on to all our details.
	for (int i=0; i < m_pTopic->GetEMNDetailCount(); i++) {
		CEMNDetail* pDetail = m_pTopic->GetDetailByIndex(i);
		if (pDetail && pDetail->m_pEmrItemAdvDlg && IsWindow(pDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
			pDetail->m_pEmrItemAdvDlg->HandleProblemChange(pChangedProblem);
		}
	}
}

// (z.manning 2009-08-26 10:01) - PLID 33911 - Added message to insert signature
// (a.walling 2009-10-29 09:37) - PLID 36089 - Made InsertSignature generic
LRESULT CEmrTopicWnd::OnInsertStockEmrItem(WPARAM wParam, LPARAM lParam)
{
	try
	{
		ASSERT((CEMRTopic*)wParam == m_pTopic);
		ASSERT(!m_bIsReadOnly);

		// (a.walling 2009-10-29 09:37) - PLID 36089
		// (j.jones 2013-08-07 15:33) - PLID 42958 - converted this function to use
		// the StockEMRItem enumeration
		switch (lParam) {
			case seiSignatureImage:
				AddSignatureImage();
				break;
			// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
			case seiAnotherUsersSignatureImage:
				AddSignatureImageForAnotherUser();
				break;
			case seiTextMacro:
				AddTextMacro();
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2010-04-01 10:41) - PLID 37980 - added ability to tell the topic to add a given image
LRESULT CEmrTopicWnd::OnAddImageToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		if(m_pTopic == NULL) {
			//shouldn't be possible
			ASSERT(FALSE);
			return 0;
		}

		if(m_pTopic->GetParentEMN()->IsLockedAndSaved()) {
			//can't add to a locked EMN!
			//Assert, because the code should not have even attempted this
			ASSERT(FALSE);
			return 0;
		}

		BSTR bstrImageFile = (BSTR)wParam;
		CString strImageFile(bstrImageFile);
		SysFreeString(bstrImageFile);

		BSTR bstrDeviceName = (BSTR)lParam;
		CString strDeviceName(bstrDeviceName);
		SysFreeString(bstrDeviceName);

		CString strFullPath = strImageFile;
		if(strImageFile.Find("\\") == -1) {
			strFullPath = GetPatientDocumentPath(m_pTopic->GetParentEMN()->GetParentEMR()->GetPatientID()) ^ strImageFile;
		}

		if(!DoesExist(strFullPath)) {
			//this image doesn't exist!
			Log("CEmrTopicWnd::OnAddImageToEMR - Could not add image file '%s' to the EMR because the file does not exist.", strFullPath);
			return 0;
		}

		// (b.savon 2012-06-06 12:27) - PLID 49144 - This is the message handler for importing from a device, make sure we
		// pass our flag in so that we position the details correctly.
		AddImage(strImageFile, strDeviceName, TRUE);

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
LRESULT CEmrTopicWnd::OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		if(m_pTopic == NULL) {
			//shouldn't be possible			
			ASSERT(FALSE);
			return 0;
		}

		if(m_pTopic->GetParentEMN()->IsLockedAndSaved()) {
			//can't add to a locked EMN!
			//Assert, because the code should not have even attempted this
			ASSERT(FALSE);
			return 0;
		}

		DevicePluginUtils::TableContent *pTableContent = (DevicePluginUtils::TableContent*)wParam;
		if(pTableContent == NULL) {
			Log("CEmrTopicWnd::OnAddGenericTableToEMR - Could not add a generic table to the EMR because the table pointer is empty.");
			return 0;
		}

		if(!pTableContent->IsValid()) {
			Log("CEmrTopicWnd::OnAddGenericTableToEMR - Could not add a generic table to the EMR because the table pointer is invalid.");
			return 0;
		}

		BSTR bstrTableName = (BSTR)lParam;
		CString strTableName(bstrTableName);
		SysFreeString(bstrTableName);

		AddGenericTable(pTableContent, strTableName);

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
void CEmrTopicWnd::AddGenericTable(DevicePluginUtils::TableContent *pGenericTableContent, CString strTableLabel)
{
	//this should only be called on a patient EMN
	if(m_bIsTemplate) {
		ASSERT(FALSE);
		return;
	}

	//this should never be NULL
	if(pGenericTableContent == NULL) {
		ASSERT(FALSE);
		return;
	}

	long nRowsNeeded = pGenericTableContent->aryRows.GetSize();
	long nColumnsNeeded = pGenericTableContent->aryColumns.GetSize();

	if(nRowsNeeded == 0 || nColumnsNeeded == 0) {
		ThrowNxException("CEmrTopicWnd::AddGenericTable was given invalid data!");
	}

	//grab the right info ID
	long nEMRInfoID = m_pTopic->GetParentEMN()->GetParentEMR()->GetCurrentGenericTableInfoID();

	//make sure we have enough records, branch if needed, which will update nEMRInfoID
	EnsureGenericTableHasEnoughRecords(nEMRInfoID, nRowsNeeded, nColumnsNeeded);

	//Create the table
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(m_pTopic, "EMRTopicWnd AddGenericTable");

	//Load it
	SourceActionInfo saiBlank;
	//pass in the generic table content
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromInfoID(nEMRInfoID, m_bIsTemplate, FALSE, saiBlank, -1, -1, -1, -1, pGenericTableContent);

	if(!strTableLabel.IsEmpty()) {
		pDetail->SetLabelText(strTableLabel);
		pDetail->SetMergeFieldOverride(strTableLabel);
	}
	
	//Add to the memory object
	m_pTopic->AddDetail(pDetail, FALSE, FALSE);
	
	//Update the screen
	RepositionDetailsInTopicByInfoID(nEMRInfoID, FALSE);

	//tell our parent to update the position of this detail 
	m_pEmrTreeWnd->SendMessage(NXM_EMN_DETAIL_UPDATE_PREVIEW_POSITION, WPARAM(pDetail), LPARAM(m_pTopic->GetParentEMN()));
}

// (a.walling 2012-05-16 15:22) - PLID 50430 - This window is transparent so don't set focus in mouse handlers
void CEmrTopicWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);
}

void CEmrTopicWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnMButtonDown(nFlags, point);
}

void CEmrTopicWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);
}

LRESULT CEmrTopicWnd::OnNcHitTest(CPoint pt)
{
	return HTTRANSPARENT;
}
