// BarcodeDetector.cpp: implementation of the CBarcodeDetector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BarcodeDetector.h"
#include <float.h>

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-07-25 10:55) - PLID 7228 - Class to detect all barcodes on an image
// Originally written for Gdiplus::Bitmap*s, but then changed to support CxImage instead
// due to pre-existing NxTwain code. Can easily be modified to support anything else.
// Has a limiter to break when the limit is hit, so we can break as soon as we find the
// first valid barcode.
//
// Several public members of this class can tweak some parameters, but in general they
// are not to be touched and function fine at their default values.
//
// The basic approach here is to detect quiet zones (areas of high luminosity / white)
// and then parse the noisy zones in between them for barcodes. The noisy zones are initially
// scanned to determine the width of the smallest bar. By default only black bars are considered,
// due to gray interpolation causing white bars to appear much smaller than they really are.
// Once the minimum bar width is calculated, the rest of the scanline can be translated into
// integer widths, which correspond to the number of digits in their 'code', using 1 for black
// and 0 for white. This may not be the most efficient implementation by looking up the codes via
// a string map, but it works out well for debugging, and the performance is more than acceptible,
// especially when we control the scan region of the image (currently the top third when set by
// Multiple Document Scanning).

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define START_A 103
#define START_B 104
#define START_C 105
#define CODE_A 101
#define CODE_B 100
#define CODE_C 99
#define STOP 127

#define START_A_STR "11010000100"
#define START_B_STR "11010010000"
#define START_C_STR "11010011100"

#define FLOAT_EQ(x,v) (((v - FLT_EPSILON) < x) && (x <( v + FLT_EPSILON)))
#define DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

#define DEBUG_SCANENTIREIMAGE
#define DEBUG_NOBARCODELIMIT

//#define AVERAGE_2PIXELHEIGHT


