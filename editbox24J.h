//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_EDITBOX24J_H__89E2B921_533B_11D4_8FA3_0001024317D6__INCLUDED_)
#define AFX_EDITBOX24J_H__89E2B921_533B_11D4_8FA3_0001024317D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBox24J.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEditBox24J dialog

class CEditBox24J : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ComboProviders;
	CEditBox24J(CWnd* pParent);   // standard constructor
	long m_iInsuranceCoID;
	long m_iProviderID;

// Dialog Data
	//{{AFX_DATA(CEditBox24J)
	enum { IDD = IDD_EDIT_BOX_24J };
	CString	m_Box24J;
	CString	m_Box24IQual;
	CNxEdit	m_nxeditEditBox24iQual;
	CNxEdit	m_nxeditEditBox24jNum;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvBox24JEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBox24J)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEditBox24J)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectionChangeComboProviders(long iNewRow);
	virtual void OnOK();
	afx_msg void OnKillFocusEditBox24J();
	afx_msg void OnRequeryFinishedComboProviders(short nFlags);
	afx_msg void OnAdvBox24JEdit();
	afx_msg void OnKillfocusEditBox24iQual();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void LoadBox24Info();
	void SaveBox24Info();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBOX24J_H__89E2B921_533B_11D4_8FA3_0001024317D6__INCLUDED_)
