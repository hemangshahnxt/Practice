// SendMessageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SendMessageDlg.h"
#include "GlobalDataUtils.h"
#include "Client.h"
#include "MessagerDlg.h"
#include "nxmessagedef.h"
#include "YakGroupSelect.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.


// (c.haag 2006-05-17 15:53) - PLID 20644 ///////////////////////////////////
// Whenever a table checker message comes in, we need to update the regarding
// patient dropdown. I wrote this function in hopes it will become a general
// patient dropdown table checker handler.
//
// This function assumes column zero is the patient ID
//
// When this becomes global, don't forget to make the MsgBox generic
//
void HandlePatientDropdownTableChange(_DNxDataListPtr& dlPatient, long nPatientID)
{
	try {
		if (dlPatient) {
			CString strSql;
			CString strFrom = (LPCTSTR)dlPatient->FromClause;
			CString strWhere = (LPCTSTR)dlPatient->WhereClause;
			CString strSelect;
			long nPatientRow = dlPatient->FindByColumn(0,nPatientID,0,FALSE);
			long i;

			//
			// First, see if the patient belongs in the dropdown. We'll build a query with
			// all the relevant datalist fields so that, if there are records, we can update
			// the datalist on the spot.
			//
			for (i=0; i < dlPatient->ColumnCount; i++) {
				IColumnSettingsPtr pCol = dlPatient->GetColumn((short)i);
				strSelect += (LPCTSTR)pCol->FieldName;
				strSelect += ", ";
			}
			strSelect.TrimRight(", ");
			if (strWhere.IsEmpty()) strWhere = "1=1";

			/*strSql.Format("SELECT %s FROM %s WHERE PersonT.ID = %d AND (%s)",
				strSelect, strFrom, nPatientID, strWhere);*/
			// (a.walling 2011-07-29 13:15) - PLID 44788 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT {CONST_STR} FROM {CONST_STR} WHERE PersonT.ID = {INT} AND ({CONST_STR})", strSelect, strFrom, nPatientID, strWhere);
			if (prs->eof) {
				//
				// If we get here, the patient does not belong. So, remove it, reset the selection,
				// if necessary, and quit. This is consistent with the logic of
				// CMainFrame::UnselectDeletedPatientFromToolbar
				//
				if (nPatientRow > -1) {
					long nNewSel = -1;
					if (nPatientRow == dlPatient->CurSel) {				
						MsgBox("The actively selected regarding patient in the PracYakker has been changed or deleted by another user. Practice will select the next patient in the PracYakker message list.");
						nNewSel = nPatientRow;
						if (nNewSel > 0) {//keeps from getting error by deleting last patient
							nNewSel--;
						} else {
							nNewSel = 0;
						}
					}
					dlPatient->RemoveRow(nPatientRow);
					if (-1 != nNewSel) {
						dlPatient->CurSel = nNewSel;
					}
				} else {
					// If we get here, the row doesn't exist anyway, so do nothing
				}
			} else {
				//
				// If we get here, the patient should be in the list. Make sure the
				// patient is there and properly updated
				//
				FieldsPtr flds = prs->Fields;
				IRowSettingsPtr pRow = dlPatient->GetRow(nPatientRow);

				if (flds->Count != dlPatient->ColumnCount) {
					AfxThrowNxException("The dropdown and data have a different number of fields!");
				}
				for (i=0; i < flds->Count; i++) {
					_variant_t v = flds->Item[i]->Value;
					pRow->Value[(short)i] = v;
				}
				if (-1 == nPatientRow) {
					dlPatient->AddRow(pRow);
				}
			}
		}
	}
	NxCatchAll("Error updating patient dropdown");
}

void CSendMessageDlg::HandleTableChange(long nPatientID)
{
	//HandlePatientDropdownTableChange(m_pPatients, nPatientID);
	
	// (a.walling 2011-07-29 13:15) - PLID 44788 - If we are dropped down, handle the classic way
	if (m_pPatients->GetDropDownState()) {
		HandlePatientDropdownTableChange(m_pPatients, nPatientID);
		return;
	}

	// (a.walling 2011-07-29 13:15) - PLID 44788
	if (m_pPatients->IsRequerying()) return;
	
	// Otherwise just update the combo text and clear the list to reload on next drop down
	if (!m_pPatients->GetIsComboBoxTextInUse()) {
		long nPatientListSelectID = (m_nRegardingID == -1) ? GetActivePatientID() : m_nRegardingID;
		m_pPatients->ComboBoxText = (LPCTSTR)GetExistingPatientName(nPatientListSelectID);
		//(e.lally 2012-01-05) PLID 47334 - Update the regardingID here, we check that the box it checked before creating the message record
		m_nRegardingID = nPatientListSelectID;
	}

	if (m_pPatients->GetRowCount()) {
		m_pPatients->Clear();
	}
}


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog


CSendMessageDlg::CSendMessageDlg(CWnd* pParent)
	: CNxModelessOwnedDialog(CSendMessageDlg::IDD, pParent),
	m_ebMessage(NXM_CTRL_ENTER)
	, dlgSelectRecipients(this)
	, dlgGroupSelect(this)
{
	m_nRegardingID = -1;
	//{{AFX_DATA_INIT(CSendMessageDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT
	m_bSendMessageDlgHasDisabledHotkeys = FALSE;
}


void CSendMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxModelessOwnedDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMessageDlg)
	DDX_Control(pDX, IDC_SEND_TO, m_btnSendTo);
	DDX_Control(pDX, IDC_SEND_TO_GROUP, m_btnSendToGroup);
	DDX_Control(pDX, IDC_REGARDING, m_btnRegarding);
	DDX_Control(pDX, IDC_LIST_GROUPS, m_nxlGroupsLabel);
	DDX_Control(pDX, IDC_LIST_USERS, m_nxlUserLabel);
	DDX_Control(pDX, IDC_TEXT, m_ebMessage);
	DDX_Control(pDX, IDC_PRIORITY, m_cbPriority);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_Control(pDX, IDOK, m_btnSend);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendMessageDlg, CNxModelessOwnedDialog)
	//{{AFX_MSG_MAP(CSendMessageDlg)
	ON_BN_CLICKED(IDC_SEND_TO, OnSendTo)
	ON_BN_CLICKED(IDC_REGARDING, OnRegarding)
	ON_BN_CLICKED(IDC_SEND_TO_GROUP, OnSendGroup)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EDIT_GROUPS, OnEditGroups)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_SEND_TO_GROUP, OnSendGroup)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg message handlers
#define ID_DROPDOWN_USERS	1000
#define ID_CLEAR_REGARDING_LIST 1001 // (a.walling 2011-07-29 13:15) - PLID 44788
BOOL CSendMessageDlg::OnInitDialog() 
{
	// (e.lally 2009-06-11) PLID 34498 - Added try/catch
	try {
		
		CNxModelessOwnedDialog::OnInitDialog();

		// (a.walling 2008-06-09 16:48) - PLID 22049 - This lacks a taskbar icon style
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			ModifyStyleEx(0, WS_EX_APPWINDOW);
		}

		m_btnSend.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// MSC - 2/26/04 - we need to hide these two static lists at first.  They are used to display the users
		// or groups if there are multiple ones selected
		ShowDlgItem(IDC_LIST_USERS, SW_HIDE);
		ShowDlgItem(IDC_LIST_GROUPS, SW_HIDE);

		// disable the edit groups button until they are choosing a group
		GetDlgItem(IDC_EDIT_GROUPS)->EnableWindow(FALSE);

		m_pUsers = BindNxDataListCtrl(IDC_COMBO_USERS);
		m_pGroups = BindNxDataListCtrl(IDC_COMBO_GROUPS);
		//(e.lally 2009-06-10) PLID 34498 - Don't requery just yet
		m_pPatients = BindNxDataListCtrl(IDC_COMBO_PATIENTS, false);
		m_pPatients->SnapToVertically = VARIANT_FALSE;
		// (e.lally 2009-06-10) PLID 34498 - Get permission for viewing patients module, hide the regarding controls
		if (GetCurrentUserPermissions(bioPatientsModule) & (sptView | sptViewWithPass)){
			// (a.walling 2011-07-29 13:15) - PLID 44788
			//m_pPatients->Requery();
		}
		else{
			//Not allowed to see patient list, hide it
			GetDlgItem(IDC_COMBO_PATIENTS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REGARDING)->ShowWindow(SW_HIDE);
		}

		m_ebMessage.SetLimitText(3500);

		// MSC - 2/26/04 - setup the labels
		m_nxlUserLabel.SetText("");
		m_nxlUserLabel.SetType(dtsHyperlink);
		m_nxlUserLabel.SetColor(0x00FF8080);

		// MSC - 2/26/04 - setup the labels
		m_nxlGroupsLabel.SetText("");
		m_nxlGroupsLabel.SetType(dtsHyperlink);
		m_nxlGroupsLabel.SetColor(0x00FF8080);

		// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
		RefreshColors();

	} NxCatchAll(__FUNCTION__);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.jones 2008-06-02 12:04) - PLID 30219 - added ability to send in a