CBarcodeDetector::CBarcodeDetector()
{
#ifdef _DEBUG
	m_bEnableBarcodeLogging = TRUE;
#else
	m_bEnableBarcodeLogging = FALSE;
#endif


#ifndef DEPRECATE_OLD_BARCODEDETECT_METHODS
	m_bLuminanceThreshold = 140;
	m_dNormalizeLimit = 220;
	m_bIncludeSpacesForMinimumWidthDetection = FALSE;
	m_cdDifferenceTolerance = 0.25f;
	m_nInvalidCharacterHealingLimit = 8;
#endif

	m_ScanRegion = eScanEntireImage;
	m_dScanPercent = 1.0f;
	m_nScanFreq = 100;

	m_hwndMessage = NULL;
	m_nLimit = 0;

	m_bEnabled = TRUE;

	m_y = 0;

	/* These values are the same for A and B, you just map them to different characters.
	m_mapDecodeB.SetAt("11011001100", 0 ); // SP
	m_mapDecodeB.SetAt("11001101100", 1 ); // !
	m_mapDecodeB.SetAt("11001100110", 2 ); // "
	m_mapDecodeB.SetAt("10010011000", 3 ); // #
	m_mapDecodeB.SetAt("10010001100", 4 ); // $
	m_mapDecodeB.SetAt("10001001100", 5 ); // %
	m_mapDecodeB.SetAt("10011001000", 6 ); // &
	m_mapDecodeB.SetAt("10011000100", 7 ); // '
	m_mapDecodeB.SetAt("10001100100", 8 ); // (
	m_mapDecodeB.SetAt("11001001000", 9 ); // )
	m_mapDecodeB.SetAt("11001000100", 10 ); // *
	m_mapDecodeB.SetAt("11000100100", 11 ); // +
	m_mapDecodeB.SetAt("10110011100", 12 ); //, 
	m_mapDecodeB.SetAt("10011011100", 13 ); // -
	m_mapDecodeB.SetAt("10011001110", 14 ); // .
	m_mapDecodeB.SetAt("10111001100", 15 ); // /
	m_mapDecodeB.SetAt("10011101100", 16 ); // 0
	m_mapDecodeB.SetAt("10011100110", 17 ); // 1
	m_mapDecodeB.SetAt("11001110010", 18 ); // 2
	m_mapDecodeB.SetAt("11001011100", 19 ); // 3
	m_mapDecodeB.SetAt("11001001110", 20 ); // 4
	m_mapDecodeB.SetAt("11011100100", 21 ); // 5
	m_mapDecodeB.SetAt("11001110100", 22 ); // 6
	m_mapDecodeB.SetAt("11101101110", 23 ); // 7
	m_mapDecodeB.SetAt("11101001100", 24 ); // 8
	m_mapDecodeB.SetAt("11100101100", 25 ); // 9
	m_mapDecodeB.SetAt("11100100110", 26 ); // :
	m_mapDecodeB.SetAt("11101100100", 27 ); // ;
	m_mapDecodeB.SetAt("11100110100", 28 ); // <
	m_mapDecodeB.SetAt("11100110010", 29 ); // =
	m_mapDecodeB.SetAt("11011011000", 30 ); // >
	m_mapDecodeB.SetAt("11011000110", 31 ); // ?
	m_mapDecodeB.SetAt("11000110110", 32 ); // @
	m_mapDecodeB.SetAt("10100011000", 33 ); // A
	m_mapDecodeB.SetAt("10001011000", 34 ); // B
	m_mapDecodeB.SetAt("10001000110", 35 ); // C
	m_mapDecodeB.SetAt("10110001000", 36 ); // D
	m_mapDecodeB.SetAt("10001101000", 37 ); // E
	m_mapDecodeB.SetAt("10001100010", 38 ); // F
	m_mapDecodeB.SetAt("11010001000", 39 ); // G
	m_mapDecodeB.SetAt("11000101000", 40 ); // H
	m_mapDecodeB.SetAt("11000100010", 41 ); // I
	m_mapDecodeB.SetAt("10110111000", 42 ); // J
	m_mapDecodeB.SetAt("10110001110", 43 ); // K
	m_mapDecodeB.SetAt("10001101110", 44 ); // L
	m_mapDecodeB.SetAt("10111011000", 45 ); // M
	m_mapDecodeB.SetAt("10111000110", 46 ); // N
	m_mapDecodeB.SetAt("10001110110", 47 ); // O
	m_mapDecodeB.SetAt("11101110110", 48 ); // P
	m_mapDecodeB.SetAt("11010001110", 49 ); // Q
	m_mapDecodeB.SetAt("11000101110", 50 ); // R
	m_mapDecodeB.SetAt("11011101000", 51 ); // S
	m_mapDecodeB.SetAt("11011100010", 52 ); // T
	m_mapDecodeB.SetAt("11011101110", 53 ); // U
	m_mapDecodeB.SetAt("11101011000", 54 ); // V
	m_mapDecodeB.SetAt("11101000110", 55 ); // W
	m_mapDecodeB.SetAt("11100010110", 56 ); // X
	m_mapDecodeB.SetAt("11101101000", 57 ); // Y
	m_mapDecodeB.SetAt("11101100010", 58 ); // Z
	m_mapDecodeB.SetAt("11100011010", 59 ); // [
	m_mapDecodeB.SetAt("11101111010", 60 ); // '\'
	m_mapDecodeB.SetAt("11001000010", 61 ); // ]
	m_mapDecodeB.SetAt("11110001010", 62 ); // ^
	m_mapDecodeB.SetAt("10100110000", 63 ); // _
	m_mapDecodeB.SetAt("10100001100", 64 ); // `
	m_mapDecodeB.SetAt("10010110000", 65 ); // a
	m_mapDecodeB.SetAt("10010000110", 66 ); // b
	m_mapDecodeB.SetAt("10000101100", 67 ); // c
	m_mapDecodeB.SetAt("10000100110", 68 ); // d
	m_mapDecodeB.SetAt("10110010000", 69 ); // e
	m_mapDecodeB.SetAt("10110000100", 70 ); // f
	m_mapDecodeB.SetAt("10011010000", 71 ); // g
	m_mapDecodeB.SetAt("10011000010", 72 ); // h
	m_mapDecodeB.SetAt("10000110100", 73 ); // i
	m_mapDecodeB.SetAt("10000110010", 74 ); // j
	m_mapDecodeB.SetAt("11000010010", 75 ); // k
	m_mapDecodeB.SetAt("11001010000", 76 ); // l
	m_mapDecodeB.SetAt("11110111010", 77 ); // m
	m_mapDecodeB.SetAt("11000010100", 78 ); // n
	m_mapDecodeB.SetAt("10001111010", 79 ); // o
	m_mapDecodeB.SetAt("10100111100", 80 ); // p
	m_mapDecodeB.SetAt("10010111100", 81 ); // q
	m_mapDecodeB.SetAt("10010011110", 82 ); // r
	m_mapDecodeB.SetAt("10111100100", 83 ); // s
	m_mapDecodeB.SetAt("10011110100", 84 ); // t
	m_mapDecodeB.SetAt("10011110010", 85 ); // u
	m_mapDecodeB.SetAt("11110100100", 86 ); // v
	m_mapDecodeB.SetAt("11110010100", 87 ); // w
	m_mapDecodeB.SetAt("11110010010", 88 ); // x
	m_mapDecodeB.SetAt("11011011110", 89 ); // y
	m_mapDecodeB.SetAt("11011110110", 90 ); // z
	m_mapDecodeB.SetAt("11110110110", 91 ); // {
	m_mapDecodeB.SetAt("10101111000", 92 ); // |
	m_mapDecodeB.SetAt("10100011110", 93 ); // }
	m_mapDecodeB.SetAt("10001011110", 94 ); // ~
	m_mapDecodeB.SetAt("10111101000", 95 ); // DEL
	m_mapDecodeB.SetAt("10111100010", 96 ); // FNC3
	m_mapDecodeB.SetAt("11110101000", 97 ); // FNC2
	m_mapDecodeB.SetAt("11110100010", 98 ); // SHIFT
	m_mapDecodeB.SetAt("10111011110", 99 ); // Code C
	m_mapDecodeB.SetAt("10111101110", 100 ); // FNC4
	m_mapDecodeB.SetAt("11101011110", 101 ); // Code A
	m_mapDecodeB.SetAt("11110101110", 102 ); // FNC1
	m_mapDecodeB.SetAt("11010000100", 103 ); // START A
	m_mapDecodeB.SetAt("11010010000", 104 ); // START B
	m_mapDecodeB.SetAt("11010011100", 105 ); // START C
	m_mapDecodeB.SetAt("11000111010", 127); // STOP
	*/

	m_mapDecode.SetAt("11011001100", 0); // SP
	m_mapDecode.SetAt("11001101100", 1); // !
	m_mapDecode.SetAt("11001100110", 2); // "
	m_mapDecode.SetAt("10010011000", 3); // #
	m_mapDecode.SetAt("10010001100", 4); // $
	m_mapDecode.SetAt("10001001100", 5); // %
	m_mapDecode.SetAt("10011001000", 6); // &
	m_mapDecode.SetAt("10011000100", 7); // '
	m_mapDecode.SetAt("10001100100", 8); // (
	m_mapDecode.SetAt("11001001000", 9); // )
	m_mapDecode.SetAt("11001000100", 10); // *
	m_mapDecode.SetAt("11000100100", 11); // +
	m_mapDecode.SetAt("10110011100", 12); // ,
	m_mapDecode.SetAt("10011011100", 13); // -
	m_mapDecode.SetAt("10011001110", 14); // .
	m_mapDecode.SetAt("10111001100", 15); // /
	m_mapDecode.SetAt("10011101100", 16); // 0
	m_mapDecode.SetAt("10011100110", 17); // 1
	m_mapDecode.SetAt("11001110010", 18); // 2
	m_mapDecode.SetAt("11001011100", 19); // 3
	m_mapDecode.SetAt("11001001110", 20); // 4
	m_mapDecode.SetAt("11011100100", 21); // 5
	m_mapDecode.SetAt("11001110100", 22); // 6
	m_mapDecode.SetAt("11101101110", 23); // 7
	m_mapDecode.SetAt("11101001100", 24); // 8
	m_mapDecode.SetAt("11100101100", 25); // 9
	m_mapDecode.SetAt("11100100110", 26); // :
	m_mapDecode.SetAt("11101100100", 27); // ;
	m_mapDecode.SetAt("11100110100", 28); // <
	m_mapDecode.SetAt("11100110010", 29); // =
	m_mapDecode.SetAt("11011011000", 30); // >
	m_mapDecode.SetAt("11011000110", 31); // ?
	m_mapDecode.SetAt("11000110110", 32); // @
	m_mapDecode.SetAt("10100011000", 33); // A
	m_mapDecode.SetAt("10001011000", 34); // B
	m_mapDecode.SetAt("10001000110", 35); // C
	m_mapDecode.SetAt("10110001000", 36); // D
	m_mapDecode.SetAt("10001101000", 37); // E
	m_mapDecode.SetAt("10001100010", 38); // F
	m_mapDecode.SetAt("11010001000", 39); // G
	m_mapDecode.SetAt("11000101000", 40); // H
	m_mapDecode.SetAt("11000100010", 41); // I
	m_mapDecode.SetAt("10110111000", 42); // J
	m_mapDecode.SetAt("10110001110", 43); // K
	m_mapDecode.SetAt("10001101110", 44); // L
	m_mapDecode.SetAt("10111011000", 45); // M
	m_mapDecode.SetAt("10111000110", 46); // N
	m_mapDecode.SetAt("10001110110", 47); // O
	m_mapDecode.SetAt("11101110110", 48); // P
	m_mapDecode.SetAt("11010001110", 49); // Q
	m_mapDecode.SetAt("11000101110", 50); // R
	m_mapDecode.SetAt("11011101000", 51); // S
	m_mapDecode.SetAt("11011100010", 52); // T
	m_mapDecode.SetAt("11011101110", 53); // U
	m_mapDecode.SetAt("11101011000", 54); // V
	m_mapDecode.SetAt("11101000110", 55); // W
	m_mapDecode.SetAt("11100010110", 56); // X
	m_mapDecode.SetAt("11101101000", 57); // Y
	m_mapDecode.SetAt("11101100010", 58); // Z
	m_mapDecode.SetAt("11100011010", 59); // [
	m_mapDecode.SetAt("11101111010", 60); // '\'
	m_mapDecode.SetAt("11001000010", 61); // ]
	m_mapDecode.SetAt("11110001010", 62); // ^
	m_mapDecode.SetAt("10100110000", 63); // _
	m_mapDecode.SetAt("10100001100", 64); // NUL
	m_mapDecode.SetAt("10010110000", 65); // SOH
	m_mapDecode.SetAt("10010000110", 66); // STX
	m_mapDecode.SetAt("10000101100", 67); // ETX
	m_mapDecode.SetAt("10000100110", 68); // EOT
	m_mapDecode.SetAt("10110010000", 69); // ENQ
	m_mapDecode.SetAt("10110000100", 70); // ACK
	m_mapDecode.SetAt("10011010000", 71); // BEL
	m_mapDecode.SetAt("10011000010", 72); // BS
	m_mapDecode.SetAt("10000110100", 73); // HT
	m_mapDecode.SetAt("10000110010", 74); // LF
	m_mapDecode.SetAt("11000010010", 75); // VT
	m_mapDecode.SetAt("11001010000", 76); // FF
	m_mapDecode.SetAt("11110111010", 77); // CR
	m_mapDecode.SetAt("11000010100", 78); // SO
	m_mapDecode.SetAt("10001111010", 79); // SI
	m_mapDecode.SetAt("10100111100", 80); // DLE
	m_mapDecode.SetAt("10010111100", 81); // DC1
	m_mapDecode.SetAt("10010011110", 82); // DC2
	m_mapDecode.SetAt("10111100100", 83); // DC3
	m_mapDecode.SetAt("10011110100", 84); // DC4
	m_mapDecode.SetAt("10011110010", 85); // NAK
	m_mapDecode.SetAt("11110100100", 86); // SYN
	m_mapDecode.SetAt("11110010100", 87); // ETB
	m_mapDecode.SetAt("11110010010", 88); // CAN
	m_mapDecode.SetAt("11011011110", 89); // EM
	m_mapDecode.SetAt("11011110110", 90); // SUB
	m_mapDecode.SetAt("11110110110", 91); // ESC
	m_mapDecode.SetAt("10101111000", 92); // FS
	m_mapDecode.SetAt("10100011110", 93); // GS
	m_mapDecode.SetAt("10001011110", 94); // RS
	m_mapDecode.SetAt("10111101000", 95); // US
	m_mapDecode.SetAt("10111100010", 96); // FNC3
	m_mapDecode.SetAt("11110101000", 97); // FNC2
	m_mapDecode.SetAt("11110100010", 98); // SHIFT
	m_mapDecode.SetAt("10111011110", 99); // Code C
	m_mapDecode.SetAt("10111101110", 100); // Code B
	m_mapDecode.SetAt("11101011110", 101); // FNC4
	m_mapDecode.SetAt("11110101110", 102); // FNC1
	m_mapDecode.SetAt("11010000100", 103); // START A
	m_mapDecode.SetAt("11010010000", 104); // START B
	m_mapDecode.SetAt("11010011100", 105); // START C
	m_mapDecode.SetAt("11000111010" /*11*/, 127); // STOP (has an extra bar!)
}

