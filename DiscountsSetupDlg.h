#if !defined(AFX_DISCOUNTSSETUPDLG_H__012A37B2_F989_495A_AF12_BEA3AB45F79F__INCLUDED_)
#define AFX_DISCOUNTSSETUPDLG_H__012A37B2_F989_495A_AF12_BEA3AB45F79F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiscountsSetupDlg.h : header file
//

// (j.gruber 2007-03-19 14:25) - PLID 25165 - created discounts tab and put category information on it
/////////////////////////////////////////////////////////////////////////////
// CDiscountsSetupDlg dialog

class CDiscountsSetupDlg : public CNxDialog
{
// Construction
public:
	CDiscountsSetupDlg(CWnd* pParent);   // standard constructor

	// (j.gruber 2007-04-02 09:40) - PLID 25165 - implement an update view 
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButton for OK
// Dialog Data
	//{{AFX_DATA(CDiscountsSetupDlg)
	enum { IDD = IDD_DISCOUNTS_SETUP };
	CNxIconButton	m_inactivate_disc_cat;
	CNxIconButton	m_del_disc_cat;
	CNxIconButton	m_add_disc_cat;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscountsSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountCatList;

	// (j.gruber 2007-03-30 11:29) - PLID 25378 - ability to inactivate
	void ChangeActiveStatus(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bInactive);
	
	// Generated message map functions
	//{{AFX_MSG(CDiscountsSetupDlg)
	afx_msg void OnAddDiscCat();
	afx_msg void OnDeleteDiscCat();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedDiscountCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingDiscountCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSize(UINT nType, int cx, int cy);  // (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
	afx_msg void OnDiscCatInactive();
	afx_msg void OnSelChosenDiscountCategories(LPDISPATCH lpRow);
	afx_msg void OnSelChangedDiscountCategories(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnClose();
	// (j.gruber 2007-05-21 12:49) - PLID 26021 - added since we are a pop up dialog now
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCOUNTSSETUPDLG_H__012A37B2_F989_495A_AF12_BEA3AB45F79F__INCLUDED_)
