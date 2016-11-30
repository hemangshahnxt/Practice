// (j.jones 2015-09-01 14:37) - PLID 66993 - Created
#include "StdAfx.h"
#include "DiagSearchConfig_FullICD10Search.h"
#include "DiagSearch_DataListProvider_FullICD10Search.h"
#include "NxUILib\DatalistUtils.h"
using namespace NXDATALIST2Lib;

//this class will be used if the DiagCodeSearchStyle is eFullICD9or10_Search

enum ICD10ResultColumns {
	ntrcCode = 0,			//code number
	ntrcDescription,		//code description
	ntrcDiagCodesID,		//DiagCodes.ID for the code, could be null if it doesn't exist
	ntrcBackgroundColor,	//background row color
	ntrcPCS,				//is a PCS code
};

CDiagSearchConfig_FullICD10Search::CDiagSearchConfig_FullICD10Search(bool bIncludePCS)
	: CDiagSearchConfig(eFullICD10_Search, bIncludePCS)
{
}


//ICD9 search specific handling of the search results.  See base class for implementation requirements.
CDiagSearchResults CDiagSearchConfig_FullICD10Search::ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
{
	//Callers need to handle this themselves otherwise our object is just junk
	if(!pSearchRow) {
		ThrowNxException("Cannot parse search results for NULL row.");
	}

	CDiagSearchResults results;

	//this is ICD-10 only
	results.m_ICD10.m_nDiagCodesID = VarLong(pSearchRow->GetValue(ntrcDiagCodesID), -1);
	results.m_ICD10.m_strCode = VarString(pSearchRow->GetValue(ntrcCode), "");
	results.m_ICD10.m_strDescription = VarString(pSearchRow->GetValue(ntrcDescription), "");

	//if the code is empty, force the description to be blank, not <No Matching Code>
	if(results.m_ICD10.m_strCode.IsEmpty()) {
		results.m_ICD10.m_strDescription = "";
	}
	results.m_bPCS = VarBool(pSearchRow->GetValue(ntrcPCS), FALSE) ? true : false;

	EnsureDataInDiagCodes(&results);

	//And we're done
	return results;
}

//ICD9 search specific implementation to create the dropdown list component of a search datalist
void CDiagSearchConfig_FullICD10Search::CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Call the base class implementation for basic checks.  This will throw exceptions if any problems are found.
	CreateResultListBase(pDataList, pCon);

	//create our ICD 10 search specific columns	
	IColumnSettingsPtr pColCode = pDataList->GetColumn(pDataList->InsertColumn(ntrcCode, _T("Code"), _T("Code"), 70, csVisible));
	pColCode->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDesc = pDataList->GetColumn(pDataList->InsertColumn(ntrcDescription, _T("Description"), _T("Description"), -1, csVisible|csWidthAuto));
	pColDesc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(ntrcDiagCodesID, _T("DiagCodesID"), _T("DiagCodesID"), 0, csVisible|csFixedWidth));
	pColDiagCodesID->FieldType = cftTextSingleLine;
	pColDiagCodesID->DataType = VT_I4;
	IColumnSettingsPtr pColBackgroundColorDesc = pDataList->GetColumn(pDataList->InsertColumn(ntrcBackgroundColor, _T("BackColor"), _T("BackColor"), 0, csVisible|csFixedWidth));
	pColBackgroundColorDesc->FieldType = cftSetRowBackColor;
	IColumnSettingsPtr pPCS = pDataList->GetColumn(pDataList->InsertColumn(ntrcPCS, _T("PCS"), _T("PCS"), 0, csVisible));
	pPCS->FieldType = cftBoolYesNo;

	//set the clauses to nothing, we won't use them
	pDataList->FromClause = _bstr_t("");
	pDataList->WhereClause = _bstr_t("");

	pDataList->PutViewType(NXDATALIST2Lib::vtSearchList);
	pDataList->PutAutoDropDown(VARIANT_FALSE);

	//force the dropdown width to be 350
	SetMinDatalistDropdownWidth(pDataList, 350);

	//set the provider
	CDiagSearch_DataListProvider* provider = new CDiagSearch_DataListProvider_FullICD10Search(pCon, m_bIncludePCS);

	// Tell the provider to interact with our datalist
	provider->Register(pDataList);

	//Make sure to release to avoid memory leaks!
	provider->Release();

	//The API returns records in sorted order. We do not need to sort.
}
