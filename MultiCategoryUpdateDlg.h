#if !defined(AFX_MULTICATEGORYUPDATEDLG_H__CB737C84_2EB0_4D31_864F_2373F7871424__INCLUDED_)
#define AFX_MULTICATEGORYUPDATEDLG_H__CB737C84_2EB0_4D31_864F_2373F7871424__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiCategoryUpdateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiCategoryUpdateDlg dialog

class CMultiCategoryUpdateDlg : public CNxDialog
{
// Construction
public:
	CMultiCategoryUpdateDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons for OK and Cancel
// Dialog Data
	//{{AFX_DATA(CMultiCategoryUpdateDlg)
	enum { IDD = IDD_MULTI_CATEGORY_UPDATE_DLG };
	CNxIconButton	m_btnRemoveAll;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2015-03-10 09:37) - PLID 64971 - added ability to select multiple categories
	CNxIconButton	m_btnSelectCategory;
	CNxIconButton	m_btnRemoveCategory;
	CNxEdit			m_editCategory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiCategoryUpdateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pSelected;
	NXDATALISTLib::_DNxDataListPtr m_pUnselected;

	// (j.jones 2015-03-10 09:37) - PLID 64971 - added ability to select multiple categories
	std::vector<long> m_aryCategoryIDs;
	long m_nDefaultCategoryID;

	// Generated message map functions
	//{{AFX_MSG(CMultiCategoryUpdateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnDblClickCellSelectedList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedList(long nRowIndex, short nColIndex);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRemoveAll();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2015-03-10 09:37) - PLID 64971 - added ability to select multiple categories
	afx_msg void OnBtnSelectCategory();
	afx_msg void OnBtnRemoveCategory();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTICATEGORYUPDATEDLG_H__CB737C84_2EB0_4D31_864F_2373F7871424__INCLUDED_)
