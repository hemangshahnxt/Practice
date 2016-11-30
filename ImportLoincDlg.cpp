// ImportLoincDlg.cpp : implementation file
//
// (s.tullis 2015-11-13 16:41) - PLID 54285 - Created
#include "stdafx.h"
#include "Practice.h"
#include "ImportLoincDlg.h"
#include "afxdialogex.h"
#include <set>
#include "NxAutoQuantum.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
using namespace NXDATALIST2Lib;
// CImportLoincDlg dialog

enum LOINCCodeImportColumn {
	lcicCheckBox=0,
	lcicCode,
	lcicShort,
	lcicDescription,
};

IMPLEMENT_DYNAMIC(CImportLoincDlg, CNxDialog)

CImportLoincDlg::CImportLoincDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_IMPORT_LOINC, pParent)
{

}

CImportLoincDlg::~CImportLoincDlg()
{
}

void CImportLoincDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMPORT_LOINC_FILE, m_btnImportCSV);
	DDX_Control(pDX, IDC_SAVE_CODES, m_btnSave);
	DDX_Control(pDX, IDC_CLOSE_CODES, m_btnClose);
	DDX_Control(pDX, IDC_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_DESELECT_ALL, m_btnDeselectAll);
	DDX_Control(pDX, IDC_DELETE_CODES, m_btnDelete);

}


BEGIN_MESSAGE_MAP(CImportLoincDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SAVE_CODES, &CImportLoincDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_DELETE_CODES, &CImportLoincDlg::OnBnClickedDeleteCodes)
	ON_BN_CLICKED(IDC_SELECT_ALL, &CImportLoincDlg::OnBnClickedSelectAll)
	ON_BN_CLICKED(IDC_DESELECT_ALL, &CImportLoincDlg::OnBnClickedDeselectAll)
	ON_BN_CLICKED(IDC_IMPORT_LOINC_FILE, &CImportLoincDlg::OnBnClickedImportLoincFile)
	ON_BN_CLICKED(IDC_CLOSE_CODES, &CImportLoincDlg::OnBnClickedCloseCodes)
END_MESSAGE_MAP()


// CImportLoincDlg message handlers
BEGIN_EVENTSINK_MAP(CImportLoincDlg, CNxDialog)
	
END_EVENTSINK_MAP()



