// DirectMessageReceiveddlg.cpp : implementation file
//
// (j.camacho 2013-10-21 18:07) - PLID 59064

#include "stdafx.h"
#include "Practice.h"
#include "DirectMessageReceiveddlg.h"
#include "DirectMessageSendDlg.h"
#include "DirectMessageViewDlg.h"
#include "NxApi.h"
#include "FinancialRC.h"

using namespace NXDATALIST2Lib; 
// CDirectMessageReceivedDlg dialog/

// (d.singleton 2014-05-07 08:38) - PLID 61800 - Mark message as unread functionality for direct messaging
#define ID_MARK_DIR_MESSAGE_READ	50001
#define ID_MARK_DIR_MESSAGE_UNREAD	50002

IMPLEMENT_DYNAMIC(CDirectMessageReceivedDlg, CNxDialog)

CDirectMessageReceivedDlg::CDirectMessageReceivedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDirectMessageReceivedDlg::IDD, pParent)
{
	// (b.spivey - June 2, 2014) - PLID 62289 = always start on page 1. 
	m_nCurrentPage = 1; 
}

CDirectMessageReceivedDlg::~CDirectMessageReceivedDlg()
{
	// (b.spivey - May 5th, 2014) PLID 61799 - destroy the icon. 
	if(m_hIconDirectMessageState) {
		DestroyIcon((HICON)m_hIconDirectMessageState); 
	}

	if(m_hIconPlaceholder) {
		DestroyIcon((HICON)m_hIconPlaceholder); 
	}
}

void CDirectMessageReceivedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_RECEIVED, m_background);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_SEND, m_btnSend);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_DIRECT_CERTIFICATE, m_btnSetupDirectCertificate);
	DDX_Control(pDX, IDC_FILTER_READ, m_btnFilterRead);
	DDX_Control(pDX, IDC_FILTER_DATE, m_btnFilterDate);
	DDX_Control(pDX, IDC_FILTER_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_FILTER_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_NEXT_PAGE, m_btnNextPage); 
	DDX_Control(pDX, IDC_PREV_PAGE, m_btnPrevPage); 
	DDX_Control(pDX, IDC_MESSAGE_COUNT, m_lblMessageBracket); 
}



BEGIN_MESSAGE_MAP(CDirectMessageReceivedDlg, CNxDialog)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_SEND, &CDirectMessageReceivedDlg::OnBnClickedDirectmessageSend)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_DELETE, &CDirectMessageReceivedDlg::OnBnClickedDirectmessageDelete)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_DIRECT_CERTIFICATE, &CDirectMessageReceivedDlg::OnBnClickedDirectMessageCertificate)
	ON_BN_CLICKED(IDC_FILTER_READ, &CDirectMessageReceivedDlg::OnBnClickedFilterRead)
	ON_BN_CLICKED(IDC_FILTER_DATE, &CDirectMessageReceivedDlg::OnBnClickedFilterDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_FROM_DATE, &CDirectMessageReceivedDlg::OnDtnDatetimechangeFilterFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_TO_DATE, &CDirectMessageReceivedDlg::OnDtnDatetimechangeFilterToDate)
	ON_COMMAND(ID_MARK_DIR_MESSAGE_READ, &CDirectMessageReceivedDlg::MarkMessageAsRead)
	ON_COMMAND(ID_MARK_DIR_MESSAGE_UNREAD, &CDirectMessageReceivedDlg::MarkMessageAsUnread)
	ON_BN_CLICKED(IDC_NEXT_PAGE, &CDirectMessageReceivedDlg::OnBnClickedNextPage) 
	ON_BN_CLICKED(IDC_PREV_PAGE, &CDirectMessageReceivedDlg::OnBnClickedPrevPage)
END_MESSAGE_MAP()


