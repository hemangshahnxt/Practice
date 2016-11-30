#if !defined(AFX_EMRTEXTDLG_H__89AC719D_FB2D_4DBD_81BB_541BE0BBDDF9__INCLUDED_)
#define AFX_EMRTEXTDLG_H__89AC719D_FB2D_4DBD_81BB_541BE0BBDDF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRTextDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRTextDlg dialog

class CEMRTextDlg : public CNxDialog
{
// Construction
public:
	CEMRTextDlg(CWnd* pParent);   // standard constructor

	CString m_text;

// Dialog Data
	//{{AFX_DATA(CEMRTextDlg)
	enum { IDD = IDD_EMR_TEXT_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditEditEmrText;
	CNxStatic	m_nxstaticEmrTextTitle;
	CNxIconButton m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRTextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMRTextDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTEXTDLG_H__89AC719D_FB2D_4DBD_81BB_541BE0BBDDF9__INCLUDED_)
