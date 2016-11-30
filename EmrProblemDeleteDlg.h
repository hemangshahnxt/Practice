#if !defined(AFX_EMRPROBLEMDELETEDLG_H__92F10AA4_1AF9_44A7_934E_8EB43DA686D9__INCLUDED_)
#define AFX_EMRPROBLEMDELETEDLG_H__92F10AA4_1AF9_44A7_934E_8EB43DA686D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrProblemDeleteDlg.h : header file
//
// (c.haag 2008-07-23 16:30) - PLID 30820 - Initial implementation
//

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemDeleteDlg dialog

class CEmrProblemDeleteDlg : public CNxDialog
{
// Construction
public:
	CEmrProblemDeleteDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrProblemDeleteDlg)
	enum { IDD = IDD_EMR_PROBLEM_DELETE };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnClose;
	CNxStatic		m_staticTopCaption;
	CNxStatic		m_staticSecondCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrProblemDeleteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// (c.haag 2008-08-05 17:17) - PLID 30942 - The caption to appear at the very top of the window
	CString m_strTopCaption;
	// (c.haag 2008-08-06 10:07) - PLID 30942 - The caption to appear below the top caption
	CString m_strSecondCaption;
	// (c.haag 2008-08-06 10:09) - PLID 30942 - If this is set to TRUE, then the dialog will only have
	// a close button
	BOOL m_bCloseButtonOnly;

public:
	// (j.jones 2009-05-29 12:44) - PLID 34301 - commented out unused code
	//CArray<long,long> m_anProblemIDs;
	// (c.haag 2008-08-05 17:17) - PLID 30942 - An array of problems that exist in memory
	CArray<CEmrProblem*,CEmrProblem*> m_apProblemsToBeDeleted;
	// (j.jones 2009-05-29 11:27) - PLID 34301 - an array of problem links that will be deleted
	CArray<CEmrProblemLink*,CEmrProblemLink*> m_apProblemLinksToBeDeleted;

public:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

protected:
	CNxColor m_nxcTop;

protected:
	CWinThread* m_pPopulateDeletedProblemValuesThread;

protected:
	// (c.haag 2008-08-05 17:16) - PLID 30942 - Add problems that exist in memory
	// (j.jones 2009-05-29 11:30) - PLID 34301 - renamed & reworked the logic to support the new structure
	void AddProblemLinksFromMemory();

	// Tries to kill the active PopulateDetailValuesThread thread
	void KillThread();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrProblemDeleteDlg)
	virtual BOOL OnInitDialog();
	// (j.jones 2009-05-29 12:44) - PLID 34301 - commented out unused code
	//afx_msg void OnRequeryFinishedListEmrDeletingProblems(short nFlags);
	virtual void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPROBLEMDELETEDLG_H__92F10AA4_1AF9_44A7_934E_8EB43DA686D9__INCLUDED_)
