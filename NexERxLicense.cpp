#include "stdafx.h"
#include "NexERxLicense.h"
#include "NexERxSetupDlg.h"
#include <foreach.h>

// (b.savon 2013-02-01 11:36) - PLID 54982 - Created

CNexERxLicense::CNexERxLicense(void)
{
	m_nLicensedPrescriberUsed = g_pLicense->GetERXLicPrescribersCountUsed();
	m_nLicensedPrescriberAllowed = g_pLicense->GetERXLicPrescribersCountAllowed();
	m_nMidlevelPrescriberUsed = g_pLicense->GetERXMidPrescribersCountUsed();
	m_nMidlevelPrescriberAllowed = g_pLicense->GetERXMidPrescribersCountAllowed();
	m_nNurseStaffUsed = g_pLicense->GetERXStaffPrescribersCountUsed();
	m_nNurseStaffAllowed = g_pLicense->GetERXStaffPrescribersCountAllowed();

	m_strAvailableLicensedPrescriberWhereClause = "";
	m_strLicensedPrescriberWhereClause = "";
	m_strInactiveLicensedPrescriberWhereClause = "";
	m_strAvailableMidlevelPrescriberWhereClause = "";
	m_strLicensedMidlevelPrescriberWhereClause = "";
	m_strInactiveMidlevelPrescriberWhereClause = "";
	m_strAvailableNurseStaffWhereClause = "";
	m_strLicensedNurseStaffWhereClause = "";
	m_strInactiveNurseStaffWhereClause = "";

	PopulateIDs();
}

CNexERxLicense::~CNexERxLicense(void)
{
}

void CNexERxLicense::PopulateIDs()
{
	PopulateLicensedPrescriberIDs();
	PopulateMidlevelPrescriberIDs();
	PopulateNurseStaffIDs();
}

void CNexERxLicense::PopulateLicensedPrescriberIDs()
{
	CDWordArray dwaInactiveIDs, dwaUsedIDs;
	CString strInactiveIDs, strUsedIDs, str;
	
	g_pLicense->GetInactiveERXLicPrescribers(dwaInactiveIDs);
	g_pLicense->GetUsedERXLicPrescribers(dwaUsedIDs);

	m_strLicensedPrescriberInactiveIDs = ArrayAsString(dwaInactiveIDs, true);
	m_strLicensedPrescriberUsedIDs = ArrayAsString(dwaUsedIDs, true);
}

void CNexERxLicense::PopulateMidlevelPrescriberIDs()
{
	CDWordArray dwaInactiveIDs, dwaUsedIDs;

	g_pLicense->GetInactiveERXMidPrescribers(dwaInactiveIDs);
	g_pLicense->GetUsedERXMidPrescribers(dwaUsedIDs);

	m_strMidlevelPrescriberInactiveIDs = ArrayAsString(dwaInactiveIDs, true);
	m_strMidlevelPrescriberUsedIDs = ArrayAsString(dwaUsedIDs, true);
}

void CNexERxLicense::PopulateNurseStaffIDs()
{
	CDWordArray dwaInactiveIDs, dwaUsedIDs;

	g_pLicense->GetInactiveERXStaffPrescribers(dwaInactiveIDs);
	g_pLicense->GetUsedERXStaffPrescribers(dwaUsedIDs);

	m_strNurseStaffInactiveIDs = ArrayAsString(dwaInactiveIDs, true);
	m_strNurseStaffUsedIDs = ArrayAsString(dwaUsedIDs, true);
}

CString CNexERxLicense::GetAvailableLicensedPrescriberWhereClause()
{
	CString str;
	if (!m_strLicensedPrescriberUsedIDs.IsEmpty() && !m_strLicensedPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 0",
				   m_strLicensedPrescriberUsedIDs, m_strLicensedPrescriberInactiveIDs);
	} else if (!m_strLicensedPrescriberUsedIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 0", m_strLicensedPrescriberUsedIDs);
	} else if (!m_strLicensedPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 0", m_strLicensedPrescriberInactiveIDs);
	} else {
		str.Format("PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 0");
	}
	return str;
}

CString CNexERxLicense::GetLicensedPrescriberWhereClause()
{
	CString str;
	if (!m_strLicensedPrescriberUsedIDs.IsEmpty()) {
		str.Format("ProvidersT.NexERxProviderTypeID = 0 AND PersonT.ID IN (%s)", m_strLicensedPrescriberUsedIDs);
	} else {
		str.Format("1=0");
	}
	return str;
}

