// DeviceImportCombineFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "AdministratorRc.h"
#include "DeviceImportCombineFilesDlg.h"
#include "FileUtils.h"
#include "TaskEditDlg.h"		// (d.lange 2011-05-25 16:53) - PLID 43253

using namespace NXDATALIST2Lib;
using namespace ADODB;

//(e.lally 2011-03-30) PLID 42733 - Created
// CDeviceImportCombineFilesDlg dialog

enum FileListColumns
{
	flcIcon=0,
	flcFilename,
	flcFilepath,
};

enum PatientColumns
{
	pcPersonID=0,
	pcLast,
	pcFirst,
	pcMiddle,
	pcUserdefinedID,
	pcBirthdate,
};

enum CategoryColumns
{
	ccCatID=0,
	ccCatName,
};

IMPLEMENT_DYNAMIC(CDeviceImportCombineFilesDlg, CNxDialog)

CDeviceImportCombineFilesDlg::CDeviceImportCombineFilesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDeviceImportCombineFilesDlg::IDD, pParent)
{
	m_nSelectedPatientID = -1;
	m_nSelectedCategoryID = -1;
}

CDeviceImportCombineFilesDlg::~CDeviceImportCombineFilesDlg()
{
}

void CDeviceImportCombineFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnImportHistory);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DEVICE_COMBINEFILES_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_DEVICE_COMBINEFILES_MOVE_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_DEVICE_COMBINEFILES_SHOWFULLPATH_CHECK, m_chkShowFullFilePath);
	DDX_Control(pDX, IDC_CHK_CREATE_TODO, m_chkCreateToDo);
}


BEGIN_MESSAGE_MAP(CDeviceImportCombineFilesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_DEVICE_COMBINEFILES_MOVE_UP, &CDeviceImportCombineFilesDlg::OnBnClickedMoveUp)
	ON_BN_CLICKED(IDC_DEVICE_COMBINEFILES_MOVE_DOWN, &CDeviceImportCombineFilesDlg::OnBnClickedMoveDown)
	ON_BN_CLICKED(IDC_DEVICE_COMBINEFILES_SHOWFULLPATH_CHECK, &CDeviceImportCombineFilesDlg::OnShowFullFilePath)
	ON_BN_CLICKED(IDC_CHK_CREATE_TODO, OnBnClickedChkCreateToDo)
END_MESSAGE_MAP()


// CDeviceImportCombineFilesDlg message handlers


BOOL CDeviceImportCombineFilesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		
		//Set controls
		m_btnImportHistory.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);

		// (d.lange 2011-05-04 12:18) - PLID 43253
		m_chkCreateToDo.SetCheck(GetRemotePropertyInt("DeviceImportCombineFiles_CreateToDo", 0, 0, GetCurrentUserName(), true));

		m_pPatientCombo = BindNxDataList2Ctrl(IDC_DEVICE_COMBINEFILES_PATIENT_LIST, true);
		//Mimic the string value of the device import dlg when no patient was set ahead of time
		m_pPatientCombo->PutComboBoxText(_bstr_t("<Select a Valid Patient>"));
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		m_pCategoryCombo = BindNxDataList2Ctrl(IDC_DEVICE_COMBINEFILES_CATEGORY_LIST, false);
		m_pCategoryCombo->WhereClause = _bstr_t(GetAllowedCategoryClause("ID"));
		m_pCategoryCombo->Requery();
		m_pFileList = BindNxDataList2Ctrl(IDC_DEVICE_COMBINEFILES_FILE_LIST, false);
		LoadFilesIntoList();

		if(m_nSelectedPatientID != -1){
			m_pPatientCombo->SetSelByColumn(pcPersonID, (long)m_nSelectedPatientID);
		}
		if(m_nSelectedCategoryID != -1){
			m_pCategoryCombo->SetSelByColumn(ccCatID, (long)m_nSelectedCategoryID);
		}

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

