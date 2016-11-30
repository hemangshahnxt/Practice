// NexSyncSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "NexSyncSettingsDlg.h"
#include "MultiSelectDlg.h"
#include "SingleSelectDlg.h"
#include "NexSyncEditSubjectLineDlg.h"
#include "NxOutlookUtils.h"
#include "RegUtils.h"
#include "SharedScheduleUtils.h"

using namespace NXDATALIST2Lib;

// (z.manning 2009-11-05 12:39) - PLID 36212 - Function to send a packet to NxServer to attempt
// to restart the NexSync service.
BOOL RestartNexSyncService(OUT CString &strFailureReason)
{
	NxSocketUtils::HCLIENT hNxServer;
	CString strNxServerIP = NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP");
	hNxServer = NxSocketUtils::Connect(NULL, strNxServerIP, STANDARD_CONNECTION_PORT);
	void* pResult;
	DWORD dwResultSize;
	PACKET_TYPE_TRY_RESTART_NEXSYNC_SERVICE_RESPONSE* response = NULL;
	BOOL bReturnValue = FALSE;
	if(NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_TRY_RESTART_NEXSYNC_SERVICE, PACKET_TYPE_TRY_RESTART_NEXSYNC_SERVICE,
		NULL, 0, pResult, dwResultSize))
	{
		response = (PACKET_TYPE_TRY_RESTART_NEXSYNC_SERVICE_RESPONSE*)pResult;
		strFailureReason = response->szFailureReason;
		bReturnValue = response->bSuccess;
	}
	else {
		strFailureReason = "Failed to get return packet";
		bReturnValue = FALSE;
	}
	
	NxSocketUtils::Disconnect(hNxServer);
	return bReturnValue;
}

// CNexSyncSettingsDlg dialog
// (z.manning 2009-08-26 14:11) - PLID 35345 - Created

#define IDM_TRANSFER_NEXPDA_PROFILE		WM_USER + 0x300A
#define IDM_NEW_NEXSYNC_PROFILE			WM_USER + 0x300B

IMPLEMENT_DYNAMIC(CNexSyncSettingsDlg, CNxDialog)

// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
CNexSyncSettingsDlg::CNexSyncSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexSyncSettingsDlg::IDD, pParent), m_dlgGoogleConnectionSettings(this)
{
	m_bChanged = FALSE;
	m_bNewProfile = FALSE;
}

CNexSyncSettingsDlg::~CNexSyncSettingsDlg()
{
}

void CNexSyncSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEXSYNC_BACKGROUND, m_nxcolorBackground);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_NEXSYNC_RESOURCES, m_btnEditResources);
	DDX_Control(pDX, IDC_EDIT_NEXSYNC_TYPES, m_btnEditApptTypes);
	DDX_Control(pDX, IDC_NEXSYNC_SUBJECT_LINE, m_nxeditSubjectLine);
	DDX_Control(pDX, IDC_NEXSYNC_PAST_DAYS, m_nxeditPastDays);
	DDX_Control(pDX, IDC_NEXSYNC_TYPE_EDIT, m_nxeditApptTypes);
	DDX_Control(pDX, IDC_NEXSYNC_RESOURCE_EDIT, m_nxeditResources);
	DDX_Control(pDX, IDC_NEXSYNC_INTERVAL, m_nxeditInterval);
	DDX_Control(pDX, IDC_EDIT_NEXSYNC_SUBJECT_LINE, m_btnEditSubjectLine);
	DDX_Control(pDX, IDC_NEXSYNC_IMPORT_CHAR, m_nxeditImportChar);
	DDX_Control(pDX, IDC_NEW_NEXSYNC_USER, m_btnNewUser);
	DDX_Control(pDX, IDC_DELETE_NEXSYNC_USER, m_btnDeleteUser);
	DDX_Control(pDX, IDC_NEXSYNC_CONNECTION_SETTINGS, m_btnConnectionSettings);
	DDX_Control(pDX, IDC_EDIT_NEXSYNC_NOTES_SECTION, m_btnEditNotesSection);
	DDX_Control(pDX, IDC_NEXSYNC_NOTES_SECTION, m_nxeditNotesSection);
}


BEGIN_MESSAGE_MAP(CNexSyncSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNexSyncSettingsDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_NEXSYNC_PAST_DAYS, &CNexSyncSettingsDlg::OnEnChangeNexsyncPastDays)
	ON_EN_CHANGE(IDC_NEXSYNC_IMPORT_CHAR, &CNexSyncSettingsDlg::OnEnChangeNexsyncImportChar)
	ON_BN_CLICKED(IDC_EDIT_NEXSYNC_TYPES, &CNexSyncSettingsDlg::OnBnClickedEditNexsyncTypes)
	ON_BN_CLICKED(IDC_EDIT_NEXSYNC_RESOURCES, &CNexSyncSettingsDlg::OnBnClickedEditNexsyncResources)
	ON_BN_CLICKED(IDC_NEXSYNC_IMPORT_CHAR_HELP, &CNexSyncSettingsDlg::OnBnClickedNexsyncImportCharHelp)
	ON_BN_CLICKED(IDC_NEW_NEXSYNC_USER, &CNexSyncSettingsDlg::OnBnClickedNewNexsyncUser)
	ON_BN_CLICKED(IDC_DELETE_NEXSYNC_USER, &CNexSyncSettingsDlg::OnBnClickedDeleteNexsyncUser)
	ON_COMMAND(IDM_TRANSFER_NEXPDA_PROFILE, OnTransferNexPdaProfile)
	ON_COMMAND(IDM_NEW_NEXSYNC_PROFILE, OnNewNexSyncProfile)
	ON_BN_CLICKED(IDC_NEXSYNC_CONNECTION_SETTINGS, &CNexSyncSettingsDlg::OnBnClickedNexsyncConnectionSettings)
	ON_BN_CLICKED(IDC_EDIT_NEXSYNC_SUBJECT_LINE, &CNexSyncSettingsDlg::OnBnClickedEditNexsyncSubjectLine)
	ON_BN_CLICKED(IDC_NEXSYNC_ALL_APPT_TYPES, &CNexSyncSettingsDlg::OnBnClickedNexsyncAllApptTypes)
	ON_BN_CLICKED(IDC_NEXSYNC_TWO_WAY, &CNexSyncSettingsDlg::OnBnClickedNexsyncTwoWay)
	ON_BN_CLICKED(IDC_NEXSYNC_ONE_WAY, &CNexSyncSettingsDlg::OnBnClickedNexsyncOneWay)
	ON_BN_CLICKED(IDC_NEXSYNC_CREATION_ONLY, &CNexSyncSettingsDlg::OnBnClickedNexsyncCreationOnly)
	ON_BN_CLICKED(IDC_EDIT_NEXSYNC_NOTES_SECTION, &CNexSyncSettingsDlg::OnBnClickedEditNexsyncNotesSection)
	ON_BN_CLICKED(IDC_NEXSYNC_CLEAR_ALL, &CNexSyncSettingsDlg::OnBnClickedNexsyncClearAll)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNexSyncSettingsDlg, CNxDialog)
