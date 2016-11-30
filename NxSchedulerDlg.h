#if !defined(AFX_NXSCHEDULERDLG_H__5BEA8D23_C898_11D1_804A_00104B2FE914__INCLUDED_)
#define AFX_NXSCHEDULERDLG_H__5BEA8D23_C898_11D1_804A_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NxSchedulerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxSchedulerDlg dialog
#include "ReservationReadSet.h"
#include "NxSchedulerMenu.h"
#include <NxNetworkLib\TableCheckerDetails.h>	// (a.wilson 2014-08-13 10:41) - PLID XXXXX
#include "client.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"

#include "CommonFFAUtils.h"

// using namespace SINGLEDAYLib;

#include <NxSystemUtilitiesLib/NxSmartPtr.h>

#define WAIT_TIME 250
#define SCROLL_WAIT_TIME 500
#define ID_MONTH_VIEW_TIMER 1383//arbitrary
#define ID_MONTH_VIEW_SCROLL_TIMER 1384


#define DECLARE_COMMAND_FORWARD_BACK(func_suffix)		\
				afx_msg void OnForward##func_suffix(); \
				afx_msg void OnBack##func_suffix();

#define ON_COMMAND_FORWARD_BACK(id_suffix, func_suffix)		\
				ON_COMMAND(ID_FORWARD_##id_suffix, OnForward##func_suffix)  \
				ON_COMMAND(ID_BACK_##id_suffix, OnBack##func_suffix)

#define IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(func_suffix, units, count) \
				void CNxSchedulerDlg::OnForward##func_suffix() { MoveCurrentDateByUnits(units, count); } \
				void CNxSchedulerDlg::OnBack##func_suffix() { MoveCurrentDateByUnits(units, -(count)); }

enum EnumNxSchedulerForwardBackUnits {
	nsfbuDays,
	nsfbuWeeks,
	nsfbuMonths, 
	nsfbuYears,
};

namespace Nx
{
	namespace Scheduler
	{
		struct TargetInfo;
	}
}


// This class is an abstract class (i.e. it contains only pure virtuals).  There are two purposes for this class:
//   1. To force anyone who wants to be a sheet of the CSchedulerView to implement certain functions
//   2. To provide a way of letting CSchedulerView call functions on the sheet while not allowing anyone else to call the functions
//  These two requirements are necessary to help avoid incorrect usage of the CNxSchedulerDlg class.  The scheduler
// is a complicated system because the various tabs of the view are not entirely independent, they each show a 
// different view of the exact same filters (i.e. what date range are we dealing with, what set of resources, etc.
// So when someone asks the scheduler view for certain information (like "what is the date represented by a certain
// column in the singleday control") the CSchedulerView class knows what it should be, but it can double-check its 
// information with the actual interface by calling the functions presented by this class.
//  NOTE: This class is not attempting to be a modular sheet of the CSchedulerView.  The only point is to provide 
// a level of code security so that people don't call these functions from inappropriate places in code.  To add 
// a sheet to the CSchedulerView that sheet must be derived from CNxSchedulerDlg, which is itself derived from this
// CSchedulerViewSheet class.
class CSchedulerViewSheet
{
	friend class CSchedulerView;

protected:
	virtual void Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const = 0;
	virtual BOOL Internal_IsMultiResourceSheet() const = 0;
	virtual BOOL Internal_IsMultiDateSheet() const = 0;

protected:
	virtual void Internal_ReflectCurrentResourceListOnGUI() = 0;
	virtual void Internal_ReflectActiveResourceOnGUI() = 0;

};

class SchedulerMixRule;

class CNxSchedulerDropTarget;

class CReservationReadSet;
class CSchedViewColumnInfo;
struct CSchedTemplateInfo;

namespace ResourceComboColumns
{
	enum Enum {
		ID,
		Relevence,
		Name,
		DefaultInterval
	};
}

// (c.haag 2010-02-04 12:17) - PLID 37221 - Now uses IReservationPtr instead of IReservation*
bool IsPatientAppointment(CReservation pRes);

// (j.jones 2014-12-11 15:07) - PLID 64178 - moved CComplexTimeRange from CommonSchedUtils.h
//TES 9/2/2010 - PLID 39630 - Class for manipulating template time ranges.  This class defines times within a single day.
// AddRange will add the given range of times to the list, combine entries as appropriate (so, if you add 10:00-10:15, 10:30-10:45,
// and then 10:15-10:30, you will end up with a range of 10:00-10:45.  RemoveRange removes from available times, splitting as necessary
// (so removing 10:15-10:30 in the previous example would leave you with 2 ranges, 10:00-10:15 and 10:30-10:45
struct TimeRange {
	COleDateTime dtStart;
	COleDateTime dtEnd;
};

class CComplexTimeRange {
public:
	void AddRange(const COleDateTime &dtStart, const COleDateTime &dtEnd);
	void RemoveRange(const COleDateTime &dtStart, const COleDateTime &dtEnd);

	COleDateTime GetStart();
	COleDateTime GetEnd();

	//TES 9/3/2010 - PLID 39630 - Get the range, as an array of contiguous ranges.
	void GetRanges(CArray<TimeRange, TimeRange&> &arOut);

	//TES 9/3/2010 - PLID 39630 - Get the number of contiguous ranges in this object.
	int GetCount();

	//TES 9/7/2010 - PLID 39630 - Remove the range that is specified by this object from the range specified by arTimes.
	// This is basically saying arTimes = arTimes & ~(this object's range).
	void RemoveFromRanges(IN OUT CArray<TimeRange, TimeRange&> &arTimes);

protected:
	CList<TimeRange, TimeRange&> m_Ranges;
};

// (c.haag 2006-05-22 11:18) - PLID 20614 - We want to keep a list of visible template items
// in memory so that we don't have to query the data to find out the name of the template
// under the mouse cursor

struct VisibleTemplateRule
{
	long nRuleID;
	BOOL bPreventOnFail; // (z.manning 2011-06-14 16:39) - PLID 41131
	BOOL bAndDetails;
	BOOL bAllAppts;
	BOOL bOverrideLocationTemplating; //TES 8/31/2010 - PLID 39630
	CDWordArray anObjectType;
	CDWordArray anObjectID;	
};

struct VisibleTemplate
{
	CString strText;
	//TES 9/3/2010 - PLID 39630 - Replaced dtStart and dtEnd with a CComplexTimeRange
	CComplexTimeRange ctrRange;
	//TES 9/3/2010 - PLID 39630 - Track the LocationID (for Location Templating)
	long nLocationID;
	COLORREF clr;
	long nPriority;
	// (c.haag 2006-12-04 10:17) - PLID 23666 - Add template block information
	int nSingleDayColumn;
	long nTemplateItemID;
	BOOL bIsBlock;
	// (c.haag 2006-12-04 10:17) - PLID 23845 - Add rules information
	CArray<VisibleTemplateRule*, VisibleTemplateRule*> aRules;
	
	// (z.manning 2008-07-24 17:07) - PLID 30804 - Added resource ID and name for use in FFA
	long nResourceID;
	// (d.singleton 2012-01-10 16:15) - PLID 46522 Added BOOL for AllResources
	BOOL bAllResources;
	CString strResource;

	// (j.gruber 2011-05-11 11:56) - PLID 41131 - Added Template Name and ID
	CString strTemplateName;
	long nTemplateID;
	////////////////////////////////////////////////////////////////////////// 

	// (b.spivey, June 14, 2011) - PLID 41131 - This could be useful at some point, but it's wrong. There are some issues
	//   with it that, at this juncture, are not worth fixing. They are listed below:
	//  - VisibleTemplateRule(VTR) has CDWordArrays which have their own pointers in memory, which means VTR would need its own
	//		assignment operator to handle the new pointers correctly. 
	//  - Deleting pNew here creates bad pointer references to the rules. 
	//  - There is no destructor for this or VTR, which means we don't know *who* is incharge of deleting the new rules pointers. 
	//		(which is beside the point that they don't get deleted at all). 
	//VisibleTemplate& operator=(VisibleTemplate& src){
	//	this->strText = src.strText;

	//	CArray<TimeRange, TimeRange&> aryRangeSrc;
	//	src.ctrRange.GetRanges(aryRangeSrc);
	//	for(int itRange = 0; itRange < aryRangeSrc.GetSize(); itRange++){
	//		this->ctrRange.AddRange(aryRangeSrc.GetAt(itRange).dtStart, aryRangeSrc.GetAt(itRange).dtEnd);
	//	}

	//	this->nLocationID = src.nLocationID; 
	//	this->clr = src.clr; 
	//	this->nPriority = src.nPriority;
	//	this->nSingleDayColumn = src.nSingleDayColumn;
	//	this->nTemplateItemID = src.nTemplateItemID; 
	//	this->bIsBlock = src.bIsBlock;

	//	for(int rule = 0; rule < src.aRules.GetSize(); rule++) {
	//		VisibleTemplateRule *pOldRule = src.aRules.GetAt(rule);
	//		VisibleTemplateRule *pNewRule = new VisibleTemplateRule();
	//		memcpy(pNewRule, pOldRule, sizeof(VisibleTemplateRule));			
	//		this->aRules.Add(pNewRule); 
	//		delete pNewRule; 
	//	}

	//	this->nResourceID = src.nResourceID;
	//	this->strResource = src.strResource;
	//	this->strTemplateName = src.strTemplateName; 
	//	return *this; 
	//}
};

// (c.haag 2006-12-04 10:19) - PLID 23666 - This structure is used during
// template block appearance calculations
struct BlockReservation
{
	VisibleTemplate* pTemplate;
	COleDateTime dtStart;
	COleDateTime dtEnd;
};

// (c.haag 2006-12-12 15:28) - PLID 23845 - This structure is used during
// complicated template block/versus/appointment appearance calculations.
// This is only populated if the "EnablePrecisionTemplating" preference
// is set to non-zero; no other functionality uses it
// (z.manning, 05/24/2007) - PLID 26122 - The precision templating preference c.haag mentions
// never actually existed in a release, thus this struct is always used.
struct VisibleAppointment
{
	long nReservationID;
	long nAptTypeID;
	CDWordArray anResourceIDs;
	CDWordArray anPurposeIDs;
	COleDateTime dtStart;
	COleDateTime dtEnd;
	short nOffsetDay;
};

//
// (c.haag 2005-12-12 16:38) - PLID 16849 - AppointmentGrabWithValidation
// is more than 255 debug characters long
//
#pragma warning(push)
#pragma warning (disable:4786)

// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support for CNxSchedulerDlg
class CNxSchedulerDropTarget;

class CNxSchedulerDlg : public CNxDialog, public CSchedulerViewSheet
{
	friend class CSchedulerView;
	friend class CResEntryDlg;
	// (a.walling 2008-09-18 16:42) - PLID 28040
	//friend class CSchedulerUpdateThread;
	friend class CAppointmentsDlg;
	friend class CSchedulerSetup;
	friend class CTemplateItemEntryGraphicalDlg;
	friend class CTemplatesDlg;
	friend class CNxSchedulerDropTarget;

	// Construction
public:

	// (b.cardillo 2003-06-30 15:44) TODO: Since this is implemented as a call to the parent anyway, 
	// we're leaving it as is.  Eventually we need to get rid of this function because whoever calls 
	// it needs to know that there is an OFFICIAL date that must be shared by all tabs, and therefore 
	// no tab is allowed to implement its own "GetActiveDate".
	const COleDateTime &GetActiveDate();

	CNxSchedulerDlg(int IDD, CWnd* pParent /*=NULL*/);
	~CNxSchedulerDlg();

	// Added by Chris (6/14/what year chris?)
	void MoveCurrentDate(COleDateTimeSpan span);

	// Added by Bob (2002-06-05)
	void MoveCurrentDateByUnits(EnumNxSchedulerForwardBackUnits nsfbuUnits, int nMoveByCount);
	void MoveCurrentDateByMonths(int nMonths);
	void MoveCurrentDateByYears(int nYears);

	virtual void UpdateViewBySingleAppt(CTableCheckerDetails* pDetails);
	virtual void UpdateViewBySingleAppt(long nResID);
	BOOL IsAppointmentVisible(long nResID);

	//TES 12/3/2014 - PLID 64180 - Similar to FindFirstAvailableAppt(), but will set the given values, and immediately launch the results window
	//NOTE: if adwPurpose has multiple entries, the results window will not immediately launch, the caller needs to warn the user about that.
	//TES 12/18/2014 - PLID 64466 - If successful, this will fill pSelectedSlot with information about the slot the user chose through FFA
	void FindFirstAvailableApptWithPresets(long nPatientID, long nAptTypeID, const CDWordArray& adwPurpose, const CDWordArray& adwResource, long nInsuredPartyID, OUT SelectedFFASlotPtr &pSelectedSlot, long nLocationID);


	// (j.luckoski 2012-04-26 10:09) - PLID 11597 - Variables for showing cancelled appts
	long m_nDateRange;
	long m_nCancelledAppt;
	long m_nCancelColor;

	CArray<VisibleTemplate*, VisibleTemplate*> m_aVisibleTemplates;
	CArray<VisibleAppointment*, VisibleAppointment*> m_aVisibleAppts;

	CNxSchedulerMenu m_Menu;

	NXDATALISTLib::_DNxDataListPtr m_dlTypeFilter;
	NXDATALISTLib::_DNxDataListPtr m_dlPurposeFilter;
	NXDATALISTLib::_DNxDataListPtr m_dlResources;
	NXDATALISTLib::_DNxDataListPtr m_dlInterval;
	//TES 6/21/2010 - PLID 21341
	NXDATALISTLib::_DNxDataListPtr m_dlLocationFilter;

	CComboBox m_cboPurposeSetFilter;

	void SelChangeIntervalCombo(long nNewSel);

	virtual void OnSetDateSelCombo();
	BOOL m_bIsDateSelComboDroppedDown;

	// (c.haag 2007-03-15 17:39) - PLID 24514 - TRUE if the scheduler is on a timer to be
	// updated if the user is rapid-firing through days
	BOOL m_bIsWaitingForTimedUpdate;

	//TES 3/16/04: Will we allow you to change the active date/resource/filters?
	bool AllowChangeView();

	//TES 2/11/2015 - PLID 64120 - Used when first creating a mix rule, to update whatever scheduler tab might already be open
	void ShowMixRulesButton();

// Dialog Data
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CNxSchedulerDlg)
	CSingleDay	m_pSingleDayCtrl;
	CSingleDay	m_pEventCtrl;
	CDateTimePicker	m_DateSelCombo;
	CNxIconButton	m_btnBack;
	CNxIconButton	m_btnForward;
	CNxIconButton	m_btnResourceOrder;

	// (j.dinatale 2010-09-08) - PLID 23009 - needed to subclass our room manager button
	CNxIconButton m_btnRoomManager;
	CNxIconButton m_btnRecall;	// (j.armen 2012-02-29 17:47) - PLID 48488
	CNxIconButton m_btnViewMixRules; //TES 11/18/2014 - PLID 64120
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	CNxLabel m_nxbtnTodayButton;
	CNxLabel m_nxbtnAmPmButton;
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Make event button a clickable label
	CNxLabel m_nxlabelEventBtnLabel;

		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use notify event handlers
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxSchedulerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnReservationClick(LPDISPATCH theRes);
	virtual void OnReservationDrag(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	virtual void OnReservationEndDrag(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	virtual void OnReservationAdded(LPDISPATCH theRes);
	virtual void OnReservationResize(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	virtual void OnReservationEndResize(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY);
	virtual void OnReservationRightClick(LPDISPATCH theRes, long x, long y);
	virtual void OnReservationMouseDown(LPDISPATCH theRes, short Button, short Shift, long x, long y);
	virtual void OnMouseUpSingleday(short Button, short Shift, long x, long y);
	virtual void OnMouseUpEvent(short Button, short Shift, long x, long y);
	virtual void OnReservationRecalculateColor(LPDISPATCH theRes);
	
	virtual void OnChangeDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCloseUpDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnDropDownDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void StoreDetails();
	virtual void RecallDetails();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	void OnMouseUpGeneric(short Button, short Shift, long x, long y, BOOL bIsOnEventControl);
	// (c.haag 2010-02-04 12:20) - PLID 37221 - We no longer use IReservation*
	// (c.haag 2010-07-13 12:03) - PLID 39615 - We now take in a dispatch, not a CReservation
	void DoPopupContextMenu(IN const CPoint &ptPopUpAt, IN LPDISPATCH theRes, OPTIONAL IN const CSchedTemplateInfo *psti);
	void DoPopupContextMenu(IN const CPoint &ptPopUpAt, IN const DWORD dwClickedResourceID, IN const COleDateTime &dtClickedDate, IN const COleDateTime &dtClickedTime, long nClickedDay, IN const BOOL bClickedEvent, OPTIONAL IN const CSchedTemplateInfo *psti);

protected:
	void OnChangeResourceViewMenuCommand(WPARAM wParam);

protected:
	virtual void Internal_ReflectCurrentResourceListOnGUI();
	virtual void Internal_ReflectActiveResourceOnGUI();

// Implementation
protected:
	// (j.jones 2014-12-02 15:39) - PLID 64182 - if a mix rule is overridden, it is passed up to the caller
	ADODB::_RecordsetPtr AppointmentGrabWithValidation(CReservation& pRes, IN const COleDateTime &dtDraggedFromDate, IN const long nDraggedFromResourceID, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot);

// (z.manning 2008-07-23 14:30) - PLID 30804 - Move all the template block relatead functions to GlobalSchedUtils	

protected:
	// (j.jones 2014-08-13 16:57) - PLID 63184 - this now takes in the ResourceIDs and PurposeIDs, rather than a recordset
	void EnsureApptInVisibleApptArray(long nResID, ADODB::_RecordsetPtr& prsAppt, CString strResourceIDs, CString strPurposeIDs, short nOffsetDay);
	void EnsureApptNotInVisibleApptArray(long nResID, short nOffsetDay);

public:
	virtual long GetPurposeSetId();
	virtual long GetPurposeId();
	//TES 6/21/2010 - PLID 21341
	virtual long GetLocationId();
	virtual void UpdateReservations(bool bUpdateTemplateBlocks, bool bResolveAndRefresh);
	// (j.gruber 2011-02-17 16:34) - PLID 42425 - added bool&
	virtual void ClearBlockReservations(int iColumn, OUT BOOL &bNeedsResolve);
	virtual void UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate);
	// (z.manning, 05/24/2007) - PLID 26062 - Added a 2nd overload of UpdateBlocks to determine how we should load template info.
	virtual void UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate, bool bLoadAllTemplateInfoAtOnce);
	virtual void UpdateTemplateBlocks(bool bResolveRefresh);
	// (z.manning, 02/13/2008) - PLID 28909 - Function to update template reservations.
	virtual void UpdateAttendanceReservations();

	//TES 3/8/2011 - PLID 41519 - Clear out the currently "highlighted" appointment (if any)
	virtual void ClearHighlightedReservation();
	//TES 3/31/2011 - PLID 41519 - Set this variable before refreshing the screen, to force the given appointment ID to be the "highlighted" appointment
	long m_nPendingHighlightID;

	// (z.manning, 02/13/2008) - PLID 28909 - Function to return the open recordset to load attendance appointments.
	virtual ADODB::_RecordsetPtr GetAttendanceResRecordsetFromBaseQuery(CString strBaseQuery);

	virtual COleDateTime GetWorkingDate(int nDay = 0);
	virtual void CommitResDrag(LPDISPATCH theRes, IN const COleDateTime &dtDraggedFromDate, IN const long nDraggedFromResourceID);
	COleDateTime GetMsTime();
	long GetMsSlot(); //-1 = Event Slot
	long GetMsDay(CSingleDay pCtrl);
	long GetMsDay();
	virtual void EnsureButtonLabels();
	BOOL SetActivePurposeSetId(long nNewId);
	BOOL SetActivePurposeId(long nNewId);
	//TES 6/21/2010 - PLID 21341
	BOOL SetActiveLocationId(long nNewId);

	// (j.jones 2014-08-14 12:47) - PLID 63184 - Added function that lets us know if
	// a given location ID is in our current location filter. Always returns true
	// if we're viewing all locations.
	bool IsLocationInView(long nLocationID);

	void GetTypeFilterIDs(CDWordArray* dwaryTypes);
	void GetPurposeFilterIDs(CDWordArray* dwaryPurposes);
	// (d.singleton 2012-07-02 15:08) - PLID 47473
	void GetLocationFilterIDs(CDWordArray* dwaryLocations);
	bool SetActiveInterval(long nInterval);
	long GetActiveInterval();
	virtual bool NeedUpdate();
	virtual void Print(CDC *pDC, CPrintInfo *pInfo);
	BOOL SetActiveDate(const COleDateTime &newDate);
	virtual void ScrollToMakeDayVisible(long nDayToMakeVisible);
	virtual void ScrollToMakeTimeRangeVisible(const COleDateTime &dtStartTime, const COleDateTime &dtEndTime);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (z.manning, 05/09/2007) - PLID 11593 - Updates only the reservation text.
	virtual void UpdateReservationText();
	
	virtual short GetOffsetDay(CReservationReadSet &rsReadSet);
	virtual bool ReFilter(CReservationReadSet &rsReadSet, bool bForceOpen = false);
	virtual ADODB::_RecordsetPtr GetSingleApptQuery(long nResID);
	virtual BOOL SingleApptFitsSlot(long iDay, ADODB::_RecordsetPtr& prsAppt);
	//Used by ReFilter and GetSingleApptQuery, gives the filter based on the type(s) and purpose(s) being filtered.
	CString GetExtraFilter();
	// (z.manning, 02/15/2008) - PLID 28909 - Gets the SQL where clause portion for type/purpose filters.
	CString GetExtraFilterAttendance();

	// (a.walling 2013-05-31 13:31) - PLID 56961 - Parameterized version of GetExtraFilter; all fields available in GetResSimpleSql
	CSqlFragment GetExtraResSimpleFilter();

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
	virtual void UpdateColumnAvailLocationsDisplay();

	// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
	virtual void UpdateActiveDateLabel();

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Keep track of available locations for each resource
	class CResourceAvailLocations
	{
	public:
		~CResourceAvailLocations();

		void ClearAll();
		void ClearForDay(long nDayIndex);

		// (a.walling 2010-06-23 09:41) - PLID 39263 - Keep track of the location IDs assigned for each resource. This will handle duplicates.
		void HandleTemplateItem(long nDayIndex, class CTemplateHitSet& templateHitSet);

		CString GetDayDescription(long nDayIndex);
		// (c.haag 2010-11-16 13:49) - PLID 39444 - Returns a further formatted day label that can include
		// resource location abbreviations
		CString GetDayLabel(long nDayIndex, const CString& strExistingText);
		// (j.luckoski 2012-05-14 14:46) - PLID 48556 - Move the GetDayLabel here with added arguements for resourceview
		CString GetDayLabel(long nDayIndex, const CString& strExistingText, bool bShowTimes);


		//TES 1/5/2012 - PLID 47310 - Is there any time on the given day on which the resource is available for the given location?
		bool IsAvailableForDay(long nDayIndex, long nLocationID);

		// (r.gonet 05/25/2012) - PLID 49059
	protected:
		// (r.gonet 05/25/2012) - PLID 49059 - Predeclare
		struct LocationTemplateItem;
		// (r.gonet 05/25/2012) - PLID 49059 - Get the highest priority template at a given slot. Returns true if there is a template. False if there is no template at this slot.
		bool GetLocationTemplate(long nDayIndex, const COleDateTime &dtTime, OUT LocationTemplateItem &locationTemplateItem);
	public:
		// (r.gonet 05/25/2012) - PLID 49059
		bool GetLocationTemplateColor(long nDayIndex, const COleDateTime &dtTime, OUT long &nLocationTemplateColor);

		//TES 8/3/2010 - PLID 39264 - For the given day and time, returns the location ID corresponding to the highest-priority Location
		// Template for that slot (will return -1 if there is no Location Template at that slot).
		long GetLocationID(long nDayIndex, const COleDateTime &dtTime);

		//TES 9/7/2010 - PLID 39630 - Adds a time range that should be excluded from location templates.
		void AddExcludedRange(long nDayIndex, const TimeRange &trRange);
		//TES 9/7/2010 - PLID 39630 - Removes a time range from the times that should be excluded from location templates
		void RemoveExcludedRange(long nDayIndex, const TimeRange &trRange);

	protected:

		//TES 8/3/2010 - PLID 39264 - Instead of just LocationIDs, track all the information we need to calculate the location
		// at a given time slot.
		// (a.walling 2011-08-09 08:44) - PLID 45023 - added operator< for collections, and a few other helper functions
		// (r.gonet 05/25/2012) - PLID 49059 - Added color of the template
		struct LocationTemplateItem {
			LocationTemplateItem()
				: nLocationID(-1)
				, nPriority(0)
				, nColor(RGB(255,255,255))
			{
			}

			COleDateTime dtStart;
			COleDateTime dtEnd;
			long nLocationID;
			long nPriority;
			// (r.gonet 05/25/2012) - PLID 49059 - The color of the template
			OLE_COLOR nColor;

			DATE GetSafeStart() const
			{
				if (COleDateTime::valid == dtStart.GetStatus()) {
					return dtStart.m_dt;
				} else {
					return 0.0;
				}
			}

			DATE GetSafeEnd() const
			{
				if (COleDateTime::valid == dtStart.GetStatus()) {
					return dtStart.m_dt;
				} else {
					return 0.0;
				}
			}

			bool operator<(const LocationTemplateItem& r) const
			{
				if (GetSafeStart() != r.GetSafeStart()) {
					return GetSafeStart() < r.GetSafeStart();
				}
				if (GetSafeEnd() != r.GetSafeEnd()) {
					return GetSafeEnd() < r.GetSafeEnd();
				}
				if (nLocationID != r.nLocationID) {
					return nLocationID < r.nLocationID;
				}
				if (nPriority != r.nPriority) {
					return nPriority < r.nPriority;
				}

				return false;
			}
		};
		CMap<long, long, CList<LocationTemplateItem>*, CList<LocationTemplateItem>*& > m_mapLocationTemplateItems;

		//TES 9/7/2010 - PLID 39630 - For each day, we also need to track a range of times for which location templates are excluded.
		CMap<long, long, CComplexTimeRange*, CComplexTimeRange*&> m_mapExcludedTimes;
	};

	CResourceAvailLocations m_ResourceAvailLocations;
	
public:
	void InitSchedViewParent(CSchedulerView *pParent);

protected:
	CSchedulerView *m_pParent;
	// (d.thompson 2009-10-21) - PLID 35598
	bool m_bResLockedByRightClick;

protected:
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Returns TRUE if it's ok to process an UpdateView-type command
	BOOL IsSafeToUpdateView() const;

public:
	// (c.haag 2010-04-30 12:20) - PLID 38379 - We now pass in an owner
	CReservation GetAppointmentReservation(const CString& strOwner, long nAppointmentID, OPTIONAL OUT BOOL *pbIsInEventControl);
	void ShowAppointment(long nAppointmentID, BOOL bShowResEntry, const IN OPTIONAL COleDateTime *pdtChangeDateTo, const IN OPTIONAL long *pnChangeResourceTo);

	//TES 10/27/2010 - PLID 40868 - A new function which will determine which precision template, if any, an appointment represented
	// by the given VisibleAppointment struct would fall.  If there is currently an appointment on this dialog with the same ID as that
	// in pAppt, it will be ignored, under the assumption that pAppt is a newly modified version intended to replace that appointment.
	long CalculateTemplateItemID(VisibleAppointment *pAppt);

	// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support
	// Gathers the mouse time / date / resource info
	Nx::Scheduler::TargetInfo CalcTargetInfo();
	// Updates the m_pParent's m_pResDispatch stuff and m_*Click* props
	void UpdateTargetInfo(Nx::Scheduler::TargetInfo targetInfo);

protected:
	void ClearAndDisable();
	
	// Call this instead of updating immediately.  It's just like calling Invalidate for drawing 
	// code, except we're not drawing we're re-loading data from the database.
	void InvalidateData();

protected:
	CSchedViewColumnInfo &m_vciColInfoOfLastMouseDownRes; // For drag/drop and cut/copy/paste operations

protected:
	unsigned long m_nPatientListStamp;
	bool m_bForceBlocksUpdate;
	COleDateTime m_dtOldBlockDate;
	bool m_bNeedUpdate;
	bool m_bAllowUpdate;
	long m_nActivePurposeSetId;
	long m_nActivePurposeId;
	//TES 6/21/2010 - PLID 21341
	long m_nActiveLocationId;
	CString m_strMultiPurposeIds;	//string list of multiple purposes
	CString m_strMultiTypeIds;		//string lsit of multiple types
	// (d.singleton 2012-06-07 10:51) - PLID 47473 string list of multiple locations
	CString m_strMultiLocationIds;
	CResEntryDlg *m_ResEntry;	
	COleDateTime m_OldDate;
	int m_nDayCnt;
	bool m_bSettingInterval;
	bool m_bIsUpdatingNow;

	// (j.jones 2014-12-03 16:08) - PLID 64276 - Resets the colors on the time slots on the left,
	// mainly for after a ResEntry window is closed and its gray color is removed.
	// Special colors such as time slots with override appts. will be reloaded unless
	// bReColorTimes is set to false, in which case all slots will lose their special colors.
	// The current time is always colored.
	void ResetTimeColors(bool bReColorTimes = true);

	// (j.jones 2014-12-03 16:14) - PLID 64276 - used to cache which time slots need which color,
	// such that you do not need to call ReFilter to find out again
	class ColoredTime {

	public:
		COleDateTime dtStart;
		COleDateTime dtEnd;
		COLORREF color;

		ColoredTime(COleDateTime dtStartTime, COleDateTime dtEndTime, COLORREF clr)
		{
			dtStart = dtStartTime;
			dtEnd = dtEndTime;
			color = clr;
		}
	};
	// (j.jones 2014-12-03 16:14) - PLID 64276 - caches all time slots that need
	// a special color that persists after ResetTimeColors is called
	std::vector<ColoredTime> m_aryColoredTimes;

	// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support for CNxSchedulerDlg
	boost::intrusive_ptr<CNxSchedulerDropTarget> m_pDropTarget;

	void RequeryPurposeFilters();

	void EnsureAmPmText(); //This function makes sure that the AmPm button is showing the right time based on 
							//the office hours for the active day.
	
	// (z.manning, 10/10/2006) - PLID 5812 - Set the visible area of the single day control.
	void SetScheduleVisibleTimeRange();

	// (z.manning, 10/12/2006) - PLID 5812 - Determine if the times fall outside the current visible range.
	BOOL NeedToUpdateVisibleTimeRange(COleDateTime dtApptStart, COleDateTime dtApptEnd);

	//This function makes sure that the lable for how many appointments there are on the screen is
	//correct.
	virtual void EnsureCountLabelText();

	void GetPrintSlotRange(long &nFirstPrintSlot, long &nLastPrintSlot, CPrintInfo *pInfo); //Gets the slots the singleday is about to print out

	//TES 12/18/2008 - PLID 32497 - New overrideable function to enable m_pSingleDayCtrl and m_pEventCtrl, the month
	// dialog sometimes wants to prevent them from being enabled.
	virtual void EnableSingleDayControls();

	// (d.lange 2010-11-08 10:22) - PLID 41192 - We return TRUE if we call LaunchDevice for the loaded device plugin
	// (j.gruber 2013-04-03 14:59) - PLID 56012 - not needed anymore
	

	// Generated message map functions
	//{{AFX_MSG(CNxSchedulerDlg)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg virtual void OnMoveDayBack();
	afx_msg virtual void OnMoveDayForward();
	afx_msg void OnAmPmBtn();
	afx_msg void OnTodayBtn();

	// (j.dinatale 2010-09-08) - PLID 23009 - Room Manager button on all scheduler tabs
	afx_msg void OnRoomManagerBtn();
	afx_msg void OnRecallBtn();	// (j.armen 2012-02-29 17:47) - PLID 48488
	afx_msg void OnReschedulingQueueBtn();

	afx_msg void OnResourceOrderBtn();
	afx_msg void OnShowPatientInfoBtn();
	afx_msg void OnSelChosenPurposeSetFilter(long lNewSel);
	afx_msg void OnSelChangingResources(long FAR* nNewSel);
	afx_msg void OnSelChosenPurposeFilter(long lNewSel);
	afx_msg virtual void OnSelChosenResources(long lNewSel);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnNewEvent();
	afx_msg void OnSelChosenIntervalCombo(long nNewSel);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelChangingPurposeSetFilter(long FAR* nNewSel);
	afx_msg void OnSelChangingPurposeFilter(long FAR* nNewSel);
	afx_msg void OnSelChosenLocationFilter(long lNewSel);
	afx_msg void OnSelChangingLocationFilter(long FAR* nNewSel);
	afx_msg void OnViewMixRules(); //TES 11/18/2014 - PLID 64120
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - Derived classes may handle their own label clicks, so they need to be able to call us so we can process ours
	virtual afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);

	DECLARE_COMMAND_FORWARD_BACK(Days1)
	DECLARE_COMMAND_FORWARD_BACK(Days2)
	DECLARE_COMMAND_FORWARD_BACK(Days3)
	DECLARE_COMMAND_FORWARD_BACK(Days4)
	DECLARE_COMMAND_FORWARD_BACK(Days5)
	DECLARE_COMMAND_FORWARD_BACK(Days6)
	DECLARE_COMMAND_FORWARD_BACK(Weeks1)
	DECLARE_COMMAND_FORWARD_BACK(Weeks2)
	DECLARE_COMMAND_FORWARD_BACK(Weeks3)
	DECLARE_COMMAND_FORWARD_BACK(Weeks4)
	DECLARE_COMMAND_FORWARD_BACK(Weeks5)
	DECLARE_COMMAND_FORWARD_BACK(Weeks6)
	DECLARE_COMMAND_FORWARD_BACK(Weeks7)
	DECLARE_COMMAND_FORWARD_BACK(Months1)
	DECLARE_COMMAND_FORWARD_BACK(Months2)
	DECLARE_COMMAND_FORWARD_BACK(Months3)
	DECLARE_COMMAND_FORWARD_BACK(Months4)
	DECLARE_COMMAND_FORWARD_BACK(Months5)
	DECLARE_COMMAND_FORWARD_BACK(Months6)
	DECLARE_COMMAND_FORWARD_BACK(Years1)
	DECLARE_COMMAND_FORWARD_BACK(Years2)
	DECLARE_COMMAND_FORWARD_BACK(Years3)
	DECLARE_COMMAND_FORWARD_BACK(Years4)
	DECLARE_COMMAND_FORWARD_BACK(Years5)
	afx_msg void OnDestroy();
	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

namespace Nx {
namespace Scheduler {

// (a.walling 2013-05-31 13:31) - PLID 56961 - Simplified version of ResExtendedQ for use by GetSingleApptQuery etc
// note this does not include all fields of ResExtendedQ, just the ones needed by the nxscheduler, week, 
// multiresource, and month views for UpdateViewBySingleAppt
CSqlFragment GetResSimpleSql();
 
}
}

#pragma warning(pop)

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXSCHEDULERDLG_H__5BEA8D23_C898_11D1_804A_00104B2FE914__INCLUDED_)
