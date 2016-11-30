#if !defined(AFX_SELECTCONTACTDLG_H__EB379DE0_F4FD_4E4F_A5DF_FB60302CFECE__INCLUDED_)
#define AFX_SELECTCONTACTDLG_H__EB379DE0_F4FD_4E4F_A5DF_FB60302CFECE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectContactDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectContactDlg dialog

class CSelectContactDlg : public CNxDialog
{
// Construction
public:
	CSelectContactDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectContactDlg)
	enum { IDD = IDD_SELECT_CONTACT_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	CString m_strLast, m_strFirst, m_strMiddle, m_strCompany, m_strTitle;
	long m_nID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectContactDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;

	// Generated message map functions
	//{{AFX_MSG(CSelectContactDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTCONTACTDLG_H__EB379DE0_F4FD_4E4F_A5DF_FB60302CFECE__INCLUDED_)
