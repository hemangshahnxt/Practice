// NexFormsImportNewFileSheet.cpp : implementation file
//

#include "stdafx.h"
#include "letterwriting.h"
#include "NexFormsImportNewFileSheet.h"
#include "FileUtils.h"
#include "RegUtils.h"
#include "FtpUtils.h"
#include "NexFormsImportInfo.h"
#include <afxinet.h>
#include "ShowProgressFeedbackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportNewFileSheet dialog


CNexFormsImportNewFileSheet::CNexFormsImportNewFileSheet(CNexFormsImportWizardMasterDlg* pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportNewFileSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportNewFileSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bNeedToLoadContent = TRUE;
	m_dwCompressedFileSize = 0;
}


void CNexFormsImportNewFileSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportNewFileSheet)
	DDX_Control(pDX, IDC_NEW_USER_IMPORT, m_btnNeither);
	DDX_Control(pDX, IDC_EXISTING_TRACKING_IMPORT, m_btnTrackingOnly);
	DDX_Control(pDX, IDC_EXISTING_USER_IMPORT, m_btnBoth);
	DDX_Control(pDX, IDC_DOWNLOAD_NEXFORMS_CONTENT, m_btnDownload);
	DDX_Control(pDX, IDC_BROWSE_NEXFORMS_CONTENT_FILE, m_btnBrowse);
	DDX_Control(pDX, IDC_NEXFORMS_CONTENT_FILENAME, m_nxeditNexformsContentFilename);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportNewFileSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportNewFileSheet)
	ON_BN_CLICKED(IDC_BROWSE_NEXFORMS_CONTENT_FILE, OnBrowseNexformsContentFile)
	ON_BN_CLICKED(IDC_DOWNLOAD_NEXFORMS_CONTENT, OnDownloadNexformsContent)
	ON_MESSAGE(NXM_TRANSFER_BEGIN, OnTransferBegin)
	ON_MESSAGE(NXM_TRANSFER_PROGRESS, OnTransferProgress)
	ON_MESSAGE(NXM_TRANSFER_END, OnTransferEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportNewFileSheet message handlers

BOOL CNexFormsImportNewFileSheet::OnInitDialog() 
{
	try
	{
		CNexFormsImportWizardSheet::OnInitDialog();

		if(FileUtils::DoesFileOrDirExist(GetSharedPath() ^ "NexFormsContent.dat")) {
			SetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, GetSharedPath() ^ "NexFormsContent.dat");
		}
		else {
			SetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, "<Could not find NexForms content file>");
		}

	}NxCatchAll("CNexFormsImportNewFileSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportNewFileSheet::OnBrowseNexformsContentFile() 
{
	try
	{
		// (z.manning, 07/02/2007) - Open the file browser and let them look for a filename.
		CFileDialog dlgOpenFile(TRUE, ".dat", GetSharedPath() ^ "NexFormsContent.dat", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Data files (*.dat)|*.dat|All Files (*.*)|*.*||");
		CString strTitle = "Please select a NexForms content file";
		dlgOpenFile.m_ofn.lpstrTitle = strTitle;
		if(dlgOpenFile.DoModal() == IDOK)
		{
			CString strOldFilename;
			GetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, strOldFilename);
			CString strNewFilename = dlgOpenFile.GetPathName();
			SetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, strNewFilename);
			// (z.manning, 07/02/2007) - If it's a different file, we need to load content again.
			if(strNewFilename != strOldFilename) {
				m_bNeedToLoadContent = TRUE;
			}
			m_pdlgMaster->GetDlgItem(IDC_NEXT)->EnableWindow(TRUE);
		}

	}NxCatchAll("CNexFormsImportNewFileSheet::OnBrowseNexformsContentFile");
}