CString CNexERxLicense::GetInactiveLicensedPrescriberWhereClause()
{
	CString str;
	if (!m_strLicensedPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID IN (%s)", m_strLicensedPrescriberInactiveIDs);
		
	} else {
		str.Format("1=0");
	}
	return str;	
}

CString CNexERxLicense::GetAvailableMidlevelPrescriberWhereClause()
{
	CString str;
	if (!m_strMidlevelPrescriberUsedIDs.IsEmpty() && !m_strMidlevelPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 1",
				   m_strMidlevelPrescriberUsedIDs, m_strMidlevelPrescriberInactiveIDs);
	} else if (!m_strMidlevelPrescriberUsedIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 1", m_strMidlevelPrescriberUsedIDs);
	} else if (!m_strMidlevelPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 1", m_strMidlevelPrescriberInactiveIDs);
	} else {
		str.Format("PersonT.Archived = 0 AND ProvidersT.NexERxProviderTypeID = 1");
	}
	return str;
}

CString CNexERxLicense::GetLicensedMidlevelPrescriberWhereClause()
{
	CString str;
	if (!m_strMidlevelPrescriberUsedIDs.IsEmpty()) {
		str.Format("ProvidersT.NexERxProviderTypeID = 1 AND PersonT.ID IN (%s)", m_strMidlevelPrescriberUsedIDs);
	} else {
		str.Format("1=0");
	}
	return str;
}

CString CNexERxLicense::GetInactiveMidlevelPrescriberWhereClause()
{
	CString str;
	if (!m_strMidlevelPrescriberInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID IN (%s)", m_strMidlevelPrescriberInactiveIDs);
		
	} else {
		str.Format("1=0");
	}
	return str;
}

CString CNexERxLicense::GetAvailableNurseStaffWhereClause()
{
	CString str;
	if (!m_strNurseStaffUsedIDs.IsEmpty() && !m_strNurseStaffInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND UsersT.NexERxUserTypeID = 2",
					m_strNurseStaffUsedIDs, m_strNurseStaffInactiveIDs);
	} else if (!m_strNurseStaffUsedIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND UsersT.NexERxUserTypeID = 2", m_strNurseStaffUsedIDs);
	} else if (!m_strNurseStaffInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID NOT IN (%s) AND PersonT.Archived = 0 AND UsersT.NexERxUserTypeID = 2", m_strNurseStaffInactiveIDs);
	} else {
		str.Format("PersonT.Archived = 0 AND UsersT.NexERxUserTypeID = 2");
	}
	return str;
}

CString CNexERxLicense::GetLicensedNurseStaffWhereClause()
{
	CString str;
	if (!m_strNurseStaffUsedIDs.IsEmpty()) {
		str.Format("UsersT.NexERxUserTypeID = 2 AND PersonT.ID IN (%s)", m_strNurseStaffUsedIDs);
	} else {
		str.Format("1=0");
	}
	return str;
}

CString CNexERxLicense::GetInactiveNurseStaffWhereClause()
{
	CString str;
	if (!m_strNurseStaffInactiveIDs.IsEmpty()) {
		str.Format("PersonT.ID IN (%s)", m_strNurseStaffInactiveIDs);
		
	} else {
		str.Format("1=0");
	}
	return str;
}

CString CNexERxLicense::GetLicenseStatisticsMessage()
{
	CString strMessage;
	strMessage += GetLicensePrescriberMessage();
	strMessage += GetMidlevelPrescriberMessage();
	strMessage += GetNurseStaffMesssage();
	return strMessage;
}

CString CNexERxLicense::GetPrescriberStatisticsMessage()
{
	CString strMessage;
	strMessage += GetLicensePrescriberMessage();
	strMessage += GetMidlevelPrescriberMessage();
	strMessage.Replace("\r\n", " and ");
	return strMessage.TrimRight(",");
}

