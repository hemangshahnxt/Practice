// AnalyticsLicensingConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "AnalyticsLicensingConfigDlg.h"
#include "afxdialogex.h"
#include <NxPracticeSharedLib\SupportUtils.h>

// (r.goldschmidt 2015-04-13 10:24) - PLID 65480 - Create Configure Analytics License dialog and all of its UI elements.
// CAnalyticsLicensingConfigDlg dialog

enum EAnalyticsDocumentsColumns {
	adcID,
	adcDocumentName,
	adcInactive
};

IMPLEMENT_DYNAMIC(CAnalyticsLicensingConfigDlg, CNxDialog)

CAnalyticsLicensingConfigDlg::CAnalyticsLicensingConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAnalyticsLicensingConfigDlg::IDD, pParent)
{
	
}

CAnalyticsLicensingConfigDlg::~CAnalyticsLicensingConfigDlg()
{
}

void CAnalyticsLicensingConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXCOLOR_ANALYTICS_LICENSING_CONFIG, m_background);
	DDX_Control(pDX, IDC_EXPIRATIONDATE, m_dtExpirationPicker);
}

BOOL CAnalyticsLicensingConfigDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		m_background.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		((CNxIconButton*) GetDlgItem(ID_SAVE_CLOSE))->AutoSet(NXB_MODIFY);
		((CNxIconButton*) GetDlgItem(ID_SAVE_ADDANOTHER))->AutoSet(NXB_MODIFY);
		((CNxIconButton*) GetDlgItem(IDCANCEL))->AutoSet(NXB_CANCEL);

		m_pDocumentList = BindNxDataList2Ctrl(IDC_DOCUMENTNAME, true);
		m_pDocumentList->Requery();

		// add row for no document
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetNewRow();
		pRow->PutValue(adcID, _variant_t((long)-1));
		pRow->PutValue(adcDocumentName, _bstr_t("< no document selected >"));
		pRow->PutValue(adcInactive, _variant_t(false));
		m_pDocumentList->AddRowBefore(pRow, m_pDocumentList->GetFirstRow());

		// select user's actual document
		// if user has inactive document, make no document say inactive document instead and select
		pRow = m_pDocumentList->FindByColumn(adcID, _variant_t(m_structInitialLicenseUserCombo.m_nDocumentID), m_pDocumentList->GetFirstRow(), TRUE);
		if (!pRow){
			pRow = m_pDocumentList->GetFirstRow();
			pRow->PutValue(adcDocumentName, _bstr_t(m_structInitialLicenseUserCombo.m_strDocumentName + " (inactive)"));
			m_pDocumentList->SetSelByColumn(adcID, _variant_t(-1));
		}

		m_dtExpirationPicker.SetTime(m_structInitialLicenseUserCombo.m_dtExpirationDate);
		m_structNewLicenseUserCombo.m_dtExpirationDate = m_structInitialLicenseUserCombo.m_dtExpirationDate;

		GetDlgItem(IDC_USERNAME)->SetWindowText(m_structInitialLicenseUserCombo.m_strUserName);
		
		GetDlgItem(IDC_DATABASENAME)->SetWindowText(m_structInitialLicenseUserCombo.m_strDatabaseName);

		EnsureControls();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// enable or disable buttons based on context; if removing, immediately saves and closes
