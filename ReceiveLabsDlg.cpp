// ReceiveLabsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "HL7Utils.h"
#include "HL7ParseUtils.h"
#include "HL7SettingsDlg.h" // (a.vengrofski 2010-07-20 10:43) - PLID <38919> - If you want to be able to change the settings you will need this.
#include "ReceiveLabsDlg.h"
#include "FileUtils.h"
#include "FinancialRc.h"
#include "DecisionRuleUtils.h"
#include <NxHL7Lib/HL7Logging.h>
#include "HL7LogDlg.h"

// CReceiveLabsDlg dialog
// (a.vengrofski 2010-07-22 10:45) - PLID <38919> - Created this file.
using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CReceiveLabsDlg, CNxDialog)

// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider - Added tablechecker
CReceiveLabsDlg::CReceiveLabsDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CReceiveLabsDlg::IDD, pParent),
  m_providerChecker(NetUtils::Providers)
{

}

CReceiveLabsDlg::~CReceiveLabsDlg()
{
}

enum SelectedLabColumns {

	slcID = 0,
	slcGroupID,
	slcGroupName,
	slcPatientName,
	slcGender,		// (s.dhole 2013-08-13 09:15) - PLID 58019 Added Gender
	slcDOB,		// (s.dhole 2013-08-13 09:15) - PLID 58019 Assed Birth Date
	slcDescription,
	slcAction,
	slcMessageDate,		// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
	slcMessageType,
	slcEventType,
	// (z.manning 2011-06-15 11:18) - PLID 40903 - Removed the message column
	//slcMessage,
	slcActionTaken,		//TES 5/12/2008 - PLID 13339 - Added columns describing what action was taken (for previously processed messages).
	slcActionID,
	slcInputDate,		//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
};

enum UnselectedLabColumns {

	ulcID = 0,
	ulcGroupID,
	ulcGroupName,
	ulcPatientName,
	ulcGender,		// (s.dhole 2013-08-13 09:15) - PLID 58019 Added Gender
	ulcDOB,		// (s.dhole 2013-08-13 09:15) - PLID 58019 Assed Birth Date
	ulcDescription,
	ulcAction,
	ulcMessageDate,		// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
	ulcMessageType,
	ulcEventType,
	// (z.manning 2011-06-15 11:18) - PLID 40903 - Removed the message column
	//ulcMessage,
	ulcActionTaken,		//TES 5/12/2008 - PLID 13339 - Added columns describing what action was taken (for previously processed messages).
	ulcActionID,
	ulcInputDate,		//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
};

enum ReceiveLabFilterColumns {

	rlfcID = 0,
	rlfcGroupName,
};

// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
enum ProviderFilterColumns{
	pfcProviderID = 0,
	pfcProviderName,
};

void CReceiveLabsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_ONE_HL7_RECEIVE_LAB, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_HL7_RECEIVE_LAB, m_btnSelectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_HL7_RECEIVE_LAB, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_HL7_RECEIVE_LAB, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_RECEIVE_LABS, m_btnReceiveLabs);
	DDX_Control(pDX, IDC_DISMISS_HL7_LAB_RESULTS_BTN, m_btnDismissLabResults);
	DDX_Control(pDX, IDC_BTN_HL7_SETTINGS, m_btnHL7Settings);
	DDX_Control(pDX, IDC_BTN_CHECK_FOR_MESSAGES, m_btnHL7NewMessages);
	DDX_Control(pDX, IDC_SHOWPREVIMPORTEDLABS, m_btnShowPrevImported);
	DDX_Control(pDX, IDC_OPEN_HL7_LOG_BTN, m_btnHL7Log);
}

BEGIN_MESSAGE_MAP(CReceiveLabsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_HL7_SETTINGS, &CReceiveLabsDlg::OnBnClickedBtnHl7Settings)
	ON_BN_CLICKED(IDC_BTN_RECEIVE_LABS, &CReceiveLabsDlg::OnBnClickedBtnReceiveLabs)
	ON_BN_CLICKED(IDC_SELECT_ONE_HL7_RECEIVE_LAB, &CReceiveLabsDlg::OnBnClickedSelectOneHl7ReceiveLab)
	ON_BN_CLICKED(IDC_SELECT_ALL_HL7_RECEIVE_LAB, &CReceiveLabsDlg::OnBnClickedSelectAllHl7ReceiveLab)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_HL7_RECEIVE_LAB, &CReceiveLabsDlg::OnBnClickedUnselectOneHl7ReceiveLab)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_HL7_RECEIVE_LAB, &CReceiveLabsDlg::OnBnClickedUnselectAllHl7ReceiveLab)
	ON_BN_CLICKED(IDC_BTN_CHECK_FOR_MESSAGES, &CReceiveLabsDlg::OnBnClickedBtnCheckForMessages)
	ON_MESSAGE(WM_TABLE_CHANGED, &CReceiveLabsDlg::OnTableChanged)
	ON_BN_CLICKED(IDC_SHOWPREVIMPORTEDLABS, &CReceiveLabsDlg::OnBnClickedShowprevimportedlabs)
	ON_BN_CLICKED(IDC_DISMISS_HL7_LAB_RESULTS_BTN, &CReceiveLabsDlg::OnBnClickedDismissHl7LabResultsBtn)
	ON_BN_CLICKED(IDC_OPEN_HL7_LOG_BTN, &CReceiveLabsDlg::OnBnClickedOpenHl7LogBtn)
END_MESSAGE_MAP()

// CReceiveLabsDlg message handlers
BEGIN_EVENTSINK_MAP(CReceiveLabsDlg, CNxDialog)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_GROUP_FILTER, 16, CReceiveLabsDlg::SelChosenHl7ReceiveLabsGroupFilter, VTS_DISPATCH)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_GROUP_FILTER, 18, CReceiveLabsDlg::RequeryFinishedHl7ReceiveLabsGroupFilter, VTS_I2)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_UNSELECTED_LIST, 3, CReceiveLabsDlg::DblClickCellHl7ReceiveLabsUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_SELECTED_LIST, 3, CReceiveLabsDlg::DblClickCellHl7ReceiveLabsSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_UNSELECTED_LIST, 6, CReceiveLabsDlg::RButtonDownHl7ReceiveLabsUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CReceiveLabsDlg, IDC_HL7_RECEIVE_LABS_SELECTED_LIST, 6, CReceiveLabsDlg::RButtonDownHl7ReceiveLabsSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CReceiveLabsDlg, IDC_NXDL_PROVIDER_FILTER, 16, CReceiveLabsDlg::SelChosenProvidersFilter, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CReceiveLabsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnReceiveLabs.AutoSet(NXB_EXPORT);
		m_btnHL7Settings.AutoSet(NXB_MODIFY);
		m_btnHL7NewMessages.AutoSet(NXB_MODIFY);
		// (r.gonet 05/01/2014) - PLID 49432 - Give the HL7 log button an icon that looks like a log.
		m_btnHL7Log.AutoSet(NXB_SCRIPT_ATTENTION);
		// (r.gonet 03/01/2013) - PLID 48419 - Don't show the dismiss button for regular users.
		m_btnDismissLabResults.AutoSet(NXB_DELETE);
		if(GetCurrentUserID() != -26) {
			m_btnDismissLabResults.EnableWindow(FALSE);
			m_btnDismissLabResults.ShowWindow(SW_HIDE);
		} else {
			m_btnDismissLabResults.EnableWindow(TRUE);
			m_btnDismissLabResults.ShowWindow(SW_SHOW);
		}

		// (j.dinatale 2011-09-16 15:39) - PLID 40024
		m_btnShowPrevImported.SetCheck(FALSE);

		m_pdlSelectedList = BindNxDataList2Ctrl(IDC_HL7_RECEIVE_LABS_SELECTED_LIST, false);		//There is no query that can display the info
		m_pdlUnselectedList = BindNxDataList2Ctrl(IDC_HL7_RECEIVE_LABS_UNSELECTED_LIST, false);	//for these lists correctly.
		m_pdcGroupFilterCombo = BindNxDataList2Ctrl(IDC_HL7_RECEIVE_LABS_GROUP_FILTER, true);

		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		m_pProviderFilterCombo = BindNxDataList2Ctrl(IDC_NXDL_PROVIDER_FILTER);
		m_pProviderFilterCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		//	Add an ALL option to the top of the list
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProviderFilterCombo->GetNewRow();
		//	(b.savon 2011-10-11) - PLID 42805 - Initialize the 'ALL' provider ID
		pRow->PutValue(pfcProviderID, (long)-1);
		pRow->PutValue(pfcProviderName, _bstr_t("< All Providers >"));
		m_pProviderFilterCombo->AddRowBefore(pRow, m_pProviderFilterCombo->GetFirstRow());
		pRow = m_pProviderFilterCombo->FindByColumn( pfcProviderName,_bstr_t(GetRemotePropertyText("ReceiveLabsProviderFilter", "< All Providers >", 0, GetCurrentUserName())), m_pProviderFilterCombo->GetFirstRow(), TRUE);
		if( pRow ){
			m_pProviderFilterCombo->PutComboBoxText(_bstr_t(GetRemotePropertyText("ReceiveLabsProviderFilter", "< All Providers >", 0, GetCurrentUserName())));
		} else{
			m_pProviderFilterCombo->PutComboBoxText(_bstr_t("< All Providers >"));
			SetRemotePropertyText("ReceiveLabsProviderFilter", VarString(m_pProviderFilterCombo->GetComboBoxText()), 0, GetCurrentUserName());
		}

		// (r.gonet 06/22/2011) - PLID 37414 - Added caching
		g_propManager.CachePropertiesInBulk("CReceiveLabsDlg-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'Labs_UseCustomToDoText' "
			"OR Name = 'Lab_DefaultTodoPriority' " //TES 9/6/2013 - PLID 51147
			"OR Name = 'AssignNewPatientSecurityCode' " // (r.gonet 04/22/2014) - PLID 53170
			")",
			_Q(GetCurrentUserName()));

		// (b.savon 2011-09-28 10:28) - PLID 42805 - Added ReceiveLabsProviderFilter
		g_propManager.CachePropertiesInBulk("CReceiveLabsDlg-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"	Name = 'Labs_CustomToDoText' OR "
			"	Name = 'ReceiveLabsProviderFilter' "
			")",
			_Q(GetCurrentUserName()));

		// (z.manning 2011-06-16 10:56) - PLID 44136 - This is now handled in UpdateView
		//PopulateLabLists();																//So the PopulateLabsList is used.
	}NxCatchAll("Error in CReceiveLabsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CReceiveLabsDlg::SelChosenHl7ReceiveLabsGroupFilter(LPDISPATCH lpRow)
{
	try {
		PopulateLabLists();//requery and fill data
	}NxCatchAll("Error in CReceiveLabsDlg::SelChosenHl7ReceiveLabsGroupFilter");
}

void CReceiveLabsDlg::RequeryFinishedHl7ReceiveLabsGroupFilter(short nFlags)
{
	try{
		IRowSettingsPtr pRow =  m_pdcGroupFilterCombo->GetNewRow();
		pRow->PutValue(rlfcID, _variant_t((long)-1));
		pRow->PutValue(rlfcGroupName, _bstr_t("< All Groups >"));
		m_pdcGroupFilterCombo->AddRowBefore(pRow, m_pdcGroupFilterCombo->GetFirstRow());
		m_pdcGroupFilterCombo->PutCurSel(pRow);
	}NxCatchAll("Error in CReceiveLabsDlg::RequeryFinishedHl7ReceiveLabsGroupFilter");
}

void CReceiveLabsDlg::OnBnClickedBtnHl7Settings()
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

		// (b.spivey, January 31, 2013) - PLID 54895 - Need to update the view in case they changed what type of 
		//		ORUs the link accepts. 
		UpdateView(true); 

	} NxCatchAll("Error in CReceiveLabsDlg::OnBnClickedBtnHl7Settings");
}

