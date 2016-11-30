// EmrTopicView.cpp : implementation file
//

#include "stdafx.h"
#include "EMN.h"
#include "EmrTopicView.h"
#include "EmrTopic.h"
#include "EmrDocument.h"
#include <foreach.h>
#include "EMNMoreInfoDlg.h"
#include "PicContainerDlg.h"
#include <map>
#include <NxAlgorithm.h>
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include <NxAdvancedUILib/NxRibbonListButton.h>
#include "EmrColors.h"

// (a.walling 2011-10-20 21:22) - PLID 46078 - Facelift - EMR Document / View / Command routing etc

// (a.walling 2012-02-29 06:42) - PLID 46644 - Supprting template editor


// (a.walling 2013-03-07 08:33) - PLID 55494 - Disable topic animation over RDP - check hidden flag


static double GetAnimationDuration()
{	
	// (a.walling 2013-03-07 08:33) - PLID 55494 - Disable topic animation over RDP - no remote prop check yet, maybe sometime, just local registry for now
	static bool bDisableTopicScrolling = !!NxRegUtils::ReadLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("Emr_DisableTopicScrolling"), 0);
	if (bDisableTopicScrolling || (0 != GetSystemMetrics(SM_REMOTESESSION))) {
		return 0;
	} else {
		BOOL bListBoxSmoothing = TRUE;
		if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &bListBoxSmoothing, 0) && !bListBoxSmoothing) {
			return 0;
		}
	}

	double animationDuration = (static_cast<double>(GetDoubleClickTime()) / 4);

	if (animationDuration < 0) {
		animationDuration = 0;
	}

	return animationDuration;
}

IMPLEMENT_DYNCREATE(CEmrTopicView, CScrollView)

CEmrTopicView::CEmrTopicView()
	: m_bForceRefreshNextRefreshTopicWindowPositions(false)
{
}

CEmrTopicView::~CEmrTopicView()
{
}

BEGIN_MESSAGE_MAP(CEmrTopicView, CScrollView)	

	////
	/// UI update

	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_EMN, &CEmrTopicView::OnUpdateNewEMN)

	ON_UPDATE_COMMAND_UI_RANGE(ID_EMR_ADD_ITEM, ID_EMR_ADD_OTHER_USERS_SIGNATURE, &CEmrTopicView::OnUpdate_AddItems)

	ON_UPDATE_COMMAND_UI(ID_EMR_ADD_TOPICS_BTN_MENU, &CEmrTopicView::OnUpdateClassicBtnAddTopics)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_TOPIC, &CEmrTopicView::OnUpdateNewTopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_SUBTOPIC, &CEmrTopicView::OnUpdateNewSubtopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_IMPORT_TOPICS, &CEmrTopicView::OnUpdateImportTopics)
	ON_UPDATE_COMMAND_UI(ID_EMR_IMPORT_SUBTOPICS, &CEmrTopicView::OnUpdateImportSubtopics)

	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_TODO, &CEmrTopicView::OnUpdateNewTodo)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_RECORDING, &CEmrTopicView::OnUpdateNewRecording)

	ON_UPDATE_COMMAND_UI(ID_EMR_TOPIC_LIST, &CEmrTopicView::OnUpdateTopicList)	// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
	ON_UPDATE_COMMAND_UI(ID_EMR_FIRST_TOPIC, &CEmrTopicView::OnUpdateFirstTopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIOUS_TOPIC, &CEmrTopicView::OnUpdatePreviousTopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEXT_TOPIC, &CEmrTopicView::OnUpdateNextTopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_LAST_TOPIC, &CEmrTopicView::OnUpdateLastTopic)

	////
	/// Commands

	ON_COMMAND(ID_EMR_ADD_ITEM, &CEmrTopicView::OnAddItem)
	ON_COMMAND(ID_EMR_ADD_IMAGE, &CEmrTopicView::OnAddImage)
	ON_COMMAND(ID_EMR_ADD_TEXT_MACRO, &CEmrTopicView::OnAddTextMacro)	

	// (j.jones 2013-08-07 17:30) - PLID 42958 - In the ribbon, ID_EMR_ADD_SIGNATURE is the button
	// that is also the dropdown, and ID_EMR_ADD_YOUR_SIGNATURE is the specific dropdown for adding
	// your signature image. Both controls call the same function.
	ON_COMMAND(ID_EMR_ADD_SIGNATURE, &CEmrTopicView::OnAddSignature)
	ON_COMMAND(ID_EMR_ADD_YOUR_SIGNATURE, &CEmrTopicView::OnAddSignature)
	ON_COMMAND(ID_EMR_ADD_OTHER_USERS_SIGNATURE, &CEmrTopicView::OnAddOtherUsersSignature)

	ON_COMMAND(ID_EMR_NEW_TOPIC, &CEmrTopicView::OnNewTopic)
	ON_COMMAND(ID_EMR_NEW_SUBTOPIC, &CEmrTopicView::OnNewSubtopic)
	ON_COMMAND(ID_EMR_IMPORT_TOPICS, &CEmrTopicView::OnImportTopics)
	ON_COMMAND(ID_EMR_IMPORT_SUBTOPICS, &CEmrTopicView::OnImportSubtopics)

	ON_COMMAND(ID_EMR_TOPIC_LIST, &CEmrTopicView::OnTopicList)	// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
	ON_COMMAND(ID_EMR_FIRST_TOPIC, &CEmrTopicView::OnFirstTopic)
	ON_COMMAND(ID_EMR_PREVIOUS_TOPIC, &CEmrTopicView::OnPreviousTopic)
	ON_COMMAND(ID_EMR_NEXT_TOPIC, &CEmrTopicView::OnNextTopic)
	ON_COMMAND(ID_EMR_LAST_TOPIC, &CEmrTopicView::OnLastTopic)

	////
	/// Messages

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_PARENTNOTIFY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()

	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

////
/// UI state helpers

bool CEmrTopicView::IsTemplate()
{
	if (m_pAttachedEMN && m_pAttachedEMN->IsTemplate()) {
		return true;
	}

	return false;
}

bool CEmrTopicView::IsLocked()
{
	if (m_pAttachedEMN && (2 == m_pAttachedEMN->GetStatus())) {
		return true;
	}

	return false;
}

bool CEmrTopicView::IsEditMode()
{
	return GetEmrFrameWnd()->IsEditMode();
}

bool CEmrTopicView::IsAllowEdit()
{
	return IsEditMode() && !IsReadOnly();
}

bool CEmrTopicView::IsWritable()
{
	if (m_pAttachedEMN && m_pAttachedEMN->IsWritable()) {
		return true;
	}

	return false;
}

bool CEmrTopicView::IsReadOnly()
{
	if (!CanWriteToEMR()) {
		return true;
	}
	return IsLocked() || !IsWritable();
}

bool CEmrTopicView::CanWriteToEMR()
{
	// (j.armen 2012-07-02 15:22) - PLID 51313 - In order for us to write to EMR, we must have the license
	if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) return false;

	if (IsTemplate()) return true;

	return !!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE);
}

bool CEmrTopicView::CanCreateEMN()
{
	if (IsTemplate()) return true;

	return CanWriteToEMR() && CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE);
}

bool CEmrTopicView::CanWriteToEMN()
{
	if (!CanWriteToEMR() || !GetAttachedEMN()) {
		return false;
	}

	if (!IsTemplate() && (2 == GetAttachedEMN()->GetStatus())) {
		return false;
	}

	if (!GetAttachedEMN()->IsWritable()) return false;

	return true;
}

bool CEmrTopicView::CanWriteToTopic()
{
	if (!GetActiveTopicWnd()) return false;

	if (!CanWriteToEMN()) return false;

	if (GetActiveTopicWnd()->IsReadOnly()) return false;

	if (IsTemplate() && -1 != GetActiveTopic()->GetSourceActionID()) {
		return false;
	}

	return true;
}

// (j.dinatale 2012-09-19 14:57) - PLID 52702 - need to be able to find the first or active topic
CEMRTopic* CEmrTopicView::GetActiveTopicOrFirst()
{
	CEMRTopic* pTopic = GetOrFindActiveTopic();
	if (!pTopic && GetAttachedEMN()) {
		pTopic = GetAttachedEMN()->GetFirstDisplayedTreeTopic();
	}
	return pTopic;
}

////
/// CEmrTopicView Command UI handlers

