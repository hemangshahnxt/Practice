#if !defined(AFX_ADVCOMMISSIONSETUPDLG_H__F53B7734_1703_45D0_9659_E5D3B8E49926__INCLUDED_)
#define AFX_ADVCOMMISSIONSETUPDLG_H__F53B7734_1703_45D0_9659_E5D3B8E49926__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvCommissionSetupDlg.h : header file
//

// (a.wetta 2007-03-28 16:35) - PLID 25326 - Dialog created

enum AdvCommissionListFields {
	aclfID = 0,
	aclfType,
	aclfCategory,
	aclfCode,
	aclfName,
};

struct CommissionInfo {
	double dblPercent;
	long nRuleID;
	CString strRuleName;
	COleCurrency cyAmount; // (b.eyers 2015-05-13) - PLID 65981
};

struct IDNameInfo {
	long nID;
	CString strName;
};

/////////////////////////////////////////////////////////////////////////////
// CAdvCommissionSetupDlg dialog

class CAdvCommissionSetupDlg : public CNxDialog
{
// Construction
public:
	CAdvCommissionSetupDlg(CWnd* pParent);   // standard constructor

	DWORD m_backgroundColor;

	BOOL m_bPercent; // (b.eyers 2015-05-13) - PLID 65981

// Dialog Data
	//{{AFX_DATA(CAdvCommissionSetupDlg)
	enum { IDD = IDD_ADV_COMMISSION_SETUP_DLG };
	NxButton	m_btnShowOutdatedRules;
	NxButton	m_btnShowInactiveProv;
	CNxIconButton	m_btnCommRuleSetup;
	CNxIconButton	m_btnClearSelectedRules;
	CNxIconButton	m_btnClearSelected;
	CNxIconButton	m_btnRemoveSelectedRule;
	CNxIconButton	m_btnApplySelectedRule;
	CNxIconButton	m_btnApplySelected;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnRemoveOne;
	CNxIconButton	m_btnRemoveAll;
	CNxIconButton	m_btnRemoveProvider;
	CNxIconButton	m_btnAddProvider;
	CNxColor	m_bkgColor2;
	CNxColor	m_bkgColor3;
	CNxColor	m_bkgColor4;
	CNxEdit	m_nxeditApplyPercentage;
	CNxLabel m_nxlabelApply; // (b.eyers 2015-05-13) - PLID 65981
	CNxEdit m_nxeditApplyAmount; // (b.eyers 2015-05-13) - PLID 65981
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvCommissionSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pChangesList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelProviders;
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailProviders;
	NXDATALIST2Lib::_DNxDataListPtr m_pAddRuleCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pRemoveRuleCombo;

	BOOL GetSelProvidersAndServiceItems(CArray<IDNameInfo,IDNameInfo> &aryProviders, CArray<IDNameInfo,IDNameInfo> &aryServiceItems);
	CString GetIDStringFromArray(CArray<IDNameInfo,IDNameInfo> &aryIDs);
	// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Removed the MoveSelectedRows() method, and for efficiency 
	// used the TakeCurrentRowAddSorted() method directly where needed instead.

	double m_dblPercent;

	COleCurrency m_cyAmount; // (b.eyers 2015-05-13) - PLID 65981

	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (b.eyers 2015-05-13) - PLID 65981

	// Generated message map functions
	//{{AFX_MSG(CAdvCommissionSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAddProv();
	afx_msg void OnRemoveProv();
	afx_msg void OnSelectAllCitems();
	afx_msg void OnSelectOneCitem();
	afx_msg void OnRemoveOneCitem();
	afx_msg void OnRemoveAllCitems();
	afx_msg void OnCommRuleSetup();
	afx_msg void OnDblClickCellAvailProvList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellSelProvList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellAvailList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellChangesList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRemoveSelectedRule();
	afx_msg void OnApplySelected();
	afx_msg void OnApplySelectedRule();
	afx_msg void OnClearSelectedRules();
	afx_msg void OnClearSelected();
	afx_msg void OnShowInactiveProv();
	afx_msg void OnShowOutDatedRules();
	afx_msg void OnKillfocusApplyPercentage();
	afx_msg void OnKillfocusApplyAmount(); // (b.eyers 2015-05-13) - PLID 65981
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVCOMMISSIONSETUPDLG_H__F53B7734_1703_45D0_9659_E5D3B8E49926__INCLUDED_)
