#include "stdafx.h"
#include "EmrPhotosPane.h"
#include "foreach.h"
#include "EmrFrameWnd.h"

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes
// (a.walling 2011-10-20 11:21) - PLID 46069 - Photos pane

IMPLEMENT_DYNCREATE(CEmrPhotosPane, CEmrPane)

BEGIN_MESSAGE_MAP(CEmrPhotosPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_VIEW_SINGLE, &CEmrPhotosPane::OnUpdatePhotoViewSingle)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_VIEW_SIDEBYSIDE, &CEmrPhotosPane::OnUpdatePhotoViewSideBySide)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_VIEW_TOPBOTTOM, &CEmrPhotosPane::OnUpdatePhotoViewTopBottom)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_VIEW_FOURGRID, &CEmrPhotosPane::OnUpdatePhotoViewFourGrid)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_SELECT_ALL, &CEmrPhotosPane::OnUpdatePhotoViewSelectAll)

	ON_COMMAND(ID_PHOTO_VIEW_SINGLE, &CEmrPhotosPane::OnPhotoViewSingle)
	ON_COMMAND(ID_PHOTO_VIEW_SIDEBYSIDE, &CEmrPhotosPane::OnPhotoViewSideBySide)
	ON_COMMAND(ID_PHOTO_VIEW_TOPBOTTOM, &CEmrPhotosPane::OnPhotoViewTopBottom)
	ON_COMMAND(ID_PHOTO_VIEW_FOURGRID, &CEmrPhotosPane::OnPhotoViewFourGrid)
	ON_COMMAND(ID_PHOTO_SELECT_ALL, &CEmrPhotosPane::OnPhotoViewSelectAll)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrPhotosPane, CEmrPane)
END_EVENTSINK_MAP()

//(e.lally 2012-04-17) PLID 49636 - Create static IDs that we can reference
#define IDC_EMR_PHOTO_VIEWER		100
#define IDC_EMR_PHOTO_SUBTABS		101

//(e.lally 2012-04-30) PLID 49639 - We can leave these always enabled
void CEmrPhotosPane::OnUpdatePhotoViewSingle(CCmdUI* pCmdUI){		pCmdUI->Enable(TRUE); }
void CEmrPhotosPane::OnUpdatePhotoViewSideBySide(CCmdUI* pCmdUI){	pCmdUI->Enable(TRUE); }
void CEmrPhotosPane::OnUpdatePhotoViewTopBottom(CCmdUI* pCmdUI){	pCmdUI->Enable(TRUE); }
void CEmrPhotosPane::OnUpdatePhotoViewFourGrid(CCmdUI* pCmdUI){		pCmdUI->Enable(TRUE); }
// (j.jones 2013-06-19 11:18) - PLID 57207 - added ability to select all
void CEmrPhotosPane::OnUpdatePhotoViewSelectAll(CCmdUI* pCmdUI){	pCmdUI->Enable(TRUE); }

int CEmrPhotosPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		m_uGroupByType = 0;
		m_bCacheSelectedTab = TRUE;
		m_strSelectedTabName = "";
		//(e.lally 2012-04-17) PLID 49636 - Create our MFC tab control
		CRect rcSubTabs = GetControlRect(IDC_EMR_PHOTO_SUBTABS);
		m_SubTabs.Create(CMFCTabCtrl::STYLE_3D_SCROLLED, rcSubTabs, this, IDC_EMR_PHOTO_SUBTABS, CMFCBaseTabCtrl::LOCATION_TOP, FALSE);

		m_PhotoViewer.m_bShowPreviewArea = false;
		//(e.lally 2012-04-30) PLID 49639 - We allow multi-selects now
		m_PhotoViewer.m_bAllowMultiSelect = true;
		m_PhotoViewer.m_bShowMirror = true;
		m_PhotoViewer.m_bShowUnited = false;

		//m_PhotoViewer.m_strImagePath = GetPatientDocumentPath(m_nPatientID, TRUE);
		//m_PhotoViewer.m_strImagePath = GetSharedPath() ^ "Images";

		//(e.lally 2012-04-17) PLID 49636 - Make the photo viewer's parent the tab control
		CRect rcPhotos = GetControlRect(IDC_EMR_PHOTO_VIEWER);
		CWnd * pWnd = (CWnd *)&m_SubTabs;
		m_PhotoViewer.Create(NULL, "", WS_VISIBLE|WS_CHILD, rcPhotos, pWnd, IDC_EMR_PHOTO_VIEWER);
		m_PhotoViewer.PrepareWindow();

		//(e.lally 2012-04-25) PLID 49967 - Check permissions
		if(!(GetCurrentUserPermissions(bioPatientHistory) & (sptDynamic1))) {
			//No permission to view photos
			m_bCanViewPhotos = false;
			m_PhotoViewer.SetAdditionalFilter("(1=0) ");
		}
		else {
			m_bCanViewPhotos = true;
			//(e.lally 2012-04-25) PLID 49967 - Make sure the pictures shown on the photo viewer are marked as photos
			m_PhotoViewer.SetAdditionalFilter("(MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) ");
		}

		m_SubTabs.EnableActiveTabCloseButton(FALSE);
		m_SubTabs.EnableTabDocumentsMenu(TRUE);
		m_SubTabs.SetDrawFrame(FALSE);
		m_SubTabs.AutoDestroyWindow(FALSE);

		//(e.lally 2012-04-30) PLID 49639 - Create a toolbar for the photo preview options
		m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EMR_PHOTO_PANE_TOOLBAR);
		m_wndToolBar.LoadToolBar(IDR_EMR_PHOTO_PANE_TOOLBAR, 0, 0, TRUE /* Is locked */);

		//OnChangeVisualStyle();
		{
			m_wndToolBar.CleanUpLockedImages();
			m_wndToolBar.LoadBitmap(IDR_EMR_PHOTO_PANE_TOOLBAR, 0, 0, TRUE /* Locked */);

		}

		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

		m_wndToolBar.SetOwner(this);

		// All commands will be routed via this control , not via the parent frame:
		m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

		//(e.lally 2012-04-30) PLID 49636 - We can now apply the last group by that was selected
		GetTopLevelFrame()->PostMessage(WM_COMMAND, MAKEWPARAM(ID_EMR_PHOTO_GROUP, 0), 0);

		//(e.lally 2012-04-30) PLID 49634 - We can now apply the last sort order
		GetTopLevelFrame()->PostMessage(WM_COMMAND, MAKEWPARAM(ID_EMR_PHOTO_SORT, 0), 0);

		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}

//(e.lally 2012-04-17) PLID 49636 - Moved to cpp
void CEmrPhotosPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	try {
		//(e.lally 2012-04-30) PLID 49639
		if(m_wndToolBar.GetSafeHwnd()){
			m_wndToolBar.MoveWindow(GetControlRect(IDR_EMR_PHOTO_PANE_TOOLBAR));
		}
		if(m_SubTabs.GetSafeHwnd()){
			m_SubTabs.MoveWindow(GetControlRect(IDC_EMR_PHOTO_SUBTABS));
		}
		if (m_PhotoViewer.GetSafeHwnd()) {
			m_PhotoViewer.MoveWindow(GetControlRect(IDC_EMR_PHOTO_VIEWER));
		}
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-17) PLID 49636 - Gets the rect for which to draw the given control in
CRect CEmrPhotosPane::GetControlRect(UINT nID)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcControl(rcClient);

	//(e.lally 2012-04-17) PLID 49636 - I thought I could stretch the photo viewer control up to use the whole client area when
	//	there are no tabs present, but the left/right scroll arrow had a drawing conflict with the photo viewer scrollbar
	//	and modifying the style of the tab ctl wouldn't add/remove those buttons like I expected.
	//(e.lally 2012-04-30) PLID 49639 - Added toolbar and tweaked the others slightly to accommodate
	switch(nID){
		case IDC_EMR_PHOTO_SUBTABS:
			//Tabs
			rcControl = CRect(rcClient.left, rcClient.top + 23, rcClient.right, rcClient.bottom);
			break;
		case IDC_EMR_PHOTO_VIEWER:
			//Photos
			rcControl = CRect(rcClient.left, rcClient.top + 22, rcClient.right, rcClient.bottom);
			break;
		case IDR_EMR_PHOTO_PANE_TOOLBAR:
		{
			int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
			rcControl = CRect(rcClient.left, rcClient.top, rcClient.right, cyTlb);
			break;
		}

	}
	return rcControl;
}
		
