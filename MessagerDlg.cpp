// MessagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PracticeRc.h"
#include "MessagerDlg.h"
#include "GlobalDataUtils.h"
#include "SendMessageDlg.h"
#include "Client.h"
#include <mmsystem.h>
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "NxMessageDef.h"
#include "MessageHistoryDlg.h"
#include "PreferenceUtils.h"
#include "ResEntryDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "IconUtils.h"
#include "regutils.h"
#include "GlobalReportUtils.h"

//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NxTab;
using namespace ADODB;
using namespace NetUtils;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMessagerDlg dialog

// (a.walling 2008-06-11 13:44) - PLID 30354 - Used the new NOTIFYICONDATA structure
// (since we can't use Windows 2000 defines yet without breaking file dialogs until we move out of MFC4.2)



enum menuCommands {
	cmdOpenYakker = 0x100,
	cmdDisableYakker,
	cmdDynamic
};

#define EXTRA_HEIGHT 255
// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - remember position
// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
CMessagerDlg::CMessagerDlg(CMainFrame* pParent /*=NULL*/)
	: CNxModelessOwnedDialog(CMessagerDlg::IDD, NULL, "CMessagerDlg")
	, m_userChecker(NetUtils::Coordinators)
	, m_pParent(pParent)
{
	//{{AFX_DATA_INIT(CMessagerDlg)
	m_strDate = _T("");
	m_strPriority = _T("");
	m_strRegarding = _T("");
	m_strSender = _T("");
	m_strText = _T("");
	m_strSentTo = _T("");
	m_bMoreInfo = true;
	m_bLoggedOn = false;
	m_bEnabled = false;
	m_bNetworkEnabled = false;
	m_arUsers = NULL;
	m_nUsers = 0;
	m_bResizing = false;
	m_pMessageHistoryDlg = NULL;
	m_pParent = pParent;
	m_NID.hWnd = NULL;
	m_bProcessingRequests = false;
	m_hNormalIcon = NULL;
	m_hFlashingIcon = NULL;
	m_hDisabledIcon = NULL;
	m_hChatIcon = NULL;
	m_bCheckForUnreadMessages = FALSE;
	m_bUnreadMessages = FALSE;
	m_nCurrentReceivedMessage = -1;
	m_nCurrentSentMessage = -1;
	m_bListsRequeried = FALSE;
	m_bSystemClose = FALSE;
	//}}AFX_DATA_INIT
	m_bMessagerDlgHasDisabledHotkeys = FALSE;
	// (a.walling 2007-07-10 17:51) - PLID 4850 - init m_nUsers

	::ZeroMemory(&m_NID, sizeof(m_NID));
	m_NID.uVersion = NOTIFYICON_VERSION;
}

CMessagerDlg::~CMessagerDlg()
{
	if(m_hNormalIcon) {
		if(m_bCanDestroyNormal) DestroyIcon(m_hNormalIcon);
		m_hNormalIcon = NULL;
	}
	if(m_hDisabledIcon) {
		if(m_bCanDestroyDisabled) DestroyIcon(m_hDisabledIcon);
		m_hDisabledIcon = NULL;
	}
	if(m_hFlashingIcon) {
		if(m_bCanDestroyFlashing) DestroyIcon(m_hFlashingIcon);
		m_hFlashingIcon = NULL;
	}
	if(m_hChatIcon) {
		if(m_bCanDestroyChat) DestroyIcon(m_hChatIcon);
		m_hChatIcon = NULL;
	}
}
void CMessagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxModelessOwnedDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessagerDlg)
	DDX_Control(pDX, IDC_VIEW_RECEIVED, m_btnViewReceived);
	DDX_Control(pDX, IDC_VIEW_SENT, m_btnViewSent);
	DDX_Text(pDX, IDC_DATE, m_strDate);
	DDX_Text(pDX, IDC_PRIORITY, m_strPriority);
	DDX_Text(pDX, IDC_REGARDING, m_strRegarding);
	DDX_Text(pDX, IDC_SENDER, m_strSender);
	DDX_Text(pDX, IDC_MESSAGE_TEXT, m_strText);
	DDX_Text(pDX, IDC_SENT_TO, m_strSentTo);
	DDX_Control(pDX, IDC_SENT_TO, m_nxeditSentTo);
	DDX_Control(pDX, IDC_DATE, m_nxeditDate);
	DDX_Control(pDX, IDC_PRIORITY, m_nxeditPriority);
	DDX_Control(pDX, IDC_SENDER, m_nxeditSender);
	DDX_Control(pDX, IDC_REGARDING, m_nxeditRegarding);
	DDX_Control(pDX, IDC_MESSAGE_TEXT, m_nxeditMessageText);
	DDX_Control(pDX, IDC_MARKER_SMALL, m_nxstaticMarkerSmall);
	DDX_Control(pDX, IDC_MARKER_LARGE, m_nxstaticMarkerLarge);
	DDX_Control(pDX, IDC_PREVIEW_MESSAGE, m_btnPreviewMessage);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_REGARDING_LABEL, m_nxstaticRegardingLabel);
	//}}AFX_DATA_MAP
}

// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Removed this; we already derive this from CWnd, silly.
// (a.walling 2007-11-29 15:00) - PLID 27998 - VS2008 - Forgot to put the other macro in! ON_WM_TIMER()
	// ON_MESSAGE(WM_TIMER, OnTimer)

BEGIN_MESSAGE_MAP(CMessagerDlg, CNxModelessOwnedDialog)
	//{{AFX_MSG_MAP(CMessagerDlg)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_REPLY, OnReply)
	ON_BN_CLICKED(IDC_REPLY_ALL, OnReplyAll)
	ON_BN_CLICKED(IDC_GOTO, OnGotoPatient)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_MESSAGE_DELETE, OnDeleteMessage)
	ON_COMMAND(ID_MESSAGE_ADD_NOTE, OnAddNote)
	ON_COMMAND(ID_USERS_SEND, OnSendMessage)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnYakTrayNotify)
	ON_COMMAND(ID_POPUP_OPENPRACYAKKER, OnOpenYakker)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_NEW_MESSAGE, OnNewMessage)
	ON_BN_CLICKED(IDC_MORE_INFO, OnMoreInfo)
	ON_BN_CLICKED(IDC_FORWARD, OnForward)
	ON_WM_CHAR()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHAT, OnChat)
	ON_BN_CLICKED(IDC_VIEW_SENT, OnViewSent)
	ON_BN_CLICKED(IDC_VIEW_RECEIVED, OnViewReceived)
	ON_COMMAND(ID_MESSAGE_HISTORY, OnShowMessageHistory)
	ON_BN_CLICKED(IDC_SHOW_MESSAGE_HISTORY, OnShowMessageHistoryBtn)
	ON_MESSAGE(NXM_GOTO_MESSAGE, OnGotoMessage)
	ON_BN_CLICKED(IDC_FORWARD_MESSAGE, OnForwardMessage)
	ON_MESSAGE(NXM_NXSERVER_CONNECTED, OnNxServerConnected)
	ON_MESSAGE(NXM_NXSERVER_DISCONNECTED, OnNxServerDisconnected)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_YAKKER_PREFS, OnYakkerPrefs)
	ON_COMMAND(ID_POPUP_DISABLEPRACYAKKER, OnDisableYakker)
	ON_MESSAGE(NXM_IM_USERLIST, RefreshUserList)
	ON_MESSAGE(NXM_IM_MESSAGE, OnMessageReceived)
	ON_MESSAGE(NXM_IM_MSGSENT, OnMessageSent)
	ON_MESSAGE(NXM_CHAT_MESSAGE, OnChatMessage)
	ON_BN_CLICKED(IDC_PREVIEW_MESSAGE, OnPreviewMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessagerDlg message handlers

BOOL CMessagerDlg::OnInitDialog() 
{
	CNxModelessOwnedDialog::OnInitDialog();

	// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the pracyakker in the taskbar if necessary
	//								  PLID 22877 - and respect the preference to not do so
	if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
		HWND hwnd = GetSafeHwnd();
		long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		nStyle |= WS_EX_APPWINDOW;
		SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
	}
	
	try{
		m_pMessages = BindNxDataListCtrl(IDC_MESSAGE_LIST, false);
		m_pUsers = BindNxDataListCtrl(IDC_USERS);
		m_pSentList = BindNxDataListCtrl(IDC_SENT_LIST, false);

		m_btnPreviewMessage.AutoSet(NXB_PRINT_PREV);
		m_btnClose.AutoSet(NXB_CLOSE);

		CString strWhere;
		strWhere.Format("MessagesT.RecipientID = %li AND DeletedByRecipient = 0", GetCurrentUserID());
		m_pMessages->WhereClause = (LPCTSTR)strWhere;
		m_pMessages->CurSel = -1;
		m_nCurrentReceivedMessage = -1;
		
		strWhere.Format("MessagesT.SenderID = %li AND DeletedBySender = 0", GetCurrentUserID());
		m_pSentList->WhereClause = (LPCTSTR)strWhere;
		m_pSentList->CurSel = -1;
		m_nCurrentSentMessage = -1;
		
		GetDlgItem(IDC_REPLY)->EnableWindow(FALSE);
		GetDlgItem(IDC_GOTO)->EnableWindow(FALSE);
		GetDlgItem(IDC_FORWARD_MESSAGE)->EnableWindow(FALSE);

		//Update the dialog title
		RefreshCaption();
		
		// defualt to the received messages being shown
		((CButton*)GetDlgItem(IDC_VIEW_RECEIVED))->SetCheck(TRUE);
		//This will call Refresh();
		OnViewReceived();

		m_pParent->RequestTableCheckerMessages(GetSafeHwnd());

		// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
		RefreshColors();

	}NxCatchAll("Error in CMessagerDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMessagerDlg, CNxModelessOwnedDialog)
    //{{AFX_EVENTSINK_MAP(CMessagerDlg)
	ON_EVENT(CMessagerDlg, IDC_MESSAGE_LIST, 1 /* SelChanging */, OnSelChangingMessageList, VTS_PI4)
	ON_EVENT(CMessagerDlg, IDC_USERS, 6 /* RButtonDown */, OnRButtonDownUsers, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMessagerDlg, IDC_MESSAGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedMessageList, VTS_I2)
	ON_EVENT(CMessagerDlg, IDC_MESSAGE_LIST, 2 /* SelChanged */, OnSelChangedMessageList, VTS_I4)
	ON_EVENT(CMessagerDlg, IDC_MESSAGE_LIST, 10 /* EditingFinished */, OnEditingFinishedMessageList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CMessagerDlg, IDC_USERS, 1 /* SelChanging */, OnSelChangingUsers, VTS_PI4)
	ON_EVENT(CMessagerDlg, IDC_MESSAGE_LIST, 7 /* RButtonUp */, OnRButtonUpMessageList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMessagerDlg, IDC_SENT_LIST, 2 /* SelChanged */, OnSelChangedSentList, VTS_I4)
	ON_EVENT(CMessagerDlg, IDC_SENT_LIST, 7 /* RButtonUp */, OnRButtonUpSentList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMessagerDlg, IDC_USERS, 3 /* DblClickCell */, OnDblClickCellUsers, VTS_I4 VTS_I2)
	ON_EVENT(CMessagerDlg, IDC_SENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSentList, VTS_I2)
	ON_EVENT(CMessagerDlg, IDC_USERS, 18 /* RequeryFinished */, OnRequeryFinishedUsers, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMessagerDlg::OnSelChangingMessageList(long FAR* nNewSel) 
{
	//All the stuff I had here should have been in OnSelChanged, or in fact, in a separate function,
	//as it now is.  Let this be a lesson unto thee.
	
}

void CMessagerDlg::OnOK() 
{
	//Let's set focus to Practice first, otherwise the z-order will be Yakker, (some other window), Practice; and that will be confusing.
	
	//Took this out of OnOpenYakker, which should have been the only place it was needed, but no, it has to be here.

	//But, Tom, you say, this is ridiculous!  Shouldn't you just need to call SetWindowPos(pMain, ...)? 
	//Yes, that's all I should have to do, according to the documentation.  However, that won't work.  I don't
	//know why, but it won't work. So instead we bring pMain to the top, and then insert after all non-topmost windows
	//(and therefore, after pMain).  It's dumb, but it works, so I don't want to hear any more complaints.
	if (!m_bSystemClose) {
		// (a.walling 2007-05-04 09:56) - PLID 4850 - If we are being closed by the system rather than the user, don't mess with this
		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - don't bring mainframe to top
		MessagerDlgEnableHotkeys();

		//SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		//SetForegroundWindow();
		//ShowWindow(SW_HIDE);
		CloseDialog();
	}
}

void CMessagerDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Do not call CNxModelessOwnedDialog::OnPaint() for painting messages
}

void CMessagerDlg::OnReply() 
{
	try{
		if(m_pMessages->CurSel != -1) {
			//Set up a send message dialog with the necessary info
			// (c.haag 2006-05-16 15:25) - PLID 20644 - This code has been adjusted to handle
			// modeless message dialogs
			// (a.walling 2007-05-04 09:58) - PLID 4850 - CSendMessageDlg is now a pointer
			CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
			ASSERT(dlg != NULL);

			CDWordArray adwRecipients;
			CStringArray astrRecipients;
			long nRegardingID;
			adwRecipients.Add(VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcSenderID)));
			astrRecipients.Add(VarString(m_pMessages->GetValue(m_pMessages->CurSel, mlcSenderName)));
			nRegardingID = VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcRegardingID), -1);
			
			// (j.gruber 2010-07-16 14:04) - PLID 39463 - send the threadID
			long nThreadID = VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcMesThreadID));
			dlg->PopUpMessage(nThreadID, nRegardingID, &adwRecipients, &astrRecipients);

			//Here's the idea, if you get a message and reply to it, then you don't want to have to deal with it
			//anymore, at least that's the way I look at it.  So if we just have the "small" version up, and we've
			//replied, then let's get out of here and let them get back to work.			
			//TES 7/14/03: However, if they DO want to deal with it, let them.
			if(!GetRemotePropertyInt("YakkerAutoClose", 1, 0, GetCurrentUserName(), true)) {
				//ShowWindow(SW_SHOW); // Should already be visible!
			} else {
				ShowWindow(SW_HIDE);
			}
		}

	}NxCatchAll("Error in CMessagerDlg::OnReply()");
	
}