// (a.vengrofski 2010-07-23 10:51) - PLID <38919> - Modified this function for labs only
void CReceiveLabsDlg::OnBnClickedBtnReceiveLabs()
{ 
	try {

		// (j.jones 2008-04-11 16:47) - PLID 29596 - this code was lifted and modified
		// from the old CHL7ImportDlg::OnCommitAllPending() function

		if(m_pdlSelectedList->GetRowCount() == 0) {
			AfxMessageBox("There are no messages in the Import Selected list. Please select some messages before importing.");
			return;
		}

		//TES 5/18/2009 - PLID 34282 - Make sure we have permission for all record types being imported
		IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
		bool bLabsFound = false;
		bool bPrevImportedFound = false;
		while(pRow != NULL && (!bLabsFound)) {
			CString strMessageType = VarString(pRow->GetValue(slcMessageType),"");
			CString strEventType = VarString(pRow->GetValue(slcEventType),"");
			HL7ImportRecordType hirt = GetRecordType(strMessageType, strEventType);
			if(hirt == hirtLab) bLabsFound = true;

			// (j.dinatale 2011-09-16 17:03) - PLID 40024 - check for previously imported messages
			long nActionID = VarLong(pRow->GetValue(slcActionID), -1);
			if(nActionID != -1) {
				bPrevImportedFound = true;
			}

			pRow = pRow->GetNextRow();
		}
		if(bLabsFound && !CheckCurrentUserPermissions(bioHL7Labs, sptDynamic0)) {
			return;
		}

		// (j.dinatale 2011-09-16 17:03) - PLID 40024 - Dont allow previously imported messages to be reimported
		if(bPrevImportedFound){
			AfxMessageBox("There are previously processed or dismissed lab messages in the selected list. Please unselect these messages and run the import again.");
			return;
		}


		//(e.lally 2007-06-14) PLID 26326 - Decided to give a warning prior to running this code.
		if(IDNO == MessageBox("This will commit all the pending HL7 messages in the Import Selected Records list. This action cannot be undone. "
			"Are you sure you wish to continue?",NULL, MB_YESNO)) {
				return;
		}

		//TES 5/12/2008 - PLID 13339 - We need to give them an extra warning if there are rows in the list that have previously
		// been committed.
		BOOL bWarned = FALSE;
		pRow = m_pdlSelectedList->GetFirstRow();
		while(pRow != NULL && !bWarned) {
			long nActionID = VarLong(pRow->GetValue(slcActionID),-1);
			if(nActionID == mqaCommit) {
				if(IDYES != MsgBox(MB_YESNO, "At least one selected message has previously been committed.  Re-committing selected messages may lead to duplicated records, "
					"or may overwrite changes to existing records.  Are you SURE you wish to do this?")) {
						return;
				}
				bWarned = TRUE;
			}
			pRow = pRow->GetNextRow();
		}

		// (z.manning 2011-06-15 15:09) - PLID 40903 - We no longer have the messages in the datalist, so let's load them now.
		CArray<long,long> arynMessageIDs;
		for(pRow = m_pdlSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long nMessageID = VarLong(pRow->GetValue(slcID));
			arynMessageIDs.Add(nMessageID);
		}
		CMap<long,long,CString,LPCTSTR> mapMessageIDToMessage;
		GetHL7MessageQueueMap(arynMessageIDs, mapMessageIDToMessage);

		BOOL bCommitFailed = FALSE;
		BOOL bOneImported = FALSE;

		long nCountSkipped_NoBatchAllowed = 0;

		//TES 10/31/2013 - PLID 59251 - Track any new interventions that get triggered
		CDWordArray arNewCDSInterventions;

		//TES 8/8/2007 - PLID 26892 - In the case of Update Patient messages, we need some special handling.  
		// Here's what we'll do: we go through each row once, and either successfully import the message, in which 
		// case we simply remove the row, fail to automatically link the patient, in which case we store the 
		// ThirdPartyID in the row and move on, or fail to commit the message entirely, in which case we leave the 
		// row as is.  Then we will prompt the user to link all the patients we couldn't auto-link, and then go back
		// through all the rows that have a ThirdPartyID stored and commit them based on the user's input, and remove
		// them from the list.  Then if there are any rows left in the list, we'll tell them that some failed, otherwise,
		// we'll report success.

		//TES 8/8/2007 - PLID 26892 - We need a separate dialog for each HL7 group.
		//CArray<CHL7LinkMultiplePatientsDlg*,CHL7LinkMultiplePatientsDlg*> arLinkMultiples;

		//(e.lally 2007-06-14) PLID 26326 - Add ability to commit all pending messages.
		{
			IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
			while(pRow) {
				// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
				long nHL7GroupID = VarLong(pRow->GetValue(slcGroupID), -1);
				BeginNewHL7Transaction(GetNewAdoConnection(CString((LPCTSTR)GetRemoteData()->GetDefaultDatabase())), GetHL7SettingInt(nHL7GroupID, "CurrentLoggingLevel"));

				CString strEventType = VarString(pRow->GetValue(slcEventType), "");

				// (j.jones 2008-08-21 13:41) - PLID 29596 - The old import disallowed committing multiple
				// 'new bill' messages at once, but did so by disabling the Import button. Instead of doing
				// that,  just skip messages we cannot process in a batch, and warn at the end of the import
				// that this occurred.
				if(!IsCommitAllEventSupported(strEventType)) {

					nCountSkipped_NoBatchAllowed++;

					bCommitFailed = TRUE;

					pRow = pRow->GetNextRow();
					continue;
				}

				const long nMessageID = VarLong(pRow->GetValue(slcID));

				// (z.manning 2011-06-15 15:12) - PLID 40903 - Get the message from the map we just made
				CString strMessage;
				if(!mapMessageIDToMessage.Lookup(nMessageID, strMessage)) {
					// (z.manning 2011-06-15 15:24) - PLID 40903 - Must have been deleted
					pRow = pRow->GetNextRow();
					continue;
				}

				//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
				HL7Message Message;
				Message.nMessageID = nMessageID;
				Message.strMessage = strMessage;
				Message.nHL7GroupID = VarLong(pRow->GetValue(slcGroupID), -1);
				Message.strHL7GroupName = VarString(pRow->GetValue(slcGroupName), "");
				Message.strPatientName = VarString(pRow->GetValue(slcPatientName), "");
				//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
				Message.dtInputDate = VarDateTime(pRow->GetValue(slcInputDate));

				BOOL bUpdatingPatients = FALSE;
				//if(strEventType == "A08" || strEventType == "A31") {
				//	bUpdatingPatients = TRUE;
				//}

				HL7_PIDFields PID;
				// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
				ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData());
				ENotFoundResult nfr = nfrFailure;
				//TES 9/18/2008 - PLID 31414 - Renamed
				long nPersonID = GetPatientFromHL7Message(PID, Message.nHL7GroupID, this, nfbSkip, &nfr);
				//It would be more efficient to commit the HL7 event all at once, not one at a time, but that is
				//not the primary concern here
				//TES 4/21/2008 - PLID 29721 - Added parameters for auditing.
				// (j.jones 2008-05-19 16:11) - PLID 30110 - don't auto-send an HL7 tablechecker, we will do it ourselves
				//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
				if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) {
					//it succeeded!  Now we need to update the data to have the correct action
					//TES 8/8/2007 - PLID 27020 - Go ahead and commit this data now, it's not much slower
					// that way, and doing that ensures that if there's a crash partway through, we'll still
					// know which messages were processed already.
					/*strTempSql.Format("UPDATE HL7MessageQueueT SET ActionID = %li WHERE ID = %li; \r\n", mqaCommit, VarLong(pRow->GetValue(mqcID)));
					strSql += strTempSql;*/
					//TES 4/18/2008 - PLID 29657 - CommitHL7Event updates HL7MessageQueueT now.
					//ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}; \r\n", mqaCommit, nID);

					bOneImported = TRUE;

					//now remove the row
					//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
					if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
						pRow->PutValue(slcActionID, (long)mqaCommit);
						pRow->PutValue(slcActionTaken, _bstr_t("Committed"));
					}
					else {
						IRowSettingsPtr pRowToRemove = pRow;
						pRow = pRow->GetNextRow();

						m_pdlSelectedList->RemoveRow(pRowToRemove);
						continue;
					}

				}
				else {
					bCommitFailed = TRUE;
				}

				pRow = pRow->GetNextRow();
			}
		}

		//refresh the screen to remove any committed rows
		UpdateView();

		if(bCommitFailed == FALSE) {
			AfxMessageBox("All messages were successfully committed.");
		}
		else {
			// (r.gonet 05/01/2014) - PLID 49432 - Tell the client where to find information about the error(s).
			AfxMessageBox("Practice failed to commit at least one message. Please refer to the HL7 Log for details.");
		}

		//TES 10/31/2013 - PLID 59251 - If this triggered any interventions, notify the user
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

	}NxCatchAll("Error in CHL7BatchDlg::OnBtnHl7Import");
}

