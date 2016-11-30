#include "stdafx.h"
#include "OpportunityProposalDlg.h"
#include "ProposalPricing.h"

// (d.lange 2010-08-24 11:27) - PLID 38029 - created
CProposalPricing::CProposalPricing(CArray<COleCurrency, COleCurrency> &aryPrices, CArray<int, int> &aryModules)
{
	m_aryPrices.Copy(aryPrices);
	m_aryModules.Copy(aryModules);
}

CProposalPricing::~CProposalPricing(void)
{
}

//////////////////// Get total functions
COleCurrency CProposalPricing::GetPMPkgTotal()
{
	COleCurrency cyTotal(0, 0);

	if(m_aryModules[ecmStdScheduler]){
		cyTotal += (m_aryModules[ecmScheduler] ? m_aryPrices[ecpSchedulerStandard] : COleCurrency (0, 0));
	}else{
		cyTotal += (m_aryModules[ecmScheduler] ? m_aryPrices[ecpScheduler] : COleCurrency(0, 0));
	}

	return cyTotal;
}

COleCurrency CProposalPricing::GetPMTotal()
{
	COleCurrency cyTotal(0, 0);

	if(m_aryModules[ecmStdScheduler]) {
		cyTotal += (m_aryModules[ecmScheduler] ? m_aryPrices[ecpSchedulerStandard] : COleCurrency (0, 0));
	}else{
		cyTotal += (m_aryModules[ecmScheduler] ? m_aryPrices[ecpScheduler] : COleCurrency (0, 0));
	}

	cyTotal += (m_aryModules[ecm1License] ? m_aryPrices[ecp1License] : COleCurrency (0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetSchedulerPrice()
{
	return m_aryPrices[ecpScheduler];
}

//This calculates based on the Finanical Package price, relates to merge field "Internal_Prop_Fin_Package"
COleCurrency CProposalPricing::GetFinancialPkgTotal()
{
	COleCurrency cyTotal(0, 0);

	if(m_aryModules[ecmBilling] && m_aryModules[ecmHCFA] && m_aryModules[ecmEBilling] && m_aryModules[ecmERemittance]
			&& m_aryModules[ecmEEligibility]){
		cyTotal += m_aryPrices[ecpPkgFinancial];
	}else{
		cyTotal += (m_aryModules[ecmBilling] ? m_aryPrices[ecpBilling] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmHCFA] ? m_aryPrices[ecpHCFA] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmEBilling] ? m_aryPrices[ecpEBilling] : COleCurrency(0, 0));
		// (d.lange 2010-12-03 09:59) - PLID 41711 - Moved ERemittance and EEligibility to financial area
		cyTotal += (m_aryModules[ecmERemittance] ? m_aryPrices[ecpERemittance] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmEEligibility] ? m_aryPrices[ecpEEligibility] : COleCurrency(0, 0));
	}

	return cyTotal;
}

//This calculates total individual Finanical Package based on the type of proposal, relates to merge field "Internal_Prop_Fin_Line"
COleCurrency CProposalPricing::GetIndividualFinTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmBilling] ? m_aryPrices[ecpBilling] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmHCFA] ? m_aryPrices[ecpHCFA] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmEBilling] ? m_aryPrices[ecpEBilling] : COleCurrency(0, 0));
	// (d.lange 2010-12-03 09:59) - PLID 41711 - Moved ERemittance and EEligibility to financial area
	cyTotal += (m_aryModules[ecmERemittance] ? m_aryPrices[ecpERemittance] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmEEligibility] ? m_aryPrices[ecpEEligibility] : COleCurrency(0, 0));

	return cyTotal;
}

