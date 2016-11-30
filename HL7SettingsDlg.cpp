// HL7SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7SettingsDlg.h"
#include "HL7ConfigCodeLinksDlg.h"
#include "HL7LabSettingsDlg.h"
#include "HL7ParseUtils.h"
#include "AuditTrail.h"
#include "HL7PatientSettingsDlg.h"
#include "HL7SchedSettingsDlg.h"
#include "HL7Utils.h"
#include "HL7GeneralSettingsDlg.h"
#include "HL7IDLinkConfigDlg.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CHL7SettingsDlg dialog


CHL7SettingsDlg::CHL7SettingsDlg(CWnd* pParent)
: CNxDialog(CHL7SettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHL7SettingsDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHL7SettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7SettingsDlg)
	DDX_Control(pDX, IDC_HL7_SETTINGS_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_RENAME_HL7_GROUP, m_btnRename);
	DDX_Control(pDX, IDC_CONFIG_LOCATIONS, m_btnConfigLocations);
	DDX_Control(pDX, IDC_DELETE_HL7_GROUP, m_btnDelete);
	DDX_Control(pDX, IDC_ADD_HL7_GROUP, m_btnAdd);
	DDX_Control(pDX, IDC_CHECK_BATCH_IMPORTS, m_checkBatchImports);
	DDX_Control(pDX, IDC_CHECK_BATCH_EXPORTS, m_checkBatchExports);
	DDX_Control(pDX, IDC_FILE_RAD, m_btnFile);
	DDX_Control(pDX, IDC_TCP_RAD, m_btnTcp);
	DDX_Control(pDX, IDC_EXPECT_ACK, m_btnExpectAck);
	DDX_Control(pDX, IDC_FILE_IMPORT_RAD, m_btnFileImport);
	DDX_Control(pDX, IDC_TCP_IMPORT_RAD, m_btnTcpImport);
	DDX_Control(pDX, IDC_SEND_ACK, m_btnSendAck);
	DDX_Control(pDX, IDC_USE_MESSAGE_DATE, m_btnUseMsgDate);
	DDX_Control(pDX, IDC_USE_CURRENT_DATE, m_btnUseCurrentDate);
	DDX_Control(pDX, IDC_IMPORT_LOCATIONS, m_btnImportLoc);
	DDX_Control(pDX, IDC_IMPORT_LOCATIONS_AS_POS, m_btnImportLocPOS);
	DDX_Control(pDX, IDC_AUTO_EXPORT, m_btnAutoExport);
	DDX_Control(pDX, IDC_AUTO_UPDATE, m_btnAutoUpdate);
	DDX_Control(pDX, IDC_SERVER_ADDR, m_nxeditServerAddr);
	DDX_Control(pDX, IDC_SERVER_PORT, m_nxeditServerPort);
	DDX_Control(pDX, IDC_SERVER_ADDR_IMPORT, m_nxeditServerAddrImport);
	DDX_Control(pDX, IDC_IMPORT_EXTENSION, m_nxeditImportExtension);
	DDX_Control(pDX, IDC_SERVER_PORT_IMPORT, m_nxeditServerPortImport);
	DDX_Control(pDX, IDC_BEGIN_CHARS, m_nxeditBeginChars);
	DDX_Control(pDX, IDC_END_CHARS, m_nxeditEndChars);
	DDX_Control(pDX, IDC_FILENAME, m_nxeditFilename);
	DDX_Control(pDX, IDC_FILENAME_IMPORT, m_nxeditFilenameImport);
	DDX_Control(pDX, IDC_ADDR_TEXT, m_nxstaticAddrText);
	DDX_Control(pDX, IDC_PORT_TEXT, m_nxstaticPortText);
	DDX_Control(pDX, IDC_ADDR_TEXT_IMPORT, m_nxstaticAddrTextImport);
	DDX_Control(pDX, IDC_EXTENSION_LABEL, m_nxstaticExtensionLabel);
	DDX_Control(pDX, IDC_PORT_TEXT_IMPORT, m_nxstaticPortTextImport);
	DDX_Control(pDX, IDC_FILENAME_TEXT, m_nxstaticFilenameText);
	DDX_Control(pDX, IDC_FILENAME_TEXT_IMPORT, m_nxstaticFilenameTextImport);
	DDX_Control(pDX, IDC_AUTO_EXPORT_APPTS, m_btnAutoExportAppts);
	DDX_Control(pDX, IDC_EXPORT_INSURANCE, m_btnExportInsurance);
	DDX_Control(pDX, IDC_AUTO_EXPORT_EMN_BILLS, m_btnAutoExportEmnBills);
	DDX_Control(pDX, IDC_ADVANCED_LAB_SETTINGS, m_nxbAdvancedLabSettings);
	DDX_Control(pDX, IDC_AUTO_IMPORT_LAB_RESULTS, m_btnAutoImportLabs);
	DDX_Control(pDX, IDC_EXPORT_LAB, m_btnAutoExportLabs);// (a.vengrofski 2010-05-25 10:04) - PLID <38547>
	DDX_Control(pDX, IDC_SENDING_LAB_TEXT, m_nxStaticSendingLab);// (a.vengrofski 2010-05-25 10:04) - PLID <38547>
	DDX_Control(pDX, IDC_CONFIG_INS_COS, m_nxbConfigInsCos);
	DDX_Control(pDX, IDC_HL7_CONFIG_RESOURCES, m_btnConfigureResources);
	DDX_Control(pDX, IDC_CONFIG_INS_RELATIONS, m_nxbConfigureRelations);
	DDX_Control(pDX, IDC_IMPORT_INSURANCE, m_btnImportInsurance);
	DDX_Control(pDX, IDC_SEND_A28_31, m_btnSendA2831);
	DDX_Control(pDX, IDC_AUTO_IMPORT_PATS_FROM_SCHEDULE_MESSAGES, m_btnAutoImportPatsFromSiu);
	DDX_Control(pDX, IDC_EXCLUDE_PROSPECTS, m_btnExcludeProspects);
	DDX_Control(pDX, IDC_HL7_ADV_SCHED_OPTIONS, m_btnAdvSchedSettings);
	DDX_Control(pDX, IDC_CONFIG_INS_TYPES, m_btnConfigInsTypes);// (d.singleton 2012-08-24 16:27) - PLID 52302
	DDX_Control(pDX, IDC_APPT_IMPORT_REF_PHYS, m_btnApptImportRefPhys);
	DDX_Control(pDX, IDC_HL7_SETTINGS_REF_PHYS, m_btnConfigRefPhys);
	DDX_Control(pDX, IDC_CONFIG_CHARGE_CODES, m_btnImportChargeCodes);
	DDX_Control(pDX, IDC_CONFIG_INVENTORY, m_btnConfigInventory); // (b.eyers 2015-06-04) - PLID 66205
	DDX_Control(pDX, IDC_ENABLE_INTELLECHART, m_btnEnableIntelleChart); // (b.eyers 2015-06-10) - PLID 66354
	DDX_Control(pDX, IDC_CHK_LINK_FT_DIAG, m_chkExclusiveFTDiagLink); // (b.savon 2015-12-22 09:45) - PLID 67782
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7SettingsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7SettingsDlg)
	ON_BN_CLICKED(IDC_ADD_HL7_GROUP, OnAddGroup)
	ON_BN_CLICKED(IDC_RENAME_HL7_GROUP, OnRenameGroup)
	ON_BN_CLICKED(IDC_DELETE_HL7_GROUP, OnDeleteGroup)
	ON_BN_CLICKED(IDC_FILE_RAD, OnFileRad)
	ON_BN_CLICKED(IDC_TCP_RAD, OnTcpRad)
	ON_BN_CLICKED(IDC_HL7_SETTINGS_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_FILE_IMPORT_RAD, OnFileImportRad)
	ON_BN_CLICKED(IDC_TCP_IMPORT_RAD, OnTcpImportRad)
	ON_BN_CLICKED(IDC_BROWSE_FILE, OnBrowseFile)
	ON_BN_CLICKED(IDC_BROWSE_FILE_IMPORT, OnBrowseFileImport)
	ON_BN_CLICKED(IDC_EXPECT_ACK, OnExpectAck)
	ON_BN_CLICKED(IDC_USE_MESSAGE_DATE, OnUseMessageDate)
	ON_BN_CLICKED(IDC_USE_CURRENT_DATE, OnUseCurrentDate)
	ON_BN_CLICKED(IDC_AUTO_EXPORT, OnAutoExport)
	ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
	ON_BN_CLICKED(IDC_SEND_ACK, OnSendAck)
	ON_BN_CLICKED(IDC_IMPORT_LOCATIONS, OnImportLocations)
	ON_BN_CLICKED(IDC_CONFIG_LOCATIONS, OnConfigLocations)
	ON_BN_CLICKED(IDC_IMPORT_LOCATIONS_AS_POS, OnImportLocationsAsPos)
	ON_BN_CLICKED(IDC_CHECK_BATCH_EXPORTS, OnCheckBatchExports)
	ON_BN_CLICKED(IDC_CHECK_BATCH_IMPORTS, OnCheckBatchImports)
	ON_BN_CLICKED(IDC_AUTO_EXPORT_APPTS, OnAutoExportAppts)
	ON_BN_CLICKED(IDC_EXPORT_INSURANCE, OnExportInsurance)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUTO_EXPORT_EMN_BILLS, &CHL7SettingsDlg::OnAutoExportEmnBills)
	ON_BN_CLICKED(IDC_ADVANCED_LAB_SETTINGS, &CHL7SettingsDlg::OnAdvancedLabSettings)
	ON_BN_CLICKED(IDC_AUTO_IMPORT_LAB_RESULTS, &CHL7SettingsDlg::OnBnClickedAutoImportLabResults)
	ON_BN_CLICKED(IDC_EXPORT_LAB, &CHL7SettingsDlg::OnBnClickedExportLab)
	ON_BN_CLICKED(IDC_CONFIG_INS_COS, &CHL7SettingsDlg::OnConfigInsCos)
	ON_BN_CLICKED(IDC_HL7_CONFIG_RESOURCES, &CHL7SettingsDlg::OnBnClickedHl7ConfigResources)
	ON_BN_CLICKED(IDC_CONFIG_INS_RELATIONS, &CHL7SettingsDlg::OnConfigInsRelations)
	ON_BN_CLICKED(IDC_IMPORT_INSURANCE, &CHL7SettingsDlg::OnImportInsurance)
	ON_BN_CLICKED(IDC_SEND_A28_31, &CHL7SettingsDlg::OnSendA2831)
	ON_BN_CLICKED(IDC_AUTO_IMPORT_PATS_FROM_SCHEDULE_MESSAGES, &CHL7SettingsDlg::OnBnClickedAutoImportPatsFromScheduleMessages)
	ON_BN_CLICKED(IDC_EXCLUDE_PROSPECTS, &CHL7SettingsDlg::OnExcludeProspects)
	ON_BN_CLICKED(IDC_ADVANCED_PATIENT_SETTINGS, &CHL7SettingsDlg::OnBnClickedAdvancedPatientSettings)
	ON_BN_CLICKED(IDC_HL7_ADV_SCHED_OPTIONS, &CHL7SettingsDlg::OnBnClickedHl7AdvSchedOptions)
	ON_BN_CLICKED(IDC_ADVANCED_GENERAL_SETTINGS, &CHL7SettingsDlg::OnAdvancedGeneralSettings)
	ON_BN_CLICKED(IDC_CONFIG_INS_TYPES, &CHL7SettingsDlg::OnBnClickedConfigInsTypes)
	ON_BN_CLICKED(IDC_HL7_SETTINGS_REF_PHYS, &CHL7SettingsDlg::OnBnClickedHl7SettingsRefPhys)
	ON_BN_CLICKED(IDC_APPT_IMPORT_REF_PHYS, &CHL7SettingsDlg::OnBnClickedApptImportRefPhys)
	ON_BN_CLICKED(IDC_AUTO_EXPORT_REF_PHYS, &CHL7SettingsDlg::OnAutoExportRefPhys)
	ON_BN_CLICKED(IDC_AUTO_UPDATE_REF_PHYS, &CHL7SettingsDlg::OnAutoUpdateRefPhys)
	ON_BN_CLICKED(IDC_CONFIG_CHARGE_CODES, &CHL7SettingsDlg::OnBnClickedConfigChargeCodes)
	ON_BN_CLICKED(IDC_CONFIG_INVENTORY, &CHL7SettingsDlg::OnBnClickedConfigInventory) // (b.eyers 2015-06-04) - PLID 66205
	ON_BN_CLICKED(IDC_ENABLE_INTELLECHART, &CHL7SettingsDlg::OnEnableIntelleChart) // (b.eyers 2015-06-10) - PLID 66354
	ON_BN_CLICKED(IDC_AUTO_EXPORT_INSURANCE, &CHL7SettingsDlg::OnBnClickedAutoExportInsurance) // (r.goldschmidt 2015-11-05 18:24) - PLID 67517
	ON_BN_CLICKED(IDC_CHK_LINK_FT_DIAG, &CHL7SettingsDlg::OnBnClickedChkLinkFtDiag)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7SettingsDlg message handlers

