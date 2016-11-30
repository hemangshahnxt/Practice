// EmrTopicHeaderWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EmrTopic.h"
#include "EmrTopicHeaderWnd.h"
#include "EmrTopicView.h"
#include "EmrItemAdvDlg.h"
#include "NxOccManager.h"
#include "GlobalDrawingUtils.h"
#include <foreach.h>
#include "EmrColors.h"

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

////
/// CEmrTopicHeaderWnd
// (a.walling 2011-10-28 13:24) - PLID 46176 - Topic window headers

// (a.walling 2014-04-29 17:03) - PLID 61968 - Ensure m_pTopic is not null before it is used anywhere

#define IDC_EMR_TOPIC_HEADER_LABEL 1000
#define IDC_EMR_TOPIC_HEADER_EXPAND 1001
#define IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE 1002

CEmrTopicHeaderWnd::CEmrTopicHeaderWnd(CEMRTopic* pTopic, bool bIsTemplate)
	: m_pTopic(pTopic)
	, m_bIsActive(false)
	, m_bIsTemplate(bIsTemplate)
{
}

BEGIN_MESSAGE_MAP(CEmrTopicHeaderWnd, CWnd)	
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_EMR_TOPIC_HEADER_LABEL, OnLabelClicked)
	ON_BN_CLICKED(IDC_EMR_TOPIC_HEADER_EXPAND, OnLabelClicked)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE, IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE + 48, OnParentLabelClicked)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrTopicHeaderWnd, CWnd)
    //{{AFX_EVENTSINK_MAP(CEmrTopicHeaderWnd)
	ON_EVENT(CEmrTopicHeaderWnd, IDC_EMR_TOPIC_HEADER_LABEL, DISPID_CLICK /* Click */, CEmrTopicHeaderWnd::OnLabelClickedEvent, VTS_NONE)
	ON_EVENT(CEmrTopicHeaderWnd, IDC_EMR_TOPIC_HEADER_EXPAND, DISPID_CLICK /* Click */, CEmrTopicHeaderWnd::OnLabelClickedEvent, VTS_NONE)
	// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
	ON_EVENT_RANGE(CEmrTopicHeaderWnd, IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE, IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE + 48, DISPID_CLICK /* Click */, CEmrTopicHeaderWnd::OnParentLabelClickedEvent, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrTopicHeaderWnd::Initialize(CWnd* pParent)
{
	// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
	LPCTSTR szWndClass = AfxRegisterWndClass(CS_PARENTDC|CS_NOCLOSE, NULL, EmrColors::Topic::Background(m_bIsTemplate));
	BOOL bAns = CreateEx(
		0, 
		szWndClass, NULL, 
		WS_VISIBLE|WS_CHILD|WS_GROUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), pParent, -1);

	ASSERT(bAns);
}

void CEmrTopicHeaderWnd::OnActiveTopicChanged()
{
	if (!::IsWindow(GetSafeHwnd())) return;

	if (m_pTopic && m_pTopic == GetEmrTopicView()->GetActiveTopic()) {
		m_bIsActive = true;
		m_wndLabel->ForeColor = RGB(0x00, 0x30, 0x70);
	} else {
		m_bIsActive = false;
		m_wndLabel->ForeColor = RGB(0x00, 0x00, 0x00);
	}
}

bool CEmrTopicHeaderWnd::IsActiveTopic()
{
	if (!m_bIsActive) {
		return false;
	}

	if (m_pTopic && GetEmrTopicView() && m_pTopic == GetEmrTopicView()->GetActiveTopic()) {
		return true;
	} else {
		OnActiveTopicChanged();
		return false;
	}
}