void CAnalyticsLicensingConfigDlg::EnsureControls()
{

	// (r.goldschmidt 2015-12-10 11:31) - PLID 67697 - Show or hide controls based on permissions
	// allow freely editing if the user has the Support Bought permission
	if (GetCurrentUserPermissions(bioSupportBought, sptWrite) || m_bIsTestAccount) {
		// the user has full permissions, allow default dialog state of everything enabled
	}
	else if (GetCurrentUserPermissions(bioSupportEditAnalyticsLicenses, sptWrite)) {
		// disable expiration date only if performing the edit action
		if (m_eAction == EAnalyticsLicenseAction::alaEdit) {
			GetDlgItem(IDC_EXPIRATIONDATE)->EnableWindow(FALSE);
		}
	}
	else { // user shouldn't have been able to get to this dialog without either of the above permissions
		ASSERT(FALSE);
		EndDialog(IDCANCEL);
	}

	//if editing, hide 'save and add another' button and update instructions
	if (m_eAction == alaEdit){
		GetDlgItem(ID_SAVE_ADDANOTHER)->EnableWindow(FALSE);
		ShowDlgItem(ID_SAVE_ADDANOTHER, SW_HIDE);
		GetDlgItem(IDC_ANALYTICS_INSTRUCTIONS)->SetWindowText("User information is required to use Nextech Analytics. It may be omitted if a license is not yet assigned to a user.\r\n\nSaving edited user information will immediately turn off analytics access for the old username/database/document combination.");
	}

	//if removing, immediately save and close dialog
	if (m_eAction == alaRemove){
		m_structNewLicenseUserCombo = CAnalyticsLicenseUserCombo(m_structInitialLicenseUserCombo.m_dtExpirationDate);
		Save();
		EndDialog(ID_SAVE_CLOSE);
	}

	//if the expiration date is in the past, disable the user name/document name/database name fields
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTime dtToday = COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
	if (m_dtExpirationPicker.GetDateTime() < dtToday) {
		GetDlgItem(IDC_USERNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOCUMENTNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_DATABASENAME)->EnableWindow(FALSE);
	}

}

bool CAnalyticsLicensingConfigDlg::Validate()
{
	UpdateNewUser();

	// not allowed to empty out user info and then save because that is the remove user action
	if (!m_structInitialLicenseUserCombo.IsUserNameEmpty() && m_structNewLicenseUserCombo.IsUserNameEmpty()){
		return false;
	}
	else {
		return m_structNewLicenseUserCombo.IsValidUserNameDocumentDatabaseCombo();
	}
}

