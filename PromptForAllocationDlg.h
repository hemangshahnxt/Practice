#if !defined(AFX_PROMPTFORALLOCATIONDLG_H__1F4ABA7B_9D20_4956_8419_AAF5DEFDE42E__INCLUDED_)
#define AFX_PROMPTFORALLOCATIONDLG_H__1F4ABA7B_9D20_4956_8419_AAF5DEFDE42E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PromptForAllocationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPromptForAllocationDlg dialog

//TES 6/13/2008 - PLID 28078 - Defined as two things that I know are different from IDCANCEL (which is the other possible return).
#define CREATE_ALLOCATION	IDYES
#define	CREATE_ORDER		IDNO

class CPromptForAllocationDlg : public CNxDialog
{
// Construction
public:
	CPromptForAllocationDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPromptForAllocationDlg)
	enum { IDD = IDD_PROMPT_FOR_ALLOCATION };
	CNxIconButton	m_nxbDismiss;
	CNxIconButton	m_nxbCreateOrder;
	CNxIconButton	m_nxbCreateAllocation;
	//}}AFX_DATA

	//TES 6/13/2008 - PLID 28078 - Set whether the buttons should be enabled.
	BOOL m_bCanCreateAllocation;
	BOOL m_bCanCreateOrder;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPromptForAllocationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPromptForAllocationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCreateAllocation();
	afx_msg void OnCreateOrder();
	afx_msg void OnDismiss();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROMPTFORALLOCATIONDLG_H__1F4ABA7B_9D20_4956_8419_AAF5DEFDE42E__INCLUDED_)
