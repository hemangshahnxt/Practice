#if !defined(AFX_ROOMMANAGERDLG_H__0805F5DF_2978_4277_A4A9_32727915703C__INCLUDED_)
#define AFX_ROOMMANAGERDLG_H__0805F5DF_2978_4277_A4A9_32727915703C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomManagerDlg.h : header file
//

#include "Color.h"
#include "SchedulerRc.h"
#include "DevicePlugin.h"
#include "NxPictureToolTip.h" //(a.wilson 2011-10-27) PLID 45936

/////////////////////////////////////////////////////////////////////////////
// CRoomManagerDlg dialog

// (j.jones 2010-08-27 10:24) - PLID 36975 - enum for the patient name preference, we have
// so many options it just makes sense to have an enum for this
enum ERoomMgrPatientNameDisplayPref {

	patnameFullName = 0,
	patnameFirstNameLastInitial = 1,
	patnameFirstInitialLastName = 2,
	patnameLastNameFirstInitial = 3,
	patnameFirstNameOnly = 4,
	patnameLastNameOnly = 5,
	patnameFirstInitialLastInitial = 6,
};

// (j.gruber 2013-04-02 12:29) - PLID 56012 - not needed
// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
class CRoomManagerDlg : public CNxModelessOwnedDialog
{
// Construction
public:
	CRoomManagerDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_pResourceCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pPatientList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRoomList;
	NXDATALIST2Lib::_DNxDataListPtr m_pWaitingAreaList;
	// (j.jones 2008-05-28 17:36) - PLID 27797 - added the checkout list
	NXDATALIST2Lib::_DNxDataListPtr m_pCheckoutList;
	// (j.jones 2009-08-03 17:28) - PLID 24600 - added the location combo
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;
	// (j.jones 2010-12-02 09:03) - PLID 38597 - added the waiting room combo
	NXDATALIST2Lib::_DNxDataListPtr m_pWaitingRoomCombo;

	void RequeryAll(BOOL bRebuildFolders);

	void ResetHourTimer();

	void FireTimeChanged();

	long m_nPendingShowAppointment;
	void ShowAppointment(long nAppointmentID);

	//(a.wilson 2011-10-27) PLID 45936 - member variable for picture tool tip reference object.
	CNxPictureToolTip m_pttPatientList;

	// (z.manning 2009-08-18 11:27) - PLID 28014 - Added a function to minimize the room manager dialog.
	void MinimizeWindow();

	// (d.lange 2010-06-28 16:14) - PLID 37317 - Added a function to ensure the columns to display
	void EnsureWaitingAreaColumns();
	void EnsureRoomsColumns();
	void EnsureCheckoutColumns();

