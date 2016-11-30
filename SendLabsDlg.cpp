// SendLabsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SendLabsDlg.h"
#include "HL7Utils.h"
#include "HL7ParseUtils.h"
#include "HL7SettingsDlg.h" // (a.vengrofski 2010-07-20 10:43) - PLID <38919> - If you want to be able to change the settings you will need this.
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "HL7ExportDlg.h" // (r.gonet 03/18/2014) - PLID 60782 - Consequence of moving header file out of mainfrm.h into mainfrm.cpp

// CSendLabsDlg dialog
// (a.vengrofski 2010-05-28 14:40) - PLID <38919> - Created this file.
using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CSendLabsDlg, CNxDialog)

CSendLabsDlg::CSendLabsDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CSendLabsDlg::IDD, pParent)
{
	// (c.haag 2010-08-11 09:43) - PLID 39799
	m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	// (r.gonet 12/11/2012) - PLID 54116 - Create a new CHL7Client which will
	// Manage HL7 transmissions between us and NxServer. We don't care about ACKs
	// here for legacy purposes.
	m_pHL7Client = new CHL7Client_Practice();
	m_pHL7Client->SetIgnoreAcks(true);
}

CSendLabsDlg::~CSendLabsDlg()
{
	if (m_hIconRedX) DestroyIcon((HICON)m_hIconRedX);
	// (r.gonet 12/11/2012) - PLID 54116 - Free up the HL7 client
	if(m_pHL7Client) {
		delete m_pHL7Client;
		m_pHL7Client = NULL;
	}
}

enum SelectedLabColumns {

	slcGroupID = 0,
	slcPatientName,
	slcGroupName,
	slcActionTaken,		//TES 5/12/2008 - PLID 13339 - Added columns describing what action was taken (for previously processed messages).
	slcInputDate,		//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
	slcMessageType,
	slcPatientID,		// (a.vengrofski 2010-07-09 12:58) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	slcFormNumber,		// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	slcMessageID,		// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	// (z.manning 2011-06-15 16:54) - PLID 40903 - Removed message from the datalist
	//slcMessage,			// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	slcFlag,				// (c.haag 2010-08-11 09:43) - PLID 39799
};

enum UnselectedLabColumns {

	ulcGroupID = 0,
	ulcPatientName,
	ulcGroupName,
	ulcActionTaken,		//TES 5/12/2008 - PLID 13339 - Added columns describing what action was taken (for previously processed messages).
	ulcInputDate,		//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
	ulcMessageType,
	ulcPatientID,		// (a.vengrofski 2010-07-09 12:58) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	ulcFormNumber,		// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	ulcMessageID,		// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	// (z.manning 2011-06-15 16:54) - PLID 40903 - Removed message from the datalist
	//ulcMessage,			// (a.vengrofski 2010-07-16 12:59) - PLID <38919> - Added some information that was needed to send HL7 Messages.
	ulcFlag,				// (c.haag 2010-08-11 09:43) - PLID 39799
};

enum SendLabFilterColumns {

	slfcID = 0,
	slfcGroupName,
};

void CSendLabsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_ONE_HL7_SEND_LAB, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_HL7_SEND_LAB, m_btnSelectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_HL7_SEND_LAB, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_HL7_SEND_LAB, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_SEND_LABS, m_btnSendLabs);
	DDX_Control(pDX, IDC_BTN_HL7_SETTINGS, m_btnHL7Settings);
	DDX_Control(pDX, IDC_BTN_HL7_SEND_RESULTS, m_btnExportLabResults);
}


