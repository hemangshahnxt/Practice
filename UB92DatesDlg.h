#if !defined(AFX_UB92DATESDLG_H__43549128_E773_4405_B275_3FBE9D3B0D96__INCLUDED_)
#define AFX_UB92DATESDLG_H__43549128_E773_4405_B275_3FBE9D3B0D96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UB92DatesDlg.h : header file
//

#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CUB92DatesDlg dialog

class CUB92DatesDlg : public CNxDialog
{
// Construction
public:

	void SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue);
	void Load();

	CUB92DatesDlg(CWnd* pParent);   // standard constructor

	CBrush m_brush;

	long m_UB92SetupGroupID;
	CString m_strGroupName;

// Dialog Data
	//{{AFX_DATA(CUB92DatesDlg)
	enum { IDD = IDD_UB92_DATES_DLG };
	NxButton	m_radioBox80_4;
	NxButton	m_radioBox80_2;
	NxButton	m_radioBox86_4;
	NxButton	m_radioBox86_2;
	NxButton	m_radioBox6_4;
	NxButton	m_radioBox6_2;
	NxButton	m_radioBox45_4;
	NxButton	m_radioBox45_2;
	NxButton	m_radioBox32_4;
	NxButton	m_radioBox32_2;
	NxButton	m_radioBox17_4;
	NxButton	m_radioBox17_2;
	NxButton	m_radioBox14_4;
	NxButton	m_radioBox14_2;
	CNxStatic	m_nxstaticFormatLabel;
	CNxStatic	m_nxstaticBox14Label;
	CNxStatic	m_nxstaticBox17Label;
	CNxStatic	m_nxstaticBox3236Label;
	CNxStatic	m_nxstaticBox80Label;
	CNxStatic	m_nxstaticBox86Label;
	CNxIconButton	m_btnClose;
	// (j.jones 2009-12-22 10:06) - PLID 27131 - added controls for UB04 Box 31 usage
	CNxStatic	m_nxstaticBox31UseLabel;
	NxButton	m_radioBox31UseChargeDate;
	NxButton	m_radioBox31UseDateOfCurAcc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUB92DatesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void UpdateTable(CString BoxName, long data);

	// Generated message map functions
	//{{AFX_MSG(CUB92DatesDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UB92DATESDLG_H__43549128_E773_4405_B275_3FBE9D3B0D96__INCLUDED_)
