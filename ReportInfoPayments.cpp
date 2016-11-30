////////////////
// DRT 8/6/03 - GetSqlPayments() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "MsgBox.h"
#include "ReportAdo.h"

const CString GetPaymentsUnderAllowedAmountReportQuery(_variant_t varEobID);

CString CReportInfo::GetSqlPayments(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;

	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

	switch (nID) {


	case 188:
		//Payments By Referral Source
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 13:57) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByReferralSourceSubQ.ID, PaymentsByReferralSourceSubQ.PatientID AS PatID, "
		"PaymentsByReferralSourceSubQ.UserDefinedID, "
		"PaymentsByReferralSourceSubQ.ProvID AS ProvID,  "
		"PaymentsByReferralSourceSubQ.TDate AS TDate,  "
		"PaymentsByReferralSourceSubQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"PaymentsByReferralSourceSubQ.CheckNo,  "
		"PaymentsByReferralSourceSubQ.CCType,  "
		"PaymentsByReferralSourceSubQ.Description,  "
		"Sum(PaymentsByReferralSourceSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByReferralSourceSubQ.PayMethod AS PayMethod,  "
		"ReferralSourceT.PersonID AS ReferralID,  "
		"ReferralSourceT.Name AS [Referral Name], "
		"PaymentsByReferralSourceSubQ.LocID AS LocID, "
		"PaymentsByReferralSourceSubQ.Location, "
		"PaymentsByReferralSourceSubQ.IDate AS IDate "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE  "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE  [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PatientsT.ReferralID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM ((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByReferralSourceFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PatientsT.ReferralID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location "
		"FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByReferralSourcePartQ "
		") AS PaymentsByReferralSourceSubQ LEFT JOIN ReferralSourceT ON PaymentsByReferralSourceSubQ.ReferralID = ReferralSourceT.PersonID) LEFT JOIN (ProvidersT LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) ON PaymentsByReferralSourceSubQ.ProvID = ProvidersT.PersonID "
		"GROUP BY PaymentsByReferralSourceSubQ.ID, PaymentsByReferralSourceSubQ.PatientID, PaymentsByReferralSourceSubQ.ProvID, PaymentsByReferralSourceSubQ.TDate, PaymentsByReferralSourceSubQ.FullName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByReferralSourceSubQ.CheckNo, PaymentsByReferralSourceSubQ.CCType, PaymentsByReferralSourceSubQ.Description, PaymentsByReferralSourceSubQ.PayMethod, ReferralSourceT.PersonID, ReferralSourceT.Name, PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.LocID, PaymentsByReferralSourceSubQ.Location, "
		"PaymentsByReferralSourceSubQ.IDate");
		break;
	

	case 171:
		//Refunds By Provider
		/*	Version History
			- TES 3/13/03: Changed to use destination location if applied.  Also fixed the previous attempt to have it 
			use the destination provider if applied, which had attempted to pull from ChargesT (this is refunds, fool!)
			TES 6/15/05 - PLID 15946 - The 'Location' field was pulling the name of the location in the deepest subquery, 
				meaning that the LocationID that is filtered on might show the wrong name.
			JMM 02/10/2006 - PLID 18268 - added fields so you could make the report print refund checks
			// (a.walling 2006-10-18 12:48) - PLID 23126 - Only pull info from the primary responsible party
			// (j.gruber 2007-02-21 15:09) - PLID 23279 - add primary and secondary insurance information
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking
		*/
		strSQL.Format("SELECT PaymentsByProviderSubQ.ID,  "
		"PaymentsByProviderSubQ.PatientID,  "
		"Sum(PaymentsByProviderSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByProviderSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByProviderSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByProviderSubQ.PatID AS PatID,  "
		"PaymentsByProviderSubQ.UserDefinedID, "
		"PaymentsByProviderSubQ.FullName,  "
		"PaymentsByProviderSubQ.IDate AS IDate,  "
		"PaymentsByProviderSubQ.TDate AS TDate,  "
		"PaymentsByProviderSubQ.PayMethod AS PayMethod,  "
		"PaymentsByProviderSubQ.Description,  "
		"PaymentsByProviderSubQ.CheckNo,  "
		"PaymentsByProviderSubQ.CCType,  "
		"Min(PaymentsByProviderSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByProviderSubQ.LineID) AS FirstOfLineID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByProviderSubQ.LocID AS LocID, "
		"LocationsT.Name AS Location, "
		"PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, "
		" PatZip, PatHomePhone, PatWorkPhone, PatSSN, "
		" RespPersonT.First AS RespFirst, RespPersonT.Middle AS RespMiddle, RespPersonT.Last AS RespLast, "
		" RespPersonT.Address1 AS RespAdd1, RespPersonT.Address2 AS RespAdd2, RespPersonT.City as RespCity, "
		" RespPersonT.State AS RespState, RespPersonT.Zip AS RespZip, RespPersonT.HomePhone AS RespHomePhone, "
		" InsCoName, InsAddress1, InsAddress2, "
		" InsCity, InsState, InsZip "
		"FROM (SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE  "
		"	WHEN _PartiallyAppliedPaysQ.ID Is NULL "
		"	THEN "
		"		CASE "
		"		WHEN LineItemT_1.ID Is Null "
		"		THEN LineItemT.Amount "
		"		ELSE AppliesT.Amount "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN DestPays.ProviderID Is NULL "
		"	THEN PaymentsT.[ProviderID] "
		"	ELSE DestPays.ProviderID "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 AS PatAdd2, "
		" PersonT.City as PatCity, PersonT.State as PatState, PersonT.Zip as PatZip, PersonT.HomePhone as PatHomePhone, PersonT.WorkPhone as PatWorkPhone, "
		" dbo.MaskSSN(PersonT.SocialSecurity, %s) as PatSSN, "
		" InsCoT.Name as InsCoName, InsPersonT.Address1 as InsAddress1, InsPersonT.Address2 as InsAddress2, "
		" InsPersonT.City As InsCity, InsPersonT.State as InsState, InsPersonT.Zip as InsZip "
		"FROM ((((((LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		" LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
		" LEFT JOIN InsuranceCoT InsCoT ON InsuredPartyT.InsuranceCoID = InsCoT.PersonID "
		" LEFT JOIN PersonT InsPersonT ON InsCoT.PersonID = InsPersonT.ID "
		" LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "

		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN PaymentsT DestPays ON LineItemT_1.ID = DestPays.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE ((PaymentsT.ID Is Not NULL) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=3))) "
		"AS _PaymentsByProviderFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 AS PatAdd2, "
		" PersonT.City as PatCity, PersonT.State as PatState, PersonT.Zip as PatZip, PersonT.HomePhone as PatHomePhone, PersonT.WorkPhone as PatWorkPhone, "
		" dbo.MaskSSN(PersonT.SocialSecurity, %s) as PatSSN, "
		" InsCoT.Name as InsCoName, InsPersonT.Address1 as InsAddress1, InsPersonT.Address2 as InsAddress2, "
		" InsPersonT.City As InsCity, InsPersonT.State as InsState, InsPersonT.Zip as InsZip "
		"FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN LineItemT ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		" LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
		" LEFT JOIN InsuranceCoT InsCoT ON InsuredPartyT.InsuranceCoID = InsCoT.PersonID "
		" LEFT JOIN PersonT InsPersonT ON InsCoT.PersonID = InsPersonT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=3))) "
		"AS _PaymentsbyProviderPartQ) "
		"AS PaymentsByProviderSubQ LEFT JOIN (ProvidersT LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) ON PaymentsByProviderSubQ.ProvID = ProvidersT.PersonID "
		"LEFT JOIN LocationsT ON PaymentsByProviderSubQ.LocID = LocationsT.ID "
		"LEFT JOIN PatientsT ON PaymentsByProviderSubQ.PatientID = PatientsT.PersonID "
		" LEFT JOIN ResponsiblePartyT ON PatientsT.PrimaryRespPartyID = ResponsiblePartyT.PatientID "
		" LEFT JOIN PersonT AS RespPersonT ON PatientsT.PrimaryRespPartyID = RespPersonT.ID "
		"GROUP BY PaymentsByProviderSubQ.ID, PaymentsByProviderSubQ.PatientID, PaymentsByProviderSubQ.ProvID, PaymentsByProviderSubQ.PatID, PaymentsByProviderSubQ.UserDefinedID, PaymentsByProviderSubQ.FullName, PaymentsByProviderSubQ.IDate, PaymentsByProviderSubQ.TDate, PaymentsByProviderSubQ.PayMethod, PaymentsByProviderSubQ.Description, PaymentsByProviderSubQ.CheckNo, PaymentsByProviderSubQ.CCType, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByProviderSubQ.LocID, LocationsT.Name, "
		"PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, "
		" PatZip, PatHomePhone, PatWorkPhone, PatSSN, RespPersonT.First, RespPersonT.Middle, "
		" RespPersonT.Last, RespPersonT.Address1, RespPersonT.Address2, RespPersonT.City, "
		" RespPersonT.State, RespPersonT.Zip, RespPersonT.HomePhone, "
		" InsCoName, InsAddress1, InsAddress2, "
		" InsCity, InsState, InsZip "
		"",
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
		return _T(strSQL);
		break;
	case 172:
		//Adjustments By Insurance Co
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			// (j.gruber 2007-05-29 16:34) - PLID 24837 - added reason code and group code
			// (j.jones 2010-09-24 15:24) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.goldschmidt 2014-01-28 14:41) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByInsCoSubQ.ID,  "
		"PaymentsByInsCoSubQ.PatientID,  "
		"Sum(PaymentsByInsCoSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByInsCoSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByInsCoSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByInsCoSubQ.PatID AS PatID,  "
		"PaymentsByInsCoSubQ.UserDefinedID, "
		"PaymentsByInsCoSubQ.FullName,  "
		"PaymentsByInsCoSubQ.IDate AS IDate,  "
		"PaymentsByInsCoSubQ.TDate AS TDate,  "
		"PaymentsByInsCoSubQ.PayMethod,  "
		"PaymentsByInsCoSubQ.Description,  "
		"PaymentsByInsCoSubQ.[Insurance Co ID] AS [Insurance Co ID],  "
		"PaymentsByInsCoSubQ.InsName,  "
		"Min(PaymentsByInsCoSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByInsCoSubQ.LineID) AS FirstOfLineID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByInsCoSubQ.LocID AS LocID, "
		"PaymentsByInsCoSubQ.Location, "
		"PaymentsByInsCoSubQ.ReasonCode, "
		"PaymentsByInsCoSubQ.GroupCode, "
		"PaymentsByInsCoSubQ.ReasonCodeDesc, "
		"PaymentsByInsCoSubQ.GroupCodeDesc "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"InsuranceCoT.PersonID AS [Insurance Co ID],  "
		"InsuranceCoT.Name AS InsName,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) "		
		"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) "
		") AS PaymentsByInsCoFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"InsuranceCoT.PersonID AS [Insurance Co ID], "
		"InsuranceCoT.Name AS InsName,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) "
		") AS PaymentsByInsCoPartQ) AS PaymentsByInsCoSubQ LEFT JOIN ProvidersT ON PaymentsByInsCoSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsByInsCoSubQ.ID, PaymentsByInsCoSubQ.PatientID, PaymentsByInsCoSubQ.ProvID, PaymentsByInsCoSubQ.PatID, "
		"PaymentsByInsCoSubQ.UserDefinedID, PaymentsByInsCoSubQ.FullName, PaymentsByInsCoSubQ.IDate, PaymentsByInsCoSubQ.TDate, PaymentsByInsCoSubQ.PayMethod, "
		"PaymentsByInsCoSubQ.Description, PaymentsByInsCoSubQ.[Insurance Co ID], PaymentsByInsCoSubQ.InsName, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByInsCoSubQ.LocID, PaymentsByInsCoSubQ.Location, "
		"PaymentsByInsCoSubQ.ReasonCode, "
		"PaymentsByInsCoSubQ.GroupCode, "
		"PaymentsByInsCoSubQ.ReasonCodeDesc, "
		"PaymentsByInsCoSubQ.GroupCodeDesc "
		"");
		break;
	case 173:
		//Adjustments By Insurance By CPT
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			DRT 4/10/2006 - PLID 11734 - Removed ProcCode usage.
			// (j.gruber 2007-05-29 16:32) - PLID 24837 - Added reason code and group code
			// (j.jones 2010-09-24 15:24) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (f.gelderloos 2013-07-23 15:33) - PLID 57043 Aliasing ServiceDate for 173, 215, and 216
			// (r.goldschmidt 2014-01-28 14:51) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByInsCoByCPTCodeFullQ.ID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatientID,  "
		"PaymentsByInsCoByCPTCodeFullQ.ApplyAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ProvID AS ProvID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID AS PatID,  "
		"PaymentsByInsCoByCPTCodeFullQ.UserDefinedID, "
		"PaymentsByInsCoByCPTCodeFullQ.FullName,  "
		"PaymentsByInsCoByCPTCodeFullQ.IDate AS IDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.TDate AS TDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayMethod AS PayMethod,  "
		"PaymentsByInsCoByCPTCodeFullQ.Description,  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfItemCode AS ItemCode,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayAmt,  "
		"PaymentsByInsCoByCPTCodeFullQ.ChargeAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ServiceDate AS ServiceDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.ItemDesc,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsID AS InsID,  "
		"CASE WHEN PaymentsByInsCoByCPTCodeFullQ.InsID IS NULL THEN 'No Insurance' ELSE PaymentsByInsCoByCPTCodeFullQ.InsName END AS InsName, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"[FirstOfItemCode] AS [CPT Code],  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfID AS CPTID, "
		"PaymentsByInsCoByCPTCodeFullQ.LocID AS LocID, "
		"PaymentsByInsCoByCPTCodeFullQ.Location, "
		"PaymentsByInsCoByCPTCodeFullQ.ReasonCode, "
		"PaymentsByInsCoByCPTCodeFullQ.GroupCode, "
		"PaymentsByInsCoByCPTCodeFullQ.ReasonCodeDesc, "
		"PaymentsByInsCoByCPTCodeFullQ.GroupCodeDesc "
		"FROM ((((SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"ApplyAmount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ChargesT.ItemCode AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"InsuranceCoT.PersonID AS InsID,  "
		"InsuranceCoT.Name AS InsName,  "
		"ChargesT.ItemSubCode,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"Min(ServiceT.ID) AS FirstOfID,  "
		"ChargesT.Category, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) "
		"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyId = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)) AND CPTCodeT.ID IS NOT NULL "
		"GROUP BY LineItemT.ID, ChargesT.ID, LineItemT.PatientID, (CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End), CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End, LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.ItemSubCode, AppliesT.ID, LineItemT.ID, ChargesT.Category, PaymentsT.ID, LineItemT.Type, PatientsT.UserDefinedID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, "
		"	AdjustmentGroupCodesT.Code, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description), "
		"	AdjustmentReasonCodesT.Code, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) "
		"HAVING (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Type)=2)) "
		") AS PaymentsByInsCoByCPTCodeFullQ LEFT JOIN ProvidersT ON PaymentsByInsCoByCPTCodeFullQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN ServiceT ON PaymentsByInsCoByCPTCodeFullQ.FirstOfID = ServiceT.ID) LEFT JOIN (SELECT CategoriesT.ID AS CategoryID,  "
		"           CategoriesT.Name AS Category, '' AS SubCategory,  "
		"           CategoriesT.ID AS ParentID "
		"      FROM CategoriesT "
		"      WHERE (((CategoriesT.Parent) = 0)) "
		"      UNION "
		"      SELECT SubCategoriesT.ID AS CategoryID,  "
		"          CategoriesT.Name AS Category,  "
		"          SubCategoriesT.Name AS SubCategory,  "
		"          SubCategoriesT.Parent AS ParentID "
		"      FROM CategoriesT RIGHT JOIN "
		"          CategoriesT AS SubCategoriesT ON  "
		"          CategoriesT.ID = SubCategoriesT.Parent "
		"      WHERE (((SubCategoriesT.Parent) <> 0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID "
		"");
		break;
	case 174:
		//Adjustments By Service Code
		/*	Version History
			DRT 2/10/2004 - PLID 9892 - Added charge date to the query and made it a date filter.  Made report editable, so they can put the
				charge date wherever they like.  Also re-organized the query (spacing mostly) to be a little easier to read.
			DRT 4/10/2006 - PLID 11734 - Removed ProcCode usage.
			// (j.gruber 2007-05-29 16:38) - PLID 24837 - added reason code and group code
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2010-09-24 16:42) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.goldschmidt 2014-01-28 14:52) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,  AdjustmentsByCPTCodeFullQ.FullName,  PersonT.Last + ', ' + PersonT.First AS DocName,   "
			"AdjustmentsByCPTCodeFullQ.TDate AS TDate,  AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod,  AdjustmentsByCPTCodeFullQ.CCType,   "
			"AdjustmentsByCPTCodeFullQ.CheckNo,  AdjustmentsByCPTCodeFullQ.Description,  AdjustmentsByCPTCodeFullQ.ApplyAmount,   "
			"CategoriesQ.Category,  CategoriesQ.SubCategory,  CategoriesQ.ParentID AS ParentID,  AdjustmentsByCPTCodeFullQ.PatID AS PatID,  "
			"AdjustmentsByCPTCodeFullQ.UserDefinedID,  AdjustmentsByCPTCodeFullQ.IDate AS IDate,  AdjustmentsByCPTCodeFullQ.Type, AdjustmentsByCPTCodeFullQ.ItemCode,  "
			"AdjustmentsByCPTCodeFullQ.ChargeAmount, AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, AdjustmentsByCPTCodeFullQ.ServiceDate, AdjustmentsByCPTCodeFullQ.ItemDesc,  "
			"AdjustmentsByCPTCodeFullQ.LocID AS LocID, AdjustmentsByCPTCodeFullQ.Location, AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, AdjustmentsByCPTCodeFullQ.ChargeDate AS ChargeDate, "
			"AdjustmentsByCPTCodeFullQ.ReasonCode, AdjustmentsByCPTCodeFullQ.GroupCode, "
			"AdjustmentsByCPTCodeFullQ.ReasonCodeDesc, AdjustmentsByCPTCodeFullQ.GroupCodeDesc "
			"FROM ((( "
			"	(SELECT LineItemT.ID,   "
			"	ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,   "
			"	CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End AS ProvID,  LineItemT.PatientID AS PatID, PatientsT.UserDefinedID,   "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  LineItemT.InputDate AS IDate,  LineItemT.Date AS TDate,  PaymentsT.PayMethod,   "
			"	LineItemT.Description,  ChargesT.ItemCode AS FirstOfItemCode,  LineItemT.Amount AS PayAmt,   "
			"	dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
			"	LineItemT_1.Date AS ServiceDate,  LineItemT_1.Description AS ItemDesc,  CreditCardNamesT.CardName AS CCType,  PaymentPlansT.CheckNo,  ChargesT.Category,  AppliesT.ID AS ApplyID,   "
			"	LineItemT.ID AS LineID,  LineItemT.Type, ChargesT.ItemCode, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, CPTCodeT.ID AS CPTID, LineItemT_1.Date AS ChargeDate, "
			"	AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
			"	AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
			"	FROM (((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "			
			"		LEFT JOIN  "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  Sum(AppliesT.Amount) AS ApplyAmt,   "
			"		/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"		LineItemT_1.PatientID   "
			"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"		ON LineItemT_1.ID = PaymentsT.ID  "			
			"		WHERE (((LineItemT_1.Deleted)=0))  "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"		HAVING (LineItemT_1.ID Is Not Null AND (MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)  "
			"		) AS PartiallyAppliedPaysQ  "
			"	ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID)  "
			"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID)  "
			"	LEFT JOIN (ChargesT LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN  "
			"	(PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
			"	LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number)  "
			"	LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	WHERE PaymentsT.ID Is Not Null AND LineItemT.Deleted=0 AND LineItemT_1.ID Is Not Null AND AppliesT.PointsToPayments=0 AND CPTCodeT.ID IS NOT NULL  "
			"	GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
			"	CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"	LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount,  "
			"	dbo.GetChargeTotal(ChargesT.ID),  "
			"	LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode,  "
			"	PatientsT.UserDefinedID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.ID, LineItemT_1.Date, "
			"	AdjustmentGroupCodesT.Code, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description), "
			"	AdjustmentReasonCodesT.Code, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) "
			"	HAVING LineItemT.Type=2  "
			"	) AS AdjustmentsByCPTCodeFullQ  "
			"LEFT JOIN  "
			"	(SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID  "
			"	FROM CategoriesT  "
			"	WHERE CategoriesT.Parent=0 "
			" "
			"	UNION  "
			"	SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID  "
			"	FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent  "
			"	WHERE SubCategoriesT.Parent<>0 "
			"	) AS CategoriesQ  "
			"ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID))");
		break;

	case 428: {
			//Past Deposit Slips
			/*	Version History
				DRT 4/19/2004 - PLID 11382 - Added info for Tips
				JMM 5/18/2004 - PLID 12412 - Changed Tips to only separate them when they have a different payment type than the payment
				JMM 11/3/2005 - PLID 13548 - Added ability for refunds
				JMJ 10/11/2006 - PLID 22955 - Supported Adjusted Batch Payments by ignoring them
				JMJ 11/13/2006 - PLID 23428 - Fixed Batch Payment deposit date
				// (j.gruber 2007-05-01 17:18) - PLID 25745 - only show the last 4 digits of the ccnumber
				(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
				// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
				// (j.gruber 2010-10-22 15:30) - PLID 31187 - fix cash tips
				// (j.jones 2011-09-15 16:48) - PLID 45202 - we now hide void and corrected payments, only showing the originals
				// (j.jones 2012-04-19 14:23) - PLID 48032 - changed to use PaymentsT.BatchPaymentID
				// (b.spivey, July 24, 2012) - PLID 44450 - Added InputDate
				// (r.gonet 2015-05-01 15:26) - PLID 65870 - Do not include Gift Certificate Refunds.
			*/
			CString str;
			CString strRefunds, strBatchRefunds;
			if (GetRemotePropertyInt("BankingIncludeRefunds", 1, 0, GetCurrentUserName(), true)) {
				strRefunds = "OR (LineItemT.Type = 3 AND PaymentsT.PayMethod IN (7,8,9))";
				strBatchRefunds = "Type <> 2 AND ";
			}
			else {
				strRefunds = "";
				strBatchRefunds = "Type = 1 AND ";
			}

			BOOL bIncludeTips = GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true);

			str.Format("SELECT PatID AS PatID, UserDefinedID, Patname, CheckNo, CCType, CASE WHEN Len(CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(CCNumber, 4) END  as CCNumber, Amount,  "
			"Paymethod, BankName, BankRoutingNum, CheckAcctNo, dbo.MaskSSN(SocialSecurity, %s) AS SocialSecurity, TDate as TDate, InputDate  "
			"FROM   "
			"	(SELECT * FROM  "
			"		(SELECT /*Payment Info*/ PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,   "
			"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PaymentPlansT.CheckNo, CreditCardNamesT.CardName AS CCType,   "
			"		PaymentPlansT.CCNumber, LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PaymentTipsT.PayMethod = PaymentsT.PayMethod AND PaymentTipsT.Paymethod <> 1) AS Amount, "
			"       PaymentsT.PayMethod,  "
			"		PaymentPlansT.BankNo AS BankName, PaymentPlansT.BankRoutingNum, PaymentPlansT.CheckAcctNo, PersonT.SocialSecurity,   "
			"		PaymentsT.DepositDate AS TDate, "
			"		LineItemT.InputDate AS InputDate " 
			"		FROM PatientsT LEFT OUTER JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID   "
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT OUTER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"		INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"		LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"		LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"		WHERE (PatientsT.PersonID > 0)  AND (PaymentsT.Deposited = 1)  "
			"		AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
			"		AND (LineItemT.Deleted = 0) AND ((LineItemT.Type = 1) %s)  "
			"		AND Amount <> 0 AND PaymentsT.BatchPaymentID Is Null AND PaymentsT.PayMethod NOT IN (4,10) "
			"		UNION ALL  "
			"		SELECT /*BatchPaymentInfo */ NULL AS PatID, NULL AS UserDefinedID, Name,   "
			"		CheckNo, NULL AS CCType, NULL AS CCNumber, CASE WHEN Type <> 1 THEN -1 * Amount ELSE Amount END, CASE WHEN Type = 1 THEN 2 WHEN Type = 3 THEN 8 ELSE 0 END AS PayMethod, BankName, BankRoutingNum, CheckAcctNo, NULL AS SocialSecurity,  "
			"		DepositDate, NULL AS InputDate "
			"		FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID   "
			"		WHERE Deleted = 0 AND Deposited = 1 AND %s  Amount <> 0 "
			"		UNION ALL  "
			"		SELECT /*Tips*/PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + '  (Tip)' AS PatName,  "
			"		NULL AS CheckNo, NULL AS CCType, NULL AS CCNumber, PaymentTipsT.Amount, "
			"		PaymentTipsT.PayMethod, "
			"		NULL AS BankName,  "
			"		NULL AS BankRoutingNum, NULL AS CheckAcctNo, PersonT.SocialSecurity, PaymentTipsT.DepositDate, "
			"		LineItemT.InputDate AS InputDate	" 
			"		FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID  "
			"		INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
			"		INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"		INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
			"		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"		LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID  "
			"		LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"		LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"		WHERE PaymentTipsT.Deposited = 1 AND Deleted = 0 "
			"		AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
			"		AND (PaymentTipsT.Paymethod = 1 OR PaymentsT.PayMethod <> PaymentTipsT.PayMethod) %s  "
			"		) SubQ "
			"	) DepositSlip", ((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
			strRefunds, strBatchRefunds, bIncludeTips ? "" : " AND PaymentTipsT.ID = -1"); 
			return str;
			  }
		break;

		//Strange goings on...
		//This was originally entered as case 262, but with the // Insurance Procedures tag... the one below here was entered as the query for 252.  These were wrong.  
		//I don't know what the 252 below is for, but I'm leaving it in the really off case someone finds a need for it (possibly misnumbered?)
	case 252:
		//Insurance Procedures
		/*	Version History
			DRT 8/26/2004 - PLID 13974 - This report does NOT include redeemed gift certificates.  But since it's an insurance report, it shouldn't matter.
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT (CASE WHEN LineItemT.Type = 1 THEN LineItemT.Amount ELSE 0 END) AS PayAmt,  "
			"(CASE WHEN LineItemT.Type = 2 THEN LineItemT.Amount ELSE 0 END) AS AdjAmt,  "
			"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, ProcedureT.Name AS ProcName, LineItemT1.Date AS TDate,  "
			"LineItemT1.InputDate AS IDate, LineItemT.PatientID AS PatID, LineItemT1.LocationID AS LocID, ChargesT.DoctorsProviders AS ProvID, ProcedureT.ID AS ProcID  "
			"FROM "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"	LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"	INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"	LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID"
			" "
			"WHERE (AppliesT.SourceID Is Not Null) AND (PaymentsT.InsuredPartyID > 0) AND (LineItemT.Deleted = 0) AND (ProcedureT.ID Is Not Null) AND (PaymentsT.PayMethod NOT IN (4,10))");
		break;

		
/*				THIS IS BAD!  I don't know where it came from, but it is NOT Insurance Procedures.
	case 252:
		//Insurance Procedures
		return _T("SELECT LineItemT.Description AS PayDesc, AppliesT.Amount AS ApplyAmt,  "
			"Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)),2) AS ChargeAmt,  "
			"LineItemT1.Description AS ChargeDesc, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID,  "
			"PatientsT.PersonID AS PatID, CASE WHEN ProcedureT.Name IS NULL THEN '[Other]' ELSE ProcedureT.Name END AS ProcName, ServiceT.Name AS ServiceName, LineItemT1.Date AS TDate,  "
			"LineItemT1.InputDate AS IDate, ChargesT.DoctorsProviders AS ProvID, InsuranceCoT.Name AS InsCoName,  "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, LineItemT1.LocationID AS LocID, ProcedureT.ID AS ProcedureID "
			"FROM "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"	LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"	LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID "
			"	INNER JOIN ChargesT ON LineItemT1.ID = ChargesT.ID "
			"	LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"	INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"	LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.InsuredPartyID > 0) AND (AppliesT.PointsToPayments = 0)");
*/
/*		return _T(
			"SELECT "
				"LineItemT.Description AS PayDesc, "
				"(CASE PaymentsT.PayMethod "
					"WHEN 1 THEN AppliesT.Amount "
					"WHEN 2 THEN AppliesT.Amount "
					"WHEN 3 THEN AppliesT.Amount "
					"WHEN 7 THEN 0 - AppliesT.Amount "
					"WHEN 8 THEN 0 - AppliesT.Amount "
					"WHEN 9 THEN 0 - AppliesT.Amount "
					"ELSE 0 END) AS PayAmt, "
				"(CASE PaymentsT.PayMethod "
					"WHEN 6 THEN AppliesT.Amount "
					"ELSE 0 END) AS AdjAmt, "
				"LineItemT1.Description AS ChargeDesc, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"PatientsT.UserDefinedID, "
				"PatientsT.PersonID AS PatID, "
				"CASE WHEN ProcedureT.Name IS NULL THEN 'No Procedure' ELSE ProcedureT.Name END AS ProcName, "
				"ProcedureT.ID AS ProcedureID, "
				"ServiceT.Name AS ServiceName, "
				"LineItemT1.Date AS TDate, "
				"LineItemT1.InputDate AS IDate, "
				"ChargesT.DoctorsProviders AS ProvID, "
				"InsuranceCoT.Name AS InsCoName, "
				"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
				"LineItemT1.LocationID AS LocID "
		"FROM "
		"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID "
			"INNER JOIN ChargesT ON LineItemT1.ID = ChargesT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number " 
			"LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID " 
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID " 
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID " 
			"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "

		"WHERE LineItemT.Deleted = 0"
			"AND PaymentsT.InsuredPartyID > 0"
			"AND AppliesT.PointsToPayments = 0");
			break;*/

	case 175:
		//Adjustments By Service Category
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			TES 5/24/04 - Added ChargeDescription as editable field.
			// (j.gruber 2007-05-29 16:37) - PLID 24837 - added reason code and group code
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2010-09-24 16:42) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.goldschmidt 2014-01-28 14:55) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT AdjustmentsByCPTCategoryFullQ.ProvID AS ProvID,  "
		"AdjustmentsByCPTCategoryFullQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First AS DocName,  "
		"AdjustmentsByCPTCategoryFullQ.TDate AS TDate,  "
		"AdjustmentsByCPTCategoryFullQ.PayMethod,  "
		"AdjustmentsByCPTCategoryFullQ.CCType,  "
		"AdjustmentsByCPTCategoryFullQ.CheckNo,  "
		"AdjustmentsByCPTCategoryFullQ.Description,  "
		"AdjustmentsByCPTCategoryFullQ.ApplyAmount,  "
		"CategoriesQ.Category AS Category,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"AdjustmentsByCPTCategoryFullQ.PatID AS PatID,  "
		"AdjustmentsByCPTCategoryFullQ.UserDefinedID, "
		"AdjustmentsByCPTCategoryFullQ.IDate AS IDate,  "
		"AdjustmentsByCPTCategoryFullQ.Type, "
		"AdjustmentsByCPTCategoryFullQ.LocID AS LocID, "
		"AdjustmentsByCPTCategoryFullQ.Location, CatFilterID AS CatFilterID, "
		"AdjustmentsByCPTCategoryFullQ.ItemDesc AS ChargeDescription, "
		"AdjustmentsByCPTCategoryFullQ.ReasonCode, "
		"AdjustmentsByCPTCategoryFullQ.GroupCode, "
		"AdjustmentsByCPTCategoryFullQ.ReasonCodeDesc, "
		"AdjustmentsByCPTCategoryFullQ.GroupCodeDesc "
		"FROM ((((SELECT LineItemT.ID,  "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
		"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ChargesT.ItemCode AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"PaymentPlansT.CheckNo,  "
		"ServiceT.Category,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"LineItemT.Type, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, ServiceT.Category AS CatFilterID, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) "
		"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), "
		"LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ServiceT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, PatientsT.UserDefinedID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, "
		"AdjustmentGroupCodesT.Code, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description), "
		"AdjustmentReasonCodesT.Code, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) "
		"HAVING (((LineItemT.Type)=2)) "
		") AS AdjustmentsByCPTCategoryFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON AdjustmentsByCPTCategoryFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCategoryFullQ.ProvID = PersonT.ID))");
		break;
	
	
	
	case 176:
		//Adjustments (Split by Product/Service)
		/*	Version History
			TES 7/27/2004 - PLID 13695 - Changed the PayAmount field to actually be the PayAmount (it was the Charge amount!)
			DRT 4/10/2006 - PLID 11734 - Removed the ProcCode data field, added a calculated field that produces the same output (0 or 1)
			// (j.gruber 2007-05-29 16:25) - PLID 24837 - adding group and reason codes
			// (j.jones 2010-09-24 16:42) - PLID 40650 - fixed group & reason codes to have descriptions in the query
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, LineItemT1.Description,  "
		"    LineItemT1.Date AS TDate, "
		"	 CASE WHEN CPTCodeT.ID IS NOT NULL THEN 1 "
		"		WHEN ProductT.ID IS NOT NULL THEN 0 ELSE NULL END AS ProcCode,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, "
		"     DocInfo.Last + ', ' + DocInfo.First + ' ' + DocInfo.Middle AS DocName, "
		"     LineItemT.Amount AS PayAmount,  "
		"    LineItemT1.InputDate AS IDate, PaymentsT.ID AS PayID,  "
		"    ProvidersT.PersonID AS ProvID, AppliesT.Amount,  "
		"    LineItemT1.ID AS LineID, AppliesT.ID AS ApplyID, "
		"	 LineItemT1.LocationID AS LocID, "
		"	 LocationsT.Name AS Location, "
		"	 AdjustmentGroupCodesT.Code AS GroupCode, AdjustmentGroupCodesT.Description AS GroupCodeDesc, "
		"	 AdjustmentReasonCodesT.Code AS ReasonCode, AdjustmentReasonCodesT.Description AS ReasonCodeDesc "
		"FROM PatientsT LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    PatientsT.PersonID = PersonT1.ID RIGHT OUTER JOIN "
		"    LineItemT RIGHT OUTER JOIN "
		"    PaymentsT ON  "
		"    LineItemT.ID = PaymentsT.ID RIGHT OUTER JOIN "
		"    AppliesT LEFT OUTER JOIN "
		"    ChargesT LEFT OUTER JOIN "
		"    (LineItemT LineItemT1 LEFT JOIN LocationsT ON LineItemT1.LocationID = LocationsT.ID) ON  "
		"    ChargesT.ID = LineItemT1.ID LEFT OUTER JOIN "
		"    ProvidersT LEFT OUTER JOIN "
		"    PersonT DocInfo ON ProvidersT.PersonID = DocInfo.ID ON  "
		"    ChargesT.DoctorsProviders = ProvidersT.PersonID ON  "
		"    AppliesT.DestID = ChargesT.ID ON  "
		"    PaymentsT.ID = AppliesT.SourceID ON  "
		"    PatientsT.PersonID = LineItemT1.PatientID "
		"	 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"	 LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"	 LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"	 LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
		"	 LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
		"WHERE (LineItemT.Type = 2) AND  "
		"    (AppliesT.PointsToPayments = 0) AND  "
		"    (LineItemT1.Deleted = 0)");
		break;
	

	case 120: {

		//Adjustments
		/*	Version History
			TES 3/13/03: Changed to use charge location if applied.
			TES 6/16/2005 - PLID 15947 - Field 'Location' was pulling from the deepest subquery, meaning that it
				was not correct if the adjustment was applied.  Now shows as the location name of what we applied
				to, if anything.
			DRT 7/7/2005 - PLID 16578 - Fixed the provider filter.  It worked fine if you were applied to a charge, but if your 
				adjustment was applied to a payment, it always used the adjustment provider (should use the destination).
				// (j.gruber 2007-05-29 16:24) - PLID 24837 - adding grop and reason codes
			// (j.jones 2008-06-30 16:09) - PLID 27647 - supported passing in a filter of Adjustment IDs
			// (j.jones 2010-09-24 16:42) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			TES 3/9/2011 - PLID 42195 - Added GroupID and Category
		*/

		CString strSql = "SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID AS PatID,   "
			"AdjustmentsSubQ.ProvID AS ProvID, SUM(AdjustmentsSubQ.Amount) AS Amount, MIN(AdjustmentsSubQ.RandomText) AS RandomText,  "
			"AdjustmentsSubQ.PatName, AdjustmentsSubQ.IDate AS IDate, AdjustmentsSubQ.TDate AS TDate, AdjustmentsSubQ.PayMethod,  "
			"AdjustmentsSubQ.Description, MIN(AdjustmentsSubQ.ApplyID) AS ApplyID,  MIN(AdjustmentsSubQ.LineID) AS LineID,   "
			"AdjustmentsSubQ.Type, AdjustmentsSubQ.UserDefinedID, AdjustmentsSubQ.LocID AS LocID, LocationsT.Name AS Location,  "
			"AdjustmentsSubQ.ReasonCode, "
			"AdjustmentsSubQ.GroupCode, "
			"AdjustmentsSubQ.ReasonCodeDesc, "
			"AdjustmentsSubQ.GroupCodeDesc, "
			"AdjustmentsSubQ.PaymentGroupID AS GroupID, AdjustmentsSubQ.GroupName AS Category "
			"FROM  "
			"	(SELECT *  "
			"	FROM  "
			"		(SELECT LineItemT.ID, LineItemT.PatientID AS PatID,  ProvID =  CASE WHEN ChargesT.DoctorsProviders is Null then  "
			"		CASE WHEN PayDestT.ProviderID IS NULL THEN PaymentsT.ProviderID ELSE PayDestT.ProviderID END ELSE ChargesT.DoctorsProviders END,   "
			"		Amount =  CASE  WHEN _PartiallyAppliedPaysQ.ID is NULL then   "
			"		CASE WHEN LineItemT1.ID is Null then LineItemT.Amount Else AppliesT.Amount End ELSE AppliesT.Amount END, 'FULL' AS RandomText,   "
			"		PatientInfo.Last + ', ' + PatientInfo.First + ' ' + PatientInfo.Middle AS PatName, LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate,  PaymentsT.PayMethod, LineItemT.Description,  AppliesT.ID AS ApplyID,  LineItemT.ID AS LineID,  "
			"		LineItemT.Type, PatientsT.UserDefinedID, CASE WHEN LineItemT1.LocationID Is Null THEN LineItemT.LocationID  "
			"		ELSE LineItemT1.LocationID END AS LocID, "
			"		AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
			"		AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc, "
			"		PaymentsT.PaymentGroupID, PaymentGroupsT.GroupName "
			"		FROM PatientsT  "
			"		LEFT OUTER JOIN PersonT AS PatientInfo ON  PatientsT.PersonID = PatientInfo.ID  "
			"		RIGHT OUTER JOIN  "
			"			(SELECT LineItemT_1.ID,  LineItemT_1.Amount AS PayAmt,  SUM(AppliesT.Amount) AS ApplyAmt,   "
			"			MIN([LineItemT_1].[Amount])  - SUM([AppliesT].[Amount]) AS Total,  LineItemT_1.PatientID  "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN  "
			"			LineItemT ON  AppliesT.DestID = LineItemT.ID) ON  PaymentsT.ID = AppliesT.SourceID) ON   "
			"			LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted) = 0))  "
			"			GROUP BY LineItemT_1.ID,  LineItemT_1.Amount,  LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) IS NOT NULL) AND   "
			"			((MIN([LineItemT_1].[Amount]) - SUM([AppliesT].[Amount])) <> 0)) "
			"			) _PartiallyAppliedPaysQ  "
			"		RIGHT OUTER JOIN LineItemT  ON  _PartiallyAppliedPaysQ.ID = LineItemT.ID  "
			"		LEFT OUTER JOIN ProvidersT LEFT OUTER JOIN PersonT DocInfo ON  ProvidersT.PersonID = DocInfo.ID  "
			"		RIGHT OUTER JOIN ChargesT ON  ProvidersT.PersonID = ChargesT.DoctorsProviders   "
			"		RIGHT OUTER JOIN LineItemT LineItemT1 ON  ChargesT.ID = LineItemT1.ID  "
			"		RIGHT OUTER JOIN AppliesT ON  LineItemT1.ID = AppliesT.DestID  "
			"		RIGHT OUTER JOIN PaymentsT ON  AppliesT.SourceID = PaymentsT.ID ON  LineItemT.ID = PaymentsT.ID ON   "
			"		PatientsT.PersonID = LineItemT.PatientID  "
			"		LEFT JOIN PaymentsT PayDestT ON LineItemT1.ID = PayDestT.ID "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
			"		LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"		WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 2) AND (PaymentsT.ID IS NOT NULL) "
			"		) AS AdjustmentsFullQ  "
			"	UNION  "
			" "
			"	SELECT *  "
			"	FROM  "
			"		(SELECT PartiallyAppliedPaysQ.ID,  LineItemT.PatientID AS PatID,  PaymentsT.ProviderID,  PartiallyAppliedPaysQ.Total AS Amount,   "
			"		'PART' AS RandomText,  PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate,  PaymentsT.PayMethod, LineItemT.Description,  0 AS ApplyID, LineItemT.ID AS LineID,  LineItemT.Type,  "
			"		PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, "
			"		AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
			"		AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc, "
			"		PaymentsT.PaymentGroupID, PaymentGroupsT.GroupName "
			"		FROM LineItemT INNER JOIN  "
			"			(SELECT LineItemT_1.ID,  LineItemT_1.Amount AS PayAmt,  SUM(AppliesT.Amount) AS ApplyAmt,   "
			"			MIN([LineItemT_1].[Amount]) - SUM([AppliesT].[Amount]) AS Total,  LineItemT_1.PatientID  "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON   "
			"			AppliesT.DestID = LineItemT.ID) ON  PaymentsT.ID = AppliesT.SourceID) ON  LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted) = 0))  "
			"			GROUP BY LineItemT_1.ID,  LineItemT_1.Amount,  LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) IS NOT NULL) AND ((MIN([LineItemT_1].[Amount]) - SUM([AppliesT].[Amount])) <> 0)) "
			"			) PartiallyAppliedPaysQ  "
			"		ON LineItemT.ID = PartiallyAppliedPaysQ.ID INNER JOIN PaymentsT ON   "
			"		LineItemT.ID = PaymentsT.ID INNER JOIN PatientsT ON  LineItemT.PatientID = PatientsT.PersonID INNER JOIN  "
			"		PersonT ON  PatientsT.PersonID = PersonT.ID  "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
			"		LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"		WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 2) "
			"		) AS AdjustmentsPartQ "
			"	) AdjustmentsSubQ  "
			"LEFT OUTER JOIN ProvidersT ON  AdjustmentsSubQ.ProvID = ProvidersT.PersonID  "
			"LEFT OUTER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
			"LEFT JOIN LocationsT ON AdjustmentsSubQ.LocID = LocationsT.ID  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID,  AdjustmentsSubQ.ProvID, AdjustmentsSubQ.PatName,   "
			"AdjustmentsSubQ.IDate, AdjustmentsSubQ.TDate,  AdjustmentsSubQ.PayMethod, AdjustmentsSubQ.Description,   "
			"AdjustmentsSubQ.Type, AdjustmentsSubQ.UserDefinedID, AdjustmentsSubQ.LocID, LocationsT.Name,"
			"AdjustmentsSubQ.ReasonCode, "
			"AdjustmentsSubQ.GroupCode, "
			"AdjustmentsSubQ.ReasonCodeDesc, "
			"AdjustmentsSubQ.GroupCodeDesc, "
			"AdjustmentsSubQ.PaymentGroupID, "
			"AdjustmentsSubQ.GroupName";

		// (j.jones 2008-06-30 16:09) - PLID 27647 - saExtraValues may contain
		// a filter for adjustment IDs if run from the Write Off Accounts screen
		if(saExtraValues.GetSize() > 0) {
			strSql += saExtraValues.GetAt(0);
		}

		return strSql;

		break;	
	}

	case 138://Payment Receipts
	case 235://Payment dialog
		/*	Version History
			DRT 7/15/03 - Added the charge Quantity field to the report, put it in before the charge total.
			JMM 3/2/04 -  added fax # of location, patient phone number, provider name, patient balance : not just the payment balance to the report PLID: 10874
			DRT 4/22/2004 - PLID 11816 - Removed a number of select *'s
			DRT 5/13/2004 - PLID 11667 - Added a subreport to show all tips.  Added a field to the main query to give the total tip amount (sum of
				all tips in the subreport).
			JMM 12/21/2004 - PLID 15037 - Fixed the patient balance query as it was negating refunds wrongly
			e.lally 2006-08-08 - PLID 21846 - Added payment category field.
			// (j.gruber 2007-02-20 12:43) - PLID 23409 - added Patient Coordinator to the Receipt
			// (j.gruber 2007-05-01 17:20) - PLID 25745 - only show the last 4 digits of the ccnumber
			// (j.gruber 2007-05-15 09:14) - PLID 25987 - Blank out CCExpDate
			// (j.gruber 2007-05-29 14:51) - PLID 25979 - added discount category
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2008-05-09 16:23) - PLID 25338 - removed CurrentSelect as a column
			// (j.gruber 2009-04-01 14:53) - PLID 33795 - updated for discount structure
			// (j.gruber 2009-12-18 13:03) - PLID 28609 - added gift certificate balance
			// (j.gruber 2010-05-19 11:59) - PLID 38765 - optimize the GC balance query
			// (j.dinatale 2011-09-30 15:22) - PLID 45773 - take into account quantity for GC calculations (quantity can be -1) 
			// (j.gruber 2014-03-28 13:57) - PLID 61460 - modified for ICD-10
			TES 3/20/2015 - PLID 65075 - Updated the GiftCertBal field to reflect gift certificate tips
			// (j.jones 2015-03-09 09:50) - PLID 65147 - if the description begins with 'Corrected Charge',
			// 'Corrected Payment', etc., strip that off
		*/
		switch(nSubLevel) {
			case 1:
			switch (nSubRepNum) {

				case 0: { //discounts
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription, DiscountTotal FROM (  "
							" 		SELECT ChargeDiscountsT.ID, ChargeDiscountsT.ChargeID, ChargesT.BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE  "
							" 		 CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE  "
							" 		 (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription, "
							" 		 Round(Convert(money,  "
							" 			((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)+(CASE WHEN([Discount] Is Null) THEN 0 ELSE [Discount] END)))  "
							" 			),2) AS DiscountTotal "
							" 		 FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
							" WHERE LineItemT.Deleted = 0 AND ChargeDiscountsT.DELETED = 0 ) Q"); 
						
							return _T(str);
						}
					break;
		
				//Tips subreport
				case 1:
					return _T("SELECT PaymentTipsT.ID, PaymentID, ProvID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName,  "
					"Amount,  "
					"CASE WHEN PayMethod = 1 THEN 'Cash' "
					"	WHEN PayMethod = 2 THEN 'Check' "
					"	WHEN PayMethod = 3 THEN 'Credit' "
					"	WHEN PayMethod = 4 THEN 'Gift Certificate' END AS PayMethodText "
					"FROM PaymentTipsT "
					"LEFT JOIN PersonT ON ProvID = PersonT.ID");
				break;
				default:
					return "";
				break;
			}
		break;
		default:
			// (j.dinatale 2011-09-30 15:18) - PLID 45773 - take into account financial corrections when calculating a gift card balance
			// (r.gonet 2015-06-04 10:52) - PLID 65657 - Updated to use the new Gift Certificates Balance query.
			return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
			"PaymentsT.ID AS PayID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT.Address1 AS PatAddress1,  "
			"PersonT.Address2 AS PatAddress2,  "
			"PersonT.City AS PatCity,  "
			"PersonT.State AS PatState,  "
			"PersonT.Zip AS PatZip,  "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS Employee,  "
			"PersonT.HomePhone as PatHomePhone, "
			"PersonT.WorkPhone As PatWorkPhone, "
			"PersonT.CellPhone as PatCellPhone, "
			"LocationsT.Name,  "
			"LocationsT.Address1 AS LocAddress1,  "
			"LocationsT.Address2 AS LocAddress2,  "
			"LocationsT.State AS LocState,  "
			"LocationsT.City AS LocCity,  "
			"LocationsT.Zip AS LocZip,  "
			"LocationsT.Phone,  "
			"LocationsT.Phone2, "
			"LocationsT.Fax as LocFax, "
			"LocationsT.ID AS LocID,  "
			"AppliesT.Amount AS ApplyAmount,  "
			"ChargesT.ItemCode,  "
			"(CASE WHEN LineItemCorrectionsT_VoidedCharge.VoidingLineItemID Is Not Null "
			"	AND Left(LineItemT_1.Description, Len('Voided Charge')) = 'Voided Charge' THEN LTRIM(Right(LineItemT_1.Description, Len(LineItemT_1.Description) - Len('Voided Charge'))) "
			" WHEN LineItemCorrectionsT_NewCharge.NewLineItemID Is Not Null "
			"	AND Left(LineItemT_1.Description, Len('Corrected Charge')) = 'Corrected Charge' THEN LTRIM(Right(LineItemT_1.Description, Len(LineItemT_1.Description) - Len('Corrected Charge'))) "
			"ELSE LineItemT_1.Description END) AS ItemDesc, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
			"LineItemT_1.Date AS ChargeDate,  "
			"PaymentsT.ID, PaymentsT.ProviderID, PaymentsT.PaymentGroupID, PaymentsT.InsuredPartyID, PaymentsT.PaymentGroup, PaymentsT.Deposited, PaymentsT.DepositDate, PaymentsT.PrePayment, PaymentsT.PayMethod, PaymentsT.BatchPaymentID, PaymentsT.QuickbooksID, PaymentsT.DepositInputDate, PaymentsT.CashReceived, PaymentsT.PaymentUniqueID, PaymentsT.QuoteID,  "
			"(CASE WHEN PaymentsT.PayMethod = 1 AND PaymentsT.CashReceived Is Not Null THEN PaymentsT.CashReceived - LineItemT.Amount ELSE NULL END) AS ChangeGiven, "
			"PaymentPlansT.ID, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, CreditCardNamesT.CardName AS CCType, CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END  as CCNumber, PaymentPlansT.CCHoldersName, Convert(DateTime, NULL) AS CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, "
			"LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, "
			" (CASE WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 1 "
			"	AND Left(LineItemT.Description, Len('Voided Payment')) = 'Voided Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Payment'))) "
			"  WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Voided Adjustment')) = 'Voided Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Adjustment'))) "
			"  WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 3 "
			"	AND Left(LineItemT.Description, Len('Voided Refund')) = 'Voided Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Refund'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
			"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
			"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
			"  WHEN LineItemCorrectionsBalancingAdjT.BalancingAdjID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Voided Charge Adjustment')) = 'Voided Charge Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Charge Adjustment'))) "
			"ELSE LineItemT.Description END) AS Description, "
			"LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, LineItemT.LocationID, LineItemT.GiftID, LineItemT.DrawerID,  "
			"PersonT.First AS [First Name],  "
			"LineItemT.InputDate AS IDate,  "

			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "

			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "

			"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
			"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
			"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
			"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "

			"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
			"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
			"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
			"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "

			"ChargeWhichCodesListV.WhichCodes9, \r\n"
			"ChargeWhichCodesListV.WhichCodes10, \r\n"
			"ChargeWhichCodesListV.WhichCodesBoth, \r\n"

			"LineItemT.Date AS TDate, "
			"ProvidersT.[Fed Employer ID] AS ProviderTaxID, "
			" (ChargesT.TaxRate - 1) AS TaxRate, "
			" (ChargesT.TaxRate2 - 1) AS TaxRate2, ChargesT.Quantity, "
			" Round(Convert(money,(((LineItemT_1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))),2) AS ChargeWithoutTax,  "
			" (SELECT Min(StartTime) FROM AppointmentsT WHERE Status <> 4 AND Date > getdate() AND PatientID = PersonT.ID) AS NextAppt, "
			" POS.Name AS POSName, PlaceofServiceCodesT.PlaceCodes AS POS, "
			" COALESCE(TotalPercentOff, 0) as PercentOff, COALESCE(TotalDiscount, convert(money,0)) as Discount,  "
			" PayProvT.First As PayProvFirst, PayProvT.Middle As PayProvMiddle, PayProvT.Last As PayProvLast, PatBalanceQ.PatBalance, CASE WHEN TipSubQ.TotalTipAmount IS NULL THEN 0 ELSE TipSubQ.TotalTipAmount END AS TotalTipAmount, "
			"PaymentGroupsT.GroupName AS PayCategory, "
			" PatCoordT.First As PatCoordFirst, PatCoordT.Middle As PatCoordMiddle, PatCoordT.Last AS PatCoordLast, "
			" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription, ChargesT.ID As ChargeID,  "
			" GiftCertificatesQ.Balance AS GiftCertBal "

			"FROM PaymentsT LEFT OUTER JOIN "
			"    LineItemT ON PaymentsT.ID = LineItemT.ID "
			"    LEFT JOIN (SELECT PaymentID, SUM(Amount) AS TotalTipAmount FROM PaymentTipsT GROUP BY PaymentID) TipSubQ ON PaymentsT.ID = TipSubQ.PaymentID "
			"    LEFT OUTER JOIN "
			"    LocationsT ON LineItemT.LocationID = LocationsT.ID RIGHT OUTER JOIN "
			"    PersonT INNER JOIN "
			"    (PatientsT LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID) ON PersonT.ID = PatientsT.PersonID ON  "
			"    LineItemT.PatientID = PatientsT.PersonID "
			"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			"    LEFT OUTER JOIN "
			"    AppliesT ON  "
			"    PaymentsT.ID = AppliesT.SourceID LEFT OUTER JOIN "
			"    ChargesT ON  "
			"    AppliesT.DestID = ChargesT.ID LEFT OUTER JOIN "			
			"    BillsT ON ChargesT.BillID = BillsT.ID "
			"	 LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			"    LEFT OUTER JOIN UsersT INNER JOIN "
			"    PersonT PersonT_1 ON  "
			"    UsersT.PersonID = PersonT_1.ID ON  "
			"    PatientsT.EmployeeID = UsersT.PersonID LEFT OUTER JOIN "
			"    ContactsT INNER JOIN "
			"    PersonT PersonT_2 ON  "
			"    ContactsT.PersonID = PersonT_2.ID ON  "
			"    UsersT.PersonID = ContactsT.PersonID LEFT OUTER JOIN "
			"    LineItemT LineItemT_1 ON  "
			"    ChargesT.ID = LineItemT_1.ID LEFT OUTER JOIN "
			"    PaymentPlansT ON  "
			"    PaymentsT.ID = PaymentPlansT.ID LEFT OUTER JOIN "
			"	 CreditCardNamesT ON "
			"	 PaymentPlansT.CreditCardID = CreditCardNamesT.ID "

			"	 LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"	 LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"	 LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"	 LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

			"	 LEFT JOIN ChargeWhichCodesListV ON ChargesT.ID = ChargeWhichCodesListV.ChargeID \r\n "

			"	 LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_VoidedPay ON PaymentsT.ID = LineItemCorrectionsT_VoidedPay.VoidingLineItemID "
			"	 LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
			"	 LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_VoidedCharge ON ChargesT.ID = LineItemCorrectionsT_VoidedCharge.VoidingLineItemID "
			"	 LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID "
			"	 LEFT JOIN LineItemCorrectionsBalancingAdjT ON PaymentsT.ID = LineItemCorrectionsBalancingAdjT.BalancingAdjID "

			"    LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"	 LEFT JOIN PlaceofServiceCodesT ON POS.POSID = PlaceOfServiceCodesT.ID "
			"	 LEFT JOIN PersonT PayProvT ON PaymentsT.ProviderID = PayProvT.ID "
			"	 LEFT JOIN (SELECT SUM (CASE WHEN LineItemT.Type = 10 THEN CASE WHEN ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1 THEN ChargeRespT.Amount ELSE 0 END ELSE 0 END) -  "
			"    SUM (CASE WHEN LineItemT.Type < 10 THEN CASE WHEN PaymentsT.InsuredPartyID = -1  THEN LineItemT.Amount ELSE 0 END ELSE 0 END) AS PatBalance, "
			"    LineItemT.PatientID "
			"    FROM "
			"    LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"    LEFT JOIN ChargeRespT ON LineItemT.ID = ChargeRespT.ChargeID "
			"    WHERE LineItemT.Deleted = 0  "
			"    Group By PatientID) AS PatBalanceQ ON PatientsT.PersonID = PatBalanceQ.PatientID " 
			"    LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	 LEFT JOIN ( "
			+ GetGiftCertificateValueQuery() +
			"	 ) GiftCertificatesQ ON GiftCertificatesQ.ID = LineItemT.GiftID "
			"    WHERE (LineItemT.Deleted = 0) ");
			break;
			}
		break;
	case 139:
		//Payments (Split By Product/Service)
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			DRT 3/15/2004 - PLID 11436 - Made report editable.
			DRT 8/26/2004 - PLID 13974 - Modified to only show applies to services or products.  This query was still following
				the long obsolete ProcCode field, which was being interfered with by gift certificates.
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"ChargesT.DoctorsProviders AS ProvID,  "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName,  "
			"LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, LineItemT.Description, "
			"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 1 "
			"	  WHEN ProductT.ID IS NOT NULL THEN 0 ELSE NULL END AS ProcCode,  "
			"Sum(AppliesT.Amount) AS SumOfAmount,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
			"ChargesT.ID, LineItemT1.LocationID AS LocID "
			"FROM  "
			"AppliesT LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
			"LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN LocationsT ON LineItemT1.LocationID = LocationsT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
			"WHERE (LineItemT.Type = 1) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND (CPTCodeT.ID IS NOT NULL OR ProductT.ID IS NOT NULL) "
			"GROUP BY  "
			"PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"ChargesT.DoctorsProviders,  "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"LineItemT.Date, LineItemT.InputDate, LineItemT.Description, "
			"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 1 "
			"	  WHEN ProductT.ID IS NOT NULL THEN 0 ELSE NULL END, "
			"dbo.GetChargeTotal(ChargesT.ID),  "
			"ChargesT.ID, LineItemT1.LocationID ");
		break;
	case 140: 
		//Payments / Refunds
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(e.lally 2007-07-16) PLID 26132 - Added credit card names to the list of available fields
			(r.goldschmidt 2014-01-29 10:53) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsRefundsSubQ.ID,  "
		"    Sum(PaymentsRefundsSubQ.Amount) as Amount,  "
		"    PaymentsRefundsSubQ.ProvID AS ProvID,  "
		"    Min(PaymentsRefundsSubQ.RandomText) AS RandText,  "
		"    PaymentsRefundsSubQ.PatID AS PatID,  "
		"    PaymentsRefundsSubQ.PatName,  "
		"    PaymentsRefundsSubQ.IDate AS IDate, PaymentsRefundsSubQ.TDate AS TDate,  "
		"    PaymentsRefundsSubQ.PayMethod AS PayMethod,  "
		"    PaymentsRefundsSubQ.Description,  "
		"    Min(PaymentsRefundsSubQ.ApplyID) as ApplyId,  "
		"    Min(PaymentsRefundsSubQ.LineID) as LineID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsRefundsSubQ.UserDefinedID, PaymentsRefundsSubQ.LocID AS LocID, PaymentsRefundsSubQ.Location, "
		"PaymentsRefundsSubQ.CreditCardName "
		"FROM (SELECT * "
		"      FROM (SELECT LineItemT.ID,  "
		"                PatientsT.PersonID AS PatID,  "
		"              ProvID =  "
		"	CASE "
		"		WHEN ChargesT.DoctorsProviders is Null then "
		"			PaymentsT.ProviderID "
		"		ELSE ChargesT.DoctorsProviders "
		"	END, "
		"                Amount =  "
		"	CASE  "
		"		WHEN _PartiallyAppliedPaysQ.ID is NULL then  "
		"		CASE "
		"			WHEN LineItemT1.ID is Null then LineItemT.Amount "
		"			Else AppliesT.Amount "
		"		End "
		"		ELSE AppliesT.Amount "
		"	End, "
		"		 'FULL' AS RandomText, PatientInfo.Last + ', ' + PatientInfo.First + ' ' + PatientInfo.Middle "
		"                 AS PatName, LineItemT.InputDate AS IDate,  "
		"                LineItemT.Date AS TDate,  "
		"                PaymentsT.PayMethod, LineItemT.Description,  "
		"                AppliesT.ID AS ApplyID,  "
		"                LineItemT.ID AS LineID,  "
		"                LineItemT.Type, PatientsT.UserDefinedID, CASE WHEN LineItemT1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT1.LocationID END AS LocID, "
		"				 CASE WHEN LineItemT1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location,  "
		"				 CreditCardNamesT.CardName AS CreditCardName "
		"FROM PatientsT LEFT OUTER JOIN "
		"                PersonT AS PatientInfo ON  "
		"                PatientsT.PersonID = PatientInfo.ID RIGHT OUTER "
		"                 JOIN "
		"                    (SELECT LineItemT_1.ID,  "
		"                       LineItemT_1.Amount AS PayAmt,  "
		"                       SUM(AppliesT.Amount) AS ApplyAmt,  "
		"                       MIN([LineItemT_1].[Amount])  "
		"                       - SUM([AppliesT].[Amount]) AS Total,  "
		"                       LineItemT_1.PatientID "
		"                  FROM LineItemT AS LineItemT_1 LEFT JOIN "
		"                       (PaymentsT LEFT JOIN "
		"                       (AppliesT LEFT JOIN "
		"                       LineItemT ON  "
		"                       AppliesT.DestID = LineItemT.ID) ON  "
		"                       PaymentsT.ID = AppliesT.SourceID) ON  "
		"                       LineItemT_1.ID = PaymentsT.ID "
		"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"                  GROUP BY LineItemT_1.ID,  "
		"                       LineItemT_1.Amount,  "
		"                       LineItemT_1.PatientID "
		"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND  "
		"                       ((MIN([LineItemT_1].[Amount])  "
		"                       - SUM([AppliesT].[Amount])) <> 0)))  "
		"                _PartiallyAppliedPaysQ RIGHT OUTER JOIN "
		"                (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON  "
		"                _PartiallyAppliedPaysQ.ID = LineItemT.ID LEFT OUTER "
		"                 JOIN "
		"                ProvidersT LEFT OUTER JOIN "
		"                PersonT DocInfo ON  "
		"                ProvidersT.PersonID = DocInfo.ID RIGHT OUTER JOIN "
		"                ChargesT ON  "
		"                ProvidersT.PersonID = ChargesT.DoctorsProviders "
		"                 RIGHT OUTER JOIN "
		"                (LineItemT LineItemT1 LEFT JOIN LocationsT AS ChargeLoc ON LineItemT1.LocationID = ChargeLoc.ID) ON  "
		"                ChargesT.ID = LineItemT1.ID RIGHT OUTER JOIN "
		"                AppliesT ON  "
		"                LineItemT1.ID = AppliesT.DestID RIGHT OUTER JOIN "
		"                PaymentsT ON  "
		"                AppliesT.SourceID = PaymentsT.ID ON  "
		"                LineItemT.ID = PaymentsT.ID ON  "
		"                PatientsT.PersonID = LineItemT.PatientID "
		"				LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"				LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE(LineItemT.Deleted "
		"                 = 0) AND (LineItemT.Type = 1 OR "
		"                LineItemT.Type = 3) AND (PaymentsT.PayMethod NOT IN (4,10)))  "
		"          AS PaymentsRefundsFullQ "
		"      UNION "
		"      SELECT * "
		"      FROM (SELECT PartiallyAppliedPaysQ.ID,  "
		"		PatientsT.PersonID AS PatID,                   "
		"                PaymentsT.ProviderID AS ProvID,  "
		"                PartiallyAppliedPaysQ.Total AS Amount,  "
		"                'PART' AS RandomText, "
		"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
		"                 AS PatName, LineItemT.InputDate AS IDate,  "
		"                LineItemT.Date AS TDate,  "
		"                PaymentsT.PayMethod, LineItemT.Description,  "
		"                0 AS ApplyID, LineItemT.ID AS LineID,  "
		"                LineItemT.Type, PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
		"				 CreditCardNamesT.CardName AS CreditCardName "
		" FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN "
		"                    (SELECT LineItemT_1.ID,  "
		"                       LineItemT_1.Amount AS PayAmt,  "
		"                       SUM(AppliesT.Amount) AS ApplyAmt,  "
		"                       MIN([LineItemT_1].[Amount])  "
		"                       - SUM([AppliesT].[Amount]) AS Total,  "
		"                       LineItemT_1.PatientID "
		"                  FROM LineItemT AS LineItemT_1 LEFT JOIN "
		"                       (PaymentsT LEFT JOIN "
		"                       (AppliesT LEFT JOIN "
		"                       LineItemT ON  "
		"                       AppliesT.DestID = LineItemT.ID) ON  "
		"                       PaymentsT.ID = AppliesT.SourceID) ON  "
		"                       LineItemT_1.ID = PaymentsT.ID "
		"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"                  GROUP BY LineItemT_1.ID,  "
		"                       LineItemT_1.Amount,  "
		"                       LineItemT_1.PatientID "
		"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND  "
		"                       ((MIN([LineItemT_1].[Amount])  "
		"                       - SUM([AppliesT].[Amount])) <> 0)))  "
		"                PartiallyAppliedPaysQ ON  "
		"                LineItemT.ID = PartiallyAppliedPaysQ.ID INNER JOIN "
		"                PaymentsT ON  "
		"                LineItemT.ID = PaymentsT.ID INNER JOIN "
		"                PatientsT ON  "
		"                LineItemT.PatientID = PatientsT.PersonID INNER JOIN "
		"                PersonT ON  "
		"                PatientsT.PersonID = PersonT.ID "
		"				LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"				LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE(LineItemT.Deleted "
		"                 = 0) AND (LineItemT.Type = 1 OR "
		"                LineItemT.Type = 3) AND (PaymentsT.PayMethod NOT IN (4,10)))  "
		"          AS PaymentsRefundsPartQ)  "
		"    PaymentsRefundsSubQ LEFT OUTER JOIN "
		"    ProvidersT ON  "
		"    PaymentsRefundsSubQ.ProvID = ProvidersT.PersonID LEFT OUTER "
		"     JOIN "
		"    PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsRefundsSubQ.ID,  "
		"    PaymentsRefundsSubQ.ProvID,  "
		"    PaymentsRefundsSubQ.PatID,  "
		"    PaymentsRefundsSubQ.PatName,  "
		"    PaymentsRefundsSubQ.IDate, PaymentsRefundsSubQ.TDate,  "
		"    PaymentsRefundsSubQ.PayMethod,  "
		"    PaymentsRefundsSubQ.Description,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
		"PaymentsRefundsSubQ.UserDefinedID, PaymentsRefundsSubQ.LocID, PaymentsRefundsSubQ.Location, "
		"PaymentsRefundsSubQ.CreditCardName ");
		break;
	case 141:
		//Payments by Service Category
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			TES 5/24/04 - Add ChargeDescription as editable field.
			DRT 8/26/2004 - PLID 13974 - This report does not include payments applied to gift certificates sold (charges
				with the ServiceID in GCTypesT)
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.gruber 2008-06-03 11:50) - PLID 22601 - added service code
			TES 1/13/2012 - PLID 33340 - Added ApplyDate, ChargeServiceDate, ChargeInputDate, BillDate, and BillInputDate
			// (d.thompson 2013-05-14) - PLID 56692 - Added POS (id, name) and POS designation for applied charges
			// (r.goldschmidt 2014-01-28 14:58) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByCPTCategoryFullQ.ProvID AS ProvID,  "
		"PaymentsByCPTCategoryFullQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First AS DocName,  "
		"PaymentsByCPTCategoryFullQ.TDate AS TDate,  "
		"PaymentsByCPTCategoryFullQ.PayMethod,  "
		"PaymentsByCPTCategoryFullQ.CCType,  "
		"PaymentsByCPTCategoryFullQ.CheckNo,  "
		"PaymentsByCPTCategoryFullQ.Description,  "
		"PaymentsByCPTCategoryFullQ.ApplyAmount,  "
		"CategoriesQ.Category,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"PaymentsByCPTCategoryFullQ.PatID AS PatID, "
		"PaymentsByCPTCategoryFullQ.UserDefinedID,  "
		"PaymentsByCPTCategoryFullQ.IDate AS IDate,  "
		"PaymentsByCPTCategoryFullQ.Type, "
		"CategoriesQ.CategoryID, "
		"PaymentsByCPTCategoryFullQ.LocID AS LocID, "
		"PaymentsByCPTCategoryFullQ.Location, CatFilterID AS CatFilterID, "
		"PaymentsByCPTCategoryFullQ.ItemDesc AS ChargeDescription, "
		"PaymentsByCPTCategoryFullQ.ServiceCode, PaymentsByCPTCategoryFullQ.ServiceName, "
		"PaymentsByCPTCategoryFullQ.ApplyDate AS ApplyDate, PaymentsByCPTCategoryFullQ.ChargeServiceDate AS ChargeServiceDate, "
		"PaymentsByCPTCategoryFullQ.ChargeInputDate AS ChargeInputDate, PaymentsByCPTCategoryFullQ.BillDate AS BillDate, "
		"PaymentsByCPTCategoryFullQ.BillInputDate AS BillInputDate, PaymentsByCPTCategoryFullQ.POSID, PaymentsByCPTCategoryFullQ.POSName, "
		"PaymentsByCPTCategoryFullQ.POSDesignation "
		"FROM ((((SELECT LineItemT.ID,  "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
		"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  "
		"PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ChargesT.ItemCode AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"PaymentPlansT.CheckNo,  "
		"ServiceT.Category,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"LineItemT.Type, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, ServiceT.Category AS CatFilterID, "
		" CPTCodeT.Code as ServiceCode, ServiceT.Name as ServiceName, "
		"AppliesT.InputDate AS ApplyDate, LineItemT_1.Date AS ChargeServiceDate, LineItemT_1.InputDate AS ChargeInputDate, "
		"BillsT.Date AS BillDate, BillsT.InputDate AS BillInputDate, BillsT.Location AS POSID, POSLocationsT.Name AS POSName, PlaceOfServiceCodesT.PlaceCodes AS POSDesignation "
		"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) "
		"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN LocationsT POSLocationsT ON BillsT.Location = POSLocationsT.ID "
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
		" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND GCTypesT.ServiceID IS NULL "
		"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ServiceT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.Code, ServiceT.Name, "
		"AppliesT.InputDate, LineItemT_1.Date, LineItemT_1.InputDate, BillsT.Date, BillsT.InputDate, BillsT.Location, POSLocationsT.Name, PlaceOfServiceCodesT.PlaceCodes "
		"HAVING (((LineItemT.Type)=1)) "
		") AS PaymentsByCPTCategoryFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON PaymentsByCPTCategoryFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON PaymentsByCPTCategoryFullQ.ProvID = PersonT.ID))");
		break;
	case 142:
		//Payments By Service Code
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2010-11-11 10:38) - PLID 41219 - added ChargeID
			// (d.thompson 2013-05-14) - PLID 56692 - Added POS (id, name) and POS designation for applied charges
			// (f.gelderloos 2013-07-26 15:00) - PLID 57483 - Renamed ServiceDate to ChargeDate, and added ChargeInputDate field
			// (r.goldschmidt 2014-01-28 14:59) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT AdjustmentsByCPTCodeFullQ.ChargeID, AdjustmentsByCPTCodeFullQ.ProvID AS ProvID, "
		"AdjustmentsByCPTCodeFullQ.FullName, "
		"PersonT.Last + ', ' + PersonT.First AS DocName, "
		"AdjustmentsByCPTCodeFullQ.TDate AS TDate, "
		"AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod, "
		"AdjustmentsByCPTCodeFullQ.CCType, "
		"AdjustmentsByCPTCodeFullQ.CheckNo, "
		"AdjustmentsByCPTCodeFullQ.Description, "
		"AdjustmentsByCPTCodeFullQ.ApplyAmount, "
		"CategoriesQ.Category, "
		"CategoriesQ.SubCategory, "
		"CategoriesQ.ParentID AS ParentID, "
		"AdjustmentsByCPTCodeFullQ.PatID AS PatID, "
		"AdjustmentsByCPTCodeFullQ.UserDefinedID, "
		"AdjustmentsByCPTCodeFullQ.IDate AS IDate, "
		"AdjustmentsByCPTCodeFullQ.Type, "
		"AdjustmentsByCPTCodeFullQ.ItemCode, "
		"AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, "
		"AdjustmentsByCPTCodeFullQ.ChargeAmount, "
		"AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, "
		"AdjustmentsByCPTCodeFullQ.ChargeDate AS ChargeDate, "
		"AdjustmentsByCPTCodeFullQ.ItemDesc, "
		"AdjustmentsByCPTCodeFullQ.LocID AS LocID, "
		"AdjustmentsByCPTCodeFullQ.Location, AdjustmentsByCPTCodeFullQ.POSID, AdjustmentsByCPTCodeFullQ.POSName, AdjustmentsByCPTCodeFullQ.POSDesignation, AdjustmentsByCPTCodeFullQ.ChargeInputDate AS ChargeInputDate "
		"FROM ((((SELECT LineItemT.ID, ChargesT.ID AS ChargeID, "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, "
		"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID, "
		"PatientsT.PersonID AS PatID, "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
		"LineItemT.InputDate AS IDate, "
		"LineItemT.Date AS TDate, "
		"PaymentsT.PayMethod, "
		"LineItemT.Description, "
		"ChargesT.ItemCode AS FirstOfItemCode, "
		"LineItemT.Amount AS PayAmt, "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
		"LineItemT_1.Date AS ChargeDate, "
		"LineItemT_1.InputDate AS ChargeInputDate, "
		"LineItemT_1.Description AS ItemDesc, "
		"CreditCardNamesT.CardName AS CCType, "
		"PaymentPlansT.CheckNo, "
		"ChargesT.Category, "
		"AppliesT.ID AS ApplyID, "
		"LineItemT.ID AS LineID, "
		"LineItemT.Type, "
		"ChargesT.ItemCode, "
		"CPTCodeT.ID AS CPTID, "
		"CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, BillsT.Location AS POSID, POSLocationsT.Name AS POSName, PlaceOfServiceCodesT.PlaceCodes AS POSDesignation "
		"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, "
		"Sum(AppliesT.Amount) AS ApplyAmt, "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total, "
		"LineItemT_1.PatientID "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN BillsT ON ChargesT.BIllID = BillsT.ID "
		"LEFT JOIN LocationsT POSLocationsT ON BillsT.Location = POSLocationsT.ID "
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND CPTCodeT.ID IS NOT NULL "
		"GROUP BY LineItemT.ID, ChargesT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.ID, BillsT.Location, POSLocationsT.Name, PlaceOfServiceCodesT.PlaceCodes, LineItemT_1.InputDate "
		"HAVING (((LineItemT.Type)=1)) "
		") AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID))");
		break;
	case 143:
		//Payments By Insurance By CPT
		/*	Revision History 
			 - DRT 11/25/02 - Previously this report was including inventory items, but grouping them all together under 1 item 
							(because it grouped by CPTCodeT.ID), which is clearly wrong.  I removed inv. items from this report, 
							and added a "payments by ins by inv item", which contains them.
			- TES 3/13/03: Changed to use charge location.
			// (j.gruber 2010-10-28 12:30) - PLID 39786 - added Birthdate, InsuranceID, policy group num, made it filter by charge service date and cleaned up the query a little to make it easier to read
			//(e.lally 2011-11-08) PLID 45541 - Added financial class
			// (r.goldschmidt 2014-01-29 10:33) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByInsCoByCPTCodeFullQ.ID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID AS PatID,  "
		"PaymentsByInsCoByCPTCodeFullQ.UserDefinedID, "
		"PaymentsByInsCoByCPTCodeFullQ.ApplyAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ProvID AS ProvID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID as patientid,  "
		"PaymentsByInsCoByCPTCodeFullQ.FullName,  "
		"PaymentsByInsCoByCPTCodeFullQ.IDate AS IDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.TDate AS TDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayMethod,  "
		"PaymentsByInsCoByCPTCodeFullQ.Description,  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfItemCode AS ItemCode,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayAmt,  "
		"PaymentsByInsCoByCPTCodeFullQ.ChargeAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ServiceDate as ServiceDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.ItemDesc,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsID AS InsID,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"[FirstOfItemCode] AS [CPT Code],  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfID AS CPTID, "
		"PaymentsByInsCoByCPTCodeFullQ.LocID AS LocID, "
		"PaymentsByInsCoByCPTCodeFullQ.Location, "
		"PaymentsByInsCoByCPTCodeFullQ.BirthDate, "
		"PaymentsByInsCoByCPTCodeFullQ.IDForInsurance, "
		"PaymentsByInsCoByCPTCodeFullQ.PolicyGroupNum "
		", PaymentsByInsCoByCPTCodeFullQ.FinancialClassID "
		", PaymentsByInsCoByCPTCodeFullQ.FinancialClass "
		"\r\n"
		"FROM  "
		"	((((SELECT LinePaysT.ID,  "
			"LinePaysT.PatientID,  "
			"ApplyAmount = CASE "
			"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
			"	THEN CASE "
			"		WHEN [LineChargesT].[ID] Is Null "
			"		THEN [LinePaysT].[Amount] "
			"		ELSE [AppliesT].[Amount] "
			"		End "
			"	ELSE [AppliesT].[Amount] "
			"	End,  "
			"ProvID = CASE WHEN [DoctorsProviders] Is Null "
			"	THEN PaymentsT.ProviderID "
			"	ELSE [DoctorsProviders] "
			"	End,  "
			"LinePaysT.PatientID AS PatID,  "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"LinePaysT.InputDate AS IDate,  "
			"LinePaysT.Date AS TDate,  "
			"PaymentsT.PayMethod,  "
			"LinePaysT.Description,  "
			"ChargesT.ItemCode AS FirstOfItemCode,  "
			"LinePaysT.Amount AS PayAmt,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
			"LineChargesT.Date AS ServiceDate,  "
			"LineChargesT.Description AS ItemDesc,  "
			"InsuranceCoT.PersonID AS InsID,  "
			"InsuranceCoT.Name AS InsName,  "
			"ChargesT.ItemSubCode,  "
			"AppliesT.ID AS ApplyID,  "
			"LinePaysT.ID AS LineID,  "
			"Min(ServiceT.ID) AS FirstOfID,  "
			"ChargesT.Category, "
			"CASE WHEN LineChargesT.LocationID Is Null THEN LinePaysT.LocationID ELSE LineChargesT.LocationID END AS LocID, "
			"CASE WHEN LineChargesT.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
			" PersonT.BirthDate, InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum "
			", InsuranceCoT.FinancialClassID, FinancialClassT.Name AS FinancialClass "
			"\r\n"
			" FROM (((((((((((LineItemT LinePaysT LEFT JOIN LocationsT ON LinePaysT.LocationID = LocationsT.ID) "
			" LEFT JOIN PaymentsT ON LinePaysT.ID = PaymentsT.ID) LEFT JOIN "
				" (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
				"Sum(AppliesT.Amount) AS ApplyAmt,  "
				"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
				"LineItemT_1.PatientID "
				"FROM LineItemT AS LineItemT_1 "
				"LEFT JOIN AppliesT ON LineItemT_1.ID = AppliesT.SourceID "
				"LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
				"LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
				"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
				"WHERE LineItemT_1.Deleted=0 AND ProductT.ID IS NOT NULL "
				"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
				"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
				") AS _PartiallyAppliedPaysQ  "
			" ON LinePaysT.ID = [_PartiallyAppliedPaysQ].ID) "
			" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) "
			" LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) "
			" LEFT JOIN LocationsT AS ChargeLoc ON LineChargesT.LocationID = ChargeLoc.ID) "
			" LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID) "
			" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) "
			" ON LinePaysT.PatientID = PatientsT.PersonID) " 
			" LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			" LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) "
			" LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyId = InsuredPartyT.PersonID) "
			" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) "
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			" LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID \r\n"

			" WHERE (((LinePaysT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)) "
			" GROUP BY LinePaysT.ID, LinePaysT.PatientID, (CASE "
			"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
			"	THEN CASE "
			"		WHEN [LineChargesT].[ID] Is Null "
			"		THEN [LinePaysT].[Amount] "
			"		ELSE [AppliesT].[Amount] "
			"		End "
			"	ELSE [AppliesT].[Amount] "
			"	End), CASE WHEN [DoctorsProviders] Is Null "
			"	THEN PaymentsT.ProviderID "
			"	ELSE [DoctorsProviders] "
			"	End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"   LinePaysT.InputDate, LinePaysT.Date, PaymentsT.PayMethod, LinePaysT.Description, ChargesT.ItemCode, LinePaysT.Amount, "
			"   dbo.GetChargeTotal(ChargesT.ID), LineChargesT.Date, LineChargesT.Description, InsuranceCoT.PersonID, InsuranceCoT.Name, "
			"   ChargesT.ItemSubCode, AppliesT.ID, LinePaysT.ID, ChargesT.Category, PaymentsT.ID, LinePaysT.Type, "
			"   CASE WHEN LineChargesT.LocationID Is Null THEN LinePaysT.LocationID ELSE LineChargesT.LocationID END, "
			"   CASE WHEN LineChargesT.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, "
			"	PersonT.BirthDate, InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum  "
			"	, InsuranceCoT.FinancialClassID, FinancialClassT.Name "
			"HAVING (((PaymentsT.ID) Is Not Null) AND ((LinePaysT.Type)=1)) "
		") AS PaymentsByInsCoByCPTCodeFullQ "
		" LEFT JOIN ProvidersT ON PaymentsByInsCoByCPTCodeFullQ.ProvID = ProvidersT.PersonID) "
		" LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) "
		" LEFT JOIN ServiceT ON PaymentsByInsCoByCPTCodeFullQ.FirstOfID = ServiceT.ID) "
		" LEFT JOIN (SELECT CategoriesT.ID AS CategoryID,  "
		"	      CategoriesT.Name AS Category, '' AS SubCategory,  "
		"         CategoriesT.ID AS ParentID "
		"	      FROM CategoriesT "
		"		  WHERE (((CategoriesT.Parent) = 0)) "
		"	      UNION "
		"		  SELECT SubCategoriesT.ID AS CategoryID,  "
		"         CategoriesT.Name AS Category,  "
		"         SubCategoriesT.Name AS SubCategory,  "
		"         SubCategoriesT.Parent AS ParentID "
		"		  FROM CategoriesT RIGHT JOIN "
		"         CategoriesT AS SubCategoriesT ON  "
		"         CategoriesT.ID = SubCategoriesT.Parent "
		"      WHERE (((SubCategoriesT.Parent) <> 0))) AS CategoriesQ "
		" ON ServiceT.Category = CategoriesQ.CategoryID "
		"");
		break;
	case 364:
		//Payments By Insurance By Inv Item
		/*	Revision History 
			 - DRT 11/25/02 - Created.  This report is a copy of the Payments by Insurance by Service Code report, but it shows ONLY
								inventory items.  Note:  This is pretty much a copy of the cpt code query, with modifications
								that were required to get inv items only.  Both of these should really be re-written at some point.
			- TES 3/13/03: Changed to use charge location
			- (r.goldschmidt 2014-01-28 15:02) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByInsCoByCPTCodeFullQ.ID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID AS PatID,  "
		"PaymentsByInsCoByCPTCodeFullQ.UserDefinedID, "
		"PaymentsByInsCoByCPTCodeFullQ.ApplyAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ProvID AS ProvID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID as patientid,  "
		"PaymentsByInsCoByCPTCodeFullQ.FullName,  "
		"PaymentsByInsCoByCPTCodeFullQ.IDate AS IDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.TDate AS TDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayMethod,  "
		"PaymentsByInsCoByCPTCodeFullQ.Description,  "
		"PaymentsByInsCoByCPTCodeFullQ.CodeName AS ItemCodeName, "
		"PaymentsByInsCoByCPTCodeFullQ.PayAmt,  "
		"PaymentsByInsCoByCPTCodeFullQ.ChargeAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ServiceDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.ItemDesc,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsID AS InsID,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfID AS ServiceID, "
		"PaymentsByInsCoByCPTCodeFullQ.LocID AS LocID, "
		"PaymentsByInsCoByCPTCodeFullQ.Location "
		"FROM ((((SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"ApplyAmount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ServiceT.Name AS CodeName, "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"InsuranceCoT.PersonID AS InsID,  "
		"InsuranceCoT.Name AS InsName,  "
		"ChargesT.ItemSubCode,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"Min(ServiceT.ID) AS FirstOfID,  "
		"ChargesT.Category, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM (((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 "
		"LEFT JOIN AppliesT ON LineItemT_1.ID = AppliesT.SourceID "
		"LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
		"LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
		"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
		"WHERE LineItemT_1.Deleted=0 AND ProductT.ID IS NOT NULL "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyId = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)) "
		"GROUP BY LineItemT.ID, LineItemT.PatientID, (CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End), CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.ItemSubCode, AppliesT.ID, LineItemT.ID, ChargesT.Category, PaymentsT.ID, LineItemT.Type, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, ServiceT.Name "
		"HAVING (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Type)=1)) "
		") AS PaymentsByInsCoByCPTCodeFullQ LEFT JOIN ProvidersT ON PaymentsByInsCoByCPTCodeFullQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN ServiceT ON PaymentsByInsCoByCPTCodeFullQ.FirstOfID = ServiceT.ID) LEFT JOIN (SELECT CategoriesT.ID AS CategoryID,  "
		"           CategoriesT.Name AS Category, '' AS SubCategory,  "
		"           CategoriesT.ID AS ParentID "
		"      FROM CategoriesT "
		"      WHERE (((CategoriesT.Parent) = 0)) "
		"      UNION "
		"      SELECT SubCategoriesT.ID AS CategoryID,  "
		"          CategoriesT.Name AS Category,  "
		"          SubCategoriesT.Name AS SubCategory,  "
		"          SubCategoriesT.Parent AS ParentID "
		"      FROM CategoriesT RIGHT JOIN "
		"          CategoriesT AS SubCategoriesT ON  "
		"          CategoriesT.ID = SubCategoriesT.Parent "
		"      WHERE (((SubCategoriesT.Parent) <> 0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID");
	case 144:
		//Payments By Insurance Co
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			- JMM 8/29/03: Changed to only show insurance payments, no patient payments
			DRT 8/25/2004 - PLID 13974 - This report does NOT show gift certificate payments.
			DRT 1/17/2006 - PLID 18864 - Added CheckNo, CCType, BankName fields.
				Also took opportunity to cleanup the query and make it readable.
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 13:18) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByInsCoSubQ.ID, PaymentsByInsCoSubQ.PatID AS PatID, PaymentsByInsCoSubQ.UserDefinedID,  "
			"Sum(PaymentsByInsCoSubQ.Amount) AS SumOfAmount, PaymentsByInsCoSubQ.ProvID AS ProvID,  "
			"Min(PaymentsByInsCoSubQ.RandomText) AS FirstOfRandomText, PaymentsByInsCoSubQ.PatID AS PatID2,   "
			"PaymentsByInsCoSubQ.FullName, PaymentsByInsCoSubQ.IDate AS IDate, PaymentsByInsCoSubQ.TDate AS TDate,   "
			"PaymentsByInsCoSubQ.PayMethod, PaymentsByInsCoSubQ.Description, PaymentsByInsCoSubQ.[Insurance Co ID] AS [Insurance Co ID],   "
			"PaymentsByInsCoSubQ.InsName, Min(PaymentsByInsCoSubQ.ApplyID) AS FirstOfApplyID, Min(PaymentsByInsCoSubQ.LineID) AS FirstOfLineID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, PaymentsByInsCoSubQ.LocID AS LocID,  "
			"PaymentsByInsCoSubQ.Location, PaymentsByInsCoSubQ.SDate AS ServiceDate, PaymentsByInsCoSubQ.CheckNo, "
			"PaymentsByInsCoSubQ.BankName, PaymentsByInsCoSubQ.CCType "
			"FROM ( "
			"	(SELECT * FROM  "
			"		(SELECT LineItemT.ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,   "
			"		Amount = CASE  "
			"			WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
			"			THEN CASE  "
			"				WHEN [LineItemT_1].[ID] Is Null  "
			"			THEN [LineItemT].[Amount]  "
			"			ELSE [AppliesT].[Amount]  "
			"			End  "
			"		ELSE [AppliesT].[Amount]  "
			"		End,   "
			"		ProvID = CASE  "
			"			WHEN [DoctorsProviders] Is Null  "
			"			THEN PaymentsT.ProviderID  "
			"			ELSE [DoctorsProviders]  "
			"			End,   "
			"		'Full' AS RandomText,  LineItemT.PatientID,  PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"		LineItemT.InputDate AS IDate, LineItemT.Date AS TDate, PaymentsT.PayMethod, LineItemT.Description,   "
			"		InsuranceCoT.PersonID AS [Insurance Co ID], InsuranceCoT.Name AS InsName, AppliesT.ID AS ApplyID,   "
			"		LineItemT.ID AS LineID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			"		CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
			"		LineItemT_1.Date AS SDate, PaymentPlansT.CheckNo, PaymentPlansT.BankNo AS BankName, CreditCardNamesT.CardName AS CCType "
			"		FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
			"		LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID)  "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"		LEFT JOIN  "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"			LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1  "
			"			LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID)  "
			"			ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID)  "
			"		LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)  "
			"		LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID)  "
			"		LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID)  "
			"		LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID)  "
			"		LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
			"		LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID)  "
			"		LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
			"		WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.InsuredPartyID <> -1) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"		) AS PaymentsByInsCoFullQ  "
			"	UNION  "
			"	SELECT * FROM  "
			"		(SELECT [_PartiallyAppliedPaysQ].ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,   "
			"		[_PartiallyAppliedPaysQ].Total AS Amount, PaymentsT.ProviderID AS ProvID, 'Part' AS RandomText,   "
			"		LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"		LineItemT.InputDate AS IDate, LineItemT.Date AS TDate,  PaymentsT.PayMethod,   "
			"		LineItemT.Description, InsuranceCoT.PersonID AS [Insurance Co ID], InsuranceCoT.Name AS InsName,   "
			"		0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"		NULL AS SDate, PaymentPlansT.CheckNo, PaymentPlansT.BankNo AS BankName, CreditCardNamesT.CardName AS CCType "
			"		FROM (((( "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"			LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN  "
			"			(AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID)  "
			"			ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
			"		ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID)  "
			"		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)  "
			"		ON LineItemT.PatientID = PatientsT.PersonID)  "
			"		LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID)  "
			"		LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.InsuredPartyID <> -1) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"		) AS PaymentsByInsCoPartQ "
			"	) AS PaymentsByInsCoSubQ  "
			"LEFT JOIN ProvidersT ON PaymentsByInsCoSubQ.ProvID = ProvidersT.PersonID)  "
			"LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
			"GROUP BY PaymentsByInsCoSubQ.ID, PaymentsByInsCoSubQ.PatientID, PaymentsByInsCoSubQ.ProvID, PaymentsByInsCoSubQ.PatID, PaymentsByInsCoSubQ.UserDefinedID, PaymentsByInsCoSubQ.FullName,  "
			"PaymentsByInsCoSubQ.IDate, PaymentsByInsCoSubQ.TDate, PaymentsByInsCoSubQ.PayMethod, PaymentsByInsCoSubQ.Description, PaymentsByInsCoSubQ.[Insurance Co ID],  "
			"PaymentsByInsCoSubQ.InsName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByInsCoSubQ.LocID, PaymentsByInsCoSubQ.Location, PaymentsByInsCoSubQ.SDate, "
			"PaymentsByInsCoSubQ.CheckNo, PaymentsByInsCoSubQ.BankName, PaymentsByInsCoSubQ.CCType");
		break;
	case 145:
		//Payments By Provider
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 15:07) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByProviderSubQ.ID,  "
		"PaymentsByProviderSubQ.PatientID,  "
		"Sum(PaymentsByProviderSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByProviderSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByProviderSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByProviderSubQ.PatID AS PatID, "
		"PaymentsByProviderSubQ.UserDefinedID, "
		"PaymentsByProviderSubQ.FullName,  "
		"PaymentsByProviderSubQ.IDate AS IDate,  "
		"PaymentsByProviderSubQ.TDate AS TDate,  "
		"PaymentsByProviderSubQ.PayMethod,  "
		"PaymentsByProviderSubQ.Description,  "
		"PaymentsByProviderSubQ.CheckNo,  "
		"PaymentsByProviderSubQ.CCType,  "
		"Min(PaymentsByProviderSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByProviderSubQ.LineID) AS FirstOfLineID,  "
		"Last + ', ' + First + ' ' + Middle AS DocName, "
		"PaymentsByProviderSubQ.LocID AS LocID, "
		"PaymentsByProviderSubQ.Location "
		"FROM (SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE  "
		"	WHEN _PartiallyAppliedPaysQ.ID Is NULL "
		"	THEN "
		"		CASE "
		"		WHEN LineItemT_1.ID Is Null "
		"		THEN LineItemT.Amount "
		"		ELSE AppliesT.Amount "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN [DoctorsProviders] Is NULL "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM ((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE ((PaymentsT.ID Is Not NULL) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByProviderFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location "
		"FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsbyProviderPartQ) "
		"AS PaymentsByProviderSubQ LEFT JOIN (ProvidersT LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) ON PaymentsByProviderSubQ.ProvID = ProvidersT.PersonID "
		"GROUP BY PaymentsByProviderSubQ.ID, PaymentsByProviderSubQ.PatientID, PaymentsByProviderSubQ.ProvID, PaymentsByProviderSubQ.PatID, PaymentsByProviderSubQ.UserDefinedID, PaymentsByProviderSubQ.FullName, PaymentsByProviderSubQ.IDate, PaymentsByProviderSubQ.TDate, PaymentsByProviderSubQ.PayMethod, PaymentsByProviderSubQ.Description, PaymentsByProviderSubQ.CheckNo, PaymentsByProviderSubQ.CCType, Last + ', ' + First + ' ' + Middle, PaymentsByProviderSubQ.LocID, PaymentsByProviderSubQ.Location "
		"");
		break;
	case 146:
		//Pays/Refs/Adjs By Category
		/*	Revision History
			1/16/03 - DRT - Added PaymentGroupsT.ID to the report, also added an external filter on that field
			3/13/03 - TES - Changed to use charge location if applied.
			3/18/2004 - PLID 11470 - Made it editable so they can use the location.
			DRT 8/25/2004 - PLID 13974 - This report does NOT show gift certificate payments.
			(r.goldschmidt 2014-01-29 11:06) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, PaymentGroupsT.GroupName as Category,  "
			"     AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID AS PatID,   "
			"    AdjustmentsSubQ.ProvID AS ProvID, SUM(AdjustmentsSubQ.Amount)   "
			"    AS Amount, MIN(AdjustmentsSubQ.RandomText)   "
			"    AS RandomText, AdjustmentsSubQ.PatName,   "
			"    AdjustmentsSubQ.IDate AS IDate, AdjustmentsSubQ.TDate AS TDate,   "
			"    CASE WHEN AdjustmentsSubQ.Type = 1 THEN (CASE WHEN AdjustmentsSubQ.PayMethod = 1 THEN 'Cash' ELSE CASE WHEN AdjustmentsSubQ.PayMethod = 2 THEN 'Check' ELSE  "
			"    CASE WHEN AdjustmentsSubQ.PayMethod = 3 THEN 'Credit Card' ELSE 'No Method' End End End)  "
			"         WHEN AdjustmentsSubQ.Type = 2 THEN ( 'Adjustment')  "
			"         WHEN AdjustmentsSubQ.Type = 3 THEN ( 'Refund') END AS Method,   "
			"	CASE WHEN AdjustmentsSubQ.PayMethod = 0 THEN CASE WHEN Type=2 THEN 4 ELSE 5 END ELSE PayMethod END AS PayMethod,  "
			"	AdjustmentsSubQ.Description,   "
			"    MIN(AdjustmentsSubQ.ApplyID) AS ApplyID,   "
			"    MIN(AdjustmentsSubQ.LineID) AS LineID,   "
			"    AdjustmentsSubQ.Type,  "
			"AdjustmentsSubQ.LocID AS LocID,  "
			"AdjustmentsSubQ.Location, PaymentGroupsT.ID AS GroupID "
			"FROM (SELECT *  "
			"      FROM (SELECT LineItemT.ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"                ProvID =   "
			"	CASE  "
			"		WHEN ChargesT.DoctorsProviders is Null then  "
			"			PaymentsT.ProviderID  "
			"		ELSE ChargesT.DoctorsProviders  "
			"	END,   "
			"        Amount =   "
			"	CASE   "
			"		WHEN _PartiallyAppliedPaysQ.ID is NULL then   "
			"		CASE  "
			"		WHEN LineItemT1.ID is Null then LineItemT.Amount  "
			"			Else AppliesT.Amount  "
			"		End  "
			"		ELSE AppliesT.Amount  "
			"		END, 'FULL' AS RandomText,   "
			"                PatientInfo.Last + ', ' + PatientInfo.First + ' ' + PatientInfo.Middle  "
			"                 AS PatName, LineItemT.InputDate AS IDate,   "
			"                LineItemT.Date AS TDate,   "
			"                PaymentsT.PayMethod, LineItemT.Description,   "
			"                AppliesT.ID AS ApplyID,   "
			"                LineItemT.ID AS LineID, LineItemT.Type, PaymentsT.PaymentGroupID as GroupID,  "
			"				 CASE WHEN LineItemT1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT1.LocationID END AS LocID, "
			"				 CASE WHEN LineItemT1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location  "
			"            FROM PatientsT LEFT OUTER JOIN  "
			"                PersonT AS PatientInfo ON   "
			"                PatientsT.PersonID = PatientInfo.ID RIGHT OUTER  "
			"                 JOIN  "
			"                    (SELECT LineItemT_1.ID,   "
			"                       LineItemT_1.Amount AS PayAmt,   "
			"                       SUM(AppliesT.Amount) AS ApplyAmt,   "
			"                       MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount]) AS Total,   "
			"                       LineItemT_1.PatientID  "
			"                  FROM LineItemT AS LineItemT_1 LEFT JOIN  "
			"                       (PaymentsT LEFT JOIN  "
			"                       (AppliesT LEFT JOIN  "
			"                       LineItemT ON   "
			"                       AppliesT.DestID = LineItemT.ID) ON   "
			"                       PaymentsT.ID = AppliesT.SourceID) ON   "
			"                       LineItemT_1.ID = PaymentsT.ID  "
			"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"                  GROUP BY LineItemT_1.ID,   "
			"                       LineItemT_1.Amount,   "
			"                       LineItemT_1.PatientID  "
			"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND   "
			"                       ((MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount])) <> 0)))   "
			"                _PartiallyAppliedPaysQ RIGHT OUTER JOIN  "
			"                (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON   "
			"                _PartiallyAppliedPaysQ.ID = LineItemT.ID LEFT OUTER  "
			"                 JOIN  "
			"                ProvidersT LEFT OUTER JOIN  "
			"                PersonT DocInfo ON   "
			"                ProvidersT.PersonID = DocInfo.ID RIGHT OUTER JOIN  "
			"                ChargesT ON   "
			"                ProvidersT.PersonID = ChargesT.DoctorsProviders  "
			"                 RIGHT OUTER JOIN  "
			"                (LineItemT LineItemT1 LEFT JOIN LocationsT AS ChargeLoc ON LineItemT1.LocationID = ChargeLoc.ID) ON   "
			"                ChargesT.ID = LineItemT1.ID RIGHT OUTER JOIN  "
			"                AppliesT ON   "
			"                LineItemT1.ID = AppliesT.DestID RIGHT OUTER JOIN  "
			"                PaymentsT ON   "
			"                AppliesT.SourceID = PaymentsT.ID ON   "
			"                LineItemT.ID = PaymentsT.ID ON   "
			"                PatientsT.PersonID = LineItemT.PatientID  "
			"            WHERE (LineItemT.Deleted = 0) AND   "
			"                (PaymentsT.ID IS NOT NULL) AND (PaymentsT.PayMethod NOT IN (4,10)))   "
			"          AS AdjustmentsFullQ  "
			"      UNION  "
			"      SELECT *  "
			"      FROM (SELECT PartiallyAppliedPaysQ.ID,   "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"                PaymentsT.ProviderID,   "
			"                PartiallyAppliedPaysQ.Total AS Amount,   "
			"                'PART' AS RandomText,   "
			"                PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			"                 AS PatName, LineItemT.InputDate AS IDate,   "
			"                LineItemT.Date AS TDate,   "
			"                PaymentsT.PayMethod, LineItemT.Description,   "
			"                0 AS ApplyID, LineItemT.ID AS LineID,   "
			"                LineItemT.Type, PaymentsT.PaymentGroupID as GroupID,  "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS Location  "
			"            FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN  "
			"                    (SELECT LineItemT_1.ID,   "
			"                       LineItemT_1.Amount AS PayAmt,   "
			"                       SUM(AppliesT.Amount) AS ApplyAmt,   "
			"                       MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount]) AS Total,   "
			"                       LineItemT_1.PatientID  "
			"                  FROM LineItemT AS LineItemT_1 LEFT JOIN  "
			"                       (PaymentsT LEFT JOIN  "
			"                       (AppliesT LEFT JOIN  "
			"                       LineItemT ON   "
			"                       AppliesT.DestID = LineItemT.ID) ON   "
			"                       PaymentsT.ID = AppliesT.SourceID) ON   "
			"                       LineItemT_1.ID = PaymentsT.ID  "
			"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"                  GROUP BY LineItemT_1.ID,   "
			"                       LineItemT_1.Amount,   "
			"                       LineItemT_1.PatientID  "
			"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND   "
			"                       ((MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount])) <> 0)))   "
			"                PartiallyAppliedPaysQ ON   "
			"                LineItemT.ID = PartiallyAppliedPaysQ.ID INNER JOIN  "
			"                PaymentsT ON   "
			"                LineItemT.ID = PaymentsT.ID INNER JOIN  "
			"                PatientsT ON   "
			"                LineItemT.PatientID = PatientsT.PersonID INNER JOIN  "
			"                PersonT ON   "
			"                PatientsT.PersonID = PersonT.ID  "
			"            WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PayMethod NOT IN (4,10))) AS AdjustmentsPartQ)   "
			"    AdjustmentsSubQ LEFT OUTER JOIN  "
			"    ProvidersT ON   "
			"    AdjustmentsSubQ.ProvID = ProvidersT.PersonID LEFT OUTER JOIN  "
			"    PersonT ON ProvidersT.PersonID = PersonT.ID Left Join PaymentGroupsT on PaymentGroupsT.ID = AdjustmentsSubQ.GroupID  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"     AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID,  "
			"AdjustmentsSubQ.UserDefinedID,   "
			"    AdjustmentsSubQ.ProvID, AdjustmentsSubQ.PatName,   "
			"    AdjustmentsSubQ.IDate, AdjustmentsSubQ.TDate,   "
			"    AdjustmentsSubQ.PayMethod, AdjustmentsSubQ.Description,   "
			"    AdjustmentsSubQ.Type, PaymentGroupsT.GroupName,  "
			"AdjustmentsSubQ.LocID, AdjustmentsSubQ.Location, PaymentGroupsT.ID ");
		break;
	case 147:
		//PrePayments
		/*	Version History
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			// (z.manning 2009-03-24 18:01) - PLID 31428 - Added applied prepayment amount and ShowApplied flag
			TES 9/22/2009 - PLID 34530 - Added LastAppliedDate and LastAppliedToDate
		*/
		return _T(FormatString("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"    PaymentsT.ProviderID AS ProvID, PaymentsT.PayMethod,   "
			"    LineItemT.Description,  "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN   "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)  "
			"ELSE  "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,  "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN 0 ELSE PrepayAppliedToQ.Amount END) AS AppliedToPrepayAmount, "
			"(CASE WHEN PrepayAppliesQ.ID IS NULL THEN 0 ELSE PrepayAppliesQ.Amount END) AS AppliedPrepayAmount, "
			"    LineItemT.Date AS TDate, PaymentPlansT.CheckNo,   "
			"    CreditCardNamesT.CardName AS CCType, LineItemT.InputDate AS IDate,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS  "
			"     DocName,  "
			"	LineItemT.LocationID AS LocID,  "
			"	LocationsT.Name AS Location, LineItemT.ID AS LineID,  "
			"	%i AS ShowApplied, "
			"	PrepayAppliesQ.AppliedDate AS LastAppliedDate, PrepayAppliedToQ.AppliedDate AS LastAppliedToDate "
			"FROM PaymentsT INNER JOIN  "
			"    (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID INNER JOIN  "
			"    PatientsT ON   "
			"    LineItemT.PatientID = PatientsT.PersonID INNER JOIN  "
			"    PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN  "
			"    PersonT PersonT1 ON   "
			"    PaymentsT.ProviderID = PersonT1.ID LEFT OUTER JOIN  "
			"    AppliesT AppliesT1 ON   "
			"    LineItemT.ID = AppliesT1.DestID LEFT OUTER JOIN  "
			"    AppliesT ON   "
			"    LineItemT.ID = AppliesT.SourceID LEFT OUTER JOIN  "
			"    PaymentPlansT ON   "
			"    PaymentsT.ID = PaymentPlansT.ID  "
			"	 LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"LEFT JOIN  "
			"/* This will total everything applied to a prepayment */  "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID, Max(AppliesT.InputDate) AS AppliedDate "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID  "
			"WHERE (LineItemT.Deleted = 0)  "
			"GROUP BY AppliesT.DestID  "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"LEFT JOIN  "
			" "
			"/* This will total everything that the prepayment is applied to */  "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID, Max(AppliesT.InputDate) AS AppliedDate  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID  "
			"WHERE LineItemT.Deleted = 0  "
			"GROUP BY AppliesT.SourceID  "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID  "
			" "
			"WHERE (LineItemT.Deleted = 0) "
			"AND (PaymentsT.Prepayment = 1) "
			"GROUP BY LineItemT.ID, PatientsT.UserDefinedID, PatientsT.PersonID,   "
			"    PaymentsT.ProviderID, PaymentsT.PayMethod,   "
			"    LineItemT.Description,  "
			"	 PrepayAppliedToQ.ID, PrepayAppliesQ.ID, LineItemT.Amount, PrepayAppliedToQ.Amount, PrepayAppliesQ.Amount, "
			"    LineItemT.Date, PaymentPlansT.CheckNo,   "
			"    CreditCardNamesT.CardName, LineItemT.InputDate,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"LineItemT.LocationID,  "
			"LocationsT.Name, "
			"PrepayAppliedToQ.AppliedDate, PrepayAppliesQ.AppliedDate "
			, bExtended && saExtraValues.GetSize() == 1 ? 1 : 0));
		break;
	

