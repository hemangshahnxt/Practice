#include "StdAfx.h"

#include "NxProtocolEmr.h"

#include <NxDataUtilitiesLib/NxStream.h>

#include <FileUtils.h>

// (a.walling 2012-09-04 12:10) - PLID 52439 - Open up stamp image as an IStream


namespace Nx
{
namespace Protocol
{
namespace Emr
{

	
namespace {
class CExposedModuleStateCmdTarget : public CCmdTarget
{
public:
	AFX_MODULE_STATE* GetModuleState() const
	{
		return m_pModuleState;
	}
};
}

IStreamPtr OpenStampImageStream(const CString& strID, IInternetProtocolSink* pSink)
{
	if (strID.IsEmpty()) return NULL;
	long nStampID = atol(strID);

	// not exactly necessary, but we are appending png for good form
	if (0 != FileUtils::GetFileExtension(strID).CompareNoCase("png")) {
		ASSERT(FALSE);
	}

	IStreamPtr pStream;
	try {
		AFX_MANAGE_STATE(((CExposedModuleStateCmdTarget*)GetMainFrame())->GetModuleState());
		EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);

		if (!pStamp || !pStamp->m_ImageInfo.nNumImageBytes || !pStamp->m_ImageInfo.arImageBytes) {
			return NULL;
		}

		pStream = StreamFromMemory(
			pStamp->m_ImageInfo.arImageBytes
			, pStamp->m_ImageInfo.nNumImageBytes
		);
	} NxCatchAllCall(__FUNCTION__, {
		return NULL;
	});

	HRESULT hr = S_OK;
	if (pStream && pSink) {
		hr = pSink->ReportProgress(
			BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE
			, L"image/png"
		);
		ASSERT(SUCCEEDED(hr));
	}

	return pStream;
}

IStreamPtr OpenStream(const CString& path, const CString& extra, IInternetProtocolSink* pSink)
{
	std::vector<CString> pathParts;

	int ix = 0;
	CString str = path.Tokenize("/\\", ix);
	while (!str.IsEmpty()) {
		pathParts.push_back(str);
		str = path.Tokenize("/\\", ix);
	};

	//if (pathParts.empty()) return NULL;
	if (pathParts.size() <= 1) return NULL;

	if (pathParts[0] != "stamp") return NULL;

	return OpenStampImageStream(pathParts[1], pSink);
}


} // namespace Emr
} // namespace Protocol
} // namespace Nx

