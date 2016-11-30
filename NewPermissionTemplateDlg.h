#if !defined(AFX_NEWPERMISSIONTEMPLATEDLG_H__46BD1D6F_A4E5_4BC1_943D_BD84658FE9E7__INCLUDED_)
#define AFX_NEWPERMISSIONTEMPLATEDLG_H__46BD1D6F_A4E5_4BC1_943D_BD84658FE9E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewPermissionTemplateDlg.h : header file
//
#include "contactsrc.h"

/////////////////////////////////////////////////////////////////////////////
// CNewPermissionTemplateDlg dialog

class CNewPermissionTemplateDlg : public CNxDialog
{
// Construction
public:
	CNewPermissionTemplateDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewPermissionTemplateDlg)
	enum { IDD = IDD_NEW_PERMISSION_TEMPLATE };
	CString	m_strName;
	CString	m_strDescription;
	CNxEdit	m_nxeditEditNewpermtempName;
	CNxEdit	m_nxeditEditNewpermtempDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewPermissionTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlItems;

	// Generated message map functions
	//{{AFX_MSG(CNewPermissionTemplateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPERMISSIONTEMPLATEDLG_H__46BD1D6F_A4E5_4BC1_943D_BD84658FE9E7__INCLUDED_)