BOOL CHL7SettingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try { // (c.haag 2006-11-20 15:49) - PLID 23598 - Added try/catch
		m_pList = BindNxDataListCtrl(IDC_HL7_FORMAT_LIST);

		m_pLabProcedureList = BindNxDataList2Ctrl(IDC_DEFAULT_LAB_PROCEDURE);
		m_pSendingLabList = BindNxDataList2Ctrl(IDC_DEFAULT_SENDING_LAB, true);// (a.vengrofski 2010-05-12 18:01) - PLID <38547>

		// (a.vengrofski 2010-05-13 09:43) - PLID <38547> - Add a No Link Row.
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pSendingLabList->GetNewRow();
		pRow->PutValue(ehlID, (long) -1);
		pRow->PutValue(ehlName, _variant_t("{No Link}"));
		m_pSendingLabList->AddRowAtEnd(pRow, NULL);

		// (a.vengrofski 2010-05-27 16:55) - PLID <38918> - Added for auditing
		m_nDefaultLabID = -1;

		// (j.jones 2008-05-08 10:12) - PLID 29953 - added nxiconbuttons for modernization
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRename.AutoSet(NXB_MODIFY);
		m_btnConfigLocations.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_nxbConfigInsCos.AutoSet(NXB_MODIFY);
		m_btnConfigureResources.AutoSet(NXB_MODIFY);
		m_btnConfigInsTypes.AutoSet(NXB_MODIFY);
		m_nxbConfigureRelations.AutoSet(NXB_MODIFY);
		m_btnImportChargeCodes.AutoSet(NXB_MODIFY);
		m_btnConfigInventory.AutoSet(NXB_MODIFY); // (b.eyers 2015-06-04) - PLID 66205

		// (j.dinatale 2013-01-23 14:53) - PLID 54631
		m_btnConfigRefPhys.AutoSet(NXB_MODIFY);

		// (v.maida 2016-06-14 10:35) - NX-100833 - Only allow the NexTech Technical Support user to configure HL7 import/export paths in RemoteApp.
		if (g_pLicense->GetAzureRemoteApp() && GetCurrentUserID() != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			EnableTechSupportControls(FALSE);
		}
		else {
			EnableTechSupportControls(TRUE);
		}

		((CEdit*)GetDlgItem(IDC_HL7_FILE_PREFIX))->SetLimitText(25);

		//set selection to the first item in the list
		m_pList->PutCurSel(0);

		long nCurSel = m_pList->GetCurSel();
		if(nCurSel != -1)
			LoadSettings(VarLong(m_pList->GetValue(nCurSel, 0)));
		else
			EnsureConnectionOptions();

		//TES 2/23/2010 - PLID 37503 - Set up our sub-tabs for different record types.
		m_tab = GetDlgItemUnknown(IDC_HL7_SETTINGS_TABS);
		if (m_tab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDC_HL7_SETTINGS_CLOSE);
			return 0;
		}
		else {
			//TES 2/23/2010 - PLID 37503 - Create a tab for each record type that we deal with.
			// (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physicians tab.
			m_tab->PutSize(5);
			m_tab->PutTabWidth(6);
			m_tab->PutLabel(0, "Patients");
			m_tab->PutLabel(1, "Appts.");
			m_tab->PutLabel(2, "Bills");
			m_tab->PutLabel(3, "Labs");
			m_tab->PutLabel(4, "Ref. Phys.");
		}
		m_tab->CurSel = 0;
		//TES 2/23/2010 - PLID 37503 - Now show the correct controls for the currently selected tab.
		ReflectCurrentTab();
	}
	NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7SettingsDlg::OnOK() 
{	
}

void CHL7SettingsDlg::OnCancel() 
{
	//TES 12/4/2006 - PLID 23737 - I moved the RefreshTable()s so they would only be called if something's actually changed.
	/*try { // (c.haag 2006-11-20 15:49) - PLID 23598 - Added try/catch
	CClient::RefreshTable(NetUtils::HL7SettingsT);
	}
	NxCatchAll("Error in OnCancel");*/
}

void CHL7SettingsDlg::OnAddGroup() 
{
	try {
		CString strValue;
		if(InputBoxNonEmpty(this, "Enter a group name:", strValue, 255) != IDOK)
			return;

		// (z.manning 2010-05-21 10:44) - PLID 38638 - Default the lab import matching fields flag to all
		// 5 of the original possible matching fields.
		//DWORD dwLabImportMatchingFieldFlags = limfPatientID|limfPatientName|limfFormNumber|limfSSN|limfBirthDate;

		//save it
		long nNewID = NewNumber("HL7SettingsT", "ID");
		//TES 6/22/2011 - PLID 44261 - HL7SettingsT just has ID and Name now
		ExecuteSql("INSERT INTO HL7SettingsT (ID, Name) values (%li, '%s')"
			, nNewID, _Q(strValue));

		//add to the list
		IRowSettingsPtr pRow;
		pRow = m_pList->GetRow(-1);
		pRow->PutValue(0, (long)nNewID);
		pRow->PutValue(1, _bstr_t(strValue));

		long nSel = m_pList->AddRow(pRow);

		if(nSel != -1)
			m_pList->PutCurSel(nSel);

		//load an empty dialog
		LoadSettings(-1);

		// (z.manning 2011-10-03 10:40) - PLID 45724 - I wanted the biopsy type option to default to false for existing
		// HL7 groups, hence its false value in CHL7SettingsCache::InitDefaults. However, I want it to default to true
		// for new groups so let's set it as such here.
		CString strSql;
		CNxParamSqlArray aryParams;
		SetHL7SettingBit(nNewID, "BiopsyTypeOBR13", TRUE, &strSql, &aryParams);

        // (j.kuziel 2011-10-14 16:54) - PLID 41419 - This should default to true for new setting groups, but leave it
        //  false for existing groups to avoid breaking everything.
        SetHL7SettingBit(nNewID, "SendIN1BeforeGT1OnORM", TRUE, &strSql, &aryParams);

		// (r.gonet 02/28/2012) - PLID 48044 - By default, we should send NTE segments before DG1 in the ORDER_DETAILS group,
		//  because that is correct according to the standard. But leave it the wrong way for legacy groups in case they 
		//  depend on the order being wrong.
		SetHL7SettingBit(nNewID, "SendNTEBeforeDG1", TRUE, &strSql, &aryParams);
        ExecuteParamSqlBatch(GetRemoteData(), strSql, aryParams);

		// (j.jones 2008-05-19 14:57) - PLID 30107 - RefreshGroup will send a tablechecker
		RefreshGroup(nNewID);

	} NxCatchAll("Error in OnAddGroup()");
}

