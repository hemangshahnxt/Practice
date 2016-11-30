// ChatSessionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MessagerDlg.h"
#include "ChatSessionDlg.h"
#include "GlobalDataUtils.h"
#include "NxPackets.h"


//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_USERS			64	//Totally, totally arbitrary
#define IDM_INVITE_USERS	(WM_APP+1)

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CChatSessionDlg dialog
using namespace ADODB;

CChatSessionDlg::CChatSessionDlg(CMessagerDlg* pMessagerDlg)
	: CNxDialog(CChatSessionDlg::IDD, NULL),
	m_ebMessage(NXM_CTRL_ENTER)
{
	m_pMessagerDlg = pMessagerDlg;
	m_nSessionId = -1;
	m_bHotkeysEnabled = true;
	//{{AFX_DATA_INIT(CChatSessionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChatSessionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatSessionDlg)
	DDX_Control(pDX, IDC_MESSAGE, m_ebMessage);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatSessionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChatSessionDlg)
	ON_MESSAGE(NXM_CHAT_MESSAGE, OnChatMessage)
	ON_MESSAGE(NXM_CHAT_CLOSE, OnChatClose)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_MESSAGE(NXM_CTRL_ENTER, OnCtrlEnter)
	ON_BN_CLICKED(IDC_INVITE, OnInvite)
	ON_WM_CLOSE()
	ON_MESSAGE(NXM_REFRESH_USER_LIST, OnRefreshUserList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatSessionDlg message handlers

BOOL CChatSessionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_pChatWindow = BindNxDataListCtrl(IDC_CHAT_WINDOW, false);

	m_pUsersInSession = BindNxDataListCtrl(IDC_USERS_IN_SESSION, false);

	if(m_nSessionId == -1) {
		m_nSessionId = CreateNewSession();
	}
	else {
		LoadSession();
	}

	// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

COLORREF GetColor(int nUserPos) {
	//We've only made 7 colors so far, if more than 7 users are in session, we'll have to start recycling.
	nUserPos = nUserPos % 7;
	switch(nUserPos) {
	case 0:
		return RGB(127,0,0); //Red
		break;
	case 1:
		return RGB(0,0,127); //Blue
		break;
	case 2:
		return RGB(0,127,0); //Green
		break;
	case 3:
		return RGB(127,127,0); //Yellow
		break;
	case 4:
		return RGB(0,127,127); //Cyan
		break;
		//NOTE: I'm refusing to return magenta, because I find it very unpleasant.
	case 5:
		return RGB(0,0,0); //Black
		break;
	case 6:
		return RGB(127,127,127); //Gray
		break;
	default:
		//This is a mathematical impossibility!
		ASSERT(FALSE);
		return RGB(0,0,0);
	}
}
long CChatSessionDlg::CreateNewSession()
{
	try {
		long nNewSessionId = NewNumber("ChatMessagesT", "SessionId");
		ExecuteSql("INSERT INTO ChatMessagesT (ID, SessionID, MessageType, Text, DateSent, SenderID) "
			"VALUES (%li, %li, %li, '***Entered Session***', getdate(), %li)", NewNumber("ChatMessagesT", "ID"), 
			nNewSessionId, cmtConnect, GetCurrentUserID());

		//Send a message to the network saying that we've logged in; we will receive that message, 
		//and then we will put ourselves in the list of logged-in users.
		_CHAT_MESSAGE msg;
		msg.dwSenderId = GetCurrentUserID();
		msg.dwSessionId = nNewSessionId;
		msg.type = cmtConnect;
		//msg.ipSource will be filled in by nxserver.
		CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &msg, sizeof(_CHAT_MESSAGE));
		return nNewSessionId;
	}NxCatchAll("Error in CChatSessionDlg::CreateNewSession()");
	//We are now in an error state.  Abandon ship!
	CDialog::OnOK();
	return -1;
}

