// ReservationReadSet.cpp: implementation of the CReservationReadSet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReservationReadSet.h"
#include "SchedulerView.h"
#include "NxStandard.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalDataUtils.h"
#include "GlobalSchedUtils.h"
#include "CommonSchedUtils.h"
using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// (a.walling 2013-06-18 11:27) - PLID 57199 - ReFilter was never called on an existing ReadSet so I removed that logic. Also removed the ReFilterResByResIDP since we can do that simply with a query for now.

CReservationReadSet::CReservationReadSet()
{
	// (c.haag 2008-03-20 10:53) - PLID 29328 - Increased from 48 to 50
	//DRT 9/10/2008 - PLID 31127 - Increased to 52 with check in/out times
	//TES 11/12/2008 - PLID 11465, 12057 - Increased to 56 with input date, referring physician info
	// (c.haag 2008-11-20 16:14) - PLID 31128 - Incremented to 57 for insurance referral summary
	// (z.manning 2011-07-01 16:53) - PLID 23273 - 58 for ArrivalTime
	// (j.armen 2011-07-05 13:35) - PLID 44205 - 59 for ConfirmedBy
	// (j.luckoski 2012-05-09 13:52) - PLID 11597 - 60 for CancelledDate
	// (j.gruber 2012-08-06 13:19) - PLID 51926 - 63 after insurance information
	// (j.gruber 2013-01-08 15:23) - PLID 54497 - 64 after appt ref phys
	// (s.tullis 2014-01-24 08:31) - PLID 49748 - 65 Add nexweb activation code to the  extra info fields
	// (s.tullis 2014-01-24 16:00) - PLID 60470 - 66 Add nexweb security code experation date to the  extra info fields
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added GetIsMixRuleOverridden
	// (j.jones 2014-12-04 13:20) - PLID 64119 - added MixRuleColor
	m_nBoolCount = 68;

	// (j.jones 2014-12-03 10:58) - PLID 64274 - added GetIsMixRuleOverridden
	m_bIsMixRuleOverridden = false;

	m_dtInvalid.SetStatus(COleDateTime::invalid);
}

CReservationReadSet::~CReservationReadSet()
{
	//TES 1/14/2010 - PLID 36762 - Clean up memory allocation for security groups
	DestroySecurityGroupMap();
}

// (a.walling 2013-06-18 11:25) - PLID 57199 - Simply refilters on a SQL fragment now
void CReservationReadSet::Open(CSqlFragment query)
{
	// Open the recordset
	// (a.walling 2013-06-18 11:12) - PLID 57196 - Use snapshot isolation for the main scheduler CReservationReadSet queries
	m_prs = CreateParamRecordset(GetRemoteDataSnapshot(), query);
	m_flds = m_prs->Fields;
	
	// Get all the common fields
	m_fldID = m_flds->Item["ID"];
	m_fldPersonID = m_flds->Item["PatientID"];
	m_fldResourceID = m_flds->Item["ResourceID"];
	m_fldUserDefinedID = m_flds->Item["UserDefinedID"];
	m_fldDate = m_flds->Item["Date"];
	m_fldStartTime = m_flds->Item["StartTime"];
	m_fldEndTime = m_flds->Item["EndTime"];
	m_fldConfirmed = m_flds->Item["Confirmed"];
	m_fldReady = m_flds->Item["Ready"];
	m_fldNotes = m_flds->Item["Notes"];
	m_fldMoveUp = m_flds->Item["MoveUp"];
	m_fldNoShow = m_flds->Item["ShowState"];
	m_fldNoShowSymbol = m_flds->Item["ShowStateSymbol"];
	m_fldPatientName = m_flds->Item["PatientName"];
	m_fldBirthDate = m_flds->Item["BirthDate"];
	m_fldHomePhone = m_flds->Item["HomePhone"];
	m_fldWorkPhone = m_flds->Item["WorkPhone"];
	m_fldWorkExt = m_flds->Item["WorkExt"];
	m_fldMobilePhone = m_flds->Item["CellPhone"];
	m_fldPreferredContact = m_flds->Item["PreferredContact"];
	m_fldColor = m_flds->Item["Color"];
	m_fldAptType = m_flds->Item["AptType"];
	m_fldAptPurpose = m_flds->Item["AptPurpose"];
	m_fldLocationName = m_flds->Item["LocationName"];
	m_fldPatBal = m_flds->Item["PatBal"];
	m_fldInsBal = m_flds->Item["InsBal"];
	m_fldRefSource = m_flds->Item["RefSourceName"];
	m_fldEmail = m_flds->Item["Email"];
	m_fldPager = m_flds->Item["Pager"];
	m_fldOtherPhone = m_flds->Item["OtherPhone"];
	m_fldCustom1 = m_flds->Item["Custom1"];
	m_fldCustom2 = m_flds->Item["Custom2"];
	m_fldCustom3 = m_flds->Item["Custom3"];
	m_fldCustom4 = m_flds->Item["Custom4"];
	m_fldInputName = m_flds->Item["CreatedLogin"];
	m_fldLMDate = m_flds->Item["LastLM"];
	m_fldModifiedLogin = m_flds->Item["ModifiedLogin"];
	m_fldModifiedDate = m_flds->Item["ModifiedDate"];
	m_fldStatusColor = m_flds->Item["ShowStateColor"];
	// (j.jones 2014-12-04 13:17) - PLID 64119 - added MixRuleColor
	m_fldMixRuleColor = m_flds->Item["MixRuleColor"];
	m_fldResourceString = m_flds->Item["ResourceString"];
	// (c.haag 2006-12-12 17:24) - PLID 23845 - More fields for
	// template block appearance calculations
	m_fldAptTypeID = m_flds->Item["AptTypeID"];
	m_fldAptResourceIDString = m_flds->Item["ResourceIDString"];
	m_fldAptPurposeIDString = m_flds->Item["PurposeIDString"];

	// (d.moore 2007-06-05 16:31) - PLID 13550 - Adding insurance fields.
	m_fldPrimaryInsurance = m_flds->Item["PrimaryIns"];
	m_fldSecondaryInsurance = m_flds->Item["SecondaryIns"];
	m_fldPrimaryCoPay = m_flds->Item["PriInsCoPay"];
	m_fldPrimaryCoPayPercent = m_flds->Item["PriInsCoPayPer"];
	// (j.gruber 2010-08-03 14:30) - PLID 39948 - no more type
	//m_fldPrimaryCoPayType = m_flds->Item["PriInsCoPayType"];
	m_fldSecondaryCoPay = m_flds->Item["SecInsCoPay"];
	m_fldSecondaryCoPayPercent = m_flds->Item["SecInsCoPayPer"];
	// (j.gruber 2010-08-03 14:30) - PLID 39948 - no more type
	//m_fldSecondaryCoPayType = m_flds->Item["SecInsCoPayType"];

	// (c.haag 2008-03-20 10:18) - PLID 29328 - Adding inventory fields
	m_fldHasOpenOrder = m_flds->Item["HasOpenOrder"];
	m_fldHasOpenAllocation = m_flds->Item["HasOpenAllocation"];

	//DRT 9/10/2008 - PLID 31127
	m_fldCheckInTime = m_flds->Item["CheckInTime"];
	m_fldCheckOutTime = m_flds->Item["CheckOutTime"];

	//TES 11/12/2008 - PLID 11465 - Added InputDate
	m_fldInputDate = m_flds->Item["CreatedDate"];

	//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	m_fldRefPhysName = m_flds->Item["RefPhysName"];
	m_fldRefPhysWorkPhone = m_flds->Item["RefPhysWorkPhone"];
	m_fldRefPhysFax = m_flds->Item["RefPhysFax"];

	// (c.haag 2008-11-20 16:14) - PLID 31128 - Insurance referral summary
	m_fldInsuranceReferralSummary = m_flds->Item["InsuranceReferralSummary"];
	m_fldConfirmedBy = m_flds->Item["ConfirmedBy"]; // (j.armen 2011-07-05 13:22) - PLID 44205
	m_fldArrivalTime = m_flds->Item["ArrivalTime"]; // (z.manning 2011-07-01 16:21) - PLID 23273
	m_fldCancelledDate = m_flds->Item["CancelledDate"]; // (j.luckoski 2012-05-09 13:52) - PLID 11597 - Add cancelledDate
	m_fldApptPriInsCo = m_flds->Item["ApptPriIns"]; // (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
	m_fldApptSecInsCo = m_flds->Item["ApptSecIns"]; // (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
	m_fldApptInsCoList = m_flds->Item["ApptInsList"] ;// (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
	m_fldApptRefPhys = m_flds->Item["ApptRefPhysName"] ;// (j.gruber 2013-01-08 15:24) - PLID 54497
	m_fldNxWebCode = m_flds->Item["SecurityCode"] ; // (s.tullis 2014-01-23 17:13) - PLID 49748 - Add nexweb activation code to the  extra info fields
    m_fldNxWebCodeDate = m_flds->Item["SecurityCodeCreationDate"] ;// (s.tullis 2014-01-24 16:02) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
	m_fldNxWebCodeExpDate = m_flds->Item["SecurityCodeExpDate"] ;// (s.tullis 2014-01-24 16:02) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
	// (j.jones 2014-12-03 11:14) - PLID 64274 - added m_fldIsMixRuleOverridden
	m_fldIsMixRuleOverridden = m_flds->Item["IsMixRuleOverridden"];
	//TES 1/14/2010 - PLID 36762 - There will now be a second recordset for any query passed in, which will have all PatientID/SecurityGroupID
	// pairs which are valid for this set of appointments.  Go through and remember everything, we will then use it to block demographic
	// info as needed.
	//TES 1/15/2010 - PLID 36762 - Get the list of patients which the user has been granted emergency access to.
	CArray<long,long> arEmergencyPatientIDs;
	GetMainFrame()->GetEmergencyAccessPatients(arEmergencyPatientIDs);
	bool bHasEmergencyAccess = false;
	_RecordsetPtr rsGroups = m_prs->NextRecordset(NULL);
	long nCurPatientID = -1, nPatientID = -1, nSecurityGroupID = -1;
	CArray<long,long> *parCurrentGroups = NULL;
	DestroySecurityGroupMap();
	while(!rsGroups->eof) {
		nPatientID = AdoFldLong(rsGroups, "PatientID");
		if(nPatientID != nCurPatientID) {
			m_mapPatientsToSecurityGroups.SetAt(nCurPatientID, parCurrentGroups);
			parCurrentGroups = new CArray<long,long>;
			nCurPatientID = nPatientID;
			//TES 1/15/2010 - PLID 36762 - Now check if we have emergency access.
			bHasEmergencyAccess = false;
			for(int i = 0; i < arEmergencyPatientIDs.GetSize() && !bHasEmergencyAccess; i++) {
				if(arEmergencyPatientIDs[i] == nCurPatientID) bHasEmergencyAccess = true;
			}
		}
		if(!bHasEmergencyAccess) {
			parCurrentGroups->Add(AdoFldLong(rsGroups, "SecurityGroupID"));
		}
		rsGroups->MoveNext();
	}
	if(nCurPatientID != -1) {
		m_mapPatientsToSecurityGroups.SetAt(nCurPatientID, parCurrentGroups);
	}

	// Next time we try to get a value, we will re-get it from the data
	InitNeedRefresh();
}

void CReservationReadSet::InitNeedRefresh()
{
	for (long i=0; i<m_nBoolCount; i++) {
		m_bNeedRefresh[i] = true;
	}
}

// (j.jones 2011-02-11 10:20) - PLID 35180 - changed nItemID into a string of resource IDs
// (a.walling 2013-06-18 11:25) - PLID 57199 - Simply refilters on a SQL fragment now
bool CReservationReadSet::ReFilter(CSqlFragment query)
{
	Open(query);
	return true;
}

bool CReservationReadSet::NeedRefresh(int nBoolNum)
{
	bool bAns = m_bNeedRefresh[nBoolNum];
	if (bAns) {
		m_bNeedRefresh[nBoolNum] = false;
	}
	return bAns;
}

void CReservationReadSet::EnsureCorrectTimes()
{
	m_dtStart = GetStartTimeDt();
	m_dtEnd = GetEndTimeDt();

	// Ensure valid start and end times
	if (m_dtStart.GetStatus()) { // Non-zero status means invalid date/time
		m_dtStart.SetTime(8, 0, 0);
	}
	if (m_dtEnd.GetStatus()) { // Non-zero status means invalid date/time
		m_dtEnd.SetTime(9, 0, 0);
	}
	if (m_dtEnd < m_dtStart) {
		m_dtEnd = m_dtStart;
	}
}

