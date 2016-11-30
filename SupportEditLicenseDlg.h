#pragma once

#include "PatientsRc.h"
#include <NxPracticeSharedLib\SupportUtils.h> // (r.ries 2013-01-09) - PLID 54643

#define MAX_LIMIT	999 //upper limit for licenses

// CSupportEditLicenseDlg dialog

// (d.singleton 2012-01-10 17:37) - PLID 28219 - files added for new dialog

// enum for license type ID
enum eLicenseTypeID
{
	ltidLicenses = 0,
	ltidConcLicenses,
	ltidNumDoctors, 
	ltidNexsync,
	ltidPalmPilot,
	ltidNexPDA,
	ltidDatabases,
	ltidEMRProv,
	ltidIpads, // (z.manning 2012-06-12 15:54) - PLID 50878
	ltidModules, // (r.ries 2013-01-09) - PLID 54643
	ltidNxWeb, // (r.ries 2013-01-23) - PLID 54767
	ltidMaxPatients, // (r.ries 2013-01-24) - PLID 54820
	ltidMaxLogins, // (r.ries 2013-01-24) - PLID 54820
	ltidLicensedPres, //(r.farnworth 2013-07-19) PLID 57522
	ltidMidPres, //(r.farnworth 2013-07-19) PLID 57522
	ltidStaffPres, //(r.farnworth 2013-07-19) PLID 57522
	ltidNxCloudUser, // (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	ltidNxCloudVPN, // (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	ltidDictation, // (z.manning 2014-01-31 14:11) - PLID 55147
	ltidPortalProviders, // (z.manning 2015-06-17 14:35) - PLID 66278
};

//enum for action type
enum eActionType
{
	atNoAction = 0, // (r.ries 2013-02-19) - PLID 55244
	atAddOn,
	atReactivate,
	atDeactivate,
	atSwap, // (r.ries 2013-01-4) - PLID 50881
	atSetExpire // (r.ries 2013-02-4) - PLID 54956
};

class CSupportEditLicenseDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSupportEditLicenseDlg)