// default group ID
// (a.walling 2008-06-11 11:35) - PLID 22049 - added ability to set priority
// (j.gruber 2010-07-16 13:19) - PLID 39463 - added threadID
void CSendMessageDlg::PopUpMessage(long nThreadID /*=-1*/, 
		long nRegardingID /*= -1*/, 		
		CDWordArray* padwRecipients /*= NULL*/,
		CStringArray* pastrRecipients /*= NULL*/,
		CString strDefaultText /*= ""*/,
		long nDefaultGroupID /*= -1*/,
		CString strPriority /* = "Medium"*/)
{
	// (e.lally 2009-06-11) PLID 34498 - Added try/catch
	try {
		BOOL bCenterWindow = (IsWindowVisible()) ? FALSE : TRUE;

		//
		// Assign default values
		//
		m_nRegardingID = nRegardingID;
		m_adwRecipients.RemoveAll();
		// (a.walling 2008-10-06 10:02) - PLID 31592
		m_astrRecipients.RemoveAll();

		m_dwRecipientsAry.RemoveAll();
		m_strRecipientsAry.RemoveAll();
		m_dwGroupRecipAry.RemoveAll();
		m_strGroupRecipAry.RemoveAll();



		dlgSelectRecipients.m_dwRecipientsAry.RemoveAll();
		dlgSelectRecipients.m_strRecipientsAry.RemoveAll();

		dlgGroupSelect.m_dwGroupsAry.RemoveAll();
		dlgGroupSelect.m_strGroupsAry.RemoveAll();

		// (j.gruber 2010-07-16 13:21) - PLID 39463 - set the thread variable
		m_nThreadID = nThreadID;

		if (padwRecipients) {
			for (int i=0; i < padwRecipients->GetSize(); i++) {
				m_adwRecipients.Add(padwRecipients->GetAt(i));
			}
		}
		if (pastrRecipients) {
			for (int i=0; i < pastrRecipients->GetSize(); i++) {
				m_astrRecipients.Add(pastrRecipients->GetAt(i));
			}
		}

		// (j.jones 2008-06-02 12:07) - PLID 30219 - load up the default group
		if(nDefaultGroupID != -1) {
			m_dwGroupRecipAry.Add(nDefaultGroupID);
		}

		m_strText = strDefaultText;
		SetDlgItemText(IDC_TEXT, strDefaultText);

		// (e.lally 2009-06-10) PLID 34498 - get Permissions - view patients module
		if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)){
			GetDlgItem(IDC_COMBO_PATIENTS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_REGARDING)->ShowWindow(SW_SHOW);
			//It is possible that we restricted the patient list from loading previously. Only requery if there
			//	are no records and it is not requerying already. 

			// (a.walling 2011-07-29 13:15) - PLID 44788
			/*if(m_pPatients->IsRequerying() == VARIANT_FALSE && m_pPatients->GetRowCount() == 0){
				m_pPatients->Requery();
			}*/
		}
		else{
			//Not allowed to see patient list, hide it
			GetDlgItem(IDC_COMBO_PATIENTS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REGARDING)->ShowWindow(SW_HIDE);
			//Make sure we don't somehow have a row selected in the background
			m_pPatients->CurSel = sriNoRow;
		}

		//
		// Load the values onto the form
		//

		//TES 5/15/2006 - We want to use TrySetSel, because it's asynchronous, but show the correct name in themeantime.
		// (a.walling 2011-07-29 13:15) - PLID 44788
		long nPatientListSelectID = (m_nRegardingID == -1) ? GetActivePatientID() : m_nRegardingID;
		if(m_pPatients->TrySetSelByColumn(0, nPatientListSelectID) < 0) {			
			m_pPatients->ComboBoxText = (LPCTSTR)GetExistingPatientName(nPatientListSelectID);
		}

		if(m_nRegardingID != -1){
			CheckDlgButton(IDC_REGARDING, BST_CHECKED);
			GetDlgItem(IDC_COMBO_PATIENTS)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_COMBO_PATIENTS)->EnableWindow(FALSE);
			CheckDlgButton(IDC_REGARDING, BST_UNCHECKED);
		}

		//(e.lally 2012-01-05) PLID 47334 - Update the regardingID here, we check that the box it checked before creating the message record
		//	Do this after we've determined if the patient list is to be enabled.
		m_nRegardingID = nPatientListSelectID;
		
		// (a.walling 2008-06-11 11:35) - PLID 22049 - Set the priority
		int nResult = m_cbPriority.SelectString(0,strPriority);
		ASSERT(nResult != CB_ERR);		
		
		// (j.jones 2008-06-02 12:08) - PLID 30219 - display the pre-selected group, if any (there can be only one)
		if(m_dwGroupRecipAry.GetSize() == 1) {

			CheckDlgButton(IDC_SEND_TO, BST_UNCHECKED);
			CheckDlgButton(IDC_SEND_TO_GROUP, BST_CHECKED);

			OnSendGroup();
			m_pGroups->SetSelByColumn(0, (long)m_dwGroupRecipAry[0]);
			OnSelChosenComboGroups(m_pGroups->CurSel);
			GetDlgItem(IDC_TEXT)->SetFocus();
		}
		else {

			//there can't be default groups and users, so put them in separate logic paths		

			CheckDlgButton(IDC_SEND_TO, BST_CHECKED);
			CheckDlgButton(IDC_SEND_TO_GROUP, BST_UNCHECKED);
			GetDlgItem(IDC_COMBO_GROUPS)->EnableWindow(FALSE);

			OnSendTo();

			m_pGroups->PutCurSel(-1);

			if(m_dwGroupRecipAry.GetSize() > 1) {
				//not currently supported
				ASSERT(FALSE);
			}

			if(m_adwRecipients.GetSize() == 0) {
				SetTimer(ID_DROPDOWN_USERS, 0, NULL);
			}
			else if (m_adwRecipients.GetSize() == 1) {
				m_pUsers->SetSelByColumn(0, (long)m_adwRecipients[0]);
				OnSelChosenComboUsers(m_pUsers->CurSel);
				GetDlgItem(IDC_TEXT)->SetFocus();
			}
			// (c.haag 2004-06-23 13:38) - If we have multiple recipients,
			// select the multiple users row
			else if (m_adwRecipients.GetSize() > 1)
			{
				// If we have recipients, we need to have names, too
				ASSERT(m_adwRecipients.GetSize() == m_astrRecipients.GetSize());
				m_pUsers->FindByColumn(1, _bstr_t(MULTIPLE_USERS), 0, TRUE);
				m_dwRecipientsAry.RemoveAll();
				m_strRecipientsAry.RemoveAll();
				for (long i=0; i < m_adwRecipients.GetSize(); i++)
				{
					m_dwRecipientsAry.Add(m_adwRecipients[i]);
					m_strRecipientsAry.Add(m_astrRecipients[i]);
					dlgSelectRecipients.m_dwRecipientsAry.Add(m_adwRecipients[i]);
					dlgSelectRecipients.m_strRecipientsAry.Add(m_astrRecipients[i]);
				}
			}
		}

		// (c.haag 2004-06-23 16:08) - Update the user combo so we can
		// guarantee a hyperlink will appear if there are multiple recipients
		RefreshUserCombo();

		//PLID 21781 - set the focus to the text box when for replying
		GetDlgItem(IDC_TEXT)->SetFocus();

		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - Don't bring mainframe to top

		SendMessageDlgDisableHotkeys();

		if (bCenterWindow) {
			CenterWindow(); // (c.haag 2006-05-16 16:20) - Would be nice if it were centered when made visible
		}
		ShowWindow(SW_RESTORE);
		SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		SetForegroundWindow();
	}NxCatchAll(__FUNCTION__);
}

void CSendMessageDlg::RefreshColors()
{
	// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));

// (a.walling 2008-06-02 08:31) - PLID 30099
#ifndef NXDIALOG_NOCLIPCHILDEN
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
#else
	Invalidate(FALSE);//need to redraw because the color changed, which overwrote the labels
#endif
}

void CSendMessageDlg::OnCancel() 
{
	// (c.haag 2006-05-16 13:32) - PLID 20621 - This window is now modeless
	// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - Don't bring mainframe to top
	SendMessageDlgEnableHotkeys();
	//SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	//SetForegroundWindow();
	//ShowWindow(SW_HIDE);

	CNxModelessOwnedDialog::OnCancel();
}