const COleDateTime &CReservationReadSet::GetStartTimeDt()
{
	if (NeedRefresh(0)) m_StartTime = AdoFldDateTime(m_fldStartTime);
	return m_StartTime;
}

const COleDateTime &CReservationReadSet::GetEndTimeDt()
{
	if (NeedRefresh(1)) m_EndTime = AdoFldDateTime(m_fldEndTime);
	return m_EndTime;
}

const COleDateTime &CReservationReadSet::GetStartTime()
{
	if (NeedRefresh(2)) EnsureCorrectTimes();
	return m_dtStart;
}

const COleDateTime &CReservationReadSet::GetEndTime()
{
	if (NeedRefresh(2)) EnsureCorrectTimes();
	return m_dtEnd;
}

const COleDateTime &CReservationReadSet::GetDate()
{
	if (NeedRefresh(3)) m_Date = AdoFldDateTime(m_fldDate);
	return m_Date;
}

const long CReservationReadSet::GetColor()
{
	// (a.walling 2013-06-18 11:27) - PLID 57199 - If cancelled, use the cancelled color.
	if (NeedRefresh(4)) {
		if (GetCancelledDate().GetStatus() == COleDateTime::valid) {
			// cancelled
			m_Color = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
		} else {
			m_Color = AdoFldLong(m_fldColor, 0);
		}
	}
	return m_Color;
}

// (j.luckoski 2012-06-20 11:31) - PLID 11597 - Added bool bIsCancelled to handle when to call MakeBoxText with a cancelled or uncancelled apt
// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
const LPCTSTR CReservationReadSet::GetBoxText(CString strMultiPurpose, long nProp_ColorApptTextWithStatus, bool bIsCancelled, bool bIsMixRuleOverridden)
{
	if (NeedRefresh(5))
	{
		if (!strMultiPurpose.IsEmpty())
		{
			NeedRefresh(7);
			m_strAptPurpose = strMultiPurpose;
		}
		// (j.luckoski 2012-06-20 11:31) - PLID 11597 - If cancelled call true and if uncancelled call false MakeBoxText
		// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
		MakeBoxText(nProp_ColorApptTextWithStatus, bIsCancelled, bIsMixRuleOverridden);
	}
	return m_BoxText;
}

const CString &CReservationReadSet::GetPatientName()
{
	if (NeedRefresh(6)) m_PatientName = AdoFldString(m_fldPatientName, "");
	return m_PatientName;
}

const CString &CReservationReadSet::GetNotes()
{
	//TES 1/14/2010 - PLID 36762 - If the patient is blocked, we want to hide the notes, and additionally include in the text why nothing
	// else is showing up.
	if(IsPatientBlocked()) {
		m_Notes = "<Patient Blocked>";
	}
	else {
		if (NeedRefresh(8)) m_Notes = AdoFldString(m_fldNotes);
	}
	return m_Notes;
}

const BOOL CReservationReadSet::GetMoveUp()
{
	if (NeedRefresh(9)) m_MoveUp = AdoFldBool(m_fldMoveUp);
	return m_MoveUp;
}
BYTE AdoFldByte(const FieldPtr &pField);
const long CReservationReadSet::GetNoShow()
{
	if (NeedRefresh(10))
	{
		m_NoShow = AdoFldLong(m_fldNoShow);
		m_NoShowSymbol = AdoFldString(m_fldNoShowSymbol);
	}
	return m_NoShow;
}

const CString& CReservationReadSet::GetNoShowSymbol()
{
	if (NeedRefresh(10))
	{
		m_NoShow = AdoFldLong(m_fldNoShow);
		m_NoShowSymbol = AdoFldString(m_fldNoShowSymbol);
	}
	return m_NoShowSymbol;
}

const long CReservationReadSet::GetConfirmed()
{
	if (NeedRefresh(11)) m_Confirmed = AdoFldLong(m_fldConfirmed);
	return m_Confirmed;
}

const CString &CReservationReadSet::GetAptType()
{
	if (NeedRefresh(12)) m_strAptType = AdoFldString(m_fldAptType, "");
	return m_strAptType;
}

const CString &CReservationReadSet::GetAptPurpose()
{
	if (NeedRefresh(7)) m_strAptPurpose = AdoFldString(m_fldAptPurpose, "");
	return m_strAptPurpose;
}

const long CReservationReadSet::GetResourceID()
{
	if (NeedRefresh(21)) m_ResourceID = AdoFldLong(m_fldResourceID);
	return m_ResourceID;
}

const long CReservationReadSet::GetPersonID()
{
	if (NeedRefresh(13)) m_PersonID = AdoFldLong(m_fldPersonID);
	return m_PersonID;
}

const long CReservationReadSet::GetUserDefinedID()
{
	if (NeedRefresh(14)) m_UserDefinedID = AdoFldLong(m_fldUserDefinedID, GetPersonID());
	return m_UserDefinedID;
}

const COleDateTime &CReservationReadSet::GetBirthDate()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_BirthDate = m_dtInvalid;
	}
	else {
		if (NeedRefresh(15)) m_BirthDate = AdoFldDateTime(m_fldBirthDate, m_dtInvalid);
	}
	return m_BirthDate;
}

const CString &CReservationReadSet::GetHomePhone()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if (NeedRefresh(16)) {
		if(IsPatientBlocked()) {
			m_HomePhone = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			m_HomePhone = "<Hidden>";
		}
		else {
			m_HomePhone = AdoFldString(m_fldHomePhone, "");
		}	
	}
	return m_HomePhone;
}

const CString &CReservationReadSet::GetWorkPhone()
{
	if (NeedRefresh(17))
	{
		//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
		if(IsPatientBlocked()) {
			m_WorkPhone = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			m_WorkPhone = "<Hidden>";
		}
		else {
			m_WorkPhone = AdoFldString(m_fldWorkPhone, "");
			m_WorkExt = AdoFldString(m_fldWorkExt, "");
		}
	}
	return m_WorkPhone;
}

const CString &CReservationReadSet::GetWorkExt()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_WorkPhone = "";
		m_WorkExt = "";
	}
	else if (NeedRefresh(17))
	{
		m_WorkPhone = AdoFldString(m_fldWorkPhone, "");
		m_WorkExt = AdoFldString(m_fldWorkExt, "");
	}
	return m_WorkExt;
}

const long CReservationReadSet::GetID()
{
	if (NeedRefresh(18)) m_ID = AdoFldLong(m_fldID);
	return m_ID;
}

const BOOL CReservationReadSet::GetReady()
{
	if (NeedRefresh(22)) m_Ready = AdoFldBool(m_fldReady);
	return m_Ready;
}

const CString &CReservationReadSet::GetMobilePhone()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if (NeedRefresh(20)) {
		if(IsPatientBlocked()) {
			m_MobilePhone = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			m_MobilePhone = "<Hidden>";
		}
		else {
			m_MobilePhone = AdoFldString(m_fldMobilePhone, "");
		}
	}
	return m_MobilePhone;
}

const long &CReservationReadSet::GetPreferredContact()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_PreferredContact = 0;
	}
	else if (NeedRefresh(23)) {
		m_PreferredContact = AdoFldLong(m_fldPreferredContact, 0);
		
		if (m_PreferredContact == 6 /*email*/) {
			// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
			if (!(GetCurrentUserPermissions(bioPatientEmail) & (SPT__R________))) {
				m_PreferredContact = 0;
			}
		}
		else if ( m_PreferredContact == 1 || m_PreferredContact == 2 ||
			      m_PreferredContact == 3 || m_PreferredContact == 4 ||
				  m_PreferredContact == 5) {
			// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
			if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
				m_PreferredContact = 0;
			}
		}			

	}
	return m_PreferredContact;
}

const CString &CReservationReadSet::GetLocationName()
{
	if (NeedRefresh(24)) m_LocationName = AdoFldString(m_fldLocationName);
	return m_LocationName;
}

const CString &CReservationReadSet::GetPatBal()
{
	COleCurrency cyZero(0,0);
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strPatBal = "";
	}
	else {
		if (NeedRefresh(25)) m_strPatBal = FormatCurrencyForInterface(AdoFldCurrency(m_fldPatBal, cyZero));
	}
	return m_strPatBal;
}

const CString &CReservationReadSet::GetInsBal()
{
	COleCurrency cyZero(0,0);
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strInsBal = "";
	}
	else {
		if (NeedRefresh(26)) m_strInsBal = FormatCurrencyForInterface(AdoFldCurrency(m_fldInsBal, cyZero));
	}
	return m_strInsBal;
}

const CString &CReservationReadSet::GetRefSource()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strRefSource = "";
	}
	else {
		if(NeedRefresh(27)) m_strRefSource = AdoFldString(m_fldRefSource, "");
	}
	return m_strRefSource;
}

const CString &CReservationReadSet::GetResourceString()
{
	if (NeedRefresh(28)) m_strResourceString = AdoFldString(m_fldResourceString);
	return m_strResourceString;
}

const CString &CReservationReadSet::GetEmail()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if (NeedRefresh(29)) {
		if(IsPatientBlocked()) {
			m_strEmail = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientEmail) & (SPT__R________))) {
			m_strEmail = "<Hidden>";
		}
		else {
			m_strEmail = AdoFldString(m_fldEmail);
		}
	}
	return m_strEmail;
}

const CString &CReservationReadSet::GetPager()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if (NeedRefresh(30)) {
		if(IsPatientBlocked()) {
			m_strPager = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			m_strPager = "<Hidden>";
		}
		else {
			m_strPager = AdoFldString(m_fldPager);
		}
	}
	return m_strPager;
}

const CString &CReservationReadSet::GetOtherPhone()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if (NeedRefresh(31))  {
		if(IsPatientBlocked()) {
			m_strOtherPhone = "";
		}
		// (j.gruber 2007-08-09 09:01) - PLID 25045 - added the hidden clause if they don't have permissions
		else if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			m_strOtherPhone = "<Hidden>";
		}
		else {
			m_strOtherPhone = AdoFldString(m_fldOtherPhone);
		}
	}
	return m_strOtherPhone;
}

const CString &CReservationReadSet::GetCustom1()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strCustom1 = "";
	}
	else {
		if (NeedRefresh(32)) m_strCustom1 = AdoFldString(m_fldCustom1,"");
	}
	return m_strCustom1;
}
const CString &CReservationReadSet::GetCustom2()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strCustom2 = "";
	}
	else {
		if (NeedRefresh(33)) m_strCustom2 = AdoFldString(m_fldCustom2,"");
	}
	return m_strCustom2;
}

const CString &CReservationReadSet::GetCustom3()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strCustom3 = "";
	}
	else {
		if (NeedRefresh(34)) m_strCustom3 = AdoFldString(m_fldCustom3,"");
	}
	return m_strCustom3;
}

const CString &CReservationReadSet::GetCustom4()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strCustom4 = "";
	}
	else {
		if (NeedRefresh(35)) m_strCustom4 = AdoFldString(m_fldCustom4,"");
	}
	return m_strCustom4;
}

const CString &CReservationReadSet::GetInputName()
{
	if (NeedRefresh(36)) m_strInputName = AdoFldString(m_fldInputName,"");
	return m_strInputName;
}

const COleDateTime &CReservationReadSet::GetLMDate()
{
	if (NeedRefresh(37)) m_dtLM = AdoFldDateTime(m_fldLMDate, m_dtInvalid);
	return m_dtLM;
}

const CString &CReservationReadSet::GetModifiedLogin()
{
	if (NeedRefresh(38)) m_strModifiedLogin = AdoFldString(m_fldModifiedLogin,"");
	return m_strModifiedLogin;
}

const COleDateTime &CReservationReadSet::GetModifiedDate()
{
	if (NeedRefresh(39)) m_dtModified = AdoFldDateTime(m_fldModifiedDate, m_dtInvalid);
	return m_dtModified;
}

const CString& CReservationReadSet::GetPrimaryInsurance()
{
	// (d.moore 2007-06-05 15:45) - PLID 13550 - Adding Insurance fields
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strPrimaryInsurance = "";
	}
	else {
		if (NeedRefresh(44)) m_strPrimaryInsurance = AdoFldString(m_fldPrimaryInsurance, "");
	}
	return m_strPrimaryInsurance;
}

const CString& CReservationReadSet::GetSecondaryInsurance()
{
	// (d.moore 2007-06-05 15:45) - PLID 13550 - Adding Insurance fields
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strSecondaryInsurance = "";
	}
	else {
		if (NeedRefresh(45)) m_strSecondaryInsurance = AdoFldString(m_fldSecondaryInsurance, "");
	}
	return m_strSecondaryInsurance;
}