void CChatSessionDlg::LoadSession()
{
	try {
		//Load all messages that have ever been sent in this session, and fill them in the list.
		_RecordsetPtr rsChatMessages = CreateRecordset("SELECT ChatMessagesT.SenderID, "
			"ChatMessagesT.MessageType, ChatMessagesT.Text FROM ChatMessagesT "
			"WHERE SessionID = %li AND MessageType IN (%li, %li, %li) ORDER BY DateSent ASC", m_nSessionId, cmtMessage, cmtConnect, cmtDisconnect);
		FieldsPtr fChatMessages = rsChatMessages->Fields;
		while(!rsChatMessages->eof) {
			IRowSettingsPtr pRow = m_pChatWindow->GetRow(-1);
			ChatUser cu = GetUser(AdoFldLong(fChatMessages, "SenderID"), 0);
			pRow->PutValue(0, (long)cu.dwIpAddress);
			pRow->PutValue(1, cu.nUserId);
			pRow->PutValue(2, _bstr_t(cu.strUsername));
			pRow->PutCellForeColor(2, cu.Color);
			pRow->PutValue(3, fChatMessages->GetItem("Text")->Value);
			m_pChatWindow->AddRow(pRow);

			rsChatMessages->MoveNext();
		}
		//Set the sel to the last row (the highlight won't show, but it will scroll to the bottom.
		m_pChatWindow->CurSel = m_pChatWindow->GetRowCount()-1;

		//Send a message to the network saying that we've logged in; we will receive that message, 
		//and then we will put ourselves in the list of logged-in users.
		_CHAT_MESSAGE msg;
		msg.dwSenderId = GetCurrentUserID();
		msg.dwSessionId = m_nSessionId;
		msg.type = cmtConnect;
		//msg.ipSource will be filled in by nxserver.
		CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &msg, sizeof(_CHAT_MESSAGE));

		//Now, find out who is in this session.
		_CHAT_MESSAGE cm;
		cm.dwSessionId = m_nSessionId;
		cm.dwSenderId = GetCurrentUserID();
		cm.type = cmtPing;
		CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &cm, sizeof(_CHAT_MESSAGE));
	}NxCatchAll("Error in CChatSessionDlg::LoadSession()");
}

void CChatSessionDlg::OnOK() 
{
	try {
		//Notify everyone that we're leaving.
		if(m_nSessionId != -1) {
			ExecuteSql("INSERT INTO ChatMessagesT (ID, SessionId, MessageType, Text, DateSent, SenderID) "
				"VALUES (%li, %li, %li, '***Left Session***', getdate(), %li)", NewNumber("ChatMessagesT", "ID"), 
				m_nSessionId, cmtDisconnect, GetCurrentUserID());

			_CHAT_MESSAGE cm;
			cm.dwSessionId = m_nSessionId;
			cm.dwSenderId = GetCurrentUserID();
			cm.type = cmtDisconnect;
			CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &cm, sizeof(_CHAT_MESSAGE));
		}
	}NxCatchAll("Error in CChatSessionDlg::OnOK()");

	//PLID 9490 - we need to re-enable hotkeys!
	if(!m_bHotkeysEnabled) {
		GetMainFrame()->EnableHotKeys();
		m_bHotkeysEnabled = true;
	}

	CDialog::OnOK();
}

void CChatSessionDlg::OnCancel() 
{
	//DRT 12/2/2003 - PLID 10286 - We need to do all the usual dismissing stuff.  It doesn't matter if the 
	//final result is cancel or OK.
	OnOK();
}

ChatUser CChatSessionDlg::GetUser(long nUserId, DWORD dwIpAddress /*= 0*/)
{
	try {
		for(int i = 0; i < m_arChatUsers.GetSize(); i++) {
			if(m_arChatUsers.GetAt(i).nUserId == nUserId) {
				if(m_arChatUsers.GetAt(i).dwIpAddress == dwIpAddress || dwIpAddress == 0) {
					return m_arChatUsers.GetAt(i);
				}
				else if(m_arChatUsers.GetAt(i).dwIpAddress == 0) {
					//This user is in the list, but never had an ip address filled in.  Let's fill it in now.
					ChatUser cu;
					cu.Color = m_arChatUsers.GetAt(i).Color;
					cu.nUserId = m_arChatUsers.GetAt(i).nUserId;
					cu.strUsername = m_arChatUsers.GetAt(i).strUsername;
					cu.dwIpAddress = dwIpAddress;
					m_arChatUsers.SetAt(i, cu);
					return cu;
				}
			}
		}
		
		//It wasn't already in the array, so add it.
		ChatUser cu;
		cu.Color = GetColor(m_arChatUsers.GetSize());
		cu.dwIpAddress = dwIpAddress;
		cu.nUserId = nUserId;
		_RecordsetPtr rsUser = CreateRecordset("SELECT Username FROM UsersT WHERE PersonID = %li", nUserId);
		cu.strUsername = AdoFldString(rsUser, "Username");
		m_arChatUsers.Add(cu);
		return cu;
	}NxCatchAll("Error in CChatSessionDlg::GetUser");
	ChatUser cu;
	return cu;
}
LRESULT CChatSessionDlg::OnChatMessage(WPARAM wp, LPARAM lp)
{
	switch( ((_CHAT_MESSAGE*)lp)->type) {
	case cmtMessage:
		OnMessageMessage(wp, lp);
		break;
	case cmtInvite:
		OnMessageInvite(wp, lp);
		break;
	case cmtConnect:
		OnMessageConnect(wp, lp);
		break;
	case cmtDisconnect:
		OnMessageDisconnect(wp, lp);
		break;
	case cmtPing:
		OnMessagePing(wp, lp);
		break;
	case cmtPong:
		OnMessagePong(wp, lp);
		break;
	default:
		//Invalid message!
		ASSERT(FALSE);
		break;
	}
	return 0;
}