void CSendMessageDlg::OnSendTo() 
{
	// MSC - 2/26/04 - if they clicked the send to button they want to send to a particular user, we need
	// to disable the groups combo and enable the users combo
	try{
		GetDlgItem(IDC_COMBO_USERS)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_GROUPS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_GROUPS)->EnableWindow(FALSE);
		
		m_nxlGroupsLabel.SetType(dtsDisabledHyperlink);
		m_nxlUserLabel.SetType(dtsHyperlink);
		m_nxlGroupsLabel.Invalidate(FALSE);
		m_nxlUserLabel.Invalidate(FALSE);

		m_pUsers->PutCurSel(-1);
		
	}NxCatchAll("Error in CSendMessageDlg::OnSendTo()");
}


void CSendMessageDlg::OnRegarding() 
{
	try {
		// (a.walling 2011-07-29 13:15) - PLID 44788
		/*
		long nRow = m_pPatients->CurSel;
		if(nRow == -1) m_nRegardingID = -1;
		else m_nRegardingID = VarLong(m_pPatients->GetValue(nRow,0));
		*/

		//(e.lally 2012-01-05) PLID 47334 - Re-instituted a way to get the regarding patient without always having to look at the datalist
		BOOL bIsEnabled = IsDlgButtonChecked(IDC_REGARDING);
		GetDlgItem(IDC_COMBO_PATIENTS)->EnableWindow(bIsEnabled);
		if(bIsEnabled){
			if(m_nRegardingID == -1){
				//We don't know who the regarding person is.
				if(m_pPatients->CurSel == sriNoRow){
					//Avoid accessing the datalist and get what we set the combobox text to, the active patient by default.
					m_nRegardingID = GetActivePatientID();
					if(m_pPatients->GetRowCount() > 0){
						//The patient list is filled so we can do a trysetsel asyncronously
						m_pPatients->TrySetSelByColumn(0, (long)m_nRegardingID);
					}
				}
				else {
					//Something is selected in the datalist, we should be using that value
					m_nRegardingID = VarLong(m_pPatients->GetValue(m_pPatients->CurSel,0));
				}
			}
		}
		else {
			//We could clear out the value, but we already check if we really are using the regarding list when sending so we'll leave it to reflect to selected record.
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-07-29 13:15) - PLID 44788 - Refresh / reload the regarding list
void CSendMessageDlg::RefreshRegardingList()
{	
	if (0 == m_pPatients->GetRowCount() && !m_pPatients->IsRequerying()) {
		m_pPatients->Requery();

		// (a.walling 2011-07-29 13:15) - PLID 44788
		long nPatientListSelectID = (m_nRegardingID == -1) ? GetActivePatientID() : m_nRegardingID;
		if(m_pPatients->TrySetSelByColumn(0, nPatientListSelectID) < 0) {			
			m_pPatients->ComboBoxText = (LPCTSTR)GetExistingPatientName(nPatientListSelectID);
			//(e.lally 2012-01-05) PLID 47334 - Update the regardingID here, we check that the box it checked before creating the message record
			m_nRegardingID = nPatientListSelectID;
		}
	}
}

void CSendMessageDlg::OnOK() 
{
	try{

		//Let's write this message to the database
		UpdateData(TRUE);

		if(m_strText.GetLength() > 3500) {
			MessageBox("The message can have no more than 3500 characters.");
			return;
		}
		else
		{
			// (c.haag 2004-06-22 17:49) - PLID 13152 - You should not be able
			// to send empty messages
			CString str = m_strText;
			str.Remove(' ');
			str.Remove('\n');
			str.Remove('\r');
			if (str.IsEmpty())
			{
				MessageBox("You cannot send an empty message");
				return;
			}
		}

		// (c.haag 2006-10-12 12:52) - PLID 22731 - If we are disconnected
		// from NxServer, try to reconnect immediately. If that fails, put the
		// user into a "try again?" loop.
		//
		// Keep in mind that the NxServer reconnection timer may still be in
		// action. If it is, it won't matter because InitNxServer exits gracefully
		// if we're connected
		//
		if (NULL == GetMainFrame()->GetNxServerSocket()) {
			BOOL bConnected = GetMainFrame()->InitNxServer();
			BOOL bQuit = FALSE;
			while (!bConnected && !bQuit) {
				if (IDYES == MsgBox(MB_YESNO | MB_ICONEXCLAMATION, "NexTech practice failed to connect to your PracYakker server. Would you like to try again?")) {
					bConnected = GetMainFrame()->InitNxServer();
				} else {
					if (IDNO == MsgBox(MB_YESNO | MB_ICONQUESTION, "Would you like to save your message so that all recipients will receive it at a later date?")) {
						return;
					} else {
						bQuit = TRUE;
					}
				}
			}
		}

		//MessageGroupID will be the same no matter how many recipients
		int nMessageGroupID = NewNumber("MessagesT", "MessageGroupID");
		
		if(IsDlgButtonChecked(IDC_SEND_TO_GROUP)){
			FillRecipAryWithGroupInfo();
		}
		
		if(m_dwRecipientsAry.GetSize() == 0){
				MsgBox("You cannot send a message with no recipients.");
				//Abort
				return;
		}
		else{//They did select some recipients
			//First let's get everything that will be the same for all messages
			int nSenderID = GetCurrentUserID();

			// (j.jones 2011-07-29 13:33) - PLID 30971 - RegardingID can be NULL
			_variant_t vtRegardingID = g_cvarNull;
			if(IsDlgButtonChecked(IDC_REGARDING) && m_nRegardingID != -1){
				vtRegardingID = (long)m_nRegardingID;
			}
			int nPriority;
			CString strPriority;
			m_cbPriority.GetWindowText(strPriority);
			if(strPriority == "Urgent") nPriority = 3;
			else if (strPriority == "Medium") nPriority = 2;
			else if (strPriority == "Low") nPriority = 1;
			else ASSERT(FALSE); //This shouldn't happen (obviously).
			COleDateTime dt = COleDateTime::GetCurrentTime();
			//Loop through the selected recipients, write each message

			// (j.gruber 2010-07-16 13:23) - PLID 39463
			if (m_nThreadID == -1) {
				//create a new thread
				// (j.armen 2014-01-30 16:46) - PLID 60565 - Idenitate MessageThreadsT
				_RecordsetPtr prs = CreateRecordsetStd(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO MessageThreadsT DEFAULT VALUES\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS MessageThreadID");

				m_nThreadID = AdoFldLong(prs, "MessageThreadID");
			}
				
			for(int i = 0; i < m_dwRecipientsAry.GetSize(); i++){
				// (j.gruber 2010-07-16 13:24) - PLID 39463 - add threadID
				// (j.jones 2011-07-29 13:33) - PLID 30971 - RegardingID can be NULL
				// (j.armen 2014-01-30 16:33) - PLID 58075 - Idenitate MesssagesT.MessageID
				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO MessagesT (Text, RecipientID, SenderID, RegardingID, Priority, DateSent, MessageGroupID, ThreadID)\r\n"
					"VALUES ({STRING}, {INT}, {INT}, {VT_I4}, {INT}, {OLEDATETIME}, {INT}, {INT})\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS MessageID",
					m_strText, (long)m_dwRecipientsAry.GetAt(i), nSenderID,
					vtRegardingID, nPriority, dt, nMessageGroupID, m_nThreadID);

				long nMessageID = AdoFldLong(prs, "MessageID");
				for (int j = 0; j < m_pMessageHome->m_nUsers; j++){
					if(m_pMessageHome->m_arUsers[j].strUserName == m_strRecipientsAry.GetAt(i)){
						CClient::SendIM(m_pMessageHome->m_arUsers[j].ip, (long)m_dwRecipientsAry.GetAt(i), dt, VarLong(vtRegardingID, -1), nPriority, nMessageGroupID, m_strText, nMessageID, nSenderID, GetCurrentUserName(), m_nThreadID);
					}
				}

				// (c.haag 2006-05-19 15:14) - Now we have to tell the PracYakker we sent a 
				// message, too.
				//
				// (a.walling 2007-05-04 09:53) - PLID 4850 - Messager dialog is a pointer now
				if (GetMainFrame()->m_pdlgMessager != NULL && IsWindow(GetMainFrame()->m_pdlgMessager->GetSafeHwnd())) {
					_IM_MESSAGE msg;
					msg.dwSentTo = (long)m_dwRecipientsAry[i];
					msg.date = dt.m_dt;
					msg.dwRegardingID = VarLong(vtRegardingID, -1);
					msg.dwPriority = nPriority;
					msg.dwMessageGroupID = nMessageGroupID;
					// (j.gruber 2010-07-16 13:57) - PLID 39643 - thread
					msg.dwThreadID = m_nThreadID;
					if(m_strText.GetLength() >= 3991) {
						msg.bCheckData = TRUE;
					}
					else {
						strcpy(msg.szMsg, m_strText);
						msg.bCheckData = FALSE;
					}
					msg.dwMessageID = nMessageID;
					msg.dwSentBy = (DWORD)nSenderID;
					strcpy(msg.szSentBy, GetCurrentUserName());
					ASSERT(GetMainFrame()->m_pdlgMessager != NULL);
					if (GetMainFrame()->m_pdlgMessager)
						GetMainFrame()->m_pdlgMessager->SendMessage(NXM_IM_MSGSENT, 0, (LPARAM)&msg);
				}
			}
		}

		//
		// (c.haag 2006-05-16 13:32) - PLID 20621 - The send message window is now modeless.
		// Notice how we put this in the catch-all; if we put it outside, then the user would
		// get an error message and the window would disappear with no indiciation of whether
		// the message was actually sent or not.
		//
		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - Don't bring mainframe to top
		SendMessageDlgEnableHotkeys();
		/*SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		SetForegroundWindow();
		ShowWindow(SW_HIDE);*/
		CNxModelessOwnedDialog::OnOK();
	}
	NxCatchAll("Error in CSendMessageDlg::OnOK()");

//	CNxModelessOwnedDialog::OnOK();
}

LRESULT CSendMessageDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case NXM_CTRL_ENTER:
		OnOK();
		return TRUE;
		break;
	case WM_SHOWWINDOW:
		// (a.walling 2011-07-29 13:15) - PLID 44788 - After a certain amount of time, free the memory
		if (0 == wParam) {
			SetTimer(ID_CLEAR_REGARDING_LIST, GetRemotePropertyInt("FreeBigDataListsDelay", 1200000, 0, "<None>", true), NULL);
		}
	default:
		return CNxModelessOwnedDialog::WindowProc(message, wParam, lParam);
		break;
	}
}

