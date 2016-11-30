#include "stdafx.h"
#include "nxexception.h"
#include "nxrapi.h"

CNxRAPI::CNxRAPI()
{
	LPTSTR pPath = new TCHAR[MAX_PATH + 10];
	GetSystemDirectory(pPath, MAX_PATH);
	CString str(pPath);
	str += "\\rapi.dll";
	m_hInst = LoadLibrary(str);
	if (!m_hInst)
		return;

	CeRapiInitEx      =		(pfnFunc12) GetProcAddress(m_hInst, "CeRapiInitEx");
	CeRapiUninit    =		(FARPROC) GetProcAddress(m_hInst, "CeRapiUninit");
	CeCreateFile    =		(pfnFunc0)GetProcAddress(m_hInst, "CeCreateFile");
	CeWriteFile     =		(pfnFunc1)GetProcAddress(m_hInst, "CeWriteFile");
	CeCloseHandle   =		(pfnFunc2)GetProcAddress(m_hInst, "CeCloseHandle");
	CeFindFirstFile =		(pfnFunc3)GetProcAddress(m_hInst, "CeFindFirstFile");
	CeGetFileSize	=		(pfnFunc4)GetProcAddress(m_hInst, "CeGetFileSize");
	CeReadFile		=		(pfnFunc5)GetProcAddress(m_hInst, "CeReadFile");
	CeFindNextFile  = 		(pfnFunc6)GetProcAddress(m_hInst, "CeFindNextFile");
	CeCreateDirectory =		(pfnFunc7)GetProcAddress(m_hInst, "CeCreateDirectory");
	CeCreateProcess  =		(pfnFunc8)GetProcAddress(m_hInst, "CeCreateProcess");
	CeGetSystemInfo  =		(pfnFunc9)GetProcAddress(m_hInst, "CeGetSystemInfo");
	CeFindAllFiles =		(pfnFunc10)GetProcAddress(m_hInst, "CeFindAllFiles");
	CeRapiFreeBuffer =		(pfnFunc11)GetProcAddress(m_hInst, "CeRapiFreeBuffer");
	CeDeleteFile =			(pfnFunc13)GetProcAddress(m_hInst, "CeDeleteFile");
	CeRapiGetError =		(pfnFunc14)GetProcAddress(m_hInst, "CeRapiGetError");
	CeGetLastError =		(pfnFunc15)GetProcAddress(m_hInst, "CeGetLastError");
}

CNxRAPI::~CNxRAPI()
{
	if (m_hInst)
		FreeLibrary(m_hInst);
}

BOOL CNxRAPI::IsValid()
{
	return (m_hInst != NULL);
}

HRESULT CNxRAPI::RapiInit()
{
	RAPIINIT ri = { sizeof(RAPIINIT) };
	HRESULT hRes = CeRapiInitEx(&ri);
	DWORD dwRet = WaitForSingleObject(ri.heRapiInit, 10000);

	if ((dwRet != WAIT_OBJECT_0) || !SUCCEEDED(ri.hrRapiInit))
	{
		// Could not initialize Rapi
		CeRapiUninit();
		return -1;
	}
	return S_OK;
}