BEGIN_MESSAGE_MAP(CSendLabsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SEND_LABS, &CSendLabsDlg::OnBnClickedBtnSendLabs)
	ON_BN_CLICKED(IDC_SELECT_ONE_HL7_SEND_LAB, &CSendLabsDlg::OnBnClickedSelectOneHl7SendLab)
	ON_BN_CLICKED(IDC_SELECT_ALL_HL7_SEND_LAB, &CSendLabsDlg::OnBnClickedSelectHl7AllSendLab)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_HL7_SEND_LAB, &CSendLabsDlg::OnBnClickedUnselectHl7OneSendLab)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_HL7_SEND_LAB, &CSendLabsDlg::OnBnClickedUnselectHl7AllSendLab)
	ON_BN_CLICKED(IDC_BTN_HL7_SETTINGS, &CSendLabsDlg::OnBnClickedBtnHl7Settings)
	ON_BN_CLICKED(IDC_BTN_HL7_SEND_RESULTS, &CSendLabsDlg::OnBnClickedExportLabResults)
	ON_MESSAGE(WM_TABLE_CHANGED, &CSendLabsDlg::OnTableChanged)
END_MESSAGE_MAP()


// CSendLabsDlg message handlers

BOOL CSendLabsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnSendLabs.AutoSet(NXB_EXPORT);
		m_btnHL7Settings.AutoSet(NXB_MODIFY);

		// (c.haag 2010-08-11 09:31) - PLID 39799 - It seems silly to bulk cache one thing but we must have
		// a starting point for more properties
		g_propManager.CachePropertiesInBulk("SendLabsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'HL7Import_FailedACKCheckDelaySeconds' "
			"OR Name = 'HL7ExportLabResults' " // (d.singleton 2012-12-06 10:05) - PLID 54047
			")",
			_Q(GetCurrentUserName()));

		m_pdlSelectedList = BindNxDataList2Ctrl(IDC_HL7_SEND_LABS_SELECTED_LIST, false);		//There is no query that can display the info
		m_pdlUnselectedList = BindNxDataList2Ctrl(IDC_HL7_SEND_LABS_UNSELECTED_LIST, false);	//for these lists correctly.
		m_pdcGroupFilterCombo = BindNxDataList2Ctrl(IDC_HL7_SEND_LABS_GROUP_FILTER, true);

		// (d.singleton 2012-12-06 09:53) - PLID 54047 need to hide the send results button by default
		BOOL bCanExportResults = GetRemotePropertyInt("HL7ExportLabResults", 0, 0, "<none>", true);
		if(!bCanExportResults) {
			m_btnExportLabResults.EnableWindow(FALSE);
			m_btnExportLabResults.ShowWindow(SW_HIDE);
		}

		// (z.manning 2011-06-16 10:56) - PLID 44136 - This is now handled in UpdateView
		//PopulateLabLists();																//So the PopulateLabsList is used.
	}NxCatchAll("Error in CSendLabsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



BEGIN_EVENTSINK_MAP(CSendLabsDlg, CNxDialog)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_GROUP_FILTER, 16, CSendLabsDlg::SelChosenHl7SendLabsGroupFilter, VTS_DISPATCH)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_UNSELECTED_LIST, 3, CSendLabsDlg::DblClickCellHl7SendLabsUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_SELECTED_LIST, 3, CSendLabsDlg::DblClickCellHl7SendLabsSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_GROUP_FILTER, 18, CSendLabsDlg::RequeryFinishedHl7SendLabsGroupFilter, VTS_I2)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_UNSELECTED_LIST, 19, CSendLabsDlg::LeftClickHl7SendLabsUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_SELECTED_LIST, 19, CSendLabsDlg::LeftClickHl7SendLabsSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_UNSELECTED_LIST, 6, CSendLabsDlg::RButtonDownHl7SendLabsUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSendLabsDlg, IDC_HL7_SEND_LABS_SELECTED_LIST, 6, CSendLabsDlg::RButtonDownHl7SendLabsSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CSendLabsDlg::OnBnClickedBtnSendLabs()
{
	try{
		CString strFormNumber = "";
		long nPatientID = -1, nHL7GroupID = -1;
		BOOL bIsACK = TRUE;
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		long nNumberOfSelectedRows = m_pdlSelectedList->GetRowCount();
		if (nNumberOfSelectedRows > 0)
		{
			bool bLabNotSent = false;
			bool bPromptedForEarlyTermination = false;

			// (z.manning 2011-06-15 15:09) - PLID 40903 - We no longer have the messages in the datalist, so let's load them now.
			CArray<long,long> arynMessageIDs;
			for(pRow = m_pdlSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
				long nMessageID = VarLong(pRow->GetValue(slcMessageID));
				arynMessageIDs.Add(nMessageID);
			}
			CMap<long,long,CString,LPCTSTR> mapMessageIDToMessage;
			GetHL7MessageLogMap(arynMessageIDs, mapMessageIDToMessage);

			long nMessageID;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
			while(pRow) {
				nHL7GroupID = VarLong(pRow->GetValue(slcGroupID));
				nMessageID = VarLong(pRow->GetValue(slcMessageID));
				NXDATALIST2Lib::IRowSettingsPtr pRemoveRow = pRow;
				pRow = pRow->GetNextRow();

				// (z.manning 2011-06-15 15:12) - PLID 40903 - Get the message from the map we just made
				CString strMessage;
				if(!mapMessageIDToMessage.Lookup(nMessageID, strMessage)) {
					// (z.manning 2011-06-15 15:24) - PLID 40903 - Must have been deleted
					continue;
				}

				// (r.gonet 12/03/2012) - PLID 54116 - Changed to use refactored send function.
				HL7ResponsePtr pResponse = m_pHL7Client->SendMessageToHL7(nMessageID, hprtLab, hemtNewLab, nHL7GroupID);
				if(pResponse == NULL) {
					bLabNotSent = true;	
				}

				// (r.gonet 12/03/2012) - PLID 54116 - Now reflect the response status in the interface.
				if(pResponse->hmssSendStatus == hmssBatched || pResponse->hmssSendStatus == hmssSent || pResponse->hmssSendStatus == hmssSent_NeedsAck ||
					pResponse->hmssSendStatus == hmssSent_AckReceived || pResponse->hmssSendStatus == hmssSent_AckFailure) 
				{
					// (r.gonet 12/03/2012) - PLID 54116 - We succeeded, remove the row from the right hand side.
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlSelectedList->FindByColumn(slcMessageID, _variant_t(pResponse->nMessageID, VT_I4), m_pdlSelectedList->GetFirstRow(), VARIANT_FALSE);
					if(pRemoveRow) {
						m_pdlSelectedList->RemoveRow(pRemoveRow);//Message was sent. Remove it.
					}
				} else {
					// (r.gonet 12/03/2012) - PLID 54116 - We failed. Display an error and let them terminate the batch early.
					bLabNotSent = true;
					if(!bPromptedForEarlyTermination) {
						int nMsgBoxResult = MessageBox(FormatString(
							"One of the lab messages has failed to export for the following reason: \r\n"
							"%s\r\n"
							"\r\n"
							"Do you want to continue this batch? ",
							pResponse->strErrorMessage), NULL,
							MB_YESNO|MB_ICONERROR);
						// (r.gonet 12/03/2012) - PLID 54116 - We prompted once.
						bPromptedForEarlyTermination = true;
						if(nMsgBoxResult != IDYES) {
							// (r.gonet 12/03/2012) - PLID 54116 - They want to terminate the batch.
							break;
						}
					}
				}
			}

			if(pRow == NULL) {
				// We're finished.
				if(bLabNotSent){
					MessageBox("At least one lab failed to send.", "Sending Labs", MB_ICONERROR);
				} else {
					CClient::RefreshTable(NetUtils::HL7MessageLogT, -1, NULL, FALSE);
				}
			}
		} else {
			MessageBox("Please select at least one lab to send.", "Send labs", MB_ICONWARNING);
		}
	}NxCatchAll("Error in CSendLabsDlg::OnBnClickedBtnSendLabs");
}