const CString& CReservationReadSet::GetPrimaryCoPay()
{
	// (d.moore 2007-06-05 15:45) - PLID 13550 - Adding Insurance fields
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strPrimaryCoPay = "";
	}
	else {
		if (NeedRefresh(46)) {
			
			// (j.gruber 2010-08-03 14:30) - PLID 39948 - change to new structure
			_variant_t varCopay, varPercent;
			varCopay = m_fldPrimaryCoPay->Value;
			varPercent = m_fldPrimaryCoPayPercent->Value;

			//there will only ever be one filled out, check money first
			if (varCopay.vt == VT_CY) {				
				m_strPrimaryCoPay = FormatCurrencyForInterface(VarCurrency(varCopay), TRUE, FALSE);
			}
			else if (varPercent.vt == VT_I4) {
					m_strPrimaryCoPay.Format("%li%%", VarLong(varPercent));				
			}
			else {
				m_strPrimaryCoPay = "";
			}
		}
	}
	return m_strPrimaryCoPay;
}

const CString& CReservationReadSet::GetSecondaryCoPay()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.

	if(IsPatientBlocked()) {
		m_strSecondaryCoPay = "";
	}
	else {
		// (d.moore 2007-06-05 15:45) - PLID 13550 - Adding Insurance fields
		if (NeedRefresh(47)) {
			// (j.gruber 2010-08-03 14:37) - PLID 39948 - changed to new structure
			_variant_t varCopay, varPercent;
			varCopay = m_fldSecondaryCoPay->Value;
			varPercent = m_fldSecondaryCoPayPercent->Value;

			//there will only ever be one filled out, check money first
			if (varCopay.vt == VT_CY) {				
				m_strSecondaryCoPay = FormatCurrencyForInterface(VarCurrency(varCopay), TRUE, FALSE);
			}
			else if (varPercent.vt == VT_I4) {
					m_strSecondaryCoPay.Format("%li%%", VarLong(varPercent));				
			}
			else {
				m_strSecondaryCoPay = "";
			}
		}
	}
	return m_strSecondaryCoPay;
}

const BOOL CReservationReadSet::GetHasOpenOrder()
{
	// (c.haag 2008-03-20 10:22) - PLID 29328 - Adding inventory fields
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_bHasOpenOrder = FALSE;
	}
	else {
		if (NeedRefresh(48)) m_bHasOpenOrder = AdoFldBool(m_fldHasOpenOrder); // Should never be null
	}
	return m_bHasOpenOrder;
}

const BOOL CReservationReadSet::GetHasOpenAllocation()
{
	// (c.haag 2008-03-20 10:22) - PLID 29328 - Adding inventory fields
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_bHasOpenAllocation = FALSE;
	}
	else {
		if (NeedRefresh(49)) m_bHasOpenAllocation = AdoFldBool(m_fldHasOpenAllocation); // Should never be null
	}
	return m_bHasOpenAllocation;
}

//DRT 9/10/2008 - PLID 31127 - Convert to properly formatted date
const CString& CReservationReadSet::GetCheckInTime()
{
	// (a.walling 2013-06-18 11:25) - PLID 57199 - to avoid ANSI_WARNINGS when using MAX() with a NULL value, we use a sentinel sql datetime value of 0 for NULL, which ends up being DATE value 2
	if(NeedRefresh(50)) {
		if(m_fldCheckInTime->Value.vt == VT_DATE && m_fldCheckInTime->Value.date > 5) {
			COleDateTime dt = VarDateTime(m_fldCheckInTime->Value);
			m_strCheckInTime = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
		}
		else {
			m_strCheckInTime = "";
		}
	}
	return m_strCheckInTime;
}

//DRT 9/10/2008 - PLID 31127 - Convert to properly formatted date
const CString& CReservationReadSet::GetCheckOutTime()
{
	if(NeedRefresh(51)) {
		// (a.walling 2013-06-18 11:25) - PLID 57199 - to avoid ANSI_WARNINGS when using MAX() with a NULL value, we use a sentinel sql datetime value of 0 for NULL, which ends up being DATE value 2		
		if(m_fldCheckOutTime->Value.vt == VT_DATE && m_fldCheckOutTime->Value.date > 5) {
			COleDateTime dt = VarDateTime(m_fldCheckOutTime->Value);
			m_strCheckOutTime = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
		}
		else {
			m_strCheckOutTime = "";
		}
	}
	return m_strCheckOutTime;
}

//TES 11/12/2008 - PLID 11465 - Added Input Date
const COleDateTime& CReservationReadSet::GetInputDate()
{
	if (NeedRefresh(52)) m_dtInput = AdoFldDateTime(m_fldInputDate, m_dtInvalid);
	return m_dtInput;
}

//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
const CString& CReservationReadSet::GetRefPhysName()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strRefPhysName = "";
	}
	else {
		if (NeedRefresh(53)) m_strRefPhysName = AdoFldString(m_fldRefPhysName, "");
	}
	return m_strRefPhysName;
}

const CString& CReservationReadSet::GetRefPhysWorkPhone()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strRefPhysWorkPhone = "";
	}
	else {
		if (NeedRefresh(54)) m_strRefPhysWorkPhone = AdoFldString(m_fldRefPhysWorkPhone, "");
	}
	return m_strRefPhysWorkPhone;
}
const CString& CReservationReadSet::GetRefPhysFax()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strRefPhysFax = "";
	}
	else {
		if (NeedRefresh(55)) m_strRefPhysFax = AdoFldString(m_fldRefPhysFax, "");
	}
	return m_strRefPhysFax;
}

// (c.haag 2008-11-20 16:16) - PLID 31128 - Insurance referral summary
const CString& CReservationReadSet::GetInsuranceReferralSummary()
{
	//TES 1/14/2010 - PLID 36762 - Hide if patient is blocked.
	if(IsPatientBlocked()) {
		m_strInsuranceReferralSummary = "";
	}
	else {
		if (NeedRefresh(56)) m_strInsuranceReferralSummary = AdoFldString(m_fldInsuranceReferralSummary, "");
	}
	return m_strInsuranceReferralSummary;
}

// (j.armen 2011-07-05 13:12) - PLID 44205
const CString& CReservationReadSet::GetConfirmedBy()
{
	if (NeedRefresh(58)) {m_strConfirmedBy = AdoFldString(m_fldConfirmedBy, ""); }
	return m_strConfirmedBy;
}

// (z.manning 2011-07-01 16:51) - PLID 23273
const COleDateTime& CReservationReadSet::GetArrivalTime()
{
	if (NeedRefresh(57)) { m_dtArrivalTime = AdoFldDateTime(m_fldArrivalTime, m_dtInvalid); }
	return m_dtArrivalTime;
}

// (j.luckoski 2012-05-09 13:58) - PLID 11597 - Get cancelledDate
const COleDateTime& CReservationReadSet::GetCancelledDate()
{
	if (NeedRefresh(59)) { m_dtCancelledDate = AdoFldDateTime(m_fldCancelledDate, m_dtInvalid); }
	return m_dtCancelledDate;
}

// (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
const CString& CReservationReadSet::GetApptPriIns()
{
	if (NeedRefresh(60)) { m_strApptPriIns = AdoFldString(m_fldApptPriInsCo, ""); }
	return m_strApptPriIns;
}

// (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
const CString& CReservationReadSet::GetApptSecIns()
{
	if (NeedRefresh(61)) { m_strApptPriIns = AdoFldString(m_fldApptSecInsCo, ""); }
	return m_strApptPriIns;
}

// (j.gruber 2012-08-06 13:17) - PLID 51926 - Insurance Information
const CString& CReservationReadSet::GetApptInsList()
{
	if (NeedRefresh(62)) { m_strApptInsList = AdoFldString(m_fldApptInsCoList, ""); }
	return m_strApptInsList;
}

// (j.gruber 2013-01-08 15:24) - PLID 54497 - referring physician
const CString& CReservationReadSet::GetApptRefPhys()
{
	if (NeedRefresh(63)) { m_strApptRefPhysName = AdoFldString(m_fldApptRefPhys, ""); }
	return m_strApptRefPhysName;
}

// (s.tullis 2014-01-23 17:10) - PLID 49748 - Add nexweb activation code to the  extra info fields

const CString& CReservationReadSet::GetNxWebCode()
{
	if (NeedRefresh(64)) { m_strNxWebCode = AdoFldString(m_fldNxWebCode, ""); }
	return m_strNxWebCode;
}
// (s.tullis 2014-01-28 13:35) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
const CString& CReservationReadSet::GetNxWebCodeDate()
{
	if (NeedRefresh(65)) {
			COleDateTime dtCreation = AdoFldDateTime(m_fldNxWebCodeDate, COleDateTime());
			int nExpirationDays = AdoFldLong(m_fldNxWebCodeExpDate, -1);
			if( dtCreation != COleDateTime() ){
				if( nExpirationDays > 0 ){
					COleDateTimeSpan dsSpan = COleDateTimeSpan(nExpirationDays, 0, 0, 0);
					m_strNxWebCodeDate= FormatDateTimeForInterface(dtCreation+dsSpan, DTF_STRIP_SECONDS, dtoDate);
				}else{
					m_strNxWebCodeDate=  "<None>";
				}
			}else{
				m_strNxWebCodeDate= "";
			}
	
	}

	return m_strNxWebCodeDate;
}

const long CReservationReadSet::GetStatusColor()
{
	if (NeedRefresh(40)) m_StatusColor = AdoFldLong(m_fldStatusColor, 0);
	return m_StatusColor;
}

// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetMixRuleColor
const long CReservationReadSet::GetMixRuleColor()
{
	if (NeedRefresh(67)) {
		m_nMixRuleColor = AdoFldLong(m_fldMixRuleColor, -1);
	}
	return m_nMixRuleColor;
}

// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetStatusOrMixRuleColor, which will return the mix rule color
// if one exists and the appt. is pending, otherwise it returns the status color
const long CReservationReadSet::GetStatusOrMixRuleColor()
{
	//don't use the member booleans, they may need to be updated first,
	//so call their Get functions
	long nStatusColor = GetStatusColor();
	long nMixRuleColor = GetMixRuleColor();
	long nNoShow = GetNoShow();

	//if the appt is pending (no show is zero) and a mix color exists, return the mix color
	if (nMixRuleColor != -1 && nNoShow == 0) {
		return nMixRuleColor;
	}
	//otherwise return the status color
	else {
		return nStatusColor;
	}
}

const long CReservationReadSet::GetAptTypeID()
{
	if (NeedRefresh(41)) m_nAptTypeID = AdoFldLong(m_fldAptTypeID, -1);
	return m_nAptTypeID;
}

const CString CReservationReadSet::GetResourceIDString()
{
	if (NeedRefresh(42)) m_strResourceIDs = AdoFldString(m_fldAptResourceIDString, "");
	return m_strResourceIDs;
}

const CString CReservationReadSet::GetPurposeIDString()
{
	if (NeedRefresh(43)) m_strPurposeIDs = AdoFldString(m_fldAptPurposeIDString, "");
	return m_strPurposeIDs;
}

// (j.jones 2014-12-03 10:58) - PLID 64274 - added GetIsMixRuleOverridden
const bool CReservationReadSet::GetIsMixRuleOverridden()
{
	//the main Reservation query checks to see if the override is still relevant,
	//so we do not need to call IsAppointmentMixRuleOverrideCurrent here
	if (NeedRefresh(66)) {
		m_bIsMixRuleOverridden = AdoFldBool(m_fldIsMixRuleOverridden, FALSE) ? true : false;
	}
	return m_bIsMixRuleOverridden;
}

