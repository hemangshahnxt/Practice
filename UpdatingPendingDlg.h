#if !defined(AFX_UPDATINGPENDINGDLG_H__2658DE75_D369_460B_AAFA_4AE82117751C__INCLUDED_)
#define AFX_UPDATINGPENDINGDLG_H__2658DE75_D369_460B_AAFA_4AE82117751C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatingPendingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatingPendingDlg dialog

class CUpdatingPendingDlg : public CNxDialog
{
// Construction
public:
	CUpdatingPendingDlg(CWnd* pParent);   // standard constructor

	int nUpdateType; //1=Out, 2=NoShow;
	void SetProgressMessage(int nCurrentPos, int nMax);

// Dialog Data
	//{{AFX_DATA(CUpdatingPendingDlg)
	enum { IDD = IDD_UPDATING_PENDING };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticUpdatePendingProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatingPendingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdatingPendingDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATINGPENDINGDLG_H__2658DE75_D369_460B_AAFA_4AE82117751C__INCLUDED_)
