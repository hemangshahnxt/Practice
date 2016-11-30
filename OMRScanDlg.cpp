// OMRScanDlg.cpp : implementation file
//

// (j.dinatale 2012-08-02 16:11) - PLID 51941 - created

#include "stdafx.h"
#include "Practice.h"
#include "OMRScanDlg.h"
#include "EmrRc.h"
#include "OMRUtils.h"
#include "OMRReviewDlg.h"
#include <process.h>
#include <NxDataUtilitiesLib/NxSafeArray.h>	// (j.dinatale 2012-08-22 14:59) - PLID 52284
#include <foreach.h>	// (j.dinatale 2012-08-22 14:59) - PLID 52284


// COMRScanDlg dialog
namespace OMRScanList{
	enum OMRScanCols{
		PatientID = 0,
		PatientName = 1,
		ReviewHyper = 2,
		NxXMLFilePath = 3,
	};
};

// (j.dinatale 2012-09-05 09:27) - PLID 51941 - enum for Form List columns
namespace OMRFormList{
	enum OMRFormCols{
		ID = 0,
		Description = 1,
	};
};

IMPLEMENT_DYNAMIC(COMRScanDlg, CNxDialog)

COMRScanDlg::COMRScanDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COMRScanDlg::IDD, pParent)
{

}

COMRScanDlg::~COMRScanDlg()
{
}

void COMRScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OMR_SCAN_BGRND, m_bkgrnd);
	DDX_Control(pDX, IDC_OMR_SCAN_BTN, m_btnScan);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
}


