#pragma once

#include "NxAPI.h"
#include <NxPracticeSharedLib\AuditUtils.h>

// CEbillingClearinghouseIntegrationSetupDlg dialog

class CEbillingClearinghouseIntegrationSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEbillingClearinghouseIntegrationSetupDlg)

	enum class EProviderComboColumn
	{
		ID = 0,
		Name
	};

	enum class ELocationComboColumn
	{
		ID = 0,
		Name
	};

	NxButton m_checkEnableEbillingClearinghouseIntegration;
	NxButton m_radioUseOneLoginForAllProvidersAndLocations;
	NxButton m_radioUseADifferentLoginPerProviderPerLocation;
	NXDATALIST2Lib::_DNxDataListPtr m_ProvCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocCombo;
	CNxEdit m_nxeditSiteID;
	CNxEdit m_nxeditPortalPassword;
	CNxEdit m_nxeditFtpPassword;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	long m_nCurProviderID = -1;
	long m_nCurLocationID = -1;
	BOOL m_bLoginInfoChanged = FALSE;

public:
	CEbillingClearinghouseIntegrationSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CEbillingClearinghouseIntegrationSetupDlg();

	// Dialog Data
	enum { IDD = IDD_EBILLING_CLEARINGHOUSE_INTEGRATION_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void BulkCacheSettings();

	afx_msg void OnBnClickedClearinghouseLoginTypeRadio();
	afx_msg void OnEnChangeClearinghouseLoginUsernameEdit();
	afx_msg void OnEnChangeClearinghousePortalPasswordEdit();
	afx_msg void OnEnChangeClearinghouseFtpPasswordEdit();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChosenClearinghouseLoginProviderCombo(LPDISPATCH lpRow);
	void SelChosenClearinghouseLoginLocationCombo(LPDISPATCH lpRow);

	void EnsureControls();
	bool UseGlobalLogin();
	NexTech_Accessor::_ClearinghouseLoginPtr CreateLoginFromControls();

	BOOL Save();
	bool ValidateBeforeSaving();
	void ValidateAfterSaving();
	void SaveEnabledState();
	void SaveLoginType();
	BOOL SaveLogin();

	void Load();
	void LoadEnabledState();
	void LoadLoginType();
	void LoadDefaultProviderCombo();
	void LoadDefaultLocationCombo();
	void ResetProviderCombo();
	void ResetLocationCombo();
	NexTech_Accessor::_ClearinghouseLoginFilterPtr CreateLoginFilterFromControls();
	void LoadLogin();
};
