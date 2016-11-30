#if !defined(AFX_USERDRAWERDLG_H__F36F042C_B85F_4278_A69A_2EAD962B67B1__INCLUDED_)
#define AFX_USERDRAWERDLG_H__F36F042C_B85F_4278_A69A_2EAD962B67B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserDrawerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserDrawerDlg dialog

class CUserDrawerDlg : public CNxDialog
{
// Construction
public:
	CUserDrawerDlg(CWnd* pParent);   // standard constructor

	CString m_strName;
	COleCurrency m_cyCash, m_cyCheck, m_cyCharge, m_cyGift;
	COleCurrency m_cyUserCash, m_cyUserCheck, m_cyUserCharge, m_cyUserGift;
	COleCurrency m_cyOpenAmt;

	long m_nID;			//id of the item we're loading
	BOOL m_bEditing;	//are we editing an already saved?  or saving new?

	COleCurrency GetUserCash();
	COleCurrency GetUserCheck();
	COleCurrency GetUserCharge();
	COleCurrency GetUserGift();

	BOOL Save();

// Dialog Data
	//{{AFX_DATA(CUserDrawerDlg)
	enum { IDD = IDD_USER_DRAWER_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditSysCash;
	CNxEdit	m_nxeditSysCheck;
	CNxEdit	m_nxeditSysCharge;
	CNxEdit	m_nxeditSysGift;
	CNxEdit	m_nxeditSysTotal;
	CNxEdit	m_nxeditUserCash;
	CNxEdit	m_nxeditUserCheck;
	CNxEdit	m_nxeditUserCharge;
	CNxEdit	m_nxeditUserGift;
	CNxEdit	m_nxeditUserTotal;
	CNxStatic	m_nxstaticDrawerNameLabel;
	CNxStatic	m_nxstaticOpenAmtLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnPreviewDrawer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserDrawerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateUserTotal();

	// Generated message map functions
	//{{AFX_MSG(CUserDrawerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusUserCash();
	afx_msg void OnKillfocusUserCheck();
	afx_msg void OnKillfocusUserCharge();
	afx_msg void OnKillfocusUserGift();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPreviewDrawer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERDRAWERDLG_H__F36F042C_B85F_4278_A69A_2EAD962B67B1__INCLUDED_)
