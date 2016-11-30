// SelectImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectImageDlg.h"
#include "GlobalUtils.h"
#include "NxTwain.h"
#include "Mirror.h"
#include "FileUtils.h"
#include "MergeEngine.h"
#include "NxWIA.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectImageDlg dialog


CSelectImageDlg::CSelectImageDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectImageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// (a.walling 2008-09-23 11:22) - PLID 31479 - This is really not necessary
	//m_hevShuttingDown = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_strMainTabName = "Anatomical Diagrams";

	m_pPicContainer = NULL;
}

CSelectImageDlg::~CSelectImageDlg()
{
	// (a.walling 2008-09-23 11:22) - PLID 31479 - Event removed
	//CloseHandle(m_hevShuttingDown);
}

void CSelectImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectImageDlg)
	//DDX_Control(pDX, IDC_PATIENT_PHOTOS, m_PatientPhotoViewer);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PATIENT_PHOTOS, m_nxstaticPatientPhotos);
	DDX_Control(pDX, IDC_SHOW_INTERFACE, m_btnShowInterface);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectImageDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectImageDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_TWAIN_IMPORT, OnTwainImport)
	ON_MESSAGE(NXM_TWAIN_XFERDONE, OnTwainXferdone)
	ON_BN_CLICKED(IDC_SHOW_INTERFACE, OnShowInterface)
	ON_BN_CLICKED(IDC_IMPORT_FILE, OnImportFile)
	ON_MESSAGE(NXM_PHOTO_LOADED, OnPhotoLoaded)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectImageDlg message handlers

