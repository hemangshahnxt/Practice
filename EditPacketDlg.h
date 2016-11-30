#if !defined(AFX_EDITPACKETDLG_H__68E63F64_7752_4C3C_962E_E6CA199E2158__INCLUDED_)
#define AFX_EDITPACKETDLG_H__68E63F64_7752_4C3C_962E_E6CA199E2158__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditPacketDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditPacketDlg dialog

class CEditPacketDlg : public CNxDialog
{
// Construction
public:
	CEditPacketDlg(CWnd* pParent);   // standard constructor
	long m_nPacketID;
	long m_nNextSel;

	// (z.manning, 04/25/2008) - PLID 29795 - Added more NxIconButtons
// Dialog Data
	//{{AFX_DATA(CEditPacketDlg)
	enum { IDD = IDD_EDIT_PACKET_DLG };
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnDone;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPacketDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pComponentList;

	// Generated message map functions
	//{{AFX_MSG(CEditPacketDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnComponentUp();
	afx_msg void OnRequeryFinishedComponentList(short nFlags);
	afx_msg void OnComponentDown();
	afx_msg void OnEditingFinishedComponentList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnClose();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPACKETDLG_H__68E63F64_7752_4C3C_962E_E6CA199E2158__INCLUDED_)
