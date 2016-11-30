#pragma once

#include <wininet.h>
#include <urlmon.h>
#include <comdef.h>


// (a.walling 2012-09-04 12:10) - PLID 52438 - Pluggable protocol registration and default IInternetProtocol implementation

// (a.walling 2012-04-25 17:49) - PLID 49996 - CNxPluggableProtocolRegistration facilitates automatically registering and unregistering an asynchronous pluggable protocol
// ProtocolImpl must implement the function LPCWSTR ProtocolImpl::GetProtocolName() to return the name of the protocol
namespace Nx
{

// (a.walling 2012-09-13 14:10) - PLID 52628 - Based on CComAggObject; does not lock the module state
//contained is the user's class that derives from CComObjectRoot and whatever
//interfaces the user wants to support on the object
template <class contained>
class CComAggObjectNoLock :
	public IUnknown,
	public ATL::CComObjectRootEx< typename contained::_ThreadModel::ThreadModelNoCS >
{
public:
	typedef contained _BaseClass;
	CComAggObjectNoLock(void* pv) : m_contained(pv)
	{
	}
	HRESULT _AtlInitialConstruct()
	{
		HRESULT hr = m_contained._AtlInitialConstruct();
		if (SUCCEEDED(hr))
		{
			hr = CComObjectRootEx< typename contained::_ThreadModel::ThreadModelNoCS >::_AtlInitialConstruct();
		}
		return hr;
	}
	//If you get a message that this call is ambiguous then you need to
	// override it in your class and call each base class' version of this
	HRESULT FinalConstruct()
	{
		CComObjectRootEx<contained::_ThreadModel::ThreadModelNoCS>::FinalConstruct();
		return m_contained.FinalConstruct();
	}
	void FinalRelease()
	{
		CComObjectRootEx<contained::_ThreadModel::ThreadModelNoCS>::FinalRelease();
		m_contained.FinalRelease();
	}
	// Set refcount to -(LONG_MAX/2) to protect destruction and 
	// also catch mismatched Release in debug builds
	virtual ~CComAggObjectNoLock()
	{
		m_dwRef = -(LONG_MAX/2);
		FinalRelease();
	}

	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG l = InternalRelease();
		if (l == 0)
			delete this;
		return l;
	}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		ATLASSERT(ppvObject != NULL);
		if (ppvObject == NULL)
			return E_POINTER;
		*ppvObject = NULL;

		HRESULT hRes = S_OK;
		if (InlineIsEqualUnknown(iid))
		{
			*ppvObject = (void*)(IUnknown*)this;
			AddRef();
		}
		else
			hRes = m_contained._InternalQueryInterface(iid, ppvObject);
		return hRes;
	}
	template <class Q>
	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
	{
		return QueryInterface(__uuidof(Q), (void**)pp);
	}
	static HRESULT WINAPI CreateInstance(LPUNKNOWN pUnkOuter, CComAggObject<contained>** pp)
	{
		ATLASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		CComAggObjectNoLock<contained>* p = NULL;
		ATLTRY(p = new CComAggObjectNoLock<contained>(pUnkOuter))
		if (p != NULL)
		{
			p->SetVoid(NULL);
			p->InternalFinalConstructAddRef();
			hRes = p->_AtlInitialConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->FinalConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->_AtlFinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes != S_OK)
			{
				delete p;
				p = NULL;
			}
		}
		*pp = p;
		return hRes;
	}

	CComContainedObject<contained> m_contained;
};

namespace Protocol
{
template<typename ProtocolImpl>
class Registration
{
public:
	Registration()
	{}

	~Registration()
	{
		Unregister();
	}

	HRESULT Register()
	{
		CComPtr<IInternetSession> pSession;
		CComPtr<IClassFactory> pFactory;
		HRESULT hr;
		
		hr = ::CoInternetGetSession(0, &pSession, 0);		
		if (!SUCCEEDED(hr)) {ASSERT(FALSE); return hr;}

		hr = ProtocolImpl::_ClassFactoryCreatorClass::CreateInstance(
			&ProtocolImpl::_CreatorClass::CreateInstance
			, __uuidof(IClassFactory)
			, (void**)&pFactory
		);
		if (!SUCCEEDED(hr)) {ASSERT(FALSE); return hr;}

		hr = pSession->RegisterNameSpace(pFactory, __uuidof(ProtocolImpl), ProtocolImpl::GetProtocolName(), 0, NULL, 0);

		if (!SUCCEEDED(hr)) {ASSERT(FALSE); return hr;}

		m_pSession = pSession;
		m_pFactory = pFactory;

		return hr;
	}

	HRESULT Unregister()
	{		
		if (!m_pSession || !m_pFactory) return S_FALSE;

		HRESULT hr;

		hr = m_pSession->UnregisterNameSpace(m_pFactory, ProtocolImpl::GetProtocolName());
		
		if (!SUCCEEDED(hr)) {ASSERT(FALSE);}

		m_pSession.Release();
		m_pFactory.Release();

		return hr;
	}

