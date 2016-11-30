// NxCDO.cpp: implementation of the NxCDO namspace.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxCDO.h"
#include "FileUtils.h"
#include "wininet.h"
#include "shlwapi.h"
#include "MiscSystemUtils.h"
#include <NxAdoLib/msadoStream.h>

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-08-29 01:10) - PLID 52053 - Extracted ADODB stream definitions into msadoStream.h
namespace ADODB {
	using namespace ADOStream;
}
using namespace ADODB;

// (a.walling 2012-08-15 13:42) - PLID 52053 - CDOSYS dependencies are internal to NxCDO.cpp
//#import "cdosys.dll" rename_namespace("CDOSYS")

// (a.walling 2012-09-05 11:35) - PLID 52053 - For lack of a better PLID - getting rid of cdosys.dll import, just use what we know is good
// unfortunately cdosys.dll will automatically find the (bad) ado typelibs due to auto_search, so this just saves everyone a big headache
#include "cdosys.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// (a.walling 2009-11-23 11:56) - PLID 36396 - Modified several input CString& references to be const

namespace NxCDO {
	// (a.walling 2007-06-08 12:14) - PLID 26263 - Need to compile all images and etc into a single MHT file
	//(e.lally 2009-09-11) PLID 35533 - Added the ability to create unencrypted file	
	// (a.walling 2010-01-13 12:55) - PLID 36840 - Added parameter to prevent overwriting an existing file
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BOOL CreateMHTFromFile(const CString& strFileIn, const CString& strFileOut, IN const SYSTEMTIME &stModifiedDate, BOOL bEncrypt /*= TRUE*/, BOOL bFailIfExists /*= FALSE*/)
	{
		// (a.walling 2007-08-08 16:08) - PLID 27017 - Removed the logging, we are good.
		CString strURL;

		DWORD dwNewLength = INTERNET_MAX_URL_LENGTH;

		// (a.walling 2007-07-09 09:57) - PLID 26261 - Need to ensure this is a valid URL
		UrlCreateFromPath(strFileIn, strURL.GetBuffer(INTERNET_MAX_URL_LENGTH), &dwNewLength, NULL);
		strURL.ReleaseBuffer(dwNewLength);

		//TRACE("File (%s) converted to URL (%s)\r\n", strFileIn, strURL);

		__int64 iiSize = -1;
		if (FileUtils::DoesFileOrStreamExist(strFileIn)) {
			iiSize = FileUtils::GetFileSize(strFileIn);
		}

		CString str;
		str.Empty();
		BSTR bstrUserName = str.AllocSysString();
		BSTR bstrPass = str.AllocSysString();

		BSTR bstrURL = strURL.AllocSysString();

		try {
			CDOSYS::IMessagePtr pMsg(__uuidof(CDOSYS::Message));
			CDOSYS::IConfigurationPtr pConfig(__uuidof(CDOSYS::Configuration));

			pMsg->put_Configuration(pConfig);

			try {
				HRESULT hr = pMsg->CreateMHTMLBody(bstrURL, CDOSYS::cdoSuppressNone, bstrUserName, bstrPass);
			} NxCatchAllThrow("Error creating MHTML body");

			//(e.lally 2009-09-11) PLID 35533 - Added parameter for whether to use encryption
			SaveMessageToFile(pMsg, strFileOut, bEncrypt, bFailIfExists, stModifiedDate);

			// ensure that the Message is released before the Configuration
			pMsg = NULL;
			pConfig = NULL;
		} NxCatchAll("Error creating MHTML Archive");

		SysFreeString(bstrURL);
		SysFreeString(bstrUserName);
		SysFreeString(bstrPass);

		return TRUE;
	}