public:
	CSupportEditLicenseDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSupportEditLicenseDlg();

	OLE_COLOR m_Color;
	long m_nPatientID; // (r.ries 2013-01-09) - PLID 54643

	eActionType m_atType;
	eLicenseTypeID m_ltidLicenseType;
	eLicenseTypeID m_ltidLicenseTypeSwap;
	CString m_strExcludeFromSwapList; 

	bool m_bIsTestAccount;
	bool bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65097

	long m_nLicenseEditAmt;

	// Existing Bought
	long m_nBLicenses;
	long m_nBIpads;
	long m_nBConc;
	long m_nBDoctors;
	long m_nBNexSync;
	long m_nBPalm;
	long m_nBNexPDA;
	long m_nBDatabases;
	long m_nBEMRProv;
	long m_nBNxWeb;
	
	long m_nBLicPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nBMidPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nBStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nBDictation; // (z.manning 2014-01-31 14:16) - PLID 55147
	long m_nBPortalProviders; // (z.manning 2015-06-17 14:41) - PLID 66278

	// Existing Pending
	long m_nPLicenses;
	long m_nPIpads;
	long m_nPConc;
	long m_nPDoctors;
	long m_nPNexSync;
	long m_nPPalm;
	long m_nPNexPDA;
	long m_nPDatabases;
	long m_nPEMRProv;
	long m_nPNxWeb;

	long m_nPLicPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nPMidPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nPStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nPDictation; // (z.manning 2014-01-31 14:16) - PLID 55147
	long m_nPPortalProviders; // (z.manning 2015-06-17 14:41) - PLID 66278

	// Existing in-use
	long m_nIULicenses;
	long m_nIUIpads;
	long m_nIUConc;
	long m_nIUDoctors;
	long m_nIUNexSync;
	long m_nIUPalm;
	long m_nIUNexPDA;
	long m_nIUDatabases;
	long m_nIUEMRProv;
	long m_nIUNxWeb;
	
	long m_nIUNxCloudUser; // (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	long m_nIUNxCloudVPN; // (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	long m_nIUNumPatients;  // (r.ries 2013-01-24) - PLID 54820
	long m_nIULoginAttempts;  // (r.ries 2013-01-24) - PLID 54820
	long m_nIULicPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nIUMidPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nIUStaffPres; //(r.farnworth 2013-07-19) PLID 57522
	long m_nIUDictation; // (z.manning 2014-01-31 14:17) - PLID 55147
	long m_nIUPortalProviders; // (z.manning 2015-06-17 14:41) - PLID 66278

	void SetBLicenses(long nLicenses);
	void SetBIpads(long nIpadsBought); // (z.manning 2012-06-12 15:49) - PLID 50878
	void SetBConc(long nConc);
	void SetBDoctors(long nDoctors);
	void SetBNexSync(long nNexSync);
	void SetBPalm(long nPalm);
	void SetBNexPDA(long nNexPDA);
	void SetBDatabases(long nDatabases);
	void SetBEMRProv(long nEMRProv);
	void SetBNxWeb(long nNxWebBought); // (r.ries 2012-01-22) - PLID 54767
	
	void SetBLicensedPres(long nLicPresBought); //(r.farnworth 2013-07-19) PLID 57522
	void SetBMidPres(long nMidPresBought); //(r.farnworth 2013-07-19) PLID 57522
	void SetBStaffPres(long nStaffPresBought); //(r.farnworth 2013-07-19) PLID 57522
	void SetBDictation(const long nDictationBought); // (z.manning 2014-01-31 14:17) - PLID 55147
	void SetBPortalProviders(const long nPortalProviders); // (z.manning 2015-06-17 14:41) - PLID 66278

	void SetPLicenses(long nPending);
	void SetPIpads(long nIpadsPending); // (z.manning 2012-06-12 15:49) - PLID 50878
	void SetPConc(long nConc);
	void SetPDoctors(long nDoctors);
	void SetPNexSync(long nNexSync);
	void SetPPalm(long nPalm);
	void SetPNexPDA(long nNexPDA);
	void SetPDatabases(long nDatabases);
	void SetPEMRProv(long nEMRProv);
	void SetPNxWeb(long nNxWebPending); // (r.ries 2012-01-22) - PLID 54767
	
	void SetPLicensedPres(long nLicPresPending); //(r.farnworth 2013-07-19) PLID 57522
	void SetPMidPres(long nMidPresPending); //(r.farnworth 2013-07-19) PLID 57522
	void SetPStaffPres(long nStaffPresPending); //(r.farnworth 2013-07-19) PLID 57522
	void SetPDictation(const long nDictationPending); // (z.manning 2014-01-31 14:17) - PLID 55147
	void SetPPortalProviders(const long nPortalProviders); // (z.manning 2015-06-17 14:42) - PLID 66278

	void SetIULicenses(long nLicenses);
	void SetIUIpads(long nIpadsInUse); // (z.manning 2012-06-12 15:50) - PLID 50878
	void SetIUConc(long nConc);
	void SetIUDoctors(long nDoctors);
	void SetIUNexSync(long nNexSync);
	void SetIUPalm(long nPalm);
	void SetIUNexPDA(long nNexPDA);
	void SetIUDatabases(long nDatabases);
	void SetIUEMRProv(long nEMRProv);
	void SetIUNxWeb(long nNxWebInUse); // (r.ries 2012-01-22) - PLID 54767
	void SetIUNumPatients(long nNumPatientsInUse);  // (r.ries 2013-01-24) - PLID 54820
	void SetIULoginAttempts(long nLoginAttemptsInUse);  // (r.ries 2013-01-24) - PLID 54820
	
	void SetIUNxCloudVPN(long nNxCloudVPN);// (s.tullis 2013-11-15 10:48) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	void SetIUNxCloudUsers(long nNxCloudUsers);// (s.tullis 2013-11-15 10:48) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	void SetIULicensedPres(long nLicPresInUse); //(r.farnworth 2013-07-19) PLID 57522
	void SetIUMidPres(long nMidPresInUse); //(r.farnworth 2013-07-19) PLID 57522
	void SetIUStaffPres(long nStaffPresInUse); //(r.farnworth 2013-07-19) PLID 57522
	void SetIUDictation(const long nDictationInUse); // (z.manning 2014-01-31 14:18) - PLID 55147
	void SetIUPortalProviders(const long nPortalProviders); // (z.manning 2015-06-17 14:42) - PLID 66278

	//function to set the upper limit of the spin control using the currently selected license in the datalist
	void SetUpperLimit();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void RefreshHelpText();

	void RefreshLicenseLockoutControls(); // (b.eyers 2015-03-05) - PLID 65100 

	CStringArray m_ModuleIDs; // (r.ries 2013-01-09) - PLID 54643

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_AddOn;
	NxButton m_Reactivate;
	NxButton m_Deactivate;
	NxButton m_SwapLicense;
	CNxStatic m_HelpText;
	CSpinButtonCtrl m_LicenseAdjust;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLicenseType;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLicenseTypeSwap;
	NXTIMELib::_DNxTimePtr m_pModuleExpDate; // (r.ries 2013-02-4) - PLID 54956
	long m_nNewLicenseValue;
	BOOL m_bInvalid;
	BOOL m_bRedHelpText; // (b.eyers 2015-03-05) - PLID 65099

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAddOn();
	afx_msg void OnBnClickedReactivate();
	afx_msg void OnBnClickedDeactivate();
	afx_msg void OnBnClickedEnableExpiration(); // (r.ries 2013-02-4) - PLID 54956
	afx_msg void OnBnClickedNoExpiration(); // (r.ries 2013-02-4) - PLID 54956
	afx_msg void OnBnClickedSetExpiration(); // (r.ries 2013-02-4) - PLID 54956
	afx_msg void OnBtnClearLockoutPeriod(); // (b.eyers 2015-03-05) PLID 65101 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_EVENTSINK_MAP()
	void SelChosenLicenseType(LPDISPATCH lpRow);
	void SelChangingLicenseType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void CPopulateLicenseTypeDataList(NXDATALIST2Lib::_DNxDataListPtr p_dlLicenseType); // (r.ries 2013-01-09) - PLID 54643
	void PopulateLicenseTypeSwapDataList(NXDATALIST2Lib::_DNxDataListPtr p_dlLicenseType); // (r.ries 2013-01-09) - PLID 54643
	void OnEnChangeLicenseAmount(); // (r.ries 2013-01-09) - PLID 54643
	void OnBnClickedSwapLicense(); // (r.ries 2013-01-09) - PLID 54643
	void SelChangingLicenseTypeSwap(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenLicenseTypeSwap(LPDISPATCH lpRow);
	void OpenMultiselectAndFilterModules(CString strWhere); // (r.ries 2013-01-09) - PLID 54643
	CString ActionEnumToString(eActionType nAction); // (r.ries 2013-01-09) - PLID 54643
	CString HelpActionEnumToString(eActionType nAction); // (r.ries 2013-01-09) - PLID 54643
	BOOL PerformModuleAction(); // (r.ries 2013-01-09) - PLID 54643
	void EnableControls(); // (r.ries 2013-01-23) - PLID 54767 -  consolidate enable/disable
	void OpenAddonModulesMultiselect(); // (r.ries 2013-01-25) - PLID 54643
	void OpenReactivateModulesMultiselect(); // (r.ries 2013-01-25) - PLID 54643
	void OpenDeactivateModulesMultiselect(); // (r.ries 2013-01-25) - PLID 54643
	void OnBnClickedExpDateAction(); // (r.ries 2013-02-4) - PLID 54956
	void OnBnClickedSetModExpDate(); // (r.ries 2013-02-4) - PLID 54956
	void OnBnClickedClearModExpDate(); // (r.ries 2013-02-4) - PLID 54956
	void OnEnKillfocusLicenseAmount(); // (r.ries 2013-02-4) - PLID 54956
	void KillFocusModuleExpDate(); // (r.ries 2013-02-4) - PLID 54956
};
