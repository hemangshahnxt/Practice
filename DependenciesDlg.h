#if !defined(AFX_DEPENDENCIESDLG_H__AF12A240_AA18_46D9_9FDE_5AC87DD9A0BF__INCLUDED_)
#define AFX_DEPENDENCIESDLG_H__AF12A240_AA18_46D9_9FDE_5AC87DD9A0BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DependenciesDlg.h : header file
//

#include "practicerc.h"

/////////////////////////////////////////////////////////////////////////////
// CDependenciesDlg dialog

class CDependenciesDlg : public CNxDialog
{
// Construction
public:
	CDependenciesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDependenciesDlg)
	enum { IDD = IDD_DEPENDENCIES };
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditFilelist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDependenciesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DisplayFileDetails(const CString &file, const CString &path);
	void DisplayFileVersion();

	CString m_output;

	// Generated message map functions
	//{{AFX_MSG(CDependenciesDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEPENDENCIESDLG_H__AF12A240_AA18_46D9_9FDE_5AC87DD9A0BF__INCLUDED_)
