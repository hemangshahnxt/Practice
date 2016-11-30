#if !defined(AFX_ATTENDANCEENTRYDLG_H__DCA4C0A6_A552_4538_883D_BB2331D76120__INCLUDED_)
#define AFX_ATTENDANCEENTRYDLG_H__DCA4C0A6_A552_4538_883D_BB2331D76120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttendanceEntryDlg.h : header file
//

#include "AttendanceUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CAttendanceEntryDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29145 - Created
// (d.thompson 2014-02-27) - PLID 61016 - Ripped out all sick hours times entirely, they are no longer part of this dialog.

class CAttendanceEntryDlg : public CNxDialog
{
// Construction
public:
	CAttendanceEntryDlg(AttendanceInfo *pInfo, CWnd* pParent);   // standard constructor
	~CAttendanceEntryDlg();

// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CAttendanceEntryDlg)
	enum { IDD = IDD_ATTENDANCE_ENTRY };
	CDateTimePicker	m_dtpDateEnd;
	NxButton	m_btnEmailMgrs;
	NxButton	m_btnFloating;
	CNxIconButton	m_btnSave;
	CNxIconButton	m_btnRequest;
	CNxIconButton	m_btnCancel;
	CDateTimePicker	m_dtpDateStart;
	CNxEdit	m_nxeditVacationHours;
	CNxEdit	m_nxeditHoursWorked;
	CNxEdit	m_nxeditOtherHours;
	CNxEdit	m_nxeditAttendanceEntryNotes;
	CNxEdit	m_nxeditApprovedBy;
	CNxEdit	m_nxeditVacationAvailable;
	CNxEdit	m_nxeditVacationAccrued;
	CNxEdit	m_nxeditVacationUsed;
	CNxEdit	m_nxeditVacationBalance;
	CNxEdit	m_nxeditAttendanceInputDate;
	CNxStatic	m_nxstaticVacation;
	CNxStatic	m_nxstaticHoursWorked;
	CNxStatic	m_nxstaticYear;
	CNxStatic	m_nxstaticDateThroughLabel;
	//}}AFX_DATA

	int DoModal(AttendanceData *pData);

	AttendanceData* GetAttendanceData();

	// (z.manning 2010-09-09 12:53) - PLID 40456
	BOOL CheckDataForDuplicateAttendanceAppointment();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttendanceEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUsers;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlReasons;

	AttendanceInfo *m_pAttendanceInfo;
	AttendanceInfo *m_pOriginalAttendanceInfo; // (z.manning 2008-11-13 16:55) - PLID 31782
	AttendanceData *m_pAttendanceData;

	BOOL m_bReadOnly;

	BOOL IsNew();

	BOOL Validate();
	void Save(BOOL bRequesting);

	void Load();
	// (d.thompson 2013-01-24) - PLID 54811
	void SaveSettings();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void UpdateViewForMultipleDays();
	void SetReadOnly(BOOL bReadOnly);

	// (z.manning, 05/19/2008) - PLID 30105 - Returns the number of days off based on the specified time range
	int GetActualDaysOff();

	void HandleDateChange();

	// Generated message map functions.
	// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
	//{{AFX_MSG(CAttendanceEntryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedUsersCombo(short nFlags);
	afx_msg void OnSelChosenUsersCombo(LPDISPATCH lpRow);
	afx_msg void OnRequestAttendanceAppointment();
	afx_msg void OnSaveAttendanceAppointment();
	virtual void OnCancel();
	afx_msg void OnKillfocusVacationHours();
	afx_msg void OnKillfocusOtherHours();
	afx_msg void OnKillfocusUnpaidHours(); // (z.manning 2011-08-02 13:02) - PLID 44524
	afx_msg void OnChangeAttendanceDateStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusHoursWorked();
	afx_msg void OnFloatingHoliday();
	afx_msg void OnDatetimechangeAttendanceDateEnd(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTENDANCEENTRYDLG_H__DCA4C0A6_A552_4538_883D_BB2331D76120__INCLUDED_)
