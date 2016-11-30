// (d.thompson 2014-02-05) - PLID 60638 - Created
#include "StdAfx.h"
#include "mainfrm.h"
#include "DiagSearchUtils.h"
#include "DiagSearch_DataListProvider.h"
#include "GlobalStringUtils.h"
#include "DiagSearchConfig.h"


using namespace NXDATALIST2Lib;


namespace DiagSearchUtils
{
	//Get a pointer to the global preference-based search config object.  Only 1 of these should exist per application.
	//	This function is internal to this cpp and should not be needed elsewhere.
	CDiagSearchConfig* GetDiagPreferenceSearchConfig()
	{
		CMainFrame *pMainFrm = GetMainFrame();
		if(!pMainFrm) {
			ThrowNxException("GetDiagPreferenceSearchConfig:  No application could be found!");
		}
		return pMainFrm->GetDiagPreferenceSearchConfig();
	}


	//Get a pointer to the global dual search config object.  Only 1 of these should exist.
	//	This function is internal to this cpp and should not be needed elsewhere.
	CDiagSearchConfig* GetDiagDualSearchConfig()
	{
		CMainFrame *pMainFrm = GetMainFrame();
		if(!pMainFrm) {
			ThrowNxException("GetDiagDualSearchConfig:  No application could be found!");
		}
		return pMainFrm->GetDiagDualSearchConfig();
	}

	//Call this to determine the present style of search.  This returns the configured value that all lists 
	//	created in the application should be following.
	DiagCodeSearchStyle GetPreferenceSearchStyle()
	{
		if(!GetDiagPreferenceSearchConfig()) {
			ThrowNxException("GetPreferenceSearchStyle:  No style has been defined!");
		}
		DiagCodeSearchStyle eSearchStyle = GetDiagPreferenceSearchConfig()->GetSearchStyle();

		// (j.jones 2014-02-18 17:11) - PLID 60719 - More style enumerations exist than the
		// preference currently allows, and a lot of code assumes that the preference can
		// only return one of these three styles.
		if(eSearchStyle != eICD9_10_Crosswalk && eSearchStyle != eManagedICD9_Search && eSearchStyle != eManagedICD10_Search) {
			//A new preference style option was added but not supported here.
			//Add it to this if statement and make sure all calls to GetPreferenceSearchStyle()
			//can support the new style you've hadded.
			ASSERT(FALSE);

			//revert to Crosswalk if this happens
			eSearchStyle = eICD9_10_Crosswalk;
		}

		return eSearchStyle;
	}

	// (j.jones 2014-03-03 11:56) - PLID 61136 - added PCS subpreference
	bool GetPreferenceIncludePCS()
	{
		if(!GetDiagPreferenceSearchConfig()) {
			ThrowNxException("GetPreferenceIncludePCS:  No search has been defined!");
		}
		return GetDiagPreferenceSearchConfig()->GetIncludePCS();
	}