void CReceiveLabsDlg::DblClickCellHl7ReceiveLabsUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		// (j.dinatale 2011-11-09 11:10) - PLID 40024 - no longer need the source list
		MoveToOtherList(pRow,m_pdlSelectedList);//move the above row to the selected list
	}NxCatchAll("Error in CReceiveLabsDlg::DblClickCellHl7ReceiveLabsUnselectedList");
}

void CReceiveLabsDlg::DblClickCellHl7ReceiveLabsSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		long nGroupFilter = VarLong(m_pdcGroupFilterCombo->CurSel->GetValue(rlfcID));
		long nMessageGroup = VarLong(pRow->GetValue(slcGroupID));
		if (nGroupFilter == nMessageGroup || nGroupFilter == -1)//if they are looking for this group
		{
			// (j.dinatale 2011-11-09 11:10) - PLID 40024 - no longer need the source list
			MoveToOtherList(pRow,m_pdlUnselectedList);//move the above row to the unselected list
		}
		else 
		{
			m_pdlSelectedList->RemoveRow(pRow);//remove
		}

		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		PopulateLabLists();

	}NxCatchAll("Error in CReceiveLabsDlg::DblClickCellHl7ReceiveLabsSelectedList");
}

void CReceiveLabsDlg::MoveToOtherList(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlDest)
{
	// (j.dinatale 2011-11-09 09:06) - PLID 40024 - we can make a call to TakeRowAddSorted here
	pdlDest->TakeRowAddSorted(pRow);
}

// (r.gonet 02/26/2013) - PLID 48419 - Dismisses the lab result given a message id, the row the message is in, and the datalist that the row is in.
// (r.gonet 03/01/2013) - PLID 48419 - Altered to return value indicating success or failure.
bool CReceiveLabsDlg::DismissMessage(long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource)
{
	// (r.gonet 02/26/2013) - PLID 48419 - Should only be performed by tech support.
	if(GetCurrentUserID() != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
		// (r.gonet 02/26/2013) - PLID 48419 - They shouldn't be getting here. But just in case.
		MessageBox("Only NexTech Technical Support can dismiss a lab result HL7 message. Please contact NexTech Technical Support if you need to dismiss a lab result HL7 message.", "Cannot Dismiss", MB_OK|MB_ICONERROR);
		return false;
	}

	if(!pRow || !pdlSource) {
		return false;
	}

	// (r.gonet 02/26/2013) - PLID 48419 - Call our new shared dismissal function.
	if(DismissImportedHL7Message(nMessageID)) {
		if(m_btnShowPrevImported.GetCheck()) {
			// (r.gonet 02/26/2013) - PLID 48419 - Don't remove if we are showing the previously imported messages.
			if(pdlSource == m_pdlSelectedList) {
				pRow->PutValue(slcActionID, (long)mqaDismiss);
				pRow->PutValue(slcActionTaken, _bstr_t("Dismissed"));
			} else {
				pRow->PutValue(ulcActionID, (long)mqaDismiss);
				pRow->PutValue(ulcActionTaken, _bstr_t("Dismissed"));
			}
		} else {
			// (r.gonet 02/26/2013) - PLID 48419 - remove the row
			pdlSource->RemoveRow(pRow);
		}
		return true;
	} else {
		return false;
	}
}

// (j.dinatale 2011-11-10 10:57) - PLID 40024 - reverted this back, since it was a datalist issue
// (j.dinatale 2011-11-09 09:06) - PLID 40024 - reworked this function to also pull the color of the row with it
void CReceiveLabsDlg::OnBnClickedSelectOneHl7ReceiveLab()
{
	try {
		m_pdlSelectedList->TakeCurrentRowAddSorted(m_pdlUnselectedList, NULL);
	}NxCatchAll("Error in CReceiveLabsDlg::OnBnClickedSelectOneHl7ReceiveLab");
}

// (j.dinatale 2011-11-10 10:57) - PLID 40024 - reverted this back, since it was a datalist issue
// (j.dinatale 2011-11-09 09:06) - PLID 40024 - reworked this function to also pull the color of the row with it
void CReceiveLabsDlg::OnBnClickedSelectAllHl7ReceiveLab()
{
	try {
		m_pdlSelectedList->TakeAllRows(m_pdlUnselectedList);
	}NxCatchAll("Error in CReceiveLabsDlg::OnBnClickedSelectAllHl7ReceiveLab");
}

void CReceiveLabsDlg::OnBnClickedUnselectOneHl7ReceiveLab()
{
	try {
		IRowSettingsPtr pRow = m_pdcGroupFilterCombo->CurSel;
		long nGroupFilter, nMessageGroup;
		if (pRow)
		{
			nGroupFilter = VarLong(pRow->GetValue(rlfcID));
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
			// (j.dinatale 2011-11-09 11:16) - PLID 40024 - Didnt add any extra logic here to take rows, since we repopulate the entire list anyways
			m_pdlUnselectedList->TakeCurrentRowAddSorted(m_pdlSelectedList, NULL);//swap
		}
		else 
		{
			m_pdlSelectedList->RemoveRow(m_pdlSelectedList->CurSel);//remove
		}

		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		PopulateLabLists();
	}NxCatchAll("Error in CReceiveLabsDlg::OnBnClickedUnselectOneHl7ReceiveLab");
}

void CReceiveLabsDlg::OnBnClickedUnselectAllHl7ReceiveLab()
{
	try {
		CheckAndMoveRows();

		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		PopulateLabLists();
	}NxCatchAll("Error in CReceiveLabsDlg::OnBnClickedUnselectAllHl7ReceiveLab");
}

