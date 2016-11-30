// WordParser.h: interface for the WordParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORDPARSER_H__B5FB5BBB_6F83_4DBE_AB0A_5A3A048B9FC4__INCLUDED_)
#define AFX_WORDPARSER_H__B5FB5BBB_6F83_4DBE_AB0A_5A3A048B9FC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "comutil.h"
#include "afxtempl.h"

// (a.walling 2008-04-10 09:58) - PLID 9446 - This class takes a path to a word document, parses out its piece table, and then
// reassembles them in a single string of all the text in the document, which can then be parsed like normal to get merge fields.

static const INT FileOffsetSize = 4;
static const UINT UnicodeMask = (UINT)(1 << 30);

struct FileOffset  
{

	UINT32 value;

	static BOOL IsUnicode(UINT fc)
	{
		return (fc & UnicodeMask) == 0;
	}

	static UINT NormalizeFc(UINT fc)
	{
		if ((fc & UnicodeMask) == 0)
			return fc;
		return (fc & ~UnicodeMask) / 2;
	}   

	static UINT GetFcDelta(BOOL isUnicode)
	{
		return isUnicode ? (UINT)2 : (UINT)1;
	}   
};

static const INT PieceDescriptorSize = 8;

struct PieceDescriptor  
{
	BYTE _Flags;
	BYTE _Fn;
	UINT32 _Fc;
	INT16 _Prm;
};

class WordParser  
{
public:
	BYTE* GetText(const CString& strPath, DWORD& nTotalLength);
	WordParser();
	virtual ~WordParser();

protected:
	CArray<PieceDescriptor, PieceDescriptor> m_arPieceDescriptors;
	CArray<FileOffset, FileOffset> m_arFileOffsets;

	void ParsePieceDescriptors(BYTE* pData, INT nOffset, UINT nLength);
	BOOL GetPieceFileBounds(int piece, UINT &start, UINT &end);

	UINT ParseString(BYTE* pData, UINT nLength, BOOL bIsUnicode, BYTE* pSequentialData);

	static int GetDescriptorsCount(int size, int structureSize);
	static int GetOffsetsCount(int size, int structureSize);
	static void GetDataFromFib(BYTE* pData, CString &strTableName, INT32 &dcOffset, UINT32 &dcLength);
};

#endif // !defined(AFX_WORDPARSER_H__B5FB5BBB_6F83_4DBE_AB0A_5A3A048B9FC4__INCLUDED_)
