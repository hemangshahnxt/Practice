#pragma once

#include <NxUILib/NxHtmlControl.h>

#include <vector>

// (a.walling 2012-08-28 08:15) - PLID 52320 - HTML interface container for interacting with data


// CStampSearchCtrl

class CStampSearchCtrl : public CNxHtmlControl
{
	DECLARE_DYNAMIC(CStampSearchCtrl)

public:
	CStampSearchCtrl();
	virtual ~CStampSearchCtrl();

protected:
	/// CNxHtmlControl virtual	
	virtual void OnInitializing();

	/// Internal methods

	/// DocHostUIHandler virtual
	virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt,
		LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved) override;	

	// allow Enter / Escape to pass through to the web page rather than be eaten by the dialog resulting in IDOK / IDCANCEL commands
	virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg,
		const GUID* pguidCmdGroup, DWORD nCmdID) override
	{
		if (lpMsg && lpMsg->message == WM_KEYDOWN) {
			switch (lpMsg->wParam) {
				case VK_RETURN:
				case VK_ESCAPE:
					::DispatchMessage(lpMsg);
					return S_OK;
			}
		}
		return S_FALSE;
	}

	// (a.walling 2012-05-08 16:04) - PLID 50105 - Context menu
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg BSTR GetAvailableStampsJson();
	afx_msg void OnStampClicked(long nStampID);

	// (a.walling 2012-08-28 08:15) - PLID 52322 - Recent stamps
	std::vector<EMRImageStamp*> GetRecentStamps(); // sorted ascending, old to new
	void SaveRecentStamps(long nNewStampID = -1);

	DECLARE_DISPATCH_MAP();
	DECLARE_MESSAGE_MAP()
};