void CChatSessionDlg::OnMessageMessage(WPARAM wp, LPARAM lp)
{
	try {
		//Add this message to our screen.
		IRowSettingsPtr pRow = m_pChatWindow->GetRow(-1);
		ChatUser cu = GetUser((long)((_CHAT_MESSAGE*)lp)->dwSenderId, ((_CHAT_MESSAGE*)lp)->ipSource);

		pRow->PutValue(0, (long)cu.dwIpAddress);
		pRow->PutValue(1, cu.nUserId);
		pRow->PutValue(2, _bstr_t(cu.strUsername));
		pRow->PutCellForeColor(2, cu.Color);
		_RecordsetPtr rsText = CreateRecordset("SELECT Text FROM ChatMessagesT WHERE ID = %u", ((_CHAT_MESSAGE*)lp)->dwExtraId);
		pRow->PutValue(3, rsText->Fields->GetItem("Text")->Value);
		m_pChatWindow->AddRow(pRow);
		//Set the sel to the last row (the highlight won't show, but it will scroll to the bottom.
		m_pChatWindow->CurSel = m_pChatWindow->GetRowCount()-1;
		
		// (a.walling 2006-09-11 17:53) - PLID 20980 - Flash the YakChat window
		Flash();
	}NxCatchAll("Error in CChatSessionDlg::OnMessageMessage()");
}

void CChatSessionDlg::OnMessageInvite(WPARAM wp, LPARAM lp)
{
	//We don't need to do anything; messagerdlg handles these for us.
}

void CChatSessionDlg::OnMessageConnect(WPARAM wp, LPARAM lp)
{
	try {
		//Add this person to our list of in-session users.

		//First, see if they're in our list of users (may include offline users).
		ChatUser cu = GetUser((long)((_CHAT_MESSAGE*)lp)->dwSenderId, ((_CHAT_MESSAGE*)lp)->ipSource);

		//Now, see if they're in our list of online users (the datalist).
		bool bUserFound = false;
		long p = m_pUsersInSession->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		
		while (p && !bUserFound)
		{
			m_pUsersInSession->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			pDisp->Release();
			
			if((DWORD)VarLong(pRow->GetValue(0)) == ((_CHAT_MESSAGE*)lp)->dwSenderId && 
				(DWORD)VarLong(pRow->GetValue(1)) == ((_CHAT_MESSAGE*)lp)->ipSource) {
				bUserFound = true;
			}			
		}

		if(!bUserFound) {
			//Add it.
			IRowSettingsPtr pRow = m_pUsersInSession->GetRow(-1);
			pRow->PutValue(0, (long)((_CHAT_MESSAGE*)lp)->dwSenderId);
			pRow->PutValue(1, (long)((_CHAT_MESSAGE*)lp)->ipSource);
			pRow->PutForeColor(cu.Color);
			pRow->PutValue(2, _bstr_t(cu.strUsername));

			m_pUsersInSession->AddRow(pRow);
		}

		//Finally, add a line to the message window indicating that this user entered.
		IRowSettingsPtr pRow = m_pChatWindow->GetRow(-1);
		pRow->PutValue(0, (long)((_CHAT_MESSAGE*)lp)->ipSource);
		pRow->PutValue(1, (long)((_CHAT_MESSAGE*)lp)->dwSenderId);
		pRow->PutValue(2, _bstr_t(cu.strUsername));
		pRow->PutCellForeColor(2, cu.Color);
		pRow->PutValue(3, _bstr_t("***Entered Session***"));
		m_pChatWindow->AddRow(pRow);
		//Set the sel to the last row (the highlight won't show, but it will scroll to the bottom.
		m_pChatWindow->CurSel = m_pChatWindow->GetRowCount()-1;

		// (a.walling 2006-09-11 17:53) - PLID 20980 - Flash the YakChat window
		Flash();
	}NxCatchAll("Error in CChatSessionDlg::OnMessageConnect()");
}