// (j.luckoski 2012-06-20 11:32) - PLID 11597 - Use a bool to indicate on whether to make the text in the formatted reservation
// text box cancelled color or whether to allow it to function correctly and to add Cancelled through AddTextItem or not
void CReservationReadSet::MakeBoxText(long nProp_ColorApptTextWithStatus, bool bIsCancelled, bool bIsMixRuleOverridden)
{
	LPCTSTR pCurFmt = NULL;
	LPCTSTR pBoxFmtEnd = NULL;
	CString strFmt;

	// (j.luckoski 2012-06-20 11:36) - PLID 11597 - If cancelled send cancelled color and not the normal color
	COLORREF clrDefaultTextColor = nProp_ColorApptTextWithStatus ? GetStatusColor() : RGB(0, 0, 0);
	if (bIsCancelled) {
		clrDefaultTextColor = nProp_ColorApptTextWithStatus ? GetRemotePropertyInt("SchedCancelledApptColor", RGB(192, 192, 192), 0, GetCurrentUserName(), true) : RGB(0, 0, 0);
	}
	
	GetResBoxFormat(&pCurFmt, &pBoxFmtEnd, m_pSchedView->m_bShowAppointmentLocations, m_pSchedView->m_bShowApptShowState == 0 ? false : true, m_pSchedView->m_bShowPatientInformation, m_pSchedView->m_bShowAppointmentResources, clrDefaultTextColor, m_pSchedView->m_nExtraInfoStyles);		
	
	ASSERT(pCurFmt != NULL && pBoxFmtEnd != NULL);

	char *pCurOutText = m_BoxText;
	char *pCurOutTextMax = pCurOutText + BOX_TEXT_LEN - 1;

	LPCTSTR pNextFmt = pCurFmt;
	
	// Find the first [ character
	while ((*pNextFmt != '[') && (*pNextFmt != NULL)) pNextFmt++;
	
	// Loop while not end of format string
	while (pNextFmt < pBoxFmtEnd) {
		// Add the formatted string to the box text
		// (j.luckoski 2012-06-20 11:37) - PLID 11597 - If cancelled, tell the AddTextItem to look for the Cancel and take care
		// of either adding CANCELLED or ""
		// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
		AddTextItem(pCurFmt, pNextFmt, pCurOutText, pCurOutTextMax, bIsCancelled, bIsMixRuleOverridden);
		
		// Get pointer to the character after the last formatting section
		pCurFmt = pNextFmt;

		// Find the next formatting section
		while ((*pNextFmt != '[') && (*pNextFmt != NULL)) pNextFmt++;
	}
	
	// Add the rest of the string
	for (; pCurFmt<pNextFmt && pCurOutText<pCurOutTextMax; pCurOutText++, pCurFmt++) *pCurOutText = *pCurFmt;

	// (c.haag 2008-03-20 10:42) - PLID 29328 - We need to append inventory information to
	// *pCurOutText. This text is not related to "More Info" information, but must be appended
	// to the end of what's currently in the string.
	if (CanShowOpenAllocationsInSchedule() && GetHasOpenAllocation()) {
		// (j.jones 2009-02-23 14:40) - PLID 32772 - renamed to has pending allocation
		CString str("   (Has pending allocation)");
		strncpy(pCurOutText, "%2b", 3); pCurOutText += 3;
		strncpy(pCurOutText, str, str.GetLength()); pCurOutText += str.GetLength();
	}
	if (CanShowOpenOrdersInSchedule() && GetHasOpenOrder()) {
		CString str("   (Has open inventory order)");
		strncpy(pCurOutText, "%2b", 3); pCurOutText += 3;
		strncpy(pCurOutText, str, str.GetLength()); pCurOutText += str.GetLength();
	}

	*pCurOutText = NULL; // Terminate the string
}

//////////////////////////////////////////////////////////////////////
// These are not robust MACROS and are for local use only! They're fast, though.
#define PreCopy(a,b,c) for (; (*a) && (*a != '['); a++, b++) { *b = *a; if (b>=c) return; }
#define CopyBuffer(a,b,c) for (i=0; a[i]; i++) { *(b++) = a[i]; if (b>=c) return; if(a[i] == '%') *(b++) = '%'; }
#define PostCopy(a,b,c) for (; a<pEnd; a++) { *(b++) = *a; if (b>=c) return; }
#define CopyCStringBuffer(a,b,c) pChar = (LPCTSTR)a; \
		if (pChar[i]) { \
			PreCopy(strPreFmt, b, c); \
			CopyBuffer(pChar, b, c); \
			PostCopy(strFormat, b, c); \
		}
//////////////////////////////////////////////////////////////////////

// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
void CReservationReadSet::AddTextItem(const char *strPreFmt, const char *& strFormat, char *& strAddTo, char *pMax, bool bIsCancelled, bool bIsMixRuleOverridden)
{
	if (*strFormat != '[') return;

	long i = 0;
	const char * pChar;

	// Point to the character after the open bracket
	strFormat++;

	// Find the end of the section
	const char *pEnd;
	for (pEnd=strFormat; (*pEnd != ']') && (*pEnd != NULL); pEnd++);
	
	
	 //Return whatever the user requests
	 //If the request is unknown, return the request
	// (j.luckoski 2012-06-20 11:38) - PLID 11597 - Added pCancel to handle the CANCELLED: text in the correct text color if applicable
	if (memcmp(strFormat, "pCancel", 7) == 0) {
		strFormat += 7;
		if(bIsCancelled) {
			PreCopy(strPreFmt, strAddTo, pMax);
			pChar = (LPCTSTR)"CANCELLED:";
			CopyBuffer(pChar, strAddTo, pMax);
			PostCopy(strFormat, strAddTo, pMax);
		} else {
			PreCopy(strPreFmt, strAddTo, pMax);
			pChar = (LPCTSTR)"";
			CopyBuffer(pChar, strAddTo, pMax);
			PostCopy(strFormat, strAddTo, pMax);
		}		
	}
	// (j.jones 2014-12-03 10:40) - PLID 64274 - if the appt. has overridden a mix rule, show the word OVERRIDE:
	else if (memcmp(strFormat, "pOverride", 9) == 0) {
		strFormat += 9;
		if (bIsMixRuleOverridden) {
			PreCopy(strPreFmt, strAddTo, pMax);
			pChar = (LPCTSTR)"OVERRIDE:";
			CopyBuffer(pChar, strAddTo, pMax);
			PostCopy(strFormat, strAddTo, pMax);
		}
		else {
			PreCopy(strPreFmt, strAddTo, pMax);
			pChar = (LPCTSTR)"";
			CopyBuffer(pChar, strAddTo, pMax);
			PostCopy(strFormat, strAddTo, pMax);
		}
	}
	else if (memcmp(strFormat, "pName", 5) == 0) {
		strFormat += 5;
		if (GetPersonID() >= 0) {
			PreCopy(strPreFmt, strAddTo, pMax);
			pChar = (LPCTSTR)GetPatientName();
			CopyBuffer(pChar, strAddTo, pMax);
			PostCopy(strFormat, strAddTo, pMax);
		}
	} else if (memcmp(strFormat, "rPurpose", 8) == 0) {
		strFormat += 8;
		CopyCStringBuffer(GetAptPurpose(), strAddTo, pMax);
	} else if (memcmp(strFormat, "rNotes", 6) == 0) {
		strFormat += 6;
		CopyCStringBuffer(GetNotes(), strAddTo, pMax);
	} else if (memcmp(strFormat, "rMoveUpConfirmed", 16) == 0) {
		strFormat += 16;
		if (GetMoveUp()) {
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'm';
			if (strAddTo>=pMax) return;
			if (GetConfirmed() == 1) {
				*(strAddTo++) = 'c';
				if (strAddTo >= pMax) return;
			}
			else if (GetConfirmed() == 2)
			{
				*(strAddTo++) = 'l';
				if (strAddTo >= pMax) return;
			}
			if (GetReady()) {
				*(strAddTo++) = 'r';
				if (strAddTo >= pMax) return;
			}
		PostCopy(strFormat, strAddTo, pMax);
		} else if (GetConfirmed() >= 1 && GetConfirmed() <= 2) {
			PreCopy(strPreFmt, strAddTo, pMax);

			if (GetConfirmed() == 1)
				*(strAddTo++) = 'c';
			else if (GetConfirmed() == 2)
			{
				*(strAddTo++) = 'l';
			}
			if (strAddTo >= pMax) return;
			
			if (GetReady()) {
				*(strAddTo++) = 'r';
				if (strAddTo >= pMax) return;
			}
			PostCopy(strFormat, strAddTo, pMax);
		} else if (GetReady()) {
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'r';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
		}
		
	} 
	else if (memcmp(strFormat, "rShowState", 10) == 0) {
		strFormat += 10;

		PreCopy(strPreFmt, strAddTo, pMax);
		*(strAddTo++) = GetNoShowSymbol().GetAt(0);
		if (strAddTo >= pMax) return;
		PostCopy(strFormat, strAddTo, pMax);

		/*switch (GetNoShow()) {
		case 0: // Pending appointment
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'p';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
			break;
		case 1: // In
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'i';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
			break;
		case 2: // Out
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'o';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
			break;
		case 3: // No Show
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'n';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
			break;
		case 4: // Received
			PreCopy(strPreFmt, strAddTo, pMax);
			*(strAddTo++) = 'x';
			if (strAddTo >= pMax) return;
			PostCopy(strFormat, strAddTo, pMax);
		}*/
	}
	else if (memcmp(strFormat, "rPurpType", 9) == 0) {
		strFormat += 9;
		CopyCStringBuffer(GetAptType(), strAddTo, pMax);
	} else if (memcmp(strFormat, "rLocationName", 13) == 0) {
		strFormat += 13;
		CopyCStringBuffer(GetLocationName(), strAddTo, pMax);
	} else if (memcmp(strFormat, "rResourceName", 13) == 0) {
		strFormat += 13;
		CopyCStringBuffer(GetResourceString(), strAddTo, pMax);
	} else if (memcmp(strFormat, "ExtraInfo", 9) == 0) {
		long tmpExtraOrd = atol(strFormat+9);
		CString strItem = GetExtraInfo(tmpExtraOrd);
		strFormat += 10;
		CopyCStringBuffer(strItem, strAddTo, pMax);
	} else {
		PreCopy(strPreFmt, strAddTo, pMax);
		PostCopy(strFormat, strAddTo, pMax);
	}

	// Move to the next character if there is any string left
	for (; strFormat<pEnd; strFormat++);
	if (*strFormat) strFormat++;
}

