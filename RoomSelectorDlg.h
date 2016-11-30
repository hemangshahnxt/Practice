#if !defined(AFX_ROOMSELECTORDLG_H__7F5F156F_A2CC_44A6_BF7E_2E08ACCEB581__INCLUDED_)
#define AFX_ROOMSELECTORDLG_H__7F5F156F_A2CC_44A6_BF7E_2E08ACCEB581__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomSelectorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomSelectorDlg dialog

class CRoomSelectorDlg : public CNxDialog
{
// Construction
public:
	CRoomSelectorDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_pRoomCombo;

	long m_nAppointmentID;
	long m_nPatientID;
	long m_nRoomID;

	// (j.jones 2010-11-24 14:59) - PLID 38597 - added ability to select a waiting room,
	// regular room, or both
	BOOL m_bShowWaitingRooms;
	BOOL m_bShowRegularRooms;

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CRoomSelectorDlg)
	enum { IDD = IDD_ROOM_SELECTOR_DLG };
	CNxStatic	m_nxstaticApptLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomSelectorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoomSelectorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenRoomSelectionCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedRoomSelectionCombo(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMSELECTORDLG_H__7F5F156F_A2CC_44A6_BF7E_2E08ACCEB581__INCLUDED_)
