// EMRPreviewPopupDlg.cpp : implementation file
//

// (a.walling 2007-04-16 14:30) - PLID 25648 - Implement a preview popup dialog for NexEMR

#include "stdafx.h"
#include "EMRPreviewPopupDlg.h"
#include "fileutils.h"
#include "NxCDO.h"
#include "EMRPreviewMultiPopupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewPopupDlg dialog


CEMRPreviewPopupDlg::CEMRPreviewPopupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRPreviewPopupDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_pEMRPreviewCtrlDlg = NULL;
	m_pNexEMRDlg = NULL;
	m_nCurEMNIndex = -1;
	// (a.walling 2011-06-17 15:43) - PLID 42367
	memset(&m_ftCurEMNLastWriteTime, 0, sizeof(m_ftCurEMNLastWriteTime));
}

CEMRPreviewPopupDlg::~CEMRPreviewPopupDlg()
{
	// destructor. clean up everything as in OnDestroy just to be safe.
	// and ASSERT since this should only happen if the preview is poorly
	// implemented.
	try {
		if(m_pEMRPreviewCtrlDlg) {
			ASSERT(FALSE);
			m_pEMRPreviewCtrlDlg->DestroyWindow();
			delete m_pEMRPreviewCtrlDlg;
			m_pEMRPreviewCtrlDlg = NULL;
		}

		if (m_strTempFile.GetLength() > 0) {
			ASSERT(FALSE);
			DeleteFileWhenPossible(m_strTempFile);
			m_strTempFile.Empty();
		}
	} NxCatchAll("Error in CEMRPreviewPopupDlg::~CEMRPreviewPopupDlg");
}


void CEMRPreviewPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRPreviewPopupDlg)
	DDX_Control(pDX, IDC_BTN_EMR_PREVIEW_PREV, m_btnPrevious);
	// (a.walling 2010-01-11 15:43) - PLID 36837 - close button
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_BTN_EMR_PREVIEW_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_PREVIEW_AREA, m_nxstaticPreviewArea);
	DDX_Control(pDX, IDC_LABEL_OPEN_THIS_EMN, m_nxlOpenThisEMN);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRPreviewPopupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRPreviewPopupDlg)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_EMR_PREVIEW_PREV, OnBtnEmrPreviewPrev)
	ON_BN_CLICKED(IDC_BTN_EMR_PREVIEW_NEXT, OnBtnEmrPreviewNext)
	// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined messages with EMRPREVIEW_ rather than a registered message
	ON_MESSAGE(NXM_EMRPREVIEW_PRINT_MULTIPLE, OnPrintMultiple)
	// (a.walling 2010-01-11 18:02) - PLID 36840 - Handle a custom preview command
	ON_MESSAGE(NXM_EMRPREVIEW_CUSTOM_COMMAND, OnCustomEmrPreviewCommand)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewPopupDlg message handlers

