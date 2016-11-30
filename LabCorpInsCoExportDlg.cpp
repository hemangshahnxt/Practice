// (r.gonet 09/27/2011) - PLID 45717 - Added

// LabCorpInsCoExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabCorpInsCoExportDlg.h"

using namespace ADODB;

// CLabCorpInsCoExportDlg dialog

IMPLEMENT_DYNAMIC(CLabCorpInsCoExportDlg, CNxDialog)

CLabCorpInsCoExportDlg::CLabCorpInsCoExportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabCorpInsCoExportDlg::IDD, pParent)
{
	m_strFilePath = "";
	m_strFolderPath = "";
}

CLabCorpInsCoExportDlg::~CLabCorpInsCoExportDlg()
{
}

void CLabCorpInsCoExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LABCORP_EXPORT_COLOR, m_nxcColor);
	DDX_Control(pDX, IDC_LABCORP_EXPORT_HL7GROUP_LBL, m_nxsHeader);
	DDX_Control(pDX, IDC_LABCORP_EXPORT_FILTER_LBL, m_nxsExportFilter);
	DDX_Control(pDX, IDC_LABCORP_EXPORT_ALL, m_radioAllCompanies);
	DDX_Control(pDX, IDC_LABCORP_EXPORT_ONLY_NEW, m_radioCompaniesWithoutCode);
	DDX_Control(pDX, IDOK, m_nxbExport);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CLabCorpInsCoExportDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CLabCorpInsCoExportDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CLabCorpInsCoExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pGroups = BindNxDataList2Ctrl(this, IDC_LABCORP_EXPORT_HL7GROUPS_LIST, GetRemoteData(), true);
		m_pGroups->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_nxbExport.AutoSet(NXB_EXPORT);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		// (r.gonet 09/27/2011) - PLID 45717 - Add a default row so null is never selected
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroups->GetNewRow();
		pRow->PutValue(elcgcID, _variant_t((long)-1, VT_I4));
		pRow->PutValue(elcgcName, _variant_t(" < Select an HL7 Settings Group >"));
		m_pGroups->AddRowBefore(pRow, m_pGroups->FindAbsoluteFirstRow(VARIANT_TRUE));
		m_pGroups->SetSelByColumn(elcgcID, _variant_t((long)-1, VT_I4));
		m_radioCompaniesWithoutCode.SetCheck(TRUE);
		m_nxbExport.EnableWindow(FALSE);

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// CLabCorpInsCoExportDlg message handlers

