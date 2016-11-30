// ANSI271Parser.cpp: implementation of the CANSI271Parser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ANSI271Parser.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "DocumentOpener.h"

using namespace ADODB;

// (j.jones 2007-05-01 16:48) - PLID 25868 - created the ANSI 271 Eligibility Response importer

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// (c.haag 2010-11-03 09:42) - PLID 40350 - This is a utility function that will take a repeating
// data element and put all the values into a string array.
void ParseRepeatingElement(IN const CString& strElements, OUT CStringArray& arResults, BOOL bClearArrayFirst)
{
	int i = -1, j = 0;
	if (bClearArrayFirst) {
		arResults.RemoveAll();
	}
	while (-1 != (i = strElements.Find('^', i + 1)))
	{
		CString strResult = strElements.Mid(j, i-j);
		arResults.Add(strResult);
		j = i + 1;
	}

	CString strResult = strElements.Mid(j);
	arResults.Add(strResult);
}

CANSI271Parser::CANSI271Parser()
{
	m_strLastOutput = "";
	m_strLoopNumber = "";
	m_bLastNM1IsInsuranceCompany = false;
	m_bCurLoop2120 = false;

	m_chSegmentTerminator = '~';
	m_chElementTerminator = '*';
	m_chCompositeSeparator = ':';

	m_strOutputIndent = "";

	m_hllCurHLLevel = hllInvalid;

	m_bBenefitTypesFilled = FALSE;
	m_bCoverageLevelsFilled = FALSE;
	m_bServiceTypesFilled = FALSE;
}

CANSI271Parser::~CANSI271Parser()
{
	ClearResponseInfo();
}

void CANSI271Parser::ClearResponseInfo() {

	try {

		//clear out the response and all its details

		for(int i=m_paryResponses.GetSize()-1;i>=0;i--) {

			EligibilityResponseMasterInfo *pMaster = (EligibilityResponseMasterInfo*)m_paryResponses.GetAt(i);

			for(int j=pMaster->arypResponses.GetSize()-1;j>=0;j--) {
				EligibilityResponseDetailInfo *pDetail = (EligibilityResponseDetailInfo*)pMaster->arypResponses.GetAt(j);

				// (j.jones 2010-03-24 17:14) - PLID 37870 - added an array of the benefit information returned
				for(int k=pDetail->arypBenefitDetails.GetSize()-1;k>=0;k--) {
					BenefitDetail *pBenefitDetail = (BenefitDetail*)pDetail->arypBenefitDetails.GetAt(k);
					delete pBenefitDetail;
					pDetail->arypBenefitDetails.RemoveAt(k);
				}

				delete pDetail;
				pMaster->arypResponses.RemoveAt(j);
			}

			delete pMaster;
			m_paryResponses.RemoveAt(i);
		}

		m_strFileName = "";

		//we do not need to clear the maps of EligibilityDataReferenceT data, they are static per Practice release

	}NxCatchAll("Error cleaning up eligibility response.");
}

EligibilityResponseMasterInfo* CANSI271Parser::GetCurrentResponseMasterInfo()
{
	if(m_paryResponses.GetSize() == 0) {
		ASSERT(FALSE);
		return NULL;
	}

	return (EligibilityResponseMasterInfo*)m_paryResponses.GetAt(m_paryResponses.GetSize()-1);
}

EligibilityResponseDetailInfo* CANSI271Parser::GetCurrentResponseDetailInfo()
{
	EligibilityResponseMasterInfo *pMaster = GetCurrentResponseMasterInfo();

	if(pMaster == NULL) {
		ASSERT(FALSE);
		return NULL;
	}

	if(pMaster->arypResponses.GetSize() == 0) {
		//this is permitted
		return NULL;
	}

	return (EligibilityResponseDetailInfo*)pMaster->arypResponses.GetAt(pMaster->arypResponses.GetSize()-1);
}

// (j.jones 2010-03-25 13:15) - PLID 37870 - added function to return the current benefit detail
BenefitDetail* CANSI271Parser::GetCurrentResponseBenefitDetailInfo()
{
	EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();

	if(pDetail == NULL) {
		//this is permitted
		return NULL;
	}

	if(pDetail->arypBenefitDetails.GetSize() == 0) {
		//this is permitted
		return NULL;
	}

	return (BenefitDetail*)pDetail->arypBenefitDetails.GetAt(pDetail->arypBenefitDetails.GetSize()-1);
}

// (j.jones 2010-07-07 10:24) - PLID 39499 - added aryRequestIDsUpdated, which needs to
// return an array of unique request IDs that we just imported to
//this also fills an array of the new responses we just imported
BOOL CANSI271Parser::ParseFile(OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned)
{
	CWaitCursor pWait;

	CFile fInputFile;
	BOOL bInputFileOpen = FALSE;
	BOOL bOutputFileOpen = FALSE;
	
	try {

		//first get the file
		CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,"All Files (*.*)|*.*|");
		if (BrowseFiles.DoModal() == IDCANCEL)
			return FALSE;

		m_strFileName = BrowseFiles.GetPathName();
		CString strOutputFile = GetNxTempPath() ^ "Eligibility Response.txt";

		BOOL bIsValidFile = FALSE;
		
		//open the file for reading
		if(!fInputFile.Open(m_strFileName,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The input file could not be found or opened.");
			return FALSE;
		}
		bInputFileOpen = TRUE;

		if(!m_OutputFile.Open(strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The output file could not be created.");
			if(bInputFileOpen)
				fInputFile.Close();
			return FALSE;
		}
		bOutputFileOpen = TRUE;

		CWaitCursor pWait;

		//reset all values
		m_strLastOutput = "";
		m_strLoopNumber = "";
		m_bLastNM1IsInsuranceCompany = false;
		m_bCurLoop2120 = false;

		m_chSegmentTerminator = '~';
		m_chElementTerminator = '*';
		m_chCompositeSeparator = ':';

		m_strOutputIndent = "";

		Log("Importing eligibility response file: %s", m_strFileName);

		// (j.jones 2010-04-19 10:25) - PLID 38202 - supported option to exclude from the output file
		m_bSuppressExcludedCoverage = (GetRemotePropertyInt("Eligibility_ExcludeFromOutputFile", 0, 0, "<None>", true) == 1);

		const int LEN_16_KB = 16384;
		CString strIn;	//input string
		long iFileSize = fInputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
		strIn.ReleaseBuffer(iFileSize);

		CString strLastSegment = "";	//stores data between reads

		if (iFileSize > 0) {
			// (j.jones 2010-03-02 09:16) - PLID 37589 - This should only be done once per file
			// rather than every segment.

			strIn.Replace("\r","");
			strIn.Replace("\n","");

			// (j.jones 2006-07-25 16:55) - PLID 21606 - Supporting non-standard terminators
			// by determing the character before the GS segment.
			int nIndex = strIn.Find("GS*");
			if(nIndex != -1) {
				CString strSegmentTerminator = strIn.Mid(nIndex-1,1);
				if(strSegmentTerminator.GetLength() > 0)
					m_chSegmentTerminator = strSegmentTerminator.GetAt(0);
			}
		}

		while(iFileSize > 0) {

			// (j.jones 2010-07-02 13:20) - PLID 39499 - call ParseContent with our contents
			ParseContent(strIn, bIsValidFile);
			if(!bIsValidFile) {
				//ParseContent determined that this is not a valid file, and would have explained why not
				if(bInputFileOpen)
					fInputFile.Close();
				if(bOutputFileOpen)
					m_OutputFile.Close();
				return FALSE;
			}

			PeekAndPump();

			strLastSegment = strIn;

			iFileSize = fInputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);

			strIn = strLastSegment += strIn;
		}

		//close the file
		if(bInputFileOpen)
			fInputFile.Close();
		if(bOutputFileOpen)
			m_OutputFile.Close();

		bInputFileOpen = FALSE;
		bOutputFileOpen = FALSE;

		//don't display junk if it's not a real file
		if(!bIsValidFile) {
			AfxMessageBox("The file you are trying to parse is not an ANSI 271 eligibility response file.");
			return FALSE;
		}

		// (j.jones 2010-07-02 13:52) - PLID 39499 - all actual importing to Practice data is now done inside this class
		BOOL bMustOpenNotepadFile = FALSE; //not used when parsing from a file
		BOOL bSuccess = ImportParsedContentToData(bMustOpenNotepadFile, FALSE, aryRequestIDsUpdated, aryResponseIDsReturned);

		//open the finished file in notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		ShellExecute(GetDesktopWindow(), NULL, "notepad.exe", ("'" + strOutputFile + "'"), NULL, SW_SHOW);

		return bSuccess;

	} NxCatchAll("Error in CANSI271Parser::ParseFile");

	if(bInputFileOpen)
		fInputFile.Close();
	if(bOutputFileOpen)
		m_OutputFile.Close();

	return FALSE;
}