//(e.lally 2011-04-04) PLID 42733 - Takes a default patientID, categoryID, and array of files to cache and load upon opening the dlg.
void CDeviceImportCombineFilesDlg::LoadFiles(long nPreselectPatientID, long nPreselectCategoryID, CStringArray& aryFilesToCombine)
{
	try {
		m_nSelectedPatientID = nPreselectPatientID;
		m_nSelectedCategoryID = nPreselectCategoryID;
		m_aryFilesToCombine.RemoveAll();

		m_aryFilesToCombine.Append(aryFilesToCombine);
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733 - Takes the cached list of files to load and puts them on the interface
void CDeviceImportCombineFilesDlg::LoadFilesIntoList()
{
	try {
		for(int i = 0; i < m_aryFilesToCombine.GetSize(); i++) {
			IRowSettingsPtr pRow = m_pFileList->GetNewRow();
			if(pRow != NULL){
				// track the source file that generated this record, 
				CString strFilePath = m_aryFilesToCombine[i];
				//(e.lally 2011-04-06) PLID 42733 - Take the first entry and set it as the default new filename,
				//	extracting the Page X.pdf from the end.
				if(i==0){
					CString strNewFile = FileUtils::GetFileName(strFilePath);
					int nPageIndex = strNewFile.Find(" - Page");
					if(nPageIndex != -1){
						//(e.lally 2011-06-03) PLID 42733 - Stop at nPageIndex, no sooner.
						strNewFile = strNewFile.Left(nPageIndex);
					}
					strNewFile.Replace(".pdf", "");
					strNewFile.Replace(".PDF", "");
					SetDlgItemText(IDC_DEVICE_COMBINEFILES_NEW_FILENAME, strNewFile);
				}
				pRow->PutValue(flcFilename, _bstr_t(FileUtils::GetFileName(strFilePath)));
				pRow->PutValue(flcFilepath, _bstr_t(strFilePath));

				m_pFileList->AddRowAtEnd(pRow, NULL);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733 - Moves the selected file one slot up in the ordered list
void CDeviceImportCombineFilesDlg::OnBnClickedMoveUp()
{
	try {
		IRowSettingsPtr pSelRow = m_pFileList->GetCurSel();
		if(pSelRow == NULL){
			return;
		}

		IRowSettingsPtr pPrevRow = pSelRow->GetPreviousRow();
		if(pPrevRow == NULL){
			//we're already the first row
			return;
		}
		IRowSettingsPtr pNewSelRow = m_pFileList->AddRowBefore(pSelRow, pPrevRow);
		m_pFileList->CurSel = pNewSelRow;
		m_pFileList->RemoveRow(pSelRow);

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733 - Moves the selected file one slot down in the ordered list
void CDeviceImportCombineFilesDlg::OnBnClickedMoveDown()
{
	try {
		IRowSettingsPtr pSelRow = m_pFileList->GetCurSel();
		if(pSelRow == NULL){
			return;
		}

		IRowSettingsPtr pNextRow = pSelRow->GetNextRow();
		if(pNextRow == NULL){
			//we're already the last row
			return;
		}
		m_pFileList->AddRowBefore(pNextRow, pSelRow);
		m_pFileList->RemoveRow(pNextRow);

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733 - Saves the ordered list of files to a single file for the selected patient, if all valid information is provided
void CDeviceImportCombineFilesDlg::OnOK()
{
	try {

		if(!IsValidState()){
			return;
		}

		CWaitCursor wcWait;
		//(e.lally 2011-04-04) PLID 42819 - Loop through the list of files and save into one new file in the patient's history tab.
		PDFCreatorPilotLib::IPDFDocument4Ptr pPDF;
		HRESULT hr = pPDF.CreateInstance(__uuidof(PDFCreatorPilotLib::PDFDocument4));

		if(FAILED(hr)) {
			if(hr == REGDB_E_CLASSNOTREG) {
				//the PDFCreatorPilot.dll is not registered, we need to register it!
				MessageBox("The PDFCreatorPilot.dll file is not properly installed on your system.\n"
					"Please contact NexTech for assistance.",
					"Practice", MB_ICONEXCLAMATION|MB_OK);
				pPDF = NULL;
				return;
			}
			else {
				//unknown error, the dll did not load, but not because it isn't registered
				CString strError;
				strError.Format("There was an unknown error (0x%x: %s) when trying to access the PDFCreatorPilot.dll.\n"
					"Please contact NexTech for assistance.", hr, FormatError((int)hr));
				MessageBox(strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
				pPDF = NULL;
				return;
			}
		}
		
		//now that is is loaded, license it
		hr = pPDF->SetLicenseData("NexTech", "EZ2PS-SRW5S-2WHKX-V8C33");
		if(hr != S_OK){
			MessageBox("There was an error when trying to validate the PDFCreatorPilot.dll license.\n"
				"Please contact NexTech for assistance.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		IRowSettingsPtr pCurRow = m_pFileList->GetFirstRow();
		if(pCurRow == NULL){
			ASSERT(FALSE);
			return;
		}


		while(pCurRow != NULL){
			CString strCurFile = VarString(pCurRow->GetValue(flcFilepath));
			hr = pPDF->Append(_bstr_t(strCurFile), "");
			if(hr != S_OK){
				CString strError;
				strError.Format("There was an unknown error (0x%x: %s) when trying to append the file '%s'.\n"
					"Please contact NexTech for assistance.", hr, FormatError((int)hr), strCurFile);
				MessageBox(strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
				pPDF = NULL;
				return;
			}
			pCurRow = pCurRow->GetNextRow();
		}

		//A blank page is created at the beginning, let's get rid of it.
		hr = pPDF->DeletePage(0);
		if(hr != S_OK){
			//ignore this error for now. I think.
		}
		CString strNewFileName;
		GetDlgItemText(IDC_DEVICE_COMBINEFILES_NEW_FILENAME, strNewFileName);
		if(strNewFileName.Right(4).CompareNoCase(".pdf") != 0){
			//add the .pdf extension on the end
			strNewFileName += ".pdf";
		}
		CString strPatDocumentPath = GetPatientDocumentPath(m_nSelectedPatientID);
		CString strFullPatientHistoryFilePath = strPatDocumentPath ^ strNewFileName;
		hr = pPDF->SaveToFile(_bstr_t(strFullPatientHistoryFilePath), false);
		if(hr != S_OK){
			CString strError;
			strError.Format("There was an unknown error (0x%x: %s) when trying to save the new file to '%s'.\n"
				"Please contact NexTech for assistance.", hr, FormatError((int)hr), strFullPatientHistoryFilePath);
			MessageBox(strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
			pPDF = NULL;
			return;
		}

		//get the selected category
		IRowSettingsPtr pSelCatRow = m_pCategoryCombo->GetCurSel();
		m_nSelectedCategoryID = -1;
		if(pSelCatRow != NULL){
			m_nSelectedCategoryID = VarLong(pSelCatRow->GetValue(ccCatID), -1);
		}

		COleDateTime dt;
		dt.SetStatus(COleDateTime::null);
		//Attach the new file to their history tab!
		// (d.singleton 2013-11-15 11:18) - PLID 59513 - need to insert the CCDAType in mailsent when generating a CCDA
		long nMailSent = CreateNewMailSentEntry(m_nSelectedPatientID, strNewFileName, SELECTION_FILE, 
			strFullPatientHistoryFilePath, GetCurrentUserName(),"", -1, dt, -1,
			m_nSelectedCategoryID, -1, -1, FALSE, -1, "", ctNone);

		//delete the original PDFs
		pCurRow = m_pFileList->GetFirstRow();
		if(pCurRow == NULL){
			ASSERT(FALSE);
			return;
		}


		while(pCurRow != NULL){
			CString strCurFile = VarString(pCurRow->GetValue(flcFilepath));
			DeleteFile(strCurFile);
			pCurRow = pCurRow->GetNextRow();
		}
		
		// (d.lange 2011-05-04 12:19) - PLID 43253 - Display the todo dialog if the checkbox is enabled
		if(IsDlgButtonChecked(IDC_CHK_CREATE_TODO)) {
			CTaskEditDlg dlg(this);
			dlg.m_nPersonID = m_nSelectedPatientID;
			dlg.m_bIsNew = TRUE;
			dlg.SetNewTaskRegardingOverrides(nMailSent, ttMailSent);
			dlg.DoModal();
		}

		//We're finished, close the dlg
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733 - Ensures that all the information is valid for saving. Returns false if not.
bool CDeviceImportCombineFilesDlg::IsValidState()
{
	//Do we have a valid patient selected
	IRowSettingsPtr pSelPatRow = m_pPatientCombo->GetCurSel();
	m_nSelectedPatientID = -1;
	if(pSelPatRow != NULL){
		m_nSelectedPatientID = VarLong(pSelPatRow->GetValue(pcPersonID), -1);
	}
	if(pSelPatRow == NULL || m_nSelectedPatientID <= 0){
		AfxMessageBox("You must select a valid patient before saving.");
		return false;
	}

	//Do we have a valid filename
	CString strNewFileName;
	GetDlgItemText(IDC_DEVICE_COMBINEFILES_NEW_FILENAME, strNewFileName);
	if(strNewFileName.IsEmpty() || strNewFileName != strNewFileName.SpanExcluding("\\/:*?\"<>|")){
		AfxMessageBox("You must select a valid filename before saving.");
		return false;
	}

	if(strNewFileName.Right(4).CompareNoCase(".pdf") != 0){
		//add the .pdf extension on the end
		strNewFileName += ".pdf";
	}

	//Check if this file already exists under this patient.
	CString strPatDocumentPath = GetPatientDocumentPath(m_nSelectedPatientID);
	CString strFullPatientHistoryFilePath = strPatDocumentPath ^ strNewFileName;
	if(FileUtils::DoesFileOrDirExist(strFullPatientHistoryFilePath)){
		AfxMessageBox(FormatString("There is already a file named '%s' for this patient. Please use a new filename before saving.", strNewFileName));
		return false;
	}

	//(e.lally 2011-04-06) PLID 42733 - Double check that these files still exist
	IRowSettingsPtr pCurRow = m_pFileList->GetFirstRow();
	if(pCurRow == NULL){
		ASSERT(FALSE);
		MessageBox("You must have two or more files in the list to combine.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		return false;
	}

	bool bAllFilesFound = true;
	CString strMissingFiles ="";
	while(pCurRow != NULL){
		CString strCurFile = VarString(pCurRow->GetValue(flcFilepath));
		if(!FileUtils::DoesFileOrDirExist(strCurFile)){
			bAllFilesFound = false;

			strMissingFiles += strCurFile + "\n";
		}
		
		pCurRow = pCurRow->GetNextRow();
	}

	if(!bAllFilesFound){
		strMissingFiles.TrimRight("\n");
		MessageBox(FormatString("The following files could not be found. They may have been imported by another user.\n\n%s",strMissingFiles),
			"Practice", MB_ICONEXCLAMATION|MB_OK);
			return false;
	}

	return true;
}

//(e.lally 2011-04-22) PLID 42733 - toggle filename and filepath columns
void CDeviceImportCombineFilesDlg::OnShowFullFilePath()
{
	try {
		//For speed of drawing
		m_pFileList->SetRedraw(VARIANT_FALSE);
		IColumnSettingsPtr pColNameOnly = m_pFileList->GetColumn(flcFilename);
		IColumnSettingsPtr pColFullPath = m_pFileList->GetColumn(flcFilepath);

		if(pColNameOnly != NULL && pColFullPath != NULL){
			BOOL bShowFullPath = m_chkShowFullFilePath.GetCheck();

			if(bShowFullPath){
				pColNameOnly->PutStoredWidth(0);
				pColNameOnly->ColumnStyle = csFixedWidth|csVisible;
				pColFullPath->ColumnStyle = csWidthAuto|csVisible;
			}
			else {
				pColFullPath->PutStoredWidth(0);
				pColFullPath->ColumnStyle = csFixedWidth|csVisible;
				pColNameOnly->ColumnStyle = csWidthAuto|csVisible;
			}
		}

		//For speed of drawing
		m_pFileList->SetRedraw(VARIANT_TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2011-05-04 12:17) - PLID 43253 - Handle when the checkbox is selected
void CDeviceImportCombineFilesDlg::OnBnClickedChkCreateToDo()
{
	try {
		SetRemotePropertyInt("DeviceImportCombineFiles_CreateToDo", IsDlgButtonChecked(IDC_CHK_CREATE_TODO), 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}