void CReceiveLabsDlg::PopulateLabLists()
{
	try {

		//first cache an array of IDs in our selected list
		CArray<long, long> arySelectedIDs;
		//and a CString, comma-delimited
		CString strSelectedIDs;

		//populate the array and string
		IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
		{
			while(pRow) {

				long nMessageID = VarLong(pRow->GetValue(slcID));
				arySelectedIDs.Add(nMessageID);

				if(!strSelectedIDs.IsEmpty()) {
					strSelectedIDs += ",";
				}
				strSelectedIDs += AsString(nMessageID);

				pRow = pRow->GetNextRow();
			}
		}

		// (j.dinatale 2011-09-19 17:13) - PLID 40024 - Rows will be added if they were previously selected anyways, no need to have the query explicitly request
		//	those selected messages.
		/*CSqlFragment sqlSelectedIDsWhere;
		if (!strSelectedIDs.IsEmpty())
		{
			sqlSelectedIDsWhere.Create(" OR HL7MessageQueueT.ID IN ({INTSTRING}) ", strSelectedIDs);
		}*/

		//now clear both lists
		m_pdlUnselectedList->Clear();
		m_pdlSelectedList->Clear();

		//find our type filter
		long nHL7GroupID = -1;
		pRow = m_pdcGroupFilterCombo->GetCurSel();
		if(pRow) {
			nHL7GroupID = VarLong(pRow->GetValue(slcID));
		}else {
			nHL7GroupID = (long)-1;//if they are not selecting a row
			m_pdcGroupFilterCombo->CurSel = m_pdcGroupFilterCombo->GetFirstRow();//make them select the all groups
		}

		CSqlFragment sqlHL7GroupID;			//no group selected, therefore we show all
		if(nHL7GroupID != -1) {
			sqlHL7GroupID.Create(" AND GroupID = {INT} ", nHL7GroupID);
		}

		// (j.dinatale 2011-09-16 15:32) - PLID 40024 - Need to check to see if we are including previously imported messages
		CSqlFragment sqlActionID;
		if(!m_btnShowPrevImported.GetCheck()){
			sqlActionID.Create(" ActionID Is Null AND ");
		}

		//pull all available messages from the data that match our filter
		//can't parameterize the where clause here
		// (z.manning 2011-06-15 16:40) - PLID 40903 - Parameterized
		// (j.dinatale 2011-09-16 15:36) - PLID 40024 - we only check the ActionID if we dont want to have all previously imported messages
		// (j.dinatale 2011-09-19 17:20) - PLID 40024 - removed the sql frag which ensured that any selected messages were pulled forward
		// (d.singleton 2013-01-28 15:44) - PLID 54895 - need to remove any ORU^R01 messages that are for history document links
		// (s.dhole 2013-08-13 09:15) - PLID 58019 added PatientDOB and PatientGender
		// (r.goldschmidt 2015-11-02 12:01) - PLID 66437 - We need to know if the message was purged
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT HL7MessageQueueT.ID, GroupID, HL7SettingsT.Name AS GroupName, PatientName, Description, \r\n"
			"	MessageType, EventType, ActionID, InputDate, \r\n"
			"	{SQL} AS MessageDateString ,\r\n"
			"	PatientDOB,  ISNULL(PatientGender,-1)  AS  PatientSex, \r\n"
			"	CASE WHEN HL7MessageQueueT.PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged \r\n"
			"FROM HL7MessageQueueT \r\n"
			"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID \r\n"
			"WHERE {SQL} (MessageType = 'ORU' AND EventType = 'R01' AND GroupID NOT IN (SELECT HL7GroupID FROM HL7GenericSettingsT WHERE Name = 'UsePatientHistoryImportMSH_3' AND BitParam = 1)) {SQL} \r\n"
			, GetMessageDateStringSql(), sqlActionID, sqlHL7GroupID);
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID", -1);
			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			CString strGroupName = AdoFldString(rs, "GroupName", "");
			CString strPatientName = AdoFldString(rs, "PatientName", "");
			CString strDescription = AdoFldString(rs, "Description", "");
			CString strMsgType = AdoFldString(rs, "MessageType", "");
			CString strEvent = AdoFldString(rs, "EventType", "");
			long nActionID = AdoFldLong(rs, "ActionID", -1);
			//TES 10/16/2008 - PLID 31712 - We'll also need the input date
			COleDateTime dtInput = AdoFldDateTime(rs, "InputDate");

			//TES 5/12/2008 - PLID 13339 - We need to fill in the "Action Taken" column.
			CString strAction = GetActionDescriptionForHL7Event(strMsgType, strEvent);
			CString strActionTaken = GetMessageActionDescription(nActionID);

			// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
			// (z.manning 2011-06-15 16:45) - PLID 40903 - We no longer need the whole message for this
			// (r.goldschmidt 2015-11-02 12:01) - PLID 66437 - If the message was purged, set the message date to the input date.
			//								Although this isn't the true message date, it is close enough for the purposes of info/sorting
			COleDateTime dtMessageDate;
			if (!!AdoFldBool(rs, "WasPurged")) {
				dtMessageDate = AdoFldDateTime(rs, "InputDate");
			}
			else {
				CString strMessageDateString = AdoFldString(rs, "MessageDateString", "");
				dtMessageDate = GetHL7DateFromStringField(strMessageDateString, nGroupID);
				if (dtMessageDate.m_status == COleDateTime::invalid) {
					//GetHL7MessageDate should never have sent back an invalid date
					ASSERT(FALSE);
					dtMessageDate = COleDateTime::GetCurrentTime();
				}
			}

			// (s.dhole 2013-08-13 09:15) - PLID 58019 load Gender and DOB
			CString strPatientGender; 
			switch(AdoFldLong (rs, "PatientSex")) {
				case 0:
					strPatientGender = "Other";
					break;
				case 1:
					strPatientGender = "Male";
					break;
				case 2:
					strPatientGender = "Female";
					break;	
				default:
					strPatientGender="";
					break;
			}
			
			_variant_t varPatientDOB = rs->Fields->Item["PatientDOB"]->Value; 
	

			//add to the datalist - but which one?

			//was this previously in the selected list?
			BOOL bFound = FALSE;
			for(int i=arySelectedIDs.GetSize()-1;i>=0 && !bFound;i--) {
				if(arySelectedIDs.GetAt(i) == nID) {

					bFound = TRUE;

					//it needs to go back into the selected list
					IRowSettingsPtr pRow = m_pdlSelectedList->GetNewRow();
					pRow->PutValue(slcID, nID);
					pRow->PutValue(slcGroupID, nGroupID);
					pRow->PutValue(slcGroupName, _bstr_t(strGroupName));
					pRow->PutValue(slcPatientName, _bstr_t(strPatientName));
					// (s.dhole 2013-08-29 14:13) - PLID 58019 load Gender and DOB
					pRow->PutValue(slcGender, _bstr_t(strPatientGender));
					if (varPatientDOB.vt == VT_DATE ){
						pRow->PutValue(slcDOB, varPatientDOB);
					}
					else{
						pRow->PutValue(slcDOB, g_cvarNull);
					}
					pRow->PutValue(slcDescription, _bstr_t(strDescription));
					pRow->PutValue(slcAction, _bstr_t(strAction));
					// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
					pRow->PutValue(slcMessageDate, _variant_t(dtMessageDate, VT_DATE));
					pRow->PutValue(slcMessageType, _bstr_t(strMsgType));					
					pRow->PutValue(slcEventType, _bstr_t(strEvent));
					//TES 5/12/2008 - PLID 13339 - Added two new columns for information about previously processed messages.
					pRow->PutValue(slcActionTaken, _bstr_t(strActionTaken));
					pRow->PutValue(slcActionID, nActionID);
					//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
					pRow->PutValue(slcInputDate, COleVariant(dtInput));
					m_pdlSelectedList->AddRowSorted(pRow, NULL);

					// (j.dinatale 2011-09-16 16:06) - PLID 40024 - make the row forecolor gray if dismissed, green if imported
					if(nActionID == 2){
						pRow->PutForeColor(RGB(127,127,127));
					}else if(nActionID == 1){
						pRow->PutForeColor(RGB(0,128,0));
					}

					//remove this entry to make the search faster next time
					arySelectedIDs.RemoveAt(i);
				}
			}


			//if it wasn't in the selected list, add to the unselected list
			if(!bFound) {
				IRowSettingsPtr pRow = m_pdlUnselectedList->GetNewRow();
				pRow->PutValue(ulcID, nID);
				pRow->PutValue(ulcGroupID, nGroupID);
				pRow->PutValue(ulcGroupName, _bstr_t(strGroupName));
				pRow->PutValue(ulcPatientName, _bstr_t(strPatientName));
				pRow->PutValue(ulcDescription, _bstr_t(strDescription));
				// (s.dhole 2013-08-13 09:15) - PLID 58019 load Gender and DOB
				pRow->PutValue(ulcGender, _bstr_t(strPatientGender));
				if (varPatientDOB.vt == VT_DATE ){
					pRow->PutValue(ulcDOB, varPatientDOB);
				}
				else{
					pRow->PutValue(ulcDOB, g_cvarNull);
				}
				pRow->PutValue(ulcAction, _bstr_t(strAction));
				// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
				pRow->PutValue(ulcMessageDate, _variant_t(dtMessageDate, VT_DATE));
				pRow->PutValue(ulcMessageType, _bstr_t(strMsgType));
				pRow->PutValue(ulcEventType, _bstr_t(strEvent));
				//TES 5/12/2008 - PLID 13339 - Added two new columns for information about previously processed messages.
				pRow->PutValue(ulcActionTaken, _bstr_t(strActionTaken));
				pRow->PutValue(ulcActionID, nActionID);
				//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
				pRow->PutValue(ulcInputDate, COleVariant(dtInput));
				m_pdlUnselectedList->AddRowSorted(pRow, NULL);

				// (j.dinatale 2011-09-16 16:06) - PLID 40024 - make the row forecolor gray if dismissed, green if imported
				if(nActionID == 2){
					pRow->PutForeColor(RGB(127,127,127));
				}else if(nActionID == 1){
					pRow->PutForeColor(RGB(0,128,0));
				}
			}

			rs->MoveNext();
		}
		rs->Close();

		//clear the array of selected IDs - if this array is non-empty, it
		//means that an item was previously in the selected list but is no
		//no longer available, it may have been imported by another user
		arySelectedIDs.RemoveAll();



		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		//	Filter the Unselected Labs List only if we have a provider selected.
		if( VarString(m_pProviderFilterCombo->GetComboBoxText()) != "< All Providers >" ){
			FilterLabListsByProvider();
		}//END Provider Filter

	}NxCatchAll("Error in CHL7BatchDlg::RequeryImportLists");
}