void CHL7SettingsDlg::OnRenameGroup() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MsgBox("You must have a valid selection before renaming a group.");
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		CString strValue;
		if(InputBoxNonEmpty(this, "Enter a new group name:", strValue, 255) != IDOK)
			return;

		//save it in data
		ExecuteSql("UPDATE HL7SettingsT SET Name = '%s' WHERE ID = %li", _Q(strValue), nID);
		RefreshGroup(nID);

		//change the name in the datalist
		IRowSettingsPtr pRow = m_pList->GetRow(nCurSel);
		pRow->PutValue(1, _bstr_t(strValue));

	} NxCatchAll("Error in OnRenameGroup()");
}

void CHL7SettingsDlg::OnDeleteGroup() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MsgBox("You must have a valid selection before deleting a group.");
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 11/3/2008 - PLID 31637 - First, make sure no labs reference any messages in this group, if so we can't
		// delete it.
		if(ReturnsRecords("SELECT TOP 1 ID FROM HL7MessageQueueT INNER JOIN LabResultsT ON LabResultsT.HL7MessageID = "
			"HL7MessageQueueT.ID WHERE HL7MessageQueueT.GroupID = %li", nID)) {
				MsgBox("You have imported Lab Results from this HL7 Group.  You cannot delete HL7 Groups which are associated with Lab Results.");
				return;
		}

		// (s.tullis 2015-11-05 15:59) - PLID 67263 - Cannot delete a group with optical perscriptions
		if (ReturnsRecords("SELECT TOP 1 HL7MessageQueueT.ID FROM HL7MessageQueueT INNER JOIN LensRxT ON LensRxT.HL7MessageID = "
			"HL7MessageQueueT.ID WHERE HL7MessageQueueT.GroupID = %li", nID)) {
			MsgBox("You have imported optical prescriptions from this HL7 Group.  You cannot delete HL7 Groups which are associated with optical prescriptions.");
			return;
		}

		if(AfxMessageBox("Are you absolutely sure you wish to delete this group?", MB_YESNO) == IDNO) 
			return;

		// (j.jones 2008-05-05 12:09) - PLID 29598 - we need to delete from multiple tables,
		// so do not delete until all prompts have been given

		BOOL bDeleteFromQueue = FALSE;
		BOOL bDeleteFromLog = FALSE;
		BOOL bDeleteFromLink = FALSE;

		//check if anything exists in the message queue ... we'll have to delete those if we want rid of this
		if(ReturnsRecords("SELECT TOP 1 ID FROM HL7MessageQueueT WHERE GroupID = %li", nID)) {
			if(MsgBox(MB_YESNO, "You have incoming messages waiting for this group.  If you continue with the group deletion, these messages will be lost.\r\n"
				"Are you absolutely sure you wish to do this?") == IDNO) {
					return;
			}

			//don't delete yet, just flag as needing deleted
			bDeleteFromQueue = TRUE;
		}

		// (j.jones 2008-05-05 12:12) - PLID 29598 - check if anything exists in the message log
		if(ReturnsRecords("SELECT TOP 1 MessageID FROM HL7MessageLogT WHERE GroupID = %li", nID)) {
			if(MsgBox(MB_YESNO, "You have exported messages for this group. If you continue with the group deletion, these messages will be lost and unrecoverable.\r\n"
				"Are you absolutely sure you wish to do this?") == IDNO) {
					return;
			}

			//don't delete yet, just flag as needing deleted
			bDeleteFromLog = TRUE;
		}

		//make sure nothing exists in IDLinkT
		if(ReturnsRecords("SELECT TOP 1 ID FROM HL7IDLinkT WHERE GroupID = %li", nID)) {
			if(MsgBox(MB_YESNO, "Patients have been imported from this group.  If you delete it, those patients will lose their link to the "
				"software they originated from.\r\n"
				"Are you absolutely sure you wish to do this?") == IDNO)
				return;

			//don't delete yet, just flag as needing deleted
			bDeleteFromLink = TRUE;
		}

		//batch only the necessary statements
		CString strSqlBatch;

		if(bDeleteFromQueue) {
			// (r.gonet 05/01/2014) - PLID 61842 - HL7 Transactions may be associated with the message,
			// so disassociate them.
			// (b.savon 2014-12-12 15:15) - PLID 64447 - Fix table name
			AddStatementToSqlBatch(strSqlBatch,
				"UPDATE HL7TransactionT SET ImportMessageID = NULL WHERE ImportMessageID IN "
				"( "
				"	SELECT HL7MessageQueueT.ID "
				"	FROM HL7MessageQueueT "
				"	WHERE HL7MessageQueueT.GroupID = %li "
				") ", nID);
			AddStatementToSqlBatch(strSqlBatch, 
				"DELETE FROM HL7MessageQueueT WHERE GroupID = %li", nID);
		}
		if(bDeleteFromLog) {
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7MessageLogT WHERE GroupID = %li", nID);
		}
		if(bDeleteFromLink) {
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7IDLinkT WHERE GroupID = %li", nID);
		}

		//TES 7/20/2007 - PLID 26761 - We also need to clear out HL7LocationLinkT
		//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7CodeLinkT WHERE HL7GroupID = %li", nID);

		// (r.gonet 10/31/2011) - PLID 45367 - Delete facility code overrides
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7OverrideFacilityCodesT WHERE HL7GroupID = %li", nID);

		//TES 9/20/2011 - PLID 44261 - Need to delete from HL7GenericSettingsT
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7GenericSettingsT WHERE HL7GroupID = %li", nID);
		//now delete the group
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7SettingsT WHERE ID = %li", nID);

		ExecuteSqlBatch(strSqlBatch);

		RefreshGroup(nID);

		//remove from the list
		m_pList->RemoveRow(nCurSel);

		//set selection to the first one
		m_pList->PutCurSel(0);

		OnSelChosenHl7FormatList(m_pList->GetCurSel());
	} NxCatchAll("Error in OnDeleteGroup()");
}

