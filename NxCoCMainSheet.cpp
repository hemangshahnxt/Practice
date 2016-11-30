// NxCoCMainSheet.cpp : implementation file
//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
//

#include "stdafx.h"
#include "Practice.h"
#include "NxCoCMainSheet.h"
#include "FileUtils.h"
#include "RegUtils.h"
#include "ShowProgressFeedbackDlg.h"
#include "FtpUtils.h"
#include "NxCoCUtils.h"
#include <afxinet.h>
#include "AdministratorRc.h"


// CNxCoCMainSheet dialog

IMPLEMENT_DYNAMIC(CNxCoCMainSheet, CNxCoCWizardSheet)

CNxCoCMainSheet::CNxCoCMainSheet(CNxCoCWizardMasterDlg* pParent /*=NULL*/)
	: CNxCoCWizardSheet(CNxCoCMainSheet::IDD, pParent)
{
	m_bNeedToLoadContent = true;
	m_dwCompressedFileSize = 0;
}

CNxCoCMainSheet::~CNxCoCMainSheet()
{
}

void CNxCoCMainSheet::DoDataExchange(CDataExchange* pDX)
{
	CNxCoCWizardSheet::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXCOC_USE_PIC, m_btnUsePic);
	DDX_Control(pDX, IDC_NXCOC_USE_LW, m_btnUseLW);
	DDX_Control(pDX, IDC_NXCOC_PATH, m_editPath);
	DDX_Control(pDX, IDC_COCIMPORT_WELCOME_TEXT, m_nxstaticWelcomeText);
	DDX_Control(pDX, IDC_COCIMPORT_DOWNLOAD_TEXT, m_nxstaticDownloadText);
}


BEGIN_MESSAGE_MAP(CNxCoCMainSheet, CNxCoCWizardSheet)
	ON_BN_CLICKED(IDC_NXCOC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_NXCOC_DOWNLOAD, OnBnClickedDownload)
	ON_MESSAGE(NXM_TRANSFER_BEGIN, OnTransferBegin)
	ON_MESSAGE(NXM_TRANSFER_PROGRESS, OnTransferProgress)
	ON_MESSAGE(NXM_TRANSFER_END, OnTransferEnd)
END_MESSAGE_MAP()


// CNxCoCMainSheet message handlers

