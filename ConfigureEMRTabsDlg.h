#if !defined(AFX_CONFIGUREEMRTABSDLG_H__CFB41FEB_F132_4B4A_84E9_E09BC557D974__INCLUDED_)
#define AFX_CONFIGUREEMRTABSDLG_H__CFB41FEB_F132_4B4A_84E9_E09BC557D974__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureEMRTabsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigureEMRTabsDlg dialog

class CConfigureEMRTabsDlg : public CNxDialog
{
// Construction
public:
	CConfigureEMRTabsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureEMRTabsDlg)
	enum { IDD = IDD_CONFIGURE_EMR_TABS };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnMoveEMNCatPriorityUp;
	CNxIconButton	m_btnMoveEMNCatPriorityDown;
	CNxIconButton	m_btnDeleteEMNCategory;
	CNxIconButton	m_btnAddEMNCategory;
	CNxIconButton	m_btnMoveEMNChartPriorityUp;
	CNxIconButton	m_btnMoveEMNChartPriorityDown;
	CNxIconButton	m_btnDeleteEMNChart;
	CNxIconButton	m_btnAddEMNChart;
	CNxStatic	m_nxstaticHistoryCatLabel;
	//}}AFX_DATA

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureEMRTabsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bChartOrderChanged; // (a.walling 2007-07-03 13:22) - PLID 26545
	BOOL m_bCatOrderChanged;
	BOOL m_bLinksChanged;

	NXDATALIST2Lib::_DNxDataListPtr m_EMNTabCategoriesList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlEmnCharts; // (z.manning, 04/03/2007) - PLID 25485
	NXDATALIST2Lib::_DNxDataListPtr m_pdlHistoryCategories;

	CNxColor m_nxcolorBackground; // (z.manning 2011-05-18 10:04) - PLID 43756

	void SwapEMNCatPriorities(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);
	void SwapEmnChartPriorities(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2); // (z.manning, 04/04/2007) - PLID 25485
	void UpdatePriorityButtons();

	// (z.manning, 04/04/2007) - PLID 25485 - Categories can be selected per chart (same as apt types and purposes)
	// so this function updates the selected categories for the currently selected chart.
	void UpdateCategoryListCheckboxes();

	// (z.manning 2008-06-30 14:47) - PLID 25574
	void UpdateHistoryCategories();

	// Generated message map functions
	//{{AFX_MSG(CConfigureEMRTabsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddEmnCategory();
	afx_msg void OnDeleteEmnCategory();
	afx_msg void OnMoveEmnCatPriorityDownBtn();
	afx_msg void OnMoveEmnCatPriorityUpBtn();
	afx_msg void OnAddEmnChart();
	afx_msg void OnDeleteEmnChart();
	afx_msg void OnMoveEmnChartPriorityDownBtn();
	afx_msg void OnMoveEmnChartPriorityUpBtn();
	afx_msg void OnSelChangedEMNCategoryList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishedEMNCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingEmnTabCatList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedEMNChartList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishedEMNCharts(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingEmnTabChartList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRButtonUpEmnChartTypes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedEmnChartTypes(short nFlags);
	afx_msg void OnLButtonUpEmnChartTypes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditingFinishedHistoryCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREEMRTABSDLG_H__CFB41FEB_F132_4B4A_84E9_E09BC557D974__INCLUDED_)
