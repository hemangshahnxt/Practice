#if !defined(AFX_POSRECEIPTCONFIGUREHEADERFOOTERDLG_H__E7911BCF_EEAE_44C1_82ED_D3174D685DA3__INCLUDED_)
#define AFX_POSRECEIPTCONFIGUREHEADERFOOTERDLG_H__E7911BCF_EEAE_44C1_82ED_D3174D685DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PosReceiptConfigureHeaderFooterDlg.h : header file
//
// (j.gruber 2008-01-21 15:41) - PLID 28308 - created for
/////////////////////////////////////////////////////////////////////////////
// CPosReceiptConfigureHeaderFooterDlg dialog

class CPosReceiptConfigureHeaderFooterDlg : public CNxDialog
{
// Construction
public:
	CPosReceiptConfigureHeaderFooterDlg(long nReceiptID, BOOL bIsHeader, CWnd* pParent);   // standard constructor
	~CPosReceiptConfigureHeaderFooterDlg();

// Dialog Data
	//{{AFX_DATA(CPosReceiptConfigureHeaderFooterDlg)
	enum { IDD = IDD_POS_RECEIPT_CONFIGURE_HEADER_FOOTER_DLG };
	CNxEdit	m_edtText;
	CNxStatic	m_nxstaticDescLabel;
	CNxIconButton	m_btnInsertField;
	CNxIconButton	m_btnPrintTest;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPosReceiptConfigureHeaderFooterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL m_bIsHeader;
	long m_nReceiptID;
	CPtrArray m_paryFonts;
	NXDATALIST2Lib::_DNxDataListPtr m_pFontList;
	
	void InsertField(CString strFieldToInsert);
	void InsertFormat(CString strFormatToInsert);

	// Generated message map functions
	//{{AFX_MSG(CPosReceiptConfigureHeaderFooterDlg)
	afx_msg void OnInsertField();
	afx_msg void OnInsertFormat();
	afx_msg void OnPrintTest();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSRECEIPTCONFIGUREHEADERFOOTERDLG_H__E7911BCF_EEAE_44C1_82ED_D3174D685DA3__INCLUDED_)