	//Do the work of calling the appriate configuration's search results conversion.
	CDiagSearchResults Common_ConvertPreferenceSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow, CDiagSearchConfig *pSearchConfig)
	{
		if(!pSearchConfig) {
			ThrowNxException("Common_ConvertPreferenceSearchResults:  No diagnosis search configuration has been specified!");
		}

		return pSearchConfig->ConvertRowToSearchResults(pSearchRow);
	}

	//See .h for usage
	CDiagSearchResults ConvertPreferenceSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
	{
		CDiagSearchConfig *pConfig = GetDiagPreferenceSearchConfig();
		return Common_ConvertPreferenceSearchResults(pSearchRow, pConfig);
	}

	//See .h for usage
	CDiagSearchResults ConvertDualSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
	{
		CDiagSearchConfig *pConfig = GetDiagDualSearchConfig();
		return Common_ConvertPreferenceSearchResults(pSearchRow, pConfig);
	}

	//This is called by BindDiagPreferenceSearchListCtrl, and does not need to be called again.  This function is internal 
	//	to this cpp only and should not be called directly by anyone else.
	void InitializeDiagSearchList(NXDATALIST2Lib::_DNxDataListPtr pDataList, LPUNKNOWN pDataConn, CDiagSearchConfig* pSearchConfig)
	{
		//throw all exceptions to the caller
		if(pDataList == NULL) {
			ThrowNxException("InitializeDiagSearchList called on a null datalist.");
		}

		ADODB::_ConnectionPtr pCon = pDataConn;
		if(pCon == NULL) {
			//currently no diag search list should be created without a valid connection
			ASSERT(FALSE);
			ThrowNxException("InitializeDiagSearchList called with a null connection.");
		}

		//clear out the existing list contents and columns
		pDataList->Clear();

		while(pDataList->GetColumnCount() > 0) {
			pDataList->RemoveColumn(0);
		}

		//Just call the config implementation for the enabled
		//	diag search type, it has already implemented the creation of the list properties, columns, 
		//	etc for this type.
		pSearchConfig->CreateResultList(pDataList, pCon);
	}

	LPUNKNOWN CommonBindSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, CDiagSearchConfig* pSearchConfig)
	{
		//We can utilize the standard bind to do the majority of the work for us, then tack on a couple specific search
		//	things afterwards.
		//Bind will do this for us:
		//	1)  Bind the control from the IDC
		//	2)  Apply the connection, if necessary
		//	3)  Requery if needed (which will never happen for a search dialog)
		//	4)  Some fancy exception handling on various failures for the above.

		//Bind it as normal.  Search view is only supported on datalist2's.
		NXDATALIST2Lib::_DNxDataListPtr pdl = ::BindNxDataList2Ctrl(pParent, nID, pDataConn, false);

		//Now do our special search things
		if (pdl) {
			//if the datalist is not using the search list, fail
			if(pdl->GetViewType() != NXDATALIST2Lib::vtSearchList) {
				//this would be a coding failure at design time
				ThrowNxException("CommonBindSearchListCtrl called on a datalist that is not a Search List.");
			}

			//initialize the datalist for our search style,
			//which will also set our DataListProvider for us
			InitializeDiagSearchList(pdl, pDataConn, pSearchConfig);
		}

		// If we made it here we have success or it failed in ::Bind
		return pdl;
	}


	//Bind the preference based search control
	LPUNKNOWN BindDiagPreferenceSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn)
	{
		return CommonBindSearchListCtrl(pParent, nID, pDataConn, GetDiagPreferenceSearchConfig());
	}

	//Bind the dual search control
	LPUNKNOWN BindDiagDualSearchListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn)
	{
		return CommonBindSearchListCtrl(pParent, nID, pDataConn, GetDiagDualSearchConfig());
	}

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
	//bShowDescriptionColumnsInCrosswalk	- by default, crosswalk lists won't show the description columns,
	//										set this to true if you want the descriptions to show in crosswalk mode
	//bAllowCodeWidthAuto					- By default, if no descriptions are shown, the code columns will be set
	//										to csWidthAuto. Set this to false if you never want this to happen.
	//bRenameDescriptionColumns				- if set to true, description columns will rename to say "Description"
	//										in ICD-9 or 10 mode, and "ICD-9 Description" (or 10) in crosswalk mode
	DiagCodeSearchStyle SizeDiagnosisListColumnsBySearchPreference(NXDATALIST2Lib::_DNxDataListPtr pDataList,
		short iColICD9Code, short iColICD10Code,
		long nMinICD9CodeWidth /*= 50*/, long nMinICD10CodeWidth /*= 50*/,
		CString strICD9EmptyFieldText /*= ""*/, CString strICD10EmptyFieldText /*= ""*/,
		short iColICD9Desc /*= -1*/, short iColICD10Desc /*= -1*/,
		bool bShowDescriptionColumnsInCrosswalk /*= false*/,
		bool bAllowCodeWidthAuto /*= true*/,
		bool bRenameDescriptionColumns /*= false*/)
	{
		//If one empty field text was provided, why wasn't the other?
		//There is no situation where one would be filled and one would not be.
		//If you truly do need this, remove the assertion.
		ASSERT(strICD9EmptyFieldText.IsEmpty() == strICD10EmptyFieldText.IsEmpty());

		//get the desired search style, this is cached at startup in MainFrame
		DiagCodeSearchStyle eSearchStyle = GetPreferenceSearchStyle();

		try {

			//confirm this is a valid style - there are more style enumerations than are allowed
			//to be return values from GetPreferenceSearchStyle
			if(eSearchStyle != eICD9_10_Crosswalk && eSearchStyle != eManagedICD9_Search && eSearchStyle != eManagedICD10_Search) {
				//This function should never be called on lists that don't follow the preference search style,
				//which can never be any style but crosswalk, Managed ICD-9, or Managed ICD-10.
				//If it is ever anything else, this code may need to be changed to reflect the new style option.
				ASSERT(FALSE);

				//revert to Crosswalk if this happens
				eSearchStyle = eICD9_10_Crosswalk;
			}

			//If the style is not the crosswalk, we would normally show only ICD-9 or
			//ICD-10, based on their search style. But if this patient has a code in
			//the hidden column, we need to show both columns, just like in crosswalk mode.
			if(eSearchStyle != eICD9_10_Crosswalk) {
				bool bHasICD9 = false;
				bool bHasICD10 = false;
				NXDATALIST2Lib::IRowSettingsPtr pRow = pDataList->GetFirstRow();
				while(pRow != NULL && (!bHasICD9 || !bHasICD10)) {
					CString strICD9Code = VarString(pRow->GetValue(iColICD9Code), "");
					CString strICD10Code = VarString(pRow->GetValue(iColICD10Code), "");

					//if the code field is not empty, and it's not the placeholder text
					//for an empty field, then we must display this column
					if(!strICD9Code.IsEmpty() && strICD9Code != strICD9EmptyFieldText) {
						bHasICD9 = true;
					}
					if(!strICD10Code.IsEmpty() && strICD10Code != strICD10EmptyFieldText) {
						bHasICD10 = true;
					}

					pRow = pRow->GetNextRow();
				}

				//if a code exists that is not in the desired search style,
				//revert to the column layout for the crosswalk search,
				//such that both ICD-9 and ICD-10 codes are displayed
				if((eSearchStyle == eManagedICD9_Search && bHasICD10)
					|| (eSearchStyle == eManagedICD10_Search && bHasICD9)) {

					eSearchStyle = eICD9_10_Crosswalk;
				}
			}

			//now ensure our columns are correct for the desired search style

			NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD9Code = pDataList->GetColumn(iColICD9Code);		
			NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD10Code = pDataList->GetColumn(iColICD10Code);

			//get the description columns, only if indexes were provided
			
			NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD9Desc = NULL;
			NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD10Desc = NULL;
			
			if(iColICD9Desc != -1 && iColICD10Desc != -1) {
				pColDiagICD9Desc = pDataList->GetColumn(iColICD9Desc);
				pColDiagICD10Desc = pDataList->GetColumn(iColICD10Desc);
			}

			//handle the ICD-9 description column
			bool bIsShowingICD9Desc = false;
			if(pColDiagICD9Desc) {

				//rename the column, if requested
				if(bRenameDescriptionColumns) {
					if(eSearchStyle == eManagedICD9_Search) {
						pColDiagICD9Desc->PutColumnTitle("Description");
					}
					else {
						pColDiagICD9Desc->PutColumnTitle("ICD-9 Description");
					}
				}

				if(eSearchStyle == eManagedICD10_Search
					|| (eSearchStyle == eICD9_10_Crosswalk && !bShowDescriptionColumnsInCrosswalk)) {
					
					//hide the ICD-9 description column
					long nWidth = 0;
					long nStyle = csVisible|csFixedWidth;

					if(eSearchStyle == eICD9_10_Crosswalk) {
						//in crosswalk mode, allow it to be resizeable
						nStyle = csVisible;
					}

					//when hiding the column, set the style first
					pColDiagICD9Desc->PutColumnStyle(nStyle);
					pColDiagICD9Desc->PutStoredWidth(nWidth);

					bIsShowingICD9Desc = false;
				}
				else {
					//normal case, width auto
					pColDiagICD9Desc->PutColumnStyle(csVisible|csWidthAuto);

					bIsShowingICD9Desc = true;
				}
			}

			//handle the ICD-10 description column
			bool bIsShowingICD10Desc = false;
			if(pColDiagICD10Desc) {

				//rename the column, if requested
				if(bRenameDescriptionColumns) {
					if(eSearchStyle == eManagedICD10_Search) {
						pColDiagICD10Desc->PutColumnTitle("Description");
					}
					else {
						pColDiagICD10Desc->PutColumnTitle("ICD-10 Description");
					}
				}

				if(eSearchStyle == eManagedICD9_Search
					|| (eSearchStyle == eICD9_10_Crosswalk && !bShowDescriptionColumnsInCrosswalk)) {
					
					//hide the ICD-10 description column
					long nWidth = 0;
					long nStyle = csVisible|csFixedWidth;

					if(eSearchStyle == eICD9_10_Crosswalk) {
						//in crosswalk mode, allow it to be resizeable
						nStyle = csVisible;
					}

					//when hiding the column, set the style first
					pColDiagICD10Desc->PutColumnStyle(nStyle);
					pColDiagICD10Desc->PutStoredWidth(nWidth);

					bIsShowingICD10Desc = false;
				}
				else {
					//normal case, width auto
					pColDiagICD10Desc->PutColumnStyle(csVisible|csWidthAuto);

					bIsShowingICD10Desc = true;
				}
			}

			//handle the ICD-9 code column
			if(eSearchStyle == eManagedICD10_Search) {
				//hide the ICD-9 code column
				//when hiding the column, set the style first
				pColDiagICD9Code->PutColumnStyle(csVisible|csFixedWidth);
				pColDiagICD9Code->PutStoredWidth(0);
			}
			else if(bAllowCodeWidthAuto && !bIsShowingICD9Desc) {
				//if we are not showing the description column,
				//change the style to csWidthAuto
				pColDiagICD9Code->PutColumnStyle(csVisible|csWidthAuto);
			}
			else {
				//normal case, width data
				//when applying data-width, set the style last
				pColDiagICD9Code->PutStoredWidth(nMinICD9CodeWidth);
				pColDiagICD9Code->PutColumnStyle(csVisible|csWidthData);
			}

			//handle the ICD-10 code column
			if(eSearchStyle == eManagedICD9_Search) {
				//hide the ICD-10 code column
				//when hiding the column, set the style first
				pColDiagICD10Code->PutColumnStyle(csVisible|csFixedWidth);
				pColDiagICD10Code->PutStoredWidth(0);
			}
			else if(bAllowCodeWidthAuto && !bIsShowingICD10Desc) {
				//if we are not showing the description column,
				//change the style to csWidthAuto
				pColDiagICD10Code->PutColumnStyle(csVisible|csWidthAuto);
			}
			else {
				//normal case, width data
				//when applying data-width, set the style last
				pColDiagICD10Code->PutStoredWidth(nMinICD10CodeWidth);
				pColDiagICD10Code->PutColumnStyle(csVisible|csWidthData);
			}

		}NxCatchAll(__FUNCTION__);

		return eSearchStyle;
	}
}