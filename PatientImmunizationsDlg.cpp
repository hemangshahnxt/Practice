// PatientImmunizationsDlg.cpp : implementation file
// (d.thompson 2009-05-12) - PLID 34232 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "PatientImmunizationsDlg.h"
#include "EditImmunizationDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "globalreportutils.h"
#include "AuditTrail.h"
#include "WellnessDataUtils.h"
#include "HL7Utils.h"
#include <NxHL7Lib\HL7Session.h>
#include <NxHL7Lib\HL7MessageFactory.h>
#include "ImmunizationExportConfig.h"

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// CPatientImmunizationsDlg dialog
enum eListColumns {
	elcID = 0,
	elcDate,
	elcType,
};

enum eCodeColumns {
	eccName = 0,
	eccCode
};

IMPLEMENT_DYNAMIC(CPatientImmunizationsDlg, CNxDialog)

CPatientImmunizationsDlg::CPatientImmunizationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientImmunizationsDlg::IDD, pParent)
{
	m_nPersonID = -1;
	m_nImmunizationHL7GroupID = -1;
}

CPatientImmunizationsDlg::~CPatientImmunizationsDlg()
{
}

void CPatientImmunizationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_IMM_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_IMM_MODIFY, m_btnModify);
	DDX_Control(pDX, IDC_IMM_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_PREVIEW_REPORT, m_btnPreview);
	DDX_Control(pDX, IDC_EXPORT_IMMUNIZATIONS, m_btnExport);
	DDX_Control(pDX, IDC_CONFIG_HL7_EXPORT, m_btnHL7ExportConfig);
	// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
	DDX_Control(pDX, IDC_PATIENT_IMMUNIZATION_PUBLICITY_DATE, m_dtpPublicity);
	DDX_Control(pDX, IDC_PATIENT_IMMUNIZATION_REGISTRY_DATE, m_dtpRegistry);
	DDX_Control(pDX, IDC_PATIENT_IMMUNIZATION_IIS_CONSENT_DATE, m_dtpIISConsent);
}


BEGIN_MESSAGE_MAP(CPatientImmunizationsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_IMM_ADD, &CPatientImmunizationsDlg::OnBnClickedImmAdd)
	ON_BN_CLICKED(IDC_IMM_MODIFY, &CPatientImmunizationsDlg::OnBnClickedImmModify)
	ON_BN_CLICKED(IDC_IMM_DELETE, &CPatientImmunizationsDlg::OnBnClickedImmDelete)
	ON_BN_CLICKED(IDC_PREVIEW_REPORT, &CPatientImmunizationsDlg::OnBnClickedPreviewReport)
	ON_BN_CLICKED(IDC_EXPORT_IMMUNIZATIONS, &CPatientImmunizationsDlg::OnBnClickedExportImmunizations)
	ON_BN_CLICKED(IDC_CONFIG_HL7_EXPORT, &CPatientImmunizationsDlg::OnBnClickedHL7ExportConfig)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PATIENT_IMMUNIZATION_PUBLICITY_DATE, &CPatientImmunizationsDlg::OnChangePublicityDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PATIENT_IMMUNIZATION_REGISTRY_DATE, &CPatientImmunizationsDlg::OnChangeRegistryDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PATIENT_IMMUNIZATION_IIS_CONSENT_DATE, &CPatientImmunizationsDlg::OnChangeIISConsentDate)
	ON_BN_CLICKED(IDC_PATIENT_IMMUNIZATION_IIS_CONSENT, &CPatientImmunizationsDlg::OnClickIISConsent) // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientImmunizationsDlg, CNxDialog)
	ON_EVENT(CPatientImmunizationsDlg, IDC_IMMUNIZATION_LIST, 3 /* DblClickCell */, CPatientImmunizationsDlg::OnDblClickCellList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEditImmunizationDlg, IDC_PATIENT_IMMUNIZATION_PUBLICITY_CODE, 1, CPatientImmunizationsDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_PATIENT_IMMUNIZATION_PUBLICITY_CODE, 16, CPatientImmunizationsDlg::OnSelChosenPublicityCode, VTS_DISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_PATIENT_IMMUNIZATION_REGISTRY_CODE, 1, CPatientImmunizationsDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_PATIENT_IMMUNIZATION_REGISTRY_CODE, 16, CPatientImmunizationsDlg::OnSelChosenRegistryCode, VTS_DISPATCH)
END_EVENTSINK_MAP()