BOOL CSelectImageDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 14:58) - PLID 29793 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.dinatale 2012-02-01 10:17) - PLID 23369 - dont check mirror if the patientID is less than 0
		if (m_nPatientID > 0 && Mirror::IsMirrorEnabled()) {
			// (c.haag 2005-03-16 10:44) - We have to force this dialog to open Mirror at
			// the get-go, otherwise the photo viewer control will have quirky problems
			// with Mirror caused by Canfield's implementation. This quirkiness is related to
			// the photo viewer calling several Mirror functions too quickly in succession if
			// the first function has to create an instance of Mirror, because the successive
			// calls may unintentionally cause Mirror to try to instantiate again and again.
			// Ultimately, Practice will freeze up.
			// (c.haag 2009-03-31 13:55) - PLID 33630 - I have my doubts that this is the case
			// any longer; but I'll keep the behavior intact
			//Mirror::IsMirror61(TRUE);
			Mirror::InitCanfieldSDK(TRUE);
		}

		// (z.manning, 06/07/2007) - PLID 23862 - If we didn't set an image path, use the default.
		if(m_strImagePath.IsEmpty()) {
			m_strImagePath = GetSharedPath() ^ "Images";
		}
		m_PatientPhotoViewer.m_strImagePath = m_strImagePath;

		m_PatientPhotoViewer.m_bShowPreviewArea = false;
		m_PatientPhotoViewer.m_bAllowMultiSelect = false;
		m_PatientPhotoViewer.m_bShowMirror = true;
		m_PatientPhotoViewer.m_bShowUnited = false;

		CRect rPhotoViewer;
		GetDlgItem(IDC_PATIENT_PHOTOS)->GetClientRect(rPhotoViewer);
		GetDlgItem(IDC_PATIENT_PHOTOS)->ClientToScreen(rPhotoViewer);
		ScreenToClient(rPhotoViewer);

		m_PatientPhotoViewer.Create(NULL, "", WS_VISIBLE|WS_CHILD, rPhotoViewer, this, 100);
		m_PatientPhotoViewer.PrepareWindow();
		
		m_PatientPhotoViewer.SetPersonID(m_nPatientID);
		// (j.jones 2013-09-19 15:19) - PLID 58547 - added a pointer to the pic
		m_PatientPhotoViewer.SetPicContainer(m_pPicContainer);

		GetDlgItem(IDC_PATIENT_PHOTOS)->DragAcceptFiles(TRUE);

		//DRT 5/8/2008 - PLID 29875 - Dropped 'this', now an NxDialog
		m_tab = GetDlgItemUnknown(IDC_IMAGE_TYPE);
		if (m_tab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return 0;
		}
		else {
			if(m_nPatientID == -1) {
				m_tab->PutTabWidth(3);
				m_tab->PutSize(1);
				m_tab->PutLabel(0, _bstr_t(m_strMainTabName));
				GetDlgItem(IDC_TWAIN_IMPORT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SHOW_INTERFACE)->ShowWindow(SW_HIDE);
			}
			else{
				m_tab->PutTabWidth(3);
				m_tab->PutSize(2);
				m_tab->PutLabel(0, "Patient's History");
				m_tab->PutLabel(1, _bstr_t(m_strMainTabName));
			}

			// (j.jones 2016-04-20 11:01) - NX-100214 - Set HeaderMode to false, which will
			// use a slightly different theme than the module tabs use.
			// A HeaderMode of false looks nicer when the tab is next to a datalist.
			m_tab->HeaderMode = false;
		}

		CheckDlgButton(IDC_SHOW_INTERFACE, GetPropertyInt("TWAINShowUI", 1, 0, true));
	}
	NxCatchAll("Error in CSelectImageDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectImageDlg::OnOK() 
{
	// (a.walling 2008-09-23 11:22) - PLID 31479 - We do not need to wait here, the photoviewer
	// can take care of itself.
	/*
	if (WAIT_TIMEOUT != WaitForSingleObject(m_hevShuttingDown, 0)) {
		CDialog::OnOK();
		return;
	}
	*/

	// (j.jones 2009-10-13 13:10) - PLID 35894 - converted to be pointers
	// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
	CArray<ImageInformationPtr, ImageInformationPtr> arSelected;
	m_PatientPhotoViewer.GetSelectedImages(arSelected);
	if(arSelected.GetSize() == 0) {
		MsgBox("Please select an image before continuing.");
		return;
	}
	ASSERT(arSelected.GetSize() == 1);
	
	// (j.jones 2013-09-19 14:52) - PLID 58547 - this variable has been renamed to strFullFilePath
	m_strFileName = arSelected.GetAt(0)->strFullFilePath;
	CString strExpected = m_nPatientID == -1 ? "INVALIDUNLIKELYIMAGENAME123@!@#" : GetPatientDocumentPath(m_nPatientID);
	if (arSelected.GetAt(0)->nType == eImageSrcMirror) {
		m_nImageType = itMirrorImage;
	}
	else if(m_nPatientID != -1 && strExpected.CompareNoCase(m_strFileName.Left(strExpected.GetLength())) == 0) {
		m_strFileName = m_strFileName.Mid(strExpected.GetLength());
		m_nImageType = itPatientDocument;
	}
	else if(m_strFileName.GetAt(0) == '\\' && m_strFileName.GetAt(1) != '\\') {
		m_nImageType = itPatientDocument;
	}
	//TES 4/22/2004: Patient documents used (I thought) to start with a single \, now sometimes they don't?
	else if(m_strFileName.Find("\\") == -1) {
		m_nImageType = itPatientDocument;
	}
	else {
		strExpected = m_strImagePath;
		if(strExpected.CompareNoCase(m_strFileName.Left(strExpected.GetLength())) == 0) {
			m_strFileName = m_strFileName.Mid(strExpected.GetLength());
			m_nImageType = itDiagram;
		}
		else {
			m_nImageType = itAbsolutePath;
		}
	}

	// (a.walling 2008-09-23 11:22) - PLID 31479 - The photoviewer can take care of itself.
	/*
	if (!m_PatientPhotoViewer.IsLoadingImages()) {
		CDialog::OnOK();
		return;
	}
	m_wCloseCommand = IDOK;
	SetEvent(m_hevShuttingDown);
	*/

	
	CDialog::OnOK();
}

void CSelectImageDlg::OnCancel() 
{
	// (a.walling 2008-09-23 11:22) - PLID 31479 - The photoviewer can take care of itself.
	/*
	if (!m_PatientPhotoViewer.IsLoadingImages() ||
		WAIT_TIMEOUT != WaitForSingleObject(m_hevShuttingDown, 0)) {
		CDialog::OnCancel();
		return;
	}
	m_wCloseCommand = IDCANCEL;
	SetEvent(m_hevShuttingDown);	
	*/
	CDialog::OnCancel();
}

LRESULT CSelectImageDlg::OnPhotoLoaded(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2008-09-23 11:22) - PLID 31479 - The photoviewer can take care of itself.
	/*
	if (WAIT_TIMEOUT != WaitForSingleObject(m_hevShuttingDown, 0)) {
		m_PatientPhotoViewer.AbortImageLoading();
		PostMessage(WM_COMMAND, m_wCloseCommand);
	}
	*/
	return 0;
}

void CSelectImageDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}