case 168:
		//Payments By Referring Physician
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 15:09) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByRefPhysSubQ.ID,  "
		"Sum(PaymentsByRefPhysSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByRefPhysSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByRefPhysSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByRefPhysSubQ.PatID AS PatID, "
		"PaymentsByRefPhysSubQ.UserDefinedID,  "
		"PaymentsByRefPhysSubQ.FullName,  "
		"PaymentsByRefPhysSubQ.IDate AS IDate,  "
		"PaymentsByRefPhysSubQ.TDate AS TDate,  "
		"PaymentsByRefPhysSubQ.PayMethod AS PayMethod,  "
		"PaymentsByRefPhysSubQ.Description,  "
		"Min(PaymentsByRefPhysSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByRefPhysSubQ.LineID) AS FirstOfLineID,  "
		"PaymentsByRefPhysSubQ.CheckNo,  "
		"PaymentsByRefPhysSubQ.CCType,  "
		"PaymentsByRefPhysSubQ.RefPhysID AS RefPhysID,  "
		"PaymentsByRefPhysSubQ.RefPhysName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByRefPhysSubQ.LocID AS LocID, "
		"PaymentsByRefPhysSubQ.Location "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM ((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location "
		"FROM ((((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysPartQ "
		") AS PaymentsByRefPhysSubQ LEFT JOIN ProvidersT ON PaymentsByRefPhysSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsByRefPhysSubQ.ID, PaymentsByRefPhysSubQ.ProvID, PaymentsByRefPhysSubQ.PatID, PaymentsByRefPhysSubQ.UserDefinedID, PaymentsByRefPhysSubQ.FullName, PaymentsByRefPhysSubQ.IDate, PaymentsByRefPhysSubQ.TDate, PaymentsByRefPhysSubQ.PayMethod, PaymentsByRefPhysSubQ.Description, PaymentsByRefPhysSubQ.CheckNo, PaymentsByRefPhysSubQ.CCType, PaymentsByRefPhysSubQ.RefPhysID, PaymentsByRefPhysSubQ.RefPhysName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByRefPhysSubQ.LocID, PaymentsByRefPhysSubQ.Location "
		"");
		break;
	

case 179:
		//Payments By Date
		/*	Version History
			- TES 3/13/03: Changed to use charge location if applied.
			- DRT 9/25/03 - PLID 9605 - Made report editable, added extended filter for cash/check/cc, added checkno field to the query
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			// (j.gruber 2010-06-09 15:36) - PLID 39074 - added InsuredPartyID
			// (r.goldschmidt 2014-01-28 15:12) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,  PaymentsByDateSubQ.PatientID,  Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,   "
			"PaymentsByDateSubQ.ProvID AS ProvID, Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,   "
			"PaymentsByDateSubQ.PatID AS PatID,  PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.FullName,   "
			"PaymentsByDateSubQ.IDate AS IDate,  PaymentsByDateSubQ.TDate AS TDate,  PaymentsByDateSubQ.PayMethod AS PayMethod,   "
			"PaymentsByDateSubQ.Description,  Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,   "
			"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,  PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
			"PaymentsByDateSubQ.LocID AS LocID, PaymentsByDateSubQ.Location, PaymentsByDateSubQ.CheckNo, PaymentsByDateSubQ.InsuredPartyID "
			"FROM ( "
			"	(SELECT * FROM  "
			"		(SELECT LineItemT.ID,   "
			"		LineItemT.PatientID,   "
			"		Amount = CASE  "
			"			WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
			"			THEN CASE  "
			"				WHEN [LineItemT_1].[ID] Is Null  "
			"				THEN [LineItemT].[Amount]  "
			"				ELSE [AppliesT].[Amount]  "
			"				End  "
			"			ELSE [AppliesT].[Amount]  "
			"			End,   "
			"		ProvID = CASE   "
			"			WHEN [DoctorsProviders] Is Null  "
			"			THEN [ProviderID]  "
			"			ELSE [DoctorsProviders]  "
			"			End,  "
			"		'Full' AS RandomText,  LineItemT.PatientID AS PatID,  PatientsT.UserDefinedID,  "
			"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate,  PaymentsT.PayMethod,  LineItemT.Description,  AppliesT.ID AS ApplyID,   "
			"		LineItemT.ID AS LineID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			"		CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, PaymentPlansT.CheckNo, PaymentsT.InsuredPartyID "
			"		FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN  "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			"			Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"			LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"			ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID)  "
			"		LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID)  "
			"		LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"		) AS PaymentsByDateFullQ  "
			" "
			"	UNION  "
			"	SELECT * FROM  "
			"		(SELECT [_PartiallyAppliedPaysQ].ID,   "
			"		LineItemT.PatientID,  [_PartiallyAppliedPaysQ].Total AS Amount,  PaymentsT.ProviderID AS ProvID,   "
			"		'Part' AS RandomText,  LineItemT.PatientID AS PatID,  PatientsT.UserDefinedID,  "
			"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"		LineItemT.InputDate AS IDate,  LineItemT.Date AS TDate,  PaymentsT.PayMethod,   "
			"		LineItemT.Description,  0 AS ApplyID,  LineItemT.ID AS LineID, LineItemT.LocationID AS LocID,  "
			"		LocationsT.Name AS Location, PaymentPlansT.CheckNo, PaymentsT.InsuredPartyID "
			"		FROM (( "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			"			Sum(AppliesT.Amount) AS ApplyAmt,  /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"			LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"			ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID =  "
			"		PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"		) AS PaymentsByDatePartQ "
			"	) AS PaymentsByDateSubQ  "
			"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
			"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate,  "
			"PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID,  "
			"PaymentsByDateSubQ.LocID, PaymentsByDateSubQ.Location, PaymentsByDateSubQ.CheckNo, PaymentsByDateSubQ.InsuredPartyID ");
		break;
	

case 215:
		//Adjustments By Inv Item
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			DRT 7/13/2004 - PLID 13442 - Reverified ttx, the ItemCode field was a string, but is always a long on 
				this report because it is only showing inv items.  It's not even really the ItemCode field, it's
				ProductT.ID renamed as ItemCode.
			DRT 4/10/2006 - PLID 11734 - Removed ProcCode.
			// (j.gruber 2007-05-29 16:35) - PLID 24837 - added reason code and group code
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2010-09-24 17:48) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.goldschmidt 2014-01-28 15:12) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,  "
		"AdjustmentsByCPTCodeFullQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First AS DocName,  "
		"AdjustmentsByCPTCodeFullQ.TDate AS TDate,  "
		"AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod,  "
		"AdjustmentsByCPTCodeFullQ.CCType,  "
		"AdjustmentsByCPTCodeFullQ.CheckNo,  "
		"AdjustmentsByCPTCodeFullQ.Description,  "
		"AdjustmentsByCPTCodeFullQ.ApplyAmount,  "
		"CategoriesQ.Category,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"AdjustmentsByCPTCodeFullQ.PatID AS PatID, "
		"AdjustmentsByCPTCodeFullQ.UserDefinedID,  "
		"AdjustmentsByCPTCodeFullQ.IDate AS IDate,  "
		"AdjustmentsByCPTCodeFullQ.Type, "
		"AdjustmentsByCPTCodeFullQ.ItemCode, "
		"AdjustmentsByCPTCodeFullQ.ChargeAmount, "
		"AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, "
		"AdjustmentsByCPTCodeFullQ.ServiceDate AS ServiceDate, "
		"AdjustmentsByCPTCodeFullQ.ItemDesc, "
		"AdjustmentsByCPTCodeFullQ.LocID AS LocID, "
		"AdjustmentsByCPTCodeFullQ.Location, "
		"AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, "
		"AdjustmentsByCPTCodeFullQ.ReasonCode, "
		"AdjustmentsByCPTCodeFullQ.GroupCode, "
		"AdjustmentsByCPTCodeFullQ.ReasonCodeDesc, "
		"AdjustmentsByCPTCodeFullQ.GroupCodeDesc "
		"FROM ((((SELECT LineItemT.ID,  "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
		"CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End AS ProvID,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ProductT.ID AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"PaymentPlansT.CheckNo,  "
		"ChargesT.Category,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"LineItemT.Type, "
		"ProductT.ID AS ItemCode, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"ProductT.ID AS CPTID, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND ProductT.ID IS NOT NULL "
		"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End, LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ProductT.ID, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, PatientsT.UserDefinedID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, ProductT.ID, "
		"AdjustmentGroupCodesT.Code, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description), "
		"AdjustmentReasonCodesT.Code, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) "
		"HAVING (((LineItemT.Type)=2)) "
		") AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID))");
		break;
	case 216:
		//Adjustments By Insurance By Inv Item
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			DRT 7/13/2004 - PLID 13442 - Reverified reports to fix a problem with the [CPT Code] field, 
				which is actually ServiceT.ID renamed as FirstOfItemCode, then renamed to [CPT Code].
			DRT 4/10/2006 - PLID 11734 - Removed ProcCode.
			// (j.gruber 2007-05-29 16:28) - PLID 24837 - added reason code and group code
			// (j.jones 2010-09-27 08:36) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.goldschmidt 2014-01-28 15:15) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByInsCoByCPTCodeFullQ.ID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatientID,  "
		"PaymentsByInsCoByCPTCodeFullQ.ApplyAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ProvID AS ProvID,  "
		"PaymentsByInsCoByCPTCodeFullQ.PatID AS PatID,  "
		"PaymentsByInsCoByCPTCodeFullQ.UserDefinedID, "
		"PaymentsByInsCoByCPTCodeFullQ.FullName,  "
		"PaymentsByInsCoByCPTCodeFullQ.IDate AS IDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.TDate AS TDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayMethod AS PayMethod,  "
		"PaymentsByInsCoByCPTCodeFullQ.Description,  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfItemCode AS ItemCode,  "
		"PaymentsByInsCoByCPTCodeFullQ.PayAmt,  "
		"PaymentsByInsCoByCPTCodeFullQ.ChargeAmount,  "
		"PaymentsByInsCoByCPTCodeFullQ.ServiceDate AS ServiceDate,  "
		"PaymentsByInsCoByCPTCodeFullQ.ItemDesc,  "
		"PaymentsByInsCoByCPTCodeFullQ.InsID AS InsID,  "
		"CASE WHEN PaymentsByInsCoByCPTCodeFullQ.InsID IS NULL THEN 'No Insurance' ELSE PaymentsByInsCoByCPTCodeFullQ.InsName END AS InsName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"[FirstOfItemCode] AS [CPT Code],  "
		"PaymentsByInsCoByCPTCodeFullQ.FirstOfID AS CPTID, "
		"PaymentsByInsCoByCPTCodeFullQ.LocID AS LocID, "
		"PaymentsByInsCoByCPTCodeFullQ.Location, "
		"PaymentsByInsCoByCPTCodeFullQ.ReasonCode, "
		"PaymentsByInsCoByCPTCodeFullQ.GroupCode, "
		"PaymentsByInsCoByCPTCodeFullQ.ReasonCodeDesc, "
		"PaymentsByInsCoByCPTCodeFullQ.GroupCodeDesc "
		"FROM ((((SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"ApplyAmount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ServiceT.ID AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"InsuranceCoT.PersonID AS InsID,  "
		"InsuranceCoT.Name AS InsName,  "
		"ChargesT.ItemSubCode,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"Min(ServiceT.ID) AS FirstOfID,  "
		"ChargesT.Category, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM (((((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyId = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)) AND ProductT.ID IS NOT NULL "
		"GROUP BY LineItemT.ID, LineItemT.PatientID, (CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End), CASE WHEN [DoctorsProviders] Is Null "
		"	THEN PaymentsT.ProviderID "
		"	ELSE [DoctorsProviders] "
		"	End, LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ServiceT.ID, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.ItemSubCode, AppliesT.ID, LineItemT.ID, ChargesT.Category, PaymentsT.ID, LineItemT.Type, PatientsT.UserDefinedID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, "
		"AdjustmentGroupCodesT.Code, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description), "
		"AdjustmentReasonCodesT.Code, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) "
		"HAVING (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Type)=2)) "
		") AS PaymentsByInsCoByCPTCodeFullQ LEFT JOIN ProvidersT ON PaymentsByInsCoByCPTCodeFullQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN ServiceT ON PaymentsByInsCoByCPTCodeFullQ.FirstOfID = ServiceT.ID) LEFT JOIN (SELECT CategoriesT.ID AS CategoryID,  "
		"           CategoriesT.Name AS Category, '' AS SubCategory,  "
		"           CategoriesT.ID AS ParentID "
		"      FROM CategoriesT "
		"      WHERE (((CategoriesT.Parent) = 0)) "
		"      UNION "
		"      SELECT SubCategoriesT.ID AS CategoryID,  "
		"          CategoriesT.Name AS Category,  "
		"          SubCategoriesT.Name AS SubCategory,  "
		"          SubCategoriesT.Parent AS ParentID "
		"      FROM CategoriesT RIGHT JOIN "
		"          CategoriesT AS SubCategoriesT ON  "
		"          CategoriesT.ID = SubCategoriesT.Parent "
		"      WHERE (((SubCategoriesT.Parent) <> 0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID");
		break;
	case 217:
		//Payments By Inv Item
		/*
			Revision History
				- DRT - 11/25/02 - Changed report files only - summary reports now group on CPTID, instead of ItemCode.  Itemcode is useless for inv items 
								(and is legacy data anyways), and was always grouping them together as blanks.  Also removed that field from the dtld reports.
				- TES 3/13/03: Changed to use charge location.
				DRT 4/10/2006 - PLID 11734 - Removed ProcCode
				(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
				// (d.thompson 2013-05-14) - PLID 56692 - Added POS (id, name) and POS designation for applied charges
				// (r.farnworth 2013-07-24 09:27) - PLID 57480 - Payments by Inventory Item - add the credit card authorization field
				// (f.gelderloos 2013-07-26 15:00) - PLID 57483 - Renamed ServiceDate to ChargeDate, and added ChargeInputDate field
				// (r.goldschmidt 2014-01-28 15:17) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
				// (j.jones 2016-03-10 08:47) - PLID 57289 - changed ItemDesc to be the product name, not the charge description
		*/
		return _T("SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID, "
			"AdjustmentsByCPTCodeFullQ.FullName, "
			"PersonT.Last + ', ' + PersonT.First AS DocName, "
			"AdjustmentsByCPTCodeFullQ.TDate AS TDate, "
			"AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod, "
			"AdjustmentsByCPTCodeFullQ.CCType, "
			"COALESCE(AdjustmentsByCPTCodeFullQ.CCAuthNo, '') AS CCAuthNo, "
			"AdjustmentsByCPTCodeFullQ.CheckNo, "
			"AdjustmentsByCPTCodeFullQ.Description, "
			"AdjustmentsByCPTCodeFullQ.ApplyAmount, "
			"CategoriesQ.Category, "
			"CategoriesQ.SubCategory, "
			"CategoriesQ.ParentID AS ParentID, "
			"AdjustmentsByCPTCodeFullQ.PatID AS PatID, "
			"AdjustmentsByCPTCodeFullQ.UserDefinedID, "
			"AdjustmentsByCPTCodeFullQ.IDate AS IDate, "
			"AdjustmentsByCPTCodeFullQ.Type, "
			"AdjustmentsByCPTCodeFullQ.ItemCode, "
			"AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, "
			"AdjustmentsByCPTCodeFullQ.ChargeAmount, "
			"AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, "
			"AdjustmentsByCPTCodeFullQ.ChargeDate AS ChargeDate, "
			"AdjustmentsByCPTCodeFullQ.ItemDesc, "
			"AdjustmentsByCPTCodeFullQ.LocID AS LocID, "
			"AdjustmentsByCPTCodeFullQ.Location, AdjustmentsByCPTCodeFullQ.POSID, AdjustmentsByCPTCodeFullQ.POSName, AdjustmentsByCPTCodeFullQ.POSDesignation, AdjustmentsByCPTCodeFullQ.ChargeInputDate AS ChargeInputDate "
			"FROM ((((SELECT LineItemT.ID, "
			"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, "
			"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID, "
			"PatientsT.PersonID AS PatID, "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
			"LineItemT.InputDate AS IDate, "
			"LineItemT.Date AS TDate, "
			"PaymentsT.PayMethod, "
			"LineItemT.Description, "
			"ChargesT.ItemCode AS FirstOfItemCode, "
			"LineItemT.Amount AS PayAmt, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"LineItemT_1.Date AS ChargeDate, "
			"LineItemT_1.InputDate AS ChargeInputDate, "
			"ServiceT.Name AS ItemDesc, "
			"CreditCardNamesT.CardName AS CCType, "
			"PaymentPlansT.CheckNo, "
			"PaymentPlansT.CCAuthNo AS CCAuthNo, "
			"ChargesT.Category, "
			"AppliesT.ID AS ApplyID, "
			"LineItemT.ID AS LineID, "
			"LineItemT.Type, "
			"ChargesT.ItemCode, "
			"ChargesT.ServiceID AS CPTID, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, BillsT.Location AS POSID, POSLocationsT.Name AS POSName, PlaceOfServiceCodesT.PlaceCodes AS POSDesignation "
			"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
			"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, "
			"Sum(AppliesT.Amount) AS ApplyAmt, "
			"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total, "
			"LineItemT_1.PatientID "
			"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
			"WHERE (((LineItemT_1.Deleted)=0)) "
			"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
			") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) "
			"LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID) ON LineItemT_1.ID = ChargesT.ID) "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT POSLocationsT ON BillsT.Location = POSLocationsT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND ProductT.ID IS NOT NULL "
			"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, ServiceT.Name, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, PaymentPlansT.CCAuthNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, ServiceT.Name, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, ChargesT.ServiceID, BillsT.Location, POSLocationsT.Name, PlaceOfServiceCodesT.PlaceCodes, LineItemT_1.InputDate "
			"HAVING (((LineItemT.Type)=1)) "
			") AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
			"FROM CategoriesT "
			"WHERE (((CategoriesT.Parent)=0)) "
			"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
			"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
			"WHERE (((SubCategoriesT.Parent)<>0)) "
			") AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID))");
		break;
	

	case 243:
		//Payments by Charge Location
		/*	Version History
			- DRT 3/11/03 - Fixed a bug with the location showing as NULL if your payment was unapplied
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificates.
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,   "
		"PaymentsByDateSubQ.PatientID,   "
		"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,   "
		"PaymentsByDateSubQ.ProvID AS ProvID,   "
		"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,   "
		"PaymentsByDateSubQ.PatID AS PatID,   "
		"PaymentsByDateSubQ.UserDefinedID,  "
		"PaymentsByDateSubQ.FullName,   "
		"PaymentsByDateSubQ.IDate AS IDate,   "
		"PaymentsByDateSubQ.TDate AS TDate,   "
		"PaymentsByDateSubQ.PayMethod AS PayMethod,   "
		"PaymentsByDateSubQ.Description,   "
		"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,   "
		"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,   "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
		"PaymentsByDateSubQ.LocID AS LocID,  "
		"LocationsT.Name "
		"FROM ( "
		"   (SELECT * FROM  "
		" "
		"	(SELECT LineItemT.ID,   "
		"	LineItemT.PatientID,   "
		"	Amount = CASE  "
		"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
		"		THEN CASE  "
		"			WHEN [LineItemT_1].[ID] Is Null  "
		"			THEN [LineItemT].[Amount]  "
		"			ELSE [AppliesT].[Amount]  "
		"			End  "
		"		ELSE [AppliesT].[Amount]  "
		"		End,   "
		"	ProvID = CASE   "
		"		WHEN [DoctorsProviders] Is Null  "
		"		THEN [ProviderID]  "
		"		ELSE [DoctorsProviders]  "
		"		End,  "
		"	'Full' AS RandomText,   "
		"	LineItemT.PatientID AS PatID,   "
		"	PatientsT.UserDefinedID,  "
		"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
		"	LineItemT.InputDate AS IDate,   "
		"	LineItemT.Date AS TDate,   "
		"	PaymentsT.PayMethod,   "
		"	LineItemT.Description,   "
		"	AppliesT.ID AS ApplyID,   "
		"	LineItemT.ID AS LineID,  "
		"	CASE WHEN LineItemT_1.LocationID IS NULL THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
		"	LocationsT.Name AS Location  "
		"	FROM (((((LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN  "
		"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
		"		Sum(AppliesT.Amount) AS ApplyAmt,   "
		"		/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
		"		LineItemT_1.PatientID   "
		"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON  "
		"		LineItemT_1.ID = PaymentsT.ID  "
		"		WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
		"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
		"		HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
		"		) AS _PartiallyAppliedPaysQ  "
		"	ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (LineItemT AS LineItemT_1 LEFT JOIN LocationsT ON  "
		"	LineItemT_1.LocationID = LocationsT.ID) ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON  "
		"	PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
		"	WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
		"	) AS PaymentsByDateFullQ  "
		"	 "
		"	UNION  "
		" "
		"	SELECT * FROM  "
		"		(SELECT [_PartiallyAppliedPaysQ].ID,   "
		"		LineItemT.PatientID,   "
		"		[_PartiallyAppliedPaysQ].Total AS Amount,   "
		"		PaymentsT.ProviderID AS ProvID,   "
		"		'Part' AS RandomText,   "
		"		LineItemT.PatientID AS PatID,   "
		"		PatientsT.UserDefinedID,  "
		"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"		LineItemT.InputDate AS IDate,   "
		"		LineItemT.Date AS TDate,   "
		"		PaymentsT.PayMethod,   "
		"		LineItemT.Description,   "
		"		0 AS ApplyID,   "
		"		LineItemT.ID AS LineID,  "
		"		LineItemT.LocationID AS LocID,  "
		"		LocationsT.Name AS Location  "
		"		FROM (( "
		"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
		"			Sum(AppliesT.Amount) AS ApplyAmt,   "
		"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
		"			LineItemT_1.PatientID   "
		"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
		"			ON LineItemT_1.ID = PaymentsT.ID  "
		"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
		"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
		"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
		"			) AS _PartiallyAppliedPaysQ  "
		"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID =  "
		"		PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
		"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
		"		) AS PaymentsByDatePartQ  "
		"	) AS PaymentsByDateSubQ  "
		"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
		"LEFT JOIN LocationsT ON PaymentsByDateSubQ.LocID = LocationsT.ID "
		" "
		"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate,  "
		"PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID,  "
		"PaymentsByDateSubQ.LocID, LocationsT.Name ");
		break;

	case 244:
		//Payments by Charge Date
		/*	Version History
			2/4/03 DRT - Added Item Code (cpt code / inv item(id)) filter, and fixed a problem with the filtering in it where it was doing the itemcode
					stuff all wrong (ChargesT.ItemCode, gah!)
			- TES 3/13/03: Changed to use charge location.
			m.cable - 4-22-03 - I changed the name of the filter on the "Payments by Charge Date" to be "Service Date" instead of "Payment Date"
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificates.
			TES 7/8/2005 - PLID 16215 - Added the bill date for filtering.
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 15:50) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,  "
		"AdjustmentsByCPTCodeFullQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First AS DocName,  "
		"AdjustmentsByCPTCodeFullQ.TDate AS TDate,  "
		"AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod,  "
		"AdjustmentsByCPTCodeFullQ.CCType,  "
		"AdjustmentsByCPTCodeFullQ.CheckNo,  "
		"AdjustmentsByCPTCodeFullQ.Description,  "
		"AdjustmentsByCPTCodeFullQ.ApplyAmount,  "
		"CategoriesQ.Category,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"AdjustmentsByCPTCodeFullQ.PatID AS PatID, "
		"AdjustmentsByCPTCodeFullQ.UserDefinedID,  "
		"AdjustmentsByCPTCodeFullQ.IDate AS IDate,  "
		"AdjustmentsByCPTCodeFullQ.Type, "
		"AdjustmentsByCPTCodeFullQ.ItemCode, "
		"AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, "
		"AdjustmentsByCPTCodeFullQ.ChargeAmount, "
		"AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, "
		"AdjustmentsByCPTCodeFullQ.ServiceDate, "
		"AdjustmentsByCPTCodeFullQ.ItemDesc, "
		"AdjustmentsByCPTCodeFullQ.LocID AS LocID, "
		"AdjustmentsByCPTCodeFullQ.Location, AdjustmentsByCPTCodeFullQ.PaymentDate, "
		"AdjustmentsByCPTCodeFullQ.BDate AS BDate "
		"FROM ((((SELECT LineItemT.ID,  "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
		"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  "
		"PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT_1.InputDate AS IDate,  "
		"LineItemT_1.Date AS TDate, "
		"LineItemT.Date AS PaymentDate, "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS ItemCode, "	//horrid naming, but theres already another one somewhere
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"PaymentPlansT.CheckNo,  "
		"ChargesT.Category,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"LineItemT.Type, "
		"ChargesT.ItemCode AS NoLongerUsedItemCode, "
		"ServiceT.ID AS CPTID, "	//this is not really CPTID, but I dont want to change the ttx file
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"BillsT.Date AS BDate "
		"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT_1.InputDate, LineItemT_1.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.ID, LineItemT.Date, ServiceT.ID, CPTCodeT.Code, BillsT.Date "
		"HAVING (((LineItemT.Type)=1)) "
		") AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID))");
		break;


	case 322:	//Batch Payments
	case 755:	// (s.tullis 2014-07-14 11:45) - PLID 62560 - Batch Vision Payments
	case 756:	// (s.tullis 2014-07-14 13:34) - PLID 62559 - Batch Medical Payments
		
		/*	Version History
			DRT 11/20/2003 - PLID 10126 - Fixed the report to no longer show the LineItem amount for all applies - but to show the Applied amount.
				(No change was made in the query - the apply amount was already selected).
			TES 12/10/2003 - PLID 10126 - Added a UNION to the report to pick up all the portions of payments which were
    			on patient's accounts, but not applied to a specific charge.
			TES 12/17/2003 - PLID 10126 - Moved the UNION query into a subquery because my previous changes, you know, 
				broke all the filters.  Picky!
			JMJ 3/31/2006 - PLID 19969 - Added ApplyID and LineItemID fields to the subquery because the UNION caused identical
			    rows to collapse upon themselves, so we needed to have a unique identifier to prevent that
			JMJ 4/9/2006 - PLID 18486 - Exposed ApplyID and LineItemID fields to the report
			JMJ 10/11/2006 - PLID 22955 - Supported Adjusted Batch Payments
			// (j.gruber 2007-02-20 11:41) - PLID 23979 - check number, BankRoutingNum, BankName, and CheckAcctNo upped version in reportInfoCallback
			// (j.jones 2007-05-04 14:05) - PLID 25837 - ensured we do not show deleted payments, and we label appropriately if no payments have been made
			// (j.jones 2008-05-01 10:05) - PLID 29076 - corrected PatID to be the patient's internal ID, and added a proper UserDefinedID field
			// (j.jones 2009-06-09 15:46) - PLID 34549 - ensured child adjustments are ignored
			// (j.jones 2009-07-06 17:15) - PLID 34776 - supported OriginalAmount
			// (j.jones 2009-09-25 11:55) - PLID 34453 - supported BatchPaymentOHIPMessagesT
			// (d.thompson 2011-09-23) - PLID 45648 - Added the location ID and Location Name of the charge the batch payment is applied to (for each charge applied)
			// (j.jones 2012-04-25 11:18) - PLID 49965 - Supported line item corrections that were takebacks, returning the value "
			// of the original payment to the batch payment. Also fixed to ignore payments that were voided (unless part of another batch payment's takeback).
			// When reporting the line items that credit the batch payment, we look for voided payments only, but actually show
			// the voiding adjustment value such that it displays as negative.
			// (z.manning 2013-05-06 11:12) - PLID 56559 - Fixed a couple where clauses so that this report no longer shows adjustments.
			// (j.jones 2013-09-05 14:50) - PLID 57831 - added adjustments that were posted on the same input date, applied to the same charge
			// (s.tullis 2014-09-09 15:50) - PLID 62561 - When running the existing Batch Payment report from the Reports module, include both Medical and Vision Batch Payments in the report. This will include chargeback payments.

		*/
		switch(nSubLevel) {
			case 1: {
				//show BatchPaymentOHIPMessagesT in a subreport

				CString str = "SELECT BatchPaymentsT.ID AS BatchPaymentID, BatchPaymentsT.Date AS TDate, BatchPaymentsT.InputDate AS IDate, "
					"BatchPaymentsT.ProviderID AS ProvID, BatchPaymentsT.Location AS LocID, BatchPaymentsT.InsuranceCoID AS InsuranceID, "
					"BatchPaymentOHIPMessagesT.Date AS TransactionDate, "
					"BatchPaymentOHIPMessagesT.Amount AS TransactionAmount, "
					"BatchPaymentOHIPMessagesT.ReasonCode, "
					"BatchPaymentOHIPMessagesT.Reason, "
					"BatchPaymentOHIPMessagesT.Message "
					"FROM BatchPaymentsT "
					"INNER JOIN BatchPaymentOHIPMessagesT ON BatchPaymentsT.ID = BatchPaymentOHIPMessagesT.BatchPaymentID "
					"WHERE BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Type = 1";

				CString strFilter = GetExtraValue();
				if(!strFilter.IsEmpty()) {
					CString strWhere;
					strWhere.Format(" AND BatchPaymentsT.ID IN (%s)", strFilter);
					str += strWhere;
				}

				return str;
				break;
			}

			case 0:
			default: {
				//main report

				CString str = "SELECT BatchPaymentsSubQ.BatchID AS BatchID, BatchPaymentsSubQ.ApplyID, BatchPaymentsSubQ.LineItemID AS ChildPaymentID, BatchPaymentsSubQ.BatchDesc AS BatchDesc, BatchPaymentsSubQ.BatchAmt AS BatchAmt, BatchPaymentsSubQ.TotalApplied AS TotalRefunded, BatchPaymentsSubQ.BatchRemaining AS BatchRemaining, BatchPaymentsSubQ.TDate AS TDate, BatchPaymentsSubQ.IDate AS IDate, "
					"BatchPaymentsSubQ.PayAmt AS PayAmt, BatchPaymentsSubQ.LineAmt AS LineAmt, BatchPaymentsSubQ.PayDate AS PayDate, "
					"BatchPaymentsSubQ.ApplyAmt AS ApplyAmt, BatchPaymentsSubQ.ChargeDescription AS ChargeDescription, "
					"BatchPaymentsSubQ.ProvName AS ProvName, BatchPaymentsSubQ.ProvID AS ProvID, "
					"BatchPaymentsSubQ.AppliedChargeProvName AS AppliedChargeProvName, BatchPaymentsSubQ.AppliedChargeProvID AS AppliedChargeProvID, "
					"BatchPaymentsSubQ.LocID AS LocID, BatchPaymentsSubQ.LocName AS LocName, BatchPaymentsSubQ.InsuranceID AS InsuranceID, BatchPaymentsSubQ.InsName AS InsName, "
					"BatchPaymentsSubQ.PatName AS PatName, BatchPaymentsSubQ.PatID AS PatID, BatchPaymentsSubQ.UserDefinedID, "
					"BatchPaymentsSubQ.CheckNo, BatchPaymentsSubQ.BankRoutingNum, "
					"BatchPaymentsSubQ.BankName, BatchPaymentsSubQ.CheckAcctNo, "
					"BatchPaymentsSubQ.OriginalAmount, "
					"CASE WHEN BatchPaymentsSubQ.PayType=0 THEN 'Adjustments & Refunds '  WHEN BatchPaymentsSubQ.PayType = 1 THEN 'Medical Payment' WHEN BatchPaymentsSubQ.PayType = 2 THEN 'Vision Payment' END AS PaymentType, "
					"(SELECT Count(*) FROM BatchPaymentOHIPMessagesT WHERE BatchPaymentOHIPMessagesT.BatchPaymentID = BatchPaymentsSubQ.BatchID) AS CountTransactionMessages, "
					"AppliedToLocationID, AppliedToLocationName, "
					"AdjustmentID, AdjustmentAmount, "
					"AdjustmentDescription, AdjustmentInputDate "
					"FROM "
					"(SELECT "
					"BatchPaymentsT.ID AS BatchID, AppliesT.ID AS ApplyID, LineItemQ.ID AS LineItemID, "
					"BatchPaymentsT.Description AS BatchDesc, BatchPaymentsT.Amount AS BatchAmt, "
					"BatchPayInfoQ.TotalApplied, BatchPayInfoQ.RemainingAmount AS BatchRemaining, "
					"BatchPaymentsT.Date AS TDate, BatchPaymentsT.InputDate AS IDate, "
					"Convert(money, LineItemQ.Amount * (CASE WHEN LineItemQ.IsReversal = 1 THEN -1 ELSE 1 END)) AS PayAmt, "
					"LineItemQ.Description AS LineAmt, LineItemQ.InputDate AS PayDate, "
					"Convert(money, (CASE WHEN AppliesT.Amount Is Null AND LineItemT1.ID Is Null THEN LineItemQ.Amount ELSE AppliesT.Amount END) * (CASE WHEN LineItemQ.IsReversal = 1 THEN -1 ELSE 1 END)) AS ApplyAmt, "
					"CASE WHEN LineItemQ.ID Is Null THEN '    <<  No Payments Made  >>   ' ELSE (CASE WHEN LineItemT1.ID IS NULL THEN (CASE WHEN LineItemQ.IsReversal = 1 THEN '    <<  Unapplied Reversal Payment  >>   ' ELSE '    <<  Unapplied Payment  >>   ' END) ELSE (CASE WHEN LineItemQ.IsReversal = 1 THEN '<< Reversal >> ' ELSE '' END) + LineItemT1.Description END) END AS ChargeDescription, "
					"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName, PersonT.ID AS ProvID, "
					"CASE WHEN AppliedToProvT.ID IS NULL THEN 'No Provider' ELSE AppliedToProvT.Last + ', ' + AppliedToProvT.First + ' ' + AppliedToProvT.Middle END AS AppliedChargeProvName, AppliedToProvT.ID AS AppliedChargeProvID, "
					"LocationsT.ID AS LocID, LocationsT.Name AS LocName, InsuranceCoT.PersonID AS InsuranceID, InsuranceCoT.Name AS InsName, "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "//s.tullis
					" BatchPaymentsT.CheckNo, BatchPaymentsT.BankRoutingNum, BatchPaymentsT.BankName, BatchPaymentsT.CheckAcctNo, BatchPaymentsT.OriginalAmount, BatchPaymentsT.PayType, "
					"LineItemT1.LocationID AS AppliedToLocationID, AppliedToLocsT.Name AS AppliedToLocationName, "
					"LineItemAdjustmentsQ.ID AS AdjustmentID, LineItemAdjustmentsQ.Amount AS AdjustmentAmount, "
					"LineItemAdjustmentsQ.Description AS AdjustmentDescription, LineItemAdjustmentsQ.InputDate AS AdjustmentInputDate "
					""
					"FROM BatchPaymentsT "
					//Left join all child payments AND the reversed payments that were credited to this batch payment.
					//It's a left join because this main query still returns the batch payment information when nothing is applied.
					//For child payments, show all corrections, but hide void line items if they were part of a reversal
					//for another batch payment, since they don't reduce this batch payment's balance.
					// (z.manning 2013-05-06 10:28) - PLID 56559 - I removed "OR LineItemCorrections_DisplayedVoidsQ.VoidingLineItemID Is Not Null"
					// from the where clause of this subquery because it was causing adjustment to show on this report. Voided
					// payments are still shown.
					"LEFT JOIN ("
					"	SELECT PaymentsT.ID, PaymentsT.BatchPaymentID, LineItemT.PatientID, "
					"	LineItemT.Amount, LineItemT.Description, LineItemT.InputDate, PaymentsT.InsuredPartyID, "
					"	0 AS IsReversal "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_DisplayedVoidsQ ON LineItemT.ID = LineItemCorrections_DisplayedVoidsQ.VoidingLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Not Null) AS LineItemCorrections_HiddenVoidsQ ON LineItemT.ID = LineItemCorrections_HiddenVoidsQ.VoidingLineItemID "
					"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"	AND LineItemCorrections_HiddenVoidsQ.VoidingLineItemID Is Null "
					"	UNION SELECT LineItemT.ID, LineItemCorrectionsT.BatchPaymentID, LineItemT.PatientID, "
					"	LineItemT.Amount AS Amount, '<< Reversal >> ' + LineItemT.Description AS Description, LineItemCorrectionsT.InputDate, NULL AS InsuredPartyID, "
					"	1 AS IsReversal "
					"	FROM LineItemT "
					"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
					"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					") AS LineItemQ ON BatchPaymentsT.ID = LineItemQ.BatchPaymentID "

					//find batch payment adjustments and batch payment refunds
					"INNER JOIN (SELECT BatchPaymentsT.ID, BatchPaymentsT.Amount, "
					"	(CASE WHEN AppliedPaymentsT.TotalApplied Is Null THEN 0 ELSE AppliedPaymentsT.TotalApplied END) AS TotalApplied, "
					"	Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
					"	BatchPaymentsT.Amount "
					"	 - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
					"	 + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
					"	 - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
					"	 AS RemainingAmount "
					"	FROM BatchPaymentsT "
					"	LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID FROM BatchPaymentsT WHERE Type <> 1 AND Deleted = 0 "
					"	GROUP BY AppliedBatchPayID, Deleted) AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
					"	LEFT JOIN (SELECT PaymentsT.* FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID WHERE Deleted = 0 AND Type = 1) AS PaymentsT ON BatchPaymentsT.ID = PaymentsT.BatchPaymentID "
					""
					//find child payments that are not voided, but include them if they are part of a takeback
					"	LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
					"		FROM LineItemT "
					"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
					"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
					"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"		AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
					"		AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
					"		AND PaymentsT.BatchPaymentID Is Not Null "
					"		GROUP BY PaymentsT.BatchPaymentID "
					"	) AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
					""
					//find payments that were part of takebacks, crediting this batch payment
					"	LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
					"		FROM LineItemT "
					"		INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
					"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"		AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
					"		GROUP BY LineItemCorrectionsT.BatchPaymentID "
					"	) AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
					""
					"	GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Amount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied "
					"	) AS BatchPayInfoQ ON BatchPaymentsT.ID = BatchPayInfoQ.ID "
					
					"LEFT JOIN AppliesT ON LineItemQ.ID = AppliesT.SourceID "

					// (j.jones 2013-09-05 14:50) - PLID 57831 - Find all adjustments that were posted on the same input date
					// as the child payment, applied to the same charge as the child payment.
					// This report won't show unapplied adjustments or reversed adjustments.
					"LEFT JOIN ("
					"	SELECT PaymentsT.ID, AppliesT_Adj.Amount, LineItemT.Description, LineItemT.InputDate, "
					"	AppliesT_Pay.ID AS PaymentApplyID, PaymentsT.InsuredPartyID "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
					"	LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
					"	INNER JOIN AppliesT AS AppliesT_Adj ON PaymentsT.ID = AppliesT_Adj.SourceID "
					"	INNER JOIN AppliesT AS AppliesT_Pay ON AppliesT_Adj.DestID = AppliesT_Pay.DestID "
					"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
					"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
					"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
					"	AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
					") AS LineItemAdjustmentsQ ON "
					"	AppliesT.ID = LineItemAdjustmentsQ.PaymentApplyID "
					"	AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, LineItemQ.InputDate))) = CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, LineItemAdjustmentsQ.InputDate))) "
					"	AND LineItemQ.IsReversal = 0 "
					"	AND Coalesce(LineItemQ.InsuredPartyID, -1) = Coalesce(LineItemAdjustmentsQ.InsuredPartyID, -1) "

					"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
					"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT1 ON ChargesT.ID = LineItemT1.ID "
					"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
					"LEFT JOIN LocationsT ON BatchPaymentsT.Location = LocationsT.ID "
					"LEFT JOIN PersonT ON BatchPaymentsT.ProviderID = PersonT.ID "
					"LEFT JOIN PersonT PersonT1 ON LineItemQ.PatientID = PersonT1.ID "
					"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
					"LEFT JOIN LocationsT AppliedToLocsT ON LineItemT1.LocationID = AppliedToLocsT.ID "
					"LEFT JOIN PersonT AppliedToProvT ON ChargesT.DoctorsProviders = AppliedToProvT.ID "
					"WHERE "
					"BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Type = 1 "
					//Batch PaymentsT unapplied amounts on patient accounts.
					"UNION SELECT "
					"BatchPaymentsT.ID AS BatchID, NULL AS ApplyID, LineItemQ.ID AS LineItemID, BatchPaymentsT.Description AS BatchDesc, BatchPaymentsT.Amount AS BatchAmt, BatchPayInfoQ.TotalApplied, BatchPayInfoQ.RemainingAmount AS BatchRemaining, BatchPaymentsT.Date AS TDate, BatchPaymentsT.InputDate AS IDate, "
					"Convert(money, LineItemQ.Amount * (CASE WHEN LineItemQ.IsReversal = 1 THEN -1 ELSE 1 END)) AS PayAmt, "
					"LineItemQ.Description AS LineAmt, LineItemQ.InputDate AS PayDate, "
					"Convert(money, (LineItemQ.Amount - AppliesQ.Amount) * (CASE WHEN LineItemQ.IsReversal = 1 THEN -1 ELSE 1 END)) AS ApplyAmt, "
					"CASE WHEN LineItemQ.IsReversal = 1 THEN '    <<  Unapplied Reversal Payment  >>   ' ELSE '    <<  Unapplied Payment  >>   ' END AS ChargeDescription, "
					"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName, PersonT.ID AS ProvID, "
					"'No Provider' AS AppliedChargeProvName, NULL AS AppliedChargeProvID, "
					"LocationsT.ID AS LocID, LocationsT.Name AS LocName, InsuranceCoT.PersonID AS InsuranceID, InsuranceCoT.Name AS InsName, "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
					" BatchPaymentsT.CheckNo, BatchPaymentsT.BankRoutingNum, BatchPaymentsT.BankName, BatchPaymentsT.CheckAcctNo, BatchPaymentsT.OriginalAmount, BatchPaymentsT.PayType, "
					"NULL AS AppliedToLocationID, NULL AS AppliedToLocationName, "
					"NULL AS AdjustmentID, NULL AS AdjustmentAmount, "
					"NULL AS AdjustmentDescription, NULL AS AdjustmentInputDate "
					""
					"FROM BatchPaymentsT "
					//Inner join all child payments AND the reversed payments that were credited to this batch payment.
					//It's an inner join because the prior query already returns the batch payment information at all times,
					//we don't need to include additional records unless they are unapplied.
					//For child payments, show all corrections, but hide void line items if they were part of a reversal
					//for another batch payment, since they don't reduce this batch payment's balance.
					// (z.manning 2013-05-06 10:28) - PLID 56559 - I removed "OR LineItemCorrections_DisplayedVoidsQ.VoidingLineItemID Is Not Null"
					// from the where clause of this subquery because it was causing adjustment to show on this report. Voided
					// payments are still shown.
					"INNER JOIN ("
					"	SELECT PaymentsT.ID, PaymentsT.BatchPaymentID, LineItemT.PatientID, "
					"	LineItemT.Amount, LineItemT.Description, LineItemT.InputDate, 0 AS IsReversal "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_DisplayedVoidsQ ON LineItemT.ID = LineItemCorrections_DisplayedVoidsQ.VoidingLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Not Null) AS LineItemCorrections_HiddenVoidsQ ON LineItemT.ID = LineItemCorrections_HiddenVoidsQ.VoidingLineItemID "
					"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"	AND LineItemCorrections_HiddenVoidsQ.VoidingLineItemID Is Null "
					"	UNION SELECT LineItemT.ID, LineItemCorrectionsT.BatchPaymentID, LineItemT.PatientID, "
					"	LineItemT.Amount AS Amount, '<< Reversal >> ' + LineItemT.Description AS Description, LineItemCorrectionsT.InputDate, 1 AS IsReversal "
					"	FROM LineItemT "
					"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
					"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					") AS LineItemQ ON BatchPaymentsT.ID = LineItemQ.BatchPaymentID "
					"INNER JOIN (SELECT BatchPaymentsT.ID, BatchPaymentsT.Amount, "
					"	(CASE WHEN AppliedPaymentsT.TotalApplied Is Null THEN 0 ELSE AppliedPaymentsT.TotalApplied END) AS TotalApplied, "
					"	Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
					"	BatchPaymentsT.Amount "
					"	 - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
					"	 + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
					"	 - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
					"	 AS RemainingAmount "
					"	FROM BatchPaymentsT "
					"	LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID FROM BatchPaymentsT WHERE Type <> 1 AND Deleted = 0 "
					"	GROUP BY AppliedBatchPayID, Deleted) AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
					"	LEFT JOIN (SELECT PaymentsT.* FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID WHERE Deleted = 0 AND Type <> 2) AS PaymentsT ON BatchPaymentsT.ID = PaymentsT.BatchPaymentID "
					""
					//find child payments that are not voided, but include them if they are part of a takeback
					"	LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
					"		FROM LineItemT "
					"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
					"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
					"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"		AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
					"		AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
					"		AND PaymentsT.BatchPaymentID Is Not Null "
					"		GROUP BY PaymentsT.BatchPaymentID "
					"	) AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
					""
					//find payments that were part of takebacks, crediting this batch payment
					"	LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
					"		FROM LineItemT "
					"		INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
					"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"		AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
					"		GROUP BY LineItemCorrectionsT.BatchPaymentID "
					"	) AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
					""
					"	GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Amount, "
					"	LineItemsInUseQ.TotalApplied, LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied "
					") AS BatchPayInfoQ ON BatchPaymentsT.ID = BatchPayInfoQ.ID "
					"LEFT JOIN (SELECT SourceID, Sum(Amount) AS Amount FROM AppliesT GROUP BY SourceID) AppliesQ ON LineItemQ.ID = AppliesQ.SourceID "
					"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
					"LEFT JOIN LocationsT ON BatchPaymentsT.Location = LocationsT.ID "
					"LEFT JOIN PersonT ON BatchPaymentsT.ProviderID = PersonT.ID "
					"LEFT JOIN PersonT PersonT1 ON LineItemQ.PatientID = PersonT1.ID "
					"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
					"WHERE "
					"BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Type = 1 "
					"AND (LineItemQ.Amount - AppliesQ.Amount <> 0)) "
					"AS BatchPaymentsSubQ";

				CString strFilter = GetExtraValue();
				if(!strFilter.IsEmpty()) {
					CString strWhere;
					strWhere.Format(" WHERE BatchPaymentsSubQ.BatchID IN (%s)", strFilter);
					str += strWhere;
				}
				// (s.tullis 2014-07-14 11:45) - PLID 62560 - Batch Vision Payments
				if (nID == 755) {

					if (!strFilter.IsEmpty()){
								
						str += " AND ((BatchPaymentsSubQ.PayType=2 OR BatchPaymentsSubQ.PayType=0))  ";
					}
					else{
						str += " Where ( (BatchPaymentsSubQ.PayType=2 OR BatchPaymentsSubQ.PayType=0)) ";
					}


				}
				// (s.tullis 2014-07-14 13:34) - PLID 62559 - Batch Medical Payments
				if (nID == 756) {

					if (!strFilter.IsEmpty()){

						str += " AND ( (BatchPaymentsSubQ.PayType=1 OR BatchPaymentsSubQ.PayType=0))  ";
					}
					else{
						str += " Where ((BatchPaymentsSubQ.PayType=1 OR BatchPaymentsSubQ.PayType=0))  ";
					}


				}
				

				

				
				return str;
				break;
			}
		}
		break;


	case 362:
		//Payments by Credit Card
		/*	Version History
			- TES 3/13/03: Changed to use charge location.
			DRT 7/16/03 - Added CC Number field to the query, made the report editable so anyone who wants to can 
					add it.
			DRT 9/3/03 - Added external filter for CC Type.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificates (because it filters on PayMethod = 3)
			// (j.gruber 2007-05-01 17:20) - PLID 25745 - only show the last 4 digits of the ccnumber
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CreditCard.
			(d.thompson 2010-11-08) - PLID 41379 - Include the QBMS credit card transaction account information as an editable field
			(d.thompson 2010-11-17) - PLID 41514 - Include Chase credit card account information as well.
			// (j.jones 2011-06-24 14:42) - PLID 17184 - added auth. code and exp. date
			// (r.goldschmidt 2014-02-03 10:31) - PLID 60510 - Reports of Payments by Credit Card can fail to display location name for payment.
			(c.haag 2015-09-09) - PLID 67194 - Supported Integrated Credit Card Processing. CardConnect_CreditTransactionT
			always has the original authorization so we join on that. CardConnect_SetupDataT contains the master list of merchant
			accounts.
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,  "
		"PaymentsByDateSubQ.PatientID,  "
		"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByDateSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByDateSubQ.PatID AS PatID,  "
		"PaymentsByDateSubQ.UserDefinedID, "
		"PaymentsByDateSubQ.FullName,  "
		"PaymentsByDateSubQ.IDate AS IDate,  "
		"PaymentsByDateSubQ.TDate AS TDate,  "
		"PaymentsByDateSubQ.PayMethod AS PayMethod,  "
		"PaymentsByDateSubQ.Description,  "
		"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByDateSubQ.LocID AS LocID, "
		"PaymentsByDateSubQ.Location, "
		"PaymentsByDateSubQ.CreditCard AS CreditCard, "
		"CASE WHEN Len(PaymentsByDateSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentsByDateSubQ.CCNumber, 4) END  as CCNumber, "
		"PaymentsByDateSubQ.CCAuthNo, PaymentsByDateSubQ.CCExpDate, "
		"ISNULL( ISNULL( QBMS_CreditTransactionsT.AccountID, Chase_CreditTransactionsT.AccountID), CardConnect_CreditTransactionT.AccountID) AS AccountID, "
		"ISNULL( ISNULL( QBMS_SetupData.Description, Chase_SetupDataT.Description), CardConnect_SetupDataT.Description) AS AccountDescription "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End, "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"CASE WHEN ASCII(SUBSTRING(CreditCardNamesT.CardName, 1, 1)) = 1 THEN '' ELSE CreditCardNamesT.CardName END AS CreditCard, "
		"PaymentPlansT.CCNumber, "
		"PaymentPlansT.CCAuthNo, "
		"PaymentPlansT.CCExpDate "
		"FROM ((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN (PaymentsT LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID) ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (LineItemT AS LineItemT_1 LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1) AND PaymentsT.PayMethod = 3) "
		") AS PaymentsByDateFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"CASE WHEN ASCII(SUBSTRING(CreditCardNamesT.CardName, 1, 1)) = 1 THEN '' ELSE CreditCardNamesT.CardName END  AS CreditCard, "
		"PaymentPlansT.CCNumber, "
		"PaymentPlansT.CCAuthNo, "
		"PaymentPlansT.CCExpDate "
		"FROM (((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN (PaymentsT LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID) ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1) AND PaymentsT.PayMethod = 3) "
		") AS PaymentsByDatePartQ) AS PaymentsByDateSubQ LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"LEFT JOIN QBMS_CreditTransactionsT ON PaymentsByDateSubQ.ID = QBMS_CreditTransactionsT.ID "
		"LEFT JOIN QBMS_SetupData ON QBMS_CreditTransactionsT.AccountID = QBMS_SetupData.ID "
		"LEFT JOIN Chase_CreditTransactionsT ON PaymentsByDateSubQ.ID = Chase_CreditTransactionsT.ID "
		"LEFT JOIN Chase_SetupDataT ON Chase_CreditTransactionsT.AccountID = Chase_SetupDataT.ID "
		"LEFT JOIN CardConnect_CreditTransactionT ON PaymentsByDateSubQ.ID = CardConnect_CreditTransactionT.ID "
		"LEFT JOIN CardConnect_SetupDataT ON CardConnect_CreditTransactionT.AccountID = CardConnect_SetupDataT.ID "
		"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID, PaymentsByDateSubQ.Location, PaymentsByDateSubQ.CreditCard, "
		"PaymentsByDateSubQ.CCNumber, PaymentsByDateSubQ.CCAuthNo, PaymentsByDateSubQ.CCExpDate, "
		"ISNULL( ISNULL( QBMS_CreditTransactionsT.AccountID, Chase_CreditTransactionsT.AccountID), CardConnect_CreditTransactionT.AccountID), "
		"ISNULL( ISNULL( QBMS_SetupData.Description, Chase_SetupDataT.Description), CardConnect_SetupDataT.Description)");
		break;

	case 366: //Payments Under Allowed Amount (by Insurance)
	{
		/*	Version History
			2/3/03 DRT - There's a little issue with reports where if you try to filter on a date, but use something like Min(TDate) AS TDate, it'll try to
					filter in the WHERE, but you can't filter a Min(TDate) >  in the WHERE, it must be in HAVING.  So I just worked around that by moving what
					used to be the main query inside a subquery and re-selected everything.
			DRT 6/2/2004 - PLID 12713 - Enabled "Create Merge Group" option.
			DRT 8/26/2004 - PLID 13974 - This report cannot show redeemed gift certificates because they cannot be included in multi fees.
			JMJ 5/26/2006 - PLID 20829 - Enabled support for quantity
			JMJ 5/26/2006 - PLID 20830 - Fixed bug where null copays prevented records from being returned
			JMJ 10/17/2006 - PLID 22173 - supported modifiers' multipliers
			JMJ 5/3/2007 - PLID 25840 - supported the IncludePatRespAllowable ability, which checks the patient responsibility for a charge
			// (j.jones 2009-10-22 16:28) - PLID 18558 - supported fee schedules by place of service
			// (j.gruber 2010-08-04 11:19) - PLID 39963 - change copay structure
			// (j.jones 2010-12-16 15:25) - PLID 41869 - the IncludePatRespAllowable setting now actually means
			// all responsibilities except the one being paid, so if this is Primary, we include patient & Secondary resps.
			// (j.jones 2010-12-17 15:50) - PLID 41870 - included applied adjustments, which require all remaining data
			// to be displayed as a group by ChargeID			
			// (j.jones 2011-03-16 13:38) - PLID 42570 - included applied payments, which could not be a subreport
			// because the result was unworkably slow
			// (j.jones 2011-03-17 15:54) - PLID 42158 - included additional charges from the same bill, also cannot be a subreport
			// (j.jones 2011-03-24 11:38) - PLID 25842 - added fields for the patient resp., insurance resp., and total of other
			// insurance resps.
			// (j.jones 2011-03-25 09:10) - PLID 42991 - exposed the fee schedule ID and name
			// (j.jones 2011-03-25 09:39) - PLID 25842 - the additional charges now show their ins. resp., ins. paid, ins. adjusted
			// (j.jones 2011-08-18 17:00) - PLID 45087 - additional charges now have pat. resp. and other resps.
			// (j.jones 2015-11-23 15:45) - PLID 67623 - we now only show payments for the same insured party as the charge's allowable insured party
			// (b.cardillo 2015-11-30 15:18) - PLID 67656 - Account for charge date relative to fee schedule effective date
		*/

		// (j.jones 2011-03-16 10:36) - PLID 21559 - E-Remittance can run this report
		// filtered by an EOBID, passed in as nExtraID
		_variant_t varEobID;
		if(nExtraID > 0) 
		{
			varEobID = _variant_t(nExtraID);
		}
		else
		{
			varEobID = g_cvarNull;
		}

		CString strSql = GetPaymentsUnderAllowedAmountReportQuery(varEobID);
		return _T(strSql);
		break;
	}

	case 374:	//Deleted Payments
	case 405:	//Deleted Adjustments
	case 406:	//Deleted Refunds
		{
		/*	Version History
			DRT 3/12/03 - Created
			DRT 7/21/03 - Added deleted adj/ref too, may as well use this query.
			// (j.gruber 2008-09-08 12:56) - PLID 27308 - made this filter by provider, the fields were already in the report
		*/
		long nType = 1;	//payment
		if(nID == 405)
			nType = 2;	//adjustment
		else if(nID == 406)
			nType = 3;	//refund

		CString str;
		str.Format("SELECT LineItemT.ID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, LineItemT.Description,  "
			"LineItemT.Amount, PaymentsT.ProviderID AS ProvID, LineItemT.Date, LineItemT.InputDate, LineItemT.DeletedBy, LineItemT.DeleteDate AS TDate, LocationsT.ID AS LocID,  "
			"LocationsT.Name AS LocName, ProvPersonT.Last + ', ' + ProvPersonT.First + ' ' + ProvPersonT.Middle AS ProvName "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ProvPersonT ON PaymentsT.ProviderID = ProvPersonT.ID "
			" "
			"WHERE LineItemT.Deleted = 1 AND LineItemT.Type = %li", nType);

		return _T(str);
		}
		break;

	case 376:
		//Payments By Patient Coordinator
		/*	Version History
			DRT 3/12/03 - Created - Payments by the Patient coordinator of the Bills they are applied to.
			DRT 8/26/2004 - PLID 13974 - Modified report file to include notes about redeemed gift
				certificates.  Also fixed a bug where a note said it was the pat coord of the Bill it was
				applied to, when the query plainly states that it is the pat coord of the Charge.
			DRT 10/22/2008 - PLID 31785 - Added Apply Date as an available field.  Added Apply Date, Charge Date, and Charge Input date
				as available date filter fields.
			TES 4/2/2012 - PLID 49124 - Added ServiceCategory
		*/
		return _T("SELECT ChargesT.BillID, ChargesT.ID AS ChargeID, PaymentsT.ID AS PaymentID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"CASE WHEN CoordPersonT.ID IS NULL THEN 'No Coordinator' ELSE CoordPersonT.Last + ', ' + CoordPersonT.First + ' ' + CoordPersonT.Middle END AS CoordName,  "
			"BillsT.PatCoord AS CoordID, LineItemT.Amount AS PayAmt, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"AppliesT.Amount AS AppliedAmount, BillsT.Date AS BillDate, BillsT.InputDate AS BillInputDate, LineChargesT.Date AS ChargeDate, LineChargesT.InputDate AS ChargeInputDate, "
			"ProvPersonT.Last + ', ' + ProvPersonT.First + ' ' + ProvPersonT.Middle AS ProvName, ProvPersonT.ID AS ProvID, LineChargesT.LocationID AS LocID,  "
			"LocationsT.Name AS LocName, LineItemT.Description, PaymentsT.Paymethod, AppliesT.InputDate AS ApplyDate, "
			"CategoriesT.Name AS ServiceCategory "
			" "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier = CPTModifierT2.Number "
			"LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID "
			"LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			" "
			"WHERE LineItemT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LineItemT.Type = 1 AND LineChargesT.Type = 10 "
			"AND AppliesT.ID IS NOT NULL AND BillsT.EntryType = 1");
		break;

	case 403: //Pays/Refs/Adjs by Input Name
		/*	Version History
			TES 7/17/03 - Created - Copied from PRA by Category, groups by input name.
			DRT 8/25/2004 - PLID 13974 - This report does NOT show gift certificate payments.
			(r.goldschmidt 2014-01-29 11:20) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
			"     AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID AS PatID,   "
			"    AdjustmentsSubQ.ProvID AS ProvID, SUM(AdjustmentsSubQ.Amount)   "
			"    AS Amount, MIN(AdjustmentsSubQ.RandomText)   "
			"    AS RandomText, AdjustmentsSubQ.PatName,   "
			"    AdjustmentsSubQ.IDate AS IDate, AdjustmentsSubQ.TDate AS TDate,   "
			"    CASE WHEN AdjustmentsSubQ.Type = 1 THEN (CASE WHEN AdjustmentsSubQ.PayMethod = 1 THEN 'Cash' ELSE CASE WHEN AdjustmentsSubQ.PayMethod = 2 THEN 'Check' ELSE  "
			"    CASE WHEN AdjustmentsSubQ.PayMethod = 3 THEN 'Credit Card' ELSE 'No Method' End End End)  "
			"         WHEN AdjustmentsSubQ.Type = 2 THEN ( 'Adjustment')  "
			"         WHEN AdjustmentsSubQ.Type = 3 THEN ( 'Refund') END AS Method,   "
			"	CASE WHEN AdjustmentsSubQ.PayMethod = 0 THEN CASE WHEN Type=2 THEN 4 ELSE 5 END ELSE PayMethod END AS PayMethod,  "
			"	AdjustmentsSubQ.Description,   "
			"    MIN(AdjustmentsSubQ.ApplyID) AS ApplyID,   "
			"    MIN(AdjustmentsSubQ.LineID) AS LineID,   "
			"    AdjustmentsSubQ.Type,  "
			"AdjustmentsSubQ.LocID AS LocID,  "
			"AdjustmentsSubQ.Location, PaymentGroupsT.ID AS GroupID, "
			"AdjustmentsSubQ.InputName AS IName "
			"FROM (SELECT *  "
			"      FROM (SELECT LineItemT.ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"                ProvID =   "
			"	CASE  "
			"		WHEN ChargesT.DoctorsProviders is Null then  "
			"			PaymentsT.ProviderID  "
			"		ELSE ChargesT.DoctorsProviders  "
			"	END,   "
			"        Amount =   "
			"	CASE   "
			"		WHEN _PartiallyAppliedPaysQ.ID is NULL then   "
			"		CASE  "
			"		WHEN LineItemT1.ID is Null then LineItemT.Amount  "
			"			Else AppliesT.Amount  "
			"		End  "
			"		ELSE AppliesT.Amount  "
			"		END, 'FULL' AS RandomText,   "
			"                PatientInfo.Last + ', ' + PatientInfo.First + ' ' + PatientInfo.Middle  "
			"                 AS PatName, LineItemT.InputDate AS IDate,   "
			"                LineItemT.Date AS TDate,   "
			"                PaymentsT.PayMethod, LineItemT.Description,   "
			"                AppliesT.ID AS ApplyID,   "
			"                LineItemT.ID AS LineID, LineItemT.Type, PaymentsT.PaymentGroupID as GroupID,  "
			"				 CASE WHEN LineItemT1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT1.LocationID END AS LocID, "
			"				 CASE WHEN LineItemT1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location,  "
			"				 LineItemT.InputName "
			"            FROM PatientsT LEFT OUTER JOIN  "
			"                PersonT AS PatientInfo ON   "
			"                PatientsT.PersonID = PatientInfo.ID RIGHT OUTER  "
			"                 JOIN  "
			"                    (SELECT LineItemT_1.ID,   "
			"                       LineItemT_1.Amount AS PayAmt,   "
			"                       SUM(AppliesT.Amount) AS ApplyAmt,   "
			"                       MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount]) AS Total,   "
			"                       LineItemT_1.PatientID  "
			"                  FROM LineItemT AS LineItemT_1 LEFT JOIN  "
			"                       (PaymentsT LEFT JOIN  "
			"                       (AppliesT LEFT JOIN  "
			"                       LineItemT ON   "
			"                       AppliesT.DestID = LineItemT.ID) ON   "
			"                       PaymentsT.ID = AppliesT.SourceID) ON   "
			"                       LineItemT_1.ID = PaymentsT.ID  "
			"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"                  GROUP BY LineItemT_1.ID,   "
			"                       LineItemT_1.Amount,   "
			"                       LineItemT_1.PatientID  "
			"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND   "
			"                       ((MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount])) <> 0)))   "
			"                _PartiallyAppliedPaysQ RIGHT OUTER JOIN  "
			"                (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON   "
			"                _PartiallyAppliedPaysQ.ID = LineItemT.ID LEFT OUTER  "
			"                 JOIN  "
			"                ProvidersT LEFT OUTER JOIN  "
			"                PersonT DocInfo ON   "
			"                ProvidersT.PersonID = DocInfo.ID RIGHT OUTER JOIN  "
			"                ChargesT ON   "
			"                ProvidersT.PersonID = ChargesT.DoctorsProviders  "
			"                 RIGHT OUTER JOIN  "
			"                (LineItemT LineItemT1 LEFT JOIN LocationsT AS ChargeLoc ON LineItemT1.LocationID = ChargeLoc.ID) ON   "
			"                ChargesT.ID = LineItemT1.ID RIGHT OUTER JOIN  "
			"                AppliesT ON   "
			"                LineItemT1.ID = AppliesT.DestID RIGHT OUTER JOIN  "
			"                PaymentsT ON   "
			"                AppliesT.SourceID = PaymentsT.ID ON   "
			"                LineItemT.ID = PaymentsT.ID ON   "
			"                PatientsT.PersonID = LineItemT.PatientID  "
			"            WHERE (LineItemT.Deleted = 0) AND   "
			"                (PaymentsT.ID IS NOT NULL) AND (PaymentsT.PayMethod NOT IN (4,10)))   "
			"          AS AdjustmentsFullQ  "
			"      UNION  "
			"      SELECT *  "
			"      FROM (SELECT PartiallyAppliedPaysQ.ID,   "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"                PaymentsT.ProviderID,   "
			"                PartiallyAppliedPaysQ.Total AS Amount,   "
			"                'PART' AS RandomText,   "
			"                PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			"                 AS PatName, LineItemT.InputDate AS IDate,   "
			"                LineItemT.Date AS TDate,   "
			"                PaymentsT.PayMethod, LineItemT.Description,   "
			"                0 AS ApplyID, LineItemT.ID AS LineID,   "
			"                LineItemT.Type, PaymentsT.PaymentGroupID as GroupID,  "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"LineItemT.InputName "
			"            FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN  "
			"                    (SELECT LineItemT_1.ID,   "
			"                       LineItemT_1.Amount AS PayAmt,   "
			"                       SUM(AppliesT.Amount) AS ApplyAmt,   "
			"                       MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount]) AS Total,   "
			"                       LineItemT_1.PatientID  "
			"                  FROM LineItemT AS LineItemT_1 LEFT JOIN  "
			"                       (PaymentsT LEFT JOIN  "
			"                       (AppliesT LEFT JOIN  "
			"                       LineItemT ON   "
			"                       AppliesT.DestID = LineItemT.ID) ON   "
			"                       PaymentsT.ID = AppliesT.SourceID) ON   "
			"                       LineItemT_1.ID = PaymentsT.ID  "
			"                  WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"                  GROUP BY LineItemT_1.ID,   "
			"                       LineItemT_1.Amount,   "
			"                       LineItemT_1.PatientID  "
			"                  HAVING (((LineItemT_1.ID) IS NOT NULL) AND   "
			"                       ((MIN([LineItemT_1].[Amount])   "
			"                       - SUM([AppliesT].[Amount])) <> 0)))   "
			"                PartiallyAppliedPaysQ ON   "
			"                LineItemT.ID = PartiallyAppliedPaysQ.ID INNER JOIN  "
			"                PaymentsT ON   "
			"                LineItemT.ID = PaymentsT.ID INNER JOIN  "
			"                PatientsT ON   "
			"                LineItemT.PatientID = PatientsT.PersonID INNER JOIN  "
			"                PersonT ON   "
			"                PatientsT.PersonID = PersonT.ID  "
			"            WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PayMethod NOT IN (4,10))) AS AdjustmentsPartQ)   "
			"    AdjustmentsSubQ LEFT OUTER JOIN  "
			"    ProvidersT ON   "
			"    AdjustmentsSubQ.ProvID = ProvidersT.PersonID LEFT OUTER JOIN  "
			"    PersonT ON ProvidersT.PersonID = PersonT.ID Left Join PaymentGroupsT on PaymentGroupsT.ID = AdjustmentsSubQ.GroupID  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"     AdjustmentsSubQ.ID, AdjustmentsSubQ.PatID,  "
			"AdjustmentsSubQ.UserDefinedID,   "
			"    AdjustmentsSubQ.ProvID, AdjustmentsSubQ.PatName,   "
			"    AdjustmentsSubQ.IDate, AdjustmentsSubQ.TDate,   "
			"    AdjustmentsSubQ.PayMethod, AdjustmentsSubQ.Description,   "
			"    AdjustmentsSubQ.Type, PaymentGroupsT.GroupName,  "
			"AdjustmentsSubQ.LocID, AdjustmentsSubQ.Location, PaymentGroupsT.ID, AdjustmentsSubQ.InputName ");

	case 412:
		//Payments by Referring Patient
		/*	Version History
			DRT 7/25/03 - Created.  Copied from Payments by date with a few modifications.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(r.goldschmidt 2014-01-28 16:22) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,  "
			"PaymentsByDateSubQ.PatientID,  "
			"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,  "
			"PaymentsByDateSubQ.ProvID AS ProvID,  "
			"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,  "
			"PaymentsByDateSubQ.PatID AS PatID,  "
			"PaymentsByDateSubQ.UserDefinedID, "
			"PaymentsByDateSubQ.FullName,  "
			"PaymentsByDateSubQ.IDate AS IDate,  "
			"PaymentsByDateSubQ.TDate AS TDate,  "
			"PaymentsByDateSubQ.PayMethod AS PayMethod,  "
			"PaymentsByDateSubQ.Description,  "
			"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,  "
			"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
			"PaymentsByDateSubQ.LocID AS LocID, "
			"PaymentsByDateSubQ.Location, RefPersonT.ID AS RefPatID, RefPersonT.Last + ', ' + RefPersonT.First + ' ' + RefPersonT.Middle AS RefPatName "
			"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
			"LineItemT.PatientID,  "
			"Amount = CASE "
			"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
			"	THEN CASE "
			"		WHEN [LineItemT_1].[ID] Is Null "
			"		THEN [LineItemT].[Amount] "
			"		ELSE [AppliesT].[Amount] "
			"		End "
			"	ELSE [AppliesT].[Amount] "
			"	End,  "
			"ProvID = CASE  "
			"	WHEN [DoctorsProviders] Is Null "
			"	THEN [ProviderID] "
			"	ELSE [DoctorsProviders] "
			"	End, "
			"'Full' AS RandomText,  "
			"LineItemT.PatientID AS PatID,  "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"LineItemT.InputDate AS IDate,  "
			"LineItemT.Date AS TDate,  "
			"PaymentsT.PayMethod,  "
			"LineItemT.Description,  "
			"AppliesT.ID AS ApplyID,  "
			"LineItemT.ID AS LineID, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
			"FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
			"Sum(AppliesT.Amount) AS ApplyAmt,  "
			"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
			"LineItemT_1.PatientID  "
			"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
			"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
			") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
			"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			") AS PaymentsByDateFullQ "
			"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
			"LineItemT.PatientID,  "
			"[_PartiallyAppliedPaysQ].Total AS Amount,  "
			"PaymentsT.ProviderID AS ProvID,  "
			"'Part' AS RandomText,  "
			"LineItemT.PatientID AS PatID,  "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"LineItemT.InputDate AS IDate,  "
			"LineItemT.Date AS TDate,  "
			"PaymentsT.PayMethod,  "
			"LineItemT.Description,  "
			"0 AS ApplyID,  "
			"LineItemT.ID AS LineID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
			"Sum(AppliesT.Amount) AS ApplyAmt,  "
			"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
			"LineItemT_1.PatientID  "
			"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
			"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
			") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			") AS PaymentsByDatePartQ) AS PaymentsByDateSubQ LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
			"LEFT JOIN PatientsT ON PaymentsByDateSubQ.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT RefPersonT ON PatientsT.ReferringPatientID = RefPersonT.ID "
			"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID, PaymentsByDateSubQ.Location, RefPersonT.ID, RefPersonT.Last, RefPersonT.First, RefPersonT.Middle ");
		break;


	case 417: //Payments by description...
		/*	Version History
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(r.goldschmidt 2014-01-28 16:49) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,  "
		"PaymentsByDateSubQ.PatientID,  "
		"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByDateSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByDateSubQ.PatID AS PatID,  "
		"PaymentsByDateSubQ.UserDefinedID, "
		"PaymentsByDateSubQ.FullName,  "
		"PaymentsByDateSubQ.IDate AS IDate,  "
		"PaymentsByDateSubQ.TDate AS TDate,  "
		"PaymentsByDateSubQ.PayMethod AS PayMethod,  "
		"PaymentsByDateSubQ.Description,  "
		"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByDateSubQ.LocID AS LocID, "
		"PaymentsByDateSubQ.Location "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End, "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS PaymentsByDateFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location "
		"FROM (((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS PaymentsByDatePartQ) AS PaymentsByDateSubQ LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID, PaymentsByDateSubQ.Location "
		"");
		break;

	case 425: //Patient Payments
		/*	Version History
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(r.goldschmidt 2014-01-28 16:25) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID,  "
		"PaymentsByDateSubQ.PatientID,  "
		"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByDateSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByDateSubQ.PatID AS PatID,  "
		"PaymentsByDateSubQ.UserDefinedID, "
		"PaymentsByDateSubQ.FullName,  "
		"PaymentsByDateSubQ.IDate AS IDate,  "
		"PaymentsByDateSubQ.TDate AS TDate,  "
		"PaymentsByDateSubQ.PayMethod AS PayMethod,  "
		"PaymentsByDateSubQ.Description,  "
		"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByDateSubQ.LocID AS LocID, "
		"PaymentsByDateSubQ.Location, "
		"PaymentsByDateSubQ.InsuredPartyID  "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"LineItemT.PatientID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE  "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End, "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"PaymentsT.InsuredPartyID  "
		"FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS PaymentsByDateFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"LineItemT.PatientID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"PaymentsT.InsuredPartyID "
		"FROM (((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS PaymentsByDatePartQ) AS PaymentsByDateSubQ LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"WHERE PaymentsByDateSubQ.InsuredPartyID = -1 "
		"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID, PaymentsByDateSubQ.Location, PaymentsByDateSubQ.InsuredPartyId");
		break;

	case 433:
		//Payments by Pay Category
		/*	Version History
			DRT 8/6/03 - Created.  Modeled after Payments by Date.
			DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
			(r.goldschmidt 2014-01-28 16:26) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount, PaymentsByDateSubQ.ProvID AS ProvID,   "
			"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText, PaymentsByDateSubQ.PatID AS PatID, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.FullName,   "
			"PaymentsByDateSubQ.IDate AS IDate, PaymentsByDateSubQ.TDate AS TDate, PaymentsByDateSubQ.PayMethod AS PayMethod, PaymentsByDateSubQ.Description,   "
			"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID, Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
			"PaymentsByDateSubQ.LocID AS LocID, PaymentsByDateSubQ.Location, PaymentsByDateSubQ.PayCatID AS PayCatID, PaymentsByDateSubQ.PayCat "
			"FROM ( "
			"	(SELECT * FROM (SELECT LineItemT.ID,   "
			"	LineItemT.PatientID,   "
			"	Amount = CASE  "
			"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
			"		THEN CASE  "
			"			WHEN [LineItemT_1].[ID] Is Null  "
			"			THEN [LineItemT].[Amount]  "
			"			ELSE [AppliesT].[Amount]  "
			"			End  "
			"		ELSE [AppliesT].[Amount]  "
			"		End,   "
			"	ProvID = CASE   "
			"		WHEN [DoctorsProviders] Is Null  "
			"		THEN [ProviderID]  "
			"		ELSE [DoctorsProviders]  "
			"		End,  "
			"	'Full' AS RandomText, LineItemT.PatientID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"	LineItemT.InputDate AS IDate, LineItemT.Date AS TDate, PaymentsT.PayMethod, LineItemT.Description, AppliesT.ID AS ApplyID,   "
			"	LineItemT.ID AS LineID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, PaymentGroupsT.ID AS PayCatID, PaymentGroupsT.GroupName AS PayCat "
			"	FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN  "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			"		Sum(AppliesT.Amount) AS ApplyAmt,   "
			"		/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"		LineItemT_1.PatientID   "
			"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"		ON LineItemT_1.ID = PaymentsT.ID  "
			"		WHERE LineItemT_1.Deleted=0 AND (PaymentsT.PayMethod NOT IN (4,10)) "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"		HAVING LineItemT_1.ID is not  Null AND MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) <> 0 "
			"		) AS _PartiallyAppliedPaysQ  "
			"	ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID)  "
			"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID)  "
			"	LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"	) AS PaymentsByDateFullQ  "
			" "
			"	UNION  "
			" "
			"	SELECT * FROM  "
			"	(SELECT [_PartiallyAppliedPaysQ].ID,   "
			"		LineItemT.PatientID, [_PartiallyAppliedPaysQ].Total AS Amount,  PaymentsT.ProviderID AS ProvID,   "
			"		'Part' AS RandomText, LineItemT.PatientID AS PatID, PatientsT.UserDefinedID,  "
			"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate, PaymentsT.PayMethod, LineItemT.Description,   "
			"		0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"		PaymentGroupsT.ID AS PayCatID, PaymentGroupsT.GroupName AS PayCat "
			"		FROM (( "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total, LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"			ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID)  "
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"		LEFT JOIn PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"		) AS PaymentsByDatePartQ "
			"	) AS PaymentsByDateSubQ  "
			"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
			"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate,  "
			"PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID,  "
			"PaymentsByDateSubQ.Location, PaymentsByDateSubQ.PayCatID, PaymentsByDateSubQ.PayCat ");
		break;


	case 465:

		/*
			Payments By Resp YTD
			/*	Version History
				- 1/26/04 JMM - Created
				DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
				// (r.gonet 2015-05-05 14:38) - PLID 66316 - Exclude Gift Certificate Refunds
			*/
		return _T("SELECT LineItemT.LocationID AS LocID, PaymentsT.ProviderID AS ProvID, InputDate AS IDate, Date AS TDate, Amount, InsuredPartyID, RIGHT(CONVERT(nVarChar, Date, 1),2) AS ServiceYear, RIGHT(CONVERT(nVarChar, InputDate, 1),2) AS InputYear, "
			" LEFT(CONVERT(nVarChar, Date, 1),2) + '/' + RIGHT(CONVERT(nVarChar, Date, 1),2) AS ServiceMonthYear, "
			" LEFT(CONVERT(nVarChar, InputDate, 1),2) + '/' + RIGHT(CONVERT(nVarChar, InputDate, 1),2) AS InputMonthYear "
			" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			" WHERE LineItemT.Type = 1 AND LineItemT.Deleted = 0 AND (PaymentsT.PayMethod NOT IN (4,10)) "); 
	break;

	case 466:

		/*Payments By Service Code YTD
			Version HIstory - 1-26-04 JMM - Created
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 16:31) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
	
		return _T(" SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,  "
			" AdjustmentsByCPTCodeFullQ.FullName,  	" 
			" PersonT.Last + ', ' + PersonT.First AS DocName,  	" 	
			" AdjustmentsByCPTCodeFullQ.TDate AS TDate,  	" 
			" AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod,  	" 
			" AdjustmentsByCPTCodeFullQ.CCType,  	" 
			" AdjustmentsByCPTCodeFullQ.CheckNo,  	" 
			" AdjustmentsByCPTCodeFullQ.Description,  	" 
			" AdjustmentsByCPTCodeFullQ.ApplyAmount,  	" 
			" CategoriesQ.Category,  	" 
			" CategoriesQ.SubCategory,  	" 
			" CategoriesQ.ParentID AS ParentID,  	" 
			" AdjustmentsByCPTCodeFullQ.PatID AS PatID, 	" 
			" AdjustmentsByCPTCodeFullQ.UserDefinedID,  	" 
			" AdjustmentsByCPTCodeFullQ.IDate AS IDate,  	" 
			" AdjustmentsByCPTCodeFullQ.Type, 	" 
			" AdjustmentsByCPTCodeFullQ.ItemCode, 	" 
			" AdjustmentsByCPTCodeFullQ.CPTID AS CPTID, 	" 
			" AdjustmentsByCPTCodeFullQ.ChargeAmount, 	" 
			" AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount, 	" 
			" AdjustmentsByCPTCodeFullQ.ServiceDate, 	" 
			" AdjustmentsByCPTCodeFullQ.ItemDesc, 	" 
			" AdjustmentsByCPTCodeFullQ.LocID AS LocID, 	" 
			" AdjustmentsByCPTCodeFullQ.Location,	" 
			" LEFT(CONVERT(nVarChar, TDate, 1),2) + '/' + RIGHT(CONVERT(nVarChar, TDate, 1),2) AS ServiceMonthYear,	" 
			" LEFT(CONVERT(nVarChar, IDate, 1),2) + '/' + RIGHT(CONVERT(nVarChar, IDate, 1),2) AS InputMonthYear,	" 
			" RIGHT(CONVERT(nVarChar, TDate, 1),2) AS ServiceYear, RIGHT(CONVERT(nVarChar, IDate, 1),2) AS InputYear	" 
			" FROM ((((SELECT LineItemT.ID,  	" 
			" ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  	" 
			" CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  	" 
			" PatientsT.PersonID AS PatID,  	" 
			" PatientsT.UserDefinedID, 	" 
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  	" 	
			" LineItemT.InputDate AS IDate,  	" 
			" LineItemT.Date AS TDate,  	" 
			" PaymentsT.PayMethod,  	" 
			" LineItemT.Description,  	" 	
			" ChargesT.ItemCode AS FirstOfItemCode,  	" 
			" LineItemT.Amount AS PayAmt,  	" 
			" dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  	" 
			" LineItemT_1.Date AS ServiceDate,  	" 
			" LineItemT_1.Description AS ItemDesc,  	" 
			" CreditCardNamesT.CardName AS CCType,  	" 
			" PaymentPlansT.CheckNo,  	" 
			" ChargesT.Category,  	" 
			" AppliesT.ID AS ApplyID,  	" 
			" LineItemT.ID AS LineID,  	" 
			" LineItemT.Type, 	" 
			" ChargesT.ItemCode, 	" 
			" CPTCodeT.ID AS CPTID, 	" 
			" CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, 	" 
			" CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location 	" 
			" FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  	" 
			" Sum(AppliesT.Amount) AS ApplyAmt,  	" 
			" /*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  	" 
			" LineItemT_1.PatientID  	" 
			" FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID 	" 
			" WHERE (((LineItemT_1.Deleted)=0)) 	" 
			" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID 	" 
			" HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) 	" 
			" ) AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID 	" 
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND CPTCodeT.ID IS NOT NULL 	" 
			" GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.ID 	" 
			" HAVING (((LineItemT.Type)=1)) 	" 
			" ) AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID 	" 
			" FROM CategoriesT 	" 
			" WHERE (((CategoriesT.Parent)=0)) 	" 
			" UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID 	" 
			" FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent 	" 
			" WHERE (((SubCategoriesT.Parent)<>0)) 	" 
			" ) AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID)) ");
		break;

		case 467:

		/* Payments By Service Category YTD
			Version History - 1-26-04 JMM - Created
			TES 5/24/04 - Added ChargeDescription as an editable field.
			DRT 8/26/2004 - PLID 13974 - This report does not include payments applied to gift certificates sold (charges
				with the ServiceID in GCTypesT)
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 16:32) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT PaymentsByCPTCategoryFullQ.ProvID AS ProvID,  "
		"PaymentsByCPTCategoryFullQ.FullName,  "
		"PersonT.Last + ', ' + PersonT.First AS DocName,  "
		"PaymentsByCPTCategoryFullQ.TDate AS TDate,  "
		"PaymentsByCPTCategoryFullQ.PayMethod,  "
		"PaymentsByCPTCategoryFullQ.CCType,  "
		"PaymentsByCPTCategoryFullQ.CheckNo,  "
		"PaymentsByCPTCategoryFullQ.Description,  "
		"PaymentsByCPTCategoryFullQ.ApplyAmount,  "
		"CategoriesQ.Category,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"PaymentsByCPTCategoryFullQ.PatID AS PatID, "
		"PaymentsByCPTCategoryFullQ.UserDefinedID,  "
		"PaymentsByCPTCategoryFullQ.IDate AS IDate,  "
		"PaymentsByCPTCategoryFullQ.Type, "
		"CategoriesQ.CategoryID, "
		"PaymentsByCPTCategoryFullQ.LocID AS LocID, "
		"PaymentsByCPTCategoryFullQ.Location, CatFilterID AS CatFilterID, "
		" LEFT(CONVERT(nVarChar, TDate, 1),2) + '/' + RIGHT(CONVERT(nVarChar, TDate, 1),2) AS ServiceMonthYear,	" 
		" LEFT(CONVERT(nVarChar, IDate, 1),2) + '/' + RIGHT(CONVERT(nVarChar, IDate, 1),2) AS InputMonthYear,	" 
		" RIGHT(CONVERT(nVarChar, TDate, 1),2) AS ServiceYear, RIGHT(CONVERT(nVarChar, IDate, 1),2) AS InputYear,	" 
		"PaymentsByCPTCategoryFullQ.ItemDesc AS ChargeDescription "
		"FROM ((((SELECT LineItemT.ID,  "
		"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
		"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  "
		"PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"ChargesT.ItemCode AS FirstOfItemCode,  "
		"LineItemT.Amount AS PayAmt,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
		"LineItemT_1.Date AS ServiceDate,  "
		"LineItemT_1.Description AS ItemDesc,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"PaymentPlansT.CheckNo,  "
		"ServiceT.Category,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"LineItemT.Type, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, ServiceT.Category AS CatFilterID "
		"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
		") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND GCTypesT.ServiceID IS NULL "
		"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ServiceT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END "
		"HAVING (((LineItemT.Type)=1)) "
		") AS PaymentsByCPTCategoryFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0)) "
		") AS CategoriesQ ON PaymentsByCPTCategoryFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON PaymentsByCPTCategoryFullQ.ProvID = PersonT.ID))");
		break;

		case 471://Payments by Payment Location
		case 553://Refunds by Refund Location
			/*	Version History
			- JMM 02-09-04  - Created
			DRT 8/25/2004 - PLID 13974 - This report does NOT show gift certificate payments.
			TES 4/20/2005 - PLID 15931,15932 - Added Adjustments by Adjustment Location and Refunds by Refund location
			// (j.gruber 2007-05-30 10:07) - PLID 26176 - split out from pay by pay loc and ref by ref loc
			// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds
			*/
		{
			CString strType;
			if(nID == 471) strType = "1";
			else if(nID == 553) strType = "3";
			else {ASSERT(FALSE); strType = "-1";}

			CString strSql;
			strSql.Format("SELECT PaymentsByDateSubQ.ID,   "
				"PaymentsByDateSubQ.PatientID,   "
				"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,   "
				"PaymentsByDateSubQ.ProvID AS ProvID,   "
				"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,   "
				"PaymentsByDateSubQ.PatID AS PatID,   "
				"PaymentsByDateSubQ.UserDefinedID,  "
				"PaymentsByDateSubQ.FullName,   "
				"PaymentsByDateSubQ.IDate AS IDate,   "
				"PaymentsByDateSubQ.TDate AS TDate,   "
				"PaymentsByDateSubQ.PayMethod AS PayMethod,   "
				"PaymentsByDateSubQ.Description,   "
				"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,   "
				"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,   "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
				"PaymentsByDateSubQ.LocID AS LocID,  "
				"LocationsT.Name "
				"FROM ( "
				"   (SELECT * FROM  "
				" "
				"	(SELECT LineItemT.ID,   "
				"	LineItemT.PatientID,   "
				"	Amount = CASE  "
				"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
				"		THEN CASE  "
				"			WHEN [LineItemT_1].[ID] Is Null  "
				"			THEN [LineItemT].[Amount]  "
				"			ELSE [AppliesT].[Amount]  "
				"			End  "
				"		ELSE [AppliesT].[Amount]  "
				"		End,   "
				"	ProvID = CASE   "
				"		WHEN [DoctorsProviders] Is Null  "
				"		THEN [ProviderID]  "
				"		ELSE [DoctorsProviders]  "
				"		End,  "
				"	'Full' AS RandomText,   "
				"	LineItemT.PatientID AS PatID,   "
				"	PatientsT.UserDefinedID,  "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
				"	LineItemT.InputDate AS IDate,   "
				"	LineItemT.Date AS TDate,   "
				"	PaymentsT.PayMethod,   "
				"	LineItemT.Description,   "
				"	AppliesT.ID AS ApplyID,   "
				"	LineItemT.ID AS LineID,  "
				"	LineItemT.LocationID AS LocID,  "
				"	LocationsT.Name AS Location "
				"	FROM (((((LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN  "
				"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
				"		Sum(AppliesT.Amount) AS ApplyAmt,   "
				"		/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
				"		LineItemT_1.PatientID   "
				"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON  "
				"		LineItemT_1.ID = PaymentsT.ID  "
				"		WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
				"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
				"		HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
				"		) AS _PartiallyAppliedPaysQ  "
				"	ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (LineItemT AS LineItemT_1 LEFT JOIN LocationsT ON  "
				"	LineItemT_1.LocationID = LocationsT.ID) ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON  "
				"	PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
				"	WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=%s)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"	) AS PaymentsByDateFullQ  "
				"	 "
				"	UNION  "
				" "
				"	SELECT * FROM  "
				"		(SELECT [_PartiallyAppliedPaysQ].ID,   "
				"		LineItemT.PatientID,   "
				"		[_PartiallyAppliedPaysQ].Total AS Amount,   "
				"		PaymentsT.ProviderID AS ProvID,   "
				"		'Part' AS RandomText,   "
				"		LineItemT.PatientID AS PatID,   "
				"		PatientsT.UserDefinedID,  "
				"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
				"		LineItemT.InputDate AS IDate,   "
				"		LineItemT.Date AS TDate,   "
				"		PaymentsT.PayMethod,   "
				"		LineItemT.Description,   "
				"		0 AS ApplyID,   "
				"		LineItemT.ID AS LineID,  "
				"		LineItemT.LocationID AS LocID,  "
				"		LocationsT.Name AS Location "
				"		FROM (( "
				"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
				"			Sum(AppliesT.Amount) AS ApplyAmt,   "
				"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
				"			LineItemT_1.PatientID   "
				"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
				"			ON LineItemT_1.ID = PaymentsT.ID  "
				"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
				"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
				"			) AS _PartiallyAppliedPaysQ  "
				"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID =  "
				"		PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
				"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=%s)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"		) AS PaymentsByDatePartQ  "
				"	) AS PaymentsByDateSubQ  "
				"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
				"LEFT JOIN LocationsT ON PaymentsByDateSubQ.LocID = LocationsT.ID "
				" "
				"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate,  "
				"PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID,  "
				"PaymentsByDateSubQ.LocID, LocationsT.Name ", strType, strType);
			return _T(strSql);
		}
		break;


		case 552://Adjustments by Adjustment Location
		/*	Version History
			- JMM 02-09-04  - Created
			DRT 8/25/2004 - PLID 13974 - This report does NOT show gift certificate payments.
			TES 4/20/2005 - PLID 15931,15932 - Added Adjustments by Adjustment Location and Refunds by Refund location
			// (j.gruber 2007-05-29 16:26) - PLID 24837 - added group codes and reason codes 
			// (j.gruber 2007-05-30 10:07) - PLID 26176 - split out from pay by pay loc and ref by ref loc
			// (j.jones 2010-09-27 08:50) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds
		*/
		
		{
			CString strSql;
			strSql.Format("SELECT PaymentsByDateSubQ.ID,   "
				"PaymentsByDateSubQ.PatientID,   "
				"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,   "
				"PaymentsByDateSubQ.ProvID AS ProvID,   "
				"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,   "
				"PaymentsByDateSubQ.PatID AS PatID,   "
				"PaymentsByDateSubQ.UserDefinedID,  "
				"PaymentsByDateSubQ.FullName,   "
				"PaymentsByDateSubQ.IDate AS IDate,   "
				"PaymentsByDateSubQ.TDate AS TDate,   "
				"PaymentsByDateSubQ.PayMethod AS PayMethod,   "
				"PaymentsByDateSubQ.Description,   "
				"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,   "
				"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,   "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
				"PaymentsByDateSubQ.LocID AS LocID,  "
				"LocationsT.Name, "
				"PaymentsByDateSubQ.ReasonCode, PaymentsByDateSubQ.GroupCode, "
				"PaymentsByDateSubQ.ReasonCodeDesc, PaymentsByDateSubQ.GroupCodeDesc "
				"FROM ( "
				"   (SELECT * FROM  "
				" "
				"	(SELECT LineItemT.ID,   "
				"	LineItemT.PatientID,   "
				"	Amount = CASE  "
				"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
				"		THEN CASE  "
				"			WHEN [LineItemT_1].[ID] Is Null  "
				"			THEN [LineItemT].[Amount]  "
				"			ELSE [AppliesT].[Amount]  "
				"			End  "
				"		ELSE [AppliesT].[Amount]  "
				"		End,   "
				"	ProvID = CASE   "
				"		WHEN [DoctorsProviders] Is Null  "
				"		THEN [ProviderID]  "
				"		ELSE [DoctorsProviders]  "
				"		End,  "
				"	'Full' AS RandomText,   "
				"	LineItemT.PatientID AS PatID,   "
				"	PatientsT.UserDefinedID,  "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
				"	LineItemT.InputDate AS IDate,   "
				"	LineItemT.Date AS TDate,   "
				"	PaymentsT.PayMethod,   "
				"	LineItemT.Description,   "
				"	AppliesT.ID AS ApplyID,   "
				"	LineItemT.ID AS LineID,  "
				"	LineItemT.LocationID AS LocID,  "
				"	LocationsT.Name AS Location, "
				"	AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
				"	AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
				"	FROM (((((((LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
				"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
				"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
				"		LEFT JOIN  "
				"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
				"		Sum(AppliesT.Amount) AS ApplyAmt,   "
				"		/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
				"		LineItemT_1.PatientID   "
				"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON  "
				"		LineItemT_1.ID = PaymentsT.ID  "
				"		WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
				"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
				"		HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
				"		) AS _PartiallyAppliedPaysQ  "
				"	ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (LineItemT AS LineItemT_1 LEFT JOIN LocationsT ON  "
				"	LineItemT_1.LocationID = LocationsT.ID) ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON  "
				"	PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
				"	WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"	) AS PaymentsByDateFullQ  "
				"	 "
				"	UNION  "
				" "
				"	SELECT * FROM  "
				"		(SELECT [_PartiallyAppliedPaysQ].ID,   "
				"		LineItemT.PatientID,   "
				"		[_PartiallyAppliedPaysQ].Total AS Amount,   "
				"		PaymentsT.ProviderID AS ProvID,   "
				"		'Part' AS RandomText,   "
				"		LineItemT.PatientID AS PatID,   "
				"		PatientsT.UserDefinedID,  "
				"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
				"		LineItemT.InputDate AS IDate,   "
				"		LineItemT.Date AS TDate,   "
				"		PaymentsT.PayMethod,   "
				"		LineItemT.Description,   "
				"		0 AS ApplyID,   "
				"		LineItemT.ID AS LineID,  "
				"		LineItemT.LocationID AS LocID,  "
				"		LocationsT.Name AS Location, "
				"		AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
				"		AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
				"		FROM (( "
				"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
				"			Sum(AppliesT.Amount) AS ApplyAmt,   "
				"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
				"			LineItemT_1.PatientID   "
				"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
				"			ON LineItemT_1.ID = PaymentsT.ID  "				
				"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
				"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
				"			) AS _PartiallyAppliedPaysQ  "
				"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID =  "
				"		PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
				"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
				"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
				"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
				"		) AS PaymentsByDatePartQ  "
				"	) AS PaymentsByDateSubQ  "
				"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
				"LEFT JOIN LocationsT ON PaymentsByDateSubQ.LocID = LocationsT.ID "
				" "
				"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate,  "
				"PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID,  "
				"PaymentsByDateSubQ.LocID, LocationsT.Name, "
				"PaymentsByDateSubQ.ReasonCode, PaymentsByDateSubQ.GroupCode, "
				"PaymentsByDateSubQ.ReasonCodeDesc, PaymentsByDateSubQ.GroupCodeDesc ");
			return _T(strSql);
		}
		break;

		case 476:	//Payments/Refunds with Check and Credit Card Numbers
			/*	Version History
				DRT 2/17/2004 - PLID 10658 - Created.
				DRT 8/25/2004 - PLID 13974 - This report does NOT include gift certificate payments.
				// (j.gruber 2007-05-01 17:21) - PLID 25745 - only show the last 4 digits of the ccnumber
				// (j.gruber 2007-05-15 09:23) - PLID 25987 - blank of CCExpDate
				(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
				(r.goldschmidt 2014-01-28 16:34) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
				// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds
			*/
			return _T("SELECT PaymentsSubQ.ID, PaymentsSubQ.PatientID, Sum(PaymentsSubQ.Amount) AS SumOfAmount, PaymentsSubQ.ProvID AS ProvID,  PaymentsSubQ.PatID AS PatID,  "
			"PaymentsSubQ.UserDefinedID, PaymentsSubQ.FullName,  PaymentsSubQ.IDate AS IDate, PaymentsSubQ.TDate AS TDate, PaymentsSubQ.PayMethod, PaymentsSubQ.Description,  "
			"PaymentsSubQ.CheckNo, PaymentsSubQ.BankNo, PaymentsSubQ.CheckAcctNo, PaymentsSubQ.CCType, CASE WHEN Len(PaymentsSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentsSubQ.CCNumber, 4) END  as CCNumber, PaymentsSubQ.CCHoldersName, Convert(datetime, NULL) AS CCExpDate,  "
			"PaymentsSubQ.CCAuthNo, PaymentsSubQ.BankRoutingNum, Min(PaymentsSubQ.ApplyID) AS FirstOfApplyID, Min(PaymentsSubQ.LineID) AS FirstOfLineID,  "
			"Last + ', ' + First + ' ' + Middle AS DocName, PaymentsSubQ.LocID AS LocID, PaymentsSubQ.Location  "
			"FROM  "
			"	(SELECT * FROM  "
			"		(SELECT LineItemT.ID, LineItemT.PatientID, CASE WHEN _PartiallyAppliedPaysQ.ID Is NULL THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount  "
			"		ELSE AppliesT.Amount End ELSE [AppliesT].[Amount] End AS Amount,   "
			"		CASE  WHEN [DoctorsProviders] Is NULL THEN PaymentsT.ProviderID ELSE [DoctorsProviders] End AS ProvID, 'Full' AS RandomText, PatientsT.PersonID AS PatID,   "
			"		PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, LineItemT.InputDate AS IDate, LineItemT.Date AS TDate,   "
			"		PaymentsT.PayMethod, LineItemT.Description, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber,  "
			"		PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID, "
			"		CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"		CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location  "
			"		FROM LineItemT "
			"		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"		LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"		LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"		LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID	/* LineItemT_1 is the destination of the apply*/ "
			"		LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID "
			"		LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID "
			"		LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"		LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "	
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"		LEFT JOIN "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total, LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1  "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			WHERE LineItemT_1.Deleted=0 AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING LineItemT_1.ID is not  Null AND (MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) <> 0) "
			"			) AS _PartiallyAppliedPaysQ  "
			"		ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID  "
			"		WHERE PaymentsT.ID Is Not NULL AND LineItemT.Deleted=0 AND (LineItemT.Type=1 OR LineItemT.Type = 3) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			"		) AS _PaymentsFullQ  "
			"	UNION  "
			" "
			"	SELECT * FROM  "
			"		(SELECT [_PartiallyAppliedPaysQ].ID, LineItemT.PatientID, [_PartiallyAppliedPaysQ].Total AS Amount, PaymentsT.ProviderID AS ProvID, 'Part' AS RandomText,   "
			"		PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate, PaymentsT.PayMethod, LineItemT.Description, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber,  "
			"		PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, 0 AS ApplyID, LineItemT.ID AS LineID,  "
			"		LineItemT.LocationID AS LocID, LocationsT.Name AS Location  "
			"		FROM LineItemT "
			"		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"		LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"		LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"		LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"		INNER JOIN  "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total, LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1  "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING LineItemT_1.ID is not  Null AND (MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) <> 0) "
			"			) AS _PartiallyAppliedPaysQ  "
			"		ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID "
			"		WHERE LineItemT.Deleted=0 AND (LineItemT.Type=1 OR LineItemT.Type = 3) AND (PaymentsT.PayMethod NOT IN (4,10)) "
			"		) AS _PaymentsPartQ "
			"	) AS PaymentsSubQ  "
			"LEFT JOIN ProvidersT ON PaymentsSubQ.ProvID = ProvidersT.PersonID  "
			"LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
			"GROUP BY PaymentsSubQ.ID, PaymentsSubQ.PatientID, PaymentsSubQ.ProvID, PaymentsSubQ.PatID, PaymentsSubQ.UserDefinedID, PaymentsSubQ.FullName,  "
			"PaymentsSubQ.IDate, PaymentsSubQ.TDate, PaymentsSubQ.PayMethod, PaymentsSubQ.Description, PaymentsSubQ.CheckNo, PaymentsSubQ.BankNo,  "
			"PaymentsSubQ.CheckAcctNo, PaymentsSubQ.CCType, PaymentsSubQ.CCNumber, PaymentsSubQ.CCHoldersName, PaymentsSubQ.CCExpDate,  "
			"PaymentsSubQ.CCAuthNo, PaymentsSubQ.BankRoutingNum, Last + ', ' + First + ' ' + Middle, PaymentsSubQ.LocID, PaymentsSubQ.Location "
			"HAVING (PaymentsSubQ.CheckNo IS NOT NULL AND PaymentsSubQ.CheckNo <> '') OR (PaymentsSubQ.CCNumber IS NOT NULL AND PaymentsSubQ.CCNumber <> '')");
			break;


		case 478:

			/* Payments by Place of Service
			Version History 
				2-23-04 JMM - Created
				DRT 8/26/2004 - PLID 13974 - Modified report file to include notes about redeemed gift
					certificates.
				JMM 03/10/2005 - PLID 15908 - Modified report to always show payment location
			*/
			return _T("SELECT PaymentsByDateSubQ.ID,   "
			"PaymentsByDateSubQ.PatientID,   "
			"Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,   "
			"PaymentsByDateSubQ.ProvID AS ProvID,   "
			"Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,   "
			"PaymentsByDateSubQ.PatID AS PatID,   "
			"PaymentsByDateSubQ.UserDefinedID,  "
			"PaymentsByDateSubQ.FullName,   "
			"PaymentsByDateSubQ.IDate AS IDate,   "
			"PaymentsByDateSubQ.TDate AS TDate,   "
			"PaymentsByDateSubQ.PayMethod AS PayMethod,   "
			"PaymentsByDateSubQ.Description,   "
			"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID,   "
			"Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  "
			"PaymentsByDateSubQ.LocID AS LocID,  "
			"LocationsT.Name, "
			"POSCode as POSCode, POSName, "
			" PaymentLocationID, PaymentLocation "
			"FROM ( "
			"   (SELECT * FROM  "
			" "
			"	(SELECT LineItemT.ID,   "
			"	LineItemT.PatientID,   "
			"	Amount = CASE  "
			"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
			"		THEN CASE  "
			"			WHEN [LineItemT_1].[ID] Is Null  "
			"			THEN [LineItemT].[Amount]  "
			"			ELSE [AppliesT].[Amount]  "
			"			End  "
			"		ELSE [AppliesT].[Amount]  "
			"		End,   "
			"	ProvID = CASE   "
			"		WHEN [DoctorsProviders] Is Null  "
			"		THEN [ProviderID]  "
			"		ELSE [DoctorsProviders]  "
			"		End,  "
			"	'Full' AS RandomText,   "
			"	LineItemT.PatientID AS PatID,   "
			"	PatientsT.UserDefinedID,  "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"	LineItemT.InputDate AS IDate,   "
			"	LineItemT.Date AS TDate,   "
			"	PaymentsT.PayMethod,   "
			"	LineItemT.Description,   "
			"	AppliesT.ID AS ApplyID,   "
			"	LineItemT.ID AS LineID,  "
			"	CASE WHEN LineItemT_1.LocationID IS NULL THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			"	LocationsT.Name AS Location,  "
			"	POS.Name as POSName, "
			"	POS.ID AS POSCode, "
			"   LineItemT.LocationID AS PaymentLocationID, "
			"   PayLocsT.Name as PaymentLocation "
			"	FROM (((((LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN  "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			"		Sum(AppliesT.Amount) AS ApplyAmt,   "
			"		/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"		LineItemT_1.PatientID   "
			"		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON  "
			"		LineItemT_1.ID = PaymentsT.ID  "
			"		WHERE (((LineItemT_1.Deleted)=0))  "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"		HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"		) AS _PartiallyAppliedPaysQ  "
			"	ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (LineItemT AS LineItemT_1 LEFT JOIN LocationsT ON  "
			"	LineItemT_1.LocationID = LocationsT.ID) ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON  "
			"	PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"   LEFT JOIN LocationsT PayLocsT ON LineItemT.LocationID = PayLocsT.ID "
			"	WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1))  "
			"	) AS PaymentsByDateFullQ  "
			"	 "
			"	UNION  "
			" "
			"	SELECT * FROM  "
			"		(SELECT [_PartiallyAppliedPaysQ].ID,   "
			"		LineItemT.PatientID,   "
			"		[_PartiallyAppliedPaysQ].Total AS Amount,   "
			"		PaymentsT.ProviderID AS ProvID,   "
			"		'Part' AS RandomText,   "
			"		LineItemT.PatientID AS PatID,   "
			"		PatientsT.UserDefinedID,  "
			"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"		LineItemT.InputDate AS IDate,   "
			"		LineItemT.Date AS TDate,   "
			"		PaymentsT.PayMethod,   "
			"		LineItemT.Description,   "
			"		0 AS ApplyID,   "
			"		LineItemT.ID AS LineID,  "
			"		LineItemT.LocationID AS LocID,  "
			"		LocationsT.Name AS Location,  "
			"		POS.Name AS POSName, "
			"		POS.ID AS POSCode, "
			"       LineItemT.LocationID AS PaymentLocationID, "
			"	    LocationsT.Name as PaymentLocation " 
			"		FROM (( "
			"			(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			"			Sum(AppliesT.Amount) AS ApplyAmt,   "
			"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			"			LineItemT_1.PatientID   "
			"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"			ON LineItemT_1.ID = PaymentsT.ID  "
			"			WHERE (((LineItemT_1.Deleted)=0))  "
			"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"			) AS _PartiallyAppliedPaysQ  "
			"		INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID =  "
			"		PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"		LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"		WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1))  "
			"		) AS PaymentsByDatePartQ  "
			"	) AS PaymentsByDateSubQ  "
			"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
			"LEFT JOIN LocationsT ON PaymentsByDateSubQ.LocID = LocationsT.ID "
			" "
			"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName, PaymentsByDateSubQ.IDate,  "
			"PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID,  "
			"PaymentsByDateSubQ.LocID, LocationsT.Name, PaymentsByDateSubQ.POSName, PaymentsByDateSubQ.POsCode, PaymentsByDateSubQ.PaymentLocationID, PaymentsByDateSubQ.PaymentLocation");
			break;

		case 489:	//Payment Tips by Provider
			/*	Version History
				DRT 3/10/2004 - PLID 11258 - Created.
				DRT 3/23/2004 - PLID 11540 - Changed tips to use providers instead of users
				TES 3/20/2015 - PLID 65077 - Updated the PayMethod formula to include "Gift Certificate"
				TES 3/26/2015 - PLID 65178 - Added tips from refunds
				// (r.gonet 2015-05-05 14:38) - PLID 66303 - Include Gift Certificate Refunds
			*/
			return _T("SELECT PaymentTipsT.ID, PaymentID, PaymentTipsT.Amount AS TipAmount, LocationsT.Name AS LocName, "
			"CASE WHEN PaymentTipsT.PayMethod IN (1,7) THEN 'Cash' WHEN PaymentTipsT.PayMethod IN (2,8) THEN 'Check' WHEN PaymentTipsT.PayMethod IN (3,9) THEN 'Charge' WHEN PaymentTipsT.PayMethod IN (4,10) THEN 'Gift Certificate' END AS PayMethod, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PersonT.ID AS PatID, PatientsT.UserDefinedID, "
			"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, PersonProv.ID AS ProvID,  "
			"LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, LineItemT.Description AS PayDescription, LocationsT.ID AS LocID, PaymentTipsT.PayMethod AS PayMethodID "
			"FROM PaymentTipsT  "
			"INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonProv ON PaymentTipsT.ProvID = PersonProv.ID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type IN (1,3) ");
			break;

		case 545:
			//Gift Certificates Redeemed
			/*	Version History
				DRT 8/25/2004 - PLID 13974 - Created.
				DRT 8/11/2005 - PLID 17226 - Reworked.  It was only showing the location & provider information from the GC
					itself, not from the location or provider of the charge it was applied to.  I also added the gift certificate
					id field.
				(a.walling 2007-03-22 15:48) - PLID 25113 - Turned the query into GCPaysQ for external and extended filters
				TES 5/14/2008 - PLID 30037 - Added the TypeName field.
				TES 5/22/2008 - PLID 28375 - Added PurchaseDate, and a filter on it.
				DRT 5/23/2008 - PLID 30159 - Removed the "SELECT * FROM (" and ") GCPaysQ" portions, which are not valid setup 
					(25113 really should have been returned long ago).  It was causing an assertion stating that it wasn't valid.
                
                v.arth 05/13/09 - PLID 34225 - Modified the query show the provider location and provider name from which the
                                               gift card was purchased.
				// (j.dinatale 2011-09-30 16:05) - PLID 45773 - need to remove original or voiding line items for items impacted by financial corrections
				TES 3/20/2015 - PLID 65073 - Added TipAmount
			*/
			return _T("SELECT FinSubQ.ID, LineItemT.Date AS PayDate, LineItemT.InputDate AS PayInputDate,  "
			"LineItemT.InputName, FinSubQ.Amount, GiftTipsQ.TotalGiftTips AS TipAmount, LineItemT.PatientID AS PatID, PatientsT.UserDefinedID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"FinSubQ.ProvID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"ReferralSourceT.PersonID AS ReferralID, ReferralSourceT.Name AS ReferralName,   "
			"PersonRefPat.ID AS RefPatID, PersonRefPat.Last + ', ' + PersonRefPat.First + ' ' + PersonRefPat.Middle AS RefPatName,   "
			"PersonRefPhys.ID AS RefPhysID, PersonRefPhys.Last + ', ' + PersonRefPhys.First + ' ' + PersonRefPhys.Middle AS RefPhysName,  "
			"FinSubQ.LocID AS LocID, LocationsT.Name AS LocName, LineItemT.Description, GiftCertificatesT.GiftID, "
			"GiftCertificatesT.DefaultTypeID AS ServiceID, GiftCertificatesT.ExpDate, ServiceT.Name AS TypeName, "
			"GiftCertificatesT.PurchaseDate AS DatePurchased, ChargeGiftCertificateQuery.LocationID AS ChargeLocationID, "
            "ChargeGiftCertificateQuery.ProviderID AS ChargeProviderID, ChargeGiftCertificateQuery.ProviderLocation AS ChargeProviderLocation, "
            "ChargeGiftCertificateQuery.ProviderFirstName AS ChargeProviderFirstName, ChargeGiftCertificateQuery.ProviderMiddleName AS ChargeProviderMiddleName, "
            "ChargeGiftCertificateQuery.ProviderLastName AS ChargeProviderLastname "
			"FROM "
			"	(SELECT ID, SUM(Amount) AS Amount, ProvID, LocID "
			"	FROM "
			"		(SELECT * FROM "
			"			(SELECT LineItemT.ID, "
			"			CASE WHEN [_PartiallyAppliedPaysQ].[ID] Is Null  "
			"			     THEN CASE WHEN [LineItemT_1].[ID] Is Null  "
			"			     THEN [LineItemT].[Amount] ELSE [AppliesT].[Amount] End ELSE [AppliesT].[Amount] End AS Amount, "
			"			CASE WHEN [DoctorsProviders] Is Null "
			"			     THEN [ProviderID]  ELSE [DoctorsProviders] End AS ProvID,   "
			"			CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID "
			"			FROM LineItemT "
			"			LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"			LEFT JOIN  "
			"				(SELECT LinePaysT.ID, LinePaysT.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,  "
			"				Min([LinePaysT].[Amount])-Sum([AppliesT].[Amount]) AS Total, LinePaysT.PatientID "
			"				FROM LineItemT AS LinePaysT  "
			"				LEFT JOIN PaymentsT ON LinePaysT.ID = PaymentsT.ID "
			"				LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"				LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"				WHERE LinePaysT.Deleted = 0 AND PaymentsT.PayMethod = 4 "
			"				GROUP BY LinePaysT.ID, LinePaysT.Amount, LinePaysT.PatientID   "
			"				HAVING (MIN([LinePaysT].[Amount]) - Sum([AppliesT].[Amount])) <> 0 "
			"				) AS _PartiallyAppliedPaysQ   "
			"			ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID  "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID "
			"			LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID "
			"			WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.PayMethod = 4 "
			"			) SubFullQ "
			" "
			"		UNION ALL "
			"		SELECT * FROM "
			" "
			"			(SELECT [_PartiallyAppliedPaysQ].ID, [_PartiallyAppliedPaysQ].Total AS Amount,   "
			"			PaymentsT.ProviderID AS ProvID, LineItemT.LocationID AS LocID "
			"			FROM  "
			"				(SELECT LinePaysT.ID, LinePaysT.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,  "
			"				Min([LinePaysT].[Amount]) - Sum([AppliesT].[Amount]) AS Total, LinePaysT.PatientID "
			"				FROM LineItemT AS LinePaysT  "
			"				LEFT JOIN PaymentsT ON LinePaysT.ID = PaymentsT.ID "
			"				LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"				LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"				WHERE LinePaysT.Deleted = 0 AND PaymentsT.PayMethod = 4 "
			"				GROUP BY LinePaysT.ID, LinePaysT.Amount, LinePaysT.PatientID   "
			"				HAVING (MIN([LinePaysT].[Amount]) - Sum([AppliesT].[Amount])) <> 0 "
			"				) AS _PartiallyAppliedPaysQ   "
			"			INNER JOIN LineItemT ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID "
			"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"			WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.PayMethod = 4 "
			"			) SubPartQ "
			"		) SubQ "
			"	GROUP BY ID, ProvID, LocID "
			"	) FinSubQ "
			"LEFT JOIN LineItemT ON FinSubQ.ID = LineItemT.ID "
			"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"LEFT JOIN(SELECT PaymentID, Sum(Amount) AS TotalGiftTips FROM PaymentTipsT WHERE PayMethod = 4 GROUP BY PaymentID) "
			"  AS GiftTipsQ ON LineItemT.ID = GiftTipsQ.PaymentID "
			"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonProv ON FinSubQ.ProvID = PersonProv.ID "
			"LEFT JOIN LocationsT ON FinSubQ.LocID = LocationsT.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
			"LEFT JOIN PersonT PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID  "
			"LEFT JOIN PersonT PersonRefPhys ON PatientsT.DefaultReferringPhyID = PersonRefPhys.ID  "
			"LEFT JOIN GiftCertificatesT ON LineItemT.GiftID = GiftCertificatesT.ID "
            "LEFT JOIN ServiceT ON GiftCertificatesT.DefaultTypeID = ServiceT.ID "
            "LEFT JOIN (SELECT GiftCertificatesT.ID AS GiftCertificateID, "
                              "LocationsT.ID AS LocationID, "
                              "PersonT.ID AS ProviderID, "
                              "LocationsT.Name AS ProviderLocation, "
                              "PersonT.First AS ProviderFirstName, "
                              "PersonT.Middle AS ProviderMiddleName, "
                              "PersonT.Last AS ProviderLastName "
                       "FROM GiftCertificatesT "
                           "LEFT JOIN LineItemT "
                           "    ON LineItemT.GiftID = GiftCertificatesT.ID "
						   "LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
						   "LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
                           "INNER JOIN LocationsT "
                           "    ON LocationsT.ID = LineItemT.LocationID "
                           "INNER JOIN ChargesT "
                           "    ON ChargesT.ID = LineItemT.ID "
                           "INNER JOIN GCTypesT "
                           "    ON GCTypesT.ServiceID = ChargesT.ServiceID "
                           "        AND GCTypesT.IsReCharge = 0 "
                           "LEFT JOIN PersonT "
                           "    ON PersonT.ID = ChargesT.DoctorsProviders "
						   "WHERE OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
                "AS ChargeGiftCertificateQuery "
                     "ON LineItemT.GiftID = ChargeGiftCertificateQuery.GiftCertificateID "
				"WHERE OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL ");
			break;

		case 565:
			//Adjustments By Referring Physician
		/*	Version History
			JMM PLID 16961 4/12/06
			// (j.gruber 2007-02-20 09:46) - PLID 24737 - fixed verification problem on input reports - no change to query
			// (j.gruber 2007-05-29 16:36) - PLID 24837 - added reason code and group code
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.jones 2010-09-27 09:05) - PLID 40650 - fixed group & reason codes to have descriptions in the query
			(r.goldschmidt 2014-01-28 17:25) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByRefPhysSubQ.ID,  "
		"Sum(PaymentsByRefPhysSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByRefPhysSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByRefPhysSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByRefPhysSubQ.PatID AS PatID, "
		"PaymentsByRefPhysSubQ.UserDefinedID,  "
		"PaymentsByRefPhysSubQ.FullName,  "
		"PaymentsByRefPhysSubQ.IDate AS IDate,  "
		"PaymentsByRefPhysSubQ.TDate AS TDate,  "
		"PaymentsByRefPhysSubQ.PayMethod AS PayMethod,  "
		"PaymentsByRefPhysSubQ.Description,  "
		"Min(PaymentsByRefPhysSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByRefPhysSubQ.LineID) AS FirstOfLineID,  "
		"PaymentsByRefPhysSubQ.CheckNo,  "
		"PaymentsByRefPhysSubQ.CCType,  "
		"PaymentsByRefPhysSubQ.RefPhysID AS RefPhysID,  "
		"PaymentsByRefPhysSubQ.RefPhysName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByRefPhysSubQ.LocID AS LocID, "
		"PaymentsByRefPhysSubQ.Location, "
		"PaymentsByRefPhysSubQ.ReasonCode, "
		"PaymentsByRefPhysSubQ.GroupCode, "
		"PaymentsByRefPhysSubQ.ReasonCodeDesc, "
		"PaymentsByRefPhysSubQ.GroupCodeDesc "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM ((((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID) "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID) "
		"LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
		"AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
		"FROM ((((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=2)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysPartQ "
		") AS PaymentsByRefPhysSubQ LEFT JOIN ProvidersT ON PaymentsByRefPhysSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsByRefPhysSubQ.ID, PaymentsByRefPhysSubQ.ProvID, PaymentsByRefPhysSubQ.PatID, PaymentsByRefPhysSubQ.UserDefinedID, "
		"PaymentsByRefPhysSubQ.FullName, PaymentsByRefPhysSubQ.IDate, PaymentsByRefPhysSubQ.TDate, PaymentsByRefPhysSubQ.PayMethod, PaymentsByRefPhysSubQ.Description, "
		"PaymentsByRefPhysSubQ.CheckNo, PaymentsByRefPhysSubQ.CCType, PaymentsByRefPhysSubQ.RefPhysID, PaymentsByRefPhysSubQ.RefPhysName, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByRefPhysSubQ.LocID, PaymentsByRefPhysSubQ.Location, "
		"PaymentsByRefPhysSubQ.ReasonCode, PaymentsByRefPhysSubQ.GroupCode, PaymentsByRefPhysSubQ.ReasonCodeDesc, PaymentsByRefPhysSubQ.GroupCodeDesc "
		"");
		break;


		case 566:
			//Refunds By Referring Physician
		/*	Version History
			JMM PLID 16961 4/12/06
			(e.lally 2007-07-12) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			(r.goldschmidt 2014-01-28 17:26) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
			// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PaymentsByRefPhysSubQ.ID,  "
		"Sum(PaymentsByRefPhysSubQ.Amount) AS SumOfAmount,  "
		"PaymentsByRefPhysSubQ.ProvID AS ProvID,  "
		"Min(PaymentsByRefPhysSubQ.RandomText) AS FirstOfRandomText,  "
		"PaymentsByRefPhysSubQ.PatID AS PatID, "
		"PaymentsByRefPhysSubQ.UserDefinedID,  "
		"PaymentsByRefPhysSubQ.FullName,  "
		"PaymentsByRefPhysSubQ.IDate AS IDate,  "
		"PaymentsByRefPhysSubQ.TDate AS TDate,  "
		"PaymentsByRefPhysSubQ.PayMethod AS PayMethod,  "
		"PaymentsByRefPhysSubQ.Description,  "
		"Min(PaymentsByRefPhysSubQ.ApplyID) AS FirstOfApplyID,  "
		"Min(PaymentsByRefPhysSubQ.LineID) AS FirstOfLineID,  "
		"PaymentsByRefPhysSubQ.CheckNo,  "
		"PaymentsByRefPhysSubQ.CCType,  "
		"PaymentsByRefPhysSubQ.RefPhysID AS RefPhysID,  "
		"PaymentsByRefPhysSubQ.RefPhysName,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
		"PaymentsByRefPhysSubQ.LocID AS LocID, "
		"PaymentsByRefPhysSubQ.Location "
		"FROM ((SELECT * FROM (SELECT LineItemT.ID,  "
		"Amount = CASE "
		"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null "
		"	THEN CASE "
		"		WHEN [LineItemT_1].[ID] Is Null "
		"		THEN [LineItemT].[Amount] "
		"		ELSE [AppliesT].[Amount] "
		"		End "
		"	ELSE [AppliesT].[Amount] "
		"	End,  "
		"ProvID = CASE "
		"	WHEN [DoctorsProviders] Is Null "
		"	THEN [ProviderID] "
		"	ELSE [DoctorsProviders] "
		"	End,  "
		"'Full' AS RandomText,  "
		"LineItemT.PatientID AS PatID, "
		"PatientsT.UserDefinedID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"AppliesT.ID AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
		"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location "
		"FROM ((((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)=3)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysFullQ "
		"UNION SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,  "
		"[_PartiallyAppliedPaysQ].Total AS Amount,  "
		"PaymentsT.ProviderID AS ProvID,  "
		"'Part' AS RandomText,  "
		"LineItemT.PatientID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"LineItemT.InputDate AS IDate,  "
		"LineItemT.Date AS TDate,  "
		"PaymentsT.PayMethod,  "
		"LineItemT.Description,  "
		"0 AS ApplyID,  "
		"LineItemT.ID AS LineID,  "
		"PaymentPlansT.CheckNo,  "
		"CreditCardNamesT.CardName AS CCType,  "
		"ReferringPhyST.PersonID AS RefPhysID,  "
		"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS RefPhysName, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location "
		"FROM ((((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
		"Sum(AppliesT.Amount) AS ApplyAmt,  "
		"/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
		"LineItemT_1.PatientID  "
		"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
		"WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
		"HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0)) "
		") AS _PartiallyAppliedPaysQ INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN ReferringPhyST ON PatientsT.DefaultReferringPhyID = ReferringPhyST.PersonID) LEFT JOIN PersonT PersonT_1 ON ReferringPhyST.PersonID = PersonT_1.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=3)) AND (PaymentsT.PayMethod NOT IN (4,10)) "
		") AS _PaymentsByRefPhysPartQ "
		") AS PaymentsByRefPhysSubQ LEFT JOIN ProvidersT ON PaymentsByRefPhysSubQ.ProvID = ProvidersT.PersonID) LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"GROUP BY PaymentsByRefPhysSubQ.ID, PaymentsByRefPhysSubQ.ProvID, PaymentsByRefPhysSubQ.PatID, PaymentsByRefPhysSubQ.UserDefinedID, PaymentsByRefPhysSubQ.FullName, PaymentsByRefPhysSubQ.IDate, PaymentsByRefPhysSubQ.TDate, PaymentsByRefPhysSubQ.PayMethod, PaymentsByRefPhysSubQ.Description, PaymentsByRefPhysSubQ.CheckNo, PaymentsByRefPhysSubQ.CCType, PaymentsByRefPhysSubQ.RefPhysID, PaymentsByRefPhysSubQ.RefPhysName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByRefPhysSubQ.LocID, PaymentsByRefPhysSubQ.Location "
		"");
		break;
		

		case 607:
			// Prepayments Applied to Quotes
			/*	Version History
				(d.moore 2007-08-13) - PLID 25115 - Report Created.
				(d.moore 2007-09-07) - PLID 25115 - Provider ID changed from PatientsT.MainPhysician to LocationsT.DefaultProviderID
				// (j.jones 2009-02-16 10:51) - PLID 33048 - fixed bug where deleted bills made for non-deleted quotes still showed up
				// (j.gruber 2009-03-25 13:48) - PLID 33691 - updated discount structure
				// (s.tullis 2015-02-18 10:14) - PLID 64862 - fixed bug where quotes billed multiple times were causing duplicates in the report
				// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
				// (r.goldschmidt 2016-02-08 13:00) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
			*/
			return _T(
			"SELECT "
				"QuoteT.ID AS QuoteID, "
				"QuoteTotalQ.QuoteTotal, "
				"dbo.AsDateNoTime(QuoteT.Date) AS QuoteDate, "
				"dbo.AsDateNoTime(QuoteT.InputDate) AS QuoteInputeDate, "
				"QuoteT.Description AS QuoteDescription, "
				"QuoteT.Location AS QuoteLocationID, "
				"LocationsT.Name AS QuoteLocationName, "
				"(CASE WHEN PackagesT.Type Is Null THEN 'Quotes' "
					"WHEN PackagesT.Type = 1 THEN 'Repeatable Packages' "
					"WHEN PackagesT.Type = 2 THEN 'Multi-Use Packages' END) AS PackageType, "
				"ConvBillT.ID AS ConvBillID, "
				"CASE WHEN ConvBillT.ID Is Not Null THEN 'Billed' ELSE 'Quote' END AS QuoteOrBill, "
				"PaymentsT.ID AS PrePayID, "
				"PaymentLineItemT.Amount AS PrePayAmount, "
				"dbo.AsDateNoTime(PaymentLineItemT.Date) AS TDate, "
				"PaymentLineItemT.LocationID AS LocID, "
				"CASE WHEN (PaymentsT.PayMethod = 1) THEN 'Cash' "
					"WHEN (PaymentsT.PayMethod = 2) THEN 'Check' "
					"WHEN (PaymentsT.PayMethod = 3) THEN 'Charge' "
					"WHEN (PaymentsT.PayMethod = 4) THEN 'Gift Cert' "
					"ELSE '' END AS PrePayMethod, "
				"PatientPersonT.ID AS PatID, "
				"PatientsT.UserDefinedID AS UserDefinedPatID, "
				"PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatName, "
				"ProviderPersonT.ID AS ProvID, "
				"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProvName, "
				"PatCoordPersonT.ID AS PatCoordID, "
				"PatCoordPersonT.Last + ', ' + PatCoordPersonT.First + ' ' + PatCoordPersonT.Middle AS PatCoordName "
			"FROM BillsT QuoteT "
				"INNER JOIN "
					"(SELECT "
						"BillID, "
						"Sum(ChargeRespT.Amount) AS QuoteTotal "
					"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
					"WHERE LineItemT.Deleted = 0 "
						"AND BillID IN "
							"(SELECT DISTINCT ID FROM BillsT WHERE EntryType = 2) "
					"GROUP BY BillID) AS QuoteTotalQ "
					"ON QuoteT.ID = QuoteTotalQ.BillID "
				"INNER JOIN PaymentsT ON QuoteT.ID = PaymentsT.QuoteID "
				"INNER JOIN LineItemT PaymentLineItemT ON PaymentsT.ID = PaymentLineItemT.ID "
				"LEFT JOIN (SELECT Min(BilledQuotesT.QuoteID) as QuoteID, Min(billsT.ID) as ID  "
				"	FROM BillsT "
				"	INNER JOIN BilledQuotesT ON BillsT.ID = BilledQuotesT.BillID "
				"	WHERE BillsT.Deleted = 0 "
				"   Group By BilledQuotesT.QuoteID ) AS ConvBillT ON QuoteT.ID = ConvBillT.QuoteID "
				"INNER JOIN PatientsT ON QuoteT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT PatientPersonT ON PatientsT.PersonID = PatientPersonT.ID "
				"LEFT JOIN PackagesT ON QuoteT.ID = PackagesT.QuoteID "
				"LEFT JOIN LocationsT ON QuoteT.Location = LocationsT.ID "
				"LEFT JOIN UsersT ON QuoteT.PatCoord = UsersT.PersonID "
				"LEFT JOIN PersonT PatCoordPersonT ON UsersT.PersonId = PatCoordPersonT.ID "
				"LEFT JOIN PersonT ProviderPersonT ON PaymentsT.ProviderID = ProviderPersonT.ID "
			"WHERE QuoteT.EntryType = 2 "
				"AND (QuoteT.Deleted = 0 OR ConvBillT.ID Is Not Null) "
				"AND (PaymentLineItemT.Deleted = 0)");
			break;

		case 669:		//Credit Card Processing Reconciliation
			/*	Version History
				(d.thompson 2009-07-07) - PLID 34798 - Created.  The query is copied from Payments by Credit Card, then
					I modified it to include additional CC processing data from the QBMS data tables.  I also redid the
					layout of the query -- it was unreadably bad, so I fixed some table naming confusion and the general
					structure of the SQL.
					// (j.gruber 2010-11-03 13:52) - PLID 40300 - added refunds
					// (d.thompson 2010-11-08) - PLID 41379 - Added the account that each payment was processed into
					// (d.thompson 2010-11-08) - PLID 41379 - Also added an extended filter for account
					// (d.thompson 2010-11-17) - PLID 41514 - Added Chase account information in addition to QBMS
					// (d.thompson 2011-01-07) - PLID 41514 - Fixed some issues properly pulling chase data, and removed TransType.  That
					//	field differs depending on your processor, and it's easier to just use the LineItem value to tell what's-what.
					// (r.goldschmidt 2014-02-03 10:41) - PLID 60510 - Reports of Payments by Credit Card can fail to display location name for payment.
			*/
			return _T("SELECT PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,  "
				"PaymentsByDateSubQ.ProvID AS ProvID, Min(PaymentsByDateSubQ.RandomText) AS FirstOfRandomText,  "
				"PaymentsByDateSubQ.PatID AS PatID, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.FullName,  "
				"PaymentsByDateSubQ.IDate AS IDate, PaymentsByDateSubQ.TDate AS TDate,  "
				"PaymentsByDateSubQ.PayMethod AS PayMethod, PaymentsByDateSubQ.Description,   "
				"Min(PaymentsByDateSubQ.ApplyID) AS FirstOfApplyID, Min(PaymentsByDateSubQ.LineID) AS FirstOfLineID,   "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, PaymentsByDateSubQ.LocID AS LocID,  "
				"PaymentsByDateSubQ.Location, PaymentsByDateSubQ.CreditCard AS CreditCard,  "
				"CASE WHEN Len(PaymentsByDateSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentsByDateSubQ.CCNumber, 4) END  as CCNumber, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.TotalAmount ELSE Chase_CreditTransactionsT.TotalAmount END AS TransactionAmount, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.IsApproved ELSE Chase_CreditTransactionsT.IsApproved END AS IsApproved, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.LocalApprovalDateTime ELSE Chase_CreditTransactionsT.LocalApprovalDateTime END AS LocalApprovalDateTime, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.AccountID ELSE Chase_CreditTransactionsT.AccountID END AS AccountID, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_SetupData.Description ELSE Chase_SetupDataT.Description END AS AccountDescription "
				"FROM  "
				"	(SELECT * FROM  "
				"		(SELECT LineItemT.ID, LineItemT.PatientID, "
				"		CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN  "
				"			CASE WHEN LineChargesT.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End  "
				"		ELSE AppliesT.Amount End AS Amount, "
				"		CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End AS ProvID, "
				"		'Full' AS RandomText, LineItemT.PatientID AS PatID, PatientsT.UserDefinedID,  "
				"		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
				"		LineItemT.InputDate AS IDate, LineItemT.Date AS TDate, PaymentsT.PayMethod,   "
				"		LineItemT.Description, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID,  "
				"		CASE WHEN LineChargesT.LocationID Is Null THEN LineItemT.LocationID ELSE LineChargesT.LocationID END AS LocID,  "
				"		CASE WHEN LineChargesT.LocationID Is Null THEN LocationsT.Name ELSE ChargeLocationT.Name END AS Location, "
				"		CreditCardNamesT.CardName AS CreditCard, PaymentPlansT.CCNumber "
				"		FROM LineItemT  "
				"		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"		LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"		INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID  "
				"		INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"		LEFT JOIN  "
				"			(SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt,   "
				"			Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total, LinePaymentsT.PatientID   "
				"			FROM LineItemT AS LinePaymentsT  "
				"			LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID  "
				"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"			WHERE LinePaymentsT.Deleted=0  "
				"			GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID  "
				"			HAVING LinePaymentsT.ID IS NOT NULL AND MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) <> 0 "
				"			) AS _PartiallyAppliedPaysQ  "
				"		ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID "
				"		LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"		LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID "
				"		LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID "
				"		LEFT JOIN LocationsT AS ChargeLocationT ON LineChargesT.LocationID = ChargeLocationT.ID "
				" "
				"		WHERE PaymentsT.ID Is Not Null AND LineItemT.Deleted=0 AND ((LineItemT.Type=1 AND PaymentsT.PayMethod = 3) OR (LineItemT.Type=3 AND PaymentsT.PayMethod = 9))"
				"		) AS PaymentsByDateFullQ  "
				" "
				"	UNION  "
				"	SELECT * FROM  "
				"		(SELECT [_PartiallyAppliedPaysQ].ID, LineItemT.PatientID, [_PartiallyAppliedPaysQ].Total AS Amount,  "
				"		PaymentsT.ProviderID AS ProvID, 'Part' AS RandomText, LineItemT.PatientID AS PatID,   "
				"		PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
				"		LineItemT.InputDate AS IDate, LineItemT.Date AS TDate, PaymentsT.PayMethod,   "
				"		LineItemT.Description, 0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID,  "
				"		LocationsT.Name AS Location, CreditCardNamesT.CardName AS CreditCard, PaymentPlansT.CCNumber  "
				"		FROM  "
				"			(SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,   "
				"			Sum(AppliesT.Amount) AS ApplyAmt, Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
				"			LinePaymentsT.PatientID   "
				"			FROM LineItemT AS LinePaymentsT "
				"			LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID "
				"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
				"			WHERE LinePaymentsT.Deleted=0 "
				"			GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID  "
				"			HAVING LinePaymentsT.ID IS NOT NULL AND MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) <> 0 "
				"			) AS _PartiallyAppliedPaysQ "
				"		INNER JOIN LineItemT ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID "
				"		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
				"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"		LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID  "
				"		INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID "
				"		WHERE LineItemT.Deleted=0 AND ((LineItemT.Type=1 AND PaymentsT.PayMethod = 3) OR (LineItemT.Type=3 AND PaymentsT.PayMethod = 9)) "
				"		) AS PaymentsByDatePartQ "
				"	) AS PaymentsByDateSubQ  "
				"LEFT JOIN ProvidersT ON PaymentsByDateSubQ.ProvID = ProvidersT.PersonID "
				"LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID  "
				"LEFT JOIN QBMS_CreditTransactionsT ON PaymentsByDateSubQ.ID = QBMS_CreditTransactionsT.ID "
				"LEFT JOIN QBMS_SetupData ON QBMS_CreditTransactionsT.AccountID = QBMS_SetupData.ID "
				"LEFT JOIN Chase_CreditTransactionsT ON PaymentsByDateSubQ.ID = Chase_CreditTransactionsT.ID "
				"LEFT JOIN Chase_SetupDataT ON Chase_CreditTransactionsT.AccountID = Chase_SetupDataT.ID "
				"WHERE (QBMS_CreditTransactionsT.ID IS NOT NULL OR Chase_CreditTransactionsT.ID IS NOT NULL) "
				"GROUP BY PaymentsByDateSubQ.ID, PaymentsByDateSubQ.PatientID, PaymentsByDateSubQ.ProvID, PaymentsByDateSubQ.PatID, PaymentsByDateSubQ.FullName,  "
				"PaymentsByDateSubQ.IDate, PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.PayMethod, PaymentsByDateSubQ.Description,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.LocID,  "
				"PaymentsByDateSubQ.Location, PaymentsByDateSubQ.CreditCard, PaymentsByDateSubQ.CCNumber, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.TotalAmount ELSE Chase_CreditTransactionsT.TotalAmount END, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.IsApproved ELSE Chase_CreditTransactionsT.IsApproved END, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.LocalApprovalDateTime ELSE Chase_CreditTransactionsT.LocalApprovalDateTime END, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_CreditTransactionsT.AccountID ELSE Chase_CreditTransactionsT.AccountID END, "
				"CASE WHEN QBMS_CreditTransactionsT.AccountID IS NOT NULL THEN QBMS_SetupData.Description ELSE Chase_SetupDataT.Description END ");
			break;

			case 722: // Get Payments/Adjustments by Service Code/Inventory Item and group by Insurance Company
					/*

					(r.wilson 2011-12-28) - PLID 42688 - Created.
					// (j.gruber 2014-03-28 12:44) - PLID 61462 - updated for ICD-10

					*/
				return _T(					
		"SELECT   "
			"PaymentsByInsCoByCPTCodeFullQ.InsID AS InsID,      "
			"PaymentsByInsCoByCPTCodeFullQ.InsName,      "
			"PaymentsByInsCoByCPTCodeFullQ.ID,        "
			"PaymentsByInsCoByCPTCodeFullQ.PatID AS PatID,        "
			"PaymentsByInsCoByCPTCodeFullQ.UserDefinedID,       "
			"PaymentsByInsCoByCPTCodeFullQ.ApplyAmount,        "
			"PaymentsByInsCoByCPTCodeFullQ.ProvID AS ProvID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName,              "
			"PaymentsByInsCoByCPTCodeFullQ.FullName,        "
			"PaymentsByInsCoByCPTCodeFullQ.IDate AS IDate,        "
			"PaymentsByInsCoByCPTCodeFullQ.TDate AS TDate,        "
			"PaymentsByInsCoByCPTCodeFullQ.PayMethod,        "
			"PaymentsByInsCoByCPTCodeFullQ.Description,        "
			"PaymentsByInsCoByCPTCodeFullQ.CodeName AS ItemCodeName,       "
			"PaymentsByInsCoByCPTCodeFullQ.PayAmt,        "
			"PaymentsByInsCoByCPTCodeFullQ.ChargeAmount,        "
			"PaymentsByInsCoByCPTCodeFullQ.ServiceDate AS ServiceDate,        "
			"PaymentsByInsCoByCPTCodeFullQ.ItemDesc,             "
			"PaymentsByInsCoByCPTCodeFullQ.FirstOfID AS ServiceID,       "
			"PaymentsByInsCoByCPTCodeFullQ.LocID AS LocID,       "
			"PaymentsByInsCoByCPTCodeFullQ.Location,      "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9Code1, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9Code2, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9Code3, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9Code4, \r\n "

			"PaymentsByInsCoByCPTCodeFullQ.ICD10Code1, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10Code2, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10Code3, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10Code4, \r\n "

			"PaymentsByInsCoByCPTCodeFullQ.ICD9CodeDesc1, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9CodeDesc2, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9CodeDesc3, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD9CodeDesc4, \r\n "

			"PaymentsByInsCoByCPTCodeFullQ.ICD10CodeDesc1, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10CodeDesc2, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10CodeDesc3, \r\n "
			"PaymentsByInsCoByCPTCodeFullQ.ICD10CodeDesc4,  \r\n "

			"PaymentsByInsCoByCPTCodeFullQ.WhichCodes,  \r\n"
			"PaymentsByInsCoByCPTCodeFullQ.BillDate AS BillDate,     "
			"PaymentsByInsCoByCPTCodeFullQ.PaymentType AS PaymentTypeID,     "
			"CASE        "
			"	WHEN PaymentsByInsCoByCPTCodeFullQ.PaymentType = 1 THEN 'Payments'       "
			"	WHEN PaymentsByInsCoByCPTCodeFullQ.PaymentType = 2 THEN 'Adjustments'       "
			"	ElSE ''       "
			"END AS PaymentType,    "
			"PaymentsByInsCoByCPTCodeFullQ.Code    "
			"FROM ((((SELECT LineItemT.ID,        "
			"LineItemT.PatientID,        "
			"ApplyAmount = CASE       "
			"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null       "
			"	THEN CASE       "
			"		WHEN [LineItemT_1].[ID] Is Null       "
			"		THEN [LineItemT].[Amount]       "
			"		ELSE [AppliesT].[Amount]       "
			"		End       "
			"	ELSE [AppliesT].[Amount]       "
			"	End,        "
			"ProvID = CASE WHEN [DoctorsProviders] Is Null       "
			"	THEN PaymentsT.ProviderID       "
			"	ELSE [DoctorsProviders]       "
			"	End, "
			"LineItemT.PatientID AS PatID,        "
			"PatientsT.UserDefinedID,       "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,        "
			"LineItemT.Type As PaymentType,      "
			"LineItemT.InputDate AS IDate,        "
			"LineItemT.Date AS TDate,        "
			"PaymentsT.PayMethod,        "
			"LineItemT.Description,        "
			"ProductsAndServiceCodeQ.Name AS CodeName,    "
			"LineItemT.Amount AS PayAmt,        "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,        "
			"LineItemT_1.Date AS ServiceDate,        "
			"LineItemT_1.Description AS ItemDesc,        "
			"InsuranceCoT.PersonID AS InsID,      "
			
			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "

			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "

			"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
			"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
			"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
			"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "

			"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
			"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
			"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
			"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "

			"ChargeWhichCodesFlatV.WhichCodes,  \r\n"
			"BillsT.Date AS BillDate,     "
			"ProductsAndServiceCodeQ.Code,    "
			"CASE        "
			"	WHEN InsuranceCoT.PersonID IS NULL THEN 'Patient Responsibility'       "
			"	ELSE InsuranceCoT.Name       "
			"END AS  InsName,      "
			"ChargesT.ItemSubCode,        "
			"AppliesT.ID AS ApplyID,        "
			"LineItemT.ID AS LineID,        "
			"Min(ProductsAndServiceCodeQ.ServiceID) AS FirstOfID,    "
			"ChargesT.Category,           "
			"LocationAppliedT.ID AS LocID,  "
			"LocationAppliedT.Name AS Location  "
			"FROM       "
			"((((((((    "
			"(LineItemT LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN   "
			"	(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,        "
			"	Sum(AppliesT.Amount) AS ApplyAmt,        "
			"	/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,        "
			"	LineItemT_1.PatientID        "
			"	FROM LineItemT AS LineItemT_1       "
			"	LEFT JOIN AppliesT ON LineItemT_1.ID = AppliesT.SourceID       "
			"	LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"	LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID  	 "
			"	LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID       "
			"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID       "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID      "
			"	WHERE LineItemT_1.Deleted=0 AND ProductT.ID IS NOT NULL       "
			"	GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID      "
			"	HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))       "
			"	)AS _PartiallyAppliedPaysQ ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID)   "
			"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)     "
			"LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) "
			"LEFT JOIN LocationsT LocationAppliedT ON LineItemT_1.LocationID = LocationAppliedT.ID  "
			"LEFT JOIN LocationsT ON  LineItemT_1.LocationID = LocationsT.ID  "
			"LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID)    "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)    "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number)    "
			"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyId = InsuredPartyT.PersonID)    "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID)    "
			"LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.VoidingLineItemID   "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID   "
			"LEFT JOIN LineItemCorrectionsT VoidedChargesLineItemOriginalT ON ChargesT.ID = VoidedChargesLineItemOriginalT.OriginalLineItemID    "
			"LEFT JOIN LineItemCorrectionsT VoidedChargesLineItemT ON ChargesT.ID = VoidedChargesLineItemT.VoidingLineItemID   "
			"	   "
			"LEFT JOIN(    "
			"SELECT ServiceT.ID AS ServiceID,NULL AS Code,ServiceT.Name FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID     "
			"UNION     "
			"SELECT ServiceT.ID AS ServiceID, Code,ServiceT.Name from ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID    "
			") AS ProductsAndServiceCodeQ ON ProductsAndServiceCodeQ.ServiceID = ChargesT.ServiceID    "
			"	   "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID      "
			"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

			"LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID \r\n "

			"LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID   "
			"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)  AND (LineItemCorrectionsT.VoidingLineItemID IS NULL) AND (BillCorrectionsT.OriginalBillId IS NULL) AND (VoidingLineItemsT.VoidingLineItemID IS NULL )  AND (VoidedChargesLineItemT.ID IS NULL) AND (VoidedChargesLineItemOriginalT.OriginalLineItemID IS NULL))       "
			"GROUP BY ICD9T1.CodeNumber, \r\n "
			"ICD9T2.CodeNumber, \r\n "
			"ICD9T3.CodeNumber, \r\n "
			"ICD9T4.CodeNumber, \r\n "

			"ICD10T1.CodeNumber, \r\n "
			"ICD10T2.CodeNumber, \r\n "
			"ICD10T3.CodeNumber, \r\n "
			"ICD10T4.CodeNumber, \r\n "

			"ICD9T1.CodeDesc, \r\n "
			"ICD9T2.CodeDesc, \r\n "
			"ICD9T3.CodeDesc, \r\n "
			"ICD9T4.CodeDesc, \r\n "

			"ICD10T1.CodeDesc, \r\n "
			"ICD10T2.CodeDesc, \r\n "
			"ICD10T3.CodeDesc, \r\n "
			"ICD10T4.CodeDesc,  \r\n "

			"ChargeWhichCodesFlatV.WhichCodes,  \r\n"
			"BillsT.Date,ProductsAndServiceCodeQ.Code,  LocationAppliedT.ID, LocationAppliedT.Name,  "
			"	LineItemT.ID, LineItemT.PatientID,  "
			"	(CASE       "
			"	WHEN [_PartiallyAppliedPaysQ].[ID] Is Null       "
			"	THEN CASE       "
			"		WHEN [LineItemT_1].[ID] Is Null       "
			"		THEN [LineItemT].[Amount]       "
			"		ELSE [AppliesT].[Amount]       "
			"		End       "
			"	ELSE [AppliesT].[Amount]       "
			"	End), "
			"		CASE WHEN [DoctorsProviders] IS NULL       "
			"	THEN PaymentsT.ProviderID       "
			"	ELSE [DoctorsProviders]       "
			"	End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.ItemSubCode, AppliesT.ID, LineItemT.ID,LineItemT.Type, ChargesT.Category, PaymentsT.ID, LineItemT.Type, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, LocationsT.Name, ProductsAndServiceCodeQ.Name       "
			"HAVING (((PaymentsT.ID) Is NOT NULL) AND ((LineItemT.Type)=1 OR (LineItemT.Type)=2))       "
			") AS PaymentsByInsCoByCPTCodeFullQ LEFT JOIN ProvidersT ON PaymentsByInsCoByCPTCodeFullQ.ProvID = ProvidersT.PersonID) "
			" LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN ServiceT ON PaymentsByInsCoByCPTCodeFullQ.FirstOfID = ServiceT.ID)     "
			"	LEFT JOIN (SELECT CategoriesT.ID AS CategoryID,        "
			"				CategoriesT.Name AS Category, '' AS SubCategory,        "
			"				CategoriesT.ID AS ParentID       "
			"				FROM     "
			"				CategoriesT       "
			"				WHERE (((CategoriesT.Parent) = 0))       "
			"				UNION       "
			"				SELECT SubCategoriesT.ID AS CategoryID,        "
			"				CategoriesT.Name AS Category,        "
			"				SubCategoriesT.Name AS SubCategory,        "
			"				SubCategoriesT.Parent AS ParentID       "
			"				FROM CategoriesT RIGHT JOIN       "
			"				CategoriesT AS SubCategoriesT ON        "
			"				CategoriesT.ID = SubCategoriesT.Parent       "
			"				WHERE (((SubCategoriesT.Parent) <> 0))    "
			"				) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID	"		  
			);
				break;
		case 754:	//Payments By Test Code
			/*

			(a.wilson 2014-08-05 12:28) - PLID 63112 - Created.
			// (r.gonet 2015-05-05 14:38) - PLID 66317 - Exclude Gift Certificate Refunds

			*/
			return (R"(
SELECT AdjustmentsByCPTCodeFullQ.LabTestCodeID as LabTestCodeID,
	AdjustmentsByCPTCodeFullQ.LabTestCode,
	AdjustmentsByCPTCodeFullQ.LabTestCodeDescription,
	AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,
	AdjustmentsByCPTCodeFullQ.FullName,
	PersonT.Last + ', ' + PersonT.First AS DocName,
	AdjustmentsByCPTCodeFullQ.TDate AS TDate,
	AdjustmentsByCPTCodeFullQ.PayMethod AS PayMethod,
	AdjustmentsByCPTCodeFullQ.CCType,
	AdjustmentsByCPTCodeFullQ.CheckNo,
	AdjustmentsByCPTCodeFullQ.Description,
	AdjustmentsByCPTCodeFullQ.ApplyAmount,
	CategoriesQ.Category,
	CategoriesQ.SubCategory,
	CategoriesQ.ParentID AS ParentID,
	AdjustmentsByCPTCodeFullQ.PatID AS PatID,
	AdjustmentsByCPTCodeFullQ.UserDefinedID,
	AdjustmentsByCPTCodeFullQ.IDate AS IDate,
	AdjustmentsByCPTCodeFullQ.Type,
	AdjustmentsByCPTCodeFullQ.ItemCode,
	AdjustmentsByCPTCodeFullQ.CPTID AS CPTID,
	AdjustmentsByCPTCodeFullQ.ChargeAmount,
	AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount,
	AdjustmentsByCPTCodeFullQ.ServiceDate,
	AdjustmentsByCPTCodeFullQ.ItemDesc,
	AdjustmentsByCPTCodeFullQ.LocID AS LocID,
	AdjustmentsByCPTCodeFullQ.Location,
	AdjustmentsByCPTCodeFullQ.PaymentDate,
	AdjustmentsByCPTCodeFullQ.BDate AS BDate
FROM (	(	(	(
				SELECT LineItemT.ID,
					BillLabTestCodesT.ID AS LabTestCodeID,
					BillLabTestCodesT.Code AS LabTestCode,
					BillLabTestCodesT.Description AS LabTestCodeDescription,
					ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID IS NULL THEN CASE WHEN LineItemT_1.ID IS NULL THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END,
					CASE WHEN DoctorsProviders IS NULL THEN PaymentsT.ProviderID ELSE DoctorsProviders END AS ProvID,
					PatientsT.PersonID AS PatID,
					PatientsT.UserDefinedID,
					PersonT.FullName,
					LineItemT_1.InputDate AS IDate,
					LineItemT_1.DATE AS TDate,
					LineItemT.DATE AS PaymentDate,
					PaymentsT.PayMethod,
					LineItemT.Description,
					CASE WHEN CPTCodeT.ID IS NULL THEN convert(NVARCHAR, ServiceT.ID) ELSE CPTCodeT.Code END AS ItemCode,
					LineItemT.Amount AS PayAmt,
					dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,
					LineItemT_1.DATE AS ServiceDate,
					LineItemT_1.Description AS ItemDesc,
					CreditCardNamesT.CardName AS CCType,
					PaymentPlansT.CheckNo,
					ChargesT.Category,
					AppliesT.ID AS ApplyID,
					LineItemT.ID AS LineID,
					LineItemT.Type,
					ChargesT.ItemCode AS NoLongerUsedItemCode,
					ServiceT.ID AS CPTID,
					CASE WHEN LineItemT_1.LocationID IS NULL THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,
					CASE WHEN LineItemT_1.LocationID IS NULL THEN LocationsT.NAME ELSE ChargeLoc.NAME END AS Location,
					BillsT.DATE AS BDate
				FROM (	(	(	(	(	(	(	(	(
													LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID
													) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
												) LEFT JOIN (
												SELECT LineItemT_1.ID,
													LineItemT_1.Amount AS PayAmt,
													Sum(AppliesT.Amount) AS ApplyAmt,
													/*First of LineItemT_1*/ MIN([LineItemT_1].[Amount]) - Sum([AppliesT].[Amount]) AS Total,
													LineItemT_1.PatientID
												FROM LineItemT AS LineItemT_1
												LEFT JOIN (
													PaymentsT LEFT JOIN (
														AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID
														) ON PaymentsT.ID = AppliesT.SourceID
													) ON LineItemT_1.ID = PaymentsT.ID
												WHERE (((LineItemT_1.Deleted) = 0))
													AND (PaymentsT.PayMethod NOT IN (4,10))
												GROUP BY LineItemT_1.ID,
													LineItemT_1.Amount,
													LineItemT_1.PatientID
												HAVING (
														((LineItemT_1.ID) IS NOT NULL)
														AND ((MIN([LineItemT_1].[Amount]) - Sum([AppliesT].[Amount])) <> 0)
														)
												) AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID
											) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID
										) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID
									) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID
								) LEFT JOIN (
								ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID
								INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID
								LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID
								LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID
								) ON LineItemT_1.ID = ChargesT.ID
							) LEFT JOIN (
							PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
							) ON LineItemT.PatientID = PatientsT.PersonID
						) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number
					LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number
					)
				LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID
				LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID
				INNER JOIN ChargeLabTestCodesT ON ChargesT.ID = ChargeLabTestCodesT.ChargeID
				INNER JOIN BillLabTestCodesT ON ChargeLabTestCodesT.BillLabTestCodeID = BillLabTestCodesT.ID
				WHERE (
						((PaymentsT.ID) IS NOT NULL)
						AND ((LineItemT.Deleted) = 0)
						AND ((LineItemT_1.ID) IS NOT NULL)
						AND ((AppliesT.PointsToPayments) = 0)
						)
					AND (PaymentsT.PayMethod NOT IN (4,10))
				GROUP BY LineItemT.ID,
					CASE WHEN PartiallyAppliedPaysQ.ID IS NULL THEN CASE WHEN LineItemT_1.ID IS NULL THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END,
					CASE WHEN DoctorsProviders IS NULL THEN PaymentsT.ProviderID ELSE DoctorsProviders END,
					PatientsT.PersonID,
					PatientsT.UserDefinedID,
					PersonT.FullName,
					LineItemT_1.InputDate,
					LineItemT_1.DATE,
					PaymentsT.PayMethod,
					LineItemT.Description,
					ChargesT.ItemCode,
					LineItemT.Amount,
					dbo.GetChargeTotal(ChargesT.ID),
					LineItemT_1.DATE,
					LineItemT_1.Description,
					CreditCardNamesT.CardName,
					PaymentPlansT.CheckNo,
					ChargesT.Category,
					AppliesT.ID,
					LineItemT.ID,
					LineItemT.Type,
					ChargesT.ItemCode,
					CASE WHEN LineItemT_1.LocationID IS NULL THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END,
					CASE WHEN LineItemT_1.LocationID IS NULL THEN LocationsT.NAME ELSE ChargeLoc.NAME END,
					CPTCodeT.ID,
					LineItemT.[DATE],
					ServiceT.ID,
					CPTCodeT.Code,
					BillLabTestCodesT.ID,
					BillLabTestCodesT.Code,
					BillLabTestCodesT.Description,
					BillsT.DATE
				HAVING (((LineItemT.Type) = 1))
				) AS AdjustmentsByCPTCodeFullQ LEFT JOIN (
				SELECT CategoriesT.ID AS CategoryID,
					CategoriesT.NAME AS Category,
					'' AS SubCategory,
					CategoriesT.ID AS ParentID
				FROM CategoriesT
				WHERE (((CategoriesT.Parent) = 0))
				
				UNION
				
				SELECT SubCategoriesT.ID AS CategoryID,
					CategoriesT.NAME AS Category,
					SubCategoriesT.NAME AS SubCategory,
					SubCategoriesT.Parent AS ParentID
				FROM CategoriesT
				RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent
				WHERE (((SubCategoriesT.Parent) <> 0))
				) AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID
			) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID)))");
			break;
	case 757:
		// (b.spivey, July 31st, 2014) - PLID 62970 - Lockbox Payments report. 
		// (b.spivey, October 3, 2014) - PLID 62970 - Changed the query to pull the patient's applies against a charge and subtract them from the patient's responsibility on a charge. 
		// (b.spivey, October 15, 2014) - PLID 62970 - Changed report query to not pull deleted payments. 
		return R"(
		SELECT LBPay.LockBoxBatchID
			,LBBatch.LocationID AS LocID
			,LBBatch.BankName
			,LBBatch.PaymentDate
			,LBBatch.PaymentDate AS TDate
			,PaymentTotalQ.PaymentTotal
			,CheckNo AS CheckNumber
			,CheckAcctNo AS AccountNumber
			,PersonT.First AS PatientFirst
			,PersonT.Last AS PatientLast
			,PersonT.Middle AS PatientMiddle
			,PatientsT.UserDefinedID
			,ChargeRespT.Amount AS TotalPatientResp
			,BankRoutingNum AS RoutingNumber
			,LITPayments.Description AS PaymentDescription
			,CASE 
				WHEN DetailAmount IS NULL
					THEN '0.00'
				ELSE DetailAmount
				END AS AppliedAmount
			,PayGroup.GroupName AS PaymentCategory
			,LITCharges.DATE AS ChargeDate
			,LITCharges.Description AS ChargeDescription
			,ChargesSubQ.ItemCode AS ItemCode
			,ChargeRespT.Amount AS PatientRespBalance
			,PaymentsT.ID AS PaymentID
			,LITCharges.ID AS ChargeID
			,TotalAppliedToChargeQ.PaymentTotal AS TotalAppliedToCharge
		FROM LockboxPaymentT LBPay
		INNER JOIN LockboxBatchT LBBatch ON LBPay.LockboxBatchID = LBBatch.ID
		LEFT JOIN LockboxPaymentMapT LBPayMap ON LBPay.ID = LBPayMap.LockboxPaymentID
		LEFT JOIN LineItemT LITPayments ON LBPayMap.PaymentID = LITPayments.ID
		LEFT JOIN PaymentsT ON LITPayments.ID = PaymentsT.ID
		LEFT JOIN PaymentGroupsT PayGroup ON PaymentsT.PaymentGroupID = PayGroup.ID
		LEFT JOIN PaymentPlansT PayPlans ON PaymentsT.ID = PayPlans.ID
		LEFT JOIN PatientsT ON LITPayments.PatientID = PatientsT.PersonID
		LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID
		LEFT JOIN (
			SELECT SUM(LineItemT.Amount) AS PaymentTotal
				,LineItemT.PatientID
				,LockboxPaymentT.LockboxBatchID
			FROM PaymentsT
			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID
			INNER JOIN LockboxPaymentMapT ON LineItemT.ID = LockboxPaymentMapT.PaymentID
			INNER JOIN LockboxPaymentT ON LockboxPaymentMapT.LockboxPaymentID = LockboxPaymentT.ID
			WHERE LineItemT.Deleted = 0
			GROUP BY LineItemT.PatientID
				,LockboxBatchID
			) PaymentTotalQ ON PersonT.ID = PaymentTotalQ.PatientID
			AND LBBatch.ID = PaymentTotalQ.LockboxBatchID
		LEFT JOIN (
			SELECT AppliesT.SourceID AS PaymentID
				,SUM(ApplyDetailsT.Amount) AS DetailAmount
				,ChargesT.ID AS ChargeID
				,ChargesT.ServiceID AS ServiceID
				,ChargesT.ItemCode AS ItemCode
			FROM AppliesT
			INNER JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID
			INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID
			INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
			WHERE LineItemT.Deleted = 0
			GROUP BY AppliesT.SourceID
				,ChargesT.ID
				,ChargesT.ServiceID
				,ChargesT.ItemCode
			) ChargesSubQ ON PaymentsT.ID = ChargesSubQ.PaymentID
		LEFT JOIN (
			SELECT SUM(AppliesT.Amount) AS PaymentTotal
				,ChargesT.ID AS ChargeID
			FROM AppliesT
			INNER JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID
			INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID 
			INNER JOIN LineItemT LITCharge ON ChargesT.ID = LITCharge.ID 
			INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID
			INNER JOIN LineItemT LITPayment ON PaymentsT.ID = LITPayment.ID
			WHERE InsuredPartyID = - 1 AND LITPayment.Deleted = 0 AND LITCharge.Deleted = 0
			GROUP BY ChargesT.ID
			) TotalAppliedToChargeQ ON ChargesSubQ.ChargeID = TotalAppliedToChargeQ.ChargeID
		LEFT JOIN LineItemT LITCharges ON ChargesSubQ.ChargeID = LITCharges.ID
		LEFT JOIN ChargeRespT ON ChargesSubQ.ChargeID = ChargeRespT.ChargeID
			AND ChargeRespT.InsuredPartyID IS NULL
		WHERE PersonT.ID IS NOT NULL AND LITPayments.Deleted = 0 AND (LITCharges.Deleted = 0 OR LITCharges.Deleted IS NULL) 
				)";
		break;
	case 759:
		//Payments by Billed Service Category
		/*	Version History
		// (s.tullis 2015-06-15 15:55) - PLID 66293 -added
		*/
		return _T("SELECT PaymentsByCPTCategoryFullQ.ProvID AS ProvID,  "
			"PaymentsByCPTCategoryFullQ.FullName,  "
			"PersonT.Last + ', ' + PersonT.First AS DocName,  "
			"PaymentsByCPTCategoryFullQ.TDate AS TDate,  "
			"PaymentsByCPTCategoryFullQ.PayMethod,  "
			"PaymentsByCPTCategoryFullQ.CCType,  "
			"PaymentsByCPTCategoryFullQ.CheckNo,  "
			"PaymentsByCPTCategoryFullQ.Description,  "
			"PaymentsByCPTCategoryFullQ.ApplyAmount,  "
			"CategoriesQ.Category,  "
			"CategoriesQ.SubCategory,  "
			"CategoriesQ.ParentID AS ParentID,  "
			"PaymentsByCPTCategoryFullQ.PatID AS PatID, "
			"PaymentsByCPTCategoryFullQ.UserDefinedID,  "
			"PaymentsByCPTCategoryFullQ.IDate AS IDate,  "
			"PaymentsByCPTCategoryFullQ.Type, "
			"CategoriesQ.CategoryID, "
			"PaymentsByCPTCategoryFullQ.LocID AS LocID, "
			"PaymentsByCPTCategoryFullQ.Location, CatFilterID AS CatFilterID, "
			"PaymentsByCPTCategoryFullQ.ItemDesc AS ChargeDescription, "
			"PaymentsByCPTCategoryFullQ.ServiceCode, PaymentsByCPTCategoryFullQ.ServiceName, "
			"PaymentsByCPTCategoryFullQ.ApplyDate AS ApplyDate, PaymentsByCPTCategoryFullQ.ChargeServiceDate AS ChargeServiceDate, "
			"PaymentsByCPTCategoryFullQ.ChargeInputDate AS ChargeInputDate, PaymentsByCPTCategoryFullQ.BillDate AS BillDate, "
			"PaymentsByCPTCategoryFullQ.BillInputDate AS BillInputDate, PaymentsByCPTCategoryFullQ.POSID, PaymentsByCPTCategoryFullQ.POSName, "
			"PaymentsByCPTCategoryFullQ.POSDesignation "
			"FROM ((((SELECT LineItemT.ID,  "
			"ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  "
			"CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,  "
			"PatientsT.PersonID AS PatID,  "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"LineItemT.InputDate AS IDate,  "
			"LineItemT.Date AS TDate,  "
			"PaymentsT.PayMethod,  "
			"LineItemT.Description,  "
			"ChargesT.ItemCode AS FirstOfItemCode,  "
			"LineItemT.Amount AS PayAmt,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
			"LineItemT_1.Date AS ServiceDate,  "
			"LineItemT_1.Description AS ItemDesc,  "
			"CreditCardNamesT.CardName AS CCType,  "
			"PaymentPlansT.CheckNo,  "
			//only change from payments by service category report
			"ChargesT.Category,  "
			"AppliesT.ID AS ApplyID,  "
			"LineItemT.ID AS LineID,  "
			"LineItemT.Type, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, ChargesT.Category AS CatFilterID, "
			" CPTCodeT.Code as ServiceCode, ServiceT.Name as ServiceName, "
			"AppliesT.InputDate AS ApplyDate, LineItemT_1.Date AS ChargeServiceDate, LineItemT_1.InputDate AS ChargeInputDate, "
			"BillsT.Date AS BillDate, BillsT.InputDate AS BillInputDate, BillsT.Location AS POSID, POSLocationsT.Name AS POSName, PlaceOfServiceCodesT.PlaceCodes AS POSDesignation "
			"FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
			"Sum(AppliesT.Amount) AS ApplyAmt,  "
			"/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
			"LineItemT_1.PatientID  "
			"FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID "
			"WHERE (((LineItemT_1.Deleted)=0)) "
			"GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0)) "
			") AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID) "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT POSLocationsT ON BillsT.Location = POSLocationsT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0))  AND GCTypesT.ServiceID IS NULL  "
			"GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ServiceT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.Code, ServiceT.Name, "
			"AppliesT.InputDate, LineItemT_1.Date, LineItemT_1.InputDate, BillsT.Date, BillsT.InputDate, BillsT.Location, POSLocationsT.Name, PlaceOfServiceCodesT.PlaceCodes, ChargesT.Category "
			"HAVING (((LineItemT.Type)=1)) "
			") AS PaymentsByCPTCategoryFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
			"FROM CategoriesT "
			"WHERE (((CategoriesT.Parent)=0)) "
			"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
			"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
			"WHERE (((SubCategoriesT.Parent)<>0)) "
			") AS CategoriesQ ON PaymentsByCPTCategoryFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON PaymentsByCPTCategoryFullQ.ProvID = PersonT.ID))");
		break;

	case 761:
		//Integrated Credit Card Payments/Refunds
		/*	Version History
 		// (j.jones 2015-09-30 10:58) - PLID 67179 - created - this is for Integrated Credit Card Processing only
 		// (b.eyers 2016-02-08) - PLID 68060 - this was showing multiple entries per processed card, should only show one
		// (r.goldschmidt 2016-06-30) - NX-100985 - ChargeRefund type CardConnect transactions should be treated as an updated amount when zero or positive. 
		//		Otherwise treat as independent transaction.
		*/
		return R"(
