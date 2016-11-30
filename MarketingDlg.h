// MarketingDlg.h: interface for the CMarketingDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MARKETINGDLG_H__9B5F6674_7FF2_4AE3_83B5_967F803B5B1B__INCLUDED_)
#define AFX_MARKETINGDLG_H__9B5F6674_7FF2_4AE3_83B5_967F803B5B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum MarketFilter;
enum MarketFilterType;

//TES 6/4/2008 - PLID 30206 - Moved here from MarketUtils.
// a.walling PLID 20928 6/5/06 this struct contains a MarketFilterType and MarketFilter. This way we can recall the customized labels when switching among tabs.
struct CMarketFilterInfo {
	void Add(MarketFilter mf, MarketFilterType mft) {
		if (mf != -1 && mft != -1) {
			mapMarketFilters.SetAt( int(mft), int(mf) );
		}
	}

	int SetFilters(); // (a.walling 2006-06-05 11:50) - PLID 20928 sets the docbar filters with the saved info

protected:
	CMap<int, int, int, int> mapMarketFilters;
};

//TES 6/4/2008 - PLID 30206 - A new base class for marketing tabs, includes some functions related to the marketing toolbar.
class CMarketingDlg : public CNxDialog  
{
public:
	CMarketingDlg(int IDD, CWnd* pParent);
	virtual ~CMarketingDlg();
	
protected:
	void EnsureFilter(MarketFilterType mft);
	void SetFilter(MarketFilter mktFilter, MarketFilterType mfType);
	BOOL UseFilter(MarketFilterType mft);
	int GetType();
	MarketFilter GetFilterType(MarketFilterType mft);
	int GetDoc();
	int GetLoc();
	int GetCategory();
	void SetType(int mktType);

	//TES 6/4/2008 - PLID 30206 - Renamed with an m_ prefix, since this is a member variable.  This was declared separately
	// in each of the marketing tab dialogs, now of course it just needs to be here.
	CMarketFilterInfo m_mfiFilterInfo; // a.walling PLID 20928 6/5/06 helper class for saving and restoring filter states
};

#endif // !defined(AFX_MARKETINGDLG_H__9B5F6674_7FF2_4AE3_83B5_967F803B5B1B__INCLUDED_)
