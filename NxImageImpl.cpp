#include "StdAfx.h"

#include "NxMD5.h"

#include <NxDataUtilitiesLib/iString.h>

#include <CxImage/ximage.h>

#include <map>

#include "NxImageImpl.h"
#include "NxImageCache.h"

#include <math.h>

#include "mirror.h"

// (a.walling 2013-06-27 13:21) - PLID 57348 - NxImageLib handles the cache and implements ISourceImage, ICachedImage, ITempBitmap


namespace NxImageLib
{	
	void CTempBitmap::FinalRelease()
	{
	}

	///

	void CCachedImage::FinalRelease()
	{
	}

	///

	HRESULT CSourceImage::Init(const CString& path)
	{
		m_remotePath = path;
		return S_OK;
	}

	HRESULT CreateCachedImage(HBITMAP bitmap, CSize originalSize, CSize newSize, ISourceImage* pOwner, ICachedImage** pCachedImageOut)
	{
		if (!bitmap) {
			ASSERT(FALSE);
			return E_FAIL;
		}

		HRESULT hr = S_OK;
		
		ITempBitmapPtr pBitmap;
		ICachedImagePtr pCachedImage;

		CComObject<CTempBitmap>* pBitmapObj = NULL;
		CComObject<CCachedImage>* pCachedImageObj = NULL;

		if (!SUCCEEDED(hr = pBitmapObj->CreateInstance(&pBitmapObj))) {
			return hr;
		}
		pBitmap = pBitmapObj;
		
		if (!SUCCEEDED(hr = pCachedImageObj->CreateInstance(&pCachedImageObj))) {
			return hr;
		}
		pCachedImage = pCachedImageObj;		

		pBitmapObj->Init(bitmap, newSize, originalSize);
		pCachedImageObj->Init(pOwner, pBitmap, newSize, originalSize); // this is non-owning since it is our 'default' image

		*pCachedImageOut = pCachedImage.Detach();

		return hr;
	}

	inline double log2(double d)  
	{  
		return log(d) / log(2.0);
	}

	inline bool IsFuzzyPowerOfTwo(double d, double fuzz)
	{
		double l = log2(d);
		double intPart;
		double fracPart = modf(l, &intPart);
		return fabs(fracPart) < fuzz;
	}

	// (a.walling 2013-06-27 13:28) - PLID 57349 - If enough difference between image's original size and the bounds, will resize and create new CCachedImage
	// otherwise will assign the default to the output var
	HRESULT CreateOrAssignCachedImage(ICachedImage* pCachedImageDefault, CSize originalSize, CSize bounds, ICachedImage** ppCachedImageOut)
	{
		HRESULT hr = S_OK;

		CSize imgSize(pCachedImageDefault->Size);

		CSize mipSize = ShrinkFit(originalSize, bounds);

		double imgRatio = Ratio(imgSize.cx, mipSize.cx);

		if (imgRatio <= 1.2) {
			// don't bother resizing if not enough difference
			pCachedImageDefault->AddRef();
			*ppCachedImageOut = pCachedImageDefault;
			hr = S_FALSE;
		} else {
			ITempBitmapPtr pTempBitmap = pCachedImageDefault->TempBitmap;
			HBITMAP bitmap = (HBITMAP)pTempBitmap->Handle;

			Nx::GdiObject<HBITMAP> newBitmapObj;
			{
				HBITMAP newBitmap = NULL;
				if (!::LoadThumbFromImage(bitmap, newBitmap, mipSize.cx, mipSize.cy)) {
					return E_FAIL;
				}
				newBitmapObj.Attach(newBitmap);
			}

			ICachedImagePtr pCachedImage;
			if (!SUCCEEDED(hr = CreateCachedImage(newBitmapObj.Get(), originalSize, mipSize, NULL, &pCachedImage))) {
				return hr;
			}
			newBitmapObj.Detach();

			*ppCachedImageOut = pCachedImage.Detach();
		}

		return hr;
	}

	bool CSourceImage::IsLoaded() const
	{
		if (m_localPath.IsEmpty() || (m_size.cx == 0 && m_size.cy == 0)) {
			return false;
		}

		if (m_imageHalfScreen || m_imageScreen || m_imageFull) {
			return true;
		}
		else {
			return false;
		}
	}