void CMessagerDlg::OnReplyAll()
{
	try{
		if(m_pMessages->CurSel != -1) {
			//Set up a send message dialog with the necessary info
			// (c.haag 2006-05-16 15:25) - PLID 20644 - This code has been adjusted to handle
			// modeless message dialogs
			CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
			ASSERT(dlg != NULL);

			CDWordArray adwRecipients;
			CStringArray astrRecipients;
			long nRegardingID;

			// (c.haag 2004-06-23 13:28) - Fill in the recipients field
			long nRow = m_dlCurrent->CurSel;
			if (nRow > -1)
			{
				_RecordsetPtr rsSentTo = CreateRecordset("SELECT RecipientID, UserName FROM (SELECT RecipientID, UserName FROM MessagesT LEFT JOIN UsersT ON RecipientID = UsersT.PersonID WHERE UserName IS NOT NULL AND MessageGroupID IN (SELECT MessageGroupID FROM MessagesT WHERE MessageID = %d) "
					"UNION SELECT SenderID AS RecipientID, UserName FROM MessagesT LEFT JOIN UsersT ON SenderID = UsersT.PersonID WHERE MessageID = %d AND Username IS NOT NULL) SubQ ORDER BY UserName",
					VarLong(m_dlCurrent->GetValue(nRow, mlcID)), VarLong(m_dlCurrent->GetValue(nRow, mlcID)));
				while (!rsSentTo->eof)
				{
					adwRecipients.Add(AdoFldLong(rsSentTo, "RecipientID"));
					astrRecipients.Add(AdoFldString(rsSentTo, "UserName", ""));
					rsSentTo->MoveNext();
				}
				rsSentTo->Close();
			}
			nRegardingID = VarLong(m_dlCurrent->GetValue(nRow, mlcRegardingID), -1);
			
			// (j.gruber 2010-07-16 14:04) - PLID 39463 - send the threadID
			long nThreadID = VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcMesThreadID));
			dlg->PopUpMessage(nThreadID, nRegardingID, &adwRecipients, &astrRecipients);

			//Here's the idea, if you get a message and reply to it, then you don't want to have to deal with it
			//anymore, at least that's the way I look at it.  So if we just have the "small" version up, and we've
			//replied, then let's get out of here and let them get back to work.
			if(!GetRemotePropertyInt("YakkerAutoClose", 1, 0, GetCurrentUserName(), true)) {
				//ShowWindow(SW_SHOW); // Should already be visible!
			} else {
				ShowWindow(SW_HIDE);
			}
		}

	}NxCatchAll("Error in CMessagerDlg::OnReplyAll()");
}

void CMessagerDlg::OnGotoPatient() 
{
	try{
		if(m_dlCurrent->CurSel != -1) {
			//Look up the patient ID
			long nID = VarLong(m_dlCurrent->GetValue(m_dlCurrent->CurSel, mlcRegardingID), -1);
			if(nID != -1) {

				CMainFrame *p = GetMainFrame();
				CNxTabView *pView;

				if (nID != GetActivePatientID()) {
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if(!p->m_patToolBar.TrySetActivePatientID(nID)) {
						return;
					}
				}

				if(p->FlipToModule(PATIENT_MODULE_NAME)) {
					pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
					if (pView) {
						if(pView->GetActiveTab() != 0) {
							pView->SetActiveTab(0);
						}

						// (j.jones 2009-06-03 10:25) - PLID 34461 - need to always update the patient view
						pView->UpdateView();
					}

					// (r.farnworth 2016-04-13 15:37) - PLID 27771 - When you select go to patient on the prac yakker, it should minimize instead of close.
					long nYakkerGoToPatient = GetRemotePropertyInt("YakkerGoToPatient", 1, 0, GetCurrentUserName(), true);
					if (nYakkerGoToPatient == 0) {
						OnOK();
					}
					else if (nYakkerGoToPatient == 1) {
						//minimize
						ShowWindow(SW_SHOWMINIMIZED);
					}
				}
			}
		}
	}NxCatchAll("Error in CMessagerDlg::OnGotoPatient()");
}

HBRUSH CMessagerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxModelessOwnedDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

 void CMessagerDlg::OnAddNote(){

	long nRow = m_dlCurrent->CurSel;
	if(nRow == -1) return;

	try {
		long nID = VarLong(m_dlCurrent->GetValue(nRow, mlcID));
		COleDateTime dtDate = VarDateTime(m_dlCurrent->GetValue(nRow, mlcDate));
		long nUserID = GetCurrentUserID();
		CString strNote = GetCurrentUserName();
		strNote += " - " + VarString(m_dlCurrent->GetValue(nRow, mlcMessage));

		long nRegardingID = VarLong(m_dlCurrent->GetValue(nRow, mlcRegardingID),-1);

		if (nRegardingID != -1) {

			//insert into notes
			// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
			ExecuteParamSql(
				"INSERT INTO Notes (PersonID,Date,UserID,Note)\r\n"
				"	VALUES ({INT},{OLEDATETIME},{INT},{STRING})",
				nRegardingID, dtDate, nUserID, strNote);
			
		}
	}NxCatchAll("Error creating note");

}

void CMessagerDlg::OnDeleteMessage(){
	

	try{
		long p = m_dlCurrent->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		while (p)
		{	
			m_dlCurrent->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			// make sure the message has been viewed, otherwise the icon will flash indefinitly but there are no messages
			// for the user to view
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(m_nCurrentView ==  vtReceived && ReturnsRecordsParam("SELECT * FROM MessagesT WHERE viewed = 0 AND MessageID = {INT}", VarLong(pRow->GetValue(mlcID)))){
				AfxMessageBox("This message cannot be deleted.  It has not been viewed yet.");
			}
			else {
				// MSC - 8/6/03 - Because messages can be shown on more then just the recipients computer, we need
				// to mark it deleted by this person instead of actually deleting it like we used to, 
				// then we can just not show this.
				CString strField;
				long nIndex = pRow->Index;
				if(m_nCurrentView == vtReceived){
					strField = "DeletedByRecipient";
				}
				else if(m_nCurrentView == vtSent){
					strField = "DeletedBySender";
				}
				else{
					ASSERT(FALSE);
				}

				ExecuteSql("UPDATE MessagesT SET %s = 1 WHERE MessageID = %li", strField, VarLong(pRow->GetValue(mlcID)));

				//
				// Remove from datalist
				//
				m_dlCurrent->RemoveRow(nIndex);
				pDisp->Release();
			}
		}

		// (c.haag 2006-05-19 10:59) - PLID 20714 - Not necessary
		//RequeryCurrentList();
		NewSelection(m_dlCurrent->CurSel);

	}NxCatchAll("Error in CMessagerDlg::OnDeleteMessage");
}

void CMessagerDlg::OnRButtonDownUsers(long nRow, short nCol, long x, long y, long nFlags) 
{
	//TODO:  When the UserPermissions table is usable, add a permission for deleting messages,
	//and if they don't have permission, don't show this menu

	if(nRow == -1) return;

	//Don't show this if this is the "Offline" row.
	if(VarLong(m_pUsers->GetValue(nRow, mulcID)) == -1)
		return;

	m_pUsers->CurSel = nRow;
	
	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();

	pMenu->InsertMenu(0, MF_BYPOSITION, ID_USERS_SEND, GetStringOfResource(IDS_SEND_MESSAGE));
	pMenu->InsertMenu(1, MF_BYPOSITION, ID_MESSAGE_HISTORY, "Message History");

	//set the default to "Send Message..." since that's what happens when you double-click
	pMenu->SetDefaultItem(0, TRUE);
		
	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}


void CMessagerDlg::OnSendMessage(){
	long nRow = m_pUsers->CurSel;
	if(nRow == -1) return;

	try{
		//Set up a send message dialog with the necessary info
		// (c.haag 2006-05-16 15:25) - PLID 20644 - This code has been adjusted to handle
		// modeless message dialogs
		CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
		ASSERT(dlg != NULL);

		CDWordArray adwRecipients;
		adwRecipients.Add(VarLong(m_pUsers->GetValue(m_pUsers->CurSel, mulcID)));
		if(adwRecipients.GetSize() > 0){
			
			//OK, here's the plan:  We want to hide the main window while they're composing a new message,
			//just to get it out of the way.  Also, if they were viewing the small version of this
			//dialog, we don't want to show it again, so that if they get a message and reply to it,
			//they don't have to bother closing the main window if that's all they wanted to do.
			dlg->PopUpMessage(-1, -1, &adwRecipients);
			if(m_bMoreInfo) {
				//ShowWindow(SW_SHOW); // Should already be visible!
			} else {
				ShowWindow(SW_HIDE);
			}
		}
		// (c.haag 2006-05-19 10:59) - PLID 20714 - Not necessary
		//RequeryCurrentList();

	}NxCatchAll("Error in CMessagerDlg::OnReply()");
}

void CMessagerDlg::OnRequeryFinishedMessageList(short nFlags) 
{
	try {
		if(IsWindowVisible()) {
			if(m_nCurrentReceivedMessage != -1) {
				m_pMessages->SetSelByColumn(mlcID,m_nCurrentReceivedMessage);
				NewSelection(m_pMessages->CurSel);
			}
			else if(m_pMessages->GetRowCount() > 0) {
				m_nCurrentReceivedMessage = VarLong(m_pMessages->GetValue(0, mlcID));
				m_pMessages->CurSel = 0;
				NewSelection(0);
			}
			else {
				NewSelection(-1);
			}
		}
			

		//Set up colors
		CString strPriority;
		IRowSettingsPtr pRow;
		for(int i = 0; i < m_pMessages->GetRowCount(); i++){
			strPriority = VarString(m_pMessages->GetValue(i, mlcPriority));
			if(strPriority == "Urgent") {
				pRow = m_pMessages->GetRow(i);
				pRow->PutCellForeColor(6,0x000000FF);
				pRow->PutCellForeColorSel(6,0x000000FF);
			}
		}

		//TES 5/3/2007 - PLID 25895 - Now that we've requeried the list, we know how many unread messages there are, 
		// so update the caption to reflect that.
		RefreshCaption();
	}NxCatchAll("Error in OnRequeryFinishedMessageList");
}

LRESULT CMessagerDlg::OnYakTrayNotify(WPARAM wp, LPARAM lp){
	try {
		//if(lp == WM_RBUTTONUP){
		if(lp == WM_RBUTTONDOWN){
			/*
			CMenu menu;
			CMenu* pSubMenu;
			menu.LoadMenu(IDR_YAK);
			pSubMenu = menu.GetSubMenu(0);
			::SetMenuDefaultItem(pSubMenu->m_hMenu, 0, TRUE);
			CPoint mouse;      
			GetCursorPos(&mouse);      
			::SetForegroundWindow(m_NID.hWnd);        
			::TrackPopupMenu(pSubMenu->m_hMenu, 0, mouse.x, mouse.y, 0, m_NID.hWnd, NULL);
			*/

			// (a.walling 2010-11-26 13:08) - PLID 40444 - Use a popup menu of our own so we can easily dynamically insert things.
			CMenu menu;
			menu.CreatePopupMenu();
			long nMenuID = 0x80;
			long nCmdID = cmdDynamic;

			
			// (a.walling 2010-11-26 13:08) - PLID 40444 - Populate all the module's menu items
			m_popupMenuHelper = Modules::PopupMenuHelper::Create(menu, nMenuID, nCmdID);

			Modules::PopupMenuHelperPtr pModulePopups = m_popupMenuHelper;
			pModulePopups->AppendAllPopupMenus();

			menu.InsertMenu(nMenuID++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			menu.InsertMenu(nMenuID++, MF_BYPOSITION, cmdOpenYakker, "&Open PracYakker...");
			menu.InsertMenu(nMenuID++, MF_BYPOSITION, cmdDisableYakker, "&Disable PracYakker");

			menu.SetDefaultItem(cmdOpenYakker);

			MENUINFO menuInfo;
			menuInfo.cbSize = sizeof(menuInfo);
			menuInfo.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
			menuInfo.dwStyle = MNS_AUTODISMISS;
			menu.SetMenuInfo(&menuInfo);

			CPoint mouse;
			GetCursorPos(&mouse);
			::SetForegroundWindow(m_NID.hWnd);
			long nCommandID = menu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD|TPM_NONOTIFY, mouse.x, mouse.y, CWnd::FromHandle(m_NID.hWnd), NULL);

			if (nCommandID <= 0) {
				return 0;
			}
			else if (nCommandID == cmdOpenYakker) {
				OnOpenYakker();
				return 0;
			}
			else if (nCommandID == cmdDisableYakker) {
				OnDisableYakker();
				return 0;
			} else if (nCommandID >= cmdDynamic) {
				// (a.walling 2010-11-26 13:08) - PLID 40444 - Handle any module's menu items
				if (pModulePopups->HandleCommand(nCommandID)) {
					// After this returns, release our member variable. However our local variable keeps it alive until it goes out of scope, huzzah
					m_popupMenuHelper.reset();
				}
				return 0;
			}
		}
		if(lp == WM_LBUTTONDBLCLK){
			//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard
			// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
			if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm") && CheckCurrentUserPermissions(bioPracYakker, sptView)) {
				OpenYakker();
			}
		}
	} NxCatchAllThread(__FUNCTION__);

	return 0;
}