BOOL CEMRPreviewPopupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (j.jones 2009-09-22 11:19) - PLID 31620 - added previous/next buttons
		m_btnPrevious.AutoSet(NXB_LEFT);
		m_btnNext.AutoSet(NXB_RIGHT);
		// (a.walling 2010-03-24 12:58) - PLID 37677
		m_nxlOpenThisEMN.SetType(dtsHyperlink);
		m_nxlOpenThisEMN.SetHzAlign(DT_CENTER);
		m_nxlOpenThisEMN.SetColor(CNxDialog::GetSolidBackgroundColor());
		CString strLabelText;
		m_nxlOpenThisEMN.GetWindowText(strLabelText);
		m_nxlOpenThisEMN.SetText(strLabelText);

		// (a.walling 2010-01-11 15:43) - PLID 36837 - close button
		m_btnClose.AutoSet(NXB_CLOSE);
		
		if (m_pEMRPreviewCtrlDlg == NULL) {
			CRect rRect;
			// (j.jones 2009-09-22 11:22) - PLID 31620 - use IDC_PREVIEW_AREA,
			// not the full client area
			GetDlgItem(IDC_PREVIEW_AREA)->GetWindowRect(rRect);
			ScreenToClient(rRect);

			m_pEMRPreviewCtrlDlg = new CEMRPreviewCtrlDlg;
			m_pEMRPreviewCtrlDlg->SetInteractive(FALSE);
			// (a.walling 2012-11-05 11:58) - PLID 53588 - CEMRPreviewCtrlDlg now expecting a control ID
			m_pEMRPreviewCtrlDlg->Create(rRect, this, -1);
			m_pEMRPreviewCtrlDlg->EnsureCSSFile();
		}

		// (a.walling 2007-07-19 09:55) - PLID 26261 - Use NxTempPath
		// (a.walling 2007-07-25 11:39) - PLID 26261 - This does not contain sensitive client info, but we'll prefix it with the common
		// temp prefix anyway
		HANDLE hFile = CreateNxTempFile("nexemrt_NexEMRTemp", "html", &m_strTempFile);
		if (hFile == INVALID_HANDLE_VALUE)
			ThrowNxException("Could not create temp file!");
		else
			CloseHandle(hFile); // we have saved the file in m_strTempFile for later.

	} NxCatchAll("Error in CEMRPreviewPopupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRPreviewPopupDlg::OnDestroy() 
{
	try {		
		// (a.walling 2010-01-11 12:38) - PLID 36837 - Save the size
		if (!IsIconic()) {
			// get the window rect and save to preferences.
			CRect rRect;
			GetWindowRect(rRect);

			if ( (rRect.Width() > 50) && (rRect.Height() > 50) ) {
				
				CString strSubsectionSuffix;
				if (!m_strSubsection.IsEmpty()) {
					strSubsectionSuffix = CString("_") + m_strSubsection;
				}

				// ensure a reasonable rect
				SetRemotePropertyInt(FormatString("EMRPreviewPopupWidth%s", strSubsectionSuffix), rRect.Width(), 0, GetCurrentUserName());
				SetRemotePropertyInt(FormatString("EMRPreviewPopupHeight%s", strSubsectionSuffix), rRect.Height(), 0, GetCurrentUserName());
				SetRemotePropertyInt(FormatString("EMRPreviewPopupTop%s", strSubsectionSuffix), rRect.top, 0, GetCurrentUserName());
				SetRemotePropertyInt(FormatString("EMRPreviewPopupLeft%s", strSubsectionSuffix), rRect.left, 0, GetCurrentUserName());
			} else {
				ASSERT(FALSE);
			}
		}

		if(m_pEMRPreviewCtrlDlg) {
			m_pEMRPreviewCtrlDlg->DestroyWindow();
			delete m_pEMRPreviewCtrlDlg;
			m_pEMRPreviewCtrlDlg = NULL;
		}

		if (m_strTempFile.GetLength() > 0) {
			DeleteFileWhenPossible(m_strTempFile);
			m_strTempFile.Empty();
		}

		CleanupTempFiles();
	} NxCatchAll("Error in CEMRPreviewPopupDlg::OnDestroy");

	CDialog::OnDestroy();
}

void CEMRPreviewPopupDlg::OnOK()
{
	ShowWindow(SW_HIDE);
	if (m_pNexEMRDlg)
		m_pNexEMRDlg->SetShowPreview(FALSE);
}

void CEMRPreviewPopupDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
	if (m_pNexEMRDlg)
		m_pNexEMRDlg->SetShowPreview(FALSE);
}

void CEMRPreviewPopupDlg::RefreshPreview()
{
	// (a.walling 2007-10-01 11:44) - PLID 25648 - Simply call PreviewEMN with the current ID
	// (j.jones 2009-09-22 11:55) - PLID 31620 - changed to just call DisplayCurrentEMNPreview()
	DisplayCurrentEMNPreview();
}

void CEMRPreviewPopupDlg::SetPatientID(long nNewPatientID, EmnPreviewPopup emn)
{
	CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
	aryEMNs.Add(emn);
	SetPatientID(nNewPatientID, aryEMNs);
}

// (j.jones 2009-09-22 12:37) - PLID 31620 - takes in a list of EMNIDs
// available for this patient
// (z.manning 2012-09-10 14:39) - PLID 52543 - Now uses the new EmnPreviewPopup object
void CEMRPreviewPopupDlg::SetPatientID(long nNewPatientID, CArray<EmnPreviewPopup, EmnPreviewPopup&> &aryEMNs)
{
	if (nNewPatientID != m_nPatientID) {
		m_nPatientID = nNewPatientID;
		// (a.walling 2007-10-01 11:45) - PLID 25648 - Reset the preview to no EMN
		// (j.jones 2009-09-22 11:55) - PLID 31620 - pass aryEMNIDs to PreviewEMN
		CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
		PreviewEMN(aryEMNs, -1);
	}
}

void CEMRPreviewPopupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED) {
		if (m_pEMRPreviewCtrlDlg) {

			CRect rRect;
			// (j.jones 2009-09-22 11:22) - PLID 31620 - use IDC_PREVIEW_AREA,
			// not the full client area
			GetDlgItem(IDC_PREVIEW_AREA)->GetWindowRect(rRect);
			ScreenToClient(rRect);

			m_pEMRPreviewCtrlDlg->MoveWindow(rRect);
		}
	}
}

BOOL CEMRPreviewPopupDlg::PreviewEMN(EmnPreviewPopup emn, const long nCurIndex)
{
	CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
	aryEMNs.Add(emn);
	return PreviewEMN(aryEMNs, nCurIndex);
}

// (j.jones 2009-09-22 11:44) - PLID 31620 - Since the preview popup has
// previous & next buttons, we need to know what EMNs are available to
// iterate through, and what order. These EMN IDs should be passed in
// through aryEMNIDs. The EMN we wish to display should be referenced
// through nCurIndex, indicating which index of the array is displayed.
// (z.manning 2012-09-10 14:39) - PLID 52543 - Now uses the new EmnPreviewPopup object
BOOL CEMRPreviewPopupDlg::PreviewEMN(CArray<EmnPreviewPopup, EmnPreviewPopup&> &aryEMNs, long nCurIndex)
{
	try {
		// (a.walling 2010-08-05 15:45) - PLID 40015
		EmnPreviewPopup currentEmn;
		if (m_nCurEMNIndex >= 0 && m_nCurEMNIndex < m_aryEMNs.GetSize() ) {
			currentEmn = m_aryEMNs[m_nCurEMNIndex];
		}

		if (nCurIndex == -1 && currentEmn.nID != 1) {
			// find the index of the currentEmn in the new array
			for (int i = 0; i < aryEMNs.GetSize(); ++i) {
				if (aryEMNs[i].nID == currentEmn.nID) {
					nCurIndex = i;
					break;
				}
			}
		}

		//populate our member variables
		m_aryEMNs.RemoveAll();
		m_aryEMNs.Append(aryEMNs);
		m_nCurEMNIndex = nCurIndex;

		// (a.walling 2010-08-05 15:45) - PLID 40015
		bool bSameEMN = false;
		if (m_nCurEMNIndex == -1 && currentEmn.nID != -1) {
			for (int i = 0; i < m_aryEMNs.GetSize(); i++) {
				if (m_aryEMNs[i].nID == currentEmn.nID) {
					m_nCurEMNIndex = i;
					bSameEMN = true;
					break;
				}
			}
		}

		//update our arrow buttons
		UpdateButtons();

		
		// (a.walling 2010-08-05 15:45) - PLID 40015
		//display our EMN
		if (!bSameEMN) {
			return DisplayCurrentEMNPreview();
		} else {
			// already displaying, don't want to interrupt
			return TRUE;
		}

	} NxCatchAll("Error in CEMRPreviewPopupDlg::PreviewEMN");

	return FALSE;
}