BOOL CImportLoincDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		m_dlLOINCImportList = BindNxDataList2Ctrl(this, IDC_LOINC_IMPORT_LIST, GetRemoteData(), false);	
		m_btnSave.AutoSet(NXB_OK);
		m_btnClose.AutoSet(NXB_CANCEL);
		m_btnImportCSV.AutoSet(NXB_MODIFY);
		m_btnSelectAll.AutoSet(NXB_MODIFY);
	    m_btnDeselectAll.AutoSet(NXB_MODIFY);
	    m_btnDelete.AutoSet(NXB_DELETE);
	}NxCatchAll(__FUNCTION__)
		return TRUE;
}
// (s.tullis 2015-11-13 16:41) - PLID 54285 - Save records in the datalist
void CImportLoincDlg::OnBnClickedOk()
{
	try {
		if (m_dlLOINCImportList->GetRowCount() == 0)
		{
			MsgBox("No records in the list to Import!\r\nClick the 'import file' button to import codes into the list.");
			return;
		}
		Nx::Quantum::Batch strSaveString;
		IRowSettingsPtr pRow = m_dlLOINCImportList->GetFirstRow();
		long nRecordCount = 0;
		while (pRow)
		{
			if (VarBool(pRow->GetValue(lcicCheckBox)) == TRUE) {
				AddStatementToSqlBatch(strSaveString, "Insert INTO LabLOINCT (Code, ShortName, LongName )  Values ('%s' , '%s', '%s' ) ",
					_Q(VarString(pRow->GetValue(lcicCode))), _Q(VarString(pRow->GetValue(lcicShort))), _Q(VarString(pRow->GetValue(lcicDescription))));
				nRecordCount++;
			}
			pRow = pRow->GetNextRow();
		}
		ExecuteSqlBatch(strSaveString.FlattenToClassic());
		MsgBox(FormatString("Imported %li records successfully!", nRecordCount));
		// (s.tullis 2015-11-19 10:30) - PLID 54285 - Since we imported successfully.. remove the imported rows from the list
		RemoveCheckedCodes();
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-11-13 16:41) - PLID 54285 - Delete current LOINCT codes (warn the user)
void CImportLoincDlg::OnBnClickedDeleteCodes()
{
	try {
		ADODB::_RecordsetPtr rsDetails = CreateRecordset("Select Count(DISTINCT LabLOINCT.ID) as LOINCTCount FROM LabLOINCT");

	int nDecision =	MsgBox(MB_ICONWARNING | MB_YESNO, FormatString(
			"You are about to delete ALL current order codes in Practice (%li codes). This cannot be undone.\r\n"
		    "Are you sure you want to continue?", AdoFldLong(rsDetails,"LOINCTCount",0)));

	if (nDecision != IDYES)
	{
		return;
	}  

	ExecuteSql("Update LabsToBeOrderedT Set LOINCID = NULL;"
		""
		"Delete FROM LabLOINCT");
	long nAuditID = -1;
	nAuditID = BeginNewAuditEvent();
	if (nAuditID != -1) {
		AuditEvent(-1, "", nAuditID, aeiDeletedLOINCTCodes, -1, "", "Order Codes Deleted", aepMedium, aetDeleted);
	}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-11-13 16:41) - PLID 54285 - Select All in the datalist
void CImportLoincDlg::OnBnClickedSelectAll()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_dlLOINCImportList->GetFirstRow();
		while (pRowIter) {
			if (!VarBool(pRowIter->GetValue(lcicCheckBox), FALSE)) {
				pRowIter->PutValue(lcicCheckBox, g_cvarTrue);
			}
			pRowIter = pRowIter->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-11-13 16:41) - PLID 54285 - Deselect All in the datalist
void CImportLoincDlg::OnBnClickedDeselectAll()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_dlLOINCImportList->GetFirstRow();
		while (pRowIter) {
			if (VarBool(pRowIter->GetValue(lcicCheckBox), TRUE)) {
				pRowIter->PutValue(lcicCheckBox, g_cvarFalse);
			}
			pRowIter = pRowIter->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-11-13 16:41) - PLID 54285 - Prompt and import the CSV into the datalist
// we don't want duplicates or empty LOINCT codes
// this should be a well formatted csv support will have to format these in excel if neccessary before importing
void CImportLoincDlg::OnBnClickedImportLoincFile()
{
	try {
		CFile fInputFile;
		std::set<std::tuple<CString, CString, CString>> LOINCset;
		CFileDialog dlg(TRUE, "csv", "OrderCodes.csv",
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "Comma Separated Values file|*.csv;*.txt||");

		if (dlg.DoModal() == IDOK) {
			m_dlLOINCImportList->Clear();
			CString strPathName = dlg.GetPathName();

			CCSVParser parse;
			boost::scoped_ptr<CCSVRecordSet> csvrs(parse.Parse(strPathName, false));
			long nFieldCount = csvrs->GetFieldCount();
			if (nFieldCount != 3)
			{
				MsgBox(MB_ICONWARNING | MB_OK,
				R"( 
The file to import is required to be simple list of order codes, short name, and long name, in that order. 
Only comma-separated-values (CSV) files are supported. These can be created from Excel and most other 
data programs. Ensure that the three columns of data are, IN ORDER, 'order code', 'short name', 'long name'. 
Then save as a CSV file. Blank order code and duplicate records will be ignored.
				)");
				return;
			}
			CString strLOINCInError = "";
			CString strShortInError = "";
			CString strDescriptionInError = "";
			CString strLOINC = "";
			CString strShort = "";
			CString strDesc = "";

			//treat it like a 2d array. 
			//Row
			for (int i = 0; i < csvrs->GetRecordCount(); i++) {

				//new row
				IRowSettingsPtr pRow = m_dlLOINCImportList->GetNewRow();

				//Column
				for (int j = 0; j < csvrs->GetFieldCount(); j++) {
					CString strValue = csvrs->GetFieldValue(i, j);

					//set up row
					switch (j + 1) {
					case lcicCode:
					{
						if (strValue.GetLength() > 20) {
							strLOINCInError += strValue + "\r\n";
							strValue.Left(20);
						}
						strLOINC = strValue;
						break;
					}
					case lcicShort:
					{
						if (strValue.GetLength() > 255) {
							strShortInError += strValue + "\r\n";
							strValue.Left(255);
						}
						strShort = strValue;
						break;
					}
					case lcicDescription:
					{
						if (strValue.GetLength() > 255) {
							strDescriptionInError += strValue + "\r\n";
							strValue.Left(255);
						}
						strDesc = strValue;
						break;
					}
					default:
					{
						//If this is hit, we don't know what it is. 
						continue;
						break;
					}
					}
				}
				// We don't want empty LOINC codes or duplicates
				auto search = LOINCset.find(std::make_tuple(strLOINC, strShort, strDesc));
				if (!strLOINC.IsEmpty() && search == LOINCset.end())
				{
					pRow->PutValue(lcicCheckBox, g_cvarFalse);
					pRow->PutValue(lcicCode, _bstr_t(strLOINC));
					pRow->PutValue(lcicShort, _bstr_t(strShort));
					pRow->PutValue(lcicDescription, _bstr_t(strDesc));
					LOINCset.insert(std::make_tuple(strLOINC, strShort, strDesc));
					//add row to datalist. 
					m_dlLOINCImportList->AddRowAtEnd(pRow, NULL);
				}

			}

			CString strErrors = (
				(strLOINCInError.GetLength() > 0 ? "\r\n Codes \r\n----- \r\n" + strLOINCInError : "")
				+ (strShortInError.GetLength() > 0 ? "\r\n Short \r\n----- \r\n" + strShortInError : "")
				+ (strDescriptionInError.GetLength() > 0 ? "\r\n Description \r\n----- \r\n" + strDescriptionInError : "")
				);

			if (strErrors.GetLength() > 0) {
				MsgBox(MB_ICONWARNING | MB_OK,
					"The following values were over the allotted length for their field. "
					"They will be automatically truncated.\r\n\r\n%s", strErrors);
			}
		}
	}NxCatchAll(__FUNCTION__)
}


void CImportLoincDlg::OnBnClickedCloseCodes()
{
	try {
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-11-19 10:30) - PLID 54285 - Remove Checked Rows from the list
void CImportLoincDlg::RemoveCheckedCodes() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_dlLOINCImportList->GetFirstRow();
		while (pRowIter) {
			NXDATALIST2Lib::IRowSettingsPtr pRowRemove = NULL;
			if (VarBool(pRowIter->GetValue(lcicCheckBox))==TRUE) {
				pRowRemove = pRowIter;
			}
			pRowIter = pRowIter->GetNextRow();

			if (pRowRemove)
			{
				m_dlLOINCImportList->RemoveRow(pRowRemove);
			}
		}
	}NxCatchAll(__FUNCTION__)
}