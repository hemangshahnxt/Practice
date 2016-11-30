// OHIPBatchEditParser.cpp: implementation of the COHIPBatchEditParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OHIPBatchEditParser.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

// (j.jones 2008-07-07 09:15) - PLID 30604 - created

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COHIPBatchEditParser::COHIPBatchEditParser()
{

}

COHIPBatchEditParser::~COHIPBatchEditParser()
{

}


void COHIPBatchEditParser::OutputData(CString &OutputString, CString strNewData) {

	OutputString += strNewData;
}

CString COHIPBatchEditParser::ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim /*= FALSE*/)
{
	//we could just have used CString::Mid() but if the line isn't the proper length,
	//we should handle it more gracefully, just returning ""

	//the specs state the position to start at using 1-based counting,
	//but CString uses 0-based counting, so subtract 1 from nStart
	nStart--;

	long nLineLength = strLine.GetLength();

	if(nStart > nLineLength) {
		ASSERT(FALSE);
		return "";
	}

	if(nStart + nLength > nLineLength) {
		ASSERT(FALSE);
		CString str = strLine.Right(nLineLength - nStart);
		if(!bDoNotTrim) {
			str.TrimLeft();
			str.TrimRight();
		}
	}

	CString str = strLine.Mid(nStart, nLength);

	if(!bDoNotTrim) {
		str.TrimLeft();
		str.TrimRight();
	}

	return str;
}

