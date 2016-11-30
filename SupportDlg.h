#pragma once

// SupportDlg.h : header file
//

#include "PatientDialog.h"
#include "SupportEditLicenseDlg.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

/////////////////////////////////////////////////////////////////////////////
// CSupportDlg dialog

//TES 6/7/2005 - These values are written to data.
enum SupportItemStatus {
	sisNotBought = 0,
	sisBought = 1,
	sisActive = 2,
	sisDeactivated = 3,
	sisActiveNotBought = 4,
};

class CSupportDlg : public CPatientDialog
{
// Construction
public:
	CSupportDlg(CWnd* pParent);   // standard constructor
	virtual void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	CString Save(int nID, const CString strPreparedAuditText="");
	void Load();
	void StoreDetails();
	long m_id;
	BOOL m_changed;

private:	// Dialog Data
	enum { IDD = IDD_SUPPORTDLG };
	CNxEdit	m_nxeditEmrProDoctorNames;
	CNxEdit	m_nxeditEmrPro;
	CNxEdit	m_nxeditBNexWebDomains;
	CNxEdit m_nxeditUNexWebDomains;
	CNxEdit m_nxeditPNexWebDomains;
	NxButton	m_btnAllowSupport;
	NxButton	m_btnLicenseActive;
	CNxEdit	m_nxeditBLic;
	CNxEdit	m_nxeditULic;
	CNxEdit	m_nxeditPLic;
	// (d.thompson 2011-10-03) - PLID 45791
	CNxEdit	m_nxeditBConcTSLic;
	CNxEdit	m_nxeditUConcTSLic;
	CNxEdit	m_nxeditPConcTSLic;
	CNxEdit	m_nxeditBDoc;
	CNxEdit	m_nxeditUDoc;
	CNxEdit	m_nxeditPDoc;
	CNxEdit	m_nxeditBPalm;
	CNxEdit	m_nxeditUPalm;
	CNxEdit	m_nxeditPPalm;
	CNxEdit	m_nxeditBPpc;
	CNxEdit	m_nxeditUPpc;
	CNxEdit	m_nxeditPPpc;
	// (z.manning 2009-10-16 15:41) - PLID 35749
	CNxEdit	m_nxeditBNexSync;
	CNxEdit	m_nxeditUNexSync;
	CNxEdit	m_nxeditPNexSync;
	CNxEdit	m_nxeditBDatabases;
	CNxEdit	m_nxeditUDatabases;
	CNxEdit	m_nxeditPDatabases;
	CNxEdit m_nxeditBLicPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditULicPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditPLicPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditBMidPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditUMidPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditPMidPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditBStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditUStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit m_nxeditPStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	CNxEdit	m_nxeditBEmrprov;
	CNxEdit	m_nxeditUEmrprov;
	CNxEdit	m_nxeditPEmrprov;
	// (z.manning 2014-01-31 12:53) - PLID 55147 - Added dictation fields
	CNxEdit m_nxeditBDictation;
	CNxEdit m_nxeditUDictation;
	CNxEdit m_nxeditPDictation;
	CNxEdit	m_nxeditMaxPatientCount;
	CNxEdit	m_nxeditMaxPracticeCount;
	CNxEdit	m_nxeditAmaVersion;
	CNxEdit	m_nxeditPurchAgr;
	CNxEdit	m_nxeditLicAgr;
	CNxEdit	m_nxeditInstalled;
	CNxEdit	m_nxeditTrainer;
	CNxEdit	m_nxeditSuppExp;
	CNxEdit	m_nxeditPracEmail;
	CNxEdit	m_nxeditRateDate;
	CNxEdit	m_nxeditLicenseKey;
	CNxEdit	m_nxeditLicenseActivatedBy;
	CNxEdit	m_nxeditSupportNotes;
	CNxEdit	m_nxeditServName;
	CNxEdit	m_nxeditServOs;
	CNxEdit	m_nxeditWsOs;
	CNxEdit	m_nxeditPathServer;
	CNxEdit	m_nxeditNxserverIp;
	// (f.dinatale 2010-05-24) - PLID 38842 - Added version history
	CNxEdit m_nxeditUpdatedon;
	CNxEdit m_nxeditUpdatedby;
	CNxEdit	m_nxeditAnchorCallerCount;
	CNxIconButton	m_btnConnectionInfo;
	CNxIconButton	m_btnThirdPartInfo;
	CNxIconButton	m_btnEditWebLogin;
	CNxIconButton	m_btnUpgradeHistory;
	// (f.dinatale 2010-07-07) - PLID 39527 - Added Lab Integration Info
	CNxIconButton	m_btnLabIntegrationInfo;
	// (f.dinatale 2010-10-25) - PLID 40827 - Added NxReminder Settings
	CNxIconButton	m_btnNxReminderSettings;	

// Overrides
	// ClassWizard generated virtual function overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_RatingCombo;
	NXDATALISTLib::_DNxDataListPtr m_pEbillingType;
	NXDATALISTLib::_DNxDataListPtr m_pItemList;
	NXDATALISTLib::_DNxDataListPtr m_pASPSForms;
	NXDATALISTLib::_DNxDataListPtr m_pEStatementType;
	NXDATALISTLib::_DNxDataListPtr m_pCurVersion;
	NXDATALISTLib::_DNxDataListPtr m_pPracticeType;
	//NXDATALISTLib::_DNxDataListPtr m_pShadowedVersion;
	//r.wilson Oct 5 2011 PLID 45836
	NXDATALISTLib::_DNxDataListPtr m_pAAOForms;