void CMessagerDlg::OnOpenYakker()
{
	//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard
	// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
	if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm") && CheckCurrentUserPermissions(bioPracYakker, sptView)) {
		OpenYakker();
	}
}

void CMessagerDlg::OpenYakker()
{
	//TES 12/19/2008 - PLID 32525 - Moved this functionality from the OnOpenYakker() function into this utility function
	// that can be called directly (as OnOpenYakker() was before, incorrectly).
	if(IsWindowVisible() && !IsIconic()) {
		//TES 5/3/2007 - PLID 18516 - If we are on screen and not minimized, we don't want to auto-advance to the 
		// next message.
		OpenYakker(false);
	}
	else {
		OpenYakker(true);
	}
}

void CMessagerDlg::OpenYakker(bool bSelectNewMessage)
{
	if(!m_bLoggedOn){
		LogOnTCP();
	}

	// (j.jones 2006-05-24 11:41) - PLID 20793 - if we haven't requeried yet, it's because
	// the yakker wasn't enabled, but now we're manually opening it, so enable requerying
	//TES 3/12/2009 - PLID 33511 - If we're not connected to the network, then we need to always requery, because
	// we won't be getting notified about new messages, so the only way to get them in our list is to requery.
	if(!m_bListsRequeried || !m_bNetworkEnabled) {
		//TES 6/15/2009 - PLID 34624 - If we're disconnected from the network, we have to keep requerying the lists.
		if(m_bNetworkEnabled) {
			m_bListsRequeried = TRUE;
		}
		RequeryCurrentList();
	}

	// (a.walling 2006-10-06 17:27) - PLID 22880 - Minimized pracyak won't always restore correctly
	if (!IsWindowVisible() || IsIconic()) {
		ShowWindow(SW_RESTORE);
		MessagerDlgDisableHotkeys();

		//Decide whether we should be big or small.
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		m_bMoreInfo = !ReturnsRecordsParam("SELECT MessageID FROM MessagesT WHERE Viewed = 0 AND DeletedByRecipient = 0 AND RecipientID = {INT}", GetCurrentUserID());
	}

	// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - Don't bring mainframe to top
	SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(); 

	try {
		//Now, deal with any pending requests.
		//TES 12/17/2003: Let's do this in a critical section type way.
		if(!m_bProcessingRequests) {
			m_bProcessingRequests = true;
			for(int i = 0; i < m_arPendingRequests.GetSize(); i++) {
				_RecordsetPtr rsUsername = CreateRecordset("SELECT Username FROM UsersT WHERE PersonID = %u", m_arPendingRequests.GetAt(i).dwUserId);
				if(IDYES == MsgBox(MB_YESNO, "%s has invited you to join a chat session.  Accept?", AdoFldString(rsUsername, "Username"))) {
					CChatSessionDlg *pDlg;
					pDlg = new CChatSessionDlg(this);
					pDlg->m_nSessionId = m_arPendingRequests.GetAt(i).dwSessionId;
					pDlg->Create(pDlg->IDD, GetDesktopWindow());
					m_arChatDlgs.Add(pDlg);
					pDlg->ShowWindow(SW_SHOW);
				}
			}
			//All requests have now been handled.
			m_arPendingRequests.RemoveAll();
			m_bProcessingRequests = false;
		}

		//Now, let the Yak handle any ordinary Yak stuff.
		//TES 5/3/2007 - PLID 18516 - Pass in our parameter for whether to auto-select the first unread message.
		Refresh(bSelectNewMessage);
	}NxCatchAll("Error in CMessagerDlg::OnOpenYakker()");


}

void CMessagerDlg::OnDisableYakker(){

	if (!m_bEnabled)
		return;
	else{
		char* szUserName = new char[strlen(GetCurrentUserName())+1];
		strcpy(szUserName, GetCurrentUserName());
		CClient::Send(PACKET_TYPE_IM_LOGOFF, (void*)szUserName, strlen(szUserName)+1);

		delete [] szUserName;
		
		Shell_NotifyIcon(NIM_DELETE, (NOTIFYICONDATA*)&m_NID);
		// (b.cardillo 2004-10-19 11:59) - We used to call DeleteObject(m_NID.hIcon) here, but not only does
		// DeleteObject() have 0 effect on icons, even if we had called DestroyIcon() (which I believe was 
		// the actually intent) we'd be calling it on a handle that had already been destroyed, because 
		// everywhere that we set m_NID.hIcon we always then call DestroyIcon() on it as soon as we're 
		// finished using it, so it should never be a valid handle at this point.

		/* (b.cardillo 2004-10-19 11:49) - This mutex was part of the system where we spawned a separate 
		// process meant to ensure the icon didn't get left in the sys tray on Practice crashing.  But for 
		// now we don't mess with that separate process because it seems to cause problems with other apps.
		/////
		//Now the icon is deleted, so we no longer need the mutex we were using.
		ReleaseMutex(m_mutIconMutex);
		*/
		m_bEnabled = false;

		//Dismiss the dialog, if it's up.
		OnOK();

	}

	
}

void CMessagerDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try{
		if(bShow){
			// (e.lally 2009-06-10) PLID 34498 - Get permission for viewing patients module, hide the regarding
			int nShowRegarding = SW_SHOW;
			if(!(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass))){
				//Don't have view permission on the patients module, hide the regarding controls
				nShowRegarding = SW_HIDE;
			}
			GetDlgItem(IDC_REGARDING_LABEL)->ShowWindow(nShowRegarding);
			GetDlgItem(IDC_REGARDING)->ShowWindow(nShowRegarding);
			GetDlgItem(IDC_GOTO)->ShowWindow(nShowRegarding);
		}
	}
	NxCatchAll("Error in OnShowWindow while checking permissions");

	try{
		CNxModelessOwnedDialog::OnShowWindow(bShow, nStatus);

	}NxCatchAll(__FUNCTION__);

	//TS 12-20-2001: We don't do this here anymore, because we hide this window when we compose a message,
	//so we now do it in OnOpenYakker and OnOK, so they'll stay disabled throughout the session.
	//We need to disable hot keys so that, for instance, the billing tab won't go crazy if it's
	//underneath the PracYakker and you hit like backspace.
	/*if(bShow) GetMainFrame()->DisableHotKeys();
	else GetMainFrame()->EnableHotKeys(); */

	//TES 10/20/03: I don't know who put this here (Matt) but this is and should be done in OnOpenYakker.	
	/*if(bShow) {
		// default to the received messages being shown
		((CButton*)GetDlgItem(IDC_VIEW_SENT))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_VIEW_RECEIVED))->SetCheck(TRUE);
		OnViewReceived();
	}*/
	
}

int CMessagerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxModelessOwnedDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	//m_pParent = (CMainFrame*)FromHandlePermanent(lpCreateStruct->hwndParent);
	ASSERT(GetMainFrame() == m_pParent);

	// (a.walling 2007-07-24 13:46) - PLID 26787 - Cache messager common properties
	// (a.walling 2007-09-04 12:37) - PLID 26787 - Added more properties
	// (k.messina 2010-03-22 10:50) - PLID 4876 - Added preview properties
	// (r.farnworth 2016-04-13 15:36) - PLID 27771 - Added YakkerGoToPatient
	g_propManager.CachePropertiesInBulk("Messager-Create", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DisplayTaskbarIcons' OR "
			"Name = 'YakWindowColor' OR "
			"Name = 'YakkerEnabled' OR " 
			"Name = 'ClassicYakIcon' OR "
			"Name = 'YakTheme' OR "
			"Name = 'AutoPopupYakker' OR "
			"Name = 'UseAreaCode' OR "
			"Name = 'YakkerBeep' OR "
			"Name = 'YakkerAutoClose' OR "
			"Name = 'YakPreviewPastMessages' OR "
			"Name = 'YakPreviewMessage' OR "
			"Name = 'YakkerGoToPatient' "
			")",
			_Q(GetCurrentUserName()));
	
	return 0;
}

void CMessagerDlg::ChangeLogonStatus(){
	CWaitCursor cuWait;
	if(m_bEnabled){
		OnDisableYakker();
	}
	else{
		//TES 12/19/2008 - PLID 32525 - We check the license here so that if they are Scheduler Standard users, they
		// will get the warning message.  However, we still let them enable the PracYakker, so they can see the icon 
		// and be tempted to buy it.
		g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm");
		OnEnablePracYakker();
	}
}

void CMessagerDlg::InitPracYakkerTray(){
	//TS:  Create system tray icon
	m_NID.cbSize = sizeof(m_NID);
	m_NID.uID = IDR_YAK;
	sprintf(m_NID.szTip, "PracYakker (disabled)");
	m_NID.hWnd = this->GetSafeHwnd();
	m_NID.hIcon = GetIcon(itDisabled);
	m_NID.uCallbackMessage = WM_TRAY_NOTIFICATION;
	m_NID.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, (NOTIFYICONDATA*)&m_NID);

	//Start a process that will detect if practice crashes
	
	//TODO: Someday, somehow, make this work right.
	/*STARTUPINFO si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	CString strCommandLine;
	//Generate a unique mutex to use with this icon
	m_strIconMutex.Format("NextechPracYakkerIconUniqueMutex%li%li%s", m_NID.hWnd, m_NID.uID, COleDateTime::GetCurrentTime().Format("%H%M%S"));
	m_mutIconMutex = CreateMutex(NULL, true, m_strIconMutex);
	CString strPath;
	//Let's try to find NxTray
	//First, we'll check the place where Practice.exe is.
	GetModuleFileName(NULL, strPath.GetBuffer(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	//Some operating systems(?, I don't know whether it's the operating system or what) capitalize GetModuleFileName, some don't.
	strPath.MakeUpper();
	strPath.Replace("PRACTICE.EXE", "NXTRAY.EXE");
	if(!DoesExist(strPath)){
		//Well, it wasn't there, let's try the working directory.
		GetCurrentDirectory(MAX_PATH, strPath.GetBuffer(MAX_PATH));
		strPath.ReleaseBuffer();
		strPath = strPath ^ "NxTray.exe";
		if(!DoesExist(strPath)){
			//That's it, we give up.  Let's get out of here.
			return;
		}
	}

		
	strCommandLine.Format("%s mutex:%s hwnd:%li uid:%li", strPath, m_strIconMutex, m_NID.hWnd, m_NID.uID);
	
	if(::CreateProcess(strPath, strCommandLine.GetBuffer(MAX_PATH + 100), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)){
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	}
	strCommandLine.ReleaseBuffer();*/
}

void CMessagerDlg::OnEnablePracYakker()
{
	if(m_bEnabled) {
		return;
	}
	else {

		m_bEnabled = true;

		m_nCurrentReceivedMessage = -1;
		//this is requeried in RequeryCurrentList()
		//m_pMessages->Requery();
		m_nCurrentSentMessage = -1;
		////this is requeried in RequeryCurrentList()
		//m_pSentList->Requery();
			
		InitPracYakkerTray();
		
		if(m_bNetworkEnabled){
			LogOnTCP();
		}

		//TES 3/29/2004: PLID 11647 - Make sure if they have new messages they are notified appropriately.
		//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard, so there
		// is no point in trying to notify them about new messages.
		if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(ReturnsRecordsParam("SELECT MessageID FROM MessagesT WHERE Viewed = 0 AND DeletedByRecipient = 0 AND RecipientID = {INT}", GetCurrentUserID())) {
				int nAction = GetRemotePropertyInt("AutoPopupYakker", 0, 0, GetCurrentUserName(), true);
				if(nAction == 1) {
					m_pParent->NotifyUser(NT_YAK, "");
				}
				else if(nAction == 2) {
					m_pParent->NotifyUser(NT_YAK, "", false);
				}
			}
		}

		RequeryCurrentList();
		
	}
}

void CMessagerDlg::OnDestroy() 
{
	SetRemotePropertyInt("YakkerEnabled", m_bEnabled, -1, GetCurrentUserName());
	if(m_arUsers != NULL)
		delete [] m_arUsers;

	//Tell any open chat sessions that they have to close now.
	for(int i = 0; i < m_arChatDlgs.GetSize(); i++) {
		if(m_arChatDlgs.GetAt(i) && m_arChatDlgs.GetAt(i)->GetSafeHwnd() && m_arChatDlgs.GetAt(i)->IsWindowVisible()) {
			m_arChatDlgs.GetAt(i)->SendMessage(NXM_CHAT_CLOSE);
		}
		delete m_arChatDlgs.GetAt(i);
		m_arChatDlgs.SetAt(i, NULL);
	}

	//Remove the timer so it stops flashing
	KillTimer(IDT_ICONFLASH_TIMER);
	//Set the icon to disabled (greyed-out)
	m_NID.hIcon = GetIcon(itDisabled);
	Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);
	
	if(m_pMessageHistoryDlg != NULL) {
		delete m_pMessageHistoryDlg;
		m_pMessageHistoryDlg = NULL;
	}

	GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());

	CNxModelessOwnedDialog::OnDestroy();
	OnDisableYakker();
}