BEGIN_EVENTSINK_MAP(CSelectImageDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectImageDlg)
	ON_EVENT(CSelectImageDlg, IDC_IMAGE_TYPE, 1 /* SelectTab */, OnSelectTabImageType, VTS_I2 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectImageDlg::OnSelectTabImageType(short newTab, short oldTab) 
{
	if(newTab == 0) {
		m_PatientPhotoViewer.SetPersonID(m_nPatientID);
	}
	else {
		m_PatientPhotoViewer.SetPersonID(-1);
	}
}

void CSelectImageDlg::OnTwainImport() 
{
	try {
		// (a.walling 2008-09-23 13:04) - PLID 31486 - Option to import from WIA device
		if (NxWIA::IsWIAAvailable(GetRemotePropertyInt("WIA_CameraDevicesOnly", TRUE, 0, GetCurrentUserName(), true))) {
			// present option for WIA or TWAIN
			CMenu mnu;
			if (mnu.CreatePopupMenu()) {
				enum EImportOptions {
					eTWAIN = 1,
					eWIA = 2,
				};

				long nPos = 0;
				mnu.InsertMenu(nPos++, 0, eTWAIN, "Import from &TWAIN device");
				mnu.InsertMenu(nPos++, 0, eWIA, "Import from &WIA device");

				CPoint pt;
				GetMessagePos(pt);
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this);

				switch (nResult) {
				case eTWAIN:
					ImportFromTWAIN();
					break;
				case eWIA:
					ImportFromWIA();
					break;
				}
			}
		} else {
			// TWAIN only
			ImportFromTWAIN();
		}
	} NxCatchAll("CSelectImageDlg::OnTwainImport():AddSelectImageDlgWaiting");

}

// (a.walling 2008-09-23 12:59) - PLID 31486 - Import from WIA
void CSelectImageDlg::ImportFromWIA()
{
	try {
		WIA::IDevicePtr pDevice = NxWIA::GetDefaultDevice(GetRemotePropertyInt("WIA_CameraDevicesOnly", TRUE, 0, GetCurrentUserName(), true));

		WIA::IItemsPtr pItems = NxWIA::GetSelectedItems(pDevice);
		if (pItems) {

			CStringArray saTempFiles;

			WIA::ICommonDialogPtr pCommonDialog = NxWIA::GetCommonDialog();

			int i = 0;
			for (i = 1; i <= pItems->Count; i++) {
				WIA::IItemPtr pItem = pItems->Item[i];

				if (pItem) {
					WIA::IImageFilePtr pImage;

					if (pCommonDialog) {
						pImage = pCommonDialog->ShowTransfer(pItem, WIA::wiaFormatJPEG, VARIANT_FALSE);
					} else {
						pImage = pItem->Transfer(WIA::wiaFormatJPEG);
					}

					if (pImage) {
						CString strFileName = GetNxTempPath() ^ FormatString("ScannedWIATempFile%lu.%s", GetUniqueSessionNum(), (LPCTSTR)pImage->GetFileExtension());
						
						pImage->SaveFile(_bstr_t(strFileName));
						saTempFiles.Add(strFileName);
					}
				}
			}
			
			CString strErrors;
			for (i = 0; i < saTempFiles.GetSize(); i++) {
				CString strFileName = saTempFiles[i];
				CString strExt = FileUtils::GetFileExtension(saTempFiles[i]);
				CString strTargetName = GetPatientDocumentPath(m_nPatientID) ^ GetPatientDocumentName(m_nPatientID, strExt);
				
				if (GetRemotePropertyInt("TWAINScanToDocumentFolder",1,0,"<None>",TRUE)) {
					if (CopyFile(strFileName, strTargetName, TRUE)) {
						AttachToMailSent(m_nPatientID, strTargetName, FALSE, 0, SELECTION_FILE, -1, -1, -1);
					} else {
						strErrors += FormatLastError("Moving %s to %s: \r\n", strFileName, strTargetName);
					}
				}

				if (GetRemotePropertyInt("TWAINScanToRemoteFolder",0,0,"<None>",TRUE)) {
					// (a.walling 2010-01-28 14:15) - PLID 28806 - Requires a connection pointer now
					CString strUserDefinedFilename = NXTWAINlib::GetUserDefinedOutputFilename(GetRemoteData(), m_nPatientID, strExt);
					if (CopyFile(strFileName, strUserDefinedFilename, FALSE)) {								
						AttachToMailSent(m_nPatientID, strUserDefinedFilename, FALSE, 0, SELECTION_FILE, -1, -1, -1);
					} else {
						strErrors += FormatLastError("Moving %s to %s: \r\n", strFileName, strUserDefinedFilename);
					}
				}
			}

			if (!strErrors.IsEmpty()) {
				MessageBox(FormatString("Errors occurred while attaching to history:\r\n\r\n%s", strErrors), NULL, MB_OK|MB_ICONEXCLAMATION);
			}
			
			if(m_tab->GetCurSel() != 0) {
				m_tab->PutCurSel(0);
				OnSelectTabImageType(0, 1);
			}
			else {
				m_PatientPhotoViewer.Refresh();
			}
		}
	} NxCatchAll("CSelectImageDlg::ImportFromWIA");
}