CString CNexERxLicense::GetLicensePrescriberMessage()
{
	long nRemaining = GetRemainingLicensedPrescriberCount();

	if (nRemaining < 0) {
		ASSERT(FALSE); // they have more licenses in use than they have licensed!!
		nRemaining = 0;
	}

	CString strMsg;
	strMsg.Format("Using %li of %li Licensed Prescriber licenses (%li remaining) \r\n", 
				  GetUsedLicensedPrescriberCount(), GetAllowedLicensedPrescriberCount(), nRemaining);
	return strMsg;
}

CString CNexERxLicense::GetMidlevelPrescriberMessage()
{
	long nRemaining = GetRemainingMidlevelPrescriberCount();

	if (nRemaining < 0) {
		ASSERT(FALSE); // they have more licenses in use than they have licensed!!
		nRemaining = 0;
	}

	CString strMsg;
	strMsg.Format("Using %li of %li Midlevel Prescriber licenses (%li remaining) \r\n", 
				  GetUsedMidlevelPrescriberCount(), GetAllowedMidlevelPrescriberCount(), nRemaining);
	return strMsg;
}

CString CNexERxLicense::GetNurseStaffMesssage()
{
	long nRemaining = GetRemainingNurseStaffCount();

	if (nRemaining < 0) {
		ASSERT(FALSE); // they have more licenses in use than they have licensed!!
		nRemaining = 0;
	}

	CString strMsg;
	strMsg.Format("Using %li of %li Nurse/Staff licenses (%li remaining) \r\n", 
				  GetUsedNurseStaffCount(), GetAllowedNurseStaffCount(), nRemaining);
	return strMsg;
}

// (a.wilson 2013-04-30 17:40) - PLID 56509 - modified to store the prescribingIDs and the user role to prevent reuse of the same query.
BOOL CNexERxLicense::IsCurrentUserLicensedForNexERx()
{
	//Users are tied directly to both Licensed and Midlevel prescribers
	//So if the NexERxUserTypeID returned is either a 0 or 1 (Licensed & Midlevel respectively)
	//then compare on the PersonID.  Otherwise, compare on the UserID (For UserTypes of 2 [Nurse/Staff])
	ADODB::_RecordsetPtr prs = CreateParamRecordset("{SQL}", GetNexERxUserRoles(GetCurrentUserID()));
	if( prs->eof ){ // They aren't configured
		return FALSE;
	}

	//They are configred for something, make sure the licensed is being used and not deactivated.
	// (a.wilson 2013-04-30 16:26) - PLID 56509 - Assign the role to the user.
	m_nexerxUser.urtUserRole = (EUserRoleTypes)AdoFldLong(prs->Fields, "UserType", urtNone);
	switch (m_nexerxUser.urtUserRole)
	{
		case urtLicensedPrescriber:
			{
				// (a.wilson 2013-04-30 16:26) - PLID 56509
				long nID = AdoFldLong(prs->Fields, "LicensedPrescriberID", -1);
				m_nexerxUser.aryPrescribingIDs.Add(nID);

				if(!IsUserLicensedNexERxLicensedPrescriber(nID)) {
					m_nexerxUser.aryPrescribingIDs.RemoveAll();
					return FALSE;
				}
				return TRUE;
			}
		case urtMidlevelPrescriber:
			{
				//First we need to check the supervisors to see if they are licensed.
				CArray<long, long> arySupervising;
				StringAsArray((LPCTSTR)AdoFldString(prs->Fields, "SupervisingIDs"), arySupervising);
				//If any aren't licensed, this user is licensed. Even though the one they might be selecting to prescriber
				//for on the Rx is licensed but another isn't, we want it to fail so they configure it properly for all prescribers.
				foreach(long nSupervisorID, arySupervising) {
					// (a.wilson 2013-04-30 16:26) - PLID 56509 - Add the ID to the array. Clear the array if they aren't configured properly.
					// We want them to be configured.
					if(!IsUserLicensedNexERxLicensedPrescriber(nSupervisorID)) {
						m_nexerxUser.aryPrescribingIDs.RemoveAll();
						return FALSE;
					}
					m_nexerxUser.aryPrescribingIDs.Add(nSupervisorID);
				}
				
				// (a.wilson 2013-04-30 16:26) - PLID 56509 - Add the midlevel
				long nMidlevelPrescriberID = AdoFldLong(prs->Fields, "MidlevelPrescriberID", -1);
				m_nexerxUser.aryPrescribingIDs.Add(nMidlevelPrescriberID);

				if(!IsUserLicensedNexERxMidlevelPrescriber(nMidlevelPrescriberID)) {
					m_nexerxUser.aryPrescribingIDs.RemoveAll();
					return FALSE;
				}
				return TRUE;
			}
		case urtNurseStaff:
			{
				//First we need to get the id's and split them into the midlevel and supervising sections
				CString strSupervising;
				CString strMidlevel;
				GetNurseStaffMidlevelIdentifiers(AdoFldString(prs->Fields, "PrescribingIDs", ""), strSupervising, strMidlevel, nsiID);
				// Split and add supervising to array.
				CArray<long, long> arySupervising;
				if(!strSupervising.IsEmpty() ) {
					StringAsArray((LPCTSTR)strSupervising, arySupervising);
				}
				//Split and add Midlevel to array.
				CArray<long, long> aryMidlevel;
				if(!strMidlevel.IsEmpty() ) {
					StringAsArray((LPCTSTR)strMidlevel, aryMidlevel);
				}
				//If any aren't licensed, this user is licensed. Even though the one they might be selecting to prescriber
				//for on the Rx is licensed but another isn't, we want it to fail so they configure it properly for all prescribers.
				foreach(long nLicensedID, arySupervising) {
					// (a.wilson 2013-04-30 16:26) - PLID 56509 - Add the ID to the array. Clear the array if they aren't configured properly.
					// We want them to be configured.
					if(!IsUserLicensedNexERxLicensedPrescriber(nLicensedID) ) {
						m_nexerxUser.aryPrescribingIDs.RemoveAll();
						return FALSE;
					}
					m_nexerxUser.aryPrescribingIDs.Add(nLicensedID);
				}
				//If any aren't licensed, this user is licensed. Even though the one they might be selecting to prescriber
				//for on the Rx is licensed but another isn't, we want it to fail so they configure it properly for all prescribers.
				foreach(long nMidlevelID, aryMidlevel){		
					if(!IsUserLicensedNexERxMidlevelPrescriber(nMidlevelID) ) {
						m_nexerxUser.aryPrescribingIDs.RemoveAll();
						return FALSE;
					}
					m_nexerxUser.aryPrescribingIDs.Add(nMidlevelID);
				}

				long nNurseStaffID = AdoFldLong(prs->Fields, "ID", -1);
				m_nexerxUser.aryPrescribingIDs.Add(nNurseStaffID);

				if(!IsUserLicensedNexERxNurseStaff(nNurseStaffID) ) {
					m_nexerxUser.aryPrescribingIDs.RemoveAll();
					return FALSE;
				}				
				return TRUE;
			}
		case urtNone:
		default:
			return FALSE;
	}
}

