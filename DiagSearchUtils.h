#pragma once

#include "DiagSearchConfig.h"

// (d.thompson 2014-02-05) - PLID 60638
//Generally useful utilities for dealing with the DiagSearch.  Calling these protects you from needing to know
//	anything about the various implementations of the search configuration.  You should consider these the
//	only public faces to anything related to diagnosis search.
namespace DiagSearchUtils
{
	//Call this to determine the present style of search.  This returns the configured value that all lists 
	//	created in the application should be following.
	DiagCodeSearchStyle GetPreferenceSearchStyle();

	// (j.jones 2014-03-03 11:56) - PLID 61136 - added PCS subpreference
	bool GetPreferenceIncludePCS();

	//Call this to setup your diag search related datalist that follows the preference..  Do not use the regular BindNxDataListCtrl
	//	function, only call this one.
	LPUNKNOWN BindDiagPreferenceSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn);

	//Call this to setup your dual search related datalist.  Do not use the regular Bind... function.
	LPUNKNOWN BindDiagDualSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn);

	// (c.haag 2015-07-08) - PLID 66019 - Exposed to allow for custom bindings
	LPUNKNOWN CommonBindSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, CDiagSearchConfig* pSearchConfig);

	//This function takes the selected pSearchRow (generally from an OnSelChosen event) and converts the 
	//	data to a CDiagSearchResults object.  This is done to simplify the types of data that users of
	//	the DiagSearch types of datalists need to implement.  They can safely know that by looking at 
	//	the CDiagSearchResults values, they will always properly interpret the selection no matter
	//	if the search is on ICD-9, ICD-10, or other.
	CDiagSearchResults ConvertPreferenceSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);

	//See ConvertSearchResults, same thing, but uses the Dual Search configuration.
	CDiagSearchResults ConvertDualSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);

	// (c.haag 2015-07-08) - PLID 66019 - Exposed to allow for custom conversions
	CDiagSearchResults Common_ConvertPreferenceSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow, CDiagSearchConfig *pSearchConfig);

	// (j.jones 2014-02-10 14:35) - PLID 60679 - Added enum for a crosswalk result's MatchType.
	// The value of this enum does not matter but the order does, as results are sorted by
	// this value in order to have exact matches show up first, then approximate,
	// then combination, with No Match entries on the bottom.
	enum EDiagCrosswalkMatchType
    {
        //these first four results are used in crosswalk searches
        ExactMatch = 0,     //both codes exist, ExactMatch is true, and Combination is false   
        ApproximateMatch,   //both codes exist, ExactMatch is false, Combination is false, this is a rough "approximate" match
        Combination,        //Combination is true
        NoMatch,            //we don't have an ICD-9 and ICD-10, we have only one of them
		CustomMatch,    // (c.haag 2015-05-12) - PLID 66018 - This is at least a partial match on a user-created NexGEM

        NotApplicable,      //should never be used in a crosswalk search, always used when not a crosswalk search
    };

	// (j.jones 2014-02-19 17:18) - PLID 60907 - For any datalist that has columns
	// for ICD-9 ID, Code, Description, then ICD-10 ID, Code Description, we want
	// to always show both sets of codes when their search is in crosswalk mode,
	// otherwise only show the columns for their desired codeset.
	// But if showing only one codeset, and a record is found that has data in
	// the opposite codeset, we need to revert to the crosswalk display in order
	// to handle showing the content.
	// The return value is the style that was applied, since it may have changed.
	//
	//Required Parameters:
	//pDataList		- the datalist that holds your ICD-9 and ICD-10 code pairs
	//iColICD9Code	- the column index that holds the ICD-9 code
	//iColICD10Code - the column index that holds the ICD-10 code
	//
	//Optional Parameters:	
	//nMinICD9CodeWidth		- if ICD-9 is shown, it will be csWidthData with this minimum width
	//nMinICD10CodeWidth	- if ICD-10 is shown, it will be csWidthData with this minimum width
	//strICD9EmptyFieldText	- if your list displays a placeholder for a missing code, like <None> or "ICD-9",
	//						send that placeholder text here so it won't be treated as though a real code exists
	//strICD10EmptyFieldText	- if your list displays a placeholder for a missing code, like <None> or "ICD-10",
	//						send that placeholder text here so it won't be treated as though a real code exists
	//iColICD9Desc			- if your list has an ICD-9 description column, this should be its column index
	//iColICD10Desc			- if your list has an ICD-10 description column, this should be its column index
	//bShowDescriptionColumnsInCrosswalk	- By default, crosswalk lists won't show the description columns.
	//										Set this to true if you want the descriptions to show in crosswalk mode
	//bAllowCodeWidthAuto					- By default, if no descriptions are shown, the code columns will be set
	//										to csWidthAuto. Set this to false if you never want this to happen.
	//bRenameDescriptionColumns				- if set to true, description columns will rename to say "Description"
	//										in ICD-9 or 10 mode, and "ICD-9 Description" (or 10) in crosswalk mode
	DiagCodeSearchStyle SizeDiagnosisListColumnsBySearchPreference(NXDATALIST2Lib::_DNxDataListPtr pDataList,
		short iColICD9Code, short iColICD10Code,
		long nMinICD9CodeWidth = 50, long nMinICD10CodeWidth = 50,
		CString strICD9EmptyFieldText = "", CString strICD10EmptyFieldText = "",
		short iColICD9Desc = -1, short iColICD10Desc = -1,
		bool bShowDescriptionColumnsInCrosswalk = false,
		bool bAllowCodeWidthAuto = true,
		bool bRenameDescriptionColumns = false);
}