void CEmrPhotosPane::SetPersonID(long nPersonID)
{
	m_PhotoViewer.SetPersonID(nPersonID);
}

// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
void CEmrPhotosPane::SetPicContainer(class CPicContainerDlg* pPicContainer)
{
	m_PhotoViewer.SetPicContainer(pPicContainer);
}

//(e.lally 2012-04-12) PLID 49566 - Gets the category ID filter array
void CEmrPhotosPane::GetCategoryFilter(CArray<long>& aryOut)
{
	aryOut.Copy(m_aryCategoryFilter);
}

//(e.lally 2012-04-12) PLID 49566 - Sets the category filter
void CEmrPhotosPane::SetCategoryFilter(long nCategoryID)
{
	m_aryCategoryFilter.RemoveAll();
	m_aryCategoryFilter.Add(nCategoryID);

	RefreshFilters();
}

//(e.lally 2012-04-12) PLID 49566 - Gets the procedure ID filter array
void CEmrPhotosPane::GetProcedureFilter(CArray<long>& aryOut)
{
	aryOut.Copy(m_aryProcedureFilter);
}

//(e.lally 2012-04-12) PLID 49566 - Sets the procedure filter
void CEmrPhotosPane::SetProcedureFilter(long nProcedureID)
{
	m_aryProcedureFilter.RemoveAll();
	m_aryProcedureFilter.Add(nProcedureID);

	RefreshFilters();
}

//(e.lally 2012-04-12) PLID 49566 - Refreshes the photos based on the current filter values
void CEmrPhotosPane::RefreshFilters()
{
	//(e.lally 2012-04-25) PLID 49967 - Make sure the pictures shown on the photo viewer are marked as photos
	CString strPhotoWhere = "(MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) ";

	//(e.lally 2012-04-25) PLID 49967 - Check permissions
	if(!m_bCanViewPhotos) {
		//No permission to view photos
		strPhotoWhere = "(1=0) ";
	}
	else {
		CString strTemp;
		bool bUsingCategoryFilter = false;
		foreach (const long& nCategoryID, m_aryCategoryFilter) {
			switch(nCategoryID){
				case ID_COMBO_FILTER_NO_KEY: //No Category
					strTemp += "MailSent.CategoryID IS NULL ";
					break;
				case ID_COMBO_FILTER_ALL:
					strTemp += "";
					break;
				default:
					strTemp += FormatString("MailSent.CategoryID = %li ", nCategoryID);
					break;
			}
			if(!strTemp.IsEmpty()){
				//(e.lally 2012-04-25) PLID 49636
				bUsingCategoryFilter = true;
				if(strPhotoWhere.IsEmpty()){
					strPhotoWhere += strTemp;
				}
				else {
					strPhotoWhere += " AND (" + strTemp + ") ";
				}
			}
		}
		//(e.lally 2012-04-25) PLID 49636 - Flag when the category filter is in use 
		//	for proper self refreshing upon category changes to a photo
		m_PhotoViewer.SetUsingCategoryFilter(bUsingCategoryFilter);


		strTemp = "";
		foreach (const long& nProcedureID, m_aryProcedureFilter) {
			switch(nProcedureID){ 
				case ID_COMBO_FILTER_NO_KEY: //No Procedure
					strTemp += "MailSent.MailID NOT IN(SELECT MailSentProcedureT.MailSentID FROM MailSentProcedureT WHERE MailSentProcedureT.MailSentID = MailSent.MailID) ";
					break;
				case ID_COMBO_FILTER_ALL:
					strTemp += "";
					break;
				default:
					strTemp += FormatString("MailSent.MailID IN(SELECT MailSentProcedureT.MailSentID FROM MailSentProcedureT WHERE MailSentProcedureT.MailSentID = MailSent.MailID AND MailSentProcedureT.ProcedureID = %li) ", nProcedureID);
					break;
			}

			if(!strTemp.IsEmpty()){
				if(strPhotoWhere.IsEmpty()){
					strPhotoWhere += strTemp;
				}
				else {
					strPhotoWhere += " AND (" + strTemp + ") ";
				}
			}
		}
	}

	//Setting the additional filter fires a refresh for us
	m_PhotoViewer.SetAdditionalFilter(strPhotoWhere);
}

