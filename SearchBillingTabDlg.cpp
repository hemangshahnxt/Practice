// SearchBillingTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillingRc.h"
#include "SearchBillingTabDlg.h"
#include "FinancialDlg.h"

// (j.jones 2015-03-16 14:27) - PLID 64926 - created

// CSearchBillingTabDlg dialog

IMPLEMENT_DYNAMIC(CSearchBillingTabDlg, CNxDialog)

//this requires CFinancialDlg as a parent
CSearchBillingTabDlg::CSearchBillingTabDlg(CFinancialDlg* pParent)
: CNxDialog(CSearchBillingTabDlg::IDD, pParent)
{
	m_pFinancialDlg = pParent;
	m_nSearchResultIndex = -1;
}

CSearchBillingTabDlg::~CSearchBillingTabDlg()
{
}

void CSearchBillingTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_BILLING_SEARCH_TEXT, m_editSearchText);
	DDX_Control(pDX, IDC_BTN_SEARCH_BILLING, m_btnSearch);
	DDX_Control(pDX, IDC_BTN_FIND_PREV, m_btnFindPrev);
	DDX_Control(pDX, IDC_BTN_FIND_NEXT, m_btnFindNext);
	DDX_Control(pDX, IDC_SEARCH_RESULTS_LABEL, m_nxstaticResultsLabel);
	DDX_Control(pDX, IDC_SEARCH_BILLING_TAB_INFO_ICON, m_icoInfoIcon);
}

BEGIN_MESSAGE_MAP(CSearchBillingTabDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_SEARCH_BILLING, OnBtnSearch)
	ON_EN_CHANGE(IDC_EDIT_BILLING_SEARCH_TEXT, OnEnChangeSearchText)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_FIND_PREV, OnBtnFindPrev)
	ON_BN_CLICKED(IDC_BTN_FIND_NEXT, OnBtnFindNext)
END_MESSAGE_MAP()

BOOL CSearchBillingTabDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnSearch.AutoSet(NXB_INSPECT);

		// (j.jones 2015-03-17 11:39) - PLID 64930 - used to iterate between results
		m_btnFindPrev.AutoSet(NXB_UP);
		m_btnFindNext.AutoSet(NXB_DOWN);
		
		m_editSearchText.SetLimitText(255);

		// (j.jones 2015-03-19 16:31) - PLID 65399 - help icon to show tooltip text
		m_icoInfoIcon.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), CString(""), false, false, false);
		m_icoInfoIcon.SetToolTip("Type currency, dates, numbers, or text to search\n"
			"all data displayed on the Billing tab.");

		ClearSearchInfo(true);

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSearchBillingTabDlg::OnCancel()
{
	try {

		//clear our search info
		ClearSearchInfo(true);

		ShowWindow(SW_HIDE);

	}NxCatchAll(__FUNCTION__);
}

