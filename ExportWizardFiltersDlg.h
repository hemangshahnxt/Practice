#if !defined(AFX_EXPORTWIZARDFILTERSDLG_H__0E1E5FF7_94D9_443E_8E19_C40ADC4A7475__INCLUDED_)
#define AFX_EXPORTWIZARDFILTERSDLG_H__0E1E5FF7_94D9_443E_8E19_C40ADC4A7475__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardFiltersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFiltersDlg dialog

class CExportWizardFiltersDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardFiltersDlg)

// Construction
public:
	CExportWizardFiltersDlg();
	~CExportWizardFiltersDlg();

	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate appointment exports based on created date or modified date
	void ChangeDateHyperlink();
	void DrawDateHyperlink(CDC *pdc);
	void DoClickHyperlink(UINT nFlags, CPoint point);
	void HideHyperlink();
	CRect m_rcDateHyperlink;

// Dialog Data
	//{{AFX_DATA(CExportWizardFiltersDlg)
	enum { IDD = IDD_EXPORT_WIZARD_FILTERS };
	CNxIconButton	m_nxbUp;
	CNxIconButton	m_nxbDown;
	CNxIconButton	m_nxbRemove;
	CNxIconButton	m_nxbAdd;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CString	m_strDateHyperlink;
	CNxEdit	m_nxeditFromPlusDays;
	CNxEdit	m_nxeditToPlusDays;
	CNxEdit	m_nxeditAdvancedSortField;
	CNxStatic	m_nxstaticDateHyperlink;
	CNxStatic	m_nxstaticAllNewRecords2;
	CNxStatic	m_nxstaticFromDaysLabel;
	CNxStatic	m_nxstaticToPlusLabel;
	CNxStatic	m_nxstaticToDaysLabel;
	CNxStatic	m_nxstaticFromPlusLabel;
	CNxStatic	m_nxstaticAdvancedSortLabel;
	NxButton	m_btnFilterGroupbox;
	NxButton	m_btnOrderGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardFiltersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pDateFilters, m_pLwFilters, m_pStandardDatesFrom, m_pStandardDatesTo, m_pSortFieldsAvail, m_pSortFieldsSelect;

	void EnableWindows();

	long m_nBasedOn;

	// Generated message map functions
	//{{AFX_MSG(CExportWizardFiltersDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAllNewRecords();
	afx_msg void OnManualSelect();
	afx_msg void OnSelChosenRecordLwFilterOptions(long nRow);
	afx_msg void OnSelChosenRecordDateFilterOptions(long nRow);
	afx_msg void OnChangeRecordDateFilterFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeRecordDateFilterTo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnSetActive();
	afx_msg void OnSelChosenStandardDatesFrom(long nRow);
	afx_msg void OnSelChosenStandardDatesTo(long nRow);
	afx_msg void OnChangeFromPlusDays();
	afx_msg void OnChangeToPlusDays();
	afx_msg void OnManualSort();
	afx_msg void OnAddSortField();
	afx_msg void OnRemoveSortField();
	afx_msg void OnSortFieldUp();
	afx_msg void OnSortFieldDown();
	afx_msg void OnSelChangedSortFieldsAvail(long nNewSel);
	afx_msg void OnSelChangedSortFieldsSelect(long nNewSel);
	afx_msg void OnChangeAdvancedSortField();
	afx_msg void OnDblClickCellSortFieldsAvail(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSortFieldsSelect(long nRowIndex, short nColIndex);
	virtual BOOL OnKillActive();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// (z.manning 2009-12-14 12:31) - PLID 36576
	afx_msg void OnBnClickedDateFilterCheck();
	afx_msg void OnBnClickedLwFilterCheck();
	afx_msg void OnBnClickedExportDateLwFilter();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDFILTERSDLG_H__0E1E5FF7_94D9_443E_8E19_C40ADC4A7475__INCLUDED_)