void CSendLabsDlg::OnBnClickedSelectOneHl7SendLab()
{
	try {
		m_pdlSelectedList->TakeCurrentRowAddSorted(m_pdlUnselectedList, NULL);
	}NxCatchAll("Error in CSendLabsDlg::OnBnClickedSelectOneHl7SendLab");
}

void CSendLabsDlg::OnBnClickedSelectHl7AllSendLab()
{
	try {
		m_pdlSelectedList->TakeAllRows(m_pdlUnselectedList);
	}NxCatchAll("Error in CSendLabsDlg::OnBnClickedSelectHl7AllSendLab");
}

void CSendLabsDlg::OnBnClickedUnselectHl7OneSendLab()
{
	try {
		IRowSettingsPtr pRow = m_pdcGroupFilterCombo->CurSel;
		long nGroupFilter, nMessageGroup;
		if (pRow)
		{
			nGroupFilter = VarLong(pRow->GetValue(slfcID));
		}
		else
		{
			return;
		}

		pRow = m_pdlSelectedList->CurSel;

		if(pRow)
		{
			nMessageGroup = VarLong(pRow->GetValue(slcGroupID));
		}
		else
		{
			return;
		}
		if (nGroupFilter == nMessageGroup || nGroupFilter == -1)//if they are looking for this group
		{
			m_pdlUnselectedList->TakeCurrentRowAddSorted(m_pdlSelectedList, NULL);//swap
		}
		else 
		{
			m_pdlSelectedList->RemoveRow(m_pdlSelectedList->CurSel);//remove
		}

		m_pdlUnselectedList->TakeCurrentRowAddSorted(m_pdlSelectedList, NULL);
	}NxCatchAll("Error in CSendLabsDlg::OnBnClickedUnselectHl7OneSendLab");
}

