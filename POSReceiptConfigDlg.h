#if !defined(AFX_POSRECEIPTCONFIGDLG_H__EDA7D22C_20E1_4A0D_9F09_B0A2FAAD93C6__INCLUDED_)
#define AFX_POSRECEIPTCONFIGDLG_H__EDA7D22C_20E1_4A0D_9F09_B0A2FAAD93C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POSReceiptConfigDlg.h : header file
//

// (j.gruber 2007-05-08 12:32) - PLID 25931 - Class Created For


/////////////////////////////////////////////////////////////////////////////
// CPOSReceiptConfigDlg dialog

class CPOSReceiptConfigDlg : public CNxDialog
{
// Construction
public:
	CPOSReceiptConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPOSReceiptConfigDlg)
	enum { IDD = IDD_POS_RECEIPT_SETUP };
	NxButton	m_btnShowLogo;
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditPosPrinterLogoPath;
	CNxEdit	m_nxeditPosPrinterLineAfterHeader;
	CNxEdit	m_nxeditPosPrinterLineAfterDetails;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPOSReceiptConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pReceiptList;
	CString m_strLogoPath;
	CString m_strLocationFormat;
	CString m_strFooterMessage;
	long m_nLinesAfterDetails;
	long m_nLinesAfterHeader;
	long m_nShowLogo;
	
	

	// (j.gruber 2008-01-21 15:55) - PLID 28308 - took out kill focus messages for the header and footer text boxes
	//that were here and added buttons
	// Generated message map functions
	//{{AFX_MSG(CPOSReceiptConfigDlg)
	virtual void OnOK();
	afx_msg void OnPosPrinterBrowse();
	afx_msg void OnKillfocusPosPrinterLineAfterDetails();
	afx_msg void OnKillfocusPosPrinterLineAfterHeader();
	afx_msg void OnKillfocusPosPrinterLogoPath();
	afx_msg void OnPosPrinterReceiptNameConfig();
	afx_msg void OnSelChangingPosPrinterReceiptsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenPosPrinterReceiptsList(LPDISPATCH lpRow);
	afx_msg void OnPosPrinterShowLogo();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedPosPrinterReceiptsList(short nFlags);
	afx_msg void OnPosPrinterMakeDefault();
	afx_msg void OnFormatLocationHeader();
	afx_msg void OnFormatFooter();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSRECEIPTCONFIGDLG_H__EDA7D22C_20E1_4A0D_9F09_B0A2FAAD93C6__INCLUDED_)