// (j.jones 2009-09-22 11:47) - PLID 31620 - will display the EMN
// preview for the EMN at m_nCurEMNIndex
BOOL CEMRPreviewPopupDlg::DisplayCurrentEMNPreview()
{
	try {

		// (j.jones 2009-09-22 11:50) - PLID 31620 - converted this function from the old PreviewEMN

		// (a.walling 2007-10-01 11:46) - PLID 25648 - Simplified this. We'll keep trying to load
		// the preview if it's a valid ID. If it is -1, then "Select an EMN".		

		EmnPreviewPopup emn;
		if(m_nCurEMNIndex != -1 && m_aryEMNs.GetSize() > m_nCurEMNIndex) {
			emn = m_aryEMNs.GetAt(m_nCurEMNIndex);
		}

		// (j.jones 2009-09-22 11:51) - PLID 31620 - disable/enable buttons
		UpdateButtons();

		if (emn.nID == -1) {
			// navigate to blank/default
			if (m_pEMRPreviewCtrlDlg) {
				NavigateToMessage(
					"<p>Please select an EMN for the preview to appear.</p>");
			}
			return TRUE;
		}

		// (a.walling 2009-11-23 11:47) - PLID 36396 - Shared method to decrypt a saved EMN Preview and return the path to the decrypted MHT file
		CString strTempDecryptedFile;

		CleanupTempFiles();

		// (a.walling 2011-06-17 15:43) - PLID 42367 - Update the last write time of the file and the actual path
		if (CEMRPreviewCtrlDlg::GetMHTFile(emn.nID, emn.dtModifiedDate, strTempDecryptedFile, &m_strCurEMNFileName, &m_ftCurEMNLastWriteTime)) {

			m_arTempFiles.Add(strTempDecryptedFile);

			if (m_pEMRPreviewCtrlDlg) {
				// (a.walling 2015-11-16 11:52) - PLID 67494 - If we are navigating to the same EMN, save the scroll position
				// when the document is loaded, this position will then be restored
				long nScrollTop = 0;

				if (m_pEMRPreviewCtrlDlg->GetDisplayedEMNID() == emn.nID) {
					// set 2nd param to TRUE to try to delete the temp file when load is complete.
					// (a.walling 2007-10-01 11:47) - PLID 25648 - Save navigate instead of navigate
					nScrollTop = m_pEMRPreviewCtrlDlg->GetScrollTop();
				}

				// set 2nd param to TRUE to try to delete the temp file when load is complete.
				// (a.walling 2007-10-01 11:47) - PLID 25648 - Save navigate instead of navigate
				m_pEMRPreviewCtrlDlg->SafeNavigate(COleVariant(strTempDecryptedFile + FormatString("?%lu", GetTickCount())), nScrollTop);


				// (a.walling 2008-11-14 08:40) - PLID 32024 - The displayed EMN is no longer valid.
				m_pEMRPreviewCtrlDlg->SetDisplayedEMNID(emn.nID);
				
				// (a.walling 2010-03-24 13:04) - PLID 37677 - We have a valid EMN
				m_nxlOpenThisEMN.SetType(dtsHyperlink);
				m_nxlOpenThisEMN.AskParentToRedrawWindow();

				return TRUE;
			}
		} else {
			// navigate to 'not exists'
			
			if (m_pEMRPreviewCtrlDlg) {
				// (a.walling 2007-10-01 12:04) - PLID 25648 - Updated message to remove 'saving'.. if the preview does not exist, it will
				// be created even without saving.
				// (j.jones 2009-09-22 12:43) - PLID 31620 - updated this message to at least include the EMN name
				// (a.walling 2010-01-11 18:03) - PLID 36840 - Include ability to create the preview. Also updated the format of this message a bit.

				CString strEMNDescription = "this EMN.";
				{
					ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Description FROM EMRMasterT WHERE ID = {INT}", emn.nID);
					if(!rs->eof) {
						strEMNDescription = ConvertToHTMLEmbeddable(FormatString("the EMN '%s.'", AdoFldString(rs, "Description")));
					}
					rs->Close();
				}

				CString strMessage;
				strMessage.Format(
					"<p>No preview is currently available for %s</p>"
					"<ul>"
					"<li>Network connectivity problems or security issues are the most common causes of this problem. Opening or saving an EMN will automatically ensure a saved preview is available.</li>"
					"<li>However, for older EMNs, the saved preview may have not yet been created.</li>"
					"<li><a href=\"nexemr://customcommand/?66\">Create and save a preview for this EMN now.</a></li>"
					"</ul>"
					, strEMNDescription
				);

				NavigateToMessage(strMessage);
				
				// (a.walling 2010-03-24 13:04) - PLID 37677 - We have a valid EMN (even though the preview is unavailable)
				m_nxlOpenThisEMN.SetType(dtsHyperlink);
				m_nxlOpenThisEMN.AskParentToRedrawWindow();

				return FALSE;
			}
		}

	} NxCatchAll("Error in CEMRPreviewPopupDlg::PreviewEMN");

	return FALSE;
}

