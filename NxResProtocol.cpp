#include "StdAfx.h"
#include "NxResProtocol.h"

#include <NxSystemUtilitiesLib/FileUtils.h>
#include <NxSystemUtilitiesLib/PathStringUtils.h>

// (a.walling 2012-09-04 12:10) - PLID 52438 - Registration and default methods now in NxPluggableProtocol.h

// (a.walling 2012-04-25 17:49) - PLID 49996 - CNxResProtocol extends the standard res protocol by allowing a special placeholder of 0 for the module name, using the exe name instead

namespace
{

// (a.walling 2012-04-25 17:49) - PLID 49996 - Crack our URL and get relevant info
struct CNxResUrlInfo
{
	CNxResUrlInfo()
		: resourceTypeInt(NULL)
	{
	}

	BOOL Parse(const CString& strUrl)
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

			success = ::InternetCrackUrl(strUrl, strUrl.GetLength(), 0 /*ICU_DECODE | ICU_ESCAPE*/, &components);
		}

		if (success) {
			path.TrimLeft('/');

			int ixType = path.Find('/');

			if (-1 != ixType) {
				resourceTypeInt = NULL;
				resourceType = path.Left(ixType);
				path = path.Mid(ixType + 1);

				resourceType.MakeLower();
			} else {
				resourceTypeInt = RT_HTML;
				resourceType = "html";
			}

			if (path.IsEmpty() && !extra.IsEmpty() && '#' == extra.GetAt(0)) {
				path = extra;
				extra.Empty();
				resourceExtension.Empty();
			} else {
				resourceExtension = FileUtils::GetFileExtension(path);
				resourceExtension.MakeLower();
			}

			return TRUE;
		} else {
			DWORD dwErr = ::GetLastError();
			ASSERT(FALSE);
			host.Empty();
			path.Empty();
			extra.Empty();
			resourceType.Empty();
			resourceTypeInt = NULL;
			return FALSE;
		}
	}

	// (a.walling 2012-04-25 17:49) - PLID 49996 - Return the exe name if '0'
	LPCTSTR GetModuleName() const
	{
		if (1 == host.GetLength() && '0' == host.GetAt(0)) {
			return GetAppModuleName();
		}
		return host;
	}

	
	// (a.walling 2012-05-03 09:06) - PLID 50160 - There were a lot of possibilities for how to determine the search paths, but I ended up at this:
	//  0) Let ModulePath == the path of the .exe
	//  1) Look for ModulePath/ResourceType/ResourceName -- note this can use integer identifiers as well, eg 23 is the RT_HTML resource type.
	//  2) If not found, walk up the directory tree until we find a res/ subdirectory, and look for /res/ResourceName (no resource type)
	//       in most cases this will find the actual Practice res file
	// Note this may not be optimal for all situations obviously, but all the heavy lifting is done, and this can be tweaked or improved in the 
	// future as the situation arises.


	// (a.walling 2012-05-03 09:06) - PLID 50160 - Returns ModulePath/ResourceType/ResourceName
	CString GetDevModePath() const
	{
		CString str = GetAppModuleFolder();
		LPCTSTR szType = GetResourceType();
		if (IS_INTRESOURCE(szType)) {
			str ^= FormatString("%lu", (UINT)szType);
		} else {
			str ^= szType;
		}
		
		return str ^ GetResourceName();
	}
	
	// (a.walling 2012-05-03 09:06) - PLID 50160 - Returns [ModulePath/../+]res/ResourceName
	CString GetDevModeResPath() const
	{
		CString str = GetAppModuleResFolder();
		
		return str ^ GetResourceName();
	}
	
	// (a.walling 2012-05-03 09:06) - PLID 50160 - Containing folder of the module
	static LPCTSTR GetAppModuleFolder()
	{
		static CString strAppModuleFolder;
		
		if (strAppModuleFolder.IsEmpty()) {
			CString str;

			::GetModuleFileName(
				NULL, 
				CString::CStrBuf(str, MAX_PATH)
				, MAX_PATH
			);

			strAppModuleFolder = FileUtils::GetFilePath(str);
		}

		return strAppModuleFolder;
	}
	
	// (a.walling 2012-05-03 09:06) - PLID 50160 - Walks up directories from the module folder looking for a res/ subdirectory
	static LPCTSTR GetAppModuleResFolder()
	{
		static CString strAppModuleResFolder;
		
		if (strAppModuleResFolder.IsEmpty()) {
			CString str = GetAppModuleFolder();
			str ^= GetAppModuleName();

			CString strParent, strCheck;

			do {
				strParent = FileUtils::GetFilePath(str);

				strCheck = strParent ^ "res";

				if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(strCheck) || strParent == str) {
					strAppModuleResFolder = strCheck;
					return strAppModuleResFolder;
				}

				str = strParent;

			} while(true);
		}

		return strAppModuleResFolder;
	}

	static LPCTSTR GetAppModuleName()
	{
		static CString strAppModuleName;

		if (strAppModuleName.IsEmpty()) {
			CString str;

			::GetModuleFileName(
				NULL, 
				CString::CStrBuf(str, MAX_PATH)
				, MAX_PATH
			);

			strAppModuleName = FileUtils::GetFileName(str);
		}

		return strAppModuleName;
	}

	// (a.walling 2012-04-25 17:49) - PLID 49996 - Guess the MIME type
	LPCWSTR GetMimeType() const
	{
		LPCWSTR mimeType;

		mimeType = FindMimeType(resourceExtension);
		if (!mimeType) {
			mimeType = FindMimeType(resourceType);
		}
		if (!mimeType) {
			ASSERT(FALSE);
			return L"text/plain";
		}

		return mimeType;
	}

	CString GetResourceName() const
	{
		return path;
	}

	LPCTSTR GetResourceType() const
	{
		if (resourceTypeInt) {
			return resourceTypeInt;
		}
		return resourceType;
	}

