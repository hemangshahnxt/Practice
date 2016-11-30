#if !defined(AFX_MARKETRETENTIONPRINTSETUPDLG_H__252EFCE0_0394_4918_A18C_E6027BF5108C__INCLUDED_)
#define AFX_MARKETRETENTIONPRINTSETUPDLG_H__252EFCE0_0394_4918_A18C_E6027BF5108C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketRetentionPrintSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionPrintSetupDlg dialog

class CMarketRetentionPrintSetupDlg : public CNxDialog
{
// Construction
public:
	CMarketRetentionPrintSetupDlg(CWnd* pParent = NULL, BOOL bPreview = TRUE);   // standard constructor
	BOOL m_bPreview;
	NXDATALISTLib::_DNxDataListPtr m_pPurposeList;
	CString m_strMultiPurposeIds;
	BOOL m_bExcludeAppts;
	CString m_strPurposeNames;

// Dialog Data
	//{{AFX_DATA(CMarketRetentionPrintSetupDlg)
	enum { IDD = IDD_MARKET_RETENTION_PRINT_SETUP };
	CNxIconButton	m_btnPreview;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnPrintStyleGroupbox;
	NxButton	m_btnReportOn;
	NxButton	m_radioSummary;
	NxButton	m_radioDetailed;
	NxButton	m_radioRetained;
	NxButton	m_radioUnretained;
	NxButton	m_radioApptExclude;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketRetentionPrintSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMarketRetentionPrintSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPreviewBtn();
	afx_msg void OnDetailedRadio();
	afx_msg void OnSummaryRadio();
	afx_msg void OnSelChosenRetentionPurposeList(long nRow);
	afx_msg void OnExcludeAppts();
	afx_msg void OnRequeryFinishedRetentionPurposeList(short nFlags);
	afx_msg void OnUnretainedRadio();
	afx_msg void OnRetainedRadio();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETRETENTIONPRINTSETUPDLG_H__252EFCE0_0394_4918_A18C_E6027BF5108C__INCLUDED_)
