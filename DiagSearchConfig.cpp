// (d.thompson 2014-02-04) - PLID 60638 - Created
#include "StdAfx.h"
#include "DiagSearchConfig.h"
#include "NxException.h"
#include "NxAPI.h"
#include "GlobalUtils.h"

//Base constructor.  When used (and it shouldn't be able to, because this is an abstract class), 
//	the search style is set to invalid.
// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearchConfig::CDiagSearchConfig(bool bIncludePCS) :
	m_eSearchStyle(eInvalidCodeSearchStyle),
	m_bIncludePCS(bIncludePCS)
{
}

//Provided for derived classes to use during construction and set the const search style.
// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearchConfig::CDiagSearchConfig(DiagCodeSearchStyle eSS, bool bIncludePCS) : 
	m_eSearchStyle(eSS),
	m_bIncludePCS(bIncludePCS)
{
}

//Simply returns the search style.  You should not need to override this in any base classes, just
//	make sure the style is set properly in your constructor.
DiagCodeSearchStyle CDiagSearchConfig::GetSearchStyle()
{
	return m_eSearchStyle;
}

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
bool CDiagSearchConfig::GetIncludePCS()
{
	return m_bIncludePCS;
}

//See .h for details when to call this.
void CDiagSearchConfig::CreateResultListBase(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon)
{
	//Do not allow this if the datalist is NULL
	if(pDataList == NULL) {
		ThrowNxException("CreateResultListBase called on a null pDataList.");
	}

	//Do not allow for a NULL connection
	if(pCon == NULL) {
		ThrowNxException("CreateResultListBase called with a null connection.");
	}

	//the caller should have cleared all columns & rows,
	//if they didn't, then this function was called improperly
	if(pDataList->GetColumnCount() > 0 || pDataList->GetRowCount() > 0) {
		//make sure this function is never called outside InitializeDiagSearchList,
		//which wipes the existing content before recreating it
		ASSERT(FALSE);
		ThrowNxException("CreateResultListBase called on a non-empty pDataList.");
	}

	//all diagnosis search lists have the same placeholder text
	pDataList->PutSearchPlaceholderText("Diagnosis Search...");
}

// (j.jones 2015-03-23 13:45) - PLID 65281 - moved SetMinDropdownWidth to GlobalUtils

// (d.thompson 2014-02-10) - PLID 60716
//Ensures that all codes exist in DiagCodes table.  See .h for detailed comments.
void CDiagSearchConfig::EnsureDataInDiagCodes(IN OUT CDiagSearchResults *pResults)
{
	//This function simply evaluates that the records exist in DiagCodes.  If they already
	//	have an ID, we can skip them.  If they don't, we need to create that record and fill the ID.
	if(!pResults) {
		ThrowNxException("EnsureDataInDiagCodes:  No results given!");
	}

	//Use the API to insert new diagnosis codes
	Nx::SafeArray<IUnknown *> aryCommits;

	//1)  ICD-9 first
	//	We only operate if there is no diag codes ID already assigned, and the code isn't empty.
	if(pResults->m_ICD9.m_nDiagCodesID == -1 && !pResults->m_ICD9.m_strCode.IsEmpty()) {
		NexTech_Accessor::_DiagnosisCodeCommitPtr pCode(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
		pCode->Code = _bstr_t(pResults->m_ICD9.m_strCode);
		pCode->description = _bstr_t(pResults->m_ICD9.m_strDescription);
		pCode->ICD10 = false;
		// (j.jones 2014-03-04 09:18) - PLID 61136 - added PCS bit
		pCode->PCS = pResults->m_bPCS;

		aryCommits.Add(pCode);
	}

	//2)  ICD-10 second
	//	We only operate if there is no diag codes ID already assigned, and the code isn't empty.
	if(pResults->m_ICD10.m_nDiagCodesID == -1 && !pResults->m_ICD10.m_strCode.IsEmpty()) {
		NexTech_Accessor::_DiagnosisCodeCommitPtr pCode(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
		pCode->Code = _bstr_t(pResults->m_ICD10.m_strCode);
		pCode->description = _bstr_t(pResults->m_ICD10.m_strDescription);
		pCode->ICD10 = true;
		// (j.jones 2014-03-04 09:18) - PLID 61136 - added PCS bit
		pCode->PCS = pResults->m_bPCS;

		aryCommits.Add(pCode);
	}

	//If we got anything that needs created, do so now.
	if(aryCommits.GetCount() > 0) {
		NexTech_Accessor::_DiagnosisCodesPtr pCodesCreated = GetAPI()->CreateDiagnosisCodes(GetAPISubkey(), GetAPILoginToken(), aryCommits);

		//Safety check:  We should have gotten some codes back, we sent some in to be created.  Tell the user to try again.
		if(pCodesCreated->Codes == NULL) {
			ThrowNxException("EnsureDataInDiagCodes:  Did not get any results for code creation.  Please attempt your search again.");
		}

		Nx::SafeArray<IUnknown *> aryResults = pCodesCreated->Codes;

		//Safety check:  Ensure we got the same # of results as we passed in requests.  Not sure why this would happen, but the user
		//	can search again to try again.  We're not sure what was saved.
		if(aryCommits.GetCount() != aryResults.GetCount()) {
			ThrowNxException("EnsureDataInDiagCodes:  Did not get results for all diagnosis commits.  Please attempt your search again.");
		}

		//Process the results and update our pResults objects
		for(unsigned int i = 0; i < aryResults.GetCount(); i++) {
			NexTech_Accessor::_DiagnosisCodePtr pCode = aryResults.GetAt(i);
			//This function is simplified --- we pased in 1 or 2 codes and there's a flag for "is 10" or "not 10".  We'll use that 
			//	to figure out which one we should update.
			if(pCode->ICD10 == false) {
				//Update our ICD9
				pResults->m_ICD9.m_nDiagCodesID = atoi(VarString(pCode->ID));
			}
			else {
				//Update our ICD10
				pResults->m_ICD10.m_nDiagCodesID = atoi(VarString(pCode->ID));
			}
		}
	}
}