LRESULT CEmrTopicHeaderWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - forward extra messages to windowless controls
	LRESULT lResult = 0;
	if (HandleExtraWindowlessMessages(m_pCtrlCont, message, wParam, lParam, &lResult)) {
		return lResult;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CEmrTopicHeaderWnd::PreTranslateMessage(MSG* pMsg)
{
	// allow tooltip messages to be filtered
	// (a.walling 2012-06-13 16:37) - PLID 46635 - Ensure this uses CNxDialogMenuSupport via __super since clicking the labels etc won't pretranslate up to the view
	if (__super::PreTranslateMessage(pMsg))
		return TRUE;

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

CWnd* CEmrTopicHeaderWnd::HitTest(CPoint ptScreen)
{
	if (IsControlValid(&m_wndLabel)) {
		CRect rc;
		GetControlWindowRect(this, &m_wndLabel, rc);

		if (rc.PtInRect(ptScreen)) {
			return &m_wndLabel;
		}

		GetControlWindowRect(this, &m_wndExpand, rc);

		if (rc.PtInRect(ptScreen)) {
			return &m_wndExpand;
		}

		foreach (ParentLabelVector::value_type parentLabel, m_parentLabels) {
			GetControlWindowRect(this, &parentLabel->label, rc);

			if (rc.PtInRect(ptScreen)) {
				return &parentLabel->label;
			}
		}
	}
	return NULL;
}

LRESULT CEmrTopicHeaderWnd::OnNcHitTest(CPoint pt)
{
	if (HitTest(pt)) {
		return HTCLIENT;
	}

	return HTTRANSPARENT;
}

int CEmrTopicHeaderWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	try {
		if (CWnd::OnCreate(lpCreateStruct) == -1)
			return -1;

		// (a.walling 2014-04-29 17:03) - PLID 61968 - ensure this is valid before dereferencing
		if (m_pTopic) {
			m_pTopic->SetTopicHeaderWnd(shared_from_this());
		}

		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()), FALSE);

		CRect rc;
		GetClientRect(&rc);

		m_wndLabel.CreateControl("", WS_VISIBLE | WS_GROUP | WS_TABSTOP, CRect(0, 0, 0, 0), this, IDC_EMR_TOPIC_HEADER_LABEL);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_wndLabel->NativeFont = EmrFonts::GetTopicHeaderFont();
		OnActiveTopicChanged(); // updates the ForeColor
		m_wndLabel->Interactive = VARIANT_TRUE;
		m_wndLabel->AlignBottom = VARIANT_TRUE;

		m_wndExpand.CreateControl("", WS_VISIBLE | WS_GROUP | WS_TABSTOP, CRect(0, 0, 0, 0), this, IDC_EMR_TOPIC_HEADER_EXPAND);
		m_wndExpand->NativeFont = EmrFonts::GetTopicHeaderWebdingsFont();
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_wndExpand->ForeColor = lerpColor(RGB(0x00, 0x00, 0x00), EmrColors::Topic::Background(m_bIsTemplate), 0.40);
		m_wndExpand->Interactive = VARIANT_TRUE;
		m_wndExpand->AlignBottom = VARIANT_TRUE;

		RefreshLabels();

	} NxCatchAll(__FUNCTION__);
	
	// Return success
	return 0;
}

// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
shared_ptr<CEmrTopicHeaderWnd::CParentTopicLabel> CEmrTopicHeaderWnd::CreateParentLabel(UINT nID)
{
	shared_ptr<CEmrTopicHeaderWnd::CParentTopicLabel> pParentLabel = make_shared<CParentTopicLabel>();

	NxWindowlessLib::NxFreeLabelControl& label(pParentLabel->label);

	label.CreateControl("", WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), this, nID);

	static COLORREF normalColor = 0;
	if (!normalColor) {
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		normalColor = lerpColor(RGB(0x00, 0x00, 0x00), EmrColors::Topic::Background(m_bIsTemplate), 0.60);
	}
	
	COLORREF foreColor;

	if (IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE == nID) {
		// 'top'
		static COLORREF topColor = 0;
		if (!topColor) {
			// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
			topColor = lerpColor(foreColor, EmrColors::Topic::Background(m_bIsTemplate), 0.20);
		}
		foreColor = topColor;
	} else {
		foreColor = normalColor;
	}


	// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
	label->NativeFont = EmrFonts::GetTopicHeaderSmallerFont();
	label->ForeColor = foreColor;
	label->Interactive = VARIANT_TRUE;
	label->AlignBottom = VARIANT_TRUE;

	return pParentLabel;
}

template<long Alignment>
inline void AlignUp(long& x)
{
	x += (Alignment - 1);
	x &= ~(Alignment - 1);
}

template<long Alignment>
inline void AlignDown(long& x)
{
	x &= ~Alignment;
}

