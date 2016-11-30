#if !defined(AFX_RETRIEVEPASTDEPOSITSDLG_H__68F21089_F7ED_4D76_B1E6_E6BB07819CDD__INCLUDED_)
#define AFX_RETRIEVEPASTDEPOSITSDLG_H__68F21089_F7ED_4D76_B1E6_E6BB07819CDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RetrievePastDepositsDlg.h : header file
//

#include "PracBanking.h"

/////////////////////////////////////////////////////////////////////////////
// CRetrievePastDepositsDlg dialog

class CRetrievePastDepositsDlg : public CNxDialog
{
// Construction
public:
	CRetrievePastDepositsDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_DepositList;

	// (j.jones 2007-03-13 09:50) - PLID 25118 - tablecheckers now replace
	// the need to point back to the parent dialog
	//PracBanking *m_pBankingTabPtr;

	// (j.jones 2008-05-08 10:59) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CRetrievePastDepositsDlg)
	enum { IDD = IDD_RETRIEVE_PAST_DEPOSITS_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRestorePayments;
	CNxIconButton	m_btnRestoreDeposit;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRetrievePastDepositsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRetrievePastDepositsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnRestorePayments();
	afx_msg void OnBtnRestoreDeposit();
	afx_msg void OnDblClickCellPastBatchList(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownPastBatchList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnChangeRestorePaymentFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpRestorePaymentFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeRestorePaymentToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpRestorePaymentToDate(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RETRIEVEPASTDEPOSITSDLG_H__68F21089_F7ED_4D76_B1E6_E6BB07819CDD__INCLUDED_)