BOOL CDirectMessageReceivedDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		g_propManager.BulkCache("DirectMessageReceivedDlg", propbitDateTime|propbitNumber, 
			"UserName = '%s' AND Name IN ( "
			" 'FilterDirectMessageStatus', "
			" 'FilterDirectMessageDate', "
			" 'FilterDirectMessageDateFrom', "
			" 'FilterDirectMessageDateTo' "
			")", _Q(GetCurrentUserName()));
		
		m_background.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnSend.AutoSet(NXB_EXPORT);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnSetupDirectCertificate.AutoSet(NXB_MODIFY); 
		//try initiating Datalists
		m_dlReceivedList = BindNxDataList2Ctrl(IDC_DIRECTMESSAGE_RECEIVED, false);
		m_dlEmailList = BindNxDataList2Ctrl(IDC_DIRECTMESSAGE_EMAIL,false);
		// (b.spivey - May 5th, 2014) PLID 61797 - Ambiguous column fixed. 
		m_dlEmailList->PutWhereClause(FormatBstr("DAUT.UserID = %li", GetCurrentUserID()));
		m_dlEmailList->Requery();
		m_dlEmailList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_dlEmailList->GetFirstRow());
		if(pRow)
		{
			m_dlEmailList->PutCurSel(pRow);
			// (b.spivey - May 5th, 2014) PLID 61797 - Use enum
			m_strActiveEmail = VarString(pRow->GetValue(dmalEmail),"");
		}

		// (b.spivey - May 5th, 2014) PLID 61799 - load the icon. 
		m_hIconDirectMessageState = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_READ_MAIL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		m_hIconPlaceholder = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_PLACEHOLDER), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		// (b.spivey - June 2, 2014) - PLID 62289 - really, always start on page 1. 
		m_nCurrentPage = 1; 

		// (d.singleton 2014-05-06 10:07) - PLID 61802 - Add an option to hide read messages in the direct message list.
		// (d.singleton 2014-05-06 10:07) - PLID 61803 - Add a filter to the direct message lists to show messages within a date range filter.
		m_btnFilterRead.SetCheck(GetRemotePropertyInt("FilterDirectMessageStatus", 0, 0, GetCurrentUserName(), true));
		m_btnFilterDate.SetCheck(GetRemotePropertyInt("FilterDirectMessageDate", 0, 0, GetCurrentUserName(), true));
		COleDateTime dtFrom = GetRemotePropertyDateTime("FilterDirectMessageDateFrom", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName(), true);
		if(dtFrom.GetStatus() == COleDateTime::valid) {
			m_dtFrom.SetTime(dtFrom);
		}
		COleDateTime dtTo = GetRemotePropertyDateTime("FilterDirectMessageDateTo", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName(), true);
		if(dtTo.GetStatus() == COleDateTime::valid) {
			m_dtTo.SetTime(dtTo);
		}
		if(m_btnFilterDate.GetCheck()) {
			m_dtFrom.EnableWindow(TRUE);
			m_dtTo.EnableWindow(TRUE);
		}
		else {
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}

		// (v.maida 2016-06-02 16:41) - NX-100797 - If this is an Azure user, and they're not a technical support user, then hide the direct message
		// certificate setup button.
		if (g_pLicense->GetAzureRemoteApp() && GetCurrentUserID() != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			m_btnSetupDirectCertificate.EnableWindow(FALSE);
			m_btnSetupDirectCertificate.ShowWindow(SW_HIDE);
		}
		else {
			m_btnSetupDirectCertificate.EnableWindow(TRUE);
			m_btnSetupDirectCertificate.ShowWindow(SW_SHOW);
		}

		// (b.spivey, February 5th, 2014) - PLID 60648 - Warn the user. 
		if (!ReturnsRecords("SELECT * FROM DirectCertificateThumbprintT ")) {
			AfxMessageBox("There is no Direct Certificate Thumbprint in data. Please enter one in the Direct Message tab "
				"of the Links Module.");
		}
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

// (j.camacho 2013-11-13 14:18) 
void CDirectMessageReceivedDlg::UpdateView(bool bForceRefresh) 
{
	try{
		m_dlEmailList->Requery();
		// (b.spivey - May 5th, 2014) PLID 61797 - Use enum
		if(!(m_dlEmailList->SetSelByColumn(dmalEmail, _variant_t(m_strActiveEmail))))
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow(m_dlEmailList->GetFirstRow());
			if(pRow)
			{
				m_dlEmailList->PutCurSel(pRow);
				// (b.spivey - May 5th, 2014) PLID 61797 - Use enum
				m_strActiveEmail = VarString(pRow->GetValue(dmalEmail), "");
			}
			else
			{
				m_strActiveEmail="";
			}
			
		}	

		// (b.spivey - June 2, 2014) - PLID 62289 - Load the messages into the list based on our filters. 
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 
		CNxDialog::UpdateView();
	}NxCatchAll(__FUNCTION__);
}


void CDirectMessageReceivedDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try{
		
		CNxDialog::OnShowWindow(bShow,nStatus);
	}NxCatchAll(__FUNCTION__);
}

void CDirectMessageReceivedDlg::OnOK()
{
	try{
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);

}


// CDirectMessageReceivedDlg message handlers

// (b.spivey, October 21, 2013) - PLID 59115 - Load the messages into the datalist. 
// (b.spivey - June 2, 2014) - PLID 62289 - Changed this to take a safe array and fill the UI with the information. 
void CDirectMessageReceivedDlg::LoadMessagesIntoList(Nx::SafeArray<IUnknown *> saMessageHeaderList)
{
	//Clear this, we're only going to receive valid messages anyways.
	m_dlReceivedList->Clear(); 

	foreach(NexTech_Accessor::_DirectMessageHeaderPtr dmhp, saMessageHeaderList) {
		//New row. 
		IRowSettingsPtr pRow = m_dlReceivedList->GetNewRow(); 
		
		//Get every message header and display it in the list. 
		pRow->PutValue(dmrlID, VarBigInt(dmhp->DirectMessageID->GetValue(), -1)); 
		pRow->PutValue(dmrlMessageSize, VarBigInt(dmhp->DirectMessageSize->GetValue(), -1)); 
		pRow->PutValue(dmrlSubject, dmhp->PublicSubject); 
		pRow->PutValue(dmrlSender, dmhp->DirectFromAddress); 

		// (b.spivey - May 2nd, 2014) - PLID 61799 - set the message state column. 
		_variant_t var(dmhp->DirectMessageState->GetValue()); 
		// (b.spivey - May 5th, 2014) PLID 61799 - set the icon and message state.
		if(VarLong(var, 0) == (long)NexTech_Accessor::DirectMessageState_New) {
			pRow->PutValue(dmrlMessageState, dmsNew); 
			pRow->PutValue(dmrlReadStateIcon, (long)m_hIconPlaceholder); 
		}
		else if (VarLong(var, 0) == (long)NexTech_Accessor::DirectMessageState_Read) {
			pRow->PutValue(dmrlMessageState, dmsRead);
			pRow->PutValue(dmrlReadStateIcon, (long)m_hIconDirectMessageState); 
		}
		else {
			ASSERT(FALSE); 
			pRow->PutValue(dmrlMessageState, dmsOther); 
		}

		COleDateTime dtReceived = COleDateTime(dmhp->ReceivedDate);
		_variant_t vtReceived = _variant_t(dtReceived);
		vtReceived.vt = VT_DATE; 
		pRow->PutValue(dmrlDateReceived, vtReceived);

		//add this row, finally.
		m_dlReceivedList->AddRowSorted(pRow, NULL); 
	}
}