// (a.walling 2011-07-29 13:15) - PLID 44788 - After a certain amount of time, free the memory
void CSendMessageDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == ID_DROPDOWN_USERS) {
		if(IsWindowVisible()) {
			m_pUsers->PutDropDownState(TRUE);
			KillTimer(ID_DROPDOWN_USERS);
		}
	} else if (nIDEvent == ID_CLEAR_REGARDING_LIST) {
		// (a.walling 2011-07-29 13:15) - PLID 44788
		KillTimer(ID_CLEAR_REGARDING_LIST);
		if (!IsWindowVisible()) {
			m_pPatients->Clear();
		}
	}
	CNxModelessOwnedDialog::OnTimer(nIDEvent);
}

void CSendMessageDlg::OnSendGroup()
{
	GetDlgItem(IDC_COMBO_GROUPS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_GROUPS)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_USERS)->EnableWindow(FALSE);
	m_nxlUserLabel.SetType(dtsDisabledHyperlink);
	m_nxlGroupsLabel.SetType(dtsHyperlink);
	m_nxlUserLabel.Invalidate(FALSE);
	m_nxlGroupsLabel.Invalidate(FALSE);
	m_pGroups->PutCurSel(-1);
}

void CSendMessageDlg::OnEditGroups() 
{
	CYakGroupSelect dlgGroupSelect(this);
	_bstr_t CurrentlySelected = "";
	if(m_pGroups->GetCurSel() != sriNoRow){
		CurrentlySelected = m_pGroups->GetValue(m_pGroups->GetCurSel(), 1);
	}
	
	dlgGroupSelect.DoModal(1);  //  1 means edit
	m_pGroups->Requery();
	
	m_pGroups->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	if (m_pGroups->GetRowCount() > 1)
	{
		IRowSettingsPtr pRow;
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pGroups->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(MULTIPLE_GROUPS);
		m_pGroups->InsertRow(pRow, 0);
	}
	m_pGroups->SetSelByColumn(1, CurrentlySelected);

	
}