void CSendLabsDlg::OnBnClickedUnselectHl7AllSendLab()
{
	try {
		CheckAndMoveRows();
	}NxCatchAll("Error in CSendLabsDlg::OnBnClickedUnselectHl7AllSendLab");
}

void CSendLabsDlg::PopulateLabLists()
{
	try {
		//find all records with a SentDate that is NULL

		//first cache an array of IDs in our selected list
		CArray<long, long> arySelectedIDs;
		//and a CString, comma-delimited
		CString strSelectedIDs = "";

		//populate the array and string
		{
			IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
			while(pRow) {

				long nMessageID = VarLong(pRow->GetValue(slcMessageID));
				arySelectedIDs.Add(nMessageID);

				if(!strSelectedIDs.IsEmpty()) {
					strSelectedIDs += ",";
				}
				strSelectedIDs += AsString(nMessageID);

				pRow = pRow->GetNextRow();
			}
		}
		CSqlFragment sqlSelectedIDsWhere;
		if (!strSelectedIDs.IsEmpty())
		{
			sqlSelectedIDsWhere.Create(" OR HL7MessageLogT.MessageID IN ({INTSTRING}) ", strSelectedIDs);
		}

		//now clear both lists
		m_pdlUnselectedList->Clear();
		m_pdlSelectedList->Clear();

		//if we have no current group selection, ensure requerying has finished, and select the first row
		long nHL7GroupID = -1;
		IRowSettingsPtr pRow = m_pdcGroupFilterCombo->GetCurSel();
		if(pRow) {
			nHL7GroupID = VarLong(pRow->GetValue(slfcID));
		}else {
			nHL7GroupID = (long)-1;//if they are not selecting a row
			m_pdcGroupFilterCombo->CurSel = m_pdcGroupFilterCombo->GetFirstRow();//make them select the all groups
		}

		CSqlFragment sqlHL7GroupID;
		if(nHL7GroupID != -1) {
			sqlHL7GroupID.Create(" AND GroupID = {INT} ", nHL7GroupID);
		}

		// (c.haag 2010-08-11 09:31) - PLID 39799 - We now include records that do have sent dates but no ACK dates for groups that
		// are expecting ACK's. The ACK usually comes almost immediately.
		//TES 6/22/2011 - PLID 44261 - The ExportType and ExpectACK settings are stored differently now.  Let's pull all the groups that
		// have both settings, and use that array in the query
		CArray<long,long> arHL7GroupsExportTcp;
		GetHL7SettingsGroupsBySetting("ExportType", (long)1, arHL7GroupsExportTcp);
		CArray<long,long> arHL7GroupsExpectACK;
		GetHL7SettingsGroupsBySetting("ExpectACK", TRUE, arHL7GroupsExpectACK);
		CArray<long,long> arHL7GroupsRequireACK;
		for(int i = 0; i < arHL7GroupsExportTcp.GetSize(); i++) {
			bool bMatched = false;
			for(int j = 0; j < arHL7GroupsExpectACK.GetSize() && !bMatched; j++) {
				if(arHL7GroupsExpectACK[j] == arHL7GroupsExportTcp[i]) {
					arHL7GroupsRequireACK.Add(arHL7GroupsExportTcp[i]);
					bMatched = true;
				}
			}
		}
		long nFailedACKCheckDelay = GetRemotePropertyInt("HL7Import_FailedACKCheckDelaySeconds", 300);
		// (z.manning 2011-06-15 17:12) - PLID 40903 - Parameterized
		// (r.gonet 02/26/2013) - PLID 47534 - Filter out dismissed messages.
		_RecordsetPtr rs = CreateParamRecordset("SELECT MessageID, GroupID, CreateDate, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"PracticeRecordType, PracticeRecordID, PersonT.ID AS PersonID, LabsT.FormNumberTextID, "
			"CONVERT(BIT, CASE WHEN (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY})) THEN 1 ELSE 0 END) AS AckMissing "
			"FROM HL7MessageLogT "
			"INNER JOIN LabsT ON LabsT.ID = HL7MessageLogT.PracticeRecordID "
			"LEFT JOIN PersonT ON LabsT.PatientID = PersonT.ID AND PracticeRecordType = {INT} "
			"WHERE (SentDate Is Null OR (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY}))) "
			"AND PracticeRecordType = {INT} "
			"AND Dismissed = 0 "
			"AND MessageType = {INT} {SQL} {SQL} "
			, nFailedACKCheckDelay, arHL7GroupsRequireACK, hprtLab, nFailedACKCheckDelay, arHL7GroupsRequireACK, hprtLab, hemtNewLab, sqlHL7GroupID, sqlSelectedIDsWhere);
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "MessageID", -1);
			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			COleDateTime dtCreateDate = AdoFldDateTime(rs, "CreateDate", COleDateTime::GetCurrentTime());
			CString strPatientName = AdoFldString(rs, "PatientName", "");
			HL7PracticeRecordType hl7RecordType = hprtLab;
			long nRecordID = AdoFldLong(rs, "PracticeRecordID", -1);
			long nPatientID = AdoFldLong(rs, "PersonID", (long)-1);
			BOOL bAckMissing = AdoFldBool(rs, "AckMissing");
			CString strFromNumberTextID = AdoFldString(rs, "FormNumberTextID", "");
			CString strMessageType = "New Lab Order";
			pRow = m_pdcGroupFilterCombo->FindByColumn(slfcID, _variant_t((long)nGroupID), m_pdcGroupFilterCombo->GetFirstRow(), VARIANT_FALSE);
			CString strGroupName = "";
			if (pRow){
				strGroupName = VarString(pRow->GetValue(slfcGroupName));
			}else {
				strGroupName = "< No Group >";
			}

			CString strActionTaken = "Waiting to be sent";

			//add to the datalist - but which one?

			//was this previously in the selected list?
			BOOL bFound = FALSE;
			for(int i=arySelectedIDs.GetSize()-1;i>=0 && !bFound;i--) {
				if(arySelectedIDs.GetAt(i) == nID) {

					bFound = TRUE;

					//it needs to go back into the selected list
					IRowSettingsPtr pRow = m_pdlSelectedList->GetNewRow();
					pRow->PutValue(slcGroupID, (long)nGroupID);
					pRow->PutValue(slcPatientName, _bstr_t(strPatientName));
					pRow->PutValue(slcGroupName, _bstr_t(strGroupName));
					pRow->PutValue(slcActionTaken, _bstr_t(strActionTaken));
					pRow->PutValue(slcInputDate, _variant_t(dtCreateDate, VT_DATE));
					pRow->PutValue(slcMessageType, _bstr_t(strMessageType));
					pRow->PutValue(slcPatientID, (long)nPatientID);
					pRow->PutValue(slcFormNumber, _bstr_t(strFromNumberTextID));
					pRow->PutValue(slcMessageID, (long)nID);
					// (c.haag 2010-08-11 09:46) - PLID 39799 - If this record was sent
					// but we never got an ACK back that we expected to get back, then
					// assign a warning icon to the record.
					if (bAckMissing) {
						pRow->PutValue(slcFlag,_variant_t((long)m_hIconRedX));
					}

					m_pdlSelectedList->AddRowSorted(pRow, NULL);

					//remove this entry to make the search faster next time
					arySelectedIDs.RemoveAt(i);
				}
			}

			//if it wasn't in the selected list, add to the unselected list
			if(!bFound) {
				IRowSettingsPtr pRow = m_pdlUnselectedList->GetNewRow();
				pRow->PutValue(ulcGroupID, (long)nGroupID);
				pRow->PutValue(ulcPatientName, _bstr_t(strPatientName));
				pRow->PutValue(ulcGroupName, _bstr_t(strGroupName));
				pRow->PutValue(ulcActionTaken, _bstr_t(strActionTaken));
				pRow->PutValue(ulcInputDate, _variant_t(dtCreateDate, VT_DATE));
				pRow->PutValue(ulcMessageType, _bstr_t(strMessageType));
				pRow->PutValue(ulcPatientID, (long)nPatientID);
				pRow->PutValue(ulcFormNumber, _bstr_t(strFromNumberTextID));
				pRow->PutValue(ulcMessageID, (long)nID);
				// (c.haag 2010-08-11 09:46) - PLID 39799 - If this record was sent
				// but we never got an ACK back that we expected to get back, then
				// assign a warning icon to the record.
				if (bAckMissing) {
					pRow->PutValue(ulcFlag,_variant_t((long)m_hIconRedX));
				}

				m_pdlUnselectedList->AddRowSorted(pRow, NULL);
			}

			rs->MoveNext();
		}
		rs->Close();

		//clear the array of selected IDs - if this array is non-empty, it
		//means that an item was previously in the selected list but is no
		//no longer available, it may have been exported by another user
		arySelectedIDs.RemoveAll();
	}NxCatchAll("Error in CSendLabsDlg::PopulateLabLists");
}

