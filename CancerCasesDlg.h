#pragma once

//TES 4/23/2014 - PLID 61854 - Created
// CCancerCasesDlg dialog

class CCancerCasesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCancerCasesDlg)

public:
	CCancerCasesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCancerCasesDlg();

// Dialog Data
	enum { IDD = IDD_CANCER_CASES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_pEmnList;
	//TES 4/25/2014 - PLID 61917 - Filter controls
	NXDATALIST2Lib::_DNxDataListPtr m_pProvCombo, m_pSecProvCombo, m_pLocCombo, m_pStatusCombo;
	CDateTimePicker m_dtFrom, m_dtTo;
	CNxIconButton m_nxbExportSelected, m_nxbDisplayResults, m_nxbResetFilters;
	CNxIconButton m_nxbViewAllErrors; //TES 5/19/2014 - PLID 62196

	void RefreshEmnList();
	//TES 4/25/2014 - PLID 61859 - Functions for the capability to remember column widths
	void SaveColumnWidths();
	void RestoreColumnWidths();

	//TES 5/2/2014 - PLID 61855 - We need to keep track of whether we're hiding exported rows, so we can update the list properly after exporting
	bool m_bHidingExported;

	//TES 5/7/2015 - PLID 65968 - Split out the Export code into a separate function
	void ExportCancerCases(long nOverrideEmnID = -1);

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingProviderFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingSecProviderFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingLocationFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingStatusFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenProviderFilterCombo(LPDISPATCH lpRow);
	void SelChosenSecProviderFilterCombo(LPDISPATCH lpRow);
	void SelChosenLocationFilterCombo(LPDISPATCH lpRow);
	void SelChosenStatusFilterCombo(LPDISPATCH lpRow);
	afx_msg void OnDtnDatetimechangeFilterDateFrom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeFilterDateTo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDisplayResults();
	afx_msg void OnBnClickedResetFilters();
	void ShowContextMenuCancerCasesList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue);
	afx_msg void OnBnClickedRememberColumnWidthsCancercases();
	void ColumnSizingFinishedCancerCasesList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnBnClickedExportSelectedCancercases();
	void LeftClickCancerCasesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedViewAllErrors();
};