BEGIN_EVENTSINK_MAP(CSendMessageDlg, CNxModelessOwnedDialog)
    //{{AFX_EVENTSINK_MAP(CSendMessageDlg)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_GROUPS, 16 /* SelChosen */, OnSelChosenComboGroups, VTS_I4)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_USERS, 16 /* SelChosen */, OnSelChosenComboUsers, VTS_I4)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_PATIENTS, 16 /* SelChosen */, OnSelChosenComboPatients, VTS_I4)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_USERS, 18 /* RequeryFinished */, OnRequeryFinishedComboUsers, VTS_I2)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_GROUPS, 18 /* RequeryFinished */, OnRequeryFinishedComboGroups, VTS_I2)
	ON_EVENT(CSendMessageDlg, IDC_COMBO_PATIENTS, 26 /* DroppingDown */, OnDroppingDownPatients, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSendMessageDlg::OnSelChosenComboGroups(long nRow) 
{
	try{
		long nCurSel = m_pGroups->GetCurSel();

		// if they haven't selected anything, clear the recips arrays and return
		if(nCurSel == sriNoRow){
			m_dwGroupRecipAry.RemoveAll();
			m_strGroupRecipAry.RemoveAll();
			m_dwRecipientsAry.RemoveAll();
			m_strRecipientsAry.RemoveAll();
			return;
		}

		
		// check and see if the user selected multiple groups
		if (nCurSel > -1 && VarString(m_pGroups->Value[nCurSel][1], "") != MULTIPLE_GROUPS)
		{
			// they only selected one group, clear the ary and then add the group
			m_dwGroupRecipAry.RemoveAll();
			m_strGroupRecipAry.RemoveAll();

			
			if (m_pGroups->Value[nCurSel][0].vt == VT_I4) {
				// add the ID to the group array
				m_dwGroupRecipAry.Add(VarLong(m_pGroups->GetValue(nCurSel, 0)));
				m_strGroupRecipAry.Add(VarString(m_pGroups->GetValue(nCurSel, 1)));
			}

		}
		else
		{
			// they want to send to multiple groups, open the dialog that will let them choose more then one group
			dlgGroupSelect.DoModal(0);
			
			m_dwGroupRecipAry.RemoveAll();
			m_strGroupRecipAry.RemoveAll();
			
			// first go through the array of groups and build a where clause so we can find which userIDs we want to
			// send to
			for(int i = 0; i < dlgGroupSelect.m_dwGroupsAry.GetSize(); i++){
				m_dwGroupRecipAry.Add(dlgGroupSelect.m_dwGroupsAry.GetAt(i));			
				m_strGroupRecipAry.Add(dlgGroupSelect.m_strGroupsAry.GetAt(i));
			}
		}

		RefreshGroupsCombo();
	}NxCatchAll("Error in OnSelChosenComboGroups()");
}

void CSendMessageDlg::OnSelChosenComboUsers(long nRow) 
{
	try{
		long nCurSel = m_pUsers->GetCurSel();
	
		if (nCurSel > -1 && VarString(m_pUsers->Value[nCurSel][1], "") != MULTIPLE_USERS)
		{
			m_dwRecipientsAry.RemoveAll();
			m_strRecipientsAry.RemoveAll();

			if (m_pUsers->Value[nCurSel][0].vt == VT_I4) {
				// add the PersonID to the first array
				m_dwRecipientsAry.Add(VarLong(m_pUsers->GetValue(nCurSel, 0)));
				// add the username to the string array
				m_strRecipientsAry.Add(VarString(m_pUsers->GetValue(nCurSel, 1)));
			}
		}
		else
		{
			dlgSelectRecipients.DoModal();

			m_dwRecipientsAry.RemoveAll();
			m_strRecipientsAry.RemoveAll();
			CString strWhere = "";
			// first go through the array of groups and build a where clause so we can find which userIDs we want to
			// send to
			for(int i = 0; i < dlgSelectRecipients.m_dwRecipientsAry.GetSize(); i++){
				m_dwRecipientsAry.Add((long)dlgSelectRecipients.m_dwRecipientsAry.GetAt(i));				
				m_strRecipientsAry.Add(dlgSelectRecipients.m_strRecipientsAry.GetAt(i));
			}

		}

		RefreshUserCombo();
	}NxCatchAll("Error in OnSelChosenComboUsers()");
}

void CSendMessageDlg::RefreshUserCombo()
{
	try{
		if(IsCurUserListMultiUser()){
			// the list has more then one user to send to
			// first hide the combo
			ShowDlgItem(IDC_COMBO_USERS, SW_HIDE);
			
			// now get a list of the users to send to
			CString strUserList;
			strUserList = GenerateUsersString();
			
			// set the text with the list of users
			m_nxlUserLabel.SetText(strUserList);
			m_nxlUserLabel.SetType(dtsHyperlink);
			m_nxlUserLabel.Invalidate();
					
			// show the list
			ShowDlgItem(IDC_LIST_USERS, SW_SHOW);
			InvalidateDlgItem(IDC_COMBO_USERS);
			InvalidateDlgItem(IDC_LIST_USERS);
		}
		else{
			// there is only one user to send to
			ASSERT(m_dwRecipientsAry.GetSize() <= 1);
			if(m_dwRecipientsAry.GetSize() == 0)
			{
				// if there is nothing selected, then set the selection to nothing
				m_pUsers->SetSelByColumn(0, (long)-1);
			} else {
				// they have one selected
				long nUserToSendTo = m_dwRecipientsAry.GetAt(0);
				// set the selection to the single one they have selected
				m_pUsers->SetSelByColumn(0, nUserToSendTo);
			}
			// hide the user list and show the combo since there is only one
			ShowDlgItem(IDC_LIST_USERS, SW_HIDE);
			ShowDlgItem(IDC_COMBO_USERS, SW_SHOW);
			InvalidateDlgItem(IDC_LIST_USERS);
			InvalidateDlgItem(IDC_COMBO_USERS);
		}
	}NxCatchAll("Error Refreshing User Combo Box");

}

void CSendMessageDlg::RefreshGroupsCombo()
{
	try{
		if(IsCurGroupListMultiGroup()){
			// they have more then one group selected
			// hide the group combo
			ShowDlgItem(IDC_COMBO_GROUPS, SW_HIDE);
			
			// create a list of groups
			CString strGroupList;
			strGroupList = GenerateGroupsString();
			
			// set the text to the list of the groups
			m_nxlGroupsLabel.SetText(strGroupList);
			m_nxlGroupsLabel.SetType(dtsHyperlink);
			m_nxlGroupsLabel.Invalidate();

			//	show the list of the groups
			ShowDlgItem(IDC_LIST_GROUPS, SW_SHOW);
			InvalidateDlgItem(IDC_COMBO_GROUPS);
			InvalidateDlgItem(IDC_LIST_GROUPS);
		}
		else{
			ASSERT(m_dwGroupRecipAry.GetSize() <= 1);
			if(m_dwGroupRecipAry.GetSize() == 0)
			{
				// nothing is in the array which means they unselectd all of the groups
				// so set the combo to nothing
				m_pGroups->SetSelByColumn(0, (long)-1);
			}
			else{
				long nGroupToSendTo = m_dwGroupRecipAry.GetAt(0);
				// set the combo to single one selcted
				m_pGroups->SetSelByColumn(0, nGroupToSendTo);
			}
			
			// hide the list of groups and show the combo with the right one selected
			ShowDlgItem(IDC_LIST_GROUPS, SW_HIDE);
			ShowDlgItem(IDC_COMBO_GROUPS, SW_SHOW);
			InvalidateDlgItem(IDC_LIST_GROUPS);
			InvalidateDlgItem(IDC_COMBO_GROUPS);
		}
	}NxCatchAll("Error Refreshing Groups Combo Box");
}


// Throw exceptions
BOOL CSendMessageDlg::IsCurUserListMultiUser()
{
	switch (m_dwRecipientsAry.GetSize()) {
	case 0:
		// No users, so not a multi-user list (we used to ASSERT(FALSE) here and throw an exception but it turns out it's legit)
		return FALSE;
		break;
	case 1:
		// Not a multi-resource list
		return FALSE;
		break;
	default:
		// Is a multi-resource list
		return TRUE;
		break;
	}
}

// Throw exceptions
BOOL CSendMessageDlg::IsCurGroupListMultiGroup()
{
	switch (m_dwGroupRecipAry.GetSize()) {
	case 0:
		// No users, so not a multi-group list
		return FALSE;
		break;
	case 1:
		// Not a multi-resource list
		return FALSE;
		break;
	default:
		// Is a multi-resource list
		return TRUE;
		break;
	}
}


CString CSendMessageDlg::GenerateUsersString() {

	CString strUsers = "";
	try{
		// loop through all of the recipients in the array and add it to the string to return
		for (long i=0; i < m_strRecipientsAry.GetSize(); i++)
		{
			long lRow = m_pUsers->FindByColumn(0, (long)m_dwRecipientsAry[i], 0, FALSE);
			if (lRow != -1)
			{
				CString strThisUser = VarString(m_pUsers->GetValue(lRow, 1));
				if (strUsers.IsEmpty())
					strUsers = strThisUser;
				else
					strUsers += ", " + strThisUser;
			}
		}
	}NxCatchAll("Error generating users string");

	return strUsers;
}


CString CSendMessageDlg::GenerateGroupsString() {

	CString strGroups;

	try{
		// loop through all of the groups in the array and add it to the string to return
		for (long i=0; i < m_strGroupRecipAry.GetSize(); i++)
		{
			long lRow = m_pGroups->FindByColumn(0, (long)m_dwGroupRecipAry[i], 0, FALSE);
			if (lRow != -1)
			{
				CString strThisGroup = VarString(m_pGroups->GetValue(lRow, 1));
				if (strGroups.IsEmpty())
					strGroups = strThisGroup;
				else
					strGroups += ", " + strThisGroup;
			}
		}
	}NxCatchAll("Error generating Groups string");

	return strGroups;
}

LRESULT CSendMessageDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	int i;
	CString strWhere = "";
	switch(nIdc) {
	case IDC_LIST_USERS:
		// they clicked on the hyperlink of the selected users, open the dialog that lets them choose multiple users
		dlgSelectRecipients.DoModal();

		m_dwRecipientsAry.RemoveAll();
		m_strRecipientsAry.RemoveAll();
		
		// first go through the array of groups and build a where clause so we can find which userIDs we want to
		// send to
		for(i = 0; i < dlgSelectRecipients.m_dwRecipientsAry.GetSize(); i++){
			m_dwRecipientsAry.Add((long)dlgSelectRecipients.m_dwRecipientsAry.GetAt(i));				
			m_strRecipientsAry.Add(dlgSelectRecipients.m_strRecipientsAry.GetAt(i));
		}

		RefreshUserCombo();
		break;
	case IDC_LIST_GROUPS:
		// they clicked on the hyperlink of the selected groups, open the dialog that lets them choose multiple groups
		
		dlgGroupSelect.DoModal(0);
		
		m_dwGroupRecipAry.RemoveAll();
		m_strGroupRecipAry.RemoveAll();
		
		// first go through the array of groups and build a where clause so we can find which userIDs we want to
		// send to
		for(i = 0; i < dlgGroupSelect.m_dwGroupsAry.GetSize(); i++){
			m_dwGroupRecipAry.Add((long)dlgGroupSelect.m_dwGroupsAry.GetAt(i));				
			m_strGroupRecipAry.Add(dlgGroupSelect.m_strGroupsAry.GetAt(i));
		}
	
		RefreshGroupsCombo();
		break;
			
	default:
		ASSERT(FALSE);
		break;
	}
	return 0;
}


