// MarketingDlg.cpp: implementation of the CMarketingDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MarketingDlg.h"
#include "MarketUtils.h"
#include "DocBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//TES 6/4/2008 - PLID 30206 - Moved from MarketUtils.h
int CMarketFilterInfo::SetFilters() { // (a.walling 2006-06-05 11:50) - PLID 20928 sets the docbar filters with the saved info
	if (GetMainFrame()->m_pDocToolBar) {
			POSITION posMapPosition = mapMarketFilters.GetStartPosition();
			
			while (posMapPosition != NULL) {
				int p_mf, p_mft;
				mapMarketFilters.GetNextAssoc(posMapPosition, p_mft, p_mf);
				GetMainFrame()->m_pDocToolBar->SetFilter(MarketFilter(p_mf), MarketFilterType(p_mft)); // set all the docbar filter texts
			}
		return 0;
	}
	else {
		return -1;
	}
}



CMarketingDlg::CMarketingDlg(int IDD, CWnd* pParent)
	: CNxDialog(IDD, pParent)
{

}

CMarketingDlg::~CMarketingDlg()
{
}

void CMarketingDlg::SetFilter(MarketFilter mktFilter, MarketFilterType mfType) 
{
	//TES 6/4/2008 - PLID 30206 - This is the key to fixing the bug this PLID is talking about - we need to make
	// sure that EVERY time we set a filter, we add it to m_mfiFilterInfo, thus ensuring that it will be remembered next
	// time this tab is loaded.  Before, it was being set sometimes, but not always.
	m_mfiFilterInfo.Add(mktFilter, mfType);

	//TES 6/4/2008 - PLID 30206 - Now go ahead and set the filter (moved from marketutils.cpp)
	GetMainFrame()->m_pDocToolBar->SetFilter(mktFilter, mfType);
	
}

//TES 6/4/2008 - PLID 30206 - The rest of these functions were all in marketutils, I went ahead and moved them here because 
// they were only ever used by marketing tabs.
void CMarketingDlg::EnsureFilter(MarketFilterType mft)
{
	GetMainFrame()->m_pDocToolBar->EnsureFilter(mft);
}

BOOL CMarketingDlg::UseFilter(MarketFilterType mft) 
{
	return GetMainFrame()->m_pDocToolBar->UseFilter(mft);
}

int CMarketingDlg::GetType() 
{
	return ::GetType();
}

MarketFilter CMarketingDlg::GetFilterType(MarketFilterType mft) 
{
	return GetMainFrame()->m_pDocToolBar->GetFilterType(mft);
}


int CMarketingDlg::GetDoc()
{
	return GetMainFrame()->m_pDocToolBar->GetDoc();
}

int CMarketingDlg::GetLoc()
{
	return GetMainFrame()->m_pDocToolBar->GetLoc();
}

int CMarketingDlg::GetCategory() {
	return GetMainFrame()->m_pDocToolBar->GetCategory();
}

void CMarketingDlg::SetType(int mktType) {

	GetMainFrame()->m_pDocToolBar->SetType(mktType);
	
}

