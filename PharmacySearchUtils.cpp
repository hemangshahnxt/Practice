#include "stdafx.h"
#include "PharmacySearchUtils.h"
#include "GlobalUtils.h"
#include "PharmacySearch_DataListProvider.h"

// (r.gonet 2016-02-17 16:31) - PLID 68404

using namespace NXDATALIST2Lib;

namespace PharmacySearchUtils {

	// (r.gonet 2016-02-17 16:31) - PLID 68404 - Defines a searchable pharmacy list.
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	LPUNKNOWN BindPharmacySearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, HICON hFavoriteIcon, long nPatientPersonID/*= -1*/, long nPatientMedicationID/*= -1*/)
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
				ThrowNxException("%s called on a datalist that is not a Search List.", __func__);
			}

			//initialize the datalist and set our DataListProvider				
			if (pDataList == nullptr) {
				ThrowNxException("%s called on a null datalist.", __func__);
			}

			ADODB::_ConnectionPtr pCon = pDataConn;
			if (pCon == nullptr) {
				//this shouldn't be created without a valid connection
				ASSERT(FALSE);
				ThrowNxException("%s called with a null connection.", __func__);
			}

			//clear out the existing list contents and columns
			pDataList->Clear();

			while (pDataList->GetColumnCount() > 0) {
				pDataList->RemoveColumn(0);
			}

			//Pharmacy searches require no characters to begin searching!
			pDataList->SearchMinChars = 0;
			pDataList->AutoSearchResults = VARIANT_TRUE;

			//now build the search content
			CreatePharmacyResultList(pDataList, pCon, hFavoriteIcon, nPatientPersonID, nPatientMedicationID);
		}

		// If we made it here we have success or it failed in ::Bind
		return pDataList;
	}

	// (r.gonet 2016-02-17 16:45) - PLID 68404 - Creates the pharmacy search results datalist, ie adds the columns and sets properties.
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	void CreatePharmacyResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID, long nPatientMedicationID)
	{
		//Do not allow this if the datalist is NULL
		if (pDataList == NULL) {
			ThrowNxException("%s called on a null pDataList.", __func__);
		}

		//Do not allow for a NULL connection
		if (pCon == NULL) {
			ThrowNxException("%s called with a null connection.", __func__);
		}

		pDataList->PutSearchPlaceholderText("Pharmacy Search...");

		AddPharmacyListColumns(pDataList);

		//set the clauses to nothing, we won't use them
		pDataList->FromClause = _bstr_t("");
		pDataList->WhereClause = _bstr_t("");

		pDataList->PutViewType(vtSearchList);
		pDataList->PutAutoDropDown(VARIANT_FALSE);

		//force the dropdown width to be 1015 - Just enough to show the maximum length name in NexERxPharmacyT.StoreName without
		//adding a horizontal scrollbar while being just under the limit of a tiny 1024px width screen.
		SetMinDatalistDropdownWidth(pDataList, 1015);

		//set the provider
		UpdatePharmacyResultList(pDataList, pCon, hFavoriteIcon, nPatientPersonID, nPatientMedicationID);
	}

	// (r.gonet 2016-02-23 00:53) - PLID 68408 - Create the renewal pharmacy combo.
	void CreatePharmacyDropDown(NXDATALIST2Lib::_DNxDataListPtr pDataList)
	{
		//Do not allow this if the datalist is NULL
		if (pDataList == NULL) {
			ThrowNxException("%s called on a null pDataList.", __func__);
		}

		AddPharmacyListColumns(pDataList);
		
		//set the clauses to nothing, we won't use them
		pDataList->FromClause = _bstr_t("");
		pDataList->WhereClause = _bstr_t("");

		pDataList->ViewType = EViewType::vtDropdownList;
		//force the dropdown width to be 1015 - Just enough to show the maximum length name in NexERxPharmacyT.StoreName without
		//adding a horizontal scrollbar while being just under the limit of a tiny 1024px width screen.
		pDataList->PutDropDownWidth(1015);
		pDataList->DisplayColumn = _bstr_t(FormatString("[%li]", EPharmacySearchColumns::Name));
		pDataList->TextSearchCol = (short)EPharmacySearchColumns::Name;
		pDataList->AutoDropDown = VARIANT_TRUE;
	}

	// (r.gonet 2016-02-23 00:53) - PLID 68408 - Split to re-use to fill the renewal pharmacy combo.
	void AddPharmacyListColumns(NXDATALIST2Lib::_DNxDataListPtr pDataList)
	{
		//Do not allow this if the datalist is NULL
		if (pDataList == NULL) {
			ThrowNxException("%s called on a null pDataList.", __func__);
		}

		//the caller should have cleared all columns & rows,
		//if they didn't, then this function was called improperly
		if (pDataList->GetColumnCount() > 0 || pDataList->GetRowCount() > 0) {
			//Careful, this wipes the existing content before recreating it
			ASSERT(FALSE);
			ThrowNxException("%s called on a non-empty pDataList.", __func__);
		}

		//create our columns
		IColumnSettingsPtr pCol = nullptr;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::ID, _T("ID"), _T("ID"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_I4;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::LeadingIcon, _T("LeadingIcon"), _T(""), 35, csVisible));
		pCol->FieldType = cftBitmapBuiltIn;
		pCol->DataType = VT_I4;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Name, _T("Pharmacy"), _T("Pharmacy"), 100, csVisible | csWidthData));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Address, _T("Address"), _T("Address"), 140, csVisible));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::City, _T("City"), _T("City"), 70, csVisible));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::State, _T("State"), _T("State"), 45, csVisible | csWidthData));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Zip, _T("Zip"), _T("Zip"), 50, csVisible | csWidthData));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::CrossStreet, _T("Cross Street"), _T("Cross Street"), 100, csVisible));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::OrderIndex, _T("OrderIndex"), _T("OrderIndex"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Phone, _T("Phone"), _T("Phone"), 50, csVisible | csWidthData));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::NCPDPID, _T("NCPDPID"), _T("NCPDPID"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Fax, _T("Fax"), _T("Fax"), 50, csVisible | csWidthData));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::EPrescribingReady, _T("ERx"), _T("ERx"), 40, csVisible));
		pCol->FieldType = cftBoolCheckbox;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Spec1, _T("Specialty 1"), _T("Specialty 1"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Spec2, _T("Specialty 2"), _T("Specialty 2"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Spec3, _T("Specialty3"), _T("Specialty3"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::Spec4, _T("Specialty4"), _T("Specialty4"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::SpecAll, _T("Specialty type"), _T("Specialty Type"), 100, csVisible));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Add a background color column so non-erx pharmacies can be colored gray.
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::RowBackColor, _T("RowBackColor"), _T("RowBackColor"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftSetRowBackColor;
		pCol->DataType = VT_I4;
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Add a background color selected column so non-erx pharmacies can be colored gray.
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::RowBackColorSel, _T("RowBackColorSel"), _T("RowBackColorSel"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftSetRowBackColorSel;
		pCol->DataType = VT_I4;
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Add a foreground color column
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::RowForeColor, _T("RowForeColor"), _T("RowForeColor"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftSetRowForeColor;
		pCol->DataType = VT_I4;
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Add a foreground color selected column
		pCol = pDataList->GetColumn(pDataList->InsertColumn((short)EPharmacySearchColumns::RowForeColorSel, _T("RowForeColorSel"), _T("RowForeColorSel"), 0, csVisible | csFixedWidth));
		pCol->FieldType = cftSetRowForeColorSel;
		pCol->DataType = VT_I4;
	}

	// (r.gonet 2016-02-17 16:45) - PLID 68404 - Switches the search provider based on nSelectedPharmacyID,
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	void UpdatePharmacyResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID, long nPatientMedicationID)
	{
		CPharmacySearch_DatalistProvider* provider = new CPharmacySearch_DatalistProvider(pCon, hFavoriteIcon, nPatientPersonID, nPatientMedicationID);
		provider->Register(pDataList);
		provider->Release();
	}

}; //end namespace