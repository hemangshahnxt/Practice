#if !defined(AFX_POSPRINTERSETTINGSDLG_H__2F70F062_AD0A_45D5_985F_D9BC9AA00CC8__INCLUDED_)
#define AFX_POSPRINTERSETTINGSDLG_H__2F70F062_AD0A_45D5_985F_D9BC9AA00CC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POSPrinterSettingsDlg.h : header file
//

// (j.gruber 2007-05-08 12:31) - PLID 25772 - class created

/////////////////////////////////////////////////////////////////////////////
// CPOSPrinterSettingsDlg dialog

class CPOSPrinterSettingsDlg : public CNxDialog
{
// Construction
public:
	CPOSPrinterSettingsDlg(COPOSPrinterDevice *pPOSPrinter, CWnd* pParent);   // standard constructor
	COPOSPrinterDevice *  GetOPOSPrinterDevice();

// Dialog Data
	//{{AFX_DATA(CPOSPrinterSettingsDlg)
	enum { IDD = IDD_POS_PRINTER_CONFIG_DLG };
	NxButton	m_btnOff;
	NxButton	m_btnOn;
	CNxIconButton	m_btnPOSPrinterTest;
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditPosPrinterName;
	CNxStatic	m_nxstaticPosPrinterStatus;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPOSPrinterSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	
	CString m_strPrinterName;
	COPOSPrinterDevice *m_pPOSPrinterDevice;
	long m_nPrinterInUse;
	void ApplySettings();
	

	
	// Generated message map functions
	//{{AFX_MSG(CPOSPrinterSettingsDlg)
	afx_msg void OnPosPrinterOn();
	afx_msg void OnPosPrinterOff();
	afx_msg void OnPosPrinterTest();
	afx_msg void OnKillfocusPosPrinterName();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePosPrinterName();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSPRINTERSETTINGSDLG_H__2F70F062_AD0A_45D5_985F_D9BC9AA00CC8__INCLUDED_)
