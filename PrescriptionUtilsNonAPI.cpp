#include "stdafx.h"
#include "PrescriptionUtilsNonAPI.h"
#include "EmrUtils.h"
#include "AuditTrail.h"
#include "NxUILib\DatalistUtils.h"//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
#include "MedicationSearch_DataListProvider.h"//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
#include "AllergySearch_DatalistProvider.h"
// (j.jones 2013-03-27 16:43) - PLID 55920 - Created to define enums & non-API functions,
// in a file separately from PrescriptionUtilsAPI.h, so that any code that needs the enums
// without also needing API accessor code will not need to also include NxAPI.h.

CSqlFragment GetNexERxUserRoles(long nUserID)
{
	CSqlFragment sqlWhere = CSqlFragment("AND UserPersonQ.ID = {INT}", nUserID);
    return CSqlFragment(
	"DECLARE @SupervisorIDT TABLE(   \r\n" 
	"	UserID INT,    \r\n"
	"	SupStr NVARCHAR(MAX),    \r\n"
	"	SupNameStr NVARCHAR(MAX)    \r\n"
	");    \r\n"
	"  \r\n"
	"DECLARE @NurseMidlevelIDT TABLE(  \r\n"
	"	UserID INT,  \r\n"
	"	MidStr NVARCHAR(MAX),  \r\n"
	"	MidNameStr NVARCHAR(MAX)  \r\n"
	");  \r\n"
	"  \r\n"
	"DECLARE @NurseSupervisorIDT TABLE(  \r\n"
	"	UserID INT,  \r\n"
	"	SupStr NVARCHAR(MAX),  \r\n"
	"	SupNameStr NVARCHAR(MAX)  \r\n"
	");  \r\n"
	" \r\n"
	"SET NOCOUNT ON;   \r\n"
	"INSERT INTO @SupervisorIDT (UserID, SupStr, SupNameStr)  \r\n" 
	"SELECT	UsersT.PersonID,    \r\n"
	"		STUFF((    \r\n"
	"		SELECT  ',' + CONVERT(NVARCHAR, InnerESP.SupervisingPersonID)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxSupervisingProviderT InnerESP ON InnerUser.PersonID = InnerESP.UserID \r\n"   
	"				AND InnerUser.NexERxProviderID = InnerESP.PersonID	 \r\n"	  
	"		WHERE InnerUser.PersonID = UsersT.PersonID   \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, ''),   \r\n"
	"		STUFF((    \r\n"
	"		SELECT  '; ' + CONVERT(NVARCHAR, InnerPerson.FullName)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxSupervisingProviderT InnerESP ON InnerUser.PersonID = InnerESP.UserID  \r\n"  
	"				AND InnerUser.NexERxProviderID = InnerESP.PersonID   \r\n"
	"		INNER JOIN PersonT AS InnerPerson ON InnerESP.SupervisingPersonID = InnerPerson.ID		  \r\n" 
	"		WHERE InnerUser.PersonID = UsersT.PersonID   \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, '')   \r\n"
	"FROM	UsersT   \r\n"
	"GROUP BY UsersT.PersonID   \r\n"
	" \r\n"
	"INSERT INTO @NurseSupervisorIDT (UserID, SupStr, SupNameStr)   \r\n"
	"SELECT	UsersT.PersonID,    \r\n"
	"		'*S*' + STUFF((    \r\n"
	"		SELECT  ',' + CONVERT(NVARCHAR, InnerNSP.NurseStaffPrescriberID)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxNurseStaffPrescriberT InnerNSP ON InnerUser.PersonID = InnerNSP.UserID  	 \r\n"  
	"		WHERE InnerUser.PersonID = UsersT.PersonID AND InnerNSP.NexERxUserType = 0  \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, ''), \r\n"  
	"		'Supervisor(s): ' + STUFF((    \r\n"
	"		SELECT  '; ' + CONVERT(NVARCHAR, InnerPerson.FullName)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxNurseStaffPrescriberT InnerNSP ON InnerUser.PersonID = InnerNSP.UserID    \r\n"
	"		INNER JOIN PersonT AS InnerPerson ON InnerNSP.NurseStaffPrescriberID = InnerPerson.ID		 \r\n"  
	"		WHERE InnerUser.PersonID = UsersT.PersonID AND InnerNSP.NexERxUserType = 0  \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, '')   \r\n"
	"FROM	UsersT   \r\n"
	"GROUP BY UsersT.PersonID   \r\n"
	"  \r\n"
	"INSERT INTO @NurseMidlevelIDT (UserID, MidStr, MidNameStr)   \r\n"
	"SELECT	UsersT.PersonID,    \r\n"
	"		'*M*' + STUFF((    \r\n"
	"		SELECT  ',' + CONVERT(NVARCHAR, InnerNSP.NurseStaffPrescriberID)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxNurseStaffPrescriberT InnerNSP ON InnerUser.PersonID = InnerNSP.UserID   \r\n"	  
	"		WHERE InnerUser.PersonID = UsersT.PersonID AND InnerNSP.NexERxUserType = 1  \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, ''),   \r\n"
	"		'Midlevel(s): ' + STUFF((    \r\n"
	"		SELECT  '; ' + CONVERT(NVARCHAR, InnerPerson.FullName)   \r\n"
	"		FROM	UsersT InnerUser   \r\n"
	"		INNER JOIN NexERxNurseStaffPrescriberT InnerNSP ON InnerUser.PersonID = InnerNSP.UserID   \r\n" 
	"		INNER JOIN PersonT AS InnerPerson ON InnerNSP.NurseStaffPrescriberID = InnerPerson.ID \r\n"		  
	"		WHERE InnerUser.PersonID = UsersT.PersonID AND InnerNSP.NexERxUserType = 1  \r\n"
	"		FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 1, '')   \r\n"
	"FROM	UsersT   \r\n"
	"GROUP BY UsersT.PersonID   \r\n"
	"SET NOCOUNT OFF;   \r\n"
	" \r\n"
	"SELECT	UserPersonQ.ID,   \r\n"
	"		UserPersonQ.FullName AS UserPersonName, \r\n"
	"		UsersT.UserName,   \r\n"
	"		COALESCE(UsersT.NexERxUserTypeID, -1) AS UserType, -- -> Combo Source -1;<None>;0;Licensed Prescriber;1;Midlevel Provider;2;Nurse/Staff   \r\n"
	"		CASE WHEN UsersT.NexERxUserTypeID = 0 THEN UsersT.NexERxProviderID ELSE NULL END AS LicensedPrescriberID,   \r\n"
	"		CASE WHEN UsersT.NexERxUserTypeID = 0 THEN (SELECT FullName FROM PersonT WHERE ID = UsersT.NexERxProviderID) ELSE NULL END AS LicensedPrescriberName,   \r\n"
	"		CASE WHEN UsersT.NexERxUserTypeID = 1 THEN UsersT.NexERxProviderID ELSE NULL END AS MidlevelPrescriberID,   \r\n"
	"		CASE WHEN UsersT.NexERxUserTypeID = 1 THEN (SELECT FullName FROM PersonT WHERE ID = UsersT.NexERxProviderID) ELSE NULL END AS MidlevelPrescriberName,   \r\n"
	"		SupT.SupStr AS SupervisingIDs,   \r\n"
	"		SupT.SupNameStr AS SupervisingNames,  \r\n"
	"		CASE WHEN NSupT.SupStr IS NULL THEN NMidT.MidStr ELSE CASE WHEN NMidT.MidStr IS NULL THEN NSupT.SupStr ELSE NSupT.SupStr + NMidT.MidStr END END AS PrescribingIDs,  \r\n"
	"		CASE WHEN NSupT.SupNameStr IS NULL THEN NMidT.MidNameStr ELSE CASE WHEN NMidT.MidNameStr IS NULL THEN NSupT.SupNameStr ELSE NSupT.SupNameStr + '; ' + NMidT.MidNameStr END END AS PrescribingNames  \r\n"
	"FROM	UsersT    \r\n"
	"	INNER JOIN PersonT AS UserPersonQ ON UsersT.PersonID = UserPersonQ.ID   \r\n"
	"	INNER JOIN @SupervisorIDT AS SupT ON UsersT.PersonID = SupT.UserID   \r\n"
	"	INNER JOIN @NurseSupervisorIDT AS NSupT ON UsersT.PersonID = NSupT.UserID  \r\n"
	"	INNER JOIN @NurseMidlevelIDT AS NMidT ON UsersT.PersonID = NMidT.UserID  \r\n"
	"WHERE	UserPersonQ.Archived = 0 {SQL} AND UserPersonQ.ID > 0 \r\n"
	"ORDER BY UsersT.UserName \r\n",
	nUserID > 0 ? sqlWhere : CSqlFragment());
}

