#pragma once
#include "InventoryRc.h"
// (s.dhole 2012-03-06 09:12) - PLID 48638 new Dialog
// CInvSelectMultipleCPTDlg dialog

class CInvSelectMultipleCPTDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvSelectMultipleCPTDlg)

public:
	CInvSelectMultipleCPTDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvSelectMultipleCPTDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_MULTIPLE_CPT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	virtual BOOL OnInitDialog();
	BOOL ValidateAndStore();
	NXDATALIST2Lib::_DNxDataListPtr m_CptList;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CArray<long,long> m_arPreSelectedID;
	CString  m_strSelectedIDs;
	CString  m_strSelectedCodes;
	CString m_strType;
	CString m_strItem ;
	void SetDlgCaption();
};
