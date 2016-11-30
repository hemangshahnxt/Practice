#include "stdafx.h"
#include "globalutils.h"
#include "globaldatautils.h"
#include "mirror.h"
#include "client.h"
#include "NxErrorDialog.h"
#include "ExportDuplicates.h"
#include "t2klib.h"
#include "graphics.h"
#include "regutils.h"
#include "nxprogressdlg.h"
#include "nxcanfieldlink.h"
#include "marketutils.h"
#include "ShowConnectingFeedbackDlg.h"
#include "audittrail.h"
#include "NxPropManager.h"
#include "NxBackupUtils.h"
#include "WellnessDataUtils.h"

#define XOR(a,b) (a && !b || !a && b)

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - We dont use the provider friendly name any longer

_ConnectionPtr g_pConMirror;//cannot be a member of the namespace (or at least I haven't figured out how to release it on exit instance)
//CString g_strMirrorPFN("This is hopefully always an invalid source nameas90hfa-zdh8vd9zu"); // The provider friendly name of the Mirror connection

extern CNxPropManager g_propManager;
extern const CString &GetNetworkLibraryString();
extern CString GetPassword();
extern long GetConnectionTimeout();
extern long GetCommandTimeout();

namespace Mirror
{
BOOL g_bForceDisable = FALSE;

// (c.haag 2006-08-02 13:27) - PLID 21670 - This is true after we've checked the license object
// to see whether or not we have a license to use the link. The result of that check goes into
// g_bHasMirrorLinkLicense
BOOL g_bIsLicensedKnown = FALSE;

// (c.haag 2006-08-02 13:28) - PLID 21670 - This is return value from CLicense::CheckForLicense
BOOL g_bHasMirrorLinkLicense = FALSE;

//Prototypes

static CString GetUniqueMirrorID(COleDateTime &CreateTime);
_RecordsetPtr CreateMirrorRecordsetStd(const CString &strSql, ADODB::CursorTypeEnum nCursorType = adOpenForwardOnly, 
								 ADODB::LockTypeEnum nLockType = adLockReadOnly, long nOptions = adCmdText, ADODB::CursorLocationEnum nCursor = ADODB::adUseClient);
_RecordsetPtr CreateMirrorRecordset(ADODB::CursorTypeEnum nCursorType, ADODB::LockTypeEnum nLockType, LPCTSTR strFmt, ...);

///////////////////////////////////////////////////////////////////////////////////
//represents the union of fields in practice and fields in mirror used by the link
class CMirrorPatient
{	
	public:
		long			personID;
		long			userID;
		CString			mirrorID;
		COleDateTime	createTime;
		CString			last;
		CString			first;
		CString			middle;
		COleDateTime	dob;
		CString			address1;
		CString			address2;
		CString			homePhone;
		CString			workPhone;
		CString			city;
		CString			state;
		CString			zip;
		CString			pin; // Social security
		CString			mrn; // Medical record number
		long			lSex;//0=unknown	1=male 2=female, same in practice and mirror

	void LoadFromMirror	(FieldsPtr &f);
	void LoadFromNextech(FieldsPtr &f);
	void SaveToMirror	(FieldsPtr &f);
	BOOL SaveExistingToNextech (FieldsPtr &f, OUT BOOL &bUpdatedGender, OUT BOOL &bUpdatedBirthdate);
	void SaveNewToNextech ();
};

BOOL IsMirrorEnabled()
{
	if (!HasMirrorLinkLicense())
		return FALSE;
	if (g_bForceDisable)
		return FALSE;
	if (GetPropertyInt("MirrorDisable", 0))
		return FALSE;
	return TRUE;
}

void ShowApplication(BOOL bShow)
{
	CanfieldLink::ShowApplication(bShow);
}


// (c.haag 2009-03-31 10:37) - PLID 33630 - This function will try to establish a link between
// NexTech Practice and Canfield Mirror. The caller must review the return value after the 
// function returns before executing Mirror functionality.
ECanfieldSDKInitResult InitCanfieldSDK(BOOL bForceSynchronousOpen, HWND hWndAsyncNotify /* = NULL */)
{
	try {
		if (!IsMirrorEnabled()) {
			// The Mirror link is disabled
			return eCSDK_Failure_Link_Disabled;
		}
		else if (!GetPropertyInt("MirrorAllow61Link", 1)) {
			// Technically speaking, the Canfield SDK link is disabled. If Mirror is running
			// on an access database, however, then the user could still use the pre-SDK link.
			return eCSDK_Failure_Link_SDK_Disabled;
		}

		// Ensure we have a link established with Mirror, and check the return value.
		switch (CanfieldLink::EnsureLink(GetSubRegistryKey(), GetNetworkLibraryString(), GetSqlServerName(), GetPassword(), GetConnectionTimeout(), GetCommandTimeout(), bForceSynchronousOpen ? 0 : GetPropertyInt("MirrorAllowAsyncOperations", 1), &g_propManager, hWndAsyncNotify, NXM_MIRROR_CANFIELD_SDK_INIT_COMPLETE)) {
			case CanfieldLink::eStatusLinkNotEstablished:
				// This is an unexpected value caused by an unknown exception in the link. The DLL is responsible
				// for handling the exceptions, and warning the user if necessary.
				if (!g_bForceDisable) {
					MsgBox(MB_OK|MB_ICONEXCLAMATION, "An unexpected error occurred while trying to link NexTech Practice with Canfield Mirror.\r\n\r\nThe link will not be attempted again until the next time Practice is started.");
					g_bForceDisable = TRUE;
				}
				return eCSDK_Failure_Error;
			case CanfieldLink::eStatusTryingToLink:
				// The link is still initializing, and a message will be posted to the notify
				// window when done
				ASSERT(!bForceSynchronousOpen);
				return eCSDK_Connecting_WillPostMessage;
			case CanfieldLink::eStatusLinkEstablished:
				// We have a link
				return eCSDK_Success;
			case CanfieldLink::eStatusIncompatibleSDK:
				// The link is incompatible or unavailable. The caller should defer to Pre-Canfield SDK functionality.
				return eCSDK_Failure_NotAvailable;
			case CanfieldLink::eStatusLinkTimedOut:
				// The link timed out; and we will not try again
				if (!g_bForceDisable) {
					MsgBox(MB_OK|MB_ICONEXCLAMATION, "The link from NexTech Practice to Canfield Mirror could not be established because Canfield Mirror did not respond in a timely manner.\r\n\r\nThe link will not be attempted again until the next time Practice is started.");
					g_bForceDisable = TRUE;
				}
				return eCSDK_Failure_TimeoutExpired;
			default:
				ASSERT(FALSE); // There are no other enumerations. Defer to the general error result.
				break;
		}
	}
	NxCatchAllThread("Error in InitCanfieldSDK");
	return eCSDK_Failure_Error;
}

// (c.haag 2009-03-31 10:41) - PLID 33630 - Deprecated
/*BOOL IsMirror61(BOOL bForceSynchronousOpen)
{
	// Returns S_OK if the Mirror product is present on their system and
	// Practice is able to interface with it.

	try {
		// CAH 5/19/03 - The MirrorAllow61Link and MirrorAllowAsyncOperations
		// properties can only be changed in NxQuery or another program outside
		// of Practice. These are per-computer properties.	
		// (c.haag 2006-12-01 10:36) - PLID 23725 - Check if we force Mirror 6.0 integration
		if (!IsMirrorEnabled())
			return FALSE;
		if (!GetPropertyInt("MirrorAllow61Link", 1))
			return FALSE;
		switch (CanfieldLink::EnsureLink(GetSubRegistryKey(), GetNetworkLibraryString(), GetSqlServerName(), GetPassword(), GetConnectionTimeout(), GetCommandTimeout(), bForceSynchronousOpen ? 0 : GetPropertyInt("MirrorAllowAsyncOperations", 1), &g_propManager))
		{
		case S_OK: return TRUE;
		//
		// (c.haag 2006-11-30 16:55) - PLID 23725 - We should not expect any answer other than
		// S_OK or S_FAILED. In fact, we should be testing "if (S_OK != ...", but I'm keeping the
		// legacy layout and comments here for historical reference.
		//
		case 0x80000000: // S_FAILED
			// (b.cardillo 2007-04-30 13:59) - PLID 25839 - We need to check for a new status that 
			// the NxCanfieldLink.dll can now set, which is that it timed out waiting for Mirror.  
			// The previous logic was that if it timed out, we should just try again.  But there 
			// were other bugs that caused subsequent attempts to give exceptions, so the timeouts 
			// would show up as weird things (like "Access Denied").  When we fixed those bugs, 
			// Practice started behaving according to the original intended logic, which was BAD 
			// because if Mirror was never going to succeed, Practice would effectively loop 
			// forever just trying over and over.  So now we're correcting that design by letting 
			// Practice know that it failed due to a timeout.  Right now, the correct behavior in 
			// such situation is to disable the link for the remainder of the session.
			if (CanfieldLink::GetLinkStatus() == CanfieldLink::eStatusLinkTimedOut) {
				if (!g_bForceDisable) {
					MsgBox(MB_OK|MB_ICONEXCLAMATION, "%s", "The link from NexTech Practice to Canfield Mirror could not be esablished because Canfield Mirror did not respond in a timely manner.\r\n\r\nThe link will not be attempted until the next time Practice is started.");
					g_bForceDisable = TRUE;
				}
			}
			break;
		default:
			ASSERT(FALSE);
			//if (!g_bForceDisable)
			//{
			//	MsgBox("The link from NexTech Practice to Mirror will be disabled for the remainder of this program session.");
			//	g_bForceDisable = TRUE;
			//}
			break;
		}
	}
	// (a.walling 2007-07-20 11:13) - PLID 26762 - Called from threads
	NxCatchAllThread("Error in IsMirror61");
	return FALSE;
}*/

BOOL IsMirror60()
{
	BOOL bRes = FALSE;
	long lType;

	if (EnsureMirrorData())
		return FALSE;

	try {
		_RecordsetPtr rsMirror = CreateMirrorRecordset(adOpenForwardOnly,
					adLockReadOnly,
					"SELECT TOP 1 SEX FROM M2000");
		if (rsMirror == NULL)
		{
			return FALSE;
		}

		switch (lType = rsMirror->Fields->Item[(long)0]->GetType())
		{
		case adVarWChar:
			bRes = TRUE;
			break;
		default:
			bRes = FALSE;
			break;
		}
		rsMirror->Close();
	}
	NxCatchAll("Error in determining Mirror version");
	return bRes;
}

_RecordsetPtr CreateMirrorRecordsetStd(const CString &strSql, ADODB::CursorTypeEnum nCursorType /*= adOpenForwardOnly*/, 
								 ADODB::LockTypeEnum nLockType /*= adLockReadOnly*/, long nOptions /*= adCmdText*/, ADODB::CursorLocationEnum nCursor /*= ADODB::adUseClient*/)
{
	// (c.haag 2009-03-31 12:06) - PLID 33630 - This will never be called if we're using the Canfield SDK.
//	if (IsMirror61()) // We encapsulate all data access in the Canfield Link
//		return NULL;

	// Create a recordset object
	_RecordsetPtr prs;
	HR(prs.CreateInstance(__uuidof(Recordset)));
	prs->CursorLocation = nCursor;

	// Open the recordset
	try {
		prs->Open(_variant_t(strSql), _variant_t((IDispatch *)g_pConMirror), nCursorType, nLockType, nOptions);
	}
	catch (_com_error&)
	{
		// There is an issue with some clients where an exception is raised that says
		// the mirror table could not be found. It is probably a race condition, so lets
		// close and reconnect to Mirror. We entrust the caller to catch the exception.
		g_pConMirror->Close();
		if (EnsureMirrorData())
			return NULL;
		prs->Open(_variant_t(strSql), _variant_t((IDispatch *)g_pConMirror), nCursorType, nLockType, nOptions);
	}
	
	// Return the now-open recordset
	return prs;
}

_RecordsetPtr CreateMirrorRecordset(ADODB::CursorTypeEnum nCursorType, ADODB::LockTypeEnum nLockType, LPCTSTR strFmt, ...)
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
	if (!IsMirrorEnabled()) {
		return NULL;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic (this is what we want)
			break;
		default:
			// Client is running (or trying to run) the Canfield SDK. This function is not supported
			return NULL;
	}

