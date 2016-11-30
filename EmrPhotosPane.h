#pragma once

#include "PhotoViewerCtrl.h"
#include "EmrPane.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes
// (a.walling 2011-10-20 11:21) - PLID 46069 - Photos pane

//(e.lally 2012-04-30) PLID 49639
class CPhotosPaneToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


class CEmrPhotosPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEmrPhotosPane)
public:
	//(e.lally 2012-04-17) PLID 49636 - 
	struct SubTabValue {
		CString strTabName; //Displayed text on the tab
		long nValueID; //ID of the category/procedure/etc we turned into tabs

		SubTabValue() {nValueID = -1;}
	};

	CEmrPhotosPane()
	{
		m_bCanViewPhotos = false; //(e.lally 2012-04-25) PLID 49967
	};

	void SetPersonID(long nPersonID);
	// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
	void SetPicContainer(class CPicContainerDlg* pPicContainer);

	//(e.lally 2012-04-12) PLID 49566
	void GetCategoryFilter(OUT CArray<long>&aryFilter);
	void SetCategoryFilter(long nCategoryID);

	//(e.lally 2012-04-12) PLID 49566
	void GetProcedureFilter(OUT CArray<long>&aryFilter);
	void SetProcedureFilter(long nProcedureID);

	//(e.lally 2012-04-13) PLID 49634
	void SetPrimarySort(UINT nSortCmd, BOOL bSortAscending);

	//(e.lally 2012-04-17) PLID 49636
	void CreateSubTabs(CArray<CEmrPhotosPane::SubTabValue>& arySubTabs);
	//(e.lally 2012-04-17) PLID 49636
	void RemoveAllSubTabs();

	//(e.lally 2012-04-17) PLID 49636
	void SetGroupByType(UINT uType);

	//(e.lally 2012-04-17) PLID 49636
	void SetFilterByName(const CString& str);

	//(e.lally 2012-04-24) PLID 49637
	void ShowTextLabels(bool bShowLabels);

	// (j.dinatale 2012-06-29 13:07) - PLID 51282
	void ReloadPhotos();

protected:
	CPhotoViewerCtrl m_PhotoViewer;
	CMFCTabCtrl m_SubTabs; //(e.lally 2012-04-17) PLID 49636
	CPhotosPaneToolBar m_wndToolBar; //(e.lally 2012-04-30) PLID 49639

	//(e.lally 2012-04-12) PLID 49566
	CArray<long> m_aryCategoryFilter;
	CArray<long> m_aryProcedureFilter;
	//(e.lally 2012-04-17) PLID 49636 - Current type of grouping
	UINT m_uGroupByType;
	// (j.jones 2012-07-12 17:56) - PLID 49636 - used to store the current tab (we only filter by name)
	BOOL m_bCacheSelectedTab;
	CString m_strSelectedTabName;
	//(e.lally 2012-04-17) PLID 49636 - holds all the tabs names and their IDs so we can look up the ID
	CMap<CString, LPCTSTR, long, long> m_mapTabValues; 
	bool m_bCanViewPhotos; //(e.lally 2012-04-25) PLID 49967

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		return TRUE;
	}

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg LRESULT OnChangeActiveTab(WPARAM wp, LPARAM lp);

	//(e.lally 2012-04-30) PLID 49639
	afx_msg void OnUpdatePhotoViewSingle(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePhotoViewSideBySide(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePhotoViewTopBottom(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePhotoViewFourGrid(CCmdUI* pCmdUI);
	// (j.jones 2013-06-19 11:18) - PLID 57207 - added ability to select all
	afx_msg void OnUpdatePhotoViewSelectAll(CCmdUI* pCmdUI);

	//(e.lally 2012-04-30) PLID 49639
	afx_msg void OnPhotoViewSingle();
	afx_msg void OnPhotoViewSideBySide();
	afx_msg void OnPhotoViewTopBottom();
	afx_msg void OnPhotoViewFourGrid();
	// (j.jones 2013-06-19 11:18) - PLID 57207 - added ability to select all
	afx_msg void OnPhotoViewSelectAll();

	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

	//(e.lally 2012-04-12) PLID 49566
	void RefreshFilters();
	//(e.lally 2012-04-17) PLID 49636
	CRect GetControlRect(UINT nID);
	//(e.lally 2012-04-17) PLID 49636
	bool HasSubTabs();

	// (j.jones 2013-06-18 16:58) - PLID 57207 - added context menu
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
};
