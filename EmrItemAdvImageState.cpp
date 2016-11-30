#include "stdafx.h"
#include "EmrItemAdvImageState.h"
#include "NxImageCache.h"
#include "EMNDetail.h"
#include "EMRTopic.h"
#include "EMN.h"
#include "EMRHotSpot.h"

using namespace MSINKAUTLib;

// (j.armen 2014-07-21 16:32) - PLID 62836 - Moved class to its own file

CEmrItemAdvImageState::CEmrItemAdvImageState()
{
	//DRT 1/23/2008 - PLID 28603 - We are now on version 3
	// (z.manning 2010-02-23 11:33) - PLID 37412 - version 4
	// (z.manning 2011-10-05 16:20) - PLID 45842 - Version 5
	m_dwVersion = 5;
	m_eitImageTypeOverride = itDiagram;
}

// (j.armen 2014-07-22 09:03) - PLID 62836 - Constructor for creating from variant safe array
CEmrItemAdvImageState::CEmrItemAdvImageState(const _variant_t& vtState)
	: CEmrItemAdvImageState()
{
	CreateFromSafeArrayVariant(vtState);
}

// Returns a one-dimensional safe array of bytes representing the current contents of this 
// object.  The variant returned from this function can later be passed into the 
// CreateFromSafeArrayVariant() function to re-create this exact state.
_variant_t CEmrItemAdvImageState::AsSafeArrayVariant()
{
	/////
	// first 4 bytes: (v1+) m_nVersion
	// next 4 bytes: (v1+) m_eitImageTypeOverride
	// next 4 bytes: (v6+) boolean if we have a pen color
	// next 4 optional bytes: (v6+) if we have a pen color, the pen color value
	// next 4 bytes: (v6+) boolean if we have a pen size
	// next 4 optional bytes: (v6+) if we have a pen size, the pen size value
	// next 4 bytes: (v1+) number of characters (not bytes) in m_strImagePathOverride (not including the null terminator)
	// next dynamic len: (v1+) m_strImagePathOverride UNICODE, not null terminated; nothing is added here if the string is empty
	// next 4 bytes: (v1+) number of bytes contained in the m_varInkData safe array or 0 if it is empty
	// next dynamic len: (v1+) the bytes contained in the m_varInkData safe array; nothing is added here if m_varInkData is empty
	// next 4 bytes: (v2+) number of bytes contained in the m_varTextData safe array or 0 if it is empty
	// next dynamic len: (v2+) the bytes contained in the m_varTextData safe array; nothing is added here if m_varTextData is empty
	// next 4 bytes: (v4+) number of bytes contained in m_varDetailImageStamps safe array or 0 if it is empty
	// next dynamic len: (v4+) the bytes contained in the m_varDetailImageStamps safe array; nothing is added here if m_varDetailImageStamps is empty
	// next 4 bytes: (v5+) number of bytes in m_varPrintData
	// next dynamic len: (v5+) m_varPrintData
	// next 4 bytes:  (v3+) number of characters (not bytes) contained in m_strSelectedHotSpotData (not including null terminator)
	// next dynamic len: (v3+) m_strSelectedHotSpotData UNICODE, not null terminated; nothing is added here if the string is empty.
	//////

	BYTE *pInkData = NULL;
	BYTE *pTextData = NULL;
	BYTE *pDetailStampData = NULL;
	BYTE *pPrintData = NULL;

	try {
		// First just calculate the size our byte array will be
		DWORD dwTotalSize;
		DWORD dwInkSize;
		DWORD dwTextSize;
		DWORD dwDetailStampSize;
		DWORD dwPrintSize;
		{
			// Start at 0
			dwTotalSize = 0;
			// first 4 bytes: (v1+) m_nVersion
			dwTotalSize += sizeof(DWORD);
			// next 4 bytes: (v1+) m_eitImageTypeOverride
			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - sizeof(enum) is invalid
			dwTotalSize += sizeof(enum eImageType);
			// next 4 bytes: (v1+) number of characters (not bytes) in m_strImagePathOverride (not including the null terminator)
			dwTotalSize += sizeof(DWORD);
			// next dynamic len: (v1+) m_strImagePathOverride UNICODE, not null terminated; nothing is added here if the string is empty
			dwTotalSize += sizeof(WCHAR) * (m_strImagePathOverride.GetLength() + 1);
			// next 4 bytes: (v1+) number of bytes contained in the m_varInkData safe array or -1 if it is empty
			dwTotalSize += sizeof(DWORD);
			// next dynamic len: (v1+) the bytes contained in the m_varInkData safe array; nothing is added here if m_varInkData is empty
			{
				if (m_varInkData.vt != VT_EMPTY && m_varInkData.vt != VT_NULL) {
					COleSafeArray saInk;
					VARIANT varTempInkData = m_varInkData.Detach();
					try {
						saInk.Attach(varTempInkData);
						long nLow = 0, nHigh = 0;
						saInk.GetLBound(1, &nLow);
						saInk.GetUBound(1, &nHigh);
						ASSERT(saInk.GetDim() == 1);
						ASSERT(saInk.GetElemSize() == 1);
						dwInkSize = nHigh - nLow + 1;
						BYTE *p;
						saInk.AccessData((void **)&p);
						pInkData = new BYTE[dwInkSize];
						memcpy(pInkData, p, dwInkSize);
						saInk.UnaccessData();

						dwTotalSize += dwInkSize;
					}
					catch (...) {
						saInk.Detach();
						m_varInkData.Attach(varTempInkData);
						throw;
					}
					varTempInkData = saInk.Detach();
					m_varInkData.Attach(varTempInkData);
				}
				else {
					// It's empty so no more bytes will be added to the total size
					dwInkSize = -1;
					pInkData = NULL;
					dwTotalSize += 0;
				}
			}
			// next 4 bytes: (v2+) number of bytes contained in the m_varTextData safe array or 0 if it is empty
			dwTotalSize += sizeof(DWORD);
			// next dynamic len: (v2+) the bytes contained in the m_varTextData safe array; nothing is added here if m_varTextData is empty
			{
				if (m_varTextData.vt != VT_EMPTY && m_varTextData.vt != VT_NULL) {
					COleSafeArray saText;
					VARIANT varTempTextData = m_varTextData.Detach();
					try {
						saText.Attach(varTempTextData);
						long nLow = 0, nHigh = 0;
						saText.GetLBound(1, &nLow);
						saText.GetUBound(1, &nHigh);
						ASSERT(saText.GetDim() == 1);
						ASSERT(saText.GetElemSize() == 1);
						dwTextSize = nHigh - nLow + 1;
						BYTE *p;
						saText.AccessData((void **)&p);
						pTextData = new BYTE[dwTextSize];
						memcpy(pTextData, p, dwTextSize);
						saText.UnaccessData();

						dwTotalSize += dwTextSize;
					}
					catch (...) {
						saText.Detach();
						m_varTextData.Attach(varTempTextData);
						throw;
					}
					varTempTextData = saText.Detach();
					m_varTextData.Attach(varTempTextData);
				}
				else {
					// It's empty so no more bytes will be added to the total size
					dwTextSize = -1;
					pTextData = NULL;
					dwTotalSize += 0;
				}
			}

			// (z.manning 2010-02-23 12:41) - PLID 37412
			// next 4 bytes: (v4+) number of bytes contained in m_varDetailImageStamps safe array or 0 if it is empty
			dwTotalSize += sizeof(DWORD);
			// next dynamic len: (v4+) the bytes contained in the m_varDetailImageStamps safe array; nothing is added here if m_varDetailImageStamps is empty
			{
				if (m_varDetailImageStamps.vt != VT_EMPTY && m_varDetailImageStamps.vt != VT_NULL) {
					COleSafeArray saDetailStamps;
					VARIANT varTempDetailStampData = m_varDetailImageStamps.Detach();
					try {
						saDetailStamps.Attach(varTempDetailStampData);
						long nLow = 0, nHigh = 0;
						saDetailStamps.GetLBound(1, &nLow);
						saDetailStamps.GetUBound(1, &nHigh);
						ASSERT(saDetailStamps.GetDim() == 1);
						ASSERT(saDetailStamps.GetElemSize() == 1);
						dwDetailStampSize = nHigh - nLow + 1;
						BYTE *p;
						saDetailStamps.AccessData((void **)&p);
						pDetailStampData = new BYTE[dwDetailStampSize];
						memcpy(pDetailStampData, p, dwDetailStampSize);
						saDetailStamps.UnaccessData();

						dwTotalSize += dwDetailStampSize;
					}
					catch (...) {
						saDetailStamps.Detach();
						m_varDetailImageStamps.Attach(varTempDetailStampData);
						throw;
					}
					varTempDetailStampData = saDetailStamps.Detach();
					m_varDetailImageStamps.Attach(varTempDetailStampData);
				}
				else {
					// It's empty so no more bytes will be added to the total size
					dwDetailStampSize = -1;
					pDetailStampData = NULL;
					dwTotalSize += 0;
				}
			}

			// (z.manning 2011-10-05 16:11) - PLID 45842
			// next 4 bytes: (v5+) number of bytes contained in the m_varPrintData safe array or 0 if it is empty
			dwTotalSize += sizeof(DWORD);
			// next dynamic len: (v5+) the bytes contained in the m_varPrintData safe array; nothing is added here if m_varPrintData is empty
			{
				if (m_varPrintData.vt != VT_EMPTY && m_varPrintData.vt != VT_NULL)
				{
					COleSafeArray saPrintData;
					VARIANT varTempPrintData = m_varPrintData.Detach();
					try
					{
						saPrintData.Attach(varTempPrintData);
						long nLow = 0, nHigh = 0;
						saPrintData.GetLBound(1, &nLow);
						saPrintData.GetUBound(1, &nHigh);
						ASSERT(saPrintData.GetDim() == 1);
						ASSERT(saPrintData.GetElemSize() == 1);
						dwPrintSize = nHigh - nLow + 1;
						BYTE *p;
						saPrintData.AccessData((void **)&p);
						pPrintData = new BYTE[dwPrintSize];
						memcpy(pPrintData, p, dwPrintSize);
						saPrintData.UnaccessData();

						dwTotalSize += dwPrintSize;
					}
					catch (...) {
						saPrintData.Detach();
						m_varPrintData.Attach(varTempPrintData);
						throw;
					}
					varTempPrintData = saPrintData.Detach();
					m_varPrintData.Attach(varTempPrintData);
				}
				else {
					// It's empty so no more bytes will be added to the total size
					dwPrintSize = -1;
					pPrintData = NULL;
					dwTotalSize += 0;
				}
			}

			//DRT 1/18/2008 - PLID 28603 - HotSpots!
			// next 4 bytes:  (v3+) number of characters (not bytes) contained in m_strSelectedHotSpotData (not including null terminator)
			dwTotalSize += sizeof(DWORD);
			// next dynamic len:  (v3+) m_strSelectedHotSpotData UNICODE, not null terminated; nothing is added here if the string is empty.
			dwTotalSize += sizeof(WCHAR) * (m_strSelectedHotSpotData.GetLength() + 1);
		}

		// Now create a byte array that large
		COleSafeArray sa;
		sa.CreateOneDim(VT_UI1, dwTotalSize);

		// Fill the variant array
		BYTE *pSafeArrayData = NULL;
		sa.AccessData((LPVOID *)&pSafeArrayData);
		ASSERT(pSafeArrayData);
		if (pSafeArrayData) {
			// Copy from the given memory to the safearray memory
				{
					BYTE *pCurPos = pSafeArrayData;
					// first 4 bytes: (v1+) m_nVersion
					memcpy(pCurPos, &m_dwVersion, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					// next 4 bytes: (v1+) m_eitImageTypeOverride
					// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - sizeof(enum) is invalid
					memcpy(pCurPos, &m_eitImageTypeOverride, sizeof(enum eImageType));
					pCurPos += sizeof(enum eImageType);
					// next 4 bytes: (v1+) number of characters (not bytes) in m_strImagePathOverride (not including the null terminator)
					DWORD dwCharCount = m_strImagePathOverride.GetLength();
					memcpy(pCurPos, &dwCharCount, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					// next dynamic len: (v1+) m_strImagePathOverride UNICODE, not null terminated; nothing is added here if the string is empty
					BSTR bstr = m_strImagePathOverride.AllocSysString();
					memcpy(pCurPos, bstr, sizeof(WCHAR) * (dwCharCount));
					SysFreeString(bstr);
					pCurPos += sizeof(WCHAR) * (dwCharCount);
					// next 4 bytes: (v1+) number of bytes contained in the m_varInkData safe array or 0 if it is empty
					memcpy(pCurPos, &dwInkSize, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwInkSize != -1) {
						// next dynamic len: (v1+) the bytes contained in the m_varInkData safe array; nothing is added here if m_varInkData is empty
						memcpy(pCurPos, pInkData, dwInkSize);
						pCurPos += dwInkSize;
					}
					// next 4 bytes: (v2+) number of bytes contained in the m_varTextData safe array or 0 if it is empty
					memcpy(pCurPos, &dwTextSize, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwTextSize != -1) {
						// next dynamic len: (v2+) the bytes contained in the m_varTextData safe array; nothing is added here if m_varTextData is empty
						memcpy(pCurPos, pTextData, dwTextSize);
						pCurPos += dwTextSize;
					}

					// (z.manning 2010-02-23 11:41) - PLID 37412 - Detail image stamp data
					// next 4 bytes: (v4+) number of bytes contained in the m_varDetailStampData safe array or 0 if it is empty
					memcpy(pCurPos, &dwDetailStampSize, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwDetailStampSize != -1) {
						// next dynamic len: (v4+) the bytes contained in the m_varDetailImageStamps safe array; nothing is added here if m_varDetailImageStamps is empty
						memcpy(pCurPos, pDetailStampData, dwDetailStampSize);
						pCurPos += dwDetailStampSize;
					}

					// (z.manning 2011-10-05 16:16) - PLID 45842
					// next 4 bytes: (v5+) number of bytes contained in the m_varPrintData safe array or 0 if it is empty
					memcpy(pCurPos, &dwPrintSize, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwPrintSize != -1) {
						// next dynamic len: (v5+) the bytes contained in the m_varPrintData safe array; nothing is added here if m_varPrintData is empty
						memcpy(pCurPos, pPrintData, dwPrintSize);
						pCurPos += dwPrintSize;
					}

					//DRT 1/18/2008 - PLID 28603 - HotSpots!
					// next 4 bytes:  (v3+) number of characters (not bytes) contained in m_strSelectedHotSpotData (not including null terminator)
					DWORD dwHotSpotCount = m_strSelectedHotSpotData.GetLength();
					memcpy(pCurPos, &dwHotSpotCount, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					// next dynamic len:  (v3+) m_strSelectedHotSpotData UNICODE, not null terminated; nothing is added here if the string is empty.
					BSTR bstrHotSpot = m_strSelectedHotSpotData.AllocSysString();
					memcpy(pCurPos, bstrHotSpot, sizeof(WCHAR) * (dwHotSpotCount));
					SysFreeString(bstrHotSpot);
					pCurPos += sizeof(WCHAR) * (dwHotSpotCount);
				}

			// Let go of the data pointer
			sa.UnaccessData();
			pSafeArrayData = NULL;
			if (pInkData) {
				delete[]pInkData;
			}
			if (pTextData) {
				delete[]pTextData;
			}
			if (pDetailStampData) {
				delete[]pDetailStampData;
			}
			if (pPrintData != NULL) {
				delete[]pPrintData;
			}
			// Get the variant object out of the safe array and return the variant
			// (a.walling 2010-08-10 13:16) - PLID 40060 - Memory leak. Detach returns a VARIANT which is copied into the _variant_t return value. The original
			// VARIANT is leaked. Returning the result of a _variant_t constructor to take advantage of in-place return value optimizations, so no copies occur.
			return _variant_t(sa.Detach(), false);
		}
		else {
			// Failure, let go of the data pointer and throw an exception
			sa.UnaccessData();
			pSafeArrayData = NULL;
			if (pInkData) {
				delete[]pInkData;
			}
			if (pTextData) {
				delete[]pTextData;
			}
			if (pDetailStampData) {
				delete[]pDetailStampData;
			}
			if (pPrintData != NULL) {
				delete[]pPrintData;
			}
			ThrowNxException(_T("CEmrItemAdvImageState::AsSafeArrayVariant: Could not access data from SafeArray"));
		}
	}
	catch (...) {
		if (pInkData) {
			delete[]pInkData;
		}
		if (pTextData) {
			delete[]pTextData;
		}
		if (pDetailStampData) {
			delete[]pDetailStampData;
		}
		if (pPrintData != NULL) {
			delete[]pPrintData;
		}
		throw;
	}
}

void CEmrItemAdvImageState::CreateFromSafeArrayVariant(const _variant_t &varState)
{
	// Make sure the data type is good; we DO NOT allow null, empty, or any other non-array variant type.
	if (!(varState.vt & VT_ARRAY)) {
		// Invalid type, just load as blank
		m_strImagePathOverride = "";
		m_strSelectedHotSpotData = "";
		m_eitImageTypeOverride = itDiagram;
		m_varInkData.Clear();
		m_varTextData.Clear();
		m_varPrintData.Clear();
		return;
	}

	// Copy the const parameter to our writable safe array
	COleSafeArray sa;
	{
		_variant_t varSafeArray(varState);
		sa.Attach(varSafeArray.Detach());
	}

	// Access the safe array byte data
	DWORD dwInkSize = -1;
	BYTE *pInkData = NULL;
	DWORD dwTextSize = -1;
	BYTE *pTextData = NULL;
	DWORD dwDetailStampSize = -1;
	BYTE *pDetailStampData = NULL;
	DWORD dwPrintSize = -1;
	BYTE *pPrintData = NULL;
	{
		DWORD nSafeArrayDataLength = sa.GetOneDimSize();
		BYTE *pSafeArrayData = NULL;
		sa.AccessData((LPVOID *)&pSafeArrayData);
		ASSERT(pSafeArrayData);
		if (pSafeArrayData) {
			// (a.walling 2013-06-06 09:41) - PLID 57069 - Hash of the state blob
			m_hash = NxMD5(pSafeArrayData, nSafeArrayDataLength);
			// Copy from the given memory to our member variables
			{
				BYTE *pCurPos = pSafeArrayData;
				// first 4 bytes: (v1+) dwVersion
				// Use a local variable rather than our member variable
				DWORD dwVersion = 0;
				{
					memcpy(&dwVersion, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}
				if (dwVersion >= 1) {
					// next 4 bytes: (v1+) m_eitImageTypeOverride
					// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - sizeof(enum) is invalid
					memcpy(&m_eitImageTypeOverride, pCurPos, sizeof(enum eImageType));
					pCurPos += sizeof(enum eImageType);
				}
				DWORD dwCharCount = 0;
				if (dwVersion >= 1) {
					// next 4 bytes: (v1+) number of characters (not bytes) in m_strImagePathOverride (not including the null terminator)
					memcpy(&dwCharCount, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}
				if (dwVersion >= 1) {
					// next dynamic len: (v1+) m_strImagePathOverride UNICODE, not null terminated; nothing is added here if the string is empty
					BSTR str = new OLECHAR[dwCharCount + 1];
					memcpy(str, pCurPos, sizeof(WCHAR) * (dwCharCount));
					pCurPos += sizeof(WCHAR) * (dwCharCount);
					str[dwCharCount] = 0;
					char *strAnsi = _com_util::ConvertBSTRToString(str);
					m_strImagePathOverride = strAnsi;
					delete[]strAnsi;
					delete[]str;
				}
				if (dwVersion >= 1) {
					// next 4 bytes: (v1+) number of bytes contained in the m_varInkData safe array or 0 if it is empty
					memcpy(&dwInkSize, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}
				if (dwVersion >= 1) {
					if (dwInkSize != -1 && dwInkSize > 0) {
						// next dynamic len: (v1+) the bytes contained in the m_varInkData safe array; nothing is added here if m_varInkData is empty
						pInkData = new BYTE[dwInkSize];
						memcpy(pInkData, pCurPos, dwInkSize);
						pCurPos += dwInkSize;
					}
				}
				if (dwVersion >= 2) {
					// next 4 bytes: (v2+) number of bytes contained in the m_varTextData safe array or 0 if it is empty
					memcpy(&dwTextSize, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}
				if (dwVersion >= 2) {
					if (dwTextSize != -1 && dwTextSize > 0) {
						// next dynamic len: (v2+) the bytes contained in the m_varTextData safe array; nothing is added here if m_varTextData is empty
						pTextData = new BYTE[dwTextSize];
						memcpy(pTextData, pCurPos, dwTextSize);
						pCurPos += dwTextSize;
					}
				}

				// (z.manning 2010-02-23 12:47) - PLID 37412 - Detail image stamp data
				if (dwVersion >= 4)
				{
					// next 4 bytes: (v4+) number of bytes contained in the m_varDetailImageStamps safe array or 0 if it is empty
					memcpy(&dwDetailStampSize, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwDetailStampSize != -1 && dwDetailStampSize > 0) {
						// next dynamic len: (v2+) the bytes contained in the m_varDetailImageStamps safe array; nothing is added here if m_varDetailImageStamps is empty
						pDetailStampData = new BYTE[dwDetailStampSize];
						memcpy(pDetailStampData, pCurPos, dwDetailStampSize);
						pCurPos += dwDetailStampSize;
					}
				}

				// (z.manning 2011-10-05 16:21) - PLID 45842
				if (dwVersion >= 5)
				{
					// next 4 bytes: (v5+) number of bytes contained in the m_varPrintData safe array or 0 if it is empty
					memcpy(&dwPrintSize, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
					if (dwPrintSize != -1 && dwPrintSize > 0) {
						// next dynamic len: (v5+) the bytes contained in the m_varPrintData safe array; nothing is added here if m_varPrintData is empty
						pPrintData = new BYTE[dwPrintSize];
						memcpy(pPrintData, pCurPos, dwPrintSize);
						pCurPos += dwPrintSize;
					}
				}

				//DRT 1/18/2008 - PLID 28603 - Hot Spots!
				DWORD dwHotSpotCount = 0;
				if (dwVersion >= 3) {
					// next 4 bytes:  (v3+) number of characters (not bytes) contained in m_strSelectedHotSpotData (not including null terminator)
					memcpy(&dwHotSpotCount, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}
				if (dwVersion >= 3) {
					// next dynamic len:  (v3+) m_strSelectedHotSpotData UNICODE, not null terminated; nothing is added here if the string is empty.
					BSTR str = new OLECHAR[dwHotSpotCount + 1];
					memcpy(str, pCurPos, sizeof(WCHAR) * (dwHotSpotCount));
					pCurPos += sizeof(WCHAR) * (dwHotSpotCount);
					str[dwHotSpotCount] = 0;
					char *strAnsi = _com_util::ConvertBSTRToString(str);
					m_strSelectedHotSpotData = strAnsi;
					delete[]strAnsi;
					delete[]str;
				}
			}

			// Let go of the data pointer
			sa.UnaccessData();
			pSafeArrayData = NULL;
		}
		else {
			// Failure
			sa.UnaccessData();
			pSafeArrayData = NULL;
			ThrowNxException(_T("CEmrItemAdvImageState::CreateFromSafeArrayVariant: Could not access data from SafeArray"));
		}

		// Now that we've released our outer safe array, we've still got to construct one for the ink data
		if (dwInkSize != -1) {
			COleSafeArray saIn;
			if (dwInkSize != 0) {
				// Create the array of the right size
				saIn.CreateOneDim(VT_UI1, dwInkSize, pInkData);
			}
			else {
				// Setup the bounds and create the empty array
				SAFEARRAYBOUND rgsabound;
				rgsabound.cElements = 0;
				rgsabound.lLbound = 0;
				saIn.Create(VT_UI1, 1, &rgsabound);
			}
			m_varInkData.Attach(saIn.Detach());
		}
		else {
			m_varInkData.Clear();
		}

		if (pInkData) {
			delete[]pInkData;
		}

		//TES 11/7/2006 - As well as one for the text data.
		if (dwTextSize != -1) {
			COleSafeArray saIn;
			if (dwTextSize != 0) {
				// Create the array of the right size
				saIn.CreateOneDim(VT_UI1, dwTextSize, pTextData);
			}
			else {
				// Setup the bounds and create the empty array
				SAFEARRAYBOUND rgsabound;
				rgsabound.cElements = 0;
				rgsabound.lLbound = 0;
				saIn.Create(VT_UI1, 1, &rgsabound);
			}
			m_varTextData.Attach(saIn.Detach());
		}
		else {
			m_varTextData.Clear();
		}

		if (pTextData) {
			delete[]pTextData;
		}

		// (z.manning 2010-02-23 12:50) - PLID 37412 - Detail image stamp data
		if (dwDetailStampSize != -1) {
			COleSafeArray saIn;
			if (dwDetailStampSize != 0) {
				// Create the array of the right size
				saIn.CreateOneDim(VT_UI1, dwDetailStampSize, pDetailStampData);
			}
			else {
				// Setup the bounds and create the empty array
				SAFEARRAYBOUND rgsabound;
				rgsabound.cElements = 0;
				rgsabound.lLbound = 0;
				saIn.Create(VT_UI1, 1, &rgsabound);
			}
			m_varDetailImageStamps.Attach(saIn.Detach());
		}
		else {
			m_varDetailImageStamps.Clear();
		}
		if (pDetailStampData != NULL) {
			delete[] pDetailStampData;
		}

		// (z.manning 2011-10-05 16:26) - PLID 45842
		if (dwPrintSize != -1)
		{
			COleSafeArray saIn;
			if (dwPrintSize != 0) {
				// Create the array of the right size
				saIn.CreateOneDim(VT_UI1, dwPrintSize, pPrintData);
			}
			else {
				// Setup the bounds and create the empty array
				SAFEARRAYBOUND rgsabound;
				rgsabound.cElements = 0;
				rgsabound.lLbound = 0;
				saIn.Create(VT_UI1, 1, &rgsabound);
			}
			m_varPrintData.Attach(saIn.Detach());
		}
		else {
			m_varPrintData.Clear();
		}
		if (pPrintData != NULL) {
			delete[]pPrintData;
		}
	}
}

CString CEmrItemAdvImageState::CreateByteStringFromInkData()
{
	return CreateByteStringFromSafeArrayVariant(m_varInkData);
}

CString CEmrItemAdvImageState::CreateByteStringFromTextData()
{
	return CreateByteStringFromSafeArrayVariant(m_varTextData);
}

CString CEmrItemAdvImageState::CalcCurrentBackgroundImageFullPath(IN long nPatIDIfNecessary, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType) const
{
	// (d.thompson 2009-03-05 17:10) - PLID 32891 - Previous decision was that if the current path was empty, to always
	//		use the default path.  We now allow a "force blank" option, so in that case we want to really use a blank path.
	if (m_eitImageTypeOverride == itForcedBlank) {
		return "";
	}

	// Calculate the image source
	if (!m_strImagePathOverride.IsEmpty()) {
		switch (m_eitImageTypeOverride) {
			case itAbsolutePath:
				return m_strImagePathOverride;
				break;
			case itPatientDocument:
				ASSERT(nPatIDIfNecessary != -1);
				return GetPatientDocumentPath(nPatIDIfNecessary) ^ m_strImagePathOverride;
				break;
			case itDiagram:
			case itSignature: // (z.manning 2008-11-03 09:54) - PLID 31890
				// (a.walling 2010-01-05 09:09) - PLID 33887 - Signatures are now relative
				return GetSharedPath() ^ "Images" ^ m_strImagePathOverride;
				break;
			case itMirrorImage:
				return m_strImagePathOverride; // This is the Mirror ID
				break;
			default:
				// Unknown, use default
				ASSERT(FALSE);
				return strOrigBackgroundImageFilePath;
				break;
		}
	}
	else if (!strOrigBackgroundImageFilePath.IsEmpty()) {
		// Use the default.
		switch (eitOrigBackgroundImageType) {
			case itForcedBlank:
			case itAbsolutePath:
				return strOrigBackgroundImageFilePath;
				break;
			case itPatientDocument:
				return GetPatientDocumentPath(nPatIDIfNecessary) ^ strOrigBackgroundImageFilePath;
				break;
			case itDiagram:
			case itSignature: // (z.manning 2008-11-03 09:55) - PLID 31890
				// (a.walling 2010-01-05 09:09) - PLID 33887 - Signatures are now relative
				return GetSharedPath() ^ "Images" ^ strOrigBackgroundImageFilePath;
				break;
			default:
				// Unknown, use default.
				ASSERT(FALSE);
				return strOrigBackgroundImageFilePath;
				break;
		}
	}
	else {
		return "";
	}
}

// (a.walling 2008-02-13 14:10) - PLID 28605 - Pass in the detail to render hotspots
HBITMAP CEmrItemAdvImageState::Render(OPTIONAL IN CEMNDetail* pDetail, OPTIONAL OUT CSize *pszOutputBmpSize, IN CRect rImageSize, IN const long nPatIDIfNecessary, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType) const
{
	// Get the full path to the current image
	CString strFullPath = CalcCurrentBackgroundImageFullPath(nPatIDIfNecessary, strOrigBackgroundImageFilePath, eitOrigBackgroundImageType);

	// Make sure we got a path
	if (strFullPath.IsEmpty()) {
		// No path so no image, which means no image to even draw on which means we expect there to be no ink.
		ASSERT(m_varInkData.vt == VT_EMPTY);
		return NULL;
	}

	NxImageLib::ISourceImagePtr pSourceImage;
	NxImageLib::ICachedImagePtr pCachedImage;
	NxImageLib::ITempBitmapPtr pTempBitmap;

	// Get the screen device context, we will use this to create our own "compatible" context for rendering
	HDC hDC = ::GetDC(NULL);
	if (hDC) {
		// Load the background image object from the path calculated above
		HBITMAP hImage = NULL;
		bool bOwnsImage = true;
		// (a.walling 2010-10-25 10:04) - PLID 41043 - Obtain a cached image reference to use an already-loaded instance if available
		if (m_eitImageTypeOverride == itMirrorImage) {
			// (a.walling 2011-05-02 15:41) - PLID 43530 - Cache the mirror image as well (loading duplicate full-res images takes time and a lot of memory)
			//g_EmrImageCache.GetCachedImage(CString("mirror://") + strFullPath, pCachedImage);
			// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
			if (pSourceImage = NxImageLib::Cache::OpenSourceImage(CString("mirror://") + strFullPath)) {
				if (SUCCEEDED(pSourceImage->raw_Load())) {
					if (pCachedImage = pSourceImage->GetCachedImage(rImageSize.Size())) {
						if (pTempBitmap = pCachedImage->TempBitmap) {
							hImage = pTempBitmap ? (HBITMAP)pTempBitmap->Handle : NULL;
						}
					}
				}
			}
			if (hImage) {
				bOwnsImage = false;
			}
		}
		else {
			// (a.walling 2010-10-25 10:04) - PLID 41043 - Obtain a cached image reference to use an already-loaded instance if available
			//g_EmrImageCache.GetCachedImage(strFullPath, pCachedImage);			
			// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
			if (pSourceImage = NxImageLib::Cache::OpenSourceImage(strFullPath)) {
				if (SUCCEEDED(pSourceImage->raw_Load())) {
					if (pCachedImage = pSourceImage->GetCachedImage(rImageSize.Size())) {
						if (pTempBitmap = pCachedImage->TempBitmap) {
							hImage = pTempBitmap ? (HBITMAP)pTempBitmap->Handle : NULL;
						}
					}
				}
			}
			if (hImage) {
				bOwnsImage = false;
			}

			// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - Detail itself stores cached image references for data output
			if (pSourceImage && pCachedImage && pDetail) {
				pDetail->m_pCachedImage = pCachedImage;
				pDetail->m_pSourceImage = pSourceImage;
			}

			// (j.jones 2010-01-06 10:09) - PLID 36754 - raise an exception if the image file does not exist,
			// but do not throw it all the way out, just handle it it right now, and cleanly exist
			// (j.jones 2014-01-13 11:56) - PLID 49971 - this now no longer throws exceptions unless the detail is null
			if (!hImage) {

				// (j.jones 2015-04-16 15:09) - PLID 60457 - We now also have a different warning if the path does exist,
				// but the image could not be loaded, so it must have been an invalid image file.
				bool bDoesFileExist = DoesExist(strFullPath);

				try {
					if (pDetail == NULL) {
						//this is unlikely - we have never seen this before
						ThrowNxException("The image file '%s' could not be loaded because pDetail is NULL.", strFullPath);
					}
					else {

						//See if we have already warned about this path for this detail,
						//if so, then don't warn again. This will eliminate multiple popups
						//for the same invalid image when loading an EMN or navigating back
						//to the same topic multiple times.
						if (strFullPath != pDetail->GetLastWarnedInvalidImageFilePath()) {

							//get the information needed for a detailed warning
							CString strDetailName = pDetail->GetLabelText();
							CString strTopicName = "";
							CString strEMNName = "";
							if (pDetail->m_pParentTopic) {
								strTopicName = pDetail->m_pParentTopic->GetName();
								if (pDetail->m_pParentTopic->GetParentEMN()) {
									strEMNName = pDetail->m_pParentTopic->GetParentEMN()->GetDescription();
								}
							}

							CString strWarning = "";

							// (j.jones 2015-04-16 15:15) - PLID 60457 - if the file exists, but just could not
							// be loaded, say so
							if (bDoesFileExist) {
								strWarning.Format("An image file could not be loaded:\n\n"
									"Image File Path: %s\n"
									"EMN: %s\n"
									"Topic: %s\n"
									"Image Detail: %s\n\n"
									"This image could not be loaded because it is no longer a valid image file.\n\n"
									"The file may have been replaced or corrupted. Please confirm that you can browse to this file "
									"and can successfully open and view the file in a standard image viewer.",
									strFullPath, strEMNName, strTopicName, strDetailName);
							}
							else {
								//file does not exist

								// (j.jones 2014-01-13 11:56) - PLID 49971 - give a clean message based on what could not be found

								//is the shared path invalid?
								CString strSharedPath = GetSharedPath();
								if (strFullPath.GetLength() > strSharedPath.GetLength()
									&& strSharedPath.CompareNoCase(strFullPath.Left(strSharedPath.GetLength())) == 0) {

									//the image is definitely in our shared path, so confirm we can access the shared path
									if (!DoesExist(strSharedPath)) {
										strWarning.Format("An image file could not be loaded:\n\n"
											"Image File Path: %s\n"
											"EMN: %s\n"
											"Topic: %s\n"
											"Image Detail: %s\n\n"
											"This image could not be loaded because your shared path, %s, could not be accessed.\n\n"
											"Please confirm your dock settings are correct, and try browsing to this path to confirm "
											"that it exists and that you have permission to access it.",
											strFullPath, strEMNName, strTopicName, strDetailName, strSharedPath);
									}
								}
								if (strWarning.IsEmpty()) {
									//the shared path is accessible, or our image is not in the shared path,
									//so see if the path exists
									CString strFolderOnly = FileUtils::GetFilePath(strFullPath);
									if (!strFolderOnly.IsEmpty() && !DoesExist(strFolderOnly)) {
										strWarning.Format("An image file could not be loaded:\n\n"
											"Image File Path: %s\n"
											"EMN: %s\n"
											"Topic: %s\n"
											"Image Detail: %s\n\n"
											"This image could not be loaded because the folder it was saved in could not be found.\n\n"
											"This folder may have been renamed or deleted. Please confirm that you can browse to this path "
											"and that you have permission to access it.",
											strFullPath, strEMNName, strTopicName, strDetailName);
									}
								}
								if (strWarning.IsEmpty()) {
									//the folder exists, so this image file must truly be missing
									strWarning.Format("An image file could not be loaded:\n\n"
										"Image File Path: %s\n"
										"EMN: %s\n"
										"Topic: %s\n"
										"Image Detail: %s\n\n"
										"This image could not be loaded because it no longer exists by this name.\n\n"
										"The file may have been renamed or deleted. Please confirm that you can browse to this file "
										"and that you have permission to access it.",
										strFullPath, strEMNName, strTopicName, strDetailName);
								}
							}

							MessageBox(GetActiveWindow(), strWarning, "Practice", MB_ICONEXCLAMATION | MB_OK);

							//now track this path in the detail, so if we hit this code again for the same detail,
							//we won't warn again for the same path
							pDetail->SetLastWarnedInvalidImageFilePath(strFullPath);
						}
					}
				} NxCatchAll("Error in CEmrItemAdvImageState::Render (1)");

				//cleanly leave this function
				::ReleaseDC(NULL, hDC);
				return NULL;
			}

			// (a.walling 2008-10-13 12:06) - PLID 31669 - Use NxGdiPlus' LoadImageFile
			//NxGdi::LoadImageFile(strFullPath, hImage);
		}

		// (j.jones 2014-01-13 14:51) - PLID 49971 - if we get here, the image path must have been valid,
		// so clear the last invalid image path so it doesn't hang around to confuse us
		if (pDetail) {
			pDetail->SetLastWarnedInvalidImageFilePath("");
		}

		// (c.haag 2007-02-16 16:33) - PLID 24796 - If hImage is null, it means we failed
		// to load the image from disk or Mirror. There is nothing more we can do here, so
		// return.
		if (NULL == hImage) {
			::ReleaseDC(NULL, hDC); // We're done with the device context
			return NULL;
		}

		// Create the memory device context
		CDC dc;
		dc.Attach(::CreateCompatibleDC(hDC));

		::ReleaseDC(NULL, hDC);
		hDC = NULL;

		//TES 2/7/2007 - PLID 18159 - Scale the image to the actual size it is on the EMN.
		HBITMAP hImageScaled = NULL;
		double dScaleX = 1.0, dScaleY = 1.0;
		BITMAP tmpBmp = { 0 };

		// (c.haag 2007-04-16 12:23) - PLID 25614 - Make sure the bitmap is legitimate. If it's not,
		// then fail.
		if (pTempBitmap && pSourceImage) {
			// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
			CSize sz = pSourceImage->Size;
			tmpBmp.bmHeight = sz.cy;
			tmpBmp.bmWidth = sz.cx;
		}
		else if (!GetObject(hImage, sizeof(tmpBmp), &tmpBmp) || tmpBmp.bmWidth <= 0 || tmpBmp.bmHeight <= 0) {
			// (a.walling 2010-10-25 10:04) - PLID 41043 - Only delete if we own the image object
			if (bOwnsImage) {
				DeleteObject(hImage);
			}
			return NULL;
		}

		// (c.haag 2007-04-16 12:17) - PLID 25614 - Check to see whether the input image dimensions
		// are valid. If they are not, then we will not scale the image; but rather, use its default
		// dimensions by looking at tmpBmp.
		//
		// The image dimensions are invalid if the width is not positive. It doesn't matter if the height
		// if positive because the calculations below compensate for it. Sometimes the caller will not
		// pass in a valid height because it wants it auto-calculated.
		//
		int nScaledWidth, nScaledHeight;
		if (rImageSize.Width() <= 0) {
			// If we get here, we have an invalid input size. This is usually caused by a user adding a
			// list detail to a template that spawns other details, but those spawned details were not
			// positioned by the user. Default the dimensions to the bitmap size.
			nScaledWidth = tmpBmp.bmWidth;
			nScaledHeight = tmpBmp.bmHeight;
			// dScaleX and dScaleY default to 1.0, so no need to change them here
		}
		else {
			dScaleX = (double)rImageSize.Width() / (double)tmpBmp.bmWidth;
			// (a.walling 2007-04-12 11:35) - PLID 25605 - Scale proportionately if the height is not specified
			if (rImageSize.Height() > 0) {
				dScaleY = (double)rImageSize.Height() / (double)tmpBmp.bmHeight;
			}
			else {
				dScaleY = dScaleX;
			}

			if (dScaleX < dScaleY) {
				dScaleY = dScaleX;
			}
			else {
				dScaleX = dScaleY;
			}

			nScaledWidth = (int)(((double)tmpBmp.bmWidth)*dScaleX);
			nScaledHeight = (int)(((double)tmpBmp.bmHeight)*dScaleY);
		}

		// (a.walling 2008-10-08 14:48) - PLID 31623 - No point scaling 1.0!
		// (a.walling 2010-10-25 10:04) - PLID 41043 - Actually by avoiding this we can easily take advantage of a cached/loaded image
		/*
		if (dScaleX == 1.0 && dScaleY == 1.0) {
		// we want full resolution, no scaling, so no point in creating a 'thumb' that is the same exact size
		hImageScaled = hImage;
		hImage = NULL;
		} else {
		*/
		{
			// always load here, even if it means 'copying' the image
			LoadThumbFromImage(hImage, hImageScaled, nScaledWidth, nScaledHeight);
			// (a.walling 2007-08-07 08:53) - PLID 26996 - We are done with this potentially huge image object
			// this was the big cause of memory leaks here.
			// (a.walling 2010-10-25 10:04) - PLID 41043 - Only delete if we own the image object
			if (bOwnsImage) {
				DeleteObject(hImage);
			}
			hImage = NULL;
		}

		// Select the image into the memory device context so that we can draw the ink on it
		HBITMAP hbmpOld = (HBITMAP)::SelectObject(dc.m_hDC, hImageScaled);

		// Draw the current set of ink strokes onto the memory dc (and therefore onto the bitmap)
		{
			// Get the ink
			IInkDispPtr pNewInk(__uuidof(InkDisp));
			if (m_varInkData.vt != VT_EMPTY) {
				pNewInk->Load(m_varInkData);
			}
			// Create a renderer and use it to draw the ink strokes
			IInkRendererPtr pRend(__uuidof(InkRenderer));
			//TES 2/7/2007 - PLID 18159 - Apply the scale we calculated.
			IInkTransformPtr pTransform(__uuidof(InkTransform));
			pTransform->ScaleTransform((float)dScaleX, (float)dScaleY);
			pRend->SetViewTransform(pTransform);

			// (j.armen 2011-12-09 09:15) - PLID 46347 - If we know what the original DPI of an Ink is, 
			//then we can scale to match that original. Since we have no way of knowing, we are going to hard code
			//96, which is the standard and default.  All new images will be scaled, meaning that they will be drawn
			//as if they are in 96 DPI for the original
			int nOriginalDPI = 96;
			float fDPIScaleX = GetDeviceCaps(dc.m_hDC, LOGPIXELSX) / (float)nOriginalDPI;
			float fDPIScaleY = GetDeviceCaps(dc.m_hDC, LOGPIXELSY) / (float)nOriginalDPI;

			IInkTransformPtr pObjectTransform(__uuidof(InkTransform));
			pObjectTransform->ScaleTransform(1 / fDPIScaleX, 1 / fDPIScaleY);
			pRend->SetObjectTransform(pObjectTransform);

			pRend->Draw((long)dc.m_hDC, pNewInk->GetStrokes());
		}

		// (a.walling 2008-02-13 14:12) - PLID 28605 - Draw the hotspots!
		if (pDetail) {
			CDWordArray dwaSelectedHotSpotIDs;
			ParseDelimitedStringToDWordArray(m_strSelectedHotSpotData, ";", dwaSelectedHotSpotIDs);

			// if none are selected, don't bother getting the array
			if (dwaSelectedHotSpotIDs.GetSize()) {
				CEMRHotSpotArray* parHotSpots = pDetail->GetHotSpotArray();

				// (a.walling 2008-02-13 15:33) - PLID 28605 - Sanity check; make sure this is not null!
				ASSERT(parHotSpots != NULL);
				if (parHotSpots) {
					// (a.walling 2008-02-13 14:41) - PLID 28605 - For each selected hotspot,
					// draw the output to the DC. Ingore unselected hotspots.
					for (int x = 0; x < dwaSelectedHotSpotIDs.GetSize(); x++) {

						for (int i = 0; i < parHotSpots->GetSize(); i++) {
							CEMRHotSpot *pehsHotSpot = parHotSpots->GetAt(i);

							if (pehsHotSpot->GetID() == (long)dwaSelectedHotSpotIDs[x]) {
								// (a.walling 2008-02-13 14:43) - PLID 28605 - Unlike the ink scale,
								// we need to pass in the reciprocal for this to work correctly
								// (c.haag 2009-02-18 11:06) - PLID 31327 - Always pass in TRUE so that
								// the hotspot color is the normal color. If a user is editing an item
								// such that they are able to write in the hotspot area, it only really
								// matters to visually queue the user in the control, and nowhere else.
								pehsHotSpot->Draw(&dc, 1 / dScaleX, 1 / dScaleY, TRUE, 0, 0);
								break;
							}
						}
					}
				}
			}
		}

		//TES 11/10/2006 - PLID 18159 - Also draw the text.
		CNxInkPictureText nipt;
		nipt.LoadFromVariant(m_varTextData);
		// (r.gonet 02/14/2012) - PLID 37682 - Make sure to assign the filter
		nipt.SetTextStringFilter(pDetail->GetImageTextStringFilter());

		// (r.gonet 05/06/2011) - PLID 43542 - We may need to draw text to scale.
		bool bEnableTextScaling = GetRemotePropertyInt("EnableEMRImageTextScaling", 1, 0, "<None>", true) == 1;
		//TES 2/7/2007 - PLID 18159 - Apply the scale we calculated.
		// (r.gonet 07/17/2012) - PLID 38240
		nipt.DrawAllStrings(dc.m_hDC, bEnableTextScaling, true, dScaleX, dScaleY);

		// Release the bitmap from the memory dc so that we can return it
		::SelectObject(dc.m_hAttribDC, hbmpOld);

		// Return the bitmap width and height if requested (if not requested, the 
		// caller can still ask the bitmap itself what its dimensions are)
		if (pszOutputBmpSize) {
			//Get the dimensions.
			BITMAP tmpBmp;
			GetObject(hImageScaled, sizeof(tmpBmp), &tmpBmp);
			*pszOutputBmpSize = CSize(tmpBmp.bmWidth, tmpBmp.bmHeight);
		}

		// And finally return the bitmap itself
		return hImageScaled;
	}
	else {
		// Couldn't get the screen device context, that's the ballgame.  But it really should never happen.
		ASSERT(FALSE);
		return NULL;
	}

}

void CEmrItemAdvImageState::SetSelectedHotSpots(IN CArray<long, long> &arynHotSpotIDs)
{
	m_strSelectedHotSpotData.Empty();
	for (int nHotSpotIndex = 0; nHotSpotIndex < arynHotSpotIDs.GetSize(); nHotSpotIndex++) {
		m_strSelectedHotSpotData += AsString(arynHotSpotIDs.GetAt(nHotSpotIndex)) + ';';
	}
}

BOOL CEmrItemAdvImageState::operator ==(CEmrItemAdvImageState &eiaisCompare)
{
	//TES 5/15/2008 - PLID 27776 - Simple enough algorithm, just compare each member variable, if any are different, return
	// FALSE, otherwise return TRUE.  I attempted to put these in order of quickest to evaluate first, so that if anything
	// gets skipped it's the slow part, but even for an image with lots of ink and text, that is identical (so all code in here
	// executes), this function only takes 1 ms.
	if (m_dwVersion != eiaisCompare.m_dwVersion) {
		return FALSE;
	}
	if (m_eitImageTypeOverride != eiaisCompare.m_eitImageTypeOverride) {
		return FALSE;
	}
	if (m_strImagePathOverride != eiaisCompare.m_strImagePathOverride) {
		return FALSE;
	}
	if (m_strSelectedHotSpotData != eiaisCompare.m_strSelectedHotSpotData) {
		return FALSE;
	}
	if (CreateByteStringFromInkData() != eiaisCompare.CreateByteStringFromInkData()) {
		return FALSE;
	}
	if (CreateByteStringFromTextData() != eiaisCompare.CreateByteStringFromTextData()) {
		return FALSE;
	}
	// (z.manning 2011-10-05 17:23) - PLID 45842 - print data
	if (CreateByteStringFromSafeArrayVariant(m_varPrintData) != CreateByteStringFromSafeArrayVariant(eiaisCompare.m_varPrintData)) {
		return FALSE;
	}
	return TRUE;
}