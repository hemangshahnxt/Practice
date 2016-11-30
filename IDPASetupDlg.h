#if !defined(AFX_IDPASETUPDLG_H__9AB53057_83DD_4553_80AE_CB13F7E60143__INCLUDED_)
#define AFX_IDPASETUPDLG_H__9AB53057_83DD_4553_80AE_CB13F7E60143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IDPASetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIDPASetupDlg dialog

class CIDPASetupDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_ProviderNumberCombo, m_ProviderNameCombo;

	CIDPASetupDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CIDPASetupDlg)
	enum { IDD = IDD_IDPA_SETUP_DLG };
	NxButton	m_btnDoNotIncludeSpaces;
	NxButton	m_btn4Digit;
	NxButton	m_btnIncludeSpaces;
	NxButton	m_btn2Digit;
	NxButton	m_btnShowCount34;
	NxButton	m_btmShowPOSBox21;
	NxButton	m_btnDoNotFill21POS;
	CNxEdit	m_nxeditIdpaPayeeNumber;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDPASetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIDPASetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDPASETUPDLG_H__9AB53057_83DD_4553_80AE_CB13F7E60143__INCLUDED_)
