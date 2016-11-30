#if !defined(AFX_CATEGORYSELECTDLG_H__881951F7_AE95_45F4_ACBA_67771B7400B3__INCLUDED_)
#define AFX_CATEGORYSELECTDLG_H__881951F7_AE95_45F4_ACBA_67771B7400B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CategorySelectDlg.h : header file
//

#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CCategorySelectDlg dialog

class CCategorySelectDlg : public CNxDialog
{
// Construction
public:
	// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect
	// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType, like "service code" or "inventory item"
	CCategorySelectDlg(CWnd* pParent, bool bAllowMultiSelect, CString strItemType);   // standard constructor

	// (j.jones 2015-03-02 08:39) - PLID 64962 - There can now be multiple categories
	// and an optional default.
	// These members track the default values when opening the dialog.
	std::vector<long> m_aryInitialCategoryIDs;
	long m_nInitialDefaultCategoryID;	
	//These members track the selected values when leaving the dialog.
	std::vector<long> m_arySelectedCategoryIDs;
	long m_nSelectedDefaultCategoryID;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CCategorySelectDlg)
	enum { IDD = IDD_CATEGORY_SELECT_DLG };
	// (j.jones 2015-02-27 16:31) - PLID 64962 - added m_checkAllowMultiSelect
	NxButton	m_checkAllowMultiSelect;
	NxButton	m_btnEnableDragDrop;
	CString m_dlgName;
	CNxIconButton	m_btnAddNew;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCategorySelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2015-02-27 16:22) - PLID 64962 - added m_bAllowMultiSelect
	bool m_bAllowMultiSelect;

	// (j.jones 2015-03-02 15:36) - PLID 64970 - added m_strItemType, like "service code" or "inventory item"
	CString m_strItemType;

	// (z.manning, 03/07/2007) - PLID 24615 - We now use a datalist2 for the category tree.
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCategoryTree;

	CArray<NXDATALIST2Lib::IRowSettings*,NXDATALIST2Lib::IRowSettings*> m_aryDragPlaceholders;

	LPDISPATCH m_lpDraggingRow;

	void AddCategory(LPDISPATCH lpParentRow);
	void DeleteCategory(LPDISPATCH lpRow);
	void PromptRenameCategory(LPDISPATCH lpRow);

	NXDATALIST2Lib::IRowSettingsPtr InsertPlaceholder(NXDATALIST2Lib::IRowSettingsPtr pParentRow);
	void ClearDragPlaceholders(NXDATALIST2Lib::IRowSettingsPtr pRowToPreserve = NULL);
	BOOL IsValidDrag(NXDATALIST2Lib::IRowSettingsPtr pFromRow, NXDATALIST2Lib::IRowSettingsPtr pDestRow, CString &strReasonForFailure);
	void OnTimer(UINT nIDEvent);

	// (j.jones 2015-03-02 10:40) - PLID 64962 - re-fills m_aryCategoryIDs with
	// the currently selected categories, if multiple are selected
	void GatherSelectedCategories();

	// Generated message map functions
	//{{AFX_MSG(CCategorySelectDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddNew();
	afx_msg void OnDblClickCellCategoriesTree(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonDownCategoriesTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDragBeginCategoriesTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellCategoriesTree(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndCategoriesTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2015-03-02 10:38) - PLID 64962 - added multi-select abilities
	afx_msg void OnAllowMultiSelect();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CATEGORYSELECTDLG_H__881951F7_AE95_45F4_ACBA_67771B7400B3__INCLUDED_)
