// OHIPBatchEditParser.h: interface for the COHIPBatchEditParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OHIPBATCHEDITPARSER_H__A0A061E7_80E6_48A1_8CFB_B907E60EAF70__INCLUDED_)
#define AFX_OHIPBATCHEDITPARSER_H__A0A061E7_80E6_48A1_8CFB_B907E60EAF70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (j.jones 2008-07-07 09:15) - PLID 30604 - created

class COHIPBatchEditParser  
{
public:
	COHIPBatchEditParser();
	virtual ~COHIPBatchEditParser();

	BOOL ParseFile();

	//stores the file name for later manipulation
	CString m_strFileName;
	// (j.jones 2008-12-17 16:28) - PLID 31900 - added File Path
	CString m_strFilePath;

private:

	CFile m_InputFile, m_OutputFile;
	void OutputData(CString &OutputString, CString strNewData);
	CString ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim = FALSE);

	void ReportRecord(CString strLine, BOOL &bHasRejections);		//the file only has this one record line

	// (j.jones 2008-12-12 09:13) - PLID 32418 - FormatAndWriteData will take in all the pertinent data from
	// ReportRecord, and output it in a nicely formatted layout
	void FormatAndWriteData(CString strBatchNumber, CString strOperatorNumber, CString strBatchCreateDate,
		CString strBatchSeqNumber, CString strMicroType, CString strGroupNumber, CString strProviderNumber,
		long nNumberOfClaims, long nNumberOfRecords, CString strBatchProcessDate, CString strEditMessage,
		BOOL bHasRejections);
};

#endif // !defined(AFX_OHIPBATCHEDITPARSER_H__A0A061E7_80E6_48A1_8CFB_B907E60EAF70__INCLUDED_)