LRESULT CMessagerDlg::RefreshUserList(WPARAM wp, LPARAM lp){

	DWORD pNumUsers = *(DWORD*)lp;
	m_nUsers = (int)pNumUsers;
	_IM_USER *ptr = (_IM_USER*)lp;
	ptr = (_IM_USER*)(((DWORD*)ptr) + 1);

	if(m_arUsers != NULL) 
		delete [] m_arUsers;
	m_arUsers = new stUserInfo[pNumUsers];
	for(int i = 0; i < (int)pNumUsers; i++){
		//m_arUsers[i] = *ptr;
		m_arUsers[i].ip = ptr->ip;
		m_arUsers[i].strUserName = ptr->szName;

		ptr = (_IM_USER*)( (char*)ptr + sizeof(DWORD) + strlen(ptr->szName) + 1 );
	}

	UpdateUsers();
	return 0;
}

void CMessagerDlg::OnNewMessage() 
{
	// (c.haag 2006-05-16 15:25) - PLID 20644 - This code has been adjusted to handle
	// modeless message dialogs
	CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
	ASSERT(dlg != NULL);
	if (dlg)
		dlg->PopUpMessage();
	// (c.haag 2006-05-19 10:59) - PLID 20714 - Not necessary
	//RequeryCurrentList();
}

void CMessagerDlg::OnMoreInfo() 
{
	m_bMoreInfo = !m_bMoreInfo;
	//TES 5/3/2007 - PLID 18516 - We don't want to auto-advance to the first unread message
	Refresh(false);
	
}

void CMessagerDlg::OnSelChangedMessageList(long nNewSel) 
{
	NewSelection(nNewSel);
}


void CMessagerDlg::NewSelection(int nRow)
{
	try{
		//First, how many rows are selected?
		long i = 0;
		long p = m_dlCurrent->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		long nSelRow;
		while (p)
		{	i++;
			m_dlCurrent->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			nSelRow = pRow->GetIndex();
			pDisp->Release();
		}
		if(i != 1){//Nothing or multiple is selected
			GetDlgItem(IDC_REPLY)->EnableWindow(FALSE);
			GetDlgItem(IDC_GOTO)->EnableWindow(FALSE);
			GetDlgItem(IDC_FORWARD)->EnableWindow(FALSE);
			GetDlgItem(IDC_FORWARD_MESSAGE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PREVIEW_MESSAGE)->EnableWindow(FALSE);
			m_strDate = "";
			m_strPriority = "";
			m_strRegarding = "";
			m_strSender = "";
			m_strText = "";
			m_strSentTo = "";
			UpdateData(FALSE);
			GetDlgItem(IDC_SHOW_MESSAGE_HISTORY)->EnableWindow(FALSE);
		}
		else{
			//The "cursel" may not actually be in the "sel enum" list.
			m_dlCurrent->CurSel = nSelRow;
			nRow = nSelRow;

			m_strDate = FormatDateTimeForInterface(VarDateTime(m_dlCurrent->GetValue(nRow,mlcDate)), DTF_STRIP_SECONDS, dtoNaturalDatetime);
			m_strPriority = VarString(m_dlCurrent->GetValue(nRow, mlcPriority));
			m_strRegarding = VarString(m_dlCurrent->GetValue(nRow, mlcRegarding));
			if(m_nCurrentView == vtReceived)
				m_strSender = VarString(m_dlCurrent->GetValue(nRow, mlcSenderName), "<Deleted User>");
			else
				m_strSender = GetCurrentUserName();

			// (k.messina 2010-03-18 12:39) - PLID 4876 add meaning to a reply by appending the previous messages
			//this will later be narrowed to the message thread's last 2, as for now we will use the previous two messages
			//that the sender and receiver have between eachother
			// (j.gruber 2010-07-16 14:41) - PLID 39463 - redone to show the whole thread
			//long nSenderID =  VarLong(m_dlCurrent->GetValue(nRow,mlcSenderID),-1);
			//long nRecipientID = VarLong(m_dlCurrent->GetValue(nRow,mlcRecipientID),-1);
			long nMessageGroupID = VarLong(m_dlCurrent->GetValue(nRow,mlcMessageGroupID),-1);
			long nMessageID =VarLong(m_dlCurrent->GetValue(nRow, mlcID));
			long nThreadID = VarLong(m_dlCurrent->GetValue(nRow, mlcMesThreadID));
			//long nLastMessages = GetRemotePropertyInt("YakPreviewPastMessages", 10, 0, GetCurrentUserName(), true);
			CString strPastMessageSql = "";
			_RecordsetPtr rsMsgInfo;
			FieldsPtr MsgFields;
			m_strText = VarString(m_dlCurrent->GetValue(nRow, mlcMessage), "");
			//if(nLastMessages > 0 ){
				
				rsMsgInfo = CreateParamRecordset("SELECT "
					//"SenderPersonT.First + ' ' + SenderPersonT.Last  AS SenderName, Text,MessagesT.MessageID "
					"SenderUsersT.UserName AS SenderName, Text, Min(MessageID) as MessageID "
					"FROM MessagesT "
					"INNER JOIN UsersT AS SenderUsersT ON MessagesT.SenderID = SenderUsersT.PersonID "
					"WHERE ((MessagesT.SenderID = {INT} AND MessagesT.DeletedBySender = 0) "
					"OR (MessagesT.RecipientID = {INT} AND MessagesT.DeletedByRecipient = 0)) "
					"AND MessagesT.MessageID < (SELECT Min(InnerM.MessageID) FROM MessagesT InnerM WHERE InnerM.MessageGroupID = {INT}) AND MessagesT.ThreadID = {INT} "
					" GROUP BY SenderUsersT.UserName, Text, MessageGroupID "
					"ORDER BY MessageID DESC "
					"\r\n" 
					"SELECT UserName FROM MessagesT LEFT JOIN UsersT ON RecipientID = UsersT.PersonID WHERE MessagesT.MessageGroupID = {INT}",
					GetCurrentUserID(), GetCurrentUserID(), nMessageGroupID, nThreadID, nMessageGroupID);
				MsgFields = rsMsgInfo->Fields;

				// (k.messina 2010-03-18 15:28) - PLID 4876 add the previous sender's name and their message (last 2)
				CString strTempPreviousMsg = "";
				while (!rsMsgInfo->eof) {
					strTempPreviousMsg += "\r\n\r\n===";
					strTempPreviousMsg += AdoFldString(MsgFields, "SenderName", ""); //previsou sender's name
					strTempPreviousMsg += " Wrote===\r\n";
					strTempPreviousMsg += AdoFldString(MsgFields, "Text", ""); //previous sender's message/
					//check message length before adding to the preview edit box
					if(strTempPreviousMsg.GetLength() + m_strText.GetLength() > PREVIEW_EDIT_TEXT_LIMIT) {
						break;
					}
					else {
						m_strText += strTempPreviousMsg;
						strTempPreviousMsg = "";
						rsMsgInfo->MoveNext();
					}
				}

				// (k.messina 2010-03-18 15:30) - PLID 4876 moving on to the next record set
				rsMsgInfo = rsMsgInfo->NextRecordset(NULL);
				MsgFields = rsMsgInfo->Fields;
			//}
			//else {
			//	rsMsgInfo = CreateParamRecordset("SELECT UserName "
			//		"FROM MessagesT LEFT JOIN UsersT ON RecipientID = UsersT.PersonID WHERE MessagesT.MessageGroupID = {INT}",
			//		nMessageGroupID);
				//MsgFields = rsMsgInfo->Fields;
			//}

			//Fill in the sent to field
			CString strSentTo;
			// (k.messina 2010-03-18 15:25) - PLID 4876 changed to have a paramertized query and a single recordset for all of the message's info.
			//_RecordsetPtr rsSentTo = CreateRecordset("SELECT UserName FROM MessagesT LEFT JOIN UsersT ON RecipientID = UsersT.PersonID WHERE MessagesT.MessageGroupID = %li", VarLong(m_dlCurrent->GetValue(nRow,mlcMessageGroupID),-1));
			//FieldsPtr SentFields = rsSentTo->Fields;
			
			//TES 11/7/2007 - PLID 27979 - VS2008 - for() loops
			int i = 0;
			//for(i = 0; !rsSentTo->eof; i++){
			for(i = 0; !rsMsgInfo->eof; i++){
				strSentTo += AdoFldString(MsgFields, "UserName", "<Deleted User>");
				strSentTo += ", ";
				//rsSentTo->MoveNext();
				rsMsgInfo->MoveNext();
			}
			long nNumberSentTo = i;

			strSentTo.TrimRight(", ");

			m_strSentTo = strSentTo;

			//This is now being read, so update the data accordingly.
			if(m_nCurrentView == vtReceived){
				if(!VarBool(m_dlCurrent->GetValue(nRow, mlcRead))){
					ExecuteSql("UPDATE MessagesT SET Viewed = 1 WHERE MessagesT.MessageID = %li", VarLong(m_dlCurrent->GetValue(nRow, mlcID)));
					m_dlCurrent->PutValue(m_dlCurrent->CurSel, mlcRead, true);
					//TES 5/3/2007 - PLID 25895 - The caption needs to be updated to reflect the number of unread messages.
					RefreshCaption();
				}
			
				GetDlgItem(IDC_REPLY)->EnableWindow(TRUE);
				GetDlgItem(IDC_FORWARD_MESSAGE)->EnableWindow(TRUE);
				
				GetDlgItem(IDC_SHOW_MESSAGE_HISTORY)->EnableWindow(TRUE);
				GetDlgItem(IDC_PREVIEW_MESSAGE)->EnableWindow(TRUE);
			}
			else{
				GetDlgItem(IDC_REPLY)->EnableWindow(FALSE);
				GetDlgItem(IDC_FORWARD_MESSAGE)->EnableWindow(FALSE);
				// if there is more then one person the message was sent to, we don't know which one they want
				// to view the hisory of so don't let them view the message history of any of them
				if(nNumberSentTo > 1){
				
					GetDlgItem(IDC_SHOW_MESSAGE_HISTORY)->EnableWindow(FALSE);
				}
				else{
					GetDlgItem(IDC_SHOW_MESSAGE_HISTORY)->EnableWindow(TRUE);
				}
			}
			//Now check to see if we can change the icon back
			m_bCheckForUnreadMessages = TRUE;

			//TES 7/7/2009 - PLID 34624 - Moved this code before RefreshTray(), RefreshTray() can now requery the list,
			// therefore it needs to be done last, otherwise variables like nRow may no longer reflect an actual row
			// in the datalist.
			//Is there a related patient
			if(VarLong(m_dlCurrent->GetValue(nRow, mlcRegardingID),-1) == -1)
				GetDlgItem(IDC_GOTO)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_GOTO)->EnableWindow(TRUE);

			if(nRow == 0){
				GetDlgItem(IDC_FORWARD)->EnableWindow(FALSE);
			}
			else{
				GetDlgItem(IDC_FORWARD)->EnableWindow(TRUE);
			}

			RefreshTray();
		}

	}NxCatchAll("Error in CMessagerDlg::NewSelection()");

	UpdateData(FALSE);
}

void CMessagerDlg::OnForward() 
{
	m_dlCurrent->CurSel = m_dlCurrent->CurSel-1;
	//TES 5/3/2007 - PLID 18516 - We don't want to auto-advance to the first unread message.
	Refresh(false);
}

void CMessagerDlg::OnEditingFinishedMessageList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	int nNewVal;
	switch(nCol){
	case mlcRead:
		try{
			nNewVal = VarBool(varNewValue) ? 1 : 0;
			ExecuteSql("UPDATE MessagesT SET Viewed = %li WHERE MessageID = %li", nNewVal, VarLong(m_pMessages->GetValue(nRow, mlcID)));

			//Check for icon change
			m_bCheckForUnreadMessages = TRUE;
			RefreshTray();
			//TES 5/3/2007 - PLID 25895 - The caption needs to be updated to reflect the number of unread messages.
			RefreshCaption();
		}NxCatchAll("Error 1 in CMessageDlg::OnEditingFinishedMessageList()");

		break;
	default:
		//Do Nothing
		break;
	}
}

void CMessagerDlg::LogOnTCP(){
	//Notify everyone that I'm online.
	if(m_bEnabled){//Don't log on if PracYakker is disabled.
		if(m_bNetworkEnabled){
			static struct {
				DWORD ip;
				char szName[256];
			} userinfo;

			userinfo.ip = 0;
			strcpy(userinfo.szName, GetCurrentUserName());
			CClient::Send(PACKET_TYPE_IM_USERLIST, (void*)&userinfo, sizeof(DWORD) + strlen(userinfo.szName) + 1);
			m_bLoggedOn = true;
			m_bCheckForUnreadMessages = TRUE;
			RefreshTray();
		}
	}
}

