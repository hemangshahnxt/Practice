#if !defined(AFX_REFERRALTREEDLG_H__E02A05E6_A458_433A_9FE4_B8FFFF9EC7B9__INCLUDED_)
#define AFX_REFERRALTREEDLG_H__E02A05E6_A458_433A_9FE4_B8FFFF9EC7B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReferralTreeDlg.h : header file
//
#include "ReferralSubDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CReferralTreeDlg dialog

class CReferralTreeDlg : public CNxDialog
{
// Construction
public:
	CReferralTreeDlg(CWnd* pParent);   // standard constructor

	//TODO:  m_dlgName should be removed, only left for legacy support until referraltree.cpp is phased out.
// Dialog Data
	//{{AFX_DATA(CReferralTreeDlg)
	enum { IDD = IDD_REFERRAL_SOURCE_DLG };
	CString m_dlgName;
	NxButton	m_btnReferralSpace;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReferralTreeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CReferralSubDlg* m_pReferralSubDlg;

	// Generated message map functions
	//{{AFX_MSG(CReferralTreeDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFERRALTREEDLG_H__E02A05E6_A458_433A_9FE4_B8FFFF9EC7B9__INCLUDED_)
