#pragma once

// (b.savon 2013-02-01 11:36) - PLID 54982 - Created 
#include "NexERxLicense.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

// (a.wilson 2013-04-30 16:17) - PLID 56509 - Created struct to store the prescribingIDs and user role.
struct NexERxUser{
	CArray<long, long> aryPrescribingIDs;
	EUserRoleTypes urtUserRole;

	NexERxUser()
	{
		urtUserRole = urtNone;
	}
};

class CNexERxLicense
{
private:
	
	long m_nLicensedPrescriberUsed;
	long m_nLicensedPrescriberAllowed;
	long m_nMidlevelPrescriberUsed;
	long m_nMidlevelPrescriberAllowed;
	long m_nNurseStaffUsed;
	long m_nNurseStaffAllowed;

	CString m_strAvailableLicensedPrescriberWhereClause;
	CString m_strLicensedPrescriberWhereClause;
	CString m_strInactiveLicensedPrescriberWhereClause;
	CString m_strAvailableMidlevelPrescriberWhereClause;
	CString m_strLicensedMidlevelPrescriberWhereClause;
	CString m_strInactiveMidlevelPrescriberWhereClause;
	CString m_strAvailableNurseStaffWhereClause;
	CString m_strLicensedNurseStaffWhereClause;
	CString m_strInactiveNurseStaffWhereClause;

	CString m_strLicensedPrescriberInactiveIDs;
	CString m_strLicensedPrescriberUsedIDs;
	CString m_strMidlevelPrescriberInactiveIDs;
	CString m_strMidlevelPrescriberUsedIDs;
	CString m_strNurseStaffInactiveIDs;
	CString m_strNurseStaffUsedIDs;

	NexERxUser m_nexerxUser;

public:
	CNexERxLicense(void);
	~CNexERxLicense(void);

	inline BOOL HasNexERx(){ return g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts; }

	inline long GetUsedLicensedPrescriberCount(){ return g_pLicense->GetERXLicPrescribersCountUsed(); }
	inline long GetAllowedLicensedPrescriberCount(){ return g_pLicense->GetERXLicPrescribersCountAllowed(); }
	inline long GetUsedMidlevelPrescriberCount(){ return g_pLicense->GetERXMidPrescribersCountUsed(); }
	inline long GetAllowedMidlevelPrescriberCount(){ return g_pLicense->GetERXMidPrescribersCountAllowed(); }
	inline long GetUsedNurseStaffCount(){ return g_pLicense->GetERXStaffPrescribersCountUsed(); }
	inline long GetAllowedNurseStaffCount(){ return g_pLicense->GetERXStaffPrescribersCountAllowed(); }
	inline long GetRemainingLicensedPrescriberCount(){ return GetAllowedLicensedPrescriberCount() - GetUsedLicensedPrescriberCount(); }
	inline long GetRemainingMidlevelPrescriberCount(){ return GetAllowedMidlevelPrescriberCount() - GetUsedMidlevelPrescriberCount(); }
	inline long GetRemainingNurseStaffCount(){ return GetAllowedNurseStaffCount() - GetUsedNurseStaffCount(); }

	inline BOOL RequestLicensedPrescriber(long nProviderID){ return g_pLicense->RequestERXLicPrescriber(nProviderID); }
	inline BOOL RequestMidelevelPrescriber(long nProviderID){ return g_pLicense->RequestERXMidPrescriber(nProviderID); }
	inline BOOL RequestNurseStaff(long nUserID){ return g_pLicense->RequestERXStaffPrescriber(nUserID); }
	inline BOOL DeactivateLicensedPrescriber(long nProviderID){ return g_pLicense->DeactivateERXLicPrescriber(nProviderID); }
	inline BOOL DeactivateMidlevelPrescriber(long nProviderID){ return g_pLicense->DeactivateERXMidPrescriber(nProviderID); }
	inline BOOL DeactivateNurseStaff(long nUserID){ return g_pLicense->DeactivateERXStaffPrescriber(nUserID); }

	inline void GetInactiveLicensedPrescriberIDs(CDWordArray &dwaLicensePrescriberIDs){ g_pLicense->GetInactiveERXLicPrescribers(dwaLicensePrescriberIDs); }
	inline void GetInactiveMidlevelPrescriberIDs(CDWordArray &dwaMidlevelPrescriberIDs){ g_pLicense->GetInactiveERXMidPrescribers(dwaMidlevelPrescriberIDs); }
	inline void GetInactiveNurseStaffIDs(CDWordArray &dwaNurseStaffPrescriberIDs){ g_pLicense->GetInactiveERXStaffPrescribers(dwaNurseStaffPrescriberIDs); }
	inline void GetUsedLicensedPrescriberIDs(CDWordArray &dwaLicensedPrescriberIDs){ g_pLicense->GetUsedERXLicPrescribers(dwaLicensedPrescriberIDs); }
	inline void GetUsedMidlevelPrescriberIDs(CDWordArray &dwaMidlevelPrescriberIDs){ g_pLicense->GetUsedERXMidPrescribers(dwaMidlevelPrescriberIDs); }
	inline void GetUsedNurseStaffIDs(CDWordArray &dwaNurseStaffIDs){ g_pLicense->GetUsedERXStaffPrescribers(dwaNurseStaffIDs); }

	inline CString GetInactiveLicensedPrescriberIDString(){ return m_strLicensedPrescriberInactiveIDs; }
	inline CString GetInactiveMidlevelPrescriberIDString(){ return m_strMidlevelPrescriberInactiveIDs; }
	inline CString GetInactiveNurseStaffIDString(){ return m_strNurseStaffInactiveIDs; }
	inline CString GetUsedLicensedPrescriberIDString(){ return m_strLicensedPrescriberUsedIDs; }
	inline CString GetUsedMidlevelPrescriberIDString(){ return m_strMidlevelPrescriberUsedIDs; }
	inline CString GetUsedNurseStaffIDString(){ return m_strNurseStaffUsedIDs; }

	// (a.wilson 2013-04-30 16:17) - PLID 56509 - Added accessor
	void GetNexERxUser(NexERxUser &nexerxUser);

	BOOL IsCurrentUserLicensedForNexERx();
	BOOL IsUserLicensedNexERxLicensedPrescriber(long nPersonIDToCompare);
	BOOL IsUserLicensedNexERxMidlevelPrescriber(long nPersonIDToCompare);
	BOOL IsUserLicensedNexERxNurseStaff(long nPersonIDToCompare);

	CString GetAvailableLicensedPrescriberWhereClause();
	CString GetLicensedPrescriberWhereClause();
	CString GetInactiveLicensedPrescriberWhereClause();
	CString GetAvailableMidlevelPrescriberWhereClause();
	CString GetLicensedMidlevelPrescriberWhereClause();
	CString GetInactiveMidlevelPrescriberWhereClause();
	CString GetAvailableNurseStaffWhereClause();
	CString GetLicensedNurseStaffWhereClause();
	CString GetInactiveNurseStaffWhereClause();

	CString GetLicenseStatisticsMessage();
	CString GetPrescriberStatisticsMessage();
	CString GetNurseStaffMesssage();

protected:
	void PopulateIDs();
	void PopulateLicensedPrescriberIDs();
	void PopulateMidlevelPrescriberIDs();
	void PopulateNurseStaffIDs();

	CString GetLicensePrescriberMessage();
	CString GetMidlevelPrescriberMessage();

	CSqlFragment GetConfiguredNexERxUserID();	
};
