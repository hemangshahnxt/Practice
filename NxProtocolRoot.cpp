#include "StdAfx.h"
#include "NxProtocolRoot.h"
#include <NxSingleton.h>
#include "NxAllocators.h"
#include <map>

// (a.walling 2012-09-04 12:10) - PLID 52438 - nx:// protocol root

namespace Nx
{

Url::Url(LPCTSTR szUrl)
{
	BOOL success = FALSE;

	{
		CString::CStrBuf hostBuf(host, MAX_PATH);
		CString::CStrBuf pathBuf(path, MAX_PATH);
		CString::CStrBuf extraBuf(extra, MAX_PATH);

		URL_COMPONENTS components = {sizeof(URL_COMPONENTS)};
		components.dwHostNameLength = MAX_PATH;
		components.dwUrlPathLength = MAX_PATH;
		components.dwExtraInfoLength = MAX_PATH;

		components.lpszHostName = hostBuf;
		components.lpszUrlPath = pathBuf;
		components.lpszExtraInfo = extraBuf;

		success = ::InternetCrackUrl(szUrl, 0, 0 /*ICU_DECODE | ICU_ESCAPE*/, &components);
	}
	ASSERT(success);

	if (!success) {
		host = path = extra = "";
	}
}

namespace Protocol
{
namespace
{
	struct HostExtensions
	{
		typedef std::map
			<
				  CString
				, HostStreamHandlerFn
				, std::less<CString>
				, Nx::RawHeapAllocator
				<
					std::pair<CString, HostStreamHandlerFn>
				>
			> HandlerMap;

		bool Register(const CString& str, HostStreamHandlerFn fn)
		{
			ENSURE(!str.IsEmpty());

			CSingleLock lock(&m_cs, TRUE);
			if (m_handlers.count(str)) return false;
			m_handlers.insert(HandlerMap::value_type(str, fn));
			return true;
		}

		bool Unregister(const CString& str)
		{
			ENSURE(!str.IsEmpty());

			CSingleLock lock(&m_cs, TRUE);
			m_handlers.erase(str);
			return true;
		}

		HostStreamHandlerFn Lookup(const CString& str)
		{
			HostStreamHandlerFn fn;
			
			if (str.IsEmpty()) return fn;

			CSingleLock lock(&m_cs, TRUE);

			HandlerMap::iterator it = m_handlers.find(str);
			if (it != m_handlers.end()) {
				fn = it->second;
			}

			return fn;
		}

		HandlerMap m_handlers;
		CCriticalSection m_cs;
	};

	HostExtensions& GetHostExtensions()
	{	
		static HostExtensions* p = NULL;
		return InitSingleton(&p);
	}
}

bool RegisterHost(const CString& str, HostStreamHandlerFn fn)
{
	return GetHostExtensions().Register(str, fn);
}

bool UnregisterHost(const CString& str)
{
	return GetHostExtensions().Unregister(str);
}

///

Root::Root()
{}

Root::~Root()
{ FinalRelease(); }

void Root::FinalRelease()
{
	m_pStream = NULL;
	m_pSink = NULL;
}

/// IInternetProtocolRoot
STDMETHODIMP Root::Start(LPCWSTR wszUrl, IInternetProtocolSink *pOIProtSink,
	IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
{ 	
	if (!wszUrl || !pOIProtSink) {
		return E_POINTER;
	}

	m_pSink = pOIProtSink;
	m_strUrl = wszUrl;

	OpenStream();

	if (!m_pStream) {
		m_pSink = NULL;
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	}

	HRESULT hr = S_OK;
	if (m_pSink) {
		// (a.walling 2012-04-25 17:49) - PLID 49996 - Since these are resources and already in memory, we can complete immediately
		hr = m_pSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, 1, 1);
		ASSERT(SUCCEEDED(hr));
	}

	return S_OK; 
}

STDMETHODIMP Root::Terminate(DWORD dwOptions)
{ 
	m_pSink = NULL;
	m_pStream = NULL;
	return S_OK;
}

/// IInternetProtocol
STDMETHODIMP Root::Read(void *pv, ULONG cb, ULONG *pcbRead) 
{ 	
	if (!pv || !pcbRead) return E_POINTER;

	HRESULT hrRet = S_OK;
	HRESULT hr = S_OK;
	if (m_pStream) {
		hrRet = m_pStream->Read(pv, cb, pcbRead);
	}

	if (SUCCEEDED(hrRet) && cb != 0 && *pcbRead == 0) {
		if (m_pSink) {
			hr = m_pSink->ReportResult(S_FALSE, 0, 0);
			//ASSERT(SUCCEEDED(hr));
		}
		hrRet = S_FALSE;
	}

	return hrRet;
}

STDMETHODIMP Root::Seek(LARGE_INTEGER dlibMove,
	DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) 
{ 
	if (!m_pStream) return E_POINTER;
	return m_pStream->Seek(dlibMove, dwOrigin, plibNewPosition);
}

void Root::OpenStream()
{
	Url url = m_strUrl;

	// (a.walling 2012-09-04 12:10) - PLID 52438 - Lookup handler for nx://host
	HostStreamHandlerFn fn = GetHostExtensions().Lookup(url.host);
	if (fn) {
		m_pStream = fn(url.path, url.extra, m_pSink);
	}
}

} // namespace Protocol
} // namespace Nx