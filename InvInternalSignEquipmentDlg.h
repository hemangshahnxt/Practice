#pragma once


// CInvInternalSignEquipmentDlg dialog

class CInvInternalSignEquipmentDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvInternalSignEquipmentDlg)

public:
	CInvInternalSignEquipmentDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvInternalSignEquipmentDlg();
	virtual int OnInitDialog();
// Dialog Data
	enum { IDD = IDD_INV_INTERNAL_SIGN_EQUIPMENT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedSkipSign();

protected:
	NxButton m_SkipSign;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	long m_nUserID;

public:
	bool m_bShowSkip;
	bool m_bSkipedSign;		// (j.fouts 2012-05-17 17:44) - PLID 50300 - If signiture was skipped
	CString m_strCheckoutUsername;
};
