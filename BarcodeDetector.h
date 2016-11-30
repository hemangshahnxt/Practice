// BarcodeDetector.h: interface for the CBarcodeDetector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BARCODEDETECTOR_H__801A178E_8386_4407_ADA0_0B9428843F36__INCLUDED_)
#define AFX_BARCODEDETECTOR_H__801A178E_8386_4407_ADA0_0B9428843F36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/

// (a.walling 2009-04-28 08:42) - PLID 33363
#define DEPRECATE_OLD_BARCODEDETECT_METHODS

// (a.walling 2008-07-25 10:55) - PLID 7228 - Class to detect all barcodes on an image
class CBarcodeDetector  
{
public:
	CBarcodeDetector();
	virtual ~CBarcodeDetector();

#ifndef DEPRECATE_OLD_BARCODEDETECT_METHODS
	// (a.walling 2009-03-05 08:58) - PLID 33363
	static int CompareDouble(const void *pDataA, const void *pDataB);

	struct CBar
	{
		CBar() {
			dBegin = 0;
			dEnd = 0;
			bIsBlack = FALSE;
			pNextBar = NULL;
			nHeal = 0;
		};

		const double GetWidth() {
			return dEnd - dBegin;
		};

		CString GetAsString(const double& dMin, long &nCount) {
			CString str;

			int nActualWidth = int(((dEnd - dBegin) / dMin) + 0.5f) + nHeal;
			for (int i = 0; i < nActualWidth; i++) {
				if (bIsBlack) {
					str += "1";
				} else {
					str += "0";
				}

				nCount++;
			}

			return str;
		};

		double dBegin;
		double dEnd;
		BOOL bIsBlack;
		long nHeal;
		CBar* pNextBar;
	};

	struct CZone
	{
		long nBegin;
		long nWidth;
	};
	

	BYTE m_bLuminanceThreshold;
	BOOL m_bIncludeSpacesForMinimumWidthDetection;
	DWORD m_nInvalidCharacterHealingLimit;
	double m_dNormalizeLimit;
	double m_cdDifferenceTolerance;
#endif

	BOOL m_bEnableBarcodeLogging;

	enum EScanRegion {
		eScanEntireImage = 0,
		eScanTop,
		eScanBottom,
	};

	long m_nScanFreq;

	// (a.walling 2008-09-03 14:16) - PLID 22821 - Use a gdiplus bitmap
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	BOOL FindBarcode(CxImage& cxImage, OUT CStringArray& saResults);

	void SetScanRegion(EScanRegion scanRegion, double dPercent = 1.0f, long nFrequency = 100);
	void SetHwnd(HWND hwnd);
	void SetLimit(long nLimit);
	void SetEnabled(BOOL bEnable);

	inline HWND GetHwnd() {
		return m_hwndMessage;
	}
	inline BOOL GetEnabled() {
		return m_bEnabled;
	};

protected:
	UINT m_y;
	EScanRegion m_ScanRegion;
	double m_dScanPercent;

	HWND m_hwndMessage;
	BOOL m_bEnabled;

	long m_nLimit;

	void BarcodeLog(LPCTSTR str, ...);

#ifndef DEPRECATE_OLD_BARCODEDETECT_METHODS
	BOOL ScanLine(BYTE* pLine, long nWidth, OUT CStringArray& saResults);
	BOOL ScanSegment(BYTE* pLine, long nWidth, OUT CString& strResult);
	// (a.walling 2009-03-05 09:06) - PLID 33363 - New method for detecting barcodes
	BOOL ScanSegment_NewMethod(CArray<CBar*, CBar*>& arBars, OUT CString& strResult);
	BOOL ScanSegment_OldMethod(CArray<CBar*, CBar*>& arBars, OUT CString& strResult);
	CString GetCharFromBarPos(const CArray<CBar*, CBar*>& arBars, long nIndex, const double& dMinWidth, long &nWhite, long &nBlack);
#endif
	
	CMap<CString, LPCTSTR, DWORD, DWORD> m_mapDecode;
	
	// (a.walling 2009-04-28 09:08) - PLID 33363 - Barcode detection, version 3
	BOOL ScanLine_V3(BYTE* pLine, long nWidth, BYTE nMax, BYTE nMin, OUT CStringArray& saResults);
	BOOL TryScanSegment_V3(float* pLinePoints, long nBeginScan, long nWidth, OUT CString& strResult);
	
	char TryScanCharacter_V3(float* pLinePoints, long &nBegin, long nWidth);
};

#endif // !defined(AFX_BARCODEDETECTOR_H__801A178E_8386_4407_ADA0_0B9428843F36__INCLUDED_)
