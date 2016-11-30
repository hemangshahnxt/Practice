#if !defined(AFX_FACILITYFEENEWSCHEDDLG_H__D1CE3A0E_2192_497D_9BCC_B3D7803DD350__INCLUDED_)
#define AFX_FACILITYFEENEWSCHEDDLG_H__D1CE3A0E_2192_497D_9BCC_B3D7803DD350__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FacilityFeeNewSchedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeNewSchedDlg dialog

class CFacilityFeeNewSchedDlg : public CNxDialog
{
// Construction
public:
	CFacilityFeeNewSchedDlg(CWnd* pParent);   // standard constructor

	long m_nHours;
	long m_nMinutes;
	COleCurrency m_cyFee;

	BOOL m_bIsFacilityFee;

	// (z.manning, 04/30/2008) - PLID 29860 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CFacilityFeeNewSchedDlg)
	enum { IDD = IDD_FACILITY_FEE_NEW_SCHED_DLG };
	CNxEdit	m_nxeditEditNewHours;
	CNxEdit	m_nxeditEditNewMinutes;
	CNxEdit	m_nxeditEditFee;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFacilityFeeNewSchedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFacilityFeeNewSchedDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnKillfocusEditHours();
	afx_msg void OnKillfocusEditMinutes();
	afx_msg void OnKillfocusEditFee();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACILITYFEENEWSCHEDDLG_H__D1CE3A0E_2192_497D_9BCC_B3D7803DD350__INCLUDED_)
