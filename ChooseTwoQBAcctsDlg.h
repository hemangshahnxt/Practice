#if !defined(AFX_CHOOSETWOQBACCTSDLG_H__BC916981_7B28_4A9A_B239_222234AC721C__INCLUDED_)
#define AFX_CHOOSETWOQBACCTSDLG_H__BC916981_7B28_4A9A_B239_222234AC721C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseTwoQBAcctsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseTwoQBAcctsDlg dialog

class CChooseTwoQBAcctsDlg : public CNxDialog
{
// Construction
public:
	CChooseTwoQBAcctsDlg(CWnd* pParent);   // standard constructor

	CString m_strFromAccountLabel;
	CString m_strToAccountLabel;

	BOOL m_bSettingDefaults;

// Dialog Data
	//{{AFX_DATA(CChooseTwoQBAcctsDlg)
	enum { IDD = IDD_CHOOSE_TWO_QB_ACCTS_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticFromAccountLabel;
	CNxStatic	m_nxstaticToAccountLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseTwoQBAcctsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CStringArray m_strInFromIDs;
	CStringArray m_strInFromNames;
	CStringArray m_strInToIDs;
	CStringArray m_strInToNames;

	CString m_strFromOutID;
	CString m_strToOutID;

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlFromCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlToCombo;

	// Generated message map functions
	//{{AFX_MSG(CChooseTwoQBAcctsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSETWOQBACCTSDLG_H__BC916981_7B28_4A9A_B239_222234AC721C__INCLUDED_)
