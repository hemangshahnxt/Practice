#if !defined(AFX_PROCEDUREDESCRIPTIONDLG_H__489FFF0C_48D3_422A_BAAA_FA41658467B5__INCLUDED_)
#define AFX_PROCEDUREDESCRIPTIONDLG_H__489FFF0C_48D3_422A_BAAA_FA41658467B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureDescriptionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureDescriptionDlg dialog

class CProcedureDescriptionDlg : public CNxDialog
{
// Construction
public:
	CProcedureDescriptionDlg(long nCPTID, CWnd* pParent);   // standard constructor
	long m_nServiceID;

// Dialog Data
	//{{AFX_DATA(CProcedureDescriptionDlg)
	enum { IDD = IDD_PROCEDURE_DESCRIPTION_DLG };
	CNxEdit	m_nxeditProcedureDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureDescriptionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcedureDescriptionDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREDESCRIPTIONDLG_H__489FFF0C_48D3_422A_BAAA_FA41658467B5__INCLUDED_)
