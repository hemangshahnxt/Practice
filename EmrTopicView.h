#pragma once

#include "EmrHelpers.h"
#include "EmrTopicWnd.h"
#include <Guard.h>
#include <NxScrollAnchor.h>
#include <vector>
#include <NxUILib/SafeMsgProc.h>

// (a.walling 2011-10-20 21:22) - PLID 46078 - Facelift - EMR Document / View / Command routing etc

// (a.walling 2012-02-29 06:42) - PLID 46644 - Supprting template editor

class CEMNMoreInfoDlg;

// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - This is now a CScrollView
// (a.walling 2012-07-05 10:54) - PLID 50853 - Use SafeMsgProc to implement try/catch around window messages and commands
class CEmrTopicView : public SafeMsgProc<CScrollView>, public Emr::AttachedEMNImpl, public Emr::InterfaceAccessImpl<CEmrTopicView>
{
	DECLARE_DYNCREATE(CEmrTopicView)
public:
	CEmrTopicView();
	~CEmrTopicView();

	// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Call the view to show the topic
	CEmrTopicWndPtr EnsureTopicWnd(CEMRTopic* pTopic, bool bForceReposition = false);
	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities
	CEmrTopicWndPtr EnsureTopic(CEMRTopic* pTopic);

	CEmrTopicWndPtr ShowTopic(CEMRTopic* pTopic);
	
	// (a.walling 2011-12-18 14:15) - PLID 49880 - Smooth topic scrolling / toggling
	void ShowTopicAnimated(CEMRTopic* pTopic, bool bShow = true);

	CEmrTopicWndPtr ShowTopicOnActivate(CEMRTopic* pTopic);
	void ScrollToTopic(CEMRTopic* pTopic);
	void EnsureTopicInView(CEMRTopic* pTopic);

	class CEmrDocument* GetEmrDocument();

	// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window!
	void ResizeTopicWnd(CEmrTopicWndPtr pTopicWnd);

	// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Reposition and show/hide all topic windows and headers
	void RefreshTopicWindowPositions(bool bForceReposition = false);

	void ForceRefreshTopicWindowPositions()
	{
		RefreshTopicWindowPositions(true);
	}

	void SetForceNextRefreshTopicWindowPositions()
	{ 
		m_bForceRefreshNextRefreshTopicWindowPositions = true; 
	}

	bool IsRefreshingTopicWindowPositions() const;

	CEMRTopic* GetOrFindActiveTopic()
	{
		CEMRTopic* pActiveTopic = GetActiveTopic();
		if (!pActiveTopic) {
			CEmrTopicWndPtr pActiveTopicWnd = FindActiveTopicWnd();
			if (pActiveTopicWnd) {
				pActiveTopic = pActiveTopicWnd->GetTopic();
			}
		}
		return pActiveTopic;
	}

	// (j.dinatale 2012-09-19 14:57) - PLID 52702 - need to be able to find the first or active topic
	CEMRTopic* GetActiveTopicOrFirst();

	CEMRTopic* GetActiveTopic() const
	{
		if (!this || !m_pActiveTopicWnd || !m_pActiveTopicWnd->GetSafeHwnd() || !m_pActiveTopicWnd->GetTopic()) {
			return NULL;
		}

		return m_pActiveTopicWnd->GetTopic();
	}

	CEmrTopicWndPtr GetActiveTopicWnd() const
	{
		if (!m_pActiveTopicWnd || !m_pActiveTopicWnd->GetSafeHwnd() || !m_pActiveTopicWnd->GetTopic()) {
			return CEmrTopicWndPtr();
		}

		return m_pActiveTopicWnd;
	}

	void SetActiveTopicWnd(CEmrTopicWndPtr pTopicWnd);

	void SetAllowEdit(BOOL bAllowEdit);

	//CBrush& GetBackgroundBrush();
	//COLORREF GetBackgroundColor();
	
	// (a.walling 2012-03-30 10:48) - PLID 49329 - Call when the EMN's tree has changed (deleting a topic / subtopic)
	void HandleTreeChanged();
	
	// (a.walling 2012-03-20 09:24) - PLID 48990 - Ensure the ribbon item has its menu fully populated with all data
	// (a.walling 2012-05-25 17:39) - PLID 50664 - Now forwarded to views too, yay
	virtual void SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement);