CString CReceiveLabsDlg::GetMessageActionDescription(long nActionID)
{
	switch(nActionID) {
	case mqaCommit:
		return "Committed";
		break;
	case mqaDismiss:
		return "Dismissed";
		break;
	default:
		return "Waiting";
		break;
	}
}
// (a.vengrofski 2010-07-23 14:17) - PLID <38919> - Borrowed from HL7BatchDlg.cpp
void CReceiveLabsDlg::ProcessHL7Files(BOOL bSilent)
{
	try {

		//this code was lifted and modified from the old CHL7ImportDlg::OnImportFromFile() function

		BOOL bOneImported = FALSE;
		BOOL bOneFailed = FALSE;
		//TES 6/25/2009 - PLID 34355 - Track if we failed to delete any files.
		BOOL bOneDeleteFailed = FALSE;
		// (a.wilson 2013-05-16 11:27) - PLID 41682 - user for making an error message more descriptive.
		bool bOneUnsupportedMessage = false;
		//TES 3/16/2011 - PLID 41912 - Used for making a more helpful error message
		BOOL bPossibleFacilityIDMismatch = FALSE;

		//TES 4/7/2008 - PLID 27364 - Track if there any files we can't access.
		int nLockedFiles = 0;

		//set m_bAutoProcessHL7Files to TRUE, such that we assume it will work,
		//and if anything says otherwise during the process, then it will be
		//set to FALSE at that time
		BOOL m_bAutoProcessHL7Files = TRUE;

		//TES 10/31/2013 - PLID 59251 - Track if any interventions get triggered
		CDWordArray arNewCDSInterventions;

		//get the information for each group
		// (j.jones 2008-05-05 10:08) - PLID 29600 - pull the BatchImports setting
		//TES 3/16/2011 - PLID 41912 - Pull the ForceFacilityIDMatch and LabFacilityID settings
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
		CArray<long,long> arHL7GroupIDs;
		GetAllHL7SettingsGroups(arHL7GroupIDs);
		for(int i = 0; i < arHL7GroupIDs.GetSize(); i++) {
			long nGroupID = arHL7GroupIDs[i];
			CString strName = GetHL7GroupName(nGroupID);
			long nType = GetHL7SettingInt(nGroupID, "ImportType");
			BOOL bBatchImports = GetHL7SettingBit(nGroupID, "BatchImports");
			//only process groups with an import type of 'file'
			if (nType == 0) {		
				CString strFolder = GetHL7SettingText(nGroupID, "ImportFile");
				if ((! FileUtils::DoesFileOrDirExist(strFolder)) || (strFolder.IsEmpty())) {
					CString strMsg;
					strMsg.Format("The folder you have specified for HL7 Imports for group '%s' is not valid.  Please edit your HL7 Settings for this group, and enter a valid path in the Filename field in the Import section.", strName);
					if(!bSilent) {
						AfxMessageBox(strMsg);
					}
					//log that this failed
					Log("CHL7BatchDlg::ProcessHL7Files message 1: " + strMsg);

					//since this failed, set m_bAutoProcessHL7Files to false so we don't auto-attempt this again upon refresh
					m_bAutoProcessHL7Files = FALSE;
					continue;
				}
				CString strExtension = GetHL7SettingText(nGroupID, "ImportExtension");

				//loop through the files in the folder and import each one
				CFileFind finder;
				CString strFile;
				int state = 2;

				if (finder.FindFile(strFolder ^ "*."+strExtension))
				{
					while (state)
					{	if (!finder.FindNextFile())
					state = 0;

					if(!finder.IsDots() && !finder.IsDirectory()) {
						strFile = finder.GetFileName();
						if(!strExtension.IsEmpty() || strFile.Find(".") == -1) {
							CString strMessage;
							try
							{

								//we found one so import it
								CFile InFile;
								if(!InFile.Open(strFolder ^ strFile,  CFile::modeRead | CFile::shareCompat)) {
									//TES 4/7/2008 - PLID 27364 - We can't access it.  Most likely it's just still being
									// written to by the third-party app.  Just skip this file.
									nLockedFiles++;
								}
								else {
									const int LEN_16_KB = 16384;
									CString strIn;	//input string
									long iFileSize = InFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
									strIn.ReleaseBuffer(iFileSize);

									while (iFileSize == LEN_16_KB) {

										strMessage += strIn;
										iFileSize = InFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
										//TES 9/11/2008 - PLID 31415 - Need to pass in the file size!
										strIn.ReleaseBuffer(iFileSize);
									}

									strMessage += strIn;

									//TES 6/24/2009 - PLID 34355 - Now, before going any farther, attempt to delete
									// the file.  If we can't delete it, we can't import it (because otherwise we'll
									// just keep re-importing it).
									InFile.Close();

									// (r.gonet 05/01/2014) - PLID 61843 - We're starting a new HL7 message transaction
									BeginNewHL7Transaction(GetNewAdoConnection(CString((LPCTSTR)GetRemoteData()->GetDefaultDatabase())), GetHL7SettingInt(nGroupID, "CurrentLoggingLevel"));

									//TES 4/21/2008 - PLID 29721 - Added strHL7GroupName parameter for auditing.
									// (j.jones 2008-05-05 10:09) - PLID 29600 - added bBatchImports parameter
									// (j.jones 2008-05-19 16:11) - PLID 30110 - don't send a tablechecker, we will do it ourselves
									//TES 6/25/2009 - PLID 34355 - Put this in its own try...catch, so that if it
									// errors out we will still attempt to re-create the original message.
									// (z.manning 2010-06-28 16:19) - PLID 38896 - I reorganized this code so that we add
									// the message first, delete the file and if the delete fails we then delete the message.
									// I also put this code in an ADO transaction for added safety in case something happens to
									// the SQL connection between adding the message to data and attempting to delete it.
									HL7Message message;
									BOOL bSuccess = FALSE;
									{
										CSqlTransaction sqlTran("EnqueueHL7Message");
										sqlTran.Begin();
										// (a.wilson 2013-05-08 17:43) - PLID 41682 - support result enum.
										HL7AddMessageToQueueResult amtqResult = AddMessageToQueue(strMessage, nGroupID, strName, FALSE, message);
										if(amtqResult.bSuccess == true) {
											if(DeleteFile(strFolder ^ strFile)) {
												bOneImported = TRUE;
												bSuccess = TRUE;
											}
											else {
												// (r.gonet 05/01/2014) - PLID 61843 - Write to the HL7 log about this deletion error
												CString strLog = FormatString("Failed to delete file '%s' and will now attempt to delete newly created HL7MessageQueueT record (ID = %li)", strFolder ^ strFile, message.nMessageID);
												Log(strLog);
												LogHL7Error(strLog);
												if(message.nMessageID != -1) {
													if(GetHL7Transaction()) {
														GetHL7Transaction()->SetHL7MessageID(-1);
													}
													// (r.gonet 05/01/2014) - PLID 61843 - The HL7 transaction now references no saved message, which is OK
													// (r.gonet 2016-05-24 9:11) - PLID-66576 - Removed a stray comma between the statements.
													ExecuteParamSql(
														"UPDATE HL7TransactionT SET ImportMessageID = NULL WHERE ImportMessageID = {INT}; "
														"DELETE FROM HL7MessageQueueT WHERE ID = {INT}"
														, message.nMessageID, message.nMessageID);
												}
												bOneDeleteFailed = TRUE;
												bOneFailed = TRUE;
											}
										}
										else {
											// (a.wilson 2013-05-08 17:44) - PLID 41682 - if the event type was not supported
											//	and everyting else worked fine then we need to move the message into a folder
											//	named "Unsupported".
											if (amtqResult.eccErrorCode == eccEventNotSupported) {
												//attempt to move the file to a seperate folder in the same path. don't worry if it fails.
												if (!MoveUnsupportedHL7Message(strFolder, strFile)) {
													Log("Failed to move file %s to the unsupported HL7 message folder.", strFolder ^ strFile);
												}
												bOneUnsupportedMessage = true;
											}
											//TES 3/16/2011 - PLID 41912 - If they're configured to force matching Facility IDs, then there's a high
											// probability that that's why this failed.
											else if (amtqResult.eccErrorCode == eccUnknownKeyIdentifier && GetHL7SettingBit(nGroupID, "ForceFacilityIDMatch") && !GetHL7SettingText(nGroupID, "LabFacilityID").IsEmpty()) {
												bPossibleFacilityIDMismatch = TRUE;
											}
											bOneFailed = TRUE;
										}

										sqlTran.Commit();
									}

									// (j.jones 2008-05-05 10:00) - PLID 29600 - only auto-commit if the group setting says to do so
									// (z.manning 2010-06-29 09:54) - PLID 38896 - I moved this code out of AddMessageToQueue
									if(bSuccess && !bBatchImports && message.nMessageID != -1) {
										//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
										CommitHL7Event(message, this, FALSE, arNewCDSInterventions);
									}

									// (z.manning 2010-06-28 16:25) - PLID 38896 - The rearrangment of the above code makes this
									// all unnecessary.
									/*else {
									//TES 6/25/2009 - PLID 34355 - It failed, re-create the original file.
									CFile OutFile;
									bool bRecreated = false;
									if(!DoesExist(strFolder ^ strFile)) {
									if(OutFile.Open(strFolder ^ strFile, CFile::modeWrite|CFile::modeCreate|CFile::shareCompat)) {
									OutFile.Write(strMessage.GetBuffer(strMessage.GetLength()), strMessage.GetLength());
									strMessage.ReleaseBuffer();
									OutFile.Close();
									bRecreated = true;
									}
									}
									if(!bRecreated) {
									//TES 6/25/2009 - PLID 34355 - Uh-oh!  We failed to re-create it, but
									// we haven't saved it to data.  We need to make sure it's not lost,
									// so let's create it as a temporary file and have them save it somewhere.
									CString strTmpFileName;
									HANDLE hFile = CreateNxTempFile("RecoveredHL7Message", strExtension, &strTmpFileName);
									if(hFile == INVALID_HANDLE_VALUE) {
									//TES 6/25/2009 - PLID 34355 - We can't even create a temp file!
									// We're just going to have to put it on screen.
									CMsgBox dlg;
									dlg.msg = "An HL7 message failed to import, and then could not be restored to its original location.  "
									"Please copy the text below, paste it into a text editor (such as Notepad or Microsoft Word), and save it.  "
									"Additionally, please contact NexTech Technical Support to notify them of this problem.\r\n"
									"\r\n"
									"WARNING! If you click OK on this message before copying the text to another location, the message will be PERMANENTLY LOST!\r\n"
									"\r\n" + strMessage;
									dlg.DoModal();
									}
									else {
									MsgBox("An HL7 Message could not be imported, and additionally could not be restored to its original location.  "
									"It will now be opened on screen in Notepad.  Please save the file to a location where you will be able to "
									"find it again, and call NexTech Technical Support.");
									// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
									ShellExecute((HWND)this, NULL, "notepad.exe", strTmpFileName, NULL, SW_SHOW);
									}
									}
									}*/
								}

							} NxCatchAll("Error in CHL7BatchDlg::ProcessHL7Files : ProcessFile");
						}
					}
					}
				}
			}

		}

		// (j.jones 2008-05-19 16:11) - PLID 30110 - send a tablechecker ourselves
		//refresh the table
		if(bOneImported) {
			CClient::RefreshTable(NetUtils::HL7MessageQueueT, -1, NULL, FALSE);
		}

		//now requery our lists to re-display results
		PopulateLabLists();

		if(bOneFailed) {
			CString strMsg;
			//TES 6/25/2009 - PLID 34355 - Give a different message if the failure was due to lack of delete permissions.
			if(bOneDeleteFailed) {
				strMsg = "Some messages were not imported because they could not be deleted out of the import path.  Please ensure that you have permission to delete files in your HL7 Import folder.";
			} else {
				// (a.wilson 2013-05-16 11:34) - PLID 41682 - add error message to explain unsupported events.
				if (bPossibleFacilityIDMismatch || bOneUnsupportedMessage) {
					//TES 3/16/2011 - PLID 41912 - If we think the problem might have been the facility ID, then say as much.
					strMsg.Format("Some messages could not be successfully imported for the following reasons:\r\n%s%s",
						(bOneUnsupportedMessage ? "\r\n- One or more messages were unsupported HL7 events." : ""), 
						(bPossibleFacilityIDMismatch ? "\r\n- The messages may have been incorrectly formatted, "
						"or their Facility ID may not match the Facility ID specified in your HL7 Settings." : ""));
				}
				else {
					strMsg = "Some messages could not be successfully imported.";
				}
			}
			if(!bSilent) {					
				AfxMessageBox(strMsg);
			}
			//log that this failed
			Log("CHL7BatchDlg::ProcessHL7Files message 2: " + strMsg);
		}
		else {
			if(bOneImported) {
				if(!bSilent) {
					AfxMessageBox("All messages imported successfully!");
				}
			}
			else {
				if(!bSilent) {
					AfxMessageBox("No messages were found in your Import path(s).");
				}
			}
		}

		if(nLockedFiles > 10) {
			//TES 4/7/2008 - PLID 27364 - Hmm, there may be something else going on here.  Let's give the user a heads up.
			CString strMsg;
			strMsg.Format("The HL7 Link has detected an unusually high number of inaccessible files (%i) in your HL7 Import folder(s).  "
				"These files may be in use by another program, or you may not have sufficient permissions to access them.\r\n\r\n"
				"There may not be anything wrong with your system; however, if you continue to get this message, please contact NexTech Technical Support to investigate this issue.",
				nLockedFiles);
			if(!bSilent) {
				MsgBox(strMsg);
			}
			//log that this failed
			Log("CHL7BatchDlg::ProcessHL7Files message 3: " + strMsg);
		}

		//TES 10/31/2013 - PLID 59251 - If this triggered any interventions, notify the user, even if bSilent is true (that's only intended to suppress 
		// matching prompts and the like
		//if(!bSilent) {
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		//}

	}NxCatchAll("Error in CHL7BatchDlg::ProcessHL7Files");
}