void CAnalyticsLicensingConfigDlg::Save()
{
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTime dtYesterday = COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0) - COleDateTimeSpan(1, 0, 0, 0);

	//variables for auditing
	CString strOld, strNew, strNotes;
	CString strFieldName = "AnalyticsLicensing";

	// (r.goldschmidt 2015-06-15 11:14) - PLID 66386 - Account for database name information for upgrading when adding and editing new licenses for Analytics.
	// if not empty, it is necessarily an added or edited license that may potentially have changed
	if (!m_structNewLicenseUserCombo.IsUserNameEmpty()){
		// Find out if the client is in the upgrade list, and the DatabaseName to be saved is not in the upgrade list
		if (ReturnsRecordsParam(R"(
			SELECT TOP 1 ClientID 
			FROM AnalyticsUpgradesT 
			WHERE ClientID = {INT} AND 
			NOT EXISTS 
				(SELECT TOP 1 CLIENTID FROM AnalyticsUpgradesT 
					WHERE ClientID = {INT} AND DatabaseName = {STRING})
			)", m_nClientID, m_nClientID, m_structNewLicenseUserCombo.m_strDatabaseName)){
			ExecuteParamSql(R"(
				INSERT INTO AnalyticsUpgradesT 
				SELECT TOP 1 ClientID, VersionID, UserID, {STRING} AS DatabaseName 
				FROM AnalyticsUpgradesT 
				WHERE ClientID = {INT} 
				)",	m_structNewLicenseUserCombo.m_strDatabaseName, m_nClientID);
		}
	}

	switch (m_eAction){

	// (r.goldschmidt 2015-04-13 10:25) - PLID 65481 - Adding new analytics license for a client, and audit.
	case alaAdd:
		strNotes = "Action: Add License/User";
		ExecuteParamSql("INSERT INTO AnalyticsLicensesT (ClientID, ExpirationDate, UserName, DocumentID, DatabaseName) VALUES ({INT}, {VT_DATE}, {VT_BSTR}, {VT_I4}, {VT_BSTR})",
			m_nClientID,
			_variant_t(m_structNewLicenseUserCombo.m_dtExpirationDate, VT_DATE),
			m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strUserName),
			m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_nDocumentID),
			m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strDatabaseName));
		// auditing
		{
			strOld = "";
			strNew = "Expiration: " + FormatDateTimeForInterface(m_structNewLicenseUserCombo.m_dtExpirationDate, dtoDate);
			if (m_structNewLicenseUserCombo.IsUserNameEmpty()){
				strNew += " User: Document: Database:";
			}
			else{
				strNew += " User: " + m_structNewLicenseUserCombo.m_strUserName;
				strNew += " Document: " + m_structNewLicenseUserCombo.m_strDocumentName;
				strNew += " Database: " + m_structNewLicenseUserCombo.m_strDatabaseName;
			}
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (add new license/user)");
		}
		break;

	// (r.goldschmidt 2015-04-13 10:25) - PLID 65482 - Editing existing analytics license for a client, and audit.
	case alaEdit:
		strNotes = "Action: Edit License/User";
		// if nothing changed, just exit
		if (m_structInitialLicenseUserCombo.IsSameExpirationDate(m_structNewLicenseUserCombo) &&
			m_structInitialLicenseUserCombo.IsSameUserNameDocumentDatabaseCombo(m_structNewLicenseUserCombo)){
			break;
		}
		// if the original user name was empty, update current record
		if (m_structInitialLicenseUserCombo.IsUserNameEmpty()){
			ExecuteParamSql("UPDATE AnalyticsLicensesT Set ExpirationDate = {VT_DATE}, UserName = {VT_BSTR}, DocumentID = {VT_I4}, DatabaseName = {VT_BSTR} WHERE ID = {INT}",
				_variant_t(m_structNewLicenseUserCombo.m_dtExpirationDate, VT_DATE),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strUserName),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_nDocumentID),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strDatabaseName),
				m_nLicenseID);
			// auditing
			{
				strOld = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strOld += " User: Document: Database:";

				strNew = "Expiration: " + FormatDateTimeForInterface(m_structNewLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strNew += " User: " + m_structNewLicenseUserCombo.m_strUserName;
				strNew += " Document: " + m_structNewLicenseUserCombo.m_strDocumentName;
				strNew += " Database: " + m_structNewLicenseUserCombo.m_strDatabaseName;

				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (apply user to license)");
			}
		}
		// if user name combo remains the same, update current record
		else if (m_structNewLicenseUserCombo.IsSameUserNameDocumentDatabaseCombo(m_structInitialLicenseUserCombo)){
			ExecuteParamSql("UPDATE AnalyticsLicensesT Set ExpirationDate = {VT_DATE} WHERE ID = {INT}",
				_variant_t(m_structNewLicenseUserCombo.m_dtExpirationDate, VT_DATE),
				m_nLicenseID);
			// auditing
			{
				strOld = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strOld += " User: " + m_structInitialLicenseUserCombo.m_strUserName;
				strOld += " Document: " + m_structInitialLicenseUserCombo.m_strDocumentName;
				strOld += " Database: " + m_structInitialLicenseUserCombo.m_strDatabaseName;

				strNew = "Expiration: " + FormatDateTimeForInterface(m_structNewLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strNew += " User: " + m_structNewLicenseUserCombo.m_strUserName;
				strNew += " Document: " + m_structNewLicenseUserCombo.m_strDocumentName;
				strNew += " Database: " + m_structNewLicenseUserCombo.m_strDatabaseName;

				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (update expiration)");
			}
		}
		// else, update the existing record to have expiration of yesterday, insert a new record with new expiration date and user name combo
		else{
			// if the user is already expired, don't save it with a new expired date
			if (m_structInitialLicenseUserCombo.m_dtExpirationDate > dtYesterday){
				ExecuteParamSql("UPDATE AnalyticsLicensesT Set ExpirationDate = {VT_DATE} WHERE ID = {INT}", _variant_t(dtYesterday, VT_DATE), m_nLicenseID);
			}
			ExecuteParamSql("INSERT INTO AnalyticsLicensesT (ClientID, ExpirationDate, UserName, DocumentID, DatabaseName) VALUES ({INT}, {VT_DATE}, {VT_BSTR}, {VT_I4}, {VT_BSTR})",
				m_nClientID,
				_variant_t(m_structNewLicenseUserCombo.m_dtExpirationDate, VT_DATE),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strUserName),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_nDocumentID),
				m_structNewLicenseUserCombo.IsUserNameEmpty() ? g_cvarNull : _variant_t(m_structNewLicenseUserCombo.m_strDatabaseName));
			// auditing
			// expiring the old record; only if the license hasn't expired already
			if (m_structInitialLicenseUserCombo.m_dtExpirationDate > dtYesterday)
			{
				strOld = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strNew = "Expiration: " + FormatDateTimeForInterface(dtYesterday, dtoDate);

				strOld += " User: " + m_structInitialLicenseUserCombo.m_strUserName;
				strOld += " Document: " + m_structInitialLicenseUserCombo.m_strDocumentName;
				strOld += " Database: " + m_structInitialLicenseUserCombo.m_strDatabaseName;

				strNew += " User: " + m_structInitialLicenseUserCombo.m_strUserName;
				strNew += " Document: " + m_structInitialLicenseUserCombo.m_strDocumentName;
				strNew += " Database: " + m_structInitialLicenseUserCombo.m_strDatabaseName;

				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (expire user)");
			}
			// adding the new record
			{
				strOld = "";
				strNew = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
				strNew += " User: " + m_structNewLicenseUserCombo.m_strUserName;
				strNew += " Document: " + m_structNewLicenseUserCombo.m_strDocumentName;
				strNew += " Database: " + m_structNewLicenseUserCombo.m_strDatabaseName;
				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (add new license/user)");
			}
		}
		break;

	// (r.goldschmidt 2015-04-13 10:25) - PLID 65483 - Removing an existing analytics user for a client, and audit.
	case alaRemove:
		strNotes = "Action: Remove User";
		ExecuteParamSql("UPDATE AnalyticsLicensesT Set ExpirationDate = {VT_DATE} WHERE ID = {INT} \r\n"
			"INSERT INTO AnalyticsLicensesT (ClientID, ExpirationDate) VALUES ({INT}, {VT_DATE})",
			_variant_t(dtYesterday, VT_DATE),
			m_nLicenseID,
			m_nClientID,
			_variant_t(m_structInitialLicenseUserCombo.m_dtExpirationDate, VT_DATE));
		// auditing
		// expiring the old record
		{
			strOld = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
			strNew = "Expiration: " + FormatDateTimeForInterface(dtYesterday, dtoDate);

			strOld += " User: " + m_structInitialLicenseUserCombo.m_strUserName;
			strOld += " Document: " + m_structInitialLicenseUserCombo.m_strDocumentName;
			strOld += " Database: " + m_structInitialLicenseUserCombo.m_strDatabaseName;
			
			strNew += " User: " + m_structInitialLicenseUserCombo.m_strUserName;
			strNew += " Document: " + m_structInitialLicenseUserCombo.m_strDocumentName;
			strNew += " Database: " + m_structInitialLicenseUserCombo.m_strDatabaseName;

			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (expire user)");
		}
		// adding the new record
		{
			strOld = "";
			strNew = "Expiration: " + FormatDateTimeForInterface(m_structInitialLicenseUserCombo.m_dtExpirationDate, dtoDate);
			strNew += " User: Document: Database:";
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strFieldName, strOld, strNew, strNotes + " (add new license)");
		}
		break;
	default:
		ASSERT(false); // there is a new action enum that needs to be accounted for
		break;
	}
}