	// (j.jones 2010-08-27 10:22) - PLID 36975 - called when Preferences are changed,
	// it means this dialog must check to see if the patient name preference has changed
	void OnPreferencesChanged();

// Dialog Data
	//{{AFX_DATA(CRoomManagerDlg)
	enum { IDD = IDD_ROOM_MANAGER_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnEditRooms;
	CNxIconButton	m_btnEditStatus;
	CNxIconButton	m_btnConfigColumns;		// (d.lange 2010-06-28 14:53) - PLID 37317 - Added 'Configure Columns' button

	
	CNxLabel	m_nxlRoomMgrMultiResourceLabel;// (s.dhole 2010-06-30 16:54) - PLID  38947 Add some color to the Room Manager
	NxButton m_chkRememberColumnWidths;
	// (j.jones 2009-09-21 14:41) - PLID 25232 - added print option
	CNxIconButton	m_btnPrintPreview;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomManagerDlg)
public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);	//(a.wilson 2011-10-27) PLID 45936
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CColor m_clrApptCheckedOut;
	CColor m_clrApptCheckedIn;
	CColor m_clrApptReadyToCheckOut;	// (j.jones 2008-05-30 09:02) - PLID 27797
	CColor m_clrApptNoShow;

	CColor m_clrFolderEmpty;

	UINT m_nTimerHourExpansion;
	UINT m_nTimerHourlyServerTimeCheck;
	UINT m_nTimerFullRefresh; // (z.manning 2009-07-10 13:41) - PLID 34848

	COleDateTimeSpan m_dtOffset;

	// (c.haag 2010-10-26 10:07) - PLID 39199 - This map is used to tell us for a given AptShowStateT.ID whether
	// it counts as an entry that should appear in the waiting area. We used to check on ID = 1 but now other ID's
	// can have that property.
	// (j.jones 2010-12-09 10:18) - PLID 41763 - we now support all custom states, but waiting area states are stored
	CMap<long,long,BOOL,BOOL> m_mapShowStateIsWaitingArea;
	CMap<long,long,CString,LPCTSTR> m_mapShowStateNames;
	CArray<long, long> m_aryShowStates;

	long m_nTopRoomID; // (c.haag 2010-05-07 11:11) - PLID 35702 - The ID of the first visible room in the room list before a requery took place, or -1 if none
	long m_nTopWaitingAreaID; // (c.haag 2010-08-02 10:46) - PLID 35702
	long m_nTopCheckoutID; // (c.haag 2010-08-02 10:46) - PLID 35702
	
	BOOL CheckEMNWarning(long nTemplateID);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	void AddToExistsEMR(long nEMRTemplateID,long nPatientID);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	bool m_bEMRExpired;// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	long  m_nRoomMgrPatientSelection;// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	long m_nRoomMgrPatientSelection_action;// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	void RequeryPatientList(BOOL bRebuildFolders);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	void CheckEMNExist(long nPatientID,CString strPatientName);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	void CheckEMRTemplateWithCollection(long nRoomAppointmentID,long nPatientID);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	void ShowEMRTemplateCollection(long nRoomAppointmentID,long nPatientID);// (s.dhole 2010-02-03 11:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	
	// (c.haag 2010-11-02 12:10) - PLID 39199 - Add custom statuses with the Waiting Area bit set to a menu of statuses.
	// We also take in a state to exclude in case the appointment is already assigned to it
	// (j.jones 2010-12-09 10:25) - PLID 41763 - renamed this function, we now support all custom statuses
	void AddShowStateStatusesToMenu(CMenu& mnu, long nShowStateToExclude);

	// (c.haag 2010-10-26 10:07) - PLID 39199 - Update the map that associates show state ID's with waiting area bits.
	// (j.jones 2010-12-09 10:20) - PLID 41763 - renamed this function, we now load all show states
	void UpdateCachedShowStateInfo();

	// (j.jones 2007-02-07 09:37) - PLID 24595 - requeries the Waiting Area list
	void RequeryWaitingAreaList();
	// (s.tullis 2013-11-13 10:36) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
	void RequeryResourceList();
	


	// (j.jones 2008-05-29 10:45) - PLID 27797 - requeries the Checkout list
	void RequeryCheckoutList();

	void CalculateTimeRange(long &nFirstHour, long &nLastHour, long nLocationID = -1);
	void GetLocationOpenCloseTimes(COleDateTime &dtOpen, COleDateTime &dtClose, long nLocationID = -1);

	// (j.jones 2007-02-07 10:57) - PLID 24595 - converted GeneratePatientMenu
	// to GenerateAppointmentMenu, to reuse the code in the Waiting Area list
	// (j.jones 2008-05-29 14:43) - PLID 27797 - added checkout option, and optional RoomAppointmentID
	// (j.jones 2010-12-02 16:36) - PLID 38597 - renamed the checkout option to bFromCheckoutList, and added bFromWaitingArea and nRoomID
	void GenerateAppointmentMenu(long nPatientID, long nApptID, CPoint &pt, BOOL bFromCheckoutList = FALSE, long nRoomAppointmentID = -1, BOOL bFromWaitingArea = FALSE, long nRoomID = -1);
	void GenerateRoomMenu(LPDISPATCH lpRow, CPoint &pt);
	// (s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	bool GoToPatient(long nPatientID, short nTab=-1); // change to support tab
	
	void GoToAppointment(long nApptID);

	void AssignToRoom(long nApptID, long nRoomID);
	void ChangeRoomAppointmentStatus(long nRoomAppointmentID, long nAppointmentID, long nNewStatusID);
	// (j.jones 2008-05-30 10:10) - PLID 27797 - renamed to PostCheckedOut, formerly was called PostRemovedFromRoom
	BOOL PostCheckedOut(long nRoomAppointmentID, long nAppointmentID);
	// (j.jones 2008-06-02 09:59) - PLID 30219 - ensures that any place that marks an appt. 'Ready To Check Out'
	// will call the appropriate PracYakker code
	void OnReadyToCheckOut(long nPatientID, CString strPatientFirstName, CString strPatientLastName, CString strAppointmentDescription);

	void CloseRoomManager();

	// (j.jones 2007-02-07 11:22) - PLID 24595 - renamed the PatientWaitTime functions
	// to reflect RoomWaitTimes, and added WaitingAreaWaitTime functions
	void UpdateAllRoomWaitTimes(BOOL bClearColors);
	void UpdateRoomWaitTime(long nAppointmentID);
	void UpdateAllWaitingAreaWaitTimes(BOOL bClearColors);
	void UpdateWaitingAreaWaitTime(long nAppointmentID);
	// (j.jones 2008-05-29 10:44) - PLID 27797 - added checkout list
	void UpdateAllCheckoutListWaitTimes(BOOL bClearColors);
	void UpdateCheckoutListWaitTime(long nAppointmentID);

	void ExpandCurrentHour(BOOL bMaintainLastHour, BOOL bTryShowNextHour);
	void CheckAutoCollapseFinishedHours();

	void CalculateLocalTimeOffset();

	// (j.jones 2014-08-18 13:24) - PLID 63404 - added class for information loaded in
	// ReflectChangedAppointment, to ensure we only load this information once
	class ReflectChangedAppointment_ApptInfo
	{
	public:	
		long m_nAppointmentID;
		bool m_bIsLoaded;	//true if we tried to load this appt. already		
		bool m_bExists;		//true if the appt. exists in data
		long m_nPatientID;
		CString m_strPatientName;
		CString m_strPurpose;
		long m_nAptTypeColor;
		_variant_t m_varArrivalTime;
		
		ReflectChangedAppointment_ApptInfo() {
			m_nAppointmentID = -1;
			m_bIsLoaded = false;
			m_bExists = false;
			m_nPatientID = -1;
			m_nAptTypeColor = 0;
			m_varArrivalTime = g_cvarNull;
		}
	};
	// (j.jones 2014-08-18 13:28) - PLID 63404 - fills a ReflectChangedAppointment_ApptInfo if
	// m_bIsLoaded is false, avoids the recordset entirely if already loaded
	void LoadReflectChangedAppointmentInfo(long nAppointmentID, ReflectChangedAppointment_ApptInfo &eInfo, CSqlFragment sqlLastArrivalTime = CSqlFragment("NULL"));

	void ReflectChangedAppointment(CTableCheckerDetails* pDetails);
	void ReflectChangedRoomAppointment(CTableCheckerDetails* pDetails);
	// (j.jones 2010-12-02 10:30) - PLID 38597 - if ReflectChangedRoomAppointment is for a waiting room,
	// this function is called to reflect the WaitingRoom list
	void ReflectChangedWaitingRoomAppointment(long nRoomAppointmentID, long nRoomID, long nAppointmentID, long nStatusID);

	void ColorApptRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (c.haag 2010-05-07 11:11) - PLID 35702 - This function requeries the room list in such a way as
	// to try to keep the vertical scroll position as close to where it was as possible. The other part
	// of the functionality is in OnRequeryFinished.
	void RequeryRoomList();

	// (j.jones 2006-12-21 16:31) - PLID 23196 - added resource filter functionality
	CString m_strResourceIDs;
	CString m_strResourceList;
	void OnMultipleResources();
	void ToggleResourceDisplay();
	BOOL m_bIsResourceComboHidden;
	CString GetStringOfResources();
	void DoClickHyperlink(UINT nFlags, CPoint point);
	//CRect m_rcMultiResourceLabel;// (s.dhole 2010-06-30 16:54) - PLID   38947 Add some color to the Room Manager
	void DrawResourceLabel(CDC *pdc);

	// (j.jones 2007-02-20 13:40) - PLID 24365 - used to flash the window when minimized or inactive
	void Flash();

	// (j.gruber 2009-07-10 10:06) - PLID 28792 - remember column widths
	CString m_strWaitingColumnWidths;
	CString m_strRoomColumnWidths;
	CString m_strCheckoutColumnWidths;
	CString m_strWaitingColumnStyles;
	CString m_strRoomColumnStyles;
	CString m_strCheckoutColumnStyles;
	void SetColumnSizes();
	void ResetColumnSizes();
	void SetDefaultColumnWidths();
	
	void ColumnSizingFinishedWaitingAreaList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void ColumnSizingFinishedRoomList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void ColumnSizingFinishedCheckoutList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);	

	// (j.jones 2010-08-31 14:45) - PLID 35012 - cache the coloring preferences
	BOOL m_bColorApptListByType;
	BOOL m_bColorPurposeColumnByType;

	// (j.jones 2010-08-27 10:22) - PLID 36975 - cache the patient name preference
	ERoomMgrPatientNameDisplayPref m_eRoomMgrPatientNameDisplayPref;

	// (j.jones 2010-08-27 10:33) - PLID 36975 - returns how the patient name should
	// be loaded in a query, based on the patient name preference
	// (j.jones 2014-08-15 15:58) - PLID 63185 - changed to return a SQL fragment
	CSqlFragment GetPatientNameSqlFormat();

	// (d.lange 2010-12-03 12:50) - PLID 40295 - EMRPreviewPopupDlg object
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	// (d.lange 2010-12-08 16:25) - PLID 40295 - EMR Preview icon
	HICON m_hIconPreview;

	// (j.armen 2012-03-05 11:30) - PLID 48555 - Recall Flag
	void OnRecallFlagClicked(NXDATALIST2Lib::IRowSettingsPtr&, const short& nStatusCol, const short& nPatientIDCol);
	void EnsureRecallFlagColumn(NXDATALIST2Lib::_DNxDataListPtr& pDL, const short& nStatusCol, const short& nIconCol, const CString& strPreference);
	HICON m_hIconRecallFlag;

	// (j.jones 2010-10-13 15:51) - PLID 39778 - returns today's EMNID, sorted by unbilled first,
	// filtered only on EMNs that have charges
	long GetTodaysBillableEMNForPatient(const long nPatientID);

	// (j.jones 2010-10-14 08:49) - PLID 39778 - open a bill with a given EMNID, may generate a menu first
	void BillEMNForPatient(const long nPatientID, const long nEMNID, const long nInsuredPartyID);

	// (j.gruber 2013-04-02 12:29) - PLID 56012 - not needed anymore
	// (j.jones 2010-12-02 09:09) - PLID 38597 - requery waiting room combo
	void RequeryWaitingRoomCombo();

	// (d.lange 2010-12-03 12:53) - PLID 40295 - Ensure that the member EMRPreviewPopupDlg is created
	void CreateEmrPreviewDialog(long nPatientID);

	// (j.jones 2014-08-15 14:06) - PLID 63185 - given two space-delimited resource strings,
	// does one have any ID that exists in the other?
	bool DoesResourceExistInFilter(CString strApptResourceIDs, CString strResourceIDFilter);

	// (j.jones 2008-05-29 11:29) - PLID 27797 - added checkout list functions
	// Generated message map functions
	//{{AFX_MSG(CRoomManagerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnRequeryFinishedTodaysPatientsList(short nFlags);
	afx_msg void OnBtnRoomEditor();
	afx_msg void OnBtnEditRoomStatus();
	afx_msg void OnRButtonDownTodaysPatientsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownRoomList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickTodaysPatientsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingRoomList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRequeryFinishedRoomList(short nFlags);
	afx_msg void OnSelChosenRoomMgrResourceCombo(LPDISPATCH lpRow);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	/*afx_msg void OnLButtonDown(UINT nFlags, CPoint point);// (s.dhole 2010-06-30 16:55) - PLID 38947 Add some color to the Room Manager
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);// (s.dhole 2010-06-30 16:55) - PLID 38947 Add some color to the Room Manager
	afx_msg void OnPaint();*/// (s.dhole 2010-06-30 16:55) - PLID  38947 Add some color to the Room Manager
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);// (s.dhole 2010-06-30 16:55) - PLID  38947 Add some color to the Room Manager
	afx_msg void OnLeftClickWaitingAreaList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownWaitingAreaList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedWaitingAreaList(short nFlags);
	afx_msg void OnRButtonDownCheckoutList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedCheckoutList(short nFlags);
	afx_msg void OnLeftClickCheckoutList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRememberColWidths();
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized); //(a.wilson 2011-11-4) PLID 45936
	// (j.jones 2009-08-04 09:12) - PLID 24600 - added location filter
	afx_msg void OnSelChosenRoomMgrLocationCombo(LPDISPATCH lpRow);
	// (j.jones 2009-09-21 15:41) - PLID 25232 - added print option
	afx_msg void OnBtnPreviewRoomMgr();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//	void LButtonDownRoomList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickRoomList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (d.lange 2010-06-30 16:47) - PLID 37317 - Added to configuring the datalist columns
	afx_msg void OnBnClickedBtnConfigColumns();
	// (j.jones 2010-12-02 09:24) - PLID 38597 - added waiting room combo
	afx_msg void OnSelChosenRoomMgrWaitingRoomCombo(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMMANAGERDLG_H__0805F5DF_2978_4277_A4A9_32727915703C__INCLUDED_)
