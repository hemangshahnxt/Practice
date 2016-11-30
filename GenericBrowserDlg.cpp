// GenericBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "GenericBrowserDlg.h"
#include "HistoryUtils.h"
#include <mshtmcid.h>
#include <NxDataUtilitiesLib/NxStringStream.h>

// (j.gruber 2012-03-07 16:38) - PLID 48701
#pragma region BrowserEnabledDlg


IMPLEMENT_DYNAMIC(CBrowserEnabledDlg, CNxDialog)

CBrowserEnabledDlg::CBrowserEnabledDlg(int IDD, CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
}

// (a.walling 2015-06-16 11:27) - PLID 66395 - Support remembering size
CBrowserEnabledDlg::CBrowserEnabledDlg(int IDD, CWnd* pParent, const CString& strSizeAndPositionConfigRT)
	: CNxDialog(IDD, pParent, strSizeAndPositionConfigRT)
{
}

// (j.gruber 2012-05-08 09:36) - PLID 50223 - this generates all the current HTML, stolen from preview pane
CString CBrowserEnabledDlg::GetCurrentHTML(IHTMLElementPtr pElement /* = NULL*/)
{
	if (pElement == NULL) {
		if (m_pBrowser == NULL)
			return "";

		// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
		// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
		// underlying COM object was being leaked.
		IDispatchPtr p;
		HRESULT hr = m_pBrowser->get_Document(&p);

		if (SUCCEEDED(hr)) {
			IHTMLDocument2Ptr pDoc;
			
			hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

			if (SUCCEEDED(hr)) {
				hr = pDoc->get_body(&pElement);

				if (!SUCCEEDED(hr) || (pElement == NULL)) {
					return "";
				}
			}
		}
	}

	if (pElement == NULL)
		return "";

	// if we are here, the pElement is valid
	BSTR bstr;
	HRESULT hr = pElement->get_innerHTML(&bstr);

	if (SUCCEEDED(hr)) {
		CString str;
		// (a.walling 2007-07-09 09:28) - Ensure the BSTR is freed by attaching it to the _bstr_t object
		_bstr_t bstrWrapper(bstr, false);
		str = (LPCTSTR)bstrWrapper;

		return str;
	} else {
		return "";
	}
}

// (j.gruber 2012-05-15 12:23) - PLID 50397 - ability to call javascript inside the page
bool CBrowserEnabledDlg::ExecScript(const CString& strScript)
{
	_variant_t varRet;
	return ExecScript((LPCTSTR)strScript, L"JavaScript", &varRet);
}

// (j.gruber 2012-05-15 12:23) - PLID 50397 - ability to call javascript inside the page
bool CBrowserEnabledDlg::ExecScript(_bstr_t code, _bstr_t language, VARIANT* pvarRet)
{	

	HRESULT hr;			
	IDispatchPtr p;

	hr = m_pBrowser->get_Document(&p);
	if (SUCCEEDED(hr)) {		
		IHTMLDocument2Ptr pDoc;
		hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

		if (SUCCEEDED(hr)) {
			IHTMLWindow2Ptr pWindow;
			hr = pDoc->get_parentWindow(&pWindow);
	
			if (SUCCEEDED(hr) && pWindow != NULL) {		
				HRESULT hr = pWindow->execScript(code, language, pvarRet);

				return SUCCEEDED(hr);
			}
		}
	}
	return false;
}