BEGIN_MESSAGE_MAP(COMRScanDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_OMR_SCAN_BTN, OnBnClickedOmrScanBtn)
	ON_MESSAGE(NXM_TWAIN_XFERDONE, OnTwainXferdone)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(COMRScanDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMNToBeBilledDlg)
	ON_EVENT(COMRScanDlg, IDC_OMR_SCAN_PENDING_LIST, 19 /* LeftClick */, OnLeftClickPendingOMRList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(COMRScanDlg, IDC_OMR_SCAN_FORM_LIST, 1, COMRScanDlg::SelChangingOmrScanFormList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()


// COMRScanDlg message handlers
BOOL COMRScanDlg::OnInitDialog()
{ 
	CNxDialog::OnInitDialog();
	try{
		m_pFormList = BindNxDataList2Ctrl(IDC_OMR_SCAN_FORM_LIST, true);
		m_pPendingOMRs = BindNxDataList2Ctrl(IDC_OMR_SCAN_PENDING_LIST, false);
		m_bkgrnd.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// (j.fouts 2013-02-27 17:00) - PLID 54715 - Added bulk cache for the Default category
		// (b.spivey, April 22, 2013) - PLID 56035 - Bulk cache. 
		g_propManager.BulkCache("OMRScanDlg", propbitNumber | propbitText, "("
			"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
			"		Name = 'OMRDefaultImportCategory' " 
			"		OR Name = 'RemarkInstallPath' "
			"		OR Name = 'RemarkOMRCommandLineFlags' "
			"		OR Name = 'RemarkTimeOutInSeconds' "
			"	)"
			")", 
			_Q(GetCurrentUserName()), (g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnScan.AutoSet(NXB_MODIFY);
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.dinatale 2012-08-22 16:13) - PLID 52265 - Load pending NxXML when we show this window
void COMRScanDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		if(bShow){
			// (j.dinatale 2012-09-05 09:24) - PLID 51941 - should be a small list so requery and set our last selection
			long nCurrFormID = -1;
			if(m_pFormList->CurSel){
				nCurrFormID = VarLong(m_pFormList->CurSel->GetValue(OMRFormList::ID), -1);
			}

			m_pFormList->Requery();
			LoadPendingNxXML();

			if(nCurrFormID > 0){
				m_pFormList->FindByColumn(OMRFormList::ID, nCurrFormID, NULL, VARIANT_TRUE);
			}else{
				m_pFormList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
				m_pFormList->CurSel = m_pFormList->GetFirstRow();
			}
		}

		CNxDialog::OnShowWindow(bShow, nStatus);
	}NxCatchAll(__FUNCTION__);
}

void COMRScanDlg::OnBnClickedCancel()
{
	try{
		// just hide, dont call the base class cancel
		ShowWindow(SW_HIDE);
	}NxCatchAll(__FUNCTION__);
}

void COMRScanDlg::OnBnClickedOmrScanBtn()
{
	try{
		if (NXTWAINlib::IsAcquiring()){
			this->MessageBox("Please wait for the current image acquisition to complete before starting a new one.");
			return;
		}

		if(!m_pFormList){
			return;
		}

		if(!m_pFormList->CurSel){
			this->MessageBox("Please select the form before scanning.", "Warning", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		long nCurrFormID = -1;
		nCurrFormID = VarLong(m_pFormList->CurSel->GetValue(OMRFormList::ID), -1);
		if(!ReturnsRecordsParam("SELECT TOP 1 1 FROM OMRFormT WHERE ID = {INT}", nCurrFormID)){
			m_pFormList->Requery();
			this->MessageBox("The selected form no longer exists. Please select a new form and scan again.");
			m_pFormList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			m_pFormList->CurSel = m_pFormList->GetFirstRow();
			return;
		}

		CString strScanPath;
		strScanPath.Format("Remark_%lu", GetTickCount());
		strScanPath = GetNxTempPath() ^ strScanPath;

		if(!FileUtils::EnsureDirectory(strScanPath)){
			this->MessageBox("Could not create a temp directory for scanned documents to be placed in. Please try scanning again.");
			return;
		}

		m_strCurrentScanPath = strScanPath;

		GetMainFrame()->RegisterTwainDlgWaiting(this);
		NXTWAINlib::Acquire(-25, m_strCurrentScanPath, COMRScanDlg::OnTWAINCallback, COMRScanDlg::OnNxTwainPreCompress, NULL, "", -1, -1, -1, NXTWAINlib::eScanToNative);
	}NxCatchAll(__FUNCTION__);
}

LRESULT COMRScanDlg::OnTwainXferdone(WPARAM wParam, LPARAM lParam)
{
	try {		
		GetMainFrame()->UnregisterTwainDlgWaiting(this);

		if(!FileUtils::IsFolderEmpty(m_strCurrentScanPath)){

			// bail if we dont have a valid form selection
			if(!m_pFormList->GetCurSel()){
				return 0;
			}

			long nFormID = VarLong(m_pFormList->GetCurSel()->GetValue(0), -1);
			ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM OMRFormT WHERE ID = {INT}", nFormID); 

			if (rs->eof) {
				return 0;
			}

			// (b.spivey, August 20, 2012) - PLID 51721 - This is getting the process running. 
			STARTUPINFO si = {sizeof(STARTUPINFO)};
			PROCESS_INFORMATION pi = {0};

			// (b.spivey, April 03, 2013) - PLID 56036 - They'll give us a time in seconds, we'll turn it into ms.
			long nPrefTimeOut = GetPropertyInt("RemarkTimeOutInSeconds", 90, 0, true); 
			//if it's 0, force it to 90 anyways. 
			if (nPrefTimeOut == 0) {
				nPrefTimeOut = 90; 
			}
			nPrefTimeOut *= 1000; 

			CString strInstallPath = GetPropertyText("RemarkInstallPath", "", 0, true);
			// (b.spivey, April 02, 2013) - PLID 56035 - Get the command line flags in the preference. 
			CString strCommandLineFlags = GetPropertyText("RemarkOMRCommandLineFlags", "/E /H /Wn /Crg ", 0, true); 
			//So it may not be clear because these flags are pretty cryptic, so I'll try to explain them as best I can.
			// "/E" - Exit on finish
			// "/H" - Hide Splash Screen 
			// "/Wn" - Window state: Normal. 
			// "/Crg" - Review exceptions; Required items and Unrecognized images. 
			//These settings, as the functionality works now, are the ideal set up. It closes the program for them (a program that
			//	practice is waiting on to finish), it hides superflous screens, shows that the program is indeed working, and 
			//  prevents lost documents. 
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
			CString strPendingPath = GetEnvironmentDirectory() ^ "NexTech" ^ "PendingOMRs";
			CString strRemarkXmlName;
			strRemarkXmlName.Format("Remark_%lu.xml", GetTickCount());
			CString strTemplateLocation = AdoFldString(rs->Fields, "FilePath", ""); 
			CString strFormName = AdoFldString(rs->Fields, "Description", "");
			DWORD dwTickCount = GetTickCount(); 
			CString strTargetDir; 
			strTargetDir.Format("[PatientID]%s%lu", strFormName, dwTickCount); 

			//Commando line. 
			CString strCmdLine;
			// (b.spivey, February 27, 2013) - PLID 54717 - PDF instead of JPG. 
			strCmdLine.Format("\"%s\" "//The location of Remark.
			"/T \"%s\" " //The template we're using. 
			"/Fxml \"%s\" \"%s\" " //The output xml
			"/Nb^op3 \"%s\" " //The target directory of the images.
			"/Ip \"%s\" " //selection source of the images
			"%s ", 
			strInstallPath, 
			strTemplateLocation, 
			(m_strCurrentScanPath ^ strRemarkXmlName), 
			strRemarkXmlName, 
			(strPendingPath ^ strTargetDir),
			(m_strCurrentScanPath ^ "*.*"),
			strCommandLineFlags); 

			//Throw that into the command line and let remark handle the rest. 
			BOOL bRemarkSuccess = CreateProcess(NULL, (LPSTR)(LPCTSTR)strCmdLine,
			NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

			CString strMessageText;
			strMessageText.Format("The Remark OMR process did not finish before the timeout (%li seconds). "
				"Would you like to continue waiting for another %li seconds? ", (nPrefTimeOut / 1000), (nPrefTimeOut / 1000));
			DWORD dwProcessStatus; 
			do {
			// (b.spivey, April 03, 2013) - PLID 56036 - Blew up the other comments-- 
			//		simply use a preference instead of trying to be clever.
				dwProcessStatus = WaitForSingleObject(pi.hProcess, nPrefTimeOut); 
			// (b.spivey, April 03, 2013) - PLID 56037 - If we timed out, give them the chance to wait again. 
			} while(dwProcessStatus == WAIT_TIMEOUT && AfxMessageBox(strMessageText, MB_YESNO | MB_ICONWARNING) == IDYES);

			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);

			
			//Error handling. 
			if(!bRemarkSuccess) {
				Log("Remark was unable to start: error 0x%08x", GetLastError());
				this->MessageBox("Remark was unable to launch. Please check your settings and try again. If the problem "
					"persists, please contact technical support.");
				//break early.
				return 0;
			}

			// (b.spivey, August 31, 2012) - PLID 51721 - File wasn't created, don't try to parse. 
			if (!FileUtils::DoesFileOrDirExist((m_strCurrentScanPath ^ strRemarkXmlName))) {
				this->MessageBox("Remark was unable to create an output file. Please check your form settings and try again. ");
				//break early. 
				return 0; 
			}

			//location of the remark output plus the form. 
			OMRUtils::RemarkOutputProcess(strPendingPath, (m_strCurrentScanPath ^ strRemarkXmlName), dwTickCount, nFormID);
		}

		// (j.dinatale 2012-08-22 11:00) - PLID 52256 - clear out our scanned directory
		FileUtils::Deltree(m_strCurrentScanPath);
		m_strCurrentScanPath = "";
		
		// (j.dinatale 2012-09-06 11:00) - PLID 52256 - need to reload our pending NxXml folder contents
		LoadPendingNxXML();
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.dinatale 2012-08-22 15:56) - PLID 52265 - need to load any pending OMRs
void COMRScanDlg::LoadPendingNxXML()
{
	if(!m_pPendingOMRs){
		return;
	}

	m_pPendingOMRs->Clear();

	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
	CString strPendingPath = GetEnvironmentDirectory() ^ "NexTech" ^ "PendingOMRs";

	if(!FileUtils::EnsureDirectory(strPendingPath)){
		return;
	}

	CString strSearch = strPendingPath ^ "*.xml";
	CFileFind NxXMLFinder;
	bool bFilesFound = !!NxXMLFinder.FindFile(strSearch);

	while(bFilesFound){
		bFilesFound = !!NxXMLFinder.FindNextFile();

		if(NxXMLFinder.IsDots() || NxXMLFinder.IsDirectory()){
			continue;
		}

		AddOMRRow(NxXMLFinder.GetFilePath());
	}
}

// (j.dinatale 2012-08-22 11:28) - PLID 52256 - need to hold onto the file path to the NxXML
void COMRScanDlg::AddOMRRow(const CString &strNxXMLFilePath)
{
	if(!m_pPendingOMRs){
		return;
	}

	NexTech_COM::INxXmlGeneratorPtr pNxXmlGen = OMRUtils::ProcessXML(strNxXMLFilePath);

	if(!pNxXmlGen){
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPendingOMRs->GetNewRow();
	if(pRow){
		long nPatientID = pNxXmlGen->GetPatientID();
		long nUserDefinedID = -1;
		CString strName = "";
		
		if(nPatientID > 0){
			nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
			strName = GetExistingPatientName(nPatientID);

			if(nUserDefinedID > 0){
				pRow->PutValue(OMRScanList::PatientID, nUserDefinedID);
			}
		}

		if(!strName.IsEmpty()){
			pRow->PutValue(OMRScanList::PatientName, _bstr_t(strName));
		}
		// (b.spivey, September 10, 2012) - PLID 52513 - Show no patient. 
		else {
			pRow->PutValue(OMRScanList::PatientName, _bstr_t("< No Patient Found >")); 
		}

		pRow->PutValue(OMRScanList::ReviewHyper, "< Review and Commit >");

		// (j.dinatale 2012-08-22 11:31) - PLID 52256 - need to hold the file path
		pRow->PutValue(OMRScanList::NxXMLFilePath, _bstr_t(strNxXMLFilePath));
		m_pPendingOMRs->AddRowSorted(pRow, NULL);
	}
}

// (j.dinatale 2012-08-22 09:16) - PLID 52060 - handle committing emns
void COMRScanDlg::OnLeftClickPendingOMRList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			return;
		}

		switch(nCol){
			case OMRScanList::ReviewHyper:
				{
					// (j.dinatale 2012-08-09 12:15) - PLID 52060 - need to be able to review our NxXML, then attempt to commit it
					// (j.dinatale 2012-08-22 12:56) - PLID 52256 - use file path instead
					CString strNxXMLPath = VarString(pRow->GetValue(OMRScanList::NxXMLFilePath), "");

					// (j.dinatale 2012-09-20 09:49) - PLID 52265 - need to check if the NxXML file exists
					if(!FileUtils::DoesFileOrDirExist(strNxXMLPath)){
						this->MessageBox("This pending OMR no longer exists. The pending OMR list will be refreshed.", "Warning", MB_OK | MB_ICONEXCLAMATION);
						LoadPendingNxXML();
						return;
					}

					NexTech_COM::INxXmlGeneratorPtr pNxXML = OMRUtils::ProcessXML(strNxXMLPath);

					if(!pNxXML)
						return;

					int nResult = IDCANCEL;
					{
						COMRReviewDlg dlg(pNxXML, this);
						nResult = dlg.DoModal();
					}

					switch(nResult)
					{
						case IDYES:
							{
								long nEMNID = OMRUtils::CommitNxXMLToEMN(pNxXML);
								if(nEMNID <= 0){
									this->MessageBox("Failed to create an EMN. Please try again.", "Error", MB_OK | MB_ICONERROR);
									return;
								}

								// (j.dinatale 2012-08-22 14:59) - PLID 52284 - attach files
								bool bSkipAttachDelete = false;
								if(!OMRUtils::AttachFilesFromNxXML(pNxXML, nEMNID)){
									CString strWarning = "One or more of the following files failed to attach. Please ensure the following files exist and manually attach any missing files:\r\n\r\n";
									Nx::SafeArray<BSTR> saryFiles(pNxXML->GetFiles());
									foreach(_bstr_t strPath, saryFiles){
										CString strFilePath = (LPCSTR)strPath;
										strWarning += (strFilePath + "\r\n");
									}

									bSkipAttachDelete = true;
									this->MessageBox(strWarning, "Warning", MB_OK | MB_ICONEXCLAMATION);
								}

								// (j.dinatale 2012-08-22 11:16) - PLID 52256 - delete files that we created for NxXML
								if(!OMRUtils::DeleteNxXMLFiles(strNxXMLPath, pNxXML, bSkipAttachDelete)){
									CString strWarning;
									strWarning.Format(
										"Failed to delete OMR files. Please manually remove the following files:\r\n\r\n%s\r\n", strNxXMLPath);

									Nx::SafeArray<BSTR> saryFiles(pNxXML->GetFiles());
									foreach(_bstr_t strPath, saryFiles){
										CString strFilePath = (LPCSTR)strPath;
										strWarning += (strFilePath + "\r\n");
									}

									this->MessageBox(strWarning, "Warning", MB_OK | MB_ICONEXCLAMATION);
								}

								m_pPendingOMRs->RemoveRow(pRow);
							}
							break;
						case IDNO:
							{
								// (j.dinatale 2012-08-22 11:16) - PLID 52256 - delete files that we created for NxXML
								CString strNxXMLPath = VarString(pRow->GetValue(OMRScanList::NxXMLFilePath), "");
								OMRUtils::DeleteNxXMLFiles(strNxXMLPath, pNxXML); 
								/*
									// (b.spivey, February 28, 2013) - PLID 54717 - This really doesn't matter anymore. We'll delete 
									//		when we can. 
								if(!OMRUtils::DeleteNxXMLFiles(strNxXMLPath, pNxXML)){
									CString strWarning;
									strWarning.Format(
										"Failed to delete OMR files. Please manually remove the following files:\r\n\r\n%s\r\n", strNxXMLPath);

									Nx::SafeArray<BSTR> saryFiles(pNxXML->GetFiles());
									foreach(_bstr_t strPath, saryFiles){
										CString strFilePath = (LPCSTR)strPath;
										strWarning += (strFilePath + "\r\n");
									}

									this->MessageBox(strWarning, "Warning", MB_OK | MB_ICONEXCLAMATION);
								}
								*/

								m_pPendingOMRs->RemoveRow(pRow);
							}
							break;
						case IDCANCEL:
						default:
							break;
					}
				}
				break;
			default:
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

void COMRScanDlg::SelChangingOmrScanFormList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);
		if(!pRow){
			lppNewSel = &lpOldSel;
		}
	}NxCatchAll(__FUNCTION__);
}

// Twain callbacks
void WINAPI CALLBACK COMRScanDlg::OnNxTwainPreCompress(
		const LPBITMAPINFO pbmi, // 
		BOOL& bAttach, // bAttach
		BOOL& bAttachChecksum, // bAttachChecksum
		long& nChecksum, // Checksum
		ADODB::_Connection* lpCon) // Connection
{
	return;
}

void WINAPI CALLBACK COMRScanDlg::OnTWAINCallback(NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach, void* pUserData, CxImage& cxImage)
{
	return;
}