void CMessagerDlg::UpdateUsers()
{
	// (b.cardillo 2004-07-23 13:21) - PLID 13628 - Rewrote this function to employ the datalist sorting 
	// so we don't have to do it ourselves.

	// Initially we need to make sure the special rows ("Online", "Offline", and the blank separator) are there
	if (m_pUsers->FindByColumn(mulcID, (long)-1, 0, false) == sriNoRow) {
		// They're not there, so add them
		{
			IRowSettingsPtr pOnline = m_pUsers->GetRow(sriGetNewRow);
			pOnline->PutValue(mulcName, "-----Online-----");
			pOnline->PutValue(mulcID, (long)-1);
			pOnline->PutValue(mulcIsValid, (long)0);
			pOnline->PutValue(mulcSection, (long)1);
			pOnline->PutBackColor(0x00FFFFFF);
			pOnline->PutForeColor(0x00000000);
			pOnline->PutBackColorSel(0x00FFFFFF);
			pOnline->PutForeColorSel(0x00000000);
			m_pUsers->AddRow(pOnline);
		}
		{
			IRowSettingsPtr pSpace = m_pUsers->GetRow(sriGetNewRow);
			pSpace->PutValue(mulcName, "");
			pSpace->PutValue(mulcID, (long)-1);
			pSpace->PutValue(mulcIsValid, (long)0);
			pSpace->PutValue(mulcSection, (long)2);
			pSpace->PutBackColor(0x00FFFFFF);
			pSpace->PutForeColor(0x00000000);
			pSpace->PutBackColorSel(0x00FFFFFF);
			pSpace->PutForeColorSel(0x00000000);
			m_pUsers->AddRow(pSpace);
		}
		{
			IRowSettingsPtr pOffline = m_pUsers->GetRow(sriGetNewRow);
			pOffline->PutValue(mulcName, "-----Offline-----");
			pOffline->PutValue(mulcID, (long)-1);
			pOffline->PutValue(mulcIsValid, (long)0);
			pOffline->PutValue(mulcSection, (long)3);
			pOffline->PutBackColor(0x00FFFFFF);
			pOffline->PutForeColor(0x00000000);
			pOffline->PutBackColorSel(0x00FFFFFF);
			pOffline->PutForeColorSel(0x00000000);
			m_pUsers->AddRow(pOffline);
		}
	}

	
	// Now reset all user rows to have be offline (have 2 for their section number), we'll reset the online ones afterward
	{
		for (long p=m_pUsers->GetFirstRowEnum(); p; ) {
			LPDISPATCH lpDisp;
			m_pUsers->GetNextRowEnum(&p, &lpDisp);
			if (lpDisp) {
				IRowSettingsPtr pRow(lpDisp);
				lpDisp->Release();
				if (VarLong(pRow->GetValue(mulcIsValid)) != 0) {
					// TODO: (b.cardillo 2004-07-23 13:25) - We could do this more efficiently if we had a 
					// function IsUserOnline() that would search m_arUsers.  Then here we simply call that 
					// function and set the section to 3 if they're offline, and 1 if they're online.  As 
					// it is right now we have to set them all to offline, and then go back afterward and 
					// set the online ones back to online.
					pRow->PutValue(mulcSection, (long)3);
					pRow->PutBackColor(0x00FFFFFF);
					pRow->PutForeColor(0x00808080);
				}
			}
		}
	}

	// Now put the online ones in the online section
	{
		for (long i=0; i<m_nUsers; i++) {
			// Find this user's row
			int nRow = m_pUsers->FindByColumn(mulcName, (LPCTSTR)m_arUsers[i].strUserName, 0, false);
			if (nRow != sriNoRow) {
				// We found it so change put it in the "online" section
				IRowSettingsPtr pRow = m_pUsers->GetRow(nRow);
				pRow->PutBackColor(0x00FFFFFF);
				pRow->PutForeColor(0x00000000);
				pRow->PutValue(mulcSection, (long)1);
			} else {
				// The row isn't there, but never fear.  Table checker messages will save us.  They will tell 
				// us when a user needs to be added to our list, then this function will be called again.
			}
		}
	}

	// Now that everything's set properly, we call sort to order things appropriately.
	m_pUsers->Sort();

	//Tell all our chat sessions that the user list has changed.
	{
		for(long i=0; i < m_arChatDlgs.GetSize(); i++) {
			if(m_arChatDlgs.GetAt(i) && m_arChatDlgs.GetAt(i)->GetSafeHwnd() && m_arChatDlgs.GetAt(i)->IsWindowVisible()) {
				m_arChatDlgs.GetAt(i)->SendMessage(NXM_REFRESH_USER_LIST, NULL, NULL);
			}
		}
	}


	/* (b.cardillo 2004-07-23 13:20) - Leaving the old way here for reference
	//First let's get rid of the special rows ("Online", "Offline", and the blank separator).
	for(int i = 0; i < 3; i++){
		int nOfflineRow = m_pUsers->FindByColumn(mulcID, (long)-1, 0, false);
		if(nOfflineRow != -1) m_pUsers->RemoveRow(nOfflineRow);
	}

	//Put the "Online" row in
	IRowSettingsPtr pOnline = m_pUsers->GetRow(-1);
	pOnline->PutValue(mulcName, "-----Online-----");
	pOnline->PutValue(mulcID, (long)-1);
	pOnline->PutBackColor(0x00FFFFFF);
	pOnline->PutForeColor(0x00000000);
	pOnline->PutBackColorSel(0x00FFFFFF);
	pOnline->PutForeColorSel(0x00000000);
	m_pUsers->InsertRow(pOnline, 0);

	//Now let's arrange all rows appropriately
	int nOnline = 1;
	for(i = 0; i < m_nUsers; i++){
		int nRow = m_pUsers->FindByColumn(mulcName, (LPCTSTR)m_arUsers[i].strUserName, 0, false); //Find the username
		if(nRow != -1 && nRow >= nOnline){ //If it exists and we haven't already inserted it.
			IRowSettingsPtr pRow = m_pUsers->GetRow(nRow);
			pRow->PutBackColor(0x00FFFFFF);
			pRow->PutForeColor(0x00000000);
			m_pUsers->TakeRowInsert(pRow, 1);
			nOnline++;
		}
	}

	//Put the "offline" rows back in
	IRowSettingsPtr pOffline = m_pUsers->GetRow(-1);
	pOffline->PutValue(mulcName, "-----Offline-----");
	pOffline->PutValue(mulcID, (long)-1);
	pOffline->PutBackColor(0x00FFFFFF);
	pOffline->PutForeColor(0x00000000);
	pOffline->PutBackColorSel(0x00FFFFFF);
	pOffline->PutForeColorSel(0x00000000);
	m_pUsers->InsertRow(pOffline, nOnline);
	IRowSettingsPtr pSpace = m_pUsers->GetRow(-1);
	pSpace->PutValue(mulcName, "");
	pSpace->PutValue(mulcID, (long)-1);
	pSpace->PutBackColorSel(0x00FFFFFF);
	m_pUsers->InsertRow(pSpace, nOnline);

	//Color all the rest of the rows gray.
	for(i = nOnline+2; i < m_pUsers->GetRowCount(); i++){
		((IRowSettingsPtr)m_pUsers->GetRow(i))->PutForeColor(0x00808080);
	}
	m_pUsers->CurSel = -1;

	//Tell all our chat sessions that the user list has changed.
	for(i=0; i < m_arChatDlgs.GetSize(); i++) {
		if(m_arChatDlgs.GetAt(i) && m_arChatDlgs.GetAt(i)->GetSafeHwnd() && m_arChatDlgs.GetAt(i)->IsWindowVisible()) {
			m_arChatDlgs.GetAt(i)->SendMessage(NXM_REFRESH_USER_LIST, NULL, NULL);
		}
	}
	*/
}

LRESULT CMessagerDlg::OnMessageReceived(WPARAM wp, LPARAM lp)
{
	try {
		PlayMessageReceivedSound();
		
		int nAction = GetRemotePropertyInt("AutoPopupYakker", 0, 0, GetCurrentUserName(), true);
		
		//Fill the message in the list.
		_IM_MESSAGE *pimmsg = (_IM_MESSAGE*)lp;

		//Only process messages meant for us.
		if((long)pimmsg->dwSentTo == GetCurrentUserID()) {
		
			//If it's already in the received list, don't go through all this.
			if(m_pMessages->FindByColumn(mlcID, (long)pimmsg->dwMessageID, 0, FALSE) == -1) {
				IRowSettingsPtr pRow = m_pMessages->GetRow(-1);
				pRow->PutValue(mlcSenderID, (long)pimmsg->dwSentBy);
				pRow->PutValue(mlcRead, false);
				pRow->PutValue(mlcID, (long)pimmsg->dwMessageID);
				COleDateTime dt;
				dt.m_dt = pimmsg->date;
				pRow->PutValue(mlcDate, COleVariant(dt));
				CString strUserName;
				strcpy(strUserName.GetBuffer(50), pimmsg->szSentBy);
				strUserName.ReleaseBuffer();
				pRow->PutValue(mlcSenderName, _bstr_t(strUserName));
				CString strText;
				if(pimmsg->bCheckData) {
					_RecordsetPtr rsText = CreateRecordset("SELECT Text FROM MessagesT WHERE MessageID = %li", (long)pimmsg->dwMessageID);
					strText = AdoFldString(rsText, "Text");
				}
				else {
					strcpy(strText.GetBuffer(4000), pimmsg->szMsg);
					strText.ReleaseBuffer();
				}
				pRow->PutValue(mlcMessage, _bstr_t(strText));
				CString strPriority;
				switch(pimmsg->dwPriority) {
				case 1:
					strPriority = "Low";
					break;
				case 2:
					strPriority = "Medium";
					break;
				case 3:
					strPriority = "Urgent";
					break;
				default:
					ASSERT(FALSE);
					break;
				}
				pRow->PutValue(mlcPriority, _bstr_t(strPriority));

				long nRegardingID = (long)pimmsg->dwRegardingID;
				CString strRegardingName = "<None>";
				if(nRegardingID != -1) strRegardingName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", nRegardingID),"<None>");
				pRow->PutValue(mlcRegardingID, nRegardingID);
				pRow->PutValue(mlcRegarding, _bstr_t(strRegardingName));

				pRow->PutValue(mlcMessageGroupID, (long)pimmsg->dwMessageGroupID);
				// (k.messina 2010-03-19 17:03) - PLID 4876
				pRow->PutValue(mlcRecipientID, (long)GetCurrentUserID());

				// (j.gruber 2010-07-16 14:02) - PLID 39463 - ThreadID
				pRow->PutValue(mlcMesThreadID, (long)pimmsg->dwThreadID);

				m_pMessages->AddRow(pRow);

				// (a.walling 2008-08-11 10:39) - PLID 30354 - If they don't have a toolbar, this will be ignored anyway.
				if (GetRemotePropertyInt("YakPreviewMessage", TRUE, 0, GetCurrentUserName(), true)) {
					// (a.walling 2008-06-11 12:46) - PLID 30354 - Popup a notification
					m_NID.uTimeout = 10000;
					m_NID.dwInfoFlags = (strPriority == "Urgent") ? NIIF_WARNING : NIIF_INFO;
					m_NID.uFlags |= NIF_INFO;

					// (e.lally 2009-06-10) PLID 34498 - Check if we can show the regarding info
					BOOL bShowRegarding = TRUE;
					if(nRegardingID == -1 || !(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass))){
						bShowRegarding = FALSE;
					}
					CString strTip;
					strTip.Format("From %s%s:\r\n%s", strUserName, bShowRegarding == FALSE ? "" : FormatString(" regarding %s", strRegardingName), strText);
					if (strTip.GetLength() > 255) {
						strTip = strTip.Left(255 - 3);
						strTip += "...";
					}
					strncpy(m_NID.szInfo, strTip, 255);
					strncpy(m_NID.szInfoTitle, FormatString("Nextech Practice - New %sMessage!", (strPriority == "Urgent") ? "Urgent " : ""), 63);
					Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);
					m_NID.uFlags &= ~NIF_INFO;					
					m_NID.uVersion = NOTIFYICON_VERSION;
				}				

				//TES 5/3/2007 - PLID 25895 - The caption needs to be updated to reflect the number of unread messages.
				RefreshCaption();
			}
			
			m_bCheckForUnreadMessages = TRUE;
			RefreshTray();

			//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard, so there
			// is no point in trying to notify them about new messages.
			// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
			if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent) && CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
				if(nAction == 1) {
					GetMainFrame()->NotifyUser(NT_YAK, "");
				}
				else if(nAction == 2) {
					GetMainFrame()->NotifyUser(NT_YAK, "", false);
				}
			}
		}
		

		
	}NxCatchAll("Error in CMessagerDlg::OnMessageReceived()");

	//TES 11/5/2004 - Now that we got everything in the packet, there's no need to refresh.
	//Refresh();
	return 0;
}

