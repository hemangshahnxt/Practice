// (j.jones 2014-02-06 11:50) - PLID 60685 - Created
#include "StdAfx.h"
#include "DiagSearchConfig_FullICD9or10Search.h"
#include "DiagSearch_DataListProvider_FullICD9or10Search.h"
#include "NxUILib\DatalistUtils.h"
using namespace NXDATALIST2Lib;

//this class will be used if the DiagCodeSearchStyle is eFullICD9or10_Search

enum ICD910ResultColumns {
	ntrcIsICD10 = 0,		//true if an ICD-10 code, false if ICD-9	
	ntrcCode,				//code number
	ntrcDescription,		//code description
	ntrcDiagCodesID,		//DiagCodes.ID for the code, could be null if it doesn't exist
	ntrcBackgroundColor,	//background row color
	ntrcPCS,				// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
};

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearchConfig_FullICD9or10Search::CDiagSearchConfig_FullICD9or10Search(bool bIncludePCS)
	: CDiagSearchConfig(eFullICD9or10_Search, bIncludePCS)
{
}


//ICD9 search specific handling of the search results.  See base class for implementation requirements.
CDiagSearchResults CDiagSearchConfig_FullICD9or10Search::ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
{
	//Callers need to handle this themselves otherwise our object is just junk
	if(!pSearchRow) {
		ThrowNxException("Cannot parse search results for NULL row.");
	}

	CDiagSearchResults results;

	//only pull ICD-9 or ICD-10, they are mutually exclusive here
	if(VarBool(pSearchRow->GetValue(ntrcIsICD10), FALSE)) {
		//ICD-10
		results.m_ICD10.m_nDiagCodesID = VarLong(pSearchRow->GetValue(ntrcDiagCodesID), -1);
		results.m_ICD10.m_strCode = VarString(pSearchRow->GetValue(ntrcCode), "");
		results.m_ICD10.m_strDescription = VarString(pSearchRow->GetValue(ntrcDescription), "");

		//if the code is empty, force the description to be blank, not <No Matching Code>
		if(results.m_ICD10.m_strCode.IsEmpty()) {
			results.m_ICD10.m_strDescription = "";
		}
	}
	else {
		//ICD-9
		results.m_ICD9.m_nDiagCodesID = VarLong(pSearchRow->GetValue(ntrcDiagCodesID), -1);
		results.m_ICD9.m_strCode = VarString(pSearchRow->GetValue(ntrcCode), "");
		results.m_ICD9.m_strDescription = VarString(pSearchRow->GetValue(ntrcDescription), "");

		//if the code is empty, force the description to be blank, not <No Matching Code>
		if(results.m_ICD9.m_strCode.IsEmpty()) {
			results.m_ICD9.m_strDescription = "";
		}
	}

	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS bit
	results.m_bPCS = VarBool(pSearchRow->GetValue(ntrcPCS), FALSE) ? true : false;

	// (d.thompson 2014-02-10) - PLID 60716 - Ensure the records exist in DiagCodes
	EnsureDataInDiagCodes(&results);

	//And we're done
	return results;
}

//ICD9 search specific implementation to create the dropdown list component of a search datalist
void CDiagSearchConfig_FullICD9or10Search::CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Call the base class implementation for basic checks.  This will throw exceptions if any problems are found.
	CreateResultListBase(pDataList, pCon);

	//create our ICD 9 & 10 search specific columns	

	IColumnSettingsPtr pColIsICD10 = pDataList->GetColumn(pDataList->InsertColumn(ntrcIsICD10, _T("IsICD10"), _T("ICD-10?"), 0, csVisible|csFixedWidth));
	pColIsICD10->FieldType = cftBoolYesNo;	
	IColumnSettingsPtr pColCode = pDataList->GetColumn(pDataList->InsertColumn(ntrcCode, _T("Code"), _T("Code"), 70, csVisible));
	pColCode->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDesc = pDataList->GetColumn(pDataList->InsertColumn(ntrcDescription, _T("Description"), _T("Description"), -1, csVisible|csWidthAuto));
	pColDesc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(ntrcDiagCodesID, _T("DiagCodesID"), _T("DiagCodesID"), 0, csVisible|csFixedWidth));
	pColDiagCodesID->FieldType = cftTextSingleLine;
	pColDiagCodesID->DataType = VT_I4;
	IColumnSettingsPtr pColBackgroundColorDesc = pDataList->GetColumn(pDataList->InsertColumn(ntrcBackgroundColor, _T("BackColor"), _T("BackColor"), 0, csVisible|csFixedWidth));
	pColBackgroundColorDesc->FieldType = cftSetRowBackColor;
	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
	IColumnSettingsPtr pPCS = pDataList->GetColumn(pDataList->InsertColumn(ntrcPCS, _T("PCS"), _T("PCS"), 0, csVisible));
	pPCS->FieldType = cftBoolYesNo;

	//set the clauses to nothing, we won't use them
	pDataList->FromClause = _bstr_t("");
	pDataList->WhereClause = _bstr_t("");

	pDataList->PutViewType(NXDATALIST2Lib::vtSearchList);
	pDataList->PutAutoDropDown(VARIANT_FALSE);

	//(s.dhole 2/27/2015 10:49 AM ) - PLID 64549
	//force the dropdown width to be 350
	SetMinDatalistDropdownWidth(pDataList, 350);

	//set the provider
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearch_DataListProvider* provider = new CDiagSearch_DataListProvider_FullICD9or10Search(pCon, m_bIncludePCS);

	// (s.dhole 3/6/ 2015 9:42 AM) - PLID 64610
	// Tell the provider to interact with our datalist
	provider->Register(pDataList);

	//Make sure to release to avoid memory leaks!
	provider->Release();

	//The API returns records in sorted order. We do not need to sort.
}