protected:

	// (a.walling 2012-04-25 17:49) - PLID 49996 - Guess the MIME type
	static LPCWSTR FindMimeType(const CString& str)
	{
		if (str.IsEmpty()) {
			return NULL;
		}

		if (str == "html" || str == "htm") {
			return L"text/html";
		}

		if (str == "js" || str == "javascript") {
			return L"text/javascript";
		}

		if (str == "css") {
			return L"text/css";
		}

		if (str == "jpg" || str == "jpeg") {
			return L"image/jpeg";
		}

		if (str == "png") {
			return L"image/png";
		}

		if (str == "xml" || str == "xsd") {
			return L"text/xml";
		}

		if (str == "xsl") {
			return L"text/xsl";
		}

		if (str == "bmp") {
			return L"image/bmp";
		}

		if (str == "gif") {
			return L"image/gif";
		}

		ASSERT(FALSE);
		return L"text/plain";
	}

	CString host;
	CString path;
	CString extra;

	//
	CString resourceExtension;
	CString resourceType;
	LPCTSTR resourceTypeInt;
};

} // namespace

///


// (a.walling 2012-05-03 09:06) - PLID 50160 - DevMode will search for an override file to use instead of the actual resource.

#ifdef _DEBUG
static bool isDevMode = false;
#else
static bool isDevMode = false;
#endif

void CNxResProtocol::EnableDevMode()
{
	isDevMode = true;
}

bool CNxResProtocol::IsDevMode()
{
	return isDevMode;
}

CNxResProtocol::CNxResProtocol()
	: m_pResource(NULL)
	, m_pResourceEnd(NULL)
	, m_pPosition(NULL)
	, m_hModule(NULL)
	, m_hFile(NULL)
{
}

CNxResProtocol::~CNxResProtocol()
{
	FinalRelease();
}

