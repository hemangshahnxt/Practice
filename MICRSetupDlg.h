#if !defined(AFX_MICRSETUPDLG_H__73E37D07_7D94_4D42_8455_3D4FCDFC4087__INCLUDED_)
#define AFX_MICRSETUPDLG_H__73E37D07_7D94_4D42_8455_3D4FCDFC4087__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MICRSetupDlg.h : header file
//

// (j.jones 2007-05-09 16:55) - PLID 25550 - created

/////////////////////////////////////////////////////////////////////////////
// CMICRSetupDlg dialog

class CMICRSetupDlg : public CNxDialog
{
// Construction
public:
	CMICRSetupDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMICRSetupDlg)
	enum { IDD = IDD_MICR_SETUP_DLG };
	NxButton	m_btnUseLegacyForm;
	CNxEdit	m_nxeditEditMicrProvCodeQual;
	CNxEdit	m_nxeditEditMicrRefOrderQual;
	CNxEdit	m_nxeditEditMicrBox44Qual;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMICRSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMICRSetupDlg)
	afx_msg void OnCheckUseOldMicrForm();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MICRSETUPDLG_H__73E37D07_7D94_4D42_8455_3D4FCDFC4087__INCLUDED_)
