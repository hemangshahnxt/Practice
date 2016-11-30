#if !defined(AFX_EXPORTWIZARDNAMEDLG_H__A1FCA97D_5C27_488E_A9EF_ECAB9309B4EB__INCLUDED_)
#define AFX_EXPORTWIZARDNAMEDLG_H__A1FCA97D_5C27_488E_A9EF_ECAB9309B4EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardNameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardNameDlg dialog

class CExportWizardNameDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardNameDlg)

// Construction
public:
	CExportWizardNameDlg();
	~CExportWizardNameDlg();

// Dialog Data
	//{{AFX_DATA(CExportWizardNameDlg)
	enum { IDD = IDD_EXPORT_WIZARD_NAME };
	CNxIconButton	m_nxbAdd;
	CNxIconButton	m_nxbRemove;
	CNxEdit	m_editName;
	CNxStatic	m_nxstaticEmnLabel;
	CNxLabel m_nxlabelCategories;
	//}}AFX_DATA

	CString m_strInitialCategoryText;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void HandleRadioButton();

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pAvail, m_pSelect;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCategoryFilter; // (z.manning 2009-12-10 15:10) - PLID 36519
	enum ECategoryFilterColumns {
		cfcID = 0,
		cfcName,
	};

	void HandleSel();
	void UpdateWizardButtons();

	// (z.manning 2009-12-10 16:09) - PLID 36519
	void OpenCategoryMultiSelectDialog();
	void UpdateCategoryFilterDisplay();

	// Generated message map functions
	//{{AFX_MSG(CExportWizardNameDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnExportPatients();
	afx_msg void OnExportCharges();
	afx_msg void OnExportAppointments();
	afx_msg void OnExportPayments();
	afx_msg void OnChangeExportName();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	afx_msg void OnExportEmns();
	afx_msg void OnAddEmnTemplate();
	afx_msg void OnSelChangedEmnTemplatesAvail(long nNewSel);
	afx_msg void OnDblClickCellEmnTemplatesAvail(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedEmnTemplatesSelect(long nNewSel);
	afx_msg void OnDblClickCellEmnTemplatesSelect(long nRowIndex, short nColIndex);
	afx_msg void OnRemoveEmnTemplate();
	afx_msg void OnAllowOtherTemplates();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedExportHistory();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void SelChosenExportCategoryFilter(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDNAMEDLG_H__A1FCA97D_5C27_488E_A9EF_ECAB9309B4EB__INCLUDED_)