void CReceiveLabsDlg::OnBnClickedBtnCheckForMessages()
{
	try {
		//process any pending HL7 files, with bSilent turned off
		ProcessHL7Files(FALSE);
	}NxCatchAll("Error in CHL7BatchDlg::OnBtnCheckForMessages");
}

//This function will iterate all the rows in the selected list and decide if it needs to be moved or removed
void CReceiveLabsDlg::CheckAndMoveRows()
{
	long nGroupFilter = VarLong(m_pdcGroupFilterCombo->CurSel->GetValue(rlfcID));
	if (nGroupFilter == -1)//if they are viewing all rows give them all.
	{
		// (j.dinatale 2011-11-09 11:16) - PLID 40024 - Didnt add any extra logic here to take rows, since we repopulate the entire list anyways
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


LRESULT CReceiveLabsDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{// (a.vengrofski 2010-08-06 16:34) - PLID <38919> - This function will receive the table checker messages from NxServer and refresh the lists.
	try{
		//If we are here then something has changed, let's just make sure that this message is for us.
		if (wParam == NetUtils::HL7MessageQueueT)
		{//There is a new message load it up.
			PopulateLabLists();
		}
	}NxCatchAll("Error in CReceiveLabsDlg::OnTableChanged");
	return 0;
}
void CReceiveLabsDlg::RButtonDownHl7ReceiveLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		m_pdlUnselectedList->PutCurSel(pRow);
		if(pRow == NULL) {
			return;
		}

		//add an ability to commit the row
		enum 
		{
			eCommit = 1,
			eDismiss,	// (r.gonet 02/26/2013) - PLID 48419 - Menu option to dismiss lab results
			eViewMessage, // (z.manning 2011-07-08 16:51) - PLID 38753
		};

		//build + popup the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		//TES 5/18/2009 - PLID 34282 - Check whether they have permission to import this type of record, and disable
		// the menu item if not.
		if(!(GetCurrentUserPermissions(bioHL7Labs) & SPT______0_____ANDPASS)) 
		{
			mnu.AppendMenu(MF_DISABLED|MF_GRAYED, eCommit, "&Commit");
		} else 
		{
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCommit, "&Commit");
		}
		// (r.gonet 02/26/2013) - PLID 48419 - The Dismiss menu option is only visible to NexTech staff because of patient safety concerns.
		if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			if(VarLong(pRow->GetValue(ulcActionID), -1) == -1) {
				// (r.gonet 02/26/2013) - PLID 48419 - Add normally. This is a pending result.
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eDismiss, "&Dismiss");
			} else {
				// (r.gonet 02/26/2013) - PLID 48419 - This is a dismissed or comitted message, disable this menu item.
				mnu.AppendMenu(MF_DISABLED|MF_GRAYED, eDismiss, "&Dismiss");
			}
		} else {
			// (r.gonet 02/26/2013) - PLID 48419 - We are currently not letting a normal user dismiss lab results for patient safety.
		}
		mnu.AppendMenu(MF_SEPARATOR);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eViewMessage, "&View HL7 Message");

		//and show the popup
		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		const long nMessageID = VarLong(pRow->GetValue(ulcID), -1);
		if(nRet == eCommit) 
		{
			// (j.dinatale 2011-11-09 09:13) - PLID 40024 - prevent previously commited messages from being committed again
			long nActionID = VarLong(pRow->GetValue(ulcActionID), -1);
			if(nActionID != -1){
				if(nActionID == 2){
					AfxMessageBox("The message could not be committed because it has been dismissed.");
				}else if(nActionID == 1){
					AfxMessageBox("The message could not be committed because it has already been processed.");
				}

				return;
			}

			//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
			HL7Message Message;
			Message.nMessageID = nMessageID;
			// (z.manning 2011-06-15 16:49) - PLID 40903 - Message is no longer in the datalist
			Message.strMessage = VarString(GetTableField("HL7MessageQueueT", "Message", "ID", Message.nMessageID), "");
			Message.nHL7GroupID = VarLong(pRow->GetValue(ulcGroupID), -1);
			Message.strHL7GroupName  = VarString(pRow->GetValue(ulcGroupName), "");
			Message.strPatientName = VarString(pRow->GetValue(ulcPatientName), "");
			//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
			Message.dtInputDate = VarDateTime(pRow->GetValue(ulcInputDate));

			// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
			BeginNewHL7Transaction(GetNewAdoConnection(CString((LPCTSTR)GetRemoteData()->GetDefaultDatabase())), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));

			//TES 10/31/2013 - PLID 59251 - Track if any interventions get triggered
			CDWordArray arNewCDSInterventions;
			if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) 
			{//success!
				// (j.dinatale 2011-11-10 10:33) - PLID 40024 - If showing prev messages, set the actionID to 1 (committed) and change the color to green.
				if(m_btnShowPrevImported.GetCheck()){
					pRow->PutValue(slcActionID, 1);
					pRow->PutForeColor(RGB(0,128,0));
				}else{
					m_pdlUnselectedList->RemoveRow(pRow);
				}
				CClient::RefreshTable(NetUtils::HL7MessageQueueT, -1, NULL, FALSE);
			} else 
			{
				// (r.gonet 05/01/2014) - PLID 49432 - Tell the client where to find information about the error(s).
				AfxMessageBox("Practice failed to commit the message. Please refer to the HL7 Log for details.");
			}

			//TES 10/31/2013 - PLID 59251 - If this triggered any interventions, notify the user
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}
		// (r.gonet 02/26/2013) - PLID 48419 - Handle the choice of dismissing a message.
		else if(nRet == eDismiss) {
			long nActionID = VarLong(pRow->GetValue(ulcActionID), -1);
			// (r.gonet 02/26/2013) - PLID 48419 - Don't allow dismissal if the message is committed or dismissed already. Shouldn't be possible to get here.
			if(nActionID != -1){
				if(nActionID == 2){
					AfxMessageBox("The message could not be dismissed because it has already been dismissed.");
				}else if(nActionID == 1){
					AfxMessageBox("The message could not be dismissed because it has been processed.");
				}

				return;
			}

			// (r.gonet 02/26/2013) - PLID 48419 - Give a warning. Do set the default to Yes so that somebody doing mulitple of these won't have
			//  as much mouse movement to do.
			if(IDYES != MessageBox("Are you SURE you want to dismiss this lab result HL7 message? If this lab result is for a live patient, make sure you have the client's written permission.", 
				"", MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1))
			{
				return;
			} else {
				// (r.gonet 02/26/2013) - PLID 48419 - Proceed with dismissal!
			}

			// (r.gonet 02/26/2013) - PLID 48419 - Proceed with dismissal.
			if(!DismissMessage(nMessageID, pRow, m_pdlUnselectedList)) {
				MessageBox("Practice failed to dismiss the message.", "Dismissal Error", MB_OK|MB_ICONERROR);
			}
		}
		else if(nRet == eViewMessage)
		{
			ViewHL7ImportMessage(nMessageID);
		}

	}NxCatchAll("Error in CReceiveLabsDlg::RButtonDownHl7ReceiveLabsUnselectedList");
}