HRESULT CNxRAPI::CopyPCToCE(const CString& strFileNamePC, const CString& strFileNameCE)
{
	if (!IsValid())
		AfxThrowNxException("Practice could not interface with your PDA. Make sure Microsoft ActiveSync is properly installed.");

	try {
		CFile filePC(strFileNamePC, CFile::modeRead | CFile::typeBinary | CFile::shareCompat);
		CWaitCursor wc;
		char* pcTemp;
		HANDLE hCEFile;
		DWORD nBytesRead, nBytesWritten;

		// Initialize the connection with the CE device
		if (S_OK != RapiInit())
		{
			AfxThrowNxException("Practice could not initialize a connection with your PDA. Make sure Microsoft ActiveSync is properly installed.");
		}

		// Create the file on the CE device
		BSTR bstr = strFileNameCE.AllocSysString(); 
		if (INVALID_HANDLE_VALUE == (hCEFile = CeCreateFile(bstr, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
		{
			AfxThrowNxException("Practice could not create the output file on your PDA. Please ensure it is properly connected to your computer, and that you have permission to create files on your PDA.");
		}

		// Read and write from one file to the other
		if (!(pcTemp = new char[32768]))
		{
			AfxThrowNxException("CopyPCToCE: Failed to create output buffer");
		}
		while((nBytesRead = filePC.Read(pcTemp, 32768)) > 0)
		{
			CeWriteFile(hCEFile, pcTemp, (DWORD)nBytesRead, &nBytesWritten, NULL);
			if (nBytesRead != nBytesWritten)
			{
				AfxThrowNxException("Practice could not write to the output file on your PDA. Please ensure it is properly connected to your computer, and that you have permission to write file information to your PDA.");
			}
		}

		// Free our transfer buffer
		delete pcTemp;

		// Close the files
		CeCloseHandle(hCEFile);
		SysFreeString(bstr);
		filePC.Close();

		// Disconnect from the CE device
		CeRapiUninit();
		return 0;
	}
	catch (CFileException* e)
	{
		char szErr[512];
		e->GetErrorMessage(szErr, 512);
		e->Delete();
		AfxThrowNxException("CopyPCToCE: %s", szErr);
	}
	catch (CNxException* e)
	{
		char sz[512];
		e->GetErrorMessage(sz, 512);
		AfxThrowNxException(sz);
	}
	return -1;
}

HRESULT CNxRAPI::CopyCEToPC(const CString& strFileNameCE, const CString& strFileNamePC)
{
	if (!IsValid())
		AfxThrowNxException("Practice could not interface with your PDA. Make sure Microsoft ActiveSync is properly installed.");

	try {
		CWaitCursor wc;
		char* pcTemp;
		HANDLE hCEFile;
		DWORD nBytesRead;

		// Initialize the connection with the CE device
		if (S_OK != RapiInit())
		{
			AfxThrowNxException("Practice could not initialize a connection with your PDA. Make sure Microsoft ActiveSync is properly installed.");
		}

		// Open the file on the CE device
		BSTR bstr = strFileNameCE.AllocSysString(); 
		if (INVALID_HANDLE_VALUE == (hCEFile = CeCreateFile(bstr, GENERIC_READ, 0, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
		{
			//DRT 7/2/03 - We know that this error means the file doesn't exist on the ce device.  However, our
			//		crappy browse dialog lets us choose invalid files.  So if we get this error, we need to give
			//		an appropriate error message, and return failure.
			//	When we make that browse dialog not suck (PLID 8631), then this should be returned to its 
			//		exception state.
			//AfxThrowNxException("CopyCEToPC: Failed to open '%s' on the Windows CE-based device", strFileNameCE);
			MsgBox("The file specified was unable to be found on your PDA. Please make sure your PDA is connected to your computer, and that the chosen file is valid.");
			return S_FALSE;
		}

		// Read and write from one file to the other
		if (!(pcTemp = new char[32768]))
		{
			AfxThrowNxException("CopyCEToPC: Failed to create output buffer");
		}

		CFile filePC(strFileNamePC, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareCompat);
		do {
			if (!CeReadFile(hCEFile, pcTemp, 32768, &nBytesRead, NULL))
			{
				//DRT 7/2/03 - If we're throwing an exception, we need to delete the file we created
				filePC.Close();
				CFile::Remove(strFileNamePC);
				AfxThrowNxException("Practice could not read the input file from your PDA. Please ensure your PDA is properly connected to your computer, and that you have permission to read files from your PDA.");
			}
			if (nBytesRead)
			{
				filePC.Write(pcTemp, nBytesRead);
			}
		} while (nBytesRead);

		// Free our transfer buffer
		delete pcTemp;

		// Close the files
		CeCloseHandle(hCEFile);
		SysFreeString(bstr);
		filePC.Close();

		// Disconnect from the CE device
		CeRapiUninit();
		return S_OK;
	}
	catch (CFileException* e)
	{
		char szErr[512];
		e->GetErrorMessage(szErr, 512);
		e->Delete();
		AfxThrowNxException("CopyPCToCE: %s", szErr);
	}
	catch (CNxException* e)
	{
		char sz[512];
		e->GetErrorMessage(sz, 512);
		AfxThrowNxException(sz);
	}
	return -1;
}
