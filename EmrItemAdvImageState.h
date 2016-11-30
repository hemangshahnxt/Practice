#pragma once

// (j.armen 2014-07-21 16:32) - PLID 62836 - Moved class to its own file

class CEmrItemAdvImageState
{
public:
	CEmrItemAdvImageState();
	CEmrItemAdvImageState(const _variant_t& vtState);

public:
	DWORD m_dwVersion;
	CString m_strImagePathOverride;
	enum eImageType m_eitImageTypeOverride;
	_variant_t m_varInkData;
	_variant_t m_varTextData;
	// (z.manning 2011-10-05 15:59) - PLID 45842 - Variant to keep track of the print data for 3D images
	_variant_t m_varPrintData;

	// (z.manning 2010-02-23 11:25) - PLID 37412 - Added another variant for the detail image stamp data. This
	// is somewhat redunant with m_varTextData, however, for many reasons needs to be separate. This data is not
	// needed by the ink picture control and it has its own table in data (EmrDetailImageStampsT).  
	_variant_t m_varDetailImageStamps;

	//DRT 1/18/2008 - PLID 28603 - We now keep a list of which hot spots are selected.
	//	Note that this is not the list of hotspots.  We currently do not consider them
	//	part of the state, because they are unchangeable.
	//This state is just a semicolon delimited list of EMRImageHotSpot IDs that are selected.
	CString m_strSelectedHotSpotData;
	// (a.walling 2013-06-06 09:41) - PLID 57069 - Hash of the state blob (via CreateFromSafeArrayVariant)
	NxMD5 m_hash;
public:
	_variant_t AsSafeArrayVariant();
	void CreateFromSafeArrayVariant(const _variant_t &varState);
	CString CreateByteStringFromInkData();
	CString CreateByteStringFromTextData();

	//TES 5/15/2008 - PLID 27776 - We need to be able to compare image states, to tell if one's been modified.
	BOOL operator ==(CEmrItemAdvImageState &eiaisCompare);

public:
	CString CalcCurrentBackgroundImageFullPath(IN long nPatIDIfNecessary, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType) const;
	//TES 2/7/2007 - PLID 18159 - Also, pass in the actual size of the image, as stored on the EMN, so this function can
	// scale the ink and text appropriately.
	// (a.walling 2008-02-13 14:10) - PLID 28605 - Pass in the detail to be able to render hot spots on images
	HBITMAP Render(OPTIONAL IN CEMNDetail* pDetail, OPTIONAL OUT CSize *pszOutputBmpSize, IN CRect rImageSize, IN const long nPatIDIfNecessary, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType) const;

	// (z.manning, 01/22/2008) - PLID 28690 - Sets the selected hot spots from an array of hot spot IDs
	void SetSelectedHotSpots(IN CArray<long, long> &arynHotSpotIDs);
};