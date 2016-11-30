// HL7IDLinkExportDlg.cpp : implementation file
//

// (j.dinatale 2013-01-14 11:51) - PLID 54602 - created

#include "stdafx.h"
#include "Practice.h"
#include "HL7IDLinkExportDlg.h"

namespace HL7IDExportList {
	enum HL7IDExportColumns{
		ID = 0,
		Name,
	};
};

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CHL7IDLinkExportDlg dialog

IMPLEMENT_DYNAMIC(CHL7IDLinkExportDlg, CNxDialog)

CHL7IDLinkExportDlg::CHL7IDLinkExportDlg(HL7IDLink_RecordType hl7IDLinkType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7IDLinkExportDlg::IDD, pParent)
{
	m_IDLinkRecordType = hl7IDLinkType;
}

CHL7IDLinkExportDlg::~CHL7IDLinkExportDlg()
{
}

void CHL7IDLinkExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_nxbExport);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CHL7IDLinkExportDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CHL7IDLinkExportDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CHL7IDLinkExportDlg message handlers
BOOL CHL7IDLinkExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pGroupList = BindNxDataList2Ctrl(IDC_HL7ID_EXPORT_HL7GROUPS_LIST, true);
		m_nxbExport.AutoSet(NXB_EXPORT);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		m_nxbExport.EnableWindow(FALSE);
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CHL7IDLinkExportDlg::GetFilePath()
{
	return m_strFilePath;
}

CString CHL7IDLinkExportDlg::GetTableName(HL7IDLink_RecordType hilrt)
{
	switch(m_IDLinkRecordType) {
		case hilrtPatient:
			return "PatientsT";
		case hilrtProvider:
			return "ProvidersT";
			break;
		case hilrtReferringPhysician:
			return "ReferringPhysT";
			break;
		default:
			ASSERT(FALSE); // we added a type and didnt account for it
			ThrowNxException("Unsupported HL7 ID Link Record Type.");
	}

	return "";
}

CString CHL7IDLinkExportDlg::GetHL7IDLinkRecordName(HL7IDLink_RecordType hilrt)
{
	switch(hilrt) {
		case hilrtPatient:
			return "Patient";
			break;
		case hilrtProvider:
			return "Provider";
			break;
		case hilrtReferringPhysician:
			return "Referring Physician";
			break;
		default:
			ASSERT(FALSE);	// added a new type, need to support it
	}

	return "";
}

CString CHL7IDLinkExportDlg::EscapeCSV(CString strValue)
{
	bool bAddQuotes = false;
	if(strValue.Find("\"") != -1 || strValue.Find(",") != -1 || strValue.Find("\n") != -1) {
		bAddQuotes = true;
		strValue.Replace("\"", "\"\"");
		strValue = "\"" + strValue + "\"";
	}
	return strValue;
}

