#if !defined(AFX_INVTRANSFERDLG_H__F8BC49A5_7F68_429B_940D_780876FC1C6C__INCLUDED_)
#define AFX_INVTRANSFERDLG_H__F8BC49A5_7F68_429B_940D_780876FC1C6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvTransferDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvTransferDlg dialog

class CInvTransferDlg : public CNxDialog
{
// Construction
public:
	CInvTransferDlg(CWnd* pParent);   // standard constructor

	int DoModal(long nProductID, long nSourceLocationID, long nDestLocationID = -1);

	BOOL m_bUseUU;

	long m_UUConversion;

	long m_nProductID;
	long m_nSourceLocationID;
	long m_nDestLocationID;
	
	BOOL TransferProductItems(double dblQty, long nSourceLocationID, long nDestLocationID);

// Dialog Data
	//{{AFX_DATA(CInvTransferDlg)
	enum { IDD = IDD_INV_TRANSFER };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditStockFrom;
	CNxEdit	m_nxeditStockUoFrom;
	CNxEdit	m_nxeditStockTo;
	CNxEdit	m_nxeditStockUoTo;
	CNxEdit	m_nxeditTransferAmt;
	CNxEdit	m_nxeditTransferAmtUo;
	CNxEdit	m_nxeditQuantityFrom;
	CNxEdit	m_nxeditQuantityUoFrom;
	CNxEdit	m_nxeditQuantityTo;
	CNxEdit	m_nxeditQuantityUoTo;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticUuLabelFrom;
	CNxStatic	m_nxstaticUoLabelFrom;
	CNxStatic	m_nxstaticUuLabelTo;
	CNxStatic	m_nxstaticUoLabelTo;
	CNxStatic	m_nxstaticUuTransferLabel;
	CNxStatic	m_nxstaticUoTransferLabel;
	CNxStatic	m_nxstaticUuLabelNew;
	CNxStatic	m_nxstaticUoLabelNew;
	CNxStatic	m_nxstaticLocFromNameLabel;
	CNxStatic	m_nxstaticLocToNameLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvTransferDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	NXDATALISTLib::_DNxDataListPtr m_LocationFrom, m_LocationTo;

	// Generated message map functions
	//{{AFX_MSG(CInvTransferDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnKillfocusTransferAmt();
	afx_msg void OnKillfocusTransferAmtUo();
	afx_msg void OnSelChosenLocationFrom(long nRow);
	afx_msg void OnSelChosenLocationTo(long nRow);
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVTRANSFERDLG_H__F8BC49A5_7F68_429B_940D_780876FC1C6C__INCLUDED_)
