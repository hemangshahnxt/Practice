#if !defined(AFX_VERIFYINSTRUCTIONSDLG_H__7D8008D5_90CD_4F90_962A_F889CF56F852__INCLUDED_)
#define AFX_VERIFYINSTRUCTIONSDLG_H__7D8008D5_90CD_4F90_962A_F889CF56F852__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VerifyInstructionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVerifyInstructionsDlg dialog

class CVerifyInstructionsDlg : public CNxDialog
{
// Construction
public:
	CVerifyInstructionsDlg(CWnd* pParent);   // standard constructor
	CVerifyInstructionsDlg(CString strFileName, CWnd* pParent);
	CString m_strTtxFileName;

	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CVerifyInstructionsDlg)
	enum { IDD = IDD_VERIFY_INS };
	CNxEdit	m_nxeditInstructions;
	CNxIconButton	m_btnCloseInstructions;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVerifyInstructionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVerifyInstructionsDlg)
	afx_msg void OnCloseInstructions();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERIFYINSTRUCTIONSDLG_H__7D8008D5_90CD_4F90_962A_F889CF56F852__INCLUDED_)
