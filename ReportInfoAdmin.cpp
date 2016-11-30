
////////////////
// DRT 8/6/03 - GetSqlAdministration() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "TemplateLineItemInfo.h"
#include "DateTimeUtils.h"
#include "EmrUtils.h"
// (f.dinatale 2010-12-13) - PLID 41275 - Added for NxReminder Usage reporting
#include "NxReminderSOAPUtils.h"
#include "GlobalSchedUtils.h"
#include "TemplateItemEntryGraphicalDlg.h"

CString CReportInfo::GetSqlAdministration(long nSubLevel, long nSubRepNum) const
{
	switch (nID) {

case 1: //Surgery List w/Prices
case 398:  //Surgery List	
	{
		// (j.gruber 2009-03-25 14:58) - PLID 33691 - updated discount structure
		// (j.jones 2010-10-08 11:03) - PLID 36781 - added package information
		
		CString strSql;
		strSql = _T(
			"SELECT SurgeriesT.Name AS OperationName, "
			"SurgeryDetailsT.PayToPractice, "
			"Round(Convert(money, (Amount * Quantity * "
			"	(CASE WHEN((SELECT Sum(PercentOff) FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = SurgeryDetailsT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = SurgeryDetailsT.ID)))/100) END)) "
			"	- (CASE WHEN (SELECT Sum(Discount) FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = SurgeryDetailsT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = SurgeryDetailsT.ID) END)),2) AS SalePrice, "
			"ServiceT.Name AS Description, "
			"CASE WHEN ProductT.ID IS NULL THEN Code + ' ' + SubCode END AS CodeNumber, "
			"CASE WHEN SurgeriesT.IsPackage = 0 THEN 0 ELSE 1 END AS IsAPackage, "
			"CASE WHEN SurgeriesT.IsPackage = 1 THEN SurgeriesT.PackageType ELSE -1 END AS PackageTypeID, "
			"CASE WHEN SurgeriesT.IsPackage = 1 AND SurgeriesT.PackageType = 1 THEN 'Repeatable' "
			"	WHEN SurgeriesT.IsPackage = 1 AND SurgeriesT.PackageType = 2 THEN 'Multi-Use' "
			"	ELSE '' END AS PackageTypeName, "
			"SurgeriesT.PackageTotalAmount, SurgeriesT.PackageTotalCount, "
			"SurgeryDetailsT.Quantity "
			"FROM SurgeriesT "
			"LEFT JOIN SurgeryDetailsT ON SurgeriesT.ID = SurgeryDetailsT.SurgeryID "
			"LEFT JOIN ServiceT ON SurgeryDetailsT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"WHERE SurgeryDetailsT.ServiceID Is Not Null ");

		return strSql;
	}
	break;


	case 430:
			//Deleted Prescriptions
			/*	Version History
				JMM 8/4/03 - Created.
				(c.haag 2007-02-02 17:01) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation and PillsPerBottle to Quantity, but
					kept them aliased to the old names to avoid messing up any custom reports out there.
				TES 3/5/2009 - PLID 33068 - Added SureScripts fields
				(d.thompson 2009-04-02) - PLID 33571 - added strength unit
				TES 4/2/2009 - PLID 33750 - Strength and DosageForm now pull from DrugList
				TES 5/11/2009 - PLID 28519 - Added SampleExpirationDate
			*/
			return _T("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"LEFT(EMRDataT.Data,255) AS MedName, PatientMedications.PatientExplanation AS Description, PatientMedications.RefillsAllowed,  "
				"PatientMedications.Quantity AS PillsPerBottle, PatientMedications.Unit, PersonT.FirstContactDate as Date, PatientMedications.LocationID AS LocID,  "
				"PatientMedications.ProviderID AS ProvID, LocationsT.Name AS LocName, "
				"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, PatientMedications.PrescriptionDate AS PDate, "
				"PatientMedications.MedicationID AS MedID, PatientMedications.DeletedBy, PatientMedications.DeletedDate, "
				"PatientMedications.DaysSupply, PatientMedications.NoteToPharmacist, PatientMedications.AllowSubstitutions, "
				"PatientMedications.PriorAuthorization, PatientMedications.PriorAuthorizationIsSample, DrugList.Strength, "
				"DrugDosageFormsT.Name AS DosageForm, dbo.GetPrescriptionDiagList(PatientMedications.ID) AS DiagnosisCodeList, "
				"StrengthUnitT.Name AS StrengthUnit, PatientMedications.SampleExpirationDate "
				"FROM PatientMedications "
				"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"INNER JOIN PersonT ON PatientMedications.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN DrugDosageFormsT ON DrugList.DosageFormID = DrugDosageFormsT.ID "
				"LEFT JOIN DrugStrengthUnitsT AS StrengthUnitT ON DrugList.StrengthUnitID = StrengthUnitT.ID "
				"LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
				"LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				" WHERE PatientMedications.Deleted = 1");
			break;


case 11:
		//Service Codes
		/* Version History
			e.lally 2007-01-03 - PLID 24075 - Added fields: TypeOfService, RVU, GlobalPeriod, UseTax1, UseTax2, 
				ShopFee, IsAnesthesiaCode, IsFacilityFee, UseAnesthesiaBilling, UseFacilityBilling, 
				AnesthBaseUnits, PromptForCoPay, IsAMA. Also added case statements for data that depends on flags.
				// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
				// (j.gruber 2012-10-31 13:22) - PLID 53242 - added subreport for locations by shopfee
		*/
	switch (nSubLevel) {
		case 1:
		return _T("SELECT LocationsT.ID as LocID, LocationsT.Name as LocName, ShopFee, ServiceID as ServiceID, ServiceT.Active as Active "
			" FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "
			" INNER JOIN ServiceT ON ServiceLocationInfoT.ServiceID = ServiceT.ID "
			" WHERE LocationsT.Managed = 1" );

		default:
			return _T("SELECT CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name AS Description, CategoriesT.Name, "
			"CASE WHEN ServiceT.UseAnesthesiaBilling = 1 OR ServiceT.UseFacilityBilling = 1 THEN NULL ELSE ServiceT.Price END AS Price, "
			"Active as Active, ServiceT.ID AS ServiceID, CPTCodeT.TypeOfService, "
			"CPTCodeT.RVU, CPTCodeT.GlobalPeriod, ServiceT.Taxable1 as UseTax1, ServiceT.Taxable2 as UseTax2, "
			"Convert(money, 0) as ShopFee, ServiceT.Anesthesia AS IsAnesthesiaCode, ServiceT.FacilityFee as IsFacilityFee, "
			"ServiceT.UseAnesthesiaBilling, ServiceT.UseFacilityBilling, "
			"CASE WHEN ServiceT.Anesthesia = 0 THEN NULL ELSE CPTCodeT.AnesthBaseUnits END AS AnesthBaseUnits, "
			"CPTCodeT.IsAMA "
			"FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID ");
		break;	
	break;
	}
	break;

case 289:
		//Services by Category
		return _T("SELECT ServiceT.Name AS ServiceName, ServiceT.Price, CPTCodeT.Code, CPTCodeT.SubCode, CategoriesT.Name AS Category, CategoriesT.ID AS CatID "
		"FROM ServiceT "
		"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"ORDER BY Category, Code, Servicename");
		break;

case 57:
		//Insurance Companies
		/* Version History 
			TES 5/21/2004 - PLID 12521 - Took out PlanType, it doesn't make sense and was messing up the report.
			// (m.cable 2004-06-04 10:11) - PLID 11988 - I added the plan types back.  
			// Instead of listing an insurance company multiple times for each insurance company, 
			// it will list the plans for each company.
			// (f.dinatale 2010-09-07) - PLID 38340 - Modified to add the Archived column to enable extended filtering.
			// (j.jones 2012-10-19 10:50) - PLID 51551 - stopped sending just a comma if the insurance contact name is blank,
			// also removed middle name because contacts don't have that field
			// (r.goldschmidt 2014-03-31 17:41) - PLID 39699 - Added Payer IDs as available fields
			// (r.farnworth 2015-03-30 17:51) - PLID 65162 - Added ConversionID
		*/
		return _T("SELECT InsuranceCoT.Name, "
			" CASE WHEN Coalesce(PersonContactT.Last, '') = '' AND Coalesce(PersonContactT.First, '') = '' THEN '' "
			" ELSE PersonContactT.Last + CASE WHEN Coalesce(PersonContactT.Last, '') <> '' AND Coalesce(PersonContactT.First, '') <> '' THEN ', ' ELSE '' END + PersonContactT.First END AS Contact, "
			" PersonT.WorkPhone, PersonT.Extension, PersonT.Address1,  "
			" PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Note,  "
			" PersonContactT.WorkPhone AS ContactPhone, PersonContactT.Extension AS ContactExt, PersonContactT.Fax AS ContactFax, "
			" dbo.GetPlanTypesString(InsuranceCoT.PersonID) AS PlanType, "
			" UBIDs.EBillingID AS UBPayerID, HCFAIDs.EBillingID AS HCFAPayerID, EligIDs.EBillingID AS EligibilityPayerID, "
			" PersonT.Archived AS Archived, "
			" InsuranceCoT.ConversionID "
			" FROM InsuranceCoT LEFT OUTER JOIN  "
			" PersonT ON InsuranceCoT.PersonID = PersonT.ID  "
			" LEFT JOIN InsuranceContactsT ON InsuranceCoT.PersonID = InsuranceContactsT.InsuranceCoID  "
			" LEFT JOIN PersonT PersonContactT ON InsuranceContactsT.PersonID = PersonContactT.ID  "
			" LEFT JOIN EbillingInsCoIDs AS UBIDs ON UBIDs.ID = InsuranceCoT.UBPayerID  "
			" LEFT JOIN EbillingInsCoIDs AS HCFAIDs ON HCFAIDs.ID = InsuranceCoT.HCFAPayerID  "
			" LEFT JOIN EbillingInsCoIDs AS EligIDs ON EligIDs.ID = InsuranceCoT.EligPayerID  "
			" "); 
		break;
	

case 86: 
		//Mail Merge Groups
	// (b.eyers 2015-04-14) - PLID 13844 - Add company name and phone numbers
		return _T("SELECT GroupsT.ID AS GroupID, GroupsT.Name AS GroupName,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS GroupItemName, "
		"    PersonT.ID AS GroupItemID, PersonT.Location as LocID, "
		"    PersonT.HomePhone, PersonT.WorkPhone, PersonT.Company "
		"FROM PersonT RIGHT OUTER JOIN "
		"    GroupDetailsT ON  "
		"    PersonT.ID = GroupDetailsT.PersonID RIGHT OUTER JOIN "
		"    GroupsT ON GroupDetailsT.GroupID = GroupsT.ID");
		break;
	

case 87:
		//Provider Multiple Fee Schedules
		/*	Version History
			DRT 2/2/2004 - PLID 9733 - Made report editable.  Added extended filter for fee group.  Added filter on location.
				Added filter on provider.  Added external filter on Insurance Company.  Added Allowable field to the query and
				report.
			// (j.jones 2009-10-23 09:01) - PLID 18558 - exposed UsePOS for proper labeling, and filtered locations accordingly
			// (j.jones 2013-04-15 13:28) - PLID 12136 - supported products in fee schedules
		*/
		switch(nSubLevel){
		case 0:
			return _T("SELECT Name, ID AS ID, Note "
			"FROM MultiFeeGroupsT");
			break;
		case 1:
			switch(nSubRepNum){
			case 0:
				return _T("SELECT MultiFeeProvidersT.FeeGroupID,  "
					"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName "
					"FROM MultiFeeProvidersT INNER JOIN "
					"    PersonT ON MultiFeeProvidersT.ProviderID = PersonT.ID");
				break;
			case 1:
				return _T("SELECT MultiFeeInsuranceT.FeeGroupID,  "
					"    InsuranceCoT.Name "
					"FROM MultiFeeInsuranceT INNER JOIN "
					"    InsuranceCoT ON  "
					"    MultiFeeInsuranceT.InsuranceCoID = InsuranceCoT.PersonID");
				break;
			case 2:
				return _T("SELECT MultiFeeLocationsT.FeeGroupID, LocationsT.Name, MultiFeeGroupsT.UsePOS "
					"FROM MultiFeeLocationsT "
					"INNER JOIN MultiFeeGroupsT ON MultiFeeLocationsT.FeeGroupID = MultiFeeGroupsT.ID "
					"INNER JOIN LocationsT ON MultiFeeLocationsT.LocationID = LocationsT.ID "
					"WHERE LocationsT.Active = 1 AND TypeID = 1 AND (UsePOS = 1 OR Managed = 1)");
				break;
			case 3:
				return _T("SELECT MultiFeeItemsT.FeeGroupID, "
					"CASE WHEN ProductT.ID Is Not Null THEN ProductT.InsCode ELSE CPTCodeT.Code END AS Code, "
					"CASE WHEN ProductT.ID Is Not Null THEN '' ELSE CPTCodeT.SubCode END AS SubCode, ServiceT.Name, "
					"ServiceT.Price AS OldPrice, "
					"MultiFeeItemsT.Price AS NewPrice, Allowable "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null)");
				break;
			default:
				return _T("");
				break;
			}
			break;
		default:
			return _T("");
			break;
		}
		break;
	

case 165:
		//Appointment Types
		return _T("SELECT ID, Name FROM AptTypeT");
		break;
	case 166:
		//Appointment Purposes
		return _T("SELECT AptPurposeT.ID, AptPurposeT.Name AS Purpose, AptTypeT.Name "
		"FROM (AptPurposeT LEFT JOIN AptPurposeTypeT ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID) LEFT JOIN AptTypeT ON AptPurposeTypeT.AptTypeID = AptTypeT.ID "
		"");
		break;
	
	case 279:
		//Appointment Types by Purpose
		return _T("SELECT AptTypeT.Name AS Type, AptPurposeT.Name AS Purpose "
			"FROM AptTypeT LEFT JOIN AptPurposeTypeT ON AptTypeT.ID = AptPurposeTypeT.AptTypeID LEFT JOIN AptPurposeT ON AptPurposeTypeT.AptPurposeID = AptPurposeT.ID");
		break;

	case 296:
		//Patients with Suppressed Statements
		/* Version History
			- JMM 9/2/03 - Changed enitre report radically, deleted past query
		*/
		return _T ("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,    "
			"			PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"			GroupTypes.GroupName,    "
			"			ProvidersT.PersonID AS ProvID,    "
			"			PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,    "
			"			PersonT.FirstContactDate AS Date,    "
			"			GroupTypes.TypeIndex AS TypeIndex,    "
			"			[_PatBalQ].PatBalance AS PatBalance,    "
			"			[_PatBalQ].InsBalance AS InsBalance, "
			"			[_PatBalbyTypeSubQ].MaxOfDate,    "
			"			LineItemT.Amount,    "
			"			[_PatBalbyTypeSubQ].PayID,   "
			"			PersonT.Location AS LocID,   "
			"			LocationsT.Name AS Location, "
			"			(SELECT Name FROM InsuranceCoT WHERE InsuranceCoT.PersonID = (SELECT InsuranceCoId FROM InsuredPartyT WHERE PatientID = PatientsT.PersonID AND RespTypeID = 1)) AS PriInsCo,  "
			"			(SELECT Name FROM InsuranceCoT WHERE InsuranceCoT.PersonID = (SELECT InsuranceCoId FROM InsuredPartyT WHERE PatientID = PatientsT.PersonID AND RespTypeID = 2)) AS SecInsCo "
			"			FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"			Payments = CASE   "
			"				WHEN [_PatPaysQ].[SumOfPatAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPaysQ].[SumOfPatAmount]   "
			"				End,    "
			"			Charges = CASE   "
			"				WHEN [PatChargeAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [PatChargeAmount]   "
			"				End,    "
			"			PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,    "
			"			PatientsT.PersonID AS ID,    "
			"			(CASE   "
			"				WHEN [PatChargeAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [PatChargeAmount]   "
			"				End)-   "
			"			(CASE   "
			"				WHEN [_PatPaysQ].[SumOfPatAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPaysQ].[SumOfPatAmount]   "
			"				End)+   "
			"			(CASE   "
			"				WHEN [_PatPrePaysQ].[SumOfPatAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPrePaysQ].[SumOfPatAmount]   "
			"				End) AS PatBalance,    "
			"			ProvidersT.PersonID AS ProvID,    "
			"			PatPrePayments = CASE   "
			"				WHEN [_PatPrePaysQ].[SumOfPatAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPrePaysQ].[SumOfPatAmount]   "
			"				End, "
			"			(CASE   "
			"				WHEN [InsChargeAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [InsChargeAmount]   "
			"				End)-   "
			"			(CASE   "
			"				WHEN [_PatPaysQ].[SumOfInsAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPaysQ].[SumOfInsAmount]   "
			"				End)+   "
			"			(CASE   "
			"				WHEN [_PatPrePaysQ].[SumOfInsAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPrePaysQ].[SumOfInsAmount]   "
			"				End) AS InsBalance,    "
			"			InsPrePayments = CASE   "
			"				WHEN [_PatPrePaysQ].[SumOfInsAmount] Is Null   "
			"				THEN 0   "
			"				ELSE [_PatPrePaysQ].[SumOfInsAmount]   "
			"				End, "
			"			PatientsT.SuppressStatement  "
			"			FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"			Sum(CASE WHEN PaymentsT.InsuredPartyID = -1 THEN LineItemT.Amount ELSE 0 END) AS SumOfPatAmount,    "
			"			Sum(CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN LineItemT.Amount ELSE 0 END) AS SumOfInsAmount,    "
			"			PatientsT.PersonID AS ID   "
			"			FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID   "
			"			WHERE (((LineItemT.Deleted)=0) AND (PatientsT.SuppressStatement = 1) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))   "
			"			GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID   "
			"			) AS _PatPaysQ ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN (SELECT LineItemT.PatientID,   "
			"			Sum(CASE WHEN ChargeRespT.InsuredPartyID IS NULL THEN ChargeRespT.Amount ELSE 0 END) AS PatChargeAmount,    "
			"			Sum(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL THEN ChargeRespT.Amount ELSE 0 END) AS InsChargeAmount, "
			"			PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName   "
			"			FROM ((LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID) LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID   "
			"			WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))   "
			"			GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle   "
			"			) AS _PatChargesQ ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN    "
			"			/* Prepays */  "
			"			(SELECT FullName, Sum(PatAmount) AS SumOfPatAmount, "
			"			Sum(InsAmount) AS SumOfInsAmount, ID   "
			"			FROM  "
			"			(   "
			"			SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"			(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
			"			    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN LineItemT.Amount ELSE 0 END) ELSE ((CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN LineItemT.Amount ELSE 0 END) - PrepayAppliesQ.InsAmount) END)   "
			"			ELSE   "
			"			    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN LineItemT.Amount ELSE 0 END)-PrepayAppliedToQ.PatAmount ELSE (CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN LineItemT.Amount ELSE 0 END) - PrepayAppliesQ.PatAmount-PrepayAppliedToQ.PatAmount END) END) AS PatAmount,    "
			"			(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
			"			    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (CASE WHEN PaymentsT.InsuredPartyID = -1 THEN LineItemT.Amount ELSE 0 END) ELSE ((CASE WHEN PaymentsT.InsuredPartyID = -1 THEN LineItemT.Amount ELSE 0 END) - PrepayAppliesQ.InsAmount) END)   "
			"			ELSE   "
			"			    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (CASE WHEN PaymentsT.InsuredPartyID = -1 THEN LineItemT.Amount ELSE 0 END)-PrepayAppliedToQ.InsAmount ELSE (CASE WHEN PaymentsT.InsuredPartyID = -1 THEN LineItemT.Amount ELSE 0 END) - PrepayAppliesQ.InsAmount-PrepayAppliedToQ.InsAmount END) END) AS InsAmount,    "
			"			PatientsT.PersonID AS ID   "
			"			FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)   "
			"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID))   "
			"			LEFT JOIN   "
			"			/* This will total everything applied to a prepayment */   "
			"			( SELECT SUM(CASE WHEN PaymentsT.InsuredPartyID = -1 THEN (AppliesT.Amount * -1) ELSE 0 END) AS PatAmount,  "
			"			SUM(CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN (AppliesT.Amount * -1) ELSE 0 END) AS InsAmount, "
			"			AppliesT.DestID AS ID   "
			"			FROM   "
			"			LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"			LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID   "
			"			LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID   "
			"			WHERE (LineItemT.Deleted = 0)   "
			"			GROUP BY AppliesT.DestID   "
			"			) PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID   "
			"			LEFT JOIN   "
			"			  "
			"			/* This will total everything that the prepayment is applied to */   "
			"			( SELECT SUM(CASE WHEN PaymentsT.InsuredPartyID = -1 THEN (AppliesT.Amount) ELSE 0 END) AS PatAmount,  "
			"			SUM(CASE WHEN PaymentsT.InsuredPartyID <> -1 THEN (AppliesT.Amount) ELSE 0 END) AS InsAmount, AppliesT.SourceID AS ID   "
			"			FROM   "
			"			LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"			LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID   "
			"			LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID   "
			"			WHERE LineItemT.Deleted = 0   "
			"			GROUP BY AppliesT.SourceID   "
			"			) PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID   "
			"			  "
			"			WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1) AND (PersonT.ID > 0) AND (PatientsT.SuppressStatement = 1)  "
			"			AND (((LineItemT.Type)<4))   "
			"			) AS PrepaySubQ  "
			"			GROUP BY FullName, ID)  "
			"			 AS _PatPrePaysQ  "
			"			/* End Prepays */  "
			"			ON PatientsT.PersonID = [_PatPrePaysQ].ID   "
			"			) AS _PatBalQ ON PatientsT.PersonID = [_PatBalQ].ID) LEFT JOIN ((SELECT PatientsT.PersonID AS PatID,   "
			"			PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"			[_PatBalbyTypeSubQ2].MaxOfDate,    "
			"			Max(LineItemT.ID) AS PayID   "
			"			FROM ((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) LEFT JOIN (SELECT PatientsT.PersonID AS PatID,    "
			"			Max(LineItemT.Date) AS MaxOfDate   "
			"			FROM PatientsT LEFT JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID   "
			"			WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))   "
			"			GROUP BY PatientsT.PersonID   "
			"			HAVING (((PatientsT.PersonID)>0))   "
			"			) AS _PatBalbyTypeSubQ2 ON PatientsT.PersonID = [_PatBalbyTypeSubQ2].PatID) LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ2].PatID = LineItemT.PatientID   "
			"			WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))   "
			"			GROUP BY PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, [_PatBalbyTypeSubQ2].MaxOfDate   "
			"			HAVING (((PatientsT.PersonID)>0))   "
			"			) AS _PatBalbyTypeSubQ LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ].PayID = LineItemT.ID) ON PatientsT.PersonID = [_PatBalbyTypeSubQ].PatID  "
			"			GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, GroupTypes.GroupName, ProvidersT.PersonID, PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle, PersonT.FirstContactDate, GroupTypes.TypeIndex,[_PatBalQ].PatBalance,  [_PatBalQ].InsBalance, [_PatBalbyTypeSubQ].MaxOfDate, LineItemT.Amount, [_PatBalbyTypeSubQ].PayID, PersonT.Location, LocationsT.Name, PatientsT.SuppressStatement  "
			"			HAVING (((PatientsT.UserDefinedID)>0) AND (PatientsT.SuppressStatement = 1))");

		break;

	case 327:
		//Cumulative Visual Acuity
		/* Version History
			m.hancock PLID 16756, 16757 - I changed the query for the report to match the new structure for refractive visits.
		*/
		return _T("SELECT "
			"Sum(SubMainQ.MaxID) AS CumulativeSum, SubMainQ.VA AS VA, SubMainQ.VisitType AS VisitType, SubMainQ.LocationID AS LocID, SubMainQ.ProviderID AS ProvID, "
			"SubMainQ.ProcedureDate AS Date, SubMainQ.PatientID AS PatID, SubMainQ.TypeName "
			"FROM "
			"( "
			"SELECT "
			"MAX(SubQ.CountID) AS MaxID, SubQ.VA AS SubVA, EyeTestsT.VA, SubQ.VisitType, SubQ.LocationID, SubQ.ProviderID, SubQ.ProcedureDate, SubQ.PatientID, SubQ.TypeName "
			"FROM "
			"EyeVisitsT INNER JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID "
			"LEFT JOIN "
			"( "
			"SELECT "
			"Count(EyeVisitsT.ID) AS CountID, EyeTestsT.VA, VisitType, EyeProceduresT.LocationID, EyeProceduresT.ProviderID, EyeProceduresT.ProcedureDate, EyeProceduresT.PatientID, "
			"EyeVisitTypesT.Type AS TypeName "
			"FROM "
			"EyeVisitsT INNER JOIN EyeProceduresT ON EyeVisitsT.EyeProcedureID = EyeProceduresT.ID INNER JOIN EyeVisitTypesT ON EyeVisitsT.VisitType = EyeVisitTypesT.ID INNER JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID "
			"WHERE EyeTestsT.VA is not null "
			"GROUP BY EyeTestsT.VA, VisitType, LocationID, ProviderID, ProcedureDate, PatientID, EyeVisitTypesT.Type "
			") SubQ ON EyeTestsT.VA >= SubQ.VA "
			"WHERE EyeTestsT.VA Is Not Null "
			"GROUP BY EyeTestsT.VA, SubQ.VA, SubQ.VisitType, SubQ.LocationID, SubQ.ProviderID, SubQ.ProcedureDate, SubQ.PatientID, SubQ.TypeName "
			") SubMainQ "
			"GROUP BY SubMainQ.VA, SubMainQ.VisitType, SubMainQ.LocationID, SubMainQ.ProviderID, SubMainQ.ProcedureDate, SubMainQ.PatientID, SubMainQ.TypeName");
		break;

	case 328:
		//Refractive Outcomes
		/* Version History
			m.hancock PLID 16757 8/29/05 - Filter refractive visits report using test types
				I changed the query because the structure of the refractive visits changed due to the addition of test types.
		*/
		{
		CString strSql;
		strSql = "SELECT "
			"/* Total Eyes */ "
			"Count(CASE WHEN Sphere < 1.0 THEN VA ELSE NULL END) AS [0to1-T], "
			"Count(CASE WHEN Sphere >= 1.0 AND Sphere < 2.0 THEN VA ELSE NULL END) AS [1to2-T], "
			"Count(CASE WHEN Sphere >= 2.0 AND Sphere < 3.0 THEN VA ELSE NULL END) AS [2to3-T], "
			"Count(CASE WHEN Sphere >= 3.0 AND Sphere < 4.0 THEN VA ELSE NULL END) AS [3to4-T], "
			"Count(CASE WHEN Sphere >= 4.0 AND Sphere < 5.0 THEN VA ELSE NULL END) AS [4to5-T], "
			"Count(CASE WHEN Sphere >= 5.0 AND Sphere < 6.0 THEN VA ELSE NULL END) AS [5to6-T], "
			"Count(CASE WHEN Sphere >= 6.0 AND Sphere < 7.0 THEN VA ELSE NULL END) AS [6to7-T], "
			"Count(CASE WHEN Sphere >= 7.0 AND Sphere < 8.0 THEN VA ELSE NULL END) AS [7to8-T], "
			"Count(CASE WHEN Sphere >= 8.0 AND Sphere < 9.0 THEN VA ELSE NULL END) AS [8to9-T], "
			"Count(CASE WHEN Sphere >= 9.0 AND Sphere < 10.0 THEN VA ELSE NULL END) AS [9to10-T], "
			"Count(CASE WHEN Sphere >= 10.0 THEN VA ELSE NULL END) AS [10+-T], "
			" "
			"/* 20's */ "
			"Count(CASE WHEN Sphere < 1.0 AND VA <= 20 THEN VA ELSE NULL END) AS [0to1-20], "
			"Count(CASE WHEN Sphere >= 1.0 AND Sphere < 2.0 AND VA <= 20 THEN VA ELSE NULL END) AS [1to2-20], "
			"Count(CASE WHEN Sphere >= 2.0 AND Sphere < 3.0 AND VA <= 20 THEN VA ELSE NULL END) AS [2to3-20], "
			"Count(CASE WHEN Sphere >= 3.0 AND Sphere < 4.0 AND VA <= 20 THEN VA ELSE NULL END) AS [3to4-20], "
			"Count(CASE WHEN Sphere >= 4.0 AND Sphere < 5.0 AND VA <= 20 THEN VA ELSE NULL END) AS [4to5-20], "
			"Count(CASE WHEN Sphere >= 5.0 AND Sphere < 6.0 AND VA <= 20 THEN VA ELSE NULL END) AS [5to6-20], "
			"Count(CASE WHEN Sphere >= 6.0 AND Sphere < 7.0 AND VA <= 20 THEN VA ELSE NULL END) AS [6to7-20], "
			"Count(CASE WHEN Sphere >= 7.0 AND Sphere < 8.0 AND VA <= 20 THEN VA ELSE NULL END) AS [7to8-20], "
			"Count(CASE WHEN Sphere >= 8.0 AND Sphere < 9.0 AND VA <= 20 THEN VA ELSE NULL END) AS [8to9-20], "
			"Count(CASE WHEN Sphere >= 9.0 AND Sphere < 10.0 AND VA <= 20 THEN VA ELSE NULL END) AS [9to10-20], "
			"Count(CASE WHEN Sphere >= 10.0 AND VA <= 20 THEN VA ELSE NULL END) AS [10+-20], "
			" "
			"/* 40's */ "
			"Count(CASE WHEN Sphere < 1.0 AND VA <= 40 THEN VA ELSE NULL END) AS [0to1-40], "
			"Count(CASE WHEN Sphere >= 1.0 AND Sphere < 2.0 AND VA <= 40 THEN VA ELSE NULL END) AS [1to2-40], "
			"Count(CASE WHEN Sphere >= 2.0 AND Sphere < 3.0 AND VA <= 40 THEN VA ELSE NULL END) AS [2to3-40], "
			"Count(CASE WHEN Sphere >= 3.0 AND Sphere < 4.0 AND VA <= 40 THEN VA ELSE NULL END) AS [3to4-40], "
			"Count(CASE WHEN Sphere >= 4.0 AND Sphere < 5.0 AND VA <= 40 THEN VA ELSE NULL END) AS [4to5-40], "
			"Count(CASE WHEN Sphere >= 5.0 AND Sphere < 6.0 AND VA <= 40 THEN VA ELSE NULL END) AS [5to6-40], "
			"Count(CASE WHEN Sphere >= 6.0 AND Sphere < 7.0 AND VA <= 40 THEN VA ELSE NULL END) AS [6to7-40], "
			"Count(CASE WHEN Sphere >= 7.0 AND Sphere < 8.0 AND VA <= 40 THEN VA ELSE NULL END) AS [7to8-40], "
			"Count(CASE WHEN Sphere >= 8.0 AND Sphere < 9.0 AND VA <= 40 THEN VA ELSE NULL END) AS [8to9-40], "
			"Count(CASE WHEN Sphere >= 9.0 AND Sphere < 10.0 AND VA <= 40 THEN VA ELSE NULL END) AS [9to10-40], "
			"Count(CASE WHEN Sphere >= 10.0 AND VA <= 40 THEN VA ELSE NULL END) AS [10+-40], "
			" "
			"MainQ.ProvName, MainQ.ProvID AS ProvID, MainQ.LocName, MainQ.LocID AS LocID, MainQ.TestName, MainQ.TestID AS TestID "
			"FROM "
			"( "
			"SELECT "
			"PreOpQ.EyeProcedureID, PreOpQ.EyeType, CASE WHEN PreOpQ.Sphere < 0 THEN -1 * PreOpQ.Sphere ELSE PreOpQ.Sphere END AS Sphere, "
			"LastVisitQ.MaxType, LastVisitQ.Type, LastVisitQ.EyeProcedureID AS LastProcID, LastVisitQ.EyeType AS LastEyeType, LastVisitQ.VA, PreOpQ.ProvName, "
			"PreOpQ.ProvID, PreOpQ.LocID, PreOpQ.LocName, PreOpQ.TestName, PreOpQ.TestID "
			"FROM "
			"( "
			"SELECT "
			"EyeVisitsT.ID, EyeVisitsT.EyeProcedureID, EyeTestsT.Sphere, EyeTestsT.VA, EyeTestsT.BCVA, EyeVisitsT.Date AS VisitDate, EyeProceduresT.ProcedureDate, "
			"EyeProceduresT.PatientID, EyeProceduresT.LocationID, EyeTestsT.EyeType, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Provname, "
			"PersonT.ID AS ProvID, LocationsT.ID AS LocID, CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS LocName, EyeTestsT.TestID, EyeTestTypesT.TestName "
			"FROM "
			"EyeProceduresT LEFT JOIN EyeVisitsT ON EyeProceduresT.ID = EyeVisitsT.EyeProcedureID LEFT JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID LEFT JOIN PersonT ON EyeProceduresT.ProviderID = PersonT.ID "
			"LEFT JOIN LocationsT ON EyeProceduresT.LocationID = LocationsT.ID LEFT JOIN EyeTestTypesT ON EyeTestsT.TestID = EyeTestTypesT.TestID "
			"WHERE VisitType = 1 /* 1 is hardcoded PreOp type */ @ReportDateFrom @ReportDateTo /* @TestType */ "
			") PreOpQ "
			"INNER JOIN "
			"( "
			"SELECT "
			"Max(EyeVisitsT.VisitType) AS MaxType, EyeVisitTypesT.Type, EyeVisitsT.ID AS EyeVisitID, EyeTestsT.VA, EyeTestsT.BCVA, EyeTestsT.Sphere, "
			"EyeProceduresT.PatientID, EyeProceduresT.LocationID, EyeProceduresT.ProcedureDate, EyeVisitsT.EyeProcedureID, EyeTestsT.EyeType, EyeTestsT.TestID "
			"FROM "
			"EyeProceduresT LEFT JOIN EyeVisitsT ON EyeProceduresT.ID = EyeVisitsT.EyeProcedureID LEFT JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID "
			"INNER JOIN EyeVisitTypesT ON EyeVisitsT.VisitType = EyeVisitTypesT.ID "
			"WHERE EyeVisitsT.VisitType > 1 /* Filter out people who have only had a PreOp */ AND EyeTestsT.VA Is Not Null "
			"GROUP BY EyeVisitTypesT.Type, EyeVisitsT.ID, EyeTestsT.VA, EyeTestsT.BCVA, EyeTestsT.Sphere, "
			"EyeProceduresT.PatientID, EyeProceduresT.LocationID, EyeProceduresT.ProcedureDate, EyeVisitsT.EyeProcedureID, EyeTestsT.EyeType, EyeTestsT.TestID "
			") LastVisitQ ON PreOpQ.EyeProcedureID = LastVisitQ.EyeProcedureID AND PreOpQ.EyeType = LastVisitQ.EyeType AND PreOpQ.TestID = LastVisitQ.TestID "
			") MainQ "
			" "
			"GROUP BY MainQ.ProvName, MainQ.ProvID, MainQ.LocName, MainQ.LocID, MainQ.TestName, MainQ.TestID";

		if(nDateRange > 0) {		//we have a date range filter on			
			CString temp;
			COleDateTimeSpan dtSpan;
			COleDateTime dt;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dt = DateTo;
			dt += dtSpan;
			temp = " AND EyeProceduresT.ProcedureDate < " + dt.Format("'%m/%d/%Y'");
			strSql.Replace("@ReportDateTo", temp);
			temp = " AND EyeProceduresT.ProcedureDate >= " + DateFrom.Format("'%m/%d/%Y'");
			strSql.Replace("@ReportDateFrom", temp);
		}
		else {
			strSql.Replace("@ReportDateFrom", "");
			strSql.Replace("@ReportDateTo", "");
		}

		return _T(strSql);
		}
		break;

	case 329:
		//ICD-9 Codes

		// (j.jones 2011-06-24 17:29) - PLID 29089 - added Active as a field, for filtering purposes
		// (b.spivey, March 31st, 2014) - PLID 61593 - Add ICD10 field. 
		return _T("SELECT CodeNumber, CodeDesc, Active AS Active, ICD10 FROM DiagCodes");
		break;

	case 361:
		//Time Log
		// (j.gruber 2008-06-25 15:40) - PLID 26137 - adding location to the time log report
		return _T("SELECT UserTimesT.UserID AS UserID, UsersT.UserName, UserTimesT.CheckIn AS Date, UserTimesT.CheckOut, "
			"CheckOut - CheckIn AS Diff, LocationID as LocID, LocationsT.Name as LocationName "
			"FROM UserTimesT "
			"INNER JOIN UsersT ON UserTimesT.UserID = UsersT.PersonID AND UsersT.PersonID > 0"
			" LEFT JOIN LocationsT ON UserTimesT.LocationID = LocationsT.ID ");
		break;

	case 392:
		//Ladders
		ASSERT(FALSE);
		//This is all done in GetRecordset() now.
		return _T("");
		break;

	case 260:
		//ASAPS Survey
		/* Version History
			TES 3/4/2004 - Took out reference to AppointmentsT.ResourceID
			// (m.hancock 2006-08-14 16:42) - PLID 10730 - Added ethnicity field
			// (m.hancock 2006-08-14 17:27) - PLID 21976 - removed inquiries from appearing in results, 
			//  so only Patient, Prospects, or Patient / Prospects will appear.
			// (c.haag 2007-02-20 17:33) - PLID 24213 - Fixed age calculation
			// (j.jones 2009-10-19 16:39) - PLID 35994 - split up race and ethnicity
			// (j.gruber 2010-01-18 11:17) - PLID 34271 - added patient and appt location name and IDs
			// (d.thompson 2012-08-09) - PLID 52045 - Reworked tables for Ethnicity, now reporting practice name field
		*/
		{
			CString sql = "SELECT ProcedureT.Name, ProcedureT.ID AS ProcID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, AppointmentsT.Date AS TDate, PersonT.Gender,  "

				//
				// (c.haag 2007-02-20 16:34) - PLID 24213 - Calculate the age of the patient such that patients
				// older than 99 years old will show properly. We take the difference of the year components
				// between this year and the patient's birthdate, and then we subtract one if the month and day
				// of the patient's birthday are less than today's month and day. The report required that the
				// age be a text field, so we will keep it that way.
				//
				//"Right(Year(GetDate() - PersonT.BirthDate),2) AS Age, "
				"convert(nvarchar, YEAR(GETDATE()) - YEAR(PersonT.BirthDate) - "
				"	CASE WHEN MONTH(PersonT.BirthDate) > MONTH(GETDATE()) THEN 1 "
				"	WHEN MONTH(PersonT.BirthDate) < MONTH(GETDATE()) THEN 0 "
				"	WHEN DAY(PersonT.BirthDate) > DAY(GETDATE()) THEN 1 "
				"	ELSE 0 END) AS Age, "

				"PatientsT.MainPhysician AS ProvID, "
				// (b.spivey, May 28, 2013) - PLID 56871
				"	LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Race, "
				"	LEFT(OfficialRaceSubQ.OfficialName, LEN(OfficialRaceSubQ.OfficialName) -1) AS CDCRace, "
				"EthnicityT.Name AS CDCEthnicity, "
				" LocationsT.ID as PatLocID, LocationsT.Name as PatLocationName, ApptLocT.ID as ApptLocID, ApptLocT.Name as ApptLocationName, ";

			if(saExtraValues.GetSize()) {
				sql += "ResourceT.ID AS ItemID ";
			}
			else {
				sql += "-1 AS ItemID ";
			}
			sql += "FROM "
				"AppointmentsT LEFT JOIN (AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
				"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				// (b.spivey, May 28, 2013) - PLID 56871 - Get race lists. 
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RT.Name + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
				"			WHERE PRT.PersonID = PersonT.ID "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) RaceSubQ (RaceName) "
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RCT.OfficialRaceName + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID "
				"			WHERE PRT.PersonID = PersonT.ID  "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) OfficialRaceSubQ (OfficialName) "
				"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
				"LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID "
				"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
				"LEFT JOIN LocationsT ApptLocT ON AppointmentsT.LocationID = ApptLocT.ID ";

			if(saExtraValues.GetSize()) {
				sql += "LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
				"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID ";
			}
			
			sql += "WHERE (ProcedureT.ID Is Not Null) AND (AppointmentsT.Status <> 4) AND "
				"(AppointmentsT.ShowState <> 3) AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4 OR AptTypeT.Category = 6) " // 3 = no show, AptTypeT.Category 3,4,6 = minor procedure, surgery, other procedure
				"AND (PatientsT.CurrentStatus = 1 OR PatientsT.CurrentStatus = 2 OR PatientsT.CurrentStatus = 3) ";
			return _T(sql);
		}
		break;

	case 262:
		//ASPS report
		/* Version History
			TES 7/9/2004: Updated to support multi-procedure.
			TES 9/21/2004: Updated to account for new EmrDetailElementsT structure.
			TES 8/8/2006: Updated to account for the removal of the new EmrDetailElementsT structure.
			// (j.gruber 2007-01-04 09:36) - PLID 24035 - took out EMRMasterT.ProviderID since it wasn't used and doens't exist anymore
			// (c.haag 2008-06-17 17:32) - PLID 30319 - Changed name calculation for details
			// (j.jones 2010-12-08 09:34) - PLID 41728 - ensured that a patient age in "months" shows in the < 18 year bucket
		*/
  		return _T("SELECT SubQ.ID, SubQ.ProcName, SubQ.InfoName, SubQ.Data, SubQ.Group4,  "
			"SubQ.Date AS TDate, SubQ.InputDate AS IDate, SubQ.ProcedureID AS ProcID, SubQ.LocationID AS LocID, Year(SubQ.Date) AS TYear "
			" "
			"FROM "
			"( "
			"/* Techniques */ "
			"SELECT "
			"EMRMasterT.ID, ProcedureT.Name AS ProcName, "
			"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS InfoName, "
			"EMRDataT.Data, NULL AS Group4,  "
			"EMRMasterT.Date, EMRMasterT.InputDate, EMRProcedureT.ProcedureID, EMRMasterT.LocationID "
			"FROM "
			" "
			"EMRMasterT LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"	INNER JOIN EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID "
			"	INNER JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID "
			"	LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"	LEFT JOIN EmrSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID "
			"	LEFT JOIN EMRDataT ON EmrSelectT.EmrDataID = EMRDataT.ID "


			"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRProcedureT.Deleted = 0 AND "
			"  EMRInfoT.DataType IN (2,3) "
			"/* End Techniques */ "
			" "
			"UNION "
			" "
			"/* Age Groups */ "
			"SELECT "
			"EMRMasterT.ID, ProcedureT.Name AS ProcName, 'Age Groups Treated' AS InfoName,  "
			"CASE 	WHEN PatientAge LIKE '%month%' OR PatientAge < 19 THEN '18 or Younger' "
			"	WHEN PatientAge < 35 THEN '19 - 34 years' "
			"	WHEN PatientAge < 51 THEN '35 - 50 years' "
			"	WHEN PatientAge < 65 THEN '51 - 64 years' "
			"	WHEN PatientAge > 64 THEN '65+ years' "
			"	ELSE 'Unknown Age' END AS AgeGroup,  "
			"CASE	WHEN PatientGender = 1 THEN 'Male' "
			"	WHEN PatientGender = 2 THEN 'Female'  "
			"	ELSE 'Unknown Gender' END AS GenderGroup,  "
			"EMRMasterT.Date, EMRMasterT.InputDate, EMRProcedureT.ProcedureID, EMRMasterT.LocationID "
			"FROM "
			"EMRMasterT LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"	INNER JOIN EmrProcedureT ON EMRMasterT.ID = EmrProcedureT.EMRID "
			"	INNER JOIN ProcedureT ON EmrProcedureT.ProcedureID = ProcedureT.ID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRProcedureT.Deleted = 0 "
			" "
			"/* End Age Groups */ "
			" "
			"UNION "
			" "
			"/* Gender */ "
			"SELECT "
			"EMRMasterT.ID, ProcedureT.Name AS ProcName, 'Gender' AS InfoName,  "
			"CASE	WHEN PatientGender = 1 THEN 'Male' "
			"	WHEN PatientGender = 2 THEN 'Female' "
			"	ELSE 'Unknown Gender' END AS GenderGroup,  "
			"NULL AS Group4,  "
			"EMRMasterT.Date, EMRMasterT.InputDate, EmrProcedureT.ProcedureID, EMRMasterT.LocationID "
			"FROM "
			" "
			"EMRMasterT LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"	INNER JOIN EmrProcedureT ON EMRMasterT.ID = EmrProcedureT.EMRID "
			"	INNER JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRProcedureT.Deleted = 0 "
			"/* End Gender */ "
			") AS SubQ "
			"ORDER BY ProcName ");
		break;

	case 399: //medication list
		/*
			Version History
			TES 7/30/03 - Added Unit field
			(c.haag 2007-02-02 17:01) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			// (d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity, Description is now PatientInstructions
			(d.thompson 2009-01-15) - PLID 32176 - DrugList.Unit is now DrugStrengthUnitsT.Name, joined from DrugList.StrengthUnitID
			// (d.thompson 2009-03-11) - PLID 33481 - Actually this should use QuantityUnitID, not StrengthUnitID
		*/
		return _T("SELECT LEFT(EMRDataT.Data,255) AS Name, PatientInstructions, DefaultRefills, DefaultQuantity, "
			"COALESCE(DrugStrengthUnitsT.Name, '') AS Unit FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"LEFT JOIN DrugStrengthUnitsT ON DrugList.QuantityUnitID = DrugStrengthUnitsT.ID ");
	break;

	case 415:
			//Referral Source List
			/* Version History
				JMM - 7/29/03 - Created
				TES - 9/15/03 - Moved this query to the Admin function, where it belongs, and added ParentName.
			*/
		return (_T("SELECT ReferralSourceT.PersonID, ReferralSourceT.Name, ReferralSourceT.Parent, CASE WHEN ParentSourceT.Name Is Null THEN '{Root}' ELSE ParentSourceT.Name END AS ParentName "
			"FROM ReferralSourceT LEFT JOIN ReferralSourceT ParentSourceT ON ReferralSourceT.Parent = ParentSourceT.PersonID"));
		break;

	case 422:
	case 423:
	case 424:
		//Service Code Barcodes
		return _T("SELECT ServiceT.ID AS ServiceID, ServiceT.Name, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Barcode, ServiceT.Price, ServiceT.Category AS CatID "
			"FROM "
			"CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"WHERE Barcode IS NOT NULL AND Rtrim(Barcode) <> '' ORDER BY CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name ");
		break;

	case 429:
		//Template Listing
		/*	Version History
			DRT 8/4/03 - Created.
			// (z.manning, 12/20/2006) - PLID 23248 - Fixed the report to account for data structure changes.
				(The apparent description is no longer a field in LineItemT since that's redundant.
				It's now all maintained within the CTemplateLineItemInfo class, however, that doesn't
				do us much good here as we need it straight from data for this report. So, rather than
				calculate the apparent description in SQL, which would make maintaining the apparent 
				description more trouble than it's worth. Instead, I indvidually load the line items into
				memory, then get the apparent description, then insert them into a table variable.
				// (j.gruber 2009-02-17 10:45) - PLID 32955 - added resourceID and Template filter 
		*/
		{
			// (j.gruber 2009-02-17 10:56) - PLID 32955 - generate the extended filter
			CString strResources;
			if (bExtended && saExtraValues.GetSize()) {
				CString strAns = " TemplateItemT.ID IN (SELECT TemplateItemID FROM TemplateItemResourceT WHERE ResourceID IN (";
				for(int i = 0; i < saExtraValues.GetSize(); i++) {
					strAns += saExtraValues[i] + ",";
				}
				strAns.TrimRight(",");
				strAns += "))";
				strResources = strAns;

				//include the all resources also
				strResources = " AND (" + strResources + " OR TemplateItemT.AllResources <> 0)";
			} else {
				// No extra filter
				strResources = "";
			}
			
			
			// (z.manning, 12/20/2006) - PLID 23248 - Declare the table variable.
			CString strTemplateListing = 
				"SET NOCOUNT ON  \r\n"
				"DECLARE @TemplateListingInfo TABLE  \r\n"
				"(  \r\n"
				"	TemplateID int,  \r\n"
				"	Name nvarchar(50),  \r\n"
				"	StartDate datetime,  \r\n"
				"	EndDate datetime,  \r\n"
				"	Priority int,  \r\n"
				// (z.manning 2009-06-03 16:55) - PLID 34476 - Changed the ApparentDescription to ntext in
				// case they have a template line item with a ton of exceptions.
				"	ApparentDescription ntext \r\n"
				")  \r\n";

			// (z.manning, 12/20/2006) - PLID 23248 - Load all template line items from data.
			// (z.manning 2015-02-04 15:43) - PLID 64295 - Added collection ID
			ADODB::_RecordsetPtr prs = CreateRecordset(
				"SELECT TemplateItemT.*, dbo.GetTemplateItemResourceIDString(TemplateItemT.ID) AS ResourceIDString, \r\n"
				"	TemplateT.ID AS TemplateID, TemplateT.Name, TemplateT.Priority, TemplateCollectionApplyT.CollectionID \r\n"
				"FROM TemplateItemT \r\n"
				"INNER JOIN TemplateT ON TemplateItemT.TemplateID = TemplateT.ID \r\n"
				"LEFT JOIN TemplateCollectionApplyT ON TemplateCollectionApplyT.ID = TemplateItemT.TemplateCollectionApplyID \r\n"
				" WHERE (1=1) %s "
				, strResources);

			// (z.manning 2011-12-07 12:18) - PLID 46910 - Need a resource map to load template items
			CMap<long,long,CString,LPCTSTR> mapResources;
			FillResourceMap(&mapResources);

			// (z.manning, 12/20/2006) - PLID 23248 - Insert info from all the template line items into the table variable.
			while(!prs->eof)
			{
				CTemplateLineItemInfo pLineItem;
				// (z.manning 2011-12-07 11:01) - PLID 46910 - This function now has a parameter for whether or not we
				// need to load template item exceptions, which we do not need here.
				pLineItem.LoadFromRecordset(prs, FALSE, &mapResources);

				strTemplateListing += FormatString(
					"INSERT INTO @TemplateListingInfo (TemplateID, Name, StartDate, EndDate, Priority, ApparentDescription)  \r\n"
					"	SELECT %li, '%s', '%s', %s, %li, '%s'  \r\n",
					AdoFldLong(prs, "TemplateID", -1), _Q(AdoFldString(prs, "Name", "")),
					FormatDateTimeForSql(pLineItem.m_dtStartDate, dtoDate),
					pLineItem.NeverEnds() ? "NULL" : "'" + FormatDateTimeForSql(pLineItem.m_dtEndDate, dtoDate) + "'", 
					AdoFldLong(prs, "Priority", 0),
					_Q(pLineItem.GetApparentDescription(stetNormal)) );

				prs->MoveNext();
			}

			strTemplateListing +=
				"SET NOCOUNT OFF  \r\n"
				"SELECT  Name, TemplateID as TemplateID, StartDate as StartDate, EndDate as EndDate, Priority, ApparentDescription  \r\n"
				"FROM @TemplateListingInfo AS TemplateQ  \r\n"
				"ORDER BY StartDate DESC  \r\n";
				
			return _T(strTemplateListing);
		}
		break;
	
	case 445:
		//PracYakker Messages by Sender
		/*  Version History
			TES 9/8/03 - Created.
			TES 3/3/04 - Filtered out messages that had been deleted by both sender and recipient.
		*/
		return _T("SELECT MessageID, "
			"CASE WHEN RegardingID = -1 THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Regarding,  "
			"CASE WHEN Priority = 1 THEN 'Low' WHEN Priority = 2 THEN 'Medium' WHEN Priority = 3 THEN 'High' END AS Priority, "
			"DateSent AS Date, SenderQ.Username AS Sender, RecipientQ.Username AS Recipient, Text, "
			"MessageGroupID, SenderQ.PersonID AS UserID  "
			"FROM MessagesT LEFT JOIN PersonT ON RegardingID = PersonT.ID LEFT JOIN UsersT SenderQ ON SenderID = SenderQ.PersonID "
			"LEFT JOIN UsersT RecipientQ ON RecipientID = RecipientQ.PersonID "
			"WHERE DeletedBySender = 0 OR DeletedByRecipient = 0");
		break;


	case 569: //Procedure Cheat Sheet Information
		/*Version History
			JMM - 8/4/2006 - Created
			// (j.gruber 2007-02-20 10:55) - PLID 24818 - Moved to Admin tab from Other Tab
			// (z.manning, 07/02/2007) - PLID 18629 - All of these custom fields are now ntext. As a
			// result, the section visiblity formulas in the report file no longer work. I had to use
			// the Crystal IsNull function, thus if we have an empty string, give it a value of NULL.
		*/
		return _T(
			"SELECT ID As ProcID, Name as ProcName, \r\n"
			" CASE WHEN Custom1 LIKE '' THEN NULL ELSE Custom1 END AS Box2, \r\n"
			" CASE WHEN Custom2 LIKE '' THEN NULL ELSE Custom2 END AS Box3, \r\n"
			" CASE WHEN Custom3 LIKE '' THEN NULL ELSE Custom3 END AS Box6, \r\n"
			" CASE WHEN Custom4 LIKE '' THEN NULL ELSE Custom4 END AS Box4, \r\n"
			" CASE WHEN Custom5 LIKE '' THEN NULL ELSE Custom5 END AS Box5, \r\n"
			" CASE WHEN Custom6 LIKE '' THEN NULL ELSE Custom6 END AS Box7, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 63) AS Box2Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 64) AS Box3Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 65) AS Box6Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 66) AS Box4Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 67) AS Box5Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 68) AS Box7Name  \r\n"
			" From ProcedureT ");
		break;
		

	case 570: //Procedure Content
			{
		/*Version History
			// (a.walling 2006-12-13 17:49) - PLID 22598 - Added alternate language consent field
			JMM - 8/4/2006 - Created
			// (z.manning, 09/04/2007) - PLID 27286 - Renamed ProcedureDetails to ProcDetails.
			// (z.manning, 09/20/2007) - PLID 27467 - Moved this report here from ReportInfoPreview.cpp.
			// Also, added the 6 procedure non-NexForms custom fields to this report.
		*/
		CString strSql;
		strSql.Format("SELECT ID As ProcID, Name as ProcName, MiniDescription, PreOp, TheDayOf,  \r\n"
			" PostOp, Recovery, ProcDetails, Risks, Alternatives, Complications, SpecialDiet,  \r\n"
			" Showering, Bandages, Consent, AltConsent, HospitalStay, CustomSection1, CustomSection2,  \r\n"
			" CustomSection3, CustomSection4, CustomSection5, CustomSection6, CustomSection7,  \r\n"
			" CustomSection8, CustomSection9, CustomSection10,  \r\n"
			" CASE WHEN Custom1 LIKE '' THEN NULL ELSE Custom1 END AS Custom1, \r\n"
			" CASE WHEN Custom2 LIKE '' THEN NULL ELSE Custom2 END AS Custom2, \r\n"
			" CASE WHEN Custom3 LIKE '' THEN NULL ELSE Custom3 END AS Custom3, \r\n"
			" CASE WHEN Custom4 LIKE '' THEN NULL ELSE Custom4 END AS Custom4, \r\n"
			" CASE WHEN Custom5 LIKE '' THEN NULL ELSE Custom5 END AS Custom5, \r\n"
			" CASE WHEN Custom6 LIKE '' THEN NULL ELSE Custom6 END AS Custom6, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 63) AS Custom1Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 64) AS Custom2Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 65) AS Custom3Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 66) AS Custom4Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 67) AS Custom5Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 68) AS Custom6Name,  \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 70) AS CustomSection1Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 71) AS CustomSection2Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 72) AS CustomSection3Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 73) AS CustomSection4Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 74) AS CustomSection5Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 75) AS CustomSection6Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 76) AS CustomSection7Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 77) AS CustomSection8Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 78) AS CustomSection9Name, \r\n"
			" (SELECT Name From CustomFieldsT WHERE ID = 79) AS CustomSection10Name \r\n"
			" From ProcedureT ");
		if(saExtraValues.GetSize() > 0) {
			//We only accept 1 parameter.
			strSql += "WHERE ProcedureT.ID = '" + saExtraValues[0] + "'";
		}
		
		return _T(strSql);
			}
		break;

	case 251:
		//Procedure List
		// (j.gruber 2007-02-20 10:55) - PLID 24818  - moved to Admin Tab from Other Tab
		// (j.gruber 2008-06-11 09:11) - PLID 30350 - make it show all procedures, not just ones with CPT code associated
		return _T("Select "
		"ProcedureT.Name AS ProcName, ServiceT.Name AS ServiceName, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Price, "
		" CASE WHEN ServiceT.ID IS NULL then 0 else 1 END as HasServiceCode, "
		" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN 1 else 0 END as IsMasterProc "
		"From "
		"ProcedureT LEFT JOIN ServiceT ON ProcedureT.ID = ServiceT.ProcedureID "
		"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID");
	break;	
	


	case 457:
		//TOPS Summary
		/*	Version History
			DRT 10/29/2003 - Created.  Attempts to show as much information that is required on the TOPS survey form.  A number of
				these things are ones that we do not track, so we pull all the custom text fields and custom list fields.
			(j.armen 2011-06-27 16:58) - PLID 44253 - Updated to use new custom list data structure
			// (j.gruber 2014-03-07 14:19) - PLID 61263 - updated for ICD9
		*/
		switch(nSubLevel) {
		case 1:	//subreport
			//Select all CPT Codes for each bill.  Will link to BillID in the main query
			return _T("SELECT BillsT.ID AS BillID, CPTCodeT.Code, ServiceT.Name "
			"FROM BillsT "
			"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"AND CPTCodeT.ID IS NOT NULL");
			break;
		default:	//main report
		return _T("SELECT "
			"/*General report information*/ "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.BirthDate,  "
			"CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"AppointmentsT.Date AS ApptDate, POSLocT.Name AS ServiceLocation, \r\n"
			" ICD9T1.CodeNumber as ICD9Code1, \r\n "
			" ICD9T2.CodeNumber as ICD9Code2, \r\n "
			" ICD9T3.CodeNumber as ICD9Code3, \r\n "
			" ICD9T4.CodeNumber as ICD9Code4, \r\n "
			" ICD10T1.CodeNumber as ICD10Code1, \r\n "
			" ICD10T2.CodeNumber as ICD10Code2, \r\n "
			" ICD10T3.CodeNumber as ICD10Code3, \r\n "
			" ICD10T4.CodeNumber as ICD10Code4, \r\n "
			"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, BillsT.Description AS BillDesc, "
			" "
			"/*Filters*/ "
			"BillsT.Date AS BillDate, BillsT.InputDate AS BillInputDate, AppointmentsT.CreatedDate AS ApptInputDate,  "
			"PatientsT.MainPhysician AS ProvID, PersonT.Location AS LocID, BillsT.ID AS BillID, "
			" "
			"/*Custom Field Information For Editable reports*/ "
			"G1Custom1Q.TextParam AS G1Custom1, G1Custom2Q.TextParam AS G1Custom2, G1Custom3Q.TextParam AS G1Custom3, "
			"G1Custom4Q.TextParam AS G1Custom4, Custom1TextQ.TextParam AS CustomText1, Custom2TextQ.TextParam AS CustomText2,  "
			"Custom3TextQ.TextParam AS CustomText3, Custom4TextQ.TextParam AS CustomText4, Custom5TextQ.TextParam AS CustomText5,  "
			"Custom6TextQ.TextParam AS CustomText6, dbo.GetCustomList(PersonT.ID, 21) AS CustomList1, dbo.GetCustomList(PersonT.ID, 22) AS CustomList2,  "
			"dbo.GetCustomList(PersonT.ID, 23) AS CustomList3, dbo.GetCustomList(PersonT.ID, 24) AS CustomList4, dbo.GetCustomList(PersonT.ID, 25) AS CustomList5,  "
			"dbo.GetCustomList(PersonT.ID, 26) AS CustomList6 "
			" "
			"FROM "
			"PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			"LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			"LEFT JOIN  "
			"	/*this query gives us a combination of every Bill / Procedure*/ "
			"	(SELECT BillID, ServiceT.ProcedureID, LineItemT.PatientID "
			"	FROM ChargesT INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			"	LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	WHERE ServiceT.ProcedureID IS NOT NULL AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"	GROUP BY BillID, ProcedureID, PatientID "
			"	) BillsQ "
			"ON AptPurposeT.ID = BillsQ.ProcedureID AND AppointmentsT.PatientID = BillsQ.PatientID "
			"LEFT JOIN BillsT ON BillsQ.BillID = BillsT.ID "
			"LEFT JOIN LocationsT POSLocT ON BillsT.Location = POSLocT.ID "
			" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
			" "
			"/*Custom Fields*/ "
			"/*G1 Custom1-Custom4*/ "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 1) G1Custom1Q "
			"ON PatientsT.PersonID = G1Custom1Q.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 2) G1Custom2Q "
			"ON PatientsT.PersonID = G1Custom2Q.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 3) G1Custom3Q "
			"ON PatientsT.PersonID = G1Custom3Q.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 4) G1Custom4Q "
			"ON PatientsT.PersonID = G1Custom4Q.PersonID "
			"/*Custom tab custom1-6*/ "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 11) Custom1TextQ "
			"ON PatientsT.PersonID = Custom1TextQ.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 12) Custom2TextQ "
			"ON PatientsT.PersonID = Custom2TextQ.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 13) Custom3TextQ "
			"ON PatientsT.PersonID = Custom3TextQ.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 14) Custom4TextQ "
			"ON PatientsT.PersonID = Custom4TextQ.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 15) Custom5TextQ "
			"ON PatientsT.PersonID = Custom5TextQ.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 16) Custom6TextQ "
			"ON PatientsT.PersonID = Custom6TextQ.PersonID "
			"WHERE PatientsT.PersonID > 0 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 "
			"AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4 OR AptTypeT.Category = 6 /*Surgery, Minor Pr, Other Pr*/)");
		break;
		}

		case 588: //Discount Categories
			/*Version History
			// (j.gruber 2007-05-31 11:14) - PLID 25975 - Created
			*/
			return _T("SELECT ID, Description, Active FROM DiscountCategoriesT");
		break;

		case 589: //Sales
			/*Version History
			// (j.gruber 2007-05-31 12:29) - PLID 25975 - Created
			*/
			return _T("SELECT SalesT.ID, SaleItemsT.PercentDiscount, SaleItemsT.MoneyDiscount, "
				" SalesT.Name as SaleName, SalesT.StartDate, SalesT.EndDate AS EndDate, DiscountCategoriesT.Description as DisCatDesc, "
				" CASE WHEN CPTCodeT.ID IS NULL THEN 0 ELSE 1 END AS IsCPTCode, "
				" ServiceT.Name as ServiceName, CPTCodeT.Code, CPTCodeT.SubCode "
				" FROM SaleItemst LEFT JOIN SalesT ON SaleItemsT.SaleID = SalesT.ID "
				" LEFT JOIN ServiceT ON SaleItemsT.ServiceID = ServiceT.ID "
				" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
				" LEFT JOIN DiscountCategoriesT ON SalesT.DiscountCategoryID = DiscountCategoriesT.ID ");
		break;

		case 590: //Coupons
			/*Version History
			// (j.gruber 2007-05-31 12:06) - PLID 25975 - created
			*/
			return _T("SELECT ID, Description, StartDate, EndDate, DiscountType, PercentOff, DiscountAmount From CouponsT");
		break;

		case 591: //Suggested Sales
			/* Version History
			// (j.gruber 2007-05-31 15:32) - PLID 25975 - Created
			*/
			return _T("SELECT SuggestedSalesT.ID, MasterServiceID, ServiceID, Reason, MasterServiceT.Name as MasterName, ServiceT.Name as ServiceName, "
				" CASE WHEN CPTCodeT.ID IS NULL THEN 0 ELSE 1 END AS IsCPTCode, "
				" CASE WHEN MasterCPTCodeT.ID IS NULL THEN 0 ELSE 1 END AS MasterIsCPTCode, "
				" CPTCodeT.Code, CPTCodeT.SubCode, "
				" MasterCPTCodeT.Code as MasterCode, MasterCPTCodeT.SubCode As MasterSubCode, "
				" ServiceT.Price, MasterServiceT.Price as MasterPrice "
				" FROM SuggestedSalesT "
				" LEFT JOIN ServiceT MasterServiceT ON SuggestedSalesT.MasterServiceID = MasterServiceT.ID " 
				" LEFT JOIN ServiceT ON SuggestedSalesT.ServiceID = ServiceT.ID "
				" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
				" LEFT JOIN CPTCodeT MasterCPTCodeT ON MasterServiceT.ID = MasterCPTCodeT.ID "
				" LEFT JOIN ProductT MasterProductT ON MasterServiceT.ID = MasterProductT.ID "
				);

		break;

		case 592: //Advanced Commissions
			/* Version History
			// (j.gruber 2007-05-31 16:55) - PLID 25975 - Created
			// (j.jones 2009-11-20 15:40) - PLID 36384 - supported tiered commissions
			*/

			return _T("SELECT UseTieredCommissions, IgnoreShopFee, "
				"ProvID AS ProvID, ServiceID, CommPercentage, RuleID, StartDate, "
				"EndDate, BasedOnStartDate, BasedOnEndDate, RulePercentage, "
				"Name, MoneyThreshold, ProvFirst, ProvMiddle, ProvLast, ProvTitle, "
				"ServiceName, ServicePrice, IsCPTCode, Code, SubCode, "
				"RuleTypeName, OverwritePriorRules "
				"FROM ("
					//select commission % for Basic Commission providers
					"SELECT ProvidersT.UseTieredCommissions, ProvidersT.IgnoreShopFee, "
					"ProvidersT.PersonID AS ProvID, ServiceID, Percentage as CommPercentage, -1 as RuleID, NULL as StartDate, "
					"NULL as EndDate, NULL as BasedOnStartDate, NULL as BasedOnEndDate, 0 as RulePercentage, "
					"'' as Name, Convert(money, 0) as MoneyThreshold, "
					"PersonT.First as ProvFirst, PersonT.Middle as ProvMiddle, PersonT.Last as ProvLast, PersonT.Title as ProvTitle, "
					"ServiceT.Name as ServiceName, ServiceT.Price as ServicePrice, "
					"CASE WHEN CPTCodeT.ID IS NULL THEN 0 else 1 END As IsCPTCode, "
					"CPTCodeT.Code, CPTCodeT.SubCode, "
					"NULL AS RuleTypeName, NULL AS OverwritePriorRules "
					"FROM CommissionT "
					"INNER JOIN ProvidersT ON CommissionT.ProvID = ProvidersT.PersonID "
					"INNER JOIN PersonT On ProvidersT.PersonID = PersonT.ID "
					"LEFT JOIN ServiceT ON CommissionT.ServiceID = ServiceT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE ProvidersT.UseTieredCommissions = 0 "
					"UNION "
					//select commission rules for Basic Commission providers
					"SELECT ProvidersT.UseTieredCommissions, ProvidersT.IgnoreShopFee, "
					"ProvidersT.PersonID AS ProvID, ServiceID, NULL as CommPercentage, RuleID, "
					"StartDate, EndDate, BasedOnStartDate, BasedOnEndDate, Percentage, CommissionRulesT.Name, MoneyThreshold, "
					"PersonT.First as ProvFirst, PersonT.Middle as ProvMiddle, PersonT.Last as ProvLast, PersonT.Title as ProvTitle, "
					"ServiceT.Name as ServiceName, ServiceT.Price as ServicePrice, "
					"CASE WHEN CPTCodeT.ID IS NULL THEN 0 else 1 END As IsCPTCode, "
					"CPTCodeT.Code, CPTCodeT.SubCode, "
					"NULL AS RuleTypeName, NULL AS OverwritePriorRules "
					"FROM CommissionRulesLinkT "
					"INNER JOIN CommissionRulesT ON CommissionRulesLinkT.RuleID = CommissionRulesT.ID "
					"INNER JOIN ProvidersT ON CommissionRulesLinkT.ProvID = ProvidersT.PersonID "
					"INNER JOIN PersonT On ProvidersT.PersonID = PersonT.ID "
					"LEFT JOIN ServiceT ON CommissionRulesLinkT.ServiceID = ServiceT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE CommissionRulesT.IsTieredCommission = 0 AND ProvidersT.UseTieredCommissions = 0 "
					"UNION "
					//select commission rules for Tiered Commission providers
					"SELECT ProvidersT.UseTieredCommissions, ProvidersT.IgnoreShopFee, "
					"ProvidersT.PersonID AS ProvID, NULL AS ServiceID, NULL AS CommPercentage, RuleID, "
					"StartDate, EndDate, BasedOnStartDate, BasedOnEndDate, Percentage, CommissionRulesT.Name, MoneyThreshold, "
					"PersonT.First as ProvFirst, PersonT.Middle as ProvMiddle, PersonT.Last as ProvLast, PersonT.Title as ProvTitle, "
					"NULL as ServiceName, NULL as ServicePrice, NULL As IsCPTCode, NULL AS Code, NULL AS SubCode, "
					"CASE WHEN RuleType = 0 THEN 'Service Codes' WHEN RuleType = 1 THEN 'Inventory Items' WHEN RuleType = 2 THEN 'Gift Certificates' ELSE 'All Sales' END AS RuleTypeName, "
					"OverwritePriorRules "
					"FROM TieredCommissionRulesLinkT "
					"INNER JOIN CommissionRulesT ON TieredCommissionRulesLinkT.RuleID = CommissionRulesT.ID "
					"INNER JOIN ProvidersT ON TieredCommissionRulesLinkT.ProviderID = ProvidersT.PersonID "
					"INNER JOIN PersonT On ProvidersT.PersonID = PersonT.ID "
					"WHERE CommissionRulesT.IsTieredCommission = 1 AND ProvidersT.UseTieredCommissions = 1"
				") AS AdvCommissionsQ");
		break;

		case 593: //Commission Rules
			/*Version History
			// (j.gruber 2007-06-01 12:07) - PLID 25975 - Created
			// (j.jones 2009-11-20 15:04) - PLID 36385 - supported tiered commissions
			*/
			return _T(" SELECT ID AS RuleID, "
				" StartDate, EndDate, BasedOnStartDate, BasedOnEndDate, Percentage, CommissionRulesT.Name, MoneyThreshold, "
				" IsTieredCommission, OverwritePriorRules "
				" FROM CommissionRulesT ");
		break;


		case 615:
		case 626:
		//Clients By EMR Specialist
		//Clients By EMR Status
		
		/*	Version History
			// (j.gruber 2007-11-09 09:08) - PLID 28047 - Created
			// (j.gruber 2008-04-02 17:17) - PLID 29440 - added new report sorted differently
			// (j.gruber 2008-05-22 13:27) - PLID 29439 - split 625  out into its own query because I'm adding a detailed version
			// (j.gruber 2008-05-28 10:48) - PLID 30181 - added new report
			// (j.gruber 2008-05-28 10:48) - PLID 30182 - added status note  and last contact date to the report
			// (j.gruber 2008-05-28 10:48) - PLID 30183 - changed the filter
			// (c.haag 2010-07-12 13:34) - PLID 39601 - Added city, state, zip, custom list 4 (specialty)
			// (j.gruber 2010-07-30 15:14) - PLID 39879 - added Secondary EMR Specialist
			// (j.armen 2011-06-27 16:59) - PLID 44253 - Updated to use new custom list data structure
		*/
		return _T("SELECT PersonT.ID as PatID, PatientsT.UserDefinedID as ClientID, PatientsT.CurrentStatus, PersonT.FirstContactDate as FCDDate, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Title, "
			"  PersonT.City, PersonT.State, PersonT.Zip, "
			"  dbo.GetCustomList(PersonT.ID, 24) AS CustomList4, "
			"  EMRSpecPersonT.First as EMRSpecFirst, EMRSpecPersonT.Last as EMRSpecLast, UsersT.Username as EMRUserName, "
			"  EMRStatusT.StatusName, EMRStatusT.ID AS EmrStatusID, EMRSpecialistT.UserID as EMRSpecialistID, "
			"  NxClientsT.EMRStatusNote, "
			"  (SELECT Top 1 Date FROM Notes WHERE PersonID = PersonT.ID AND UserID = NxClientsT.EMRSpecialistID ORDER BY ID DESC) as LastContactDate, "
			"  SecEMRSpecPersonT.First as SecEMRSpecFirst, SecEMRSpecPersonT.Last as SecEMRSpecLast, SecUsersT.Username as SecEMRUserName, SecEMRSpecialistT.UserID as SecEMRSpecialistID "
			"  FROM PersonT INNER JOIN NxClientsT ON PersonT.ID = NxClientsT.PersonID "
			"  INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"  LEFT JOIN EMRSpecialistT ON NXClientsT.EMRSpecialistID = EMRSpecialistT.UserID "
			"  LEFT JOIN UsersT ON EMRSpecialistT.UserID = UsersT.PersonID "
			"  LEFT JOIN PersonT EMRSpecPersonT ON UsersT.PersonID = EMRSpecPersonT.ID "
			"  LEFT JOIN EMRStatusT ON NXClientsT.EMRStatusID = EMRStatusT.ID "
			"  LEFT JOIN EMRSpecialistT SecEMRSpecialistT ON NXClientsT.SecondaryEMRSpecialistID = SecEMRSpecialistT.UserID "
			"  LEFT JOIN UsersT SecUsersT ON SecEMRSpecialistT.UserID = SecUsersT.PersonID "
			"  LEFT JOIN PersonT SecEMRSpecPersonT ON SecUsersT.PersonID = SecEMRSpecPersonT.ID "
			"  WHERE (NXClientsT.NexEMR <> 0) OR (NxClientsT.NexEMR = 0 AND (NxClientsT.EMRSpecialistID IS NOT NULL OR NxClientsT.EMRStatusID IS NOT NULL OR NxClientsT.SecondaryEMRSpecialistID IS NOT NULL)) " );
		break;



		case 625:
		//EMR Clients by Name
		/*	Version History
			// (j.gruber 2007-11-09 09:08) - PLID 28047 - Created
			// (j.gruber 2008-04-02 17:17) - PLID 29440 - added new report sorted differently
			// (j.gruber 2008-05-22 13:27) - PLID 29439 - split this out into its own query because I'm adding a detailed version
			// (j.gruber 2008-05-28 10:48) - PLID 30183 - changed the filter
			// (j.armen 2011-06-27 16:59) - PLID 44253 - Updated to use new custom list data structure
		
		*/
		return _T("SELECT PersonT.ID as PatID, PatientsT.UserDefinedID as ClientID, PatientsT.CurrentStatus, PersonT.FirstContactDate as FCDDate, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Title, "
			"  EMRSpecPersonT.First as EMRSpecFirst, EMRSpecPersonT.Last as EMRSpecLast, UsersT.Username as EMRUserName, "
			"  EMRStatusT.StatusName, EMRStatusT.ID AS EmrStatusID, EMRSpecialistT.UserID as EMRSpecialistID, "
			"  NxClientsT.EMRStatusNote, NxClientsT.EMRFollowupDate, PersonT.City, PersonT.State, "
			"  dbo.GetCustomList(PersonT.ID, 24) as Specialty, "
			"  (SELECT Top 1 Date FROM Notes WHERE PersonID = PersonT.ID AND UserID = NxClientsT.EMRSpecialistID ORDER BY ID DESC) as LastContactDate, "
			"  EMRTypeT.TypeName "
			"  FROM PersonT INNER JOIN NxClientsT ON PersonT.ID = NxClientsT.PersonID "
			"  INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"  LEFT JOIN EMRSpecialistT ON NXClientsT.EMRSpecialistID = EMRSpecialistT.UserID "
			"  LEFT JOIN UsersT ON EMRSpecialistT.UserID = UsersT.PersonID "
			"  LEFT JOIN PersonT EMRSpecPersonT ON UsersT.PersonID = EMRSpecPersonT.ID "
			"  LEFT JOIN EMRStatusT ON NXClientsT.EMRStatusID = EMRStatusT.ID "
			"  LEFT JOIN EMRTypeT ON NXClientsT.EMRTypeID = EMRTypeT.ID "
			"  WHERE (NXClientsT.NexEMR <> 0) OR (NxClientsT.NexEMR = 0 AND (NxClientsT.EMRSpecialistID IS NOT NULL OR NxClientsT.EMRStatusID IS NOT NULL)) " );
		break;

		

		case 469:
		//Payment Categories
		/*	Version History
			JMM 2/3/04 - Created.
			DRT	4/22/2004 - PLID 11816 - Using SELECT * is not safe!  I replaced it with correct code.
		*/
		return _T("SELECT ID, GroupName, Explanation FROM PaymentGroupsT");
		break;

		case 685:
			/*Nexweb Patient Percentage
			Version History
			// (j.gruber 2009-10-28 17:48) - PLID 35772 - created			
			*/
			return _T(" SELECT  "
				" (SELECT Count(*) FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE Archived = 0 AND PersonID IN  (SELECT PersonID FROM NexwebLoginInfoT where enabled= 1)) as [Patients With Nexweb Logins],  "
				" (SELECT Count(*) FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE Archived = 0 AND CurrentStatus <> 4) AS [Total Patients] ");


		case 701:
			/*NexReminder Client Usage
			Version History
			// (f.dinatale 2010-12-10) - PLID 41275 - Created
			*/
			{
				// Set up all the objects for use.
				CNxRetrieveNxReminderClient nxReminderClientInfo("-1");
				std::vector<CNxRetrieveNxReminderClient::ClientInfo> vClientUsageInfo;
				CString strBatch, strTemp;
				CArray<long> narrLicense;

				try {
					// Retrieve all the information from NexReminder Server
					int nStatus;
 					if(nDateRange == -1) {
						nStatus = nxReminderClientInfo.GetClientUsage(narrLicense, vClientUsageInfo);
					} else {
						nStatus = nxReminderClientInfo.GetClientUsage(narrLicense, vClientUsageInfo, DateFrom, DateTo);
					}

					if(nStatus == 0) {
						// Construct batch here
						// If the transaction fails, we don't really need to rollback but it's good to just in case.
						strBatch = "BEGIN TRAN;\r\n";
						strBatch += "SET XACT_ABORT ON;\r\n";
						strBatch += "SET NOCOUNT ON;\r\n";

						// Declare a table variable to hold all of our SOAP results.
						strBatch += "DECLARE @ClientUsageT TABLE("
							"ClientID INT, "
							"Active BIT, "
							"StartDate DATETIME, "
							"EndDate DATETIME, "
							"AllotedMessages INT, "
							"RemainingCredits INT, "
							"CreditsUsedSending INT, "
							"CreditsUsedReceiving INT);\r\n";

						// (f.dinatale 2011-01-18) - PLID 41275 - Fixed the end date issue.
						CString strEnd;
						// Insert each of the record from the SOAP message into the table variable so that they can be accessed.
						// (f.dinatale 2011-01-24) - PLID 41275 - Fixed the end date to be NULL if there is no end date in the SOAP message.
						for(int i = 0; i < ((int)vClientUsageInfo.size()); i++) {
							if(vClientUsageInfo[i].strEndDate == "") {
								strEnd = "NULL";
							} else {
								strEnd = "'" + vClientUsageInfo[i].strEndDate + "'";
							}

							strTemp.Format("INSERT INTO @ClientUsageT VALUES(%li, %li, '%s', %s, %li, %li, %li, %li);\r\n", 
								vClientUsageInfo[i].nClientID, vClientUsageInfo[i].bActive, vClientUsageInfo[i].strStartDate, 
								strEnd, vClientUsageInfo[i].nAllottedCredits, vClientUsageInfo[i].nRemainingCredits, 
								vClientUsageInfo[i].nCreditsUsedSending, vClientUsageInfo[i].nCreditsUsedReceiving);
							strBatch += strTemp;
						}

						// Select all the values from the table.
						strBatch += "SET NOCOUNT OFF;\r\n";
						strBatch += "SELECT UsageT.ClientID, PersonT.Last AS LastName, PersonT.First AS FirstName, "
							//"(IF UsageT.Active 'Activated' ELSE 'Inactive') AS Active, "
							"(CASE WHEN UsageT.Active = 1 THEN 'Activated' ELSE 'Inactive' END) AS Active, "
							"UsageT.StartDate, UsageT.EndDate, UsageT.AllotedMessages, "
							"UsageT.RemainingCredits, UsageT.CreditsUsedSending, UsageT.CreditsUsedReceiving "
							"FROM @ClientUsageT UsageT "
							"INNER JOIN NxClientsT "
							"ON UsageT.ClientID = NxClientsT.LicenseKey "
							"INNER JOIN PersonT "
							"ON NxClientsT.PersonID = PersonT.ID "
							";\r\n";
						strBatch += "COMMIT TRAN;\r\n";

						return _T(strBatch);
					} else {
						// We couldn't get a valid response from NexReminder Server.
						return _T("");
					}
				} NxCatchAll(__FUNCTION__);

			}


		case 582:
			//AAFPRS Survey
			/* // (j.gruber 2006-11-28 15:00) - PLID 7048 - Created
			// (c.haag 2007-01-10 15:01) - PLID 7048 - I wrote case 3 and onward, and added
			// some comments to the other cases			
			*/
			switch (nSubLevel) {
				case 0:
					// (c.haag 2007-01-10 15:42) - PLID 7048 - There is no sublevel 0 (main) report
					return _T("SELECT 0 AS EmptyColumn");
				break;

				case 1:
					switch (nSubRepNum) {
						case 0:  //Percent Patients By Ethnicity
							//
							// 2006 form:
							// 1. Please estimate the following percentage of your patients by race / ethnicity
							//
							// (c.haag 2007-01-10 16:18) - This does not reference the configuration
							// (j.gruber 2010-07-15 10:36) - PLID 36903 - filter on patients with appts this year
							// (b.spivey, May 28, 2013) - PLID 56871 - Changed this logic to use the new data structure. 
							// (b.spivey, June 17, 2013) - PLID 56871 - Added an extra field to protect formula integrity. 
							return _T("SELECT Count(PersonT.ID) AS CountOfPersons, "
								"	CASE WHEN RaceSubQ.RaceName IS NULL THEN 'None Specified' "
								"		ELSE LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) END AS NAME, "
								"	CAST((CASE WHEN RaceSubQ.RaceName IS NULL THEN 'None Specified' "
								"		ELSE LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) END) AS NVARCHAR(255)) AS ShortName "
								"	FROM "
								" PersonT "
								"	CROSS APPLY "
								"	( "
								"		SELECT ( "
								"			SELECT RT.Name + ', ' "
								"			FROM PersonRaceT PRT "
								"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
								"			WHERE PRT.PersonID = PersonT.ID "
								"			FOR XML PATH(''), TYPE "
								"		).value('/', 'nvarchar(max)') "
								"	) RaceSubQ (RaceName) "
								" WHERE PersonT.ID IN (SELECT PatientID FROM AppointmentsT WHERE AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4 AND "
								" YEAR(AppointmentsT.Date) = #INPUTYEAR) "
								" GROUP BY RaceSubQ.RaceName ");
						break;

						case 1:  //Percent Procedures By Locations
							//
							// 2006 form:
							// 2. Please estimate the percentage of procedures (surgical and non-surgical) you performed
							// in each of the following locations during 2005 (Must add to 100%)
							//
							// (c.haag 2007-01-10 16:18) - This does not reference the configuration
							//
							return _T("SELECT Count(AppointmentsT.ID) AS CountOfAppointments, LocationsT.Name "
								" FROM AppointmentsT LEFT JOIN LocationsT "
								" ON AppointmentsT.LocationId = LocationsT.ID "
								" WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6)) "
								" AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4 "
								" AND YEAR(AppointmentsT.Date) = #INPUTYEAR "
								" GROUP BY LocationsT.Name");
						break;

						case 2:  //Percentage of Pats with Multiple Procs
							//
							// 2006 form:
							// 3. Please estimate the percentage of your patients on whom you performed multiple facial
							// procedures (at the same time or during the course of the same year):
							//
							// This references the configuration where the ID equals 4 (afsFacial)
							//
							// Appointment categories: AC_MINOR (3), AC_SURGERY (4), AC_OTHER (6)
							// (j.gruber 2010-07-15 10:36) - PLID 36903 - filter on appts this year							
							//
							return _T("SELECT Count(PatsWithMultipleProcs) as CountofPatsMultipleProcs, (SELECT Count(PersonID) FROM PatientsT) AS TotalPats FROM "
								"  ( "
								" SELECT PatientID as PatsWithMultipleProcs FROM AppointmentsT  "
								" WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6)) "
								" AND AppointmentsT.ID IN "
								"    (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN "
								"        (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 4 "
								"         UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 4) "
								"        ) "
								"    ) "
								" AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4 "
								" AND YEAR(AppointmentsT.Date) = #INPUTYEAR "
								" GROUP BY Year(AppointmentsT.Date), PatientID "								
								" Having Count(AppointmentsT.ID) > 1 "
								" ) Q ");
						break;

						case 3:  //Percent Procedures By Locations
							//
							// 2006 form:
							// 4. In your opinion, which facial cosmetic surgical procedure is most performed among each
							// of the following racial groups.
							//
							// This references the configuration where the ID equals 5 (afsFacialCosmetic)
							//
							// Appointment categories: AC_MINOR (3), AC_SURGERY (4), AC_OTHER (6)
							//
							// (b.spivey, May 28, 2013) - PLID 56871 - Changed this logic to use the new data structure. 
							// (b.spivey, June 17, 2013) - PLID 56871 - Changed from blob/memo to nvarhcar(255) 
							return _T("SELECT 1 as ID, " 
									"	CAST((CASE WHEN RaceSubQ.RaceName IS NULL THEN 'None Specified' "
									"		ELSE LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) END) AS NVARCHAR(255)) AS EthnicityName, "
									"	CASE WHEN ProcedureT.MasterProcedureID IS NOT NULL THEN (SELECT Name FROM ProcedureT P2 WHERE P2.ID = ProcedureT.MasterProcedureID) ELSE ProcedureT.Name END AS ProcName  "
									"	FROM AppointmentsT "
									"	LEFT JOIN LocationsT ON AppointmentsT.LocationId = LocationsT.ID  "
									"	LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
									"	CROSS APPLY "
									"	( "
									"		SELECT ( "
									"			SELECT RT.Name + ', ' "
									"			FROM PersonRaceT PRT "
									"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
									"			WHERE PRT.PersonID = PersonT.ID "
									"			FOR XML PATH(''), TYPE "
									"		).value('/', 'nvarchar(max)') "
									"	) RaceSubQ (RaceName) "
									"	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
									"	LEFT JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID  "
									"	WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN  "
									"		(SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
									"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
									"		AND ( "
									"			AppointmentPurposeT.PurposeID IN ( "
									"				SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 5 "
									"				UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 5) "
									"			) "
									"		) ");

							// This is the old query that Jennie wrote
/*							return _T("SELECT 1 as ID, CASE WHEN RaceT.Name IS NULL THEN 'None Specified' ELSE RaceT.Name END AS EthnicityNam, "
								"  CASE WHEN ProcedureT.MasterProcedureID IS NOT NULL THEN (SELECT Name FROM ProcedureT P2 WHERE P2.ID = ProcedureT.MasterProcedureID) ELSE  "
								"  ProcedureT.Name END AS ProcName "
								"  FROM AppointmentsT LEFT JOIN LocationsT "
								" ON AppointmentsT.LocationId = LocationsT.ID "
								" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
								" LEFT JOIN RaceT ON PersonT.RaceID = RaceT.ID "
								" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
								" LEFT JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
								" WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6)) "
								" AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4 "
								" AND (AppointmentPurposeT.PurposeID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 5) OR "
								" AppointmentPurposeT.PurposeID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID IN  "
								" (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 5)))");*/
						break;

						case 4:  //Number of cosmetic surgical procedures performed
							//
							// 2006 form:
							// 5. Please estimate the total number of procedures you have performed in 2005
							//
							// This references the configuration where the ID equals 1 (afsCosmeticSurgical)
							//
							// Appointment categories: AC_MINOR (3), AC_SURGERY (4), AC_OTHER (6)
							//
							return _T("SELECT 'Cosmetic Surgical Procedures' AS Category, Count(AppointmentsT.ID) AS CountOfAppointments, "
								"	CASE WHEN PersonT.Gender = 1 THEN 'Male'  "
								"	WHEN PersonT.Gender = 2 THEN 'Female'  " 
								"	ELSE 'Unspecified' END AS Gender  "
								"FROM AppointmentsT "
								"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
								"WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"AND AppointmentsT.ID IN  "
								"    (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN  "
								"        (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 1 "
								"         UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 1)  "
								"        )  "
								"    )  "
								"GROUP BY PersonT.Gender "
								" "
								"UNION SELECT 'Cosmetic Non-Surgical Procedures' AS Category, Count(AppointmentsT.ID), "
								"	CASE WHEN PersonT.Gender = 1 THEN 'Male' " 
								"	WHEN PersonT.Gender = 2 THEN 'Female'  " 
								"	ELSE 'Unspecified' END AS Gender  "
								"FROM AppointmentsT "
								"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
								"WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"AND AppointmentsT.ID IN  "
								"    (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN  "
								"        (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 2 "
								"         UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 2)  "
								"        )  "
								"    )  "
								"GROUP BY PersonT.Gender "
								" "
								"UNION SELECT 'Reconstructive Surgical Procedures' AS Category, Count(AppointmentsT.ID), "
								"	CASE WHEN PersonT.Gender = 1 THEN 'Male'  "
								"	WHEN PersonT.Gender = 2 THEN 'Female'  " 
								"	ELSE 'Unspecified' END AS Gender  "
								"FROM AppointmentsT "
								"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
								"WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"AND AppointmentsT.ID IN  "
								"    (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN  "
								"        (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 3 "
								"         UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 3)  "
								"        ) "
								"    ) "
								"GROUP BY PersonT.Gender ");
							break;



						case 5: // Number of female patients having procedures done
							//
							// 2006 form:
							// 7. Please estimate the number of procedures, the percentage of female patients receiving each
							// procedure, and the average cost of procedures.
							//
							// * We are not filling out the average procedure cost; the client must do that *
							//
							// This references the configuration where the ID equals 6 (afsSurgicalProcedures) and
							// 7 (afsMinimallyInvasiveProcedures)
							//
							// Appointment categories: AC_MINOR (3), AC_SURGERY (4), AC_OTHER (6)
							//
							return _T("SELECT 'Surgical Procedures' AS Category, ProcName, Sum(NumPerformed) AS TotalPerformed, "
										"Sum(CASE WHEN Gender = 2 THEN NumPerformed ELSE 0 END) AS TotalFemalePerformed, "
										"convert(nvarchar, round(convert(real, Sum(CASE WHEN Gender = 2 THEN NumPerformed ELSE 0 END)) / convert(real, Sum(NumPerformed)) * 100.0, 2)) + '%' AS PercentFemale "
										"FROM ( "
										"	SELECT Count(AppointmentsT.ID) AS NumPerformed, PersonT.Gender,   "
										"		CASE WHEN ProcedureT.MasterProcedureID IS NOT NULL THEN (SELECT Name FROM ProcedureT P2 WHERE P2.ID = ProcedureT.MasterProcedureID) ELSE ProcedureT.Name END AS ProcName   "
										"		FROM AppointmentsT  "
										"		LEFT JOIN LocationsT ON AppointmentsT.LocationId = LocationsT.ID   "
										"		LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
										"		LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID   "
										"		LEFT JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID   "
										"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN   "
										"			(SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))   "
										"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4   "
										"			AND (  "
										"				AppointmentPurposeT.PurposeID IN (  "
										"					SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 6 "
										"					UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 6)  "
										"				)  "
										"			)  "
										"	GROUP BY Gender, ProcedureT.MasterProcedureID, ProcedureT.Name) SubQ "
										"GROUP BY ProcName "
										" "
										"UNION SELECT 'Minimally Invasive Procedures' AS Category, ProcName, Sum(NumPerformed) AS TotalPerformed, "
										"Sum(CASE WHEN Gender = 2 THEN NumPerformed ELSE 0 END) AS TotalFemalePerformed, "
										"convert(nvarchar, round(convert(real, Sum(CASE WHEN Gender = 2 THEN NumPerformed ELSE 0 END)) / convert(real, Sum(NumPerformed)) * 100.0, 2)) + '%' AS PercentFemale "
										"FROM ( "
										"	SELECT Count(AppointmentsT.ID) AS NumPerformed, PersonT.Gender,   "
										"		CASE WHEN ProcedureT.MasterProcedureID IS NOT NULL THEN (SELECT Name FROM ProcedureT P2 WHERE P2.ID = ProcedureT.MasterProcedureID) ELSE ProcedureT.Name END AS ProcName   "
										"		FROM AppointmentsT  "
										"		LEFT JOIN LocationsT ON AppointmentsT.LocationId = LocationsT.ID   "
										"		LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
										"		LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID   "
										"		LEFT JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID   "
										"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN   "
										"			(SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))   "
										"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4   "
										"			AND (  "
										"				AppointmentPurposeT.PurposeID IN (  "
										"					SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 7 "
										"					UNION SELECT ID FROM ProcedureT WHERE MasterProcedureID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = 7)  "
										"				)  "
										"			)  "
										"	GROUP BY Gender, ProcedureT.MasterProcedureID, ProcedureT.Name) SubQ "
										"GROUP BY ProcName ");
							break;


						case 6: // Percentage of patients by age group per procedure
							//
							// 2006 form:
							// 8. Please estimate the percentage of patients by age group for each procedure you performed in 2005
							//
							// This does not reference the configuration
							//
							// Appointment categories: AC_MINOR (3), AC_SURGERY (4), AC_OTHER (6)
							//

							return _T("SELECT Category, convert(nvarchar, round(AgeResult, 1)) + '%' AS AgeResult, "
								"CASE WHEN ProcedureT.MasterProcedureID IS NOT NULL THEN (SELECT Name FROM ProcedureT P2 WHERE P2.ID = ProcedureT.MasterProcedureID) ELSE ProcedureT.Name END AS ProcName "
								"FROM ( "
								"	SELECT 1 AS CatOrderIndex, '21 and Under' AS Category, "
								"	convert(real, PersonCount) * 100.0 / convert(real, (SELECT COUNT(ID) FROM PersonT WHERE ID IN (   "
								"		SELECT PatientID FROM AppointmentsT "
								"		LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"		AND AppointmentPurposeT.PurposeID = ProcedureID "
								"	))) AS AgeResult, "
								"	ProcedureID FROM ( "
								"		SELECT Count(PersonID) AS PersonCount, ProcedureID FROM  "
								"		( "
								"			SELECT PersonT.ID AS PersonID, ProcedureT.ID AS ProcedureID FROM AppointmentsT  "
								"			LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"			LEFT JOIN ProcedureT ON ProcedureT.ID = AppointmentPurposeT.PurposeID "
								"			LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
								"			WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								" "		 
								"			/* Person age is under or equal to 21 */  "
								"			AND (PersonT.Birthdate IS NOT NULL AND YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 "
								"				WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END <= 21) "
								" "
								"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"			AND ProcedureT.ID IS NOT NULL "
								"			GROUP BY ProcedureT.ID, PersonT.ID "
								"		) SubQ "
								"		GROUP BY ProcedureID "
								"	) BasicQ "
								" "
								"	UNION SELECT 2 AS CatOrderIndex, '22-40' AS Category, "
								"	convert(real, PersonCount) * 100.0 / convert(real, (SELECT COUNT(ID) FROM PersonT WHERE ID IN (   "
								"		SELECT PatientID FROM AppointmentsT "
								"		LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"		AND AppointmentPurposeT.PurposeID = ProcedureID "
								"	))) AS AgeResult, "
								"	ProcedureID FROM ( "
								"		SELECT Count(PersonID) AS PersonCount, ProcedureID FROM  "
								"		( "
								"			SELECT PersonT.ID AS PersonID, ProcedureT.ID AS ProcedureID FROM AppointmentsT  "
								"			LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"			LEFT JOIN ProcedureT ON ProcedureT.ID = AppointmentPurposeT.PurposeID "
								"			LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
								"			WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								" "		 
								"			/* Person age is between 22 and 40, inclusively */ "
								"			AND (PersonT.Birthdate IS NOT NULL AND "
								"				(YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END >= 22) AND "
								"				(YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END <= 40) "
								"			) "
								" "
								"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"			AND ProcedureT.ID IS NOT NULL "
								"			GROUP BY ProcedureT.ID, PersonT.ID "
								"		) SubQ "
								"		GROUP BY ProcedureID "
								"	) BasicQ "
								" "
								"	UNION SELECT 3 AS CatOrderIndex, '41-60' AS Category, "
								"	convert(real, PersonCount) * 100.0 / convert(real, (SELECT COUNT(ID) FROM PersonT WHERE ID IN (   "
								"		SELECT PatientID FROM AppointmentsT "
								"		LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"		AND AppointmentPurposeT.PurposeID = ProcedureID "
								"	))) AS AgeResult, "
								"	ProcedureID FROM ( "
								"		SELECT Count(PersonID) AS PersonCount, ProcedureID FROM  "
								"		( "
								"			SELECT PersonT.ID AS PersonID, ProcedureT.ID AS ProcedureID FROM AppointmentsT  "
								"			LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"			LEFT JOIN ProcedureT ON ProcedureT.ID = AppointmentPurposeT.PurposeID "
								"			LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
								"			WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								" "		 
								"			/* Person age is between 22 and 40, inclusively */ "
								"			AND (PersonT.Birthdate IS NOT NULL AND "
								"				(YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END >= 41) AND "
								"				(YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END <= 60) "
								"			) "
								" "
								"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"			AND ProcedureT.ID IS NOT NULL "
								"			GROUP BY ProcedureT.ID, PersonT.ID "
								"		) SubQ "
								"		GROUP BY ProcedureID "
								"	) BasicQ "
								" "
								"	UNION SELECT 4 AS CatOrderIndex, '61 or Over' AS Category, "
								"	convert(real, PersonCount) * 100.0 / convert(real, (SELECT COUNT(ID) FROM PersonT WHERE ID IN (   "
								"		SELECT PatientID FROM AppointmentsT "
								"		LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"		AND AppointmentPurposeT.PurposeID = ProcedureID "
								"	))) AS AgeResult, "
								"	ProcedureID FROM ( "
								"		SELECT Count(PersonID) AS PersonCount, ProcedureID FROM  "
								"		( "
								"			SELECT PersonT.ID AS PersonID, ProcedureT.ID AS ProcedureID FROM AppointmentsT  "
								"			LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"			LEFT JOIN ProcedureT ON ProcedureT.ID = AppointmentPurposeT.PurposeID "
								"			LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
								"			WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								" "		 
								"			/* Person age is 61 or over, inclusively */ "
								"			AND (PersonT.Birthdate IS NOT NULL AND "
								"				(YEAR(GETDATE()) - YEAR(PersonT.Birthdate) - CASE WHEN MONTH(PersonT.Birthdate) > MONTH(getdate()) THEN 1 WHEN MONTH(PersonT.Birthdate) < MONTH(getdate()) THEN 0 WHEN DAY(PersonT.Birthdate) > DAY(PersonT.Birthdate) THEN 1 ELSE 0 END >= 61) "
								"			) "
								" "
								"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"			AND ProcedureT.ID IS NOT NULL "
								"			GROUP BY ProcedureT.ID, PersonT.ID "
								"		) SubQ "
								"		GROUP BY ProcedureID "
								"	) BasicQ "
								" "
								"	UNION SELECT 5 AS CatOrderIndex, 'Unspecified' AS Category, "
								"	convert(real, PersonCount) * 100.0 / convert(real, (SELECT COUNT(ID) FROM PersonT WHERE ID IN (   "
								"		SELECT PatientID FROM AppointmentsT "
								"		LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"		WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								"		AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"		AND AppointmentPurposeT.PurposeID = ProcedureID "
								"	))) AS AgeResult, "
								"	ProcedureID FROM ( "
								"		SELECT Count(PersonID) AS PersonCount, ProcedureID FROM  "
								"		( "
								"			SELECT PersonT.ID AS PersonID, ProcedureT.ID AS ProcedureID FROM AppointmentsT  "
								"			LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
								"			LEFT JOIN ProcedureT ON ProcedureT.ID = AppointmentPurposeT.PurposeID "
								"			LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
								"			WHERE YEAR(AppointmentsT.Date) = #INPUTYEAR AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6))  "
								" "		 
								"			/* Person has no age */ "
								"			AND (PersonT.Birthdate IS NULL) "
								" "
								"			AND AppointmentsT.ShowState <> 3 AND AppointmentsT.Status <> 4  "
								"			AND ProcedureT.ID IS NOT NULL "
								"			GROUP BY ProcedureT.ID, PersonT.ID "
								"		) SubQ "
								"		GROUP BY ProcedureID "
								"	) BasicQ "
								") DataQ "
								"LEFT JOIN ProcedureT ON ProcedureT.ID = ProcedureID "
								"ORDER BY CatOrderIndex ");
							break;

					}
				break;

				}

		/*// (j.jones 2010-01-15 13:57) - PLID 36898 - this report was removed from Practice
		case 688: //Patient Encounter Percentages With E-Eligibility Confirmed

			// (j.jones 2009-12-18 10:17) - PLID 35773 - created
			// This report shows all non-cancelled, non-no-show appts. of all types (it's a filter),
			// and sets HasPriorConfirmedEligibility to TRUE if there is a confirmed eligibility response
			// for that patient at any time on that appointment date or any date in the past that is also AFTER
			// their last unique appointment date. (ie. two appts. on the same day can share the same response,
			// provided it came a day or more after the previous date they had an appointment.)

			return _T("SELECT AppointmentsT.Date AS Date, AppointmentsT.LocationID AS LocID, AppointmentsT.PatientID AS PatID, "
				"AppointmentsT.AptTypeID AS AptTypeID, AptTypeT.Name AS AptTypeName, "
				"CASE WHEN AppointmentsT.PatientID IN (SELECT PatientID FROM EligibilityRequestsT "
					"INNER JOIN EligibilityResponsesT ON EligibilityRequestsT.ID = EligibilityResponsesT.RequestID "
					"INNER JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
					"WHERE EligibilityResponsesT.ConfirmationStatus = 1 "
					//the response has to have been received on the same day as this appointment or earlier
					"AND dbo.AsDateNoTime(EligibilityResponsesT.DateReceived) <= dbo.AsDateNoTime(AppointmentsT.Date) "
					//the response has to have been received a day after the patient's last unique appt. date
					//(which is 1/1/1900 if no previous appt. exists - sorry, we don't support ANSI eligibility
					//requests received during the industrial revolution)
					"AND dbo.AsDateNoTime(EligibilityResponsesT.DateReceived) > "
						"Coalesce((SELECT Max(Date) AS LastDate "
						"FROM AppointmentsT AppointmentsQ "
						"WHERE AppointmentsQ.Status <> 4 AND AppointmentsQ.ShowState <> 3 "
						"AND AppointmentsQ.PatientID = AppointmentsT.PatientID "
						"AND dbo.AsDateNoTime(AppointmentsQ.Date) < dbo.AsDateNoTime(AppointmentsT.Date)), Convert(datetime, '1900/1/1')) "
					") THEN 'Confirmed' ELSE 'Not Confirmed' END "
					"AS HasPriorConfirmedEligibility, "
				"LocationsT.Name AS LocName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"PatientsT.UserDefinedID "
				"FROM AppointmentsT "
				"INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND "
				"AppointmentsT.PatientID > 0 ");
			break;
		*/

		//case 690:
			/*Version History - Patient EMN Percentages By EMR Data Code
			 (j.gruber 2009-12-23 11:17) - PLID 35771 - create for
			 (j.gruber 2009-12-23 16:45) - PLID 35767 - added educational resource code to filters here and reportinfocallback
			 (j.gruber 2009-12-23 16:46) - PLID 35758 - added clinical summary code to filters here and in ric
			 // (j.gruber 2010-01-18 10:05) - PLID 36929 - remove report
			*/
			/*return _T(" SELECT PersonT.ID as PatID, PersonProvT.ID as ProvID, EMRDataCodesT.ID as EMRDataCodeID, LocationsT.ID as LocID, "
			" EMRMasterT.ID as EMNID, EMRMasterT.Date as Date, "
			" PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			" EMRDataCodesT.Code, EMRDataCodesT.Description, LocationsT.Name as LocationName,  "
			" PersonProvT.First as ProvFirst, PersonProvT.Middle as ProvMiddle, PersonProvT.Last as ProvLast, "
			" CASE WHEN EMRSelectT.ID IS NULL THEN 'Not Selected' else 'Selected' END as IsSelected "
			" FROM "
			" EMRInfoT INNER JOIN (SELECT * FROM EMRDataCodesT WHERE Code IN ('REPORT_Transitions', 'REPORT_ClinicalSummary', 'REPORT_EdResources')) EMRDataCodesT "
			" ON EMRInfoT.DataCodeID = EMRDataCodesT.ID "
			" INNER JOIN EMRDetailsT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			" INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
			" LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID "
			" LEFT JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" LEFT JOIN EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
			" LEFT JOIN ProvidersT ON EMRProvidersT.ProviderID = ProvidersT.PersonID  "
			" LEFT JOIN PersonT PersonProvT ON ProvidersT.PersonID = PersonProvT.Id "
			" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
			" WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 and EMRInfoT.DataType IN (2,3)");*/
		//break;

		/*/ (j.jones 2010-01-15 13:57) - PLID 36898 - this report was removed from Practice
		case 691: //Patient Lab Result Percentages With Details
			
			// (j.jones 2009-12-31 10:37) - PLID 35770 - created
			return _T("SELECT LabsT.ID AS LabID, LabResultsT.ResultID, "
				"PersonT.ID AS PatID, LabsT.LocationID AS LocID, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"LocationsT.Name AS LocName, "
				"LabsT.FormNumberTextID, "
				"LabsT.BiopsyDate AS TDate, LabsT.InputDate AS IDate, "
				"LabResultsT.SlideTextID, "
				"LabResultsT.DiagnosisDesc, LabResultsT.ClinicalDiagnosisDesc, "
				"LabResultsT.DateReceived, LabResultFlagsT.Name AS FlagName, "
				"   LabResultsT.Name as ResultName, "
				"   LabResultsT.DateReceived as ResultReceived, "
				"   LabResultsT.Value as ResultValue, "
				"	LabResultsT.Units as ResultUnits, "
				"   LabResultsT.Reference as ResultReference, "
				"	LabResultStatusT.Description AS ResultStatus, "
				"	LabResultsT.Comments AS ResultComments, "
				""
				"CASE WHEN LabResultsT.Value Is Null OR SUBSTRING(LabResultsT.Value, 1, 100) = '' THEN 0 ELSE 1 END AS HasValue, "
				"CASE WHEN LabResultsT.Units = '' THEN 0 ELSE 1 END AS HasUnits, "
				"CASE WHEN LabResultsT.SlideTextID = '' THEN 0 ELSE 1 END AS HasSlideID, "
				"CASE WHEN LabResultsT.DiagnosisDesc Is Null OR SUBSTRING(LabResultsT.DiagnosisDesc, 1, 100) = '' THEN 0 ELSE 1 END AS HasDiagnosis, "
				"CASE WHEN LabResultsT.ClinicalDiagnosisDesc Is Null OR SUBSTRING(LabResultsT.ClinicalDiagnosisDesc, 1, 100) = '' THEN 0 ELSE 1 END AS HasClinicalDiagnosis, "
				"CASE WHEN LabResultsT.FlagID Is Null THEN 0 ELSE 1 END AS HasFlag, "
				"CASE WHEN LabResultsT.Reference = '' THEN 0 ELSE 1 END AS HasResultReference, "
				"CASE WHEN LabResultsT.HL7MessageID Is Null THEN 0 ELSE 1 END AS IsFromHL7, "
				"CASE WHEN LabResultsT.StatusID Is Null THEN 0 ELSE 1 END AS HasStatus "
				"FROM LabsT "
				"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
				"INNER JOIN PersonT ON LabsT.PatientID = PersonT.ID "
				"LEFT JOIN LabResultFlagsT ON LabResultsT.FlagID = LabResultFlagsT.ID "
				"LEFT JOIN LabResultStatusT ON LabResultsT.StatusID = LabResultStatusT.ID "
				"LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
				" "
				"WHERE LabsT.Deleted = 0 AND LabResultsT.Deleted = 0");
			break;
		*/
	
	case 705:	//Reward Point Redemption List

		// (j.jones 2011-04-22 09:46) - PLID 42341 - created

		return _T("SELECT ServiceT.ID AS ServiceID, "
			"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Codes' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Items' ELSE '' END AS Type, "
			"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 0 WHEN ProductT.ID IS NOT NULL THEN 1 ELSE -1 END AS TypeID, "
			"CPTCodeT.Code, CPTCodeT.Subcode, ServiceT.Name AS Name, CategoriesT.ID AS CatID, CategoriesT.Name AS Category, "
			"Convert(float, ServiceT.PointCost) AS PointCost, "
			"RewardDiscountsT.DiscountPercent, RewardDiscountsT.DiscountDollars, ServiceT.Price AS Price "
			"FROM ServiceT "
			"LEFT JOIN RewardDiscountsT ON ServiceT.ID = RewardDiscountsT.ServiceID "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE ServiceT.ID NOT IN (SELECT ServiceID FROM GCTypesT) "
			"AND ServiceT.ID NOT IN (SELECT ID FROM AdministrativeFeesT)");
	
	case 710:	//Deleted PracYakker Messages by Sender
	
		//(c.copits 2011-07-12) PLID 17459 - Have a report where an administrator can view the deleted pracyakker messages
		
		/*  Version History
			c.copits 7/12/11 - Created.
			TES 2/20/2015 - PLID 64223 - Changed to include messages deleted by the sender OR recipient, rather than AND
		*/
		return _T("SELECT MessageID, "
			"CASE WHEN RegardingID = -1 THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Regarding,  "
			"CASE WHEN Priority = 1 THEN 'Low' WHEN Priority = 2 THEN 'Medium' WHEN Priority = 3 THEN 'High' END AS Priority, "
			"DateSent AS Date, SenderQ.Username AS Sender, RecipientQ.Username AS Recipient, Text, "
			"MessageGroupID, SenderQ.PersonID AS UserID  "
			"FROM MessagesT LEFT JOIN PersonT ON RegardingID = PersonT.ID LEFT JOIN UsersT SenderQ ON SenderID = SenderQ.PersonID "
			"LEFT JOIN UsersT RecipientQ ON RecipientID = RecipientQ.PersonID "
			"WHERE DeletedBySender = 1 OR DeletedByRecipient = 1");
		break;
         // (a.levy 2012-09-27 15:59) - PLID 52903 - Modified for Detailed report of Sales productivity report
		/*(a.levy 2012-09-06 15:53) - PLID 52490- Create Sales Productivity Report - Version 1
                 -Show resources and show how much "Sales Internet Demo" they have scheduled. -"Appointments" .
                 Additionally
                 -Construct an Even Details Scheduled List -
                 -Appointment Details Scheduled (Detailed)
				 /* Version History
				  a.levy 9/06/2012 -updated for Event Name
				  a.levy 9/06/2012 - Created
				 
				 */
	case 741:

		return _T("SELECT AppointmentsT.ID AS ApptID,PatientID AS PatID,Item AS ResourceName,CreatedDate AS CreateDate, [Date] AS ScheduledDate, AptTypeID AS [Event],AptTypeT.NAME AS EventName,[First] + ' ' + [Last] AS OfficeContact,Company,Notes FROM PersonT "
                  "INNER JOIN "
                  "AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
                  "INNER JOIN " 
                  "AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
                  "INNER JOIN "
                  "ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
                  "INNER JOIN   AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
                  "WHERE CancelledDate IS NULL AND ResourceT.Inactive <> 1 " ); 
		break;
         
	default:
		//no report was found with that id
		return _T(""); 
		break;
	}


}
