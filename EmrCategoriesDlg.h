#if !defined(AFX_EMRCATEGORIESDLG_H__96C12962_F96A_470F_8058_1ADAD6CBAD11__INCLUDED_)
#define AFX_EMRCATEGORIESDLG_H__96C12962_F96A_470F_8058_1ADAD6CBAD11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrCategoriesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrCategoriesDlg dialog

class CEmrCategoriesDlg : public CNxDialog
{
// Construction
public:
	CEmrCategoriesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrCategoriesDlg)
	enum { IDD = IDD_EMR_CATEGORIES_DLG };
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnAddEMRCategory;
	CNxIconButton	m_btnDeleteEMRCategory;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrCategoriesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pCategories;

	BOOL m_bIsLevel1EMR;

	// Generated message map functions
	//{{AFX_MSG(CEmrCategoriesDlg)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddEmrCategory();
	virtual void OnOK();
	afx_msg void OnDeleteEmrCategory();
	afx_msg void OnEditingFinishingEmrCategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmrCategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEmrCategoryUp();
	afx_msg void OnEmrCategoryDown();
	afx_msg void OnSelChangedEmrCategoryList(long nNewSel);
	afx_msg void OnRequeryFinishedEmrCategoryList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRCATEGORIESDLG_H__96C12962_F96A_470F_8058_1ADAD6CBAD11__INCLUDED_)