LRESULT CMessagerDlg::OnMessageSent(WPARAM wp, LPARAM lp)
{
	try {
		//Fill the message in the sent list.
		_IM_MESSAGE *pimmsg = (_IM_MESSAGE*)lp;

		// Only process messages sent by us (not that this should ever be false)
		if ((long)pimmsg->dwSentBy == GetCurrentUserID()) {

			//If it's already in the sent list, don't go through all this.
			if(m_pSentList->FindByColumn(mlcID, (long)pimmsg->dwMessageID, 0, FALSE) == -1) {
				IRowSettingsPtr pRow = m_pSentList->GetRow(-1);
				pRow->PutValue(mlcSenderID, (long)pimmsg->dwSentBy);
				pRow->PutValue(mlcRead, false);
				pRow->PutValue(mlcID, (long)pimmsg->dwMessageID);
				COleDateTime dt;
				dt.m_dt = pimmsg->date;
				pRow->PutValue(mlcDate, COleVariant(dt));

				//since we have to make at least one data access, just get all our data accesses done at once
				_RecordsetPtr rsInfo = CreateParamRecordset("SELECT MessagesT.Text, "
					"UsersT.Username AS SentTo, "
					"CASE WHEN PersonT.ID Is Null THEN '' ELSE Last + ', ' + First + ' ' + Middle END AS RegardingName "
					"FROM MessagesT "
					"LEFT JOIN UsersT ON MessagesT.RecipientID = UsersT.PersonID "
					"LEFT JOIN PersonT ON MessagesT.RegardingID = PersonT.ID "
					"WHERE MessageID = {INT}", (long)pimmsg->dwMessageID);

				if(rsInfo->eof) {
					ThrowNxException("Tried to load an invalid sent message.");
				}

				// (j.jones 2010-07-07 11:16) - PLID 39345 - in the sent list,
				// this is the Sent To name, not the Sender name
				CString strUserName = AdoFldString(rsInfo, "SentTo", "<Deleted User>");
				pRow->PutValue(mlcSenderName, _bstr_t(strUserName));

				CString strText;
				if(pimmsg->bCheckData) {
					strText = AdoFldString(rsInfo, "Text", "");
				}
				else {
					strcpy(strText.GetBuffer(4000), pimmsg->szMsg);
					strText.ReleaseBuffer();
				}
				pRow->PutValue(mlcMessage, _bstr_t(strText));
				CString strPriority;
				switch(pimmsg->dwPriority) {
				case 1:
					strPriority = "Low";
					break;
				case 2:
					strPriority = "Medium";
					break;
				case 3:
					strPriority = "Urgent";
					break;
				default:
					ASSERT(FALSE);
					break;
				}
				pRow->PutValue(mlcPriority, _bstr_t(strPriority));

				long nRegardingID = (long)pimmsg->dwRegardingID;
				CString strRegardingName = "<None>";
				if(nRegardingID != -1) {
					strRegardingName = AdoFldString(rsInfo, "RegardingName", "");
				}
				pRow->PutValue(mlcRegardingID, nRegardingID);
				pRow->PutValue(mlcRegarding, _bstr_t(strRegardingName));

				pRow->PutValue(mlcMessageGroupID, (long)pimmsg->dwMessageGroupID);
				// (k.messina 2010-03-19 17:07) - PLID 4876
				pRow->PutValue(mlcRecipientID, (long)pimmsg->dwSentTo);

				// (j.gruber 2010-07-16 15:03) - PLID 39463 - threadID
				pRow->PutValue(mlcMesThreadID, (long)pimmsg->dwThreadID);

				rsInfo->Close();

				long nRow = m_pSentList->AddRow(pRow);
				m_pSentList->CurSel = nRow;
				if (m_dlCurrent == m_pSentList) {
					NewSelection(nRow);
				}
			}
		}
	}NxCatchAll("Error in CMessagerDlg::OnMessageSent()");
	return 0;
}

void CMessagerDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//TODO: It turns out that the dialog doesn't get this message if the datalist has focus,
	//and if the datalist doesn't have focus, we don't want to do anything.  So, we'll figure
	//this out some other time, possibly by adding functionality to the datalist, possibly some 
	//other way.
	//TODO: Check that they have permission to delete messages before deleting messages.
	/*switch(nChar){
	case VK_DELETE:
		//Make sure the message list has focus
		if(::GetFocus() == GetDlgItem(IDC_MESSAGE_LIST)->GetSafeHwnd()){
		}
		break;
	default:
		//Do nothing
		break;
	}*/
	CNxModelessOwnedDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CMessagerDlg::OnSelChangingUsers(long FAR* nNewSel) 
{
	//Keep them from selecting the "Offline" row.
	if(*nNewSel == -1) return;
	if(VarLong(m_pUsers->GetValue(*nNewSel,mulcID)) == -1){
		*nNewSel = m_pUsers->CurSel;
	}
}

void CMessagerDlg::PopupYakker()
{
	// (a.walling 2008-08-11 11:48) - PLID 30354 - This will clear out any notification balloons that might be around
	m_NID.dwInfoFlags = NIIF_INFO;
	m_NID.uFlags |= NIF_INFO;
	strcpy(m_NID.szInfo, "");
	strcpy(m_NID.szInfoTitle, "");
	Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);
	m_NID.uFlags &= ~NIF_INFO;

	OpenYakker();
}

void CMessagerDlg::RefreshColors()
{
	// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL2))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));
	Invalidate(FALSE);
}

void CMessagerDlg::OnClose() 
{
	OnOK();
}

LRESULT CMessagerDlg::OnChatMessage(WPARAM wp, LPARAM lp)
{
	try {
		bool bSent = false;
		for(int i=0; i < m_arChatDlgs.GetSize(); i++) {
			if(m_arChatDlgs.GetAt(i) && m_arChatDlgs.GetAt(i)->GetSafeHwnd() && m_arChatDlgs.GetAt(i)->IsWindowVisible()) {
				if(((_CHAT_MESSAGE*)lp)->dwSessionId == -1 || ((_CHAT_MESSAGE*)lp)->dwSessionId == (DWORD)m_arChatDlgs.GetAt(i)->m_nSessionId) {
					bSent = true;
					m_arChatDlgs.GetAt(i)->SendMessage(NXM_CHAT_MESSAGE, wp, lp);
				}
			}
		}
		if(!bSent) {
			//If this is an invite, add it to our list.
			if( ((_CHAT_MESSAGE*)lp)->type == cmtInvite) {
				bool bFound = false;
				for(int i = 0; i < m_arPendingRequests.GetSize(); i++) {
					if(m_arPendingRequests.GetAt(i).dwSessionId == ((_CHAT_MESSAGE*)lp)->dwSessionId) {
						bFound = true;
					}
				}
				if(!bFound) {
					Invitation invite;
					invite.dwSessionId = ((_CHAT_MESSAGE*)lp)->dwSessionId;
					invite.dwUserId = ((_CHAT_MESSAGE*)lp)->dwSenderId;
					m_arPendingRequests.Add(invite);

					//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard, 
					// so there is no point in trying to notify them about new messages.
					// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
					if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent) && CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
						int nAction = GetRemotePropertyInt("AutoPopupYakker", 0, 0, GetCurrentUserName(), true);
						if(nAction == 1) {//Automatically pop it up.
							//TODO: Send enough information in the nxserver packet so that we don't have to check the database here.
							// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
							if(ReturnsRecordsParam("SELECT MessageID FROM MessagesT WHERE RecipientID = {INT} AND Viewed = 0 AND DeletedByRecipient = 0", GetCurrentUserID())) {
								//This will pop up the yak in such a way as to not mess up what they're doing.
								CMainFrame *pMain = GetMainFrame();
								if(!pMain) {
									//Hmm.  Well, we know there's a main frame because it's our parent.  So...
									pMain = m_pParent;
								}
								pMain->NotifyUser(NT_YAK, "");
							}
							
						}
						else if(nAction == 2) {//Use the notification window.
							// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
							if(ReturnsRecordsParam("SELECT MessageID FROM MessagesT WHERE RecipientID = {INT} AND Viewed = 0 AND DeletedByRecipient = 0", GetCurrentUserID())) {
								//This will pop up the yak in such a way as to not mess up what they're doing.
								CMainFrame *pMain = GetMainFrame();
								if(!pMain) {
									//Hmm.  Well, we know there's a main frame because it's our parent.  So...
									pMain = m_pParent;
								}
								pMain->NotifyUser(NT_YAK, "", false);
							}
						}
					}
				}

				//Flash the tray.
				m_bCheckForUnreadMessages = TRUE;
				RefreshTray();
			}
		}
	}NxCatchAll("Error in CMessagerDlg::OnChatMessage()");
	return 0;
}

void CMessagerDlg::OnChat() 
{
	try {
		CChatSessionDlg *pDlg;
		pDlg = new CChatSessionDlg(this);
		pDlg->m_nSessionId = -1;
		pDlg->Create(pDlg->IDD, GetDesktopWindow());
		m_arChatDlgs.Add(pDlg);
		pDlg->ShowWindow(SW_SHOW);
	}NxCatchAll("Error in CMessagerDlg::OnChat()");
}

void CMessagerDlg::OnRButtonUpMessageList(long nRow, short nCol, long x, long y, long nFlags) 
{
	HandleRButtonUp(nRow, nCol, x, y, nFlags);
}

void CMessagerDlg::OnViewSent() 
{
	// enable the sent datalist
	GetDlgItem(IDC_SENT_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_SENT_LIST)->ShowWindow(SW_SHOW);

	// disable the received datalist
	GetDlgItem(IDC_MESSAGE_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_MESSAGE_LIST)->ShowWindow(SW_HIDE);

	// set the current view
	m_nCurrentView = vtSent;

	// set the current datalist to be the sent list
	m_dlCurrent = m_pSentList;
		
	// set the selection to be using this list now
	RequeryCurrentList();
}

void CMessagerDlg::OnViewReceived() 
{
	// enable the sent datalist
	GetDlgItem(IDC_MESSAGE_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_MESSAGE_LIST)->ShowWindow(SW_SHOW);
	
	// disable the received datalist
	GetDlgItem(IDC_SENT_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_SENT_LIST)->ShowWindow(SW_HIDE);

	// the show message histroy button should be enabled if we are viewing the received
	GetDlgItem(IDC_SHOW_MESSAGE_HISTORY)->EnableWindow(TRUE);
	
	// set the current view
	m_nCurrentView = vtReceived;
	
	// set the current datalist to be the sent list
	m_dlCurrent = m_pMessages;
	
	// set the selection
	RequeryCurrentList();
}

void CMessagerDlg::ShowMessageHistory(const IN long nOtherUserID)
{
	if(!m_pMessageHistoryDlg) {
		m_pMessageHistoryDlg = new CMessageHistoryDlg(this);		
		m_pMessageHistoryDlg->Create(IDD_MESSAGE_HISTORY);
	}
	m_pMessageHistoryDlg->m_pMainYakDlg = this;
	// (a.walling 2010-11-16 16:37) - PLID 41510 - This will cause it to load
	m_pMessageHistoryDlg->SetOtherUserID(nOtherUserID);
	// show the window
	m_pMessageHistoryDlg->ShowWindow(SW_SHOWNORMAL);
	m_pMessageHistoryDlg->SetForegroundWindow();	
}

void CMessagerDlg::OnSelChangedSentList(long nNewSel) 
{
	NewSelection(nNewSel);
}

void CMessagerDlg::OnShowMessageHistoryBtn()
{
	// (k.messina 2010-03-18 14:22) - PLID 4876 moved function to CMessagerDlg::GetOtherUserID();
	//CString sql;
	//long nOtherUserID;
	//if(m_nCurrentView == vtSent){
	//	// we want to get the username of the person we sent the message to if we are viewing the sent
	//	// items
	//	sql.Format("SELECT PersonID FROM UsersT WHERE Username = '%s'", _Q(m_strSentTo));
	//}
	//else{
	//	sql.Format("SELECT PersonID FROM UsersT WHERE Username = '%s'", _Q(m_strSender));
	//}

	//_RecordsetPtr rsUser = CreateRecordset("%s", sql);
	//if(rsUser->eof){
	//	nOtherUserID = 0;
	//}
	//else{
	//	nOtherUserID = AdoFldLong(rsUser, "PersonID");
	//}

	//ShowMessageHistory(nOtherUserID);

	ShowMessageHistory(GetOtherUserID());
}

void CMessagerDlg::OnShowMessageHistory()
{
	try{
		long nOtherUserID = m_pUsers->GetValue(m_pUsers->GetCurSel(), mulcID);
		ShowMessageHistory(nOtherUserID);
	}NxCatchAll("Error in OnShowMessageHistory");
}

void CMessagerDlg::OnRButtonUpSentList(long nRow, short nCol, long x, long y, long nFlags) 
{
	HandleRButtonUp(nRow, nCol, x, y, nFlags);
}

LRESULT CMessagerDlg::OnGotoMessage(WPARAM wParam, LPARAM lParam)
{
	try {
		long nMessageID = (long)wParam;
		//Are we the recipient of this message, or the sender?
		_RecordsetPtr rsMessage = CreateRecordset("SELECT SenderID, RecipientID FROM MessagesT WHERE MessageID = %li", nMessageID);
		if(AdoFldLong(rsMessage, "RecipientID") == GetCurrentUserID()) {
			//OK, we need to be showing our Received list.
			if(!IsDlgButtonChecked(IDC_VIEW_RECEIVED)) {
				CheckRadioButton(IDC_VIEW_RECEIVED, IDC_VIEW_SENT, IDC_VIEW_RECEIVED);
				OnViewReceived();
			}
			m_pMessages->SetSelByColumn(2, nMessageID);
		}
		else if(AdoFldLong(rsMessage, "SenderID") == GetCurrentUserID()) {
			//OK, we need to be showing our Sent list
			if(!IsDlgButtonChecked(IDC_VIEW_SENT)) {
				CheckRadioButton(IDC_VIEW_RECEIVED, IDC_VIEW_SENT, IDC_VIEW_SENT);
				OnViewSent();
			}
			m_pSentList->SetSelByColumn(2, nMessageID);
		}
		else {
			//We can't go to this message.  Sorry.
			return 0;
		}
		//And pop up the yakker as needed.
		OpenYakker();
		
	}NxCatchAll("Error in CMessageDlg::OnGotoMessage()");
	return 0;
}
void CMessagerDlg::OnForwardMessage() 
{
	try{
		if(m_pMessages->CurSel != -1) {
			//Set up a send message dialog with the necessary info
			// (c.haag 2006-05-16 15:25) - PLID 20644 - This code has been adjusted to handle
			// modeless message dialogs
			CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
			ASSERT(dlg != NULL);

			CString strText = VarString(m_pMessages->GetValue(m_pMessages->CurSel, mlcSenderName)) + " wrote:\r\n" + VarString(m_pMessages->GetValue(m_pMessages->CurSel, mlcMessage));
			long nRegardingID = VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcRegardingID),-1);
			// (j.gruber 2010-07-16 14:07) - PLID 39463 - forwards are new threads
			dlg->PopUpMessage(-1, nRegardingID, NULL, NULL, strText);
		}

	}NxCatchAll("Error in CMessagerDlg::OnForward()");
}