// CPatientImmunizationsDlg message handlers
BOOL CPatientImmunizationsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (d.singleton 2012-12-11 15:49) - PLID 54141 cache properties
		g_propManager.CachePropertiesInBulk("PatientImmunizationsDlg", propText, 
			"(UserName = '<none>' OR UserName = '%s') AND"
			" Name = 'HL7ImmunizationFacilityID' OR"
			" Name = 'HL7ImmunizationFacilityName' OR"
			" Name = 'HL7ImmunizationVFCStatus'", _Q(GetCurrentUserName()));

		// (d.singleton 2013-06-05 14:01) - PLID 57057 - new dialog to choose hl7 group id, facility name, facility id for immunization hl7 export
		g_propManager.CachePropertiesInBulk("PatientImmunizationsDlg", propNumber, 
			"(UserName = '<none>' OR UserName = '%s') AND"
			" Name = 'HL7ImmunizationHL7GroupID'", _Q(GetCurrentUserName()));

		//interface niceties
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnModify.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (a.walling 2010-02-18 10:44) - PLID 37434
		m_btnExport.AutoSet(NXB_EXPORT);
		// (d.singleton 2012-12-11 10:49) - PLID 54141 add config dlg for immunization hl7 settings
		m_btnHL7ExportConfig.AutoSet(NXB_MODIFY);
		m_pList = BindNxDataList2Ctrl(IDC_IMMUNIZATION_LIST, GetRemoteData(), false);
		// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
		m_pPublicity = BindNxDataList2Ctrl(IDC_PATIENT_IMMUNIZATION_PUBLICITY_CODE, GetRemoteData(), true);
		m_pRegistryStatus = BindNxDataList2Ctrl(IDC_PATIENT_IMMUNIZATION_REGISTRY_CODE, GetRemoteData(), true);

		//load interface
		if(m_nPersonID == -1) {
			//not set
			MessageBox("You must provide a patient when using this dialog.");
			CDialog::OnCancel();
			return TRUE;
		}

		//Set the patient
		m_pList->WhereClause = _bstr_t(FormatString("PersonID = %li", m_nPersonID));
		m_pList->Requery();

		// Create the icon on the "Preview" button
		m_btnPreview.AutoSet(NXB_PRINT_PREV);

		// Disable the immunization hl7 export and export config buttons if they're not licensed for hL7
		if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
		{
			m_btnExport.EnableWindow(TRUE);
			m_btnHL7ExportConfig.EnableWindow(TRUE);
			// (d.singleton 2013-06-05 14:01) - PLID 57057 - new dialog to choose hl7 group id, facility name, facility id for immunization hl7 export
			m_nImmunizationHL7GroupID = GetRemotePropertyInt("HL7ImmunizationHL7GroupID", -1, 0, "<None>", true);
		}
		else
		{
			m_btnExport.EnableWindow(FALSE);
			m_btnHL7ExportConfig.EnableWindow(FALSE);
			m_nImmunizationHL7GroupID = -1;
		}

		// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
		{
			ADODB::_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
				"SELECT "
				  "PatientsT.PersonID "
				", PatientImmunizationInfoT.PublicityCode "
				", PatientImmunizationInfoT.PublicityDate "
				", PatientImmunizationInfoT.RegistryStatusCode "
				", PatientImmunizationInfoT.RegistryStatusDate "
				", PatientImmunizationInfoT.IISConsent " // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
				", PatientImmunizationInfoT.IISConsentDate "
				"FROM PatientsT "
				"LEFT JOIN PatientImmunizationInfoT "
					"ON PatientsT.PersonID = PatientImmunizationInfoT.PatientID "
				"WHERE PatientsT.PersonID = {INT}"
				, m_nPersonID
			);

			if (prs->eof) {
				// patient doesn't exist?
				MessageBox("The selected patient does not exist.");
				CDialog::OnCancel();
				return TRUE;
			}

			m_info.publicityCode = AdoFldVar(prs, "PublicityCode");
			m_info.publicityDate = AdoFldVar(prs, "PublicityDate");
			m_info.registryCode = AdoFldVar(prs, "RegistryStatusCode");
			m_info.registryDate = AdoFldVar(prs, "RegistryStatusDate");
			m_info.iisConsent = AdoFldVar(prs, "IISConsent"); // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
			m_info.iisConsentDate = AdoFldVar(prs, "IISConsentDate");

			m_pPublicity->SetSelByColumn(eccCode, m_info.publicityCode);
			m_dtpPublicity.SetValue(m_info.publicityDate);
			m_pRegistryStatus->SetSelByColumn(eccCode, m_info.registryCode);
			m_dtpRegistry.SetValue(m_info.registryDate);

			m_dtpIISConsent.SetValue(m_info.iisConsentDate);

			CButton* pCheckIISConsent = (CButton*)GetDlgItem(IDC_PATIENT_IMMUNIZATION_IIS_CONSENT);
			if (m_info.iisConsent.vt == VT_NULL) {
				pCheckIISConsent->SetCheck(BST_INDETERMINATE);
			} else {
				pCheckIISConsent->SetCheck(VarLong(m_info.iisConsent) ? BST_CHECKED : BST_UNCHECKED);
			}
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CPatientImmunizationsDlg::OnBnClickedImmAdd()
{
	try {
		//Create a new immunization by providing no ID
		CEditImmunizationDlg dlg(this);
		dlg.m_nPersonID = m_nPersonID;
		// (d.thompson 2013-07-16) - PLID 57513 - For auditing
		dlg.m_strPersonName = m_strPersonName;
		if(dlg.DoModal() == IDOK) {
			//Changes may have happened, reload
			m_pList->Requery();
		}

	} NxCatchAll("Error in OnBnClickedImmAdd");
}

