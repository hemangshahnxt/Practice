// (j.jones 2014-02-10 09:09) - PLID 60666
#include "StdAfx.h"
#include "DiagSearchConfig_Crosswalk.h"
#include "DiagSearch_DataListProvider_Crosswalk.h"
#include "NxUILib\DatalistUtils.h"

using namespace NXDATALIST2Lib;

//this class will be used if the DiagCodeSearchStyle is eICD9_10_Crosswalk

enum CrosswalkResultColumns {	
	crcICD9Code = 0,		//ICD-9 code number
	crcICD9Description,		//ICD-9 code description
	crcICD9_DiagCodesID,	//DiagCodes.ID for the ICD-9 code, could be null if it doesn't exist	
	crcICD10Code,			//ICD-10 code number
	crcICD10Description,	//ICD-10 code description
	crcICD10_DiagCodesID,	//DiagCodes.ID for the ICD-10 code, could be null if it doesn't exist
	crcMatchType,			//the EDiagCrosswalkMatchType enum, identifies if it's an exact match, no match, combination, etc.
	crcBackgroundColor,		//background row color
	crcIsQuicklist,			// (z.manning 2014-03-03 11:18) - PLID 61140 - quicklist flag
	crcPCS,					// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
};

//Call the base constructor to set the search style
// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearchConfig_Crosswalk::CDiagSearchConfig_Crosswalk(bool bIncludePCS)
	: CDiagSearchConfig(eICD9_10_Crosswalk, bIncludePCS)
{
}




//Croswalk specific handling of the search results.  See base class for implementation requirements.
CDiagSearchResults CDiagSearchConfig_Crosswalk::ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
{
	//Callers need to handle this themselves otherwise our object is just junk
	if(!pSearchRow) {
		ThrowNxException("Cannot parse search results for NULL row.");
	}

	CDiagSearchResults results;

	//Simply pull out the ICD-9 data
	results.m_ICD9.m_nDiagCodesID = VarLong(pSearchRow->GetValue(crcICD9_DiagCodesID), -1);
	results.m_ICD9.m_strCode = VarString(pSearchRow->GetValue(crcICD9Code), "");
	results.m_ICD9.m_strDescription = VarString(pSearchRow->GetValue(crcICD9Description), "");

	//if the code is empty, force the description to be blank, not <No Matching Code>
	if(results.m_ICD9.m_strCode.IsEmpty()) {
		results.m_ICD9.m_strDescription = "";
	}

	//Then the ICD-10 data
	results.m_ICD10.m_nDiagCodesID = VarLong(pSearchRow->GetValue(crcICD10_DiagCodesID), -1);
	results.m_ICD10.m_strCode = VarString(pSearchRow->GetValue(crcICD10Code), "");
	results.m_ICD10.m_strDescription = VarString(pSearchRow->GetValue(crcICD10Description), "");

	//if the code is empty, force the description to be blank, not <No Matching Code>
	if(results.m_ICD10.m_strCode.IsEmpty()) {
		results.m_ICD10.m_strDescription = "";
	}

	// (z.manning 2014-03-03 14:59) - PLID 61140 - If we have an icon in the quicklist column that means
	// it is in the user's quicklist
	_variant_t varIsQuicklist = pSearchRow->GetValue(crcIsQuicklist);
	results.m_bIsQuicklist = (varIsQuicklist.vt != VT_NULL && varIsQuicklist.vt != VT_EMPTY);

	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS bit
	results.m_bPCS = VarBool(pSearchRow->GetValue(crcPCS), FALSE) ? true : false;

	// (d.thompson 2014-02-10) - PLID 60716 - Ensure the records exist in DiagCodes
	EnsureDataInDiagCodes(&results);

	//And we're done
	return results;
}

