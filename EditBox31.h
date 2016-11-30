#if !defined(AFX_EDITBOX31_H__7B994BBA_7F8E_4B08_BF06_E638A15C56BD__INCLUDED_)
#define AFX_EDITBOX31_H__7B994BBA_7F8E_4B08_BF06_E638A15C56BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBox31.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditBox31 dialog

class CEditBox31 : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ComboProviders;
	long m_iInsuranceCoID;
	long m_iProviderID;
	void LoadBox31Info();
	void SaveBox31Info();
	CEditBox31(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditBox31)
	enum { IDD = IDD_EDIT_BOX31 };
	CString	m_Box31;
	CNxEdit	m_nxeditEditBox31;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvBox31Edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBox31)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditBox31)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboProviders(long nRow);
	afx_msg void OnAdvBox31Edit();
	virtual void OnOK();
	afx_msg void OnKillfocusEditBox31();
	afx_msg void OnRequeryFinishedComboProviders(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBOX31_H__7B994BBA_7F8E_4B08_BF06_E638A15C56BD__INCLUDED_)