//(e.lally 2012-04-13) PLID 49634
void CEmrPhotosPane::SetPrimarySort(UINT nSortCmd, BOOL bSortAscending)
{
	CPhotoViewerCtrl::EDisplaySortCriteria edscCriterion = CPhotoViewerCtrl::dscAttachDate;
	switch(nSortCmd){
		case ID_EMR_PHOTO_SORT_CATEGORY:
			edscCriterion = CPhotoViewerCtrl::dscCategory;
			break;
		case ID_EMR_PHOTO_SORT_ATTACHDATE:
			edscCriterion = CPhotoViewerCtrl::dscAttachDate;
			break;
		case ID_EMR_PHOTO_SORT_PROCEDURE:
			edscCriterion = CPhotoViewerCtrl::dscProcedureName;
			break;
		case ID_EMR_PHOTO_SORT_STAFF:
			edscCriterion = CPhotoViewerCtrl::dscStaff;
			break;
		case ID_EMR_PHOTO_SORT_NOTE:
			edscCriterion = CPhotoViewerCtrl::dscNote;
			break;
		//(e.lally 2012-04-16) PLID 39543 - Added service date
		case ID_EMR_PHOTO_SORT_SERVICEDATE:
			edscCriterion = CPhotoViewerCtrl::dscServiceDate;
			break;
	}

	//(e.lally 2012-04-25) PLID 49634 - Remember last sort order
	SetRemotePropertyInt("PhotoViewerSortCriteria", (long)edscCriterion, 0, GetCurrentUserName());
	
	if(m_PhotoViewer.GetSafeHwnd()){
		if(m_PhotoViewer.SetPrimarySort(edscCriterion, bSortAscending)){
			m_PhotoViewer.Invalidate();
		}
	}
}

//(e.lally 2012-04-17) PLID 49636 - Do we have any subtabs
bool CEmrPhotosPane::HasSubTabs()
{
	return m_SubTabs.GetTabsNum() > 0 ? true : false;
}

