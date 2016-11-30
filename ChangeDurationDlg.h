#if !defined(AFX_CHANGEDURATIONDLG_H__8B23B5AC_08F0_4CB7_AE6C_815466A6E8A4__INCLUDED_)
#define AFX_CHANGEDURATIONDLG_H__8B23B5AC_08F0_4CB7_AE6C_815466A6E8A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeDurationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeDurationDlg dialog
#include "administratorrc.h"

class CChangeDurationDlg : public CNxDialog
{
// Construction
public:
	CChangeDurationDlg(CWnd* pParent);   // standard constructor
	int Open(long nDefaultValue, long nMinimumValue);
	long GetDefaultDuration();
	long GetMinimumDuration();

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CChangeDurationDlg)
	enum { IDD = IDD_CHANGE_DURATION };
	CNxEdit	m_nxeditEditDefault;
	CNxEdit	m_nxeditEditMinimum;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeDurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nDefault;
	long m_nMinimum;

	// Generated message map functions
	//{{AFX_MSG(CChangeDurationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEDURATIONDLG_H__8B23B5AC_08F0_4CB7_AE6C_815466A6E8A4__INCLUDED_)