SELECT PaymentsSubQ.ID
	,PaymentsSubQ.PatientID
	,Sum(PaymentsSubQ.Amount) AS SumOfAmount
	,CASE WHEN CardConnect_CreditTransactionT.ID IS NOT NULL
		THEN CASE WHEN PaymentsSubQ.PayMethod = 9 AND CardConnect_CreditTransactionT.AuthAmount >= CONVERT(MONEY, 0) 
				/* assume a positive or zero value for a charge refund is an updated amount, not an independent transaction */
				THEN CardConnect_CreditTransactionT.Amount 
				/* assume a negative amount for charge refunds, or any amount for any other pay method, is an independent transaction */
				ELSE CardConnect_CreditTransactionT.AuthAmount 
				END
		ELSE Convert(MONEY, 0)
		END AS ProcessedAmount
	,CASE WHEN CardConnect_CreditTransactionT.ID IS NULL
		THEN Sum(PaymentsSubQ.Amount)
		ELSE Convert(MONEY, 0)
		END AS UnprocessedAmount
	,PaymentsSubQ.ProvID AS ProvID
	,PaymentsSubQ.PatID AS PatID
	,PaymentsSubQ.UserDefinedID
	,PaymentsSubQ.FullName
	,PaymentsSubQ.IDate AS IDate
	,PaymentsSubQ.TDate AS TDate
	,PaymentsSubQ.InputName AS InputName
	,PaymentsSubQ.PayMethod AS PayMethod
	,PaymentsSubQ.Description
	,PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName
	,PaymentsSubQ.LocID AS LocID
	,PaymentsSubQ.Location
	,PaymentsSubQ.CreditCard AS CreditCard
	,Right(PaymentsSubQ.CCNumber, 4) AS CCLast4
	,PaymentsSubQ.CCAuthNo
	,PaymentsSubQ.CCExpDate
	,CardConnect_CreditTransactionT.AccountID AS AccountID
	,CardConnect_SetupDataT.Description AS AccountDescription