// (b.savon 2013-01-11 10:58) - PLID 54584
void GetNurseStaffMidlevelIdentifiers(CString strPrescribers, CString &strSupervising, CString &strMidlevel, ENurseStaffIdentifier nsiIdentifier)
{
	// There are 4 cases
	// (1)	<NONE>
	// (2)	*S*x, y, z*M*a, b, c
	// (3)	*M*a, b, c
	// (4)	*S*x, y, z

	// Case 1 - <NONE>
	// There is no data, get out.
	if( strPrescribers.CompareNoCase("-1") == 0 ){
		return;
	}

	// Set the correct identifier params
	CString strSupervisorIdentifier;
	CString strMidlevelIdentifier;
	switch( nsiIdentifier ){
		case nsiID:
			{
				strSupervisorIdentifier = "*S*";
				strMidlevelIdentifier = "*M*";
			}
			break;
		case nsiName:
			{
				// (b.savon 2013-01-17 09:07) - PLID 54656 - Work around for CString::TrimLeft(..) bug
				strSupervisorIdentifier = "Supervisor(s):";
				strMidlevelIdentifier = "Midlevel(s):";
			}
			break;
		default:
			{
				// Need to handle new identifier
				ASSERT(FALSE);
			}
			break;
	}

	// Find the supervising and midlevel start positions (if any).
	int nSupervisingStart = strPrescribers.Find(strSupervisorIdentifier);
	int nMidlevelStart = strPrescribers.Find(strMidlevelIdentifier);

	// If we have some midlevel ids, let
	if( nMidlevelStart > -1 ){
		// Case 2 - *S*x, y, z*M*a, b, c
		// If we have some supervisors
		if( nSupervisingStart > -1 ){
			CString strTemp = strPrescribers.Left(nMidlevelStart);
			strTemp.TrimLeft(strSupervisorIdentifier);
			strTemp.Trim();
			strSupervising = strTemp;	
			strTemp = strPrescribers.Right(strPrescribers.GetLength()-nMidlevelStart);
			strTemp.TrimLeft(strMidlevelIdentifier);
			strTemp.Trim();
			strMidlevel = strTemp;
		}else{ // There aren't any supervisors - Case 3 - *M*a, b, c
			strPrescribers.TrimLeft(strMidlevelIdentifier);
			strMidlevel = strPrescribers;
		}
	}else{ // There are just supervisors - Case 4 - *S*x, y, z
		strPrescribers.TrimLeft(strSupervisorIdentifier);
		strSupervising = strPrescribers;
	}
}

