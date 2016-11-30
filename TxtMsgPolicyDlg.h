// (r.gonet 01-11-2011) - PLID 42010 - Created

#pragma once


// CTxtMsgPolicyDlg dialog

class CTxtMsgPolicyDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTxtMsgPolicyDlg)

private:
	CString m_strPolicyMessage;
	BOOL m_bShowLeaveAsIsButton;

	// Buttons
	CNxIconButton m_btnOptAllOut;
	CNxIconButton m_btnOptAllIn;
	CNxIconButton m_btnLeaveAsIs;
	CNxIconButton m_btnCancel;
	// Edit Boxes
	CNxEdit	m_nxeditPolicy;
	// Color Boxes
	CNxColor m_nxcolorPolicy;

public:
	CTxtMsgPolicyDlg(CWnd* pParent);   // standard constructor
	virtual ~CTxtMsgPolicyDlg();

// Dialog Data
	enum { IDD = IDD_TXTMSG_POLICY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTxtMsgOptAllOutBtn();
	afx_msg void OnBnClickedTxtMsgOptAllInBtn();
	afx_msg void OnBnClickedTxtMsgLeaveasisBtn();

	// (r.gonet 01-11-2011) - PLID 42010 - Toggles on and off the leave as is button
	void ShowLeaveAsIsButton(BOOL bShow);
};