//loads the dialog with the given ID.
//Pass -1 if everything is to be cleared
void CHL7SettingsDlg::LoadSettings(long nGroupID)
{
	try { // (c.haag 2006-11-20 15:50) - PLID 23598 - Added try/catch
		//TES 5/20/2010 - PLID 38810 - Some settings have been moved to CHL7LabSettingsDlg
		CString strName, strAddr, strPort, strFile, strBeginChar, strEndChar;
		CString strImportAddr, strImportPort, strImportFile;
		CString strImportExtension = "hl7";
		BOOL bExpectAck = TRUE, bUseBillDate = TRUE, bExportNewPatients = FALSE, bSendAck = TRUE, bUseImportedLocation = FALSE, bUseImportedLocationAsPOS = FALSE, bExportUpdatedPatients = FALSE, bExportAppts = FALSE, bExportInsurance = FALSE, bExportEmnBills = FALSE;
		// (j.fouts 2013-05-30 15:29) - PLID 56800 - Batch imports defaults to FALSE
		BOOL bBatchImports = FALSE, bBatchExports = FALSE;
		BOOL bAutoImportLabs = TRUE;
		BOOL bAutoExportLabs = FALSE;// (a.vengrofski 2010-05-25 10:42) - PLID <38547>
		BOOL bAutoExportNewRefPhys = FALSE; // (v.maida 2014-12-23 12:19) - PLID 64472 - Added automatic referring physician exporting.
		BOOL bAutoExportUpdatedRefPhys = FALSE; // (v.maida 2014-12-23 12:19) - PLID 64472 - Added automatic referring physician exporting.
		// (j.fouts 2013-05-30 15:29) - PLID 56800 - Type defaults to 0
		long nType = 0;
		// (j.fouts 2013-05-30 15:29) - PLID 56800 - ImportType defaults to 0
		long nImportType = 0;
		long nDefaultLabProcedureID = -1;
		m_nDefaultLabID = -1;// (a.vengrofski 2010-06-11 13:42) - PLID <38547>
		BOOL bImportInsurance = TRUE; //TES 7/19/2010 - PLID 39720
		BOOL bSendA2831 = FALSE; //TES 8/2/2010 - PLID 39935
		BOOL bAutoImportPatsFromSiu = FALSE; // (z.manning 2010-08-09 10:07) - PLID 39985
		BOOL bExcludeProspects = FALSE; //TES 9/20/2010 - PLID 40595
		CString strFilenamePrefix; // (z.manning 2010-10-01 15:32) - PLID 40786
		BOOL bUseOBX3_5 = FALSE; //TES 4/25/2011 - PLID 43423
		BOOL bImportApptRefPhys = FALSE; // (j.dinatale 2013-01-15 16:32) - PLID 54631
		BOOL bEnableIntelleChart = FALSE; // (b.eyers 2015-06-10) - PLID 66354
		BOOL bAutoExportInsurance = FALSE; // (r.goldschmidt 2015-11-05 18:11) - PLID 67517
		BOOL bExclusivelyLinkFTandDG = FALSE; // (b.savon 2015-12-22 09:48) - PLID 67782

		if(nGroupID != -1) {
			//TES 12/4/2006 - PLID 23737 - lookup the fields from the global cache
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			strAddr = GetHL7SettingText(nGroupID, "ExportAddr");
			strPort = GetHL7SettingText(nGroupID, "ExportPort");
			strFile = GetHL7SettingText(nGroupID, "ExportFile");
			nType = GetHL7SettingInt(nGroupID, "ExportType");
			strBeginChar = GetHL7SettingText(nGroupID, "ExportBeginChars");
			strEndChar = GetHL7SettingText(nGroupID, "ExportEndChars");

			strImportAddr = GetHL7SettingText(nGroupID, "ImportAddr");
			strImportPort = GetHL7SettingText(nGroupID, "ImportPort");
			strImportFile = GetHL7SettingText(nGroupID, "ImportFile");
			strImportExtension = GetHL7SettingText(nGroupID, "ImportExtension");
			nImportType = GetHL7SettingInt(nGroupID, "ImportType");
			bExpectAck = GetHL7SettingBit(nGroupID, "ExpectACK");
			bUseBillDate = GetHL7SettingBit(nGroupID, "UseBillDate");
			bExportNewPatients = GetHL7SettingBit(nGroupID, "ExportNewPatients");
			bSendAck = GetHL7SettingBit(nGroupID, "SendACK");
			bUseImportedLocation = GetHL7SettingBit(nGroupID, "UseImportedLocation");
			bUseImportedLocationAsPOS = GetHL7SettingBit(nGroupID, "UseImportedLocationAsPOS");

			// (j.gruber 2007-08-23 17:34) - PLID 24628 - Auto Update
			bExportUpdatedPatients = GetHL7SettingBit(nGroupID, "ExportUpdatedPatients");

			// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
			bBatchImports = GetHL7SettingBit(nGroupID, "BatchImports");
			bBatchExports = GetHL7SettingBit(nGroupID, "BatchExports");

			// (z.manning 2008-07-16 09:55) - PLID 30753 - Export appts option
			bExportAppts = GetHL7SettingBit(nGroupID, "ExportAppts");

			//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
			bExportEmnBills = GetHL7SettingBit(nGroupID, "ExportEmnBills");

			//TES 10/10/2008 - PLID 21093 - Default Lab Procedure
			nDefaultLabProcedureID = GetHL7SettingInt(nGroupID, "DefaultLabProcedureID");

			// (z.manning 2008-12-22 16:35) - PLID 32250 - Export insurance option
			bExportInsurance = GetHL7SettingBit(nGroupID, "ExportInsurance");

			// (z.manning 2010-05-21 10:56) - PLID 38638
			bAutoImportLabs = GetHL7SettingBit(nGroupID, "AutoImportLabs");

			// (a.vengrofski 2010-05-25 10:42) - PLID <38547> - Auto Export Labs
			bAutoExportLabs = GetHL7SettingBit(nGroupID, "AutoExportLabs");

			// (a.vengrofski 2010-05-25 10:42) - PLID <38547> - Auto Export Labs
			m_nDefaultLabID = GetHL7SettingInt(nGroupID, "DefaultLabID");

			//TES 7/19/2010 - PLID 39720
			bImportInsurance = GetHL7SettingBit(nGroupID, "ImportInsurance");

			//TES 8/2/2010 - PLID 39935
			bSendA2831 = GetHL7SettingBit(nGroupID, "SendA2831");

			// (z.manning 2010-08-09 10:07) - PLID 39985
			bAutoImportPatsFromSiu = GetHL7SettingBit(nGroupID, "AutoCreatePatientsFromScheduleMessages");

			//TES 9/20/2010 - PLID 40595
			bExcludeProspects = GetHL7SettingBit(nGroupID, "ExcludeProspects");

			// (z.manning 2010-10-01 15:32) - PLID 40654
			strFilenamePrefix = GetHL7SettingText(nGroupID, "FilenamePrefix");

			//TES 9/15/2011 - PLID 45523 - Moved SendingFacilityID and ForceFacilityIDMatch to CHL7GeneralSettingsDlg

			//TES 4/25/2011 - PLID 43423
			bUseOBX3_5 = GetHL7SettingBit(nGroupID, "UseOBX3_5");

			// (j.dinatale 2013-01-15 16:32) - PLID 54631
			bImportApptRefPhys = GetHL7SettingBit(nGroupID, "IMPORTAPPTREFPHYS");

			// (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physicians tab / flag.
			bAutoExportNewRefPhys = GetHL7SettingBit(nGroupID, "EXPORTNEWREFPHYS");
			bAutoExportUpdatedRefPhys = GetHL7SettingBit(nGroupID, "EXPORTUPDATEDREFPHYS");

			bEnableIntelleChart = GetHL7SettingBit(nGroupID, "EnableIntelleChart"); // (b.eyers 2015-06-10) - PLID 66354
			
			bAutoExportInsurance = GetHL7SettingBit(nGroupID, "AutoExportInsurance"); // (r.goldschmidt 2015-11-05 18:21) - PLID 67517

			bExclusivelyLinkFTandDG = GetHL7SettingBit(nGroupID, "ExclusivelyLinkFTandDG"); // (b.savon 2015-12-22 09:48) - PLID 67782

		}

		//load all the fields
		SetDlgItemText(IDC_SERVER_ADDR,		strAddr);
		SetDlgItemText(IDC_SERVER_PORT,		strPort);
		SetDlgItemText(IDC_FILENAME,		strFile);
		SetDlgItemText(IDC_BEGIN_CHARS,		strBeginChar);
		SetDlgItemText(IDC_END_CHARS,		strEndChar);
		SetDlgItemText(IDC_SERVER_ADDR_IMPORT,		strImportAddr);
		SetDlgItemText(IDC_SERVER_PORT_IMPORT,		strImportPort);
		SetDlgItemText(IDC_FILENAME_IMPORT,		strImportFile);
		SetDlgItemText(IDC_IMPORT_EXTENSION,	strImportExtension);
		// (z.manning 2010-10-01 17:36) - PLID 40654
		SetDlgItemText(IDC_HL7_FILE_PREFIX, strFilenamePrefix);

		//load the type
		BOOL bFile = TRUE;
		if(nType) {
			//File export = 0, TCP export = 1
			bFile = FALSE;
		}

		CheckDlgButton(IDC_FILE_RAD, bFile);
		CheckDlgButton(IDC_TCP_RAD, !bFile);

		bFile = TRUE;
		if(nImportType) {
			bFile = FALSE;
		}
		CheckDlgButton(IDC_FILE_IMPORT_RAD, bFile);
		CheckDlgButton(IDC_TCP_IMPORT_RAD, !bFile);

		CheckDlgButton(IDC_EXPECT_ACK, bExpectAck);

		CheckRadioButton(IDC_USE_MESSAGE_DATE, IDC_USE_CURRENT_DATE, bUseBillDate?IDC_USE_MESSAGE_DATE:IDC_USE_CURRENT_DATE);

		CheckDlgButton(IDC_AUTO_EXPORT, bExportNewPatients);

		CheckDlgButton(IDC_SEND_ACK, bSendAck);

		CheckDlgButton(IDC_IMPORT_LOCATIONS, bUseImportedLocation);
		CheckDlgButton(IDC_IMPORT_LOCATIONS_AS_POS, bUseImportedLocationAsPOS);
		GetDlgItem(IDC_CONFIG_LOCATIONS)->EnableWindow(bUseImportedLocation||bUseImportedLocationAsPOS);

		// (j.gruber 2007-08-23 17:35) - PLID 24628 - Auto Update
		CheckDlgButton(IDC_AUTO_UPDATE, bExportUpdatedPatients);

		// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
		CheckDlgButton(IDC_CHECK_BATCH_IMPORTS, bBatchImports);
		CheckDlgButton(IDC_CHECK_BATCH_EXPORTS, bBatchExports);

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physicians tab / flag.
		CheckDlgButton(IDC_AUTO_EXPORT_REF_PHYS, bAutoExportNewRefPhys);
		CheckDlgButton(IDC_AUTO_UPDATE_REF_PHYS, bAutoExportUpdatedRefPhys);

		CheckDlgButton(IDC_ENABLE_INTELLECHART, bEnableIntelleChart); // (b.eyers 2015-06-10) - PLID 66354

		CheckDlgButton(IDC_AUTO_EXPORT_INSURANCE, bAutoExportInsurance); // (r.goldschmidt 2015-11-05 18:11) - PLID 67517

		// (z.manning 2008-07-16 09:57) - PLID 30753
		m_btnAutoExportAppts.SetCheck(bExportAppts ? BST_CHECKED : BST_UNCHECKED);

		//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
		m_btnAutoExportEmnBills.SetCheck(bExportEmnBills ? BST_CHECKED : BST_UNCHECKED);

		//TES 10/10/2008 - PLID 21093 - Default Lab Procedure
		m_pLabProcedureList->SetSelByColumn(0, nDefaultLabProcedureID);

		// (z.manning 2008-12-22 16:34) - PLID 32550 - Export insurance option
		m_btnExportInsurance.SetCheck(bExportInsurance ? BST_CHECKED : BST_UNCHECKED);

		// (z.manning 2010-05-21 10:57) - PLID 38638
		m_btnAutoImportLabs.SetCheck(bAutoImportLabs ? BST_CHECKED : BST_UNCHECKED);

		// (a.vengrofski 2010-05-25 10:40) - PLID <38547> - Auto Export Lab Requests
		m_btnAutoExportLabs.SetCheck(bAutoExportLabs ? BST_CHECKED : BST_UNCHECKED);

		// (a.vengrofski 2010-05-25 10:40) - PLID <38547> - Auto Export Lab Requests
		m_pSendingLabList->SetSelByColumn(ehlID, m_nDefaultLabID);

		//TES 7/19/2010 - PLID 39720 - Import insurance option
		m_btnImportInsurance.SetCheck(bImportInsurance ? BST_CHECKED : BST_UNCHECKED);

		//TES 8/2/2010 - PLID 39935 - Option to send A28/A31 messagse
		m_btnSendA2831.SetCheck(bSendA2831 ? BST_CHECKED : BST_UNCHECKED);

		m_btnAutoImportPatsFromSiu.SetCheck(bAutoImportPatsFromSiu ? BST_CHECKED : BST_UNCHECKED);

		//TES 9/20/2010 - PLID 40595 - Option to exclude prospects when exporting patients
		m_btnExcludeProspects.SetCheck(bExcludeProspects?BST_CHECKED:BST_UNCHECKED);

		// (j.dinatale 2013-01-15 16:32) - PLID 54631
		m_btnApptImportRefPhys.SetCheck(bImportApptRefPhys?BST_CHECKED:BST_UNCHECKED);

		// (b.savon 2015-12-22 09:49) - PLID 67782
		m_chkExclusiveFTDiagLink.SetCheck(bExclusivelyLinkFTandDG == FALSE ? BST_UNCHECKED : BST_CHECKED);

		// (b.spivey, February 3, 2016) - PLID 68169 - If IntelleChart setings are on, we force "SendAck" to true and disable the checkbox.
		if (m_btnEnableIntelleChart.GetCheck()) {
			SetHL7SettingBit(nGroupID, "SendAck", TRUE);
			m_btnSendAck.SetCheck(TRUE); 
			m_btnSendAck.EnableWindow(FALSE); 
		}
		else {
			m_btnSendAck.EnableWindow(TRUE);
		}

		//ensure the connection items are shown/hidden properly
		EnsureConnectionOptions();
	}
	NxCatchAll("Error in LoadSettings");
}

