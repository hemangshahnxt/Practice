#if !defined(AFX_EMRITEMADVMULTIPOPUPDLG_H__493FF8A7_87AC_4F4A_9996_A21B2806B54A__INCLUDED_)
#define AFX_EMRITEMADVMULTIPOPUPDLG_H__493FF8A7_87AC_4F4A_9996_A21B2806B54A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRItemAdvMultiPopupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvMultiPopupDlg dialog
class CEMNDetail;
class CShowProgressFeedbackDlg;
class CEMRItemAdvPopupWnd;

//TES 1/11/2008 - PLID 24157 - We need to maintain a tree of details and source details, so that if one of our details
// spawns another detail, it can be inserted in the appropriate place in the tree.
struct DetailPopup {
	friend class CEMRItemAdvMultiPopupDlg;
	//TES 1/11/2008 - PLID 24157 - This is the "payload" information for this node, the detail being spawned, its
	// source detail (which is only used for constructing the tree, matching spawners to spawnees), and its 
	// CEMRItemAdvPopupWnd, if we've created it.
	CEMNDetail *m_pDetail;
	CEMNDetail *m_pSourceDetail;

	CEMRItemAdvPopupWnd *m_pWindow;
	
	//TES 1/11/2008 - PLID 24157 - This tracks our position in the tree. Either m_pParent or m_pPrevious should always
	// be NULL.
	DetailPopup *m_pParent;
	DetailPopup *m_pChild;
	DetailPopup *m_pPrevious;
	DetailPopup *m_pNext;

	DetailPopup();
	~DetailPopup();

	//TES 1/11/2008 - PLID 24157 - Returns the very last node, in "tree order"
	DetailPopup* GetLastInTree();
	//TES 1/11/2008 - PLID 24157 - Returns the node that immediately precedes us, in "tree order"
	DetailPopup* GetPreviousInTree();
	//TES 1/11/2008 - PLID 24157 - Returns our parent if we have one, otherwise, the parent of the head of our list.
	DetailPopup* GetParentInTree();
	//TES 1/11/2008 - PLID 24157 - Returns the note that immediately follows us, in "tree order"
	DetailPopup* GetNextInTree();
	//TES 1/11/2008 - PLID 24157 - Removes the node with the given detail from the tree.  Note that this will have no 
	// effect if called on the node which contains this detail (because it can't delete itself)
	void RemoveDetailFromTree(CEMNDetail *pDetail);
};

class CEMRItemAdvMultiPopupDlg : public CNxDialog
{
	// (a.walling 2009-01-20 14:34) - PLID 29800 - Need to IMPLEMENT_/DECLARE_DYNAMIC to enable MFC's CObject-based RTTI
	DECLARE_DYNAMIC(CEMRItemAdvMultiPopupDlg);
// Construction
public:
	CEMRItemAdvMultiPopupDlg(CWnd* pParent);   // standard constructor
	~CEMRItemAdvMultiPopupDlg();

	//TES 1/11/2008 - PLID 24157 - Adds the detail to the list of details that need to be popped up (if the dialog is
	// already visible, the detail will immediately be visible as well).  This function will add a reference to pDetail,
	// but not to pSourceDetail, because the dialog doesn't actually do anything with it, it just uses it to arrange
	// the other details in the right order.
	void AddDetail(CEMNDetail* pDetail, CEMNDetail *pSourceDetail);
	//TES 1/11/2008 - PLID 24157 - Removes the detail from the list of details that need to be popped up (if the dialog
	// is already visible, the detail will be immediately taken off the screen).  No effect if the detail was never added.
	void RemoveDetail(CEMNDetail* pDetail);

	//TES 1/23/2008 - PLID 24157 - Updates the detail's state.
	// (a.walling 2010-03-25 08:17) - PLID 37802 - Pass in bRemovingItem if the item is being removed (which occurs when editing or updating the item for example)
	void HandleDetailChange(CEMNDetail* pDetail, BOOL bRemovingItem = FALSE);

	//TES 1/23/2008 - PLID 28673 - Initializes the dialog with full tree of details, from a previous instance of
	// this dialog.
	void SetPoppedUpDetails(DetailPopup *pDetails);

	//TES 1/28/2008 - PLID 28673 - Access the tree of details to popup, and additionally tells this dialog that it is
	// not responsible for cleaning them up (although, at the moment, it never actually is responsible anyway).
	DetailPopup* TakeDetailsToPopup();

	CShowProgressFeedbackDlg *m_pActionProgressDlg;