	CString strSql;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, strFmt);
	strSql.FormatV(strFmt, argList);
	va_end(argList);

	return CreateMirrorRecordsetStd(strSql, nCursorType, nLockType, adCmdText|adAsyncFetch);
}

void CMirrorPatient::LoadFromMirror(FieldsPtr &f)
{
	//does not set mirrorID, userID or personID

	COleDateTime now = COleDateTime::GetCurrentTime();
	COleDateTime invalid = COleDateTime(0,0,0,0,0,0);
	CString strSex;

	createTime	= AdoFldDateTime(f, "CREATETIME",	now);
	last		= AdoFldString	(f, "LASTNAME",		"");
	first		= AdoFldString	(f, "FIRSTNAME",	"");
	middle		= AdoFldString	(f, "MI",			"");
	dob			= AdoFldDateTime(f, "DOB",			invalid);
	homePhone	= AdoFldString	(f, "PHONE1",		"");
	workPhone	= AdoFldString	(f, "PHONE2",		"");
	address1	= AdoFldString	(f, "ADDRESS1",		"");
	address2	= AdoFldString	(f, "ADDRESS2",		"");
	city		= AdoFldString	(f, "CITY",			"");
	state		= AdoFldString	(f, "STATE",		"");
	zip			= AdoFldString	(f, "ZIP",			"");
	pin			= AdoFldString	(f, "PIN",			"");
	mrn			= AdoFldString	(f, "REFNO",		"");

	if (IsMirror60())
	{
		strSex			= AdoFldString  (f, "SEX",			"");
		if (strSex == "Male")
			lSex = 1;
		else if (strSex = "Female")
			lSex = 2;
		else
			lSex = 0;
	}
	else {
		lSex			= AdoFldLong	(f, "SEX",			0);
//		if (lSex != 1 && lSex != 2)
//				lSex = 0;

		switch (lSex)
		{
		case 0: // Female
			lSex = 2; break;
		case 1: // Male
			break;
		default: // Undefined
			lSex = 0; break;
		}
	}
}

void CMirrorPatient::LoadFromNextech(FieldsPtr &f)
{
	//does not set mirrorID, userID, personID

	COleDateTime now = COleDateTime::GetCurrentTime();
	COleDateTime invalid = COleDateTime(0,0,0,0,0,0);

	createTime	= AdoFldDateTime(f, "FirstContactDate",	now);
	last		= AdoFldString	(f, "Last",				"");
	first		= AdoFldString	(f, "First",			"");
	middle		= AdoFldString	(f, "Middle",			"");
	dob			= AdoFldDateTime(f, "BirthDate",		invalid);
	homePhone	= AdoFldString	(f, "HomePhone",		"");
	workPhone	= AdoFldString	(f, "WorkPhone",		"");
	address1	= AdoFldString	(f, "Address1",			"");
	address2	= AdoFldString	(f, "Address2",			"");
	city		= AdoFldString	(f, "City",				"");
	state		= AdoFldString	(f, "State",			"");
	zip			= AdoFldString	(f, "Zip",				"");
	
	// If the user wants to use the user defined ID as the mirror SSN.
	// get the user defined ID instead.
	if (GetLinkMirrorSSNToUserDefinedID())
	{
		char sz[128];
		pin		= itoa(AdoFldLong(f, "UserDefinedID",	0), sz, 10);
	}
	else
		pin		= AdoFldString	(f, "SocialSecurity",	"");

	if (GetLinkMirrorMRNToUserDefinedID())
	{
		char sz[128];
		mrn		= itoa(AdoFldLong(f, "UserDefinedID",	0), sz, 10);
	}
	else
		mrn		= "";

	lSex		= AdoFldByte	(f, "Gender",			0);
	if (lSex != 1 && lSex != 2)
		lSex = 0;
}

void CMirrorPatient::SaveNewToNextech()
{
	CString strDob, strFcd;

	if (dob.GetStatus() == COleDateTime::valid)
		strDob = dob.Format("'%m/%d/%Y'");
	else strDob = "NULL";

	if (createTime.GetStatus() == COleDateTime::valid)
		strFcd = createTime.Format("'%m/%d/%Y'");
	else strFcd = "NULL";

	try {
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("Import New Mirror Patient");
		trans.Begin();

		// (j.jones 2010-01-14 17:27) - PLID 31927 - check the default text message privacy field
		long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);
		
		personID = NewNumber("PersonT", "ID");
		userID = NewNumber("PatientsT", "UserDefinedID");

		// If the user wants to link the mirror SSN with the practice user defined id,
		// we will assign a new PIN to that mirror patient.
		if (GetLinkMirrorSSNToUserDefinedID())
		{
			CString str;
			pin.Format("%d", userID);

			// Update the mirror database too
			str.Format("UPDATE M2000 SET PIN = '%s' WHERE RECNUM = '%s'", pin, mirrorID);
			g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);

			// Add the patient to Practice
			// (j.jones 2010-01-14 17:27) - PLID 31927 - supported defaulting the text message privacy field
			ExecuteSql("INSERT INTO PersonT "
				"(ID, Location, FirstContactDate, Last, First, Middle, Birthdate, HomePhone, WorkPhone, "
				"Address1, Address2, City, State, Zip, Gender, TextMessage) "
				"SELECT %i, %d, %s, '%s', '%s', '%s', %s, '%s', '%s', "
				"'%s', '%s', '%s', '%s', '%s', %i, %li",
				personID, GetCurrentLocationID(), strFcd, _Q(last), _Q(first), _Q(middle), strDob, _Q(homePhone), _Q(workPhone), 
				_Q(address1), _Q(address2), _Q(city), _Q(state), _Q(zip), lSex, nTextMessagePrivacy);
		}
		else if (GetLinkMirrorMRNToUserDefinedID())
		{
			CString str;
			mrn.Format("%d", userID);

			// Update the mirror database too
			str.Format("UPDATE M2000 SET REFNO = '%s' WHERE RECNUM = '%s'", mrn, mirrorID);
			g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);

			// Add the patient to Practice
			// (j.jones 2010-01-14 17:27) - PLID 31927 - supported defaulting the text message privacy field
			ExecuteSql("INSERT INTO PersonT "
				"(ID, Location, FirstContactDate, Last, First, Middle, Birthdate, HomePhone, WorkPhone, "
				"Address1, Address2, City, State, Zip, Gender, SocialSecurity, TextMessage) "
				"SELECT %i, %d, %s, '%s', '%s', '%s', %s, '%s', '%s', "
				"'%s', '%s', '%s', '%s', '%s', %i, '%s', %li",
				personID, GetCurrentLocationID(), strFcd, _Q(last), _Q(first), _Q(middle), strDob, _Q(homePhone), _Q(workPhone), 
				_Q(address1), _Q(address2), _Q(city), _Q(state), _Q(zip), lSex, _Q(pin), nTextMessagePrivacy);
		}
		else {			
			// (j.jones 2010-01-14 17:27) - PLID 31927 - supported defaulting the text message privacy field
			ExecuteSql("INSERT INTO PersonT "
				"(ID, Location, FirstContactDate, Last, First, Middle, Birthdate, HomePhone, WorkPhone, "
				"Address1, Address2, City, State, Zip, Gender, SocialSecurity, TextMessage) "
				"SELECT %i, %d, %s, '%s', '%s', '%s', %s, '%s', '%s', "
				"'%s', '%s', '%s', '%s', '%s', %i, '%s', %li",
				personID, GetCurrentLocationID(), strFcd, _Q(last), _Q(first), _Q(middle), strDob, _Q(homePhone), _Q(workPhone), 
				_Q(address1), _Q(address2), _Q(city), _Q(state), _Q(zip), lSex, _Q(pin), nTextMessagePrivacy);
		}

		CString strProv;
		if(GetRemotePropertyInt("MirrorImportAssignProvider", 0) == 0)
			strProv = "NULL";
		else 
			strProv.Format("%li", GetRemotePropertyInt("MirrorImportProvider"));

		ExecuteSql("INSERT INTO PatientsT (PersonID, UserDefinedID, MirrorID, CurrentStatus, MainPhysician)"
			"SELECT %i, %i, '%s', 1, %s",
			personID, userID, mirrorID, strProv);

		// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
		ExecuteParamSql(
			"DECLARE @SecurityGroupID INT\r\n"
			"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
			"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
			"BEGIN\r\n"
			"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
			"END\r\n", personID);

		// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
		UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), personID);

		trans.Commit();

		//add to the patient toolbar
		// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
		GetMainFrame()->m_patToolBar.UpdatePatient(personID);
	} NxCatchAllThrow(__FUNCTION__);
}

// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - We weren't updating patient wellness qualification records 
// when a gender or dob changed; now we pass the boolean of whether it was changed back out, and the caller will 
// update the qualification records.
BOOL CMirrorPatient::SaveExistingToNextech(FieldsPtr &f, OUT BOOL &bUpdatedGender, OUT BOOL &bUpdatedBirthdate)
{
	bUpdatedGender = FALSE;
	bUpdatedBirthdate = FALSE;
	
	//if we are a new patient
/*	if (userID == -1)
	{	personID = NewNumber("PersonT", "ID");
		userID = NewNumber("PatientsT", "UserDefinedID");
		f->Item["PersonID"]->Value			= personID;
		f->Item["ID"]->Value				= personID;
		f->Item["UserDefinedID"]->Value		= userID;
	}*/


	// If the user wants to link the mirror ssn with the practice userdefined id,
	// save pin to the userdefinedid field.
	if (GetLinkMirrorSSNToUserDefinedID())
	{
		// Make sure the pin number is valid
		if (!atoi(pin))
		{
			MsgBox("You may not import a patient whose social security number in Mirror is empty");
			return TRUE;
		}
		// Make sure there are no alphabetical chars
		if (pin.FindOneOf("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-,./") != -1)
		{
			MsgBox("You may not import a patient who has non-numerical text in their social security number");
			return TRUE;
		}
		// Make sure there are no duplicates
		{
			_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 UserDefinedID FROM PatientsT WHERE UserDefinedID = %s AND PersonID <> %d", pin,
				personID);
			if (!rs->eof)
			{
				MsgBox("There is another patient in NexTech Practice whose ID matches the Mirror social security number. There may be more than one patient in Practice with the name %s %s. The import for this patient cannot continue.", first, last);
				return TRUE;
			}
		}
				
		f->Item["UserDefinedID"]->Value	= _variant_t((long)atoi(pin));
	}
	else
		f->Item["SocialSecurity"]->Value= _bstr_t(pin);


	if (GetLinkMirrorMRNToUserDefinedID())
	{
		// Make sure there are no alphabetical chars
		// (c.haag 2008-03-18 08:23) - PLID 29302 - Check the MRN for invalid characters, not the SSN!
		if (!IsNumeric(mrn))
		{
			MsgBox("You may not import a patient who has non-numerical text in their MRN");
			return TRUE;
		}
		// Make sure the pin number is valid
		const long nMRN = atol(mrn);
		if (0 == nMRN)
		{
			MsgBox("You may not import a patient whose MRN in Mirror is empty");
			return TRUE;
		}
		// Make sure there are no duplicates
		{
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 UserDefinedID FROM PatientsT WHERE UserDefinedID = {INT} AND PersonID <> {INT}", nMRN, personID);
			if (!rs->eof)
			{
				MsgBox("There is another patient in NexTech Practice whose ID matches the Mirror MRN. There may be more than one patient in Practice with the name %s %s. The import for this patient cannot continue.", first, last);
				return TRUE;
			}
		}

		f->Item["UserDefinedID"]->Value	= _variant_t(nMRN);
	}


	f->Item["MirrorID"]->Value			= _bstr_t(mirrorID);
	f->Item["CurrentStatus"]->Value		= 1L;
	f->Item["FirstContactDate"]->Value	= _variant_t(createTime);
	f->Item["Last"]->Value				= _bstr_t(last);
	f->Item["First"]->Value				= _bstr_t(first);
	f->Item["Middle"]->Value			= _bstr_t(middle);
	if (dob.GetStatus() == COleDateTime::valid) {
		COleDateTime dtCurBirthDate = VarDateTime(f->Item["BirthDate"]->Value);
		if (dtCurBirthDate.GetStatus() != COleDateTime::valid || dob != dtCurBirthDate) {
			f->Item["BirthDate"]->Value		= _variant_t(dob);
			bUpdatedBirthdate = TRUE;
		}
	}
	f->Item["HomePhone"]->Value			= _bstr_t(homePhone);
	f->Item["WorkPhone"]->Value			= _bstr_t(workPhone);
	f->Item["Address1"]->Value			= _bstr_t(address1);
	f->Item["Address2"]->Value			= _bstr_t(address2);
	f->Item["City"]->Value				= _bstr_t(city);
	f->Item["State"]->Value				= _bstr_t(state);
	f->Item["Zip"]->Value				= _bstr_t(zip);
	
	if (lSex != VarLong(f->Item["Gender"]->Value)) {
		f->Item["Gender"]->Value			= lSex;
		bUpdatedGender = TRUE;
	}


	if (GetRemotePropertyInt("MirrorImportAssignProviderExisting", 0))
	{
		if (f->Item["MainPhysician"]->Value.vt == VT_NULL)
			f->Item["MainPhysician"]->Value = GetRemotePropertyInt("MirrorImportProvider");
	}

	return FALSE;
}

void CMirrorPatient::SaveToMirror(FieldsPtr &f)
{
	CString user = GetCurrentUserName();
	COleDateTime now = COleDateTime::GetCurrentTime();
	
	//should write all fields
	f->Item["RECNUM"]->Value		= _bstr_t(mirrorID);
	f->Item["CREATETIME"]->Value	= _variant_t(createTime);
	f->Item["LASTNAME"]->Value		= _bstr_t(last.Left(20));
	f->Item["FIRSTNAME"]->Value		= _bstr_t(first.Left(20));
	f->Item["MI"]->Value			= _bstr_t(middle.Left(1));
	if (dob.GetStatus() == COleDateTime::valid)
		f->Item["DOB"]->Value		= _variant_t(dob);
	f->Item["PHONE1"]->Value		= _bstr_t(homePhone.Left(15));
	f->Item["PHONE2"]->Value		= _bstr_t(workPhone.Left(15));
	f->Item["ADDRESS1"]->Value		= _bstr_t(address1.Left(30));
	f->Item["ADDRESS2"]->Value		= _bstr_t(address2.Left(30));
	f->Item["CITY"]->Value			= _bstr_t(city.Left(25));
	f->Item["STATE"]->Value			= _bstr_t(state.Left(6));
	f->Item["ZIP"]->Value			= _bstr_t(zip.Left(15));
	f->Item["PIN"]->Value			= _bstr_t(pin.Left(15));

	if (GetLinkMirrorMRNToUserDefinedID())
		f->Item["REFNO"]->Value		= _bstr_t(mrn.Left(30));

	if (IsMirror60())
		f->Item["SEX"]->Value		= (lSex == 1 ? _bstr_t("Male") : (lSex == 2 ? _bstr_t("Female") : _bstr_t("")));
	else switch (lSex)
	{
		case 1: f->Item["SEX"]->Value = (long)1; break; // Male
		case 2: f->Item["SEX"]->Value = (long)0; break; // Female
		default: f->Item["SEX"]->Value = (long)2; break; // Undefined
	}


	f->Item["MODTIME"]->Value		= _variant_t(now);
	f->Item["MODUSER"]->Value		= _bstr_t(user.Left(20));

	if (f->Item["CREATEUSER"]->Value.vt != VT_BSTR)
		f->Item["CREATEUSER"]->Value = _bstr_t(user.Left(20));

	//update the link
	Link(userID, mirrorID);
}
///////////////////////////////////////////////////////////////////////////////////
//properties
//_ConnectionPtr g_pConMirror;
CString g_strMirrorDataPath;
CString g_strLastImageFileName;

//public functions

LPDISPATCH GetMirrorData()
{
	if (!IsMirrorEnabled())
		return NULL;
	if (EnsureMirrorData())
		return NULL;
	return g_pConMirror;
}

void CloseMirrorLink()
{
	// This is a safe operation even if Mirror 6.1 is not installed
	CanfieldLink::DisconnectLink();
}

void ResetMirrorDataPath()//reset path to data stored in database - usually for when another user changed it
{
	g_strMirrorDataPath = "";
	g_pConMirror = NULL;
}

CString GetMirrorDataPath()
{
	// (c.haag 2006-12-01 10:39) - PLID 23725 - It should not matter if we are using
	// the Mirror 6.1 link or not
	//if (IsMirror61())
	//	return "";

	//if we are open to a database already, don't send a different path, even if someone changed it
	if (g_strMirrorDataPath == ""){
		//(e.lally 2011-08-26) PLID 44950 - Auto-create the pref so we don't have the added round-trip the first time every login.
		return GetRemotePropertyText("MirrorDataPath", 0, 0, "<None>", true);
	}
	else return g_strMirrorDataPath;
}

