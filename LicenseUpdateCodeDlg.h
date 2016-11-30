#if !defined(AFX_LICENSEUPDATECODEDLG_H__A2DF8ED2_9925_44F5_9848_F38CC33B3219__INCLUDED_)
#define AFX_LICENSEUPDATECODEDLG_H__A2DF8ED2_9925_44F5_9848_F38CC33B3219__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LicenseUpdateCodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLicenseUpdateCodeDlg dialog
#define ID_DOWNLOAD_CODE	1000
class CLicenseUpdateCodeDlg : public CNxDialog
{
// Construction
public:
	CLicenseUpdateCodeDlg(CWnd* pParent);   // standard constructor

	CString m_strUpdateCode;

// Dialog Data
	//{{AFX_DATA(CLicenseUpdateCodeDlg)
	enum { IDD = IDD_LICENSE_UPDATE_CODE_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditLicenseUpdateCode;
	CNxStatic	m_nxstaticLicenseUpdateCodeCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseUpdateCodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLicenseUpdateCodeDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDownloadUpdateCode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEUPDATECODEDLG_H__A2DF8ED2_9925_44F5_9848_F38CC33B3219__INCLUDED_)