Nx::SafeArray<IUnknown *> CDirectMessageReceivedDlg::GetMessageHeaderListFromAPI(CString strEmailID)
{
	Nx::SafeArray<IUnknown *> saMessageHeaderList;

	try {
		//Need a wait cursor because this can potentially be slow. 
		CWaitCursor pWait; 
	
		// (b.spivey, February 5th, 2014) - PLID 60648 - Don't bother. They don't have a certificate. 
		if (!ReturnsRecords("SELECT * FROM DirectCertificateThumbprintT ")) {
			return saMessageHeaderList;
		}

		//If we have no ID, we cannot load anything!
		if (strEmailID.IsEmpty()) {
			return saMessageHeaderList;
		}


		// (b.spivey - June 2, 2014) - PLID 62289 - Build the filter
		NexTech_Accessor::_SESMessageFilterPtr pMessageFilter(__uuidof(NexTech_Accessor::SESMessageFilter)); 
		//The page decides which bracket of 50 we use. 
		pMessageFilter->Page = m_nCurrentPage; 
		//The dates start nullable because they might not have a range selected. 
		//	The reason we do it this way is because C++ and C# date min-maxes don't agree with each other. 
		NexTech_Accessor::_NullableDateTimePtr pNullableToDate(__uuidof(NexTech_Accessor::NullableDateTime));
		NexTech_Accessor::_NullableDateTimePtr pNullableFromDate(__uuidof(NexTech_Accessor::NullableDateTime));

		//if we have a filter checked, then we set the dates and pass it to the API.
		if (m_btnFilterDate.GetCheck()) {

			COleDateTime dtFrom, dtTo; 
			m_dtFrom.GetTime(dtFrom);
			m_dtTo.GetTime(dtTo);

			//We have to manually set the time to the end of the day. 
			dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
			dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 23, 59, 59);
			
			pNullableToDate->SetDateTime(dtTo);
			pNullableFromDate->SetDateTime(dtFrom); 

			pMessageFilter->toDate = pNullableToDate; 
			pMessageFilter->fromDate = pNullableFromDate; 
		}
		else {
			//In every other case just pass null. 
			pNullableToDate->SetNull(); 
			pNullableFromDate->SetNull();

			pMessageFilter->toDate = pNullableToDate;
			pMessageFilter->fromDate = pNullableFromDate; 
		}

		//If we have a status filter we set that too. 
		if (m_btnFilterRead.GetCheck()) {
			pMessageFilter->MessageFilterState = NexTech_Accessor::SESMessageStateFilter_Unread;
		}
		else {
			pMessageFilter->MessageFilterState = NexTech_Accessor::SESMessageStateFilter_All; 
		}

		
		//Get the list of messages, put it into a safe array.
		NexTech_Accessor::_DirectMessageHeaderListPtr dmhlp = 
			GetAPI()->GetDirectMessageHeaderListByEmailWithFilter(GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_strActiveEmail), pMessageFilter); 
		saMessageHeaderList = dmhlp->DirectMessageHeaders; 
		return saMessageHeaderList; 

	} 
	// (b.spivey, May 19th, 2014) PLID 62069 - catch configuration errors and tell the user what's going on. 
	catch (_com_error e) {
		CString errorMessage = (LPCTSTR)e.Description();
		Log(e.Description()); 
		if (errorMessage.Find("An unsecured or incorrectly secured fault was received from the other party.") >= 0
			&& errorMessage.Find("An error occurred when verifying security for the message.") >= 0) {
				AfxMessageBox("An error was discovered verifying the security of the request. Typically this means that the server time is skewed more "
					"than five minutes from the current time. Please check your server time and try again."); 
		} 
		else if (errorMessage.Find("and allowed clock skew is ''00:05:00''") >= 0 && errorMessage.Find("timestamp is invalid because its creation time") >= 0) {
			AfxMessageBox("Your server time is skewed more than five minutes from the current time. Please update your service time and try again."); 
		}
		else {
			//Welp, I don't know. 
			throw e; 
		}
	}

	return saMessageHeaderList;
}