void CPatientImmunizationsDlg::OnBnClickedImmModify()
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow != NULL) {
			//Edit it
			CEditImmunizationDlg dlg(this);
			dlg.m_nID = VarLong(pRow->GetValue(elcID));
			dlg.m_nPersonID = m_nPersonID;
			// (d.thompson 2013-07-16) - PLID 57513 - For auditing
			dlg.m_strPersonName = m_strPersonName;
			if(dlg.DoModal() == IDOK) {
				//Changes may have happened, reload
				m_pList->Requery();
			}
		}

	} NxCatchAll("Error in OnBnClickedImmModify");
}

void CPatientImmunizationsDlg::OnBnClickedImmDelete()
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow != NULL) {
			long nID = VarLong(pRow->GetValue(elcID));

			//TES 6/9/2009 - PLID 34525 - Don't let them delete this if it's been used to complete a Wellness alert.
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM PatientWellnessCompletionItemT "
				"WHERE RecordType = 2 AND CompletionRecordID = {INT}", nID);
			if(!prs->eof) {
				MsgBox("This immunization has been used on Wellness Alerts, and cannot be deleted.");
				return;
			}

			//Simply delete it from data.  There are no references at this point, no concerns.
			if(AfxMessageBox("Are you sure you wish to delete this immunization?", MB_YESNO) == IDNO) {
				return;
			}

			// (d.thompson 2013-07-16) - PLID 57513 - Gather some screen data for audits.
			CString strOldTypeName = VarString(pRow->GetValue(elcType));
			COleDateTime dtOldDate = VarDateTime(pRow->GetValue(elcDate));

			
			// (b.cardillo 2009-06-08 18:12) - PLID 34511 - Get the type of the immunization we're deleting
			long nOldTypeID;
			{
				ADODB::_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
					"SET NOCOUNT ON \r\n"
					"DECLARE @nOldTypeID INT \r\n"
					"SET @nOldTypeID = (SELECT ImmunizationID FROM PatientImmunizationsT WHERE ID = {INT}) \r\n"
					" \r\n"
					"SET NOCOUNT ON \r\n"
					"DELETE FROM PatientImmunizationsT WHERE ID = {INT}; \r\n"
					"SET NOCOUNT OFF \r\n"
					" \r\n"
					"SELECT @nOldTypeID As OldTypeID WHERE @nOldTypeID IS NOT NULL\r\n"
					, nID, nID);

				if (!prs->eof) {
					nOldTypeID = AdoFldLong(prs->GetFields()->GetItem("OldTypeID"));
				} else {
					nOldTypeID = -1;
				}
				prs->Close();
			}

			// (d.thompson 2013-07-16) - PLID 57513 - Audit deletion
			AuditEvent(m_nPersonID, m_strPersonName, BeginNewAuditEvent(), aeiPatientImmDeleted, nID, 
				FormatString("%s (%s)", strOldTypeName, FormatDateTimeForInterface(dtOldDate)), "", aepHigh, aetDeleted);
			
			// (b.cardillo 2009-06-08 18:12) - PLID 34511 - When we delete an immunization we need to update patient 
			// qualification records for that immunization type
			if (nOldTypeID != -1) {
				//TES 7/8/2009 - PLID 34534 - This function now updates both qualifications and completion items.
				UpdatePatientWellness_Immunization(GetRemoteData(), m_nPersonID, nOldTypeID, GetCurrentUserID());
			}

			//Refresh!
			m_pList->Requery();
		}

	} NxCatchAll("Error in OnBnClickedImmDelete");
}

