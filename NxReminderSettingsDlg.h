#pragma once

#include "NxReminderSOAPUtils.h"

class CNxReminderSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNxReminderSettingsDlg)

public:
	CNxReminderSettingsDlg( const CString& nLicenseKey, CWnd* pParent);
	virtual ~CNxReminderSettingsDlg();

// Dialog Data
	enum { IDD = IDD_NXREMINDER_SETTINGS_DLG };

	enum dlcNxSettingsTiers
	{
		dlSettingsID = 0,
		dlSettingsName,
		dlSettingsMessages,
		dlSettingsPrice,
	};

protected:
	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnEditTiers();
	afx_msg void OnCelltrustOk();
	afx_msg LRESULT OnDialogLoaded(WPARAM wParam, LPARAM lParam);
	
	NXDATALIST2Lib::_DNxDataListPtr m_lTiers;
	CNxRetrieveNxReminderClient m_nxremDlgInfo;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnSelChangingClientTierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};