//This calculates based on the Cosmetic Package price, relates to merge field "Internal_Prop_Cosm_Package"
COleCurrency CProposalPricing::GetCosmeticPkgTotal(long nMergeTypeID)
{
	COleCurrency cyTotal(0, 0);

	if(m_aryModules[ecmLetterWriting] && m_aryModules[ecmQuotesMarketing] && m_aryModules[ecmNexTrak] == 1 && m_aryModules[ecmNexForms] == 1) {
		cyTotal += m_aryPrices[ecpPkgCosmetic];
	}else{
		cyTotal += (m_aryModules[ecmLetterWriting] ? m_aryPrices[ecpLetters] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmQuotesMarketing] ? m_aryPrices[ecpQuotes] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmNexTrak] ? m_aryPrices[ecpTracking] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmNexForms] ? m_aryPrices[ecpNexForms] : COleCurrency(0, 0));
	}
	if((nMergeTypeID == eptNexRes || nMergeTypeID == eptNexStartup) && m_aryModules[ecmHCFA] == 0 && m_aryModules[ecmLetterWriting] == 1) {
		cyTotal -= m_aryPrices[ecpLetters];
	}
	return cyTotal;
}

//This calculates total individual Cosmetic Package based on the type of proposal, relates to merge field "Internal_Prop_Cosm_Line"
COleCurrency CProposalPricing::GetIndividualCosTotal(long nMergeTypeID)
{
	COleCurrency cyTotal(0, 0);

	if((nMergeTypeID == eptNexRes || nMergeTypeID == eptNexStartup) && m_aryModules[ecmHCFA] == 0){
		cyTotal += COleCurrency(0, 0);
	}else{
		cyTotal += (m_aryModules[ecmLetterWriting] ? m_aryPrices[ecpLetters] : COleCurrency(0, 0));
	}

	cyTotal += (m_aryModules[ecmQuotesMarketing] ? m_aryPrices[ecpQuotes] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmNexTrak] ? m_aryPrices[ecpTracking] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmNexForms] ? m_aryPrices[ecpNexForms] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetOtherTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmInventory] ? m_aryPrices[ecpInventory] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmCandA] ? m_aryPrices[ecpCandA] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmNexSpa] ? m_aryPrices[ecpSpa] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmASC] ? m_aryPrices[ecpASC] : COleCurrency(0, 0));
	// (d.lange 2010-12-03 09:59) - PLID 41711 - Moved ERemittance and EEligibility to financial area
	/*cyTotal += (m_aryModules[ecmERemittance] ? m_aryPrices[ecpERemittance] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmEEligibility] ? m_aryPrices[ecpEEligibility] : COleCurrency(0, 0));*/
	cyTotal += (m_aryModules[ecmNexPhoto] ? m_aryPrices[ecpNexPhoto] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetNexWebPkgTotal()
{
	COleCurrency cyTotal(0, 0);

	if(m_aryModules[ecmLeadGen] && m_aryModules[ecmPatientPortal]) {
		cyTotal += m_aryPrices[ecpPkgNexWeb];
	}else{
		cyTotal += (m_aryModules[ecmLeadGen] ? m_aryPrices[ecpNexWebLeads] : COleCurrency(0, 0));
		cyTotal += (m_aryModules[ecmPatientPortal] ? m_aryPrices[ecpNexWebPortal] : COleCurrency(0, 0)); 
	}
	return cyTotal;
}

COleCurrency CProposalPricing::GetIndividualNexWebTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmLeadGen] ? m_aryPrices[ecpNexWebLeads] : COleCurrency(0, 0));
	cyTotal += (m_aryModules[ecmPatientPortal] ? m_aryPrices[ecpNexWebPortal] : COleCurrency(0, 0)); 

	return cyTotal;
}