// (j.camacho 2013-10-23 17:20) - PLID 58929 - Opens send message dialog
void CDirectMessageReceivedDlg::OnBnClickedDirectmessageSend()
{
	try{ 
		// TODO: Add your control notification handler code here
		CDirectMessageSendDlg dlgMessage;
		dlgMessage.DoModal();
	
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CDirectMessageReceivedDlg, CNxDialog)
	ON_EVENT(CDirectMessageReceivedDlg, IDC_DIRECTMESSAGE_RECEIVED, 3, CDirectMessageReceivedDlg::DblClickCellDirectmessageReceived, VTS_DISPATCH VTS_I2)
	ON_EVENT(CDirectMessageReceivedDlg, IDC_DIRECTMESSAGE_EMAIL, 2, CDirectMessageReceivedDlg::SelChangedDirectmessageEmail, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CDirectMessageReceivedDlg, IDC_DIRECTMESSAGE_EMAIL, 1, CDirectMessageReceivedDlg::SelChangingDirectmessageEmail, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CDirectMessageReceivedDlg, IDC_DIRECTMESSAGE_RECEIVED, 6, CDirectMessageReceivedDlg::RButtonDownDirectmessageReceived, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CDirectMessageReceivedDlg::DblClickCellDirectmessageReceived(LPDISPATCH lpRow, short nColIndex)
{
	try{
		CWaitCursor pWaitCursor; 
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			CDirectMessageViewDlg dlgView;
			dlgView.SetMessageID(VarBigInt(pRow->GetValue(dmrlID)),m_strActiveEmail);
			dlgView.SetMessageSize(VarBigInt(pRow->GetValue(dmrlMessageSize), -1));
			dlgView.DoModal() ;
			pWaitCursor.Restore(); 
			if(dlgView.m_bErrorOnLoading == true)
			{
				// (b.spivey - June 2, 2014) - PLID 62289 - Load the list differently now. 
				AfxMessageBox("There was an error loading that message. Reloading list...\n\rIf this continues to happen please contact Nextech Support.");
				LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
				UpdateControls(m_dlReceivedList->GetRowCount()); 
				return;
			}
			if(dlgView.CheckDelete())
			{
				RemoveMessageFromList(VarBigInt(pRow->GetValue(dmrlID)));
				// (b.spivey - June 4, 2014) - PLID 62291 - if we remove the message, we should go ahead and update the count label too. 
				UpdateMessageCountLabel(); 
				return;
			}
			if(dlgView.CheckUnread())
			{
				// (d.singleton 2014-05-16 08:36) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 			
				MarkMessageAsUnread();
				return;
			}
			// (d.singleton 2014-05-15 13:30) - PLID 62165 - need to mark the message as read in the ui when closing an opened direct message.
			pRow->PutValue(dmrlMessageState, (long)dmsRead);
			pRow->PutValue(dmrlReadStateIcon, (long)m_hIconDirectMessageState);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-10-24 10:53) - PLID 59148 - delete currently selected message. 
void CDirectMessageReceivedDlg::OnBnClickedDirectmessageDelete()
{
	try{
		IRowSettingsPtr pRow = m_dlReceivedList->GetCurSel();
		if(pRow)
		{
			__int64 messageid = pRow->GetValue(dmrlID);
			NexTech_Accessor::_DirectMessageDeleteResultPtr msgDel;
			msgDel = GetAPI()->DeleteDirectMessageByID(GetAPISubkey(), GetAPILoginToken(), messageid,_bstr_t(m_strActiveEmail));
			RemoveMessageFromList(messageid);
			// (b.spivey - June 2, 2014) - PLID 62291 - When we delete, it would be inefficient to reload the list. So instead we just remove the header from the UI and change our bracket. 
			UpdateMessageCountLabel(); 
		}		

	}
	catch(_com_error e)
	{
		Log(e.Description());
		CString error = (LPCTSTR)e.Description();
		if(error.Find("No Message for Message")>=0)
		{
			AfxMessageBox("This message could not be deleted because it has already been deleted somewhere else. Reloading list...");
		}
		else
		{
			AfxMessageBox("This message could not be deleted. Reloading list...\n\rIf this continues to happen please contact Nextech Support.");
		}
		// (b.spivey - June 2, 2014) - PLID 62289 
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 

	}NxCatchAll(__FUNCTION__);
}

void CDirectMessageReceivedDlg::SelChangedDirectmessageEmail(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		// (b.spivey - May 5th, 2014) PLID 61797 - Use enum
		m_strActiveEmail = VarString(m_dlEmailList->GetCurSel()->GetValue(dmalEmail));
		// (b.spivey - June 2, 2014) - PLID 62289 - 
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 
	}NxCatchAll(__FUNCTION__);	
}

void CDirectMessageReceivedDlg::RemoveMessageFromList(__int64 messageID )
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow;
#pragma TODO("remove message needs to remove the correct item by id not by cursel")
		pRow = m_dlReceivedList->GetCurSel();
		m_dlReceivedList->RemoveRow(pRow);
	}NxCatchAll(__FUNCTION__);
}

void CDirectMessageReceivedDlg::SelChangingDirectmessageEmail(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	} NxCatchAll(__FUNCTION__);
}

