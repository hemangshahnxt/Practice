#if !defined(AFX_USERPERMISSIONSDLG_H__3DAB397D_02F0_4EC4_8F8D_06263E02BF13__INCLUDED_)
#define AFX_USERPERMISSIONSDLG_H__3DAB397D_02F0_4EC4_8F8D_06263E02BF13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserPermissionsDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CUserPermissionsDlg dialog

class CUserPermissionsDlg : public CNxDialog
{
// Construction
public:
	CUserPermissionsDlg(CWnd* pParent);   // standard constructor
	virtual ~CUserPermissionsDlg();
	int m_id;
// Dialog Data
	//{{AFX_DATA(CUserPermissionsDlg)
	enum { IDD = IDD_USER_PERMISSION_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserPermissionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPtrArray m_box;
	CFont m_font;
	void CreateBoxes(RECT &area, RECT &box, double font);
	// Generated message map functions
	//{{AFX_MSG(CUserPermissionsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERPERMISSIONSDLG_H__3DAB397D_02F0_4EC4_8F8D_06263E02BF13__INCLUDED_)
