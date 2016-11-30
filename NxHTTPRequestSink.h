#pragma once
#include "oaidl.h"

// (a.walling 2014-04-24 12:00) - VS2013 - MSXML Import
#include <NxDataUtilitiesLib/NxMsxml6Import.h>

// (a.walling 2009-03-06 12:33) - PLID 33383 - Event sink to catch OnreadyStateChange events from async IXMLHTTPRequests

// WPARAM - m_dwID (ID of this specific request)
// LPARAM - Reserved
extern const UINT NXM_HTTPREQUEST_COMPLETE;

class NxHTTPRequestSink : public IDispatch
{
public:
	NxHTTPRequestSink(MSXML2::IXMLHTTPRequest* request, HWND hwndPostBack, DWORD dwID)
		: m_refCount(1), m_request(request), m_hwndPostBack(hwndPostBack), m_dwID(dwID)
	{
	};
	virtual ~NxHTTPRequestSink() { };

    // throw in some IUnknown jibber-jabber
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // and some IDispatch jibber-jabber
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);        
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);    
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);

    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

private:
	DWORD m_dwID;
    LONG m_refCount;
	HWND m_hwndPostBack;
	MSXML2::IXMLHTTPRequest *m_request;
};