void CReceiveLabsDlg::RButtonDownHl7ReceiveLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		m_pdlSelectedList->PutCurSel(pRow);
		if(pRow == NULL) {
			return;
		}

		//add an ability to commit the row
		enum 
		{
			eCommit = 1,
			eDismiss, // (r.gonet 02/26/2013) - PLID 48419 - Menu option to dismiss a lab result.
			eViewMessage, // (z.manning 2011-07-08 16:51) - PLID 38753
		};

		//build + popup the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		//TES 5/18/2009 - PLID 34282 - Check whether they have permission to import this type of record, and disable
		// the menu item if not.
		if(!(GetCurrentUserPermissions(bioHL7Labs) & SPT______0_____ANDPASS)) 
		{
			mnu.AppendMenu(MF_DISABLED|MF_GRAYED, eCommit, "&Commit");
		} else 
		{
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCommit, "&Commit");
		}
		// (r.gonet 02/26/2013) - PLID 48419 - The Dismiss menu option is only visible to NexTech staff because of patient safety concerns.
		if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			if(VarLong(pRow->GetValue(slcActionID), -1) == -1) {
				// (r.gonet 02/26/2013) - PLID 48419 - Add normally. This is a pending result.
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eDismiss, "&Dismiss");
			} else {
				// (r.gonet 02/26/2013) - PLID 48419 - This is a dismissed or comitted message, disable this menu item.
				mnu.AppendMenu(MF_DISABLED|MF_GRAYED, eDismiss, "&Dismiss");
			}
		} else {
			// (r.gonet 02/26/2013) - PLID 48419 - We are currently not letting a normal user dismiss lab results for patient safety.
		}
		mnu.AppendMenu(MF_SEPARATOR);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eViewMessage, "&View HL7 Message");

		//and show the popup
		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		const long nMessageID = VarLong(pRow->GetValue(ulcID), -1);
		if(nRet == eCommit) 
		{
			// (j.dinatale 2011-11-09 09:13) - PLID 40024 - prevent previously commited messages from being committed again
			long nActionID = VarLong(pRow->GetValue(slcActionID), -1);
			if(nActionID != -1){
				if(nActionID == 2){
					AfxMessageBox("The message could not be committed because it has been dismissed.");
				}else if(nActionID == 1){
					AfxMessageBox("The message could not be committed because it has already been processed.");
				}

				return;
			}

			//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
			HL7Message Message;
			Message.nMessageID = nMessageID;
			// (z.manning 2011-06-15 16:49) - PLID 40903 - Message is no longer in the datalist
			Message.strMessage = VarString(GetTableField("HL7MessageQueueT", "Message", "ID", Message.nMessageID), "");
			Message.nHL7GroupID = VarLong(pRow->GetValue(ulcGroupID), -1);
			Message.strHL7GroupName  = VarString(pRow->GetValue(ulcGroupName), "");
			Message.strPatientName = VarString(pRow->GetValue(ulcPatientName), "");
			//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
			Message.dtInputDate = VarDateTime(pRow->GetValue(ulcInputDate));

			// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
			BeginNewHL7Transaction(GetNewAdoConnection(CString((LPCTSTR)GetRemoteData()->GetDefaultDatabase())), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));

			//TES 10/31/2013 - PLID 59251 - Track if any interventions get triggered
			CDWordArray arNewCDSInterventions;
			if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) 
			{//success!
				// (j.dinatale 2011-11-10 10:33) - PLID 40024 - If showing prev messages, set the actionID to 1 (committed) and change the color to green.
				//		Move it to the uselected list because it is now committed
				if(m_btnShowPrevImported.GetCheck()){
					pRow->PutValue(slcActionID, 1);
					pRow->PutForeColor(RGB(0,128,0));
					m_pdlUnselectedList->TakeRowAddSorted(pRow);
				}else{
					m_pdlSelectedList->RemoveRow(pRow);
				}
				CClient::RefreshTable(NetUtils::HL7MessageQueueT, -1, NULL, FALSE);
			} else 
			{
				// (r.gonet 05/01/2014) - PLID 49432 - Tell the client where to find information about the error(s).
				AfxMessageBox("Practice failed to commit the message. Please refer to the HL7 Log for details.");
			}

			//TES 10/31/2013 - PLID 59251 - If this triggered any interventions, notify the user
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}
		// (r.gonet 02/26/2013) - PLID 48419 - Handle the choice of dismissing a message.
		else if(nRet == eDismiss) {
			long nActionID = VarLong(pRow->GetValue(slcActionID), -1);
			// (r.gonet 02/26/2013) - PLID 48419 - Don't allow dismissal if the message is committed or dismissed already. Shouldn't be possible to get here.
			if(nActionID != -1){
				if(nActionID == 2){
					AfxMessageBox("The message could not be dismissed because it has already been dismissed.");
				}else if(nActionID == 1){
					AfxMessageBox("The message could not be dismissed because it has been processed.");
				}

				return;
			}

			// (r.gonet 02/26/2013) - PLID 48419 - Give a warning. Do set the default to Yes so that somebody doing mulitple of these won't have
			//  as much mouse movement to do.
			if(IDYES != MessageBox("Are you SURE you want to dismiss this lab result HL7 message? If this lab result is for a live patient, make sure you have the client's written permission.", 
				"", MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1))
			{
				return;
			} else {
				// (r.gonet 02/26/2013) - PLID 48419 - Proceed with dismissal!
			}

			// (r.gonet 02/26/2013) - PLID 48419 - Proceed with dismissal.
			if(!DismissMessage(nMessageID, pRow, m_pdlSelectedList)) {
				MessageBox("Practice failed to dismiss the message.", "Dismissal Error", MB_OK|MB_ICONERROR);
			}
		}
		else if(nRet == eViewMessage)
		{
			ViewHL7ImportMessage(nMessageID);
		}
	}NxCatchAll("Error in CReceiveLabsDlg::RButtonDownHl7ReceiveLabsSelectedList");
}

void CReceiveLabsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try{
		// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
		//											 Added tablechecker.
		if (m_providerChecker.Changed()){
			m_pProviderFilterCombo->Requery();
			m_pProviderFilterCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			//	Add an ALL option to the top of the list
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pProviderFilterCombo->GetNewRow();
			//	(b.savon 2011-10-11) - PLID 42805 - Initialize the 'ALL' provider ID
			pRow->PutValue(pfcProviderID, (long)-1);
			pRow->PutValue(pfcProviderName, _bstr_t("< All Providers >"));
			m_pProviderFilterCombo->AddRowBefore(pRow, m_pProviderFilterCombo->GetFirstRow());
			if( m_pProviderFilterCombo->FindByColumn( pfcProviderName,_bstr_t(GetRemotePropertyText("ReceiveLabsProviderFilter", "< All Providers >", 0, GetCurrentUserName())), m_pProviderFilterCombo->GetFirstRow(), TRUE) ){
				m_pProviderFilterCombo->PutComboBoxText(_bstr_t(GetRemotePropertyText("ReceiveLabsProviderFilter", "< All Providers >", 0, GetCurrentUserName())));
			} else{
				m_pProviderFilterCombo->PutComboBoxText(_bstr_t("< All Providers >"));
				SetRemotePropertyText("ReceiveLabsProviderFilter", VarString(m_pProviderFilterCombo->GetComboBoxText()), 0, GetCurrentUserName());
			}
		}

		PopulateLabLists();
	}NxCatchAll("Error in CReceiveLabsDlg::UpdateView");
}

