#pragma once

#include "AnalyticsLicensingConfigDlg.h"

// (r.goldschmidt 2015-04-07 13:13) - PLID 65479 - Create Manage Analytics Licenses dialog and all of its UI elements.
// CAnalyticsLicensingDlg dialog

class CAnalyticsLicensingDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAnalyticsLicensingDlg)

public:
	CAnalyticsLicensingDlg(CWnd* pParent = NULL, long m_nClientID = -1);   // standard constructor
	virtual ~CAnalyticsLicensingDlg();

	bool m_bIsTestAccount;

// Dialog Data
	enum { IDD = IDD_ANALYTICSLICENSING_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxColor m_background;

	CNxIconButton m_nxbAdd;
	CNxIconButton m_nxbEdit;
	CNxIconButton m_nxbRemove;
	CNxIconButton m_nxbClose;

	long m_nClientID;
	NXDATALIST2Lib::_DNxDataListPtr m_pListLicenses; // datalist of analytics licenses
	bool m_bShowExpired; // for checkbox to show expired

	virtual BOOL OnInitDialog();
	void SecureControls();
	void SetControls(BOOL bEnable);
	void ToggleRows();
	void UpdateLicenseCount();
	void ConfigureAnalyticsLicensing(EAnalyticsLicenseAction eAction, NXDATALIST2Lib::IRowSettingsPtr pRow = NULL);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedCheckShowexpired();
	DECLARE_EVENTSINK_MAP()
	void SelChangedListanalyticslicenses(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedListanalyticslicenses(short nFlags);
};
