#if !defined(AFX_TEMPLATEITEMENTRYGRAPHICALDLG_H__56C9FA61_B29D_4470_BC51_FAF0AD1C1AE0__INCLUDED_)
#define AFX_TEMPLATEITEMENTRYGRAPHICALDLG_H__56C9FA61_B29D_4470_BC51_FAF0AD1C1AE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateItemEntryGraphicalDlg.h : header file
//

// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"
#include "NxAPI.h"
#include "TemplateLineItemInfo.h"

class CTemplatesDlg;
class CLocationOfficeHours;
class CTemplateSelectDlg;

// (z.manning 2014-12-01 14:54) - PLID 64205
enum ESchedulerTemplateEditorType
{
	stetNormal,
	stetLocation,
	stetCollection,
};

enum EClipReservationType
{
	crtNone = 0,
	crtCut = 1,
	crtCopy = 2,
};

struct TemplateItemReservationInfo
{
	long m_nID;
	COleDateTime m_dtStartTime;
	COleDateTime m_dtEndTime;
	long m_nPriority;
	CString m_strText;
	OLE_COLOR m_dwColor;

	TemplateItemReservationInfo() {}

	TemplateItemReservationInfo(long nID, COleDateTime dtStartTime, COleDateTime dtEndTime, long nPriority, CString strText, OLE_COLOR dwColor)
	{
		m_nID = nID;
		m_dtStartTime = dtStartTime;
		m_dtEndTime = dtEndTime;
		m_nPriority = nPriority;
		m_strText = strText;
		m_dwColor = dwColor;
	}
};

// (j.jones 2010-12-09 12:42) - PLID 41782 - added struct to cache all start/endtime info per resource
struct ResourceStartEndTime {

	long nResourceID;
	COleDateTime dtStartTime;
	COleDateTime dtEndTime;
};

// (z.manning 2015-01-15 09:45) - PLID 64210 - Background color when editing template collections
extern CBrush g_brTemplateCollectionBackground;

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemEntryGraphicalDlg dialog

// (z.manning, 11/02/2006) - PLID 7555 - The class for the scheduler template entry graphical dialog.
class CTemplateItemEntryGraphicalDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2014-12-01 14:54) - PLID 64204 - Added type as required param to the constructor
	CTemplateItemEntryGraphicalDlg(ESchedulerTemplateEditorType type, CWnd* pParent);   // standard constructor
	~CTemplateItemEntryGraphicalDlg();

	void RefreshEverything();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void UpdateTemplateReservations();
	void UpdateDayOfWeekText();

	COleDateTime GetActiveDate();
	long GetActiveResourceID();	
	CString GetActiveResourceName(); // (z.manning 2011-12-07 11:32) - PLID 46910
	void SetActiveDate(const COleDateTime dt); // (z.manning 2009-07-10 10:54) - PLID 22054

	// (c.haag 2015-01-02) - PLID 64257 - Returns true if a collection is used in the current view
	bool IsCollectionUsed(long nID);
	// (c.haag 2014-12-16) - PLID 64255 - Update the Used Collections label based on the contents of m_UsedCollections
	void UpdateUsedCollectionsLabel();

	CTemplateLineItemInfo* FindLineItemInArray(CArray<CTemplateLineItemInfo*, CTemplateLineItemInfo*> &arypLineItems, long nTemplateID, long nLineitemID);
	void LoadResourceList();
	void SetDefaultResourceID(long nResourceID);
	// (j.jones 2011-07-15 14:38) - PLID 39838 - set a default date
	void SetDefaultDate(const COleDateTime dt);
	// (c.haag 2014-12-15) - PLID 64246 - Set a default collection ID
	void SetDefaultCollectionID(_bstr_t bstrCollectionID);

	// (c.haag 2010-04-30 12:30) - PLID 38379 - We now take in an owner
	CReservation AddReservation(const CString& strOwner, COleDateTime dtStart, COleDateTime dtEnd, long nID, CString strText, OLE_COLOR dwColor, BOOL bIsBlock);

	// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
	void DeleteLineItem(LPDISPATCH theRes);
	void PasteLineItem(COleDateTime dtNewStart);
	void CopyEntireDay();// (a.vengrofski 2009-12-30 12:26) - PLID <28394> - Copy an entire days worth of templates.
	void PasteEntireDay();// (a.vengrofski 2009-12-30 12:26) - PLID <28394> - Paste an entire days worth of templates.

	// (c.haag 2014-12-12) - PLID 64221 - Non-Collections editor version of DeleteLineItem
	void DeleteTemplateLineItem(LPDISPATCH theRes);
	// (c.haag 2014-12-12) - PLID 64221 - Collections editor version of DeleteLineItem
	void DeleteCollectionTemplate(LPDISPATCH theRes);

	void PutFocusOnSingleDay();

	// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
	void SetClippedReservation(LPDISPATCH theRes, EClipReservationType eType);

	// (z.manning, 11/27/2006) - PLID 23665 - Returns the slot number and the time of the slot where the mouse is currently positioned.
	int GetCurrentSlot();
	COleDateTime GetCurrentSlotTime();

	// (z.manning 2009-07-10 10:51) - PLID 22054 - Added an option to have this dialog be invisible
	BOOL m_bVisible;

	// (z.manning 2009-07-10 11:03) - PLID 22054 - Option to load all resources at once.
	BOOL m_bLoadAllResources;

	// (z.manning 2009-07-10 11:06) - PLID 22054 - Populates the array with all loaded resource IDs
	void GetAllLoadedResourceIDs(OUT CArray<long,long> &arynResourceIDs);

	// (z.manning 2009-07-10 12:13) - PLID 22054 - Gets the total minutes for the given resource for
	// the active date.
	long GetTotalMinutesByResourceID(const long nResourceID);
	void ClearTotalMinuteMap();

	// (j.jones 2010-12-09 12:27) - PLID 41782 - added ability to get an array of all start/end times
	// for templates for a given resource ID
	void GetStartEndTimeArrayForResourceID(IN const long nResourceID, OUT CArray<ResourceStartEndTime, ResourceStartEndTime> &aryStartEndTimes);
	void ClearStartEndTimeArray();
	// (s.tullis 2015-08-24 14:37) - PLID 66411- Renamed
	// (z.manning 2009-07-18 13:35) - PLID 22054
	void SetTemplateIDFilter(CArray<long,long> &arynTemplateIDs);

	// (a.vengrofski 2009-12-30 12:20) - PLID <28394>
	BOOL m_bCopyChanged;
	long m_lResID;
	
	// (a.vengrofski 2010-03-18 10:39) - PLID <28394> - This will copy the source to the current selected resource.
	void PasteSingleItem(CTemplateLineItemInfo* pSourceLineItem, COleDateTime dtNewStart, BOOL bNoRefresh = FALSE);

	// (z.manning 2014-12-04 10:01) - PLID 64215
	void LoadTemplateCollection(NexTech_Accessor::_SchedulerTemplateCollectionPtr pCollection);

	// (c.haag 2014-12-16) - PLID 64255 - Collections that are used by template items for the current filter.
	// This only applies to the non-collections editor style.
	struct UsedCollection
	{
		long ID;
		CString Name;
	};
	CArray<UsedCollection, UsedCollection&> m_UsedCollections;

