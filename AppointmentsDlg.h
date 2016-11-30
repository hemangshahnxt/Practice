#include "PatientDialog.h"
#if !defined(AFX_APPOINTMENTSDLG_H__7C4B3513_27BE_11D2_80A0_00104B2FE914__INCLUDED_)
#define AFX_APPOINTMENTSDLG_H__7C4B3513_27BE_11D2_80A0_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AppointmentsDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

#define ID_APPT_RESTORE 32768
#define ID_APPT_DELETE  32769
#define ID_APPT_NOSHOW  32770
#define ID_APPT_GOTO    32771
#define ID_APPT_CANCEL  32772
#define ID_APPT_SHOW	32773
#define ID_APPT_LINK	32777
#define ID_APPT_CONFIRMED 32779
#define ID_APPT_UNCONFIRMED 32780
#define ID_APPT_MOVE_UP 32781
#define ID_APPT_REMOVE_MOVE_UP 32782
#define ID_APPT_NEW_INV_ALLOCATION 32783	// (j.jones 2007-11-21 14:36) - PLID 28147
#define ID_APPT_NEW_INV_ORDER	32784	// (j.jones 2008-03-18 14:25) - PLID 29309
#define ID_APPT_NEW_BILL	32786		// (j.jones 2008-06-23 16:16) - PLID 30455
#define ID_APPT_EDIT_INV_ALLOCATION 32787	// (j.gruber 2008-09-10 15:38) - PLID 30282
#define ID_APPT_CREATE_CASE_HISTORY	32788	// (j.jones 2009-08-28 12:54) - PLID 35381 - added case history options
#define ID_APPT_EDIT_CASE_HISTORY	32789
#define ID_APPT_LEFTMESSAGE			32790	//(e.lally 2009-09-28) PLID 18809
//IDs of 32791 through 32797 are used for cut/copy/delete, etc.
#define ID_APPT_NEW_PRIMARY_COPAY	32798	// (j.jones 2010-09-24 11:48) - PLID 34518 - added ability to create a copay
#define ID_APPT_VIEW_ELIGIBILITY_RESPONSES	32799	// (j.jones 2010-09-27 11:21) - PLID 38447 - added ability to view eligibility requests/responses

#define ID_APPT_PENDING	32800
#define ID_APPT_IN		32801
#define ID_APPT_OUT		32802
#define ID_APPT_NOSHOWSTATUS 32803
#define ID_APPT_RECEIVED 32804
#define ID_APPT_USERDEFINED 32805
// (c.haag 2003-08-07 10:05) - All ID's after 32805 are reserved for
// show states.

// (a.walling 2014-12-22 15:51) - PLID 64418 - Check if on Rescheduling Queue and provide option to open if so
#define ID_APPT_RESCHEDULE 32700
// (z.manning 2015-07-22 15:05) - PLID 67241
#define ID_APPT_MANAGE_PAYMENT_PROFILES 32701

/////////////////////////////////////////////////////////////////////////////
// CAppointmentsDlg dialog

class CAppointmentsDlg : public CPatientDialog
{
// Construction
public:
	// Built in status
	enum EAppointmentShowState {
		astToBeRescheduled = -3L, // (a.walling 2014-12-22 16:44) - PLID 64382 - Only applicable when cancelled and in rescheduling queue
		astCancelledNoShow = -2L,
		astCancelled = -1L,
		astPending = 0L,
		astIn = 1L,
		astOut = 2L,
		astNoShow = 3L,
		astReceived = 4L
	};
	virtual void SetColor(OLE_COLOR nNewColor);
	bool m_IsComboVisible;
	bool m_bAllowUpdate;
	void RefreshList(); // (j.dinatale 2011-08-12 17:59) - PLID 30025 - Renamed to GenerateWhere() to RefreshList(), to improve readability
	void SwitchToAppointment();
	CString m_SQLStr;
	CAppointmentsDlg(CWnd* pParent);   // standard constructor

	// (b.savon 2012-03-29 11:11) - PLID 49074 - Add recall icon
	virtual ~CAppointmentsDlg();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void UpdateViewBySingleAppt(long nApptID);
	COleVariant m_varStatus;
	void SetDays();	//DRT 9/20/01 - for showing the textual day of the week
	CDWordArray m_adwShowState;
	CStringArray m_astrShowState;

	// (j.gruber 2010-01-11 14:48) - PLID 22964 - no show/cancel counts
// Dialog Data
	//{{AFX_DATA(CAppointmentsDlg)
	enum { IDD = IDD_APPOINTMENTS_DLG };
	NxButton	m_btnShowReasonForNoShow;
	NxButton	m_btnShowCancelledDate;
	NxButton	m_btnShowModifiedDate;
	NxButton	m_btnRememberColumnSettings;
	NxButton	m_btnShowCancelled;
	NxButton	m_btnShowNoShows;
	CNxEdit		m_edtNumCancellations;
	CNxEdit		m_edtNumNoShows;
	CNxColor	m_bg;
	CNxIconButton m_btnRecall;	// (j.armen 2012-02-29 10:34) - PLID 48487
	CNxIconButton m_btnPatientEncounter; // (b.savon 2014-12-01 10:33) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter
	//}}AFX_DATA

