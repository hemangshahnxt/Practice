#if !defined(AFX_ATTENDANCEUSERSETUPDLG_H__5864C2FB_B4AC_4C95_9748_6EB66A2E1C33__INCLUDED_)
#define AFX_ATTENDANCEUSERSETUPDLG_H__5864C2FB_B4AC_4C95_9748_6EB66A2E1C33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttendanceUserSetupDlg.h : header file
//

#include "AttendanceUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CAttendanceUserSetupDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29147 - Created

class CAttendanceUserSetupDlg : public CNxDialog
{
// Construction
public:
	CAttendanceUserSetupDlg(AttendanceInfo *pAttendanceInfo, CWnd* pParent);   // standard constructor
	~CAttendanceUserSetupDlg();

	void SetDefaultUserID(long nUserID);

	// (z.manning, 12/04/2007) - Public boolean that is used for parent windows to tell if something was saved here.
	BOOL m_bAtLeastOneSave;

// Dialog Data
	//{{AFX_DATA(CAttendanceUserSetupDlg)
	enum { IDD = IDD_ATTENDANCE_USER_SETUP };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	CNxLabel		m_nxlDepartments;
	CNxEdit	m_nxeditVacationAllowance;
	CNxEdit	m_nxeditVacationBonus;
	CNxEdit	m_nxeditSickAllowance;
	CNxEdit m_nxeditNotes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttendanceUserSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUsers;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUserTypes;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDepartments;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlYear;

	// (z.manning, 04/11/2008) - PLID 29628 - Added date of hire
	NXTIMELib::_DNxTimePtr m_nxtDateOfHire;

	AttendanceInfo *m_pMainAttendanceInfo;
	AttendanceInfo *m_pTempAttendanceInfo;

	CArray<long,long> m_arynDepartmentIDs;

	long m_nDefaultUserID;

	long m_nLastSelectedUserID;

	BOOL m_bChanged;

	void Load();
	BOOL Validate();
	void Save(long nUserID);

	void RefreshDepartmentDisplay();

	void HandleDepartmentMultipleSelection();

	// Generated message map functions
	//{{AFX_MSG(CAttendanceUserSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenAttendanceUserList(LPDISPATCH lpRow);
	afx_msg void OnChangeSickAllowance();
	afx_msg void OnChangeVacationAllowance();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChosenAttendanceUserType(LPDISPATCH lpRow);
	afx_msg void OnChangeEffectiveYear();
	afx_msg void OnChangeVacationBonus();
	afx_msg void OnRequeryFinishedDepartmentSelect(short nFlags);
	afx_msg void OnSelChosenDepartmentSelect(LPDISPATCH lpRow);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelChosenEffectiveYear(LPDISPATCH lpRow);
	afx_msg void OnChangedAttendanceDateOfHire();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (z.manning 2008-11-18 17:14) - PLID 32073
	afx_msg void OnEnChangeAttendanceUserNotes();
	afx_msg void OnAttendanceUserSetupOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTENDANCEUSERSETUPDLG_H__5864C2FB_B4AC_4C95_9748_6EB66A2E1C33__INCLUDED_)
