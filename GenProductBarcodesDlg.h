#if !defined(AFX_GENPRODUCTBARCODESDLG_H__88FB3E54_879E_41E6_B359_CA7DFBAE5CA8__INCLUDED_)
#define AFX_GENPRODUCTBARCODESDLG_H__88FB3E54_879E_41E6_B359_CA7DFBAE5CA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GenProductBarcodesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGenProductBarcodesDlg dialog

class CGenProductBarcodesDlg : public CNxDialog
{
// Construction
public:
	CGenProductBarcodesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGenProductBarcodesDlg)
	enum { IDD = IDD_GEN_PRODUCT_BARCODES };
	int		m_nStart;
	CNxEdit	m_nxeditEditBargenStart;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenProductBarcodesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlProducts;

	void GenerateBarcodes();
	void TryPrintBarcodes();

	// Generated message map functions
	//{{AFX_MSG(CGenProductBarcodesDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENPRODUCTBARCODESDLG_H__88FB3E54_879E_41E6_B359_CA7DFBAE5CA8__INCLUDED_)