// (j.dinatale 2011-09-16 15:40) - PLID 40024
void CReceiveLabsDlg::OnBnClickedShowprevimportedlabs()
{
	try{
		PopulateLabLists();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
void CReceiveLabsDlg::SelChosenProvidersFilter(LPDISPATCH lpRow)
{
	if( lpRow != NULL ){
		try {
			//	Set the current filter in the combo text.
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pProviderFilterCombo->GetCurSel();
			m_pProviderFilterCombo->PutComboBoxText(_bstr_t(pRow->GetValue(pfcProviderName)));
			//	Save filter selection for another day.
			SetRemotePropertyText("ReceiveLabsProviderFilter", VarString(pRow->GetValue(pfcProviderName)), 0, GetCurrentUserName());

			//	PopulateLabLists calls FilterLabListsByProvider() at the end of the method.
			PopulateLabLists();
		}NxCatchAll("CReceiveLabsDlg::SelChosenProvidersFilter - Unable to select provider.");
	} else{
		try{
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pProviderFilterCombo->FindByColumn(pfcProviderName,_bstr_t(GetRemotePropertyText("ReceiveLabsProviderFilter", "< All Providers >", 0, GetCurrentUserName())), m_pProviderFilterCombo->GetFirstRow(), VARIANT_TRUE);
			m_pProviderFilterCombo->PutComboBoxText(_bstr_t(pRow->GetValue(pfcProviderName)));
		}NxCatchAll("CReceiveLabsDlg::SelChosenProvidersFilter - Unable to filter provider.");
	}
}//END CReceiveLabsDlg::SelChosenProvidersFilter(LPDISPATCH lpRow)

// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
void CReceiveLabsDlg::FilterLabListsByProvider()
{
	try{
		//	Declare array of all rows to be removed.
		CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> paryRowsToRemove;

		//	Get the first row of the Unselected Labs list.
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pdlUnselectedList->GetFirstRow();

		//	Iterate through the list of Unselected labs, grab their IDs, and concatenate a string
		CString strSingleID = _T("");
		CString strMessageIDs = _T("");
		while(pRow){
			strSingleID.Format("%d, ", VarLong(pRow->GetValue(ulcID), -1));
			strMessageIDs += strSingleID;
			pRow = pRow->GetNextRow();
		}//END while( iterate rows )

		//	Strip off the trailing comma and space
		strMessageIDs = strMessageIDs.Left(strMessageIDs.GetLength()-2);

		//	Pass our concatenated message ID string and and array to populate with providers.
		//	aryProviders is populated with the Providers parsed from the HL7 message
		CArray<CString, LPCSTR> aryProviders;
		CArray<CString, LPCSTR> aryProviderIDs;
		PopulateHL7LabProviders( strMessageIDs, aryProviders, aryProviderIDs );

		//	Our array is populated with providers now, let's run through the list
		//	and compare the filter with the providers in our array are we setup our vars.
		CString strFilterProvider = VarString(m_pProviderFilterCombo->GetComboBoxText(), _bstr_t(""));
		CString strFilterProviderID = _T("");
		CString strFilterThirdPartyProviderID = _T("");
		CArray<CString, LPCSTR> aryFilterThirdPartyIDs;

		// Assign current filter Provider ID
		long nProviderID = VarLong(m_pProviderFilterCombo->CurSel->GetValue(pfcProviderID), -1);
		strFilterProviderID.Format("%d", nProviderID);
		//	Add it to the list of the Third party IDs, incase of bi-directional link.
		aryFilterThirdPartyIDs.Add(strFilterProviderID);

		//	Get all the third party IDs reference in the table. (There may be more than 1 which is why we need an array)
		_RecordsetPtr prs = CreateParamRecordset("SELECT ThirdPartyID FROM HL7IDLinkT WHERE PersonID = {INT} AND RecordType = 2", nProviderID);

		while( !prs->eof ){
			//	Add the third party IDs we have linked already for this provider.
			strFilterThirdPartyProviderID = AdoFldString(prs, "ThirdPartyID", "");
			aryFilterThirdPartyIDs.Add(strFilterThirdPartyProviderID);
			//Next please
			prs->MoveNext();
		}
		//Cleanup
		prs->Close();

		//	Now that we have a list of possible third party HL7 links, let's see if they match any of the incoming messages.
		CString strMessageProviderID;
		CString strLinkedThirdPartyProviderID;
		CString strMessageProviderName;
		CString strTempFirst, strTempLast;
		long nIdx = 0, nUnmatchedOdd = 0;
		BOOL bRemove;
		pRow = m_pdlUnselectedList->GetFirstRow();

		//	Iterate through, either matching or removing rows from the list.
		while(pRow){
			strMessageProviderID = aryProviderIDs.GetAt(nIdx);
			bRemove = TRUE;

			//	Checked Practice ID for bidirectional link and any HL7 links previously defined in Practice.
			for( int naIdx = 0; naIdx < aryFilterThirdPartyIDs.GetSize(); naIdx++ ){
				strLinkedThirdPartyProviderID = aryFilterThirdPartyIDs.GetAt(naIdx);
				if( strMessageProviderID.CompareNoCase(strLinkedThirdPartyProviderID) == 0 ){
					bRemove = FALSE;
				}
			}
			
			//	Then, check the Ordering provider name.  If it doesn't match, we remove the row.
			strMessageProviderName = aryProviders.GetAt(nIdx);
			if( strMessageProviderName.CompareNoCase(strFilterProvider) == 0 ){
				bRemove = FALSE;
			} 

			//	If the providers don't match, add it to the list of rows to remove.
			if( bRemove){
				//	Keep count of HL7 Messages we could not determine the ordering provider for
				strTempLast = aryProviders.GetAt(nIdx).Left(aryProviders.GetAt(nIdx).Find(','));
				strTempFirst = aryProviders.GetAt(nIdx).Right(aryProviders.GetAt(nIdx).GetLength()-(aryProviders.GetAt(nIdx).Find(',')+2));
				if( aryProviderIDs.GetAt(nIdx).IsEmpty() && (strTempLast.IsEmpty() || strTempFirst.IsEmpty()) ){
					++nUnmatchedOdd;
					pRow->PutBackColor(RGB(242, 208, 208)); //Display odd results and mark them with a pink back color
				} else{
					paryRowsToRemove.Add(pRow);
				}
			}

			//	Next please.
			pRow = pRow->GetNextRow();	
			++nIdx;
		}

		//	Iterate through all the rows to be removed, and Remove them from the unselected labs list.
		for( int idx = 0; idx < paryRowsToRemove.GetSize(); idx++ ){
			m_pdlUnselectedList->RemoveRow(paryRowsToRemove.GetAt(idx));
		}

		//	Let the user know if Practice found any odd results.
		if( nUnmatchedOdd > 0 ){
			CString strMessage = _T("");
			strMessage.Format("Practice could not determine the Ordering Provider for (%li) labs.\n\nThese labs will be displayed in the list with a pink row color.  Please review these labs manually.", nUnmatchedOdd);
			MessageBox(strMessage, "Unable To Determine Ordering Provider", MB_OK | MB_ICONWARNING );
		}

		//Done.

	}NxCatchAll("CReceiveLabsDlg::FilterLabListsByProvider");
}//END CReceiveLabsDlg::FilterLabListsByProvider()

// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
void CReceiveLabsDlg::PopulateHL7LabProviders(CString strMessageIDs, CArray<CString, LPCSTR> &aryProviderNames, CArray<CString, LPCSTR> &aryProviderIDs)
{
	//	If we have no IDs, return
	if( strMessageIDs.IsEmpty() ){
		return;
	}

	//	Optimization, get ALL the messages from our comma delimited message string.
	try{
		//	Get the HL7 messages for all messages in the list
		_RecordsetPtr prs = CreateRecordset("SELECT GroupID, Message FROM HL7MessageQueueT WHERE ID IN (" + strMessageIDs + " )");

		//	Return if we have no records.
		if( prs->eof ){
			return;
		}

		//	Iterate through each of our records in the recordset and add the provider
		//	name to our array that GetHL7MessageComponent supplied to us.
		CString strMessage;
		CString strFirst, strLast, strName, strProviderID;
		int nHL7GroupID;
		while(!prs->eof) {
			//	Let's get a clean slate
			strName = strFirst = strLast = strProviderID = _T("");

			//	Assign iteration vars.
			strMessage = AdoFldString(prs, "Message", "");
			nHL7GroupID = AdoFldLong(prs, "GroupID", -1);

			//	Attempt to get the Ordering Provider from the OBR16 Segment
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 16, 1, 1, strProviderID);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 16, 1, 2, strLast);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 16, 1, 3, strFirst);
			
			//	If either the first or last name are empty, Try the ORC12 Segment.
			if( strProviderID.IsEmpty() && ( strLast.IsEmpty() || strFirst.IsEmpty() )){
				GetHL7MessageComponent(strMessage, nHL7GroupID, "ORC", 1, 12, 1, 1, strProviderID);	
				GetHL7MessageComponent(strMessage, nHL7GroupID, "ORC", 1, 12, 1, 2, strLast);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "ORC", 1, 12, 1, 3, strFirst);
			}

			//	Format our name and add it to the array.
			strLast.Replace(',', ' ');
			strFirst.Replace(',', ' ');
			strName = strLast + ", " + strFirst;
			strName.Trim();
			strProviderID.Trim();

			//	Save
			aryProviderNames.Add(strName);
			aryProviderIDs.Add(strProviderID);

			//	Next please.
			prs->MoveNext();
		} 
		//	Cleanup
		prs->Close();
	}NxCatchAll("CReceiveLabsDlg::PopulateHL7LabProviders - Unable to populate providers");
}//END CString CReceiveLabsDlg::GetHL7LabProvider(const long nMessageID)

// (r.gonet 03/01/2013) - PLID 48419 - Dismiss all the messages in the selected list.
void CReceiveLabsDlg::OnBnClickedDismissHl7LabResultsBtn()
{
	try {
		if(m_pdlSelectedList->GetRowCount() == 0) {
			AfxMessageBox("There are no messages in the selected list. Please select some messages before dismissing.");
			return;
		}

		// (r.gonet 03/01/2013) - PLID 48419 - Don't allow them to dismiss non-pending messages.
		IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
		bool bPrevImportedFound = false;
		while(pRow != NULL) {
			// (j.dinatale 2011-09-16 17:03) - PLID 40024 - check for previously imported messages
			long nActionID = VarLong(pRow->GetValue(slcActionID), -1);
			if(nActionID != -1) {
				bPrevImportedFound = true;
			}

			pRow = pRow->GetNextRow();
		}

		// (r.gonet 03/01/2013) - PLID 48419 - Error out if there are committed or dismissed messages in the list.
		if(bPrevImportedFound){
			AfxMessageBox("There are previously processed or dismissed lab messages in the selected list. Please unselect these messages.");
			return;
		}

		// (r.gonet 02/26/2013) - PLID 48419 - Give a warning.
		if(IDYES != MessageBox("Are you SURE you want to dismiss the selected lab result HL7 messages? If these lab results are for any live patients, make sure you have the client's written permission.", 
			"", MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1))
		{
			return;
		} else {
			// (r.gonet 02/26/2013) - PLID 48419 - Proceed with dismissal!
		}

		pRow = m_pdlSelectedList->GetFirstRow();
		while(pRow) {
			long nMessageID = VarLong(pRow->GetValue(slcID), -1);
			// (r.gonet 03/01/2013) - PLID 48419 - Get the next row now, because it will be removed by the Dismiss function.
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if(!DismissMessage(nMessageID, pRow, m_pdlSelectedList)) {
				MessageBox("Practice failed to dismiss one of the selected messages. Aborting dismissal.", "Dismissal Error", MB_OK|MB_ICONERROR);
			}
			pRow = pNextRow;
		}

		//refresh the screen to remove any committed rows
		UpdateView();

		AfxMessageBox("All messages were successfully dismissed.");
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Open the HL7 log dialog.
void CReceiveLabsDlg::OnBnClickedOpenHl7LogBtn()
{
	try {
		CHL7LogDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}
