// NxCDO.h: interface for the NxCDO namspace.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXCDO_H__C4A70022_DAAF_43F6_9D80_EC037666513C__INCLUDED_)
#define AFX_NXCDO_H__C4A70022_DAAF_43F6_9D80_EC037666513C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

// (a.walling 2012-08-15 13:42) - PLID 52053 - CDOSYS dependencies are internal to NxCDO.cpp


#define ENCRYPT_VERSION 0xA2
// the encrypt version will start at A1 and increment upwards.
// (z.manning 2012-09-11 09:29) - PLID 52543 - A2 - Added modified date

namespace NxCDO {
	// (a.walling 2009-11-23 11:56) - PLID 36396 - Modified several input CString& references to be const

	// (a.walling 2007-06-08 12:14) - PLID 26263 - Need to compile all images and etc into a single MHT file
	//(e.lally 2009-09-11) PLID 35533 - Added parameter for whether or not to encrypt the file. Default to encrypt.
	// (a.walling 2010-01-13 12:55) - PLID 36840 - Added parameter to prevent overwriting an existing file
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BOOL CreateMHTFromFile(const CString& strFileIn, const CString& strFileOut, IN const SYSTEMTIME &stModifiedDate, BOOL bEncrypt = TRUE, BOOL bFailIfExists = FALSE);

	// (a.walling 2007-06-11 10:46) - PLID 26261 - Decrypt the MHT to prepare to be loaded.
	// (a.walling 2011-06-17 15:35) - PLID 42367 - Output param for file time
	BOOL DecryptMHTFromFile(const CString& strFileIn, HANDLE hFile, FILETIME* pLastWriteTime = NULL);

	// (a.walling 2007-06-08 12:14) - PLID 26263 - Save the message stream to a file
	//(e.lally 2009-09-11) PLID 35533 - Added parameter for whether or not to encrypt the file
	// (a.walling 2010-01-13 12:55) - PLID 36840 - Added parameter to prevent overwriting an existing file
	// (a.walling 2012-08-15 13:42) - PLID 52053 - Using IDispatch parameter to remove external CDOSYS dependencies
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BOOL SaveMessageToFile(IDispatch* pMsgDisp, const CString& strFileOut, BOOL bEncrypt, BOOL bFailIfExists, IN const SYSTEMTIME &stModifiedDate);
	
	// (a.walling 2007-06-08 12:15) - PLID 26261 - Encrypt a given ADODB::_Stream
	// (a.walling 2012-08-15 13:42) - PLID 52053 - Using IDispatch parameter to remove external CDOSYS dependencies
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BYTE* EncryptStream(IN IDispatch* pStreamDisp, IN const SYSTEMTIME &stModifiedDate, OUT DWORD *pdwOutputLen);

	// (a.walling 2007-06-08 12:15) - PLID 26261 - Helper function to encrypt a versioned array of bytes
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BYTE* Encrypt(IN const BYTE *pInput, IN const DWORD dwInputLen, IN const SYSTEMTIME &stModifiedDate, OUT DWORD *pdwOutputLen);
	// (a.walling 2007-06-08 12:15) - PLID 26261 - Helper function to decrypt a versioned array of bytes
	BYTE* Decrypt(IN const BYTE *pInput, IN const DWORD dwInputLen, OUT DWORD *pdwOutputLen);

	// (a.walling 2009-11-23 11:55) - PLID 36396 - Extract an MHTML archive into component parts
	void ExtractMHTToPath(const CString& strFileIn, const CString& strOutputPath, CString& strHTMLFile, CStringArray& saTempFiles, const CString& strOutputFilePrefix = "");

	// (z.manning 2012-09-11 13:15) - PLID 52543
	COleDateTime GetModifiedDateFieldFromFile(const CString& strFileIn);
};

#endif // !defined(AFX_NXCDO_H__C4A70022_DAAF_43F6_9D80_EC037666513C__INCLUDED_)
