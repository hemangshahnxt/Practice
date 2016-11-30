#if !defined(AFX_PRINTWAITDLG_H__74D56368_1B39_4C1B_B846_82AF9D56815D__INCLUDED_)
#define AFX_PRINTWAITDLG_H__74D56368_1B39_4C1B_B846_82AF9D56815D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrintWaitDlg.h : header file
//

// (z.manning, 05/21/2008) - PLID 30050 - Created
/////////////////////////////////////////////////////////////////////////////
// CPrintWaitDlg dialog

class CPrintWaitDlg : public CDialog
{
// Construction
public:
	CPrintWaitDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrintWaitDlg)
	enum { IDD = IDD_PRINT_WAIT_DLG };
	CNxStatic	m_nxstaticLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrintWaitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrintWaitDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTWAITDLG_H__74D56368_1B39_4C1B_B846_82AF9D56815D__INCLUDED_)