FROM (
	(	SELECT LineItemT.ID
			,LineItemT.PatientID
			,LineItemT.Amount
			,ProviderID AS ProvID
			,LineItemT.PatientID AS PatID
			,PatientsT.UserDefinedID
			,PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName
			,LineItemT.InputDate AS IDate
			,LineItemT.DATE AS TDate
			,LineItemT.InputName
			,PaymentsT.PayMethod
			,LineItemT.Description
			,LineItemT.LocationID AS LocID
			,LocationsT.NAME AS Location
			,CASE WHEN ASCII(SUBSTRING(CreditCardNamesT.CardName, 1, 1)) = 1 THEN '' ELSE CreditCardNamesT.CardName END AS CreditCard
			,PaymentPlansT.CCNumber
			,PaymentPlansT.CCAuthNo
			,PaymentPlansT.CCExpDate
		FROM LineItemT
		LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID
		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
		LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID
		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID
		LEFT JOIN (
			PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
			) ON LineItemT.PatientID = PatientsT.PersonID
		LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID
		WHERE LineItemT.Deleted = 0
			AND LineItemT.Type IN (1, 3)
			AND PaymentsT.PayMethod IN (3, 9)
		) AS PaymentsSubQ 
	LEFT JOIN ProvidersT ON PaymentsSubQ.ProvID = ProvidersT.PersonID
	)
