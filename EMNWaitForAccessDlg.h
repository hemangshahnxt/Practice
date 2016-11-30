#pragma once

#include "WriteTokenInfo.h"

// (j.jones 2013-05-16 14:35) - PLID 56596 - changed EMN.h to a forward declare
class CEMN;

/////////////////////////////////////////////////////////////////////////////
// CEMNWaitForAccessDlg dialog
// (a.walling 2008-07-03 12:58) - PLID 30498 - Dialog to show waiting for user to save/release write access

class CEMNWaitForAccessDlg : public CNxDialog
{
// Construction
public:
	CEMNWaitForAccessDlg(CWnd* pParent);   // standard constructor
	// (j.jones 2013-05-16 14:40) - PLID 56596 - added a destructor
	~CEMNWaitForAccessDlg();

	void SetInfo(long nID, const CString &strDescription, const CWriteTokenInfo& wtInfo);

// Dialog Data
	//{{AFX_DATA(CEMNWaitForAccessDlg)
	enum { IDD = IDD_EMN_FORCE_ACCESS_WAIT_DLG };
	CNexTechIconButton	m_nxibForceNow;
	CNxStatic	m_nxstaticTimer;
	CNxStatic	m_nxstaticInfo;
	CNxIconButton	m_nxibOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMNWaitForAccessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long	m_nEmnID;
	// (j.jones 2013-05-16 14:39) - PLID 56596 - converted to be a reference so we don't need to include a .h for it
	CWriteTokenInfo &m_wtInfo;
	CString m_strDescription;
	COleDateTime m_dtInitialTime;

	void UpdateTimer();

	// Generated message map functions
	//{{AFX_MSG(CEMNWaitForAccessDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnForceNow();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};