COleCurrency CProposalPricing::GetEMRTotal(int nMultipleEMR)
{
	COleCurrency cyFirstTotal(0, 0), cyAddtlTotal(0, 0);

	if(m_aryModules[ecmEMR] >= 1) {
		if(m_aryModules[ecmStdEMR]) {
			cyFirstTotal += m_aryPrices[ecpEMRFirstStandard];
		}
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic emr
		else if(m_aryModules[ecmCosmeticEMR]) {
			cyFirstTotal += m_aryPrices[ecpEMRCosmetic];
		}
		else {
			cyFirstTotal += m_aryPrices[ecpEMRFirstDoctor];
		}
	}

	if(m_aryModules[ecmEMR] > 1) {
		if(m_aryModules[ecmStdEMR]) {
			cyAddtlTotal += ((m_aryModules[ecmEMR] - 1) * m_aryPrices[ecpEMRAddtlStandard]);
		}
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic emr is same price for all docs
		else if(m_aryModules[ecmCosmeticEMR]) {
			cyAddtlTotal += ((m_aryModules[ecmEMR] - 1) * m_aryPrices[ecpEMRCosmetic]);
		}
		else {
			cyAddtlTotal += ((m_aryModules[ecmEMR] - 1) * m_aryPrices[ecpEMRAddtlDoctor]);
		}
	}
	if(nMultipleEMR) {
		return cyAddtlTotal;
	}else{
		return cyFirstTotal;
	}
}

COleCurrency CProposalPricing::GetPDATotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal +=  (m_aryModules[ecmPDA] * m_aryPrices[ecpPDA]);

	return cyTotal;
}

COleCurrency CProposalPricing::GetNexSyncTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmNexSync] * m_aryPrices[ecpNexSync]);

	return cyTotal;
}

COleCurrency CProposalPricing::GetWorkstationPrice()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += m_aryPrices[ecpWorkstation];

	return cyTotal;
}

COleCurrency CProposalPricing::GetMultiDoctorPrice()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += m_aryPrices[ecpDoctor];

	return cyTotal;
}

// Relates to the merge field "Internal_Prop_Inv_Cost"
COleCurrency CProposalPricing::GetInventoryTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmInventory] ? m_aryPrices[ecpInventory] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_CandA_Cost"
COleCurrency CProposalPricing::GetCandATotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmCandA] ? m_aryPrices[ecpCandA] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_Spa_Cost"
COleCurrency CProposalPricing::GetNexSpaTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmNexSpa] ? m_aryPrices[ecpSpa] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_NexPhoto_Cost"
COleCurrency CProposalPricing::GetNexPhotoTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmNexPhoto] ? m_aryPrices[ecpNexPhoto] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_ASC_Cost"
COleCurrency CProposalPricing::GetASCTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmASC] ? m_aryPrices[ecpASC] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_ERemit_Cost"
COleCurrency CProposalPricing::GetERemittanceTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmERemittance] ? m_aryPrices[ecpERemittance] : COleCurrency (0, 0));
	
	return cyTotal;
}

// Relates to the merge field "Internal_Prop_EElig_Cost"
COleCurrency CProposalPricing::GetEEligibilityTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmEEligibility] ? m_aryPrices[ecpEEligibility] : COleCurrency (0, 0));
	
	return cyTotal;
}

COleCurrency CProposalPricing::GetMirrorTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmMirror] ? m_aryPrices[ecpMirror] : COleCurrency (0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetUnitedTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmUnited] ? m_aryPrices[ecpUnited] : COleCurrency (0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetHL7Total()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmHL7] ? m_aryPrices[ecpHL7] : COleCurrency (0, 0));

	return cyTotal;
}

// (d.thompson 2013-07-08) - PLID 57334 - HIE
COleCurrency CProposalPricing::GetHIETotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmHIE] ? m_aryPrices[ecpHIE] : COleCurrency (0, 0));

	return cyTotal;
}

CString CProposalPricing::GetInformQuickbooksString(CString strInformPrice, CString strQuickbooksPrice)
{
	CString strSecretText;

	if(m_aryModules[ecmInform] == 1) {
		strSecretText = FormatString("Software Link to Inform ($%s)", strInformPrice);
	}
	if(m_aryModules[ecmQuickBooks] == 1) {
		if(!strSecretText.IsEmpty()) {
			strSecretText += ", ";
		}

		strSecretText += FormatString("Software Link to Quickbooks ($%s)", strQuickbooksPrice);
	}

	return strSecretText;
}