bool BrowseMirrorPath()
{
	CString strInitPath,
			strInOutPath;

	CFileDialog dlgBrowse(TRUE, "mdb", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Mirror Database Files|*.mde;*.mdb|All Files|*.*|");

	strInitPath = GetMirrorDataPath(); // We need to store this because the next line is a pointer to it
	if (strInitPath != "")
		GetFilePath(strInitPath);
	else strInitPath = "c:\\";

	dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
	if (dlgBrowse.DoModal() == IDOK) 
	{	// If the user clicked okay, that means she selected a file so remember it
		CString str = dlgBrowse.GetPathName();

		// Store the path
		SetMirrorDataPath(str);
		return true;
	} 
	return false;
}

CString GetMirrorImagePath()
{
	// (c.haag 2006-12-01 10:39) - PLID 23725 - It should not matter if we are using
	// the Mirror 6.1 link or not
	//if (IsMirror61())
	//	return "";

	CString path = GetRemotePropertyText("MirrorImagePath");
	if (path == "")
	{	path = GetMirrorDataPath();
		path = path.Left(path.ReverseFind('\\'));
		return path ^ "pics";
	}
	else return path;
}

HRESULT FillList61(NXDATALISTLib::_DNxDataListPtr& dlList)
{
	return CanfieldLink::FillList(dlList);
}

HRESULT FillList61Incremental(NXDATALISTLib::_DNxDataListPtr& dlList, long nStep, CString& strRecnum)
{
	return CanfieldLink::FillListIncremental(dlList, nStep, strRecnum);
}

HRESULT CancelFillList61()
{
	return CanfieldLink::CancelFillList();
}

void SetMirrorDataPath(const CString &path)
{
	// (c.haag 2006-12-01 10:39) - PLID 23725 - It should not matter if we are using
	// the Mirror 6.1 link or not
	//if (IsMirror61())
	//	return;

	try
	{
		SetRemotePropertyText("MirrorDataPath", path);
		CClient::RefreshTable(NetUtils::MirrorDataPath);

		if (g_pConMirror != NULL && g_pConMirror->State == adStateOpen)
		{	g_pConMirror->Close();
			g_strMirrorDataPath = "";
		}
	}
	NxCatchAll("Could not store mirror path");
}

void SetMirrorImagePath(const CString &path)
{
	// (c.haag 2006-12-01 10:39) - PLID 23725 - It should not matter if we are using
	// the Mirror 6.1 link or not
	//if (IsMirror61())
	//	return;

	CClient::RefreshTable(NetUtils::MirrorImagePath);

	SetRemotePropertyText("MirrorImagePath", path);
}

BOOL GetHighRes()
{
	// (c.haag 2009-03-31 11:34) - PLID 33630 - This is a practice-specific property.
	// No need to get the DLL involved.
	return GetRemotePropertyInt("HiRes", TRUE) ? TRUE : FALSE;
}

void SetHighRes(BOOL bHighRes)
{
	// (c.haag 2009-03-31 11:34) - PLID 33630 - This is a practice-specific property.
	// No need to get the DLL involved.
	SetRemotePropertyInt("HiRes", bHighRes ? TRUE : FALSE);
}

// (c.haag 2006-10-23 11:00) - PLID 23181 - This functionality is now depreciated
// in favor of the "Show primary thumbnail only" view option in the patients module,
// which now applies to both Practice and Mirror thumbs.

/*BOOL GetPrimaryImageOnly()
{
//	if (!IsMirror61())
		return FALSE;

	if (GetRemotePropertyInt("MirrorPrimaryImageOnly", FALSE))
		return TRUE;
	else return FALSE;
}

void SetPrimaryImageOnly(BOOL bPrimaryImageOnly)
{
//	if (!IsMirror61())
		return;

	if (bPrimaryImageOnly)
		SetRemotePropertyInt("MirrorPrimaryImageOnly", TRUE);
	else SetRemotePropertyInt("MirrorPrimaryImageOnly", FALSE);
}*/

BOOL GetLinkMirrorSSNToUserDefinedID()
{
	// (c.haag 2009-03-31 11:28) - PLID 33630 - No reason to test the link integrity; just get the property
	// from here because it should always match in the DLL
	return GetRemotePropertyInt("LinkMirrorSSNToUserDefinedID", FALSE) ? TRUE : FALSE;
}

BOOL GetLinkMirrorMRNToUserDefinedID()
{
	// (c.haag 2009-03-31 11:28) - PLID 33630 - No reason to test the link integrity; just get the property
	// from here because it should always match in the DLL
	return GetRemotePropertyInt("LinkMirrorMRNToUserDefinedID", FALSE) ? TRUE : FALSE;
}

void SetLinkMirrorSSNToUserDefinedID(BOOL bLinkMirrorSSNToUserDefinedID)
{
	// (c.haag 2009-03-31 11:28) - PLID 33630 - No reason to test the link integrity; this is a simple property set
	CanfieldLink::SetLinkMirrorSSNToUserDefinedID(bLinkMirrorSSNToUserDefinedID);
	SetRemotePropertyInt("LinkMirrorSSNToUserDefinedID", bLinkMirrorSSNToUserDefinedID ? TRUE : FALSE);
}

void SetLinkMirrorMRNToUserDefinedID(BOOL bLinkMirrorMRNToUserDefinedID)
{
	// (c.haag 2009-03-31 11:28) - PLID 33630 - No reason to test the link integrity; this is a simple property set
	CanfieldLink::SetLinkMirrorMRNToUserDefinedID(bLinkMirrorMRNToUserDefinedID);
	SetRemotePropertyInt("LinkMirrorMRNToUserDefinedID", bLinkMirrorMRNToUserDefinedID ? TRUE : FALSE);
}


// (c.haag 2009-03-31 12:41) - PLID 33630 - The old access export logic is encapsulated here
MirrorResult Export_PreCanfieldSDK(int nID, CString &recnum, BOOL bAssumeOneMatchingNameLinks)
{
	CMirrorPatient pat;
	CString last, first, middle, strSSN;
	BOOL bRetried = FALSE;

	try
	{
		if (EnsureMirrorData())
			return Stop;

		//Open Practice Patient
		_RecordsetPtr rsNextech = CreateRecordset(adOpenKeyset, adLockBatchOptimistic, 
			"SELECT MirrorID, FirstContactDate, Last, First, Middle, BirthDate, "
			"HomePhone, WorkPhone, Address1, Address2, City, State, Zip, Gender, SocialSecurity, UserDefinedID "
			"FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE UserDefinedID = %i", nID);

		//Get Recnum, and see if we are on an old mirror patient
		FieldsPtr fNextech = rsNextech->Fields;
		variant_t var;
		bool existing = false;

		if (!rsNextech->eof)
		{
			var = fNextech->Item["MirrorID"]->Value;
			if (var.vt == VT_BSTR)
			{	pat.mirrorID = VarString(var);
				existing = true;
			}

			//check for duplicates in mirror
			CExportDuplicates dlg(NULL);
			last = AdoFldString(fNextech,"Last","");
			first = AdoFldString(fNextech,"First","");
			middle = AdoFldString(fNextech,"Middle","");

			// If the user wants to use the user defined ID as the mirror SSN.
			// look up the user defined ID instead.
			if (GetLinkMirrorSSNToUserDefinedID())
			{
				char sz[128];
				strSSN = itoa(AdoFldLong(fNextech, "UserDefinedID",	0), sz, 10);
			}
			else
				strSSN = AdoFldString(fNextech,"SocialSecurity","");

			//only check if we are exporting a new patient.
			if(existing == false) {
				if(dlg.FindDuplicates(first, last, middle, "Practice", "Mirror", g_pConMirror,Mirror::GetMirrorDataPath(), TRUE, "", bAssumeOneMatchingNameLinks, strSSN)) {
					int choice = dlg.DoModal();
					switch(choice) {
					case(1): //add new
						existing = false;
						//continue normally
						break;
					case(2): //update to selected patient
						existing = true;
						pat.mirrorID = CString(dlg.m_varIDToUpdate.bstrVal);
						//continue normally
						break;
					case(3): //skip patient
						return Skip;
						break;
					default: //stop exporting
						return Stop;
						break;
					}
				}
			}
		}

		if (!existing)
			pat.mirrorID = GetUniqueMirrorID(COleDateTime::GetCurrentTime());

		// (c.haag 2004-06-03 09:03) - PLID 12811 - We need to prevent
		// exports from overwriting Mirror data in some cases
		if (existing && !GetRemotePropertyInt("MirrorExportOverwrite", 1))
		{
			Link(nID, pat.mirrorID);
			recnum = pat.mirrorID;
			return Update;
		}

		//open the mirror patient
		CString sql;

		sql.Format("SELECT RECNUM, CREATETIME, MODTIME, CREATEUSER, MODUSER, LASTNAME, FIRSTNAME, MI, DOB, PHONE1, PHONE2, PIN, "
			"ADDRESS1, ADDRESS2, CITY, STATE, ZIP, SEX, REFNO FROM M2000 WHERE RECNUM = '%s'", pat.mirrorID);

		_RecordsetPtr rsMirror;

		HR(rsMirror.CreateInstance(__uuidof(Recordset)));


retry_save:

		rsMirror->Open(_variant_t(_bstr_t(sql)), 
			_variant_t((IDispatch *)g_pConMirror), 
			adOpenKeyset, 
			adLockBatchOptimistic,
			adOptionUnspecified);

		if (!XOR(existing, rsMirror->eof))
		{	if (existing)//don't ASSERT - since Yazi tries this all the time.
				MsgBox("Could not export data for %s %s to Mirror.\n"
					"This patient exists in Practice, but is linked to a missing record in Mirror.",
					first, last);
			else 
			{	MsgBox("Could not export a new patient to Mirror\n"
					"An existing patient with the same ID was found");
			}
			return Skip;
		}

		try {
			if (!existing)
				rsMirror->AddNew();

			FieldsPtr fMirror = rsMirror->Fields;

			//now we have both patients open
			pat.userID = nID;
			pat.LoadFromNextech(fNextech);
			pat.SaveToMirror(fMirror);
			rsMirror->UpdateBatch(adAffectAll);

			CClient::RefreshTable(NetUtils::MirrorPatients);
			recnum = pat.mirrorID;

			if (existing)
				return Update;
			else return Addnew;
		}
		catch (_com_error& e)
		{
			if (bRetried) // Give up after one attempt
				throw e;
			else if (e.Error() == 0x80004005) // Connection failure - lets try this again
			{
				bRetried = TRUE;
				rsMirror->Close();
				g_pConMirror->Close();
				if (EnsureMirrorData())
					return Stop;
				goto retry_save;
			}
		}
	}
	NxCatchAll("Could not send patient to mirror");
	return Stop;
}

MirrorResult Export(int nID, CString &recnum, BOOL bAssumeOneMatchingNameLinks)
{
	CWaitCursor wc;

	// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
	if (!IsMirrorEnabled()) {
		return Stop;
	}
	ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
	switch (result) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			return Export_PreCanfieldSDK(nID, recnum, bAssumeOneMatchingNameLinks);
		case eCSDK_Failure_Link_Disabled:
		case eCSDK_Failure_TimeoutExpired:
		case eCSDK_Failure_Error:
			// Failure
			return Stop;
		default:
			break;
	}

	// (c.haag 2006-07-24 16:24) - PLID 21588 - Make sure the backup progress window is not visible
	CNxSuppressBkpProgress sbp;

	// Get the person ID from the user defined ID
	_RecordsetPtr prs = CreateRecordset("SELECT PersonID FROM PatientsT WHERE UserDefinedID = %d", nID);
	if (prs->eof)
	{
		MsgBox("The patient with the ID %d was not found in the database. Somebody else may have changed or deleted the patient with this ID. Please refresh the patient (if you are in the Link to Mirror window, close and reopen the window) and try again.", nID);
		return Stop;
	}
	nID = AdoFldLong(prs, "PersonID");
	prs->Close();
	// (c.haag 2004-11-23 16:37) - PLID 14783 - Needed for auditing
	CanfieldLink::SetCurrentUserName(GetCurrentUserName());
	CanfieldLink::SetCurrentLocationName(GetCurrentLocationName());
	switch (CanfieldLink::ExportPatient(nID, recnum, bAssumeOneMatchingNameLinks))
	{
	case CanfieldLink::eAddNew:
		return Addnew;
	case CanfieldLink::eUpdate:
		return Update;
	case CanfieldLink::eSkip:
		return Skip;
	}
	return Stop;
}