CString CReservationReadSet::GetExtraInfo(int nOrdinal)
{
	if (m_pSchedView && m_pSchedView->m_bExtraInfo[nOrdinal]) {
		switch (m_pSchedView->m_nExtraFields[nOrdinal]) {
		case efID:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					char strID[128];
					_itoa(nPatID, strID, 10);
					return CString("ID:") + CString(strID);
				} else {
					return "";
				}
			}
			break;
		case efHomePhone:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					LPCTSTR strHomePhone = GetHomePhone();
					if (strcmp(strHomePhone, "Private") == 0) {
						return "H:Private";
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					}else if (strcmp(strHomePhone, "<Hidden>") ==0) {
						return "H:<Hidden>";
					} else if (strlen(strHomePhone) > 0) {
						CString strAns;
						if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
							FormatText (strHomePhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
						}
						else {
							//DRT 10/30/2007 - PLID 27910 - Needs to show the phone as-is if we're not formatting
							strAns = strHomePhone;
						}
						return "H:" + strAns;
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efWorkPhone:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					LPCTSTR strWorkPhone = GetWorkPhone();
					if (strcmp(strWorkPhone, "Private") == 0) {
						return "W:Private";
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					}else if (strcmp(strWorkPhone, "<Hidden>") ==0) {
						return "W:<Hidden>";
					} else if (strlen(strWorkPhone) > 0) {
						CString strAns;
						if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
							FormatText (strWorkPhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
						}
						else {
							//DRT 10/30/2007 - PLID 27910 - Needs to show the phone as-is if we're not formatting
							strAns = strWorkPhone;
						}
						if (GetWorkExt().GetLength())
							strAns += " x" + GetWorkExt();
						return "W:" + strAns;
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efBirthDate:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					COleDateTime dt = GetBirthDate();
					if (dt.GetStatus() == COleDateTime::valid) {
						return "BD:" + FormatDateTimeForInterface(dt, NULL, dtoDate);
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		// (z.manning 2008-11-19 14:29) - PLID 12282 - Age
		case efAge:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					COleDateTime dtBirthDate = GetBirthDate();
					COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
					if (dtBirthDate.GetStatus() == COleDateTime::valid && dtBirthDate < dtCurrent) {
						// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
						//  validation should only be done when bdays are entered/changed
						// (z.manning 2010-01-13 11:32) - PLID 22672 - Age is now a string
						return "Age:" + GetPatientAgeOnDate(dtBirthDate, dtCurrent, TRUE);
					}
					else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efMobilePhone:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					LPCTSTR strMobilePhone = GetMobilePhone();
					if (strcmp(strMobilePhone, "Private") == 0) {
						return "M:Private";
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					}else if (strcmp(strMobilePhone, "<Hidden>") ==0) {
						return "M:<Hidden>";
					} else if (strlen(strMobilePhone) > 0) {
						CString strAns;
						if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
							FormatText (strMobilePhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
						}
						else {
							//DRT 10/30/2007 - PLID 27910 - Needs to show the phone as-is if we're not formatting
							strAns = strMobilePhone;
						}
						return "M:" + strAns;
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efPrefContact:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					long nPreferredContact = GetPreferredContact();
					CString strPrefContact = "";
					CString strPrefContactInfo = "", strAns = "";
					int iFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
					switch(nPreferredContact) {
						case 1:
							strPrefContactInfo = GetHomePhone();
							if (strcmp(strPrefContactInfo, "Private") != 0 && strcmp(strPrefContactInfo, "<Hidden>") != 0 && strlen(strPrefContactInfo) > 0) {
								if(iFormatPhoneNums == 1) {
									FormatText(strPrefContactInfo, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
									strPrefContactInfo = strAns;
								}
							} else {
								strPrefContactInfo = "";
							}
							if(strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:H:" + strPrefContactInfo;
							break;
						case 2:
							strPrefContactInfo = GetWorkPhone();
							if (strcmp(strPrefContactInfo, "Private") != 0 && strcmp(strPrefContactInfo, "<Hidden>") != 0 && strlen(strPrefContactInfo) > 0) {
								if(iFormatPhoneNums == 1) {
									FormatText(strPrefContactInfo, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
									if (GetWorkExt().GetLength())
										strAns += " x" + GetWorkExt();
									strPrefContactInfo = strAns;
								}
							} else {
								if (strcmp(strPrefContactInfo, "Private") != 0 && strcmp(strPrefContactInfo, "<Hidden>") != 0 && GetWorkExt().GetLength())
									strPrefContactInfo = "x" + GetWorkExt();
								else
									strPrefContactInfo = "";
							}
							if(strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:W:" + strPrefContactInfo;
							break;
						case 3:
							strPrefContactInfo = GetMobilePhone();
							if (strcmp(strPrefContactInfo, "Private") != 0 && strcmp(strPrefContactInfo, "<Hidden>") != 0 && strlen(strPrefContactInfo) > 0) {
								if(iFormatPhoneNums == 1) {
									FormatText(strPrefContactInfo, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
									strPrefContactInfo = strAns;
								}
							} else {
								strPrefContactInfo = "";
							}
							if(strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:M:" + strPrefContactInfo;
							break;
						case 4:
							strPrefContactInfo = GetPager();
							if(strcmp(strPrefContactInfo, "<Hidden>") != 0 && strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:P:" + strPrefContactInfo;
							break;							
						case 5:
							strPrefContactInfo = GetOtherPhone();
							if(strcmp(strPrefContactInfo, "<Hidden>") != 0 && strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:O:" + strPrefContactInfo;
							break;
						case 6:
							strPrefContactInfo = GetEmail();
							if(strcmp(strPrefContactInfo, "<Hidden>") != 0 && strPrefContactInfo.GetLength() > 0)
								strPrefContact = "P:E:" + strPrefContactInfo;
							break;
						default:
							strPrefContact = "";
							break;
					}
					return strPrefContact;
				}
				else {
					return "";
				}
			}
			break;
		case efPatBalance: // (c.haag 2003-07-18 15:20) - Patient balance
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					if(!(GetCurrentUserPermissions(bioPatientBilling) & SPT__R_________ANDPASS)) {
						return "Pat:Private";
					}
					else {
						//TES 1/14/2010 - PLID 36762 - This may be empty now, if the patient is blocked.
						CString strPatBal = GetPatBal();
						if(!strPatBal.IsEmpty()) {
							return "Pat:" + strPatBal;
						}
						else {
							return "";
						}
					}
				}
				else {
					return "";
				}
			}
			break;
		case efInsBalance: // (c.haag 2003-07-18 15:20) - Insurance balance
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					if(!(GetCurrentUserPermissions(bioPatientBilling) & SPT__R_________ANDPASS)) {
						return "Ins:Private";
					}
					else {
						//TES 1/14/2010 - PLID 36762 - This may be empty now, if the patient is blocked.
						CString strInsBal = GetInsBal();
						if(!strInsBal.IsEmpty()) {
							return "Ins:" + GetInsBal();
						}
						else {
							return "";
						}
					}
				}
				else {
					return "";
				}
			}
			break;
		case efRefSource:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					CString strRef = GetRefSource();
					if(strRef.GetLength() > 0) 
						return "R:" + GetRefSource();
					else
						return "";
				}
				else {
					return "";
				}
			}
			break;
		case efDuration:
			{
				COleDateTime dtStart = GetStartTime();
				COleDateTime dtEnd = GetEndTime();
				//Both valid, not both midnight.
				if(dtStart.GetStatus() == COleDateTime::valid && dtEnd.GetStatus() == COleDateTime::valid &&
					(dtStart.GetHour() != 0 || dtStart.GetMinute() != 0 || dtEnd.GetHour() != 0 || dtEnd.GetMinute() != 0) ) {
					CString strReturn;
					strReturn.Format("D:%li:%02li", (dtEnd-dtStart).GetHours(), (dtEnd-dtStart).GetMinutes());
					return strReturn;
				}
				else {
					return "";
				}
			}
		case efEmail:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					CString strEmail = GetEmail();
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					if (strcmp(strEmail, "<Hidden>") == 0) {
						return "E:<Hidden>";
					}
					else if(strEmail.GetLength() > 0) 
						return "E:" + GetEmail();
					else
						return "";
				}
				else {
					return "";
				}
			}
			break;
		case efPager:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					LPCTSTR strPager = GetPager();
					if (strcmp(strPager, "Private") == 0) {
						return "P:Private";
					}
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					else if (strcmp(strPager, "<Hidden>") ==0) {
						return "P:<Hidden>";
					} else if (strlen(strPager) > 0) {
						CString strAns;
						if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
							FormatText (strPager, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
						}
						else {
							//DRT 10/30/2007 - PLID 27910 - Needs to show the phone as-is if we're not formatting
							strAns = strPager;
						}
						return "P:" + strAns;
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efOtherPhone:
			{
				long nPatID = GetUserDefinedID();
				if (nPatID >= 0) {
					LPCTSTR strOtherPhone = GetOtherPhone();
					if (strcmp(strOtherPhone, "Private") == 0) {
						return "O:Private";
					}
					// (j.gruber 2007-08-09 08:45) - PLID 25045 - make the text show correctly if its hidden
					else if (strcmp(strOtherPhone, "<Hidden>") ==0) {
						return "O:<Hidden>";
					} else if (strlen(strOtherPhone) > 0) {
						CString strAns;
						if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
							FormatText (strOtherPhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
						}
						else {
							//DRT 10/30/2007 - PLID 27910 - Needs to show the phone as-is if we're not formatting
							strAns = strOtherPhone;
						}
						return "O:" + strAns;
					} else {
						return "";
					}
				}
				else {
					return "";
				}
			}
			break;
		case efCustom1:
			{
			CString strAns = GetCustom1();
			if(strAns != "") strAns = "C1:"+strAns;
			return strAns;
			}
			break;
		case efCustom2:
			{
			CString strAns = GetCustom2();
			if(strAns != "") strAns = "C2:"+strAns;
			return strAns;
			}
			break;
		case efCustom3:
			{
			CString strAns = GetCustom3();
			if(strAns != "") strAns = "C3:"+strAns;
			return strAns;
			}
			break;
		case efCustom4:
			{
			CString strAns = GetCustom4();
			if(strAns != "") strAns = "C4:"+strAns;
			return strAns;
			}
			break;

		case efLastLM:
			{
				COleDateTime dt = GetLMDate();
				if (dt.GetStatus() == COleDateTime::valid && dt.m_dt > 0) {
					return "LM:" + FormatDateTimeForInterface(dt, NULL, dtoDateTime);
				} else {
					return "";
				}
			}
			break;

		case efInputName:
			{
			CString strAns = GetInputName();
			if(strAns != "") strAns = "I:"+strAns;
			return strAns;
			}
			break;

		case efModifiedLogin:
			{
			CString strAns = GetModifiedLogin();
			if(strAns != "") strAns = "MB:"+strAns;
			return strAns;
			}
			break;

		case efModifiedDate:
			{
				COleDateTime dt = GetModifiedDate();
				if (dt.GetStatus() == COleDateTime::valid && dt.m_dt > 0) {
					return "MD:" + FormatDateTimeForInterface(dt, NULL, dtoDate);
				} else {
					return "";
				}
			}
			break;
		// (d.moore 2007-06-05 15:33) - PLID 13550 - Adding Insurance related fields.
		case efPrimaryInsurance:
			{
				CString strPriIns = GetPrimaryInsurance();
				if (strPriIns.GetLength() > 0) {
					// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
					return "Med Pri Ins:" + strPriIns;
				} else {
					return "";
				}
			}
			break;
		case efPrimaryCoPay:
			{
				CString strPriCoPay = GetPrimaryCoPay();
				if (strPriCoPay.GetLength() > 0) {
					// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
					return "Med Pri Co:" + strPriCoPay;
				} else {
					return "";
				}
			}
			break;
		case efSecondaryInsurance:
			{
				CString strSecIns = GetSecondaryInsurance();
				if (strSecIns.GetLength() > 0) {
					// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
					return "Med Sec Ins:" + strSecIns;
				} else {
					return "";
				}
			}
			break;
		case efSecondaryCoPay:
			{
				CString strSecCoPay = GetSecondaryCoPay();
				if (strSecCoPay.GetLength() > 0) {
					// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
					return "Med Sec Co:" + strSecCoPay;
				} else {
					return "";
				}
			}
			break;

		case efCheckInTime:	//DRT 9/10/2008 - PLID 31127
			{
				CString strCheckIn = GetCheckInTime();
				if(!strCheckIn.IsEmpty()) {
					return "In:" + strCheckIn;
				}
				else {
					return "";
				}
			}
			break;

		case efCheckOutTime:	//DRT 9/10/2008 - PLID 31127
			{
				CString strCheckOut = GetCheckOutTime();
				if(!strCheckOut.IsEmpty()) {
					return "Out:" + strCheckOut;
				}
				else {
					return "";
				}
			}
			break;

		case efInputDate:	//TES 11/12/2008 - PLID 11465 - Added InputDate
			{
				COleDateTime dt = GetInputDate();
				if (dt.GetStatus() == COleDateTime::valid && dt.m_dt > 0) {
					return "IDate:" + FormatDateTimeForInterface(dt, NULL, dtoDate);
				} else {
					return "";
				}
			}
			break;

		//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
		case efRefPhysName:	
			{
				CString str = GetRefPhysName();
				if(!str.IsEmpty()) {
					return "RP:" + str;
				}
				else {
					return "";
				}
			}
			break;
		case efRefPhysWorkPhone:
			{
				CString str = GetRefPhysWorkPhone();
				if(!str.IsEmpty()) {
					return "RPTel:" + str;
				}
				else {
					return "";
				}
			}
			break;
		case efRefPhysFax:
			{
				CString str = GetRefPhysFax();
				if(!str.IsEmpty()) {
					return "RPFax:" + str;
				}
				else {
					return "";
				}
			}
			break;
		// (c.haag 2008-11-20 16:15) - PLID 31128 - Insurance referral summary
		case efInsuranceReferrals:
			{
				CString str = GetInsuranceReferralSummary();
				if(!str.IsEmpty()) {
					return "Ins. Referrals: " + str;
				}
				else {
					return "";
				}
			}
			break;
		case efConfirmedBy: // (j.armen 2011-07-05 17:04) - PLID 44205
			{
				CString str = GetConfirmedBy();
				if(!str.IsEmpty()) {
					return "Conf. By: " + str;
				}
				else {
					return "";
				}
			}
			break;
		case efArrivalTime: // (z.manning 2011-07-01 16:17) - PLID 23273
			{
				COleDateTime dtArrival = GetArrivalTime();
				CString strArrivalTime;
				if(dtArrival.GetStatus() == COleDateTime::valid) {
					strArrivalTime = "Arrival Time:" + FormatDateTimeForInterface(dtArrival, DTF_STRIP_SECONDS, dtoTime);
				}
				return strArrivalTime;
			}
			break;
		case efCancelledDate: // (j.luckoski 2012-05-09 13:53) - PLID 11597 - CancelledDate
			{
				COleDateTime dtCancelled = GetCancelledDate();
				CString strCancelledDate;
				if (dtCancelled.GetStatus() == COleDateTime::valid && dtCancelled.m_dt > 0) {
					strCancelledDate = "Cancelled Date:" + FormatDateTimeForInterface(dtCancelled, NULL, dtoDate);
				}
				return strCancelledDate;
			}
			break;
		case efApptPrimaryIns: // (j.gruber 2012-08-06 13:23) - PLID 51926
			{
				CString strApptPriIns = GetApptPriIns();
				if (!strApptPriIns.IsEmpty()) {
					return "Appt Pri Ins: " + strApptPriIns;
				}
				return "";
			}
			break;
		case efApptSecondaryIns: // (j.gruber 2012-08-06 13:23) - PLID 51926
			{
				CString strApptSecIns = GetApptSecIns();
				if (!strApptSecIns.IsEmpty()) {
					return "Appt Sec Ins: " + strApptSecIns;
				}
				return "";
			}
			break;
		case efApptInsList: // (j.gruber 2012-08-06 13:23) - PLID 51926
			{
				CString strApptInsList = GetApptInsList();
				if (!strApptInsList.IsEmpty()) {
					return "Appt Ins List: " + strApptInsList;
				}
				return "";
			}
			break;
		case efApptRefPhys: // (j.gruber 2012-08-06 13:23) - PLID 54497
			{
				CString strApptRefPhys = GetApptRefPhys();
				if (!strApptRefPhys.IsEmpty()) {
					return "Appt RP: " + strApptRefPhys;
				}
				return "";
			}
			break;
		case efNxWebCode:// (s.tullis 2014-01-23 17:26) - PLID 49748 - Add nexweb activation code to the  extra info fields
			{
				CString strNxWebCode = GetNxWebCode();
				if(!strNxWebCode.IsEmpty()){
					return "NexWeb Code:"+ strNxWebCode;
				}
				return "";

			}
		case efNxWebCodeDate:// (s.tullis 2014-01-28 11:51) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
			{
				CString strNxWebCodeDate = GetNxWebCodeDate();
				if(!strNxWebCodeDate.IsEmpty()){
					return "NexWeb Code Exp. Date:"+ strNxWebCodeDate;
				}
				return "";


			}
		default:
			return "";
			break;
		}
	} else {
		return "";
	}
}

BOOL CReservationReadSet::IsOpen()
{
	if (m_prs != NULL && m_prs->State != adStateClosed) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void CReservationReadSet::Close()
{
	HR(m_prs->Close());
}

BOOL CReservationReadSet::IsEOF()
{
	if (m_prs->eof) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CReservationReadSet::IsBOF()
{
	if (m_prs->eof) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void CReservationReadSet::MoveNext()
{
	HR(m_prs->MoveNext());
	InitNeedRefresh();
}

void CReservationReadSet::MovePrev()
{
	HR(m_prs->MovePrevious());
	InitNeedRefresh();
}


void CReservationReadSet::DestroySecurityGroupMap()
{
	//TES 1/14/2010 - PLID 36762 - Clean up all the memory allocated to m_mapPatientsToSecurityGroups
	POSITION pos = m_mapPatientsToSecurityGroups.GetStartPosition();
	long nPatientID = -1;
	CArray<long,long> *pAr = NULL;
	while (pos != NULL) {		
		m_mapPatientsToSecurityGroups.GetNextAssoc(pos, nPatientID, pAr);
		delete pAr;
	}
	m_mapPatientsToSecurityGroups.RemoveAll();
}

bool CReservationReadSet::IsPatientBlocked()
{
	//TES 1/14/2010 - PLID 36762 - Check our map for which groups this patient is a member of.  If any of them are blocked for this user
	// then the patient is blocked.
	CArray<long,long> *parGroups = NULL;
	if(m_mapPatientsToSecurityGroups.Lookup(m_PersonID, parGroups)) {
		CArray<IdName, IdName> arBlockedGroups;
		GetMainFrame()->GetBlockedGroups(arBlockedGroups);
		for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
			for(int j = 0; j < parGroups->GetSize(); j++) {
				if(arBlockedGroups[i].nID == parGroups->GetAt(j)) {
					return true;
				}
			}
		}
	}
	return false;
}

// (a.walling 2013-06-18 11:27) - PLID 57199 - Create ResExtendedQ alternative using modular SQL with Common Table Expression so a minimal query with only the data needed can be used dynamically.
namespace Nx {
namespace Scheduler {

	// what information is selected to be displayed?
	struct ResExtraFields
	{
		bool patBalances;
		bool patCustom;
		bool patRefPhys;
		bool patPrimaryIns;
		bool patPrimaryInsCopay;
		bool patSecondaryIns;
		bool patSecondaryInsCopay;
		bool patRefSourceName;
		bool patInsReferralSummary;
		bool apptTimes;
		bool apptInsInfo;
		bool apptInsList;
		bool apptRefPhys;
		bool nxWebCode;// (s.tullis 2014-01-28 08:50) - PLID 49748 - Add nexweb security code to the  extra info fields
		bool nxWebCodeDate;// (s.tullis 2014-01-28 11:50) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
		bool apptResourceString;

		ResExtraFields(CSchedulerView* pSchedView = NULL)
			: patBalances(false)
			, patCustom(false)
			, patRefPhys(false)
			, patPrimaryIns(false)
			, patPrimaryInsCopay(false)
			, patSecondaryIns(false)
			, patSecondaryInsCopay(false)
			, patRefSourceName(false)
			, patInsReferralSummary(false)
			, apptTimes(false)
			, apptInsInfo(false)
			, apptInsList(false)
			, apptRefPhys(false)
			, nxWebCode(false) // (s.tullis 2014-01-24 09:28) - PLID 49748 - Add nexweb activation code to the  extra info fields
			, nxWebCodeDate(false)// (s.tullis 2014-01-28 08:51) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
			, apptResourceString(false)
		{
			ENSURE(pSchedView);

			if (GetRemotePropertyInt("ResShowResourceName", 0, 0, GetCurrentUserName(), false)) {
				apptResourceString = true;
			}

			for (int f = 0; f < 8; ++f) {
				if (!pSchedView->m_bExtraInfo[f]) {
					continue;
				}

				switch (pSchedView->m_nExtraFields[f]) {
					case efID: //  = 0
					case efHomePhone: //  = 1
					case efWorkPhone: //  = 2
					case efBirthDate: //  = 3
					case efMobilePhone: //  = 4
					case efPrefContact: //  = 5
						break;
					case efPatBalance: //  = 6
					case efInsBalance: //  = 7
						patBalances = true;
						break;
					case efRefSource: //  = 8
						patRefSourceName = true;
						break;
					case efDuration: //  = 9
					case efPager: //  = 10
					case efOtherPhone: //  = 11
					case efEmail: //  = 12
					case efLastLM: //  = 13
					case efInputName: //  = 14
					case efModifiedLogin: //  = 15
					case efModifiedDate: //  = 16
						break;
					case efPrimaryInsurance: //  = 17
						patPrimaryIns = true;
						break;
					case efPrimaryCoPay: //  = 18
						patPrimaryIns = true;
						patPrimaryInsCopay = true;
						break;
					case efSecondaryInsurance: //  = 19
						patSecondaryIns = true;
						break;
					case efSecondaryCoPay: //  = 20
						patSecondaryIns = true;
						patSecondaryInsCopay = true;
						break;
					case efCheckInTime: //  = 21
					case efCheckOutTime: //  = 22
						apptTimes = true;
						break;
					case efInputDate: //  = 23
						break;
					case efRefPhysName: //  = 24
					case efRefPhysWorkPhone: //  = 25
					case efRefPhysFax: //  = 26
						patRefPhys = true;
						break;
					case efAge: //  = 27
						break;
					case efInsuranceReferrals: //  = 28
						patInsReferralSummary = true;
						break;
					case efConfirmedBy: //  = 29
					case efArrivalTime: //  = 30
					case efCancelledDate: //  = 31
						break;
					case efApptPrimaryIns: //  = 32
					case efApptSecondaryIns: //  = 33
						apptInsInfo = true;
						break;
					case efApptInsList: //  = 34
						apptInsList = true;
						break;
					case efApptRefPhys: //  = 35
						apptRefPhys = true;
						break;
					case efNxWebCode://=36 (s.tullis 2014-01-24 09:30) - PLID 49748 - Add nexweb activation code to the  extra info fields
						nxWebCode = true;
						break;
					case efNxWebCodeDate: // (s.tullis 2014-01-28 11:49) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
						nxWebCodeDate= true;
						break;
					case efCustom1:
					case efCustom2:
					case efCustom3:
					case efCustom4:
						patCustom = true;
						break;
					default:
						break;
				}
			}
		}
	};
	
	// returns the dynamic query for the selected fields
	// (j.jones 2014-12-05 13:49) - PLID 64274 - added date ranges and converted to CSqlFragment
	CSqlFragment GetResExtendedBase(const ResExtraFields& src, const COleDateTime dtStartDate, const COleDateTime dtEndDate)
	{
		// (j.jones 2014-12-04 10:07) - PLID 64119 - we need this preference for a few
		// decisions later in this query
		long nProp_ColorApptBgWithStatus = GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true);

		CSqlFragment sqlUnformattedPrequeries("");
		{
			CString str;
			str.Preallocate(0x8000);

			str +=
				";WITH \r\n"
				"PatientInfoC \r\n"
				"AS ( \r\n"
				"SELECT \r\n"
				"PatientsT.PersonID \r\n"
				", PersonT.FullName AS PatientName \r\n"
				", CASE WHEN PersonT.PrivCell = 1 THEN 'Private' \r\n"
				"ELSE PersonT.CellPhone \r\n"
				"END AS CellPhone \r\n"
				", CASE WHEN persont.PrivHome = 1 THEN 'Private' \r\n"
				"ELSE PersonT.HomePhone \r\n"
				"END AS HomePhone \r\n"
				", CASE WHEN persont.PrivWork = 1 THEN 'Private' \r\n"
				"ELSE PersonT.WorkPhone \r\n"
				"END AS WorkPhone \r\n"
				", CASE WHEN PersonT.PrivEmail = 1 THEN 'Private' \r\n"
				"ELSE PersonT.Email \r\n"
				"END AS Email \r\n"
				", CASE WHEN PersonT.PrivOther = 1 THEN 'Private' \r\n"
				"ELSE PersonT.OtherPhone \r\n"
				"END AS OtherPhone \r\n"
				", CASE WHEN PersonT.PrivPager = 1 THEN 'Private' \r\n"
				"ELSE PersonT.Pager \r\n"
				"END AS Pager \r\n"
				", PersonT.Extension AS WorkExt \r\n"
				", PersonT.BirthDate \r\n"
				", PatientsT.PreferredContact \r\n"
				", PatientsT.UserDefinedID \r\n";
			if (src.patRefSourceName) {
				str +=
					", ReferralSourceT.Name AS RefSourceName \r\n";
			}
			else {
				str +=
					", '' AS RefSourceName \r\n";
			}
			if (src.nxWebCode){
				str +=// (s.tullis 2014-01-24 09:38) - PLID 49748 - Add nexweb activation code to the  extra info fields
					", PatientsT.SecurityCode AS SecurityCode \r\n";
			}
			else {
				str +=
					", '' AS SecurityCode \r\n";
			}
			if (src.nxWebCodeDate){
				str +=// (s.tullis 2014-01-28 08:52) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
					", PatientsT.SecurityCodeCreationDate  \r\n"
					", (SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebSecurityCodeExpire'), -1)) AS SecurityCodeExpDate \r\n";
			}
			else {
				str +=
					", '' AS SecurityCodeCreationDate \r\n"
					", '' AS SecurityCodeExpDate  \r\n";
			}
			if (src.patRefPhys) {
				str +=
					", ReferringPhysPersonT.FullName AS RefPhysName \r\n"
					", ReferringPhysPersonT.WorkPhone AS RefPhysWorkPhone \r\n"
					", ReferringPhysPersonT.Fax AS RefPhysFax \r\n";
			}
			else {
				str +=
					", '' AS RefPhysName \r\n"
					", '' AS RefPhysWorkPhone \r\n"
					", '' AS RefPhysFax \r\n";
			}
			str +=
				"FROM PatientsT \r\n"
				"LEFT JOIN PersonT \r\n"
				"ON PersonT.ID = PatientsT.PersonID \r\n";
			if (src.patRefSourceName) {
				str +=
					"LEFT JOIN ReferralSourceT \r\n"
					"ON ReferralSourceT.PersonID = PatientsT.ReferralID \r\n";
			}
			if (src.patRefPhys) {
				str +=
					"LEFT JOIN PersonT ReferringPhysPersonT \r\n"
					"ON ReferringPhysPersonT.ID = PatientsT.DefaultReferringPhyID \r\n";
			}
			str +=
				") \r\n"
				", Custom1234C \r\n"
				"AS ( \r\n"
				"SELECT \r\n"
				"CustomFieldDataT.PersonID \r\n"
				", MAX(CASE FieldID WHEN 1 THEN TextParam ELSE '' END) AS Custom1 \r\n"
				", MAX(CASE FieldID WHEN 2 THEN TextParam ELSE '' END) AS Custom2 \r\n"
				", MAX(CASE FieldID WHEN 3 THEN TextParam ELSE '' END) AS Custom3 \r\n"
				", MAX(CASE FieldID WHEN 4 THEN TextParam ELSE '' END) AS Custom4 \r\n"
				"FROM CustomFieldDataT \r\n"
				"GROUP BY PersonID \r\n"
				") \r\n";
			if (src.patPrimaryIns || src.patSecondaryIns) {
				str +=
					", InsuranceInfoC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"InsuredPartyT.PatientID \r\n"
					", InsuredPartyT.RespTypeID \r\n"
					", InsuredPartyT.PersonID AS InsuredPartyID \r\n"
					", InsuranceCoT.Name \r\n"
					"FROM InsuredPartyT \r\n"
					"LEFT JOIN InsuranceCoT \r\n"
					"ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n"
					") \r\n";
			}
			if (src.patPrimaryInsCopay || src.patSecondaryInsCopay) {
				str +=
					", InsuranceCopaysC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"  InsuredPartyPayGroupsT.InsuredPartyID \r\n"
					", InsuredPartyPayGroupsT.CopayMoney \r\n"
					", InsuredPartyPayGroupsT.CopayPercentage \r\n"
					"FROM InsuredPartyPayGroupsT \r\n"
					"INNER JOIN ServicePayGroupsT \r\n"
					"ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PaygroupID \r\n"
					"AND ServicePayGroupsT.Name = 'Copay' \r\n"
					") \r\n";
			}

			// (j.jones 2014-12-04 10:14) - PLID 64119 - this is used in ApptInsInfoC and SchedulerMixRuleColorC
			if (src.apptInsInfo || nProp_ColorApptBgWithStatus) {
				str +=
					", ApptInsuredPartyC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"AppointmentInsuredPartyT.AppointmentID, \r\n"
					"InsuredPartyT.PatientID, \r\n"
					"InsuredPartyT.InsuranceCoID, \r\n"
					"AppointmentInsuredPartyT.Placement \r\n"
					"FROM AppointmentInsuredPartyT \r\n"
					"INNER JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID \r\n"
					") \r\n";
			}

			if (src.apptInsInfo) {
				str +=
					", ApptInsInfoC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"ApptInsuredPartyC.AppointmentID, \r\n"
					"ApptInsuredPartyC.PatientID, \r\n"
					"MAX(CASE ApptInsuredPartyC.Placement WHEN 1 THEN InsuranceCoT.Name ELSE '' END) AS ApptPriIns, \r\n"
					"MAX(CASE ApptInsuredPartyC.Placement WHEN 2 THEN InsuranceCoT.Name ELSE '' END) AS ApptSecIns \r\n"
					"FROM ApptInsuredPartyC \r\n"
					"INNER JOIN InsuranceCoT ON ApptInsuredPartyC.InsuranceCoID = InsuranceCoT.PersonID \r\n"
					"GROUP BY ApptInsuredPartyC.AppointmentID, ApptInsuredPartyC.PatientID \r\n"
					") \r\n";
			}

			if (src.apptTimes) {
				str +=
					", ShowStateHistoryC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"AptShowStateHistoryT.AppointmentID \r\n"
					", MAX(CASE AptShowStateHistoryT.ShowStateID WHEN 1 THEN AptShowStateHistoryT.TimeStamp ELSE CONVERT(DATETIME, 0) END) AS CheckinTime \r\n"
					", MAX(CASE AptShowStateHistoryT.ShowStateID WHEN 2 THEN AptShowStateHistoryT.TimeStamp ELSE CONVERT(DATETIME, 0) END) AS CheckoutTime \r\n"
					"FROM AptShowStateHistoryT \r\n"
					"GROUP BY AptShowStateHistoryT.AppointmentID \r\n"
					") \r\n";
			}
			str +=
				", OrderInfoC \r\n"
				"AS ( \r\n"
				"SELECT \r\n"
				"OrderAppointmentsT.AppointmentID \r\n"
				"FROM OrderAppointmentsT \r\n"
				"INNER JOIN OrderT \r\n"
				"ON OrderT.ID = OrderAppointmentsT.OrderID \r\n"
				"AND OrderT.Deleted = 0 \r\n"
				"INNER JOIN OrderDetailsT \r\n"
				"ON OrderDetailsT.OrderID = OrderT.ID \r\n"
				"AND OrderDetailsT.Deleted = 0 \r\n"
				"AND OrderDetailsT.DateReceived IS NULL \r\n"
				"GROUP BY OrderAppointmentsT.AppointmentID \r\n"
				") \r\n"
				", AllocationInfoC \r\n"
				"AS ( \r\n"
				"SELECT \r\n"
				"PatientInvAllocationsT.AppointmentID \r\n"
				"FROM PatientInvAllocationsT \r\n"
				"WHERE PatientInvAllocationsT.Status = 1 \r\n"
				"GROUP BY PatientInvAllocationsT.AppointmentID \r\n"
				") \r\n";
			if (src.patBalances) {
				str +=
					", BalancesC \r\n"
					"AS ( \r\n"
					"SELECT \r\n"
					"LineItemT.PatientID \r\n"
					", SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NULL THEN COALESCE(ChargeRespT.Amount, $0) ELSE $0 END) AS PatCharge \r\n"
					", SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL THEN COALESCE(ChargeRespT.Amount, $0) ELSE $0 END) AS InsCharge \r\n"
					", SUM(CASE WHEN PaymentsT.InsuredPartyID = -1 THEN COALESCE(LineItemT.Amount, $0) ELSE $0 END) AS PatPay \r\n"
					", SUM(CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN COALESCE(LineItemT.Amount, $0) ELSE $0 END) AS InsPay \r\n"
					"FROM LineItemT \r\n"
					"LEFT JOIN ChargeRespT \r\n"
					"ON ChargeRespT.ChargeID = LineItemT.ID \r\n"
					"AND LineItemT.Type = 10 \r\n"
					"LEFT JOIN PaymentsT \r\n"
					"ON PaymentsT.ID = LineItemT.ID \r\n"
					"AND LineItemT.Type <> 10 \r\n"
					"AND LineItemT.Type <> 11 \r\n"
					"WHERE \r\n"
					"LineItemT.Deleted = 0 \r\n"
					"GROUP BY LineItemT.PatientID \r\n"
					") \r\n";
			}

			// (j.jones 2014-12-04 10:07) - PLID 64119 - if they are coloring the appt. background by status,
			// we need to calculate if any scheduler mix rules apply to an appointment, and if they have
			// a color, because if they do we need to later color the appt. background with this color
			// if the status is Pending
			if (nProp_ColorApptBgWithStatus) {
				str +=
					", SchedulerMixRuleColorC \r\n"
					"AS ( \r\n"
					//if multiple rules apply, we can only use one color, so let's just arbitrarily use the highest value color
					"	SELECT AppointmentsT.ID AS AppointmentID, Max(ScheduleMixColoredRulesQ.Color) AS Color \r\n"
					"	FROM AppointmentsT \r\n"
					"	LEFT JOIN (\r\n"
					"		SELECT AppointmentID, InsuranceCoID \r\n"
					"		FROM ApptInsuredPartyC \r\n"
					"		WHERE Placement = 1 \r\n"
					"	) AS ApptInsuredPartyQ ON AppointmentsT.ID = ApptInsuredPartyQ.AppointmentID \r\n"
					"	INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID \r\n"
					//begin by finding all rules that apply to this appt. resource
					"	INNER JOIN ScheduleMixRuleDetailsT ON AppointmentResourceT.ResourceID = ScheduleMixRuleDetailsT.ResourceID \r\n"
					//we only want rules that have colors enabled
					"	INNER JOIN (SELECT * FROM ScheduleMixRulesT WHERE IsNull(Color, -1) <> -1) AS ScheduleMixColoredRulesQ ON ScheduleMixRuleDetailsT.RuleID = ScheduleMixColoredRulesQ.ID \r\n"
					"	LEFT JOIN ScheduleMixRuleLocationsT ON ScheduleMixColoredRulesQ.ID = ScheduleMixRuleLocationsT.RuleID \r\n"
					"	LEFT JOIN ScheduleMixRuleInsuranceCosT ON ScheduleMixColoredRulesQ.ID = ScheduleMixRuleInsuranceCosT.RuleID \r\n"
					"	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID \r\n"
					"	WHERE "
					//rule is active on the appt. date
					"		ScheduleMixColoredRulesQ.StartDate <= AppointmentsT.Date AND (ScheduleMixColoredRulesQ.EndDate Is Null OR ScheduleMixColoredRulesQ.EndDate >= AppointmentsT.Date) \r\n"
					//rule is either for all locations or includes the appt. location
					"		AND (ScheduleMixRuleLocationsT.LocationID Is Null OR ScheduleMixRuleLocationsT.LocationID = AppointmentsT.LocationID) \r\n"
					//rule is either for all insurance companies or includes the appt's primary insured party's insurance company
					"		AND (ScheduleMixRuleInsuranceCosT.InsuranceCoID Is Null OR ScheduleMixRuleInsuranceCosT.InsuranceCoID = ApptInsuredPartyQ.InsuranceCoID) \r\n"
					//the rule detail is either for all appt. types or for this appt. type
					"		AND (ScheduleMixRuleDetailsT.AptTypeID IS NULL OR ScheduleMixRuleDetailsT.AptTypeID = AppointmentsT.AptTypeID) \r\n"
					//the rule detail is either for all appt. types or for this appt. type
					"		AND (ScheduleMixRuleDetailsT.AptPurposeID IS NULL OR ScheduleMixRuleDetailsT.AptPurposeID = AppointmentPurposeT.PurposeID) \r\n"
					"	GROUP BY AppointmentsT.ID \r\n"
					") \r\n";
			}

			//put all our unformatted text in first
			sqlUnformattedPrequeries = CSqlFragment(str);
		}

		// (j.jones 2015-01-13 15:09) - PLID 64274 - ReservationReadSet doesn't change
		// to join in mix rule overrides unless there really are overridden appts. in the date range
		CSqlFragment sqlFormattedPrequeries("");
		CString strJoinAppointmentMixRuleOverridesC = "";
		CString strIsMixRuleOverridden("Convert(bit, 0) AS IsMixRuleOverridden");

		{
			// (j.jones 2014-12-03 11:14) - PLID 64274 - To calculate IsMixRuleOverridden,
			// we need more than just looking up AppointmentMixRuleOverridesT. We also need
			// to find out if the rule is still overridden on that appointment date.
			// Cancelled appointments, if shown, are always treated as not overridden.
			std::vector<long> aryMixRuleOverrideAppts = GetOverrideAppointmentsInDateRange(dtStartDate, dtEndDate);

			if (aryMixRuleOverrideAppts.size() > 0) {

				CSqlFragment sqlDefineAppointmentMixRuleOverridesC(
					", AppointmentMixRuleOverridesC \r\n"
					"AS ( \r\n"
					"	SELECT AppointmentID \r\n"
					"	FROM AppointmentMixRuleOverridesT \r\n"
					"	WHERE AppointmentID IN ({INTVECTOR}) \r\n"
					"	GROUP BY AppointmentID \r\n"
					") \r\n", aryMixRuleOverrideAppts);

				sqlFormattedPrequeries = CSqlFragment("{SQL} {SQL}", sqlFormattedPrequeries, sqlDefineAppointmentMixRuleOverridesC);
				strJoinAppointmentMixRuleOverridesC = "LEFT JOIN AppointmentMixRuleOverridesC ON AppointmentsT.ID = AppointmentMixRuleOverridesC.AppointmentID";
				strIsMixRuleOverridden = "Convert(bit, CASE WHEN AppointmentMixRuleOverridesC.AppointmentID Is Not Null THEN 1 ELSE 0 END) AS IsMixRuleOverridden";
			}
		}

		CSqlFragment sqlUnformattedSelectQuery("");
		{
			CString str;
			str.Preallocate(0x8000);

			str +=
				"SELECT \r\n"
				"AppointmentsT.ID \r\n"
				", AppointmentsT.PatientID \r\n"
				//"--, AppointmentsT.PurposeID -- not used \r\n"
				", AppointmentsT.StartTime \r\n"
				", AppointmentsT.EndTime \r\n"
				", AppointmentsT.Date \r\n"
				", AppointmentsT.CancelledDate \r\n"
				", AppointmentsT.ArrivalTime \r\n"
				", AppointmentsT.LastLM \r\n"
				", AppointmentsT.ModifiedDate \r\n"
				", AppointmentsT.ModifiedLogin \r\n"
				", AppointmentsT.Status \r\n"
				", AppointmentsT.Confirmed \r\n"
				", AppointmentsT.ConfirmedBy \r\n"
				", CONVERT(BIT, CASE WHEN WaitingListT.AppointmentID IS NOT NULL THEN 1 ELSE 0 END) AS MoveUp \r\n"
				", AppointmentsT.ShowState \r\n"
				", AppointmentsT.Ready \r\n"
				", AppointmentsT.Notes \r\n"
				", COALESCE(AptTypeT.Color, 0) AS Color \r\n"
				", PatientInfoC.PatientName \r\n"
				", PatientInfoC.CellPhone \r\n"
				", PatientInfoC.HomePhone \r\n"
				", PatientInfoC.WorkPhone \r\n"
				", PatientInfoC.Email \r\n"
				", PatientInfoC.OtherPhone \r\n"
				", PatientInfoC.SecurityCode \r\n"// (s.tullis 2014-01-24 09:58) - PLID 49748 - Add nexweb activation code to the  extra info fields
				", PatientInfoC.SecurityCodeCreationDate \r\n"// (s.tullis 2014-01-28 09:08) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
				", PatientInfoC.SecurityCodeExpDate \r\n" // (s.tullis 2014-01-28 09:09) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
				// (j.jones 2014-12-03 11:14) - PLID 64274 - added IsMixRuleOverridden
				", " + strIsMixRuleOverridden + " \r\n"
				", PatientInfoC.Pager \r\n"
				", PatientInfoC.WorkExt \r\n"
				", PatientInfoC.BirthDate \r\n"
				", PatientInfoC.PreferredContact \r\n"
				", PatientInfoC.UserDefinedID \r\n"
				", AptPurposeT.Name AS AptPurpose \r\n"
				", AptTypeT.Name AS AptType \r\n"
				", AppointmentsT.AptTypeID \r\n"
				", AppointmentPurposeT.PurposeID AS AptPurposeID \r\n"
				", AppointmentResourceT.ResourceID \r\n"
				", ResourceT.Item AS ResourceName \r\n"
				", AppointmentsT.LocationID \r\n"
				", LocationsT.Name AS LocationName \r\n"
				", AptShowStateT.Symbol AS ShowStateSymbol \r\n"
				", AptShowStateT.Color AS ShowStateColor \r\n";
			if (src.patBalances) {
				str +=
					", COALESCE(BalancesC.PatCharge, $0) - COALESCE(BalancesC.PatPay, $0) AS PatBal \r\n"
					", COALESCE(BalancesC.InsCharge, $0) - COALESCE(BalancesC.InsPay, $0) AS InsBal \r\n";
			}
			else {
				str +=
					", $0 AS PatBal \r\n"
					", $0 AS InsBal \r\n";
			}
			str +=
				", PatientInfoC.RefSourceName \r\n";
			if (src.patCustom) {
				str +=
					", Custom1234C.Custom1 \r\n"
					", Custom1234C.Custom2 \r\n"
					", Custom1234C.Custom3 \r\n"
					", Custom1234C.Custom4 \r\n";
			}
			else {
				str +=
					", '' AS Custom1 \r\n"
					", '' AS Custom2 \r\n"
					", '' AS Custom3 \r\n"
					", '' AS Custom4 \r\n";
			}
			str +=
				", AppointmentsT.CreatedLogin \r\n";
			if (src.patPrimaryIns) {
				str +=
					", PrimaryInsurance.Name AS PrimaryIns \r\n";
			}
			else {
				str +=
					", '' AS PrimaryIns \r\n";
			}
			if (src.patPrimaryInsCopay) {
				str +=
					", PrimaryCopays.CopayMoney	AS PriInsCopay \r\n"
					", PrimaryCopays.CopayPercentage AS PriInsCopayPer \r\n";
			}
			else {
				str +=
					", NULL AS PriInsCopay \r\n"
					", NULL AS PriInsCopayPer \r\n";
			}
			if (src.patSecondaryIns) {
				str +=
					", SecondaryInsurance.Name AS SecondaryIns \r\n";
			}
			else {
				str +=
					", '' AS SecondaryIns \r\n";
			}
			if (src.patSecondaryInsCopay) {
				str +=
					", SecondaryCopays.CopayMoney	AS SecInsCopay \r\n"
					", SecondaryCopays.CopayPercentage AS SecInsCopayPer \r\n";
			}
			else {
				str +=
					", NULL	AS SecInsCopay \r\n"
					", NULL AS SecInsCopayPer \r\n";
			}
			str +=
				", CONVERT(BIT, CASE WHEN OrderInfoC.AppointmentID IS NULL THEN 0 ELSE 1 END) AS HasOpenOrder \r\n"
				", CONVERT(BIT, CASE WHEN AllocationInfoC.AppointmentID IS NULL THEN 0 ELSE 1 END) AS HasOpenAllocation \r\n";
			if (src.apptTimes) {
				str +=
					", ShowStateHistoryC.CheckinTime \r\n"
					", ShowStateHistoryC.CheckoutTime \r\n";
			}
			else {
				str +=
					", NULL AS CheckinTime \r\n"
					", NULL AS CheckoutTime \r\n";
			}
			str +=
				", AppointmentsT.CreatedDate \r\n"
				", PatientInfoC.RefPhysName \r\n"
				", PatientInfoC.RefPhysWorkPhone \r\n"
				", PatientInfoC.RefPhysFax \r\n";
			if (src.apptInsInfo) {
				str +=
					", ApptInsInfoC.ApptPriIns \r\n"
					", ApptInsInfoC.ApptSecIns \r\n";
			}
			else {
				str +=
					", '' AS ApptPriIns \r\n"
					", '' AS ApptSecIns \r\n";
			}
			if (src.apptInsList) {
				str +=
					", dbo.GetApptInsuranceList(AppointmentsT.ID) AS ApptInsList \r\n";
			}
			else {
				str +=
					", '' AS ApptInsList \r\n";
			}
			if (src.apptRefPhys) {
				str +=
					", ApptRefPhysPersonT.FullName AS ApptRefPhysName \r\n";
			}
			else {
				str +=
					", '' AS ApptRefPhysName \r\n";
			}

			if (src.patInsReferralSummary) {
				str +=
					", dbo.InsReferralSummary(AppointmentsT.PatientID, GETDATE()) AS InsuranceReferralSummary \r\n";
			}
			else {
				str +=
					", '' AS InsuranceReferralSummary \r\n";
			}

			if (src.apptResourceString) {
				str +=
					", REPLACE(dbo.GetResourceString(AppointmentsT.ID), ' ', ',') AS ResourceString \r\n";
			}
			else {
				str +=
					", '' AS ResourceString \r\n";
			}

			str +=
				", dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDString \r\n"
				", dbo.GetPurposeIDString(AppointmentsT.ID) AS PurposeIDString \r\n";

			// (j.jones 2014-12-04 12:37) - PLID 64119 - added MixRuleColor, required when nProp_ColorApptBgWithStatus is not 0,
			// always NULL otherwise
			if (nProp_ColorApptBgWithStatus) {
				str += ", SchedulerMixRuleColorC.Color AS MixRuleColor \r\n";
			}
			else {
				str += ", NULL AS MixRuleColor \r\n";
			}

			str +=
				"FROM AppointmentsT \r\n"
				"LEFT JOIN PatientInfoC \r\n"
				"ON PatientInfoC.PersonID = AppointmentsT.PatientID \r\n"
				"LEFT JOIN AppointmentResourceT \r\n"
				"ON AppointmentResourceT.AppointmentID = AppointmentsT.ID \r\n"
				"LEFT JOIN ResourceT \r\n"
				"ON ResourceT.ID = AppointmentResourceT.ResourceID \r\n"
				"LEFT JOIN LocationsT \r\n"
				"ON LocationsT.ID = AppointmentsT.LocationID \r\n"
				"LEFT JOIN AptShowStateT \r\n"
				"ON AppointmentsT.ShowState = AptShowStateT.ID \r\n";
			if (src.patBalances) {
				str +=
					"LEFT JOIN BalancesC \r\n"
					"ON BalancesC.PatientID = AppointmentsT.PatientID \r\n";
			}
			str +=
				"LEFT JOIN AptTypeT \r\n"
				"ON AptTypeT.ID = AppointmentsT.AptTypeID \r\n"
				"LEFT JOIN AppointmentPurposeT \r\n"
				"ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID \r\n"
				"LEFT JOIN AptPurposeT \r\n"
				"ON AptPurposeT.ID = AppointmentPurposeT.PurposeID \r\n";
			if (src.apptTimes) {
				str +=
					"LEFT JOIN ShowStateHistoryC \r\n"
					"ON ShowStateHistoryC.AppointmentID = AppointmentsT.ID \r\n";
			}
			str +=
				"LEFT JOIN WaitingListT \r\n"
				"ON WaitingListT.AppointmentID = AppointmentsT.ID \r\n";
			if (src.patCustom) {
				str +=
					"LEFT JOIN Custom1234C \r\n"
					"ON Custom1234C.PersonID = AppointmentsT.PatientID \r\n";
			}
			str +=
				"LEFT JOIN OrderInfoC \r\n"
				"ON OrderInfoC.AppointmentID = AppointmentsT.ID \r\n"
				"LEFT JOIN AllocationInfoC \r\n"
				"ON AllocationInfoC.AppointmentID = AppointmentsT.ID \r\n";
			if (src.apptRefPhys) {
				str +=
					"LEFT JOIN PersonT ApptRefPhysPersonT \r\n"
					"ON ApptRefPhysPersonT.ID = AppointmentsT.RefPhysID \r\n";
			}
			if (src.patPrimaryIns || src.patPrimaryInsCopay) {
				str +=
					"LEFT JOIN InsuranceInfoC PrimaryInsurance \r\n"
					"ON PrimaryInsurance.PatientID = AppointmentsT.PatientID \r\n"
					"AND PrimaryInsurance.RespTypeID = 1 \r\n";
			}
			if (src.patPrimaryInsCopay) {
				str +=
					"LEFT JOIN InsuranceCopaysC PrimaryCopays \r\n"
					"ON PrimaryCopays.InsuredPartyID = PrimaryInsurance.InsuredPartyID \r\n";
			}
			if (src.patSecondaryIns || src.patSecondaryInsCopay) {
				str +=
					"LEFT JOIN InsuranceInfoC SecondaryInsurance \r\n"
					"ON SecondaryInsurance.PatientID = AppointmentsT.PatientID \r\n"
					"AND SecondaryInsurance.RespTypeID = 2 \r\n";
			}
			if (src.patSecondaryInsCopay) {
				str +=
					"LEFT JOIN InsuranceCopaysC SecondaryCopays \r\n"
					"ON SecondaryCopays.InsuredPartyID = SecondaryInsurance.InsuredPartyID \r\n";
			}
			if (src.apptInsInfo) {
				str +=
					"LEFT JOIN ApptInsInfoC \r\n"
					"ON ApptInsInfoC.AppointmentID = AppointmentsT.ID \r\n"
					"AND ApptInsInfoC.PatientID = AppointmentsT.PatientID \r\n";
			}

			// (j.jones 2014-12-03 11:14) - PLID 64274 - for IsMixRuleOverridden
			str += FormatString("%s \r\n", strJoinAppointmentMixRuleOverridesC);

			// (j.jones 2014-12-04 10:07) - PLID 64119 - if they are coloring the appt. background by status,
			// we need to calculate if any scheduler mix rules apply to an appointment, and if they have
			// a color, because if they do we need to later color the appt. background with this color
			// if the status is Pending
			if (nProp_ColorApptBgWithStatus) {
				str += "LEFT JOIN SchedulerMixRuleColorC ON AppointmentsT.ID = SchedulerMixRuleColorC.AppointmentID \r\n";
			}

			sqlUnformattedSelectQuery = CSqlFragment(str);
		}

		CSqlFragment sqlFinal("{SQL} {SQL} {SQL}", sqlUnformattedPrequeries, sqlFormattedPrequeries, sqlUnformattedSelectQuery);

		return sqlFinal;
	}

	// (j.jones 2014-12-05 13:49) - PLID 64274 - added date ranges and converted to CSqlFragment
	CSqlFragment GetResExtendedBase(CSchedulerView* pSchedView, const COleDateTime dtStartDate, const COleDateTime dtEndDate)
	{
		return GetResExtendedBase(ResExtraFields(pSchedView), dtStartDate, dtEndDate);
	}
}
}