	HRESULT __stdcall CSourceImage::raw_Load()
	{
		AFX_MANAGE_STATE(AfxGetAppModuleState());

		HRESULT hr = S_OK;

		try {
			// (a.walling 2014-07-23 16:07) - PLID 63027 - Do not reload if we are already loaded!
			if (IsLoaded()) {
				return hr;
			}

			// (a.walling 2013-10-08 11:40) - PLID 58915 - Handle mirror:// images
			if (0 == m_remotePath.Find("mirror://")) {
				// this is a mirror image, no local cache
				m_localPath = m_remotePath;
				CString strMirrorPath = m_remotePath.Mid(9);

				int nSep = strMirrorPath.ReverseFind('-');
				if (nSep == -1) {
					hr = E_FAIL;
					Error("Failed to parse mirror image path");
					return hr;
				}

				CString strMirrorID;
				CString strIndex;
				long nIndex;
				long nCount;
				strMirrorID = strMirrorPath.Left(nSep);
				nIndex = atoi(strMirrorPath.Right(strMirrorPath.GetLength() - nSep - 1));
				
				Nx::GdiObject<HBITMAP> fullBitmap(
					Mirror::LoadMirrorImage(strMirrorID, nIndex, nCount, -1)
				);

				if (!fullBitmap) {
					hr = E_FAIL;
					Error("Failed to load mirror image");
					return hr;
				}

				BITMAP bm = {0};
				::GetObject(fullBitmap.Get(), sizeof(bm), &bm);
				m_size.SetSize(bm.bmWidth,bm.bmHeight);
				
				// store full image
				{
					ICachedImagePtr pCachedImage;
					if (!SUCCEEDED(hr = CreateCachedImage(fullBitmap.Get(), m_size, m_size, NULL, &pCachedImage))) {
						Error("Failed to create full cached mirror image");
						return hr;
					}
					fullBitmap.Detach();
					m_imageFull = pCachedImage;
				}

				return hr;
			}

			CxImage img;
			m_localPath = Cache::CacheAndLoadFileOnLocalMachine(m_remotePath, img);

			if (!img.IsValid()) {
				Error("Cannot load invalid image");
				return E_FAIL;
			}

			//if (img.GetBpp() < 24) {
			//	img.IncreaseBpp(24);
			//}

			m_size.SetSize(img.GetWidth(), img.GetHeight());
			
			{
				Nx::GdiObject<HBITMAP> fullBitmap(
					img.MakeBitmap()
				);
				img.Destroy();
				
				// store full image
				{
					ICachedImagePtr pCachedImage;
					if (!SUCCEEDED(hr = CreateCachedImage(fullBitmap.Get(), m_size, m_size, NULL, &pCachedImage))) {
						Error("Failed to create full cached image");
						return hr;
					}
					fullBitmap.Detach();
					m_imageFull = pCachedImage;
				}
			}

			// (a.walling 2013-06-27 13:28) - PLID 57349 - Store the screen-resized image for common operations
			CSize virtualScreenSize(::GetSystemMetrics(SM_CXVIRTUALSCREEN), ::GetSystemMetrics(SM_CYVIRTUALSCREEN));
			if (!SUCCEEDED(hr = CreateOrAssignCachedImage(m_imageFull, m_size, virtualScreenSize, &m_imageScreen))) {
				Error("Failed to create screen cached image");
				return hr;
			}

			// (a.walling 2013-06-27 13:28) - PLID 57349 - Free the full image if it was resized for screen
			if (m_imageScreen != m_imageFull) {
	#pragma TODO("Perhaps avoid removing the full image if the EMRPreview_UseOriginalFileSize pref is set to 1, but unsafe to access db from here")
				m_imageFull = NULL;
			}

			// (a.walling 2013-06-27 13:28) - PLID 57349 - Finally store the half screen image if still large enough
			CSize imageScreenSize = m_imageScreen->Size;
			CSize halfScreen(imageScreenSize.cx / 2, imageScreenSize.cy / 2);
			if (halfScreen.cx > 192 || halfScreen.cy >= 192) {
				if (!SUCCEEDED(hr = CreateOrAssignCachedImage(m_imageScreen, m_size, halfScreen, &m_imageHalfScreen))) {
					Error("Failed to create halfscreen cached image");
					return hr;
				}
			}

		}
		// (z.manning 2014-07-03 14:40) - PLID 62666 - Replaced catch(...) here to prevent possible memory leaks.
		NxCatchAllPreCallIgnore({
			hr = E_FAIL;
			Error(NxCatch::GetErrorMessage(__FUNCTION__));
		});
		return hr;
	}

