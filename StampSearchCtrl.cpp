// StampSearchCtrl.cpp : implementation file
//

#include "StdAfx.h"
#include "StampSearchCtrl.h"
#include "StampSearchDlg.h"
#include <foreach.h>
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <string>

// (a.walling 2012-08-28 08:15) - PLID 52320 - HTML interface container for interacting with data

// CStampSearchCtrl

IMPLEMENT_DYNAMIC(CStampSearchCtrl, CNxHtmlControl)

CStampSearchCtrl::CStampSearchCtrl()
{
}

CStampSearchCtrl::~CStampSearchCtrl()
{
}


BEGIN_MESSAGE_MAP(CStampSearchCtrl, CNxHtmlControl)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CStampSearchCtrl, CNxHtmlControl)
	DISP_FUNCTION(CStampSearchCtrl, "GetAvailableStampsJson", GetAvailableStampsJson, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CStampSearchCtrl, "OnStampClicked", OnStampClicked, VT_EMPTY, VTS_I4)
END_DISPATCH_MAP()


// CStampSearchCtrl message handlers

void CStampSearchCtrl::OnInitializing()
{
	LoadUrl("nxres://0/StampSearch.html");
}

HRESULT CStampSearchCtrl::OnShowContextMenu(DWORD dwID, LPPOINT ppt,
	LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
	return S_OK;
}

void CStampSearchCtrl::OnContextMenu(CWnd* pWnd, CPoint pos)
{
}

BSTR CStampSearchCtrl::GetAvailableStampsJson()
{
	CString strStamps;

	try {
		GetMainFrame()->LoadEMRImageStamps();

		strStamps.Preallocate( (GetMainFrame()->m_aryEMRImageStamps.GetSize() + 20) * 96 );
		strStamps.AppendChar('[');

		// (a.walling 2012-08-28 10:23) - PLID 51742 - stamp category name now in the EMRImageStamp structure
		
		CString strNoCategory = "(Uncategorized)";
		
		// (a.walling 2012-08-28 08:15) - PLID 52322 - Recent stamps are ordered ascending, add in reverse
		reverse_foreach (EMRImageStamp* pStamp, GetRecentStamps()) {
			if (pStamp->bInactive) continue;
			strStamps.AppendFormat(
				"{ "
					"id: %li, "
					"category: %s, "
					"shortText: %s, "
					"longText: %s, "
					"description: %s, "
					"color: %s, "
					"hasImage: %s "		// (a.walling 2012-09-04 12:10) - PLID 52439 - Notify document if an image is available
				"},\r\n"
				, pStamp->nID
				, "'(Recent)'"
				, js::Stringify(pStamp->strStampText)
				, js::Stringify(pStamp->strTypeName)
				, js::Stringify(pStamp->strDescription)
				, js::ColorStringify(pStamp->nTextColor)
				, js::Boolify(pStamp->m_ImageInfo.arImageBytes && pStamp->m_ImageInfo.nNumImageBytes)
			);
		}

		// (a.walling 2012-08-28 08:15) - PLID 52322 - Now just continue adding stamps as normal
		foreach (EMRImageStamp* pStamp, GetMainFrame()->m_aryEMRImageStamps) {
			if (pStamp->bInactive) continue;

			strStamps.AppendFormat(
				"{ "
					"id: %li, "
					"category: %s, "
					"shortText: %s, "
					"longText: %s, "
					"description: %s, "
					"color: %s, "
					"hasImage: %s "		// (a.walling 2012-09-04 12:10) - PLID 52439 - Notify document if an image is available
				"},\r\n"
				, pStamp->nID
				, js::Stringify(pStamp->strCategoryName.IsEmpty() ? strNoCategory : pStamp->strCategoryName) // (a.walling 2012-08-28 10:23) - PLID 51742 - stamp category name now in the EMRImageStamp structure
				, js::Stringify(pStamp->strStampText)
				, js::Stringify(pStamp->strTypeName)
				, js::Stringify(pStamp->strDescription)
				, js::ColorStringify(pStamp->nTextColor)
				, js::Boolify(pStamp->m_ImageInfo.arImageBytes && pStamp->m_ImageInfo.nNumImageBytes)
			);
		}
	
		strStamps.TrimRight(",\r\n");

		strStamps.AppendChar(']');
	} NxCatchAll(__FUNCTION__);
	
	return strStamps.AllocSysString();
}

// (a.walling 2012-08-28 08:15) - PLID 52322 - Recent stamps
// sorted ascending, old to new
std::vector<EMRImageStamp*> CStampSearchCtrl::GetRecentStamps()
{
	using namespace std;
	using namespace boost;

	vector<EMRImageStamp*> recentStamps;
	vector<long> recentStampIDs;
	map<long, EMRImageStamp*> stampPtrs;

	string strStamps = GetRemotePropertyText("EMR.StampSearch.RecentStamps", "", 0, GetCurrentUserName());

	foreach (string strStampID, tokenizer<escaped_list_separator<char>>(strStamps)) {
		recentStampIDs.push_back(lexical_cast<long>(strStampID));
		stampPtrs[recentStampIDs.back()] = NULL;
	}

	long toFind = stampPtrs.size();

	if (!toFind) {
		return recentStamps;
	}

	foreach (EMRImageStamp* pStamp, GetMainFrame()->m_aryEMRImageStamps) {
		if (stampPtrs.count(pStamp->nID)) {
			stampPtrs[pStamp->nID] = pStamp;
			if (0 == --toFind) {
				break;
			}
		}
	}

	foreach (long nStampID, recentStampIDs) {
		EMRImageStamp* pStamp = stampPtrs[nStampID];
		if (!pStamp || pStamp->bInactive) {
			continue;
		}
		recentStamps.push_back(pStamp);
	}

	return recentStamps;
}

// (a.walling 2012-08-28 08:15) - PLID 52322 - Recent stamps
// sorted ascending, old to new
void CStampSearchCtrl::SaveRecentStamps(long nNewStampID)
{
	try {
		std::vector<EMRImageStamp*> recentStamps = GetRecentStamps();
		std::vector<long> recentStampIDs;

		recentStampIDs.reserve(recentStamps.size() + 1);

		foreach (EMRImageStamp* pStamp, recentStamps) {
			if (pStamp->nID != nNewStampID) {
				recentStampIDs.push_back(pStamp->nID);
			}
		}

		if (-1 != nNewStampID) {
			recentStampIDs.push_back(nNewStampID);
		}

		int firstIndex = recentStampIDs.size() - 20;
		if (firstIndex < 0) {
			firstIndex = 0;
		}

		CString strRecentStampIDs;
		for(std::vector<long>::iterator it = recentStampIDs.begin() + firstIndex; it != recentStampIDs.end(); ++it) {
			strRecentStampIDs.AppendFormat("%li,", *it);
		}
		strRecentStampIDs.TrimRight(",");
		SetRemotePropertyText("EMR.StampSearch.RecentStamps", strRecentStampIDs, 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

void CStampSearchCtrl::OnStampClicked(long nStampID)
{
	try {
		SaveRecentStamps(nStampID);
		// (a.walling 2012-08-28 08:15) - PLID 52321 - Notify the host dialog
		static_cast<CStampSearchDlg*>(GetParent())->OnStampClicked(nStampID);
	} NxCatchAll(__FUNCTION__);
}
