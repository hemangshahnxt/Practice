#if !defined(AFX_CFINANCIALLINEITEMPOSTINGCONFIGURECOLUMNSDLG_H__97CB20DE_80DE_4823_80B6_DD2F98C5E4F1__INCLUDED_)
#define AFX_CFINANCIALLINEITEMPOSTINGCONFIGURECOLUMNSDLG_H__97CB20DE_80DE_4823_80B6_DD2F98C5E4F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CFinancialLineItemPostingConfigureColumnsDlg.h : header file
//

#include "FinancialLineItemPostingDlg.h"

// (z.manning 2008-06-18 17:27) - PLID 26544 - Created
/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingConfigureColumnsDlg dialog

class CFinancialLineItemPostingConfigureColumnsDlg : public CNxDialog
{
// Construction
public:
	CFinancialLineItemPostingConfigureColumnsDlg(CFinancialLineItemPostingDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFinancialLineItemPostingConfigureColumnsDlg)
	enum { IDD = IDD_FINANCIAL_LINE_ITEM_CONFIGURE_COLUMNS };
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinancialLineItemPostingConfigureColumnsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlColumns;

	CFinancialLineItemPostingDlg *m_pdlgLineItemPosting;

	void UpdateButtons();

	void SwapColumnRows(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);

	enum ColumnListColumns
	{
		clcColumnName = 0,
	};

	// Generated message map functions
	//{{AFX_MSG(CFinancialLineItemPostingConfigureColumnsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLineItemColumnDown();
	afx_msg void OnLineItemColumnUp();
	virtual void OnOK();
	afx_msg void OnSelChangedLineItemColumns(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFINANCIALLINEITEMPOSTINGCONFIGURECOLUMNSDLG_H__97CB20DE_80DE_4823_80B6_DD2F98C5E4F1__INCLUDED_)
