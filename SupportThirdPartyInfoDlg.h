#if !defined(AFX_SUPPORTTHIRDPARTYINFODLG_H__ECB4E0D0_4F29_4943_B6F2_795F25115621__INCLUDED_)
#define AFX_SUPPORTTHIRDPARTYINFODLG_H__ECB4E0D0_4F29_4943_B6F2_795F25115621__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SupportThirdPartyInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSupportThirdPartyInfoDlg dialog

class CSupportThirdPartyInfoDlg : public CNxDialog
{
// Construction
public:
	CSupportThirdPartyInfoDlg(CWnd* pParent);   // standard constructor
	OLE_COLOR m_Color;
	long m_nPatientID;

// Dialog Data
	//{{AFX_DATA(CSupportThirdPartyInfoDlg)
	enum { IDD = IDD_SUPPORT_THIRD_PARTY_INFO_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditMirrorPath;
	CNxEdit	m_nxeditInformPath;
	CNxEdit	m_nxeditUnitedPath;
	CNxEdit	m_nxeditPalmPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSupportThirdPartyInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSupportThirdPartyInfoDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUPPORTTHIRDPARTYINFODLG_H__ECB4E0D0_4F29_4943_B6F2_795F25115621__INCLUDED_)
