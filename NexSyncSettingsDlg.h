#pragma once

// CNexSyncSettingsDlg dialog
// (z.manning 2009-08-26 14:11) - PLID 35345 - Created

#include "NexSyncConnectionSettingsDlg.h"
#include "NexSyncGoogleConnectionSettingsDlg.h"

enum TwoWaySyncValue;


class CNexSyncSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexSyncSettingsDlg)

public:
	CNexSyncSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CNexSyncSettingsDlg();

// Dialog Data
	enum { IDD = IDD_NEXSYNC_SETTINGS };

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlUsers;
	enum EUserComboColumns {
		uccID = 0,
		uccName,
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pdlLocations;
	enum ELocationComboColumns {
		lccID = 0,
		lccName,
	};

	BOOL m_bChanged;
	BOOL m_bNewProfile;
	NXDATALIST2Lib::IRowSettingsPtr m_prowLastSelectedUser;
	
	// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connectiono settings dlg
	// (d.singleton 2013-26-03 14:44) - PLID 55377
	CNexSyncGoogleConnectionSettingsDlg m_dlgGoogleConnectionSettings;

	CArray<long,long> m_arynApptTypes;
	CArray<long,long> m_arynResources;

	void LoadCurrentProfile();
	void LoadNewProfile();
	BOOL ValidateAndSaveCurrentProfile(LPDISPATCH lpUserRow);

	void CreateNewNexSyncUser();

	void EnableControls(BOOL bEnable);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void UpdateConnectionsSettingsButton(); // (z.manning 2009-10-08 16:27) - PLID 35574
	BOOL AreConnectionSettingsValid(); // (z.manning 2009-10-08 16:38) - PLID 35574

	// (z.manning 2009-10-28 10:25) - PLID 36066
	BOOL HandleLicenseCheck();

	void UpdateSyncTypeRadios(TwoWaySyncValue eTwoWaySync); // (z.manning 2011-04-04 11:59) - PLID 38193

	CNxColor m_nxcolorBackground;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnEditApptTypes;
	CNxIconButton m_btnEditResources;
	CNxIconButton m_btnEditSubjectLine;
	CNxIconButton m_btnNewUser;
	CNxIconButton m_btnDeleteUser;
	CNxIconButton m_btnConnectionSettings; // (z.manning 2009-10-08 16:29) - PLID 35574
	CNxEdit m_nxeditPastDays;
	CNxEdit m_nxeditSubjectLine;
	CNxEdit m_nxeditApptTypes;
	CNxEdit m_nxeditResources;
	CNxEdit m_nxeditInterval;
	CNxEdit m_nxeditImportChar;

	// (b.savon 2012-05-07 17:34) - PLID 37288 - Handle notes (or body of the email) formatting
	CNxIconButton m_btnEditNotesSection;
	CNxEdit m_nxeditNotesSection;

	BOOL m_bUseGoogleAPI;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void SelChosenNexsyncProfileList(LPDISPATCH lpRow);
	void RequeryFinishedNexsyncProfileList(short nFlags);
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeNexsyncPastDays();
	void SelChosenNexsyncLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnEnChangeNexsyncImportChar();
	afx_msg void OnBnClickedEditNexsyncTypes();
	afx_msg void OnBnClickedEditNexsyncResources();
	afx_msg void OnBnClickedNexsyncImportCharHelp();
	afx_msg void OnBnClickedNewNexsyncUser();
	afx_msg void OnBnClickedDeleteNexsyncUser();
	afx_msg void OnNewNexSyncProfile();
	afx_msg void OnTransferNexPdaProfile(); // (z.manning 2009-10-06 12:42) - PLID 35801
	afx_msg void OnBnClickedNexsyncConnectionSettings();
	afx_msg void OnBnClickedEditNexsyncSubjectLine(); // (z.manning 2009-10-19 16:16) - PLID 35997
	afx_msg void OnBnClickedNexsyncAllApptTypes(); // (z.manning 2010-10-22 12:14) - PLID 36017
	afx_msg void OnBnClickedNexsyncTwoWay(); // (z.manning 2011-04-04 11:43) - PLID 38193
	afx_msg void OnBnClickedNexsyncOneWay(); // (z.manning 2011-04-04 11:43) - PLID 38193
	afx_msg void OnBnClickedNexsyncCreationOnly(); // (z.manning 2014-01-28 12:06) - PLID 60016
	afx_msg void OnBnClickedEditNexsyncNotesSection(); // (b.savon 2012-05-07 17:35) - PLID 37288
	afx_msg void OnBnClickedNexsyncClearAll(); // (z.manning 2014-05-30 16:39) - PLID 58352
};