BOOL CSendMessageDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (GetDlgItem(IDC_LIST_GROUPS)->IsWindowVisible() && m_nxlGroupsLabel.GetType() == dtsHyperlink)
	{
		CRect rc;
		GetDlgItem(IDC_LIST_GROUPS)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (GetDlgItem(IDC_LIST_USERS)->IsWindowVisible() && m_nxlUserLabel.GetType() == dtsHyperlink)
	{
		CRect rc;
		GetDlgItem(IDC_LIST_USERS)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CNxModelessOwnedDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CSendMessageDlg::FillRecipAryWithGroupInfo()
{
	// we should have our group(s), create a where clause that has all of the GroupIDs so we can find what users
	// to send to
	CString strToAdd, strWhere = "";
		
	// make sure there is something selected
	if(m_dwGroupRecipAry.GetSize() == 0){
		m_dwRecipientsAry.RemoveAll();
		m_strRecipientsAry.RemoveAll();
		RefreshGroupsCombo();
		return;
	}


	for(int x = 0; x < m_dwGroupRecipAry.GetSize(); x++){
		strToAdd.Format("%li, ", m_dwGroupRecipAry.GetAt(x));
		strWhere += strToAdd;
	}

	// get rid of the last ", "
	strWhere.Delete(strWhere.GetLength() - 2, 2);

	// (a.walling 2010-11-11 17:13) - PLID 41459 - No foreign key on here, hence an exception if it still contains PersonIDs that are invalid.
	// Just INNER JOIN for now so we know there won't be any errors due to NULL values. A mod will clean up the data and add a foreign key constraint.
	//(e.lally 2011-06-17) PLID 36505 - Skip inactive users that are still in this group.
	_RecordsetPtr rsUsers = CreateParamRecordset("SELECT YakGroupDetailsT.PersonID AS ID, UserName "
		"FROM YakGroupDetailsT "
		"INNER JOIN UsersT ON UsersT.PersonID = YakGroupDetailsT.PersonID "
		"INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID "
		"WHERE PersonT.Archived =0 AND YakGroupDetailsT.GroupID IN ({INTSTRING}) "
		"GROUP BY YakGroupDetailsT.PersonID, UsersT.UserName", strWhere);

	m_dwRecipientsAry.RemoveAll();
	m_strRecipientsAry.RemoveAll();
		
	// go through the users that were in selected groups and add them to the recipients array
	while(!rsUsers->eof)
	{
		m_dwRecipientsAry.Add(AdoFldLong(rsUsers, "ID"));
		m_strRecipientsAry.Add(AdoFldString(rsUsers, "UserName"));
		rsUsers->MoveNext();
	}
}

void CSendMessageDlg::OnSelChosenComboPatients(long nRow) 
{
	if(nRow == -1) m_nRegardingID = -1;
	else m_nRegardingID = VarLong(m_pPatients->GetValue(nRow,0));
}

// (a.walling 2011-07-29 13:15) - PLID 44788 - The list is dropping down, so populate it
void CSendMessageDlg::OnDroppingDownPatients()
{
	try {
		RefreshRegardingList();
	} NxCatchAll(__FUNCTION__);
}

void CSendMessageDlg::OnRequeryFinishedComboUsers(short nFlags) 
{
	// add a row to send to multiple users
	if (m_pUsers->GetRowCount() > 1)
	{
		IRowSettingsPtr pRow;
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pUsers->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(MULTIPLE_USERS);
		m_pUsers->InsertRow(pRow, 0);
	}
}

void CSendMessageDlg::OnRequeryFinishedComboGroups(short nFlags) 
{
	// add a row to send to multiple groups
	if (m_pGroups->GetRowCount() > 1)
	{
		IRowSettingsPtr pRow;
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pGroups->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(MULTIPLE_GROUPS);
		m_pGroups->InsertRow(pRow, 0);
	}	
}

void CSendMessageDlg::SendMessageDlgDisableHotkeys()
{
	// (z.manning, 06/20/2007) - PLID 26390 - Only disable the hotkeys if this dialog hasn't already done so.
	// Otherwise we may end up disabling them multiple times but only enabling them once which will
	// prevent them from working.
	if(!m_bSendMessageDlgHasDisabledHotkeys) {
		if(GetMainFrame()) {
			GetMainFrame()->DisableHotKeys();
		}
	}
	m_bSendMessageDlgHasDisabledHotkeys = TRUE;
}

void CSendMessageDlg::SendMessageDlgEnableHotkeys()
{
	// (z.manning, 06/20/2007) - PLID 26390 - Re-enable hotkeys.
	if(GetMainFrame()) {
		GetMainFrame()->EnableHotKeys();
	}
	m_bSendMessageDlgHasDisabledHotkeys = FALSE;
}