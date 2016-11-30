#pragma once

#include "WriteTokenInfo.h"

// (j.jones 2013-05-16 14:35) - PLID 56596 - changed EMN.h to a forward declare
class CEMN;

// (a.walling 2008-06-11 10:15) - PLID 22049 - Modeless alert dialog for EMR messages (created for multi-user)

/////////////////////////////////////////////////////////////////////////////
// CEMRAlertDlg dialog

class CEMRAlertDlg : public CNxDialog
{
// Construction
public:
	CEMRAlertDlg(CWnd* pParent);   // standard constructor
	~CEMRAlertDlg();

// Dialog Data
	//{{AFX_DATA(CEMRAlertDlg)
	enum { IDD = IDD_EMR_ALERT_DLG };
	NxButton	m_nxbForce;
	CNexTechIconButton	m_nxibRetry;
	CNxStatic	m_nxsAlertText;
	CNxIconButton	m_nxibSendYak;
	CNxIconButton	m_nxibCancel;
	CNxColor	m_background;
	//}}AFX_DATA

	void SetText(const CString& strText);
	void SetInfo(const CWriteTokenInfo& wtInfo, long nEmnID, BOOL bIsTemplate, const CString& strTitle, BOOL bTryAcquireWriteAccessButton);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRAlertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL CanForce();

	CString m_strText;
	CString m_strEmnTitle;

	BOOL m_bIsTemplate;
	BOOL m_bTryAcquireWriteAccess;

	long m_nEmnID;

	// (j.jones 2013-05-16 14:39) - PLID 56596 - converted to be a reference so we don't need to include a .h for it
	CWriteTokenInfo &m_wtInfo;

	// Generated message map functions
	//{{AFX_MSG(CEMRAlertDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSendYak();
	afx_msg void OnDestroy();
	afx_msg void OnRetryWriteAccess();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};