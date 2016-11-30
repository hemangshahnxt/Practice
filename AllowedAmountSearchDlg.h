#if !defined(AFX_ALLOWEDAMOUNTSEARCHDLG_H__93258B6E_C623_4D52_AFA0_E3297EDA6C8D__INCLUDED_)
#define AFX_ALLOWEDAMOUNTSEARCHDLG_H__93258B6E_C623_4D52_AFA0_E3297EDA6C8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AllowedAmountSearchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAllowedAmountSearchDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

class CAllowedAmountSearchDlg : public CNxDialog
{
// Construction
public:
	CAllowedAmountSearchDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_pDateFilterCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceCoCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsurancePlanCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceCodeCombo;
	// (j.jones 2016-01-18 13:16) - PLID 67501 - added inventory item dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_pInventoryItemCombo;

	NXDATALIST2Lib::_DNxDataListPtr m_pResultList;

	// (j.jones 2008-05-07 10:43) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CAllowedAmountSearchDlg)
	enum { IDD = IDD_ALLOWED_AMOUNT_SEARCH_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnSearch;
	CNxIconButton	m_btnPreview;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxEdit	m_nxeditEditAllowableAmount;
	// (j.jones 2016-01-18 13:24) - PLID 67501 - added service/product radio buttons
	NxButton	m_radioServiceCodes;
	NxButton	m_radioInvItems;
	CNxStatic	m_staticServiceCodeLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAllowedAmountSearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	BOOL ValidateAndBuildWhereClause(CString &strWhere, CString &strHaving, COleCurrency &cyAllowable);

	// Generated message map functions
	//{{AFX_MSG(CAllowedAmountSearchDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSearchAllowables();
	afx_msg void OnSelChosenAllowableDateFilterCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenAllowableInsCoCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedAllowedAmountResultList(short nFlags);
	afx_msg void OnBtnPreviewAllowables();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2016-01-18 13:24) - PLID 67501 - added service/product radio buttons
	afx_msg void OnRadioServiceCodes();
	afx_msg void OnRadioInvItems();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALLOWEDAMOUNTSEARCHDLG_H__93258B6E_C623_4D52_AFA0_E3297EDA6C8D__INCLUDED_)
