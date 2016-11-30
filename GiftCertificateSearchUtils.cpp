#include "stdafx.h"
#include "GiftCertificateSearchUtils.h"
#include "GlobalUtils.h"
#include "GiftCertificateSearch_DataListProvider_Payments.h"
#include "GiftCertificateSearch_DataListProvider_General.h"

// (j.jones 2015-03-23 13:34) - PLID 65281 - created

using namespace NXDATALIST2Lib;

namespace GiftCertificateSearchUtils {

	// (j.jones 2015-03-23 13:14) - PLID 65281 - Defines a searchable gift certificate list
	// for the Payment dialog, which searches non-expired GCs by the user defined GiftID.
	// If nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	// and any GCs not assigned to any patient. Otherwise it's all GCs.
	LPUNKNOWN BindGiftCertificatePaymentSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, long nOnlyGCsForPatientID)
	{
		//throw all exceptions to the caller

		//We can utilize the standard bind to do the majority of the work for us, then tack on a couple specific search
		//	things afterwards.
		//Bind will do this for us:
		//	1)  Bind the control from the IDC
		//	2)  Apply the connection, if necessary
		//	3)  Requery if needed (which will never happen for a search dialog)
		//	4)  Some fancy exception handling on various failures for the above.

		//Bind it as normal.  Search view is only supported on datalist2's.
		_DNxDataListPtr pDataList = ::BindNxDataList2Ctrl(pParent, nID, pDataConn, false);

		//Now do our special search things
		if (pDataList) {
			//if the datalist is not using the search list, fail
			if (pDataList->GetViewType() != vtSearchList) {
				//this would be a coding failure at design time
				ThrowNxException("BindGiftCertificatePaymentSearchListCtrl called on a datalist that is not a Search List.");
			}

			//initialize the datalist and set our DataListProvider				
			if (pDataList == NULL) {
				ThrowNxException("BindGiftCertificatePaymentSearchListCtrl called on a null datalist.");
			}

			ADODB::_ConnectionPtr pCon = pDataConn;
			if (pCon == NULL) {
				//this shouldn't be created without a valid connection
				ASSERT(FALSE);
				ThrowNxException("BindGiftCertificatePaymentSearchListCtrl called with a null connection.");
			}

			//clear out the existing list contents and columns
			pDataList->Clear();

			while (pDataList->GetColumnCount() > 0) {
				pDataList->RemoveColumn(0);
			}

			//GC searches only require one character to begin searching
			pDataList->SearchMinChars = 1;

			//now build the search content
			CreatePaymentResultList(pDataList, pCon, nOnlyGCsForPatientID);
		}

		// If we made it here we have success or it failed in ::Bind
		return pDataList;
	}

	// (j.jones 2015-03-23 13:38) - PLID 65281 - creates the columns for the GC search on the payment dialog
	//if nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	//and any GCs not assigned to any patient
	void CreatePaymentResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID)
	{
		//Do not allow this if the datalist is NULL
		if (pDataList == NULL) {
			ThrowNxException("CreatePaymentResultList called on a null pDataList.");
		}

		//Do not allow for a NULL connection
		if (pCon == NULL) {
			ThrowNxException("CreatePaymentResultList called with a null connection.");
		}

		//the caller should have cleared all columns & rows,
		//if they didn't, then this function was called improperly
		if (pDataList->GetColumnCount() > 0 || pDataList->GetRowCount() > 0) {
			//make sure this function is never called outside BindGiftCertificateSearchListCtrl,
			//which wipes the existing content before recreating it
			ASSERT(FALSE);
			ThrowNxException("CreatePaymentResultList called on a non-empty pDataList.");
		}

		pDataList->PutSearchPlaceholderText("Gift Certificate Search...");

		//create our columns
		IColumnSettingsPtr pColID = pDataList->GetColumn(pDataList->InsertColumn(gcpscID, _T("ID"), _T("ID"), 0, csVisible | csFixedWidth));
		pColID->FieldType = cftNumberBasic;
		pColID->DataType = VT_I4;
		IColumnSettingsPtr pColGiftID = pDataList->GetColumn(pDataList->InsertColumn(gcpscGiftID, _T("GiftID"), _T("Certificate #"), 75, csVisible | csWidthData));
		pColGiftID->FieldType = cftTextSingleLine;
		pColGiftID->DataType = VT_BSTR;
		IColumnSettingsPtr pColTotalValue = pDataList->GetColumn(pDataList->InsertColumn(gcpscTotalValue, _T("TotalValue"), _T("Value"), 55, csVisible | csWidthData));
		pColTotalValue->FieldType = cftTextSingleLine;
		pColTotalValue->DataType = VT_CY;
		IColumnSettingsPtr pColBalance = pDataList->GetColumn(pDataList->InsertColumn(gcpscBalance, _T("Balance"), _T("Balance"), 55, csVisible | csWidthData));
		pColBalance->FieldType = cftTextSingleLine;
		pColBalance->DataType = VT_CY;
		IColumnSettingsPtr pColPurchasedDate = pDataList->GetColumn(pDataList->InsertColumn(gcpscPurchasedDate, _T("PurchasedDate"), _T("Purchase Date"), 95, csVisible | csWidthData));
		pColPurchasedDate->FieldType = cftDateShort;
		pColPurchasedDate->DataType = VT_DATE;
		IColumnSettingsPtr pColExpireDate = pDataList->GetColumn(pDataList->InsertColumn(gcpscExpireDate, _T("ExpireDate"), _T("Exp. Date"), 80, csVisible | csWidthData));
		pColExpireDate->FieldType = cftDateShort;
		pColExpireDate->DataType = VT_DATE;
		// (r.gonet 2015-06-01 09:23) - PLID 65281 - Added patient columns per request from project management.
		IColumnSettingsPtr pColPurchasedBy = pDataList->GetColumn(pDataList->InsertColumn(gcpscPurchasedBy, _T("PurchasedBy"), _T("Purchased By"), -1, csVisible | csWidthAuto));
		pColPurchasedBy->FieldType = cftTextSingleLine;
		pColPurchasedBy->DataType = VT_BSTR;
		IColumnSettingsPtr pColReceivedBy = pDataList->GetColumn(pDataList->InsertColumn(gcpscReceivedBy, _T("ReceivedBy"), _T("Received By"), -1, csVisible | csWidthAuto));
		pColReceivedBy->FieldType = cftTextSingleLine;
		pColReceivedBy->DataType = VT_BSTR;

		//set the clauses to nothing, we won't use them
		pDataList->FromClause = _bstr_t("");
		pDataList->WhereClause = _bstr_t("");

		pDataList->PutViewType(vtSearchList);
		pDataList->PutAutoDropDown(VARIANT_FALSE);

		//force the dropdown width to be 600
		SetMinDatalistDropdownWidth(pDataList, 600);

		//set the provider
		UpdatePaymentResultList(pDataList, pCon, nOnlyGCsForPatientID);
	}

	// (j.jones 2015-03-23 15:25) - PLID 65281 - switches the search provider based on nOnlyGCsForPatientID,
	//if nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	//and any GCs not assigned to any patient
	void UpdatePaymentResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID)
	{
		CGiftCertificateSearch_DataListProvider_Payments* provider = new CGiftCertificateSearch_DataListProvider_Payments(pCon, nOnlyGCsForPatientID);
		provider->Register(pDataList);
		provider->Release();
	}

	// (j.jones 2015-03-25 12:52) - PLID 65390 - Defines a searchable gift certificate list
	// for the Find Gift Certificate dialog, which searches all GCs in the system by the user defined GiftID.
	LPUNKNOWN BindGiftCertificateGeneralSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided)
	{
		//throw all exceptions to the caller

		//We can utilize the standard bind to do the majority of the work for us, then tack on a couple specific search
		//	things afterwards.
		//Bind will do this for us:
		//	1)  Bind the control from the IDC
		//	2)  Apply the connection, if necessary
		//	3)  Requery if needed (which will never happen for a search dialog)
		//	4)  Some fancy exception handling on various failures for the above.

		//Bind it as normal.  Search view is only supported on datalist2's.
		_DNxDataListPtr pDataList = ::BindNxDataList2Ctrl(pParent, nID, pDataConn, false);

		//Now do our special search things
		if (pDataList) {
			//if the datalist is not using the search list, fail
			if (pDataList->GetViewType() != vtSearchList) {
				//this would be a coding failure at design time
				ThrowNxException("BindGiftCertificateGeneralSearchListCtrl called on a datalist that is not a Search List.");
			}

			//initialize the datalist and set our DataListProvider				
			if (pDataList == NULL) {
				ThrowNxException("BindGiftCertificateGeneralSearchListCtrl called on a null datalist.");
			}

			ADODB::_ConnectionPtr pCon = pDataConn;
			if (pCon == NULL) {
				//this shouldn't be created without a valid connection
				ASSERT(FALSE);
				ThrowNxException("BindGiftCertificateGeneralSearchListCtrl called with a null connection.");
			}

			//clear out the existing list contents and columns
			pDataList->Clear();

			while (pDataList->GetColumnCount() > 0) {
				pDataList->RemoveColumn(0);
			}

			//GC searches only require one character to begin searching
			pDataList->SearchMinChars = 1;

			//now build the search content
			CreateGeneralResultList(pDataList, pCon, bHideZeroBalances, bHideExpired, bHideVoided);
		}

		// If we made it here we have success or it failed in ::Bind
		return pDataList;
	}

	// (j.jones 2015-03-25 13:02) - PLID 65390 - creates the columns for the GC search on the Find Gift Certificates dialog
	void CreateGeneralResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided)
	{
		//Do not allow this if the datalist is NULL
		if (pDataList == NULL) {
			ThrowNxException("CreateGeneralResultList called on a null pDataList.");
		}

		//Do not allow for a NULL connection
		if (pCon == NULL) {
			ThrowNxException("CreateGeneralResultList called with a null connection.");
		}

		//the caller should have cleared all columns & rows,
		//if they didn't, then this function was called improperly
		if (pDataList->GetColumnCount() > 0 || pDataList->GetRowCount() > 0) {
			//make sure this function is never called outside BindGiftCertificateSearchListCtrl,
			//which wipes the existing content before recreating it
			ASSERT(FALSE);
			ThrowNxException("CreateGeneralResultList called on a non-empty pDataList.");
		}

		pDataList->PutSearchPlaceholderText("Gift Certificate Search...");
		
		//create our columns
		IColumnSettingsPtr pColID = pDataList->GetColumn(pDataList->InsertColumn(gcgscID, _T("ID"), _T("ID"), 0, csVisible | csFixedWidth));
		pColID->FieldType = cftNumberBasic;
		pColID->DataType = VT_I4;
		IColumnSettingsPtr pColGiftID = pDataList->GetColumn(pDataList->InsertColumn(gcgscGiftID, _T("GiftID"), _T("Certificate #"), 75, csVisible | csWidthData));
		pColGiftID->FieldType = cftTextSingleLine;
		pColGiftID->DataType = VT_BSTR;
		IColumnSettingsPtr pColTypeName = pDataList->GetColumn(pDataList->InsertColumn(gcgscTypeName, _T("TypeName"), _T("Type"), -1, csVisible | csWidthAuto));
		pColTypeName->FieldType = cftTextSingleLine;
		pColTypeName->DataType = VT_BSTR;
		IColumnSettingsPtr pColPurchasedDate = pDataList->GetColumn(pDataList->InsertColumn(gcgscPurchasedDate, _T("PurchasedDate"), _T("Purch. Date"), 75, csVisible | csWidthData));
		pColPurchasedDate->FieldType = cftDateShort;
		pColPurchasedDate->DataType = VT_DATE;
		IColumnSettingsPtr pColExpireDate = pDataList->GetColumn(pDataList->InsertColumn(gcgscExpireDate, _T("ExpireDate"), _T("Exp. Date"), 60, csVisible | csWidthData));
		pColExpireDate->FieldType = cftDateShort;
		pColExpireDate->DataType = VT_DATE;
		IColumnSettingsPtr pColPurchasedBy = pDataList->GetColumn(pDataList->InsertColumn(gcgscPurchasedBy, _T("PurchasedBy"), _T("Purchased By"), -1, csVisible | csWidthAuto));
		pColPurchasedBy->FieldType = cftTextSingleLine;
		pColPurchasedBy->DataType = VT_BSTR;
		IColumnSettingsPtr pColReceivedBy = pDataList->GetColumn(pDataList->InsertColumn(gcgscReceivedBy, _T("ReceivedBy"), _T("Received By"), -1, csVisible | csWidthAuto));
		pColReceivedBy->FieldType = cftTextSingleLine;
		pColReceivedBy->DataType = VT_BSTR;
		IColumnSettingsPtr pColTotalValue = pDataList->GetColumn(pDataList->InsertColumn(gcgscTotalValue, _T("TotalValue"), _T("Total Value"), 75, csVisible | csWidthData));
		pColTotalValue->FieldType = cftTextSingleLine;
		pColTotalValue->DataType = VT_CY;
		IColumnSettingsPtr pColAmtUsed = pDataList->GetColumn(pDataList->InsertColumn(gcgscAmtUsed, _T("AmtUsed"), _T("Used"), 50, csVisible | csWidthData));
		pColAmtUsed->FieldType = cftTextSingleLine;
		pColAmtUsed->DataType = VT_CY;
		IColumnSettingsPtr pColBalance = pDataList->GetColumn(pDataList->InsertColumn(gcgscBalance, _T("Balance"), _T("Balance"), 55, csVisible | csWidthData));
		pColBalance->FieldType = cftTextSingleLine;
		pColBalance->DataType = VT_CY;
		IColumnSettingsPtr pColVoided = pDataList->GetColumn(pDataList->InsertColumn(gcgscVoided, _T("Void"), _T("Voided"), 50, csVisible | csWidthData));
		pColVoided->FieldType = cftBoolYesNo;
		pColVoided->DataType = VT_BOOL;

		//set the clauses to nothing, we won't use them
		pDataList->FromClause = _bstr_t("");
		pDataList->WhereClause = _bstr_t("");

		pDataList->PutViewType(vtSearchList);
		pDataList->PutAutoDropDown(VARIANT_FALSE);

		//force the dropdown width to be 750
		SetMinDatalistDropdownWidth(pDataList, 750);

		//set the provider
		UpdateGeneralResultList(pDataList, pCon, bHideZeroBalances, bHideExpired, bHideVoided);
	}

	// (j.jones 2015-03-25 13:45) - PLID 65390 - switches the search provider based on the filters
	void UpdateGeneralResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided)
	{
		CGiftCertificateSearch_DataListProvider_General* provider = new CGiftCertificateSearch_DataListProvider_General(pCon, bHideZeroBalances, bHideExpired, bHideVoided);
		provider->Register(pDataList);
		provider->Release();
	}

}; //end namespace