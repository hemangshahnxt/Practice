#if !defined(AFX_TEMPLATEINTERFACEOPTIONSDLG_H__C78F4D11_CF3F_440A_AE49_13791CA02839__INCLUDED_)
#define AFX_TEMPLATEINTERFACEOPTIONSDLG_H__C78F4D11_CF3F_440A_AE49_13791CA02839__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateInterfaceOptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTemplateInterfaceOptionsDlg dialog

// (z.manning, 12/07/2006) - PLID 23809 - Added an options dialog to the graphical schedule template editor.
class CTemplateInterfaceOptionsDlg : public CNxDialog
{
// Construction
public:
	CTemplateInterfaceOptionsDlg(CWnd* pParent);   // standard constructor

	//TES 6/19/2010 - PLID 5888
	bool m_bUseResourceAvailTemplates;

	void Save();

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CTemplateInterfaceOptionsDlg)
	enum { IDD = IDD_TEMPLATE_INTERFACE_OPTIONS };
	NxButton	m_btnDefAllResources;
	NxButton	m_btnDefAsBlock;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateInterfaceOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTemplateInterfaceOptionsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDefaultLineItemsAsBlocks();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEINTERFACEOPTIONSDLG_H__C78F4D11_CF3F_440A_AE49_13791CA02839__INCLUDED_)
