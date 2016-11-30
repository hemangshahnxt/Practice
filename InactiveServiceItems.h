#if !defined(AFX_INACTIVESERVICEITEMS_H__12E0ABF2_8472_4AEF_B735_9A8675391BA7__INCLUDED_)
#define AFX_INACTIVESERVICEITEMS_H__12E0ABF2_8472_4AEF_B735_9A8675391BA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactiveServiceItems.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactiveServiceItems dialog

class CInactiveServiceItems : public CNxDialog
{
// Construction
public:
	void RestoreItem(long ID);
	NXDATALISTLib::_DNxDataListPtr m_List;

	//1 - CPT, 2 - Inventory Items
	long m_ServiceType;

	BOOL m_Changed;

	CInactiveServiceItems(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInactiveServiceItems)
	enum { IDD = IDD_INACTIVE_SERVICE_ITEMS };
	CNxIconButton	m_btnClose;
	CNxStatic	m_nxstaticInactiveTitleLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactiveServiceItems)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInactiveServiceItems)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRButtonDownInactiveItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickInactiveItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellInactiveItemList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVESERVICEITEMS_H__12E0ABF2_8472_4AEF_B735_9A8675391BA7__INCLUDED_)