// (c.haag 2009-03-31 12:41) - PLID 33630 - The old access export logic is encapsulated here
MirrorResult Import_PreCanfieldSDK(const CString &recnum, long &userID, BOOL bAssumeOneMatchingNameLinks)
{
	CMirrorPatient	pat;
	_RecordsetPtr	rsNextech, 
					rsMirror;
	FieldsPtr		fNextech, 
					fMirror;
	bool			existing = false;
	variant_t		var;
	CString			sql;

	try
	{
		if (EnsureMirrorData())
			return Stop;

		//Open Practice Patient
		rsNextech = CreateRecordset(adOpenKeyset, adLockOptimistic, 
			"SELECT ID, PersonID, UserDefinedID, MirrorID, "
				"FirstContactDate, Last, First, Middle, BirthDate, CurrentStatus, "
				"HomePhone, WorkPhone, Address1, Address2, City, State, Zip, Gender, SocialSecurity, MainPhysician "
			"FROM PersonT INNER JOIN PatientsT ON PersonID = ID "
			"WHERE MirrorID = '%s'", recnum);

		//Get userID, and see if we are on an existing nextech patient
		fNextech = rsNextech->Fields;

		if (!rsNextech->eof)
		{	var = fNextech->Item["UserDefinedID"]->Value;
			if (var.vt == VT_I4)
			{	pat.userID = VarLong(var);
				// (c.haag 2005-12-23 PLID 18630) - We need to set the person
				// ID here so that the callign function gets the proper person ID
				// returned to it.
				pat.personID = AdoFldLong(fNextech, "PersonID");
				existing = true;
			}
		}

		//open the mirror patient
		sql.Format("SELECT RECNUM, CREATETIME, MODTIME, CREATEUSER, MODUSER, LASTNAME, FIRSTNAME, MI, DOB, PHONE1, PHONE2, PIN, "
			"ADDRESS1, ADDRESS2, CITY, STATE, ZIP, SEX, REFNO FROM M2000 WHERE RECNUM = '%s'", recnum);

		HR(rsMirror.CreateInstance(__uuidof(Recordset)));
		rsMirror->Open(_variant_t(_bstr_t(sql)), 
			_variant_t((IDispatch *)g_pConMirror), 
			adOpenKeyset, 
			adLockBatchOptimistic,
			adOptionUnspecified);

		fMirror = rsMirror->Fields;
		pat.mirrorID = recnum;
		pat.LoadFromMirror(fMirror);

		if(!rsMirror->eof && existing == false) {
			//check for duplicates in mirror
			CExportDuplicates dlg(NULL);
			CString last, first, middle;
			last = AdoFldString(fMirror,"LASTNAME","");
			first = AdoFldString(fMirror,"FIRSTNAME","");
			middle = AdoFldString(fMirror,"MI","");

			if(dlg.FindDuplicates(first, last, middle, "Mirror", "Practice", g_pConMirror,Mirror::GetMirrorDataPath())) {
				int choice = dlg.DoModal();
				switch(choice) {
				case(1): //add new
					existing = false;
					//continue normally
					break;
				case(2): //update to selected patient
					{
					//reopen Practice Patient
					rsNextech->Close();
					
					_RecordsetPtr prsPatientID = CreateRecordset("SELECT UserDefinedID FROM PatientsT WHERE PersonID = %d",
						dlg.m_varIDToUpdate.lVal);
					if (!prsPatientID->eof)
					{
						Link(AdoFldLong(prsPatientID, "UserDefinedID"), recnum);
					}
					prsPatientID->Close();

					rsNextech = CreateRecordset(adOpenKeyset, adLockOptimistic, 
						"SELECT ID, PersonID, UserDefinedID, MirrorID, "
							"FirstContactDate, Last, First, Middle, BirthDate, CurrentStatus, "
							"HomePhone, WorkPhone, Address1, Address2, City, State, Zip, Gender, SocialSecurity "
						"FROM PersonT INNER JOIN PatientsT ON PersonID = ID "
						"WHERE MirrorID = '%s'", recnum);

					//Get userID, and see if we are on an existing nextech patient
					fNextech = rsNextech->Fields;

					if (!rsNextech->eof)
					{	var = fNextech->Item["UserDefinedID"]->Value;
						if (var.vt == VT_I4)
						{	pat.userID = VarLong(var);
							// (c.haag 2005-12-23 PLID 18630) - We need to set the person
							// ID here so that the callign function gets the proper person ID
							// returned to it.
							pat.personID = AdoFldLong(fNextech, "PersonID");
							existing = true;
						}
					}
					//continue normally
					}
					break;
				case(3): //skip patient
					return Skip;
					break;
				default: //stop exporting
					return Stop;
					break;
				}
			}
		}

		if (GetRemotePropertyInt("MirrorImportOverwrite", 0) || !existing)
		{
			if (existing)
			{	pat.personID = AdoFldLong(fNextech, "PersonID");
				fNextech = rsNextech->Fields;
				BOOL bUpdatedGender = FALSE, bUpdatedBirthdate = FALSE;
				if (!pat.SaveExistingToNextech(fNextech, bUpdatedGender, bUpdatedBirthdate)) {
					rsNextech->UpdateBatch(adAffectAll);
					
					// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - If the gender or dob changed, 
					// update the patient wellness qualification records
					if (bUpdatedGender) {
						UpdatePatientWellnessQualification_Gender(GetRemoteData(), pat.personID);
					}
					if (bUpdatedBirthdate) {
						UpdatePatientWellnessQualification_Age(GetRemoteData(), pat.personID);
					}
				}
			}
			else pat.SaveNewToNextech();

			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here (Mirror always imports as an Active Patient)
			CClient::RefreshPatCombo(pat.personID, false, CClient::pcatActive, CClient::pcstPatient);
		}
		userID = pat.personID;

		if (existing)
			return Update;
		else return Addnew;
	}
	NxCatchAll("Could not send patient to practice");
	return Stop;
}

MirrorResult Import(const CString &recnum, long &userID, BOOL bAssumeOneMatchingNameLinks /*=FALSE*/)
{
	CWaitCursor wc;

	// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
	if (!IsMirrorEnabled()) {
		return Stop;
	}
	ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
	switch (result) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			return Import_PreCanfieldSDK(recnum, userID, bAssumeOneMatchingNameLinks);
		case eCSDK_Failure_Link_Disabled:
		case eCSDK_Failure_TimeoutExpired:
		case eCSDK_Failure_Error:
			// Failure
			return Stop;
		default:
			break;
	}

	// (c.haag 2006-07-24 16:24) - PLID 21588 - Make sure the backup progress window is not visible
	CNxSuppressBkpProgress sbp;

	// (c.haag 2004-11-23 16:37) - PLID 14783 - Needed for auditing
	CanfieldLink::SetCurrentUserName(GetCurrentUserName());
	CanfieldLink::SetCurrentLocationName(GetCurrentLocationName());
	switch (CanfieldLink::ImportPatient(userID, recnum, bAssumeOneMatchingNameLinks))
	{
	case CanfieldLink::eAddNew:
		//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here (Mirror always imports as an Active Patient)
		CClient::RefreshPatCombo(userID, false, CClient::pcatActive, CClient::pcstPatient);
		return Addnew;
	case CanfieldLink::eUpdate:
		CClient::RefreshTable(NetUtils::PatCombo, userID);
		return Update;
	case CanfieldLink::eSkip:
		return Skip;
	case CanfieldLink::eError:
		MsgBox("An error occured and the import failed");
		return Stop;
	}
	return Stop;
}

// (c.haag 2009-03-31 12:47) - PLID 33630 - Encapsulated old code here
HRESULT Link_PreCanfieldSDK(int nID, const CString &recnum)
{
	try
	{	
		ExecuteSql("UPDATE PatientsT SET MirrorID = '%s' WHERE UserDefinedID = %i", _Q(recnum), nID);

		_RecordsetPtr prs = CreateRecordset("SELECT PersonID FROM PatientsT WHERE UserDefinedID = %d",
			nID);
		if (!prs->eof) {
			long AuditID = BeginNewAuditEvent();
			if(AuditID != -1) {
				AuditEvent(-1, "",AuditID,aeiPatientMirrorID,AdoFldLong(prs, "PersonID"),"",recnum,aepLow);
			}
		}
		return S_OK;
	}
	NxCatchAll("Could not link patient");
	return 0x80000000/*S_FAILED*/;
}

// (c.haag 2008-02-06 12:42) - PLID 28622 - We require a return value so that the
// Link Common Patients dialog can abort the linking process if an error occurs.
// The CanfieldLink library returns S_FAILED if LinkPatient fails, or 0 (S_OK) on
// success. We will do the same here.
HRESULT Link(int nID, const CString &recnum)
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 - Use new logic to handle Mirror connectivity
	if (!IsMirrorEnabled()) {
		return 0x80000000/*S_FAILED*/;
	}
	ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
	if (eCSDK_Failure_Link_SDK_Disabled == result || eCSDK_Failure_NotAvailable == result) {
		// Defer to pre-SDK logic
		return Link_PreCanfieldSDK(nID, recnum);
	}
	if (eCSDK_Success == result) {
		return CanfieldLink::LinkPatient(nID, recnum);
	} else {
		// Failure
		return 0x80000000/*S_FAILED*/;
	}
}

// (c.haag 2009-03-31 12:47) - PLID 33630 - Encapsulated old code here
void Unlink_PreCanfieldSDK(int nID)
{
	try
	{
		ExecuteSql("UPDATE PatientsT SET MirrorID = NULL WHERE UserDefinedID = %i", nID);

		_RecordsetPtr prs = CreateRecordset("SELECT PersonID FROM PatientsT WHERE UserDefinedID = %d",
			nID);
		if (!prs->eof) {
			long AuditID = BeginNewAuditEvent();
			if(AuditID != -1) {
				AuditEvent(-1, "",AuditID,aeiPatientMirrorID,AdoFldLong(prs, "PersonID"),"","<unlinked>",aepLow);
			}
		}
	}	
	NxCatchAll("Could not unlink patient");
}

