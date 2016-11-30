#pragma once

#include <mshtml.h>
#include <mshtmhst.h>

// (a.walling 2009-05-05 18:09) - PLID 34179 - Generic browser dialog

// CGenericBrowserDlg dialog


// (j.gruber 2012-03-07 16:38) - PLID 48701
#pragma region BrowserEnabledDlg
class CBrowserEnabledDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBrowserEnabledDlg)

public:
	//CBrowserEnabledDlg(CWnd* pParent);
	CBrowserEnabledDlg(int IDD, CWnd *pParent);
	CBrowserEnabledDlg(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT); // (a.walling 2015-06-16 11:27) - PLID 66395
	
	// (j.gruber 2012-05-08 09:36) - PLID 50223 - this generates all the current HTML, stolen from preview pane
	CString GetCurrentHTML(IHTMLElementPtr pElement = NULL);

	// (j.gruber 2012-05-15 12:23) - PLID 50397 - ability to call javascript inside the page
	bool ExecScript(_bstr_t code, _bstr_t language, VARIANT* pvarRet);
	bool ExecScript(const CString& strScript);

	IHTMLElementPtr GetElementByID(const CString &strID);

	virtual IOleCommandTargetPtr GetOleCommandTarget();
	class IGenericBrowserInterface* m_piClientSite = nullptr;

protected:
	IWebBrowser2Ptr m_pBrowser;
};
#pragma endregion

// (j.gruber 2012-03-07 16:38) - PLID 48701
class CGenericBrowserDlg : public CBrowserEnabledDlg
{
	DECLARE_DYNAMIC(CGenericBrowserDlg)

public:
	CGenericBrowserDlg(CWnd* pParent);   // standard constructor
	CGenericBrowserDlg(CWnd* pParent, const CString& strSizeAndPositionConfigRT); // (a.walling 2015-06-16 11:27) - PLID 66395
	virtual ~CGenericBrowserDlg();

	virtual INT_PTR DoModal();
	void NavigateToStream(IStreamPtr pStream);
	void NavigateToHtml(CString strHtml); // (a.walling 2015-06-16 11:27) - PLID 66395

	IStreamPtr m_pPendingStream;

	BOOL m_bAutoDelete = TRUE;

// Dialog Data
	enum { IDD = IDD_GENERIC_BROWSER_DLG };

	CNxIconButton m_nxibOK;
	CNxIconButton m_nxibCancel;
	CNxLabel m_nxlabelCaption;

	// (a.walling 2010-02-25 16:35) - PLID 37547 - Get the IOleCommandTarget
	// (j.gruber 2012-03-07 16:38) - PLID 48701 - take out
	//virtual IOleCommandTargetPtr GetOleCommandTarget();

protected:	
	// (a.walling 2009-05-13 10:52) - PLID 34243 - Prepare controls (virtual for derived classes)
	virtual void InitializeControls();
	virtual void UpdateControls();

	// (j.gruber 2012-03-07 16:38) - PLID 48701 - take out
	//class IGenericBrowserInterface* m_piClientSite;
	//IWebBrowser2Ptr m_pBrowser;
	void LoadPendingStream();

	BOOL m_bIsModal = FALSE;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (a.walling 2009-05-13 10:57) - PLID 34243 - Label clicked (handled in derived classes)
	afx_msg virtual LRESULT OnLabelClicked(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	void DocumentCompleteGenericBrowser(LPDISPATCH pDisp, VARIANT* URL);
	void NewWindow2GenericBrowser(LPDISPATCH* ppDisp, BOOL* Cancel);
	void NewWindow3GenericBrowser(LPDISPATCH* ppDisp, BOOL* Cancel, unsigned long dwFlags, LPCTSTR bstrUrlContext, LPCTSTR bstrUrl);
	void TitleChangeGenericBrowser(LPCTSTR Text);
	afx_msg void OnBnClickedCancel();
};

class IGenericBrowserInterface : public IOleClientSite, public IDocHostUIHandler
{
public:
	IGenericBrowserInterface();
	virtual ~IGenericBrowserInterface();