	// (a.walling 2007-06-11 10:46) - PLID 26261 - Decrypt the MHT to prepare to be loaded.
	// (a.walling 2011-06-17 15:35) - PLID 42367 - Output param for file time
	BOOL DecryptMHTFromFile(const CString& strFileIn, HANDLE hFile, FILETIME* pLastWriteTime /* = NULL */)
	{
		BYTE* pData = NULL;
		BYTE* pDecryptedData = NULL;

		try {
			CFile f;
			// (a.walling 2013-07-10 14:56) - PLID 57474 - allow readers
			if (f.Open(strFileIn, CFile::modeRead | CFile::shareDenyWrite)) {
				//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs.  However,
				// there's no point going to great lengths to fully support that, this file should never be more than 4 GB.
				// If it is, just throw an exception.
#if _MSC_VER > 1300
				ULONGLONG ullSize = f.GetLength();
				if(ullSize > ULONG_MAX) {
					AfxThrowNxException("DecryptMHTFromFile() - file found larger than maximum supported size (4 GB)");
				}
				DWORD dwSize = (DWORD)ullSize;
#else
				DWORD dwSize = f.GetLength();
#endif

				// (a.walling 2011-06-17 15:35) - PLID 42367 - Update the lastwrite time if we have it
				if (pLastWriteTime && !::GetFileTime(f.m_hFile, NULL, NULL, pLastWriteTime)) {
					pLastWriteTime->dwLowDateTime = 0;
					pLastWriteTime->dwHighDateTime = 0;
				}

				pData = new BYTE[dwSize];
				f.Read(pData, dwSize);

				DWORD dwDecryptedLen = 0;
				pDecryptedData = Decrypt(pData, dwSize, &dwDecryptedLen);

				if (dwDecryptedLen > 0 && pDecryptedData) {
					// Write our decrypted data to the outfile
					DWORD dwWritten = 0;
					WriteFile(hFile, pDecryptedData, dwDecryptedLen, &dwWritten, NULL);

					ASSERT(dwWritten == dwDecryptedLen);
				}

				if (pDecryptedData) {
					delete[] pDecryptedData;
					pDecryptedData = NULL;
				}

				if (pData) {
					delete[] pData;
					pData = NULL;
				}

				return TRUE;
			} else {
				// file not found
				return FALSE;
			} 
		} NxCatchAll("Failed to decrypt secure EMN preview");

		if (pData)
			delete[] pData;
		if (pDecryptedData)
			delete[] pDecryptedData;

		return FALSE;
	}