BOOL CNexERxLicense::IsUserLicensedNexERxLicensedPrescriber(long nPersonIDToCompare)
{
	CDWordArray dwaLicensedPrescriber;
	GetUsedLicensedPrescriberIDs(dwaLicensedPrescriber);

	foreach(DWORD dwID, dwaLicensedPrescriber){
		if( (long)dwID == nPersonIDToCompare ){
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNexERxLicense::IsUserLicensedNexERxMidlevelPrescriber(long nPersonIDToCompare)
{
	CDWordArray dwaMidlevelPrescriber;
	GetUsedMidlevelPrescriberIDs(dwaMidlevelPrescriber);

	foreach(DWORD dwID, dwaMidlevelPrescriber){
		if( (long)dwID == nPersonIDToCompare ){
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNexERxLicense::IsUserLicensedNexERxNurseStaff(long nPersonIDToCompare)
{
	CDWordArray dwaNurseStaff;
	GetUsedNurseStaffIDs(dwaNurseStaff);

	foreach(DWORD dwID, dwaNurseStaff){
		if( (long)dwID == nPersonIDToCompare ){
			return TRUE;
		}
	}

	return FALSE;
}

// (a.wilson 2013-04-30 16:19) - PLID 56509 - accessor for pulling the current users erx settings.
void CNexERxLicense::GetNexERxUser(NexERxUser &nexerxUser)
{
	// Copy the array
	foreach(long nID, m_nexerxUser.aryPrescribingIDs){
		nexerxUser.aryPrescribingIDs.Add(nID);
	}

	// Copy the user role
	nexerxUser.urtUserRole = m_nexerxUser.urtUserRole;
}