	VOID SetDefaultClientSite(IOleClientSite *pClientSite);
	IOleClientSite *GetDefaultClientSite()
				{ return m_defaultClientSite; }
	IDocHostUIHandler *GetDefaultDocHostUIHandler()
				{ return m_defaultDocHostUIHandler; }

// *** IUnknown ***
	STDMETHOD(QueryInterface)(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ VOID **ppvObject);

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

// *** IOleClientSite ***        
	STDMETHOD(SaveObject)();
	STDMETHOD(GetMoniker)(
		/* [in] */ DWORD dwAssign,
		/* [in] */ DWORD dwWhichMoniker,
		/* [out] */ IMoniker **ppmk);

	STDMETHOD(GetContainer)(
		/* [out] */ IOleContainer **ppContainer);

	STDMETHOD(ShowObject)();
	STDMETHOD(OnShowWindow)(
		/* [in] */ BOOL fShow);

	STDMETHOD(RequestNewObjectLayout)();

	// *** IDocHostUIHandler ***

	STDMETHOD(ShowContextMenu)( 
		/* [in] */ DWORD dwID,
		/* [in] */ POINT *ppt,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved);
	STDMETHOD(GetHostInfo)( 
		/* [out][in] */ DOCHOSTUIINFO *pInfo);
	STDMETHOD(ShowUI)( 
		/* [in] */ DWORD dwID,
		/* [in] */ IOleInPlaceActiveObject *pActiveObject,
		/* [in] */ IOleCommandTarget *pCommandTarget,
		/* [in] */ IOleInPlaceFrame *pFrame,
		/* [in] */ IOleInPlaceUIWindow *pDoc);
	STDMETHOD(HideUI)();
	STDMETHOD(UpdateUI)();
	STDMETHOD(EnableModeless)( 
		/* [in] */ BOOL fEnable);
	STDMETHOD(OnDocWindowActivate)( 
		/* [in] */ BOOL fActivate);
	STDMETHOD(OnFrameWindowActivate)( 
		/* [in] */ BOOL fActivate);
	STDMETHOD(ResizeBorder)( 
		/* [in] */ LPCRECT prcBorder,
		/* [in] */ IOleInPlaceUIWindow *pUIWindow,
		/* [in] */ BOOL fRameWindow);
	STDMETHOD(TranslateAccelerator)( 
		/* [in] */ LPMSG lpMsg,
		/* [in] */ const GUID *pguidCmdGroup,
		/* [in] */ DWORD nCmdID);
	STDMETHOD(GetOptionKeyPath)( 
		/* [out] */ LPOLESTR *pchKey,
		/* [in] */ DWORD dw);
	STDMETHOD(GetDropTarget)( 
		/* [in] */ IDropTarget *pDropTarget,
		/* [out] */ IDropTarget **ppDropTarget);
	STDMETHOD(GetExternal)( 
		/* [out] */ IDispatch **ppDispatch);
	STDMETHOD(TranslateUrl)( 
		/* [in] */ DWORD dwTranslate,
		/* [in] */ OLECHAR *pchURLIn,
		/* [out] */ OLECHAR **ppchURLOut);
	STDMETHOD(FilterDataObject)( 
		/* [in] */ IDataObject *pDO,
		/* [out] */ IDataObject **ppDORet);

	inline void SetBrowser(IWebBrowser2Ptr pBrowser) {
		m_pBrowser = pBrowser;
	}

	// (j.gruber 2012-03-07 16:38) - PLID 48701 - changed to BrowserEnabledDlg
	void SetBrowserDlg(CBrowserEnabledDlg* pBrowserDlg) {
		m_pBrowserDlg = pBrowserDlg;
	}
	
	// (j.gruber 2012-03-07 16:38) - PLID 48701 - changed to BrowserEnabledDlg
	CBrowserEnabledDlg* m_pBrowserDlg;

private:
	// return S_FALSE to perform web browser's default behavior
	// (a.walling 2010-02-25 16:37) - PLID 37547 - Default now will provide limited options
	virtual HRESULT CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);

	LONG				m_cRef;
	IOleClientSite		*m_defaultClientSite;
	IDocHostUIHandler	*m_defaultDocHostUIHandler;
	IWebBrowser2Ptr		m_pBrowser;
	

	
};