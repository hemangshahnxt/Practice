#if !defined(AFX_RECEIPTCONFIGDLG_H__3D6BB651_617E_4AA2_AD9E_187F952069EF__INCLUDED_)
#define AFX_RECEIPTCONFIGDLG_H__3D6BB651_617E_4AA2_AD9E_187F952069EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReceiptConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReceiptConfigDlg dialog

class CReceiptConfigDlg : public CNxDialog
{
// Construction
public:
	CReceiptConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReceiptConfigDlg)
	enum { IDD = IDD_RECEIPT_CONFIG };
	NxButton	m_btnHideApplyInfo;
	NxButton	m_btnShowTax;
	CNxEdit	m_nxeditReceiptCustomText;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReceiptConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReceiptConfigDlg)
	afx_msg void OnReceiptShowChargeInfo();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECEIPTCONFIGDLG_H__3D6BB651_617E_4AA2_AD9E_187F952069EF__INCLUDED_)
