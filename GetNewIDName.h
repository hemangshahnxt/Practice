#if !defined(AFX_GETNEWIDNAME_H__BF810721_F316_11D2_B68F_0000C0832801__INCLUDED_)
#define AFX_GETNEWIDNAME_H__BF810721_F316_11D2_B68F_0000C0832801__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetNewIDName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGetNewIDName dialog

class CGetNewIDName : public CNxDialog
{
// Construction
public:

	CString* m_pNewName;
	CString m_strCaption;

	int m_nMaxLength;

	CGetNewIDName(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGetNewIDName)
	enum { IDD = IDD_NEWIDNAME };
	CNxEdit	m_Name;
	CNxStatic	m_nxstaticCaption;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetNewIDName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetNewIDName)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETNEWIDNAME_H__BF810721_F316_11D2_B68F_0000C0832801__INCLUDED_)