void CEMRPreviewPopupDlg::NavigateToMessage(CString strMessage)
{
	try {
		// (a.walling 2010-01-12 08:41) - PLID 36840 - Updated this to match our EMR Preview Pane stylesheet a bit closer
		CString strFull;
		strFull.Format("<html><head>\r\n"
			"<style type=\"text/css\">\r\n"
			"body {margin: 0px; padding: 4px; background: #FFFBFB; font-family: Calibri, Arial, Verdana, Times New Roman, Book Antiqua, Palatino, Serif;}\r\n"
			"h2 {padding: 2px; margin: 2px; color: #800080;}\r\n"
			"</style>\r\n"
			"</head><body>"
			"<h2>EMN Preview</h2>"
			"<div>"
			"%s"
			"</div>"
			"</body></html>", strMessage);

		// (a.walling 2007-10-01 11:43) - PLID 25648 - Safely navigate to HTML text rather than rely on temp
		// files. This is much faster too.
		if (m_pEMRPreviewCtrlDlg) {
			m_pEMRPreviewCtrlDlg->SafeNavigateToHTML(strFull); 
		}
		
		// (a.walling 2010-03-24 13:04) - PLID 37677 - We have a valid EMN
		m_nxlOpenThisEMN.SetType(dtsDisabledHyperlink);
		m_nxlOpenThisEMN.AskParentToRedrawWindow();
	} NxCatchAll("Error in CEMRPreviewPopupDlg::NavigateToMessage");
}

void CEMRPreviewPopupDlg::CleanupTempFiles()
{
	for (int i = 0; i < m_arTempFiles.GetSize(); i++) {
		DeleteFileWhenPossible(m_arTempFiles.GetAt(i));
	}

	m_arTempFiles.RemoveAll();
}

// (j.jones 2009-09-22 11:19) - PLID 31620 - added previous/next buttons
void CEMRPreviewPopupDlg::OnBtnEmrPreviewPrev()
{
	try {

		// (a.walling 2009-11-23 16:20) - PLID 36404
		{				
			// (a.walling 2015-07-09 16:33) - PLID 66504 - Allow users to ignore and continue if the print template teardown did not fire properly
			long nRet = IDTRYAGAIN;
			while (m_pEMRPreviewCtrlDlg && m_pEMRPreviewCtrlDlg->IsPrinting() && nRet == IDTRYAGAIN) {
				nRet = MessageBox("Please wait for the current print job to complete and be sent to the spooler before changing the currently displayed record. This should complete in less than a minute.", nullptr, MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);
				if (nRet == IDCANCEL) {
					return;
				}
			}
		}

		if(m_nCurEMNIndex <= 0) {
			//we're already at the beginning of the list
			UpdateButtons();
			return;
		}

		//decrement our index, and display the newly current EMN
		m_nCurEMNIndex--;
		UpdateButtons();
		DisplayCurrentEMNPreview();

	}NxCatchAll("Error in CEMRPreviewPopupDlg::OnBtnEmrPreviewPrev");
}

// (j.jones 2009-09-22 11:19) - PLID 31620 - added previous/next buttons
void CEMRPreviewPopupDlg::OnBtnEmrPreviewNext()
{
	try {

		// (a.walling 2009-11-23 16:20) - PLID 36404
		{				
			// (a.walling 2015-07-09 16:33) - PLID 66504 - Allow users to ignore and continue if the print template teardown did not fire properly
			long nRet = IDTRYAGAIN;
			while (m_pEMRPreviewCtrlDlg && m_pEMRPreviewCtrlDlg->IsPrinting() && nRet == IDTRYAGAIN) {
				nRet = MessageBox("Please wait for the current print job to complete and be sent to the spooler before changing the currently displayed record. This should complete in less than a minute.", nullptr, MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);
				if (nRet == IDCANCEL) {
					return;
				}
			}
		}

		if(m_aryEMNs.GetSize() == m_nCurEMNIndex + 1) {
			//we're already at the end of the list
			UpdateButtons();
			return;
		}

		//increment our index, and display the newly current EMN
		m_nCurEMNIndex++;
		UpdateButtons();
		DisplayCurrentEMNPreview();

	}NxCatchAll("Error in CEMRPreviewPopupDlg::OnBtnEmrPreviewNext");
}

