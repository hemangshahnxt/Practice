#if !defined(AFX_PRESCRIPTIONTEMPLATESETUPDLG_H__255E837A_6855_4003_9530_A8B7E85C8CA4__INCLUDED_)
#define AFX_PRESCRIPTIONTEMPLATESETUPDLG_H__255E837A_6855_4003_9530_A8B7E85C8CA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrescriptionTemplateSetupDlg.h : header file
//

// (j.jones 2008-06-06 11:37) - PLID 29154 - created

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionTemplateSetupDlg dialog

class CPrescriptionTemplateSetupDlg : public CNxDialog
{
// Construction
public:
	CPrescriptionTemplateSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrescriptionTemplateSetupDlg)
	enum { IDD = IDD_PRESCRIPTION_TEMPLATE_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrescriptionTemplateSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_DefTemplateCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_TemplateList;

	//tracks counts that we removed templates for
	CArray<long, long> m_aryDeletedTemplateCounts;

	// Generated message map functions
	//{{AFX_MSG(CPrescriptionTemplateSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddTemplate();
	afx_msg void OnDeleteTemplate();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESCRIPTIONTEMPLATESETUPDLG_H__255E837A_6855_4003_9530_A8B7E85C8CA4__INCLUDED_)
