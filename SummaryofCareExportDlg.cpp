// SummaryofCareExportDlg.cpp : implementation file
//

// (j.gruber 2013-11-05 12:46) - PLID 59323 - created for

#include "stdafx.h"
#include "Practice.h"
#include "SummaryofCareExportDlg.h"
#include "NxAPI.h"
#include "NxAPIUtils.h"

enum ExportListColumns
{
	elcPersonID,
	elcEMNID,
	elcExport,
	elcUserDefinedID,	
	elcDescription,
};

// CSummaryofCareExportDlg dialog

IMPLEMENT_DYNAMIC(CSummaryofCareExportDlg, CNxDialog)

CSummaryofCareExportDlg::CSummaryofCareExportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSummaryofCareExportDlg::IDD, pParent)
{

}

CSummaryofCareExportDlg::~CSummaryofCareExportDlg()
{
}

void CSummaryofCareExportDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EXPORT_SUMMARIES, m_btnExport);
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSummaryofCareExportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EXPORT_SUMMARIES, &CSummaryofCareExportDlg::OnBnClickedExportSummaries)
END_MESSAGE_MAP()


// CSummaryofCareExportDlg message handlers
BOOL CSummaryofCareExportDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();	

	try {

		m_pExportList = BindNxDataList2Ctrl(IDC_PATIENTS_TO_EXPORT_LIST, false);

		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnExport.AutoSet(NXB_EXPORT);

		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT PersonT.ID as PersonID, PatientsT.UserDefinedID, "
			" PersonT.FullName, EMRMasterT.ID as EMNID, EMRMasterT.Date as EMNDate, EMRMasterT.Description "
			" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" INNER JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
			" WHERE EMRMasterT.Deleted = 0 "
			" ORDER BY UserDefinedID, FullName, PersonID, EMRMasterT.Date" );
		
		long nPersonID = -1;
		long nCurrentPersonID = -2;
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL;
		while (!prs->eof)
		{
			nCurrentPersonID = AdoFldLong(prs->Fields, "PersonID");			
			if (nPersonID != nCurrentPersonID)
			{
				//we are on a new patient, make a parent row
				nPersonID = nCurrentPersonID;
				pParentRow = m_pExportList->GetNewRow();
				pParentRow->PutValue(elcPersonID, nCurrentPersonID);				
				pParentRow->PutValue(elcExport, g_cvarFalse);	
				long nUserDefinedID = AdoFldLong(prs->Fields, "UserDefinedID");
				pParentRow->PutValue(elcUserDefinedID, nUserDefinedID);				
				CString strPatientName = AdoFldString(prs->Fields, "FullName");
				CString strDescription = AsString(nUserDefinedID) + " - " + strPatientName;
				pParentRow->PutValue(elcDescription, _variant_t(strDescription));							
				m_pExportList->AddRowAtEnd(pParentRow, NULL);
			}

			//now for the child row
			if (pParentRow)
			{
				NXDATALIST2Lib::IRowSettingsPtr pChildRow = m_pExportList->GetNewRow();
				pChildRow->PutValue(elcPersonID, nCurrentPersonID);
				pChildRow->PutValue(elcEMNID, AdoFldLong(prs->Fields, "EMNID"));
				pChildRow->PutValue(elcExport, g_cvarFalse);
				pChildRow->PutValue(elcUserDefinedID, AdoFldLong(prs->Fields, "UserDefinedID"));				
				
				COleDateTime dt = AdoFldDateTime(prs->Fields, "EMNDate");
				
				CString strDescription = FormatDateTimeForInterface(dt, 0, dtoDate, true) + " - " + AdoFldString(prs->Fields, "Description");
				pChildRow->PutValue(elcDescription, _variant_t(strDescription));

				m_pExportList->AddRowAtEnd(pChildRow, pParentRow);
			}

			prs->MoveNext();

		}
		


	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CSummaryofCareExportDlg::GetCheckedEMNs(NXDATALIST2Lib::IRowSettingsPtr pPatientRow, CCDAExport &ccdaExport)
{

	if (pPatientRow)
	{			

		//loop through its children and see which ones are checked
		NXDATALIST2Lib::IRowSettingsPtr pEMNRow = pPatientRow->GetFirstChildRow();
		while (pEMNRow)
		{
			BOOL bIsChecked = VarBool(pEMNRow->GetValue(elcExport));
			if (bIsChecked)
			{							

				long nEMNID = VarLong(pEMNRow->GetValue(elcEMNID));
				ccdaExport.emnIDs.push_back(nEMNID);
			}
			pEMNRow = pEMNRow->GetNextRow();
		}
	}

}

BOOL CSummaryofCareExportDlg::FillExportList()
{

	//first clear anything that may be in here
	m_mapExports.empty();

	//loop through the list and determine with patients are checked
	NXDATALIST2Lib::IRowSettingsPtr pPatientRow = m_pExportList->GetFirstRow();
	while (pPatientRow)
	{
		BOOL bIsChecked = VarBool(pPatientRow->GetValue(elcExport));
		if (bIsChecked)
		{
			CCDAExport ccdaExport;
			ccdaExport.strPatientName = VarString(pPatientRow->GetValue(elcDescription));

			//see which emns are checked			
			GetCheckedEMNs(pPatientRow, ccdaExport);

			//make sure they checked some EMNs
			if (ccdaExport.emnIDs.size() == 0)
			{
				MsgBox("Please choose at least one EMN to export per patient");
				m_pExportList->CurSel = pPatientRow;
				return FALSE;
			}

			//now add to our map
			//we need to use UserDefinedID
			long nPatientID = VarLong(pPatientRow->GetValue(elcUserDefinedID));

			// (a.walling 2014-04-24 12:00) - VS2013 - never specify the function template for make_pair
			m_mapExports.insert(std::make_pair(nPatientID, ccdaExport));		
		}
		pPatientRow = pPatientRow->GetNextRow();
	}

	if (m_mapExports.size() == 0)
	{
		MsgBox("Please choose at least one patient record to export.");
		return FALSE;
	}

	return TRUE;

}