// (j.jones 2009-09-22 11:56) - PLID 31620 - enable/disable next/previous buttons
void CEMRPreviewPopupDlg::UpdateButtons()
{
	if(m_aryEMNs.GetSize() <= 1) {
		m_btnPrevious.EnableWindow(FALSE);
		m_btnNext.EnableWindow(FALSE);
		// (a.walling 2010-01-11 15:45) - PLID 36837 - Don't show the previous/next at all if we don't have > 1 items
		m_btnPrevious.ShowWindow(SW_HIDE);
		m_btnNext.ShowWindow(SW_HIDE);
	} else {
		if(m_aryEMNs.GetSize() == m_nCurEMNIndex + 1) {
			m_btnPrevious.EnableWindow(TRUE);
			m_btnNext.EnableWindow(FALSE);
		}
		else if(m_nCurEMNIndex <= 0) {
			m_btnPrevious.EnableWindow(FALSE);
			m_btnNext.EnableWindow(TRUE);
		}
		else {
			m_btnPrevious.EnableWindow(TRUE);
			m_btnNext.EnableWindow(TRUE);
		}

		// (a.walling 2010-01-11 15:45) - PLID 36837 - Ensure the next/previous buttons are visible
		if (!m_btnPrevious.IsWindowVisible()) {
			m_btnPrevious.ShowWindow(SW_SHOWNA);
		}
		if (!m_btnNext.IsWindowVisible()) {
			m_btnNext.ShowWindow(SW_SHOWNA);
		}
	}
}

// (c.haag 2013-03-07) - PLID 55365 - Repurposed for printing one EMN with multiple layouts, or multiple EMN's
LRESULT CEMRPreviewPopupDlg::OnPrintMultiple(WPARAM wParam, LPARAM lParam)
{
	try {
		LPUNKNOWN lpunkEMRCustomPreviewLayouts = (LPUNKNOWN)wParam;
		long nSingleEmnID = (long)lParam;

		if (m_nPatientID == -1) {
			ThrowNxException("Could not print; no patient found.");
		}

		// (z.manning 2011-09-22 11:50) - PLID 45624 - Filter out deleted EMNs
		// (z.manning 2012-09-11 17:51) - PLID 52543 - Added modified date
		// (c.haag 2013-02-28) - PLID 55368 - Added template ID
		// (c.haag 2013-03-07) - PLID 55365 - Supported filtering on an EMN
		ADODB::_RecordsetPtr prsAllEMNs;
		if (nSingleEmnID > 0)
		{
			prsAllEMNs = CreateParamRecordset(
				"SELECT EMRGroupsT.Description AS EMRDescription, EMRMasterT.PatientID, EMRMasterT.ID, EMRMasterT.Description, "
				"	EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate, EMRMasterT.TemplateID "
				"FROM EMRGroupsT "
				"LEFT JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
				"WHERE EMRMasterT.ID = {INT} AND EmrMasterT.Deleted = 0 "
				, nSingleEmnID);
			if (prsAllEMNs->eof) {
				MessageBox("The EMN was not found for this patient!");
				return 0;
			}
		} else {
			prsAllEMNs = CreateParamRecordset(
				"SELECT EMRGroupsT.Description AS EMRDescription, EMRMasterT.PatientID, EMRMasterT.ID, EMRMasterT.Description, "
				"	EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate, EMRMasterT.TemplateID "
				"FROM EMRGroupsT "
				"LEFT JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
				"LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID "
				"WHERE EMRMasterT.ID IS NOT NULL AND EmrMasterT.Deleted = 0 "
				"	AND EMRMasterT.PatientID = {INT} "
				, m_nPatientID);
			if (prsAllEMNs->eof) {
				MessageBox("No EMNs found for this patient!");
				return 0;
			}
		}

		// (b.savon 2011-11-22 11:54) - PLID 25782 - Added PatientID
		CEMRPreviewMultiPopupDlg dlg(m_nPatientID, this);
		// (c.haag 2013-02-28) - PLID 55368 - Assign the custom preview layouts
		dlg.SetCustomPreviewLayoutList(lpunkEMRCustomPreviewLayouts);
		// (c.haag 2013-02-28) - PLID 55368 - Release the custom preview layouts if we assigned them to the dialog
		if (NULL != lpunkEMRCustomPreviewLayouts) {
			lpunkEMRCustomPreviewLayouts->Release();
		}

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::null);
		while (!prsAllEMNs->eof) {
			CString strEMRDescription = AdoFldString(prsAllEMNs, "EMRDescription", "");
			CString strEMNDescription = AdoFldString(prsAllEMNs, "Description", "");
			COleDateTime dtEMNDate = AdoFldDateTime(prsAllEMNs, "Date", dtInvalid);
			COleDateTime dtEMNInputDate = AdoFldDateTime(prsAllEMNs, "InputDate", dtInvalid);
			COleDateTime dtEMNModifiedDate = AdoFldDateTime(prsAllEMNs, "ModifiedDate", dtInvalid);
			long nEMNID = AdoFldLong(prsAllEMNs, "ID", -1);
			// (c.haag 2013-02-28) - PLID 55368 - We also need the EMN template ID
			long nTemplateID = AdoFldLong(prsAllEMNs, "TemplateID", -1);

			dlg.AddAvailableEMN(NULL, nEMNID, nTemplateID, strEMRDescription, strEMNDescription, dtEMNDate, dtEMNInputDate, dtEMNModifiedDate);

			prsAllEMNs->MoveNext();
		}

		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);

	return 0;
}


