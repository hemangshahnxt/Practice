#pragma once

// (r.gonet 2016-02-17 16:31) - PLID 68404 - Created

namespace PharmacySearchUtils {
	// (r.gonet 2016-02-17 16:31) - PLID 68404 - Defines a searchable pharmacy list.
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	LPUNKNOWN BindPharmacySearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, HICON hFavoriteIcon, long nPatientPersonID = -1, long nPatientMedicationID = -1);

	// (r.gonet 2016-02-17 16:43) - PLID 68404 - Enum for the pharmacy search columns.
	enum class EPharmacySearchColumns {
		ID = 0,
		LeadingIcon,
		Name,
		Address,
		City,
		State,
		Zip,
		CrossStreet, // (r.gonet 2016-02-24 10:59) - PLID 68418
		OrderIndex,
		Phone,	//TES 5/6/2009 - PLID 34178
		NCPDPID,	//TES 5/6/2009 - PLID 34178
		Fax,		//(r.farnworth 2/5/2013) - PLID 54667
		EPrescribingReady,	//TES 8/10/2009 - PLID 35130
		Spec1,	//(r.farnworth 2/5/2013) - PLID 54667
		Spec2,	//(r.farnworth 2/5/2013) - PLID 54667
		Spec3,	//(r.farnworth 2/5/2013) - PLID 54667
		Spec4,	//(r.farnworth 2/5/2013) - PLID 54667
		SpecAll,
		RowBackColor, // (r.gonet 2016-02-23 11:02) - PLID 67962
		RowBackColorSel, // (r.gonet 2016-02-23 11:02) - PLID 67962
		RowForeColor, // (r.gonet 2016-02-23 11:02) - PLID 67962
		RowForeColorSel, // (r.gonet 2016-02-23 11:02) - PLID 67962
	};

	// (r.gonet 2016-02-17 16:45) - PLID 68404 - Creates the pharmacy search results datalist, ie adds the columns and sets properties.
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	void CreatePharmacyResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID, long nPatientMedicationID);
	// (r.gonet 2016-02-23 00:53) - PLID 68408 - Create the renewal pharmacy combo.
	void CreatePharmacyDropDown(NXDATALIST2Lib::_DNxDataListPtr pDataList);
	// (r.gonet 2016-02-23 00:53) - PLID 68408 - Split to re-use to fill the renewal pharmacy combo.
	void AddPharmacyListColumns(NXDATALIST2Lib::_DNxDataListPtr pDataList);

	// (r.gonet 2016-02-17 16:45) - PLID 68404 - Switches the search provider based on nSelectedPharmacyID,
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	void UpdatePharmacyResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID, long nPatientMedicationID);
}; //end namespace