#pragma once
// NxDialog.h : header file
//

// CNxDialog dialog

// (a.walling 2010-06-09 16:03) - PLID 39087 - Most of this is all in CNexTechDialog now. However some practice-specific stuff is kept here.

class CNxDialog : public CNexTechDialog
{
	DECLARE_DYNAMIC(CNxDialog);
	
public:
	// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent
	CNxDialog(CWnd* pParent);
	CNxDialog(int IDD, CWnd *pParent);
	CNxDialog(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT);
	virtual ~CNxDialog();

	virtual void Print(CDC *pDC, CPrintInfo *pInfo);

	void Capitalize (int ID);
	// (d.moore 2007-04-23 11:46) - PLID 23118 - Capitalizes all text for a DlgItem.
	void CapitalizeAll (int ID);
	// (a.wetta 2007-03-19 14:55) - PLID 24983 - Add function to fix capitalization
	//TES 4/5/2010 - PLID 38040 - Moved to GlobalUtils
	//CString FixCapitalization(CString strCapitalized);
	void FormatItem (int ID, CString format);

	// You can use GetDlgItemUnknown as an alternative to this, if for some reason you are using CNexTechDialog rather than CNxDialog.
	LPUNKNOWN BindNxDataListCtrl(UINT nID, bool bAutoRequery = true);
	LPUNKNOWN BindNxDataList2Ctrl(UINT nID, bool bAutoRequery = true);

	LPUNKNOWN BindNxDataListCtrl(UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery = true);
	//DRT 4/17/2008 - PLID 29678
	LPUNKNOWN BindNxDataListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery);
	LPUNKNOWN BindNxDataList2Ctrl(UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery = true);
	//DRT 4/17/2008 - PLID 29678
	LPUNKNOWN BindNxDataList2Ctrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery);

	// (z.manning 2010-07-07 14:30) - PLID 36511
	void EndDialogAndModalParent(int nResult);
	
	// (j.jones 2008-12-30 11:30) - PLID 32585 - added OnKickIdle to handle when a modal NxDialog is idle
	afx_msg virtual LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	// (a.walling 2013-11-11 14:17) - PLID 59412 - so we don't have to write the if (*lppNewSel == NULL) { SafeSetComPtr / etc boilerplate over and over again.
	void RequireDataListSel(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

public:
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Notifies active dialog when the parent view is activated/deactivated
	virtual void OnParentViewActivate(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) {};
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();

private:
	// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Saving & Recalling dialog size and position
	CString m_strSizeAndPositionConfigRT;
	void RecallSizeAndPosition();
	void SaveSizeAndPosition();
public:
	void SetRecallSizeAndPosition(const CString& strSizeAndPositionConfigRT, bool bRecallNow);

	// (a.walling 2013-11-27 15:55) - PLID 59857 - AutoSet as member of CNxDialog
	bool AutoSet(int id, NXB_TYPE type, DWORD attributes = NXIB_ALL);
	bool AutoSet(CWnd* pWnd, NXB_TYPE type, DWORD attributes = NXIB_ALL);
};

///

// (a.walling 2013-02-05 12:15) - PLID 55018 - CNxModelessDialog - CNxDialog-derived class for modeless dialogs, both owned or not.
// Will automatically use WS_EX_APPWINDOW exstyle to give it an icon in the taskbar
class CNxModelessDialog : public CNxDialog
{
	DECLARE_DYNAMIC(CNxModelessDialog);

	// Once more windows are consolidated under this class, it may be useful to expand its scope
	// for better automatic modeless window management.

public:
	DECLARE_MESSAGE_MAP()

public:
	CNxModelessDialog(CWnd* pParent);
	CNxModelessDialog(int IDD, CWnd *pParent);
	CNxModelessDialog(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT);

	virtual ~CNxModelessDialog();

	// will always add the WS_EX_APPWINDOW exstyle
	// override and bypass if you want to avoid this
	virtual void PreSubclassWindow();

	// hides, but does not destroy, the dialog
	virtual void CloseDialog();

	// show or hide the dialog, calling CloseDialog if SW_HIDE, otherwise adapting the standard ShowWindow commands
	virtual void ShowDialog(int nCmd = SW_SHOW);

protected:
};

///

// (a.walling 2013-02-05 12:15) - PLID 55018 - CNxModelessOwnedDialog - prevents owner window activation
class CNxModelessOwnedDialog : public CNxModelessDialog
{
	DECLARE_DYNAMIC(CNxModelessOwnedDialog);

public:
	DECLARE_MESSAGE_MAP()

public:
	CNxModelessOwnedDialog(CWnd* pParent);
	CNxModelessOwnedDialog(int IDD, CWnd *pParent);
	CNxModelessOwnedDialog(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT);

	virtual ~CNxModelessOwnedDialog();

	// overrides to ensure Close is called (and, vicariously, PassFocus)
	virtual void OnOK();
	virtual void OnCancel();
	void EndDialog(int nResult);

	virtual BOOL DestroyWindow();

	// calls PassFocus, and then hides the window. Without PassFocus, the owner will be activated.
	virtual void CloseDialog();

	void ShowWindow(int nCmd);

	// Passes focus to next top-level window in the thread.
	void PassFocus();

protected:

	afx_msg void OnClose();

	// (a.walling 2014-06-09 09:37) - PLID 57388 - Placeholders - useful for debugging without rebuilding everything
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnActivateTopLevel(WPARAM wParam, LPARAM lParam);

};