LEFT JOIN PersonT ON PaymentsSubQ.ProvID = PersonT.ID
LEFT JOIN CardConnect_CreditTransactionT ON PaymentsSubQ.ID = CardConnect_CreditTransactionT.ID
LEFT JOIN CardConnect_SetupDataT ON CardConnect_CreditTransactionT.AccountID = CardConnect_SetupDataT.ID
GROUP BY PaymentsSubQ.ID, PaymentsSubQ.PatientID, PaymentsSubQ.ProvID, CardConnect_CreditTransactionT.ID
, CardConnect_CreditTransactionT.Amount, CardConnect_CreditTransactionT.AuthAmount, CardConnect_CreditTransactionT.Token
, CardConnect_CreditTransactionT.AuthCode, PaymentsSubQ.PatID, PaymentsSubQ.FullName, PaymentsSubQ.IDate
, PaymentsSubQ.TDate, PaymentsSubQ.InputName, PaymentsSubQ.PayMethod, PaymentsSubQ.Description
, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PaymentsSubQ.UserDefinedID, PaymentsSubQ.LocID
, PaymentsSubQ.Location, PaymentsSubQ.CreditCard, PaymentsSubQ.CCNumber, PaymentsSubQ.CCAuthNo, PaymentsSubQ.CCExpDate
, CardConnect_CreditTransactionT.AccountID, CardConnect_SetupDataT.Description
			)";
		break;

	// Capitation Batch Payments
	case 762:	// (r.gonet 2015-11-09) - PLID 67466 - Capitation Distribution	
	case 763:   // (s.tullis 2015-11-10 11:31) - PLID 67467 - Capitation Batch Payments
	{
		// (r.gonet 2015-11-09) - PLID 67466 - Created the query to serve as a base for a couple capitation batch payment reports. Started with the Batch Payments report and heavily modified it.
		CString str = R"(
SELECT 
	BatchPaymentInsuranceCoT.PersonID AS InsuranceID, 
	BatchPaymentInsuranceCoT.Name AS InsName,
	BatchPaymentsT.ID AS BatchID, 
	PaymentLineItemQ.ID AS ChildPaymentID, 
	BatchPaymentsT.Description AS BatchDescription, 
	BatchPaymentsT.Amount AS BatchAmount,
	BatchPaymentsT.OriginalAmount AS BatchOriginalAmount,
	BatchPaymentsT.[Date] AS TDate, 
	BatchPaymentsT.InputDate AS IDate, 
	PaymentLineItemQ.InputDate AS PayDate,
	BatchPaymentsT.CheckNo AS CheckNo, 
	BatchPaymentsT.BankRoutingNum AS RoutingNum, 
	BatchPaymentsT.BankName AS BankNum, 
	BatchPaymentsT.CheckAcctNo AS CheckAcctNo, 
	BatchPaymentLocationsT.ID AS LocID, 
	BatchPaymentLocationsT.Name AS LocName,
	BatchPaymentsT.ServiceDateFrom AS BatchServiceDateFrom,
	BatchPaymentsT.ServiceDateTo AS BatchServiceDateTo,
	ChargeLineItemQ.LocationID AS ChargeLocationID, 
	ChargeLineItemQ.Date AS ChargeDate,
	ChargeLocationsT.Name AS ChargeLocationName,
	PatientsT.PersonID AS PatID, 
	PatientsT.UserDefinedID,
	PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatName, 
	ChargesT.BillID,
	ChargesT.ID AS ChargeID,
	ChargesT.Allowable as ChargeAllowable,
	ChargeLineItemQ.Description AS ChargeDescription,
	BatchPaymentCapitationDetailsT.AllowableUsed,
	CONVERT(MONEY, ISNULL(PatientChargeRespQ.Amount, '0.00')) AS PatientResp,
	CONVERT(MONEY, ISNULL(BatchPaymentCapitationDetailsT.Copay, '0.00')) AS Copay,
	PaymentLineItemQ.Amount AS PayAmt,
	BatchPaymentCapitationDetailsT.CalculatedPayment,
	CapitationReimbursementRateQ.ReimbursementRate,
	dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmt,
	ChargeProviderPersonT.ID AS ProvID, 
	ChargeProviderPersonT.Last + ', ' + ChargeProviderPersonT.First + ' ' + ChargeProviderPersonT.Middle AS ProvName, 
	ChargeProvidersT.CapitationDistribution AS ProvCapitationDistribution,
	BillsT.Date AS BillDate,
	BillsT.InputDate AS BillInputDate
FROM BatchPaymentsT 
INNER JOIN BatchPaymentCapitationDetailsT ON BatchPaymentsT.ID = BatchPaymentCapitationDetailsT.BatchPaymentID
INNER JOIN
(
	SELECT 
		BatchPaymentsT.ID AS BatchPaymentID, 
		ROUND(100.0 * 
			CONVERT(FLOAT, BatchPaymentsT.Amount + SUM(BatchPaymentCapitationDetailsT.Copay)) 
			/ CONVERT(FLOAT, SUM(BatchPaymentCapitationDetailsT.AllowableUsed))
		, 2) AS ReimbursementRate
	FROM BatchPaymentsT
	INNER JOIN BatchPaymentCapitationDetailsT ON BatchPaymentsT.ID = BatchPaymentCapitationDetailsT.BatchPaymentID
	WHERE BatchPaymentsT.Capitation = 1
	GROUP BY 
		BatchPaymentsT.ID, 
		BatchPaymentsT.Amount 
) CapitationReimbursementRateQ ON BatchPaymentsT.ID = CapitationReimbursementRateQ.BatchPaymentID 
INNER JOIN ChargesT ON BatchPaymentCapitationDetailsT.ChargeID = ChargesT.ID 
INNER JOIN 
(
	SELECT 
		LineItemT.ID, 
		LineItemT.Description, 
		LineItemT.LocationID,
		LineItemT.Date
	FROM LineItemT 
	WHERE LineItemT.Deleted = 0
) ChargeLineItemQ ON ChargesT.ID = ChargeLineItemQ.ID 
LEFT JOIN
(
	SELECT
		ChargeRespT.ChargeID,
		ChargeRespT.Amount
	FROM ChargeRespT 
	WHERE ChargeRespT.InsuredPartyID IS NULL
) PatientChargeRespQ ON ChargesT.ID = PatientChargeRespQ.ChargeID
INNER JOIN PersonT ChargeProviderPersonT ON ChargesT.DoctorsProviders = ChargeProviderPersonT.ID 
INNER JOIN ProvidersT ChargeProvidersT ON ChargeProviderPersonT.ID = ChargeProvidersT.PersonID 
INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID
INNER JOIN InsuranceCoT BatchPaymentInsuranceCoT ON BatchPaymentsT.InsuranceCoID = BatchPaymentInsuranceCoT.PersonID 
INNER JOIN LocationsT BatchPaymentLocationsT ON BatchPaymentsT.Location = BatchPaymentLocationsT.ID 
INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID 
INNER JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID 
LEFT JOIN
(
	SELECT 
		PaymentsT.ID, 
		PaymentsT.BatchPaymentID, 
		LineItemT.PatientID, 
		LineItemT.Amount, 
		LineItemT.Description, 
		LineItemT.InputDate, 
		PaymentsT.InsuredPartyID,
		AppliesT.DestID AS ApplyDestID
	FROM PaymentsT 
	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
	LEFT JOIN 
	(
		SELECT 
			LineItemCorrectionsT.ID,
			LineItemCorrectionsT.OriginalLineItemID, 
			LineItemCorrectionsT.VoidingLineItemID 
		FROM LineItemCorrectionsT 
		WHERE LineItemCorrectionsT.BatchPaymentID IS NULL
	) LineItemCorrectionsQ ON LineItemT.ID = LineItemCorrectionsQ.OriginalLineItemID OR LineItemT.ID = LineItemCorrectionsQ.VoidingLineItemID 
	INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID
	WHERE LineItemT.Deleted = 0 
		AND LineItemT.Type = 1
		AND LineItemCorrectionsQ.ID IS NULL
) PaymentLineItemQ ON BatchPaymentsT.ID = PaymentLineItemQ.BatchPaymentID AND PatientsT.PersonID = PaymentLineItemQ.PatientID AND ChargesT.ID = PaymentLineItemQ.ApplyDestID
INNER JOIN LocationsT ChargeLocationsT ON ChargeLineItemQ.LocationID = ChargeLocationsT.ID
WHERE BatchPaymentsT.Deleted = 0 
	AND BatchPaymentsT.Type = 1 
	AND BatchPaymentsT.Capitation = 1
)";
		// (r.gonet 2015-11-09) - PLID 67466 - If this is the Capitation Distribution report, then filter out the providers with no capitation distribution.
		if (nID == 762) {
			str += " AND ChargeProvidersT.CapitationDistribution > 0";
		}

		return str;
	}
	break;

	default:
		return _T("");
		break;
	}
}