//saves the given item
void CHL7SettingsDlg::Save(int nID)
{
	try {
		CString strField, strValue;

		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1)
			return;

		long nGroupID = VarLong(m_pList->GetValue(nCurSel, 0));

		//get the data to save
		GetDlgItemText(nID, strValue);

		//find the field to save
		switch(nID) {
		case IDC_SERVER_ADDR:
			strField = "ExportAddr";
			break;
		case IDC_SERVER_PORT:
			strField = "ExportPort";
			break;
		case IDC_FILENAME:
			strField = "ExportFile";
			break;
		case IDC_BEGIN_CHARS:
			strField = "ExportBeginChars";
			break;
		case IDC_END_CHARS:
			strField = "ExportEndChars";
			break;
		case IDC_SERVER_ADDR_IMPORT:
			strField = "ImportAddr";
			break;
		case IDC_SERVER_PORT_IMPORT:
			strField = "ImportPort";
			break;
		case IDC_FILENAME_IMPORT:
			strField = "ImportFile";
			break;
		case IDC_IMPORT_EXTENSION:
			strField = "ImportExtension";
			break;
		case IDC_HL7_FILE_PREFIX: // (z.manning 2010-10-01 15:48) - PLID 40786
			strField = "FilenamePrefix";
			break;
		default:
			//unhandled case, don't try to save
			return;
		}

		//now do the update
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingText(nGroupID, strField, strValue);
		RefreshGroup(nGroupID);

	} NxCatchAll("Error in Save()");
}

#define HIDE_SETTINGS_WND(idc, bShow)	{	int sw; if(bShow) sw = SW_SHOW; else sw = SW_HIDE; CWnd* pWnd = GetDlgItem(idc);  pWnd->ShowWindow(sw);	}

//shows / hides the connection options depending what is checked
void CHL7SettingsDlg::EnsureConnectionOptions()
{
	try { // (c.haag 2006-11-20 15:50) - PLID 23598 - Added try/catch
		BOOL bShow = FALSE;
		if(!IsDlgButtonChecked(IDC_TCP_RAD)) 
			bShow = TRUE;

		//ensure the file options
		HIDE_SETTINGS_WND(IDC_FILENAME, bShow);
		HIDE_SETTINGS_WND(IDC_FILENAME_TEXT, bShow);
		HIDE_SETTINGS_WND(IDC_BROWSE_FILE, bShow);
		HIDE_SETTINGS_WND(IDC_HL7_FILE_PREFIX, bShow);
		// (z.manning 2010-10-01 16:04) - PLID 40654
		HIDE_SETTINGS_WND(IDC_FILE_PREFIX_LABEL, bShow);

		//and the server options
		HIDE_SETTINGS_WND(IDC_SERVER_ADDR, !bShow);
		HIDE_SETTINGS_WND(IDC_SERVER_PORT, !bShow);
		HIDE_SETTINGS_WND(IDC_ADDR_TEXT, !bShow);
		HIDE_SETTINGS_WND(IDC_PORT_TEXT, !bShow);
		HIDE_SETTINGS_WND(IDC_EXPECT_ACK, !bShow);

		//Again for the import
		bShow = FALSE;
		if(!IsDlgButtonChecked(IDC_TCP_IMPORT_RAD)) 
			bShow = TRUE;

		//ensure the file options
		HIDE_SETTINGS_WND(IDC_FILENAME_IMPORT, bShow);
		HIDE_SETTINGS_WND(IDC_FILENAME_TEXT_IMPORT, bShow);
		HIDE_SETTINGS_WND(IDC_BROWSE_FILE_IMPORT, bShow);
		HIDE_SETTINGS_WND(IDC_IMPORT_EXTENSION, bShow);
		HIDE_SETTINGS_WND(IDC_EXTENSION_LABEL, bShow);

		//and the server options
		HIDE_SETTINGS_WND(IDC_SERVER_ADDR_IMPORT, !bShow);
		HIDE_SETTINGS_WND(IDC_SERVER_PORT_IMPORT, !bShow);
		HIDE_SETTINGS_WND(IDC_ADDR_TEXT_IMPORT, !bShow);
		HIDE_SETTINGS_WND(IDC_PORT_TEXT_IMPORT, !bShow);
	}
	NxCatchAll("Error in EnsureConnectionOptions");
}

void CHL7SettingsDlg::OnFileRad() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1)
			return;

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//save it
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingInt(nID, "ExportType", 0);
		RefreshGroup(nID);

	} NxCatchAll("Error in OnFileRad()");

	EnsureConnectionOptions();
}

void CHL7SettingsDlg::OnTcpRad() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1)
			return;

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//save it
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingInt(nID, "ExportType", 1);
		RefreshGroup(nID);

	} NxCatchAll("Error in OnTcpRad()");

	EnsureConnectionOptions();
}

void CHL7SettingsDlg::OnClose() 
{
	//TES 12/4/2006 - PLID 23737 - I moved the RefreshTable()s so they would only be called if something's actually changed.
	/*try { // (c.haag 2006-11-20 15:51) - PLID 23598 - Added try/catch
	CClient::RefreshTable(NetUtils::HL7SettingsT);
	} NxCatchAll("Error in OnClose");*/
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CHL7SettingsDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CHL7SettingsDlg)
	ON_EVENT(CHL7SettingsDlg, IDC_HL7_FORMAT_LIST, 16 /* SelChosen */, OnSelChosenHl7FormatList, VTS_I4)
	ON_EVENT(CHL7SettingsDlg, IDC_HL7_FORMAT_LIST, 1 /* SelChanging */, OnSelChangingHl7FormatList, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CHL7SettingsDlg, IDC_DEFAULT_LAB_PROCEDURE, 16, CHL7SettingsDlg::OnSelChosenDefaultLabProcedure, VTS_DISPATCH)
	ON_EVENT(CHL7SettingsDlg, IDC_HL7_SETTINGS_TABS, 1, CHL7SettingsDlg::OnSelectTabHl7SettingsTabs, VTS_I2 VTS_I2)
	ON_EVENT(CHL7SettingsDlg, IDC_DEFAULT_SENDING_LAB, 16, CHL7SettingsDlg::SelChosenDefaultSendingLab, VTS_DISPATCH)
	ON_EVENT(CHL7SettingsDlg, IDC_DEFAULT_SENDING_LAB, 18, CHL7SettingsDlg::RequeryFinishedDefaultSendingLab, VTS_I2)
END_EVENTSINK_MAP()

void CHL7SettingsDlg::OnSelChosenHl7FormatList(long nRow) 
{
	try { // (c.haag 2006-11-20 15:51) - PLID 23598 - Added try/catch
		long nID = -1;

		if(nRow != -1) 
			nID = VarLong(m_pList->GetValue(nRow, 0));

		LoadSettings(nID);
	}
	NxCatchAll("Error in OnSelChosenHL7FormatList");
}

void CHL7SettingsDlg::OnSelChangingHl7FormatList(long FAR* nNewSel) 
{
}

BOOL CHL7SettingsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(HIWORD(wParam)) {
	case EN_KILLFOCUS:
		Save(LOWORD(wParam));
		break;

	default:
		break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CHL7SettingsDlg::OnFileImportRad() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1)
			return;

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//save it
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingInt(nID, "ImportType", 0);
		RefreshGroup(nID);

	} NxCatchAll("Error in OnFileImportRad()");

	EnsureConnectionOptions();
}

void CHL7SettingsDlg::OnTcpImportRad() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1)
			return;

		//TES 7/31/2008 - PLID 29690 - If they're auto-importing, via TCP/IP, and don't have a default location, it won't
		// work.
		if(!IsDlgButtonChecked(IDC_CHECK_BATCH_IMPORTS)) {
			if(!ReturnsRecords("SELECT ID FROM LocationsT WHERE IsDefault = 1")) {
				MsgBox("NOTE: You have chosen to automatically import new patients via TCP/IP, but you do not have a default location for "
					"new patients to be assigned to.  Please select a default location (by going to the Locations tab in the "
					"Administrator module), all incoming patients will be batched until then.");
			}
		}
		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//save it
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingInt(nID, "ImportType", 1);
		RefreshGroup(nID);

	} NxCatchAll("Error in OnTcpImportRad()");

	EnsureConnectionOptions();
}

