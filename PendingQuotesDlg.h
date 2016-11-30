#if !defined(AFX_PENDINGQUOTESDLG_H__0E364AF6_2827_4B46_8B7A_4A4FA9EFABEB__INCLUDED_)
#define AFX_PENDINGQUOTESDLG_H__0E364AF6_2827_4B46_8B7A_4A4FA9EFABEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PendingQuotesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPendingQuotesDlg dialog

class CPendingQuotesDlg : public CNxDialog
{
// Construction
public:
	CPendingQuotesDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_List;

	void FillList();
	void BillQuote(long QuoteID);
	void MakePrePayment(long QuoteID);
	void InsertSeparatorLine(long QuoteID);
	void InsertBlankRow(long QuoteID);
	void OnCancel();
	void Requery();

	BOOL m_bShowAll;

	BOOL Create();

// Dialog Data
	//{{AFX_DATA(CPendingQuotesDlg)
	enum { IDD = IDD_PENDING_QUOTES_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPendingQuotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CWnd* m_pParent;
	int m_nID;

	//used to determine if the datalist is drawing
	BOOL m_bIsScreenEnabled;

	//reference count for the functions below
	long m_DisableRefCount;

	//will disable/enable drawing of the datalist
	void DisableQuotesScreen();
	void EnableQuotesScreen();

	// Generated message map functions
	//{{AFX_MSG(CPendingQuotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowAllQuotes();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PENDINGQUOTESDLG_H__0E364AF6_2827_4B46_8B7A_4A4FA9EFABEB__INCLUDED_)
