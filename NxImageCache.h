#pragma once

#include <NxUILib/NxImage.h>

// (a.walling 2013-06-27 13:21) - PLID 57348 - NxImageLib handles the cache

namespace NxImageLib
{	

	namespace Cache
	{
		HRESULT OpenSourceImage(const CString& strPath, ISourceImage** ppSourceImage);

		inline ISourceImagePtr OpenSourceImage(const CString& strPath)
		{			
			ISourceImage* _result = NULL;
			HRESULT _hr = OpenSourceImage(strPath, &_result);
			if (FAILED(_hr)) _com_raise_error(_hr);
			return ISourceImagePtr(_result, false);
		}

		///

		CString CalcLocalCachedFileName(CString strRemotePath);

		// finds an existing image and verifies its timestamp; returns "" if invalid
		CString FindExistingLocalCachedImage(const CString& strRemotePath, WIN32_FILE_ATTRIBUTE_DATA& remoteAttributes);

		// (a.walling 2010-10-25 10:04) - PLID 41043 - Copy the remote file to the local machine if necessary
		// (a.walling 2013-06-06 09:42) - PLID 57070 - Amortize image load CPU across network IO
		// returns local path to cached file, or empty if failure
		CString CacheAndLoadFileOnLocalMachine(const CString& strRemotePath, OUT CxImage& img);
	}
}