COleCurrency CProposalPricing::GetInformQuickbooksTotal()
{
	COleCurrency cySecretTotal(0, 0);

	cySecretTotal += (m_aryModules[ecmInform] == 1 ? m_aryPrices[ecpInform] : COleCurrency(0, 0));
	cySecretTotal += (m_aryModules[ecmQuickBooks] == 1 ? m_aryPrices[ecpQuickbooks] : COleCurrency(0, 0));
	return cySecretTotal;
}

// (d.thompson 2010-12-29) - PLID 41948 - We are no longer using Addtl / First nomenclature.  However we aren't
//	adding in a "special" price.  So all is still calculated by using the pricing for first.
COleCurrency CProposalPricing::GetEMRAllBasePrice()
{
	//Since we're not adding a separate price, just treat the "first" price as the "all" price.
	return GetEMRFirstBasePrice();
}

COleCurrency CProposalPricing::GetEMRAddtlBasePrice()
{
	COleCurrency cyAddtlTotal(0, 0);
	// (d.thompson 2013-07-05) - PLID 57333 - added cosmetic emr
	cyAddtlTotal += (m_aryModules[ecmStdEMR] ? m_aryPrices[ecpEMRAddtlStandard] : 
		(m_aryModules[ecmCosmeticEMR] ? m_aryPrices[ecpEMRCosmetic] : m_aryPrices[ecpEMRAddtlDoctor]));

	return cyAddtlTotal;
}

COleCurrency CProposalPricing::GetEMRFirstBasePrice()
{
	COleCurrency cyFirstTotal(0, 0);
	// (d.thompson 2013-07-05) - PLID 57333 - added cosmetic emr
	cyFirstTotal += (m_aryModules[ecmStdEMR] ? m_aryPrices[ecpEMRFirstStandard] : 
		(m_aryModules[ecmCosmeticEMR] ? m_aryPrices[ecpEMRCosmetic] : m_aryPrices[ecpEMRFirstDoctor]));

	return cyFirstTotal;
}

COleCurrency CProposalPricing::GetInformTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmInform] ? m_aryPrices[ecpInform] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetQuickbooksTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmQuickBooks] ? m_aryPrices[ecpQuickbooks] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::GetTravelTotal(long nTravelType)
{
	COleCurrency cyTotal(0, 0);
	// (d.lange 2010-12-22 10:17) - PLID 41889 - Support additional travel rules
	switch(nTravelType) {
		case ettDomestic:
			cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravel]);
			break;
		case ettCanada:
			cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravelCanada]);
			break;
		case ettInternational:
			cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravelInternational]);
			break;
		case ettNewYorkCity:
			cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravelNewYorkCity]);
			break;
		default:
			cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravel]);
			break;
	}

	return cyTotal;
}

// (d.lange 2010-09-21 12:16) - PLID 40361 - Added EMR & Financial Conversion
COleCurrency CProposalPricing::GetConversionTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmConversion] * m_aryPrices[ecpConversion]);
	cyTotal += (m_aryModules[ecmEmrConversion] * m_aryPrices[ecpEmrConversion]);
	cyTotal += (m_aryModules[ecmFinancialConversion] * m_aryPrices[ecpFinancialConversion]);
	
	return cyTotal;
}

COleCurrency CProposalPricing::GetTrainingPrice()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += m_aryPrices[ecpTraining];

	return cyTotal;
}

COleCurrency CProposalPricing::GetCosmeticPkgBasePrice()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += m_aryPrices[ecpPkgCosmetic];

	return cyTotal;
}

COleCurrency CProposalPricing::GetAddtlDomainTotal()
{
	COleCurrency cyTotal(0, 0);
	cyTotal += (m_aryModules[ecmAddDomains] * m_aryPrices[ecpNexWebAddtlDomains]);

	return cyTotal;
}

