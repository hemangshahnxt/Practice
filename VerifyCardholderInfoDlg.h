#if !defined(AFX_VERIFYCARDHOLDERINFODLG_H__73D40348_C3A9_4764_B911_FBCF3A8EFF13__INCLUDED_)
#define AFX_VERIFYCARDHOLDERINFODLG_H__73D40348_C3A9_4764_B911_FBCF3A8EFF13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VerifyCardholderInfoDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CVerifyCardholderInfoDlg dialog
// (j.gruber 2007-08-03 11:29) - PLID 26582 - created for
class CVerifyCardholderInfoDlg : public CNxDialog
{
// Construction
public:
	CVerifyCardholderInfoDlg(CString strCardHolderName, long nPatientID, CWnd* pParent);   // standard constructor

	CString m_strCardholderName;
	CString m_strCardholderAddress1;
	CString m_strCardholderAddress2;
	CString m_strCardholderZip;
	CString m_strCardholderSecurityCode;
	long m_nPatientID;
	BOOL m_bCodeIllegible;
	BOOL m_bCodeNotProvidedByCustomer;
	
// Dialog Data
	//{{AFX_DATA(CVerifyCardholderInfoDlg)
	enum { IDD = IDD_VERIFY_CARDHOLDER_INFO_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_edtCVVD;
	CNxEdit	m_editCardholderName;
	CNxEdit	m_edtAddress2;
	CNxEdit	m_edtAddress1;
	CNxEdit	m_editZipCode;
	NxButton	m_btnIllegible;
	NxButton	m_btnNotProvided;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVerifyCardholderInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CVerifyCardholderInfoDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnCodeIllegible();
	afx_msg void OnCodeNotProvided();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERIFYCARDHOLDERINFODLG_H__73D40348_C3A9_4764_B911_FBCF3A8EFF13__INCLUDED_)