/// <summary>
/// Returns the report SQL query for the Payments Under Allowed Amount report.
/// </summary>
/// <remarks>
/// Uses a temp table to store intermediate results for when the caller is filtering on
/// a specific EOB. This is following the precedent of the Financial Activity queries.
/// This improves the cardinality estimates and allows us to filter line items and charges 
/// much earlier because there will be much fewer of these tied to a particular EOB than 
/// all charges and line items in the database.
/// If no EOB filtering is done though, then the usual monolithic report query is returned
/// because the temp tables and table variables could be flooded. I contend that this
/// still results in better performance though on certain test databases, but don't trust it
/// fully to use in general.
/// </remarks>
/// <param name="varEobID">If the report is to filter on a specific EOB, then this
/// should be the ID (VT_I4) of that EOB. Otherwise, it should be VT_NULL.</param>
const CString GetPaymentsUnderAllowedAmountReportQuery(_variant_t varEobID)
{
	CString strTempTableName;
	CParamSqlBatch sqlTempTableCreationBatch;
	CSqlFragment sqlEobBatchPaymentsJoin, sqlAppliedToChargeJoin;
	
	bool bUseEobFilter = (varEobID != g_cvarNull && varEobID.vt == VT_I4);
	
	// If we're not doing any filtering, just defer to the old query since we can potentially flood the temp table
	// with millions and millions of records. This is also a possibility if filtering on large date ranges, in which case
	// the user can opt to disable using a temp table for this report through ConfigRT. If this optimization fails for a client,
	// you can disable it through inserting a record in ConfigRT with the name SupportTempPaymentsUnderAllowedAmountReport.
	bool bUseTempTableOptimization = (bUseEobFilter && GetRemotePropertyInt("SupportTempPaymentsUnderAllowedAmountReport", 1, 0, "<None>", false));
	
	if (bUseTempTableOptimization)
	{
		// Split out some of the work into table variables. SQL plan for the large subquery later seems more agreeable with estimates when these are split out.
		// I know that table variables don't use statistics but this seems to work in our favor. I also tried temp tables and the plan's cardinality
		// estimate was better, but the plan was worse.
		sqlTempTableCreationBatch.Add(R"(
DECLARE @EobID INT SET @EobID = {INT};

DECLARE @EobBatchPaymentT TABLE
(
	BatchPaymentID INT NOT NULL PRIMARY KEY
);

-- From observation of different databases, this is commonly going to select a single row, but not always.
INSERT INTO @EobBatchPaymentT (BatchPaymentID)
SELECT DISTINCT ERemittanceHistoryT.BatchPayID
FROM ERemittanceHistoryT
WHERE ERemittanceHistoryT.EOBID = @EobID;
)", VarLong(varEobID));

		sqlEobBatchPaymentsJoin = CSqlFragment("INNER JOIN @EobBatchPaymentT EobBatchPaymentT ON PaymentsT.BatchPaymentID = EobBatchPaymentT.BatchPaymentID");

		sqlTempTableCreationBatch.Add(R"(
DECLARE @AppliedToChargeT TABLE
(
	ChargeID INT NOT NULL PRIMARY KEY
);

INSERT INTO @AppliedToChargeT (ChargeID)
SELECT DISTINCT
	ChargeRespT.ChargeID
FROM AppliesT 
INNER JOIN LineItemT PaymentLineItemT ON AppliesT.SourceID = PaymentLineItemT.ID 
INNER JOIN PaymentsT ON PaymentLineItemT.ID = PaymentsT.ID 
{SQL}
INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID
WHERE PaymentLineItemT.Type = 1 AND PaymentLineItemT.Deleted = 0;
)", sqlEobBatchPaymentsJoin);

		sqlAppliedToChargeJoin = CSqlFragment("INNER JOIN @AppliedToChargeT AppliedToChargeT ON ChargesT.ID = AppliedToChargeT.ChargeID");

		// We want statistics here. Use a temp table.
		strTempTableName = FormatString("#ChargesWithPaymentsUnderAllowedAmountT_%s", NewPlainUUID());
		sqlTempTableCreationBatch.Add(FormatString(R"(
IF OBJECT_ID('tempdb..%s') IS NOT NULL
	DROP TABLE %s

CREATE TABLE %s
(
	BillID INT,
	ChargeID INT,
	ChargeRespID INT,
	Quantity FLOAT,
	ChargeDate DATETIME,
	ItemCode NVARCHAR(50),
	ChargeDescription NVARCHAR(255),
	LocID INT,
	POSID INT,
	ServiceID INT,
	LocationID INT,
	DoctorsProviders INT,
	Date DATETIME,
	PatID INT,
	InsuredPartyID INT,
	InsuranceCoID INT,
	InsuranceName NVARCHAR(255),
	RespTypeName NVARCHAR(255),
	RespTypeID INT,
	ChargeAmt MONEY,
	AppliedAmount MONEY,
	Allowable MONEY,
	AllowableQty FLOAT,
	MultiFeeInsID INT,
	CoPay MONEY,
	IncludeCoPayAllowable BIT, 
	IncludePatRespAllowable BIT, 
	InsResp MONEY, 
	PatientResp MONEY, 
	OtherInsResps MONEY, 
	FeeScheduleID INT, 
	FeeScheduleName NVARCHAR(50)
);

INSERT INTO %s
(
	BillID,
	ChargeID,
	ChargeRespID,
	Quantity,
	ChargeDate,
	ItemCode,
	ChargeDescription,
	LocID,
	POSID,
	ServiceID,
	LocationID,
	DoctorsProviders,
	Date,
	PatID,
	InsuredPartyID,
	InsuranceCoID,
	InsuranceName,
	RespTypeName,
	RespTypeID,
	ChargeAmt,
	AppliedAmount,
	Allowable,
	AllowableQty,
	MultiFeeInsID,
	CoPay,
	IncludeCoPayAllowable, 
	IncludePatRespAllowable, 
	InsResp, 
	PatientResp, 
	OtherInsResps, 
	FeeScheduleID, 
	FeeScheduleName
))", strTempTableName, strTempTableName, strTempTableName, strTempTableName));
	} 
	else if(bUseEobFilter)
	{
		// We aren't using the temp table optimization, per a ConfigRT setting, but still need to filter on the EOB ID.
		// Create the filter here.
		sqlEobBatchPaymentsJoin = CSqlFragment(
R"(INNER JOIN
(
	SELECT DISTINCT ERemittanceHistoryT.BatchPayID
	FROM ERemittanceHistoryT
	WHERE ERemittanceHistoryT.EOBID = {INT}
) EobBatchPaymentQ ON PaymentsT.BatchPaymentID = EobBatchPaymentQ.BatchPayID )", VarLong(varEobID));
	}


	// Get the large subquery that, if optimized, we'll be storing a temp table.

	CSqlFragment sqlChargesWithPaymentsUnderAllowedAmountQ(
R"(SELECT Min(AllowablesQ.BillID) AS BillID, AllowablesQ.ChargeID, AllowablesQ.ChargeRespID, 
	AllowablesQ.Quantity, AllowablesQ.ChargeDate, AllowablesQ.ItemCode, AllowablesQ.ChargeDescription, 
	AllowablesQ.LocID AS LocID, 
	AllowablesQ.POSID AS POSID, 
	AllowablesQ.ServiceID, AllowablesQ.LocationID, AllowablesQ.DoctorsProviders, AllowablesQ.Date, 
	AllowablesQ.PatID AS PatID, AllowablesQ.InsuredPartyID, 
	AllowablesQ.InsuranceCoID, AllowablesQ.InsuranceName, AllowablesQ.RespTypeName, AllowablesQ.RespTypeID, 
	AllowablesQ.ChargeAmt, Sum(AllowablesQ.AppliedAmount) AS AppliedAmount, Allowable, 
	Round(Convert(money, Allowable * Quantity * 
	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * 
	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * 
	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * 
	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) 
	), 2) AS AllowableQty, 
	MultiFeeInsuranceT.InsuranceCoID AS MultiFeeInsID, 
	Coalesce((CASE WHEN AllowablesQ.CopayType = 0 THEN AllowablesQ.CoPay ELSE Round(convert(money, AllowablesQ.ChargeAmt * (Convert(float,AllowablesQ.CopayPercent)/Convert(float,100))),2) END),0) AS CoPay, 
	MultiFeeGroupsT.IncludeCoPayAllowable, MultiFeeGroupsT.IncludePatRespAllowable, 
	Coalesce(AllowablesQ.InsResp,0) AS InsResp, 
	Coalesce(AllowablesQ.PatientResp,0) AS PatientResp, 
	Coalesce(AllowablesQ.OtherInsResps,0) AS OtherInsResps, 
	MultiFeeGroupsT.ID AS FeeScheduleID, MultiFeeGroupsT.Name AS FeeScheduleName 
FROM ( 
	SELECT BillsT.ID AS BillID, ChargesT.ID AS ChargeID, ChargeRespT.Amount AS InsResp, ChargeRespT.ID AS ChargeRespID, ChargesT.Quantity AS Quantity, 
	ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, 
	LineItemT.Date AS ChargeDate, AppliesT.Date AS TDate, AppliesT.Date AS IDate, ChargesT.ItemCode, LineItemT.Description AS ChargeDescription, 
	LineItemT.LocationID AS LocID, BillsT.Location AS POSID, ChargesT.ServiceID, LineItemT.LocationID, ChargesT.DoctorsProviders, 
	BillsT.Date, BillsT.PatientID AS PatID, 
	InsuredPartyT.PersonID AS InsuredPartyID, 
	InsuredPartyT.InsuranceCoID AS InsuranceCoID, InsuranceCoT.Name AS InsuranceName, 
	RespTypeT.TypeName AS RespTypeName, RespTypeT.ID AS RespTypeID, 
	dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmt, 
	(CASE WHEN AppliesT.Amount Is Null THEN 0 ELSE AppliesT.Amount END) AS AppliedAmount, 
	InsuredPartyT.CoPay, 
	CASE WHEN InsuredPartyT.Copay IS NULL AND InsuredPartyT.CopayPercent IS NOT NULL THEN 1 ELSE 0 END AS CopayType, 
	InsuredPartyT.CopayPercent, 
	-- OtherRespTotal is the total of patient resp + insurance resps that aren't this insured party,
	-- this total is used in our allowable calculations below
	dbo.GetChargeTotal(ChargesT.ID) - ChargeRespT.Amount AS OtherRespTotal, 				
	Coalesce(PatientRespQ.TotalAmount, 0) AS PatientResp, 
	-- OtherInsResps is the total of other insurance resps that aren't this insured party
	dbo.GetChargeTotal(ChargesT.ID) - ChargeRespT.Amount - Coalesce(PatientRespQ.TotalAmount, 0) AS OtherInsResps 
	FROM BillsT 
	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID
	{SQL} 
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID 
	LEFT JOIN (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM 
		(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT 
		LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID 
		) AS InsPartyPayGroupsT ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID 
	) AS InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID 
	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
	LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID 
	INNER JOIN (SELECT RespID, LineItemT.InputDate, LineItemT.Date, Sum(CASE WHEN AppliesT.Amount Is Null THEN 0 ELSE AppliesT.Amount END) AS Amount FROM AppliesT 
		INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID 
		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID 
		{SQL}
		WHERE LineItemT.Type = 1 AND LineItemT.Deleted = 0  
		GROUP BY AppliesT.RespID, LineItemT.InputDate, LineItemT.Date 
	) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID 

	-- PatientRespQ gets the patient resp for this charge, there
	-- really should never be multiple records for one charge
	LEFT JOIN (SELECT ChargeID, Sum(Amount) AS TotalAmount 
		FROM ChargeRespT 
		WHERE InsuredPartyID Is Null 	
		GROUP BY ChargeRespT.ChargeID 
	) AS PatientRespQ ON ChargesT.ID = PatientRespQ.ChargeID 

	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 

	-- we only want payments for the same insured party as the allowable
	AND IsNull(InsuredPartyT.PersonID, -1) = IsNull(ChargesT.AllowableInsuredPartyID, -1) 
) AS AllowablesQ 

INNER JOIN MultiFeeItemsT ON AllowablesQ.ServiceID = MultiFeeItemsT.ServiceID 
INNER JOIN MultiFeeInsuranceT ON MultiFeeItemsT.FeeGroupID = MultiFeeInsuranceT.FeeGroupID 
INNER JOIN MultiFeeGroupsT ON MultiFeeItemsT.FeeGroupID = MultiFeeGroupsT.ID 
LEFT JOIN MultiFeeLocationsT ON MultiFeeItemsT.FeeGroupID = MultiFeeLocationsT.FeeGroupID 
LEFT JOIN MultiFeeProvidersT ON MultiFeeItemsT.FeeGroupID = MultiFeeProvidersT.FeeGroupID 
GROUP BY 
	AllowablesQ.ChargeID, AllowablesQ.ChargeRespID, AllowablesQ.Quantity, AllowablesQ.ChargeDate, AllowablesQ.LocID, AllowablesQ.POSID, AllowablesQ.ItemCode, 
	AllowablesQ.CPTMultiplier1, AllowablesQ.CPTMultiplier2, AllowablesQ.CPTMultiplier3, AllowablesQ.CPTMultiplier4, AllowablesQ.ChargeDescription, 
	AllowablesQ.ServiceID, AllowablesQ.LocationID, AllowablesQ.DoctorsProviders, AllowablesQ.Date, AllowablesQ.PatID, AllowablesQ.InsuredPartyID,
	AllowablesQ.InsuranceCoID, AllowablesQ.InsuranceName, AllowablesQ.RespTypeName, AllowablesQ.RespTypeID, 
	AllowablesQ.ChargeAmt, AllowablesQ.CopayType, AllowablesQ.CoPay, AllowablesQ.CopayPercent, 
	Allowable, MultiFeeInsuranceT.InsuranceCoID, MultiFeeLocationsT.LocationID, MultiFeeProvidersT.ProviderID, MultiFeeGroupsT.IncludeCoPayAllowable, MultiFeeGroupsT.IncludePatRespAllowable, 
	AllowablesQ.OtherRespTotal, MultiFeeGroupsT.UsePOS, AllowablesQ.InsResp, AllowablesQ.PatientResp, AllowablesQ.OtherInsResps, 
	MultiFeeGroupsT.ID, MultiFeeGroupsT.Name 
	, MultiFeeGroupsT.EffectiveFromDate, MultiFeeGroupsT.EffectiveToDate 
HAVING (Sum(AllowablesQ.AppliedAmount) > 0) AND (Sum(AllowablesQ.AppliedAmount) < ChargeAmt) AND (Allowable Is Not Null) 
AND (
-- compare the allowable to the amount paid
(IncludeCoPayAllowable = 0 AND IncludePatRespAllowable = 0 AND 
	Round(Convert(money, Allowable * Quantity * 
	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * 
	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * 
	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * 
	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) 
	), 2) > Sum(AllowablesQ.AppliedAmount)) 
OR 
-- compare the allowable to the amount paid + copay
(IncludeCoPayAllowable <> 0 AND IncludePatRespAllowable = 0 AND 
	Round(Convert(money, Allowable * Quantity * 
	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * 
	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * 
	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * 
	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) 
	), 2) > (Sum(AllowablesQ.AppliedAmount) + Coalesce((((CASE WHEN AllowablesQ.CopayType = 0 THEN AllowablesQ.CoPay ELSE Round(convert(money, AllowablesQ.ChargeAmt * (Convert(float,AllowablesQ.CopayPercent)/Convert(float,100))),2) END))),0))) 
OR 
-- compare the allowable to the amount paid + pat./other resp.
(IncludeCoPayAllowable = 0 AND IncludePatRespAllowable <> 0 AND 
	Round(Convert(money, Allowable * Quantity * 
	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * 
	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * 
	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * 
	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) 
	), 2) > Sum(AllowablesQ.AppliedAmount) + OtherRespTotal) 
