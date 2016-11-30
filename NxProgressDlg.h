#if !defined(AFX_NXPROGRESSDLG_H__D96D48B7_564E_11D3_A38D_00C04F42E33B__INCLUDED_)
#define AFX_NXPROGRESSDLG_H__D96D48B7_564E_11D3_A38D_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxProgressDlg.h : header file
//

#include "NxStandard.h"

/////////////////////////////////////////////////////////////////////////////
// CNxProgressDlg dialog

// (a.walling 2008-09-19 09:53) - PLID 28040 - NxStandard has been consolidated into Practice

// (a.walling 2008-09-19 09:59) - PLID 28040 - No longer an exported class
class CNxProgressDlg : public CDialog
{
// Construction
public:
	virtual BOOL Create(int nProcessCnt = 1, LPCTSTR strInitDesc = NULL, bool bVisible = true, CWnd *pParent = NULL);
	void SetDescription(LPCTSTR strNewDesc);
	void SetProgress(int nNewProgress, LPCTSTR strDescription = NULL);
	void ResetProgress(int nThreadCount = 1);
	void StartNextProgress(CWinThread *pThread, int nMin, int nMax, LPCTSTR strDescription = NULL);
	CNxProgressDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNxProgressDlg)
	CProgressCtrl	m_CurrentThreadProgress;
	CProgressCtrl	m_OverallProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxProgressDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nCurrentThreadIndex;
	int m_nThreadStepLength;
	int m_nThreadMinPos;
	int m_nThreadMaxPos;
	CWinThread *m_pCurrentThread;

	// Generated message map functions
	//{{AFX_MSG(CNxProgressDlg)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXPROGRESSDLG_H__D96D48B7_564E_11D3_A38D_00C04F42E33B__INCLUDED_)
