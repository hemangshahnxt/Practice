// AnalyticsLicensingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "AnalyticsLicensingDlg.h"
#include "afxdialogex.h"

// (r.goldschmidt 2015-04-07 13:13) - PLID 65479 - Create Manage Analytics Licenses dialog and all of its UI elements.
// CAnalyticsLicensingDlg dialog

// (r.goldschmidt 2015-04-08 20:03) - PLID 65478 - Updates to Internal database structure to track assignments for Analytics Licensing.
/*

CREATE TABLE AnalyticsDocumentsT
(
	ID int IDENTITY(1,1) NOT NULL,
	DocumentName nvarchar(255) NOT NULL,
	Inactive bit DEFAULT 0 NOT NULL,
	CONSTRAINT PK_AnalyticsDocumentsT PRIMARY KEY CLUSTERED (ID)
)


CREATE TABLE AnalyticsLicensesT
(
	ID int IDENTITY(1,1) NOT NULL,
	ClientID int NOT NULL,
	ExpirationDate datetime NOT NULL,
	UserName nvarchar(100),
	DocumentID int,
	DatabaseName nvarchar(100),
	LicenseApplied bit DEFAULT 0 NOT NULL,
	CONSTRAINT PK_AnalyticsLicensesT PRIMARY KEY CLUSTERED (ID),
	CONSTRAINT FK_AnalyticsLicensesT_NxClientsT FOREIGN KEY (ClientID) REFERENCES NxClientsT(PersonID),
	CONSTRAINT FK_AnalyticsLicensesT_AnalyticsDocumentsT FOREIGN KEY (DocumentID) REFERENCES AnalyticsDocumentsT(ID)
)

*/

enum EAnalyticsLicenseColumns {
	alcID,
	alcLive,
	alcExpirationDate,
	alcIsNotExpired,
	alcUserName,
	alcDocumentID,
	alcDocumentName,
	alcDocumentInactive,
	alcDatabaseName
};

IMPLEMENT_DYNAMIC(CAnalyticsLicensingDlg, CNxDialog)

CAnalyticsLicensingDlg::CAnalyticsLicensingDlg(CWnd* pParent /*=NULL*/, long nClientID /*=-1*/)
: CNxDialog(CAnalyticsLicensingDlg::IDD, pParent), m_nClientID(nClientID)
{

}

CAnalyticsLicensingDlg::~CAnalyticsLicensingDlg()
{
}

void CAnalyticsLicensingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXCOLOR_ANALYTICS_LICENSING, m_background);
	DDX_Control(pDX, IDADD_ANALYTICS, m_nxbAdd);
	DDX_Control(pDX, IDEDIT_ANALYTICS, m_nxbEdit);
	DDX_Control(pDX, IDREMOVE_ANALYTICS, m_nxbRemove);
	DDX_Control(pDX, IDCLOSE, m_nxbClose);
}

BOOL CAnalyticsLicensingDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbEdit.AutoSet(NXB_MODIFY);
		m_nxbRemove.AutoSet(NXB_DELETE);
		m_nxbClose.AutoSet(NXB_CLOSE);

		m_background.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_bShowExpired = false;

		CString strWhere;
		strWhere.Format("AnalyticsLicensesT.ClientID = %li", m_nClientID);

		m_pListLicenses = BindNxDataList2Ctrl(IDC_LISTANALYTICSLICENSES, false);
		m_pListLicenses->WhereClause = _bstr_t(strWhere);
		m_pListLicenses->Requery();

		SecureControls();
		SetControls(FALSE);

		// (r.goldschmidt 2015-05-07 11:19) - PLID 65935 - Add Analytics Version information to the Manage Analytics Licenses dialog in Internal Practice
		CString strCurrentVersionInfo = "Current Analytics Version: <not active>";
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT AnalyticsVersionsT.Version FROM NxClientsT LEFT JOIN AnalyticsVersionsT ON NxClientsT.AnalyticsVersionID = AnalyticsVersionsT.ID WHERE NxClientsT.PersonID = {INT}", m_nClientID);
		if (!prs->eof){
			strCurrentVersionInfo = "Current Analytics Version: " + AdoFldString(prs, "Version", "<not active>");
		}
		GetDlgItem(IDC_ANALYTICS_CURRENT_VERSION)->SetWindowText(strCurrentVersionInfo);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CAnalyticsLicensingDlg, CNxDialog)
	ON_BN_CLICKED(IDADD_ANALYTICS, &CAnalyticsLicensingDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDEDIT_ANALYTICS, &CAnalyticsLicensingDlg::OnBnClickedEdit)
	ON_BN_CLICKED(IDREMOVE_ANALYTICS, &CAnalyticsLicensingDlg::OnBnClickedRemove)
	ON_BN_CLICKED(IDCLOSE, &CAnalyticsLicensingDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_CHECK_SHOWEXPIRED, &CAnalyticsLicensingDlg::OnBnClickedCheckShowexpired)
