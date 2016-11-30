#if !defined(AFX_RENAMEFILEDLG1_H__D1FADFAE_ACFE_4775_A719_653FF94F3A2F__INCLUDED_)
#define AFX_RENAMEFILEDLG1_H__D1FADFAE_ACFE_4775_A719_653FF94F3A2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenameFileDlg1.h : header file
//

#define WM_TEMP_TIMER WM_USER+522

/////////////////////////////////////////////////////////////////////////////
// RenameFileDlg dialog

class CRenameFileDlg : public CNxDialog
{
// Construction
public:
	CRenameFileDlg(CString strSourceFilename, CString strDstPath, CWnd* pParent);
	BOOL OnInitDialog();
	CString m_strDstFullPath;
	void OnTimer(UINT nIDEvent);
	void OnOK();

// Dialog Data
	//{{AFX_DATA(RenameFileDlg)
	enum { IDD = IDD_RENAME_FILE };
		// NOTE: the ClassWizard will add data members here
	CNxEdit m_edtNewFilename;
	CNxStatic	m_nxstaticRenameFileLabel;
	CNxStatic	m_nxstaticRenameLabel1;
	CNxStatic	m_nxstaticFileExtension;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RenameFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRenameFileDlg(CWnd* pParent);   // standard constructor
	CString m_strOldFilename;
	CString m_strExtension;
	// Generated message map functions
	//{{AFX_MSG(RenameFileDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMEFILEDLG1_H__D1FADFAE_ACFE_4775_A719_653FF94F3A2F__INCLUDED_)
