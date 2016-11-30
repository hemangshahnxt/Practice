#if !defined(AFX_QUICKBOOKSLINKCONFIGDEMOGDLG_H__26FBE3FD_E3AC_4455_8E62_07EAEBDEF28C__INCLUDED_)
#define AFX_QUICKBOOKSLINKCONFIGDEMOGDLG_H__26FBE3FD_E3AC_4455_8E62_07EAEBDEF28C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuickbooksLinkConfigDemogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkConfigDemogDlg dialog

class CQuickbooksLinkConfigDemogDlg : public CNxDialog
{
// Construction
public:
	CQuickbooksLinkConfigDemogDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQuickbooksLinkConfigDemogDlg)
	enum { IDD = IDD_QUICKBOOKS_LINK_CONFIG_DEMOG_DLG };
	NxButton	m_checkCharge;
	NxButton	m_checkCheck;
	NxButton	m_checkCash;
	NxButton	m_checkPatID;
	NxButton	m_checkPatient;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickbooksLinkConfigDemogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CQuickbooksLinkConfigDemogDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKBOOKSLINKCONFIGDEMOGDLG_H__26FBE3FD_E3AC_4455_8E62_07EAEBDEF28C__INCLUDED_)
