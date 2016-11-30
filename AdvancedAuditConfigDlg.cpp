// AdvancedAuditConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "AdvancedAuditConfigDlg.h"
#include "ConfigExcludableAuditEventsDlg.h"

// (a.walling 2009-12-18 10:46) - PLID 36626

// CAdvancedAuditConfigDlg dialog

IMPLEMENT_DYNAMIC(CAdvancedAuditConfigDlg, CNxDialog)

CAdvancedAuditConfigDlg::CAdvancedAuditConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvancedAuditConfigDlg::IDD, pParent)
{

}

CAdvancedAuditConfigDlg::~CAdvancedAuditConfigDlg()
{
}

void CAdvancedAuditConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXCOLORCTRL_AUDITCFG, m_nxcolor);
	DDX_Control(pDX, IDC_IPADDRESS_SYSLOG, m_ipaddress);
	DDX_Control(pDX, IDC_CHECK_SYSLOG, m_checkEnableSyslog);
	DDX_Control(pDX, IDC_LABEL_SYSLOG_IP, m_labelSyslogIP);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDC_BTN_CONFIG_AUDIT_EVENTS, m_btnConfigEvents);
}


BEGIN_MESSAGE_MAP(CAdvancedAuditConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CONFIG_AUDIT_EVENTS, OnBtnConfigAuditEvents)
END_MESSAGE_MAP()



// CAdvancedAuditConfigDlg message handlers

BOOL CAdvancedAuditConfigDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		// (j.jones 2010-01-07 16:32) - PLID 35778 - added ability to configure audit events
		m_btnConfigEvents.AutoSet(NXB_MODIFY);

		// load
		m_checkEnableSyslog.SetCheck(GetRemotePropertyInt("RelayAuditingToSyslog", FALSE, 0, "<None>", true) ? BST_CHECKED : BST_UNCHECKED);
		m_ipaddress.SetAddress((DWORD)GetRemotePropertyInt("RelayAuditingToSyslog_IP", 0, 0, "<None>", true));

	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CAdvancedAuditConfigDlg::OnOK()
{
	try {
		// save

		CString strMessages;

		// syslog configuration
		{
			BOOL bEnabled = m_checkEnableSyslog.GetCheck() == BST_CHECKED ? TRUE : FALSE;

			DWORD dwAddress = 0;
			if (4 != m_ipaddress.GetAddress(dwAddress) || dwAddress == 0) {
				if (bEnabled) {
					// don't allow saving an invalid address if enabled
					MessageBox("This IP address is not valid. All octects of the IP address must be filled, and the address may not be 0.0.0.0.", NULL, MB_ICONSTOP);
					return;
				}
			}

			BOOL bOldEnabled = GetRemotePropertyInt("RelayAuditingToSyslog", FALSE, 0, "<None>", true) ? TRUE : FALSE;
			DWORD dwOldAddress = (DWORD)GetRemotePropertyInt("RelayAuditingToSyslog_IP", 0, 0, "<None>", true);

			if (bOldEnabled != bEnabled) {
				SetRemotePropertyInt("RelayAuditingToSyslog", bEnabled, 0, "<None>");

				strMessages += "Enabling and disabling logging to a syslog server will take effect after Practice is restarted.\r\n\r\n";
			}
			
			if (dwOldAddress != dwAddress) {
				SetRemotePropertyInt("RelayAuditingToSyslog_IP", (long)dwAddress, 0, "<None>");
			}
		}

		strMessages.TrimRight("\r\n");
		if (!strMessages.IsEmpty()) {
			MessageBox(strMessages, NULL, MB_ICONINFORMATION);
		}

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-01-07 16:32) - PLID 35778 - added ability to configure audit events
void CAdvancedAuditConfigDlg::OnBtnConfigAuditEvents()
{
	try {

		//there is one permission checked in order to enter the AdvancedAuditConfigDlg,
		//so we do not need to check it a second time to open this dialog

		CConfigExcludableAuditEventsDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}