// b.spivey, January 22nd, 2014 - PLID 59596
void CDirectMessageReceivedDlg::OnBnClickedDirectMessageCertificate() 
{
	try {

		//Get the first one, current iteration assumes there's only ever one anyways. 
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1* FROM DirectCertificateThumbprintT ");
		CString strResult = "";

		//If we have something, lets go ahead and set the result field. 
		if(!prs->eof) {
			strResult = AdoFldString(prs->Fields, "ThumbPrint", "");
		}

		//I don't want to update or reload if nothing changed!
		CString strBefore = strResult;
			
		//If they hit OK, and the values are different, then go ahead and update/refresh. 
		if(InputBoxLimited(this, "Please enter a Direct Certificate Thumbprint ", strResult, "", 100, false, false, NULL) == IDOK 
			&& !(strBefore.Compare(strResult) == 0)) {
			
			CSqlFragment sql = ""; 
			
			//Conditionally update, they could've completely removed the row, y'know. 
			if(!strResult.IsEmpty()) {
				sql = CSqlFragment("INSERT DirectCertificateThumbprintT (ThumbPrint) VALUES ({STRING}) ", strResult); 
			}
			
			ExecuteParamSql("DELETE FROM DirectCertificateThumbprintT "
				"{SQL} ", sql); 

			//refresh
			UpdateView();
		}
	}NxCatchAll(__FUNCTION__); 
}

// (d.singleton 2014-05-06 08:49) - PLID 61802 - Add an option to hide read messages in the direct message list.
void CDirectMessageReceivedDlg::OnBnClickedFilterRead()
{
	try {
		SetRemotePropertyInt("FilterDirectMessageStatus", m_btnFilterRead.GetCheck(), 0, GetCurrentUserName());
		// (b.spivey - June 2, 2014) - PLID 62289 - There's no way to maintain the page and the updated filters, go back to page one with the new filters. 
		m_nCurrentPage = 1;
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-05-06 08:50) - PLID 61803 - Add a filter to the direct message lists to show messages within a date range filter.
void CDirectMessageReceivedDlg::OnBnClickedFilterDate()
{
	try {
		SetRemotePropertyInt("FilterDirectMessageDate", m_btnFilterDate.GetCheck(), 0, GetCurrentUserName());
		if(m_btnFilterDate.GetCheck()) {			
			m_dtFrom.EnableWindow(TRUE);
			m_dtTo.EnableWindow(TRUE);			
		}
		else {			
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}
		// (b.spivey - June 2, 2014) - PLID 62289 - There's no way to maintain the page and the new filters, go back to page 1 with the filters. 
		m_nCurrentPage = 1;
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-05-06 08:50) - PLID 61803 - Add a filter to the direct message lists to show messages within a date range filter.
void CDirectMessageReceivedDlg::OnDtnDatetimechangeFilterFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		COleDateTime dtFrom, dtTo;
		m_dtFrom.GetTime(dtFrom);
		m_dtTo.GetTime(dtTo);
		//make sure selection is valid
		if(dtFrom.GetStatus() != COleDateTime::valid) {
			AfxMessageBox("Please choose a valid \"From\" date.");
			return;
		}
		//make sure the to date is after the from date
		else if(dtFrom > dtTo){
			AfxMessageBox("Please choose a \"From\" date less than or equal to the \"To\" date.");
			m_dtFrom.SetTime(GetRemotePropertyDateTime("FilterDirectMessageDateFrom", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName(), true));
			return;
		}				
		SetRemotePropertyDateTime("FilterDirectMessageDateFrom", dtFrom, 0, GetCurrentUserName());
		*pResult = 0;

		// (b.spivey - June 2, 2014) - PLID 62289 - Update filters on kill focus.
		m_nCurrentPage = 1;
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-05-06 08:50) - PLID 61803 - Add a filter to the direct message lists to show messages within a date range filter.
void CDirectMessageReceivedDlg::OnDtnDatetimechangeFilterToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		COleDateTime dtTo, dtFrom;
		m_dtTo.GetTime(dtTo);
		m_dtFrom.GetTime(dtFrom);
		//make sure selection is valid
		if(dtTo.GetStatus() != COleDateTime::valid) {
			AfxMessageBox("Please choose a valid \"To\" date.");
			return;
		}
		//make sure the to date is after the from date
		else if(dtTo < dtFrom){
			AfxMessageBox("Please choose a \"To\" date greater than or equal to the \"From\" date.");
			m_dtTo.SetTime(GetRemotePropertyDateTime("FilterDirectMessageDateTo", &COleDateTime::GetCurrentTime(), 0, GetCurrentUserName(), true));
			return;
		}				
		SetRemotePropertyDateTime("FilterDirectMessageDateTo", dtTo, 0, GetCurrentUserName());
		*pResult = 0;

		// (b.spivey - June 2, 2014) - PLID 62289 - Update filters on kill focus. 
		m_nCurrentPage = 1;
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 

	}NxCatchAll(__FUNCTION__);
}