BOOL CNxCoCMainSheet::OnInitDialog()
{
	try {
		CNxCoCWizardSheet::OnInitDialog();

		// (z.manning 2009-04-09 12:42) - PLID 33934 - Fix this test to handle EMR std importing
		m_nxstaticWelcomeText.SetWindowText("Welcome to the NexTech " + m_pdlgMaster->GetImportTypeText() + 
			" content importing wizard.  This wizard will guide you through acquiring the latest " + m_pdlgMaster->GetImportTypeText() + 
			" content, including packets and letter writing merge templates.");

		// (z.manning 2009-04-09 11:26) - PLID 33934 - Reword necessary text for EMR standard
		if(m_pdlgMaster->m_bEmrStandard)
		{
			// (z.manning 2009-04-09 12:14) - PLID 33934 - We do not currently support downloading
			// the EMR standard content file.
			GetDlgItem(IDC_NXCOC_DOWNLOAD)->EnableWindow(FALSE);
			GetDlgItem(IDC_NXCOC_DOWNLOAD)->ShowWindow(SW_HIDE);

			// (z.manning 2009-04-20 11:43) - PLID 33934 - Per h.butty, we will likely never have PIC packets
			// as a part of EMR standard, so let's just hide these options.
			GetDlgItem(IDC_NXCOC_USE_LW)->EnableWindow(FALSE);
			GetDlgItem(IDC_NXCOC_USE_LW)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NXCOC_USE_PIC)->EnableWindow(FALSE);
			GetDlgItem(IDC_NXCOC_USE_PIC)->ShowWindow(SW_HIDE);

			m_editPath.SetWindowText("<Click Browse to select an EMR Standard content file>");

			m_nxstaticDownloadText.SetWindowText("To start, click on the browse button and then select "
				"an EMR Standard content file.  Once you have selected a valid file, choose your "
				"method of import, and click the Next button to continue the wizard.");
		}
		else
		{
			//Attempt to set the file path if a correctly-named file was found.
			if(FileUtils::DoesFileOrDirExist(GetSharedPath() ^ "NxCycleOfCare.dat")) {
				SetDlgItemText(IDC_NXCOC_PATH, GetSharedPath() ^ "NxCycleOfCare.dat");
			}
			else {
				SetDlgItemText(IDC_NXCOC_PATH, "<Could not find Cycle Of Care content file>");
			}

			m_nxstaticDownloadText.SetWindowText("To start, click on the download button below to ensure "
				"that you have the most up-to-date content.  Once the download finishes, choose your "
				"method of import, and click the Next button to continue the wizard.");
		}

		// (d.thompson 2008-12-02) - PLID 32288 - Set the proper text on the radio button options.  It
		//	has a max of 255 in the resources, but we can put more this way.
		// (z.manning 2009-04-09 12:42) - PLID 33934 - Reword this text to handle EMR std importing
		SetDlgItemText(IDC_NXCOC_USE_PIC, FormatString("Use &PIC Packets - This option will attempt to import all %s packets into the Procedure Information Center.  You should use this option if you have the Tracking tab, as it allows you to merge procedure information into your packets like the surgery date, time, and location information.", m_pdlgMaster->GetImportTypeText()));
		SetDlgItemText(IDC_NXCOC_USE_LW, FormatString("Use &Letter Writing - This option will import all %s packets into Letter Writing. The packets will be available to be merged from Letter Writing and the History tab only. You should only use this option if you DO NOT have the Tracking tab.", m_pdlgMaster->GetImportTypeText()));

		//Auto select the PIC vs LW based on their licensing.  If they do not have tracking, default them to LW.
		if(g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
			//they have tracking, so default that option
			if(m_pdlgMaster->m_bEmrStandard) {
				// (z.manning 2009-04-09 12:36) - PLID 33934 - For EMR Standard importing, default to
				// letter writing.
				CheckDlgButton(IDC_NXCOC_USE_LW, BST_CHECKED);
			}
			else {
				CheckDlgButton(IDC_NXCOC_USE_PIC, BST_CHECKED);
			}
		}
		else {
			//No tracking, go with LW
			CheckDlgButton(IDC_NXCOC_USE_LW, BST_CHECKED);

			//Additionally, disable the PIC option entirely.  If they were to accidentally change
			//	back to this, they'd have content and be completely unable to use it.  Principle
			//	of Least Taunting.
			GetDlgItem(IDC_NXCOC_USE_PIC)->EnableWindow(FALSE);
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CNxCoCMainSheet::OnBnClickedBrowse()
{
	try {
		//Basic browse functionality.  File should be named .dat
		CString strDefaultFile = GetSharedPath();
		if(!m_pdlgMaster->m_bEmrStandard) {
			// (z.manning 2009-04-09 12:41) - PLID 33934 - Don't use this filename if we're doing an EMR std import
			strDefaultFile = strDefaultFile ^ "NxCycleOfCare.dat";
		}
		else {
			// (z.manning 2009-04-20 11:56) - PLID 33934 - We need to send in a filename here or else the
			// browse file dialog won't work on Windows XP.
			strDefaultFile = strDefaultFile ^ "EmrStandardContent.dat";
		}
		CFileDialog dlgOpenFile(TRUE, ".dat", strDefaultFile, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Data files (*.dat)|*.dat|All Files (*.*)|*.*||");
		CString strTitle = "Please select a NexTech " + m_pdlgMaster->GetImportTypeText() + " content file";
		dlgOpenFile.m_ofn.lpstrTitle = strTitle;
		if(dlgOpenFile.DoModal() == IDOK) {
			//They picked a new file.  If it changes, then we need to flag the content for reload.
			CString strOldFilename;
			GetDlgItemText(IDC_NXCOC_PATH, strOldFilename);
			CString strNewFilename = dlgOpenFile.GetPathName();
			SetDlgItemText(IDC_NXCOC_PATH, strNewFilename);
			if(strNewFilename != strOldFilename) {
				m_bNeedToLoadContent = true;
			}
			//Make sure the 'Next' button is enabled once we have a valid path
			m_pdlgMaster->GetDlgItem(IDC_NXCOC_NEXT)->EnableWindow(TRUE);
		}
	}NxCatchAll("Error in OnBnClickedBrowse");
}

void CNxCoCMainSheet::OnBnClickedDownload()
{
	CWaitCursor wc;
	CInternetSession* pInternetSession = NULL;
	CFtpConnection *pFtp = NULL;
	try {
		bool bDownload = true;

		//The defaults will almost always be used, but we provide registry overrides just in case something happens, and also for
		//	testing of new versions.
		CString strLogin = NxRegUtils::ReadString(GetRegistryBase() + "NxCoCUpdateLogin", "NxCoC");
		CString strPass = NxRegUtils::ReadString(GetRegistryBase() + "NxCoCUpdatePassword", "nxc0c.08");
		//DRT 11/12/2008 - PLID 31970 - Updated to subdomain-specific usage
		CString strSite = NxRegUtils::ReadString(GetRegistryBase() + "NxCoCUpdateSite", "nxcoc.nextech.com");
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "NxCoCUpdatePath", "/ftpUsers/NxCoC");

		//Open the internet session, and login with the credentials gained above
		pInternetSession = new CInternetSession;
		// (a.walling 2010-06-25 11:48) - PLID 39358 - Use a passive FTP connection. Active FTP connections are fraught with peril!
		pFtp = pInternetSession->GetFtpConnection(strSite, strLogin, strPass, INTERNET_INVALID_PORT_NUMBER, TRUE);

		if(pFtp == NULL) {
			MessageBox("A connection with the FTP site could not be made. Please check your internet connection and try again.");
			bDownload = false;
		}

		//Ensure we're on the right directory on the FTP site.  When you login using an ftp client, this works automatically, but for
		//	some reason when connecting through code, it leaves you in the FTP root, and we have to set the directory ourselves.
		if(!pFtp->SetCurrentDirectory(strPath)) {
			MessageBox("Failed to properly find the NexTech " + m_pdlgMaster->GetImportTypeText() + " content. Please check your internet connection and try "
				"again. If you continue to have problems, please contact NexTech Technical Support.");
			bDownload = false;
		}

		if(bDownload) {
			//Keep a progress bar, this will be updated via posted messages from the transfer
			CShowProgressFeedbackDlg dlgProgress(0, FALSE);
			dlgProgress.ShowProgress(SW_SHOW);
			dlgProgress.SetCaption("Downloading NexTech " + m_pdlgMaster->GetImportTypeText() + " Content");

			//While downloading, the screen is in a sort of pseudo-async mode.  Messages are being pumped
			//	to get the download progress, so we need to be sure the user cannot change anything on 
			//	screen.
			EnableDownloadButtons(FALSE);

			//This will do the download and pump messages for the progress.  This will block until the download completes.
			FtpUtils::DownloadFileNxCompressedIncremental(pFtp, "NxCycleOfCare.dat", GetSharedPath() ^ "NxCycleOfCare.dat", TRUE, FtpUtils::eroExpectNotReadOnly, this, &dlgProgress);

			//Now that we're successful, update the path with what we just downloaded.
			SetDlgItemText(IDC_NXCOC_PATH, GetSharedPath() ^ "NxCycleOfCare.dat");
			m_bNeedToLoadContent = true;
		}
	}NxCatchAllCall("Error downloading NxCycleOfCare content file", EnableDownloadButtons(TRUE));

	//Cleanup of all FTP objects
	try {
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

	}NxCatchAll("Error cleaning up after NxCycleOfCare content file download");
}

BOOL CNxCoCMainSheet::Validate()
{
	//Set the type of import that we are trying to do
	if(IsDlgButtonChecked(IDC_NXCOC_USE_PIC)) {
		m_pdlgMaster->m_ImportInfo.SetType(eUsePIC);
	}
	else if(IsDlgButtonChecked(IDC_NXCOC_USE_LW)) {
		m_pdlgMaster->m_ImportInfo.SetType(eUseLW);
	}
	else {
		MessageBox("Please select which type of import you wish to perform.");
		return FALSE;
	}

	//The content must be loaded before we can validate this sheet
	if(m_bNeedToLoadContent) {
		CString strFile;
		GetDlgItemText(IDC_NXCOC_PATH, strFile);

		if(FileUtils::DoesFileOrDirExist(strFile)) {
			//We'll open the file once and read out the header information.  This lets us ensure that
			//	we're actually looking at an NxCoC file, and the user didn't give us some nonsense in place.
			long nContentFileVersion = 0, nNxCoCVersion = 0;
			COleDateTime dtWritten;
			if(m_pdlgMaster->m_ImportInfo.ReadVersionInfoFromNxCoCContentFile(strFile, nContentFileVersion, nNxCoCVersion, dtWritten)) {
				//Ensure the versions are acceptable
				if(NXCOC_OUTPUT_FILE_VERSION < nContentFileVersion) {
					//This .exe only knows about version x, but the content file is version x + 1 or greater.  We
					//	cannot import this content.
					MessageBox("Your current version of NexTech is out of date with this content. Please contact "
						"NexTech technical support about upgrading Practice.");
					return FALSE;
				}
				else if(NXCOC_OUTPUT_FILE_VERSION > nContentFileVersion) {
					//This .exe expects version x, but the content file is version x- 1 or lesser.  There's no reason we cannot support
					//	this.  We just need to make sure that each addition of new structure change is properly handled in the parsing
					//	functionality with if(nContentFileVersion >= 2) {...} etc.   Consider this a mandate for all future changes.
				}

				//Put up a window that says we're doing something and gives the user how far to completion.
				CShowProgressFeedbackDlg dlgProgress(0, TRUE);
				dlgProgress.ShowProgress(SW_SHOW);
				dlgProgress.SetCaption("Loading NexTech " + m_pdlgMaster->GetImportTypeText() + " Content...");

				//Load the entire content file into our memory structure.  This will open the file, and fill out all the memory
				//	structures in the m_ImportInfo object.
				if(!m_pdlgMaster->m_ImportInfo.LoadFromNxCoCContentFile(strFile, &dlgProgress)) {
					return FALSE;
				}
				m_bNeedToLoadContent = false;

				//Once loaded, don't let them come back to this sheet
				m_bNoReturn = TRUE;
			}
			else {
				MessageBox(FormatString("'%s' is not a valid NexTech " + m_pdlgMaster->GetImportTypeText() + " content file.", strFile), "Invalid File");
				return FALSE;
			}
		}
		else {
			//The content file is not there.
			MessageBox("Please select a valid content file before continuing.");
			return FALSE;
		}
	}

	return TRUE;
}

//Message handler for notification that the download is beginning.  wParam is the size of the file.
LRESULT CNxCoCMainSheet::OnTransferBegin(WPARAM wParam, LPARAM lParam)
{
	try {
		m_dwCompressedFileSize = (DWORD)wParam;

	}NxCatchAll("Error in OnTransferBegin");

	//Check the status of the general destruction handle to see if we should quit.
	return (WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_hevDestroying, 0) == WAIT_TIMEOUT);
}

//Message handler for notification of progres.  We will continually get these messages while the file
//	is downloading, so we can properly update our progress dialog.  lparam is the dialog itself, 
//	wParam is the number of bytes downloaded so far.
LRESULT CNxCoCMainSheet::OnTransferProgress(WPARAM wParam, LPARAM lParam)
{
	try {
		//Format a nice string that tells the user how many bytes we've downloaded and
		//	what the percentage works out to.
		CShowProgressFeedbackDlg *pProgress = (CShowProgressFeedbackDlg*)lParam;
		DWORD dwBytesDownloaded = (DWORD)wParam;
		pProgress->SetProgress(0, m_dwCompressedFileSize, dwBytesDownloaded);
		if(dwBytesDownloaded >= m_dwCompressedFileSize) {
			pProgress->SetCaption("Extracting file");
		}
		else {
			pProgress->SetCaption(FormatString("Download %.0f%% complete (%.2f of %.2f MB)", dwBytesDownloaded / (float)m_dwCompressedFileSize * 100., dwBytesDownloaded / 1024. / 1024., m_dwCompressedFileSize / 1024. / 1024.));
		}

	}NxCatchAllCall("Error in OnTransferProgress", EnableDownloadButtons(TRUE));

	//If the user pressed cancel during the download, this event will be set.  We'll use its status to stop the download
	//	here if that happens.
	return (WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_hevDestroying, 0) == WAIT_TIMEOUT);
}

//Message handler to signify that the download has finished.
LRESULT CNxCoCMainSheet::OnTransferEnd(WPARAM wParam, LPARAM lParam)
{
	try {
		EnableDownloadButtons(TRUE);

	}NxCatchAll("Error in OnTransferEnd");

	//The transfer is already complete, so always return TRUE here.  NexForms checks for the cancel event , but if they've already
	//	reached this point, we may as well let any last download cleanup (uncompress?) happen.  That way they don't have to 
	//	download the entire file again later.
	return TRUE;
}

//General helper function to limit user access to the interface while a download is in progress.  Any buttons / controls which
//	can cause "work" to be done should be included here.
void CNxCoCMainSheet::EnableDownloadButtons(BOOL bEnable)
{
	m_pdlgMaster->GetDlgItem(IDC_NXCOC_NEXT)->EnableWindow(bEnable);
	GetDlgItem(IDC_NXCOC_DOWNLOAD)->EnableWindow(bEnable);
	GetDlgItem(IDC_NXCOC_BROWSE)->EnableWindow(bEnable);
	GetDlgItem(IDC_NXCOC_PATH)->EnableWindow(bEnable);
}
