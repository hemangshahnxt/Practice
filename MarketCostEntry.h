#if !defined(AFX_MARKETCOSTENTRY_H__C8B630C1_D738_11D3_BA21_00AA0064A698__INCLUDED_)
#define AFX_MARKETCOSTENTRY_H__C8B630C1_D738_11D3_BA21_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketCostEntry.h : header file
//

#include "ReferralSubDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMarketCostEntry dialog

class CMarketCostEntry : public CNxDialog
{
// Construction
public:
	CMarketCostEntry(CWnd* pParent);   // standard constructor
	virtual void OnCancel();

	// (z.manning, 05/12/2008) - PLID 29702 - Converted notes from rich text to edit control
// Dialog Data
	//{{AFX_DATA(CMarketCostEntry)
	enum { IDD = IDD_MARKET_COST_ENTRY };
	CNxIconButton	m_deleteButton;
	CNxIconButton	m_cancelButton;
	CNxIconButton	m_okButton;
	CDateTimePicker	m_dtPaid;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxEdit	m_nxeditCost;
	CNxEdit	m_nxeditReciever;
	CNxEdit	m_nxeditNumber;
	NxButton	m_btnReferralArea;
	CNxEdit	m_nxeditNotes;
	//}}AFX_DATA
	bool m_bIsNew;	//is this a new cost?


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketCostEntry)
	public:
	virtual int DoModal(int setID = 0);
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_id;
	bool m_bActive;
	CString m_strOldAmt, m_strOldDatePaid;

	// (c.haag 2009-02-16 09:15) - PLID 33100 - Added a procedure dropdown.
	// For consistency, the locations dropdown has been converted to a datalist 2.
	NXDATALIST2Lib::_DNxDataListPtr m_dlLocations;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProcedures;
	CArray<long,long> m_anProcedureIDs;
	CArray<long,long> m_anOldProcedureIDs;

	// (c.haag 2009-02-16 09:48) - PLID 33100 - Multi-procedure label
	CNxStatic m_labelMultiProcedure;
	CRect m_rcMultiProcedureLabel;
	CString m_strProcedureList;

	CReferralSubDlg* m_pReferralSubDlg;

protected:
	// (c.haag 2009-02-16 10:05) - PLID 33100 - Handles when a user want to select multiple procedures
	void OnMultiProcedure();
	// (c.haag 2009-02-16 10:04) - PLID 33100 - Updates the procedure region on the dialog
	void DisplayProcedureInfo();
	// (c.haag 2009-02-16 10:05) - PLID 33100 - Returns a procedure name
	CString GetProcedureName(long nID);
	// (c.haag 2009-02-16 11:00) - PLID 33100 - Returns a string of procedure names
	CString GetProcedureNameString(CArray<long,long>& anProcedureIDs);

	// Generated message map functions
	//{{AFX_MSG(CMarketCostEntry)
	virtual void OnOK();
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusCost();
	afx_msg void OnSelectContact();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SelChangingCostLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenCostLocation(LPDISPATCH lpRow);
	void SelChangingComboExpProcedures(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenComboExpProcedures(LPDISPATCH lpRow);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETCOSTENTRY_H__C8B630C1_D738_11D3_BA21_00AA0064A698__INCLUDED_)
