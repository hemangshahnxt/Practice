// CCDAViewerDlg.cpp : implementation file
//

// (d.singleton 2014-06-04 10:01) - PLID 61927 - created

#include "stdafx.h"
#include "Practice.h"
#include "CCDAViewerDlg.h"
#include "HistoryUtils.h"
#include <mshtmcid.h>
#include "NxAPI.h"
#include "NxAPIUtils.h"


// CCCDAViewerDlg

IMPLEMENT_DYNAMIC(CCCDAViewerDlg, CGenericXMLBrowserDlg)

CCCDAViewerDlg::CCCDAViewerDlg(CWnd* pParent, long nMailSentID) : CGenericXMLBrowserDlg(pParent)
{
	m_nMailSentID = nMailSentID;
}

// CCCDAViewerDlg message handlers
BOOL CCCDAViewerDlg::OnInitDialog()
{
	try {

		if (!m_bIsModal && GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		m_pBrowser = GetDlgItem(IDC_GENERIC_BROWSER)->GetControlUnknown();

		CNxDialog::OnInitDialog();
		InitializeControls();

		m_piCCDAClientSite = new ICCDAInterface(m_nMailSentID);

		m_piCCDAClientSite->SetBrowser(m_pBrowser);
		m_piCCDAClientSite->SetBrowserDlg(this);	

		IUnknown* pUnkBrowser = GetDlgItem(IDC_GENERIC_BROWSER)->GetControlUnknown();
		if (pUnkBrowser) {
			IOleObjectPtr pBrowserOleObject = NULL;

			pUnkBrowser->QueryInterface(IID_IOleObject, (void**)&pBrowserOleObject);

			if (pBrowserOleObject != NULL) {
				IOleClientSite* oldClientSite = NULL;

				if (pBrowserOleObject->GetClientSite(&oldClientSite) == S_OK) {
					m_piCCDAClientSite->SetDefaultClientSite(oldClientSite);
					oldClientSite->Release();
				}
				pBrowserOleObject->SetClientSite(m_piCCDAClientSite);
			}
		}

		COleVariant varUrl("about:blank");

		if (m_pBrowser) {
			m_pBrowser->Navigate2(varUrl, NULL, NULL, NULL, NULL);
		}		
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}


#pragma region BrowserInterface
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ICCDAInterface::ICCDAInterface(long nMailSentID)
{
	m_nMailSentID = nMailSentID;
}

HRESULT STDMETHODCALLTYPE ICCDAInterface::GetExternal(IDispatch **ppDispatch)
{
	try {
		*ppDispatch = m_pBrowserDlg->GetIDispatch(TRUE);
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// return S_FALSE to perform web browser's default behavior
HRESULT ICCDAInterface::CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	// (a.walling 2010-02-25 16:37) - PLID 37547 - Default now will provide limited options
	//return S_OK; // we will prevent the context menu by default

	IOleWindow	*oleWnd = NULL;
	HWND		hwnd = NULL;
	HMENU		hMainMenu = NULL;
	HMENU		hPopupMenu = NULL;
	HRESULT		hr = 0;

	if ((ppt == NULL) || (pcmdtReserved == NULL))
		return S_OK;

	hr = pcmdtReserved->QueryInterface(IID_IOleWindow, (void**)&oleWnd);
	if ((hr != S_OK) || (oleWnd == NULL))
		return S_OK;

	hr = oleWnd->GetWindow(&hwnd);
	if ((hr != S_OK) || (hwnd == NULL))
		return S_OK;

	CMenu mnu;
	long n = 0;
	mnu.CreatePopupMenu();

	enum EGenericContextMenuItems {
		miPrint = 1638, // just some random number
		miPrintPreview,
		miSaveAs,
		miSaveAsPDF,
		miFind,
	};

	mnu.InsertMenu(n++, MF_BYPOSITION, miPrint, "&Print...");
	mnu.InsertMenu(n++, MF_BYPOSITION, miPrintPreview, "Print Pre&view...");
	mnu.InsertMenu(n++, MF_BYPOSITION | MF_SEPARATOR, 0, "");
	mnu.InsertMenu(n++, MF_BYPOSITION, miSaveAs, "&Save As...");
	if (m_nMailSentID > -1) {
		mnu.InsertMenu(n++, MF_BYPOSITION, miSaveAsPDF, "Save As P&DF...");
	}	
	mnu.InsertMenu(n++, MF_BYPOSITION | MF_SEPARATOR, 0, "");
	mnu.InsertMenu(n++, MF_BYPOSITION, miFind, "&Find...");

	long nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY | TPM_VERPOSANIMATION,
		ppt->x, ppt->y, CWnd::FromHandle(hwnd), NULL);

	if (nSelection > 0) {
		// (a.walling 2010-02-25 16:52) - PLID 37547 - You would think that pcmdtReserved would able to be QIed to the IOleCommandTarget interface.
		// However, for some reason, that is not always the case, which is why we ended up using this in the EMRPreviewCtrl. So I'll do the same here,
		// although I think that other issue was limited to IE6.
		IOleCommandTargetPtr pCmdTarg = m_pBrowserDlg->GetOleCommandTarget();
		switch (nSelection) {
			// (a.walling 2010-02-25 17:24) - PLID 37547 - For now, this will use the default print template. Unfortunately with default settings, this
			// prints the URL at the bottom right. This will always be about:blank. 
		case miPrint:
		{
			pCmdTarg->Exec(&CGID_MSHTML,
				IDM_PRINT,
				OLECMDEXECOPT_PROMPTUSER,
				NULL, // use default print template
				NULL);
		} break;
		case miPrintPreview:
		{
			pCmdTarg->Exec(&CGID_MSHTML,
				IDM_PRINTPREVIEW,
				OLECMDEXECOPT_PROMPTUSER,
				NULL, // use default print template
				NULL);
		} break;
		case miSaveAs:
		{
			CString strWindowTitle;
			m_pBrowserDlg->GetWindowText(strWindowTitle);
			CString strPath = MakeValidFolderName(strWindowTitle);
			strPath += ".htm";
			_variant_t varPath = (LPCTSTR)strPath;
			pCmdTarg->Exec(&CGID_MSHTML,
				IDM_SAVEAS,
				OLECMDEXECOPT_PROMPTUSER,
				&varPath,
				NULL);
		} break;
		case miSaveAsPDF:
		{
			// (d.singleton 2014-05-29 15:10) - PLID 61927 generate pdf and allow user to save
			CWaitCursor cursor;
			//need to get the file name
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PathName FROM Mailsent WHERE MailID = {INT}", m_nMailSentID);
			CString strFileName = AdoFldString(prs, "PathName", "");
			strFileName = strFileName.Left(strFileName.ReverseFind('.'));
			//get our file data
			NexTech_Accessor::_HistoryEntryPDFResultPtr pResult = GetAPI()->GetHistoryEntryPDF(GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_nMailSentID));
			Nx::SafeArray<BYTE> fileBytes = pResult->PDFFile;
			// get the desired save path
			CString strFilePath;
			CFileDialog FileDlg(FALSE, "PDF", strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||");
			if (FileDlg.DoModal() == ID_CANCEL) {
				return S_OK;
			}
			strFilePath = FileDlg.GetPathName();
			// create the file
			CFile OutFile(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
			OutFile.Write(fileBytes.cbegin(), fileBytes.GetLength());
			OutFile.Close();
		} break;
		case miFind:
		{
			pCmdTarg->Exec(&CGID_MSHTML,
				IDM_FIND,
				OLECMDEXECOPT_DODEFAULT,
				NULL,
				NULL);
		} break;
		}
	}

	return S_OK;
}
#pragma endregion

