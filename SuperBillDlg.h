#if !defined(AFX_SUPERBILLDLG_H__13D402C1_11C4_11D3_B68F_0000C0832801__INCLUDED_)
#define AFX_SUPERBILLDLG_H__13D402C1_11C4_11D3_B68F_0000C0832801__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SuperBillDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSuperBillDlg dialog

class CSuperBillDlg : public CNxDialog
{
// Construction
public:
	CSuperBillDlg();   // constructor

// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CSuperBillDlg)
	enum { IDD = IDD_SUPERBILL };
	NxButton	m_btnRememberSelected;
	NxButton	m_btnShowInactiveResources;
	NxButton	m_btnMergeToPrinter;
	NxButton	m_btnLocFilter;
	NxButton	m_btnAdvancedTemplateConfig;
	CNxIconButton	m_cancelBtn;
	CNxIconButton	m_resAddBtn;
	CNxIconButton	m_resRemoveAllBtn;
	CNxIconButton	m_resRemoveBtn;
	CNxIconButton	m_resAddAllBtn;
	CNxIconButton	m_aptAddBtn;
	CNxIconButton	m_aptRemoveBtn;
	CNxIconButton	m_aptRemoveAllBtn;
	CNxIconButton	m_aptAddAllBtn;
	CNxIconButton	m_printBtn;
	CDateTimePicker	m_from;
	CDateTimePicker	m_to;
	CNxIconButton	m_btnDefaultTemplate;
	CNxIconButton	m_btnEditTemplate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuperBillDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nDefTemplateRowIndex;

	NxButton	m_dateFilter,
				m_nameSort,
				m_dateSort,
				// (z.manning 2009-08-03 12:52) - PLID 19645 - Added ability to sort by resource
				m_btnResourceDateSort;

	NXDATALISTLib::_DNxDataListPtr m_resUnselected,
					m_resSelected,
					m_aptUnselected,
					m_aptSelected,
					m_template,
					m_pLocation;

	CString GetPurposeFilter();
	CString GetResourceFilter();
	void SaveSelectedItems();
	void FillSuperBillList(CString strPath);
	void EnsureControls();

	afx_msg void OnSuperbills(bool printOnly = false);

	// (d.thompson 2009-09-30) - PLID 9975 - Helper functions for generating the proper merge data
	bool GenerateBaseWhereClauseFromInterface(CString &strBaseWhere);
	bool SetupMergeEngine(CMergeEngine *pmi);
	void FreeMemoryInPointerArrayMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources);
	void FillAppointmentMapsFromBaseQuery(CString strBaseWhere, CMap<long, long, bool, bool> *pmapAllAppts, 
													 CMap<long, long, long, long> *pmapApptsToTypes, 
													 CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources);
	void FillTypeGroupMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmapTypeGroups,
										CMap<long, long, bool, bool> *pmapAllAppts, 
										CMap<long, long, long, long> *pmapApptsToTypes, 
										CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources);
	void FillResourceGroupMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmapResourceGroups, 
										CMap<long, long, bool, bool> *pmapAllAppts, 
										CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources);
	bool GenerateWhereClauseAndPathFromIDArray(const IN CDWordArray *pary, OUT CString &strWhereClause, OUT CString &strTemplatePath,
														  const IN CString strPathTable, const IN long nGroupID, const IN CString strPromptText);


	// Generated message map functions
	// (a.walling 2008-05-13 14:57) - PLID 27591 - Use Notify handlers for DateTimePicker
	//{{AFX_MSG(CSuperBillDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAptAdd();
	afx_msg void OnAptAddAll();
	afx_msg void OnAptRemove();
	afx_msg void OnAptRemoveAll();
	afx_msg void OnResAdd();
	afx_msg void OnResAddAll();
	afx_msg void OnResRemove();
	afx_msg void OnResRemoveAll();
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPrintSuperbills();
	afx_msg void OnDblClickCellResSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellResUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellTypeSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellTypeUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChosenTemplate(long nRow);
	afx_msg void OnDeftemplateBtn();
	afx_msg void OnEditTemplateBtn();
	afx_msg void OnRememberSelected();
	afx_msg void OnShowInactiveResources();
	afx_msg void OnMergePrinter();
	afx_msg void OnSelChangingLocationFilter(long FAR* nNewSel);
	afx_msg void OnCheckLocationFilter();
	afx_msg void OnUseDateRange();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedUseAdvancedConfig();
	afx_msg void OnBnClickedTemplateConfig();

	// (a.walling 2010-09-15 09:20) - PLID 7018 - bIsFresh determines whether this is a new superbill or a re-printed copy
	void PrintSuperbills(CMergeEngine& mi, const CString& strBaseWhere, bool bIsFresh);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUPERBILLDLG_H__13D402C1_11C4_11D3_B68F_0000C0832801__INCLUDED_)
