#if !defined(AFX_ADVINSURANCEPAYDESCRIPTIONSETUPDLG_H__A5A84BAB_4473_46AC_B6C4_FAA14EF4AB59__INCLUDED_)
#define AFX_ADVINSURANCEPAYDESCRIPTIONSETUPDLG_H__A5A84BAB_4473_46AC_B6C4_FAA14EF4AB59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvInsurancePayDescriptionSetupDlg.h : header file
//

// (j.jones 2008-07-11 09:29) - PLID 30679 - created

/////////////////////////////////////////////////////////////////////////////
// CAdvInsurancePayDescriptionSetupDlg dialog

class CAdvInsurancePayDescriptionSetupDlg : public CNxDialog
{
// Construction
public:
	CAdvInsurancePayDescriptionSetupDlg(CWnd* pParent);   // standard constructor

	long m_nCurInsCoID;	//takes in the insurance co ID that is being edited on the edit ins. list dialog
	BOOL m_bChangesMadeToCurrentIns; //tracks whether we changed anything on m_nCurInsCoID

// Dialog Data
	//{{AFX_DATA(CAdvInsurancePayDescriptionSetupDlg)
	enum { IDD = IDD_ADV_INSURANCE_PAY_DESCRIPTION_SETUP_DLG };
	NxButton	m_radioPayment;
	NxButton	m_radioAdjustment;
	CNxStatic	m_nxstaticDescriptionLabel;
	CNxStatic	m_nxstaticCategoryLabel;
	CNxEdit	m_nxeditDescription;
	CNxIconButton	m_btnEditCats;
	CNxIconButton	m_btnApplyDesc;
	CNxIconButton	m_btnApplyCategory;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvInsurancePayDescriptionSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList,
									m_SelectedList,
									m_CategoryCombo;

	// Generated message map functions
	//{{AFX_MSG(CAdvInsurancePayDescriptionSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdvInsDescSelectOne();
	afx_msg void OnBtnAdvInsDescSelectAll();
	afx_msg void OnBtnAdvInsDescUnselectOne();
	afx_msg void OnBtnAdvInsDescUnselectAll();
	afx_msg void OnBtnEditPayCats();
	afx_msg void OnBtnApplyDesc();
	afx_msg void OnBtnApplyCategory();
	afx_msg void OnRadioAdvPayDesc();
	afx_msg void OnRadioAdvAdjDesc();
	afx_msg void OnDblClickCellAdvInsDescUnselectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellAdvInsDescSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnAdvInsDescClose();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVINSURANCEPAYDESCRIPTIONSETUPDLG_H__A5A84BAB_4473_46AC_B6C4_FAA14EF4AB59__INCLUDED_)