public:
	////
	/// Custom events	

	void OnTopicHeaderClicked(CEMRTopic* pTopic);

public:	
	////
	/// UI state helpers
	bool IsTemplate();
	bool IsEditMode();
	bool IsAllowEdit();
	bool IsLocked();
	bool IsWritable();
	bool IsReadOnly();
	bool CanWriteToEMR();
	bool CanCreateEMN();
	bool CanWriteToEMN();
	bool CanWriteToTopic();

protected:

	////
	/// Command UI handlers	
	void OnUpdateNewEMN(CCmdUI* pCmdUI);

	void OnUpdate_AddItems(CCmdUI* pCmdUI);

	void OnUpdateClassicBtnAddTopics(CCmdUI* pCmdUI);
	void OnUpdateNewTopic(CCmdUI* pCmdUI);
	void OnUpdateNewSubtopic(CCmdUI* pCmdUI);
	void OnUpdateImportTopics(CCmdUI* pCmdUI);
	void OnUpdateImportSubtopics(CCmdUI* pCmdUI);

	void OnUpdateNewTodo(CCmdUI* pCmdUI);
	void OnUpdateNewRecording(CCmdUI* pCmdUI);

	// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
	// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
	void OnUpdateTopicList(CCmdUI* pCmdUI);
	void OnUpdateFirstTopic(CCmdUI* pCmdUI);
	void OnUpdatePreviousTopic(CCmdUI* pCmdUI);
	void OnUpdateNextTopic(CCmdUI* pCmdUI);
	void OnUpdateLastTopic(CCmdUI* pCmdUI);

	////
	/// Commands

	void OnAddItem();
	void OnAddImage();
	void OnAddTextMacro();
	void OnAddSignature();
	// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
	void OnAddOtherUsersSignature();

	void OnNewTopic();
	void OnNewSubtopic();
	void OnImportTopics();
	void OnImportSubtopics();

	// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
	// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
	void OnTopicList();
	void OnFirstTopic();
	void OnPreviousTopic();
	void OnNextTopic();
	void OnLastTopic();

protected:
	/// scrolling

	// (a.walling 2011-12-18 14:15) - PLID 49880 - Smooth topic scrolling / toggling
	void ScrollToPositionAnimated(CPoint pt);
	
	//TES 3/7/2011 - PLID 42699 - Variables to track when we're dragging the window around.
	// (a.walling 2011-03-19 12:23) - PLID 42694 - This stuff is mostly handled in NxScrollAnchor now
	scoped_ptr<CNxScrollAnchor> m_pScrollAnchor;
	bool StartAnchorScroll(UINT nFlags, const CPoint& point, UINT nMessageButtonUp, BOOL bDragOnly);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	CEmrTopicWndPtr GetTopicWndFromPoint(CPoint pt);
	CEmrTopicWndPtr FindActiveTopicWnd();

protected:
	//CBrush m_brBackground;

	CEmrTopicHeaderWndPtr EnsureTopicHeaderWnd(CEMRTopic* pTopic);

	std::vector<CEmrTopicWndPtr> m_topicWnds;
	std::vector<CEmrTopicHeaderWndPtr> m_topicHeaderWnds;

	CEmrTopicWndPtr m_pActiveTopicWnd;

	// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Reposition and show/hide all topic windows and headers
	CBoolGuard m_bRefreshingTopicWindowPositions;
	bool m_bForceRefreshNextRefreshTopicWindowPositions;
	
	bool SaveFocusHierarchy();					// (a.walling 2012-05-16 14:56) - PLID 50431 - Saves the focus hierarchy
	bool RestoreFocusHierarchy();				// (a.walling 2012-05-16 14:56) - PLID 50431 - Restores the focus window by skipping invalid windows in the hierarchy
	std::vector<HWND> m_focusHierarchy;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);		// (a.walling 2012-05-16 14:56) - PLID 50431 - Restore the focus window if possible, otherwise do the default which gives focus to the CEmrTopicView itself
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnParentNotify(UINT message, LPARAM lParam);

	virtual void OnInitialUpdate() override;
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE) override;
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	// (a.walling 2012-05-16 14:56) - PLID 50431 - Save the focus hierarchy when deactivating
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd) override;
		
	virtual void OnDraw(CDC* pDC) override;
};

inline bool CEmrTopicView::IsRefreshingTopicWindowPositions() const
{
	return m_bRefreshingTopicWindowPositions;
}