ON_EVENT(CNexSyncSettingsDlg, IDC_NEXSYNC_PROFILE_LIST, 16, CNexSyncSettingsDlg::SelChosenNexsyncProfileList, VTS_DISPATCH)
ON_EVENT(CNexSyncSettingsDlg, IDC_NEXSYNC_PROFILE_LIST, 18, CNexSyncSettingsDlg::RequeryFinishedNexsyncProfileList, VTS_I2)
ON_EVENT(CNexSyncSettingsDlg, IDC_NEXSYNC_LOCATION_COMBO, 16, CNexSyncSettingsDlg::SelChosenNexsyncLocationCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

// CNexSyncSettingsDlg message handlers

BOOL CNexSyncSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();
		
		// (d.singleton 2013-02-28 16:08) - PLID 55932
		g_propManager.CachePropertiesInBulk("CNexSyncSettingsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"	'NexSyncInterval', 'NexSyncUseGoogleAPI'"
			")",
			_Q(GetCurrentUserName()));

		m_pdlLocations = BindNxDataList2Ctrl(IDC_NEXSYNC_LOCATION_COMBO, true);
		m_pdlUsers = BindNxDataList2Ctrl(IDC_NEXSYNC_PROFILE_LIST, true);

		// (z.manning 2009-08-26 14:56) - This isn't really part of any module, so let's just use
		// patients module color.
		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnNewUser.AutoSet(NXB_NEW);
		m_btnDeleteUser.AutoSet(NXB_DELETE);
		m_btnEditApptTypes.AutoSet(NXB_MODIFY);
		m_btnEditResources.AutoSet(NXB_MODIFY);
		m_btnEditSubjectLine.AutoSet(NXB_MODIFY);

		// (b.savon 2012-05-07 17:37) - PLID 37288 - Set the button type.  Set text limit is handled by the edit dialog, and since the edit is
		// uneditable, we do not need to handle it here as well.
		m_btnEditNotesSection.AutoSet(NXB_MODIFY);

		// (d.singleton 2013-02-28 16:08) - PLID 55932 need to know if they are using the google API or CalDav
		m_bUseGoogleAPI = GetRemotePropertyInt("NexSyncUseGoogleAPI", 0, 0, "<None>");

		// (z.manning 2013-09-25 11:08) - PLID 58764 - Indicate which NexSync we're using on the connection settings button
		if(m_bUseGoogleAPI) {
			m_btnConnectionSettings.SetWindowText("Connection Settings (Google API)");
		}
		else {
			m_btnConnectionSettings.SetWindowText("Connection Settings (CalDAV)");
		}

		// (z.manning 2009-10-01 09:24) - Keep the sync interval value reasonable
		m_nxeditInterval.SetLimitText(6);
		m_nxeditImportChar.SetLimitText(1);
		// (d.singleton 2013-02-28 16:08) - PLID 55668 make changes if they use google API
		// (d.singleton 2013-08-16 11:39) - PLID 57745 - need caldav to follow the same behavior		
		m_nxeditPastDays.SetLimitText(2);

		int nSyncInterval = GetRemotePropertyInt("NexSyncInterval", 30, 0, "<None>");
		SetDlgItemInt(IDC_NEXSYNC_INTERVAL, nSyncInterval);

		SetDlgItemText(IDC_NEXSYNC_TYPE_EDIT_ALL, "< All >");		

		// (z.manning 2014-05-30 15:20) - PLID 58352 - Show the clear all option for the nextech user
		if (GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			GetDlgItem(IDC_NEXSYNC_CLEAR_ALL)->ShowWindow(SW_SHOWNA);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNexSyncSettingsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	if(m_pdlUsers->GetCurSel() == NULL) {
		m_bNewProfile = FALSE;
		EnableControls(FALSE);
	}
	else {
		EnableControls(TRUE);
	}

	if (m_pdlUsers->GetRowCount() > 0) {
		SetDlgItemText(IDOK, "OK and Sync");
	}
	else {
		SetDlgItemText(IDOK, "OK");
	}

	if(m_bNewProfile) {
		LoadNewProfile();
	}
	else {
		LoadCurrentProfile();
	}

	UpdateConnectionsSettingsButton();
}

void CNexSyncSettingsDlg::SelChosenNexsyncProfileList(LPDISPATCH lpRow)
{
	try
	{
		if(m_bChanged) {
			int nResult = MessageBox("The current user's settings have changed. Would you like to save them?", "NexSync", MB_YESNO|MB_ICONQUESTION);
			if(nResult == IDYES) {
				if(!ValidateAndSaveCurrentProfile(m_prowLastSelectedUser)) {
					m_pdlUsers->PutCurSel(m_prowLastSelectedUser);
					return;
				}
			}
			else {
				m_pdlUsers->PutCurSel(m_prowLastSelectedUser);
				return;
			}
		}

		UpdateView();
		m_prowLastSelectedUser = lpRow;

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::LoadNewProfile()
{
	m_arynApptTypes.RemoveAll();
	m_arynResources.RemoveAll();
	m_nxeditApptTypes.SetWindowText("");
	m_nxeditResources.SetWindowText("");	
	// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg

	// (z.manning 2009-10-01 16:53) - Same as NxOutlookAddin
	m_nxeditSubjectLine.SetWindowText(DEFAULT_SYNC_SUBJECT_LINE);
	// (b.savon 2012-05-07 17:38) - PLID 37288 - Default to [Notes] 
	m_nxeditNotesSection.SetWindowText(DEFAULT_SYNC_NOTES_SECTION);
	// (d.singleton 2013-26-03 14:44) - 55377 reset the refresh token;
	m_dlgGoogleConnectionSettings.SetRefreshToken("");
	
	// (z.manning 2009-10-01 16:53) - Default to syncing just 14 days in the past because not only will
	// it make syncing more efficient, but most phone have a limit of how far in the past they'll sync as well.
	// (d.singleton 2013-26-03 14:44) - 55668 we now default to 7 instead of 14 if they use google api
	// (d.singleton 2013-08-16 11:39) - PLID 57745 - need caldav to follow the same behavior
	m_nxeditPastDays.SetWindowText("7");
		
	// (z.manning 2014-06-02 14:43) - PLID 58352 - Reset the clear all flag
	CheckDlgButton(IDC_NEXSYNC_CLEAR_ALL, BST_UNCHECKED);

	m_nxeditImportChar.SetWindowText("~");

	// (z.manning 2009-10-01 16:59) - Default import location to the most common import location
	ADODB::_RecordsetPtr prsLocID = CreateRecordset(
		"SELECT TOP 1 LocationID \r\n"
		"FROM AppointmentsT \r\n"
		"INNER JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID \r\n"
		"WHERE LocationsT.Active = 1 \r\n"
		"GROUP BY LocationID \r\n"
		"ORDER BY COUNT(*) DESC \r\n"
		);
	if(!prsLocID->eof) {
		m_pdlLocations->SetSelByColumn(lccID, AdoFldLong(prsLocID->GetFields(), "LocationID"));
	}

	// (z.manning 2011-04-04 11:12) - PLID 38193 - Default to two-way syncing
	// (z.manning 2014-01-28 12:13) - PLID 60016 - Default to only allow creation
	UpdateSyncTypeRadios(twsvCreateOnly);
}

void CNexSyncSettingsDlg::LoadCurrentProfile()
{
	m_arynApptTypes.RemoveAll();
	m_arynResources.RemoveAll();

	IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
	if(pUserRow == NULL)
	{
		m_nxeditPastDays.SetWindowText("");
		m_nxeditSubjectLine.SetWindowText("");
		m_nxeditApptTypes.SetWindowText("");
		m_nxeditResources.SetWindowText("");
		m_nxeditImportChar.SetWindowText("");		
		// (b.savon 2012-05-07 16:14) - PLID 37288
		m_nxeditNotesSection.SetWindowText("");
		// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
		// (d.singleton 2013-26-03 14:44) - 55377 reset the refresh token;
		m_dlgGoogleConnectionSettings.SetRefreshToken("");
	}
	else
	{
		long nUserID = VarLong(pUserRow->GetValue(uccID));

		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"DECLARE @nCategoryID int \r\n"
			// (z.manning 2009-11-03 15:27) - PLID 35801 - We could be transferring a profile from
			// the NexPDA link here, which means it is possible that this profile has more than one
			// calendar category. NexSync does not allow such things not only because just about no
			// one actually uses this, but it's also a way to pretty easily circumvent our licensing.
			// To handle that here we're going to arbitrarily select the first category for a user
			// rather than prompt them for something that likely makes no sense to them.
			"SET @nCategoryID = (SELECT TOP 1 ID FROM OutlookCategoryT WHERE UserID = {INT}) \r\n"
			"SET NOCOUNT OFF \r\n"
			"\r\n"
			// (z.manning 2009-10-08 16:16) - PLID 35574 - Also load the address, user, and pw for NexSync.
			// (z.manning 2010-02-08 13:44) - PLID 37142 - Added AccountType
			// (z.manning 2010-10-25 09:40) - PLID 36017 - Added SyncAllApptTypes
			// (z.manning 2011-04-04 11:13) - PLID 38193 - Added TwoWaySync
			// (b.savon 2012-05-07 15:04) - PLID 37288 - Added BodyScript
			// (d.singleton 2013-26-03 14:44) - 55377 - Added GoogleRefreshToken
			// (z.manning 2014-05-30 15:22) - PLID 58352 - Added clear all
			"SELECT SubjectScript, TimeSpan, LocationID, ImportCharacter, SyncAllApptTypes, ClearAll \r\n"
			"	, GoogleRefreshToken, NexSyncUsername, AccountType, TwoWaySync, OutlookCalendarT.BodyScript \r\n"
			"FROM OutlookCalendarT \r\n"
			"LEFT JOIN OutlookCategoryT ON OutlookCalendarT.CategoryID = OutlookCategoryT.ID \r\n"
			"LEFT JOIN OutlookFolderT ON OutlookCategoryT.UserID = OutlookFolderT.UserID AND Type = 9 \r\n"
			"LEFT JOIN OutlookProfileT ON OutlookFolderT.UserID = OutlookProfileT.UserID \r\n"
			"WHERE OutlookCategoryT.ID = @nCategoryID \r\n"
			"\r\n"
			"SELECT AptTypeID, Name \r\n"
			"FROM OutlookAptTypeT \r\n"
			"LEFT JOIN AptTypeT ON OutlookAptTypeT.AptTypeID = AptTypeT.ID \r\n"
			"WHERE CategoryID = @nCategoryID \r\n"
			"ORDER BY Name \r\n"
			"\r\n"
			"SELECT AptResourceID, Item \r\n"
			"FROM OutlookAptResourceT \r\n"
			"INNER JOIN ResourceT ON OutlookAptResourceT.AptResourceID = ResourceT.ID \r\n"
			"WHERE CategoryID = @nCategoryID \r\n"
			"ORDER BY Item \r\n"
			, nUserID);

		if(prs->eof) {
			ThrowNxException("CNexSyncSettingsDlg::LoadCurrentProfile - Failed to load info for user ID %li", nUserID);
		}

		// (b.savon 2012-05-07 15:05) - PLID 37288 - Added BodyScript
		m_nxeditNotesSection.SetWindowText(AdoFldString(prs->GetFields(), "BodyScript", ""));
		m_nxeditSubjectLine.SetWindowText(AdoFldString(prs->GetFields(), "SubjectScript", ""));
		m_nxeditPastDays.SetWindowText(AsString(AdoFldLong(prs->GetFields(), "TimeSpan")));
		char chImportChar = (char)AdoFldByte(prs->GetFields(), "ImportCharacter");
		m_nxeditImportChar.SetWindowText(CString(chImportChar));
		// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
		// (d.singleton 2013-26-03 14:44) - 55377 set the refresh token;
		m_dlgGoogleConnectionSettings.SetRefreshToken(DecryptStringFromVariant(prs->GetFields()->GetItem("GoogleRefreshToken")->GetValue()));
		// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
		// (d.singleton 2013-07-08 17:23) - PLID 57068 - new endpoint url needs calendarID ( username ) moved to OAuth2.0 dlg,  stored in same db field
		m_dlgGoogleConnectionSettings.SetNexSyncUserName(AdoFldString(prs->GetFields(), "NexSyncUserName", ""));
		// (z.manning 2010-10-25 09:41) - PLID 36017 - SyncAllApptTypes
		BOOL bSyncAllTypes = AdoFldBool(prs->GetFields(), "SyncAllApptTypes", FALSE);
		CheckDlgButton(IDC_NEXSYNC_ALL_APPT_TYPES, bSyncAllTypes ? BST_CHECKED : BST_UNCHECKED);
		// (z.manning 2011-04-04 11:14) - PLID 38193 - We now adhere to the two-way sync field
		// (z.manning 2014-01-28 12:14) - PLID 60016 - This is now a tinyint field rather than a bit
		TwoWaySyncValue eSyncType = (TwoWaySyncValue)AdoFldByte(prs->GetFields(), "TwoWaySync", twsvOneWay);
		UpdateSyncTypeRadios(eSyncType);
		// (z.manning 2014-05-30 15:24) - PLID 58352 - Clear all check
		CheckDlgButton(IDC_NEXSYNC_CLEAR_ALL, AdoFldBool(prs, "ClearAll") ? BST_CHECKED : BST_UNCHECKED);

		m_pdlLocations->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pdlLocations->SetSelByColumn(lccID, AdoFldLong(prs->GetFields(), "LocationID"));

		prs = prs->NextRecordset(NULL);

		CString strTypeString;
		for(; !prs->eof; prs->MoveNext()) {
			CString strType = AdoFldString(prs->GetFields(), "Name", "");
			long nAptTypeID = AdoFldLong(prs->GetFields(), "AptTypeID");
			m_arynApptTypes.Add(nAptTypeID);
			if(nAptTypeID == -1) {
				strType = "{ No Type }";
			}
			strTypeString += strType + ", ";
		}
		strTypeString.Delete(strTypeString.GetLength() - 2, 2);
		m_nxeditApptTypes.SetWindowText(strTypeString);

		prs = prs->NextRecordset(NULL);

		CString strResourceString;
		for(; !prs->eof; prs->MoveNext()) {
			CString strResource = AdoFldString(prs->GetFields(), "Item", "");
			m_arynResources.Add(AdoFldLong(prs->GetFields(), "AptResourceID"));
			strResourceString += strResource + "; ";
		}
		strResourceString.Delete(strResourceString.GetLength() - 2, 2);
		m_nxeditResources.SetWindowText(strResourceString);

		EnableControls(TRUE);
	}

	m_bChanged = FALSE;
}

void CNexSyncSettingsDlg::RequeryFinishedNexsyncProfileList(short nFlags)
{
	try
	{
		// (z.manning 2009-08-26 16:03) - We just finished the intial user list load,
		// so let's try and select one.
		if(m_pdlUsers->GetRowCount() > 0) {
			if(m_pdlUsers->SetSelByColumn(uccID, GetCurrentUserID()) == NULL) {
				// (z.manning 2009-08-26 16:04) - Current user doesn't have a profile, so
				// just arbitrarily select the first row.
				m_pdlUsers->PutCurSel(m_pdlUsers->GetFirstRow());
			}
		}
		m_prowLastSelectedUser = m_pdlUsers->GetCurSel();

		UpdateView();

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::EnableControls(BOOL bEnable)
{
	m_btnEditResources.EnableWindow(bEnable);
	m_btnEditSubjectLine.EnableWindow(bEnable);
	m_nxeditPastDays.EnableWindow(bEnable);
	m_nxeditImportChar.EnableWindow(bEnable);
	m_pdlLocations->PutReadOnly(bEnable ? VARIANT_FALSE : VARIANT_TRUE);
	m_btnConnectionSettings.EnableWindow(bEnable);
	GetDlgItem(IDC_NEXSYNC_ALL_APPT_TYPES)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEXSYNC_TWO_WAY)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEXSYNC_ONE_WAY)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEXSYNC_CREATION_ONLY)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEXSYNC_CLEAR_ALL)->EnableWindow(bEnable);
	// (b.savon 2012-05-07 15:06) - PLID 37288
	m_btnEditNotesSection.EnableWindow(bEnable);

	// (z.manning 2010-10-25 09:59) - PLID 36017 - Handle the all types option
	if(IsDlgButtonChecked(IDC_NEXSYNC_ALL_APPT_TYPES) == BST_CHECKED) {
		m_btnEditApptTypes.EnableWindow(FALSE);
		GetDlgItem(IDC_NEXSYNC_TYPE_EDIT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXSYNC_TYPE_EDIT_ALL)->ShowWindow(SW_SHOW);
	}
	else {
		m_btnEditApptTypes.EnableWindow(bEnable);
		GetDlgItem(IDC_NEXSYNC_TYPE_EDIT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXSYNC_TYPE_EDIT_ALL)->ShowWindow(SW_HIDE);
	}
}

