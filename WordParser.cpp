// WordParser.cpp: implementation of the WordParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WordParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2008-04-10 09:58) - PLID 9446 - This class takes a path to a word document, parses out its piece table, and then
// reassembles them in a single string of all the text in the document, which can then be parsed like normal to get merge fields.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WordParser::WordParser()
{

}

WordParser::~WordParser()
{

}

BYTE* WordParser::GetText(const CString &strPath, DWORD &nTotalLength)
{
	BYTE* pSequentialData = NULL;
	BYTE* pData = NULL;
	BYTE* pTableData = NULL;

	HR(StgIsStorageFile(_bstr_t(strPath)));

	IStoragePtr pStorage;
	HR(StgOpenStorage(_bstr_t(strPath), NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, NULL, 0, (IStorage**)&pStorage));

	try {
		IStreamPtr pWordStream;
		HR(pStorage->OpenStream(_bstr_t("WordDocument"), NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, NULL, (IStream**)&pWordStream));

		STATSTG stat;
		HR(pWordStream->Stat(&stat, 0));
		ULARGE_INTEGER nSize = stat.cbSize;
		DWORD dwSize = (DWORD)nSize.QuadPart; // will fail if > 4gb
		pData = new BYTE[dwSize];
		HR(pWordStream->Read(pData, dwSize, 0));

		INT32 dcOffset;
		UINT32 dcLength;
		CString strTableName;
		GetDataFromFib(pData, strTableName, dcOffset, dcLength);

		IStreamPtr pTableStream;
		HR(pStorage->OpenStream(_bstr_t(strTableName), NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, NULL, (IStream**)&pTableStream));
		pTableStream->Stat(&stat, 0);
		nSize = stat.cbSize;
		dwSize = (DWORD)nSize.QuadPart; // will fail if > 4gb
		pTableData = new BYTE[dwSize];
		HR(pTableStream->Read(pTableData, dwSize, 0));

		ParsePieceDescriptors(pTableData, dcOffset, dcLength);
		delete[] pTableData;
		pTableData = NULL;

		// Get the total length
		nTotalLength = 0;
		int i;
		for (i = 0; i < m_arPieceDescriptors.GetSize(); i++)
		{			
			UINT pieceStart;
			UINT pieceEnd;
			BOOL bIsUnicode = GetPieceFileBounds(i, pieceStart, pieceEnd);

			nTotalLength += (pieceEnd - pieceStart) / (bIsUnicode ? 2 : 1);
		}

		// get our buffer
		pSequentialData = new BYTE[nTotalLength];
		ZeroMemory(pSequentialData, nTotalLength);
		BYTE* pSequentialDataPos = pSequentialData;
		for (i = 0; i < m_arPieceDescriptors.GetSize(); i++)
		{
			UINT pieceStart;
			UINT pieceEnd;
			BOOL bIsUnicode = GetPieceFileBounds(i, pieceStart, pieceEnd);

			BYTE* pStart = pData + pieceStart;

			UINT pieceLength = ParseString(pStart, pieceEnd - pieceStart, bIsUnicode, pSequentialDataPos);
			pSequentialDataPos += pieceLength;
		}
		
		delete[] pData;
		pData = NULL;
	} NxCatchAllCallThrow("Error parsing word piece table!", {delete[] pData; delete[] pSequentialData; delete[] pTableData;});

	return pSequentialData;
}

UINT WordParser::ParseString(BYTE* pData, UINT nLength, BOOL bIsUnicode, BYTE* pSequentialData)
{
	if (nLength == 0)
		return 0;

	if (bIsUnicode)
		nLength = nLength / 2;

	UINT nRealLength = 0;

	for (UINT i = 0; i < nLength; i++) {
		if (!bIsUnicode) {
			BYTE ch = *pData;
			pData += sizeof(BYTE);
			pSequentialData[i] = (char)ch;
			nRealLength++;
		} else {
			INT16 ch = *(INT16*)pData;
			pData += sizeof(INT16);
			pSequentialData[i] = (char)ch;
			nRealLength++;
		}
	}

	ASSERT(nRealLength == nLength);

	return nRealLength;
}



//static
void WordParser::GetDataFromFib(BYTE* pData, CString &strTableName, INT32 &dcOffset, UINT32 &dcLength)
{
	UINT16 flags = *((UINT16*)(pData + 10));
	// (a.walling 2009-09-17 16:30) - PLID 35587 - I have no idea why I was ANDing with 9. Clearly, I intended to check
	// the 9th bit (ie, 0x200). This corresponds to fWhichTblStm within the flags byte (10th byte of the FIB structure).
	// see pg 143 of the Word92-2007BinaryFIleFormat(doc)Specification.pdf : File Information Block (FIB)
	strTableName = (flags & 0x200) ? "1Table" : "0Table";

	BYTE* pDataPointer = pData;

	pDataPointer += 418;
	dcOffset = *(INT32*)pDataPointer;
	pDataPointer += sizeof(INT32);
	dcLength = *(UINT32*)pDataPointer;
}    

//static
int WordParser::GetOffsetsCount(int size, int structureSize)
{
	return GetDescriptorsCount(size, structureSize) + 1;
}

//static
int WordParser::GetDescriptorsCount(int size, int structureSize)
{
	int ptrSize = 4;
	return (size - ptrSize) / (structureSize + ptrSize);
}

BOOL WordParser::GetPieceFileBounds(int piece, UINT &start, UINT &end)
{
	start = 0xffffffff;
	end = 0xffffffff;

	PieceDescriptor pd = m_arPieceDescriptors[piece];

	UINT fc = pd._Fc;
	BOOL isUnicode = FileOffset::IsUnicode(fc);
	start = FileOffset::NormalizeFc(fc);

	UINT length = m_arFileOffsets[piece + 1].value - m_arFileOffsets[piece].value;
	end = (UINT)(start + length * FileOffset::GetFcDelta(isUnicode));

	return isUnicode;
}

void WordParser::ParsePieceDescriptors(BYTE *pData, INT nOffset, UINT nLength)
{
	BYTE* p = pData;
	p += nOffset;

	while (p <= (pData + nOffset + nLength)) {
		BYTE byteType = *p;
		p += sizeof(BYTE);

		switch(byteType)
		{
			case 0:
				p += sizeof(BYTE);
				break;
			case 1: {
				INT16 cbGrpprl = *(INT16*)p;
				p += sizeof(INT16);
				p += (sizeof(BYTE) * cbGrpprl);
					}
				break;
			case 2:
				INT32 nTableLen = *(INT32*)p;
				p += sizeof(INT32);

				// Parse offsets
				m_arFileOffsets.RemoveAll();
				long nOffsets = GetOffsetsCount(nTableLen, PieceDescriptorSize);

				for (int i = 0; i < nOffsets; i++) {
					FileOffset offset;
					offset.value = *(UINT32*)p;
					p += sizeof(UINT32);
					m_arFileOffsets.Add(offset);
				}

				// Parse descriptors
				m_arPieceDescriptors.RemoveAll();
				long nDescriptors = GetDescriptorsCount(nTableLen, PieceDescriptorSize);

				for (i = 0; i < nDescriptors; i++) {
					PieceDescriptor pd;
					pd._Flags = (BYTE)(*p);
					p += sizeof(BYTE);
					pd._Fn = (BYTE)(*p);
					p += sizeof(BYTE);
					pd._Fc = *(UINT32*)p;
					p += sizeof(UINT32);
					pd._Prm = *(INT16*)p;
					p += sizeof(INT16);

					m_arPieceDescriptors.Add(pd);
				}
				break;
		}
	}      
}

