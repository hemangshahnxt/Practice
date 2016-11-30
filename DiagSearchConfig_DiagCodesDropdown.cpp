// (d.thompson 2014-02-04) - PLID 60668 - Created
#include "StdAfx.h"
#include "DiagSearchConfig_DiagCodesDropdown.h"
#include "DiagSearch_DataListProvider_DiagCodesDropdown.h"

using namespace NXDATALIST2Lib;

//this class will be used if the DiagCodeSearchStyle is eDiagCodes_Dropdown

enum DiagCodesResultColumns {
	ddcIsICD10 = 0,		//DiagCodes.ICD10
	ddcCode,			//DiagCodes.CodeNumber
	ddcDescription,		//DiagCodes.Description
	ddcDiagCodesID,		//DiagCodes.ID, never null in this class
	ddcBackgroundColor,	//background row color
};

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
// though this search does not utilize it
CDiagSearchConfig_DiagCodesDropdown::CDiagSearchConfig_DiagCodesDropdown(bool bIncludePCS)
	: CDiagSearchConfig(eDiagCodes_Dropdown, bIncludePCS)
{
}


//ICD9 search specific handling of the search results.  See base class for implementation requirements.
CDiagSearchResults CDiagSearchConfig_DiagCodesDropdown::ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow)
{
	//Callers need to handle this themselves otherwise our object is just junk
	if(!pSearchRow) {
		ThrowNxException("Cannot parse search results for NULL row.");
	}

	CDiagSearchResults results;

	//only pull ICD-9 or ICD-10, they are mutually exclusive here
	if(VarBool(pSearchRow->GetValue(ddcIsICD10), FALSE)) {
		//ICD-10
		results.m_ICD10.m_nDiagCodesID = VarLong(pSearchRow->GetValue(ddcDiagCodesID), -1);
		results.m_ICD10.m_strCode = VarString(pSearchRow->GetValue(ddcCode), "");
		results.m_ICD10.m_strDescription = VarString(pSearchRow->GetValue(ddcDescription), "");

		//if the code is empty, force the description to be blank, not <No Matching Code>
		if(results.m_ICD10.m_strCode.IsEmpty()) {
			results.m_ICD10.m_strDescription = "";
		}
	}
	else {
		//ICD-9
		results.m_ICD9.m_nDiagCodesID = VarLong(pSearchRow->GetValue(ddcDiagCodesID), -1);
		results.m_ICD9.m_strCode = VarString(pSearchRow->GetValue(ddcCode), "");
		results.m_ICD9.m_strDescription = VarString(pSearchRow->GetValue(ddcDescription), "");

		//if the code is empty, force the description to be blank, not <No Matching Code>
		if(results.m_ICD9.m_strCode.IsEmpty()) {
			results.m_ICD9.m_strDescription = "";
		}
	}

	//not needed, because this is a list of DiagCodes-only entries
	//EnsureDataInDiagCodes(&results);

	//And we're done
	return results;
}

//DiagCodes search specific implementation to create the dropdown list component of a search datalist
void CDiagSearchConfig_DiagCodesDropdown::CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Call the base class implementation for basic checks.  This will throw exceptions if any problems are found.
	CreateResultListBase(pDataList, pCon);

	//create our diagcodes dropdown specific columns
	
	//this is not fixed-width
	IColumnSettingsPtr pColIsICD10 = pDataList->GetColumn(pDataList->InsertColumn(ddcIsICD10, _T("ICD10"), _T("ICD-10?"), 0, csVisible));
	pColIsICD10->FieldType = cftBoolYesNo;	
	IColumnSettingsPtr pColCode = pDataList->GetColumn(pDataList->InsertColumn(ddcCode, _T("CodeNumber"), _T("Code"), 70, csVisible));
	pColCode->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDesc = pDataList->GetColumn(pDataList->InsertColumn(ddcDescription, _T("CodeDesc"), _T("Description"), -1, csVisible|csWidthAuto));
	pColDesc->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColDiagCodesID = pDataList->GetColumn(pDataList->InsertColumn(ddcDiagCodesID, _T("ID"), _T("DiagCodesID"), 0, csVisible|csFixedWidth));
	pColDiagCodesID->FieldType = cftTextSingleLine;
	IColumnSettingsPtr pColBackgroundColorDesc = pDataList->GetColumn(pDataList->InsertColumn(ddcBackgroundColor, _T("BackColor"), _T("BackColor"), 0, csVisible|csFixedWidth));
	pColBackgroundColorDesc->FieldType = cftSetRowBackColor;

	//set the clauses up, the dropdown uses them instead of a provider

	CString strFromClause;
	strFromClause.Format("(SELECT ID, CodeNumber, CodeDesc, ICD10, %li AS BackColor FROM DiagCodes WHERE Active = 1) AS DropdownQ", (long)RGB(255,255,255));
	pDataList->FromClause = _bstr_t(strFromClause);
	pDataList->WhereClause = _bstr_t("");

	//this is a dropdown list, not a search
	pDataList->PutViewType(NXDATALIST2Lib::vtDropdownList);

	//set the dropdown settings
	pDataList->PutAutoDropDown(VARIANT_TRUE);
	pDataList->PutTextSearchCol(ddcCode);
	pDataList->PutDisplayColumn("1");

	//force the dropdown width to be 350
	SetMinDropdownWidth(pDataList, 350);

	//Initialize the sort order to be code then description,
	//with no split by ICD-9 or 10.
	pDataList->GetColumn(ddcCode)->PutSortPriority(0);
	pDataList->GetColumn(ddcDescription)->PutSortPriority(1);

	pDataList->Requery();
}
