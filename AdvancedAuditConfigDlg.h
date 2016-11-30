#pragma once
#include "afxcmn.h"
#include "afxwin.h"

// (a.walling 2009-12-18 10:46) - PLID 36626 - Configuration dialog for audit options, initially just the syslog echo

// CAdvancedAuditConfigDlg dialog

class CAdvancedAuditConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdvancedAuditConfigDlg)

public:
	CAdvancedAuditConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CAdvancedAuditConfigDlg();

// Dialog Data
	enum { IDD = IDD_ADVANCED_AUDIT_CONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	CNxColor m_nxcolor;
	CIPAddressCtrl m_ipaddress;
	NxButton m_checkEnableSyslog;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;
	CNxStatic m_labelSyslogIP;
	// (j.jones 2010-01-07 16:32) - PLID 35778 - added ability to configure audit events
	CNxIconButton m_btnConfigEvents;
	afx_msg void OnBtnConfigAuditEvents();
};
