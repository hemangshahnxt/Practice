#include "stdafx.h"
#include "AsyncDiagSearchQuery.h"
#include "NxAPI.h"

using namespace NexTech_Accessor;

UINT AsyncDiagSearchQuery::NotifyMessage = ::RegisterWindowMessage("Nx::AsyncDiagSearchQuery::Notify");

// (j.armen 2014-03-10 08:32) - PLID 61284 - Runs an async search and posts the data back to the main thread
// (j.armen 2014-03-20 09:58) - PLID 60943 - Made this generic such that it can search for 9's or 10's.
void AsyncDiagSearchQuery::Run(HWND hwndNotify, const CString& strSearchText, const eDiagSearchType& eDiagSearchType)
{
	try
	{
		_DiagSearchResultsPtr pDiagResults;

		DiagSearchType eAPIDiagSearchType;
		switch(eDiagSearchType)
		{
			case ManagedICD9Only:
				eAPIDiagSearchType = DiagSearchType_ManagedICD9Only;
				break;
			case FullICD10Only:
				eAPIDiagSearchType = DiagSearchType_FullICD10Only;
				break;
		}

		// Only execute search if we have results
		if(!strSearchText.IsEmpty() && eDiagSearchType != INVALID)
			pDiagResults = GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), eAPIDiagSearchType, false, _bstr_t(strSearchText), -1);

		if(pDiagResults && pDiagResults->SearchResults)
		{
			Nx::SafeArray<IUnknown *> saResults = pDiagResults->SearchResults;

			for each(_DiagSearchResultPtr pResult in saResults)
			{
				//convert our results to variants, with Null for no value
				std::pair<shared_ptr<DiagCode9>, shared_ptr<DiagCode10>> code;

				_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
				if(pICD9Code) {
					
					code.first = make_shared<DiagCode9>();

					code.first->strCode = pICD9Code->Code;
					code.first->strDescription = pICD9Code->description;

					CString strICD9_DiagCodesID = (LPCTSTR)pICD9Code->DiagCodesID;
					if(!strICD9_DiagCodesID.IsEmpty())
					{
						long nDiagCodeID = atoi(strICD9_DiagCodesID);
						if(nDiagCodeID > 0) {
							code.first->nDiagCodesID = nDiagCodeID;
						} else {
							ThrowNxException("Diag Code Search Callback: Expected Diag Code ID");
						}
					} else {
						code.first->nDiagCodesID = -2;
					}

					code.first->nBackColor = RGB(255,255,255);
					CString strColor = (LPCTSTR)pResult->Color;
					if(strColor.GetLength() == 7 && strColor.Left(1) == "#") {
						code.first->nBackColor = ConvertHexStringToCOLORREF(strColor);
					}
				}

				_SearchDiagCodePtr pICD10Code = pResult->ICD10Code;
				if(pICD10Code) {
					
					code.second = make_shared<DiagCode10>();

					code.second->strCode = pICD10Code->Code;
					code.second->strDescription = pICD10Code->description;
					code.second->vbPCS = pResult->PCS;

					CString strICD10_DiagCodesID = (LPCTSTR)pICD10Code->DiagCodesID;
					if(!strICD10_DiagCodesID.IsEmpty())
					{
						long nDiagCodeID = atoi(strICD10_DiagCodesID);
						if(nDiagCodeID > 0) {
							code.second->nDiagCodesID = nDiagCodeID;
						} else {
							ThrowNxException("Diag Code Search Callback: Expected Diag Code ID");
						}
					} else {
						code.second->nDiagCodesID = -2;
					}

					code.second->nBackColor = RGB(255,255,255);
					CString strColor = (LPCTSTR)pResult->Color;
					if(strColor.GetLength() == 7 && strColor.Left(1) == "#") {
						code.second->nBackColor = ConvertHexStringToCOLORREF(strColor);
					}
				}

				aryCodes.push_back(code);
			}
		}

		if(aryCodes.empty())
		{
			//add a "no results found" row
			std::pair<shared_ptr<DiagCode9>, shared_ptr<DiagCode10>> code;

			switch(eDiagSearchType)
			{
				case ManagedICD9Only:
					code.first = make_shared<DiagCode9>();
					code.first->nDiagCodesID = -1;
					code.first->strDescription =  "< No Results Found >";
					code.first->nBackColor = RGB(255,255,255);	//white
					break;
				case FullICD10Only:
					code.second = make_shared<DiagCode10>();
					code.second->nDiagCodesID = -1;
					code.second->strDescription =  "< No Results Found >";
					code.second->nBackColor = RGB(255,255,255);	//white
					break;
			}
			
			aryCodes.push_back(code);
		}

		// Post back to the main thread
		::PostMessage(hwndNotify, AsyncDiagSearchQuery::NotifyMessage, (WPARAM)0, (LPARAM)this);

	}NxCatchAllThread(__FUNCTION__);
}