void Unlink(int nID)
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 - Use new logic to handle Mirror connectivity
	if (!IsMirrorEnabled()) {
		return;
	}
	ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
	if (eCSDK_Failure_Link_SDK_Disabled == result || eCSDK_Failure_NotAvailable == result) {
		// Defer to pre-SDK logic
		Unlink_PreCanfieldSDK(nID);
	}
	else if (eCSDK_Success == result) {
		// (c.haag 2004-11-23 16:37) - PLID 14783 - Needed for auditing
		CanfieldLink::SetCurrentUserName(GetCurrentUserName());
		CanfieldLink::SetCurrentLocationName(GetCurrentLocationName());
		CanfieldLink::UnlinkPatient(nID);
	}
}

// (c.haag 2009-03-31 15:05) - PLID 33630 - Moved from LoadMirrorImage into its own function
HBITMAP LoadMirrorImage_PreCanfieldSDK(const CString &recnum, long &nIndex, long &nCount, long nQualityOverride)
{
	_RecordsetPtr	rs;
	CString			sql, 
					file;
	HBITMAP			image;
	long nOriginalIndex;
	BOOL bFailed = TRUE;

	if (nIndex < 0)	nIndex = 0;

	if (EnsureMirrorData())
	{
		nIndex = -1;
		return NULL;
	}

	try {
		rs = CreateMirrorRecordset(adOpenForwardOnly,
				adLockReadOnly,
					"SELECT [IMAGE].IMAGE "
					"FROM [IMAGE] "
					"WHERE LINKNUM = '%s' "
					"ORDER BY IMAGENUMBER ", recnum);

		// Abort if there are no records
		if (rs->eof)
		{	
			nIndex = -1;
			return NULL;
		}

		// Establish how many thumbs there are, and bind nIndex to that range
		nCount = rs->RecordCount;
		if (nCount <= nIndex)
			nIndex = nCount - 1;

		// Move to the record
		rs->Move(nIndex);

		// Store the index for determining if we failed or not
		nOriginalIndex = nIndex;
		//do {
			file = AdoFldString(rs, "IMAGE", "");

			if (!file.IsEmpty())
			{
				file = GetMirrorImagePath() ^ file;
				g_strLastImageFileName = file;

				if (GetHighRes())
					image = T2KGetModifiedBitmap(file);
				else
					image = T2KGetStampBitmap(file);

				if (image != NULL)
				{
					bFailed = FALSE;
				}
			}

		/*	if (bFailed)
			{
				if (nIndex == nCount - 1)
					rs->MoveFirst();
				else
					rs->MoveNext();
				nIndex = (nIndex + 1) % nCount;
			}
		} while (nIndex != nOriginalIndex && bFailed);*/

		if (!bFailed)
		{
			return DDBToDIB(GetDC(NULL), image);
		}
	}
	// (a.walling 2007-07-20 11:09) - PLID 26762 - Use threaded exception handling (called from thread)
	NxCatchAllThread("Could not load patient's mirror image");

	//nIndex = -1;
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
//private functions
BOOL EnsureMirrorData()
{
	BOOL bRetry = FALSE;

	// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
	if (!IsMirrorEnabled()) {
		return FALSE;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic (this is what we want)
			break;
		default:
			// Client is running (or trying to run) the Canfield SDK. This function is not supported
			return FALSE;
	}

	if (Mirror::GetMirrorDataPath() == "")
		return TRUE;

	do {
		try
		{
			if (g_pConMirror == NULL)
				g_pConMirror.CreateInstance(__uuidof(Connection));

			if (g_pConMirror == NULL)
				AfxThrowNxException("Could not create Mirror Connection object");
			
			if (g_pConMirror->State == adStateClosed)
			{
				CShowConnectingFeedbackDlg dlgConnecting;
				dlgConnecting.SetWaitMessage("Please wait while Practice connects to your Mirror database...");

				g_pConMirror.Release();
				g_pConMirror = NULL;
				g_pConMirror.CreateInstance(__uuidof(Connection));


				CString con, 
						path = Mirror::GetMirrorDataPath();

				con =	"Provider=Microsoft.Jet.OLEDB.4.0;"
						"Data Source=" + path + ";"
						"User Id=admin;"
						"Password=;";

				HR(g_pConMirror->Open(_bstr_t((LPCTSTR)con), "","",NULL));
				g_pConMirror->CursorLocation = adUseClient; // ???
				// (c.haag 2006-02-20 11:20) - PLID 17472 - In the event we get disconnected from
				// the SQL Server, we need to have the Provider Friendly Name so that we can match
				// it up with the thrown _com_error. The problem is that if the SQL Server is down,
				// you can't get the Provider Friendly Name. So, we need to get it ahead of time.
				//g_strMirrorPFN = GetConnectionPropertyAsString(g_ptrRemoteData, "Provider Friendly Name",
				//		"This is hopefully always an invalid source nameas90hfa-zdh8vd9zu");
			}
			bRetry = FALSE;
		}
		catch (_com_error& e)
		{
			g_pConMirror.Release();
			g_pConMirror = NULL;

			if (e.Error() == 0x80004005)
			{
				if (IDYES == MsgBox(MB_YESNO, "Practice could not access your Mirror data. You may be improperly connected to your Mirror server on your Windows network. Would you like to try connecting again?")) {
					if (NO_ERROR == TryAccessNetworkFile(Mirror::GetMirrorDataPath())) {
						bRetry = TRUE;
						continue;
					}
				}

				MsgBox("Your Mirror database is not accessible at this time. The link from NexTech Practice to Mirror will be disabled for the remainder of this program session. To re-enable it, please go to the Mirror Settings dialog and verify you are properly connected to your Mirror data.");
				g_bForceDisable = TRUE;
				bRetry = FALSE;
			}
		}
	} while (bRetry);

	if (g_pConMirror == NULL) return TRUE;
	if (g_pConMirror->State == adStateClosed) return TRUE;
//	if (!IsMirror61()) g_bForceDisable = FALSE;
	return FALSE;

/*	catch (_com_error &e)
	{
		if (e.Error() != 0x80004005)
			throw e;
	}

	HWND hWnd = GetMainFrame()->m_hWnd;
	if (IDYES == MessageBox(
		hWnd,
		"The Mirror Link has not been properly set.\r\n"
		"Do you want to browse for your mirror data?", 
		"Mirror data could not be opened", 
		MB_YESNO))
	{
		if (BrowseMirrorPath())
		{
			EnsureMirrorData();
			return;
		}
	}

	if (!GetMirrorDataPath().IsEmpty())
	{
		if (IDYES == MessageBox(hWnd, "Do you want to disable integration with Canfield Mirror?", "", MB_YESNO))
			SetMirrorDataPath("");
		else
			MsgBox("Could not link to mirror data");
	}
	AfxThrowUserException();*/
}

static CString GetUniqueMirrorID(COleDateTime &CreateTime)
{
	//1997  01 03  18 28 13 390	= 1/3/1997 6:28:13 PM
	CString val;

	val.Format("%03i", rand() % 1000);		
	val = CreateTime.Format("%Y%m%d%H%M%S") + val;

	return val;
}

// (c.haag 2009-03-31 14:58) - PLID 33630 - I moved the contained code
// from GetImageCount to this function
long GetImageCount_PreCanfieldSDK(const CString &strMirrorId)
{
	if (EnsureMirrorData())
	{
		return 0;
	}

	try {
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,
			"SELECT COUNT(IMAGE.IMAGE) AS ImageCount "
			"FROM [IMAGE] "
			"WHERE LINKNUM = '%s' ", strMirrorId);

		if (prs->eof) {
			HR(prs->Close());
			return 0;
		} else {
			long nAns = AdoFldLong(prs, "ImageCount");
			prs->Close();
			return nAns;
		}
	}
	catch (_com_error&)
	{
		Log("GetImageCount() failed");
	}
	return -1;
}

long GetPatientCount()
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 - Use new logic to test Mirror connectivity
	if (!IsMirrorEnabled()) {
		return -1;
	}
	if (eCSDK_Success == InitCanfieldSDK(TRUE)) {
		return CanfieldLink::GetPatientCount();
	} else {
		return -1;
	}
}

// (c.haag 2010-02-24 10:32) - PLID 37364 - Moved Mirror 6.0 (pre-SDK) logic into its own function
long GetFirstValidImageIndex_PreCanfieldSDK(const CString &strMirrorId)
{
	long iIndex = 0;

	if (strMirrorId.IsEmpty())
		return -1;
	if (EnsureMirrorData())
		return -1;

	try {
		CString strSql, strFile;
		CFile f;
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly, "SELECT [IMAGE].IMAGE "
			"FROM [IMAGE] WHERE LINKNUM = '%s' ", strMirrorId);

		while (!prs->eof) {
			strFile = GetMirrorImagePath() ^ AdoFldString(prs, "IMAGE", "");
			if (f.Open(strFile, CFile::modeRead | CFile::shareCompat))
			{
				prs->Close();
				f.Close();
				return iIndex;
			}
			prs->MoveNext();
			iIndex++;
		}
		prs->Close();
	}
	catch (_com_error&)
	{
		Log("GetFirstValidImageIndex() failed");
	}
	return -1;
}

// (c.haag 2010-02-24 10:32) - PLID 37364 - Moved Mirror 6.0 (pre-SDK) logic into its own function
long GetLastValidImageIndex_PreCanfieldSDK(const CString &strMirrorId)
{
	long iIndex;

	try {
		if (EnsureMirrorData())
			return -1;

		CString strSql, strFile;
		CFile f;
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly, "SELECT [IMAGE].IMAGE "
			"FROM [IMAGE] WHERE LINKNUM = '%s' ", strMirrorId);

		if (prs->eof)
			return -1;

		prs->MoveLast();
		iIndex = prs->GetRecordCount() - 1;

		while (!prs->bof) {
			strFile = GetMirrorImagePath() ^ AdoFldString(prs, "IMAGE", "");
			if (f.Open(strFile, CFile::modeRead | CFile::shareCompat))
			{
				prs->Close();
				f.Close();
				return iIndex;
			}
			prs->MovePrevious();
			iIndex--;
		}
		prs->Close();
	}
	catch (_com_error&)
	{
		Log("GetLastValidImageIndex() failed");
	}
	return -1;
}

static inline void HR(HRESULT hr) {	if (FAILED(hr)) { _com_issue_error(hr); } }