//function to apply filters to all rows in our list
void CDirectMessageReceivedDlg::RefreshFiltersOnList()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReceivedList->FindAbsoluteFirstRow(VARIANT_FALSE);

	for(int i = 0; pRow && i < m_dlReceivedList->GetRowCount(); i++) {		
		pRow = m_dlReceivedList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
	}
}

// (d.singleton 2014-05-07 08:27) - PLID 61800 - Mark message as unread functionality for direct messaging
void CDirectMessageReceivedDlg::RButtonDownDirectmessageReceived(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			//make this row the cur sel
			m_dlReceivedList->PutCurSel(pRow);
			//now make pop up
			BOOL bIsRead = FALSE;
			if(VarLong(pRow->GetValue(dmrlMessageState), 0) == (long)dmsRead) {
				bIsRead = TRUE;
			}

			CMenu pMenu;
			pMenu.CreatePopupMenu();

			int index = 0;
			if(bIsRead) {	
				// (d.singleton 2014-05-13 09:35) - PLID 62119 - need option to mark a message as unread in the direct message dlg
				pMenu.InsertMenu(++index, MF_BYPOSITION, ID_MARK_DIR_MESSAGE_UNREAD, "Mark as &unread");
			}
			else {
				pMenu.InsertMenu(++index, MF_BYPOSITION, ID_MARK_DIR_MESSAGE_READ, "Mark as &read");
			}

			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-05-07 11:19) - PLID 61800 - Mark message as unread functionality for direct messaging