// Dialog Data
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use CDateTimePicker
	// (c.haag 2014-12-15) - PLID 64246 - Added m_btnCreateCollectionFromView
	// (c.haag 2014-12-16) - PLID 64255 - Added m_nxlCollectionsUsed
	// (c.haag 2014-12-17) - PLID 64253 - Added m_btnRemoveTemplates
	//{{AFX_DATA(CTemplateItemEntryGraphicalDlg)
	enum { IDD = IDD_TEMPLATE_ITEM_ENTRY_GRAPHICAL };
	CNxIconButton	m_btnMoveDayBack;
	CNxIconButton	m_btnMoveDayForward;
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	CNxLabel	m_btnToday;
	CNxLabel	m_btnAmPm;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnOptions;
	CNxIconButton	m_btnCreateCollectionFromView;
	CNxIconButton	m_btnHelp;
	CDateTimePicker	m_dtpActiveDate;
	CNxStatic	m_nxstaticDayOfWeek;
	NxButton	m_btnTemplateListPlaceholder;
	CNxLabel		m_nxlCollectionsUsed;
	CNxIconButton	m_btnRemoveTemplates;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateItemEntryGraphicalDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CTemplatesDlg *m_pdlgTemplateList;

	CSingleDay m_pSingleDayCtrl;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlResourceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlInterval;

	// (z.manning 2014-12-03 10:32) - PLID 64210
	ESchedulerTemplateEditorType m_eEditorType;

	//TES 6/19/2010 - PLID 5888
	bool m_bUseResourceAvailTemplates;
	EBuiltInObjectIDs m_bio;

	BOOL m_bIsDateSelComboDroppedDown;
	BOOL m_bGoToAM;
	BOOL m_bResourceListLoaded;
	long m_nDefaultResourceID;
	// (j.jones 2011-07-15 14:38) - PLID 39838 - set a default date
	COleDateTime m_dtDefaultDate;
	// (c.haag 2014-12-15) - PLID 64246 - Default collection ID
	_bstr_t m_bstrDefaultCollectionID;

	CLocationOfficeHours &m_lohOfficeHours;

	COleDateTime m_dtResPreMoveStartTime;
	COleDateTime m_dtResPreMoveEndTime;
	BOOL m_bResMoving;

	CMap<long, long, long, long> m_mapLineItemIDToPriority;
	CMap<long, long, BOOL, BOOL> m_mapLineItemIDToIsBlock;
	CMap<long, long, BOOL, BOOL> m_mapLineItemIDToFromCollection; // (c.haag 2014-12-17) - PLID 64294

	CTemplateLineItemInfo* m_pClippedLineItem;
	CTemplateLineItemInfo* m_pCopyAllLineItem;// (a.vengrofski 2009-12-31 18:16) - PLID <28394>
	EClipReservationType m_eLastClipType;
	// (z.manning 2014-12-18 08:44) - PLID 64225 - Variable to keep track of cut/copied collection template
	NexTech_Accessor::_SchedulerTemplateCollectionTemplatePtr m_pClippedCollectionTemplate;

	// (z.manning 2009-07-10 11:36) - PLID 22054 - Stores the toal templates minutes per resource
	CMap<long,long,long,long> m_mapResourceIDToTotalMinutes;

	// (j.jones 2010-12-09 12:45) - PLID 41782 - stores each start/end time for each template for each resource
	CArray<ResourceStartEndTime, ResourceStartEndTime> m_aryResourceStartEndTimes;

	// (a.vengrofski 2010-03-18 15:10) - PLID <28394> - Array of resverations to be copied.
	CArray<long> m_aryToBeCopied;

