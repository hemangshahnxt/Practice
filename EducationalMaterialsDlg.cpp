// EducationalMaterialsDlg.cpp : implementation file
//
// (c.haag 2010-09-23 09:15) - PLID 40640 - Initial implementation. This class is used to detect what patient educational
// templates need to be generated for this patient, and then merge them.
//

#include "stdafx.h"
#include "Practice.h"
#include "EducationalMaterialsDlg.h"
#include "Filter.h"
#include "Groups.h"
#include "MergeEngine.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "SelectSenderDlg.h"
#include "LetterWriting.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

typedef enum {
	ecChecked = 0,
	ecFilterID = 1,
	ecMergeTemplateID = 2
} EColumns;

// This thread calculates what educational templates need to be merged for the patient, and notifies
// the CEducationalMaterialsDlg object as it finds them.
UINT CalculateRelevantTemplates(LPVOID lpData)
{
	try {
		CEducationalMaterialsThreadData* pData = (CEducationalMaterialsThreadData*)lpData;
		_ConnectionPtr pConn = GetThreadRemoteData();

		// Run the query to get a list of all the possible educational filters and templates
		_RecordsetPtr prs = CreateRecordsetStd(pConn, 
			"SELECT EducationTemplatesT.ID, FilterID, FiltersT.Name, FiltersT.Filter, MergeTemplateID FROM EducationTemplatesT "
			"INNER JOIN FiltersT ON FiltersT.ID = EducationTemplatesT.FilterID "
			);
		// Retain the record count
		long nRecords = prs->GetRecordCount();
		long nCurRecord = 0;
		::PostMessage(pData->hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nRecords-1));

		// Now go through each letter writing filter and run queries to see which filters qualify. Build a map to prevent the same
		// query from being run twice.
		CMap<CString, LPCTSTR, BOOL, BOOL> mapTriedFilters;
		while (!prs->eof)
		{
			if (WaitForSingleObject(pData->hStopThread, 0) != WAIT_TIMEOUT) {
				break;
			}

			// Update status text
			CString strText;
			strText.Format("Please wait while Practice calculates which templates to merge...(%d / %d)",
				nCurRecord+1, nRecords);
			::PostMessage(pData->hwndProgressBar, PBM_SETPOS, nCurRecord++, 0);
			::SetWindowText(pData->hwndProgressText, strText);

			// The following block of code checks whether the patient satisfies the current filter. First we convert the
			// filter string to a WHERE clause, then we run a query using that clause to see if this patient satisfies the
			// clause. We then take the results of that query and if it works out, then we add it to a map of accepted
			// filters.
			const long nEducationTemplateID = AdoFldLong(prs, "ID");
			const long nFilterID = AdoFldLong(prs, "FilterID");
			const long nTemplateID = AdoFldLong(prs, "MergeTemplateID");
			const CString strName = AdoFldString(prs, "Name", "");
			const CString strFilter = AdoFldString(prs, "Filter", "");
			CString strFilterWhere, strFilterFrom;
			if(!CFilter::ConvertFilterStringToClause(nFilterID, strFilter, fboPerson, &strFilterWhere, &strFilterFrom, NULL, NULL, FALSE, pConn)) 
			{
				AfxMessageBox("Materials could not be generated because the filter " + strName + "is invalid.");
				return -1;
			}
			CString strSql;
			strSql.Format("SELECT CASE WHEN %d IN ("
					"SELECT PersonT.ID AS PersonID FROM %s WHERE (%s)"
					") THEN 1 ELSE 0 END AS FilterSatisfied"
				,pData->nPatientID, strFilterFrom, strFilterWhere); 

			BOOL bSuccess = FALSE;
			if (!mapTriedFilters.Lookup(strSql, bSuccess)) 
			{
				_RecordsetPtr prsExists = CreateRecordsetStd(pConn, strSql);
				if (!prsExists->eof) {
					bSuccess = (AdoFldLong(prsExists, "FilterSatisfied") == 0) ? FALSE : TRUE;
				} else {
					ThrowNxException("Error checking filter '%s': prsExists was eof!", strName);
				}
				mapTriedFilters.SetAt(strSql, bSuccess);
			}
			else 
			{
				// We already tried this query. We took the cached success value and put it in bSuccess.
			}

			if (bSuccess) {
				// Notify the dialog that we found a good word template
				CEducationalMaterialsThreadMessage* m = new CEducationalMaterialsThreadMessage();
				m->nFilterID = nFilterID;
				m->nTemplateID = nTemplateID;
				::PostMessage(pData->hwndParent, NXM_EDUCATIONAL_MATERIALS_ADD_ROW, (WPARAM)m, 0);
			}

			prs->MoveNext();
		} // while (!prs->eof)
		prs->Close();

		::PostMessage(pData->hwndParent, NXM_EDUCATIONAL_MATERIALS_PROCESSING_FINISHED, 0, 0);
		delete pData;
		return 0;
	}
	NxCatchAll(__FUNCTION__);
	return -1;
}