	NXTIMELib::_DNxTimePtr m_pAMAInstalled;
	NXTIMELib::_DNxTimePtr m_pASPSPurchDate;
	NXTIMELib::_DNxTimePtr m_pLicenseActivatedDate;
	//r.wilson Oct 5 2011 PLID 45836
	NXTIMELib::_DNxTimePtr m_pAAOPurchDate;

	void RequeryItemList();
	void AddRowToList(CString strField, CString strName, long nStatus, long nUsageCount, COleDateTime dtExpiration);
	 // (r.ries 2013-01-09) - PLID 54643 - Removed AuditSupporItem, moved to CSupportUtils
	BOOL AllowEMRType(long nRow, VARIANT FAR* pvarNewValue);
	
	void LoadNexCloudStatus(BOOL bValue);//l.banegas PLID 57275 load's NexCloud status. If the Usere is a NexCloud User, TRUE or FALSE 


	BOOL m_bItemsEnabled;
	BOOL m_bHideForNewClient;
	CString m_strUpdatedBy;
	CString m_strUpdatedOn;
	BOOL m_bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65097

	//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
	bool IsTestAccount();			// True if test account, false otherwise

	//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
	void EnableBoughtUsedLicenseFields(BOOL bEnable);
	bool m_bLicenseWizardUsed;
	CString m_strLicenseWizardAuditReason;

	BOOL AreIpadsEnabled(); // (z.manning 2012-06-12 17:28) - PLID 50878
	// (r.gonet 2016-05-10) - NX-100478 - Returns whether or not the client is licensed for Azure RemoteApp
	bool IsAzureRemoteAppEnabled();

	// (z.manning 2015-12-18 10:47) - PLID 67738
	int PromptForDictationInfo();

	// (b.savon 2016-04-11) - NX-100081
	bool DisallowERxLicenseChange(const CString &strField, const SupportItemStatus &sisNewStatus);

	// Generated message map functions
	//{{AFX_MSG(CSupportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShadowSetup();
	afx_msg void OnAllowSupport();
	afx_msg void OnSelChosenRating(long nNewSel);
	afx_msg void OnSupportFile();
	afx_msg void OnSelChosenEbillingType(long nRow);
	afx_msg void OnSelChangingEbillingType(long FAR* nNewSel);
	afx_msg void OnEnableSupportItems();
	afx_msg void OnEditingFinishedSupportItemsList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingSupportItemsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnViewSupportHistory();
	afx_msg void OnSelChosenAspsForms(long nRow);
	//r.wilson Oct 5 2011 PLID 45836
	afx_msg void OnSelChosenAaoForms(long nRow);
	afx_msg void OnKillFocusAspsPurchDate();
	//r.wilson Oct 5 2011 PLID 45836 
	afx_msg void OnKillFocusAaoPurchDate();
	afx_msg void OnKillFocusAmaInstalledOnDate();
	afx_msg void OnSelChosenEstatementType(long nRow);
	afx_msg void OnSelChangingEstatementType(long FAR* nNewSel);
	afx_msg void OnEditWebLogin();
	afx_msg void OnSelChosenVersionCur(long nRow);
	afx_msg void OnNewVersion();
	afx_msg void OnThirdPartyInfo();
	afx_msg void OnIsLicenseActive();
	afx_msg void OnSelChangingPracticeType(long FAR* nNewSel);
	afx_msg void OnSelChosenPracticeType(long nRow);
	afx_msg void OnKillFocusLicenseActivatedOn();
	afx_msg void OnAllowShadow();
	afx_msg void OnConnectionInfo();
	afx_msg void OnVerHistory();
	afx_msg void OnIntegration();
	// (f.dinatale 2010-10-25) - PLID 40827
	afx_msg void OnNxReminderSettings();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedConcTsListBtn();
	//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
	afx_msg void OnBnClickedChangeLicenses();
	afx_msg CString SwapLicenseWith(eLicenseTypeID ltidLicenseTypeSwap, long lamtToSwap); // (r.ries 2013-1-3) - PLID 50881
	afx_msg void OnEnableIpadsCheck(); // (z.manning 2012-06-12 16:57) - PLID 50878
	void EnableIpads(const CString& strPreparedAuditText=""); // (r.ries 2013-2-20) - PLID 55244
	// (r.gonet 2016-05-10) - NX-100478 - Added AzureRemoteApp
	afx_msg void OnAzureRemoteAppCheck();
	// (r.gonet 2016-05-10) - NX-100478 - Enables/Disables Azure RemoteApp licensing depending on the checkbox state.
	void EnableAzureRemoteApp();
	afx_msg void OnBnClickedNexcloud();
	afx_msg void OnIsSubscriptionLocalHosted(); // (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted

private:	// Dialog Data
	long m_nAnchorCallerCount = 0;	// (j.armen 2013-10-03 07:32) - PLID 57853 - Anchor Caller Count
	CString m_strAnchorPassword;	// (j.armen 2013-10-03 07:32) - PLID 57853 - Anchor Password

private:	// Dialog Event Handlers
	afx_msg void OnBnClickedChangeAnchorPassword();		// (j.armen 2013-10-03 07:32) - PLID 57853 - Anchor Password Change Dlg
	afx_msg void OnAnchorCallerCountFocusLost();	// (j.armen 2013-10-03 07:32) - PLID 57853 - Anchor Caller Count Modified

private:	// Non-Event Functions
	void PushAnchorData();	// (j.armen 2013-10-03 08:07) - PLID 57853 - Single point for any call needing up update Hawse Data
public:
	afx_msg void OnBnClickedAnalyticslicensingBtn(); // (r.goldschmidt 2015-04-06 17:31) - PLID 65479 - Manage Analytics Licenses
	afx_msg void OnBnClickedEditDictationInfo();
};