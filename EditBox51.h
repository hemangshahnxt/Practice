#if !defined(AFX_EDITBOX51_H__DEA7FAEE_551A_40F6_9D38_7F6EAE425284__INCLUDED_)
#define AFX_EDITBOX51_H__DEA7FAEE_551A_40F6_9D38_7F6EAE425284__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBox51.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditBox51 dialog

class CEditBox51 : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ComboProviders;
	long m_iInsuranceCoID;
	long m_iProviderID;
	void LoadBox51Info();
	void SaveBox51Info();
	CEditBox51(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditBox51)
	enum { IDD = IDD_EDIT_BOX51 };
	CString	m_Box51;
	CNxEdit	m_nxeditEditBox51;
	CNxStatic	m_nxstaticBox51Label;
	CNxStatic	m_nxstaticBox51Caption;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvBox51Edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBox51)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditBox51)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboProviders(long nRow);
	afx_msg void OnAdvBox51Edit();
	virtual void OnOK();
	afx_msg void OnKillfocusEditBox51();
	afx_msg void OnRequeryFinishedComboProviders(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBOX51_H__DEA7FAEE_551A_40F6_9D38_7F6EAE425284__INCLUDED_)