IHTMLElementPtr CBrowserEnabledDlg::GetElementByID(const CString &strID)
{
	COleVariant varDivID(strID, VT_BSTR);
	COleVariant varIndex((long)0, VT_I4);

	HRESULT hr;

	// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
	// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
	// underlying COM object was being leaked.
	IDispatchPtr p;

	if (m_pBrowser == NULL)
		return NULL;

	hr = m_pBrowser->get_Document(&p);
	if (SUCCEEDED(hr)) {
		IHTMLDocument2Ptr pDoc;
		hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

		if (SUCCEEDED(hr)) {
			IHTMLElementCollectionPtr pElemColl = NULL;
			hr = pDoc->get_all(&pElemColl);

			if (SUCCEEDED(hr)) {
				// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
				// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
				// underlying COM object was being leaked.
				IDispatchPtr pDispItem = NULL;
				IHTMLElementPtr pElement = NULL;

				hr = pElemColl->item(varDivID, varIndex, &pDispItem);

				if (SUCCEEDED(hr)) {
					if (pDispItem) {
						hr = pDispItem->QueryInterface(IID_IHTMLElement, (void**)&pElement);

						if (SUCCEEDED(hr)) {
							if (pElement) {
								return pElement; // caller's smart pointer should add ref
							}
						} else {
							// it is possible that several items meet this criteria... rather than fail,
							// try to get the IHTMLElementCollection and return the first item (asserting too).
							IHTMLElementCollectionPtr pElementCollection;

							hr = pDispItem->QueryInterface(IID_IHTMLElementCollection, (void**)&pElementCollection);

							if (SUCCEEDED(hr) && (pElementCollection != NULL)) {
								// yep, returned multiple items, so let's get the first one.
								long nLength;

								hr = pElementCollection->get_length(&nLength);

								if (SUCCEEDED(hr)) {
									// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
									// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
									// underlying COM object was being leaked.
									IDispatchPtr pDispSingleItem;

									_variant_t varIndex = (long)0;

									hr = pElementCollection->item(varIndex, varIndex, &pDispSingleItem);

									if (SUCCEEDED(hr) && (pDispSingleItem != NULL)) {
										// alright, try to get the IHTMLElement interface again
										hr = pDispSingleItem->QueryInterface(IID_IHTMLElement, (void**)&pElement);

										if (SUCCEEDED(hr) && (pElement != NULL)) {
#ifdef _DEBUG
											LogDetail("WARNING: GetElementByID matched %li items, only first item returned.", nLength);
#endif										
											ASSERT(FALSE);
											return pElement;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL; // element was not found, or some other error.
}

// (a.walling 2010-02-25 16:35) - PLID 37547 - Get the IOleCommandTarget
IOleCommandTargetPtr CBrowserEnabledDlg::GetOleCommandTarget()
{
	IDispatchPtr pDisp = NULL;
	m_pBrowser->get_Document(&pDisp);
	if(pDisp) {
		IOleCommandTargetPtr pCmdTarg(pDisp);
		if(pCmdTarg) {
			return pCmdTarg;
		}
	}

	return NULL;
}
#pragma endregion



// (a.walling 2009-05-05 18:09) - PLID 34179 - Generic browser dialog

#pragma region BrowserInterface
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IGenericBrowserInterface::IGenericBrowserInterface()
{
	m_defaultClientSite			= NULL;
	m_defaultDocHostUIHandler	= NULL;
	m_cRef						= 0;
	m_pBrowser					= NULL;
	m_pBrowserDlg				= NULL;
}

IGenericBrowserInterface::~IGenericBrowserInterface()
{
	//We release our default interfaces
	this->SetDefaultClientSite(NULL);
}

VOID IGenericBrowserInterface::SetDefaultClientSite(IOleClientSite *pClientSite)
{
	if (pClientSite != NULL)
	{
		pClientSite->AddRef();

		m_defaultClientSite = pClientSite;
		m_defaultClientSite->QueryInterface(IID_IDocHostUIHandler, (VOID **)&m_defaultDocHostUIHandler);
	}
	else
	{
		if (m_defaultClientSite != NULL)
		{
			m_defaultClientSite->Release();
			m_defaultClientSite = NULL;
		}

		if (m_defaultDocHostUIHandler != NULL)
		{
			m_defaultDocHostUIHandler->Release();
			m_defaultDocHostUIHandler = NULL;
		}
	}
}
// *** IUnknown ***

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::QueryInterface(REFIID riid, LPVOID *ppv) 
{
	HRESULT result = S_OK;

	// Always set out parameter to NULL, validating it first 
	if (IsBadWritePtr(ppv, sizeof(LPVOID))) 
		result = E_INVALIDARG;  

	if (result == S_OK)
	{
		*ppv = NULL; 
 
		if ( IsEqualIID( riid, IID_IUnknown ) )
			*ppv = this;
		else if ( IsEqualIID( riid, IID_IOleClientSite ) )
			*ppv = (IOleClientSite *) this;
		else if ( IsEqualIID( riid, IID_IDocHostUIHandler ) )
			*ppv = (IDocHostUIHandler *) this;
		else
			result = E_NOINTERFACE;
	}

    if (result == S_OK)
        this->AddRef(); 
 
    return result; 
}

ULONG STDMETHODCALLTYPE IGenericBrowserInterface::AddRef() 
{    
    InterlockedIncrement(&m_cRef); 
    return m_cRef; 
} 
 
ULONG STDMETHODCALLTYPE IGenericBrowserInterface::Release() 
{ 
    // Decrement the object's internal counter 
    ULONG ulRefCount = InterlockedDecrement(&m_cRef); 
 
    if (0 == m_cRef) 
    {
        delete this; 
    }
 
    return ulRefCount; 
} 

// *** IOleClientSite ***

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::SaveObject()
{
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->SaveObject();
	else
		return E_FAIL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetMoniker(DWORD dwAssign, 
										DWORD dwWhichMoniker, LPMONIKER *ppmk)
{
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->GetMoniker(dwAssign, dwWhichMoniker, ppmk);
	else
	{
		if (! IsBadWritePtr(ppmk, sizeof(*ppmk)))
			*ppmk = NULL;

		return E_NOTIMPL;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetContainer(LPOLECONTAINER *ppContainer)
{	
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->GetContainer(ppContainer);
	else
	{
		if (! IsBadWritePtr(ppContainer, sizeof(*ppContainer)))
			*ppContainer = NULL;

		return E_NOINTERFACE;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::ShowObject()
{
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->ShowObject();
	else
		return S_OK;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::OnShowWindow(BOOL fShow)
{
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->OnShowWindow(fShow);
	else
		return S_OK;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::RequestNewObjectLayout()
{
	if (m_defaultClientSite != NULL)
		return m_defaultClientSite->RequestNewObjectLayout();
	else
		return E_NOTIMPL;
}

// *** IDocHostUIHandler ***

// (a.walling 2007-04-10 16:41) - PLID 25548 - Called when context menu about to be shown
HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::ShowContextMenu(DWORD dwID, POINT *ppt, 
					IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	HRESULT result	= S_FALSE;

	try {
		result = CustomContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
		// if returned S_FALSE, do the web browser's default
		if (result == S_FALSE) {
			if (m_defaultDocHostUIHandler != NULL)
				result = m_defaultDocHostUIHandler->ShowContextMenu(dwID, ppt, 
						pcmdtReserved, pdispReserved);
			else
				result = S_FALSE;
		}

		return result;
	} NxCatchAll("Error in IGenericBrowserInterface::ShowContextMenu");

	return S_OK;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
	HRESULT hr = S_OK;

	if (m_defaultDocHostUIHandler != NULL) {
		hr = m_defaultDocHostUIHandler->GetHostInfo(pInfo);
	}

	// (a.walling 2012-04-12 17:45) - PLID 49679 - Must explicitly request themed controls with DOCHOSTUIFLAG_THEME
	pInfo->dwFlags |= DOCHOSTUIFLAG_THEME;

	return hr;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::ShowUI(DWORD dwID, 
			IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
				IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->ShowUI(dwID, pActiveObject, 
						pCommandTarget, pFrame, pDoc);
	else
		return S_FALSE;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::HideUI()
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->HideUI();
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::UpdateUI()
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->UpdateUI();
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::EnableModeless(BOOL fEnable)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->EnableModeless(fEnable);
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::OnDocWindowActivate(BOOL fActivate)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->OnDocWindowActivate(fActivate);
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::OnFrameWindowActivate(BOOL fActivate)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->OnFrameWindowActivate(fActivate);
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::ResizeBorder(LPCRECT prcBorder,
			IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->ResizeBorder(prcBorder, pUIWindow, fRameWindow);
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::TranslateAccelerator(LPMSG lpMsg,
			const GUID *pguidCmdGroup, DWORD nCmdID)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
	else
		return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetOptionKeyPath( 
						LPOLESTR *pchKey, DWORD dw)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->GetOptionKeyPath(pchKey, dw);
	else
	{
		if (! IsBadWritePtr(pchKey, sizeof(*pchKey))) 
			*pchKey = NULL;

		return E_NOTIMPL;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetDropTarget( 
			IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
	if (m_defaultDocHostUIHandler != NULL)
	{
		HRESULT result = m_defaultDocHostUIHandler->GetDropTarget(pDropTarget, ppDropTarget);

		//Returning S_FALSE seems to disable DragNDrop, while DragNDrop is by default on.
		//Changing return code to E_FAIL seems to fix things. We probably want to disable
		//this anyway, though.
		if (result == S_FALSE)
			result = E_FAIL;

		return result;
	}
	else
	{
		if (! IsBadWritePtr(ppDropTarget, sizeof(*ppDropTarget))) 
			*ppDropTarget = NULL;

		return E_NOTIMPL;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::GetExternal(IDispatch **ppDispatch)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->GetExternal(ppDispatch);
	else
	{
		if (! IsBadWritePtr(ppDispatch, sizeof(*ppDispatch))) 
			*ppDispatch = NULL;

		return E_NOTIMPL;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::TranslateUrl( 
			DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
	else
	{
		if (! IsBadWritePtr(ppchURLOut, sizeof(*ppchURLOut))) 
			*ppchURLOut = NULL;

		return S_FALSE;
	}
}

HRESULT STDMETHODCALLTYPE IGenericBrowserInterface::FilterDataObject( 
			IDataObject *pDO, IDataObject  **ppDORet)
{
	if (m_defaultDocHostUIHandler != NULL)
		return m_defaultDocHostUIHandler->FilterDataObject(pDO, ppDORet);
	else
	{
		if (! IsBadWritePtr(ppDORet, sizeof(*ppDORet))) 
			*ppDORet = NULL;

		return S_FALSE;
	}
}

// return S_FALSE to perform web browser's default behavior
HRESULT IGenericBrowserInterface::CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	// (a.walling 2010-02-25 16:37) - PLID 37547 - Default now will provide limited options
	//return S_OK; // we will prevent the context menu by default

	IOleWindow	*oleWnd			= NULL;
    HWND		hwnd			= NULL;
    HMENU		hMainMenu		= NULL;
	HMENU		hPopupMenu		= NULL;
	HRESULT		hr				= 0;

	if ((ppt == NULL) || (pcmdtReserved == NULL))
		return S_OK;

    hr = pcmdtReserved->QueryInterface(IID_IOleWindow, (void**)&oleWnd);
	if ( (hr != S_OK) || (oleWnd == NULL))
		return S_OK;

	hr = oleWnd->GetWindow(&hwnd);
	if ( (hr != S_OK) || (hwnd == NULL))
		return S_OK;
	
	CMenu mnu;
	long n = 0;
	mnu.CreatePopupMenu();

	enum EGenericContextMenuItems {
		miPrint = 1638, // just some random number
		miPrintPreview,
		miSaveAs,
		miFind,
	};

	mnu.InsertMenu(n++, MF_BYPOSITION, miPrint, "&Print...");
	mnu.InsertMenu(n++, MF_BYPOSITION, miPrintPreview, "Print Pre&view...");
	mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");	
	mnu.InsertMenu(n++, MF_BYPOSITION, miSaveAs, "&Save As...");
	mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");	
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


#pragma region BrowserImplementation
// CGenericBrowserDlg dialog

// (j.gruber 2012-03-07 16:38) - PLID 48701
IMPLEMENT_DYNAMIC(CGenericBrowserDlg, CBrowserEnabledDlg)

CGenericBrowserDlg::CGenericBrowserDlg(CWnd* pParent /*=NULL*/)
	: CBrowserEnabledDlg(CGenericBrowserDlg::IDD, pParent)
{
}

// (a.walling 2015-06-16 11:27) - PLID 66395 - Support remembering size
CGenericBrowserDlg::CGenericBrowserDlg(CWnd* pParent /*=NULL*/, const CString& strSizeAndPositionConfigRT) 
	: CBrowserEnabledDlg(CGenericBrowserDlg::IDD, pParent, strSizeAndPositionConfigRT)
{
}

CGenericBrowserDlg::~CGenericBrowserDlg()
{
}

void CGenericBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_STATIC_CAPTION, m_nxlabelCaption);
}

void CGenericBrowserDlg::NavigateToStream(IStreamPtr pStream)
{
	if (pStream == NULL) {		
		_com_issue_error(DISP_E_BADVARTYPE);
		return;
	}

	m_pPendingStream = pStream;

	LoadPendingStream();
}

// (a.walling 2015-06-16 11:27) - PLID 66395
void CGenericBrowserDlg::NavigateToHtml(CString strHtml)
{
	IStreamPtr pStream(new NxStringStream(strHtml), false);
	NavigateToStream(pStream);
}

void CGenericBrowserDlg::LoadPendingStream()
{	
	if (m_pPendingStream && m_pBrowser) { // (a.walling 2015-06-16 11:27) - PLID 66395
		// load the stream into the browser
		IDispatchPtr pDoc;
		HR(m_pBrowser->get_Document(&pDoc));
		if (pDoc) {
			IPersistStreamInitPtr pPersistStreamPtr;

			HR(pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&pPersistStreamPtr));
			if (pPersistStreamPtr) {
				HR(pPersistStreamPtr->InitNew());
				HR(pPersistStreamPtr->Load(m_pPendingStream));

				m_pPendingStream = NULL;
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CGenericBrowserDlg, CNxDialog)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClicked)
END_MESSAGE_MAP()


INT_PTR CGenericBrowserDlg::DoModal()
{
	m_bIsModal = TRUE;
	m_bAutoDelete = FALSE;
	return CNxDialog::DoModal();
}

// CGenericBrowserDlg message handlers
BOOL CGenericBrowserDlg::OnInitDialog()
{
	try {
		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the dialog in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (!m_bIsModal && GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		m_pBrowser = GetDlgItem(IDC_GENERIC_BROWSER)->GetControlUnknown();
		
		m_piClientSite = new IGenericBrowserInterface;

		m_piClientSite->SetBrowser(m_pBrowser);
		m_piClientSite->SetBrowserDlg(this);

		IUnknown* pUnkBrowser = GetDlgItem(IDC_GENERIC_BROWSER)->GetControlUnknown(); // not reference counted, do not release

		if (pUnkBrowser) {
			IOleObjectPtr pBrowserOleObject = NULL;

			// retrieve our OleObject interface so we can set our custom client site
			pUnkBrowser->QueryInterface(IID_IOleObject, (void**)&pBrowserOleObject);

			if (pBrowserOleObject != NULL)
			{
				IOleClientSite *oldClientSite = NULL;

				if (pBrowserOleObject->GetClientSite(&oldClientSite) == S_OK)
				{
					m_piClientSite->SetDefaultClientSite(oldClientSite);
					oldClientSite->Release();
				}

				pBrowserOleObject->SetClientSite(m_piClientSite);
			}

			
			COleVariant varUrl("about:blank");

			if (m_pBrowser) {
				m_pBrowser->Navigate2(varUrl, NULL, NULL, NULL, NULL);
			}

			// smart pointer should take care of decrementing the pBrowserOleObject refcount.

			CNxDialog::OnInitDialog();
			InitializeControls();
		}		
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (a.walling 2009-05-13 10:52) - PLID 34243 - Prepare controls (virtual for derived classes)
void CGenericBrowserDlg::InitializeControls()
{
	m_nxibCancel.AutoSet(NXB_CLOSE);
	m_nxibCancel.SetFocus();

	UpdateControls();
}

void CGenericBrowserDlg::UpdateControls()
{
	// no OK button by default
	m_nxibOK.EnableWindow(FALSE);
	m_nxibOK.ShowWindow(SW_HIDE);

	m_nxlabelCaption.ShowWindow(SW_HIDE);
	m_nxlabelCaption.EnableWindow(FALSE);
	m_nxlabelCaption.SetType(dtsDisabledHyperlink);
}

// (a.walling 2009-05-13 10:57) - PLID 34243 - Label clicked (handled in derived classes)
LRESULT CGenericBrowserDlg::OnLabelClicked(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CGenericBrowserDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	OleInitialize(NULL); // does not throw exceptions

	
	// (a.walling 2009-06-12 13:31) - PLID 34176 - Register to the mainframe
	if (GetMainFrame()) {
		GetMainFrame()->RegisterModelessWindow(GetSafeHwnd());
	}

	return 0;
}

void CGenericBrowserDlg::OnDestroy()
{
	// (a.walling 2009-06-12 13:31) - PLID 34176 - Unregister to the mainframe
	if (GetMainFrame()) {
		GetMainFrame()->UnregisterModelessWindow(GetSafeHwnd());
	}
	CNxDialog::OnDestroy();

	OleUninitialize(); // does not throw exceptions
}

void CGenericBrowserDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		// (a.walling 2015-06-16 11:27) - PLID 66395 - skip NxDialog sizing since we do it all ourselves
		CDialogEx::OnSize(nType, cx, cy);

		// (a.walling 2009-05-12 09:25) - PLID 34179 - Resize smarter
		CRect rcClient;
		GetClientRect(rcClient);
		
		// (j.armen 2014-08-06 10:06) - PLID 63161 - Now using a vector
		for (auto it = m_aryControls.begin(); it != m_aryControls.end(); it++)
		{
			const CControlInfo& c = *it;
			CWnd* pWnd = CWnd::FromHandle(c.hwnd);

			if (pWnd) {
				CRect rc;
				if ( (c.hwnd == m_nxibOK.GetSafeHwnd()) || (c.hwnd == m_nxibCancel.GetSafeHwnd()) ) {
					rc.bottom = rcClient.bottom - (originalHeight - c.nBottom);
					rc.top = rcClient.bottom - (originalHeight - c.nTop);
					rc.left = rcClient.right - (originalWidth - c.nLeft);
					rc.right = rcClient.right - (originalWidth - c.nRight);

					pWnd->MoveWindow(rc, FALSE);
				} else if (c.hwnd == m_nxlabelCaption.GetSafeHwnd()) {
					rc.bottom = rcClient.bottom - (originalHeight - c.nBottom);
					rc.top = rcClient.bottom - (originalHeight - c.nTop);
					rc.left = rcClient.left + (c.nLeft);
					rc.right = rcClient.right - (originalWidth - c.nRight);

					pWnd->MoveWindow(rc, FALSE);
				} else if (c.hwnd == GetDlgItem(IDC_GENERIC_BROWSER)->GetSafeHwnd()) {
					rc.top = 0;
					rc.left = 0;
					rc.right = rcClient.right;
					rc.bottom = rcClient.bottom - (originalHeight - c.nBottom);

					pWnd->MoveWindow(rc, FALSE);
				} else {
					// this will also move and invalidate the window
					SetSingleControlPos(CWnd::FromHandle(c.hwnd), NULL, c.nLeft, c.nTop, c.nRight - c.nLeft, c.nBottom - c.nTop, SWP_NOZORDER|SWP_NOREDRAW);
				}
			}
		}		

		// have to redraw everything unfortunately due to the gripper in the bottom right; normally CNexTechDialog::OnSize does this.
		CRect rc;
		GetClientRect(&rc);
		RedrawWindow(rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);
	} NxCatchAll("CGenericBrowserDlg::OnSize");
}

BOOL CGenericBrowserDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		if (m_nxlabelCaption.GetType() == dtsHyperlink) {
			CPoint pt;
			::GetMessagePos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			m_nxlabelCaption.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	} NxCatchAllIgnore()
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CGenericBrowserDlg::OnOK()
{
	CNxDialog::OnOK();
	
	if (m_bAutoDelete) {
		::DestroyWindow(GetSafeHwnd());
	}
}

void CGenericBrowserDlg::OnCancel()
{
	CNxDialog::OnCancel();

	if (m_bAutoDelete) {
		::DestroyWindow(GetSafeHwnd());
	}
}

void CGenericBrowserDlg::PostNcDestroy()
{
	CNxDialog::PostNcDestroy();

	if (m_bAutoDelete) {
		delete this;
	}
}

BEGIN_EVENTSINK_MAP(CGenericBrowserDlg, CNxDialog)
	ON_EVENT(CGenericBrowserDlg, IDC_GENERIC_BROWSER, 259, CGenericBrowserDlg::DocumentCompleteGenericBrowser, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CGenericBrowserDlg, IDC_GENERIC_BROWSER, 251, CGenericBrowserDlg::NewWindow2GenericBrowser, VTS_PDISPATCH VTS_PBOOL)
	ON_EVENT(CGenericBrowserDlg, IDC_GENERIC_BROWSER, 273, CGenericBrowserDlg::NewWindow3GenericBrowser, VTS_PDISPATCH VTS_PBOOL VTS_UI4 VTS_BSTR VTS_BSTR)
	ON_EVENT(CGenericBrowserDlg, IDC_GENERIC_BROWSER, 113, CGenericBrowserDlg::TitleChangeGenericBrowser, VTS_BSTR)
END_EVENTSINK_MAP()

void CGenericBrowserDlg::DocumentCompleteGenericBrowser(LPDISPATCH pDisp, VARIANT* URL)
{
	LoadPendingStream();
}

void CGenericBrowserDlg::NewWindow2GenericBrowser(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	*Cancel = TRUE;
}

void CGenericBrowserDlg::NewWindow3GenericBrowser(LPDISPATCH* ppDisp, BOOL* Cancel, unsigned long dwFlags, LPCTSTR bstrUrlContext, LPCTSTR bstrUrl)
{
	*Cancel = TRUE;
}

void CGenericBrowserDlg::TitleChangeGenericBrowser(LPCTSTR Text)
{
	SetWindowText(Text);
}

#pragma endregion