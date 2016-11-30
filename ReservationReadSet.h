// ReservationReadSet.h: interface for the CReservationReadSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESERVATIONREADSET_H__949D53E3_0303_11D3_9447_00C04F4C8415__INCLUDED_)
#define AFX_RESERVATIONREADSET_H__949D53E3_0303_11D3_9447_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define BOX_TEXT_LEN		8192

class CSchedulerView;

class CReservationReadSet
{
// For backward compatibility ///////////
public:
	void MoveNext();
	void MovePrev();
	BOOL IsOpen();
	void Close();
	BOOL IsEOF();
	BOOL IsBOF();
/////////////////////////////////////////

public:
	CReservationReadSet();
	~CReservationReadSet();

protected:
	void Open(CSqlFragment query);
	char m_BoxText[BOX_TEXT_LEN];
	// (j.luckoski 2012-06-20 11:40) - PLID 11597 - Added bIsCancelled
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
	void MakeBoxText(long nProp_ColorApptTextWithStatus, bool bIsCancelled, bool bIsMixRuleOverridden);
	// (j.luckoski 2012-06-20 11:40) - PLID 11597 - Added bIsCancelled
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
	void AddTextItem(const char *strPreFmt, const char *& strFormat, char *& strAddTo, char *pMax, bool bIsCancelled, bool bIsMixRuleOverridden);
	CString GetExtraInfo(int nOrdinal);
	void InitNeedRefresh();
	void EnsureCorrectTimes();

public:
	// (j.jones 2011-02-11 10:20) - PLID 35180 - changed nItemID into a string of resource IDs
	// (a.walling 2013-06-18 11:25) - PLID 57199 - Simply refilters on a SQL fragment now
	bool ReFilter(CSqlFragment query);
	
	// Misc variables needed from outside
	CSchedulerView *m_pSchedView;

	// For Getting values out of the current record
	const COleDateTime &	GetStartTimeDt();
	const COleDateTime &	GetEndTimeDt();
	const COleDateTime &	GetStartTime();
	const COleDateTime &	GetEndTime();
	const COleDateTime &	GetDate();
	const long				GetColor();
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
	const LPCTSTR			GetBoxText(CString strMultiPurpose, long nProp_ColorApptTextWithStatus, bool bIsCancelled, bool bIsMixRuleOverridden);
	const CString &			GetPatientName();
	const CString &			GetNotes();
	const BOOL				GetMoveUp();
	const long				GetNoShow();
	const CString &			GetNoShowSymbol();
	const long				GetConfirmed();
	const BOOL				GetReady();
	const long				GetPersonID();
	const long				GetUserDefinedID();
	const COleDateTime &	GetBirthDate();
	const CString &			GetHomePhone();
	const CString &			GetWorkPhone();
	const CString &			GetWorkExt();
	const CString &			GetMobilePhone();
	const long &			GetPreferredContact();
	const CString &			GetLocationName();
	const long				GetID();
	const CString &			GetAptPurpose();
	const CString &			GetAptType();
	const long				GetResourceID();
	//TES 1/14/2010 - PLID 36762 - Changed PatBal and InsBal to CStrings, if the patient is blocked we want them to show as blank rather
	// than as $0.00
	const CString&			GetPatBal();
	const CString&			GetInsBal();
	const CString&			GetRefSource();
	const CString&			GetResourceString();
	const CString&			GetEmail();
	const CString&			GetPager();
	const CString&			GetOtherPhone();
	const CString&			GetCustom1();
	const CString&			GetCustom2();
	const CString&			GetCustom3();
	const CString&			GetCustom4();
	const CString&			GetInputName();
	const COleDateTime&		GetLMDate();
	const CString&			GetModifiedLogin();
	const COleDateTime&		GetModifiedDate();
	const long				GetStatusColor();
	// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetMixRuleColor
	const long				GetMixRuleColor();
	// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetStatusOrMixRuleColor, which will return the mix rule color
	// if one exists and the appt. is pending, otherwise it returns the status color
	const long				GetStatusOrMixRuleColor();
	const long				GetAptTypeID();
	const CString			GetResourceIDString();
	const CString			GetPurposeIDString();
	// (d.moore 2007-06-05 15:42) - PLID 13550 - Adding insurance fields
	const CString &			GetPrimaryInsurance();
	const CString &			GetSecondaryInsurance();
	const CString &			GetPrimaryCoPay();
	const CString &			GetSecondaryCoPay();
	// (c.haag 2008-03-20 10:14) - PLID 29328 - Adding inventory fields
	const BOOL				GetHasOpenOrder();
	const BOOL				GetHasOpenAllocation();
	//DRT 9/10/2008 - PLID 31127 - Added check in and check out times
	const CString &			GetCheckInTime();
	const CString &			GetCheckOutTime();