void CHL7SettingsDlg::OnBrowseFile() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		// (j.armen 2011-10-25 14:56) - PLID 46137 - HL7 Paths should be based on the practice path
		CString strExportPath = GetHL7SettingText(nID, "ExportFile");
		if (strExportPath.IsEmpty()) {
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
			strExportPath = GetEnvironmentDirectory() ^ "HL7Export\\";
		}
		// (v.maida 2016-05-19 17:01) - NX-100684 - Changed to FileUtils::CreatePath() so that non-existent intermediate paths are created.
		FileUtils::CreatePath(strExportPath);
		CString strPath;
		// (v.maida 2016-06-02 17:53) - NX-100805 - Changed to BrowseToFolder(), so that the initial directory will be correctly displayed when the shared path
		// is a UNC path and network discovery is disabled.
		if (BrowseToFolder(&strPath, "Select Path", m_hWnd, NULL, strExportPath, false)) { 
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
			SetHL7SettingText(nID, "ExportFile", strPath);
			SetDlgItemText(IDC_FILENAME, strPath);
			RefreshGroup(nID);
		}
	}NxCatchAll("Error setting file name");

}

void CHL7SettingsDlg::OnBrowseFileImport() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		// (j.armen 2011-10-25 14:52) - PLID 46137 - HL7 Paths should be based on the practice path
		CString strImportPath = GetHL7SettingText(nID, "ImportFile");
		if (strImportPath.IsEmpty()) {
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
			strImportPath = GetEnvironmentDirectory() ^ "HL7Import\\";
		}
		// (v.maida 2016-05-19 17:01) - NX-100684 - Changed to FileUtils::CreatePath() so that non-existent intermediate paths are created.
		FileUtils::CreatePath(strImportPath);
		CString strPath;
		// (v.maida 2016-06-02 17:53) - NX-100805 - Changed to BrowseToFolder(), so that the initial directory will be correctly displayed when the shared path
		// is a UNC path and network discovery is disabled.
		if (BrowseToFolder(&strPath, "Select Path", m_hWnd, NULL, strImportPath, false)) {
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
			SetHL7SettingText(nID, "ImportFile", strPath);
			RefreshGroup(nID);
			SetDlgItemText(IDC_FILENAME_IMPORT, strPath);
		}
	}NxCatchAll("Error setting FileName");
}

void CHL7SettingsDlg::OnExpectAck() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExpectACK", IsDlgButtonChecked(IDC_EXPECT_ACK));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnExpectAck()");
}

void CHL7SettingsDlg::OnUseMessageDate() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "UseBillDate", TRUE);
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnUseBillDate()");
}

void CHL7SettingsDlg::OnUseCurrentDate() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "UseBillDate", FALSE);
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnUseCurrentDate()");
}

void CHL7SettingsDlg::OnAutoExport()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExportNewPatients", IsDlgButtonChecked(IDC_AUTO_EXPORT));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnAutoExport()");
}

// (j.gruber 2007-08-23 17:37) - PLID 24628  - auto updating
void CHL7SettingsDlg::OnAutoUpdate()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExportUpdatedPatients", IsDlgButtonChecked(IDC_AUTO_UPDATE));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnAutoExport()");
}


void CHL7SettingsDlg::OnSendAck() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "SendAck", IsDlgButtonChecked(IDC_SEND_ACK));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnSendAck()");
}

void CHL7SettingsDlg::OnImportLocations() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "UseImportedLocation", IsDlgButtonChecked(IDC_IMPORT_LOCATIONS));
		RefreshGroup(nID);

		GetDlgItem(IDC_CONFIG_LOCATIONS)->EnableWindow(IsDlgButtonChecked(IDC_IMPORT_LOCATIONS) || IsDlgButtonChecked(IDC_IMPORT_LOCATIONS_AS_POS));

	}NxCatchAll("Error in CHL7SettingsDlg::OnImportLocations()");
}

void CHL7SettingsDlg::OnConfigLocations() 
{
	try { // (c.haag 2006-11-20 15:51) - PLID 23598 - Added try/catch
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		//TES 5/24/2010 - PLID 38865 - Renamed CHL7ConfigLocationsDlg to CHL7ConfigCodeLinksDlg, we now need to pass in the 
		// hclrtLocation enum so it knows we're configuring Location mapping.
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtLocation;
		dlg.DoModal();
	}
	NxCatchAll("Error in OnConfigLocations");
}

void CHL7SettingsDlg::OnImportLocationsAsPos() 
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "UseImportedLocationAsPOS", IsDlgButtonChecked(IDC_IMPORT_LOCATIONS_AS_POS));
		RefreshGroup(nID);

		GetDlgItem(IDC_CONFIG_LOCATIONS)->EnableWindow(IsDlgButtonChecked(IDC_IMPORT_LOCATIONS) || IsDlgButtonChecked(IDC_IMPORT_LOCATIONS_AS_POS));
	}NxCatchAll("Error in CHL7SettingsDlg::OnImportLocationsAsPos()");
}

void CHL7SettingsDlg::RefreshGroup(long nGroupID)
{
	CClient::RefreshTable(NetUtils::HL7SettingsT, nGroupID);
	RefreshHL7Group(nGroupID);
}

// (j.jones 2008-04-24 14:17) - PLID 29600 - added OnCheckBatchExports
void CHL7SettingsDlg::OnCheckBatchExports() 
{
	try {

		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "BatchExports", IsDlgButtonChecked(IDC_CHECK_BATCH_EXPORTS));
		
		RefreshGroup(nID);

	}NxCatchAll("Error in CHL7SettingsDlg::OnSendAck()");
}

// (j.jones 2008-04-24 14:17) - PLID 29600 - added OnCheckBatchImports
void CHL7SettingsDlg::OnCheckBatchImports() 
{
	try {

		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		//TES 5/6/2008 - PLID 29690 - If they're unchecking this, they want to auto-import.  Check whether we'll be able to
		// do that.
		//TES 7/31/2008 - PLID 29690 - Only do this if they're importing via TCP/IP; if they import via File it's no problem
		// because it will just use the logged-in location.
		if(!IsDlgButtonChecked(IDC_CHECK_BATCH_IMPORTS) && IsDlgButtonChecked(IDC_TCP_IMPORT_RAD)) {
			if(!ReturnsRecords("SELECT ID FROM LocationsT WHERE IsDefault = 1")) {
				MsgBox("NOTE: You have chosen to automatically import new patients via TCP/IP, but you do not have a default location for "
					"new patients to be assigned to.  Please select a default location (by going to the Locations tab in the "
					"Administrator module), all incoming patients will be batched until then.");
			}
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "BatchImports", IsDlgButtonChecked(IDC_CHECK_BATCH_IMPORTS));
		
		RefreshGroup(nID);

	}NxCatchAll("Error in CHL7SettingsDlg::OnSendAck()");
}

void CHL7SettingsDlg::OnAutoExportAppts()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		// (z.manning 2008-07-16 10:04) - PLID 30753 - Update the export appointments option for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExportAppts", m_btnAutoExportAppts.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll("CHL7SettingsDlg::OnAutoExportAppts");
}

void CHL7SettingsDlg::OnSelChosenDefaultLabProcedure(LPDISPATCH lpRow)
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		_variant_t varLabProcedureID = g_cvarNull;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			varLabProcedureID = pRow->GetValue(0);
		}

		//TES 10/10/2008 - PLID 21093 - Update the default Lab Procedure for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingInt(nID, "DefaultLabProcedureID", VarLong(varLabProcedureID, -1));
		
		RefreshGroup(nID);

	}NxCatchAll("CHL7SettingsDlg::OnSelChosenDefaultLabProcedure");
}

void CHL7SettingsDlg::OnExportInsurance()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		// (z.manning 2008-07-16 10:04) - PLID 30753 - Update the export appointments option for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExportInsurance", m_btnExportInsurance.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

		//TES 6/30/2010 - PLID 39320 - Insurance company codes are only used if you're exporting insurance information,
		// so disable the button if we aren't.
		//TES 7/19/2010 - PLID 39720 - There are more insurance buttons now, and they're useable if you export OR import, and no 
		// other controls work this way, so I'm just going to leave it enabled.
		//m_nxbConfigInsCos.EnableWindow(m_btnExportInsurance.GetCheck() == BST_CHECKED || m_btnImportInsurance.GetCheck() == BST_CHECKED);

	}NxCatchAll("CHL7SettingsDlg::OnExportInsurance");
}

void CHL7SettingsDlg::OnAutoExportEmnBills()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 7/10/2009 - PLID 34856 - Update the export EMN bills option for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExportEmnBills", m_btnAutoExportEmnBills.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll("CHL7SettingsDlg::OnAutoExportEmnBills");
}

void CHL7SettingsDlg::OnSelectTabHl7SettingsTabs(short newTab, short oldTab)
{
	try {
		//TES 2/23/2010 - PLID 37503 - Show/hide the appropriate controls.
		ReflectCurrentTab();
	}NxCatchAll(__FUNCTION__);
}

