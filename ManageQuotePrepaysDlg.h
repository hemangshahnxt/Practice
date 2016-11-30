#if !defined(AFX_MANAGEQUOTEPREPAYSDLG_H__B55CD42D_54D2_4AE8_BF20_7ABB68F1AD3B__INCLUDED_)
#define AFX_MANAGEQUOTEPREPAYSDLG_H__B55CD42D_54D2_4AE8_BF20_7ABB68F1AD3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ManageQuotePrepaysDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CManageQuotePrepaysDlg dialog

class CManageQuotePrepaysDlg : public CNxDialog
{
// Construction
public:
	CManageQuotePrepaysDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_PrePayList, m_QuoteList;

	long m_PatientID;
	long m_DefaultQuoteID;

	void UpdateButtons();

// Dialog Data
	//{{AFX_DATA(CManageQuotePrepaysDlg)
	enum { IDD = IDD_MANAGE_QUOTE_PREPAYS_DLG };
	NxButton	m_btnShowUnlinked;
	CNxIconButton	m_btnNewPrePay;
	CNxIconButton	m_btnPrePayLink;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CManageQuotePrepaysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	void OnEditPrepay();

	// Generated message map functions
	//{{AFX_MSG(CManageQuotePrepaysDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenQuoteList(long nRow);
	afx_msg void OnBtnNewPrepay();
	afx_msg void OnPrepayLinkBtn();
	afx_msg void OnCheckShowUnlinkedPrepays();
	afx_msg void OnRButtonDownLinkedPrepayList(long nRow, short nCol, long x, long y, long nFlags);
	virtual void OnOK();
	afx_msg void OnRequeryFinishedLinkedPrepayList(short nFlags);
	afx_msg void OnSelChangedLinkedPrepayList(long nNewSel);
	afx_msg void OnDblClickCellLinkedPrepayList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANAGEQUOTEPREPAYSDLG_H__B55CD42D_54D2_4AE8_BF20_7ABB68F1AD3B__INCLUDED_)
