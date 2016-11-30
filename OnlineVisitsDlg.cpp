// COnlineVisitsDlg.cpp : implementation file
// (r.farnworth 2016-03-04 15:32) - PLID 68396 - Created

#include "stdafx.h"
#include "Practice.h"
#include "OnlineVisitsDlg.h"
#include "PatientAssignmentDlg.h"
#include "FinancialRc.h"
#include "afxdialogex.h"
#include "NxAPI.h"
#include "SelectDlg.h"
#include "Base64.h"
#include "mergeengine.h"
#include <NxAPILib/NxAPIUtils.h>


using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(COnlineVisitsDlg, CNxDialog)

// (r.farnworth 2016-03-02 15:14) - PLID 68455
namespace ProviderColumns {
	enum _E {
		ID = 0,
		Name,
	};
};

// (r.farnworth 2016-03-07 07:27) - PLID 68396
namespace OnlineVisitColumns {
	enum _E {
		ID = 0,
		Checkbox,
		Preview,
		PatientID,
		UID,
		IagnosisPatientID,
		Patient,
		FirstName,
		LastName,
		Email,
		Address1,
		Address2,
		City,
		State,
		Zip,
		HomePhone,
		CellPhone,
		Gender,
		Birthdate,
		Provider,
		Description,
		CreatedDate,
		ModifiedDate,
		PDF,
	};
};

COnlineVisitsDlg::COnlineVisitsDlg(CWnd* pParent)
	: CNxDialog(COnlineVisitsDlg::IDD, pParent)
{
	m_hMagnifyingGlass = NULL;
	m_bHistoryWritePerm = true; // (r.farnworth 2016-03-07 08:35) - PLID 68401
}

COnlineVisitsDlg::~COnlineVisitsDlg()
{
	DestroyIcon(m_hMagnifyingGlass);
	m_mapCheckedRows.RemoveAll();
}

void COnlineVisitsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REFRESH_BUTTON, m_btnRefresh);
	DDX_Control(pDX, IDC_CONFIGURE_BUTTON, m_btnConfigure);
}