// (a.walling 2010-01-11 12:36) - PLID 36837 - Support different 'subsections' for positioning preferences
void CEMRPreviewPopupDlg::RestoreSize(const CString& strSubsection)
{
	try {
		m_strSubsection = strSubsection;

		CRect rRect, rNewRect;

		GetWindowRect(rRect);
		rNewRect = rRect;

		// (a.walling 2010-01-11 15:16) - PLID 36837 - Get the suffix, or none if the default (from the patient NexEMR dialog)
		CString strSubsectionSuffix;
		if (!m_strSubsection.IsEmpty()) {
			strSubsectionSuffix = CString("_") + m_strSubsection;
			// bulk cache
			g_propManager.CachePropertiesInBulk(
				FormatString("EMRPreviewPopup%s", strSubsectionSuffix), 
				propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'EMRPreviewPopupWidth%s' OR "
				"Name = 'EMRPreviewPopupHeight%s' OR "
				"Name = 'EMRPreviewPopupTop%s' OR "
				"Name = 'EMRPreviewPopupLeft%s' "
				")", 
				_Q(GetCurrentUserName()),
				strSubsectionSuffix, strSubsectionSuffix, strSubsectionSuffix, strSubsectionSuffix
			);
		}

		long nWidth = GetRemotePropertyInt(FormatString("EMRPreviewPopupWidth%s", strSubsectionSuffix), rRect.Width(), 0, GetCurrentUserName(), true);
		long nHeight = GetRemotePropertyInt(FormatString("EMRPreviewPopupHeight%s", strSubsectionSuffix), rRect.Height(), 0, GetCurrentUserName(), true);
		long nTop = GetRemotePropertyInt(FormatString("EMRPreviewPopupTop%s", strSubsectionSuffix), rRect.top, 0, GetCurrentUserName(), true);
		long nLeft = GetRemotePropertyInt(FormatString("EMRPreviewPopupLeft%s", strSubsectionSuffix), rRect.left, 0, GetCurrentUserName(), true);
		
		if ( (nWidth > 50) && (nHeight > 50) ) {
			// ensure a reasonable rect
			rNewRect.left = nLeft;
			rNewRect.top = nTop;
			rNewRect.right = rNewRect.left + nWidth;
			rNewRect.bottom = rNewRect.top + nHeight;

			CRect rDesktopRect;
			GetDesktopWindow()->GetWindowRect(rDesktopRect);
			rDesktopRect.DeflateRect(0, 0, 30, 50); // deflate the rect's bottom right corner to ensure the top left
				// corner of the new window rect is visible and able to move

			// either the top left or top right corner should be in our desktop rect
			CPoint ptTopLeft = rNewRect.TopLeft();
			CPoint ptTopRight = ptTopLeft;
			ptTopRight.x += rNewRect.Width();

			if (! (rDesktopRect.PtInRect(ptTopLeft) || rDesktopRect.PtInRect(ptTopRight) )) {
				rNewRect.CopyRect(rRect); // get the initial rect
				// now set the width and height
				rNewRect.right = rNewRect.left + nWidth;
				rNewRect.bottom = rNewRect.top + nHeight;
			}

			MoveWindow(rNewRect);
		} else {
			ASSERT(FALSE);
		}
	} NxCatchAll("Error restoring EMR Preview popup size");
}

void CEMRPreviewPopupDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try {
		// (a.walling 2010-01-11 16:13) - PLID 27733 - If we are being 'shown', then make sure we are restored
		if (bShow && IsIconic()) {
			// (a.walling 2011-06-17 15:51) - PLID 42367 - Refresh if the source has changed
			RefreshPreviewIfModified();
			ShowWindow(SW_RESTORE);
		}
	} NxCatchAllIgnore();
}