void CChatSessionDlg::OnMessageDisconnect(WPARAM wp, LPARAM lp)
{
	try {
		//Take them out of our list of online users.
		long p = m_pUsersInSession->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		bool bRowRemoved = false;
		int nCurrentRow = 0;//This is valid, because Bob tells me so.
		while (p && !bRowRemoved)
		{
			m_pUsersInSession->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			pDisp->Release();
			
			if((DWORD)VarLong(pRow->GetValue(0)) == ((_CHAT_MESSAGE*)lp)->dwSenderId && 
				(DWORD)VarLong(pRow->GetValue(1)) == ((_CHAT_MESSAGE*)lp)->ipSource) {
				RemoveUser(((_CHAT_MESSAGE*)lp)->ipSource, ((_CHAT_MESSAGE*)lp)->dwSenderId, "***Left Session***");
			}
			nCurrentRow++;
		}

		RefreshInviteMenu();

		// (a.walling 2006-09-11 17:53) - PLID 20980 - Flash the YakChat window
		Flash();
	}NxCatchAll("Error in CChatSessionDlg::OnMessageDisconnect()");
}

void CChatSessionDlg::OnMessagePing(WPARAM wp, LPARAM lp)
{
	try {
		//Is this for our session?
		if(((_CHAT_MESSAGE*)lp)->dwSessionId == (DWORD)m_nSessionId) {
			//Pong.
			_CHAT_MESSAGE cm;
			cm.dwDestIP = NULL;
			cm.dwExtraId = NULL;
			cm.dwSessionId = m_nSessionId;
			cm.type = cmtPong;
			cm.dwSenderId = GetCurrentUserID();
			CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &cm, sizeof(_CHAT_MESSAGE));
		}
	}NxCatchAll("Error in CChatSessionDlg::OnMessagePing()");
}

void CChatSessionDlg::OnMessagePong(WPARAM wp, LPARAM lp)
{
	try {
		//Is this for our session?
		if(((_CHAT_MESSAGE*)lp)->dwSessionId == (DWORD)m_nSessionId) {
			//Make sure this person is in our list of online users.
			ChatUser cu = GetUser((long)((_CHAT_MESSAGE*)lp)->dwSenderId, ((_CHAT_MESSAGE*)lp)->ipSource);
			bool bUserFound = false;
			long p = m_pUsersInSession->GetFirstRowEnum();
			LPDISPATCH pDisp = NULL;
			
			while (p && !bUserFound)
			{
				m_pUsersInSession->GetNextRowEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
				
				if((DWORD)VarLong(pRow->GetValue(0)) == ((_CHAT_MESSAGE*)lp)->dwSenderId && 
					(DWORD)VarLong(pRow->GetValue(1)) == ((_CHAT_MESSAGE*)lp)->ipSource) {
					bUserFound = true;
				}
			}

			if(!bUserFound) {
				//Add it.
				IRowSettingsPtr pRow = m_pUsersInSession->GetRow(-1);
				pRow->PutValue(0, (long)((_CHAT_MESSAGE*)lp)->dwSenderId);
				pRow->PutValue(1, (long)((_CHAT_MESSAGE*)lp)->ipSource);
				pRow->PutForeColor(cu.Color);
				pRow->PutValue(2, _bstr_t(cu.strUsername));

				m_pUsersInSession->AddRow(pRow);
			}
		}
	}NxCatchAll("Error in CChatSessionDlg::OnMessagePong()");
}

LRESULT CChatSessionDlg::OnChatClose(WPARAM wParam, LPARAM lParam)
{
	OnOK();
	return 0;
}

