#include "stdafx.h"
#include "GiftCertificateSearch_DataListProvider_General.h"
#include "GiftCertificateSearchUtils.h"
#include "NxAPI.h"

// (j.jones 2015-03-25 14:10) - PLID 65390 - Created

using namespace GiftCertificateSearchUtils;
using namespace ADODB;

CGiftCertificateSearch_DataListProvider_General::CGiftCertificateSearch_DataListProvider_General(ADODB::_ConnectionPtr pCon,
	bool bHideZeroBalances, bool bHideExpired, bool bHideVoided)
{
	m_bHideZeroBalances = bHideZeroBalances;
	m_bHideExpired = bHideExpired;
	m_bHideVoided = bHideVoided;
}

CGiftCertificateSearch_DataListProvider_General::~CGiftCertificateSearch_DataListProvider_General(void)
{
}

SAFEARRAY *CGiftCertificateSearch_DataListProvider_General::Search(BSTR bstrSearchString)
{
	CWaitCursor pWait;

	NexTech_Accessor::_GiftCertificateGeneralListResultsPtr pResults = GetAPI()->SearchGiftCertificates_GeneralList(GetAPISubkey(), GetAPILoginToken(), bstrSearchString, m_bHideZeroBalances, m_bHideExpired, m_bHideVoided);

	// Get the results array
	if (pResults != NULL) {
		return pResults->Results;
	}
	else {
		return NULL;
	}
}

void CGiftCertificateSearch_DataListProvider_General::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_GiftCertificateGeneralListResultPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varID = (long)atoi(VarString(pResult->ID));	//convert to long
	_variant_t varGiftID = pResult->GiftID;
	_variant_t varTypeName = pResult->TypeName;
	_variant_t varPurchasedDate = pResult->PurchaseDate;
	_variant_t varExpireDate = pResult->ExpDate->GetValueOrDefault(g_cvarNull);
	_variant_t varPurchasedBy = pResult->PurchasedBy;
	_variant_t varReceivedBy = pResult->ReceivedBy;
	//convert to money
	COleCurrency cyTotalValue = AsCurrency(VarFloat(pResult->TotalValue));
	_variant_t varTotalValue = cyTotalValue;
	//convert to money
	COleCurrency cyAmtUsed = AsCurrency(VarFloat(pResult->AmtUsed));
	_variant_t varAmtUsed = cyAmtUsed;
	//convert to money
	COleCurrency cyBalance = AsCurrency(VarFloat(pResult->Balance));
	_variant_t varBalance = cyBalance;
	//convert to a bit
	int iVoided = VarByte(pResult->Voided, 0);
	_variant_t varVoided = iVoided == 0 ? g_cvarFalse : g_cvarTrue;
	
	//add all our values into a safearray
	// Speed boost: create all elements in the array at once
	{
		// Write values to the safearray elements
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		//Speed boost: avoid copying the variants

		aryFieldValues[gcgscID] = varID.Detach();
		aryFieldValues[gcgscGiftID] = varGiftID.Detach();
		aryFieldValues[gcgscTypeName] = varTypeName.Detach();
		aryFieldValues[gcgscPurchasedDate] = varPurchasedDate.Detach();
		aryFieldValues[gcgscExpireDate] = varExpireDate.Detach();
		aryFieldValues[gcgscPurchasedBy] = varPurchasedBy.Detach();
		aryFieldValues[gcgscReceivedBy] = varReceivedBy.Detach();
		aryFieldValues[gcgscTotalValue] = varTotalValue.Detach();
		aryFieldValues[gcgscAmtUsed] = varAmtUsed.Detach();
		aryFieldValues[gcgscBalance] = varBalance.Detach();
		aryFieldValues[gcgscVoided] = varVoided.Detach();
	}
}

HRESULT CGiftCertificateSearch_DataListProvider_General::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[gcgscID] = _variant_t((long)GC_NO_RESULTS_ROW, VT_I4).Detach();
	aryFieldValues[gcgscGiftID] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[gcgscTypeName] = g_cvarNull;
	aryFieldValues[gcgscPurchasedDate] = g_cvarNull;
	aryFieldValues[gcgscExpireDate] = g_cvarNull;
	aryFieldValues[gcgscPurchasedBy] = g_cvarNull;
	aryFieldValues[gcgscReceivedBy] = g_cvarNull;
	aryFieldValues[gcgscTotalValue] = g_cvarNull;
	aryFieldValues[gcgscAmtUsed] = g_cvarNull;
	aryFieldValues[gcgscBalance] = g_cvarNull;
	aryFieldValues[gcgscVoided] = g_cvarNull;
	return S_OK;
}