// (a.walling 2008-09-23 12:59) - PLID 31486 - Import from TWAIN
void CSelectImageDlg::ImportFromTWAIN()
{
	try {
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame) {
			// (a.walling 2008-07-24 17:53) - PLID 30836 - Made this more generic. Also why do we do this twice?
			//pMainFrame->RegisterSelectImageDlgWaiting(this);
			pMainFrame->RegisterTwainDlgWaiting(this);
		}
			
		// (a.walling 2008-09-23 11:20) - PLID 31285 - This is causing issues since TWAIN always needs a valid window
		// in general we should never set this at all anymore
		//NXTWAINlib::SetTwainMessageRecipient(GetSafeHwnd());
		NXTWAINlib::Acquire(m_nPatientID, GetPatientDocumentPath(m_nPatientID), NULL, OnNxTwainPreCompress, NULL);
	} NxCatchAll("CSelectImageDlg::ImportFromTWAIN");
}

LRESULT CSelectImageDlg::OnTwainXferdone(WPARAM wParam, LPARAM lParam)
{
	// (b.cardillo 2006-04-05 16:33) - We called SetTwainMessageRecipient() from OnTwainImport(), 
	// which should cause NxTwain to send us this message when it's done with the transfer.  But 
	// thanks to some robustness issues in NxTwain, it doesn't send us the message.  In fact it 
	// ONLY sends it to mainframe, hence the bug described by PLID 17605.  NxTwain has to be 
	// fixed so that it sends messages correctly again, but to do that a lot more of it has to 
	// be cleaned there first.  I've created pl item 20009 to tackle all this clean-up.  In the 
	// mean time, for pl item 17605, I've simply made mainframe forward the NXM_TWAIN_XFERDONE 
	// message to us directly; that way we still receive it and know to refresh.  Once we've 
	// received it we can remove ourselves from the set of windows requesting the message.
	try {
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame) {
			// (a.walling 2008-07-24 17:53) - PLID 30836 - Made this more generic. Also why do we do this twice?
			//pMainFrame->UnregisterSelectImageDlgWaiting(this);
			pMainFrame->UnregisterTwainDlgWaiting(this);
		}
	} NxCatchAll("CSelectImageDlg::OnTwainImport():AddSelectImageDlgWaiting");


	if(m_tab->GetCurSel() != 0) {
		m_tab->PutCurSel(0);
		OnSelectTabImageType(0, 1);
	}
	else {
		m_PatientPhotoViewer.Refresh();
	}

	// (a.walling 2008-09-23 11:20) - PLID 31285 - This is causing issues since TWAIN always needs a valid window
	// in general we should never set this at all anymore
	//NXTWAINlib::SetTwainMessageRecipient(NULL);
	return 0;
}

void CSelectImageDlg::OnShowInterface() 
{
	SetPropertyInt("TWAINShowUI", IsDlgButtonChecked(IDC_SHOW_INTERFACE), 0);
}

// (c.haag 2004-04-30 10:14) - Copied straight from fileutils.cpp
bool DoesFileOrDirExist(LPCTSTR strFile)
{
	// Only search if strFile is not NULL
	if (strFile) {
		CFileFind f;
		// First check to see if strFile exists, and if not, check to see if strFile\*.* exists
		if (f.FindFile(strFile) || f.FindFile(CString(strFile) ^ _T("*.*"))) {
			return true;
		}
	}
	
	// If we made it to here we have failed (NULL strFile also indicates failure)
	return false;
}

bool CreatePath(LPCTSTR strPath)
{
	// First make sure parent directory exits
	CString strTmp = GetFilePath(strPath);
	if (!strTmp.IsEmpty()) {
		if (!DoesFileOrDirExist(strTmp)) {
			CreatePath(strTmp);
		}
	}
	
	// Then create the specified path
	if (CreateDirectory(strPath, NULL)) {
		return true;
	} else {
		return false;
	}
}

