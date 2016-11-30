#pragma once

#include <NxUILib/NxImage.h>
#include <NxSystemUtilitiesLib/NxHandle.h>

#include <boost/container/flat_map.hpp>

// (a.walling 2013-06-27 13:21) - PLID 57348 - NxImageLib handles the cache and implements ISourceImage, ICachedImage, ITempBitmap

namespace NxImageLib
{	

class CSourceImage;
class CCachedImage;
class CTempBitmap;

namespace Cache
{
	void UnRegisterSourceImage(const CString& path, CSourceImage* pSourceImage);
}

///

#define DECLARE_NOT_AGGREGATABLE_NOLOCK(x)  public:\
	typedef ATL::CComCreator2< ATL::CComCreator< ATL::CComObjectNoLock< x > >, ATL::CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;

///

class 
	__declspec(novtable) 
	__declspec(uuid("{3193AE26-F933-425d-84D8-E7966EE6653C}"))
	CTempBitmap
		: public CComObjectRootEx<CComMultiThreadModel>
		, public CComCoClass<CTempBitmap, &__uuidof(CTempBitmap)>
		, public ISupportErrorInfoImpl<&__uuidof(CTempBitmap)>
		, public ITempBitmap
{
public:

	BEGIN_COM_MAP(CTempBitmap)
		COM_INTERFACE_ENTRY(ITempBitmap)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(CTempBitmap);

	CTempBitmap()
		: m_size(0, 0)
		, m_originalSize(0, 0)
	{}
	~CTempBitmap()
	{}
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{ return S_OK; }

	void FinalRelease();

	void Init(HBITMAP bitmap, SIZE size, SIZE originalSize)
	{
		m_bitmap.Attach(bitmap);
		m_size = size;
		m_originalSize = originalSize;
	}

	/// ITempBitmap

	virtual HRESULT __stdcall get_Handle(OLE_HANDLE* pHandle) override
	{
		if (!m_bitmap) {
			return E_POINTER;
		}
		*pHandle = (OLE_HANDLE)m_bitmap.Get();
		return S_OK;
	}

	virtual HRESULT __stdcall get_Size(tagSIZE* pSize) override
	{
		*pSize = m_size;
		return S_OK;
	}

	virtual HRESULT __stdcall get_SourceSize(tagSIZE* pSize) override
	{
		*pSize = m_originalSize;
		return S_OK;
	}

protected:

	CSize m_size;
	CSize m_originalSize;
	Nx::GdiObject<HBITMAP> m_bitmap;
};

///

class 
	__declspec(novtable) 
	__declspec(uuid("{0294BC51-64D3-438c-81FD-BD5E2FE5260B}"))
	CCachedImage
		: public CComObjectRootEx<CComMultiThreadModel>
		, public CComCoClass<CCachedImage, &__uuidof(CCachedImage)>
		, public ISupportErrorInfoImpl<&__uuidof(CCachedImage)>
		, public ICachedImage
{
public:
	friend class CSourceImage;

	BEGIN_COM_MAP(CCachedImage)
		COM_INTERFACE_ENTRY(ICachedImage)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(CCachedImage);

	CCachedImage()
		: m_size(0, 0)
	{}
	~CCachedImage()
	{}
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{ return S_OK; }

	void FinalRelease();

	void Init(ISourceImage* source, ITempBitmap* bitmap, CSize size, CSize originalSize)
	{
		m_source = source;
		m_tempBitmap = bitmap;
		m_size = size;
		m_originalSize = originalSize;
	}

	/// ICachedImage

	virtual HRESULT __stdcall get_Size(tagSIZE* pSize) override
	{
		*pSize = m_size;
		return S_OK;
	}

	virtual HRESULT __stdcall get_SourceSize(tagSIZE* pSize) override
	{
		*pSize = m_originalSize;
		return S_OK;
	}

	virtual HRESULT __stdcall get_TempBitmap(ITempBitmap** ppTempBitmap) override
	{
		if (!m_tempBitmap) {
			return E_FAIL;
		}

		m_tempBitmap->AddRef();
		*ppTempBitmap = m_tempBitmap;
		return S_OK;
	}

protected:

	CSize m_size;
	CSize m_originalSize;

	ISourceImagePtr m_source; // may be null
	ITempBitmapPtr m_tempBitmap;
};

///

class 
	__declspec(novtable) 
	__declspec(uuid("{3134835C-5A31-48c5-BD8B-D2942399DFFC}"))
	CSourceImage
		: public CComObjectRootEx<CComMultiThreadModel>
		, public CComCoClass<CSourceImage, &__uuidof(CSourceImage)>
		, public ISupportErrorInfoImpl<&__uuidof(CSourceImage)>
		, public ISourceImage
{
public:

	BEGIN_COM_MAP(CSourceImage)
		COM_INTERFACE_ENTRY(ISourceImage)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(CSourceImage);

	CSourceImage()
		: m_size(0, 0)
	{}
	~CSourceImage()
	{}
	
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{ 
		return S_OK; 
	}

	void FinalRelease()
	{
		Cache::UnRegisterSourceImage(m_remotePath, this);
	}

	/// CSourceImage

	HRESULT Init(const CString& path);
	bool IsLoaded() const;

	/// ISourceImage

	virtual HRESULT __stdcall raw_Load() override;

	virtual HRESULT __stdcall get_Size(tagSIZE* pSize) override
	{
		*pSize = m_size;
		return S_OK;
	}

	virtual HRESULT __stdcall get_Path(BSTR* pPath) override
	{
		*pPath = m_remotePath.AllocSysString();
		return S_OK;
	}

	virtual HRESULT __stdcall get_LocalPath(BSTR* pPath) override
	{
		*pPath = m_localPath.AllocSysString();
		return S_OK;
	}

	virtual HRESULT __stdcall raw_GetCachedImage(tagSIZE sz, ICachedImage** ppCachedImage) override;
	virtual HRESULT __stdcall get_CommonCachedImage(ICachedImage** ppCachedImage) override;
	virtual HRESULT __stdcall get_FullCachedImage(ICachedImage** ppCachedImage) override;

protected:

	HRESULT EnsureFullImage();

	CSize m_size;

	CString m_remotePath;
	CString m_localPath;

	ICachedImagePtr m_imageFull;
	ICachedImagePtr m_imageScreen;
	ICachedImagePtr m_imageHalfScreen;
};

}