//Crosswalk specific implementation to create the dropdown list component of a search datalist
void CDiagSearchConfig_Crosswalk::CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Call the base class implementation for basic checks.  This will throw exceptions if any problems are found.
	CreateResultListBase(pDataList, pCon);

	//create our crosswalk specific columns	
	IColumnSettingsPtr pColICD9Code = pDataList->GetColumn(pDataList->InsertColumn(crcICD9Code, _T("ICD9Code"), _T("ICD-9"), 55, csVisible));
	pColICD9Code->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColICD9Desc = pDataList->GetColumn(pDataList->InsertColumn(crcICD9Description, _T("ICD9Desc"), _T("ICD-9 Description"), 200, csVisible));
	pColICD9Desc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColICD9DiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(crcICD9_DiagCodesID, _T("ICD9_DiagCodesID"), _T("ICD-9 DiagCodesID"), 0, csVisible|csFixedWidth));
	pColICD9DiagCodesID->FieldType = cftTextSingleLine;	
	pColICD9DiagCodesID->DataType = VT_I4;
	IColumnSettingsPtr pColICD10Code = pDataList->GetColumn(pDataList->InsertColumn(crcICD10Code, _T("ICD10Code"), _T("ICD-10"), 70, csVisible));
	pColICD10Code->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColICD10Desc = pDataList->GetColumn(pDataList->InsertColumn(crcICD10Description, _T("ICD10Desc"), _T("ICD-10 Description"), 255, csVisible));
	pColICD10Desc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColICD10DiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(crcICD10_DiagCodesID, _T("ICD10_DiagCodesID"), _T("ICD-10 DiagCodesID"), 0, csVisible|csFixedWidth));
	pColICD10DiagCodesID->FieldType = cftTextSingleLine;
	pColICD10DiagCodesID->DataType = VT_I4;
	IColumnSettingsPtr pColMatchType = pDataList->GetColumn(pDataList->InsertColumn(crcMatchType, _T("MatchType"), _T("MatchType"), 0, csVisible|csFixedWidth));
	pColMatchType->FieldType = cftTextSingleLine;
	pColMatchType->DataType = VT_I4;
	IColumnSettingsPtr pColBackgroundColorDesc = pDataList->GetColumn(pDataList->InsertColumn(crcBackgroundColor, _T("BackColor"), _T("BackColor"), 0, csVisible|csFixedWidth));
	pColBackgroundColorDesc->FieldType = cftSetRowBackColor;
	// (z.manning 2014-03-03 11:25) - PLID 61140 - Quicklist flag
	IColumnSettingsPtr pIsQuicklist = pDataList->GetColumn(pDataList->InsertColumn(crcIsQuicklist, _T("IsQuicklist"), _T(""), 30, csVisible));
	pIsQuicklist->FieldType = cftBitmapBuiltIn;
	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
	IColumnSettingsPtr pPCS = pDataList->GetColumn(pDataList->InsertColumn(crcPCS, _T("PCS"), _T("PCS"), 0, csVisible));
	pPCS->FieldType = cftBoolYesNo;

	//set the clauses to nothing, we won't use them
	pDataList->FromClause = _bstr_t("");
	pDataList->WhereClause = _bstr_t("");

	pDataList->PutViewType(NXDATALIST2Lib::vtSearchList);
	pDataList->PutAutoDropDown(VARIANT_FALSE);

	//force the dropdown width to be 600
	// (z.manning 2014-03-03 14:30) - PLID 61140 - Now 630 to support quicklist icon column
	SetMinDatalistDropdownWidth(pDataList, 630);

	//(s.dhole 2/27/2015 10:49 AM ) - PLID 64549
	SetDatalistMinDropdownWidth(pDataList, 630);

	//set the provider
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearch_DataListProvider* provider = new CDiagSearch_DataListProvider_Crosswalk(pCon, m_bIncludePCS);
	
	// (s.dhole 3/6/ 2015 9:42 AM) - PLID 64610
	// Tell the provider to interact with our datalist
	provider->Register(pDataList);

	//Make sure to release to avoid memory leaks!
	provider->Release();

	//The API returns records in sorted order. We do not need to sort.
}


