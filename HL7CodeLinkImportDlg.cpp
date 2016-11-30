// (r.gonet 09/27/2011) - PLID 45719 - Added
// HL7CodeLinkImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "HL7CodeLinkImportDlg.h"
#include <NxPracticeSharedLib\CSVUtils.h>		// (d.lange 2016-01-11 10:54) - PLID 67829 - Moved to NxPracticeSharedLib


// CHL7CodeLinkImportDlg dialog

IMPLEMENT_DYNAMIC(CHL7CodeLinkImportDlg, CNxDialog)

CHL7CodeLinkImportDlg::CHL7CodeLinkImportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7CodeLinkImportDlg::IDD, pParent)
{
	m_n3rdPartyCodeColumn = -1;
	m_nPracticeIDColumn = -1;
}

CHL7CodeLinkImportDlg::~CHL7CodeLinkImportDlg()
{
}

void CHL7CodeLinkImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HL7_CODE_IMPORT_COLOR, m_nxcColor);
	DDX_Control(pDX, IDC_HL7_CODE_IMPORT_HEADER_LBL, m_nxsFilePath);
	DDX_Control(pDX, IDC_HL7_CODE_IMPORT_PATH_EDIT, m_nxeImportFilePath);
	DDX_Control(pDX, IDC_HL7_CODE_IMPORT_BROWSE_BTN, m_btnBrowse);
	DDX_Control(pDX, IDC_HL7_HAS_HEADER_CHECKBOX, m_checkHasFieldNamesInFirstRow);
	DDX_Control(pDX, IDC_HL7_CODE_IMPORT_LOAD_BTN, m_btnLoad);
	DDX_Control(pDX, IDC_HL7_3RD_PARTY_CODE_LBL, m_nxs3rdPartyCode);
	DDX_Control(pDX, IDC_HL7_PRACTICE_ID_LBL, m_nxsPracticeID);
	DDX_Control(pDX, IDOK, m_btnImport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CHL7CodeLinkImportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_HL7_CODE_IMPORT_BROWSE_BTN, &CHL7CodeLinkImportDlg::OnBnClickedHl7CodeImportBrowseBtn)
	ON_BN_CLICKED(IDC_HL7_CODE_IMPORT_LOAD_BTN, &CHL7CodeLinkImportDlg::OnBnClickedHl7CodeImportLoadBtn)
	ON_BN_CLICKED(IDOK, &CHL7CodeLinkImportDlg::OnBnClickedImport)
END_MESSAGE_MAP()


// CHL7CodeLinkImportDlg message handlers