	// (j.gruber 2015-01-08 13:58) - PLID 64545 - Change the Patient's appointment dialog to use a datalist 2 instead of a datalist 1
	NXDATALIST2Lib::_DNxDataListPtr	m_pApptList;

// Overrides
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-07 14:33) - PLID 63168 - added an Ex handler
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppointmentsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

private:

protected:	
	// (a.walling 2010-10-13 16:23) - PLID 40977
	long m_id;

	// (j.jones 2014-08-07 15:10) - PLID 63168 - added tablechecker for ShowStates
	CTableChecker m_AptShowStateChecker;

	// (b.savon 2012-03-29 11:44) - PLID 49074
	HICON m_hiRecall;
	// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
	HICON m_hExtraNotesIcon;

	void LoadIncludeModifiedDateColumns();
	void ReflectIncludeModifiedDateColumns();
	void LoadIncludeCancelledDateColumns();
	void ReflectIncludeCancelledDateColumns();
	void LoadIncludeNoShowReasonColumn();
	void ReflectIncludeNoShowReasonColumn();
	void LoadShowStateArray();
	void OnMarkIn();
	void OnMarkOut();
	void OnMarkPending();
	void OnMarkReceived();
	void OnMarkUserDefinedShowState(EAppointmentShowState nState);
	CString GetShowStateName(EAppointmentShowState type);
	void SetColumnSizes();
	void ResetColumnSizes();
	CString m_strOriginalColSizes;
	void SaveColumnSizes();

	// (j.gruber 2010-01-11 14:49) - PLID 22964 - noshow/cancel counts
	void LoadNoShowCancelBoxes();

// Implementation
protected:	
	void SetAppointmentColors();
	COleVariant varSelItem;
	
	// (d.lange 2010-11-08 09:34) - PLID 41192 - We return TRUE if we call LaunchDevice for the loaded device plugin
	// (j.gruber 2013-04-03 14:51) - PLID 56012 - not needed anymore
	
	// (b.savon 2012-03-27 16:10) - PLID 49074 - Add recall icon in Patients->Appts tab when the appt is linked to a recall
	// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Changed to include notes icon too
	void ShowIcons();

	// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
	// (j.jones 2008-03-18 14:26) - PLID 29309 - added ability to create a new inventory order
	// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
	// (j.gruber 2008-09-10 15:43) - PLID 30282 - added ability to edit an allocation
	// Generated message map functions
	//{{AFX_MSG(CAppointmentsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGotoAppointment();
	afx_msg void OnRescheduleAppointment();
	afx_msg void OnDeleteAppointment();
	afx_msg void OnRestoreAppointment();
	afx_msg void OnCancelAppointment();
	afx_msg void OnMarkNoShow();
	afx_msg void OnMarkShow();
	afx_msg void OnMarkConfirmed();
	afx_msg void OnMarkMoveUp();
	afx_msg void OnMarkUnConfirmed();
	afx_msg void OnRemoveMoveUp();
	afx_msg void OnLinkAppointment();
	afx_msg void OnIncludeModifiedDateColumns();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnIncludeCancelledDateColumns();
	afx_msg void OnRememberColumnSettings();
	afx_msg void OnColumnSizingFinishedAppointments(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnShowNoShowReason();
	afx_msg void OnNewInvAllocation();
	afx_msg void OnNewInvOrder();
	afx_msg void OnNewBill();
	afx_msg void OnEditInvAllocation();
	// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
	afx_msg void OnApptCreateCaseHistory();
	afx_msg void OnApptEditCaseHistory();
	afx_msg void OnMarkLeftMessage();
	// (j.jones 2010-09-27 11:40) - PLID 38447 - added ability to view eligibility responses
	afx_msg void OnViewEligibilityResponses();
	// (j.dinatale 2011-06-24 17:00) - PLID 30025 - Show cancelled and no show appts
	afx_msg void OnShowCancelled();
	afx_msg void OnShowNoShows();
	afx_msg void OnBtnRecallClicked();	// (j.armen 2012-02-29 10:34) - PLID 48487	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnPatientEncounter();
	void DblClickCellApptList(LPDISPATCH lpRow, short nColIndex);
	void RButtonDownApptList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishedApptList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingApptList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void RequeryFinishedApptList(short nFlags);
	void SelChangedApptList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void EditingStartingApptList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void ColumnSizingFinishedApptList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void LeftClickApptList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);// (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
	afx_msg void OnManagePaymentProfiles(); // (z.manning 2015-07-22 15:17) - PLID 67241
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPOINTMENTSDLG_H__7C4B3513_27BE_11D2_80A0_00104B2FE914__INCLUDED_)
