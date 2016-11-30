#pragma once


//TES 12/23/2008 - PLID 32145 - Created
// CEnterpriseSchedulerBlockedDlg dialog

class CEnterpriseSchedulerBlockedDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEnterpriseSchedulerBlockedDlg)

public:
	CEnterpriseSchedulerBlockedDlg(CWnd* pParent);   // standard constructor
	virtual ~CEnterpriseSchedulerBlockedDlg();

	CString m_strFeatureDescription;
	CString m_strManualBookmark;

// Dialog Data
	enum { IDD = IDD_ENTERPRISE_SCHEDULER_BLOCKED_DLG };

protected:
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbOpenManual;
	//TES 12/23/2008 - PLID 32145 - We want the buttons centered, so there's an extra Close button that's only visible
	// if the Open Manual button isn't.
	CNxIconButton m_nxbCloseCentered;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	//TES 12/23/2008 - PLID 32145 - We use our own fonts for the different labels.
	CFont m_CaptionFont;
	CFont m_MoreInfoFont;
	CFont m_FeatureDescFont;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenManual();
	afx_msg void OnCloseCentered();
};
