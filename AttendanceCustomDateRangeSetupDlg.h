#if !defined(AFX_ATTENDANCECUSTOMDATERANGESETUPDLG_H__AAE2BBAB_CD29_46F5_B083_D0C9FD407667__INCLUDED_)
#define AFX_ATTENDANCECUSTOMDATERANGESETUPDLG_H__AAE2BBAB_CD29_46F5_B083_D0C9FD407667__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttendanceCustomDateRangeSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAttendanceCustomDateRangeSetupDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

class CAttendanceCustomDateRangeSetupDlg : public CNxDialog
{
// Construction
public:
	CAttendanceCustomDateRangeSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAttendanceCustomDateRangeSetupDlg)
	enum { IDD = IDD_ATTENDANCE_CUSTOM_DATE_RANGE_SETUP };
	NxButton	m_btnMonthly;
	NxButton	m_btnWeekly;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditNumberOfTimesPerMonth;
	CNxEdit	m_nxeditNumberOfWeeks;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttendanceCustomDateRangeSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void Load();
	BOOL Validate();
	void Save();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// Generated message map functions
	//{{AFX_MSG(CAttendanceCustomDateRangeSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadPerMonth();
	afx_msg void OnRadWeekly();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTENDANCECUSTOMDATERANGESETUPDLG_H__AAE2BBAB_CD29_46F5_B083_D0C9FD407667__INCLUDED_)
