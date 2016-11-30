#if !defined(AFX_MARKETPRINTSETUPDLG_H__E49738CB_FEB7_4A0E_B6A0_7C2E863043BA__INCLUDED_)
#define AFX_MARKETPRINTSETUPDLG_H__E49738CB_FEB7_4A0E_B6A0_7C2E863043BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketPrintSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketPrintSetupDlg dialog

class CMarketPrintSetupDlg : public CNxDialog
{
// Construction
public:
	CMarketPrintSetupDlg(CWnd* pParent = NULL, BOOL bPreview = TRUE);   // standard constructor
	BOOL m_bPreview;

// Dialog Data
	//{{AFX_DATA(CMarketPrintSetupDlg)
	enum { IDD = IDD_MARKET_PRINT_SETUP_DLG };
	CNxIconButton m_btnPreview;
	CNxIconButton m_btnCancel;
	NxButton	m_btnPrintStyleGroupbox;
	NxButton	m_radioSummary;
	NxButton	m_radioDetailed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketPrintSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMarketPrintSetupDlg)
	afx_msg void OnPreviewBtn();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETPRINTSETUPDLG_H__E49738CB_FEB7_4A0E_B6A0_7C2E863043BA__INCLUDED_)