OR 
-- compare the allowable to the greater of the amount paid or the pat./other resp.
(IncludeCoPayAllowable <> 0 AND IncludePatRespAllowable <> 0 AND 
	Round(Convert(money, Allowable * Quantity * 
	(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * 
	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * 
	(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * 
	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) 
	), 2) > (Sum(AllowablesQ.AppliedAmount) + (CASE WHEN OtherRespTotal > Coalesce((((CASE WHEN AllowablesQ.CopayType = 0 THEN AllowablesQ.CoPay ELSE Round(convert(money, AllowablesQ.ChargeAmt * (Convert(float,AllowablesQ.CopayPercent)/Convert(float,100))),2) END))),0) THEN OtherRespTotal ELSE Coalesce((((CASE WHEN AllowablesQ.CopayType = 0 THEN AllowablesQ.CoPay ELSE Round(convert(money, AllowablesQ.ChargeAmt * (Convert(float,AllowablesQ.CopayPercent)/Convert(float,100))),2) END))),0) END))) 
) 
-- Account for charge date relative to fee schedule effective date
AND (MultiFeeGroupsT.EffectiveFromDate IS NULL OR (AllowablesQ.ChargeDate >= MultiFeeGroupsT.EffectiveFromDate AND (MultiFeeGroupsT.EffectiveToDate IS NULL OR AllowablesQ.ChargeDate <= MultiFeeGroupsT.EffectiveToDate))) 
-- We must compare on location and provider, but they do not explicitly have to match
AND (AllowablesQ.LocationID = MultiFeeLocationsT.LocationID OR MultiFeeLocationsT.LocationID Is Null) 
AND (MultiFeeLocationsT.LocationID Is Null 
	OR (UsePOS = 0 AND MultiFeeLocationsT.LocationID = AllowablesQ.LocationID) 
	OR (UsePOS = 1 AND MultiFeeLocationsT.LocationID = AllowablesQ.POSID) 
	) 