void CAnalyticsLicensingConfigDlg::SetSaveType(EAnalyticsLicenseAction eAction)
{
	m_eAction = eAction;
}

void CAnalyticsLicensingConfigDlg::SetClient(long nClientID)
{
	m_nClientID = nClientID;
}

void CAnalyticsLicensingConfigDlg::SetLicense(long nLicenseID)
{
	m_nLicenseID = nLicenseID;
}

void CAnalyticsLicensingConfigDlg::SetInitialUser(const CAnalyticsLicenseUserCombo &structLicenseUser)
{
	m_structInitialLicenseUserCombo = structLicenseUser;
}

COleDateTime CAnalyticsLicensingConfigDlg::GetSavedUserExpiration()
{
	return m_structNewLicenseUserCombo.m_dtExpirationDate;
}

// set new user values to current dialog values
void CAnalyticsLicensingConfigDlg::UpdateNewUser()
{
	//update expiration date
	m_structNewLicenseUserCombo.m_dtExpirationDate = m_dtExpirationPicker.GetDateTime();

	//update user name
	GetDlgItem(IDC_USERNAME)->GetWindowText(m_structNewLicenseUserCombo.m_strUserName);
		
	//update database name
	GetDlgItem(IDC_DATABASENAME)->GetWindowText(m_structNewLicenseUserCombo.m_strDatabaseName);
		
	//update document id
	m_structNewLicenseUserCombo.m_nDocumentID = VarLong(m_pDocumentList->CurSel->GetValue(adcID), -1);

	//update document name
	m_structNewLicenseUserCombo.m_strDocumentName = VarString(m_pDocumentList->CurSel->GetValue(adcDocumentName), "");

}

