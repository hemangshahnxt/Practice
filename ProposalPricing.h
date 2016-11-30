#pragma once
// (d.lange 2010-08-24 11:27) - PLID 38029 - created

// (d.lange 2010-04-05) - PLID 37956 - added this for EMR base pricing, either First or Addtl
enum eEMRBasePrice {
	embFirstEMR = 0,
	embAddtlEMR,
};

class CProposalPricing
{
public:
	CProposalPricing(CArray<COleCurrency, COleCurrency> &aryPrices, CArray<int, int> &aryModules);
	~CProposalPricing(void);

	// Get total functions
	COleCurrency GetPMPkgTotal();
	COleCurrency GetPMTotal();
	COleCurrency GetSchedulerPrice();
	COleCurrency GetFinancialPkgTotal();
	COleCurrency GetIndividualFinTotal();
	COleCurrency GetCosmeticPkgTotal(long nMergeTypeID);
	COleCurrency GetIndividualCosTotal(long nMergeTypeID);
	COleCurrency GetOtherTotal();
	COleCurrency GetNexWebPkgTotal();
	COleCurrency GetIndividualNexWebTotal();
	COleCurrency GetEMRTotal(int nMultipleEMR);
	COleCurrency GetPDATotal();
	COleCurrency GetNexSyncTotal();
	COleCurrency GetWorkstationPrice();
	COleCurrency GetMultiDoctorPrice();
	COleCurrency GetTrainingPrice();
	COleCurrency GetInventoryTotal();
	COleCurrency GetCandATotal();
	COleCurrency GetNexSpaTotal();
	COleCurrency GetNexPhotoTotal();
	COleCurrency GetASCTotal();
	COleCurrency GetERemittanceTotal();
	COleCurrency GetEEligibilityTotal();
	COleCurrency GetMirrorTotal();
	COleCurrency GetUnitedTotal();
	COleCurrency GetHL7Total();
	COleCurrency GetHIETotal();							// (d.thompson 2013-07-08) - PLID 57334
	COleCurrency GetEMRFirstBasePrice();
	COleCurrency GetEMRAddtlBasePrice();
	COleCurrency GetEMRAllBasePrice();					// (d.thompson 2010-12-29) - PLID 41948
	COleCurrency GetInformTotal();
	COleCurrency GetQuickbooksTotal();
	COleCurrency GetTravelTotal(long nTravelType);		// (d.lange 2010-12-22 10:17) - PLID 41889 - Support additional travel rules
	COleCurrency GetConversionTotal();
	COleCurrency GetCosmeticPkgBasePrice();
	COleCurrency GetAddtlDomainTotal();
	COleCurrency GetExtraDBTotal();
	COleCurrency GetSupportMonthlyTotal(long nMergeTypeID);

	COleCurrency GetPrePkgTotal();
	COleCurrency GetListSubTotal(long nMergeTypeID);
	COleCurrency GetSubTotal(long nMergeTypeID);
	// (d.lange 2010-12-22 10:17) - PLID 41889 - Support additional travel rules
	COleCurrency GetGrandTotal(long nMergeTypeID, long nTravelType, COleCurrency cySupportTotal, COleCurrency cyDiscount);

	CString GetInformQuickbooksString(CString strInformPrice, CString strQuickbooksPrice);
	COleCurrency GetInformQuickbooksTotal();

	// Get quantity numbers of modules
	int GetInventoryQty();
	int GetCandAQty();
	int GetNexSpaQty();
	int GetNexPhotoQty();
	int GetASCQty();
	int GetERemittanceQty();
	int GetEEligibilityQty();
	int GetEMRWorkstationQty();
	int GetMultiDoctorQty();
	int GetPDAQty();
	int GetNexSyncQty();
	int GetMirrorQty();
	int GetUnitedQty();
	int GetHL7Qty();
	int GetHIEQty();					// (d.thompson 2013-07-08) - PLID 57334
	int GetNexWebLeadQty();
	int GetNexWebPortalQty();
	int GetEMRQty();
	int GetTravelQty();
	int GetConversionQty();
	int GetEmrConversionQty();			// (d.lange 2010-11-11 16:34) - PLID 40361 - Added EMR Conversion
	int GetFinancialConversionQty();	// (d.lange 2010-11-11 16:34) - PLID 40361 - Added Financial Conversion
	int GetEMRTrainingQty();
	int GetExtraDBQty();
	int GetWorkstationQty(long nMergeTypeID);
	int GetWorkstationTSQty();			// (d.thompson 2012-10-12) - PLID 53155
	int GetNumSupport();
	long GetPMTrainingQty(long nMergeTypeID);

protected:
	CArray<COleCurrency, COleCurrency> m_aryPrices;
	CArray<int, int> m_aryModules;

	// Calculate total functions
	COleCurrency CalcAddtlDomainTotal();
	COleCurrency CalcConversionTotal();
	COleCurrency CalcTravelTotal();
	COleCurrency CalcMirrorTotal();
	COleCurrency CalcUnitedTotal();
	COleCurrency CalcHL7Total();
	COleCurrency CalcInformTotal();
	COleCurrency CalcQuickbooksTotal();
};