BOOL CNexFormsImportNewFileSheet::Validate()
{
	// (z.manning, 07/30/2007) - Make sure they've chosen their import type.
	// (z.manning, 07/31/2007) - Make sure we set this BEFORE reading from the NexForms content file and before
	// we try to prompt to update the font of any existing content.
	if(IsDlgButtonChecked(IDC_NEW_USER_IMPORT)) {
		m_pdlgMaster->m_ImportInfo.m_eImportType = nfitNew;
	}
	else if(IsDlgButtonChecked(IDC_EXISTING_USER_IMPORT)) {
		m_pdlgMaster->m_ImportInfo.m_eImportType = nfitExisting;
	}
	else if(IsDlgButtonChecked(IDC_EXISTING_TRACKING_IMPORT)) {
		m_pdlgMaster->m_ImportInfo.m_eImportType = nfitNewNexFormsExistingTracking;
	}
	else {
		MessageBox("Please select which type of NexForms user you are.");
		return FALSE;
	}

	// (z.manning, 07/02/2007) - Ok, if we haven't yet, let's load the content.
	if(m_bNeedToLoadContent)
	{
		CString strFile;
		GetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, strFile);

		if(FileUtils::DoesFileOrDirExist(strFile))
		{
			// (z.manning, 07/12/2007) - First, load just the version stuff
			long nCodeVersion, nContentFileVersion, nNexFormsVersion;
			CString strAspsVersion;
			if(ReadVersionInfoFromNexFormsContentFile(strFile, nCodeVersion, nContentFileVersion, strAspsVersion, nNexFormsVersion))
			{
				// (z.manning, 07/12/2007) - The code versions must be the same or else who knows what'll happen.
				if(NEXFORMS_IMPORT_EXPORT_CODE_VERSON < nCodeVersion) {
					// (z.manning, 07/12/2007) - Ok, our version is less than the version that was exported.
					// They likely just need to upgrade.
					MessageBox("Your current version of NexTech is out of date with this NexForms content. Please contact "
						"NexTech technical support about upgrading Practice.");
					return FALSE;
				}
				else if(NEXFORMS_IMPORT_EXPORT_CODE_VERSON > nCodeVersion) {
					// (z.manning, 07/12/2007) - Practice is too new for the content file. Either this is fixed by
					// downloading the latest content from out ftp site, or something is terribly wrong.
					MessageBox("Your current version of NexTech is out of date with this NexForms content. Please click the "
						"download button to make sure you have the most up-to-date NexForms content downloaded.");
					return FALSE;
				}
				else {
					// (z.manning, 07/12/2007) - They're the same, huzzah!  Continue with the import wizard.
					ASSERT(NEXFORMS_IMPORT_EXPORT_CODE_VERSON == nCodeVersion);
				}

				// (z.manning, 10/11/2007) - PLID 27719 - The code to update the font for all procedures is now in NexFormsUtils.

				LoadFromNexFormsContentFile(strFile, m_pdlgMaster->m_ImportInfo);
				m_bNeedToLoadContent = FALSE;

				// (z.manning, 07/13/2007) - To be safe, let's only let them load content once per wizard run.
				// Meaning, once they've loaded it once, they can't come back to this sheet.
				m_bNoReturn = TRUE;
			}
			else
			{
				// (z.manning, 10/17/2007) - We could not read the version info, so the file must not be valid.
				MessageBox(FormatString("'%s' is not a valid NexForms content file.", strFile), "Invalid File");
				return FALSE;
			}
		}
		else
		{
			MessageBox("Please select a valid file before continuing");
			return FALSE;
		}
	}

	return TRUE;
}

