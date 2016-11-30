#include "stdafx.h"
#include "NxHTTPRequestSink.h"
// (a.walling 2009-03-06 12:33) - PLID 33383 - Event sink to catch OnreadyStateChange events from async IXMLHTTPRequests
// Inspired by article from Sven Groot (http://www.ookii.org)

const UINT NXM_HTTPREQUEST_COMPLETE = ::RegisterWindowMessage("NXM_HTTPREQUEST_COMPLETE_{7FE221A0-389f-5ADE-9EEE-7329FBAC2D3F}");

STDMETHODIMP NxHTTPRequestSink::QueryInterface(const IID &riid, void **ppvObject)
{
    if( ppvObject == NULL )
        return E_INVALIDARG;

    *ppvObject = NULL;

    if( riid == IID_IUnknown )
        *ppvObject = static_cast<IUnknown*>(this);
    else if( riid == IID_IDispatch )
        *ppvObject = static_cast<IDispatch*>(this);

    if( *ppvObject == NULL )
        return E_NOINTERFACE;

    AddRef();

    return S_OK;
}

STDMETHODIMP_(ULONG) NxHTTPRequestSink::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}


STDMETHODIMP_(ULONG) NxHTTPRequestSink::Release()
{
    LONG refCount = InterlockedDecrement(&m_refCount);
    if( refCount == 0 )
    {
        delete this;
        return 0;
    }
    else
        return refCount;
}

STDMETHODIMP NxHTTPRequestSink::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP NxHTTPRequestSink::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP NxHTTPRequestSink::GetIDsOfNames(const IID &riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP NxHTTPRequestSink::Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	ASSERT(dispIdMember == 0); // Invoke should only be called for onreadystatechange

	try {
		/*
(0) (UNINITIALIZED)
The object has been created, but not initialized (the open method has not been called).
(1) LOADING
The object has been created, but the send method has not been called.
(2) LOADED
The send method has been called, but the status and headers are not yet available.
(3) INTERACTIVE
Some data has been received. Calling the responseBody and responseText properties at this state to obtain partial results will return an error, because status and response headers are not fully available.
(4) COMPLETED
All the data has been received, and the complete data is available in the responseBody and responseText properties.
		*/
		if(m_request != NULL && m_request->readyState == 4)
		{
			if (::IsWindow(m_hwndPostBack)) {
				::PostMessage(m_hwndPostBack, NXM_HTTPREQUEST_COMPLETE, (WPARAM)m_dwID, (LPARAM)0);
			}
		}
	} catch(...) {
		// error?!
	}
    return S_OK;
}
