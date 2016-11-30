// LicenseEMRProvidersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "LicenseEMRProvidersDlg.h"
#include "PracticeRc.h"
#include "audittrail.h"
#include "NxAdo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.
// (z.manning 2013-02-04 09:22) - PLID 55001 - Reworked this dialog to also support Nuance user license setup


/////////////////////////////////////////////////////////////////////////////
// CLicenseEMRProvidersDlg dialog


enum EEMRProvColumns {
	epcID = 0,
	epcName,
};

CLicenseEMRProvidersDlg::CLicenseEMRProvidersDlg(ELicenseManageDialogType eType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CLicenseEMRProvidersDlg::IDD, pParent)
	, m_eType(eType)
{
	//{{AFX_DATA_INIT(CLicenseEMRProvidersDlg)
	//}}AFX_DATA_INIT
	m_bAllReady = false;
	m_bUsedReady = false;
	m_bInactiveReady = false;
	m_nUsed = 0;
	m_nAllowed = 0;
}


void CLicenseEMRProvidersDlg::DoDataExchange(CDataExchange* pDX)
{
	// (a.walling 2008-04-03 14:09) - PLID 29497 - Added a CNxStatic member
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseEMRProvidersDlg)
	DDX_Control(pDX, IDC_EMRLICENSING_INFO, m_labelLicensingInfo);
	DDX_Control(pDX, IDC_EMRPROV_REQUEST, m_btnRequest);
	DDX_Control(pDX, IDC_EMRPROV_DEACTIVATE, m_btnDeactivate);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseEMRProvidersDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLicenseEMRProvidersDlg)
	ON_BN_CLICKED(IDC_EMRPROV_DEACTIVATE, OnEmrProvDeactivate)
	ON_BN_CLICKED(IDC_EMRPROV_REQUEST, OnEmrProvRequest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLicenseEMRProvidersDlg message handlers

BOOL CLicenseEMRProvidersDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnDeactivate.AutoSet(NXB_RIGHT);
		m_btnRequest.AutoSet(NXB_RIGHT);
		m_btnClose.AutoSet(NXB_CLOSE);

		extern CPracticeApp theApp;

		GetDlgItem(IDC_EMRLICENSING_INFO)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_brush.CreateSolidBrush(PaletteColor(0x00BDDF97));

		// (a.walling 2006-12-19 10:20) - PLID 23397 - Initialize our datalists
		m_dlAllProvs = BindNxDataList2Ctrl(this, IDC_LIST_ALLPROVS, GetRemoteData(), false);
		m_dlUsedProvs = BindNxDataList2Ctrl(this, IDC_LIST_USEDPROVS, GetRemoteData(), false);
		m_dlInactiveProvs = BindNxDataList2Ctrl(this, IDC_LIST_INACTIVEPROVS, GetRemoteData(), false);

		CDWordArray dwaInactiveIDs, dwaUsedIDs;
		CString strInactiveIDs, strUsedIDs, str;

		BOOL bUsers = FALSE;
		switch(m_eType)
		{
			case lmdtEmrProviders:
				m_nUsed = g_pLicense->GetEMRProvidersCountUsed();
				m_nAllowed = g_pLicense->GetEMRProvidersCountAllowed();
				g_pLicense->GetInactiveEMRProviders(dwaInactiveIDs);
				g_pLicense->GetUsedEMRProviders(dwaUsedIDs);
				break;

			case lmdtNuanceUsers:
				m_nUsed = g_pLicense->GetNuanceUserCountUsed();
				m_nAllowed = g_pLicense->GetNuanceUserCountAllowed();
				g_pLicense->GetInactiveNuanceUsers(dwaInactiveIDs);
				g_pLicense->GetUsedNuanceUsers(dwaUsedIDs);
				SetWindowText("Manage Dictation User Licenses");
				SetDlgItemText(IDC_MANAGE_LICENSES_LABEL, "Licensed Dictation Users");
				SetDlgItemText(IDC_MANAGE_LICENSES_AVAILABLE_LABEL, "Available Users");
				SetDlgItemText(IDC_MANAGE_LICENSES_INACTIVE_LABEL, "Inactive Dictation Users");
				bUsers = TRUE;
				break;

			case lmdtPortalProviders: // (z.manning 2015-06-18 15:10) - PLID 66280
				m_nUsed = g_pLicense->GetPortalProviderCountUsed();
				m_nAllowed = g_pLicense->GetPortalProviderCountAllowed();
				g_pLicense->GetInactivePortalProviders(dwaInactiveIDs);
				g_pLicense->GetUsedPortalProviders(dwaUsedIDs);
				SetWindowText("Manage Portal Provider Licenses");
				SetDlgItemText(IDC_MANAGE_LICENSES_LABEL, "Licensed Portal Providers");
				SetDlgItemText(IDC_MANAGE_LICENSES_AVAILABLE_LABEL, "Available Providers");
				SetDlgItemText(IDC_MANAGE_LICENSES_INACTIVE_LABEL, "Inactive Portal Providers");
				break;
		}

		if(bUsers)
		{
			_bstr_t bstrFrom = "PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID";
			_bstr_t bstrNameCol = "UsersT.Username";
			_bstr_t bstrWhere = "PersonT.ID > 0";
			m_dlAllProvs->FromClause = bstrFrom;
			m_dlUsedProvs->FromClause = bstrFrom;
			m_dlInactiveProvs->FromClause = bstrFrom;
			m_dlAllProvs->WhereClause = bstrWhere;
			m_dlUsedProvs->WhereClause = bstrWhere;
			m_dlInactiveProvs->WhereClause = bstrWhere;
			m_dlAllProvs->GetColumn(epcName)->FieldName = bstrNameCol;
			m_dlUsedProvs->GetColumn(epcName)->FieldName = bstrNameCol;
			m_dlInactiveProvs->GetColumn(epcName)->FieldName = bstrNameCol;
		}
		else {
			// (z.manning 2013-02-04 09:36) - PLID 55001 - Only other person type supported currently is providers
			// which are already setup in resources.
		}

		strInactiveIDs = ArrayAsString(dwaInactiveIDs, true);
		strUsedIDs = ArrayAsString(dwaUsedIDs, true);

		m_bAllReady = false;
		m_bUsedReady = false;
		m_bInactiveReady = false;
		
		if (!strInactiveIDs.IsEmpty()) {
			str.Format("PersonT.ID > 0 AND PersonT.ID IN (%s)", strInactiveIDs);
			m_dlInactiveProvs->PutWhereClause((_bstr_t)str);
			m_dlInactiveProvs->Requery();
		}
		else {
			m_bInactiveReady = true;
		}
			
		if (!strUsedIDs.IsEmpty()) {
			str.Format("PersonT.ID > 0 AND PersonT.ID IN (%s)", strUsedIDs);
			m_dlUsedProvs->PutWhereClause((_bstr_t)str);
			m_dlUsedProvs->Requery();
		}
		else {
			m_bUsedReady = true;
		}

		// (z.manning 2014-02-06 09:46) - PLID 55001 - Be sure to filter on person ID greater than 0
		if (!strUsedIDs.IsEmpty() && !strInactiveIDs.IsEmpty()) {
			str.Format("PersonT.ID > 0 AND PersonT.ID NOT IN (%s) AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0", strUsedIDs, strInactiveIDs);
		}
		else if (!strUsedIDs.IsEmpty()) {
			str.Format("PersonT.ID > 0 AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0", strUsedIDs);
		}
		else if (!strInactiveIDs.IsEmpty()) {
			str.Format("PersonT.ID > 0 AND PersonT.ID NOT IN (%s) AND PersonT.Archived = 0", strInactiveIDs);
		}
		else {
			str.Format("PersonT.ID > 0 AND PersonT.Archived = 0");
		}
		m_dlAllProvs->PutWhereClause((_bstr_t)str);
		m_dlAllProvs->Requery();

		EnableButtons();
	}
	NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLicenseEMRProvidersDlg::OnEmrProvDeactivate() 
{
	try
	{
		CWaitCursor cws;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlUsedProvs->GetCurSel();

		if (pRow)
		{
			CString str1, str2, strPersonType;
			switch(m_eType)
			{
				case lmdtEmrProviders:
					str1 = "provider's EMR";
					str2 = "EMRs";
					strPersonType = "provider";
					break;

				case lmdtNuanceUsers:
					str1 = "user's dictation";
					str2 = "dictation";
					strPersonType = "user";
					break;

				case lmdtPortalProviders: // (z.manning 2015-06-18 15:10) - PLID 66280
					str1 = "provider's portal";
					str2 = "the portal";
					strPersonType = "provider";
					break;
			}
			CString strMessage = FormatString("Once a %s license has been deactivated, it can not be reactivated! "
				"Please use this feature only if the %s will no longer be using %s.\r\n"
				"If you have any questions or need assistance, please call Nextech Support.\r\n\r\n"
				"Are you sure you are ready to deactivate this %s license?"
				, str1, strPersonType, str2, str1);
			if (IDYES == MessageBox(strMessage, "Practice", MB_YESNO|MB_ICONQUESTION))
			{
				long nID = VarLong(pRow->GetValue(epcID), -1);

				if (nID > 0)
				{
					BOOL bResult = FALSE;
					AuditEventItems aei;
					switch(m_eType)
					{
						case lmdtEmrProviders:
							{
								//(r.wilson 7/29/2013) PLID 48684 - See if any providers have this provider s their primary EMR provider
								// (z.manning 2014-01-28 16:29) - PLID 60512 - This query forgot the provider ID so this never
								// actually worked (just used a junk value). I fixed that and parameterized.
								ADODB::_RecordsetPtr rs = CreateParamRecordset(
									"SELECT Count(PersonID) AS LinkedCount \r\n"
									"FROM ProvidersT \r\n"
									"WHERE EMRDefaultProviderID = {INT} \r\n"
									, nID);
								long nLinkedCount = 0;
								if(!rs->eof){
									nLinkedCount = AdoFldLong(rs, "LinkedCount");	
								}
								// (z.manning 2014-01-28 16:32) - PLID 60512 - Changed the count to check greater than zero (instead of 1)
								if(nLinkedCount > 0 && MessageBox("This provider is set as the default licensed EMR provider for one or more providers. Are you sure you want to inactivate this provider's license?","Practice",MB_YESNO) == IDNO)
								{
									return;
								}

								bResult = g_pLicense->DeactivateEMRProvider(nID);
								aei = aeiEMRProviderDeactivate;

								//(r.wilson 7/29/2013) PLID 48684 - NULL out this provider
								// (z.manning 2014-01-28 16:34) - PLID 60512 - Parameterized
								if(nLinkedCount > 0) {
									ExecuteParamSql(
										"UPDATE ProvidersT SET EMRDefaultProviderID = NULL WHERE EMRDefaultProviderID = {INT}"
										, nID);
								}
							}
							break;

						case lmdtNuanceUsers:
							bResult = g_pLicense->DeactivateNuanceUser(nID);
							aei = aeiNuanceUserDeactivate;
							break;

						case lmdtPortalProviders: // (z.manning 2015-06-18 15:11) - PLID 66280
							bResult = g_pLicense->DeactivatePortalProvider(nID);
							aei = aeiPortalProviderDeactivate;
							break;
					}

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();
						// (a.walling 2008-06-02 17:57) - PLID 28267 - Audit licensing request
						AuditEvent(-1, GetExistingContactName(nID), nAuditID, aei, nID, "Licensed", "Deactivated", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlInactiveProvs->GetNewRow();

						pNewRow->PutValue(epcID, pRow->GetValue(epcID));
						pNewRow->PutValue(epcName, pRow->GetValue(epcName));

						m_dlInactiveProvs->AddRowAtEnd(pNewRow, NULL);
						m_dlUsedProvs->RemoveRow(pRow);					
						EnableButtons();
					}
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CLicenseEMRProvidersDlg::OnEmrProvRequest() 
{
	try
	{
		CWaitCursor cws;
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAllProvs->GetCurSel();

		if (pRow)
		{
			CString strPersonType, strLicenseType;
			CString strArticle = "a";
			switch(m_eType)
			{
				case lmdtEmrProviders:
					strPersonType = "provider";
					strLicenseType = "EMR";
					strArticle = "an";
					break;

				case lmdtNuanceUsers:
					strPersonType = "user";
					strLicenseType = "dictation";
					break;

				case lmdtPortalProviders: // (z.manning 2015-06-18 15:12) - PLID 66280
					strPersonType = "provider";
					strLicenseType = "portal";
					break;
			}
			CString strMessage = FormatString("Once a %s acquires %s %s license, the %s will hold that license until they are deactivated.\r\n"
				"If you have any questions or need assistance, please call Nextech Support.\r\n\r\n"
				"Are you sure you are ready to allocate this %s license?"
				, strPersonType, strArticle, strLicenseType, strPersonType, strLicenseType);
			if (IDYES == MessageBox(strMessage, "Practice", MB_YESNO|MB_ICONQUESTION))
			{
				long nID = VarLong(pRow->GetValue(epcID), -1);

				if (nID > 0)
				{
					BOOL bResult = FALSE;
					AuditEventItems aei;
					switch(m_eType)
					{
						case lmdtEmrProviders:
							bResult = g_pLicense->RequestEMRProvider(nID);
							aei = aeiEMRProviderRequest;
							break;

						case lmdtNuanceUsers:
							bResult = g_pLicense->RequestNuanceUser(nID);
							aei = aeiNuanceUserRequest;
							break;

						case lmdtPortalProviders: // (z.manning 2015-06-18 15:13) - PLID 66280
							bResult = g_pLicense->RequestPortalProvider(nID);
							aei = aeiPortalProviderRequest;
							break;
					}

					if (bResult)
					{
						long nAuditID = BeginNewAuditEvent();
						// (a.walling 2008-06-02 17:57) - PLID 28267 - Audit licensing request
						AuditEvent(-1, GetExistingContactName(nID), nAuditID, aei, nID, "", "Licensed", aepHigh, aetChanged);
						
						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlUsedProvs->GetNewRow();

						pNewRow->PutValue(epcID, pRow->GetValue(epcID));
						pNewRow->PutValue(epcName, pRow->GetValue(epcName));

						m_dlUsedProvs->AddRowAtEnd(pNewRow, NULL);
						m_dlAllProvs->RemoveRow(pRow);
						EnableButtons();
					}
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CLicenseEMRProvidersDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLicenseEMRProvidersDlg)
	ON_EVENT(CLicenseEMRProvidersDlg, IDC_LIST_ALLPROVS, 18 /* RequeryFinished */, OnRequeryFinishedListAllProvs, VTS_I2)
	ON_EVENT(CLicenseEMRProvidersDlg, IDC_LIST_USEDPROVS, 18 /* RequeryFinished */, OnRequeryFinishedListUsedProvs, VTS_I2)
	ON_EVENT(CLicenseEMRProvidersDlg, IDC_LIST_INACTIVEPROVS, 18 /* RequeryFinished */, OnRequeryFinishedListInactiveProvs, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CLicenseEMRProvidersDlg, IDC_LIST_ALLPROVS, 3, CLicenseEMRProvidersDlg::DblClickCellListAllprovs, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CLicenseEMRProvidersDlg::OnRequeryFinishedListAllProvs(short nFlags) 
{
	m_bAllReady = true;	
	EnableButtons();
}

void CLicenseEMRProvidersDlg::OnRequeryFinishedListUsedProvs(short nFlags) 
{
	m_bUsedReady = true;	
	EnableButtons();
}

void CLicenseEMRProvidersDlg::OnRequeryFinishedListInactiveProvs(short nFlags) 
{
	m_bInactiveReady = true;	
	EnableButtons();
}

void CLicenseEMRProvidersDlg::EnableButtons()
{
	if (m_bAllReady && m_bUsedReady && m_bInactiveReady) {
		UpdateInfoLabel();

		if (m_nUsed > 0) {
			m_btnDeactivate.EnableWindow(TRUE);
		} else {
			m_btnDeactivate.EnableWindow(FALSE);
		}

		if (m_nUsed == m_nAllowed) {
			m_btnRequest.EnableWindow(FALSE);
		} else {
			m_btnRequest.EnableWindow(TRUE);
		}

		//m_btnDeactivate.AutoSet(NXB_RIGHT);
		//m_btnRequest.AutoSet(NXB_RIGHT);
	} else {
		m_btnDeactivate.EnableWindow(FALSE);
		m_btnRequest.EnableWindow(FALSE);
	}
}

void CLicenseEMRProvidersDlg::UpdateInfoLabel()
{
	try {

		switch(m_eType)
		{
			case lmdtEmrProviders:
				m_nUsed = g_pLicense->GetEMRProvidersCountUsed();
				m_nAllowed = g_pLicense->GetEMRProvidersCountAllowed();
				break;

			case lmdtNuanceUsers:
				m_nUsed = g_pLicense->GetNuanceUserCountUsed();
				m_nAllowed = g_pLicense->GetNuanceUserCountAllowed();
				break;

			case lmdtPortalProviders: // (z.manning 2015-06-18 15:14) - PLID 66280
				m_nUsed = g_pLicense->GetPortalProviderCountUsed();
				m_nAllowed = g_pLicense->GetPortalProviderCountAllowed();
				break;
		}

		long nRemaining = m_nAllowed - m_nUsed;

		if (nRemaining < 0) {
			ASSERT(FALSE); // they have more licenses in use than they have licensed!!
			nRemaining = 0;
		}

		CString strMsg;
		strMsg.Format("Using %li of %li licenses (%li remaining)", m_nUsed, m_nAllowed, nRemaining);

		SetDlgItemText(IDC_EMRLICENSING_INFO, strMsg);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2013-02-04 16:30) - PLID 55001
void CLicenseEMRProvidersDlg::DblClickCellListAllprovs(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		OnEmrProvRequest();
	}
	NxCatchAll(__FUNCTION__)
}
