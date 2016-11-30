#if !defined(AFX_ROOMSTATUSSETUPDLG_H__A4056D3B_9782_475A_901E_35E38A424BD6__INCLUDED_)
#define AFX_ROOMSTATUSSETUPDLG_H__A4056D3B_9782_475A_901E_35E38A424BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomStatusSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomStatusSetupDlg dialog

class CRoomStatusSetupDlg : public CNxDialog
{
// Construction
public:
	CRoomStatusSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_pStatusList;

// Dialog Data
	//{{AFX_DATA(CRoomStatusSetupDlg)
	enum { IDD = IDD_ROOM_STATUS_SETUP_DLG };
	NxButton	m_checkShowInactive;
	CNxIconButton	m_btnInactivate;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomStatusSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoomStatusSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBtnAddRoomStatus();
	afx_msg void OnBtnDeleteRoomStatus();
	afx_msg void OnBtnInactivateRoomStatus();
	afx_msg void OnBtnCloseRoomStatusSetup();
	afx_msg void OnCheckShowInactiveRoomStatuses();
	afx_msg void OnEditingFinishingRoomStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedRoomStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedRoomStatusList(short nFlags);
	afx_msg void OnEditingStartingRoomStatusList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedRoomStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMSTATUSSETUPDLG_H__A4056D3B_9782_475A_901E_35E38A424BD6__INCLUDED_)