	// (a.walling 2007-06-08 12:14) - PLID 26263 - Save the message stream to a file
	//(e.lally 2009-09-11) PLID 35533 - Added the ability to create unencrypted file
	// (a.walling 2010-01-13 12:55) - PLID 36840 - Added parameter to prevent overwriting an existing file
	// (a.walling 2012-08-15 13:42) - PLID 52053 - Using IDispatch parameter to remove external CDOSYS dependencies
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BOOL NxCDO::SaveMessageToFile(IDispatch* pMsgDisp, const CString& strFileOut, BOOL bEncrypt, BOOL bFailIfExists, IN const SYSTEMTIME &stModifiedDate)
	{		
		ADODB::_StreamPtr	pStream = NULL;

		try {
			CDOSYS::IMessagePtr pMsg(pMsgDisp);

			if (!pMsg) {
				ThrowNxException("Failed to get stream from dispatch");
			}

			pStream = pMsg->GetStream();

			DWORD dwLen = 0;
			BYTE* pOutputData;
			//(e.lally 2009-09-11) PLID 35533 - Added the ability to create unencrypted file
			if(bEncrypt){
				pOutputData = EncryptStream(pStream, stModifiedDate, &dwLen);
			}
			else {
				DWORD dwBytes = pStream->GetSize();
				_variant_t varData = pStream->ReadText(dwBytes);
				if (varData.vt == VT_BSTR) {
					CString strData = VarString(varData);
					dwLen = strData.GetLength()+1;
					pOutputData = new BYTE[dwLen];
					memcpy(pOutputData, (BYTE*)strData.GetBuffer(dwLen), dwLen);
					strData.ReleaseBuffer();
				}
			}
			pStream = NULL; // releases

			if (pOutputData && dwLen > 0) {
				HANDLE hFile = CreateFile(strFileOut, GENERIC_WRITE, FILE_SHARE_READ, NULL, bFailIfExists ? CREATE_NEW : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				try {
					if (hFile == INVALID_HANDLE_VALUE) {
						// failed, try to create path.
						bool bCreatePath = FileUtils::CreatePath(FileUtils::GetFilePath(strFileOut));
						hFile = CreateFile(strFileOut, GENERIC_WRITE, FILE_SHARE_READ, NULL, bFailIfExists ? CREATE_NEW : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile == INVALID_HANDLE_VALUE) {
							DWORD nLastError = GetLastError();
							if (bFailIfExists && (nLastError == ERROR_FILE_EXISTS)) {
								// (a.walling 2010-01-13 13:09) - PLID 36840 - We still throw an exception here, but since we are expecting to fail
								// if it exists, we don't need to log or provide any further debug information.
								ThrowNxException("The file %s already exists.", strFileOut);
							} else {
								// (b.savon 2013-06-25 10:54) - PLID 50011 - Handle Error 1265 and 1326 gracefully
								//http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
								CString strMessage;
								BOOL bHandled = FALSE;
								switch( nLastError ){
									case ERROR_DOWNGRADE_DETECTED /*1265*/:
										{
											strMessage.Format("Practice is attempting to create the file '%s' but cannot contact a domain controller to service the authentication request.\r\n\r\nPlease contact your network administrator for assistance.", strFileOut);
											bHandled = TRUE;
										}
										break;
									case ERROR_LOGON_FAILURE /*1326*/:
										{
											strMessage.Format("Practice is attempting to create the file '%s' but your username and/or password is incorrect.\r\n\r\nPlease contact your network administrator for assistance.", strFileOut);
											bHandled = TRUE;
										}
										break;
									default:
										{
											ThrowNxException(FormatString("Could not access file %s (%s)", strFileOut, FormatError(nLastError)));
										}
										break;
								}

								// If we constructed a graceful error message: show the message box, cleanup after ourselves, and return to the caller
								// that we failed.
								if(bHandled){
									//The the message we're showing the user and the Windows error
									CString strLogMessage = strMessage + "\r\n" + FormatString("Windows Error Message (%s)", FormatError(nLastError));
									Log(strLogMessage);
									GetMainFrame()->PopUpMessageBoxAsync(strMessage, MB_ICONINFORMATION);
									if(pOutputData){
										delete[] pOutputData;
										pOutputData = NULL;
									}
									return FALSE;
								}
							}
						}
					}
				} catch (...) {
					if(pOutputData){
						delete[] pOutputData;
						pOutputData = NULL;
					}

					throw;					
				}

				BOOL bSuccessfulWrite = TRUE;
				DWORD nLastError = 0;
				if (hFile != INVALID_HANDLE_VALUE) {
					DWORD dwWritten = 0;
					bSuccessfulWrite = WriteFile(hFile, pOutputData, dwLen, &dwWritten, NULL);
					nLastError = GetLastError();

					::CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
				
				if(pOutputData){
					delete[] pOutputData;
					pOutputData = NULL;
				}

				if (!bSuccessfulWrite) {
					ThrowNxException(FormatString("Could not write to file %s (%s)", strFileOut, FormatError(nLastError)));
				}

			} else {
				ASSERT(FALSE);
			}

			return TRUE;
		} NxCatchAll("Error in SaveMessageToFile - Could not write stream");

		return FALSE;
	}

	// (a.walling 2007-06-08 12:15) - PLID 26261 - Encrypt a given CDOSYS::_Stream
	// (a.walling 2012-08-15 13:42) - PLID 52053 - Using IDispatch parameter to remove external CDOSYS dependencies
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BYTE* EncryptStream(IDispatch* pStreamDisp, IN const SYSTEMTIME &stModifiedDate, OUT DWORD *pdwOutputLen)
	{
		try {
			ADODB::_StreamPtr pStream(pStreamDisp);

			if (!pStream) {
				ThrowNxException("Failed to get stream from dispatch");
			}

			DWORD dwBytes = pStream->GetSize();

			//TES 6/14/2012 - PLID 50997 - For some reason, at Dr. McAdoo's office, the ReadText line was throwing an exception.  Also for some
			// reason, having it read 100 bytes first and then read the rest caused the error to go away.  So, that's what we do here now.
			_variant_t varHundred = pStream->ReadText(100);
			_variant_t varData = pStream->ReadText(dwBytes);			

			if (varHundred.vt == VT_BSTR && varData.vt == VT_BSTR) {
				CString strData = VarString(varHundred) + VarString(varData);
				dwBytes = strData.GetLength();
				BYTE* pData = (BYTE*)strData.GetBuffer(dwBytes); // we just need a const array of bytes

				DWORD dwBytesEncrypted = 0;
				BYTE* pEncryptedData = Encrypt(pData, dwBytes, stModifiedDate, pdwOutputLen);
				strData.ReleaseBuffer();

				return pEncryptedData;
			} else {
				ThrowNxException("Unexpected data format");
			}
		} NxCatchAllThrow("Error encrypting stream");

		return NULL;
	}

	// (a.walling 2007-06-08 12:15) - PLID 26261 - Helper function to encrypt a versioned array of bytes
	// (z.manning 2012-09-11 09:53) - PLID 52543 - Added modified date
	BYTE* Encrypt(IN const BYTE *pInput, IN const DWORD dwInputLen, IN const SYSTEMTIME &stModifiedDate, OUT DWORD *pdwOutputLen)
	{
		if (pInput != NULL && dwInputLen > 0)
		{
			// add one byte for version (0xA1 initially)
			// (z.manning 2012-09-11 09:54) - PLID 52543 - Allocate space for modified date
			DWORD dwSize = dwInputLen + 1 + sizeof(SYSTEMTIME);
			BYTE *pEncrypt = new BYTE[dwSize];
			BYTE *pData = pEncrypt;
			try {
				pData[0] = ENCRYPT_VERSION;
				pData++;
				// (z.manning 2012-09-11 10:00) - PLID 52543 - We now include the modified date in the file
				memcpy(pData, &stModifiedDate, sizeof(SYSTEMTIME));
				pData += sizeof(SYSTEMTIME);
				BYTE prev = ENCRYPT_VERSION; // rather than starting with 0, let's start with our encrypt version.
				for (DWORD i = 0; i < dwInputLen; i++) {
					pData[i] = (pInput[i] + 0x7d) ^ (prev ^ 0x4d);
					prev = pInput[i];
				}
				*pdwOutputLen = dwSize;
				return pEncrypt;
			}
			catch (...) {
				*pdwOutputLen = 0;
				delete[] pEncrypt;
				ThrowNxException("Error encrypting bytes");
			}
		}

		// input is null, or length = 0
		// invalid parameter(s); could throw an exception here, but we'll just set last error to 87 and return null
		// since it's not really an exception, we technically successfully encrypted zero-length data.
		LogDetail("\tEncryption attempted on NULL input, or zero-length input!");
		SetLastError(87);
		return NULL;
	}

	// (z.manning 2012-09-11 13:15) - PLID 52543
	COleDateTime GetModifiedDateFieldFromFile(const CString& strFileIn)
	{
		COleDateTime dtModifiedDate = g_cdtInvalid;

		CFile file;
		// (a.walling 2013-07-10 14:56) - PLID 57474 - allow readers
		if(file.Open(strFileIn, CFile::modeRead | CFile::shareDenyWrite))
		{
			BYTE nVersion;
			file.Read(&nVersion, 1);
			if(nVersion >= 0xA2)
			{
				SYSTEMTIME st;
				file.Read(&st, sizeof(SYSTEMTIME));
				dtModifiedDate = SystemTimeToVariantTimePrecise(st);
			}

			file.Close();
		}

		return dtModifiedDate;
	}

	// (a.walling 2007-06-08 12:15) - PLID 26261 - Helper function to decrypt a versioned array of bytes
	BYTE* Decrypt(IN const BYTE *pInput, IN const DWORD dwInputLen, OUT DWORD *pdwOutputLen)
	{
		if (pInput != NULL && dwInputLen > 0)
		{
			try
			{
				// don't need the one byte version
				DWORD dwDecryptSize = dwInputLen - 1;
				const BYTE *pData = pInput;
				BYTE nVersion = pData[0];
				pData++;

				// (z.manning 2012-09-11 12:36) - PLID 52543 - Version 0xA2 - Modified date
				if (nVersion >= 0xA2)
				{
					// (z.manning 2012-09-11 12:37) - PLID 52543 - Read the modified date
					SYSTEMTIME stModifiedDate;
					memcpy(&stModifiedDate, pData, sizeof(SYSTEMTIME));
					// (z.manning 2012-09-11 12:38) - PLID 52543 - The decrypted data doesn't contain the modified date
					// so subtract its size.
					dwDecryptSize -= sizeof(SYSTEMTIME);
					// (z.manning 2012-09-11 12:38) - PLID 52543 - Now advance to where the actual data begins.
					pData += sizeof(SYSTEMTIME);
				}

				BYTE *pDecrypt = new BYTE[dwDecryptSize]; 
				if (nVersion >= 0xA1)
				{
					BYTE prev = nVersion;
					for (DWORD i = 0; i < dwDecryptSize; i++) {
						pDecrypt[i] = (pData[i] ^ (prev ^ 0x4d)) - 0x7d;
						prev = pDecrypt[i];
					}
					*pdwOutputLen = dwInputLen - 1; // minus one byte for version
					return pDecrypt;
				}
				else {
					// unsupported version
					*pdwOutputLen = 0;
					delete[] pDecrypt;
					ThrowNxException("Error decrypting bytes - unsupported version");
				}
			}
			catch (...) {
				*pdwOutputLen = 0;
				ThrowNxException("Error decrypting bytes");
			}
		}

		// input is null, or length = 0
		// invalid parameter(s); could throw an exception here, but we'll just set last error to 87 and return null
		// since it's not really an exception, we technically successfully encrypted zero-length data.
		SetLastError(87);
		return NULL;
	}

	// (a.walling 2009-11-23 11:55) - PLID 36396 - Extract an MHTML archive into component parts
	void ExtractMHTToPath(const CString& strFileIn, const CString& strOutputPath, CString& strHTMLFile, CStringArray& saTempFiles, const CString& strOutputFilePrefix /* = ""*/)
	{
		ADODB::_StreamPtr pStream(__uuidof(ADODB::Stream));

		_bstr_t bstrUserName("");
		_bstr_t bstrPass("");

		tagVARIANT varOptional = COleVariant((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		pStream->Open(&varOptional, adModeUnknown, adOpenStreamUnspecified, bstrUserName, bstrPass);
		pStream->LoadFromFile((LPCTSTR)strFileIn);

		CDOSYS::IMessagePtr pMessage(__uuidof(CDOSYS::Message));

		pMessage->DataSource->OpenObject(pStream, "IStream");
		_bstr_t bstrHTMLBody = pMessage->HTMLBody;
		CString strHtmlBody = (LPCTSTR)bstrHTMLBody;

		CDOSYS::IBodyPartsPtr pBodyParts = pMessage->BodyPart->BodyParts;

		if (pBodyParts) {
			for (long i = 1; i <= pBodyParts->Count; i++) {
				CDOSYS::IBodyPartPtr pBodyPart = pBodyParts->Item[i];

				if (!pBodyPart) 
					continue;

				CString strMediaType = (LPCTSTR)pBodyPart->ContentMediaType;

				_variant_t varContentID = pBodyPart->Fields->Item[CDOSYS::cdoContentId]->Value;
				if (varContentID.vt != VT_BSTR) 
					continue;

				CString strContentID = VarString(varContentID);

				//TRACE("Item %li: media-type %s, content-id: %s\n", i, strMediaType, strContentID);

				if (strContentID.GetLength() <= 2)
					continue;
				
				CString strCIDReference;
				strCIDReference.Format("cid:%s", strContentID.Mid(1, strContentID.GetLength() - 2));

				if (-1 == strHtmlBody.Find(strCIDReference)) {
					continue;
				}

				CString strExtension;

				if (strMediaType == "image/gif") {
					strExtension = "gif";
				} else if (strMediaType == "image/jpeg") {
					strExtension = "jpg";
				} else if (strMediaType == "text/css") {
					strExtension = "css";
				} else {
					// (a.walling 2012-09-04 16:53) - PLID 52447 - Even if we don't have the mime type, still extract to some file and let MSHTML deal with sniffing the MIME type.
					strExtension = "cdo";
				}
				
				// GetNewUniqueID now in NxSystemUtilitiesLib
				CString strTempFileName;
				strTempFileName.Format("%s%s.%s", strOutputFilePrefix, NewUUID(), strExtension);
				CString strTempFilePath = strOutputPath ^ strTempFileName;

				DeleteFile(strTempFilePath);
				pBodyPart->SaveToFile((LPCTSTR)strTempFilePath);								

				// (a.walling 2007-06-12 17:16) - PLID 26261 - For super-duper-extra-safety, go ahead and delay deletion
				// of this file at reboot. This will help cover us for abnormal program terminations and poweroutages.
				// Can't just have these sensitive files laying around in the temp drive.
				MoveFileEx(strTempFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

				saTempFiles.Add(strTempFilePath);

				long nInstances = strHtmlBody.Replace(strCIDReference, strTempFileName);
			}
		}

		strHTMLFile.Format("%s%s.html", strOutputFilePrefix, NewUUID());
		CString strHTMLFilePath = strOutputPath ^ strHTMLFile;
		CFile fOut;
		// (a.walling 2013-07-10 14:56) - PLID 57474 - specify exclusive
		if (fOut.Open(strHTMLFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
			saTempFiles.Add(strHTMLFilePath);
			fOut.Write((LPCTSTR)strHtmlBody, strHtmlBody.GetLength());
			fOut.Close();
			
			// (a.walling 2007-06-12 17:16) - PLID 26261 - For super-duper-extra-safety, go ahead and delay deletion
			// of this file at reboot. This will help cover us for abnormal program terminations and poweroutages.
			// Can't just have these sensitive files laying around in the temp drive.
			MoveFileEx(strHTMLFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
	}
};
