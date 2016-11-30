// (j.jones 2014-02-10 09:03) - PLID 60667 - Created
#include "StdAfx.h"
#include "DiagSearchConfig_ManagedICD10Search.h"
#include "DiagSearch_DataListProvider_ManagedICD10Search.h"
#include "NxUILib\DatalistUtils.h"

using namespace NXDATALIST2Lib;

//this class will be used if the DiagCodeSearchStyle is eManagedICD10_Search

enum ICD10ResultColumns {	
	trcCode = 0,		//ICD-10 code number
	trcDescription,		//ICD-10 code description
	trcDiagCodesID,		//DiagCodes.ID for the ICD-10 code, never null because this is a managed search
	trcBackgroundColor,	//background row color
	crcIsQuicklist,		// (z.manning 2014-03-03 11:18) - PLID 61140 - quicklist flag
};

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
// though this search does not utilize it
CDiagSearchConfig_ManagedICD10Search::CDiagSearchConfig_ManagedICD10Search(bool bIncludePCS)
	: CDiagSearchConfig(eManagedICD10_Search, bIncludePCS)
{
}


//ICD9 search specific handling of the search results.  See base class for implementation requirements.
CDiagSearchResults CDiagSearchConfig_ManagedICD10Search::ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
{
	//Callers need to handle this themselves otherwise our object is just junk
	if(!pSearchRow) {
		ThrowNxException("Cannot parse search results for NULL row.");
	}

	CDiagSearchResults results;

	//ICD-10
	results.m_ICD10.m_nDiagCodesID = VarLong(pSearchRow->GetValue(trcDiagCodesID), -1);
	results.m_ICD10.m_strCode = VarString(pSearchRow->GetValue(trcCode), "");
	results.m_ICD10.m_strDescription = VarString(pSearchRow->GetValue(trcDescription), "");

	//if the code is empty, force the description to be blank, not <No Matching Code>
	if(results.m_ICD10.m_strCode.IsEmpty()) {
		results.m_ICD10.m_strDescription = "";
	}

	// (z.manning 2014-03-03 14:59) - PLID 61140 - If we have an icon in the quicklist column that means
	// it is in the user's quicklist
	_variant_t varIsQuicklist = pSearchRow->GetValue(crcIsQuicklist);
	results.m_bIsQuicklist = (varIsQuicklist.vt != VT_NULL && varIsQuicklist.vt != VT_EMPTY);

	//not needed, because this is a list of DiagCodes-only entries
	//EnsureDataInDiagCodes(&results);

	//And we're done
	return results;
}

//ICD9 search specific implementation to create the dropdown list component of a search datalist
void CDiagSearchConfig_ManagedICD10Search::CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Call the base class implementation for basic checks.  This will throw exceptions if any problems are found.
	CreateResultListBase(pDataList, pCon);

	//create our ICD10 search specific columns	
	IColumnSettingsPtr pColCode = pDataList->GetColumn(pDataList->InsertColumn(trcCode, _T("ICD10Code"), _T("ICD-10"), 70, csVisible));
	pColCode->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDesc = pDataList->GetColumn(pDataList->InsertColumn(trcDescription, _T("ICD10Desc"), _T("ICD-10 Description"), -1, csVisible|csWidthAuto));
	pColDesc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(trcDiagCodesID, _T("ICD10_DiagCodesID"), _T("ICD-10 DiagCodesID"), 0, csVisible|csFixedWidth));
	pColDiagCodesID->FieldType = cftTextSingleLine;
	pColDiagCodesID->DataType = VT_I4;
	IColumnSettingsPtr pColBackgroundColorDesc = pDataList->GetColumn(pDataList->InsertColumn(trcBackgroundColor, _T("BackColor"), _T("BackColor"), 0, csVisible|csFixedWidth));
	pColBackgroundColorDesc->FieldType = cftSetRowBackColor;
	// (z.manning 2014-03-03 11:25) - PLID 61140 - Quicklist flag
	IColumnSettingsPtr pIsQuicklist = pDataList->GetColumn(pDataList->InsertColumn(crcIsQuicklist, _T("IsQuicklist"), _T(""), 30, csVisible));
	pIsQuicklist->FieldType = cftBitmapBuiltIn;

	//set the clauses to nothing, we won't use them
	pDataList->FromClause = _bstr_t("");
	pDataList->WhereClause = _bstr_t("");

	pDataList->PutViewType(NXDATALIST2Lib::vtSearchList);
	pDataList->PutAutoDropDown(VARIANT_FALSE);

	//(s.dhole 2/27/2015 10:49 AM ) - PLID 64549
	//force the dropdown width to be 350
	SetMinDatalistDropdownWidth(pDataList, 350);

	//set the provider
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
	// though this search does not utilize it
	CDiagSearch_DataListProvider* provider = new CDiagSearch_DataListProvider_ManagedICD10Search(pCon, m_bIncludePCS);
	// Tell the provider to interact with our datalist
	// (s.dhole 3/6/ 2015 9:42 AM) - PLID 64610
	provider->Register(pDataList);
	//Make sure to release to avoid memory leaks!
	provider->Release();

	//The API returns records in sorted order. We do not need to sort.
}