void CHL7IDLinkExportDlg::OnBnClickedOk()
{
	CFile fOutputFile;
	bool bOutputFileOpen = false;
	try {
		// get the hl7settings group we want to export for
		long nHL7GroupID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupList->CurSel;
		if(!pRow || (nHL7GroupID = VarLong(pRow->GetValue(HL7IDExportList::ID))) == -1) {
			MessageBox("Please select an HL7 Settings Group before exporting.");
			return;
		}

		CString strRecordName = GetHL7IDLinkRecordName(m_IDLinkRecordType);
		CString strTableName = GetTableName(m_IDLinkRecordType);

		{
			CString strFileName, strName = strRecordName;
			strName.Replace(" ", "");
			strFileName.Format("HL7%sExtract.csv", strName);

			//first get the file
			CFileDialog dlgSaveFile(FALSE, "csv",  FileUtils::GetFilePath(GetModuleFileName()) ^ strFileName, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*)|*.*||");
			if (dlgSaveFile.DoModal() == IDCANCEL) {
				return;
			}

			m_strFilePath = dlgSaveFile.GetPathName();
		}
		
		//open the file for reading
		if(!fOutputFile.Open(m_strFilePath,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			MessageBox("The output file could not be opened for writing. Ensure this file is not already open by another program.", "Error", MB_ICONERROR);
			return;
		}

		bOutputFileOpen = true;
		CWaitCursor pWait;

		CString strHeaderLine = "THIRDPARTYID,PERSONID,NAME,ADDRESS_LINE_1,ADDRESS_LINE_2,CITY,STATE,ZIP\r\n";
		fOutputFile.Write(strHeaderLine, strHeaderLine.GetLength());

		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT "
			"COALESCE(HL7IDLinkT.ThirdPartyID, '') AS ThirdPartyID, "
			"PersonT.ID AS PersonID, "
			"PersonT.FullName AS Name, "
			"PersonT.Address1 AS AddressLine1, "
			"PersonT.Address2 AS AddressLine2, "
			"PersonT.City AS City, "
			"PersonT.State AS State, "
			"REPLACE(PersonT.Zip, '-', '') AS Zip "
			"FROM {CONST_STRING} "
			"LEFT JOIN PersonT ON {CONST_STRING}.PersonID = PersonT.ID "
			"LEFT JOIN HL7IDLinkT ON PersonT.ID = HL7IDLinkT.PersonID AND HL7IDLinkT.RecordType = {INT} AND HL7IDLinkT.GroupID = {INT} ",
			strTableName, strTableName, m_IDLinkRecordType, nHL7GroupID);

		// Write each record
		while(!prs->eof) {
			CString strThirdPartyID = AdoFldString(prs,"ThirdPartyID", "");
			long nPersonID = AdoFldLong(prs, "PersonID", -1); // its ok to export this, our importer ignores negatives
			CString strName = AdoFldString(prs, "Name", "");
			CString strAddress1 = AdoFldString(prs, "AddressLine1", "");
			CString strAddress2 = AdoFldString(prs, "AddressLine2", "");
			CString strCity = AdoFldString(prs, "City", "");;
			CString strState = AdoFldString(prs, "State", "");
			CString strZip = AdoFldString(prs, "Zip", "");

			CString strLine = FormatString("%s,%li,%s,%s,%s,%s,%s,%s\r\n",
				EscapeCSV(strThirdPartyID), nPersonID, EscapeCSV(strName), 
				EscapeCSV(strAddress1), EscapeCSV(strAddress2), 
				EscapeCSV(strCity), EscapeCSV(strState), EscapeCSV(strZip));
			fOutputFile.Write(strLine, strLine.GetLength());

			prs->MoveNext();
		}

		if(bOutputFileOpen){
			bOutputFileOpen = false; // just in case close below blows up, we dont want to try and close it again in our catch
			fOutputFile.Close();
		}

		CNxDialog::OnOK();
	} NxCatchAllCall(__FUNCTION__,	if(bOutputFileOpen){ fOutputFile.Close(); });
}

BEGIN_EVENTSINK_MAP(CHL7IDLinkExportDlg, CNxDialog)
	ON_EVENT(CHL7IDLinkExportDlg, IDC_HL7ID_EXPORT_HL7GROUPS_LIST, 16, CHL7IDLinkExportDlg::SelChosenHl7idExportHl7groupsList, VTS_DISPATCH)
	ON_EVENT(CHL7IDLinkExportDlg, IDC_HL7ID_EXPORT_HL7GROUPS_LIST, 1, CHL7IDLinkExportDlg::SelChangingHl7idExportHl7groupsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CHL7IDLinkExportDlg, IDC_HL7ID_EXPORT_HL7GROUPS_LIST, 18, CHL7IDLinkExportDlg::RequeryFinishedHl7idExportHl7groupsList, VTS_I2)
END_EVENTSINK_MAP()

void CHL7IDLinkExportDlg::SelChosenHl7idExportHl7groupsList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow && VarLong(pRow->GetValue(HL7IDExportList::ID)) != -1) {
			m_nxbExport.EnableWindow(TRUE);
		} else {
			m_nxbExport.EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CHL7IDLinkExportDlg::SelChangingHl7idExportHl7groupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7IDLinkExportDlg::RequeryFinishedHl7idExportHl7groupsList(short nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupList->GetNewRow();
		pRow->PutValue(HL7IDExportList::ID, _variant_t((long)-1, VT_I4));
		pRow->PutValue(HL7IDExportList::Name, _variant_t(" < Select an HL7 Settings Group >"));
		m_pGroupList->AddRowBefore(pRow, m_pGroupList->GetFirstRow());
		m_pGroupList->CurSel = pRow;
	}NxCatchAll(__FUNCTION__);
}