void CEmrTopicView::OnUpdateNewEMN(CCmdUI* pCmdUI)
{
	if (!CanCreateEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}
}

void CEmrTopicView::OnUpdate_AddItems(CCmdUI* pCmdUI)
{
	// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN, but not for templates
	if (!CanWriteToTopic()
		|| (this->IsTemplate() && (ID_EMR_ADD_SIGNATURE == pCmdUI->m_nID || ID_EMR_ADD_YOUR_SIGNATURE == pCmdUI->m_nID || ID_EMR_ADD_OTHER_USERS_SIGNATURE == pCmdUI->m_nID))) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
	return;
}

//(e.lally 2012-02-16) PLID 48065
void CEmrTopicView::OnUpdateClassicBtnAddTopics(CCmdUI* pCmdUI)
{
	OnUpdateNewTopic(pCmdUI);
}

void CEmrTopicView::OnUpdateNewTopic(CCmdUI* pCmdUI)
{
	if (!CanWriteToEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
	return;
}

void CEmrTopicView::OnUpdateNewSubtopic(CCmdUI* pCmdUI)
{
	if (!CanWriteToEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
	return;
}

void CEmrTopicView::OnUpdateImportTopics(CCmdUI* pCmdUI)
{
	if (!CanWriteToEMN() || !GetActiveTopic()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
	return;
}

void CEmrTopicView::OnUpdateImportSubtopics(CCmdUI* pCmdUI)
{
	if (!CanWriteToEMN() || !GetActiveTopic()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
	return;
}

void CEmrTopicView::OnUpdateNewTodo(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 16:02) - PLID 49831 - Keeping in line w/ previous versions, this is based on permissions
	pCmdUI->Enable(CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE));
}

void CEmrTopicView::OnUpdateNewRecording(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 16:02) - PLID 49831 - Keeping in line w/ previous versions, this is based on permissions
	pCmdUI->Enable(CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE));
}

////
/// Commands

