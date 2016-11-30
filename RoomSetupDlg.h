#if !defined(AFX_ROOMSETUPDLG_H__45493072_A508_4DA8_AA6A_979A29243DBB__INCLUDED_)
#define AFX_ROOMSETUPDLG_H__45493072_A508_4DA8_AA6A_979A29243DBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomSetupDlg dialog

class CRoomSetupDlg : public CNxDialog
{
// Construction
public:
	CRoomSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_pRoomList;

// Dialog Data
	//{{AFX_DATA(CRoomSetupDlg)
	enum { IDD = IDD_ROOM_SETUP_DLG };
	NxButton	m_checkShowInactive;
	CNxIconButton	m_btnInactivate;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoomSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBtnAddRoom();
	afx_msg void OnBtnDeleteRoom();
	afx_msg void OnBtnInactivateRoom();
	afx_msg void OnBtnCloseRoomSetup();
	afx_msg void OnEditingFinishingRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnCheckShowInactiveRooms();
	afx_msg void OnEditingStartingRoomList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedRoomList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMSETUPDLG_H__45493072_A508_4DA8_AA6A_979A29243DBB__INCLUDED_)
