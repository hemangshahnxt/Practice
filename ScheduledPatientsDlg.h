#if !defined(AFX_SCHEDULEDPATIENTSDLG_H__16FA4227_2F4E_4786_82B0_B111A00FC632__INCLUDED_)
#define AFX_SCHEDULEDPATIENTSDLG_H__16FA4227_2F4E_4786_82B0_B111A00FC632__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScheduledPatientsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScheduledPatientsDlg dialog

class CScheduledPatientsDlg : public CNxDialog
{
	friend class CNxSchedulerDlg;

// Construction
public:
	CScheduledPatientsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScheduledPatientsDlg)
	enum { IDD = IDD_SCHEDULED_PATIENTS_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScheduledPatientsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CScheduledPatientsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDownScheduledPatientList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

////////////////////////////////////////////////////

// Interface to CNxSchedulerDlg only
protected:
	long AddAppointment(long nApptID, COleDateTime dtStartTime, long nPatientID, const CString &strPatientName, BOOL bIsPending);
	void DeleteAppointment(long nResID);
	void Clear();
	void OnMarkIn();

protected:
	NXDATALISTLib::_DNxDataListPtr m_dlScheduledPatientList;

public:
	void OnOK();
	void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULEDPATIENTSDLG_H__16FA4227_2F4E_4786_82B0_B111A00FC632__INCLUDED_)