COleCurrency CProposalPricing::GetExtraDBTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmExtraDB] * m_aryPrices[ecpExtraDB]);

	return cyTotal;
}

COleCurrency CProposalPricing::GetSupportMonthlyTotal(long nMergeTypeID)
{
	COleCurrency cySupportTotal = GetListSubTotal(nMergeTypeID);

	if(nMergeTypeID == eptNexStartup || nMergeTypeID == eptNexRes) {
		cySupportTotal -= m_aryPrices[ecpScheduler];
		cySupportTotal -= m_aryPrices[ecp1License];
		cySupportTotal -= m_aryPrices[ecpBilling];

		if(m_aryModules[ecmHCFA]) {
			cySupportTotal -= m_aryPrices[ecpHCFA];
		}else if(m_aryModules[ecmLetterWriting]) {
			cySupportTotal -= m_aryPrices[ecpLetters];
		}

		cySupportTotal -= m_aryPrices[ecpWorkstation];
	}

	COleCurrency cyMonthlySupport = (((double)m_aryModules[ecmSupportPercent] / 100) * cySupportTotal) / long(12);

	__int64 nTotal = cyMonthlySupport.m_cur.int64;
	//the value is set 1/10,000th's
	__int64 nCents = nTotal % 10000;	//For example, 66 cents would be 6600
	__int64 nDollars = nTotal / 10000;

	if(nCents != 0)
		cyMonthlySupport.SetCurrency((long)(nDollars + 1), 0);

	return cyMonthlySupport;
}

COleCurrency CProposalPricing::GetPrePkgTotal()
{
	COleCurrency cySubTotal(0, 0);

	// (d.thompson 2012-10-23) - PLID 53155 - Added TS workstation qty.  Same price as regular workstations
	// (d.thompson 2013-07-09) - PLID 57334 - HIE
	cySubTotal += (GetOtherTotal() + ((m_aryModules[ecmWorkstations] + GetEMRWorkstationQty() + GetWorkstationTSQty()) * GetWorkstationPrice()) + 
		(GetMultiDoctorQty() * GetMultiDoctorPrice()) + GetPDATotal() + GetNexSyncTotal() + GetMirrorTotal() + 
		GetUnitedTotal() + GetHL7Total() + GetHIETotal() + GetInformQuickbooksTotal() + GetEMRTotal(embFirstEMR) + GetEMRTotal(embAddtlEMR) +
		GetAddtlDomainTotal());

	return cySubTotal;
}

COleCurrency CProposalPricing::GetSubTotal(long nMergeTypeID)
{
	COleCurrency cySubTotal(0, 0);
	COleCurrency cyListTotal(0, 0);

	cySubTotal += GetPrePkgTotal();

	cySubTotal += (GetPMPkgTotal() + GetNexWebPkgTotal());

	if((nMergeTypeID == eptNexRes || nMergeTypeID == eptNexStartup) && m_aryModules[ecmHCFA] == 0 && m_aryModules[ecmLetterWriting] == 1) {
		cySubTotal += (m_aryModules[ecmLetterWriting] ? m_aryPrices[ecpLetters] : COleCurrency (0, 0));
		cySubTotal += (m_aryModules[ecmQuotesMarketing] ? m_aryPrices[ecpQuotes] : COleCurrency (0, 0));
		cySubTotal += (m_aryModules[ecmNexTrak] ? m_aryPrices[ecpTracking] : COleCurrency (0, 0));
		cySubTotal += (m_aryModules[ecmNexForms] ? m_aryPrices[ecpNexForms] : COleCurrency (0, 0));
	}else{
		cySubTotal += GetCosmeticPkgTotal(nMergeTypeID);
	}

	if(nMergeTypeID == eptNexRes || nMergeTypeID == eptNexStartup) {
		cySubTotal += (m_aryModules[ecmBilling] ? m_aryPrices[ecpBilling] : COleCurrency (0, 0));
		cySubTotal += (m_aryModules[ecmHCFA] ? m_aryPrices[ecpHCFA] : COleCurrency (0, 0));
		cySubTotal += (m_aryModules[ecmEBilling] ? m_aryPrices[ecpEBilling] : COleCurrency (0, 0));
	}else{
		cySubTotal += GetFinancialPkgTotal();
	}

	return cySubTotal;
}