// (j.jones 2010-07-02 13:29) - PLID 39499 - launches the parser from a text response
// rather than browsing for a file
// (j.jones 2010-07-07 10:14) - PLID 39499 - added aryRequestIDsUpdated, which needs to
// return an array of unique request IDs that we just imported to
// (j.jones 2010-11-05 09:31) - PLID 41341 - the array of responses is now the array of requests
// that received responses, response text is tracked inside the request
// (r.goldschmidt 2015-11-12 12:55) - PLID 65363 - add bNotepadWasOpened so we know if notepad was opened
//this also fills an array of the new responses we just imported
BOOL CANSI271Parser::ParseRealTimeResponses(CArray<EligibilityInfo*, EligibilityInfo*> &aryRequestsWithResponses,
	OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned, OUT bool &bNotepadWasOpened)
{
	//this function parses text responses of generic 271 content, but is specific
	//to our real-time implementation, using some specific settings

	CWaitCursor pWait;

	BOOL bOutputFileOpen = FALSE;
	
	try {

		CString strOutputFile = GetNxTempPath() ^ "Eligibility Response.txt";

		BOOL bIsValidFile = FALSE;
		
		if(!m_OutputFile.Open(strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The output file could not be created.");
			return FALSE;
		}
		bOutputFileOpen = TRUE;

		CWaitCursor pWait;

		//reset all values
		m_strLastOutput = "";
		m_strLoopNumber = "";
		m_bLastNM1IsInsuranceCompany = false;
		m_bCurLoop2120 = false;

		m_chSegmentTerminator = '~';
		m_chElementTerminator = '*';
		m_chCompositeSeparator = ':';

		m_strOutputIndent = "";

		// (j.jones 2010-04-19 10:25) - PLID 38202 - supported option to exclude from the output file
		m_bSuppressExcludedCoverage = (GetRemotePropertyInt("Eligibility_ExcludeFromOutputFile", 0, 0, "<None>", true) == 1);

		CString strLastSegment = "";	//stores data between reads

		for(int i=0;i<aryRequestsWithResponses.GetSize();i++) {

			// (j.jones 2010-11-05 09:36) - PLID 41341 - we are now given an array of requests that
			// had responses received, the response is in the request pointer
			EligibilityInfo *pRequestInfo = (EligibilityInfo*)aryRequestsWithResponses.GetAt(i);

			CString strIn = pRequestInfo->strRealTime271Response;

			if(strIn.IsEmpty()) {
				//just assert & continue, it should not be possible
				//for an empty response to be in this array
				ASSERT(FALSE);
				continue;
			}

			strIn.Replace("\r","");
			strIn.Replace("\n","");

			// (j.jones 2006-07-25 16:55) - PLID 21606 - Supporting non-standard terminators
			// by determing the character before the GS segment.
			int nIndex = strIn.Find("GS*");
			if(nIndex != -1) {
				CString strSegmentTerminator = strIn.Mid(nIndex-1,1);
				if(strSegmentTerminator.GetLength() > 0)
					m_chSegmentTerminator = strSegmentTerminator.GetAt(0);
			}

			// (j.jones 2010-07-02 13:20) - PLID 39499 - call ParseContent with our contents			
			// (j.jones 2010-11-05 09:49) - PLID 41341 - pass in our request ID
			ParseContent(strIn, bIsValidFile, pRequestInfo->nID);

			//output blank lines between each result
			CString strOut;
			strOut = "\r\n\r\n";
			m_OutputFile.Write(strOut,strOut.GetLength());

			if(!bIsValidFile) {
				//ParseContent determined that this is not valid content, and would have explained why not
				if(bOutputFileOpen)
					m_OutputFile.Close();
				return FALSE;
			}

			PeekAndPump();
		}

		//close the file
		if(bOutputFileOpen)
			m_OutputFile.Close();

		bOutputFileOpen = FALSE;

		//don't display junk if it's not a real file
		if(!bIsValidFile) {
			AfxMessageBox("The data you are trying to import is not an ANSI 271 eligibility response.");
			return FALSE;
		}

		// (j.jones 2010-07-02 13:52) - PLID 39499 - all actual importing to Practice data is now done inside this class
		BOOL bMustOpenNotepadFile = FALSE; //not used when parsing from a file
		BOOL bSuccess = ImportParsedContentToData(bMustOpenNotepadFile, TRUE, aryRequestIDsUpdated, aryResponseIDsReturned);

		//if the preference says so, OR if bMustOpenNotepadFile = TRUE, open it
		//(this preference is cached by the caller)
		BOOL bPrefOpenNotepad = (GetRemotePropertyInt("GEDIEligibilityRealTime_OpenNotepad", 1, 0, "<None>", true) == 1);
		bNotepadWasOpened = (bPrefOpenNotepad || bMustOpenNotepadFile); // ... notepad is _about_ to open
		if(bNotepadWasOpened) {
			//open the finished file in notepad
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			ShellExecute(GetDesktopWindow(), NULL, "notepad.exe", ("'" + strOutputFile + "'"), NULL, SW_SHOW);
		}

		return bSuccess;

	} NxCatchAll("Error in CANSI271Parser::ParseRealTimeResponses");

	if(bOutputFileOpen)
		m_OutputFile.Close();

	return FALSE;
}

// (j.jones 2010-07-02 12:46) - PLID 39499 - moved the parsing into a new ParseContent function,
// used by both ParseFile and ParseTextResponse
// (j.jones 2010-11-05 09:49) - PLID 41341 - pass in our request ID, we only know it if
// we are in a realtime import, as that is the only time the parsing would have only one response
void CANSI271Parser::ParseContent(IN OUT CString &strIn, BOOL &bIsValidFile, long nRealTimeRequestID /*= -1*/)
{
	CString strSegment,	//current segment
			strElement;	//current element

	strIn.Replace("\r","");
	strIn.Replace("\n","");

	// (j.jones 2010-11-05 10:47) - PLID 40914 - we need to know if this
	// isn't an ANSI file at all, and warn accordingly
	BOOL bIsANSI = FALSE;

	while((strIn.GetLength() > 0) && (strIn.Find(m_chSegmentTerminator) != -1)) {

		//parse the first segment
		strSegment = ParseSegment(strIn);

		//parse the first element, the identifier, into strIdent
		CString strIdent = ParseElement(strSegment);

		//now call the right function, and send the remainder of that segment
		if(strIdent == "ISA")
			ANSI_ISA(strSegment);
		else if(strIdent == "TA1")
			ANSI_TA1(strSegment);
		else if(strIdent == "GS")
			ANSI_GS(strSegment);
		else if(strIdent == "ST") {
			//we know it's ANSI at this point
			bIsANSI = TRUE;
			bIsValidFile = ANSI_ST(strSegment);
			if(!bIsValidFile) {
				return;
			}
		}
		else if(strIdent == "SE")
			ANSI_SE(strSegment);
		else if(strIdent == "GE")
			ANSI_GE(strSegment);
		else if(strIdent == "IEA")
			ANSI_IEA(strSegment);
		
		//271-specific fields
		else if(strIdent == "BHT")
			ANSI_BHT(strSegment);
		else if(strIdent == "HL")
			ANSI_HL(strSegment);
		else if(strIdent == "AAA")
			ANSI_AAA(strSegment);
		else if(strIdent == "NM1")
			ANSI_NM1(strSegment);
		else if(strIdent == "REF")
			ANSI_REF(strSegment);
		else if(strIdent == "PER")
			ANSI_PER(strSegment);
		else if(strIdent == "TRN")
			ANSI_TRN(strSegment);
		else if(strIdent == "N3")
			ANSI_N3(strSegment);
		else if(strIdent == "N4")
			ANSI_N4(strSegment);
		else if(strIdent == "DMG")
			ANSI_DMG(strSegment);
		else if(strIdent == "INS")
			ANSI_INS(strSegment);
		// (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment HI
		else if(strIdent == "HI")
			ANSI_HI(strSegment);
		else if(strIdent == "DTP")
			ANSI_DTP(strSegment);
		// (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment MPI
		else if(strIdent == "MPI")			
			ANSI_MPI(strSegment);
		else if(strIdent == "EB")
			ANSI_EB(strSegment);
		else if(strIdent == "HSD")
			ANSI_HSD(strSegment);
		else if(strIdent == "MSG")
			ANSI_MSG(strSegment);
		else if(strIdent == "III")
			ANSI_III(strSegment);				
		else if(strIdent == "PRV")
			ANSI_PRV(strSegment);
		else if(strIdent == "LS")
			ANSI_LS(strSegment);
		else if(strIdent == "LE")
			ANSI_LE(strSegment);
		else {
//#ifdef _DEBUG
			//AfxMessageBox(strIdent);
//#endif
		}
	}

	// (j.jones 2010-11-05 10:47) - PLID 40914 - warn that the file is invalid
	if(!bIsANSI && !bIsValidFile) {
		AfxMessageBox("The data you are trying to import is not an ANSI 271 eligibility response.");
		return;
	}


	// (j.jones 2010-11-05 09:54) - PLID 41341 - if this is a real-time send,
	// the response is linked to one and only one request, and if for some 
	// reason the trace lookup failed, we still know which request has this
	// real-time response
	if(bIsValidFile && nRealTimeRequestID != -1) {
		EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
		if(pDetail) {

			//do we already have this filled in?
			if(pDetail->nEligibilityRequestID == -1) {

				//shady, but oh well, we can still fix it,
				//just link and log (wow that could be a bad pun)
				pDetail->nEligibilityRequestID = nRealTimeRequestID;

				Log("**** Request ID %li did not get a valid trace response. Matched by real-time send.", nRealTimeRequestID);
			}
		}
	}
}

void CANSI271Parser::OutputData(CString &OutputString, CString strNewData) {

	//insert the indentation after any newlines
	CString strDataToInsert = "\r\n" + m_strOutputIndent;
	strNewData.Replace("\r\n", strDataToInsert);

	//store this data
	m_strLastOutput = strNewData;

	//now output
	OutputString += strNewData;
}

CString CANSI271Parser::ParseSection(CString &strIn, char chDelimiter) {

	//the same code is used for ParseSegment/Element/Composite, only the character changes

	CString strOut;

	//if we are in the middle of the file
	if(strIn.Find(chDelimiter) != -1) {
		strOut = strIn.Left(strIn.Find(chDelimiter));		
		strIn = strIn.Right(strIn.GetLength() - strIn.Find(chDelimiter) - 1);	
	}
	//else if we are on the last segment
	else if(strIn.GetLength() > 0) {
		strOut = strIn;
		strIn = "";
	}

	return strOut;
}

CString CANSI271Parser::ParseSegment(CString &strIn) {

	return ParseSection(strIn, m_chSegmentTerminator);
}

CString CANSI271Parser::ParseElement(CString &strIn) {

	return ParseSection(strIn, m_chElementTerminator);
}

CString CANSI271Parser::ParseComposite(CString &strIn) {

	return ParseSection(strIn, m_chCompositeSeparator);
}

CString CANSI271Parser::FixCapitalization(CString strCapitalized)
{
	CString strInit = "", strFixed = "";
	TCHAR chTmp = 0;
	_variant_t varReturn;
	bool bNextUpper = true;
	strInit = strCapitalized;
	
	for( int i = 0; i < strInit.GetLength(); i++){
		chTmp = strInit.GetAt(i);
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - ambiguous CString::+= operator
		if(bNextUpper) //This will be true the first time
			strFixed += (char)toupper(strInit.GetAt(i));
		else 
			strFixed += (char)::tolower(strInit.GetAt(i));

		if( chTmp == ' ')
			bNextUpper = true;
		else 
			bNextUpper = false;
	}

	strFixed.TrimLeft();
	strFixed.TrimRight();
	return strFixed;
}

CString CANSI271Parser::ConvertDateFormat(CString strCCYYMMDD)
{
	CString strDate = "Invalid Date";

	if(strCCYYMMDD.GetLength() == 6) {
		//format YYMMDD into MM/DD/CCYY
		strDate = strCCYYMMDD.Right(4) + "/20" + strCCYYMMDD.Left(2);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	else if(strCCYYMMDD.GetLength() == 8) {
		//format CCYYMMDD into MM/DD/CCYY
		strDate = strCCYYMMDD.Right(4) + "/" + strCCYYMMDD.Left(4);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}

	return strDate;
}

CString CANSI271Parser::ConvertTimeFormat(CString strHHMMSS)
{
	CString strTime = "Invalid Time";

	if(strHHMMSS.GetLength() == 4) {
		//format HHMM into HH:MM
		strTime = strHHMMSS.Left(2) + ":" + strHHMMSS.Right(2);
	}
	else if(strHHMMSS.GetLength() >= 6) {
		//format HHMMSS into HH:MM (also works for HHMMSSDD)
		strHHMMSS = strHHMMSS.Left(4);
		strTime = strHHMMSS.Left(2) + ":" + strHHMMSS.Right(2);
	}

	return strTime;
}

BOOL CANSI271Parser::ANSI_ST(CString &strIn) {
	
	//010		ST			Transaction Set Header				M			1

	CString OutputString, str, strFmt;
	
	//Ref.		Data		Name								Attributes
	//Des.		Element

	//ST01		143			Transaction Set Identifier Code		M	ID	3/3

	str = ParseElement(strIn);
	//this should always be 271, if not, give a warning
	if(str != "271") {
		if(str == "277")
			AfxMessageBox("The file you are trying to import is not an ANSI 271 eligibility response file, it is an ANSI 277 response file.\n"
			"Please go to the E-Billing Batch tab and click on the \"Format 277 Report\" button to view this file.");
		else if(str == "997")
			AfxMessageBox("The file you are trying to import is not an ANSI 271 eligibility response file, it is an ANSI 997 acknowledgement file.\n"
				"Please go to the E-Billing Batch tab and click on the \"Format 997 Report\" button to view this file.");
		else if(str == "835")
			AfxMessageBox("The file you are trying to import is not an ANSI 271 eligibility response file, it is an ANSI 835 electronic remittance file.\n"
				"Please go to the Batch Payments tab and click on the \"Electronic Remittance\" button to import this file.");
		else
			AfxMessageBox("The data you are trying to import is not an ANSI 271 eligibility response.");
		return FALSE;
	}

	//ST02		329			Transaction Set Control Number		M	AN	4/9

	str = ParseElement(strIn);
	//this is the batch number
	strFmt.Format("Batch Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());

	//ST03		1705		Implementation Convention Reference	O	AN 1/35
	// (c.haag 2010-10-19 15:23) - PLID 40350 - The version that tells us whether this
	// is ANSI 5010 or not is returned here, but we ignore it now because of our "accept all
	// versions" philosophy.
	// (Note: ANSI 5010 requires it to be a value of "005010X279" or "005010X279A1").
	str = ParseElement(strIn);

	return TRUE;
}

void CANSI271Parser::ANSI_SE(CString &strIn) {

	//080		SE			Transaction Set Trailer				M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element


	//SE01		96			Number Of Included Segments			M	N0	1/10

	//SE02		329			Transaction Set Control Number		M	AN	4/9

	//OutputString = "SE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_ISA(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//ISA01	I01			Authorization Information Qualifier		M	ID	2/2

	str = ParseElement(strIn);
	//we won't need this

	//ISA02	I02			Authorization Information				M	AN	10/10

	str = ParseElement(strIn);
	//we won't need this

	//ISA03	I03			Security Information Qualifier			M	ID	2/2

	str = ParseElement(strIn);
	//we won't need this

	//ISA04	I04			Security Information					M	AN	10/10

	str = ParseElement(strIn);
	//we won't need this

	//ISA05	I05			Interchange ID Qualifier				M	ID	2/2

	str = ParseElement(strIn);
	//we won't need this

	//ISA06	I06			Interchange Sender ID					M	AN	15/15

	str = ParseElement(strIn);
	//we won't need this

	//ISA07	I05			Interchange ID Qualifier				M	ID	2/2

	str = ParseElement(strIn);
	//we won't need this

	//ISA08	I07			Interchange Receiver ID					M	AN	15/15

	str = ParseElement(strIn);
	//we won't need this

	//ISA09	I08			Interchange Date						M	DT	6/6

	str = ParseElement(strIn);
	//processing date

	CString strDate = ConvertDateFormat(str);	
	strFmt.Format("Processing Date: %s",strDate);
	OutputString = strFmt;

	//ISA10	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//processing time
	
	CString strTime = ConvertTimeFormat(str);
	strFmt.Format(" %s\r\n",strTime);
	OutputData(OutputString, strFmt);

	//ISA11	I10			Interchange Control Standards Ident.	M	ID	1/1
	// (c.haag 2010-10-19 15:43) - PLID 40350 - No change here but in 5010
	// this is a Repetition Separator.

	str = ParseElement(strIn);
	//we won't need this

	//ISA12	I11			Interchange Control Version Number		M	ID	5/5

	str = ParseElement(strIn);
	//we won't need this

	//ISA13	I12			Interchange Control Number				M	N0	9/9

	str = ParseElement(strIn);
	//we won't need this

	//ISA14	I13			Acknowledgement Requested				M	ID	1/1

	str = ParseElement(strIn);
	//we won't need this

	//ISA15	I14			Usage Indicator							M	ID	1/1

	str = ParseElement(strIn);
	//we won't need this

	//ISA16	I15			Component Element Separator				M		1/1

	str = ParseElement(strIn);
	//this is the composite separator
	if(str.GetLength() > 0)
		m_chCompositeSeparator = str.GetAt(0);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_IEA(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

	//IEA02	I12			Interchange Control Number				M	N0	9/9

	//OutputString = "IEA\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_TA1(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//TA101	I12			Interchange Control Number				M	N0	9/9

	str = ParseElement(strIn);	
	strFmt.Format("Interchange Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	//TA102	I08			Interchange Date						M	DT	6/6

	str = ParseElement(strIn);
	//claim date

	CString strDate = ConvertDateFormat(str);
	strFmt.Format("Eligibility Creation Date: %s",strDate);
	OutputData(OutputString, strFmt);

	//TA103	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//claim time
	
	CString strTime = ConvertTimeFormat(str);
	strFmt.Format(" %s\r\n\r\n",strTime);
	OutputData(OutputString, strFmt);

	//TA104	I17			Interchange Acknowledgement Code		M	ID	1/1

	str = ParseElement(strIn);
	//was the interchange header valid?
	if(str == "A")
		//The Transmitted Interchange Control Structure Header and Trailer Have Been Received and Have No Errors.
		strFmt.Format("Interchange Acknowledgement Code A: - The Transmitted Interchange Control Structure Header and Trailer Have Been Received and Have No Errors.\r\n\r\n");
	else if(str == "E")
		//The Transmitted Interchange Control Structure Header and Trailer Have Been Received and And Are Accepted But Errors Are Noted. This Means The Sender Must Not Resend The Data.
		strFmt.Format("Interchange Acknowledgement Code E: - The Transmitted Interchange Control Structure Header and Trailer Have Been Received and And Are Accepted But Errors Are Noted. This Means The Sender Must Not Resend The Data.\r\n\r\n");
	else if(str == "R")
		//The Transmitted Interchange Control Structure Header and Trailer are Rejected Because of Errors.
		strFmt.Format("Interchange Acknowledgement Code R: - The Transmitted Interchange Control Structure Header and Trailer are Rejected Because of Errors.\r\n\r\n");
	OutputData(OutputString, strFmt);

	//TA105	I18			Interchange Note Code					M	ID	3/3

	str = ParseElement(strIn);
	//interchange error code
	if(str == "000") {
		//No error
		strFmt.Format("Interchange Error Code %s: No Error\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "001") {
		//The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement
		strFmt.Format("Interchange Error Code %s: The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "002") {
		//This Standard as Noted in the Control Standards Identifier is Not Supported
		strFmt.Format("Interchange Error Code %s: This Standard as Noted in the Control Standards Identifier is Not Supported\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "003") {
		//This Version of the Controls is Not Supported
		strFmt.Format("Interchange Error Code %s: This Version of the Controls is Not Supported\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "004") {
		//The Segment Terminator is Invalid
		strFmt.Format("Interchange Error Code %s: The Segment Terminator is Invalid\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "005") {
		//Invalid Interchange ID Qualifier for Sender
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Sender\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "006") {
		//Invalid Interchange Sender ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Sender ID\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "007") {
		//Invalid Interchange ID Qualifier for Receiver
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Receiver\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "008") {
		//Invalid Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Receiver ID\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "009") {
		//Unknown Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Unknown Interchange Receiver ID\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "010") {
		//Invalid Authorization Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Qualifier Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "011") {
		//Invalid Authorization Information Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "012") {
		//Invalid Security Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Security Information Qualifier Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "013") {
		//Invalid Security Information
		strFmt.Format("Interchange Error Code %s: Invalid Security Information\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "014") {
		//Invalid Interchange Date Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Date Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "015") {
		//Invalid Interchange Time Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Time Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "016") {
		//Invalid Interchange Standards Identifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Standards Identifier Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "017") {
		//Invalid Interchange Version ID Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Version ID Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "018") {
		//Invalid Interchange Control Number Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control Number Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "019") {
		//Invalid Acknowledgement Requested Value
		strFmt.Format("Interchange Error Code %s: Invalid Acknowledgement Requested Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "020") {
		//Invalid Test Indicator Value
		strFmt.Format("Interchange Error Code %s: Invalid Test Indicator Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "021") {
		//Invalid Number of Included Groups Value
		strFmt.Format("Interchange Error Code %s: Invalid Number of Included Groups Value\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "022") {
		//Invalid Control Structure
		strFmt.Format("Interchange Error Code %s: Invalid Control Structure\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "023") {
		//Improper (Premature) End-Of-File (Transmission)
		strFmt.Format("Interchange Error Code %s: Improper (Premature) End-Of-File (Transmission)\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "024") {
		//Invalid Interchange Control (e.g., Invalid GS Segment)
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control (e.g., Invalid GS Segment)\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "025") {
		//Duplicate Interchange Control Number
		strFmt.Format("Interchange Error Code %s: Duplicate Interchange Control Number\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "026") {
		//Invalid Data Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Data Element Separator\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "027") {
		//Invalid Component Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Component Element Separator\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "028") {
		//Invalid Delivery Date in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Date in Deferred Delivery Request\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "029") {
		//Invalid Delivery Time in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time in Deferred Delivery Request\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "030") {
		//Invalid Delivery Time Code in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time Code in Deferred Delivery Request\r\n",str);
		OutputData(OutputString, strFmt);
	}
	else if(str == "031") {
		//Invalid Grade of Service Code
		strFmt.Format("Interchange Error Code %s: Invalid Grade of Service Code\r\n",str);
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_GS(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GS01	479			Functional Identifier Code				M	ID	2/2

	//GS02	142			Application Sender's Code				M	AN	2/15

	//GS03	124			Application Receiver's Code				M	AN	2/15

	//GS04	373			Date									M	DT	8/8

	//GS05	337			Time									M	TM	4/8

	//GS06	28			Group Control Number					M	N0	1/9

	//GS07	455			Responsible Agency Code					M	ID	1/2

	//GS08	480			Version/Release/Industry Ident. Code	M	AN	1/12
	// (c.haag 2010-10-19 15:43) - PLID 40350 - GS08 Version guide:
	// GS08 ANSI 4010 - 004010X092A1
	// GS08 ANSI 5010 - 005010X279 OR 005010X279A1

	//OutputString = "GS\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_GE(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GE01	97			Number of Transaction Sets Included		M	N0	1/6

	//GE02	28			Group Control Number					M	N0	1/9

	//OutputString = "GE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

///////////////////////////////////////
//Begin the 271-specific fields      //
///////////////////////////////////////

void CANSI271Parser::ANSI_BHT(CString &strIn) {

	//Beginning of Hierarchical Transaction, used in:

	//156		BHT		Beginning of Hierarchical Transaction	R		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//BHT01	1005		Hierarchical Structure Code				M	ID	4/4

	//0022 - Information Source, Information Receiver, Subscriber, Dependent
	str = ParseElement(strIn);

	//BHT02	353			Transaction Set Purpose Code			M	ID	2/2

	//11 - Response
	str = ParseElement(strIn);

	//BHT03	127			Reference Identification				O	AN	1/30

	//Submitter Transaction Identifier
	str = ParseElement(strIn);

	//BHT04	373			Date									O	DT	8/8

	//Transaction Set Creation Date
	str = ParseElement(strIn);

	CString strDate = ConvertDateFormat(str);
	strFmt.Format("Transaction Set Creation Date: %s", strDate);
	OutputData(OutputString, strFmt);

	//BHT05	337			Time									O	TM	4/8

	//Transaction Set Creation Time
	str = ParseElement(strIn);

	CString strTime = ConvertTimeFormat(str);
	strFmt.Format(" %s\r\n", strTime);
	OutputData(OutputString, strFmt);

	//BHT06 NOT USED

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_HL(CString &strIn) {

	//Hierarchical Level, used in:

	//158		HL		Information Source Level				R		1
	//175		HL		Information Receiver Level				R		1
	//187		HL		Subscriber Level						R		1
	//265		HL		Dependent Level							R		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//HL01	628			Hierarchical ID Number					M	AN	1/12

	str = ParseElement(strIn);

	//HL02	734			Hierarchical Parent ID Number			O	AN	1/12

	//not used in Loop 2000A

	str = ParseElement(strIn);

	//HL03	735			Hierarchical Level Code					M	ID	1/2

	//if Loop 2000A:
		//20 - Information Source
	//if Loop 2000B:
		//21 - Information Receiver
	//if Loop 2000C:
		//22 - Subscriber (may or may not be the patient)
	//if Loop 2000D:
		//23 - Dependent
	str = ParseElement(strIn);

	//For each loop, set up the new indentation value, such that all following lines
	//between this and the next loop will be indented accordingly.
	//Then output a header value.

	if(str == "20") {
		m_strOutputIndent = "/  ";
		OutputData(OutputString, "\r\n====================\r\n");
		OutputData(OutputString, " Source Information\r\n");
		OutputData(OutputString, "====================\r\n");

		//if the last segment was patient, subscriber, or invalid, this is
		//a new master info object
		if(m_hllCurHLLevel == hllSubscriber || m_hllCurHLLevel == hllPatient || m_hllCurHLLevel == hllInvalid) {
			EligibilityResponseMasterInfo *pMaster = new EligibilityResponseMasterInfo;
			m_paryResponses.Add(pMaster);
		}

		m_hllCurHLLevel = hllSource;
	}
	else if(str == "21") {
		m_strOutputIndent = "/ /  ";
		OutputData(OutputString, "\r\n======================\r\n");
		OutputData(OutputString, " Receiver Information\r\n");
		OutputData(OutputString, "======================\r\n");

		//if the last segment was patient, subscriber, or invalid, this is
		//a new master info object
		if(m_hllCurHLLevel == hllSubscriber || m_hllCurHLLevel == hllPatient || m_hllCurHLLevel == hllInvalid) {
			EligibilityResponseMasterInfo *pMaster = new EligibilityResponseMasterInfo;
			m_paryResponses.Add(pMaster);
		}

		m_hllCurHLLevel = hllReceiver;
	}
	else if(str == "22") {

		//anytime we get a subscriber HL segment, we have a new response,
		//so create one and add it to our current master info struct
		EligibilityResponseMasterInfo *pMaster = GetCurrentResponseMasterInfo();
		if(pMaster) {
			EligibilityResponseDetailInfo *pDetail = new EligibilityResponseDetailInfo;

			// (j.jones 2009-12-17 16:07) - PLID 36641 - default the detail to have
			// the same confirmation status as the master
			pDetail->ercsStatus = pMaster->ercsStatus;

			pMaster->arypResponses.Add(pDetail);
		}
		
		m_strOutputIndent = "/ / /  ";
		OutputData(OutputString, "\r\n========================\r\n");
		OutputData(OutputString, " Subscriber Information\r\n");
		OutputData(OutputString, "========================\r\n");

		m_hllCurHLLevel = hllSubscriber;
	}
	else if(str == "23") {
		m_strOutputIndent = "/ / / /  ";
		OutputData(OutputString, "\r\n=======================\r\n");
		OutputData(OutputString, " Dependent Information\r\n");
		OutputData(OutputString, "=======================\r\n");

		//if the last segment was source, receiver, or invalid, this is
		//a new detail info object
		if(m_hllCurHLLevel == hllSource || m_hllCurHLLevel == hllReceiver || m_hllCurHLLevel == hllInvalid) {
			EligibilityResponseMasterInfo *pMaster = GetCurrentResponseMasterInfo();
			if(pMaster) {
				EligibilityResponseDetailInfo *pDetail = new EligibilityResponseDetailInfo;

				// (j.jones 2009-12-17 16:07) - PLID 36641 - default the detail to have
				// the same confirmation status as the master
				pDetail->ercsStatus = pMaster->ercsStatus;
				
				pMaster->arypResponses.Add(pDetail);
			}
		}

		m_hllCurHLLevel = hllPatient;
	}

	//HL04	736			Hierarchical Child Code					O	ID	1/1

	//always 0 in Loop 2000D

	//0 - No Subordinate HL Segment in This Hierarchical Structure.
	//1 - Additional Subordinate HL Data Segment in This Hierarchical Structure.
	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_AAA(CString &strIn) {

	//Request Validation, used in:

	//160		AAA		Request Validation						S		9
	//172		AAA		Request Validation						S		9
	//184		AAA		Information Receiver Request Validation	S		9
	//207		AAA		Subscriber Request Validation			S		9
	//242		AAA		Subscriber Request Validation			S		9
	//284		AAA		Dependent Request Validation			S		9
	//318		AAA		Dependent Request Validation			S		9

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//AAA01	1073		Yes/No Condition or Response Code		M	ID	1/1

	//Valid Request Indicator

	//N - No
	//Y - Yes
	str = ParseElement(strIn);

	// (j.jones 2009-12-17 14:54) - PLID 36641 - a response's ercsStatus defaults to confirmed,
	// but if the AAA segment exists, then it is not confirmed.
	// Track which status it really is.
	EligibilityResponseConfirmationStatusEnum ercsStatus = ercsDenied;

	if(str == "N") {
		strFmt = "******ELIGIBILITY REQUEST INVALID******\r\n";
		ercsStatus = ercsInvalid;
	}
	else {
		strFmt = "******ELIGIBILITY REQUEST DENIED******\r\n";
		ercsStatus = ercsDenied;
	}

	EligibilityResponseMasterInfo *pMaster = GetCurrentResponseMasterInfo();
	if(pMaster == NULL) {
		//shouldn't be possible - is our ANSI file even valid?
		ThrowNxException("NULL EligibilityResponseMasterInfo found in ANSI_AAA!");
	}
	if(pMaster->arypResponses.GetSize() == 0) {
		//we're not on a detail level, we're on a master level
		pMaster->ercsStatus = ercsStatus;
	}
	else {
		EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
		if(pDetail) {
			//we're on a detail level, so track the status
			pDetail->ercsStatus = ercsStatus;
		}
		else {
			//should be logically impossible since we already checked the array was non-empty
			ThrowNxException("NULL EligibilityResponseDetailInfo found in ANSI_AAA!");
		}
	}

	//output to the file
	OutputData(OutputString, strFmt);

	//and store in memory
	strFmt.Replace("*","");
	// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
	// add to its ExtraMessage field
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if(pBenefitDetail) {
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		strFmt.TrimRight("\r\n");
		pBenefitDetail->strExtraMessage += strFmt;
	}
	else {
		StoreOutputInMemory(strFmt);
	}

	//AAA02 NOT USED

	str = ParseElement(strIn);

	//AAA03	901			Reject Reason Code						O	ID	2/2

	//some of these are unique to certain loops, noted accordingly
	//04 - Authorized Quantity Exceeded (2000A, 2100A)
	//15 - Required application data missing (2100B, 2100C, 2110C, 2100D, 2110D)
	//41 - Authorization/Access Restrictions (2000A, 2100A, 2100B)
	//42 - Unable to Respond at Current Time (2000A, 2100A, 2100C, 2100D)
	//43 - Invalid/Missing Provider Identification (2100B, 2100C, 2100D)
	//44 - Invalid/Missing Provider Name (2100B)
	//45 - Invalid/Missing Provider Specialty (2100B, 2100C, 2100D)
	//46 - Invalid/Missing Provider Phone Number (2100B)
	//47 - Invalid/Missing Provider State (2100B, 2100C, 2100D)
	//48 - Invalid/Missing Referring Provider Identification Number (2100B, 2100C, 2100D)
	//49 - Provider is Not Primary Care Physician (2100C, 2100D)		
	//50 - Provider Ineligible for Inquiries (2100B)
	//51 - Provider Not on File (2100B, 2100C, 2100D)
	//52 - Service Dates Not Within Provider Plan Enrollment (2100C, 2110C, 2100D, 2110D)
	//53 - Inquired Benefit Inconsistent with Provider Type (2110C, 2110D)
	//54 - Inappropriate Product/Service ID Qualifier (2110C, 2110D)
	//55 - Inappropriate Product/Service ID (2110C, 2110D)
	//56 - Inappropriate Date (2100C, 2110C, 2100D, 2110D)
	//57 - Invalid/Missing Date(s) of Service (2100C, 2110C, 2100D, 2110D)
	//58 - Invalid/Missing Date-of-Birth (2100C, 2100D)
	//60 - Date of Birth Follows Date(s) of Service (2100C, 2110C, 2100D, 2110D)
	//61 - Date of Death Precedes Date(s) of Service (2100C, 2110C, 2100D, 2110D)
	//62 - Date of Service Not Within Allowable Inquiry Period (2100C, 2110C, 2100D, 2110D)
	//63 - Date of Service in Future (2100C, 2110C, 2100D, 2110D)
	//64 - Invalid/Missing Patient ID (2100C, 2100D)
	//65 - Invalid/Missing Patient Name (2100C, 2100D)
	//66 - Invalid/Missing Patient Gender Code (2100C, 2100D)
	//67 - Patient Not Found (2100C, 2100D)
	//68 - Duplicate Patient ID Number (2100C, 2100D)
	//69 - Inconsistent with Patient’s Age (2110C, 2110D)
	//70 - Inconsistent with Patient’s Gender (2110C, 2110D)
	//71 - Patient Birth Date Does Not Match That for the Patient on the Database (2100C, 2100D)
	//72 - Invalid/Missing Subscriber/Insured ID (2100C)
	//73 - Invalid/Missing Subscriber/Insured Name (2100C)
	//74 - Invalid/Missing Subscriber/Insured Gender Code (2100C)
	//75 - Subscriber/Insured Not Found (2100C)
	//76 - Duplicate Subscriber/Insured ID Number (2100C)
	//77 - Subscriber Found, Patient Not Found (2100C)
	//78 - Subscriber/Insured Not in Group/Plan Identified (2100C)
	//79 - Invalid Participant Identification (2000A, 2100A, 2100B)
	//80 - No Response received - Transaction Terminated (2100A)
	//97 - Invalid or Missing Provider Address (2100B)
	//T4 - Payer Name or Identifier Missing (2100A, 2100B)
	str = ParseElement(strIn);

	CString strReject;

	if(str == "04")
		strReject = "Authorized Quantity Exceeded";
	else if(str == "15")
		strReject = "Required application data missing";
	else if(str == "41")
		strReject = "Authorization/Access Restrictions";
	else if(str == "42")
		strReject = "Unable to Respond at Current Time";
	else if(str == "43")
		strReject = "Invalid/Missing Provider Identification";
	else if(str == "44")
		strReject = "Invalid/Missing Provider Name";
	else if(str == "45")
		strReject = "Invalid/Missing Provider Specialty";
	else if(str == "46")
		strReject = "Invalid/Missing Provider Phone Number";
	else if(str == "47")
		strReject = "Invalid/Missing Provider State";
	else if(str == "48")
		strReject = "Invalid/Missing Referring Provider Identification Number";
	else if(str == "49")
		strReject = "Provider is Not Primary Care Physician";
	else if(str == "50")
		strReject = "Provider Ineligible for Inquiries";
	else if(str == "51")
		strReject = "Provider Not on File";
	else if(str == "52")
		strReject = "Service Dates Not Within Provider Plan Enrollment";
	else if(str == "53")
		strReject = "Inquired Benefit Inconsistent with Provider Type";
	else if(str == "54")
		strReject = "Inappropriate Product/Service ID Qualifier";
	else if(str == "55")
		strReject = "Inappropriate Product/Service ID";
	else if(str == "56")
		strReject = "Inappropriate Date";
	else if(str == "57")
		strReject = "Invalid/Missing Date(s) of Service";
	else if(str == "58")
		strReject = "Invalid/Missing Date-of-Birth";
	else if(str == "60")
		strReject = "Date of Birth Follows Date(s) of Service";
	else if(str == "61")
		strReject = "Date of Death Precedes Date(s) of Service";
	else if(str == "62")
		strReject = "Date of Service Not Within Allowable Inquiry Period";
	else if(str == "63")
		strReject = "Date of Service in Future";
	else if(str == "64")
		strReject = "Invalid/Missing Patient ID";
	else if(str == "65")
		strReject = "Invalid/Missing Patient Name";
	else if(str == "66")
		strReject = "Invalid/Missing Patient Gender Code";
	else if(str == "67")
		strReject = "Patient Not Found";
	else if(str == "68")
		strReject = "Duplicate Patient ID Number";
	else if(str == "69")
		strReject = "Inconsistent with Patient’s Age";
	else if(str == "70")
		strReject = "Inconsistent with Patient’s Gender";
	else if(str == "71")
		strReject = "Patient Birth Date Does Not Match That for the Patient on the Database";
	else if(str == "72")
		strReject = "Invalid/Missing Subscriber/Insured ID";
	else if(str == "73")
		strReject = "Invalid/Missing Subscriber/Insured Name";
	else if(str == "74")
		strReject = "Invalid/Missing Subscriber/Insured Gender Code";
	else if(str == "75")
		strReject = "Subscriber/Insured Not Found";
	else if(str == "76")
		strReject = "Duplicate Subscriber/Insured ID Number";
	else if(str == "77")
		strReject = "Subscriber Found, Patient Not Found";
	else if(str == "78")
		strReject = "Subscriber/Insured Not in Group/Plan Identified";
	else if(str == "79")
		strReject = "Invalid Participant Identification";
	else if(str == "80")
		strReject = "No Response received - Transaction Terminated";
	else if(str == "97")
		strReject = "Invalid or Missing Provider Address";
	else if(str == "T4")
		strReject = "Payer Name or Identifier Missing";
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strReject = "";
	}

	strFmt.Format("Rejection Reason: %s\r\n", strReject);
	OutputData(OutputString, strFmt);

	//store in memory
	// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
	// add to its ExtraMessage field
	if(pBenefitDetail) {
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		strFmt.TrimRight("\r\n");
		pBenefitDetail->strExtraMessage += strFmt;
	}
	else {
		StoreOutputInMemory(strFmt);
	}

	//AAA04	889			Follow-up Action Code					O	ID	1/1

	//C - Please Correct and Resubmit
	//N - Resubmission Not Allowed
	//P - Please Resubmit Original Transaction
	//R - Resubmission Allowed
	//S - Do Not Resubmit; Inquiry Initiated to a Third Party
	//W - Please Wait 30 Days and Resubmit
	//X - Please Wait 10 Days and Resubmit
	//Y - Do Not Resubmit; We Will Hold Your Request and Respond Again Shortly
	str = ParseElement(strIn);

	CString strAdvice;
	if(str == "C")
		strAdvice = "Please Correct and Resubmit";
	else if(str == "N")
		strAdvice = "Resubmission Not Allowed";
	else if(str == "P")
		strAdvice = "Please Resubmit Original Transaction";
	else if(str == "R")
		strAdvice = "Resubmission Allowed";
	else if(str == "S")
		strAdvice = "Do Not Resubmit; Inquiry Initiated to a Third Party";
	else if(str == "W")
		strAdvice = "Please Wait 30 Days and Resubmit";
	else if(str == "X")
		strAdvice = "Please Wait 10 Days and Resubmit";
	else if(str == "Y")
		strAdvice = "Do Not Resubmit; We Will Hold Your Request and Respond Again Shortly";
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strAdvice = "";
	}

	strFmt.Format("Follow-Up Action: %s\r\n", strAdvice);
	OutputData(OutputString, strFmt);

	//store in memory
	// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
	// add to its ExtraMessage field
	if(pBenefitDetail) {
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		strFmt.TrimRight("\r\n");
		pBenefitDetail->strExtraMessage += strFmt;
	}
	else {
		StoreOutputInMemory(strFmt);
	}

	OutputData(OutputString, "\r\n**************************************\r\n\r\n");
	
	// (j.jones 2010-04-19 14:38) - PLID 38202 - if we have a benefit detail,
	// and it is suppressed, do not output
	if(pBenefitDetail == NULL || !pBenefitDetail->bSuppressFromOutput) {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

void CANSI271Parser::ANSI_NM1(CString &strIn) {

	//Individual or Organizational Name, used in:

	//163		NM1		Information Source Name					R		1
	//178		NM1		Information Receiver Name				R		1
	//193		NM1		Subscriber Name							R		1
	//250		NM1		Subscriber Benefit Related Entity Name	R		1
	//271		NM1		Dependent Name							R		1
	//326		NM1		Dependent Benefit Related Entity Name	R		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//NM101	98			Entity Identifier Code					M	ID	2/3

	//different options are available in different loops, noted accordingly
	//03 - Dependent (2100D)
	//13 - Contracted Service Provider (2120C, 2120D)
	//1P - Provider (2100B, 2120C, 2120D)
	//2B - Third-Party Administrator (2100A, 2100B, 2120C, 2120D)
	//36 - Employer (2100A, 2100B, 2120C, 2120D)
	//73 - Other Physician (2120C, 2120D)
	//80 - Hospital (2100B)
	//FA - Facility (2100B, 2120C, 2120D)
	//GP - Gateway Provider (2100A, 2100B, 2120C, 2120D)
	//IL - Insured or Subscriber (2100C, 2120C, 2120D)
	//LR - Legal Representative (2120C, 2120D)
	//P3 - Primary Care Provider (2120C, 2120D)
	//P4 - Prior Insurance Carrier (2120C, 2120D)
	//P5 - Plan Sponsor (2100A, 2100B, 2120C, 2120D)
	//PR - Payer (2100A, 2100B, 2120C, 2120D)
	//PRP - Primary Payer (2120C, 2120D)
	//SEP - Secondary Payer (2120C, 2120D)
	//TTP - Tertiary Payer (2120C, 2120D)
	//VER - Party Performing Verification (2120C, 2120D) (c.haag 2010-11-02 16:25) - PLID 40350 - ANSI 5010 A1
	//VN - Vendor (2120C, 2120D)
	//X3 - Utilization Management Organization (2120C, 2120D)
	//Y2 - Managed Care Organization (2120C, 2120D (c.haag 2010-11-02 16:25) - PLID 40350 - ANSI 5010 A1
	str = ParseElement(strIn);

	m_bLastNM1IsInsuranceCompany = false;

	if(str == "03")
		strFmt = "Dependent: ";
	else if(str == "13")
		strFmt = "Contracted Service Provider: ";
	else if(str == "1P")
		strFmt = "Provider: ";
	else if(str == "2B")
		strFmt = "Third-Party Administrator: ";
	else if(str == "36")
		strFmt = "Employer: ";
	else if(str == "73")
		strFmt = "Other Physician: ";			
	else if(str == "80")
		strFmt = "Hospital: ";
	else if(str == "FA")
		strFmt = "Facility: ";
	else if(str == "GP")
		strFmt = "Gateway Provider: ";		
	else if(str == "IL")
		strFmt = "Insured or Subscriber: ";		
	else if(str == "LR")
		strFmt = "Legal Representative: ";		
	else if(str == "P3")
		strFmt = "Primary Care Provider: ";		
	else if (str == "P4") {
		strFmt = "Prior Insurance Carrier: ";

		// (j.jones 2016-05-16 09:00) - NX-100357 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else if(str == "P5")
		strFmt = "Plan Sponsor: ";		
	else if (str == "PR") {
		strFmt = "Payer: ";

		// (j.jones 2016-05-16 09:00) - NX-100357 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else if (str == "PRP") {
		strFmt = "Primary Payer: ";

		// (j.jones 2016-05-16 09:00) - NX-100357 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else if (str == "SEP") {
		strFmt = "Secondary Payer: ";

		// (j.jones 2016-05-16 09:00) - NX-100357 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else if (str == "TTP") {
		strFmt = "Tertiary Payer: ";

		// (j.jones 2016-05-16 09:00) - NX-100357 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else if(str == "VN")
		strFmt = "Vendor: ";
	else if(str == "X3")
		strFmt = "Utilization Management Organization: ";
	// (c.haag 2010-11-02 16:25) - Found in ANSI 5010 A1
	else if(str == "VER")
		strFmt = "Party Performing Verification: ";
	else if (str == "Y2") {
		strFmt = "Managed Care Organization: ";

		// (j.jones 2016-05-17 16:25) - NX-100668 - track that this is an insurance payer
		m_bLastNM1IsInsuranceCompany = true;
	}
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strFmt = "";
	}

	CString strQualifierName = strFmt;

	OutputData(OutputString, strFmt);

	//j.jones - Rather than using FixCapitalization in NM1, it is actually
	//more visually handy to have the payer / provider / patient names
	//in caps. And it's not always sent to us in caps, so capitalize it.

	//NM102	1065		Entity Type Qualifier					M	ID	1/1

	//1 - Person
	//2 - Non-Person Entity
	str = ParseElement(strIn);

	//NM103	1035		Name Last or Organization Name			O	AN	1/35

	str = ParseElement(strIn);

	CString strLast = str;

	str.MakeUpper();
	OutputData(OutputString, str);

	//NM104	1036		Name First								O	AN	1/25

	str = ParseElement(strIn);

	CString strFirst = str;

	if(!str.IsEmpty()) {
		str.MakeUpper();
		strFmt.Format(", %s", str);
		OutputData(OutputString, strFmt);
	}

	//NM105	1037		Name Middle								O	AN	1/25

	str = ParseElement(strIn);

	CString strMiddle = str;

	if(!str.IsEmpty()) {
		str.MakeUpper();
		strFmt.Format(" %s", str);
		OutputData(OutputString, strFmt);
	}

	// (j.jones 2016-05-17 16:29) - NX-100668 - handle person names, just incase
	CString strContactName = strLast;
	if (!strContactName.IsEmpty() && !strFirst.IsEmpty()) {
		strContactName += ", " + strFirst;
	}
	if (!strContactName.IsEmpty() && !strMiddle.IsEmpty()) {
		strContactName += " " + strMiddle;
	}

	// (j.jones 2016-05-16 09:00) - NX-100357 - if a payer, add this payer to our insurance list
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if (!strContactName.IsEmpty() && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "U")) {

		strContactName = FixCapitalization(strContactName);

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("%s %s", strQualifierName, strContactName);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track as a specific company
			pBenefitDetail->strInsuranceCompanyName = strContactName;
		}
	}

	//NM106 NOT USED

	str = ParseElement(strIn);

	//NM107	1039		Name Suffix								O	AN	1/10

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		str.MakeUpper();
		strFmt.Format(" %s", str);
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "\r\n\r\n");

	//NM108	66			Identification Code Qualifier			X	ID	1/2

	//if 2100A:
		//24 - Employer’s Identification Number
		//46 - Electronic Transmitter Identification Number (ETIN)
		//FI - Federal Taxpayer’s Identification Number
		//NI - National Association of Insurance Commissioners (NAIC) Identification
		//PI - Payor Identification
		//XV - Health Care Financing Administration National PlanID
		//XX - Health Care Financing Administration National Provider Identifier
	//if 2100B:
		//24 - Employer’s Identification Number
		//34 - Social Security Number
		//FI - Federal Taxpayer’s Identification Number
		//PI - Payor Identification
		//PP - Pharmacy Processor Number
		//SV - Service Provider Number
		//XV - Health Care Financing Administration National PlanID
		//XX - Health Care Financing Administration National Provider Identifier
	//if 2100C or 2100D:
		//MI - Member Identification Number
		//ZZ - Mutually Defined
	//if 2120C or 2120D:
		//24 - Employer’s Identification Number
		//34 - Social Security Number
		//46 - Electronic Transmitter Identification Number (ETIN)
		//FA - Facility Identification
		//FI - Federal Taxpayer’s Identification Number
		//MI - Member Identification Number
		//NI - National Association of Insurance Commissioners (NAIC) Identification
		//PI - Payor Identification
		//PP - Pharmacy Processor Number
		//SV - Service Provider Number
		//XV - Health Care Financing Administration National PlanID
		//XX - Health Care Financing Administration National Provider Identifier
		//ZZ - Mutually Defined
	str = ParseElement(strIn);

	bool bIsPayerID = false;

	if(str == "24")
		strFmt = "Employer’s Identification Number: ";
	else if(str == "46")
		strFmt = "Electronic Transmitter Identification Number: ";
	else if(str == "FI")
		strFmt = "Federal Taxpayer’s Identification Number: ";
	else if(str == "NI")
		strFmt = "National Association of Insurance Commissioners (NAIC) Identification: ";
	else if(str == "PI")
		strFmt = "Payor Identification: ";
	else if(str == "XV")
		strFmt = "Health Care Financing Administration National PlanID: ";
	else if(str == "XX")
		strFmt = "Health Care Financing Administration National Provider Identifier: ";
	else if(str == "34")
		strFmt = "Social Security Number: ";
	else if (str == "PI") {
		strFmt = "Payor Identification: ";

		bIsPayerID = true;
	}
	else if(str == "PP")
		strFmt = "Pharmacy Processor Number: ";
	else if(str == "SV")
		strFmt = "Service Provider Number: ";
	else if(str == "MI")
		strFmt = "Member Identification Number: ";
	else if(str == "ZZ")
		strFmt = "Mutually Defined ID: ";
	else if(str == "FA")
		strFmt = "Facility Identification: ";
	// (j.jones 2009-02-20 17:25) - PLID 33120 - blank is legal
	else if(str == "") {
		strFmt = "";
	}
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strFmt = "";
	}

	CString strQual = str;

	OutputData(OutputString, strFmt);

	//NM109	67			Identification Code						X	AN	2/80

	str = ParseElement(strIn);

	// (j.jones 2016-05-16 09:00) - NX-100357 - if a payer ID, store it
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	if (!str.IsEmpty() && bIsPayerID && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("%s %s", strFmt, str);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the payer ID
			pBenefitDetail->strInsuranceCompanyPayerID = str;
		}
	}

	// (j.jones 2016-05-19 15:41) - NX-100694 - if a member ID is provided, show it in the main response
	if ((m_hllCurHLLevel == hllSubscriber || m_hllCurHLLevel == hllPatient)
		&& strQual == "MI") {
		CString strMsg;
		strMsg.Format("%s %s\r\n", strFmt, str);
		StoreOutputInMemory(strMsg);
	}

	OutputData(OutputString, str);

	//NM110 NOT USED

	str = ParseElement(strIn);
	
	//NM111 NOT USED

	str = ParseElement(strIn);

	//NM112 NOT USED
	// (c.haag 2010-10-19 15:23) - PLID 40350 - NM112

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_REF(CString &strIn) {

	//Reference Identification, used in:

	//166		REF		Information Source Additional Identification	S	9
	//182		REF		Information Receiver Additional Identification	S	9
	//196		REF		Subscriber Additional Identification	S		9
	//238		REF		Subscriber Additional Identification	S		9
	//274		REF		Dependent Additional Identification		S		9
	//314		REF		Dependent Additional Identification		S		9

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//REF01	128			Reference Identification Qualifier		M	ID	2/3

	//if 2100A:
		//18 - Plan Number
		//55 - Sequence Number
	//if 2100B:
		//0B - State License Number
		//1C - Medicare Provider Number
		//1D - Medicaid Provider Number
		//1J - Facility ID Number
		//4A - Personal Identification Number (PIN)
		//CT - Contract Number
		//EL - Electronic device pin number
		//EO - Submitter Identification Number
		//HPI - Health Care Financing Administration National Provider Identifier
		//JD - User Identification
		//N5 - Provider Plan Network Identification Number
		//N7 - Facility Network Identification Number
		//Q4 - Prior Identifier Number
		//SY - Social Security Number
		//TJ - Federal Taxpayer’s Identification Number
	//if 2100C or 2100D:
		//18 - Plan Number
		//1L - Group or Policy Number
		//1W - Member Identification Number
		//3H - Case Number (2100C only)
		//49 - Family Unit Number
		//6P - Group Number
		//A6 - Employee Identification Number
		//EA - Medical Record Identification Number
		//EJ - Patient Account Number
		//F6 - Health Insurance Claim (HIC) Number
		//GH - Identification Card Serial Number
		//HJ - Identity Card Number
		//IF - Issue Number
		//IG - Insurance Policy Number
		//ML - Military Rank/Civilian Pay Grade Number
		//M7 - Medical Assistance Category (2100D only)
		//N6 - Plan Network Identification Number
		//NQ - Medicaid Recipient Identification Number
		//Q4 - Prior Identifier Number
		//SY - Social Security Number
		//CE - Class of Contract (c.haag 2010-11-02 16:27) - PLID 40350 - ANSI 5010 A1 
	//if 2110C or 2110D:		// (j.jones 2013-05-13 16:45) - PLID 56621 - updated with 5010 options
		//18 - Plan Number
		//1L - Group or Policy Number
		//1W - Member Identification Number
		//49 - Family Unit Number
		//6P - Group Number
		//9F - Referral Number
		//ALS - Alternative List ID - Allows the source to identify the list identifier of a
								//list of drugs and its alternative drugs with the associated formulary status for the patient.
		//CLI - Coverage List ID - Allows the source to identify the list identifier of a
								//list of drugs that have coverage limitations for the associated patient.
		//F6 - Health Insurance Claim (HIC) Number
		//FO - Drug Formulary Number
		//G1 - Prior Authorization Number
		//IG - Insurance Policy Number
		//N6 - Plan Network Identification Number
		//NQ - Medicaid Recipient Identification Number

	str = ParseElement(strIn);

	CString strLabel;

	bool bIsPolicyNumber = false;
	bool bIsGroupNumber = false;

	if(str == "0B")
		strLabel = "State License Number: ";
	else if (str == "18") {
		strLabel = "Plan Number: ";
		bIsPolicyNumber = true;
	}
	else if(str == "1C")
		strLabel = "Medicare Provider Number: ";
	else if(str == "1D")
		strLabel = "Medicaid Provider Number: ";
	else if(str == "1J")
		strLabel = "Facility ID Number: ";
	else if (str == "1L") {
		strLabel = "Group or Policy Number: ";
		bIsPolicyNumber = true;
	}
	else if(str == "1W")
		strLabel = "Member Identification Number: ";
	else if(str == "3H")
		strLabel = "Case Number: ";
	else if(str == "49")
		strLabel = "Family Unit Number: ";
	else if(str == "4A")
		strLabel = "Personal Identification Number (PIN): ";
	else if(str == "55")
		strLabel = "Sequence Number: ";
	else if (str == "6P") {
		strLabel = "Group Number: ";
		bIsGroupNumber = true;
	}
	else if(str == "9F")
		strLabel = "Referral Number: ";
	else if(str == "A6")
		strLabel = "Employee Identification Number: ";
	else if(str == "CT")
		strLabel = "Contract Number: ";
	else if(str == "EA")
		strLabel = "Medical Record Identification Number: ";
	else if(str == "EJ")
		strLabel = "Patient Account Number: ";
	else if(str == "EL")
		strLabel = "Electronic device pin number: ";
	else if(str == "EO")
		strLabel = "Submitter Identification Number: ";
	else if(str == "F6")
		strLabel = "Health Insurance Claim (HIC) Number: ";
	else if(str == "G1")
		strLabel = "Prior Authorization Number: ";
	else if(str == "GH")
		strLabel = "Identification Card Serial Number: ";
	else if(str == "HJ")
		strLabel = "Identity Card Number: ";
	else if(str == "HPI")
		strLabel = "Health Care Financing Administration National Provider Identifier: ";
	else if(str == "IF")
		strLabel = "Issue Number: ";
	else if (str == "IG") {
		strLabel = "Insurance Policy Number: ";
		bIsPolicyNumber = true;
	}
	else if(str == "JD")
		strLabel = "User Identification: ";
	else if(str == "M7")
		strLabel = "Medical Assistance Category: ";
	else if(str == "ML")
		strLabel = "Military Rank/Civilian Pay Grade Number: ";	
	else if(str == "N5")
		strLabel = "Provider Plan Network Identification Number: ";
	else if(str == "N6")
		strLabel = "Plan Network Identification Number: ";
	else if(str == "N7")
		strLabel = "Facility Network Identification Number: ";
	else if(str == "NQ")
		strLabel = "Medicaid Recipient Identification Number: ";
	else if(str == "Q4")
		strLabel = "Prior Identifier Number: ";
	else if(str == "SY")
		strLabel = "Social Security Number: ";
	else if(str == "TJ")
		strLabel = "Federal Taxpayer’s Identification Number: ";
	// (c.haag 2010-11-02 16:27) - PLID 40350 - ANSI 5010 A1
	else if(str == "CE")
		strLabel = "Class of Contract: ";
	// (j.jones 2013-05-13 16:45) - PLID 56621 - updated with 5010 options
	// (j.jones 2013-08-13 15:38) - PLID 58012 - fixed so these compared... and not assign!
	else if(str == "ALS")
		strLabel = "Alternative List ID: ";
	else if(str == "CLI")
		strLabel = "Coverage List ID: ";
	else if(str == "FO")
		strLabel = "Drug Formulary Number: ";
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strLabel = "";
	}

	//REF02	127			Reference Identification				X	AN	1/30

	str = ParseElement(strIn);

	// (j.jones 2016-05-16 09:00) - NX-100357 - if this is a group or policy number, store it,
	// although sometimes this can stupidly be before an NM1 record so check the benefit type
	// for an additional payer too
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if (!str.IsEmpty() && pBenefitDetail != NULL
		&& (bIsPolicyNumber || bIsGroupNumber)
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("%s %s", strLabel, str);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the ID
			if (bIsPolicyNumber) {
				pBenefitDetail->strInsuredPartyPolicyNumber = str;
			}
			else if (bIsGroupNumber) {
				pBenefitDetail->strInsuredPartyGroupNumber = str;
			}
		}
	}

	if(!str.IsEmpty()) {
		strFmt.Format("%s%s\r\n",strLabel,str);
		OutputData(OutputString, strFmt);
	}

	//REF03	352			Description								X	AN	1/80

	str = ParseElement(strIn);
	// (j.jones 2013-05-13 16:45) - PLID 56621 - output this
	if(!str.IsEmpty()) {
		OutputData(OutputString, str);
	}

	//REF04 NOT USED

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_PER(CString &strIn) {

	//Administrative Communications Contact, used in:

	//168		PER		Information Source Contact Information	S		3
	//203		PER		Subscriber Contact Information			S		3
	//257		PER		Subscriber Benefit Related Entity Contact Information	S	3
	//280		PER		Dependent Contact Information			S		3
	//333		PER		Dependent Benefit Related Entity Contact Information	S	3

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//PER01	366			Contact Function Code					M	ID	2/2

	//IC - Information Contact
	str = ParseElement(strIn);

	//PER02	93			Name									O	AN	1/60

	str = ParseElement(strIn);
	str = FixCapitalization(str);

	strFmt.Format("Contact Information: %s\r\n", str);
	OutputData(OutputString, strFmt);

	// (j.jones 2016-05-16 09:00) - NX-100357 - if this is for an insurance company, store it
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if (!str.IsEmpty() && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {
		
		//we need to store this as the Extra Message
		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		strFmt.TrimRight("\r\n");
		pBenefitDetail->strExtraMessage += strFmt;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the contact
			pBenefitDetail->strInsuranceCompanyContactName = str;
		}
	}

	//PER03	365			Communication Number Qualifier			X	ID	2/2

	//HP - Home Phone Number
	//ED - Electronic Data Interchange Access Number
	//EM - Electronic Mail
	//EX - Telephone Extension
	//FX - Facsimile
	//TE - Telephone
	//WP - Work Phone Number
	//UR - Uniform Resource Locator (URL) (c.haag 2010-10-19 15:37) - PLID 40350
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		if(str == "HP")
			strFmt = "Home Phone: ";
		else if(str == "ED")
			strFmt = "Electronic Data Interchange Access Number: ";
		else if(str == "EM")
			strFmt = "E-Mail Address: ";
		else if(str == "EX")
			strFmt = "Extension: ";
		else if(str == "FX")
			strFmt = "Fax Number: ";
		else if(str == "TE")
			strFmt = "Phone Number: ";
		else if(str == "WP")
			strFmt = "Work Phone: ";
		else if(str == "UR")
			strFmt = "URL: ";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strFmt = "";
		}
		OutputData(OutputString, strFmt);
	}

	//PER04	364			Communication Number					X	AN	1/80

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		OutputData(OutputString, FormatString("%s\r\n", str));
	}

	// (j.jones 2016-05-16 09:00) - NX-100357 - if this is for an insurance company, store it
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	if (!str.IsEmpty() && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

		if (str.GetLength() >= 10) {
			str = FormatPhone(str, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		}

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("%s %s", strFmt, str);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the contact number
			pBenefitDetail->strInsuranceCompanyPhone = str;
		}
	}

	//PER05	365			Communication Number Qualifier			X	ID	2/2

	//HP - Home Phone Number
	//ED - Electronic Data Interchange Access Number
	//EM - Electronic Mail
	//EX - Telephone Extension
	//FX - Facsimile
	//TE - Telephone
	//WP - Work Phone Number
	//UR - Uniform Resource Locator (URL) (c.haag 2010-10-19 15:37) - PLID 40350
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		if(str == "HP")
			strFmt = "Home Phone: ";
		else if(str == "ED")
			strFmt = "Electronic Data Interchange Access Number: ";
		else if(str == "EM")
			strFmt = "E-Mail Address: ";
		else if(str == "EX")
			strFmt = "Extension: ";
		else if(str == "FX")
			strFmt = "Fax Number: ";
		else if(str == "TE")
			strFmt = "Phone Number: ";
		else if(str == "WP")
			strFmt = "Work Phone: ";
		else if(str == "UR")
			strFmt = "URL: ";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strFmt = "";
		}
		OutputData(OutputString, strFmt);
	}

	//PER06	364			Communication Number					X	AN	1/80

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("%s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER07	365			Communication Number Qualifier			X	ID	2/2

	//HP - Home Phone Number
	//ED - Electronic Data Interchange Access Number
	//EM - Electronic Mail
	//EX - Telephone Extension
	//FX - Facsimile
	//TE - Telephone
	//WP - Work Phone Number
	//UR - Uniform Resource Locator (URL) (c.haag 2010-10-19 15:37) - PLID 40350
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		if(str == "HP")
			strFmt = "Home Phone: ";
		else if(str == "ED")
			strFmt = "Electronic Data Interchange Access Number: ";
		else if(str == "EM")
			strFmt = "E-Mail Address: ";
		else if(str == "EX")
			strFmt = "Extension: ";
		else if(str == "FX")
			strFmt = "Fax Number: ";
		else if(str == "TE")
			strFmt = "Phone Number: ";
		else if(str == "WP")
			strFmt = "Work Phone: ";
		else if(str == "UR")
			strFmt = "URL: ";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strFmt = "";
		}
		OutputData(OutputString, strFmt);
	}

	//PER08	364			Communication Number					X	AN	1/80

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("%s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER09 NOT USED

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_TRN(CString &strIn) {

	//Trace, used in:

	//190		TRN		Subscriber Trace Number					S		3
	//268		TRN		Dependent Trace Number					S		3

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//TRN01	481			Trace Type Code							M	ID	1/2

	//1 - Current Transaction Trace Numbers
		//"The term "Current Transaction Trace Numbers"
		//refers to trace or reference numbers assigned by
		//the creator of the 271 transaction (the information
		//source).
		//If a clearinghouse has assigned a TRN segment and
		//intends on returning their TRN segment in the 271
		//response to the information receiver, they must
		//convert the value in TRN01 to "1" (since it will be
		//returned by the information source as a "2").
	//2 - Referenced Transaction Trace Numbers
		//The term "Referenced Transaction Trace Numbers"
		//refers to trace or reference numbers originally sent
		//in the 270 transaction and now returned in the 271.
	str = ParseElement(strIn);

	//set to true if it's supposed to be our trace number
	BOOL bCheckTraceNumber = FALSE;
	
	CString strTraceDesc;
	if(str == "1") {
		strTraceDesc = "New Trace Identifier";
	}
	else if(str == "2") {
		strTraceDesc = "Referenced Trace Identifier";
		//this is supposed to be our trace number
		bCheckTraceNumber = TRUE;
	}

	//TRN02	127			Reference Identification				M	AN	1/30

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("%s: %s\r\n", strTraceDesc, str);
		OutputData(OutputString, strFmt);

		//now, add it to our detail
		if(bCheckTraceNumber) {
			EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
			if(pDetail) {

				//do we already have this filled in?
				if(pDetail->nEligibilityRequestID == -1) {
					//if not, fill it with this information
					CalculateIDFromTrace1(pDetail, str);
				}
			}
		}
	}

	//TRN03	509			Originating Company Identifier			O	AN	10/10

	//If TRN01 is "2", this is the value received in the original 270 transaction.
	//If TRN01 is "1", use this information to identify the organization that assigned this trace number.
	str = ParseElement(strIn);

	//TRN04	127			Reference Identification				O	AN	1/30

	//If TRN01 is "2", this is the value received in the original 270 transaction.
	//If TRN01 is "1", use this information if necessary to further identify
	//a specific component, such as a specific division or group of the
	//entity identified in the previous data element (TRN03).
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("%s: %s\r\n", strTraceDesc, str);
		OutputData(OutputString, strFmt);

		//now, add it to our detail
		if(bCheckTraceNumber) {
			EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
			if(pDetail) {

				//do we already have these filled in?
				if(pDetail->nPatientID == -1 || pDetail->nInsuredPartyID == -1) {
					//if not, fill it with this information
					CalculateIDFromTrace2(pDetail, str);

					//Do not try to calculate the request ID if it is blank,
					//because we may have another, better trace coming up next.
					//After parsing, we'll try to calculate any request IDs
					//that remain unfilled.
				}
			}
		}
	}

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_N3(CString &strIn) {

	//Address Information, used in:

	//200		N3		Subscriber Address						S		1
	//254		N3		Subscriber Benefit Related Entity Address		S	1
	//277		N3		Dependent Address						S		1
	//330		N3		Dependent Benefit Related Entity Address		S	1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//N301	166			Address Information						M	AN	1/55

	str = ParseElement(strIn);
	str = FixCapitalization(str);

	// (j.jones 2016-05-16 09:00) - NX-100357 - if this is for an insurance company, store it
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if (!str.IsEmpty() && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("Address: %s", str);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the address
			pBenefitDetail->strInsuranceCompanyAddress1 = str;
		}
	}

	if(!str.IsEmpty()) {
		strFmt.Format("Address: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//N302	166			Address Information						O	AN	1/55

	str = ParseElement(strIn);
	str = FixCapitalization(str);

	// (j.jones 2016-05-16 09:00) - NX-100357 - if this is for an insurance company, store it
	// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
	// of if the benefit type is 'contact following entity'
	if (!str.IsEmpty() && pBenefitDetail != NULL
		&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

		CString strMsg;
		//we need to store this as the Extra Message
		strMsg.Format("%s", str);

		if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strMsg;

		if (pBenefitDetail->strBenefitTypeQualifier == "R") {
			//if "Other or Additional Payor", track the address
			pBenefitDetail->strInsuranceCompanyAddress2 = str;
		}
	}

	//indent to be aligned with Address 1
	if(!str.IsEmpty()) {
		strFmt.Format("         %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//don't add another new line, city/state/zip is surely coming next
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_N4(CString &strIn) {

	//Geographic Location, used in:

	//201		N3		Subscriber City/State/Zip						S		1
	//255		N3		Subscriber Benefit Related Entity City/State/Zip		S	1
	//378		N3		Dependent City/State/Zip						S		1
	//331		N3		Dependent Benefit Related Entity City/State/Zip			S	1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//N401	19			City Name								O	AN	2/30

	CString strCity = ParseElement(strIn);

	//N402	156			State or Province Code					O	ID	2/2

	CString strState = ParseElement(strIn);

	//N403	116			Postal Code								O	ID	3/15

	CString strZip = ParseElement(strIn);

	//indent to be aligned with Address 1
	if(!strCity.IsEmpty() || !strState.IsEmpty() || !strZip.IsEmpty()) {
		strFmt.Format("         %s, %s %s\r\n", FixCapitalization(strCity), strState, strZip);
		OutputData(OutputString, strFmt);

		// (j.jones 2016-05-16 09:00) - NX-100357 - if this is for an insurance company, store it
		// (j.jones 2016-05-17 16:25) - NX-100668 - also include it in the notes if in loop 2120,
		// of if the benefit type is 'contact following entity'
		BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
		if (pBenefitDetail != NULL
			&& (m_bLastNM1IsInsuranceCompany || m_bCurLoop2120 || pBenefitDetail->strBenefitTypeQualifier == "R")) {

			if (!pBenefitDetail->strExtraMessage.IsEmpty()) {
				pBenefitDetail->strExtraMessage += "\r\n";
			}
			strFmt.TrimLeft();
			strFmt.TrimRight("\r\n");
			pBenefitDetail->strExtraMessage += strFmt;

			if (pBenefitDetail->strBenefitTypeQualifier == "R") {
				//if "Other or Additional Payor", track the address
				pBenefitDetail->strInsuranceCompanyCity = FixCapitalization(strCity);
				pBenefitDetail->strInsuranceCompanyState = strState;
				pBenefitDetail->strInsuranceCompanyZip = strZip;
			}
		}
	}

	//N404	26			Country Code							O	ID	2/3

	str = ParseElement(strIn);

	//N405	309			Location Qualifier						X	ID	1/2

	//CY - County/Parish
	//FI - Federal Information Processing Standards (FIPS) 55 (Named Populated Places)
	str = ParseElement(strIn);

	//N406	310			Location Identifier						O	AN	1/30

	str = ParseElement(strIn);
	
	//N407	1715			Country Subdivision Code				X 1 ID 1/3
	// (c.haag 2010-10-19 15:43) - PLID 40350 - N407. We ignore this.

	str = ParseElement(strIn);


	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_DMG(CString &strIn) {

	//Demographic Information, used in:

	//210		DMG		Subscriber Demographic Information		S		1
	//287		DMG		Dependent Demographic Information		S		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//DMG01	1250		Date Time Period Format Qualifier		X	ID	2/3

	//D8 - Date Expressed in Format CCYYMMDD
	str = ParseElement(strIn);

	//DMG02	1251		Date Time Period						X	AN	1/35
	
	//Loop 2100C: Subscriber Birth Date
	//Loop 2100D: Dependent Birth Date
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		CString strDate = ConvertDateFormat(str);
		strFmt.Format("Birthdate: %s\r\n",strDate);
		OutputData(OutputString, strFmt);
	}

	//DMG03	1068		Gender Code								O	ID	1/1

	//F - Female
	//M - Male
	//U - Unknown
	str = ParseElement(strIn);

	//j.jones - We could output the gender here, but I see no point.

	//DMG04 NOT USED

	str = ParseElement(strIn);

	//DMG05 NOT USED

	str = ParseElement(strIn);

	//DMG06 NOT USED

	str = ParseElement(strIn);

	//DMG07 NOT USED

	str = ParseElement(strIn);

	//DMG08 NOT USED

	str = ParseElement(strIn);
	
	//DMG09 NOT USED

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_INS(CString &strIn) {

	//Insured Benefit, used in:

	//212		INS		Subscriber Relationship					S		1
	//289		INS		Dependent Relationship					S		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//INS01	1073		Yes/No Condition or Response Code		M	ID	1/1

	//Insured Indicator

	//Y - Yes (2100C)
	//N - No (2100D)
	str = ParseElement(strIn);

	//INS02	1069		Individual Relationship Code			M	ID	2/2

	//If Loop 2100C:
		//18 - Self
	//If Loop 2100D:
		//01 - Spouse
		//19 - Child
		//21 - Unknown
		//34 - Other Adult
	str = ParseElement(strIn);

	CString strRel;

	if(str == "18")
		strRel = "Self";
	else if(str == "01")
		strRel = "Spouse";
	else if(str == "19")
		strRel = "Child";
	else if(str == "21")
		strRel = "Unknown";
	else if(str == "34")
		strRel = "Other Adult";
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strRel = "";
	}

	if(!strRel.IsEmpty()) {
		strFmt.Format("Relationship To Insured: %s\r\n", strRel);
		OutputData(OutputString, strFmt);
	}

	//INS03	875			Maintenance Type Code					O	ID	3/3

	//001 - Change
	str = ParseElement(strIn);

	//INS04	1203		Maintenance Reason Code					O	ID	2/3

	//25 - Change in Indentifying Data Elements
		//Use this code to indicate that a change has been
		//made to the primary elements that identify a specific
		//person. Such elements are first name, last name,
		//date of birth, identification numbers, and address.
	str = ParseElement(strIn);

	//INS05 NOT USED

	str = ParseElement(strIn);

	//INS06 NOT USED

	str = ParseElement(strIn);

	//INS07 NOT USED

	str = ParseElement(strIn);

	//INS08 NOT USED

	str = ParseElement(strIn);

	//INS09	1220		Student Status Code						O	ID	1/1

	//F - Full-time
	//N - Not a Student
	//P - Part-time
	str = ParseElement(strIn);

	//INS10	1073		Yes/No Condition or Response Code		O	ID	1/1

	//Handicap Indicator

	//N - No
	//Y - Yes
	str = ParseElement(strIn);

	//INS11 NOT USED

	str = ParseElement(strIn);

	//INS12 NOT USED

	str = ParseElement(strIn);

	//INS13 NOT USED

	str = ParseElement(strIn);

	//INS14 NOT USED

	str = ParseElement(strIn);

	//INS15 NOT USED

	str = ParseElement(strIn);

	//INS16 NOT USED

	str = ParseElement(strIn);

	//INS17	1470		Number									O	N0	1/9

	//Birth Sequence Number
	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

// (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment HI
void CANSI271Parser::ANSI_HI(CString& strIn) {

	//Dependent Health Care Diagnosis Code, used in:
	//
	//170 1150 HI Dependent Health Care Diagnosis Code S 1
	//
	CString OutputString, str, strFmt;

	//HI01 C022 HEALTH CARE CODE INFORMATION M 1
	CString strComposite = ParseElement(strIn);
		//HI01 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 1: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI01 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 1: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI01 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI01 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI01 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI01 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI01 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI01 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI01 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI02 C022 HEALTH CARE CODE INFORMATION O 1
	strComposite = ParseElement(strIn);
		//HI02 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 2: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI02 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 2: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI02 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI02 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI02 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI02 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI02 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI02 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI02 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI03 C022 HEALTH CARE CODE INFORMATION O 1
	strComposite = ParseElement(strIn);
		//HI03 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 3: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI03 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 3: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI03 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI03 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI03 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI03 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI03 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI03 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI03 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI04 C022 HEALTH CARE CODE INFORMATION O 1
	strComposite = ParseElement(strIn);
		//HI04 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 4: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI04 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 4: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI04 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI04 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI04 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI04 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI04 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI04 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI04 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI05 C022 HEALTH CARE CODE INFORMATION O 1 - Hi five!
	strComposite = ParseElement(strIn);
		//HI05 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 5: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI05 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 5: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI05 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI05 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI05 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI05 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI05 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI05 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI05 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI06 C022 HEALTH CARE CODE INFORMATION O 1
	strComposite = ParseElement(strIn);
		//HI06 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 6: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI06 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 6: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI06 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI06 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI06 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI06 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI06 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI06 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI06 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI07 C022 HEALTH CARE CODE INFORMATION O 1
	strComposite = ParseElement(strIn);
		//HI07 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 7: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI07 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 7: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI07 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI07 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI07 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI07 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI07 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI07 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI07 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI08 - 2 1271 Industry Code M AN 1/30
	strComposite = ParseElement(strIn);
		//HI08 - 1 1270 Code List Qualifier Code M ID 1/3
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Code List Qualifier Code 8: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		//HI08 - 2 1271 Industry Code M AN 1/30
		str = ParseComposite(strComposite);
		if (!str.IsEmpty()) {
			strFmt.Format("Industry Code 8: %s\r\n", str);
			OutputData(OutputString, strFmt);
		}
		str = ParseComposite(strComposite); //HI08 - 3 1250 Date Time Period Format Qualifier X ID 2/3 (NU)		
		str = ParseComposite(strComposite); //HI08 - 4 1251 Date Time Period X AN 1/35 (NU)		
		str = ParseComposite(strComposite); //HI08 - 5 782 Monetary Amount O R 1/18 (NU)		
		str = ParseComposite(strComposite); //HI08 - 6 380 Quantity O R 1/15 (NU)		
		str = ParseComposite(strComposite); //HI08 - 7 799 Version Identifier O AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI08 - 8 1271 Industry Code X AN 1/30 (NU)		
		str = ParseComposite(strComposite); //HI08 - 9 1073 Yes/No Condition or Response Code X ID 1/1 (NU)

	//HI09 C022 HEALTH CARE CODE INFORMATION O 1 (NU)
	str = ParseElement(strIn);

	//HI10 C022 HEALTH CARE CODE INFORMATION O 1 (NU)
	str = ParseElement(strIn);

	//HI11 C022 HEALTH CARE CODE INFORMATION O 1 (NU)
	str = ParseElement(strIn);

	//HI12 C022 HEALTH CARE CODE INFORMATION O 1 (NU)
	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_DTP(CString &strIn) {

	//Date or Time or Period, used in:

	//216		DTP		Subscriber Date							S		9
	//240		DTP		Subscriber Eligibility/Benefit Date		S		20
	//293		DTP		Dependent Date							S		9
	//316		DTP		Dependent Eligibility/Benefit Date		S		20

	CString OutputString, str, strFmt;

	BenefitDetail *pBenefitDetail = NULL;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//DTP01	374			Date/Time Qualifier						M	ID	3/3

	//096 - Discharge (c.haag 2010-10-19 15:43) - PLID 40350
	//102 - Issue
	//152 - Effective Date of Change
	//193 - Period Start
	//194 - Period End
	//198 - Completion
	//290 - Coordination of Benefits
	//291 - Plan
	//292 - Benefit
	//295 - Primary Care Provider
	//304 - Latest Visit or Consultation
	//307 - Eligibility
	//318 - Added
	//340 - Consolidated Omnibus Budget Reconciliation Act (COBRA) Begin
	//341 - Consolidated Omnibus Budget Reconciliation Act (COBRA) End
	//342 - Premium Paid to Date Begin
	//343 - Premium Paid to Date End
	//346 - Plan Begin
	//347 - Plan End
	//348 - Benefit Begin
	//349 - Benefit End
	//356 - Eligibility Begin
	//357 - Eligibility End
	//382 - Enrollment
	//435 - Admission
	//442 - Date of Death
	//458 - Certification
	//472 - Service
	//539 - Policy Effective
	//540 - Policy Expiration
	//636 - Date of Last Update
	//771 - Status
	str = ParseElement(strIn);

	CString strLabel;

	if (str == "096")
		strLabel = "Discharge";
	else if(str == "102")
		strLabel = "Issue Date";
	else if(str == "152")
		strLabel = "Effective Date of Change";
	else if(str == "193")
		strLabel = "Period Start Date";
	else if(str == "194")
		strLabel = "Period End Date";
	else if(str == "198")
		strLabel = "Completion Date";
	else if(str == "290")
		strLabel = "Coordination of Benefits Date";
	else if(str == "291")
		strLabel = "Plan Date";
	else if(str == "292")
		strLabel = "Benefit Date";
	else if(str == "295")
		strLabel = "Primary Care Provider Date";
	else if(str == "304")
		strLabel = "Latest Visit or Consultation Date";
	else if(str == "307")
		strLabel = "Eligibility Date";
	else if(str == "318")
		strLabel = "Date Added";
	else if(str == "340")
		strLabel = "COBRA Begin Date";
	else if(str == "341")
		strLabel = "COBRA End Date";
	else if(str == "342")
		strLabel = "Premium Paid to Date Begin";
	else if(str == "343")
		strLabel = "Premium Paid to Date End";
	else if(str == "346")
		strLabel = "Plan Begin Date";
	else if(str == "347")
		strLabel = "Plan End Date";
	else if(str == "348")
		strLabel = "Benefit Begin Date";
	else if(str == "349")
		strLabel = "Benefit End Date";
	else if(str == "356")
		strLabel = "Eligibility Begin Date";
	else if(str == "357")
		strLabel = "Eligibility End Date";
	else if(str == "382")
		strLabel = "Enrollment Date";
	else if(str == "435")
		strLabel = "Admission Date";
	else if(str == "442")
		strLabel = "Date of Death";
	else if(str == "458")
		strLabel = "Certification Date";
	else if(str == "472")
		strLabel = "Service Date";
	else if(str == "539")
		strLabel = "Policy Effective Date";
	else if(str == "540")
		strLabel = "Policy Expiration Date";
	else if(str == "636")
		strLabel = "Date of Last Update";
	else if(str == "771")
		strLabel = "Status Date";
	else {
		//Either we screwed up the parsing, or this is an illegal qualifier!
		ASSERT(FALSE);
		strLabel = "";
	}

	//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

	//D8 - Date Expressed in Format CCYYMMDD
	//RD8 - Range of Dates Expressed in Format CCYYMMDD-CCYYMMDD
	str = ParseElement(strIn);

	BOOL bTwoDates = str == "RD8";

	//DTP03	1251		Date Time Period						M	AN	1/35

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strOut;

		if(bTwoDates) {
			CString strDate1, strDate2;
			int nDash = str.Find("-");
			if(nDash != -1) {
				strDate1 = str.Left(nDash);
				strDate2 = str.Right(str.GetLength() - nDash - 1);

				strOut.Format("%s - %s", ConvertDateFormat(strDate1), ConvertDateFormat(strDate2));
			}
			else {
				//only one date after all?
				strOut = ConvertDateFormat(str);
			}
		}
		else {
			strOut = ConvertDateFormat(str);
		}

		strFmt.Format("%s: %s\r\n",strLabel,strOut);
		OutputData(OutputString, strFmt);

		// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
		// add to its ExtraMessage field
		pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
		if(pBenefitDetail) {
			if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
				pBenefitDetail->strExtraMessage += "\r\n";
			}
			strFmt.TrimRight("\r\n");
			pBenefitDetail->strExtraMessage += strFmt;
		}
		else {
			StoreOutputInMemory(strFmt);
		}
	}

	OutputData(OutputString, "\r\n");

	// (j.jones 2010-04-19 14:38) - PLID 38202 - if we have a benefit detail,
	// and it is suppressed, do not output
	if(pBenefitDetail == NULL || !pBenefitDetail->bSuppressFromOutput) {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

// (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment MPI
void CANSI271Parser::ANSI_MPI(CString &strIn)
{
	//SUBSCRIBER MILITARY PERSONNEL INFORMATION used in:
	//
	//1250 MPI Military Personnel Information O 9
	//1275 MPI Subscriber Military Personnel Information S 1 LOOP ID
	//1275 MPI Dependent Military Personnel Information S 1
	//1275 MPI Military Personnel Information O 9
	//
	CString OutputString, str, strFmt;

	//MPI01 1201 Information Status Code M 1 ID 1/1
	str = ParseElement(strIn);
	if(!str.IsEmpty()) {
		CString strCode;

		if (str == "A")
			strCode = "Partial";
		else if (str == "C")
			strCode = "Current";
		else if (str == "L")
			strCode = "Latest";
		else if (str == "O")
			strCode = "Oldest";
		else if (str == "P")
			strCode = "Prior";
		else if (str == "S")
			strCode = "Second Most Current";
		else if (str == "T")
			strCode = "Third Most Current";
		else
			strCode = CString("Unknown (") + str + ")";
	
		strFmt.Format("Information Status Code: %s\r\n",strCode);
		OutputData(OutputString, strFmt);	
	}

	//MPI02 584 Employment Status Code M 1 ID 2/2
	str = ParseElement(strIn);
	if(!str.IsEmpty()) {
		CString strCode;

		if (str == "AE")
			strCode = "Active Reserve";
		else if (str == "AO")
			strCode = "Active Military - Overseas";
		else if (str == "AS")
			strCode = "Academy Student";
		else if (str == "AT")
			strCode = "Presidential Appointee";
		else if (str == "AU")
			strCode = "Active Military - USA";
		else if (str == "CC")
			strCode = "Contractor";
		else if (str == "DD")
			strCode = "Dishonorably Discharged";
		else if (str == "HD")
			strCode = "Honorably Discharged";
		else if (str == "IR")
			strCode = "Inactive Reserves";
		else if (str == "LX")
			strCode = "Leave of Absence: Military";
		else if (str == "PE")
			strCode = "Plan to Enlist";
		else if (str == "RE")
			strCode = "Recommissioned";
		else if (str == "RM")
			strCode = "Retired Military - Overseas";
		else if (str == "RR")
			strCode = "Retired Without Recall";
		else
			strCode = CString("Unknown (") + str + ")";

		strFmt.Format("Employment Status Code: %s\r\n",strCode);
		OutputData(OutputString, strFmt);	
	}

	//MPI03 1595 Government Service Affiliation Code M 1 ID 1/1
	str = ParseElement(strIn);
	if(!str.IsEmpty()) {
		CString strCode;

		if (str == "A")
			strCode = "Air Force";
		else if (str == "B")
			strCode = "Air Force Reserves";
		else if (str == "C")
			strCode = "Army";
		else if (str == "D")
			strCode = "Army Reserves";
		else if (str == "E")
			strCode = "Coast Guard";
		else if (str == "F")
			strCode = "Marine Corps";
		else if (str == "G")
			strCode = "Marine Corps Reserves";
		else if (str == "H")
			strCode = "National Guard";
		else if (str == "I")
			strCode = "Navy";
		else if (str == "J")
			strCode = "Navy Reserves";
		else if (str == "K")
			strCode = "Other";
		else if (str == "L")
			strCode = "Peace Corp";
		else if (str == "M")
			strCode = "Regular Armed Forces";
		else if (str == "N")
			strCode = "Reserves";
		else if (str == "O")
			strCode = "U.S. Public Health Service";
		else if (str == "Q")
			strCode = "Foreign Military";
		else if (str == "R")
			strCode = "American Red Cross";
		else if (str == "S")
			strCode = "Department of Defense";
		else if (str == "U")
			strCode = "United Services Organization";
		else if (str == "W")
			strCode = "Military Sealift Command";
		else
			strCode = CString("Unknown (") + str + ")";

		strFmt.Format("Government Service Affiliation Code: %s\r\n",strCode);
		OutputData(OutputString, strFmt);	
	}

	//MPI04 352 Description O 1 AN 1/80
	str = ParseElement(strIn);

	//MPI05 1596 Military Service Rank Code O 1 ID 2/2
	str = ParseElement(strIn);

	//MPI06 1250 Date Time Period Format Qualifier X 1 ID 2/3
	str = ParseElement(strIn);

	//MPI07 1251 Date Time Period X 1 AN 1/35
	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI271Parser::ANSI_EB(CString &strIn) {

	// (j.jones 2010-03-24 17:20) - PLID 37870 - get the current response detail object
	EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
	if(pDetail == NULL) {
		//this should be impossible unless they received a seriously screwed up response file
		ASSERT(FALSE);
		ThrowNxException("EB loop found without a Subscriber or Dependent HL loop!");
	}

	//create the new benefit detail object (this will later be saved into EligibilityDetailsT)
	BenefitDetail *pBenefitDetail = new BenefitDetail;
	pDetail->arypBenefitDetails.Add(pBenefitDetail);
	// (c.haag 2010-11-03 10:18) - PLID 40350 - EB03 is a repeating element, so we have to track all the codes here.
	CStringArray astrEB03Codes;

	//Eligibility or Benefit Information, used in:

	//218		EB		Subscriber Eligibility or Benefit Information	S	1
	//295		EB		Dependent Eligibility or Benefit Information	S	1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//EB01	1390		Eligibility or Benefit Information		M	ID	1/2

	//1 - Active Coverage
	//2 - Active - Full Risk Capitation
	//3 - Active - Services Capitated
	//4 - Active - Services Capitated to Primary Care Physician
	//5 - Active - Pending Investigation
	//6 - Inactive
	//7 - Inactive - Pending Eligibility Update
	//8 - Inactive - Pending Investigation
	//A - Co-Insurance
	//B - Co-Payment
	//C - Deductible
	//CB - Coverage Basis
	//D - Benefit Description
	//E - Exclusions
	//F - Limitations
	//G - Out of Pocket (Stop Loss)
	//H - Unlimited
	//I - Non-Covered
	//J - Cost Containment
	//K - Reserve
	//L - Primary Care Provider
	//M - Pre-existing Condition
	//MC - Managed Care Coordinator
	//N - Services Restricted to Following Provider
	//O - Not Deemed a Medical Necessity
	//P - Benefit Disclaimer
	//Q - Second Surgical Opinion Required
	//R - Other or Additional Payor
	//S - Prior Year(s) History
	//T - Card(s) Reported Lost/Stolen
	//U - Contact Following Entity for Eligibility or Benefit Information
	//V - Cannot Process
	//W - Other Source of Data
	//X - Health Care Facility
	//Y - Spend Down
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
	
		CString strInfo;
		long nBenefitTypeRefID = -1;
		BOOL bExcludeFromOutput = FALSE;

		// (j.jones 2010-03-26 13:58) - PLID 37905 - this list is now tracked in EligibilityDataReferenceT,
		// tracked as ListType = 1
		GetBenefitTypeFromMap(str, nBenefitTypeRefID, strInfo, bExcludeFromOutput);

		// (j.jones 2010-04-19 14:38) - PLID 38202 - if excluded, flag the benefit as suppressed
		if(bExcludeFromOutput) {
			pBenefitDetail->bSuppressFromOutput = TRUE;
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->nBenefitTypeRefID = nBenefitTypeRefID;

		// (j.jones 2016-05-16 09:07) - NX-100357 - added the benefit type qualifier
		str.MakeUpper();
		pBenefitDetail->strBenefitTypeQualifier = str;

		if(!strInfo.IsEmpty()) {
			strFmt.Format("%s:\r\n", strInfo);
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//EB02	1207		Coverage Level Code						O	ID	3/3

	//CHD - Children Only
	//DEP - Dependents Only
	//ECH - Employee and Children
	//EMP - Employee Only
	//ESP - Employee and Spouse
	//FAM - Family
	//IND - Individual
	//SPC - Spouse and Children
	//SPO - Spouse Only
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strCoverage;
		long nCoverageLevelRefID = -1;
		BOOL bExcludeFromOutput = FALSE;

		// (j.jones 2010-03-26 13:58) - PLID 37905 - this list is now tracked in EligibilityDataReferenceT,
		// tracked as ListType = 2
		GetCoverageLevelFromMap(str, nCoverageLevelRefID, strCoverage, bExcludeFromOutput);

		// (j.jones 2010-04-19 14:38) - PLID 38202 - if excluded, flag the benefit as suppressed
		if(bExcludeFromOutput) {
			pBenefitDetail->bSuppressFromOutput = TRUE;
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->nCoverageLevelRefID = nCoverageLevelRefID;

		if(!strCoverage.IsEmpty()) {
			strFmt.Format("Coverage: %s\r\n", strCoverage);
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//EB03	1365		Service Type Code						O	ID	1/2

	/*
	1 Medical Care
	2 Surgical
	3 Consultation
	4 Diagnostic X-Ray
	5 Diagnostic Lab
	6 Radiation Therapy
	7 Anesthesia
	8 Surgical Assistance
	9 Other Medical
	10 Blood Charges
	11 Used Durable Medical Equipment
	12 Durable Medical Equipment Purchase
	13 Ambulatory Service Center Facility
	14 Renal Supplies in the Home
	15 Alternate Method Dialysis
	16 Chronic Renal Disease (CRD) Equipment
	17 Pre-Admission Testing
	18 Durable Medical Equipment Rental
	19 Pneumonia Vaccine
	20 Second Surgical Opinion
	21 Third Surgical Opinion
	22 Social Work
	23 Diagnostic Dental
	24 Periodontics
	25 Restorative
	26 Endodontics
	27 Maxillofacial Prosthetics
	28 Adjunctive Dental Services
	30 Health Benefit Plan Coverage
	32 Plan Waiting Period
	33 Chiropractic
	34 Chiropractic Office Visits
	35 Dental Care
	36 Dental Crowns
	37 Dental Accident
	38 Orthodontics
	39 Prosthodontics
	40 Oral Surgery
	41 Routine (Preventive) Dental
	42 Home Health Care
	43 Home Health Prescriptions
	44 Home Health Visits
	45 Hospice
	46 Respite Care
	47 Hospital
	48 Hospital - Inpatient
	49 Hospital - Room and Board
	50 Hospital - Outpatient
	51 Hospital - Emergency Accident
	52 Hospital - Emergency Medical
	53 Hospital - Ambulatory Surgical
	54 Long Term Care
	55 Major Medical
	56 Medically Related Transportation
	57 Air Transportation
	58 Cabulance
	59 Licensed Ambulance
	60 General Benefits
	61 In-vitro Fertilization
	62 MRI/CAT Scan
	63 Donor Procedures
	64 Acupuncture
	65 Newborn Care
	66 Pathology
	67 Smoking Cessation
	68 Well Baby Care
	69 Maternity
	70 Transplants
	71 Audiology Exam
	72 Inhalation Therapy
	73 Diagnostic Medical
	74 Private Duty Nursing
	75 Prosthetic Device
	76 Dialysis
	77 Otological Exam
	78 Chemotherapy
	79 Allergy Testing
	80 Immunizations
	81 Routine Physical
	82 Family Planning
	83 Infertility
	84 Abortion
	85 AIDS
	86 Emergency Services
	87 Cancer
	88 Pharmacy
	89 Free Standing Prescription Drug
	90 Mail Order Prescription Drug
	91 Brand Name Prescription Drug
	92 Generic Prescription Drug
	93 Podiatry
	94 Podiatry - Office Visits
	95 Podiatry - Nursing Home Visits
	96 Professional (Physician)
	97 Anesthesiologist
	98 Professional (Physician) Visit - Office
	99 Professional (Physician) Visit - Inpatient
	A0 Professional (Physician) Visit - Outpatient
	A1 Professional (Physician) Visit - Nursing Home
	A2 Professional (Physician) Visit - Skilled Nursing Facility
	A3 Professional (Physician) Visit - Home
	A4 Psychiatric
	A5 Psychiatric - Room and Board
	A6 Psychotherapy
	A7 Psychiatric - Inpatient
	A8 Psychiatric - Outpatient
	A9 Rehabilitation
	AA Rehabilitation - Room and Board
	AB Rehabilitation - Inpatient
	AC Rehabilitation - Outpatient
	AD Occupational Therapy
	AE Physical Medicine
	AF Speech Therapy
	AG Skilled Nursing Care
	AH Skilled Nursing Care - Room and Board
	AI Substance Abuse
	AJ Alcoholism
	AK Drug Addiction
	AL Vision (Optometry)
	AM Frames
	AN Routine Exam
	AO Lenses
	AQ Nonmedically Necessary Physical
	AR Experimental Drug Therapy
	BA Independent Medical Evaluation
	BB Partial Hospitalization (Psychiatric)
	BC Day Care (Psychiatric)
	BD Cognitive Therapy
	BE Massage Therapy
	BF Pulmonary Rehabilitation
	BG Cardiac Rehabilitation
	BH Pediatric
	BI Nursery
	BJ Skin
	BK Orthopedic
	BL Cardiac
	BM Lymphatic
	BN Gastrointestinal
	BP Endocrine
	BQ Neurology
	BR Eye
	BS Invasive Procedures
	// (c.haag 2010-10-19 16:07) - PLID 40350 - New codes
	BT Gynecological
	BU Obstetrical
	BV Obstetrical/Gynecological
	BW Mail Order Prescription Drug: Brand Name
	BX Mail Order Prescription Drug: Generic
	BY Physician Visit - Office: Sick
	BZ Physician Visit - Office: Well
	C1 Coronary Care
	CA Private Duty Nursing - Inpatient
	CB Private Duty Nursing - Home
	CC Surgical Benefits - Professional (Physician)
	CD Surgical Benefits - Facility
	CE Mental Health Provider - Inpatient
	CF Mental Health Provider - Outpatient
	CG Mental Health Facility - Inpatient
	CH Mental Health Facility - Outpatient
	CI Substance Abuse Facility - Inpatient
	CJ Substance Abuse Facility - Outpatient
	CK Screening X-ray
	CL Screening laboratory
	CM Mammogram, High Risk Patient
	CN Mammogram, Low Risk Patient
	CO Flu Vaccination
	CP Eyewear and Eyewear Accessories
	CQ Case Management
	DG Dermatology
	DM Durable Medical Equipment
	DS Diabetic Supplies
	GF Generic Prescription Drug - Formulary
	GN Generic Prescription Drug - Non-Formulary
	GY Allergy
	IC Intensive Care
	MH Mental Health
	NI Neonatal Intensive Care
	ON Oncology
	PT Physical Therapy
	PU Pulmonary
	RN Renal
	RT Residential Psychiatric Treatment
	TC Transitional Care
	TN Transitional Nursery Care
	UC Urgent Care
	*/
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strService;
		long nServiceTypeRefID = -1;
		BOOL bExcludeFromOutput = FALSE;

		// (c.haag 2010-11-03 08:37) - PLID 40350 - From the specs: "EB03 is a repeating data element 
		// that may be repeated up to 99 times." We must support this. We do so by checking to see if
		// it's repeated, and if it is, we will replicate the benefit information once for every unique code.
		ParseRepeatingElement(str, astrEB03Codes, TRUE);
		// Do the first code in pBenefitDetail
		str = astrEB03Codes[0];

		// (j.jones 2010-03-26 13:58) - PLID 37905 - this list is now tracked in EligibilityDataReferenceT,
		// tracked as ListType = 3
		GetServiceTypeFromMap(str, nServiceTypeRefID, strService, bExcludeFromOutput);

		// (j.jones 2010-04-19 14:38) - PLID 38202 - if excluded, flag the benefit as suppressed
		if(bExcludeFromOutput) {
			pBenefitDetail->bSuppressFromOutput = TRUE;
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->nServiceTypeRefID = nServiceTypeRefID;

		if(!strService.IsEmpty()) {
			strFmt.Format("Service Type: %s\r\n", strService);
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//EB04	1336		Insurance Type Code						O	ID	1/3

	//12 - Medicare Secondary Working Aged Beneficiary or Spouse with Employer Group Health Plan
	//13 - Medicare Secondary End-Stage Renal Disease Beneficiary in the 12 month coordination period with an employer’s group health plan
	//14 - Medicare Secondary, No-fault Insurance including Auto is Primary
	//15 - Medicare Secondary Worker’s Compensation
	//16 - Medicare Secondary Public Health Service (PHS) or Other Federal Agency
	//41 - Medicare Secondary Black Lung
	//42 - Medicare Secondary Veteran’s Administration
	//43 - Medicare Secondary Disabled Beneficiary Under Age 65 with Large Group Health Plan (LGHP)
	//47 - Medicare Secondary, Other Liability Insurance is Primary
	//AP - Auto Insurance Policy
	//C1 - Commercial
	//CO - Consolidated Omnibus Budget Reconciliation Act (COBRA)
	//CP - Medicare Conditionally Primary
	//D - Disability
	//DB - Disability Benefits
	//EP - Exclusive Provider Organization
	//FF - Family or Friends
	//GP - Group Policy
	//HM - Health Maintenance Organization (HMO)
	//HN - Health Maintenance Organization (HMO) - Medicare Risk
	//HS - Special Low Income Medicare Beneficiary
	//IN - Indemnity
	//IP - Individual Policy
	//LC - Long Term Care
	//LD - Long Term Policy
	//LI - Life Insurance
	//LT - Litigation
	//MA - Medicare Part A
	//MB - Medicare Part B
	//MC - Medicaid
	//MH - Medigap Part A
	//MI - Medigap Part B
	//MP - Medicare Primary
	//OT - Other
	//PE - Property Insurance - Personal
	//PL - Personal
	//PP - Personal Payment (Cash - No Insurance)
	//PR - Preferred Provider Organization (PPO)
	//PS - Point of Service (POS)
	//QM - Qualified Medicare Beneficiary
	//RP - Property Insurance - Real
	//SP - Supplemental Policy
	//TF - Tax Equity Fiscal Responsibility Act (TEFRA)
	//WC - Workers Compensation
	//WU - Wrap Up Policy
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strInsType;

		if(str == "12")
			strInsType = "Medicare Secondary Working Aged Beneficiary or Spouse with Employer Group Health Plan";
		else if(str == "13")
			strInsType = "Medicare Secondary End-Stage Renal Disease Beneficiary in the 12 month coordination period with an employer’s group health plan";
		else if(str == "14")
			strInsType = "Medicare Secondary, No-fault Insurance including Auto is Primary";
		else if(str == "15")
			strInsType = "Medicare Secondary Worker’s Compensation";
		else if(str == "16")
			strInsType = "Medicare Secondary Public Health Service (PHS) or Other Federal Agency";
		else if(str == "41")
			strInsType = "Medicare Secondary Black Lung";
		else if(str == "42")
			strInsType = "Medicare Secondary Veteran’s Administration";
		else if(str == "43")
			strInsType = "Medicare Secondary Disabled Beneficiary Under Age 65 with Large Group Health Plan (LGHP)";
		else if(str == "47")
			strInsType = "Medicare Secondary, Other Liability Insurance is Primary";
		else if(str == "AP")
			strInsType = "Auto Insurance Policy";
		else if(str == "C1")
			strInsType = "Commercial";
		else if(str == "CO")
			strInsType = "Consolidated Omnibus Budget Reconciliation Act (COBRA)";
		else if(str == "CP")
			strInsType = "Medicare Conditionally Primary";
		else if(str == "D")
			strInsType = "Disability";
		else if(str == "DB")
			strInsType = "Disability Benefits";
		else if(str == "EP")
			strInsType = "Exclusive Provider Organization";
		else if(str == "FF")
			strInsType = "Family or Friends";
		else if(str == "GP")
			strInsType = "Group Policy";
		else if(str == "HM")
			strInsType = "Health Maintenance Organization (HMO)";
		else if(str == "HN")
			strInsType = "Health Maintenance Organization (HMO)- Medicare Risk";
		else if(str == "HS")
			strInsType = "Special Low Income Medicare Beneficiary";
		else if(str == "IN")
			strInsType = "Indemnity";
		else if(str == "IP")
			strInsType = "Individual Policy";
		else if(str == "LC")
			strInsType = "Long Term Care";
		else if(str == "LD")
			strInsType = "Long Term Policy";
		else if(str == "LI")
			strInsType = "Life Insurance";
		else if(str == "LT")
			strInsType = "Litigation";
		else if(str == "MA")
			strInsType = "Medicare Part A";
		else if(str == "MB")
			strInsType = "Medicare Part B";
		else if(str == "MC")
			strInsType = "Medicaid";
		else if(str == "MH")
			strInsType = "Medigap Part A";
		else if(str == "MI")
			strInsType = "Medigap Part B";
		else if(str == "MP")
			strInsType = "Medicare Primary";
		else if(str == "OT")
			strInsType = "Other";
		else if(str == "PE")
			strInsType = "Property Insurance - Personal";
		else if(str == "PL")
			strInsType = "Personal";
		else if(str == "PP")
			strInsType = "Personal Payment (Cash - No Insurance)";
		else if(str == "PR")
			strInsType = "Preferred Provider Organization (PPO)";
		else if(str == "PS")
			strInsType = "Point of Service (POS)";
		else if(str == "QM")
			strInsType = "Qualified Medicare Beneficiary";
		else if(str == "RP")
			strInsType = "Property Insurance - Real";
		else if(str == "SP")
			strInsType = "Supplemental Policy";
		else if(str == "TF")
			strInsType = "Tax Equity Fiscal Responsibility Act (TEFRA)";
		else if(str == "WC")
			strInsType = "Workers Compensation";
		else if(str == "WU")
			strInsType = "Wrap Up Policy";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strInsType = "";
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strInsuranceType = strInsType;

		if(!strInsType.IsEmpty()) {
			strFmt.Format("Insurance Type: %s\r\n", strInsType);
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//EB05	1204		Plan Coverage Description				O	AN	1/50

	//this is free-text
	str = ParseElement(strIn);

	// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
	pBenefitDetail->strCoverageDesc = str;

	//EB06	615			Time Period Qualifier					O	ID	1/2

	//6 - Hour
	//7 - Day
	//13 - 24 Hours
	//21 - Years
	//22 - Service Year
	//23 - Calendar Year
	//24 - Year to Date
	//25 - Contract
	//26 - Episode
	//27 - Visit
	//28 - Outlier
	//29 - Remaining
	//30 - Exceeded
	//31 - Not Exceeded
	//32 - Lifetime
	//33 - Lifetime Remaining
	//34 - Month
	//35 - Week
	//36 - Admisson
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strTime;

		if(str == "6")
			strTime = "Hour";
		else if(str == "7")
			strTime = "Day";
		else if(str == "13")
			strTime = "24 Hours";
		else if(str == "21")
			strTime = "Years";
		else if(str == "22")
			strTime = "Service Year";
		else if(str == "23")
			strTime = "Calendar Year";
		else if(str == "24")
			strTime = "Year to Date";
		else if(str == "25")
			strTime = "Contract";
		else if(str == "26")
			strTime = "Episode";
		else if(str == "27")
			strTime = "Visit";
		else if(str == "28")
			strTime = "Outlier";
		else if(str == "29")
			strTime = "Remaining";
		else if(str == "30")
			strTime = "Exceeded";
		else if(str == "31")
			strTime = "Not Exceeded";
		else if(str == "32")
			strTime = "Lifetime";
		else if(str == "33")
			strTime = "Lifetime Remaining";
		else if(str == "34")
			strTime = "Month";
		else if(str == "35")
			strTime = "Week";
		else if(str == "36")
			strTime = "Admission";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strTime = "";
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strTimePeriod = strTime;

		if(!strTime.IsEmpty()) {
			strFmt.Format("Benefit Availability: %s\r\n", strTime);
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//EB07	782			Monetary Amount							O	R	1/18

	//Benefit Amount
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		COleCurrency cy;
		//attempt to parse it and print cleanly
		if(cy.ParseCurrency(str)) {
			strFmt.Format("Benefit Amount: %s\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));

			// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
			pBenefitDetail->varAmount = _variant_t(cy);
		}
		else {
			//otherwise just print what we received
			strFmt.Format("Benefit Amount: %s\r\n", str);
			ASSERT(FALSE);
		}
		OutputData(OutputString, "  *** " + strFmt);
	}
	
	//EB08	954			Percent									O	R	1/10

	//Benefit Percent
	str = ParseElement(strIn);
	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		double dblPercentage = (double)atof(str);
		dblPercentage = dblPercentage * 100.0;
		CString strPercentage;
		
		// (j.jones 2010-06-14 15:12) - PLID 39154 - increased the precision here
		strPercentage.Format("%0.09g", dblPercentage);
		pBenefitDetail->varPercentage = _bstr_t(strPercentage);

		strFmt.Format("Benefit Percent: %s%%\r\n", strPercentage);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB09	673			Quantity Qualifier						X	ID	2/2

	//99 - Quantity Used
	//CA - Covered - Actual
	//CE - Covered - Estimated
	//DB - Deductible Blood Units
	//DY - Days
	//HS - Hours
	//LA - Life-time Reserve - Actual
	//LE - Life-time Reserve - Estimated
	//MN - Month
	//P6 - Number of Services or Procedures
	//QA - Quantity Approved
	//S7 - Age, High Value
	//S8 - Age, Low Value
	//VS - Visits
	//YY - Years
	// (c.haag 2010-10-19 16:07) - PLID 40350
	//8H - Minimum
	//D3 - Number of Co-insurance Days
	//M2 - Maximum
	str = ParseElement(strIn);

	CString strQuantity;

	if(!str.IsEmpty()) {

		if(str == "99")
			strQuantity = "Quantity Used";
		else if(str == "CA")
			strQuantity = "Covered - Actual";
		else if(str == "CE")
			strQuantity = "Covered - Estimated";
		else if(str == "DB")
			strQuantity = "Deductible Blood Units";
		else if(str == "DY")
			strQuantity = "Days";
		else if(str == "HS")
			strQuantity = "Hours";
		else if(str == "LA")
			strQuantity = "Life-time Reserve - Actual";
		else if(str == "LE")
			strQuantity = "Life-time Reserve - Estimated";
		else if(str == "MN")
			strQuantity = "Month";
		else if(str == "P6")
			strQuantity = "Number of Services or Procedures";
		else if(str == "QA")
			strQuantity = "Quantity Approved";
		else if(str == "S7")
			strQuantity = "Age, High Value";
		else if(str == "S8")
			strQuantity = "Age, Low Value";
		else if(str == "VS")
			strQuantity = "Visits";
		else if(str == "YY")
			strQuantity = "Years";
		else if(str == "8H")
			strQuantity = "Minimum";
		else if(str == "D3")
			strQuantity = "Number of Co-insurance Days";
		else if(str == "M2")
			strQuantity = "Maximum";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strQuantity = "";
		}

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strQuantityType = strQuantity;
	}

	//EB10	380			Quantity								X	R	1/15
	
	//Benefit Quantity
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		double dblQuantity = (double)atof(str);
		pBenefitDetail->varQuantity = _variant_t(dblQuantity, VT_R8);

		// (j.jones 2010-06-14 15:12) - PLID 39154 - increased the precision here
		strFmt.Format("%s: %0.09g\r\n", strQuantity, dblQuantity);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB11	1073		Yes/No Condition or Response Code		O	ID	1/1

	//Authorization or Certification Indicator

	//N - No
	//U - Unknown
	//Y - Yes
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strYesNo;

		if(str == "N") {
			strYesNo = "No";
			pBenefitDetail->varAuthorized = g_cvarFalse;
		}
		else if(str == "Y") {
			strYesNo = "Yes";
			pBenefitDetail->varAuthorized = g_cvarTrue;
		}
		else {
			strYesNo = "Unknown";
			pBenefitDetail->varAuthorized = g_cvarNull;
		}
		
		strFmt.Format("Authorization / Certification Required: %s\r\n", strYesNo, str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB12	1073		Yes/No Condition or Response Code		O	ID	1/1

	//In Plan Network Indicator

	//N - No
	//U - Unknown
	//Y - Yes
	// (c.haag 2010-10-19 16:07) - PLID 40350 - New code
	//W - Not Applicable
	str = ParseElement(strIn);

	if(!str.IsEmpty()) {

		CString strYesNo;

		if(str == "N") {
			strYesNo = "No";
			pBenefitDetail->varInNetwork = g_cvarFalse;
		}
		else if(str == "Y") {
			strYesNo = "Yes";
			pBenefitDetail->varInNetwork = g_cvarTrue;
		}
		else if (str == "W") {
			strYesNo = "Not Applicable";
			pBenefitDetail->varInNetwork = g_cvarNull;
		}
		else {
			strYesNo = "Unknown";
			pBenefitDetail->varInNetwork = g_cvarNull;
		}
		
		strFmt.Format("In-Plan-Network: %s\r\n", strYesNo, str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB13	C003		COMPOSITE MEDICAL PROCEDURE IDENTIFIER

	CString strComposite = ParseElement(strIn);

	//EB13 - 1	235		Product/Service ID Qualifier			M	ID	2/2

	//AD - American Dental Association Codes
	//CJ - Current Procedural Terminology (CPT) Codes
	//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
	//ID - International Classification of Diseases Clinical Modification (ICD-9-CM) - Procedure
	//IV - Home Infusion EDI Coalition (HIEC) Product/Service (added in A1 Addendum)
	//N4 - National Drug Code in 5-4-2 Format
	//DO NOT USE - ND - National Drug Code (NDC) (removed in A1 Addendum)
	//ZZ - Mutually Defined
	str = ParseComposite(strComposite);

	//EB13 - 2	234		Product/Service ID						M	AN	1/48

	str = ParseComposite(strComposite);

	BOOL bNeedsNewLine = FALSE;

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strServiceCode = str;

		strFmt.Format("Service Code: %s\t", str);
		bNeedsNewLine = TRUE;
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB13 - 3	1339	Procedure Modifier						O	AN	2/2

	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strModifiers = str;

		strFmt.Format("Modifiers: %s  ", str);
		OutputData(OutputString, strFmt);
	}

	//EB13 - 4	1339	Procedure Modifier						O	AN	2/2

	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strModifiers += ",";
		pBenefitDetail->strModifiers += str;

		strFmt.Format("%s  ", str);
		bNeedsNewLine = TRUE;
		OutputData(OutputString, strFmt);
	}
	
	//EB13 - 5	1339	Procedure Modifier						O	AN	2/2

	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strModifiers += ",";
		pBenefitDetail->strModifiers += str;

		strFmt.Format("%s  ", str);
		OutputData(OutputString, strFmt);
	}

	//EB13 - 6	1339	Procedure Modifier						O	AN	2/2

	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		// (j.jones 2010-03-24 17:26) - PLID 37870 - track this value
		pBenefitDetail->strModifiers += ",";
		pBenefitDetail->strModifiers += str;

		strFmt.Format("%s  ", str);
		OutputData(OutputString, strFmt);
	}

	//EB13 - 7 NOT USED

	str = ParseComposite(strComposite);

	//EB13 - 8	234	Product/Service ID								O		1/48
	// (c.haag 2010-10-20 13:14) - PLID 41017 - Added for ANSI 5010. Track this value.
	str = ParseComposite(strComposite);
	if(!str.IsEmpty()) {

		pBenefitDetail->strServiceCodeRangeEnd = str;

		strFmt.Format("Service Code Range End: %s\t", str);
		bNeedsNewLine = TRUE;
		OutputData(OutputString, "  *** " + strFmt);
	}

	if(bNeedsNewLine) {
		OutputData(OutputString, "\r\n");
	}

	OutputData(OutputString, "\r\n");

	// (c.haag 2010-10-19 16:07) - PLID 40350 - New element EB14 and all its composites
	//EB14	C004		COMPOSITE DIAGNOSIS CODE POINTER	O		1

	strComposite = ParseElement(strIn);

	//EB14 - 1	1328	Diagnosis Code Pointer							M		1/2
	
	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		strFmt.Format("Diagnosis Code Pointer 1: %s\t", str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB14 - 2	1328	Diagnosis Code Pointer							M		1/2
	
	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		strFmt.Format("Diagnosis Code Pointer 2: %s\t", str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB14 - 3	1328	Diagnosis Code Pointer							M		1/2
	
	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		strFmt.Format("Diagnosis Code Pointer 3: %s\t", str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	//EB14 - 4	1328	Diagnosis Code Pointer							M		1/2
	
	str = ParseComposite(strComposite);

	if(!str.IsEmpty()) {

		strFmt.Format("Diagnosis Code Pointer 4: %s\t", str);
		OutputData(OutputString, "  *** " + strFmt);
	}

	OutputData(OutputString, "\r\n");


	// (c.haag 2010-11-03 10:18) - PLID 40350 - If EB03 has more than one element, then we need to clone
	// the benefit detail object once per additional element. It's important to do this now because all the
	// other fields for the benefit detail object have to be filled in by now.
	if (astrEB03Codes.GetSize() > 1)
	{
		for (int i=1; i < astrEB03Codes.GetSize(); i++) 
		{
			CString strService;
			long nServiceTypeRefID = -1;
			BenefitDetail *pBenefitDetailClone = new BenefitDetail(pBenefitDetail);
			pDetail->arypBenefitDetails.Add(pBenefitDetailClone);
			BOOL bExcludeFromOutput = FALSE;

			// (j.jones 2010-03-26 13:58) - PLID 37905 - this list is now tracked in EligibilityDataReferenceT,
			// tracked as ListType = 3
			GetServiceTypeFromMap(astrEB03Codes[i], nServiceTypeRefID, strService, bExcludeFromOutput);
			if(bExcludeFromOutput) {
				pBenefitDetailClone->bSuppressFromOutput = TRUE;
			}
			pBenefitDetailClone->nServiceTypeRefID = nServiceTypeRefID;

			if(!strService.IsEmpty()) {
				strFmt.Format("Additional Service Type: %s\r\n", strService);
				OutputData(OutputString, "  *** " + strFmt);
			}
		}
		OutputData(OutputString, "\r\n");
	}

	// (j.jones 2010-04-19 14:38) - PLID 38202 - if suppressed, do not output
	if(!pBenefitDetail->bSuppressFromOutput) {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

void CANSI271Parser::ANSI_HSD(CString &strIn) {

	//Health Care Services Delivery, used in:

	//233		HSD		Health Care Services Delivery			S		9
	//309		HSD		Health Care Services Delivery			S		9

	CString OutputString, str, strFmt;

	//indent all benefit information lines
	OutputData(OutputString, "  *** Benefit Usage:  ");
	
	//used for formatting later
	BOOL bData1Output = FALSE;
	BOOL bData2Output = FALSE;
	BOOL bData3Output = FALSE;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//HSD01	673			Quantity Qualifier						X	ID	2/2

	//DY - Days
	//FL - Units
	//HS - Hours
	//MN - Month
	//VS - Visits
	str = ParseElement(strIn);

	CString strQuantityDesc;

	if(!str.IsEmpty()) {

		if(str == "DY")
			strQuantityDesc = "Days";
		else if(str == "FL")
			strQuantityDesc = "Units";
		else if(str == "HS")
			strQuantityDesc = "Hours";
		else if(str == "MN")
			strQuantityDesc = "Months";
		else if(str == "VS")
			strQuantityDesc = "Visits";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strQuantityDesc = "";
		}
	}

	//HSD02	380			Quantity								X	R	1/15

	//Benefit Quantity
	CString strQuantity = ParseElement(strIn);

	if(!strQuantity.IsEmpty()) {
		strFmt.Format("%s %s", strQuantity, strQuantityDesc);
		OutputData(OutputString, strFmt);

		bData1Output = TRUE;
	}

	//HSD03 355			Unit or Basis for Measurement Code		O	ID	2/2

	//DA - Days
	//MO - Months
	//VS - Visit
	//WK - Week
	//YR - Years
	str = ParseElement(strIn);

	CString strMeasurementDesc;

	if(!str.IsEmpty()) {

		if(str == "DA")
			strMeasurementDesc = "Day";
		else if(str == "MO")
			strMeasurementDesc = "Month";
		else if(str == "VS")
			strMeasurementDesc = "Visit";
		else if(str == "WK")
			strMeasurementDesc = "Week";
		else if(str == "YR")
			strMeasurementDesc = "Year";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strMeasurementDesc = "";
		}
	}

	//HSD04	1167		Sample Selection Modulus				O	R	1/6

	CString strModulus = ParseElement(strIn);

	if(!strModulus.IsEmpty()) {
		strFmt.Format("%s%s %s per %s", bData1Output ? ", " : "", strModulus, strQuantityDesc, strMeasurementDesc);
		OutputData(OutputString, strFmt);

		bData2Output = TRUE;
	}

	//HSD05	615			Time Period Qualifier					X	ID	1/2

	//6 Hour
	//7 Day
	//21 Years
	//22 Service Year
	//23 Calendar Year
	//24 Year to Date
	//25 Contract
	//26 Episode
	//27 Visit
	//28 Outlier
	//29 Remaining
	//30 Exceeded
	//31 Not Exceeded
	//32 Lifetime
	//33 Lifetime Remaining
	//34 Month
	//35 Week
	str = ParseElement(strIn);

	CString strTime;

	if(!str.IsEmpty()) {

		if(str == "6")
			strTime = "Hour";
		else if(str == "7")
			strTime = "Day";
		else if(str == "21")
			strTime = "Years";
		else if(str == "22")
			strTime = "Service Year";
		else if(str == "23")
			strTime = "Calendar Year";
		else if(str == "24")
			strTime = "Year to Date";
		else if(str == "25")
			strTime = "Contract";
		else if(str == "26")
			strTime = "Episode";
		else if(str == "27")
			strTime = "Visit";
		else if(str == "28")
			strTime = "Outlier";
		else if(str == "29")
			strTime = "Remaining";
		else if(str == "30")
			strTime = "Exceeded";
		else if(str == "31")
			strTime = "Not Exceeded";
		else if(str == "32")
			strTime = "Lifetime";
		else if(str == "33")
			strTime = "Lifetime Remaining";
		else if(str == "34")
			strTime = "Month";
		else if(str == "35")
			strTime = "Week";
		else {
			//Either we screwed up the parsing, or this is an illegal qualifier!
			ASSERT(FALSE);
			strTime = "";
		}
	}

	//HSD06	616			Number of Periods						O	N0	1/3

	//Period Count
	CString strPeriodCount = ParseElement(strIn);

	if(!strPeriodCount.IsEmpty() || !strTime.IsEmpty()) {

		CString strIndent;
		if(bData2Output)
			strIndent = ", ";
		else if(bData1Output)
			strIndent = " ";

		if(!strPeriodCount.IsEmpty())
			strPeriodCount += " ";

		strFmt.Format("%sfor %s%s", strIndent, strPeriodCount, strTime);
		OutputData(OutputString, strFmt);

		bData3Output = TRUE;
	}

	//HSD07	678			Ship/Delivery or Calendar Pattern Code	O	ID	1/2

	//1 1st Week of the Month
	//2 2nd Week of the Month
	//3 3rd Week of the Month
	//4 4th Week of the Month
	//5 5th Week of the Month
	//6 1st & 3rd Weeks of the Month
	//7 2nd & 4th Weeks of the Month
	//8 1st Working Day of Period
	//9 Last Working Day of Period
	//A Monday through Friday
	//B Monday through Saturday
	//C Monday through Sunday
	//D Monday
	//E Tuesday
	//F Wednesday
	//G Thursday
	//H Friday
	//J Saturday
	//K Sunday
	//L Monday through Thursday
	//M Immediately
	//N As Directed
	//O Daily Mon. through Fri.
	//P 1/2 Mon. & 1/2 Thurs.
	//Q 1/2 Tues. & 1/2 Thurs.
	//R 1/2 Wed. & 1/2 Fri.
	//S Once Anytime Mon. through Fri.
	//SG Tuesday through Friday
	//SL Monday, Tuesday and Thursday
	//SP Monday, Tuesday and Friday
	//SX Wednesday and Thursday
	//SY Monday, Wednesday and Thursday
	//SZ Tuesday, Thursday and Friday
	//T 1/2 Tue. & 1/2 Fri.
	//U 1/2 Mon. & 1/2 Wed.
	//V 1/3 Mon., 1/3 Wed., 1/3 Fri.
	//W Whenever Necessary
	//X 1/2 By Wed., Bal. By Fri.
	//Y None (Also Used to Cancel or Override a Previous Pattern)
	CString strShipCalendarCode = ParseElement(strIn);

	//HSD08	679			Ship/Delivery Pattern Time Code			O	ID	1/1

	//A - 1st Shift (Normal Working Hours)
	//B - 2nd Shift
	//C - 3rd Shift
	//D - A.M.
	//E - P.M.
	//F - As Directed
	//G - Any Shift
	//Y - None (Also Used to Cancel or Override a Previous Pattern)
	CString strShipTimeCode = ParseElement(strIn);

	//store in memory
	strFmt = OutputString;
	strFmt.Replace("  *** ", "");

	// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
	// add to its ExtraMessage field
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if(pBenefitDetail) {
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strFmt;
	}
	else {
		StoreOutputInMemory(strFmt + "\r\n");
	}

	OutputData(OutputString, "\r\n\r\n");
	// (j.jones 2010-04-19 14:38) - PLID 38202 - if suppressed, do not output
	if(!pBenefitDetail->bSuppressFromOutput) {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

void CANSI271Parser::ANSI_MSG(CString &strIn) {

	BenefitDetail *pBenefitDetail = NULL;

	//Message Text, used in:

	//244		MSG		Message Text							S		10
	//320		MSG		Message Text							S		10

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//MSG01	933			Free-Form Message Text					M	AN	1/264

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("Message: %s\r\n", str);
		OutputData(OutputString, strFmt);

		//store in memory
		// (j.jones 2010-03-25 13:19) - PLID 37870 - if we have a benefit detail,
		// add to its ExtraMessage field
		pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
		if(pBenefitDetail) {
			if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
				pBenefitDetail->strExtraMessage += "\r\n";
			}
			pBenefitDetail->strExtraMessage += str;
		}
		else {
			StoreOutputInMemory(strFmt + "\r\n");
		}
	}

	//MSG02 NOT USED

	str = ParseElement(strIn);

	//MSG03 NOT USED

	str = ParseElement(strIn);

	OutputData(OutputString, "\r\n");
	// (j.jones 2010-04-19 14:38) - PLID 38202 - if suppressed, do not output
	if(!pBenefitDetail->bSuppressFromOutput) {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

void CANSI271Parser::ANSI_III(CString &strIn) {

	// (j.jones 2012-02-10 10:02) - PLID 48039 - this data is meaningless unless it
	// ties to a benefit detail, so return if we don't have one
	BenefitDetail *pBenefitDetail = GetCurrentResponseBenefitDetailInfo();
	if(pBenefitDetail == NULL) {
		return;
	}

	//Information, used in:

	//246		III		Subscriber Eligibility or Benefit Additional Information	S	1
	//322		III		Dependent Eligibility or Benefit Additional Information		S	1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//III01	1270		Code List Qualifier Code				X	ID	1/3

	//BF - Diagnosis
	//BK - Principal Diagnosis
	//ZZ - Mutually Defined
	//(c.haag 2010-10-19 15:43) - PLID 40350 - More codes. I did not find these on the 4010-5010 comparison
	//spreadsheet, but they were in the PDF so I'm adding them here.
	//GR - National Council on Compensation Insurance (NCCI) Nature of Injury Code
	//NI - Nature of Injury Code
	str = ParseElement(strIn);

	CString strCodeType;

	if(!str.IsEmpty()) {
		
		if(str == "BF")
			strCodeType = "Diagnosis";
		else if(str == "BK")
			strCodeType = "Principal Diagnosis";
		else if(str == "ZZ")
			strCodeType = "Facility Code";
		else if (str == "GR")
			strCodeType = "National Council on Compensation Insurance (NCCI) Nature of Injury Code";
		else if (str == "NI")
			strCodeType = "Nature of Injury Code";
	}

	//III02	1271		Industry Code							X	AN	1/30

	str = ParseElement(strIn);

	if(!str.IsEmpty()) {
		strFmt.Format("%s: %s\r\n", strCodeType, str);

		// (j.jones 2012-02-10 10:06) - PLID 48039 - tack on to our extra message
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strFmt;

		// (j.jones 2012-03-13 09:44) - PLID 48845 - if this benefit detail is hidden in the output,
		// this element should also be hidden
		if(!pBenefitDetail->bSuppressFromOutput) {
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//III03 NOT USED (4010)
	//III03 1136		Code Category							O	ID 2/2
	// (c.haag 2010-10-19 15:43) - PLID 40350 - III03 is situational as of 5010

	str = ParseElement(strIn);

	CString strCodeCategory;
	if(!str.IsEmpty()) {
		
		if(str == "44")
			strCodeCategory = "Nature of Injury";
		else 
			strCodeCategory = "(Unspecified category)";
	}

	//III04 NOT USED (4010)
	//III04 933			Free-form Message Text			X	AN 1/264
	// (c.haag 2010-10-19 15:43) - PLID 40350 - III04 is situational as of 5010
	str = ParseElement(strIn);

	if (!str.IsEmpty()) {
		strFmt.Format("%s: %s\r\n", strCodeCategory, str);

		// (j.jones 2012-02-10 10:06) - PLID 48039 - tack on to our extra message
		if(!pBenefitDetail->strExtraMessage.IsEmpty()) {
			pBenefitDetail->strExtraMessage += "\r\n";
		}
		pBenefitDetail->strExtraMessage += strFmt;

		// (j.jones 2012-03-13 09:44) - PLID 48845 - if this benefit detail is hidden in the output,
		// this element should also be hidden
		if(!pBenefitDetail->bSuppressFromOutput) {
			OutputData(OutputString, "  *** " + strFmt);
		}
	}

	//III05 NOT USED

	str = ParseElement(strIn);

	//III06 NOT USED

	str = ParseElement(strIn);

	//III07 NOT USED

	str = ParseElement(strIn);

	//III08 NOT USED

	str = ParseElement(strIn);

	//III09 NOT USED

	str = ParseElement(strIn);

	// (j.jones 2012-03-13 09:44) - PLID 48845 - if this benefit detail is hidden in the output,
	// this element should also be hidden
	if(!pBenefitDetail->bSuppressFromOutput) {
		OutputData(OutputString, "\r\n");
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}
}

void CANSI271Parser::ANSI_PRV(CString &strIn) {

	//Provider Information, used in:

	//261		PRV		Subscriber Benefit Related Provider Information		S		1
	//337		PRV		Dependent Benefit Related Provider Information		S		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//PRV01	1221		Provider Code							M	ID	1/3

	//AD - Admitting (Added in A1 Addendum)
	//AT - Attending
	//BI - Billing
	//CO - Consulting
	//CV - Covering
	//H - Hospital
	//HH - Home Health Care
	//LA - Laboratory
	//OT - Other Physician
	//P1 - Pharmacist
	//P2 - Pharmacy
	//PC - Primary Care Physician
	//PE - Performing
	//R - Rural Health Clinic
	//RF - Referring
	//SB - Submitting (Added in A1 Addendum)
	//SK - Skilled Nursing Facility
	//SU - Supervising (Added in A1 Addendum)
	str = ParseElement(strIn);

	//PRV02	128			Reference Identification Qualifier		M	ID	2/3

	//9K - Servicer
	//D3 - National Association of Boards of Pharmacy Number
	//EI - Employer’s Identification Number
	//HPI - Health Care Financing Administration National Provider Identifier
	//SY - Social Security Number
	//TJ - Federal Taxpayer’s Identification Number
	//ZZ - Mutually Defined
	//PXC - Health Care Provider Taxonomy Code (c.haag 2010-10-19 15:43) - PLID 40350
	str = ParseElement(strIn);

	//PRV03	127			Reference Identification				M	AN	1/30

	str = ParseElement(strIn);

	//PRV04	NOT USED

	str = ParseElement(strIn);

	//PRV06	NOT USED

	str = ParseElement(strIn);

	//PRV06	NOT USED

	str = ParseElement(strIn);
}

void CANSI271Parser::ANSI_LS(CString &strIn) {

	//Loop Header, used in:

	//249		LS		Loop Header								S		1
	//325		LS		Dependent Eligibility or Benefit Information	S	1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//LS01	447			Loop Identifier Code					M	AN	1/6

	//must be "2120"
	str = ParseElement(strIn);
	// (j.jones 2016-05-17 15:38) - NX-100668 - track when we are entering a 2120 loop
	if (str == "2120") {
		m_bCurLoop2120 = true;
	}
}

void CANSI271Parser::ANSI_LE(CString &strIn) {

	//Loop Trailer, used in:

	//264		LE		Loop Trailer								S		1
	//340		LE		Loop Trailer								S		1

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//LE01	447			Loop Identifier Code					M	AN	1/6

	//must be "2120"
	str = ParseElement(strIn);
	// (j.jones 2016-05-17 15:38) - NX-100668 - track when we are leaving a 2120 loop
	if (str == "2120") {
		m_bCurLoop2120 = false;
	}
}

//compares the trace string to EligibilityRequestsT.ID, and sets it in the detail if it exists
void CANSI271Parser::CalculateIDFromTrace1(EligibilityResponseDetailInfo *pDetail, CString strTrace1)
{
	//if already filled in, do nothing
	if(pDetail->nEligibilityRequestID != -1) {
		ASSERT(FALSE);
		return;
	}

	//now, check the trace against EligibilityRequestsT.ID

	long nID = atol(strTrace1);
	
	if(nID > 0 && ReturnsRecords("SELECT ID FROM EligibilityRequestsT WHERE ID = %li", nID)) {
		//it's a valid request ID
		pDetail->nEligibilityRequestID = nID;
	}
	else {
		ASSERT(FALSE);
		Log("**** Trace Line 1: %s could not match up with an eligibility request ID.", strTrace1);
	}

	//otherwise, this is not the trace number we are looking for
}

//compares the trace string to PatientsT.PersonID and InsuredPartyT.PersonID, and sets them in the detail if they exist
void CANSI271Parser::CalculateIDFromTrace2(EligibilityResponseDetailInfo *pDetail, CString strTrace2)
{
	//if both are already filled in, do nothing
	if(pDetail->nPatientID != -1 && pDetail->nInsuredPartyID != -1)
		return;

	//now, check the trace against PatientsT.PersonID and InsuredPartyT.PersonID

	int nDash = strTrace2.Find("-");

	if(nDash == -1) {
		//if no dash, then this can't be the trace number we sent
		ASSERT(FALSE);
		Log("**** Trace Line 2: %s could not match up with a patient and insurance ID.", strTrace2);
		return;
	}

	long nPatID = atol(strTrace2.Left(nDash));
	// (j.jones 2009-07-31 13:10) - PLID 35072 - fixed the parsing of the insurance ID
	long nInsID = atol(strTrace2.Right(strTrace2.GetLength() - nDash - 1));
	
	// (j.jones 2009-02-16 14:44) - PLID 33120 - fixed so it selected PersonID, not ID!
	if(nPatID > 0 && nInsID > 0 && ReturnsRecords("SELECT PersonID FROM InsuredPartyT WHERE PersonID = %li AND PatientID = %li", nInsID, nPatID)) {
		//they are valid IDs
		pDetail->nPatientID = nPatID;
		pDetail->nInsuredPartyID = nInsID;
	}
	else {
		ASSERT(FALSE);
		Log("**** Trace Line 2: %s could not match up with a patient and insurance ID.", strTrace2);
	}

	//otherwise, this is not the trace number we are looking for
}

//if we don't have an eligibility request ID from a trace, but we do have an
//insured party ID, calculate the likely request ID
void CANSI271Parser::CalculateRequestID(EligibilityResponseDetailInfo *pDetail)
{
	//can't do anything if we don't have an insured party ID
	if(pDetail->nEligibilityRequestID == -1 && pDetail->nInsuredPartyID != -1) {

		//this in theory shouldn't happen, but if it does, fall back to matching to the latest
		//request that had been sent, by insured party ID

		//sort by LastSent descending, whether we have a response,
		//and EligibilityRequestsT.ID descending, such that we are looking
		//for the most recent request sent for this insured party,
		//and if multiple were sent at once then prioritize by those without responses,
		//then if we still have exact matches then order by their request creation order, the highest ID
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 EligibilityRequestsT.ID FROM EligibilityRequestsT "
			"LEFT JOIN EligibilityResponsesT ON EligibilityRequestsT.ID = EligibilityResponsesT.RequestID "
			"WHERE InsuredPartyID = {INT} "
			"ORDER BY EligibilityRequestsT.LastSentDate DESC, "
			"CASE WHEN EligibilityResponsesT.ID IS NULL THEN 0 ELSE 1 END ASC, "
			"EligibilityRequestsT.ID DESC", pDetail->nInsuredPartyID);

		if(!rs->eof) {

			pDetail->nEligibilityRequestID = AdoFldLong(rs, "ID",-1);
		}
		rs->Close();
	}
}

//appends the text to either the current master response text, or the current detail response text,
//based on the m_hllCurHLLevel value
void CANSI271Parser::StoreOutputInMemory(CString strText)
{
	//and store in memory
	if(m_hllCurHLLevel == hllSource || m_hllCurHLLevel == hllReceiver) {
		EligibilityResponseMasterInfo *pMaster = GetCurrentResponseMasterInfo();
		if(pMaster) {
			pMaster->strResponseMasterInfo += strText;
		}
	}
	else if(m_hllCurHLLevel == hllSubscriber || m_hllCurHLLevel == hllPatient) {
		EligibilityResponseDetailInfo *pDetail = GetCurrentResponseDetailInfo();
		if(pDetail) {
			pDetail->strResponseDetailInfo += strText;
		}
	}
}

// (j.jones 2010-03-26 13:34) - PLID 37905 - return the RefID and description, load map on first use
// (j.jones 2010-04-19 10:56) - PLID 38202 - added return value for whether the type is excluded from the output
void CANSI271Parser::GetBenefitTypeFromMap(CString strQualifier, long &nBenefitTypeRefID, CString &strDescription, BOOL &bExcludeFromOutput)
{
	try {

		nBenefitTypeRefID = -1;
		strDescription = "";

		if(strQualifier.IsEmpty()) {
			//not allowed
			return;
		}

		if(!m_bBenefitTypesFilled) {
			//first use, fill the maps

			m_mapBenefitTypeQualifiersToIDs.RemoveAll();
			m_mapBenefitTypeQualifiersToDescriptions.RemoveAll();
			// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
			m_mapBenefitTypeQualifiersToExcluded.RemoveAll();

			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Qualifier, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 1");
			while(!rs->eof) {

				long nID = AdoFldLong(rs, "ID");
				CString strLoadQualifier = AdoFldString(rs, "Qualifier");
				CString strLoadDescription = AdoFldString(rs, "Description");
				BOOL bFilterExcluded = AdoFldBool(rs, "FilterExcluded");

				m_mapBenefitTypeQualifiersToIDs.SetAt(strLoadQualifier, nID);
				m_mapBenefitTypeQualifiersToDescriptions.SetAt(strLoadQualifier, strLoadDescription);
				// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
				m_mapBenefitTypeQualifiersToExcluded.SetAt(strLoadQualifier, bFilterExcluded);				

				rs->MoveNext();
			}
			rs->Close();

			m_bBenefitTypesFilled = TRUE;
		}

		//now try to look up the values in our maps
		m_mapBenefitTypeQualifiersToIDs.Lookup(strQualifier, nBenefitTypeRefID);
		m_mapBenefitTypeQualifiersToDescriptions.Lookup(strQualifier, strDescription);

		// (j.jones 2010-04-19 10:59) - PLID 38202 - if we are suppressing excluded items,
		// see if this item is excluded
		bExcludeFromOutput = FALSE;
		if(m_bSuppressExcludedCoverage) {
			m_mapBenefitTypeQualifiersToExcluded.Lookup(strQualifier, bExcludeFromOutput);
		}

		if(nBenefitTypeRefID == -1) {
			//if we found no result, we were given an unknown qualifier!
			ASSERT(strQualifier.IsEmpty());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-26 13:34) - PLID 37905 - return the RefID and description, load map on first use
// (j.jones 2010-04-19 10:56) - PLID 38202 - added return value for whether the type is excluded from the output
void CANSI271Parser::GetCoverageLevelFromMap(CString strQualifier, long &nCoverageLevelRefID, CString &strDescription, BOOL &bExcludeFromOutput)
{
	try {

		nCoverageLevelRefID = -1;
		strDescription = "";

		if(strQualifier.IsEmpty()) {
			//not allowed
			return;
		}

		if(!m_bCoverageLevelsFilled) {
			//first use, fill the maps

			m_mapCoverageLevelQualifiersToIDs.RemoveAll();
			m_mapCoverageLevelQualifiersToDescriptions.RemoveAll();
			// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
			m_mapCoverageLevelQualifiersToExcluded.RemoveAll();

			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Qualifier, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 2");
			while(!rs->eof) {

				long nID = AdoFldLong(rs, "ID");
				CString strLoadQualifier = AdoFldString(rs, "Qualifier");
				CString strLoadDescription = AdoFldString(rs, "Description");
				BOOL bFilterExcluded = AdoFldBool(rs, "FilterExcluded");

				m_mapCoverageLevelQualifiersToIDs.SetAt(strLoadQualifier, nID);
				m_mapCoverageLevelQualifiersToDescriptions.SetAt(strLoadQualifier, strLoadDescription);
				// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
				m_mapCoverageLevelQualifiersToExcluded.SetAt(strLoadQualifier, bFilterExcluded);

				rs->MoveNext();
			}
			rs->Close();

			m_bCoverageLevelsFilled = TRUE;
		}

		//now try to look up the values in our maps
		m_mapCoverageLevelQualifiersToIDs.Lookup(strQualifier, nCoverageLevelRefID);
		m_mapCoverageLevelQualifiersToDescriptions.Lookup(strQualifier, strDescription);

		// (j.jones 2010-04-19 10:59) - PLID 38202 - if we are suppressing excluded items,
		// see if this item is excluded
		bExcludeFromOutput = FALSE;
		if(m_bSuppressExcludedCoverage) {
			m_mapCoverageLevelQualifiersToExcluded.Lookup(strQualifier, bExcludeFromOutput);
		}

		if(nCoverageLevelRefID == -1 && !strQualifier.IsEmpty()) {
			//if we found no result, we were given an unknown qualifier!
			ASSERT(strQualifier.IsEmpty());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-26 13:34) - PLID 37905 - return the RefID and description, load map on first use
// (j.jones 2010-04-19 10:56) - PLID 38202 - added return value for whether the type is excluded from the output
void CANSI271Parser::GetServiceTypeFromMap(CString strQualifier, long &nServiceTypeRefID, CString &strDescription, BOOL &bExcludeFromOutput)
{
	try {

		nServiceTypeRefID = -1;
		strDescription = "";

		if(strQualifier.IsEmpty()) {
			//not allowed
			return;
		}

		if(!m_bServiceTypesFilled) {
			//first use, fill the maps

			m_mapServiceTypeQualifiersToIDs.RemoveAll();
			m_mapServiceTypeQualifiersToDescriptions.RemoveAll();
			// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
			m_mapServiceTypeQualifiersToExcluded.RemoveAll();

			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Qualifier, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 3");
			while(!rs->eof) {

				long nID = AdoFldLong(rs, "ID");
				CString strLoadQualifier = AdoFldString(rs, "Qualifier");
				CString strLoadDescription = AdoFldString(rs, "Description");
				BOOL bFilterExcluded = AdoFldBool(rs, "FilterExcluded");

				m_mapServiceTypeQualifiersToIDs.SetAt(strLoadQualifier, nID);
				m_mapServiceTypeQualifiersToDescriptions.SetAt(strLoadQualifier, strLoadDescription);
				// (j.jones 2010-04-19 10:52) - PLID 38202 - added map of excluded qualifiers
				m_mapServiceTypeQualifiersToExcluded.SetAt(strLoadQualifier, bFilterExcluded);

				rs->MoveNext();
			}
			rs->Close();

			m_bServiceTypesFilled = TRUE;
		}

		//now try to look up the values in our maps
		m_mapServiceTypeQualifiersToIDs.Lookup(strQualifier, nServiceTypeRefID);
		m_mapServiceTypeQualifiersToDescriptions.Lookup(strQualifier, strDescription);

		// (j.jones 2010-04-19 10:59) - PLID 38202 - if we are suppressing excluded items,
		// see if this item is excluded
		bExcludeFromOutput = FALSE;
		if(m_bSuppressExcludedCoverage) {
			m_mapServiceTypeQualifiersToExcluded.Lookup(strQualifier, bExcludeFromOutput);
		}

		if(nServiceTypeRefID == -1) {
			//if we found no result, we were given an unknown qualifier!
			ASSERT(strQualifier.IsEmpty());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-02 13:52) - PLID 39499 - all actual importing to Practice data is now done inside this class,
// and we now fill aryRequestIDsUpdated with a list of unique request IDs that we just imported to
//this also fills an array of the new responses we just imported
BOOL CANSI271Parser::ImportParsedContentToData(BOOL &bMustOpenNotepadFile, BOOL bIsRealTimeImport, OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned)
{
	// (j.jones 2010-12-01 10:38) - PLID 41661 - increase the timeout for this operation
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
	CIncreaseCommandTimeout ict(600);

	try {

		// (j.jones 2007-06-26 14:20) - PLID 25868 - now save the information into the database

		BOOL bWarnedFailure = FALSE;

		//We will make two passes through the eligibility responses:
		//Pass 1: validate the data, and get an ID list of the request IDs
		//interim: use one recordset to calculate mailsent IDs
		//Pass 2: save to data

		//Pass 1: verify the requests are sound, warn if not, and get a list of request IDs
		
		//TES 11/5/2007 - PLID 27978 - VS 2008 - for() loops
		int i = 0;
		for(i=0;i<m_paryResponses.GetSize();i++) {

			EligibilityResponseMasterInfo *pMaster = (EligibilityResponseMasterInfo*)m_paryResponses.GetAt(i);

			for(int j=0;j<pMaster->arypResponses.GetSize();j++) {
				EligibilityResponseDetailInfo *pDetail = (EligibilityResponseDetailInfo*)pMaster->arypResponses.GetAt(j);

				long nRequestID = pDetail->nEligibilityRequestID;

				BOOL bFailed = FALSE;

				//do we actually have an ID?
				if(nRequestID == -1) {

					// (j.jones 2010-11-05 09:45) - PLID 41341 - if this was a real-time send, the request
					// and response should always be matched up, so ASSERT, as this should not be possible
					if(bIsRealTimeImport) {
						ASSERT(FALSE);
					}

					//no, which means we didn't get our trace back, let's see if we got our insured party ID back
					long nInsuredPartyID = pDetail->nInsuredPartyID;
					if(nInsuredPartyID == -1) {
						//we have no information, we cannot attach this response to a patient
						bFailed = TRUE;
					}
					else {

						//we have an insured party ID, so calculate the request ID from the insured party ID
						//note: this runs a recordset, but it shouldn't actually be called in normal circumstances
						CalculateRequestID(pDetail);

						//try this again
						nRequestID = pDetail->nEligibilityRequestID;

						//now is it valid?
						if(nRequestID == -1) {
							//we have no information, we cannot attach this response to a patient
							bFailed = TRUE;
						}
					}

					if(bFailed) {
						if(!bWarnedFailure) {
							AfxMessageBox("At least one eligibility response could not be matched up with a patient's eligibility request,\n"
								"because the clearinghouse failed to return critical information in the response.\n\n"
								"Please refer to the Eligibility Response file in Notepad to view all the returned responses,\n"
								"and contact NexTech technical support so this issue can be investigated further.");
							bWarnedFailure = TRUE;
						}
						continue;
					}
				}

				//alrighty, now we know we have a request ID, so that's half the battle over
				// (j.jones 2009-07-31 13:27) - PLID 35074 - track the request IDs in an array, uniquely
				// (j.jones 2010-07-07 10:28) - PLID 39499 - now this array is a parameter
				if(nRequestID != -1) {

					BOOL bFound = FALSE;
					for(int z=0; z < (long)aryRequestIDsUpdated.size() && !bFound; z++) {
						if(aryRequestIDsUpdated.at(z) == nRequestID) {
							bFound = TRUE;
						}
					}

					if(!bFound) {
						aryRequestIDsUpdated.push_back(nRequestID);
					}
				}
			}
		}

		if(aryRequestIDsUpdated.size() == 0) {
			//that's not good
			AfxMessageBox("The eligibility response could not be imported into Practice because no matching eligibility requests were found.");
			return FALSE;
		}
		
		CString strRequestInClause = "";
		CString strTempTable = "";

		// (j.jones 2010-12-01 10:46) - PLID 41661 - if we are importing more than 20 responses, use
		// a temp table instead of a comma delimited list of IDs
		if(aryRequestIDsUpdated.size() > 20) {
			//make a temp table
			
			CStringArray saXMLStatements;
			CStringArray aryFieldNames;
			CStringArray aryFieldTypes;
			CString strIn = "<ROOT>";

			for(i=0; i < (long)aryRequestIDsUpdated.size(); i++) {
				CString str;
				str.Format("<P ID=\"%li\"/>", aryRequestIDsUpdated.at(i));
				strIn += str;
				//Limit the size of a single XML statement, otherwise it can cause errors.
				if(strIn.GetLength() > 2000) {
					//End this statement.
					strIn += "</ROOT>";
					saXMLStatements.Add(strIn);
					//Start a new one.
					strIn = "<ROOT>";
				}
			}

			if(!strIn.IsEmpty()) {
				//now add the trailer
				strIn += "</ROOT>";
				saXMLStatements.Add(strIn);
			}

			aryFieldNames.Add("ID");
			aryFieldTypes.Add("int");

			//now create our temp table
			strTempTable = CreateTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(0));
			for(int i = 1; i < saXMLStatements.GetSize(); i++) {
				AppendToTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(i), strTempTable);
			}
			strRequestInClause.Format("SELECT ID FROM %s", strTempTable);
		}
		else {
			//simply search on a comma delimited list
			CString strRequestIDs;
			for(i=0; i < (long)aryRequestIDsUpdated.size(); i++) {
				strRequestIDs += AsString(aryRequestIDsUpdated.at(i)) + ",";
			}
			strRequestIDs.TrimRight(",");
			strRequestInClause = strRequestIDs;
		}

		//now, with our list of request IDs, calculate the mailsent variables all at once

		//need to pull information from the eligibility request

		CMap<long, long, long, long> mapPatientIDsToRequests;
		CMap<long, long, long, long> mapLocationIDsToRequests;
		CMap<long, long, CString, CString> mapInsCoNamesToRequests;

		CString strSql;
		strSql.Format("SELECT EligibilityRequestsT.ID AS RequestID, "
			"PatientsT.PersonID AS PatientID, "
			"EligibilityRequestsT.LocationID, "
			"InsuranceCoT.Name AS InsCoName "
			"FROM EligibilityRequestsT "
			"LEFT JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
			"WHERE EligibilityRequestsT.ID IN (%s)", strRequestInClause);
		_RecordsetPtr rs = CreateParamRecordset(strSql);

		if(rs->eof) {
			//that's not good
			AfxMessageBox("The eligibility response file could not be imported into Practice because no matching eligibility requests were found.");

			//drop our temp table, if we have one
			if(!strTempTable.IsEmpty()) {
				ExecuteSql("DROP TABLE %s", _Q(strTempTable));
			}

			return FALSE;
		}

		// (j.jones 2009-07-31 13:27) - PLID 35074 - warn if we don't find enough requests in data
		if(rs->GetRecordCount() < (long)aryRequestIDsUpdated.size()) {
			CString str;
			str.Format("Some eligibility responses (%li out of %li) could not be matched up with a patient's eligibility request.\n\n"
				"Please refer to the Eligibility Response file in Notepad to view all the returned responses,\n"
				"and contact NexTech technical support so this issue can be investigated further.",
				(long)aryRequestIDsUpdated.size() - rs->GetRecordCount(), (long)aryRequestIDsUpdated.size());
			Log("**** %s", str);
			AfxMessageBox(str);
			bMustOpenNotepadFile = TRUE;
		}

		while(!rs->eof) {
			long nRequestID = AdoFldLong(rs, "RequestID",-1);
			long nPatientID = AdoFldLong(rs, "PatientID",-1);
			long nLocationID = AdoFldLong(rs, "LocationID",-1);
			//TES 11/5/2007 - PLID 27978 - VS 2008 - I'm not quite sure how VS 6.0 didn't catch this, but this
			// line was passing in -1 as the default.
			CString strInsuranceCoName = AdoFldString(rs, "InsCoName","");

			//store in maps
			mapPatientIDsToRequests.SetAt(nRequestID, nPatientID);
			mapLocationIDsToRequests.SetAt(nRequestID, nLocationID);
			mapInsCoNamesToRequests.SetAt(nRequestID, strInsuranceCoName);

			rs->MoveNext();
		}
		rs->Close();

		//drop our temp table, if we have one
		if(!strTempTable.IsEmpty()) {
			ExecuteSql("DROP TABLE %s", _Q(strTempTable));
		}

		//Pass 2: save to data

		//make a new record for each master/detail combination

		CString strSqlBatch;
		BOOL bDeclarationsMade = FALSE;

		for(i=0;i<m_paryResponses.GetSize();i++) {

			EligibilityResponseMasterInfo *pMaster = (EligibilityResponseMasterInfo*)m_paryResponses.GetAt(i);

			for(int j=0;j<pMaster->arypResponses.GetSize();j++) {
				EligibilityResponseDetailInfo *pDetail = (EligibilityResponseDetailInfo*)pMaster->arypResponses.GetAt(j);

				long nRequestID = pDetail->nEligibilityRequestID;

				if(nRequestID == -1) {
					//already warned about this, so just skip it now
					continue;
				}

				//need to pull information from our maps
				long nPatientID = -1;
				long nLocationID = -1;
				CString strInsuranceCoName = "";
				BOOL bFailed = FALSE;
				if(!mapPatientIDsToRequests.Lookup(nRequestID, nPatientID)) {
					bFailed = TRUE;
				}
				if(!mapLocationIDsToRequests.Lookup(nRequestID, nLocationID)) {
					bFailed = TRUE;
				}
				if(!mapInsCoNamesToRequests.Lookup(nRequestID, strInsuranceCoName)) {
					bFailed = TRUE;
				}
				
				if(bFailed) {
					//whoa, that's no good, it means we didn't find a valid request in the data
					//which means the request is non-importable!
					CString strLog;
					strLog.Format("***Could not match up an Eligibility response with request ID %li", nRequestID);
					Log(strLog);
					continue;
				}
				
				//next, build the Response text
				CString strResponse;
				strResponse += pMaster->strResponseMasterInfo;
				if(!strResponse.IsEmpty()) {
					strResponse += "\r\n";
				}
				strResponse += pDetail->strResponseDetailInfo;
				strResponse.TrimRight("\r\n");


				// (j.jones 2007-06-27 17:58) - PLID 26480 - converted this save to also create MailSent entries

				//have we made our SQL declarations yet?
				if(!bDeclarationsMade) {
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewResponseID INT");
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewMailSentID int");
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewMailBatchID int");
					// (j.jones 2014-08-04 13:30) - PLID 63141 - we now track the patient ID too
					//this tracks the new response IDs, MailSentID, and the patient ID
					//While nullable, realistically these should never be null.
					//This is enforced outside the transaction rather than inside, such if
					//one response has null issues, it won't stop the entire batch from saving.
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewResponseIDs TABLE (ResponseID INT, MailSentID INT, PatientID INT)");
					//initialize the IDs

					// (j.dinatale 2012-02-06 10:35) - PLID 39756 - solve deadlocks!
					AddStatementToSqlBatch(strSqlBatch, "SELECT @nNewMailBatchID = COALESCE(MAX(MailBatchID), 0) + 1 FROM MailSent WITH (UPDLOCK, HOLDLOCK);");

					bDeclarationsMade = TRUE;
				}
				else {
					//increment the IDs
					AddStatementToSqlBatch(strSqlBatch, "SET @nNewMailBatchID = @nNewMailBatchID + 1");
				}

				//and finally add the insert statement
				// (j.jones 2009-12-17 14:54) - PLID 36641 - save the confirmation status
				// (j.armen 2014-01-30 14:36) - PLID 60533 - Idenitate EligibilityResponsesT
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EligibilityResponsesT (RequestID, Response, ConfirmationStatus) "
					"VALUES (%li, '%s', %li)",nRequestID, _Q(strResponse), (long)(pDetail->ercsStatus));

				AddStatementToSqlBatch(strSqlBatch, "SET @nNewResponseID = SCOPE_IDENTITY()");

				// (j.jones 2010-07-06 11:31) - PLID 39499 - If a Trizetto SOAP Import,
				// follow the preference to unbatch or unselect ONLY if the response is Confirmed.
				// Do not touch the batched status if Denied or Invalid
				if(bIsRealTimeImport && pDetail->ercsStatus == ercsConfirmed) {
					
					//this is cached by the caller
					long nGEDIUnbatchOnReceive = GetRemotePropertyInt("GEDIEligibilityRealTime_UnbatchOnReceive", 1, 0, "<None>", true);
					//1 - unbatch, 0 - unselect (which we do anyways if unbatching)
					AddStatementToSqlBatch(strSqlBatch, "UPDATE EligibilityRequestsT SET Batched = %li, Selected = 0 WHERE ID = %li",
						nGEDIUnbatchOnReceive == 1 ? 0 : 1, nRequestID);
				}

				// (j.jones 2010-03-24 17:08) - PLID 37870 - added EligibilityResponseDetailsT
				for(int k=0;k<pDetail->arypBenefitDetails.GetSize();k++) {
					BenefitDetail *pBenefitDetail = (BenefitDetail*)pDetail->arypBenefitDetails.GetAt(k);

					CString strAmount = "NULL";
					if(pBenefitDetail->varAmount.vt == VT_CY) {
						// (j.jones 2011-12-21 10:15) - PLID 47140 - fixed bug where this had called FormatCurrencyForInterface
						strAmount.Format("Convert(money, '%s')", _Q(FormatCurrencyForSql(VarCurrency(pBenefitDetail->varAmount))));
					}

					CString strPercentage = "NULL";
					if(pBenefitDetail->varPercentage.vt == VT_BSTR) {
						//this is a float value, not a real string, but formatted as text
						strPercentage = VarString(pBenefitDetail->varPercentage);
					}

					CString strQuantity = "NULL";
					if(pBenefitDetail->varQuantity.vt == VT_R8) {
						// (j.jones 2010-06-14 15:12) - PLID 39154 - increased the precision here
						strQuantity.Format("%0.09g", VarDouble(pBenefitDetail->varQuantity));
					}

					CString strAuthorized = "NULL";
					if(pBenefitDetail->varAuthorized.vt == VT_BOOL) {
						strAuthorized.Format("%li", VarBool(pBenefitDetail->varAuthorized) ? 1 : 0);
					}

					CString strInNetwork = "NULL";
					if(pBenefitDetail->varInNetwork.vt == VT_BOOL) {
						strInNetwork.Format("%li", VarBool(pBenefitDetail->varInNetwork) ? 1 : 0);
					}

					pBenefitDetail->strExtraMessage.TrimRight("\r\n");

					// (j.jones 2010-03-26 13:34) - PLID 37905 - changed BenefitType, CoverageLevel, and BenefitType, CoverageLevel, ServiceType
					//to be IDs from EligibilityDataReferenceT, instead of full text
					CString strBenefitTypeRefID = "NULL";
					if(pBenefitDetail->nBenefitTypeRefID != -1) {
						strBenefitTypeRefID.Format("%li", pBenefitDetail->nBenefitTypeRefID);
					}

					CString strCoverageLevelRefID = "NULL";
					if(pBenefitDetail->nCoverageLevelRefID != -1) {
						strCoverageLevelRefID.Format("%li", pBenefitDetail->nCoverageLevelRefID);
					}

					CString strServiceTypeRefID = "NULL";
					if(pBenefitDetail->nServiceTypeRefID != -1) {
						strServiceTypeRefID.Format("%li", pBenefitDetail->nServiceTypeRefID);
					}

					// (c.haag 2010-10-20 13:14) - PLID 41017 - Added ServiceCodeRangeEnd for ANSI 5010.
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EligibilityResponseDetailsT ("
						"ResponseID, BenefitTypeRefID, CoverageLevelRefID, ServiceTypeRefID, InsuranceType, CoverageDesc, TimePeriod, "
						"Amount, Percentage, QuantityType, Quantity, Authorized, InNetwork, ServiceCode, Modifiers, "
						"ExtraMessage, ServiceCodeRangeEnd) "
						"VALUES (@nNewResponseID, %s, %s, %s, '%s', '%s', '%s', "
						"%s, %s, '%s', %s, %s, %s, '%s', '%s', "
						"'%s', '%s')",
						strBenefitTypeRefID, strCoverageLevelRefID, strServiceTypeRefID,
						_Q(pBenefitDetail->strInsuranceType), _Q(pBenefitDetail->strCoverageDesc), _Q(pBenefitDetail->strTimePeriod),
						strAmount, strPercentage, _Q(pBenefitDetail->strQuantityType), strQuantity,
						strAuthorized, strInNetwork, _Q(pBenefitDetail->strServiceCode), _Q(pBenefitDetail->strModifiers),
						_Q(pBenefitDetail->strExtraMessage), _Q(pBenefitDetail->strServiceCodeRangeEnd)
						);

					// (j.jones 2016-05-16 10:32) - NX-100357 - track other payers, if provided
					if (!pBenefitDetail->strInsuranceCompanyName.IsEmpty()) {
						//protect against truncation errors
						if (pBenefitDetail->strInsuranceCompanyName.GetLength() > 255) {
							pBenefitDetail->strInsuranceCompanyName = pBenefitDetail->strInsuranceCompanyName.Left(255);
						}
						if (pBenefitDetail->strInsuranceCompanyAddress1.GetLength() > 255) {
							pBenefitDetail->strInsuranceCompanyAddress1 = pBenefitDetail->strInsuranceCompanyAddress1.Left(255);
						}
						if (pBenefitDetail->strInsuranceCompanyAddress2.GetLength() > 255) {
							pBenefitDetail->strInsuranceCompanyAddress2 = pBenefitDetail->strInsuranceCompanyAddress2.Left(255);
						}
						if (pBenefitDetail->strInsuranceCompanyCity.GetLength() > 255) {
							pBenefitDetail->strInsuranceCompanyCity = pBenefitDetail->strInsuranceCompanyCity.Left(255);
						}
						if (pBenefitDetail->strInsuranceCompanyState.GetLength() > 30) {
							pBenefitDetail->strInsuranceCompanyState = pBenefitDetail->strInsuranceCompanyState.Left(30);
						}
						if (pBenefitDetail->strInsuranceCompanyZip.GetLength() > 30) {
							pBenefitDetail->strInsuranceCompanyZip = pBenefitDetail->strInsuranceCompanyZip.Left(30);
						}
						if (pBenefitDetail->strInsuranceCompanyContactName.GetLength() > 255) {
							pBenefitDetail->strInsuranceCompanyContactName = pBenefitDetail->strInsuranceCompanyContactName.Left(255);
						}
						if (pBenefitDetail->strInsuranceCompanyPhone.GetLength() > 30) {
							pBenefitDetail->strInsuranceCompanyPhone = pBenefitDetail->strInsuranceCompanyPhone.Left(30);
						}
						if (pBenefitDetail->strInsuranceCompanyPayerID.GetLength() > 50) {
							pBenefitDetail->strInsuranceCompanyPayerID = pBenefitDetail->strInsuranceCompanyPayerID.Left(50);
						}
						if (pBenefitDetail->strInsuredPartyPolicyNumber.GetLength() > 50) {
							pBenefitDetail->strInsuredPartyPolicyNumber = pBenefitDetail->strInsuredPartyPolicyNumber.Left(50);
						}
						if (pBenefitDetail->strInsuredPartyGroupNumber.GetLength() > 50) {
							pBenefitDetail->strInsuredPartyGroupNumber = pBenefitDetail->strInsuredPartyGroupNumber.Left(50);
						}

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EligibilityResponseOtherPayorsT ("
							"ResponseID, InsuranceCompanyName, Address1, Address2, City, State, Zip, "
							"ContactName, ContactPhone, PayerID, PolicyNumber, GroupNumber) "
							"VALUES (@nNewResponseID, '%s', '%s', '%s', '%s', '%s', '%s', "
							"'%s', '%s', '%s', '%s', '%s')",
							_Q(pBenefitDetail->strInsuranceCompanyName), _Q(pBenefitDetail->strInsuranceCompanyAddress1), _Q(pBenefitDetail->strInsuranceCompanyAddress2),
							_Q(pBenefitDetail->strInsuranceCompanyCity), _Q(pBenefitDetail->strInsuranceCompanyState), _Q(pBenefitDetail->strInsuranceCompanyZip),
							_Q(pBenefitDetail->strInsuranceCompanyContactName), _Q(pBenefitDetail->strInsuranceCompanyPhone),
							_Q(pBenefitDetail->strInsuranceCompanyPayerID), _Q(pBenefitDetail->strInsuredPartyPolicyNumber), _Q(pBenefitDetail->strInsuredPartyGroupNumber)
						);
					}
				}

				// (j.jones 2009-12-17 15:28) - PLID 36641 - track the status in the MailSent note
				CString strStatus = "Request Confirmed";
				if(pDetail->ercsStatus == ercsDenied) {
					strStatus = "Request Denied";
				}
				else if(pDetail->ercsStatus == ercsInvalid) {
					strStatus = "Invalid Request";
				}

				CString strNote;
				strNote.Format("Eligibility Response received: Status - %s (Insurance Company: %s)", strStatus, strInsuranceCoName);

				//and the MailSent statements					
				// (j.jones 2008-09-04 15:15) - PLID 30288 - supported MailSentNotesT
				// (c.haag 2010-01-27 12:04) - PLID 36271 - Use GetDate() for the service date, not COleDateTime::GetCurrentTime()
				// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
				AddStatementToSqlBatch(strSqlBatch, 
					"INSERT INTO MailSent (MailBatchID, PersonID, Selection, Subject, PathName, Sender, [Date], Location, ServiceDate, InternalRefID, InternalTblName) "
					"VALUES (@nNewMailBatchID, %li, '%s', '', '%s', '%s', GetDate(), %li, GetDate(), @nNewResponseID, 'EligibilityResponsesT')",
					nPatientID, _Q(SELECTION_FILE), _Q(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE), _Q(GetCurrentUserName()), nLocationID);
				AddStatementToSqlBatch(strSqlBatch, "SET @nNewMailSentID = SCOPE_IDENTITY()");
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MailSentNotesT (MailID, Note) VALUES (@nNewMailSentID, '%s')", _Q(strNote));
				// (j.jones 2014-08-04 13:30) - PLID 63141 - we now track the patient ID too
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewResponseIDs (ResponseID, MailSentID, PatientID) VALUES (@nNewResponseID, @nNewMailSentID, %li)", nPatientID);
			}
		}

		if(!strSqlBatch.IsEmpty()) {

			CWaitCursor pWait;
			
			CString strFinalRecordset;
			strFinalRecordset.Format(
					"SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"%s "
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT ResponseID, MailSentID, PatientID FROM @NewResponseIDs",
					strSqlBatch);

			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushPerformanceWarningLimit ppw(-1);
			_RecordsetPtr prsResults = CreateRecordsetStd(strFinalRecordset,
				adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);
			while(!prsResults->eof) {
				//get the response ID
				long nResponseID = AdoFldLong(prsResults, "ResponseID");
				aryResponseIDsReturned.push_back(nResponseID);

				//refresh the mailsent tablechecker				
				long nMailSentID = AdoFldLong(prsResults, "MailSentID");
				long nPatientID = AdoFldLong(prsResults, "PatientID");
				// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
				CClient::RefreshMailSentTable(nPatientID, nMailSentID);
				prsResults->MoveNext();
			}
			prsResults->Close();
		}

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}