void CHL7SettingsDlg::ReflectCurrentTab()
{
	//TES 2/23/2010 - PLID 37503 - What record type are we showing?
	UINT nShowPatInfo = SW_HIDE, nShowApptInfo = SW_HIDE, nShowBillInfo = SW_HIDE, nShowLabInfo = SW_HIDE, nShowRefPhysInfo = SW_HIDE;
	switch(m_tab->CurSel) {
		case 0: //Patients
			nShowPatInfo = SW_SHOW;
			break;
		case 1: //Appointments
			nShowApptInfo = SW_SHOW;
			break;
		case 2: //Bills (NexEMR)
			nShowBillInfo = SW_SHOW;
			break;
		case 3: //Labs
			nShowLabInfo = SW_SHOW;
			break;
		case 4: // Referring Physicians
			nShowRefPhysInfo = SW_SHOW; // (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physicians tab / flag.
			break;
		default:
			AfxThrowNxException("Invalid tab %li selected on HL7 Settings dialog!", m_tab->CurSel);
			break;
	}

	//TES 2/23/2010 - PLID 37503 - Now show/hide all controls appropriately.
	GetDlgItem(IDC_AUTO_EXPORT)->ShowWindow(nShowPatInfo);
	GetDlgItem(IDC_AUTO_UPDATE)->ShowWindow(nShowPatInfo);
	GetDlgItem(IDC_EXPORT_INSURANCE)->ShowWindow(nShowPatInfo);
	GetDlgItem(IDC_CONFIG_INS_COS)->ShowWindow(nShowPatInfo);
	GetDlgItem(IDC_CONFIG_INS_RELATIONS)->ShowWindow(nShowPatInfo);
	GetDlgItem(IDC_IMPORT_INSURANCE)->ShowWindow(nShowPatInfo); //TES 7/21/2010 - PLID 39720
	GetDlgItem(IDC_SEND_A28_31)->ShowWindow(nShowPatInfo); //TES 8/2/2010 - PLID 39935
	GetDlgItem(IDC_EXCLUDE_PROSPECTS)->ShowWindow(nShowPatInfo); //TES 9/20/2010 - PLID 40595
	GetDlgItem(IDC_ADVANCED_PATIENT_SETTINGS)->ShowWindow(nShowPatInfo); // (z.manning 2010-10-04 16:57) - PLID 40795
	GetDlgItem(IDC_AUTO_EXPORT_INSURANCE)->ShowWindow(nShowPatInfo); // (r.goldschmidt 2015-11-05 18:23) - PLID 67517

	GetDlgItem(IDC_AUTO_EXPORT_APPTS)->ShowWindow(nShowApptInfo);
	GetDlgItem(IDC_HL7_CONFIG_RESOURCES)->ShowWindow(nShowApptInfo); // (z.manning 2010-07-07 16:11) - PLID 39559
	GetDlgItem(IDC_AUTO_IMPORT_PATS_FROM_SCHEDULE_MESSAGES)->ShowWindow(nShowApptInfo); // (z.manning 2010-08-09 09:59) - PLID 39985
	GetDlgItem(IDC_HL7_ADV_SCHED_OPTIONS)->ShowWindow(nShowApptInfo); // (z.manning 2011-04-21 12:10) - PLID 43361
	GetDlgItem(IDC_CONFIG_INS_TYPES)->ShowWindow(nShowApptInfo);// (d.singleton 2012-08-27 15:42) - PLID 51938
	GetDlgItem(IDC_APPT_IMPORT_REF_PHYS)->ShowWindow(nShowApptInfo);	// (j.dinatale 2013-01-15 16:53) - PLID 54631
	GetDlgItem(IDC_HL7_SETTINGS_REF_PHYS)->ShowWindow(nShowApptInfo);	// (j.dinatale 2013-01-08 10:47) - PLID 54491

	GetDlgItem(IDC_AUTO_EXPORT_EMN_BILLS)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_BILL_DATE_LABEL)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_USE_MESSAGE_DATE)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_USE_CURRENT_DATE)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_BILL_LOCATION_LABEL)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_IMPORT_LOCATIONS)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_IMPORT_LOCATIONS_AS_POS)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_CONFIG_LOCATIONS)->ShowWindow(nShowBillInfo);
	GetDlgItem(IDC_CONFIG_CHARGE_CODES)->ShowWindow(nShowBillInfo); // (r.farnworth 2015-01-20 13:37) - PLID 64624
	GetDlgItem(IDC_CHK_LINK_FT_DIAG)->ShowWindow(nShowBillInfo); // (b.savon 2015-12-22 09:45) - PLID 67782

	// (b.eyers 2015-06-04) - PLID 66205
	long nCurSel = m_pList->GetCurSel();
	if (nCurSel != -1) {
		BOOL bEnableIntelleChart = GetHL7SettingBit(VarLong(m_pList->GetValue(nCurSel, eflID)), "EnableIntelleChart");
		if (bEnableIntelleChart) {
			GetDlgItem(IDC_CONFIG_INVENTORY)->ShowWindow(nShowBillInfo);
		}
		else
			GetDlgItem(IDC_CONFIG_INVENTORY)->ShowWindow(SW_HIDE);
	}
	else
		GetDlgItem(IDC_CONFIG_INVENTORY)->ShowWindow(SW_HIDE);

	//GetDlgItem(IDC_CONFIG_INVENTORY)->ShowWindow(nShowBillInfo); // (b.eyers 2015-06-04) - PLID 66205

	GetDlgItem(IDC_LAB_PROCEDURE_LABEL)->ShowWindow(nShowLabInfo);
	GetDlgItem(IDC_DEFAULT_LAB_PROCEDURE)->ShowWindow(nShowLabInfo);
	GetDlgItem(IDC_ADVANCED_LAB_SETTINGS)->ShowWindow(nShowLabInfo);
	GetDlgItem(IDC_AUTO_IMPORT_LAB_RESULTS)->ShowWindow(nShowLabInfo); // (z.manning 2010-05-21 14:01) - PLID 38638
	GetDlgItem(IDC_EXPORT_LAB)->ShowWindow(nShowLabInfo); // (a.vengrofski 2010-05-25 10:02) - PLID <38547>
	GetDlgItem(IDC_DEFAULT_SENDING_LAB)->ShowWindow(nShowLabInfo); // (a.vengrofski 2010-05-25 10:02) - PLID <38547>
	GetDlgItem(IDC_SENDING_LAB_TEXT)->ShowWindow(nShowLabInfo); // (a.vengrofski 2010-05-25 10:02) - PLID <38547>

	// (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physicians tab / flag.
	GetDlgItem(IDC_AUTO_EXPORT_REF_PHYS)->ShowWindow(nShowRefPhysInfo);
	GetDlgItem(IDC_AUTO_UPDATE_REF_PHYS)->ShowWindow(nShowRefPhysInfo);
}

void CHL7SettingsDlg::OnAdvancedLabSettings()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MsgBox("Please select an HL7 Settings Group to modify");
			return;
		}

		CHL7LabSettingsDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-21 14:17) - PLID 38638
void CHL7SettingsDlg::OnBnClickedAutoImportLabResults()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		BOOL bChecked = m_btnAutoImportLabs.GetCheck() == BST_CHECKED;
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "AutoImportLabs", bChecked);
		
		RefreshGroup(nID);

		// (z.manning 2010-05-21 14:23) - PLID 38638 - Audit this change.
		CString strOld, strNew;
		if(bChecked) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7AutoImportLabs, nID, strOld, strNew, aepHigh, aetChanged);

	}NxCatchAll(__FUNCTION__);
}

// (a.vengrofski 2010-05-25 10:37) - PLID <38547> - Updates the database.
void CHL7SettingsDlg::OnBnClickedExportLab()
{
	try{
		long nCurRow = VarLong(m_pList->GetCurSel(), (long)-1);
		if (nCurRow == -1){
			return;
		}
		BOOL bChecked = m_btnAutoExportLabs.GetCheck() == BST_CHECKED;
		long nLabID = m_pList->GetValue(nCurRow,eflID);
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nLabID, "AutoExportLabs", bChecked);
		RefreshGroup(nLabID);

		// (a.vengrofski 2010-05-27 16:21) - PLID <38918> - Can never have no much auditing.
		CString strOld, strNew;
		if(bChecked) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7AutoExportLabs, nLabID, strOld, strNew, aepHigh, aetChanged);

	}NxCatchAll("CHL7SettingsDlg::OnBnClickedExportLab");
}
// (a.vengrofski 2010-07-01 16:01) - PLID <38547> 
void CHL7SettingsDlg::SelChosenDefaultSendingLab(LPDISPATCH lpRow)
{
	try{
		_variant_t varSendingLabID, varHL7ID;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow)
		{
			m_pSendingLabList->SetSelByColumn(ehlID, (long)-1);//-1 == {No Link}
			varSendingLabID = (long)-1;
			pRow = m_pSendingLabList->GetCurSel();
		} else {
			varSendingLabID = m_pSendingLabList->GetCurSel()->GetValue(ehlID);
		}
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1){
			return;//if there is no Link to associate this change with, it does not matter...
		} else {
			CString strSQL = "";
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
			CArray<long,long> arHL7GroupIDs;
			GetHL7SettingsGroupsBySetting("DefaultLabID", VarLong(varSendingLabID), arHL7GroupIDs);
			long nNoLink = VarLong(pRow->GetValue(ehlID));
			if ((nNoLink == -1) || arHL7GroupIDs.GetSize() == 0) {//If the first row ({No Link}) has been selected, we should save that.
				varHL7ID = m_pList->GetValue(nCurSel, eflID);
				//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
				SetHL7SettingInt(VarLong(varHL7ID), "DefaultLabID", VarLong(varSendingLabID));
				// (a.vengrofski 2010-05-27 16:21) - PLID <38918> - Can never have no much auditing.
				// (a.vengrofski 2010-08-11 16:03) - PLID <38918> - Need to audit the Names not numbers, also make it more understandable.
				CString strOld = "Group: ", strNew = "From: ";
				_variant_t varAudit = g_cvarEmpty;
				NXDATALIST2Lib::IRowSettingsPtr pFirstSend = m_pSendingLabList->GetFirstRow();
				NXDATALIST2Lib::IRowSettingsPtr pRowOld = m_pSendingLabList->FindByColumn(ehlID, m_nDefaultLabID, pFirstSend, FALSE);
				NXDATALIST2Lib::IRowSettingsPtr pRowNew = m_pSendingLabList->FindByColumn(ehlID, varSendingLabID, pFirstSend, FALSE);
				strOld += VarString(m_pList->GetValue(nCurSel, eflName), "{No Group}");
				if (pRowOld)
				{
					strNew += VarString(pRowOld->GetValue(ehlName)) + " To: ";
				}
				else
				{
					//This should not be possible; making a selection that is not in the list
					strNew += "{No Link} To: ";
				}
				if (pRowNew)
				{
					strNew += VarString(pRowNew->GetValue(ehlName));
				}
				else
				{
					//This should not be possible; making a selection that is not in the list
					strNew += "{No Link}";
				}
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7DefaultLabLink, VarLong(varHL7ID), strOld, strNew, aepHigh, aetChanged);
				RefreshGroup(VarLong(varHL7ID));
				m_nDefaultLabID = VarLong(varSendingLabID);
			} else {
				//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
				long nCurID = GetHL7SettingInt(VarLong(m_pList->GetValue(nCurSel,eflID)), "DefaultLabID");
				if (nNoLink != nCurID){
					MessageBox("That lab company already has a default link. Please select another company.","Select another lab company",MB_ICONWARNING);
					m_pSendingLabList->SetSelByColumn(ehlID, nCurID);
				}
			}
		}
	}NxCatchAll("Error in CHL7SettingsDlg::SelChosenSendingLab");
}

