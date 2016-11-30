#if !defined(AFX_CEMRLockReminderDlg_H__A7C2627D_F1BC_4B6D_8D24_F6B1136F3325__INCLUDED_)
#define AFX_CEMRLockReminderDlg_H__A7C2627D_F1BC_4B6D_8D24_F6B1136F3325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRLockReminderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRLockReminderDlg dialog

enum ELockRemindField {
	lrfLastModified = 0,
	lrfInput
}; // corresponds with remote property EmnRemindField integer

class CEMRLockReminderDlg : public CNxDialog
{
// Construction
public:
	CEMRLockReminderDlg(CWnd* pParent);   // standard constructor
	// (z.manning 2008-07-30 09:59) - PLID 30883 - Added an optional parameter to merely check that
	// there's at least one unlocked EMN (which is quicker) since this dialog no longer shows the count.
	static long NumUnlockedAgedEMNs(OPTIONAL BOOL bOnlyCheckForAtLeastOne = FALSE); // returns the number of unlocked EMNs that have been aged beyond the user preference
	static CString GetRemindField(); // returns the appropriate field to filter the date on

	// (z.manning 2008-07-30 09:53) - PLID 30883 - Took out this function since we no longer display
	// the count of unlocked EMNs on this dialog.
	//void SetUnlockedAgedEMNs(long nNum); // sets m_nUnlockedAgedEMNs and updates the controls

	void UpdateText(); // (z.manning 2008-07-30 09:53) - PLID 30883

// Dialog Data
	//{{AFX_DATA(CEMRLockReminderDlg)
	enum { IDD = IDD_LOCK_REMINDER };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticStatus;
	CNxStatic	m_nxstaticAged;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRLockReminderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	

protected:
	// (z.manning 2008-07-30 10:16) - PLID 30883 - No longer show the count of unlocked EMNs on this dialog.
	//long m_nUnlockedAgedEMNs; // set this before creating the window to prevent running the function again
	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CEMRLockReminderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnManage();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEMRLockReminderDlg_H__A7C2627D_F1BC_4B6D_8D24_F6B1136F3325__INCLUDED_)