	CComPtr<IInternetSession> m_pSession;
	CComPtr<IClassFactory> m_pFactory;
};

class ATL_NO_VTABLE IInternetProtocolImpl
	: public IInternetProtocol
{
public:
	static LPCWSTR GetProtocolName(); // undefined so you will get an error if you use it without defining it!

	/// IInternetProtocolRoot
	STDMETHOD(Start)(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
		IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
	= 0; // must implement

	STDMETHOD(Continue)(PROTOCOLDATA *pProtocolData)
	{ return S_OK; }

	STDMETHOD(Abort)(HRESULT hrReason, DWORD dwOptions)
	{ return S_OK; }

	STDMETHOD(Terminate)(DWORD dwOptions)
	{ return S_OK; }

	STDMETHOD(Suspend)()
	{ return E_NOTIMPL; }

	STDMETHOD(Resume)()
	{ return E_NOTIMPL; }

public:
	/// IInternetProtocol
	STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead) 
	= 0; // must implement

	STDMETHOD(Seek)(LARGE_INTEGER dlibMove,
		DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) 
	= 0; // must implement

	STDMETHOD(LockRequest)(DWORD dwOptions)
	{ return S_OK; }

	STDMETHOD(UnlockRequest)()
	{ return S_OK; }
};

// (a.walling 2013-01-02 11:40) - PLID 54408 - IInternetProtocolInfo implementation
class ATL_NO_VTABLE IInternetProtocolInfoImpl
	: public IInternetProtocolInfo
{
public:
	// returns PARSE_SECURITY_URL as url file://c:\ 
	// returns PARSE_DOMAIN as localhost
    STDMETHOD(ParseUrl)( 
        /* [in] */ LPCWSTR pwzUrl,
        /* [in] */ PARSEACTION ParseAction,
        /* [in] */ DWORD dwParseFlags,
        /* [out] */ LPWSTR pwzResult,
        /* [in] */ DWORD cchResult,
        /* [out] */ DWORD *pcchResult,
        /* [in] */ DWORD dwReserved)
	{
		HRESULT hr = INET_E_DEFAULT_ACTION;

		if (!pcchResult || !pwzResult) {
			return E_POINTER;
		}

		if (PARSE_SECURITY_URL == ParseAction) {

			const wchar_t wszDefault[] = L"file://c:\\";

			wcsncpy_s(pwzResult, cchResult, wszDefault, _TRUNCATE);
			hr = S_OK;
		} else if (PARSE_DOMAIN == ParseAction) {
			const wchar_t wszLocalHost[] = L"localhost";
			wcscpy(pwzResult, wszLocalHost);
			*pcchResult = wcslen(wszLocalHost) + 1;
			return S_OK;
		}

		return hr;
	}
    
    STDMETHOD(CombineUrl)( 
        /* [in] */ LPCWSTR pwzBaseUrl,
        /* [in] */ LPCWSTR pwzRelativeUrl,
        /* [in] */ DWORD dwCombineFlags,
        /* [out] */ LPWSTR pwzResult,
        /* [in] */ DWORD cchResult,
        /* [out] */ DWORD *pcchResult,
		/* [in] */ DWORD dwReserved)
	{
		return INET_E_DEFAULT_ACTION;
	}
    
    STDMETHOD(CompareUrl)( 
        /* [in] */ LPCWSTR pwzUrl1,
        /* [in] */ LPCWSTR pwzUrl2,
        /* [in] */ DWORD dwCompareFlags)
	{
		return E_NOTIMPL;
	}
    
	// QUERY_IS_SAFE = TRUE
	// QUERY_USES_NETWORK = FALSE
	// QUERY_IS_SECURE = FALSE
    STDMETHOD(QueryInfo)( 
        /* [in] */ LPCWSTR pwzUrl,
        /* [in] */ QUERYOPTION QueryOption,
        /* [in] */ DWORD dwQueryFlags,
        /* [size_is][out][in] */ LPVOID pBuffer,
        /* [in] */ DWORD cbBuffer,
        /* [out][in] */ DWORD *pcbBuf,
        /* [in] */ DWORD dwReserved)
	{
		switch (QueryOption) {
			case QUERY_IS_SAFE:
			case QUERY_USES_NETWORK:
			case QUERY_IS_SECURE:
				{
					if (!pBuffer || cbBuffer < sizeof(BOOL)) {
						return E_FAIL;
					}

					if (pcbBuf) {
						*pcbBuf = sizeof(DWORD);
					}
					DWORD* pResult = (DWORD*)pBuffer;

					if (QUERY_IS_SAFE == QueryOption) {
						*pResult = TRUE;
					} else if (QUERY_USES_NETWORK == QueryOption) {
						*pResult = FALSE;
					} else if (QUERY_IS_SECURE == QueryOption) {
						*pResult = FALSE;
					}
				}
				return S_OK;
			default:
				return INET_E_DEFAULT_ACTION;
		}
	}
};

} // namespace Protocol

} // namespace Nx