#if !defined(AFX_NYWCSETUPDLG_H__1E19E697_BF61_4502_AF5E_FAE1A6C1F991__INCLUDED_)
#define AFX_NYWCSETUPDLG_H__1E19E697_BF61_4502_AF5E_FAE1A6C1F991__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NYWCSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNYWCSetupDlg dialog

class CNYWCSetupDlg : public CNxDialog
{
// Construction
public:
	CNYWCSetupDlg(CWnd* pParent);   // standard constructor

	void EnableOverride(BOOL bEnable);

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CNYWCSetupDlg)
	enum { IDD = IDD_NYWC_SETUP_DLG };
	NxButton	m_btnUseIllnessDate;
	NxButton	m_btnUseAccidentDate;
	NxButton	m_radioBillProvider;
	NxButton	m_radioOverride;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditAddress;
	CNxEdit	m_nxeditCity;
	CNxEdit	m_nxeditState;
	CNxEdit	m_nxeditZip;
	CNxEdit	m_nxeditPhone;
	CNxStatic	m_nxstaticBox33NameLabel;
	CNxStatic	m_nxstaticO2static;
	CNxStatic	m_nxstaticO3static;
	CNxStatic	m_nxstaticO5static;
	CNxStatic	m_nxstaticO6static;
	CNxStatic	m_nxstaticO4static;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNYWCSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNYWCSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioUsebillprov();
	afx_msg void OnRadioUseovrrdprov();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NYWCSETUPDLG_H__1E19E697_BF61_4502_AF5E_FAE1A6C1F991__INCLUDED_)
