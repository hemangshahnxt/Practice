#pragma once

#include "NxPluggableProtocol.h"

// (a.walling 2012-09-04 12:10) - PLID 52438 - Registration and default methods now in NxPluggableProtocol.h

// (a.walling 2012-04-25 17:49) - PLID 49996 - CNxResProtocol extends the standard res protocol by allowing a special placeholder of 0 for the module name, using the exe name instead

class 
	ATL_NO_VTABLE
	__declspec(uuid("{7229BE3A-4B68-4b0f-95AF-AE08CD52F0B4}"))
	CNxResProtocol
		: public CComObjectRootEx<CComMultiThreadModel>
		, public CComCoClass<CNxResProtocol, &__uuidof(CNxResProtocol)>
		, public Nx::Protocol::IInternetProtocolImpl
		, public Nx::Protocol::IInternetProtocolInfoImpl // (a.walling 2013-01-02 15:53) - PLID 54408 - Implement IInternetProtocolInfo
{
public:
	static LPCWSTR GetProtocolName() { return L"nxres"; }

	// (a.walling 2012-05-03 09:06) - PLID 50160 - DevMode will search for an override file to use instead of the actual resource.
	// There were a lot of possibilities for how to determine the search paths, but I ended up at this:
	//  0) Let ModulePath == the path of the .exe
	//  1) Look for ModulePath/ResourceType/ResourceName -- note this can use integer identifiers as well, eg 23 is the RT_HTML resource type.
	//  2) If not found, walk up the directory tree until we find a res/ subdirectory, and look for /res/ResourceName (no resource type)
	//       in most cases this will find the actual Practice res file
	// Note this may not be optimal for all situations obviously, but all the heavy lifting is done, and this can be tweaked or improved in the 
	// future as the situation arises.
	static void EnableDevMode();
	static bool IsDevMode();

	BEGIN_COM_MAP(CNxResProtocol)
		COM_INTERFACE_ENTRY(IInternetProtocol)
		COM_INTERFACE_ENTRY(IInternetProtocolRoot)
		COM_INTERFACE_ENTRY(IInternetProtocolInfo)
	END_COM_MAP()

	CNxResProtocol();
	~CNxResProtocol();
	
	// (a.walling 2012-09-13 14:10) - PLID 52628 - Create no lock objects for new instances and aggregated instances
	typedef ATL::CComCreator2< ATL::CComCreator< ATL::CComObjectNoLock< CNxResProtocol > >, ATL::CComCreator< Nx::CComAggObjectNoLock< CNxResProtocol > > > _CreatorClass;
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease();

public:	
	/// IInternetProtocolRoot
	STDMETHOD(Start)(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
		IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);

	STDMETHOD(Terminate)(DWORD dwOptions);

public:
	/// IInternetProtocol
	STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);

	STDMETHOD(Seek)(LARGE_INTEGER dlibMove,
		DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
protected:

	bool Init(LPCWSTR wszUrl, IInternetProtocolSink *pOIProtSink);

	BYTE* m_pResource;
	BYTE* m_pResourceEnd;
	BYTE* m_pPosition;
	HMODULE m_hModule;

	// (a.walling 2012-05-03 09:06) - PLID 50160 - DevMode override file handle
	HANDLE m_hFile;
	// (a.walling 2013-01-02 11:19) - PLID 54248 - if the bind flags include BINDF_NEEDFILE, we must reply with a cache file name.
	CStringW m_wstrCacheFile;
	
	CComPtr<IInternetProtocolSink> m_spOIProtSink;
};
