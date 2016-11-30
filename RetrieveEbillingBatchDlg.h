#if !defined(AFX_RETRIEVEEBILLINGBATCHDLG_H__429A9ED1_BE14_44C7_A4B5_C1142728E43A__INCLUDED_)
#define AFX_RETRIEVEEBILLINGBATCHDLG_H__429A9ED1_BE14_44C7_A4B5_C1142728E43A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RetrieveEbillingBatchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRetrieveEbillingBatchDlg dialog

#include "EbillingFormDlg.h"

class CRetrieveEbillingBatchDlg : public CNxDialog
{
// Construction
public:
	CRetrieveEbillingBatchDlg(CWnd* pParent);   // standard constructor

	CEbillingFormDlg *m_pEbillingTabPtr;

	NXDATALISTLib::_DNxDataListPtr m_BatchList;

	// (j.jones 2008-05-08 10:59) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CRetrieveEbillingBatchDlg)
	enum { IDD = IDD_RETRIEVE_EBILLING_BATCH_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRestoreClaims;
	CNxIconButton	m_btnRestoreBatch;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRetrieveEbillingBatchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRetrieveEbillingBatchDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonDownPastBatchList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnRestoreClaims();
	afx_msg void OnBtnRestoreBatch();
	afx_msg void OnChangeRestoreClaimFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpRestoreClaimFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeRestoreClaimToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpRestoreClaimToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequeryFinishedPastBatchList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RETRIEVEEBILLINGBATCHDLG_H__429A9ED1_BE14_44C7_A4B5_C1142728E43A__INCLUDED_)
