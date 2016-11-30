#if !defined(AFX_SUPPORTCONNECTIONDLG_H__CDF86F60_0DAA_4A66_860B_02840783EDE9__INCLUDED_)
#define AFX_SUPPORTCONNECTIONDLG_H__CDF86F60_0DAA_4A66_860B_02840783EDE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SupportConnectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSupportConnectionDlg dialog

class CSupportConnectionDlg : public CNxDialog
{
// Construction
public:
	CSupportConnectionDlg(CWnd* pParent);   // standard constructor

	OLE_COLOR m_Color;
	long m_nPatientID;

// Dialog Data
	//{{AFX_DATA(CSupportConnectionDlg)
	enum { IDD = IDD_SUPPORT_CONNECTION_INFO_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditPca;
	CNxEdit	m_nxeditPcaIp;
	CNxEdit	m_nxeditLogin;
	CNxEdit	m_nxeditPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSupportConnectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSupportConnectionDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUPPORTCONNECTIONDLG_H__CDF86F60_0DAA_4A66_860B_02840783EDE9__INCLUDED_)