// (b.savon 2013-01-23 10:21) - PLID 54758 - Moved to utility
void SplitNames(CString strDelimetedNames, CArray<CString, LPCTSTR> &arr, CString strDelimeter)
{
	int curPos = 0;
	CString strToken = strDelimetedNames.Tokenize(_T(strDelimeter), curPos);

	while(strToken != _T("")){
		strToken.Trim();
		arr.Add(strToken);
		strToken = strDelimetedNames.Tokenize(_T(strDelimeter), curPos);
		strToken.Trim();
	}
}

// (j.jones 2013-01-23 16:30) - PLID 53259 - added IsERxStatus, returns true
//if the given QueueStatus is pqseTransmitAuthorized, pqseTransmitSuccess, pqseTransmitError, or pqseTransmitPending
// (b.savon 2013-09-23 07:15) - PLID 58486 - Prevent same statuses in GetERxStatusFilter
// (b.eyers 2016-02-05) - PLID 67980 - added dispensed in house
BOOL IsERxStatus(PrescriptionQueueStatus eStatus)
{
	if(eStatus == pqseTransmitAuthorized
		|| eStatus == pqseTransmitSuccess
		|| eStatus == pqsPrinted
		|| eStatus == pqseTransmitPending
		|| eStatus == pqseVoid
		|| eStatus == pqseFaxed
		|| eStatus == pqseDispensedInHouse) {

		//this is an ERxStatus
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.jones 2013-01-23 16:30) - PLID 53259 - added GetERxStatusFilter, returns a SQL fragment of
// comma-delimited {CONST} values with the known ERx statuses
// (b.savon 2013-09-23 07:15) - PLID 58486 - When writing a prescription from an EMN, you are able
// to right click and delete the rx after it has been printed.  Swapped error with Printed; and added Void
// (b.eyers 2016-02-05) - PLID 67980 - dispsened in house cannot be deleted
CSqlFragment GetERxStatusFilter()
{
	return CSqlFragment("{CONST},{CONST},{CONST},{CONST},{CONST},{CONST},{CONST}",
		(long)pqseTransmitAuthorized,
		(long)pqseTransmitSuccess,
		(long)pqsPrinted,
		(long)pqseTransmitPending,
		(long)pqseVoid,
		(long)pqseFaxed,
		(long)pqseDispensedInHouse);
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
// (j.jones 2012-10-17 10:03) - PLID 53179 - UpdateHasNoAllergiesStatus will update and audit
// changes to PatientsT.HasNoAllergy. This function assumes this status has changed, and doesn't re-verify that fact.
void UpdateHasNoAllergiesStatus(BOOL bHasNoAllergies, long nPatientID, CTableChecker* pTableChecker /*=NULL*/)
{
	try {

		//save the change
		ExecuteParamSql("UPDATE PatientsT SET HasNoAllergies = {BIT} WHERE PersonID = {INT}", bHasNoAllergies, nPatientID);

		//audit
		long nAuditID = BeginNewAuditEvent();
		CString strOldValue, strNewValue;
		if(bHasNoAllergies) {
			//they are checking the box
			strOldValue = "'No Known Allergies' Status Cleared";
			strNewValue = "'No Known Allergies' Status Selected";
		}
		else {
			//they are unchecking the box
			strOldValue = "'No Known Allergies' Status Selected";
			strNewValue = "'No Known Allergies' Status Cleared";
		}
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientHasNoAllergiesStatus, nPatientID, strOldValue, strNewValue, aepMedium, aetChanged);

		//send a tablechecker
		if(pTableChecker)
		{
			pTableChecker->Refresh(nPatientID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
// (j.jones 2012-10-17 13:08) - PLID 51713 - UpdateHasNoMedsStatus will update and audit changes to PatientsT.HasNoMeds.
// This function assumes this status has changed, and doesn't re-verify that fact.
void UpdateHasNoMedsStatus(BOOL bHasNoMeds, long nPatientID, CTableChecker* pTableChecker /*=NULL*/)
{
	try {

		//save the change
		ExecuteParamSql("UPDATE PatientsT SET HasNoMeds = {BIT} WHERE PersonID = {INT}", bHasNoMeds, nPatientID);

		//audit
		long nAuditID = BeginNewAuditEvent();
		CString strOldValue, strNewValue;
		if(bHasNoMeds) {
			//they are checking the box
			strOldValue = "'No Medications' Status Cleared";
			strNewValue = "'No Medications' Status Selected";
		}
		else {
			//they are unchecking the box
			strOldValue = "'No Medications' Status Selected";
			strNewValue = "'No Medications' Status Cleared";
		}
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientHasNoMedicationsStatus, nPatientID, strOldValue, strNewValue, aepMedium, aetChanged);

		//send a tablechecker
		if(pTableChecker)
		{
			pTableChecker->Refresh(nPatientID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-02-05 14:40) - PLID 54463 - Created a utility function for displaying the sig/drug name
// (j.fouts 2013-04-22 10:26) - PLID 56155 - This now just uses the defined sig
CString GenerateDrugSigDisplayName(const CString& strDrugName,
								   const CString& strSig)
{
	return strDrugName + (strSig.IsEmpty()? "" : " - " + strSig);
}

// (j.fouts 2013-02-05 14:40) - PLID 54463 - Created a utility function for generating a sig
CString GenerateSig(const CString& strDosageQuantity, 
					   const CString& strDosageUnitSingular,
					   const CString& strDosageUnitPlural,
					   const CString& strDosageRoute, 
					   const CString& strDosageFrequency)
{
	const CString& strDosageUnit = strDosageQuantity.Compare("1") == 0? strDosageUnitSingular : strDosageUnitPlural;
	
	CString strSig;
	strSig += (strDosageRoute.IsEmpty()? "" : strDosageRoute + " ");
	strSig += (strDosageQuantity.IsEmpty()? "" : strDosageQuantity + " ");
	strSig += (strDosageUnit.IsEmpty()? "" : strDosageUnit + " ");
	strSig += (strDosageFrequency.IsEmpty()? "" : strDosageFrequency);

	if(!strSig.IsEmpty())
	{
		strSig.SetAt(0,toupper(strSig[0]));
	}

	return strSig.Trim();
}

//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
LPUNKNOWN BindMedicationSearchListCtrl(CWnd *pParent,  UINT nID, ADODB::_ConnectionPtr pDataConn, bool bFormulary, bool bIncludeFDBMedsOnly)
{
	//We can utilize the standard bind to do the majority of the work for us, then tack on a couple specific search
	//	things afterwards.
	//Bind will do this for us:
	//	1)  Bind the control from the IDC
	//	2)  Apply the connection, if necessary
	//	3)  Requery if needed (which will never happen for a search dialog)
	//	4)  Some fancy exception handling on various failures for the above.

	//Bind it as normal.  Search view is only supported on datalist2's.
	NXDATALIST2Lib::_DNxDataListPtr pdl = BindNxDataList2Ctrl(pParent,nID, pDataConn, false);

	//Now do our special search things
	if (pdl) {
		//if the datalist is not using the search list, fail
		if (pdl->GetViewType() != NXDATALIST2Lib::vtSearchList) {
			//this would be a coding failure at design time
			ThrowNxException("BindMedicationSearchListCtrl called on a datalist that is not a Search List.");
		}
		if (bFormulary == FALSE)
		{
			SetDatalistMinDropdownWidth(pdl, 400);
		}
		else
		{
			SetDatalistMinDropdownWidth(pdl, 430);
		}
		//all Medication search lists have the same placeholder text
		pdl->PutSearchPlaceholderText("Medication Search...");
		//initialize the datalist for our search style,
		//which will also set our DataListProvider for us
		//set the provider
		// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
		CMedicationSearch_DataListProvider* provider = new CMedicationSearch_DataListProvider(pDataConn, bFormulary, bIncludeFDBMedsOnly);

		// (s.dhole 3/6/ 2015 9:42 AM) - PLID 64610
		// Tell the provider to interact with our datalist
		provider->Register(pdl);

		//Make sure to release to avoid memory leaks!
		provider->Release();
		//The API returns records in sorted order. We do not need to sort.
	}
	// If we made it here we have success or it failed in ::Bind
	return pdl;
}

//(s.dhole 3/10/2015 5:25 PM ) - PLID 64564
// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
LPUNKNOWN BindAllergySearchListCtrl(CWnd *pParent, UINT nID, ADODB::_ConnectionPtr pDataConn, bool bIncludeFDBAllergiesOnly)
{
	//We can utilize the standard bind to do the majority of the work for us, then tack on a couple specific search
	//	things afterwards.
	//Bind will do this for us:
	//	1)  Bind the control from the IDC
	//	2)  Apply the connection, if necessary
	//	3)  Requery if needed (which will never happen for a search dialog)
	//	4)  Some fancy exception handling on various failures for the above.

	//Bind it as normal.  Search view is only supported on datalist2's.
	NXDATALIST2Lib::_DNxDataListPtr pdl = BindNxDataList2Ctrl(pParent, nID, pDataConn, false);

	//Now do our special search things
	if (pdl) {
		//if the datalist is not using the search list, fail
		if (pdl->GetViewType() != NXDATALIST2Lib::vtSearchList) {
			//this would be a coding failure at design time
			ThrowNxException("BindMedicationSearchListCtrl called on a datalist that is not a Search List.");
		}
		SetDatalistMinDropdownWidth(pdl, 400);
		
		//all Medication search lists have the same placeholder text
		pdl->PutSearchPlaceholderText("Allergy Search...");
		//initialize the datalist for our search style,
		//which will also set our DataListProvider for us
		//set the provider
		// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
		CAllergySearch_DatalistProvider* provider = new CAllergySearch_DatalistProvider(pDataConn, bIncludeFDBAllergiesOnly);

		// (s.dhole 3/6/ 2015 9:42 AM) - PLID 64610
		// Tell the provider to interact with our datalist
		provider->Register(pdl);

		//Make sure to release to avoid memory leaks!
		provider->Release();
		//The API returns records in sorted order. We do not need to sort.
	}
	// If we made it here we have success or it failed in ::Bind
	return pdl;
}

// (j.jones 2016-01-21 11:34) - PLID 67970 - Added a warning used whenever a user toggles on the free-text search options.
// bIsAllergySearch is false for med searches, true for allergy searches.
// Returns true if the user still wants to continue.
bool ConfirmFreeTextSearchWarning(CWnd *pParent, bool bIsAllergySearch)
{
	CString strWarning, strType;
	strType = bIsAllergySearch ? "allergies" : "medications";
	strWarning.Format("Using free text %s (white background) rather than those from the imported list (salmon background) "
		"will result in a lack of interaction warnings.\n\n"
		"Are you sure you wish to include free text %s in your search?", strType, strType);

	if (IDYES == MessageBox(pParent ? pParent->GetSafeHwnd() : GetActiveWindow(),
		strWarning, "Practice", MB_ICONWARNING | MB_YESNO)) {
		return true;
	}
	else {
		return false;
	}
}