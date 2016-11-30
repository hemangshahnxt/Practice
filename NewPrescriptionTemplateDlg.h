#if !defined(AFX_NEWPRESCRIPTIONTEMPLATEDLG_H__4DD0DD57_BF16_45D6_9EC5_ECA24BB14CE5__INCLUDED_)
#define AFX_NEWPRESCRIPTIONTEMPLATEDLG_H__4DD0DD57_BF16_45D6_9EC5_ECA24BB14CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewPrescriptionTemplateDlg.h : header file
//

// (j.jones 2008-06-09 11:23) - PLID 29154 - added

/////////////////////////////////////////////////////////////////////////////
// CNewPrescriptionTemplateDlg dialog

class CNewPrescriptionTemplateDlg : public CNxDialog
{
// Construction
public:
	CNewPrescriptionTemplateDlg(CWnd* pParent);   // standard constructor

	//these variables will store the new count and template name to use
	long m_nCount;
	CString m_strTemplateName;

	//these arrays will take in the existing counts and templates,
	//so we can warn about duplicates before exiting the dialog
	CArray<long, long> m_aryExistingCounts;
	CArray<CString, CString> m_aryExistingTemplates;

// Dialog Data
	//{{AFX_DATA(CNewPrescriptionTemplateDlg)
	enum { IDD = IDD_NEW_PRESCRIPTION_TEMPLATE_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxEdit	m_editCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewPrescriptionTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_TemplateCombo;

	// Generated message map functions
	//{{AFX_MSG(CNewPrescriptionTemplateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPRESCRIPTIONTEMPLATEDLG_H__4DD0DD57_BF16_45D6_9EC5_ECA24BB14CE5__INCLUDED_)
