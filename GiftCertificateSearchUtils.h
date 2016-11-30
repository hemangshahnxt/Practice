#pragma once

// (j.jones 2015-03-23 13:34) - PLID 65281 - created

namespace GiftCertificateSearchUtils {

//sentinel for when the GC search has no hits
#define GC_NO_RESULTS_ROW -100

	// (j.jones 2015-03-23 13:14) - PLID 65281 - Defines a searchable gift certificate list
	// for the Payment dialog, which searches non-expired GCs by the user defined GiftID.
	// If nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	// and any GCs not assigned to any patient. Otherwise it's all GCs.
	LPUNKNOWN BindGiftCertificatePaymentSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, long nOnlyGCsForPatientID);

	// (j.jones 2015-03-23 13:41) - PLID 65281 - enum for the payment result columns
	enum GCPaymentSearchColumns {
		gcpscID = 0,
		gcpscGiftID,
		gcpscTotalValue,
		gcpscBalance,
		gcpscPurchasedDate,
		gcpscExpireDate,
		// (r.gonet 2015-06-01 09:25) - PLID 65281 - Added patient columns per request from product management.
		gcpscPurchasedBy,
		gcpscReceivedBy,
	};

	// (j.jones 2015-03-23 13:38) - PLID 65281 - creates the columns for the GC search on the payment dialog
	void CreatePaymentResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID);

	// (j.jones 2015-03-23 15:25) - PLID 65281 - switches the search provider based on nOnlyGCsForPatientID,
	//if nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	//and any GCs not assigned to any patient
	void UpdatePaymentResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID);

	// (j.jones 2015-03-25 12:52) - PLID 65390 - Defines a searchable gift certificate list
	// for the Find Gift Certificate dialog, which searches all GCs in the system by the user defined GiftID.
	LPUNKNOWN BindGiftCertificateGeneralSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided);

	// (j.jones 2015-03-25 13:02) - PLID 65390 - enum for the general result columns
	enum GCGeneralSearchColumns {
		gcgscID = 0,
		gcgscGiftID,
		gcgscTypeName,
		gcgscPurchasedDate,
		gcgscExpireDate,
		gcgscPurchasedBy,
		gcgscReceivedBy,
		gcgscTotalValue,
		gcgscAmtUsed,
		gcgscBalance,
		gcgscVoided,
	};

	// (j.jones 2015-03-25 13:02) - PLID 65390 - creates the columns for the GC search on the Find Gift Certificates dialog
	void CreateGeneralResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided);

	// (j.jones 2015-03-25 13:45) - PLID 65390 - switches the search provider based on the filters
	void UpdateGeneralResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon,
		bool bHideZeroBalances, bool bHideExpired, bool bHideVoided);

}; //end namespace