void CSelectImageDlg::OnImportFile() 
{
	try {
		CString strDestFolder;
		bool bPatientDocument = false;
		if(m_nPatientID > 0 && m_tab->GetCurSel() != 1) {
			strDestFolder = GetPatientDocumentPath(m_nPatientID);
			bPatientDocument = true;
		}
		else {
			strDestFolder = m_strImagePath;
		}

		// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, 
			"Image (*.bmp, *.jpg, *.pcx, *.tiff)|*.bmp;*.jpg;*.pcx;*.tiff|"
			"|"
			, this);
		if(dlg.DoModal() == IDOK) {
			POSITION p = dlg.GetStartPosition();
			while (p) {
				CString strSourcePath = dlg.GetNextPathName(p);
				CString strDstPath = strDestFolder ^ GetFileName(strSourcePath);

				// (c.haag 2004-04-30 10:14) - Ensure the folder exists
				if (!DoesFileOrDirExist(strDestFolder))
					CreatePath(strDestFolder);

				if (CopyFile(strSourcePath, strDstPath, TRUE)) {
					if(bPatientDocument) 
						AttachToMailSent(m_nPatientID, GetFileName(strDstPath));
						
				}
				else {
					if(bPatientDocument) {
						MsgBox("The file '%s' could not be imported into the patient's documents folder. The file either does not exist, or a file with the same name already exists in the patient's documents folder.", strSourcePath);
					}
					else {
						MsgBox("The file '%s' could not be imported into the %s folder. The file either does not exist, or a file with the same name already exists in the %s folder.", strSourcePath, m_strMainTabName, m_strMainTabName);
					}
				}
			}
		}
		m_PatientPhotoViewer.Refresh();
	}NxCatchAll("Error in CSelectImageDlg::OnImportFile()");
}

BOOL CSelectImageDlg::PreTranslateMessage(MSG *pMsg)
{
	if( (pMsg->hwnd == m_PatientPhotoViewer.GetSafeHwnd() || pMsg->hwnd == GetDlgItem(IDC_PATIENT_PHOTOS)->GetSafeHwnd())
		&& pMsg->message == WM_DROPFILES) {
		OnDropFiles((HDROP)pMsg->wParam);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CSelectImageDlg::OnDropFiles(HDROP hDropInfo)
{
	SetForegroundWindow();

	CString strFileName;
	int nFileCount = DragQueryFile(hDropInfo, -1, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
	strFileName.ReleaseBuffer();
	
	//OK, first, go through and load our folder structure.
	FileFolder fRoot;
	fRoot.strFolderName = "";

	for(int i = 0; i < nFileCount; i++) {
		DragQueryFile(hDropInfo, i, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
		strFileName.ReleaseBuffer();
		//Is it a directory?
		CFileFind ff;
		if(ff.FindFile(strFileName)) {
			ff.FindNextFile();
			if(ff.IsDirectory()) {
				fRoot.arSubFolders.Add(LoadFolder(strFileName));
			}
			else {
				fRoot.saFileNames.Add(strFileName);
			}
		}
	}

	//OK, let's make a message box out of it.
	CString strFileNames;
	int nFilesAdded = 0;
	GetFileNameList(fRoot, strFileNames, "", nFilesAdded, 10);

	if(m_tab->CurSel == 0 && m_nPatientID != -1) {
		CString strMessage = "Are you sure you want to import the following files into this patient's default documents folder?"
			"\r\n"
			"\r\n"
			+ strFileNames;

		if(IDYES == MsgBox(MB_YESNO, "%s", strMessage)) {
			CWaitCursor cuWait;
			ImportAndAttachFolder(fRoot, m_nPatientID, GetSafeHwnd());
			m_PatientPhotoViewer.Refresh();
		}
	}
	else {
		CString strMessage = FormatString("Are you sure you want to import the following files into the global %s folder?\r\n\r\n", m_strMainTabName) + strFileNames;
		if(IDYES == MsgBox(MB_YESNO, "%s", strMessage)) {
			CWaitCursor cuWait;
			WriteFolderToDisk(fRoot, m_strImagePath);
			m_PatientPhotoViewer.Refresh();
		}
	}

	DragFinish(hDropInfo);

}

void CSelectImageDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();
	
	try {
		//DRT 6/2/2008 - PLID 28229 - This is created manually in OnInitDialog, so we must manually destroy it.
		m_PatientPhotoViewer.DestroyWindow();
	} NxCatchAll("Error in OnDestroy");
}
