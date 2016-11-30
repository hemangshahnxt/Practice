#if !defined(AFX_INTERNALAMABILLINGDLG_H__5648DF52_BC67_4F8F_B57E_17121F94D2B7__INCLUDED_)
#define AFX_INTERNALAMABILLINGDLG_H__5648DF52_BC67_4F8F_B57E_17121F94D2B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InternalAMABillingDlg.h : header file
//

//DRT - 7/2/2007 - PLID 26522
/////////////////////////////////////////////////////////////////////////////
// CInternalAMABillingDlg dialog

class CInternalAMABillingDlg : public CNxDialog
{
// Construction
public:
	CInternalAMABillingDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInternalAMABillingDlg)
	enum { IDD = IDD_INTERNAL_AMA_BILLING_DLG };
	CDateTimePicker	m_pickerFrom;
	CDateTimePicker	m_pickerTo;
	CNxStatic	m_nxstaticAmaCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInternalAMABillingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pClientList;

	// Generated message map functions
	//{{AFX_MSG(CInternalAMABillingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAmaCommit();
	afx_msg void OnAmaCompare();
	afx_msg void OnRequeryFinishedAmaList(short nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERNALAMABILLINGDLG_H__5648DF52_BC67_4F8F_B57E_17121F94D2B7__INCLUDED_)