//simple method for getting a patient name, usually to see if we are mislinked
bool GetMirrorName(const CString &mirrorID, CString &first, CString &last)
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
	if (!IsMirrorEnabled()) {
		return false;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			break;
		case eCSDK_Success:
			// Defer to SDK logic
			{
				CString strMI;
				if (!CanfieldLink::GetPatientName(mirrorID, first, strMI, last))
					return true;
				return false;
			}
		default:
			// Client is trying to run the Canfield SDK and failing
			return false;
	}

	if (EnsureMirrorData())
		return false;

	try {
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,	"SELECT LASTNAME, FIRSTNAME "
			"FROM M2000 WHERE RECNUM = '%s' ", mirrorID);

		if (prs->eof)
		{	HR(prs->Close());
			return false;
		} 
		else 
		{	first = AdoFldString(prs, "FIRSTNAME");
			last = AdoFldString(prs, "LASTNAME");
			prs->Close();
			return true;
		}
	}
	catch (_com_error&)
	{
		Log("GetMirrorName() failed");
	}
	return false;
} 

CString GetPatientMirrorID(unsigned long dwPatientID)
{
	try {
		_RecordsetPtr prs = CreateRecordset("SELECT MirrorID FROM PatientsT WHERE PersonID = %d", dwPatientID);
		CString strID;

		if (prs->eof)
			strID = "";
		else
			strID = AdoFldString(prs, "MirrorID");
		
		prs->Close();
		return strID;
	}
	NxCatchAll("Could not get mirror patient ID");
	return "";
}

// Open the Mirror program
// (a.walling 2008-07-07 17:40) - PLID 29900 - Pass in a patientID
// (c.haag 2009-03-31 13:02) - PLID 33630 - Making this void because nothing ever
// checks the return value (nor needs to)
void Run(long nPatientID)
{
	if (nPatientID == -1) {
		nPatientID = GetActivePatientID();
	}

	// (c.haag 2009-03-31 12:38) - PLID 33630 -  Use new logic to test Mirror connectivity
	if (!IsMirrorEnabled()) {
		return;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			break;
		case eCSDK_Success:
			// Defer to SDK logic
			CanfieldLink::OpenChart(Mirror::GetPatientMirrorID(GetActivePatientID()));
			return;
		default:
			// Client is trying to run the Canfield SDK and failing
			return;
	}

	CString	path;
	CString strParams;

	// Try to get the AppPath from the registry
	path = NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Mirror Image\\Mirror Professional\\AppPath");

	// It wasn't there; lets do it the old way
	if (path.IsEmpty())
	{
		path = GetRemotePropertyText ("MirrorImagePath", "", 0, "<None>");
		path = path.Left (path.ReverseFind('\\'));
	}
	path += "\\mpro\\mpro.exe ";
	strParams.Format("/recnum=%s /opendialog", Mirror::GetPatientMirrorID(GetActivePatientID()));
	path += strParams;

	WinExec(path, SW_MAXIMIZE);
}

void Troubleshoot()
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 -  Use new logic to test Mirror connectivity
	if (!IsMirrorEnabled()) {
		return;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			break;
		default:
			// Client is trying to run the Canfield SDK. Not supported here
			return;
	}

	// See if we have JET 4.0 installed
	if (!IsJet40())
	{
		MsgBox("Your system does not have the Microsoft Jet 4.0 Database drivers installed, and is therefore unable to access the Mirror data. Please consult your hardware administrator.");
		return;
	}

	// See if we even have a mirror path
	/*if (!IsMirrorEnabled())
	{
		if (IDYES == MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
			"The Mirror Link has not been properly set.\r\n"
			"Do you want to browse for your mirror data?", 
			"Mirror data could not be opened", MB_YESNO))
		{
			if (BrowseMirrorPath())
			{
				EnsureMirrorData();
				return;
			}
		}

		if (!GetMirrorDataPath().IsEmpty())
		{
			if (IDYES == MessageBox(AfxGetMainWnd()->GetSafeHwnd(), "Do you want to disable integration with Canfield Mirror?", "", MB_YESNO))
				SetMirrorDataPath("");
			else
				MsgBox("Could not link to mirror data");
		}
		return;
	}*/

	// See if we have permission to get to the data to begin with
	if (GetFileAttributes(GetMirrorDataPath()) == -1)
	{
		switch (GetLastError())
		{
		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
			MsgBox("The mirror data cannot be found. Please ensure the Mirror server is turned on.\n\nIf the Mirror server is on, ensure the the mirror link is looking in the correct place for data.");
			break;
		case ERROR_ACCESS_DENIED:
			MsgBox("You do not have the necessary permissions in Microsoft Windows to access the Mirror data.\n\nPlease contact your system administrator for support.");
			break;
		default:
			CString strError;	
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();
			MsgBox(CString("Error loading Mirror data: ") + strError);
			break;
		}
	}
	else if (GetFileAttributes(g_strLastImageFileName) == -1)
	{
		switch (GetLastError())
		{
		case ERROR_PATH_NOT_FOUND:
			MsgBox("The Mirror image %s cannot be found. Please ensure the Mirror server is turned on.\n\nIf the Mirror server is on, ensure the the mirror link is looking in the correct place for pictures.",
				g_strLastImageFileName);
			break;
		case ERROR_ACCESS_DENIED:
			MsgBox("You do not have the necessary permissions in Microsoft Windows to access the thumbnail %s.\n\nPlease contact your system administrator for support.",
				g_strLastImageFileName);
			break;
		default:
			CString strError;	
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();
			MsgBox(CString("Error loading thumbnail ") + g_strLastImageFileName + ": " + strError);
			break;
		}
	}
	else
	{
		MsgBox("An unknown error has occured. If you are in the General 1 tab, please try refreshing the patient's information at a later time.");
	}
}

// Code Sample 1.0
// check the validity of a RECNUM
#define RECNUM_LEN   17
BOOL IsValidRECNUM(LPCTSTR lpcszRECNUM)
{
   BOOL bValid = FALSE;
   CString strRECNUM(lpcszRECNUM);
   strRECNUM.TrimLeft();
   strRECNUM.TrimRight();
   int nLen = strRECNUM.GetLength();
   if (RECNUM_LEN == nLen)
   {
      // everything in string must be a numeric digit
      bValid = TRUE;
      for (register int i=0; i < nLen; i++)
      {
         if (!_istdigit(strRECNUM[i]))
         {
            bValid = FALSE;
            break;
         }
      }

      // check for "CCYYMMDDHHMMSSmmm"
      if (bValid)
      {
         int nYear = _ttoi(strRECNUM.Left(4));
         int nMonth = _ttoi(strRECNUM.Mid(4,2));
         int nDay = _ttoi(strRECNUM.Mid(6,2));
         int nHour = _ttoi(strRECNUM.Mid(8,2));
         int nMinute = _ttoi(strRECNUM.Mid(10,2));
         int nSecond = _ttoi(strRECNUM.Mid(12,2));
         int nFraction = _ttoi(strRECNUM.Right(3));

         try
         {
            COleDateTime dtCheck(nYear,
                                 nMonth,
                                 nDay,
                                 nHour,
                                 nMinute,
                                 nSecond);
            bValid = (COleDateTime::valid == dtCheck.GetStatus()) ?
                       TRUE : FALSE;
         }
         catch (COleException *e)
         {
            bValid = FALSE;
            e->Delete();
         }			
      }
   }

   return bValid;		
}

// Code Sample 1.1
// generate a new RECNUM / LINKNUM
TCHAR * GenerateRECNUM(TCHAR * buffer)
{
   static SYSTEMTIME	lastStamp;
   static BOOL firstStamp = TRUE;
   SYSTEMTIME	st;
   //get system time
   GetLocalTime(&st);
   if (firstStamp)
   {
      firstStamp = FALSE;
   }
   else
   {
      //ensure this time stamp is different
      //than the last one
      while(!memcmp(&st,
                    &lastStamp,
                    sizeof(SYSTEMTIME)))
      {
         Sleep(1);
         GetLocalTime(&st);
      }
   }
	
   //remember last time stamp so we can
   //force next one to be different
   lastStamp = st;
   wsprintf(buffer,
             _T("%.4d%.2d%.2d%.2d%.2d%.2d%.3d"),
             st.wYear,
             st.wMonth,
             st.wDay,
             st.wHour,
             st.wMinute,
             st.wSecond,
             st.wMilliseconds);
   return	buffer;
}

BOOL HasInvalidRECNUMs()
{
	// (c.haag 2009-03-31 12:38) - PLID 33630 -  Use new logic to test Mirror connectivity
	if (!IsMirrorEnabled()) {
		return FALSE;
	} else switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Failure_Link_SDK_Disabled:
		case eCSDK_Failure_NotAvailable:
			// Defer to pre-SDK logic
			break;
		default:
			// Client is trying to run the Canfield SDK. Not necessary here
			return FALSE;
	}

	try {
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,	"SELECT RECNUM FROM M2000 WHERE LEN(RECNUM) < 17");
		if (!prs->eof)
			return TRUE;

	} NxCatchAll("Error checking for invalid Mirror IDs");
	return FALSE;
}