void CNxResProtocol::FinalRelease()
{
	m_pPosition = NULL;
	m_pResource = NULL;
	m_pResourceEnd = NULL;

	if (m_hModule) {
		::FreeLibrary(m_hModule);
		m_hModule = NULL;
	}

	// (a.walling 2012-05-03 09:06) - PLID 50160 - Clear out our file handle if necessary
	if (m_hFile) {
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

// (a.walling 2012-04-25 17:49) - PLID 49996 - Start a request
STDMETHODIMP CNxResProtocol::Start(LPCWSTR wszUrl, 
	IInternetProtocolSink *pOIProtSink, IInternetBindInfo* pOIBindInfo, 
	DWORD grfPI, HANDLE_PTR dwReserved)
{
	if (!wszUrl || !pOIProtSink) {
		return E_POINTER;
	}

	CComPtr<IInternetProtocolSink> pSink = pOIProtSink;

	DWORD grfBINDF = 0;
	BINDINFO bi = {sizeof(bi)};
	if (pOIBindInfo) {
		pOIBindInfo->GetBindInfo(&grfBINDF, &bi);
	}

	if (!Init(wszUrl, pOIProtSink)) {		
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
	}

	DWORD total = 0;
	if (m_hFile) {
		total = ::GetFileSize(m_hFile, NULL);
	} else {
		total = m_pResourceEnd - m_pResource;
	}

	// (a.walling 2013-01-02 11:19) - PLID 54248 - if the bind flags include BINDF_NEEDFILE, we must reply with a cache file name.
	// Otherwise, synchronous calls (eg BindToStorage, UrlDownloadToFile, UrlOpenBlockingStream etc) will fail with INET_E_DATA_NOT_AVAILABLE
	// However, asynchronous operations do not seem to require a file, even though BINDF_NEEDFILE is set.
	if (!(grfBINDF & BINDF_ASYNCHRONOUS)) {
		if (grfBINDF & BINDF_NEEDFILE) {
			if (m_wstrCacheFile.IsEmpty()) {
				CString strCacheFile;
				HANDLE h = CreateNxTempFile("nxres", "tmp", &strCacheFile);
				if (h != INVALID_HANDLE_VALUE) {
					DWORD written = 0;
					if (m_pResource) {
						::WriteFile(h, m_pResource, m_pResourceEnd - m_pResource, &written, NULL);
					} else if (m_hFile) {
						DWORD readen = 0; // hahaha
						BYTE buf[0x1000];
						while (::ReadFile(m_hFile, buf, sizeof(buf), &readen, NULL) && (readen != 0)) {
							::WriteFile(h, buf, readen, &written, NULL);
						}
						static LONG highPos = 0;
						::SetFilePointer(m_hFile, 0, &highPos, FILE_BEGIN);
					}
					::CloseHandle(h);
					m_wstrCacheFile = CStringW(strCacheFile);
					FileUtils::DeleteFileOnTerm(strCacheFile);
				}
			}
		}
	}

	// (a.walling 2012-04-25 17:49) - PLID 49996 - Since these are resources and already in memory, we can complete immediately
	
	// (a.walling 2013-01-02 11:19) - PLID 54248 - if the bind flags include BINDF_NEEDFILE, we must reply with a cache file name.
	if (!m_wstrCacheFile.IsEmpty()) {
		pSink->ReportProgress(BINDSTATUS_CACHEFILENAMEAVAILABLE, m_wstrCacheFile);
	}
	pSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, total, total);
	pSink->ReportResult(S_OK, 0, NULL);

	return S_OK;
}

// (a.walling 2012-04-25 17:49) - PLID 49996 - Load up the resource and set our internal pointers
bool CNxResProtocol::Init(LPCWSTR wszUrl, IInternetProtocolSink *pOIProtSink)
{	
	CNxResUrlInfo urlInfo;

	if (!urlInfo.Parse(wszUrl)) return false;

	// (a.walling 2012-05-03 09:06) - PLID 50160 - If DevMode, search for override file
	if (IsDevMode()) {
		CString strUrl(wszUrl);
		CString strPath = urlInfo.GetDevModePath();

		//TRACE("nxres-DevMode: url `%s` checking for override at `%s`\n", strUrl, strPath);

		//if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(strPath)) {
		m_hFile = ::CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE == m_hFile) {
			m_hFile = NULL;
		} else {
			TRACE("nxres-DevMode: \tNow using `%s` for url `%s`\n", strPath, strUrl);
		}

		if (!m_hFile) {
			strPath = urlInfo.GetDevModeResPath();

			//TRACE("nxres-DevMode: url `%s` checking for res override at `%s`\n", strUrl, strPath);

			m_hFile = ::CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
			if (INVALID_HANDLE_VALUE == m_hFile) {
				m_hFile = NULL;
			} else {
				TRACE("nxres-DevMode: \tNow using res `%s` for url `%s`\n", strPath, strUrl);
			}
		}
	}

	if (!m_hFile) {

		m_hModule = ::LoadLibrary(urlInfo.GetModuleName());
		if (!m_hModule) return false;

		HRSRC hRes = ::FindResource(m_hModule, urlInfo.GetResourceName(), urlInfo.GetResourceType());
		if (!hRes) return false;

		{
			HGLOBAL notAnHGlobal = ::LoadResource(m_hModule, hRes);
			if (!notAnHGlobal) return false;
			
			ULONG nSize = ::SizeofResource(m_hModule, hRes);
			if (!nSize) return false;

			void* pData = ::LockResource(notAnHGlobal);
			if (!pData) return false;

			m_pResource = static_cast<BYTE*>(pData);
			m_pResourceEnd = m_pResource + nSize;

			m_pPosition = m_pResource;
		}
	}

	m_spOIProtSink = pOIProtSink;

	m_spOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, urlInfo.GetMimeType());

	return true;
}