// (z.manning, 07/31/2007) - PLID 25715 - Download the latest NexForms content file from our ftp site.
void CNexFormsImportNewFileSheet::OnDownloadNexformsContent() 
{
	CWaitCursor wc;
	CInternetSession* pInternetSession = NULL;
	CFtpConnection *pFtp = NULL;
	NxSocketUtils::HSERVER hLicenseActivationServer = NULL;
	try
	{
		//Open a connection to the license activation server.
		hLicenseActivationServer = ConnectToLicenseActivationServer(GetSubRegistryKey());

		if(!hLicenseActivationServer) {
			//Failed to connect
			MessageBox("Failed to connect to the NexTech Activation Server. Please ensure that you have internet access and try again later.");
			return;
		}

		//Format a request packet with our license key.
		_PACKET_TYPE_NEXFORMS_VERSION_REQUEST pktRequest;
		pktRequest.dwKey = g_pLicense->GetLicenseKey();

		//Data for the response
		void* pResponse = NULL;
		DWORD dwSizeOut = 0;

		BOOL bDownload = TRUE;

		//Send a request for the NexForms version.  If successful, we will have our pResponse filled.
		DWORD dwVersion = 0;
		BOOL bSync = NxSocketUtils::SyncPacketWait(hLicenseActivationServer, PACKET_TYPE_NEXFORMS_VERSION_REQUEST, PACKET_TYPE_NEXFORMS_VERSION_REQUEST_RESPONSE, 
			(void*)&pktRequest, sizeof(_PACKET_TYPE_NEXFORMS_VERSION_REQUEST), pResponse, dwSizeOut);
		if(!bSync)
		{
			//Failed to send the packet.  We either timed out waiting for information, or there was some 
			//network failure on either end that did not allow the send / response to happen.
			MessageBox("Failed to get authorization from the server. This may indicate a problem with your internet "
				"connection, or may be caused by interference with a firewall or other 3rd party application.");
			bDownload = FALSE;
		}
		else
		{
			//We got our response back!
			_PACKET_TYPE_NEXFORMS_VERSION_REQUEST_RESPONSE* pPktOut = (_PACKET_TYPE_NEXFORMS_VERSION_REQUEST_RESPONSE*)pResponse;

			dwVersion = pPktOut->dwContentFileVersion;

			if(dwVersion == 0)
			{
				// (z.manning, 07/13/2007) - PLID 25715 - 0 means our activation server did not approve the request. Either
				// this client is not allowed to download the content for some reason, or perhaps there was
				// an error processing the request.
				MessageBox("NexTech Activation Server did not approve an update of the NexForms content. Please contact NexTech Technical Support.");
				bDownload = FALSE;
			}
			else
			{
				CString strFile;
				GetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, strFile);

				if(FileUtils::DoesFileOrDirExist(strFile))
				{
					// (z.manning, 07/12/2007) - First, load just the version stuff
					long nCodeVersion, nContentFileVersion, nNexFormsVersion;
					CString strAspsVersion;
					if(!ReadVersionInfoFromNexFormsContentFile(strFile, nCodeVersion, nContentFileVersion, strAspsVersion, nNexFormsVersion)) {
						nContentFileVersion = 0;
					}

					if((long)dwVersion == nContentFileVersion) {
						MessageBox("You already have the latest version of the NexForms content.");
						bDownload = FALSE;
					}
				}
			}
		}

		if(bDownload && dwVersion > 0)
		{
			CString strLogin = NxRegUtils::ReadString(GetRegistryBase() + "NexFormsUpdateLogin", "NexForms");
			CString strPass = NxRegUtils::ReadString(GetRegistryBase() + "NexFormsUpdatePassword", "n3xf0rms.07");
			//DRT 11/12/2008 - PLID 31970 - Updated to nexforms-specific subdomain
			CString strSite = NxRegUtils::ReadString(GetRegistryBase() + "NexFormsUpdateSite", "nexforms.nextech.com");
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "NexFormsUpdatePath", "/ftpUsers/NexForms");

			pInternetSession = new CInternetSession;
			// (a.walling 2010-06-25 11:48) - PLID 39358 - Use a passive FTP connection. Active FTP connections are fraught with peril!
			pFtp = pInternetSession->GetFtpConnection(strSite, strLogin, strPass, INTERNET_INVALID_PORT_NUMBER, TRUE);

			if(pFtp == NULL) {
				MessageBox("A connection with the FTP site could not be made. Please check your internet connection and try again.");
				bDownload = FALSE;
			}

			if(!pFtp->SetCurrentDirectory(strPath)) {
				MessageBox("Failed to properly find the NexForms content. Please check your internet connection and try "
					"again. If you continue to have problems, please contact NexTech Technical Support.");
				bDownload = FALSE;
			}

			if(bDownload)
			{
				// (z.manning, 07/19/2007) - PLID 26729 - Show a progress bar.
				CShowProgressFeedbackDlg dlgProgress(0, FALSE);
				dlgProgress.ShowProgress(SW_SHOW);
				dlgProgress.SetCaption("Downloading NexForms Content");

				// (z.manning, 10/16/2007) - Disable most of the controls while the download is in progress.
				EnableDownloadButtons(FALSE);

				FtpUtils::DownloadFileNxCompressedIncremental(pFtp, "NexFormsContent.dat", GetSharedPath() ^ "NexFormsContent.dat", TRUE, FtpUtils::eroExpectNotReadOnly, this, &dlgProgress);
			
				SetDlgItemText(IDC_NEXFORMS_CONTENT_FILENAME, GetSharedPath() ^ "NexFormsContent.dat");
				m_bNeedToLoadContent = TRUE;
			}
		}

	}NxCatchAllCall("CNexFormsImportNewFileSheet::OnDownloadNexformsContent (download file)", EnableDownloadButtons(TRUE));

	try
	{
		if(hLicenseActivationServer != NULL) {
			NxSocketUtils::Disconnect(hLicenseActivationServer);
		}

		if(pFtp != NULL) {
			pFtp->Close();
			delete pFtp;
			pFtp = NULL;
		}
		if(pInternetSession != NULL) {
			pInternetSession->Close();
			delete pInternetSession;
			pInternetSession = NULL;
		}

	}NxCatchAll("CNexFormsImportNewFileSheet::OnDownloadNexformsContent (clean up)");
}