BOOL RepairImageTable(int& nRepairedRecords)
{
	try {
		CNxProgressDlg dlgProgress(NULL);

		// (IMAGE TABLE REPAIR STEP 01)
		_RecordsetPtr prsImage = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,	"SELECT RECNUM, [IMAGE].[IMAGE], LINKNUM FROM [IMAGE]");
		int nRecords;
		if (!prsImage->eof)
		{
			prsImage->MoveLast();
			nRecords = prsImage->GetRecordCount();
			dlgProgress.Create();
			dlgProgress.SetWindowText("Mirror Reindexing");
			dlgProgress.StartNextProgress(NULL, 0, nRecords, "");
		}
		prsImage->Close();
		prsImage.Detach();
		prsImage = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,	"SELECT RECNUM, [IMAGE].[IMAGE], LINKNUM FROM [IMAGE]");
		int iCurRec = 0;

		while (!prsImage->eof)
		{
			CString strFirst, strMI, strLast, strM2000ID, strImageID;
			CString strRecnum; // M2000 recnum
			CString str = AdoFldString(prsImage, "IMAGE");
			CString strProgress;

			strProgress.Format("Checking record %d/%d",
				++iCurRec, nRecords);
			dlgProgress.SetProgress(iCurRec, strProgress);

			// Break up the image name into individual components
			// (IMAGE TABLE REPAIR STEP 02)
			strLast = str.Left( str.Find(" ") );
			strLast.Remove(',');

			str = str.Right( str.GetLength() - str.Find(" ") - 1 );
			strFirst = str.Left( str.Find(" ") );

			str = str.Right( str.GetLength() - str.Find(" ") - 1 );
			strMI = str.Left( str.Find(" ") );

			str = str.Right( str.GetLength() - str.Find(" ") - 1 );
			strM2000ID = str.Left( str.Find(" ") );
			strM2000ID.Remove('(');
			strM2000ID.Remove(')');

			str = str.Right( str.GetLength() - str.Find(" ") - 1 );
			strImageID = str;
			strImageID.Replace(".t2k", "");

			// (IMAGE TABLE REPAIR STEP 03)
			if (!AdoFldString(prsImage, "LINKNUM", "").IsEmpty())
			{
				// (IMAGE TABLE REPAIR STEP 12)
				_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
					adLockReadOnly,	"SELECT RECNUM FROM M2000 WHERE RECNUM = '%s'",
					AdoFldString(prsImage, "LINKNUM"));

				// (IMAGE TABLE REPAIR STEP 13)
				if (!prs->eof)
				{
					strRecnum = AdoFldString(prs, "RECNUM");
				}
			}

			if (strRecnum.IsEmpty())
			{
				// (IMAGE TABLE REPAIR STEP 04)
				_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
					adLockReadOnly,	"SELECT RECNUM FROM M2000 WHERE LASTNAME = '%s' AND FIRSTNAME = '%s' AND (MI %s)",
					_Q(strLast), _Q(strFirst), strMI.IsEmpty() ? "Is Null OR Len(MI) = 0" : CString("= '") + _Q(strMI) + CString("'"));

				// (IMAGE TABLE REPAIR STEP 05)
				BOOL bOneMatchFound = FALSE;
				if (!prs->eof)
				{
					prs->MoveLast();
					bOneMatchFound = prs->GetRecordCount() == 1;
				}
				if (bOneMatchFound)
				{
					prs->MoveFirst();
					strRecnum = AdoFldString(prs, "RECNUM");

					// (IMAGE TABLE REPAIR STEP 06)
					str.Format("UPDATE [IMAGE] SET LINKNUM = '%s' WHERE RECNUM = '%s'",
						strRecnum, AdoFldString(prsImage, "RECNUM"));
					g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);
					nRepairedRecords++;
				}
			}
			
			if (strRecnum.IsEmpty())
			{
				// (IMAGE TABLE REPAIR STEP 11)
				MsgBox("The patient %s %s could not be properly re-indexed in the image table. This patient will be skipped and the re-indexing will continue. Please contact NexTech for support when the re-indexing is completed.",
					strFirst, strLast);
			}
			else
			{
				CString strNewImageName;
				strNewImageName.Format("%s, %s %s (%s) %s.t2k",
					_Q(strLast), _Q(strFirst), _Q(strMI), strRecnum, AdoFldString(prsImage, "RECNUM"));

				// (IMAGE TABLE REPAIR STEP 14)
				str.Format("UPDATE [IMAGE] SET [IMAGE].[IMAGE] = '%s' WHERE RECNUM = '%s'",
					strNewImageName, AdoFldString(prsImage, "RECNUM"));
				g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);

				// (IMAGE TABLE REPAIR STEP 15)
				strNewImageName.Format("%s, %s %s (%s) %s.t2k",
					(strLast), (strFirst), (strMI), strRecnum, AdoFldString(prsImage, "RECNUM"));
				MoveFile(GetMirrorImagePath() ^ AdoFldString(prsImage, "IMAGE"),
					GetMirrorImagePath() ^ strNewImageName);
			}

			// (IMAGE TABLE REPAIR STEP 07 "Image row complete")

			// (IMAGE TABLE REPAIR STEP 08)
			prsImage->MoveNext();
		}
		// (IMAGE TABLE REPAIR STEP 09 "End of file")

		// (IMAGE TABLE REPAIR STEP 10)
		return FALSE;
	} NxCatchAll("Error repairing image table");
	return TRUE; // Return failure
}

BOOL RepairM2000Table(int& nRepairedRecords)
{
	try {
		// M2000 TABLE REPAIR STEP 01
		_RecordsetPtr prs = CreateMirrorRecordset(adOpenForwardOnly,
			adLockReadOnly,	"SELECT RECNUM, LASTNAME, FIRSTNAME, MI FROM M2000 WHERE LEN(RECNUM) < 17");
		CString str;
		// M2000 TABLE REPAIR STEP 02
		while (!prs->eof)
		{
			CString strOldRecnum = AdoFldString(prs, "RECNUM");
			TCHAR szNewRecnum[RECNUM_LEN];
			GenerateRECNUM(szNewRecnum); // M2000 TABLE REPAIR STEP 03

			// Update the IMAGE table (M2000 TABLE REPAIR STEP 05)
			_RecordsetPtr prsImage = CreateMirrorRecordset(adOpenForwardOnly,
				adLockReadOnly,	"SELECT RECNUM, [IMAGE].[IMAGE] FROM [IMAGE] WHERE LINKNUM = '%s'",
				strOldRecnum);
			while (!prsImage->eof) // M2000 TABLE REPAIR STEP 06
			{
				CString strImageRecnum = AdoFldString(prsImage, "RECNUM");
				CString strNewImageName;
				strNewImageName.Format("%s, %s %s (%s) %s.t2k",
					AdoFldString(prs, "LASTNAME", ""),
					AdoFldString(prs, "FIRSTNAME", ""),
					AdoFldString(prs, "MI", ""),
					szNewRecnum,
					strImageRecnum);

				// Update the name of the picture itself  (M2000 TABLE REPAIR STEP 09)
				if (!MoveFile(GetMirrorImagePath() ^ AdoFldString(prsImage, "IMAGE"),
					GetMirrorImagePath() ^ strNewImageName))
				{
					MsgBox("Failed to rename file %s to %s. Please ensure the file exists and that you have write access to the picture folder. The reindexing will now stop.",
						GetMirrorImagePath() ^ AdoFldString(prsImage, "IMAGE"),
						GetMirrorImagePath() ^ strNewImageName);
					return TRUE; // Return failure
				}
				else
				{
					strNewImageName.Format("%s, %s %s (%s) %s.t2k",
						_Q(AdoFldString(prs, "LASTNAME", "")),
						_Q(AdoFldString(prs, "FIRSTNAME", "")),
						_Q(AdoFldString(prs, "MI", "")),
						szNewRecnum,
						strImageRecnum);

					// Update the LINKNUM value (M2000 TABLE REPAIR STEP 07)
					str.Format("UPDATE [IMAGE] SET LINKNUM = '%s' WHERE RECNUM = '%s'",
						szNewRecnum, strImageRecnum);
					g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);

					// Update the name in the data  (M2000 TABLE REPAIR STEP 08)
					str.Format("UPDATE [IMAGE] SET [IMAGE].[IMAGE] = '%s' WHERE RECNUM = '%s'",
						strNewImageName, strImageRecnum);
					g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);
				}
				nRepairedRecords++;
				prsImage->MoveNext(); // M2000 TABLE REPAIR STEP 10
			}

			// Update M2000 (M2000 TABLE REPAIR STEP 04)
			str.Format("UPDATE M2000 SET RECNUM = '%s' WHERE RECNUM = '%s'",
				szNewRecnum, strOldRecnum);
			g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);

			// Update the Practice database
			ExecuteSql("UPDATE PatientsT SET MirrorID = '%s' WHERE MirrorID = '%s'",
				szNewRecnum, strOldRecnum);

			nRepairedRecords++;
			prs->MoveNext(); // M2000 TABLE REPAIR STEP 11			
		}
		return FALSE; // Return success
	} NxCatchAll("Error repairing M2000 table");
	return TRUE; // Return failure
}

BOOL Get61Version(long& nMajor, long& nMinor)
{
	if (!IsMirrorEnabled())
		return FALSE;
	CanfieldLink::GetCanfieldSDKVersion(nMajor, nMinor);
	return TRUE;
}

BOOL HasMirrorLinkLicense()
{
	// (c.haag 2006-08-02 13:29) - PLID 21670 - What we do here is check the license object, and then
	// cache that value for every successive call to HasMirrorLinkLicense().
	if (!g_bIsLicensedKnown) {
		BOOL bHasAvailableSeat = g_pLicense->CheckForLicense(CLicense::lcMirror, CLicense::cflrSilent);
		if (FALSE == (g_bHasMirrorLinkLicense = g_pLicense->CheckForLicense(CLicense::lcMirror, CLicense::cflrUse))) {
			if (bHasAvailableSeat) {
				// (c.haag 2006-08-02 15:17) - If we get here, it means that they refused to use a license
				// seat for the Mirror link.
				MsgBox("The link to Mirror will be disabled for the remainder of this session of NexTech Practice.");
			}
		}
		g_bIsLicensedKnown = TRUE;
	}
	return g_bHasMirrorLinkLicense;
}

// (c.haag 2009-03-31 13:36) - PLID 33630 - Returns TRUE if Practice is trying (not necessarily successfully)
// to use the Canfield SDK functionality. The link's enabled/disabled and some error states are irrelevant here.
BOOL Mirror::IsUsingCanfieldSDK()
{
	switch (InitCanfieldSDK(TRUE)) {
		case eCSDK_Success:
			return TRUE; // We got connected
		case eCSDK_Connecting_WillPostMessage:
			ASSERT(FALSE); // This should never happen in synchronous mode, but it is technically a success condition
			return TRUE;
		case eCSDK_Failure_Link_Disabled:
			// We did not attempt a connection because the link is disabled. However, the default behavior is to
			// try to use the Canfield SDK unless it's impossible due to the absence of Mirror, or the special ConfigRT
			// setting that disables the SDK. So, this counts as an affirmative
			return TRUE;		
		case eCSDK_Failure_Link_SDK_Disabled:
			// There is a ConfigRT property that disabled the express use of the Canfield SDK
			return FALSE;
		case eCSDK_Failure_NotAvailable:
			// The Canfield SDK was not detected on this machine
			return FALSE;
		case eCSDK_Failure_TimeoutExpired:
			// A timeout occured attempting to connect to the Canfield SDK, but we were at least able to instantiate
			// the object. So, that counts as an affirmative.
			return TRUE; 
		case eCSDK_Failure_Error:
			// An unexpected error occured attempting to use the Canfield SDK, but we were at least able to instantiate
			// the object. So, that counts as an affirmative.
			return TRUE;
		default:
			ASSERT(FALSE); // This should never happen, but err on the assumption that we should be using the SDK
			return TRUE;
	}
}

}// end Mirror namespace
