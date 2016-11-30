#if !defined(AFX_SENDMESSAGEDLG_H__F8DE72D3_3A6C_11D5_B877_00C04F4C8415__INCLUDED_)
#define AFX_SENDMESSAGEDLG_H__F8DE72D3_3A6C_11D5_B877_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendMessageDlg.h : header file
//
#include "SelectRecipientsDlg.h"
#include "MessageEdit.h"
#include "YakGroupSelect.h"
/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog

#define MULTIPLE_USERS	" { Multiple Users } "
#define MULTIPLE_GROUPS	" { Multiple Groups } "

class CMessagerDlg;

// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
class CSendMessageDlg : public CNxModelessOwnedDialog
{
// Construction
public:
	CSendMessageDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendMessageDlg)
	enum { IDD = IDD_SEND_MESSAGE_DLG };
	NxButton	m_btnRegarding;
	NxButton	m_btnSendTo;
	NxButton	m_btnSendToGroup;
	CNxLabel	m_nxlGroupsLabel;
	CNxLabel	m_nxlUserLabel;
	CMessageEdit	m_ebMessage;
	CComboBox	m_cbPriority;
	CDWordArray m_adwRecipients; //Who is this being sent to?
	CStringArray m_astrRecipients; //Who is this being sent to?
	int m_nRegardingID; //Who is this about?
	CString	m_strText;
	//CSelectRecipientsDlg m_dlgSelectRecipients; // this was not used
	CMessagerDlg * m_pMessageHome;
	CNxIconButton	m_btnSend;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_pUsers;
	NXDATALISTLib::_DNxDataListPtr m_pPatients;
	NXDATALISTLib::_DNxDataListPtr m_pGroups;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMessageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// (c.haag 2006-05-16 13:29) - PLID 20621 - This is now a modeless window,
	// so we need these functions to keep it as up to date as it was back when
	// it was modal
	
	// (j.jones 2008-06-02 12:04) - PLID 30219 - added ability to send in a
	// default group ID
	// (a.walling 2008-06-11 11:35) - PLID 22049 - added ability to set priority
	// (j.gruber 2010-07-16 13:19) - PLID 39463 - added ThreadID
	void PopUpMessage(long nThreadID = -1,
		long nRegardingID = -1, 		
		CDWordArray* padwRecipients = NULL,
		CStringArray* pastrRecipients = NULL,
		CString strDefaultText = "",
		long nDefaultGroupID = -1,
		CString strPriority = "Medium");

	void RefreshColors();
	void HandleTableChange(long nPatientID);

protected:
	CDWordArray m_dwRecipientsAry;
	CStringArray m_strRecipientsAry;
	CDWordArray m_dwGroupRecipAry;
	CStringArray m_strGroupRecipAry;
	CSelectRecipientsDlg dlgSelectRecipients;
	CYakGroupSelect dlgGroupSelect;
	void DrawUserHyperlinkList(CDC *pdc);
	BOOL IsCurUserListMultiUser();
	BOOL IsCurGroupListMultiGroup();
	CString GenerateUsersString();
	CString GenerateGroupsString();

	// (j.gruber 2010-07-16 13:21) - PLID 39463 - threads
	long m_nThreadID;

	void RefreshUserCombo();
	void RefreshGroupsCombo();
	void RefreshRegardingList(); // (a.walling 2011-07-29 13:15) - PLID 44788 - Refresh / reload the regarding list
	void CALLBACK OnCtrlEnter();
	void FillRecipAryWithGroupInfo();

	// (z.manning, 06/20/2007) - PLID 26390 - These function and member variable are for special
	// handling for hotkeys for the PracYakker dialog.
	BOOL m_bSendMessageDlgHasDisabledHotkeys;
	void SendMessageDlgDisableHotkeys();
	void SendMessageDlgEnableHotkeys();

	// Generated message map functions
	//{{AFX_MSG(CSendMessageDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSendTo();
	afx_msg void OnRegarding();
	virtual void OnOK();
	afx_msg void OnSendGroup();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditGroups();
	afx_msg void OnSelChosenComboGroups(long nRow);
	afx_msg void OnSelChosenComboUsers(long nRow);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelChosenComboPatients(long nRow);
	afx_msg void OnDroppingDownPatients();
	afx_msg void OnRequeryFinishedComboUsers(short nFlags);
	afx_msg void OnRequeryFinishedComboGroups(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMESSAGEDLG_H__F8DE72D3_3A6C_11D5_B877_00C04F4C8415__INCLUDED_)
