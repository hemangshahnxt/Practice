#if !defined(AFX_CHOOSEIDDLG_H__0B41BDE1_36C8_4510_9C0A_C2CFE0ECA42B__INCLUDED_)
#define AFX_CHOOSEIDDLG_H__0B41BDE1_36C8_4510_9C0A_C2CFE0ECA42B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseIdDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseIdDlg dialog

class CChooseIdDlg : public CNxDialog
{
// Construction
public:
	CChooseIdDlg(CWnd* pParent);   // standard constructor

	CString m_strDefaultLabel;

// Dialog Data
	//{{AFX_DATA(CChooseIdDlg)
	enum { IDD = IDD_CHOOSE_ID_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticAccountLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseIdDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CStringArray m_strInIDs;
	CStringArray m_strInNames;

	CString m_strOutID;

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlCombo;

	// Generated message map functions
	//{{AFX_MSG(CChooseIdDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEIDDLG_H__0B41BDE1_36C8_4510_9C0A_C2CFE0ECA42B__INCLUDED_)