void CSendLabsDlg::SelChosenHl7SendLabsGroupFilter(LPDISPATCH lpRow)
{
	try {
		PopulateLabLists();//requery and fill data
	}NxCatchAll("Error in CSendLabsDlg::SelChosenHl7SendLabsGroupFilter");
}

void CSendLabsDlg::DblClickCellHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		MoveToOtherList(pRow,m_pdlUnselectedList,m_pdlSelectedList);//move the above row to the selected list
	}NxCatchAll("Error in CSendLabsDlg::DblClickCellHl7SendLabsUnselectedList");
}

void CSendLabsDlg::DblClickCellHl7SendLabsSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		long nGroupFilter = VarLong(m_pdcGroupFilterCombo->CurSel->GetValue(slfcID));
		long nMessageGroup = VarLong(pRow->GetValue(slcGroupID));
		if (nGroupFilter == nMessageGroup || nGroupFilter == -1)//if they are looking for this group
		{
			MoveToOtherList(pRow,m_pdlSelectedList,m_pdlUnselectedList);//move the above row to the unselected list
		}
		else 
		{
			m_pdlSelectedList->RemoveRow(pRow);//remove
		}
	}NxCatchAll("Error in CSendLabsDlg::DblClickCellHl7SendLabsSelectedList");
}