COleCurrency CProposalPricing::GetListSubTotal(long nMergeTypeID)
{
	COleCurrency cyListSubTotal(0, 0);

	cyListSubTotal += GetPrePkgTotal();

	//Now calculate the subtotal without package price

	cyListSubTotal += GetPMTotal();
	cyListSubTotal += (m_aryModules[ecmBilling] ? m_aryPrices[ecpBilling] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmHCFA] ? m_aryPrices[ecpHCFA] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmEBilling] ? m_aryPrices[ecpEBilling] : COleCurrency (0, 0));
	// (d.lange 2010-12-07 16:50) - PLID 41711 - Moved E-Remittance and E-Eligibility to financial
	cyListSubTotal += (m_aryModules[ecmERemittance] ? m_aryPrices[ecpERemittance] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmEEligibility] ? m_aryPrices[ecmEEligibility] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmLetterWriting] ? m_aryPrices[ecpLetters] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmQuotesMarketing] ? m_aryPrices[ecpQuotes] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmNexTrak] ? m_aryPrices[ecpTracking] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmNexForms] ? m_aryPrices[ecpNexForms] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmLeadGen] ? m_aryPrices[ecpNexWebLeads] : COleCurrency (0, 0));
	cyListSubTotal += (m_aryModules[ecmPatientPortal] ? m_aryPrices[ecpNexWebPortal] : COleCurrency (0, 0));

	return cyListSubTotal;
}

COleCurrency CProposalPricing::GetGrandTotal(long nMergeTypeID, long nTravelType, COleCurrency cySupportTotal, COleCurrency cyDiscount)
{
	COleCurrency cyGrandTotal(0, 0);

	cyGrandTotal += GetSubTotal(nMergeTypeID);

	cyGrandTotal -= cyDiscount;
	
	cyGrandTotal += ((GetPMTrainingQty(nMergeTypeID) + GetEMRTrainingQty()) * GetTrainingPrice());
	cyGrandTotal += cySupportTotal;
	cyGrandTotal += GetConversionTotal();
	cyGrandTotal += GetTravelTotal(nTravelType);	// (d.lange 2010-12-22 10:17) - PLID 41889 - Support additional travel rules
	cyGrandTotal += GetExtraDBTotal();

	return cyGrandTotal;
}

// Get quantity of module
int CProposalPricing::GetInventoryQty()
{
	return m_aryModules[ecmInventory];
}

int CProposalPricing::GetCandAQty()
{
	return m_aryModules[ecmCandA];
}

int CProposalPricing::GetNexSpaQty()
{
	return m_aryModules[ecmNexSpa];
}

int CProposalPricing::GetNexPhotoQty()
{
	return m_aryModules[ecmNexPhoto];
}

int CProposalPricing::GetASCQty()
{
	return m_aryModules[ecmASC];
}

int CProposalPricing::GetERemittanceQty()
{
	return m_aryModules[ecmERemittance];
}

int CProposalPricing::GetEEligibilityQty()
{
	return m_aryModules[ecmEEligibility];
}

int CProposalPricing::GetEMRWorkstationQty()
{
	return m_aryModules[ecmEMRWorkstations];
}

int CProposalPricing::GetMultiDoctorQty()
{
	return m_aryModules[ecmMultiDoctor];
}

int CProposalPricing::GetPDAQty()
{
	return m_aryModules[ecmPDA];
}

