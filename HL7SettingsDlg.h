#if !defined(AFX_HL7SETTINGSDLG_H__572048CF_90C1_4D04_860F_E2876C956CB8__INCLUDED_)
#define AFX_HL7SETTINGSDLG_H__572048CF_90C1_4D04_860F_E2876C956CB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7SettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7SettingsDlg dialog

class CHL7SettingsDlg : public CNxDialog
{
// Construction
public:
	CHL7SettingsDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-04-24 14:11) - PLID 29600 - added m_checkBatchImports and m_checkBatchExports
	// (j.jones 2008-05-08 10:12) - PLID 29953 - added nxiconbuttons for modernization
	// (z.manning 2008-07-16 09:59) - PLID 30753 - Added variable for the auto export appts button
// Dialog Data
	//{{AFX_DATA(CHL7SettingsDlg)
	// (a.vengrofski 2010-05-11 09:44) - PLID <38547> - Added enum for clarity.
	enum eHL7ListCol {
		ehlID = 0,
		ehlName = 1,
	};

	enum eFormatListCol {
		eflID = 0,
		eflName = 1,
	};

	enum { IDD = IDD_HL7_SETTINGS_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnConfigLocations;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnImportChargeCodes;
	NxButton	m_chkExclusiveFTDiagLink; // (b.savon 2016-01-18 07:06) - PLID 67782
	NxButton	m_checkBatchImports;
	NxButton	m_checkBatchExports;
	NxButton	m_btnFile;
	NxButton	m_btnTcp;
	NxButton	m_btnExpectAck;
	NxButton	m_btnFileImport;
	NxButton	m_btnTcpImport;
	NxButton	m_btnSendAck;
	NxButton	m_btnUseMsgDate;
	NxButton	m_btnUseCurrentDate;
	NxButton	m_btnImportLoc;
	NxButton	m_btnImportLocPOS;
	NxButton	m_btnAutoExport;
	NxButton	m_btnAutoUpdate;
	CNxEdit	m_nxeditServerAddr;
	CNxEdit	m_nxeditServerPort;
	CNxEdit	m_nxeditServerAddrImport;
	CNxEdit	m_nxeditImportExtension;
	CNxEdit	m_nxeditServerPortImport;
	CNxEdit	m_nxeditBeginChars;
	CNxEdit	m_nxeditEndChars;
	CNxEdit	m_nxeditFilename;
	CNxEdit	m_nxeditFilenameImport;
	CNxStatic	m_nxstaticAddrText;
	CNxStatic	m_nxstaticPortText;
	CNxStatic	m_nxstaticAddrTextImport;
	CNxStatic	m_nxstaticExtensionLabel;
	CNxStatic	m_nxstaticPortTextImport;
	CNxStatic	m_nxstaticFilenameText;
	CNxStatic	m_nxstaticFilenameTextImport;
	NxButton	m_btnAutoExportAppts;
	NxButton	m_btnExportInsurance;
	NxButton	m_btnAutoExportEmnBills;
	NxButton	m_btnAutoExportLabs;// (a.vengrofski 2010-05-11 09:17) - PLID <38547> - NX-ify the button.
	CNxStatic	m_nxStaticSendingLab;// (a.vengrofski 2010-05-12 11:24) - PLID <38547>
	CNxIconButton m_nxbAdvancedLabSettings;
	NxButton	m_btnAutoImportLabs; // (z.manning 2010-05-21 10:07) - PLID 38638
	CNxIconButton	m_nxbConfigInsCos;
	CNxIconButton	m_btnConfigureResources; // (z.manning 2010-07-07 16:14) - PLID 39599
	CNxIconButton	m_nxbConfigureRelations;
	NxButton	m_btnImportInsurance;
	NxButton	m_btnSendA2831;
	NxButton	m_btnAutoImportPatsFromSiu;
	NxButton	m_btnExcludeProspects;
	NxButton	m_btnApptImportRefPhys;	// (j.dinatale 2013-01-15 16:32) - PLID 54631
	CNxIconButton m_btnConfigRefPhys;	// (j.dinatale 2013-01-15 16:32) - PLID 54631
	CNxIconButton m_btnAdvSchedSettings;
	CNxIconButton m_btnConfigInsTypes;// (d.singleton 2012-08-24 16:26) - PLID 52302
	CNxIconButton m_btnConfigInventory; // (b.eyers 2015-06-04) - PLID 66205
	NxButton m_btnEnableIntelleChart; // (b.eyers 2015-06-10) - PLID 66354
	//}}AFX_DATA

	//TES 2/23/2010 - PLID 37503 - Added subtabs for the different record types.
	NxTab::_DNxTabPtr m_tab;

	// (a.vengrofski 2010-05-27 16:54) - PLID <38918> - Added for auditing
	long m_nDefaultLabID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7SettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;
	//TES 10/10/2008 - PLID 21093 - Added the option for which "procedure" to use when the HL7 Link creates a new Lab record.
	NXDATALIST2Lib::_DNxDataListPtr m_pLabProcedureList;

	NXDATALIST2Lib::_DNxDataListPtr m_pSendingLabList;// (a.vengrofski 2010-05-12 11:21) - PLID <38547> - Defualt sending lab to associate with.
	

	void LoadSettings(long nGroupID);
	void Save(int nID);
	void EnsureConnectionOptions();

	//TES 2/23/2010 - PLID 37503 - Show/hide controls based on the currently selected sub-tab.
	void ReflectCurrentTab();

	//TES 12/12/2006 - PLID 23737 - Tell other computers (and ourself) that this group has changed.
	void RefreshGroup(long nGroupID);

	// (v.maida 2016-06-14 10:35) - NX-100833 - Created a function for enabling/disabling controls that only tech support should use.
	void EnableTechSupportControls(BOOL bEnable);

	//TES 9/15/2011 - PLID 45523 - Moved SendingFacilityID and ForceFacilityIDMatch to CHL7GeneralSettingsDlg

	// (j.jones 2008-04-24 14:17) - PLID 29600 - added OnCheckBatchImports & Exports
	// Generated message map functions
	// (z.manning 2008-12-22 17:01) - PLID 32550 - Added export insurance option
	//{{AFX_MSG(CHL7SettingsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddGroup();
	afx_msg void OnRenameGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnFileRad();
	afx_msg void OnTcpRad();
	afx_msg void OnClose();
	afx_msg void OnSelChosenHl7FormatList(long nRow);
	afx_msg void OnSelChangingHl7FormatList(long FAR* nNewSel);
	afx_msg void OnFileImportRad();
	afx_msg void OnTcpImportRad();
	afx_msg void OnBrowseFile();
	afx_msg void OnBrowseFileImport();
	afx_msg void OnExpectAck();
	afx_msg void OnUseMessageDate();
	afx_msg void OnUseCurrentDate();
	afx_msg void OnAutoExport();
	afx_msg void OnAutoUpdate();
	afx_msg void OnSendAck();
	afx_msg void OnImportLocations();
	afx_msg void OnConfigLocations();
	afx_msg void OnImportLocationsAsPos();
	afx_msg void OnCheckBatchExports();
	afx_msg void OnCheckBatchImports();
	afx_msg void OnAutoExportAppts();
	afx_msg void OnExportInsurance();
	afx_msg void OnAdvancedLabSettings();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnSelChosenDefaultLabProcedure(LPDISPATCH lpRow);
	afx_msg void OnAutoExportEmnBills();
	afx_msg void OnUseObx13();
	void OnSelectTabHl7SettingsTabs(short newTab, short oldTab);
	afx_msg void OnUseObxLocation();
	afx_msg void OnBnClickedExportLab();// (a.vengrofski 2010-05-11 12:15) - PLID <38547> - Function to export batched labs.
	afx_msg void OnBnClickedAutoImportLabResults();
	void SelChosenDefaultSendingLab(LPDISPATCH lpRow);// (a.vengrofski 2010-05-11 12:15) - PLID <38547>
	void RequeryFinishedDefaultSendingLab(short nFlags);// (a.vengrofski 2010-05-11 12:15) - PLID <38547>
	afx_msg void OnConfigInsCos();
	afx_msg void OnBnClickedHl7ConfigResources(); // (z.manning 2010-07-07 16:11) - PLID 39559
	afx_msg void OnConfigInsRelations();
	afx_msg void OnImportInsurance();
	afx_msg void OnSendA2831();
	afx_msg void OnBnClickedAutoImportPatsFromScheduleMessages(); // (z.manning 2010-08-09 10:04) - PLID 39985
	afx_msg void OnExcludeProspects();
	afx_msg void OnBnClickedAdvancedPatientSettings(); // (z.manning 2010-10-04 16:53) - PLID 40795
	afx_msg void OnBnClickedHl7AdvSchedOptions(); // (z.manning 2011-04-21 10:40) - PLID 43361
	afx_msg void OnEnableIntelleChart(); // (b.eyers 2015-06-10) - PLID 66354
public:
	afx_msg void OnAdvancedGeneralSettings();
	afx_msg void OnBnClickedConfigInsTypes();// (d.singleton 2012-08-24 16:33) - PLID 52302
	afx_msg void OnBnClickedHl7SettingsRefPhys();	// (j.dinatale 2013-01-08 10:47) - PLID 54491
	afx_msg void OnBnClickedApptImportRefPhys();	// (j.dinatale 2013-01-15 16:32) - PLID 54631
	afx_msg void OnAutoExportRefPhys();
	afx_msg void OnAutoUpdateRefPhys();
	afx_msg void OnBnClickedConfigChargeCodes();
	afx_msg void OnBnClickedConfigInventory(); // (b.eyers 2015-06-04) - PLID 66205
	afx_msg void OnBnClickedAutoExportInsurance(); // (r.goldschmidt 2015-11-05 18:24) - PLID 67517
	afx_msg void OnBnClickedChkLinkFtDiag();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7SETTINGSDLG_H__572048CF_90C1_4D04_860F_E2876C956CB8__INCLUDED_)