STDMETHODIMP CNxResProtocol::Terminate(DWORD dwOptions)
{
 	m_spOIProtSink.Release();

	// (a.walling 2013-11-18 11:37) - PLID 59557 - fix early file close upon IInternetProtocol::Terminate

	return S_OK;
}

// (a.walling 2012-04-25 17:49) - PLID 49996 - Copies the data into the requested buffer
STDMETHODIMP CNxResProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	if (!pv || !pcbRead) {
		return E_POINTER;
	}

	if (0 == cb) {
		*pcbRead = 0;
		return S_OK;
	}

	// (a.walling 2012-05-03 09:06) - PLID 50160 - If DevMode, just read from the file directly. Since this is only for debugging, implementing an asynchronous protocol handler was not worth the time
	if (m_hFile) {
		DWORD bytesRead = 0;
		BOOL success = ::ReadFile(m_hFile, pv, cb, &bytesRead, NULL);
		*pcbRead = bytesRead;
		//if (!success) {
		//	return HRESULT_FROM_WIN32(GetLastError());
		//}
	} else {
		ULONG cbRemaining = m_pResourceEnd - m_pPosition;
		*pcbRead = min(cb, cbRemaining);

		memcpy(pv, m_pPosition, *pcbRead);
		m_pPosition += *pcbRead;
	}

	if (*pcbRead < cb) {
 		return S_FALSE;
	}

	return S_OK;
}

// (a.walling 2012-04-25 17:49) - PLID 49996 - Seek the current position as appropriate
STDMETHODIMP CNxResProtocol::Seek(LARGE_INTEGER dlibMove,
    DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	// (a.walling 2012-05-03 09:06) - PLID 50160 - If DevMode, just seek the file pointer directly
	if (m_hFile) {
		DWORD newPos = ::SetFilePointer(m_hFile, dlibMove.LowPart, NULL, dwOrigin);
		if (!newPos) {
			return E_FAIL;
		}
		if (plibNewPosition) {
			plibNewPosition->QuadPart = newPos;
		}
		return S_OK;
	}

	BYTE* pos = NULL;

	if (dwOrigin == FILE_BEGIN) {
		pos = m_pResource + dlibMove.QuadPart;
	} else if (dwOrigin == FILE_END) {
		pos = m_pResourceEnd - dlibMove.QuadPart;
	} else if (dwOrigin == FILE_CURRENT) {		
		pos = m_pPosition + dlibMove.QuadPart;
	} else {
		return E_FAIL;
	}

	if (pos > m_pResourceEnd || pos < m_pResource) {
		return E_FAIL;
	}

	m_pPosition = pos;

	if (plibNewPosition) {
		plibNewPosition->QuadPart = (m_pPosition - m_pResource);
	}

	return S_OK;
}