BEGIN_MESSAGE_MAP(COnlineVisitsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, &COnlineVisitsDlg::OnBnClickedRefreshButton)
	ON_BN_CLICKED(IDC_CONFIGURE_BUTTON, &COnlineVisitsDlg::OnBnClickedConfigureButton)
	ON_BN_CLICKED(IDC_IMPORT_BUTTON, &COnlineVisitsDlg::OnBnClickedImportButton)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(COnlineVisitsDlg, CNxDialog)
	ON_EVENT(COnlineVisitsDlg, IDC_ONLINE_VISTS_LIST, 19, COnlineVisitsDlg::OnLeftClickOnlineVisitsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COnlineVisitsDlg, IDC_PROVIDER_COMBO, 16 /* SelChanged */, OnSelChosenProviderCombo, VTS_DISPATCH)
	ON_EVENT(COnlineVisitsDlg, IDC_ONLINE_VISTS_LIST, 10, COnlineVisitsDlg::EditingFinishedOnlineVistsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(COnlineVisitsDlg, IDC_PROVIDER_COMBO, 1, COnlineVisitsDlg::OnSelChangingProviderCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

BOOL COnlineVisitsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// (r.farnworth 2016-03-03 09:53) - PLID 68453
	g_propManager.CachePropertiesInBulk("COnlineVisitsDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'OnlineVisitsDocumentCategory' "
		")",
		_Q(GetCurrentUserName()));

	m_btnRefresh.AutoSet(NXB_REFRESH);
	m_btnConfigure.AutoSet(NXB_MODIFY);
	m_hMagnifyingGlass = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16, 16, 0);

	m_pOnlineVisits = BindNxDataList2Ctrl(this, IDC_ONLINE_VISTS_LIST, GetRemoteData(), false);
	m_dlProviderCombo = BindNxDataList2Ctrl(this, IDC_PROVIDER_COMBO, GetRemoteData(), false); 	// (r.farnworth 2016-03-02 15:14) - PLID 68455

	// (r.farnworth 2016-03-03 14:46) - PLID 68452 - Create a permission to access the "Configuration" button within the Online Visits tab, within the Links module.
	if (!(GetCurrentUserPermissions(bioOnlineVisitConfigure) & sptWrite)) {
		GetDlgItem(IDC_CONFIGURE_BUTTON)->EnableWindow(FALSE);
	}

	// (r.farnworth 2016-03-07 08:35) - PLID 68401 - The user cannot import documents if they don't have this permission
	if (!(GetCurrentUserPermissions(bioPatientHistory) & sptWrite)) {
		m_bHistoryWritePerm = false;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void COnlineVisitsDlg::OnBnClickedRefreshButton()
{
	CWaitCursor wc;
	m_mapCheckedRows.RemoveAll();

	try {
		NexTech_Accessor::_LoadNewOnlineVisitsResultPtr pOnlineVisitsResult(__uuidof(NexTech_Accessor::LoadNewOnlineVisitsResult));
		pOnlineVisitsResult = GetAPI()->LoadNewOnlineVisits(GetAPISubkey(), GetAPILoginToken());
		if (pOnlineVisitsResult != NULL) {
			PopulateVisitsListWithResults(pOnlineVisitsResult->Visits);
		}
		EnableImportButton();
	}NxCatchAll(__FUNCTION__);
}

void COnlineVisitsDlg::OnLeftClickOnlineVisitsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		switch (nCol) {
			case OnlineVisitColumns::Preview:
				{
					// (r.farnworth 2016-03-07 07:27) - PLID 68396 - The user needs to be able to display these files
					if (pRow->GetValue(OnlineVisitColumns::Preview) == _variant_t(long(m_hMagnifyingGlass)))
					{
						DisplayPDF(pRow);
					}
				}
				break;
			case OnlineVisitColumns::Patient:
				{
					// (r.farnworth 2016-03-08 11:44) - PLID 68400 - If the Patient is not linked, then the user will be able to link that patient to a new or existing one
					if (pRow->GetCellLinkStyle(OnlineVisitColumns::Patient) == dlLinkStyleTrue &&
						pRow->GetCellFormatOverride(OnlineVisitColumns::Patient)->GetFieldType() == NXDATALIST2Lib::cftTextSingleLineLink)
					{
						m_pOnlineVisits->PutCurSel(pRow);
						SpawnAssignmentDlg(pRow);
					}
				}
				break;
			default:
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

void COnlineVisitsDlg::EditingFinishedOnlineVistsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		bool bExists;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		switch (nCol)
		{
			case OnlineVisitColumns::Checkbox:
			{
				// (r.farnworth 2016-03-08 16:36) - PLID 68401 - Only enable the Import button if there is at least one row checked
				int nRowID = VarLong(pRow->GetValue(OnlineVisitColumns::ID));

				if (VarBool(pRow->GetValue(OnlineVisitColumns::Checkbox), FALSE))
				{
					m_mapCheckedRows.SetAt(nRowID, true);
					GetDlgItem(IDC_IMPORT_BUTTON)->EnableWindow(TRUE);
				}
				else if (m_mapCheckedRows.Lookup(nRowID, bExists))
				{
					m_mapCheckedRows.RemoveKey(nRowID);
					EnableImportButton();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-02 15:14) - PLID 68455 - We need to filter the Online Visits list by provider
void COnlineVisitsDlg::OnSelChosenProviderCombo(LPDISPATCH lpRow)
{
	try 
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			FilterVisitsList();
			EnableImportButton();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-02 15:14) - PLID 68455 - We need to filter the Online Visits list by provider
void COnlineVisitsDlg::OnSelChangingProviderCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Check your pointer...
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);
		if (!pRow) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}


// (r.farnworth 2016-03-02 15:14) - PLID 68455 - We need to filter the Online Visits list by provider
void COnlineVisitsDlg::FilterVisitsList()
{
	long nDropdownID;
	CString strDropdownProvider, strDatalistProvider;
	NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_dlProviderCombo->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pVisitRow = m_pOnlineVisits->GetFirstRow();

	if (pVisitRow && pProviderRow)
	{
		nDropdownID = VarLong(pProviderRow->GetValue(ProviderColumns::ID));
		strDropdownProvider = VarString(pProviderRow->GetValue(ProviderColumns::Name));

		while (pVisitRow != NULL)
		{
			strDatalistProvider = VarString(pVisitRow->GetValue(OnlineVisitColumns::Provider));
			// -1 implies { Any Provider } is selected, in which case we show everything
			pVisitRow->PutVisible(((strDropdownProvider == strDatalistProvider) || nDropdownID == -1) ? VARIANT_TRUE : VARIANT_FALSE);
			pVisitRow = pVisitRow->GetNextRow();
		}

		//Refresh and snap back to the top of the list
		m_pOnlineVisits->SetRedraw(VARIANT_TRUE);
		m_pOnlineVisits->EnsureRowInView(m_pOnlineVisits->FindAbsoluteFirstRow(VARIANT_TRUE));
	}
}

// (r.farnworth 2016-03-02 15:44) - PLID 68453 - Create a "Configuration" button within the Online Visits tab
void COnlineVisitsDlg::OnBnClickedConfigureButton()
{
	try {
		CSelectDlg dlg(this);
		dlg.m_strTitle = "Select Document Category";
		dlg.m_strCaption = "Please select the default category for documents imported into the patient's history tab";
		dlg.m_strFromClause = "NoteCatsF";
		dlg.m_strWhereClause = "IsPatientTab = 1";
		dlg.AddColumn("ID", "ID", FALSE, FALSE);
		dlg.AddColumn("Description", "Category", TRUE, FALSE, TRUE);
		dlg.m_bAllowNoSelection = true;
		
		dlg.m_varPreSelectedID = GetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>", true);
		dlg.m_nPreSelectColumn = 0;

		if (dlg.DoModal() == IDOK) {
			long nNewID = (dlg.m_arSelectedValues.GetAt(0) == g_cvarNull) ? -1 : dlg.m_arSelectedValues.GetAt(0);
			SetRemotePropertyInt("OnlineVisitsDocumentCategory", nNewID, 0, "<None>");
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-04 15:35) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
void COnlineVisitsDlg::DisplayPDF(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		long nPatientID = VarLong(pRow->GetValue(OnlineVisitColumns::PatientID), -1);;
		CString strFullFileName;

		CFile fOut(FileUtils::CreateTempFileInPath(GetNxTempPath(), "", "pdf", &strFullFileName, FALSE));
		fOut.Close();
		GenerateFile(pRow, strFullFileName);

		if (!strFullFileName.IsEmpty())
		{
			OpenDocument(strFullFileName, nPatientID);
			FileUtils::DeleteFileOnTerm(strFullFileName);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-09 07:51) - PLID 68400 - Implement an auto-link / manual link for patients that are returned for the online visit
void COnlineVisitsDlg::SpawnAssignmentDlg(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		CPatientAssignmentDlg dlg(CPatientAssignmentDlg::AssignmentPurpose::apOnlineVisit, this);
		dlg.Prepopulate(
			VarString(pRow->GetValue(OnlineVisitColumns::FirstName), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::LastName), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::Gender), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::Email), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::Address1), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::Address2), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::City), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::State), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::Zip), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::HomePhone), ""),
			VarString(pRow->GetValue(OnlineVisitColumns::CellPhone), ""),
			VarDateTime(pRow->GetValue(OnlineVisitColumns::Birthdate), dtInvalid)
			);

		if (IDOK == (dlg.DoModal()))
		{
			CWaitCursor wc;
			int visitID = VarLong(pRow->GetValue(OnlineVisitColumns::ID));
			int patientID = dlg.GetAssignedPatientID();

			NexTech_Accessor::_LinkPatientToIagnosisOnlineVisitsResultPtr pOnlineVisitsLinkResult(__uuidof(NexTech_Accessor::LinkPatientToIagnosisOnlineVisitsResult));
			pOnlineVisitsLinkResult = GetAPI()->LinkPatientToIagnosisOnlineVisits(GetAPISubkey(), GetAPILoginToken(), visitID, patientID);
			if (pOnlineVisitsLinkResult != NULL) {
				PopulateVisitsListWithResults(pOnlineVisitsLinkResult->Visits);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-07 08:07) - PLID 68401 - Implement the import feature for when the user wants to pull the online visit into the patients chart
void COnlineVisitsDlg::OnBnClickedImportButton()
{
	if (IDNO == MessageBox("You are about to import all selected documents (regardless of the provider filter) and attach them to their respective patients.\n"
		"Are you sure you wish to continue?", "Practice", MB_YESNO | MB_ICONQUESTION)) {
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pOnlineVisits->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pRemoveRow;
	CString strErr = "";
	bool bOneFailed = false;
	bool bOneImport = false;

	while (pCurrentRow != NULL)
	{
		long nSuccess = -1;
		BOOL bChecked = VarBool(pCurrentRow->GetValue(OnlineVisitColumns::Checkbox), FALSE);
		int nRowID = VarLong(pCurrentRow->GetValue(OnlineVisitColumns::ID));
		CString strFormat = "";

		try
		{
			if(bChecked)
			{
				long nPatientID = VarLong(pCurrentRow->GetValue(OnlineVisitColumns::PatientID), -1);
				CString strFilePath = GetPatientDocumentPath(nPatientID);
				CString strProvider = VarString(pCurrentRow->GetValue(OnlineVisitColumns::Provider));
				CString strDescription = VarString(pCurrentRow->GetValue(OnlineVisitColumns::Description));
				COleDateTime dtCreatedDate = VarDateTime(pCurrentRow->GetValue(OnlineVisitColumns::CreatedDate));
				if (!FileUtils::EnsureDirectory(strFilePath)) 
				{
					strFormat.Format("Could not create or access the folder '%s' \r\n", strFilePath);
					strErr += strFormat;
					bOneFailed = true;
					pCurrentRow = pCurrentRow->GetNextRow();
					continue;
				}

				CString strFullFileName = strFilePath ^  GetPatientDocumentName(nPatientID, "pdf");
				GenerateFile(pCurrentRow, strFullFileName);
				nSuccess = AttachToHistory(strFullFileName, strProvider, strDescription, dtCreatedDate, nPatientID, nRowID);
				if (nSuccess != -1)
				{
					bOneImport = true;
					pRemoveRow = pCurrentRow;
					pCurrentRow = pCurrentRow->GetNextRow();
					m_pOnlineVisits->RemoveRow(pRemoveRow);
					m_mapCheckedRows.RemoveKey(nRowID);
				}
			}
		} NxCatchAllCall(__FUNCTION__, 
			{
				// (r.farnworth 2016-03-07 08:07) - PLID 68401 - Need error handling for the scenario where the patient has been deleted from Practice
				strFormat.Format("Error attempting to attach document for patient %s %s. It is possible that they no longer exist. \r\n",
					VarString(pCurrentRow->GetValue(OnlineVisitColumns::FirstName)), VarString(pCurrentRow->GetValue(OnlineVisitColumns::LastName)));
				strErr += strFormat;
				bOneFailed = true;
				pCurrentRow->PutValue(OnlineVisitColumns::Checkbox, g_cvarFalse);
				m_mapCheckedRows.RemoveKey(nRowID);
			});

		if (nSuccess == -1)
		{
			pCurrentRow = pCurrentRow->GetNextRow();
		}
	}

	CString strFinalErr;
	if (bOneFailed && bOneImport)
	{
		strFinalErr.Format("Some documents could not be successfully imported for the following reasons. Check your log files for more details.\r\n\r\n%s", strErr);
		AfxMessageBox(strFinalErr, MB_OK | MB_ICONSTOP);
	}
	else if (!bOneFailed && bOneImport)
	{
		AfxMessageBox("All documents have been successfully imported.");
	}
	else if (bOneFailed && !bOneImport)
	{
		strFinalErr.Format("None of the documents could be successfully imported for the following reasons. Check your log files for more details.\r\n\r\n%s", strErr);
		AfxMessageBox(strFinalErr, MB_OK | MB_ICONSTOP);
	}

	PopulateProviderDropdown();
	EnableImportButton();
}

// (r.farnworth 2016-03-07 08:38) - PLID 68401 - Break file creation out into its own function
void COnlineVisitsDlg::GenerateFile(NXDATALIST2Lib::IRowSettingsPtr pRow, CString strFullFileName)
{
	CFileException fe;
	CFile fOut;
	if (!fOut.Open(strFullFileName, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive, &fe)) {
		MessageBox(CString("Error opening output file for writing - ") + FormatLastError(), "Practice", MB_OK | MB_ICONSTOP);
		return;
	}

	CString strPDF = VarString(pRow->GetValue(OnlineVisitColumns::PDF));
	strPDF.Replace("\r\n", "");
	strPDF.Replace("\r", "");
	strPDF.Replace("\n", "");
	strPDF.Replace(" ", "");
	strPDF.Replace("\t", "");
	BYTE *pBytes = NULL;
	int nLength = -1;

	CBase64::Decode(strPDF, pBytes, nLength);
	fOut.Write(pBytes, nLength);
	fOut.Close();

	delete[] pBytes;
}

// (r.farnworth 2016-03-07 09:24) - PLID 68401 - Attach the file to the patient's history document
long COnlineVisitsDlg::AttachToHistory(const CString &strFilePath, const CString &strProvider, const CString &strDescription, const COleDateTime &dtCreated, long nPatientID, int nRowID)
{
	COleDateTime dtVisitDate;
	CString strFinalDesc = "Online Visit - " + strDescription + " - Provider: " + strProvider;
	long nCategoryID = GetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>");

	return CreateNewMailSentEntry(nPatientID, strFinalDesc, SELECTION_FILE, strFilePath, GetCurrentUserName(), "", GetCurrentLocationID(), dtCreated,
			-1, nCategoryID, -1, -1, FALSE, -1, "", ctNone, nRowID);
}

void COnlineVisitsDlg::PopulateVisitsListWithResults(NexTech_Accessor::_OnlineVisitsPtr pOnlineVisits)
{
	try {
		// (r.farnworth 2016-03-02 15:14) - PLID 68455 - Reset to { Any Provider } any time the refresh
		m_dlProviderCombo->SetSelByColumn(ProviderColumns::ID, -1);
		m_pOnlineVisits->Clear();

		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

		Nx::SafeArray<IUnknown*> saResults = NULL;

		if (pOnlineVisits != NULL && pOnlineVisits->Visits != NULL) {
			saResults = Nx::SafeArray<IUnknown*>(pOnlineVisits->Visits);

			for each (NexTech_Accessor::_OnlineVisitPtr pVisit in saResults)
			{
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pOnlineVisits->GetNewRow();

				pNewRow->PutValue(OnlineVisitColumns::ID, _variant_t(pVisit->ID));
				pNewRow->PutValue(OnlineVisitColumns::PatientID, VarLong(pVisit->patientPersonID->GetValue(), -1));
				pNewRow->PutValue(OnlineVisitColumns::UID, _variant_t(pVisit->Patient->PatientUID));
				pNewRow->PutValue(OnlineVisitColumns::IagnosisPatientID, _variant_t(pVisit->IagnosisPatientID));
				pNewRow->PutValue(OnlineVisitColumns::FirstName, _variant_t(pVisit->Patient->FirstName));
				pNewRow->PutValue(OnlineVisitColumns::LastName, _variant_t(pVisit->Patient->LastName));
				pNewRow->PutValue(OnlineVisitColumns::Email, _variant_t(pVisit->Patient->Email));
				pNewRow->PutValue(OnlineVisitColumns::Address1, _variant_t(pVisit->Patient->Address1));
				pNewRow->PutValue(OnlineVisitColumns::Address2, _variant_t(pVisit->Patient->Address2));
				pNewRow->PutValue(OnlineVisitColumns::City, _variant_t(pVisit->Patient->City));
				pNewRow->PutValue(OnlineVisitColumns::State, _variant_t(pVisit->Patient->State));
				pNewRow->PutValue(OnlineVisitColumns::Zip, _variant_t(pVisit->Patient->Zip));
				pNewRow->PutValue(OnlineVisitColumns::HomePhone, _variant_t(pVisit->Patient->HomePhone));
				pNewRow->PutValue(OnlineVisitColumns::CellPhone, _variant_t(pVisit->Patient->CellPhone));
				pNewRow->PutValue(OnlineVisitColumns::Gender, _variant_t(pVisit->Patient->Gender));
				pNewRow->PutValue(OnlineVisitColumns::Birthdate, ValidateVarDate(pVisit->Patient->Birthdate->GetValue()));
				pNewRow->PutValue(OnlineVisitColumns::Provider, _variant_t(pVisit->Provider));
				pNewRow->PutValue(OnlineVisitColumns::Description, _variant_t(pVisit->description));
				pNewRow->PutValue(OnlineVisitColumns::CreatedDate, VariantDate(pVisit->CreatedDate));
				pNewRow->PutValue(OnlineVisitColumns::ModifiedDate, VariantDate(pVisit->ModifiedDate));
				pNewRow->PutValue(OnlineVisitColumns::PDF, variant_t(pVisit->PDF));

				// (r.farnworth 2016-03-15 09:10) - PLID 68401 - Only allow the user to import documents if they have the permission to write to patient history
				bool bIncludeCheck = m_bHistoryWritePerm;
				int nRowID = VarLong(_variant_t(pVisit->ID));
				bool bExists;

				if (pVisit->Patient->PatientUID.length() != 0 && pVisit->patientPersonID->IsNull() == VARIANT_FALSE)
				{
					//autolink and enable import checkbox
					pNewRow->PutValue(OnlineVisitColumns::Patient, _variant_t("Autolinked"));
				}
				else if (pVisit->Patient->PatientUID.length() == 0 && pVisit->patientPersonID->IsNull() == VARIANT_FALSE)
				{
					//Link and enable import checkbox
					pNewRow->PutValue(OnlineVisitColumns::Patient, _variant_t("Linked"));
				}
				else if ((pVisit->Patient->PatientUID.length() != 0 && pVisit->patientPersonID->IsNull() == VARIANT_TRUE)
					|| (pVisit->Patient->PatientUID.length() == 0 && pVisit->patientPersonID->IsNull() == VARIANT_TRUE))
				{
					//hyperlink, no checkbox, hyperlink the cell, and color it blue
					pNewRow->PutValue(OnlineVisitColumns::Patient, _variant_t("Choose Patient"));
					pNewRow->PutCellLinkStyle(OnlineVisitColumns::Patient, dlLinkStyleTrue);
					pNewRow->PutRefCellFormatOverride(OnlineVisitColumns::Patient, pHyperLink);
					pNewRow->PutCellForeColor(OnlineVisitColumns::Patient, RGB(0, 0, 200));
					bIncludeCheck = false;
					if (m_mapCheckedRows.Lookup(nRowID, bExists)) {
						m_mapCheckedRows.RemoveKey(nRowID);
					}
				}

				if (pVisit->PDF.length() != 0)
				{
					pNewRow->PutValue(OnlineVisitColumns::Preview, _variant_t(long(m_hMagnifyingGlass)));
				}
				else
				{
					bIncludeCheck = false;
				}

				pNewRow->PutValue(OnlineVisitColumns::Checkbox, ((bIncludeCheck) ? ((m_mapCheckedRows.Lookup(nRowID, bExists)) ? g_cvarTrue : g_cvarFalse) :  g_cvarNull));
				
				m_pOnlineVisits->AddRowSorted(pNewRow, NULL);
			}
		}

		PopulateProviderDropdown(); // (r.farnworth 2016-03-09 12:45) - PLID 68455 - Populate the Dropdown
		FilterVisitsList(); 		// (r.farnworth 2016-03-02 15:14) - PLID 68455 - Refresh the list
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-03-08 16:45) - PLID 68401 - Only display the import button if at least one row is checked
void COnlineVisitsDlg::EnableImportButton()
{

	bool bExists;
	int nVisitID;
	POSITION pos = m_mapCheckedRows.GetStartPosition();
	NXDATALIST2Lib::IRowSettingsPtr pNewRow;

	for (;;)
	{
		if (pos == NULL) break;
		m_mapCheckedRows.GetNextAssoc(pos, nVisitID, bExists);

		if (bExists)
		{
			pNewRow = m_pOnlineVisits->FindByColumn(OnlineVisitColumns::ID, _variant_t(nVisitID), NULL, FALSE);
			if (pNewRow->Visible == VARIANT_TRUE)
			{
				GetDlgItem(IDC_IMPORT_BUTTON)->EnableWindow(TRUE);
				return;
			}
		}
	}

	GetDlgItem(IDC_IMPORT_BUTTON)->EnableWindow(FALSE);
}

// (r.farnworth 2016-03-09 12:11) - PLID 68455
void COnlineVisitsDlg::PopulateProviderDropdown()
{
	NXDATALIST2Lib::IRowSettingsPtr pVisitRow;
	CString strVisitProvider;
	std::set<CString> setProviders;

	pVisitRow = m_pOnlineVisits->GetFirstRow();
	while (pVisitRow != NULL)
	{
		strVisitProvider = VarString(pVisitRow->GetValue(OnlineVisitColumns::Provider));
		setProviders.insert(strVisitProvider);
		pVisitRow = pVisitRow->GetNextRow();
	}

	m_dlProviderCombo->Clear();
	long nProviderID = 0;

	for (auto val : setProviders) {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlProviderCombo->GetNewRow();
		pNewRow->PutValue(ProviderColumns::ID, _variant_t(nProviderID++));
		pNewRow->PutValue(ProviderColumns::Name, _variant_t(val));
		m_dlProviderCombo->AddRowSorted(pNewRow, NULL);
	}

	if (m_dlProviderCombo->GetRowCount() > 0)
	{
		NXDATALIST2Lib::IRowSettingsPtr pDefaultRow = m_dlProviderCombo->GetNewRow();
		pDefaultRow->PutValue(ProviderColumns::ID, _variant_t((long)-1));
		pDefaultRow->PutValue(ProviderColumns::Name, _variant_t(" { Any Provider } "));
		m_dlProviderCombo->AddRowSorted(pDefaultRow, NULL);
		m_dlProviderCombo->SetSelByColumn(ProviderColumns::ID, -1);

		GetDlgItem(IDC_PROVIDER_COMBO)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_PROVIDER_COMBO)->EnableWindow(FALSE);
	}	
}

_variant_t COnlineVisitsDlg::ValidateVarDate(_variant_t vtDate)
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);

	if (vtDate == VariantDate(GetSqlDateTimeZero())) {
		vtDate = VariantDate(dtInvalid);
	}

	return vtDate;
}