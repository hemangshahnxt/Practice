#pragma once

#include "WindowlessUtils.h"
#include "EmrHelpers.h"

class CEMRTopic;

// (a.walling 2011-10-28 13:24) - PLID 46176 - Topic window headers
// (a.walling 2012-06-13 16:37) - PLID 46635 - Ensure this uses CNxDialogMenuSupport since clicking the labels etc won't pretranslate up to the view
class CEmrTopicHeaderWnd : public CNxDialogMenuSupport<CWnd>, public Emr::TopicInterfaceAccessImpl<CEmrTopicHeaderWnd>, public boost::enable_shared_from_this<CEmrTopicHeaderWnd>
{
public:
	explicit CEmrTopicHeaderWnd(CEMRTopic* pTopic, bool bIsTemplate);

	void Initialize(CWnd* pParent);

	void OnActiveTopicChanged();

	bool IsActiveTopic();

	const CSize& GetContentSize() const {
		return m_szContentSize;
	}

	void RefreshLabels(); // also sets m_szContentSize
	
	CEMRTopic* GetTopic()
	{
		return m_pTopic;
	}

	CWnd* HitTest(CPoint ptScreen);
	
	//void DrawFittedText(CDC& dc, CRect rc, const CString& strText);

	// Generated message map functions
protected:	

	// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
	NxWindowlessLib::NxFreeLabelControl m_wndLabel;
	NxWindowlessLib::NxFreeLabelControl m_wndExpand;

	// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
	struct CParentTopicLabel
	{
		CParentTopicLabel(CEMRTopic* pTopic = NULL)
			: pTopic(pTopic)
		{
		}

		~CParentTopicLabel()
		{
			if (IsControlValid(&label)) {
				label.DestroyWindow();
			}
		}

		CEMRTopic* pTopic;
		NxWindowlessLib::NxFreeLabelControl label;
	};
	
	typedef std::vector<shared_ptr<CParentTopicLabel>> ParentLabelVector;
	ParentLabelVector m_parentLabels;

	shared_ptr<CParentTopicLabel> CreateParentLabel(UINT nID);

	CParentTopicLabel& EnsureParentLabel(size_t ix);

	///

	CEMRTopic* m_pTopic;

	CSize m_szContentSize;

	bool m_bIsActive;
	bool m_bIsTemplate;


	CSize GetLabelSize(CDC& dc, CWnd* pWnd);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);	
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg LRESULT OnNcHitTest(CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void OnLabelClickedEvent();
	BOOL OnParentLabelClickedEvent(UINT nID);
	void OnLabelClicked();
	void OnParentLabelClicked(UINT nID); // (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
};

typedef shared_ptr<CEmrTopicHeaderWnd> CEmrTopicHeaderWndPtr;
typedef weak_ptr<CEmrTopicHeaderWnd> CEmrTopicHeaderWndRef;
