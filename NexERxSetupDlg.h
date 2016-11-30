#pragma once
#include "PracticeRc.h"
#include "NexERxLicense.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

// CNexERxSetupDlg dialog
// (b.savon 2013-01-11 10:00) - PLID 54578 - Created

enum EProviderRoleColumns{
	prcPersonID = 0,
	prcProviderName,
	prcProviderRoleID,
};

enum EUserRoleColumns{
	urcID = 0,
	urcUserName,
	urcUserType,
	urcLicensedPrescriberID,
	urcLicensedPrescriber,
	urcMidlevelPrescriberID,
	urcMidlevelPrescriber,
	urcSupervisorIDs,
	urcSupervisor,
	urcPrescriberIDs,
	urcPrescribers,
};

enum EProviderRoleTypes{
	prtNone = -1,
	prtLicensedPrescriber = 0,
	prtMidlevelPrescriber,
};

struct LicensedPrescriberReturn{
	BOOL	bCancelled;
	int		nID;
	CString	strFullName;
};

struct MidlevelPrescriberReturn{
	BOOL	bCancelled;
	int		nID;
	CString	strFullName;	
};

struct MultiMidlevelPrescriberReturn{
	BOOL	bCancelled;
	CString	strIDs;
	CString	strFullNames;	
};

struct SupervisingReturn{
	BOOL	bCancelled;
	CString	strIDs;
	CString	strNames;
};

struct UserCommitInfo{
	UserCommitInfo()
	{
		bInvalid = FALSE;
		nUserPersonID = -1;
		urtUserRole = urtNone;
		nLicensedPrescriberID = -1;
		nMidlevelID = -1;
		strUserName = "";
	}
	BOOL			bInvalid;
	long			nUserPersonID;
	EUserRoleTypes	urtUserRole;
	long			nLicensedPrescriberID;
	long			nMidlevelID;
	CString			strUserName;
	CSimpleArray<long> aryMidlevel;
	CSimpleArray<long> arySupervising;
};

class CNexERxSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxSetupDlg)

private:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnConfigureNexERxLicenses;
	
	CNxColor m_nxcProviderBack;
	CNxColor m_nxcUserBack;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlProviderRole;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlUserRole;

	void LoadUserRoles();
	// (b.savon 2013-01-11 10:08) - PLID 54578 - Set the user role cell properties
	void SetUserRolesCellProperties();

	// (b.savon 2013-01-11 10:08) - PLID 54581 - Clear licensed Prescriber
	void ClearLicensedPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (b.savon 2013-01-11 10:25) - PLID 54583 - Clear midlevel
	void ClearMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void ClearSupervisor(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void ClearPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (b.savon 2013-01-11 10:09) - PLID 54581 - Put licensed Prescriber
	void PutLicensedPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow, LicensedPrescriberReturn lprLicensed);
	// (b.savon 2013-01-11 10:25) - PLID 54583 - Clear midlevel
	void PutMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow, MidlevelPrescriberReturn mprLicensed);
	void PutSupervisors(NXDATALIST2Lib::IRowSettingsPtr pRow, SupervisingReturn srSupervisor);
	void PutPrescribers(NXDATALIST2Lib::IRowSettingsPtr pRow, SupervisingReturn srSupervisor, MultiMidlevelPrescriberReturn mmprMidlevel, BOOL bOverride = FALSE);

	// (b.savon 2013-01-11 10:09) - PLID 54581 - Licensed Prescriber wizard
	LicensedPrescriberReturn LicensedPrescriberWizard();
	LicensedPrescriberReturn SelectLicensedPrescriber();

	// (b.savon 2013-01-11 10:25) - PLID 54583 - Midlevel Wizard
	void MidlevelPrescriberWizard(NXDATALIST2Lib::IRowSettingsPtr pRow);
	MidlevelPrescriberReturn SelectMidlevelPrescriber();
	SupervisingReturn SelectSupervisors(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (b.savon 2013-01-11 10:33) - PLID 54584 - Nurse/Staff Wizard
	void NurseStaffWizard(NXDATALIST2Lib::IRowSettingsPtr pRow);
	MultiMidlevelPrescriberReturn SelectMultiMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (b.savon 2013-01-11 10:09) - PLID 54578 - Utilities
	UserCommitInfo GetCurrentUserInfo(NXDATALIST2Lib::IRowSettingsPtr pRow);
	CSqlFragment GetLinkSql(UserCommitInfo uciCurrentUser);
	BOOL GetUserPersonID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nUserPersonID);
	BOOL GetUserRole(NXDATALIST2Lib::IRowSettingsPtr pRow, EUserRoleTypes &urtUserRole);
	BOOL GetUserName(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strUserName);
	// (b.savon 2013-01-11 10:10) - PLID 54581 - Licensed Prescriber
	BOOL GetLicensedPrescriberID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nLicensedPrescriberID);
	// (b.savon 2013-01-11 10:26) - PLID 54583 - Midlevel
	BOOL GetMidlevelPrescriberID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nMidlevelID);
	BOOL GetSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CSimpleArray<long> &arySupervising);
	BOOL GetSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<long, long> &arySupervising);
	BOOL GetSupervisingNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<CString, LPCTSTR> &arySupervising);
	// (b.savon 2013-01-11 10:33) - PLID 54584 - Nurse/Staff Wizard
	BOOL GetNurseStaffPrescriberIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CSimpleArray<long> &arySupervising, CSimpleArray<long> &aryMidlevel);
	BOOL GetNurseStaffPrescriberIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<long, long> &arySupervising, CArray<long, long> &aryMidlevel);
	BOOL GetNurseStaffPrescriberNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<CString, LPCTSTR> &arySupervisingNames, CArray<CString, LPCTSTR> &aryMidlevelNames);
	void GetNurseStaffMidlevelSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strSupervising, CString &strMidlevel);
	void GetNurseStaffMidlevelSupervisingNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strSupervising, CString &strMidlevel);
	
	// (b.savon 2013-01-28 15:36) - PLID 54578
	void SaveUserRole(NXDATALIST2Lib::IRowSettingsPtr pRow, EUserRoleTypes urtCurRole);
	BOOL IsProviderAssignedToUsers(long nProviderID);

	BOOL IsEqual(long &lhs, long &rhs);
	BOOL IsFound(CSimpleArray<long> &arynIDs, long &nKey);
	void SaveUserID(long nUserID);
	BOOL CheckLicensedPrescriberProviderID(long &nLicensedPrescriberID, long &nProviderID);
	BOOL CheckMidlevelPrescriberProviderID(long &nMidlevelPrescriberID, long &nProviderID);
	BOOL CheckSupervisingProviderID(CSimpleArray<long> &arynIDs, long &nKey);
	BOOL CheckMultiMidlevelProviderID(CSimpleArray<long> &arynIDs, long &nKey);
	CArray<long, long> m_arynUserIDsToUpdate;

	void RemoveSupervisingProvider(CArray<long, long> &arynSupervisingIDs, CArray<CString, LPCTSTR> &arysSupervisingNames, long nProviderID);
	void ConstructDelimetedString(CArray<CString, LPCTSTR> &arynNames,CString &strNames, const CString &strDelim);
	void ConstructDelimetedString(CArray<long, long> &arynIDs, CString &strIDs, const CString &strDelim);

	BOOL SaveRoles(BOOL bSilent = FALSE);

	// (b.savon 2013-04-01 17:25) - PLID 54578
	long GetCountProviderRole(EProviderRoleTypes prtProviderRole);
	long GetCountUserRole(EUserRoleTypes urtUserRole);
	BOOL IsValidPrescriberSelection(long nProviderID, EUserRoleColumns urcProviderIDColumn);
	
	CNexERxLicense m_lNexERxLicense;
	void EnforceDeactivatedLicenses();
	void HandleDeactivatedPrescriber(DWORD dwProviderID);
	void UnregisterPrescriber(long nProviderID);
	BOOL IsProviderOrUserUsingLicense(long nPersonID);
	BOOL IsProviderOrUserDeactivated(long nPersonID);

public:
	CNexERxSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexERxSetupDlg();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	
	UserCommitInfo GetUserRoleInfo(long nPersonID);	
// Dialog Data
	enum { IDD = IDD_NEXERX_USER_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedNxdlUserRoles(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishedNxdlProviderRole(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void LeftClickNxdlUserRoles(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedOk();
	void EditingStartingNxdlProviderRole(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	afx_msg void OnBnClickedBtnConfigureLicenses();
	afx_msg void OnClose();
	void EditingStartingNxdlUserRoles(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};