	// (a.walling 2013-06-27 13:28) - PLID 57349 - Ensure the full image is available
	HRESULT CSourceImage::EnsureFullImage()
	{
		if (m_imageFull) {
			return S_OK;
		}

		if (m_localPath.IsEmpty()) {
			Error("Image not locally cached");
			return E_FAIL;
		}

		CxImage img;
		long nFormatID = GuessCxFormatFromExtension(m_localPath);
		if (!img.Load(m_localPath, nFormatID)) {
			if (!img.Load(m_localPath, CXIMAGE_FORMAT_UNKNOWN)) {
				//ThrowNxException("Failed to load image `%s` from path `%s` during cache reload", m_remotePath, m_localPath);
				Error("Failed to load cached image");
				return E_FAIL;
			}
		}
		
		{
			Nx::GdiObject<HBITMAP> fullBitmap(
				img.MakeBitmap()
			);
			img.Destroy();
			
			// store full image
			{
				HRESULT hr = S_OK;
				ICachedImagePtr pCachedImage;
				if (!SUCCEEDED(hr = CreateCachedImage(fullBitmap.Get(), m_size, m_size, NULL, &pCachedImage))) {
					Error("Failed to ensure full image");
					return hr;
				}
				fullBitmap.Detach();
				m_imageFull = pCachedImage;
			}
		}

		return S_OK;
	}

	inline bool FitsWithin(CSize size, CSize bounds)
	{
		return FitRatio(size, bounds) >= 1.0;
	}
	
	HRESULT CSourceImage::raw_GetCachedImage(tagSIZE sz, ICachedImage** ppCachedImage)
	{
		// if zero, get the full resolution
		if (!sz.cx && !sz.cy) {
			return get_FullCachedImage(ppCachedImage);
		}

		if (!sz.cx) {
			sz.cx = (long)(Ratio(m_size.cx, m_size.cy) * sz.cy);
		}
		if (!sz.cy) {
			sz.cy = (long)(Ratio(m_size.cy, m_size.cx) * sz.cx);
		}

		// (a.walling 2013-06-27 13:28) - PLID 57349 - Find the smallest cached image that is larger than the given size
		ICachedImage* pBestFit = NULL;

		if (m_imageHalfScreen && FitsWithin(sz, m_imageHalfScreen->Size)) {
			pBestFit = m_imageHalfScreen;
		} else if (m_imageScreen && FitsWithin(sz, m_imageScreen->Size)) {
			pBestFit = m_imageScreen;
		} else {
			HRESULT hr = EnsureFullImage();
			if (!SUCCEEDED(hr)) {
				return hr;
			}
			pBestFit = m_imageFull;
		}

		if (!pBestFit) {
			Error("Image unavailable");
			return E_FAIL;
		}

		pBestFit->AddRef();
		*ppCachedImage = pBestFit;

		return S_OK;
	}
	
	HRESULT CSourceImage::get_CommonCachedImage(ICachedImage** ppCachedImage)
	{
		if (!m_imageScreen) {
			return get_FullCachedImage(ppCachedImage);
		}

		m_imageScreen->AddRef();
		*ppCachedImage = m_imageScreen;
		return S_OK;
	}
	
	HRESULT CSourceImage::get_FullCachedImage(ICachedImage** ppCachedImage)
	{
		HRESULT hr = S_OK;
		hr = EnsureFullImage();
		if (!SUCCEEDED(hr)) {
			return hr;
		}
		if (!m_imageFull) {
			return E_FAIL;
		}

		m_imageFull->AddRef();
		*ppCachedImage = m_imageFull;
		return S_OK;
	}

	///
	
	namespace Cache
	{
		struct Manager {
			typedef boost::container::flat_map<CiString, CSourceImage*> SourceMap;

			SourceMap sources;
			CCriticalSection cs;
		} theManager;

		void UnRegisterSourceImage(const CString& path, CSourceImage* pSourceImage)
		{
			CSingleLock lock(&theManager.cs, TRUE);
			theManager.sources.erase(path);
		}

		HRESULT OpenSourceImage(const CString& strPath, ISourceImage** ppSourceImage)
		{
			HRESULT _hr = S_OK;
			ISourceImagePtr pImage;
			CComObject<CSourceImage>* pNewImage = NULL;
			{
				CSingleLock lock(&theManager.cs, TRUE);

				{
					Cache::Manager::SourceMap::iterator it = theManager.sources.find(strPath);
					if (it != theManager.sources.end() && it->second) {
						pImage = it->second;
					}
				}

				if (!pImage) {
					if (!SUCCEEDED(_hr = pNewImage->CreateInstance(&pNewImage))) {
						return _hr;
					}
					pImage = pNewImage;
					if (!SUCCEEDED(_hr = pNewImage->Init(strPath))) {
						return _hr;
					}
					// (a.walling 2014-04-24 12:00) - VS2013 - emplace rather than insert
					theManager.sources.emplace(strPath, pNewImage);
				}
			}
			if (pImage) {
				*ppSourceImage = pImage.Detach();
			} else {
				_hr = E_FAIL;
			}
			return _hr;
		}
	}
}