// (a.walling 2010-01-11 18:02) - PLID 36840 - Handle a custom preview command
LRESULT CEMRPreviewPopupDlg::OnCustomEmrPreviewCommand(WPARAM wParam, LPARAM lParam)
{
	try {
		if (lParam == 66) {
			// (a.walling 2010-01-11 18:02) - PLID 36840 - This is our special identifier to create a preview if one did not exist

			if (m_nCurEMNIndex >= 0 && m_nCurEMNIndex < m_aryEMNs.GetSize()) {
				long nEmnID = m_aryEMNs.GetAt(m_nCurEMNIndex).nID;

				if (nEmnID != -1) {
					// (a.walling 2010-01-13 13:21) - PLID 36840 - Ensure this does not overwrite anything
					// (z.manning 2012-09-11 12:59) - PLID 52543 - Moved this code to its own function
					RegenerateEmnPreviewFromData(nEmnID, TRUE);

					DisplayCurrentEMNPreview();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (a.walling 2010-03-24 13:02) - PLID 37677 - Set the appropriate cursor
BOOL CEMRPreviewPopupDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try{
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if(m_nxlOpenThisEMN.IsWindowVisible() && m_nxlOpenThisEMN.IsWindowEnabled() && m_nxlOpenThisEMN.GetType() == dtsHyperlink) {
			m_nxlOpenThisEMN.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		static bool bNotifiedOnce = false;
		if(!bNotifiedOnce){ 
			bNotifiedOnce = true;
			try { throw; } NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2010-03-24 12:59) - PLID 37677 - Handle the label click
LRESULT CEMRPreviewPopupDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_LABEL_OPEN_THIS_EMN:
			{
				// (a.walling 2010-03-24 13:07) - PLID 37677 - Open up the current EMN!
				OpenCurrentEMN();
			}
			break;		
		default:
			//Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	
	return 0;
}

// (a.walling 2010-03-24 13:07) - PLID 37677 - Open up the current EMN!
void CEMRPreviewPopupDlg::OpenCurrentEMN()
{
	try {		
		long nID = -1;
		if(m_nCurEMNIndex != -1 && m_aryEMNs.GetSize() > m_nCurEMNIndex) {
			nID = m_aryEMNs.GetAt(m_nCurEMNIndex).nID;
		}
		
		if (nID == -1) {
			ThrowNxException("Could not determine current EMN");
		}

		// need to get the PICID!
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT PicT.ID "
			"FROM PicT "
			"INNER JOIN EMRGroupsT ON PicT.EmrGroupID = EMRGroupsT.ID "
			"INNER JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
			"WHERE EMRMasterT.ID = {INT}", nID);

		
		long nPicID = -1;

		if (!prs->eof) {
			nPicID = AdoFldLong(prs, "ID", -1);
		}

		// (c.haag 2010-08-04 12:20) - PLID 39980 - This is acceptable now, albeit a very rare occurrence
		//if (nPicID == -1) {
		//	ThrowNxException("Could not find PIC record for the current EMN");
		//}

		// minimize ourselves?
		/*
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp)) {
			//Check if we are not minimized
			if (!IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				SetWindowPlacement(&wp);
			}
		}
		*/

		// (a.walling 2010-03-24 13:16) - PLID 37677 - Finally open up the EMN
		GetMainFrame()->EditEmrRecord(nPicID, nID);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-06-17 15:49) - PLID 42367 - Activate handler
void CEMRPreviewPopupDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{	
	try {
		if (nState == WA_INACTIVE) return;
		
		RefreshPreviewIfModified();
	} NxCatchAllIgnore()
}

// (a.walling 2011-06-17 15:43) - PLID 42367 - Will refresh the preview if the source file has been modified
void CEMRPreviewPopupDlg::RefreshPreviewIfModified()
{
	if (m_strCurEMNFileName.IsEmpty()) return;
	if (m_ftCurEMNLastWriteTime.dwHighDateTime == 0 && m_ftCurEMNLastWriteTime.dwLowDateTime == 0) return;

	{
		CFile f;
		if (!f.Open(m_strCurEMNFileName, CFile::modeRead | CFile::shareDenyNone)) {
			return;
		}

		FILETIME ftLastWriteTime;

		if (::GetFileTime(f.m_hFile, NULL, NULL, &ftLastWriteTime)) {
			if (0 == memcmp(&m_ftCurEMNLastWriteTime, &ftLastWriteTime, sizeof(FILETIME))) {
				return;
			}
		}

		f.Close();
	}

	RefreshPreview();
}