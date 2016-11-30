#if !defined(AFX_MODIFIERADDNEW_H__F85BE537_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
#define AFX_MODIFIERADDNEW_H__F85BE537_3614_11D3_A36A_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifierAddNew.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModifierAddNew dialog

class CModifierAddNew : public CNxDialog
{
// Construction
public:
	CModifierAddNew(CWnd* pParent);   // standard constructor
	CString strMod, strDesc;
	double nMultiplier;

	// (z.manning, 05/01/2008) - PLID 29864 - Set button styles
// Dialog Data
	//{{AFX_DATA(CModifierAddNew)
	enum { IDD = IDD_MODIFIER_ADDNEW };
	CNxEdit	m_nxeditModifier;
	CNxEdit	m_nxeditDesc;
	CNxEdit	m_nxeditMultiplier;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifierAddNew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CModifierAddNew)
	virtual void OnOK();
	afx_msg void OnOkBtn();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFIERADDNEW_H__F85BE537_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
