#pragma once

// (j.jones 2015-03-16 14:27) - PLID 64926 - created

// CSearchBillingTabDlg dialog

#include <NxUILib/NxStaticIcon.h>

class CFinancialDlg;

// (j.jones 2015-03-16 16:14) - PLID 64927 - Used as a return value for SearchBillingTab
struct BillingTabSearchResult {
	long nLineID;				//the line id on the screen, could become invalid if they collapse a bill or make any changes
	long nBillID;				//the bill ID of the row we found the result in (can be -1)
	long nChargeID;				//the charge ID of the row we found the result in (can be -1)
	long nPaymentID;			//the payment ID of the row we found the result in (can be -1), if charge ID is -1 then this is an unapplied payment
	long nPayToPayID;			//the ID of a refund or adjustment applied to a payment (can be -1), nPaymentID should never be -1 in this case
	_variant_t varValue;		//the value of the cell we found
	short nColumnIndex;			//which column is this in?

	BillingTabSearchResult() {
		nLineID = -1;
		nBillID = -1;
		nChargeID = -1;
		nPaymentID = -1;
		nPayToPayID = -1;
		varValue = g_cvarNull;
		nColumnIndex = -1;
	}
};
typedef boost::shared_ptr<BillingTabSearchResult> BillingTabSearchResultPtr;
typedef std::vector<BillingTabSearchResultPtr> BillingTabSearchResults;

class CSearchBillingTabDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSearchBillingTabDlg)

public:
	//this requires CFinancialDlg as a parent
	CSearchBillingTabDlg(CFinancialDlg* pParent);
	virtual ~CSearchBillingTabDlg();

	//clears the results, optionally clears the search term
	void ClearSearchInfo(bool bClearSearchTerm);

	// (j.jones 2015-03-19 17:23) - PLID 64931 - checks to see if our search results are still
	// potentially valid, clears the results if we know they are not
	void RevalidateSearchResults();

// Dialog Data
	enum { IDD = IDD_SEARCH_BILLING_TAB_DLG };

protected:

	CFinancialDlg *m_pFinancialDlg;

	CNxEdit m_editSearchText;
	CNxIconButton m_btnSearch;
	// (j.jones 2015-03-17 11:39) - PLID 64930 - used to iterate between results
	CNxIconButton m_btnFindPrev;
	CNxIconButton m_btnFindNext;
	// (j.jones 2015-03-17 11:39) - PLID 64930 - used to show the result count and X of Y iteration
	CNxStatic m_nxstaticResultsLabel;
	// (j.jones 2015-03-19 16:31) - PLID 65399 - help icon to show tooltip text
	CNxStaticIcon m_icoInfoIcon;

	// (j.jones 2015-03-17 11:45) - PLID 64930 - tracks the current results and which result we're on
	BillingTabSearchResults m_aryResults;
	long m_nSearchResultIndex;

	// (j.jones 2015-03-17 11:48) - PLID 64930 - updates the prev/next arrows and label
	// to reflect the current m_nSearchResultIndex, and finds the cell on the billing tab
	void ReflectCurrentSearchIndex();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCancel();
	afx_msg void OnBtnSearch();
	afx_msg void OnEnChangeSearchText();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (j.jones 2015-03-17 11:39) - PLID 64930 - used to iterate between results
	afx_msg void OnBtnFindPrev();
	afx_msg void OnBtnFindNext();
};
