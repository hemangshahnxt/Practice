#pragma once

// (j.camacho 2013-10-07 14:52) - PLID 58678 - UMLS Setup dialog
// CUMLSLoginDlg dialog

class CUMLSLoginDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CUMLSLoginDlg)

public:
	CUMLSLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUMLSLoginDlg();
	//Dialog Elements
	CNxEdit m_nxeditLoginID;
	CNxEdit m_nxeditPassword;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor m_nxcolorBackground;
	CNxLabel m_nxlabelMakeAccount;

	//Derived Functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();

 
// Dialog Data
	enum { IDD = IDD_UMLSLOGIN };

protected:
	BOOL SaveFields();
	void LoadFields();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void OpenMakeNewAccount();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString m_LoginID;
	CString m_Password;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
};
