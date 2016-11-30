#pragma once

// CLinkCaseHistoryToBillDlg dialog

// (j.jones 2009-08-10 12:17) - PLID 24077 - created

class CLinkCaseHistoryToBillDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLinkCaseHistoryToBillDlg)

public:
	CLinkCaseHistoryToBillDlg(CWnd* pParent);   // standard constructor

	long m_nPatientID;
	long m_nCaseHistoryID;
	CString m_strPatientName;
	CString m_strCaseHistoryDesc;
	COleDateTime m_dtCaseHistoryDate;

// Dialog Data
	enum { IDD = IDD_LINK_CASE_HISTORY_TO_BILL_DLG };
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
