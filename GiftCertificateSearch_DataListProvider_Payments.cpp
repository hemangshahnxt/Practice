#include "stdafx.h"
#include "GiftCertificateSearch_DataListProvider_Payments.h"
#include "GiftCertificateSearchUtils.h"
#include "NxAPI.h"

// (j.jones 2014-02-10 09:13) - PLID 60679 - Created

using namespace GiftCertificateSearchUtils;
using namespace ADODB;

//if nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
//and any GCs not assigned to any patient
CGiftCertificateSearch_DataListProvider_Payments::CGiftCertificateSearch_DataListProvider_Payments(ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID)
{
	m_nOnlyGCsForPatientID = nOnlyGCsForPatientID;
}

CGiftCertificateSearch_DataListProvider_Payments::~CGiftCertificateSearch_DataListProvider_Payments(void)
{
}

SAFEARRAY *CGiftCertificateSearch_DataListProvider_Payments::Search(BSTR bstrSearchString)
{
	CWaitCursor pWait;

	NexTech_Accessor::_GiftCertificatePaymentListResultsPtr pResults = GetAPI()->SearchGiftCertificates_PaymentList(GetAPISubkey(), GetAPILoginToken(), bstrSearchString, _bstr_t(AsString(m_nOnlyGCsForPatientID)));

	// Get the results array
	if (pResults != NULL) {
		return pResults->Results;
	}
	else {
		return NULL;
	}
}

void CGiftCertificateSearch_DataListProvider_Payments::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_GiftCertificatePaymentListResultPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varID = (long)atoi(VarString(pResult->ID));	//convert to long
	_variant_t varGiftID = pResult->GiftID;
	//convert to money
	COleCurrency cyTotalValue = AsCurrency(VarFloat(pResult->TotalValue));
	_variant_t varTotalValue = cyTotalValue;
	//convert to money
	COleCurrency cyBalance = AsCurrency(VarFloat(pResult->Balance));
	_variant_t varBalance = cyBalance;
	_variant_t varPurchasedDate = pResult->PurchaseDate;
	_variant_t varExpireDate = pResult->ExpDate->GetValueOrDefault(g_cvarNull);
	// (r.gonet 2015-06-01 09:25) - PLID 60679 - Added patient columns per request from product management.
	_variant_t varPurchasedBy = pResult->PurchasedBy;
	_variant_t varReceivedBy = pResult->ReceivedBy;
	
	//add all our values into a safearray
	// Speed boost: create all elements in the array at once
	{
		// Write values to the safearray elements
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		//Speed boost: avoid copying the variants

		aryFieldValues[gcpscID] = varID.Detach();
		aryFieldValues[gcpscGiftID] = varGiftID.Detach();
		aryFieldValues[gcpscTotalValue] = varTotalValue.Detach();
		aryFieldValues[gcpscBalance] = varBalance.Detach();
		aryFieldValues[gcpscPurchasedDate] = varPurchasedDate.Detach();
		aryFieldValues[gcpscExpireDate] = varExpireDate.Detach();
		// (r.gonet 2015-06-01 09:25) - PLID 60679 - Added patient columns per request from product management.
		aryFieldValues[gcpscPurchasedBy] = varPurchasedBy.Detach();
		aryFieldValues[gcpscReceivedBy] = varReceivedBy.Detach();
	}
}

HRESULT CGiftCertificateSearch_DataListProvider_Payments::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[gcpscID] = _variant_t((long)GC_NO_RESULTS_ROW, VT_I4).Detach();
	aryFieldValues[gcpscGiftID] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[gcpscTotalValue] = g_cvarNull;
	aryFieldValues[gcpscBalance] = g_cvarNull;
	aryFieldValues[gcpscPurchasedDate] = g_cvarNull;
	aryFieldValues[gcpscExpireDate] = g_cvarNull;
	return S_OK;
}