BOOL CSearchBillingTabDlg::PreTranslateMessage(MSG* pMsg)
{
	try {
		switch (pMsg->message) {
		case WM_KEYDOWN:
			if (pMsg->wParam == VK_RETURN) {
				//begin the search now
				OnBtnSearch();
				return TRUE;
			}
			break;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::PreTranslateMessage(pMsg);
}

//clears the results, optionally clears the search term
void CSearchBillingTabDlg::ClearSearchInfo(bool bClearSearchTerm)
{
	try {
		//not every call to this function clears the search box
		if (bClearSearchTerm) {
			m_editSearchText.SetWindowText("");
		}

		CString strSearchTerm;
		strSearchTerm = m_editSearchText.GetText();
		strSearchTerm.Trim();

		//disable the search button if there is no content
		if (strSearchTerm.IsEmpty()) {
			m_btnSearch.EnableWindow(FALSE);
			m_nxstaticResultsLabel.SetWindowText("Enter a Search Term");
		}
		else {
			m_btnSearch.EnableWindow(TRUE);
			m_nxstaticResultsLabel.SetWindowText("");
		}

		// (j.jones 2015-03-17 11:39) - PLID 64930 - disable prev/next and clear our results
		m_btnFindPrev.EnableWindow(FALSE);
		m_btnFindNext.EnableWindow(FALSE);
		m_aryResults.clear();
		m_nSearchResultIndex = -1;

		// (j.jones 2015-03-17 14:11) - PLID 64929 - clear any highlighted cells
		m_pFinancialDlg->RevertSearchResultCellColor();
	
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-19 17:23) - PLID 64931 - checks to see if our search results are still
// potentially valid, clears the results if we know they are not
void CSearchBillingTabDlg::RevalidateSearchResults()
{
	try {

		bool bResultsValid = true;
		if (m_aryResults.size() == 0 || m_nSearchResultIndex < 0 || m_nSearchResultIndex >= (long)m_aryResults.size()) {
			//no results
			bResultsValid = false;
		}
		//re-validate every result
		for (int i = 0; i<(long)m_aryResults.size() && bResultsValid; i++) {		
			if (m_pFinancialDlg->FindSearchResult(m_aryResults[i]) == NULL) {
				bResultsValid = false;
			}
		}

		if (bResultsValid) {
			//if we're still here, our search is still valid
			bool bFound = m_pFinancialDlg->HighlightSearchResult(m_aryResults[m_nSearchResultIndex]);

			// (j.jones 2015-03-20 09:46) - PLID 64931 - in this function it should not be possible
			// for the result to not exist, because we checked for it earlier
			if (!bFound) {
				//why is bResultsValid true?
				ASSERT(FALSE);
				m_nxstaticResultsLabel.SetWindowText(FormatString("Result %li no longer exists.", m_nSearchResultIndex));
			}
		}
		else {
			//The search results are not valid anymore, so
			//clear them out. Don't clear the search term
			//when we obviously wanted to try to retain it.
			ClearSearchInfo(false);
		}

	}NxCatchAll(__FUNCTION__);
}

void CSearchBillingTabDlg::OnBtnSearch()
{
	try {

		CString strSearchTerm;
		strSearchTerm = m_editSearchText.GetText();
		strSearchTerm.Trim();
		if (strSearchTerm.IsEmpty()) {
			AfxMessageBox("You must enter a search term.");
			return;
		}

		if (m_pFinancialDlg == NULL) {
			//should be impossible
			ASSERT(FALSE);
			ThrowNxException("Cannot search: no Billing tab provided!");
		}

		CWaitCursor pWait;

		// (j.jones 2015-03-17 11:39) - PLID 64930 - clear the result text and disable the up / down buttons
		ClearSearchInfo(false);

		m_aryResults = m_pFinancialDlg->SearchBillingTab(strSearchTerm);

		// (j.jones 2015-03-17 11:41) - PLID 64930 - show how many results were returned
		if (m_aryResults.size() == 0) {
			m_nxstaticResultsLabel.SetWindowText("No results found.");
		}
		else {
			//start at the first result			
			m_nSearchResultIndex = 0;
			ReflectCurrentSearchIndex();
		}

	}NxCatchAll(__FUNCTION__);
}

void CSearchBillingTabDlg::OnEnChangeSearchText()
{
	try {
		
		// (j.jones 2015-03-17 11:39) - PLID 64930 - disable prev/next until a search begins
		ClearSearchInfo(false);

	}NxCatchAll(__FUNCTION__);
}

void CSearchBillingTabDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {

		CNxDialog::OnShowWindow(bShow, nStatus);

		//whether we are showing or hiding, ensure the search info is cleared out
		ClearSearchInfo(true);

	}NxCatchAll(__FUNCTION__);
}


// (j.jones 2015-03-17 11:39) - PLID 64930 - used to iterate between results
void CSearchBillingTabDlg::OnBtnFindPrev()
{
	try {

		if (m_aryResults.size() == 0) {
			//should be impossible
			ASSERT(FALSE);
			ReflectCurrentSearchIndex();
			return;
		}

		if (m_nSearchResultIndex < 0) {
			//should be impossible
			ASSERT(FALSE);
			m_nSearchResultIndex = 0;
		}

		//if we're not on the first index, go back by one
		if (m_nSearchResultIndex > 0) {
			m_nSearchResultIndex--;
		}

		//this will enable/disable the prev/next buttons appropriately
		//and jump to the new index result
		ReflectCurrentSearchIndex();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-17 11:39) - PLID 64930 - used to iterate between results
void CSearchBillingTabDlg::OnBtnFindNext()
{
	try {

		if (m_aryResults.size() == 0) {
			//should be impossible
			ASSERT(FALSE);
			ReflectCurrentSearchIndex();
			return;
		}

		if (m_nSearchResultIndex >= (long)m_aryResults.size()) {
			//should be impossible
			ASSERT(FALSE);
			m_nSearchResultIndex = m_aryResults.size() - 1;
		}

		//if we're not on the last index, go forward by one
		if (m_nSearchResultIndex < (long)m_aryResults.size() - 1) {
			m_nSearchResultIndex++;
		}

		//this will enable/disable the prev/next buttons appropriately
		//and jump to the new index result
		ReflectCurrentSearchIndex();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-17 11:48) - PLID 64930 - updates the prev/next arrows and label
// to reflect the current m_nSearchResultIndex, and finds the cell on the billing tab
void CSearchBillingTabDlg::ReflectCurrentSearchIndex()
{
	try {

		//ensure the index is valid
		if (m_nSearchResultIndex < 0 || m_nSearchResultIndex >= (long)m_aryResults.size()) {
			//how did this happen?
			ASSERT(FALSE);

			if (m_aryResults.size() > 0) {
				//start at the beginning
				m_nSearchResultIndex = 0;
			}
			else {
				//there are no results
				ClearSearchInfo(false);
				return;
			}
		}

		//update the label and the arrows		
		if (m_aryResults.size() == 1) {
			m_nxstaticResultsLabel.SetWindowText("Showing 1 result.");
			m_btnFindPrev.EnableWindow(FALSE);
			m_btnFindNext.EnableWindow(FALSE);
			//force the result index to be 0
			m_nSearchResultIndex = 0;
		}
		else {
			m_nxstaticResultsLabel.SetWindowText(FormatString("Showing %li of %li results", m_nSearchResultIndex + 1, m_aryResults.size()));
			if (m_nSearchResultIndex == 0) {
				//we're on the first result
				m_btnFindPrev.EnableWindow(FALSE);
				m_btnFindNext.EnableWindow(TRUE);
			}
			else if (m_nSearchResultIndex == m_aryResults.size() - 1) {
				//we're on the last result
				m_btnFindPrev.EnableWindow(TRUE);
				m_btnFindNext.EnableWindow(FALSE);
			}
			else {
				//we're on a middle result
				m_btnFindPrev.EnableWindow(TRUE);
				m_btnFindNext.EnableWindow(TRUE);
			}
		}

		// (j.jones 2015-03-17 12:42) - PLID 64929 - find this result on the billing tab
		BillingTabSearchResultPtr pResult = m_aryResults[m_nSearchResultIndex];
		bool bFound = m_pFinancialDlg->HighlightSearchResult(pResult);

		// (j.jones 2015-03-20 09:46) - PLID 64931 - it shouldn't really be possible
		// for the result to not exist, unless something u
		if (!bFound) {
			//what caused an update without calling RevalidateSearchResults()?
			ASSERT(FALSE);
			m_nxstaticResultsLabel.SetWindowText(FormatString("Result %li no longer exists.", m_nSearchResultIndex));
		}

	}NxCatchAll(__FUNCTION__);
}