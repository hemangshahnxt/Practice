#if !defined(AFX_MESSAGERDLG_H__F1877B72_4C55_4438_9443_2E01C177D664__INCLUDED_)
#define AFX_MESSAGERDLG_H__F1877B72_4C55_4438_9443_2E01C177D664__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessagerDlg.h : header file
//


#include "ChatSessionDlg.h"
#include "MessageHistoryDlg.h"

// (a.walling 2012-12-04 12:25) - PLID 54023 - use NxTrayIcon.h for NXNOTIFYICONDATA
#include <NxUILib/NxTrayIcon.h>

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;

#define IDT_ICONFLASH_TIMER	1000

#define ID_MESSAGE_DELETE 32768
#define ID_MESSAGE_ADD_NOTE 32770
#define ID_USERS_SEND 32769
#define ID_MESSAGE_HISTORY 32771
#define WM_TRAY_NOTIFICATION WM_USER+0 //TS:  ID for trayicon messages
#define PREVIEW_EDIT_TEXT_LIMIT 32768 // (k.messina 2010-03-19 10:15) - PLID 4876 show previous messages
/////////////////////////////////////////////////////////////////////////////
// CMessagerDlg dialog

//Structure to hold the IM_USER_LIST packet the server sends me when I log on.
typedef struct{
	DWORD ip;
	CString strUserName;
} stUserInfo;

typedef struct{
	DWORD dwUserId;
	DWORD dwSessionId;
} Invitation;

enum ViewType{
	vtReceived = 0,
	vtSent = 1,
};

typedef enum {
	itNormal = 0,
	itDisabled = 1,
	itNewMessages = 2,
	itChatRequests = 3,
} IconType;

enum EMessagerUsersListColumns {
	mulcID,			// The user's PersonID
	mulcIsValid,	// 1 if the row is a user, 0 if it's a "section header" row
	mulcSection,	// A number the identifies which section a given row belongs in (1 for "online", 2 for "offline")
	mulcName,		// The user's name
};

enum EMessageListColumns
{
	mlcSenderID = 0,	//MessagesT.SenderID
	mlcRead = 1,		//MessagesT.Viewed
	mlcID = 2,			//MessagesT.MessageID
	mlcDate = 3,		//MessagesT.DateSent
	mlcSenderName = 4,	//UsersT.UserName of MessagesT.SenderID
	mlcMessage = 5,		//MessagesT.Message
	mlcPriority = 6,	//MessagesT.Priority, converted to string
	mlcRegardingID = 7,	//MessagesT.RegardingID
	mlcRegarding = 8,	//Name of patient for MessagesT.RegardingID
	mlcMessageGroupID = 9,//MessagesT.MessageGroupID
	mlcRecipientID = 10, //MessagesT.RecipientID
	mlcMesThreadID = 11, // (j.gruber 2010-07-16 13:58) - PLID 39463 - threadID

};

// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
class CMessagerDlg : public CNxModelessOwnedDialog
{
// Construction
public:
	CMessagerDlg(CMainFrame* pParent = NULL);   // standard constructor
	~CMessagerDlg();

	CTableChecker m_userChecker;

	NXDATALISTLib::_DNxDataListPtr m_pMessages;
	NXDATALISTLib::_DNxDataListPtr m_pUsers;