bool CSummaryofCareExportDlg::DetermineSavePath(CString &strExportPath)
{	
	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
	// non-existent intermediate paths are created.
	FileUtils::CreatePath(GetEnvironmentDirectory() ^ "SummaryOfCare\\");

	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
	CString dir = GetEnvironmentDirectory() ^ "SummaryOfCare\\";
	//Browse to the Additional path
	if (BrowseToFolder(&strExportPath, "Select Folder for Summary of Care documents", GetSafeHwnd(), NULL, dir)) {				 
		return true;
	}
	else {
		return false;
	}	
}

//fills our export list
void CSummaryofCareExportDlg::OnBnClickedExportSummaries()
{
	CString strPatientID;
	CString strPatientName;
	try{
		//get the export path
		CString strExportPath;
		
		
		//first fill the export list
		if (FillExportList()) {
			if (DetermineSavePath(strExportPath))
			{
				//now export
				std::map<int, CCDAExport>::iterator itPatients;
				for(itPatients = m_mapExports.begin(); itPatients != m_mapExports.end(); itPatients++)
				{
					//get our options structure
					NexTech_Accessor::_CCDAPatientFilterPtr patFilter(__uuidof(NexTech_Accessor::CCDAPatientFilter)); 
					patFilter->patientID = (LPCTSTR)AsString(itPatients->first);
					
									
					CCDAExport ccdaExport = itPatients->second;

					strPatientName = ccdaExport.strPatientName;		
					strPatientID = AsString(itPatients->first);

					Nx::SafeArray<BSTR> aryEMNIDs;			

					long nLength = ccdaExport.emnIDs.size();
					for (int i = 0; i < nLength; i++)
					{
						aryEMNIDs.Add(_bstr_t(AsString(ccdaExport.emnIDs[i])));
					}

					patFilter->EmnIDs = aryEMNIDs;

					//we only want the xml
					NexTech_Accessor::_CCDAOutputOptionsPtr outputOptions(__uuidof(NexTech_Accessor::CCDAOutputOptions));
					outputOptions->IncludeXMLFile = true;
					outputOptions->IncludePDFFile = false;

					NexTech_Accessor::_CCDAGenerationResultPtr ccdaResult = NULL;
		
					// (j.jones 2014-01-10 16:20) - PLID 60274 - GenerateCCDA can return an API soap fault
					// when a message box (such as when the CCDA is not set up) is desired instead of an exception.
					// We need to look for the special error, and handle it. If it is not handled, then throw it
					// as a normal exception.
					try {
						ccdaResult = GetAPI()->GenerateSummaryofCare(GetAPISubkey(), GetAPILoginToken(), patFilter, outputOptions);
					}catch (_com_error &e) {
						//this function will turn expected soap exceptions into messageboxes,
						//return TRUE if it did, FALSE if the error still needs handled
						if(!ProcessAPIComError_AsMessageBox(GetSafeHwnd(), "Practice", e)) {
							throw e;
						}

						return;
					}

					Nx::SafeArray <BYTE> aryBytes = ccdaResult->XMLFile;

					//write the xml to our path
					COleDateTime dtNow = COleDateTime::GetCurrentTime();
					CString strNow = FormatDateTimeForSql(dtNow, dtoDateTime);
					FileUtils::ReplaceInvalidFileChars(strNow, '_');
					CString strName = ccdaExport.strPatientName;
					FileUtils::ReplaceInvalidFileChars(strName, '_');
					CString strFileName = strName + "_" + strNow + ".xml";
				
					
					CString strOutput((const char*)aryBytes.begin(), aryBytes.GetLength());								
					CFile OutFile(strExportPath ^ strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);				
					
					OutFile.Write((LPCTSTR)strOutput, strOutput.GetLength());

				}

				//let them know we are done
				AfxMessageBox("Done!");
				OnOK();
			}
		}

	}NxCatchAll("Error exporting patient: " + strPatientID +" - " + strPatientName);


}
BEGIN_EVENTSINK_MAP(CSummaryofCareExportDlg, CNxDialog)
	ON_EVENT(CSummaryofCareExportDlg, IDC_PATIENTS_TO_EXPORT_LIST, 10, CSummaryofCareExportDlg::EditingFinishedPatientsToExportList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CSummaryofCareExportDlg::EditingFinishedPatientsToExportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow)
		{
			//expand it
			pRow->Expanded = true;

			//check off all its children
			NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
			while (pChildRow)
			{
				pChildRow->PutValue(elcExport, varNewValue);

				pChildRow = pChildRow->GetNextRow();
			}

			//add its parent if we are checking it
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if (VarBool(varNewValue)) {				
				if (pParentRow)
				{
					pParentRow->PutValue(elcExport, varNewValue);
				}
			}

			//now go back through and see if all our emns are checked off
			if (pParentRow && VarBool(varNewValue) == FALSE)
			{
				NXDATALIST2Lib::IRowSettingsPtr pChRow = pParentRow->GetFirstChildRow();
				BOOL bChecked = FALSE;
				while (pChRow)
				{
					BOOL bExport = VarBool(pChRow->GetValue(elcExport));
					if (bExport)
					{
						bChecked = TRUE;
					}
					pChRow = pChRow->GetNextRow();
				}
				if (!bChecked)
				{
					//all the children are unchecked, get the parent too
					pParentRow->PutValue(elcExport, varNewValue);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}
