#if !defined(AFX_EMNQUOTESELECTIONDLG_H__F81C3EE9_2239_427B_AFC1_3790B8275DA5__INCLUDED_)
#define AFX_EMNQUOTESELECTIONDLG_H__F81C3EE9_2239_427B_AFC1_3790B8275DA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMNQuoteSelectionDlg.h : header file
//

enum ELinkedQuoteBillType {
	elqbtQuoteAndEMN = 0,
	elqbtQuoteOnly,
	elqbtEMNOnly,
	elqbtEMNOnlyNoLink,
};

/////////////////////////////////////////////////////////////////////////////
// CEMNQuoteSelectionDlg dialog

class CEMNQuoteSelectionDlg : public CNxDialog
{
// Construction
public:
	CEMNQuoteSelectionDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_QuoteCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_EMNCharges;
	NXDATALIST2Lib::_DNxDataListPtr m_QuoteCharges;

	long m_nPatientID;
	long m_nEMNID;
	// (j.jones 2012-01-19 09:24) - PLID 47597 - optional filter for EMR charges,
	// NULL will mean no filter, -1 means to filter by patient resp, else insured party ID
	_variant_t m_varInsuredPartyID;

	long m_nLinkedToQuoteID;
	// (j.jones 2010-04-09 09:19) - PLID 27671 - track if the quote is a package
	BOOL m_bLinkedQuoteIsPackage;
	ELinkedQuoteBillType m_nLinkedQuoteBillType;

// Dialog Data
	//{{AFX_DATA(CEMNQuoteSelectionDlg)
	enum { IDD = IDD_EMN_QUOTE_SELECTION_DLG };
	NxButton	m_btnQuoteAndEMN;
	NxButton	m_btnQuoteOnly;
	NxButton	m_btnChargesOnly;
	NxButton	m_btnNoQuote;
	CNxStatic	m_nxstaticQuoteLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMNQuoteSelectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMNQuoteSelectionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenQuoteCombo(LPDISPATCH lpRow);
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedQuoteCombo(short nFlags);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMNQUOTESELECTIONDLG_H__F81C3EE9_2239_427B_AFC1_3790B8275DA5__INCLUDED_)
