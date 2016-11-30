#if !defined(AFX_CARECREDITMARKETINGDLG_H__86DFB575_448C_4661_922C_CBDE8BEE33D1__INCLUDED_)
#define AFX_CARECREDITMARKETINGDLG_H__86DFB575_448C_4661_922C_CBDE8BEE33D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CareCreditMarketingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCareCreditMarketingDlg dialog

class CCareCreditMarketingDlg : public CNxDialog
{
// Construction
public:
	CCareCreditMarketingDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CCareCreditMarketingDlg)
	enum { IDD = IDD_CARE_CREDIT_MARKING_DLG };
	NxButton		m_btnDontShow;
	CNxIconButton	m_btnOpenCareCredit;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnUpdateLicense;
	CNxIconButton	m_btnEmailNextech;
	CNxIconButton	m_btnOpenCareCreditWebsite;
	CNxStatic	m_nxstaticCarecreditMarketingText;
	CNxStatic	m_nxstaticUpdateLicenseText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCareCreditMarketingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCareCreditMarketingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnOpenCarecredit();
	afx_msg void OnUpdateLicense();
	afx_msg void OnEmailNexTech();
	afx_msg void OnOpenCareCreditWebpage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CARECREDITMARKETINGDLG_H__86DFB575_448C_4661_922C_CBDE8BEE33D1__INCLUDED_)