void CChatSessionDlg::OnSend() 
{
	try {
		CString strMessage;
		GetDlgItemText(IDC_MESSAGE, strMessage);
		strMessage.TrimRight();
		if(strMessage.GetLength() > 0) {
			//Put it in the database.
			long nMessageId = NewNumber("ChatMessagesT", "ID");
			ExecuteSql("INSERT INTO ChatMessagesT (ID, SessionID, MessageType, Text, DateSent, SenderID) "
				"VALUES (%li, %li, %li, '%s', getdate(), %li)", nMessageId, m_nSessionId, 
				cmtMessage, _Q(strMessage), GetCurrentUserID());

			//Tell the world!
			_CHAT_MESSAGE cm;
			cm.dwExtraId = (DWORD)nMessageId;
			cm.dwSessionId = m_nSessionId;
			cm.dwSenderId = GetCurrentUserID();
			cm.type = cmtMessage;
			CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &cm, sizeof(_CHAT_MESSAGE));

			//Clear out the edit box.
			SetDlgItemText(IDC_MESSAGE, "");
			GetDlgItem(IDC_MESSAGE)->SetFocus();
		}
	}NxCatchAll("Error in CChatSessionDlg::OnSend()");
}

void CChatSessionDlg::RefreshInviteMenu()
{
	try {
		m_arInviteUsers.RemoveAll();
		for(int i = 0; i < m_pMessagerDlg->m_nUsers; i++) {
			//Is this fellow in our list of online users?
			bool bUserFound = false;
			long p = m_pUsersInSession->GetFirstRowEnum();
			LPDISPATCH pDisp = NULL;
			
			while (p && !bUserFound)
			{
				m_pUsersInSession->GetNextRowEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
				
				if(VarString(pRow->GetValue(2)) == m_pMessagerDlg->m_arUsers[i].strUserName &&
					(DWORD)VarLong(pRow->GetValue(1)) == m_pMessagerDlg->m_arUsers[i].ip) {
					bUserFound = true;
				}
			}
			if(!bUserFound) {
				//Nope, we should invite them.
				ChatUser cu;
				cu.strUsername = m_pMessagerDlg->m_arUsers[i].strUserName;
				cu.dwIpAddress = m_pMessagerDlg->m_arUsers[i].ip;
				m_arInviteUsers.Add(cu);
			}
		}
	}NxCatchAll("Error in CChatSessionDlg::RefreshInviteMenu()");
}

void CChatSessionDlg::OnInvite()
{
	try {
		RefreshInviteMenu();
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		
		if(m_arInviteUsers.GetSize() == 0) {
			mnu.InsertMenu(0, MF_BYPOSITION|MF_GRAYED, IDM_INVITE_USERS, "<None>");
		}
		else {
			for(int i = 0; i < m_arInviteUsers.GetSize(); i++) {
				mnu.InsertMenu(i, MF_BYPOSITION, IDM_INVITE_USERS+i, m_arInviteUsers.GetAt(i).strUsername);
			}
		}

		//Show the menu.
		CRect rc;
		GetDlgItem(IDC_INVITE)->GetWindowRect(rc);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
	}NxCatchAll("Error in CChatSessionDlg::OnInvite()");
}

LRESULT CChatSessionDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		// See if the command was one of the "resource view" menus
		if (wParam >= IDM_INVITE_USERS && wParam <= (IDM_INVITE_USERS + MAX_USERS-1)) {
			// Call our handler for this
			OnInviteUser(wParam-IDM_INVITE_USERS);
			return 0;
		}
		break;
	case WM_ACTIVATE:
		switch(LOWORD(wParam)) {
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				//These calls to mainframe are nested, so we need to disable the hotkeys 
				//exactly once, and enable them exactly once.
				if(m_bHotkeysEnabled) {
					GetMainFrame()->DisableHotKeys();
					m_bHotkeysEnabled = false;
				}
				break;
			case WA_INACTIVE:
				if(!m_bHotkeysEnabled) {
					GetMainFrame()->EnableHotKeys();
					m_bHotkeysEnabled = true;
				}
				break;
		}
		break;
	}
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CChatSessionDlg::OnInviteUser(int nMenuPos)
{
	try {
		CString strUsername = m_arInviteUsers.GetAt(nMenuPos).strUsername;
		
		_CHAT_MESSAGE cm;
		cm.dwDestIP = m_arInviteUsers.GetAt(nMenuPos).dwIpAddress;
		_RecordsetPtr rsUserId = CreateRecordset("SELECT PersonID FROM UsersT WHERE Username = '%s'", _Q(strUsername));
		cm.dwExtraId = (DWORD)AdoFldLong(rsUserId, "PersonID");
		cm.dwSenderId = GetCurrentUserID();
		cm.dwSessionId = m_nSessionId;
		cm.type = cmtInvite;
		CClient::Send(PACKET_TYPE_CHAT_MESSAGE, &cm, sizeof(_CHAT_MESSAGE));
		
	}NxCatchAll("Error in CChatSessionDlg::OnInviteUser()");
}