void CEmrTopicView::OnAddItem()
{
	try {
		if (!GetActiveTopicWnd()) {
			return;
		}

		GetEmrFrameWnd()->GetEmrTreeWnd()->AddItemsToTopic();
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnAddImage()
{
	try {
		if (!GetActiveTopicWnd()) {
			return;
		}

		GetActiveTopicWnd()->AddImage();
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnAddTextMacro()
{
	try {
		if (!GetActiveTopicWnd()) {
			return;
		}

		GetActiveTopicWnd()->AddTextMacro();
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnAddSignature()
{
	try {
		if (!GetActiveTopicWnd()) {
			return;
		}

		GetActiveTopicWnd()->AddSignatureImage();
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
void CEmrTopicView::OnAddOtherUsersSignature()
{
	try {
		if (!GetActiveTopicWnd()) {
			return;
		}

		GetActiveTopicWnd()->AddSignatureImageForAnotherUser();

	} NxCatchAll(__FUNCTION__);
}


void CEmrTopicView::OnNewTopic()
{
	try {
		if (!GetEmrTreeWnd()) {
			return;
		}

		GetEmrTreeWnd()->AddNewTopic(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnNewSubtopic()
{
	try {
		if (!GetActiveTopicWnd() || !GetEmrTreeWnd()) {
			return;
		}

		GetEmrTreeWnd()->AddNewTopic(TRUE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnImportTopics()
{
	try {
		if (!GetActiveTopicWnd() || !GetEmrTreeWnd()) {
			return;
		}

		GetEmrTreeWnd()->ImportTopics(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnImportSubtopics()
{
	try {
		if (!GetActiveTopicWnd() || !GetEmrTreeWnd()) {
			return;
		}

		GetEmrTreeWnd()->ImportTopics(TRUE);
	} NxCatchAll(__FUNCTION__);
}

////
//

CEmrDocument* CEmrTopicView::GetEmrDocument()
{
	if (!this) return NULL;

	return polymorphic_downcast<CEmrDocument*>(GetDocument());
}

CEmrTopicWndPtr CEmrTopicView::EnsureTopicWnd(CEMRTopic* pTopic, bool bForceReposition)
{	
	CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

	if (!pTopicWnd) {

		EnsureTopicHeaderWnd(pTopic);

		pTopicWnd = CEmrTopicWndPtr(new CEmrTopicWnd());

		// (a.walling 2012-07-10 15:02) - PLID 49353 - Ensure we start off with the proper ReadOnly and AllowEdit state -- the topicwnd itself will take care of checking for spawned topics
		pTopicWnd->Initialize(pTopic, this, GetEmrFrameWnd()->GetEmrTreeWnd(), CRect(0, 0, 0, 0), IsTemplate(), IsReadOnly(), IsAllowEdit());

		m_topicWnds.push_back(pTopicWnd);

		RefreshTopicWindowPositions();

	} else if (bForceReposition) {
		pTopicWnd->RepositionDetailsInTopicByInfoID(-1, FALSE);
	}

	return pTopicWnd;
}

CEmrTopicHeaderWndPtr CEmrTopicView::EnsureTopicHeaderWnd(CEMRTopic* pTopic)
{
	CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();

	// (a.walling 2014-04-29 17:03) - PLID 61968 - Don't try to create one for a NULL pTopic
	if (!pTopicHeaderWnd && pTopic) {

		pTopicHeaderWnd = CEmrTopicHeaderWndPtr(new CEmrTopicHeaderWnd(pTopic, IsTemplate()));

		pTopicHeaderWnd->Initialize(this);

		m_topicHeaderWnds.push_back(pTopicHeaderWnd);

		RefreshTopicWindowPositions();
	}

	return pTopicHeaderWnd;
}

// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Call the view to show the topic
CEmrTopicWndPtr CEmrTopicView::EnsureTopic(CEMRTopic* pTopic)
{
	ASSERT(!m_pAttachedEMN || (pTopic->GetParentEMN() == m_pAttachedEMN));

	CEmrTopicWndPtr pTopicWnd = EnsureTopicWnd(pTopic);

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - do not scroll / activate here
	//if (bScrollToTopic) {
	//	ScrollToTopic(pTopic);
	//}

	return pTopic->GetTopicWnd();
}

// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - do not activate here
CEmrTopicWndPtr CEmrTopicView::ShowTopic(CEMRTopic* pTopic)
{
	CEmrTopicWndPtr pTopicWnd = EnsureTopic(pTopic);
	if (!m_pActiveTopicWnd) {
		ScrollToTopic(pTopic);
	} else {
		ShowTopicAnimated(pTopic);
	}
	return pTopicWnd;
}

CEmrTopicWndPtr CEmrTopicView::ShowTopicOnActivate(CEMRTopic* pTopic) 
{
	CEmrTopicWndPtr pTopicWnd = EnsureTopic(pTopic);
	if (!m_pActiveTopicWnd) {
		ScrollToTopic(pTopic);
	} else if (pTopic->GetTopicWnd() && m_pActiveTopicWnd != pTopic->GetTopicWnd()) {
		ShowTopicAnimated(pTopic);
	}
	return pTopicWnd;
}

void CEmrTopicView::ScrollToTopic(CEMRTopic* pTopic)
{
	CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();
	CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

	if (!pTopicHeaderWnd || !pTopicHeaderWnd->GetSafeHwnd()) {
		return;
	}

	if (!pTopicWnd || !pTopicWnd->GetSafeHwnd()) {
		return;
	}

	// (a.walling 2013-05-22 09:21) - PLID 56812 - If the preview pane is active, there is no
	// active topic view, and this would end up activating itself and causing the preview pane
	// to deactivate. We should only activate this topic view if there is another topic view
	// already active, that is not the same as this, which would mean a different EMN.
	CEmrTopicView* pActiveEmrTopicView = GetEmrFrameWnd()->GetActiveEmrTopicView();
	if (pActiveEmrTopicView && this != pActiveEmrTopicView) {
		GetEmrFrameWnd()->Activate(this);
	}

	if (!pTopicWnd->IsWindowVisible() || !pTopicHeaderWnd->IsWindowVisible()) {
		pTopicHeaderWnd->ShowWindow(SW_SHOWNA);
		pTopicWnd->ShowWindow(SW_SHOWNA);

		RefreshTopicWindowPositions(true);
	}

	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	CRect rc;
	pTopicHeaderWnd->GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.OffsetRect(ptOffset);

	ScrollToPosition(CPoint(0, rc.top));

	SetActiveTopicWnd(pTopicWnd);
}

void CEmrTopicView::EnsureTopicInView(CEMRTopic* pTopic)
{
	EnsureTopic(pTopic);

	CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();
	CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

	if (!pTopicHeaderWnd || !pTopicHeaderWnd->GetSafeHwnd() || !pTopicHeaderWnd->IsWindowVisible()) {
		return;
	}

	if (!pTopicWnd || !pTopicWnd->GetSafeHwnd()) {
		return;
	}

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - do not activate here
	if (!pTopicWnd->IsWindowVisible()) {
		pTopicWnd->ShowWindow(SW_SHOWNA);

		RefreshTopicWindowPositions(true);
	}
	
	if (!pTopicWnd->IsWindowVisible() || !pTopicHeaderWnd->IsWindowVisible()) {
		return;
	}

	SetActiveTopicWnd(pTopicWnd);

	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	CRect rc;
	pTopicHeaderWnd->GetWindowRect(&rc);

	{
		CRect rcTopic;

		pTopicWnd->GetWindowRect(&rcTopic);

		rc.bottom += rcTopic.Height();
	}

	ScreenToClient(&rc);
	rc.OffsetRect(ptOffset);

	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rcCur(CPoint(0, GetScrollPosition().y), rcClient.Size());

	long nScrollTo = rcCur.top;

	if (rc.Height() >= rcCur.Height()) {
		nScrollTo = rc.top;
	} else {
		// fits within the current view
		if (rc.top <= rcCur.top) {
			// scrolling up
			nScrollTo = rc.top;
		} else {
			// scrolling down

			// try to align the bottom
			nScrollTo = rc.top - (rcCur.Height() - rc.Height());

			if (nScrollTo <= rcCur.top) {
				// don't want to scroll up at all
				nScrollTo = rcCur.top;
			}
		}
	}

	ASSERT(nScrollTo >= 0);

	long yMax = GetScrollLimit(SB_VERT);

	if (nScrollTo > yMax) {
		nScrollTo = yMax;
	}

	if (nScrollTo != rcCur.top) {
		ScrollToPositionAnimated(CPoint(0, nScrollTo));
	}
}

void CEmrTopicView::SetActiveTopicWnd(CEmrTopicWndPtr pTopicWnd)
{
	if (pTopicWnd == m_pActiveTopicWnd) {
		// (z.manning 2012-06-29 17:40) - PLID 49547 - Even if the active topic wnd didn't change, ensure this
		// topic is selected in the tree (a non-topic row may have been selected previously).
		if (m_pActiveTopicWnd != NULL && m_pActiveTopicWnd->GetTopic() != NULL) {
			GetEmrFrameWnd()->GetEmrTreeWnd()->HighlightActiveTopic(m_pActiveTopicWnd->GetTopic());
		}
		return;
	}

	m_pActiveTopicWnd.swap(pTopicWnd);

	UpdateTitle(this);

	if (m_pActiveTopicWnd) {
		GetEmrFrameWnd()->GetEmrTreeWnd()->HighlightActiveTopic(m_pActiveTopicWnd->GetTopic());

		// (j.dinatale 2012-09-20 13:09) - PLID 52702 - need to set the active topic here too.
		GetEmrFrameWnd()->ActivateTopic(m_pActiveTopicWnd->GetTopic());
	}

	if (pTopicWnd) {
		pTopicWnd->OnActiveTopicChanged();
	}
	if (m_pActiveTopicWnd) {
		m_pActiveTopicWnd->OnActiveTopicChanged();
			
		// (a.walling 2012-06-19 08:24) - PLID 48371 - Set focus to the new active topic wnd
		if (CWnd* pFocus = GetFocus()) {
			if (this == pFocus || IsChild(pFocus)) {
				if (m_pActiveTopicWnd.get() != pFocus && !m_pActiveTopicWnd->IsChild(pFocus)) {
					m_pActiveTopicWnd->SetFocus();
				}
			}
		}
	}
}

void CEmrTopicView::ResizeTopicWnd(CEmrTopicWndPtr pTopicWnd)
{
	RefreshTopicWindowPositions();
}

// (a.walling 2012-03-30 10:48) - PLID 49329 - Call when the EMN's tree has changed (deleting a topic / subtopic)
void CEmrTopicView::HandleTreeChanged()
{
	try {
		if (!GetAttachedEMN()) {
			return;
		}

		std::set<CEMRTopic*> existingTopics;
		{
			CArray<CEMRTopic*, CEMRTopic*> topics;
			m_pAttachedEMN->GetAllTopics(topics);

			foreach (CEMRTopic* pTopic, topics) {
				existingTopics.insert(pTopic);
			}
		}

		CEmrTopicWndPtr validActiveTopicWnd;
		
		if (m_pActiveTopicWnd && existingTopics.count(m_pActiveTopicWnd->GetTopic())) {
			validActiveTopicWnd = m_pActiveTopicWnd;
		}

		
		std::vector<CEmrTopicWndPtr> expiredTopicWnds;
		std::vector<CEmrTopicHeaderWndPtr> expiredTopicHeaderWnds;

		std::vector<CEmrTopicWndPtr> validTopicWnds;
		std::vector<CEmrTopicHeaderWndPtr> validTopicHeaderWnds;


		foreach(CEmrTopicWndPtr pTopicWnd, m_topicWnds) {
			if (!pTopicWnd || !existingTopics.count(pTopicWnd->GetTopic())) {
				expiredTopicWnds.push_back(pTopicWnd);
			} else {
				validTopicWnds.push_back(pTopicWnd);
			}
		}
		
		foreach(CEmrTopicHeaderWndPtr pTopicHeaderWnd, m_topicHeaderWnds) {
			if (!pTopicHeaderWnd || !existingTopics.count(pTopicHeaderWnd->GetTopic())) {
				expiredTopicHeaderWnds.push_back(pTopicHeaderWnd);
			} else {
				validTopicHeaderWnds.push_back(pTopicHeaderWnd);
			}
		}

		foreach(CEmrTopicWndPtr pTopicWnd, expiredTopicWnds) {
			if (pTopicWnd && ::IsWindow(pTopicWnd->GetSafeHwnd())) {
				pTopicWnd->DestroyWindow();
			}
		}

		foreach(CEmrTopicHeaderWndPtr pTopicHeaderWnd, expiredTopicHeaderWnds) {
			if (pTopicHeaderWnd && ::IsWindow(pTopicHeaderWnd->GetSafeHwnd())) {
				pTopicHeaderWnd->DestroyWindow();
			}
		}
		
		m_topicWnds = validTopicWnds;
		m_topicHeaderWnds = validTopicHeaderWnds;

		// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities
		if (validActiveTopicWnd && boost::exists(m_topicWnds, validActiveTopicWnd)) {
			SetActiveTopicWnd(validActiveTopicWnd);
		} else {
			SetActiveTopicWnd(CEmrTopicWndPtr());
		}

		ForceRefreshTopicWindowPositions();

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
void CEmrTopicView::SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement)
{
	if (!pElement) return;

	switch (pElement->GetID()) {
		case ID_EMR_TOPIC_LIST:
			if (dynamic_ptr<CNxRibbonListButton> pList = pElement) {

				pList->Clear();

				CEMRTopic* pActiveTopic = GetActiveTopic();
				int nActiveTopicIndex = -1;

				CNxRibbonListButton::ItemArray items;

				CArray<CEMRTopic*, CEMRTopic*> topics;
				m_pAttachedEMN->GetAllTopics(topics);

				CString strTopicName;

				foreach (CEMRTopic* pTopic, topics) {
					// (a.walling 2012-07-09 17:22) - PLID 46646 - Use CEMRTopic::IsDisplayed
					if (pTopic->IsDisplayed()) {

						strTopicName.Empty();
						for (CEMRTopic* pParentTopic = pTopic->GetParentTopic(); pParentTopic; pParentTopic = pParentTopic->GetParentTopic()) {
							strTopicName.AppendChar('-');
						}
						strTopicName.Append(pTopic->GetDisplayName());

						if (pTopic == pActiveTopic) {
							nActiveTopicIndex = items.size();
						}

						items.push_back(CNxRibbonListButton::Item(strTopicName, (DWORD_PTR)pTopic));
					}
				}
				
				pList->AddGroup("Topics", items);

				if (nActiveTopicIndex != -1) {
					pList->SelectItem(nActiveTopicIndex);
				}
			}
			break;
	}
}

template<typename SortedT>
struct TopicNotInSetImpl
{
	TopicNotInSetImpl(const SortedT& s)
		: s_(s)
	{}

	const SortedT& s_;

	template<typename ValueT>
	bool operator()(const ValueT& val) const
	{
		return !boost::binary_search(s_, val->GetTopic());
	}
};


template<typename SetT>
TopicNotInSetImpl<SetT> TopicNotInSet(const SetT& s)
{
	return TopicNotInSetImpl<SetT>(s);
}

// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Reposition and show/hide all topic windows and headers
// Note only the topic headers are shown, the topic window is never shown, since that occurs in response to user action or
// SetActiveTopic etc
void CEmrTopicView::RefreshTopicWindowPositions(bool bForceReposition)
{
	if (m_bRefreshingTopicWindowPositions) {
		return;
	}

	if (!m_pAttachedEMN) {
		return;
	}

	CGuardLock guardRefesh(m_bRefreshingTopicWindowPositions);

	if (m_bForceRefreshNextRefreshTopicWindowPositions) {
		m_bForceRefreshNextRefreshTopicWindowPositions = false;
		bForceReposition = true;
	}

	CArray<CEMRTopic*, CEMRTopic*> topics;
	m_pAttachedEMN->GetAllTopics(topics);

	// (a.walling 2011-10-28 13:24) - PLID 46176 - Topic window headers

	// (a.walling 2012-04-24 10:22) - PLID 49880 - Split into visible and hidden topics
	std::vector<CEMRTopic*> visibleTopics;
	std::vector<CEMRTopic*> hiddenTopics;
	visibleTopics.reserve(topics.GetSize());
	hiddenTopics.reserve(topics.GetSize());

	{
		foreach(CEMRTopic* pTopic, topics)
		{
			// (a.walling 2012-07-09 17:22) - PLID 46646 - Use CEMRTopic::IsDisplayed
			if (pTopic->IsDisplayed()) {
				visibleTopics.push_back(pTopic);

				// (a.walling 2012-04-24 10:22) - PLID 49880 - Go ahead and ensure the topic header window if visible
				EnsureTopicHeaderWnd(pTopic);
			} else {
				hiddenTopics.push_back(pTopic);
			}
		}
	}

	// (a.walling 2012-04-25 12:14) - PLID 46176 - Destroy any windows that might no longer exist in the topics
	{
		using namespace boost;
		std::vector<CEMRTopic*> validTopics;
		validTopics.reserve(visibleTopics.size() + hiddenTopics.size());

		insert(validTopics, validTopics.end(), visibleTopics);
		insert(validTopics, validTopics.end(), hiddenTopics);
		sort(validTopics);

		{
			std::vector<CEmrTopicWndPtr> invalidTopicWnds;
			std::vector<CEmrTopicHeaderWndPtr> invalidTopicHeaderWnds;

			foreach (CEmrTopicWndPtr& topicWnd, m_topicWnds) {
				if (!binary_search(validTopics, topicWnd->GetTopic())) {
					invalidTopicWnds.push_back(topicWnd);
				}
			}
			foreach (CEmrTopicHeaderWndPtr& topicHeaderWnd, m_topicHeaderWnds) {
				if (!binary_search(validTopics, topicHeaderWnd->GetTopic())) {
					invalidTopicHeaderWnds.push_back(topicHeaderWnd);
				}
			}

			foreach (CEmrTopicWndPtr& topicWnd, invalidTopicWnds) {
				if (topicWnd->GetSafeHwnd()) {
					topicWnd->DestroyWindow();
				}
			}
			foreach (CEmrTopicHeaderWndPtr& topicHeaderWnd, invalidTopicHeaderWnds) {
				if (topicHeaderWnd->GetSafeHwnd()) {
					topicHeaderWnd->DestroyWindow();
				}
			}
		}

		remove_erase_if(m_topicWnds, TopicNotInSet(validTopics));
		remove_erase_if(m_topicHeaderWnds, TopicNotInSet(validTopics));
	}

	// (a.walling 2012-04-24 10:22) - PLID 49880 - Hide what needs to be hidden
	// Note that DeferWindowPos can only move OR show/hide windows. If you use SWP_SHOWWINDOW / SWP_HIDEWINDOW at all, all your movement is lost, so we break it up into parts
	{
		HDWP dwp = BeginDeferWindowPos(hiddenTopics.size());

		foreach(CEMRTopic* pTopic, hiddenTopics)
		{
			CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();
			CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();

			if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd() && pTopicHeaderWnd->IsWindowVisible()) {
				dwp = DeferWindowPos(dwp, pTopicHeaderWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));
			}

			if (pTopicWnd && pTopicWnd->GetSafeHwnd() && pTopicWnd->IsWindowVisible()) {
				dwp = DeferWindowPos(dwp, pTopicWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));
			}
		}

		BOOL dwpResult = EndDeferWindowPos(dwp);
		_ASSERTE(dwpResult);
	}

	// Forece reposition for visible topics if necessary
	if (bForceReposition) {
		foreach(CEMRTopic* pTopic, visibleTopics)
		{
			CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

			if (pTopicWnd) {
				pTopicWnd->RepositionDetailsInTopicByInfoID(-1, FALSE);
			}
		}
	}
	
	// (a.walling 2012-04-24 10:22) - PLID 49880 - Calculate bounds of visible topics
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		CSize bounds(rcClient.Width(), 0);

		foreach(CEMRTopic* pTopic, visibleTopics)
		{
			CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();
			CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

			// (a.walling 2011-10-28 13:24) - PLID 46176 - Topic window headers
			CRect rcTopicHeader;

			if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd()) {
				pTopicHeaderWnd->RefreshLabels();
				pTopicHeaderWnd->GetWindowRect(&rcTopicHeader);
				bounds.cy += rcTopicHeader.Height();
			}

			if (pTopicWnd && pTopicWnd->GetSafeHwnd() && pTopicWnd->IsWindowVisible()) {
				const CSize& szTopicContentSize = pTopicWnd->GetDisplayContentSize();
				bounds.cy += szTopicContentSize.cy;
				
				if (bounds.cx < szTopicContentSize.cx) {
					bounds.cx = szTopicContentSize.cx;
				}
			}
		}
		
		CRect rcParent;
		GetParent()->GetWindowRect(&rcParent);
		CSize szPage = rcParent.Size();
		szPage.cx = MulDiv(szPage.cx, 4, 5);
		szPage.cy = MulDiv(szPage.cy, 4, 5);
	
		if (m_totalDev != bounds || m_pageDev != szPage) {
			SetScrollSizes(MM_TEXT, bounds, szPage);
		}
	}


	// (a.walling 2012-04-24 10:22) - PLID 49880 - Layout topics in order in the view
	{				
		CString strHeaderText;

		CRect rcClient;
		GetClientRect(&rcClient);

		CPoint sposClient(0, 0);
		CPoint sposScreen = sposClient;
		MapWindowPoints(NULL, &sposScreen, 1);
		
		CPoint sposOffset = sposScreen - sposClient;

		CPoint posScroll(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));

		CPoint pos(0, 0);

		HDWP dwp = BeginDeferWindowPos(visibleTopics.size() * 2);

		foreach(CEMRTopic* pTopic, visibleTopics)
		{
			{
				CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();

				// (a.walling 2012-04-24 10:22) - PLID 49880 - Need to use MapWindowPoints to get the proper relative rect, and then offset by the scroll pos
				CRect rcTopicHeaderCurrent;
				pTopicHeaderWnd->GetClientRect(&rcTopicHeaderCurrent);
				pTopicHeaderWnd->MapWindowPoints(this, &rcTopicHeaderCurrent);

				CSize szTopicHeaderContentSize = pTopicHeaderWnd->GetContentSize();
				szTopicHeaderContentSize.cx = rcClient.Width();

				CRect rcTopicHeader(pos, szTopicHeaderContentSize);
				rcTopicHeader.OffsetRect(posScroll);

				// (a.walling 2011-10-28 13:24) - PLID 46176 - Topic window headers
				if (rcTopicHeader != rcTopicHeaderCurrent) {
					CPoint windowOffset = rcTopicHeader.TopLeft() - rcTopicHeaderCurrent.TopLeft();
					dwp = DeferWindowPos(dwp, pTopicHeaderWnd->GetSafeHwnd(), NULL, rcTopicHeader.left, rcTopicHeader.top, rcTopicHeader.Width(), rcTopicHeader.Height(), (SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));
				}

				pos.y += rcTopicHeader.Height();
			}

			{
				CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

				if (pTopicWnd && pTopicWnd->GetSafeHwnd() && pTopicWnd->IsWindowVisible()) {

					CRect rcTopicCurrent;
					pTopicWnd->GetClientRect(&rcTopicCurrent);
					pTopicWnd->MapWindowPoints(this, &rcTopicCurrent);

					CSize szTopicContentArea = pTopicWnd->GetDisplayContentSize();

					if (szTopicContentArea.cx < rcClient.Width()) {
						szTopicContentArea.cx = rcClient.Width();
					}
					
					CRect rcTopic(pos, szTopicContentArea);
					rcTopic.OffsetRect(posScroll);
					
					if (rcTopic != rcTopicCurrent) {
						CPoint windowOffset = rcTopic.TopLeft() - rcTopicCurrent.TopLeft();
						dwp = DeferWindowPos(dwp, pTopicWnd->GetSafeHwnd(), NULL, rcTopic.left, rcTopic.top, rcTopic.Width(), rcTopic.Height(), (SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));
					}

					pos.y += rcTopic.Height();
				}
			}
		}

		EndDeferWindowPos(dwp);
	}

	// (a.walling 2012-04-24 10:22) - PLID 49880 - Show what needs to be shown
	{
		HDWP dwp = BeginDeferWindowPos(visibleTopics.size());

		foreach(CEMRTopic* pTopic, visibleTopics)
		{
			CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();
			CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();


			if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd() && !pTopicHeaderWnd->IsWindowVisible()) {
				dwp = DeferWindowPos(dwp, pTopicHeaderWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));
			}

			// (a.walling 2012-04-24 10:22) - PLID 49880 - We are not in charge of showing the topic here, really, since it might be collapsed
			//if (pTopicWnd && pTopicWnd->GetSafeHwnd() && !pTopicWnd->IsWindowVisible()) {
			//	dwp = DeferWindowPos(dwp, pTopicWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));
			//}
		}

		BOOL dwpResult = EndDeferWindowPos(dwp);
		_ASSERTE(dwpResult);
	}
}

void CEmrTopicView::OnTopicHeaderClicked(CEMRTopic* pTopic)
{	
	try {
		CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();

		if (!pTopicWnd || !pTopicWnd->GetSafeHwnd() || !pTopicWnd->IsWindowVisible()) {
			ShowTopicAnimated(pTopic, true);
		} else {
			ShowTopicAnimated(pTopic, false);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::SetAllowEdit(BOOL bAllowEdit)
{
	foreach (CEmrTopicWndPtr pTopicWnd, m_topicWnds) {
		// (a.wetta 2006-11-16 10:48) - PLID 19474 - Don't allow edit if the topic is a spawned topic on a template
		if(IsTemplate() && (pTopicWnd->GetTopic()->GetSourceActionID() != -1)) {
			pTopicWnd->SetAllowEdit(FALSE);
		} else {
			pTopicWnd->SetAllowEdit(bAllowEdit);			
		}
	}
}

void CEmrTopicView::OnDestroy()
{
	try {
		CScrollView::OnDestroy();

		foreach (CEmrTopicWndPtr pTopicWnd, m_topicWnds) {
			if (::IsWindow(pTopicWnd->GetSafeHwnd())) {
				::DestroyWindow(pTopicWnd->GetSafeHwnd());
			}
		}

		if (m_pActiveTopicWnd && m_pActiveTopicWnd->GetSafeHwnd()) {
			::DestroyWindow(m_pActiveTopicWnd->GetSafeHwnd());
		}
		
		foreach (CEmrTopicHeaderWndPtr pTopicHeaderWnd, m_topicHeaderWnds) {
			if (::IsWindow(pTopicHeaderWnd->GetSafeHwnd())) {
				::DestroyWindow(pTopicHeaderWnd->GetSafeHwnd());
			}
		}

		m_topicWnds.clear();
		m_pActiveTopicWnd.reset();
		m_topicHeaderWnds.clear();
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnParentNotify(UINT message, LPARAM lParam)
{
	try {
		switch (message) {
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_XBUTTONDOWN:
				{			
					CEmrTopicWndPtr pTopicWnd = GetTopicWndFromPoint(GetMessagePos());

					if (pTopicWnd) {
						SetActiveTopicWnd(pTopicWnd);
					}
				}
				break;
		}

		CScrollView::OnParentNotify(message, lParam);
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnPaint()
{
	//this is done to avoid CView::OnPaint
	Default();
}

void CEmrTopicView::OnDraw(CDC* pDC)
{
	// (a.walling 2014-05-06 16:39) - PLID 62046 - VS2013 - MFC bug causing taskbar previews to display a progress spinner or not work properly.
	TRACE0(__FUNCTION__" - not supported.");
	ASSERT(FALSE);
};

BOOL CEmrTopicView::OnEraseBkgnd(CDC* pDC) 
{
	try {
		CRect rc;
		GetClientRect(&rc);

		pDC->FillSolidRect(rc, EmrColors::Topic::Background(IsTemplate()));
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEmrTopicView::OnSize(UINT nType, int cx, int cy) 
{
	try {
		// (a.walling 2012-03-06 16:01) - PLID 46175 - Ensure the window is entirely invalidated when the scroll bar visible state changes
		BOOL bVertBarPre, bVertBarPost;
		BOOL bHorzBarPre, bHorzBarPost;
		CheckScrollBars(bHorzBarPre, bVertBarPre);

		CScrollView::OnSize(nType, cx, cy);

		RefreshTopicWindowPositions();

		CheckScrollBars(bHorzBarPost, bVertBarPost);

		int scrollBarStatePre = (bVertBarPre ? 2 : 0) | (bHorzBarPre ? 1 : 0);
		int scrollBarStatePost = (bVertBarPost ? 2 : 0) | (bHorzBarPost ? 1 : 0);

		if (scrollBarStatePre != scrollBarStatePost) {
			RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
		}
	} NxCatchAll(__FUNCTION__);
}

int CEmrTopicView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		if (CScrollView::OnCreate(lpCreateStruct) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(0,0));
	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CEmrTopicView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

void CEmrTopicView::OnClose()
{
	CScrollView::OnClose();
}

BOOL CEmrTopicView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	try {
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;

		if (LOBYTE(nScrollCode) == SB_THUMBTRACK)
		{
			GetScrollInfo(SB_HORZ, &info);
			nPos = info.nTrackPos;
		}

		if (HIBYTE(nScrollCode) == SB_THUMBTRACK)
		{
			GetScrollInfo(SB_VERT, &info);
			nPos = info.nTrackPos;
		}
	} NxCatchAll(__FUNCTION__);

	BOOL ret = CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);

	return ret;
}

void CEmrTopicView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {
		CPoint ptScreen = point;
		ClientToScreen(&ptScreen);
		CEmrTopicWndPtr pTopicWnd = GetTopicWndFromPoint(ptScreen);

		if (pTopicWnd) {
			SetActiveTopicWnd(pTopicWnd);
		}

		// (a.walling 2011-03-19 12:23) - PLID 42694 - Set the focus so we get mousewheel messages
		SetFocus();

		// (a.walling 2011-03-19 12:23) - PLID 42694 - Start the anchor scrolling, drag only
		if (StartAnchorScroll(nFlags, point, WM_LBUTTONUP, TRUE)) {
			return;
		}
	} NxCatchAll(__FUNCTION__);

	CScrollView::OnLButtonDown(nFlags, point);
}

void CEmrTopicView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	try {
		CPoint ptScreen = point;
		ClientToScreen(&ptScreen);
		CEmrTopicWndPtr pTopicWnd = GetTopicWndFromPoint(ptScreen);

		if (pTopicWnd) {
			SetActiveTopicWnd(pTopicWnd);
		}

		// (a.walling 2011-03-19 12:23) - PLID 42694 - Set the focus so we get mousewheel messages
		SetFocus();

		// (a.walling 2011-03-19 12:23) - PLID 42694 - Start the anchor scrolling, drag only or anchored
		if (StartAnchorScroll(nFlags, point, WM_MBUTTONUP, FALSE)) {
			return;
		}
	} NxCatchAll(__FUNCTION__);

	//CScrollView::OnMButtonDown(nFlags, point);
}

void CEmrTopicView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	try {
		CPoint ptScreen = point;
		ClientToScreen(&ptScreen);
		CEmrTopicWndPtr pTopicWnd = GetTopicWndFromPoint(ptScreen);

		if (pTopicWnd) {
			SetActiveTopicWnd(pTopicWnd);
		}

		// (a.walling 2011-03-19 12:23) - PLID 42694 - Set the focus so we get mousewheel messages
		SetFocus();

	} NxCatchAll(__FUNCTION__);

	CScrollView::OnRButtonDown(nFlags, point);
}

// (a.walling 2011-12-18 14:15) - PLID 49880 - Smooth topic scrolling / toggling
void CEmrTopicView::ScrollToPositionAnimated(CPoint pt)
{
	double animationDuration = GetAnimationDuration();

	if (0 == animationDuration) {
		// (a.walling 2013-03-07 08:33) - PLID 55494 - Disable topic animation over RDP - this was still delaying afterwards and scrolling twice
		ScrollToPosition(pt);
		return;
	}

	CPoint ptCur(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	static const DWORD dwTargetFPS = 60;
	DWORD dwStepCount = static_cast<DWORD>(((animationDuration * dwTargetFPS) / 1000) + 0.5);

	double curX = ptCur.x;
	double curY = ptCur.y;

	double totalX = pt.x - ptCur.x;
	double totalY = pt.y - ptCur.y;

	double stepX = totalX / dwStepCount;
	double stepY = totalY / dwStepCount;

	DWORD dwSleep = static_cast<DWORD>((animationDuration / dwStepCount));

	if (dwStepCount) {
		--dwStepCount;
	}

	for (DWORD dw = 0; dw < dwStepCount; ++dw) {
		CPoint ptNew(static_cast<long>(curX), static_cast<long>(curY));

		ScrollToPosition(ptNew);
		UpdateWindow();

		::Sleep(dwSleep);

		curX += stepX;
		curY += stepY;
	}

	::Sleep(dwSleep);

	ScrollToPosition(pt);
}

// (a.walling 2011-12-18 14:15) - PLID 49880 - Smooth topic scrolling / toggling
void CEmrTopicView::ShowTopicAnimated(CEMRTopic* pTopic, bool bShow)
{		
	// (a.walling 2013-03-07 08:33) - PLID 55494 - Disable topic animation over RDP
	double animationDuration = GetAnimationDuration();

	CEmrTopicWndPtr pTopicWnd = EnsureTopicWnd(pTopic, true);
	RefreshTopicWindowPositions(true);

	if (pTopicWnd->IsWindowVisible()) {
		if (bShow) {
			animationDuration = 0;
		}
	} else {
		if (!bShow) {
			animationDuration = 0;
		}
	}

	if (0 == animationDuration) {
		pTopicWnd->ShowWindow(bShow ? SW_SHOWNA : SW_HIDE);
	} else {
		CGuardLock guardRefesh(m_bRefreshingTopicWindowPositions);

		CRect rcTarget;
		pTopicWnd->GetWindowRect(&rcTarget);
		ScreenToClient(&rcTarget);

		static const DWORD dwTargetFPS = 60;
		DWORD dwStepCount = static_cast<DWORD>(((animationDuration * dwTargetFPS) / 1000) + 0.5);

		double stepY = rcTarget.Height() / dwStepCount;
		double curHeight;
		if (!bShow) {
			curHeight = rcTarget.Height();
			stepY = -stepY;
		} else {
			curHeight = 0;
		}

		DWORD dwSleep = static_cast<DWORD>((animationDuration / dwStepCount));

		if (dwStepCount) {
			--dwStepCount;
		}

		CRect lastRect = rcTarget;

		if (bShow) {
			lastRect.bottom = lastRect.top;
			pTopicWnd->MoveWindow(lastRect, FALSE);
			pTopicWnd->ShowWindow(SW_SHOWNA);
			pTopicWnd->BringWindowToTop();
		}

		for (DWORD dw = 0; dw < dwStepCount; ++dw) {
			
			curHeight += stepY;

			CRect newRect = lastRect;
			newRect.bottom = static_cast<long>(newRect.top + curHeight);

			pTopicWnd->MoveWindow(newRect);
			pTopicWnd->UpdateWindow();
			UpdateWindow();

			::Sleep(dwSleep);
		}

		::Sleep(dwSleep);

		if (!bShow) {
			pTopicWnd->ShowWindow(SW_HIDE);
		}

		pTopicWnd->MoveWindow(rcTarget);
	}

	RefreshTopicWindowPositions();

	if (bShow) {
		EnsureTopicInView(pTopicWnd->GetTopic());
	}
}


// (a.walling 2011-03-19 12:23) - PLID 42694 - Starts the anchor scrolling, either drag only or drag and anchor
// and continue until the appropriate button up event comes to the PreTranslateMessage of the scroll window
bool CEmrTopicView::StartAnchorScroll(UINT nFlags, const CPoint& point, UINT nMessageButtonUp, BOOL bDragOnly)
{
	// ignore if control is set, I think
	if (nFlags & MK_CONTROL) {
		return false;
	}

	if (m_pScrollAnchor) {
		BOOL bWasValid = m_pScrollAnchor->IsValid();
		m_pScrollAnchor.reset();

		if (bWasValid) { // if not valid, it was already destroyed, so continue			
			return false;
		}
	}

	BOOL bHorzBar, bVertBar;
	CheckScrollBars(bHorzBar, bVertBar);

	// if there's no scrollbars, we don't do anything!
	if (!bVertBar && !bHorzBar)
	{
		return false;
	}

	// create a local scoped pointer to a new CNxScrollAnchor
	scoped_ptr<CNxScrollAnchor> pNewScrollAnchor(new CNxScrollAnchor);

	if (!pNewScrollAnchor->CreateAnchor(this, point, bHorzBar, bVertBar, nMessageButtonUp, bDragOnly)) {
		// we failed; will be deleted when the scoped_ptr goes out of scope.
		return false;
	}

	// we succeeded, so swap with the member so it stays alive
	m_pScrollAnchor.swap(pNewScrollAnchor);

	return true;
}

CEmrTopicWndPtr CEmrTopicView::FindActiveTopicWnd()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nMaxDisplayed = 0;
	CEmrTopicWndPtr pMaxDisplayed;

	foreach (CEmrTopicWndPtr pTopicWnd, m_topicWnds) {
		if (pTopicWnd && pTopicWnd->GetSafeHwnd() && pTopicWnd->IsWindowVisible()) {
			
			CRect rcTopicArea;

			{
				CRect rcTopicWnd;
				pTopicWnd->GetWindowRect(&rcTopicWnd);

				CRect rcTopicHeader;
				CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopicWnd->GetTopic()->GetTopicHeaderWnd();
				if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd() && pTopicHeaderWnd->IsWindowVisible()) {
					pTopicHeaderWnd->GetWindowRect(&rcTopicHeader);
					rcTopicArea.UnionRect(&rcTopicWnd, &rcTopicHeader);
				} else {
					rcTopicArea = rcTopicWnd;
				}
			}

			ScreenToClient(&rcTopicArea);

			CRect rcIntersect;
			if (rcIntersect.IntersectRect(&rcClient, &rcTopicArea)) {
				if (rcIntersect.Height() > nMaxDisplayed) {
					nMaxDisplayed = rcIntersect.Height();
					pMaxDisplayed = pTopicWnd;
				}
			}
		}
	}

	return pMaxDisplayed;
}

CEmrTopicWndPtr CEmrTopicView::GetTopicWndFromPoint(CPoint pt)
{
	CRect rcScreen;
	GetClientRect(&rcScreen);
	MapWindowPoints(NULL, &rcScreen);

	if (!rcScreen.PtInRect(pt)) {
		return CEmrTopicWndPtr();
	}

	std::pair<CEmrTopicWndPtr, long> lastTopic(CEmrTopicWndPtr(), 0);

	foreach (CEmrTopicWndPtr pTopicWnd, m_topicWnds) {
		if (pTopicWnd && pTopicWnd->GetSafeHwnd() && pTopicWnd->IsWindowVisible()) {
			CRect rc;
			pTopicWnd->GetClientRect(&rc);
			pTopicWnd->MapWindowPoints(NULL, &rc);

			if (pt.y >= rc.top && pt.y < rc.bottom) {
				return pTopicWnd;
			}

			if (pTopicWnd->GetTopic() && rc.bottom > lastTopic.second) {
				lastTopic.second = rc.bottom;
				lastTopic.first = pTopicWnd;
			}
		}

		if (pTopicWnd && pTopicWnd->GetTopic()) {
			CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopicWnd->GetTopic()->GetTopicHeaderWnd();

			if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd() && pTopicHeaderWnd->IsWindowVisible()) {
				CRect rc;
				pTopicHeaderWnd->GetClientRect(&rc);
				pTopicHeaderWnd->MapWindowPoints(NULL, &rc);

				if (pt.y >= rc.top && pt.y < rc.bottom) {
					return pTopicWnd;
				}

				if (rc.bottom > lastTopic.second) {
					lastTopic.second = rc.bottom;
					lastTopic.first = CEmrTopicWndPtr();
				}
			}
		}
	}

	if (lastTopic.first && lastTopic.second < pt.y) {
		return lastTopic.first;
	}

	// otherwise, just don't mess with it

	return CEmrTopicWndPtr();

	//if (m_pAttachedEMN) {
	//	CArray<CEMRTopic*, CEMRTopic*> topics;
	//	m_pAttachedEMN->GetAllTopics(topics);

	//	reverse_foreach (CEMRTopic* pTopic, topics) {
	//		CEmrTopicHeaderWndPtr pTopicHeaderWnd = pTopic->GetTopicHeaderWnd();

	//		if (pTopicHeaderWnd && pTopicHeaderWnd->GetSafeHwnd() && pTopicHeaderWnd->IsWindowVisible()) {
	//			CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();
	//			if (pTopicWnd && pTopicWnd->IsWindowVisible()) {
	//				return pTopicWnd;
	//			} else {
	//				break;
	//			}
	//		}
	//	}
	//}
}

void CEmrTopicView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	try {
		CEmrTopicWndPtr pTopicWnd = GetTopicWndFromPoint(point);

		if (!pTopicWnd || pTopicWnd->IsReadOnly()) {
			CScrollView::OnContextMenu(pWnd, point);
			return;
		}

		SetActiveTopicWnd(pTopicWnd);

		AfxGetPracticeApp()->ShowPopupMenu(IDR_POPUP_EMR_TOPIC, point.x, point.y, this);
	} NxCatchAll(__FUNCTION__);
}

BOOL CEmrTopicView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		// (a.walling 2011-03-19 12:23) - PLID 42694 - If scroll anchor is running, ignore since it sets the cursor itself
		if (m_pScrollAnchor && m_pScrollAnchor->IsValid()) {
			return FALSE;
		}

		if (HTCLIENT == nHitTest) {
			if (pWnd == this || pWnd->GetParent() == this) {
				BOOL bHorzBar, bVertBar;
				CheckScrollBars(bHorzBar, bVertBar);

				if (bVertBar || bHorzBar) {
					// (a.walling 2012-03-09 11:22) - PLID 48765 - If we can scroll, use the hand cursor

					static HICON hCursorHand = AfxGetApp()->LoadCursor(IDC_HAND_CUR);
					SetCursor(hCursorHand);

					return TRUE;
				}
			}
		}
	} NxCatchAllIgnore();

	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2012-07-03 10:56) - PLID 51284 - Ensure the appropriate EMN is activated and highlighted in the tree when this view becomes active
void CEmrTopicView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	try {
		if (bActivate && this == pActivateView) {
			if (CEMN* pEMN = GetAttachedEMN()) {
				if (m_pActiveTopicWnd && m_pActiveTopicWnd->GetTopic() && pEMN->GetTopicByPointer((long)m_pActiveTopicWnd->GetTopic())) {
					GetEmrFrameWnd()->ActivateTopic(m_pActiveTopicWnd->GetTopic());
				} else {
					// (j.dinatale 2012-09-19 11:53) - PLID 52702 - need to activate the first topic, if we dont have a topic, just activate the EMN
					CEMRTopic *pTopic = GetActiveTopicOrFirst();
					if(pTopic){
						GetEmrFrameWnd()->ActivateTopic(pTopic);
					}else{
						GetEmrFrameWnd()->ActivateEMN(pEMN, NULL);
					}
				}
			}
			UpdateTitle(this);
		}

		// (a.walling 2012-05-16 14:56) - PLID 50431 - Save the focus hierarchy when deactivating
		if (SaveFocusHierarchy())
			return;     // don't call base class when focus is already set

		// (a.walling 2012-05-16 14:50) - PLID 49881 - Skip CScrollView's implementation of OnActivateView which does not handle being a control container well
		CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-16 14:56) - PLID 50431 - Save the focus hierarchy when deactivating
void CEmrTopicView::OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd)
{
	try {
		if (nState == WA_INACTIVE)
			SaveFocusHierarchy();     // save focus when frame loses activation
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
void CEmrTopicView::OnUpdateTopicList(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 09:00) - PLID 52629 - Handle situations with no attached emn
	if (!GetAttachedEMN()) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

// (a.walling 2012-07-09 17:22) - PLID 46646 - Updated topic navigation to handle displayed topics appropriately and navigate among EMNs

void CEmrTopicView::OnUpdateFirstTopic(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 09:00) - PLID 52629 - Handle situations with no attached emn
	if (!GetAttachedEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	CEMRTopic* pTarget = NULL;
	CEMN* pTargetEMN = NULL;

	pTarget = GetAttachedEMN()->GetFirstDisplayedTreeTopic();
	if (pTarget && pTarget == GetActiveTopic()) {
		pTarget = NULL;
	}

	if (!pTarget) {
		pTargetEMN = GetAttachedEMN()->GetPrevEMN();
	}

	if (pTarget || pTargetEMN) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CEmrTopicView::OnUpdatePreviousTopic(CCmdUI* pCmdUI)
{	
	// (a.walling 2013-05-02 09:00) - PLID 52629 - Handle situations with no attached emn
	if (!GetAttachedEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	CEMRTopic* pTarget = NULL;
	CEMN* pTargetEMN = NULL;

	if (CEMRTopic* pTopic = GetActiveTopic()) {
		pTarget = pTopic->GetPrevDisplayedTreeTopic();
	}
	if (!pTarget) {
		pTargetEMN = GetAttachedEMN()->GetPrevEMN();
	}

	if (pTarget || pTargetEMN) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CEmrTopicView::OnUpdateNextTopic(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 09:00) - PLID 52629 - Handle situations with no attached emn
	if (!GetAttachedEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	CEMRTopic* pTarget = NULL;
	CEMN* pTargetEMN = NULL;

	if (CEMRTopic* pTopic = GetActiveTopic()) {
		pTarget = pTopic->GetNextDisplayedTreeTopic();
	} else {
		pTarget = GetAttachedEMN()->GetFirstDisplayedTreeTopic();
	}

	if (!pTarget) {
		pTargetEMN = GetAttachedEMN()->GetNextEMN();
	}

	if (pTarget || pTargetEMN) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CEmrTopicView::OnUpdateLastTopic(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 09:00) - PLID 52629 - Handle situations with no attached emn
	if (!GetAttachedEMN()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	CEMRTopic* pTarget = NULL;
	CEMN* pTargetEMN = NULL;

	pTarget = GetAttachedEMN()->GetLastDisplayedTreeTopic();
	if (pTarget && pTarget == GetActiveTopic()) {
		pTarget = NULL;
	}

	if (!pTarget) {
		pTargetEMN = GetAttachedEMN()->GetNextEMN();
	}

	if (pTarget || pTargetEMN) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
void CEmrTopicView::OnTopicList()
{
	try {
		int index = CMFCRibbonGallery::GetLastSelectedItem(ID_EMR_TOPIC_LIST);

		dynamic_ptr<CNxRibbonListButton> pList = GetEmrFrameWnd()->GetStatusBar().FindChildElement(ID_EMR_TOPIC_LIST);

		if (!pList) {
			ASSERT(FALSE);
			return;
		}

		const CNxRibbonListButton::ItemArray& topicEntries = pList->GetItems();

		if (index < 0 || index >= (int)topicEntries.size()) {
			ASSERT(FALSE);
			return;
		}

		CEMRTopic* pTopic = reinterpret_cast<CEMRTopic*>(topicEntries[index].second);

		if (!m_pAttachedEMN->GetTopicByPointer((long)pTopic)) {
			ASSERT(FALSE);
			return;
		}

		ShowTopicAnimated(pTopic);

		// (j.dinatale 2012-09-20 11:40) - PLID 52702 - need to activate the newly selected topic
		GetEmrFrameWnd()->ActivateTopic(pTopic);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-07-09 17:22) - PLID 46646 - Updated topic navigation to handle displayed topics appropriately and navigate among EMNs

// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
void CEmrTopicView::OnFirstTopic()
{	
	try {
		CEMRTopic* pTarget = NULL;
		CEMN* pTargetEMN = NULL;

		// (j.dinatale 2012-09-20 11:33) - PLID 52702 - handle moving to the previous topic via the first topic button
		if(GetEmrFrameWnd()->GetEMRSavePref() == 2){
			CEMN * pActiveEMN = GetAttachedEMN();

			if(pActiveEMN && !pActiveEMN->IsTemplate() && !pActiveEMN->IsLockedAndSaved() && pActiveEMN->IsUnsaved()){
				GetEmrFrameWnd()->GetEmrTreeWnd()->SaveEMR(esotEMN, (long)pActiveEMN, FALSE);
			}
		}

		// navigate to first topic of this EMN
		if (CEMRTopic* pActiveTopic = GetActiveTopic()) {
			pTarget = GetAttachedEMN()->GetFirstDisplayedTreeTopic();
			if (pTarget && pTarget != pActiveTopic) {
				ShowTopicAnimated(pTarget, true);
				GetEmrFrameWnd()->ActivateTopic(pTarget);
				return;
			}
		}

		// otherwise, navigate to the beginning of previous EMN
		// i tried this where it navigated to the last topic, but really it didnt make much sense
		// that can occur when using the next/prev topic at least
		if (pTargetEMN = GetAttachedEMN()->GetPrevEMN()) {
			pTarget = pTargetEMN->GetFirstDisplayedTreeTopic();

			if (pTarget) {
				GetEmrFrameWnd()->ActivateTopic(pTarget);
			} else {
				GetEmrFrameWnd()->EnsureEmrTopicView(pTargetEMN);
				GetEmrFrameWnd()->ActivateEMN(pTargetEMN, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
void CEmrTopicView::OnPreviousTopic()
{
	try {
		CEMRTopic* pTarget = NULL;
		CEMN* pTargetEMN = NULL;

		// (j.dinatale 2012-09-18 15:11) - PLID 52702 - handle moving to the previous topic via the next button
		if(GetEmrFrameWnd()->GetEMRSavePref() == 2){
			CEMN * pActiveEMN = GetAttachedEMN();

			if(pActiveEMN && !pActiveEMN->IsTemplate() && !pActiveEMN->IsLockedAndSaved() && pActiveEMN->IsUnsaved()){
				GetEmrFrameWnd()->GetEmrTreeWnd()->SaveEMR(esotEMN, (long)pActiveEMN, FALSE);
			}
		}
			
		if (CEMRTopic* pActiveTopic = GetActiveTopic()) {
			// navigate to previous topic
			pTarget = pActiveTopic->GetPrevDisplayedTreeTopic();
			if (pTarget) {
				ShowTopicAnimated(pTarget, true);
				GetEmrFrameWnd()->ActivateTopic(pTarget);
				return;
			}
		}

		// otherwise, navigate to the end of previous EMN
		if (pTargetEMN = GetAttachedEMN()->GetPrevEMN()) {
			pTarget = pTargetEMN->GetLastDisplayedTreeTopic();

			if (pTarget) {
				GetEmrFrameWnd()->ActivateTopic(pTarget);
			} else {
				GetEmrFrameWnd()->EnsureEmrTopicView(pTargetEMN);
				GetEmrFrameWnd()->ActivateEMN(pTargetEMN, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
void CEmrTopicView::OnNextTopic()
{
	try {
		CEMRTopic* pTarget = NULL;
		CEMN* pTargetEMN = NULL;

		// (j.dinatale 2012-09-18 15:11) - PLID 52702 - handle moving to the previous topic via the next button
		if(GetEmrFrameWnd()->GetEMRSavePref() == 2){
			CEMN * pActiveEMN = GetAttachedEMN();

			if(pActiveEMN && !pActiveEMN->IsTemplate() && !pActiveEMN->IsLockedAndSaved() && pActiveEMN->IsUnsaved()){
				GetEmrFrameWnd()->GetEmrTreeWnd()->SaveEMR(esotEMN, (long)pActiveEMN, FALSE);
			}
		}
		
		// navigate to next topic
		if (CEMRTopic* pActiveTopic = GetActiveTopic()) {
			pTarget = pActiveTopic->GetNextDisplayedTreeTopic();
		} else {
			pTarget = GetAttachedEMN()->GetFirstDisplayedTreeTopic();
		}

		if (pTarget) {
			ShowTopicAnimated(pTarget, true);
			GetEmrFrameWnd()->ActivateTopic(pTarget);
			return;
		}

		// otherwise, navigate to the end of previous EMN
		if (pTargetEMN = GetAttachedEMN()->GetNextEMN()) {
			pTarget = pTargetEMN->GetFirstDisplayedTreeTopic();

			if (pTarget) {
				GetEmrFrameWnd()->ActivateTopic(pTarget);
			} else {
				GetEmrFrameWnd()->EnsureEmrTopicView(pTargetEMN);
				GetEmrFrameWnd()->ActivateEMN(pTargetEMN, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
void CEmrTopicView::OnLastTopic()
{
	try {
		CEMRTopic* pTarget = NULL;
		CEMN* pTargetEMN = NULL;

		// (j.dinatale 2012-09-20 11:33) - PLID 52702 - handle moving to the previous topic via the last topic button
		if(GetEmrFrameWnd()->GetEMRSavePref() == 2){
			CEMN * pActiveEMN = GetAttachedEMN();

			if(pActiveEMN && !pActiveEMN->IsTemplate() && !pActiveEMN->IsLockedAndSaved() && pActiveEMN->IsUnsaved()){
				GetEmrFrameWnd()->GetEmrTreeWnd()->SaveEMR(esotEMN, (long)pActiveEMN, FALSE);
			}
		}

		// navigate to last topic of this EMN
		if (CEMRTopic* pActiveTopic = GetActiveTopic()) {
			pTarget = GetAttachedEMN()->GetLastDisplayedTreeTopic();
			if (pTarget && pTarget != pActiveTopic) {
				ShowTopicAnimated(pTarget, true);
				GetEmrFrameWnd()->ActivateTopic(pTarget);
				return;
			}
		}

		// otherwise, navigate to the beginning of the previous EMN
		if (pTargetEMN = GetAttachedEMN()->GetNextEMN()) {
			pTarget = pTargetEMN->GetFirstDisplayedTreeTopic();

			if (pTarget) {
				GetEmrFrameWnd()->ActivateTopic(pTarget);
			} else {
				GetEmrFrameWnd()->EnsureEmrTopicView(pTargetEMN);
				GetEmrFrameWnd()->ActivateEMN(pTargetEMN, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-16 14:56) - PLID 50431 - Restore the focus window if possible, otherwise do the default which gives focus to the CEmrTopicView itself
void CEmrTopicView::OnSetFocus(CWnd* pOldWnd)
{
	try {
		if (!RestoreFocusHierarchy()) {
			CScrollView::OnSetFocus(pOldWnd);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrTopicView::OnKillFocus(CWnd* pNewWnd)
{
	try {
		ASSERT(TRUE);
		CScrollView::OnKillFocus(pNewWnd);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-16 14:56) - PLID 50431 - Saves the focus hierarchy
bool CEmrTopicView::SaveFocusHierarchy()
{
	// save focus window if focus is on this window's controls
	CWnd* pWndFocus = GetFocus();

	if (!pWndFocus->GetSafeHwnd()) return false;
	if (!IsChild(pWndFocus)) return false;

	m_focusHierarchy.clear();

	while (pWndFocus->GetSafeHwnd() && pWndFocus != this) {
		m_focusHierarchy.push_back(pWndFocus->GetSafeHwnd());
		pWndFocus = pWndFocus->GetParent();
	}

	return true;
}

// (a.walling 2012-05-16 14:56) - PLID 50431 - Restores the focus window by skipping invalid windows in the hierarchy
bool CEmrTopicView::RestoreFocusHierarchy()
{
	foreach (HWND hwnd, m_focusHierarchy) {
		if (!::IsWindow(hwnd) || !::IsChild(m_hWnd, hwnd) ) {
			continue;
		}
		m_focusHierarchy.clear();
#ifdef _DEBUG
		CWnd* pWndFocus = CWnd::FromHandle(hwnd);
#endif
		::SetFocus(hwnd);
		return true;
	}

	return false;
}

