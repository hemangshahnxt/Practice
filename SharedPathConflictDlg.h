// (c.haag 2013-01-15) - PLID 59257 - Initial implementation. This dialog will appear
// if this workstation has multiple dock subkeys with the same shared path.

#pragma once

// CSharedPathConflictDlg dialog

class CSharedPathConflictDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSharedPathConflictDlg)

public:
	CSharedPathConflictDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedPathConflictDlg();

// Dialog Data
	enum { IDD = IDD_SHAREDPATHCONFLICT };

protected:
	CNxStatic m_nxs1;
	CNxStatic m_nxs2;
	CNxIconButton m_nxbOK;

protected:
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSharedpathconflictEmail();
};