	BOOL m_bSystemClose; // (a.walling 2007-05-04 09:56) - PLID 4850 - TRUE if closing via the system rather than a user

// Dialog Data
	//{{AFX_DATA(CMessagerDlg)
	enum { IDD = IDD_MESSAGE_HOME };
	NxButton	m_btnViewSent;
	NxButton	m_btnViewReceived;
	CString	m_strDate;
	CString	m_strPriority;
	CString	m_strRegarding;
	CString	m_strSender;
	CString	m_strText;
	bool m_bEnabled;  //Are we accepting messages?
	bool m_bMoreInfo; //Is the dialog expanded?
	bool m_bLoggedOn; //Does the TCP/IP server know we're logged on?
	bool m_bNetworkEnabled; //Is the TCP/IP server running?
	CString	m_strSentTo;
	stUserInfo * m_arUsers; //Array of users associated with IP addresses.
	int m_nUsers;  //Number of users in m_arUsers.
	CNxEdit	m_nxeditSentTo;
	CNxEdit	m_nxeditDate;
	CNxEdit	m_nxeditPriority;
	CNxEdit	m_nxeditSender;
	CNxEdit	m_nxeditRegarding;
	CNxEdit	m_nxeditMessageText;
	CNxStatic	m_nxstaticMarkerSmall;
	CNxStatic	m_nxstaticMarkerLarge;
	CNxIconButton	m_btnPreviewMessage;
	CNxIconButton	m_btnClose;
	CNxStatic	m_nxstaticRegardingLabel;
	//}}AFX_DATA

	void ChangeLogonStatus();
	void InitPracYakkerTray();

	// (a.walling 2008-06-11 13:00) - PLID 30354
	NXNOTIFYICONDATA m_NID;
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Keeps the popup menu helper alive
	Modules::PopupMenuHelperPtr m_popupMenuHelper;

	/* (b.cardillo 2004-10-19 11:49) - This mutex was part of the system where we spawned a separate 
	// process meant to ensure the icon didn't get left in the sys tray on Practice crashing.  But for 
	// now we don't mess with that separate process because it seems to cause problems with other apps.
	/////
	CString m_strIconMutex;
	HANDLE m_mutIconMutex;
	*/
	void OnEnablePracYakker();
	void OnDisableYakker();
	LRESULT RefreshUserList(WPARAM wp, LPARAM lp);
	LRESULT OnMessageReceived(WPARAM wp, LPARAM lp);
	LRESULT OnMessageSent(WPARAM wp, LPARAM lp);
	LRESULT OnChatMessage(WPARAM wp, LPARAM lp);
	void NewSelection(int nRow); //Load a message into the top half, called by OnRequeryFinished and OnSelChanged.
	void LogOnTCP(); //Tell the server we're here.
	void UpdateUsers(); //Actually refresh the datalist (called only be refresh userlist).
	void PopupYakker(); //All it actually does is call OnOpenYakker, but it's for public consumption, and might do something else later.
	void RefreshColors();

	// (b.cardillo 2006-07-05 15:56) - PLID 20948 - We're back to having another public 
	// function similar to RefreshTray(), RefreshIcons() which simply clears the cached 
	// icons, reloads them, and then assigns the current notification tray icon to use 
	// the correct new one.  This is an expensive operation, so call it with care.
	void RefreshIcons();

	BOOL m_bListsRequeried;
	
	//m_pSentList is now used
	NXDATALISTLib::_DNxDataListPtr m_pSentList;

	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessagerDlg)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nCurrentView;
	bool m_bResizing;
	void ShowMessageHistory(const IN long nOtherUserID);
	CMessageHistoryDlg *m_pMessageHistoryDlg;

	//Set before requerying.
	long m_nCurrentReceivedMessage;
	long m_nCurrentSentMessage;

	//this is the current list, whether it be the received or sent messages
	NXDATALISTLib::_DNxDataListPtr m_dlCurrent;

	CMainFrame *m_pParent;
	
	//List of chat requests we haven't yet dealt with.
	CArray<Invitation, Invitation> m_arPendingRequests; 
	//All chat dialogs we have open.
	CArray<CChatSessionDlg*, CChatSessionDlg*> m_arChatDlgs;

	//Figures out which list is visible, remembers what row is selected, requeries it.
	//Should only be called by Refresh()!
	void RequeryCurrentList();

	BOOL m_bCheckForUnreadMessages;
	BOOL m_bUnreadMessages;

	//These are the only two functions to call, ever.  Refresh() calls RefreshTray(), but you
	//can call RefreshTray() independently if necessary.
	//TES 5/3/2007 - PLID 18516 - This function always used to auto-select the oldest unread message, but there are
	// many situations where one would want to refresh the screen without also changing the selection.  So I added a
	// parameter, bSelectNewMessage, to control whether or not it will make that attempt.
	void Refresh(bool bSelectNewMessage);

	//TES 5/3/2007 - PLID 18516  - This function calls Refresh(), and therefore also needs a bSelectNewMessage parameter.
	void OpenYakker(bool bSelectNewMessage);
	//TES 12/23/2008 - PLID 32525 - Added an overload that doesn't take a bSelectNewMessage and calculates it itself.
	// This is exactly what the OnOpenYakker message handler used to do, but it was being called directly, which it
	// shouldn't have been; now those places will just call this overload.
	void OpenYakker();

	//TES 5/3/2007 - PLID 25895 - Updates the caption to reflect the logged-in user, and the number of unread messages.
	void RefreshCaption();

	// (k.messina 2010-03-18 14:23) - PLID 4876 Get the other user's personID
	long GetOtherUserID();

