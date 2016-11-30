#if !defined(AFX_ATTENDANCEDLG_H__80D6A77B_21F0_4436_9531_97DC2A537B89__INCLUDED_)
#define AFX_ATTENDANCEDLG_H__80D6A77B_21F0_4436_9531_97DC2A537B89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttendanceDlg.h : header file
//

#include "AttendanceUtils.h"


enum EDepartmentColumns
{
	dcID = 0,
	dcName,
};

enum EUserListColumns
{
	ulcID = 0,
	ulcUserName,
};

/////////////////////////////////////////////////////////////////////////////
// CAttendanceDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

class CAttendanceDlg : public CNxDialog
{
	friend class CConfigureDepartmentsDlg;
	friend class CDepartmentsAssignUsersDlg;

// Construction
public:
	CAttendanceDlg(CWnd* pParent);   // standard constructor
	~CAttendanceDlg();

	// (z.manning, 02/12/2008) - PLID 28885 - Added buttons to lock payroll and to filter the date range on the current pay period.
// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CAttendanceDlg)
	enum { IDD = IDD_ATTENDANCE };
	NxButton	m_btnCustom;
	NxButton	m_btnYear;
	NxButton	m_btnMonth;
	NxButton	m_btnDay;
	CNxIconButton	m_btnDateCurrentPayPeriod;
	CNxIconButton	m_btnLockPayroll;
	CNxIconButton	m_btnAttendanceOptions;
	CNxIconButton	m_btnResetDateFilter;
	CNxIconButton	m_btnEditCustomDateRange;
	CNxIconButton	m_btnRequestVacation;
	CNxIconButton	m_btnUserSetup;
	CNxIconButton	m_btnConfigureDepartments;
	CNxIconButton	m_btnMoveNextMonth;
	CNxIconButton	m_btnMovePreviousMonth;
	CDateTimePicker	m_dtpDateFilterStart;
	CDateTimePicker	m_dtpDateFilterEnd;
	CNxEdit	m_nxeditAttendanceYear;
	CNxIconButton	m_btnNextUser;
	CNxIconButton	m_btnPreviousUser;
	//}}AFX_DATA
	
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttendanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlAttendance;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUsers;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDepartments;

	AttendanceInfo m_AttendanceInfo;

	AttendanceData *m_pClippedAttendanceData;

	long m_nLastSelectedDepartmentID;
	long m_nLastSelectedUserID;

	CArray<long,long> m_arynSelectedUserIDs;

	BOOL m_bInitialLoad;

	// (z.manning, 02/27/2008) - PLID 28295 - We store whether or not the current user is a manager.
	BOOL m_bIsCurrentUserManager;

	void LoadAttendanceList(BOOL bEnsureCurrentDateVisible);

	void RefilterUserList();

	COleDateTime GetDateFromAttendanceRow(LPDISPATCH lpRow);
	void UpdateSelectedUserIDs();

	void AddDynamicColumns();
	void RemoveDynamicColumns();

	void EnsureDateVisible(COleDateTime dtDate);

	void EnsureValidDateFilter();
	void HandleDateFilterChange();

	// (z.manning, 11/28/2007) - The following functions are used to add rows and row information to
	// the main attendance list.
	void AddRowsToDatalist_Daily();
	void AddRowsToDatalist_Monthly();
	void AddRowsToDatalist_Yearly();
	void AddRowsToDatalist_Custom();
	void AddHeaderFooterRows();
	void SetAttendanceRowDefaults(short nColumnCount, NXDATALIST2Lib::IRowSettingsPtr pRow);

	void AddAttendanceDataToDatalist(AttendanceUser *pUser);

	void RefreshAttendanceInfoByUser(long nUserID);

	void GetAllHighlightedRows(OUT CArray<LPDISPATCH,LPDISPATCH> &arylpRows);

	short GetUserStartColumn(const short nRelativeToColumnIndex);
	AttendanceUser* GetAttendanceUserFromColumn(const short nColumnIndex);

	void UpdateCellColor(LPDISPATCH lpRow, short nCol, AttendanceData *pData);

	// (z.manning 2010-09-09 12:14) - PLID 40456
	int OpenAttendanceEntryDlg(AttendanceData *pData, BOOL bRefreshOnOk = TRUE, OUT AttendanceData *pAttendanceEntryData = NULL);

	// Generated message map functions
	// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
	// (z.manning 2008-12-04 11:07) - PLID 32333 - Added handlers for the user combo left/right buttons.
	//{{AFX_MSG(CAttendanceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMoveNextYear();
	afx_msg void OnMovePreviousYear();
	afx_msg void OnRadioDay();
	afx_msg void OnRadioMonth();
	afx_msg void OnRadioCustom();
	afx_msg void OnRadioYear();
	afx_msg void OnLeftClickAttendanceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedDepartmentFilter(short nFlags);
	afx_msg void OnSelChosenDepartmentFilter(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedAttendanceUserList(short nFlags);
	afx_msg void OnConfigureDepartments();
	afx_msg void OnSelChosenAttendanceUserList(LPDISPATCH lpRow);
	afx_msg void OnUserSetup();
	afx_msg void OnRequestVacation();
	afx_msg void OnEditCustomDateRange();
	afx_msg void OnChangeDateFilterStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDateFilterEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnResetDateFilter();
	afx_msg void OnRButtonDownAttendanceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingAttendanceList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingAttendanceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedAttendanceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDestroy();
	afx_msg void OnAttendanceOptions();
	afx_msg void OnDblClickCellAttendanceList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnLockPayroll();
	afx_msg void OnDateCurrentPayPeriod();
	// (b.cardillo 2008-06-30 14:03) - PLID 30529 - Changed it to use the new ClosedUp event ordinal and parameter list
	afx_msg void OnClosedUpDepartmentFilter(LPDISPATCH lpSel);
	afx_msg void OnClosedUpAttendanceUserList(LPDISPATCH lpSel);
	afx_msg void OnKillfocusAttendanceYear();
	afx_msg void OnMoveNextUser();
	afx_msg void OnMovePreviousUser();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAttendanceTodayBtn(); // (z.manning 2010-09-03 17:38) - PLID 40046
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTENDANCEDLG_H__80D6A77B_21F0_4436_9531_97DC2A537B89__INCLUDED_)
