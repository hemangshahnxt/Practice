#pragma once

#include <NxUILib/NxHtmlControl.h>

// COMRReviewBrowserCtrl

// (j.dinatale 2012-09-14 10:21) - PLID 51911 - necessary to prevent the context menu from showing up.

class COMRReviewBrowserCtrl : public CNxHtmlControl
{
	DECLARE_DYNAMIC(COMRReviewBrowserCtrl)

public:
	COMRReviewBrowserCtrl();
	virtual ~COMRReviewBrowserCtrl();

	// (b.spivey, February 27, 2013) - PLID  - Deprecated. 
	// (j.fouts 2012-12-13 15:48) - PLID 54184 - Set the image url and notifty the javascript
	//void SetImageUrl(const CString& strImageUrl);

protected:
	/// CNxHtmlControl virtual	
	virtual void OnInitializing();

	/// Internal methods
	// (b.spivey, February 27, 2013) - PLID  - Deprecated
	/*
	// (j.fouts 2012-12-13 15:48) - PLID 54184 - External function for javascript to get the image url
	BSTR GetImageUrl();
	CString m_strImageUrl;

	// (j.fouts 2012-12-13 15:48) - PLID 54184 - Callback to tell the javascript the image file is ready to be loaded
	CNxScriptCallback m_callbackImageReady;
	void OnImageReady();

	// (j.fouts 2012-12-13 15:48) - PLID 54184 - When the document if finished loading set the image url
	virtual void OnDocumentLoaded(LPDISPATCH pDisp, VARIANT *URL);
	*/

	/// DocHostUIHandler virtual
	virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved) override;


	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	//afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
	//DECLARE_DISPATCH_MAP()
};