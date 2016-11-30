#pragma once


// CInvInternalReturnChecklist dialog

class CInvInternalReturnChecklistDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvInternalReturnChecklistDlg)

public:
	CInvInternalReturnChecklistDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvInternalReturnChecklistDlg();

// Dialog Data
	enum { IDD = IDD_INV_INTERNAL_RETURN_CHECKLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual int OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();

protected:
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

public:
	int m_nReqID;	// (j.fouts 2012-05-15 15:37) - PLID 50297 - The ID of the request that we are returning
};