CBarcodeDetector::~CBarcodeDetector()
{

}

void CBarcodeDetector::SetHwnd(HWND hwnd)
{
	m_hwndMessage = hwnd;
}

void CBarcodeDetector::SetLimit(long nLimit)
{
	m_nLimit = nLimit;
}

void CBarcodeDetector::SetEnabled(BOOL bEnable)
{
	m_bEnabled = bEnable;
}

void CBarcodeDetector::SetScanRegion(EScanRegion scanRegion, double dPercent, long nFrequency)
{
	m_ScanRegion = scanRegion;
	m_dScanPercent = dPercent;
	m_nScanFreq = nFrequency;
}

// (a.walling 2008-09-03 14:16) - PLID 22821 - Use a gdiplus bitmap
// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
BOOL CBarcodeDetector::FindBarcode(CxImage& cxImage, OUT CStringArray& saResults)
{	
#ifdef DEBUG_SCANENTIREIMAGE
	m_ScanRegion = eScanEntireImage;
	m_dScanPercent = 1.0f;
#endif

#ifdef DEBUG_NOBARCODELIMIT
	m_nLimit = 0;
#endif

	{
		// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
		DWORD nWidth = cxImage.GetWidth();
		DWORD nHeight = cxImage.GetHeight();
		
		// (a.walling 2008-07-31 13:02) - PLID 7228 - Ensure the step is at minimum 3
		long nVerticalStep = max((nHeight / m_nScanFreq), 3); // scan

		DWORD nVerticalBegin = 0;
		DWORD nVerticalStop = 0;
		if (m_ScanRegion == eScanTop || m_ScanRegion == eScanEntireImage) {
			// keep at zero
			nVerticalStop = DWORD(nHeight * m_dScanPercent);
		} else if (m_ScanRegion == eScanBottom) {
			nVerticalBegin = nHeight - 1; // zero-based
			nVerticalStep *= -1; // scan upwards

			nVerticalStop = DWORD(nHeight - (nHeight * m_dScanPercent));
		}

		BYTE* pLine = new BYTE[nWidth];
		
		// (a.walling 2010-04-08 08:44) - PLID 38170 - Use CxImage again
		//Gdiplus::Color c, c2;

		RGBQUAD c, c2;
		::ZeroMemory(&c, sizeof(RGBQUAD));
		::ZeroMemory(&c2, sizeof(RGBQUAD));

		for (m_y = nVerticalBegin; (m_ScanRegion == eScanBottom) ? m_y > nHeight : m_y < nHeight; m_y += nVerticalStep) {

			BYTE nMax = 0;
			BYTE nMin = 255;
			UINT x = 0;
			for (x = 0; x < nWidth; x++) {

				// (a.walling 2010-04-08 08:44) - PLID 38170 - Use CxImage again
				c = cxImage.GetPixelColor(x, m_y, false);
				//pBitmap->GetPixel(x, m_y, &c);
#ifdef AVERAGE_2PIXELHEIGHT
				if (m_y + 1 < nHeight) {
					c2 = cxImage.GetPixelColor(x, m_y, false);
					//pBitmap->GetPixel(x, m_y + 1, &c2);
				} else {
					c2 = c;
				}

				/*
				pLine[x] = BYTE(
					( ( ((float(c.rgbRed) * 0.299f) + (float(c.rgbGreen) * 0.587f) + (float(c.rgbBlue) * 0.114f)) +
						((float(c2.rgbRed) * 0.299f) + (float(c2.rgbGreen) * 0.587f) + (float(c2.rgbBlue) * 0.114f)) )
					/ 2.0f ) + 0.5f);
				*/
				
				pLine[x] = BYTE(
					( ( ((float(c.rgbRed) * 0.299f) + (float(c.rgbGreen) * 0.587f) + (float(c.rgbBlue) * 0.114f)) +
						((float(c2.rgbRed) * 0.299f) + (float(c2.rgbGreen * 0.587f) + (float(c2.rgbBlue * 0.114f)) )
					/ 2.0f ) + 0.5f);
#else
				pLine[x] = BYTE(
					((float(c.rgbRed) * 0.299f) + (float(c.rgbGreen) * 0.587f) + (float(c.rgbBlue) * 0.114f)) + 0.5f);
#endif

				if (pLine[x] > nMax) {
					nMax = pLine[x];
				}

				if (pLine[x] < nMin) {
					nMin = pLine[x];
				}

				/*
				// cut off the 5% range at either edge of the luminosity spectrum
				double dCutoff = 0.05f;
				if (pLuminosityLine[x] < dCutoff) pLuminosityLine[x] = 0.0f;
				if (pLuminosityLine[x] > (255.0f - dCutoff)) pLuminosityLine[x] = 255.0f;
				*/
			}


			CString strResult;
			CStringArray saScanLineResults;
#ifndef DEPRECATE_OLD_BARCODEDETECT_METHODS
			BOOL bFound = ScanLine(pLine, nWidth, saScanLineResults);
#else
			BOOL bFound = ScanLine_V3(pLine, nWidth, nMax, nMin, saScanLineResults);
#endif

			if (bFound) {
				for (int iScanResult = 0; iScanResult < saScanLineResults.GetSize(); iScanResult++) {
					CString strResult = saScanLineResults[iScanResult];
					TRACE("Line %li: \tBarcode data detected: '%s'\n", m_y, strResult);

					BOOL bExists = FALSE;
					for (int iNewResult = 0; iNewResult < saResults.GetSize(); iNewResult++) {
						if (saResults[iNewResult] == strResult) {
							bExists = TRUE;
							break;
						}
					}

					if (!bExists) {
						BarcodeLog("Barcode data detected: '%s'\n", strResult);
						saResults.Add(strResult);
					}
				}

				if (m_nLimit != 0 && saResults.GetSize() >= m_nLimit) {
					break;
				}
			}			
		}

		delete[] pLine;
		pLine = NULL;
	}

	return saResults.GetSize() > 0;
}

#pragma region Deprecated barcode detection methods
#ifndef DEPRECATE_OLD_BARCODEDETECT_METHODS


// (a.walling 2009-03-05 08:58) - PLID 33363
int CBarcodeDetector::CompareDouble(const void *pDataA, const void *pDataB)
{
	const double* a = (const double*)(pDataA);
	const double* b = (const double*)(pDataB);

	if (*a < *b)
		return -1;
	if (*a > *b)
		return 1;
	if (*a == *b)
		return 0;
	else
		return 0;
}

BOOL CBarcodeDetector::ScanLine(BYTE* pLine, long nWidth, OUT CStringArray& saResults)
{
	CArray<CZone*, CZone*> arZones;

	long nQuietToleranceWidth = nWidth / 16; // 1/16th of the width
	long nQuietWidth = 0;
	long nQuietStart = -1;
	BOOL bInQuietZone = FALSE;
	for (int i = 0; i < nWidth + 1; i++) { // goes one over!
		BOOL bIsBlack = FALSE;
		if (i >= nWidth)
			bIsBlack = TRUE;
		else bIsBlack = pLine[i] < m_bLuminanceThreshold;

		if (bIsBlack && bInQuietZone) {
			// quiet zone ends!
			if (nQuietWidth > nQuietToleranceWidth) {
				CZone* pZone = new CZone;
				pZone->nWidth = nQuietWidth;
				pZone->nBegin = nQuietStart;

				arZones.Add(pZone);
			}

			nQuietWidth = 0;
			nQuietStart = -1;
			bInQuietZone = FALSE;			
		} else if (!bIsBlack && !bInQuietZone) {
			bInQuietZone = TRUE;
			nQuietStart = i;
			nQuietWidth++;
		} else if (!bIsBlack && bInQuietZone) {
			nQuietWidth++;
		} else if (bIsBlack && !bInQuietZone) {
			// do nothing
		}
	}

	// now we have a list of quiet zones. scan them!
	for (int f = 0; f < arZones.GetSize() - 1; f++) {
		CZone* pZone = arZones[f];
		CZone* pNextZone = arZones[f+1];
		BYTE* pSmallerLine = pLine + pZone->nBegin + pZone->nWidth - nQuietToleranceWidth;
		long nSmallerWidth = nQuietToleranceWidth + ((pNextZone->nBegin) - (pZone->nBegin + pZone->nWidth)) + nQuietToleranceWidth;

		CString strResult;
		if (ScanSegment(pSmallerLine, nSmallerWidth, strResult)) {
			saResults.Add(strResult);
		} else if (strResult.GetLength()) {
			BarcodeLog("Found barcode %s but parity was invalid\n", strResult);
		}
	}

	for (int final = 0; final < arZones.GetSize(); final++) {
		delete arZones[final];
	}

	return saResults.GetSize() > 0;
}

BOOL CBarcodeDetector::ScanSegment(BYTE* pLine, long nWidth, OUT CString& strResult)
{
	CArray<CBar*, CBar*> arBars;

	/*for (int u = 0; u < nWidth; u++) {
		BarcodeLog("%02x.", pLine[u]);
	}
	BarcodeLog("\n");*/

	BOOL bIsBlack = FALSE;
	CBar* pBar = NULL;
	for (int iBar = 0; iBar < nWidth; iBar++) {
		if (pBar) {
			pBar->pNextBar = new CBar;
			pBar = pBar->pNextBar;
		} else {
			pBar = new CBar;
		}

		// get contiguous sections
		pBar->bIsBlack = pLine[iBar] < m_bLuminanceThreshold;

		if (pBar->bIsBlack) {
			if (iBar != 0) {
				pBar->dBegin = (iBar - 1) + (double(pLine[iBar - 1]) / 255);
			} else {
				pBar->dBegin = iBar;
			}
		} else {
			if (iBar != 0) {
				pBar->dBegin = (iBar - 1) + (double(255 - pLine[iBar - 1]) / 255);
			} else {
				pBar->dBegin = iBar;
			}
		}

		for (int k = iBar + 1; k < nWidth; k++) {
			BOOL bIsPixelBlack = pLine[k] < m_bLuminanceThreshold;
			if (bIsPixelBlack != pBar->bIsBlack) {
				// end
				if (!pBar->bIsBlack) {
					pBar->dEnd = k + (double(pLine[k]) / 255);
				} else {
					pBar->dEnd = k + (double(255 - pLine[k]) / 255);
				}

				break;
			}
		}
		if (k == nWidth) {
			pBar->dEnd = k;
		}

		iBar = k - 1;


		arBars.Add(pBar);
	}

	// (a.walling 2009-03-05 09:03) - PLID 33363 - Cull out the quiet zones at either end.
	if (arBars.GetSize() > 0) {
		CBar* pBar = arBars[0];
		if (!pBar->bIsBlack) {
			delete pBar;
			arBars.RemoveAt(0);
		}

		if (arBars.GetSize() > 0) {
			pBar = arBars[arBars.GetSize() - 1];
			if (!pBar->bIsBlack) {
				delete pBar;
				arBars.RemoveAt(arBars.GetSize() - 1);
				if (arBars.GetSize() > 0) {
					arBars[arBars.GetSize() - 1]->pNextBar = NULL;
				}
			}
		}
	}

	BOOL bFound = FALSE;

	if (arBars.GetSize() > 0) {
		bFound = ScanSegment_NewMethod(arBars, strResult);

		if (!bFound) {
			bFound = ScanSegment_OldMethod(arBars, strResult);
		}
	}

	for (int f = 0; f < arBars.GetSize(); f++) {
		CBar* pBar = arBars[f];
		delete pBar;
	}

	return bFound;
}

// (a.walling 2009-03-05 09:06) - PLID 33363 - New method for detecting barcodes
// (a.walling 2009-04-28 09:05) - PLID 33363 - Superseded by V3
BOOL CBarcodeDetector::ScanSegment_NewMethod(CArray<CBar*, CBar*>& arBars, OUT CString& strResult)
{
	if (arBars.GetSize() == 0)
		return FALSE;

	// let's get all the sizes
	CArray<double, double> arSizes;
	double dMinWidth = 1.e+38;
	int i = 0;

	for (i = 0; i < arBars.GetSize(); i++) {
		double dCurrentWidth = arBars[i]->GetWidth();

		arSizes.Add(dCurrentWidth);

		// also get the minimum width
		if (dCurrentWidth < dMinWidth) {
			dMinWidth = dCurrentWidth;
		}
	}

	// now sort our size array
	qsort(arSizes.GetData(), arSizes.GetSize(), sizeof(double), CBarcodeDetector::CompareDouble);

	// and create an array of deviations
	CArray<double, double> arDeviations;
	// arSizes is guaranteed to be > 0
	
	double dCutoffMin = 1.e+38;
	double dCutoffMax = 0;

	double dPreviousSize = arSizes[0];
	for (i = 0; i < arSizes.GetSize(); i++) {
		double dDeviation = arSizes[i] - dPreviousSize; // should always be >= 0 since sorted
		arDeviations.Add(dDeviation);
		dPreviousSize = arSizes[i];

		if (dDeviation > dCutoffMax)
			dCutoffMax = dDeviation;
		if (dDeviation < dCutoffMin)
			dCutoffMin = dDeviation;
	}

	// now we want to separate these sizes into 4 groups, 1-4
	double dCutoff = dMinWidth / 2; // initially start at the minimum width / 2. This works well in most cases.

	CMap<double, double, long, long> mapSizeToWidth;

	BOOL bKeepSearching = TRUE;
	long nIterations = 0;
	const long cMaxIterations = 100;
	while (bKeepSearching) {
		nIterations++;
		mapSizeToWidth.RemoveAll();

		long nCurrentWidth = 1;
		
		for (i = 0; i < arSizes.GetSize(); i++) {
			if (arDeviations[i] > dCutoff) {
				nCurrentWidth++;
			}

			mapSizeToWidth.SetAt(arSizes[i], nCurrentWidth);
		}

		if (nCurrentWidth == 4) {
			// good, 4 groups.
			bKeepSearching = FALSE;
		} else {
			if (nIterations > cMaxIterations || dCutoff < 0.5 || dCutoff > 500 || FLOAT_EQ(dCutoffMax, dCutoffMin) || FLOAT_EQ(dCutoff, dCutoffMin) || FLOAT_EQ(dCutoff, dCutoffMax)) {
				// can't find a good cutoff to group; so exit.
				return FALSE;
			}

			if (nCurrentWidth > 4) {
				// too many groups. increase the cutoff.
				dCutoffMin = dCutoff;
				dCutoff = (dCutoff + dCutoffMax) / 2;
			} else {
				// too few groups. decrease the cutoff.
				dCutoffMax = dCutoff;
				dCutoff = (dCutoff + dCutoffMin) / 2;
			}
		}
	}

	// now we have a map of size to width. Let's generate our barcode data string and start parsing.
	CString strBarcodeData;
	for (i = 0; i < arBars.GetSize(); i++) {
		CBar* pBar = arBars[i];

		long nBarWidth = mapSizeToWidth[pBar->GetWidth()];

		for (int j = 0; j < nBarWidth; j++) {
			strBarcodeData += (pBar->bIsBlack) ? "1" : "0";
		}
	}

	BarcodeLog("Barcode data: %s\n", strBarcodeData);

	{
		CString strSegment;
		CByteArray arBytes;

		BOOL bStop = FALSE;
		for (i = 0; i < strBarcodeData.GetLength() && !bStop; i++) {
			if ((i > 0) && ( i % 11 == 0)) { // 11 chars per segment
				// parse this segment, and start a new one.

				DWORD dwValue = 0;
				if (m_mapDecode.Lookup(strSegment, dwValue)) {
					if (dwValue == 127) {
						bStop = TRUE;
					} else {
						arBytes.Add((BYTE)dwValue);
					}
				} else {
					// bad segment
					BarcodeLog("Bad segment: %s\n", strSegment);
					return FALSE;
				}

				strSegment.Empty();
			}

			strSegment += strBarcodeData[i];
		}

		// now we have our bytes; let's decode and checksum
		DWORD nCurrentMode = 0;
		DWORD dwRunningChecksum = 0;
		CString strDecodedBarcode;
		for (i = 0; i < arBytes.GetSize(); i++) {
			DWORD dwValue = (DWORD)arBytes[i];

			if (i < arBytes.GetSize() - 1) {
				switch(dwValue) {
					case START_A:
						if (i == 0) {
							// the start variant is always added to the checksum when first,
							// if not first it is multiplied by the position like all others
							dwRunningChecksum = START_A;
						} else {
							dwRunningChecksum += (i * dwValue);
						}
						nCurrentMode = START_A;
						break;
					case START_B:
						if (i == 0) {
							// the start variant is always added to the checksum when first,
							// if not first it is multiplied by the position like all others
							dwRunningChecksum = START_B;
						} else {
							dwRunningChecksum += (i * dwValue);
						}
						nCurrentMode = START_B;
						break;
					case START_C:
						BarcodeLog("START_C detected; unsupported.\n");
						// unsupported
						return FALSE;
						break;
					default:
						{
							dwRunningChecksum += (i * dwValue);

							char cNewChar = 0;
							if (nCurrentMode == START_B) {
								cNewChar = char(dwValue + 32);
							} else if (nCurrentMode == START_A) {
								if (dwValue < 64) {
									cNewChar = char(dwValue + 32);
								} else {
									cNewChar = char(dwValue - 64);
								}
							} else {
								ASSERT(FALSE); // no current mode!
								BarcodeLog("No current mode is set!\n");
								return FALSE;
							}

							if (cNewChar == 0) {
								BarcodeLog("\tEmbedded NUL characters are unsupported!\n");
								cNewChar = ' '; // replace with a space, why not.
							}
							
							strDecodedBarcode += cNewChar;
						}

						break;
				}
			} else {
				// this is the checksum character
				DWORD dwChecksum = dwRunningChecksum % 103;

				if (dwValue == dwChecksum) {
					// checksum OK, we are good!
					strResult = strDecodedBarcode;
					return TRUE;
				} else {
					// checksum failed
					BarcodeLog("Checksum %lu does not match check digit %lu!\n", dwChecksum, dwValue);
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CBarcodeDetector::ScanSegment_OldMethod(CArray<CBar*, CBar*>& arBars, OUT CString& strResult)
{
	BOOL bFoundBarcode = FALSE;
	CString strDecode;

	if (arBars.GetSize() > 0) {
		double dMinWidth = 1.e+38;
		for (int j = 0; j < arBars.GetSize(); j++) {
			CBar* pBar = arBars[j];

			if (m_bIncludeSpacesForMinimumWidthDetection) {
				if (pBar->dEnd - pBar->dBegin < dMinWidth) {
					dMinWidth = pBar->dEnd - pBar->dBegin;
				}
			} else {
				if (pBar->bIsBlack && pBar->dEnd - pBar->dBegin < dMinWidth) {
					dMinWidth = pBar->dEnd - pBar->dBegin;
				}
			}
		}

		BarcodeLog("%li bars, minimum width %g\n", arBars.GetSize(), dMinWidth);

		/*
		for (j = 0; j < arBars.GetSize(); j++) {
			CBar* pBar = arBars[j];
			BarcodeLog("Bar %li:\tBlack %li, Start %g, End %g, Width %g, Adj %g, FinAdj %li\n", j, pBar->bIsBlack, pBar->dBegin, pBar->dEnd, pBar->dEnd - pBar->dBegin, (pBar->dEnd - pBar->dBegin) / dMinWidth, int(((pBar->dEnd - pBar->dBegin) / dMinWidth) + 0.5f));
		}
		*/


		CString strEntireCode;
		CString strFloorCode;
		for (int s = 0; s < arBars.GetSize(); s++) {		
			CBar* pBar = arBars[s];

			double dAdjWidth = (pBar->dEnd - pBar->dBegin) / dMinWidth;
			for (int i = 0; i < (int)(dAdjWidth + 0.5f); i++) {
				strEntireCode += (pBar->bIsBlack) ? "1" : "0";
			}
			for (i = 0; i < floor(dAdjWidth); i++) {
				strFloorCode += (pBar->bIsBlack) ? "1" : "0";
			}
		}

		BarcodeLog("Preliminary Barcode Data: %s\n", strEntireCode);
		BarcodeLog("Preliminary Floor Data: %s\n", strFloorCode);

		// START-B = 11010010000
		// STOP = 11000111010 + 11

		CBar* pStartBar = NULL;
		//for (int i = 0; i < arBars.GetSize(); i++) {
		{
			// all codes are in groups of 6 bars

			long nWhite = 0;
			long nBlack = 0;

			BYTE nCurrentMode = 0;
			BOOL bFloor = FALSE;

			long nIxStart = strEntireCode.Find(START_A_STR, 0);
			if (nIxStart != -1) {
				nCurrentMode = START_A;
				BarcodeLog("Detected START_A\n");
			} else {
				nIxStart = strEntireCode.Find(START_B_STR, 0);
				if (nIxStart != -1) {
					nCurrentMode = START_B;
					BarcodeLog("Detected START_B\n");
				} else {
					nCurrentMode = 0;
				}
			}

			if (nCurrentMode == 0) {
				nIxStart = strFloorCode.Find(START_A_STR, 0);
				if (nIxStart != -1) {
					nCurrentMode = START_A;
					bFloor = TRUE;
					BarcodeLog("Floor Detected START_A\n");
				} else {
					nIxStart = strFloorCode.Find(START_B_STR, 0);
					if (nIxStart != -1) {
						nCurrentMode = START_B;
						bFloor = TRUE;
						BarcodeLog("Floor Detected START_B\n");
					} else {
						nCurrentMode = 0;
					}
				}
			}

			/*
			CString strChar = GetCharFromBarPos(arBars, i, dMinWidth, nWhite, nBlack);

			DWORD dwStartValue;
			if (m_mapDecode.Lookup(strChar, dwStartValue)) {
				if (dwStartValue == START_A) {
				} else if (dwStartValue == START_B) {
					nCurrentMode = START_B;
					BarcodeLog("Detected START_B\n");
				} else if (dwStartValue == START_C) {
					nCurrentMode = START_C;
					BarcodeLog("Detected START_C\n");
					// wait, we don't support C at this time.
					nCurrentMode = 0;
				}
			}
			*/

			if (nCurrentMode != 0) {
				long nCurrentPositionInString = 0;
				int i = -1;
				for (int iStringFind = 0; iStringFind < arBars.GetSize() && nCurrentPositionInString < nIxStart; iStringFind++) {
					CBar* pBar = arBars[iStringFind];
					if (bFloor) {
						nCurrentPositionInString += int(floor((pBar->dEnd - pBar->dBegin) / dMinWidth));
					} else {
						nCurrentPositionInString += int(((pBar->dEnd - pBar->dBegin) / dMinWidth) + 0.5f);
					}

					if (nCurrentPositionInString == nIxStart) {
						i = iStringFind + 1;
					}
				}

				if (i != -1) {
					CString strChar;

					if (m_bEnableBarcodeLogging) {
						CString str;
						for (int s = i; s < arBars.GetSize(); s++) {		
							CBar* pBar = arBars[s];

							if ((s - i) % 6 == 0) {
								str += ".";
							}

							double dAdjWidth = (pBar->dEnd - pBar->dBegin) / dMinWidth;
							if (bFloor) {
								for (int t = 0; t < (int)floor(dAdjWidth); t++) {
									str += (pBar->bIsBlack) ? "1" : "0";
								}
							} else {
								for (int t = 0; t < (int)(dAdjWidth + 0.5f); t++) {
									str += (pBar->bIsBlack) ? "1" : "0";
								}
							}
						}

						BarcodeLog("Barcode Data: %s\n", str);
					}

					DWORD dwRunningChecksum = nCurrentMode; // start off with first character, the START_x character.
					DWORD dwPos = 0;
					DWORD dwLastValue = 0;
					DWORD dwInvalidCount = 0;

					for (int t = i+6; t < arBars.GetSize(); t+=6) {
						nWhite = 0;
						nBlack = 0;
						strChar = GetCharFromBarPos(arBars, t, dMinWidth, nWhite, nBlack);
						
						BarcodeLog("Parse character code %s...\n", strChar);

						BOOL bValid = TRUE;
						if (nBlack + nWhite != 11) {
							BarcodeLog("\tBad number of bars: %li (%li + %li)\n", nBlack + nWhite, nBlack, nWhite);
							bValid = FALSE;
						}
						if (nBlack % 2 == 1) {
							BarcodeLog("\tEven number of black bars! %li\n", nBlack);
							bValid = FALSE;
						}
						if (nWhite % 2 == 0) {
							BarcodeLog("\tOdd number of spaces! %li\n", nWhite);
							bValid = FALSE;
						}

						if (!bValid && (dwInvalidCount <= m_nInvalidCharacterHealingLimit) && (nBlack + nWhite <= 13) && (nBlack + nWhite >= 9)) {
							dwInvalidCount++;
							if (m_bEnableBarcodeLogging) {
								for (int nCheckBars = t; nCheckBars < arBars.GetSize() && nCheckBars - t < 6; nCheckBars++) {
									CBar* pBar = arBars[nCheckBars];
									
									long nDummy;
									CString strOutput = pBar->GetAsString(dMinWidth, nDummy);
									int nActualWidth = int(((pBar->dEnd - pBar->dBegin) / dMinWidth) + 0.5f);
									BarcodeLog("\tBar %s (%s) using minwidth %g 0x%08x:\n", strOutput, pBar->bIsBlack ? "Black" : "White", dMinWidth, this);
									BarcodeLog("\t\tdBegin:\t%g\n", pBar->dBegin);
									BarcodeLog("\t\tdEnd:\t%g\n", pBar->dEnd);
									BarcodeLog("\t\tWidth:\t%g\n", pBar->dEnd - pBar->dBegin);
									BarcodeLog("\t\tAdjustedWidth:\t%g\n", (pBar->dEnd - pBar->dBegin) / dMinWidth);
									BarcodeLog("\t\tActualWidth:\t%li\n", nActualWidth);
									BarcodeLog("\t\tRounded Diff:\t%g\n", ((pBar->dEnd - pBar->dBegin) / dMinWidth) - double(nActualWidth));
									BarcodeLog("\n");
								}
							}

							// self-heal if we can.
							double dMinMaxDifference = 0;
							long nMinMaxDifferenceIndex = -1;
							for (int nHeal = t; nHeal < arBars.GetSize() && nHeal - t < 6; nHeal++) {
								CBar* pHealBar = arBars[nHeal];
								double dAdjustedWidth = (pHealBar->dEnd - pHealBar->dBegin) / dMinWidth;
								int nActualWidth = int(dAdjustedWidth + 0.5f);
								double dDifference = dAdjustedWidth - double(nActualWidth);

								if (nBlack + nWhite > 11) {
									// too wide
									if (dDifference < dMinMaxDifference) {
										dMinMaxDifference = dDifference;
										nMinMaxDifferenceIndex = nHeal;
									}
								} else {
									// too narrow
									if (dDifference > dMinMaxDifference) {
										dMinMaxDifference = dDifference;
										nMinMaxDifferenceIndex = nHeal;
									}
								}
							}

							if (nMinMaxDifferenceIndex != -1) {
								if (fabs(dMinMaxDifference) > m_cdDifferenceTolerance) {
									CBar* pHealBar = arBars[nMinMaxDifferenceIndex];
									
									if (nBlack + nWhite > 11) {
										pHealBar->nHeal = -1;
									} else {
										pHealBar->nHeal = 1;
									}
								}
								
								// now let us check again
								nWhite = 0;
								nBlack = 0;
								
								strChar = GetCharFromBarPos(arBars, t, dMinWidth, nWhite, nBlack);
								
								BarcodeLog("Parse healed character code %s...\n", strChar);

								BOOL bHealedValid = TRUE;
								if (nBlack + nWhite != 11) {
									BarcodeLog("\tBad number of bars: %li (%li + %li)\n", nBlack + nWhite, nBlack, nWhite);
									bHealedValid = FALSE;
								}
								if (nBlack % 2 == 1) {
									BarcodeLog("\tEven number of black bars! %li\n", nBlack);
									bHealedValid = FALSE;
								}
								if (nWhite % 2 == 0) {
									BarcodeLog("\tOdd number of spaces! %li\n", nWhite);
									bHealedValid = FALSE;
								}

								if (bHealedValid) {
									BarcodeLog("\tHealed character appears to be valid\n");
								} else {
									for (int nHeal = t; nHeal < arBars.GetSize() && nHeal - t < 6; nHeal++) {
										CBar* pHealBar = arBars[nHeal];
										double dAdjustedWidth = (pHealBar->dEnd - pHealBar->dBegin) / dMinWidth;
										int nActualWidth = int(dAdjustedWidth + 0.5f);
										double dDifference = dAdjustedWidth - double(nActualWidth);

										if (nBlack + nWhite > 11) {
											// too wide
											if (dDifference < -m_cdDifferenceTolerance) {
												pHealBar->nHeal = -1;
											}
										} else {
											// too narrow
											if (dDifference > m_cdDifferenceTolerance) {
												pHealBar->nHeal = 1;
											}
										}
									}

									nWhite = 0;
									nBlack = 0;
									
									strChar = GetCharFromBarPos(arBars, t, dMinWidth, nWhite, nBlack);
									
									BarcodeLog("2nd pass parse healed character code %s...\n", strChar);

									BOOL bHealedValid = TRUE;
									if (nBlack + nWhite != 11) {
										BarcodeLog("\tBad number of bars: %li (%li + %li)\n", nBlack + nWhite, nBlack, nWhite);
										bHealedValid = FALSE;
									}
									if (nBlack % 2 == 1) {
										BarcodeLog("\tEven number of black bars! %li\n", nBlack);
										bHealedValid = FALSE;
									}
									if (nWhite % 2 == 0) {
										BarcodeLog("\tOdd number of spaces! %li\n", nWhite);
										bHealedValid = FALSE;
									}

									if (bHealedValid) {
										BarcodeLog("\t2nd pass healed character appears to be valid\n");
									}
								}
							}
						}

						DWORD dwValue = 0;

						if (m_mapDecode.Lookup(strChar, dwValue)) {
							if (dwValue == 127) { // STOP
								dwRunningChecksum -= (dwPos * dwLastValue); // remove the last value from the checksum!!
								DWORD dwFinalChecksum = dwRunningChecksum % 103;
								BarcodeLog("Stop character found! Final checksum value: %li\n", dwFinalChecksum);

								if (dwFinalChecksum != dwLastValue) {
									BarcodeLog("Final checksum (%li [%li]) != checksum character (%li)\n", dwFinalChecksum, dwRunningChecksum, dwLastValue);
									strDecode = strDecode.Left(strDecode.GetLength() - 1);
								} else {
									strDecode = strDecode.Left(strDecode.GetLength() - 1);
									bFoundBarcode = TRUE;
								}

								break;
							} else {
								dwPos++;
								dwRunningChecksum += (dwPos * dwValue);
								dwLastValue = dwValue;

								// switch to different symbologies if needed
								if (dwValue == START_A) {
									BarcodeLog("Switch to START_A symbology\n");
									nCurrentMode = START_A;
								} else if (dwValue == START_B) {
									BarcodeLog("Switch to START_B symbology\n");
									nCurrentMode = START_B;
								} else if (dwValue == START_C) {
									BarcodeLog("Switch to START_C symbology\n");
									nCurrentMode = START_C;
									ASSERT(FALSE);
								} else {
									char cNewChar = 0;
									if (nCurrentMode == START_B) {
										cNewChar = char(dwValue + 32);
									} else if (nCurrentMode == START_A) {
										if (dwValue < 64) {
											cNewChar = char(dwValue + 32);
										} else {
											cNewChar = char(dwValue - 64);
										}
									}

									if (cNewChar == 0) {
										BarcodeLog("\tEmbedded NUL characters are unsupported!\n");
										cNewChar = ' '; // replace with a space, why not.
									}
									
									strDecode += cNewChar;

									BarcodeLog("\t%s (char '%s' added value %li * %li (%li) = %li)\n", strDecode, CString(cNewChar), dwValue, dwPos, dwPos * dwValue, dwRunningChecksum);
								}
							}
						} else {
							BarcodeLog("Bad pattern %s\n", strChar);
							strDecode += "{???}";
							BarcodeLog("\t%s\n", strDecode);
							dwPos++;
						}
					}
				}

				//break;
			}
		};

		BarcodeLog("Decoded string: %s\n", strDecode);
	}

	strResult = strDecode;
	return bFoundBarcode;
}

CString CBarcodeDetector::GetCharFromBarPos(const CArray<CBar*, CBar*>& arBars, long nIndex, const double& dMinWidth, long &nWhite, long &nBlack) 
{
	CString strChar;
	for (int b = 0; b < 6; b++) {
		if ((nIndex + b) < arBars.GetSize()) {
			CBar* pBar = arBars[nIndex+b];

			if (pBar->bIsBlack) {
				strChar += pBar->GetAsString(dMinWidth, nBlack);
			} else {
				strChar += pBar->GetAsString(dMinWidth, nWhite);
			}
		} else {
			BarcodeLog("Out of bars! %li / %li\n", nIndex+b, arBars.GetSize());
			break;
		}
	}

	return strChar;
}

#endif

#pragma endregion

void CBarcodeDetector::BarcodeLog(LPCTSTR str, ...)
{
	if (!m_bEnableBarcodeLogging)
		return;

	va_list args;
	va_start(args, str);
	CString strFormatted;
	strFormatted.FormatV(str, args);
	va_end(args);

	if (strFormatted.GetLength() < 500) {
		TRACE("%s", strFormatted);
	} else {
		TRACE("%s", strFormatted.Left(500));
	}

	CStdioFile f;
	if (f.Open("ScanBarcodeDetectLog.txt", CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareCompat)) {
		f.SeekToEnd();
		f.WriteString(strFormatted);
	}
}


// (a.walling 2009-04-28 09:08) - PLID 33363 - Barcode detection, version 3
/***************************************************************/
// Begin barcode detection version 3!
//
// This method is greatly improved in both speed and accuracy.
//
// Several shortcomings of the previous methods were addressed. Specifically,
// I modeled this as an actual, hardware barcode scanner would work. The
// previous approaches relied too much on gathering lots of data at once.
// This one works as follows:
//
// Preprocessing simply takes a line of RGB values, converts to luminosity,
// then compresses the range of values to floats between -1 and 1, where
// -1 is black and 1 is white.
//
// First off, we find a quiet zone, which is an area of low noise that is
// at least 1/16th of the page width. We calculate this by creating a
// 'window' of that size and summing the luminosity values within. If it
// drops below a certain range, we know we have noise.
//
// Once we have a quiet zone, we begin the scan, which goes one character
// (or 6 bars) at a time. The character recognition is the big departure
// from previous methods. We used to try to figure out all characters at
// once, which led to most of the problems. This just does one at a time.
// The classical method, finding the minimum width and figuring out the
// bars' multiples based on that, is still used, along with variations of
// a new approach. Since we know that each character is comprised of 6
// bars, with 11 bits each, then we can simply divide our processing range
// by 11, and calculate out the on/off state of those 11 segments individually,
// which leaves us with a character value. One approach attempts to use
// fractional values, which is useful for low quality images. Another
// attempt just uses a naive whole number approach, which works out very
// well for higher resolution images. Finally, a third 'compromise'
// approach is, per its name, a compromise of the fractional and whole
// approaches, with a bias towards 1.
/***************************************************************/

BOOL CBarcodeDetector::ScanLine_V3(BYTE* pLine, long nWidth, BYTE nMax, BYTE nMin, OUT CStringArray& saResults)
{
	long nFound = 0;

	float* pLinePoints = new float[nWidth];

	if (nMax == 0xFF && nMin == 0x00) {
		for (int i = 0; i < nWidth; i++) {
			pLinePoints[i] = pLine[i];

			pLinePoints[i] -= 128;

			if (pLinePoints[i] > 0) {
				pLinePoints[i] /= 127.0f;
			} else {
				pLinePoints[i] /= 128.8f;
			}
		}
	} else {
		BYTE nRange = nMax - nMin;
		if (nRange < 20) {
			// (a.walling 2010-03-25 19:51) - PLID 36812 - Must free memory! Can easily leak hundreds of megabytes with massive scanning.
			delete[] pLinePoints;
			return FALSE;
		}

		BYTE nPivot = BYTE((float(nRange) / 2) + 0.5f);

		for (int i = 0; i < nWidth; i++) {
			pLinePoints[i] = pLine[i];

			pLinePoints[i] -= nMin;
			pLinePoints[i] -= nPivot;

			if (pLinePoints[i] > 0) {
				pLinePoints[i] /= (nRange - nPivot);
			} else {
				pLinePoints[i] /= nPivot;
			}
		}
	}


	/*
	{
		Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(nWidth, 5, PixelFormat32bppRGB);

		if (pBitmap) {
			for (int f = 0; f < nWidth; f++) {
				float fVal = pLinePoints[f];
				fVal += 1;
				BYTE bVal = (BYTE)((fVal / 2.0f) * 0xFF);
				Gdiplus::Color color;
				color.SetValue(Gdiplus::Color::MakeARGB(255, bVal, bVal, bVal));
				pBitmap->SetPixel(f, 0, color);
				pBitmap->SetPixel(f, 1, color);
				pBitmap->SetPixel(f, 2, color);
				pBitmap->SetPixel(f, 3, color);
				pBitmap->SetPixel(f, 4, color);
			}

			CString strFileName;
			strFileName.Format("C:\\temp\\BarCodeInfo_%li.png", m_y);
			NxGdi::SaveToPNG(pBitmap, strFileName);
		}
	}
	*/

	// now scan for quiet zone
	long nQuietToleranceWidth = nWidth / 16; // 1/16th of the width
	const float fHighNoiseTolerance = nQuietToleranceWidth * 0.95f;
	const float fLowNoiseTolerance = nQuietToleranceWidth * 0.94f;

	float fRunningTotal = 0;
	for (int i = 0; i < nQuietToleranceWidth; i++) {
		fRunningTotal += pLinePoints[i];
	}

	bool bInQuietZone = fRunningTotal > fHighNoiseTolerance;
	long nCurrentQuietZoneWidth = bInQuietZone ? nQuietToleranceWidth : 0;

	for (long nCurrentPos = nQuietToleranceWidth; (nCurrentPos < nWidth) && (m_nLimit == 0 || nFound < m_nLimit); nCurrentPos++) {
		fRunningTotal -= pLinePoints[nCurrentPos - nQuietToleranceWidth];
		fRunningTotal += pLinePoints[nCurrentPos];

		if (!bInQuietZone) {
			if (fRunningTotal > fHighNoiseTolerance) {
				bInQuietZone = true;
				nCurrentQuietZoneWidth = 0;
			}
		} else {
			if (fRunningTotal < fLowNoiseTolerance) {
				bInQuietZone = false;
				if (nCurrentQuietZoneWidth >= nQuietToleranceWidth) {
					CString strResult;
					if (TryScanSegment_V3(pLinePoints, nCurrentPos - nQuietToleranceWidth, nWidth, strResult)) {
						nFound++;
						saResults.Add(strResult);
					};
				}
				nCurrentQuietZoneWidth = 0;
			} else {
				nCurrentQuietZoneWidth++;
			}
		}
	}

	delete[] pLinePoints;


	return nFound != 0;
}

BOOL CBarcodeDetector::TryScanSegment_V3(float* pLinePoints, long nBeginScan, long nWidth, OUT CString& strResult)
{
	// first we need to find the first bar.

	long nBegin;
	for (nBegin = nBeginScan; nBegin < nWidth; nBegin++) {
		if (pLinePoints[nBegin] < 0) {
			// begin the scan! 
			break;
		}
	}

	CString strFound;

	char c = -1;
	char cLastValue = 0;
	DWORD dwPos = 0;
	DWORD dwChecksum = 0;
	DWORD dwCurrentMode = 0;
	do {
		c = TryScanCharacter_V3(pLinePoints, nBegin, nWidth);
		if (c != -1 ) {
			switch(c) {
				case START_A:
				case START_B:
				case START_C:
				case CODE_A:
				case CODE_B:
				case CODE_C:
					dwCurrentMode = c;
					if (dwPos == 0) {
						dwChecksum += c;
					} else {
						dwChecksum += (dwPos * c);
						// (j.armen 2012-05-09 13:26) - PLID 50161 - If this is the case, then we are in the 
						//	middle of adding printable characters.  It is possible for the checksum to trigger
						//	be one of these codes.  Therefore, we add the printable char '\b' (backspace)
						//	so that we can allocate the correct text length in the string
						strFound += '\b';
					}
					break;
				case STOP:
					// stop character.
					{
						//Remove the last value from the checksum...
						dwChecksum -= ((dwPos -1) * cLastValue);

						DWORD dwFinalChecksum = dwChecksum % 103;

						if (dwFinalChecksum == cLastValue) {
							// huzzah!

							// remove the check digit
							strFound = strFound.Left(strFound.GetLength() - 1);

							// (j.armen 2012-05-09 13:28) - PLID 50161 - Ensure that we have removed all '\b' characters
							strFound.Replace("\b", "");

							strResult = strFound;
							return TRUE;
						} else {
							return FALSE;
						}
					}
					break;
				default:
					{
						dwChecksum += (dwPos * c);

						char cPrint = c;

						if (dwCurrentMode == START_B || dwCurrentMode == CODE_B) {
							cPrint += 32;

						} else if (dwCurrentMode == START_A || dwCurrentMode == CODE_A) {
							if (cPrint < 64) {
								cPrint += 32;
							} else {
								cPrint -= 64;
							}
						} else {
							return FALSE;
						}

						if (cPrint != 0) {
							strFound += cPrint;
						} else {
							ASSERT(FALSE);
							strFound += " ";
						}
					}
					break;
			}

			cLastValue = c;
			dwPos++;
		}
	} while (c != -1);

	return FALSE;
}

char CBarcodeDetector::TryScanCharacter_V3(float* pLinePoints, long &nBegin, long nWidth)
{
	// ok, let us begin.
	if (nWidth - nBegin < (nWidth / 16)) {
		return -1;
	}
	//TRACE("\n");
	
	// there are 6 bars in each segment,
	// and each bar has a total of 11 in width.
	// we can use this info to help us detect and error-correct.
	char c = -1;
	char d = -1;
	char b = -1;
	char e = -1;

	float fWidths[6];
	BYTE nWidths[6];

	::ZeroMemory(fWidths, sizeof(float) * 6);
	::ZeroMemory(nWidths, sizeof(BYTE) * 6);

	long nCurrentPos = nBegin;
	long nCurrentBar = 0;
	bool bBlackBar = true;

	BYTE nMinWidth = 0xFF;
	BYTE nMinBlackWidth = 0xFF;
	BYTE nMinWhiteWidth = 0xFF;
	
	bool bContinue = true;

	while (bContinue) {
		if (bBlackBar && pLinePoints[nCurrentPos] > 0) {
			// we are now in a white bar

			if (nWidths[nCurrentBar] < nMinWidth) {
				nMinWidth = nWidths[nCurrentBar];
			}
			if (nWidths[nCurrentBar] < nMinBlackWidth) {
				nMinBlackWidth = nWidths[nCurrentBar];
			}

			bBlackBar = false;
			nCurrentBar++;
		} else if (!bBlackBar && pLinePoints[nCurrentPos] < 0) {

			if (nWidths[nCurrentBar] < nMinWidth) {
				nMinWidth = nWidths[nCurrentBar];
			}
			if (nWidths[nCurrentBar] < nMinWhiteWidth) {
				nMinWhiteWidth = nWidths[nCurrentBar];
			}

			bBlackBar = true;
			nCurrentBar++;
		}
		
		if (nCurrentBar >= 6 || nCurrentPos >= nWidth) {
			bContinue = false;
		} else {		
			fWidths[nCurrentBar] += pLinePoints[nCurrentPos];
			nWidths[nCurrentBar]++;

			nCurrentPos++;
		}
	}

	CString strCharacter;

	char szBitCharacter[12];
	char szWholeBitCharacter[12];
	char szCompromiseCharacter[12];
	::ZeroMemory(szBitCharacter, sizeof(char) * 12);
	::ZeroMemory(szWholeBitCharacter, sizeof(char) * 12);
	::ZeroMemory(szCompromiseCharacter, sizeof(char) * 12);

	{
		long nCurrentBit = 0;

		float fBitWidth = float(nCurrentPos - nBegin) / 11.0f;
		
		for (float fBitStart = (float)nBegin; nCurrentBit < 11; fBitStart += fBitWidth) {
			float fBitEnd = fBitStart + fBitWidth;
			long nBeginBitPixels = (long)floor(fBitStart);
			long nEndBitPixels = (long)ceil(fBitEnd);

			float fWholeAvg;
			for (long nCurrentPixelPos = long(fBitStart + 0.5f); nCurrentPixelPos <= long(fBitEnd + 0.5f); nCurrentPixelPos++) {
				float fValue = pLinePoints[nCurrentPixelPos];

				if (nCurrentPixelPos == long(fBitStart + 0.5f)) {
					fWholeAvg = fValue;
				} else {
					fWholeAvg += fValue;
				}
			}

			float fAvg;
			float fAvgElements = 0;

			for (long nCurrentPixelPos = nBeginBitPixels; nCurrentPixelPos <= nEndBitPixels; nCurrentPixelPos++) {
				float fValue = pLinePoints[nCurrentPixelPos];

				if (nCurrentPixelPos == nEndBitPixels) {
					float fDiff = nEndBitPixels - fBitEnd;

					if (fDiff != 0) {
						fValue *= fDiff;

						fAvgElements += fDiff;
					}
				}

				if (nCurrentPixelPos == nBeginBitPixels) {
					float fDiff = fBitStart - nBeginBitPixels;

					if (fDiff != 0) {
						fValue *= (1.0f - fDiff);

						fAvgElements += (1.0f - fDiff);
					}
				} 

				fAvgElements += 1.0f;

				if (nCurrentPixelPos == nBeginBitPixels) {
					fAvg = fValue;
				} else {
					fAvg += fValue;
				}
			}

			// ok, we have an average...
			fAvg /= fAvgElements;
			fWholeAvg /= (nEndBitPixels - nBeginBitPixels + 1);

			//TRACE("Bit %x: Averages: Fractional %g, whole %g\n", nCurrentBit, fAvg, fWholeAvg);

			const float fAvgThreshold = 0.09f;

			char cFractionalChar = 0;
			char cWholeChar = 0;

			if (fAvg <= fAvgThreshold) {
				cFractionalChar = '1';
			} else {
				cFractionalChar = '0';
			}

			if (fWholeAvg <= fAvgThreshold) {
				cWholeChar = '1';
			} else {
				cWholeChar = '0';
			}

			szBitCharacter[nCurrentBit] = cFractionalChar;
			szWholeBitCharacter[nCurrentBit] = cWholeChar;

			if (cWholeChar != cFractionalChar) {
				szCompromiseCharacter[nCurrentBit] = '1';
			} else {
				szCompromiseCharacter[nCurrentBit] = cWholeChar;
			}

			nCurrentBit++;
		}
		
		DWORD dwValue = 0;
		if (!m_mapDecode.Lookup(szBitCharacter, dwValue)) {
			b = -1;
		} else {
			b = (char)dwValue;
		}
		
		DWORD dwValue2 = 0;
		if (!m_mapDecode.Lookup(szWholeBitCharacter, dwValue2)) {
			d = -1;
		} else {
			d = (char)dwValue2;
		}

		DWORD dwValue3 = 0;
		if (!m_mapDecode.Lookup(szCompromiseCharacter, dwValue3)) {
			e = -1;
		} else {
			e = (char)dwValue3;
		}
	}
	
	nBegin = nCurrentPos;
	
	// ok, now we have 6 bars and their float and integer widths
	// we just have to downsample it to 1-4.
	// since all codes must have 11 'bits', we can be sure that only
	// one value will have 4 bits, because otherwise it would be
	// impossible to have 4 more bars in 3 remaining bits.
	// all codes should have at least one single-width black or white bar.

	BYTE nAdjustedWidths[6];
	long nTotalWidth = 0;
	for (int i = 0; i < 6; i++) {
		nAdjustedWidths[i] = BYTE((float(nWidths[i]) / float(nMinWidth)) + 0.5f);
		nTotalWidth += nAdjustedWidths[i];
	}

	if (nTotalWidth != 11) {
		// this is not valid so far. We can still determine how close we are.
		c = -1;
	} else {
		// this can be sped up
		for (int i = 0; i < 6; i++) {
			for (int n = 0; n < nAdjustedWidths[i]; n++) {
				strCharacter += (i % 2 == 0) ? "1" : "0";
			}
		}

		DWORD dwValue = 0;
		if (!m_mapDecode.Lookup(strCharacter, dwValue)) {
			c = -1;
		} else {
			c = (char)dwValue;
		}
	}	

#ifdef _DEBUG
	if (
		(d != b || d != e || e != b) ||
		(c != -1 && (b == -1 && d == -1 && e == -1))
		)  {
		TRACE("%s fractional bit\t -> %li\n", szBitCharacter, (long)b);
		TRACE("%s whole bit     \t -> %li\n", szWholeBitCharacter, (long)d);
		TRACE("%s compromise bit\t -> %li\n", szCompromiseCharacter, (long)e);
		if (!strCharacter.IsEmpty()) {
			TRACE("%s classic min   \t -> %li\n\n", strCharacter, (long)c);
		}
	}
#endif

	if (c != -1) {
		return c;
	} else if (e != -1) {
		return e;
	} else if (d != -1) {
		return d;
	} else if (b != -1) {
		return b;
	}

	return -1;
}