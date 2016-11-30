// ICCPAdvSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "ICCPAdvSettingsDlg.h"
#include "NxPracticeSharedLib\ICCPUtils.h"


// CICCPAdvSettingsDlg dialog
// (z.manning 2015-09-30 13:58) - PLID 67255 - Created

IMPLEMENT_DYNAMIC(CICCPAdvSettingsDlg, CNxDialog)

CICCPAdvSettingsDlg::CICCPAdvSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CICCPAdvSettingsDlg::IDD, pParent)
{
	m_bChanged = FALSE;
	m_bRestartNeeded = FALSE;
}

CICCPAdvSettingsDlg::~CICCPAdvSettingsDlg()
{
}

void CICCPAdvSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CICCPAdvSettingsDlg, CNxDialog)
	ON_EN_KILLFOCUS(IDC_ICCP_COM_PORT, &CICCPAdvSettingsDlg::OnEnKillfocusIccpComPort)
	ON_EN_KILLFOCUS(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE, &CICCPAdvSettingsDlg::OnEnKillfocusIccpDeviceProperitesOverride)
	ON_EN_CHANGE(IDC_ICCP_COM_PORT, &CICCPAdvSettingsDlg::OnEnChangeIccpComPort)
	ON_EN_CHANGE(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE, &CICCPAdvSettingsDlg::OnEnChangeIccpDeviceProperitesOverride)
END_MESSAGE_MAP()


// CICCPAdvSettingsDlg message handlers

BOOL CICCPAdvSettingsDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		((CEdit*)GetDlgItem(IDC_ICCP_COM_PORT))->SetLimitText(100);
		((CEdit*)GetDlgItem(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE))->SetLimitText(1000);

		// (z.manning 2015-10-01 09:29) - PLID 67255 - Only show the device properties field for the Nextech user
		if (GetCurrentUserID() == -26) {
			GetDlgItem(IDC_ICCP_DEVICE_PROPERTIES_LABEL)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE)->ShowWindow(SW_SHOWNA);
		}

		CString strComPort = ICCP::GetComPortSetting(&g_propManager);
		CString strDeviceProps = ICCP::GetDevicePropertiesSetting(&g_propManager);

		SetDlgItemText(IDC_ICCP_COM_PORT, strComPort);
		SetDlgItemText(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE, strDeviceProps);

		m_bChanged = FALSE;
		m_bRestartNeeded = FALSE;
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CICCPAdvSettingsDlg::OnCancel()
{
	// (z.manning 2015-09-30 15:26) - PLID 67255 - Do nothing
}

void CICCPAdvSettingsDlg::OnEnKillfocusIccpComPort()
{
	try
	{
		CString strComPort;
		GetDlgItemText(IDC_ICCP_COM_PORT, strComPort);
		SetRemotePropertyText("IngenicoComPortPerMachine", strComPort, 0, g_propManager.GetSystemName());
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPAdvSettingsDlg::OnEnKillfocusIccpDeviceProperitesOverride()
{
	try
	{
		CString strDeviceProps;
		GetDlgItemText(IDC_ICCP_DEVICE_PROPERITES_OVERRIDE, strDeviceProps);
		SetRemotePropertyText("CardConnectIngenicoDevicePropsPerMachine", strDeviceProps, 0, g_propManager.GetSystemName());
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPAdvSettingsDlg::OnEnChangeIccpComPort()
{
	try
	{
		m_bChanged = TRUE;
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPAdvSettingsDlg::OnEnChangeIccpDeviceProperitesOverride()
{
	try
	{
		m_bChanged = TRUE;
		m_bRestartNeeded = TRUE;
	}
	NxCatchAll(__FUNCTION__);
}
