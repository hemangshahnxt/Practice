#pragma once

#include "NxPluggableProtocol.h"
#include <boost/function.hpp>

// (a.walling 2012-09-04 12:10) - PLID 52438 - nx:// protocol root

namespace Nx
{

struct Url
{
	Url()
	{}

	Url(LPCTSTR szUrl);

	CString host;
	CString path;
	CString extra;

	bool IsEmpty() const
	{ return host.IsEmpty() && path.IsEmpty() && extra.IsEmpty(); }

	friend bool operator==(const Url& l, const Url& r)
	{ return l.host == r.host && l.path == r.path && l.extra == r.extra; }
};

namespace Protocol
{

class 
	ATL_NO_VTABLE 
	__declspec(uuid("{7E4E3FAD-E104-467c-88D3-B79E6F63DA8B}"))
	Root
		: public CComObjectRootEx<CComMultiThreadModel>
		, public CComCoClass<Root, &__uuidof(Root)>
		, public IInternetProtocolImpl
		, public IInternetProtocolInfoImpl // (a.walling 2013-01-02 15:53) - PLID 54408 - Implement IInternetProtocolInfo
{
public:

	BEGIN_COM_MAP(Root)
		COM_INTERFACE_ENTRY(IInternetProtocol)
		COM_INTERFACE_ENTRY(IInternetProtocolRoot)
		COM_INTERFACE_ENTRY(IInternetProtocolInfo)
	END_COM_MAP()

	Root();
	~Root();

	// (a.walling 2012-09-13 14:10) - PLID 52628 - Create no lock objects for new instances and aggregated instances
	typedef ATL::CComCreator2< ATL::CComCreator< ATL::CComObjectNoLock< Root > >, ATL::CComCreator< Nx::CComAggObjectNoLock< Root > > > _CreatorClass;
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{ return S_OK; }

	void FinalRelease();
	
	static LPCWSTR GetProtocolName()
	{ return L"nx";	}

public:
	/// IInternetProtocolRoot
	STDMETHOD(Start)(LPCWSTR wszUrl, IInternetProtocolSink *pOIProtSink,
		IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);

	STDMETHOD(Terminate)(DWORD dwOptions);

public:
	/// IInternetProtocol
	STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);

	STDMETHOD(Seek)(LARGE_INTEGER dlibMove,
		DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);

protected:
	void OpenStream();
	
	CString m_strUrl;
	IInternetProtocolSinkPtr m_pSink;
	IStreamPtr m_pStream; 
};

/// Registration

typedef boost::function<IStreamPtr(const CString&, const CString&, IInternetProtocolSink*)> HostStreamHandlerFn;
// (a.walling 2012-09-04 12:10) - PLID 52438 - handler registration for nx://host
bool RegisterHost(const CString& str, HostStreamHandlerFn fn);
bool UnregisterHost(const CString& str);

} // namespace Protocol
} // namespace Nx