// (a.vengrofski 2010-05-13 09:20) - PLID <38547> - 
void CHL7SettingsDlg::RequeryFinishedDefaultSendingLab(short nFlags)
{
	try{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel != -1) {
			_variant_t varID(m_pList->GetValue(nCurSel, eflID));
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
			m_pSendingLabList->SetSelByColumn(ehlID, GetHL7SettingInt(VarLong(varID, -1), "DefaultLabID"));
		} else {
			m_pSendingLabList->SetSelByColumn(ehlID, (long)-1);//-1 = {No Link}
		}
	}NxCatchAll("Error in CHL7SettingsDlg::RequeryFinishedSendingLab");
}

void CHL7SettingsDlg::OnConfigInsCos()
{
	try {

		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}
		
		//TES 6/30/2010 - PLID 39320 - Pop up CHL7ConfigCodeLinksDlg, telling it to look at Insurance Companies
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtInsCo;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-07-07 15:57) - PLID 39559
void CHL7SettingsDlg::OnBnClickedHl7ConfigResources()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}
		
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtResource;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

//TES 7/13/2010 - PLID 39610
void CHL7SettingsDlg::OnConfigInsRelations()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}
		
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtRelationToPatient;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHL7SettingsDlg::OnImportInsurance()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 7/19/2010 - PLID 39720 - Update the import insurance option for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ImportInsurance", m_btnImportInsurance.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll(__FUNCTION__);
}

void CHL7SettingsDlg::OnSendA2831()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 8/2/2010 - PLID 39935 - Update the SendA2831 option for this HL7 group
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "SendA2831", m_btnSendA2831.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll(__FUNCTION__);

}

// (z.manning 2010-08-09 10:02) - PLID 39985
void CHL7SettingsDlg::OnBnClickedAutoImportPatsFromScheduleMessages()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "AutoCreatePatientsFromScheduleMessages", m_btnAutoImportPatsFromSiu.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll(__FUNCTION__);
}

void CHL7SettingsDlg::OnExcludeProspects()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 9/20/2010 - PLID 40595 - Update the field in HL7SettingsT
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "ExcludeProspects", m_btnExcludeProspects.GetCheck() == BST_CHECKED);
		
		RefreshGroup(nID);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-10-04 16:53) - PLID 40795
void CHL7SettingsDlg::OnBnClickedAdvancedPatientSettings()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MessageBox("Please select an HL7 Settings Group first.");
			return;
		}

		long nGroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		CHL7PatientSettingsDlg dlg(nGroupID, this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

//TES 9/15/2011 - PLID 45523 - Moved SendingFacilityID and ForceFacilityIDMatch to CHL7GeneralSettingsDlg

// (z.manning 2011-04-21 10:39) - PLID 43361
void CHL7SettingsDlg::OnBnClickedHl7AdvSchedOptions()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MessageBox("Please select an HL7 Settings Group first.");
			return;
		}

		long nGroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		CHL7SchedSettingsDlg dlg(nGroupID, this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHL7SettingsDlg::OnAdvancedGeneralSettings()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			MessageBox("Please select an HL7 Settings Group first.");
			return;
		}

		//TES 9/15/2011 - PLID 45523 - Pop up the dialog for General options, including SendingFacilityID and ForceFacilityIDMatch
		long nGroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		CHL7GeneralSettingsDlg dlg(this);
		dlg.m_nHL7GroupID = nGroupID;
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (d.singleton 2012-08-24 16:30) - PLID 52302
void CHL7SettingsDlg::OnBnClickedConfigInsTypes()
{
	// TODO: Add your control notification handler code here
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtInsuranceCategoryType;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-01-08 10:47) - PLID 54491 - open the ID link config dialog
void CHL7SettingsDlg::OnBnClickedHl7SettingsRefPhys()
{
	try{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));

		CHL7IDLinkConfigDlg dlg(hilrtReferringPhysician, nHL7GroupID, this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-01-15 16:28) - PLID 54631 - Save our setting for importing ref phys from appts
void CHL7SettingsDlg::OnBnClickedApptImportRefPhys()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));
		SetHL7SettingBit(nID, "IMPORTAPPTREFPHYS", m_btnApptImportRefPhys.GetCheck() == BST_CHECKED);
		RefreshGroup(nID);
	}NxCatchAll(__FUNCTION__);
}

// (v.maida 2014-12-23 12:19) - PLID 64472 - Added checkboxes for autoexporting referring physicians.
void CHL7SettingsDlg::OnAutoExportRefPhys()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "EXPORTNEWREFPHYS", IsDlgButtonChecked(IDC_AUTO_EXPORT_REF_PHYS));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnAutoExportRefPhys()");
}

// (v.maida 2014-12-23 12:19) - PLID 64472 - Added checkboxes for autoexporting referring physicians.
void CHL7SettingsDlg::OnAutoUpdateRefPhys()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(nID, "EXPORTUPDATEDREFPHYS", IsDlgButtonChecked(IDC_AUTO_UPDATE_REF_PHYS));
		RefreshGroup(nID);
	}NxCatchAll("Error in CHL7SettingsDlg::OnAutoUpdateRefPhys()");
}

// (r.farnworth 2015-01-20 13:34) - PLID 64624 - Add the ability to override Charge Codes when importing charges through HL7.
void CHL7SettingsDlg::OnBnClickedConfigChargeCodes()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtServiceCode;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2015-06-04) - PLID 66205 - MDI clients can import inventory through HL7, setup codes dialog
void CHL7SettingsDlg::OnBnClickedConfigInventory()
{
	try
	{
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = VarLong(m_pList->GetValue(nCurSel, 0));
		dlg.m_hclrtType = hclrtInventory;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2015-06-10) - PLID 66354 - enable the advanced settings for mdi intellechart
void CHL7SettingsDlg::OnEnableIntelleChart()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		SetHL7SettingBit(nID, "EnableIntelleChart", IsDlgButtonChecked(IDC_ENABLE_INTELLECHART));

		// (b.spivey, February 3, 2016) - PLID 68169 - If IntelleChart setings are on, we force "SendAck" to true and disable the checkbox.
		if (IsDlgButtonChecked(IDC_ENABLE_INTELLECHART)) {
			SetHL7SettingBit(nID, "SendAck", TRUE);
			m_btnSendAck.SetCheck(TRUE); 
			m_btnSendAck.EnableWindow(FALSE);
		}
		else {
			m_btnSendAck.EnableWindow(TRUE);
		}

		RefreshGroup(nID);

		// (b.eyers 2015-06-15) - PLID 66205
		ReflectCurrentTab();

	}NxCatchAll("Error in CHL7SettingsDlg::OnEnableIntelleChart()");
}

// (r.goldschmidt 2015-11-05 18:14) - PLID 67517 - New HL7 setting to not mass auto export ADT messages when any insurace Company specific fields are edited
void CHL7SettingsDlg::OnBnClickedAutoExportInsurance()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		SetHL7SettingBit(nID, "AutoExportInsurance", IsDlgButtonChecked(IDC_AUTO_EXPORT_INSURANCE));
		RefreshGroup(nID);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-22 09:46) - PLID 67782 - Add HL7 option to read FT1 and DG1 segments exclusively
void CHL7SettingsDlg::OnBnClickedChkLinkFtDiag()
{
	try {
		long nCurSel = m_pList->GetCurSel();
		if (nCurSel == -1) {
			return;
		}

		long nID = VarLong(m_pList->GetValue(nCurSel, 0));

		SetHL7SettingBit(nID, "ExclusivelyLinkFTandDG", IsDlgButtonChecked(IDC_CHK_LINK_FT_DIAG));
		RefreshGroup(nID);
	}NxCatchAll(__FUNCTION__);
}

// (v.maida 2016-06-14 10:35) - NX-100833 - Created a function for enabling/disabling controls that only tech support should use.
void CHL7SettingsDlg::EnableTechSupportControls(BOOL bEnable)
{
	// if FALSE then we're disabling buttons and enabling the read only status of edit boxes, and vice versa
	( (CEdit *)GetDlgItem(IDC_FILENAME))->SetReadOnly(!bEnable);
	GetDlgItem(IDC_BROWSE_FILE)->EnableWindow(bEnable);
	( (CEdit *)GetDlgItem(IDC_FILENAME_IMPORT))->SetReadOnly(!bEnable);
	GetDlgItem(IDC_BROWSE_FILE_IMPORT)->EnableWindow(bEnable);
}