	//TES 11/12/2008 - PLID 11465 - Added InputDate
	const COleDateTime&		GetInputDate();
	//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	const CString &			GetRefPhysName();
	const CString &			GetRefPhysWorkPhone();
	const CString &			GetRefPhysFax();

	// (c.haag 2008-11-20 16:10) - PLID 31128 - Insurance Referral summary
	const CString &			GetInsuranceReferralSummary();
	const CString &			GetConfirmedBy(); // (j.armen 2011-07-05 13:12) - PLID 44205
	const COleDateTime &	GetArrivalTime(); // (z.manning 2011-07-01 16:19) - PLID 23273
	const COleDateTime &	GetCancelledDate(); // (j.luckoski 2012-05-09 13:59) - PLID 11597
	// (j.gruber 2012-08-06 13:30) - PLID 51926 - appointment Insurance
	const CString &			GetApptPriIns();
	const CString &			GetApptSecIns();
	const CString &			GetApptInsList();
	// (j.gruber 2013-01-08 15:14) - PLID 54497
	const CString &			GetApptRefPhys();
	// (s.tullis 2014-01-21 15:39) - PLID 49748 - Add nexweb activation code to the  extra info fields
	const CString &			GetNxWebCode();
	// (s.tullis 2014-01-24 15:17) - PLID 60470 - Add nexweb experation date to the  extra info fields
	const CString &			GetNxWebCodeDate();
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added GetIsMixRuleOverridden
	const bool				GetIsMixRuleOverridden();

protected:
	bool NeedRefresh(int nBoolNum);

protected:	
	// Wanted to make this a static const in the cpp but I can't figure 
	// out a good way of initializing a COleDateTime to an invalid state
	COleDateTime m_dtInvalid;

	// For maintaining consistent and non-repetitive access to the field values
	bool m_bNeedRefresh[512];
	int m_nBoolCount;
	unsigned long m_nLastStamp;

protected: // temp storage for calculating reasonable start/end times
	COleDateTime m_dtStart;
	COleDateTime m_dtEnd;

protected:
	ADODB::_RecordsetPtr m_prs; // For storing the opened recordset
	ADODB::FieldsPtr m_flds; // For quick access to the recordset's fields collection