void CMessagerDlg::OnDblClickCellUsers(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1) return;

	//Don't show this if this is the "Offline" row.
	if(VarLong(m_pUsers->GetValue(nRowIndex, mulcID)) == -1)
		return;

	m_pUsers->CurSel = nRowIndex;
	OnSendMessage();
}

void CMessagerDlg::OnRequeryFinishedSentList(short nFlags) 
{
	try {
		if(IsWindowVisible()) {
			if(m_nCurrentSentMessage != -1) {
				m_pSentList->SetSelByColumn(2,m_nCurrentSentMessage);
				NewSelection(m_pSentList->CurSel);
			}
			else if(m_pSentList->GetRowCount() > 0) {
				m_nCurrentSentMessage = VarLong(m_pSentList->GetValue(0, mlcID));
				m_pSentList->CurSel = 0;
				NewSelection(0);
			}
			else {
				NewSelection(-1);
			}
		}
		
	}NxCatchAll("Error in OnRequeryFinishedSentList");
}

void CMessagerDlg::RequeryCurrentList()
{
	//if not enabled and we haven't opened manually,
	//don't requery
	if(!m_bEnabled && !m_bListsRequeried)
		return;

	if(m_nCurrentView == vtReceived) {
		m_nCurrentReceivedMessage = m_pMessages->CurSel == -1 ? -1 : VarLong(m_pMessages->GetValue(m_pMessages->CurSel, mlcID), -1);
		m_pMessages->Requery();
	}
	else {
		m_nCurrentSentMessage = m_pSentList->CurSel == -1 ? -1 : VarLong(m_pSentList->GetValue(m_pSentList->CurSel, mlcID), -1);
		m_pSentList->Requery();
	}

	//TES 6/15/2009 - PLID 34624 - If we're disconnected from the network, we need to keep requerying the lists
	if(m_bNetworkEnabled) {
		m_bListsRequeried = TRUE;
	}
}

void CMessagerDlg::Refresh(bool bSelectNewMessage)
{
	try {
		m_bCheckForUnreadMessages = TRUE;
		RefreshTray();
		if(IsWindowVisible()) {
			//TES 5/3/2007 - PLID 18516 - We only want to do this if our caller specified that we should (we used
			// to always advance to the first unread message, which occasionally led to strange behavior).
			if(bSelectNewMessage) {
				_RecordsetPtr rsOldestUnread = CreateRecordset("SELECT TOP 1 MessageID FROM MessagesT WHERE Viewed = 0 AND RecipientID = %li AND DeletedByRecipient = 0 ORDER BY DateSent ASC", GetCurrentUserID());
				//Next, make sure we have an appropriate message selected.
				if(!rsOldestUnread->eof) {
					//Get the oldest unread message
					if(m_nCurrentView == vtSent) {
						//We want to look at a received message.
						CheckRadioButton(IDC_VIEW_RECEIVED, IDC_VIEW_SENT, IDC_VIEW_RECEIVED);
						OnViewReceived();
						//OnViewReceived will call Refresh() (but we are sure we won't hit this line,
						//since it will also set m_nCurrentView to vtReceived), so we're done.
						return;
					}
					//At this point we're confident that the current list is the received list.
					m_nCurrentReceivedMessage = AdoFldLong(rsOldestUnread, "MessageID");
					m_pMessages->SetSelByColumn(2, m_nCurrentReceivedMessage);
				}
			}
			if(m_dlCurrent->CurSel == -1 && m_dlCurrent->GetRowCount() > 0) {
				m_dlCurrent->CurSel = 0;
			}
			NewSelection(m_dlCurrent->CurSel);
			

			//Now, make sure the dialog is the correct size.
			CRect rBottomRight;
			if(m_bMoreInfo) {
				GetDlgItem(IDC_MARKER_LARGE)->GetWindowRect(rBottomRight);
				SetDlgItemText(IDC_MORE_INFO, "<< Less Info");
			}
			else {
				GetDlgItem(IDC_MARKER_SMALL)->GetWindowRect(rBottomRight);
				SetDlgItemText(IDC_MORE_INFO, "More Info >>");
			}
			// (b.cardillo 2015-12-09 15:26) - PLID 67692 - We can't predict border size so use the control 
			// positions within the client to calcualte the full window's *client* rect, then have Windows 
			// convert it to full window rect for us.
			ScreenToClient(rBottomRight);
			rBottomRight.top = 0;
			rBottomRight.left = 0;
			AdjustWindowRectEx(rBottomRight, GetStyle(), FALSE, GetExStyle());
			SetWindowPos(NULL, 0, 0, rBottomRight.Width(), rBottomRight.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
			
			//TES 5/3/2007 - PLID 25895 - Also update the window title.
			RefreshCaption();
		}
	}NxCatchAll("Error in CMessagerDlg::Refresh()");
}


LRESULT CMessagerDlg::OnNxServerConnected(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-05-07 08:31) - PLID 4850
	// This message can be sent from the mainframe when switching users, although we
	// are actually reusing the same NxServer connection.
	m_bNetworkEnabled = true;
	LogOnTCP();

	return 0;
}

LRESULT CMessagerDlg::OnNxServerDisconnected(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2006-10-12 12:32) - PLID 22731 - Flag the Yakker network as being
	// disconnected and refresh the tray to reflect that
	m_bNetworkEnabled = false;
	//TES 6/15/2009 - PLID 34624 - We'll need to requery the lists from now on until we're reconnected, as we won't
	// be getting any messages via NxServer
	m_bListsRequeried = false;
	RefreshTray();
	return 0;
}

void CMessagerDlg::OnTimer(UINT nTimerID)
{
	if (nTimerID == IDT_ICONFLASH_TIMER)
	{
		// c.haag PLID 
		if (m_bEnabled)
		{
			if (m_bWhichFlash)
				m_NID.hIcon = GetIcon(itNewMessages);
			else
				m_NID.hIcon = AfxGetApp()->LoadIcon(IDI_BLANK);

			m_bWhichFlash = !m_bWhichFlash;

			Shell_NotifyIcon(NIM_MODIFY,(NOTIFYICONDATA*)&m_NID);
		}
	}
}

void CMessagerDlg::RefreshTray()
{
	//Only do this if the tray has been initialized.
	if(m_NID.hWnd) {
		//Now, set up the system tray.  Do we have any pending messages?
		IconType it = m_bNetworkEnabled ? itNormal : itDisabled;
		if(m_bCheckForUnreadMessages) {
			// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
			if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent) && CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
				_RecordsetPtr rsOldestUnread = CreateRecordset("SELECT TOP 1 MessageID FROM MessagesT WHERE RecipientID = %li AND Viewed = 0 AND DeletedByRecipient = 0 ORDER BY DateSent Asc", GetCurrentUserID());
				if(!rsOldestUnread->eof) {
					m_bUnreadMessages = TRUE;
					//TES 7/7/2009 - PLID 34624 - We know there are unread messages in the data, are they in the datalist as well?
					long nUnreadRow = m_pMessages->FindByColumn(mlcRead, g_cvarFalse, 0, VARIANT_FALSE);
					if(nUnreadRow == sriNoRow) {
						//TES 7/7/2009 - PLID 34624 - Uh-oh.  NxServer must have dropped a message.  Refresh the list.
						m_pMessages->Requery();
					}
				}
				else {
					m_bUnreadMessages = FALSE;
				}
			}
			else {
				//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard, 
				// so there is no point in trying to notify them about new messages, so just always treat them as 
				// having no new messages.
				m_bUnreadMessages = FALSE;
			}
		}

		m_bCheckForUnreadMessages = FALSE;

		if(m_bUnreadMessages) {
			it = itNewMessages;
		}

		//Do we have any chat requests?
		if(m_arPendingRequests.GetSize() > 0) {
			//Chat requests take precedent.
			it = itChatRequests;
		}
		

		switch(it) {
		case itNormal:
			{
				//First, set the icon.
				m_NID.hIcon = GetIcon(it);
				// Stop the flashing icon
				KillTimer(IDT_ICONFLASH_TIMER);
				//Also, we need to change the tooltip on the icon.
				strcpy(m_NID.szTip, "PracYakker");
				Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);

				//Hide the notification window if it's open.
				m_pParent->UnNotifyUser(NT_YAK);
			}
			break;
		
		case itDisabled:
			{
				//First, set the icon.
				m_NID.hIcon = GetIcon(it);
				// Stop the flashing icon
				KillTimer(IDT_ICONFLASH_TIMER);
				//Also, we need to change the tooltip on the icon.
				strcpy(m_NID.szTip, "PracYakker (disabled)");
				Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);

				//Hide the notification window if it's open.
				m_pParent->UnNotifyUser(NT_YAK);
			}
			break;
			
		case itNewMessages:
			{
				strcpy(m_NID.szTip, "You have new message(s)");
				m_NID.hIcon = GetIcon(itNewMessages);
				Shell_NotifyIcon(NIM_MODIFY,(NOTIFYICONDATA*)&m_NID);
				m_bWhichFlash = false;
				SetTimer(IDT_ICONFLASH_TIMER,495,NULL);
			}
			break;

		case itChatRequests:
			{
				strcpy(m_NID.szTip, "You have been invited to chat.");
				m_NID.hIcon = GetIcon(itNewMessages);
				Shell_NotifyIcon(NIM_MODIFY,(NOTIFYICONDATA*)&m_NID);
				m_bWhichFlash = false;
				SetTimer(IDT_ICONFLASH_TIMER,495,NULL);
			}
			break;
		}
	}
}

LRESULT CMessagerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2003-11-13 09:15) - If this window is deactivated,
	// we want to reset the internal timer that tracks how long
	// a view has been inactive (because it must be inactive this moment).
	// This way, if a client is distracted with managing their todos, they
	// won't have to enter a password to get back to the view they were
	// working in.
	if (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)
	{
		if (GetMainFrame() && GetMainFrame()->GetSafeHwnd())
		{
			CNxTabView* pView = GetMainFrame()->GetActiveView();

			// Reset the timer before the view gets its activation message
			if (pView && pView->GetSafeHwnd()) pView->ResetSecurityTimer();
		}
	}
	
	return CNxModelessOwnedDialog::WindowProc(message, wParam, lParam);
}

HICON CMessagerDlg::GetIcon(IconType it)
{
	switch(it) {
	case itNormal:
		{
			if(!m_hNormalIcon) {
				_variant_t varIcon = GetRemotePropertyImage("YakIconNormal", GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true),GetCurrentUserName(),true);
				if(varIcon.vt == VT_NULL || varIcon.vt == VT_EMPTY) {
					if(GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true)) {
						m_hNormalIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
						m_bCanDestroyNormal = false;
					}
					else {
						m_hNormalIcon = AfxGetApp()->LoadIcon(IDI_YAK);
						m_bCanDestroyNormal = false;
					}
				}
				else {
					CDC *pDC = GetDC();
					HICON hRet = GetIconFromVariant(pDC, varIcon);
					ReleaseDC(pDC);
					if(hRet) {
						m_hNormalIcon = hRet;
						m_bCanDestroyNormal = true;
					}
					else {
						if(GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true)) {
							m_hNormalIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
							m_bCanDestroyNormal = false;
						}
						else {
							m_hNormalIcon = AfxGetApp()->LoadIcon(IDI_YAK);
							m_bCanDestroyNormal = false;
						}
					}
				}
			}
			return m_hNormalIcon;
		}
		break;
	case itDisabled:
		{
			if(!m_hDisabledIcon) {
				_variant_t varIcon = GetRemotePropertyImage("YakIconDisabled", GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true),GetCurrentUserName(),true);
				if(varIcon.vt == VT_NULL || varIcon.vt == VT_EMPTY) {
					if(GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true)) {
						m_hDisabledIcon = AfxGetApp()->LoadIcon(IDI_GRAY_BALL);
						m_bCanDestroyDisabled = false;
					}
					else {
						m_hDisabledIcon = AfxGetApp()->LoadIcon(IDI_YAKDISABLED);
						m_bCanDestroyDisabled = false;
					}
				}
				else {
					CDC *pDC = GetDC();
					HICON hRet = GetIconFromVariant(pDC, varIcon);
					ReleaseDC(pDC);
					if(hRet) {
						m_hDisabledIcon = hRet;
						m_bCanDestroyDisabled = true;
					}
					else {
						if(GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true)) {
							m_hDisabledIcon = AfxGetApp()->LoadIcon(IDI_GRAY_BALL);
							m_bCanDestroyDisabled = false;
						}
						else {
							m_hDisabledIcon = AfxGetApp()->LoadIcon(IDI_YAKDISABLED);
							m_bCanDestroyDisabled = false;
						}
					}
				}
			}
			return m_hDisabledIcon;
		}
		break;
	case itNewMessages:
		{
			if(!m_hFlashingIcon) {
				_variant_t varIcon = GetRemotePropertyImage("YakIconNew", GetRemotePropertyInt("YakTheme", GetRemotePropertyInt("ClassicYakIcon",1,0,GetCurrentUserName(),false),0,GetCurrentUserName(),true),GetCurrentUserName(),true);
				if(varIcon.vt == VT_NULL || varIcon.vt == VT_EMPTY) {
					m_hFlashingIcon = AfxGetApp()->LoadIcon(IDI_YAKNEW);
					m_bCanDestroyFlashing = false;
				}
				else {
					CDC *pDC = GetDC();
					HICON hRet = GetIconFromVariant(pDC, varIcon);
					ReleaseDC(pDC);
					if(hRet) {
						m_hFlashingIcon = hRet;
						m_bCanDestroyFlashing = true;
					}
					else {
						m_hFlashingIcon = AfxGetApp()->LoadIcon(IDI_YAKNEW);
						m_bCanDestroyFlashing = false;
					}
				}
			}
			return m_hFlashingIcon;
		}
		break;
	case itChatRequests:
		if(!m_hChatIcon) {
			m_hChatIcon = AfxGetApp()->LoadIcon(IDI_BLUE_QUESTION);
			m_bCanDestroyChat = false;
		}
		return m_hChatIcon;
		break;
	}
	ASSERT(FALSE);
	return NULL;
}

