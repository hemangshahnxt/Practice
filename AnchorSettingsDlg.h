#pragma once

// (j.armen 2014-07-09 17:30) - PLID 58034 - Created

#pragma push_macro("foreach")
#undef foreach
#define BOOST_BIMAP_DISABLE_SERIALIZATION
#include <boost/bimap.hpp>
#pragma pop_macro("foreach")

class CAnchorSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAnchorSettingsDlg)
	enum { IDD = IDD_ANCHOR_SETTINGS_DLG };

public:
	CAnchorSettingsDlg(CWnd* pParent);
	~CAnchorSettingsDlg();

protected:
	BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;

private:
	void RefreshSettings();
	void LoadCallers();
	void RefreshOverrideUI();
	void EnsureButtonState();
	bool SaveSettings();
	void SaveOverrides();
	bool VerifyPassword(NXDATALIST2Lib::IRowSettingsPtr pRow, long nUserID);

	inline bool IsNextechAdmin() { return GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID; }

private:
	bool m_bHasChanges = false;
	bool m_bAddingNew = false;
	long m_nMaxCallers = -1;
	long m_nAnchorKey = 0;
	IUnknownPtr m_pStatus;
	boost::bimap<long, CiString> m_mapUsers;
	boost::bimap<long, CiString> m_mapSubdomains;	// (j.armen 2014-09-23 12:12) - PLID 63758 - Map for subdomain combo

protected:
	CNxColor m_nxColor1;
	CNxColor m_nxColor2;
	CNxColor m_nxColor3;
	NxButton m_checkEnabled;
	NxButton m_checkConnected;
	NxButton m_checkAffirmed;
	CNxLabel m_nxlStatus;
	CNxIconButton m_btnRefresh;
	CNxLabel m_nxlCallerCount;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCallers;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnRemove;
	CNxIconButton m_btnSetPracticeUserPassword;
	NxButton m_checkShowPasswords;
	NxButton m_checkOverride;
	CNxEdit m_editAnchorKey;
	CNxEdit m_editAnchorPassword;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAnchorSettingsRefresh();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedAnchorSettingsEnabled();
	afx_msg void OnBnClickedAddCaller();
	afx_msg void OnBnClickedRemoveCaller();
	afx_msg void OnBnClickedAnchorSetPracticeUserPassword();
	afx_msg void OnBnClickedShowCallerPasswords();
	afx_msg void OnBnClickedAnchorOverride();
	afx_msg void OnEnKillfocusAnchorKey();
	afx_msg void OnEnKillfocusAnchorPassword();

	DECLARE_EVENTSINK_MAP()
	void ShowContextMenuAnchorCallers(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue);
	void SelChangedAnchorCallers(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void EditingFinishedAnchorCallers(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingAnchorCallers(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};