// CEducationalMaterialsDlg dialog

IMPLEMENT_DYNAMIC(CEducationalMaterialsDlg, CNxDialog)

CEducationalMaterialsDlg::CEducationalMaterialsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEducationalMaterialsDlg::IDD, pParent)
{
	m_nPatientID = -1;
	m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CEducationalMaterialsDlg::~CEducationalMaterialsDlg()
{
	CloseHandle(m_hStopThread);
}

void CEducationalMaterialsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_EDUCATIONAL_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CEducationalMaterialsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_MESSAGE(NXM_EDUCATIONAL_MATERIALS_ADD_ROW, OnProcessingAddRow)
	ON_MESSAGE(NXM_EDUCATIONAL_MATERIALS_PROCESSING_FINISHED, OnProcessingFinished)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEducationalMaterialsDlg message handlers

BOOL CEducationalMaterialsDlg::OnInitDialog()
{
	try 
	{
		CNxDialog::OnInitDialog();

		// Set up controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// Force a requery to populate the dropdowns
		m_dlList = BindNxDataList2Ctrl(this, IDC_LIST_QUALIFYING_TEMPLATES, GetRemoteData(), true);

		// Start the calculation thread
		CEducationalMaterialsThreadData* pData = new CEducationalMaterialsThreadData();
		pData->hwndParent = GetSafeHwnd();
		pData->hwndProgressBar = GetDlgItem(IDC_PROGRESS_EDUCATIONAL)->GetSafeHwnd();
		pData->hwndProgressText = GetDlgItem(IDC_STATIC_EDUCATIONAL_PROGRESS)->GetSafeHwnd();
		pData->nPatientID = m_nPatientID;
		pData->hStopThread = m_hStopThread;

		m_pThread = AfxBeginThread(CalculateRelevantTemplates, (LPVOID)pData, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		m_pThread->m_bAutoDelete = false;
		m_pThread->ResumeThread();
	}
	NxCatchAll(__FUNCTION__);
	return FALSE;
}

void CEducationalMaterialsDlg::OnDestroy() 
{
	try {
		if (m_pThread) 
		{
			// Set the event to break the loop in the thread
			SetEvent(m_hStopThread);

			// Get the exit code
			DWORD dwExitCode = 0;
			::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);
			// See if the thread is still active
			if (dwExitCode == STILL_ACTIVE) {
				// The thread is still going so post a quit message to it and let it delete itself
				// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
				m_pThread->m_bAutoDelete = TRUE;
				PostThreadMessage(m_pThread->m_nThreadID, WM_QUIT, 0, 0);
			} else {
				// The thread is finished, so just delete it
				delete m_pThread;
			}
			
			// Either we just deleted the thread or it will automatically be deleted later
			m_pThread = NULL;
		}
	}
	NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

void CEducationalMaterialsDlg::OnOK()
{
	try {
		// Iterate through the datalist finding template paths to merge to
		IRowSettingsPtr pRow = m_dlList->GetFirstRow();
		CStringArray astrPaths;
		while (NULL != pRow) 
		{
			if (VarBool(pRow->GetValue(ecChecked))) 
			{
				astrPaths.Add(VarString(pRow->GetOutputValue(ecMergeTemplateID)));
			}
			pRow = pRow->GetNextRow();
		}

		if (0 == astrPaths.GetSize()) {
			AfxMessageBox("Please select at least one template to merge to.", MB_ICONSTOP | MB_OK);
			return;
		}

		// Ensure all the templates exist. Do not do partial merges.
		int nTemplate;
		for (nTemplate=0; nTemplate < astrPaths.GetSize(); nTemplate++)
		{
			CString strPath = astrPaths.GetAt(nTemplate);
			
			//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
			if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
				strPath = GetSharedPath() ^ strPath;
			}

			// PLID  15371: make sure the path exists and if it doesn't, output a warning
			if (!DoesExist(strPath)) {
				CString strMessage;
				strMessage.Format("The file %s in the packet does not exist.\nPlease check the path of the template. This packet cannot be merged.", strPath);
				// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
				MessageBox(strMessage, NULL, MB_ICONINFORMATION);
				return;
			}
		}

		// Now get the sender info
		CSelectSenderDlg dlgSender(this);
		if(dlgSender.DoModal() == IDCANCEL)
			return;

		// Now merge each template separately
		for (nTemplate=0; nTemplate < astrPaths.GetSize(); nTemplate++)
		{
			CMergeEngine mi;
			CString strPath = astrPaths.GetAt(nTemplate);
			
			//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
			if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
				strPath = GetSharedPath() ^ strPath;
			}

			/// Generate the temp table
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
			CString strMergeTo = CreateTempIDTable(strSql, "ID");
			
			// Merge flags
			if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
			mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

			// Sender info
			mi.m_strSubjectMatter = dlgSender.m_strSubjectMatter;
			mi.m_strSender = dlgSender.m_strFirst + (dlgSender.m_strMiddle.IsEmpty() ? "" : (" "+ dlgSender.m_strMiddle)) + " " + dlgSender.m_strLast;
			mi.m_strSenderFirst = dlgSender.m_strFirst;
			mi.m_strSenderMiddle = dlgSender.m_strMiddle;
			mi.m_strSenderLast = dlgSender.m_strLast;
			mi.m_strSenderEmail = dlgSender.m_strEmail;
			mi.m_strSenderTitle = dlgSender.m_strTitle;

			// Do the merge
			CString strCurrentTemplateFilePath;
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
			if (!mi.MergeToWord(strPath, std::vector<CString>(), strMergeTo, "", -1, -1, true, FormatString("Template %i of %i", nTemplate + 1, astrPaths.GetSize()))) {
				break;
			}

		} // for (nTemplate=0; nTemplate < astrPaths.GetSize(); nTemplate++)	

		__super::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

// This message is posted from the worker thread to inform this object that it found an educational
// template to merge
LRESULT CEducationalMaterialsDlg::OnProcessingAddRow(WPARAM wParam, LPARAM lParam)
{
	try {
		CEducationalMaterialsThreadMessage* m = (CEducationalMaterialsThreadMessage*)wParam;
		IRowSettingsPtr pRow = m_dlList->GetNewRow();
		pRow->PutValue(ecChecked, g_cvarTrue);
		pRow->PutValue(ecFilterID, m->nFilterID);
		pRow->PutValue(ecMergeTemplateID, m->nTemplateID);
		m_dlList->AddRowAtEnd(pRow, NULL);
		delete m;
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// This message is posted from the worker thread to inform this object that the processing is finished
LRESULT CEducationalMaterialsDlg::OnProcessingFinished(WPARAM wParam, LPARAM lParam)
{
	try {
		SetDlgItemText(IDC_STATIC_EDUCATIONAL_PROGRESS, "Ready");
		GetDlgItem(IDC_PROGRESS_EDUCATIONAL)->PostMessage(PBM_SETPOS, 0);
		if (m_dlList->GetRowCount() > 0) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			// (c.haag 2010-09-28 12:42) - Per DT, this isn't necessary
			//AfxMessageBox(FormatString("%d educational template(s) were found.", m_dlList->GetRowCount()), MB_ICONINFORMATION | MB_OK);
		}
		else {
			AfxMessageBox("This patient does not meet the filter criteria for any Educational templates.", MB_ICONINFORMATION | MB_OK);
			PostMessage(WM_COMMAND, IDCANCEL);
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}
