// OMRReviewBrowserCtrl.cpp : implementation file
//

#include "StdAfx.h"
#include "OMRReviewBrowserCtrl.h"

// COMRReviewBrowserCtrl

// (j.dinatale 2012-09-14 10:21) - PLID 51911 - necessary to prevent the context menu from showing up.

IMPLEMENT_DYNAMIC(COMRReviewBrowserCtrl, CNxHtmlControl)

COMRReviewBrowserCtrl::COMRReviewBrowserCtrl()
{
}

COMRReviewBrowserCtrl::~COMRReviewBrowserCtrl()
{
}

void COMRReviewBrowserCtrl::OnInitializing()
{
	// (b.spivey, February 27, 2013) - PLID  - Deprecated
	// (j.fouts 2012-12-13 15:48) - PLID 54184 - Use the OmrReview.html
	//LoadUrl("nxres://0/OmrReview.html");
}

BEGIN_MESSAGE_MAP(COMRReviewBrowserCtrl, CNxHtmlControl)
	ON_WM_CONTEXTMENU()
	//ON_WM_DESTROY()
END_MESSAGE_MAP()
/*
BEGIN_DISPATCH_MAP(COMRReviewBrowserCtrl, CCmdTarget)
	//{{AFX_DISPATCH_MAP(COMRReviewBrowserCtrl)
	DISP_FUNCTION(COMRReviewBrowserCtrl, "GetImageUrl", GetImageUrl, VT_BSTR, VTS_NONE)

	DISP_CALLBACK(COMRReviewBrowserCtrl, "WhenImageReady", m_callbackImageReady)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()*/

HRESULT COMRReviewBrowserCtrl::OnShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
	return S_OK;
}

void COMRReviewBrowserCtrl::OnContextMenu(CWnd* pWnd, CPoint pos)
{
}
/*
// (j.fouts 2012-12-13 15:49) - PLID 54184 - Callback to tell the javascript the image file is ready to be loaded
void COMRReviewBrowserCtrl::OnImageReady()
{
	// notify html control
	//if (m_callbackImageReady) {
	//	CNxScriptCallback callback = m_callbackImageReady;

	//	callback.Invoke(this);
	//}
}

// (j.fouts 2012-12-13 15:49) - PLID 54184 - Set the image url and notifty the javascript
void COMRReviewBrowserCtrl::SetImageUrl(const CString& strImageUrl)
{
	m_strImageUrl = strImageUrl;
	OnImageReady();
}

// (j.fouts 2012-12-13 15:49) - PLID 54184 - External function for javascript to get the image url
BSTR COMRReviewBrowserCtrl::GetImageUrl()
{
	return m_strImageUrl.AllocSysString();
}

// (j.fouts 2012-12-13 15:49) - PLID 54184 - When the document if finished loading set the image url
void COMRReviewBrowserCtrl::OnDocumentLoaded(LPDISPATCH pDisp, VARIANT *URL)
{
	OnImageReady();
}

void COMRReviewBrowserCtrl::OnDestroy()
{
	if (m_callbackImageReady) {
		m_callbackImageReady.Clear();
	}
}
*/