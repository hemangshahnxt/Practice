// License.h : header file
//

#if !defined(AFX_LICENSE_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_)
#define AFX_LICENSE_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg dialog

// (a.walling 2008-04-10 12:52) - EditBox.h is no longer used

class CLicenseDlg : public CNxDialog
{
// Construction
public:
	CRichEditCtrl m_RichEdit;

	CLicenseDlg(CWnd* pParent);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLicenseDlg)
	enum { IDD = IDD_LICENSE };
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnIgnore;
	CNxStatic	m_nxstaticAgreementStatic;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

public:
	BOOL m_bRequireAgreement;
	//DRT 5/16/2008 - PLID 30089 - We now also have an Allergan beta NDA license agreement that works, so I 
	//	made the actual resource file that is used customizable for anyone.
	UINT m_nLicenseResourceToUse;

// Implementation
protected:
	HICON m_hIcon;
	BOOL IsLicenseScrolledToEnd();
	BOOL m_bAllowAgreeButton;

	// Generated message map functions
	//{{AFX_MSG(CLicenseDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLicenseScroll();
	//TES 11/7/2007 - PLID 27981 - VS2008 - Our first parameter must be a NMHDR*.
	afx_msg void OnLicenseMsgFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnIgnoreBtn();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_License_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_)
