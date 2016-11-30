#pragma once

// (s.dhole 2011-03-15 12:11) - PLID 42835 new Dialog to Maintain Glasses Catalog custom items
// CInvGlassesItemDlg dialog
#include "InventoryRc.h"

class CInvGlassesItemDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvGlassesItemDlg)

public:
	virtual ~CInvGlassesItemDlg();
	BOOL OnInitDialog();
	HRESULT Open(CString strTable,CString strName, CString strCode,CString strSQL, CString strDescription);
	CInvGlassesItemDlg(CWnd* pParent);   // standard constructor
	

// Dialog Data
	enum { IDD = IDD_GLASSES_ITEM_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_strMultiID;
	CString m_strTable;
	CString m_strName; 
	CString m_strCode;
	CString m_strDescription;
	CString m_strSQL;
	
	BOOL ValidateAndStore();
	void EditData();
	void DeleteData();

	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	void GenerateMenu( CPoint &pt);
	
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedItemList;

	DECLARE_MESSAGE_MAP()
public:
	
	CString   GetMultiSelectIDString();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void RButtonDownGlassesDesignList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