int CProposalPricing::GetNexSyncQty()
{
	return m_aryModules[ecmNexSync];
}

int CProposalPricing::GetMirrorQty()
{
	return m_aryModules[ecmMirror];
}

int CProposalPricing::GetUnitedQty()
{
	return m_aryModules[ecmUnited];
}

int CProposalPricing::GetHL7Qty()
{
	return m_aryModules[ecmHL7];
}

// (d.thompson 2013-07-08) - PLID 57334
int CProposalPricing::GetHIEQty()
{
	return m_aryModules[ecmHIE];
}

int CProposalPricing::GetNexWebLeadQty()
{
	return m_aryModules[ecmLeadGen];
}

int CProposalPricing::GetNexWebPortalQty()
{
	return m_aryModules[ecmPatientPortal];
}

int CProposalPricing::GetEMRQty()
{
	return m_aryModules[ecmEMR];
}

int CProposalPricing::GetTravelQty()
{
	return m_aryModules[ecmTravel];
}

int CProposalPricing::GetConversionQty()
{
	return m_aryModules[ecmConversion];
}

// (d.lange 2010-11-11 16:34) - PLID 40361 - Added EMR Conversion
int CProposalPricing::GetEmrConversionQty()
{
	return m_aryModules[ecmEmrConversion];
}

// (d.lange 2010-11-11 16:34) - PLID 40361 - Added Financial Conversion
int CProposalPricing::GetFinancialConversionQty()
{
	return m_aryModules[ecmFinancialConversion];
}

int CProposalPricing::GetEMRTrainingQty()
{
	return m_aryModules[ecmEMRTraining];
}

int CProposalPricing::GetExtraDBQty()
{
	return m_aryModules[ecmExtraDB];
}

int CProposalPricing::GetNumSupport()
{
	return m_aryModules[ecmSupportMonths];
}

int CProposalPricing::GetWorkstationQty(long nMergeTypeID)
{
	int nWorkstationsCounted = 0;
	if(nMergeTypeID == eptNexStartup || nMergeTypeID == eptNexRes) {
		nWorkstationsCounted = 1;
	}

	return (m_aryModules[ecmWorkstations] - nWorkstationsCounted);
}

// (d.thompson 2012-10-12) - PLID 53155 - TS workstations
int CProposalPricing::GetWorkstationTSQty()
{
	return m_aryModules[ecmWorkstationsTS];
}

long CProposalPricing::GetPMTrainingQty(long nMergeTypeID)
{
	long nPMTraining = 0;
	if(nMergeTypeID == eptNexStartup || nMergeTypeID == eptNexRes) {
		nPMTraining = 1;
	}

	return (m_aryModules[ecmPMTraining] - nPMTraining);
}


//////////////////// Calculation functions


COleCurrency CProposalPricing::CalcAddtlDomainTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmAddDomains] * m_aryPrices[ecpNexWebAddtlDomains]);

	return cyTotal;
}

COleCurrency CProposalPricing::CalcConversionTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmConversion] * m_aryPrices[ecpConversion]);

	return cyTotal;
}

COleCurrency CProposalPricing::CalcTravelTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmTravel] * m_aryPrices[ecpTravel]);

	return cyTotal;
}

COleCurrency CProposalPricing::CalcMirrorTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmMirror] ? m_aryPrices[ecpMirror] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::CalcUnitedTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmUnited] ? m_aryPrices[ecpUnited] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::CalcHL7Total()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmHL7] ? m_aryPrices[ecpHL7] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::CalcInformTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmInform] ? m_aryPrices[ecpInform] : COleCurrency(0, 0));

	return cyTotal;
}

COleCurrency CProposalPricing::CalcQuickbooksTotal()
{
	COleCurrency cyTotal(0, 0);

	cyTotal += (m_aryModules[ecmQuickBooks] ? m_aryPrices[ecpQuickbooks] : COleCurrency(0, 0));

	return cyTotal;
}