LRESULT CChatSessionDlg::OnCtrlEnter(WPARAM wParam, LPARAM lParam) 
{
	OnSend();
	return 0;
}

void CChatSessionDlg::OnClose()
{
	OnOK();
}

LRESULT CChatSessionDlg::OnRefreshUserList(WPARAM wParam, LPARAM lParam)
{
	try {
		//We want to go through each of the users in our session, and make sure they're all still online.
		for(int i = 0; i < m_arChatUsers.GetSize(); i++) {
			bool bOnline = false;
			for(int j = 0; j < m_pMessagerDlg->m_nUsers && !bOnline; j++) {
				if(m_arChatUsers.GetAt(i).dwIpAddress == m_pMessagerDlg->m_arUsers[j].ip &&
					m_arChatUsers.GetAt(i).strUsername == m_pMessagerDlg->m_arUsers[j].strUserName) {
					bOnline = true;
				}
			}
			if(!bOnline) {
				RemoveUser(m_arChatUsers.GetAt(i).dwIpAddress, m_arChatUsers.GetAt(i).nUserId, "***Disconnected***");
			}
		}
		RefreshInviteMenu();
	}NxCatchAll("Error in CChatSessionDlg::OnRefreshUserList()");
	return TRUE;
}

void CChatSessionDlg::RemoveUser(DWORD dwIpAddress, long nUserID, const CString &strMessage)
{
	//Take them out of our list of online users.
	long p = m_pUsersInSession->GetFirstRowEnum();
	LPDISPATCH pDisp = NULL;
	bool bRowRemoved = false;
	int nCurrentRow = 0;//This is valid, because Bob tells me so.
	while (p && !bRowRemoved)
	{
		m_pUsersInSession->GetNextRowEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		pDisp->Release();
		
		if(VarLong(pRow->GetValue(0)) == nUserID && 
			(DWORD)VarLong(pRow->GetValue(1)) == dwIpAddress) {
			m_pUsersInSession->RemoveRow(nCurrentRow);
			bRowRemoved = true;
		}
		nCurrentRow++;


	}

	if(bRowRemoved) {
		//Add a line to the message window.
		ChatUser cu = GetUser(nUserID, dwIpAddress);
		IRowSettingsPtr pRow = m_pChatWindow->GetRow(-1);
		pRow->PutValue(0, (long)dwIpAddress);
		pRow->PutValue(1, (long)nUserID);
		pRow->PutValue(2, _bstr_t(cu.strUsername));
		pRow->PutCellForeColor(2, cu.Color);
		pRow->PutValue(3, _bstr_t(strMessage));
		m_pChatWindow->AddRow(pRow);
		//Set the sel to the last row (the highlight won't show, but it will scroll to the bottom.
		m_pChatWindow->CurSel = m_pChatWindow->GetRowCount()-1;
	}
}

// (a.walling 2006-09-11 17:53) - PLID 20980 - Flash the YakChat window
void CChatSessionDlg::Flash()
{
	HWND hwndActive = ::GetActiveWindow();
	HWND hwndDlg = this->GetSafeHwnd();
	if (hwndActive == hwndDlg) {
		return; // do not flash if we are the active window
	}

	FLASHWINFO fwi;
	fwi.cbSize = sizeof(FLASHWINFO);
	fwi.hwnd = this->GetSafeHwnd();
	fwi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
	fwi.dwTimeout = 0;
	fwi.uCount = 10;
	// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - CWnd::FlashWindowEx actually exists in mfc7
	::FlashWindowEx(&fwi);
}
