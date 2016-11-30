#pragma once
// (s.dhole 2011-03-15 14:59) - PLID 42845 Add/Edit Glasses catlog item
#include "InventoryRc.h"
// CInvGlassesItemEditDlg dialog

class CInvGlassesItemEditDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvGlassesItemEditDlg)

public:
	CInvGlassesItemEditDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvGlassesItemEditDlg();
	
	HRESULT Open(CString strTable,CString strName, CString strCode,CString strNameValue, CString strCodeValue,long nGlassOrderProcessType,long nID, CString strDescription);
	CString  m_strNewName, m_strNewCode, m_strNewGlassOrderProcessType;
	long m_nNewID, m_nNewGlassOrderProcessTypeID ;
// Dialog Data
	enum { IDD = IDD_GLASSES_ITEM_EDIT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog() ;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CString m_strNameValue, m_strCodeValue;
	CNxEdit  m_nxeName;
	CNxEdit  m_nxeCode;
	CString m_strTable,m_strName,m_strCode ,m_strDescription;
	long m_nID;
	BOOL SaveData();
	BOOL CheckEmptyField();
	long m_nGlassOrderProcessType;
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
