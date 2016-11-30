#if !defined(AFX_CHATSESSIONDLG_H__A4BF6347_62DF_439F_B6DB_2FBD92595D08__INCLUDED_)
#define AFX_CHATSESSIONDLG_H__A4BF6347_62DF_439F_B6DB_2FBD92595D08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatSessionDlg.h : header file
//
class CMessagerDlg;
#include "MessageEdit.h"

typedef enum {
	cmtMessage = 0,
	cmtInvite = 1,
	cmtConnect = 2,
	cmtDisconnect = 3,
	cmtPing = 4,
	cmtPong = 5,
} ChatMessageType;

typedef struct {
	long nUserId;
	DWORD dwIpAddress;
	CString strUsername;
	COLORREF Color;
} ChatUser;

/////////////////////////////////////////////////////////////////////////////
// CChatSessionDlg dialog

class CChatSessionDlg : public CNxDialog
{
// Construction
public:
	CChatSessionDlg(CMessagerDlg* pMessagerDlg);   // standard constructor

	long m_nSessionId;

// Dialog Data
	//{{AFX_DATA(CChatSessionDlg)
	enum { IDD = IDD_CHAT_SESSION_DLG };
	CMessageEdit	m_ebMessage;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatSessionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pChatWindow, m_pUsersInSession;

	//Returns the m_arChatUsers entry corresponding to what you pass in, and performs all necessary
	//maintenance on m_arChatUsers.
	ChatUser GetUser(long nUserID, DWORD dwIpAddress = 0);

	CArray<ChatUser, ChatUser> m_arChatUsers;

	//Initializes a new chat session.
	long CreateNewSession();

	//Loads an existing chat session.
	void LoadSession();

	//MessagerDlg maintains a list of online users, which we would like to reference.
	CMessagerDlg* m_pMessagerDlg;

	// (a.walling 2006-09-11 17:53) - PLID 20980 - Flash the YakChat window
	void Flash(); //Flash the YakChat window

	LRESULT OnChatMessage(WPARAM wp, LPARAM lp);
	LRESULT OnChatClose(WPARAM wParam, LPARAM lParam);

	void OnMessageMessage(WPARAM wp, LPARAM lp);
	void OnMessageInvite(WPARAM wp, LPARAM lp);
	void OnMessageConnect(WPARAM wp, LPARAM lp);
	void OnMessageDisconnect(WPARAM wp, LPARAM lp);
	void OnMessagePing(WPARAM wp, LPARAM lp);
	void OnMessagePong(WPARAM wp, LPARAM lp);

	//All users that should show in our right-click menu.
	CArray<ChatUser, ChatUser> m_arInviteUsers;

	//Refresh above array, basically by subtracting our list of online users from m_pMessagerDlg's.
	void RefreshInviteMenu();

	//Handler for the invite menu.
	void OnInviteUser(int nMenuPos);

	//Removes the given user from the session, displaying strMessage to the other users.
	void RemoveUser(DWORD dwIpAddress, long nUserID, const CString &strMessage);

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	//Used to keep track of whether we need to enable/disable hotkeys in mainframe.
	bool m_bHotkeysEnabled;

	// Generated message map functions
	//{{AFX_MSG(CChatSessionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSend();
	afx_msg void OnInvite();
	afx_msg LRESULT OnCtrlEnter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg LRESULT OnRefreshUserList(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATSESSIONDLG_H__A4BF6347_62DF_439F_B6DB_2FBD92595D08__INCLUDED_)