//public://TES 2004-02-06: I will also permit mainframe to call RefreshTray(), in case the preferences change.
protected://TES 2004-11-05: Actually, let's just use TableCheckers for that like we should.
	void RefreshTray();
protected:

	HICON GetIcon(IconType it);

	HICON m_hNormalIcon, m_hFlashingIcon, m_hDisabledIcon, m_hChatIcon;
	BOOL m_bCanDestroyNormal, m_bCanDestroyFlashing, m_bCanDestroyDisabled, m_bCanDestroyChat;

	void HandleRButtonUp(long nRow, short nCol, long x, long y, long nFlags);
	
	bool m_bWhichFlash;
	bool m_bProcessingRequests;

	void PlayMessageReceivedSound();

	// (z.manning, 06/20/2007) - PLID 26390 - These function and member variable are for special
	// handling for hotkeys for the PracYakker dialog.
	BOOL m_bMessagerDlgHasDisabledHotkeys;
	void MessagerDlgDisableHotkeys();
	void MessagerDlgEnableHotkeys();

	// Generated message map functions
	//{{AFX_MSG(CMessagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingMessageList(long FAR* nNewSel);
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnReply();
	afx_msg void OnReplyAll();
	afx_msg void OnGotoPatient();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeleteMessage();
	afx_msg void OnAddNote();
	afx_msg void OnRButtonDownUsers(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSendMessage();
	afx_msg void OnRequeryFinishedMessageList(short nFlags);
	afx_msg LRESULT OnYakTrayNotify(WPARAM wp, LPARAM lp);
	afx_msg void OnOpenYakker();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNewMessage();
	afx_msg void OnMoreInfo();
	afx_msg void OnSelChangedMessageList(long nNewSel);
	afx_msg void OnForward();
	afx_msg void OnEditingFinishedMessageList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelChangingUsers(long FAR* nNewSel);
	afx_msg void OnClose();
	afx_msg void OnSelectTabMessageTab(short newTab, short oldTab);
	afx_msg void OnRequestChat();
	afx_msg void OnChat();
	afx_msg void OnRButtonUpMessageList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnViewSent();
	afx_msg void OnViewReceived();
	afx_msg void OnShowMessageHistory();
	afx_msg void OnShowMessageHistoryBtn();
	afx_msg void OnSelChangedSentList(long nNewSel);
	afx_msg void OnRButtonUpSentList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg LRESULT OnGotoMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnForwardMessage();
	afx_msg void OnDblClickCellUsers(long nRowIndex, short nColIndex);
	afx_msg void OnRequeryFinishedSentList(short nFlags);
	afx_msg LRESULT OnNxServerConnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNxServerDisconnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedUsers(short nFlags);
	afx_msg void OnYakkerPrefs();
	afx_msg void OnPreviewMessage();
	afx_msg void OnTimer(UINT nTimerID);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGERDLG_H__F1877B72_4C55_4438_9443_2E01C177D664__INCLUDED_)