void CPatientImmunizationsDlg::OnDblClickCellList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		//Just modify
		OnBnClickedImmModify();

	} NxCatchAll("Error in OnDblClickCellList");
}

// v.arth 2009-05-21 PLID 34321 - Preview immunization report
void CPatientImmunizationsDlg::OnBnClickedPreviewReport()
{
	try
	{
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(664)]);
		infReport.nPatient = m_nPersonID;

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, (CWnd *)this, "Patient Immunization Information");

		EndDialog(IDCANCEL);


		// Get the patient's full name so that it can be stored in the audit trail
		CString patientFullName = GetExistingPatientName(m_nPersonID);

		// Create the audit event
        long nAuditID = BeginNewAuditEvent();
        if (nAuditID != -1)
        {
            AuditEvent(m_nPersonID, patientFullName, nAuditID, aeiPatientImmunizationRecord, m_nPersonID, "", "", aepLow, aetOpened);
        }
	}
	NxCatchAll("Error in CPatientImmunizationsDlg::OnBnClickedPreviewReport");
}

// (a.walling 2010-02-18 10:13) - PLID 37434 - Export immunizations
void CPatientImmunizationsDlg::OnBnClickedExportImmunizations()
{
	try
	{
		CMenu menu;
		menu.CreatePopupMenu();

		enum eMenu {
			eThisPatient = 1,
			eAllPatients,
		};

		menu.AppendMenu(MF_STRING|MF_BYPOSITION, eThisPatient, "For this patient");
		menu.AppendMenu(MF_STRING|MF_BYPOSITION, eAllPatients, "For all patients");

		CPoint pt;
		GetMessagePos(pt);

		int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if (nResult == 0) {
			return;
		}

		// (j.armen 2013-01-28 13:47) - PLID 54224 - Filter now uses the patientID
		scoped_ptr<long> pnPatientID;

		if (nResult == eThisPatient) {
			m_pList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

			if (m_pList->GetRowCount() == 0) {
				MessageBox("This patient has no immunizations recorded!");
				return;
			}

			pnPatientID.reset(new long(m_nPersonID));
		} else if (nResult == eAllPatients) {
			// no where clause
		}

		CString strFileName = FormatString("Immunizations %s.hl7", COleDateTime::GetCurrentTime().Format("%Y%m%d%H%M%S"));

		CFileDialog SaveAs(FALSE, NULL, strFileName, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "HL7 Files (*.hl7)|*.hl7|All Files (*.*)|*.*||");

		if (SaveAs.DoModal() == IDCANCEL) {
			return;
		}
		strFileName = SaveAs.GetPathName();

		CString strResults;
		{
			CWaitCursor cws;
			
			// (r.gonet 12/03/2012) - PLID 54111 - Create a session containing some state variables.
			CHL7Session session(GetCurrentLocationID(), GetCurrentLocationName(), GetCurrentUserID(), GetCurrentUserName());
			// (r.gonet 12/03/2012) - PLID 54111 - Use the HL7 message factory to build a vaccination message batch.
			CHL7MessageFactory factory(&session, GetRemoteData(), GetHL7SettingsCache());
			strResults = factory.CreateVaccinationBatch(pnPatientID.get(), m_nImmunizationHL7GroupID);
		}

		if (strResults.IsEmpty()) {
			MessageBox("There were no results!");
			return;
		}

		{
			CFile f(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);

			f.Write(strResults, strResults.GetLength());
		}

		MessageBox(FormatString("The exported HL7 batch has been saved to %s.", strFileName));
	}
	NxCatchAll("Error in CPatientImmunizationsDlg::OnBnClickedExportImmunizations");
}

// (d.singleton 2012-12-11 10:34) - PLID 54141 option to change the facility id neede for some hl7 integrations
// (d.singleton 2013-06-05 14:39) - PLID 57057 - new dialog to choose hl7 group id, facility name, facility id for immunization hl7 export
void CPatientImmunizationsDlg::OnBnClickedHL7ExportConfig()
{
	try {
		//get stored values
		CString strFacilityID = GetRemotePropertyText("HL7ImmunizationFacilityID", "", 0, "<None>", true);
		CString strFacilityName = GetRemotePropertyText("HL7ImmunizationFacilityName", "", 0, "<None>", true);		
		//initialize dialog
		CImmunizationExportConfig dlg;
		dlg.m_strFacilityID = strFacilityID;
		dlg.m_strFacilityName = strFacilityName;
		dlg.m_nHL7GroupID = m_nImmunizationHL7GroupID;
		if(dlg.DoModal() == IDOK) {
			//get our new group id
			m_nImmunizationHL7GroupID = dlg.m_nHL7GroupID;
		}
	}NxCatchAll(__FUNCTION__);
}


// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.

void CPatientImmunizationsDlg::OnChangePublicityDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		ImmunizationInfo info = m_info;

		info.publicityDate = m_dtpPublicity.GetValue();
		
		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

void CPatientImmunizationsDlg::OnChangeRegistryDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		ImmunizationInfo info = m_info;

		info.registryDate = m_dtpRegistry.GetValue();
		
		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

void CPatientImmunizationsDlg::OnChangeIISConsentDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		ImmunizationInfo info = m_info;

		info.iisConsentDate = m_dtpIISConsent.GetValue();
		
		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

void CPatientImmunizationsDlg::OnSelChosenPublicityCode(LPDISPATCH lpRow)
{
	try {
		ImmunizationInfo info = m_info;

		if (NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow) {
			info.publicityCode = pRow->Value[eccCode];
			if (!VarNull(info.publicityCode) && m_info.publicityCode != info.publicityCode) {
				info.publicityDate = _variant_t(AsDateNoTime(COleDateTime::GetCurrentTime()), VT_DATE);
				m_dtpPublicity.SetValue(info.publicityDate);
			}
		}
		
		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

void CPatientImmunizationsDlg::OnSelChosenRegistryCode(LPDISPATCH lpRow)
{
	try {
		ImmunizationInfo info = m_info;

		if (NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow) {
			info.registryCode = pRow->Value[eccCode];
			if (!VarNull(info.registryCode) && m_info.registryCode != info.registryCode) {
				info.registryDate = _variant_t(AsDateNoTime(COleDateTime::GetCurrentTime()), VT_DATE);
				m_dtpRegistry.SetValue(info.registryDate);
			}
		}
		
		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
void CPatientImmunizationsDlg::OnClickIISConsent()
{
	try {
		ImmunizationInfo info = m_info;

		CButton* pCheckIISConsent = (CButton*)GetDlgItem(IDC_PATIENT_IMMUNIZATION_IIS_CONSENT);
		int chk = pCheckIISConsent->GetCheck();
		if (chk == BST_INDETERMINATE) {
			info.iisConsent = g_cvarNull;
		} else {
			info.iisConsent = chk ? 1L : 0L;
		}

		if (VarNull(info.iisConsentDate) && info.iisConsent != m_info.iisConsent) {
			info.iisConsentDate = _variant_t(AsDateNoTime(COleDateTime::GetCurrentTime()), VT_DATE);
			m_dtpIISConsent.SetValue(info.iisConsentDate);
		}

		ApplyInfo(info);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
void CPatientImmunizationsDlg::ApplyInfo(const ImmunizationInfo& newInfo)
{
	try {
		if (newInfo == m_info) {
			return;
		}

		 // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
		ExecuteParamSql(
			"UPDATE PatientImmunizationInfoT "
			"SET PublicityCode = {VT_BSTR}, PublicityDate = {VT_DATE}, RegistryStatusCode = {VT_BSTR}, RegistryStatusDate = {VT_DATE}, IISConsent = {VT_I4}, IISConsentDate = {VT_DATE} "
			"WHERE PatientID = {INT}; "
			"IF @@ROWCOUNT = 0 INSERT INTO PatientImmunizationInfoT(PatientID, PublicityCode, PublicityDate, RegistryStatusCode, RegistryStatusDate, IISConsent, IISConsentDate) "
			"VALUES({INT}, {VT_BSTR}, {VT_DATE}, {VT_BSTR}, {VT_DATE}, {VT_I4}, {VT_DATE}); "
			, newInfo.publicityCode
			, newInfo.publicityDate
			, newInfo.registryCode
			, newInfo.registryDate
			, newInfo.iisConsent
			, newInfo.iisConsentDate
			, m_nPersonID
			, m_nPersonID
			, newInfo.publicityCode
			, newInfo.publicityDate
			, newInfo.registryCode
			, newInfo.registryDate
			, newInfo.iisConsent
			, newInfo.iisConsentDate
		);

		m_info = newInfo;
	} NxCatchAll(__FUNCTION__);
}

