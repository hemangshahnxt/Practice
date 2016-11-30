#include "stdafx.h"
#include "practice.h"
#include "BillingRc.h"
#include "mainfrm.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "FinancialApply.h"
#include "NxErrorDialog.h"
#include "HCFADlg.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "PaymentDlg.h"
#include "nxsecurity.h"
#include "ShiftInsRespsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "PhaseTracking.h"
#include "AnesthesiaTimePromptDlg.h"
#include "FinanceChargesDlg.h"
#include "Rewards.h"
#include "invutils.h"
#include "EligibilityRequestDetailDlg.h"
#include "OHIPUtils.h"
#include "MiscSystemUtils.h"	// (j.dinatale 2011-05-24 12:27) - PLID 44810 - need the UUID utils
#include "FinancialCorrection.h"
#include "SingleSelectMultiColumnDlg.h"
#include "AlbertaHLINKUtils.h"
#include "GCEntryDlg.h"
#include "NxPracticeSharedLib\ICCPUtils.h"
#include "NxAPIUtils.h"
#include <boost/container/flat_set.hpp>
#include "NxAPI.h"
#include "PaymentProfileDlg.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "CreditCardReceiptPromptDlg.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <NxSystemUtilitiesLib/NxGZip.h>
#include <NxDataUtilitiesLib/NxStream.h>
#include "EEligibility.h"

//#include <afxdao.h>

//extern CDaoDatabase g_dbPractice;

using namespace ADODB;
using namespace NexTech_Accessor;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//#define NEW_HCFA

//TS 8/23/2002: Obviously, you will notice that the way the dates are compared will never work, 
//because the InputDate includes the time.  This ought to be changed; however, it is conceivable that
//there are offices out there who rely upon the fact that certain employees can _never_ edit an existing
//payment, and we don't want to change that without giving them the option of having it the way it was before.
//This will be done in the context of generally changing the permission structure.

//JJ 7/10/2000 - Used in UserPermissions. If a user can create a bill/payment, and cannot edit it,
//they are still permitted to edit it IF they created it AND they created it the same day.
//Passed in variables: str - "Bill" or "Pay/Ref/Adj", what they are trying to edit
//ID - the BillID or LineItemID
// (c.haag 2009-03-10 12:33) - PLID 33431 - Use CString, not LPCTSTR.
BOOL CanEdit(const CString& str, long ID) {

	CString CurrUser;
	_RecordsetPtr rs(__uuidof(Recordset));
	_variant_t varName, varDate;
	COleDateTime dtDateTime = COleDateTime::GetCurrentTime(), CurrDate, CreateDate;
	CurrDate.SetDate(dtDateTime.GetYear(), dtDateTime.GetMonth(), dtDateTime.GetDay());

	// (j.jones 2007-04-19 11:22) - PLID 25721 - if -1, return FALSE
	// (but this is probably impossible, so ASSERT)
	if(ID == -1) {
		ASSERT(FALSE);
		return FALSE;
	}
	
	try{

	if (!GetRemotePropertyInt("BillingAllowSameDayEdits",0,0,"<None>",TRUE))
		return FALSE;

	//if they are editing a bill
	if(str == "Bill" || str == "Quote") {
		// (c.haag 2009-03-11 14:29) - PLID 32433 - Parameterized
		rs = CreateParamRecordset("SELECT InputName, InputDate FROM BillsT WHERE ID = {INT}",ID);
		if(rs->eof) return false;
		varName = rs->Fields->Item["InputName"]->Value; 
		varDate = rs->Fields->Item["InputDate"]->Value;
		if(varName.vt == VT_NULL || varName.vt == VT_EMPTY || varDate.vt == VT_NULL || varDate.vt == VT_EMPTY) return false;
		CurrUser.Format("%d", VarLong(varName));
		CreateDate = varDate.date;
		CreateDate.SetDateTime(CreateDate.GetYear(), CreateDate.GetMonth(), CreateDate.GetDay(), 0,0,0);
		if((atoi(CurrUser) == GetCurrentUserID()) && (CurrDate == CreateDate)) {
			rs->Close();
			return TRUE;
		}
	}
	//if they are editing a pay/ref/adj, or a charge
	// (j.jones 2011-07-20 17:19) - PLID 44648 - if called on a charge, we should use the same code
	// as a pay/adj/ref, as it is still a LineItemT check
	else if(str == "Charge" || str == "Payment" || str == "Adjustment" || str == "Refund") {
		// (c.haag 2009-03-11 14:30) - PLID 32433 - Parameterized
		rs = CreateParamRecordset("SELECT InputName, InputDate, Type FROM LineItemT WHERE ID = {INT}",ID);
		if(rs->eof) return false;
		varName = rs->Fields->Item["InputName"]->Value; 
		varDate = rs->Fields->Item["InputDate"]->Value;
		if(varName.vt == VT_NULL || varName.vt == VT_EMPTY || varDate.vt == VT_NULL || varDate.vt == VT_EMPTY) return false;
		CurrUser = VarString(varName);
		CreateDate = varDate.date;
		CreateDate.SetDateTime(CreateDate.GetYear(), CreateDate.GetMonth(), CreateDate.GetDay(), 0,0,0);
		if((CurrUser == GetCurrentUserName()) && (CurrDate == CreateDate)) {
			rs->Close();
			return TRUE;
		}
	}
	// (j.jones 2008-04-30 14:20) - PLID 28358 - added support for batch payments
	// (c.haag 2009-03-10 10:56) - PLID 32433 - Also support "BatchAdjustment" and "BatchRefund"
	else if(str == "BatchPayment" || str == "BatchAdjustment" || str == "BatchRefund") {
		_RecordsetPtr rs = CreateParamRecordset("SELECT UserID, InputDate FROM BatchPaymentsT WHERE ID = {INT}", ID);
		if(rs->eof) {
			return FALSE;
		}
		long nUserID = AdoFldLong(rs, "UserID", -1);
		CreateDate = AdoFldDateTime(rs, "InputDate");

		if(nUserID == -1) {
			return FALSE;
		}

		CreateDate.SetDateTime(CreateDate.GetYear(), CreateDate.GetMonth(), CreateDate.GetDay(), 0,0,0);
		if(nUserID == GetCurrentUserID() && CurrDate == CreateDate) {
			rs->Close();
			return TRUE;
		}
	}
	// (j.jones 2011-03-14 15:13) - PLID 42799 - supported Applies
	else if(str == "Apply") {
		rs = CreateParamRecordset("SELECT InputName, InputDate FROM AppliesT WHERE ID = {INT}",ID);
		if(rs->eof) {
			return FALSE;
		}
		varName = rs->Fields->Item["InputName"]->Value; 
		varDate = rs->Fields->Item["InputDate"]->Value;
		if(varName.vt == VT_NULL || varName.vt == VT_EMPTY || varDate.vt == VT_NULL || varDate.vt == VT_EMPTY) {
			return FALSE;
		}
		CurrUser = VarString(varName);
		CreateDate = varDate.date;
		CreateDate.SetDateTime(CreateDate.GetYear(), CreateDate.GetMonth(), CreateDate.GetDay(), 0,0,0);
		if((CurrUser == GetCurrentUserName()) && (CurrDate == CreateDate)) {
			rs->Close();
			return TRUE;
		}
	}
	// (j.jones 2013-08-01 15:09) - PLID 53317 - finance charge batches cannot be automagically
	// edited the same day they were created, you need explicit permissions
	else if(str == "FinanceChargeHistory") {
		return FALSE;
	}
	// (r.gonet 07/10/2014) - PLID 62556 - Chargebacks cannot be edited. 
	else if (str == "Chargeback") {
		return FALSE;
	}
	// (c.haag 2009-03-10 11:13) - PLID 32433 - Safety check (if we get here, we've spotted a legacy bug)
	else {
		ASSERT(FALSE);
	}

	if(rs->State != adStateClosed) rs->Close();
	}NxCatchAll("Error in GlobalFinancialUtils::CanEdit()");
	return FALSE;
}

// (c.haag 2009-03-10 11:26) - PLID 32433 - Returns a message-friendly description of a type
CString CanChangeHistoricFinancial_FormatType(CString str)
{
	if (str == "BatchPayment") {
		str = "batch payment";
	} else if (str == "BatchAdjustment") {
		str = "batch adjustment";
	} else if (str == "BatchRefund") {
		str = "batch refund";
	// (j.jones 2013-08-01 15:09) - PLID 53317 - supported finance charge batches
	} else if (str == "FinanceChargeHistory") {
		str = "finance charge batch";
	} else {
		str.MakeLower();
	}
	return str;
}


// (c.haag 2009-03-25 12:27) - PLID 32433 - Returns TRUE if a user can override the BillingAllowHistoricEdit
// preference given their user permissions.
// (j.jones 2011-01-21 09:26) - PLID 42156 - added bSilent and a string for the billing dlg Edit button to use
BOOL CanChangeHistoricFinancial_CanOverridePreference(const CString& strType, ESecurityPermissionType permtype,
													  BOOL bSilent = FALSE, CString *pstrBillEditWarning = NULL)
{
	if (permtype != sptWrite && permtype != sptDelete) {
		ASSERT(FALSE); // We should never get here; no other permission types apply
		return FALSE;
	}

	EBuiltInObjectIDs bioOverride = bioInvalidID;
	if ("Bill" == strType || "Charge" == strType) {
		bioOverride = bioBillBackdate;
	} else if ("Payment" == strType || "BatchPayment" == strType) {
		bioOverride = bioPaymentBackdate;
	} else if ("Adjustment" == strType || "BatchAdjustment" == strType) {
		bioOverride = bioAdjBackdate;
	} else if ("Refund" == strType || "BatchRefund" == strType) {
		bioOverride = bioRefundBackdate;
	// (j.jones 2011-01-19 14:37) - PLID 42149 - if this is called for an apply, there is no
	// service date, and therefore no backdating permission
	} else if ("Apply" == strType) {
		return FALSE;
	// (j.jones 2013-08-01 15:09) - PLID 53317 - finance charge batches also do not have backdating abilities
	} else if("FinanceChargeHistory" == strType) {
		return FALSE;
	}
	// (r.gonet 07/10/2014) - PLID 62556 - Chargebacks can bypass financial closes since they are zero sum on AR
	else if ("Chargeback" == strType) {
		return TRUE;
	} else {
		// We don't support overriding this kind of security object.
	}
	if (bioInvalidID != bioOverride) {
		if (!(GetCurrentUserPermissions(bioOverride) & (permtype))) {
			// If we get here, the user doesn't have override permission. Check for
			// override with password.
			if ((GetCurrentUserPermissions(bioOverride) & (permtype << 1))) { // With password
				// This will make them enter a password.
				// (j.jones 2011-01-21 11:09) - PLID 42156 - don't prompt for a password if bSilent is true
				if(bSilent || CheckCurrentUserPermissions(bioOverride, permtype)) {
					return TRUE; // User has permission to override the preference.
				} else {
					// No permission to override the preference.
				}
			} else {
				// No permission to override the preference.
			}
		} else {				
			return TRUE; // User has permission to override the preference.
		}
	} else {
		// The change is for a kind of line item that is not associated with any overrideable
		// permission. This should not be tripped at the time of this item being implemented;
		// but in the future, if it gets tripped, you need to add support for backdating permissions.
		ASSERT(FALSE);
	}
	return FALSE;
}

// (j.jones 2011-01-19 12:06) - PLID 42149 - I moved the ECanOverrideHardClose enum to the header file

// (j.jones 2010-12-27 16:07) - PLID 41852 - checks the user's permission to edit a line item past the hard close
ECanOverrideHardClose CanChangeHistoricFinancial_CanOverrideHardClose(const CString& strType, ESecurityPermissionType permtype)
{
	if (permtype != sptWrite && permtype != sptDelete) {
		ASSERT(FALSE); // We should never get here; no other permission types apply
		return cohcNo;
	}

	EBuiltInObjectIDs bioOverride = bioInvalidID;
	if("Bill" == strType || "Charge" == strType) {
		bioOverride = bioBillEditPastClose;
	}
	else if("Payment" == strType || "BatchPayment" == strType) {
		bioOverride = bioPaymentEditPastClose;
	}
	else if("Adjustment" == strType || "BatchAdjustment" == strType) {
		bioOverride = bioAdjEditPastClose;
	}
	else if("Refund" == strType || "BatchRefund" == strType) {
		bioOverride = bioRefundEditPastClose;
	}
	// (j.jones 2011-01-19 14:36) - PLID 42149 - supported applies
	else if("Apply" == strType) {
		bioOverride = bioApplyDeletePastClose;
	}
	// (j.jones 2013-08-01 15:09) - PLID 53317 - supported finance charge batches
	else if("FinanceChargeHistory" == strType) {
		bioOverride = bioUndoFinanceChargesPastClose;
	}
	// (r.gonet 07/25/2014) - PLID 62556 - Allow them to undo chargebacks since they are zero sum on AR.
	else if ("Chargeback" == strType) {
		return cohcYes;
	}
	else {
		// We don't support overriding this kind of security object.
	}

	if(bioInvalidID != bioOverride) {
		if(!(GetCurrentUserPermissions(bioOverride) & (permtype))) {
			// If we get here, the user doesn't have override permission. Check for
			// override with password.
			if((GetCurrentUserPermissions(bioOverride) & (permtype << 1))) { // With password
				//they will need a password
				return cohcWithPass;
			}
			else {
				// No permission to override the preference.
			}
		}
		else {				
			return cohcYes; // User has permission to override the preference.
		}
	} else {
		// The change is for a kind of line item that is not associated with any overrideable
		// permission. This should not be tripped at the time of this item being implemented;
		// but in the future, if it gets tripped, you need to add support for overriding the hard close.
		ASSERT(FALSE);
	}
	return cohcNo;
}

// (c.haag 2009-03-10 09:11) - PLID 32433 - This function returns TRUE if a user is allowed to
// change a financial date to a certain day. This function applies to:
//
//	- Bill dates
//	- Charges dates
//	- Payment, Adjustment, Refund dates
//	- Batch Payment dates
//
// The logic is:
//
// 1 - Is the date in the "Date" field in the past?
//      - No: Change the item
// 2    - Yes: Is the user an administrator
//         - Yes - Change the item
// 3       - No - Is the preference off, or, does the user override it?
//             - Yes - Change the item
// 4           - No - Only allow the change if the date meets the date range criteria
//
// Parameters:
//		strType: The type of item being tested in readable text ("Bill", "Payment"...)
//		dtServiceDate: The date of the item being tested (usually the service date, but it can be a bill date)
//		bWarnByFormField: TRUE if the user is in a modal form clicking on "Save", or doing something closely equivalent.
//			FALSE if they're trying to just delete a bill from a list of bills. This influences the fail message appearance.
//		permtype: The desired type of access. Standard permissions should have already been checked by now; this is only used
//			to see if we can override the preference.
//
// (d.thompson 2009-09-02) - PLID 34694 - Added suppression of messages
// (j.jones 2011-01-21 09:26) - PLID 42156 - renamed the message suppression to bSilent, because it needs to also suppress password prompts,
// and also added a string for the billing dlg Edit button to use
BOOL CanChangeHistoricFinancial_ByServiceDate(const CString& strType, COleDateTime dtServiceDate, BOOL bWarnByFormField, ESecurityPermissionType permtype /* = sptWrite */,
											  BOOL bSilent /* = FALSE*/, CString *pstrBillEditWarning /*= NULL*/)
{
	// Do all comparisons on this machine's date, not the server date, because that's how CanEdit does it
	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	dtToday.SetDate( dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay() );
	dtServiceDate.SetDate( dtServiceDate.GetYear(), dtServiceDate.GetMonth(), dtServiceDate.GetDay() );

	// (j.jones 2011-01-21 13:51) - PLID 42156 - I concluded that since attempting to change a backdated bill
	// does not currently warn you in any way (either you can or you can't, you aren't told about it),
	// there's really nothing we can do to pstrBillEditWarning. I'm keeping the parameter in here for if we
	// ever change the logic in the future, but for now, it is intentionally set to empty, to not warn.
	if(bSilent && pstrBillEditWarning != NULL) {
		*pstrBillEditWarning = "";
	}

	// Always return success if the service date is on or after today (1)
	if (dtServiceDate >= dtToday) {
		return TRUE;
	}

	// (c.haag 2009-03-10 15:46) - PLID 32433 - Return success if the user is an administrator (2)
	if (IsCurrentUserAdministrator()) {
		return TRUE;
	}

	// See how we're comparing this (3)
	int nPreferenceMethod = GetRemotePropertyInt("BillingAllowHistoricEdit",1,0,"<None>",TRUE);
	if (1 == nPreferenceMethod) {
		// (c.haag 2009-03-10 15:46) - PLID 32433 - If the method is 1, then we always allow historic edits and backdating (3)
		return TRUE;
	}
	// (c.haag 2009-03-11 10:04) - PLID 32433 - When we get this far, we know the preference can definitely
	// influence the result of this function. Check to see if the user can override the preference. (3)
	if (permtype != (ESecurityPermissionType)0) {
		if (CanChangeHistoricFinancial_CanOverridePreference(strType, permtype, bSilent, pstrBillEditWarning)) {
			return TRUE;
		}
	} else {
		// If we get here, we already checked the override permissions
	}

	if (2 == nPreferenceMethod) { // (4)
		// Let the user edit the item if the service date is no more than X days in the past
		long nDaysAgo = max(0,GetRemotePropertyInt("BillingDisallowHistoric_DaysAgo",30,0,"<None>",TRUE));
		COleDateTimeSpan dts = dtToday - dtServiceDate;
		long nDayDiff = dts.GetDays();
		if (nDayDiff <= nDaysAgo) {
			return TRUE; // The service date is not more than X days in the past
		} else {
			// (d.thompson 2009-09-02) - PLID 34694 - Added option to suppress the messages
			if(!bSilent) {
				// (c.haag 2009-03-10 15:49) - PLID 32433 - Format the message based on whether we're in a modal dialog form
				if (bWarnByFormField) {
					if (0 == nDaysAgo) {
						MsgBox(MB_OK | MB_ICONERROR, "The %s date of '%s' can not be saved because it is dated before today.", CanChangeHistoricFinancial_FormatType(strType), FormatDateTimeForInterface(dtServiceDate, 0, dtoDate, false), nDaysAgo);
					} else {
						MsgBox(MB_OK | MB_ICONERROR, "The %s date of '%s' can not be saved because it is more than %d day(s) in the past.", CanChangeHistoricFinancial_FormatType(strType), FormatDateTimeForInterface(dtServiceDate, 0, dtoDate, false), nDaysAgo);
					}
				} else {
					if (0 == nDaysAgo) {
						MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify the %s because it is dated before today.", CanChangeHistoricFinancial_FormatType(strType), nDaysAgo);
					} else {
						MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify the %s because its date is more than %d day(s) in the past.", CanChangeHistoricFinancial_FormatType(strType), nDaysAgo);
					}
				}
			}
			return FALSE; // The service date is too old; so the user cannot edit the item
		}
	}
	else if (3 == nPreferenceMethod) { // (4)
		// Let the user edit the item if today's date is not more than X days after the item's month's end
		if (dtServiceDate.GetMonth() == dtToday.GetMonth() && dtServiceDate.GetYear() == dtToday.GetYear()) {
			// Service date is this month, always return success
			return TRUE;
		}

		// If we get here, the service date was some time before this month. Now we need to see if it's in
		// last month.
		COleDateTime dtLastMonth = dtToday;
		while (dtLastMonth.GetMonth() == dtToday.GetMonth()) {
			dtLastMonth += COleDateTimeSpan(-1,0,0,0);
		}
		if (dtServiceDate.GetMonth() == dtLastMonth.GetMonth() && dtServiceDate.GetYear() == dtLastMonth.GetYear()) {
			// If we get here, the service date is definitely last month. So, lets do the comparison.
			long nSinceLastMonth = min(31,max(0,GetRemotePropertyInt("BillingDisallowHistoric_SinceLastMonth",10,0,"<None>",TRUE)));
			COleDateTime dtFirstOfThisMonth;
			dtFirstOfThisMonth.SetDate(dtToday.GetYear(), dtToday.GetMonth(), 1);
			COleDateTime dtDeadline = dtFirstOfThisMonth + COleDateTimeSpan(nSinceLastMonth-1,0,0,0);
			if (dtToday <= dtDeadline) {
				// If we get here, the user is allowed to edit the item
				return TRUE;				
			}
			else {
				// If we get here, we missed the deadline, so the user cannot edit the item
				// (d.thompson 2009-09-02) - PLID 34694 - Added option to suppress the messages
				if(!bSilent) {
					if (nSinceLastMonth > 0) {
						// (c.haag 2009-03-10 15:49) - PLID 32433 - Format the message based on whether we're in a modal dialog form
						if (bWarnByFormField) {
							MsgBox(MB_OK | MB_ICONERROR, "The %s date of '%s' can not be saved because today is more than %d day(s) after the month of the date.", CanChangeHistoricFinancial_FormatType(strType), FormatDateTimeForInterface(dtServiceDate, 0, dtoDate, false), nSinceLastMonth);
						} else {
							MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify the %s because today is more than %d day(s) after the month of the %s.", CanChangeHistoricFinancial_FormatType(strType), nSinceLastMonth, CanChangeHistoricFinancial_FormatType(strType));
						}
					} else {
						// (c.haag 2009-03-10 15:49) - PLID 32433 - Format the message based on whether we're in a modal dialog form
						if (bWarnByFormField) {
							MsgBox(MB_OK | MB_ICONERROR, "The %s date of '%s' can not be saved because it occurs before this month.", CanChangeHistoricFinancial_FormatType(strType), FormatDateTimeForInterface(dtServiceDate, 0, dtoDate, false));
						} else {
							MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify the %s because it is dated before this month.", CanChangeHistoricFinancial_FormatType(strType));
						}
					}
				}
				return FALSE;
			}
		} else {
			// If we get here, the service date is before last month, so always return failure.
			// (c.haag 2009-03-10 15:49) - PLID 32433 - Format the message based on whether just the date field is being changed
			// (d.thompson 2009-09-02) - PLID 34694 - Added option to suppress the messages
			if(!bSilent) {
				if (bWarnByFormField) {
					MsgBox(MB_OK | MB_ICONERROR, "The %s date of '%s' can not be saved because it occurs before last month.", CanChangeHistoricFinancial_FormatType(strType), FormatDateTimeForInterface(dtServiceDate, 0, dtoDate, false));
				} else {
					MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify the %s because it is dated before last month.", CanChangeHistoricFinancial_FormatType(strType));
				}
			}
			return FALSE;
		}
	}
	else {
		// Invalid preference value
		ASSERT(FALSE);
		return FALSE;
	}
}

// (j.jones 2010-12-28 10:34) - PLID 41852 - used to enforce the ability to edit an input date against a hard close
BOOL CanChangeHistoricFinancial_ByInputDate(const CString& strType, COleDateTime dtInputDate, ESecurityPermissionType permtype /*= sptWrite*/)
{

	// (j.jones 2011-01-20 16:30) - PLID 41999 - this function only checks to see if an input date is prior to a close "as of" date,
	// it does not perform service date checking

	// (j.jones 2011-01-06 09:29) - PLID 42000 - do not automatically skip this if
	// they are an administrator user, because we still need to warn if the item is closed

	// (j.jones 2010-12-20 17:11) - PLID 41852 - check our last date/time of a hard close
	_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseDate FROM FinancialCloseHistoryT");
	if(!rsHardClose->eof) {
		//this is nullable since this is a Max()
		COleDateTime dtHardClose = VarDateTime(rsHardClose->Fields->Item["HardCloseDate"]->Value, g_cdtInvalid);
		// (j.jones 2011-01-06 08:33) - PLID 41998 - ensure the compare is <= to, not <
		if(dtHardClose.GetStatus() != COleDateTime::invalid && dtInputDate <= dtHardClose) {
			//they have closed the data in the past, and this input date is on or before that close date
			
			//can they bypass a hard close?
			ECanOverrideHardClose eCanOverride = CanChangeHistoricFinancial_CanOverrideHardClose(strType, permtype);

			CString strFormatTypeDesc = CanChangeHistoricFinancial_FormatType(strType);

			//if they can't bypass it, warn and quit
			if(eCanOverride == cohcNo) {
				MsgBox(MB_OK | MB_ICONERROR, "The %s input date of '%s' can not be saved because it occurs before the most recent closing date of %s.", strFormatTypeDesc, FormatDateTimeForInterface(dtInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
				return FALSE;
			}

			// (j.jones 2011-01-06 09:35) - PLID 42000 - if they can bypass it, warn first
			CString strWarn;	
			strWarn.Format("The %s input date of '%s' occurs before the most recent closing date of %s.\n\n"
				"Are you absolutely certain you wish to save this date?", strFormatTypeDesc, FormatDateTimeForInterface(dtInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
			if(IDNO == MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}

			//check the password if needed
			if(eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
				//fail, but clarify why we failed
				MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to save this input date.");
				return FALSE;
			}
		}
	}
	rsHardClose->Close();

	//if we got here, this input date is fine
	return TRUE;
}

// (c.haag 2009-03-10 09:11) - PLID 32433 - This function returns TRUE if a user is allowed to
// edit a bill, charge, payment, adjustment, refund or batch payment given that item's service
// date, the practice's preferences, and user permissions. This function applies to:
//
//	- Bill dates
//	- Charges dates
//	- Payment, Adjustment, Refund dates
//	- Batch Payment dates
//
// This function is called upon opening a line item, or attempting an instant action on a line
// item, just as deleting it. This is not called for new items. Here is a breakdown of the logic:
//
// 1 - Is the user an administrator?
//     - Yes: Let them open the item for editing no matter what the service date is
// 2   - No: Does the preference apply given the inputs?
//          - No: Just check standard permissions and return a success code
// 3        - Yes: Did the user create the item today?
//             - Yes: Let them open for editing
// 4           - No: Do they satisfy standard permissions?
//                - No: Fail
// 5              - Yes: Do further validation based on the date, and return a success code
//
// (d.thompson 2009-09-02) - PLID 34694 - Added suppression of messages
// (j.jones 2011-01-21 09:26) - PLID 42156 - renamed the message suppression to bSilent, because it needs to also suppress password prompts,
// and also added a string for the billing dlg Edit button to use/
// (r.gonet 07/07/2014) - PLID 62571 - Added an option to ignore financial closes.
BOOL CanChangeHistoricFinancial(const CString& strType, long ID, EBuiltInObjectIDs bio, ESecurityPermissionType perm,
								BOOL bSilent /*= FALSE*/, CString *pstrBillEditWarning /*= NULL*/, BOOL bIgnoreFinancialClose/*= FALSE*/)
{
	try {
		// This should never happen, but check for it
		if (-1 == ID) {
			ASSERT(FALSE);
			return FALSE;
		}

		COleDateTime dtItemInputDate = g_cdtInvalid;
		COleDateTime dtItemServiceDate = g_cdtInvalid;

		CString strSourceType; //only used for applies

		// (j.jones 2011-01-06 09:18) - PLID 42000 - do not return immediately if the
		// user is an administrator, because if it is a closed item, we still need to warn		

		// (j.jones 2010-12-20 17:11) - PLID 41852 - if writing or deleting, check our last date/time of a hard close
		if(perm == sptWrite || perm == sptDelete) {
			if (!bIgnoreFinancialClose) {
				// (j.jones 2011-01-20 13:59) - PLID 41999 - get both the close date and input date of the most recent close
				_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseAsOfDate, "
					"Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT");
				if(!rsHardClose->eof) {
					//this is nullable since this is a Max()
					_variant_t varHardCloseAsOfDate = rsHardClose->Fields->Item["HardCloseAsOfDate"]->Value;
					_variant_t varHardCloseInputDate = rsHardClose->Fields->Item["HardCloseInputDate"]->Value;
					COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
					COleDateTime dtHardCloseInputDate = g_cdtInvalid;
					//impossible for one to be NULL and not the other
					if(varHardCloseAsOfDate.vt == VT_DATE || varHardCloseInputDate.vt == VT_DATE) {
						dtHardCloseAsOfDate = VarDateTime(varHardCloseAsOfDate);					
						dtHardCloseInputDate = VarDateTime(varHardCloseInputDate);
					}
				
					if(dtHardCloseAsOfDate.GetStatus() != COleDateTime::invalid
						|| dtHardCloseInputDate.GetStatus() != COleDateTime::invalid) {
						//they have closed the data in the past
					
						// (j.jones 2011-01-06 09:21) - PLID 42000 - even if you have permission to override,
						// we now always have to open a recordset to check the input date, because the user
						// would still have to be warned first before editing

						// (j.jones 2011-01-06 08:33) - PLID 41998 - ensure the compare is <= to, not <

						// (j.jones 2011-01-20 13:51) - PLID 41999 - Now the compare is even weirder,
						// if the service date is <= the close date and the input date is <= the
						// CLOSE input date, it is also locked. Items with a service date <= the close date
						// but their input date is after the input date of the close are not locked.
						BOOL bIsClosedByInputDate = FALSE;
						BOOL bIsClosedByServiceDate = FALSE;
						if(strType == "Bill") {
							_RecordsetPtr rs = CreateParamRecordset("SELECT Date, InputDate, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} AND Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
								"FROM BillsT "
								"WHERE ID = {INT} "
								"AND (InputDate <= {VT_DATE} "
								" OR (InputDate <= {VT_DATE} AND Date <= {VT_DATE}))",
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate,
								ID,
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate);
							if(!rs->eof) {
								dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
								dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
								bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
								bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
							}
							rs->Close();
						}
					else if (strType == "Charge" || strType == "Payment" || strType == "Adjustment" || strType == "Refund") {
							_RecordsetPtr rs = CreateParamRecordset("SELECT Date, InputDate, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} AND Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
								"FROM LineItemT "
								"WHERE ID = {INT} "
								"AND (InputDate <= {VT_DATE} "
								" OR (InputDate <= {VT_DATE} AND Date <= {VT_DATE}))",
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate,
								ID,
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate);
							if(!rs->eof) {
								dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
								dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
								bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
								bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
							}
							rs->Close();
						}
						else if(strType == "BatchPayment" || strType == "BatchAdjustment" || strType == "BatchRefund") {
							_RecordsetPtr rs = CreateParamRecordset("SELECT Date, InputDate, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
								"Convert(bit, CASE WHEN InputDate <= {VT_DATE} AND Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
								"FROM BatchPaymentsT "
								"WHERE ID = {INT} "
								"AND (InputDate <= {VT_DATE} "
								" OR (InputDate <= {VT_DATE} AND Date <= {VT_DATE}))",
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate,
								ID,
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate);
							if(!rs->eof) {
								dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
								dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
								bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
								bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
							}
							rs->Close();
						}
						// Applies (delete-only) are locked if their source payment is locked.
						else if(strType == "Apply") {
							_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.Date, LineItemT.InputDate, LineItemT.Type, "
								"Convert(bit, CASE WHEN LineItemT.InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
								"Convert(bit, CASE WHEN LineItemT.InputDate <= {VT_DATE} AND LineItemT.Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
								"FROM LineItemT "
								"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
								"WHERE AppliesT.ID = {INT} "
								"AND (LineItemT.InputDate <= {VT_DATE} "
								" OR (LineItemT.InputDate <= {VT_DATE} AND LineItemT.Date <= {VT_DATE}))",
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate,
								ID,
								varHardCloseAsOfDate,
								varHardCloseInputDate, varHardCloseAsOfDate);
							if (!rs->eof) {
								dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
								dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
								bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
								bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
								long nType = VarLong(rs->Fields->Item["Type"]->Value);
								switch (nType) {
								case 1:
									strSourceType = "payment";
									break;
								case 3:
									strSourceType = "refund";
									break;
								case 2:
								case 0: //not valid, but assume adjustment
									strSourceType = "adjustment";
									break;
								default:
									//whatever we say the source type is, it's going to be wrong
									ASSERT(FALSE);
									break;
								}
							}
							rs->Close();
						}
						// (j.jones 2013-08-01 15:09) - PLID 53317 - supported finance charge batches
						else if(strType == "FinanceChargeHistory") {
							_RecordsetPtr rs = CreateParamRecordset("SELECT InputDate FROM FinanceChargeHistoryT "
								"WHERE ID = {INT} AND InputDate <= {VT_DATE}", ID, varHardCloseAsOfDate);
							if(!rs->eof) {
								dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
								bIsClosedByInputDate = TRUE;
								//set service date to true even though it is meaningless for finance charges
								bIsClosedByServiceDate = TRUE;
							}
							rs->Close();
					// (r.gonet 07/25/2014) - PLID 62556 - Supported chargebacks
					} else if (strType == "Chargeback") {
						_RecordsetPtr rs = CreateParamRecordset("SELECT MIN(Date) AS Date, MIN(InputDate) AS InputDate, "
							"Convert(bit, CASE WHEN MIN(InputDate) <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
							"Convert(bit, CASE WHEN MIN(InputDate) <= {VT_DATE} AND MIN(Date) <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
							"FROM LineItemT "
							"INNER JOIN ChargebacksT ON LineItemT.ID = ChargebacksT.PaymentID OR LineItemT.ID = ChargebacksT.AdjustmentID "
							"WHERE ChargebacksT.ID = {INT} "
							"AND (InputDate <= {VT_DATE} "
							" OR (InputDate <= {VT_DATE} AND Date <= {VT_DATE}))",
							varHardCloseAsOfDate,
							varHardCloseInputDate, varHardCloseAsOfDate,
							ID,
							varHardCloseAsOfDate,
							varHardCloseInputDate, varHardCloseAsOfDate);
						if (!rs->eof) {
							_variant_t varItemInputDate = rs->Fields->Item["InputDate"]->Value;
							_variant_t varItemServiceDate = rs->Fields->Item["Date"]->Value;
							if (varItemInputDate.vt == VT_DATE && varItemServiceDate.vt == VT_DATE) {
								dtItemInputDate = VarDateTime(varItemInputDate);
								dtItemServiceDate = VarDateTime(varItemServiceDate);
								bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
								bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
							} else {
								// This chargeback occurred after the last financial close.
							}
						}
						rs->Close();
					}


					//is it closed?
					if(bIsClosedByInputDate || bIsClosedByServiceDate) {

						//can they bypass a hard close?
						ECanOverrideHardClose eCanOverride = CanChangeHistoricFinancial_CanOverrideHardClose(strType, perm);

						CString strFormatTypeDesc = CanChangeHistoricFinancial_FormatType(strType);

						//if they can't bypass it, warn and quit
						if(eCanOverride == cohcNo) {
							// (j.jones 2011-01-21 09:29) - PLID 42156 - suppress this message if bSilent
							if(!bSilent) {
								// (j.jones 2011-01-20 14:12) - PLID 41999 - now we have to explain why it is closed based on input or service date
								if(bIsClosedByInputDate) {
									//the item is closed based on its input date, which is before the "as of" close date
									MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify this %s because its input date of %s is before the most recent closing date of %s.", strFormatTypeDesc, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									//the item is closed because its service date is before the "as of" close date,
									//its input date is before the close input date, so it is still closed anyways
									MsgBox(MB_OK | MB_ICONERROR, "You do not have permission to modify this %s because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.",
										strFormatTypeDesc, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
							}
							// (j.jones 2011-01-21 11:11) - PLID 42156 - if silent, and we need the Bill Edit warning, fill it
							else if(pstrBillEditWarning != NULL) {

								ASSERT(strType == "Bill" || strType == "Charge");

								if(bIsClosedByInputDate) {
									//the item is closed based on its input date, which is before the "as of" close date
									pstrBillEditWarning->Format("This %s is closed because its input date of %s is before the most recent closing date of %s.\n\n"
										"You will not be able to edit the bill date, provider, patient coordinator, location, or any value that changes existing charge totals.", strFormatTypeDesc, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									//the item is closed because its service date is before the "as of" close date,
									//its input date is before the close input date, so it is still closed anyways
									pstrBillEditWarning->Format("This %s is closed because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.\n\n"
										"You will not be able to edit the bill date, provider, patient coordinator, location, or any value that changes existing charge totals.",
										strFormatTypeDesc, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
							}

							return FALSE;
						}

						// (j.jones 2011-01-21 09:29) - PLID 42156 - suppress this prompt if bSilent
						if(!bSilent) {

							// (j.jones 2011-01-06 09:24) - PLID 42000 - if they can bypass it, warn first
							CString strWarn;
							// (j.jones 2011-01-20 14:12) - PLID 41999 - now we have to explain why it is closed based on input or service date
							if(bIsClosedByInputDate) {
								//the item is closed based on its input date, which is before the "as of" close date
								strWarn.Format("This %s has already been closed. Its %sinput date of %s is before the most recent closing date of %s.\n\n"
									"Are you absolutely certain you wish to modify this closed %s?",
									strFormatTypeDesc, strType == "Apply" ? strSourceType + "'s " : "", FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime), strFormatTypeDesc);
							}
							else {
								//the item is closed because its service date is before the "as of" close date,
								//its input date is before the close input date, so it is still closed anyways
								strWarn.Format("This %s has already been closed. Its %sservice date of %s is before the most recent closing date of %s, "
									"and its %sinput date of %s is before the most recent close that was created on %s.\n\n"
									"Are you absolutely certain you wish to modify this closed %s?",
									strFormatTypeDesc, strType == "Apply" ? "payment's " : "", FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
									strType == "Apply" ? strSourceType + "'s " : "",
									FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									strFormatTypeDesc);
							}

							if(IDNO == MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
								return FALSE;
							}

							//check the password if needed
							if(eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
								//fail, but clarify why we failed
								MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to make changes to this %s.", strFormatTypeDesc);
								return FALSE;
							}
						}
						// (j.jones 2011-01-21 11:11) - PLID 42156 - if silent, and we need the Bill Edit warning, fill it
						else if(pstrBillEditWarning != NULL) {

							ASSERT(strType == "Bill" || strType == "Charge");

							if(bIsClosedByInputDate) {
								//the item is closed based on its input date, which is before the "as of" close date
								pstrBillEditWarning->Format("This %s is closed because its input date of %s is before the most recent closing date of %s.\n\n"
									"%s edit the bill date, provider, patient coordinator, location, or any value that changes existing charge totals.",
									strFormatTypeDesc, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
									eCanOverride == cohcWithPass ? "You will need to enter your password to be able to" : "You will be warned if you attempt to");
							}
							else {
								//the item is closed because its service date is before the "as of" close date,
								//its input date is before the close input date, so it is still closed anyways
								pstrBillEditWarning->Format("This %s is closed because its service date of %s is before the most recent closing date of %s, "
									"and its input date of %s is before the most recent close that was created on %s.\n\n"
									"%s edit to edit the bill date, provider, patient coordinator, location, or any value that changes existing charge totals.",
									strFormatTypeDesc, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									eCanOverride == cohcWithPass ? "You will need to enter your password to be able to" : "You will be warned if you attempt to");
							}
						}

						//if it is closed, but you can edit it anyways, that overrules the "historic date edit"
						//logic, you can change it regardless, therefore we can leave this function now

						// (r.gonet 07/25/2014) - PLID 62556 - If it is anything but a chargeback, our criteria for determining whether to allow the change is met. 
						if (strType != "Chargeback") {
							return TRUE;
						} else {
							// With chargebacks though, there is no historic financial permission for those. Chargebacks can be changed even behind a financial close since AR is unaffected.
							// Do further processing to ensure the user has basic permissions though, e.g. can delete a payment.
						}
					}
				}
			}
				rsHardClose->Close();
			} else {
				// Certain things can be changed even if a financial close is in effect... The caller has specified to ignore that a financial close is in effect.
			}
		}

		// (j.jones 2011-01-06 09:19) - PLID 42000 - now we can return right away if the user is an administrator
		if (IsCurrentUserAdministrator()) {
			return TRUE;
		}

		// See where the preference is active or not. If not active, break into special logic that acts
		// like the old way of doing things. If the desired permission is simply read access, meaning
		// the user can't actually change anything, that nullifies the effectiveness of the preference.
		int nPreferenceMethod = GetRemotePropertyInt("BillingAllowHistoricEdit",1,0,"<None>",TRUE);
		// Also see whether they can override the preference via special permissions
		BOOL bCanOverridePreference = (perm == sptWrite || perm == sptDelete) ? CanChangeHistoricFinancial_CanOverridePreference(strType, perm, bSilent, pstrBillEditWarning) : FALSE;

		if (1 == nPreferenceMethod || bCanOverridePreference || sptRead == perm) {

			// (j.jones 2011-01-21 13:28) - PLID 42156 - If the pstrBillEditWarning was given to us,
			// it means that the user already had permission to edit (or CanEdit is true), and we
			// are merely checking the possibility of a hard close or backdating restriction.
			// If we get here, neither apply, so we DON'T need to check the same permission again.
			if(pstrBillEditWarning != NULL) {
				//make sure that what we are checking is what we already checked
				if((strType == "Bill" || strType == "Charge") && bio == bioBill && perm == sptWrite) {
					return TRUE;
				}
				else {
					//somebody screwed up the coding, so ASSERT, and carry on normally,
					//checking the perms a second time
					ASSERT(FALSE);
				}
			}

			// If we get here, it means the preference is moot (which is the default). So, we need to revert
			// to the execution path before this feature was introduced; which was to see if the user had
			// permissions, then see if they can override them, then actually check the permissions.
			if ((GetCurrentUserPermissions(bio) & perm) || 
				(CanEdit(strType, ID)) ||
				(CheckCurrentUserPermissions(bio, perm, FALSE, FALSE, bSilent)))
			{
				// The user meets at least one of the three requirements; so return success
				return TRUE;
			} else {
				// The user meets no requirements, and the billing-allow-historic-edit preference is irrelevant;
				// so return failure
				return FALSE;
			}
		}

		// Return TRUE if the item was created today by the logged in user
		if (CanEdit(strType, ID)) {
			return TRUE;
		}

		// Now test on user permissions
		if (!CheckCurrentUserPermissions(bio, perm, FALSE, FALSE, bSilent)) {
			// No permission, or no valid password

			// (j.jones 2011-01-21 13:28) - PLID 42156 - if the pstrBillEditWarning was given to us,
			// ASSERT, because we should have never had that parameter if they don't have regular edit permission
			ASSERT(pstrBillEditWarning == NULL);

			return FALSE;
		} else {
			// They do have permission
		}

		// (j.jones 2011-01-19 14:37) - PLID 42149 - if this is called for an apply, there is no
		// service date, and therefore no backdating permission
		// (j.jones 2011-03-14 15:22) - PLID 42799 - ...which means we should return TRUE, not FALSE!
		if(strType == "Apply") {
			return TRUE;
		}

		// Pull the service date from data and pass it into our final validation
		// (j.jones 2011-01-20 13:53) - PLID 41999 - don't need to run another recordset if
		// the hard close check earlier already cached the service date
		if(dtItemServiceDate.GetStatus() == COleDateTime::invalid) {

			_RecordsetPtr prs;

			if(strType == "Bill") {
				prs = CreateParamRecordset("SELECT [Date] FROM BillsT WHERE ID = {INT}", ID);
			} else if(strType == "Charge" || strType == "Payment" || strType == "Adjustment" || strType == "Refund") {
				prs = CreateParamRecordset("SELECT [Date] FROM LineItemT WHERE ID = {INT}", ID);
			} else if(strType == "BatchPayment" || strType == "BatchAdjustment" || strType == "BatchRefund") {
				prs = CreateParamRecordset("SELECT [Date] FROM BatchPaymentsT WHERE ID = {INT}", ID);
			// (j.jones 2011-01-19 14:37) - PLID 42149 - if this is called for an apply, there is no
			// service date, and therefore no backdating permission
			// (j.jones 2011-03-14 15:22) - PLID 42799 - ...which means we should return TRUE, not FALSE!
			} else if(strType == "Apply") {
				return TRUE;
			}
			if(!prs->eof) {
				dtItemServiceDate = AdoFldDateTime(prs, "Date");
			}
			prs->Close();
		}
		
		if(dtItemServiceDate.GetStatus() == COleDateTime::invalid) {
			// Item may have been deleted. We can't authorize editing something that doesn't exist.
			if(!bSilent) {
				MsgBox(MB_OK | MB_ICONERROR, "You may not modify the %s. It may have been deleted from the system.", CanChangeHistoricFinancial_FormatType(strType));
			}
			return FALSE;
		}

		// Do the date-based validation (5)
		return CanChangeHistoricFinancial_ByServiceDate(strType, dtItemServiceDate, FALSE /* Accessing entire item; not just the date */, (ESecurityPermissionType)0 /* We already checked the override, so pass in 0 */, bSilent, pstrBillEditWarning);
	}
	NxCatchAll("Error in CanChangeHistoricFinancial");
	return FALSE;
}

/* Round a currency value to two decimal places */
void RoundCurrency(COleCurrency& cy)
{
	CURRENCY c = cy;
	BOOL boNegative = FALSE;
	LONGLONG lost_value;

	LONGLONG int64;

	int64 = c.int64;

	if (int64 < 0) {
		int64 = -int64;
		boNegative = TRUE;
	}

	lost_value = int64 % (LONGLONG)100;
		
	// Round off the value by removing the last 2 digits
	int64 = (int64 / 100) * 100;	

	// If the rounded off value was 50 or more, increase by one cent
	if (lost_value >= 50)
		int64 += 100;

	if (boNegative)
		int64 = -int64;

	c.int64 = int64;
	cy = c;
}

/* Apply what you can, leave the rest unapplied */
// (j.jones 2010-05-17 17:02) - PLID 16503 - added bAlwaysAllowZeroDollarApply
// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
// for when the apply is part of an E-Remittance posting
// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
BOOL ApplyPayToBill(long PayID, long PatientID, COleCurrency PayAmount, CString ClickedType, long BillID, long nInsuredPartyID /* = -1 */,
					BOOL PointsToPayments /* = FALSE */, BOOL bShiftToPatient /* = FALSE */, BOOL bAdjustBalance /* = FALSE */, BOOL bPromptToShift/*= TRUE */, BOOL bWarnLocation /* = TRUE*/,
					BOOL bAlwaysAllowZeroDollarApply /* = FALSE*/, BOOL bDisableAllowableCheck /* = FALSE*/, BOOL bIsERemitPosting/* = FALSE*/, BOOL bWarnLocationPatientResp /* = TRUE*/)
{
	_RecordsetPtr rsBookingInfo, rsBillInfo, rsApplied;
	COleCurrency TempAmount, cy, cyRemainingCharge;
	_variant_t var;
	CString str;
	int nSkippedCharges = 0;
	long nActivePatientID = PatientID;

	// (j.jones 2010-05-17 17:04) - PLID 16503 - we now accept an override for this setting
	BOOL bAllowZeroDollarApply = bAlwaysAllowZeroDollarApply || IsZeroDollarPayment(PayID);

	//Patient apply specific check
	if (PayAmount == COleCurrency(0,0) && !bAllowZeroDollarApply && nInsuredPartyID == -1)
		return FALSE;

	TempAmount = PayAmount;

	/* Open up a table with a list of appliable items */
	if (ClickedType == "Bill") {
		// Get all the charges for a bill
		//(e.lally 2007-03-30) PLID 25263 - dynamically create filter using insured party logic
		// (j.jones 2009-10-06 11:46) - PLID 27713 - moved into the recordset
		/*
		CString strInsuredPartyFilter;
		if(nInsuredPartyID == -1){
			strInsuredPartyFilter = "IS NULL";
		}
		else{
			strInsuredPartyFilter.Format("= %li", nInsuredPartyID);
		}
		*/

		// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
		// (j.jones 2009-10-23 10:29) - PLID 18558 - added POSID
		// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
		rsBillInfo = CreateParamRecordset("SELECT PatientChargesQ.ID, PatientChargesQ.ServiceID, PatientChargesQ.DoctorsProviders, "
			"Convert(money, Sum(CASE WHEN ({INT} = -1 AND InsuredPartyT.PersonID IS Null) OR ({INT} <> -1 AND InsuredPartyT.PersonID = {INT}) THEN ChargeRespT.Amount ELSE 0 End)) AS TotalCharges, "
			"PatientChargesQ.LocationID, PatientChargesQ.POSID "
			"FROM (SELECT LineItemT.*, ChargesT.BillID, BillsT.Location AS POSID, ChargesT.DoctorsProviders, ChargesT.ServiceID "
			"FROM LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE (((LineItemT.PatientID) = {INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type) = 10)) "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			") AS PatientChargesQ INNER JOIN (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID "
			"WHERE (((PatientChargesQ.BillID) = {INT})) "
			"GROUP BY PatientChargesQ.ID, PatientChargesQ.ServiceID, PatientChargesQ.DoctorsProviders, PatientChargesQ.LocationID, PatientChargesQ.POSID",
			nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nActivePatientID, BillID);

	}
	else if (ClickedType == "Charge") {
		// Get a single charge
		//(e.lally 2007-03-30) PLID 25263 - dynamically create filter using insured party logic
		// (j.jones 2009-10-06 11:46) - PLID 27713 - moved into the recordset
		/*
		CString strInsuredPartyFilter;
		if(nInsuredPartyID ==-1){
			strInsuredPartyFilter = "IS NULL";
		}
		else{
			strInsuredPartyFilter.Format("= %li", nInsuredPartyID);
		}
		*/

		// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
		// (j.jones 2009-10-23 10:29) - PLID 18558 - added POSID
		rsBillInfo = CreateParamRecordset("SELECT PatientChargesQ.ID, PatientChargesQ.ServiceID, PatientChargesQ.DoctorsProviders, "
			"Convert(money,Sum(CASE WHEN ({INT} = -1 AND InsuredPartyT.PersonID IS Null) OR ({INT} <> -1 AND InsuredPartyT.PersonID = {INT}) THEN ChargeRespT.Amount ELSE 0 End)) AS TotalCharges, "
			"PatientChargesQ.LocationID, PatientChargesQ.POSID "
			"FROM (SELECT LineItemT.*, ChargesT.BillID, BillsT.Location AS POSID, ChargesT.DoctorsProviders, ChargesT.ServiceID "
			"FROM LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"WHERE (((LineItemT.PatientID) = {INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type) = 10)) "
			") AS PatientChargesQ INNER JOIN (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID "
			"WHERE (((PatientChargesQ.ID) = {INT})) "
			"GROUP BY PatientChargesQ.ID, PatientChargesQ.ServiceID, PatientChargesQ.DoctorsProviders, PatientChargesQ.LocationID, PatientChargesQ.POSID",
			nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nActivePatientID, BillID);
	}
	else if ((ClickedType == "Payment" || ClickedType == "PrePayment" || 
		      ClickedType == "Adjustment" || ClickedType == "Refund") && nInsuredPartyID == -1) {
		/* Added by Chris 9/24/99 - You can't apply a payment to
		that of another insurance type */

		//(e.lally 2007-03-30) PLID 25263 - Combined this logic into one trip to the DB.
		// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
		rsBookingInfo = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"DECLARE @nActivePatientID INT \r\n"
			"SET @nActivePatientID = {INT} \r\n"
			
			//first get the source payment's insured ID
			"DECLARE @nSourceInsID INT \r\n"
			"SET @nSourceInsID = COALESCE((SELECT InsuredPartyID FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE ((LineItemT.PatientID)= @nActivePatientID) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
			"AS PatientPaymentsQ WHERE ID = {INT}), 0) \r\n"
			
			//now get the destination payment's insured ID
			"DECLARE @nDestInsID INT \r\n"
			"SET @nDestInsID = COALESCE((SELECT InsuredPartyID FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE ((LineItemT.PatientID)=@nActivePatientID) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
			"AS PatientPaymentsQ WHERE ID = {INT}), 0) \r\n"
			
			
			"IF (@nSourceInsID <> @nDestInsID) BEGIN \r\n"
			//Return our results
			//(e.lally 2007-07-17) PLID 25263 - Changed these queries to join on InsuredPartyT for correctness.
			"	SET NOCOUNT OFF \r\n"
			"	SELECT COALESCE(("
			"			SELECT TypeName "
			"			FROM InsuredPartyT "
			"			INNER JOIN RespTypeT ON RespTypeID = RespTypeT.ID "
			"			WHERE PersonID = @nSourceInsID), "
			"		'Patient') AS SourceInsName, "
			"	COALESCE(("
			"			SELECT TypeName "
			"			FROM InsuredPartyT "
			"			INNER JOIN RespTypeT ON RespTypeID = RespTypeT.ID "
			"			WHERE PersonID = @nDestInsID), "
			"		'Patient') AS DestInsName \r\n "
			"	RETURN; \r\n"
			"END \r\n"
			//return nothing
			"SET NOCOUNT OFF ", nActivePatientID, PayID, BillID);
		if(rsBookingInfo->GetState() == adStateOpen && !rsBookingInfo->eof){
			CString strSourceInsName, strDestInsName;
			//(e.lally 2007-07-17) PLID 25263 - Fixed a typo in this field name
			strSourceInsName = AdoFldString(rsBookingInfo, "SourceInsName", "");
			strDestInsName = AdoFldString(rsBookingInfo, "DestInsName", "");
			str.Format("You may not apply this item because it has %s reponsibility, and you are attempting to apply to %s responsibility.", strSourceInsName, strDestInsName);
			MsgBox(str);
			rsBookingInfo->Close();
			return FALSE;
		}

		// Get a payment
		// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
		// (j.jones 2009-10-23 10:29) - PLID 18558 - added POSID, but it is not used in this case
		rsBillInfo = CreateParamRecordset("SELECT ID, "
			"Convert(money, Amount) AS TotalCharges, LocationID, -1 AS POSID "
			"FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
			") AS PatientPaymentsQ WHERE PatientPaymentsQ.ID = {INT}", nActivePatientID, BillID);
	}
	else {
		// (j.jones 2014-01-15 16:16) - I got this when I passed in an insured party ID
		// when applying a refund to a payment. I have no idea why that is a problem.
		// Rather than change this ancient code, I added this assert to warn future developers
		// that pay-to-pay applies should have a -1 insured party ID at all times.
		ASSERT(FALSE);
		return FALSE;
	}

	CWaitCursor cuWait;  //This starts the wait cursor, and ends it when this object loses scope.  This is much
						 //easier than putting "EndWaitCursor" before every return.

	if(!rsBillInfo->eof)
		rsBillInfo->MoveLast();

	//DRT 4/25/03 - Get the location of the applied and the applied-to items.  If they differ, prompt a warning
	//		of such and let them cancel the save.
	//	  4/28/03 - Get the type of warning we're going to do (0 = nothing, 1 = warn, 2 = forbid)
	long nWarnType = GetRemotePropertyInt("PaymentLocationApplyWarning", 1, 0, "<None>", true);
	long nAppliedItemLocation = -1;
	long nAppliedToLocation = -1;
	long nAppliedToPOS = -1;

	// (j.jones 2009-01-13 11:31) - PLID 32716 - we need these locations regardless of the preference

	//get the location of what we're applying to
	if(!rsBillInfo->eof) {
		//despite the fact that every charge has a location, they're all bill-based, 
		//so we only need to look at the first one
		nAppliedToLocation = AdoFldLong(rsBillInfo, "LocationID", -1);
		// (j.jones 2009-10-23 10:29) - PLID 18558 - we now also need the Place Of Service
		nAppliedToPOS = AdoFldLong(rsBillInfo, "POSID", -1);
	}

	//lookup the LocationID of the item we're applying
	_RecordsetPtr prsApplyLoc = CreateParamRecordset("SELECT LocationID FROM LineItemT WHERE ID = {INT}", PayID);
	if(!prsApplyLoc->eof) {
		nAppliedItemLocation = AdoFldLong(prsApplyLoc, "LocationID", -1);
	}

	//(e.lally 2007-03-30) PLID 25263 - Insurance applies has a bWarnLocation to check for
	// (r.gonet 2015-02-11 13:36) - PLID 64852 - Fixed a logic bug with bWarnLocationPatientResp within the if condition, which was checking bWarnlocationPatientResp even if the responsibility was non-patient.
	if(nWarnType != 0 && ((bWarnLocation && nInsuredPartyID != -1) || (nInsuredPartyID == -1 && bWarnLocationPatientResp))) {
		//only do any of this if we're going to do something about it

		//and do the final check
		if(nAppliedItemLocation != nAppliedToLocation) {

			CString strAppliedItemLocName, strAppliedToLocName;
			//(e.lally 2007-03-30) PLID 25263 - Combined 3 create recordsets into 1.
			CString strPayType = "payment";
			// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"(SELECT Name FROM LocationsT WHERE ID = {INT}) AS AppliedItemLocation, "
				"(SELECT Name FROM LocationsT WHERE ID = {INT}) AS AppliedToLocation, "
				"(SELECT Type FROM LineItemT WHERE ID = {INT}) AS Type "
				,nAppliedItemLocation, nAppliedToLocation, PayID);
			if(!rs->eof) {
				strAppliedItemLocName = AdoFldString(rs, "AppliedItemLocation","");
				strAppliedToLocName = AdoFldString(rs, "AppliedToLocation","");
				long Type = AdoFldLong(rs, "Type", -1);
				if(Type == 2)
					strPayType = "adjustment";
				else if(Type == 3)
					strPayType = "refund";
			}
			rs->Close();

			if(nWarnType == 1) {
				//warn them
				str.Format("You are attempting to apply a %s with a different location.\n"
					"(You are applying a line-item from '%s' to one from '%s'.)\n\n"
					"Are you sure you wish to do this?\n"
					"If you do not wish to apply now, the %s will be created but not applied. You can later edit this %s to change the location before applying.", strPayType, strAppliedItemLocName, strAppliedToLocName, strPayType, strPayType);
				if(MsgBox(MB_YESNO, str) == IDNO)
					return FALSE;
			}
			else {				
				//forbidden
				str.Format("You are attempting to apply a %s with a different location.\n"
					"(You are applying a line-item from '%s' to one from '%s'.)\n\n"
					"This has been disallowed by your administrator.\n"
					"The %s will be saved, but not applied. You can edit it to fix any inconsistencies.",strPayType, strAppliedItemLocName, strAppliedToLocName, strPayType);
				MsgBox(str);
				return FALSE;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////
// Go through each appliable item ("charge") trying to apply some amount
// from PayAmount
///////////////////////////////////////////////////////////////////////////////////
	for (int i=0; i < rsBillInfo->GetRecordCount(); i++) {

		// Exit if there is nothing left to apply
		if (TempAmount == COleCurrency(0,0) && !bAllowZeroDollarApply)
			return TRUE;

		// (a.walling 2007-11-29 14:23) - PLID 28231 - Use 0 as default just in case
		cy = AdoFldCurrency(rsBillInfo, "TotalCharges", COleCurrency(0, 0));

		// If the charge is $0.00, skip it
		if (cy == COleCurrency(0,0) && !bAllowZeroDollarApply) {
			rsBillInfo->MovePrevious();
			nSkippedCharges++;
			continue;
		}

		// Open the table that gives us the sum of existing applies directed to the
		// current "charge".
		//(e.lally 2007-03-30) PLID 25263 - Insurance applies always used the first rsApplied recordset
		if (ClickedType == "Bill" || ClickedType == "Charge" || nInsuredPartyID != -1) {
			var = rsBillInfo->Fields->GetItem("ID")->Value;

			// (j.jones 2009-10-06 11:53) - PLID 27713 - moved into the recordset
			/*
			CString strInsuredPartyFilter;
			if(nInsuredPartyID ==-1){
				strInsuredPartyFilter = "(PatientAppliesQ.InsuredPartyID = -1 OR PatientAppliesQ.InsuredPartyID IS NULL) AND ";
			}
			else{
				strInsuredPartyFilter.Format("(PatientAppliesQ.InsuredPartyID = %li) AND ", nInsuredPartyID);
			}
			*/
			
			// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
			rsApplied = CreateParamRecordset("SELECT Sum(Convert(money,(CASE WHEN PatientAppliesQ.Amount Is Null THEN 0 ELSE PatientAppliesQ.Amount END))) AS SumOfAmount "
				"FROM (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
					") AS PatientChargesQ "
				"LEFT JOIN (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
					"FROM AppliesT "
					"LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
						"FROM LineItemT "
						"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND (LineItemT.Type >= 1 And LineItemT.Type <= 3) "	
						") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
						"WHERE PatientPaymentsQ.ID Is Not Null "
					") AS PatientAppliesQ ON PatientChargesQ.ID = PatientAppliesQ.DestID "
				"WHERE PatientAppliesQ.PointsToPayments = 0 "
				"AND (({INT} = -1 AND (PatientAppliesQ.InsuredPartyID = -1 OR PatientAppliesQ.InsuredPartyID IS NULL)) "
					"OR ({INT} <> -1 AND PatientAppliesQ.InsuredPartyID = {INT}))"
				"AND PatientChargesQ.ID = {INT}", nActivePatientID, nActivePatientID, nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, VarLong(var));
		}
		else {
			// (j.jones 2005-07-01 10:29) - PLID 16638 - I'm not convinced the old query really worked properly, this is much easier
			// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
			rsApplied = CreateParamRecordset("SELECT Coalesce((SELECT Sum(AppliesT.Amount) AS SumOfAmount FROM AppliesT WHERE DestID = {INT}),0) "
				"- Coalesce((SELECT Sum(AppliesT.Amount) AS SumOfAmount FROM AppliesT WHERE SourceID = {INT}),0) AS SumOfAmount ", BillID, BillID);
		}

		// Get how much has been applied for this "charge"
		if (!rsApplied->eof) {
			var = rsApplied->Fields->GetItem("SumOfAmount")->Value;
			COleCurrency cyApplied = VarCurrency(var, COleCurrency(0,0));
			if(cyApplied < COleCurrency(0,0))
				cyApplied = -cyApplied;
			cyRemainingCharge = cy - cyApplied;
		}
		else {
			cyRemainingCharge = cy;
		}
		//rsApplied->Close();

		// If this is a negative amount and a type of payment, make it positive
		if (ClickedType != "Bill" && ClickedType != "Charge" &&	cyRemainingCharge < COleCurrency(0,0))
			cyRemainingCharge *= -1;


		// If there's nothing left we need to apply, continue
		// (j.jones 2010-06-28 12:50) - PLID 37989 - bad data could make the charge less than zero
		// and we would happily keep applying, so stop if less than or equal to zero
		if (cyRemainingCharge <= COleCurrency(0,0) && !bAllowZeroDollarApply) {
			rsBillInfo->MovePrevious();
			nSkippedCharges++;
			continue;
		}

		//TS:  Get the ChargeRespT ID (Patient responsibilty)
		_RecordsetPtr rsResp;
		long nRespID = -1;
		CString strChargeRespSql;
		if(!PointsToPayments) {

			// (j.jones 2009-10-06 11:46) - PLID 27713 - moved into the recordset
			/*
			CString strInsuredPartyFilter;
			if(nInsuredPartyID == -1)
				strInsuredPartyFilter = " IS NULL ";
			else
				strInsuredPartyFilter.Format(" = %li", nInsuredPartyID);
			*/

			// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
			rsResp = CreateParamRecordset("SELECT ID FROM ChargeRespT WHERE ChargeID = {INT} "
				"AND (({INT} = -1 AND InsuredPartyID Is Null) OR ({INT} <> -1 AND InsuredPartyID = {INT}))",
				AdoFldLong(rsBillInfo, "ID"), nInsuredPartyID, nInsuredPartyID, nInsuredPartyID);
			if(!rsResp->eof)
				nRespID = AdoFldLong(rsResp, "ID");
			else {
				//JJ - if the charge is completely insurance, then there would be no patient resp entry
				//5/2/2003 - however, we can now apply a $0.00 payment, and there may not be a charge resp entry for it
				//so if it is a zero dollar payment, create a ChargeResp entry for it and apply
				if(!bAllowZeroDollarApply) {
					rsBillInfo->MovePrevious();
					nSkippedCharges++;
					continue;
				} else {
					//(e.lally 2007-03-30) PLID 25263 - Save this query to be run in conjuction with any detail records too.

					// (j.jones 2008-02-04 10:37) - PLID 28254 - make sure we create the resp for the proper
					// insured party, or patient
					CString strInsuredPartyID = "NULL";
					if(nInsuredPartyID != -1) {
						strInsuredPartyID.Format("%li", nInsuredPartyID);
					}

					// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
					strChargeRespSql.Format("SET NOCOUNT ON \r\n"
					"DECLARE @NewChargeRespID INT \r\n"
					"INSERT INTO ChargeRespT (ChargeID, InsuredPartyID, Amount) VALUES (%li, %s, Convert(money,0))"
					"SET @NewChargeRespID = SCOPE_IDENTITY() \r\n",
					AdoFldLong(rsBillInfo, "ID"), strInsuredPartyID);
				}
			}
		}

		if(nRespID == -1){
			//JJ: we need to insert a ChargeRespDetailT if none exists for this charge resp and it's a $0.00 charge
			if(bAllowZeroDollarApply && !PointsToPayments) {
				//(e.lally 2007-03-30) PLID 25263 - Moved the check for an existing charge resp detail record into the
					//sql statement. Added the insert statement to our sql string.
				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				strChargeRespSql += 
					"IF(NOT EXISTS(SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = @NewChargeRespID) ) BEGIN \r\n"
					"	INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) "
					"	VALUES (@NewChargeRespID, Convert(money,0), GetDate()) \r\n"
					"END \r\n"; 
			}

			if(!strChargeRespSql.IsEmpty()){
				//Run the inserts and return the new ChargeRespID.
				// (j.jones 2009-10-06 11:46) - PLID 27713 - cannot be parameterized
				_RecordsetPtr rsChargeResp = CreateRecordset("%s \r\n SET NOCOUNT OFF \r\n"
					"SELECT @NewChargeRespID AS NewChargeRespID ", strChargeRespSql);
				if(rsChargeResp->GetState() == adStateOpen && !rsChargeResp->eof){
					nRespID = AdoFldLong(rsChargeResp, "NewChargeRespID", -1);
					rsChargeResp->Close();
				}
			}
		}
		else if(bAllowZeroDollarApply && !PointsToPayments) {
			//JJ: we need to insert a ChargeRespDetailT if none exists for this charge resp and it's a $0.00 charge
			//(e.lally 2007-03-30) PLID 25263 - There was already an existing ChargeRespID, now we need to separately
			//check for a ChargeRespDetailID, using our known RespID
			// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
			// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
			ExecuteParamSql( 
				"IF(NOT EXISTS(SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = {INT}) ) BEGIN \r\n"
				"	INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) "
				"	VALUES ({INT}, Convert(money,0), GetDate()) \r\n"
				"END \r\n", nRespID, nRespID); 
		}


		if(nInsuredPartyID == -1){

			_variant_t varRespID = g_cvarNull;
			if(nRespID != -1) {
				varRespID = nRespID;
			}

			long ApplyID;
			// Go here if the "charge" exceeds or equals what he have left to
			// apply (also compare the reverse if the amount to apply is negative)
			if ((TempAmount > COleCurrency(0,0) && cyRemainingCharge >= TempAmount) ||
				(TempAmount < COleCurrency(0,0) && cyRemainingCharge >= (-TempAmount)) ||
				(TempAmount == COleCurrency(0,0) && bAllowZeroDollarApply)) {
				//(e.lally 2007-03-30) PLID 25263 - Combined the NewNumber logic into the insert statement and return
				//its result.
				// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
				// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
				_RecordsetPtr rs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName)\r\n" 
					"	VALUES ({INT}, {INT}, {VT_I4}, {OLECURRENCY}, {INT}, GetDate(), {STRING})\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID",
					PayID, AdoFldLong(rsBillInfo, "ID"), varRespID, TempAmount, PointsToPayments ? 1 : 0, GetCurrentUserName());
				if(rs->GetState() == adStateClosed){
					_variant_t varNull;
					rs = rs->NextRecordset(&varNull);
				}
				if(!rs->eof){
					ApplyID = AdoFldLong(rs, "ApplyID", -1);
					rs->Close();
				}
				ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, TempAmount, ApplyID, PayID, bAllowZeroDollarApply);

				TempAmount = COleCurrency(0,0);

				//if it was a $0.00 payment, then exit now, don't keep trying to apply
				if(bAllowZeroDollarApply)
					return TRUE;
			}
			// Go here if what we have to apply exceeds the "charge"
			else {
				if(TempAmount < COleCurrency(0,0))
					cyRemainingCharge = -cyRemainingCharge;
				//(e.lally 2007-03-30) PLID 25263 - Combined the NewNumber logic into the insert statement and return
				//its result.
				// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
				// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
				_RecordsetPtr rs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName)\r\n"
					"	VALUES ({INT}, {INT}, {VT_I4}, {OLECURRENCY}, {INT}, GetDate(), {STRING})\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID",
					PayID, AdoFldLong(rsBillInfo, "ID"), varRespID, cyRemainingCharge, PointsToPayments ? 1 : 0, GetCurrentUserName());
				if(rs->GetState() == adStateClosed){
					_variant_t varNull;
					rs = rs->NextRecordset(&varNull);
				}
				if(!rs->eof){
					ApplyID = AdoFldLong(rs, "ApplyID", -1);
					rs->Close();
				}

				//PLID 16397 - we need to be sending what is being applied, not the whole amount
				ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, cyRemainingCharge, ApplyID, PayID, bAllowZeroDollarApply);

				//need to account for refunds being larger than the payment they are refunding
				if(TempAmount > COleCurrency(0,0)) {
					TempAmount -= cyRemainingCharge;
				}
				else if(TempAmount < COleCurrency(0,0)) {
					cyRemainingCharge = -cyRemainingCharge;
					TempAmount += cyRemainingCharge;				
				}
			}
		}
		else {
			//It is an insurance apply
			// Go here if the "charge" exceeds or equals what he have left to
			// apply, or if the amount to apply is negative.
			// (j.jones 2016-05-10 16:08) - NX-100502 - fixed the use of bAllowZeroDollarApply
			if (cyRemainingCharge >= TempAmount ||
				TempAmount < COleCurrency(0,0) || 
				(TempAmount == COleCurrency(0,0) && bAllowZeroDollarApply)) {

				long nChargeID = AdoFldLong(rsBillInfo, "ID");

				long nApplyID = -1;
				//(e.lally 2007-03-30) PLID 25263 - Combined the NewNumber logic into the insert statement and return
				//its result.
				// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
				// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
				_RecordsetPtr rs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO AppliesT (DestID, SourceID, RespID, Amount, InputName, InputDate, PointsToPayments)\r\n"
					"	VALUES ({INT}, {INT}, {INT}, {OLECURRENCY}, {STRING}, GetDate(), 0)\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID",
					nChargeID, PayID, nRespID, TempAmount, GetCurrentUserName());

				if(rs->GetState() == adStateClosed){
					_variant_t varNull;
					rs = rs->NextRecordset(&varNull);
				}
				if(!rs->eof){
					nApplyID = AdoFldLong(rs, "ApplyID", -1);
					rs->Close();
				}

				CString strRespID = AsStringForSql(nRespID);

				ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, TempAmount, nApplyID, PayID, bAllowZeroDollarApply);

				cyRemainingCharge = cyRemainingCharge - TempAmount;
				TempAmount = COleCurrency(0,0);

				//if the charge is greater than the payment, prompt to
				//shift the remaining balance to the other insurance co.
				if(cyRemainingCharge > TempAmount && !bShiftToPatient && !bAdjustBalance && bPromptToShift) {
					// (j.jones 2013-07-22 11:43) - PLID 57653 - added the payment ID and date
					PostInsuranceApply(PayID, nInsuredPartyID, PatientID, BillID, ClickedType);
				}

				// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
				// for when the apply is part of an E-Remittance posting
				// (j.jones 2011-03-22 17:24) - PLID 42936 - moved to be after shifting
				if(!bDisableAllowableCheck && (ClickedType == "Bill" || ClickedType == "Charge")) {

					if(nInsuredPartyID != -1) {
						// (j.jones 2011-03-24 09:38) - PLID 42936 - If this is an insurance payment,
						// then we should not check the allowable until after shifting is done,
						// which would be outside of this function if bPromptToShift is FALSE.
						// Therefore, every call to this function that sets bPromptToShift to FALSE
						// should then also set bDisableAllowableCheck to TRUE and handle the call
						// to WarnAllowedAmount after shifting completes.
						ASSERT(bPromptToShift);
					}

					//only warn for allowables when you are applying a payment, not an adjustment

					CSqlFragment sqlClickedType("");
					if(ClickedType == "Bill") {
						sqlClickedType = CSqlFragment("AND BillsT.ID = {INT}", BillID);
					}
					else if(ClickedType == "Charge") {
						//BillID is ChargeID, brilliant idea...
						sqlClickedType = CSqlFragment("AND ChargesT.ID = {INT}", BillID);
					}

					_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
						"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
						"FROM LineItemT "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
						"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
						"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
						"INNER JOIN LineItemT PaymentLineItemT ON AppliesT.SourceID = PaymentLineItemT.ID "
						"WHERE LineItemT.Deleted = 0 AND PaymentLineItemT.Type = 1 "
						"AND AppliesT.SourceID = {INT} {SQL} "
						"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
						"InsuredPartyT.PersonID, ChargesT.ID", PayID, sqlClickedType);
					while(!rsAppliedTo->eof) {
						//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
						WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
							AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
							AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
							AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

						//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
						rsAppliedTo->MoveNext();
					}
					rsAppliedTo->Close();
				}

				//if it was a $0.00 payment, then exit now, don't keep trying to apply
				if(bAllowZeroDollarApply)
					return TRUE;
			}
			// Go here if what we have to apply exceeds the "charge"
			else {

				long nApplyID=-1;
				//(e.lally 2007-03-30) PLID 25263 - Combined the NewNumber logic into the insert statement and return
				//its result.
				// (j.jones 2009-10-06 11:46) - PLID 27713 - parameterized
				// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
				_RecordsetPtr rs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO AppliesT (DestID, SourceID, RespID, Amount, InputName, InputDate, PointsToPayments)\r\n"
					"	VALUES ({INT}, {INT}, {INT}, {OLECURRENCY}, {STRING}, GetDate(), 0)\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID ",
					AdoFldLong(rsBillInfo, "ID"), PayID, nRespID, cyRemainingCharge, GetCurrentUserName());
				if(rs->GetState() == adStateClosed){
					_variant_t varNull;
					rs = rs->NextRecordset(&varNull);
				}
				if(!rs->eof){
					nApplyID = AdoFldLong(rs, "ApplyID", -1);
					rs->Close();
				}

				CString strRespID = AsStringForSql(nRespID);

				//PLID 16634 - this should be cyRemainingAmount, I didn't fix it before because the logic of ApplyToDetails
				//handles this, but just to be correct, it should be changed
				ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, cyRemainingCharge, nApplyID, PayID, bAllowZeroDollarApply);

				TempAmount = TempAmount - cyRemainingCharge;
			}
		}

		rsBillInfo->MovePrevious();
	}//end for loop

	if (nSkippedCharges == i) {
		// (j.jones 2016-05-10 16:15) - NX-100502 - we now actually use this switch statement, finally
		if (nInsuredPartyID != -1) {
			switch (GetInsuranceTypeFromID(nInsuredPartyID)) {
			case 1: str.Format(GetStringOfResource(IDS_APPLY_ZERO_PRI_RESP_CONFIRM), ClickedType.MakeLower()); break;
			case 2: str.Format(GetStringOfResource(IDS_APPLY_ZERO_SEC_RESP_CONFIRM), ClickedType.MakeLower()); break;
			case 3: str.Format(GetStringOfResource(IDS_APPLY_ZERO_TER_RESP_CONFIRM), ClickedType.MakeLower()); break;
			default:
				str.Format("There is no remaining insurance responsibility for this %s. The balance will be left unapplied.", ClickedType.MakeLower());
				break;
			}
		}
		else {
			str.Format(GetStringOfResource(IDS_APPLY_ZERO_PAT_RESP_CONFIRM), ClickedType.MakeLower());
		}
		AfxMessageBox(str);
	}

	CClient::RefreshTable(NetUtils::PatBal, nActivePatientID);
	PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, nActivePatientID, COleDateTime::GetCurrentTime(),PayID,true,-1);

	return TRUE; 
}

// (j.jones 2006-12-28 10:31) - PLID 23160 - supported applying based on revenue code
// copied from ApplyInsurancePayToBill and modified to filter by revenue code
// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific, and added two insured party IDs
// (j.jones 2011-03-23 10:49) - PLID 42936 - added bDisableAllowableCheck
void ApplyPayToBillWithRevenueCode(long PayID, long PatientID, COleCurrency PayAmount, long BillID, long RevenueCodeID, long nInsuredPartyIDToPost, long nInsuredPartyIDForRevCode, BOOL bShiftToPatient /*= FALSE*/, BOOL bAdjustBalance /*= FALSE*/, BOOL bPromptToShift /*= TRUE*/, BOOL bWarnLocation /* = TRUE*/,
											BOOL bAlwaysAllowZeroDollarApply /* = FALSE*/, BOOL bDisableAllowableCheck /*= FALSE*/)
{
	_RecordsetPtr rsBookingInfo, rsBillInfo, rsApplied;
	COleCurrency TempAmount, cy, cyRemainingCharge;
	_variant_t var;
	CString str;
	int nSkippedCharges = 0;
	long nActivePatientID = PatientID;

	// (j.jones 2010-05-17 17:04) - PLID 16503 - we now accept an override for this setting
	BOOL bAllowZeroDollarApply = bAlwaysAllowZeroDollarApply || IsZeroDollarPayment(PayID);

	if(nInsuredPartyIDForRevCode == -1)
		return;

	TempAmount = PayAmount;

	// Get all the charges for a bill that use the given revenue code
	// (j.jones 2009-10-06 12:36) - PLID 27713 - parameterized
	// (j.jones 2009-10-23 10:33) - PLID 18558 - added POSID
	// (j.jones 2010-06-03 11:59) - PLID 37200 - this function now takes in a separate nInsuredPartyIDForRevCode, which
	// might not be the resp. we're posting to, but we need to calculate revenue codes using that ID
	// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
	rsBillInfo = CreateParamRecordset("SELECT ChargesT.ID, ChargesT.ServiceID, ChargesT.DoctorsProviders, LineItemT.LocationID, "
		"Convert(money, Sum(ChargeRespT.Amount)) AS TotalCharges, BillsT.Location AS POSID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
		"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
		"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = {INT}) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 			
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND ChargesT.BillID = {INT} "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND InsuredPartyT.PersonID = {INT} "
		"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
		"GROUP BY ChargesT.ID, ChargesT.ServiceID, ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location",
		nInsuredPartyIDForRevCode, nActivePatientID, BillID, nInsuredPartyIDForRevCode, RevenueCodeID);
	
	CWaitCursor cuWait;  //This begins the wait cursor, and it will end when this object goes out of scope.  Nifty, eh?

	// (j.jones 2008-04-15 16:06) - PLID 29668 - added an assertion if eof
	if(rsBillInfo->eof) {
		//should be impossible
		ASSERT(FALSE);
	}

	rsBillInfo->MoveLast();

///////////////////////////////////////////////////////////////////////////////////
// Go through each appliable item ("charge") trying to apply some amount
// from PayAmount
///////////////////////////////////////////////////////////////////////////////////

	//DRT 4/28/03 - Get the location of the applied and the applied-to items.  If they differ, prompt a warning
	//		of such and let them cancel the save.
	//	  4/28/03 - Get the warning type property and act accordingly
	long nWarnType = GetRemotePropertyInt("PaymentLocationApplyWarning", 1, 0, "<None>", true);
	long nAppliedItemLocation = -1;
	long nAppliedToLocation = -1;
	long nAppliedToPOS = -1;

	// (j.jones 2009-01-13 11:31) - PLID 32716 - we need these locations regardless of the preference

	//get the location of what we're applying to
	if(!rsBillInfo->eof) {
		//despite the fact that every charge has a location, they're all bill-based, 
		//so we only need to look at the first one
		nAppliedToLocation = AdoFldLong(rsBillInfo, "LocationID", -1);
		// (j.jones 2009-10-23 10:29) - PLID 18558 - we now also need the Place Of Service
		nAppliedToPOS = AdoFldLong(rsBillInfo, "POSID", -1);
	}

	//lookup the LocationID of the item we're applying
	_RecordsetPtr prsApplyLoc = CreateParamRecordset("SELECT LocationID FROM LineItemT WHERE ID = {INT}", PayID);
	if(!prsApplyLoc->eof) {
		nAppliedItemLocation = AdoFldLong(prsApplyLoc, "LocationID", -1);
	}

	if(bWarnLocation && nWarnType != 0) {
		//only process this if we're going to do something about it

		//and do the final check
		if(nAppliedItemLocation != nAppliedToLocation) {

			CString strAppliedItemLocName, strAppliedToLocName;
			CString strPayType = "payment";

			// (j.jones 2009-10-06 12:36) - PLID 27713 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"(SELECT Name FROM LocationsT WHERE ID = {INT}) AS AppliedItemLocation, "
				"(SELECT Name FROM LocationsT WHERE ID = {INT}) AS AppliedToLocation, "
				"(SELECT Type FROM LineItemT WHERE ID = {INT}) AS Type "
				,nAppliedItemLocation, nAppliedToLocation, PayID);
			if(!rs->eof) {
				strAppliedItemLocName = AdoFldString(rs, "AppliedItemLocation","");
				strAppliedToLocName = AdoFldString(rs, "AppliedToLocation","");
				long Type = AdoFldLong(rs, "Type", -1);
				if(Type == 2)
					strPayType = "adjustment";
				else if(Type == 3)
					strPayType = "refund";
			}
			rs->Close();

			if(nWarnType == 1) {
				CString strWarning;
				strWarning.Format("You are attempting to apply a %s with a different location.\n"
					"(You are applying a line-item from '%s' to one from '%s'.)\n\n"
					"Are you sure you wish to do this?\n"
					"If you do not wish to apply now, the %s will be created but not applied. You can later edit this %s to change the location before applying.", strPayType, strAppliedItemLocName, strAppliedToLocName, strPayType, strPayType);
				//warning
				if(MsgBox(MB_YESNO, strWarning) == IDNO)
					return;
			}
			else {
				//forbidden
				CString strWarning;
				strWarning.Format("You are attempting to apply a %s with a different location.\n"
					"(You are applying a line-item from '%s' to one from '%s'.)\n\n"
					"This has been disallowed by your administrator.\n"
					"The %s will be saved, but not applied. You can edit it to fix any inconsistencies.", strPayType, strAppliedItemLocName, strAppliedToLocName, strPayType);
				MsgBox(strWarning);
				return;
			}
		}
	}

	//get the insured party ID

	for (int i=0; i < rsBillInfo->GetRecordCount(); i++) {

		// Exit if there is nothing left to apply
		if (TempAmount == COleCurrency(0,0) && !bAllowZeroDollarApply)
			return;

		// (a.walling 2007-11-29 14:23) - PLID 28231 - Use 0 as default just in case
		cy = AdoFldCurrency(rsBillInfo, "TotalCharges", COleCurrency(0, 0));

		// If the charge is $0.00, skip it
		if (cy == COleCurrency(0,0) && !bAllowZeroDollarApply) {
			rsBillInfo->MovePrevious();
			nSkippedCharges++;
			continue;
		}

		// Open the table that gives us the sum of existing applies directed to the
		// current "charge".
		// (j.jones 2009-10-06 12:36) - PLID 27713 - parameterized
		rsApplied = CreateParamRecordset("SELECT Sum(Convert(money, (CASE WHEN PatientAppliesQ.Amount Is Null THEN 0 ELSE PatientAppliesQ.Amount END))) AS SumOfAmount "
			"FROM (SELECT LineItemT.*, ChargesT.BillID, ChargesT.ServiceID, ChargesT.DoctorsProviders "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE (((LineItemT.PatientID) = {INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=10)) "
			") AS PatientChargesQ LEFT JOIN (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
			"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
			") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
			"WHERE (((PatientPaymentsQ.ID) Is Not Null)) "
			") AS PatientAppliesQ ON PatientChargesQ.ID = PatientAppliesQ.DestID "
			"WHERE ({INT} = -1 AND PatientAppliesQ.InsuredPartyID Is Null) OR PatientAppliesQ.InsuredPartyID = {INT} "
			"GROUP BY PatientChargesQ.ID HAVING (((PatientChargesQ.ID)={INT}))", 
			nActivePatientID, nActivePatientID, nInsuredPartyIDToPost, nInsuredPartyIDToPost, AdoFldLong(rsBillInfo, "ID"));
		
		// Get how much has been applied for this "charge"
		if (!rsApplied->eof) {
			cyRemainingCharge = cy - AdoFldCurrency(rsApplied, "SumOfAmount", COleCurrency(0,0));
		}
		else {
			cyRemainingCharge = cy;
		}
		//rsApplied->Close();

		// If there's nothing left we need to apply, continue
		// (j.jones 2010-06-28 12:50) - PLID 37989 - bad data could make the charge less than zero
		// and we would happily keep applying, so stop if less than or equal to zero
		if (cyRemainingCharge <= COleCurrency(0,0) && !bAllowZeroDollarApply) {
			rsBillInfo->MovePrevious();
			nSkippedCharges++;
			continue;
		}

		//TS:  Get ChargeRespT ID
		_RecordsetPtr rsResp;
		long nRespID = -1;
		// (j.jones 2009-10-06 12:06) - PLID 27713 - parameterized
		rsResp = CreateParamRecordset("SELECT ID FROM ChargeRespT "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"WHERE ChargeID = {INT} AND PersonID = {INT}", AdoFldLong(rsBillInfo, "ID"), nInsuredPartyIDToPost);
		if(!rsResp->eof)
			nRespID = AdoFldLong(rsResp, "ID");
		else {
			//JJ - if the charge is completely patient, then there would be no ins. resp entry
			//5/12/2003 - however, we can now apply a $0.00 payment, and there may not be a charge resp entry for it
			//so if it is a zero dollar payment, create a ChargeResp entry for it and apply
			if(!bAllowZeroDollarApply) {
				rsBillInfo->MovePrevious();
				nSkippedCharges++;
				continue;
			} else {
				// (j.jones 2009-10-06 12:06) - PLID 27713 - parameterized
				_variant_t vtInsuredPartyIDToPost = (nInsuredPartyIDToPost == -1) ? g_cvarNull : nInsuredPartyIDToPost;

				// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO ChargeRespT (ChargeID, InsuredPartyID, Amount) VALUES ({INT}, {VT_I4}, Convert(money,0))\r\n"
					"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ChargeRespID\r\n"
					"SET NOCOUNT OFF\r\n",
					AdoFldLong(rsBillInfo, "ID"), vtInsuredPartyIDToPost);

				nRespID = AdoFldLong(prs, "ChargeRespID");
			}
		}
		rsResp->Close();

		//JJ: we need to insert a ChargeRespDetailT if none exists for this charge resp and it's a $0.00 charge		
		if(bAllowZeroDollarApply) {
			// (j.jones 2009-10-07 09:09) - PLID 27713 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = {INT}",nRespID);
			if(rs->eof) {
				// (j.jones 2009-10-07 08:53) - PLID 27713 - parameterized
				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				ExecuteParamSql("INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) VALUES ({INT}, Convert(money,0), GetDate());", nRespID); 
			}
			rs->Close();
		}

		// Go here if the "charge" exceeds or equals what he have left to
		// apply, or if the amount to apply is negative.
		// (j.jones 2012-08-17 14:50) - PLID 52212 - this was causing all payments
		// to always apply if bAllowZeroDollarApply was true, when it should only 
		// have allowed $0.00 applies
		if (cyRemainingCharge >= TempAmount || TempAmount < COleCurrency(0,0) ||
			(TempAmount == COleCurrency(0,0) && bAllowZeroDollarApply)) {

			// (j.jones 2009-10-07 08:49) - PLID 27713 - parameterized
			// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"INSERT INTO AppliesT (DestID, SourceID, RespID, Amount, InputName, InputDate, PointsToPayments)\r\n"
				"	VALUES ({INT}, {INT}, {INT}, {OLECURRENCY}, {STRING}, GetDate(), 0)\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID",
				AdoFldLong(rsBillInfo, "ID"), PayID, nRespID, TempAmount, GetCurrentUserName());

			long nApplyID = AdoFldLong(prs, "ApplyID");

			CString strRespID = AsStringForSql(nRespID);

			ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, TempAmount, nApplyID, PayID, bAllowZeroDollarApply);

			cyRemainingCharge = cyRemainingCharge - TempAmount;
			TempAmount = COleCurrency(0,0);

			//if the charge is greater than the payment, prompt to
			//shift the remaining balance to the other insurance co.
			if(cyRemainingCharge > TempAmount && !bShiftToPatient && !bAdjustBalance && bPromptToShift && nInsuredPartyIDToPost != -1) {
				// (j.jones 2013-07-22 11:43) - PLID 57653 - added the payment ID and date
				PostInsuranceApply(PayID, nInsuredPartyIDToPost, PatientID, BillID, "Charge");
			}

			//only warn for allowables when you are applying a payment, not an adjustment
			// (j.jones 2009-10-07 09:09) - PLID 27713 - parameterized
			// (j.jones 2011-03-22 17:24) - PLID 42936 - moved to be after shifting
			// (j.jones 2011-03-23 10:49) - PLID 42936 - added bDisableAllowableCheck
			if(nInsuredPartyIDToPost != -1 && !bDisableAllowableCheck) {

				//only warn for allowables when you are applying a payment, not an adjustment
				_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
					"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
					"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
					"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
					"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN LineItemT PaymentLineItemT ON AppliesT.SourceID = PaymentLineItemT.ID "
					"WHERE LineItemT.Deleted = 0 AND PaymentLineItemT.Type = 1 "
					"AND AppliesT.SourceID = {INT} AND BillsT.ID = {INT} "
					"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
					"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
					"InsuredPartyT.PersonID, ChargesT.ID", PayID, BillID);
				while(!rsAppliedTo->eof) {
					//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
					WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
						AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
						AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
						AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

					//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
					rsAppliedTo->MoveNext();
				}
				rsAppliedTo->Close();
			}

			//if it was a $0.00 payment, then exit now, don't keep trying to apply
			if(bAllowZeroDollarApply)
				return;
		}
		// Go here if what we have to apply exceeds the "charge"
		// (j.jones 2012-08-17 14:50) - PLID 52212 - don't apply to zero dollar charges
		// unless bAllowZeroDollarApply is true
		else if(cyRemainingCharge > COleCurrency(0,0) || (cyRemainingCharge == COleCurrency(0,0) && bAllowZeroDollarApply)) {
			
			// (j.jones 2009-10-07 08:49) - PLID 27713 - parameterized
			// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"INSERT INTO AppliesT (DestID, SourceID, RespID, Amount, InputName, InputDate, PointsToPayments)\r\n"
				"	VALUES ({INT}, {INT}, {INT}, {OLECURRENCY}, {STRING}, GetDate(), 0)\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ApplyID",
				AdoFldLong(rsBillInfo, "ID"), PayID, nRespID, cyRemainingCharge, GetCurrentUserName());

			long nApplyID = AdoFldLong(prs, "ApplyID");

			CString strRespID = AsStringForSql(nRespID);

			//PLID 16634 - this should be cyRemainingAmount, I didn't fix it before because the logic of ApplyToDetails
			//handles this, but just to be correct, it should be changed
			ApplyToDetails(AdoFldLong(rsBillInfo, "ID"), nRespID, cyRemainingCharge, nApplyID, PayID, bAllowZeroDollarApply);

//				rsApplied.GetFieldValue("SumOfAmount", var);
			TempAmount = TempAmount - cyRemainingCharge;
		}

		rsBillInfo->MovePrevious();
	}

	rsBillInfo->Close();
	PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, nActivePatientID, COleDateTime::GetCurrentTime(),PayID,true,-1);

	if (nSkippedCharges == i) {

		// (j.jones 2016-05-10 16:15) - NX-100502 - we now actually use this switch statement, finally
		if(nInsuredPartyIDToPost != -1) {
			switch (GetInsuranceTypeFromID(nInsuredPartyIDToPost)) {
			case 1: str.Format(GetStringOfResource(IDS_APPLY_ZERO_PRI_RESP_CONFIRM), "charge"); break;
			case 2: str.Format(GetStringOfResource(IDS_APPLY_ZERO_SEC_RESP_CONFIRM), "charge"); break;
			case 3: str.Format(GetStringOfResource(IDS_APPLY_ZERO_TER_RESP_CONFIRM), "charge"); break;
			default:
				str = "There is no remaining insurance responsibility for this charge. The balance will be left unapplied.";
				break;
			}
		}
		else {
			str.Format(GetStringOfResource(IDS_APPLY_ZERO_PAT_RESP_CONFIRM), "charge");
		}

		AfxMessageBox(str);
	}
	CClient::RefreshTable(NetUtils::PatBal, nActivePatientID);
}

/*PostInsuranceApply is only called from ApplyInsurancePayToBill, and is
only called when it is needed, so we don't need to check and see if it is needed.
We DO, however, need to check and see if another insurance co. exists.*/

// (j.jones 2006-12-29 11:28) - PLID 23160 - added support for revenue codes
// (j.jones 2013-07-22 11:43) - PLID 57653 - added the payment ID we applied
void PostInsuranceApply(long nPaymentID, long nInsuredPartyID, long nPatientID,
						long nRecordID, CString strClickedType, long nRevenueCodeID /*= -1*/)
{
	if(nInsuredPartyID == -1) {
		return;
	}

	long nNextInsuredPartyID = -1;

	//try to switch to the next insurance company, and if there is none, try to switch to patient

	//We could go with nRespTypeID + 1 but they might only have a Primary and a Tertiary, rather
	//than Primary, Secondary, Tertiary. Sure it doesn't make logistical sense, but it's allowed
	//in our software so we have to handle that possibility. Just get the next RespTypeID, in order.
	
	// (j.jones 2009-10-06 12:36) - PLID 27713 - parameterized
	// (j.jones 2010-09-03 10:19) - PLID 40392 - we need the next resp that's the same category
	// as our current resp, we don't want to automatically select a resp from a different category
	// (j.jones 2013-07-22 13:25) - PLID 57653 - improved this query and added one for the payment date
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID, RespTypeT.TypeName "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"INNER JOIN (SELECT RespTypeT.Priority, RespTypeT.CategoryType, InsuredPartyT.PatientID "
		"	FROM InsuredPartyT "
		"	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"	WHERE InsuredPartyT.PersonID = {INT} "
		") AS CurInsuredPartyQ ON InsuredPartyT.PatientID = CurInsuredPartyQ.PatientID AND RespTypeT.CategoryType = CurInsuredPartyQ.CategoryType "
		"WHERE InsuredPartyT.PatientID = {INT} "
		"AND RespTypeT.Priority > CurInsuredPartyQ.Priority "
		"ORDER BY RespTypeT.Priority; "
		""
		"SELECT Date FROM LineItemT WHERE ID = {INT}",
		nInsuredPartyID, nPatientID,
		nPaymentID);
	CString strPrompt;
	if(!rs->eof) {
		nNextInsuredPartyID = AdoFldLong(rs, "PersonID");
		strPrompt.Format("Would you like to shift the remaining balance to %s responsibility?", AdoFldString(rs, "TypeName"));
	}
	else {
		nNextInsuredPartyID = -1;
		strPrompt.Format("Would you like to shift the remaining balance to Patient responsibility?");
	}

	rs = rs->NextRecordset(NULL);

	//the payment recordset should never be empty, but we don't need to throw an exception in this case
	COleDateTime dtPaymentDate = COleDateTime::GetCurrentTime();
	if(!rs->eof) {
		dtPaymentDate = AdoFldDateTime(rs, "Date");
	}
	rs->Close();

	if(IDYES==MessageBox(GetActiveWindow(), strPrompt, "Practice", MB_ICONQUESTION|MB_YESNO)) {
		// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
		ShiftInsBalance(nRecordID, nPatientID, nInsuredPartyID, nNextInsuredPartyID, strClickedType,
			"after applying a payment/adjustment",
			true, nRevenueCodeID);
	}

	// (j.jones 2013-07-22 10:14) - PLID 57653 - If they have configured insurance companies
	// to force unbatching due to primary crossover to secondary, force unbatching now.
	// This needs to be after shifting/batching has occurred in the normal posting flow.
	if(nInsuredPartyID != -1) {
		long nBillID = -1;
		if(strClickedType == "Charge") {
			rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}", nRecordID);
			if(!rs->eof) {
				nBillID = VarLong(rs->Fields->Item["BillID"]->Value);
			}
			rs->Close();
		}
		else {
			nBillID = nRecordID;
		}
		if(nBillID != -1) {
			//This ConfigRT name is misleading, it actually just means that if we do unbatch a crossed over claim,
			//claim history will only include batched charges. If false, then claim history includes all charges.
			bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

			//This function assumes that the bill's current insured party ID is now the "secondary" insured party
			//we crossed over to, and the insured party who paid was primary.
			//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
			CheckUnbatchCrossoverClaim(nPatientID, nBillID, nInsuredPartyID, dtPaymentDate,
				bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");
		}
	}
}

// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
// for when the apply is part of an E-Remittance posting
// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
void AutoApplyPayToBill(long PayID, long PatientID, CString ClickedType, long BillID, BOOL bShiftToPatient /* = FALSE*/, BOOL bAdjustBalance /*= FALSE*/,
						BOOL bPromptToShift /* = TRUE*/, BOOL bWarnLocation /* = TRUE*/, BOOL bDisableAllowableCheck /*= FALSE*/, BOOL bIsERemitPosting/* = FALSE*/)
{
	_RecordsetPtr rsParam;
	_RecordsetPtr rs;
	COleCurrency cyPayment;
	_variant_t var;
	CString strSQL;
	long nActivePatientID = PatientID;

	//JJ - stop the user from being sneaky and applying a refund to a bill
	// (j.jones 2009-10-06 12:39) - PLID 27713 - parameterized
	rs = CreateParamRecordset("SELECT PayMethod FROM PaymentsT WHERE ID = {INT}",PayID);
	if(!rs->eof) {
		var = rs->Fields->Item["PayMethod"]->Value;
		if(var.vt==VT_I4 && var.lVal>=7) {
			AfxMessageBox("You can't apply a refund to a charge. This refund will be saved, but not applied.");
			return;
		}
	}
	rs->Close();

	// (j.jones 2009-10-06 12:39) - PLID 27713 - parameterized
 	rsParam = CreateParamRecordset("SELECT (PatientPaymentsQ.Amount + Sum(CASE WHEN(IncomingAppliesT.Amount Is Null) THEN 0 ELSE IncomingAppliesT.Amount END) - Sum(CASE WHEN(OutgoingAppliesT.Amount Is Null) THEN 0 ELSE OutgoingAppliesT.Amount END)) AS Amount, "
			"PaymentsT.InsuredPartyID "
			"FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
			") AS PatientPaymentsQ "
			"INNER JOIN PaymentsT ON PatientPaymentsQ.ID = PaymentsT.ID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, DestID FROM AppliesT WHERE DestID = {INT} GROUP BY DestID) AS IncomingAppliesT ON PaymentsT.ID = IncomingAppliesT.DestID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, SourceID FROM AppliesT WHERE SourceID = {INT} GROUP BY SourceID) AS OutgoingAppliesT ON PaymentsT.ID = OutgoingAppliesT.SourceID "
			"GROUP BY PatientPaymentsQ.ID, PaymentsT.InsuredPartyID, PatientPaymentsQ.Amount "
			"HAVING PatientPaymentsQ.ID = {INT}", nActivePatientID, PayID, PayID, PayID);

	if(rsParam->eof) {
		//should be impossible - it would mean the calling function is passing invalid values
		ASSERT(false);
		return;
	}
	
	// For insurance: see if the insurance is NULL. If not, force
	// an insurance apply
	cyPayment = AdoFldCurrency(rsParam, "Amount");
	var = rsParam->Fields->GetItem("InsuredPartyID")->Value;
	rsParam->Close();

	if (VarLong(var, 0) > 0) {
		// (j.jones 2009-10-06 12:39) - PLID 27713 - parameterized
		rs = CreateParamRecordset("SELECT RespTypeID FROM InsuredPartyT "
			"WHERE InsuredPartyT.PersonID = {INT}", VarLong(var));

		_variant_t varRespType;
		if(!rs->eof) {
			varRespType = rs->Fields->Item["RespTypeID"]->Value;
			//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
			// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables
			// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
			ApplyPayToBill(PayID, nActivePatientID, cyPayment, ClickedType, BillID, VarLong(var), FALSE, bShiftToPatient, bAdjustBalance,
				bPromptToShift, bWarnLocation, FALSE, bDisableAllowableCheck, bIsERemitPosting);
		}
		else {
			//shouldnt happen - they have an ins party but no record is found (or the record doesnt have an ins type)
			ASSERT(false);
		}
	}
	else {
		// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
		// (previously all the booleans were set to their default, so I continued that logic)
		// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
		ApplyPayToBill(PayID, nActivePatientID, cyPayment, ClickedType, BillID, -1, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, bDisableAllowableCheck, bIsERemitPosting);
	}
}



void AutoApplyPayToPay(long SrcPayID, long PatientID, CString ClickedType, long DestPayID)
{
	_RecordsetPtr rs;
	COleCurrency cyPayment;
	_variant_t var;
	CString strSQL, strMethod;
	long nActivePatientID = PatientID;

	// (j.jones 2009-10-06 12:39) - PLID 36423 - parameterized
	rs = CreateParamRecordset("SELECT PatientPaymentsQ.Amount, "
		"CASE WHEN Type=1 THEN Str(PaymentsT.PayMethod) ELSE CASE WHEN Type=2 THEN '6' ELSE '7' End End AS Method FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
		") AS PatientPaymentsQ INNER JOIN PaymentsT ON PatientPaymentsQ.ID = PaymentsT.ID WHERE PatientPaymentsQ.ID = {INT}", nActivePatientID, SrcPayID);
	
	cyPayment = AdoFldCurrency(rs, "Amount");
	
	strMethod = AdoFldString(rs, "Method");
	rs->Close();

	ApplyPayToBill(SrcPayID, nActivePatientID, cyPayment, ClickedType, DestPayID, -1 /*Patient Resp*/, TRUE);

	// If the source payment is a refund, the destination payment should
	// lose its prepayment.
/*		if (strMethod == "7") {
		strSQL.Format("UPDATE Payments SET PrePayment = FALSE WHERE PaymentID = %d", DestPayID);
		g_dbPractice.Execute(strSQL);
	}
*/	
}

BOOL GetPatientTotal(long PatientID, COleCurrency *cyTotal)
{
	COleCurrency cyCharges, cyPayAdjRef, cyZero = COleCurrency(0,0);
	// (j.jones 2009-10-06 12:39) - PLID 36423 - parameterized and combined recordsets
	_RecordsetPtr prs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS Total "
		"FROM ChargeRespT "
		"INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND InsuredPartyID IS NULL "
		""
		"SELECT Sum(LineItemT.Amount) AS Total "
		"FROM LineItemT "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND PaymentsT.InsuredPartyID = -1", PatientID, PatientID);
	if (prs->eof)
		cyCharges = COleCurrency(0,0);
	else
		cyCharges = AdoFldCurrency(prs, "Total", cyZero);

	prs = prs->NextRecordset(NULL);

	if (prs->eof)
		cyPayAdjRef = COleCurrency(0,0);
	else
		cyPayAdjRef = AdoFldCurrency(prs, "Total", cyZero);

	prs->Close();

	*cyTotal = cyCharges - cyPayAdjRef;
	return TRUE;
}

BOOL GetPatientInsuranceTotal(long PatientID, COleCurrency *cyTotal)
{
	COleCurrency cyCharges, cyPayAdjRef, cyZero = COleCurrency(0,0);
	// (j.jones 2009-10-06 12:39) - PLID 36423 - parameterized and combined recordsets
	_RecordsetPtr prs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS Total "
		"FROM ChargeRespT "
		"INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND InsuredPartyID IS NOT NULL "
		""
		"SELECT Sum(LineItemT.Amount) AS Total "
		"FROM LineItemT "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND PaymentsT.InsuredPartyID <> -1", PatientID, PatientID);
	if (prs->eof)
		cyCharges = COleCurrency(0,0);
	else
		cyCharges = AdoFldCurrency(prs, "Total", cyZero);

	prs = prs->NextRecordset(NULL);

	if (prs->eof)
		cyPayAdjRef = COleCurrency(0,0);
	else
		cyPayAdjRef = AdoFldCurrency(prs, "Total", cyZero);
	
	prs->Close();

	*cyTotal = cyCharges - cyPayAdjRef;
	return TRUE;
}

BOOL GetBillTotals(int iBillID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds, COleCurrency *cyInsResp)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	COleVariant var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted this recordset to be parameterized

	// Open with the new SQL
	rs = CreateParamRecordset("SELECT PatientBillsQ.ID, "	//FinTabGetBillTotalsQ.sql
		"Min(PatientBillsQ.Date) AS FirstOfDate, "
		"Min(PatientBillsQ.Description) AS FirstOfNotes, "
		"FinTabGetBillTotalSubQ.BillSubTotal AS BillTotal, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=1 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfPayAmount, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=2 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfAdjAmount, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=3 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfRefAmount, "
		"FinTabGetBillTotalSubQ.InsRespTotal AS InsResp "
		"FROM ((((SELECT BillsT.* "
		"FROM BillsT "
		"WHERE (((BillsT.PatientID)={INT}) AND ((BillsT.Deleted)=0)) "
		") AS PatientBillsQ INNER JOIN (SELECT ChargesT.BillID, "
		"Sum(ChargeRespT.Amount) AS BillSubTotal, "
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 THEN ChargeRespT.Amount ELSE 0 End) AS InsRespTotal "
		"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10)) "
		") AS PatientChargesQ INNER JOIN (ChargesT LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
		"GROUP BY ChargesT.BillID "
		"HAVING (((ChargesT.BillID)={INT})) "
		") AS FinTabGetBillTotalSubQ ON PatientBillsQ.ID = FinTabGetBillTotalSubQ.BillID) LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10)) "
		") AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID) LEFT JOIN (SELECT PatientAppliesQ.* "
		"FROM (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"WHERE (((PatientPaymentsQ.ID) Is Not Null)) "
		") AS PatientAppliesQ "
		"WHERE (((PatientAppliesQ.PointsToPayments)=0)) "
		") AS PatientChargeAppliesQ ON PatientChargesQ.ID = PatientChargeAppliesQ.DestID) LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
		") AS PatientPaymentsQ ON PatientChargeAppliesQ.SourceID = PatientPaymentsQ.ID "
		"GROUP BY PatientBillsQ.ID, FinTabGetBillTotalSubQ.BillSubTotal, FinTabGetBillTotalSubQ.InsRespTotal "
		"HAVING (((PatientBillsQ.ID)={INT})); ", nActivePatientID, nActivePatientID, iBillID, nActivePatientID, nActivePatientID, nActivePatientID, iBillID);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charges
	if (cyCharges) {
		*cyCharges = AdoFldCurrency(rs, "BillTotal", COleCurrency(0,0));
		RoundCurrency(*cyCharges);
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}

	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}

	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	// Get total insurance responsibility
	if (cyInsResp) {
		*cyInsResp = AdoFldCurrency(rs, "InsResp", COleCurrency(0,0));
	}

	rs->Close();

	return TRUE;
}

// (j.jones 2006-12-28 14:13) - PLID 23160 - added revenue code totals functions
BOOL GetRevenueCodeTotals(long nRevenueCodeID, long nBillID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds, COleCurrency *cyInsResp)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	COleVariant var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	// Open with the new SQL
	// (j.jones 2008-04-21 16:57) - PLID 29737 - ensured the charges were grouped properly so we don't double values
	// when multiple charge resps exist
	rs = CreateParamRecordset("SELECT PatientBillsQ.ID, "	//FinTabGetBillTotalsQ.sql
		"Min(PatientBillsQ.Date) AS FirstOfDate, "
		"Min(PatientBillsQ.Description) AS FirstOfNotes, "
		"FinTabGetBillTotalSubQ.BillSubTotal AS BillTotal, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=1 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfPayAmount, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=2 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfAdjAmount, "
		"Sum(CASE WHEN PatientChargeAppliesQ.Type=3 And (PatientPaymentsQ.InsuredPartyID Is Null Or PatientPaymentsQ.InsuredPartyID<1) THEN PatientChargeAppliesQ.Amount ELSE 0 End) AS SumOfRefAmount, "
		"FinTabGetBillTotalSubQ.InsRespTotal AS InsResp "
		"FROM ((((SELECT BillsT.* "
		"FROM BillsT "
		"WHERE BillsT.PatientID = {INT} AND BillsT.Deleted = 0 "
		") AS PatientBillsQ INNER JOIN (SELECT ChargesT.BillID, "
		"Sum(ChargeRespT.Amount) AS BillSubTotal, "
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 THEN ChargeRespT.Amount ELSE 0 End) AS InsRespTotal "
		"FROM ((SELECT LineItemT.ID, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
		"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
		"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
		"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
		"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
		"GROUP BY LineItemT.ID, ChargesT.BillID, ChargesT.DoctorsProviders "
		") AS PatientChargesQ INNER JOIN (ChargesT LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
		"GROUP BY ChargesT.BillID "
		"HAVING ChargesT.BillID = {INT} "
		") AS FinTabGetBillTotalSubQ ON PatientBillsQ.ID = FinTabGetBillTotalSubQ.BillID) LEFT JOIN (SELECT LineItemT.ID, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
		"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
		"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
		"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
		"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
		"GROUP BY LineItemT.ID, ChargesT.BillID, ChargesT.DoctorsProviders "
		") AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID) LEFT JOIN (SELECT PatientAppliesQ.* "
		"FROM (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <=3 "
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"WHERE PatientPaymentsQ.ID IS NOT NULL "
		") AS PatientAppliesQ "
		"WHERE PatientAppliesQ.PointsToPayments = 0 "
		") AS PatientChargeAppliesQ ON PatientChargesQ.ID = PatientChargeAppliesQ.DestID) LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
		") AS PatientPaymentsQ ON PatientChargeAppliesQ.SourceID = PatientPaymentsQ.ID "
		"GROUP BY PatientBillsQ.ID, FinTabGetBillTotalSubQ.BillSubTotal, FinTabGetBillTotalSubQ.InsRespTotal "
		"HAVING PatientBillsQ.ID = {INT}", nActivePatientID, nActivePatientID, nRevenueCodeID, nBillID, nActivePatientID, nActivePatientID, nRevenueCodeID, nActivePatientID, nBillID);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charges
	if (cyCharges) {
		*cyCharges = AdoFldCurrency(rs, "BillTotal", COleCurrency(0,0));
		RoundCurrency(*cyCharges);
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}

	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}

	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	// Get total insurance responsibility
	if (cyInsResp) {
		*cyInsResp = AdoFldCurrency(rs, "InsResp", COleCurrency(0,0));
	}

	rs->Close();

	return TRUE;
}

// (j.jones 2011-07-22 12:07) - PLID 42231 - removed the old GetBillInsuranceTotals, in favor of GetBillInsuranceTotals_InsParty, which I then renamed to the old name
BOOL GetBillInsuranceTotals(int iBillID, long PatientID, int nInsPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds)
{
	// (b.cardillo 2011-06-17 16:30) - PLID 39176 - Changed iInsType to nInsPartyID since callers can often 
	// give that to us from memory.  Also, since nothing was preventing iInsType from being -1, there's no 
	// guarantee that we would even return the correct results in that case because an inactive insured 
	// party would just be selected at random.

	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	int iGuarantorID = nInsPartyID;

	// Open with the new SQL
	// (j.jones 2009-10-06 12:39) - PLID 36423 - parameterized
	rs = CreateParamRecordset("SELECT Sum(PatientChargeInsTotalsQ.InsResp) AS InsuranceResp, "		//PatientBillInsTotalsQ.sql
		"Sum(PatientChargeInsTotalsQ.PayAmount) AS SumOfPayAmount, "
		"Sum(PatientChargeInsTotalsQ.AdjAmount) AS SumOfAdjAmount, "
		"Sum(PatientChargeInsTotalsQ.RefAmount) AS SumOfRefAmount, "
		"PatientBillsQ.ID "
		"FROM (SELECT BillsT.* "
		"FROM BillsT "
		"WHERE (((BillsT.PatientID)={INT}) AND ((BillsT.Deleted)=0)) "
		") AS PatientBillsQ LEFT JOIN (SELECT CASE WHEN ChargeRespSumQ.Amount Is Null THEN 0 ELSE ChargeRespSumQ.Amount End AS InsResp, "
		"Sum(CASE WHEN PatientAppliesQ.Type=1 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS PayAmount, "
		"Sum(CASE WHEN PatientAppliesQ.Type=2 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS AdjAmount, "
		"Sum(CASE WHEN PatientAppliesQ.Type=3 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS RefAmount, "
		"PatientChargesQ.BillID, "
		"PatientChargesQ.ID "
		"FROM (((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) INNER JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=10)) "
		") AS PatientChargesQ ON PatientsT.PersonID = PatientChargesQ.PatientID) INNER JOIN (ChargesT LEFT JOIN (SELECT * FROM ChargeRespT WHERE InsuredPartyID={INT}) AS ChargeRespSumQ ON ChargesT.ID = ChargeRespSumQ.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) LEFT JOIN (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3)) "
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"WHERE (((PatientPaymentsQ.ID) Is Not Null)) "
		") AS PatientAppliesQ ON ChargesT.ID = PatientAppliesQ.DestID "
		"GROUP BY ChargeRespSumQ.Amount, PatientChargesQ.BillID, PatientChargesQ.ID "
		") AS PatientChargeInsTotalsQ ON PatientBillsQ.ID = PatientChargeInsTotalsQ.BillID "
		"GROUP BY PatientBillsQ.ID "
		"HAVING (((PatientBillsQ.ID)={INT}))", nActivePatientID, iGuarantorID, iGuarantorID, iGuarantorID, iGuarantorID, iGuarantorID, iGuarantorID, nActivePatientID, iGuarantorID, nActivePatientID, iBillID);


	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charges
	if (cyTotalResp) {
		*cyTotalResp = AdoFldCurrency(rs, "InsuranceResp", COleCurrency(0,0));
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}
	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}
	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	rs->Close();

	return TRUE;
}

// (j.jones 2013-03-25 17:45) - PLID 55686 - added function to get the insured party ID with the highest balance
// on this bill, will return -1 if it is the patient, also will return -1 if the highest balance is zero
long GetBillInsuredPartyIDWithHighestBalance(long nBillID)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyID "
		"FROM "
		"(SELECT BillsT.ID AS BillID, RespQ.InsuredPartyID, RespQ.TotalCharges, "
		"Convert(money, SUM(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END)) AS TotalPays "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"INNER JOIN (SELECT ChargeRespT.ID, ChargeID, SUM(ChargeRespT.Amount) AS TotalCharges, "
			"CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID "
			"FROM ChargeRespT "
			"LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID "
			") RespQ ON ChargesT.ID = RespQ.ChargeID "
		"LEFT JOIN (SELECT AppliesT.RespID, SUM(AppliesT.Amount) AS Amount "
			"FROM PaymentsT "
			"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null "
			"GROUP BY AppliesT.RespID"
			") PaysQ ON RespQ.ID = PaysQ.RespID "
		"WHERE LineItemT.Deleted = 0 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY BillsT.ID, RespQ.InsuredPartyID, RespQ.TotalCharges "
		") AS SubQ "
		"WHERE (TotalCharges - TotalPays) > CONVERT(money,'0') "
		"GROUP BY BillID, InsuredPartyID "
		"HAVING BillID = {INT} "
		"ORDER BY Sum(TotalCharges - TotalPays) DESC", nBillID);

	if(!rs->eof) {
		//if null, it's patient resp., so return -1
		return VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);
	}
	rs->Close();
	
	//nothing found, perhaps the balance is zero,
	//so just return -1 for patient responsibility
	return -1;
}

// (j.jones 2013-03-25 17:45) - PLID 55686 - added function to get the insured party ID with the highest balance
// on this charge, will return -1 if it is the patient, also will return -1 if the highest balance is zero
long GetChargeInsuredPartyIDWithHighestBalance(long nChargeID)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyID "
		"FROM "
		"(SELECT ChargesT.ID AS ChargeID, RespQ.InsuredPartyID, RespQ.TotalCharges, "
		"Convert(money, SUM(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END)) AS TotalPays "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"INNER JOIN (SELECT ChargeRespT.ID, ChargeID, SUM(ChargeRespT.Amount) AS TotalCharges, "
			"CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID "
			"FROM ChargeRespT "
			"LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID "
			") RespQ ON ChargesT.ID = RespQ.ChargeID "
		"LEFT JOIN (SELECT AppliesT.RespID, SUM(AppliesT.Amount) AS Amount "
			"FROM PaymentsT "
			"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null "
			"GROUP BY AppliesT.RespID"
			") PaysQ ON RespQ.ID = PaysQ.RespID "
		"WHERE LineItemT.Deleted = 0 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY ChargesT.ID, RespQ.InsuredPartyID, RespQ.TotalCharges "
		") AS SubQ "
		"WHERE (TotalCharges - TotalPays) > CONVERT(money,'0') "
		"AND ChargeID = {INT} "
		"ORDER BY (TotalCharges - TotalPays) DESC", nChargeID);

	if(!rs->eof) {
		//if null, it's patient resp., so return -1
		return VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);
	}
	rs->Close();
	
	//nothing found, perhaps the balance is zero,
	//so just return -1 for patient responsibility
	return -1;
}

// (j.jones 2011-03-08 10:48) - PLID 41877 - renamed the pay/adj/ref parameters to reflect what they really mean
BOOL GetChargeTotals(int iChargeID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPatientPayments, COleCurrency *cyPatientAdjustments, COleCurrency *cyPatientRefunds, COleCurrency *cyInsResp)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	try {
	
		// (j.jones 2009-10-06 12:47) - PLID 36423 - parameterized and combined
		rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS ChargeTotal, "
			"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 THEN ChargeRespT.Amount ELSE 0 END) AS InsResp "
			"FROM ChargeRespT WHERE ChargeID = {INT} "
			""
			"SELECT Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS SumOfPayAmount, "
			"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS SumOfAdjAmount, "
			"Sum(CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END) AS SumOfRefAmount "
			"FROM AppliesT "
			"LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
			"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID < 1) "
			"AND AppliesT.DestID = {INT}", iChargeID, iChargeID);

		if (rs->eof) {
			rs->Close();
			return FALSE;
		}

		// Get total charge
		if (cyCharges) {
			*cyCharges = AdoFldCurrency(rs, "ChargeTotal", COleCurrency(0,0));
		}

		// Get total insurance responsibility
		if (cyInsResp) {
			*cyInsResp = AdoFldCurrency(rs, "InsResp", COleCurrency(0,0));;
		}

		rs = rs->NextRecordset(NULL);

		if (rs->eof) {
			rs->Close();
			return FALSE;
		}

		// Get total patient payments
		if (cyPatientPayments) {
			*cyPatientPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
		}

		// Get total patient adjustments
		if (cyPatientAdjustments) {
			*cyPatientAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
		}

		// Get total patient refunds
		if (cyPatientRefunds) {
			*cyPatientRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
		}	

		rs->Close();

	}NxCatchAllCall("Error in GetChargeTotals", { return FALSE; });
	
	return TRUE;
}

// (j.jones 2006-12-28 14:03) - PLID 23160 - added revenue code totals functions
BOOL GetRevenueCodeInsuranceTotals(long nRevenueCodeID, long nBillID, long nPatientID, long nInsuredPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	CString str, strSQL;
	long nActivePatientID = nPatientID;

	// Open with the new SQL
	// (j.jones 2008-04-21 16:57) - PLID 29737 - ensured the charges were grouped properly so we don't double values
	// when multiple charge resps exist
	rs = CreateParamRecordset("SELECT Sum(PatientChargeInsTotalsQ.InsResp) AS InsuranceResp, "		//PatientBillInsTotalsQ.sql
		"Sum(PatientChargeInsTotalsQ.PayAmount) AS SumOfPayAmount, "
		"Sum(PatientChargeInsTotalsQ.AdjAmount) AS SumOfAdjAmount, "
		"Sum(PatientChargeInsTotalsQ.RefAmount) AS SumOfRefAmount, "
		"PatientBillsQ.ID "
		"FROM (SELECT BillsT.* "
		"FROM BillsT "
		"WHERE BillsT.PatientID = {INT} AND BillsT.Deleted = 0 "
		") AS PatientBillsQ LEFT JOIN (SELECT CASE WHEN ChargeRespSumQ.Amount Is Null THEN 0 ELSE ChargeRespSumQ.Amount End AS InsResp, "
		"Sum(CASE WHEN PatientAppliesQ.Type=1 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS PayAmount, "
		"Sum(CASE WHEN PatientAppliesQ.Type=2 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS AdjAmount, "
		"Sum(CASE WHEN PatientAppliesQ.Type=3 And PatientAppliesQ.InsuredPartyID={INT} And {INT}>0 THEN PatientAppliesQ.Amount ELSE 0 End) AS RefAmount, "
		"PatientChargesQ.BillID, "
		"PatientChargesQ.ID "
		"FROM (((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) "
		"INNER JOIN (SELECT LineItemT.ID, LineItemT.PatientID, ChargesT.BillID, ChargesT.DoctorsProviders "
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
		"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
		"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
		"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
		"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
		"GROUP BY LineItemT.ID, LineItemT.PatientID, ChargesT.BillID, ChargesT.DoctorsProviders "
		") AS PatientChargesQ ON PatientsT.PersonID = PatientChargesQ.PatientID) INNER JOIN (ChargesT LEFT JOIN (SELECT * FROM ChargeRespT WHERE InsuredPartyID = {INT}) AS ChargeRespSumQ ON ChargesT.ID = ChargeRespSumQ.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) LEFT JOIN (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"WHERE PatientPaymentsQ.ID Is Not Null "
		") AS PatientAppliesQ ON ChargesT.ID = PatientAppliesQ.DestID "
		"GROUP BY ChargeRespSumQ.Amount, PatientChargesQ.BillID, PatientChargesQ.ID "
		") AS PatientChargeInsTotalsQ ON PatientBillsQ.ID = PatientChargeInsTotalsQ.BillID "
		"GROUP BY PatientBillsQ.ID "
		"HAVING PatientBillsQ.ID = {INT}", nActivePatientID, nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nInsuredPartyID, nActivePatientID, nRevenueCodeID, nInsuredPartyID, nActivePatientID, nBillID);


	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charges
	if (cyTotalResp) {
		*cyTotalResp = AdoFldCurrency(rs, "InsuranceResp", COleCurrency(0,0));
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}
	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}
	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	rs->Close();

	return TRUE;
}

BOOL GetChargeInsuranceTotals(int iChargeID, long PatientID, int nInsPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds)
{
	//DRT 5/8/03 - Changed iInsType to nInsPartyID.  The calculation is now based off an insured party, not
	//		a type (which is just used to lookup the ins party).

	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	CString strSQL;
	long nInsID = nInsPartyID;
	long nActivePatientID = PatientID;
	
	// (j.jones 2009-10-06 12:47) - PLID 36423 - parameterized and combined
	rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS InsuranceResp "
		"FROM ChargeRespT "
		"WHERE ChargeID = {INT} AND InsuredPartyID = {INT} "
		""
		"SELECT Sum(CASE WHEN LineItemT.Type=1 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfPayAmount, "
		"Sum(CASE WHEN LineItemT.Type=2 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfAdjAmount, "
		"Sum(CASE WHEN LineItemT.Type=3 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfRefAmount "
		"FROM AppliesT "
		"LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE AppliesT.DestID = {INT}",
		iChargeID, nInsID,
		nInsID, nInsID, nInsID, nInsID, nInsID, nInsID, iChargeID);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charge
	if (cyTotalResp) {
		*cyTotalResp = AdoFldCurrency(rs, "InsuranceResp", COleCurrency(0,0));
	}

	rs = rs->NextRecordset(NULL);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}

	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}

	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	rs->Close();
	
	return TRUE;
}

BOOL GetInactiveInsTotals(int InsuredPartyID, int BillID, int ChargeID, long PatientID, COleCurrency &cyResp, COleCurrency &cyApplies)
{	
	try {

		COleVariant var;
		CString str;

		cyResp = cyApplies = COleCurrency(0,0);

		////////////////////////////////////////////////////////////////////////
		// Get the total responsibilities

		//DRT 5/27/03 - Added a WHERE on InsuredPartyID as well - for a LONG time this has been returning the total inactive resp - but
		//		a patient may have dozens of inactive responsibilities.  If we've got the insPartyID, why not use it and just return
		//		the correct resp, not just all of it!

		if (BillID != -1) {
			// (j.jones 2009-10-06 13:33) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS SumOfAmount, ChargesT.BillID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE BillsT.PatientID = {INT} AND ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND InsuredPartyT.RespTypeID = -1 AND InsuredPartyT.PersonID = {INT} "
				"GROUP BY ChargesT.BillID", PatientID, BillID, InsuredPartyID);
			if(!rs->eof) {
				var = rs->Fields->Item["SumOfAmount"]->Value;
				if (var.vt != VT_NULL)
					cyResp = var.cyVal;
			}
			rs->Close();
		}
		else {
			// (j.jones 2009-10-06 13:33) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS SumOfAmount, ChargesT.ID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE BillsT.PatientID = {INT} AND ChargesT.ID = {INT} AND LineItemT.Deleted = 0 AND InsuredPartyT.RespTypeID = -1 AND InsuredPartyT.PersonID = {INT} "
				"GROUP BY ChargesT.ID", PatientID, ChargeID, InsuredPartyID);
			if(!rs->eof) {
				var = rs->Fields->Item["SumOfAmount"]->Value;
				if (var.vt != VT_NULL)
					cyResp = var.cyVal;
			}
			rs->Close();
		}


		////////////////////////////////////////////////////////////////////////
		// Get all applies (From where payment insurance ID is InsuredPartyID)
		if (BillID != -1) {
			// (j.jones 2009-10-06 13:33) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Sum([_PatientChargeAppliesQ].Amount) AS SumOfAmount, [_PatientChargesQ].BillID, [_PatientPaymentsQ].InsuredPartyID FROM "
				"((SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS _PatientPaymentsQ "
				"INNER JOIN (SELECT [_PatientAppliesQ].* FROM (SELECT AppliesT.*, [_PatientPaymentsQ].Type, [_PatientPaymentsQ].Description AS ApplyDesc, [_PatientPaymentsQ].InsuredPartyID "
				"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE (((LineItemT.PatientID)={INT}) "
				"AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS _PatientPaymentsQ ON AppliesT.SourceID = [_PatientPaymentsQ].ID WHERE ((([_PatientPaymentsQ].ID) Is Not Null))) AS _PatientAppliesQ "
				"WHERE ((([_PatientAppliesQ].PointsToPayments)=0))) AS _PatientChargeAppliesQ ON [_PatientPaymentsQ].ID = [_PatientChargeAppliesQ].SourceID) LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
				"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS _PatientChargesQ ON [_PatientChargeAppliesQ].DestID = [_PatientChargesQ].ID "
				"WHERE [_PatientChargesQ].BillID={INT} AND [_PatientPaymentsQ].InsuredPartyID={INT} "
				"GROUP BY [_PatientChargesQ].BillID, [_PatientPaymentsQ].InsuredPartyID",PatientID,PatientID,PatientID,BillID,InsuredPartyID);
			if(!rs->eof) {
				var = rs->Fields->Item["SumOfAmount"]->Value;
				if (var.vt != VT_NULL)
					cyApplies = var.cyVal;
			}
			rs->Close();
		}
		else {
			// (j.jones 2009-10-06 13:33) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Sum([_PatientChargeAppliesQ].Amount) AS SumOfAmount, [_PatientPaymentsQ].InsuredPartyID FROM ((SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS _PatientPaymentsQ INNER JOIN (SELECT [_PatientAppliesQ].* FROM (SELECT AppliesT.*, [_PatientPaymentsQ].Type, [_PatientPaymentsQ].Description AS ApplyDesc, [_PatientPaymentsQ].InsuredPartyID "
				"FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS _PatientPaymentsQ ON AppliesT.SourceID = [_PatientPaymentsQ].ID "
				"WHERE ((([_PatientPaymentsQ].ID) Is Not Null))) AS _PatientAppliesQ WHERE ((([_PatientAppliesQ].PointsToPayments)=0))) AS _PatientChargeAppliesQ ON [_PatientPaymentsQ].ID = [_PatientChargeAppliesQ].SourceID) LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS _PatientChargesQ ON [_PatientChargeAppliesQ].DestID = [_PatientChargesQ].ID "
				"WHERE (([_PatientChargesQ].ID)={INT}) AND ((([_PatientPaymentsQ].InsuredPartyID)={INT})) "
				"GROUP BY [_PatientPaymentsQ].InsuredPartyID",PatientID,PatientID,PatientID,ChargeID,InsuredPartyID);
			if(!rs->eof) {
				var = rs->Fields->Item["SumOfAmount"]->Value;
				if (var.vt != VT_NULL)
					cyApplies = var.cyVal;
			}
			rs->Close();
		}

	}NxCatchAll("Error in GetInactiveInsTotals");

	return TRUE;
}

// (j.jones 2006-12-28 14:04) - PLID 23160 - added revenue code totals functions
BOOL GetRevenueCodeInactiveInsTotals(int InsuredPartyID, long nBillID, long nRevenueCodeID, long PatientID, COleCurrency &cyResp, COleCurrency &cyApplies)
{	
	COleVariant var;

	cyResp = cyApplies = COleCurrency(0,0);

	////////////////////////////////////////////////////////////////////////
	// Get the total responsibilities

	//DRT 5/27/03 - Added a WHERE on InsuredPartyID as well - for a LONG time this has been returning the total inactive resp - but
	//		a patient may have dozens of inactive responsibilities.  If we've got the insPartyID, why not use it and just return
	//		the correct resp, not just all of it!
	
	try
	{
		// (j.jones 2009-10-06 13:35) - PLID 36423 - parameterized and combined
		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS SumOfAmount, ChargesT.BillID "
			"FROM BillsT "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "			
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "			
			"WHERE BillsT.PatientID = {INT} AND ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND InsuredPartyT.RespTypeID = -1 AND InsuredPartyT.PersonID = {INT} "
			"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
			"GROUP BY ChargesT.BillID "
			""
			"SELECT Sum([_PatientChargeAppliesQ].Amount) AS SumOfAmount, [_PatientChargesQ].BillID, [_PatientPaymentsQ].InsuredPartyID FROM "
			"((SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >=1 AND LineItemT.Type <= 3"
			") AS _PatientPaymentsQ "
			"INNER JOIN "
			"(SELECT [_PatientAppliesQ].* "
			"FROM (SELECT AppliesT.*, [_PatientPaymentsQ].Type, [_PatientPaymentsQ].Description AS ApplyDesc, [_PatientPaymentsQ].InsuredPartyID "
			"FROM AppliesT "
			"LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
			") AS _PatientPaymentsQ ON AppliesT.SourceID = [_PatientPaymentsQ].ID WHERE [_PatientPaymentsQ].ID Is Not Null "
			") AS _PatientAppliesQ "
			"WHERE [_PatientAppliesQ].PointsToPayments = 0 "
			") AS _PatientChargeAppliesQ ON [_PatientPaymentsQ].ID = [_PatientChargeAppliesQ].SourceID) "
			"LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
			"FROM LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "			
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "	
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"AND (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
			") AS _PatientChargesQ ON [_PatientChargeAppliesQ].DestID = [_PatientChargesQ].ID "
			"WHERE [_PatientChargesQ].BillID = {INT} AND [_PatientPaymentsQ].InsuredPartyID = {INT} "
			"GROUP BY [_PatientChargesQ].BillID, [_PatientPaymentsQ].InsuredPartyID",
			PatientID, nBillID, InsuredPartyID, nRevenueCodeID,
			PatientID, PatientID, PatientID, nRevenueCodeID, nBillID, InsuredPartyID);
		if (!rs->eof) {
			var = rs->Fields->Item["SumOfAmount"]->Value;
			if (var.vt != VT_NULL)
				cyResp = var.cyVal;
		}

		rs = rs->NextRecordset(NULL);

		////////////////////////////////////////////////////////////////////////
		// Get all applies (From where payment insurance ID is InsuredPartyID)
		if (!rs->eof) {
			var = rs->Fields->Item["SumOfAmount"]->Value;
			if (var.vt != VT_NULL)
				cyApplies = var.cyVal;
		}

		rs->Close();
	}NxCatchAll("Error getting insurance totals: ");

	return TRUE;
}

BOOL GetPayAdjRefTotals(int iID, long PatientID, COleCurrency &cyInitialAmount, COleCurrency &cyOutgoingApplies, COleCurrency &cyIncomingApplies)
{
	_RecordsetPtr rs;
	_variant_t var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	//(e.lally 2007-04-03) PLID 25481 - Combined the 3 recordsets into 1.
	// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
	rs = CreateParamRecordset("SELECT "
		//Inititial Amount
		"(SELECT Amount FROM "
		"	(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"	 FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"	 WHERE (LineItemT.PatientID={INT} AND LineItemT.Deleted=0 AND (LineItemT.Type>=1 And LineItemT.Type<=3)) "
		") AS PatientPaymentsQ "
		"WHERE PatientPaymentsQ.ID = {INT} "
		") AS InitialAmount, "
		//Outgoing Applies
		"(SELECT Sum(PatientAppliesQ.Amount) AS OutgoingApplies "
		"FROM	(SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"		 FROM AppliesT "
		"		 LEFT JOIN	(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"					 FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"					 WHERE (LineItemT.PatientID={INT} AND LineItemT.Deleted=0 AND "
		"					 (LineItemT.Type>=1 And LineItemT.Type<=3)) "
		"					) AS PatientPaymentsQ "
		"		ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"		WHERE PatientPaymentsQ.ID Is Not Null "
		"		) AS PatientAppliesQ "
		"WHERE PatientAppliesQ.SourceID={INT} "
		") AS OutgoingApplies, "
		//Incoming Applies
		"(SELECT Sum(PatientAppliesQ.Amount) AS IncomingApplies "
		" FROM	(SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
		"		 FROM AppliesT "
		"		 LEFT JOIN	(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
		"					 FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"					 WHERE (LineItemT.PatientID={INT} AND LineItemT.Deleted=0 AND "
		"					 (LineItemT.Type>=1 And LineItemT.Type<=3)) "
		"					) AS PatientPaymentsQ "
		"		 ON AppliesT.SourceID = PatientPaymentsQ.ID "
		"		 WHERE PatientPaymentsQ.ID Is Not Null "
		"		) AS PatientAppliesQ "
		" WHERE PatientAppliesQ.PointsToPayments=1 AND PatientAppliesQ.DestID={INT}"
		") AS IncomingApplies ",
		nActivePatientID, iID,
		nActivePatientID, iID,
		nActivePatientID, iID);
	
	//This should now not be possible, but will check it for safety sake.
	if (rs->eof) {
		rs->Close();
		return FALSE;
	}
	// Make sure the payment exists
	_variant_t varInitialAmount = rs->Fields->Item["InitialAmount"]->Value;
	if(varInitialAmount.vt == VT_NULL){
		//The payment did not exist
		rs->Close();
		return FALSE;
	}
	//Get the initial amount
	cyInitialAmount = VarCurrency(varInitialAmount);
	// Get the the outgoing applies 
	cyOutgoingApplies = AdoFldCurrency(rs, "OutgoingApplies", COleCurrency(0, 0));
	// Get the incoming applies (PointsToPayments=TRUE) 
	cyIncomingApplies = AdoFldCurrency(rs, "IncomingApplies", COleCurrency(0,0));

	rs->Close();

	return TRUE;
}

BOOL DeleteBill(int iBillID, BOOL bDeleteCharges /*TRUE*/)
{
	//do not check for permissions inside this function, do it before you call it

	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Deleting Bill %d", iBillID);

	try {
		_RecordsetPtr rs;
		CString str;
		int iChargeID;
		long AuditID;
		short nType; // 1=bill, 2=quote

		long nPatientID = 0;
		CString strBillInfo = "";

		// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT EntryType, PatientID, Description, Date FROM BillsT WHERE ID = {INT}",iBillID);
		if(!rs->eof) {
			CString strDesc;
			COleDateTime dt;
			strDesc = AdoFldString(rs, "Description","(No Description)");
			dt = AdoFldDateTime(rs, "Date");
			strBillInfo.Format("Description: '%s', Date: %s",strDesc,FormatDateTimeForInterface(dt,dtoDate));
			nPatientID = AdoFldLong(rs, "PatientID",0);
			// m.carlson 4/28/2005 PL 16372 - Check if we're deleting a bill or quote
			nType = AdoFldByte(rs, "EntryType");
		}
		rs->Close();

		//(e.lally 2007-04-04) PLID 25487 - Updated delete/update statements to be in a batch.
		CString strBatchSql = BeginSqlBatch();
		
		AddStatementToSqlBatch(strBatchSql,"UPDATE BillsT SET Deleted = -1, DeleteDate = GetDate(), DeletedBy = %li WHERE ID = %d", GetCurrentUserID(), iBillID);
		AddStatementToSqlBatch(strBatchSql,"UPDATE PaymentsT SET QuoteID = NULL WHERE QuoteID = %li",iBillID);
		// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
		//AddStatementToSqlBatch(strBatchSql,"DELETE FROM HCFAT WHERE BillID = %d", iBillID);
		AddStatementToSqlBatch(strBatchSql,"DELETE FROM HCFATrackT WHERE BillID = %d", iBillID);

		// (j.dinatale 2012-03-23 12:09) - PLID 49146 - set the bill ID in GlassesOrderT to NULL when a bill is deleted
		AddStatementToSqlBatch(strBatchSql, "UPDATE GlassesOrderT SET BillID = NULL WHERE BillID = %d", iBillID);

		//Is this a quote?
		// (j.jones 2009-10-06 15:53) - PLID 36423 - removed unnecessary recordset
		if(nType == 1) {
			//Unapply the event that may be tracked.
			PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_BillCreated, iBillID);

			// (j.dinatale 2012-07-10 17:25) - PLID 51468 - delete from ProcInfoBillsT
			AddStatementToSqlBatch(strBatchSql,"DELETE FROM ProcInfoBillsT WHERE BillID = %li", iBillID);
			AddStatementToSqlBatch(strBatchSql,"UPDATE ProcInfoT SET BillID = NULL WHERE BillID = %li", iBillID);

			// (j.jones 2007-05-07 10:19) - PLID 25906 - delete from the last bill date array
			// if it is present in the array
			DeleteLastBillDate(iBillID);

		}
		else {
			PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_QuoteCreated, iBillID);
			AddStatementToSqlBatch(strBatchSql,"DELETE FROM ProcInfoQuotesT WHERE QuoteID = %li", iBillID);
			AddStatementToSqlBatch(strBatchSql,"UPDATE ProcInfoT SET ActiveQuoteID = NULL WHERE ActiveQuoteID = %li", iBillID);

			// (j.jones 2007-05-07 10:20) - PLID 25906 - delete from the last quote date array
			// if it is present in the array
			DeleteLastQuoteDate(iBillID);
		}

		// (j.jones 2005-08-19 11:53) - update repeat packages accordingly (DeleteCharge will handle the dollar amount)
		if(nType == 1) {
			// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT QuoteID FROM BilledQuotesT WHERE BillID = {INT}", iBillID);
			while(!rs->eof) {
				long nBilledQuote = AdoFldLong(rs, "QuoteID",-1);
				// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
				_RecordsetPtr rsPackage = CreateParamRecordset("SELECT QuoteID FROM PackagesT WHERE QuoteID = {INT} AND Type = 1",nBilledQuote);
				if(!rsPackage->eof) {
					//we're deleting a bill linked to a repeat package
					AddStatementToSqlBatch(strBatchSql,"UPDATE PackagesT SET CurrentCount = CurrentCount + 1 WHERE QuoteID = %li",nBilledQuote);
				}
				rsPackage->Close();

				rs->MoveNext();
			}
			rs->Close();
		}

		/////////////////////////////////////////////
		// Delete charges and free inventory
		/////////////////////////////////////////////

		//(e.lally 2007-04-04) PLID 25487 - Put in a check to see if we need to delete the charges
			//before we commit our bill deletion batch. Defaulted to true. This could be false in
			//situations like a delete charge initiating the delete bill when it is the last charge.
		if(bDeleteCharges){
			//Only select non-deleted charges
			// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
			rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT}", iBillID);

			// Go through each charge
			while (!rs->eof) {
				iChargeID = rs->Fields->Item["ID"]->Value.lVal;

				//DRT 4/30/2004 - PLID 11797 - We can't just ignore the return value here.  If the 
				//charge isn't deleted, we cannot continue on with this transaction, 
				//it needs to be rolled back, otherwise you can get a deleted bill w/
				//non-deleted charges.
				if(!DeleteCharge(iChargeID,iBillID,FALSE,FALSE)) {
					//failed to delete the charge
					//(e.lally 2007-04-04) PLID 25487 - We have not committed yet, so just return
					return FALSE;	//no reason to continue
				}

				rs->MoveNext();
			}
			rs->Close();
		}

		//(e.lally 2007-04-04) PLID 25487 - Commit the batch of statements
		ExecuteSqlBatch(strBatchSql);
		AuditID = BeginNewAuditEvent();
		// m.carlson 4/28/2005 PL 16372 - if nType is 2, audit as deleting a quote, else assume a bill
		//(e.lally 2007-04-04) PLID 25487 - Moved auditing down here to after our commit
		if (nType == 2)
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID),AuditID,aeiQuoteDeleted,iBillID,strBillInfo,"<Deleted>",aepMedium,aetDeleted);
		else
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID),AuditID,aeiBillDeleted,iBillID,strBillInfo,"<Deleted>",aepHigh,aetDeleted);

		// (a.walling 2007-05-21 11:22) - PLID 26079 - Update reward points
		try {
			Rewards::UnapplyBillAll(iBillID, Rewards::erdrRemoved);
		} NxCatchAll("Error updating rewards points: DeleteBill");

		CClient::RefreshTable(NetUtils::PatBal, nPatientID);
		return TRUE;
	}NxCatchAll("Error 100: DeleteBill");
	return FALSE;
}

BOOL DeleteCharge(long iChargeID, long iBillID, BOOL boUpdateLineIDs, BOOL boDeleteBill)
{
	_RecordsetPtr rs;
	_variant_t varSrgyTrackID, var;
	//(e.lally 2007-04-04) PLID 25487 - Use an audit transaction to delay the commit
	long nAuditTransactionID = -1;
	bool boInsurance = FALSE;
	CString str;
	long nPatientID = 0;
	short nType;  // 1=bill, 2=quote

	//(e.lally 2007-04-04) PLID 25487 - To keep the behavior the same, we need to silently catch
		//any exceptions and rollback our audit transaction.
	try{

		rs = CreateParamRecordset("SELECT EntryType, PatientID FROM BillsT WHERE ID = {INT}", iBillID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID",0);
			nType = AdoFldByte(rs, "EntryType",0);
		}
		rs->Close();

		//
		//DRT 4/6/2004 - If we are deleting a charge which is for a gift certificate, we need to give an extra warning.  There are 2 levels of
		//	warning here:  1)  If there are multiple charges for a GC, we need to warn them it will decrease the balance.  2)  If there is
		//	only one charge, we should see if they want to completely delete the GC.  If option #2, the GC cannot be deleted if there are
		//	payments that have used it.
		//NOTE:  Any changes here must be replicated in the above DeleteBill function
		_RecordsetPtr prs = CreateParamRecordset("SELECT LineItemT.GiftID, COUNT(ID) AS GiftCnt "
				"FROM LineItemT WHERE Deleted = 0 AND Type = 10 "
				"AND GiftID = (SELECT GiftID FROM LineItemT WHERE ID = {INT}) "
				"GROUP BY LineItemT.GiftID", iChargeID);
		if(!prs->eof) {
			long nCnt = AdoFldLong(prs, "GiftCnt");
			if(nCnt > 1) {
				//multiple charges, warn them balance will be lowered.
				if(MsgBox(MB_YESNO, "This charge is for a gift certificate.  If you continue deleting, the available balance for this gift certificate will "
					"be lowered.  Are you absolutely sure you wish to delete?") != IDYES)
					return FALSE;
			}
			else if(nCnt == 1) {
				//1 charge, we need to delete the GC too
				if(MsgBox(MB_YESNO, "This is the only charge for a gift certificate.  If you continue deleting, this gift certificate will be removed from the system "
					"as well as the charge.  Are you absolutely sure you wish to delete?") != IDYES)
					return FALSE;

				//that passed, but we must pass deleting the GC before we can delete the charge.
				//This function will warn them
				if(!DeleteGiftCertificate(AdoFldLong(prs, "GiftID"), TRUE))
					return FALSE;
			}
		}
		//end gift certificate checks
		//

		// Remove HCFAT relations
		// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
		//ChargeToHCFA(iBillID, iChargeID, FALSE); // ProviderID doesn't matter when deleting

		//(e.lally 2007-04-04) PLID 25487 - Updated sql statements to be in a batch
		CString strSqlBatch = BeginSqlBatch();
		//delete from ApplyDetailsT
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = %d AND PointsToPayments = 0)", iChargeID);

		//Delete affiliated applies
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM AppliesT WHERE DestID = %d AND PointsToPayments = 0", iChargeID);

		//Delete affiliated ChargedProductItems
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ChargedProductItemsT WHERE ChargeID = %li", iChargeID);

		// (a.walling 2007-05-24 11:40) - PLID 26114
		//Delete redeemed rewards
		AddStatementToSqlBatch(strSqlBatch,"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE Deleted = 0 AND Source = %li AND ChargeID = %li", Rewards::erdrRemoved, Rewards::ersRedeemedByCharge, iChargeID);

		// (j.gruber 2009-03-12 12:45) - PLID 33360 - - update ChargeDiscountsT
		AddStatementToSqlBatch(strSqlBatch,"UPDATE ChargeDiscountsT SET DELETED = 1, DeletedBy = '%s', DeleteDate = GetDate() WHERE ChargeID = %d ", _Q(GetCurrentUserName()), iChargeID);
		
		// Delete charge
		CString strChargeInfo = "";
		rs = CreateParamRecordset("SELECT LineItemT.Description, Amount, Quantity, LineItemT.Date, PackageChargeRefID, "
			"Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS NonTaxChargeTotal "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"WHERE LineItemT.ID = {INT} AND LineItemT.Deleted = 0",iChargeID);
		if(!rs->eof) {
			CString strDesc;
			COleCurrency cyAmt, cyNonTaxTotal;
			double dblQty;
			COleDateTime dt;
			strDesc = AdoFldString(rs, "Description","(No Description)");
			cyAmt = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
			cyNonTaxTotal = AdoFldCurrency(rs, "NonTaxChargeTotal",COleCurrency(0,0));
			dblQty = AdoFldDouble(rs, "Quantity",1.0);
			dt = AdoFldDateTime(rs, "Date");
			long nPackageChargeRefID = AdoFldLong(rs, "PackageChargeRefID",-1);
			strChargeInfo.Format("Description: '%s', Amount: %s, Qty: %g, Date: %s",strDesc,FormatCurrencyForInterface(cyAmt,TRUE,TRUE),dblQty,FormatDateTimeForInterface(dt,dtoDate));

			//audit and delete inside this section, because we don't want to re-audit deleting an already-deleted charge
			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();
			// m.carlson 4/28/2005 PL 16372 - if nType is 2, audit as deleting a quote, else assume a bill
			//(e.lally 2007-04-04) PLID 25487 - Use an audit transaction to delay the commit to it until after the statements are executed
			if (nType == 2)
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID),nAuditTransactionID,aeiQuoteChargeDeleted,iChargeID,strChargeInfo,"<Deleted>",aepMedium,aetDeleted);
			else
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID),nAuditTransactionID,aeiChargeLineDeleted,iChargeID,strChargeInfo,"<Deleted>",aepHigh,aetDeleted);

			AddStatementToSqlBatch(strSqlBatch,"UPDATE LineItemT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %d",
			_Q(GetCurrentUserName()), iChargeID);

			// (j.jones 2005-08-19 11:57) - j.jones - update packages accordingly: 
			//update the CurrentAmount for both package types, and increase the QtyRemaining for multi-use packages
			if(nType == 1 && nPackageChargeRefID != -1) {

				// (j.jones 2008-02-14 10:01) - PLID 28922 - we can't just update the package if this charge is
				// on a bill made from a package, we also have to ensure this charge itself was from that package
				// (I also combined the PackagesT.Type recordset into this one, and parameterized)
				_RecordsetPtr rsPackage = CreateParamRecordset("SELECT PackagesT.QuoteID, PackagesT.Type FROM BilledQuotesT "
					"INNER JOIN BillsT ON BilledQuotesT.QuoteID = BilledQuotesT.QuoteID "
					"INNER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
					"WHERE BilledQuotesT.BillID = {INT} "
					"AND PackagesT.QuoteID IN (SELECT BillID FROM ChargesT WHERE ID = {INT}) "
					, iBillID, nPackageChargeRefID);
				while(!rsPackage->eof) {
					//this charge was billed from a package, so update properly

					long nQuoteID = AdoFldLong(rsPackage, "QuoteID", -1);

					//update the CurrentAmount no matter what
					AddStatementToSqlBatch(strSqlBatch,"UPDATE PackagesT SET CurrentAmount = CurrentAmount + Convert(money,'%s') WHERE QuoteID = %li", _Q(FormatCurrencyForSql(cyNonTaxTotal)), nQuoteID);

					//if a multi-use package, update the quantity
					long nPackageType = AdoFldLong(rsPackage, "Type", -1);
					if(nPackageType == 2) {
						AddStatementToSqlBatch(strSqlBatch,"UPDATE ChargesT SET PackageQtyRemaining = PackageQtyRemaining + %g WHERE ID = %li", dblQty, nPackageChargeRefID);
					}

					rsPackage->MoveNext();
				}
				rsPackage->Close();
			}
		}
		rs->Close();	

		//update any LineIDs greater than this ID
		if(boUpdateLineIDs) {
			rs = CreateParamRecordset("SELECT ChargesT.ID, LineID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND BillID = {INT} AND LineID > (SELECT LineID FROM ChargesT WHERE ID = {INT})",iBillID, iChargeID);
			while(!rs->eof) {
				long othChargeID;
				long OldLineID;
				othChargeID = rs->Fields->Item["ID"]->Value.lVal;
				OldLineID = rs->Fields->Item["LineID"]->Value.lVal;
				OldLineID--;
				AddStatementToSqlBatch(strSqlBatch,"UPDATE ChargesT SET LineID = %li WHERE ID = %li",OldLineID, othChargeID);
				rs->MoveNext();
			}
			rs->Close();
		}

		//(e.lally 2007-04-04) PLID 25487 - Commit our batch of statements for the charge
		// (j.jones 2009-10-06 13:36) - PLID 36423 - can't be parameterized
		rs = CreateRecordset("SET NOCOUNT ON \r\n"
			"BEGIN TRAN \r\n"
			" %s \r\n" //strSqlBatch
			"COMMIT TRAN \r\n"
			"SET NOCOUNT OFF \r\n"
			//Check if this is the last charge associate with the bill
			// (j.jones 2009-03-03 09:54) - PLID 33287 - added check for how many charges are batched
			"SELECT COALESCE(COUNT(ChargesT.BillID),0) AS RemainingChargesCount, "
			"Sum(CASE WHEN ChargesT.Batched = 1 THEN 1 ELSE 0 END) AS CountBatched "
			"FROM LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE ChargesT.BillID = %d AND LineItemT.Deleted = 0 ",
			strSqlBatch, iBillID);
		
		// See if the bill has any charges. If not, delete the bill		
		if(rs->GetState() == adStateClosed) {
			_variant_t varNull;
			rs = rs->NextRecordset(&varNull);
		}
		if(rs->GetState() == adStateOpen && !rs->eof){
			long nRemainingCharges = VarLong(rs->Fields->GetItem("RemainingChargesCount")->Value, 0);
			long nCountBatched = VarLong(rs->Fields->GetItem("CountBatched")->Value, 0);
			//If this was the last charge, delete the bill
			// (j.jones 2009-04-03 16:46) - PLID 33287 - moved the boDeleteBill check here,
			// so we can unbatch even if we don't delete
			if(nRemainingCharges == 0 && boDeleteBill) {
				DeleteBill(iBillID, FALSE);
			}
			// (j.jones 2009-03-03 09:54) - PLID 33287 - if no remaining charges are batched,
			// make sure the claim is not batched
			else if(nCountBatched == 0) {
				//unbatch the claim
				BatchBill(iBillID, 0, TRUE);
			}
		}
		if(rs->GetState() == adStateOpen) {
			rs->Close();
		}

		//Run our auditing transaction
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

	}NxCatchAllSilentCallThrow(if(nAuditTransactionID==-1){RollbackAuditTransaction(nAuditTransactionID);})

	CClient::RefreshTable(NetUtils::PatBal, nPatientID);
	return TRUE;
}

// (z.manning 2016-02-15 15:08) - PLID 68258 - Changed return type to boolean
BOOL DeletePayment(int iPaymentID, BOOL bPAYMENTECHForceCCDelete /* = FALSE*/)
{
	long BatchPayID = -1;

	_RecordsetPtr rs;
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Deleting Payment %d", iPaymentID);
	try
	{
		CString str;

		long nPatientID = 0;
		long nDrawerID = -1;
		// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT PatientID FROM LineItemT WHERE ID = {INT}",iPaymentID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID",0);
		}
		rs->Close();

		// (d.thompson 2009-04-16) - PLID 34004 - Somehow this flag got applied to ALL warnings, not just CC deletion.  This means
		//	that if you wanted to force CC delete, you'd also force several other rules that are supposed to be in place.  I am
		//	removing that functionality for now, it seems incredibly wrong.  It was added by PLID 15416 (with no comment in this code!)
		//This flag is only ever currently used from Paymentech processing, which is not available to the general public.
		//I also renamed the flag to be Paymentech specific.
		if (! bPAYMENTECHForceCCDelete ) {
			// (j.gruber 2007-07-16 11:53) - PLID 15416 - Check that this transaction isn't processed successfully
			// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
			rs = CreateParamRecordset("SELECT TOP 1 ID FROM CreditTransactionsT WHERE ID = {INT} AND IsApproved = 1", iPaymentID);
			if(!rs->eof) {
				//it's been approved, they can't delete it
				MsgBox("This payment has been processed and is approved, you may not delete it.  You will need to void the transaction or refund the payment.");
				return FALSE;
			}
			rs->Close();
		}

		// (d.thompson 2009-04-16) - PLID 33957 - Do not allow a payment to be deleted if it is tied to an IGS transaction
		//	that was approved.  These are permanent.
		// (d.thompson 2010-11-19) - PLID 40305 - Don't allow deletion of Chase trans either
		// (c.haag 2015-08-24) - PLID 67198 - Use a utility function that handles all kinds of transactions
		if (IsCCPaymentProcessed(iPaymentID))
		{
			MsgBox("This credit card payment has been processed and approved, you may not delete it.");
			return FALSE;
		}

		//DRT 4/21/2004 - If this is put into a cash drawer, they really can't delete it or their amounts will be off.  We'll
		//	let them anyways (they'll have to edit their amounts), but we will warn first.
		// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT TOP 1 ID FROM LineItemT WHERE ID = {INT} AND DrawerID IS NOT NULL AND DrawerID IN "
			"(SELECT ID FROM CashDrawersT WHERE DateClosed IS NOT NULL)", iPaymentID);
		if(!rs->eof) {
			if(MsgBox(MB_YESNO, "This payment has been linked to a closed cash drawer.  If you continue, the calculated "
				"amounts for this drawer will be changed.  You may need to edit the drawer to reflect those changed values.  Are "
				"you SURE you wish to delete this payment?") == IDNO) {
				return FALSE;
			}
		}
		rs->Close();

		//Figure out if this is a batch payment
		// (j.jones 2009-06-09 15:46) - PLID 34549 - ignore if an adjustment
		rs = CreateParamRecordset("SELECT BatchPaymentID FROM PaymentsT WHERE ID = {INT} AND PayMethod <> 0",iPaymentID);
		if(!rs->eof) {
			BatchPayID = AdoFldLong(rs, "BatchPaymentID",-1);
			if(BatchPayID != -1) {
				// (j.jones 2009-12-29 11:41) - PLID 27237 - removed a poorly used \n, which has nothing to do with this PL item,
				// I just fixed it while testing it
				if(IDNO == MessageBox(GetActiveWindow(),"This is part of a batch insurance payment. Deleting this payment will return "
					"the funds back to the batch payment to be re-applied elsewhere."
					"\n\nAre you sure you wish to delete this payment?","Practice",MB_ICONQUESTION|MB_YESNO))
					return FALSE;
			}
		}
		rs->Close();

		// (j.jones 2007-02-26 16:18) - PLID 24927 - warn if linked to the ReturnedProductsT table
		// (j.jones 2008-06-03 14:28) - PLID 29928 - only look at ReturnedProductsT records that are not for deleted charges
		// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT ID FROM ReturnedProductsT "
			"WHERE ChargeID NOT IN (SELECT ID FROM LineItemT WHERE Deleted = 1) "
			"AND (FinRefundID = {INT} OR FinAdjID = {INT})", iPaymentID, iPaymentID);
		if(!rs->eof) {
			if(IDNO == MessageBox(GetActiveWindow(),"This item is linked to a Returned Product record. If you delete this line item,\n"
							"reports such as Tax Totals and Provider Commissions (Charges) will not include all\n"
							"Returned Products properly, if you choose to show Returned Products in those reports.\n\n"
							"Are you sure you wish to mark this item deleted?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
							return FALSE;
			}
		}
		rs->Close();

		// Figure out if any of this payment has been applied 
		// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE (((LineItemT.PatientID)={INT}) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
			"WHERE (((PatientPaymentsQ.ID) Is Not Null)) AND SourceID = {INT}", nPatientID, iPaymentID);
		if (!rs->eof) {	// At least one apply exists
			if (IDNO == MessageBox(GetActiveWindow(), "This payment has been applied to several line items. Do you also wish to remove all related applies?", "NexTech", MB_YESNO))
				return FALSE;
		}
		rs->Close();

		//(e.lally 2007-04-04) PLID 25487 - Updated statements to be in a batch
		CString strSqlBatch = BeginSqlBatch();
		//Delete From ApplyDetailsT
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID = %li)", iPaymentID);
		
		////////////////////////////////////////////////////
		// Delete all patient and insurance applies from this payment
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM AppliesT WHERE SourceID = %li", iPaymentID);
	
		//Delete From ApplyDetailsT
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = %li AND PointsToPayments = 1)", iPaymentID);
		
		////////////////////////////////////////////////////
		// Delete all line items applied to this payment
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM AppliesT WHERE DestID = %li AND PointsToPayments = 1", iPaymentID);

		AuditEventItems aeiItem = aeiPaymentDeleted;

		//get info for auditing
		// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT Type, Date, Amount, Description FROM LineItemT WHERE ID = {INT}",iPaymentID);
		CString desc;
		if(!rs->eof) {
			int Type = rs->Fields->Item["Type"]->Value.lVal;
			CString strType;
			if(Type == 3) {
				strType = "Refund";
				aeiItem = aeiRefundDeleted;
			}
			else if(Type == 2) {
				strType = "Adjustment";
				aeiItem = aeiAdjustmentDeleted;
			}
			else {
				strType = "Payment";
				aeiItem = aeiPaymentDeleted;
			}
			COleDateTime date = rs->Fields->Item["Date"]->Value.date;
			CString strAmount = FormatCurrencyForInterface(rs->Fields->Item["Amount"]->Value.cyVal);
			CString strDescription = AdoFldString(rs, "Description","");
			desc.Format("%s %s, %s, %s",strAmount,strType,FormatDateTimeForInterface(date),_Q(strDescription));
		}
		rs->Close();

		////////////////////////////////////////////////////
		// Delete the payment
		AddStatementToSqlBatch(strSqlBatch,"UPDATE LineItemT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li",
			_Q(GetCurrentUserName()), iPaymentID);
		//Commit our batch of statements
		ExecuteSqlBatch(strSqlBatch);

		// (j.jones 2007-05-04 10:25) - PLID 23280 - delete from the last payment date array
		// if it is present
		DeleteLastPaymentDate(iPaymentID);

		//Do the Auditing
		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID != -1)
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID),AuditID,aeiItem,iPaymentID,desc,"<Deleted>",aepHigh,aetDeleted);
		
		PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_PaymentApplied, iPaymentID);
		CClient::RefreshTable(NetUtils::PatBal, nPatientID);

		return TRUE;
	}
	NxCatchAll("Error in Delete Payment");

	return FALSE;
}

////////////////////////////////////////////////////////////
// This function deletes an apply. The input is the apply id.
// (j.jones 2011-03-21 17:10) - PLID 24273 - added a bool that tells auditing that an EOB posting caused the unapply
BOOL DeleteApply(long nApplyID, BOOL bWarn /*= TRUE*/, BOOL bUnappliedDuringEOBPosting /*= FALSE*/)
{
	CString str;

	long AuditID = -1;
	try {
		
		//see if this is an applied batch payment
		// (j.jones 2009-06-09 15:46) - PLID 34549 - ignore if an adjustment
		if(bWarn) {
			// (j.jones 2009-10-07 09:09) - PLID 36423 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 PaymentsT.ID FROM PaymentsT "
				"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE PaymentsT.BatchPaymentID Is Not Null AND AppliesT.ID = {INT} AND PayMethod <> 0", nApplyID);
			if(!rs->eof) {
				if(IDNO == MessageBox(GetActiveWindow(),"This payment is part of a batch insurance payment."
					"\nAre you sure you want to unapply this item? (Funds will not be returned to the batch payment until this payment is deleted.)",
					"Practice",MB_ICONINFORMATION|MB_YESNO))
					return FALSE;
			}
			rs->Close();
		}

		//save amount and patient ID for Auditing
		CString strAmount;
		long nPatientID = -1;
		//TES 7/10/2008 - PLID 30671 - We also need the source and dest IDs and Types
		long nSourceID = -1, nSourceType = -1, nDestID = -1, nDestType = -1;
		// (j.jones 2008-05-27 09:44) - PLID 29337 - pulled the patient ID from the destination record
		_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.SourceID, SourceItem.Type AS SourceType, "
			"AppliesT.DestID, DestItem.Type AS DestType, AppliesT.Amount, DestItem.PatientID "
			"FROM AppliesT INNER JOIN LineItemT SourceItem ON AppliesT.SourceID = SourceItem.ID "
			"INNER JOIN LineItemT DestItem ON AppliesT.DestID = DestItem.ID "
			"WHERE AppliesT.ID = {INT}", nApplyID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID", -1);
			COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
			strAmount.Format("%s",FormatCurrencyForInterface(cyAmount,TRUE,TRUE));
			strAmount.Replace("(","");
			strAmount.Replace(")","");
			nSourceID = AdoFldLong(rs, "SourceID");
			nSourceType = AdoFldLong(rs, "SourceType");
			nDestID = AdoFldLong(rs, "DestID");
			nDestType = AdoFldLong(rs, "DestType");
		}
		else {
			//TES 7/10/2008 - PLID 30671 - Why would we keep going if this apply doesn't actually exist?
			return FALSE;
		}
		rs->Close();

		//(e.lally 2007-04-04) PLID 25487 - Put statements in a batch
		CString strSqlBatch = BeginSqlBatch();
		//Delete from ApplyDetailsT
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ApplyDetailsT WHERE ApplyID = %li", nApplyID);
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM AppliesT WHERE ID = %li", nApplyID);
		//(e.lally 2007-04-04) PLID 25487 - Commit the batch of statements
		ExecuteSqlBatch(strSqlBatch);

		AuditID = BeginAuditTransaction();
		if(AuditID != -1) {
			// (j.jones 2008-05-27 09:44) - PLID 29337 - audited the proper patient name
			//AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiApplyDeleted, nApplyID, "", strAmount, aepHigh, aetDeleted);
			//TES 7/10/2008 - PLID 30671 - We now audit both the source and the dest that are being unapplied
			//Source
			AuditEventItems aeiSourceType;
			switch(nSourceType) {
			case 1:
				// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
				// if an EOB posting caused this
				if(bUnappliedDuringEOBPosting) {
					aeiSourceType = aeiPaymentUnappliedByERemitPosting;
				}
				else {
					aeiSourceType = aeiPaymentUnapplied;
				}
				break;
			case 2:
				// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
				// if an EOB posting caused this
				if(bUnappliedDuringEOBPosting) {
					aeiSourceType = aeiAdjustmentUnappliedByERemitPosting;
				}
				else {
					aeiSourceType = aeiAdjustmentUnapplied;
				}
				break;
			case 3:
				// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
				// if an EOB posting caused this
				if(bUnappliedDuringEOBPosting) {
					aeiSourceType = aeiRefundUnappliedByERemitPosting;
				}
				else {
					aeiSourceType = aeiRefundUnapplied;
				}
				break;
			default:
				AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
				break;
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiSourceType, nSourceID, "", strAmount + " Unapplied", aepHigh, aetDeleted);
			
			AuditEventItems aeiDestType;
			switch(nDestType) {
				case 10:
					// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
					// if an EOB posting caused this
					if(bUnappliedDuringEOBPosting) {
						aeiDestType = aeiItemUnappliedFromChargeByERemitPosting;
					}
					else {
						aeiDestType = aeiItemUnappliedFromCharge;
					}
					break;
				case 1:
					ASSERT(bUnappliedDuringEOBPosting == FALSE);

					aeiDestType = aeiItemUnappliedFromPayment;
					break;
				case 2:
					ASSERT(bUnappliedDuringEOBPosting == FALSE);

					aeiDestType = aeiItemUnappliedFromAdjustment;
					break;
				case 3:
					ASSERT(bUnappliedDuringEOBPosting == FALSE);

					aeiDestType = aeiItemUnappliedFromRefund;
					break;
				default:
					AfxThrowNxException("Bad Dest Type %li found while auditing unapply!", nDestType);
				break;
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiDestType, nDestID, "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);

			CommitAuditTransaction(AuditID);
		}
		return TRUE;
	}
	NxCatchAllCall("Error in DeleteApply", {if(AuditID != -1) RollbackAuditTransaction(AuditID);});

	return FALSE;
}

// (r.gonet 07/11/2014) - PLID 62556 - Deletes a chargeback that is applied to a certain charge. Returns the amount to the batch payment.
// - chargeback - Reference to the chargeback to delete
// - bSilent - If true, then Practice will not display any warnings to the user. If false, Practice will warn the user about certain things.
// Returns true if the deletion was successful and false otherwise.
bool DeleteChargeback(Chargeback &chargeback, bool bSilent/*=false*/)
{
	long nAuditTransactionID = -1;
	try {
		nAuditTransactionID = BeginAuditTransaction();

		if (!bSilent && IDYES != MsgBox(MB_ICONQUESTION | MB_YESNO, "You have selected to undo this chargeback. Doing this will delete the chargeback payment and adjustment "
			"and reduce the Batch Vision Payment in which it was posted from. Do you wish to continue?")) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		} else {
			// Proceed with deleting the chargeback
		}

		// Undoing a chargeback can be done by any user after a financial close (since it makes no difference on AR,
		// but warn them about it anyway). Also check the permission to delete payments.
		if (!CanChangeHistoricFinancial("Chargeback", chargeback.nID, bioPayment, sptDelete, bSilent ? TRUE : FALSE)) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		} else {
			// They have permission to change historical financial 
		}

		long nPatientID = 0;
		long nBatchPayID = -1;
		CParamSqlBatch sqlInfoBatch;
		sqlInfoBatch.Add("SELECT PatientID FROM LineItemT WHERE ID = {INT}", chargeback.nChargeID);
		sqlInfoBatch.Add("SELECT BatchPaymentID FROM PaymentsT WHERE ID = {INT}", chargeback.nPaymentID);
		sqlInfoBatch.Add("SELECT ID, Type, Date, Amount, Description FROM LineItemT WHERE ID = {INT}", chargeback.nPaymentID);
		_RecordsetPtr prsInfo = sqlInfoBatch.CreateRecordset(GetRemoteDataSnapshot());
		// Get the patient
		if (!prsInfo->eof) {
			nPatientID = AdoFldLong(prsInfo->Fields, "PatientID", 0);
		} else {
			// The charge doesn't exist? Then what is this chargeback applied to?
			ASSERT(FALSE);
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}
		//Get the associated batch payment so we can return the funds to it
		prsInfo = prsInfo->NextRecordset(NULL);
		if (!prsInfo->eof) {
			nBatchPayID = AdoFldLong(prsInfo->Fields, "BatchPaymentID", -1);
		} else {
			// What? All chargebacks must come from batch payments...
			ASSERT(FALSE);
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}

		// Audit what we are about to do
		prsInfo = prsInfo->NextRecordset(NULL);
		if (!prsInfo->eof) {
			long nID = AdoFldLong(prsInfo->Fields, "ID");
			COleCurrency cyPaymentAmount = AdoFldCurrency(prsInfo->Fields, "Amount");

			if (!bSilent) {
				// Check to see if this will cause the batch payment amount to go negative
				COleCurrency cyRemainingBatchPaymentAmount = GetRemainingBatchPaymentAmount(nBatchPayID);
				if (cyRemainingBatchPaymentAmount + cyPaymentAmount < g_ccyZero) {
					if (IDYES != MsgBox(MB_ICONWARNING | MB_YESNO, "You are attempting to return a chargeback to a Batch Vision Payment, but the total "
						"amount remaining on the check is %s. If you choose to continue, this will result in a negative balance on the check. Are you sure you wish to do this?"
						, FormatCurrencyForInterface(cyRemainingBatchPaymentAmount))) {
						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
						}
						return false;
					} else {
						// Proceed with chargeback deletion
					}
				}
			} else {
				// We won't warn them that the batch payment is about to go negative. Just do it.
			}

			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strAmount = FormatCurrencyForInterface(cyPaymentAmount);
			CString strOldValue = FormatString("%s Chargeback", strAmount);
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiChargebackDeleted, nID, strOldValue, "<Deleted>", aepHigh, aetDeleted);
		} else {
			// You can't have a chargeback without a payment
			ASSERT(FALSE);
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}
		prsInfo->Close();

		CParamSqlBatch sqlDeleteBatch;
		// Delete From ApplyDetailsT
		sqlDeleteBatch.Add("DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID IN ({INT}, {INT}))", chargeback.nPaymentID, chargeback.nAdjustmentID);
		// Delete all patient and insurance applies from this payment
		sqlDeleteBatch.Add("DELETE FROM AppliesT WHERE SourceID IN ({INT}, {INT})", chargeback.nPaymentID, chargeback.nAdjustmentID);
		// Delete From ApplyDetailsT
		sqlDeleteBatch.Add("DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID IN ({INT}, {INT}) AND PointsToPayments = 1)", chargeback.nPaymentID, chargeback.nAdjustmentID);
		// Delete all line items applied to this payment. Chargeback line items should not have applies though.
		sqlDeleteBatch.Add("DELETE FROM AppliesT WHERE DestID IN ({INT}, {INT}) AND PointsToPayments = 1", chargeback.nPaymentID, chargeback.nAdjustmentID);
		// Delete the payment
		sqlDeleteBatch.Add("UPDATE LineItemT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} WHERE ID IN ({INT}, {INT})",
			GetCurrentUserName(), chargeback.nPaymentID, chargeback.nAdjustmentID);
		// Delete the chargeback
		sqlDeleteBatch.Add("DELETE FROM ChargebacksT WHERE ID = {INT}", chargeback.nID);
		// Commit our batch of statements
		sqlDeleteBatch.Execute(GetRemoteData());

		// Delete from the last payment date array if it is present
		DeleteLastPaymentDate(chargeback.nPaymentID);
		DeleteLastPaymentDate(chargeback.nAdjustmentID);

		CClient::RefreshTable(NetUtils::PatBal, nPatientID);

		CommitAuditTransaction(nAuditTransactionID);
		return true;
	} NxCatchAllCall(__FUNCTION__,
	if (nAuditTransactionID != -1) {
		RollbackAuditTransaction(nAuditTransactionID);
	});
	return false;
}

// (r.gonet 07/25/2014) - PLID 62556 - Returns the remaining amount on a batch payment
// - nBatchPaymentID: ID of the batch payment for which to get the remaining balance.
COleCurrency GetRemainingBatchPaymentAmount(long nBatchPaymentID)
{
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
		R"(
SELECT BatchPaymentsT.Amount 
  - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) 
  + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) 
  - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) 
  AS RemainingAmount 

FROM BatchPaymentsT 

LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied 
	FROM LineItemT 
	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID 
	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID 
	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID 
	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 
	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null 
	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null 
	AND PaymentsT.BatchPaymentID Is Not Null 
	GROUP BY PaymentsT.BatchPaymentID 
) AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID 

LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed 
	FROM LineItemT 
	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID 
	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 
	AND LineItemCorrectionsT.BatchPaymentID Is Not Null 
	GROUP BY LineItemCorrectionsT.BatchPaymentID 
) AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID 

LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID 
	FROM BatchPaymentsT 
	WHERE Type <> 1 AND Deleted = 0 
	GROUP BY AppliedBatchPayID, Deleted 
) AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID 

GROUP BY BatchPaymentsT.ID, 
BatchPaymentsT.Amount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied
HAVING BatchPaymentsT.ID = {INT}
)"
, nBatchPaymentID);

	if (!prs->eof) {
		return AdoFldCurrency(prs->Fields, "RemainingAmount", g_ccyZero);
	} else {
		return g_ccyZero;
	}
}

//DRT 4/6/2004 - Function to delete a gift certificate.
//returns TRUE if successfully delete
//returns FALSE if it could not be deleted
//Cannot be deleted if:
//	Any payments have been made with this GiftID
//	Any charges exist for this item UNLESS bIgnoreCharges is set to TRUE
BOOL DeleteGiftCertificate(long nID, BOOL bIgnoreCharges /*= FALSE*/)
{
	try {

		// (j.jones 2015-04-23 12:51) - PLID 65711 - disallow deleting gift certificates that have been used in a balance transfer
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM GiftCertificateTransfersT WHERE SourceGiftID = {INT} OR DestGiftID = {INT}", nID, nID);
		if (!rs->eof) {
			MsgBox("This gift certificate cannot be deleted because it has been used in a transfer of gift certificate balances.");
			return FALSE;
		}
		rs->Close();

		//check for payments
		// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
		rs = CreateParamRecordset("SELECT ID FROM LineItemT WHERE GiftID = {INT} AND Deleted = 0 AND Type = 1", nID);
		if(!rs->eof) {
			MsgBox("This gift certificate cannot be deleted because there are payments which have been made.  These "
				"payments must be deleted first.");
			return FALSE;
		}
		rs->Close();

		//we may want to ignore charges if this GC is being deleted at the time
		//you are trying to delete a charge
		if(!bIgnoreCharges) {
			// (j.jones 2009-10-07 09:19) - PLID 36423 - parameterized
			rs = CreateParamRecordset("SELECT ID FROM LineItemT WHERE GiftID = {INT} AND Deleted = 0 AND Type = 1", nID);
			if(!rs->eof) {
				MsgBox("This gift certificate cannot be deleted because there are charges which apply.  These "
					"charges must be deleted first.");
				return FALSE;
			}
			rs->Close();
		}
		//(e.lally 2007-04-04) PLID 25487 - Put statements in a batch
		CString strSqlBatch = BeginSqlBatch();

		//if we got here we're OK to delete
		AddStatementToSqlBatch(strSqlBatch,"UPDATE LineItemT SET GiftID = NULL WHERE GiftID = %li", nID);	//fix all line item records
		// (r.gonet 2015-04-29 19:24) - PLID 65657 - Clear the FK reference.
		AddStatementToSqlBatch(strSqlBatch,"UPDATE PaymentsT SET RefundedFromGiftID = NULL WHERE RefundedFromGiftID = %li", nID);	//fix all payments referencing this gift certificate.
		AddStatementToSqlBatch(strSqlBatch,"DELETE FROM GiftCertificatesT WHERE ID = %li", nID);			//wipe out the gift certificate
		//(e.lally 2007-04-04) PLID 25487 - Commit the batch of statements
		ExecuteSqlBatch(strSqlBatch);

		return TRUE;
	} NxCatchAll("Error deleting gift certificate.");

	return FALSE;
}

void DateToFieldString(CString& str)
{
	if (str.GetLength() == 0)
		str = "Null";
	else {
		COleDateTime cyDate;
		cyDate.ParseDateTime(str);

		if (cyDate.m_status == COleDateTime::invalid)
			str = "Null";
		else
			str = CString("'") + str + "'";
	}
}

/* BVB
void UpdateWithWait(CDaoRecordset* rs)
{
	if (!rs) return;

	while (1) {
		try {
			rs->Update();
		}
		catch (CDaoException* e) {
			if (e->m_pErrorInfo->m_lErrorCode == E_DAO_WriteConflictM) {
				if (IDYES == ThrowFinancialException(e)) {
					e->Delete();
					continue;
				}
			}
			throw e;
		}
		break;
	}
}*/

/* BVB
int ThrowFinancialException(CDaoException* e, LPCTSTR lpszFunctionName, CDaoRecordset* pRS)
{
	CNxErrorDialog dlg;
	CString str;

	switch ( e->m_pErrorInfo->m_lErrorCode) {
	case E_DAO_WriteConflictM: // 3260
	case E_DAO_DataHasChanged: // 3197
		{
			if (lpszFunctionName != NULL)
				str.Format("[%s]\nPractice cannot update this data because it is currently being written to by another user. Do you wish to retry?", lpszFunctionName);
			else
				str.Format("Practice cannot update this data because it is currently being written to by another user. Do you wish to retry?");
			LogDetail("%s", str);
			return dlg.DoModal(str, "Financial Exception", MINOR_ERROR);//MessageBox(GetActiveWindow(), str, 0, MB_YESNO);
		}
	case E_DAO_RecordDeleted: // 3167
		if (lpszFunctionName != NULL)
			str.Format("[%s]\nPractice cannot access this data because it has been deleted by the system or another user.", lpszFunctionName);
		else
			str.Format("Practice cannot access this data because it has been deleted by the system or another user");
		break;
	case E_DAO_TooManyOpenTables: // 3014
		str.Format("The following error has occurred:\n\nError: %li\nSource: %s\nDescription: %s", e->m_pErrorInfo->m_lErrorCode, e->m_pErrorInfo->m_strSource, e->m_pErrorInfo->m_strDescription);
		break;
	default:
		if (lpszFunctionName != NULL)
			str.Format("[%s]\nThe following error has occurred:\n\nError: %li\nSource: %s\nDescription: %s", lpszFunctionName, e->m_pErrorInfo->m_lErrorCode, e->m_pErrorInfo->m_strSource, e->m_pErrorInfo->m_strDescription);
		else
			str.Format("The following error has occurred:\n\nError: %li\nSource: %s\nDescription: %s", e->m_pErrorInfo->m_lErrorCode, e->m_pErrorInfo->m_strSource, e->m_pErrorInfo->m_strDescription);
		break;
	}

	return dlg.DoModal(str, "Financial Exception", MINOR_ERROR);
	//MsgBox(str);
	if (pRS) {
		CString strLog;
		strLog.Format("\r\nSQL=%s\r\nFilter=%s\r\nSort=%s\r\n", pRS->GetSQL(), pRS->m_strFilter, pRS->m_strSort);
		str += strLog;
	}
	LogDetail("%s", str);
	
	return 0;
}*/

int GetFormTypeFromBillID(long iBillID) {

	long FormType = -1;

	// (j.jones 2009-10-06 13:36) - PLID 36423 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT FormType FROM BillsT WHERE ID = {INT}",iBillID);
	if(!rs->eof) {
		FormType = AdoFldLong(rs, "FormType",-1);
	}
	rs->Close();

	return FormType;
}
// (s.tullis 2016-02-24 13:47) - PLID 68319 - Get Claim for type for particular insurance/location config
// if we don't have insurance we'll just default to the location default
int GetFormTypeFromLocationInsuranceSetup(long nInsuredPartyID, long nLocationID)
{
	int nFormType = -1;

	if (nLocationID == -1) {
		return nFormType;
	}

	try {
			_RecordsetPtr rs = CreateParamRecordset( R"(
				DECLARE @ClaimFormType INT
				SET NOCOUNT ON
				SET @CLaimFormType = (	
					SELECT FormType 
					FROM ClaimFormLocationInsuranceSetupT 
					Inner Join InsuredPartyT
					ON InsuredPartyT.InsuranceCoID = ClaimFormLocationInsuranceSetupT.InsuranceID
					WHERE InsuredPartyT.PersonID = {INT} AND ClaimFormLocationInsuranceSetupT.LocationID = {INT})
				IF(@ClaimFormType IS NULL)
				BEGIN
					SET @CLaimFormType = (		
						Select DefaultClaimForm
						FROM LocationsT WHERE ID = {INT})
				END
				SET NOCOUNT OFF
				SELECT @ClaimFormType AS FormType
				)", nInsuredPartyID, nLocationID, nLocationID);
			if (!rs->eof) {
				nFormType = AdoFldLong(rs, "FormType", -1);
			}
			rs->Close();

		
	}NxCatchAll(__FUNCTION__)
	return nFormType;
}
// (s.tullis 2016-02-24 16:48) - PLID 68319- Update Form Type
void UpdateBillClaimForm(long nBillID)
{
	try {
		ExecuteParamSql(R"(
		DECLARE @BillID INT; 
		DECLARE @BillLocationID INT;
		DECLARE @BillInsuranceID  INT;
		DECLARE @ClaimFormTypeID INT;
		SET @BILLID = {INT};

		SELECT @BillLocationID = BillsT.Location,  
		       @BillInsuranceID = InsuredPartyT.InsuranceCoID
	    FROM BillsT
		LEFT JOIN InsuredPartyT 
		ON InsuredPartyT.PersonID = BillsT.InsuredPartyID 
		WHERE BillsT.ID = @BillID;

		-- Check our setup Records
		SET @ClaimFormTypeID = (SELECT FormType FROM ClaimFormLocationInsuranceSetupT WHERE LocationID = @BillLocationID AND InsuranceID = @BillInsuranceID);
		
		IF(@ClaimFormTypeID IS NOT NULL)
			BEGIN
				UPDATE BillsT SET FormType = @ClaimFormTypeID WHERE BillsT.ID = @BillID;
			END	
		ELSE 
			BEGIN 
				-- if now we do not have a formtype for the insurance/ location use the default associated with the location
				SET @ClaimFormTypeID = (SELECT DefaultClaimForm FROM LocationsT WHERE LocationsT.ID = @BillLocationID);
				IF(@ClaimFormTypeID IS NOT NULL)
				BEGIN
					UPDATE BillsT SET FormType = @ClaimFormTypeID WHERE BillsT.ID = @BillID;
				END
			END
		)", nBillID);

	}NxCatchAll(__FUNCTION__)
}
// (j.jones 2008-02-11 16:58) - PLID 28847 - added bSkipRespCheck to save a recordset if
// we already validated that the bill is ok prior to calling BatchBill
// (j.dinatale 2012-11-06 14:05) - PLID 50792 - return what change was made
BatchChange::Status BatchBill(int iBillID, int iBatch, BOOL bSkipRespCheck /*= FALSE*/)
{
	// (j.jones 2008-02-11 14:42) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
	// (but you can unbatch claims)
	if(iBatch > 0 && !bSkipRespCheck) {
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(iBillID, FALSE)) {
			return BatchChange::None;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - no change
		}
	}

	_RecordsetPtr rs(__uuidof(Recordset));
	int nCharges = 0, iProviderID = -9999;

	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - keep track of our change.
	BatchChange::Status eBatchStatus = BatchChange::None;

	try {
		// (j.jones 2009-10-06 13:36) - PLID 36424 - parameterized
		rs = CreateParamRecordset("SELECT ID, Batch FROM HCFATrackT WHERE BillID = {INT}", iBillID); // (b.eyers 2015-10-06) - PLID 42101 - get Batch for update and delete
		if (!rs->eof && iBatch > 0) { // table has a record and needs to be updated
			long nOldValue = AdoFldLong(rs, "Batch"); // (b.eyers 2015-10-06) - PLID 42101
			rs->Close();
			//always reset ValidationState to 0 if the bill has changed
			ExecuteParamSql("UPDATE HCFATrackT SET Batch = {INT}, ValidationState = 0 WHERE BillID = {INT}", iBatch, iBillID);
			// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, either paper to electronic or electronic to paper
			if (nOldValue != iBatch) {
				std::vector<long> aryBillID;
				aryBillID.push_back(iBillID);
				AuditInsuranceBatch(aryBillID, iBatch, nOldValue);
			}
			eBatchStatus = BatchChange::Batched;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - batched!
		}
		else if ((rs->eof || rs->Fields->Item["ID"]->Value.vt == VT_NULL || rs->Fields->Item["ID"]->Value.vt == VT_EMPTY) && iBatch > 0) { // table does not have a record, and needs to be updated
			rs->Close();
			
			// (j.jones 2008-05-30 14:18) - PLID 27881 - removed the FormHistoryT creation, since we aren't saving anything

			//try to get the true last send date
			CString strSendDate;
			//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value
			int iSendType = ClaimSendType::HCFA; //int iSendType = 1;
			if(iBatch==2) //Ebilling
				iSendType = ClaimSendType::Electronic;	//iSendType = 0;
			else
				iSendType = GetFormTypeFromBillID(iBillID);

			// (j.jones 2009-10-06 13:36) - PLID 36424 - parameterized and combined into one database access,
			// and also fixed a bug on the fly where we previously did not select the Max date from ClaimHistory,
			// and instead picked the first date, which was wrong
			ExecuteParamSql("SET NOCOUNT ON "
				"DECLARE @nID INT "
				"SET @nID = Coalesce((SELECT Max(ID) FROM HCFATrackT), 0) + 1 "
				""
				"DECLARE @dtDate datetime "
				"SET @dtDate = (SELECT Max(Date) FROM ClaimHistoryT WHERE BillID = {INT} AND SendType = {INT}) "
				""
				"INSERT INTO HCFATrackT (ID, BillID, Batch, LastSendDate) VALUES (@nID, {INT}, {INT}, @dtDate)",
				iBillID, iSendType,
				iBillID, iBatch);

			eBatchStatus = BatchChange::Batched;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - batched!

			// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, unbatched to either electronic or paper
			std::vector<long> aryBillID;
			aryBillID.push_back(iBillID);
			AuditInsuranceBatch(aryBillID, iBatch, 0);

		}
		else if (!rs->eof && iBatch == 0) { // table has a record and needs to be deleted
			long nOldValue = AdoFldLong(rs, "Batch"); // (b.eyers 2015-10-06) - PLID 42101
			rs->Close();
			ExecuteParamSql("DELETE FROM HCFATrackT WHERE BillID = {INT}", iBillID);
			eBatchStatus = BatchChange::Unbatched;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - unbatched!
			// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electornic or paper to unbatched
			std::vector<long> aryBillID;
			aryBillID.push_back(iBillID);
			AuditInsuranceBatch(aryBillID, iBatch, nOldValue);
		}
		else
			rs->Close();
		
		return eBatchStatus;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - return our change
	} NxCatchAll("Error in BatchBill");

	return BatchChange::None;	// (j.dinatale 2012-11-06 14:05) - PLID 50792 - boo, something is broken we failed
}

int FindHCFABatch(int iBillID)
{
	_RecordsetPtr rs(__uuidof(Recordset));
	short nBatch = 0;

	try {
		// (j.jones 2009-10-06 13:36) - PLID 36424 - parameterized
		rs = CreateParamRecordset("SELECT Batch FROM HCFATrackT WHERE BillID = {INT}", iBillID);
		if (!rs->eof) {
			nBatch = (short)rs->Fields->Item["Batch"]->Value.lVal;
		}
		rs->Close();
	} NxCatchAll("Error in FindBatch()");

	return nBatch;
}

int FindDefaultHCFABatch(long InsuredPartyID) {

	// (j.jones 2012-01-17 12:08) - PLID 47510 - default to paper,
	// we will only return this default value if the insurance co.
	// is not in any HCFA Group and thus has no setting in place
	int batch = 1;

	// (j.jones 2009-10-06 13:36) - PLID 36424 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT CASE WHEN RespTypeID = 1 THEN DefBatch ELSE DefBatchSecondary END AS DefaultBatch FROM HCFASetupT INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = {INT}",InsuredPartyID);

	if(!rs->eof) {
		// (j.jones 2012-01-17 12:07) - PLID 47510 - this is not a nullable field, so do not confuse things with a default value
		batch = AdoFldLong(rs, "DefaultBatch");
	}
	rs->Close();

	return batch;
}

int FindDefaultUB92Batch(long InsuredPartyID) {

	int batch = 0;

	// (j.jones 2009-10-06 13:36) - PLID 36424 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT CASE WHEN RespTypeID = 1 THEN DefBatch ELSE DefBatchSecondary END AS DefaultBatch FROM UB92SetupT INNER JOIN InsuranceCoT ON UB92SetupT.ID = InsuranceCoT.UB92SetupGroupID "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = {INT}",InsuredPartyID);

	if(rs->eof)
		return batch;
	else
		batch = AdoFldLong(rs, "DefaultBatch",0);

	rs->Close();

	return batch;
}

// (j.jones 2006-12-29 09:39) - PLID 23160 - altered shifting functions to support revenue code
// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
void ShiftInsBalance(int ID, long PatientID, int SrcInsPartyID, int DstInsPartyID, CString strLineType, CString strAuditFromProcess, bool bShowDlg /*= true*/, long nRevenueCode /*= -1*/) {
	//DRT 5/8/03 - Lots of changes.
	//		1)  New parameter:  bShowDlg.  If this is true (default), it acts as it used to, giving you the
	//		opportunity to prompt the user for an amount.  If it is false, ShiftInsuranceResp is called
	//		with the full responsibility of SrcInsPartyID
	//		2)  SrcRespType and DstRespType have been changed to (a more logical) SrcInsPartyID and 
	//		DstInsPartyID.  Why it hasn't always done this, I have no idea.
	//		Patient Responsibility stays the same, using -1 to denote no insured party.
	//		3)  The source resp balance is calculated outside the dialog and passed in for use.  This
	//		allows us to use the bShowDlg option to not show anything, and just use the whole balance.
	//		In this case, the shift date is set to COleDateTime::GetCurrentTime().

	//if this is empty, someone coded this call incorrectly
	ASSERT(!strAuditFromProcess.IsEmpty());

	//calculate the balance from the source responsibility out here
	COleCurrency cySourceBalance(0, 0);

	//load the total source resp. amount
	try {

		CString str;

		//the "Bill" if statement will also fire if somehow the caller is trying to shift by
		//revenue code without providing a revenue code - shouldn't happen without bad code,
		//but might as well handle it
		_RecordsetPtr rs;

		if (strLineType == "Bill" || (strLineType == "RevCode" && nRevenueCode == -1)) {
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
			rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillID = {INT}", PatientID, ID);
		}
		else if(strLineType == "RevCode" && nRevenueCode != -1) {
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
			rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillID = {INT} AND ChargesT.ID IN "
				"	(SELECT ChargesT.ID FROM ChargesT "
				"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
				"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
				"	LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
				"	LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
				"		INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
				"	LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
				"	WHERE (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT})", PatientID, ID, nRevenueCode);
		}
		else {	//charges
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
				"AND ChargesT.ID = {INT}", PatientID, ID);
		}

		while (!rs->eof) {
			long ChargeID = AdoFldLong(rs, "ID");

			COleCurrency cyTotalResp, cyPayments, cyAdjustments, cyRefunds, cyBalance, cyInsurance;

			if (SrcInsPartyID != -1) {
				//we are xferring from an insurance responbility (non-patient)

				if (!GetChargeInsuranceTotals(ChargeID, PatientID, SrcInsPartyID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
			}
			else {
				//patient responsibility
				if (!GetChargeTotals(ChargeID, PatientID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
				cyTotalResp -= cyInsurance;
			}

			cySourceBalance += (cyTotalResp - cyPayments - cyAdjustments - cyRefunds);

			rs->MoveNext();
		}
		rs->Close();

	} NxCatchAll("Error determining source responsibility");

	//DRT 9/30/03 - PLID 9499 - Stop them if they try to shift $0, we currently don't allow that anyways.
	//		(It never actually changed data if you did it previously)
	if(cySourceBalance == COleCurrency(0, 0)) {
		MsgBox("You cannot shift a $0 balance to another responsibility.");
	}
	else {
		if(bShowDlg) {
			//Show the dialog!
			CShiftInsRespsDlg dlg(NULL);
			dlg.m_ID = ID;
			dlg.m_PatientID = PatientID;
			// (j.jones 2013-08-21 08:44) - PLID 58194 - pass in our audit string
			dlg.m_strAuditFromProcess = strAuditFromProcess;

			dlg.m_cySrcAmount = cySourceBalance;
			dlg.m_nSrcInsPartyID = SrcInsPartyID;
			dlg.m_nDstInsPartyID = DstInsPartyID;
			if(strLineType == "RevCode" && nRevenueCode != -1)
				dlg.m_nRevenueCode = nRevenueCode;

			dlg.m_strLineType = strLineType;
			dlg.DoModal();
		}
		else {
			//call shift without doing anything with the dialog
			// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
			ShiftInsuranceResponsibility(ID, PatientID, SrcInsPartyID, DstInsPartyID, strLineType, cySourceBalance,
				strAuditFromProcess, COleDateTime::GetCurrentTime(), strLineType == "RevCode" ? nRevenueCode : -1);
		}
	}
}

// (j.jones 2006-12-29 09:39) - PLID 23160 - altered shifting functions to support revenue code
// (j.jones 2007-08-03 09:55) - PLID 25844 - Modified to allow flag to follow the "require copay" setting.
// (j.jones 2010-08-03 15:16) - PLID 39938 - replaced the "prompt for copay" flag with a PayGroupID
// (j.jones 2011-03-21 17:46) - PLID 24273 - added flag to indicate that this was caused by an EOB
// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
void ShiftInsuranceResponsibility(int ID, long PatientID, long nSrcInsPartyID, long nDstInsPartyID, const CString& strLineType, COleCurrency cyAmtToShift,
								  CString strAuditFromProcess, COleDateTime dtDateOfShift,
								  long nRevenueCode /*= -1*/, long nOnlyShiftPayGroupID /*= -1*/, BOOL bIsERemitPosting /*= FALSE*/)
{
	//if this is empty, someone coded this call incorrectly
	ASSERT(!strAuditFromProcess.IsEmpty());

	//DRT 5/8/03 - Changes:
	//		1)  Source/Dest ins types are now changed to insured party ID's.

	//DRT 9/30/03 - PLID 9499 - Stop them if they try to shift $0, we currently don't allow that anyways.
	//		(It never actually changed data if you did it previously)
	if(cyAmtToShift == COleCurrency(0, 0)) {
		MsgBox("You cannot shift a $0 balance to another responsibility.");
		return;
	}

	// (j.jones 2015-11-05 12:31) - PLID 63866 - ensure this is rounded
	RoundCurrency(cyAmtToShift);

	_RecordsetPtr rs, rsResp, rsIns;
	COleCurrency cyTotalResp, cyPayments, cyAdjustments, cyRefunds, cyBalance, cyInsurance;
	_variant_t var;
	long nBillID;
	long nSourceRespID, nDestRespID;

	if (strLineType == "Bill" || strLineType == "RevCode") {
		nBillID = ID;		
	}
	else {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",ID);
		if(!rs->eof) {
			nBillID = AdoFldLong(rs, "BillID");
		}
		rs->Close();
	}

	// (j.jones 2005-10-27 16:34) - PLID 17292 - in the case where we are trying to shift a balance
	// from patient to any other resp and there is NO insurance selected on the bill, then call
	// SwapInsuranceCompanies to force an insurance selection on the bill
	// (j.jones 2009-09-21 11:56) - PLID 35564 - InsuredPartyID can no longer be -1
	if(nSrcInsPartyID == -1) {
		// (j.jones 2009-10-07 09:19) - PLID 36424 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM BillsT WHERE ID = {INT} AND InsuredPartyID Is Null", nBillID);
		if(!rs->eof) {
			SwapInsuranceCompanies(nBillID, nSrcInsPartyID, nDstInsPartyID, FALSE);
		}
		rs->Close();
	}

	// (j.jones 2015-11-24 09:39) - PLID 67613 - track the RespTypeT.CategoryType and Priority,
	// for Category use -1 for patient, in data Inactive is always 1
	long nSourcePriority = -1, nDestPriority = -1;
	long nSourceCategory = -1, nDestCategory = -1;

	// (j.jones 2007-02-21 09:16) - PLID 24696 - for auditing
	long nAuditID = -1;
	CString strPatientName = GetExistingPatientName(PatientID);
	CString strSource, strDest;	
	if(nSrcInsPartyID == -1) {
		strSource = "Patient";
	}
	else {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		// (j.jones 2015-11-24 09:39) - PLID 67613 - track the RespTypeT.CategoryType and Priority
		_RecordsetPtr rs = CreateParamRecordset("SELECT RespTypeT.TypeName, InsuranceCoT.Name, "
			"RespTypeT.CategoryType, RespTypeT.Priority "
			"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PersonID = {INT}", nSrcInsPartyID);
		if(!rs->eof) {
			strSource.Format("%s (%s)", AdoFldString(rs, "Name",""), AdoFldString(rs, "TypeName",""));
			nSourceCategory = AdoFldLong(rs, "CategoryType");
			nSourcePriority = AdoFldLong(rs, "Priority");
		}
		rs->Close();
	}
	if(nDstInsPartyID == -1) {
		strDest = "Patient";
	}
	else {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		// (j.jones 2015-11-24 09:39) - PLID 67613 - track the RespTypeT.CategoryType and Priority
		_RecordsetPtr rs = CreateParamRecordset("SELECT RespTypeT.TypeName, InsuranceCoT.Name, "
			"RespTypeT.CategoryType, RespTypeT.Priority "
			"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PersonID = {INT}", nDstInsPartyID);
		if(!rs->eof) {
			strDest.Format("%s (%s)", AdoFldString(rs, "Name",""), AdoFldString(rs, "TypeName",""));
			nDestCategory = AdoFldLong(rs, "CategoryType");
			nDestPriority = AdoFldLong(rs, "Priority");
		}
		rs->Close();
	}

	//This string is used only for writing to ChargeRespDetailT, which we don't ever want to have a time in it.
	CString strDate = FormatDateTimeForSql(COleDateTime(dtDateOfShift.GetYear(),dtDateOfShift.GetMonth(),dtDateOfShift.GetDay(),0,0,0));

	//the "Bill" if statement will also fire if somehow the caller is trying to shift by
	//revenue code without providing a revenue code - shouldn't happen without bad code,
	//but might as well handle it
	// (j.jones 2007-08-03 09:56) - PLID 25844 - Added join to CPTCodeT and selection of PromptForCoPay flag.	
	if (strLineType == "Bill" || (strLineType == "RevCode" && nRevenueCode == -1)) {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		// (j.jones 2010-08-03 10:33) - PLID 39938 - removed PromptForCoPay flag, replaced with the PayGroupID for the source insured party
		// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillID = {INT}", nSrcInsPartyID, PatientID, ID);
	}
	else if(strLineType == "RevCode" && nRevenueCode != -1) {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		// (j.jones 2010-08-03 10:33) - PLID 39938 - removed PromptForCoPay flag, replaced with the PayGroupID for the source insured party
		// (j.jones 2011-09-13 15:37) - PLID 44887 - skip original & void charges
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillID = {INT} AND ChargesT.ID IN "
			"	(SELECT ChargesT.ID FROM ChargesT "
			"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
			"	LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"	LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"		INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"	WHERE (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT})",
			nSrcInsPartyID, PatientID, ID, nRevenueCode);
	}
	else {
		// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
		// (j.jones 2010-08-03 10:33) - PLID 39938 - removed PromptForCoPay flag, replaced with the PayGroupID for the source insured party
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND ChargesT.ID = {INT}", nSrcInsPartyID, PatientID, ID);
	}

	while (!rs->eof && cyAmtToShift > COleCurrency(0,0)) {
		
		long nChargeID = AdoFldLong(rs, "ID");

		// (j.jones 2010-08-03 15:21) - PLID 39938 - the copay logic has changed to use pay groups,
		// now we are passed in a nOnlyShiftPayGroupID, our recordset calculated the PayGroupID
		// (could be NULL) for the source insured party, so shift accordingly if they match
		
		long nPayGroupID = AdoFldLong(rs, "PayGroupID", -1);

		if(nOnlyShiftPayGroupID != -1 && nOnlyShiftPayGroupID != nPayGroupID) {
			//we're asked to shift a certain pay group ID, and this charge
			//doesn't use it for the source insured party, so skip this charge
			rs->MoveNext();
			continue;
		}

		if (nSrcInsPartyID != -1) {
			//insurance resp

			if (!GetChargeInsuranceTotals(nChargeID, PatientID, nSrcInsPartyID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds)) {
				// This should never happen, but just in case...
				rs->MoveNext();
				continue;
			}
		}
		else {
			if (!GetChargeTotals(nChargeID, PatientID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
				// This should never happen, but just in case...
				rs->MoveNext();
				continue;
			}
			cyTotalResp -= cyInsurance;
		}

		COleCurrency cyRespBalance = cyTotalResp - cyPayments - cyAdjustments - cyRefunds;

		//if this particular charge's balance is zero, then we skip this charge
		if(cyRespBalance == COleCurrency(0,0)) {
			rs->MoveNext();
			continue;
		}
		
		// Remove balance from source insurance responsibility
		if(nSrcInsPartyID != -1) {
			//insurance resp
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			rsResp = CreateParamRecordset("SELECT ChargeRespT.ID AS RespID FROM ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID WHERE ChargeID = {INT} AND InsuredPartyT.PersonID = {INT}", nChargeID, nSrcInsPartyID);
		}
		else {
			//patient resp
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			rsResp = CreateParamRecordset("SELECT ChargeRespT.ID AS RespID FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID Is Null", nChargeID);
		}
		
		if(!rsResp->eof) {
			nSourceRespID = AdoFldLong(rsResp, "RespID", -1);
		}
		else {
			rsResp->Close();
			rs->MoveNext();
			continue;
		}
		rsResp->Close();

		// (j.jones 2008-02-12 15:34) - PLID 28848 - if the preferences are set to disallow the "batched" setting
		// on patient charges, we should auto-set that option when shifted to insurance, provided that the insurance
		// resp was previously zero

		//check the preferences prior to the transaction, so they don't access data later
		BOOL bHidePatientChargesOnClaims = (GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1 &&
				GetRemotePropertyInt("HidePatientChargesOnClaims",0,0,"<None>",TRUE) == 1);	

		BEGIN_TRANS("ShiftInsBalance") {

			COleCurrency cyChgAmtToTransfer = COleCurrency(0,0);

			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			rsResp = CreateParamRecordset("SELECT Amount FROM ChargeRespT WHERE ID = {INT}", nSourceRespID);
			if(!rsResp->eof) {
				if(cyRespBalance < cyAmtToShift) 	//if the remaining balance for this resp is less than the total, set the amt to xfer to be our balance
					cyChgAmtToTransfer = cyRespBalance;
				else	//remaining balance is > than the total needing shifted, so we can satisfy the whole shift with this item
					cyChgAmtToTransfer = cyAmtToShift;

				// (j.jones 2008-02-12 15:38) - PLID 28848 - now batch the charges if necessary
				if(bHidePatientChargesOnClaims && nDstInsPartyID != -1 && cyChgAmtToTransfer > COleCurrency(0,0)) {
					EnsurePatientChargesBatchedUponShift(nChargeID);
				}

				// (j.jones 2009-10-07 08:49) - PLID 36424 - parameterized
				ExecuteParamSql("UPDATE ChargeRespT SET Amount = Amount - Convert(money, {STRING}) WHERE ID = {INT}", FormatCurrencyForSql(cyChgAmtToTransfer), nSourceRespID);
			}

			//update ChargeRespDetailsT

			// (j.jones 2006-11-01 10:59) - PLID 23301 - we need to shift from the BALANCE of a resp detail,
			// counting applies, as opposed from randomly shifting any amount we please.
			// *Note: this is almost same query as in ApplyToDetails, and so if one changes, the other should change too
			// (except this one orders by Date descending, and also selects the full amount)
			// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
			_RecordsetPtr rsRespDetails = CreateParamRecordset("SELECT ChargeRespDetailT.ID, ChargeRespDetailT.Amount AS FullAmount, "
				"(ChargeRespDetailT.Amount - CASE WHEN ApplyDetailsQ.Amount IS NULL THEN 0 ELSE ApplyDetailsQ.Amount END) AS Balance "
				"FROM ChargeRespDetailT "
				"LEFT JOIN (SELECT DetailID, Sum(CASE WHEN Amount IS NULL THEN 0 ELSE Amount END) AS Amount "
					"FROM ApplyDetailsT "
					"WHERE DetailID IN (SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = {INT}) "
					"GROUP BY DetailID) ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
				"WHERE ChargeRespID = {INT} "
				"ORDER BY Date DESC", nSourceRespID, nSourceRespID);

			long nDetailID;
			COleCurrency cyDetailAmtToTransfer, cyAmtLeftToShift;
			COleCurrency cyDetailFullAmount = COleCurrency(0,0);
			COleCurrency cyDetailBalance = COleCurrency(0,0);
			COleDateTime dtDate;

			cyAmtLeftToShift = cyChgAmtToTransfer;

			while (! rsRespDetails->eof && cyAmtLeftToShift > COleCurrency(0,0))  {

				//get the information
				nDetailID = AdoFldLong(rsRespDetails, "ID");
				cyDetailFullAmount = AdoFldCurrency(rsRespDetails, "FullAmount");
				cyDetailBalance = AdoFldCurrency(rsRespDetails, "Balance");

				//see if we need all of it
				if (cyDetailBalance >=  cyAmtLeftToShift) {
					//we have available the amount we need, so only use the amount we need
					cyDetailAmtToTransfer = cyAmtLeftToShift;
				}
				//TS 5/14/03:  We need to set cyDetailAmtToTransfer in any case, right?  Am I crazy?
				else {
					cyDetailAmtToTransfer = cyDetailBalance;
				}

				//add it to the total
				cyAmtLeftToShift = cyAmtLeftToShift - cyDetailAmtToTransfer;

				//we always need to know the total amount applied, if any
				_RecordsetPtr rsApplyTotal = CreateParamRecordset("SELECT ChargeRespDetailT.Amount AS RespAmount, Sum(ApplyDetailsT.Amount) AS TotalApplyAmount "
					"FROM ApplyDetailsT "
					"INNER JOIN ChargeRespDetailT ON ApplyDetailsT.DetailID = ChargeRespDetailT.ID "
					"WHERE ChargeRespDetailT.ID = {INT} "
					"GROUP BY ChargeRespDetailT.ID, ChargeRespDetailT.Amount", nDetailID);

				//check to see if the responsibility would now be 0
				if (cyDetailFullAmount == cyDetailAmtToTransfer) {

					//the amounts are equal which means the detail would be 0, so just delete it
					//make sure that nothing is applied to this first
					if (rsApplyTotal->eof) {
						ExecuteParamSql("DELETE FROM ChargeRespDetailT WHERE ID = {INT}", nDetailID);
					}
					else {
						//this should not be possible unless the applied amount is zero, fail if it is not zero
						COleCurrency cyApplyTotal = VarCurrency(rsApplyTotal->Fields->Item["TotalApplyAmount"]->Value);
						if (cyApplyTotal == COleCurrency(0, 0)) {
							//it is zero, we can safely update this
							ExecuteParamSql("UPDATE ChargeRespDetailT SET Amount = 0 WHERE ID = {INT}", nDetailID);
						}
						else {
							//this should not be possible - throw a detailed exception

							//this function will get the current call stack, which is translateable using
							//nx-internal\Development\CallStackSymbolizer
							CString strCallStack = GetCallStack();

							ThrowNxException("ShiftInsuranceResponsibility (1) failed to update ChargeRespDetailT.ID %li to %s due to an ApplyDetailsT records totalling %s.\n\n"
								"%s ID = %li, PatientID = %li, nSrcInsPartyID = %li, nDstInsPartyID = %li, cyAmtToShift = %s, strAuditFromProcess = %s, "
								"dtDateOfShift = %s, nRevenueCode = %li, nOnlyShiftPayGroupID = %li, bIsERemitPosting = %li\n\n"
								"%s",
								nDetailID, FormatCurrencyForInterface(COleCurrency(0, 0)), FormatCurrencyForInterface(cyApplyTotal),
								strLineType, ID, PatientID, nSrcInsPartyID, nDstInsPartyID, FormatCurrencyForInterface(cyAmtToShift), strAuditFromProcess,
								FormatDateTimeForInterface(dtDateOfShift), nRevenueCode, nOnlyShiftPayGroupID, bIsERemitPosting ? 1 : 0,
								strCallStack);
						}
					}
				}
				else {

					//first make sure we don't have applies exceeding this amount
					if (!rsApplyTotal->eof) {
						COleCurrency cyRespAmount = VarCurrency(rsApplyTotal->Fields->Item["RespAmount"]->Value);
						COleCurrency cyApplyTotal = VarCurrency(rsApplyTotal->Fields->Item["TotalApplyAmount"]->Value);
						if (cyApplyTotal > (cyRespAmount - cyDetailAmtToTransfer)) {

							//this should not be possible - throw a detailed exception

							//this function will get the current call stack, which is translateable using
							//nx-internal\Development\CallStackSymbolizer
							CString strCallStack = GetCallStack();

							ThrowNxException("ShiftInsuranceResponsibility (2) failed to update ChargeRespDetailT.ID %li from %s to %s due to ApplyDetailsT records totalling %s.\n\n"
								"%s ID = %li, PatientID = %li, nSrcInsPartyID = %li, nDstInsPartyID = %li, cyAmtToShift = %s, strAuditFromProcess = %s, "
								"dtDateOfShift = %s, nRevenueCode = %li, nOnlyShiftPayGroupID = %li, bIsERemitPosting = %li\n\n"
								"%s",
								nDetailID, FormatCurrencyForInterface(cyRespAmount), FormatCurrencyForInterface(cyRespAmount - cyDetailAmtToTransfer), FormatCurrencyForInterface(cyApplyTotal),
								strLineType, ID, PatientID, nSrcInsPartyID, nDstInsPartyID, FormatCurrencyForInterface(cyAmtToShift), strAuditFromProcess,
								FormatDateTimeForInterface(dtDateOfShift), nRevenueCode, nOnlyShiftPayGroupID, bIsERemitPosting ? 1 : 0,
								strCallStack);
						}
					}

					//Update the charge resp					
					ExecuteParamSql("UPDATE ChargeRespDetailT SET Amount = Amount - Convert(money, {STRING}) WHERE ID = {INT}", FormatCurrencyForSql(cyDetailAmtToTransfer), nDetailID);
				}

				rsRespDetails->MoveNext();
			}

			// Add balance to destination insurance responsibility
			if(nDstInsPartyID != -1) {
				//ins resp
				// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
				rsResp = CreateParamRecordset("SELECT ChargeRespT.ID AS RespID FROM ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID WHERE ChargeID = {INT} AND InsuredPartyT.PersonID = {INT}", nChargeID, nDstInsPartyID);
			}
			else {
				//patient resp
				// (j.jones 2009-10-06 14:13) - PLID 36424 - parameterized
				rsResp = CreateParamRecordset("SELECT ChargeRespT.ID AS RespID FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID IS Null", nChargeID);
			}

			if(!rsResp->eof)
				nDestRespID = AdoFldLong(rsResp, "RespID", -1);
			else
				nDestRespID = -1;
			rsResp->Close();

			if(nDestRespID == -1){  //This resp did not already exist

				_variant_t vtInsPartyID = (nDstInsPartyID == -1) ? g_cvarNull : nDstInsPartyID;

				// (j.jones 2009-10-07 08:49) - PLID 36424 - parameterized
				// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				//	We just inserted a new ChargeRespT record, there is no way a detail could exist at this point,
				//	so lets clean up our transactions a bit here.
				ExecuteParamSql(
					"BEGIN TRAN\r\n"
					"DECLARE @ChargeRespID INT\r\n"
					"INSERT INTO ChargeRespT (ChargeID, Amount, InsuredPartyID) VALUES ({INT}, {OLECURRENCY}, {VT_I4})\r\n"
					"SET @ChargeRespID = SCOPE_IDENTITY()\r\n"
					"\r\n"
					"INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) VALUES (@ChargeRespID, {OLECURRENCY}, {STRING})\r\n"
					"COMMIT TRAN",
					nChargeID, cyChgAmtToTransfer, vtInsPartyID,
					cyChgAmtToTransfer, strDate);
			}
			else
			{	
				//make sure there are not applies greater than this amount
				_RecordsetPtr rsApplies = CreateParamRecordset("SELECT ChargeRespDetailT.ID, "
					"ChargeRespDetailT.Amount AS CurrentTotal, (ChargeRespDetailT.Amount + {OLECURRENCY}) AS DesiredTotal, "
					"ApplyDetailsQ.TotalApplied "
					"FROM ChargeRespDetailT "
					"INNER JOIN ("
					"	SELECT ApplyDetailsT.DetailID, Sum(ApplyDetailsT.Amount) AS TotalApplied "
					"	FROM ApplyDetailsT "
					"	GROUP BY ApplyDetailsT.DetailID "
					") AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
					"WHERE ChargeRespDetailT.ChargeRespID = {INT} AND ChargeRespDetailT.Date = {STRING} "
					"AND ApplyDetailsQ.TotalApplied > (ChargeRespDetailT.Amount + {OLECURRENCY})",
					cyChgAmtToTransfer, nDestRespID, strDate, cyChgAmtToTransfer);
				if (!rsApplies->eof) {
					//this should not be possible - throw a detailed exception

					long nChargeRespDetailID = AdoFldLong(rsApplies->Fields, "ID");
					COleCurrency cyCurrentTotal = AdoFldCurrency(rsApplies->Fields, "CurrentTotal");
					COleCurrency cyDesiredTotal = AdoFldCurrency(rsApplies->Fields, "DesiredTotal");
					COleCurrency cyTotalApplied = AdoFldCurrency(rsApplies->Fields, "TotalApplied");

					//this function will get the current call stack, which is translateable using
					//nx-internal\Development\CallStackSymbolizer
					CString strCallStack = GetCallStack();

					ThrowNxException("ShiftInsuranceResponsibility (3) failed to update ChargeRespDetailT.ID %li from %s to %s for date %s due to a total ApplyDetailsT amount of %s.\n\n"
						"%s ID = %li, PatientID = %li, nSrcInsPartyID = %li, nDstInsPartyID = %li, cyAmtToShift = %s, strAuditFromProcess = %s, "
						"dtDateOfShift = %s, nRevenueCode = %li, nOnlyShiftPayGroupID = %li, bIsERemitPosting = %li\n\n"
						"%s",
						nChargeRespDetailID, FormatCurrencyForInterface(cyCurrentTotal), FormatCurrencyForInterface(cyDesiredTotal), strDate, FormatCurrencyForInterface(cyTotalApplied),
						strLineType, ID, PatientID, nSrcInsPartyID, nDstInsPartyID, FormatCurrencyForInterface(cyAmtToShift), strAuditFromProcess,
						FormatDateTimeForInterface(dtDateOfShift), nRevenueCode, nOnlyShiftPayGroupID, bIsERemitPosting ? 1 : 0,
						strCallStack);
				}
				rsApplies->Close();

				//This resp already exists
				// (j.jones 2009-10-07 08:49) - PLID 36424 - parameterized
				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				ExecuteParamSql(
					"BEGIN TRAN\r\n"
					"DECLARE @DestRespID INT	SET @DestRespID = {INT}\r\n"
					"DECLARE @Amount MONEY		SET @Amount = {OLECURRENCY}\r\n"
					"DECLARE @Date DATETIME		SET @Date = {STRING}\r\n"

					"UPDATE ChargeRespT SET Amount = Amount + @Amount WHERE ID = @DestRespID\r\n"

					"UPDATE ChargeRespDetailT SET Amount = Amount + @Amount WHERE ChargeRespID = @DestRespID AND Date = @Date\r\n"
					
					"IF @@ROWCOUNT = 0\r\n"
					"BEGIN\r\n"
					"	INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) VALUES (@DestRespID, @Amount, @Date)\r\n"
					"END\r\n"
					"COMMIT TRAN",
					nDestRespID, cyChgAmtToTransfer, strDate);
			}

			cyAmtToShift -= cyChgAmtToTransfer;

			{
				// (j.jones 2015-11-24 09:44) - PLID 67613 - try to update the AllowableInsuredPartyID for this charge
				// if we shifted across categories, or shifted FROM patient, or shifted to/from inactive (they have no category)
				// but not if we shifted TO patient, or within active parties in the same category
				
				//if we shifted TO patient, we don't recalculate,
				//so don't do this unless we are shifting to an insured party
				if(nDstInsPartyID != -1) {

					//did we shift from patient, between categories, or between any inactive resp?
					if (nSrcInsPartyID == -1 || nSourceCategory != nDestCategory
						|| nSourcePriority == -1 || nDestPriority == -1) {


						//recalculate the AllowableInsuredPartyID for this charge
						ExecuteParamSql("UPDATE ChargesT SET AllowableInsuredPartyID = dbo.CalcMajorityInsCategoryPrimaryInsuredParty(ID) "
							"WHERE ID = {INT}", nChargeID);
					}
				}
			}

			//throw an exception if any ChargeRespT record for this charge mismatches the ChargeRespDetailT total underneath it,
			//since this is in a transaction, this error will undo the shifting entirely
			_RecordsetPtr rsBadResps = CreateParamRecordset("SELECT ChargeRespT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount, "
				"Sum(IsNull(ChargeRespDetailT.Amount, Convert(money,0))) AS DetailTotal "
				"FROM ChargeRespT "
				"LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
				"WHERE ChargeRespT.ChargeID = {INT} "
				"GROUP BY ChargeRespT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount "
				"HAVING ChargeRespT.Amount <> Sum(IsNull(ChargeRespDetailT.Amount, Convert(money, 0)))", nChargeID);
			if (!rsBadResps->eof) {
				
				CString strErrorText;
				strErrorText.Format("ShiftInsuranceResponsibilities could not save Charge ID %li because its ChargeRespDetailT responsibilites do not match the ChargeRespT responsibility total.\n\n", nChargeID);
				
				//loop through incase we broke multiple resps at once
				while (!rsBadResps->eof) {
					
					long nChargeRespID = AdoFldLong(rsBadResps->Fields, "ID");
					long nInsuredPartyID = AdoFldLong(rsBadResps->Fields, "InsuredPartyID", -1);
					COleCurrency cyChargeRespAmount = AdoFldCurrency(rsBadResps->Fields, "Amount");
					COleCurrency cyChargeRespDetailTotal = AdoFldCurrency(rsBadResps->Fields, "DetailTotal");

					CString strBadResp;
					strBadResp.Format("ChargeRespT.ID = %li, ChargeRespT.InsuredPartyID = %li, "
						"ChargeRespT.Amount = %s, ChargeRespDetailT Total = %s\n\n",
						nChargeRespID, nInsuredPartyID, FormatCurrencyForInterface(cyChargeRespAmount), FormatCurrencyForInterface(cyChargeRespDetailTotal));

					strErrorText += strBadResp;

					rsBadResps->MoveNext();
				}

				//this function will get the current call stack, which is translateable using
				//nx-internal\Development\CallStackSymbolizer
				CString strCallStack = GetCallStack();	
				strErrorText += strCallStack;
				ThrowNxException(strErrorText);
			}
			rsBadResps->Close();

			// (j.jones 2007-02-21 09:07) - PLID 24696 - audit shifting the charge responsibility		
			COleCurrency cyChargeTotal;
			GetChargeTotals(nChargeID, PatientID, &cyChargeTotal, 0, 0, 0, 0);
			if(nAuditID == -1)
				nAuditID = BeginNewAuditEvent();
			CString strNewValue;
			// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why,
			// so that this audit description was more clear as to what dialog and action shifted the money
			strNewValue.Format("%s (of a %s charge) shifted from %s to %s, %s.",
				FormatCurrencyForInterface(cyChgAmtToTransfer),
				FormatCurrencyForInterface(cyChargeTotal),
				strSource, strDest,
				strAuditFromProcess);
			// (j.jones 2011-03-21 17:51) - PLID 24273 - added a unique audit for when an EOB causes this
			AuditEvent(PatientID, strPatientName, nAuditID, bIsERemitPosting ? aeiResponsibilityShiftedByERemitPosting : aeiResponsibilityShifted, nChargeID, "", strNewValue, aepHigh, aetChanged);

		} END_TRANS_CATCH_ALL("ShiftInsuranceResponsibility");
		
		rs->MoveNext();
	}
	CClient::RefreshTable(NetUtils::PatBal, PatientID);
}

/*JJ - this function is used if we wish to increase the insurance responsibility of a charge. It can be used
in AutoApply functions when the user wishes to shift patient balance to insurance.*/
BOOL IncreaseInsBalance(long ChargeID, long PatientID, long InsID, COleCurrency cyAmountToIncrease) {

	CString str;
	COleCurrency cyPatRespAmt;

	//Get the amount of the patient resp in ChargeRespT, if any
	//DRT 10/21/2008 - PLID 31774 - Parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Amount FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID Is NULL", ChargeID);
	if(!rs->eof) {
		cyPatRespAmt = COleCurrency(rs->Fields->Item["Amount"]->Value.cyVal);
	}
	else {
		cyPatRespAmt = COleCurrency(0,0);
	}
	rs->Close();
	
	//this will happen if there is no patient resp, or if it is less than the amount we are trying to increase
	if(cyAmountToIncrease > cyPatRespAmt) {
		// (j.jones 2011-11-02 16:30) - PLID 38686 - removed an unnecessary \n
		str.Format("You are trying to transfer %s from patient to insurance responsibility, but the patient responsibility is only %s. The change will not be made.",FormatCurrencyForInterface(cyAmountToIncrease),FormatCurrencyForInterface(cyPatRespAmt));
		AfxMessageBox(str);
		return FALSE;
	}

	// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
	ShiftInsuranceResponsibility(ChargeID, PatientID, -1, InsID, "Charge", cyAmountToIncrease,
		"to support a manually created apply", COleDateTime::GetCurrentTime());

	return TRUE;
}

// (j.jones 2013-08-20 12:13) - PLID 39987 - Added EnsureInsuranceResponsibility, which shifts as needed
// from the source resp. to make the dest. resp. equal to the desired responsibility.
// It may only shift a portion of the money, or perhaps none at all if the resp. is <= what we are wanting.
//
// If the dest. resp is greater than our desired amount, we do nothing unless bForceExactResp is true.
// If bForceExactResp is enabled, we would shift any overage from the dest. resp. back to the source resp.
// to ensure the dest. resp. is == to our desired amount.
//
// Returns true if the responsibility could be successfully acquired (or if nothing changed because it was already available),
// returns false if we could not acquire the desired responsibility, likely due to existing applies that prevented shifting.

// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
bool EnsureInsuranceResponsibility(int ID, long PatientID, long nSrcInsPartyID, long nDstInsPartyID, const CString& strLineType, COleCurrency cyDesiredRespAmount,
								   CString strAuditFromProcess, COleDateTime dtDateOfShift,
								   bool bForceExactResp /*= false*/, long nRevenueCode /*= -1*/,
								   long nOnlyShiftPayGroupID /*= -1*/, BOOL bIsERemitPosting /*= FALSE*/)
{
	//if this is empty, someone coded this call incorrectly
	ASSERT(!strAuditFromProcess.IsEmpty());

	long nBillID;
	if (strLineType == "Bill" || strLineType == "RevCode") {
		nBillID = ID;		
	}
	else {
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",ID);
		if(!rs->eof) {
			nBillID = AdoFldLong(rs, "BillID");
		}
		rs->Close();
	}

	_RecordsetPtr rs;

	if (strLineType == "Bill" || (strLineType == "RevCode" && nRevenueCode == -1)) {
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillID = {INT}", nSrcInsPartyID, PatientID, ID);
	}
	else if(strLineType == "RevCode" && nRevenueCode != -1) {
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillID = {INT} AND ChargesT.ID IN "
			"	(SELECT ChargesT.ID FROM ChargesT "
			"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "		
			"	LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"	LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"		INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID AND ServiceRevCodesT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"	WHERE (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT})",
			nSrcInsPartyID, PatientID, ID, nRevenueCode);
	}
	else {
		rs = CreateParamRecordset("SELECT ChargesT.ID, Coalesce(InsPayGroupLinkQ.PayGroupID, ServiceT.PayGroupID) AS PayGroupID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"AND ChargesT.ID = {INT}", nSrcInsPartyID, PatientID, ID);
	}

	COleCurrency cyTotalExistingDestResponsibility = COleCurrency(0,0);
	COleCurrency cyTotalSourceRespBalance = COleCurrency(0,0);
	COleCurrency cyTotalDestRespBalance = COleCurrency(0,0);

	while(!rs->eof) {
		long nChargeID = AdoFldLong(rs, "ID");
		long nPayGroupID = AdoFldLong(rs, "PayGroupID", -1);

		if(nOnlyShiftPayGroupID != -1 && nOnlyShiftPayGroupID != nPayGroupID) {
			//we're asked to shift a certain pay group ID, and this charge
			//doesn't use it for the source insured party, so skip this charge
			rs->MoveNext();
			continue;
		}

		//calculate the total responsibility for this destination resp
		{
			COleCurrency cyTotalResp, cyPayments, cyAdjustments, cyRefunds, cyBalance, cyInsurance;
			if (nDstInsPartyID != -1) {
				//insurance resp
				if (!GetChargeInsuranceTotals(nChargeID, PatientID, nDstInsPartyID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
			}
			else {
				if (!GetChargeTotals(nChargeID, PatientID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
				cyTotalResp -= cyInsurance;
			}

			cyTotalExistingDestResponsibility += cyTotalResp;
			cyTotalDestRespBalance += (cyTotalResp - cyPayments - cyAdjustments - cyRefunds);
		}

		//calculate the total balance for this source resp
		{
			COleCurrency cyTotalResp, cyPayments, cyAdjustments, cyRefunds, cyBalance, cyInsurance;
			if (nSrcInsPartyID != -1) {
				//insurance resp
				if (!GetChargeInsuranceTotals(nChargeID, PatientID, nSrcInsPartyID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
			}
			else {
				if (!GetChargeTotals(nChargeID, PatientID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
				cyTotalResp -= cyInsurance;
			}

			cyTotalSourceRespBalance += (cyTotalResp - cyPayments - cyAdjustments - cyRefunds);
		}

		rs->MoveNext();
	}

	if(cyTotalExistingDestResponsibility == cyDesiredRespAmount) {
		//the current destination resp is exactly what we want it to be,
		//so do nothing
		return true;
	}
	else if(cyTotalExistingDestResponsibility < cyDesiredRespAmount) {
		//the current destination resp. is less than what we want it to be,
		//so try to shift enough to meet our desired amount
		COleCurrency cyAmtToShift = cyDesiredRespAmount - cyTotalExistingDestResponsibility;

		bool bShiftedFullAmount = true;
		if(cyTotalSourceRespBalance < cyAmtToShift) {
			//They've already paid off some of this resp., so we cannot possibly shift.
			//Reduce it so we shift what we can.
			cyAmtToShift = cyTotalSourceRespBalance;
			bShiftedFullAmount = false;

			if(cyAmtToShift == COleCurrency(0,0)) {
				return false;
			}
		}

		// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
		ShiftInsuranceResponsibility(ID, PatientID, nSrcInsPartyID, nDstInsPartyID, strLineType, cyAmtToShift, strAuditFromProcess, dtDateOfShift, nRevenueCode, nOnlyShiftPayGroupID, bIsERemitPosting);
		//we already calculated whether the full amount was shiftable, so return our known result
		return bShiftedFullAmount;
	}
	else if(cyTotalExistingDestResponsibility > cyDesiredRespAmount && bForceExactResp) {
		//the current destination resp. is greater than what we want it to be,
		//and bForceExactResp is enabled, so try to shift the overage from the
		//dest. resp. back to the source resp.
		COleCurrency cyAmtToShift = cyTotalExistingDestResponsibility - cyDesiredRespAmount;

		bool bShiftedFullAmount = true;
		if(cyTotalDestRespBalance < cyAmtToShift) {
			//They've already paid off some of this resp., so we cannot possibly shift.
			//Reduce it so we shift what we can.
			cyAmtToShift = cyTotalDestRespBalance;
			bShiftedFullAmount = false;

			if(cyAmtToShift == COleCurrency(0,0)) {
				return false;
			}
		}

		// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
		ShiftInsuranceResponsibility(ID, PatientID, nDstInsPartyID, nSrcInsPartyID, strLineType, cyAmtToShift, strAuditFromProcess, dtDateOfShift, nRevenueCode, nOnlyShiftPayGroupID, bIsERemitPosting);
		//we already calculated whether the full amount was shiftable, so return our known result
		return bShiftedFullAmount;
	}

	//if we're here, we didn't shift anything, likely because the
	//amount was > than what we needed, and bForceExactResp was false
	return true;
}

// (j.jones 2006-12-28 13:52) - PLID 23160 - added support for adjusting based on grouping by revenue code

//LineItemType -  1: Bill, 2: Charge, 3: Revenue Code
// (j.jones 2008-04-29 09:45) - PLID 29744 - renamed Responsibility parameter to nRespTypeID
COleCurrency AdjustBalance(long nID, long nBillID, long nPatientID, int iLineItemType, long nRespTypeID, long nInsuredPartyID)
{
	// (j.jones 2008-04-29 09:44) - PLID 29744 - this function handled responsibilities all wrong,
	// it didn't account for inactive insurances properly at all, and didn't support responsibilities
	// beyond primary and secondary

	COleCurrency cyTotal, cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;

	if(iLineItemType == 1) {	//Bill
		if(nRespTypeID == -1) {
			//inactive insurance
			GetInactiveInsTotals(nInsuredPartyID, nBillID, -1, nPatientID, cyCharges, cyPayments);
			cyTotal = -(cyCharges - cyPayments);
		}
		else if(nRespTypeID == 0) {
			//patient resp
			if (!GetBillTotals(nBillID, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance);
		}
		else {
			// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
			if (!GetBillInsuranceTotals(nBillID, nPatientID, nInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			// cyCharges is the total insurance responsbility, less applies
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds);
		}
	}
	else if(iLineItemType == 2) {	//Charge
		if(nRespTypeID == -1) {
			//inactive insurance
			GetInactiveInsTotals(nInsuredPartyID, -1, nID, nPatientID, cyCharges, cyPayments);
			cyTotal = -(cyCharges - cyPayments);
		}
		else if(nRespTypeID == 0) {
			//patient resp
			if (!GetChargeTotals(nID, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance);
		}
		else {
			if (!GetChargeInsuranceTotals(nID, nPatientID, nInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			// cyCharges is the total insurance responsbility, less applies
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds);
		}
	}
	// (j.jones 2006-12-28 13:53) - PLID 23160 - added Revenue Code option
	else if(iLineItemType == 3) {	//Revenue Code
		if(nRespTypeID == -1) {
			//inactive insurance
			GetRevenueCodeInactiveInsTotals(nInsuredPartyID, nBillID, nID, nPatientID, cyCharges, cyPayments);
			cyTotal = -(cyCharges - cyPayments);
		}
		else if(nRespTypeID == 0) {
			//patient resp
			if (!GetRevenueCodeTotals(nID, nBillID, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance);
		}
		else {
			if (!GetRevenueCodeInsuranceTotals(nID, nBillID, nPatientID, nInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
				cyTotal.SetStatus(COleCurrency::invalid);
				return cyTotal;
			}
			// cyCharges is the total insurance responsbility, less applies
			cyTotal = -(cyCharges - cyPayments - cyAdjustments - cyRefunds);
		}
	}
	return cyTotal;
}

BOOL InvokeFinancialApplyDlg(int iDestID, int iSourceID, long PatientID, CString strClickedType)
{
	//DRT 5/29/03 - Removed the GuarantorID1 and GuarantorID2 parameters.  Instead of doing anything resembling
	//		making sense, this code was selecting the InsuredParty from data then comparing against those 2 params
	//		to see if it was pri or sec, instead of just pulling the InsType out of data along with the InsPartyID
	//		Plus it needed updated for our multiple responsibility stuff.

	// Apply the payment to the bill, charge, or some type of payment
	CFinancialApply dlg(NULL);
	COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
	COleCurrency cyAmount, cyOutgoing, cyIncoming;	

	long InsuredPartyID = -1;
	long nRespTypeID;
	CString str;
	_variant_t var;
	long nActivePatientID = PatientID;

	_RecordsetPtr rs;

	CWaitCursor cuWait;

	if (!CheckCurrentUserPermissions(bioPayment,sptRead))
		return FALSE;

	/* Get the insurance company of the payment. That will determine whether it
	goes to primary, secondary, inactive, or non-insurance */
	str.Format("SELECT InsCoNumber, RespTypeID FROM [_PatientPaymentsQ] WHERE [_PatientPaymentsQ].ID = %d",
		iSourceID);

	// (j.jones 2009-10-06 14:13) - PLID 36425 - parameterized
	// (j.jones 2013-07-22 11:06) - PLID 57653 - get the payment date
	rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.RespTypeID, LineItemT.Date "
		"FROM LineItemT "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
		"WHERE LineItemT.ID = {INT} "
		"AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3", iSourceID);
	COleDateTime dtPayment = g_cdtInvalid;
	if(rs->eof) {
		ThrowNxException("Invalid payment ID %li!", iSourceID);
	}
	else {
		InsuredPartyID = AdoFldLong(rs, "InsuredPartyID", -1);
		nRespTypeID = AdoFldLong(rs, "RespTypeID", -1);
		dtPayment = VarDateTime(rs->Fields->Item["Date"]->Value);
	}
	rs->Close();

	dlg.m_PatientID = nActivePatientID;
	dlg.m_boAdjustBalance = FALSE; //set this by default
	dlg.m_nResponsibility = 0;	//patient by default

	if (InsuredPartyID > 0) {
		//insurance exists
		dlg.m_nResponsibility = nRespTypeID;
	}

	if (strClickedType == "Bill") {
		/* Get the totals for a bill */
		switch (dlg.m_nResponsibility) {
		case -1:
			{
				//Inactive insurance
				GetInactiveInsTotals(InsuredPartyID, iDestID, -1, nActivePatientID, cyCharges, cyPayments);
				dlg.m_cyNetCharges = cyCharges - cyPayments;
			}
			break;
		case 0:
			{
				//Patient resp
				if (!GetBillTotals(iDestID, nActivePatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					return FALSE;
				}
				dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
			}
			break;
		default:
			{
				//Any other insurance resp
				// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
				if (!GetBillInsuranceTotals(iDestID, nActivePatientID, InsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
					return FALSE;
				}
				// cyCharges is the total insurance responsbility, less applies
				dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds;
			}
			break;
		}
		dlg.m_boShowAdjCheck = TRUE;
		dlg.m_boShowIncreaseCheck = FALSE;
	}
	else if (strClickedType == "Charge") {
		/* Get the totals for a charge */
		switch (dlg.m_nResponsibility) {
		case -1:
			//Inactive insurance
			GetInactiveInsTotals(InsuredPartyID, -1, iDestID, nActivePatientID, cyCharges, cyPayments);
			dlg.m_cyNetCharges = cyCharges - cyPayments;
			break;
		case 0:
			//Patient resp
			if (!GetChargeTotals(iDestID, nActivePatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
				return FALSE;
			}
			dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
			break;
		default:
			//any other insurance resp
			if (!GetChargeInsuranceTotals(iDestID, nActivePatientID, InsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
				return FALSE;
			}
			// cyCharges is the total insurance responsbility, less applies
			dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds;
			break;
		}
		dlg.m_boShowAdjCheck = TRUE;
		dlg.m_boShowIncreaseCheck = TRUE;
	}
	else {
		// Get the payment, adjustment, or refund total. This
		// is defined as: The original value, less all outgoing
		// applies, plus all applies directed to this payment/
		// adjustment/refund.			

		if (!GetPayAdjRefTotals(iDestID, nActivePatientID, cyAmount, cyOutgoing, cyIncoming)) {
			return FALSE;
		}
		cyAmount = cyAmount - cyOutgoing + cyIncoming;

		if (cyAmount < COleCurrency(0,0))
			cyAmount *= -1;

		dlg.m_cyNetCharges = cyAmount;
		dlg.m_nResponsibility = 0;
	}
/////////////////////////////////////////////////////////////
// Get totals for the pay/adj/ref we will apply from
/////////////////////////////////////////////////////////////
	if (!GetPayAdjRefTotals(iSourceID, nActivePatientID, cyAmount, cyOutgoing, cyIncoming)) {
		return FALSE;
	}
	dlg.m_cyNetPayment = cyAmount - cyOutgoing + cyIncoming;

	//before allowing them to apply, see if the source is a prepayment and the destination
	//is not a bill or charge created from the quote that the prepayment is linked with
	if(!AllowPaymentApply(iSourceID,iDestID,strClickedType)) {
		return FALSE;
	}

/////////////////////////////////////////////////////////////
// Make the financial apply dialog appear so the user may
// specify how much to apply, and also, if this person wants
// to write off the remaining balance to the patient if it's
// an insurance related apply.
/////////////////////////////////////////////////////////////
	if (IDCANCEL == dlg.DoModal() || dlg.m_cyApplyAmount == COleCurrency(0,0)) {
		return FALSE;
	}

/////////////////////////////////////////////////////////////
// Get ID of item that will be applied to and make the apply
/////////////////////////////////////////////////////////////

	long nBillID = -1;

	if (strClickedType == "Bill") {
		nBillID = (long)iDestID;
		if (dlg.m_nResponsibility == 0)
			ApplyPayToBill(iSourceID, nActivePatientID, dlg.m_cyApplyAmount, strClickedType, iDestID);
		else {
			//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
			ApplyPayToBill(iSourceID, nActivePatientID, dlg.m_cyApplyAmount, strClickedType, iDestID, InsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
		}
		if (dlg.m_boAdjustBalance) {
			CPaymentDlg paydlg(NULL);
			paydlg.m_iDefaultPaymentType = 1;
			paydlg.m_cyFinalAmount = AdjustBalance(iDestID,iDestID,nActivePatientID,1,dlg.m_nResponsibility,InsuredPartyID);
			if(paydlg.m_cyFinalAmount.GetStatus()==COleCurrency::invalid) {
				return FALSE;
			}
			paydlg.m_varBillID = nBillID;
			paydlg.m_ApplyOnOK = TRUE;
			// (j.jones 2008-04-29 10:29) - PLID 29744 - this didn't support inactive insurance
			if(dlg.m_nResponsibility != 0 && InsuredPartyID > 0) {
				paydlg.m_iDefaultInsuranceCo = InsuredPartyID;
			}
			paydlg.DoModal(__FUNCTION__, __LINE__);
		}
		if(dlg.m_nResponsibility != 0) {
			// See if we need to shift the remaining balance to the 
			// patient in the case of an insurance apply.
			if (dlg.m_boShiftBalance) {
				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
				ShiftInsBalance(iDestID, nActivePatientID, GetInsuranceIDFromType(nActivePatientID, dlg.m_nResponsibility), GetInsuranceIDFromType(nActivePatientID, dlg.m_nShiftToResp), "Bill",
					"after applying an existing credit");
			}

			// (j.jones 2011-03-23 17:28) - PLID 42936 - now we have to check the allowable for what we applied
			_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
				"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
				"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
				"FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
				"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID = {INT} AND BillsT.ID = {INT} "
				"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
				"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
				"InsuredPartyT.PersonID, ChargesT.ID", iSourceID, iDestID);
			while(!rsAppliedTo->eof) {
				//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
				WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
					AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
					AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
					AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

				//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
				rsAppliedTo->MoveNext();
			}
			rsAppliedTo->Close();
		}

		CheckUnbatchClaim(nBillID);
	}
	else if (strClickedType == "Charge") {

		// (j.jones 2009-10-06 14:13) - PLID 36425 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",iDestID);
		if(!rs->eof) {
			nBillID = AdoFldLong(rs, "BillID");
		}
		rs->Close();

		if (dlg.m_nResponsibility == 0)
			ApplyPayToBill(iSourceID, nActivePatientID, dlg.m_cyApplyAmount, strClickedType, iDestID);
		else {
			if(dlg.m_boIncreaseInsBalance) {
				if(!IncreaseInsBalance(iDestID,nActivePatientID,InsuredPartyID,dlg.m_cyApplyAmount)) {
					return FALSE;
				}
			}
			//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
			ApplyPayToBill(iSourceID, nActivePatientID, dlg.m_cyApplyAmount, strClickedType, iDestID, GetInsuranceIDFromType(nActivePatientID,dlg.m_nResponsibility), FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
			
			// See if we need to shift the remaining balance to the 
			// patient in the case of an insurance apply.
			if (dlg.m_boShiftBalance) {
				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
				ShiftInsBalance(iDestID, nActivePatientID, GetInsuranceIDFromType(nActivePatientID, dlg.m_nResponsibility), GetInsuranceIDFromType(nActivePatientID, dlg.m_nShiftToResp), "Charge",
					"after applying an existing credit");
			}

			// (j.jones 2011-03-23 17:28) - PLID 42936 - now we have to check the allowable for what we applied
			_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
				"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
				"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
				"FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
				"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID = {INT} AND ChargesT.ID = {INT} "
				"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
				"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
				"InsuredPartyT.PersonID, ChargesT.ID", iSourceID, iDestID);
			while(!rsAppliedTo->eof) {
				//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
				WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
					AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
					AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
					AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

				//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
				rsAppliedTo->MoveNext();
			}
			rsAppliedTo->Close();
		}
		if (dlg.m_boAdjustBalance) {
			CPaymentDlg paydlg(NULL);
			paydlg.m_iDefaultPaymentType = 1;
			paydlg.m_cyFinalAmount = AdjustBalance(iDestID,nBillID,nActivePatientID,1,dlg.m_nResponsibility,InsuredPartyID);
			if(paydlg.m_cyFinalAmount.GetStatus()==COleCurrency::invalid) {
				return FALSE;
			}
			paydlg.m_varChargeID = (long)iDestID;
			paydlg.m_ApplyOnOK = TRUE;
			// (j.jones 2008-04-29 10:29) - PLID 29744 - this didn't support inactive insurance
			if(dlg.m_nResponsibility != 0 && InsuredPartyID > 0) {
				paydlg.m_iDefaultInsuranceCo = InsuredPartyID;
			}
			paydlg.DoModal(__FUNCTION__, __LINE__);
		}

		CheckUnbatchClaim(nBillID);
	}
	else {
		ApplyPayToBill(iSourceID, nActivePatientID, dlg.m_cyApplyAmount, strClickedType, iDestID, -1/*Patient Resp*/, TRUE);
	}

	// (j.jones 2013-07-22 10:14) - PLID 57653 - If they have configured insurance companies
	// to force unbatching due to primary crossover to secondary, force unbatching now.
	// This needs to be after shifting/batching has occurred in the normal posting flow.
	if((strClickedType == "Bill" || strClickedType == "Charge")
		&& nBillID != -1 && InsuredPartyID != -1) {
		//This ConfigRT name is misleading, it actually just means that if we do unbatch a crossed over claim,
		//claim history will only include batched charges. If false, then claim history includes all charges.
		bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

		//This function assumes that the bill's current insured party ID is now the "secondary" insured party
		//we crossed over to, and the insured party who paid was primary.
		//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
		CheckUnbatchCrossoverClaim(PatientID, nBillID, InsuredPartyID, dtPayment,
			bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");
	}

	return TRUE;
}

////////////////////////////////////////////////////////
// This function, given the Insured Party ID,
// will return what type of insurance it is
//
// Return value: 0 = None, otherwise, the RespTypeID
int GetInsuranceTypeFromID(int InsuredPartyID)
{
	// (j.jones 2009-10-06 14:13) - PLID 36425 - parameterized
	_RecordsetPtr tmpRS = CreateParamRecordset("SELECT RespTypeID FROM InsuredPartyT WHERE PersonID = {INT}",InsuredPartyID);
	if(!tmpRS->eof) {
		long ID = tmpRS->Fields->Item["RespTypeID"]->Value.lVal;
		tmpRS->Close();
		return ID;
	}
	else {
		tmpRS->Close();
		return 0;
	}
}

///////////////////////////////////////////////////
// This function, given the patient and type of
// insurance, will return the Insured Party ID
//
//iType is the ID in RespTypeT
int GetInsuranceIDFromType(int PatientID, int iType)
{
	// (j.jones 2009-10-06 14:33) - PLID 36425 - parameterized
	_RecordsetPtr tmpRS = CreateParamRecordset("SELECT PersonID AS ID FROM InsuredPartyT "
		"WHERE PatientID = {INT} AND RespTypeID = {INT}", PatientID, iType);
	if(!tmpRS->eof) {

		if(iType == -1) {
			// (j.jones 2005-04-20 09:48) - This will give inaccurate results if you attempt to do this
			// with an Inactive resp and there are more than one Inactive resps. It will only return
			// the ID of the first Inactive resp, which may not be the one we are using!
			// So ASSERT if there are more than one of this resp type. That way we will know that
			// the calling function should really not be using this function.
			ASSERT(tmpRS->GetRecordCount() == 1);
		}

		long ID = tmpRS->Fields->Item["ID"]->Value.lVal;
		tmpRS->Close();
		return ID;
	}
	else {
		tmpRS->Close();
		return -1;
	}
}

/////////////////////////////////////////////////////////////
// This function will unapply a given responsibility from a bill in the total amount of cySumOfDesiredUnapplies.
//
// Return value: Sum of amount actually unapplied from bill
COleCurrency UnapplyFromBill(long nBillID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cySumOfDesiredUnapplies)
{
	// (j.jones 2009-10-06 14:13) - PLID 36425 - parameterized
	// (j.jones 2011-09-15 15:29) - PLID 44891 - you can't unapply from original/void charges, so skip them
	_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"WHERE LineItemT.Deleted = 0 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND ChargesT.BillID = {INT}", nBillID);
	while (!rs->eof && cySumOfDesiredUnapplies != COleCurrency(0,0)) {
		cySumOfDesiredUnapplies -= UnapplyFromCharge(AdoFldLong(rs, "ID"), nInsuredPartyID, bInsuranceApplies, cySumOfDesiredUnapplies);
		rs->MoveNext();
	}
	rs->Close();

	return cySumOfDesiredUnapplies;
}

// (j.jones 2007-01-04 09:35) - PLID 24030 - added revenue code support
// (j.jones 2010-06-02 16:20) - PLID 37200 - added optional array of affected PaymentIDs
COleCurrency UnapplyFromRevenueCode(long nBillID, long nRevenueCodeID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cySumOfDesiredUnapplies, CArray<UnappliedAmount, UnappliedAmount> *paryUnappliedPaymentIDs /*= NULL*/)
{
	//find all charges on the bill with the given revenue code, and unapply from each charge
	// (j.jones 2009-10-06 15:53) - PLID 36425 - parameterized
	// (j.jones 2011-09-15 15:29) - PLID 44891 - you can't unapply from original/void charges, so skip them
	_RecordsetPtr rsCharges = CreateParamRecordset("SELECT "
		"ChargesT.ID AS ChargeID "
		"FROM "
		"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
		"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
		"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = {INT}) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
		"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND BillsT.ID = {INT} AND "
		"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT}",
		nInsuredPartyID, nBillID, nRevenueCodeID);

	while(!rsCharges->eof && cySumOfDesiredUnapplies != COleCurrency(0,0)) {
		// (j.jones 2009-10-07 10:29) - PLID 35837 - fixed bug where this referenced an invalid recordset field
		long nChargeID = AdoFldLong(rsCharges, "ChargeID");
		cySumOfDesiredUnapplies -= UnapplyFromCharge(nChargeID, nInsuredPartyID, bInsuranceApplies, cySumOfDesiredUnapplies, paryUnappliedPaymentIDs);
		rsCharges->MoveNext();
	}
	rsCharges->Close();

	return cySumOfDesiredUnapplies;
}

// (j.jones 2010-06-02 16:20) - PLID 37200 - added optional array of affected PaymentIDs
// (j.jones 2011-03-21 17:10) - PLID 24273 - added a bool that tells auditing that an EOB posting caused the unapply
COleCurrency UnapplyFromCharge(long nChargeID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cyAmountToUnapply, CArray<UnappliedAmount, UnappliedAmount> *paryUnappliedPaymentIDs /*= NULL*/, BOOL bUnappliedDuringEOBPosting /*= FALSE*/)
{
	CString str;
	COleCurrency cySumOfDesiredUnapplies = cyAmountToUnapply;
	COleCurrency cy, cyUnapplied = COleCurrency(0,0);

	//default this filter to patient applies
	CSqlFragment sqlInsFilter("(PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID < 1)");

	if(bInsuranceApplies) {
		//insurance applies
		CSqlFragment sqlInsFilter2("PaymentsT.InsuredPartyID = {INT}", nInsuredPartyID);

		sqlInsFilter = sqlInsFilter2;
	}

	//TES 7/10/2008 - PLID 30673 - For auditing purposes, we also need to track the source and destination types and IDs, and
	// the patient name.
	// (j.jones 2009-10-06 15:53) - PLID 36425 - parameterized
	// (a.walling 2010-01-21 15:55) - PLID 37023
	// (j.jones 2011-09-13 15:31) - PLID 44893 - skip original & voided applies, and then with what's
	// left, check to see if the apply is closed, which is only calculated based on its input date being <= the 'As Of' date
	//TES 7/28/2014 - PLID 62557 - Chargebacks always add up to $0.00, so there's no reason to unapply them (plus, we don't generally allow them to be unapplied anyway).
	// So, filter them out
	// Applies are considered closed if their source payment is closed.
	_RecordsetPtr rsApplies = CreateParamRecordset("SELECT AppliesT.ID, AppliesT.Amount, AppliesT.SourceID, "
		"SourceItem.Type AS SourceType, AppliesT.DestID, PaymentsT.InsuredPartyID AS SourceInsuredPartyID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PersonT.ID AS PatID, "
		"Convert(bit, CASE WHEN FinancialCloseHistoryQ.HardCloseAsOfDate Is Null THEN 0 "
		"	WHEN SourceItem.InputDate <= FinancialCloseHistoryQ.HardCloseAsOfDate "
		"	OR (SourceItem.InputDate <= FinancialCloseHistoryQ.HardCloseInputDate AND SourceItem.Date <= FinancialCloseHistoryQ.HardCloseAsOfDate) "
		"	THEN 1 ELSE 0 END) AS IsClosed "
		"FROM AppliesT "
		"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
		"INNER JOIN LineItemT SourceItem ON PaymentsT.ID = SourceItem.ID "
		"INNER JOIN PersonT ON SourceItem.PatientID = PersonT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON SourceItem.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON SourceItem.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
		"CROSS JOIN (SELECT Max(CloseDate) AS HardCloseAsOfDate, "
		"	Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT) AS FinancialCloseHistoryQ "
		"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
		"WHERE AppliesT.DestID = {INT} "
		"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
		"AND PointsToPayments = 0 "
		"AND ChargebacksT.ID Is Null "
		"AND {SQL}", nChargeID, sqlInsFilter);
	
	//(e.lally 2007-04-04) PLID 25500 - Put the while loop of statements in a batch so we can execute them at the end.
	CString strSqlBatch = BeginSqlBatch();
	long nAuditTransactionID = -1;
	try {
		while (!rsApplies->eof && cySumOfDesiredUnapplies != COleCurrency(0,0)) {
			
			cy = AdoFldCurrency(rsApplies, "Amount");

			long nApplyID = AdoFldLong(rsApplies, "ID");
			long nSourceID = AdoFldLong(rsApplies, "SourceID");
			long nSourceInsuredPartyID = AdoFldLong(rsApplies, "SourceInsuredPartyID");
			long nPatientID = AdoFldLong(rsApplies, "PatID");
			// (j.jones 2011-09-13 15:31) - PLID 44893 - track the IsClosed value
			BOOL bIsClosed = AdoFldBool(rsApplies, "IsClosed", FALSE);

			// (j.jones 2015-03-20 11:52) - PLID 65400 - added a preference to always void and correct
			bool bAlwaysVoidAndCorrect = (GetRemotePropertyInt("UnapplyFromCharge_VoidAndCorrect", 0, 0, "<None>", true) == 1);

			////////////////////////////////////////////////////
			// If this apply is less or equal to the amount we
			// want desired, unapply it in full
			if (cy <= cySumOfDesiredUnapplies) {

				// (j.jones 2011-09-13 15:31) - PLID 44893 - if the apply is closed, create a financial correction,
				// otherwise we can just delete it (only do this if the corrections feature is enabled)
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				if (bIsClosed || bAlwaysVoidAndCorrect) {

					//log this if caused by an EOB posting
					if (bUnappliedDuringEOBPosting) {
						// (j.jones 2015-03-20 11:52) - PLID 65400 - this log is now context sensitive
						if (bIsClosed) {
							Log("UnapplyFromCharge: Applied Line Item ID %li is closed, will be voided and corrected.", nSourceID);
						}
						else if (bAlwaysVoidAndCorrect) {
							Log("UnapplyFromCharge: Applied Line Item ID %li is not closed, will be voided and corrected due to the preference.", nSourceID);
						}
					}

					CFinancialCorrection finCor;

					CString strUsername = GetCurrentUserName();
					long nCurrUserID = GetCurrentUserID();

					finCor.AddCorrection(ctPayment, nSourceID, strUsername, nCurrUserID);

					// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
					// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.

					// (j.jones 2012-04-23 15:27) - PLID 49847 - tell the auditing that an EOB did this
					finCor.ExecuteCorrections(bUnappliedDuringEOBPosting);

					// (j.jones 2015-02-19 15:21) - PLID 64938 - corrections now automatically retain their applies,
					// so now we just need the new apply ID from the correction
					ApplyIDInfo eInfo = FindNewestCorrectedApply(nApplyID);

					//since we just performed the correction, this should be impossible
					if (eInfo.nID == -1) {
						//this means that FindNewestCorrectedApply failed to find the apply
						//that we just corrected inside this function
						ASSERT(FALSE);
						ThrowNxException("UnapplyFromCharge failed to unapply ApplyID %li after it was voided & corrected.", nApplyID);
				}

					nApplyID = eInfo.nID;
					nSourceID = eInfo.nSourceID;

				} //end if (bIsClosed || bAlwaysVoidAndCorrect)

				// (j.jones 2011-03-21 17:33) - PLID 24273 - pass in the flag for whether this was caused by an EOB
				DeleteApply(nApplyID, FALSE, bUnappliedDuringEOBPosting);

				cySumOfDesiredUnapplies -= cy;
				cyUnapplied += cy;

				// (j.jones 2010-06-02 16:27) - PLID 37200 - if we were given the paryUnappliedPaymentIDs array,
				// fill it with the SourceIDs & amounts
				if(paryUnappliedPaymentIDs) {					
					BOOL bFound = FALSE;
					for(int i=0;i<paryUnappliedPaymentIDs->GetSize() && !bFound; i++) {
						UnappliedAmount ua = (UnappliedAmount)paryUnappliedPaymentIDs->GetAt(i);
						if(ua.nPaymentID == nSourceID) {
							bFound = TRUE;

							ua.cyAmtUnapplied += cy;
						}
					}
					if(!bFound) {
						UnappliedAmount ua;
						ua.nPaymentID = nSourceID;
						ua.cyAmtUnapplied = cy;
						paryUnappliedPaymentIDs->Add(ua);
					}
				}
			} //end if (cy <= cySumOfDesiredUnapplies)
			////////////////////////////////////////////////////
			// Otherwise, modify the apply amount
			else {

				// (j.jones 2011-09-13 15:31) - PLID 44893 - if the apply is closed, create a financial correction,
				// otherwise we can just change it's applied amount (only do this if the corrections feature is enabled)
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				if (bIsClosed || bAlwaysVoidAndCorrect) {

					//log this if caused by an EOB posting
					if(bUnappliedDuringEOBPosting) {
						if (bIsClosed) {
							Log("UnapplyFromCharge: Applied Line Item ID %li is closed, will be voided and corrected.", nSourceID);
						}
						else if (bAlwaysVoidAndCorrect) {
							Log("UnapplyFromCharge: Applied Line Item ID %li is not closed, will be voided and corrected due to the preference.", nSourceID);
						}
					}

					CFinancialCorrection finCor;

					CString strUsername = GetCurrentUserName();
					long nCurrUserID = GetCurrentUserID();

					finCor.AddCorrection(ctPayment, nSourceID, strUsername, nCurrUserID);

					// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
					// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.

					// (j.jones 2012-04-23 15:27) - PLID 49847 - tell the auditing that an EOB did this
					finCor.ExecuteCorrections(bUnappliedDuringEOBPosting);

					// (j.jones 2015-02-19 15:21) - PLID 64938 - corrections now automatically retain their applies,
					// so now we just need the new apply ID from the correction
					ApplyIDInfo eInfo = FindNewestCorrectedApply(nApplyID);

					//since we just performed the correction, this should be impossible
					if (eInfo.nID == -1) {
						//this means that FindNewestCorrectedApply failed to find the apply
						//that we just corrected inside this function
						ASSERT(FALSE);
						ThrowNxException("UnapplyFromCharge failed to unapply ApplyID %li after it was voided & corrected.", nApplyID);
					}

					nApplyID = eInfo.nID;
					nSourceID = eInfo.nSourceID;

				} //end if (bClosed || bAlwaysVoidAndCorrect)

				//change its applied amount

					//update the main apply
					AddStatementToSqlBatch(strSqlBatch,"UPDATE AppliesT SET Amount = Convert(money,'%s') WHERE ID = %li",
						_Q(FormatCurrencyForSql(cy - cySumOfDesiredUnapplies)),nApplyID);

					// (j.jones 2010-06-02 16:27) - PLID 37200 - if we were given the paryUnappliedPaymentIDs array,
					// fill it with the SourceIDs & amounts
					if(paryUnappliedPaymentIDs) {					
						BOOL bFound = FALSE;
						for(int i=0;i<paryUnappliedPaymentIDs->GetSize() && !bFound; i++) {
							UnappliedAmount ua = (UnappliedAmount)paryUnappliedPaymentIDs->GetAt(i);
							if(ua.nPaymentID == nSourceID) {
								bFound = TRUE;

								ua.cyAmtUnapplied += cySumOfDesiredUnapplies;
							}
						}
						if(!bFound) {
							UnappliedAmount ua;
							ua.nPaymentID = nSourceID;
							ua.cyAmtUnapplied = cySumOfDesiredUnapplies;
							paryUnappliedPaymentIDs->Add(ua);
						}
					}

					//now update the apply details the same way
					// (j.jones 2009-10-06 15:53) - PLID 36425 - parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT ApplyDetailsT.ID, ApplyDetailsT.Amount "
						"FROM ApplyDetailsT WHERE ApplyID = {INT} ORDER BY DetailID DESC", nApplyID);
					while(!rs->eof && cySumOfDesiredUnapplies != COleCurrency(0,0)) {

						COleCurrency cyDetail = AdoFldCurrency(rs, "Amount");
						long nApplyDetailID = AdoFldLong(rs, "ID");
			
						if (cyDetail <= cySumOfDesiredUnapplies) {
							AddStatementToSqlBatch(strSqlBatch,"DELETE FROM ApplyDetailsT WHERE ID = %li",nApplyDetailID);
							cySumOfDesiredUnapplies -= cyDetail;
							cyUnapplied += cyDetail;
						}
						else {
							AddStatementToSqlBatch(strSqlBatch,"UPDATE ApplyDetailsT SET Amount = Convert(money,'%s') WHERE ID = %li",
								_Q(FormatCurrencyForSql(cyDetail - cySumOfDesiredUnapplies)),nApplyDetailID);
							cyUnapplied += cySumOfDesiredUnapplies;
							cySumOfDesiredUnapplies = COleCurrency(0,0);
						}

						rs->MoveNext();
					}
					rs->Close();

				//TES 7/10/2008 - PLID 30673 - Audit the unapply, for both the source and destination.
				CString strAmount = FormatCurrencyForInterface(cySumOfDesiredUnapplies, TRUE, TRUE);
				strAmount.Replace("(","");
				strAmount.Replace(")","");

				CString strPatName = AdoFldString(rsApplies, "PatName");

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AuditEventItems aeiSourceItem;
				long nSourceType = AdoFldLong(rsApplies, "SourceType");

				switch(nSourceType) {
				case 1:
					// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
					// if an EOB posting caused this
					if(bUnappliedDuringEOBPosting) {
						aeiSourceItem = aeiPaymentUnappliedByERemitPosting;
					}
					else {
						aeiSourceItem = aeiPaymentUnapplied;
					}
					break;
				case 2:
					// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
					// if an EOB posting caused this
					if(bUnappliedDuringEOBPosting) {
						aeiSourceItem = aeiAdjustmentUnappliedByERemitPosting;
					}
					else {
						aeiSourceItem = aeiAdjustmentUnapplied;
					}
					break;
				case 3:
					// (j.jones 2011-03-21 17:22) - PLID 24273 - use a different audit
					// if an EOB posting caused this
					if(bUnappliedDuringEOBPosting) {
						aeiSourceItem = aeiRefundUnappliedByERemitPosting;
					}
					else {
						aeiSourceItem = aeiRefundUnapplied;
					}
					break;
				default:
					AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
					break;
				}

				AuditEvent(nPatientID, strPatName, nAuditTransactionID, aeiSourceItem, AdoFldLong(rsApplies, "SourceID"), "", strAmount + " Unapplied", aepHigh, aetDeleted);

				//TES 7/10/2008 - PLID 30673 - Now the destination, which by definition for this function is a charge
				// (j.jones 2011-03-21 17:35) - PLID 24273 - now we use a different audit if an EOB posting caused this
				AuditEvent(nPatientID, strPatName, nAuditTransactionID, bUnappliedDuringEOBPosting ? aeiItemUnappliedFromChargeByERemitPosting : aeiItemUnappliedFromCharge, AdoFldLong(rsApplies, "DestID"), "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);

				cySumOfDesiredUnapplies = COleCurrency(0,0);
			}
			
			rsApplies->MoveNext();
		}
		rsApplies->Close();

		//Commit our batch of statements
		if(!strSqlBatch.IsEmpty()){
			ExecuteSqlBatch(strSqlBatch);
		}
		//TES 7/10/2008 - PLID 30673 - And commit our auditing
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}
	}NxCatchAllSilentCallThrow( if(nAuditTransactionID != -1) {RollbackAuditTransaction(nAuditTransactionID);});

	
	return cyUnapplied;
}

void WriteHCFAToHistoricTable(int iBillID, int iPatientID)
{
	CHCFADlg hcfa(NULL);
	int dummy;

	hcfa.m_ShowWindowOnInit = FALSE;
	hcfa.m_PatientID = iPatientID;
	hcfa.m_ID = iBillID;
	hcfa.Create(IDD_HCFA, NULL);
	hcfa.Save(TRUE, 0, dummy);
	hcfa.DestroyWindow();
}

void CloseInvisibleNxControls()
{
	ASSERT(FALSE);
	//BVB - you had stuff checked out when I removed this function from NxTabView.
	//Remove this function and all calls to it.
}

/**************************************************************************
*	CalculateTax
*	The billing dialog calculated tax in too many places, and we had to change it way too much.
*	Here we can keep it all confined to one spot.
*
*	cyPreTax is the total currency to be taxed
*	dblTax is the taxrate in 1.065 format (ie. 1.065 = 6.5%)
*	bReturnTotalVal -if TRUE, return the calculated total final value of currency with tax,
*	if FALSE, just return the amount of the tax. Since we support multiple taxrates, we should
*	almost always be calling this function with FALSE
*
*	Throws CNxException
*
*   RAC - I changed the implementation of this function to multiply first by the clean tax 
*			rate (without the 4.5 instead of 1.045) and then divide by 100, plus I implemented 
*			the COleCurrency::operator *(double) more correctly, so now it calculates better)
***************************************************************************/
COleCurrency CalculateTax(COleCurrency cyPreTax, double dblTax, BOOL bReturnTotalVal)
{
	ASSERT(dblTax >= (double)1 && dblTax < (double)2);

	// (j.jones 2006-04-26 08:38) - PLID 20293 - we don't need to make a trip to the
	// server if the taxrate is 1.0 (or 0.0, though that shouldn't be possible)
	if(dblTax == 1.0) {
		if(bReturnTotalVal) {
			return cyPreTax;
		}
		else {
			return COleCurrency(0,0);
		}
	}
	else if(dblTax == 0.0) {
		return COleCurrency(0,0);
	}

	// We're assuming we can safely multiply the tax by a million without going out of LONG bounds
	if ((dblTax >= (double)1) && (dblTax < (double)2)) {
		/*
		OLD WAY
		JMJ - I changed this to use a query on 12/19/2003.

		COleCurrency cyMultByTax = cyPreTax * ((dblTax-1.0) * 100.0); // We want to FORCE the multiplication to happen first
		COleCurrency cyTaxAmount = cyMultByTax / 100.0; // Convert back to the appropriate decimal
		

		// Then return the correct result based on what was asked for
		if (bReturnTotalVal) {
			//calcuate the total value of cyPreTax plus the additional tax
			return cyPreTax + cyTaxAmount;
		}
		else {
			//only return the additional tax
			return cyTaxAmount;
		}
		*/

		//JMJ 12/19/2003 - Multiplying a COleCurrency by a double had plagued us for YEARS.
		//This finally does it as right as can be. Since it doesn't actually access any records,
		//the recordset is pretty fast, and shouldn't be a problem.

		COleCurrency cyTaxAmount = COleCurrency(0,0);
		COleCurrency cyTotalAmount = COleCurrency(0,0);

		// (j.jones 2009-10-06 15:53) - PLID 36425 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Convert(money,Convert(money,{STRING}) * {DOUBLE}) AS TotalAmt, "
			"Convert(money,Convert(money,{STRING}) * {DOUBLE}) AS TaxAmt",
			FormatCurrencyForSql(cyPreTax),dblTax,
			FormatCurrencyForSql(cyPreTax),dblTax-1.0);
		if(!rs->eof) {
			cyTotalAmount = AdoFldCurrency(rs, "TotalAmt");
			cyTaxAmount = AdoFldCurrency(rs, "TaxAmt");		

			
			// (a.walling 2012-03-12 17:33) - PLID 48839 - Compare with new implementation, ASSERT if there is a difference
#ifdef _DEBUG
			{
				COleCurrency cyTotalTest = cyPreTax * dblTax;
				__int64 diff = (cyTotalAmount.m_cur.int64 - cyTotalTest.m_cur.int64);
				ASSERT(0 == diff);
			}
			{
				COleCurrency cyTaxTest = cyPreTax * (dblTax - 1.0);
				__int64 diff = (cyTaxAmount.m_cur.int64 - cyTaxTest.m_cur.int64);
				ASSERT(0 == diff);
			}
#endif

		}
		rs->Close();

		if(bReturnTotalVal)
			return cyTotalAmount;
		else
			return cyTaxAmount;

	} else {
		// Invalid tax rate given.  We need to throw an exception
		AfxThrowNxException("Invalid tax rate given: %.20f", dblTax);
		COleCurrency cyInvalid;
		cyInvalid.SetStatus(COleCurrency::invalid);
		return cyInvalid;
	}
}

//JMJ 3/11/2004 - we now support a double for a quantity, which has the same problems
//as the taxrate, so we use a similar method for calculating proper amounts
COleCurrency CalculateAmtQuantity(COleCurrency cyAmount, double dblQuantity)
{
	//JMJ - Multiplying a COleCurrency by a double had plagued us for YEARS.
	//This finally does it as right as can be. Since it doesn't actually access any records,
	//the recordset is pretty fast, and shouldn't be a problem.

	// (j.jones 2006-04-26 08:38) - PLID 20293 - we don't need to make a trip to the
	// server if the dblQuantity is 1.0 (or 0.0, though that shouldn't be possible)
	if(dblQuantity == 1.0) {
		return cyAmount;
	}
	else if(dblQuantity == 0.0) {
		return COleCurrency(0,0);
	}

	COleCurrency cyTotalAmount = COleCurrency(0,0);

	// (j.jones 2009-10-06 16:13) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT Convert(money,Convert(money,{STRING}) * {DOUBLE}) AS TotalAmt",FormatCurrencyForSql(cyAmount),dblQuantity);
	if(!rs->eof) {
		cyTotalAmount = AdoFldCurrency(rs, "TotalAmt");
	}
	rs->Close();

	// (a.walling 2012-03-12 17:33) - PLID 48839 - Compare with new implementation, ASSERT if there is a difference
#ifdef _DEBUG
	{
		COleCurrency cyTotalTest = cyAmount * dblQuantity;
		__int64 diff = (cyTotalAmount.m_cur.int64 - cyTotalTest.m_cur.int64);
		ASSERT(0 == diff);
	}
#endif

	return cyTotalAmount;
}

/// <summary>
/// Calculates the total balance of available prepayments linked to a given quote.
/// Enabling bIncludePrepaysWithNoQuotes will also include prepayments not linked to any quote.
/// </summary>
COleCurrency CalculatePrePayments(long nPatientID, long nQuoteID, bool bIncludePrepaysWithNoQuotes)
{
	try {

		CSqlFragment sqlFilter = CSqlFragment("AND PaymentsT.QuoteID = {INT}", nQuoteID);

		//If off, we only count prepayments linked with this quote.
		//If on, then we also include prepayments that are not linked to any quote.
		if (bIncludePrepaysWithNoQuotes)
		{
			sqlFilter = CSqlFragment("AND (PaymentsT.QuoteID = {INT} OR PaymentsT.QuoteID Is Null)", nQuoteID);
		}

		return CalculatePrePayments(nPatientID, sqlFilter);

	}NxCatchAll(__FUNCTION__);

	return COleCurrency(0, 0);
}

/// <summary>
/// Calculates the total balance of available prepayments on the patient's account.
/// The optional filter can be used as a where clause on LineItemT or PaymentsT.
/// </summary>
COleCurrency CalculatePrePayments(long nPatientID, CSqlFragment sqlFilter) {

	//return the value of all UNAPPLIED PrePayments

	try {
				
		_RecordsetPtr rsPay = CreateParamRecordset(
			"SELECT Sum(LineItemT.Amount - IsNull(OutgoingAppliesT.Total, 0) + IsNull(IncomingAppliesT.Total,0)) As PrePayBalance "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN(SELECT Sum(Amount) AS Total, DestID FROM AppliesT GROUP BY DestID) AS IncomingAppliesT ON PaymentsT.ID = IncomingAppliesT.DestID "
			"LEFT JOIN(SELECT Sum(Amount) AS Total, SourceID FROM AppliesT GROUP BY SourceID) AS OutgoingAppliesT ON PaymentsT.ID = OutgoingAppliesT.SourceID "
			"WHERE PaymentsT.PrePayment = 1 AND LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 "
			"{SQL}",
			nPatientID, sqlFilter);
		if(!rsPay->eof) {
			COleCurrency cyPrePayBalance = VarCurrency(rsPay->Fields->Item["PrePayBalance"]->Value, COleCurrency(0, 0));
			return cyPrePayBalance;
		}
		rsPay->Close();

	}NxCatchAll(__FUNCTION__);
	
	return COleCurrency(0, 0);
}

/// <summary>
/// Calculates the remaining balance of a package with and without tax,
/// taking into account any prepayments linked to the package, or payments
/// applied to billed uses of the package.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax, COleCurrency &cyRemBalanceWithTax)
{
	//call the main function stating that we do want the 'with tax' version
	CalculateRemainingPackageBalance(nQuoteID, nPatientID, varOriginalCurrentAmount, cyRemBalanceNoTax, true, cyRemBalanceWithTax);
	return;
}

/// <summary>
/// Calculates the remaining balance of a package only without tax,
/// taking into account any prepayments linked to the package, or payments
/// applied to billed uses of the package.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax)
{
	//this variable will not be filled
	COleCurrency cyRemBalanceWithTax = COleCurrency(0, 0);

	//call the main function stating that we do not want the 'with tax' version
	CalculateRemainingPackageBalance(nQuoteID, nPatientID, varOriginalCurrentAmount, cyRemBalanceNoTax, false, cyRemBalanceWithTax);
	return;
}

/// <summary>
/// Calculates the remaining balance of a package, taking into account any prepayments
/// linked to the package, or payments applied to billed uses of the package.
/// Will not calculate the 'With Tax' variable if bCalculateValueWithTax is set to false.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax, bool bCalculateValueWithTax, COleCurrency &cyRemBalanceWithTax)
{
	//this function will now calculate both the non-tax and the with-tax values,
	//both to prevent code duplication, and to conserve recordsets

	cyRemBalanceNoTax = COleCurrency(0,0);
	cyRemBalanceWithTax = g_ccyInvalid;

	//first calculate the initial tax total, but only if the 'with tax' value is requested
	if(bCalculateValueWithTax) {
		// (j.jones 2007-04-23 13:03) - PLID 25735 - the Original package value
		// may not equal the "Total" package value
		cyRemBalanceWithTax = CalculateOriginalPackageValueWithTax(nQuoteID);
	}
	
	//now calculate the initial no-tax total, which means getting the original current amount from the package,
	//hopefully our caller provider it, if not we have to load it from data
	if (varOriginalCurrentAmount.vt == VT_CY)
	{
		//good work, non-lazy caller!
		cyRemBalanceNoTax = VarCurrency(varOriginalCurrentAmount);
	}
	else
	{
		//boo

		//Currently no code hits this area because all the callers are smart enough
		//to know to provide this field. If you are SURE you can't have your caller
		//provide this field, comment out the assert in shame.
		ASSERT(FALSE);

		_RecordsetPtr rs = CreateParamRecordset("SELECT OriginalCurrentAmount FROM PackagesT WHERE QuoteID = {INT}", nQuoteID);
		if(!rs->eof) {
			cyRemBalanceNoTax = AdoFldCurrency(rs, "OriginalCurrentAmount", COleCurrency(0,0));
		}
		rs->Close();
	}

	//Calculate cyRemBalance based on all the applies agains bills generated
	//from this package, plus any available prepayments.

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted this recordset to be parameterized

	// (j.jones 2007-04-24 09:09) - PLID 25753 - The problem here is that if we're trying to get
	// the non-tax total, we can't calculate all the previous payments because they would
	// include taxed amounts as well (if a taxed bill was fully paid, at least).
	// We need to get the non-tax bill total, the total amount paid, and then if the total
	// amount paid is greater than the non-tax bill total, only subtract the non-tax bill
	// total from cyRemBalance.

	// (j.jones 2008-02-13 17:35) - PLID 28794 - we need to look at PackageChargeRefID to
	// ensure we are only looking at charges for the same package, and not charges added later

	// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

	// (j.gruber 2009-03-18 09:14) - PLID 33360 - updated for discount structure change

	CSqlFragment sqlTaxTotal("Sum(Round(dbo.GetChargeTotal(ChargesT.ID),2))");
	if (!bCalculateValueWithTax) {
		//save ourselves some calculating
		sqlTaxTotal = CSqlFragment("NULL");
	}

	_RecordsetPtr rsBills = CreateParamRecordset("SELECT BillsT.ID, "
		"{SQL} AS BillTaxTotal, "
		"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal, "
		"Sum(Coalesce(AppliesQ.AppliedAmount,0)) AS TotalApplies "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		// (j.gruber 2009-03-16 13:42) - PLID 33360 - new discount structure
		"LEFT JOIN (SELECT ChargeID, Sum(PercentOff) AS TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentOffQ ON ChargesT.ID = TotalPercentOffQ.ChargeID "
		"LEFT JOIN (SELECT ChargeID, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
		// (j.jones 2007-04-24 09:07) - PLID 25760 - need to account for applied
		// payments AND adjustments, not just applied payments alone
		"LEFT JOIN ("
		"	SELECT Sum(AppliesT.Amount) AS AppliedAmount, DestID "
		"	FROM AppliesT "
		"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"	WHERE Deleted = 0 GROUP BY DestID "
		") AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
		"INNER JOIN BilledQuotesT ON BillsT.ID = BilledQuotesT.BillID "
		"INNER JOIN ("
		"	SELECT ChargesT.ID "
		"	FROM ChargesT "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 "
		") AS ChargeRefQ ON ChargesT.PackageChargeRefID = ChargeRefQ.ID "
		"WHERE BilledQuotesT.QuoteID = {INT} AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "		
		"GROUP BY BillsT.ID", 
		sqlTaxTotal, nQuoteID, nQuoteID);
	while(!rsBills->eof) {

		COleCurrency cyBillNonTaxTotal = AdoFldCurrency(rsBills, "BillNonTaxTotal",COleCurrency(0,0));
		COleCurrency cyTotalApplies = AdoFldCurrency(rsBills, "TotalApplies",COleCurrency(0,0));

		//if the apply total is greater than the calculated total, only subtract the bill total
		//(this should be impossible when including tax is true, but quite possible for the non-tax balance)
		if (bCalculateValueWithTax) {
			COleCurrency cyBillTaxTotal = AdoFldCurrency(rsBills, "BillTaxTotal", COleCurrency(0, 0));
			if (cyTotalApplies > cyBillTaxTotal)
				cyRemBalanceWithTax -= cyBillTaxTotal;
			else
				cyRemBalanceWithTax -= cyTotalApplies;
		}

		if(cyTotalApplies > cyBillNonTaxTotal)
			cyRemBalanceNoTax -= cyBillNonTaxTotal;
		else
			cyRemBalanceNoTax -= cyTotalApplies;

		rsBills->MoveNext();
	}
	rsBills->Close();

	//now subtract any prepayments from the remaining balance		
	COleCurrency cyPrePayments = CalculatePrePayments(nPatientID, nQuoteID, false);

	cyRemBalanceNoTax -= cyPrePayments;

	//there's no need to make it less than zero
	if (cyRemBalanceNoTax < COleCurrency(0, 0)) {
		cyRemBalanceNoTax = COleCurrency(0, 0);
	}
	
	if (bCalculateValueWithTax) {
		cyRemBalanceWithTax -= cyPrePayments;

		if (cyRemBalanceWithTax < COleCurrency(0, 0)) {
			cyRemBalanceWithTax = COleCurrency(0, 0);
		}
	}	
}

void CopyQuote(long QuoteID) {

	COleDateTime dt = COleDateTime::GetCurrentTime();

	// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
	// (j.armen 2013-06-28 10:37) - PLID 57367 - Idenitate BillsT
	// (a.walling 2014-02-28 17:34) - PLID 61124 - BillDiagCodeT - Ignore diag codes... this is a quote
	_RecordsetPtr prs = CreateParamRecordset(
		"SET NOCOUNT ON\r\n"
		"INSERT INTO BillsT (PatientID, Date, EntryType, ExtraDesc, Note, Location, InputDate, InputName, Description, PatCoord, UseExp, ExpDays) "
		"SELECT PatientID, {STRING}, EntryType, ExtraDesc, Note, Location, GetDate(), {INT}, Description, PatCoord, UseExp, ExpDays "
		"FROM BillsT WHERE ID = {INT}\r\n"
		"\r\n"
		"SELECT CAST(SCOPE_IDENTITY() AS INT) AS BillID\r\n"
		"SET NOCOUNT OFF\r\n",
		FormatDateTimeForSql(dt, dtoDate), GetCurrentUserID(), QuoteID);

	long nNewQuoteID = AdoFldLong(prs, "BillID");

	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"WHERE BillID = {INT} AND Deleted = 0",QuoteID);
	while(!rs->eof) {		

		long nChargeID = AdoFldLong(rs, "ID");

		// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID)\r\n"
			"SELECT PatientID, Type, Amount, Description, {STRING}, GetDate(), {STRING}, LocationID FROM LineItemT WHERE ID = {INT}\r\n"
			"SELECT CAST(SCOPE_IDENTITY() AS INT) AS LineItemID\r\n"
			"SET NOCOUNT OFF\r\n",
			FormatDateTimeForSql(dt, dtoDate),GetCurrentUserName(),nChargeID);

		long nNewChargeID = AdoFldLong(prs, "LineItemID");

		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (a.walling 2006-06-16 10:40) - PLID 21054 Add the quantities remaining for multi-use quotes (use the original qty, since its a new quote, none would be billed. Previously it would just be 1)
		//		-also copy over the Patient Coordinator field (PatCoordID) for the charges (it used to only be copied for the bill)
		// (j.jones 2007-04-23 12:48) - PLID 25735 - supported OriginalPackageQtyRemaining
		// (j.gruber 2009-03-18 09:15) - PLID 33360 - change discount structure
		// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
		// (a.walling 2014-03-10 14:21) - PLID 61124 - No more WhichCodes. Also, Quotes don't have diag codes in the first place.
		// (b.spivey, February 20, 2015) - PLID 63666 - Include CPTMultipliers 
		ExecuteParamSql("INSERT INTO ChargesT (ID, BillID, ServiceID, ItemCode, ItemSubCode, Category, SubCategory, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, "
				"TaxRate, Quantity, DoctorsProviders, ServiceDateFrom, ServiceDateTo, ServiceLocationID, "
				"ServiceType, OthrBillFee, LineID, TaxRate2, PatCoordID, PackageQtyRemaining, OriginalPackageQtyRemaining, CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4) "
				"SELECT {INT}, {INT}, ServiceID, ItemCode, ItemSubCode, Category, "
				"SubCategory, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, TaxRate, Quantity, DoctorsProviders, ServiceDateFrom, "
				"ServiceDateTo, ServiceLocationID, ServiceType, OthrBillFee, LineID, TaxRate2, PatCoordID, Quantity, Quantity, "
				"CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4 FROM ChargesT WHERE ID = {INT} "
				"",nNewChargeID,nNewQuoteID,nChargeID);

		// (j.gruber 2009-03-18 09:16) - PLID 33360 - copy the charge discounts
		//the old query never updated couponID or discount category information, so I'm going to keep that behavior
		// (j.jones 2009-08-04 15:35) - PLID 35102 - make sure we exclude deleted discounts
		// (j.gruber 2014-08-05 11:30) - PLID 42658 - copying quote does not copy over discount categories from previous quote
		ExecuteParamSql("INSERT INTO ChargeDiscountsT (ChargeID, PercentOff, Discount, DiscountCategoryID, CustomDiscountDesc) "
			" SELECT {INT}, PercentOff, Discount,  DiscountCategoryID, CustomDiscountDesc FROM ChargeDiscountsT WHERE ChargeID = {INT} AND Deleted = 0", nNewChargeID, nChargeID);

		// (j.jones 2007-01-09 15:55) - PLID 24169 - copy the charge resps too!

		//Charge Resps
		// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
		_RecordsetPtr rsChargeResps = CreateParamRecordset("SELECT ID FROM ChargeRespT WHERE ChargeID = {INT}", nChargeID);
		while(!rsChargeResps->eof) {

			long nChargeRespID = AdoFldLong(rsChargeResps, "ID");
		
			// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
			// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
			// (d.lange 2013-08-20 11:38) - PLID 58128 - Removed the extra left bracket
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"INSERT INTO ChargeRespT (ChargeID, InsuredPartyID, Amount)\r\n"
				"	SELECT {INT}, InsuredPartyID, Amount FROM ChargeRespT WHERE ID = {INT}\r\n"
				"SELECT CAST(SCOPE_IDENTITY() AS INT) AS ChargeRespID\r\n"
				"SET NOCOUNT OFF", 
				nNewChargeID, nChargeRespID);

			long nNewChargeRespID = AdoFldLong(prs, "ChargeRespID");

			//Charge Resp Details
			// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
			_RecordsetPtr rsChargeRespDetails = CreateParamRecordset("SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = {INT}", nChargeRespID);
			while(!rsChargeRespDetails->eof) {

				long nChargeRespDetailID = AdoFldLong(rsChargeRespDetails, "ID");
			
				// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				ExecuteParamSql(
					"INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) "
					"SELECT {INT}, Amount, Date FROM ChargeRespDetailT WHERE ID = {INT}", 
					nNewChargeRespID, nChargeRespDetailID);

				rsChargeRespDetails->MoveNext();
			}
			rsChargeRespDetails->Close();

			rsChargeResps->MoveNext();
		}
		rsChargeResps->Close();

		rs->MoveNext();
	}
	rs->Close();

	// (a.walling 2006-06-16 10:35) - PLID 21054 Multi-use packages are now copied without becoming Repeatable packages (Added type to this insert statement)
	// (j.jones 2007-04-23 12:48) - PLID 25735 - supported OriginalCurrentCount, OriginalCurrentAmount
	// (j.jones 2009-10-07 09:01) - PLID 36425 - parameterized
	ExecuteParamSql("INSERT INTO PackagesT (QuoteID,TotalCount,TotalAmount,CurrentCount,CurrentAmount,Type, OriginalCurrentCount, OriginalCurrentAmount) "
		"SELECT {INT},TotalCount,TotalAmount,CurrentCount,CurrentAmount,Type, OriginalCurrentCount, OriginalCurrentAmount FROM PackagesT WHERE QuoteID = {INT}",nNewQuoteID,QuoteID);

	// (c.haag 2010-09-09 09:26) - PLID 38408 - Make sure that if the source quote is in ProcInfoQuotesT, that we add the new quote
	// to the same PIC as well.
	ExecuteParamSql("INSERT INTO ProcInfoQuotesT (ProcInfoID, QuoteID) SELECT ProcInfoID, {INT} FROM ProcInfoQuotesT WHERE QuoteID = {INT} "
		,nNewQuoteID, QuoteID);
}

COleCurrency CalculateRemainingBatchPaymentBalance(long BatchPayID)
{
	try {
		// (j.jones 2009-06-09 15:46) - PLID 34549 - ignore adjustments
		// (j.jones 2012-04-25 10:53) - PLID 48032 - Supported line item corrections that were takebacks,
		// returning the value of the original payment to the batch payment. Also fixed to ignore
		// payments that were voided (unless part of another batch payment's takeback).
		_RecordsetPtr rs = CreateParamRecordset("SELECT BatchPaymentsT.ID, BatchPaymentsT.Amount, "
			"Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
			"BatchPaymentsT.Amount "
			" - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
			" + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
			" - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
			" AS RemainingAmount "
			"FROM BatchPaymentsT "
			""
			//find child payments that are not voided, but include them if they are part of a takeback
			"LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
			"	FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
			"	AND PaymentsT.BatchPaymentID Is Not Null "
			"	GROUP BY PaymentsT.BatchPaymentID "
			") AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
			""
			//find payments that were part of takebacks, crediting this batch payment
			"LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
			"	FROM LineItemT "
			"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
			"	GROUP BY LineItemCorrectionsT.BatchPaymentID "
			") AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
			""
			//find the batch payment's adjustments or refunds
			"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID "
			"	FROM BatchPaymentsT "
			"	WHERE Type <> 1 AND Deleted = 0 "
			"	GROUP BY AppliedBatchPayID, Deleted "
			") AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
			""
			"GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Amount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied "
			"HAVING BatchPaymentsT.ID = {INT}", BatchPayID);

		COleCurrency cy = COleCurrency(0,0);

		if(!rs->eof) {
			cy = AdoFldCurrency(rs, "RemainingAmount",COleCurrency(0,0));		
		}

		return cy;

	}NxCatchAll("Error calculating remaining batch payment balance.");

	return COleCurrency(0,0);
}

/******************************************************************************
//TES 6/12/03: The following three functions are now in InternationalUtils
/******************************************************************************
//checks that the text currency is valid, per language
/*BOOL IsValidCurrencyText(CString strAmount) {

	strAmount.TrimLeft(GetCurrencySymbol());
	for(int i = 0; i<strAmount.GetLength(); i++) {

		if(((strAmount.GetAt(i)<'0' || strAmount.GetAt(i)>'9') && strAmount.GetAt(i)!= '.' && 
			strAmount.GetAt(i)!=',' && strAmount.GetAt(i)!='-' && strAmount.GetAt(i)!='\'') ||
			(strAmount.GetAt(i)=='\'' && GetUserDefaultLCID() != MAKELCID(MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_SWISS),SORT_DEFAULT))) {			
			return FALSE;
		}
	}

	return TRUE;
}

CString GetCurrencySymbol() {

	if(GetUserDefaultLCID() == MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT))
		return CString("$");
	else if(GetUserDefaultLCID() == MAKELCID(MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_SWISS),SORT_DEFAULT))
		return CString("SFr. ");
	else
		return CString(char(-128)); //euro
}

BOOL ParseCurrency(COleCurrency &cy, CString strValue) {

	//we're removing commas and currency symbols
	//strValue = FormatCurrency(_variant_t(_bstr_t(strValue)),FALSE,TRUE);
	strValue.Replace(CString(char(-128)), "");
	return cy.ParseCurrency(strValue);

	return cy.ParseCurrency(strValue,0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));
}*/

int GetInsuranceCoTaxType(long InsuredPartyID) {

	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT TaxType FROM InsuranceCoT "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
		"WHERE InsuredPartyT.PersonID = {INT}",InsuredPartyID);

	long TaxType = 2;

	if(!rs->eof) {
		TaxType = AdoFldLong(rs, "TaxType",2);
	}
	rs->Close();

	return TaxType;
}

long GetInsuranceCoID(long InsuredPartyID) {
	
	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}",InsuredPartyID);

	long InsuranceCoID = -1;

	if(!rs->eof)
		InsuranceCoID = AdoFldLong(rs, "InsuranceCoID",-1);
	rs->Close();

	return InsuranceCoID;
}

long GetBillLocation(long BillID) {

	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.LocationID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Deleted = 0 AND BillsT.ID = {INT}",BillID);

	long Location = -1;

	if(!rs->eof) {
		Location = AdoFldLong(rs, "LocationID",-1);
	}
	rs->Close();

	return Location;
}

// (j.jones 2009-10-06 17:30) - PLID 36425 - this function is not currently used
/*
long GetBillPOS(long BillID) {

	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT Location FROM BillsT WHERE ID = {INT}",BillID);

	long POS = -1;

	if(!rs->eof) {
		POS = AdoFldLong(rs, "Location",-1);
	}
	rs->Close();

	return POS;
}
*/

BOOL IsZeroDollarPayment(long nPaymentID) {
	// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM LineItemT WHERE Amount = Convert(money,'0') AND ID = {INT}",nPaymentID);
	return (!rs->eof);
}

//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, LocationID, ChargeID, and the dollar amount to apply
// (j.jones 2009-10-23 10:22) - PLID 18558 - this requires the Place Of Service ID as well now
void WarnAllowedAmount(long ServiceID, long InsCoID, long ProviderID, long LocationID, long nPlaceOfServiceID, long nInsuredPartyID, long nChargeID, COleCurrency cyApplyAmt)
{
	
	try {

		//get the copay for this insured party
		CString strCopay = "Convert(money,'0')";

		// (j.jones 2009-10-06 16:15) - PLID 36425 - parameterized
		// (j.gruber 2010-08-04 11:56) - PLID 39948 - change copay structure
		// (j.jones 2015-11-23 14:53) - PLID 67622 - get the charge's AllowableInsuredPartyID
		_RecordsetPtr rs = CreateParamRecordset("SELECT AllowableInsuredPartyID FROM ChargesT WHERE ID = {INT}; \r\n"
			"\r\n"
			"SELECT Copay, CopayPercent FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM "
			"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
			" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
			" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID ) InsuredPartyT WHERE PersonID = {INT}",
			nChargeID,
			nInsuredPartyID);

		// (j.jones 2015-11-23 14:53) - PLID 67622 - if the provided insured party ID is not the same as
		// ChargesT.AllowableInsuredPartyID, we do not need to warn about the allowable
		if (!rs->eof) {
			long nAllowableInsuredPartyID = VarLong(rs->Fields->Item["AllowableInsuredPartyID"]->Value, -1);
			if (nAllowableInsuredPartyID != nInsuredPartyID) {
				//the provided insured party ID is not the same insured party as the allowable,
				//so we do not need to warn the user, as the warning is meaningless
				return;
			}
		}

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {

			
			_variant_t varCopay, varPercent;
			varCopay = rs->Fields->Item["Copay"]->Value;
			varPercent = rs->Fields->Item["CopayPercent"]->Value;

			if(varCopay.vt == VT_CY) {
				//flat copay
				strCopay.Format("Convert(money,'%s')",FormatCurrencyForSql(AdoFldCurrency(rs, "Copay",COleCurrency(0,0))));
			}
			else if (varPercent.vt == VT_I4) {
				//percentage copay
				long nCopayPercent = AdoFldLong(rs, "CopayPercent",0);
				strCopay.Format("Round(convert(money, dbo.GetChargeTotal(%li) * (Convert(float,%li)/Convert(float,100))),2)",nChargeID,nCopayPercent);
			}
			else {
				//use $0
				strCopay.Format("Convert(money,'0')");
			}

		}
		rs->Close();

		// (j.jones 2006-10-17 16:54) - PLID 22173 - factored in multipliers with allowables
		// (j.jones 2007-05-03 12:40) - PLID 25840 - supported the IncludePatRespAllowable ability,
		// which checks the patient responsibility for a charge
		// (j.jones 2009-10-06 17:44) - PLID 36425 - parameterized all but the Copay calculation
		// (j.gruber 2009-10-20 11:01) - PLID 36005 - fix ambigouous column name
		// (j.jones 2009-10-23 10:22) - PLID 18558 - changed to support places of service
		// (j.jones 2010-12-16 15:25) - PLID 41869 - the IncludePatRespAllowable setting now actually means
		// all responsibilities except the one being paid, so if this is Primary, we include patient & Secondary resps.
		// (j.jones 2011-03-22 15:48) - PLID 42944 - split the "other resps" totals into insurance and patient,
		// and returned them in the query results
		// (j.jones 2013-04-15 11:05) - PLID 12136 - removed the CPTCodeT join, we now show the charge's ItemCode, which
		// might be blank for products
		// (b.cardillo 2015-11-30 15:18) - PLID 67656 - Account for charge date relative to fee schedule effective date
		rs = CreateParamRecordset(FormatString("SELECT MultiFeeItemsT.Allowable, %s AS Copay, "
			"Round(Convert(money, MultiFeeItemsT.Allowable * ChargesT.Quantity * "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"), 2) AS AllowableQty, "
			"ChargesT.Quantity, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, "
			"ServiceT.Name, ChargesT.ItemCode, IncludeCoPayAllowable, IncludePatRespAllowable, "
			"Coalesce(OtherInsRespsQ.TotalAmount, 0) AS OtherInsRespTotal, Coalesce(PatientRespQ.TotalAmount, 0) AS PatientRespTotal "
			"FROM MultiFeeGroupsT "
			"LEFT JOIN MultiFeeLocationsT ON MultiFeeGroupsT.ID = MultiFeeLocationsT.FeeGroupID "
			"LEFT JOIN MultiFeeInsuranceT ON MultiFeeGroupsT.ID = MultiFeeInsuranceT.FeeGroupID "
			"LEFT JOIN MultiFeeProvidersT ON MultiFeeGroupsT.ID = MultiFeeProvidersT.FeeGroupID "
			"LEFT JOIN MultiFeeItemsT ON MultiFeeGroupsT.ID = MultiFeeItemsT.FeeGroupID "
			"LEFT JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
			"LEFT JOIN ("
			"	SELECT ChargesT.ID "
			"		, ChargesT.ServiceID "
			"		, ChargesT.Quantity "
			"		, ChargesT.ItemCode "
			"		, ChargesT.CPTMultiplier1 "
			"		, ChargesT.CPTMultiplier2 "
			"		, ChargesT.CPTMultiplier3 "
			"		, ChargesT.CPTMultiplier4 "
			"		, LineItemT.Date "
			"	FROM ChargesT "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	WHERE ChargesT.ID = {INT}"
			"	) AS ChargesT ON ServiceT.ID = ChargesT.ServiceID "
			//OtherInsRespsQ summarizes all insurance resps besides the one being paid
			"LEFT JOIN (SELECT ChargeID, Sum(Amount) AS TotalAmount "
			"	FROM ChargeRespT "
			"	WHERE InsuredPartyID Is Not Null AND InsuredPartyID <> {INT} "	
			"	GROUP BY ChargeRespT.ChargeID "
			"	) AS OtherInsRespsQ ON ChargesT.ID = OtherInsRespsQ.ChargeID "
			//PatientRespQ gets the patient resp for this charge
			"LEFT JOIN (SELECT ChargeID, Sum(Amount) AS TotalAmount "
			"	FROM ChargeRespT "
			"	WHERE InsuredPartyID Is Null "	
			"	GROUP BY ChargeRespT.ChargeID "
			"	) AS PatientRespQ ON ChargesT.ID = PatientRespQ.ChargeID "
			"WHERE WarnAllowable = 1 AND (InsuranceCoID = {INT} OR InsuranceCoID Is Null) "
			"AND (MultiFeeProvidersT.ProviderID = {INT} OR MultiFeeProvidersT.ProviderID Is NULL) "
			"AND (MultiFeeLocationsT.LocationID Is NULL "
			"	OR (UsePOS = 0 AND MultiFeeLocationsT.LocationID = {INT}) "
			"	OR (UsePOS = 1 AND MultiFeeLocationsT.LocationID = {INT}) "
			") "
			"AND MultiFeeItemsT.ServiceID = {INT} "
			"AND MultiFeeGroupsT.Inactive = 0 "
			"AND (MultiFeeGroupsT.EffectiveFromDate IS NULL "
			"	OR ( "
			"		ChargesT.Date >= MultiFeeGroupsT.EffectiveFromDate "
			"		AND (MultiFeeGroupsT.EffectiveToDate IS NULL "
			"			OR ChargesT.Date <= MultiFeeGroupsT.EffectiveToDate)) "
			") "
			"AND MultiFeeItemsT.Allowable Is Not Null "
			"AND ChargesT.Quantity Is Not Null "
			"AND ("
			//compare the allowable to the amount paid
			"(IncludeCoPayAllowable = 0 AND IncludePatRespAllowable = 0 AND "
			"	Round(Convert(money, MultiFeeItemsT.Allowable * ChargesT.Quantity * "
			"	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) > Convert(money,{STRING})) "
			"OR "
			//compare the allowable to the amount paid + copay
			"(IncludeCoPayAllowable <> 0 AND IncludePatRespAllowable = 0 AND "
			"	Round(Convert(money, MultiFeeItemsT.Allowable * ChargesT.Quantity * "
			"	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) > (Convert(money,{STRING}) + %s)) "
			"OR "
			//compare the allowable to the amount paid + pat. resp.
			"(IncludeCoPayAllowable = 0 AND IncludePatRespAllowable <> 0 AND "
			"	Round(Convert(money, MultiFeeItemsT.Allowable * ChargesT.Quantity * "
			"	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) > Convert(money,{STRING}) + (Coalesce(OtherInsRespsQ.TotalAmount,0) + Coalesce(PatientRespQ.TotalAmount,0))) "
			"OR "
			//compare the allowable to the greater of the amount paid or the pat. resp.
			"(IncludeCoPayAllowable <> 0 AND IncludePatRespAllowable <> 0 AND "
			"	Round(Convert(money, MultiFeeItemsT.Allowable * ChargesT.Quantity * "
			"	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) > (Convert(money,{STRING}) + (CASE WHEN (Coalesce(OtherInsRespsQ.TotalAmount,0) + Coalesce(PatientRespQ.TotalAmount,0)) > %s THEN (Coalesce(OtherInsRespsQ.TotalAmount,0) + Coalesce(PatientRespQ.TotalAmount,0)) ELSE %s END))) "
			") "
			"ORDER BY MultiFeeInsuranceT.FeeGroupID DESC, MultiFeeProvidersT.FeeGroupID DESC, MultiFeeLocationsT.FeeGroupID DESC ",
			strCopay, strCopay, strCopay, strCopay),
			nChargeID, nInsuredPartyID, InsCoID, ProviderID, LocationID, nPlaceOfServiceID, ServiceID, 
			FormatCurrencyForSql(cyApplyAmt), FormatCurrencyForSql(cyApplyAmt),
			FormatCurrencyForSql(cyApplyAmt), FormatCurrencyForSql(cyApplyAmt));
		
		if(!rs->eof) {
			//will only have a record in it if there is an allowable associated with this
			//Doctor/InsCo/CPTCode and our apply amount is less than that allowed amount

			COleCurrency cyAllowed = AdoFldCurrency(rs, "Allowable", COleCurrency(0,0));
			COleCurrency cyAllowedQty = AdoFldCurrency(rs, "AllowableQty", COleCurrency(0,0));
			double dblQuantity = AdoFldDouble(rs, "Quantity", 0.0);
			double dblMultiplier1 = AdoFldDouble(rs, "CPTMultiplier1", 1.0);
			double dblMultiplier2 = AdoFldDouble(rs, "CPTMultiplier2", 1.0);
			double dblMultiplier3 = AdoFldDouble(rs, "CPTMultiplier3", 1.0);
			double dblMultiplier4 = AdoFldDouble(rs, "CPTMultiplier4", 1.0);
			COleCurrency cyCopayAmt = AdoFldCurrency(rs, "Copay", COleCurrency(0,0));
			COleCurrency cyOtherInsResp = AdoFldCurrency(rs, "OtherInsRespTotal", COleCurrency(0,0));
			COleCurrency cyPatientResp = AdoFldCurrency(rs, "PatientRespTotal", COleCurrency(0,0));
			CString strServiceName = AdoFldString(rs, "Name", "");
			CString strItemCode = AdoFldString(rs, "ItemCode", "");
			strItemCode.TrimLeft(); strItemCode.TrimRight();

			BOOL bHasModifiers = FALSE;
			if(dblMultiplier1 != 1.0 || dblMultiplier2 != 1.0 || dblMultiplier3 != 1.0 || dblMultiplier4 != 1.0)
				bHasModifiers = TRUE;

			BOOL bIncludeCoPayAllowable = AdoFldBool(rs, "IncludeCoPayAllowable",FALSE);
			// (j.jones 2007-05-03 12:40) - PLID 25840 - supported the IncludePatRespAllowable ability,
			// which checks the patient responsibility for a charge
			BOOL bIncludePatRespAllowable = AdoFldBool(rs, "IncludePatRespAllowable",FALSE);

			COleCurrency cyDiff = cyAllowedQty - cyApplyAmt;

			// (j.jones 2006-05-25 17:58) - PLID 20816 - handle multiple quantity, or (less likely) fractional quantity
			CString strAllowed;
			if(dblQuantity != 1.0) {
				//alter the warning appropriately
				strAllowed.Format("%s (an allowable of %s multiplied by a quantity of %g%s)",
					FormatCurrencyForInterface(cyAllowedQty), FormatCurrencyForInterface(cyAllowed), dblQuantity,
					bHasModifiers ? ", plus applicable modifiers" : "");
			}
			else {

				if(!bHasModifiers) {
					strAllowed = FormatCurrencyForInterface(cyAllowed);
				}
				else {
					//alter the warning appropriately
					strAllowed.Format("%s (an allowable of %s multiplied by applicable modifiers)",
						FormatCurrencyForInterface(cyAllowedQty), FormatCurrencyForInterface(cyAllowed));
				}
			}

			// (j.jones 2007-05-03 12:40) - PLID 25840 - supported the IncludePatRespAllowable ability,
			// which checks the patient responsibility for a charge

			// (j.jones 2011-03-22 15:57) - PLID 42944 - the warning message has been completely rewritten,
			// we now track what info. needs to be reported
			CString strCopayWarn = "";
			CString strOtherInsRespWarn = "";

			//explain if the copay was included
			if(bIncludeCoPayAllowable && cyCopayAmt > COleCurrency(0,0)
				&& !(bIncludePatRespAllowable && (cyOtherInsResp + cyPatientResp) > COleCurrency(0,0))) {

				//reflect that the copay was included
				cyDiff -= cyCopayAmt;

				// (j.jones 2011-03-22 16:04) - PLID 42944 - we only report the copay amount if
				// it was used towards the calculation
				strCopayWarn.Format("CoPay: %s\n",FormatCurrencyForInterface(cyCopayAmt));
			}
			//explain if patient/other resp. was included
			else if(!(bIncludeCoPayAllowable && cyCopayAmt > COleCurrency(0,0))
				&& bIncludePatRespAllowable && (cyOtherInsResp + cyPatientResp) > COleCurrency(0,0)) {

				//reflect that the patient/other responsibility was included
				cyDiff -= (cyOtherInsResp + cyPatientResp);

				// (j.jones 2011-03-22 16:04) - PLID 42944 - we only report the other ins. resp. if
				// it was used towards the calculation
				if(cyOtherInsResp > COleCurrency(0,0)) {
					strOtherInsRespWarn.Format("Other Insurance Resp.: %s\n",FormatCurrencyForInterface(cyOtherInsResp));
				}
			}
			//explain if the greater of copay and patient/other resp is included
			else if(bIncludeCoPayAllowable && cyCopayAmt > COleCurrency(0,0)
				&& bIncludePatRespAllowable && (cyOtherInsResp + cyPatientResp) > COleCurrency(0,0)) {

				//reflect that the greater of the copay or patient/other responsibility was included

				COleCurrency cyGreaterValue;
				if(cyCopayAmt > (cyOtherInsResp + cyPatientResp)) {
					cyGreaterValue = cyCopayAmt;

					// (j.jones 2011-03-22 16:04) - PLID 42944 - we only report the other ins. resp. if
					// it was used towards the calculation
					strCopayWarn.Format("CoPay: %s\n",FormatCurrencyForInterface(cyCopayAmt));
				}
				else {
					cyGreaterValue = (cyOtherInsResp + cyPatientResp);

					// (j.jones 2011-03-22 16:04) - PLID 42944 - we only report the other ins. resp. if
					// it was used towards the calculation
					if(cyOtherInsResp > COleCurrency(0,0)) {
						strOtherInsRespWarn.Format("Other Insurance Resp.: %s\n",FormatCurrencyForInterface(cyOtherInsResp));
					}
				}

				cyDiff -= cyGreaterValue;
			}
			//otherwise give a plain warning			
			else {
				// (j.jones 2011-03-22 16:04) - PLID 42944 - we don't need to do anything
				// special now, everything else will be reported normally
			}

			// (j.jones 2011-03-22 15:57) - PLID 42944 - this message has been completely rewritten
			// (j.jones 2011-04-27 13:57) - PLID 43449 - clarified that this allowable comes from the fee schedule
			// (j.jones 2013-04-15 10:55) - PLID 12136 - products can be warned about now, I improved the charge description
			// to handle cases where the product does or does not have a code
			CString strWarning;
			strWarning.Format("WARNING - This charge has been paid %s less than the allowed amount.\n\n"
				"Charge: %s%s%s\n"
				"Fee Schedule Allowable: %s\n"
				"Payments Applied: %s\n"	
				"Patient Resp.: %s\n"
				"%s"
				"%s",
				FormatCurrencyForInterface(cyDiff),
				strItemCode, strItemCode.IsEmpty() ? "" : " - ", strServiceName,
				strAllowed,
				FormatCurrencyForInterface(cyApplyAmt), 
				FormatCurrencyForInterface(cyPatientResp),
				strCopayWarn, strOtherInsRespWarn);
			
			strWarning.TrimRight("\n");
			
			// (j.jones 2010-01-26 15:09) - PLID 37075 - cannot use MsgBox here incase a service code
			// has a % in its description
			AfxMessageBox(strWarning);
		}
		rs->Close();
		
	}NxCatchAll("Error verifying allowed amount.");
}

// (j.jones 2009-10-06 17:30) - PLID 36426 - this function is not currently used
/*
BOOL GetChargeRespTotals(int iChargeRespID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds, COleCurrency *cyInsResp)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	CString str, strSQL;
	long nActivePatientID = PatientID;

	try {
	
	// (j.jones 2009-10-06 16:15) - PLID 36426 - parameterized and combined
	rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS ChargeRespTotal, "
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 THEN ChargeRespT.Amount ELSE 0 END) AS InsResp "
		"FROM ChargeRespT WHERE ID = {INT} "
		""
		"SELECT Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS SumOfPayAmount, "
		"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS SumOfAdjAmount, "
		"Sum(CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END) AS SumOfRefAmount "
		"FROM AppliesT "
		"LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID < 1) AND AppliesT.RespID = {INT}",
		iChargeRespID, iChargeRespID);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charge
	if (cyCharges) {
		*cyCharges = AdoFldCurrency(rs, "ChargeRespTotal", COleCurrency(0,0));
	}

	// Get total insurance responsibility
	if (cyInsResp) {
		*cyInsResp = AdoFldCurrency(rs, "InsResp", COleCurrency(0,0));;
	}

	rs = rs->NextRecordset(NULL);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}

	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}

	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}	

	rs->Close();

	}NxCatchAllCall("Error in GetChargeTotals", { return FALSE; });
	
	return TRUE;
}
*/

// (j.jones 2009-10-06 17:30) - PLID 36426 - this function is not currently used
/*
BOOL GetChargeRespInsuranceTotals(int iChargeRespID, long PatientID, int iInsType, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds)
{
	_RecordsetPtr rs;
	COleCurrency cyTotal = COleCurrency(0,0);
	_variant_t var;
	long nInsID = GetInsuranceIDFromType(PatientID, iInsType), nActivePatientID = PatientID;
	
	// (j.jones 2009-10-06 16:15) - PLID 36426 - parameterized and combined
	rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS InsuranceResp "
		"FROM ChargeRespT WHERE ID = {INT} AND InsuredPartyID = {INT} "
		""
		"SELECT Sum(CASE WHEN LineItemT.Type=1 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfPayAmount, "
		"Sum(CASE WHEN LineItemT.Type=2 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfAdjAmount, "
		"Sum(CASE WHEN LineItemT.Type=3 And PaymentsT.InsuredPartyID={INT} And {INT}>0 THEN AppliesT.Amount ELSE 0 End) AS SumOfRefAmount "
		"FROM AppliesT LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE AppliesT.RespID = {INT}",
		iChargeRespID,nInsID,
		nInsID, nInsID, nInsID, nInsID, nInsID, nInsID,iChargeRespID);
	
	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total charge
	if (cyTotalResp) {
		*cyTotalResp = AdoFldCurrency(rs, "InsuranceResp", COleCurrency(0,0));
	}

	rs = rs->NextRecordset(NULL);

	if (rs->eof) {
		rs->Close();
		return FALSE;
	}

	// Get total payments
	if (cyPayments) {
		*cyPayments = AdoFldCurrency(rs, "SumOfPayAmount", COleCurrency(0,0));
	}

	// Get total adjustments
	if (cyAdjustments) {
		*cyAdjustments = AdoFldCurrency(rs, "SumOfAdjAmount", COleCurrency(0,0));
	}

	// Get total refunds
	if (cyRefunds) {
		*cyRefunds = AdoFldCurrency(rs, "SumOfRefAmount", COleCurrency(0,0));
	}

	rs->Close();
	
	return TRUE;
}
*/

CString GetNameFromRespTypeID(long nTypeID) {
	//DRT 5/8/03 - Returns the name of a resp. type given it's ID.
	//		Just a simple lookup and return function.

	CString strType;

	try {

		// (j.jones 2009-10-06 16:23) - PLID 36426 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT TypeName FROM RespTypeT WHERE ID = {INT}", nTypeID);
		if(!rs->eof) {
			strType = AdoFldString(rs, "TypeName", "");
		}
		else {
			strType = "Patient";
		}

	} NxCatchAll("Error getting resp. type name.");

	return strType;
}

CString GetNameFromRespPartyID(long nInsPartyID) {
	//DRT 5/8/03 - Returns the name of a resp. type given an insured party ID.
	//		Just a simple lookup and return function.

	CString strType;

	try {

		// (j.jones 2009-10-06 16:23) - PLID 36426 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT TypeName FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON RespTypeID = RespTypeT.ID "
			"WHERE PersonID = {INT}", nInsPartyID);

		if(!rs->eof) {
			strType = AdoFldString(rs, "TypeName", "");
		}
		else {
			strType = "Patient";
		}

	} NxCatchAll("Error getting resp. type name.");

	return strType;
}


/*This function loops through each ChargeDetail of a responsibility and inserts a record*/
// (j.jones 2009-10-06 12:22) - PLID 36426 - changed strRespID to nRespID
// (j.jones 2010-05-17 17:02) - PLID 16503 - added bAllowZeroDollarApply
void ApplyToDetails(long nChargeID, long nRespID, COleCurrency cyAmtToApply, long nApplyID, long nPayID, BOOL bAllowZeroDollarApply) {

	CString strSQL;
	COleCurrency cyZero(0,0);

	//(e.lally 2007-04-04) PLID 25503 - Add the insert statements into one batch and commit it at the end.
	// (a.walling 2010-11-01 10:49) - PLID 40965 - Parameterize, this fills up the cache!

	CParamSqlBatch batch;

	if(nRespID != -1) {

		// (j.jones 2015-02-04 13:26) - PLID 64800 - added a safety check to repair missing ChargeRespDetailT records
		EnsureChargeRespDetailRecord(nRespID);

		// *Note: this is virtually the same query as in ApplyToDetails, and so if one changes, the other should change too
		// (j.jones 2009-10-06 12:23) - PLID 36426 - parameterized		
		_RecordsetPtr rsChargeDetail = CreateParamRecordset("SELECT ChargeRespDetailT.ID, "
			"(ChargeRespDetailT.Amount - CASE WHEN ApplyDetailsQ.Amount IS NULL THEN 0 ELSE ApplyDetailsQ.Amount END) AS Amount "
			"FROM ChargeRespDetailT "
			"LEFT JOIN (SELECT DetailID, Sum(CASE WHEN Amount IS NULL THEN 0 ELSE Amount END) AS Amount "
				"FROM ApplyDetailsT "
				"WHERE DetailID IN (SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = {INT}) "
				"GROUP BY DetailID) ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
			"WHERE ChargeRespID = {INT} ", nRespID, nRespID);
		COleCurrency cyDetailAmt(0,0);
		COleCurrency cyAmtLeftToApply(0,0);
		_variant_t varDetailID;

		cyAmtLeftToApply = cyAmtToApply;
		
		// (j.jones 2010-05-17 17:04) - PLID 16503 - we now take in a parameter for this setting
		//BOOL bAllowZeroDollarApply = IsZeroDollarPayment(nPayID);

		
		if (! rsChargeDetail->eof) {

			//its applied to a charge

			while ( (!rsChargeDetail->eof)  && (cyAmtLeftToApply != COleCurrency(0,0) || bAllowZeroDollarApply)) {

				cyDetailAmt = AdoFldCurrency(rsChargeDetail, "Amount");
				varDetailID = rsChargeDetail->Collect["ID"];

				if (cyDetailAmt > cyAmtLeftToApply) {

					//only apply what we have left to apply
					// (a.walling 2010-11-01 10:49) - PLID 40965 - Getting the ApplyDetailsT.ID can be combined into a single statement; should be an identity eventually
					// We can also use VT_I4 for the DetailID so all these branches use the same query
					// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
					batch.Add(
						"INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount)\r\n"
						"	VALUES ({INT}, {VT_I4}, {OLECURRENCY})\r\n",
						nApplyID, varDetailID, cyAmtLeftToApply);

					//PLID 15032 - the detail amount in greater than what we have to apply,
					//so when we are done, we must be left with nothing, this used to be a wrong calculation
					cyAmtLeftToApply = cyZero;
				}
				//this just ensures we're not always creating a $0.00 apply for fully paid ChargeRespDetail balances
				else if(cyDetailAmt > cyZero || bAllowZeroDollarApply) {
					
					//apply the amount of the detail
					// (a.walling 2010-11-01 10:49) - PLID 40965 - Getting the ApplyDetailsT.ID can be combined into a single statement; should be an identity eventually
					// We can also use VT_I4 for the DetailID so all these branches use the same query
					// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
					batch.Add(
						"INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount)\r\n"
						" VALUES({INT}, {VT_I4}, {OLECURRENCY})\r\n",
						nApplyID, varDetailID, cyDetailAmt);

					cyAmtLeftToApply = cyAmtLeftToApply - cyDetailAmt;

				}

				//decrement the amount we have left to apply
				rsChargeDetail->MoveNext();
			}
		}
		else {

			//this shouldn't happen!!
			ASSERT(FALSE);
		}
	}
	else {

		//it a payment to payment apply, we need to put the whole amount in and make it null for the chargerespdetailId
		// (a.walling 2010-11-01 10:49) - PLID 40965 - Getting the ApplyDetailsT.ID can be combined into a single statement; should be an identity eventually
		// We can also use VT_I4 for the DetailID so all these branches use the same query
		// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
		batch.Add(
			"INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount)\r\n"
			"	VALUES({INT}, {VT_I4}, {OLECURRENCY})\r\n",
			nApplyID, g_cvarNull, cyAmtToApply);

	}

	//Commit our batch of inserts
	batch.Execute(GetRemoteData());
}

//checks for any active global period, and warns accordingly
// (a.walling 2008-07-07 18:02) - PLID 29900 - Added nPatientID
BOOL CheckWarnGlobalPeriod(long nPatientID, COleDateTime dtToCheck /*= COleDateTime::GetCurrentTime()*/, BOOL bUseIgnoreDate /*= FALSE*/, COleDateTime dtToIgnore /*= COleDateTime::GetCurrentTime()*/) {

	try {

		//JJ - 5/8/2003 - We want to know if there are any charges that have a Global Period
		//associated with their CPT Code, and if so, are we currently in that Global Period?
		//We compare the global period days with the ServiceDate, and if we're in that timeframe,
		//prompt the user with a warning about which CPT Code is active and how long it is active.
		//They can then decide if to proceed making the bill.

		//first check their preference, they may not want to be warned at all
		// (j.jones 2012-08-07 16:43) - PLID 51664 - changed the default to be off
		if(GetRemotePropertyInt("CheckWarnGlobalPeriod",0,0,"<None>",TRUE)==0)
			return TRUE;

		// (j.jones 2012-07-23 17:24) - PLID 51651 - added a preference to only track global periods for
		// surgical codes only, if it is disabled when we would look at all codes
		long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

		// (j.jones 2012-07-26 10:31) - PLID 50489 - added another preference to NOT track global periods
		// if the charge uses modifier 78
		long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

		CString order = "ASC";
		if(GetRemotePropertyInt("GlobalPeriodSort",0,0,"<None>",TRUE)==1)
			order = "DESC";

		_RecordsetPtr rs;
		

		// (a.walling 2008-07-07 18:02) - PLID 29900 - use nPatientID
		if(!bUseIgnoreDate) {
			//find all global periods active on the dtToCheck date, which is most often the current date
			// (j.jones 2009-10-06 16:23) - PLID 36426 - parameterized
			// (j.gruber 2011-09-09 09:48) - PLID 45408 - take out voided and originals
			// (j.jones 2012-07-23 17:47) - PLID 51651 - optionally filter on surgical codes only
			// (j.jones 2012-07-26 10:32) - PLID 50489 - optionally exclude modifier 78
			rs = CreateParamRecordset(FormatString("SELECT CPTCodeT.Code, ServiceT.Name, LineItemT.Date, CPTCodeT.GlobalPeriod, "
				"DATEADD(day,GlobalPeriod,LineItemT.Date) AS ExpDate "
				"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
				"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 AND "
				"LineItemT.Date <= {STRING} AND DATEADD(day,GlobalPeriod,LineItemT.Date) > {STRING} "
				"AND LineItemT.PatientID = {INT} "
				"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
				"AND ({INT} <> 1 OR ServicePayGroupsT.Category = {CONST}) "
				"AND ({INT} <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
				"ORDER BY DATEADD(day,GlobalPeriod,LineItemT.Date) %s", order),
				FormatDateTimeForSql(dtToCheck, dtoDate), FormatDateTimeForSql(dtToCheck, dtoDate),
				nPatientID, nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78);
		}
		else {
			//find all global periods active on the dtToCheck date but NOT active on the dtToIgnore date
			// (j.jones 2009-10-06 16:23) - PLID 36426 - parameterized
			// (j.gruber 2011-09-09 09:48) - PLID 45408 - take out voided and originals
			// (j.jones 2012-07-23 17:47) - PLID 51651 - optionally filter on surgical codes only
			// (j.jones 2012-07-26 10:32) - PLID 50489 - optionally exclude modifier 78
			rs = CreateParamRecordset(FormatString("SELECT CPTCodeT.Code, ServiceT.Name, LineItemT.Date, CPTCodeT.GlobalPeriod, "
				"DATEADD(day,GlobalPeriod,LineItemT.Date) AS ExpDate "
				"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
				"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 AND "
				"LineItemT.Date <= {STRING} AND DATEADD(day,GlobalPeriod,LineItemT.Date) > {STRING} "
				"AND (LineItemT.Date > {STRING} OR DATEADD(day,GlobalPeriod,LineItemT.Date) <= {STRING}) "
				"AND LineItemT.PatientID = {INT} "
				"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
				"AND ({INT} <> 1 OR ServicePayGroupsT.Category = {CONST}) "
				"AND ({INT} <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
				"ORDER BY DATEADD(day,GlobalPeriod,LineItemT.Date) %s", order),
				FormatDateTimeForSql(dtToCheck, dtoDate), FormatDateTimeForSql(dtToCheck, dtoDate),
				FormatDateTimeForSql(dtToIgnore, dtoDate), FormatDateTimeForSql(dtToIgnore, dtoDate),
				nPatientID, nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78);
		}

		if(!rs->eof) {

			CString str = "";
			CString strMsg = "This patient is still under the global period for:";

			while(!rs->eof) {

				CString strCPTCode = AdoFldString(rs, "Code","");
				CString strCPTDesc = AdoFldString(rs, "Name","");
				long nGlobalPeriod = AdoFldLong(rs, "GlobalPeriod");
				COleDateTime dtServiceDate = AdoFldDateTime(rs, "Date");
				COleDateTime dtExpDate = AdoFldDateTime(rs, "ExpDate");

				CString strServiceDate = FormatDateTimeForInterface(dtServiceDate);
				CString strExpDate = FormatDateTimeForInterface(dtExpDate);

				str.Format("\n\nService Code: (%s) %s, Service Date: %s, Global Period: %li days, Expires: %s",
					strCPTCode,strCPTDesc,strServiceDate,nGlobalPeriod,strExpDate);

				strMsg += str;	
				
				rs->MoveNext();
			}

			//they will be prompted with this information and given the option to cancel saving
			if(IDOK == MessageBox(GetActiveWindow(),strMsg,"Practice",MB_ICONQUESTION|MB_OKCANCEL))
				return TRUE;
			else
				return FALSE;
		}
		rs->Close();


	}NxCatchAll("Error checking status of Global Period");

	return TRUE;
}

COleCurrency GetChargePatientResp(long nChargeID) {

	COleCurrency cyPatResp = COleCurrency(0,0);

	// (j.jones 2009-10-06 16:26) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(Amount) AS TotResp FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID Is Null",nChargeID);
	if(!rs->eof) {
		cyPatResp = AdoFldCurrency(rs, "TotResp",COleCurrency(0,0));
	}
	rs->Close();

	return cyPatResp;
}

// (j.jones 2011-11-16 13:44) - PLID 46348 - added GetBillPatientBalance
COleCurrency GetBillPatientBalance(long nBillID, long nPatientID)
{
	COleCurrency cyCharges = COleCurrency(0,0),
		cyPayments = COleCurrency(0,0),
		cyAdjustments = COleCurrency(0,0),
		cyRefunds = COleCurrency(0,0),
		cyInsResp = COleCurrency(0,0);

	GetBillTotals(nBillID, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsResp);
	return cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsResp;
}

COleCurrency GetChargePatientBalance(long nChargeID, long nPatientID) {

	COleCurrency cyCharges = COleCurrency(0,0),
		cyPayments = COleCurrency(0,0),
		cyAdjustments = COleCurrency(0,0),
		cyRefunds = COleCurrency(0,0),
		cyInsResp = COleCurrency(0,0);

	GetChargeTotals(nChargeID, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsResp);
	return cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsResp;
}

// (j.jones 2009-10-06 08:59) - PLID 21304 - added GetChargeTotalBalance
COleCurrency GetChargeTotalBalance(long nChargeID) {

	// (j.jones 2011-03-08 10:50) - PLID 41877 - this function was previously wrong, it should be
	// the total balance of any responsibility, it was previously only subtracting patient applies

	_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS ChargeTotal "
		"FROM ChargeRespT WHERE ChargeID = {INT} "
		""
		"SELECT Sum(AppliesT.Amount) AS TotalApplied "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE AppliesT.DestID = {INT} AND LineItemT.Deleted = 0", nChargeID, nChargeID);

	if(rs->eof) {
		rs->Close();
		return COleCurrency(0,0);
	}

	// Get the total charge
	COleCurrency cyCharges = AdoFldCurrency(rs, "ChargeTotal", COleCurrency(0,0));

	rs = rs->NextRecordset(NULL);

	if (rs->eof) {
		rs->Close();
		//should be impossible due to the Sum() function,
		//but if so, return the charge total
		return cyCharges;
	}

	// Get the total applies
	COleCurrency cyTotalApplied = AdoFldCurrency(rs, "TotalApplied", COleCurrency(0,0));

	rs->Close();

	return cyCharges - cyTotalApplied;
}

// (j.jones 2011-03-08 10:56) - PLID 41877 - added GetChargePatientRespAndInsBalance,
// which should really only be used in E-Remittance, it returns the total patient responsibility
// plus the total insurance balance for all insurance responsibilities
// (j.jones 2011-11-08 16:41) - PLID 46240 - this has been renamed, its purpose is now
// to find the charge's patient resp., balance for the given insured party, and responsibility
// for all other insured parties
COleCurrency GetChargeInsBalanceAndOtherResps(long nChargeID, long nInsuredPartyID)
{
	// (j.jones 2011-03-08 11:09) - The purpose of this function is to find out
	// how much money is effectively the patient's responsibility in the long term,
	// which means how much is in patient resp. (regardless of applies), plus how much
	// is in insurance resp. that has not yet been paid.
	// In other words, Total Patient Resp + Total Insurance Balance.
	// (j.jones 2011-11-08 16:41) - PLID 46240 - renamed the function and tweaked the query,
	// it is now the patient's resp (regardless of applies), the open balance for the given
	// insured party, and the total resp. for all other insurances (regardless of applies)

	//get the total of each resp type, and also the total applied to just this insurance
	_RecordsetPtr rs = CreateParamRecordset("SELECT "
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 THEN ChargeRespT.Amount ELSE 0 END) AS InsResp, "
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1 THEN ChargeRespT.Amount ELSE 0 END) AS PatResp "
		"FROM ChargeRespT WHERE ChargeID = {INT} "
		""
		"SELECT Sum(AppliesT.Amount) AS TotalInsApplied "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE AppliesT.DestID = {INT} AND LineItemT.Deleted = 0 "
		"AND PaymentsT.InsuredPartyID = {INT}", nChargeID, nChargeID, nInsuredPartyID);

	if(rs->eof) {
		rs->Close();
		return COleCurrency(0,0);
	}

	// Get the total insurance resp
	COleCurrency cyInsResp = AdoFldCurrency(rs, "InsResp", COleCurrency(0,0));

	// Get the total patient resp
	COleCurrency cyPatResp = AdoFldCurrency(rs, "PatResp", COleCurrency(0,0));

	rs = rs->NextRecordset(NULL);

	if (rs->eof) {
		rs->Close();
		//should be impossible due to the Sum() function,
		//but if so, return the resp totals
		return cyPatResp + cyInsResp;
	}

	// Get the total insurance applies for just this insured party
	COleCurrency cyTotalInsApplied = AdoFldCurrency(rs, "TotalInsApplied", COleCurrency(0,0));

	rs->Close();

	//return the total patient resp + total insurance balance
	return cyPatResp + (cyInsResp - cyTotalInsApplied);
}

// (j.jones 2011-11-08 16:41) - PLID 46240 - this finds the charge's patient resp. and responsibility
// for all other insured parties, completely ignoring the given insured party
COleCurrency GetChargePatientAndOtherRespTotals(long nChargeID, long nInsuredPartyID)
{
	//get the total of each resp type, patient or insurance, that is NOT the given insured party
	_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) AS OtherRespTotal "
		"FROM ChargeRespT WHERE ChargeID = {INT} "
		"AND (InsuredPartyID Is Null OR InsuredPartyID <> {INT})", nChargeID, nInsuredPartyID);

	if(rs->eof) {
		rs->Close();
		return COleCurrency(0,0);
	}

	// Get the total other insurance resp
	COleCurrency cyOtherRespTotal = AdoFldCurrency(rs, "OtherRespTotal", COleCurrency(0,0));

	return cyOtherRespTotal;
}

COleCurrency GetChargeInsResp(long nChargeID, long nInsuredPartyID) {

	COleCurrency cyInsResp = COleCurrency(0,0);

	// (j.jones 2009-10-06 16:26) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(Amount) AS TotResp FROM ChargeRespT WHERE ChargeID = {INT} AND InsuredPartyID = {INT}",nChargeID,nInsuredPartyID);
	if(!rs->eof) {
		cyInsResp = AdoFldCurrency(rs, "TotResp",COleCurrency(0,0));
	}
	rs->Close();

	return cyInsResp;
}

COleCurrency GetChargeInsBalance(long nChargeID, long nPatientID, long nInsuredPartyID) {

	COleCurrency cyCharges = COleCurrency(0,0),
		cyPayments = COleCurrency(0,0),
		cyAdjustments = COleCurrency(0,0),
		cyRefunds = COleCurrency(0,0);

	GetChargeInsuranceTotals(nChargeID, nPatientID, nInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds);
	return cyCharges - cyPayments - cyAdjustments - cyRefunds;
}

BOOL AllowPaymentApply(long nPaymentID, long nDestID, CString strClickedType) {

	//before allowing them to apply, see if the source is a prepayment and the destination
	//is not a bill or charge created from the quote that the prepayment is linked with
	// (j.jones 2009-10-06 16:26) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT PaymentsT.QuoteID, BillsT.Description, (CASE WHEN PackagesT.QuoteID Is Null THEN 0 ELSE 1 END) AS IsPackage "
		"FROM PaymentsT INNER JOIN BillsT ON PaymentsT.QuoteID = BillsT.ID "
		"LEFT JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
		"WHERE PaymentsT.ID = {INT} AND PaymentsT.QuoteID Is Not Null",nPaymentID);
	if(!rs->eof) {
		//it is indeed linked with a quote... make sure we are applying to it
		long nQuoteID = AdoFldLong(rs, "QuoteID",-1);
		CString strPackage = AdoFldLong(rs, "IsPackage",0) == 0 ? "quote" : "package";
		CString strDesc = AdoFldString(rs, "Description","");

		CString strMessage = "";
		if(strClickedType == "Bill") {
			// (j.jones 2009-10-06 16:26) - PLID 36426 - parameterized
			_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM BillsT WHERE ID = {INT} "
				"AND ID IN (SELECT BillID FROM BilledQuotesT WHERE QuoteID = {INT})",nDestID,nQuoteID);
			if(rsCheck->eof) {
				strMessage.Format("The prepayment you are attempting to apply was made for the %s: '%s'.\n"
					"\nAre you SURE you wish to apply it to this bill?",strPackage,strDesc);
			}
			rsCheck->Close();
		}
		else if(strClickedType == "Charge") {
			// (j.jones 2009-10-06 16:26) - PLID 36426 - parameterized
			_RecordsetPtr rsCheck = CreateParamRecordset("SELECT BillsT.ID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"WHERE BillsT.ID IN (SELECT BillID FROM BilledQuotesT WHERE QuoteID = {INT}) AND ChargesT.ID = {INT}",nQuoteID,nDestID);
			if(rsCheck->eof) {
				strMessage.Format("The prepayment you are attempting to apply was made for the %s: '%s'.\n"
					"\nAre you SURE you wish to apply it to this charge?",strPackage,strDesc);
			}
			rsCheck->Close();
		}
		else {
			//if it's not a bill or charge, then we know it is not the quote
			strMessage.Format("The prepayment you are attempting to apply was made for the %s: '%s'.\n"
				"\nAre you SURE you wish to apply it to this line item?",strPackage,strDesc);
		}

		if(strMessage != "") {
			//we do have to warn them
			if(IDNO == MessageBox(GetActiveWindow(),strMessage,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
		}
	}
	rs->Close();

	return TRUE;
}

// (j.jones 2009-02-06 16:25) - PLID 32951 - now this function requires the bill location
// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID, though
// in most cases they will be the same person
// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
ECLIANumber UseCLIANumber(long nBillID, long nInsuredPartyID, long nBillingProviderID, long nRenderingProviderID, long nLocationID)
{
	//calculate if we are going to use the CLIA number, and if so, set up eCLIANumber accordingly

	ECLIANumber eCLIANumber;
	eCLIANumber.bUseCLIANumber = FALSE;
	eCLIANumber.strCLIANumber = "";
	eCLIANumber.bUseCLIAInHCFABox23 = TRUE;
	eCLIANumber.bUseCLIAInHCFABox32 = TRUE;
	eCLIANumber.bCLIAUseBillProvInHCFABox17 = TRUE;

	// (j.jones 2011-04-05 10:20) - PLID 42372 - the CLIA logic has been completely reworked

	//first load the insurance co information and CLIA number for the matching company,
	//location, and provider (provider may match by billing or rendering provider,
	//based on InsuranceCoT.CLIAUseRenderingProvider)
	_RecordsetPtr rsCLIA = CreateParamRecordset("SELECT InsuranceCoT.PersonID, "
		"InsuranceCoT.UseCLIAModifier, InsuranceCoT.CLIAModifier, "
		"Convert(bit, CASE WHEN EXISTS (SELECT TOP 1 ID FROM ServiceT WHERE UseCLIA = 1) THEN 1 ELSE 0 END) AS MatchByCPT, "
		"CLIANumbersT.CLIANumber, "
		"InsuranceCoT.UseCLIAInHCFABox23, InsuranceCoT.UseCLIAInHCFABox32, InsuranceCoT.CLIAUseBillProvInHCFABox17, "
		"InsuranceCoT.CLIAUseRenderingProvider "
		"FROM InsuranceCoT "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
		"INNER JOIN CLIANumbersT ON InsuranceCoT.PersonID = CLIANumbersT.InsuranceCoID "
		"WHERE InsuredPartyT.PersonID = {INT} "
		"AND CLIANumbersT.LocationID = {INT} "
		"AND ("
		"	(InsuranceCoT.CLIAUseRenderingProvider = 0 AND CLIANumbersT.ProviderID = {INT}) "
		"	OR "
		"	(InsuranceCoT.CLIAUseRenderingProvider = 1 AND CLIANumbersT.ProviderID = {INT}) "
		")", nInsuredPartyID, nLocationID, nBillingProviderID, nRenderingProviderID);
	if(!rsCLIA->eof) {

		//there is a matching CLIA number, but we can't use it just yet

		//make sure that the modifer fits, if used
		if(VarBool(rsCLIA->Fields->Item["UseCLIAModifier"]->Value, FALSE)) {
			CString strModifier = VarString(rsCLIA->Fields->Item["CLIAModifier"]->Value, "");
			if(!strModifier.IsEmpty()) {
				// (j.jones 2009-10-07 09:19) - PLID 36426 - parameterized
				_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE Deleted = 0 AND BillID = {INT} "
					"AND (CPTModifier = {STRING} OR CPTModifier2 = {STRING} OR CPTModifier3 = {STRING} OR CPTModifier4 = {STRING})",
					nBillID,strModifier,strModifier,strModifier,strModifier);
				if(rsCheck->eof) {
					//no modifier present, leave now
					eCLIANumber.strCLIANumber = "";
					eCLIANumber.bUseCLIANumber = FALSE;
					return eCLIANumber;
				}
				rsCheck->Close();
			}
		}

		//now check the CPT codes
		if(VarBool(rsCLIA->Fields->Item["MatchByCPT"]->Value, FALSE)) {

			//at least one CPT is required for the CLIA number to be used

			// (j.jones 2009-10-07 09:19) - PLID 36426 - parameterized
			_RecordsetPtr rsCPT = CreateParamRecordset("SELECT ServiceID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"WHERE Deleted = 0 AND BillID = {INT} AND UseCLIA = 1",nBillID);
			if(rsCPT->eof) {
				//they are not billing a CPT code that uses a CLIA number, leave now
				eCLIANumber.strCLIANumber = "";
				eCLIANumber.bUseCLIANumber = FALSE;
				return eCLIANumber;
			}
			rsCPT->Close();
		}

		//if we're still here, our number is good (provided it is not blank)

		eCLIANumber.strCLIANumber = VarString(rsCLIA->Fields->Item["CLIANumber"]->Value, "");
		if(!eCLIANumber.strCLIANumber.IsEmpty()) {

			eCLIANumber.bUseCLIANumber = TRUE;
			
			//load the rest of our settings
			eCLIANumber.bUseCLIAInHCFABox23 = VarBool(rsCLIA->Fields->Item["UseCLIAInHCFABox23"]->Value, TRUE);
			eCLIANumber.bUseCLIAInHCFABox32 = VarBool(rsCLIA->Fields->Item["UseCLIAInHCFABox32"]->Value, TRUE);
			eCLIANumber.bCLIAUseBillProvInHCFABox17 = VarBool(rsCLIA->Fields->Item["CLIAUseBillProvInHCFABox17"]->Value, TRUE);
			return eCLIANumber;
		}
		else {
			eCLIANumber.bUseCLIANumber = FALSE;
		}
	}
	rsCLIA->Close();

	//no CLIA number was loaded
	eCLIANumber.strCLIANumber == "";
	eCLIANumber.bUseCLIANumber = FALSE;
	return eCLIANumber;
}

CString GetDefaultBillingColumnWidths()
{
	// (j.gruber 2007-03-21 15:35) - PLID 24870 - adding disc cat tab
	// (j.jones 2007-12-14 10:41) - PLID 27988 - added allocation detail list ID
	// (j.gruber 2009-03-20 11:08) - PLID 33385 - take out percent off, discount, and discount category and add total discount
	// (j.jones 2010-09-01 10:55) - PLID 40330 - added allowable
	// (j.jones 2010-11-09 09:37) - PLID 31392 - added claim provider
	// (j.jones 2011-10-25 10:01) - PLID 46088 - added Calls
	// (d.singleton 2012-03-09 16:21) - PLID 25098 added status
	// (d.singleton 2012-03-23 09:03) - PLID 49136 added notes
	// (d.singleton 2012-05-22 10:15) - PLID 48152 added skillcode
	// (j.jones 2014-04-28 14:29) - PLID 61836 - added referring, ordering, supervising providers
	// (r.gonet 2015-03-27 10:18) - PLID 65277 - Added Value column.
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - Added missing widths for WHICH_CODES_EXT and ON_HOLD columns, which were bugs.
	//  Placed each width on its own line. Added badly needed column name comments.
	// (s.tullis 2015-07-01 09:29) - PLID 64977 - need to take care of category column here
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO BillingDlg.h's BillColumns ENUMERATION, YOU MUST ADD 
	// THE COLUMN'S WIDTH TO THIS LIST _AND_ THE FORMAT SPECIFIER WIDTH LIST BELOW.
	CString strWidths =
		"0,"	//COLUMN_LINE_ID
		"0,"	//COLUMN_CHARGE_ID
		"0,"	//BILL_VALIDATION_STATUS
		"74,"	//BILL_COLUMN_DATE
		"0,"	//COLUMN_SERVICE_DATE_TO
		"0,"	//COLUMN_INPUT_DATE
		"74,"	//BILL_COLUMN_PROVIDER
		"0,"	//BILL_COLUMN_CLAIM_PROVIDER
		"0,"	//BILL_COLUMN_REFERRING_PROVIDER
		"0,"	//BILL_COLUMN_ORDERING_PROVIDER
		"0,"	//BILL_COLUMN_SUPERVISING_PROVIDER
		"0,"	//COLUMN_PATCOORD
		"0,"	//COLUMN_SERVICE_ID
		"52,"	//BILL_COLUMN_CPT_CODE
		"37,"	//BILL_COLUMN_CPT_SUB_CODE
		"70,"	//BILL_COLUMN_CPT_CATEGORY
		"0,"	//BILL_COLUMN_CPT_TYPE
		"37,"	//BILL_COLUMN_CPT_TYPEOFSERVICE
		"44,"	//COLUMN_MODIFIER1
		"44,"	//COLUMN_MODIFIER2
		"0,"	//COLUMN_MODIFIER3
		"0,"	//COLUMN_MODIFIER4
		"20,"	//COLUMN_CALLS
		"20,"	//COLUMN_SKILL
		"10,"	//BILL_COLUMN_NOTES
		"52,"	//BILL_COLUMN_WHICH_CODES
		//"20,"	//BILL_COLUMN_WHICH_CODES_EXT -- Intentionally absent even though it is in the column enumeration. The Which Codes Ext column is handled specially in CBillingDlg::GetColumnWidth(), where it actually decrements the passed column index if it is beyond Which Codes.
		"20,"	//BILL_COLUMN_DESCRIPTION
		"37,"	//BILL_COLUMN_QUANTITY
		"74,"	//BILL_COLUMN_UNIT_COST
		"74,"	//BILL_COLUMN_VALUE
		"74,"	//BILL_COLUMN_ALLOWABLE
		"52,"	//COLUMN_TOTAL_DISCOUNT
		"74,"	//BILL_COLUMN_LINE_TOTAL
		"74,"	//COLUMN_INS_RESP
		"0,"	//COLUMN_INS_PARTY_ID
		"44,"	//COLUMN_TAX_RATE_1
		"44,"	//COLUMN_TAX_RATE_2
		"0,"	//COLUMN_ITEM_TYPE
		"0,"	//COLUMN_PRODUCT_ITEM_ID
		"0,"	//COLUMN_ALLOCATION_DETAIL_LIST_ID
		"0,"	//COLUMN_BATCHED
		"0,"	//COLUMN_PACKAGE_CHARGE_REF_ID
		"0";	//BILL_COLUMN_ON_HOLD

	try {
		//create the record if it doesn't exist
		// (j.gruber 2009-03-20 11:09) - PLID 33385 - take out percent off, discount, and discount category and add total discount

		// (j.jones 2009-10-06 17:53) - PLID 36426 - parameterized
		_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM ConfigBillColumnsT WHERE LocationID = {INT}", GetCurrentLocationID());
		if(prs->eof) {
			prs->Close();
			//create the record and reopen the recordset
			prs = CreateParamRecordset("SET NOCOUNT ON "
				"INSERT INTO ConfigBillColumnsT (LocationID) VALUES ({INT})"
				"SET NOCOUNT OFF "
				"SELECT * FROM ConfigBillColumnsT WHERE LocationID = {INT}", GetCurrentLocationID(), GetCurrentLocationID());
		}

		FieldsPtr f = prs->Fields;
		if (!prs->eof)
		{
			// (r.gonet 2015-03-27 10:18) - PLID 65277 - Added Value column.
			// (r.gonet 2015-03-27 10:18) - PLID 65462 - Added missing widths for WHICH_CODES_EXT, SKILLS, PACKAGE_CHARGE_REF_ID, and ON_HOLD columns, which were bugs. Added Value column. 
			//  Placed each width on its own line. Added badly needed column name comments.
			// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO BillingDlg.h's BillColumns ENUMERATION, YOU MUST ADD 
			// (s.tullis 2015-07-01 09:29) - PLID 64977 - Need to take care of category column here
			// THE COLUMN'S WIDTH TO THIS LIST _AND_ THE strWidths LIST ABOVE.
			strWidths.Format(
				"0,"	//COLUMN_LINE_ID
				"0,"	//COLUMN_CHARGE_ID
				"0,"	//BILL_VALIDATION_STATUS
				"74,"	//BILL_COLUMN_DATE
				"%d,"	//COLUMN_SERVICE_DATE_TO
				"%d,"	//COLUMN_INPUT_DATE
				"74,"	//BILL_COLUMN_PROVIDER
				"%d,"	//BILL_COLUMN_CLAIM_PROVIDER
				"0,"	//BILL_COLUMN_REFERRING_PROVIDER
				"0,"	//BILL_COLUMN_ORDERING_PROVIDER
				"0,"	//BILL_COLUMN_SUPERVISING_PROVIDER
				"%d,"	//COLUMN_PATCOORD
				"0,"	//COLUMN_SERVICE_ID
				"%d,"	//BILL_COLUMN_CPT_CODE
				"%d,"	//BILL_COLUMN_CPT_SUB_CODE
				"%d,"	//BILL_COLUMN_CPT_CATEGORY
				"0,"	//BILL_COLUMN_CPT_TYPE
				"%d,"	//BILL_COLUMN_CPT_TYPEOFSERVICE
				"%d,"	//COLUMN_MODIFIER1
				"%d,"	//COLUMN_MODIFIER2
				"%d,"	//COLUMN_MODIFIER3
				"%d,"	//COLUMN_MODIFIER4
				"20,"	//COLUMN_CALLS
				"20,"	//COLUMN_SKILL
				"10,"	//BILL_COLUMN_NOTES
				"%d,"	//BILL_COLUMN_WHICH_CODES
				//"20,"	//BILL_COLUMN_WHICH_CODES_EXT -- Intentionally absent even though it is in the column enumeration. The Which Codes Ext column is handled specially in CBillingDlg::GetColumnWidth(), where it actually decrements the passed column index if it is beyond Which Codes.
				"20,"	//BILL_COLUMN_DESCRIPTION
				"37,"	//BILL_COLUMN_QUANTITY
				"74,"	//BILL_COLUMN_UNIT_COST
				"74,"	//BILL_COLUMN_VALUE
				"%d,"	//BILL_COLUMN_ALLOWABLE
				"%d,"	//COLUMN_TOTAL_DISCOUNT
				"74,"	//BILL_COLUMN_LINE_TOTAL
				"74,"	//COLUMN_INS_RESP
				"0,"	//COLUMN_INS_PARTY_ID
				"%d,"	//COLUMN_TAX_RATE_1
				"%d,"	//COLUMN_TAX_RATE_2
				"0,"	//COLUMN_ITEM_TYPE
				"0,"	//COLUMN_PRODUCT_ITEM_ID
				"0,"	//COLUMN_ALLOCATION_DETAIL_LIST_ID
				"0,"	//COLUMN_BATCHED
				"0,"	//COLUMN_PACKAGE_CHARGE_REF_ID
				"0"		//BILL_COLUMN_ON_HOLD
				, AdoFldBool(f, "BillServiceDateTo") ? 74 : 0, // Bill service date to (3)
				AdoFldBool(f, "BillInputDate") ? 74 : 0, // Bill input date (4)
				// (j.jones 2010-11-09 09:37) - PLID 31392 - added claim provider
				AdoFldBool(f, "BillClaimProvider") ? 74 : 0, // Claim Provider (6)				
				AdoFldBool(f, "BillPatientCoordinator") ? 74 : 0, // Patient coordinator (10)
				AdoFldBool(f, "BillCPTCode") ? 52 : 0, // Service code (12)
				AdoFldBool(f, "BillCPTSubCode") ? 37 : 0, // Subcode (13)
				AdoFldBool(f, "BillChargeCategory") ? 70 : 0, // CPT Category(15)
				AdoFldBool(f, "BillTOS") ? 37 : 0, // TOS (15)
				AdoFldBool(f, "BillMod1") ? 44 : 0, // Modifier 1 (16)
				AdoFldBool(f, "BillMod2") ? 44 : 0, // Modifier 2 (17)
				AdoFldBool(f, "BillMod3") ? 44 : 0, // Modifier 3 (18)
				AdoFldBool(f, "BillMod4") ? 44 : 0, // Modifier 4 (19)
				AdoFldBool(f, "BillDiagCs") ? 52 : 0, // Diag codes (20)
				// (j.jones 2010-09-01 11:06) - PLID 40330 - added bill allowable
				AdoFldBool(f, "BillAllowable") ? 74 : 0, // Allowable (24)
				AdoFldBool(f, "BillTotalDiscount") ? 74 : 0, // Total Discount (25)
				AdoFldBool(f, "BillTax1") ? 44 : 0, // Tax 1 (29)
				AdoFldBool(f, "BillTax2") ? 44 : 0); // Tax 2 (30)
		}
	}
	NxCatchAll("Error getting the default billing column widths");
	return strWidths;
}

// (j.jones 2009-12-23 09:19) - PLID 32587 - added bShowInitialValue
CString GetDefaultQuoteColumnWidths(BOOL bIsMultiUsePackage, BOOL bShowInitialValue)
{
	// (j.gruber 2007-03-21 15:35) - PLID 24870 - adding disc cat tab
	// (j.jones 2007-07-06 13:49) - PLID 26098 - added item type column to quotes
	// (j.gruber 2009-03-20 11:09) - PLID 33385 - take out percent off, discount, and discount category and add total discount
	// (j.gruber 2009-10-19 13:11) - PLID 36000 - add allowable to quote
	// (j.jones 2009-12-23 09:23) - PLID 32587 - added initial package qty.
	// (j.jones 2010-09-01 10:55) - PLID 40330 - moved the allowable to be after the unit costs
	// (j.jones 2011-10-25 10:01) - PLID 46088 - added Calls
	// (d.singleton 2012-05-22 10:19) - PLID 48152 added skillcode
	// (b.savon 2012-05-31 16:20) - PLID 50570 - This was out of sync with the DEFAULT_QUOTE_COLUMN_WIDTHS in ButtonCallbacks.h	
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - Placed each width on its own line. Added badly needed column name comments.
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO BillingDlg.h's QuoteColumns ENUMERATION, YOU MUST ADD 
	// (s.tullis 2015-07-01 09:29) - PLID 64977 - Need to take care of category column here
	// THE COLUMN'S WIDTH TO THIS LIST _AND_ THE FORMAT SPECIFIER WIDTH LIST BELOW.
	CString strWidths = 
		"0,"	//COLUMN_LINE_ID
		"0,"	//COLUMN_CHARGE_ID
		"74,"	//QUOTE_COLUMN_PROVIDER
		"0,"	//QUOTE_COLUMN_SERVICE_ID
		"44,"	//QUOTE_COLUMN_CPT_CODE
		"34,"	//QUOTE_COLUMN_CPT_SUB_CODE
		"70,"	//QUOTE_COLUMN_CPT_CATEGORY
		"0,"	//QUOTE_COLUMN_CPT_TYPE
		"0,"	//QUOTE_COLUMN_MODIFIER1
		"0,"	//QUOTE_COLUMN_MODIFIER2
		"0,"	//QUOTE_COLUMN_MODIFIER3
		"0,"	//QUOTE_COLUMN_MODIFIER4
		"29,"	//QUOTE_COLUMN_CALLS
		"20,"	//QUOTE_COLUMN_SKILL
		"133,"	//QUOTE_COLUMN_DESCRIPTION
		"29,"	//QUOTE_COLUMN_QUANTITY
		"58,"	//QUOTE_COLUMN_PACKAGE_QTY_REM
		"63,"	//QUOTE_COLUMN_PACKAGE_ORIGINAL_QTY_REM
		"93,"	//QUOTE_COLUMN_UNIT_COST
		"93,"	//QUOTE_COLUMN_UNIT_COST_OUTSIDE_PRACTICE
		"89,"	//QUOTE_COLUMN_ALLOWABLE
		"60,"	//QUOTE_COLUMN_TOTAL_DISCOUNT
		"89,"	//QUOTE_COLUMN_LINE_TOTAL
		"37,"	//QUOTE_COLUMN_TAX_RATE_1
		"37,"	//QUOTE_COLUMN_TAX_RATE_2
		"0";	//QUOTE_COLUMN_ITEM_TYPE

	try {

		// (j.jones 2009-10-06 17:53) - PLID 36426 - parameterized
		_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM ConfigBillColumnsT WHERE LocationID = {INT}", GetCurrentLocationID());
		if(prs->eof) {
			prs->Close();
			//create the record and reopen the recordset
			prs = CreateParamRecordset("SET NOCOUNT ON "
				"INSERT INTO ConfigBillColumnsT (LocationID) VALUES ({INT})"
				"SET NOCOUNT OFF "
				"SELECT * FROM ConfigBillColumnsT WHERE LocationID = {INT}", GetCurrentLocationID(), GetCurrentLocationID());
		}
		FieldsPtr f = prs->Fields;
		if (!prs->eof)
		{
			// (b.savon 2012-05-31 16:20) - PLID 50570 - Which means this was also out of sync with the DEFAULT_QUOTE_COLUMN_WIDTHS in ButtonCallbacks.h
			// changed a few of the formats to reflect the default values.  When adding a new column, this must be updated.
			// (r.gonet 2015-03-27 10:18) - PLID 65462 - Placed each width on its own line. Added badly needed column name comments.
			// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO BillingDlg.h's QuoteColumns ENUMERATION, YOU MUST ADD 
			// (s.tullis 2015-07-01 09:29) - PLID 64977 - Need to take care of category column here
			// THE COLUMN'S WIDTH TO THIS LIST _AND_ THE strWidths LIST ABOVE.
			strWidths.Format(
				"0,"	//COLUMN_LINE_ID
				"0,"	//COLUMN_CHARGE_ID
				"%d,"	//QUOTE_COLUMN_PROVIDER
				"0,"	//QUOTE_COLUMN_SERVICE_ID
				"%d,"	//QUOTE_COLUMN_CPT_CODE
				"%d,"	//QUOTE_COLUMN_CPT_SUB_CODE
				"%d,"	//QUOTE_COLUMN_CPT_CATEGORY
				"0,"	//QUOTE_COLUMN_CPT_TYPE
				"%d,"	//QUOTE_COLUMN_MODIFIER1
				"%d,"	//QUOTE_COLUMN_MODIFIER2
				"%d,"	//QUOTE_COLUMN_MODIFIER3
				"%d,"	//QUOTE_COLUMN_MODIFIER4
				"29,"	//QUOTE_COLUMN_CALLS
				"20,"	//QUOTE_COLUMN_SKILL
				"133,"	//QUOTE_COLUMN_DESCRIPTION
				"29,"	//QUOTE_COLUMN_QUANTITY
				"%d,"	//QUOTE_COLUMN_PACKAGE_QTY_REM
				"%d,"	//QUOTE_COLUMN_PACKAGE_ORIGINAL_QTY_REM
				"93,"	//QUOTE_COLUMN_UNIT_COST
				"%d,"	//QUOTE_COLUMN_UNIT_COST_OUTSIDE_PRACTICE
				"%d,"	//QUOTE_COLUMN_ALLOWABLE
				"%d,"	//QUOTE_COLUMN_TOTAL_DISCOUNT
				"89,"	//QUOTE_COLUMN_LINE_TOTAL
				"%d,"	//QUOTE_COLUMN_TAX_RATE_1
				"%d,"	//QUOTE_COLUMN_TAX_RATE_2
				"0"		//QUOTE_COLUMN_ITEM_TYPE
				, AdoFldBool(f, "QuoteProvider") ? 74 : 0,
				AdoFldBool(f, "QuoteCPTCode") ? 44 : 0, // Service Code (4)
				AdoFldBool(f, "QuoteCPTSubCode") ? 34 : 0, // Subcode (5)
				AdoFldBool(f, "QuoteChargeCategory") ? 70 : 0, // CPT Category (6)
				AdoFldBool(f, "QuoteMod1") ? 44 : 0, // Modifier 1 (7)
				AdoFldBool(f, "QuoteMod2") ? 44 : 0, // Modifier 2 (8)
				AdoFldBool(f, "QuoteMod3") ? 44 : 0, // Modifier 3 (9)
				AdoFldBool(f, "QuoteMod4") ? 44 : 0, // Modifier 4 (10)
				bIsMultiUsePackage ? 58 : 0,	//Package Qty. Remaining (4)
				// (j.jones 2009-12-23 09:23) - PLID 32587 - added initial package qty.
				bIsMultiUsePackage && bShowInitialValue ? 63 : 0, //Initial Package Qty.
				AdoFldBool(f, "QuoteOutsideCost") ? 93 : 0, // Unit cost outside practice (14)
				AdoFldBool(f, "QuoteAllowable") ? 89 : 0, // Allowable
				AdoFldBool(f, "QuoteTotalDiscount") ? 60 : 0, // TotalDiscount (16)
				//AdoFldBool(f, "QuoteDiscount") ? 52 : 0, // Discount (16)
				// (j.gruber 2007-03-21 15:36) - PLID 24870 - show the discount category column if either discount or percent off is showing
				//AdoFldBool(f, "QuotePercentOff") ? 7: AdoFldBool(f, "QuoteDiscount") ? 7 : 0, 
				AdoFldBool(f, "QuoteTax1") ? 37 : 0, // Tax 1 (18)
				AdoFldBool(f, "QuoteTax2") ? 37 : 0); // Tax 2 (19)				
		}
	}
	NxCatchAll("Error getting the default quote column widths");
	return strWidths;
}

// (j.jones 2010-01-18 13:56) - PLID 36913 - added bForceTimePrompt, which will prompt for times
// even if we already have them, incase we provided defaults that we wish the user to confirm
BOOL CheckAnesthesia(long nServiceID, BOOL &bAnesthesia, COleCurrency &cyAnesthUnitCost, double &dblAnesthUnits, long &nAnesthMinutes, CString &strStartTime, CString &strEndTime, long nPlaceOfServiceID, BOOL bRoundUp /*= TRUE*/, BOOL bForceTimePrompt /*= FALSE*/) {

	// (j.jones 2004-07-07 11:11) - check to see if it is an anesthesia code and we're using anesthesia billing,
	// (j.jones 2009-10-07 08:46) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceT.ID "
		"FROM ServiceT "
		"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = {INT} AND Anesthesia = 1 AND UseAnesthesiaBilling = 1", nServiceID);
	if(!rs->eof) {
		//we do indeed to make the anesthesia calculation
		bAnesthesia = TRUE;

		//load up the configuration for this place of service
		long nAnesthesiaFeeBillType = 1;
		long nAnesthTimePromptType = 1;

		// (j.jones 2007-10-15 14:14) - PLID 27757 - converted this data structure to be per service code and place of service
		// (don't worry right now if no setup exists)
		_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthesiaFeeBillType, AnesthTimePromptType FROM AnesthesiaSetupT WHERE ServiceID = {INT} AND LocationID = {INT}", nServiceID, nPlaceOfServiceID);
		if(!rs->eof) {
			nAnesthesiaFeeBillType = AdoFldLong(rs, "AnesthesiaFeeBillType",1);
			nAnesthTimePromptType = AdoFldLong(rs, "AnesthTimePromptType",1);			
		}
		rs->Close();

		long nCurAnesthMinutes = 0;		

		if(nAnesthesiaFeeBillType != 1) { // if a flat fee, we don't need to prompt for minutes

			//prompt for minutes
			if(nAnesthTimePromptType == 1) {
				//prompt for begin and end times

				// (j.jones 2010-01-18 13:56) - PLID 36913 - if bForceTimePrompt is TRUE,
				// we will prompt no matter what defaults we have
				if(!bForceTimePrompt && strStartTime != "" && strEndTime != "" && nAnesthMinutes != 0) {
					//we already have the times, so do not prompt
					nCurAnesthMinutes = nAnesthMinutes;
				}
				else {

					BOOL bDone = FALSE;
					while(!bDone) {
						CAnesthesiaTimePromptDlg dlg(NULL);

						// (j.jones 2010-01-18 13:56) - PLID 36913 - pre-fill whatever defaults we may already have
						if(strStartTime != "") {
							dlg.m_strStartTime = strStartTime;
						}
						if(strEndTime != "") {
							dlg.m_strEndTime = strEndTime;
						}

						dlg.m_nServiceID = nServiceID;
						if(dlg.DoModal() == IDCANCEL) {
							if(IDYES == MessageBox(GetActiveWindow(),"You must enter anesthesia start and end times in order to add this anesthesia charge.\n"
								"If you cancel, the charge will not be added.\n\n"
								"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return FALSE;
							}
						}
						else bDone = TRUE;

						nCurAnesthMinutes = dlg.m_nMinutes;
						strStartTime = dlg.m_strStartTime;
						strEndTime = dlg.m_strEndTime;
					}
				}
			}
			else if(nAnesthTimePromptType == 2) {

				//prompt for anesthesia minutes

				// (j.jones 2010-01-18 13:56) - PLID 36913 - if bForceTimePrompt is TRUE,
				// we will prompt no matter what defaults we have
				if(!bForceTimePrompt && nAnesthMinutes != 0) {
					//we already have the minutes, so do not prompt
					nCurAnesthMinutes = nAnesthMinutes;
				}
				else {
				
					BOOL bDone = FALSE;

					CString strPrompt = "Anesthesia Minutes";

					_RecordsetPtr rs = CreateParamRecordset("SELECT Code + ' - ' + Name AS Description FROM ServiceT "
						"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE CPTCodeT.ID = {INT}",nServiceID);
					if(!rs->eof) {
						CString strDesc = AdoFldString(rs, "Description","");
						strPrompt.Format("Enter the minutes of anesthesia for code:\r\n%s",strDesc);
					}
					rs->Close();

					while(!bDone) {
						CString strMinutes = "0";

						// (j.jones 2010-01-18 13:56) - PLID 36913 - pre-fill a default if we have one
						if(nAnesthMinutes != 0) {
							strMinutes = AsString(nAnesthMinutes);
						}

						if (InputBox(NULL, strPrompt, strMinutes, "", false, false, "Cancel", TRUE) != IDOK) {
							if(IDYES == MessageBox(GetActiveWindow(),"You must enter the anesthesia minutes in order to add this anesthesia charge.\n"
								"If you cancel, the charge will not be added.\n\n"
								"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return FALSE;
							}
						}
						else {

							if(strMinutes == "0") {
								AfxMessageBox("You must enter an amount between 1 and 1440 for the anesthesia minutes.");
								continue;
							}

							if(strMinutes.GetLength() > 6) {
								AfxMessageBox("You have entered an invalid amount for the anesthesia minutes.");
								continue;
							}

							if(atoi(strMinutes) > 1440) {
								AfxMessageBox("You may not enter more than 1440 minutes.");
								continue;
							}
							
							bDone = TRUE;
						}

						nCurAnesthMinutes = atoi(strMinutes);
						strStartTime = "";
						strEndTime = "";
					}
				}
			}
		}

		if(nAnesthMinutes == 0 || nCurAnesthMinutes != 0)			
			nAnesthMinutes = nCurAnesthMinutes;

		CalcAnesthesia(nServiceID, cyAnesthUnitCost, dblAnesthUnits, nCurAnesthMinutes, nPlaceOfServiceID, bRoundUp);
	}
	else {
		bAnesthesia = FALSE;
		nAnesthMinutes = 0;
		dblAnesthUnits = 0;
		cyAnesthUnitCost = COleCurrency(0,0);
	}
	rs->Close();

	return TRUE;
}

void CalcAnesthesia(long nServiceID, COleCurrency &cyAnesthUnitCost, double &dblAnesthUnits, long nTotalAnesthMinutes, long nPlaceOfServiceID, BOOL bRoundUp /*= TRUE*/)
{
	//now actually calculate the anesthesia fee

	// (j.jones 2007-10-15 14:14) - PLID 27757 - converted this data structure to be per service code and place of service

	//load up all the values at once - if no record exists, create a new one and load its defaults
	_RecordsetPtr rsAnesthSetup = CreateParamRecordset("SELECT AnesthesiaSetupT.ID, AnesthesiaFeeBillType, AnesthFlatFee, "
		"AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, AnesthAddlIncrement, "
		"AnesthFeeTableSearchType, AnesthUnitBaseCost, AnesthMinutesPerUnit, "
		"AnesthTimePromptType, AnesthesiaPayTo, AnesthOutsideFee FROM AnesthesiaSetupT "
		"INNER JOIN LocationsT ON AnesthesiaSetupT.LocationID = LocationsT.ID "
		"WHERE AnesthesiaSetupT.ServiceID = {INT} AND AnesthesiaSetupT.LocationID = {INT}", nServiceID, nPlaceOfServiceID);

	if(rsAnesthSetup->eof) {
		//if no records exist for this CPT/POS, create a new record, and pull the default values
		rsAnesthSetup->Close();
		rsAnesthSetup = CreateParamRecordset("SET NOCOUNT ON\r\n"
			"INSERT INTO AnesthesiaSetupT (ServiceID, LocationID) VALUES ({INT}, {INT})\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT AnesthesiaSetupT.ID, AnesthesiaFeeBillType, AnesthFlatFee, "
			"AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, AnesthAddlIncrement, "
			"AnesthFeeTableSearchType, AnesthUnitBaseCost, AnesthMinutesPerUnit, "
			"AnesthTimePromptType, AnesthesiaPayTo, AnesthOutsideFee FROM AnesthesiaSetupT "
			"INNER JOIN LocationsT ON AnesthesiaSetupT.LocationID = LocationsT.ID "
			"WHERE AnesthesiaSetupT.ID = Convert(int, SCOPE_IDENTITY())",nServiceID, nPlaceOfServiceID);
	}

	if(rsAnesthSetup->eof) {
		ThrowNxException("CalcAnesthesia could not create a new setup record!");
	}

	long nAnesthesiaSetupID = AdoFldLong(rsAnesthSetup, "ID");

	//load up the configuration for this place of service
	long nAnesthesiaFeeBillType = AdoFldLong(rsAnesthSetup, "AnesthesiaFeeBillType",1);	

	long nCurAnesthMinutes = nTotalAnesthMinutes;		

	if(nAnesthesiaFeeBillType == 1) {	//flat fee
		//just return the flat fee now since we don't need to do any calculations
		cyAnesthUnitCost = AdoFldCurrency(rsAnesthSetup, "AnesthFlatFee",COleCurrency(0,0));
		dblAnesthUnits = 1.0;
	}
	else if(nAnesthesiaFeeBillType == 2) { //incremental fee

		dblAnesthUnits = 1.0;
		
		//increments are 1 - hour, 2 - half hour, 3 - quarter hour
		COleCurrency cyAnesthIncrementalBaseFee = AdoFldCurrency(rsAnesthSetup, "AnesthIncrementalBaseFee",COleCurrency(0,0));
		long nAnesthBaseIncrement = AdoFldLong(rsAnesthSetup, "AnesthBaseIncrement",1);
		COleCurrency cyAnesthIncrementalAddlFee = AdoFldCurrency(rsAnesthSetup, "AnesthIncrementalAddlFee",COleCurrency(0,0));
		long nAnesthAddlIncrement = AdoFldLong(rsAnesthSetup, "AnesthAddlIncrement",1);
		
		if(nAnesthBaseIncrement == 1) { //hour
			if(nCurAnesthMinutes <= 60) {
				//if under an hour, give the 1 hour rate, and set remaining time to zero
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes = 0;
			}
			else {
				//otherwise, still give the hour rate, and reduce remaining time by 60 minutes
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes -= 60;
			}
		}
		else if(nAnesthBaseIncrement == 2) { //half-hour
			if(nCurAnesthMinutes <= 30) {
				//if under a half-hour, give the half-hour rate, and set remaining time to zero
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes = 0;
			}
			else {
				//otherwise, still give the half-hour rate, and reduce remaining time by 30 minutes
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes -= 30;
			}
		}
		else if(nAnesthBaseIncrement == 3) { //quarter-hour
			if(nCurAnesthMinutes <= 15) {
				//if under a quarter-hour, give the quarter-hour rate, and set remaining time to zero
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes = 0;
			}
			else {
				//otherwise, still give the quarter-hour rate, and reduce remaining time by 15 minutes
				cyAnesthUnitCost = cyAnesthIncrementalBaseFee;
				nCurAnesthMinutes -= 15;
			}
		}

		//if there is time left
		while(nCurAnesthMinutes > 0) {
			if(nAnesthAddlIncrement == 1) { //hour
				if(nCurAnesthMinutes <= 60) {
					//if under an hour, give the 1 hour rate, and set remaining time to zero
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes = 0;
				}
				else {
					//otherwise, still give the hour rate, and reduce remaining time by 60 minutes
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes -= 60;
				}
			}
			else if(nAnesthAddlIncrement == 2) { //half-hour
				if(nCurAnesthMinutes <= 30) {
					//if under a half-hour, give the half-hour rate, and set remaining time to zero
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes = 0;
				}
				else {
					//otherwise, still give the half-hour rate, and reduce remaining time by 30 minutes
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes -= 30;
				}
			}
			else if(nAnesthAddlIncrement == 3) { //quarter-hour
				if(nCurAnesthMinutes <= 15) {
					//if under a quarter-hour, give the quarter-hour rate, and set remaining time to zero
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes = 0;
				}
				else {
					//otherwise, still give the quarter-hour rate, and reduce remaining time by 15 minutes
					cyAnesthUnitCost += cyAnesthIncrementalAddlFee;
					nCurAnesthMinutes -= 15;
				}
			}
		}
	}
	else if(nAnesthesiaFeeBillType == 3) { //scheduled fee
		
		dblAnesthUnits = 1.0;

		//1 - lesser time, 2 - greater time
		long nAnesthFeeTableSearchType = AdoFldLong(rsAnesthSetup, "AnesthFeeTableSearchType",2);

		//now grab the right fee from LocationAnesthesiaFeesT
		CString str;
		str.Format("SELECT Fee FROM LocationAnesthesiaFeesT "
			"WHERE AnesthesiaSetupID = {INT} AND ((Hours * 60) + Minutes) %s {INT} ORDER BY Hours %s, Minutes %s",
			nAnesthFeeTableSearchType == 2 ? ">=" : "<=", nAnesthFeeTableSearchType == 2 ? "ASC" : "DESC",
			nAnesthFeeTableSearchType == 2 ? "ASC" : "DESC");
		_RecordsetPtr rsFees = CreateParamRecordset(str, nAnesthesiaSetupID, nCurAnesthMinutes);
		if(!rsFees->eof) {
			cyAnesthUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
		}
		else {
			//try not filtering out the ones less than or greater than the value
			rsFees->Close();
			// (j.jones 2009-10-07 09:09) - PLID 36426 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM LocationAnesthesiaFeesT "
				"WHERE (Hours * 60) + Minutes > {INT} AND AnesthesiaSetupID = {INT}",nCurAnesthMinutes,nAnesthesiaSetupID);

			if(rs->eof) {
				//our time entered is greater than anything in the list
				rsFees = CreateParamRecordset("SELECT TOP 1 Fee FROM LocationAnesthesiaFeesT "
					"WHERE AnesthesiaSetupID = {INT} "
					"ORDER BY Hours DESC, Minutes DESC ", nAnesthesiaSetupID);
				if(!rsFees->eof) {
					cyAnesthUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
					dblAnesthUnits = 1.0;
					return;
				}
				rsFees->Close();
			}
			rs->Close();

			// (j.jones 2009-10-07 09:09) - PLID 36426 - parameterized
			rs = CreateParamRecordset("SELECT ID FROM LocationAnesthesiaFeesT "
				"WHERE (Hours * 60) + Minutes < {INT} AND AnesthesiaSetupID = {INT}",nCurAnesthMinutes,nAnesthesiaSetupID);

			if(rs->eof) {
				//our time entered is less than anything in the list
				rsFees = CreateParamRecordset("SELECT TOP 1 Fee FROM LocationAnesthesiaFeesT "
					"WHERE AnesthesiaSetupID = {INT} "
					"ORDER BY Hours ASC, Minutes ASC ", nAnesthesiaSetupID);
				if(!rsFees->eof) {
					cyAnesthUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
					dblAnesthUnits = 1.0;
					return;
				}
				rsFees->Close();
			}
			rs->Close();

			//we shouldn't be able to get here unless there is nothing set up in the anesthesia
			//table, which is now impossible to do
			cyAnesthUnitCost = COleCurrency(0,0);
			dblAnesthUnits = 1.0;
			return;
		}
	}
	else if(nAnesthesiaFeeBillType == 4) { //unit-based calculation

		//now that we have the minute calculated, calculate the units
		long nBaseUnits = 0;
		_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthBaseUnits FROM CPTCodeT WHERE ID = {INT}",nServiceID);
		if(!rs->eof) {
			nBaseUnits = AdoFldLong(rs, "AnesthBaseUnits",0);
		}
		rs->Close();

		long nMinutesPerUnit = AdoFldLong(rsAnesthSetup, "AnesthMinutesPerUnit",1);
		cyAnesthUnitCost = AdoFldCurrency(rsAnesthSetup, "AnesthUnitBaseCost",COleCurrency(0,0));

		// (j.jones 2004-07-07 12:34) - calculate the base units plus time units, and round to next whole number, or tenths
		double dblUnits = (double)nBaseUnits + ((double)nCurAnesthMinutes / (double)nMinutesPerUnit);
		if((double)(long)dblUnits != dblUnits) {
			//need to round... but to how much?
			if(bRoundUp) //round to next whole number
				dblAnesthUnits = (long)dblUnits + 1;
			else { //round to next 1/10th
				if((double)(long)(dblUnits * 10) != (dblUnits * 10)) {
					dblAnesthUnits = ((long)(((dblUnits * 10) + 1)))/10.0;
				}
				else {
					//already at the 1/10th mark
					dblAnesthUnits = dblUnits;
				}
			}
		}
		else {
			dblAnesthUnits = (long)dblUnits;
		}
	}
}

// (j.jones 2010-01-18 13:56) - PLID 36913 - added bForceTimePrompt, which will prompt for times
// even if we already have them, incase we provided defaults that we wish the user to confirm
BOOL CheckFacilityFee(long nServiceID, BOOL &bFacilityFee, COleCurrency &cyFacilityUnitCost, long &nFacilityMinutes, CString &strStartTime, CString &strEndTime, long nPlaceOfServiceID, BOOL bForceTimePrompt /*= FALSE*/) {

	// (j.jones 2005-07-01 11:34) - check to see if it is a facility fee code and we're using facility fee billing
	// (j.jones 2009-10-07 09:19) - PLID 36426 - parameterized
	_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ServiceT.ID FROM ServiceT "
		"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = {INT} AND FacilityFee = 1 AND UseFacilityBilling = 1", nServiceID);
	if(!rsCheck->eof) {

		//we do indeed to make the facility fee calculation
		bFacilityFee = TRUE;

		//load up the configuration for this place of service
		long nFacilityFeeBillType = 1;
		long nFacilityTimePromptType = 1;

		// (j.jones 2007-10-15 14:14) - PLID 27757 - converted this data structure to be per service code and place of service
		// (don't worry right now if no setup exists)		
		_RecordsetPtr rs = CreateParamRecordset("SELECT FacilityFeeBillType, FacilityTimePromptType FROM FacilityFeeSetupT WHERE ServiceID = {INT} AND LocationID = {INT}", nServiceID, nPlaceOfServiceID);
		if(!rs->eof) {
			nFacilityFeeBillType = AdoFldLong(rs, "FacilityFeeBillType",1);
			nFacilityTimePromptType = AdoFldLong(rs, "FacilityTimePromptType",1);			
		}
		rs->Close();

		long nCurFacilityMinutes = 0;		

		if(nFacilityFeeBillType != 1) { // if a flat fee, we don't need to prompt for minutes			

			//prompt for minutes

			if(nFacilityTimePromptType == 1) {
				//prompt for begin and end times

				// (j.jones 2010-01-18 13:56) - PLID 36913 - if bForceTimePrompt is TRUE,
				// we will prompt no matter what defaults we have
				if(!bForceTimePrompt && strStartTime != "" && strEndTime != "" && nFacilityMinutes != 0) {
					//we already have the times, so do not prompt
					nCurFacilityMinutes = nFacilityMinutes;
				}
				else {

					BOOL bDone = FALSE;
					while(!bDone) {
						CAnesthesiaTimePromptDlg dlg(NULL);

						// (j.jones 2010-01-18 13:56) - PLID 36913 - pre-fill whatever defaults we may already have
						if(strStartTime != "") {
							dlg.m_strStartTime = strStartTime;
						}
						if(strEndTime != "") {
							dlg.m_strEndTime = strEndTime;
						}

						dlg.m_nServiceID = nServiceID;
						// (j.jones 2010-11-22 17:23) - PLID 39602 - this is now an enum
						dlg.m_eTimePromptType = tptFacilityFee;
						if(dlg.DoModal() == IDCANCEL) {
							if(IDYES == MessageBox(GetActiveWindow(),"You must enter facility start and end times in order to add this facility charge.\n"
								"If you cancel, the charge will not be added.\n\n"
								"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return FALSE;
							}
						}
						else bDone = TRUE;

						nCurFacilityMinutes = dlg.m_nMinutes;
						strStartTime = dlg.m_strStartTime;
						strEndTime = dlg.m_strEndTime;
					}
				}
			}
			else if(nFacilityTimePromptType == 2) {
				//prompt for minutes

				// (j.jones 2010-01-18 13:56) - PLID 36913 - if bForceTimePrompt is TRUE,
				// we will prompt no matter what defaults we have
				if(!bForceTimePrompt && nFacilityMinutes != 0) {
					//we already have the minutes, so do not prompt
					nCurFacilityMinutes = nFacilityMinutes;
				}
				else {

					BOOL bDone = FALSE;

					CString strPrompt = "Facility Minutes";

					_RecordsetPtr rs = CreateParamRecordset("SELECT Code + ' - ' + Name AS Description FROM ServiceT "
						"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE CPTCodeT.ID = {INT}",nServiceID);
					if(!rs->eof) {
						CString strDesc = AdoFldString(rs, "Description","");
						strPrompt.Format("Enter the facility fee minutes for code:\r\n%s",strDesc);
					}
					rs->Close();

					while(!bDone) {
						CString strMinutes = "0";

						// (j.jones 2010-01-18 13:56) - PLID 36913 - pre-fill a default if we have one
						if(nFacilityMinutes != 0) {
							strMinutes = AsString(nFacilityMinutes);
						}

						if (InputBox(NULL, strPrompt, strMinutes, "", false, false, "Cancel", TRUE) != IDOK) {
							if(IDYES == MessageBox(GetActiveWindow(),"You must enter the facility fee minutes in order to add this facility charge.\n"
								"If you cancel, the charge will not be added.\n\n"
								"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return FALSE;
							}
						}
						else {

							if(strMinutes == "0") {
								AfxMessageBox("You must enter an amount between 1 and 1440 for the facility minutes.");
								continue;
							}

							if(strMinutes.GetLength() > 6) {
								AfxMessageBox("You have entered an invalid amount for the facility minutes.");
								continue;
							}

							if(atoi(strMinutes) > 1440) {
								AfxMessageBox("You may not enter more than 1440 minutes.");
								continue;
							}
							
							bDone = TRUE;
						}

						nCurFacilityMinutes = atoi(strMinutes);
						strStartTime = "";
						strEndTime = "";
					}
				}
			}
		}

		if(nFacilityMinutes == 0 || nCurFacilityMinutes != 0)
			nFacilityMinutes = nCurFacilityMinutes;

		CalcFacilityFee(nServiceID, cyFacilityUnitCost, nCurFacilityMinutes, nPlaceOfServiceID);
	}
	else {
		cyFacilityUnitCost = COleCurrency(0,0);
	}
	rsCheck->Close();

	return TRUE;
}

void CalcFacilityFee(long nServiceID, COleCurrency &cyFacilityUnitCost, long nTotalFacilityMinutes, long nPlaceOfServiceID)
{
	//now actually calculate the facility fee

	// (j.jones 2007-10-15 14:14) - PLID 27757 - converted this data structure to be per service code and place of service

	//load up all the values at once - if no record exists, create a new one and load its defaults
	_RecordsetPtr rsFacFeeSetup = CreateParamRecordset("SELECT FacilityFeeSetupT.ID, FacilityFeeBillType, FacilityFlatFee, "
		"FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, FacilityAddlIncrement, "
		"FacilityFeeTableSearchType, FacilityTimePromptType, "
		"FacilityPayTo, FacilityOutsideFee FROM FacilityFeeSetupT "
		"INNER JOIN LocationsT ON FacilityFeeSetupT.LocationID = LocationsT.ID "
		"WHERE FacilityFeeSetupT.ServiceID = {INT} AND FacilityFeeSetupT.LocationID = {INT}", nServiceID, nPlaceOfServiceID);

	if(rsFacFeeSetup->eof) {
		//if no records exist for this CPT/POS, create a new record, and pull the default values
		rsFacFeeSetup->Close();
		rsFacFeeSetup = CreateParamRecordset("SET NOCOUNT ON\r\n"
			"INSERT INTO FacilityFeeSetupT (ServiceID, LocationID) VALUES ({INT}, {INT})\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT FacilityFeeSetupT.ID, FacilityFeeBillType, FacilityFlatFee, "
			"FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, FacilityAddlIncrement, "
			"FacilityFeeTableSearchType, FacilityTimePromptType, "
			"FacilityPayTo, FacilityOutsideFee FROM FacilityFeeSetupT "
			"INNER JOIN LocationsT ON FacilityFeeSetupT.LocationID = LocationsT.ID "
			"WHERE FacilityFeeSetupT.ID = Convert(int, SCOPE_IDENTITY())",nServiceID, nPlaceOfServiceID);
	}

	if(rsFacFeeSetup->eof) {
		ThrowNxException("CalcFacilityFee could not create a new setup record!");
	}

	long nFacilityFeeSetupID = AdoFldLong(rsFacFeeSetup, "ID");

	//load up the configuration for this place of service
	long nFacilityFeeBillType = AdoFldLong(rsFacFeeSetup, "FacilityFeeBillType",1);

	long nFacilityMinutes = nTotalFacilityMinutes;		

	if(nFacilityFeeBillType == 1) {	//flat fee
		//just return the flat fee now since we don't need to do any calculations
		cyFacilityUnitCost = AdoFldCurrency(rsFacFeeSetup, "FacilityFlatFee", COleCurrency(0,0));
		return;
	}
	else if(nFacilityFeeBillType == 2) { //incremental fee
		
		//increments are 1 - hour, 2 - half hour, 3 - quarter hour
		COleCurrency cyFacilityIncrementalBaseFee = AdoFldCurrency(rsFacFeeSetup, "FacilityIncrementalBaseFee",COleCurrency(0,0));
		long nFacilityBaseIncrement = AdoFldLong(rsFacFeeSetup, "FacilityBaseIncrement",1);
		COleCurrency cyFacilityIncrementalAddlFee = AdoFldCurrency(rsFacFeeSetup, "FacilityIncrementalAddlFee",COleCurrency(0,0));
		long nFacilityAddlIncrement = AdoFldLong(rsFacFeeSetup, "FacilityAddlIncrement",1);				
		
		if(nFacilityBaseIncrement == 1) { //hour
			if(nFacilityMinutes <= 60) {
				//if under an hour, give the 1 hour rate, and set remaining time to zero
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes = 0;
			}
			else {
				//otherwise, still give the hour rate, and reduce remaining time by 60 minutes
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes -= 60;
			}
		}
		else if(nFacilityBaseIncrement == 2) { //half-hour
			if(nFacilityMinutes <= 30) {
				//if under a half-hour, give the half-hour rate, and set remaining time to zero
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes = 0;
			}
			else {
				//otherwise, still give the half-hour rate, and reduce remaining time by 30 minutes
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes -= 30;
			}
		}
		else if(nFacilityBaseIncrement == 3) { //quarter-hour
			if(nFacilityMinutes <= 15) {
				//if under a quarter-hour, give the quarter-hour rate, and set remaining time to zero
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes = 0;
			}
			else {
				//otherwise, still give the quarter-hour rate, and reduce remaining time by 15 minutes
				cyFacilityUnitCost = cyFacilityIncrementalBaseFee;
				nFacilityMinutes -= 15;
			}
		}

		//if there is time left
		while(nFacilityMinutes > 0) {
			if(nFacilityAddlIncrement == 1) { //hour
				if(nFacilityMinutes <= 60) {
					//if under an hour, give the 1 hour rate, and set remaining time to zero
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes = 0;
				}
				else {
					//otherwise, still give the hour rate, and reduce remaining time by 60 minutes
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes -= 60;
				}
			}
			else if(nFacilityAddlIncrement == 2) { //half-hour
				if(nFacilityMinutes <= 30) {
					//if under a half-hour, give the half-hour rate, and set remaining time to zero
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes = 0;
				}
				else {
					//otherwise, still give the half-hour rate, and reduce remaining time by 30 minutes
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes -= 30;
				}
			}
			else if(nFacilityAddlIncrement == 3) { //quarter-hour
				if(nFacilityMinutes <= 15) {
					//if under a quarter-hour, give the quarter-hour rate, and set remaining time to zero
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes = 0;
				}
				else {
					//otherwise, still give the quarter-hour rate, and reduce remaining time by 15 minutes
					cyFacilityUnitCost += cyFacilityIncrementalAddlFee;
					nFacilityMinutes -= 15;
				}
			}
		}
	}
	else if(nFacilityFeeBillType == 3) { //scheduled fee			

		//1 - lesser time, 2 - greater time
		long nFacilityFeeTableSearchType = AdoFldLong(rsFacFeeSetup, "FacilityFeeTableSearchType",2);

		//now grab the right fee from LocationFacilityFeesT
		CString str;
		str.Format("SELECT Fee FROM LocationFacilityFeesT "
			"WHERE FacilityFeeSetupID = {INT} AND ((Hours * 60) + Minutes) %s {INT} ORDER BY Hours %s, Minutes %s", 
			nFacilityFeeTableSearchType == 2 ? ">=" : "<=", nFacilityFeeTableSearchType == 2 ? "ASC" : "DESC",
			nFacilityFeeTableSearchType == 2 ? "ASC" : "DESC");
		_RecordsetPtr rsFees = CreateParamRecordset(str, nFacilityFeeSetupID, nFacilityMinutes);
		if(!rsFees->eof) {
			cyFacilityUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
		}
		else {
			//try not filtering out the ones less than or greater than the value
			rsFees->Close();

			// (j.jones 2009-10-07 09:09) - PLID 36426 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM LocationFacilityFeesT "
				"WHERE (Hours * 60) + Minutes > {INT} AND FacilityFeeSetupID = {INT}",nFacilityMinutes,nFacilityFeeSetupID);
			if(rs->eof) {
				//our time entered is greater than anything in the list
				rsFees = CreateParamRecordset("SELECT TOP 1 Fee FROM LocationFacilityFeesT "
					"WHERE FacilityFeeSetupID = {INT} "
					"ORDER BY Hours DESC, Minutes DESC ", nFacilityFeeSetupID);
				if(!rsFees->eof) {
					cyFacilityUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
					return;
				}
				rsFees->Close();
			}

			// (j.jones 2009-10-07 09:09) - PLID 36426 - parameterized
			rs = CreateParamRecordset("SELECT ID FROM LocationFacilityFeesT "
				"WHERE (Hours * 60) + Minutes < {INT} AND FacilityFeeSetupID = {INT}",nFacilityMinutes,nFacilityFeeSetupID);
			if(rs->eof) {
				//our time entered is less than anything in the list
				rsFees = CreateParamRecordset("SELECT TOP 1 Fee FROM LocationFacilityFeesT "
					"WHERE FacilityFeeSetupID = {INT} "
					"ORDER BY Hours ASC, Minutes ASC ", nFacilityFeeSetupID);
				if(!rsFees->eof) {
					cyFacilityUnitCost = AdoFldCurrency(rsFees, "Fee",COleCurrency(0,0));
					return;
				}
				rsFees->Close();
			}

			//we shouldn't be able to get here unless there is nothing set up in the fee
			//table, which is now impossible to do
			cyFacilityUnitCost = COleCurrency(0,0);
			return;
		}
	}
}

// (j.jones 2010-11-22 16:18) - PLID 39602 - added support for billing Assisting Codes, which work similarly to anesthesia & facility codes
// (j.jones 2011-10-31 16:43) - PLID 41558 - added minutes & start time as parameters
BOOL CheckAssistingCode(IN long nServiceID, OUT BOOL &bAssistingCode, OUT COleCurrency &cyAssistingCodeUnitCost, long &nMinutes, CString &strStartTime, CString &strEndTime)
{
	//this is OHIP only, so if OHIP is disabled, act like it is not an assisting code

	if(!UseOHIP()) {
		bAssistingCode = FALSE;
		cyAssistingCodeUnitCost = COleCurrency(0,0);
		//TRUE means we still add this charge
		return TRUE;
	}

	//is this an assisting code?
	_RecordsetPtr rs = CreateParamRecordset("SELECT CPTCodeT.Code + ' - ' + ServiceT.Name AS Description, "
		"CPTCodeT.AssistingBaseUnits "
		"FROM ServiceT "
		"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = {INT} AND AssistingCode = 1", nServiceID);
	if(!rs->eof) {
		//we do indeed to make the assisting code calculation
		bAssistingCode = TRUE;

		long nTimePrompt = GetRemotePropertyInt("OHIPAssistingCode_TimePrompt", 0, 0, "<None>", true);
		//0 - times, 1 - minutes

		//prompt for the time spent
		if(nTimePrompt == 0) {
			//prompt for begin and end times

			// (j.jones 2011-10-31 16:45) - PLID 41558 - if valid times were given to us, don't prompt
			if(strStartTime == "" || strEndTime == "") {

				BOOL bDone = FALSE;
				while(!bDone) {
					CAnesthesiaTimePromptDlg dlg(NULL);
					dlg.m_eTimePromptType = tptAssistingCode;
					dlg.m_nServiceID = nServiceID;
					if(dlg.DoModal() == IDCANCEL) {
						if(IDYES == MessageBox(GetActiveWindow(),"You must enter assisting start and end times in order to add this assisting code charge.\n"
							"If you cancel, the charge will not be added.\n\n"
							"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							return FALSE;
						}
					}
					else bDone = TRUE;

					nMinutes = dlg.m_nMinutes;
					strStartTime = dlg.m_strStartTime;
					strEndTime = dlg.m_strEndTime;
				}
			}
		}
		else if(nTimePrompt == 1) {

			//prompt for minutes

			// (j.jones 2011-10-31 16:45) - PLID 41558 - if >0 minutes were given to us, don't prompt
			if(nMinutes <= 0) {
				
				BOOL bDone = FALSE;

				CString strPrompt = "Assisting Minutes";

				CString strDesc = AdoFldString(rs, "Description","");
				strPrompt.Format("Enter the assisting minutes for code:\r\n%s",strDesc);

				while(!bDone) {
					CString strMinutes = "0";

					if (InputBox(NULL, strPrompt, strMinutes, "", false, false, "Cancel", TRUE) != IDOK) {
						if(IDYES == MessageBox(GetActiveWindow(),"You must enter the assisting minutes in order to add this assisting code charge.\n"
							"If you cancel, the charge will not be added.\n\n"
							"Do you still wish to cancel adding this charge?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							return FALSE;
						}
					}
					else {

						if(strMinutes == "0") {
							AfxMessageBox("You must enter an amount between 1 and 1440 for the assisting minutes.");
							continue;
						}

						if(strMinutes.GetLength() > 6) {
							AfxMessageBox("You have entered an invalid amount for the assisting minutes.");
							continue;
						}

						if(atoi(strMinutes) > 1440) {
							AfxMessageBox("You may not enter more than 1440 minutes.");
							continue;
						}
						
						bDone = TRUE;
					}

					nMinutes = atoi(strMinutes);
				}
			}
		}

		//the base units are added into the time units
		long nBaseUnits = AdoFldLong(rs, "AssistingBaseUnits", 0);

		CalcAssistingCode(nServiceID, nMinutes, nBaseUnits, cyAssistingCodeUnitCost);
	}
	else {
		bAssistingCode = FALSE;
		cyAssistingCodeUnitCost = COleCurrency(0,0);
	}
	rs->Close();

	return TRUE;
}

// (j.jones 2010-11-22 16:18) - PLID 39602 - added support for billing Assisting Codes, which work similarly to anesthesia & facility codes
void CalcAssistingCode(IN long nServiceID, IN long nTotalAssistingMinutes, IN long nBaseUnits, OUT COleCurrency &cyAssistingCodeUnitCost)
{
	//now actually calculate the assisting code's value

	//load all the records from AssistingCodesSetupT that have a
	//StartAfterMinute that is less than (NOT equal to) our total minutes,
	//for example if StartAfterMinute is 60, and our total minutes is 60,
	//we do not calculate the 60 minute unit, but we would if our total was 61
	_RecordsetPtr rsSetup = CreateParamRecordset("SELECT StartAfterMinute, Units "
		"FROM AssistingCodesSetupT "
		"WHERE StartAfterMinute < {INT} "
		"ORDER BY StartAfterMinute DESC", nTotalAssistingMinutes);

	long nTotalUnits = 0;
	long nMinutesRemaining = nTotalAssistingMinutes;

	while(!rsSetup->eof) {

		long nStartAfterMinute = VarLong(rsSetup->Fields->Item["StartAfterMinute"]->Value);
		long nUnits = VarLong(rsSetup->Fields->Item["Units"]->Value);

		//now, how many minutes were they at this level?
		long nMinutesAtThisLevel = nMinutesRemaining - nStartAfterMinute;

		//reduce our remaining minutes
		nMinutesRemaining -= nMinutesAtThisLevel;

		//we calculate units by 15 minute intervals or any part thereof,
		//so how many 15 minute intervals were there?
		long nIntervalsAtThisLevel = nMinutesAtThisLevel / 15;
		
		//nIntervalsAtThisLevel is now only how many full 15 minute intervals
		//existed, we still need to account for any partial intervals
		if((nMinutesAtThisLevel - (nIntervalsAtThisLevel * 15)) > 0) {
			
			//there was a partial interval, increment our interval count by 1
			nIntervalsAtThisLevel++;
		}

		//multiply our intervals at this level by units at this level,
		//and add to our running total
		nTotalUnits += (nIntervalsAtThisLevel * nUnits);

		rsSetup->MoveNext();
	}
	rsSetup->Close();

	//get the global fee per unit
	CString strUnitFee = GetRemotePropertyText("OHIPAssistingCode_BasePrice", "$11.58", 0, "<None>", true);
	COleCurrency cyUnitFee;
	if(!cyUnitFee.ParseCurrency(strUnitFee) || cyUnitFee.GetStatus() == COleCurrency::invalid) {
		//should not be possible, but set to zero
		cyUnitFee = COleCurrency(0,0);
	}

	//add our base units to the total
	nTotalUnits += nBaseUnits;

	//multiply the unit fee by total units
	cyAssistingCodeUnitCost = cyUnitFee * nTotalUnits;
}

BOOL CheckProcedureDiscount(long nServiceID, long nPatientID, long &nPercentOff, COleDateTime dtBillDate, long nBillID)
{
	//first check that the ServiceID exists in a procedure that has recurring discounts
	long nProcedureID = -1;
	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ProcedureID FROM ServiceT WHERE ID = {INT} AND ProcedureID Is Not Null",nServiceID);	
	if(!rs->eof) {
		nProcedureID = AdoFldLong(rs, "ProcedureID",-1);
	}
	rs->Close();

	if(nProcedureID == -1)
		return FALSE;

	// (j.jones 2005-11-07 12:01) - PLID 16034 - if it is a detail procedure, use the master procedure discount configuration
	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	rs = CreateParamRecordset("SELECT ID FROM ProcedureT WHERE ID IN (SELECT MasterProcedureID FROM ProcedureT WHERE ID = {INT})", nProcedureID);
	if(!rs->eof) {
		//this is indeed a detail procedure, so instead find the discount for the master
		nProcedureID = AdoFldLong(rs, "ID");
	}
	rs->Close();

	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	rs = CreateParamRecordset("SELECT ProcedureID FROM ProcedureDiscountsT WHERE ProcedureID = {INT}",nProcedureID);
	if(rs->eof) {
		return FALSE;
	}
	rs->Close();

	//if we get here, we know the service is linked to a procedure, and that procedure has discounts set up,
	//so now we must calculate which occurrence of a bill this is

	long nOccurrence = 0;

	if(nBillID == -1) {
		//if a new bill, simply find out how many bills have been made with this procedure code
		
		// (j.jones 2005-11-07 12:06) - PLID 16034 - ...or the detail procedure's code, as nProcedureID
		// is now the master procedure's code

		// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
		rs = CreateParamRecordset("SELECT Count(ID) AS CountOfBills FROM BillsT "
			"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.PatientID = {INT} AND BillsT.ID IN "
			"(SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE LineItemT.Deleted = 0 AND ServiceID IN (SELECT ID FROM ServiceT WHERE "
			"(ProcedureID = {INT} OR ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID = {INT}))))",
			nPatientID, nProcedureID, nProcedureID);

		if(!rs->eof) {
			nOccurrence = AdoFldLong(rs, "CountOfBills",0);
		}
		rs->Close();

		//now increment the occurrence to account for this new bill
		nOccurrence++;
	}
	else {
		//if an existing bill....?????
	}

	//now that we have an Occurrence index, find out of a specified PercentOff exists

	//if no special PercentOff exists for this exact occurrence,
	//we need to see if one is still in effect from an earlier occurrence
	//(example: if there is a 10% discount at the 5th occurrence, and a 20% discount
	//at the 10th occurrence, that 10% will still apply on occurrences 6 through 9,
	//even though no record exists for those exact occurrence indices)
	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	rs = CreateParamRecordset("SELECT TOP 1 PercentOff FROM ProcedureDiscountsT "
		"WHERE ProcedureID = {INT} AND Occurrence <= {INT} "
		"ORDER BY Occurrence DESC",nProcedureID, nOccurrence);

	if(!rs->eof) {
		nPercentOff = AdoFldLong(rs, "PercentOff",0);
		return TRUE;
	}
	rs->Close();

	return FALSE;
}

void CalculateFinanceCharges()
{
	//process the finance charges
	try {

		CString strWarn;
		strWarn = "This action will calculate finance charges for all outstanding bills that have not already had the appropriate finance charges added.\n\n";
		
		if(GetRemotePropertyInt("FCChooseInvididualPatients",0,0,"<None>",true) == 1) {
			// (j.jones 2009-06-11 16:00) - PLID 34577 - we now prompt AFTER we detect which
			// patients need finance charges, but before applying finance charges, so just
			// change the warning if this setting is enabled
			strWarn += "You will be given the ability to choose which patient accounts are affected prior to finance charges being created.\n\n";
		}

		strWarn += "Are you sure you wish to calculate finance charges now?";

		if(IDNO == MessageBox(GetActiveWindow(), strWarn,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CFinanceChargesDlg dlg(NULL);
		dlg.DoModal();
		return;

	}NxCatchAll("Error calculating finance charges.");
}

void SwapInsuranceCompanies(long nBillID, long nSrcInsPartyID, long nDstInsPartyID, BOOL bShowWarnings /*= TRUE*/)
{

	// (j.jones 2009-03-11 11:23) - PLID 32864 - added the old insuredparty IDs, old insco names, and
	// new insco names, for auditing
	_RecordsetPtr rsBill = CreateParamRecordset("SELECT BillsT.ID AS BillID, BillsT.PatientID, "
		"PriInsuredPartyT.PersonID AS PriInsuredPartyID, SecInsuredPartyT.PersonID AS SecInsuredPartyID, "
		"PriInsuredPartyT.Name AS PriInsCoName, SecInsuredPartyT.Name AS SecInsCoName, "
		"BillsT.InsuredPartyID AS BillInsuredPartyID, BillsT.OthrInsuredPartyID AS BillOthrInsuredPartyID, "
		"InsuranceCoT.Name As BillInsCoName, OthrInsuranceCoT.Name AS BillOthrInsCoName "
		"FROM BillsT "
		"LEFT JOIN (SELECT InsuredPartyT.PersonID, InsuredPartyT.PatientID, InsuredPartyT.RespTypeID, "
		"	InsuranceCoT.Name "
		"	FROM InsuredPartyT "
		"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"	WHERE InsuredPartyT.RespTypeID = 1) "	//1 is always pri
		"	AS PriInsuredPartyT ON BillsT.PatientID = PriInsuredPartyT.PatientID "
		"LEFT JOIN (SELECT InsuredPartyT.PersonID, InsuredPartyT.PatientID, InsuredPartyT.RespTypeID, "
		"	InsuranceCoT.Name "
		"	FROM InsuredPartyT "
		"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"	WHERE InsuredPartyT.RespTypeID = 2) "	//2 is always sec
		"	AS SecInsuredPartyT ON BillsT.PatientID = SecInsuredPartyT.PatientID "
		"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
		"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
		"WHERE BillsT.ID = {INT}", nBillID);

	if(!rsBill->eof) {
		long BillID = AdoFldLong(rsBill, "BillID",-1);		
		long PriInsuredPartyID = AdoFldLong(rsBill, "PriInsuredPartyID",-2);
		long SecInsuredPartyID = AdoFldLong(rsBill, "SecInsuredPartyID",-2);

		// (j.jones 2009-03-11 11:31) - PLID 32864 - track these fields for auditing
		long nPatientID = AdoFldLong(rsBill, "PatientID",-1);
		long nOldInsuredPartyID = AdoFldLong(rsBill, "BillInsuredPartyID",-1);
		long nOldOthrInsuredPartyID = AdoFldLong(rsBill, "BillOthrInsuredPartyID",-1);
		CString strOldBillInsCoName = AdoFldString(rsBill, "BillInsCoName", "<None Selected>");
		CString strOldBillOthrInsCoName = AdoFldString(rsBill, "BillOthrInsCoName", "<None Selected>");
		CString strPriInsCoName = AdoFldString(rsBill, "PriInsCoName", "<None Selected>");
		CString strSecInsCoName = AdoFldString(rsBill, "SecInsCoName", "<None Selected>");

		long NewInsuredPartyID = PriInsuredPartyID;
		long NewOthrInsuredPartyID = SecInsuredPartyID;
		CString strNewInsCoName = strPriInsCoName;
		CString strNewOthrInsCoName = strSecInsCoName;

		//set the bill's primary ins co to be the destination ins co from the switch
		//set the bill's secondary ins co to be the source ins co from the switch

		//if the dest is patient, do nothing
		//if the source is patient, make the secondary ins co be the opposite of the primary

		if(nDstInsPartyID == PriInsuredPartyID) {	//if(DstType == 1) {
			//we switched to primary, so set the new pri ins co to be the primary,
			//and sec ins co to be secondary
			NewInsuredPartyID = PriInsuredPartyID;
			NewOthrInsuredPartyID = SecInsuredPartyID;

			// (j.jones 2009-03-11 11:36) - PLID 32864 - assign the insco names too
			strNewInsCoName = strPriInsCoName;
			strNewOthrInsCoName = strSecInsCoName;
		}
		else if(nDstInsPartyID == SecInsuredPartyID) {		//if(DstType == 2) {
			//we switched to secondary, so set the new pri ins co to be the secondary,
			//and sec ins co to be primary
			NewInsuredPartyID = SecInsuredPartyID;
			NewOthrInsuredPartyID = PriInsuredPartyID;

			// (j.jones 2009-03-11 11:36) - PLID 32864 - assign the insco names too
			strNewInsCoName = strSecInsCoName;
			strNewOthrInsCoName = strPriInsCoName;
		}
		else {
			//DRT 2/16/2004 - PLID 8814 - If they have chosen to swap and are not using pri/sec, then
			//	we want to set our new ins co to the Primary (on the bill), and whatever was primary
			//	on the bill becomes secondary (on the bill).  It is not possible to reach this point 
			//	and be using patient responsibility.
			NewInsuredPartyID = nDstInsPartyID;
			NewOthrInsuredPartyID = PriInsuredPartyID;

			// (j.jones 2009-03-11 11:36) - PLID 32864 - unfortunately we need another recordset
			// to get this InsCoName
			CString strDstInsCoName = "<None Selected>";
			if(nDstInsPartyID != -1) {
				_RecordsetPtr rsIns = CreateParamRecordset("SELECT InsuranceCoT.Name FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nDstInsPartyID);
				if(!rsIns->eof) {
					strDstInsCoName = AdoFldString(rsIns, "Name", "<None Selected>");
				}
				rsIns->Close();
			}
			strNewInsCoName = strDstInsCoName;

			strNewOthrInsCoName = strPriInsCoName;
		}

		if(nSrcInsPartyID == PriInsuredPartyID) {		//if(SrcInsType == 1)
			//if the source was primary, make the sec ins co primary
			NewOthrInsuredPartyID = PriInsuredPartyID;

			// (j.jones 2009-03-11 11:36) - PLID 32864 - assign the insco names too
			strNewOthrInsCoName = strPriInsCoName;
		}
		else if(nSrcInsPartyID == SecInsuredPartyID) {		//if(SrcInsType == 2)
			//if the source was secondary, make the sec ins co secondary
			NewOthrInsuredPartyID = SecInsuredPartyID;

			// (j.jones 2009-03-11 11:36) - PLID 32864 - assign the insco names too
			strNewOthrInsCoName = strSecInsCoName;
		}

		//danger, will robinson!
		//it is possible that either ID is -2, and that's not allowed in the data
		//JMJ - 11/4/2003 - actually, at the moment it should be impossible to get to this block of code
		//without a primary and secondary insured party, thus they should never be -2
		if(NewInsuredPartyID == -2) {
			NewInsuredPartyID = -1;
			strNewInsCoName = "<None Selected>";
		}
		else if(NewOthrInsuredPartyID == -2) {
			NewOthrInsuredPartyID = -1;
			strNewOthrInsCoName = "<None Selected>";
		}

		//JMJ - 12/11/2003 - Let's check and see if we're actually changing anything.
		//If not, inform the user.
		// (j.jones 2009-10-07 09:09) - PLID 36426 - parameterized
		// (j.jones 2012-08-20 17:16) - PLID 50148 - finally got rid of this prompt, as it was quite pointless
		/*
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM BillsT "
			"WHERE ID = {INT} AND InsuredPartyID = {INT} AND OthrInsuredPartyID = {INT}",
			BillID,NewInsuredPartyID,NewOthrInsuredPartyID);
		if(!rs->eof) {
			if(bShowWarnings) {
				AfxMessageBox("You chose to swap the insurance company placement on the bill, but they are already in their intended positions.\n"
					"No change will be made.");
			}
		}
		rs->Close();
		*/

		// (j.jones 2009-09-21 11:37) - PLID 35564 - -1 is no longer allowed for insured party IDs, so set to NULL
		_variant_t varNewInsuredPartyID = g_cvarNull;
		_variant_t varNewOthrInsuredPartyID = g_cvarNull;
		if(NewInsuredPartyID != -1) {
			varNewInsuredPartyID = NewInsuredPartyID;
		}
		if(NewOthrInsuredPartyID != -1) {
			varNewOthrInsuredPartyID = NewOthrInsuredPartyID;
		}

		ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {VT_I4}, OthrInsuredPartyID = {VT_I4} WHERE ID = {INT}",varNewInsuredPartyID,varNewOthrInsuredPartyID,BillID);

		// (s.tullis 2016-02-24 13:47) - PLID 68319 - since we shifted resp 
		// update the bill claim form to reflect this new insurance selection 
		if (NewInsuredPartyID != -1) {
			UpdateBillClaimForm(BillID);
		}

		// (j.jones 2009-03-11 11:44) - PLID 32864 - audit InsuredPartyID and OthrInsuredPartyID
		long nAuditID = -1;
		if(nOldInsuredPartyID != NewInsuredPartyID) {

			if(nOldInsuredPartyID == -1) {
				strOldBillInsCoName = "<None Selected>";
			}

			if(NewInsuredPartyID == -1) {
				strNewInsCoName = "<None Selected>";
			}

			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiBillInsurancePlan, BillID, strOldBillInsCoName, strNewInsCoName, aepMedium, aetChanged);
		}

		if(nOldOthrInsuredPartyID != NewOthrInsuredPartyID) {

			if(nOldOthrInsuredPartyID == -1) {
				strOldBillOthrInsCoName = "<None Selected>";
			}

			if(NewOthrInsuredPartyID == -1) {
				strNewOthrInsCoName = "<None Selected>";
			}

			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiBillOtherInsurancePlan, BillID, strOldBillOthrInsCoName, strNewOthrInsCoName, aepMedium, aetChanged);
		}

		//now that we switched the insured party ID, we should switch the POS Designation,
		//only if there is a specific POS designation for that company

		long nPOS = -1;
		_RecordsetPtr rs = CreateParamRecordset("SELECT POSID FROM POSLocationLinkT INNER JOIN InsuranceCoT ON POSLocationLinkT.HCFASetupGroupID = InsuranceCoT.HCFASetupGroupID "
			"WHERE LocationID = (SELECT Location FROM BillsT WHERE ID = {INT}) AND InsuranceCoT.PersonID = {INT}",
			BillID,	GetInsuranceCoID(NewInsuredPartyID));
		if(!rs->eof) {
			nPOS = AdoFldLong(rs, "POSID", -1);
			if(nPOS != -1) {
				ExecuteParamSql("UPDATE ChargesT SET ServiceLocationID = {INT} WHERE BillID = {INT}", nPOS,BillID);
			}
		}
		rs->Close();
	}
	rsBill->Close();
}

// (j.jones 2013-08-01 14:19) - PLID 53317 - added ability to undo finance charges
void UndoFinanceCharges()
{
	long nAuditTransactionID = -1;

	try {

		if(!CheckCurrentUserPermissions(bioFinanceCharges, sptDelete)) {
			return;
		}

		//prepare our columns
		CStringArray aryColumns;
		CStringArray aryColumnHeaders;
		CSimpleArray<short> arySortOrder;
		CSimpleArray<bool> arySortAscending;

		//our ID is the FinanceChargeHistoryT.ID, it does not sort
		aryColumns.Add("FinanceChargeHistoryT.ID");
		aryColumnHeaders.Add("ID");
		arySortOrder.Add(-1);
		arySortAscending.Add(true);	//ascending (meaningless on unsorted columns)

		//sort by FinanceChargeHistoryT.InputDate descending
		aryColumns.Add("FinanceChargeHistoryT.InputDate");
		aryColumnHeaders.Add("Created Date");
		arySortOrder.Add(0);
		arySortAscending.Add(false);	//false means descending

		//show the total finance charges
		aryColumns.Add("FCQ.TotalCharges");
		aryColumnHeaders.Add("Total Finance Charge Value");
		arySortOrder.Add(-1);
		arySortAscending.Add(true);

		//show the count of finance charges
		aryColumns.Add("FCQ.CountCharges");		
		aryColumnHeaders.Add("Count of Finance Charges");
		arySortOrder.Add(-1);
		arySortAscending.Add(true);
		
		// Open the dialog
		CSingleSelectMultiColumnDlg dlg;
		if(dlg.Open("FinanceChargeHistoryT "
			"LEFT JOIN ("
			"	SELECT FinanceChargeHistoryID, Sum(Amount) AS TotalCharges, Count(*) AS CountCharges "
			"	FROM FinanceChargesT "
			"	GROUP BY FinanceChargeHistoryID "
			") AS FCQ ON FinanceChargeHistoryT.ID = FCQ.FinanceChargeHistoryID ",	/*From*/
		  "",																		/*Where*/
		  aryColumns,																/*Select*/
		  aryColumnHeaders,															/*Column Names*/
		  arySortOrder,																/*Sort Order*/
		  arySortAscending,															/*Sort Ascending/Descending */
		  "[1] - [2]",																/*Display Columns*/
		  "Please select a finance charge batch below to permanently delete.",		/*Description*/
		  "Select a Finance Charge Batch to Undo"									/*Title Bar Header*/
		  ) == IDOK)
		{
			CVariantArray varySelectedValues;
			dlg.GetSelectedValues(varySelectedValues);

			if(varySelectedValues.GetSize() == 0){
				//they didn't select anything, don't warn, just ignore
				return;
			}

			if (IDNO == MessageBox(GetActiveWindow(), "Removing a batch of finance charges will delete all charges that were created in this batch. "
				"This action is PERMANENT and CANNOT be undone.\n\n"
				"Are you sure you wish to undo this batch of finance charges?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			long nBatchIDToUndo = VarLong(varySelectedValues.GetAt(0));

			// this will check against financial closes
			if(!CanChangeHistoricFinancial("FinanceChargeHistory", nBatchIDToUndo, bioFinanceCharges, sptDelete)) {
				return;
			}

			CWaitCursor pWait;

			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			nAuditTransactionID = BeginAuditTransaction();

			CString strOldFinanceChargeBatchInformation = "";

			//if we get here, we can undo these finance charges
			_RecordsetPtr rs = CreateParamRecordset("SELECT InputDate, TotalCharges, CountCharges "
				"FROM FinanceChargeHistoryT "
				"LEFT JOIN ("
				"	SELECT FinanceChargeHistoryID, Sum(Amount) AS TotalCharges, Count(*) AS CountCharges "
				"	FROM FinanceChargesT "
				"	GROUP BY FinanceChargeHistoryID "
				") AS FCQ ON FinanceChargeHistoryT.ID = FCQ.FinanceChargeHistoryID "
				"WHERE ID = {INT}; \r\n"
				""
				"SELECT FinanceChargesT.ID, FinanceChargesT.FinanceChargeID, FinanceChargesT.Amount, "
				"LineItemT.Deleted, LineItemT.Amount AS ChargeAmount, dbo.GetChargeTotal(LineItemT.ID) AS ChargeTotal, "
				"LineItemCorrectionsT_Orig.ID AS LineItemCorrectionID_Orig, "
				"LineItemCorrectionsT_New.ID AS LineItemCorrectionID_New, "
				"PersonT.ID AS PatientID, PersonT.FullName AS PatientName, "
				"LineItemT.Date AS ChargeDate, LineItemT.Description, "
				"Convert(bit, CASE WHEN AppliesQ.DestID Is Not Null THEN 1 ELSE 0 END) AS HasApplies "
				"FROM FinanceChargesT "
				"INNER JOIN LineItemT ON FinanceChargesT.FinanceChargeID = LineItemT.ID "
				"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
				"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_Orig ON LineItemT.ID = LineItemCorrectionsT_Orig.OriginalLineItemID "
				"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_New ON LineItemT.ID = LineItemCorrectionsT_New.NewLineItemID "
				"LEFT JOIN (SELECT DestID FROM AppliesT GROUP BY DestID) AS AppliesQ ON LineItemT.ID = AppliesQ.DestID "
				"WHERE FinanceChargesT.FinanceChargeHistoryID = {INT};",
				nBatchIDToUndo, nBatchIDToUndo);
			if(rs->eof) {
				//shouldn't be possible
				ThrowNxException("FinanceChargeHistoryT.ID of %li was not found!", nBatchIDToUndo);
			}
			else {
				COleDateTime dtInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
				COleCurrency cyTotalCharges = VarCurrency(rs->Fields->Item["TotalCharges"]->Value, COleCurrency(0,0));
				long nCountCharges = VarLong(rs->Fields->Item["CountCharges"]->Value, 0);
				//Fun fact: Totalling and totaling are both legit spellings of this word. We use totalling everywhere else in the program.
				strOldFinanceChargeBatchInformation.Format("%li finance charges totalling %s created on %s.",
					nCountCharges, FormatCurrencyForInterface(cyTotalCharges), FormatDateTimeForInterface(dtInputDate));
			}

			//now process the deletion

			rs = rs->NextRecordset(NULL);

			long nCountChargesDeleted = 0;
			long nCountChargesReduced = 0;
			COleCurrency cyTotalChargeAmtRemoved = COleCurrency(0,0);
			CMap<long, long, bool, bool> mapPatientsAffected;
			CMap<long, long, COleCurrency, COleCurrency> mapChargesToAmounts;
			CMap<long, long, COleCurrency, COleCurrency> mapChargeRespsToAmounts;
			CMap<long, long, COleCurrency, COleCurrency> mapChargeRespDetailsToAmounts;
			CMap<long, long, bool, bool> mapChargesToDeleted;
			CMap<long, long, bool, bool> mapChargeRespsToDeleted;
			CMap<long, long, bool, bool> mapChargeRespDetailsToDeleted;

			//variables for checking for errors later
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespTotalToCheck money");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespIDToCheck INT");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespInsuredPartyIDToCheck INT");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespAmountToCheck money");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespDetailTotalToCheck money");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @errorText nvarchar(max)");

			while(!rs->eof) {
				long nFinanceChargeRecordID = VarLong(rs->Fields->Item["ID"]->Value);
				long nChargeID = VarLong(rs->Fields->Item["FinanceChargeID"]->Value);
				COleCurrency cyFinanceChargeAmt = VarCurrency(rs->Fields->Item["Amount"]->Value);
				long nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
				CString strPatientName = VarString(rs->Fields->Item["PatientName"]->Value, "");
				CString strChargeDescription = VarString(rs->Fields->Item["Description"]->Value, "");

				//track if it has been flagged for deletion - we'll still delete the finance charge record if so
				bool bDeleted = false;
				mapChargesToDeleted.Lookup(nChargeID, bDeleted);

				//if the charge has already been deleted, the hard part is already done
				if(!VarBool(rs->Fields->Item["Deleted"]->Value) && !bDeleted) {

					//get the charge information
					COleCurrency cyChargeAmount = VarCurrency(rs->Fields->Item["ChargeAmount"]->Value);
					COleDateTime dtChargeDate = VarDateTime(rs->Fields->Item["ChargeDate"]->Value);

					//has this charge been corrected?
					long nCorrectionID_Orig = VarLong(rs->Fields->Item["LineItemCorrectionID_Orig"]->Value, -1);
					long nCorrectionID_New = VarLong(rs->Fields->Item["LineItemCorrectionID_New"]->Value, -1);
					if(nCorrectionID_Orig != -1 || nCorrectionID_New != -1) {
						//fail, because we refuse to auto-undo corrections
						CString strWarn;
						//use the word 'correction' even though it might have been only voided, because that is how we refer to the feature in general
						strWarn.Format("This finance charge batch can not be undone because patient %s has a %s charge from %s "
							"that has since been corrected. This correction must be manually reversed before the finance charge batch can be undone.",
							strPatientName, FormatCurrencyForInterface(cyChargeAmount), FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate));
						AfxMessageBox(strWarn);

						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
						}
						return;
					}

					//has the charge value been altered?
					{
						COleCurrency cyChargeTotal = VarCurrency(rs->Fields->Item["ChargeTotal"]->Value);
						if(cyChargeAmount != cyChargeTotal) {
							//No finance charge should be like this.
							//All finance charges we created would have quantity 1, no tax, no modifiers.
							//If the line amount and total are different, they have manually altered this charge.
							CString strWarn;
							strWarn.Format("This finance charge batch can not be undone because patient %s has a %s charge from %s "
								"that has since been manually edited. Finance charges should not be manually edited to have a quantity, tax rate, "
								"or any other change that alters its price. These changes must be manually repaired before the finance charge batch can be undone.",
								strPatientName, FormatCurrencyForInterface(cyChargeTotal), FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate));
							AfxMessageBox(strWarn);

							if (nAuditTransactionID != -1) {
								RollbackAuditTransaction(nAuditTransactionID);
							}
							return;
						}
					}

					//does this charge have any applies?
					if(VarBool(rs->Fields->Item["HasApplies"]->Value)) {
						//We do not unapply payments here, because if they paid off the finance charge
						//should they really be undoing the batch?
						//Either way, they need to manually unapply it and deal with the new credit balance as they see fit.
						CString strWarn;
						strWarn.Format("This finance charge batch can not be undone because patient %s has a %s charge from %s "
							"that has credits applied to it. These credits must be manually unapplied before the finance charge batch can be undone.",
							strPatientName, FormatCurrencyForInterface(cyChargeAmount), FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate));
						AfxMessageBox(strWarn);

						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
						}
						return;
					}

					//we're good to go, this charge can now be modified safely

					//first, reconcile the charge amount with our map, in the event we've already modified it
					COleCurrency cyTemp = COleCurrency(0,0);
					if(mapChargesToAmounts.Lookup(nChargeID, cyTemp)) {
						//we have already modified this charge, so update our charge amount to the real amount
						cyChargeAmount = cyTemp;
					}

					//does the charge need deleted, or just reduced?
					if(cyChargeAmount > cyFinanceChargeAmt) {
						//just reduce the value, don't delete the charge

						COleCurrency cyRemainingAmtToReduce = cyFinanceChargeAmt;

						//update ChargeRespDetailT and ChargeRespT, which may mean altering multiple records

						//try to reduce patient balance first
						_RecordsetPtr rsChargeResp = CreateParamRecordset("SELECT ID, Amount FROM ChargeRespT "
							"WHERE ChargeID = {INT} "
							"ORDER BY (CASE WHEN InsuredPartyID Is Null THEN 0 ELSE 1 END)", nChargeID);						
						while(!rsChargeResp->eof && cyRemainingAmtToReduce > COleCurrency(0,0)) {
							long nChargeRespID = VarLong(rsChargeResp->Fields->Item["ID"]->Value);
							COleCurrency cyChargeRespAmt = VarCurrency(rsChargeResp->Fields->Item["Amount"]->Value);

							//ignore if it has been flagged for deletion
							bDeleted = false;
							mapChargeRespsToDeleted.Lookup(nChargeRespID, bDeleted);
							if(bDeleted) {
								rsChargeResp->MoveNext();
								continue;
							}

							//reconcile the charge resp amount with our map, in the event we've already modified it
							cyTemp = COleCurrency(0,0);
							if(mapChargeRespsToAmounts.Lookup(nChargeRespID, cyTemp)) {
								//we have already modified this resp, so update our resp amount to the real amount
								cyChargeRespAmt = cyTemp;
							}

							if(cyChargeRespAmt > cyRemainingAmtToReduce) {
								//reduce this charge resp

								COleCurrency cyNewChargeRespAmt = cyChargeRespAmt - cyRemainingAmtToReduce;

								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ChargeRespT SET Amount = {OLECURRENCY} WHERE ID = {INT}", cyNewChargeRespAmt, nChargeRespID);
								//map this new amount
								mapChargeRespsToAmounts.SetAt(nChargeRespID, cyNewChargeRespAmt);

								COleCurrency cyRemainingChargeRespDetailAmtToReduce = cyRemainingAmtToReduce;
								cyRemainingAmtToReduce = COleCurrency(0,0);

								//update charge resp details, in order of most recent first
								//get the apply information merely for error checking
								_RecordsetPtr rsChargeRespDetail = CreateParamRecordset(
									"SELECT ChargeRespDetailT.ID, ChargeRespDetailT.Amount, ApplyDetailsQ.TotalApplied "
									"FROM ChargeRespDetailT "
									"LEFT JOIN ("
									"	SELECT DetailID, Sum(Amount) AS TotalApplied "
									"	FROM ApplyDetailsT "
									"	GROUP BY DetailID "
									") AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
									"WHERE ChargeRespDetailT.ChargeRespID = {INT} "
									"ORDER BY ChargeRespDetailT.Date DESC", nChargeRespID);
								while(!rsChargeRespDetail->eof && cyRemainingChargeRespDetailAmtToReduce > COleCurrency(0,0)) {
									long nChargeRespDetailID = VarLong(rsChargeRespDetail->Fields->Item["ID"]->Value);
									COleCurrency cyChargeRespDetailAmt = VarCurrency(rsChargeRespDetail->Fields->Item["Amount"]->Value);
									COleCurrency cyApplyDetailTotalAmt = VarCurrency(rsChargeRespDetail->Fields->Item["TotalApplied"]->Value, COleCurrency(0,0));

									//ignore if it has been flagged for deletion
									bDeleted = false;
									mapChargeRespDetailsToDeleted.Lookup(nChargeRespDetailID, bDeleted);
									if(bDeleted) {
										rsChargeRespDetail->MoveNext();
										continue;
									}

									//reconcile the charge resp detail amount with our map, in the event we've already modified it
									cyTemp = COleCurrency(0,0);
									if(mapChargeRespDetailsToAmounts.Lookup(nChargeRespDetailID, cyTemp)) {
										//we have already modified this detail, so update our detail amount to the real amount
										cyChargeRespDetailAmt = cyTemp;
									}

									if(cyChargeRespDetailAmt > cyRemainingChargeRespDetailAmtToReduce) {
										//reduce this amount
										COleCurrency cyNewChargeRespDetailAmt = cyChargeRespDetailAmt - cyRemainingChargeRespDetailAmtToReduce;

										AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ChargeRespDetailT SET Amount = {OLECURRENCY} WHERE ID = {INT}", cyNewChargeRespDetailAmt, nChargeRespDetailID);

										//map this new amount
										mapChargeRespDetailsToAmounts.SetAt(nChargeRespDetailID, cyNewChargeRespDetailAmt);

										cyRemainingChargeRespDetailAmtToReduce = COleCurrency(0,0);
									}
									else {

										//this should not be possible unless the applied amount is zero, fail if it is not zero
										if (cyApplyDetailTotalAmt == COleCurrency(0, 0)) {
											//it is zero, we can safely update this
											AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ChargeRespDetailT WHERE ID = {INT}", nChargeRespDetailID);
										}
										else {
											//this should not be possible - throw an exception

											//this function will get the current call stack, which is translateable using
											//nx-internal\Development\CallStackSymbolizer
											CString strCallStack = GetCallStack();

											ThrowNxException("UndoFinanceCharges failed to update ChargeRespDetailT.ID %li to %s due to a total ApplyDetailsT amount of %s.\n\n"
												"%s",
												nChargeRespDetailID, FormatCurrencyForInterface(COleCurrency(0, 0)), FormatCurrencyForInterface(cyApplyDetailTotalAmt),
												strCallStack);
										}

										cyRemainingChargeRespDetailAmtToReduce -= cyChargeRespDetailAmt;

										//map this new amount
										mapChargeRespDetailsToAmounts.SetAt(nChargeRespDetailID, COleCurrency(0,0));
										mapChargeRespDetailsToDeleted.SetAt(nChargeRespDetailID, true);
									}

									rsChargeRespDetail->MoveNext();
								}
								rsChargeRespDetail->Close();
							}
							else {
								//remove the charge resp entirely
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ChargeRespDetailT WHERE ChargeRespID = {INT}", nChargeRespID);
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ChargeRespT WHERE ID = {INT}", nChargeRespID);
								cyRemainingAmtToReduce -= cyChargeRespAmt;

								//map this new amount
								mapChargeRespsToAmounts.SetAt(nChargeRespID, COleCurrency(0,0));
								//don't need to map that the details were deleted,
								//because they should be skipped from now on due to the
								//charge resp being deleted
								mapChargeRespsToDeleted.SetAt(nChargeRespID, true);
							}

							rsChargeResp->MoveNext();
						}
						rsChargeResp->Close();

						//Now reduce the actual charge amount, and audit.
						COleCurrency cyNewChargeAmount = cyChargeAmount - cyFinanceChargeAmt;
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE LineItemT SET Amount = {OLECURRENCY} WHERE ID = {INT}", cyNewChargeAmount, nChargeID);
						//map this new amount
						mapChargesToAmounts.SetAt(nChargeID, cyNewChargeAmount);

						//throw an exception if any ChargeRespT record for this charge mismatches the ChargeRespDetailT total underneath it
						AddStatementToSqlBatch(strSqlBatch, "SELECT "
							"@ChargeRespIDToCheck = ChargeRespT.ID, "
							"@ChargeRespInsuredPartyIDToCheck = ChargeRespT.InsuredPartyID, "
							"@ChargeRespAmountToCheck = ChargeRespT.Amount, "
							"@ChargeRespDetailTotalToCheck = Sum(IsNull(ChargeRespDetailT.Amount, Convert(money,0))) "
							"FROM ChargeRespT "
							"LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
							"WHERE ChargeRespT.ChargeID = %li "
							"GROUP BY ChargeRespT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount "
							"HAVING ChargeRespT.Amount <> Sum(IsNull(ChargeRespDetailT.Amount, Convert(money, 0)))", nChargeID);
						AddStatementToSqlBatch(strSqlBatch, "IF @ChargeRespIDToCheck Is Not Null \r\n"
							"BEGIN \r\n"
							"SET @errorText = 'The finance charge ID ' + Convert(nvarchar, %li) + ' cannot be undone because its ChargeRespDetailT responsibilites do not match the ChargeRespT responsibility total.\n\n"
							"ChargeRespT.ID = ' + Convert(nvarchar, @ChargeRespIDToCheck) + ', ChargeRespT.InsuredPartyID = ' + Convert(nvarchar, IsNull(@ChargeRespInsuredPartyIDToCheck, -1)) + ', "
							"ChargeRespT.Amount = ' + Convert(nvarchar, @ChargeRespAmountToCheck) + ', ChargeRespDetailT Total = ' + Convert(nvarchar, @ChargeRespDetailTotalToCheck); \r\n"
							"RAISERROR(@errorText, 16, 1) ROLLBACK TRAN RETURN \r\n"
							"END", nChargeID);

						//It is possible that we might audit the same charge more than once if we remove two
						//finance charges that point to the same charge. If so, multiple audits are intentional.
						CString strOldChargeValue, strNewChargeValue;
						strOldChargeValue.Format("Description: '%s', Amount: %s, Date: %s", strChargeDescription, FormatCurrencyForInterface(cyChargeAmount), FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate));
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiChargeLineAmount, nChargeID, strOldChargeValue, FormatCurrencyForInterface(cyNewChargeAmount), aepHigh, aetChanged);

						//track our details of what changed
						nCountChargesReduced++;
						cyTotalChargeAmtRemoved += cyFinanceChargeAmt;
						mapPatientsAffected.SetAt(nPatientID, true);
					}
					else {
						//delete the charge altogether, even if for some reason the
						//current amount is less than what we were expecting

						//Remove the charge, and audit.
						//It is possible that we might audit the deletion after previously auditing a reduction,
						//in the event that multiple finance charges point to the same charge.
						//If so, multiple audits are intentional.
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE LineItemT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} WHERE ID = {INT}", GetCurrentUserName(), nChargeID);

						//map this new amount
						mapChargesToAmounts.SetAt(nChargeID, COleCurrency(0,0));
						mapChargesToDeleted.SetAt(nChargeID, true);

						CString strOldChargeValue;
						strOldChargeValue.Format("Description: '%s', Amount: %s, Date: %s", strChargeDescription, FormatCurrencyForInterface(cyChargeAmount), FormatDateTimeForInterface(dtChargeDate, NULL, dtoDate));
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiChargeLineDeleted, nChargeID, strOldChargeValue, "<Deleted>", aepHigh, aetDeleted);

						//track our details of what changed
						nCountChargesDeleted++;
						cyTotalChargeAmtRemoved += cyFinanceChargeAmt;
						mapPatientsAffected.SetAt(nPatientID, true);
					}
				}

				//remove the finance charge record, and audit
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM FinanceChargesT WHERE ID = {INT}", nFinanceChargeRecordID);

				CString strOldFCValue;
				strOldFCValue.Format("For %s", FormatCurrencyForInterface(cyFinanceChargeAmt));
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiFinanceChargeDelete, nFinanceChargeRecordID, strOldFCValue, "<Deleted>", aepHigh, aetDeleted);

				rs->MoveNext();
			}
			rs->Close();

			//now remove the finance charge history record, and audit
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM FinanceChargeHistoryT WHERE ID = {INT}", nBatchIDToUndo);
			AuditEvent(-1, "", nAuditTransactionID, aeiUndoFinanceChargeBatch, nBatchIDToUndo, strOldFinanceChargeBatchInformation, "<Deleted>", aepHigh, aetDeleted);

			//fire away
			NxAdo::PushMaxRecordsWarningLimit pmr(100000);
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;

			//refresh the active view
			CMainFrame *pMainFrame = GetMainFrame();
			if(pMainFrame) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView) {
					pView->UpdateView();
				}
			}

			//now tell the user what we just did
			CString strSuccess;
			strSuccess.Format("The selected finance charge batch has been deleted.\n\n"
				"%s in charges have been removed from the system, affecting %li patient%s. "
				"%li charge%s deleted, and %li charge%s had %s amount%s reduced.",
				FormatCurrencyForInterface(cyTotalChargeAmtRemoved),
				mapPatientsAffected.GetCount(), mapPatientsAffected.GetCount() == 1 ? "" : "s",
				nCountChargesDeleted, nCountChargesDeleted == 1 ? " was" : "s were",
				nCountChargesReduced, nCountChargesReduced == 1 ? "" : "s", nCountChargesReduced == 1 ? "its" : "their", nCountChargesReduced == 1 ? "" : "s");
			MessageBox(GetActiveWindow(), strSuccess, "Practice", MB_ICONINFORMATION|MB_OK);
		}

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void AutoUpdateBatchPaymentDepositDates(long nBatchPaymentID)
{
	//used because now we try to keep child payments in line with the batch payments
	//in terms of deposit date, etc. This is used if we create a child payment after
	//the batch payment has been deposited.
	
	_RecordsetPtr rs = CreateParamRecordset("SELECT DepositDate, DepositInputDate FROM BatchPaymentsT WHERE ID = {INT} AND Deposited = 1",nBatchPaymentID);
	if(!rs->eof) {
		// (j.jones 2009-06-09 15:46) - PLID 34549 - ignore adjustments
		// (j.jones 2009-10-07 09:01) - PLID 36426 - parameterized
		ExecuteParamSql("UPDATE PaymentsT SET Deposited = 1, DepositDate = {STRING}, DepositInputDate = {STRING} WHERE BatchPaymentID = {INT} AND PayMethod <> 0",
			FormatDateTimeForSql(AdoFldDateTime(rs, "DepositDate"), dtoDateTime),
			FormatDateTimeForSql(AdoFldDateTime(rs, "DepositInputDate"), dtoDateTime),
			nBatchPaymentID);
	}
	rs->Close();
}

// (j.jones 2012-08-23 12:42) - PLID 42438 - added silent option
BOOL CheckUnbatchClaim(long nBillID, BOOL bSilent /*= FALSE*/)
{
	//given a BillID, see if there is $0.00 insurance balance left,
	//and if the claim is still batched, prompt to unbatch it

	//first see if the claim is batched
	long nBatch = FindHCFABatch(nBillID);
	
	if(nBatch == 0) {
		//already unbatched
		return FALSE;
	}

	COleCurrency cyTotalInsuranceBalance = COleCurrency(0,0);

	// (j.jones 2008-04-29 10:11) - PLID 29819 - parameterized and included PatientID and InsuredPartyID
	_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.PatientID, InsuredPartyT.PersonID, InsuredPartyT.PatientID, RespTypeID "
		"FROM InsuredPartyT "
		"INNER JOIN BillsT ON InsuredPartyT.PatientID = BillsT.PatientID "
		"WHERE BillsT.ID = {INT}", nBillID);

	while(!rs->eof) {

		COleCurrency cyCharges = COleCurrency(0,0), cyPayments = COleCurrency(0,0), cyAdjustments = COleCurrency(0,0), cyRefunds = COleCurrency(0,0);

		long nInsuredPartyID = AdoFldLong(rs, "PersonID");
		long nPatientID = AdoFldLong(rs, "PatientID");
		long nRespTypeID = AdoFldLong(rs, "RespTypeID");

		// (j.jones 2008-04-29 10:12) - PLID 29819 - if inactive, GetBillInsuranceTotals
		// is unreliable, instead we have to call GetInactiveInsTotals in its place
		if(nRespTypeID == -1) {
			//inactive insurance
			if(!GetInactiveInsTotals(nInsuredPartyID, nBillID, -1, nPatientID, cyCharges, cyPayments)) {
				return FALSE;
			}			
		}
		else {
			// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
			if(!GetBillInsuranceTotals(nBillID,
				AdoFldLong(rs, "PatientID"), nInsuredPartyID, 
				&cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
				return FALSE;
			}
		}

		COleCurrency cyRespBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
		cyTotalInsuranceBalance += cyRespBalance;

		rs->MoveNext();
	}
	rs->Close();

	//they have an insurance balance, so don't do anything
	if(cyTotalInsuranceBalance > COleCurrency(0,0)) {
		return FALSE;
	}
	
	//if we get here, they have a zero insurance balance and the bill is batched, so prompt to unbatch

	// (j.jones 2012-08-23 12:42) - PLID 42438 - added silent option
	if(!bSilent) {
		CString str;
		str.Format("This bill is currently in the %s batch, but the insurance balance is now %s.\n"
			"Would you like to unbatch the bill?",nBatch == 1 ? "paper" : "electronic", FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));

		if(IDNO == MessageBox(GetActiveWindow(), str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return FALSE;
		}
	}

	//now unbatch the bill
	BatchBill(nBillID,0);
	return TRUE;
}

COleCurrency CalculateTotalPackageValueWithTax(long nQuoteID)
{	
	COleCurrency cy = COleCurrency(0,0);

	long nTotalCount = 0;
	COleCurrency cyTotalAmount = COleCurrency(0,0);
	long nType = 1;

	// (j.jones 2009-10-23 13:05) - PLID 32904 - check the preference for how tax should be estimated
	BOOL bOverestimateTax = GetRemotePropertyInt("PackageTaxEstimation", 0, 0, "<None>", true) == 0;

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted all recordsets in this function to be parameterized
	
	//first get the count of uses
	_RecordsetPtr rs = CreateParamRecordset("SELECT Type, TotalCount, TotalAmount FROM PackagesT WHERE QuoteID = {INT}", nQuoteID);
	if(!rs->eof) {
		nType = AdoFldLong(rs, "Type",1);
		nTotalCount = AdoFldLong(rs, "TotalCount",0);
		cyTotalAmount = AdoFldCurrency(rs, "TotalAmount",COleCurrency(0,0));
	}
	rs->Close();

	//now get the charges for each usage of the bill

	if(nType == 1) {

		//repeat package

		// (j.jones 2009-10-26 09:36) - PLID 32904 - If we are over-estimating tax, we want to calculate tax per use,
		// times the total uses. If we are under-estimating tax, we want to multiply by the total uses, then add tax.
		long nCountToLoop = nTotalCount;
		long nCountToMultiply = 1;
		if(!bOverestimateTax) {
			nCountToMultiply = nTotalCount;
			nCountToLoop = 1;
		}

		for(int i=0; i<nCountToLoop; i++) {

			// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

			rs = CreateParamRecordset("SELECT Round(Convert(money,(PackageChargesQ.PracBillFee*[Quantity]*{INT})),2) AS ChargeNonTaxTotal, "
				"Round(Convert(money,(((PackageChargesQ.PracBillFee*[Quantity]*{INT})) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate-1)) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate2-1)) "
				")),2) AS ChargeAmount "
				""
				"FROM (SELECT ChargesT.ID AS ChargeID, (CASE WHEN TotalCount-{INT} > 1 THEN "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(TotalAmount/TotalCount/NumCharges/Quantity)),2) ELSE "
				//calculate using the percentage of the total
				"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2) END)"
				"ELSE "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN (CASE WHEN Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
					"Round(Convert(money,(Convert(money,{STRING}) - Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * (NumCharges-1)) / Quantity),2) "
					"ELSE Round(Convert(money,(Convert(money,{STRING})/NumCharges/Quantity)),2) END) ELSE "
				//calculate using the percentage of the total
				"(CASE WHEN Round(Convert(money,(Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
				//bad ->"Round(Convert(money,(CurrentAmount - Round(Convert(money,(CurrentAmount * ChargeNonTaxTotal / BillNonTaxTotal)),2) * (NumCharges-1)) / Quantity),2) "
				"Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2)"
				"ELSE Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2) END) END) END) AS PracBillFee "
				""
				"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
				""
				//get the non tax charge total
				// (j.gruber 2009-03-18 09:23) - PLID 33360 - change the discount structure
				"Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
				""
				"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE LineItemT.Deleted=0 AND LineItemT.Type>=10) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
				"INNER JOIN (SELECT PackagesT.*, TotalAmount/TotalCount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
				"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
				""
				//get the non tax bill total
				// (j.gruber 2009-03-18 09:25) - PLID 33360 - change discount structure
				"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
				""
				"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
				"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
				"WHERE PatientChargesQ.BillID = {INT} "
				") AS PackageChargesQ "
				"INNER JOIN ChargesT ON PackageChargesQ.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 ",
				nCountToMultiply, nCountToMultiply, nCountToMultiply, nCountToMultiply,
				i,FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount), FormatCurrencyForSql(cyTotalAmount),
				nQuoteID, nQuoteID);

			while(!rs->eof) {
				cy += AdoFldCurrency(rs, "ChargeAmount");
				cyTotalAmount -= AdoFldCurrency(rs, "ChargeNonTaxTotal");

				rs->MoveNext();
			}
			rs->Close();
		}
	}
	else {
		//multi-use package

		// (j.jones 2007-03-26 14:46) - PLID 25287 - calculate the potential offset
		// and "target" service ID before our main calculation, using the _ForTotal version
		COleCurrency cyMatchRemainingPackageAmount = CalculateMultiUsePackageOverageAmount_ForTotal(nQuoteID);

		// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
		long nTargetChargeID = -1;
		//no need to calculate the charge ID if the offset is zero
		if(cyMatchRemainingPackageAmount != COleCurrency(0,0)) {
			nTargetChargeID = CalculateMultiUsePackageOverageChargeID(nQuoteID);
		}

		// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientChargesQ.ID, "
			//handle a zero dollar bill
			"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(TotalAmount/NumCharges/Quantity)),2) ELSE "
			//calculate using the percentage of the total
			"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) /Quantity)),2) END) "
			"AS PracBillFee, "
			""
			"TaxRate, TaxRate2, ChargesT.ServiceID, ChargesT.Quantity "
			""
			"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
			""
			//get the non tax charge total
			// (j.gruber 2009-03-18 09:26) - PLID 33360 - change the discount structure
			"Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
			""
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE LineItemT.Deleted = 0) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
			"INNER JOIN (SELECT PackagesT.*, TotalAmount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
			"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
			""
			//get the non tax bill total
			// (j.gruber 2009-03-18 09:27) - PLID 33360 - changed the discount structure
			"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
			""
			"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
			"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
			"WHERE PatientChargesQ.BillID = {INT} ORDER BY ChargesT.LineID ", nQuoteID, nQuoteID);

		while(!rs->eof) {
			long nChargeID = AdoFldLong(rs, "ID");
			COleCurrency cyChargeNonTaxTotal = AdoFldCurrency(rs, "PracBillFee");
			long nServiceID = AdoFldLong(rs, "ServiceID",-1);
			double dblQuantity = AdoFldDouble(rs, "Quantity",0.0);
			double dblTax1 = AdoFldDouble(rs, "TaxRate",0.0);
			double dblTax2 = AdoFldDouble(rs, "TaxRate2",0.0);

			// (j.jones 2007-03-26 14:46) - PLID 25287 - if we have nTargetServiceID,
			// and are on that service, it means we need to split the charge such that
			// we calculate a charge with a quantity of 1 and our offset, then calculate
			// the remaining quantity as a separate charge
			// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
			if(nTargetChargeID != -1 && nTargetChargeID == nChargeID) {

				if(dblQuantity == 1.0) {
					//need to do nothing special but apply our overage
					cyChargeNonTaxTotal += cyMatchRemainingPackageAmount;
				}
				else if(dblQuantity > 1.0) {

					//separate out a charge with a quantity of 1,
					//calculate its value, decrement the remaining
					//quantity by 1, continue on

					COleCurrency cyNewChargeNonTaxTotal = cyChargeNonTaxTotal + cyMatchRemainingPackageAmount;

					COleCurrency cyTotalTax1 = CalculateTax(cyNewChargeNonTaxTotal,dblTax1);
					COleCurrency cyTotalTax2 = CalculateTax(cyNewChargeNonTaxTotal,dblTax2);

					COleCurrency cyChargeTaxTotal = cyNewChargeNonTaxTotal;

					cyChargeTaxTotal += cyTotalTax1;
					cyChargeTaxTotal += cyTotalTax2;

					cy += cyChargeTaxTotal;

					//decrement the quantity by 1 before continuing
					dblQuantity -= 1.0;
				}
				else if(dblQuantity > 0.0) {
					//we should not allow a quantity < 1
					ASSERT(FALSE);
				}
			}			

			// (j.jones 2007-04-23 15:23) - PLID 25752 - We need to calculate tax and quantity
			// a bit backwards from the way we normally do it. In a normal bill calculation,
			// it's amount * quantity * tax, with the result rounded. But with a multi-use
			// package it's different because in most cases, the charges will be billed individually,
			// and so we need to have amount * tax, rounded, then multiply by quantity, rounding again.

			// (j.jones 2009-10-26 10:18) - PLID 32904 - if we are over-estimating tax, we tax each use,
			// but if we are under-estimating tax, we tax the total
			if(bOverestimateTax) {
				//calculate tax first
				COleCurrency cyTotalTax1 = CalculateTax(cyChargeNonTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargeNonTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargeNonTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;

				//round after tax
				RoundCurrency(cyChargeTaxTotal);

				//now multiply the tax total by the quantity
				cyChargeTaxTotal = CalculateAmtQuantity(cyChargeTaxTotal,dblQuantity);

				//round after quantity
				RoundCurrency(cyChargeTaxTotal);

				cy += cyChargeTaxTotal;
			}
			else {
				//calculate quantity first
				COleCurrency cyChargePreTaxTotal = CalculateAmtQuantity(cyChargeNonTaxTotal,dblQuantity);

				//round after quantity
				RoundCurrency(cyChargePreTaxTotal);

				COleCurrency cyTotalTax1 = CalculateTax(cyChargePreTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargePreTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargePreTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;

				//round after tax
				RoundCurrency(cyChargeTaxTotal);				

				cy += cyChargeTaxTotal;
			}

			rs->MoveNext();
		}
		rs->Close();

		if(cy < COleCurrency(0,0))
			cy = COleCurrency(0,0);
	}

	return cy;
}

// (j.jones 2007-04-23 13:03) - PLID 25735 - the Original package value
// may not equal the "Total" package value, as it is based off of the original "current amount"
// (j.gruber 2007-08-15 11:46) - PLID 25851 - changed function to allow just getting tax1 or tax2
COleCurrency CalculateOriginalPackageValueWithTax(long nQuoteID, long nTaxRateToCalculate /*=-1*/)
{	
	COleCurrency cy = COleCurrency(0,0);

	long nOriginalCount = 0;
	COleCurrency cyOriginalAmount = COleCurrency(0,0);
	long nType = 1;

	// (j.jones 2009-10-23 13:05) - PLID 32904 - check the preference for how tax should be estimated
	BOOL bOverestimateTax = GetRemotePropertyInt("PackageTaxEstimation", 0, 0, "<None>", true) == 0;

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted all recordsets in this function to be parameterized
	
	//first get the count of uses
	_RecordsetPtr rs = CreateParamRecordset("SELECT Type, OriginalCurrentCount, OriginalCurrentAmount FROM PackagesT WHERE QuoteID = {INT}", nQuoteID);
	if(!rs->eof) {
		nType = AdoFldLong(rs, "Type",1);
		nOriginalCount = AdoFldLong(rs, "OriginalCurrentCount",0);
		cyOriginalAmount = AdoFldCurrency(rs, "OriginalCurrentAmount",COleCurrency(0,0));
	}
	rs->Close();

	//now get the charges for each usage of the bill

	if(nType == 1) {

		//repeat package

		// (j.jones 2009-10-26 09:36) - PLID 32904 - If we are over-estimating tax, we want to calculate tax per use,
		// times the total uses. If we are under-estimating tax, we want to multiply by the total uses, then add tax.
		long nCountToLoop = nOriginalCount;
		long nCountToMultiply = 1;
		if(!bOverestimateTax) {
			nCountToMultiply = nOriginalCount;
			nCountToLoop = 1;
		}

		for(int i=0; i<nCountToLoop; i++) {

			// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts
			// (j.gruber 2009-03-16 11:29) - PLID 33360 - update discount structure
			rs = CreateParamRecordset("SELECT Round(Convert(money,(PackageChargesQ.PracBillFee*[Quantity]*{INT})),2) AS ChargeNonTaxTotal, "
				"Round(Convert(money,(((PackageChargesQ.PracBillFee*[Quantity]*{INT})) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate-1)) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate2-1)) "
				")),2) AS ChargeAmount, "
				"Round(Convert(money,(((PackageChargesQ.PracBillFee*[Quantity])*(TaxRate-1))) "
				"),2) AS Tax1Amount, "
			    "Round(Convert(money,(((PackageChargesQ.PracBillFee*[Quantity])*(TaxRate2-1))) "
				"),2) AS Tax2Amount "
				""
				"FROM (SELECT ChargesT.ID AS ChargeID, (CASE WHEN OriginalCurrentCount-{INT} > 1 THEN "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(OriginalCurrentAmount/OriginalCurrentCount/NumCharges/Quantity)),2) ELSE "
				//calculate using the percentage of the total
				"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2) END)"
				"ELSE "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN (CASE WHEN Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
					"Round(Convert(money,(Convert(money,{STRING}) - Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * (NumCharges-1)) / Quantity),2) "
					"ELSE Round(Convert(money,(Convert(money,{STRING})/NumCharges/Quantity)),2) END) ELSE "
				//calculate using the percentage of the total
				"(CASE WHEN Round(Convert(money,(Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
				//bad ->"Round(Convert(money,(CurrentAmount - Round(Convert(money,(CurrentAmount * ChargeNonTaxTotal / BillNonTaxTotal)),2) * (NumCharges-1)) / Quantity),2) "
				"Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2)"
				"ELSE Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal) / Quantity)),2) END) END) END) AS PracBillFee "
				""
				"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
				""
				//get the non tax charge total
				"Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
				""
				"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE LineItemT.Deleted=0 AND LineItemT.Type>=10) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
				"INNER JOIN (SELECT PackagesT.*, OriginalCurrentAmount/OriginalCurrentCount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
				"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
				""
				//get the non tax bill total
				"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, Sum(PercentOff) AS TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentOffQ ON ChargesT.ID = TotalPercentOffQ.ChargeID LEFT JOIN (SELECT ChargeID, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
				""
				"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
				"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
				"WHERE PatientChargesQ.BillID = {INT} "
				") AS PackageChargesQ "
				"INNER JOIN ChargesT ON PackageChargesQ.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 ",
				nCountToMultiply, nCountToMultiply, nCountToMultiply, nCountToMultiply,
				i,FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount), FormatCurrencyForSql(cyOriginalAmount),
				nQuoteID, nQuoteID);

			while(!rs->eof) {
				if (nTaxRateToCalculate == 1) {
					cy += AdoFldCurrency(rs, "Tax1Amount");
					cyOriginalAmount -= AdoFldCurrency(rs, "ChargeNonTaxTotal");
				}
				else if (nTaxRateToCalculate == 2) {
					cy += AdoFldCurrency(rs, "Tax2Amount");
					cyOriginalAmount -= AdoFldCurrency(rs, "ChargeNonTaxTotal");
				}
				else {
					cy += AdoFldCurrency(rs, "ChargeAmount");
					cyOriginalAmount -= AdoFldCurrency(rs, "ChargeNonTaxTotal");
				}

				rs->MoveNext();
			}
			rs->Close();
		}
	}
	else {
		//multi-use package

		// (j.jones 2007-03-26 14:46) - PLID 25287 - calculate the potential offset
		// and "target" service ID before our main calculation
		COleCurrency cyMatchRemainingPackageAmount = CalculateMultiUsePackageOverageAmount_ForBalance(nQuoteID);
		
		// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
		long nTargetChargeID = -1;
		//no need to calculate the charge ID if the offset is zero
		if(cyMatchRemainingPackageAmount != COleCurrency(0,0)) {
			nTargetChargeID = CalculateMultiUsePackageOverageChargeID(nQuoteID);
		}

		// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientChargesQ.ID, "
			// (j.jones 2007-07-03 10:29) - PLID 26539 - handled divide by zero for 0-quantities
			"(CASE WHEN OriginalPackageQtyRemaining = 0 THEN Convert(money,0) ELSE "
			//handle a zero dollar bill
			"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(OriginalCurrentAmount/NumCharges/OriginalPackageQtyRemaining)),2) ELSE "
			//calculate using the percentage of the total
			"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / OriginalPackageQtyRemaining)),2) END) "
			"END) "
			"AS PracBillFee, "
			""
			"TaxRate, TaxRate2, ChargesT.ServiceID, ChargesT.OriginalPackageQtyRemaining AS Quantity "
			""
			"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
			""
			//get the non tax charge total
			"Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
			""
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE LineItemT.Deleted = 0) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
			"INNER JOIN (SELECT PackagesT.*, OriginalCurrentAmount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
			"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
			""
			//get the non tax bill total
			"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, Sum(PercentOff) AS TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentOffQ ON ChargesT.ID = TotalPercentOffQ.ChargeID LEFT JOIN (SELECT ChargeID, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
			""
			"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
			"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
			"WHERE PatientChargesQ.BillID = {INT} ORDER BY ChargesT.LineID ", nQuoteID, nQuoteID);

		while(!rs->eof) {
			long nChargeID = AdoFldLong(rs, "ID");
			COleCurrency cyChargeNonTaxTotal = AdoFldCurrency(rs, "PracBillFee");
			long nServiceID = AdoFldLong(rs, "ServiceID",-1);
			double dblQuantity = AdoFldDouble(rs, "Quantity",0.0);
			double dblTax1 = AdoFldDouble(rs, "TaxRate",0.0);
			double dblTax2 = AdoFldDouble(rs, "TaxRate2",0.0);

			// (j.jones 2007-03-26 14:46) - PLID 25287 - if we have nTargetServiceID,
			// and are on that service, it means we need to split the charge such that
			// we calculate a charge with a quantity of 1 and our offset, then calculate
			// the remaining quantity as a separate charge
			// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
			if(nTargetChargeID != -1 && nTargetChargeID == nChargeID) {

				if(dblQuantity == 1.0) {
					//need to do nothing special but apply our overage
					cyChargeNonTaxTotal += cyMatchRemainingPackageAmount;
				}
				else if(dblQuantity > 1.0) {

					//separate out a charge with a quantity of 1,
					//calculate its value, decrement the remaining
					//quantity by 1, continue on

					COleCurrency cyNewChargeNonTaxTotal = cyChargeNonTaxTotal + cyMatchRemainingPackageAmount;

					COleCurrency cyTotalTax1 = CalculateTax(cyNewChargeNonTaxTotal,dblTax1);
					COleCurrency cyTotalTax2 = CalculateTax(cyNewChargeNonTaxTotal,dblTax2);

					COleCurrency cyChargeTaxTotal = cyNewChargeNonTaxTotal;

					cyChargeTaxTotal += cyTotalTax1;
					cyChargeTaxTotal += cyTotalTax2;

					if (nTaxRateToCalculate == 1) {
						cy += cyTotalTax1;
					}
					else if (nTaxRateToCalculate == 2) {
						cy += cyTotalTax2;
					}
					else {
						cy += cyChargeTaxTotal;
					}

					//decrement the quantity by 1 before continuing
					dblQuantity -= 1.0;
				}
				else if(dblQuantity > 0.0) {
					//we should not allow a quantity < 1
					ASSERT(FALSE);
				}
			}			

			// (j.jones 2007-04-23 15:23) - PLID 25752 - We need to calculate tax and quantity
			// a bit backwards from the way we normally do it. In a normal bill calculation,
			// it's amount * quantity * tax, with the result rounded. But with a multi-use
			// package it's different because in most cases, the charges will be billed individually,
			// and so we need to have amount * tax, rounded, then multiply by quantity, rounding again.

			// (j.jones 2009-10-26 10:18) - PLID 32904 - if we are over-estimating tax, we tax each use,
			// but if we are under-estimating tax, we tax the total
			if(bOverestimateTax) {
				//calculate tax first			
				COleCurrency cyTotalTax1 = CalculateTax(cyChargeNonTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargeNonTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargeNonTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;
				
				if (nTaxRateToCalculate == 1) {
					//round after tax
					RoundCurrency(cyTotalTax1);
					//now multiply the tax total by the quantity
					cyTotalTax1 = CalculateAmtQuantity(cyTotalTax1,dblQuantity);
					//round after quantity
					RoundCurrency(cyTotalTax1);
					cy += cyTotalTax1;
					
				}
				else if (nTaxRateToCalculate == 2) {
					//round after tax
					RoundCurrency(cyTotalTax2);
					//now multiply the tax total by the quantity
					cyTotalTax2 = CalculateAmtQuantity(cyTotalTax2,dblQuantity);
					//round after quantity
					RoundCurrency(cyTotalTax2);
					cy += cyTotalTax2;
				}
				else { 
					//round after tax
					RoundCurrency(cyChargeTaxTotal);
					//now multiply the tax total by the quantity
					cyChargeTaxTotal = CalculateAmtQuantity(cyChargeTaxTotal,dblQuantity);
					//round after quantity
					RoundCurrency(cyChargeTaxTotal);
					cy += cyChargeTaxTotal;
				}
			}
			else {
				//calculate quantity first
				COleCurrency cyChargePreTaxTotal = CalculateAmtQuantity(cyChargeNonTaxTotal,dblQuantity);

				//round after quantity
				RoundCurrency(cyChargePreTaxTotal);

				COleCurrency cyTotalTax1 = CalculateTax(cyChargePreTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargePreTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargePreTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;
				
				if (nTaxRateToCalculate == 1) {
					//round after tax
					RoundCurrency(cyTotalTax1);
					cy += cyTotalTax1;
					
				}
				else if (nTaxRateToCalculate == 2) {
					//round after tax
					RoundCurrency(cyTotalTax2);
					cy += cyTotalTax2;
				}
				else { 
					//round after tax
					RoundCurrency(cyChargeTaxTotal);
					cy += cyChargeTaxTotal;
				}
			}	

			rs->MoveNext();
		}
		rs->Close();

		if(cy < COleCurrency(0,0))
			cy = COleCurrency(0,0);
	}

	return cy;
}

COleCurrency CalculateRemainingPackageValueWithTax(long nQuoteID)
{
	COleCurrency cy = COleCurrency(0,0);

	long nCurrentCount = 0;
	COleCurrency cyCurrentAmount = COleCurrency(0,0);
	long nType = 1;

	// (j.jones 2009-10-23 13:05) - PLID 32904 - check the preference for how tax should be estimated
	BOOL bOverestimateTax = GetRemotePropertyInt("PackageTaxEstimation", 0, 0, "<None>", true) == 0;

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted all recordsets in this function to be parameterized
	
	//first get the count of uses
	_RecordsetPtr rs = CreateParamRecordset("SELECT Type, CurrentCount, CurrentAmount FROM PackagesT WHERE QuoteID = {INT}", nQuoteID);
	if(!rs->eof) {
		nType = AdoFldLong(rs, "Type",1);
		nCurrentCount = AdoFldLong(rs, "CurrentCount",0);
		cyCurrentAmount = AdoFldCurrency(rs, "CurrentAmount",COleCurrency(0,0));
	}
	rs->Close();

	//now get the charges for each usage of the bill

	if(nType == 1) {
		//repeat package

		// (j.jones 2009-10-26 09:36) - PLID 32904 - If we are over-estimating tax, we want to calculate tax per use,
		// times the total uses. If we are under-estimating tax, we want to multiply by the total uses, then add tax.
		long nCountToLoop = nCurrentCount;
		long nCountToMultiply = 1;
		if(!bOverestimateTax) {
			nCountToMultiply = nCurrentCount;
			nCountToLoop = 1;
		}

		for(int i=0; i<nCountToLoop; i++) {

			// (j.jones 2007-04-23 12:51) - PLID 25735 - any references to TotalAmount and TotalCount should
			// instead use OriginalCurrentCount and OriginalCurrentAmount, since we're truly basing our calculations
			// off of that information

			// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

			rs = CreateParamRecordset("SELECT Round(Convert(money,(PackageChargesQ.PracBillFee*[Quantity]*{INT})),2) AS ChargeNonTaxTotal, "
				"Round(Convert(money,(((PackageChargesQ.PracBillFee*[Quantity]*{INT})) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate-1)) + "
				"((PackageChargesQ.PracBillFee*[Quantity]*{INT})*(TaxRate2-1)) "
				")),2) AS ChargeAmount "
				""
				"FROM (SELECT ChargesT.ID AS ChargeID, (CASE WHEN CurrentCount-{INT} > 1 THEN "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(OriginalCurrentAmount/OriginalCurrentCount/NumCharges/Quantity)),2) ELSE "
				//calculate using the percentage of the total
				"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) /Quantity)),2) END) "
				"ELSE "
				//handle a zero dollar bill
				"(CASE WHEN BillNonTaxTotal = 0 THEN (CASE WHEN Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
					"Round(Convert(money,(Convert(money,{STRING}) - Round(Convert(money,(Convert(money,{STRING})/NumCharges)),2) * (NumCharges-1)) / Quantity),2) "
					"ELSE Round(Convert(money,(Convert(money,{STRING})/NumCharges/Quantity)),2) END) ELSE "
				//calculate using the percentage of the total
				"(CASE WHEN Round(Convert(money,(Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal)),2) * NumCharges <> Convert(money,{STRING}) AND LineID = 1 THEN "
				//bad ->"Round(Convert(money,(CurrentAmount - Round(Convert(money,(CurrentAmount * ChargeNonTaxTotal / BillNonTaxTotal)),2) * (NumCharges-1)) / Quantity),2) "
				"Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal)/Quantity)),2)"
				"ELSE Round(Convert(money,((Convert(money,{STRING}) * ChargeNonTaxTotal / BillNonTaxTotal) /Quantity)),2) END) END) END) AS PracBillFee "
				""
				"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
				""
				//get the non tax charge total
				// (j.gruber 2009-03-18 09:30) - PLID 33360 - change discount category
				"Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
				""
				"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE LineItemT.Deleted=0 AND LineItemT.Type>=10) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
				"INNER JOIN (SELECT PackagesT.*, OriginalCurrentAmount/OriginalCurrentCount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
				"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
				""
				//get the non tax bill total
				// (j.gruber 2009-03-18 09:34) - PLID 33360 - new discount structure
				"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
				""
				"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
				"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
				"WHERE PatientChargesQ.BillID = {INT} "
				") AS PackageChargesQ "
				"INNER JOIN ChargesT ON PackageChargesQ.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 ",
				nCountToMultiply, nCountToMultiply, nCountToMultiply, nCountToMultiply,
				i,FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount), FormatCurrencyForSql(cyCurrentAmount),
				nQuoteID, nQuoteID);

			while(!rs->eof) {
				cy += AdoFldCurrency(rs, "ChargeAmount");
				cyCurrentAmount -= AdoFldCurrency(rs, "ChargeNonTaxTotal");

				rs->MoveNext();
			}
			rs->Close();
		}
	}
	else {
		//multi-use package

		// (j.jones 2007-03-26 14:47) - PLID 25287 - calculate the potential offset
		// and "target" service ID before our main calculation
		COleCurrency cyMatchRemainingPackageAmount = CalculateMultiUsePackageOverageAmount_ForBalance(nQuoteID);
		
		// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
		long nTargetChargeID = -1;
		//no need to calculate the charge ID if the offset is zero
		if(cyMatchRemainingPackageAmount != COleCurrency(0,0)) {
			nTargetChargeID = CalculateMultiUsePackageOverageChargeID(nQuoteID);
		}

		// (j.jones 2007-04-23 12:51) - PLID 25735 - any references to TotalAmount and TotalCount should
		// instead use OriginalCurrentCount and OriginalCurrentAmount, since we're truly basing our calculations
		// off of that information

		// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientChargesQ.ID, "
			// (j.jones 2007-07-03 10:29) - PLID 26539 - handled divide by zero for 0-quantities
			"(CASE WHEN OriginalPackageQtyRemaining = 0 THEN Convert(money,0) ELSE "
			//handle a zero dollar bill
			"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(OriginalCurrentAmount/NumCharges/OriginalPackageQtyRemaining)),2) ELSE "
			//calculate using the percentage of the total
			"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / OriginalPackageQtyRemaining)),2) END) "
			"END) "
			"AS PracBillFee, "
			""
			"TaxRate, TaxRate2, ChargesT.ServiceID, ChargesT.PackageQtyRemaining "
			""
			"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
			""
			//get the non tax charge total
			// (j.gruber 2009-03-18 09:37) - PLID 33360 - updated discount structure
			"Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
			""
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE LineItemT.Deleted = 0) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
			"INNER JOIN (SELECT PackagesT.*, OriginalCurrentAmount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
			"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
			""
			//get the non tax bill total
			// (j.gruber 2009-03-18 09:37) - PLID 33360 - updated discount structure
			"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
			""
			"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
			"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
			"WHERE PatientChargesQ.BillID = {INT} ORDER BY ChargesT.LineID ", nQuoteID, nQuoteID);

		while(!rs->eof) {
			long nChargeID = AdoFldLong(rs, "ID");
			COleCurrency cyChargeNonTaxTotal = AdoFldCurrency(rs, "PracBillFee");
			long nServiceID = AdoFldLong(rs, "ServiceID",-1);
			double dblQuantity = AdoFldDouble(rs, "PackageQtyRemaining",0.0);
			
			if(dblQuantity <= 0.0) {
				rs->MoveNext();
				continue;
			}

			double dblTax1 = AdoFldDouble(rs, "TaxRate",0.0);
			double dblTax2 = AdoFldDouble(rs, "TaxRate2",0.0);

			// (j.jones 2007-03-26 14:47) - PLID 25287 - if we have nTargetServiceID,
			// and are on that service, it means we need to split the charge such that
			// we calculate a charge with a quantity of 1 and our offset, then calculate
			// the remaining quantity as a separate charge
			// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
			if(nTargetChargeID != -1 && nTargetChargeID == nChargeID) {

				if(dblQuantity == 1.0) {
					//need to do nothing special but apply our overage
					cyChargeNonTaxTotal += cyMatchRemainingPackageAmount;
				}
				else if(dblQuantity > 1.0) {

					//separate out a charge with a quantity of 1,
					//calculate its value, decrement the remaining
					//quantity by 1, continue on

					COleCurrency cyNewChargeNonTaxTotal = cyChargeNonTaxTotal + cyMatchRemainingPackageAmount;

					COleCurrency cyTotalTax1 = CalculateTax(cyNewChargeNonTaxTotal,dblTax1);
					COleCurrency cyTotalTax2 = CalculateTax(cyNewChargeNonTaxTotal,dblTax2);

					COleCurrency cyChargeTaxTotal = cyNewChargeNonTaxTotal;

					cyChargeTaxTotal += cyTotalTax1;
					cyChargeTaxTotal += cyTotalTax2;

					cy += cyChargeTaxTotal;

					//decrement the quantity by 1 before continuing
					dblQuantity -= 1.0;
				}
				else if(dblQuantity > 0.0) {
					//we should not allow a quantity < 1
					ASSERT(FALSE);
				}
			}

			// (j.jones 2007-04-23 15:23) - PLID 25752 - We need to calculate tax and quantity
			// a bit backwards from the way we normally do it. In a normal bill calculation,
			// it's amount * quantity * tax, with the result rounded. But with a multi-use
			// package it's different because in most cases, the charges will be billed individually,
			// and so we need to have amount * tax, rounded, then multiply by quantity, rounding again.

			// (j.jones 2009-10-26 10:18) - PLID 32904 - if we are over-estimating tax, we tax each use,
			// but if we are under-estimating tax, we tax the total
			if(bOverestimateTax) {
				//calculate tax first			
				COleCurrency cyTotalTax1 = CalculateTax(cyChargeNonTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargeNonTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargeNonTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;

				//round after tax
				RoundCurrency(cyChargeTaxTotal);

				//now multiply the tax total by the quantity
				cyChargeTaxTotal = CalculateAmtQuantity(cyChargeTaxTotal,dblQuantity);

				//round after quantity
				RoundCurrency(cyChargeTaxTotal);

				cy += cyChargeTaxTotal;
			}
			else {
				//calculate quantity first
				COleCurrency cyChargePreTaxTotal = CalculateAmtQuantity(cyChargeNonTaxTotal,dblQuantity);

				//round after quantity
				RoundCurrency(cyChargePreTaxTotal);

				COleCurrency cyTotalTax1 = CalculateTax(cyChargePreTaxTotal,dblTax1);
				COleCurrency cyTotalTax2 = CalculateTax(cyChargePreTaxTotal,dblTax2);

				COleCurrency cyChargeTaxTotal = cyChargePreTaxTotal;

				cyChargeTaxTotal += cyTotalTax1;
				cyChargeTaxTotal += cyTotalTax2;

				//round after tax
				RoundCurrency(cyChargeTaxTotal);				

				cy += cyChargeTaxTotal;
			}

			rs->MoveNext();
		}
		rs->Close();

		if(cy < COleCurrency(0,0))
			cy = COleCurrency(0,0);
	}

	return cy;
}

// (j.jones 2007-09-18 12:13) - PLID 25287 - calculate any "overage" that may be required
// for a multi-use package to balance out with the package total
COleCurrency CalculateMultiUsePackageOverageAmount_ForTotal(long nQuoteID)
{			
	COleCurrency cyMatchRemainingPackageAmount = COleCurrency(0,0);

	COleCurrency cyNonTaxTotal = COleCurrency(0,0);

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted this recordset to be parameterized

	// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts

	_RecordsetPtr rs = CreateParamRecordset("SELECT "
		"PackagesT.TotalAmount, "
		// (j.jones 2007-07-03 10:29) - PLID 26539 - handled divide by zero for 0-quantities
		"(CASE WHEN OriginalPackageQtyRemaining = 0 THEN Convert(money,0) ELSE "
		//handle a zero dollar bill
		"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(TotalAmount/NumCharges/OriginalPackageQtyRemaining)),2) ELSE "
		//calculate using the percentage of the total
		"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / OriginalPackageQtyRemaining)),2) END) "
		"END) "
		"AS PracBillFee, "
		""
		"ChargesT.OriginalPackageQtyRemaining AS Quantity, ChargesT.PackageQtyRemaining "
		""
		"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
		""
		//get the non tax charge total
		// (j.gruber 2009-03-18 09:39) - PLID 33360 - update discount structure
		"Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
		""
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"WHERE LineItemT.Deleted = 0) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
		"INNER JOIN (SELECT PackagesT.*, TotalAmount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
		"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
		""
		//get the non tax bill total
		// (j.gruber 2009-03-18 09:40) - PLID 333360 - updated discount structure
		"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
		""
		"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
		"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
		"WHERE PatientChargesQ.BillID = {INT} AND PackagesT.Type = 2 ORDER BY ChargesT.LineID ", nQuoteID, nQuoteID);

	while(!rs->eof) {
		COleCurrency cyChargeNonTaxTotal = AdoFldCurrency(rs, "PracBillFee");
		double dblQuantity = AdoFldDouble(rs, "Quantity",0.0);
		
		if(dblQuantity <= 0.0) {
			rs->MoveNext();
			continue;
		}

		COleCurrency cyTotalAmount = AdoFldCurrency(rs, "TotalAmount");

		rs->MoveNext();

		cyChargeNonTaxTotal = CalculateAmtQuantity(cyChargeNonTaxTotal,dblQuantity);

		//if we are now eof, see if the last charge will need to bring the pre-tax total up to the total package amount
		if(rs->eof && cyNonTaxTotal + cyChargeNonTaxTotal != cyTotalAmount) {
			//indeed, we would be less than the total package amount, so calculate the overage
			cyMatchRemainingPackageAmount = cyTotalAmount - cyNonTaxTotal - cyChargeNonTaxTotal;
			continue;
		}
		
		//track the non tax total
		cyNonTaxTotal += cyChargeNonTaxTotal;
	}
	rs->Close();

	return cyMatchRemainingPackageAmount;
}

// (j.jones 2007-03-26 14:47) - PLID 25287 - calculate any "overage" that may be required
// for a multi-use package to balance out with the package balance
COleCurrency CalculateMultiUsePackageOverageAmount_ForBalance(long nQuoteID)
{			
	COleCurrency cyMatchRemainingPackageAmount = COleCurrency(0,0);

	COleCurrency cyNonTaxTotal = COleCurrency(0,0);

	// (j.jones 2007-04-23 12:51) - PLID 25735 - any references to TotalAmount and TotalCount should
	// instead use OriginalCurrentCount and OriginalCurrentAmount, since we're truly basing our calculations
	// off of that information

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted this recordset to be parameterized

	// (j.jones 2008-05-30 12:02) - PLID 28898 - ensured we properly handle outside fees with discounts
	// (j.gruber 2009-03-16 12:39) - PLID 33360 - convert to new discount structure

	_RecordsetPtr rs = CreateParamRecordset("SELECT "
		"PackagesT.OriginalCurrentAmount, "
		// (j.jones 2007-07-03 10:29) - PLID 26539 - handled divide by zero for 0-quantities
		"(CASE WHEN OriginalPackageQtyRemaining = 0 THEN Convert(money,0) ELSE "
		//handle a zero dollar bill
		"(CASE WHEN BillNonTaxTotal = 0 THEN Round(Convert(money,(OriginalCurrentAmount/NumCharges/OriginalPackageQtyRemaining)),2) ELSE "
		//calculate using the percentage of the total
		"Round(Convert(money,((BillTotal * ChargeNonTaxTotal / BillNonTaxTotal) / OriginalPackageQtyRemaining)),2) END) "
		"END) "
		"AS PracBillFee, "
		""
		"ChargesT.OriginalPackageQtyRemaining AS Quantity, ChargesT.PackageQtyRemaining "
		""
		"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, "
		""
		//get the non tax charge total
		"Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2) AS ChargeNonTaxTotal "
		""
		"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"WHERE LineItemT.Deleted = 0) AS PatientChargesQ INNER JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID) "
		"INNER JOIN (SELECT PackagesT.*, OriginalCurrentAmount AS BillTotal FROM PackagesT) AS PackagesT ON PatientChargesQ.BillID = PackagesT.QuoteID "
		"INNER JOIN BillsT ON PatientChargesQ.BillID = BillsT.ID "
		""
		//get the non tax bill total
		"INNER JOIN (SELECT BillsT.ID, Sum(Round(Convert(money,((([Amount]*[OriginalPackageQtyRemaining]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN(CPTMultiplier4 Is Null) THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))))),2)) AS BillNonTaxTotal FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN (SELECT ChargeID, Sum(PercentOff) AS TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentOffQ ON ChargesT.ID = TotalPercentOffQ.ChargeID LEFT JOIN (SELECT ChargeID, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 GROUP BY BillsT.ID) AS BillTotalsQ ON BillsT.ID = BillTotalsQ.ID "
		""
		"INNER JOIN (SELECT Count(ChargesT.ID) AS NumCharges, ChargesT.BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"GROUP BY ChargesT.BillID, Convert(int,LineItemT.Deleted) "
		"HAVING ChargesT.BillID = {INT} AND Convert(int,LineItemT.Deleted) = 0) AS CountChargesQ ON ChargesT.BillID = CountChargesQ.BillID "
		"WHERE PatientChargesQ.BillID = {INT} AND PackagesT.Type = 2 ORDER BY ChargesT.LineID ", nQuoteID, nQuoteID);

	while(!rs->eof) {
		COleCurrency cyChargeNonTaxTotal = AdoFldCurrency(rs, "PracBillFee");
		double dblQuantity = AdoFldDouble(rs, "Quantity",0.0);
		
		if(dblQuantity <= 0.0) {
			rs->MoveNext();
			continue;
		}

		COleCurrency cyOriginalPackageAmount = AdoFldCurrency(rs, "OriginalCurrentAmount");

		rs->MoveNext();

		cyChargeNonTaxTotal = CalculateAmtQuantity(cyChargeNonTaxTotal,dblQuantity);

		//if we are now eof, see if the last charge will need to bring the pre-tax total up to the total package amount
		if(rs->eof && cyNonTaxTotal + cyChargeNonTaxTotal != cyOriginalPackageAmount) {
			//indeed, we would be less than the total package amount, so calculate the overage
			cyMatchRemainingPackageAmount = cyOriginalPackageAmount - cyNonTaxTotal - cyChargeNonTaxTotal;
			continue;
		}
		
		//track the non tax total
		cyNonTaxTotal += cyChargeNonTaxTotal;
	}
	rs->Close();

	return cyMatchRemainingPackageAmount;
}

// (j.jones 2007-03-26 14:47) - PLID 25287 - calculate the target ServiceID that an overage
// would be applied to
// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
long CalculateMultiUsePackageOverageChargeID(long nQuoteID)
{
	long nFirstChargeID = -1;

	// (j.jones 2007-08-09 14:35) - PLID 27029 - converted all recordsets in this function to be parameterized

	//We want to find the first charge in the quote that has a dollar amount,
	//OR just the first charge altogether if all charges are zero.
	
	//try to find the first non-zero charge
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ChargesT.ID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"WHERE Deleted = 0 AND Amount > Convert(money, '0.00') "
		"AND BillID = {INT} ORDER BY LineID", nQuoteID);
	if(!rs->eof) {
		//we found the first non-zero charge
		nFirstChargeID = AdoFldLong(rs, "ID",-1);
	}
	rs->Close();

	if(nFirstChargeID == -1) {

		//we didn't find a non-zero charge, now find the first charge altogether

		rs = CreateParamRecordset("SELECT TOP 1 ChargesT.ID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE Deleted = 0 AND BillID = {INT} ORDER BY LineID", nQuoteID);
		if(!rs->eof) {
			nFirstChargeID = AdoFldLong(rs, "ID",-1);
		}
		else {
			//should be impossible
			ASSERT(FALSE);
		}
		rs->Close();
	}

	return nFirstChargeID;
}

CString GetAnesthStartTimeFromBill(long nBillID) {

	CString strReturn = "";

	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthStartTime FROM BillsT WHERE ID = {INT}",nBillID);
	if(!rs->eof) {
		_variant_t var;

		CString strStart;
		var = rs->Fields->Item["AnesthStartTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			strReturn = dt.Format("%I:%M%p");
		}
		else {
			return "";
		}
	}
	rs->Close();

	return strReturn;
}

CString GetAnesthEndTimeFromBill(long nBillID) {

	CString strReturn = "";

	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthEndTime FROM BillsT WHERE ID = {INT}",nBillID);
	if(!rs->eof) {
		_variant_t var;

		CString strStart;
		var = rs->Fields->Item["AnesthEndTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			strReturn = dt.Format("%I:%M%p");
		}
		else {
			return "";
		}
	}
	rs->Close();

	return strReturn;
}

long GetAnesthMinutesFromBill(long nBillID) {

	CString strReturn = "";

	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthesiaMinutes FROM BillsT WHERE ID = {INT}",nBillID);
	if(!rs->eof) {
		return AdoFldLong(rs, "AnesthesiaMinutes", -1);
	}
	rs->Close();

	return -1;
}

CString CalculateAnesthesiaNoteForHCFA(long nBillID)
{
	// (j.jones 2006-09-18 16:14) - PLID 22455 - the new HCFA requires the anesthesia information
	// on the anesthesia charge's supplemental note line, formatted with a qualifier of 7,
	// the begin and end time, and then the minutes, but only show the minutes if they are different
	// from the quantity.

	//if the minutes are the units, report the data as:
	//7Begin 1245 End 1415
	//if the minutes are not the units (such as you have a calculated number of units) then you report:
	//7Begin 1245 End 1415 Time 90 minutes

	CString strAnesthStart = "", strAnesthEnd = "";

	// (j.jones 2009-10-07 08:49) - PLID 36426 - parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.AnesthStartTime, BillsT.AnesthEndTime, "
		"DateDiff(minute, AnesthStartTime, AnesthEndTime) AS Minutes, ChargesT.Quantity "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"WHERE BillsT.ID = {INT} AND ServiceT.Anesthesia <> 0 "
		"AND AnesthStartTime Is Not Null AND AnesthEndTime Is Not Null",nBillID);
	if(!rs->eof) {
		_variant_t var;

		CString strStart;
		var = rs->Fields->Item["AnesthStartTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			// (a.walling 2008-10-21 09:02) - PLID 31747 - Invalid format string %H%M% causes errors (trailing % is bad!)
			strAnesthStart = dt.Format("%H%M");
		}
		else {
			return "";
		}

		var = rs->Fields->Item["AnesthEndTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			// (a.walling 2008-10-21 09:02) - PLID 31747 - Invalid format string %H%M% causes errors (trailing % is bad!)
			strAnesthEnd = dt.Format("%H%M");
		}
		else {
			return "";
		}

		double dblQuantity = AdoFldDouble(rs, "Quantity", 0.0);
		long nMinutes = AdoFldLong(rs, "Minutes", 0);

		//if the quantity = minutes, don't show the time
		if((long)dblQuantity == nMinutes) {
			CString strFormat;
			strFormat.Format("7Begin %s End %s", strAnesthStart, strAnesthEnd);
			return strFormat;
		}
		else {
			CString strFormat;
			strFormat.Format("7Begin %s End %s Time %li minutes", strAnesthStart, strAnesthEnd, nMinutes);
			return strFormat;
		}
	}
	rs->Close();

	return "";
}

// (j.jones 2011-07-06 13:59) - PLID 44327 - added CalculateAnesthesiaNoteForANSI
CString CalculateAnesthesiaNoteForANSI(long nBillID)
{
	//Output Anesthesia times as:
	//START 0856 STOP 0913

	CString strAnesthStart = "", strAnesthEnd = "";

	_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.AnesthStartTime, BillsT.AnesthEndTime "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"WHERE BillsT.ID = {INT} AND ServiceT.Anesthesia <> 0 "
		"AND AnesthStartTime Is Not Null AND AnesthEndTime Is Not Null", nBillID);
	if(!rs->eof) {
		_variant_t var;

		CString strStart;
		var = rs->Fields->Item["AnesthStartTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			strAnesthStart = dt.Format("%H%M");
		}
		else {
			return "";
		}

		var = rs->Fields->Item["AnesthEndTime"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = VarDateTime(var);
			strAnesthEnd = dt.Format("%H%M");
		}
		else {
			return "";
		}

		if(!strAnesthStart.IsEmpty() && !strAnesthEnd.IsEmpty()) {
			CString strFormat;
			strFormat.Format("START %s STOP %s", strAnesthStart, strAnesthEnd);
			return strFormat;
		}
	}
	rs->Close();

	return "";
}

//(e.lally 2007-10-30) PLID 27892 - Added global function to change credit card numbers into 
// XXXXXXXXXXXX#### format where only the last 4 digits are displayed.
//(e.lally 2007-12-11) PLID 28325 - Added in check for credit card processing license and hidden preference to
	//turn the masking off.
CString MaskCCNumber(IN const CString& strFullCCNumber)
{
	long nUseCCMasking = GetRemotePropertyInt("MaskCreditCardNumbers", 1, 0, "<None>", true);
	// (d.thompson 2010-09-02) - PLID 40371 - Applies to any processing method
	BOOL bHasCCProcessingLicense = g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent);

	//ONLY ALLOW THE FULL CC NUMBER IF THEY DO NOT HAVE THE CREDIT CARD PROCESSING LICENSE
	if(nUseCCMasking != 1 && !bHasCCProcessingLicense){
		//No cc processing license, and we changed the preference from the default. 
		//Allow the full number to be returned
		return strFullCCNumber;
	}

	CString strMaskedCCNumber;
	long nLength=strFullCCNumber.GetLength();

	//Let's not bother if the string is too short to begin with
	if(nLength < 4){
		//return what was sent in
		return strFullCCNumber;
	}
	
	//Buffer our string with X's for each digit but the last 4
	for(int i=0; i<nLength-4; i++){
		strMaskedCCNumber+="X";
	}
	
	//Append our last 4 digits
	strMaskedCCNumber += strFullCCNumber.Right(4);

	return strMaskedCCNumber;
}

// (j.jones 2008-02-11 15:26) - PLID 28847 - added function to check whether we can create a claim on a bill
// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created, and takes in
// bSilent to disable that warning
BOOL CanCreateInsuranceClaim(long nBillID, BOOL bSilent)
{
	if(GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1) {

		// (j.jones 2011-09-13 15:37) - PLID 44887 - ignore original/void charges
		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(Coalesce(ChargeRespT.Amount, 0)) AS SumOfInsResp "
			"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 "
			"AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > -1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillsT.ID = {INT} "
			"GROUP BY BillsT.ID, LineItemT.Deleted, BillsT.Deleted "
			"HAVING Sum(Coalesce(ChargeRespT.Amount, 0)) > 0", nBillID);
		if(rs->eof) {
			//warn unless told not to
			if(!bSilent) {
				AfxMessageBox("This claim cannot be opened or batched because your preference to disallow batching or printing "
					"claims with no insurance responsibility has been enabled.");
			}
			return FALSE;
		}
		rs->Close();

		// (j.jones 2008-02-12 11:00) - PLID 28848 - if we can batch it, check the preference
		// whether patient-only charges should not be batched, and update their status here
		EnsurePatientChargesUnbatched(nBillID);
	}

	// (j.jones 2010-10-21 14:51) - PLID 41051 - if we are still here, simply warn if no claims are batched
	// (j.jones 2011-09-13 15:37) - PLID 44887 - ignore original/void charges
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ChargesT.ID  "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 "
		"AND ChargesT.Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND BillsT.ID = {INT}", nBillID);
	if(rs->eof) {
		//warn unless told not to
		if(!bSilent) {
			AfxMessageBox("This claim cannot be opened or batched because no charges in the bill are marked as batched.");
		}
		return FALSE;
	}
	rs->Close();

	return TRUE;
}

// (j.jones 2008-02-12 09:04) - PLID 28848 - added functions to ensure that patient-only charges are not batched
void EnsurePatientChargesUnbatched(long nBillID)
{
	//require both preferences to be enabled for this to go into effect
	if(GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1 &&
		GetRemotePropertyInt("HidePatientChargesOnClaims",0,0,"<None>",TRUE) == 1) {

		//given a bill ID, ensure that each charge with $0.00 insurance resp is not batched
		
		CArray<long, long> aryChargeIDs;

		// (j.jones 2010-09-29 16:27) - PLID 40686 - skip charges that are configured to batch if zero
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID, LineItemT.PatientID, "
			"Last + ', ' + First + ' ' + Middle AS PatName, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, LineItemT.Date "
			"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND BillsT.ID = {INT} "
			"AND (CPTCodeT.ID Is Null OR CPTCodeT.BatchIfZero = 0 OR dbo.GetChargeTotal(ChargesT.ID) <> Convert(money, 0)) "
			"GROUP BY ChargesT.ID, ChargesT.Batched, LineItemT.Deleted, BillsT.Deleted, "
			"LineItemT.PatientID, Last + ', ' + First + ' ' + Middle, LineItemT.Date "
			"HAVING Sum(Coalesce(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > -1 "
			"		THEN ChargeRespT.Amount ELSE 0 END, 0)) = 0 "
			"AND Batched = 1", nBillID);
		
		long nCount = rs->GetRecordCount();
		if(nCount > 0) {

			long nAuditTransactionID = -1;

			try {

				CArray<long, long> aryChargesToUnbatch;

				while(!rs->eof) {

					long nChargeID = VarLong(rs->Fields->Item["ID"]->Value);
					aryChargesToUnbatch.Add(nChargeID);

					long nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
					CString strPatientName = VarString(rs->Fields->Item["PatName"]->Value);
					COleCurrency cyChargeTotal = VarCurrency(rs->Fields->Item["ChargeTotal"]->Value);
					COleDateTime dtDate = VarDateTime(rs->Fields->Item["Date"]->Value);

					// (j.jones 2011-08-25 16:56) - PLID 44796 - audit the batched status
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldValue;
					strOldValue.Format("Batched (%s charge, %s)", FormatCurrencyForInterface(cyChargeTotal), FormatDateTimeForInterface(dtDate, NULL, dtoDate));
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiChargeBatched, nChargeID, strOldValue, "Unbatched due to zero insurance. resp.", aepMedium, aetChanged);

					rs->MoveNext();
				}
				rs->Close();

				ExecuteParamSql("UPDATE ChargesT SET Batched = 0 WHERE ID IN ({INTARRAY})", aryChargesToUnbatch);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}

			}NxCatchAllCall(__FUNCTION__,
				if (nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
			);
		}
	}
}

// (j.jones 2008-02-12 15:37) - PLID 28848 - added function to ensure that patient-only charges are batched when
// shifted to insurance
void EnsurePatientChargesBatchedUponShift(long nChargeID)
{
	//require both preferences to be enabled for this to go into effect
	if(GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1 &&
		GetRemotePropertyInt("HidePatientChargesOnClaims",0,0,"<None>",TRUE) == 1) {

		//given a charge ID, batch it, provided that it currently has $0.00 insurance resp
		//and is already unbatched

		//this will only be called upon shifting to insurance

		//batch the charge if currently unbatched and it has a $0.00 insurance resp
		
		// (j.jones 2011-08-25 17:04) - PLID 44796 - we now audit the batched status, which means
		// we need to iterate through each charge that we are changing
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID, LineItemT.PatientID, "
			"Last + ', ' + First + ' ' + Middle AS PatName, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, LineItemT.Date "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT} "
			"GROUP BY ChargesT.ID, ChargesT.Batched, LineItemT.Deleted, "
			"LineItemT.PatientID, Last + ', ' + First + ' ' + Middle, LineItemT.Date "
			"HAVING Sum(Coalesce(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > -1 "
			"		THEN ChargeRespT.Amount ELSE 0 END, 0)) = 0 "
			"AND Batched = 0 AND ChargesT.ID = {INT}", nChargeID, nChargeID);

		long nCount = rs->GetRecordCount();
		if(nCount > 0) {

			long nAuditTransactionID = -1;

			try {

				CArray<long, long> aryChargesToBatch;

				while(!rs->eof) {

					long nChargeID = VarLong(rs->Fields->Item["ID"]->Value);
					aryChargesToBatch.Add(nChargeID);

					long nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
					CString strPatientName = VarString(rs->Fields->Item["PatName"]->Value);
					COleCurrency cyChargeTotal = VarCurrency(rs->Fields->Item["ChargeTotal"]->Value);
					COleDateTime dtDate = VarDateTime(rs->Fields->Item["Date"]->Value);

					// (j.jones 2011-08-25 16:56) - PLID 44796 - audit the batched status
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldValue;
					strOldValue.Format("Unbatched (%s charge, %s)", FormatCurrencyForInterface(cyChargeTotal), FormatDateTimeForInterface(dtDate, NULL, dtoDate));
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiChargeBatched, nChargeID, strOldValue, "Batched due to shifting to insurance. resp.", aepMedium, aetChanged);

					rs->MoveNext();
				}
				rs->Close();

				ExecuteParamSql("UPDATE ChargesT SET Batched = 1 WHERE ID IN ({INTARRAY})", aryChargesToBatch);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}

			}NxCatchAllCall(__FUNCTION__,
				if (nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
			);
		}
	}
}

// (j.jones 2008-05-27 16:17) - PLID 27982 - this function should be called before
// deleting a bill or charge, and will warn if the bill or charge has serialized items,
// returns true/false if deletion should continue
// (j.jones 2013-07-23 10:01) - PLID 46493 - added flag for voiding, only needed to be used
// if we're only voiding a bill or charge, it is not needed if we're voiding & correcting
bool CheckWarnDeletingChargedProductItems(bool bIsBill, bool bIsVoiding, long nID)
{
	try {

		if(nID <= -1) {
			//must be a new charge or bill, skip it
			return true;
		}

		bool bHasProductItems = false;
		CString strWarn = "";

		_RecordsetPtr rs;
		
		if(bIsBill) {
			rs = CreateParamRecordset("SELECT ServiceT.Name, ProductItemsT.SerialNum, "
				"ProductItemsT.ExpDate, ProductItemsT.Status "
				"FROM ChargedProductItemsT "
				"INNER JOIN ProductItemsT ON ChargedProductItemsT.ProductItemID = ProductItemsT.ID "
				"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} ", nID);

			if(!rs->eof) {
				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn = "This bill references the following products with serial numbers and/or expiration dates that will be put back into stock if this bill is voided:\n\n";
				}
				else {
					//we're deleting
					strWarn = "This bill references the following products with serial numbers and/or expiration dates that will be put back into stock if this bill is deleted:\n\n";
				}
				bHasProductItems = true;
			}
		}
		else {
			rs = CreateParamRecordset("SELECT ServiceT.Name, ProductItemsT.SerialNum, "
				"ProductItemsT.ExpDate, ProductItemsT.Status "
				"FROM ChargedProductItemsT "
				"INNER JOIN ProductItemsT ON ChargedProductItemsT.ProductItemID = ProductItemsT.ID "
				"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT} ", nID);

			if(!rs->eof) {
				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn = "This charge references the following products with serial numbers and/or expiration dates that will be put back into stock if this charge is voided:\n\n";
				}
				else {
					//we're deleting
					strWarn = "This charge references the following products with serial numbers and/or expiration dates that will be put back into stock if this charge is deleted:\n\n";
				}
				bHasProductItems = true;
			}
		}

		if(!bHasProductItems) {
			//no product items, so continue normally
			return true;
		}
		else {

			bool bHasConsignment = false;

			while(!rs->eof) {

				CString strProductName = AdoFldString(rs, "Name","");
				CString strSerialNum = AdoFldString(rs, "SerialNum","");
				CString strExpDate = "";
				if(rs->Fields->Item["ExpDate"]->Value.vt == VT_DATE) {
					strExpDate = FormatDateTimeForInterface(AdoFldDateTime(rs, "ExpDate"), NULL, dtoDate);
				}
				InvUtils::ProductItemStatus pisStatus = (InvUtils::ProductItemStatus)(AdoFldLong(rs, "Status"));

				CString strWarnAdd = strProductName;
				if(pisStatus == InvUtils::pisConsignment) {
					bHasConsignment = TRUE;
					strWarnAdd += " (Consignment)";
				}
				if(!strSerialNum.IsEmpty()) {
					CString strAdd;
					strAdd.Format(", Serial Number: %s", strSerialNum);
					strWarnAdd += strAdd;
				}
				if(!strExpDate.IsEmpty()) {
					CString strAdd;
					strAdd.Format(", Exp. Date: %s", strExpDate);
					strWarnAdd += strAdd;
				}

				strWarnAdd += "\n";

				strWarn += strWarnAdd;
				
				rs->MoveNext();
			}

			rs->Close();

			if(bIsBill) {				
				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn += "\nVoiding this bill will return these products back into stock.";
				}
				else {
					//we're deleting
					strWarn += "\nDeleting this bill will return these products back into stock.";
				}

				if(bHasConsignment) {
					strWarn += " Your consignment usage statistics will be changed to reflect this.";
				}

				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn += "\nAre you absolutely sure you wish to void this bill?";
				}
				else {
					//we're deleting
					strWarn += "\nAre you absolutely sure you wish to delete this bill?";
				}
			}
			else {
				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn += "\nVoiding this charge will return these products back into stock.";
				}
				else {
					//we're deleting
					strWarn += "\nDeleting this charge will return these products back into stock.";
				}

				if(bHasConsignment) {
					strWarn += " Your consignment usage statistics will be changed to reflect this.";
				}

				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting
				if(bIsVoiding) {
					strWarn += "\nAre you absolutely sure you wish to void this charge?";
				}
				else {
					//we're deleting
					strWarn += "\nAre you absolutely sure you wish to delete this charge?";
				}
			}

			if(IDYES == MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return true;
			}
			else {
				return false;
			}
		}

		return true;

	}NxCatchAll("Error in CheckWarnDeletingChargedProductItems");

	return false;
}

// (j.jones 2008-06-09 16:32) - PLID 28392 - This function should be called before
// deleting a bill or charge, and will warn if the bill or charge has products that
// have been returned. Returns true/false if the user wishes to continue.
// If bIsEditingCharge is true, it means they are trying to edit a quantity or serialized item,
// and not deleting, so the warnings need tweaked accordingly
// (j.jones 2013-07-23 10:01) - PLID 46493 - added flag for voiding, only needed to be used
// if we're only voiding a bill or charge, it is not needed if we're voiding & correcting
bool CheckWarnAlteringReturnedProducts(bool bIsBill, bool bIsVoiding, long nID, bool bIsEditingCharge /*= false*/)
{
	try {

		if(nID <= -1) {
			//must be a new charge or bill, skip it
			return true;
		}

		//ensure this wasn't called with bad parameters
		if(bIsBill) {
			ASSERT(!bIsEditingCharge);
		}

		BOOL bHasReturnedProducts = FALSE;
		CString strWarn = "";

		_RecordsetPtr rs;
		
		if(bIsBill) {
			rs = CreateParamRecordset("SELECT ServiceT.Name, Sum(ReturnedProductsT.QtyReturned) AS AmtReturned "
				"FROM ReturnedProductsT "
				"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
				"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} "
				"GROUP BY ServiceT.ID, ServiceT.Name ", nID);

			if(!rs->eof) {
				
				strWarn = "This bill references the following products that have previously been returned:\n\n";
				bHasReturnedProducts = TRUE;
			}
		}
		else {
			rs = CreateParamRecordset("SELECT ServiceT.Name, ReturnedProductsT.QtyReturned AS AmtReturned "
				"FROM ReturnedProductsT "
				"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
				"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT} ", nID);

			if(!rs->eof) {

				strWarn = "This charge references the following products that have previously been returned:\n\n";
				bHasReturnedProducts = TRUE;
			}
		}

		if(!bHasReturnedProducts) {
			//no returned products, so continue normally
			return true;
		}
		else {

			while(!rs->eof) {

				CString strProductName = AdoFldString(rs, "Name","");
				double dblAmtReturned = AdoFldDouble(rs, "AmtReturned", 0.0);
				CString str;
				str.Format("%s (Quantity returned: %g)\n", strProductName, dblAmtReturned);
				strWarn += str;
				
				rs->MoveNext();
			}
			rs->Close();

			// (j.jones 2015-11-04 09:30) - PLID 67459 - we now disallow voiding and deleting,
			// although void and corrects are allowed
			if(bIsBill) {	
				// (j.jones 2013-07-23 10:14) - PLID 46493 - change the message if we're voiding, not deleting				
				if(bIsVoiding) {
					strWarn += "\nThis bill cannot be Voided. You can, however, Void & Correct this bill.";
				}
				else {
					//we're deleting
					strWarn += "\nThis bill cannot be deleted.";
				}
			}
			else {
				if(!bIsEditingCharge) {
					if(bIsVoiding) {
						strWarn += "\nThis charge cannot be Voided. You can, however, Void & Correct this charge.";
					}
					else {
						//we're deleting
						strWarn += "\nThis charge cannot be deleted.";
					}
				}
				else {
					// (j.jones 2013-07-23 10:14) - PLID 46493 - should be impossible for editing & voiding to both be tru
					ASSERT(bIsVoiding == false);

					strWarn += "\nEditing this charge would affect your amount in stock and "
						"intefere with the product return information. "
						"For this reason, you may not edit this charge's quantity.";
				}
			}

			MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONINFORMATION | MB_OK);
			return false;
		}

		return true;

	}NxCatchAll("Error in CheckWarnAlteringReturnedProducts");

	return false;
}

// (j.jones 2008-06-24 09:20) - PLID 30455 - this function returns a query that is used
// both in the bill's appointment dropdown, and also used to validate that a given appointment
// will exist in that dropdown
// (j.gruber 2010-07-20 15:48) - PLID 39739 - we can now bill non-procedural appt types
CString GetBillAppointmentQuery()
{
	return "SELECT * FROM ( "
		"SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.Date, "
		"AppointmentsT.StartTime, AppointmentsT.LocationID, "
		"dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
		"CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) <> '' THEN AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) ELSE AptTypeT.Name END AS Description, "
		"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS Color "
		"FROM AppointmentsT "
		"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		//look at only non-cancelled, non-no-show appointments
		"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 "
		//must have a procedural appointment type
		// (j.jones 2011-07-22 13:10) - PLID 42059 - Other Procedure appts. now use the procedure's codes, not the appt type codes
		"AND AptTypeT.Category IN (3,4,6) "
		//must have a purpose that has a service linked to it
		"AND AppointmentsT.ID IN "
		"	(SELECT AppointmentID FROM AppointmentPurposeT "
		"	WHERE PurposeID IN (SELECT ProcedureID FROM ServiceT WHERE Active = 1 AND ProcedureID Is Not Null)) "
		" UNION "
		"SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.Date, "
		"AppointmentsT.StartTime, AppointmentsT.LocationID, "
		"dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
		"CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) <> '' THEN AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) ELSE AptTypeT.Name END AS Description, "
		"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS Color "
		"FROM AppointmentsT "
		"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		//the type must be linked to a service
		"INNER JOIN ApptTypeServiceLinkT ON AptTypeT.ID = ApptTypeServiceLinkT.AptTypeID "
		//look at only non-cancelled, non-no-show appointments
		"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 "
		//must have a non-procedural appointment type
		// (j.jones 2011-07-22 13:10) - PLID 42059 - Other Procedure appts. now use the procedure's codes, not the appt type codes
		"AND AptTypeT.Category NOT IN (3,4,6) )Q WHERE (1=1) ";
		
		

}

// (j.jones 2008-09-09 09:08) - PLID 18695 - this function serves as a global lookup
// for the proper ANSI code for a given InsuranceTypeCode
// (j.jones 2010-10-15 14:29) - PLID 40953 - split into 4010 and 5010 usage
CString GetANSI4010_SBR09CodeFromInsuranceType(InsuranceTypeCode eCode)
{
	switch(eCode) {

		///***If you ever add a new case to this function, make sure it is reflected in the GlobalFinancialUtils
		//enum for InsuranceTypeCode, the function GetNameFromInsuranceType(), the EditInsInfo Insurance Type List,
		//and the AdvPayerIDDlg list

		case itcSelfPay:
			return "09";
		case itcCentralCertification:
			return "10";
		case itcOtherNonFederalPrograms:
			return "11";
		case itcPPO:
			return "12";
		case itcPOS:
			return "13";
		case itcEPO:
			return "14";
		case itcIndemnityInsurance:
			return "15";
		case itcHMO_MedicareRisk:
			return "16";
		case itcAutomobileMedical:
			return "AM";
		case itcBCBS:
			return "BL";
		case itcChampus:
			return "CH";
		case itcCommercial:
		case itcCommercialSupplemental:	// (j.jones 2009-03-27 17:22) - PLID 33724 - same as commercial in SBR09
			return "CI";
		case itcDisability:
			return "DS";
		case itcHMO:
			return "HM";
		case itcLiability:
			return "LI";
		case itcLiabilityMedical:
			return "LM";
		case itcMedicarePartB:
			return "MB";
		case itcMedicaid:
			return "MC";
		case itcOtherFederalProgram:
			return "OF";
		case itcTitleV:
			return "TV";
		case itcVeteranAdministrationPlan:
			return "VA";
		case itcWorkersComp:
			return "WC";
		case itcOther:
		//these are new for 5010, not supported in 4010
		case itcDentalMaintenanceOrganization:
		case itcFederalEmployeesProgram:
		case itcMedicarePartA:
		// (j.jones 2013-11-13 12:52) - PLID 58931 - added HRSA Other, for MU purposes
		case itcHRSAOther:
		default:
			return "ZZ";
	}
};

// (j.jones 2010-10-15 14:29) - PLID 40953 - split into 4010 and 5010 usage
CString GetANSI5010_SBR09CodeFromInsuranceType(InsuranceTypeCode eCode)
{
	switch(eCode) {

		///***If you ever add a new case to this function, make sure it is reflected in the GlobalFinancialUtils
		//enum for InsuranceTypeCode, the function GetNameFromInsuranceType(), the EditInsInfo Insurance Type List,
		//and the AdvPayerIDDlg list

		case itcOtherNonFederalPrograms:
			return "11";
		case itcPPO:
			return "12";
		case itcPOS:
			return "13";
		case itcEPO:
			return "14";
		case itcIndemnityInsurance:
			return "15";
		case itcHMO_MedicareRisk:
			return "16";
		//new for 5010
		case itcDentalMaintenanceOrganization:
			return "17";
		case itcAutomobileMedical:
			return "AM";
		case itcBCBS:
			return "BL";
		case itcChampus:
			return "CH";
		case itcCommercial:
		case itcCommercialSupplemental:	// (j.jones 2009-03-27 17:22) - PLID 33724 - same as commercial in SBR09
			return "CI";
		case itcDisability:
			return "DS";
		//new for 5010
		case itcFederalEmployeesProgram:
			return "FI";
		case itcHMO:
			return "HM";		
		case itcLiabilityMedical:
			return "LM";
		//new for 5010
		case itcMedicarePartA:
			return "MA";
		case itcMedicarePartB:
			return "MB";
		case itcMedicaid:
			return "MC";
		case itcOtherFederalProgram: //also used if Medicare Part D
			return "OF";
		case itcTitleV:
			return "TV";
		case itcVeteranAdministrationPlan:
			return "VA";
		case itcWorkersComp:
			return "WC";
		case itcOther:
		//these codes are no longer valid in 5010
		case itcSelfPay:
		case itcCentralCertification:
		case itcLiability:
		// (j.jones 2013-11-13 12:52) - PLID 58931 - added HRSA Other, for MU purposes
		case itcHRSAOther:
		default:
			return "ZZ";
	}
};

// (j.jones 2008-09-09 10:20) - PLID 18695 - this function serves as a global lookup
// for the proper NSF code for a given InsuranceTypeCode
CString GetNSFCodeFromInsuranceType(InsuranceTypeCode eCode)
{
	switch(eCode) {

		///NSF is obsolete, so this function is barely necessary, except for THIN
		//usage in Ebilling::ANSI_2010BB and the Edit Ins List display.
		//You should never need to add to this function.

		case itcSelfPay:
			return "A";
		case itcPPO:
			return "X";
		case itcBCBS:
			return "G";
		case itcChampus:
			return "H";
		case itcCommercial:
		case itcCommercialSupplemental:	// (j.jones 2009-03-27 17:22) - PLID 33724 - same as commercial in NSF
			return "F";
		case itcHMO:
			return "I";
		case itcMedicarePartB:
			return "C";
		case itcMedicaid:
			return "D";
		case itcOtherFederalProgram:
			return "E";
		case itcTitleV:
			return "T";
		case itcVeteranAdministrationPlan:
			return "V";
		case itcWorkersComp:
			return "B";
		case itcOther:		
		default:
			return "Z";
	}
};

// (r.gonet 12/03/2012) - PLID 53798 - Moved GetNameFromInsuranceType to NxPracticeSharedLib/SharedInsuranceUtils.cpp

// (j.jones 2009-01-16 12:24) - PLID 32762 - added GetANSI_2320SBR05_CodeFromInsuranceType
// (j.jones 2010-10-15 15:12) - PLID 40953 - renamed to reflect that this is 4010-only
// because it has a different code list than the SBR09 codes
CString GetANSI4010_2320SBR05_CodeFromInsuranceType(InsuranceTypeCode eCode)
{
	switch(eCode) {

		///***If you ever add a new case to this function, make sure it is reflected in the GlobalFinancialUtils
		//enum for InsuranceTypeCode, the function GetNameFromInsuranceType(), the EditInsInfo Insurance Type List,
		//and the AdvPayerIDDlg list

		case itcSelfPay:
			return "PP";		
		case itcHMO:
		case itcHMO_MedicareRisk:
			return "HM";
		case itcAutomobileMedical:
			return "AP";
		case itcBCBS:
		case itcCommercial:
			return "C1";
		// (j.jones 2009-03-27 17:22) - PLID 33724 - supported supplemental insurance
		case itcCommercialSupplemental:
			return "SP";
		case itcMedicarePartB:
			return "MB";
		case itcMedicaid:
			return "MC";
		case itcOther:
		case itcCentralCertification:
		case itcOtherNonFederalPrograms:
		case itcPPO:
		case itcPOS:
		case itcEPO:
		case itcIndemnityInsurance:
		case itcChampus:
		case itcDisability:
		case itcLiability:
		case itcLiabilityMedical:		
		case itcOtherFederalProgram:
		case itcTitleV:
		case itcVeteranAdministrationPlan:
		case itcWorkersComp:
		// (j.jones 2013-11-13 12:52) - PLID 58931 - added HRSA Other, for MU purposes
		case itcHRSAOther:
		default:
			return "OT";
	}
}

// (j.jones 2009-09-16 12:44) - PLID 26481 - moved from CEligibilityRequestDlg so we could build
// the same content in multiple locations
// (j.jones 2013-03-28 15:25) - PLID 52182 - this is now in data, and thus obsolete
/*
void BuildEligibilityBenefitCategoryCombo(NXDATALIST2Lib::_DNxDataListPtr &pCategoryCombo,
										  int nCodeColumn, int nCategoryColumn)
{
	//throw exceptions to the caller

	//build this insanely long combo
	AddToEligibilityBenefitCategoryCombo("1", "Medical Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("2", "Surgical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("3", "Consultation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("4", "Diagnostic X-Ray", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("5", "Diagnostic Lab", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("6", "Radiation Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("7", "Anesthesia", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("8", "Surgical Assistance", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("9", "Other Medical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("10", "Blood Charges", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("11", "Used Durable Medical Equipment", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("12", "Durable Medical Equipment Purchase", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("13", "Ambulatory Service Center Facility", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("14", "Renal Supplies in the Home", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("15", "Alternate Method Dialysis", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("16", "Chronic Renal Disease (CRD) Equipment", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("17", "Pre-Admission Testing", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("18", "Durable Medical Equipment Rental", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("19", "Pneumonia Vaccine", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("20", "Second Surgical Opinion", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("21", "Third Surgical Opinion", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("22", "Social Work", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("23", "Diagnostic Dental", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("24", "Periodontics", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("25", "Restorative", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("26", "Endodontics", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("27", "Maxillofacial Prosthetics", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("28", "Adjunctive Dental Services", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("30", "Health Benefit Plan Coverage", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("32", "Plan Waiting Period", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("33", "Chiropractic", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("34", "Chiropractic Office Visits", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("35", "Dental Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("36", "Dental Crowns", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("37", "Dental Accident", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("38", "Orthodontics", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("39", "Prosthodontics", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("40", "Oral Surgery", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("41", "Routine (Preventive) Dental", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("42", "Home Health Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("43", "Home Health Prescriptions", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("44", "Home Health Visits", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("45", "Hospice", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("46", "Respite Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("47", "Hospital", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("48", "Hospital - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("49", "Hospital - Room and Board", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("50", "Hospital - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("51", "Hospital - Emergency Accident", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("52", "Hospital - Emergency Medical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("53", "Hospital - Ambulatory Surgical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("54", "Long Term Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("55", "Major Medical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("56", "Medically Related Transportation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("57", "Air Transportation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("58", "Cabulance", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("59", "Licensed Ambulance", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("60", "General Benefits", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("61", "In-vitro Fertilization", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("62", "MRI/CAT Scan", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("63", "Donor Procedures", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("64", "Acupuncture", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("65", "Newborn Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("66", "Pathology", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("67", "Smoking Cessation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("68", "Well Baby Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("69", "Maternity", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("70", "Transplants", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("71", "Audiology Exam", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("72", "Inhalation Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("73", "Diagnostic Medical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("74", "Private Duty Nursing", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("75", "Prosthetic Device", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("76", "Dialysis", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("77", "Otological Exam", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("78", "Chemotherapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("79", "Allergy Testing", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("80", "Immunizations", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("81", "Routine Physical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("82", "Family Planning", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("83", "Infertility", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("84", "Abortion", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("85", "AIDS", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("86", "Emergency Services", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("87", "Cancer", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("88", "Pharmacy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("89", "Free Standing Prescription Drug", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("90", "Mail Order Prescription Drug", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("91", "Brand Name Prescription Drug", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("92", "Generic Prescription Drug", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("93", "Podiatry", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("94", "Podiatry - Office Visits", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("95", "Podiatry - Nursing Home Visits", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("96", "Professional (Physician)", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("97", "Anesthesiologist", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("98", "Professional (Physician) Visit - Office", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("99", "Professional (Physician) Visit - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A0", "Professional (Physician) Visit - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A1", "Professional (Physician) Visit - Nursing Home", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A2", "Professional (Physician) Visit - Skilled Nursing Facility", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A3", "Professional (Physician) Visit - Home", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A4", "Psychiatric", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A5", "Psychiatric - Room and Board", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A6", "Psychotherapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A7", "Psychiatric - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A8", "Psychiatric - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("A9", "Rehabilitation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AA", "Rehabilitation - Room and Board", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AB", "Rehabilitation - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AC", "Rehabilitation - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AD", "Occupational Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AE", "Physical Medicine", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AF", "Speech Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AG", "Skilled Nursing Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AH", "Skilled Nursing Care - Room and Board", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AI", "Substance Abuse", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AJ", "Alcoholism", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AK", "Drug Addiction", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AL", "Vision (Optometry)", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AM", "Frames", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AN", "Routine Exam", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AO", "Lenses", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AQ", "Nonmedically Necessary Physical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("AR", "Experimental Drug Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BA", "Independent Medical Evaluation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BB", "Partial Hospitalization (Psychiatric)", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BC", "Day Care (Psychiatric)", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BD", "Cognitive Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BE", "Massage Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BF", "Pulmonary Rehabilitation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BG", "Cardiac Rehabilitation", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BH", "Pediatric", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BI", "Nursery", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BJ", "Skin", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BK", "Orthopedic", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BL", "Cardiac", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BM", "Lymphatic", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BN", "Gastrointestinal", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BP", "Endocrine", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BQ", "Neurology", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BR", "Eye", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BS", "Invasive Procedures", pCategoryCombo, nCodeColumn, nCategoryColumn);
	// (c.haag 2010-10-20 15:07) - PLID 40352 - More codes for ANSI 5010
	AddToEligibilityBenefitCategoryCombo("BT", "Gynecological", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BU", "Obstetrical", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BV", "Obstetrical/Gynecological", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BW", "Mail Order Prescription Drug: Brand Name", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BX", "Mail Order Prescription Drug: Generic", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BY", "Physician Visit - Office: Sick", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("BZ", "Physician Visit - Office: Well", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("C1", "Coronary Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CA", "Private Duty Nursing - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CB", "Private Duty Nursing - Home", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CC", "Surgical Benefits - Professional (Physician)", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CD", "Surgical Benefits - Facility", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CE", "Mental Health Provider - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CF", "Mental Health Provider - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CG", "Mental Health Facility - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CH", "Mental Health Facility - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CI", "Substance Abuse Facility - Inpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CJ", "Substance Abuse Facility - Outpatient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CK", "Screening X-ray", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CL", "Screening laboratory", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CM", "Mammogram, High Risk Patient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CN", "Mammogram, Low Risk Patient", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CO", "Flu Vaccination", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CP", "Eyewear and Eyewear Accessories", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("CQ", "Case Management", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("DG", "Dermatology", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("DM", "Durable Medical Equipment", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("DS", "Diabetic Supplies", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("GF", "Generic Prescription Drug - Formulary", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("GN", "Generic Prescription Drug - Non-Formulary", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("GY", "Allergy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("IC", "Intensive Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("MH", "Mental Health", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("NI", "Neonatal Intensive Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("ON", "Oncology", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("PT", "Physical Therapy", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("PU", "Pulmonary", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("RN", "Renal", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("RT", "Residential Psychiatric Treatment", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("TC", "Transitional Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("TN", "Transitional Nursery Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
	AddToEligibilityBenefitCategoryCombo("UC", "Urgent Care", pCategoryCombo, nCodeColumn, nCategoryColumn);
}

void AddToEligibilityBenefitCategoryCombo(CString strCode, CString strCategory,
										  NXDATALIST2Lib::_DNxDataListPtr &pCategoryCombo,
										  int nCodeColumn, int nCategoryColumn)
{
	//throw exceptions to the caller

	NXDATALIST2Lib::IRowSettingsPtr pRow = pCategoryCombo->GetNewRow();
	pRow->PutValue(nCodeColumn, _bstr_t(strCode));
	pRow->PutValue(nCategoryColumn, _bstr_t(strCategory));
	pCategoryCombo->AddRowAtEnd(pRow, NULL);
}
*/

// (j.jones 2009-12-28 09:45) - PLID 32150 - placed queries for deleting service codes into
// one function such that the individual deletion in CCPTCodes::OnDeleteCpt() and mass deletion in
// CMainFrame::OnDeleteUnusedServiceCodes() would use the same delete code
void DeleteServiceCodes(CString &strSqlBatch, CString strServiceIDInClause)
{
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CPTInsNotesT WHERE CPTCodeID IN (%s)", strServiceIDInClause);
	// (a.wetta 2007-03-30 10:22) - PLID 24872 - Also delete any rule links associated with the service code
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CommissionRulesLinkT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CommissionT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrActionsT SET Deleted = 1 WHERE (SourceType = %li AND SourceID IN (%s)) OR (DestType = %li AND DestID IN (%s))",eaoCpt,strServiceIDInClause,eaoCpt,strServiceIDInClause);
	// (r.gonet 02/20/2014) - PLID 60778 - Renamed the table to remove the reference to ICD-9
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CPTDiagnosisGroupsT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiFeeItemsT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceRevCodesT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CredentialedCPTCodesT WHERE CPTCodeID IN (%s)", strServiceIDInClause);
	// (a.walling 2007-04-02 09:50) - PLID 25356 - Delete suggested sales
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM SuggestedSalesT WHERE MasterServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM SuggestedSalesT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (j.jones 2007-07-03 17:01) - PLID 26098 - delete links to modifiers
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiServiceModifierLinkT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsCoServicePayGroupLinkT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (a.walling 2007-07-25 16:10) - PLID 15998 - Delete from sales
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM SaleItemsT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (j.jones 2007-10-15 14:49) - PLID 27757 - handle deleting anesth/facility setups
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID IN (%s))", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM FacilityFeeSetupT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID IN (%s))", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AnesthesiaSetupT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (j.jones 2008-05-02 16:38) - PLID 29519 - delete reward discounts
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM RewardDiscountsT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM BlockedCPTCodesT WHERE ServiceID1 IN (%s) OR ServiceID2 IN (%s)", strServiceIDInClause, strServiceIDInClause);
	//(e.lally 2010-02-16) PLID 36849 - delete TOPS cpt code links to AMA codes
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TopsCptCodeLinkT WHERE CptCodeID IN (%s)", strServiceIDInClause);
	// (j.gruber 2010-07-20 14:50) - PLID 30481 - delete from the ApptTypeLink
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ApptTypeServiceLinkT WHERE ServiceID IN (%s)", strServiceIDInClause);
	// (j.gruber 2011-05-06 16:38) - PLID 43550 - delete from conversion groups
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ApptServiceConvServicesT WHERE ServiceID IN (%s)", strServiceIDInClause);
	//TES 5/20/2011 - PLID 43698 - Delete any links to GlassesCatalog tables
	// (s.dhole 2012-04-09 15:42) - PLID 43849 No longer valid
	/*AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesCatalogDesignsT SET CptID = NULL WHERE CptID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesCatalogMaterialsT SET CptID = NULL WHERE CptID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesCatalogTreatmentsT SET CptID = NULL WHERE CptID IN (%s)", strServiceIDInClause);*/
	// (d.singleton 2011-10-04 17:58) - PLID 44946 - delete any links to AlbertaCptModLinkT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AlbertaCptModLinkT WHERE ServiceID IN (%s)", strServiceIDInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CPTCodeT WHERE ID IN (%s)", strServiceIDInClause);

	// (j.gruber 2012-12-04 08:49) - PLID 48566 - delete from serviceLocationInfoT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceLocationInfoT WHERE ServiceID IN (%s)", strServiceIDInClause);

	// (a.wilson 2014-5-5) PLID 61831 - clear out ChargeLevelProviderConfigT where service was used.
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ChargeLevelProviderConfigT WHERE ServiceID IN (%s)", strServiceIDInClause);

	// (j.jones 2015-02-26 10:52) - PLID 65063 - clear ServiceMultiCategoryT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceMultiCategoryT WHERE ServiceID IN (%s)", strServiceIDInClause);

	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceT WHERE ID IN (%s)", strServiceIDInClause);
}

// (j.jones 2012-03-27 09:41) - PLID 45752 - added ability to mass delete diagnosis codes,
// putting the delete code in one function so it can be reused when deleting one, or many
// (j.armen 2012-04-03 15:52) - PLID 48299 - Added RecallTemplateDiagLink, use CParamCqlBatch, Take in CSqlFragment IN Clause
// (c.haag 2014-02-21) - PLID 60953 - Added diagnosis quick lists
// (b.savon 2014-12-26 15:02) - PLID 64355 - Removed setting emr actions as deleted if they are tied to diagnosis code being deleted
void DeleteDiagnosisCodes(CParamSqlBatch &sqlBatch, CSqlFragment &sqlDiagIDInClause)
{
	sqlBatch.Add("DELETE FROM DiagnosisQuickListT WITH (TABLOCK) WHERE ICD9ID IN ({SQL}) OR ICD10ID IN ({SQL})", sqlDiagIDInClause, sqlDiagIDInClause);
	sqlBatch.Add("UPDATE ConfigRT WITH (TABLOCK) SET IntParam = -1 WHERE Name = 'DefaultICD9Code' AND Username = '<None>' AND IntParam IN ({SQL})", sqlDiagIDInClause);
	sqlBatch.Add("DELETE FROM DiagCodeToEMRInfoT WITH (TABLOCK) WHERE DiagCodeID IN ({SQL})", sqlDiagIDInClause);
	sqlBatch.Add("UPDATE LabDiagnosisT WITH (TABLOCK) SET DiagID = NULL WHERE DiagID IN ({SQL})", sqlDiagIDInClause);
	sqlBatch.Add("DELETE FROM DiagInsNotesT WITH (TABLOCK) WHERE DiagCodeID IN ({SQL})", sqlDiagIDInClause);
	sqlBatch.Add("DELETE FROM CPTDiagnosisGroupsT WITH (TABLOCK) WHERE DiagCodeID IN ({SQL})", sqlDiagIDInClause);
	// (r.gonet 02/20/2014) - PLID 60778 - Renamed table to remove reference to ICD-9
	sqlBatch.Add("DELETE FROM BlockedDiagnosisCodesT WITH (TABLOCK) WHERE DiagCodeID1 IN ({SQL}) OR DiagCodeID2 IN ({SQL})", sqlDiagIDInClause, sqlDiagIDInClause);
	sqlBatch.Add("DELETE FROM RecallTemplateDiagLinkT WITH (TABLOCK) WHERE DiagCodesID IN ({SQL})", sqlDiagIDInClause);
	// (c.haag 2015-06-24) - PLID 66018 - Delete from custom crosswalks
	sqlBatch.Add("DELETE FROM DiagCodeCustomCrosswalksT WITH (TABLOCK) WHERE DiagCodeID IN ({SQL}) OR DiagCodeID_ICD10 IN ({SQL})", sqlDiagIDInClause, sqlDiagIDInClause);
	sqlBatch.Add("DELETE FROM DiagCodes WITH (TABLOCK) WHERE ID IN ({SQL})", sqlDiagIDInClause);
}

// (j.jones 2012-03-27 09:41) - PLID 47448 - added ability to mass delete CPT modifiers,
// putting the delete code in one function so it can be reused when deleting one, or many
void DeleteCPTModifiers(CString &strSqlBatch, CString strModifierInClause)
{
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiServiceModifierLinkT WHERE Modifier IN (%s)", strModifierInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CPTModifierT WHERE Number IN (%s)", strModifierInClause);
}

// (j.jones 2010-04-14 08:45) - PLID 38194 - EBilling_Calculate2010_REF will calculate the ID and qualifier to send
// in either 2010AA or 2010AB for ANSI HCFA and ANSI UB claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
// 2010AA and 2010AB output the same REF data, but they have different Adv. Ebilling Setup overrides.
void EBilling_Calculate2010_REF(BOOL bIs2010AB, CString &strQualifier, CString &strID, CString &strLoadedFrom, BOOL bIsUBClaim,
								 long nProviderID, long nInsuranceCoID, long nLocationID, long nInsuredPartyID,
								 long nHCFASetupID, CString strDefaultBox33GRP,
								 long nUBSetupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup)
{
	try {

		strQualifier = "";
		strID = "";
		strLoadedFrom = "";

		//Box33PinANSI will give us the qualifier AND ID, even if the ID is blank		
		if(!bIsUBClaim) {
			EBilling_Box33PinANSI(strQualifier,strID,nProviderID,nHCFASetupID,
				nInsuranceCoID, nInsuredPartyID, nLocationID);
			strLoadedFrom = "Box 33b";
		}
		else {
			// (j.jones 2007-04-02 10:00) - PLID 25276 - pre-loaded the qualifier if UB04
			strLoadedFrom = "Box 82";
			if(GetUBFormType() == eUB04) {
				strQualifier = strUB04Box76Qual;
				strQualifier.TrimLeft();
				strQualifier.TrimRight();
				strLoadedFrom = "Box 76";
			}
			EBilling_Box8283NumANSI(strQualifier,strID,nProviderID,nBox82Num,nInsuranceCoID,nUBSetupID,nBox82Setup == 3);
		}

		// (j.jones 2007-02-23 11:44) - PLID 24816 - previously we only sent the group number
		// if the claims were exported as a group instead of individual, but too few people
		// actually do that, so instead do not check for m_bIsGroup and always use the group
		// number if it exists, while retaining the previously loaded qualifier as before.

		//Show the provider's group ID if one exists, BUT use the strQualifier from before
		//if(m_bIsGroup) {
			//first check the Insurance Co's group
		if(!bIsUBClaim || nBox82Setup != 3) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups INNER JOIN InsuredPartyT ON InsuranceGroups.InsCoID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT} AND GRP <> ''",nInsuredPartyID,nProviderID);
			if(!rsTemp->eof) {
				CString strGRP = AdoFldString(rsTemp, "GRP","");
				strGRP.TrimLeft();
				strGRP.TrimRight();
				if(!strGRP.IsEmpty()) {
					strID = strGRP;
					// (j.jones 2010-11-03 14:32) - PLID 41309 - clarify that this
					// still uses the Box 33 Qualifier
					strLoadedFrom = "Insurance Company Group ID (using the Box 33b Qualifier)";
				}
			}
			rsTemp->Close();
		}

		if(!bIsUBClaim) {
			//then check the HCFA group's default
			if(strDefaultBox33GRP.GetLength() != 0) {
				strID = strDefaultBox33GRP;
				// (j.jones 2010-11-03 14:32) - PLID 41309 - clarify that this
				// still uses the Box 33 Qualifier
				strLoadedFrom = "Default HCFA Group ID (using the Box 33b Qualifier)";
			}

			//last check for the override
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} AND GRP <> ''",nProviderID,nLocationID,nHCFASetupID);
			if(!rsTemp->eof) {
				CString strGRP = AdoFldString(rsTemp, "GRP","");
				strGRP.TrimLeft();
				strGRP.TrimRight();
				if(!strGRP.IsEmpty()) {
					strID = strGRP;
					// (j.jones 2010-11-03 14:32) - PLID 41309 - clarify that this
					// still uses the Box 33 Qualifier
					strLoadedFrom = "Advanced ID Setup Group ID (using the Box 33b Qualifier)";
				}
			}
			rsTemp->Close();
		}
		//}

		//last, check the override
		_RecordsetPtr rsOver;
		if(!bIsUBClaim) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			if(!bIs2010AB) {
				rsOver = CreateParamRecordset("SELECT ANSI_2010AA AS IDNum, ANSI_2010AA_Qual AS Qual FROM EbillingSetupT WHERE Use_2010AA = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nHCFASetupID,nProviderID,nLocationID);
			}
			else {
				rsOver = CreateParamRecordset("SELECT ANSI_2010AB AS IDNum, ANSI_2010AB_Qual AS Qual FROM EbillingSetupT WHERE Use_2010AB = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nHCFASetupID,nProviderID,nLocationID);
			}
		}
		else {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			if(!bIs2010AB) {
				rsOver = CreateParamRecordset("SELECT ANSI_2010AA AS IDNum, ANSI_2010AA_Qual AS Qual FROM UB92EbillingSetupT WHERE Use_2010AA = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nUBSetupID,nProviderID,nLocationID);
			}
			else {
				rsOver = CreateParamRecordset("SELECT ANSI_2010AB AS IDNum, ANSI_2010AB_Qual AS Qual FROM UB92EbillingSetupT WHERE Use_2010AB = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nUBSetupID,nProviderID,nLocationID);
			}
		}
		if(!rsOver->eof) {
			// (j.jones 2008-04-29 16:39) - PLID 29619 - this now allows blank values,
			// so we must accept whatever is in the data, albeit with spaces trimmed
			strID = AdoFldString(rsOver, "IDNum", "");
			strID.TrimLeft();
			strID.TrimRight();
			strQualifier = AdoFldString(rsOver, "Qual", "");
			strQualifier.TrimLeft();
			strQualifier.TrimRight();
			if(!bIs2010AB) {
				strLoadedFrom = "Adv. Ebilling Setup 2010AA Override";
			}
			else {
				strLoadedFrom = "Adv. Ebilling Setup 2010AB Override";
			}
		}
		rsOver->Close();

		return;

	}NxCatchAll(__FUNCTION__);

	strQualifier = "";
	strID = "";
	strLoadedFrom = "";
}

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310B_REF will calculate the ID and qualifier to send
// in 2310B for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310B_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID)
{
	EBilling_Box33PinANSI(strQualifier,strID,nProviderID,nHCFASetupID,
		nInsuranceCoID, nInsuredPartyID, nLocationID);
	strLoadedFrom = "Box 33b";

	//check the override
	// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value
	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2310B, ANSI_2310B_Qual FROM EbillingSetupT WHERE Use_2310B = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nHCFASetupID,nProviderID,nLocationID);
	if(!rsOver->eof) {
		// (j.jones 2008-04-29 16:39) - PLID 29619 - this now allows blank values,
		// so we must accept whatever is in the data, albeit with spaces trimmed
		strID = AdoFldString(rsOver, "ANSI_2310B", "");
		strID.TrimLeft();
		strID.TrimRight();
		strQualifier = AdoFldString(rsOver, "ANSI_2310B_Qual", "");
		strQualifier.TrimLeft();
		strQualifier.TrimRight();
		strLoadedFrom = "Adv. Ebilling Setup 2310B Override";
	}
	rsOver->Close();
}

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310E_REF will calculate the ID and qualifier to send
// in 2310E for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310E_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nSupervisingProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID)
{
	//Use Box33PinANSI for the default value to use		
	EBilling_Box33PinANSI(strQualifier,strID,nSupervisingProviderID,nHCFASetupID,
		nInsuranceCoID, nInsuredPartyID, nLocationID);
	strLoadedFrom = "Box 33b";

	// (j.jones 2008-12-11 13:52) - PLID 32413 - added support for an override
	_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2310E, ANSI_2310E_Qual FROM EbillingSetupT WHERE Use_2310E = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nHCFASetupID,nSupervisingProviderID,nLocationID);
	if(!rsOver->eof) {
		//this does allow blank values
		strID = AdoFldString(rsOver, "ANSI_2310E", "");
		strID.TrimLeft();
		strID.TrimRight();
		strQualifier = AdoFldString(rsOver, "ANSI_2310E_Qual", "");
		strQualifier.TrimLeft();
		strQualifier.TrimRight();
		strLoadedFrom = "Adv. Ebilling Setup 2310E Override";
	}
	rsOver->Close();
}

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2420A_REF will calculate the ID and qualifier to send
// in 2420A for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2420A_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID)
{
	EBilling_Box33PinANSI(strQualifier,strID,nProviderID,nHCFASetupID,
		nInsuranceCoID, nInsuredPartyID, nLocationID);
	strLoadedFrom = "Box 33b";

	//JJ - 03/04/2003 - Apparently, 1A (Blue Cross) is not valid here,
	//only 1B (Blue Shield) is. But I strongly believe 1A is correct. So,
	//in the rare event this record is called, display 1B instead.
	//Note: This is for Noridian Blue Cross, which always demands this record.

	//04/01/2003 - I removed the ability (in Box33PinANSI and Box17aANSI) to even export 1A,
	//so if I leave it like that, then this code is not needed.

	//if(strIdentifier == "1A")
	//	strIdentifier = "1B";

	//check the override
	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2420A, ANSI_2420A_Qual FROM EbillingSetupT WHERE Use_2420A = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nHCFASetupID, nProviderID, nLocationID);
	if(!rsOver->eof) {
		// (j.jones 2008-04-29 16:39) - PLID 29619 - this now allows blank values,
		// so we must accept whatever is in the data, albeit with spaces trimmed
		strID = AdoFldString(rsOver, "ANSI_2420A", "");
		strID.TrimLeft();
		strID.TrimRight();
		strQualifier = AdoFldString(rsOver, "ANSI_2420A_Qual", "");
		strQualifier.TrimLeft();
		strQualifier.TrimRight();
		strLoadedFrom = "Adv. Ebilling Setup 2420A Override";
	}
	rsOver->Close();
}

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310B_REF will calculate the ID and qualifier to send
// in 2310A for ANSI UB claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310A_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nUBSetupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID)
{
	//Box8283NumANSI will give us the qualifier AND ID, even if the ID is blank
	// (j.jones 2007-04-02 10:00) - PLID 25276 - pre-loaded the qualifier if UB04
	if(GetUBFormType() == eUB04) {
		strQualifier = strUB04Box76Qual;
		strQualifier.TrimLeft();
		strQualifier.TrimRight();
	}
	EBilling_Box8283NumANSI(strQualifier,strID,nProviderID,nBox82Num,nInsuranceCoID,nUBSetupID, nBox82Setup == 3);

	//check the override
	if(nBox82Setup != 3) {
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2310A, ANSI_2310A_Qual FROM UB92EbillingSetupT WHERE Use_2310A = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",nUBSetupID,nProviderID,nLocationID);
		if(!rsOver->eof) {
			// (j.jones 2008-04-29 16:39) - PLID 29619 - this now allows blank values,
			// so we must accept whatever is in the data, albeit with spaces trimmed
			strID = AdoFldString(rsOver, "ANSI_2310A", "");
			strID.TrimLeft();
			strID.TrimRight();
			strQualifier = AdoFldString(rsOver, "ANSI_2310A_Qual", "");
			strQualifier.TrimLeft();
			strQualifier.TrimRight();
			strLoadedFrom = "Adv. Ebilling Setup 2310A Override";
		}
		rsOver->Close();
	}
}

// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils
void EBilling_Box17aANSI(long Box17a, CString Box17aQual, CString &strIdentifier, CString &strID, long RefPhyID)
{
	//this is the ID specified by the setup of Box17

	// (j.jones 2006-09-15 09:18) - PLID 22520 - the qualifier is now set up in the
	// HCFA Setup, so load that first, and if it is blank, auto-load the default.
	// Some fields, like Custom fields, have no default, so default to 'N5'.
	// That way, the rejection might make more sense than one without any qualifier at all.

	_variant_t var;

	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	_RecordsetPtr rsTemp = CreateParamRecordset("SELECT * FROM ReferringPhysT WHERE PersonID = {INT}",RefPhyID);
	
	if(rsTemp->eof) {
		//end of file does not necessarily mean a bad claim
		strIdentifier = "";
		strID = "";
		return;
	}

	strIdentifier = Box17aQual;
	strIdentifier.TrimRight();
	strIdentifier.TrimLeft();

	switch(Box17a) {
	case 1: { //Referring 

		//XX - Referring NPI Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "XX";

		//ReferringPhysT.NPI
		var = rsTemp->Fields->Item["NPI"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 2: { //Referring ID
		/* Referring ID has no Identifier in ANSI*/
		
		//N5 - Provider Plan Network Identification Number
		// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
		if(strIdentifier.IsEmpty())
			strIdentifier = "G2";

		//ReferringPhysT.ReferringPhyID
		var = rsTemp->Fields->Item["ReferringPhyID"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 3: { //Referring UPIN

		//1G - Provider UPIN Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1G";

		//ReferringPhysT.UPIN
		var = rsTemp->Fields->Item["UPIN"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 4: { //Referring Blue Shield

		//1B - BCBS Provider Number
		// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, 1B isn't valid.
		if(strIdentifier.IsEmpty())
			strIdentifier = "G2";

		//ReferringPhysT.BlueShieldID
		var = rsTemp->Fields->Item["BlueShieldID"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 5:	//Referring FedEmployerID

		//EI - Employer's Identification Number
		// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, EI isn't valid.
		if(strIdentifier.IsEmpty())
			strIdentifier = "G2";

		//ReferringPhysT.FedEmployerID
		var = rsTemp->Fields->Item["FedEmployerID"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	case 6:	//Referring DEANumber

		//0B - State License Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "0B";

		//ReferringPhysT.DEANumber
		var = rsTemp->Fields->Item["DEANumber"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	case 7:	//Referring MedicareNumber

		//1C - Medicare Provider Number
		// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, 1C isn't valid.
		if(strIdentifier.IsEmpty())
			strIdentifier = "G2";

		//ReferringPhysT.MedicareNumber
		var = rsTemp->Fields->Item["MedicareNumber"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	case 8:	//Referring MedicaidNumber

		//1D - Medicaid Provider Number
		// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, 1D isn't valid.
		if(strIdentifier.IsEmpty())
			strIdentifier = "G2";

		//ReferringPhysT.MedicaidNumber
		var = rsTemp->Fields->Item["MedicaidNumber"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;

	//The remaining choices have no Identifier in ANSI

	case 9:	//Referring WorkersCompNumber

		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		//ReferringPhysT.WorkersCompNumber
		var = rsTemp->Fields->Item["WorkersCompNumber"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	case 10:	//Referring OtherIDNumber
		//ReferringPhysT.OtherIDNumber
		
		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		var = rsTemp->Fields->Item["OtherIDNumber"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;

	// (j.jones 2007-06-20 11:47) - PLID 26399 - supported the License number
	case 11:	//Referring License
		//ReferringPhysT.License
		
		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		var = rsTemp->Fields->Item["License"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;

	// (j.jones 2007-04-24 13:01) - PLID 25764 - supported Taxonomy Code
	// (j.jones 2012-03-07 15:58) - PLID 48676 - removed Taxonomy Code
	/*
	case 12:  //Referring Taxonomy Code
		//ReferringPhysT.TaxonomyCode

		//ZZ - Referring Taxonomy Code
		if(strIdentifier.IsEmpty())
			strIdentifier = "ZZ";

		var = rsTemp->Fields->Item["TaxonomyCode"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	*/

	case 16: { //Custom ID 1
		//Custom1.TextParam

		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 6",RefPhyID);
		strID = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 17: { //Custom ID 2
		//Custom2.TextParam

		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 7",RefPhyID);
		strID = "";
		if(!rsCustom->eof) {
			// (j.jones 2007-06-13 14:14) - PLID 26318 - this didn't work previously, now it does - it never assigned to var!
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 18: { //Custom ID 3
		//Custom3.TextParam

		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 8",RefPhyID);
		strID = "";
		if(!rsCustom->eof) {
			// (j.jones 2007-06-13 14:14) - PLID 26318 - this didn't work previously, now it does - it never assigned to var!
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 19: { //Custom ID 4
		//Custom4.TextParam

		if(strIdentifier.IsEmpty())
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2, N5 isn't valid.
			strIdentifier = "G2"; //we're just guessing

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 9",RefPhyID);
		strID = "";
		if(!rsCustom->eof) {
			// (j.jones 2007-06-13 14:14) - PLID 26318 - this didn't work previously, now it does - it never assigned to var!
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	default:
		strIdentifier = "";
		strID = "";
		break;
	}

	rsTemp->Close();

	//if the ID is blank, don't output an identifier
	if(strID == "")
		strIdentifier = "";
}

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box33PinANSI to GlobalFinancialUtils	
void EBilling_Box33PinANSI(CString &strIdentifier, CString &strID, long nProviderID, long nHCFASetupID,
						   long nInsuranceCoID, long nInsuredPartyID, long nLocationID)
{
	//PIN

	//This does duplicate a lot of code from the normal Box33Pin, but this also returns
	//an ANSI identifier for the ID returned.

	// (j.jones 2006-09-15 09:18) - PLID 22520 - the qualifier is now set up in the
	// HCFA Setup, so load that first, and if it is blank, auto-load the default.
	// Some fields, like Custom fields, have no default, so default to 'XX'.
	// That way, the rejection might make more sense than one without any qualifier at all.

	_RecordsetPtr rsTemp,rs;
	_variant_t var;

	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	rs = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE PersonID = {INT}",nProviderID);
	
	if(rs->eof) {
		//end of file does not necessarily mean a bad claim
		strIdentifier = "";
		strID = "";
		return;
	}

	// (j.jones 2007-02-27 09:41) - PLID 24905 - use values from the specified group, instead of the member struct
	CString strBox33bQual = "";
	long nBox33 = 2;
	CString strBox33GRP = "";
	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	_RecordsetPtr rsHCFASetup = CreateParamRecordset("SELECT Box33bQual, Box33, Box33GRP FROM HCFASetupT WHERE ID = {INT}", nHCFASetupID);
	if(!rsHCFASetup->eof) {
		strBox33bQual = AdoFldString(rsHCFASetup, "Box33bQual", "");
		nBox33 = AdoFldLong(rsHCFASetup, "Box33", 2);
		strBox33GRP = AdoFldString(rsHCFASetup, "Box33GRP", "");
	}
	rsHCFASetup->Close();

	strIdentifier = strBox33bQual;
	strIdentifier.TrimRight();
	strIdentifier.TrimLeft();

	switch(nBox33) {
	case 1: { //NPI

		//XX - NPI Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "XX";

		//ProvidersT.NPI
		var = rs->Fields->Item["NPI"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 2: { //Social Security Number

		//SY - Social Security Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "SY";

		//PersonT.SocialSecurity
		var = rs->Fields->Item["SocialSecurity"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		StripNonNumericChars(strID);
		break;
	}
	case 3: { //Federal ID Number

		//EI - Employer's Identification Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "EI";

		//ProvidersT.[Fed Employer ID]
		var = rs->Fields->Item["Fed Employer ID"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		StripNonNumericChars(strID);
		break;
	}
	case 4: { //DEA Number

		//0B - State License Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "0B";

		//ProvidersT.[DEA Number]
		var = rs->Fields->Item["DEA Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 5: { //BCBS Number
		
		//1B - Blue Cross Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1B";

		//ProvidersT.[BCBS Number]
		var = rs->Fields->Item["BCBS Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 6: { //Medicare Number

		//1C - Medicare Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1C";

		//ProvidersT.[Medicare Number]
		var = rs->Fields->Item["Medicare Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 7: { //Medicaid Number

		//1D - Medicaid Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1D";

		//ProvidersT.[Medicaid Number]
		var = rs->Fields->Item["Medicaid Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 8: { //Workers Comp Number

		//Worker's Comp has no Identifier in ANSI
		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		//ProvidersT.[Workers Comp Number]
		var = rs->Fields->Item["Workers Comp Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 9: { //Other ID

		///Other ID has no Identifier in ANSI

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		//ProvidersT.[Other ID Number]
		var = rs->Fields->Item["Other ID Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 11: { //UPIN

		//1G - Provider UPIN Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1G";

		//ProvidersT.UPIN
		var = rs->Fields->Item["UPIN"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	
	//The remaining choices have no Identifier in ANSI

	case 12: { //Box24J

		//InsuranceBox24J.Box24JNumber

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsBox24J = CreateParamRecordset("SELECT Box24IQualifier, Box24JNumber FROM InsuranceBox24J WHERE ProviderID = {INT} AND InsCoID = {INT}",nProviderID, nInsuranceCoID);
		if(!rsBox24J->eof && rsBox24J->Fields->Item["Box24JNumber"]->Value.vt == VT_BSTR) {
			strID = CString(rsBox24J->Fields->Item["Box24JNumber"]->Value.bstrVal);

			if(strIdentifier.IsEmpty()) {
				strIdentifier = AdoFldString(rsBox24J, "Box24IQualifier","");
			}
			
		}
		else
			strID = "";
		rsBox24J->Close();

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		break;
	}

	case 13: {	//GRP
		// (j.jones 2007-01-17 14:30) - PLID 24263 - nobody's really going to select this when submitting electronically,
		// but since it is an option we have no choice but to support it

		//first check the Insurance Co's group 
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups INNER JOIN InsuredPartyT ON InsuranceGroups.InsCoID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}", nInsuredPartyID,nProviderID);
		if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
			strID = CString(rsTemp->Fields->Item["GRP"]->Value.bstrVal);
		else
			strID = "";
		rsTemp->Close();

		//then check the HCFA group's default
		if(strBox33GRP.GetLength() != 0)
			strID = strBox33GRP;

		//last check for the override
		rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",nProviderID, nLocationID, nHCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["GRP"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					strID = strTemp;
			}
		}
		rsTemp->Close();

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		break;
	}

	// (j.jones 2007-04-24 12:59) - PLID 25764 - supported Taxonomy Code
	case 14: { //Taxonomy Code
		//ProvidersT.TaxonomyCode

		//ZZ - Provider Taxonomy Code
		if(strIdentifier.IsEmpty())
			strIdentifier = "ZZ";

		var = rs->Fields->Item["TaxonomyCode"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}

	case 16: { //Custom ID 1
		//Custom1.TextParam

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 6 AND PersonID = {INT}",nProviderID);
		strID = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 17: { //Custom ID 2
		//Custom2.TextParam

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 7 AND PersonID = {INT}",nProviderID);
		strID = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 18: { //Custom ID 3
		//Custom3.TextParam

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 8 AND PersonID = {INT}",nProviderID);
		strID = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 19: { //Custom ID 4
		//Custom4.TextParam

		if(strIdentifier.IsEmpty()) {
			// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
			// so do not try to fill with anything, especially not XX, which is NPI
		}

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 9 AND PersonID = {INT}",nProviderID);
		strID = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
		}
		rsCustom->Close();
		// (j.jones 2007-07-17 16:09) - PLID 26722 - gimme a break...
		break;
	}
	default:
		strIdentifier = "";
		strID = "";
		break;
	}

	rs->Close();

	//if the ID is blank, don't output an identifier
	if(strID == "")
		strIdentifier = "";
}

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box8283NumANSI to GlobalFinancialUtils	
void EBilling_Box8283NumANSI(CString &strIdentifier, CString &strID, long nProviderID, long nIndex,
							 long nInsuranceCoID, long nUBGroupID, BOOL bIsRefPhy)
{
	//Box82 or 83 Number

	_RecordsetPtr rsTemp,rs;
	_variant_t var;

	// (j.jones 2007-04-02 09:34) - PLID 25276 - if a UB04, we allow the provider IDs
	// that we cannot auto-calculate a qualifier for
	BOOL bIsUB04 = GetUBFormType() == eUB04;

	// (j.jones 2007-05-10 14:20) - PLID 25948 - supported referring physician
	if(bIsRefPhy) {
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT * FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE PersonID = {INT}",nProviderID);
	}
	else {
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE PersonID = {INT}",nProviderID);
	}
	
	if(rs->eof) {
		//end of file does not necessarily mean a bad claim
		strIdentifier = "";
		strID = "";
		return;
	}

	// (j.jones 2007-04-02 09:54) - PLID 25276 - if a UB04, pull the qualifier from the setup, provided it wasn't passed to us
	if(bIsUB04 && strIdentifier.IsEmpty()) {
		CString strQual = "";
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsUBSetup = CreateParamRecordset("SELECT UB04Box76Qual FROM UB92SetupT WHERE ID = {INT}", nUBGroupID);
		if(!rsUBSetup->eof) {
			strQual = AdoFldString(rsUBSetup, "UB04Box76Qual", "");
		}
		rsUBSetup->Close();

		strIdentifier = strQual;
		strIdentifier.TrimRight();
		strIdentifier.TrimLeft();
	}

	switch(nIndex) {
	case 2: { //Social Security Number

		//SY - Social Security Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "SY";

		//PersonT.SocialSecurity
		var = rs->Fields->Item["SocialSecurity"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		StripNonNumericChars(strID);
		break;
	}
	case 3: { //Federal ID Number

		//EI - Employer's Identification Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "EI";

		//ProvidersT.[Fed Employer ID]
		if(bIsRefPhy)
			var = rs->Fields->Item["FedEmployerID"]->Value;
		else
			var = rs->Fields->Item["Fed Employer ID"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		StripNonNumericChars(strID);
		break;
	}
	case 4: { //Medicare Number

		//1C - Medicare Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1C";

		//ProvidersT.[Medicare Number]
		if(bIsRefPhy)
			var = rs->Fields->Item["MedicareNumber"]->Value;
		else
			var = rs->Fields->Item["Medicare Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 5: { //Medicaid Number

		//1D - Medicaid Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1D";

		//ProvidersT.[Medicaid Number]
		if(bIsRefPhy)
			var = rs->Fields->Item["MedicaidNumber"]->Value;
		else
			var = rs->Fields->Item["Medicaid Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 6: { //BCBS Number
		
		//1A - Blue Cross Provider Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1B";

		//ProvidersT.[BCBS Number]
		if(bIsRefPhy)
			var = rs->Fields->Item["BlueShieldID"]->Value;
		else
			var = rs->Fields->Item["BCBS Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 7: { //DEA Number

		//0B - State License Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "0B";

		//ProvidersT.[DEA Number]
		if(bIsRefPhy)
			var = rs->Fields->Item["DEANumber"]->Value;
		else
			var = rs->Fields->Item["DEA Number"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}	
	case 8: { //Workers Comp Number

		// (j.jones 2007-04-02 09:56) - PLID 25276 - only allow if UB04
		if(bIsUB04) {

			//Worker's Comp has no Identifier in ANSI
			if(strIdentifier.IsEmpty()) {
				// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
				// so do not try to fill with anything, especially not XX, which is NPI
			}

			//ProvidersT.[Workers Comp Number]
			if(bIsRefPhy)
				var = rs->Fields->Item["WorkersCompNumber"]->Value;
			else
				var = rs->Fields->Item["Workers Comp Number"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
			else
				strID = "";
		}
		break;		
	}
	case 9: { //Other ID

		// (j.jones 2007-04-02 09:56) - PLID 25276 - only allow if UB04
		if(bIsUB04) {

			//Other ID has no Identifier in ANSI
			if(strIdentifier.IsEmpty()) {
				// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
				// so do not try to fill with anything, especially not XX, which is NPI
			}

			//ProvidersT.[Other ID Number]
			if(bIsRefPhy)
				var = rs->Fields->Item["OtherIDNumber"]->Value;
			else
				var = rs->Fields->Item["Other ID Number"]->Value;
			if(var.vt == VT_BSTR)
				strID = CString(var.bstrVal);
			else
				strID = "";
		}
		break;		
	}
	case 10: {	//Box 51

		// (j.jones 2007-04-02 09:56) - PLID 25276 - only allow if UB04
		if(bIsUB04) {

			//Box 51 has no Identifier in ANSI
			if(strIdentifier.IsEmpty()) {
				// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
				// so do not try to fill with anything, especially not XX, which is NPI
			}

			//ref phy would be blank
			if(bIsRefPhy) {
				strID = "";
				strIdentifier = "";
				return;
			}

			//first check the per-provider ID
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box51Info AS Box51 FROM InsuranceBox51 "
				"WHERE InsCoID = {INT} AND ProviderID IN (SELECT ClaimProviderID FROM ProvidersT WHERE PersonID = {INT})", nInsuranceCoID, nProviderID);
			if (!rs->eof) {
				strID = AdoFldString(rs, "Box51","");
			}
			rs->Close();

			//now check the group's default
			rs = CreateParamRecordset("SELECT Box51Default AS Box51 FROM UB92SetupT WHERE ID = {INT}", nUBGroupID);
			if (!rs->eof) {
				CString strDefault = AdoFldString(rs, "Box51","");
				if(!strDefault.IsEmpty())
					strID = strDefault;
			}
			rs->Close();
		}
		break;
	}
	case 11: {	//Group Number

		// (j.jones 2007-04-02 09:56) - PLID 25276 - only allow if UB04
		if(bIsUB04) {

			//Group Number has no Identifier in ANSI
			if(strIdentifier.IsEmpty()) {
				// (j.jones 2010-04-15 16:50) - PLID 38149 - we don't have a default,
				// so do not try to fill with anything, especially not XX, which is NPI
			}

			//ref phy would be blank
			if(bIsRefPhy) {
				strID = "";
				strIdentifier = "";
				return;
			}

			//only check the per-provider ID, there is no group-wide default on UB forms
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT GRP FROM InsuranceGroups "
				"WHERE InsCoID = {INT} AND ProviderID IN (SELECT ClaimProviderID FROM ProvidersT WHERE PersonID = {INT})", nInsuranceCoID, nProviderID);
			if (!rs->eof) {
				strID = AdoFldString(rs, "GRP","");
			}
			rs->Close();
		}
		break;
	}
	case 12: { //NPI

		//XX - NPI Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "XX";

		//ProvidersT.NPI
		var = rs->Fields->Item["NPI"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	case 1: { //UPIN

		//1G - Provider UPIN Number
		if(strIdentifier.IsEmpty())
			strIdentifier = "1G";

		//ProvidersT.UPIN
		var = rs->Fields->Item["UPIN"]->Value;
		if(var.vt == VT_BSTR)
			strID = CString(var.bstrVal);
		else
			strID = "";
		break;
	}
	default:
		
		// (j.jones 2008-05-21 15:56) - PLID 30139 - we now allow blank values
		strIdentifier = "";
		strID = "";
		break;
	}

	rs->Close();

	//if the ID is blank, don't output an identifier
	if(strID == "") {
		strIdentifier = "";
	}
}

// (j.jones 2010-07-23 14:09) - PLID 34105 - this function returns true/false if at least one
// HCFA group exists that can send claims with the "Assignments Of Benefits" not filled
// (j.jones 2010-07-27 09:19) - PLID 39854 - moved to CBlankAssignmentBenefitsDlg
//BOOL CanAssignmentOfBenefitsBeBlank()

// (j.jones 2010-07-23 14:45) - PLID 39783 - moved Accept Assignment calculations into these functions
BOOL GetAcceptAssignment_ByInsCo(long nInsuranceCoID, long nProviderID)
{
	//Assume it IS accepted if we do not have an entry in InsuranceAcceptedT.
	//This makes it impossible to have it not be accepted unless they have
	//intentionally set it up to do so.

	BOOL bAccepted = TRUE;

	//save a recordset if we're passed bad data
	if(nInsuranceCoID == -1 || nProviderID == -1) {
		return bAccepted;
	}

	_RecordsetPtr rsAcc = CreateParamRecordset("SELECT Accepted FROM InsuranceAcceptedT "
		"WHERE InsuranceCoID = {INT} AND ProviderID = {INT}", nInsuranceCoID, nProviderID);
	if(!rsAcc->eof) {
		bAccepted = AdoFldBool(rsAcc, "Accepted", TRUE);
	}
	rsAcc->Close();

	return bAccepted;
}

// (j.jones 2010-07-23 14:45) - PLID 39783 - moved Accept Assignment calculations into these functions
BOOL GetAcceptAssignment_ByInsuredParty(long nInsuredPartyID, long nProviderID)
{
	//Assume it IS accepted if we do not have an entry in InsuranceAcceptedT.
	//This makes it impossible to have it not be accepted unless they have
	//intentionally set it up to do so.

	BOOL bAccepted = TRUE;

	//save a recordset if we're passed bad data
	if(nInsuredPartyID == -1 || nProviderID == -1) {
		return bAccepted;
	}

	_RecordsetPtr rsAcc = CreateParamRecordset("SELECT InsuranceAcceptedT.Accepted FROM InsuranceAcceptedT "
		"INNER JOIN InsuredPartyT ON InsuranceAcceptedT.InsuranceCoID = InsuredPartyT.InsuranceCoID "
		"WHERE InsuredPartyT.PersonID = {INT} AND InsuranceAcceptedT.ProviderID = {INT}", nInsuredPartyID, nProviderID);
	if(!rsAcc->eof) {
		bAccepted = AdoFldBool(rsAcc, "Accepted", TRUE);
	}
	rsAcc->Close();

	return bAccepted;
}

// (j.jones 2010-09-24 13:32) - PLID 34518 - Given an insured party ID,
// open a payment for that insured party, default it to be a copay, and pull in the
// $ amount from the CoPay pay group. $0.00 otherwise.
void PromptForCopay(long nInsuredPartyID)
{
	try {

		if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) {
			return;
		}

		//try to load the default Copay $ amount for this insured party

		long nPatientID = -1;
		COleCurrency cyDefaultCoPayAmt = COleCurrency(0,0);

		//remember they might not actually have a copay pay group with a $ amount filled in,
		//if so we just default the payment to zero
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, CopayMoney "
			"FROM InsuredPartyT "
			"LEFT JOIN (SELECT CopayMoney, InsuredPartyID FROM InsuredPartyPayGroupsT "
			"	INNER JOIN ServicePayGroupsT ON InsuredPartyPayGroupsT.PayGroupID = ServicePayGroupsT.ID "
			"	WHERE CopayMoney Is Not Null AND Name = 'Copay') AS PayGroupsQ ON InsuredPartyT.PersonID = PayGroupsQ.InsuredPartyID "
			"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID");
			cyDefaultCoPayAmt = AdoFldCurrency(rs, "CopayMoney", COleCurrency(0,0));
		}
		else {
			//invalid insured party ID
			ThrowNxException("Invalid InsuredPartyID (%li)", nInsuredPartyID);
		}
		rs->Close();

		// (j.jones 2015-11-05 12:31) - PLID 63866 - ensure this is rounded
		RoundCurrency(cyDefaultCoPayAmt);

		CPaymentDlg dlg(NULL);
		dlg.m_PatientID = nPatientID;
		
		// (j.jones 2011-11-11 10:53) - PLID 46348 - changed to an array of pointers, but since
		// we do not apply to a bill through this function, we only need to track the nInsuredPartyID
		CArray<InsuranceCoPayApplyList*, InsuranceCoPayApplyList*> arypInsuranceCoPayApplyList;
		InsuranceCoPayApplyList *icpaiApplyList = new InsuranceCoPayApplyList();
		icpaiApplyList->nInsuredPartyID = nInsuredPartyID;
		arypInsuranceCoPayApplyList.Add(icpaiApplyList);
		dlg.m_parypInsuranceCoPayApplyList = &arypInsuranceCoPayApplyList;

		dlg.m_iDefaultPaymentType = 0;
		dlg.m_bIsCoPay = TRUE;
		dlg.m_cyCopayAmount = cyDefaultCoPayAmt;
		dlg.m_cyFinalAmount = cyDefaultCoPayAmt;
		dlg.DoModal(__FUNCTION__, __LINE__);

		// (j.jones 2011-11-11 10:53) - PLID 46348 - clear our memory object
		delete icpaiApplyList;

	}NxCatchAll(__FUNCTION__);
}

// displays all eligibility requests/responses for a patient, using the most-primary
// insured party on their account
void ShowAllEligibilityRequestsForInsuredParty_ByPatient(CWnd *pParentWnd, long nPatientID)
{
	try {

		//call this overload
		ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(pParentWnd, nPatientID, -1);

	}NxCatchAll(__FUNCTION__);
}

// displays all eligibility requests/responses for a patient, using the most-primary
// insured party on their account or the appointment's insured party
void ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(CWnd *pParentWnd, long nPatientID, long nAppointmentID)
{
	try {

		long nInsuredPartyID = -1;
		//If an appointment was provided, see if the appt. has an insured party
		//on it, and if so, use that insurance.
		//Otherwise use the most-primary party on the patient's account.
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON; "
			"DECLARE @PatientInsuredPartyID INT; "
			"SET @PatientInsuredPartyID = ({SQL}); "
			""
			"DECLARE @ApptInsuredPartyID INT, @ApptID INT; "
			"SET @ApptID = {INT}; "
			"IF (@ApptID <> -1) "
			"BEGIN "
			"	SET @ApptInsuredPartyID = ("
			"		SELECT AppointmentInsuredPartyPrimQ.InsuredPartyID "
			"		FROM AppointmentsT "
			"		INNER JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartyPrimQ ON "
			"			((AppointmentInsuredPartyPrimQ.AppointmentID = AppointmentsT.ID) AND (AppointmentInsuredPartyPrimQ.Placement = 1)) "
			"		WHERE AppointmentsT.ID = @ApptID "
			"		) "
			"END "
			""
			//use the appt. primary insurance if one exists, else the patient primary insurance,
			//if both are null then the upcoming recordset will return no results
			"DECLARE @InsuredPartyID INT; "
			"SET @InsuredPartyID = IsNull(@ApptInsuredPartyID, @PatientInsuredPartyID); "
			""
			"SET NOCOUNT OFF;"
			""
			"SELECT @InsuredPartyID AS InsuredPartyID",
			GetPatientPrimaryInsuredPartyIDSql(nPatientID),
			nAppointmentID);
		if (!rs->eof) {
			nInsuredPartyID = AdoFldLong(rs->Fields, "InsuredPartyID", -1);
		}
		rs->Close();

		if (nInsuredPartyID == -1) {
			//should be impossible if the patient has any insurance
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				"The selected patient has no insured parties.", "Practice", MB_ICONINFORMATION | MB_OK);
			return;
		}

		ShowAllEligibilityRequestsForInsuredParty(pParentWnd, nInsuredPartyID, nAppointmentID);

	}NxCatchAll(__FUNCTION__);
}

//displays all eligibility requests/responses for a given insured party, which may generate a new request
//if our current list is outdated
void ShowAllEligibilityRequestsForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID)
{
	try {

		//call the main overload with no appointment ID, it is ok to be -1
		ShowAllEligibilityRequestsForInsuredParty(pParentWnd, nInsuredPartyID, -1);

	}NxCatchAll(__FUNCTION__);
}

//master function that the other two call, may generate a new request
//if our current list is outdated
void ShowAllEligibilityRequestsForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID)
{
	try {

		if (!CheckCurrentUserPermissions(bioEEligibility, sptRead)) {
			return;
		}

		//nAppointmentID is an optional parameter, we are not showing only
		//requests linked to that appointment, it is only used for 
		//auto-creating requests

		//first check to see if the insured party has no responses, or no responses in the past 3 days
		//if an appt is provided, check against the appt. provider/location/insurance
		AutoUpdateOutdatedEligibilityRequests(pParentWnd, nInsuredPartyID, nAppointmentID);

		//load all requests for this insured party, sort by most recent first, with the date
		//calculated by the most recent response date or the create date if no response exists
		_RecordsetPtr rs = CreateParamRecordset("SELECT EligibilityRequestsT.ID "
			"FROM EligibilityRequestsT "
			"INNER JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN (SELECT RequestID, Max(DateReceived) AS MostRecentDateReceived "
			"	FROM EligibilityResponsesT GROUP BY RequestID) AS ResponsesQ ON EligibilityRequestsT.ID = ResponsesQ.RequestID "
			"WHERE InsuredPartyT.PersonID = {INT} "
			"ORDER BY (CASE WHEN ResponsesQ.RequestID Is Not Null THEN ResponsesQ.MostRecentDateReceived ELSE EligibilityRequestsT.CreateDate END) DESC",
			nInsuredPartyID);

		// (r.goldschmidt 2014-10-10 12:51) - PLID 62644 - make eligibility request detail dialog modeless
		std::vector<long> aryRequestIDsUpdated;

		if(rs->eof) {
			AfxMessageBox("This patient has no E-Eligibility requests.");
			return;
		}
		else {

			while(!rs->eof) {

				aryRequestIDsUpdated.push_back(AdoFldLong(rs, "ID"));

				rs->MoveNext();
			}
		}
		rs->Close();

		GetMainFrame()->ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated);

	}NxCatchAll(__FUNCTION__);
}

//For a given insured party, and optionally appointment, will check to see if an appropriate
//eligibility response exists within the past 3 days. If not, a new one will be created.
void AutoUpdateOutdatedEligibilityRequests(CWnd *pParentWnd, long nInsuredPartyID)
{
	try {

		//call the main overload with no appointment ID, it is ok to be -1
		AutoUpdateOutdatedEligibilityRequests(pParentWnd, nInsuredPartyID, -1);

	}NxCatchAll(__FUNCTION__);
}

//For a given insured party, and optionally appointment, will check to see if an appropriate
//eligibility response exists within the past 3 days. If not, a new one will be created.
void AutoUpdateOutdatedEligibilityRequests(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID)
{
	try {

		if (nInsuredPartyID <= 0) {
			//don't look up responses for no insured party
			return;
		}

		//We need to check to see if this insured party has a response in the past 3 days.
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 EligibilityRequestsT.ID "
			"FROM EligibilityRequestsT "
			"INNER JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN (SELECT RequestID, Max(DateReceived) AS MostRecentDateReceived "
			"	FROM EligibilityResponsesT GROUP BY RequestID) AS ResponsesQ ON EligibilityRequestsT.ID = ResponsesQ.RequestID "
			"WHERE InsuredPartyT.PersonID = {INT} "
			"AND dbo.AsDateNoTime(ResponsesQ.MostRecentDateReceived) >= DATEADD(day, -3, dbo.AsDateNoTime(GetDate()))",
			nInsuredPartyID);

		if (!rs->eof) {
			//we have a result in the past 3 full business days, we're good to go!
			return;
		}

		//if we get here, we need to create a request

		//If real-time eligibility is off, check to see if a request exists in the batch,
		//no point in duplicating it if it's already there for the same insured party.
		//Real-time will just generate and export.
		BOOL bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);
		if (!bUseRealTimeElig) {
			if (ReturnsRecordsParam("SELECT ID FROM EligibilityRequestsT WHERE InsuredPartyID = {INT} AND Batched = 1", nInsuredPartyID)) {
				MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
					"This patient's insurance does not have an E-Eligibility response within the past 3 days.\n\n"
					"An E-Eligibility request for this insured party already exists in the E-Eligibility tab of the Financial module. "
					"Please export the current Eligibility batch to get a new response.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				return;
			}
		}

		//now create the request
		if (!GenerateNewEligibilityRequestForInsuredParty(pParentWnd, nInsuredPartyID, nAppointmentID)) {
			//if this fails they probably got a warning as to why it failed, but give
			//a second one to explain why we even tried
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				"This patient's insurance does not have an E-Eligibility response within the past 3 days. Nextech was unable to automatically send a new request.", "Practice", MB_ICONINFORMATION | MB_OK);
		}

	}NxCatchAll(__FUNCTION__);
}

//Auto-generates a new eligibility request for an insured party.
//Returns false if a request was not created, or did not successfully send in real-time.
bool GenerateNewEligibilityRequestForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID)
{
	try {

		//call the main overload with no appointment ID, it is ok to be -1
		return GenerateNewEligibilityRequestForInsuredParty(pParentWnd, nInsuredPartyID, -1);

	}NxCatchAll(__FUNCTION__);

	return false;
}

//Auto-generates a new eligibility request for an insured party,
//if an appointment is provided it will be used for provider/location defaults.
//Returns false if a request was not created, or did not successfully send in real-time.
bool GenerateNewEligibilityRequestForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID)
{
	try {

		if (nInsuredPartyID <= 0) {
			//don't make a request for no insured party
			return false;
		}

		//if they do not have permission, give a unique warning
		if (!(GetCurrentUserPermissions(bioEEligibility) & (sptWrite | sptWriteWithPass))			
			//if they have permission, check it, it may need a password
			|| !CheckCurrentUserPermissions(bioEEligibility, sptWrite)) {
			
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(), "A new E-Eligibility request could not be created because you do not have permission to do so. "
				"Please contact your Office Manager.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return false;
		}
		
		long nProviderID = -1;
		long nLocationID = -1;
		
		//get the default benefit category, usually 30
		CString strCategory = GetRemotePropertyText("DefaultEligibilityBenefitCategory", "30", 0, "<None>", TRUE);
		_variant_t varCategoryID = g_cvarNull;

		CSqlFragment sqlAppt("");
		//nAppointmentID is an optional parameter, it is only used to
		//influence the default provider/location, and possibly insurance
		if (nAppointmentID != -1) {
			sqlAppt = CSqlFragment("SELECT TOP 1 "
				"ResourceProviderLinkT.ProviderID, AppointmentsT.LocationID, AppointmentInsuredPartyPrimQ.InsuredPartyID "
				"FROM AppointmentsT "
				"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
				"LEFT JOIN ResourceProviderLinkT ON AppointmentResourceT.ResourceID = ResourceProviderLinkT.ResourceID "
				"LEFT JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID "
				"LEFT JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartyPrimQ ON "
				"	((AppointmentInsuredPartyPrimQ.AppointmentID = AppointmentsT.ID) AND (AppointmentInsuredPartyPrimQ.Placement = 1)) "
				//we want to skip inactive providers, but having no provider is fine
				"WHERE (PersonT.ID Is Null OR PersonT.Archived = 0) "
				"AND AppointmentsT.ID = {INT}", nAppointmentID);
		}

		//load our defaults, first from the patient, then from the appointment (which may be -1)
		_RecordsetPtr rs = CreateParamRecordset(
			//get the benefit category ID of 30, which is our default
			"SELECT ID FROM EligibilityBenefitCategoriesT WHERE Code = '30'; "
			""
			//get the benefit category ID from their preference
			"SELECT ID FROM EligibilityBenefitCategoriesT WHERE Code = {STRING}; "
			""
			//get the G1 provider, G2 location
			"SELECT PatientsT.MainPhysician, PersonT.Location "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID "
			"WHERE InsuredPartyT.PersonID = {INT}; "
			""
			//optional appt. lookup
			"{SQL}",
			strCategory,
			nInsuredPartyID,
			sqlAppt);

		if (!rs->eof) {
			//get the default benefit category of 30
			long nCodeID = VarLong(rs->Fields->Item["ID"]->Value, -1);
			if (nCodeID != -1) {
				varCategoryID = nCodeID;
			}
		}

		rs = rs->NextRecordset(NULL);

		if (!rs->eof) {
			//get the preference for the benefit category
			long nCodeID = VarLong(rs->Fields->Item["ID"]->Value, -1);
			if (nCodeID != -1) {
				varCategoryID = nCodeID;
			}
		}

		rs = rs->NextRecordset(NULL);

		if (!rs->eof) {
			//get the G1 provider, G2 location
			nProviderID = VarLong(rs->Fields->Item["MainPhysician"]->Value, -1);
			nLocationID = VarLong(rs->Fields->Item["Location"]->Value, -1);
		}

		//if we have an appointment, load that recordset
		if (nAppointmentID != -1) {

			rs = rs->NextRecordset(NULL);

			if (!rs->eof) {
				//get the appointment information, if any
				long nApptProviderID = VarLong(rs->Fields->Item["ProviderID"]->Value, -1);
				long nApptLocationID = VarLong(rs->Fields->Item["LocationID"]->Value, -1);
				long nApptInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);

				if (nApptProviderID != -1) {
					nProviderID = nApptProviderID;
				}
				if (nApptLocationID != -1) {
					nLocationID = nApptLocationID;
				}
				if (nApptInsuredPartyID != -1) {
					//why are we calling this function with an appt. but not using the appt's insured party?
					ASSERT(nInsuredPartyID == nApptInsuredPartyID);
					// well, we're using it now!
					nInsuredPartyID = nApptInsuredPartyID;
				}
			}
		}

		rs->Close();

		//ensure we have all three IDs
		if (nProviderID == -1) {
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				FormatString("A new E-Eligibility request could not be created because the selected patient has no provider selected in General 1%s.",
					nAppointmentID == -1 ? "" : ", and no provider linked to this appointment's resource"), "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
		if (nLocationID == -1) {
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				FormatString("A new E-Eligibility request could not be created because the selected patient has no location selected in General 2%s.",
					nAppointmentID == -1 ? "" : ", and no location selected on this appointment"), "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
		if (nInsuredPartyID == -1) {
			//it should be impossible to get here, this function is meant to be called with an insured party
			ASSERT(FALSE);
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				FormatString("A new E-Eligibility request could not be created because the selected patient has no insured parties%s.",
					nAppointmentID == -1 ? "" : ", and no primary insurance selected on this appointment"), "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		//finally, ready to make the request
		long nNewRequestID = NewNumber("EligibilityRequestsT", "ID");
		ExecuteParamSql("INSERT INTO EligibilityRequestsT (ID, InsuredPartyID, ProviderID, LocationID, Batched, Selected, CreateDate, LastSentDate, BenefitCategoryID) "
			"VALUES ({INT}, {INT}, {INT}, {INT}, 1, 0, GetDate(), NULL, {VT_I4})",
			nNewRequestID, nInsuredPartyID, nProviderID, nLocationID, varCategoryID);

		//now, can we auto-export it?
		BOOL bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);

		if (!bUseRealTimeElig) {
			MessageBox(pParentWnd == NULL ? GetActiveWindow() : pParentWnd->GetSafeHwnd(),
				"This patient's insurance does not have an E-Eligibility response within the past 3 days.\n\n"
					"A new E-Eligibility request was created and can be exported from the E-Eligibility tab of the Financial module. "
					"Please export the current Eligibility batch to get a new response.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
		else {
			long nFormatID = GetDefaultEbillingANSIFormatID();

			CEEligibility dlg(pParentWnd);
			dlg.m_FormatID = nFormatID;
			dlg.m_bUseRealTimeElig = bUseRealTimeElig;
			//add this request
			dlg.m_aryRequestIDsToExport.Add(nNewRequestID);
			dlg.m_bCEligibilityRequestDetailDlg = FALSE;
			dlg.DoModal();

			return true;
		}

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2010-10-19 09:55) - PLID 40931 - returns TRUE if at least one ANSI setup is set to use 5010
BOOL Is5010Enabled() {

	return ReturnsRecordsParam("SELECT TOP 1 ANSIVersion FROM EBillingFormatsT WHERE ANSIVersion = {INT}", (long)av5010);
}

// (j.jones 2011-03-21 13:49) - PLID 42917 - returns the server's path to where
// we store EOBWarning logs from E-Remittance
CString l_strNxEOBWarningLogsPath;
const CString &GetNxEOBWarningLogsPath()
{
	//much like GetNxTempPath(), we only check this once per session,
	//and this does not support the unlikely case where the folder is
	//deleted during a Practice session

	if (l_strNxEOBWarningLogsPath.IsEmpty()) {
		l_strNxEOBWarningLogsPath = GetSharedPath() ^ "NexTech" ^ "EOBWarningLogs";
		if (!DoesExist(l_strNxEOBWarningLogsPath)) {
			// If the path doesn't exist, create it
			CreatePath(l_strNxEOBWarningLogsPath);
		}
	}

	return l_strNxEOBWarningLogsPath;
}

// (j.jones 2011-03-21 13:49) - PLID 42917 - returns the server's path to where
// we store converted EOBs from E-Remittance
CString l_strNxConvertedEOBsPath;
const CString &GetNxConvertedEOBsPath()
{
	//much like GetNxTempPath(), we only check this once per session,
	//and this does not support the unlikely case where the folder is
	//deleted during a Practice session

	if (l_strNxConvertedEOBsPath.IsEmpty()) {
		l_strNxConvertedEOBsPath = GetSharedPath() ^ "NexTech" ^ "ConvertedEOBs";
		if (!DoesExist(l_strNxConvertedEOBsPath)) {
			// If the path doesn't exist, create it
			CreatePath(l_strNxConvertedEOBsPath);
		}
	}

	return l_strNxConvertedEOBsPath;
}

// (b.spivey, October 9th, 2014) PLID 62701 - 
CString l_strNxEOBStoragePath;
const CString &GetNxEOBStoragePath()
{
	if (l_strNxEOBStoragePath.IsEmpty()) {
		l_strNxEOBStoragePath = GetSharedPath() ^ "Documents" ^ "_EOBs";
		if (!DoesExist(l_strNxEOBStoragePath)) {
			// If the path doesn't exist, create it
			CreatePath(l_strNxEOBStoragePath);
		}
	}

	return l_strNxEOBStoragePath; 
}

// (b.spivey, October 9th, 2014) PLID 62701 - Global function that can be called from our individual parsers. 
CString CopyParsedEOBToServerStorage(CString strInputFile)
{
	CString strServerPath = GetNxEOBStoragePath() ^ GetFileName(strInputFile);
	
	if (!CopyFile(strInputFile, strServerPath, TRUE)) {
		//failed
		ThrowNxException("Cannot copy EOB to server, filename: %s", strServerPath);
	}
	
	return strServerPath;
}


// (j.jones 2011-04-20 10:45) - PLID 41490 - gets an array of all diagnosis codes on a bill, and their descriptions
// (j.jones 2011-04-20 10:45) - PLID 41490 - This function supports the ability for claims to ignore
//diagnosis codes not linked to charges. 
// (a.walling 2014-02-28 17:46) - PLID 61128 - works with BillDiagCodeT, has options for including ICD9 or ICD10

// (a.walling 2014-03-05 16:05) - PLID 61202 - GetDiagnosisCodesForHCFA - Support ICD10 option, handle ChargeWhichCodesT
// (a.walling 2014-03-19 10:05) - PLID 61346 - The entire confusing 'skipped diag indexes' concept is going away. This now returns a vector of diag codes which can be used to determine whichcodes for individual charges.
std::vector<Nx::DiagCode> GetBillDiagCodes(
	long nBillID
	, bool bUseICD10, bool bSkipUnlinkedDiagsOnClaims
	, IN OPTIONAL const CArray<long, long>* paryChargeIDsToExport
)
{
	//gather all our diagnosis codes on the bill
	// (j.jones 2011-04-20 10:45) - PLID 41490 - gets a collection of all diagnosis codes on a bill, and their descriptions
	// (a.walling 2014-02-28 17:46) - PLID 61128 - works with BillDiagCodeT, has options for including ICD9 or ICD10
	// (a.walling 2014-03-17 14:01) - PLID 61401 - Now only includes unique codes ordered by min(OrderIndex)
	_RecordsetPtr rsDiags;
	if (bSkipUnlinkedDiagsOnClaims && paryChargeIDsToExport && !paryChargeIDsToExport->IsEmpty()) {
		rsDiags = CreateParamRecordset(
			"SELECT "
				"DiagCodes.ID AS DiagID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM BillDiagCodeT "
			"INNER JOIN ChargeWhichCodesT "
				"ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID "
			"INNER JOIN DiagCodes "
				"ON BillDiagCodeT.{CONST_STR} = DiagCodes.ID "
			"WHERE BillDiagCodeT.BillID = {INT} "
				"AND ChargeWhichCodesT.ChargeID IN ({INTARRAY}) "
			"GROUP BY DiagCodes.ID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"ORDER BY MIN(BillDiagCodeT.OrderIndex)"
			, bUseICD10 ? "ICD10DiagID" : "ICD9DiagID"
			, nBillID
			, *paryChargeIDsToExport
		);
	} else {
		rsDiags = CreateParamRecordset(
			"SELECT "
				"DiagCodes.ID AS DiagID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM BillDiagCodeT "
			"INNER JOIN DiagCodes "
				"ON BillDiagCodeT.{CONST_STR} = DiagCodes.ID "
			"WHERE BillDiagCodeT.BillID = {INT} "
			"GROUP BY DiagCodes.ID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"ORDER BY MIN(BillDiagCodeT.OrderIndex)"
			, bUseICD10 ? "ICD10DiagID" : "ICD9DiagID"
			, nBillID
		);
	}

	// (a.walling 2014-03-19 10:05) - PLID 61346
	std::vector<Nx::DiagCode> diagCodes;
	for (; !rsDiags->eof; rsDiags->MoveNext()) {
		diagCodes.push_back(
			Nx::DiagCode(
				AdoFldLong(rsDiags, "DiagID")
				, AdoFldString(rsDiags, "CodeNumber", "")
				, AdoFldString(rsDiags, "CodeDesc", "")
			)
		);
	}

	return diagCodes;
}

// (a.walling 2014-03-19 10:05) - PLID 61346 - utility function to generate <BillDiags><Diag><ID>##</ID><OrderIndex>#</OrderIndex></Diag>...</BillDiags> xml 
// where OrderIndex is the order within the collection passed in
CString MakeBillDiagsXml(const std::vector<Nx::DiagCode>& diags)
{
	CString strBillDiagsXml;
	strBillDiagsXml.Preallocate(32 + (64 * diags.size()));

	strBillDiagsXml += "<BillDiags>";
	int orderIndex = 0;
	for each (const Nx::DiagCode& diag in diags) {
		++orderIndex;

		strBillDiagsXml.AppendFormat(
			"<Diag><ID>%li</ID><OrderIndex>%li</OrderIndex></Diag>"
			, diag.id
			, orderIndex
		);
	}
	strBillDiagsXml += "</BillDiags>";

	return strBillDiagsXml;
}

// (j.jones 2011-04-20 10:45) - PLID 41490 - This function supports the ability for claims to ignore
//diagnosis codes not linked to charges. Taking in a Bill ID and an array of Charge IDs that will be
//sent on the claim, this function will fill aryNewDiagnosisCodesToExport with an ordered list of
//new diagnosis code IDs,aryNewDiagnosisDescriptionsToExport with their descriptions,
//and a list of diagnosis indexes that need to be skipped.
//Returns FALSE if the preference for this is off, or if nothing needs to be skipped.
//Returns TRUE only if something was skipped.
// (j.jones 2013-08-09 12:47) - PLID 57955 - Renamed to reflect that this is now called
// at all times, it might skip diagnosis codes based on the SkipUnlinkedDiagsOnClaims preference,
// but it will fill our arrays even if none are skipped.
// bUseICD10 to true to use ICD10, false to use ICD-9.
// (a.walling 2014-03-05 16:05) - PLID 61202 - GetDiagnosisCodesForHCFA - Support ICD10 option, handle ChargeWhichCodesT
// Deprecated now by GetUniqueDiagnosisCodesForHCFA for forms and GetAllBillDiagCodes for getting a vector of data
// (a.walling 2014-03-19 10:05) - PLID 61346 - The entire confusing 'skipped diag indexes' concept is going away
//BOOL GetDiagnosisCodesForHCFA(long nBillID, IN CArray<long, long> &aryChargeIDsToExport,
//							  OUT CStringArray &aryDiagnosisCodesToExport,
//							  OUT CStringArray &aryDiagnosisDescriptionsToExport,
//							  OUT CArray<long, long> &arySkippedDiagIndexes,
//							  bool bUseICD10);

// (a.walling 2014-03-17 14:01) - PLID 61401 - returns coalesced unique DiagCodes for the bill according to the ICD9/ICD10 state and passed in charge, and 'onlylinkedcharges' preference, and handles whichcodes.
// strBillDiagsXml is output for convenience
void GetUniqueDiagnosisCodesForHCFA(long nBillID, IN CArray<long, long> &aryChargeIDsToExport,
							  OUT CStringArray &aryDiagnosisCodesToExport,
							  OUT CStringArray &aryDiagnosisDescriptionsToExport,
							  OUT CString& strBillDiagsXml,
							  bool bUseICD10)
{
	//default our final list to all codes
	strBillDiagsXml.Empty();
	aryDiagnosisCodesToExport.RemoveAll();
	aryDiagnosisDescriptionsToExport.RemoveAll();

	// (j.jones 2013-08-09 12:49) - PLID 57955 - check the preference now to see if we need to skip diagnoses not selected with WhichCodes
	bool bSkipUnlinkedDiagsOnClaims = !!GetRemotePropertyInt("SkipUnlinkedDiagsOnClaims", 0, 0, "<None>", true);

	// (a.walling 2014-03-19 10:05) - PLID 61346 - Call GetBillDiagCodes
	std::vector<Nx::DiagCode> diags = GetBillDiagCodes(nBillID, bUseICD10, bSkipUnlinkedDiagsOnClaims, &aryChargeIDsToExport);
	
	strBillDiagsXml = MakeBillDiagsXml(diags);

	for each (const Nx::DiagCode& diag in diags) {
		aryDiagnosisCodesToExport.Add(diag.number);
		aryDiagnosisDescriptionsToExport.Add(diag.description);
	}
}

// (j.jones 2011-04-20 11:28) - PLID 43330 - added a function to calculate
// how to load the diagnosis codes & descriptions on the claim
// (j.jones 2013-08-09 11:53) - PLID 57955 - Reworked this function, adding support for 12 diag codes (still only 4 description fields).
// This function simply takes in all the diag codes that should be on the array, and returns appropriate _Q()-escaped SQL-ready
// strings for diag fields 1 - 12 and diag desc. fields 1 - 4.
// Example: "'123.4' AS DiagCode, '432.1' AS DiagCode2," etc.
void CalculateHCFADiagCodeFields(IN CStringArray &aryNewDiagnosisCodesToExport,
								 IN CStringArray &aryNewDiagnosisCodeDescriptionsToExport,
								 OUT CString &strTop12DiagCodeFields, OUT CString &strTop4DiagCodeDescFields)
{
	//Normally we will load diag. codes & descriptions from a recordset such as:
	//DiagCodes1.CodeNumber AS DiagCode, DiagCodes2.CodeNumber AS DiagCode2, etc.
	//DiagCodes1.CodeDesc AS DiagCode1Desc, DiagCodes2.CodeDesc AS DiagCode2Desc, etc.

	//if we are skipping diagnosis codes, we'll have recalculated which codes to export,
	//and need to update the query's fields accordingly

	//first initialize each field to be blank text, incase we skipped all codes
	CString strDiagCode1Field = "''";
	CString strDiagCode2Field = "''";
	CString strDiagCode3Field = "''";
	CString strDiagCode4Field = "''";
	CString strDiagCode5Field = "''";
	CString strDiagCode6Field = "''";
	CString strDiagCode7Field = "''";
	CString strDiagCode8Field = "''";
	CString strDiagCode9Field = "''";
	CString strDiagCode10Field = "''";
	CString strDiagCode11Field = "''";
	CString strDiagCode12Field = "''";
	CString strDiagCode1DescField = "''";
	CString strDiagCode2DescField = "''";
	CString strDiagCode3DescField = "''";
	CString strDiagCode4DescField = "''";

	//now fill these fields with whatever data we have
	// (j.jones 2013-08-09 11:56) - PLID 57955 - now supports a maximum of 12 codes
	for(int i=0; i<aryNewDiagnosisCodesToExport.GetSize() && i<12; i++) {
		
		CString strDiagCode, strDiagCodeDesc;
		
		strDiagCode.Format("'%s'", _Q(aryNewDiagnosisCodesToExport.GetAt(i)));
		strDiagCodeDesc = "''";

		if(aryNewDiagnosisCodeDescriptionsToExport.GetSize() > i) {

			strDiagCodeDesc.Format("'%s'", _Q(aryNewDiagnosisCodeDescriptionsToExport.GetAt(i)));
		}
		else {
			//while we don't actually care about descriptions past the 4th code,
			//it should still be impossible for the arrays to be different sizes, so
			//ASSERT if they are, but use a blank description

			ASSERT(FALSE);
			strDiagCodeDesc = "''";
		}
	
		//now we have our fields, which index are we on?
		switch(i) {
			case 0:
				strDiagCode1Field = strDiagCode;
				strDiagCode1DescField = strDiagCodeDesc;
				break;
			case 1:
				strDiagCode2Field = strDiagCode;
				strDiagCode2DescField = strDiagCodeDesc;
				break;
			case 2:
				strDiagCode3Field = strDiagCode;
				strDiagCode3DescField = strDiagCodeDesc;
				break;
			case 3:
				strDiagCode4Field = strDiagCode;
				strDiagCode4DescField = strDiagCodeDesc;
				break;
			// (j.jones 2013-08-09 11:57) - PLID 57955 - codes 5 and above do not track descriptions
			case 4:
				strDiagCode5Field = strDiagCode;
				break;
			case 5:
				strDiagCode6Field = strDiagCode;
				break;
			case 6:
				strDiagCode7Field = strDiagCode;
				break;
			case 7:
				strDiagCode8Field = strDiagCode;
				break;
			case 8:
				strDiagCode9Field = strDiagCode;
				break;
			case 9:
				strDiagCode10Field = strDiagCode;
				break;
			case 10:
				strDiagCode11Field = strDiagCode;
				break;
			case 11:
				strDiagCode12Field = strDiagCode;
				break;
			default:
				//shouldn't be possible
				ASSERT(FALSE);
				break;
		}
	}

	//now build our SQL fields
	strTop12DiagCodeFields.Format("%s AS DiagCode, %s AS DiagCode2, %s AS DiagCode3, %s AS DiagCode4, "
		"%s AS DiagCode5, %s AS DiagCode6, %s AS DiagCode7, %s AS DiagCode8, "
		"%s AS DiagCode9, %s AS DiagCode10, %s AS DiagCode11, %s AS DiagCode12",
		strDiagCode1Field, strDiagCode2Field, strDiagCode3Field, strDiagCode4Field,
		strDiagCode5Field, strDiagCode6Field, strDiagCode7Field, strDiagCode8Field,
		strDiagCode9Field, strDiagCode10Field, strDiagCode11Field, strDiagCode12Field);
	strTop4DiagCodeDescFields.Format("%s AS DiagCode1Desc, %s AS DiagCode2Desc, %s AS DiagCode3Desc, %s AS DiagCode4Desc",
		strDiagCode1DescField, strDiagCode2DescField, strDiagCode2DescField, strDiagCode2DescField);
}

// (j.jones 2013-08-09 12:40) - PLID 57955 - Made a modular function to replace many code repeats of it.
// Takes in a bill ID and calculates all diagnosis codes that should show on that bill, returning appropriate
// _Q()-escaped SQL-ready strings for diag fields 1 - 12 and diag desc. fields 1 - 4.
// Example: "'123.4' AS DiagCode, '432.1' AS DiagCode2," etc.
// (a.walling 2014-03-06 08:47) - PLID 61216 - insured party ID for determining ICD10 state
void GenerateHCFADiagCodeFieldsForBill(bool bIsNewHCFA, long nBillID, long nInsuredPartyID, CSqlFragment sqlChargeFilter,
									   OUT CString &strTop12DiagCodeFields, OUT CString &strTop4DiagCodeDescFields,
									   OUT CString &strWhichCodes, OUT CString &strICDIndicator)
{
	//need to build an array of batched charges on the bill

	CArray<long, long> aryChargesToExport;
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID "
		"FROM BillsT "
		"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		""
		"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "				
		"LEFT JOIN ProvidersT ClaimProvidersT ON ActualProvidersT.ClaimProviderID = ClaimProvidersT.PersonID "
		"LEFT JOIN InsuredPartyT BillInsuredPartyT ON BillsT.InsuredPartyID = BillInsuredPartyT.PersonID "
		"LEFT JOIN HCFAClaimProvidersT AS HCFAClaimProvidersT ON ActualProvidersT.PersonID = HCFAClaimProvidersT.ProviderID AND BillInsuredPartyT.InsuranceCoID = HCFAClaimProvidersT.InsuranceCoID "
		""
		"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"{SQL} "
		"ORDER BY ChargesT.LineID", nBillID, sqlChargeFilter);

	while(!rsCharges->eof) {
		aryChargesToExport.Add(VarLong(rsCharges->Fields->Item["ChargeID"]->Value));

		rsCharges->MoveNext();
	}
	rsCharges->Close();

	CStringArray aryDiagnosisCodesToExport;
	CStringArray aryDiagnosisCodeDescriptionsToExport;
	CString strBillDiagsXml;

	// (a.walling 2014-03-05 16:29) - PLID 61202 - GetDiagnosisCodesForHCFA - Pass bUseICD10 as appropriate for this InsuredPartyID and BillID
	bool bUseICD10 = ShouldUseICD10(nInsuredPartyID, nBillID);

	//Now get all the diagnosis codes on this claim. This might skip diagnosis codes
	//based on the SkipUnlinkedDiagsOnClaims preference, but it will fill our arrays
	//even if none are skipped.
	// (a.walling 2014-03-17 14:01) - PLID 61401 - returns coalesced unique DiagCodes for the bill according to the ICD9/ICD10 state and passed in charge, and 'onlylinkedcharges' preference, and handles whichcodes.
	GetUniqueDiagnosisCodesForHCFA(nBillID, aryChargesToExport, aryDiagnosisCodesToExport, aryDiagnosisCodeDescriptionsToExport, strBillDiagsXml, bUseICD10);

	// (j.jones 2013-08-12 11:36) - PLID 57955 - this is now always the same function call,
	// it will turn indexes into letters if on the new HCFA
	// (a.walling 2014-03-10 13:29) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
	// (a.walling 2014-03-17 14:06) - PLID 61401 - dbo.MakeWhichCodes generates the appropriate WhichCodes string given the Diag xml for the bill and the Diag xml for the charge. dbo.GetChargeWhichDiagsXml returns the appropriate Diag xml for the charge according to ICD9/10 state.
	strWhichCodes.Format("dbo.MakeWhichCodes('%s', dbo.GetChargeWhichDiagsXml(ChargesT.ID, %li), %li)"
		, _Q(strBillDiagsXml)
		, bUseICD10 ? 1 : 0
		, bIsNewHCFA ? 1 : 0
	);

	//this function will build our fields to plug into the query
	CalculateHCFADiagCodeFields(aryDiagnosisCodesToExport, aryDiagnosisCodeDescriptionsToExport,
		strTop12DiagCodeFields, strTop4DiagCodeDescFields);

	//strICDIndicator is a one digit code. This is Box 21's ICD Ind. field.
	//If the codes are ICD-10, strICDIndicator is 0. Not ten. Zero.
	//If the codes are ICD-9, strICDIndicator is 9.
	if(bUseICD10) {
		strICDIndicator = "0";
	}
	else {
		strICDIndicator = "9";
	}
}

// (j.jones 2011-04-26 17:20) - PLID 42705 - added function that safely changes an allowable on a charge,
// with proper auditing if necessary
void SaveChargeAllowable(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID,
						 COleCurrency cyAllowable, ChargeAllowableEntryMethod caemEntryMethod)
{
	try {

		// (j.jones 2011-06-24 09:33) - PLID 44314 - this should not be called with a -1 charge ID
		if(nChargeID == -1) {
			ThrowNxException("SaveChargeAllowable called with a -1 charge ID!");
		}

		CAuditTransaction atAuditTrans;
		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//is there already 
		_RecordsetPtr rsOldAllowable = CreateParamRecordset("SELECT "
			"ChargeAllowablesT.Allowable, ChargeAllowablesT.InputDate, ChargeAllowablesT.EntryMethod, "
			"ChargesT.ItemCode, InsuranceCoT.Name AS InsCoName "
			"FROM ChargeAllowablesT "
			"INNER JOIN ChargesT ON ChargeAllowablesT.ChargeID = ChargesT.ID "
			"INNER JOIN InsuredPartyT ON ChargeAllowablesT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE ChargeAllowablesT.ChargeID = {INT} AND ChargeAllowablesT.InsuredPartyID = {INT}", nChargeID, nInsuredPartyID);
		if(!rsOldAllowable->eof) {
			//there is an old allowable, we will need to change it

			//always remove the old entry, we'll recreate one with a new date
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ChargeAllowablesT "
				"WHERE ChargeID = {INT} AND InsuredPartyID = {INT}", nChargeID, nInsuredPartyID);

			//audit the change ONLY if the allowable value changed
			COleCurrency cyOldAllowable = VarCurrency(rsOldAllowable->Fields->Item["Allowable"]->Value);
			if(cyOldAllowable != cyAllowable) {
				//audit this
				CString strOldValue, strNewValue;
				CString strOldEntryMethod, strNewEntryMethod;

				ChargeAllowableEntryMethod caemOldEntryMethod = (ChargeAllowableEntryMethod)VarLong(rsOldAllowable->Fields->Item["EntryMethod"]->Value);
				switch(caemOldEntryMethod) {
					case caemERemittance:
						strOldEntryMethod = "E-Remittance";
						break;
					case caemLineItemPosting:
						strOldEntryMethod = "Line Item Posting";
						break;
					case caemManualPayment:
						strOldEntryMethod = "Payment Entry";
						break;
					default:
						ASSERT(FALSE);
						strOldEntryMethod = "<Unknown>";
						break;
				}

				switch(caemEntryMethod) {
					case caemERemittance:
						strNewEntryMethod = "E-Remittance";
						break;
					case caemLineItemPosting:
						strNewEntryMethod = "Line Item Posting";
						break;
					case caemManualPayment:
						strNewEntryMethod = "Payment Entry";
						break;
					default:
						ASSERT(FALSE);
						strNewEntryMethod = "<Unknown>";
						break;
				}

				strOldValue.Format("%s from %s for code %s (entered on %s from %s)",					
					FormatCurrencyForInterface(cyOldAllowable),
					VarString(rsOldAllowable->Fields->Item["InsCoName"]->Value, ""),
					VarString(rsOldAllowable->Fields->Item["ItemCode"]->Value, ""),
					FormatDateTimeForInterface(VarDateTime(rsOldAllowable->Fields->Item["InputDate"]->Value), NULL, dtoDate),
					strOldEntryMethod);

				strNewValue.Format("%s (entered from %s)",
					FormatCurrencyForInterface(cyAllowable), strNewEntryMethod);

				AuditEvent(nPatientID, strPatientName, atAuditTrans, aeiChargeAllowable, nChargeID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
		}
		rsOldAllowable->Close();

		//save the new record, always
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO ChargeAllowablesT (ChargeID, InsuredPartyID, Allowable, InputDate, EntryMethod) "
			"VALUES ({INT}, {INT}, Convert(money, {STRING}), GetDate(), {INT}) ",
			nChargeID, nInsuredPartyID, FormatCurrencyForSql(cyAllowable), (long)caemEntryMethod);

		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushMaxRecordsWarningLimit pmr(2);
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//there might not be anything in our audit transaction if
		//the allowable didn't change, but it gracefully does nothing
		//when the audit batch is empty
		atAuditTrans.Commit();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-12-19 15:42) - PLID 43925 - Added ability to save deductible and coinsurance info. to a charge.
// If we only have one of the two, the other currency should be $0.00.
// (j.jones 2012-08-14 14:27) - PLID 50285 - Now returns TRUE if it created a new record, FALSE if it updated an existing
// record. This won't often be referenced, but it's needed for calling code to tell if this is the first time we've saved
// this data for a charge & insured party.
// (j.jones 2013-08-27 10:58) - PLID 57398 - added copay
BOOL SaveChargeCoinsurance(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID,
						   COleCurrency cyDeductible, COleCurrency cyCoinsurance, COleCurrency cyCopay,
						   ChargeCoinsuranceEntryMethod ccemEntryMethod)
{
	try {

		BOOL bIsNewRecord = TRUE;

		//this should not be called with a -1 charge ID
		if(nChargeID == -1) {
			ThrowNxException("SaveChargeCoinsurance called with a -1 charge ID!");
		}

		CAuditTransaction atAuditTrans;
		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//is there already a record?
		// (j.jones 2013-08-27 10:58) - PLID 57398 - added copay
		_RecordsetPtr rsOldCoinsurance = CreateParamRecordset("SELECT "
			"ChargeCoinsuranceT.Deductible, ChargeCoinsuranceT.Coinsurance, ChargeCoinsuranceT.Copay, "
			"ChargeCoinsuranceT.InputDate, ChargeCoinsuranceT.EntryMethod, "
			"ChargesT.ItemCode, InsuranceCoT.Name AS InsCoName "
			"FROM ChargeCoinsuranceT "
			"INNER JOIN ChargesT ON ChargeCoinsuranceT.ChargeID = ChargesT.ID "
			"INNER JOIN InsuredPartyT ON ChargeCoinsuranceT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE ChargeCoinsuranceT.ChargeID = {INT} AND ChargeCoinsuranceT.InsuredPartyID = {INT}", nChargeID, nInsuredPartyID);
		if(!rsOldCoinsurance->eof) {
			//there is an old entry, we will need to change it

			CString strOldEntryMethod;
			ChargeCoinsuranceEntryMethod ccemOldEntryMethod = (ChargeCoinsuranceEntryMethod)VarLong(rsOldCoinsurance->Fields->Item["EntryMethod"]->Value);
			switch(ccemOldEntryMethod) {
				case ccemERemittance:
					strOldEntryMethod = "E-Remittance";
					break;
				case ccemLineItemPosting:
					strOldEntryMethod = "Line Item Posting";
					break;
				case ccemManualPayment:
					strOldEntryMethod = "Payment Entry";
					break;
				default:
					ASSERT(FALSE);
					strOldEntryMethod = "<Unknown>";
					break;
			}

			//audit the change ONLY if the deductible or coinsurance value changed
			COleCurrency cyOldDeductible = VarCurrency(rsOldCoinsurance->Fields->Item["Deductible"]->Value);
			if(cyOldDeductible != cyDeductible) {
				//audit this
				CString strOldValue, strNewValue;
				CString strNewEntryMethod;

				switch(ccemEntryMethod) {
					case ccemERemittance:
						strNewEntryMethod = "E-Remittance";
						break;
					case ccemLineItemPosting:
						strNewEntryMethod = "Line Item Posting";
						break;
					case ccemManualPayment:
						strNewEntryMethod = "Payment Entry";
						break;
					default:
						ASSERT(FALSE);
						strNewEntryMethod = "<Unknown>";
						break;
				}

				strOldValue.Format("%s from %s for code %s (entered on %s from %s)",					
					FormatCurrencyForInterface(cyOldDeductible),
					VarString(rsOldCoinsurance->Fields->Item["InsCoName"]->Value, ""),
					VarString(rsOldCoinsurance->Fields->Item["ItemCode"]->Value, ""),
					FormatDateTimeForInterface(VarDateTime(rsOldCoinsurance->Fields->Item["InputDate"]->Value), NULL, dtoDate),
					strOldEntryMethod);

				strNewValue.Format("%s (entered from %s)",
					FormatCurrencyForInterface(cyDeductible), strNewEntryMethod);

				AuditEvent(nPatientID, strPatientName, atAuditTrans, aeiChargeDeductible, nChargeID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			COleCurrency cyOldCoinsurance = VarCurrency(rsOldCoinsurance->Fields->Item["Coinsurance"]->Value);
			if(cyOldCoinsurance != cyCoinsurance) {
				//audit this
				CString strOldValue, strNewValue;
				CString strNewEntryMethod;

				switch(ccemEntryMethod) {
					case ccemERemittance:
						strNewEntryMethod = "E-Remittance";
						break;
					case ccemLineItemPosting:
						strNewEntryMethod = "Line Item Posting";
						break;
					case ccemManualPayment:
						strNewEntryMethod = "Payment Entry";
						break;
					default:
						ASSERT(FALSE);
						strNewEntryMethod = "<Unknown>";
						break;
				}

				strOldValue.Format("%s from %s for code %s (entered on %s from %s)",					
					FormatCurrencyForInterface(cyOldCoinsurance),
					VarString(rsOldCoinsurance->Fields->Item["InsCoName"]->Value, ""),
					VarString(rsOldCoinsurance->Fields->Item["ItemCode"]->Value, ""),
					FormatDateTimeForInterface(VarDateTime(rsOldCoinsurance->Fields->Item["InputDate"]->Value), NULL, dtoDate),
					strOldEntryMethod);

				strNewValue.Format("%s (entered from %s)",
					FormatCurrencyForInterface(cyCoinsurance), strNewEntryMethod);

				AuditEvent(nPatientID, strPatientName, atAuditTrans, aeiChargeCoinsurance, nChargeID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			// (j.jones 2013-08-27 10:58) - PLID 57398 - added copay
			COleCurrency cyOldCopay = VarCurrency(rsOldCoinsurance->Fields->Item["Copay"]->Value);
			if(cyOldCopay != cyCopay) {
				//audit this
				CString strOldValue, strNewValue;
				CString strNewEntryMethod;

				switch(ccemEntryMethod) {
					case ccemERemittance:
						strNewEntryMethod = "E-Remittance";
						break;
					case ccemLineItemPosting:
						strNewEntryMethod = "Line Item Posting";
						break;
					case ccemManualPayment:
						strNewEntryMethod = "Payment Entry";
						break;
					default:
						ASSERT(FALSE);
						strNewEntryMethod = "<Unknown>";
						break;
				}

				strOldValue.Format("%s from %s for code %s (entered on %s from %s)",					
					FormatCurrencyForInterface(cyOldCopay),
					VarString(rsOldCoinsurance->Fields->Item["InsCoName"]->Value, ""),
					VarString(rsOldCoinsurance->Fields->Item["ItemCode"]->Value, ""),
					FormatDateTimeForInterface(VarDateTime(rsOldCoinsurance->Fields->Item["InputDate"]->Value), NULL, dtoDate),
					strOldEntryMethod);

				strNewValue.Format("%s (entered from %s)",
					FormatCurrencyForInterface(cyCopay), strNewEntryMethod);

				AuditEvent(nPatientID, strPatientName, atAuditTrans, aeiChargeCopay, nChargeID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			// (j.jones 2012-08-14 14:29) - PLID 50285 - track that this was an existing record that we changed
			// as opposed to a brand new entry for this charge & insured party
			bIsNewRecord = FALSE;

			//always remove the old entry, we'll recreate one with a new date
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ChargeCoinsuranceT "
				"WHERE ChargeID = {INT} AND InsuredPartyID = {INT}", nChargeID, nInsuredPartyID);
		}
		rsOldCoinsurance->Close();

		//save the new record, always
		// (j.jones 2013-08-27 10:58) - PLID 57398 - added copay
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO ChargeCoinsuranceT (ChargeID, InsuredPartyID, Deductible, Coinsurance, Copay, InputDate, EntryMethod) "
			"VALUES ({INT}, {INT}, {OLECURRENCY}, {OLECURRENCY}, {OLECURRENCY}, GetDate(), {INT}) ",
			nChargeID, nInsuredPartyID, cyDeductible, cyCoinsurance, cyCopay, (long)ccemEntryMethod);

		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushMaxRecordsWarningLimit pmr(2);
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//there might not be anything in our audit transaction if
		//the values didn't change, but it gracefully does nothing
		//when the audit batch is empty
		atAuditTrans.Commit();

		return bIsNewRecord;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.dinatale 2012-02-01 15:24) - PLID 45511 - this is now a non beta feature
// (j.jones 2011-09-15 15:21) - PLID 44905 - Added IsLineItemCorrectionsEnabled_Beta,
// which will be removed when this feature is out of beta. Its return value determines
// if line item correction functionality is enabled.
/*BOOL IsLineItemCorrectionsEnabled_Beta()
{
	long nIsEnabled = GetRemotePropertyInt("IsLineItemCorrectionsEnabled_Beta", 0, 0, "<None>", true);

	return nIsEnabled == 1;
}*/

// (j.jones 2011-07-22 09:24) - PLID 44662 - made this be a global function
long CalculateAlbertaClaimNumberCheckDigit(CString strSequenceNumber)
{
	//this should have been given a 7 digit string
	if(strSequenceNumber.GetLength() != 7) {
		ThrowNxException("CalculateAlbertaClaimNumberCheckDigit called with invalid sequence number %s!", strSequenceNumber);
	}

	// (j.jones 2013-04-10 16:27) - PLID 56191 - Alberta has a convoluted set of documentation
	// on how to calculate their sequence number. What they don't admit to, is that it's just
	// the Luhn algorithm.
	// So call CalculateLuhn on our sequence number - the result is the Alberta check digit.
	return CalculateLuhn(strSequenceNumber);
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - IsOriginalOrVoidLineItem
// returns TRUE if the line item is an original or corrected line item,
// or a balancing adjustment. These line items are read-only.
BOOL IsOriginalOrVoidLineItem(long nLineItemID)
{
	if(nLineItemID < 0) {
		//nobody should have called this on a negative line item ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT ID "
		"FROM LineItemT "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemQ.VoidingLineItemID "
		"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
		"WHERE LineItemT.ID = {INT} "
		"AND (LineItemCorrections_OriginalLineItemQ.OriginalLineItemID Is Not Null "
		"	OR LineItemCorrections_VoidingLineItemQ.VoidingLineItemID Is Not Null "
		"	OR LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Not Null)", nLineItemID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - If the source line item for a given
// apply is part of a correction (ie. it is an original or void item, or a balancing
// adjustment), you cannot unapply it from where it is currently applied.
BOOL IsOriginalOrVoidApply(long nApplyID)
{
	if(nApplyID < 0) {
		//nobody should have called this on a negative ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.ID "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemQ.VoidingLineItemID "
		"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
		"WHERE LineItemT.Deleted = 0 "
		"AND AppliesT.ID = {INT} "
		"AND (LineItemCorrections_OriginalLineItemQ.OriginalLineItemID Is Not Null "
		"	OR LineItemCorrections_VoidingLineItemQ.VoidingLineItemID Is Not Null "
		"	OR LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Not Null)", nApplyID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - IsVoidedBill
// returns TRUE if the bill is an original/voided bill.
// There is only one bill for this, not two. This bill is read-only.
BOOL IsVoidedBill(long nBillID)
{
	if(nBillID < 0) {
		//nobody should have called this on a negative bill ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT ID "
		"FROM BillCorrectionsT "
		"WHERE OriginalBillID = {INT}", nBillID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - DoesBillHaveOriginalOrVoidCharges
// returns TRUE if the bill has charges in it that are Original or Void,
// which are read-only.
BOOL DoesBillHaveOriginalOrVoidCharges(long nBillID)
{
	if(nBillID < 0) {
		//nobody should have called this on a negative bill ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ChargesT.ID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"WHERE Deleted = 0 AND ChargesT.BillID = {INT} "
		"AND (LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Not Null "
		"OR LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Not Null)",
		nBillID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - DoesBillHaveUncorrectedCharges
// returns TRUE if the bill has charges in it that are NOT Original or Void,
// which are not read-only, which means the bill can be re-corrected.
BOOL DoesBillHaveUncorrectedCharges(long nBillID)
{
	if(nBillID < 0) {
		//nobody should have called this on a negative bill ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ChargesT.ID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"WHERE Deleted = 0 AND ChargesT.BillID = {INT} "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",
		nBillID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - GetLineItemCorrectionStatus
// returns a LineItemCorrectionsStatus enum for whether the line item
// has not been involved in a correction, or is an Original, Void, or
// Corrected line item. Original and Void line items are read-only.
// If a Corrected line item is later corrected itself, it would return
// licsOriginal, not licsCorrected.
LineItemCorrectionStatus GetLineItemCorrectionStatus(long nLineItemID)
{
	if(nLineItemID < 0) {
		//nobody should have called this on a negative line item ID
		return licsNormal;
	}

	//we sort such that if a line item was a NewLineItem, and then later
	//an OriginalLineItem, the "original" status would be found first
	//(to handle the event that you correct an existing correction)
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
		"Convert(bit, CASE WHEN OriginalLineItemID = {INT} THEN 1 ELSE 0 END) AS IsOriginal, "
		"Convert(bit, CASE WHEN VoidingLineItemID = {INT} OR BalancingAdjID = {INT} THEN 1 ELSE 0 END) AS IsVoid, "
		"Convert(bit, CASE WHEN NewLineItemID = {INT} THEN 1 ELSE 0 END) AS IsNew "
		"FROM LineItemCorrectionsT "
		"LEFT JOIN LineItemCorrectionsBalancingAdjT ON LineItemCorrectionsT.ID = LineItemCorrectionsBalancingAdjT.LineItemCorrectionID "
		"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} "
		"OR NewLineItemID = {INT} OR LineItemCorrectionsBalancingAdjT.BalancingAdjID = {INT} "
		"ORDER BY (CASE WHEN OriginalLineItemID = {INT} THEN 0 ELSE 1 END) ASC",
		nLineItemID, nLineItemID, nLineItemID, nLineItemID,
		nLineItemID, nLineItemID, nLineItemID, nLineItemID,
		nLineItemID);
	if(rs->eof) {
		//not used in any financial line item correction
		return licsNormal;
	}
	else {
		BOOL bIsOriginal = VarBool(rs->Fields->Item["IsOriginal"]->Value, FALSE);
		BOOL bIsVoid = VarBool(rs->Fields->Item["IsVoid"]->Value, FALSE);
		BOOL bIsNew = VarBool(rs->Fields->Item["IsNew"]->Value, FALSE);

		if(bIsOriginal) {
			//this line item is an Original line item,
			//meaning it has been corrected, and is now read-only
			return licsOriginal;
		}
		else if(bIsVoid) {
			//this line item is an Voiding line item,
			//meaning it cancelled out another line item
			//and is now read-only
			//(cancelling means it could be an adjustment to balance
			//out a charge, or a voiding line item)
			return licsVoid;
		}
		else if(bIsNew) {
			//this line item is a New line item,
			//meaning it replaced another line item,
			//and is now editable
			return licsCorrected;
		}
	}

	return licsNormal;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - DoesBillHaveOriginalOrVoidApplies
// returns TRUE if the given bill has any apply which cannot be deleted,
// and therefore the bill cannot be deleted.
BOOL DoesBillHaveOriginalOrVoidApplies(long nBillID)
{
	if(nBillID < 0) {
		//nobody should have called this on a negative ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.ID "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemQ.VoidingLineItemID "
		"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
		"WHERE LineItemT.Deleted = 0 "
		"AND ChargesT.BillID = {INT} "
		"AND (LineItemCorrections_OriginalLineItemQ.OriginalLineItemID Is Not Null "
		"	OR LineItemCorrections_VoidingLineItemQ.VoidingLineItemID Is Not Null "
		"	OR LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Not Null)", nBillID);
	return !rs->eof;
}

// (j.jones 2011-09-13 15:37) - PLID 44887 - DoesLineItemHaveOriginalOrVoidApplies
// returns TRUE if the given charge/payment has any apply which cannot be deleted,
// and therefore the charge cannot be deleted.
BOOL DoesLineItemHaveOriginalOrVoidApplies(long nLineItemID)
{
	if(nLineItemID < 0) {
		//nobody should have called this on a negative ID
		return FALSE;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.ID "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemQ.OriginalLineItemID " 
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemQ.VoidingLineItemID "
		"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
		"WHERE LineItemT.Deleted = 0 "
		"AND AppliesT.DestID = {INT} "
		"AND (LineItemCorrections_OriginalLineItemQ.OriginalLineItemID Is Not Null "
		"	OR LineItemCorrections_VoidingLineItemQ.VoidingLineItemID Is Not Null "
		"	OR LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Not Null)", nLineItemID);
	return !rs->eof;
}

// (j.jones 2011-08-24 17:18) - PLID 45176 - checks if the user has permission to apply, whether the source item is closed,
// and whether they can apply closed line items
// (j.jones 2013-07-01 09:42) - PLID 55517 - Now can potentially auto-correct the source item, if closed.
// If so, nSourcePayID will be changed.
ECanApplyLineItemResult CanApplyLineItem(IN OUT long &nSourcePayID, BOOL bSilent /*= FALSE*/)
{
	try {

		// This should never happen, but check for it
		if (-1 == nSourcePayID) {
			ASSERT(FALSE);
			return ecalirCannotApply;
		}

		// (j.jones 2013-07-01 13:04) - PLID 55517 - Cache whether they have permission
		// to correct line items. They might need a password.
		bool bHasCorrectionPermission = !(GetCurrentUserPermissions(bioFinancialCorrections) & (sptWrite|sptWriteWithPass)) ? false : true;
		bool bSourcePayIDChanged = false;

		COleDateTime dtItemInputDate = g_cdtInvalid;
		COleDateTime dtItemServiceDate = g_cdtInvalid;

		//check our last date/time of a hard close
		_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseAsOfDate, "
			"Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT");
		if(!rsHardClose->eof) {
			//this is nullable since this is a Max()
			_variant_t varHardCloseAsOfDate = rsHardClose->Fields->Item["HardCloseAsOfDate"]->Value;
			_variant_t varHardCloseInputDate = rsHardClose->Fields->Item["HardCloseInputDate"]->Value;
			COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
			COleDateTime dtHardCloseInputDate = g_cdtInvalid;
			//impossible for one to be NULL and not the other
			if(varHardCloseAsOfDate.vt == VT_DATE || varHardCloseInputDate.vt == VT_DATE) {
				dtHardCloseAsOfDate = VarDateTime(varHardCloseAsOfDate);					
				dtHardCloseInputDate = VarDateTime(varHardCloseInputDate);
			}
			
			if(dtHardCloseAsOfDate.GetStatus() != COleDateTime::invalid
				|| dtHardCloseInputDate.GetStatus() != COleDateTime::invalid) {
				//they have closed the data in the past
				
				// even if you have permission to override, we have to open
				// a recordset to check the input date, because the user
				// would still have to be warned first before editing

				// If the service date is <= the close date and the input date is <= the
				// CLOSE input date, it is also locked. Items with a service date <= the close date
				// but their input date is after the input date of the close are not locked.
				BOOL bIsClosedByInputDate = FALSE;
				BOOL bIsClosedByServiceDate = FALSE;
				long nType = 1;
				CString strSourceType = "payment";
				
				_RecordsetPtr rs = CreateParamRecordset("SELECT Type, Date, InputDate, "
					"Convert(bit, CASE WHEN InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
					"Convert(bit, CASE WHEN InputDate <= {VT_DATE} AND Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
					"FROM LineItemT "
					"WHERE ID = {INT} "
					"AND (InputDate <= {VT_DATE} "
					" OR (InputDate <= {VT_DATE} AND Date <= {VT_DATE}))",
					varHardCloseAsOfDate,
					varHardCloseInputDate, varHardCloseAsOfDate,
					nSourcePayID,
					varHardCloseAsOfDate,
					varHardCloseInputDate, varHardCloseAsOfDate);
				if(!rs->eof) {
					dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
					bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
					bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
					nType = VarLong(rs->Fields->Item["Type"]->Value);
					switch(nType) {
						case 1:
							strSourceType = "payment";
							break;
						case 3:
							strSourceType = "refund";
							break;
						case 2:
						case 0: //not valid, but assume adjustment
							strSourceType = "adjustment";
							break;
						default:
							//whatever we say the source type is, it's going to be wrong
							ASSERT(FALSE);
							break;
					}
				}
				rs->Close();

				//is it closed?
				if(bIsClosedByInputDate || bIsClosedByServiceDate) {

					//can they bypass a hard close?
					ECanOverrideHardClose eCanOverride = cohcNo;
					if(!(GetCurrentUserPermissions(bioApplyCreatePastClose) & sptCreate)) {
						// If we get here, the user doesn't have override permission. Check for
						// override with password.
						if((GetCurrentUserPermissions(bioApplyCreatePastClose) & sptCreateWithPass)) { // With password
							//they will need a password
							eCanOverride = cohcWithPass;
						}
						else {
							eCanOverride = cohcNo;
						}
					}
					else {				
						eCanOverride = cohcYes; // User has permission to override the preference.
					}

					bool bCorrectLineItem = false;

					//if they can't bypass it, warn and quit
					if(eCanOverride == cohcNo) {
						
						if(!bSilent) {
							//explain why it is closed based on input or service date
							CString strMessage;

							if(bIsClosedByInputDate) {
								//the item is closed based on its input date, which is before the "as of" close date
								if(!bHasCorrectionPermission) {
									strMessage.Format("You do not have permission to apply this %s because its input date of %s is before the most recent closing date of %s.", strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
									// line items, change the message to ask if they wish to do that.
									strMessage.Format("You do not have permission to apply this %s because its input date of %s is before the most recent closing date of %s.\n\n"
										"Would you like to void and correct this %s and apply the corrected %s?", strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										strSourceType, strSourceType);
								}
							}
							else {
								//the item is closed because its service date is before the "as of" close date,
								//its input date is before the close input date, so it is still closed anyways
								if(!bHasCorrectionPermission) {
									strMessage.Format("You do not have permission to apply this %s because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.",
										strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
									// line items, change the message to ask if they wish to do that.
									strMessage.Format("You do not have permission to apply this %s because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.\n\n"
										"Would you like to void and correct this %s and apply the corrected %s?",
										strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime),
										strSourceType, strSourceType);
								}
							}

							if(!bHasCorrectionPermission) {
								MessageBox(GetActiveWindow(), strMessage, "Practice", MB_OK | MB_ICONERROR);
								return ecalirCannotApply;
							}
							else {
								// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
								// line items, the message will ask if they want to void & correct.
								if(IDNO == MessageBox(GetActiveWindow(), strMessage, "Practice", MB_YESNO | MB_ICONQUESTION)) {
									return ecalirCannotApply;
								}
								else {
									//flag that we need to correct this line item
									bCorrectLineItem = true;
								}
							}
						}
					}
					// (j.jones 2011-01-21 09:29) - PLID 42156 - suppress this prompt if bSilent
					else if(!bSilent) {

						// (j.jones 2011-01-06 09:24) - PLID 42000 - if they can bypass it, warn first
						CString strWarn;
						// (j.jones 2011-01-20 14:12) - PLID 41999 - now we have to explain why it is closed based on input or service date
						if(bIsClosedByInputDate) {
							//the item is closed based on its input date, which is before the "as of" close date
							if(!bHasCorrectionPermission) {
								strWarn.Format("This %s has already been closed. Its input date of %s is before the most recent closing date of %s.\n\n",
									strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
							}
							else {
								// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
								// line items, change the message to ask if they wish to do that.
								strWarn.Format("This %s has already been closed. Its input date of %s is before the most recent closing date of %s.\n\n",
									strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
							}
						}
						else {
							//the item is closed because its service date is before the "as of" close date,
							//its input date is before the close input date, so it is still closed anyways
							if(!bHasCorrectionPermission) {
								strWarn.Format("This %s has already been closed. Its service date of %s is before the most recent closing date of %s, "
									"and its input date of %s is before the most recent close that was created on %s.\n\n",
									strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
							}
							else {
								// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
								// line items, change the message to ask if they wish to do that.
								strWarn.Format("This %s has already been closed. Its service date of %s is before the most recent closing date of %s, "
									"and its input date of %s is before the most recent close that was created on %s.\n\n",
									strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate),
									FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
									FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
							}
						}

						CString strQuestion;
						// (j.jones 2013-07-01 13:04) - PLID 55517 - If they have permission to correct
						// line items, change the message to ask if they wish to do that.
						// Otherwise, confirm they wish to apply the closed line item.
						if(!bHasCorrectionPermission) {
							strQuestion.Format("Are you absolutely certain you wish to apply this closed %s?", strSourceType);
						}
						else {
							strQuestion.Format("Would you like to void and correct this %s and apply the corrected %s?", strSourceType, strSourceType);
						}
						if(IDNO == MessageBox(GetActiveWindow(), strWarn + strQuestion, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
							if(!bHasCorrectionPermission) {
								//they were prompted to apply, and declined
								return ecalirCannotApply;
							}
							else {
								//they were prompted to void & correct, and declined,
								//so now ask if they want to just apply the closed item
								strQuestion.Format("Are you absolutely certain you wish to apply this closed %s?", strSourceType);
								if(IDNO == MessageBox(GetActiveWindow(), strQuestion, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
									return ecalirCannotApply;
								}
							}
						}
						else {
							//if they have the correction permission, they just agreed to
							//void & correct, so track that
							bCorrectLineItem = bHasCorrectionPermission;
						}

						//if they are simply applying the closed line item, check the password if needed
						if(!bCorrectLineItem) {
							if(eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
								//fail, but clarify why we failed
								MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to apply this %s.", strSourceType);
								return ecalirCannotApply;
							}
						}
					}

					//if we get here, the source item is closed, but you can apply it anyways

					// (j.jones 2013-07-01 13:10) - PLID 55517 - if bCorrectLineItem is true,
					// then we must void & correct first, suppressing some of the warnings
					if(bCorrectLineItem) {
						// (j.jones 2013-07-01 13:58) - PLID 55517 - try to correct the line item,
						// if it fails then they cannot 
						strSourceType.MakeLower();
						if(!VoidAndCorrectPayAdjRef(nSourcePayID, strSourceType, true, true)) {
							//don't try to ask if they want to apply anyways,
							//because they chose to correct, and it didn't work,
							//so abort
							return ecalirCannotApply;
						}
						else {
							//they can apply this item, but the ID changed
							bSourcePayIDChanged = true;

							//don't return, we'll check apply permissions at the end of this function
						}
					}
					else {
						//they can apply this item, and it was not corrected
						bSourcePayIDChanged = false;

						//don't return, we'll check apply permissions at the end of this function
					}
				}
			}
		}
		rsHardClose->Close();

		//now we can return if the user is an administrator
		if (IsCurrentUserAdministrator()) {
			if(bSourcePayIDChanged) {
				return ecalirCanApply_IDHasChanged;
			}
			else {
				return ecalirCanApply;
			}
		}

		//check the normal apply permission
		if(!CheckCurrentUserPermissions(bioApplies, sptCreate, FALSE, FALSE, bSilent)) {
			// No permission, or no valid password
			return ecalirCannotApply;
		} else {
			// They do have permission
			if(bSourcePayIDChanged) {
				return ecalirCanApply_IDHasChanged;
			}
			else {
				return ecalirCanApply;
			}
		}

	}NxCatchAll(__FUNCTION__);

	return ecalirCannotApply;
}

//same concept as CanApplyLineItem, checks their permissions to unapply a line item (closed or unclosed),
//and offers to auto-correct closed line items.
ECanApplyLineItemResult CanUnapplyLineItem(IN OUT long &nApplyID, BOOL bSilent /*= FALSE*/)
{
	try {

		// This should never happen, but check for it
		if (-1 == nApplyID) {
			ASSERT(FALSE);
			return ecalirCannotApply;
		}

		// Cache whether they have permission to correct line items. They might need a password.
		bool bHasCorrectionPermission = !(GetCurrentUserPermissions(bioFinancialCorrections) & (sptWrite | sptWriteWithPass)) ? false : true;
		bool bSourceIDsChanged = false;
		long nSourcePayID = -1;

		COleDateTime dtItemInputDate = g_cdtInvalid;
		COleDateTime dtItemServiceDate = g_cdtInvalid;

		//check our last date/time of a hard close
		_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseAsOfDate, "
			"Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT");
		if (!rsHardClose->eof) {
			//this is nullable since this is a Max()
			_variant_t varHardCloseAsOfDate = rsHardClose->Fields->Item["HardCloseAsOfDate"]->Value;
			_variant_t varHardCloseInputDate = rsHardClose->Fields->Item["HardCloseInputDate"]->Value;
			COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
			COleDateTime dtHardCloseInputDate = g_cdtInvalid;
			//impossible for one to be NULL and not the other
			if (varHardCloseAsOfDate.vt == VT_DATE || varHardCloseInputDate.vt == VT_DATE) {
				dtHardCloseAsOfDate = VarDateTime(varHardCloseAsOfDate);
				dtHardCloseInputDate = VarDateTime(varHardCloseInputDate);
			}

			if (dtHardCloseAsOfDate.GetStatus() != COleDateTime::invalid
				|| dtHardCloseInputDate.GetStatus() != COleDateTime::invalid) {
				//they have closed the data in the past

				// even if you have permission to override, we have to open
				// a recordset to check the input date, because the user
				// would still have to be warned first before editing

				// If the service date is <= the close date and the input date is <= the
				// CLOSE input date, it is also locked. Items with a service date <= the close date
				// but their input date is after the input date of the close are not locked.
				BOOL bIsClosedByInputDate = FALSE;
				BOOL bIsClosedByServiceDate = FALSE;
				long nType = 1;
				CString strSourceType = "payment";

				_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.ID, LineItemT.Type, LineItemT.Date, LineItemT.InputDate, "
					"Convert(bit, CASE WHEN LineItemT.InputDate <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByInput, "
					"Convert(bit, CASE WHEN LineItemT.InputDate <= {VT_DATE} AND LineItemT.Date <= {VT_DATE} THEN 1 ELSE 0 END) AS ClosedByService "
					"FROM LineItemT "
					"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
					"WHERE AppliesT.ID = {INT} "
					"AND (LineItemT.InputDate <= {VT_DATE} "
					" OR (LineItemT.InputDate <= {VT_DATE} AND LineItemT.Date <= {VT_DATE}))",
					varHardCloseAsOfDate,
					varHardCloseInputDate, varHardCloseAsOfDate,
					nApplyID,
					varHardCloseAsOfDate,
					varHardCloseInputDate, varHardCloseAsOfDate);
				if (!rs->eof) {
					nSourcePayID = VarLong(rs->Fields->Item["ID"]->Value);
					dtItemInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					dtItemServiceDate = VarDateTime(rs->Fields->Item["Date"]->Value);
					bIsClosedByInputDate = VarBool(rs->Fields->Item["ClosedByInput"]->Value);
					bIsClosedByServiceDate = VarBool(rs->Fields->Item["ClosedByService"]->Value);
					nType = VarLong(rs->Fields->Item["Type"]->Value);
					switch (nType) {
					case 1:
						strSourceType = "payment";
						break;
					case 3:
						strSourceType = "refund";
						break;
					case 2:
					case 0: //not valid, but assume adjustment
						strSourceType = "adjustment";
						break;
					default:
						//whatever we say the source type is, it's going to be wrong
						ASSERT(FALSE);
						break;
					}
				}
				rs->Close();

				//is it closed?
				if (bIsClosedByInputDate || bIsClosedByServiceDate) {

					//can they bypass a hard close?
					ECanOverrideHardClose eCanOverride = cohcNo;
					if (!(GetCurrentUserPermissions(bioApplyDeletePastClose) & sptCreate)) {
						// If we get here, the user doesn't have override permission. Check for
						// override with password.
						if ((GetCurrentUserPermissions(bioApplyDeletePastClose) & sptDeleteWithPass)) { // With password
																										//they will need a password
							eCanOverride = cohcWithPass;
						}
						else {
							eCanOverride = cohcNo;
						}
					}
					else {
						eCanOverride = cohcYes; // User has permission to override the preference.
					}

					bool bCorrectLineItem = false;

					//if they can't bypass it, warn and quit
					if (eCanOverride == cohcNo) {

						if (!bSilent) {
							//explain why it is closed based on input or service date
							CString strMessage;

							if (bIsClosedByInputDate) {
								//the item is closed based on its input date, which is before the "as of" close date
								if (!bHasCorrectionPermission) {
									strMessage.Format("You do not have permission to unapply this %s because its input date of %s is before the most recent closing date of %s.", strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									// If they have permission to correct line items, change the message to ask if they wish to do that.
									strMessage.Format("You do not have permission to unapply this %s because its input date of %s is before the most recent closing date of %s.\n\n"
										"Would you like to void and correct this %s and unapply the corrected %s?", strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										strSourceType, strSourceType);
								}
							}
							else {
								//the item is closed because its service date is before the "as of" close date,
								//its input date is before the close input date, so it is still closed anyways
								if (!bHasCorrectionPermission) {
									strMessage.Format("You do not have permission to unapply this %s because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.",
										strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
								}
								else {
									// If they have permission to correct line items, change the message to ask if they wish to do that.
									strMessage.Format("You do not have permission to unapply this %s because its service date of %s is before the most recent closing date of %s, "
										"and its input date of %s is before the most recent close that was created on %s.\n\n"
										"Would you like to void and correct this %s and unapply the corrected %s?",
										strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate), FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
										FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime),
										strSourceType, strSourceType);
								}
							}

							if (!bHasCorrectionPermission) {
								MessageBox(GetActiveWindow(), strMessage, "Practice", MB_OK | MB_ICONERROR);
								return ecalirCannotApply;
							}
							else {
								// If they have permission to correct line items, the message will ask if they want to void & correct.
								if (IDNO == MessageBox(GetActiveWindow(), strMessage, "Practice", MB_YESNO | MB_ICONQUESTION)) {
									return ecalirCannotApply;
								}
								else {
									//flag that we need to correct this line item
									bCorrectLineItem = true;
								}
							}
						}
					}
					// suppress this prompt if bSilent
					else if (!bSilent) {

						// if they can bypass it, warn first
						CString strWarn;
						// now we have to explain why it is closed based on input or service date
						if (bIsClosedByInputDate) {
							//the item is closed based on its input date, which is before the "as of" close date
							strWarn.Format("This %s has already been closed. Its input date of %s is before the most recent closing date of %s.\n\n",
								strSourceType, FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
								FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
						}
						else {
							//the item is closed because its service date is before the "as of" close date,
							//its input date is before the close input date, so it is still closed anyways
							strWarn.Format("This %s has already been closed. Its service date of %s is before the most recent closing date of %s, "
								"and its input date of %s is before the most recent close that was created on %s.\n\n",
								strSourceType, FormatDateTimeForInterface(dtItemServiceDate, NULL, dtoDate),
								FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
								FormatDateTimeForInterface(dtItemInputDate, DTF_STRIP_SECONDS, dtoDateTime),
								FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
						}

						CString strQuestion;
						// If they have permission to correct line items, change the message to ask if they wish to do that.
						// Otherwise, confirm they wish to apply the closed line item.
						if (!bHasCorrectionPermission) {
							strQuestion.Format("Are you absolutely certain you wish to unapply this closed %s?", strSourceType);
						}
						else {
							strQuestion.Format("Would you like to void and correct this %s and unapply the corrected %s?", strSourceType, strSourceType);
						}
						if (IDNO == MessageBox(GetActiveWindow(), strWarn + strQuestion, "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
							if (!bHasCorrectionPermission) {
								//they were prompted to unapply, and declined
								return ecalirCannotApply;
							}
							else {
								//they were prompted to void & correct, and declined,
								//so now ask if they want to just unapply the closed item
								strQuestion.Format("Are you absolutely certain you wish to unapply this closed %s?", strSourceType);
								if (IDNO == MessageBox(GetActiveWindow(), strQuestion, "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
									return ecalirCannotApply;
								}
							}
						}
						else {
							//if they have the correction permission, they just agreed to
							//void & correct, so track that
							bCorrectLineItem = bHasCorrectionPermission;
						}

						//if they are simply applying the closed line item, check the password if needed
						if (!bCorrectLineItem) {
							if (eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
								//fail, but clarify why we failed
								MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to unapply this %s.", strSourceType);
								return ecalirCannotApply;
							}
						}
					}

					//if we get here, the source item is closed, but you can apply it anyways

					// if bCorrectLineItem is true, then we must void & correct first, suppressing some of the warnings
					if (bCorrectLineItem) {
						// (j.jones 2013-07-01 13:58) - PLID 55517 - try to correct the line item,
						// if it fails then they cannot 
						strSourceType.MakeLower();
						if (!VoidAndCorrectPayAdjRef(nSourcePayID, strSourceType, true, true)) {
							//don't try to ask if they want to apply anyways,
							//because they chose to correct, and it didn't work,
							//so abort
							return ecalirCannotApply;
						}
						else {
							//they can unapply this item, but the ID changed
							bSourceIDsChanged = true;

							ApplyIDInfo eInfo = FindNewestCorrectedApply(nApplyID);

							//since we just performed the correction, this should be impossible
							if (eInfo.nID == -1) {
								//this means that FindNewestCorrectedApply failed to find the apply
								//that we just corrected inside this function
								ASSERT(FALSE);
								ThrowNxException("CanUnapplyLineItem failed to find a new ApplyID for ID %li after it was voided & corrected.", nApplyID);
							}

							nApplyID = eInfo.nID;
							nSourcePayID = eInfo.nSourceID;

							//don't return, we'll check unapply permissions at the end of this function
						}
					}
					else {
						//they can unapply this item, and it was not corrected
						bSourceIDsChanged = false;

						//don't return, we'll check unapply permissions at the end of this function
					}
				}
			}
		}
		rsHardClose->Close();

		//now we can return if the user is an administrator
		if (IsCurrentUserAdministrator()) {
			if (bSourceIDsChanged) {
				return ecalirCanApply_IDHasChanged;
			}
			else {
				return ecalirCanApply;
			}
		}

		//check the normal apply permission
		if (!CanChangeHistoricFinancial("Apply", nApplyID, bioApplies, sptDelete, bSilent)) {
			// No permission, or no valid password
			return ecalirCannotApply;
		}
		else {
			// They do have permission
			if (bSourceIDsChanged) {
				return ecalirCanApply_IDHasChanged;
			}
			else {
				return ecalirCanApply;
			}
		}

	}NxCatchAll(__FUNCTION__);

	return ecalirCannotApply;
}

// (j.jones 2011-09-21 11:46) - PLID 45462 - checks permissions and closed status, 
// and warns if the item is closed
BOOL CanUndoCorrection(CString strType, COleDateTime dtCorrectionInputDate)
{
	if(dtCorrectionInputDate.GetStatus() == COleDateTime::invalid) {
		ThrowNxException("CanUndoCorrection called with invalid dtCorrectionInputDate!");
	}

	//check our last date/time of a hard close
	_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseDate FROM FinancialCloseHistoryT");
	if(!rsHardClose->eof) {
		//this is nullable since this is a Max()
		COleDateTime dtHardClose = VarDateTime(rsHardClose->Fields->Item["HardCloseDate"]->Value, g_cdtInvalid);
		
		if(dtHardClose.GetStatus() != COleDateTime::invalid && dtCorrectionInputDate <= dtHardClose) {			
			//they have closed the data in the past, and this input date is on or before that close date
			
			//can they bypass a hard close?
			ECanOverrideHardClose eCanOverride = cohcNo;
			if(!(GetCurrentUserPermissions(bioFinancialCorrections) & (sptDynamic0))) {
				// If we get here, the user doesn't have override permission. Check for
				// override with password.
				if((GetCurrentUserPermissions(bioFinancialCorrections) & (sptDynamic0WithPass))) { // With password
					//they will need a password
					eCanOverride = cohcWithPass;
				}
				else {
					// No permission to override the preference.
					eCanOverride = cohcNo;
				}
			}
			else {
				eCanOverride = cohcYes; // User has permission to override the preference.
			}

			//if they can't bypass it, warn and quit
			if(eCanOverride == cohcNo) {
				MsgBox(MB_OK | MB_ICONERROR, "This %s was corrected on %s, and cannot be reversed because the correction occurred before the most recent closing date of %s.",
					strType, FormatDateTimeForInterface(dtCorrectionInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
				return FALSE;
			}

			// (j.jones 2011-01-06 09:35) - PLID 42000 - if they can bypass it, warn first
			CString strWarn;	
			strWarn.Format("This %s was corrected on %s, which occurred before the most recent closing date of %s.\n\n"
				"Are you absolutely certain you wish to undo this correction?",
				strType, FormatDateTimeForInterface(dtCorrectionInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
			if(IDNO == MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}

			//check the password if needed
			if(eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
				//fail, but clarify why we failed
				MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to undo this correction.");
				return FALSE;
			}
		}
	}
	rsHardClose->Close();

	//if we are still here, then we need to check the normal Undo permission,
	//because just like any other action past close, you still need the normal
	//permission in addition to the past close permission

	//return if the user is an administrator
	if (IsCurrentUserAdministrator()) {
		return TRUE;
	}

	//check the normal apply permission
	if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptDelete)) {
		// No permission, or no valid password
		return FALSE;
	} else {
		// They do have permission
		return TRUE;
	}
}

// (j.jones 2015-06-16 10:42) - PLID 50008 - Given the information on a corrected payment/adjustment/refund,
// this checks to see if the payment is applied in such a way that undoing the apply would cause a negative balance.
// Returns false if the user agreed to cancel the undo process.
// Returns true if the undo can continue.
// The array tracks any applies that need to be deleted when the correction undo process finishes.
bool UndoCorrection_CheckInvalidApplies(const CString strLineItemTypeName,
	const long nCorrectionOriginalID, const long nCorrectionVoidingID, const long nCorrectionNewID,
	bool bVerboseWarnings, std::vector<long> &aryAppliesToUndo)
{
	//do NOT clear aryAppliesToUndo, it may have data in it already

	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(),
		//charges
		"SELECT AppliesT.ID, LineItemT_Pay.Amount, AppliesT.Amount AS ApplyAmount, dbo.GetChargeTotal(LineItemT_Charge.ID) AS ChargeAmount, PersonT.FullName "
		"FROM ChargeRespT "		
		"INNER JOIN (SELECT RespID, Sum(Amount) AS TotalApplies "
		"	FROM AppliesT "
		"	WHERE SourceID <> {INT} AND SourceID <> {INT} "
		"	GROUP BY RespID "
		") AS AppliesQ ON ChargeRespT.ID = AppliesQ.RespID "
		"INNER JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID AND AppliesT.SourceID = {INT} "
		"INNER JOIN LineItemT LineItemT_Pay ON AppliesT.SourceID = LineItemT_Pay.ID "
		"INNER JOIN LineItemT LineItemT_Charge ON AppliesT.DestID = LineItemT_Charge.ID "
		"INNER JOIN PersonT ON LineItemT_Pay.PatientID = PersonT.ID "
		"WHERE AppliesQ.TotalApplies > ChargeRespT.Amount "
		"GROUP BY AppliesT.ID, LineItemT_Pay.Amount, AppliesT.Amount, dbo.GetChargeTotal(LineItemT_Charge.ID), PersonT.FullName; "
		""
		//payments
		"SELECT AppliesT.ID, LineItemT.Type, LineItemT_Source.Amount, AppliesT.Amount AS ApplyAmount, LineItemT_Dest.Amount AS DestAmount, PersonT.FullName "
		"FROM LineItemT "
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "		
		"INNER JOIN (SELECT DestID, Sum(-Amount) AS TotalApplies "
		"	FROM AppliesT "
		"	WHERE SourceID <> {INT} AND SourceID <> {INT} "
		"	GROUP BY DestID "
		") AS AppliesQ ON LineItemT.ID = AppliesQ.DestID "
		"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID AND AppliesT.SourceID = {INT} "
		"INNER JOIN LineItemT LineItemT_Source ON AppliesT.SourceID = LineItemT_Source.ID "
		"INNER JOIN LineItemT LineItemT_Dest ON AppliesT.DestID = LineItemT_Dest.ID "
		"INNER JOIN PersonT ON LineItemT_Source.PatientID = PersonT.ID "
		"WHERE AppliesQ.TotalApplies > LineItemT.Amount "
		"GROUP BY AppliesT.ID, LineItemT.Type, LineItemT_Source.Amount, AppliesT.Amount, LineItemT_Dest.Amount, PersonT.FullName; ",
		nCorrectionVoidingID, nCorrectionNewID, nCorrectionOriginalID,
		nCorrectionVoidingID, nCorrectionNewID, nCorrectionOriginalID);
	//charges
	if (!rs->eof) {
		CString strWarning;
		if (bVerboseWarnings) {
			CString strPatientName = VarString(rs->Fields->Item["FullName"]->Value);
			CString strLineItemAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["Amount"]->Value));
			CString strAppliedAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["ApplyAmount"]->Value));
			CString strChargeAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["ChargeAmount"]->Value));
			//this is being called from an e-remit, so include more details about what the issue is
			strWarning.Format("For patient %s, the original %s %s from this correction had %s applied to a %s charge that no longer has enough responsibility available. "
				"To undo this correction, the original %s will need to be unapplied.\n\n"
				"Are you sure you wish to continue with this correction?",
				strPatientName, strLineItemAmount, strLineItemTypeName, strAppliedAmount, strChargeAmount, strLineItemTypeName);
		}
		else {
			strWarning.Format("The original %s from this correction was applied to a charge that no longer has enough responsibility available. "
				"To undo this correction, the original %s will need to be unapplied.\n\n"
				"Are you sure you wish to continue with this correction?",
				strLineItemTypeName, strLineItemTypeName);
		}
		if (IDNO == MessageBox(GetActiveWindow(), strWarning, "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
			return false;
		}
	}
	while (!rs->eof) {
		//track these applies
		aryAppliesToUndo.push_back(VarLong(rs->Fields->Item["ID"]->Value));
		rs->MoveNext();
	}

	rs = rs->NextRecordset(NULL);

	//payments
	if (!rs->eof) {
		CString strWarning;
		long nAppliedType = VarLong(rs->Fields->Item["Type"]->Value);
		CString strAppliedType = "charge";
		if (nAppliedType == LineItem::Payment) {
			strAppliedType = "payment";
		}
		else if (nAppliedType == LineItem::Adjustment) {
			strAppliedType = "adjustment";
		}
		else if (nAppliedType == LineItem::Refund) {
			strAppliedType = "refund";
		}
		if (bVerboseWarnings) {
			//this is being called from an e-remit, so include more details about what the issue is
			CString strPatientName = VarString(rs->Fields->Item["FullName"]->Value);
			CString strSourceAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["Amount"]->Value));
			CString strAppliedAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["ApplyAmount"]->Value));
			CString strDestAmount = FormatCurrencyForInterface(VarCurrency(rs->Fields->Item["DestAmount"]->Value));
			strWarning.Format("For patient %s, the original %s %s from this correction had %s applied to a %s %s that no longer has enough responsibility available. "
				"To undo this correction, the original %s will need to be unapplied.\n\n"
				"Are you sure you wish to continue with this correction?",
				strPatientName, strSourceAmount, strLineItemTypeName, strAppliedAmount, strDestAmount, strAppliedType, strLineItemTypeName);
		}
		else {
			strWarning.Format("The original %s from this correction was applied to a %s that no longer has enough responsibility available. "
				"To undo this correction, the original %s will need to be unapplied.\n\n"
				"Are you sure you wish to continue with this correction?",
				strLineItemTypeName, strAppliedType, strLineItemTypeName);
		}
		if (IDNO == MessageBox(GetActiveWindow(), strWarning, "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
			return false;
		}
	}
	while (!rs->eof) {
		//track these applies
		aryAppliesToUndo.push_back(VarLong(rs->Fields->Item["ID"]->Value));
		rs->MoveNext();
	}
	rs->Close();

	return true;
}

// (j.jones 2011-11-01 14:31) - PLID 45462 - IsLineItemAppliedToNewCorrection
// returns TRUE if the pay/adj/ref in question is applied to a line item that
// was later corrected, therefore it needs to be undone first
BOOL IsPaymentAppliedToNewCorrection(long nPaymentID, CString strOriginalCorrectionsTable, long nOriginalCorrectionID, OUT CString &strAppliedToType)
{
	if(nPaymentID < 0) {
		//nobody should have called this on a negative ID
		return FALSE;
	}

	//is our given payment applied to an original line item that was corrected later than our correction in question?
	_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT CASE WHEN DestLineItemT.Type = 1 THEN 'payment' WHEN DestLineItemT.Type = 2 THEN 'adjustment' "
		"WHEN DestLineItemT.Type = 3 THEN 'refund' ELSE 'charge' END AS AppliedToType "
		"FROM AppliesT "
		"INNER JOIN LineItemT AS SourceLineItemT ON AppliesT.SourceID = SourceLineItemT.ID "
		"INNER JOIN LineItemT AS DestLineItemT ON AppliesT.DestID = DestLineItemT.ID "
		"INNER JOIN (SELECT OriginalLineItemID, InputDate FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemQ ON DestLineItemT.ID = LineItemCorrections_OriginalLineItemQ.OriginalLineItemID "
		"WHERE SourceLineItemT.Deleted = 0 AND DestLineItemT.Deleted = 0 "
		"AND AppliesT.SourceID = {INT} "
		"AND LineItemCorrections_OriginalLineItemQ.InputDate > (SELECT InputDate FROM %s WHERE ID = {INT})", strOriginalCorrectionsTable),
		nPaymentID, nOriginalCorrectionID);

	if(rs->eof) {
		strAppliedToType = "";
		return FALSE;
	}
	else {
		strAppliedToType = VarString(rs->Fields->Item["AppliedToType"]->Value);
		return TRUE;
	}
}

// (j.jones 2012-04-24 11:50) - PLID 49847 - Given a line item ID, see if it is corrected,
// and if so, return the corrected line item ID. If it's not corrected, return the original ID,
// implying it is still the "newest" version to use. If it has been voided, but not corrected,
// return -1.
// This uses a recursive ad-hoc function, so if you correct the correction, it finds the newest available version.
// (j.jones 2014-01-15 15:51) - PLID 58726 - this was failing due to SQL comments, I turned them into C++ comments
// and added semicolons and newlines for clarity
long FindNewestCorrectedLineItemID(long nOriginalLineItemID)
{
	//this really shouldn't be called with a Void line item, but if it is,
	//return the corrected line item for that void

	//this ad-hoc function will recursively calculate the newest line item ID until
	//there is no newer line item ID
	_RecordsetPtr rs = CreateParamRecordset(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @lastLineItemID INT, @nextLineItemID INT; \r\n"
		"DECLARE @recurseAgain BIT; \r\n"
		""
		"SET @lastLineItemID = {INT}; \r\n"
		"SET @recurseAgain = 1; \r\n"
		""
		//loop while we keep finding newer line item IDs
		"WHILE (@recurseAgain = 1) \r\n"
		"BEGIN \r\n"
		"	SET @nextLineItemID = NULL; \r\n"
		"	SELECT @nextLineItemID = Coalesce(NewLineItemID, -1) FROM LineItemCorrectionsT WHERE OriginalLineItemID = @lastLineItemID OR VoidingLineItemID = @lastLineItemID; \r\n"
		"	IF(@nextLineItemID Is Null) \r\n"
		"	BEGIN \r\n"
		//		no new line item found, stop looping
		"		SET @recurseAgain = 0; \r\n"
		"	END \r\n"
		"	ELSE BEGIN \r\n"
		//		store the new line item as the last line item, and loop again 
		//		this can be -1 
		"		SET @lastLineItemID = @nextLineItemID; \r\n"
		"		SET @recurseAgain = 1; \r\n"
		"	END; \r\n"
		"END; \r\n"
		""
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT @lastLineItemID AS NewLineItemID; "
		, nOriginalLineItemID);
	if(rs->eof) {
		//nothing new to report
		return nOriginalLineItemID;
	}
	else {
		//this will be -1 if the original was voided but no new correction was made,
		//it will be the original line item ID if it was never corrected
		long nNewLineItemID = VarLong(rs->Fields->Item["NewLineItemID"]->Value);
		return nNewLineItemID;
	}
}

// (j.jones 2015-02-19 15:26) - PLID 64938 - Given an AppliesT ID, see if it is corrected,
// and if so, return the corrected AppliesT ID. If it's not corrected, return the original ID,
// implying it is still the "newest" version to use.

// If it was voided, but not corrected, this returns -1 for the apply ID.
// It will also return -1 if the source & destination were corrected but an apply of the same amount no longer exists.
// This uses a recursive ad-hoc function, so if you correct the correction, it finds the newest available version.
ApplyIDInfo FindNewestCorrectedApply(long nOriginalApplyID)
{
	//find the source and destination IDs
	_RecordsetPtr rsOriginalApply = CreateParamRecordset("SELECT SourceID, DestID, Amount FROM AppliesT WHERE ID = {INT}", nOriginalApplyID);
	if (rsOriginalApply->eof) {
		//this isn't a valid apply ID
		ASSERT(FALSE);
		return ApplyIDInfo();
	}
	else {
		long nOriginalSourceID = AdoFldLong(rsOriginalApply->Fields, "SourceID");
		long nOriginalDestID = AdoFldLong(rsOriginalApply->Fields, "DestID");
		COleCurrency cyOriginalAmount = AdoFldCurrency(rsOriginalApply->Fields, "Amount");

		rsOriginalApply->Close();

		long nCorrectedSourceID = FindNewestCorrectedLineItemID(nOriginalSourceID);
		long nCorrectedDestID = FindNewestCorrectedLineItemID(nOriginalDestID);

		//if the source and destination were not corrected, return the original apply ID
		if (nOriginalSourceID == nCorrectedSourceID && nOriginalDestID == nCorrectedDestID) {
			ApplyIDInfo eInfo;
			eInfo.nID = nOriginalApplyID;
			eInfo.nSourceID = nOriginalSourceID;
			eInfo.nDestID = nOriginalDestID;
			return eInfo;
		}

		if (nCorrectedSourceID == -1 || nCorrectedDestID == -1) {
			//One of the source/dest records doesn't exist anymore, so neither will the apply.
			return ApplyIDInfo();
		}

		//now is the same apply still present on these corrected items?
		_RecordsetPtr rsNewApply = CreateParamRecordset("SELECT ID FROM AppliesT "
			"WHERE SourceID = {INT} AND DestID = {INT} AND Amount = {OLECURRENCY}", nCorrectedSourceID, nCorrectedDestID, cyOriginalAmount);
		if (!rsNewApply->eof) {
			//success! this apply still exists on the new line items
			ApplyIDInfo eInfo;
			eInfo.nID = AdoFldLong(rsNewApply->Fields, "ID");
			eInfo.nSourceID = nCorrectedSourceID;
			eInfo.nDestID = nCorrectedDestID;
			return eInfo;
		}
		else {
			//This apply doesn't exist anymore. It is possible it might exist, but with a different amount.
			return ApplyIDInfo();
		}
	}

	//should be logically impossible to get here
	ASSERT(FALSE);
	return ApplyIDInfo();
}

// (j.jones 2012-07-26 09:57) - PLID 50489 - Simplifies code that compares four charge modifiers
// to one modifier. If the provided value is in use in any of the 4 provided modifier fields, return TRUE.
// Return FALSE if none of the provided fields contains the desired modifier.
BOOL ContainsModifier(CString strModifierToSearch, CString strChargeModifier1, CString strChargeModifier2, CString strChargeModifier3, CString strChargeModifier4)
{
	strModifierToSearch.TrimLeft();
	strModifierToSearch.TrimRight();
	strChargeModifier1.TrimLeft();
	strChargeModifier1.TrimRight();
	strChargeModifier2.TrimLeft();
	strChargeModifier2.TrimRight();
	strChargeModifier3.TrimLeft();
	strChargeModifier3.TrimRight();
	strChargeModifier4.TrimLeft();
	strChargeModifier4.TrimRight();

	if(strModifierToSearch.IsEmpty())
	{
		//This function is not meant to be called with no modifier,
		//we don't know what that's supposed to mean. If you hit
		//this ASSERT, the developer responsible for the calling code
		//needs to check and see if it was intended to do this.
		ASSERT(FALSE);
		return FALSE;
	}

	if(strChargeModifier1.CompareNoCase(strModifierToSearch) == 0) {
		return TRUE;
	}
	if(strChargeModifier2.CompareNoCase(strModifierToSearch) == 0) {
		return TRUE;
	}
	if(strChargeModifier3.CompareNoCase(strModifierToSearch) == 0) {
		return TRUE;
	}
	if(strChargeModifier4.CompareNoCase(strModifierToSearch) == 0) {
		return TRUE;
	}

	//not found
	return FALSE;
}

// (j.jones 2010-08-04 12:29) - PLID 38613 - added a generic function to calculate percentages of currencies
// (j.jones 2012-07-30 14:38) - PLID 47778 - moved from billing to global financial utils
COleCurrency CalculatePercentOfAmount(COleCurrency cyAmount, long nPercent)
{
	// (j.jones 2010-08-04 12:19) - PLID 38613 - we must call CalculateAmtQuantity for
	// multiplying currency by a double, but we can avoid it if our nPercent is 100
	if(nPercent != 100) {
		double dbl = ((double)(nPercent))/((double)100);
		cyAmount = CalculateAmtQuantity(cyAmount, dbl);
	}

	RoundCurrency(cyAmount);

	return cyAmount;
}

// (j.jones 2012-08-08 11:22) - PLID 47778 - Returns TRUE not just if the insured party is the patient's first insurance,
// instead it returns TRUE if the insured party if its RespTypeT.Priority = 1, RespTypeT.CategoryPlacement = 1 or if the
// insured party has the "Send As Primary" option checked off.
BOOL IsInsuredPartySentAsPrimary(long nInsuredPartyID)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE InsuredPartyT.PersonID = {INT} "
		"AND (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) ", nInsuredPartyID);
	BOOL bIsPrimaryIns = FALSE;
	if(!rs->eof) {
		bIsPrimaryIns = TRUE;
	}
	rs->Close();

	return bIsPrimaryIns;
}

// (j.jones 2011-01-06 16:57) - PLID 41785 - added generic function to create a billing note
// (j.jones 2012-08-14 15:21) - PLID 50285 - moved from EOBDlg to globalfinancialutils, no changes to the content of the function
// (j.dinatale 2012-12-19 11:26) - PLID 54256 - function needs to be able to add notes to bills
void CreateBillingNote(long nPatientID, long nID, CString strNote, long nCategoryID, BOOL bShowOnStatement, BOOL bIsBillNote /*= FALSE*/)
{
	//throw exceptions to the caller

	// (j.jones 2013-09-04 11:46) - PLID 58330 - don't save the note with no ID
	if(nID == -1) {
		//the caller should not be calling this function on a -1 ID,
		//review the calling code to ensure it is safely handling -1,
		//and make sure it's not trying to do anything else with this bad data
		ASSERT(FALSE);
		return;
	}

	CSqlFragment sqlBatch;

	CString strNoteIDField;
	if(bIsBillNote){
		strNoteIDField = "BillID";
	}else{
		strNoteIDField = "LineItemID";
	}

	_variant_t varCategoryID = g_cvarNull;
	if(nCategoryID != -1) {
		varCategoryID = (long)nCategoryID;
	}

	sqlBatch +=  
		"DECLARE @newNoteID INT \r\n"
		"DECLARE @nCategoryID INT \r\n";

	//ensure the category is valid
	sqlBatch += CSqlFragment(
		"SET @nCategoryID = {VT_I4} \r\n"
		"IF @nCategoryID Is Not Null AND @nCategoryID NOT IN (SELECT ID FROM NoteCatsF) \r\n"
		"BEGIN \r\n"
		"	SET @nCategoryID = NULL \r\n"
		"END \r\n", varCategoryID);


	// (j.jones 2011-04-04 14:42) - PLID 43129 - if ShowOnStatement is true, we have to remove the flag
	// from all other notes on this charge, as a charge can only have one note flagged to show on the
	// statement
	if(bShowOnStatement) {
		sqlBatch += CSqlFragment("UPDATE NoteInfoT SET ShowOnStatement = 0 WHERE {CONST_STR} = {INT} \r\n", strNoteIDField, nID);
	}
	
	// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
	sqlBatch += CSqlFragment(
		"INSERT INTO Notes (PersonID, Date, UserID, Note, Category) "
		"VALUES ({INT}, GetDate(), {INT}, {STRING}, @nCategoryID) \r\n"
		"SET @newNoteID = SCOPE_IDENTITY()\r\n",
		nPatientID, GetCurrentUserID(), strNote
	);

	sqlBatch += CSqlFragment(
		"INSERT INTO NoteInfoT (NoteID, {CONST_STR}, ShowOnStatement) "
		"VALUES (@newNoteID, {INT}, {INT}) \r\n",
		strNoteIDField, nID, bShowOnStatement ? 1 : 0
	);

	ExecuteParamSql(sqlBatch);
}

// (j.dinatale 2012-12-28 11:26) - PLID 54365 - need to be able to get the provider's submitter prefix
// (j.dinatale 2012-12-31 13:04) - PLID 54382 - moved from ebilling.cpp
CString GetAlbertaProviderSubmitterPrefix(long nProviderID)
{
	// grab our preferences
	long nSubmitterPrefixCustomField = GetRemotePropertyInt("Alberta_SubmitterPrefixCustomField", -1, 0, "<None>", true);

	// check and see if we have a valid custom field
	if(nSubmitterPrefixCustomField > 0){
		// query the custom field data
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT TextParam FROM CustomFieldDataT WHERE FieldID = {INT} AND PersonID = {INT}", nSubmitterPrefixCustomField, nProviderID);

		// and if we got anything back
		if(!rs->eof){
			CString strCustSubmitterPrefix = AdoFldString(rs, "TextParam", "");

			// trim it and validate that its not empty
			strCustSubmitterPrefix.Trim();
			if(!strCustSubmitterPrefix.IsEmpty()){
				return strCustSubmitterPrefix;
			}
		}
	}

	// if we end up down here, we need to return our default
	CString strDefSubmitterPrefix = GetRemotePropertyText("Alberta_SubmitterPrefix", "", 0, "<None>", true);
	strDefSubmitterPrefix.Trim();
	return strDefSubmitterPrefix;
}

// (j.jones 2013-01-23 09:03) - PLID 54734 - Added a function to insert one bill into claim history.
// If bTrackLastSendDate is enabled, we will update HCFATrackT.LastSendDate.
// If bBatchedChargesOnly is enabled, only batched charges will add to claim history details, otherwise all charges are added.
void AddToClaimHistory(long nBillID, long nInsuredPartyID, ClaimSendType::ESendType eClaimSendType, CString strClearingHouseName /*= ""*/,
					   BOOL bTrackLastSendDate /*= TRUE*/, BOOL bBatchedChargesOnly /*= TRUE*/)
{
	//(r.wilson 10/8/2012) plid 52970 - Replace SendType value to an emun. Old value was a hard coded 6
	//Send Type 5 - Paper MICR Form
	// (j.jones 2013-01-23 08:55) - PLID 54734 - the ClaimHistoryT.ID column is now an identity
	ExecuteParamSql("DECLARE @nBillID INT \r\n"
		"SET @nBillID = {INT} \r\n"
		""
		"INSERT INTO ClaimHistoryT (BillID, InsuredPartyID, SendType, Clearinghouse, Date, UserName) "
		"VALUES (@nBillID, {INT}, {INT}, {STRING}, GetDate(), {STRING}) \r\n"
		""
		"DECLARE @nNewClaimHistoryID INT \r\n"
		"SET @nNewClaimHistoryID = SCOPE_IDENTITY() \r\n"
		""
		"INSERT INTO ClaimHistoryDetailsT (ClaimHistoryID, ChargeID) "
		"SELECT @nNewClaimHistoryID, ChargesT.ID FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE Deleted = 0 AND ({INT} = 0 OR Batched = 1) "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND BillID = @nBillID \r\n"
		""
		"IF ({INT} = 1) \r\n"
		"BEGIN \r\n"
		"	UPDATE HCFATrackT SET LastSendDate = GetDate() WHERE BillID = @nBillID \r\n"
		"END",
		nBillID,
		nInsuredPartyID, (long)eClaimSendType, strClearingHouseName, GetCurrentUserName(),
		bBatchedChargesOnly ? 1 : 0,
		bTrackLastSendDate ? 1 : 0);
}

// (j.jones 2013-01-25 12:16) - PLID 54853 - Called in ANSI E-Remittance to check whether
// a given code qualifier is one that we could have legitimately submitted. Also called in
// ANSI E-Billing and E-Eligibility exports to enforce that we would never send a qualifier
// not covered by this function.
BOOL IsValidServiceCodeQualifier(CString strQualifier)
{
	//Right now the only code qualifier we export is HC.
	//If we ever support more code qualifiers, they need to be added to this list
	//in order for claim exports to function properly.
	if(strQualifier.CompareNoCase("HC") == 0) {
		return TRUE;
	}
	//We don't export ZZ, but it has been used in E-Remits on valid codes,
	//so we consider this to be a valid return value.
	else if(strQualifier.CompareNoCase("ZZ") == 0) {
		return TRUE;
	}

	return FALSE;
}

// (j.jones 2013-07-01 13:31) - PLID 55517 - Made voiding & correcting a pay/adj/ref be a modular function
// that can optionally suppress the pre-correction warnings.
// If successful, return true. If bNeedCorrectedPaymentID is true, nPaymentID will change to reflect the new corrected ID.
// If it fails, return false. The payment ID will be unchanged.
bool VoidAndCorrectPayAdjRef(IN OUT long &nPaymentID, CString strLineType, bool bNeedCorrectedPaymentID /*= false*/, bool bSilentWarnings /*= false*/)
{
	// (j.jones 2013-07-01 13:36) - PLID 55517 - this code was moved from OnContextMenu

	// (j.dinatale 2011-06-27 10:20) - PLID 44812
	if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
		return false;
	}

	// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
	// an already corrected payment
	LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPaymentID);
	if(licsStatus == licsOriginal) {
		//ignore the silent flag on failure
		CString strWarn;
		strWarn.Format("This %s has already been corrected, and can no longer be modified.", strLineType);
		AfxMessageBox(strWarn);
		return false;
	}
	else if(licsStatus == licsVoid) {
		//ignore the silent flag on failure
		CString strWarn;
		strWarn.Format("This %s is a void %s from an existing correction, and cannot be modified.", strLineType, strLineType);
		AfxMessageBox(strWarn);
		return false;
	}

	// (j.jones 2013-07-01 13:41) - PLID 55517 - the following messages and prompts can all be skipped
	// if bSilentWarnings is true
	if(!bSilentWarnings) {
		// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
		//		etc, etc, etc.
		CString strMessage1, strMessage2;

		// (j.dinatale 2011-12-09 09:29) - PLID 46898 - simplified this message
		strMessage1.Format("This feature should be used when the %s has been incorrectly created or incorrectly applied and it cannot be unapplied.\r\n"
			"\r\n"
			"NOTE: A %s's provider and location are reported based on the line item it is applied to. DO NOT correct an applied %s if these are the fields "
			"that should be changed. Consider correcting the line item it is applied to instead.", 
			strLineType, strLineType, strLineType);
		if(IDCANCEL == AfxMessageBox(strMessage1, MB_OKCANCEL)) {
			return false;
		}

		// (j.dinatale 2011-09-13 17:30) - PLID 44808 - we are no longer creating adjustments, need to reflect as such in the message boxes
		strMessage2.Format(
				"The following actions will occur:\r\n"
				"\r\n"
				"- A reverse %s will be created and applied to the original %s to offset its amount.\r\n"
				"- A new corrected %s will be created to replace the original %s."
				, strLineType, strLineType, strLineType, strLineType);

		if(IDCANCEL == AfxMessageBox(strMessage2, MB_OKCANCEL)) {
			return false;
		}

		// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
		if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
			"Are you sure you want to perform this correction?", MB_YESNO)) {
			return false;
		}
	}

	CFinancialCorrection finCor;

	CString strUsername = GetCurrentUserName();
	long nCurrUserID = GetCurrentUserID();

	finCor.AddCorrection(ctPayment, nPaymentID, strUsername, nCurrUserID);

	finCor.ExecuteCorrections();

	//if requested, find the new payment ID and return it via the passed in parameter
	if(bNeedCorrectedPaymentID) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT NewLineItemID FROM LineItemCorrectionsT WHERE OriginalLineItemID = {INT}", nPaymentID);
		if(!rs->eof) {
			//replace the existing payment ID with the new one
			nPaymentID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);
		}
		else {
			//eof should be impossible, and our caller is expecting a new ID
			ThrowNxException("VoidAndCorrectPayAdjRef failed to generate a new payment ID.");
		}
	}

	return true;
}

// (j.jones 2013-07-18 11:23) - PLID 57616 - This function is meant to be called AFTER a claim is paid,
// and potentially already shifted & batched to secondary. It will check to see if the paying company is a primary
// company that is configured to NOT batch to secondary. It uses the bill's current 'main' insured party ID
// is the secondary insurer.
// If configured to not batch to this payer, it will unbatch the claim, add a billing note, and update claim history.
// nPaidInsuredPartyID is who paid, who was the 'primary' insurance on the claim, who crossed over to a secondary company.
BOOL CheckUnbatchCrossoverClaim(long nPatientID, long nBillID, long nPaidInsuredPartyID, COleDateTime dtPaymentDate,
								bool bBatchedChargesOnlyInClaimHistory, AuditEventItems aeiAuditItem,
								CString strAuditOldValue, CString strAuditNewValue)
{
	if(nBillID == -1 || nPaidInsuredPartyID == -1) {
		//why was this function called?
		ASSERT(FALSE);
		return FALSE;
	}

	//check the following:
	//- is the bill's current InsuredPartyID different from the insured party who paid?
	//		- if it is different, then we assume that's the newly shifted 'secondary' payer,
	//		also referred to as the crossed over insured party ID
	//- is the paying insurer for the patient's primary medical insurance?
	//		- this feature is only supported on medical claims
	//- if this bill is a HCFA, is the HCFA group's DontBatchSecondary flag on?
	//- if this bill is a UB, is the UB group's DontBatchSecondary flag on?
	//- is the crossed over insured party ID for a company NOT in the secondary exclusions list?
	//		- the crossed over insured party ID is the bill's current insured party ID
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.InsuredPartyID "
		"FROM BillsT "
		"INNER JOIN InsuredPartyT PrimaryInsuredPartyT ON BillsT.PatientID = PrimaryInsuredPartyT.PatientID "
		"INNER JOIN RespTypeT ON PrimaryInsuredPartyT.RespTypeID = RespTypeT.ID "
		"INNER JOIN InsuranceCoT ON PrimaryInsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"INNER JOIN InsuredPartyT BillInsuredPartyT ON BillsT.InsuredPartyID = BillInsuredPartyT.PersonID "
		"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
		"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
		"LEFT JOIN HCFASecondaryExclusionsT ON HCFASetupT.ID = HCFASecondaryExclusionsT.HCFASetupID AND BillInsuredPartyT.InsuranceCoID = HCFASecondaryExclusionsT.InsuranceCoID "
		"LEFT JOIN UBSecondaryExclusionsT ON UB92SetupT.ID = UBSecondaryExclusionsT.UBSetupID AND BillInsuredPartyT.InsuranceCoID = UBSecondaryExclusionsT.InsuranceCoID "		
		"WHERE BillsT.ID = {INT} AND PrimaryInsuredPartyT.PersonID = {INT} "
		"AND RespTypeT.Priority = 1 "
		"AND BillInsuredPartyT.PersonID <> PrimaryInsuredPartyT.PersonID "
		"AND ("
		//if a HCFA, check if its group's DontBatchSecondary is on, and the bill's current insured party is not excluded from this feature
		"	(BillsT.FormType = 1 AND HCFASetupT.DontBatchSecondary = 1 AND HCFASecondaryExclusionsT.InsuranceCoID Is Null) "
		"	OR "
		//if a UB, check if its group's DontBatchSecondary is on, and the bill's current insured party is not excluded from this feature
		"	(BillsT.FormType = 2 AND UB92SetupT.DontBatchSecondary = 1 AND UBSecondaryExclusionsT.InsuranceCoID Is Null) "
		") ",
		nBillID, nPaidInsuredPartyID);
	if(!rs->eof) {
		//if this returns records, then all our requirements match,
		//and we need to unbatch this claim
		long nCrossedOverInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value);

		UnbatchCrossoverClaim(nPatientID, nBillID, nCrossedOverInsuredPartyID, dtPaymentDate,
			bBatchedChargesOnlyInClaimHistory, aeiAuditItem, strAuditOldValue, strAuditNewValue);

		return TRUE;
	}

	return FALSE;
}

// (j.jones 2013-07-18 11:22) - PLID 57616 - Unified function to handle when primary crossed over to secondary.
// This adds a note to History and Billing Notes, unbatches the claim, uniquely audits, and updates claim history.
// nCrossedOverInsuredPartyID is the insured party that was secondary, that was the insurance that the primary crossed over to,
// who we are unbatching on behalf of.
// From EOBs with MA18, it is who was the bill's OthrInsuredPartyID before posting.
// For manual postings, or EOBs without MA18, it's whoever is the bill's current InsuredPartyID.
void UnbatchCrossoverClaim(long nPatientID, long nBillID, long nCrossedOverInsuredPartyID, COleDateTime dtPaymentDate,
						   bool bBatchedChargesOnlyInClaimHistory, AuditEventItems aeiAuditItem,
						   CString strAuditOldValue, CString strAuditNewValue)
{
	CString strDesc = "Automatic Crossover to Supplemental Insurance(s)";
	if(!dtPaymentDate.GetStatus()){ // if GetStatus() == COleDateTime::valid
		strDesc += (" - Payment Date " + FormatDateTimeForInterface(dtPaymentDate, NULL, dtoDate));
	}

	CreateNewMailSentEntry(nPatientID, strDesc, "", "", GetCurrentUserName(), "", GetCurrentLocation());
	CreateBillingNote(nPatientID, nBillID, "Automatic Crossover to Supplemental Insurance(s)", -1, FALSE, TRUE);

	COleDateTime dtClaimHistory = COleDateTime::GetCurrentTime();
	if(dtPaymentDate.GetStatus() == COleDateTime::valid && dtPaymentDate > g_cdtSqlMin) {
		dtClaimHistory = dtPaymentDate;
	}

	// (j.jones 2013-01-23 08:55) - PLID 54734 - the ID column is now an identity
	//(s.dhole 2013-04-25 16:03) - PLID 56122 Added ClaimHistoryDetailsT
	if (nCrossedOverInsuredPartyID != -1) {
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON; "
			"DECLARE @BillID INT; "
			"SET @BillID = {INT}; "
			""
			"DECLARE @nNewClaimHistoryID INT "
			""
			"INSERT INTO ClaimHistoryT (BillID, InsuredPartyID, SendType, Date, UserName) "
			"VALUES (@BillID, {INT}, {INT}, {OLEDATETIME}, {STRING}) "
			""
			"SET @nNewClaimHistoryID = SCOPE_IDENTITY() "
			""
			"INSERT INTO ClaimHistoryDetailsT (ClaimHistoryID, ChargeID) "
			"SELECT @nNewClaimHistoryID, ChargesT.ID "
			"	FROM ChargesT "
			"   INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"   WHERE LineItemT.Deleted = 0 AND ({INT} = 0 OR Batched = 1) "
			"   AND BillID = @BillID; "
			""
			"SET NOCOUNT OFF; "
			"SELECT InsuredPartyID FROM BillsT WHERE ID = @BillID",
			nBillID,
			nCrossedOverInsuredPartyID, ClaimSendType::Electronic, dtClaimHistory, GetCurrentUserName(),
			bBatchedChargesOnlyInClaimHistory ? 1 : 0);
		if (!rs->eof) {
			//if the current insured party ID is *not* what we think the crossover insured party should be,
			//do not continue, do not unbatch the claim, because it's not currently batched for that
			//insured party after all
			long nCurInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);
			if (nCurInsuredPartyID != nCrossedOverInsuredPartyID) {
				return;
			}
		}
		rs->Close();
	}

	// (j.jones 2014-06-24 11:43) - PLID 60349 - If we get here, always unbatch the claim.
	// This ensures that if nCrossedOverInsuredPartyID was -1, we will still unbatch even
	// if we didn't make a claim history entry.
	if(BatchBill(nBillID, 0) == BatchChange::Unbatched) {
		long nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1){
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiAuditItem, 
				nBillID, strAuditOldValue, strAuditNewValue, aepMedium, aetChanged);
		}
	}
}

// (j.jones 2013-08-06 15:37) - PLID 57299 - Calculates the HCFA form to use for
// an insured party. Optionally takes in a bill ID, -1 if we don't have one.
bool UseNewHCFAForm(long nInsuredPartyID, OPTIONAL long nBillID /*= -1*/)
{
	// (j.jones 2014-03-28 11:53) - PLID 61589 - this used to default to 10/1/2014, it is now 4/1/2014
	COleDateTime dtHCFAUpgradeDate = g_cdtDefaultHCFAUpgrade;
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	bool bOldFormAllowed = false;

	//don't run a recordset if we don't have an insured party ID
	if(nInsuredPartyID != -1) {
		_RecordsetPtr rsInsCo = CreateParamRecordset("SELECT InsuranceCoT.HCFAUpgradeDate, InsuranceCoT.HCFAOldFormAllowed "
			"FROM InsuranceCoT "
			"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
		if(!rsInsCo->eof) {
			dtHCFAUpgradeDate = VarDateTime(rsInsCo->Fields->Item["HCFAUpgradeDate"]->Value);
			bOldFormAllowed = VarBool(rsInsCo->Fields->Item["HCFAOldFormAllowed"]->Value) ? true : false;
		}
		rsInsCo->Close();

		//'Old form allowed' means that we use the old form even after
		//the upgrade date has passed, if the bill has at least one
		//service date from before the upgrade date.
		//This is not common, so we will not run the bill query unless we need to.
		if(bOldFormAllowed && nBillID != -1) {
			_RecordsetPtr rsServiceDate = CreateParamRecordset("SELECT Min(LineItemT.Date) AS FirstServiceDate "
				"FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "		
				"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", nBillID);
			if(!rsServiceDate->eof) {
				_variant_t varDate = rsServiceDate->Fields->Item["FirstServiceDate"]->Value;
				if(varDate.vt == VT_DATE) {
					COleDateTime dtFirstServiceDate = VarDateTime(varDate);
					//if the upgrade date is in the past, return true only if the earliest
					//service date is not before the upgrade date
					return (dtHCFAUpgradeDate <= dtNow && dtFirstServiceDate >= dtHCFAUpgradeDate);
				}
			}
			rsServiceDate->Close();
		}
	}

	//If we get here, then we did not use the 'old form allowed' logic.
	//Return true if we have passed the upgrade date.
	return dtHCFAUpgradeDate <= dtNow;
}

// (j.jones 2014-01-14 15:51) - PLID 58726 - Given a correction ID for a payment,
// this function will copy all the applies from the original payment to the new payment,
// effectively reapplying the new payment identically to the old payment.
// If nSkipChargeID is filled, applies to that charge will not be copied.
// (j.jones 2015-02-19 15:14) - PLID 64938 - this is obsolete, corrections now automatically do this
/*
void CopyOriginalPaymentApplies(long nLineItemCorrectionID, bool bIsERemitPosting, long nSkipChargeID /*= -1*//*)
{
	//Given a correction ID, find all the applies from this payment to something else,
	//skipping a given charge if requested. They may have since been corrected, but probably are not.
	//Also find all applies to this payment from something else, which is almost definitely corrected.
	
	//In both cases we hide anything that is a voiding line item in a correction, but not original line
	//items, as the purpose of this feature is to rebuild 'original' applies.cor
	//We also hide original corrections UNLESS they are the same timestamp as the one we're copying.

	CSqlFragment sqlSkipChargeID("");
	if(nSkipChargeID != -1) {
		sqlSkipChargeID = CSqlFragment(" AND AppliesT.DestID <> {INT} ", nSkipChargeID);
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemCorrectionsT.NewLineItemID, "
		"LineItemT.PatientID, PaymentsT.InsuredPartyID "
		"FROM LineItemCorrectionsT "
		"INNER JOIN PaymentsT ON LineItemCorrectionsT.NewLineItemID = PaymentsT.ID "
		"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"WHERE LineItemCorrectionsT.ID = {INT} AND LineItemT.Deleted = 0 \n"
		""		
		"SELECT AppliesT.DestID, AppliesT.Amount, AppliesT.PointsToPayments, PaymentsT.InsuredPartyID "
		"FROM AppliesT "
		"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
		"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "		
		"INNER JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
		"LEFT JOIN LineItemT LineItemT_Dest ON AppliesT.DestID = LineItemT_Dest.ID "
		"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT_Dest.ID = OrigLineItemsT.OriginalLineItemID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT_Dest.ID = VoidingLineItemsT.VoidingLineItemID "
		"WHERE LineItemCorrectionsT.ID = {INT} AND LineItemT.Deleted = 0 {SQL} "
		"AND VoidingLineItemsT.VoidingLineItemID IS NULL "
		"AND (OrigLineItemsT.OriginalLineItemID IS NULL OR LineItemCorrectionsT.InputDate = OrigLineItemsT.InputDate) "
		""
		"SELECT AppliesT.SourceID, AppliesT.Amount, AppliesT.PointsToPayments, PaymentsT.InsuredPartyID "
		"FROM AppliesT "
		"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
		"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"INNER JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
		"LEFT JOIN LineItemT LineItemT_Source ON AppliesT.SourceID = LineItemT_Source.ID "
		"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT_Source.ID = OrigLineItemsT.OriginalLineItemID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT_Source.ID = VoidingLineItemsT.VoidingLineItemID "
		"WHERE LineItemCorrectionsT.ID = {INT} AND LineItemT.Deleted = 0 "
		"AND VoidingLineItemsT.VoidingLineItemID IS NULL "
		"AND (OrigLineItemsT.OriginalLineItemID IS NULL OR LineItemCorrectionsT.InputDate = OrigLineItemsT.InputDate)",
		nLineItemCorrectionID,
		nLineItemCorrectionID, sqlSkipChargeID,
		nLineItemCorrectionID);

	//first recordset is the new payment ID created by the Void & Correct call
	long nNewPaymentID = -1;
	long nPatientID = -1;
	if(rs->eof) {
		//this should not be possible, this function should never
		//be called unless after the code created a Void & Correct correction
		ASSERT(FALSE);
		ThrowNxException("CopyOriginalPaymentApplies called on an invalid correction! (Correction ID %li)", nLineItemCorrectionID);
	}
	else {
		nNewPaymentID = VarLong(rs->Fields->Item["NewLineItemID"]->Value);
		nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);		
	}

	rs = rs->NextRecordset(NULL);

	//second recordset is everything the old payment was applied to, except nSkipChargeID
	while(!rs->eof) {

		long nOriginalDestID = VarLong(rs->Fields->Item["DestID"]->Value);
		COleCurrency cyAmount = VarCurrency(rs->Fields->Item["Amount"]->Value);
		BOOL bPointsToPayments = VarBool(rs->Fields->Item["PointsToPayments"]->Value);
		long nInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);

		//in this process it is not likely that the destination ID had been corrected,
		//but check it anyways
		long nLatestDestID = FindNewestCorrectedLineItemID(nOriginalDestID);

		//This should not be possible if this was called on a payment applied to a charge,
		//because this function is only called after only the payment was corrected.
		//If you hit this, confirm the function was not called anywhere it should not have been,
		//and review this function to see if its logic needs to change at all to support this
		//scenario.
		if(!bPointsToPayments) {
			ASSERT(nOriginalDestID == nLatestDestID);
		}
		
		if(nLatestDestID != -1) {
			ApplyPayToBill(nNewPaymentID, nPatientID, cyAmount, bPointsToPayments ? "Payment" : "Charge", nLatestDestID, nInsuredPartyID, bPointsToPayments, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, bIsERemitPosting ? TRUE : FALSE);
		}
		else {
			//This means the destination item was eventually voided, and not corrected.
			//There is currently no code that should ever hit this, so if you do
			//hit this, confirm the function was not called anywhere it should not
			//have been, and review this function to see if its logic needs to change
			//at all to support this scenario.
			ASSERT(FALSE);
		}

		rs->MoveNext();
	}

	rs = rs->NextRecordset(NULL);

	//third recordset is everything that was applied to the old payment
	while(!rs->eof) {

		long nOriginalSourceID = VarLong(rs->Fields->Item["SourceID"]->Value);
		COleCurrency cyAmount = VarCurrency(rs->Fields->Item["Amount"]->Value);
		bool bPointsToPayments = VarBool(rs->Fields->Item["PointsToPayments"]->Value) ? true : false;
		long nInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);

		//Unless it's bad data, this should never be false. Don't throw an exception though,
		//we are assuming this is always a payment even if it is false.
		//If you hit this, confirm if this is bad data or not, and if not, then
		//see if this logic needs re-worked.
		ASSERT(bPointsToPayments);

		//it is likely that this changed because if anything was applied to this payment (like a refund),
		//that applied item would have been corrected as well, so this call is required
		long nLatestSourceID = FindNewestCorrectedLineItemID(nOriginalSourceID);

		//This function is only called on payments we just corrected.
		//If anything was applied to that payment, it should have also been corrected.
		//If you hit this, find out why this wasn't the case. This code or the calling
		//code may need changed if this really isn't a corrected item.
		ASSERT(nLatestSourceID != nOriginalSourceID);

		if(nLatestSourceID != -1) {
			// (j.jones 2014-01-15 16:16) - I have no idea why, but if you're applying something to a payment,
			// you cannot pass in an insured party ID.
			ApplyPayToBill(nLatestSourceID, nPatientID, cyAmount, "Payment", nNewPaymentID, -1, bPointsToPayments ? TRUE : FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, bIsERemitPosting ? TRUE : FALSE);
		}
		else {
			//This means the source item was eventually voided, and not corrected.
			//There is currently no code that should ever hit this, so if you do
			//hit this, confirm the function was not called anywhere it should not
			//have been, and review this function to see if its logic needs to change
			//at all to support this scenario.
			ASSERT(FALSE);
		}

		rs->MoveNext();
	}
	rs->Close();
}
*/

// (b.spivey March 5th, 2014) - PLID 61182 - Determines if the bill should be using ICD10 or not. 
bool ShouldUseICD10(long nInsuredPartyID, long nBillID)
{
	
	//Defaults to invalid. Not having a date doesn't mean that it automatically turns over on 10/1/2014!
	COleDateTime dtICD10GoLiveDate = g_cdtInvalid;
	COleDateTime dtFirstServiceDate = g_cdtInvalid; 
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	
	//lets do a check for insured partyID
	if (!(nBillID > 0))	{
		// (b.spivey - March 6th, 2014) PLID 61182 - 
		//If you are seeing this then you called this function without a valid insured party ID and/or Bill ID. At the time of 
		//	 writing this, we assumed that there is no place that would have a valid reason to call this function 
		//	 without specifying the insured party ID and/or Bill ID. If you think there is, add handling for that case here, 
		//	 otherwise find the insured party ID and or Bill ID outside of this function. 
		ASSERT(FALSE);
		return false;
	}

	_RecordsetPtr rsInsCo;

	// (a.walling 2014-03-05 16:35) - PLID 61202 - I don't have a particular insured party available everywhere, so I included this to get it from the bill. At the very least we can determine whether this is OK eventually by finding callers to this overload.
	if (nInsuredPartyID == -1) {
		rsInsCo = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT InsuranceCoT.PersonID AS InsuredPartyID "
			"FROM BillsT "
			"INNER JOIN InsuredPartyT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
			"INNER JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"WHERE BillsT.ID = {INT}"
			, nBillID
		);

		if (!rsInsCo->eof) {
			nInsuredPartyID = AdoFldLong(rsInsCo->Fields, "InsuredPartyID", -1);
		}

		if (!(nInsuredPartyID > 0)) {
			return false; 
		}

	} 

	//Get that bill and find that date. 
	_RecordsetPtr rsServiceDate = CreateParamRecordset(GetRemoteDataSnapshot(), 
		"SELECT Min(LineItemT.Date) AS FirstServiceDate "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "		
		"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", nBillID);
	if(!rsServiceDate->eof) {
		dtFirstServiceDate = AdoFldDateTime(rsServiceDate->Fields, "FirstServiceDate", g_cdtInvalid); 
	}
	rsServiceDate->Close();

	//These could've been null or invalid, either way this is false. 
	if (dtFirstServiceDate.GetStatus() != COleDateTime::valid) {
		return false; 
	}

	//Should be good to return this check if we get this far. 
	return ShouldUseICD10(nInsuredPartyID, dtFirstServiceDate);  
}

// (b.spivey March 7th, 2014) - PLID 61247 - Takes an insured party and date and figures out if we want to use ICD10 or not. 
bool ShouldUseICD10(long nInsuredPartyID, COleDateTime dtProcessDate)
{
	//Defaults to invalid. Not having a date doesn't mean that it automatically turns over on 10/1/2014!
	COleDateTime dtICD10GoLiveDate = g_cdtInvalid;
	
	//lets do a check for insured partyID
	if (!(nInsuredPartyID > 0))	{
		//There is no way to get the go live date without this. 
		//	 If you're getting this, you may want the other ShouldUseICD10 function.
		ASSERT(FALSE); 
		return false;
	}

	_RecordsetPtr rsInsCo;

	//Get the insured party and get that date. 
	rsInsCo = CreateParamRecordset(GetRemoteDataSnapshot(), 
		"SELECT InsuranceCoT.ICD10GoLiveDate "
		"FROM InsuranceCoT "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
		"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
	
	if(!rsInsCo->eof) {
		dtICD10GoLiveDate = AdoFldDateTime(rsInsCo->Fields, "ICD10GoLiveDate", g_cdtInvalid);
	}
	rsInsCo->Close();

	// if they have no ICD10 Go Live date, return false before querying for charge dates
	if (dtICD10GoLiveDate.GetStatus() != COleDateTime::valid) {
		return false; 
	}

	//These could've been null or invalid, either way this is false. 
	if (dtProcessDate.GetStatus() != COleDateTime::valid) {
		return false; 
	}

	//Should be good to return this check if we get this far. 
	return (dtProcessDate >= dtICD10GoLiveDate); 
}

// (j.jones 2014-04-30 13:19) - PLID 61838 - Calculate which specialty charge providers are required for a combination
// of service code, charge provider, bill location, and insured party.
// If they have no setup information, successive calls won't touch the database unless bForceReload is set to true.
ChargeClaimProviderSettings CalculateChargeClaimProviders(const long nServiceID, const long nProviderID, const long nLocationID, const long nInsuredPartyID, const bool bForceReload)
{
	try {

		if (nServiceID == -1 || nProviderID == -1 || nLocationID == -1 || nInsuredPartyID == -1) {
			//the code should not have called this function without these four IDs
			ASSERT(FALSE);

			//return an empty settings object
			ChargeClaimProviderSettings pEmpty;
			return pEmpty;
		}

		//cache whether they even have data set up
		static bool bHasCheckedSetup = false;
		static bool bHasSetupData = false;

		//if bForceReload is true, clear the cached flags
		//such that we always re-check the setup
		if (bForceReload) {
			bHasCheckedSetup = false;
			bHasSetupData = false;
		}

		if (bHasCheckedSetup && !bHasSetupData) {
			//return an empty settings object
			ChargeClaimProviderSettings pEmpty;
			return pEmpty;
		}

		CSqlFragment sqlCheckSetup("");
		if (!bHasCheckedSetup) {
			//the first check will lookup the requested info., but
			//also check to see if any rows exist with these settings enabled
			sqlCheckSetup = CSqlFragment("\r\nSELECT TOP 1 ServiceID FROM ChargeLevelProviderConfigT "
				"WHERE (ReferringProviderOption <> {CONST_INT} OR OrderingProviderOption <> {CONST_INT} OR SupervisingProviderOption <> {CONST_INT});",
				ChargeLevelProviderConfigOption::NoSelection, ChargeLevelProviderConfigOption::NoSelection, ChargeLevelProviderConfigOption::NoSelection);
		}

		//if a specific provider is selected as the default, but that provider is inactive,
		//we'll return -1, and it will appear to the caller as though the setup has no default
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"ChargeLevelProviderConfigT.ReferringProviderOption, ReferringPersonT.ID AS ReferringProviderID, "
			"ChargeLevelProviderConfigT.OrderingProviderOption, OrderingPersonT.ID AS OrderingProviderID, "
			"ChargeLevelProviderConfigT.SupervisingProviderOption, SupervisingPersonT.ID AS SupervisingProviderID "
			"FROM ChargeLevelProviderConfigT "
			"INNER JOIN InsuranceCoT ON ChargeLevelProviderConfigT.HCFAGroupID = InsuranceCoT.HCFASetupGroupID "
			"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ReferringPersonT ON ChargeLevelProviderConfigT.ReferringProviderID = ReferringPersonT.ID "
			"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS OrderingPersonT ON ChargeLevelProviderConfigT.OrderingProviderID = OrderingPersonT.ID "
			"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS SupervisingPersonT ON ChargeLevelProviderConfigT.SupervisingProviderID = SupervisingPersonT.ID "
			"WHERE ServiceID = {INT} AND ProviderID = {INT} AND LocationID = {INT} AND InsuredPartyT.PersonID = {INT} "
			"AND (ReferringProviderOption <> {CONST_INT} OR OrderingProviderOption <> {CONST_INT} OR SupervisingProviderOption <> {CONST_INT}); "
			""
			"{SQL}",
			nServiceID, nProviderID, nLocationID, nInsuredPartyID,
			ChargeLevelProviderConfigOption::NoSelection, ChargeLevelProviderConfigOption::NoSelection, ChargeLevelProviderConfigOption::NoSelection,
			sqlCheckSetup);

		if (!rs->eof) {
			//if we have a hit, clearly we have data
			bHasCheckedSetup = true;
			bHasSetupData = true;

			//fill our settings object
			ChargeClaimProviderSettings pSettings;
			pSettings.eReferringProviderOption = (ChargeLevelProviderConfigOption)VarLong(rs->Fields->Item["ReferringProviderOption"]->Value);
			pSettings.nReferringProviderID = VarLong(rs->Fields->Item["ReferringProviderID"]->Value, -1);
			pSettings.eOrderingProviderOption = (ChargeLevelProviderConfigOption)VarLong(rs->Fields->Item["OrderingProviderOption"]->Value);
			pSettings.nOrderingProviderID = VarLong(rs->Fields->Item["OrderingProviderID"]->Value, -1);
			pSettings.eSupervisingProviderOption = (ChargeLevelProviderConfigOption)VarLong(rs->Fields->Item["SupervisingProviderOption"]->Value);
			pSettings.nSupervisingProviderID = VarLong(rs->Fields->Item["SupervisingProviderID"]->Value, -1);
			return pSettings;
		}

		//if this is false, we should have a second recordset
		if (!bHasCheckedSetup) {
			rs = rs->NextRecordset(NULL);
			bHasCheckedSetup = true;
			if (!rs->eof) {
				//if this is non-empty, we know setup data exists,
				//it just doesn't for this requested data
				bHasSetupData = true;
			}
			else {
				//there is no setup data, so we don't need to check again
				bHasSetupData = false;
			}
		}
		

	}NxCatchAll(__FUNCTION__)

	//return an empty settings object
	ChargeClaimProviderSettings pEmpty;
	return pEmpty;
}

// (j.jones 2014-06-24 11:47) - PLID 60349 - Calculates the next insured party ID or resp type ID for
// a patient of the same insurance type (Medical, Vision, etc.).
// So if the current insured party ID is primary medical, the next insured party is Secondary Medical
// if one exists, or -1 if none exists.
long GetNextInsuredPartyIDByPriority(long nPatientID, long nCurInsuredPartyID)
{
	long nRespTypeID = GetInsuranceTypeFromID(nCurInsuredPartyID);

	long nNextInsuredPartyID = -1;
	long nNextRespTypeID = 0;

	GetNextInsuredPartyByPriority(nPatientID, nRespTypeID, nNextInsuredPartyID, nNextRespTypeID);

	return nNextInsuredPartyID;
}

long GetNextRespTypeByPriority(long nPatientID, long nRespTypeID)
{
	long nNextInsuredPartyID = -1;
	long nNextRespTypeID = 0;

	GetNextInsuredPartyByPriority(nPatientID, nRespTypeID, nNextInsuredPartyID, nNextRespTypeID);

	return nNextRespTypeID;
}

void GetNextInsuredPartyByPriority(long nPatientID, long nRespTypeID, OUT long &nNextInsuredPartyID, OUT long &nNextRespTypeID)
{
	nNextInsuredPartyID = -1;
	nNextRespTypeID = 0;	//default to 0 for patient, not -1

	// (j.jones 2010-09-02 16:40) - PLID 40392 - we need the next resp that's the same category
	// as our current resp, can't auto-shift across categories!
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID, RespTypeT.ID "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"CROSS JOIN (SELECT Priority, CategoryType FROM RespTypeT WHERE ID = {INT}) AS CurrentRespTypeQ "
		"WHERE InsuredPartyT.PatientID = {INT} "
		"AND RespTypeT.Priority > CurrentRespTypeQ.Priority "
		"AND RespTypeT.CategoryType = CurrentRespTypeQ.CategoryType", nRespTypeID, nPatientID);
	if (!rs->eof) {
		nNextInsuredPartyID = AdoFldLong(rs, "PersonID", -1);
		nNextRespTypeID = AdoFldLong(rs, "ID", 0);
	}
	rs->Close();
}

//TES 7/25/2014 - PLID 63048 - Used to identify charges that cannot be deleted, due to being associated with Chargebacks
bool DoesChargeHaveChargeback(CSqlFragment sqlChargesTFilter)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID "
		"FROM ChargesT "
		"LEFT JOIN ChargebacksT Chargebacks_Charges ON ChargesT.ID = Chargebacks_Charges.ChargeID "
		"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
		"LEFT JOIN ChargebacksT Chargebacks_Pays ON AppliesT.SourceID = Chargebacks_Pays.PaymentID OR AppliesT.SourceID = Chargebacks_Pays.AdjustmentID "
		"WHERE {SQL} AND "
		"(Chargebacks_Charges.ID Is Not Null OR Chargebacks_Pays.ID Is Not Null)", sqlChargesTFilter);
	if (rs->eof) {
		return false;
	}
	else {
		return true;
	}
}

//TES 7/25/2014 - PLID 63049 - Used to identify payments that cannot be deleted, due to being associated with Chargebacks
bool DoesPayHaveChargeback(CSqlFragment sqlPaymentsTFilter)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT PaymentsT.ID "
		"FROM PaymentsT "
		"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
		"WHERE {SQL} AND "
		"(ChargebacksT.ID Is Not Null)", sqlPaymentsTFilter);
	if (rs->eof) {
		return false;
	}
	else {
		return true;
	}
}

// (j.jones 2014-07-08 09:07) - PLID 62570 - added the ability to split charges to a new bill
// returns the new bill ID, -1 if one was not made
long SplitChargesIntoNewBill(long nCurBillID, CArray<long, long> &aryChargeIDsToMove)
{
	try {

		if (aryChargeIDsToMove.GetCount() == 0) {
			//the caller should not have called this function
			//without charges to split
			ASSERT(FALSE);
			ThrowNxException("No charges were selected to split into the new bill.");
			return -1;
		}

		//make sure that we have an array of valid charge IDs
		for (int i = 0; i < aryChargeIDsToMove.GetCount(); i++) {
			if (aryChargeIDsToMove.GetAt(i) <= 0) {
				//an invalid charge ID was found - the bill should have been saved first
				ASSERT(FALSE);
				ThrowNxException("The provided charges have not yet been saved.");
				return -1;
			}
		}

		//now create the new bill, move the charges,
		//and copy all relevant data

		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON");

		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @CurrentBillID INT");
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @CurrentBillID = {INT}", nCurBillID);

		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargesToMove TABLE (ID INT NOT NULL PRIMARY KEY)");
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO @ChargesToMove (ID) "
			"SELECT ID FROM ChargesT WHERE ID IN ({INTARRAY}) GROUP BY ID", aryChargeIDsToMove);

		//if we are moving any corrected charges, we need to also move their original & void charges
		//this needs to be recursive incase the charge was corrected multiple times

		//this also must filter only on the current bill, we don't care if the original charge was
		//on another bill

		AddStatementToSqlBatch(strSqlBatch, "DECLARE @recurseAgain BIT \r\n"
			"SET @recurseAgain = 1 \r\n"
			"WHILE (@recurseAgain = 1) \r\n"
			"BEGIN \r\n"
			//recurseAgain will only be set to 1 if we add new charges to the list
			"	SET @recurseAgain = 0 \r\n"
			"	DECLARE rsCorrections CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"	SELECT OriginalLineItemID, VoidingLineItemID, NewLineItemID FROM LineItemCorrectionsT "
			"	INNER JOIN (SELECT ID FROM ChargesT WHERE BillID = @CurrentBillID) ChargesTOriginal ON LineItemCorrectionsT.OriginalLineItemID = ChargesTOriginal.ID "
			"	INNER JOIN (SELECT ID FROM ChargesT WHERE BillID = @CurrentBillID) ChargesTVoiding ON LineItemCorrectionsT.VoidingLineItemID = ChargesTVoiding.ID "
			"	LEFT JOIN (SELECT ID FROM ChargesT WHERE BillID = @CurrentBillID) ChargesTNew ON LineItemCorrectionsT.NewLineItemID = ChargesTNew.ID "
			"	WHERE ChargesTOriginal.ID IN (SELECT ID FROM @ChargesToMove) "
			"		OR ChargesTVoiding.ID IN (SELECT ID FROM @ChargesToMove) "
			"		OR (ChargesTNew.ID Is Not Null AND ChargesTNew.ID IN (SELECT ID FROM @ChargesToMove)) "	
			"	OPEN rsCorrections \r\n"
			"	DECLARE @OriginalLineItemID INT, @VoidingLineItemID INT, @NewLineItemID INT \r\n"
			"	FETCH NEXT FROM rsCorrections INTO @OriginalLineItemID, @VoidingLineItemID, @NewLineItemID \r\n"
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			//	add each charge to our table of charges to move
			"		IF NOT EXISTS (SELECT ID FROM @ChargesToMove WHERE ID = @OriginalLineItemID) \r\n"
			"		BEGIN \r\n"
			"			INSERT INTO @ChargesToMove (ID) VALUES (@OriginalLineItemID) \r\n"
			"			SET @recurseAgain = 1 \r\n"
			"		END \r\n"
			"		IF NOT EXISTS (SELECT ID FROM @ChargesToMove WHERE ID = @VoidingLineItemID) \r\n"
			"		BEGIN \r\n"
			"			INSERT INTO @ChargesToMove (ID) VALUES (@VoidingLineItemID) \r\n"
			"			SET @recurseAgain = 1 \r\n"
			"		END \r\n"
			"		IF @NewLineItemID Is Not Null AND NOT EXISTS (SELECT ID FROM @ChargesToMove WHERE ID = @NewLineItemID) \r\n"
			"		BEGIN \r\n"
			"			INSERT INTO @ChargesToMove (ID) VALUES (@NewLineItemID) \r\n"
			"			SET @recurseAgain = 1 \r\n"
			"		END \r\n"
			""
			//now recurse back on the original line item ID
			""
			"		FETCH NEXT FROM rsCorrections INTO @OriginalLineItemID, @VoidingLineItemID, @NewLineItemID \r\n"
			"	END \r\n"	//end looping through rsCorrections
			"	CLOSE rsCorrections \r\n"
			"	DEALLOCATE rsCorrections \r\n"
			"END"	//end the @recurseAgain loop
			);
				
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @OldBillID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewBillID INT");
		//this is meant to fail if the charges are from different bills
		AddStatementToSqlBatch(strSqlBatch, "SET @OldBillID = (SELECT BillID FROM ChargesT WHERE ID IN (SELECT ID FROM @ChargesToMove) GROUP BY BillID)");
		AddStatementToSqlBatch(strSqlBatch, "IF (@OldBillID Is Null) BEGIN RAISERROR('No valid bill ID was found!', 16, 1) ROLLBACK TRAN RETURN END \r\n");
		//make sure that this is the same bill ID we were provided
		AddStatementToSqlBatch(strSqlBatch, "IF (@OldBillID <> @CurrentBillID) BEGIN RAISERROR('A different bill ID was found on at least one charge than the bill that is being split!', 16, 1) ROLLBACK TRAN RETURN END \r\n");

		//fail if the old bill would no longer have charges on it
		AddStatementToSqlBatch(strSqlBatch, "IF NOT EXISTS (SELECT ChargesT.ID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE ChargesT.BillID = @OldBillID AND LineItemT.Deleted = 0 "
			"AND ChargesT.ID NOT IN (SELECT ID FROM @ChargesToMove)) "
			"BEGIN RAISERROR('Splitting charges failed because the old bill would no longer have charges remaining.', 16, 1) ROLLBACK TRAN RETURN END");
		
		//create the new bill entry, it will be identical to the old bill except for
		//the current date as the input date and current user as the creator
		//StatusNoteID is intentionally not copied; I'm explicitly stating it as NULL here
		// (a.walling 2016-03-09 15:25) - PLID 68560 - UB04 Enhancements - update bill splitting for financial corrections and SplitChargesIntoNewBill
		// - added UB04ClaimInfo
		// - removed UB92Box32|UB92Box33|UB92Box34|UB92Box35|UB92Box36|UB04Box36")
		// (r.gonet 2016-04-07) - NX-100072 - Split FirstConditionDate into mulitple date fields.
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO BillsT (InputDate, InputName, "
			"PatientID, Date, EntryType, ExtraDesc, RelatedToEmp, RelatedToAutoAcc, State, RelatedToOther, ConditionDate, "
			"FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, "
			"NoWorkFrom, NoWorkTo, HospFrom, HospTo, RefPhyID, OutsideLab, OutsideLabCharges, MedicaidResubmission, OriginalRefNo, PriorAuthNum, "
			"HCFABlock19, Note, Location, InsuredPartyID, OthrInsuredPartyID, Description, PatCoord, UseExp, ExpDays, FormType, InsuranceReferralID, "
			"Active, HCFABox10D, AnesthStartTime, AnesthEndTime, FacilityStartTime, FacilityEndTime, AnesthesiaMinutes, FacilityMinutes, SendCorrespondence, "
			"UB92Box79, UB92Box44, DischargeStatusID, DeductibleLeftToMeet, CoInsurance, "
			"OutOfPocketLeftToMeet, SendPaperwork, PaperworkType, PaperworkTx, SendTestResults, TestResultID, TestResultType, TestResult, "
			"SupervisingProviderID, ManualReview, ANSI_ClaimTypeCode, PriorAuthType, HCFABox13Over, AffiliatePhysID, AffiliatePhysAmount, AffiliateNote, "
			"AffiliateStatusID, AssistingMinutes, AssistingStartTime, AssistingEndTime, ConditionDateType, UBBox14, UBBox15, LastQuoteReportID, LastQuoteReportNumber, "
			"AdmissionTime, DischargeTime, HCFABox8, HCFABox9b, HCFABox9c, HCFABox11bQual, HCFABox11b, StatusID, StatusModifiedDate, StatusNoteID, UB04ClaimInfo) "
			""
			"SELECT GetDate(), {INT}, "
			"PatientID, Date, EntryType, ExtraDesc, RelatedToEmp, RelatedToAutoAcc, State, RelatedToOther, ConditionDate, "
			"FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, "
			"NoWorkFrom, NoWorkTo, HospFrom, HospTo, RefPhyID, OutsideLab, OutsideLabCharges, MedicaidResubmission, OriginalRefNo, PriorAuthNum, "
			"HCFABlock19, Note, Location, InsuredPartyID, OthrInsuredPartyID, Description, PatCoord, UseExp, ExpDays, FormType, InsuranceReferralID, "
			"Active, HCFABox10D, AnesthStartTime, AnesthEndTime, FacilityStartTime, FacilityEndTime, AnesthesiaMinutes, FacilityMinutes, SendCorrespondence, "
			"UB92Box79, UB92Box44, DischargeStatusID, DeductibleLeftToMeet, CoInsurance, "
			"OutOfPocketLeftToMeet, SendPaperwork, PaperworkType, PaperworkTx, SendTestResults, TestResultID, TestResultType, TestResult, "
			"SupervisingProviderID, ManualReview, ANSI_ClaimTypeCode, PriorAuthType, HCFABox13Over, AffiliatePhysID, AffiliatePhysAmount, AffiliateNote, "
			"AffiliateStatusID, AssistingMinutes, AssistingStartTime, AssistingEndTime, ConditionDateType, UBBox14, UBBox15, LastQuoteReportID, LastQuoteReportNumber, "
			"AdmissionTime, DischargeTime, HCFABox8, HCFABox9b, HCFABox9c, HCFABox11bQual, HCFABox11b, StatusID, StatusModifiedDate, NULL, UB04ClaimInfo "
			"FROM BillsT WHERE ID = @OldBillID", GetCurrentUserID());
		AddStatementToSqlBatch(strSqlBatch, "SET @NewBillID = CAST(SCOPE_IDENTITY() AS INT)");

		//move the charges to this bill
		AddStatementToSqlBatch(strSqlBatch, "UPDATE ChargesT SET BillID = @NewBillID WHERE ID IN (SELECT ID FROM @ChargesToMove)");

		//now deal with additional tables that need updates

		//need to copy the bill diagnosis codes, and remap the charge which codes
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO BillDiagCodeT (BillID, ICD9DiagID, ICD10DiagID, OrderIndex) "
			"SELECT @NewBillID, ICD9DiagID, ICD10DiagID, OrderIndex FROM BillDiagCodeT WHERE BillID = @OldBillID");
		AddStatementToSqlBatch(strSqlBatch, "UPDATE ChargeWhichCodesT SET ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID "
			"FROM ChargeWhichCodesT "
			"INNER JOIN ChargesT ON ChargeWhichCodesT.ChargeID = ChargesT.ID AND ChargesT.ID IN (SELECT ID FROM @ChargesToMove) "
			"INNER JOIN BillDiagCodeT ON ChargesT.BillID = BillDiagCodeT.BillID "
			"INNER JOIN BillDiagCodeT BillDiagCodeT_Old ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT_Old.ID AND BillDiagCodeT_Old.OrderIndex = BillDiagCodeT.OrderIndex");

		//create invoice numbers only if the preference says so
		//it's possible it may have been on for the old bill, and off now
		if (GetRemotePropertyInt("EnableBillInvoiceNumbers", 0, 0, "<None>", true) == 1) {
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @FirstProviderID INT");
			AddStatementToSqlBatch(strSqlBatch, "SELECT TOP 1 @FirstProviderID = DoctorsProviders "
				"FROM ChargesT "
				"WHERE Coalesce(DoctorsProviders, -1) <> -1 "
				"AND ID IN (SELECT ID FROM @ChargesToMove) "
				"ORDER BY LineID");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewInvoiceID INT");
			AddStatementToSqlBatch(strSqlBatch,
				"IF (Coalesce(@FirstProviderID, -1) <> -1) \r\n"
				"BEGIN \r\n"
				"	SELECT @NewInvoiceID = COALESCE(MAX(InvoiceID), 0) + 1 FROM BillInvoiceNumbersT WITH(UPDLOCK, HOLDLOCK) WHERE ProviderID = @FirstProviderID \r\n"
				"	INSERT INTO BillInvoiceNumbersT (BillID, ProviderID, InvoiceID) VALUES (@NewBillID, @FirstProviderID, @NewInvoiceID) \r\n"
				"END \r\n");
		}

		//billing notes need copied, not moved
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @OldBillNoteID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewBillNoteID INT");
		// (r.gonet 07/24/2014) - PLID 62524 - Copy the IsBillStatusNote flag
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE rsBillNotes CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"SELECT NoteID FROM NoteInfoT WHERE BillID = @OldBillID \r\n"
			"OPEN rsBillNotes \r\n"
			"FETCH NEXT FROM rsBillNotes INTO @OldBillNoteID \r\n"
			"WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			"	INSERT INTO NoteDataT (PersonID, Date, UserID, Category, Note, Priority, NoteInputDate) "
			"		SELECT PersonID, Date, {INT}, Category, Note, Priority, GetDate() FROM NoteDataT WHERE ID = @OldBillNoteID \r\n"
			"	SET @NewBillNoteID = CAST(SCOPE_IDENTITY() AS INT) \r\n"
			"	INSERT INTO NoteInfoT (NoteID, BillID, MailID, PatientMessagingThreadID, IsPatientCreated, ShowOnStatement, SendOnClaim, IsBillStatusNote) "
			"		SELECT @NewBillNoteID, @NewBillID, MailID, PatientMessagingThreadID, IsPatientCreated, ShowOnStatement, SendOnClaim, IsBillStatusNote "
			"		FROM NoteInfoT WHERE NoteID = @OldBillNoteID \r\n"
			//if this is the bill status note ID, copy the new ID to the new bill
			"	IF EXISTS (SELECT ID FROM BillsT WHERE ID = @OldBillID AND StatusNoteID = @OldBillNoteID) \r\n"
			"	BEGIN \r\n"
			"		UPDATE BillsT SET StatusNoteID = @NewBillNoteID WHERE ID = @NewBillID \r\n"
			"	END \r\n"
			"	FETCH NEXT FROM rsBillNotes INTO @OldBillNoteID \r\n"
			"END \r\n"
			"CLOSE rsBillNotes \r\n"
			"DEALLOCATE rsBillNotes \r\n",
			GetCurrentUserID());

		//copy claim history information, then move charge details to the copy
		//this keeps the user who originally submitted the claim,
		//and moves, not copies, details for the charges that were moved
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @OldClaimHistoryID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewClaimHistoryID INT");
		AddStatementToSqlBatch(strSqlBatch, "DECLARE rsClaimHistory CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"SELECT ClaimHistoryT.ID FROM ClaimHistoryT "
			"INNER JOIN ClaimHistoryDetailsT ON ClaimHistoryT.ID = ClaimHistoryDetailsT.ClaimHistoryID "
			"WHERE ClaimHistoryT.BillID = @OldBillID "
			"AND ClaimHistoryDetailsT.ChargeID IN (SELECT ID FROM @ChargesToMove) "
			"GROUP BY ClaimHistoryT.ID \r\n"
			"OPEN rsClaimHistory \r\n"
			"FETCH NEXT FROM rsClaimHistory INTO @OldClaimHistoryID \r\n"
			"WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			"	INSERT INTO ClaimHistoryT (BillID, InsuredPartyID, SendType, Clearinghouse, Date, UserName) "
			"		SELECT @NewBillID, InsuredPartyID, SendType, Clearinghouse, Date, UserName FROM ClaimHistoryT WHERE ID = @OldClaimHistoryID \r\n"
			"	SET @NewClaimHistoryID = CAST(SCOPE_IDENTITY() AS INT) \r\n"
			"	UPDATE ClaimHistoryDetailsT SET ClaimHistoryID = @NewClaimHistoryID "
			"		WHERE ClaimHistoryID = @OldClaimHistoryID "
			"		AND ChargeID IN (SELECT ID FROM @ChargesToMove) \r\n"
			"	FETCH NEXT FROM rsClaimHistory INTO @OldClaimHistoryID \r\n"
			"END \r\n"
			"CLOSE rsClaimHistory \r\n"
			"DEALLOCATE rsClaimHistory");

		//copy the batch status, but wipe the validation and current select flags
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewHCFATrackID INT");
		AddStatementToSqlBatch(strSqlBatch, "SELECT @NewHCFATrackID = COALESCE(MAX(ID), 0) + 1 FROM HCFATrackT WITH(UPDLOCK, HOLDLOCK)");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO HCFATrackT (ID, Date, Batch, BillID, CurrentSelect, LastSendDate, ValidationState) "
			"SELECT @NewHCFATrackID, Date, Batch, @NewBillID, 0, LastSendDate, 0 FROM HCFATrackT WHERE BillID = @OldBillID");
		
		//all done!
		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT OFF");
		
		//get the new bill ID and information for auditing
		AddStatementToSqlBatch(strSqlBatch, "SELECT @NewBillID AS NewBillID, BillsT.PatientID, PersonT.FullName, "
			"BillsT.Date, BillsT.Description, (SELECT Count(*) FROM @ChargesToMove) AS CountChargesMoved "
			"FROM BillsT "
			"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
			"WHERE BillsT.ID = @OldBillID");

		AddStatementToSqlBatch(strSqlBatch, "SELECT ChargesT.ID, LineItemT.Description, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE ChargesT.BillID = @NewBillID AND LineItemT.Deleted = 0");

		_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);		

		long nNewBillID = -1;
		long nAuditID = -1, nPatientID = -1;
		CString strPatientName, strOldBillAuditDesc;

		if (!rs->eof) {
			//grab the new bill ID to return to the caller
			nNewBillID = VarLong(rs->Fields->Item["NewBillID"]->Value);

			//audit this action
			nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
			strPatientName = VarString(rs->Fields->Item["FullName"]->Value);
			CString strBillDescription = VarString(rs->Fields->Item["Description"]->Value);
			COleDateTime dtBillDate = VarDateTime(rs->Fields->Item["Date"]->Value);
			
			strOldBillAuditDesc.Format("From Bill: %s (%s)", strBillDescription, FormatDateTimeForInterface(dtBillDate, NULL, dtoDate));

			CString strNewValue;
			strNewValue.Format("%li charges were moved to the new bill.", VarLong(rs->Fields->Item["CountChargesMoved"]->Value));

			nAuditID = BeginNewAuditEvent();
			AuditEvent(nPatientID, strPatientName, nAuditID, aeiBillCreatedFromExistingCharges, nNewBillID, strOldBillAuditDesc, strNewValue, aepHigh, aetCreated);
		}
		else {
			//this should have been impossible
			ASSERT(FALSE);
			ThrowNxException("Failed to split charges into a new bill.");
		}

		rs = rs->NextRecordset(NULL);

		while (!rs->eof) {

			if (nNewBillID == -1 || nAuditID == -1) {
				//this should be impossible
				ASSERT(FALSE);
				ThrowNxException("Failed to split charges into a new bill.");
			}

			//audit every charge as having been moved
			long nChargeID = VarLong(rs->Fields->Item["ID"]->Value);
			COleCurrency cyChargeTotal = VarCurrency(rs->Fields->Item["ChargeTotal"]->Value);
			CString strChargeDescription = VarString(rs->Fields->Item["Description"]->Value);

			CString strNewValue;
			strNewValue.Format("%s charge: %s", FormatCurrencyForInterface(cyChargeTotal), strChargeDescription);
			AuditEvent(nPatientID, strPatientName, nAuditID, aeiChargeMovedToNewBill, nChargeID, strOldBillAuditDesc, strNewValue, aepHigh, aetChanged);

			rs->MoveNext();
		}
		rs->Close();

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, copy what the split bill had (unbatched to electronic or paper)
		rs = CreateParamRecordset("SELECT Batch FROM HCFATrackT WHERE BillID = {INT}", nNewBillID);
		if (!rs->eof) {
			long nNewValue = AdoFldLong(rs, "Batch");
			std::vector<long> aryBillID;
			aryBillID.push_back(nNewBillID);
			AuditInsuranceBatch(aryBillID, nNewValue, 0);
		}
		rs->Close();

		return nNewBillID;

	}NxCatchAll(__FUNCTION__);

	return -1;
}

// (r.gonet 07/09/2014) - PLID 62571 - Changes a bill's on hold status.
bool SetBillOnHold(long nBillID, BOOL bOnHold, BOOL bSilent/*= FALSE*/)
{
	long nAuditTransactionID = -1;
	try {
		nAuditTransactionID = BeginAuditTransaction();

		if (nBillID < 0) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}

		// (r.gonet 07/07/2014) - PLID 62571 - Get the old status and bill description. Also get the system On Hold status. Also get the patient id and name so we can audit.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT BillStatusT.ID AS StatusID, BillStatusT.Name AS StatusName, BillStatusT.Type AS StatusType, BillStatusT.Custom, BillsT.Description AS BillDescription, \r\n"
			"PersonT.ID AS PatientID, PersonT.FullName AS PatientFullName \r\n"
			"FROM BillsT \r\n"
			"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID \r\n"
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n"
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID \r\n"
			"WHERE BillsT.ID = {INT}; \r\n"
			"\r\n"
			"SELECT TOP 1 BillStatusT.ID AS StatusID, BillStatusT.Name AS StatusName \r\n"
			"FROM BillStatusT \r\n"
			"WHERE BillStatusT.Type = {CONST_INT} \r\n"
			"AND BillStatusT.Inactive = 0 \r\n"
			"AND BillStatusT.Custom = 0 \r\n"
			, nBillID, (long)EBillStatusType::OnHold);
		if (prs->eof) {
			// Even deleted bills would be returned here... so this is exceptional.
			ThrowNxException("%s : Could not find patient bill with ID = %li", __FUNCTION__, nBillID);
		}
		// (r.gonet 07/07/2014) - PLID 62571 - We need the old status to audit.
		long nOldStatusID = AdoFldLong(prs->Fields, "StatusID", -1);
		CString strOldStatusName = AdoFldString(prs->Fields, "StatusName", "<No Status>");
		EBillStatusType eOldStatusType = (EBillStatusType)AdoFldLong(prs->Fields, "StatusType", (long)EBillStatusType::None);
		CString strOldBillDescription = AdoFldString(prs->Fields, "BillDescription", "");
		BOOL bOldStatusIsCustom = AdoFldBool(prs->Fields, "Custom", FALSE);
		long nPatientID = AdoFldLong(prs->Fields, "PatientID");
		CString strPatientFullName = AdoFldString(prs->Fields, "PatientFullName");

		// (r.gonet 07/07/2014) - PLID 62571 - We need to update the bill's description to either add
		// or remove the On Hold prefix. The prefix is part of the description though, so we need to
		// parse out the description without the prefix.
		CString strOldBillDescriptionNoPrefix;
		CString strBillOnHoldPrefix = "ON HOLD: ";
		if (eOldStatusType == EBillStatusType::OnHold && strOldBillDescription.Find(strBillOnHoldPrefix) == 0) {
			strOldBillDescriptionNoPrefix = strOldBillDescription.Mid(strBillOnHoldPrefix.GetLength());
		} else {
			// (r.gonet 07/07/2014) - PLID 62571 - Either the bill is not on hold or it doesn't contain a prefix, oddly enough.
			strOldBillDescriptionNoPrefix = strOldBillDescription;
		}

		_variant_t varNewStatusID;
		CString strNewStatusName;
		CString strNewBillDescription;

		// (r.gonet 07/07/2014) - PLID 62571 - Add or remove the prefix according to the desired on hold state.
		if (bOnHold) {
			strNewBillDescription = strBillOnHoldPrefix + strOldBillDescriptionNoPrefix;
		} else {
			strNewBillDescription = strOldBillDescriptionNoPrefix;
		}
		// (r.gonet 07/07/2014) - PLID 62571 - Bill description is limited to 255 characters. Truncate silently.
		strNewBillDescription = strNewBillDescription.Left(255);

		prs = prs->NextRecordset(NULL);

		// (r.gonet 07/07/2014) - PLID 62571 - Get the ID and name of the new bill status.
		if (bOnHold) {
			if (!prs->eof) {
				varNewStatusID = _variant_t(AdoFldLong(prs->Fields, "StatusID"), VT_I4);
				strNewStatusName = AdoFldString(prs->Fields, "StatusName");
			} else {
				ThrowNxException("%s : Practice could not find a non-custom, On Hold bill status.", __FUNCTION__);
			}
		} else {
			// (r.gonet 07/07/2014) - PLID 62571 - This is easy since the bill's status will just be nulled out.
			varNewStatusID = g_cvarNull;
			strNewStatusName = "<No Status>";
		}
		prs->Close();

		// (r.gonet 07/07/2014) - PLID 62571 - If we aren't doing anything, don't bother running any further queries.
		if (nOldStatusID == VarLong(varNewStatusID, -1)) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}

		// If the Bill is set to an 'On Hold' status other than the built in 'On Hold' status, prompt the user.
		if (!bOnHold && bOldStatusIsCustom && !bSilent) {
			if (IDYES != MsgBox(MB_ICONQUESTION | MB_YESNO,
				"This bill has a current status of \"%s\", which is an 'On Hold' status. Removing the hold from this bill will reset its status. Are you sure you wish to do this?",
				strOldStatusName)) {

				if (nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return false;
			}
		}

		// (r.gonet 07/07/2014) - PLID 62571 - Do the update to the bill.
		ExecuteParamSql(
			"UPDATE BillsT \r\n"
			"SET BillsT.StatusID = {VT_I4}, BillsT.StatusModifiedDate = GETDATE(), BillsT.Description = {STRING} \r\n"
			"FROM BillsT \r\n"
			"WHERE BillsT.ID = {INT} \r\n"
			, varNewStatusID, strNewBillDescription, nBillID);

		// (r.gonet 07/07/2014) - PLID 62571 - Audit
		AuditEvent(nPatientID, strPatientFullName, nAuditTransactionID, aeiBillStatusChanged, nBillID, strOldStatusName, strNewStatusName, aepMedium, aetChanged);
		AuditEvent(nPatientID, strPatientFullName, nAuditTransactionID, aeiBillDescription, nBillID, strOldBillDescription, strNewBillDescription, aepLow, aetChanged);
		CommitAuditTransaction(nAuditTransactionID);

		return true;
	} NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return false;
}

//TES 9/24/2014 - PLID 62782 - Attempts to update the given bill to the given status.
//TES 9/30/2014 - PLID 62782 - This now sets the bill to the given status, unless it's On Hold, all other statuses will be overwritten by this function.
// (s.tullis 2014-10-02 12:16) - PLID 62780 - Call the overloaded Fuction
bool TrySetAlbertaStatus(long nBillID, AlbertaBillingStatus eNewStatus)
{
	MFCArray<long> arrBillID;
	arrBillID.Add(nBillID);

	return TrySetAlbertaStatus(arrBillID, eNewStatus);
}

// (s.tullis 2014-10-01 11:13) - PLID 62780 - Overloaded to set an array of Bills to a new status
bool TrySetAlbertaStatus(MFCArray<long> arrBillID, AlbertaBillingStatus eNewStatus)
{
	long nAuditTransactionID = -1;
	try {
		

		nAuditTransactionID = BeginAuditTransaction();

		if (arrBillID.IsEmpty()) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}

		//TES 9/24/2014 - PLID 62782 - Get the existing status, and whether it's On Hold
		MFCArray<long> arrOldStatusID;
		_variant_t varNewStatusID;
		MFCArray<CString> arrOldStatusName, arrNewStatusName;
		MFCArray< EBillStatusType> arreOldStatusType;
		MFCArray< BOOL> arrbOldStatusIsCustom;
		MFCArray<long> arrPatientID;
		MFCArray<CString> arrPatientName;
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT BillStatusT.ID AS StatusID, BillStatusT.Name AS StatusName, BillStatusT.Type AS StatusType, BillStatusT.Custom,  BillsT.PatientID, PersonT.FullName, BillsT.ID AS BillID \r\n"
			"FROM BillsT \r\n"
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID \r\n"
			"LEFT JOIN PersonT ON PersonT.ID = BillsT.PatientID \r\n"
			"WHERE BillsT.ID IN ({INTVECTOR}); \r\n"
			"\r\n"
			, arrBillID);
		if (prs->eof) {
			// Even deleted bills would be returned here... so this is exceptional.
			ThrowNxException("%s : Could not find bill ID(s)  ", __FUNCTION__);

		}
		// need to make sure these all stay in order
		arrBillID.RemoveAll();
		while (!prs->eof){
			arrBillID.Add(AdoFldLong(prs->Fields, "BillID", -1));
			arrPatientName.Add(AdoFldString(prs->Fields, "FullName", ""));
			arrPatientID.Add(AdoFldLong(prs->Fields, "PatientID", -1));
			arrOldStatusID.Add( AdoFldLong(prs->Fields, "StatusID", -1));
			arrOldStatusName.Add( AdoFldString(prs->Fields, "StatusName", "<No Status>"));
			arreOldStatusType.Add( (EBillStatusType)AdoFldLong(prs->Fields, "StatusType", (long)EBillStatusType::None));
			arrbOldStatusIsCustom.Add( AdoFldBool(prs->Fields, "Custom", FALSE));
			prs->MoveNext();
		}

		prs->Close();

		bool bChangeStatus = true;
		long nNewStatusID = -1;
		long nPostedID = GetAlbertaBillStatusID(ePosted);
		long nPartiallyPaidID = GetAlbertaBillStatusID(ePartiallyPaid);
		long nRejectedID = GetAlbertaBillStatusID(eRejected);
		long nPendingID = GetAlbertaBillStatusID(ePending);
		
		for (int i = 0; i <arreOldStatusType.GetSize(); i++) {
			bChangeStatus = true;
			if (arreOldStatusType[i] == EBillStatusType::OnHold) {
				bChangeStatus = false;
			}
			else {
				switch (eNewStatus) {
				case ePosted:
					nNewStatusID = nPostedID;
					break;
				case ePartiallyPaid:
					nNewStatusID = nPartiallyPaidID;
					break;
				case eRejected:
					nNewStatusID = nRejectedID;
					break;
				case ePending:
					nNewStatusID = nPendingID;
					break;
				default:
					ThrowNxException("Unexpected Alberta Billing Status %li passed to TrySetAlbertaStatus()", eNewStatus);
					break;
				}
			}

			if (!bChangeStatus || nNewStatusID == arrOldStatusID[i]) {
				arrPatientName.RemoveAt(i);
				arrPatientID.RemoveAt(i);
				arrBillID.RemoveAt(i);
				arrOldStatusID.RemoveAt(i);
				arrOldStatusName.RemoveAt(i);
				arreOldStatusType.RemoveAt(i);
				arrbOldStatusIsCustom.RemoveAt(i);
				i--;// since we called remove we need to shift the index down
			}
		}

		if (nNewStatusID==-1) {
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return false;
		}
		
		CString strNewStatusName = GetAlbertaBillStatusName(eNewStatus);
		
		ExecuteParamSql(
			"UPDATE BillsT \r\n"
			"SET BillsT.StatusID = {INT}, BillsT.StatusModifiedDate = GETDATE() \r\n"
			"FROM BillsT \r\n"
			"WHERE BillsT.ID IN ({INTVECTOR}) \r\n"
			, nNewStatusID, arrBillID);

		long loopUpperBound = arrBillID.GetSize();
		// these should all be the same size if they're not we'll have issues
		ASSERT((arrBillID.GetSize() + arrOldStatusName.GetSize()+ arrPatientID.GetSize()+arrPatientName.GetSize()) % 4 == 0 );
			for (int i = 0; i < loopUpperBound; i++){
			
			AuditEvent(arrPatientID[i], arrPatientName[i], nAuditTransactionID, aeiBillStatusChanged, arrBillID[i], arrOldStatusName[i], strNewStatusName, aepMedium, aetChanged);
		}
		CommitAuditTransaction(nAuditTransactionID);

		return true;
	} NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
		RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return false;
}

//TES 10/2/2014 - PLID 63821 - Sets the given text as the "Status Note" on the given bill, with auditing
void SetBillStatusNote(long nBillID, long nPatientID, CString strNote)
{
	//TES 10/2/2014 - PLID 63821 - Get the existing note
	CString strOldNote;
	_RecordsetPtr rsOldNote = CreateParamRecordset("SELECT NoteDataT.Note FROM BillsT INNER JOIN NoteDataT ON BillsT.StatusNoteID = NoteDataT.ID "
		"WHERE BillsT.ID = {INT}", nBillID);
	if (!rsOldNote->eof) {
		strOldNote = AdoFldString(rsOldNote, "Note");
	}
	if (!strNote.IsEmpty()) {
		//TES 10/2/2014 - PLID 63821 - Add the note to data
		CString strNoteWithUsername = FormatString("%s (%s)", strNote, GetCurrentUserName());
		CParamSqlBatch sqlBatch;
		sqlBatch.Declare(
			"DECLARE @nNewID INT; "
			"SET NOCOUNT ON ");
		sqlBatch.Add(
			"INSERT INTO Notes (PersonID, Date, UserID, Note) "
			"VALUES ({INT}, GETDATE(), {INT}, {STRING})",
			nPatientID, GetCurrentUserID(), strNoteWithUsername);
		sqlBatch.Add(
			"SET @nNewID = SCOPE_IDENTITY()");
		sqlBatch.Add(
			"INSERT INTO NoteInfoT (NoteID, BillID, ShowOnStatement, SendOnClaim, IsBillStatusNote) "
			"VALUES (@nNewID, {INT}, 0, 0, 1) ",
			nBillID);
		sqlBatch.Add("UPDATE BillsT SET StatusNoteID = @nNewID WHERE ID = {INT}", nBillID);
		sqlBatch.Declare(
			"SET NOCOUNT OFF "
			"SELECT @nNewID AS NoteID");

		_RecordsetPtr rsID = sqlBatch.CreateRecordset(GetRemoteData());
		//TES 10/2/2014 - PLID 63821 - Need to know what the ID of the note is in order to audit.
		long nNewBillStatusNoteID = AdoFldLong(rsID->Fields, "NoteID");

		long AuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiPatientNote, nNewBillStatusNoteID, "", strNoteWithUsername, aepMedium, aetCreated);
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiBillStatusNoteChanged, nBillID, strOldNote, strNoteWithUsername, aepMedium);
		CommitAuditTransaction(AuditID);
	}
	else {
		//TES 10/2/2014 - PLID 63821 - The note was blank, so remove the bill's status note
		ExecuteParamSql("UPDATE BillsT SET StatusNoteID = NULL WHERE BillsT.ID = {INT}", nBillID);
		long AuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), AuditID, aeiBillStatusNoteChanged, nBillID, strOldNote, "", aepMedium);
		CommitAuditTransaction(AuditID);
	}
}

// (j.jones 2015-02-04 13:26) - PLID 64800 - repairs missing ChargeRespDetailT records
void EnsureChargeRespDetailRecord(long nChargeRespID)
{
	if (nChargeRespID == -1) {
		//why is this function being called on a -1 resp ID?
		ASSERT(FALSE);
		return;
	}

	//1 means to use the charge input date, 0 means to use the bill service date
	CSqlFragment sqlAssignmentDate("BillsT.Date");
	if (GetRemotePropertyInt("DefaultAssignmentToInputDate", 0, 0, "<None>", true)) {
		//use the charge input date
		sqlAssignmentDate = CSqlFragment("LineItemT.InputDate");
	}

	//if the charge resp does not have a ChargeRespDetailT record, make one now
	_RecordsetPtr rs = CreateParamRecordset(
		"SET NOCOUNT ON \r\n"
		"DECLARE @Status INT \r\n"	//status of 0 is no problem found, 1 is found and fixed
		"SET @Status = NULL \r\n"	//status of null means something went wrong
		"DECLARE @ChargeRespID INT \r\n"
		"SET @ChargeRespID = {INT} \r\n"
		"DECLARE @Amount MONEY \r\n"	//track the amount of the charge resp
		"SET @Amount = (SELECT ChargeRespT.Amount FROM ChargeRespT \r\n"
		"		LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID \r\n"
		"		WHERE ChargeRespT.ID = @ChargeRespID AND ChargeRespDetailT.ID Is Null) \r\n"
		"\r\n"
		"IF (@Amount Is Not Null) \r\n"
		//if found, fix it
		"BEGIN \r\n"
			//the insert checks for the bad data a second time to make absolutely sure
			//we only insert when the record is truly missing
		"	INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) \r\n"
		"	SELECT ChargeRespT.ID, ChargeRespT.Amount, dbo.AsDateNoTime({SQL}) \r\n"
		"	FROM ChargeRespT \r\n"
		"	INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID \r\n"
		"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n"
		"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID \r\n"
		"	LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID \r\n"
		"	WHERE ChargeRespT.ID = @ChargeRespID \r\n"
		"	AND ChargeRespDetailT.ID Is Null \r\n"
		"\r\n"
		//set the status to fixed
		"	SET @Status = 1 \r\n"
		"END \r\n"
		"ELSE BEGIN \r\n"
		//set the status to OK
		"	SET @Status = 0 \r\n"
		"END \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT @Status AS Status, @Amount AS Amount", nChargeRespID, sqlAssignmentDate);

	if (!rs->eof) {
		_variant_t varStatus = rs->Fields->Item["Status"]->Value;
		if (varStatus.vt != VT_I4) {
			//should be impossible
			ASSERT(FALSE);
			ThrowNxException("EnsureChargeRespDetailRecord returned an invalid status!");
		}
		else {
			long nStatus = VarLong(varStatus);
			//A status of 1 means we found and fixed data, though it is not bad data if the amount was $0.00.
			if (nStatus == 1) {
				//Was this a non-zero dollar amount?
				//Creating zero-dollar amount resps is perfectly fine.
				//Creating non-zero resps means there is a serious bug somewhere.
				COleCurrency cyAmount = VarCurrency(rs->Fields->Item["Amount"]->Value, COleCurrency(0, 0));
				if (cyAmount != COleCurrency(0, 0)) {
					//We just fixed bad data.
					//If you applied to a recently created bill, this means there is
					//a current bug in the system that is causing ChargeRespDetailT
					//records to either be deleted, or not be created.
					ASSERT(FALSE);
				}
			}
		}
	}
	else {
		//should be impossible
		ASSERT(FALSE);
		ThrowNxException("EnsureChargeRespDetailRecord returned eof!");
	}
	rs->Close();
}


// (j.jones 2015-03-20 14:12) - PLID 65402 - silently returns true
// if the LineItemT record is closed
bool IsLineItemClosed(long nLineItemID)
{
	//throws exceptions to the caller

	_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.ID "
		"FROM LineItemT "
		"CROSS JOIN (SELECT Max(CloseDate) AS HardCloseAsOfDate, "
		"	Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT) AS FinancialCloseHistoryQ "
		"WHERE LineItemT.ID = {INT} "
		"AND Convert(bit, CASE WHEN FinancialCloseHistoryQ.HardCloseAsOfDate Is Null THEN 0 "
		"	WHEN LineItemT.InputDate <= FinancialCloseHistoryQ.HardCloseAsOfDate "
		"	OR (LineItemT.InputDate <= FinancialCloseHistoryQ.HardCloseInputDate AND LineItemT.Date <= FinancialCloseHistoryQ.HardCloseAsOfDate) " 
		"THEN 1 ELSE 0 END) = 1",
		nLineItemID);

	if (rs->eof) {
		//it is not closed
		return false;
	}
	else {
		//the apply is closed
		return true;
	}
}

// (j.jones 2015-03-20 14:12) - PLID 65402 - silently returns true
// if the AppliesT record is closed.
// An apply is closed if its source payment is closed.
bool IsApplyClosed(long nApplyID)
{
	//throws exceptions to the caller

	_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.ID "
		"FROM AppliesT "
		"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"CROSS JOIN (SELECT Max(CloseDate) AS HardCloseAsOfDate, "
		"	Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT) AS FinancialCloseHistoryQ "
		"WHERE AppliesT.ID = {INT} "
		"AND Convert(bit, CASE WHEN FinancialCloseHistoryQ.HardCloseAsOfDate Is Null THEN 0 "
		"	WHEN LineItemT.InputDate <= FinancialCloseHistoryQ.HardCloseAsOfDate "
		"	OR (LineItemT.InputDate <= FinancialCloseHistoryQ.HardCloseInputDate AND LineItemT.Date <= FinancialCloseHistoryQ.HardCloseAsOfDate) " 
		"THEN 1 ELSE 0 END) = 1",
		nApplyID);

	if (rs->eof) {
		//it is not closed
		return false;
	}
	else {
		//the apply is closed
		return true;
	}
}

// (j.jones 2015-03-20 15:33) - PLID 65402 - For when the user is about to create a refund
// for an applied payment, check the preference to automatically void & correct the payment
// and if it's enabled, do so.
// This will also check Unapply permissions to determine if the apply can proceed.
// If a Void & Correct happens, nPaymentID and nApplyID will be updated with the newest IDs.
// Returns true if the refund can continue being applied, false to abort the refund.
// nSourceRefundID isn't actually used for anything but warnings.
// (r.goldschmidt 2016-03-02 16:09) - PLID 68447 - make preference global; change default
bool RefundingAppliedPayment_CheckVoidAndCorrect(IN OUT long &nPaymentID, IN OUT long &nApplyID, OPTIONAL long nRefundID /*= -1*/)
{
	try {

		long nAutoVoidAndCorrect = GetRemotePropertyInt("RefundingAPayment_VoidAndCorrect_Global", 1, 0, "<None>", true);

		bool bAutoCorrect = false;

		//never automatically correct
		if (nAutoVoidAndCorrect == 0) {
			//carry on
			bAutoCorrect = false;
		}
		//the other options correct based on whether or not it is closed
		else {
			//automatically correct if the payment is closed (1) or always (2)
			if (nAutoVoidAndCorrect == 1) {
				//correct only if the apply is closed
				bAutoCorrect = IsApplyClosed(nApplyID);
			}
			else if (nAutoVoidAndCorrect == 2) {
				//always correct
				bAutoCorrect = true;
			}
			else {
				//somebody changed the preference without updating this code!
				ASSERT(FALSE);
				ThrowNxException("RefundingAPayment_VoidAndCorrect_Global preference has an unsupported value (%li)!", nAutoVoidAndCorrect);
			}

			//we're set to auto-correct, do they want a warning first?
			if (bAutoCorrect) {
				bool bWarnBeforeVoiding = (GetRemotePropertyInt("RefundingAPayment_VoidAndCorrect_Warn_Global", 1, 0, "<None>", true) == 1);
				if (bWarnBeforeVoiding) {
					int nRet = MessageBox(GetActiveWindow(),
						FormatString("Would you like to Void & Correct the payment before %s the refund?", nRefundID == -1 ? "creating" : "applying"),
						"Practice", MB_ICONQUESTION | MB_YESNOCANCEL);
					if (nRet == IDCANCEL) {
						return false;
					}
					else if (nRet == IDNO) {
						bAutoCorrect = false;
					}
				}
			}
		}

		//check permissions, but ignore the close if we are going to auto-correct
		if (!CanChangeHistoricFinancial("Apply", nApplyID, bioApplies, sptDelete, FALSE, NULL, bAutoCorrect ? TRUE : FALSE)) {
			return false;
		}

		if (bAutoCorrect) {
			//if we get here, we need to automatically correct the apply record
			CFinancialCorrection finCor;

			CString strUsername = GetCurrentUserName();
			long nCurrUserID = GetCurrentUserID();

			finCor.AddCorrection(ctPayment, nPaymentID, strUsername, nCurrUserID);
			finCor.ExecuteCorrections();

			//get the new apply ID from the correction
			ApplyIDInfo eInfo = FindNewestCorrectedApply(nApplyID);
			nApplyID = eInfo.nID;
			nPaymentID = eInfo.nSourceID;
		}

		//regardless of whether a correction occurred,
		//if we get here the refund can continue
		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2015-03-02 14:30) - PLID 64963 - standardized saving service categories via the API
// aryServiceIDs cannot be empty, aryCategoryIDs *can* be empty
// nDefaultCategoryID is -1 if there is no default, it cannot otherwise be an ID that is not in aryCategoryIDs
// bIsNewItem is true if you just created this item, false if it is an existing item.
// If bIsNewItem is false, the API will audit the category changes and send tablecheckers.
void UpdateServiceCategories(IN std::vector<long> &aryServiceIDs, IN std::vector<long> &aryCategoryIDs, IN long nDefaultCategoryID, bool bIsNewItem)
{
	//throw exceptions to the caller

	if (aryServiceIDs.size() == 0) {
		//why was this function called?
		ThrowNxException("UpdateServiceCategories called with no service IDs!");
	}

	Nx::SafeArray<BSTR> saryServiceIDs;
	for each (long nServiceID in aryServiceIDs)
	{
		saryServiceIDs.Add(_bstr_t(AsString(nServiceID)));
	}

	Nx::SafeArray<BSTR> saryCategoryIDs;
	bool bDefaultCategoryFound = false;
	for each (long nCategoryID in aryCategoryIDs)
	{
		saryCategoryIDs.Add(_bstr_t(AsString(nCategoryID)));

		//we need to validate if the default category (if provided)
		//is in our category list
		if (nDefaultCategoryID == nCategoryID) {
			bDefaultCategoryFound = true;
		}
	}

	if (nDefaultCategoryID != -1 && !bDefaultCategoryFound) {
		ThrowNxException("UpdateServiceCategories called with a default category that is not in the category list!");
	}

	if (nDefaultCategoryID != -1 && aryCategoryIDs.size() == 0) {
		//the calling code did something wrong if the default
		//category is set but the category list is not filled
		ThrowNxException("UpdateServiceCategories called with a default category, but no category list!");
	}

	NexTech_Accessor::_GetServiceCategoriesInputPtr updateInfo(__uuidof(NexTech_Accessor::GetServiceCategoriesInput));
	updateInfo->ServiceIDs = saryServiceIDs;
	updateInfo->CategoryIDs = saryCategoryIDs;
	if (nDefaultCategoryID != -1) {
		updateInfo->DefaultCategoryID = _bstr_t(AsString(nDefaultCategoryID));
	}
	updateInfo->IsNewItem = bIsNewItem;

	GetAPI()->UpdateServiceCategories(GetAPISubkey(), GetAPILoginToken(), updateInfo);
}

// (j.jones 2015-03-02 14:15) - PLID 64963 - standardized loading categories and descriptions
// for services/products that have multiple categories
void LoadServiceCategories(IN long nServiceID, OUT std::vector<long> &aryCategoryIDs, OUT CString &strCategoryNames, OUT long &nDefaultCategoryID)
{
	//throw exceptions to the caller

	aryCategoryIDs.clear();
	strCategoryNames = "";
	nDefaultCategoryID = -1;

	if (nServiceID == -1) {
		//why was this function called?
		ThrowNxException("LoadServiceCategories called with no service ID!");
	}

	// (j.jones 2015-03-10 11:25) - if this logic changes, be sure to update dbo.GetServiceCategoryNames()
	_RecordsetPtr rs = CreateParamRecordset("SELECT CategoriesT.ID, CategoriesT.Name, ServiceT.Category "
		"FROM CategoriesT "
		"INNER JOIN ServiceMultiCategoryT ON CategoriesT.ID = ServiceMultiCategoryT.CategoryID "
		"INNER JOIN ServiceT ON ServiceMultiCategoryT.ServiceID = ServiceT.ID "
		"WHERE ServiceT.ID = {INT} "
		"ORDER BY (CASE WHEN ServiceT.Category = CategoriesT.ID THEN 0 ELSE 1 END) ASC, CategoriesT.Name ASC", nServiceID);
	bool bAppendedDefaultSuffix = false;
	while (!rs->eof) {

		//track the default 
		nDefaultCategoryID = AdoFldLong(rs, "Category", -1);

		if (!strCategoryNames.IsEmpty()) {

			//we append (Default) only when multiple categories exist
			if (nDefaultCategoryID != -1 && !bAppendedDefaultSuffix) {

				strCategoryNames += " (Default)";
				bAppendedDefaultSuffix = true;

				//at this point there should only be one tracked category ID so far,
				//and it should be the default category
				ASSERT(aryCategoryIDs.size() == 1 && nDefaultCategoryID == aryCategoryIDs[0]);
			}

			strCategoryNames += ", ";
		}

		aryCategoryIDs.push_back(AdoFldLong(rs, "ID"));
		strCategoryNames += AdoFldString(rs, "Name");

		rs->MoveNext();
	}
	rs->Close();
}

// (j.jones 2015-03-03 13:43) - PLID 64965 - this version queries only based on the category IDs
void LoadServiceCategories(IN std::vector<long> &aryCategoryIDs, IN long nDefaultCategoryID, OUT CString &strCategoryNames)
{
	//throw exceptions to the caller

	strCategoryNames = "";

	if (aryCategoryIDs.size() == 0) {
		//leave the category names blank
		return;
	}

	// (j.jones 2015-03-10 11:25) - if this logic changes, be sure to update dbo.GetServiceCategoryNames()
	_RecordsetPtr rs = CreateParamRecordset("SELECT CategoriesT.ID, CategoriesT.Name "
		"FROM CategoriesT "
		"WHERE ID IN ({INTVECTOR}) "
		"ORDER BY (CASE WHEN ID = {INT} THEN 0 ELSE 1 END) ASC, CategoriesT.Name ASC", aryCategoryIDs, nDefaultCategoryID);
	bool bAppendedDefaultSuffix = false;
	while (!rs->eof) {

		if (!strCategoryNames.IsEmpty()) {

			//we append (Default) only when multiple categories exist
			if (nDefaultCategoryID != -1 && !bAppendedDefaultSuffix) {

				strCategoryNames += " (Default)";
				bAppendedDefaultSuffix = true;
			}

			strCategoryNames += ", ";
		}

		strCategoryNames += AdoFldString(rs, "Name");

		rs->MoveNext();
	}
	rs->Close();}

// (r.gonet 2015-04-20) - PLID 65326 - Returns true if nPayMethod is a valid pay method value. false otherwise.
bool IsValidPayMethod(long nPayMethod)
{
	static const std::vector<EPayMethod> s_sortedPayMethods = {
		EPayMethod::Invalid,
		EPayMethod::Adjustment,
		EPayMethod::CashPayment, EPayMethod::CheckPayment, EPayMethod::ChargePayment, EPayMethod::GiftCertificatePayment,
		EPayMethod::LegacyAdjustment,
		EPayMethod::CashRefund, EPayMethod::CheckRefund, EPayMethod::ChargeRefund, EPayMethod::GiftCertificateRefund
	};

	for (unsigned int i = 0; i < s_sortedPayMethods.size(); i++) {
		if ((long)s_sortedPayMethods[i] == nPayMethod) {
			return true;
		}
	}
	if (nPayMethod >(long)s_sortedPayMethods[s_sortedPayMethods.size() - 1]) {
		// Unhandled pay method. Maybe somebody didn't know to add it to s_allPayMethods?
		ASSERT(FALSE);
	}
	return false;
}

// (r.gonet 2015-04-20) - PLID 65326 - Returns the nPayMethod casted as an EPayMethod or returns a default if the nPayMethod is an invalid pay method.
// Note that this changes the legacy adjustments into adjustments behind the scenes.
EPayMethod AsPayMethod(long nPayMethod, EPayMethod eDefaultValue/*=EPayMethod::Invalid*/)
{
	if (!IsValidPayMethod(nPayMethod)) {
		return eDefaultValue;
	}
	EPayMethod ePayMethod = (EPayMethod)nPayMethod;
	if (ePayMethod == EPayMethod::LegacyAdjustment) {
		ePayMethod = EPayMethod::Adjustment;
	}
	return ePayMethod;
}

// (r.gonet 2015-04-20) - PLID 65326 - Gets whether a paymethod is a Payment, an Adjustment, or a Refund. Returns LineItem::Type::Invalid if the paymethod is not not handled.
LineItem::Type GetLineItemTypeFromPayMethod(EPayMethod e)
{
	if (e == EPayMethod::CashPayment || e == EPayMethod::CheckPayment || e == EPayMethod::ChargePayment || e == EPayMethod::GiftCertificatePayment) {
		return LineItem::Type::Payment;
	} else if (e == EPayMethod::LegacyAdjustment || e == EPayMethod::Adjustment) {
		return LineItem::Type::Adjustment;
	} else if (e == EPayMethod::CashRefund || e == EPayMethod::CheckRefund || e == EPayMethod::ChargeRefund || e == EPayMethod::GiftCertificateRefund) {
		return LineItem::Type::Refund;
	} else {
		return LineItem::Type::Invalid;
	}}


// (j.jones 2015-04-23 13:59) - PLID 65711 - main function to transfer gift certificate balances
void TransferGiftCertificateAmount(long nSourceGiftID, long nDestGiftID, COleCurrency cyAmtToTransfer)
{
	//throw exceptions to the caller

	if (nSourceGiftID == -1 || nDestGiftID == -1) {
		//no caller should have ever allowed this
		ASSERT(FALSE);
		ThrowNxException("Invalid gift certificate IDs provided for transfer.");
	}

	if (nSourceGiftID == nDestGiftID) {
		//no caller should have ever allowed this
		ASSERT(FALSE);
		ThrowNxException("Cannot transfer a gift certificate amount to itself!");
	}

	double dblAmount = (double)((CURRENCY)cyAmtToTransfer).int64 / 10000.0;

	NexTech_Accessor::_TransferGiftCertificateAmountInputPtr transferInfo(__uuidof(NexTech_Accessor::TransferGiftCertificateAmountInput));
	transferInfo->SourceGiftCertificateID = _bstr_t(AsString(nSourceGiftID));
	transferInfo->DestGiftCertificateID = _bstr_t(AsString(nDestGiftID));
	transferInfo->AmountToTransfer = (float)dblAmount;

	GetAPI()->TransferGiftCertificateAmount(GetAPISubkey(), GetAPILoginToken(), transferInfo);
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Added a function create a new gift certificate with prefilled fields.
// (j.jones 2015-05-11 16:57) - PLID 65714 - added GCCreationStyle
bool CreateNewGiftCertificate(CWnd *pParentWnd, GCCreationStyle eCreationStyle, OUT long &nNewGiftID, OUT CString &strNewCertNumber,
	OPTIONAL CString strCertNumber/* = ""*/, OPTIONAL long nTypeID/* = -1*/,
	OPTIONAL COleCurrency cyValue/* = g_ccyNull*/, OPTIONAL COleCurrency cyDisplayedValue/* = g_ccyNull*/, OPTIONAL COleCurrency cyPrice/* = g_ccyNull*/,
	OPTIONAL long nProviderID/* = -1*/, OPTIONAL long nLocationID/* = -1*/,
	OPTIONAL COleDateTime dtPurchaseDate/* = g_cdtNull*/, OPTIONAL bool bPresetExpirationDate/* = false*/, OPTIONAL COleDateTime dtExpirationDate/* = g_cdtNull*/,
	OPTIONAL long nPurchasedByPatientID/* = -1*/, OPTIONAL bool bPresetReceivedByPatientID/* = false*/, OPTIONAL long nReceivedByPatientID/* = -1*/)
{
	CGCEntryDlg dlg(pParentWnd, eCreationStyle);
	if (strCertNumber != "") {
		dlg.SetCertNumber(strCertNumber);
	}
	if (nTypeID != -1) {
		dlg.SetService(nTypeID);
	}
	if (cyValue != g_ccyNull) {
		dlg.SetValue(cyValue);
	}
	if (cyDisplayedValue != g_ccyNull) {
		dlg.SetDisplayedValue(cyDisplayedValue);
	}
	if (cyPrice != g_ccyNull) {
		dlg.SetPrice(cyPrice);
	}
	if (nLocationID != -1) {
		dlg.SetLocation(nLocationID);
	}
	if (nProviderID != -1) {
		dlg.SetProvider(nProviderID);
	}
	if (dtPurchaseDate != g_cdtNull) {
		dlg.SetPurchaseDate(dtPurchaseDate);
	}
	if (bPresetExpirationDate) {
		dlg.SetExpirationDate(dtExpirationDate);
	}
	if (nPurchasedByPatientID != -1) {
		dlg.SetPurchasedByPatientID(nPurchasedByPatientID);
	}
	if (bPresetReceivedByPatientID) {
		dlg.SetReceivedByPatientID(nReceivedByPatientID);
	}
	if (dlg.DoModal() == IDOK) {
		nNewGiftID = dlg.m_nID;
		strNewCertNumber = dlg.m_strCertNumber;
		return true;
	} else {
		nNewGiftID = -1;
		strNewCertNumber = "";
		return false;
	}
}

// (z.manning 2015-07-22 10:30) - PLID 67241
void OpenPaymentProfileDlg(const long nPatientPersonID, CWnd *pwndParent)
{
	CICCPPaymentProfileDlg dlg(nPatientPersonID, pwndParent);
	dlg.DoModal();
}

// (z.manning 2015-07-23 09:35) - PLID 67241 - Checks the license and config property to see if ICCP is enabled
BOOL IsICCPEnabled()
{
	return IsICCPEnabled(FALSE);
}


// (z.manning 2015-09-04 09:09) - PLID 67236 - Added overload
BOOL IsICCPEnabled(BOOL bIgnorePreference)
{
	if (g_pLicense != NULL && g_pLicense->CheckForLicense(CLicense::lcICCP, CLicense::cflrSilent))
	{
		if (bIgnorePreference || GetRemotePropertyInt("ICCPEnabled", 0, 0) != 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//(j.camacho 2016-02-01) plid 68010
BOOL IsIntellechartToBeBilledEnabled()
{
	if (ReturnsRecordsParam("SELECT TOP 1 HL7GroupID FROM HL7GenericSettingsT WHERE Name = 'EnableIntelleChart' AND BitParam = 1 "))
	{
		return TRUE;
	}
	else {
		return FALSE;
	}

}

// (j.jones 2015-09-30 10:34) - PLID 67171 - added a global function to see if a payment
// was processed under non-ICCP processing (Chase or Intuit)
bool IsCCPaymentProcessedPreICCP(long nPaymentID)
{
	//this function doesn't care if the process was approved
	if (ReturnsRecordsParam("SELECT TOP 1 ID FROM QBMS_CreditTransactionsT WHERE ID = {INT} "
		"UNION SELECT TOP 1 ID FROM Chase_CreditTransactionsT WHERE ID = {INT}", nPaymentID, nPaymentID)) {

		//the payment ID was indeed processed by Chase or Intuit
		return true;
	}
	
	return false;
}

// (c.haag 2015-08-24) - PLID 67198 - Returns true if a payment was processed under ANY credit card processing
bool IsCCPaymentProcessed(long nPaymentID)
{
	return (VARIANT_FALSE == GetAPI()->IsCreditCardPaymentProcessed(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nPaymentID))) ? false : true;
}

// (c.haag 2015-08-24) - PLID 67198 - Returns true if the patient has at least one payment associated with any kind of credit card transaction; approved or otherwise
bool DoesPatientHaveCreditCardTransactions(long nUserDefinedID)
{
	return (VARIANT_FALSE == GetAPI()->DoesPatientHaveCreditCardTransactions(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nUserDefinedID))) ? false : true;
}

// (c.haag 2015-08-24) - PLID 67198 - Returns true if the user is associated with any kind of credit card transaction; approved or otherwise
bool DoesUserHaveCreditCardTransactions(long nUserID)
{
	return (VARIANT_FALSE == GetAPI()->DoesUserHaveCreditCardTransactions(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nUserID))) ? false : true;
}

// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
CString LeftJustify(CString strText, long nLineWidth) {

	//this is really just printing out the line regularly, but I put it in a function in case we 
	//do something with it in the future
	CString strDesc;

	strDesc = strText;

	long nLineLength = nLineWidth - strText.GetLength();
	/*for (int i = 0; i < nLineLength; i++) {
	strDesc += " ";
	}*/
	return strDesc + "\n";

}

// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
CString LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth) {

	CString strDesc;

	long nSpaceLength = nLineWidth - strTextLeft.GetLength() - strTextRight.GetLength();
	strDesc = strTextLeft;
	for (int i = 0; i < nSpaceLength - 1; i++) {
		strDesc += " ";
	}

	strDesc += strTextRight;

	return strDesc + "\n";
}

// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
CString CenterLine(CString strText, long nLineWidth) {

	CString strDesc;

	long nCenterLine = (nLineWidth) / 2;
	long nCenterName = strText.GetLength() / 2;
	long nDiff = nCenterLine - nCenterName;
	for (int i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strDesc += strText;
	/*for (i = 0; i < nDiff; i++) {
	strDesc += " ";
	}*/

	return strDesc + "\n";
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
// (d.thompson 2011-01-05) - PLID 42005 - Reversed some extremely bizarre behavior w/ nLocationID.  It is now a 
//	pre-filled parameter, not calculated by this function.
// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
BOOL PrintCreditReceiptHeader(COPOSPrinterDevice *pPOSPrinter, long nLocationID)
{
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();
	CString strLine;

	//get the location information
	// (z.manning 2015-09-08 12:03) - PLID 67224 - Parameterized
	_RecordsetPtr rsLocation = CreateParamRecordset(
		"SELECT Name, Address1, Address2, City, State, Zip, Phone \r\n"
		"FROM LocationsT \r\n"
		"WHERE ID = {INT} \r\n"
		, nLocationID);

	if (rsLocation->eof) {
		ThrowNxException("Could not obtain location information");
		return FALSE;
	}

	CString strName, strAdd1, strAdd2, strCity, strState, strZip, strPhone;

	strName = AdoFldString(rsLocation, "Name");
	strAdd1 = AdoFldString(rsLocation, "Address1", "");
	strAdd2 = AdoFldString(rsLocation, "Address2", "");
	strCity = AdoFldString(rsLocation, "City", "");
	strState = AdoFldString(rsLocation, "State", "");
	strZip = AdoFldString(rsLocation, "Zip", "");
	strPhone = AdoFldString(rsLocation, "Phone", "");

	strOutput += CenterLine(strName, nLineWidth);
	strOutput += CenterLine(strAdd1, nLineWidth);

	if (!strAdd2.IsEmpty()) {
		strOutput += CenterLine(strAdd2, nLineWidth);
	}

	CString strCSZ = strCity + ", " + strState + " " + strZip;
	strOutput += CenterLine(strCSZ, nLineWidth);

	strOutput += CenterLine(strPhone, nLineWidth);

	//output them in bold, centered
	CString strTmp = char(27);
	strOutput = "\x1b|bC" + strOutput + "\x1b|N";

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
BOOL PrintCreditReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, BOOL bIsRefund, const CString &strCardName
	, const CString &strCardNumberToDisplay, BOOL *pbSwipe)
{
	//now put the input name
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();

	CString strSaleType = (bIsRefund ? "Refund" : "Sale");

	//the user who processed it
	strOutput += LeftRightJustify(GetCurrentUserName(), strSaleType, nLineWidth);

	//now the date and time
	COleDateTime dtNow;
	dtNow = COleDateTime::GetCurrentTime();
	strOutput += LeftRightJustify(FormatDateTimeForInterface(dtNow, NULL, dtoDate, FALSE),
		FormatDateTimeForInterface(dtNow, NULL, dtoTime, FALSE), nLineWidth);

	//now the card type and number
	strOutput += LeftRightJustify(strCardName, strCardNumberToDisplay, nLineWidth);

	//whether the card was swiped
	if (pbSwipe != NULL)
	{
		if (*pbSwipe) {
			strOutput += LeftJustify("Card Swiped", nLineWidth);
		}
		else {
			//we aren't going to do this just yet because they 
			// (d.thompson 2011-01-05) - PLID 41994 - I'm not sure what the half sentence above meant to say, but we do 
			//	need to print this.  Visa guidelines state that you should always print the swipe / keyed status on the receipt.
			strOutput += LeftJustify("Card Manually Entered", nLineWidth);
		}
	}

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}

}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
// (c.haag 2015-09-15) - PLID 67195 - We now take in a signature
BOOL PrintCreditReceiptFooter(COPOSPrinterDevice *pPOSPrinter, BOOL bMerchantCopy, BOOL bIsRefund
	, const COleCurrency &cyAmount, const CString &strCardHolderName, const CString& strSignatureBMPFileName)
{
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strOutput += "\n\n\n";

	CString strPrint = "Amount:";
	CString strAmount = FormatCurrencyForInterface(cyAmount);
	long nLen = 18 - strAmount.GetLength();
	for (int i = 0; i < nLen; i++) {
		strAmount = " " + strAmount;
	}
	strPrint += strAmount;

	//put the amount
	strOutput += LeftRightJustify("", strPrint, nLineWidth);

	//first give them some lines
	strOutput += "\n\n\n";

	// (j.gruber 2007-08-07 10:37) - PLID 26997 - only show the tip and total lines if they want to
	CString strDesc;
	// (d.thompson 2009-07-06) - PLID 34764 - Slightly reworked this preference, default to off
	if (GetRemotePropertyInt("CCProcessingShowTipLine", 0, 0, "<None>", true)) {
		//now print the tip line
		strDesc = "Tip: __________________";
		//figure out the white space
		long nWhiteLen = nLineWidth - strDesc.GetLength();
		for (i = 0; i < nWhiteLen; i++) {
			strDesc = " " + strDesc;
		}
		strOutput += LeftJustify(strDesc, nLineWidth);


		strOutput += "\n\n";

		//now the Total line
		strDesc = "Total: __________________";
		nWhiteLen = nLineWidth - strDesc.GetLength();
		for (i = 0; i < nWhiteLen; i++) {
			strDesc = " " + strDesc;
		}
		strOutput += LeftJustify(strDesc, nLineWidth);

		strOutput += "\n\n";
	}

	//and the signature line
	// (z.manning 2015-09-09 09:42) - PLID 67224 - Only show on merchant copy
	// (z.manning 2015-09-09 10:15) - PLID 67224 - Don't need a signature on refunds
	if (bMerchantCopy && !bIsRefund)
	{
		// (c.haag 2015-09-15) - PLID 67195 - If we already have a signature then print out whatever is in
		// strOutput, and then print the signature. Otherwise add a signature line to strOutput.
		if (!strSignatureBMPFileName.IsEmpty())
		{
			if (!pPOSPrinter->PrintText(strOutput)) 
			{
				return FALSE;
			}
			if (!pPOSPrinter->PrintBitmap(strSignatureBMPFileName))
			{
				return FALSE;
			}

			strOutput = "";
		}
		else
		{
			strDesc = "x";
			for (i = 0; i < nLineWidth - 1; i++) {
				strDesc += "_";
			}
			strOutput += LeftJustify(strDesc, nLineWidth);
			strOutput += "\n\n";
		}
	}

	//now add the cardholders name
	//let's center it
	strDesc = "";

	long nCenterLine = (nLineWidth - 1) / 2;
	long nCenterName = strCardHolderName.GetLength() / 2;
	long nDiff = nCenterLine - nCenterName;
	for (i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strDesc += strCardHolderName;
	for (i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strOutput += LeftJustify(strDesc, nLineWidth);

	//print merchant copy
	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strOutput += "\n\n\n";
	if (bMerchantCopy) {
		strOutput += CenterLine("Merchant Copy", nLineWidth);
	}
	else {
		strOutput += CenterLine("Customer Copy", nLineWidth);
	}

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (c.haag 2015-09-15) - PLID 67195 - Writes a signature to an empty BMP file
void WriteSignatureToBitmap(const CString& strSignatureBMPFileName, const IStreamPtr &pSignatureImageStream)
{
	IStreamPtr pFileStream;
	HRESULT hr = SHCreateStreamOnFile(strSignatureBMPFileName, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, &pFileStream);
	if (!SUCCEEDED(hr)) {
		ThrowNxException("Unable to open signature BMP file for writing.");
	}

	ULARGE_INTEGER bytesToCopy = { UINT_MAX };
	hr = pSignatureImageStream->CopyTo(pFileStream, bytesToCopy, nullptr, nullptr);
	if (!SUCCEEDED(hr)) {
		ThrowNxException("Unable to write signature BMP file.");
	}
	pFileStream.Release(); // This will flush and close the file
}

// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
// (j.jones 2015-09-30 10:56) - PLID 67180 - added signature bitmap stream, optional
BOOL PrintCreditCardReceipt(CWnd* pwndParent, BOOL bPrintMerchantCopy, BOOL bPrintCustomerCopy, long nPaymentID
	, long nLocationID, BOOL bIsRefund, const COleCurrency &cyAmount, const CString &strCardHolderName
	, const CString &strCardName, const CString &strCardNumberToDisplay, BOOL *pbSwipe, const IStreamPtr &pSignatureImageStream)
{
	std::vector<BOOL> vectorReceiptFlags;
	if (bPrintMerchantCopy) {
		vectorReceiptFlags.push_back(TRUE);
	}
	if (bPrintCustomerCopy) {
		vectorReceiptFlags.push_back(FALSE);
	}

	if (vectorReceiptFlags.size() == 0) {
		return FALSE;
	}

	// (j.gruber 2007-07-30 09:39) - PLID 26720 - see if they have a receipt printer configured
	if (GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE))
	{
		for (BOOL bMerchantCopy : vectorReceiptFlags)
		{
			//first let's acquire the receipt printer
			//TES 12/6/2007 - PLID 28192 - We're ready to start using the POS Printer, so claim it.
			// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
			// not to mention providing exception safety.
			POSPrinterAccess pPOSPrinter;
			if (nullptr != pPOSPrinter) 
			{
				// (c.haag 2015-09-15) - PLID 67195 - We now take in a signature image stream. If it's not null, we will
				// generate a temporary bitmap file here
				FileUtils::CAutoRemoveTempFile* pTempBMPFile = nullptr;
				try
				{
					CString strSignatureBMPFileName;
					if (nullptr != pSignatureImageStream)
					{
						strSignatureBMPFileName = FormatString("%s\\Signature_%li.bmp", GetNxTempPath(), GetTickCount());
						pTempBMPFile = new FileUtils::CAutoRemoveTempFile(strSignatureBMPFileName);
						WriteSignatureToBitmap(strSignatureBMPFileName, pSignatureImageStream);
					}

					//First print the header
					if (!PrintCreditReceiptHeader(pPOSPrinter, nLocationID)) {
						return FALSE;
					}

					if (!PrintCreditReceiptMiddle(pPOSPrinter, bIsRefund, strCardName, strCardNumberToDisplay, pbSwipe)) {
						return FALSE;
					}

					if (!PrintCreditReceiptFooter(pPOSPrinter, bMerchantCopy, bIsRefund, cyAmount, strCardHolderName, strSignatureBMPFileName)) {
						return FALSE;
					}

					//feed some line out
					// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
					pPOSPrinter->PrintText("\n\n\n\n\n\n");

					// (a.walling 2011-04-28 10:02) - PLID 43492
					if (!pPOSPrinter->FlushAndTryCut()) {
						return FALSE;
					}

					if (nullptr != pTempBMPFile)
					{
						delete pTempBMPFile;
					}
				}
				catch (...)
				{
					if (nullptr != pTempBMPFile)
					{
						try
						{
							delete pTempBMPFile;
						}
						catch (...) 
						{
						}
					}
					throw;
				}
			}
			else 
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	else
	{
		CPrintInfo prInfo;
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prInfo.m_bPreview = FALSE;
		prInfo.m_bDirect = FALSE;
		prInfo.m_bDocObject = FALSE;
		if (prInfo.m_pPD) {
			delete prInfo.m_pPD;
		}
		prInfo.m_pPD = dlg;

		BOOL bReturn;
		for (BOOL bMerchantCopy : vectorReceiptFlags)
		{
			//they don't have a receipt printer configured, run the standard one
			// (z.manning 2015-09-09 10:37) - PLID 67224 - Support customer copy here too
			int nReportID = (bMerchantCopy ? 605 : 604);
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

			infReport.nExtraID = nPaymentID;

			//Set up the parameters.
			CPtrArray paParams;
			CRParameterInfo *paramInfo;

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = GetCurrentUserName();
			paramInfo->m_Name = "CurrentUserName";
			paParams.Add(paramInfo);

			paramInfo = new CRParameterInfo;
			if (pbSwipe != NULL)
			{
				if (*pbSwipe) {
					paramInfo->m_Data = "Card Swiped";
				}
				else {
					paramInfo->m_Data = "Card Manually Entered";
				}
			}
			paramInfo->m_Name = "CardSwipeInfo";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			if (bIsRefund) {
				paramInfo->m_Data = "Refund";
			}
			else {
				paramInfo->m_Data = "Sale";
			}
			paramInfo->m_Name = "TransactionType";
			paParams.Add((void *)paramInfo);

			// (d.thompson 2009-07-06) - PLID 34764 - Must follow the tip line preference
			paramInfo = new CRParameterInfo;
			paramInfo->m_Name = "ShowTipLine";
			paramInfo->m_Data = FormatString("%li", GetRemotePropertyInt("CCProcessingShowTipLine", 0, 0, "<None>", true));
			paParams.Add(paramInfo);

			// (j.jones 2015-09-30 10:56) - PLID 67180 - added Signature
			{
				infReport.strExtraText = "NULL";

				if (pSignatureImageStream != NULL) {
					// Seek back to the beginning of the bitmap stream.
					LARGE_INTEGER dlibMove;
					dlibMove.HighPart = dlibMove.LowPart = 0;
					dlibMove.QuadPart = 0;
					pSignatureImageStream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
					// Get the size of the bitmap in the stream.
					STATSTG stats;
					pSignatureImageStream->Stat(&stats, STATFLAG_NONAME);
					// Everywhere that we'll be using size only takes a max of 2^32 bytes,
					// so if the image is > 4 GB then taking just the low part here will fail, 
					// but I'm willing to take that chance.
					DWORD dwSize = stats.cbSize.LowPart;
					BYTE *pImageBytes = NULL;
					COleSafeArray sa;
					// Create a variant array of bytes to store the bitmap's bytes.
					sa.CreateOneDim(VT_UI1, dwSize);
					sa.AccessData((LPVOID*)&pImageBytes);
					// Read the bitmap into our byte array that's attached to our variant array.
					ULONG nBytesRead;
					pSignatureImageStream->Read(pImageBytes, dwSize, &nBytesRead);
					ASSERT(nBytesRead == dwSize);
					sa.UnaccessData();

					//send the image as ExtraText
					infReport.strExtraText = CreateByteStringFromSafeArrayVariant(sa.Detach());
				}
			}

			bReturn = RunReport(&infReport, &paParams, FALSE, pwndParent, 
				bMerchantCopy ? "Credit Card Merchant Copy" : "Credit Card Customer Copy", 
				&prInfo);
			ClearRPIParameterList(&paParams);

			// (z.manning 2015-09-09 13:07) - PLID 67224 - After printing once, set direct to true so we don't
			// keep prompting if they're printing both merchant and customer copies.
			prInfo.m_bDirect = TRUE;

			if (!bReturn) {
				return FALSE;
			}
		}

		return bReturn;
	}
}

// (z.manning 2015-09-11 09:05) - PLID 67224
COleCurrency DoubleToOleCurrency(double dblAmount)
{
	long nAmount = (long)dblAmount;
	COleCurrency cyAmount;
	cyAmount.SetCurrency(nAmount, (long)((dblAmount - (double)nAmount) * 10000));
	return cyAmount;
}

// (z.manning 2015-09-14 14:31) - PLID 67221
double OleCurrencyToDouble(const COleCurrency &cy)
{
	double dbl = (double)((CURRENCY)cy).int64 / 10000.0;
	return dbl;
}

// (z.manning 2015-09-08 15:49) - PLID 67224 - ICCP receipt printing
void PrintICCPReceipts(CWnd *pwndParent, long nPaymentID, BOOL bMerchantCopy, BOOL bCustomerCopy)
{
	if (!bMerchantCopy && !bCustomerCopy) {
		return;
	}

	CWaitCursor wc;

	_ICCPPaymentInfoPtr pPayment = GetAPI()->GetICCPPaymentInfo(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nPaymentID));

	long nLocationID = AsLong(pPayment->locationID);
	COleCurrency cyAmount = DoubleToOleCurrency(pPayment->amount);
	CString strNameForReceipt = (LPCTSTR)pPayment->NameForReceipt;
	CString strCardName = (LPCTSTR)pPayment->CardType;
	CString strCardNumber = (LPCTSTR)pPayment->MaskedCardNumber;

	// (j.jones 2015-09-30 10:56) - PLID 67180 - get the signature on file for this transaction
	IStreamPtr pSignatureImageStream = NULL;

	if (pPayment->GZippedSignatureBitmap != NULL) {

		//get the safearray of the signature
		Nx::SafeArray<BYTE> saSignature = pPayment->GZippedSignatureBitmap;

		//get the signature BMP as an image stream
		pSignatureImageStream = GetICCPSignatureBitmapStream(nPaymentID, saSignature);
	}

	PrintCreditCardReceipt(pwndParent, bMerchantCopy, bCustomerCopy, nPaymentID, nLocationID, FALSE, cyAmount
		, strNameForReceipt, strCardName, strCardNumber, NULL, pSignatureImageStream);
}

// (j.jones 2015-09-30 10:56) - PLID 67180 - given a PaymentID and its ICCP signature safearray (a gzip file),
// returns an IStreamPtr of the signature image
IStreamPtr GetICCPSignatureBitmapStream(long nPaymentID, Nx::SafeArray<BYTE> &saGZippedSignature)
{
	if (saGZippedSignature == NULL) {
		ThrowNxException("GetICCPSignatureBitmapStream called with a null signature.");
	}

	//make a temp gzip file
	CString strGZipFileName;
	strGZipFileName.Format("%s\\Signature_%li_%li.gz", GetNxTempPath(), nPaymentID, GetTickCount());
	FileUtils::CAutoRemoveTempFile tempGZFile(strGZipFileName);

	//write to the gz file
	CFile(tempGZFile.GetPath(), CFile::modeWrite | CFile::modeCreate).Write(saGZippedSignature.begin(), saGZippedSignature.GetLength());

	//unzip the image		
	BYTE *image = NULL;
	DWORD dwLen = 0;
	CNxGZip zipSignature(tempGZFile.GetPath());
	if (!zipSignature.UncompressFileToMemory(&image, dwLen)) {
		ThrowNxException(_T("GetICCPSignatureBitmapStream could not uncompress signature data."));
	}

	//convert to a stream
	IStreamPtr pImageStream = Nx::StreamFromMemory(image, dwLen);

	//clear our memory
	if (image != NULL) {
		delete[] image;
		image = NULL;
	}

	return pImageStream;
}

// (z.manning 2015-09-09 09:52) - PLID 67226
void PromptToPrintICCPReceipts(CWnd *pwndParent, long nPaymentID)
{
	CCreditCardReceiptPromptDlg dlg(nPaymentID, pwndParent);
	dlg.DoModal();
}

// (z.manning 2015-08-18 14:50) - PLID 67248
// (c.haag 2015-09-22) - PLID 67202 - Allow for custom prompts
BOOL PromptForCreditCardExpirationDate(CWnd *pwndParent, OUT COleDateTime &dtExpiration, const CString& strPromptOverride)
{
	dtExpiration = g_cdtInvalid;

	BOOL bPromptForExpirationDate = TRUE;
	while (bPromptForExpirationDate)
	{
		CString strPrompt = strPromptOverride.GetLength() > 0 ? strPromptOverride : "To save the payment profile, please enter the expiration date of the card used in MM/YY format:";
		CString strExpDate;
		// (z.manning 2015-08-17 10:46) - PLID 67248 - Use a masked input to make things easier for the user
		// (z.manning 2015-08-24 08:50) - PLID 67251 - Now a function for the input mask
		int nResult = InputBoxMasked(pwndParent, strPrompt, ICCP::GetInputMaskForExpirationDate(), strExpDate, "MM/YY");

		bPromptForExpirationDate = FALSE;

		if (ICCP::ParseExpirationDate(strExpDate, dtExpiration)) {
			// (z.manning 2015-08-18 14:58) - PLID 67248 - 
			return TRUE;
		}
		else if (nResult == IDOK) {
			HWND hwndParent = (pwndParent == NULL ? NULL : pwndParent->GetSafeHwnd());
			MessageBox(hwndParent, "Please enter a valid expiration date.", NULL, MB_ICONERROR);
			// (z.manning 2015-08-14 15:13) - PLID 67248 - Give them another chance to enter the expiration date
			bPromptForExpirationDate = TRUE;
		}
		else {
			// (z.manning 2015-08-18 14:58) - PLID 67248 - User canceled
			return FALSE;
		}
	}

	return FALSE;
}

// (c.haag 2015-08-18) - PLID 67191 - Do a test swipe with a credit card. The card will not be charged.
// Returns TRUE on success, or FALSE on failure.
//		pwndParent - The parent window
//		nAccountID - The account ID of the merchant to test with
// Returns TRUE on success
BOOL DoCreditCardTestSwipe(CWnd* pwndParent, long nAccountID)
{
	// Safety check: We need a positive merchant account
	if (nAccountID <= 0)
	{
		ThrowNxException("AuthorizePayment was called with a non-positive merchant account ID!");
	}

	HWND hwndParent = (pwndParent == nullptr ? nullptr : pwndParent->GetSafeHwnd());

	// This loop allows the user to swipe subsequent times if the initial swipe failed
	while (true)
	{
		// Get an interface to the device
		NexTech_COM::IICCPSessionManager* pDevice = GetICCPDevice();
		if (nullptr == pDevice)
		{
			ThrowNxException("GetICCPDevice() returned NULL!");
		}

		// Have the customer perform the swipe
		BSTR bstrEncryptedTrack;
		BSTR bstrSwipeToken;
		CString strCreditCardToken;
		VARIANT_BOOL vbWasSigned;
		if (!ICCP::GetCardSecureTokenFromDevice(
			GetICCPDevice()
			, hwndParent
			, OleCurrencyToDouble(COleCurrency(0,0))
			, VARIANT_TRUE
			, VARIANT_FALSE
			, &bstrSwipeToken
			, &bstrEncryptedTrack
			, &vbWasSigned
			))
		{
			// Customer cancelled
			return FALSE;
		}
		strCreditCardToken = (LPCTSTR)_bstr_t(bstrSwipeToken);

		// This loop allows for retrying authorization with the payment processor
		BOOL bRetryAuthorization;
		do
		{
			// Reset the retry flag so we don't stay in this loop by default
			bRetryAuthorization = FALSE;

			// Now authorize the credit card purchase
			_TestCreditCardAuthorizationResultPtr pResult;
			{
				CWaitCursor wc;
				pResult = GetAPI()->TestCreditCardAuthorization(GetAPISubkey(), GetAPILoginToken(),
					_bstr_t(nAccountID), _bstr_t(strCreditCardToken), bstrEncryptedTrack);
			}

			// Look at the authorization result to see what we should do next
			if (VARIANT_FALSE != pResult->Success)
			{
				// Success! We have an approved (though uncaptured) authorization and there's nothing else to do here
				return TRUE;
			}
			else
			{
				// Failure
				NxTaskDialog dlgRetry;
				CString strFailureReason = (LPCTSTR)pResult->FailureReason;
				dlgRetry.Config()
					.WarningIcon()
					.ZeroButtons()
					.MainInstructionText("The payment processor failed to complete the authorization.")
					.ContentText(strFailureReason)
					.AddCommand(1000, "Try again")
					.AddCommand(1001, "Cancel")
					.DefaultButton(1000);

				if (1001 == dlgRetry.DoModal())
				{
					// User opted to cancel the authorization
					return FALSE;
				}
				else
				{
					// Lets retry the authorization
					bRetryAuthorization = TRUE;
				}
			}
		} while (bRetryAuthorization);

	} // while (true)
}

// (c.haag 2015-08-18) - PLID 67203 - Handles swiping and authorizing credit card payments
// (c.haag 2015-08-18) - PLID 67202 - Handles authorizing "Card Not Present" credit card payments
// (c.haag 2015-08-25) - PLID 67196 - Added nPaymentIDToRefund
// (c.haag 2015-08-27) - PLID 67191 - Added bTestAuthorization
// (z.manning 2015-08-31 09:39) - PLID 67230 - Added get siganture flag
// (z.manning 2015-09-01 11:35) - PLID 67229 - Added bWasSignedElectronically
//
//		pwndParent - The parent window
//		nLineItemID - The LineItemT.ID of the payment being authorized (which must exist before we can do an authorization)
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bCardPresent - True if the card is present to swipe; false if it is not present
//		bGetSignature - True if the device should capture the signature, false if not
//		bTestAuthorization - True if this is only a test authorization and we are not charging the card
//		pmapNonExpiredCreditCardTokens - A set of existing tokens to check for duplicates if creating a profile(can be null)
//
//		nPatientPersonID - The person ID of the patient
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount to charge. This should be a negative value if a refund is being performed.
//		dtCardExpiration - The expriation date of the card being used if already known
//		bCreatePaymentProfile - True to create a payment profile after authorization
//
//		strCreditCardToken - The credit card token
//		lpdispAuthorizationResult - The authorization result
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
// Returns TRUE on success
BOOL AuthorizeCreditCardTransaction(CWnd *pwndParent, long nLineItemID, long nPaymentIDToRefund, BOOL bCardPresent, BOOL bGetSignature, std::set<CString> *psetNonExpiredCreditCardTokens,
	long nPatientPersonID,  long nAccountID, COleCurrency& cyAmount, COleDateTime &dtCardExpiration, BOOL bCreatePaymentProfile,
	IN OUT CString &strCreditCardToken, OUT LPDISPATCH &lpdispAuthorizationResult, OUT BOOL &bWasSignedElectronically)
{
	// Safety check: All developers should know that this class should not be used if Integrated Credit Card Processing is enabled
	if (!IsICCPEnabled())
	{
		ThrowNxException("AuthorizePayment was called with ICCP disabled!");
	}
	else if (nAccountID <= 0)
	{
		ThrowNxException("AuthorizePayment was called with a non-positive merchant account ID!");
	}

	HWND hwndParent = (pwndParent == nullptr ? nullptr : pwndParent->GetSafeHwnd());

	bWasSignedElectronically = FALSE;

	// (z.manning 2015-09-02 09:52) - PLID 67232 - Check and see if a token was passed in, in which
	// case we don't need to prompt for a card from the device.
	BOOL bPromptForCreditCardToken = strCreditCardToken.IsEmpty();

	// This loop allows the user to swipe subsequent times if the initial swipe failed
	while (true)
	{
		// Get an interface to the device
		NexTech_COM::IICCPSessionManager* pDevice = GetICCPDevice();
		if (nullptr == pDevice)
		{
			ThrowNxException("GetICCPDevice() returned NULL!");
		}

		// (z.manning 2015-08-27 15:06) - PLID 67232 - Check and see if we already had the token
		// such as when using a card on file in which case we can skip this.
		BSTR bstrEncryptedTrack;
		if (bPromptForCreditCardToken)
		{
			// Have the customer perform the swipe or the user enter the card information
			BSTR bstrSwipeToken;
			VARIANT_BOOL vbWasSigned;
			// (z.manning 2015-09-14 14:29) - PLID 67221 - Pass in the amount
			if (!ICCP::GetCardSecureTokenFromDevice(
				GetICCPDevice()
				, hwndParent
				, OleCurrencyToDouble(cyAmount)
				, bCardPresent ? VARIANT_TRUE : VARIANT_FALSE
				, bGetSignature ? VARIANT_TRUE : VARIANT_FALSE
				, &bstrSwipeToken
				, &bstrEncryptedTrack
				, &vbWasSigned
			))
			{
				// Customer cancelled
				return FALSE;
			}
			strCreditCardToken = (LPCTSTR)_bstr_t(bstrSwipeToken);
			// (z.manning 2015-09-01 10:15) - PLID 67230 - Get the flag for whether or not we got a signature
			bWasSignedElectronically = (vbWasSigned ? TRUE : FALSE);
		}

		// (z.manning 2015-08-26 08:48) - PLID 67231 - If we are creating a payment profile as part of this
		// authorization then check and make sure the profile doesn't already exist.
		if (bCreatePaymentProfile)
		{
			if (psetNonExpiredCreditCardTokens != NULL)
			{
				if (psetNonExpiredCreditCardTokens->count(strCreditCardToken) > 0) {
					bCreatePaymentProfile = FALSE;
				}
			}
		}

		// (c.haag 2015-08-18) - PLID 67202 - Now prompt for the expiration date
		// (z.manning 2015-08-26 09:16) - PLID 67231 - If we're creating a payment profile then we
		// need to prompt for an expiration date no matter what. Normally, you could get the exp 
		// date from the track, but we the track info we get is encrypted so we cannot access it.
		// (z.manning 2015-08-27 15:21) - PLID 67232 - Check that we don't already have a valid exp date
		if ((!bCreatePaymentProfile && bCardPresent) || dtCardExpiration.GetStatus() == COleDateTime::valid)
		{
			// If the card is present, the expiration date is contained in the encrypted track
		}
		else if (!PromptForCreditCardExpirationDate(pwndParent, dtCardExpiration, !bCardPresent ? "Please enter the expiration date of the card used in MM/YY format:" : ""))
		{
			// (z.manning 2015-10-09 08:51) - PLID 67231 - Added a message prompt here at product's request
			if (bCreatePaymentProfile) {
				MessageBox(hwndParent, "You must enter an expiration date when creating a payment profile.", "Expiration Date", MB_ICONINFORMATION);
			}
			// User cancelled
			return FALSE;
		}

		// This loop allows for retrying authorization with the payment processor
		BOOL bRetryAuthorization;
		do
		{
			// Reset the retry flag so we don't stay in this loop by default
			bRetryAuthorization = FALSE;

			// Now authorize the credit card purchase
			NexTech_Accessor::_AuthorizeCreditCardPaymentResultPtr pResult;
			{
				CWaitCursor wc;
				double dAmount = (double)((CURRENCY)cyAmount).int64 / 10000.0;

				// (c.haag 2015-08-25) - PLID 67196 - If we have the ID of a payment being refunded, we must call
				// a different API function because it handles things a little differently and draws on the original
				// payment's account information. It also expects a positive amount so we have to flip the sign.
				if (nPaymentIDToRefund > 0)
				{
					if (dAmount > 0)
					{
						ThrowNxException("AuthorizePayment called with a payment ID to refund and a positive amount!");
					}
					else
					{
						// (c.haag 2015-10-07) - PLID 67310 - This now takes a request object
						NexTech_Accessor::_AuthorizeCreditCardPaymentRequestPtr pRequest(__uuidof(NexTech_Accessor::AuthorizeCreditCardPaymentRequest));
						pRequest->patientPersonID = _bstr_t(nPatientPersonID);
						pRequest->accountID = _bstr_t(nAccountID);
						pRequest->cardConnectCardToken = _bstr_t(strCreditCardToken);
						pRequest->Expiry = (dtCardExpiration.GetStatus() == COleDateTime::valid ? GetNullableDate(dtCardExpiration) : nullptr);
						pRequest->track = bCardPresent ? bstrEncryptedTrack : _bstr_t();
						pRequest->amount = -dAmount;
						pRequest->CreatePaymentProfile = bCreatePaymentProfile ? VARIANT_TRUE : VARIANT_FALSE;
						pResult = GetAPI()->RefundCreditCardPaymentWithOtherCard(GetAPISubkey(), GetAPILoginToken(),
							pRequest, _bstr_t(nPaymentIDToRefund));
					}
				}
				else
				{
					// (z.manning 2015-08-25 13:58) - PLID 67231 - This now takes a request object
					NexTech_Accessor::_AuthorizeCreditCardPaymentRequestPtr pRequest(__uuidof(NexTech_Accessor::AuthorizeCreditCardPaymentRequest));
					pRequest->patientPersonID = _bstr_t(nPatientPersonID);
					pRequest->accountID = _bstr_t(nAccountID);
					pRequest->cardConnectCardToken = _bstr_t(strCreditCardToken);
					pRequest->Expiry = (dtCardExpiration.GetStatus() == COleDateTime::valid ? GetNullableDate(dtCardExpiration) : nullptr);
					pRequest->track = bCardPresent ? bstrEncryptedTrack : _bstr_t();
					pRequest->amount = dAmount;
					// (z.manning 2015-08-25 16:26) - PLID 67231 - Pass in whether or not to create a payment profile
					pRequest->CreatePaymentProfile = bCreatePaymentProfile ? VARIANT_TRUE : VARIANT_FALSE;
					pResult = GetAPI()->AuthorizeCreditCardPayment(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nLineItemID), pRequest);
				}
			}

			// Look at the authorization result to see what we should do next
			if (pResult->status == NexTech_Accessor::ICCPTransactionStatus_Approved)
			{
				// Success! We have an approved authorization and there's nothing else to do here
				lpdispAuthorizationResult = pResult;
				return TRUE;
			}
			else if (pResult->status == NexTech_Accessor::ICCPTransactionStatus_Retry)
			{
				// Failure; the processor wants us to try again with the same data
				NxTaskDialog dlgRetry;
				dlgRetry.Config()
					.WarningIcon()
					.ZeroButtons()
					.MainInstructionText("The payment processor was reached but failed to complete the authorization")
					.ContentText("This can happen if the processor's network or system is temporarily unavailable.")
					.AddCommand(1000, "Try again")
					.AddCommand(1001, "Cancel")
					.DefaultButton(1000);

				if (1001 == dlgRetry.DoModal())
				{
					// User opted to cancel the authorization
					return FALSE;
				}
				else
				{
					// Lets retry the authorization
					bRetryAuthorization = TRUE;
				}
			}
			else
			{
				// Declined.
				//Allow the customer to swipe again but it may not always help.
				NxTaskDialog dlgRetry;
				dlgRetry.Config()
					.WarningIcon()
					.ZeroButtons()
					.MainInstructionText("The authorization was declined")
					.ContentText(pResult->ErrorText)
					.AddCommand(1000, "Try again")
					.AddCommand(1001, "Cancel")
					.DefaultButton(1000);

				if (1001 == dlgRetry.DoModal())
				{
					// User opted to cancel the authorization
					return FALSE;
				}
				else
				{
					// bRetryAuthorization was already set to false at the start of this loop. Now we'll start over
					// from scratch back to the start of the function.

					// If the card wasn't present, we need to make them enter the expiry again because maybe
					// that was what they entered incorrectly. In all other cases leave the expiry alone. If they
					// entered it incorrectly for the purposes of creating a payment profile, they can always go
					// back into the patient's payment profile list and fix it.
					if (!bCardPresent)
					{
						dtCardExpiration = g_cdtInvalid;
					}
				}
			}
		} while (bRetryAuthorization);

	} // while (true)
}

// (c.haag 2015-08-18) - PLID 67203 - Records an authorized credit card transaction to data
// (c.haag 2015-08-25) - PLID 67196 - Added nPaymentIDToRefund
// (z.manning 2015-09-01 11:35) - PLID 67229 - Added bWasSignedElectronically
//
//		lpdispAuthorizationResult - The result of a prior call to ReadAndAuthorizePaymentSwipe
//
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount the customer attempted to charge
//		bstrToken - The credit card token returned from a prior call to ReadAndAuthorizePaymentSwipe
//		nAuthorizedLineItemID - The ID of the authorized line item
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
void RecordAuthorizedTransaction(LPDISPATCH lpdispAuthorizationResult,
	long nAccountID, const COleCurrency& cyAmount, _bstr_t bstrToken, long nAuthorizedLineItemID, long nPaymentIDToRefund, BOOL bWasSignedElectronically)
{
	NexTech_Accessor::_AuthorizeCreditCardPaymentResultPtr pResult(lpdispAuthorizationResult);
	double dAmount = (double)((CURRENCY)cyAmount).int64 / 10000.0;

	// (c.haag 2015-08-25) - PLID 67196 - If we have the ID of a payment being refunded, we must call
	// a different API function because it handles things a little differently.
	if (nPaymentIDToRefund > 0)
	{
		GetAPI()->RecordAuthorizedCreditCardRefund(GetAPISubkey(), GetAPILoginToken(),
			_bstr_t(nAccountID), bstrToken, dAmount, _bstr_t(nPaymentIDToRefund), pResult->authorizationCode, 
			pResult->referenceNumber, pResult->amount, bWasSignedElectronically ? VARIANT_TRUE : VARIANT_FALSE, _bstr_t(nAuthorizedLineItemID));
	}
	else
	{
		GetAPI()->RecordAuthorizedCreditCardPayment(GetAPISubkey(), GetAPILoginToken(),
			_bstr_t(nAccountID), bstrToken, dAmount, _bstr_t(nAuthorizedLineItemID), pResult->authorizationCode,
			pResult->referenceNumber, pResult->amount, bWasSignedElectronically ? VARIANT_TRUE : VARIANT_FALSE);
	}
}

// (c.haag 2015-08-25) - PLID 67197 - Refunds an authorized credit card payment
//
//		nPaymentID - The ID of the payment to refund
//		cyAmount - The amount to refund
//		lpdispAuthorizationResult - The authorization result
//
// Returns TRUE on success
BOOL RefundAuthorizedPayment(long nPaymentID, const COleCurrency& cyAmount, OUT LPDISPATCH& lpdispAuthorizationResult)
{
	// Safety check: All developers should know that this class should not be used if Integrated CDredit Card Processing is enabled
	if (!IsICCPEnabled())
	{
		ThrowNxException("RefundAuthorizedPayment was called with ICCP disabled!");
	}
	else if (nPaymentID <= 0)
	{
		ThrowNxException("RefundAuthorizedPayment was called with a non-positive payment ID!");
	}

	// This loop allows for retrying authorization with the payment processor
	while (true)
	{
		// Now authorize the credit card purchase
		NexTech_Accessor::_AuthorizeCreditCardPaymentResultPtr pResult;
		{
			CWaitCursor wc;
			double dAmount = (double)((CURRENCY)cyAmount).int64 / 10000.0;

			// Can't refund a positive amount
			// If it is a zero amount, a voided payment will post as a full refund, so do not allow that either
			if (dAmount >= 0) {
				ThrowNxException("RefundAuthorizedPayment was called with a positive or zero amount!");
			}

			// ICCP expects a positive amount when processing a refund so we have to flip the sign.
			dAmount = -dAmount;

			try
			{
				pResult = GetAPI()->RefundCreditCardPayment(GetAPISubkey(), GetAPILoginToken(),
					_bstr_t(nPaymentID), dAmount);
			}
			catch (_com_error& e)
			{
				// (c.haag 2015-09-22) - PLID 67193 - Look for an exception denoting that the transaction was
				// previously voided. This can happen if a user accidentally tries to refund a transaction twice.
				CString str = (LPCTSTR)e.Description();
				if (-1 != str.Find("The transaction was previously voided."))
				{
					MessageBox(GetActiveWindow(), "The refund failed because the payment was voided.",
						"Practice", MB_ICONINFORMATION | MB_OK);
					return FALSE;
				}
				else
				{
					throw;
				}
			}
		}

		// Look at the authorization result to see what we should do next
		if (pResult->status == NexTech_Accessor::ICCPTransactionStatus_Approved)
		{
			// Success! We have an approved authorization and there's nothing else to do here
			lpdispAuthorizationResult = pResult;
			return TRUE;
		}
		else if (pResult->status == NexTech_Accessor::ICCPTransactionStatus_Retry)
		{
			// Failure; the processor wants us to try again with the same data
			NxTaskDialog dlgRetry;
			dlgRetry.Config()
				.WarningIcon()
				.ZeroButtons()
				.MainInstructionText("The payment processor was reached but failed to complete the refund")
				.ContentText("This can happen if the processor's network or system is temporarily unavailable.")
				.AddCommand(1000, "Try again")
				.AddCommand(1001, "Cancel")
				.DefaultButton(1000);

			if (1001 == dlgRetry.DoModal())
			{
				// User opted to cancel the refund
				return FALSE;
			}
			else
			{
				// Lets retry the refund
			}
		}
		else
		{
			// Declined. Allow the customer to swipe again but it may not always help.
			NxTaskDialog dlgRetry;
			dlgRetry.Config()
				.WarningIcon()
				.ZeroButtons()
				.MainInstructionText("The refund was declined")
				.ContentText(pResult->ErrorText)
				.AddCommand(1000, "Try again")
				.AddCommand(1001, "Cancel")
				.DefaultButton(1000);

			if (1001 == dlgRetry.DoModal())
			{
				// User opted to cancel the refund
				return FALSE;
			}
			else
			{
				// Lets retry the refund
			}
		}
	} // while (true)
}

// (c.haag 2015-08-25) - PLID 67197 - Records an authorized refund
//
//		lpdispAuthorizationResult - The result of a prior call to ReadAndAuthorizePaymentSwipe
//
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount the customer attempted to charge
//		bstrToken - The credit card token returned from a prior call to ReadAndAuthorizePaymentSwipe
//		nAuthorizedLineItemID - The ID of the authorized line item
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
void RecordAuthorizedPaymentRefund(LPDISPATCH lpdispAuthorizationResult,
	long nAccountID, const COleCurrency& cyAmount, _bstr_t bstrToken, long nAuthorizedLineItemID, long nPaymentIDToRefund, BOOL bWasSignedElectronically)
{
	RecordAuthorizedTransaction(lpdispAuthorizationResult, nAccountID, cyAmount, bstrToken, nAuthorizedLineItemID, nPaymentIDToRefund, bWasSignedElectronically);
}

// (j.jones 2015-09-30 10:34) - PLID 67171 - This confusingly named function
// is meant to be called when about to make a refund for a credit card payment.
// If ICCP is enabled, and the payment was not processed using ICCP, this will warn
// the user that they cannot process the refund using the same card as the payment.
void CheckWarnCreditCardPaymentRefundMismatch(long nCreditCardPaymentID)
{
	if (IsICCPEnabled()) {
		if (IsCCPaymentProcessedPreICCP(nCreditCardPaymentID)) {

			MessageBox(GetActiveWindow(), "To refund this payment, you will need to swipe or dip the credit card, "
				"manually enter the card information using a Card Not Present transaction, "
				"use a card on file (if available), or other type of refund that will not be processed.",
				"Practice", MB_ICONINFORMATION | MB_OK);

			//don't return, it's ok to continue
		}
	}
}

// (b.eyers 2015-10-06) - PLID 42101 - audit when the batch changes for insurance claim bills
void AuditInsuranceBatch(std::vector<long> &aryBillIDs, long nNewBatch, long nOldBatch)
{
	//batch = 1 is paper, batch = 2 is elecronic, not in HCFATrackT is unbatched or for purposes of this, 0

	std::vector<CString> aryPatientName;
	std::vector<long> aryPatientID;
	std::vector<CString> aryOldValue;

	_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.ID, BillsT.Date, BillsT.Description, BillsT.PatientID, PersonT.FullName "
		"FROM BillsT "
		"LEFT JOIN PersonT ON PersonT.ID = BillsT.PatientID "
		"WHERE BillsT.ID IN ({INTVECTOR})", aryBillIDs);
	aryBillIDs.clear();
	//get bill information for auditing
	while (!rs->eof) {
		aryBillIDs.push_back(AdoFldLong(rs, "ID"));
		aryPatientName.push_back(AdoFldString(rs, "FullName"));
		aryPatientID.push_back(AdoFldLong(rs, "PatientID"));
		
		CString strOldValue;
		COleDateTime dtDate;
		
		//build old value for auditing
		if (nOldBatch == 2)
			strOldValue = "Electronic Batch - ";
		else if (nOldBatch == 1)
			strOldValue = "Paper Batch - ";
		else 
			strOldValue = "Unbatched - ";
		dtDate = AdoFldDateTime(rs, "Date");
		strOldValue += FormatDateTimeForInterface(dtDate, 0, dtoDate);
		strOldValue += " ";
		strOldValue += AdoFldString(rs, "Description");
		aryOldValue.push_back(strOldValue);

		rs->MoveNext();
	}
	rs->Close();

	CString strNewValue;
	if (nNewBatch == 2)
		strNewValue = "Electronic Batch";
	else if (nNewBatch == 1)
		strNewValue = "Paper Batch";
	else
		strNewValue = "Unbatched";

	long nSize = aryBillIDs.size();
	long nAuditTransactionID = BeginAuditTransaction();
	for (int i = 0; i < nSize; i++) 
	{
		AuditEvent(aryPatientID[i], aryPatientName[i], nAuditTransactionID, aeiBatchInsurance, aryBillIDs[i], aryOldValue[i], strNewValue, aepMedium, aetChanged);
	}
	CommitAuditTransaction(nAuditTransactionID);
}

// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format,
// returns EbillingFormatsT.ID
long GetDefaultEbillingANSIFormatID()
{
	//get the last remembered FormatStyle, this is unified between E-Billing & E-Eligibility
	long nFormatID = GetRemotePropertyInt("FormatStyle", -1, 0, "<None>", true);

	if (nFormatID == -1) {
		//this sorts by ANSIVersion such that if they have a 5010 entry, that will be returned instead of a 4010 entry
		_RecordsetPtr rsFormat = CreateRecordset("SELECT TOP 1 ID FROM EbillingFormatsT ORDER BY ANSIVersion DESC");
		if (!rsFormat->eof) {
			nFormatID = AdoFldLong(rsFormat, "ID");
		}
		rsFormat->Close();
	}

	return nFormatID;
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeCode_HCFA(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:
			return "6";
			break;
		case ctcReplacement:
			return "7";
			break;
		case ctcVoid:
			return "8";
			break;
		case ctcOriginal:
		default:
			return "1";
			break;
	}
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeDescription_HCFA(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:
			return "Corrected";
			break;
		case ctcReplacement:
			return "Replacement";
			break;
		case ctcVoid:
			return "Void";
			break;
		case ctcOriginal:
		default:
			return "Original";
			break;
	}
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeCode_UB(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:				//for the UB, this is actually "Adjustment of Prior Claim"
			return "6";
			break;
		case ctcReplacement:
			return "7";
			break;
		case ctcVoid:
			return "8";
			break;
		case ctcOriginal:	//for the UB, this is actually "Admit through Discharge Claim"
		default:
			return "1";
			break;
	}
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeDescription_UB(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:
			return "Adjustment of Prior Claim";
			break;
		case ctcReplacement:
			return "Replacement of Prior Claim";
			break;
		case ctcVoid:
			return "Void/Cancel of Prior Claim";
			break;
		case ctcOriginal:
		default:
			return "Original (Admit Through Discharge Claim)";
			break;
	}
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeCode_Alberta(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:		//Change
			return "C";
			break;
		case ctcReplacement:	//Reassess
			return "R";
			break;
		case ctcVoid:			//Delete
			return "D";
			break;
		case ctcOriginal:		//Add
		default:
			return "A";
			break;
	}
}

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeDescription_Alberta(ANSI_ClaimTypeCode ctcType)
{
	switch (ctcType) {
		case ctcCorrected:
			return "Change";
			break;
		case ctcReplacement:
			return "Reassess";
			break;
		case ctcVoid:
			return "Delete";
			break;
		case ctcOriginal:
		default:
			return "Add";
			break;
	}
}

//The following three functions returns a SQL fragment that acts as an INNER JOIN
//on EligibilityResponsesT, can be used in any query that includes EligibilityResponsesT.

//This will filter only on responses that indicate they have active coverage.
CSqlFragment GetEligibilityActiveCoverageInnerJoin()
{
	//active coverage is defined by a valid response (EligibilityResponsesT.ConfirmationStatus = 1)
	//that has EligibilityResponseDetailsT records with one of the five Active benefit types:
	//1 - Active Coverage
	//2 - Active - Full Risk Capitation
	//3 - Active - Services Capitated
	//4 - Active - Services Capitated to Primary Care Physician
	//5 - Active - Pending Investigation
	return CSqlFragment(" INNER JOIN ("
		"	SELECT EligibilityResponsesT.ID "
		"	FROM EligibilityResponsesT "
		"	INNER JOIN ("
		"		SELECT ResponseID "
		"		FROM EligibilityResponseDetailsT "
		"		INNER JOIN EligibilityDataReferenceT ON EligibilityResponseDetailsT.BenefitTypeRefID = EligibilityDataReferenceT.ID "
		"		WHERE EligibilityDataReferenceT.ListType = 1 "	//Benefit Types
		"		AND EligibilityDataReferenceT.Qualifier IN ('1', '2', '3', '4', '5') "	//these are the five Active coverage types
		"		GROUP BY ResponseID "
		"	) AS ActiveResponseDetailsQ ON EligibilityResponsesT.ID = ActiveResponseDetailsQ.ResponseID "
		"	WHERE EligibilityResponsesT.ConfirmationStatus = {CONST_INT} "
		"	GROUP BY EligibilityResponsesT.ID "
		") AS ActiveResponsesQ ON EligibilityResponsesT.ID = ActiveResponsesQ.ID ",
		EligibilityResponseConfirmationStatusEnum::ercsConfirmed);
}

//This will filter only on responses that indicate they have inactive coverage.
CSqlFragment GetEligibilityInactiveCoverageInnerJoin()
{
	//inactive coverage is defined by a valid response (EligibilityResponsesT.ConfirmationStatus = 1)
	//that has EligibilityResponseDetailsT records with one of the three Inactive benefit types:
	//6 - Inactive
	//7 - Inactive - Pending Eligibility Update
	//8 - Inactive - Pending Investigation
	return CSqlFragment(" INNER JOIN ("
		"	SELECT EligibilityResponsesT.ID "
		"	FROM EligibilityResponsesT "
		"	INNER JOIN ("
		"		SELECT ResponseID "
		"		FROM EligibilityResponseDetailsT "
		"		INNER JOIN EligibilityDataReferenceT ON EligibilityResponseDetailsT.BenefitTypeRefID = EligibilityDataReferenceT.ID "
		"		WHERE EligibilityDataReferenceT.ListType = 1 "	//Benefit Types
		"		AND EligibilityDataReferenceT.Qualifier IN ('6', '7', '8') "	//these are the three Inactive coverage types
		"		GROUP BY ResponseID "
		"	) AS InactiveResponseDetailsQ ON EligibilityResponsesT.ID = InactiveResponseDetailsQ.ResponseID "
		"	WHERE EligibilityResponsesT.ConfirmationStatus = {CONST_INT} "
		"	GROUP BY EligibilityResponsesT.ID "
		") AS InactiveResponsesQ ON EligibilityResponsesT.ID = InactiveResponsesQ.ID ",
		EligibilityResponseConfirmationStatusEnum::ercsConfirmed);
}

//This will filter only on responses that have a failure reported.
CSqlFragment GetEligibilityFailedResponseInnerJoin()
{
	//a denied or failed response is flagged as such in EligibilityResponsesT.ConfirmationStatus,
	//but the absence of EligibilityResponseDetailsT records also mean something failed
	return CSqlFragment(" INNER JOIN ("
		"	SELECT EligibilityResponsesT.ID "
		"	FROM EligibilityResponsesT "
		"	LEFT JOIN ("
		"		SELECT ResponseID "
		"		FROM EligibilityResponseDetailsT "
		"		GROUP BY ResponseID "
		"	) AS AnyResponseDetailsQ ON EligibilityResponsesT.ID = AnyResponseDetailsQ.ResponseID "
		"	WHERE EligibilityResponsesT.ConfirmationStatus IN ({CONST_INT}, {CONST_INT}) "
		"		OR AnyResponseDetailsQ.ResponseID Is Null "
		"	GROUP BY EligibilityResponsesT.ID "
		") AS FailedResponsesQ ON EligibilityResponsesT.ID = FailedResponsesQ.ID ",
		EligibilityResponseConfirmationStatusEnum::ercsDenied,
		EligibilityResponseConfirmationStatusEnum::ercsInvalid);
}

//gets the highest priority insured party (e.g. RespType of 1) for this patient,
//returns a SQL query to return that TOP 1 InsuredPartyID
CSqlFragment GetPatientPrimaryInsuredPartyIDSql(long nPatientID)
{
	return CSqlFragment("SELECT TOP 1 InsuredPartyT.PersonID "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE InsuredPartyT.PatientID = {INT} "
		"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC",
		nPatientID);
}