protected:
	void EnsureValidEndTime(COleDateTime &dtEndTime);

	// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
	void PopupContextMenu(LPDISPATCH theRes);

	// Returns the the new reservation object if pRes is split in 2, returns NULL otherwise.
	CReservation SplitNonBlockReservation(CReservation pRes, COleDateTime dtSplitStart, COleDateTime dtSplitEnd);
	// (c.haag 2010-07-14 16:31) - PLID 39615 - We now take in an LPDISPATCH instead of a CReservation
	void SplitBlockReservation(LPDISPATCH theBlock, COleDateTime dtSplitStart, COleDateTime dtSplitEnd, int nBlockPriority);
	
	void SetResourceListSelectionAnyRow();

	void SetVisibleTimeRange(); // (z.manning, 03/13/2007) - PLID 23635

	void UpdateVisibleControls();

	// (z.manning 2014-12-17 13:15) - PLID 64427
	void CommitCollectionTemplate(const _variant_t &varCollectionTemplateID, CTemplateSelectDlg *pdlgTemplateSelect);
	void CommitCollectionTemplate(const _variant_t &varCollectionTemplateID, const long nTemplateID
		, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, BOOL bIsBlock);

	// (c.haag 2015-02-06) - PLID 64353 - Calculates which resources are affected by a call to CreateTemplateExceptionsForResourceTimeRange
	ADODB::_RecordsetPtr GetAffectedResourcesForTemplateExceptionCreation(const COleDateTime& dtFromTime, const COleDateTime& dtToTime);
	// (c.haag 2014-12-17) - PLID 64253 - Gets the warning message for removing templates in a given time range
	CString GetRemoveTemplatesWarningTextByTimeRange(const COleDateTime& dtFromTime, const COleDateTime& dtToTime, OUT BOOL& bQuestion);
	// (c.haag 2015-02-06) - PLID 64353 - Gets the warning message for removing templates in the day
	CString GetRemoveTemplatesWarningText();
	// (s.tullis 2015-08-24 14:37) - PLID 66411 - Renamed
	CArray<long,long> m_arynTemplateIDFilter;

	// Generated message map functions
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use notify event handlers
	// (c.haag 2014-12-15) - PLID 64246 - Added OnCreateCollectionFromViewBtn
	// (c.haag 2014-12-17) - PLID 64253 - Added OnRemoveTemplatesBtn
	// (c.haag 2015-01-02) - PLID 64257 - Added OnSetCursor and OnLabelClick
	//{{AFX_MSG(CTemplateItemEntryGraphicalDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseBtn();
	afx_msg void OnChangeDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnCloseUpDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnDropDownDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnMoveDayForward();
	afx_msg void OnMoveDayBack();
	afx_msg void OnTodayBtn();
	afx_msg void OnAmPmBtn();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChosenInterval(LPDISPATCH lpRow);
	afx_msg void OnReservationRightClickTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y);
	afx_msg void OnReservationClickTemplateItemEntryCtrl(LPDISPATCH theRes);
	afx_msg void OnReservationAddedTemplateItemEntryCtrl(LPDISPATCH theRes);
	afx_msg void OnSelChosenResourceList(LPDISPATCH lpRow);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnUpdateView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnReservationEndDragTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	afx_msg void OnReservationEndResizeTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	afx_msg void OnReservationDragTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	afx_msg void OnReservationResizeTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	afx_msg void OnOptionsBtn();
	afx_msg void OnCreateCollectionFromViewBtn(); 
	afx_msg void OnRemoveTemplatesBtn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseUpTemplateItemEntryCtrl(short Button, short Shift, long x, long y);
	afx_msg void OnTrySetSelFinishedResourceList(long nRowEnum, long nFlags);
	afx_msg void OnHelp();
	afx_msg void OnMouseMoveTemplateItemEntryCtrl(short Button, short Shift, long x, long y);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEITEMENTRYGRAPHICALDLG_H__56C9FA61_B29D_4470_BC51_FAF0AD1C1AE0__INCLUDED_)