END_MESSAGE_MAP()


// CAnalyticsLicensingDlg message handlers

// (r.goldschmidt 2015-04-13 10:33) - PLID 65481 - Adding new analytics license for a client, and audit.
void CAnalyticsLicensingDlg::OnBnClickedAdd()
{
	try {
		ConfigureAnalyticsLicensing(alaAdd);
	} NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2015-04-13 10:33) - PLID 65482 - Editing existing analytics license for a client, and audit.
void CAnalyticsLicensingDlg::OnBnClickedEdit()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListLicenses->GetCurSel();
		if (pRow){
			ConfigureAnalyticsLicensing(alaEdit, pRow);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2015-04-13 10:32) - PLID 65483 - Removing an existing analytics user for a client, and audit.
void CAnalyticsLicensingDlg::OnBnClickedRemove()
{
	try {
		if (IDYES == MessageBox("This action will immediately turn off access for this username/database/document combination and may not be undone. However, the license will remain available and may be reassigned.\r\n\r\nAre you sure you wish to continue?", "", MB_ICONEXCLAMATION | MB_YESNO)) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListLicenses->GetCurSel();
			if (pRow){
				ConfigureAnalyticsLicensing(alaRemove, pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}


void CAnalyticsLicensingDlg::OnBnClickedClose()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}


void CAnalyticsLicensingDlg::OnBnClickedCheckShowexpired()
{
	try{

		if (m_bShowExpired)
		{
			m_bShowExpired = false;
		}
		else {
			m_bShowExpired = true;
		}

		ToggleRows();

	}NxCatchAll(__FUNCTION__);
}

// show or hide buttons based on permissions
void CAnalyticsLicensingDlg::SecureControls()
{
	if (GetCurrentUserPermissions(bioSupportBought, sptWrite) || m_bIsTestAccount){
		ShowDlgItem(IDADD_ANALYTICS, SW_SHOW);
		ShowDlgItem(IDEDIT_ANALYTICS, SW_SHOW);
		ShowDlgItem(IDREMOVE_ANALYTICS, SW_SHOW);
	}
	// (r.goldschmidt 2015-12-10 11:27) - PLID 67697 - New permission for editing analytics licenses
	else if (GetCurrentUserPermissions(bioSupportEditAnalyticsLicenses, sptWrite)) {
		ShowDlgItem(IDADD_ANALYTICS, SW_HIDE);
		ShowDlgItem(IDEDIT_ANALYTICS, SW_SHOW);
		ShowDlgItem(IDREMOVE_ANALYTICS, SW_SHOW);
	}
	else{
		ShowDlgItem(IDADD_ANALYTICS, SW_HIDE);
		ShowDlgItem(IDEDIT_ANALYTICS, SW_HIDE);
		ShowDlgItem(IDREMOVE_ANALYTICS, SW_HIDE);
	}

}

void CAnalyticsLicensingDlg::SetControls(BOOL bEnable)
{
	try{
		GetDlgItem(IDEDIT_ANALYTICS)->EnableWindow(bEnable);
		GetDlgItem(IDREMOVE_ANALYTICS)->EnableWindow(bEnable);
	}NxCatchAll(__FUNCTION__)
}

// checks status of checkbox to decide which licenses to display
void CAnalyticsLicensingDlg::ToggleRows()
{
	// make sure we finish the requery before showing/hiding rows (otherwise all rows get shown no matter what)
	m_pListLicenses->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListLicenses->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()){
		if (!m_bShowExpired && !pRow->GetValue(alcIsNotExpired)){
			pRow->PutVisible(VARIANT_FALSE);
		}
		else {
			pRow->PutVisible(VARIANT_TRUE);
		}
	}
}

void CAnalyticsLicensingDlg::ConfigureAnalyticsLicensing(EAnalyticsLicenseAction eAction, NXDATALIST2Lib::IRowSettingsPtr pRow /*= NULL*/)
{
	CAnalyticsLicenseUserCombo structLicenseUser = CAnalyticsLicenseUserCombo();
	long nLicenseID = -1;
	bool bAddAnother = false;
	long nResult;

	switch (eAction){

	case alaAdd:
		bAddAnother = true;
		while(bAddAnother){
			bAddAnother = false;

			CAnalyticsLicensingConfigDlg dlg(this);
			dlg.SetSaveType(eAction);
			dlg.SetClient(m_nClientID);
			dlg.SetLicense(nLicenseID);
			dlg.SetInitialUser(structLicenseUser);
			dlg.m_bIsTestAccount = m_bIsTestAccount; // (r.goldschmidt 2015-12-10 12:56) - PLID 67697

			nResult = dlg.DoModal();
			if (nResult == ID_SAVE_ADDANOTHER){
				bAddAnother = true;
				structLicenseUser = CAnalyticsLicenseUserCombo(dlg.GetSavedUserExpiration());
				m_pListLicenses->Requery();
			}
		}
		break;

	case alaEdit:
	case alaRemove:
		if (pRow){
			structLicenseUser.m_dtExpirationDate = VarDateTime(pRow->GetValue(alcExpirationDate));
			structLicenseUser.m_nDocumentID = VarLong(pRow->GetValue(alcDocumentID), -1);
			structLicenseUser.m_strDocumentName = VarString(pRow->GetValue(alcDocumentName), "");
			// if there is no document id, then the user name remains empty (instead of as '< available user >')
			if (structLicenseUser.m_nDocumentID != -1){
				structLicenseUser.m_strUserName = VarString(pRow->GetValue(alcUserName), "");
			}
			structLicenseUser.m_strDatabaseName = VarString(pRow->GetValue(alcDatabaseName), "PracData");
			nLicenseID = VarLong(pRow->GetValue(alcID), -1);

			CAnalyticsLicensingConfigDlg dlg(this);
			dlg.SetSaveType(eAction);
			dlg.SetClient(m_nClientID);
			dlg.SetLicense(nLicenseID);
			dlg.SetInitialUser(structLicenseUser);
			dlg.m_bIsTestAccount = m_bIsTestAccount; // (r.goldschmidt 2015-12-10 12:56) - PLID 67697

			nResult = dlg.DoModal();
		}
		break;
	
	}

	if (nResult != IDCANCEL){
		m_pListLicenses->Requery();
		SetControls(FALSE);
	}
}

BEGIN_EVENTSINK_MAP(CAnalyticsLicensingDlg, CNxDialog)
	ON_EVENT(CAnalyticsLicensingDlg, IDC_LISTANALYTICSLICENSES, 2, CAnalyticsLicensingDlg::SelChangedListanalyticslicenses, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAnalyticsLicensingDlg, IDC_LISTANALYTICSLICENSES, 18, CAnalyticsLicensingDlg::RequeryFinishedListanalyticslicenses, VTS_I2)
END_EVENTSINK_MAP()

// The user must have a selection in the datalist to access the edit and remove buttons
void CAnalyticsLicensingDlg::SelChangedListanalyticslicenses(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		if (lpNewSel == NULL){
			SetControls(FALSE);
		}
		else if (lpNewSel != NULL){
			SetControls(TRUE);

			// if selection has expiration date in the past or if there is no user yet, disable remove button
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
			COleDateTime dtSelectedRowExpiration = VarDateTime(pRow->GetValue(alcExpirationDate));
			long nDocumentID = VarLong(pRow->GetValue(alcDocumentID), -1);
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTime dtToday = COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
			if (dtSelectedRowExpiration < dtToday || nDocumentID == -1){
				GetDlgItem(IDREMOVE_ANALYTICS)->EnableWindow(FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__)
}


void CAnalyticsLicensingDlg::RequeryFinishedListanalyticslicenses(short nFlags)
{
	try{
		ToggleRows();
		UpdateLicenseCount();
	}NxCatchAll(__FUNCTION__)
}

// update display of count of unexpired licenses
void CAnalyticsLicensingDlg::UpdateLicenseCount()
{
	// make sure we finish the requery before calculating
	m_pListLicenses->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	long nLicenseCount = 0;
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListLicenses->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()){
		if (pRow->GetValue(alcIsNotExpired)){
			nLicenseCount++;
		}
	}
	CString strLicenseCount;
	strLicenseCount.Format("%li", nLicenseCount);
	GetDlgItem(IDC_ANALYTICS_LICENSE_COUNT)->SetWindowText(strLicenseCount);
}