	// For quick access to field value
	ADODB::FieldPtr m_fldID;
	ADODB::FieldPtr m_fldPersonID;
	ADODB::FieldPtr m_fldResourceID;
	ADODB::FieldPtr m_fldUserDefinedID;
	ADODB::FieldPtr m_fldDate;
	ADODB::FieldPtr m_fldStartTime;
	ADODB::FieldPtr m_fldEndTime;
	ADODB::FieldPtr m_fldConfirmed;
	ADODB::FieldPtr m_fldReady;	
	ADODB::FieldPtr m_fldNotes;
	ADODB::FieldPtr m_fldMoveUp;
	ADODB::FieldPtr m_fldNoShow;
	ADODB::FieldPtr m_fldNoShowSymbol;
	ADODB::FieldPtr m_fldPatientName;
	ADODB::FieldPtr m_fldBirthDate;
	ADODB::FieldPtr m_fldHomePhone;
	ADODB::FieldPtr m_fldWorkPhone;
	ADODB::FieldPtr m_fldWorkExt;
	ADODB::FieldPtr m_fldMobilePhone;
	ADODB::FieldPtr m_fldPreferredContact;
	ADODB::FieldPtr m_fldLocationName;
	ADODB::FieldPtr m_fldColor;
	ADODB::FieldPtr m_fldAptType;
	ADODB::FieldPtr m_fldAptPurpose;
	ADODB::FieldPtr m_fldPatBal;
	ADODB::FieldPtr m_fldInsBal;
	ADODB::FieldPtr m_fldRefSource;
	ADODB::FieldPtr m_fldResourceString;
	ADODB::FieldPtr m_fldEmail;
	ADODB::FieldPtr m_fldPager;
	ADODB::FieldPtr m_fldOtherPhone;
	ADODB::FieldPtr m_fldCustom1;
	ADODB::FieldPtr m_fldCustom2;
	ADODB::FieldPtr m_fldCustom3;
	ADODB::FieldPtr m_fldCustom4;
	ADODB::FieldPtr m_fldInputName;
	ADODB::FieldPtr m_fldLMDate;
	ADODB::FieldPtr m_fldModifiedLogin;
	ADODB::FieldPtr m_fldModifiedDate;	
	ADODB::FieldPtr m_fldStatusColor;
	// (j.jones 2014-12-04 13:17) - PLID 64119 - added MixRuleColor
	ADODB::FieldPtr m_fldMixRuleColor;
	ADODB::FieldPtr m_fldAptTypeID;
	ADODB::FieldPtr m_fldAptResourceIDString;
	ADODB::FieldPtr m_fldAptPurposeIDString;
	// (d.moore 2007-06-05 15:53) - PLID 13550 - Adding insurance fields
	ADODB::FieldPtr m_fldPrimaryInsurance;
	ADODB::FieldPtr m_fldSecondaryInsurance;
	ADODB::FieldPtr m_fldPrimaryCoPay;
	ADODB::FieldPtr m_fldPrimaryCoPayPercent;
	// (j.gruber 2010-08-03 14:30) - PLID 39948 - no more type
	//ADODB::FieldPtr m_fldPrimaryCoPayType;
	ADODB::FieldPtr m_fldSecondaryCoPay;
	ADODB::FieldPtr m_fldSecondaryCoPayPercent;
	// (j.gruber 2010-08-03 14:30) - PLID 39948 - no more type
	//ADODB::FieldPtr m_fldSecondaryCoPayType;
	// (c.haag 2008-03-20 10:24) - PLID 29328 - Adding inventory fields
	ADODB::FieldPtr m_fldHasOpenOrder;
	ADODB::FieldPtr m_fldHasOpenAllocation;
	//DRT 9/10/2008 - PLID 31127
	ADODB::FieldPtr m_fldCheckInTime;
	ADODB::FieldPtr m_fldCheckOutTime;
	//TES 11/12/2008 - PLID 11465 - Added Input Date
	ADODB::FieldPtr m_fldInputDate;
	//TES 11/12/2008 - PLID 12057 - Added Referring Physician Info
	ADODB::FieldPtr m_fldRefPhysName;
	ADODB::FieldPtr m_fldRefPhysWorkPhone;
	ADODB::FieldPtr m_fldRefPhysFax;
	// (c.haag 2008-11-20 16:13) - PLID 31128 - Insurance referral summary
	ADODB::FieldPtr m_fldInsuranceReferralSummary;
	ADODB::FieldPtr m_fldConfirmedBy; // (j.armen 2011-07-05 13:15) - PLID 44205
	ADODB::FieldPtr m_fldArrivalTime; // (z.manning 2011-07-01 16:20) - PLID 23237
	ADODB::FieldPtr m_fldCancelledDate; // (j.luckoski 2012-05-09 14:07) - PLID 11597
	ADODB::FieldPtr m_fldApptPriInsCo; // (j.gruber 2012-08-06 13:14) - PLID 51926
	ADODB::FieldPtr m_fldApptSecInsCo; // (j.gruber 2012-08-06 13:14) - PLID 51926
	ADODB::FieldPtr m_fldApptInsCoList; // (j.gruber 2012-08-06 13:14) - PLID 51926
	ADODB::FieldPtr m_fldApptRefPhys; // (j.gruber 2013-01-08 15:16) - PLID 54497
	ADODB::FieldPtr m_fldNxWebCode;// (s.tullis 2014-01-21 15:41) - PLID 49748 - Add nexweb activation code to the  extra info fields
	ADODB::FieldPtr m_fldNxWebCodeDate;// (s.tullis 2014-01-24 15:18) - PLID 60470 - Add nexweb experation date to the  extra info fields
    ADODB::FieldPtr m_fldNxWebCodeExpDate; // (s.tullis 2014-01-24 15:18) - PLID 60470 - Add nexweb experation date to the  extra info fields
	ADODB::FieldPtr m_fldIsMixRuleOverridden; // (j.jones 2014-12-03 11:14) - PLID 64274 - added m_fldIsMixRuleOverridden
protected: // Fields
	long			m_ID;
	long			m_ItemRelevence;
	long			m_PersonID;
	long			m_UserDefinedID;
	long			m_PurposeID;
	long			m_ItemID;
	COleDateTime	m_Date;
	COleDateTime	m_StartTime;
	COleDateTime	m_EndTime;
	BOOL			m_Confirmed;
	BOOL			m_Ready;	
	CString			m_Notes;
	BOOL			m_MoveUp;
	long			m_NoShow;
	CString			m_NoShowSymbol;
	long			m_RecordID;
	//long			m_Status; // (a.walling 2013-01-21 16:48) - PLID 54742 - Removed dead code 
	CString			m_PatientName;
	long			m_SetID;
	COleDateTime	m_BirthDate;
	CString			m_HomePhone;
	CString			m_WorkPhone;
	CString			m_WorkExt;
	CString			m_MobilePhone;
	long			m_PreferredContact;
	CString			m_LocationName;
	long			m_Color;
	COleDateTime	m_InputDate;
	CString			m_strAptType;
	CString			m_strAptPurpose;
	long			m_ResourceID;
	CString			m_strPatBal;
	CString			m_strInsBal;
	CString			m_strRefSource;
	CString			m_strResourceString;
	CString			m_strEmail;
	CString			m_strPager;
	CString			m_strOtherPhone;
	CString			m_strCustom1;
	CString			m_strCustom2;
	CString			m_strCustom3;
	CString			m_strCustom4;
	CString			m_strInputName;
	COleDateTime	m_dtLM; // Last time they left a message
	CString			m_strModifiedLogin;
	COleDateTime	m_dtModified;		
	long			m_StatusColor;
	// (j.jones 2014-12-04 13:17) - PLID 64119 - added MixRuleColor
	long			m_nMixRuleColor;
	long			m_nAptTypeID;
	long			m_nCancelColor;
	CString			m_strResourceIDs;
	CString			m_strPurposeIDs;
	// (d.moore 2007-06-05 15:55) - PLID 13550 - Adding insurance fields
	CString			m_strPrimaryInsurance;
	CString			m_strSecondaryInsurance;
	CString			m_strPrimaryCoPay;
	CString			m_strSecondaryCoPay;
	// (c.haag 2008-03-20 10:24) - PLID 29328 - Adding inventory fields
	BOOL			m_bHasOpenOrder;
	BOOL			m_bHasOpenAllocation;
	//DRT 9/10/2008 - PLID 31127
	CString			m_strCheckInTime;
	CString			m_strCheckOutTime;
	//TES 11/12/2008 - PLID 11465 - Added Input Date
	COleDateTime	m_dtInput;
	//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	CString			m_strRefPhysName;
	CString			m_strRefPhysWorkPhone;
	CString			m_strRefPhysFax;
	// (c.haag 2008-11-20 16:13) - PLID 31128 - Insurance referral summary
	CString			m_strInsuranceReferralSummary;
	CString			m_strConfirmedBy; // (j.armen 2011-07-05 13:14) - PLID 44205
	COleDateTime	m_dtArrivalTime; // (z.manning 2011-07-01 16:20) - PLID 23273
	COleDateTime	m_dtCancelledDate; // (j.luckoski 2012-05-09 14:00) - PLID 11597
	CString			m_strApptPriIns; // (j.gruber 2012-08-06 13:14) - PLID 51926
	CString			m_strApptSecIns; // (j.gruber 2012-08-06 13:14) - PLID 51926
	CString			m_strApptInsList; // (j.gruber 2012-08-06 13:14) - PLID 51926
	CString			m_strApptRefPhysName; // (j.gruber 2013-01-08 15:17) - PLID 54497
	CString			m_strNxWebCode; // (s.tullis 2014-01-21 15:41) - PLID 49748 - Add nexweb activation code to the  extra info fields
	CString			m_strNxWebCodeDate; // (s.tullis 2014-01-24 15:19) - PLID 60470 - Add nexweb experation date to the  extra info fields
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added GetIsMixRuleOverridden
	bool			m_bIsMixRuleOverridden;
	//TES 1/14/2010 - PLID 36762 - Track which patients are members of which groups.
	CMap<long, long, CArray<long,long>*, CArray<long,long>*> m_mapPatientsToSecurityGroups;
	//TES 1/14/2010 - PLID 36762 - Cleans up memory allocated to m_mapPatientsToSecurityGroups.
	void DestroySecurityGroupMap();

	bool IsPatientBlocked();
};

// (a.walling 2013-06-18 11:25) - PLID 57199 - Create ResExtendedQ alternative using modular SQL with Common Table Expression so a minimal query with only the data needed can be used dynamically.
namespace Nx {
namespace Scheduler {
	// (j.jones 2014-12-05 13:49) - PLID 64274 - added date ranges and converted to CSqlFragment
	CSqlFragment GetResExtendedBase(CSchedulerView* pSchedView, const COleDateTime dtStartDate, const COleDateTime dtEndDate);
}
}

#endif // !defined(AFX_RESERVATIONREADSET_H__949D53E3_0303_11D3_9447_00C04F4C8415__INCLUDED_)