LRESULT CMessagerDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
		case NetUtils::Coordinators:
			try {
				m_pUsers->Requery();
				//The OnRequeryFinished will handle all the Online/Offline stuff.
			} NxCatchAll("Error in CMessagerDlg::OnTableChanged:Coordinators");
			break;
		// (b.cardillo 2006-07-05 10:43) - PLID 20948 - After discussing with t.schneider et al, we concluded 
		// that removing the entire "case NetUtils::ConfigRT" from here was an appropriate solution.  Our icons 
		// are now only refreshed by CMainFrame after the user clicks OK on preferences.
		}
	} NxCatchAll("Error in CMessagerDlg::OnTableChanged");

	return 0;
}

void CMessagerDlg::OnRequeryFinishedUsers(short nFlags) 
{
	try {
		if(m_bLoggedOn) {
			UpdateUsers();
		}
	} NxCatchAll("Error updating users!");
}

void CMessagerDlg::RefreshIcons()
{
	try {
		// Clear all our existing icons
		if(m_hNormalIcon) {
			if(m_bCanDestroyNormal) DestroyIcon(m_hNormalIcon);
			m_hNormalIcon = NULL;
		}
		if(m_hDisabledIcon) {
			if(m_bCanDestroyDisabled) DestroyIcon(m_hDisabledIcon);
			m_hDisabledIcon = NULL;
		}
		if(m_hFlashingIcon) {
			if(m_bCanDestroyFlashing) DestroyIcon(m_hFlashingIcon);
			m_hFlashingIcon = NULL;
		}
		if(m_hChatIcon) {
			if(m_bCanDestroyChat) DestroyIcon(m_hChatIcon);
			m_hChatIcon = NULL;
		}
		
		// Now update the current tray icon if the tray has been initialized.
		if (m_NID.hWnd) {
			// Decide on the correct icon based on the logical priority (1. pending chat 
			// requests, 2. pending unread messages, 3. yakker disabled, 4. normal)
			IconType it;
			if (m_arPendingRequests.GetSize() > 0) {
				it = itChatRequests;
			} else if (m_bUnreadMessages) {
				it = itNewMessages;
			} else if (!m_bNetworkEnabled) {
				it = itDisabled;
			} else {
				it = itNormal;
			}

			// Now set the icon if it's a stable icon (if it's a flashing icon, there's 
			// no need because it's ABOUT to flash!).  Also, even if we are setting icon 
			// here, we know we're not CHANGING it, so there's no need to set the tooltip 
			// or anything else.
			switch(it) {
			case itNormal:
				m_NID.hIcon = GetIcon(it);
				Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);
				break;
			case itDisabled:
				m_NID.hIcon = GetIcon(it);
				Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&m_NID);
				break;
			case itNewMessages:
			case itChatRequests:
				// Flashing, so do nothing
				break;
			}
		}
	} NxCatchAll("CMessagerDlg::RefreshIcons");
}

void CMessagerDlg::OnYakkerPrefs() 
{
		try {
			//TES 6/23/2009 - PLID 34155 - Changed the View permission to Read
			if (CheckCurrentUserPermissions(bioPreferences, sptRead)) {
				ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piPracYakker);

				// (c.haag 2006-05-16 12:17) - PLID 20621 - Update the background color
				RefreshColors();
				ASSERT(GetMainFrame()->m_pdlgSendYakMessage != NULL);
				GetMainFrame()->m_pdlgSendYakMessage->RefreshColors();

				// (b.cardillo 2006-07-05 15:53) - PLID 20948 - In case the icons have changed for this user, refresh them.
				RefreshIcons();

				// (c.haag 2005-01-26 14:12) - PLID 15415 - Flush the remote property cache because one or
				// more preferences may have changed, which means ConfigRT changed without intervention from
				// the cache.
				// (c.haag 2005-11-07 16:27) - PLID 16595 - This is now obselete; but we still flush in regular intervals
				//FlushRemotePropertyCache();
				SetShowAreaCode(GetRemotePropertyInt("UseAreaCode", 1, 0, "<None>", true) == 0 ? false : true);//This may have changed in ConfigRT.
				GetMainFrame()->UpdateAllViews();

				// CAH 5/16/03 - There is one preference that affects the query used in
				// the patient dropdown in the ResEntryDlg. To be safe, we need to update
				// that dropdown.

				// This probably won't be the only case where we will have to do this in
				// the future. Maybe we should make a NXM_ONPREFERENCESCHANGED message
				// to broadcast to all the open views....perhaps the CNxTabView class could
				// have an overrideable function that calls UpdateView() by default upon
				// reception of the message.
				CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
				if (pView && pView->GetResEntry())
					pView->GetResEntry()->RequeryPersons();
			}
		}NxCatchAll("Error displaying preferences.");
}

void CMessagerDlg::OnPreviewMessage() 
{
	try {
		if(m_dlCurrent->CurSel == -1) return;
		CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(490)];
		_RecordsetPtr rsMessage = CreateRecordset("SELECT MessagesT.MessageGroupID FROM MessagesT WHERE MessageID = %li", 
			VarLong(m_dlCurrent->GetValue(m_dlCurrent->CurSel, mlcID)));
		infReport.nExtraID = AdoFldLong(rsMessage, "MessageGroupID");
		OnOK();

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, (CWnd *)this, "Individual PracYakker Message");

	}NxCatchAll("Error in CMessagerDlg::OnPreviewMessage()");
}

void CMessagerDlg::HandleRButtonUp(long nRow, short nCol, long x, long y, long nFlags) 
{
	int nSelectedRows = 0;
	bool bClickedRowSelected = false;
	long p = m_dlCurrent->GetFirstSelEnum();
	LPDISPATCH pDisp = NULL;
	while (p)
	{	
		nSelectedRows++;
		m_dlCurrent->GetNextSelEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		if(pRow->GetIndex() == nRow) bClickedRowSelected = true;
		pDisp->Release();
	}
	if(nSelectedRows > 1 && bClickedRowSelected) {
		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MESSAGE_DELETE, GetStringOfResource(IDS_DELETE_MESSAGE));
		
		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	}

	else {
		m_dlCurrent->CurSel = nRow;
		if(m_dlCurrent == m_pMessages)
			OnSelChangedMessageList(nRow);
		else
			OnSelChangedSentList(nRow);

		if(nRow == -1) return;

		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MESSAGE_DELETE, GetStringOfResource(IDS_DELETE_MESSAGE));
		

		//check to see if is a patient message
		long nRegardingID = VarLong(m_dlCurrent->GetValue(nRow, mlcRegardingID),-1);

		if (nRegardingID != -1) {
			pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MESSAGE_ADD_NOTE, "Add to Patient Note");
		}
  
		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	}
	
}

void CMessagerDlg::PlayMessageReceivedSound()
{
	//TES 12/19/2008 - PLID 32525 - They're not allowed to open the Yakker if they have Scheduler Standard, so there
	// is no point in trying to notify them about new messages.
	// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
	if (!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent) || !CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
		return;
	}

	//Ensure that the registry keys exist.
	if(!NxRegUtils::DoesKeyExist("HKEY_CURRENT_USER\\AppEvents\\EventLabels\\PracYakkerMessage")) {
		NxRegUtils::CreateKey("HKEY_CURRENT_USER\\AppEvents\\EventLabels\\PracYakkerMessage\\");
		NxRegUtils::WriteString("HKEY_CURRENT_USER\\AppEvents\\EventLabels\\PracYakkerMessage\\", "PracYakker Message Received", false);
	}
	if(!NxRegUtils::DoesKeyExist("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\PracYakker\\PracYakkerMessage\\.Current")) {
		NxRegUtils::CreateKey("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\PracYakker\\PracYakkerMessage\\.Current\\");
		if(GetRemotePropertyInt("YakkerBeep", 0, 0, GetCurrentUserName(), true)) {
			NxRegUtils::WriteString("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\PracYakker\\PracYakkerMessage\\.Current\\", 
				NxRegUtils::ReadString("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\.Default\\SystemExclamation\\.Current\\", ""), false);
		}
	}

	CString strSound = NxRegUtils::ReadString("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\PracYakker\\PracYakkerMessage\\.Current\\", "");
	if(strSound != "") {
		PlaySound(strSound, NULL, SND_FILENAME);
	}
}

void CMessagerDlg::RefreshCaption()
{
	//TES 5/3/2007 - PLID 25895 - The caption should reflect the logged-in user, as well as the number of unread messages
	// waiting to be read.
	int nUnread = 0;
	if(!m_bListsRequeried) {
		//We don't know how many messages are unread yet, so just assume there aren't any, once the lists are requeried
		// we'll update the caption again.
		nUnread = 0;
	}
	else {
		//TES 12/11/2007 - PLID 28331 - This way of looping is orders of magnitude faster than the old way, particularly
		// for lists with a lot of rows.  I was able to drop the time for a list with ~24,000 rows from 11096 ms to 63.
		long p = m_pMessages->GetFirstRowEnum();
		while (p) {
			IRowSettingsPtr pRow;
			// Get the row object and move to the next enum
			{
				LPDISPATCH lpDisp = NULL;
				m_pMessages->GetNextRowEnum(&p, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
			}
			if(!VarBool(pRow->GetValue(mlcRead))) {
				nUnread++;
			}
		}
		/*for(int i = 0; i < m_pMessages->GetRowCount(); i++) {
			if(!VarBool(m_pMessages->GetValue(i, mlcRead))) {
				nUnread++;
			}
		}*/
	}

	CString strCaption;
	if(nUnread > 0) {
		//TES 5/3/2007 - PLID 25895 - Tell the user how many unread messages there are.
		strCaption.Format("PracYakker - %s [%i unread message%s waiting]", GetCurrentUserName(), nUnread, nUnread == 1 ? "" : "s");
	}
	else {
		//TES 5/3/2007 - PLID 25895 - Just use the default caption.
		strCaption.Format("PracYakker - %s", GetCurrentUserName());
	}
	SetWindowText(strCaption);
}

void CMessagerDlg::MessagerDlgDisableHotkeys()
{
	// (z.manning, 06/20/2007) - PLID 26390 - Only disable the hotkeys if this dialog hasn't already done so.
	// Otherwise we may end up disabling them multiple times but only enabling them once which will
	// prevent them from working.
	if(!m_bMessagerDlgHasDisabledHotkeys) {
		if(GetMainFrame()) {
			GetMainFrame()->DisableHotKeys();
		}
	}
	m_bMessagerDlgHasDisabledHotkeys = TRUE;
}

void CMessagerDlg::MessagerDlgEnableHotkeys()
{
	// (z.manning, 06/20/2007) - PLID 26390 - Re-enable hotkeys.
	if(GetMainFrame()) {
		GetMainFrame()->EnableHotKeys();
	}
	m_bMessagerDlgHasDisabledHotkeys = FALSE;
}

// (k.messina 2010-03-18 14:20) - PLID 4876 return the other user personID
long CMessagerDlg::GetOtherUserID()
{
	long nOtherUserID;

	// (a.walling 2010-11-16 16:37) - PLID 41510 - Parameterized this thing
	CString strOtherUser;
	if(m_nCurrentView == vtSent){
		// we want to get the username of the person we sent the message to if we are viewing the sent
		// items
		strOtherUser = m_strSentTo;
	}
	else{
		strOtherUser = m_strSender;
	}

	_RecordsetPtr rsUser = CreateParamRecordset("SELECT PersonID FROM UsersT WHERE Username = {STRING}", strOtherUser);
	if(rsUser->eof){
		nOtherUserID = -1;
	}
	else{
		nOtherUserID = AdoFldLong(rsUser, "PersonID");
	}

	return nOtherUserID;
}