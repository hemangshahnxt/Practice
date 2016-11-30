#if !defined(AFX_CALCULATEPERCENTAGEDLG_H__EEB3D8AD_E3C4_4016_A4F5_A43060CB0DF1__INCLUDED_)
#define AFX_CALCULATEPERCENTAGEDLG_H__EEB3D8AD_E3C4_4016_A4F5_A43060CB0DF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CalculatePercentageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCalculatePercentageDlg dialog

class CCalculatePercentageDlg : public CNxDialog
{
// Construction
public:
	CCalculatePercentageDlg(CWnd* pParent);   // standard constructor

	COleCurrency m_cyOriginalAmt,
				 m_cyFinalAmt;

	CString m_strOriginalAmt;

	double m_dblPercentage;

	CBrush m_bg;

	int m_color;

// Dialog Data
	//{{AFX_DATA(CCalculatePercentageDlg)
	enum { IDD = IDD_CALCULATE_PERCENTAGE_DLG };
	CNxColor	m_bkg;
	CNxEdit	m_nxeditOriginalAmount;
	CNxEdit	m_nxeditEditPercentage;
	CNxEdit	m_nxeditEditCalculatedAmount;
	CNxStatic	m_nxstaticLabelOriginalAmt;
	CNxStatic	m_nxstaticCurrencyLabel1;
	CNxStatic	m_nxstaticLabelPercent;
	CNxStatic	m_nxstaticPercent;
	CNxStatic	m_nxstaticCalculatedTotal;
	CNxStatic	m_nxstaticCurrencyLabel2;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnCalculate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCalculatePercentageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void GetNonNegativeAmountExtent(int &nStart, int &nFinish);

	// Generated message map functions
	//{{AFX_MSG(CCalculatePercentageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCalculate();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALCULATEPERCENTAGEDLG_H__EEB3D8AD_E3C4_4016_A4F5_A43060CB0DF1__INCLUDED_)