// (r.gonet 09/27/2011) - PLID 45719 - Go get a CSV file from explorer
void CHL7CodeLinkImportDlg::OnBnClickedHl7CodeImportBrowseBtn()
{
	try {
		CFileDialog fd(TRUE, "csv", "", OFN_EXPLORER|OFN_FILEMUSTEXIST, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*)|*.*||", this);
		if (fd.DoModal() == IDOK) {
			CString strFilePath = fd.GetPathName();
			m_nxeImportFilePath.SetWindowText(strFilePath);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45719 - Load the chosen CSV file into memory.
void CHL7CodeLinkImportDlg::OnBnClickedHl7CodeImportLoadBtn()
{
	CFile fInputFile;
	
	try {
		// (r.gonet 09/27/2011) - PLID 45719 - Reset everything.
		if(m_pcsvRecordSet != NULL) {
			m_pcsvRecordSet = boost::shared_ptr<CCSVRecordSet>();
		}
		m_p3rdPartyCodeCombo->Clear();
		m_pPracticeIDCombo->Clear();
		m_p3rdPartyCodeCombo->Enabled = VARIANT_FALSE;
		m_pPracticeIDCombo->Enabled = VARIANT_FALSE;
		m_btnImport.EnableWindow(FALSE);

		CString strFilePath;
		m_nxeImportFilePath.GetWindowText(strFilePath);
		// Test the file out first to see if it exists.
		if(!fInputFile.Open(strFilePath, CFile::modeRead | CFile::shareCompat)) {
			MessageBox("Could not open the import file for reading.", "Error", MB_ICONERROR);
			return;
		}
		fInputFile.Close();

		// This may take a while to parse, so have the user wait.
		CWaitCursor pWait;

		// Parse the file and get the parsed results.
		CCSVParser parser;
		if(!(m_pcsvRecordSet = boost::shared_ptr<CCSVRecordSet>(parser.Parse(strFilePath, m_checkHasFieldNamesInFirstRow.GetCheck() != 0)))) {
			// Oh snap. The file failed to parse!
			CString strErrorMessage = parser.GetLastParseError();
			MessageBox("File is not in a valid CSV format.\r\n" + strErrorMessage, "Format Error", MB_ICONERROR);
			return;
		}
		
		// Put some default values for the two field selection combos
		NXDATALIST2Lib::IRowSettingsPtr pDefaultRow = m_p3rdPartyCodeCombo->GetNewRow();
		pDefaultRow->PutValue(etpccID, _variant_t((long)-1, VT_I4));
		pDefaultRow->PutValue(etpccName, _variant_t(" < Select a Field >"));
		m_p3rdPartyCodeCombo->AddRowAtEnd(pDefaultRow, NULL);

		pDefaultRow = m_pPracticeIDCombo->GetNewRow();
		pDefaultRow->PutValue(epicID, _variant_t((long)-1, VT_I4));
		pDefaultRow->PutValue(epicName, _variant_t(" < Select a Field >"));
		m_pPracticeIDCombo->AddRowAtEnd(pDefaultRow, NULL);
		
		// Now add in all the parsed field names.
		for(int i = 0; i < m_pcsvRecordSet->GetFieldCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_p3rdPartyCodeCombo->GetNewRow();
			pRow->PutValue(etpccID, _variant_t((long)i, VT_I4));
			pRow->PutValue(etpccName, _variant_t(m_pcsvRecordSet->GetFieldName(i)));
			m_p3rdPartyCodeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_pPracticeIDCombo->GetNewRow();
			pRow->PutValue(epicID, _variant_t((long)i, VT_I4));
			pRow->PutValue(etpccName, _variant_t(m_pcsvRecordSet->GetFieldName(i)));
			m_pPracticeIDCombo->AddRowAtEnd(pRow, NULL);
		}

		// Select the defaults and then let the user choose which fields they want to use as the import fields.
		m_p3rdPartyCodeCombo->SetSelByColumn(etpccID, _variant_t((long)-1, VT_I4));
		m_pPracticeIDCombo->SetSelByColumn(epicID, _variant_t((long)-1, VT_I4));
		m_p3rdPartyCodeCombo->Enabled = VARIANT_TRUE;
		m_pPracticeIDCombo->Enabled = VARIANT_TRUE;
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CHL7CodeLinkImportDlg, CNxDialog)
	ON_EVENT(CHL7CodeLinkImportDlg, IDC_HL7_3RD_PARTY_CODE_COMBO, 16, CHL7CodeLinkImportDlg::SelChosenHl73rdPartyIdCombo, VTS_DISPATCH)
	ON_EVENT(CHL7CodeLinkImportDlg, IDC_HL7_3RD_PARTY_CODE_COMBO, 1, CHL7CodeLinkImportDlg::SelChangingHl73rdPartyIdCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CHL7CodeLinkImportDlg, IDC_HL7_PRACTICE_ID_COMBO, 16, CHL7CodeLinkImportDlg::SelChosenHl7PracticeIdCombo, VTS_DISPATCH)
	ON_EVENT(CHL7CodeLinkImportDlg, IDC_HL7_PRACTICE_ID_COMBO, 1, CHL7CodeLinkImportDlg::SelChangingHl7PracticeIdCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 09/27/2011) - PLID 45719 - The user chose one field, should we let them import yet?
void CHL7CodeLinkImportDlg::SelChosenHl73rdPartyIdCombo(LPDISPATCH lpRow)
{
	try {
		// We need both fields selected.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pPracticeIDRow = m_pPracticeIDCombo->CurSel;
		if(pRow && VarLong(pRow->GetValue(etpccID)) != -1 &&
			pPracticeIDRow && VarLong(pPracticeIDRow->GetValue(epicID)) != -1) 
		{
			m_btnImport.EnableWindow(TRUE);
		} else {
			m_btnImport.EnableWindow(FALSE);
		}
		m_n3rdPartyCodeColumn = VarLong(pRow->GetValue(etpccID));
	} NxCatchAll(__FUNCTION__);
}

void CHL7CodeLinkImportDlg::SelChangingHl73rdPartyIdCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45719 - The user chose one field, should we let them import yet?
void CHL7CodeLinkImportDlg::SelChosenHl7PracticeIdCombo(LPDISPATCH lpRow)
{
	try {
		// We need both fields selected.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr p3rdPartyCodeRow = m_p3rdPartyCodeCombo->CurSel;
		if(pRow && VarLong(pRow->GetValue(epicID)) != -1 &&
			p3rdPartyCodeRow && VarLong(p3rdPartyCodeRow->GetValue(etpccID)) != -1) 
		{
			m_btnImport.EnableWindow(TRUE);
		} else {
			m_btnImport.EnableWindow(FALSE);
		}
		m_nPracticeIDColumn = VarLong(pRow->GetValue(epicID));
	} NxCatchAll(__FUNCTION__);
}

void CHL7CodeLinkImportDlg::SelChangingHl7PracticeIdCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

void CHL7CodeLinkImportDlg::OnBnClickedImport()
{
	try {
		// The caller must actually do the work
		OnOK();
	} NxCatchAll(__FUNCTION__);
}

BOOL CHL7CodeLinkImportDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_p3rdPartyCodeCombo = BindNxDataList2Ctrl(this, IDC_HL7_3RD_PARTY_CODE_COMBO, GetRemoteData(), false);
		m_pPracticeIDCombo = BindNxDataList2Ctrl(this, IDC_HL7_PRACTICE_ID_COMBO, GetRemoteData(), false);
		m_btnLoad.AutoSet(NXB_REFRESH);
		m_btnImport.AutoSet(NXB_IMPORTBOX);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_p3rdPartyCodeCombo->Enabled = VARIANT_FALSE;
		m_pPracticeIDCombo->Enabled = VARIANT_FALSE;
		m_btnImport.EnableWindow(FALSE);
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 09/27/2011) - PLID 45719 - Get the CSV column that the user selected to contain the third party code
long CHL7CodeLinkImportDlg::GetThirdPartyCodeColumn()
{
	return m_n3rdPartyCodeColumn;
}

// (r.gonet 09/27/2011) - PLID 45719 - Get the CSV column that the user selected to contain the Practice ID code
long CHL7CodeLinkImportDlg::GetPracticeIDColumn()
{
	return m_nPracticeIDColumn;
}

// (r.gonet 09/27/2011) - PLID 45719 - Get the parsed CSV file.
boost::shared_ptr<CCSVRecordSet> CHL7CodeLinkImportDlg::GetCSVRecordSet()
{
	return m_pcsvRecordSet;
}