void CSendLabsDlg::MoveToOtherList(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource, NXDATALIST2Lib::_DNxDataListPtr pdlDest)
{
	pdlDest->AddRowSorted(pRow, NULL);//move it, move it
	pdlSource->RemoveRow(pRow);//remove it, remove it
}

// (r.gonet 02/26/2013) - PLID 47534 - Dismisses a message given a message ID, the row it is from, and the datalist the row is from.
void CSendLabsDlg::DismissMessage(long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource)
{
	if(!pRow || !pdlSource) {
		return;
	}

	if(IDYES != MessageBox("Are you sure you want to dismiss this lab order HL7 message? It has not been successfully sent yet.", 
		"", MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
	{
		return;
	} else {
		// (r.gonet 02/26/2013) - PLID 47534 - Proceed with dismissal!
	}

	if(DismissExportedHL7Message(nMessageID)) {
		// (r.gonet 02/26/2013) - PLID 47534 - Remove the row
		pdlSource->RemoveRow(pRow);
	} else {
		MessageBox("Practice failed to dismiss the message.", "Dismissal Error", MB_OK|MB_ICONERROR);
	}
}

void CSendLabsDlg::RequeryFinishedHl7SendLabsGroupFilter(short nFlags)
{
	try {
		IRowSettingsPtr pRow =  m_pdcGroupFilterCombo->GetNewRow();
		pRow->PutValue(slfcID, _variant_t((long)-1));
		pRow->PutValue(slfcGroupName, _bstr_t("< All Groups >"));
		m_pdcGroupFilterCombo->AddRowBefore(pRow, m_pdcGroupFilterCombo->GetFirstRow());
		m_pdcGroupFilterCombo->PutCurSel(pRow);
	}NxCatchAll("Error in CSendLabsDlg::RequeryFinishedHl7SendLabsGroupFilter");
}

void CSendLabsDlg::OnBnClickedBtnHl7Settings()
{
	try {
		// (a.vengrofski 2010-07-20 10:36) - PLID <38919> - Borrowed from BatchDlg
		//TES 5/27/2009 - PLID 34282 - Check their permission
		if(!CheckCurrentUserPermissions(bioHL7Settings, sptView)) {
			return;
		}
		CHL7SettingsDlg dlg(this);
		dlg.DoModal();

		// (a.vengrofski 2010-08-25 17:31) - PLID <38919> - Need to refresh the list when we are done.
		m_pdcGroupFilterCombo->Requery();
	} NxCatchAll("Error in CSendLabsDlg::OnBnClickedBtnHl7Settings");
}

void CSendLabsDlg::CheckAndMoveRows()
{//This function will iterate all the rows in the selected list and decide if it needs to be moved or removed
	long nGroupFilter = VarLong(m_pdcGroupFilterCombo->CurSel->GetValue(slfcID));
	if (nGroupFilter == -1)//if they are viewing all rows give them all.
	{
		m_pdlUnselectedList->TakeAllRows(m_pdlSelectedList);
		return;
	}
	long nRowGroup = -1;
	IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
	IRowSettingsPtr pRowNext = pRow;
	m_pdlSelectedList->PutCurSel(pRow);
	while (pRow)
	{
		nRowGroup = VarLong(pRow->GetValue(slcGroupID));
		if (nGroupFilter == nRowGroup)
		{
			pRowNext = pRow->GetNextRow();
			m_pdlUnselectedList->AddRowSorted(pRow,NULL);
			m_pdlSelectedList->RemoveRow(pRow);
		}
		else
		{
			pRowNext = pRow->GetNextRow();
			m_pdlSelectedList->RemoveRow(pRow);
		}
		pRow = pRowNext;
	}
}

LRESULT CSendLabsDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{// (a.vengrofski 2010-08-06 16:34) - PLID <38919> - This function will receive the table checker messages from NxServer and refresh the lists.
	try{
		//If we are here then something has changed, let's just make sure that this message is for us.
		if (wParam == NetUtils::HL7MessageLogT)
		{//There is a new message load it up.
			PopulateLabLists();
		}
	}NxCatchAll("Error in CSendLabsDlg::OnTableChanged");
	return 0;
}

// (c.haag 2010-08-11 09:43) - PLID 39799 - If the user clicked on the icon column, explain what it means
void CSendLabsDlg::LeftClickHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (ulcFlag == nCol) {
			IRowSettingsPtr pRow(lpRow);
			if (NULL != pRow && pRow->GetValue(ulcFlag).vt != VT_EMPTY) {
				AfxMessageBox("This record was previously exported, but the receiving server never responded with an acknowledgement. "
					"This could be due to a communication problem or the server not responding in a timely manner.\n\n"
					"It is recommended that you attempt to export this record again in case the server never received it originally.",
					MB_OK | MB_ICONERROR);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-08-11 09:43) - PLID 39799 - If the user clicked on the icon column, explain what it means
void CSendLabsDlg::LeftClickHl7SendLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (slcFlag == nCol) {
			IRowSettingsPtr pRow(lpRow);
			if (NULL != pRow && pRow->GetValue(slcFlag).vt != VT_EMPTY) {
				AfxMessageBox("This record was previously exported, but the receiving server never responded with an acknowledgement. "
					"This could be due to a communication problem or the server not responding in a timely manner.\n\n"
					"It is recommended that you attempt to export this record again in case the server never received it originally.",
					MB_OK | MB_ICONERROR);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSendLabsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try{
		PopulateLabLists();
	}NxCatchAll("Error in CSendLabsDlg::UpdateView");
}

void CSendLabsDlg::RButtonDownHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		m_pdlUnselectedList->PutCurSel(pRow);

		const long nMessageID = VarLong(pRow->GetValue(ulcMessageID), -1);
		// (r.gonet 02/26/2013) - PLID 47534 - Add in the row and datalist.
		PopupMenu(nMessageID, pRow, m_pdlUnselectedList);
	}
	NxCatchAll(__FUNCTION__);
}

void CSendLabsDlg::RButtonDownHl7SendLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		m_pdlSelectedList->PutCurSel(pRow);

		const long nMessageID = VarLong(pRow->GetValue(slcMessageID), -1);
		// (r.gonet 02/26/2013) - PLID 47534 - Add in the row and datalist
		PopupMenu(nMessageID, pRow, m_pdlSelectedList);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-08 16:56) - PLID 38753
// (r.gonet 02/26/2013) - PLID 47534 - Added the row and datalist parameters
void CSendLabsDlg::PopupMenu(const long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource)
{
	CMenu mnu;
	mnu.CreatePopupMenu();

	enum MenuItems {
		miDismissMessage = 1, // (r.gonet 02/26/2013) - PLID 47534 - Added an option to dismiss the current message.
		miViewMessage = 2, // (r.gonet 02/26/2013) - PLID 47534 - Shifted menu option.
		
	};

	// (r.gonet 02/26/2013) - PLID 47534 - Add the dismiss menu item and a separator
	mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miDismissMessage, "&Dismiss");
	mnu.AppendMenu(MF_SEPARATOR);
	mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miViewMessage, "&View HL7 Message");

	CPoint pt;
	GetCursorPos(&pt);
	int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

	switch(nRet)
	{
		// (r.gonet 02/26/2013) - PLID 47534 - Handle when the user chooses to dismiss.
		case miDismissMessage:
			DismissMessage(nMessageID, pRow, pdlSource);
			break;
		case miViewMessage:
			ViewHL7ExportMessage(nMessageID);
			break;
	}
}

// (d.singleton 2012-10-19 14:15) - PLID 53282
void CSendLabsDlg::OnBnClickedExportLabResults()
{
	try {
		CHL7ExportDlg dlg;
		dlg.DoModal(hprtLabResult);
	}NxCatchAll(__FUNCTION__);
}