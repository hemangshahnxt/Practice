#if !defined(AFX_NXTWAINLAUNCHPICKERDLG_H__A7D56F81_ACC6_4E2C_9AAE_501DA49D2234__INCLUDED_)
#define AFX_NXTWAINLAUNCHPICKERDLG_H__A7D56F81_ACC6_4E2C_9AAE_501DA49D2234__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxTWAINLaunchPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxTWAINLaunchPickerDlg dialog

#include "PracticeRc.h"
#import "NxDataList.tlb"

class CNxTWAINLaunchPickerDlg : public CNxDialog
{
// Construction
public:
	CNxTWAINLaunchPickerDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNxTWAINLaunchPickerDlg)
	enum { IDD = IDD_NXTWAIN_LAUNCHER };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxTWAINLaunchPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSelectedPIDWindowText(const CString& str);
	DWORD GetSelectedPID();
	void SetPIDs(const CDWordArray& adwPIDs);
	CDWordArray m_adwPIDs;

protected:
	CString m_strPIDWindowText;
	DWORD m_dwSelectedPID;
	NXDATALISTLib::_DNxDataListPtr m_list;

	// Generated message map functions
	//{{AFX_MSG(CNxTWAINLaunchPickerDlg)
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnOK();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXTWAINLAUNCHPICKERDLG_H__A7D56F81_ACC6_4E2C_9AAE_501DA49D2234__INCLUDED_)