LRESULT CNexFormsImportNewFileSheet::OnTransferBegin(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning, 07/18/2007) - PLID 26729 - Remember the compressed file size.
		m_dwCompressedFileSize = (DWORD)wParam;

	}NxCatchAll("CNexFormsImportNewFileSheet::OnTransferBegin");

	// (z.manning, 07/18/2007) - PLID 26729 - So long as the dialog isn't being destroyed, then return
	// true to tell the transfter to continue.
	return (WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_hevDestroying, 0) == WAIT_TIMEOUT);
}

LRESULT CNexFormsImportNewFileSheet::OnTransferProgress(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning, 07/18/2007) - PLID 26729 - Update the progress bar.
		CShowProgressFeedbackDlg *pProgress = (CShowProgressFeedbackDlg*)lParam;
		DWORD dwBytesDownloaded = (DWORD)wParam;
		pProgress->SetProgress(0, m_dwCompressedFileSize, dwBytesDownloaded);
		if(dwBytesDownloaded >= m_dwCompressedFileSize) {
			pProgress->SetCaption("Extracting file");
		}
		else {
			pProgress->SetCaption(FormatString("Download %.0f%% complete (%.2f of %.2f MB)", dwBytesDownloaded / (float)m_dwCompressedFileSize * 100., dwBytesDownloaded / 1024. / 1024., m_dwCompressedFileSize / 1024. / 1024.));
		}

	}NxCatchAllCall("CNexFormsImportNewFileSheet::OnTransferProgress", EnableDownloadButtons(TRUE));

	// (z.manning, 07/18/2007) - PLID 26729 - So long as the dialog isn't being destroyed, then return
	// true to tell the transfter to continue.
	return (WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_hevDestroying, 0) == WAIT_TIMEOUT);
}

LRESULT CNexFormsImportNewFileSheet::OnTransferEnd(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning, 10/16/2007) - Re-enable the buttons we disabled when starting the download.
		EnableDownloadButtons(TRUE);

	}NxCatchAll("CNexFormsImportNewFileSheet::OnTransferEnd");

	// (z.manning, 07/18/2007) - PLID 26729 - So long as the dialog isn't being destroyed, then return
	// true to tell the transfter to continue.
	return (WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_hevDestroying, 0) == WAIT_TIMEOUT);
}

void CNexFormsImportNewFileSheet::EnableDownloadButtons(BOOL bEnable)
{
	m_pdlgMaster->GetDlgItem(IDC_NEXT)->EnableWindow(bEnable);
	GetDlgItem(IDC_DOWNLOAD_NEXFORMS_CONTENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BROWSE_NEXFORMS_CONTENT_FILE)->EnableWindow(bEnable);
}
