#if !defined(AFX_EMREMCODECATEGORYSETUPDLG_H__7E6D3B91_B546_4373_A484_D1A4A4BF0179__INCLUDED_)
#define AFX_EMREMCODECATEGORYSETUPDLG_H__7E6D3B91_B546_4373_A484_D1A4A4BF0179__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMCodeCategorySetupDlg.h : header file
//

// (j.jones 2007-08-15 10:09) - PLID 27052 - created

/////////////////////////////////////////////////////////////////////////////
// CEMREMCodeCategorySetupDlg dialog

class CEMREMCodeCategorySetupDlg : public CNxDialog
{
// Construction
public:
	CEMREMCodeCategorySetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_EMCategoryList;

	CDWordArray m_dwaryDeletedIDs;	//store IDs for every category we delete

// Dialog Data
	//{{AFX_DATA(CEMREMCodeCategorySetupDlg)
	enum { IDD = IDD_EMR_EM_CODE_CATEGORY_SETUP_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMCodeCategorySetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMREMCodeCategorySetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddEmCategory();
	afx_msg void OnBtnRemoveEmCategory();
	afx_msg void OnEditingFinishingEmCodeCategoryList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmCodeCategoryList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCODECATEGORYSETUPDLG_H__7E6D3B91_B546_4373_A484_D1A4A4BF0179__INCLUDED_)
