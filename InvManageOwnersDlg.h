#pragma once
#include "InventoryRc.h"

// (j.fouts 2012-08-10 09:09) - PLID 50934 - Created.

// CInvManageOwnersDlg dialog

class CInvManageOwnersDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvManageOwnersDlg)

public:
	CInvManageOwnersDlg(CWnd* pParent = NULL, long nPreselectedID = -1);   // standard constructor
	virtual ~CInvManageOwnersDlg();

// Dialog Data
	enum { IDD = IDD_INV_OWNERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void AddOwner();
	void RemoveOwner();
	void RequeryOwnersList();

	//List of all equipment that can have owners
	NXDATALIST2Lib::_DNxDataListPtr m_pItems;
	//List of all possible oweners
	NXDATALIST2Lib::_DNxDataListPtr m_pAllOwners;
	//List of owners for an item
	NXDATALIST2Lib::_DNxDataListPtr m_pItemOwners;

	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;

	long m_nPreselectedID;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	//Add and Remove owners from selected item
	afx_msg void OnBnClickedAddOwner();
	afx_msg void OnBnClickedRemoveOwner();
	void DblClickCellAllOwners(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellItemOwners(LPDISPATCH lpRow, short nColIndex);
	void SelChangedOwnableItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void LeftClickOwnableItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
