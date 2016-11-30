#pragma once

// (j.jones 2009-06-24 15:41) - PLID 24076 - created

// CLinkEMNToBillDlg dialog

class CLinkEMNToBillDlg : public CNxDialog
{

public:
	CLinkEMNToBillDlg(CWnd* pParent);   // standard constructor

	long m_nPatientID;
	long m_nEMNID;
	CString m_strPatientName;
	CString m_strEMNDesc;
	COleDateTime m_dtEMNDate;

// Dialog Data
	enum { IDD = IDD_LINK_EMN_TO_BILL_DLG };
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxStatic	m_nxstaticDesc;
	CNxStatic	m_nxstaticDate;
	NxButton	m_checkIncludeLinked;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg void OnCheckIncludeAlreadyLinked();
};
