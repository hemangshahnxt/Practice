// OHIPClaimsErrorParser.h: interface for the COHIPClaimsErrorParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OHIPCLAIMSERRORPARSER_H__6422C641_DDCE_4401_959C_FA94CE7BC7C1__INCLUDED_)
#define AFX_OHIPCLAIMSERRORPARSER_H__6422C641_DDCE_4401_959C_FA94CE7BC7C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (j.jones 2008-07-07 11:18) - PLID 21968 - created

class COHIPClaimsErrorParser  
{
public:
	COHIPClaimsErrorParser();
	virtual ~COHIPClaimsErrorParser();

	BOOL ParseFile();

	//stores the file name for later manipulation
	CString m_strFileName;
	// (j.jones 2008-12-17 16:28) - PLID 31900 - added File Path
	CString m_strFilePath;

private:

	// (j.jones 2009-10-09 12:26) - PLID 35904 - stores the temp table in which we will generate patient health numbers
	CString m_strHealthNumbersT;

	CFile m_InputFile, m_OutputFile;
	void OutputData(CString &OutputString, CString strNewData);
	CString ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim = FALSE);
	COleCurrency ParseOHIPCurrency(CString strAmount, CString strSign);

	void HX1_Header(CString strLine);			//HX1 - Group/Provider Header Record
	void HXH_ClaimsHeader1(CString strLine);	//HXH - Claims Header 1 Record
	void HXR_ClaimsHeader2(CString strLine);	//HXR - Claims Header 2 Record (RMB Claims Only)
	void HXT_ClaimItem(CString strLine);		//HXT - Claim Item Record
	void HX8_ExplanCode(CString strLine);		//HX8 - Explan Code Message Record (Optional)
	void HX9_Trailer(CString strLine);			//HX9 - Group/Provider Trailer Record

	CString GetErrorDescription(CString strErrorCode);
};

#endif // !defined(AFX_OHIPCLAIMSERRORPARSER_H__6422C641_DDCE_4401_959C_FA94CE7BC7C1__INCLUDED_)