//(e.lally 2012-04-17) PLID 49636 - Removes any existing subtabs and creates new ones based on the passed in list.
void CEmrPhotosPane::CreateSubTabs(CArray<CEmrPhotosPane::SubTabValue>& arySubTabs)
{
	if(HasSubTabs()){
		m_SubTabs.RemoveAllTabs();
	}
	m_mapTabValues.RemoveAll();

	long nSelectTabIndex = -1;

	// (j.jones 2012-07-12 17:59) - PLID 49636 - temporarily disable caching tabs upon selection,
	// because if we cleared all tabs, the first tab we add below will auto-select, which would
	// otherwise cause our cached tab to always be replaced with the first tab
	m_bCacheSelectedTab = FALSE;

	for(int i=0; i< arySubTabs.GetSize(); i ++){
		// (j.jones 2012-06-26 11:31) - PLID 49636 - Replace & with && so we don't convert to an accelerator.
		// This has to be the same value in both m_mapTabValues and m_SubTabs because the tab changing code
		// looks up content by tab name.
		CString strTabName = arySubTabs.GetAt(i).strTabName;
		strTabName.Replace("&","&&");
		m_mapTabValues.SetAt(strTabName, arySubTabs.GetAt(i).nValueID);
		m_SubTabs.AddTab(&m_PhotoViewer, strTabName, (UINT) -1, FALSE);

		// (j.jones 2012-07-12 17:59) - PLID 49636 - re-select the previously selected tab
		if(!m_strSelectedTabName.IsEmpty() && m_strSelectedTabName == strTabName) {
			//track this tab index, we will select it later
			nSelectTabIndex = i;
		}
	}

	// (j.jones 2012-07-12 17:59) - PLID 49636 - re-enable caching tabs upon selection
	m_bCacheSelectedTab = TRUE;

	if(arySubTabs.GetCount() > 0){
		m_SubTabs.Invalidate();
		m_PhotoViewer.Invalidate();
	}
	else {
		// (j.jones 2012-07-12 17:59) - PLID 49636 - if we have no tabs, clear the previously selected tab name
		m_strSelectedTabName = "";

		//(e.lally 2012-04-30) PLID 49636 - Force the where clause to refresh
		RefreshFilters();
	}

	// (j.jones 2012-07-12 17:59) - PLID 49636 - re-select the previously selected tab
	if(nSelectTabIndex != -1) {
		m_SubTabs.SetActiveTab(nSelectTabIndex);
	}

	//(e.lally 2012-04-17) PLID 49636 - The order of operations can cause weird results when we remove and re-add tabs.
	//	This can leave us with a hidden photo viewer window, so ensure it is showing here.
	m_PhotoViewer.ShowWindow(SW_SHOWNA);
}

//(e.lally 2012-04-17) PLID 49636 - Removes all the subtabs
void CEmrPhotosPane::RemoveAllSubTabs()
{
	m_SubTabs.RemoveAllTabs();
	m_SubTabs.Invalidate();
	m_PhotoViewer.Invalidate();
	SetGroupByType(0);
	RefreshFilters();
}

//(e.lally 2012-04-17) PLID 49636 - Sets the current group by type
void CEmrPhotosPane::SetGroupByType(UINT uNewGroupByType)
{
	switch(m_uGroupByType){
		case ID_EMR_PHOTO_GROUP_PROCEDURE:
			m_aryProcedureFilter.RemoveAll();
			break;
		case ID_EMR_PHOTO_GROUP_CATEGORY:
			m_aryCategoryFilter.RemoveAll();
			break;
	}

	//see if our grouping changed
	if(m_uGroupByType != uNewGroupByType) {
		// (j.jones 2012-07-12 17:56) - PLID 49636 - if our grouping has changed, reset the tab filter
		m_strSelectedTabName = "";

		//(e.lally 2012-04-30) PLID 49636 - remember the selected value
		SetRemotePropertyInt("EmrPhotoTabType", (long)uNewGroupByType, 0, GetCurrentUserName());
	}

	m_uGroupByType = uNewGroupByType;

	switch(uNewGroupByType){
		case ID_EMR_PHOTO_GROUP_PROCEDURE:
			m_aryProcedureFilter.RemoveAll();
			break;
		case ID_EMR_PHOTO_GROUP_CATEGORY:
			m_aryCategoryFilter.RemoveAll();
			break;
	}
}

