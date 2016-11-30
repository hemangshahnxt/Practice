#if !defined(AFX_EDITCREDITCARDSDLG_H__4718515D_2796_42FD_83ED_CCDA0912C219__INCLUDED_)
#define AFX_EDITCREDITCARDSDLG_H__4718515D_2796_42FD_83ED_CCDA0912C219__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditCreditCardsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditCreditCardsDlg dialog

class CEditCreditCardsDlg : public CNxDialog
{
// Construction
public:
	CEditCreditCardsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditCreditCardsDlg)
	enum { IDD = IDD_EDIT_CREDITCARDS_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditCreditCardsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pChargeCards;

	CArray<long, long> m_aryChangedCards;
	CArray<long, long> m_aryDeletedCards;


	void Save();
	void GenerateDeletedCardSql(CString& strSqlBatch);
	void GenerateNewCardSql(CString& strSqlBatch);
	void GenerateUpdateSql(CString& strSqlBatch);

	// Generated message map functions
	//{{AFX_MSG(CEditCreditCardsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishingCreditCardSetup(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddCard();
	afx_msg void OnRemoveCard();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCREDITCARDSDLG_H__4718515D_2796_42FD_83ED_CCDA0912C219__INCLUDED_)