void CEmrTopicHeaderWnd::RefreshLabels()
{
	if (!::IsWindow(GetSafeHwnd())) return;

	static const int cnExpandBuffer = 4;
	static const int cnTopicIndent = 12;
	static const int cnParentTopicBuffer = 16;
	CClientDC dc(this);

	CString strLabel;

	CPoint pos(0, 0);

	long nIndent = cnExpandBuffer;
	{
		CEMRTopic* pParentTopic = m_pTopic ? m_pTopic->GetParentTopic() : NULL;
		while (pParentTopic) {
			nIndent += cnTopicIndent;
			pParentTopic = pParentTopic->GetParentTopic();
		}
	}
	
	m_szContentSize.SetSize(0, 0);

	CSize szMainLabel(0, 0);

	// first the main topic label
	{
		strLabel = GetControlText(&m_wndLabel);

		// (a.walling 2012-04-30 10:58) - PLID 46176 - Ensure something is displayed rather than nothing if the topic's label is blank
		CString strTopicName = m_pTopic ? ConvertToControlText(m_pTopic->GetDisplayName()) : strLabel;

		if (strLabel != strTopicName) {
			m_wndLabel.SetWindowText(strTopicName);

			szMainLabel = GetLabelSize(dc, &m_wndLabel);
		} else {
			szMainLabel = GetControlSize(&m_wndLabel);
		}


		// expander
		{
			bool bExpanded = false;
			if (m_pTopic) {
				CEmrTopicWndPtr pTopicWnd = m_pTopic->GetTopicWnd();

				if (pTopicWnd && pTopicWnd->IsWindowVisible()) {
					bExpanded = true;
				}
			}

			strLabel = GetControlText(&m_wndExpand);

			static const char* cszExpanded = "6";
			static const char* cszCollapsed = "4";

			if (bExpanded) {
				if (strLabel != cszExpanded) {
					m_wndExpand.SetWindowText(cszExpanded);
				}
			} else {
				if (strLabel != cszCollapsed) {
					m_wndExpand.SetWindowText(cszCollapsed);
				}
			}

			CSize szExpand(24, szMainLabel.cy);

			MoveChildControl(this, &m_wndExpand, CRect(pos, szExpand));

			pos.x += szExpand.cx;
			pos.x += nIndent;
		}

		MoveChildControl(this, &m_wndLabel, CRect(pos, szMainLabel));

		pos.x += szMainLabel.cx;

		if (m_szContentSize.cy < szMainLabel.cy) {
			m_szContentSize.cy = szMainLabel.cy;
		}
	}

	m_szContentSize.cx = pos.x;

	if (!IsActiveTopic()) {
		m_parentLabels.clear();
		GetControlContainer()->m_pSiteFocus = m_wndLabel.GetControlSite();
		return;
	} else {
		bool bMouseInRect = false;

		CPoint pt;
		::GetCursorPos(&pt);
		CRect rc;
		GetWindowRect(&rc);
		if (rc.PtInRect(pt)) {
			bMouseInRect = true;
		} else if (m_pTopic) {
			if (CEmrTopicWndPtr pTopicWnd = m_pTopic->GetTopicWnd()) {
				if (pTopicWnd->IsWindowVisible()) {
					pTopicWnd->GetWindowRect(&rc);					
					if (rc.PtInRect(pt)) {
						bMouseInRect = true;
					}
				}
			}
		}

		if (bMouseInRect) {
			CWnd* pOverWnd = CWnd::WindowFromPoint(pt);
			if (!pOverWnd) {
				bMouseInRect = false;
			} else if (pOverWnd != GetParent() && !GetParent()->IsChild(pOverWnd)) {
				bMouseInRect = false;
			}
		}		

		if (!bMouseInRect) {
			m_parentLabels.clear();
			GetControlContainer()->m_pSiteFocus = m_wndLabel.GetControlSite();
			return;
		}
	}

	// (a.walling 2012-02-23 16:10) - PLID 48371 - now the parent topics
	CRect rcClient;
	GetClientRect(&rcClient);

	CPoint posTopics(rcClient.right - cnParentTopicBuffer, pos.y);

	CSize sz;

	std::vector<CEMRTopic*> parentChain;
	parentChain.reserve(16);

	if (m_pTopic) {
		CEMRTopic* pParentTopic = m_pTopic;
		
		do {
			pParentTopic = pParentTopic->GetParentTopic();
			parentChain.push_back(pParentTopic);
		} while (pParentTopic);
	}

	CString strExistingLabel;
	size_t ix = 0;
	reverse_foreach (CEMRTopic* pParentTopic, parentChain) {
		CEmrTopicHeaderWnd::CParentTopicLabel& parentLabel(EnsureParentLabel(ix));

		bool bNeedRecalc = false;

		if (parentLabel.pTopic != pParentTopic) {
			parentLabel.pTopic = pParentTopic;
			bNeedRecalc = true;
		}

		strLabel = GetControlText(&parentLabel.label);

		// (a.walling 2012-04-30 10:58) - PLID 46176 - Ensure something is displayed rather than nothing if the topic's label is blank
		strExistingLabel = pParentTopic ? ConvertToControlText(pParentTopic->GetDisplayName()) : "top";
		if (strLabel != strExistingLabel) {
			parentLabel.label.SetWindowText(strExistingLabel);
			bNeedRecalc = true;
		}

		if (bNeedRecalc) {
			sz = GetLabelSize(dc, &parentLabel.label);
			sz.cy = szMainLabel.cy - 2;
		} else {
			sz = GetControlSize(&parentLabel.label);
		}

		posTopics.x -= sz.cx;
		if (posTopics.x < pos.x) {
			parentLabel.pTopic = NULL;
			break;
		}

		CRect rc(posTopics, sz);

		MoveChildControl(this, &parentLabel.label, rc);

		posTopics.x -= cnParentTopicBuffer;

		AlignDown<32>(posTopics.x);

		++ix;
	}

	m_parentLabels.resize(ix);

	GetControlContainer()->m_pSiteFocus = m_wndLabel.GetControlSite();
}

// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
CEmrTopicHeaderWnd::CParentTopicLabel& CEmrTopicHeaderWnd::EnsureParentLabel(size_t ix)
{
	shared_ptr<CEmrTopicHeaderWnd::CParentTopicLabel> pParentLabel;

	if (ix < m_parentLabels.size()) {
		pParentLabel = m_parentLabels.at(ix);

		pParentLabel->label.SetDlgCtrlID(IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE + ix);
	} else {
		ASSERT(ix == m_parentLabels.size());
		pParentLabel = CreateParentLabel(IDC_EMR_TOPIC_HEADER_PARENT_LABEL_BASE + ix);

		m_parentLabels.push_back(pParentLabel);
	}

	return *pParentLabel;
}

CSize CEmrTopicHeaderWnd::GetLabelSize(CDC& dc, CWnd* pWnd)
{
	CSize szLabel(LONG_MAX, LONG_MAX);
	CalcControlIdealDimensions(&dc, pWnd, szLabel, FALSE, CSize(0, 0));

	return szLabel;
}

void CEmrTopicHeaderWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	RefreshLabels();
}

BOOL CEmrTopicHeaderWnd::OnEraseBkgnd(CDC* pDC)
{
	// (a.walling 2012-06-14 15:06) - PLID 51002 - Exclude the windowless controls when erasing the background
	CNxOleControlContainer* pNxCtrlCont = polymorphic_downcast<CNxOleControlContainer*>(GetControlContainer());

	BOOL bTouchedClipRegion = FALSE;

	if (pNxCtrlCont) {		
		bTouchedClipRegion = pNxCtrlCont->ExcludeWindowlessClipRegion(pDC);
	}

	//CEmrTopicView* pEmrTopicView = GetEmrTopicView();

	//if (!pEmrTopicView) {
	//	ASSERT(FALSE);
	//	return CWnd::OnEraseBkgnd(pDC);
	//}

	//CRect rcClient;
	//GetClientRect(&rcClient);

	//pDC->FillRect(rcClient, &pEmrTopicView->GetBackgroundBrush());

	CWnd::OnEraseBkgnd(pDC);

	if (bTouchedClipRegion) {
		pDC->SelectClipRgn(NULL);
	}

	return TRUE;
}

void CEmrTopicHeaderWnd::OnPaint() 
{	
	CWnd::OnPaint();
}

HBRUSH CEmrTopicHeaderWnd::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	try {
		HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

		CEmrTopicView* pEmrTopicView = GetEmrTopicView();

		if (pEmrTopicView) {
			switch (nCtlColor) {
				case CTLCOLOR_DLG:
				case CTLCOLOR_STATIC:
					// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
					pDC->SetBkColor(EmrColors::Topic::Background(m_bIsTemplate));
					pDC->SetTextColor(RGB(0x00, 0x00, 0x00));
					break;
			}
		}

		return hbr;
	} NxCatchAll(__FUNCTION__);

	return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CEmrTopicHeaderWnd::OnLabelClickedEvent()
{
	PostMessage(WM_COMMAND, MAKEWPARAM(IDC_EMR_TOPIC_HEADER_LABEL, BN_CLICKED), 0);
}

// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
BOOL CEmrTopicHeaderWnd::OnParentLabelClickedEvent(UINT nID)
{
	PostMessage(WM_COMMAND, MAKEWPARAM(nID, BN_CLICKED), 0);
	return TRUE;
}

void CEmrTopicHeaderWnd::OnLabelClicked()
{
	try {
		GetEmrTopicView()->OnTopicHeaderClicked(m_pTopic);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-02-23 16:10) - PLID 48371 - parent topic labels in the topic header window
void CEmrTopicHeaderWnd::OnParentLabelClicked(UINT nID)
{
	try {
		foreach (ParentLabelVector::value_type parentLabel, m_parentLabels) {
			if (nID == parentLabel->label.GetDlgCtrlID()) {
				if (parentLabel->pTopic) {
					GetEmrTopicView()->EnsureTopicInView(parentLabel->pTopic);
				} else {
					GetEmrTopicView()->PostMessage(WM_COMMAND, ID_EMR_FIRST_TOPIC);
				}
				return;
			}
		}

		TRACE(__FUNCTION__" \t Warning: could not find topic for ID %lu\n", nID);
	} NxCatchAll(__FUNCTION__);
}
