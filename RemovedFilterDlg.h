#if !defined(AFX_REMOVEDFILTERDLG_H__197DEDA8_8FDF_4E24_A831_6C01ECCE57E8__INCLUDED_)
#define AFX_REMOVEDFILTERDLG_H__197DEDA8_8FDF_4E24_A831_6C01ECCE57E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RemovedFilterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRemovedFilterDlg dialog

class CRemovedFilterDlg : public CNxDialog
{
// Construction
public:
	CRemovedFilterDlg(CWnd* pParent);   // standard constructor
	CString m_strCaption, m_strHelpBookmark, m_strHelpLocation;

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButton for Ok
// Dialog Data
	//{{AFX_DATA(CRemovedFilterDlg)
	enum { IDD = IDD_REMOVED_FILTER };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticRemovalExplanation;
	CNxIconButton m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemovedFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRemovedFilterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOVEDFILTERDLG_H__197DEDA8_8FDF_4E24_A831_6C01ECCE57E8__INCLUDED_)