void CDirectMessageReceivedDlg::MarkMessageAsRead()
{
	try {
		IRowSettingsPtr pRow = m_dlReceivedList->GetCurSel();
		if(pRow) {			
			NexTech_Accessor::_NullableDirectMessageStatePtr pMessageState(__uuidof(NexTech_Accessor::NullableDirectMessageState));
			pMessageState->SetNullableDirectMessageState(NexTech_Accessor::DirectMessageState_Read);
			try {
				BOOL bSuccess = GetAPI()->UpdateDirectMessageState(GetAPISubkey(), GetAPILoginToken(), VarBigInt(pRow->GetValue(dmrlID)), _bstr_t(m_strActiveEmail), pMessageState);
				if(bSuccess) {
					pRow->PutValue(dmrlMessageState, (long)dmsRead);
					pRow->PutValue(dmrlReadStateIcon, (long)m_hIconDirectMessageState);
					// (b.spivey - June 2, 2014) - PLID 62291 
					UpdateMessageCountLabel(); 
				}
			}
			catch(_com_error e) {
				CString strMessage = (LPCTSTR)e.Description();

				if(strMessage.Find("No Message for Message Uid") >= 0) {
					AfxMessageBox("Failed to mark message as read.  The file does not exist.  The list will now reload.");
					// (b.spivey - June 2, 2014) - PLID 62289 - Refresh list with the new method. 
					LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
					UpdateControls(m_dlReceivedList->GetRowCount()); 
				}
				else {
					throw e;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-05-13 09:21) - PLID 62119 - need option to mark a message as unread in the direct message dlg
void CDirectMessageReceivedDlg::MarkMessageAsUnread()
{
	try {
		IRowSettingsPtr pRow = m_dlReceivedList->GetCurSel();
		if(pRow) {			
			NexTech_Accessor::_NullableDirectMessageStatePtr pMessageState(__uuidof(NexTech_Accessor::NullableDirectMessageState));
			pMessageState->SetNullableDirectMessageState(NexTech_Accessor::DirectMessageState_New);			
			try {
				BOOL bSuccess = GetAPI()->UpdateDirectMessageState(GetAPISubkey(), GetAPILoginToken(), VarBigInt(pRow->GetValue(dmrlID)), _bstr_t(m_strActiveEmail), pMessageState);
				if(bSuccess) {
					pRow->PutValue(dmrlMessageState, (long)dmsNew);
					pRow->PutValue(dmrlReadStateIcon, (long)m_hIconPlaceholder);
				}
			}
			catch(_com_error e) {
				CString strMessage = (LPCTSTR)e.Description();

				if(strMessage.Find("No Message for Message Uid") >= 0) {
					AfxMessageBox("Failed to mark message as unread.  The file does not exist.  The list will now reload.");
					// (b.spivey - June 2, 2014) - PLID 62289 - Refresh list with the new method. 
					LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
					UpdateControls(m_dlReceivedList->GetRowCount()); 
				}
				else {
					throw e;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey - June 2, 2014) - PLID 62289 - Get the next page. 
void CDirectMessageReceivedDlg::OnBnClickedNextPage()
{
	try {
		//Advance the page count. 
		m_nCurrentPage++;

		//So this is a bizarre way to do things, but we have an edge case that is hard to pin down. 
		//	 Basically we can't know how many messages we have without grabbing the whole list (something we're trying to avoid), so when we have exactly 50 on the 
		//	 page we don't know if the next page exists or not, so what we do is we get the next page first. Then we test if there are any headers. If There are, move on as normal. 
		//	 If there aren't, then we decrement the page and update the controls, then exit early. 
		Nx::SafeArray<IUnknown *> aryHeaders = GetMessageHeaderListFromAPI(m_strActiveEmail);
		if (aryHeaders.GetLength() == 0) {
			m_nCurrentPage--;
			UpdateControls(0); 
			return; 
		}
		LoadMessagesIntoList(aryHeaders);
		UpdateControls(m_dlReceivedList->GetRowCount()); 
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey - June 2, 2014) - PLID 62289 - Get the previous page. 
void CDirectMessageReceivedDlg::OnBnClickedPrevPage()
{
	try {
		//Decrement the page count
		m_nCurrentPage--;
		//Load messages, update controls. 
		LoadMessagesIntoList(GetMessageHeaderListFromAPI(m_strActiveEmail));
		UpdateControls(m_dlReceivedList->GetRowCount()); 
	}NxCatchAll(__FUNCTION__); 
}

// (b.spivey - June 2, 2014) - PLID 62289 - Update controls based on page and row count. 
void CDirectMessageReceivedDlg::UpdateControls(long nRowCount) 
{
	//If we're on page 1, we can't go back anymore. Disable this button. 
	if (m_nCurrentPage == 1) {
		m_btnPrevPage.EnableWindow(FALSE); 
	}
	else {
		m_btnPrevPage.EnableWindow(TRUE); 
	}

	//If we have less than 50 rows, there is nothing on the next page, disable the next button. 
	if (nRowCount < 50) {
		m_btnNextPage.EnableWindow(FALSE);
	}
	else {
		m_btnNextPage.EnableWindow(TRUE); 
	}

	UpdateMessageCountLabel(); 
}

// (b.spivey - June 2, 2014) - PLID 62291 - Message count bracket on the UI to let the user know where they are in the pages. 
void CDirectMessageReceivedDlg::UpdateMessageCountLabel()
{
	//Take the number of rows in the datalist. 
	long nRowCount = m_dlReceivedList->GetRowCount();
	//If, for whatever reason, we end up with zero rows, then just get rid of the label. 
	if (nRowCount == 0) {
		m_lblMessageBracket.SetText(""); 
		return; 
	}

	CString strMessageBracket;
	//Take the (current page -1) * 50, then add 1 (page 1 gets you 1, page 2 gets you 51, etc., etc.,)
	long nStart = ((m_nCurrentPage - 1) * 50) + 1; 
	//Take the above equation but add the row count instead (page 1 would get you 50, page 2 gets you 100, etc., 
	//	 Unless you have less than 50, which would be the last page, i.e. page 2 would be 67 if page 2 had 17 results). 
	long nEnd = ((m_nCurrentPage - 1) * 50) + nRowCount; 

	//Set the bracket text to show the range we're in. 
	strMessageBracket.Format("%li - %li", nStart, nEnd);
	
	m_lblMessageBracket.SetText(strMessageBracket); 
}