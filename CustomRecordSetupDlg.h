#if !defined(AFX_CUSTOMRECORDSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_)
#define AFX_CUSTOMRECORDSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSetupDlg dialog

class CCustomRecordSetupDlg : public CNxDialog
{
// Construction
public:
	CCustomRecordSetupDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	CTableChecker m_procedureNameChecker, m_ICD9Checker;

	// (z.manning, 04/25/2008) - PLID 29566 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CEMRSetupDlg)
	enum { IDD = IDD_CUSTOM_RECORD_SETUP };
	NxButton	m_btnList;
	NxButton	m_btnText;
	CNxEdit	m_nxeditItemName;
	CNxEdit	m_nxeditSentence;
	CNxEdit	m_nxeditDataValue;
	CNxStatic	m_nxstaticDataTypeLabel;
	CNxStatic	m_nxstaticDataLabel;
	CNxStatic	m_nxstaticDataLabel2;
	CNxIconButton	m_btnAddInfoItem;
	CNxIconButton	m_btnDeleteInfoItem;
	CNxIconButton	m_btnAddDataItem;
	CNxIconButton	m_btnDeleteDataItem;
	CNxIconButton	m_btnItemAction;
	CNxIconButton	m_btnDataAction;
	CNxIconButton	m_btnSetupHeader;
	CNxIconButton	m_btnOfficeVisit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordSetupDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pItemList, m_pProcedureDropdown, m_pProcedureList, m_pDiagDropdown, m_pDiagList,
		m_pCategoryList, m_pDataList;

	void Load(long nItemID);

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordSetupDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioTextSelect();
	afx_msg void OnRadioListSelect();
	afx_msg void OnSelChosenEMRInfoList(long nRow);
	afx_msg void OnSelChangingEMRInfoList(long FAR* nNewSel);
	afx_msg void OnSelChosenEMRDataList(long nRow);
	afx_msg void OnSelChangedEMRDataList(long nNewSel);
	afx_msg void OnAddInfoItem();
	afx_msg void OnDeleteInfoItem();
	afx_msg void OnSelChosenProcedure(long nRow);
	afx_msg void OnRButtonUpEMRProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDeleteProcedure();
	afx_msg void OnEditingFinishedEMRProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenDiagDropdown(long nRow);
	afx_msg void OnRButtonUpDiagList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDeleteDiag();
	afx_msg void OnEditingFinishedDiagList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnKillfocusItemName();
	afx_msg void OnSelChosenEMRCategories(long nRow);
	afx_msg void OnSelChangingEMRCategories(long FAR* nNewSel);
	afx_msg void OnKillfocusSentence();
	afx_msg void OnAddDataItem();
	afx_msg void OnDeleteDataItem();
	afx_msg void OnEditingFinishedEMRDataList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnKillfocusDataValue();
	afx_msg void OnItemAction();
	afx_msg void OnDataAction();
	afx_msg void OnEditEMRCategories();
	afx_msg void OnSetupHeader();
	afx_msg void OnOfficeVisit();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_)