BEGIN_MESSAGE_MAP(CAnalyticsLicensingConfigDlg, CNxDialog)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPIRATIONDATE, &CAnalyticsLicensingConfigDlg::OnDtnDatetimechangeExpirationdate)
	ON_BN_CLICKED(ID_SAVE_CLOSE, &CAnalyticsLicensingConfigDlg::OnBnClickedSaveClose)
	ON_BN_CLICKED(ID_SAVE_ADDANOTHER, &CAnalyticsLicensingConfigDlg::OnBnClickedSaveAddanother)
END_MESSAGE_MAP()


// CAnalyticsLicensingConfigDlg message handlers

// when date range is changed, make sure it isn't set to the past, otherwise reset it.
void CAnalyticsLicensingConfigDlg::OnDtnDatetimechangeExpirationdate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try{
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtToday = COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
		if (m_dtExpirationPicker.GetDateTime() < dtToday) {
			m_dtExpirationPicker.SetTime(m_structNewLicenseUserCombo.m_dtExpirationDate);
		}
		else{
			m_structNewLicenseUserCombo.m_dtExpirationDate = m_dtExpirationPicker.GetDateTime();
			GetDlgItem(IDC_USERNAME)->EnableWindow(TRUE);
			GetDlgItem(IDC_DOCUMENTNAME)->EnableWindow(TRUE);
			GetDlgItem(IDC_DATABASENAME)->EnableWindow(TRUE);
		}
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CAnalyticsLicensingConfigDlg, CNxDialog)
END_EVENTSINK_MAP()


void CAnalyticsLicensingConfigDlg::OnBnClickedSaveClose()
{
	try{
		if (Validate()){
			Save();
			EndDialog(ID_SAVE_CLOSE);
		}
		else {
			MessageBox("Please make sure there are proper entries for User Name, Document, and Database Name.");
		}
	}NxCatchAll(__FUNCTION__);
}


void CAnalyticsLicensingConfigDlg::OnBnClickedSaveAddanother()
{
	try{
		if (Validate()){
			Save();
			EndDialog(ID_SAVE_ADDANOTHER);
		}
		else {
			MessageBox("Please make sure there are proper entries for User Name, Document, and Database Name.");
		}
	}NxCatchAll(__FUNCTION__);
}