AND (AllowablesQ.DoctorsProviders = MultiFeeProvidersT.ProviderID OR MultiFeeProvidersT.ProviderID Is Null) 
AND AllowablesQ.InsuranceCoID = MultiFeeInsuranceT.InsuranceCoID 
)", sqlAppliedToChargeJoin, sqlEobBatchPaymentsJoin);

	// Need to figure out how this will get put into the outer report query depending on if optimization is on or not.
	CSqlFragment sqlChargesWithPaymentsUnderAllowedAmountAsSubQ;
	if (bUseTempTableOptimization)
	{
		// Results are stored in a temp table. If you'll recall from above, the last statement in the batch
		// is half an incomplete insert statement which inserts into the temp table. This completes that insert.
		sqlTempTableCreationBatch.Add(sqlChargesWithPaymentsUnderAllowedAmountQ);
		sqlChargesWithPaymentsUnderAllowedAmountAsSubQ = CSqlFragment("{CONST_STRING}", strTempTableName);
	}
	else
	{
		// Embed the subquery. Don't use the temp table.
		sqlChargesWithPaymentsUnderAllowedAmountAsSubQ = CSqlFragment("({SQL})", sqlChargesWithPaymentsUnderAllowedAmountQ);
	}


	// Top level report query.
	CSqlFragment sqlOuterReportQuery(R"(
SELECT SubQ.BillID, SubQ.ChargeID, SubQ.ChargeRespID, SubQ.Quantity, 
	SubQ.ChargeDate, SubQ.LocID AS LocID, SubQ.ServiceID, SubQ.Date, SubQ.ItemCode, SubQ.ChargeDescription, 
	SubQ.PatID AS PatID, 
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, 
	PatientsT.UserDefinedID, SubQ.InsuredPartyID, 
	SubQ.RespTypeName, SubQ.RespTypeID AS PaidRespTypeID, SubQ.InsuranceCoID AS InsuranceCoID, SubQ.InsuranceName, 
	SubQ.ChargeAmt, SubQ.InsResp, SubQ.PatientResp, SubQ.OtherInsResps, 
	SubQ.AppliedAmount, SubQ.Allowable, SubQ.AllowableQty, SubQ.MultiFeeInsID, SubQ.CoPay, SubQ.IncludeCoPayAllowable, 
	SubQ.FeeScheduleID, SubQ.FeeScheduleName, 
	AdditionalLineItemsQ.GroupCode AS AdjGroupCode, AdditionalLineItemsQ.GroupCodeDesc AS AdjGroupCodeDesc, 
	AdditionalLineItemsQ.ReasonCode AS AdjReasonCode, AdditionalLineItemsQ.ReasonCodeDesc AS AdjReasonCodeDesc, 
	AdditionalLineItemsQ.ID AS AppliedLineItemID, AdditionalLineItemsQ.AppliedAmount AS LineItemAppliedAmount, 
	AdditionalLineItemsQ.Amount AS LineItemAmount, AdditionalLineItemsQ.Date AS LineItemDate, AdditionalLineItemsQ.InputDate AS LineItemInputDate, 
	AdditionalLineItemsQ.Description AS LineItemDescription, AdditionalLineItemsQ.Type AS LineItemType, 
	AdditionalLineItemsQ.OtherChargeItemCode, AdditionalLineItemsQ.OtherChargeQuantity, 
	AdditionalLineItemsQ.OtherChargeInsResp, AdditionalLineItemsQ.OtherChargeInsPaid, AdditionalLineItemsQ.OtherChargeInsAdj,  
	AdditionalLineItemsQ.OtherChargePatientResp, AdditionalLineItemsQ.OtherChargeOtherInsResps 
FROM {SQL} SubQ
INNER JOIN PatientsT ON SubQ.PatID = PatientsT.PersonID
INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID 
-- join all applied credits for the same insured party,
-- and all other charges for the same bill
INNER JOIN (SELECT LineItemT.ID, AppliesQ.BillID, LineItemT.Type, AppliesQ.DestID, AppliesQ.RespID, 
	AppliesQ.AppliedAmount, LineItemT.Amount, 
	LineItemT.Date, LineItemT.InputDate, LineItemT.Description, PaymentsT.InsuredPartyID, 
	AdjustmentGroupCodesT.Code AS GroupCode, Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, 
	AdjustmentReasonCodesT.Code AS ReasonCode, Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc, 
	NULL AS OtherChargeItemCode, NULL AS OtherChargeQuantity, 
	NULL AS OtherChargeInsResp, NULL AS OtherChargeInsPaid, NULL AS OtherChargeInsAdj,  
	NULL AS OtherChargePatientResp, NULL AS OtherChargeOtherInsResps 
	FROM LineItemT 
	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID 
	INNER JOIN (SELECT AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, ChargesT.BillID, 
		Sum(AppliesT.Amount) AS AppliedAmount 
		FROM AppliesT 
		INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID 
		INNER JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID 
		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
		WHERE LineItemT.Deleted = 0 
		GROUP BY AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, ChargesT.BillID
	) AS AppliesQ ON LineItemT.ID = AppliesQ.SourceID 
	LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID 
	LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID 
	WHERE LineItemT.Deleted = 0 AND (LineItemT.Type = 1 OR LineItemT.Type = 2) 

	UNION ALL 

	SELECT LineItemT.ID, ChargesT.BillID, LineItemT.Type, NULL DestID, NULL AS RespID, 
	NULL AS AppliedAmount, dbo.GetChargeTotal(LineItemT.ID) AS Amount, 
	LineItemT.Date, LineItemT.InputDate, LineItemT.Description, InsuredPartyT.PersonID AS InsuredPartyID, 
	NULL AS GroupCode, NULL AS GroupCodeDesc, 
	NULL AS ReasonCode, NULL AS ReasonCodeDesc, 
	ChargesT.ItemCode AS OtherChargeItemCode, ChargesT.Quantity AS OtherChargeQuantity, 
	Coalesce(ChargeRespT.Amount, Convert(money, 0)) AS OtherChargeInsResp, 
	Coalesce(AppliesQ.PayAmount, Convert(money, 0)) AS OtherChargeInsPaid, 
	Coalesce(AppliesQ.AdjAmount, Convert(money, 0)) AS OtherChargeInsAdj,  
	Coalesce(PatientRespQ.TotalAmount, 0) AS OtherChargePatientResp, 
	-- OtherInsResps is the total of other insurance resps that aren't this insured party
	dbo.GetChargeTotal(ChargesT.ID) - ChargeRespT.Amount - Coalesce(PatientRespQ.TotalAmount, 0) AS OtherChargeOtherInsResps 
	FROM LineItemT 
	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID 
	INNER JOIN InsuredPartyT ON LineItemT.PatientID = InsuredPartyT.PatientID 
	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID AND InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID 
	LEFT JOIN (SELECT RespID, 
		Sum(CASE WHEN AppliesT.Amount Is Null OR LineItemPaysT.Type <> 1 THEN 0 ELSE AppliesT.Amount END) AS PayAmount, 
		Sum(CASE WHEN AppliesT.Amount Is Null OR LineItemPaysT.Type <> 2 THEN 0 ELSE AppliesT.Amount END) AS AdjAmount 
		FROM AppliesT 
		INNER JOIN LineItemT LineItemPaysT ON AppliesT.SourceID = LineItemPaysT.ID 
		INNER JOIN PaymentsT ON LineItemPaysT.ID = PaymentsT.ID 
		WHERE LineItemPaysT.Deleted = 0 AND (LineItemPaysT.Type = 1 OR LineItemPaysT.Type = 2) 
		GROUP BY AppliesT.RespID 
	) AS AppliesQ ON ChargeRespT.ID = AppliesQ.RespID 
	-- PatientRespQ gets the patient resp for this charge, there
	-- really should never be multiple records for one charge
	LEFT JOIN (SELECT ChargeID, Sum(Amount) AS TotalAmount 
		FROM ChargeRespT 
		WHERE InsuredPartyID Is Null 	
		GROUP BY ChargeRespT.ChargeID 
	) AS PatientRespQ ON ChargesT.ID = PatientRespQ.ChargeID 
	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 
) AS AdditionalLineItemsQ 
-- this join matches the applies by applied-to-billID, insured party ID, and ChargeRespID,
-- then matches the other charges by bill ID, insured party ID, a null ChargeRespID, and enforces
-- that it is not the same charge that we are reporting as underpaid
ON SubQ.BillID = AdditionalLineItemsQ.BillID AND SubQ.InsuredPartyID = AdditionalLineItemsQ.InsuredPartyID 
AND SubQ.ChargeID <> AdditionalLineItemsQ.ID 
AND (AdditionalLineItemsQ.RespID Is Null OR SubQ.ChargeRespID = AdditionalLineItemsQ.RespID) 
)", sqlChargesWithPaymentsUnderAllowedAmountAsSubQ);

	if (bUseTempTableOptimization) {
		// Create and populate the temp table
		ADODB::_ConnectionPtr pCon = CReportInfo::IsServerSide(366, true) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot();
		NxAdo::PushMaxRecordsWarningLimit pmr(1000000); // Debug only: We expect well over 100 records to be inserted into the temp table. We raise the warning cap to 1M.
														// Ten minute timeout by default
		long nCommandTimeout = GetRemotePropertyInt("SupportTempPaymentsUnderAllowedAmountReportTimeout", 600, 0, "<None>", false);
		CIncreaseCommandTimeout cict(pCon, nCommandTimeout);
		// Show a message box for the temp table creation, because it is tightly coupled with the report query 
		// itself and that already is shown in a message box.
#ifdef _DEBUG
		CMsgBox dlg(NULL);
		dlg.msg = sqlTempTableCreationBatch.Flatten();
		dlg.m_bWordWrap = false;
		dlg.DoModal();
#endif

		ExecuteSql(pCon, "%s", sqlTempTableCreationBatch.Flatten());
	}
	else
	{
		// No need for prep work. Its just one query.
	}
	return sqlOuterReportQuery.Flatten();
}