void CNexSyncSettingsDlg::OnBnClickedOk()
{
	try
	{
		int nSyncInterval = GetDlgItemInt(IDC_NEXSYNC_INTERVAL);
		if(nSyncInterval <= 0) {
			MessageBox("Please enter a value greater than zero for the sync interval.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		//(d.singleton 2013-26-03 15:31) - 55668 if this is using google api we only allow up to 30 days in past
		// (d.singleton 2013-08-16 11:39) - PLID 57745 - need caldav to follow the same behavior
		int nTimeSpan = GetDlgItemInt(IDC_NEXSYNC_PAST_DAYS);
		if(nTimeSpan > 30) {
			MessageBox("Please enter a smaller value for the number of days to sync in the past.  30 is the maximum value", NULL, MB_OK|MB_ICONERROR);
			return;
		}
		
		if(m_pdlUsers->GetRowCount() == 0) {
			// (z.manning 2009-11-05 14:28) - If we don't have any users then make sure this property
			// doesn't exist as that will let the NexSync service know to not do anything.
			ExecuteParamSql("DELETE FROM ConfigRT WHERE Name = {STRING}", "NexSyncInterval");
		}
		else {
			SetRemotePropertyInt("NexSyncInterval", nSyncInterval, 0, "<None>");
		}

		// If we have at least one registered user make sure to save any changes that may have been made.
		if(ValidateAndSaveCurrentProfile(m_pdlUsers->GetCurSel()))
		{
			// If we have at least one user that was successfully validated and saved, let's send a sync request
			// for ALL validated users for all database. The LOE for a specific user and specific subkey was too high
			// and the solution prior to this was already doing the same since 2009 with no complaints from clients. 
			if(m_pdlUsers->GetRowCount() > 0)
			{				
				GetAPI()->SyncGoogleAPI(GetAPISubkey(), GetAPILoginToken());
			}

			CNxDialog::OnOK();
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CNexSyncSettingsDlg::ValidateAndSaveCurrentProfile(LPDISPATCH lpUserRow)
{
	try
	{
		// (z.manning 2009-10-28 11:39) - PLID 36066 - Safety check
		if(m_bNewProfile) {
			if(!HandleLicenseCheck()) {
				return FALSE;
			}
		}

		IRowSettingsPtr pUserRow(lpUserRow);
		if(pUserRow == NULL) {
			return TRUE;
		}
		long nUserID = VarLong(pUserRow->GetValue(uccID));

		BOOL bSyncAllTypes = (IsDlgButtonChecked(IDC_NEXSYNC_ALL_APPT_TYPES) == BST_CHECKED);

		IRowSettingsPtr pLocationRow = m_pdlLocations->GetCurSel();
		if(pLocationRow == NULL) {
			MessageBox("You must select location for imported appointments.", "NexSync", MB_OK|MB_ICONERROR);
			return FALSE;
		}

		if(m_arynApptTypes.GetSize() == 0 && !bSyncAllTypes) {
			MessageBox("You must select at least one appointment type to sync.", "NexSync", MB_OK|MB_ICONERROR);
			return FALSE;
		}
		if(m_arynResources.GetSize() == 0) {
			MessageBox("You must select at least one resource to sync.", "NexSync", MB_OK|MB_ICONERROR);
			return FALSE;
		}

		// (z.manning 2011-04-04 11:22) - PLID 38193 - Two-way sync option
		TwoWaySyncValue eTwoWaySync;
		if(IsDlgButtonChecked(IDC_NEXSYNC_TWO_WAY) == BST_CHECKED) {
			eTwoWaySync = twsvTwoWay;
		}
		else if(IsDlgButtonChecked(IDC_NEXSYNC_ONE_WAY) == BST_CHECKED) {
			eTwoWaySync = twsvOneWay;
		}
		else if(IsDlgButtonChecked(IDC_NEXSYNC_CREATION_ONLY) == BST_CHECKED) {
			eTwoWaySync = twsvCreateOnly;
		}
		else {
			MessageBox("Please choose which type of syncing.", "NexSync", MB_OK|MB_ICONERROR);
			return FALSE;
		}

		// (z.manning 2009-10-08 16:48) - PLID 35574
		if(!AreConnectionSettingsValid()) {
			if(IDYES != MessageBox("The connection settings are not valid. Are you sure you want to save?", "NexSync", MB_YESNO|MB_ICONQUESTION)) {
				return FALSE;
			}
		}

		if(!m_bChanged) {
			return TRUE;
		}

		// (b.savon 2012-05-07 17:39) - PLID 37288 - Added strNotesSection
		CString strSubject, strImportChar, strNotesSection;
		m_nxeditNotesSection.GetWindowText(strNotesSection);
		m_nxeditSubjectLine.GetWindowText(strSubject);
		m_nxeditImportChar.GetWindowText(strImportChar);
		BYTE nImportChar = 0;
		if(strImportChar.GetLength() > 0) {
			nImportChar = (BYTE)strImportChar.GetAt(0);
		}
		long nLocationID = VarLong(pLocationRow->GetValue(lccID));
		UINT nPastDays = GetDlgItemInt(IDC_NEXSYNC_PAST_DAYS);		

		// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connectiono settings dlg
		// (d.singleton 2013-07-08 17:29) - PLID 57068 - new endpoint url, moved username to OAuth2.0 dlg
		CString strNexSyncUserName = m_dlgGoogleConnectionSettings.GetNexSyncUserName();
		// (d.singleton 2013-02-28 15:36) - PLID 55377 need to encrypt the google refreshtoken token
		_variant_t varEncryptedGoogleRefreshToken = EncryptStringToVariant(m_dlgGoogleConnectionSettings.GetRefreshToken());

		CParamSqlBatch sqlBatch;

		sqlBatch.Declare("DECLARE @nUserID int");
		sqlBatch.Declare("DECLARE @nCategoryID int");
		sqlBatch.Add("SET @nUserID = {INT}", nUserID);

		if(m_bNewProfile)
		{
			// (z.manning 2009-10-21 12:33) - PLID 36009 - The NxOutlookAddin project doesn't handle some
			// data very well so before we create some of the new structure, ensure this user doesn't
			// have any previously existing calendar/category data.
			sqlBatch.Add("DELETE FROM OutlookAptResourceT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID)");
			sqlBatch.Add("DELETE FROM OutlookAptTypeT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID)");
			sqlBatch.Add("DELETE FROM OutlookCalendarT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID)");
			sqlBatch.Add("DELETE FROM OutlookCategoryT WHERE UserID = @nUserID");
			// (z.manning 2009-10-29 15:13) - PLID 35428 - Delete from NexSyncEventsT
			sqlBatch.Add("DELETE FROM NexSyncEventsT WHERE OutlookFolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = @nUserID AND Type = 9)");
			sqlBatch.Add("DELETE FROM OutlookUpdateT WHERE FolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = @nUserID AND Type = 9)");
			sqlBatch.Add("DELETE FROM OutlookAirlockT WHERE FolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = @nUserID AND Type = 9)");
			sqlBatch.Add("DELETE FROM OutlookFolderT WHERE UserID = @nUserID AND Type = 9");
			// (z.manning 2009-10-01 17:16) - HandheldType is 100% pointless and SyncFlags is not used by 
			// NexSync, but since we're sharing data structure with the NexPDA link we need to populate
			// those fields since they're non-nullable.
			// (z.manning 2009-10-20 12:54) - PLID 36009 - Use zero for sync flags because it's only used by NexPDA
			// link and we need to make sure it knows to not sync the calendar anymore.
			sqlBatch.Add(
				"IF NOT EXISTS (SELECT UserID FROM OutlookProfileT WHERE UserID = @nUserID) BEGIN \r\n"
				"	INSERT INTO OutlookProfileT (UserID, HandheldType, SyncFlags, TwoWaySync) VALUES (@nUserID, 0, 0, 1) \r\n"
				"END");
			// (z.manning 2009-10-01 17:20) - Type 9 = calendar, which is all NexSync currently supports
			// (z.manning 2009-10-08 16:53) - PLID 35574 - Added server address, user, and password
			// (z.manning 2010-02-08 15:07) - PLID 37142 - Added AccountType
			// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
			// (z.manning 2014-05-30 16:46) - PLID 62287 - Need to save the refresh token here
			sqlBatch.Add("INSERT INTO OutlookFolderT (Type, UserID, ForcePracticeExport, IsNexSync, NexSyncUserName, GoogleRefreshToken) \r\n"
				"VALUES (9, @nUserID, 1, 1, {STRING}, {VARBINARY})"
				, strNexSyncUserName,  varEncryptedGoogleRefreshToken);
			sqlBatch.Add("INSERT INTO OutlookCategoryT (UserID) VALUES (@nUserID) \r\n"
				"SET @nCategoryID = SCOPE_IDENTITY()");
			// (z.manning 2010-10-25 09:44) - PLID 36017 - Added SyncAllApptTypes
			// (b.savon 2012-05-07 17:40) - PLID 37288 - Added strNotesSection
			sqlBatch.Add("INSERT INTO OutlookCalendarT (CategoryID, SubjectScript, BodyScript, TimeSpan, LocationID, ImportCharacter, SyncAllApptTypes) \r\n"
				"VALUES (@nCategoryID, {STRING}, {STRING}, {INT}, {INT}, {INT}, {BIT})"				
				, strSubject, strNotesSection, nPastDays, nLocationID, nImportChar, bSyncAllTypes);
		}
		else
		{
			sqlBatch.Add("SET @nCategoryID = (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID)");
			// (z.manning 2009-10-01 12:47) - Save the basic data
			// (z.manning 2010-10-25 09:48) - PLID 36017 - Added SyncAllApptTypes
			// (b.savon 2012-05-07 17:40) - PLID 37288 - Added BodyType. strNotesSection
			sqlBatch.Add(
				"UPDATE OutlookCalendarT SET \r\n"
				"	SubjectScript = {STRING} \r\n"
				"	, BodyScript = {STRING} \r\n"
				"	, TimeSpan = {INT} \r\n"
				"	, LocationID = {INT} \r\n"
				"	, ImportCharacter = {INT} \r\n"
				"	, SyncAllApptTypes = {BIT} \r\n"
				"WHERE CategoryID = @nCategoryID"
				, strSubject, strNotesSection, nPastDays, nLocationID, nImportChar, bSyncAllTypes);

			sqlBatch.Add("UPDATE OutlookCategoryT SET Name = NULL WHERE ID = @nCategoryID");
			
			// (z.manning 2009-10-08 16:53) - PLID 35574 - Added server address, user, and password
			// (z.manning 2010-02-08 15:09) - PLID 37142 - Added account type
			// (d.singleton 2013-02-28 15:36) - PLID 55377 added google refresh token
			// (d.singleton 2013-07-03 17:00) - PLID 57446 - deleted references to the old CalDav connection settings dlg
			// (z.manning 2014-05-30 15:28) - PLID 58352 - Added clear all
			sqlBatch.Add(
				"UPDATE OutlookFolderT SET \r\n"
				"GoogleRefreshToken = {VARBINARY}, NexSyncUserName = {STRING} \r\n"
				"WHERE UserID = @nUserID AND Type = 9"
				, varEncryptedGoogleRefreshToken, strNexSyncUserName);
		}

		// (z.manning 2009-10-01 12:47) - Save appt types
		if(!m_bNewProfile) {
			sqlBatch.Add("DELETE FROM OutlookAptTypeT WHERE CategoryID = @nCategoryID");
		}
		for(int nTypeIndex = 0; nTypeIndex < m_arynApptTypes.GetSize(); nTypeIndex++) {
			int nTypeID = m_arynApptTypes.GetAt(nTypeIndex);
			sqlBatch.Add(
				"INSERT INTO OutlookAptTypeT (CategoryID, AptTypeID) VALUES (@nCategoryID, {INT})"
				, nTypeID);
		}

		// (z.manning 2009-10-01 12:47) - Save resources
		if(!m_bNewProfile) {
			sqlBatch.Add("DELETE FROM OutlookAptResourceT WHERE CategoryID = @nCategoryID");
		}
		for(int nResourceIndex = 0; nResourceIndex < m_arynResources.GetSize(); nResourceIndex++) {
			int nResourceID = m_arynResources.GetAt(nResourceIndex);
			sqlBatch.Add(
				"INSERT INTO OutlookAptResourceT (CategoryID, AptResourceID) VALUES (@nCategoryID, {INT})"
				, nResourceID);
		}

		if(!m_bNewProfile) {
			// (z.manning 2009-10-06 15:06) - PLID 35801 - This may be a profile transferred from the NexPDA
			// link so set the IsNexSync flag to 1 here to ensure it's now NexSync.
			sqlBatch.Add("UPDATE OutlookFolderT SET ForcePracticeExport = 1, IsNexSync = 1 WHERE UserID = @nUserID AND Type = 9");
		}

		// (z.manning 2014-06-02 14:37) - PLID 58352 - Save the clear all option if logged in as the Nextech support user
		if (GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			BOOL bClearAll = (IsDlgButtonChecked(IDC_NEXSYNC_CLEAR_ALL) == BST_CHECKED);
			sqlBatch.Add("UPDATE OutlookFolderT SET ClearAll = {BIT} WHERE UserID = @nUserID AND Type = 9", bClearAll);
		}

		// (z.manning 2009-10-20 11:50) - PLID 36009 - Also make sure we turn off calendar from the 
		// SyncFlags field so the NexPDA link knows to not sync calendar any longer.
		// (z.manning 2011-04-04 11:25) - PLID 38193 - Handle the two-way sync option
		sqlBatch.Add(
			"UPDATE OutlookProfileT SET SyncFlags = (SyncFlags & ~{INT}), TwoWaySync = {INT} WHERE UserID = @nUserID"
			, SYNC_FLAG_CALENDAR, eTwoWaySync);

		sqlBatch.Execute(GetRemoteData());

		m_bNewProfile = FALSE;
		m_bChanged = FALSE;
		return TRUE;
	
	}NxCatchAll(__FUNCTION__);
	return FALSE;
}
void CNexSyncSettingsDlg::OnEnChangeNexsyncPastDays()
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::SelChosenNexsyncLocationCombo(LPDISPATCH lpRow)
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnEnChangeNexsyncImportChar()
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedEditNexsyncTypes()
{
	try
	{
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptTypeT");
		dlg.PreSelect(m_arynApptTypes);

		IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
		if(pUserRow == NULL) {
			return;
		}
		long nUserID = VarLong(pUserRow->GetValue(uccID));

		// (z.manning 2009-10-01 11:45) - Select all active types AND all types associated with the current
		// user as that may include inactive types.
		CString strFrom = FormatString(
			"( \r\n"
			"	SELECT -1 AS ID, ' { No Type }' AS Name \r\n"
			"	UNION \r\n"
			"	SELECT ID, Name FROM AptTypeT WHERE Inactive = 0 \r\n"
			"	UNION \r\n"
			"	SELECT AptTypeT.ID, AptTypeT.Name \r\n"
			"	FROM OutlookAptTypeT \r\n"
			"	INNER JOIN AptTypeT ON OutlookAptTypeT.AptTypeID = AptTypeT.ID \r\n"
			"	INNER JOIN OutlookCategoryT ON OutlookAptTypeT.CategoryID = OutlookCategoryT.ID \r\n"
			"	WHERE OutlookCategoryT.UserID = %li \r\n"
			") AptTypeQ \r\n"
			, nUserID);
		int nResult = dlg.Open(strFrom, "", "ID", "Name", "Select the appointment types you would like to sync", 1);
		if(nResult == IDOK)
		{
			m_bChanged = TRUE;
			m_arynApptTypes.RemoveAll();
			dlg.FillArrayWithIDs(m_arynApptTypes);
			m_nxeditApptTypes.SetWindowText(dlg.GetMultiSelectString(", "));
		}

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedEditNexsyncResources()
{
	try
	{
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ResourceT");
		dlg.PreSelect(m_arynResources);

		IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
		if(pUserRow == NULL) {
			return;
		}
		long nUserID = VarLong(pUserRow->GetValue(uccID));

		// (z.manning 2009-10-01 11:45) - Select all active resources AND all resources associated with the current
		// user as that may include inactive resources.
		CString strFrom = FormatString(
			"( \r\n"
			"	SELECT ID, Item FROM ResourceT WHERE Inactive = 0 \r\n"
			"	UNION \r\n"
			"	SELECT ResourceT.ID, ResourceT.Item \r\n"
			"	FROM OutlookAptResourceT \r\n"
			"	INNER JOIN ResourceT ON OutlookAptResourceT.AptResourceID = ResourceT.ID \r\n"
			"	INNER JOIN OutlookCategoryT ON OutlookAptResourceT.CategoryID = OutlookCategoryT.ID \r\n"
			"	WHERE OutlookCategoryT.UserID = %li \r\n"
			") AptResourceQ \r\n"
			, nUserID);
		int nResult = dlg.Open(strFrom, "", "ID", "Item", "Select the resource you would like to sync", 1);
		if(nResult == IDOK)
		{
			m_bChanged = TRUE;
			m_arynResources.RemoveAll();
			dlg.FillArrayWithIDs(m_arynResources);
			m_nxeditResources.SetWindowText(dlg.GetMultiSelectString("; "));
		}

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedNexsyncImportCharHelp()
{
	try
	{
		// (z.manning 2009-10-01 12:46) - Show a message box explaining what the import character field
		// is for.
		MessageBox(
			"If you want to enter an appointment on your mobile device and have it come into NexTech, "
			"this character is how you do that. Simply enter this character as the last character is the "
			"subject/title/summary line of the appointment when you create it in your mobile device.\r\n\r\n"
			"For example, if your import character is '~' and you enter an appointment with the following "
			"subject line...\r\n\r\n"
			"\tLunch~\r\n\r\n"
			"...then that appointment will be created in NexTech when the next sync occurs."
			, "NexSync", MB_ICONINFORMATION);

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedNewNexsyncUser()
{
	try
	{
		if(m_bChanged) {
			int nResult = MessageBox("The current user's settings have changed. Would you like to save them?", "NexSync", MB_YESNO|MB_ICONQUESTION);
			if(nResult == IDYES) {
				if(!ValidateAndSaveCurrentProfile(m_pdlUsers->GetCurSel())) {
					return;
				}
			}
			else {
				return;
			}
		}

		// (z.manning 2009-10-28 10:13) - PLID 36066 - Make sure they can't create more profiles than they
		// are licensed for.
		if(!HandleLicenseCheck()) {
			return;
		}

		// (z.manning 2009-10-06 12:23) - PLID 35801 - Check and see if there are any existing NexPDA
		// calendar profiles.
		ADODB::_RecordsetPtr prs = CreateRecordset(
			"SELECT TOP 1 UserID FROM OutlookProfileT \r\n"
			"WHERE UserID IN (SELECT UserID FROM OutlookFolderT WHERE IsNexSync = 0) \r\n"
			"	AND UserID NOT IN (SELECT UserID FROM OutlookFolderT WHERE IsNexSync = 1) \r\n"
			);
		if(prs->eof) {
			// (z.manning 2009-10-06 12:33) - PLID 35801 - There aren't any NexPDA profiles so simply
			// create a new profile from scratch.
			CreateNewNexSyncUser();
		}
		else {
			// (z.manning 2009-10-06 12:34) - PLID 35801 - They do have NexPDA profiles, so let's give
			// a menu with options to either transfer one of those or make a new one from scratch.
			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;
			mnu.AppendMenu(MF_ENABLED, IDM_NEW_NEXSYNC_PROFILE, "New Nex&Sync Profile...");
			mnu.AppendMenu(MF_ENABLED, IDM_TRANSFER_NEXPDA_PROFILE, "Transfer Nex&PDA Profile...");
			
			CRect rc;
			this->m_btnNewUser.GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		}

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::CreateNewNexSyncUser()
{
	CSingleSelectDlg dlg(this);
	int nResult = dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID"
		, "Archived = 0 AND PersonT.ID NOT IN (SELECT UserID FROM OutlookProfileT) AND PersonT.ID > 0"
		, "PersonT.ID", "PersonT.Last + ', ' + PersonT.First"
		, "Please select a user for the new NexSync profile"
		, true);
	if(nResult == IDOK)
	{
		IRowSettingsPtr pNewUserRow = m_pdlUsers->GetNewRow();
		pNewUserRow->PutValue(uccID, dlg.GetSelectedID());
		pNewUserRow->PutValue(uccName, dlg.GetSelectedDisplayValue());
		m_pdlUsers->PutCurSel(m_pdlUsers->AddRowSorted(pNewUserRow, NULL));
		m_bNewProfile = TRUE;
		// (z.manning 2010-10-25 10:01) - PLID 36017 - Default the all types option to on
		CheckDlgButton(IDC_NEXSYNC_ALL_APPT_TYPES, BST_CHECKED);
		// (z.manning 2011-04-04 11:17) - PLID 38193 - Default to two-way sincing
		// (z.manning 2014-01-28 12:15) - PLID 60016 - Default to create only now
		UpdateSyncTypeRadios(twsvCreateOnly);
		UpdateView();
		m_prowLastSelectedUser = m_pdlUsers->GetCurSel();
	}
}

void CNexSyncSettingsDlg::OnBnClickedDeleteNexsyncUser()
{
	try
	{
		IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
		if(pUserRow == NULL) {
			return;
		}
		long nUserID = VarLong(pUserRow->GetValue(uccID));

		int nPrompt = MessageBox("Are you sure you want to delete this NexSync user profile? This cannot be undone.", "NexSync", MB_YESNO|MB_ICONQUESTION);
		if(nPrompt != IDYES) {
			return;
		}

		if(!m_bNewProfile)
		{
			// (z.manning 2009-11-04 10:01) - PLID 36009 - It is possible to have the same sync profile
			// be half NexSync (for calendar) and half NexPDA (for contacts). Originally I was just deleting
			// the NexSync portion here. However, that can lead to possible licensing violations, so we
			// are now simply deleting the entire profile here and if this user still wants to sync contacs
			// they can recreate a NexPDA only profile to do so if they have an available license.
			ExecuteParamSql(
				"SET XACT_ABORT ON \r\n"
				"BEGIN TRAN \r\n"
				"DECLARE @nUserID int \r\n"
				"SET @nUserID = {INT} \r\n"
				"DECLARE @FolderIDs TABLE (ID int not null) \r\n"
				"INSERT INTO @FolderIDs (ID) SELECT ID FROM OutlookFolderT WHERE UserID = @nUserID \r\n"// "AND IsNexSync = 1 "
				"\r\n"
				"DELETE FROM OutlookUpdateT WHERE FolderID IN (SELECT ID FROM @FolderIDs) \r\n"
				"DELETE FROM OutlookAirlockT WHERE FolderID IN (SELECT ID FROM @FolderIDs) \r\n"
				// (z.manning 2009-10-29 15:11) - PLID 35428 - Delete from NexSyncEventsT
				"DELETE FROM NexSyncEventsT WHERE OutlookFolderID IN (SELECT ID FROM @FolderIDs) \r\n"
				"DELETE FROM OutlookFolderT WHERE ID IN (SELECT ID FROM @FolderIDs) \r\n"
				"\r\n"
				//"IF NOT EXISTS (SELECT UserID FROM OutlookFolderT WHERE UserID = @nUserID) BEGIN \r\n"
				"	DELETE FROM OutlookAddressBookT WHERE UserID = @nUserID \r\n"
				"	DELETE FROM OutlookAptResourceT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID) \r\n"
				"	DELETE FROM OutlookAptTypeT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID) \r\n"
				"	DELETE FROM OutlookCalendarT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = @nUserID) \r\n"
				"	DELETE FROM OutlookCategoryT WHERE UserID = @nUserID \r\n"
				"	DELETE FROM OutlookProfileT WHERE UserID = @nUserID \r\n"
				//"END \r\n"
				"COMMIT TRAN \r\n"
				, nUserID);
		}

		m_pdlUsers->RemoveRow(pUserRow);
		m_pdlUsers->PutCurSel(m_pdlUsers->GetFirstRow());
		m_bNewProfile = FALSE;	// (j.dinatale 2013-03-12 17:07) - PLID 55608 - we should be setting this to FALSE before updating our view, because we are going to an existing user
		UpdateView();
		m_prowLastSelectedUser = m_pdlUsers->GetCurSel();

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnNewNexSyncProfile()
{
	try
	{
		this->CreateNewNexSyncUser();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-10-06 12:44) - PLID 35801
void CNexSyncSettingsDlg::OnTransferNexPdaProfile()
{
	try
	{
		CSingleSelectDlg dlg(this);
		int nResult = dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID"
			, "Archived = 0 AND PersonT.ID > 0 AND PersonT.ID IN (SELECT UserID FROM OutlookFolderT WHERE IsNexSync = 0) "
			  "		AND PersonT.ID NOT IN (SELECT UserID FROM OutlookFolderT WHERE IsNexSync = 1) "
			, "PersonT.ID"
			, "PersonT.Last + ', ' + PersonT.First"
			, "Please select a NexPDA profile to transfer to NexSync"
			, true);
		if(nResult == IDOK)
		{
			long nNewUserID = dlg.GetSelectedID();
			IRowSettingsPtr pExistingUserRow = m_pdlUsers->FindByColumn(uccID, nNewUserID, m_pdlUsers->GetFirstRow(), VARIANT_FALSE);
			if(pExistingUserRow == NULL) {
				IRowSettingsPtr pNewUser = m_pdlUsers->GetNewRow();
				pNewUser->PutValue(uccID, nNewUserID);
				pNewUser->PutValue(uccName, dlg.GetSelectedDisplayValue());
				m_pdlUsers->PutCurSel(m_pdlUsers->AddRowSorted(pNewUser, NULL));
			}
			else {
				m_pdlUsers->PutCurSel(pExistingUserRow);
			}
			UpdateView();
			m_bChanged = TRUE;
			m_bNewProfile = TRUE;
			m_prowLastSelectedUser = m_pdlUsers->GetCurSel();
		}

	}NxCatchAll(__FUNCTION__);	
}

// (z.manning 2009-10-08 16:34) - PLID 35574
void CNexSyncSettingsDlg::OnBnClickedNexsyncConnectionSettings()
{
	try
	{
		// (d.singleton 2013-02-28 15:36) - PLID 55377 need to load different dialog if using google API
		// and check refresh token instead of username and password
		// (d.singleton 2013-07-03 17:35) - PLID 57446 - unify our authentication for nexsync between CalDav and GoogleApi
		CString strOldRefreshToken = m_dlgGoogleConnectionSettings.GetRefreshToken();
		m_dlgGoogleConnectionSettings.DoModal();

		if(m_dlgGoogleConnectionSettings.GetRefreshToken() != strOldRefreshToken) {
			m_bChanged = TRUE;
		}
				

		UpdateConnectionsSettingsButton();

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::UpdateConnectionsSettingsButton()
{
	// (z.manning 2009-10-08 16:34) - PLID 35574 - If the connection settings dialog needs attention
	// then set the text to red.
	if(!AreConnectionSettingsValid()) {
		m_btnConnectionSettings.SetTextColor(RGB(255, 0, 0));
	}
	else {
		m_btnConnectionSettings.AutoSet(NXB_MODIFY);
	}
	m_btnConnectionSettings.Invalidate();
}

BOOL CNexSyncSettingsDlg::AreConnectionSettingsValid()
{
	// (d.singleton 2013-02-28 15:36) - PLID 55377 need to check the correct settings if we are doing
	// google api or caldav server
	// (d.singleton 2013-07-03 17:35) - PLID 57446 - removed references to the old caldav connection settings dlg
	if(m_dlgGoogleConnectionSettings.GetRefreshToken().IsEmpty()) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (z.manning 2009-10-19 16:15) - PLID 35997
void CNexSyncSettingsDlg::OnBnClickedEditNexsyncSubjectLine()
{
	try
	{
		CNexSyncEditSubjectLineDlg dlg(this);
		CString strSubject;
		m_nxeditSubjectLine.GetWindowText(strSubject);
		dlg.SetSubject(strSubject);
		if(dlg.DoModal() == IDOK) {
			m_bChanged = TRUE;
			m_nxeditSubjectLine.SetWindowText(dlg.GetSubject());
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-10-28 10:25) - PLID 36066
BOOL CNexSyncSettingsDlg::HandleLicenseCheck()
{
	int nLicensesAllowed = g_pLicense->GetNexSyncCountAllowed();
	if(nLicensesAllowed == 0) {
		MessageBox("You are not licensed to use NexSync. Please contact NexTech at (800) 490-0821 if you are interested in purchasing it."
			, "NexSync", MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	// (z.manning 2009-10-28 10:43) - PLID 36066 - For added security, check the profile count in data
	// instead of simply using the user combo.
	ADODB::_RecordsetPtr prs = CreateRecordset(
		"SELECT COUNT(*) AS LicensesUsed \r\n"
		"FROM OutlookProfileT \r\n"
		"WHERE OutlookProfileT.UserID IN (SELECT UserID FROM OutlookFolderT WHERE IsNexSync = 1) \r\n"
		);
	long nLicensesUsed = AdoFldLong(prs->GetFields(), "LicensesUsed");
	if(nLicensesUsed >= nLicensesAllowed) {
		MessageBox(FormatString("You are licensed for %li NexSync users and are at that limit.",nLicensesAllowed)
			, "NexSync", MB_OK|MB_ICONERROR);
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (z.manning 2010-10-22 12:14) - PLID 36017
void CNexSyncSettingsDlg::OnBnClickedNexsyncAllApptTypes()
{
	try
	{
		m_bChanged = TRUE;
		EnableControls(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedNexsyncTwoWay()
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncSettingsDlg::OnBnClickedNexsyncOneWay()
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-01-28 12:27) - PLID 60016
void CNexSyncSettingsDlg::OnBnClickedNexsyncCreationOnly()
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-04-04 11:59) - PLID 38193
void CNexSyncSettingsDlg::UpdateSyncTypeRadios(TwoWaySyncValue eTwoWaySync)
{
	CheckDlgButton(IDC_NEXSYNC_TWO_WAY, BST_UNCHECKED);
	CheckDlgButton(IDC_NEXSYNC_ONE_WAY, BST_UNCHECKED);
	CheckDlgButton(IDC_NEXSYNC_CREATION_ONLY, BST_UNCHECKED);
	switch(eTwoWaySync)
	{
		case twsvTwoWay:
			CheckDlgButton(IDC_NEXSYNC_TWO_WAY, BST_CHECKED);
			break;
		case twsvOneWay:
			CheckDlgButton(IDC_NEXSYNC_ONE_WAY, BST_CHECKED);
			break;
		case twsvCreateOnly: // (z.manning 2014-01-28 12:25) - PLID 60016
			CheckDlgButton(IDC_NEXSYNC_CREATION_ONLY, BST_CHECKED);
			break;
	}
}

// (b.savon 2012-05-07 15:15) - PLID 37288 - Handle the Notes section customizing
void CNexSyncSettingsDlg::OnBnClickedEditNexsyncNotesSection()
{
	try
	{
		CNexSyncEditSubjectLineDlg dlg(this);
		CString strNotes;
		m_nxeditNotesSection.GetWindowText(strNotes);
		dlg.SetSubject(strNotes);
		dlg.SetDefaultWindowText("Edit NexSync Notes Section");
		dlg.SetDefaultDescription("Please define the layout of the appointment notes section. This will be the text that appears in the notes section of the appointment on your phone.");
		dlg.SetDefaultSampleText("Sample Notes Section");
		dlg.SetDefaultSubjectText("Notes Section");

		if(dlg.DoModal() == IDOK) {
			m_bChanged = TRUE;
			m_nxeditNotesSection.SetWindowText(dlg.GetSubject());
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-05-30 14:54) - PLID 58352
void CNexSyncSettingsDlg::OnBnClickedNexsyncClearAll()
{
	try
	{
		m_bChanged = TRUE;

		if (IsDlgButtonChecked(IDC_NEXSYNC_CLEAR_ALL) == BST_CHECKED)
		{
			// (z.manning 2014-05-30 14:54) - PLID 58352 - Give a strong warning here.
			int nMsgResult = MessageBox("Selecting this option will cause ALL Google events to be deleted from this Google calendar "
				"on the next sync, including any non-Nextech events.\r\n\r\nAre you sure you want to delete all events from this user's "
				"Google calendar?", NULL, MB_YESNO|MB_ICONQUESTION);
			if (nMsgResult != IDYES) {
				CheckDlgButton(IDC_NEXSYNC_CLEAR_ALL, BST_UNCHECKED);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}