//(e.lally 2012-04-17) PLID 49636 - Takes a tab name and filters based on the current group by type
void CEmrPhotosPane::SetFilterByName(const CString& str)
{
	// (j.jones 2012-07-12 17:54) - PLID 49636 - Cache this tab filter, only if caching is enabled.
	// It would be disabled if we were in the process of clearing & re-adding tabs, because adding tabs
	// tries to auto-select by default.
	if(m_bCacheSelectedTab) {
		m_strSelectedTabName = str;
	}

	long nValueID = -99;  //sentinel that should not be in use or conflict with other sentinels
	if(m_mapTabValues.Lookup(str, nValueID)){
		ASSERT(nValueID != -99);
		switch(m_uGroupByType){
			case ID_EMR_PHOTO_GROUP_PROCEDURE:
				SetProcedureFilter(nValueID);
				break;
			case ID_EMR_PHOTO_GROUP_CATEGORY:
				SetCategoryFilter(nValueID);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	else {
		ASSERT(FALSE);
	}
}

//(e.lally 2012-04-17) PLID 49636 - Handles the Change Active Tab message
//	re-filters based on the tab label and grouping type
LRESULT CEmrPhotosPane::OnChangeActiveTab(WPARAM wp, LPARAM /*lp*/)
{
	try {
		//This fires when all tabs are removed too, so just process when we have tabs
		if(HasSubTabs()){
			CString strLabel;
			m_SubTabs.GetTabLabel((int) wp, strLabel);

			SetFilterByName(strLabel);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

//(e.lally 2012-04-24) PLID 49637 - Shows/Hides the text label (caption) under the photos
void CEmrPhotosPane::ShowTextLabels(bool bShowLabels)
{
	m_PhotoViewer.ShowTextLabels(bShowLabels);
	m_PhotoViewer.Invalidate();
}

//(e.lally 2012-04-30) PLID 49639
void CEmrPhotosPane::OnPhotoViewSingle()
{
	try {
		if(m_PhotoViewer.GetSafeHwnd()){
			m_PhotoViewer.PreviewSelectedPhotos(CPhotoViewerCtrl::daPreview1);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-30) PLID 49639
void CEmrPhotosPane::OnPhotoViewSideBySide()
{
	try {
		if(m_PhotoViewer.GetSafeHwnd()){
			m_PhotoViewer.PreviewSelectedPhotos(CPhotoViewerCtrl::daPreviewSideSide);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-30) PLID 49639
void CEmrPhotosPane::OnPhotoViewTopBottom()
{
	try {
		if(m_PhotoViewer.GetSafeHwnd()){
			m_PhotoViewer.PreviewSelectedPhotos(CPhotoViewerCtrl::daPreviewTopBottom);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-30) PLID 49639
void CEmrPhotosPane::OnPhotoViewFourGrid()
{
	try {
		if(m_PhotoViewer.GetSafeHwnd()){
			m_PhotoViewer.PreviewSelectedPhotos(CPhotoViewerCtrl::daPreview4);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-19 11:18) - PLID 57207 - added ability to select all
void CEmrPhotosPane::OnPhotoViewSelectAll()
{
	try {
		if(m_PhotoViewer.GetSafeHwnd()){
			//this will select all currently filtered images on the selected tab
			m_PhotoViewer.SelectAllDisplayedImages();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-06-29 13:07) - PLID 51282
void CEmrPhotosPane::ReloadPhotos()
{
	if(m_PhotoViewer.GetSafeHwnd()){
		m_PhotoViewer.Refresh();
	}
}

// (j.jones 2013-06-18 16:58) - PLID 57207 - added context menu for when an image isn't clicked on,
// this will come up when right clicking whitespace
void CEmrPhotosPane::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	try {

		CNxMenu mnu;
		mnu.CreatePopupMenu();

		enum {
			miSelectAllImages = -100,
		};

		mnu.AppendMenu(MF_BYPOSITION, miSelectAllImages, "Select &All Images");

		long nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_VERPOSANIMATION,
			pos.x, pos.y, this, NULL);
		switch(nSelection) {
			// (j.jones 2013-06-18 17:08) - PLID 57207 - added ability to select all images
			case miSelectAllImages:
				//this will select all currently filtered images on the selected tab
				m_PhotoViewer.SelectAllDisplayedImages();
				break;
			default:
				break;
		}

	}NxCatchAll(__FUNCTION__);
}