	// (a.walling 2008-12-19 09:29) - PLID 29800 - Refresh custom stamps for images in the multipopup tree (excluding pIgnore)
	void RefreshCustomStamps(LPCTSTR strNewStamps, CWnd* pIgnore);

	//TES 1/28/2008 - PLID 28673 - Tells the dialog where it was spawned from, this can be used later to make
	// it possible to re-generate this dialog.
	CEMNDetail *m_pSourceDetail;
	long m_nSourceDataGroupID;
	//TES 1/28/2008 - PLID 28673 - We also need to store the ID of the item that spawned it, as well as the
	// DataGroupID, because if a detail is loaded new (not from a template detail) it doesn't bother to load
	// the source DataGroupID, because it knows that the SourceID is enough to correctly identify it.  But
	// we want to keep storing the DataGroupID as well, because if it is loaded from a detail or template detail,
	// then we may need to use it.
	long m_nSourceDataID;
// Dialog Data
	//{{AFX_DATA(CEMRItemAdvMultiPopupDlg)
	enum { IDD = IDD_EMR_ITEM_ADV_MULTI_POPUP_DLG };
	CNxIconButton	m_nxbShowList;
	CNxIconButton	m_nxbNext;
	CNxIconButton	m_nxbPrevious;
	CNxIconButton	m_nxbOK;
	CNxIconButton	m_nxbBackToEmn;
	CNxColor	m_nxc;
	CNxStatic	m_nxstaticItemPlaceholder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRItemAdvMultiPopupDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pDetailList;

	//TES 1/11/2008 - PLID 24157 - This is the head of our tree of details (it's a tree so that as the details in the
	// tree spawn other details, we can keep everything in the right order, even though on screen it will display as 
	// a simple list).  It also contains EMRItemAdvPopupWnds for each detail, if we've created them.
	DetailPopup* m_pDetailsToPopup;

	//TES 1/14/2008 - PLID 24157 - Tracks whichever window we're currently displaying.
	CEMRItemAdvPopupWnd *m_pCurrentWnd;

	//TES 1/11/2008 - PLID 24157 - Make sure the screen reflects the details in m_pDetailsToPopup.
	void RefreshDetails();
	//TES 1/14/2008 - PLID 24157 - Makes sure all the rows in the list of details are colored appropriately.
	void RefreshDetailColors();

	//TES 1/14/2008 - PLID 24157 - Copied from CEMRItemAdvPopupDlg.
	void CreateControlsWindow(CEMNDetail *pDetail);

	//TES 1/14/2008 - PLID 24157 - Make sure that the detail being displayed matches up with the row selected in the
	// datalist.
	void RefreshDetailWindow();

	//TES 2/21/2008 - PLID 28827 - Make sure that the caption reflects the current detail.
	void RefreshTitle();

	//TES 5/6/2010 - PLID 38551 - Moved some of the RefreshDetailWindow() code into its own function.
	void RefreshButtons();

	//TES 2/21/2008 - PLID 28827 - Stored calculation for the preferred size for items (such as text and slider items)
	// that don't really care what size they are.
	CSize m_szIdealPopupWndSize;

	//TES 2/21/2008 - PLID 28827 - Determine the maximum size for the popupwnd that will keep the entire dialog on screen.
	CSize CalcMaxSize();

	//TES 2/21/2008 - PLID 28827 - Remember whether or not we're showing the detail list.
	bool m_bShowList;

	//TES 2/21/2008 - PLID 28827 - Store the last location of a popupwnd.
	CRect m_rcPopupWnd;

	//TES 3/11/2008 - PLID 28827 - Store whether or not we need to reposition the Back To EMN screen.
	// (a.walling 2010-01-14 16:09) - PLID 31118 - Deprecated
	//bool m_bNeedRepositionBackToEmn;
protected:
	// (z.manning 2008-11-04 09:37) - PLID 31904 - Added OnEmrMinimizePic()
	// Generated message map functions
	//{{AFX_MSG(CEMRItemAdvMultiPopupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedDetailList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnNextDetail();
	afx_msg void OnPreviousDetail();
	afx_msg LRESULT OnPostStateChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBackToEmn();
	afx_msg void OnSelChangingDetailList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnLeftClickDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg LRESULT OnPopupResized(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowList();
	LRESULT OnEmrMinimizePic(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVMULTIPOPUPDLG_H__493FF8A7_87AC_4F4A_9996_A21B2806B54A__INCLUDED_)