// (r.gonet 09/27/2011) - PLID 45717 - Export and close the dialog.
void CLabCorpInsCoExportDlg::OnBnClickedOk()
{
	CFile fOutputFile;
	bool bOutputFileOpen = false;
	try {
		// get the hl7settings group we want to export for
		long nHL7GroupID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroups->CurSel;
		if(!pRow || (nHL7GroupID = VarLong(pRow->GetValue(elcgcID))) == -1) {
			MessageBox("Please select an HL7 Settings Group before exporting.");
			return;
		}

		//first get the file
		CFileDialog dlgSaveFile(FALSE, "csv", "LabCorp_InsuranceExtract.csv", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*||*.*||");
		if (dlgSaveFile.DoModal() == IDCANCEL) {
			return;
		}
		m_strFilePath = dlgSaveFile.GetPathName();
		m_strFolderPath = dlgSaveFile.GetFolderPath();
		
		//open the file for reading
		if(!fOutputFile.Open(m_strFilePath,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			MessageBox("The output file could not be opened for writing. Ensure this file is not already open by another program.", "Error", MB_ICONERROR);
			return;
		}
		bOutputFileOpen = true;

		CWaitCursor pWait;

		// (r.gonet 09/27/2011) - PLID 45717 - Write the header with all of the field names.
		CString strHeaderLine = "labcorp_code,PAYER_ID,PAYER_NAME,ADDRESS_LINE_1,ADDRESS_LINE_2,CITY,STATE,ZIP\r\n";
		fOutputFile.Write(strHeaderLine, strHeaderLine.GetLength());

		// I thought about the possibility of there being multiple third party codes for each practiceid, but I think it best to give
		//  labcorp the whole list and have them handle it.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT COALESCE(HL7CodeLinkT.ThirdPartyCode, '') AS labcorp_code, PersonT.ID AS PAYER_ID, InsuranceCoT.Name AS PAYER_NAME, "
			"	PersonT.Address1 AS ADDRESS_LINE_1, PersonT.Address2 AS ADDRESS_LINE_2, "
			"	PersonT.City AS CITY, PersonT.State AS STATE, REPLACE(PersonT.Zip, '-', '') AS ZIP "
			"FROM InsuranceCoT "
			"	JOIN "
			"PersonT ON InsuranceCoT.PersonID = PersonT.ID "
			"	LEFT JOIN "
			"HL7CodeLinkT ON PersonT.ID = HL7CodeLinkT.PracticeID AND HL7CodeLinkT.Type = 5 AND HL7CodeLinkT.HL7GroupID = {INT}; ",
			nHL7GroupID);

		// Write each record
		while(!prs->eof) {
			CString strLabCorpCode = VarString(prs->Fields->Item["labcorp_code"]->Value);
			long nPayerID = VarLong(prs->Fields->Item["PAYER_ID"]->Value);
			CString strPayerName = VarString(prs->Fields->Item["PAYER_NAME"]->Value);
			CString strAddress1 = VarString(prs->Fields->Item["ADDRESS_LINE_1"]->Value);
			CString strAddress2 = VarString(prs->Fields->Item["ADDRESS_LINE_2"]->Value);
			CString strCity = VarString(prs->Fields->Item["CITY"]->Value);
			CString strState = VarString(prs->Fields->Item["STATE"]->Value);
			CString strZip = VarString(prs->Fields->Item["ZIP"]->Value);

			if(m_radioAllCompanies.GetCheck() || (m_radioCompaniesWithoutCode.GetCheck() && strLabCorpCode.IsEmpty())) {
				CString strLine = FormatString("%s,%li,%s,%s,%s,%s,%s,%s\r\n",
					EscapeCSV(strLabCorpCode), nPayerID, EscapeCSV(strPayerName), 
					EscapeCSV(strAddress1), EscapeCSV(strAddress2), 
					EscapeCSV(strCity), EscapeCSV(strState), EscapeCSV(strZip));
				fOutputFile.Write(strLine, strLine.GetLength());
			}

			prs->MoveNext();
		}
	} NxCatchAll(__FUNCTION__);

	if(bOutputFileOpen) {
		fOutputFile.Close();
	}

	OnOK();
}

// (r.gonet 09/27/2011) - PLID 45717 - The value may need to be escaped within quotes if it contains special characters.
CString CLabCorpInsCoExportDlg::EscapeCSV(CString strValue)
{
	bool bAddQuotes = false;
	if(strValue.Find("\"") != -1 || strValue.Find(",") != -1 || strValue.Find("\n") != -1) {
		bAddQuotes = true;
		strValue.Replace("\"", "\"\"");
		strValue = "\"" + strValue + "\"";
	}
	return strValue;
}

BEGIN_EVENTSINK_MAP(CLabCorpInsCoExportDlg, CNxDialog)
ON_EVENT(CLabCorpInsCoExportDlg, IDC_LABCORP_EXPORT_HL7GROUPS_LIST, 16, CLabCorpInsCoExportDlg::SelChosenLabcorpExportHl7groupsList, VTS_DISPATCH)
ON_EVENT(CLabCorpInsCoExportDlg, IDC_LABCORP_EXPORT_HL7GROUPS_LIST, 1, CLabCorpInsCoExportDlg::SelChangingLabcorpExportHl7groupsList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 09/27/2011) - PLID 45717 - User selected an hl7 group, so we can let them pass now.
void CLabCorpInsCoExportDlg::SelChosenLabcorpExportHl7groupsList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow && VarLong(pRow->GetValue(elcgcID)) != -1) {
			m_nxbExport.EnableWindow(TRUE);
		} else {
			m_nxbExport.EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45717 - Don't let the user select an invalid HL7 group
void CLabCorpInsCoExportDlg::SelChangingLabcorpExportHl7groupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45717 - Gets the folder that the export file was saved to.
CString CLabCorpInsCoExportDlg::GetExportFolderPath()
{
	return m_strFolderPath;
}

// (r.gonet 09/27/2011) - PLID 45717 - Gets the file that was exported to
CString CLabCorpInsCoExportDlg::GetExportFilePath()
{
	return m_strFilePath;
}