BOOL COHIPBatchEditParser::ParseFile()
{	
	CWaitCursor pWait;

	try {

		//first get the file
		/*
		CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST);
		if (BrowseFiles.DoModal() == IDCANCEL)
			return FALSE;
		
		m_strFileName = BrowseFiles.GetPathName();
		*/

		// (j.jones 2008-12-17 16:05) - PLID 31900 - This ability is only called from the OHIP Report Manager now,
		// so we no longer prompt for a file.
		CString strFullFilePath = m_strFilePath ^ m_strFileName;
		if(!DoesExist(strFullFilePath)) {
			//return silently, but assert, this should not have been allowed to be called on non-existent file
			ASSERT(FALSE);
			return FALSE;
		}

		CString strOutputFile;
		// (j.jones 2008-12-17 16:29) - PLID 31900 - add the filename to the end of the output name		
		strOutputFile.Format("BatchEditReport_%s.txt", m_strFileName);
		
		strOutputFile = GetNxTempPath() ^ strOutputFile;

		BOOL bIsValidFile = FALSE;	
		
		//open the file for reading
		if(!m_InputFile.Open(strFullFilePath,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The batch edit report input file could not be found or opened.");
			return FALSE;
		}

		if(!m_OutputFile.Open(strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The batch edit report output file could not be created.");
			return FALSE;
		}

		CWaitCursor pWait;

		Log("Importing batch edit report file: " + strFullFilePath);

		CArchive arIn(&m_InputFile, CArchive::load);

		CString strLine;

		//we will track if any rejections were found
		BOOL bHasRejections = FALSE;

		CString strOutputString;
		strOutputString.Format("OHIP Batch Edit Report\r\n"
			"==========================================================================\r\n\r\n");
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());

		while(arIn.ReadString(strLine)) {

			if(strLine == "\r"
				|| strLine == "\n"
				|| strLine == "\r\n") {
				//skip carriage returns
				continue;
			}

			if(strLine.GetLength() < 3) {
				//no valid line is this small
				continue;
			}

			CString strIdentifier = strLine.Left(3);
			CString strRecordID = strIdentifier.Right(1);

			long nRecordID = atoi(strRecordID);

			//if the initial part of the line matches the OHIP specs,
			//assume it is a valid file
			if(strIdentifier.Left(2) == "HB") {				
				bIsValidFile = TRUE;
			}
			else {

				//don't display junk if it's not a real file
				if(!bIsValidFile) {

					//close the file
					arIn.Close();
					m_InputFile.Close();
					m_OutputFile.Close();

					//see if it is a remittance file
					if(strIdentifier.Left(2) == "HR" && nRecordID >= 1 && nRecordID <= 8) {
						
						//it seems to be a remit file
						AfxMessageBox("The file you are trying to parse is not an OHIP batch edit file. "
							"It appears to be an OHIP remittance file, and should be imported through "
							"the Electronic Remittance feature in the Batch Payments tab.");
					}
					// (j.jones 2008-07-07 11:27) - PLID 21968 - see if it is a claims error file
					else if(strIdentifier.Left(2) == "HX") {
						// (j.jones 2008-12-17 15:47) - PLID 31900 - the 'Format Claims Error Report' button has been removed,
						// and you can only get to this code through the Report Manager, which should have not allowed us to
						// parse the wrong report.
						AfxMessageBox("The file you are trying to parse is not an OHIP batch edit file. "
							"It appears to be an OHIP claims error file. Please contact NexTech for assistance.");
					}
					else {
						AfxMessageBox("The file you are trying to parse is not an OHIP batch edit file.");
					}
					return FALSE;
				}
			}

			//track the same bHasRejections value through all records
			ReportRecord(strLine, bHasRejections);

			// (j.jones 2009-02-03 17:50) - PLID 31900 - removed, it was pointless
			//PeekAndPump();
		}

		//leave if it's not a real file
		if(!bIsValidFile) {

			//close the file
			arIn.Close();
			m_InputFile.Close();
			m_OutputFile.Close();

			AfxMessageBox("The file you are trying to parse is not an OHIP batch edit file.");
			return FALSE;
		}

		//close the file
		arIn.Close();
		m_InputFile.Close();
		m_OutputFile.Close();

		// (j.jones 2008-12-17 16:15) - PLID 31900 - after parsing is complete, update the OHIPReportHistoryT record
		// by file name - we act like that's a primary key
		//update the LastImportDate, and also update the FirstImportDate if it is NULL
		ExecuteParamSql("UPDATE OHIPReportHistoryT SET LastImportDate = GetDate(), "
			"FirstImportDate = CASE WHEN FirstImportDate Is Null THEN GetDate() ELSE FirstImportDate END "
			"WHERE FileName = {STRING}", m_strFileName);

		//open the finished file in notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.jones 2010-09-02 15:44) - PLID 40388 - fixed crash caused by passing in "this" as a parent
		HINSTANCE hInst = ShellExecute((HWND)GetDesktopWindow(), NULL, "notepad.exe", ("'" + strOutputFile + "'"), NULL, SW_SHOW);

		return TRUE;

	}NxCatchAll("Error parsing OHIP Batch Edit file.");

	return FALSE;
}

/*
//Format Legend

A - Alphabetic

N - Numeric

X - Alphanumeric

D - Date (YYYYMMDD)

S - Spaces
*/

//the file only has this one record line
void COHIPBatchEditParser::ReportRecord(CString strLine, BOOL &bHasRejections)
{
	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HB'
	
	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '1'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Tech Spec Release Identifier	4 / 3 / X				Always 'V03'

	CString strTechSpec = ParseElement(strLine, 4, 3);

	//Batch Number					7 / 5 / X				A number assigned by ministry

	CString strBatchNumber = ParseElement(strLine, 7, 5);

	//Operator Number				12 / 6 / X				From batch header record

	CString strOperatorNumber = ParseElement(strLine, 12, 6);

	//Batch Create Date				18 / 8 / D				From batch header record, format YYYYMMDD

	CString strBatchCreateDate = ParseElement(strLine, 18, 8);

	//Batch Sequence Number			26 / 4 / X				From batch header record

	CString strBatchSeqNumber = ParseElement(strLine, 24, 4);

	//Micro Start					30 / 11 / X				Assigned by ministry: identifies the first record in a batch,
	//														blank if batch rejected

	CString strMicroStart = ParseElement(strLine, 30, 11);

	//Micro End						41 / 5 / X				Assigned by ministry: identifies the last record in a batch,
	//														blank if batch rejected

	CString strMicroEnd = ParseElement(strLine, 41, 5);

	//Micro Type					46 / 7 / X				Always 'HCP/WCB' or 'RMB'

	CString strMicroType = ParseElement(strLine, 46, 7);

	//Group Number					53 / 4 / X				From batch header record

	CString strGroupNumber = ParseElement(strLine, 53, 4);

	//Provider Number				57 / 6 / X				From batch header record

	CString strProviderNumber = ParseElement(strLine, 57, 6);

	//Number of Claims				63 / 5 / X				Total number of claims in the batch as calculated by the ministry

	CString strNumberOfClaims = ParseElement(strLine, 63, 5);
	long nNumberOfClaims = atoi(strNumberOfClaims);
	
	//Number of Records				68 / 6 / X				Total number of records in the batch as calculated by the ministry

	CString strNumberOfRecords = ParseElement(strLine, 68, 6);
	long nNumberOfRecords = atoi(strNumberOfRecords);

	//Batch Process Date			74 / 8 / D				Date batch was processed by MOH, format YYYYMMDD

	CString strBatchProcessDate = ParseElement(strLine, 74, 8);

	//Edit Message					82 / 40 / X				'BATCH TOTALS' left justified to indicate an accepted batch,
	//														or blank if a sub-total line, or 'R' at position 40 to indicate
	//														a rejected batch, preceded by a reason for the batch rejection.

	CString strEditMessage = ParseElement(strLine, 82, 40);

	if(!strEditMessage.IsEmpty()) {

		if(strEditMessage.Find("BATCH") != 0 && strEditMessage.Right(1) == "R") {
			bHasRejections = TRUE;
		}
	}

	//Reserved for MOH Use			122 / 11 / X			Spaces

	CString strReserved = ParseElement(strLine, 122, 11);	

	// (j.jones 2008-12-12 09:13) - PLID 32418 - output the results in a function, instead of doing it while we read it in
	FormatAndWriteData(strBatchNumber, strOperatorNumber, strBatchCreateDate,
		strBatchSeqNumber, strMicroType, strGroupNumber, strProviderNumber,
		nNumberOfClaims, nNumberOfRecords, strBatchProcessDate, strEditMessage,
		bHasRejections);
}

// (j.jones 2008-12-12 09:13) - PLID 32418 - FormatAndWriteData will take in all the pertinent data from
// ReportRecord, and output it in a nicely formatted layout
void COHIPBatchEditParser::FormatAndWriteData(CString strBatchNumber, CString strOperatorNumber, CString strBatchCreateDate,
											  CString strBatchSeqNumber, CString strMicroType, CString strGroupNumber, CString strProviderNumber,
											  long nNumberOfClaims, long nNumberOfRecords, CString strBatchProcessDate, CString strEditMessage,
											  BOOL bHasRejections)
{
	//pass exceptions to the caller

	CString strOutputString, str;

	CString strDateLine;
	if(strBatchCreateDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strBatchCreateDate.Left(4)), atoi(strBatchCreateDate.Mid(4, 2)), atoi(strBatchCreateDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			str.Format("Batch Create Date: %s", FormatDateTimeForInterface(dt, NULL, dtoDate));			
			strDateLine += str;
		}
	}

	if(strBatchProcessDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strBatchProcessDate.Left(4)), atoi(strBatchProcessDate.Mid(4, 2)), atoi(strBatchProcessDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			str.Format("Batch Process Date: %s", FormatDateTimeForInterface(dt, NULL, dtoDate));
			if(!strDateLine.IsEmpty()) {
				strDateLine += "\t\t";
			}
			strDateLine += str;
		}
	}

	if(!strDateLine.IsEmpty()) {
		strDateLine += "\r\n";
		OutputData(strOutputString, strDateLine);
	}

	str.Format("Batch Number: %s\t\tOperator Number: %s\r\n", strBatchNumber, strOperatorNumber);
	OutputData(strOutputString, str);

	str.Format("Batch Sequence Number: %s\tBatch Type: %s\r\n", strBatchSeqNumber, strMicroType);
	OutputData(strOutputString, str);

	str.Format("Group Number: %s\t\tProvider Number: %s\r\n", strGroupNumber, strProviderNumber);
	OutputData(strOutputString, str);

	CString strTotalsLine;
	strTotalsLine.Format("Total Claims In Batch: %li", nNumberOfClaims);

	if(nNumberOfRecords > 0) {
		str.Format("\t\tTotal Records In Batch: %li", nNumberOfRecords);
		strTotalsLine += str;
	}

	strTotalsLine += "\r\n";
	OutputData(strOutputString, strTotalsLine);

	if(!strEditMessage.IsEmpty()) {

		if(bHasRejections) {
			str.Format("\r\n***Reject Message: %s\r\n", strEditMessage);
			OutputData(strOutputString, str);
		}
		else {
			str.Format("\r\nMessage: %s\r\n", strEditMessage);
			OutputData(strOutputString, str);
		}
	}

	OutputData(strOutputString, "\r\n==========================================================================\r\n\r\n");

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}