// ANSI835Parser.cpp: implementation of the CANSI835Parser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ANSI835Parser.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "FileUtils.h"

using namespace ADODB;

// (j.jones 2012-05-25 13:21) - PLID 44367 - All calls to AfxMessageBox were changed to MessageBox
// to make the modeless dialog behave better.

// (j.jones 2015-07-06 10:42) - PLID 66359 - remark & reason codes
// are now initialized only once per session, since they only change
// when the software is upgraded
void InitializeAdjustmentCodes()
{
	if (g_bAdjustmentCodesInitialized) {
		return;
	}

	g_mapReasonCodes.clear();
	g_mapRemarkCodes.clear();

	_RecordsetPtr rs = CreateRecordset("SELECT Type, Code, Description "
		"FROM AdjustmentCodesT "
		"WHERE Type IN (2,3) "
		"ORDER BY Type, Code");
	while (!rs->eof) {
		long nType = VarLong(rs->Fields->Item["Type"]->Value);
		CString strCode = VarString(rs->Fields->Item["Code"]->Value);
		CString strDescription = VarString(rs->Fields->Item["Description"]->Value);
		if (nType == 2) {
			//reason code
			g_mapReasonCodes[strCode] = strDescription;
		}
		else if (nType == 3) {
			//remark code
			g_mapRemarkCodes[strCode] = strDescription;
		}
		rs->MoveNext();
	}
	rs->Close();

	g_bAdjustmentCodesInitialized = true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CANSI835Parser::CANSI835Parser()
{
	m_pParentWnd = NULL;

	m_strLastOutput = "";
	m_strLoopNumber = "";

	m_CountOfEOBs = 0;

	m_ptrProgressBar = NULL;
	m_ptrProgressStatus = NULL;

	m_chSegmentTerminator = '~';
	m_chElementTerminator = '*';
	m_chCompositeSeparator = ':';

	m_bSkipCurrentEOB = FALSE;
	m_bSkipCurrentClaim = FALSE;

	m_bEnablePeekAndPump = TRUE;

	// (j.jones 2011-03-14 15:48) - PLID 42806 - initialize m_bIs5010File
	m_bIs5010File = FALSE;
}

CANSI835Parser::~CANSI835Parser()
{
	// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
	ClearAllEOBs();
}

// (j.jones 2010-02-09 12:03) - PLID 37254 - added ClearClaimInfo
void CANSI835Parser::ClearClaimInfo(EOBInfo *ptrEOBInfo, EOBClaimInfo *ptrClaimInfoToClear)
{
	try {

		if (ptrEOBInfo) {
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				for (int i = ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1; i >= 0; i--) {
					EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[i];
					if (pClaimInfo == ptrClaimInfoToClear) {
						if (pClaimInfo->arypEOBLineItemInfo.GetSize() > 0) {
							for (int q = pClaimInfo->arypEOBLineItemInfo.GetSize() - 1; q >= 0; q--) {
								EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[q];

								if (pLineItemInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
									for (int z = pLineItemInfo->arypEOBAdjustmentInfo.GetSize() - 1; z >= 0; z--) {
										delete pLineItemInfo->arypEOBAdjustmentInfo[z];
										pLineItemInfo->arypEOBAdjustmentInfo.RemoveAt(z);
									}
								}

								delete pLineItemInfo;
								pClaimInfo->arypEOBLineItemInfo.RemoveAt(q);
							}
						}

						if (pClaimInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
							for (int z = pClaimInfo->arypEOBAdjustmentInfo.GetSize() - 1; z >= 0; z--) {
								delete (pClaimInfo->arypEOBAdjustmentInfo[z]);
								pClaimInfo->arypEOBAdjustmentInfo.RemoveAt(z);
							}
						}

						delete pClaimInfo;
						pClaimInfo = NULL;
						ptrClaimInfoToClear = NULL;
						ptrEOBInfo->arypEOBClaimInfo.RemoveAt(i);
						return;
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 09:27) - PLID 37174 - added ClearEOB
void CANSI835Parser::ClearEOB(EOBInfo *ptrEOBInfoToClear) {

	try {

		for (int a = m_arypEOBInfo.GetSize() - 1; a >= 0; a--) {
			EOBInfo *ptrEOBInfo = m_arypEOBInfo[a];
			if (ptrEOBInfo == ptrEOBInfo) {
				if (ptrEOBInfo) {
					if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
						for (int i = ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1; i >= 0; i--) {
							EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[i];
							ClearClaimInfo(ptrEOBInfo, pClaimInfo);
						}
						ptrEOBInfo->arypEOBClaimInfo.RemoveAll();
					}
					delete ptrEOBInfo;
				}

				ptrEOBInfo = NULL;
				ptrEOBInfoToClear = NULL;
				m_arypEOBInfo.RemoveAt(a);
				m_CountOfEOBs--;
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
void CANSI835Parser::ClearAllEOBs() {

	try {

		//clear out the EOB and all its sub-arrays

		for (int a = m_arypEOBInfo.GetSize() - 1; a >= 0; a--) {
			EOBInfo *ptrEOBInfo = m_arypEOBInfo[a];
			ClearEOB(ptrEOBInfo);
		}

		m_arypEOBInfo.RemoveAll();
		m_CountOfEOBs = 0;

		m_strFileName = "";
		m_strOutputFile = "";

		// (j.jones 2011-03-14 15:48) - PLID 42806 - reset m_bIs5010File
		m_bIs5010File = FALSE;

	}NxCatchAll("Error cleaning up EOB.");
}

// (j.jones 2012-05-25 13:53) - PLID 44367 - pass in a parent
BOOL CANSI835Parser::ParseFile(CWnd *pParentWnd)
{
	CWaitCursor pWait;

	try {

		// (j.jones 2012-05-25 13:54) - PLID 44367 - track the parent window
		m_pParentWnd = pParentWnd;

		// (j.jones 2008-12-19 09:39) - PLID 32519 - Supported auto-opening a file,
		// which would be passed in as m_strFileName. So if we don't have a file,
		// or it is not valid, then browse.
		if (m_strFileName.IsEmpty() || !DoesExist(m_strFileName)) {

			//browse for a file
			CFileDialog BrowseFiles(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, NULL, m_pParentWnd);
			if (BrowseFiles.DoModal() == IDCANCEL) {
				return FALSE;
			}

			// (j.jones 2006-12-19 15:16) - PLID 23913 - store the file name for later manipulation
			m_strFileName = BrowseFiles.GetPathName();
		}

		m_strOutputFile = GetNxTempPath() ^ "EOB.txt";

		BOOL bIsValidFile = FALSE;

		//open the file for reading
		if (!m_InputFile.Open(m_strFileName, CFile::modeRead | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The input file could not be found or opened.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}

		if (!m_OutputFile.Open(m_strOutputFile, CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The output file could not be created.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}

		CWaitCursor pWait;

		Log("Importing remittance file: " + m_strFileName);

		// (j.jones 2009-10-01 14:40) - PLID 35711 - we need to cache possible prepended patient ID codes
		// (j.jones 2010-02-09 09:20) - PLID 37174 - cache the contents of ERemitEOBFilteredIDsT
		// (j.jones 2010-02-09 10:44) - PLID 37254 - cache the contents of ERemitClaimFilteredIDsT
		// (j.armen 2012-02-20 09:25) - PLID 34344 - batch these queries
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT PrependPatientIDCode FROM EbillingFormatsT WHERE PrependPatientIDCode <> '' AND PrependPatientID = 1 GROUP BY PrependPatientIDCode;\r\n"
			"SELECT FilteredID FROM ERemitEOBFilteredIDsT;\r\n"
			"SELECT FilteredID FROM ERemitClaimFilteredIDsT;\r\n");

		m_arystrPrependedPatientIDCodes.RemoveAll();
		while (!prs->eof) {
			m_arystrPrependedPatientIDCodes.Add(AdoFldString(prs, "PrependPatientIDCode", ""));
			prs->MoveNext();
		}
		prs = prs->NextRecordset(NULL);

		m_arystrEOBFilteredIDs.RemoveAll();
		while (!prs->eof) {
			m_arystrEOBFilteredIDs.Add(AdoFldString(prs, "FilteredID", ""));
			prs->MoveNext();
		}
		prs = prs->NextRecordset(NULL);

		m_arystrClaimFilteredIDs.RemoveAll();
		while (!prs->eof) {
			m_arystrClaimFilteredIDs.Add(AdoFldString(prs, "FilteredID", ""));
			prs->MoveNext();
		}

		// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking
		m_bSkipCurrentEOB = FALSE;
		// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
		m_bSkipCurrentClaim = FALSE;
		// (j.jones 2010-02-09 10:45) - PLID 37254 - reset skipped claim tracking
		m_bSkipCurrentClaim = FALSE;

		const int LEN_16_KB = 16384;
		CString strIn;	//input string
		long iFileSize = m_InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
		strIn.ReleaseBuffer(iFileSize);

		CString strLastSegment = "";	//stores data between reads

		if (iFileSize > 0) {
			// (a.walling 2007-08-29 09:54) - PLID 27226 - This should only be done once per file
			// rather than every segment.

			// (a.walling 2008-01-28 16:05) - PLID 28722 - Need to filter any linebreaks here,
			// so they won't be mistakenly used as a terminator.
			strIn.Replace("\r", "");
			strIn.Replace("\n", "");

			// (j.jones 2006-07-25 16:55) - PLID 21606 - Supporting non-standard terminators
			// by determing the character before the GS segment.
			int nIndex = strIn.Find("GS*");
			if (nIndex != -1) {
				CString strSegmentTerminator = strIn.Mid(nIndex - 1, 1);
				if (strSegmentTerminator.GetLength() > 0)
					m_chSegmentTerminator = strSegmentTerminator.GetAt(0);
			}
		}

		while (iFileSize > 0) {

			CString strSegment,	//current segment
				strElement;	//current element

			strIn.Replace("\r", "");
			strIn.Replace("\n", "");

			m_ptrProgressBar->SetPos(0);

			m_ptrProgressBar->SetRange(0, strIn.GetLength());

			while ((strIn.GetLength() > 0) && (strIn.Find(m_chSegmentTerminator) != -1)) {

				//parse the first segment
				strSegment = ParseSegment(strIn);

				m_ptrProgressBar->SetPos(m_ptrProgressBar->GetPos() + strSegment.GetLength());

				//parse the first element, the identifier, into strIdent
				CString strIdent = ParseElement(strSegment);

				//now call the right function, and send the remainder of that segment
				if (strIdent == "ISA")
					ANSI_ISA(strSegment);
				else if (strIdent == "TA1")
					ANSI_TA1(strSegment);
				else if (strIdent == "GS")
					ANSI_GS(strSegment);
				else if (strIdent == "ST") {
					bIsValidFile = ANSI_ST(strSegment);

					if (!bIsValidFile) {
						m_InputFile.Close();
						m_OutputFile.Close();
						return FALSE;
					}
				}
				else if (strIdent == "SE")
					ANSI_SE(strSegment);
				else if (strIdent == "GE")
					ANSI_GE(strSegment);
				else if (strIdent == "IEA")
					ANSI_IEA(strSegment);

				//835-specific fields
				else if (strIdent == "BPR")
					ANSI_BPR(strSegment);
				else if (strIdent == "TRN")
					ANSI_TRN(strSegment);
				else if (strIdent == "CUR")
					ANSI_CUR(strSegment);
				else if (strIdent == "REF")
					ANSI_REF(strSegment);
				else if (strIdent == "DTM")
					ANSI_DTM(strSegment);
				else if (strIdent == "N1")
					ANSI_N1(strSegment);
				else if (strIdent == "N3")
					ANSI_N3(strSegment);
				else if (strIdent == "N4")
					ANSI_N4(strSegment);
				else if (strIdent == "PER")
					ANSI_PER(strSegment);
				else if (strIdent == "LX")
					ANSI_LX(strSegment);
				else if (strIdent == "TS3")
					ANSI_TS3(strSegment);
				else if (strIdent == "TS2")
					ANSI_TS2(strSegment);
				else if (strIdent == "CLP")
					ANSI_CLP(strSegment);
				else if (strIdent == "CAS")
					ANSI_CAS(strSegment);
				else if (strIdent == "NM1")
					ANSI_NM1(strSegment);
				else if (strIdent == "MIA")
					ANSI_MIA(strSegment);
				else if (strIdent == "MOA")
					ANSI_MOA(strSegment);
				else if (strIdent == "AMT")
					ANSI_AMT(strSegment);
				else if (strIdent == "QTY")
					ANSI_QTY(strSegment);
				else if (strIdent == "SVC")
					ANSI_SVC(strSegment);
				else if (strIdent == "LQ")
					ANSI_LQ(strSegment);
				else if (strIdent == "PLB")
					ANSI_PLB(strSegment);
				// (c.haag 2010-10-20 16:38) - PLID 40349 - ANSI 5010 segments
				else if (strIdent == "RDM")
					ANSI_RDM(strSegment);
				else {
#ifdef _DEBUG
					//MessageBox(m_pParentWnd->GetSafeHwnd(), strIdent, "Practice", MB_ICONEXCLAMATION|MB_OK);
#endif
				}

				// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
				// function, as we may have disabled this ability
				PeekAndPump_ANSI835Parser();
			}

			strLastSegment = strIn;

			iFileSize = m_InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);

			strIn = strLastSegment += strIn;
		}


		//close the file
		m_InputFile.Close();
		m_OutputFile.Close();

		//don't display junk if it's not a real file
		if (!bIsValidFile) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an ANSI 835 remittance file.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}

		// (j.jones 2012-04-23 12:20) - PLID 49846 - Most remit files with reversals will list the reversal,
		// then the re-posted claim immediately following. If so, we link them together during the parsing.
		// But if they aren't listed this way, we need to link them together now.
		VerifyReversedClaimLinks();

		// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up both the remit
		// file and the EOB.txt to the server, which we would only do if it was a valid remit file
		CopyConvertedEOBToServer();

		//open the finished file in notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.jones 2011-01-11 14:11) - PLID 42074 - replaced ShellExecute with ShellExecuteEx to fix a VS2008 debug crash

		// Create the shell-execute info object
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

		CString strParameter;
		strParameter.Format("'%s'", m_strOutputFile);

		// Init the info object according to the parameters given
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		//sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.hwnd = (HWND)GetDesktopWindow();
		sei.lpFile = "notepad.exe";
		sei.lpParameters = strParameter;
		sei.nShow = SW_SHOW;
		sei.hInstApp = NULL;

		// Run ShellExecute
		ShellExecuteEx(&sei);

		return TRUE;

	}
	catch (CException &e) {
		MessageBox(m_pParentWnd->GetSafeHwnd(), "Error!", "Practice", MB_ICONEXCLAMATION | MB_OK);
		e.Delete();
	}

	return FALSE;
}

void CANSI835Parser::OutputData(CString &OutputString, CString strNewData) {

	//store this data
	m_strLastOutput = strNewData;
	OutputString += strNewData;
}

CString CANSI835Parser::ParseSection(CString &strIn, char chDelimiter) {

	//the same code is used for ParseSegment/Element/Composite, only the character changes

	CString strOut;

	//if we are in the middle of the file
	if (strIn.Find(chDelimiter) != -1) {
		strOut = strIn.Left(strIn.Find(chDelimiter));
		strIn = strIn.Right(strIn.GetLength() - strIn.Find(chDelimiter) - 1);
	}
	//else if we are on the last segment
	else if (strIn.GetLength() > 0) {
		strOut = strIn;
		strIn = "";
	}

	return strOut;
}

CString CANSI835Parser::ParseSegment(CString &strIn) {

	return ParseSection(strIn, m_chSegmentTerminator);
}

CString CANSI835Parser::ParseElement(CString &strIn) {

	return ParseSection(strIn, m_chElementTerminator);
}

CString CANSI835Parser::ParseComposite(CString &strIn) {

	return ParseSection(strIn, m_chCompositeSeparator);
}

BOOL CANSI835Parser::ANSI_ST(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is a new transaction set
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	//010		ST			Transaction Set Header				M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//ST01		143			Transaction Set Identifier Code		M	ID	3/3

	str = ParseElement(strIn);
	//this should always be 835, if not, give a warning
	if (str != "835") {
		if (str == "277")
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an ANSI 835 remittance file, it is an ANSI 277 response file.\n"
			"Please go to the E-Billing Batch tab and click on the \"Format 277 Report\" button to view this file.", "Practice", MB_ICONINFORMATION | MB_OK);
		else if (str == "997")
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an ANSI 835 remittance file, it is an ANSI 997 acknowledgement file.\n"
			"Please go to the E-Billing Batch tab and click on the \"Format 997 Report\" button to view this file.", "Practice", MB_ICONINFORMATION | MB_OK);
		else
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an ANSI 835 remittance file.", "Practice", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	//ST02		329			Transaction Set Control Number		M	AN	4/9

	str = ParseElement(strIn);
	//this is the batch number
	strFmt.Format("Batch Number: %s\r\n\r\n", str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString, OutputString.GetLength());

	return TRUE;
}

void CANSI835Parser::ANSI_SE(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is the end of a transaction set
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	//080		SE			Transaction Set Trailer				M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element


	//SE01		96			Number Of Included Segments			M	N0	1/10

	//SE02		329			Transaction Set Control Number		M	AN	4/9

	//OutputString = "SE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI835Parser::ANSI_ISA(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is a new transaction set
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

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

	CString strDate;
	if (str.GetLength() == 6) {
		//format YYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/20" + str.Left(2);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	else if (str.GetLength() == 8) {
		//format CCYYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/" + str.Left(4);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	strFmt.Format("Processing Date: %s", strDate);
	OutputString = strFmt;

	//ISA10	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//processing time

	CString strTime;
	if (str.GetLength() == 4) {
		//format HHMM into HH:MM
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	else if (str.GetLength() >= 6) {
		//format HHMMSS into HH:MM (also works for HHMMSSDD)
		str = str.Left(4);
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	strFmt.Format(" %s\r\n", strTime);
	OutputData(OutputString, strFmt);

	//ISA11	I10			Interchange Control Standards Ident.	M	ID	1/1

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
	if (str.GetLength() > 0)
		m_chCompositeSeparator = str.GetAt(0);

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString, OutputString.GetLength());
}

void CANSI835Parser::ANSI_IEA(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is the end of a transaction
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

	//IEA02	I12			Interchange Control Number				M	N0	9/9

	//OutputString = "IEA\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI835Parser::ANSI_TA1(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - interchange level, do not skip nor reset the "skipped EOB" flag
	// (j.jones 2010-02-09 14:33) - PLID 37254 - same for claim skipping

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//TA101	I12			Interchange Control Number				M	N0	9/9

	str = ParseElement(strIn);
	strFmt.Format("Interchange Control Number: %s\r\n\r\n", str);
	OutputString = strFmt;

	//TA102	I08			Interchange Date						M	DT	6/6

	str = ParseElement(strIn);
	//claim date

	CString strDate;
	if (str.GetLength() == 6) {
		//format YYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/20" + str.Left(2);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	else if (str.GetLength() == 8) {
		//format CCYYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/" + str.Left(4);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	strFmt.Format("Claim Creation Date: %s", strDate);
	OutputData(OutputString, strFmt);

	//TA103	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//claim time

	CString strTime;
	if (str.GetLength() == 4) {
		//format HHMM into HH:MM
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	else if (str.GetLength() >= 6) {
		//format HHMMSS into HH:MM (also works for HHMMSSDD)
		str = str.Left(4);
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	strFmt.Format(" %s\r\n\r\n", strTime);
	OutputData(OutputString, strFmt);

	//TA104	I17			Interchange Acknowledgement Code		M	ID	1/1

	str = ParseElement(strIn);
	//was the interchange header valid?
	if (str == "A")
		//The Transmitted Interchange Control Structure Header and Trailer Have Been Received and Have No Errors.
		strFmt.Format("Interchange Acknowledgement Code A: - The Transmitted Interchange Control Structure Header and Trailer Have Been Received and Have No Errors.\r\n\r\n");
	else if (str == "E")
		//The Transmitted Interchange Control Structure Header and Trailer Have Been Received and And Are Accepted But Errors Are Noted. This Means The Sender Must Not Resend The Data.
		strFmt.Format("Interchange Acknowledgement Code E: - The Transmitted Interchange Control Structure Header and Trailer Have Been Received and And Are Accepted But Errors Are Noted. This Means The Sender Must Not Resend The Data.\r\n\r\n");
	else if (str == "R")
		//The Transmitted Interchange Control Structure Header and Trailer are Rejected Because of Errors.
		strFmt.Format("Interchange Acknowledgement Code R: - The Transmitted Interchange Control Structure Header and Trailer are Rejected Because of Errors.\r\n\r\n");
	OutputData(OutputString, strFmt);

	//TA105	I18			Interchange Note Code					M	ID	3/3

	str = ParseElement(strIn);
	//interchange error code
	if (str == "000") {
		//No error
		strFmt.Format("Interchange Error Code %s: No Error\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "001") {
		//The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement
		strFmt.Format("Interchange Error Code %s: The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "002") {
		//This Standard as Noted in the Control Standards Identifier is Not Supported
		strFmt.Format("Interchange Error Code %s: This Standard as Noted in the Control Standards Identifier is Not Supported\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "003") {
		//This Version of the Controls is Not Supported
		strFmt.Format("Interchange Error Code %s: This Version of the Controls is Not Supported\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "004") {
		//The Segment Terminator is Invalid
		strFmt.Format("Interchange Error Code %s: The Segment Terminator is Invalid\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "005") {
		//Invalid Interchange ID Qualifier for Sender
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Sender\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "006") {
		//Invalid Interchange Sender ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Sender ID\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "007") {
		//Invalid Interchange ID Qualifier for Receiver
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Receiver\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "008") {
		//Invalid Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Receiver ID\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "009") {
		//Unknown Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Unknown Interchange Receiver ID\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "010") {
		//Invalid Authorization Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Qualifier Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "011") {
		//Invalid Authorization Information Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "012") {
		//Invalid Security Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Security Information Qualifier Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "013") {
		//Invalid Security Information
		strFmt.Format("Interchange Error Code %s: Invalid Security Information\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "014") {
		//Invalid Interchange Date Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Date Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "015") {
		//Invalid Interchange Time Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Time Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "016") {
		//Invalid Interchange Standards Identifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Standards Identifier Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "017") {
		//Invalid Interchange Version ID Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Version ID Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "018") {
		//Invalid Interchange Control Number Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control Number Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "019") {
		//Invalid Acknowledgement Requested Value
		strFmt.Format("Interchange Error Code %s: Invalid Acknowledgement Requested Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "020") {
		//Invalid Test Indicator Value
		strFmt.Format("Interchange Error Code %s: Invalid Test Indicator Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "021") {
		//Invalid Number of Included Groups Value
		strFmt.Format("Interchange Error Code %s: Invalid Number of Included Groups Value\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "022") {
		//Invalid Control Structure
		strFmt.Format("Interchange Error Code %s: Invalid Control Structure\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "023") {
		//Improper (Premature) End-Of-File (Transmission)
		strFmt.Format("Interchange Error Code %s: Improper (Premature) End-Of-File (Transmission)\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "024") {
		//Invalid Interchange Control (e.g., Invalid GS Segment)
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control (e.g., Invalid GS Segment)\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "025") {
		//Duplicate Interchange Control Number
		strFmt.Format("Interchange Error Code %s: Duplicate Interchange Control Number\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "026") {
		//Invalid Data Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Data Element Separator\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "027") {
		//Invalid Component Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Component Element Separator\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "028") {
		//Invalid Delivery Date in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Date in Deferred Delivery Request\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "029") {
		//Invalid Delivery Time in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time in Deferred Delivery Request\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "030") {
		//Invalid Delivery Time Code in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time Code in Deferred Delivery Request\r\n", str);
		OutputData(OutputString, strFmt);
	}
	else if (str == "031") {
		//Invalid Grade of Service Code
		strFmt.Format("Interchange Error Code %s: Invalid Grade of Service Code\r\n", str);
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "\r\n");
	m_OutputFile.Write(OutputString, OutputString.GetLength());
}

void CANSI835Parser::ANSI_GS(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is a new EOB "file"
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GS01	479			Functional Identifier Code				M	ID	2/2
	str = ParseElement(strIn);

	//GS02	142			Application Sender's Code				M	AN	2/15
	str = ParseElement(strIn);

	//GS03	124			Application Receiver's Code				M	AN	2/15
	str = ParseElement(strIn);

	//GS04	373			Date									M	DT	8/8
	str = ParseElement(strIn);

	//GS05	337			Time									M	TM	4/8
	str = ParseElement(strIn);

	//GS06	28			Group Control Number					M	N0	1/9
	str = ParseElement(strIn);

	//GS07	455			Responsible Agency Code					M	ID	1/2
	str = ParseElement(strIn);

	//GS08	480			Version/Release/Industry Ident. Code	M	AN	1/12
	str = ParseElement(strIn);

	// (c.haag 2010-10-21 15:30) - PLID 40349 - ANSI 5010 version comparisons
	//GS08 4010 Version: 004010X091
	//GS08 5010 Version: 005010X221 OR 005010X221A1

	// (j.jones 2011-03-14 15:46) - PLID 42806 - track if this is 5010
	if (str.Find("5010") != -1) {
		m_bIs5010File = TRUE;
	}

	//OutputString = "GS\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI835Parser::ANSI_GE(CString &strIn) {

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is the end of a EOB "file"
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GE01	97			Number of Transaction Sets Included		M	N0	1/6

	//GE02	28			Group Control Number					M	N0	1/9

	//OutputString = "GE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

///////////////////////////////////////
//Begin the 835-specific fields      //
///////////////////////////////////////

void CANSI835Parser::ANSI_BPR(CString &strIn) {

	//do NOT skip this segment, it is a new EOB

	//44		BPR			Finanical Information				R		1

	CString OutputString, str, strFmt;

	if (strIn != "") {
		OutputData(OutputString, "****************************************************************************\r\n");
		OutputData(OutputString, "****************************************************************************\r\n\r\n");
	}

	// (j.jones 2010-02-09 09:27) - PLID 37174 - reset EOB tracking, this is a new EOB
	m_bSkipCurrentEOB = FALSE;

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking
	m_bSkipCurrentClaim = FALSE;

	EOBInfo *ptrEOBInfo = new (EOBInfo);
	ptrEOBInfo->nIndex = m_CountOfEOBs;
	ptrEOBInfo->nLikelyInsuranceCoID = -1;
	ptrEOBInfo->nHCFAGroupID = -1;
	ptrEOBInfo->cyTotalPaymentAmt = COleCurrency(0, 0);
	// (j.jones 2010-04-09 11:52) - PLID 31309 - default the dtCheckDate to today's date,
	// just incase it is not provided in the file
	ptrEOBInfo->dtCheckDate = COleDateTime::GetCurrentTime();

	m_arypEOBInfo.Add(ptrEOBInfo);

	m_CountOfEOBs++;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//BPR01		305			Transaction Handling Code			M	ID	1/2
	//C - Payment Accompanies Remittance Advice
	//D - Make Payment Only
	//H - Notification Only
	//I - Remittance Information Only
	//P - Prenotification of Future Transfers
	//U - Split Payment and Remittance
	//X - Handling Party’s Option to Split Payment and Remittance
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "Remittance Transaction Type: ";
		if (str == "C")
			strFmt += "Payment Accompanies Remittance Advice.";
		else if (str == "D")
			strFmt += "Make Payment Only";
		else if (str == "H")
			strFmt += "Notification Only";
		else if (str == "I")
			strFmt += "Remittance Information Only";
		else if (str == "P")
			strFmt += "Prenotification of Future Transfers";
		else if (str == "U")
			strFmt += "Split Payment and Remittance";
		else if (str == "X")
			strFmt += "Handling Party's Option to Split Payment and Remittance";
		else
			strFmt += "Unknown";

		OutputData(OutputString, strFmt);
		OutputData(OutputString, "\r\n\r\n");
	}

	OutputData(OutputString, "****************************************************************************\r\n\r\n");

	//BPR02		762			Monetary Amount						M	R	1/18
	//Total Actual Provider Payment Amount
	str = ParseElement(strIn);
	COleCurrency cy(0, 0);
	if (str != "") {
		cy.ParseCurrency(str);
	}
	strFmt.Format("Total Payment Amount for this EOB: %s\r\n\r\n", _Q(FormatCurrencyForInterface(cy, TRUE, TRUE)));
	OutputData(OutputString, strFmt);

	ptrEOBInfo->cyTotalPaymentAmt += cy;

	//BPR03		478			Credit/Debit Flag Code				M	ID	1/1
	//C - Credit
	//D - Debit (in theory it will never be debit)
	str = ParseElement(strIn);

	//BPR04		591			Payment Method Code					M	ID	3/3
	//ACH - Automated Clearing House (ACH)
	//BOP - Financial Institution Option
	//CHK - Check
	//FWT - Federal Reserve Funds/Wire Transfer - Nonrepetitive
	//NON - Non-Payment Data
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "Payment Method: ";
		if (str == "ACH")
			strFmt += "Automated Clearing House (ACH)";
		else if (str == "BOP")
			strFmt += "Financial Institution Option";
		else if (str == "CHK")
			strFmt += "Check";
		else if (str == "FWT")
			strFmt += "Wire Transfer";
		else if (str == "NON")
			strFmt += "No Payment (Information Only)";
		else
			strFmt += "Unknown";

		OutputData(OutputString, strFmt);
		OutputData(OutputString, "\r\n\r\n");
	}

	//BPR05		812			Payment Format Code					O	ID	1/10
	//CCP Cash Concentration/Disbursement plus Addenda (CCD+) (ACH)
	//CTX Corporate Trade Exchange (CTX) (ACH)
	str = ParseElement(strIn);

	//BPR06		506			(DFI) ID Number Qualifier			X	ID	2/2
	//Depository Financial Institution (DFI) Identification Number Qualifier
	//01 - ABA Transit Routing Number Including Check Digits (9 digits)
	//ABA is a unique number identifying every bank in the United States.
	//04 - Canadian Bank Branch and Institution Number
	str = ParseElement(strIn);

	//BPR07		507			(DFI) Identification Number			X	AN	3/12
	//Sender DFI Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Sender Bank Number: %s", str);
		OutputData(OutputString, strFmt);
		ptrEOBInfo->strPayBankRoutingNo = str;
	}

	//BPR08		569			Account Number Qualifier			O	ID	1/3
	//DA - Demand Deposit
	str = ParseElement(strIn);

	//BPR09		508			Account Number						X	AN	1/35
	//Sender Bank Account Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("\t\tAccount Number: %s", str);
		OutputData(OutputString, strFmt);
		ptrEOBInfo->strPayAccount = str;
	}

	//BPR10		509			Originating Company Identifier		O	AN	10/10
	//Payer Identifier
	//"must be the Federal Tax ID number, preceeded by a '1'"
	str = ParseElement(strIn);

	//BPR11		510			Originating Company Supplmntl. Code	O	AN	9/9
	str = ParseElement(strIn);

	//BPR12		506			(DFI) ID Number Qualifier			X	ID	2/2
	//Depository Financial Institution (DFI) Identification Number Qualifier
	//01 - ABA Transit Routing Number Including Check Digits (9 digits)
	//ABA is a unique number identifying every bank in the United States.
	//04 - Canadian Bank Branch and Institution Number
	str = ParseElement(strIn);

	//BPR13		507			(DFI) Identification Number			X	AN	3/12
	//Receiver or Provider Bank ID Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("\r\nReceiver Bank Number: %s", str);
		OutputData(OutputString, strFmt);
	}

	//BPR14		569			Account Number Qualifier			O	ID	1/3
	//DA - Demand Deposit
	//SG - Savings
	str = ParseElement(strIn);

	//BPR15		508			Account Number						X	AN	1/35
	//Receiver or Provider Account Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("\t\tAccount Number: %s", str);
		OutputData(OutputString, strFmt);
	}

	//BPR16		373			Date								O	DT	8/8
	//Check Issue or EFT Effective Date (CCYYMMDD)
	str = ParseElement(strIn);
	if (str != "") {
		CString strDate;
		strDate.Format("%s/%s/%s", str.Mid(4, 2), str.Right(2), str.Left(4));
		strFmt.Format("\r\n\r\nCheck Issue / EFT Effective Date: %s\r\n", strDate);
		OutputData(OutputString, strFmt);
		ptrEOBInfo->dtCheckDate.ParseDateTime(strDate);
	}

	//BPR17		1048		Business Function Code				O	ID	1/3
	//NOT USED
	str = ParseElement(strIn);

	//BPR18		506			(DFI) ID Number Qualifier			X	ID	2/2
	//NOT USED
	str = ParseElement(strIn);

	//BPR19		507			(DFI) Identification Number			X	AN	3/12
	//NOT USED
	str = ParseElement(strIn);

	//BPR20		569			Account Number Qualifier			O	ID	1/3
	//NOT USED
	str = ParseElement(strIn);

	//BPR21		508			Account Number						X	AN	1/35
	//NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_TRN(CString &strIn) {

	// (j.jones 2010-02-09 14:33) - PLID 37254 - don't check the skipped claim here,
	// this is part of the EOB-level

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//52		TMP			Reassociation Trace Number			R		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//TRN01		481			Trace Type Code						M	ID	1/2
	//1 - Current Transaction Trace Numbers
	str = ParseElement(strIn);

	//TRN02		127			Reference Identification			M	AN	1/30
	//Check or EFT Trace Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Check / EFT Trace Number: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
		ptrEOBInfo->strCheckNumber = str;
	}

	//TRN03		509			Originating Company Identifier		O	AN	10/10
	//Payer Identifier
	str = ParseElement(strIn);

	//TRN04		127			Reference Identification			O	AN	1/30
	//Originating Company Supplemental Code
	str = ParseElement(strIn);

	OutputData(OutputString, "****************************************************************************\r\n\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_CUR(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - don't check the skipped claim here,
	// this is part of the EOB-level

	//54		CUR			Foreign Currency Information		S		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//CUR01		98			Entity Identifier Code				M	ID	2/3
	//PR - Payer
	str = ParseElement(strIn);

	//CUR02		100			Currency Code						M	ID	3/3
	str = ParseElement(strIn);

	//CUR03		280			Exchange Rate						O	R	4/10
	str = ParseElement(strIn);

	//CUR04		98			Entity Identifier Code				O	ID	2/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR05		100			Currency Code						O	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR06		669			Currency Market/Exchange Code		O	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR07		374			Date/Time Qualifier					X	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR08		373			Date								O	DT	8/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR09		337			Time								O	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR10		374			Date/Time Qualifier					X	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR11		373			Date								X	DT	8/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR12		337			Time								X	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR13		374			Date/Time Qualifier					X	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR14		373			Date								X	DT	8/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR15		337			Time								X	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR16		374			Date/Time Qualifier					X	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR17		373			Date								X	DT	8/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR18		337			Time								X	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR19		374			Date/Time Qualifier					X	ID	3/3
	//NOT USED
	str = ParseElement(strIn);

	//CUR20		373			Date								X	DT	8/8
	//NOT USED
	str = ParseElement(strIn);

	//CUR21		337			Time								X	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//m_OutputFile.Write(OutputString,OutputString.GetLength());

}

void CANSI835Parser::ANSI_REF(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//Reference Identification, used in:

	//57		REF			Receiver Identification				S		1
	//58		REF			Version Identification				S		1
	//67		REF			Additional Payer Identification		S		4
	//77		REF			Payee Additional Identification		S		>1
	//126		REF			Other Claim Related Identification	S		5
	//128		REF			Rendering Provider Identification	S		10
	//154		REF			Service Identification				S		7
	//156		REF			Rendering Provider Information		S		10
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 
	//			REF			Line Item Control Number
	//			REF			HealthCare Policy Identification
	CString OutputString, str, strFmt;

	CString strQual, strIdent;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//REF01		128			Reference Identification Qualifier	M	ID	2/3
	//in the opening segments:
	//EV - Receiver Identification Number
	//F2 - Version Code - Local
	//in Loop 1000A:
	//2U - Payer Identification Number
	//EO - Submitter Identification Number
	//HI - Health Industry Number (HIN)
	//NF National Association of Insurance Commissioners (NAIC) Code
	//in Loop 1000B:
	//0B - State License Number
	//1A - Blue Cross Provider Number
	//1B - Blue Shield Provider Number
	//1C - Medicare Provider Number
	//1D - Medicaid Provider Number
	//1E - Dentist License Number
	//1F - Anesthesia License Number
	//1G - Provider UPIN Number
	//1H - CHAMPUS Identification Number
	//D3 - National Association of Boards of Pharmacy Number
	//G2 - Provider Commercial Number
	//N5 - Provider Plan Network Identification Number
	//PQ - Payee Identification
	//TJ - Federal Taxpayer’s Identification Number
	//in Loop 2100 (Other Claim Related Identification):
	//1L - Group or Policy Number
	//1W - Member Identification Number
	//9A - Repriced Claim Reference Number
	//9C - Adjusted Repriced Claim Reference Number
	//A6 - Employee Identification Number
	//BB - Authorization Number
	//CE - Class of Contract Code
	//EA - Medical Record Identification Number
	//F8 - Original Reference Number
	//G1 - Prior Authorization Number
	//G3 - Predetermination of Benefits Identification Number
	//IG - Insurance Policy Number
	//SY - Social Security Number
	//in Loop 2100 (Rendering Provider Identification):
	//1A - Blue Cross Provider Number
	//1B - Blue Shield Provider Number
	//1C - Medicare Provider Number
	//1D - Medicaid Provider Number
	//1G - Provider UPIN Number
	//1H - CHAMPUS Identification Number
	//D3 - National Association of Boards of Pharmacy Number
	//G2 - Provider Commercial Number
	//in Loop 2110 (Service Identification):
	//1S - Ambulatory Patient Group (APG) Number
	//6R - Provider Control Number (This is the Line Item Control Number submitted in the 837)
	//BB - Authorization Number
	//E9 - Attachment Code
	//G1 - Prior Authorization Number
	//G3 - Predetermination of Benefits Identification Number
	//LU - Location Number
	//RB - Rate code number
	//in Loop 2110 (Rendering Provider Information):
	//1A - Blue Cross Provider Number
	//1B - Blue Shield Provider Number
	//1C - Medicare Provider Number
	//1D - Medicaid Provider Number
	//1G - Provider UPIN Number
	//1H - CHAMPUS Identification Number
	//1J - Facility ID Number
	//HPI - Health Care Financing Administration National Provider Identifier
	//SY - Social Security Number
	//TJ - Federal Taxpayer’s Identification Number
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 Codes in loop 2110 (Line Item Control Number)
	//6R - Provider control number
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 Codes in loop 2110 (HealthCare Policy Identification)
	//0K - Policy Form Identifying Number

	strQual = ParseElement(strIn);

	//REF02		127			Reference Identification			X	AN	1/30
	strIdent = ParseElement(strIn);
	if (strIdent != "") {
		strFmt = "";
		if (strQual == "EV") {
			strFmt.Format("Receiver ID: %s\r\n\r\n", strIdent);
		}
		else if (strQual == "2U" || strQual == "EO" || strQual == "HI" || strQual == "NF") {
			strFmt.Format("Payer ID: %s\r\n\r\n", strIdent);
		}
		else if (strQual == "0B" || strQual == "1A" || strQual == "1B" || strQual == "1C" ||
			strQual == "1D" || strQual == "1E" || strQual == "1F" || strQual == "1G" ||
			strQual == "1H" || strQual == "D3" || strQual == "G2" || strQual == "N5" ||
			strQual == "PQ" || strQual == "TJ" || strQual == "HPI" || strQual == "1J") {
			strFmt.Format("\r\nPayee/Provider ID: %s\r\n\r\n", strIdent);
		}
		else if (strQual == "1L") {
			strFmt.Format("Group or Policy Number: %s\r\n\r\n", strIdent);

			EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				pInfo->strGroupOrPolicyNum = strIdent;
			}
		}
		else if (strQual == "1W")
			strFmt.Format("Member Identification Number: %s\r\n\r\n", strIdent);
		else if (strQual == "9A")
			strFmt.Format("Repriced Claim Reference Number: %s\r\n\r\n", strIdent);
		else if (strQual == "9C")
			strFmt.Format("Adjusted Repriced Claim Reference Number: %s\r\n\r\n", strIdent);
		else if (strQual == "A6")
			strFmt.Format("Employee Identification Number: %s\r\n\r\n", strIdent);
		else if (strQual == "BB")
			strFmt.Format("Authorization Number: %s\r\n\r\n", strIdent);
		else if (strQual == "CE")
			strFmt.Format("Class of Contract Code: %s\r\n\r\n", strIdent);
		else if (strQual == "EA")
			strFmt.Format("Medical Record Identification Number: %s\r\n\r\n", strIdent);
		else if (strQual == "F8")
			strFmt.Format("Original Reference Number: %s\r\n\r\n", strIdent);
		else if (strQual == "G1")
			strFmt.Format("Prior Authorization Number: %s\r\n\r\n", strIdent);
		else if (strQual == "G3")
			strFmt.Format("Predetermination of Benefits Identification Number: %s\r\n\r\n", strIdent);
		else if (strQual == "IG")
			strFmt.Format("Insurance Policy Number: %s\r\n\r\n", strIdent);
		else if (strQual == "SY")
			strFmt.Format("Social Security Number: %s\r\n\r\n", strIdent);
		else if (strQual == "1S")
			strFmt.Format("Ambulatory Patient Group (APG) Number: %s\r\n\r\n", strIdent);
		else if (strQual == "E9")
			strFmt.Format("Attachment Code: %s\r\n\r\n", strIdent);
		else if (strQual == "LU")
			strFmt.Format("Location Number: %s\r\n\r\n", strIdent);
		else if (strQual == "RB")
			strFmt.Format("Rate Code Number: %s\r\n\r\n", strIdent);
		// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 codes
		else if (strQual == "6R") {
			strFmt.Format("Provider Control Number: %s\r\n\r\n", strIdent);
			// Fill out strTraceNumber
			EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				if (pInfo->arypEOBLineItemInfo.GetSize() > 0) {
					EOBLineItemInfo *pLineInfo = pInfo->arypEOBLineItemInfo[pInfo->arypEOBLineItemInfo.GetSize() - 1];
					pLineInfo->strTraceNumber = strIdent;
				}
			}
		}
		else if (strQual == "0K")
			strFmt.Format("Policy Form Identifying Number: %s\r\n\r\n", strIdent);
		OutputData(OutputString, strFmt);
	}

	//REF03		352			Description							X	AN	1/80
	//NOT USED
	str = ParseElement(strIn);

	//REF04		C040		Reference Identifier				O
	//NOT USED
	str = ParseElement(strIn);


	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_DTM(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//Date, used in:

	//60		DTM			Production Date						S		1
	//130		DTM			Claim Date							S		4
	//146		DTM			Service Date						S		3
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 
	//			DTM			Coverage Expiration Date
	//			DTM			Claim Received Date

	CString OutputString, str, strFmt;

	CString strQual, strDate;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//DTM01		374			Date/Time Qualifier					M	ID	3/3
	//in the opening segments:
	//405 - Production (Production Date)
	//in Loop 2100 (Claim Date):
	//036 - Expiration (Expiration Date)
	//050 - Received (date the claim was received by the Payer)
	//232 - Claim Statement Period Start
	//233 - Claim Statement Period End
	//in Loop 2110 (Service Date)
	//150 - Service Period Start (only when a multi-day service)
	//151 - Service Period End (only when a multi-day service)
	//472 - Service (if a single day service)
	strQual = ParseElement(strIn);

	//DTM02		373			Date								X	DT	8/8
	//CCYYMMDD
	strDate = ParseElement(strIn);
	if (strDate != "") {
		strFmt = "";

		CString strDateFmt;
		strDateFmt.Format("%s/%s/%s", strDate.Mid(4, 2), strDate.Right(2), strDate.Left(4));

		if (strQual == "405") {
			strFmt.Format("Claim Production Date: %s\r\n\r\n", strDateFmt);
		}
		else if (strQual == "036") {
			strFmt.Format("Coverage Expiration Date: %s\r\n\r\n", strDateFmt);
		}
		else if (strQual == "050") {
			strFmt.Format("Date Claim Received: %s\r\n\r\n", strDateFmt);
		}
		else if (strQual == "232") {
			strFmt.Format("Claim Statement Period Start: %s\r\n\r\n", strDateFmt);

			//set as the current claim date
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				pInfo->dtBillDate.ParseDateTime(strDateFmt);
				pInfo->bBillDatePresent = TRUE;
			}
		}
		else if (strQual == "233") {
			strFmt.Format("Claim Statement Period End: %s\r\n\r\n", strDateFmt);
		}
		else if (strQual == "150") {
			strFmt.Format("Service Period Start: %s\r\n\r\n", strDateFmt);

			//set as the current charge date
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				if (pInfo->arypEOBLineItemInfo.GetSize() > 0) {
					EOBLineItemInfo *pLineInfo = pInfo->arypEOBLineItemInfo[pInfo->arypEOBLineItemInfo.GetSize() - 1];
					pLineInfo->dtChargeDate.ParseDateTime(strDateFmt);
					pLineInfo->bChargeDatePresent = TRUE;
				}
			}
		}
		else if (strQual == "151") {
			strFmt.Format("Service Period End: %s\r\n\r\n", strDateFmt);
		}
		else if (strQual == "472") {
			strFmt.Format("Service Date: %s\r\n\r\n", strDateFmt);

			//set as the current charge date
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				if (pInfo->arypEOBLineItemInfo.GetSize() > 0) {
					EOBLineItemInfo *pLineInfo = pInfo->arypEOBLineItemInfo[pInfo->arypEOBLineItemInfo.GetSize() - 1];
					pLineInfo->dtChargeDate.ParseDateTime(strDateFmt);
					pLineInfo->bChargeDatePresent = TRUE;
				}
			}
		}
		OutputData(OutputString, strFmt);

		if (strQual == "405")
			OutputData(OutputString, "****************************************************************************\r\n\r\n");
	}

	//DTM03		337			Time								X	TM	4/8
	//NOT USED
	str = ParseElement(strIn);

	//DTM04		623			Time Code							O	ID	2/2
	//NOT USED
	str = ParseElement(strIn);

	//DTM05		1250		Date Time Period Format Qualifier	X	ID	2/3
	//NOT USED
	str = ParseElement(strIn);

	//DTM06		1251		Date Time Period					X	AN	1/35
	//NOT USED
	str = ParseElement(strIn);


	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_N1(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - this is the level we set the skipped EOB flag, so do not skip nor reset it

	// (j.jones 2010-02-09 14:33) - PLID 37254 - don't check the skipped claim here,
	// this is part of the EOB-level

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//Name, used in:

	//62		N1			Payer Identification				R		1
	//72		N1			Payee Identification				R		1

	CString OutputString, str, strFmt;

	CString strIdent, strName, strQual, strCode;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//N101		98			Entity Identifier Code				M	ID	2/3
	//PR - Loop 1000A - Payer
	//PE - Loop 1000B - Payee
	strIdent = ParseElement(strIn);

	if (strIdent == "PR")
		m_strLoopNumber = "1000A";
	else if (strIdent == "PE")
		m_strLoopNumber = "1000B";

	//N102		93			Name								X	AN	1/80
	strName = ParseElement(strIn);
	if (strName != "") {
		strFmt = "";
		if (strIdent == "PR") {
			//Payer Name
			strFmt.Format("Payer: \r\n%s ", strName);
			ptrEOBInfo->strInsuranceCoName = strName;
		}
		else if (strIdent == "PE") {
			//Payee Name
			strFmt.Format("Payee: \r\n%s ", strName);
			ptrEOBInfo->strPayeeName = strName;
		}
		OutputData(OutputString, strFmt);
	}

	//N103		66			Identification Code Qualifier		X	ID	1/2
	//if N101 == "PR" then
	//XV - Health Care Financing Administration National PlanID
	//if N101 == "PE" then
	//FI - Federal Taxpayer’s Identification Number
	//XX - Health Care Financing Administration National Provider Identifier
	strQual = ParseElement(strIn);

	//N104		67			Identification Code					X	AN	2/80
	//if N101 == "PR" then Payer Identifier
	//if N101 == "PE" then Payee Identification Code
	strCode = ParseElement(strIn);
	CString strPayeeID;
	if (strCode != "") {
		strFmt = "";
		if (strIdent == "PR") {
			//Payer ID
			strFmt.Format("(ID: %s)", strCode);
			ptrEOBInfo->strPayerID = strCode;
		}
		else if (strIdent == "PE") {
			//Payee ID
			// (j.jones 2010-02-09 09:12) - PLID 37174 - track in its own variable
			strPayeeID = strCode;
			strFmt.Format("(ID: %s)", strPayeeID);
		}
		OutputData(OutputString, strFmt);
	}

	//N105		706			Entity Relationship Code			O	ID	2/2
	//NOT USED
	str = ParseElement(strIn);

	//N106		98			Entity Identifier Code				O	ID	2/3
	//NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString, OutputString.GetLength());

	// (j.jones 2010-02-09 09:12) - PLID 37174 - See if we need to skip this EOB.
	// If no IDs are in ERemitEOBFilteredIDsT, then we import all EOBs. If there are any
	// IDs in ERemitEOBFilteredIDsT, then we only import the EOBs that have a PayeeID
	// tracked in that table (the contents of which are cached in m_arystrEOBFilteredIDs)

	if (!strPayeeID.IsEmpty() && m_arystrEOBFilteredIDs.GetSize() > 0) {
		//we have a Payee ID, and we are filtering on at least one EOB Payee ID
		for (int i = 0; i<m_arystrEOBFilteredIDs.GetSize(); i++) {
			if (strPayeeID.CompareNoCase(m_arystrEOBFilteredIDs.GetAt(i)) == 0) {
				//they match, which means we WILL import this EOB,
				//so just return now
				return;
			}
		}

		//if we got here it means that we are filtering on IDs, but the payee ID
		//for the current EOB is not part of our filter, so we need to say so,
		//and drop the currently tracked EOB
		EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();
		ClearEOB(ptrEOBInfo);

		//we are now going to skip everything in the EOB file until the next EOB begins
		m_bSkipCurrentEOB = TRUE;

		//and output that we skipped this EOB
		OutputString = "";
		OutputData(OutputString, "\r\n\r\n************************************************************************************************\r\n");
		OutputData(OutputString, "**** This EOB has been SKIPPED due to having a Payee ID that is not in your filtered list.  ****\r\n");
		OutputData(OutputString, "**** The list of filtered IDs can be reviewed in the 'Configure EOB Import Filters' screen. ****\r\n");
		OutputData(OutputString, "************************************************************************************************\r\n\r\n\r\n");
		m_OutputFile.Write(OutputString, OutputString.GetLength());
		return;
	}
}

void CANSI835Parser::ANSI_N3(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - don't check the skipped claim here,
	// this is part of the EOB-level

	//Address, used in:

	//64		N3			Payer Address						R		1
	//74		N3			Payee Address						R		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//N301		166		Address Information						M	AN	1/55
	//Address 1
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("\r\n%s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//N302		166		Address Information						O	AN	1/55
	//Address 2
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("\r\n%s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());
}

void CANSI835Parser::ANSI_N4(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - don't check the skipped claim here,
	// this is part of the EOB-level

	//City/State/Zip, used in:

	//65		N4			Payer City, State, Zip Code			R		1
	//75		N4			Payee City, State, Zip Code			S		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//N401		19			City Name							O	AN	2/30
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("%s, ", str);
		OutputData(OutputString, strFmt);
	}

	//N402		156			State or Province Code				O	ID	2/2
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("%s ", str);
		OutputData(OutputString, strFmt);
	}

	//N403		116			Postal Code							O	ID	3/15
	//Zip Code
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("%s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//N404		26			Country Code						O	ID	2/3
	//if Loop 1000A (Payer) then NOT USED
	//if Loop 1000B (Payee) then used if country is other than USA
	str = ParseElement(strIn);

	//N405		309			Location Qualifier					X	ID	1/2
	//NOT USED
	str = ParseElement(strIn);

	//N406		310			Location Identifier					O	AN	1/30
	//NOT USED
	str = ParseElement(strIn);

	if (OutputString != "")
		OutputData(OutputString, "\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_PER(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//Administrative Communications Contact, used in:

	//69		PER			Payer Contact Information					S		1
	//132		PER			Claim Contact Information					S		3
	// (c.haag 2010-10-21 15:56) - PLID 40349 - ANSI 5010 segments
	//			PER			Payer Technical Contact Information	R		>1
	//			PER			Payer Web site								S		1					

	CString OutputString, str, strFmt;

	CString strQual;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//PER01		366			Contact Function Code				M	ID	2/2
	//CX - Payers Claim Office
	// (c.haag 2010-10-21 15:56) - PLID 40349 - ANSI 5010 - More meanings
	//BL - Technical Department
	//IC - Information contact
	str = ParseElement(strIn);

	//PER02		93			Name								O	AN	1/60
	//Contact Name
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Contact Name: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER03		365			Communication Number Qualifier		X	ID	2/2
	//EM - Electronic Mail
	//FX - Facsimile
	//TE - Telephone
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 - More meanings
	//UR - Uniform Resource Locator (URL)
	strQual = ParseElement(strIn);

	//PER04		364			Communication Number				X	AN	1/80
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "";
		if (strQual == "EM")
			strFmt.Format("E-Mail Address: %s\r\n", str);
		else if (strQual == "FX")
			strFmt.Format("Fax: %s\r\n", str);
		else if (strQual == "TE")
			strFmt.Format("Phone: %s\r\n", str);
		else if (strQual == "UR") // (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010
			strFmt.Format("URL: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER05		365			Communication Number Qualifier		X	ID	2/2
	//EM - Electronic Mail
	//EX - Telephone Extension
	//FX - Facsimile
	//TE - Telephone
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 - More meanings
	//UR - Uniform Resource Locator (URL)
	strQual = ParseElement(strIn);

	//PER06		364			Communication Number				X	AN	1/80
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "";
		if (strQual == "EM")
			strFmt.Format("E-Mail Address: %s\r\n", str);
		else if (strQual == "FX")
			strFmt.Format("Fax: %s\r\n", str);
		else if (strQual == "TE")
			strFmt.Format("Phone: %s\r\n", str);
		else if (strQual == "EX")
			strFmt.Format("Extension: %s\r\n", str);
		else if (strQual == "UR") // (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010
			strFmt.Format("URL: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER07		365			Communication Number Qualifier		X	ID	2/2
	//EX - Telephone Extension
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 - More meanings
	//EM - Electronic Mail
	//FX - Faxsimile
	//UR - URL
	strQual = ParseElement(strIn);

	//PER08		364			Communication Number				X	AN	1/80
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "";
		if (strQual == "EX")
			strFmt.Format("Extension: %s\r\n", str);
		// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 - More meanings
		else if (strQual == "EM")
			strFmt.Format("E-Mail Address: %s\r\n", str);
		else if (strQual == "FX")
			strFmt.Format("Fax: %s\r\n", str);
		else if (strQual == "UR")
			strFmt.Format("URL: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PER09		443			Contact Inquiry Reference			O	AN	1/20
	//NOT USED
	str = ParseElement(strIn);

	if (OutputString != "")
		OutputData(OutputString, "\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());
}

void CANSI835Parser::ANSI_LX(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking, this is a header between claims
	m_bSkipCurrentClaim = FALSE;

	//79		LX			Header Number						S		1

	m_strLoopNumber = "2000";

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//LX01		554			Assigned Number						M	N0	1/6
	str = ParseElement(strIn);

	//m_OutputFile.Write(OutputString,OutputString.GetLength());

}

void CANSI835Parser::ANSI_TS3(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking, this is a header between claims
	m_bSkipCurrentClaim = FALSE;

	//80		TS3			Provider Summary Information		S		1

	m_strLoopNumber = "2000";

	CString OutputString, str, strFmt;

	COleCurrency cy;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//TS301		127			Reference Identification			M	AN	1/30
	//Provider Identifier (Provider Number)
	str = ParseElement(strIn);
	if (str != "") {
		if (m_strLastOutput.Right(2) != "\r\n")
			OutputData(OutputString, "\r\n\r\n");
		strFmt.Format("Provider ID: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS302		1331		Facility Code Value					M	AN	1/2
	//Facility Type Code (Place Of Service Designation)
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Facility Code: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS303		373			Date								M	DT	8/8
	//Fiscal Period Date (CCYYMMDD) (the last day of the provider's fiscal year)
	str = ParseElement(strIn);

	//TS304		380			Quantity							M	R	1/15
	//Total Claim Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Claim Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS305		782			Monetary Amount						M	R	1/18
	//Total Claim Charge Amount (total reported charges of all claims)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Claim Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
		//This is only the total for the claim
		/*
		if(ptrEOBInfo->paryEOBClaimInfo.GetSize() > 0) {
		EOBClaimInfo *pInfo = (EOBClaimInfo*)ptrEOBInfo->paryEOBClaimInfo.GetAt(ptrEOBInfo->paryEOBClaimInfo.GetSize() - 1);
		pInfo->cyClaimChargeAmt = cy;
		}
		*/
	}

	//TS306		782			Monetary Amount						O	R	1/18
	//Total Covered Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Covered Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS307		782			Monetary Amount						O	R	1/18
	//Total Non-Covered Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Non-Covered Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS308		782			Monetary Amount						O	R	1/18
	//Total Denied Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Denied Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS309		782			Monetary Amount						O	R	1/18
	//Total Provider Payment Amount (includes TS310 interest amount)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Provider Payment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
		//This is only the total for the claim
		/*
		if(ptrEOBInfo->paryEOBClaimInfo.GetSize() > 0) {
		EOBClaimInfo *pInfo = (EOBClaimInfo*)ptrEOBInfo->paryEOBClaimInfo.GetAt(ptrEOBInfo->paryEOBClaimInfo.GetSize() - 1);
		pInfo->cyClaimPaymentAmt = cy;
		}
		*/
	}

	//TS310		782			Monetary Amount						O	R	1/18
	//Total Interest Amount (total amount of interest paid)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Interest Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS311		782			Monetary Amount						O	R	1/18
	//Total Contractual Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Contractual Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS312		782			Monetary Amount						O	R	1/18
	//Total Gramm-Rudman Reduction Amount (hell if I know what this is)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Gramm-Rudman Reduction Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS313		782			Monetary Amount						O	R	1/18
	//Total MSP Payer Amount (total MSP primary payer amount)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total MSP Payer Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS314		782			Monetary Amount						O	R	1/18
	//Total Blood Deductible Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Blood Deductible Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS315		782			Monetary Amount						O	R	1/18
	//Total Non-Lab Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Non-Lab Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS316		782			Monetary Amount						O	R	1/18
	//Total Coinsurance Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Coinsurance Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS317		782			Monetary Amount						O	R	1/18
	//Total HCPCS Reported Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total HCPCS Reported Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS318		782			Monetary Amount						O	R	1/18
	//Total HCPCS Payable Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total HCPCS Payable Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS319		782			Monetary Amount						O	R	1/18
	//Total Deductible Amount (total cash deductible)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Deductible Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS320		782			Monetary Amount						O	R	1/18
	//Total Professional Component Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Professional Component Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS321		782			Monetary Amount						O	R	1/18
	//Total MSP Patient Liability Met Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total MSP Patient Liability Met Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS322		782			Monetary Amount						O	R	1/18
	//Total Patient Reimbursement Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Patient Reimbursement Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS323		380			Quantity							O	R	1/15
	//Total PIP Claim Count (count of PIP claims)
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total PIP Claim Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS324		782			Monetary Amount						O	R	1/18
	//Total PIP Adjustment Amount (total payment amount for PIP claims)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total PIP Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_TS2(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking, this is a header between claims
	m_bSkipCurrentClaim = FALSE;

	//85		TS2			Provider Supplemental Summary Info.	S		1

	m_strLoopNumber = "2000";

	CString OutputString, str, strFmt;

	COleCurrency cy;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//TS201		782			Monetary Amount						O	R	1/18
	////Total DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS202		782			Monetary Amount						O	R	1/18
	//Total Federal Specific Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Federal Specific Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS203		782			Monetary Amount						O	R	1/18
	//Total Hospital Specific Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Hospital Specific Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS204		782			Monetary Amount						O	R	1/18
	//Total Disproportionate Share Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Disproportionate Share Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS205		782			Monetary Amount						O	R	1/18
	//Total Capital Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Capital Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS206		782			Monetary Amount						O	R	1/18
	//Total Indirect Medical Education Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Indirect Medical Education Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS207		380			Quantity							O	R	1/15
	//Total Outlier Day Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total Outlier Day Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS208		782			Monetary Amount						O	R	1/18
	//Total Day Outlier Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Day Outlier Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS209		782			Monetary Amount						O	R	1/18
	//Total Cost Outlier Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Cost Outlier Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS210		380			Quantity							O	R	1/15
	//Average DRG Length Of Stay
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Average DRG Length Of Stay: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS211		380			Quantity							O	R	1/15
	//Total Discharge Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total Discharge Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS212		380			Quantity							O	R	1/15
	//Total Cost Report Day Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total Cost Report Day Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS213		380			Quantity							O	R	1/15
	//Total Covered Day Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total Covered Day Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS214		380			Quantity							O	R	1/15
	//Total Non-Covered Day Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Total Non-Covered Day Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS215		782			Monetary Amount						O	R	1/18
	//Total MSP Pass-Through Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total MSP Pass-Through Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS216		380			Quantity							O	R	1/15
	//Average DRG Weight
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Average DRG Weight: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//TS217		782			Monetary Amount						O	R	1/18
	//Total PPS Capital FSP DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total PPS Capital FSP DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS218		782			Monetary Amount						O	R	1/18
	//Total PPS Capital HSP DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total PPS Capital HSP DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//TS219		782			Monetary Amount						O	R	1/18
	//Total PPS DSH DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total PPS DSH DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_CLP(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:19) - PLID 37254 - reset claim tracking, this is a new claim
	m_bSkipCurrentClaim = FALSE;

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//89		CLP			Claim Payment Information			R		1

	m_strLoopNumber = "2100";

	CString OutputString, str, strFmt;

	COleCurrency cy;

	// (j.jones 2010-01-27 11:32) - PLID 36998 - check whether the last claim had any services on it
	if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
		const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
		if (pInfo->arypEOBLineItemInfo.GetSize() == 0) {
			//this is not good, the EOB had a claim listed that had no services,
			//we need to output a warning
			strFmt.Format("------------------------------------------------------------------------------\r\n"
				"****WARNING: The EOB did not report any services for the above claim!\r\n"
				"You must contact your clearinghouse to receive a corrected EOB.\r\n"
				"No posting will be made for the claim listed above without a corrected EOB.\r\n"
				"------------------------------------------------------------------------------\r\n\r\n"
				"------------------------------------------------------------------------------\r\n\r\n");
			OutputData(OutputString, strFmt);
		}
	}

	EOBClaimInfo *pClaimInfo = new (EOBClaimInfo);
	// (j.dinatale 2012-12-19 14:44) - PLID 54256 - need to keep track of the check date
	pClaimInfo->dtCheckDate = GetCurrentEOBInfo()->dtCheckDate;

	// (j.jones 2012-05-01 13:47) - PLID 47477 - finally made a constructor for this,
	// so nothing needs to initialize here

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//CLP01		1028		Claim Submitter's Identifier		M	AN	1/38
	//Patient Control Number (Patient ID)
	str = ParseElement(strIn);
	if (str != "") {
		if (m_strLastOutput.Right(2) != "\r\n")
			OutputData(OutputString, "\r\n\r\n");
		strFmt.Format("Patient ID: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		// (j.jones 2009-10-01 14:26) - PLID 35711 - after outputting what number
		// really came back, we need to strip off our prepended code, if our setting
		// to export claims 
		BOOL bFound = FALSE;
		for (int i = 0; i<m_arystrPrependedPatientIDCodes.GetSize() && !bFound; i++) {
			CString strCode = m_arystrPrependedPatientIDCodes.GetAt(i);
			if (str.GetLength() > strCode.GetLength()
				&& str.Left(strCode.GetLength()).CompareNoCase(strCode) == 0) {

				//we have a match, so remove the code
				bFound = TRUE;
				str = str.Right(str.GetLength() - strCode.GetLength());
			}
		}

		pClaimInfo->strPatientID = str;
	}

	//CLP02		1029		Claim Status Code					M	ID	1/2



	// (j.jones 2015-07-16 09:37) - PLID 60207 - added ability to control whether we respect the Processed As flag	
	// (j.jones 2016-04-13 16:13) - NX-100184 - removed this preference, now we just track the ProcessedAs flag at
	// all times and let the matching logic later deal with it
	/*
	enum EProcessedAs {
		eProcessedAs_DoNotUse = 0,				//0 - Do not use the 'Processed As' flag
		eProcessedAs_Always = 1,				// 1 - Always use the 'Processed As' flag
		eProcessedAs_PrimaryOnly = 2,			// 2 - Only use 'Processed As Primary'
		eProcessedAs_SecondaryTertiaryOnly = 3,	// 3 - Only use 'Processed As Secondary/Tertiary'
	};
	EProcessedAs eProcessedAsSetting = (EProcessedAs)GetRemotePropertyInt("EOB_ProcessedAs", 0, 0, "<None>", true);
	*/

	//1 - Processed as Primary
	//2 - Processed as Secondary
	//3 - Processed as Tertiary
	//4 - Denied
	//5 - Pended
	//10 - Received, but not in process
	//13 - Suspended
	//15 - Suspended - investigation with field
	//16 - Suspended - return with material
	//17 - Suspended - review pending
	//19 - Processed as Primary, Forwarded to Additional Payer(s)
	//20 - Processed as Secondary, Forwarded to Additional Payer(s)
	//21 - Processed as Tertiary, Forwarded to Additional Payer(s)
	//22 - Reversal of Previous Payment
	//23 - Not Our Claim, Forwarded to Additional Payer(s)
	//25 - Predetermination Pricing Only - No Payment
	//27 - Reviewed
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "Claim Status: ";
		if (str == "1") {
			strFmt += "Processed as Primary";

			// (j.jones 2012-05-01 13:45) - PLID 47477 - track that it was processed as priority 1
			pClaimInfo->nProcessedAs = 1;
		}
		else if (str == "2") {
			strFmt += "Processed as Secondary";

			// (j.jones 2012-05-01 13:45) - PLID 47477 - track that it was processed as priority 2
			pClaimInfo->nProcessedAs = 2;
		}
		else if (str == "3") {
			strFmt += "Processed as Tertiary";

			// (j.jones 2012-05-01 13:45) - PLID 47477 - track that it was processed as priority 3
			pClaimInfo->nProcessedAs = 3;
		}
		else if (str == "4")
			strFmt += "Denied";
		else if (str == "5")
			strFmt += "Pended";
		else if (str == "10")
			strFmt += "Received, but not in process";
		else if (str == "13")
			strFmt += "Suspended";
		else if (str == "15")
			strFmt += "Suspended - investigation with field";
		else if (str == "16")
			strFmt += "Suspended - return with material";
		else if (str == "17")
			strFmt += "Suspended - review pending";
		else if (str == "19") {
			strFmt += "Processed as Primary, Forwarded to Additional Payer(s)";

			// (j.jones 2016-04-13 16:16) - NX-100184 - track that it was processed as priority 1
			pClaimInfo->nProcessedAs = 1;
		}
		else if (str == "20") {
			strFmt += "Processed as Secondary, Forwarded to Additional Payer(s)";

			// (j.jones 2016-04-13 16:16) - NX-100184 - track that it was processed as priority 2
			pClaimInfo->nProcessedAs = 2;
		}
		else if (str == "21") {
			strFmt += "Processed as Tertiary, Forwarded to Additional Payer(s)";

			// (j.jones 2016-04-13 16:16) - NX-100184 - track that it was processed as priority 3
			pClaimInfo->nProcessedAs = 3;
		}
		else if (str == "22") {
			strFmt += "Reversal of Previous Payment";

			// (j.jones 2011-02-09 11:11) - PLID 42391 - track that this is a reversal
			// (j.jones 2012-04-23 12:07) - PLID 49846 - we now track specifically that this was a reversed claim
			pClaimInfo->bIsReversedClaim = TRUE;
		}
		else if (str == "23")
			strFmt += "Not Our Claim, Forwarded to Additional Payer(s)";
		else if (str == "25")
			strFmt += "Predetermination Pricing Only - No Payment";
		else if (str == "27")
			strFmt += "Reviewed";
		else
			strFmt += "Unknown";

		OutputData(OutputString, strFmt);
		OutputData(OutputString, "\r\n\r\n");
	}

	//CLP03		782			Monetary Amount						M	R	1/18
	//Total Claim Charge Amount (total charge amount for this claim)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Claim Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
		pClaimInfo->cyClaimChargeAmt = cy;
	}

	//CLP04		782			Monetary Amount						M	R	1/18
	//Claim Payment Amount (total payment amount for this claim)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Total Claim Payment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
		pClaimInfo->cyClaimPaymentAmt = cy;
	}

	//CLP05		782			Monetary Amount						O	R	1/18
	//Patient Responsibility Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Patient Responsibility Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
		pClaimInfo->cyClaimPatientResp = cy;
	}

	//CLP06		1032		Claim Filing Indicator Code			O	ID	1/2
	//plan type
	//12 - Preferred Provider Organization (PPO)
	//13 - Point of Service (POS)
	//14 - Exclusive Provider Organization (EPO)
	//15 - Indemnity Insurance
	//16 - Health Maintenance Organization (HMO) Medicare Risk
	//AM - Automobile Medical
	//CH - Champus
	//DS - Disability
	//HM - Health Maintenance Organization
	//LM - Liability Medical
	//MA - Medicare Part A
	//MB - Medicare Part B
	//MC - Medicaid
	//OF - Other Federal Program
	//TV - Title V
	//VA - Veteran Administration Plan
	//WC - Workers’ Compensation Health Claim
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "Claim Type: ";
		if (str == "12")
			strFmt += "Preferred Provider Organization (PPO)";
		else if (str == "13")
			strFmt += "Point of Service (POS)";
		else if (str == "14")
			strFmt += "Exclusive Provider Organization (EPO)";
		else if (str == "15")
			strFmt += "Indemnity Insurance";
		else if (str == "16")
			strFmt += "Health Maintenance Organization (HMO) Medicare Risk";
		else if (str == "AM")
			strFmt += "Automobile Medical";
		else if (str == "CH")
			strFmt += "Champus";
		else if (str == "DS")
			strFmt += "Disability";
		else if (str == "HM")
			strFmt += "Health Maintenance Organization";
		else if (str == "LM")
			strFmt += "Liability Medical";
		else if (str == "MA")
			strFmt += "Medicare Part A";
		else if (str == "MB")
			strFmt += "Medicare Part B";
		else if (str == "MC")
			strFmt += "Medicaid";
		else if (str == "OF")
			strFmt += "Other Federal Program";
		else if (str == "TV")
			strFmt += "Title V";
		else if (str == "VA")
			strFmt += "Veteran Administration Plan";
		else if (str == "WC")
			strFmt += "Workers’ Compensation Health Claim";
		else
			strFmt += "Unknown";
		OutputData(OutputString, strFmt);
		OutputData(OutputString, "\r\n\r\n");
	}

	//CLP07		127			Reference Identification			O	AN	1/30
	//Payer Claim Control Number
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Payer Claim Control Number: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
		// (b.spivey, November 01, 2012) - PLID 49943 - store this in a new variable for the EOBClaimInfo item. 
		// (b.spivey, November 08, 2012) - PLID 49943 - I was getting the previous EOBClaimInfo object, not the current one. 
		//		This was wrong.
		pClaimInfo->strOriginalRefNum = str;

	}

	//CLP08		1331		Facility Code Value					O	AN	1/2
	//Facility Type Code (The code from CLM05-1 of the 837 claim)
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Facility Type Code: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//CLP09		1325		Claim Frequency Type Code			O	ID	1/1
	//Claim Frequency Code (The code from CLM05-2 of the 837 claim)
	str = ParseElement(strIn);

	//CLP10		1352		Patient Status Code					O	ID	1/2
	//NOT USED
	str = ParseElement(strIn);

	//CLP11		1354		Diagnosis Related Group (DRG) Code	O	ID	1/4
	str = ParseElement(strIn);

	//CLP12		380			Quantity							O	R	1/15
	//Diagnosis Related Group (DRG) Weight
	str = ParseElement(strIn);

	//CLP13		954			Percent								O	R	1/10
	//Discharge Fraction
	str = ParseElement(strIn);

	//CLP14		1073			Yes/No Condition or Response Code		O 1 ID 1/1
	// (c.haag 2010-10-21 15:50) - PLID 40349 - ANSI 5010 element
	str = ParseElement(strIn);


	/////////Reversal Check/////////////////

	// (j.jones 2011-02-09 11:52) - PLID 42391 - now we can find out if this is
	// the re-posting of a previously reversed claim, which means comparing to
	// ensure that this is the same claim as the previous one, with a reversed
	// cyClaimChargeAmt value
	if (!pClaimInfo->bIsReversedClaim && ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
		//this claim is not a reversal, and we have at least one existing claim
		EOBClaimInfo *pLastClaimInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
		if (pLastClaimInfo->bIsReversedClaim
			&& pLastClaimInfo->strPatientID == pClaimInfo->strPatientID
			&& pLastClaimInfo->cyClaimChargeAmt == -(pClaimInfo->cyClaimChargeAmt)) {

			//The last claim was a reversal for the same patient,
			//with an opposite claim charge amount, which means that
			//this claim is re-posting the previous claim.
			//We do not currently support this, so we need to flag this
			//claim as also being a reversal.

			// (j.jones 2012-04-23 12:07) - PLID 49846 - We now track specifically that this was a reposted claim
			// and we track the pointer to the reversed claim. Similarly, the reversed claim points to this reposted claim.
			pClaimInfo->bIsRepostedClaim = TRUE;
			pClaimInfo->pReversedSibling = pLastClaimInfo;
			pLastClaimInfo->pRepostedSibling = pClaimInfo;
		}
	}

	////////////////////////////////////////


	//now add this claim to the array
	ptrEOBInfo->arypEOBClaimInfo.Add(pClaimInfo);

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_CAS(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//Claims Adjustment, used in:

	//95		CAS			Claim Adjustment					S		99
	//148		CAS			Service Adjustment					S		99

	//JMJ - we track the m_strLoopNumber so we can have a good idea whether we are on a claim adjustment
	//or a service adjustment, since the brilliant people who came up with this standard kinda sorta
	//forgot to put that indicator in this segment. Presumably, the CLP should be the first segment in
	//the 2100 (Claim) loop and SVC should be the first segment in the 2210 (Service) loop, and as long
	//as the sender of the file creates it in that manner then we will know which type of adjustment this is

	CString OutputString, str, strFmt;

	CString strGroupCode, strReasonCode;

	COleCurrency cy;

	if (strIn != "") {
		OutputData(OutputString, "----------------------------------------------------------------------------\r\n\r\n");
		if (m_strLoopNumber == "2100")
			OutputData(OutputString, "Claim Adjustment\r\n\r\n");
		else if (m_strLoopNumber == "2110")
			OutputData(OutputString, "Line Item Adjustment\r\n\r\n");
	}

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//CAS01		1033		Claim Adjustment Group Code			M	ID	1/2
	//CO - Contractual Obligations
	//CR - Correction and Reversals
	//OA - Other adjustments
	//PI - Payor Initiated Reductions
	//PR - Patient Responsibility
	strGroupCode = ParseElement(strIn);

	// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description

	if (strGroupCode != "") {
		strFmt.Format("Claim Adjustment Category: %s - ", strGroupCode);
		if (strGroupCode == "CO")
			strFmt += "Contractual Obligations";
		else if (strGroupCode == "CR")
			strFmt += "Correction and Reversals";
		else if (strGroupCode == "OA")
			strFmt += "Other Adjustments";
		else if (strGroupCode == "PI")
			strFmt += "Payor Initiated Reductions";
		else if (strGroupCode == "PR")
			strFmt += "Patient Responsibility";
		OutputData(OutputString, strFmt);
		OutputData(OutputString, "\r\n");
	}

	//CAS02		1034		Claim Adjustment Reason Code		M	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	CString strReason;
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS03		782			Monetary Amount						M	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - If the code is PR, it means patient responsibility, and is not treated as an adjustment
		//in our system, so we will set the patient resp. indicator to TRUE
		//add the adjustment
		// (j.jones 2012-04-23 11:11) - PLID 49846 - Reversals use CR on all group codes, so we have
		// to look for reason codes 1, 2, or 3 (Deductible, Coinsurance, Copayment) to identify patient resp.
		// We can probably do that at all times, but it's safer to only check when the group is CR.
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS04		380			Quantity							O	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//the remaining iterations are used when additional adjustments apply
	//within the group identified in CAS01

	//CAS05		1034		Claim Adjustment Reason Code		X	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	strReason = "";
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS06		782			Monetary Amount						X	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - see CAS03
		//add the adjustment
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS07		380			Quantity							X	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//CAS08		1034		Claim Adjustment Reason Code		X	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	strReason = "";
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS09		782			Monetary Amount						X	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - see CAS03
		//add the adjustment
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS10		380			Quantity							X	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//CAS11		1034		Claim Adjustment Reason Code		X	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	strReason = "";
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS12		782			Monetary Amount						X	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - see CAS03
		//add the adjustment
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS13		380			Quantity							X	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//CAS14		1034		Claim Adjustment Reason Code		X	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	strReason = "";
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS15		782			Monetary Amount						X	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - see CAS03
		//add the adjustment
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS16		380			Quantity							X	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//CAS17		1034		Claim Adjustment Reason Code		X	ID	1/5
	//(Code Source 139: Adjustment Reason Code)
	strReasonCode = ParseElement(strIn);
	strReason = "";
	if (strReasonCode != "") {
		strReason = GetReasonCode(strReasonCode);
		// (j.jones 2008-05-20 16:21) - PLID 28931 - ensured we output the code alongside the description
		strFmt.Format("\r\nAdjustment Reason: %s - %s\r\n", strReasonCode, strReason);
		OutputData(OutputString, strFmt);
	}

	//CAS18		782			Monetary Amount						X	R	1/18
	//Adjustment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		//JMJ - see CAS03
		//add the adjustment
		BOOL bPatResp = (strGroupCode == "PR" || (strGroupCode == "CR" && (strReasonCode == "1" || strReasonCode == "2" || strReasonCode == "3")));
		AddAdjustment(cy, bPatResp, strGroupCode, strReasonCode, strReason);
	}

	//CAS19		380			Quantity							X	R	1/15
	//Adjustment Quantity (units of service being adjusted)
	//a positive value decreases the paid units of service,
	//a negative value increases the paid units
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Adjustment Quantity: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "\r\n");

	OutputData(OutputString, "----------------------------------------------------------------------------\r\n\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_NM1(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything,
	// even though this is the segment where we detect whether to skip a claim, we would not do so
	// if we're already skipping it, and this boolean is reset for the next new claim
	if (m_bSkipCurrentClaim) {
		return;
	}

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//Name, used in:

	//102		NM1			Patient Name						R		1
	//105		NM1			Insured Name						S		1
	//108		NM1			Corrected Patient/Insured Name		S		1
	//111		NM1			Service Provider Name				S		1
	//114		NM1			Crossover Carrier Name				S		1
	//116		NM1			Corrected Priority Payer Name		S		2
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 
	//			NM1			Other Subscriber Name

	m_strLoopNumber = "2100";

	CString OutputString, str, strFmt;

	CString strEntity, strQual, strIdent, strCode;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//NM101		98			Entity Identifier Code				M	ID	2/3
	//QC - Patient
	//IL - Insured or Subscriber
	//74 - Corrected Insured
	//82 - Rendering Provider
	//TT - Transfer To
	//PR - Payer
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010 enumerations
	//GB - Other insured
	strEntity = ParseElement(strIn);
	if (strEntity != "") {
		if (strEntity == "QC")
			strFmt.Format("Patient Name: ");
		else if (strEntity == "IL")
			strFmt.Format("Insured/Subscriber Name: ");
		else if (strEntity == "74")
			strFmt.Format("Corrected Insured Name: ");
		else if (strEntity == "82")
			strFmt.Format("Rendering Provider Name: ");
		else if (strEntity == "TT")
			strFmt.Format("Transfer To: ");
		else if (strEntity == "PR")
			strFmt.Format("Payer Name: ");
		else if (strEntity == "GB")
			strFmt.Format("Other Insured: ");
		else
			strFmt.Format("Unknown Identity: ");
		OutputData(OutputString, strFmt);
	}

	//fill the provider name iff strEntity = 82
	CString strProviderName = "";

	//NM102		1065		Entity Type Qualifier				M	ID	1/1
	//1 - Person
	//2 - Non-Person Entity
	strQual = ParseElement(strIn);

	//NM103		1035		Name Last or Organization Name		O	AN	1/35
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("%s", str);

		if (strEntity == "82") {
			strProviderName = str;
		}
		else if (strEntity == "QC") {
			//store the patient's name
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				pInfo->strPatientLast = str;
			}
		}

		OutputData(OutputString, strFmt);
	}

	//NM104		1036		Name First							O	AN	1/25
	//if NM102 == "2" then NOT USED
	str = ParseElement(strIn);
	if (strQual != "2" && str != "") {
		strFmt.Format(", %s", str);

		if (strEntity == "82") {
			strProviderName += strFmt;
		}
		else if (strEntity == "QC") {
			//store the patient's name
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				pInfo->strPatientFirst = str;
			}
		}

		OutputData(OutputString, strFmt);
	}

	//NM105		1037		Name Middle							O	AN	1/25
	//if NM102 == "2" then NOT USED
	str = ParseElement(strIn);
	if (strQual != "2" && str != "") {
		strFmt.Format(" %s", str);

		if (strEntity == "82") {
			strProviderName += strFmt;
		}
		else if (strEntity == "QC") {
			//store the patient's name
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				pInfo->strPatientMiddle = str;
			}
		}

		OutputData(OutputString, strFmt);
	}

	if (strEntity == "82") {
		ptrEOBInfo->strProviderName = strProviderName;
	}

	//NM106		1038		Name Prefix							O	AN	1/10
	//NOT USED
	str = ParseElement(strIn);

	//NM107		1039		Name Suffix							O	AN	1/10
	//if NM102 == "2" then NOT USED
	str = ParseElement(strIn);
	if (strQual != "2" && str != "") {
		strFmt.Format(" %s", str);
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "\r\n");

	//NM108		66			Identification Code Qualifier		X	ID	1/2
	//if NM101 == "QC" then:
	//34 - Social Security Number
	//HN - Health Insurance Claim (HIC) Number
	//II - United States National Individual Identifier
	//MI - Member Identification Number
	//MR - Medicaid Recipient Identification Number
	//if NM101 == "IL" then:
	//34 Social Security Number
	//HN Health Insurance Claim (HIC) Number
	//MI Member Identification Number
	//if NM101 == "74" then:
	//C - Insured's Changed Unique Identification Number
	//if NM101 == "82" then:
	//BD - Blue Cross Provider Number
	//BS - Blue Shield Provider Number
	//FI - Federal Taxpayer’s Identification Number
	//MC - Medicaid Provider Number
	//PC - Provider Commercial Number
	//SL - State License Number
	//UP - Unique Physician Identification Number (UPIN)
	//XX - Health Care Financing Administration National Provider Identifier
	//if NM101 == "TT" or "PR" then:
	//AD - Blue Cross Blue Shield Association Plan Code
	//FI - Federal Taxpayer’s Identification Number
	//NI - National Association of Insurance Commissioners (NAIC) Identification
	//PI - Payor Identification
	//PP - Pharmacy Processor Number
	//XV - Health Care Financing Administration National PlanID
	// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010: if NM101 == "GB" then
	//FI - Federal Taxpayer’s Identification Number
	//II - Standard Unique Health Identifier for each Individual in the United States
	//MI - Member Identification Number
	strIdent = ParseElement(strIn);

	//NM109		67			Identification Code					X	AN	2/80
	CString strRenderingProviderID;
	strCode = ParseElement(strIn);
	if (strCode != "") {
		if (strEntity == "QC") {
			strFmt.Format("Patient ID: %s\r\n\r\n", strCode);
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				// (j.jones 2011-03-16 16:24) - PLID 42866 - only fill the insurance ID if an IL segment has not already done so,
				// which is likely to happen because QC usually comes before IL
				if (pInfo->strOriginalPatientInsuranceID.IsEmpty()) {
					pInfo->strOriginalPatientInsuranceID = strCode;
				}
			}
		}
		else if (strEntity == "IL") {
			strFmt.Format("Insured/Subscriber ID: %s\r\n\r\n", strCode);
			if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
				EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
				// (j.jones 2011-03-16 16:24) - PLID 42866 - fill the "original" insurance ID, always, because if IL
				// exists, it is certainly more accurate than a QC patient ID
				pInfo->strOriginalPatientInsuranceID = strCode;
			}
		}
		else if (strEntity == "74") {
			strFmt.Format("Corrected Insured ID: %s\r\n\r\n", strCode);
			// (j.jones 2010-08-12 10:00) - PLID 40088 - if they send a corrected ID, use it
			strCode.TrimLeft();
			strCode.TrimRight();
			if (!strCode.IsEmpty()) {
				if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
					EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
					// (j.jones 2011-03-16 16:28) - PLID 42866 - track this as a corrected ID so we don't lose the original ID
					pInfo->strCorrectedPatientInsuranceID = strCode;
				}
			}
		}
		else if (strEntity == "82") {
			// (j.jones 2010-02-09 11:58) - PLID 37254 - track the rendering provider ID
			strRenderingProviderID = strCode;
			strFmt.Format("Rendering Provider ID: %s\r\n\r\n", strRenderingProviderID);
		}
		else if (strEntity == "TT")
			strFmt.Format("Transfer To ID: %s\r\n\r\n", strCode);
		else if (strEntity == "PR") {
			strFmt.Format("Payer ID: %s\r\n\r\n", strCode);
			ptrEOBInfo->strPayerID = strCode;
		}
		// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010
		else if (strEntity == "GB") {
			strFmt.Format("Other Subscriber ID: %s\r\n\r\n", strCode);
		}

		else
			strFmt.Format("Unknown Identity ID: %s\r\n\r\n", strCode);
		OutputData(OutputString, strFmt);
	}

	//NM110		706			Entity Relationship Code			X	ID	2/2
	//NOT USED
	str = ParseElement(strIn);

	//NM111		98			Entity Identifier Code				O	ID	2/3
	//NOT USED
	str = ParseElement(strIn);


	m_OutputFile.Write(OutputString, OutputString.GetLength());

	// (j.jones 2010-02-09 11:58) - PLID 37254 - See if we need to skip this claim.
	// If no IDs are in ERemitClaimFilteredIDsT, then we import all claims. If there are any
	// IDs in ERemitClaimFilteredIDsT, then we only import the claims that have a Rendering
	// Provider ID tracked in that table (the contents of which are cached in m_arystrClaimFilteredIDs)	
	if (!strRenderingProviderID.IsEmpty() && m_arystrClaimFilteredIDs.GetSize() > 0) {
		//we have a provider ID, and we are filtering on at least one claim provider ID
		for (int i = 0; i<m_arystrClaimFilteredIDs.GetSize(); i++) {
			if (strRenderingProviderID.CompareNoCase(m_arystrClaimFilteredIDs.GetAt(i)) == 0) {
				//they match, which means we WILL import this claim,
				//so just return now
				m_bSkipCurrentClaim = FALSE;
				return;
			}
		}

		//if we got here it means that we are filtering on IDs, but the provider ID
		//for the current claim is not part of our filter, so we need to say so,
		//and drop the currently tracked claim
		if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
			EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
			ClearClaimInfo(ptrEOBInfo, pInfo);
		}

		//we are now going to skip everything in the claim file until the next claim begins
		m_bSkipCurrentClaim = TRUE;

		//and output that we skipped this claim
		OutputString = "";
		OutputData(OutputString, "***************************************************************************************************************\r\n");
		OutputData(OutputString, "**** This claim has been SKIPPED due to having a Rendering Provider ID that is not in your filtered list.  ****\r\n");
		OutputData(OutputString, "**** The list of filtered IDs can be reviewed in the 'Configure EOB Import Filters' screen.                ****\r\n");
		OutputData(OutputString, "***************************************************************************************************************\r\n\r\n\r\n");
		m_OutputFile.Write(OutputString, OutputString.GetLength());
		return;
	}
}

void CANSI835Parser::ANSI_MIA(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//118		MIA			Inpatient Adjudication Information	S		1

	m_strLoopNumber = "2100";

	CString OutputString, str, strFmt;

	COleCurrency cy;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//MIA01		380			Quantity							M	R	1/15
	//Covered Days or Visits Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Covered Days or Visits Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//MIA02		380			Quantity							O	R	1/15
	//PPS Operating Outlier Amount
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("PPS Operating Outlier Amount: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//MIA03		380			Quantity							O	R	1/15
	//Lifetime Psychiatric Days Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Lifetime Psychiatric Days Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//MIA04		782			Monetary Amount						O	R	1/18
	//Claim DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA05		127			Reference Identification			O	AN	1/30
	//Remark Code
	str = ParseElement(strIn);

	//MIA06		782			Monetary Amount						O	R	1/18
	//Claim Disproportionate Share Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim Disproportionate Share Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA07		782			Monetary Amount						O	R	1/18
	//Claim MSP Pass-through Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim MSP Pass-through Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA08		782			Monetary Amount						O	R	1/18
	//Claim PPS Capital Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim PPS Capital Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA09		782			Monetary Amount						O	R	1/18
	//PPS-Capital FSP DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Capital FSP DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA10		782			Monetary Amount						O	R	1/18
	//PPS-Capital HSP DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Capital HSP DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA11		782			Monetary Amount						O	R	1/18
	//PPS-Capital DSH DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Capital DSH DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA12		782			Monetary Amount						O	R	1/18
	//Old Capital Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Old Capital Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA13		782			Monetary Amount						O	R	1/18
	//PPS-Capital IME Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Capital IME Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA14		782			Monetary Amount						O	R	1/18
	//PPS-Operating Hospital Specific DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Operating Hospital Specific DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA15		380			Quantity							O	R	1/15
	//Cost Report Day Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Cost Report Day Count: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//MIA16		782			Monetary Amount						O	R	1/18
	//PPS-Operating Federal Specific DRG Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Operating Federal Specific DRG Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA17		782			Monetary Amount						O	R	1/18
	//Claim PPS Capital Outlier Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim PPS Capital Outlier Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA18		782			Monetary Amount						O	R	1/18
	//Claim Indirect Teaching Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim Indirect Teaching Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA19		782			Monetary Amount						O	R	1/18
	//Nonpayable Professional Component Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Nonpayable Professional Component Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MIA20		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);
	}

	//MIA21		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);
	}

	//MIA22		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);
	}

	//MIA23		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);
	}

	//MIA24		782			Monetary Amount						O	R	1/18
	//PPS-Capital Exception Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("PPS-Capital Exception Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_MOA(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//123		MOA			Outpatient Adjudication Information	S		1

	m_strLoopNumber = "2100";

	CString OutputString, str, strFmt;

	COleCurrency cy;

	// (j.dinatale 2012-11-06 10:01) - PLID 50792 - need the last claim's information
	EOBClaimInfo *pLastClaimInfo = GetLastClaimInfo();

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//MOA01		954			Percent								O	R	1/10
	//Reimbursement Rate
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Reinbursement Rate: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//MOA02		782			Monetary Amount						O	R	1/18
	//Claim HCPCS Payable Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim HCPCS Payable Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MOA03		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);

		// (j.dinatale 2012-11-05 11:59) - PLID 50792 - signal that we want to unbatch
		// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to check for N89
		if (!pLastClaimInfo->bHasMA18orN89 && (!str.CompareNoCase("MA18") || !str.CompareNoCase("N89"))){
			pLastClaimInfo->bHasMA18orN89 = TRUE;
		}
	}

	//MOA04		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);

		// (j.dinatale 2012-11-05 11:59) - PLID 50792 - signal that we want to unbatch
		// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to check for N89
		if (!pLastClaimInfo->bHasMA18orN89 && (!str.CompareNoCase("MA18") || !str.CompareNoCase("N89"))){
			pLastClaimInfo->bHasMA18orN89 = TRUE;
		}
	}

	//MOA05		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);

		// (j.dinatale 2012-11-05 11:59) - PLID 50792 - signal that we want to unbatch
		// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to check for N89
		if (!pLastClaimInfo->bHasMA18orN89 && (!str.CompareNoCase("MA18") || !str.CompareNoCase("N89"))){
			pLastClaimInfo->bHasMA18orN89 = TRUE;
		}
	}

	//MOA06		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);

		// (j.dinatale 2012-11-05 11:59) - PLID 50792 - signal that we want to unbatch
		// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to check for N89
		if (!pLastClaimInfo->bHasMA18orN89 && (!str.CompareNoCase("MA18") || !str.CompareNoCase("N89"))){
			pLastClaimInfo->bHasMA18orN89 = TRUE;
		}
	}

	//MOA07		127			Reference Identification			O	AN	1/30
	//Remark Code (Code Source 411)
	str = ParseElement(strIn);
	if (str != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Remittance Remark: %s - %s\r\n\r\n", str, GetRemarkCode(str));
		OutputData(OutputString, strFmt);

		// (j.dinatale 2012-11-05 11:59) - PLID 50792 - signal that we want to unbatch
		// (j.dinatale 2012-12-19 11:09) - PLID 54256 - need to check for N89
		if (!pLastClaimInfo->bHasMA18orN89 && (!str.CompareNoCase("MA18") || !str.CompareNoCase("N89"))){
			pLastClaimInfo->bHasMA18orN89 = TRUE;
		}
	}

	//MOA08		782			Monetary Amount						O	R	1/18
	//Claim ESRD Payment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Claim ESRD Payment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//MOA09		782			Monetary Amount						O	R	1/18
	//Nonpayable Professional Component Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Nonpayable Professional Component Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_AMT(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//Monetary Amount, used in:

	//135		AMT			Claim Supplemental Information		S		14
	//158		AMT			Service Supplemental Amount			S		12

	CString OutputString, str, strFmt;

	CString strQual;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//AMT01		522			Amount Qualifier Code				M	ID	1/3
	//if Loop 2100 (Claim Supplemental Information):
	//AU - Coverage Amount (Use this monetary amount to report the total covered charges.)
	//D8 - Discount Amount (Prompt Pay Discount Amount)
	//DY - Per Day Limit
	//F5 - Patient Amount Paid (Use this monetary amount for the amount the	patient has already paid.)
	//I - Interest
	//NL - Negative Ledger Balance
	//T - Tax
	//T2 - Total Claim Before Taxes
	//ZK - Federal Medicare or Medicaid Payment Mandate - Category 1
	//ZL - Federal Medicare or Medicaid Payment Mandate - Category 2
	//ZM - Federal Medicare or Medicaid Payment Mandate - Category 3
	//ZN - Federal Medicare or Medicaid Payment Mandate - Category 4
	//ZO - Federal Medicare or Medicaid Payment Mandate - Category 5
	//ZZ - Mutually Defined
	//if Loop 2110 (Service Supplemental Amount):
	//B6 - Allowed - Actual
	//DY -  Per Day Limit (Medicare uses this to report the provider per diem amount, where applicable.)
	//KH - Deduction Amount (Late Filing Reduction)
	//NE -  Net Billed		
	//T - Tax
	//T2 - Total Claim Before Taxes (Use this monetary amount for the service charge before taxes.)
	//ZK - Federal Medicare or Medicaid Payment Mandate - Category 1
	//ZL - Federal Medicare or Medicaid Payment Mandate - Category 2
	//ZM - Federal Medicare or Medicaid Payment Mandate - Category 3
	//ZN - Federal Medicare or Medicaid Payment Mandate - Category 4
	//ZO - Federal Medicare or Medicaid Payment Mandate - Category 5
	strQual = ParseElement(strIn);

	//AMT02		782			Monetary Amount						M	R	1/18
	str = ParseElement(strIn);
	if (str != "") {
		COleCurrency cy;
		cy.ParseCurrency(str);
		strFmt = "";
		if (strQual == "AU")
			strFmt.Format("Coverage Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "D8")
			strFmt.Format("Discount Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "DY")
			strFmt.Format("Per Day Limit: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "F5")
			strFmt.Format("Patient Amount Paid: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "I")
			strFmt.Format("Interest: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "NL")
			strFmt.Format("Negative Ledger Balance: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "T")
			strFmt.Format("Tax: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "T2")
			strFmt.Format("Total Claim Before Taxes: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZK")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 1 Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZL")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 2 Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZM")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 3 Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZN")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 4 Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZO")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 5 Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "ZZ")
			strFmt.Format("Operational Cost / Day Outlier Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "B6") {
			strFmt.Format("Allowed - Actual Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));

			// (j.jones 2011-01-07 16:43) - PLID 41980 - track this allowable
			if (m_strLoopNumber == "2100") {
				//claim allowable
				if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
					EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
					//by default this is invalid, initialize it to zero, then add the given amount
					if (pInfo->cyClaimAllowedAmt.GetStatus() == COleCurrency::invalid) {
						pInfo->cyClaimAllowedAmt = COleCurrency(0, 0);
					}
					pInfo->cyClaimAllowedAmt += cy;
				}
			}
			else if (m_strLoopNumber == "2110") {
				//line item adjustment
				if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
					const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
					if (pInfo->arypEOBLineItemInfo.GetSize() > 0) {
						EOBLineItemInfo *pLineInfo = pInfo->arypEOBLineItemInfo[pInfo->arypEOBLineItemInfo.GetSize() - 1];
						//by default this is invalid, initialize it to zero, then add the given amount
						if (pLineInfo->cyChargeAllowedAmt.GetStatus() == COleCurrency::invalid) {
							pLineInfo->cyChargeAllowedAmt = COleCurrency(0, 0);
						}
						pLineInfo->cyChargeAllowedAmt += cy;
					}
				}
			}
		}
		else if (strQual == "KH")
			strFmt.Format("Deduction Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		else if (strQual == "NE")
			strFmt.Format("Net Billed Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//AMT03		478			SomeSortOfNote						O	ID	1/1
	//NOT USED
	str = ParseElement(strIn);


	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_QTY(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//Quantity, used in:

	//137		QTY			Claim Supplemental Info. Quantity	S		15
	//160		QTY			Service Supplemental Quantity		S		6

	CString OutputString, str, strFmt;

	CString strQual;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//QTY01		673			Quantity Qualifier					M	ID	2/2
	//if Loop 2100 (Claim Supplemental Info. Quantity):
	//CA - Covered - Actual
	//CD - Co-insured - Actual
	//LA - Life-time Reserve - Actual
	//LE - Life-time Reserve - Estimated
	//NA - Number of Non-covered Days
	//NE - Non-Covered - Estimated
	//NR - Not Replaced Blood Units
	//OU - Outlier Days
	//PS - Prescription
	//VS - Visits
	//ZK - Federal Medicare or Medicaid Payment Mandate - Category 1
	//ZL - Federal Medicare or Medicaid Payment Mandate - Category 2
	//ZM - Federal Medicare or Medicaid Payment Mandate - Category 3
	//ZN - Federal Medicare or Medicaid Payment Mandate - Category 4
	//ZO - Federal Medicare or Medicaid Payment Mandate - Category 5
	//if Loop 2110 (Service Supplemental Quantity):
	//NE -  Non-Covered - Estimated (Use this code for actual line item non-covered visits.)
	//ZK - Federal Medicare or Medicaid Payment Mandate - Category 1
	//ZL - Federal Medicare or Medicaid Payment Mandate - Category 2
	//ZM - Federal Medicare or Medicaid Payment Mandate - Category 3
	//ZN - Federal Medicare or Medicaid Payment Mandate - Category 4
	//ZO - Federal Medicare or Medicaid Payment Mandate - Category 5
	strQual = ParseElement(strIn);

	//QTY02		380			Quantity							X	R	1/15
	str = ParseElement(strIn);
	if (str != "") {
		strFmt = "";
		if (strQual == "CA")
			strFmt.Format("Covered - Actual: %s\r\n\r\n", str);
		else if (strQual == "CD")
			strFmt.Format("Co-insured - Actual: %s\r\n\r\n", str);
		else if (strQual == "LA")
			strFmt.Format("Life-time Reserve - Actual: %s\r\n\r\n", str);
		else if (strQual == "LE")
			strFmt.Format("Life-time Reserve - Estimated: %s\r\n\r\n", str);
		else if (strQual == "NA")
			strFmt.Format("Number of Non-covered Days: %s\r\n\r\n", str);
		else if (strQual == "NE")
			strFmt.Format("Non-Covered - Estimated: %s\r\n\r\n", str);
		else if (strQual == "NR")
			strFmt.Format("Not Replaced Blood Units: %s\r\n\r\n", str);
		else if (strQual == "OU")
			strFmt.Format("Outlier Days: %s\r\n\r\n", str);
		else if (strQual == "PS")
			strFmt.Format("Prescription: %s\r\n\r\n", str);
		else if (strQual == "VS")
			strFmt.Format("Visits: %s\r\n\r\n", str);
		else if (strQual == "ZK")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 1: %s\r\n\r\n", str);
		else if (strQual == "ZL")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 2: %s\r\n\r\n", str);
		else if (strQual == "ZM")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 3: %s\r\n\r\n", str);
		else if (strQual == "ZN")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 4: %s\r\n\r\n", str);
		else if (strQual == "ZO")
			strFmt.Format("Federal Medicare or Medicaid Payment Mandate - Category 5: %s\r\n\r\n", str);

		OutputData(OutputString, strFmt);
	}

	//QTY03		C001		Composite Unit Of Measure			O
	//NOT USED
	str = ParseElement(strIn);

	//QTY04		61			Free-Form Message					X	AN	1/30
	//NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_SVC(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//139		SVC			Service Payment Information			S		1

	m_strLoopNumber = "2110";

	CString OutputString, str, strFmt;

	CString strQual, strID;

	COleCurrency cy;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	if (strIn != "") {
		OutputData(OutputString, "----------------------------------------------------------------------------\r\n\r\n");
	}

	EOBLineItemInfo *pLineItemInfo = new (EOBLineItemInfo);
	// (j.jones 2012-05-01 14:13) - PLID 47477 - finally made a constructor for this,
	// we no longer have to initialize the content here


	//SVC01		C003		Composite Medical Procedure Ident.	M
	CString strComposite = ParseElement(strIn);

	//SVC01 - 1	235			Product/Service ID Qualifier		M	ID	2/2
	//AD - American Dental Association Codes
	//ER - Jurisdiction Specific Procedure and Supply Codes
	//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
	//ID - International Classification of Diseases Clinical Modification (ICD-9-CM) - Procedure
	//IV - Home Infusion EDI Coalition (HIEC) Product/Service Code
	//N1 - National Drug Code in 4-4-2 Format
	//N2 - National Drug Code in 5-3-2 Format
	//N3 - National Drug Code in 5-4-1 Format
	//N4 - National Drug Code in 5-4-2 Format
	//ND - National Drug Code (NDC)
	//NU - National Uniform Billing Committee (NUBC) UB92 Codes
	//RB - National Uniform Billing Committee (NUBC) UB82 Codes
	//ZZ - Mutually Defined
	// (c.haag 2010-10-21 15:24) - PLID 40349 - ANSI 5010 code HP
	//HP Health Insurance Prospective Payment System (HIPPS) Skilled Nursing Facility Rate Code
	strQual = ParseComposite(strComposite);

	//SVC01 - 2	234			Product/Service ID					M	AN	1/48
	//Procedure Code
	strID = ParseComposite(strComposite);
	if (strID != "") {
		strFmt.Format("Service ID: %s\r\n\r\n", strID);
		OutputData(OutputString, strFmt);

		pLineItemInfo->strServiceID = strID;
	}

	//SVC01 - 3	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		pLineItemInfo->strModifier1 = str;
	}

	//SVC01 - 4	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		pLineItemInfo->strModifier2 = str;
	}

	//SVC01 - 5	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		pLineItemInfo->strModifier3 = str;
	}

	//SVC01 - 6 1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		pLineItemInfo->strModifier4 = str;
	}

	//SVC01 - 7	352			Description							O	AN	1/80
	//Procedure Code Description
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Description: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//SVC02		782			Monetary Amount						M	R	1/18
	//Line Item Charge Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Line Item Charge Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		pLineItemInfo->cyLineItemChargeAmt = cy;
	}

	//SVC03		782			Monetary Amount						O	R	1/18
	//Line Item Provider Payment Amount
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Line Item Provider Payment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);

		pLineItemInfo->cyLineItemPaymentAmt = cy;
	}

	//SVC04		234			Product/Service ID					O	AN	1/48
	//Revenue Code (when an Institutional claim)
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Revenue Code: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//SVC05		380			Quantity							O	R	1/15
	//Units Of Service Paid Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Units of Service Paid: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//SVC06		C003		Composite Medical Procedure Ident.	O
	//Only used if the code adjudicated is different from the code submitted.
	//If the codes are the same, SV106 will not be present.
	strComposite = ParseElement(strIn);

	//SVC06 - 1	235			Product/Service ID Qualifier		M	ID	2/2
	//AD - American Dental Association Codes
	//ER - Jurisdiction Specific Procedure and Supply Codes
	//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
	//ID - International Classification of Diseases Clinical Modification (ICD-9-CM) - Procedure
	//IV - Home Infusion EDI Coalition (HIEC) Product/Service Code
	//N4 - National Drug Code in 5-4-2 Format
	//NU - National Uniform Billing Committee (NUBC) UB92 Codes
	//RB - National Uniform Billing Committee (NUBC) UB82 Codes
	//ZZ - Mutually Defined
	CString strSVC06Qual = ParseComposite(strComposite);

	//SVC06 - 2	234			Product/Service ID					M	AN	1/48
	//Procedure Code
	str = ParseComposite(strComposite);
	str.TrimLeft();
	str.TrimRight();
	if (str != "" && str != "0") {
		strFmt.Format("(The above code is the service that was adjudicated. The following is the original code submitted.)\r\n\r\n"
			"Service ID: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		//overwrite our existing info

		// (j.jones 2004-08-23 09:09) - For unknown reasons, Monteiro was getting completely inaccurate data
		//returned in this line, that did not match up in Practice. I think that this is either 1. garbage or 2.
		//this is the code adjudicated. It is most definitely not the original code submitted.

		// (j.jones 2009-06-26 12:36) - PLID 34736 - We brought this feature back because a client had a legitimate
		// case where this code would have worked. We have a preference for this ability now, defaulting to on.

		// (j.jones 2013-01-25 12:15) - PLID 54853 - We saw cases where N4 was returned and this code was an NDC code.
		// We don't send NDC codes, so there's no way the contents of this element are accurate.
		// If this is not a code type we could have ever sent, then IsValidServiceCodeQualifier() will return false.
		// We currently only send HC, so anything that is not HC would be invalid, but IsValidServiceCodeQualifier()
		// is called so we can define more valid qualifiers in the future.
		if (IsValidServiceCodeQualifier(strSVC06Qual) && GetRemotePropertyInt("EOBSupportOriginalCode", 1, 0, "<None>", true) == 1) {
			pLineItemInfo->strServiceID = str;
		}
	}

	//SVC06 - 3	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		/*
		pLineItemInfo->strModifier1 = str;
		*/
	}

	//SVC06 - 4	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		/*
		pLineItemInfo->strModifier2 = str;
		*/
	}

	//SVC06 - 5	1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		/*
		pLineItemInfo->strModifier3 = str;
		*/
	}

	//SVC06 - 6 1339		Product Modifier					O	AN	2/2
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Modifier: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);

		/*
		pLineItemInfo->strModifier4 = str;
		*/
	}

	//SVC06 - 7	352			Description							O	AN	1/80
	//Procedure Code Description
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Service Description: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//SVC07		380			Quantity							O	R	1/15
	//Original Units of Service Count
	str = ParseElement(strIn);
	if (str != "") {
		strFmt.Format("Original Units of Service Submitted: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//now add to the last claim
	if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
		EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
		pInfo->arypEOBLineItemInfo.Add(pLineItemInfo);
	}

	OutputData(OutputString, "----------------------------------------------------------------------------\r\n\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_LQ(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything
	if (m_bSkipCurrentClaim) {
		return;
	}

	//162		LQ			Health Care Remark Codes			S		99

	CString OutputString, str, strFmt;

	CString strQual, strCode;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//LQ01		1270		Code List Qualifier Code			O	ID	1/3
	//HE - Claim Payment Remark Codes (Code Source 411)
	//RX - National Council for Prescription Drug Programs Reject/Payment Codes
	strQual = ParseElement(strIn);

	//LQ02		1271		Industry Code						X	AN	1/30
	//Remark Code
	strCode = ParseElement(strIn);
	if (strQual == "HE" && strCode != "") {
		// (j.jones 2015-07-06 11:06) - PLID 66359 - changed to output the remark code as well
		strFmt.Format("Claim Payment Remark: %s - %s\r\n\r\n", strCode, GetRemarkCode(strCode));
		OutputData(OutputString, strFmt);
	}

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

void CANSI835Parser::ANSI_PLB(CString &strIn) {

	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - neither reset nor skip the claim, this is a non-claim-specific record

	//164		PLB			Provider Adjustment					S		>1

	CString OutputString, str, strFmt;

	COleCurrency cy;

	if (strIn != "") {
		OutputData(OutputString, "****************************************************************************\r\n\r\n");
		OutputData(OutputString, "Provider Adjustments (not claim-specific): \r\n\r\n");
	}

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//PLB01		127			Reference Identification			M	AN	1/30
	//Provider Identifier (as assigned by the payer)
	str = ParseElement(strIn);
	if (str != "") {
		if (m_strLastOutput.Right(2) != "\r\n")
			OutputData(OutputString, "\r\n\r\n");
		strFmt.Format("Provider ID: %s\r\n\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB02		373			Date								M	DT	8/8
	//Fiscal Period Date (CCYYMMDD) (last day of the provider's fiscal year)
	str = ParseElement(strIn);

	//PLB03		C042		Adjustment Identifier				M
	CString strComposite = ParseElement(strIn);

	//PLB03 - 1	426			Adjustment Reason Code				M	ID	2/2
	//50 - Late Charge (used for the Late Claim Filing Penalty or Medicare Late Cost Report Penalty)
	//51 - Interest Penalty Charge (used for the interest assessment for late filing)
	//72 - Authorized Return (This monetary amount is the provider refund adjustment. This adjustment acknowledges a refund	received from a provider for previous overpayment)
	//90 - Early Payment Allowance
	//AM - Applied to Borrower’s Account
	//AP - Acceleration of Benefits (used to reflect accelerated payment amounts or withholdings)
	//B2 - Rebate (used for the refund adjustment.)
	//B3 - Recovery Allowance (used to represent the check received from the provider for overpayments generated by payments from other payers)
	//BD - Bad Debt Adjustment (used for the bad debt passthrough)
	//BN - Bonus
	//C5 - Temporary Allowance (This is the tentative adjustment)
	//CR - Capitation Interest
	//CS - Adjustment
	//CT - Capitation Payment
	//CV - Capital Passthru
	//CW - Certified Registered Nurse Anesthetist Passthru.
	//DM - Direct Medical Education Passthru
	//E3 - Withholding
	//FB - Forwarding Balance (used for the balance forward)
	//FC - Fund Allocation
	//GO - Graduate Medical Education Passthru
	//IP - Incentive Premium Payment
	//IR - Internal Revenue Service Withholding
	//IS - Interim Settlement (used for the interim rate lump sum)
	//J1 - Nonreimbursable (used to offset claim or service level data that reflects what could be paid if not for demonstration program or other limitation that prevents issuance of payment)
	//L3 - Penalty (used for the capitation-related penalty, penalty withholding, or penalty release adjustment)
	//L6 - Interest Owed (used for the interest paid on	claims in this 835)
	//LE - Levy (IRS Levy)
	//LS - Lump Sum (used for a disproportionate share adjustment, indirect medical education passthrough, nonphysician passthrough, passthrough lump sum adjustment, or other passthrough amount)
	//OA - Organ Acquisition Passthru
	//OB - Offset for Affiliated Providers
	//PI - Periodic Interim Payment (used for the PIP lump sum, PIP	payment, or adjustment after PIP)
	//PL - Payment Final (used for the final settlement)
	//RA - Retro-activity Adjustment
	//RE - Return on Equity
	//SL - Student Loan Repayment
	//TL - Third Party Liability
	//WO - Overpayment Recovery (used for the recovery of previous overpayment)
	//WU - Unspecified Recovery (used for the outside recovery adjustment)
	//ZZ - Mutually Defined
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB03 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB04		782			Monetary Amount						M	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//PLB05		C042		Adjustment Identifier				X
	str = strComposite = ParseElement(strIn);

	//PLB05 - 1	426			Adjustment Reason Code				M	ID	2/2
	//See PLB03 - 1
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB05 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB06		782			Monetary Amount						X	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//PLB07		C042		Adjustment Identifier				X
	str = strComposite = ParseElement(strIn);

	//PLB07 - 1	426			Adjustment Reason Code				M	ID	2/2
	//See PLB03 - 1
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB07 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB08		782			Monetary Amount						X	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//PLB09		C042		Adjustment Identifier				X
	str = strComposite = ParseElement(strIn);

	//PLB09 - 1	426			Adjustment Reason Code				M	ID	2/2
	//See PLB03 - 1
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB09 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB10		782			Monetary Amount						X	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//PLB11		C042		Adjustment Identifier				X
	str = strComposite = ParseElement(strIn);

	//PLB11 - 1	426			Adjustment Reason Code				M	ID	2/2
	//See PLB03 - 1
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB11 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB12		782			Monetary Amount						X	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	//PLB13		C042		Adjustment Identifier				X
	str = strComposite = ParseElement(strIn);

	//PLB13 - 1	426			Adjustment Reason Code				M	ID	2/2
	//See PLB03 - 1
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment Reason: %s\r\n", GetAdjustmentPLBCode(str));
		OutputData(OutputString, strFmt);
	}

	//PLB13 - 2	127			Reference Identification			O	ID	1/30
	//Provider Adjustment Identifier
	str = ParseComposite(strComposite);
	if (str != "") {
		strFmt.Format("Adjustment ID: %s\r\n", str);
		OutputData(OutputString, strFmt);
	}

	//PLB14		782			Monetary Amount						X	R	1/18
	//Provider Adjustment Amount (for the preceding adjustment reason)
	str = ParseElement(strIn);
	if (str != "") {
		cy.ParseCurrency(str);
		strFmt.Format("Provider Adjustment Amount: %s\r\n\r\n", FormatCurrencyForInterface(cy, TRUE, TRUE));
		OutputData(OutputString, strFmt);
	}

	OutputData(OutputString, "****************************************************************************\r\n");

	m_OutputFile.Write(OutputString, OutputString.GetLength());

}

// (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010
void CANSI835Parser::ANSI_RDM(CString& strIn)
{
	// (j.jones 2010-02-09 09:45) - PLID 37174 - if we're on a skipped EOB, do not parse anything
	if (m_bSkipCurrentEOB) {
		return;
	}

	// (j.jones 2010-02-09 14:33) - PLID 37254 - if we're on a skipped claim, do not parse anything,
	// even though this is the segment where we detect whether to skip a claim, we would not do so
	// if we're already skipping it, and this boolean is reset for the next new claim
	if (m_bSkipCurrentClaim) {
		return;
	}

	m_strLoopNumber = "1000B";

	CString OutputString, str, strFmt;

	//RDM01 756 Report Transmission Code M 1 ID 1/2
	//
	//BM - By Mail
	//EM - E-Mail
	//FT - File Transfer
	//OL - On-Line
	CString strEntity = ParseElement(strIn);
	if (strEntity != "")
	{
		if (strEntity == "BM")
			strFmt.Format("Address: ");
		else if (strEntity == "EM")
			strFmt.Format("EMail: ");
		else if (strEntity == "FT")
			strFmt.Format("FTP Site: ");
		else if (strEntity == "OL")
			strFmt.Format("Online Site: ");
		else
			strFmt.Format("Unknown Transmission Method: ");
	}

	//RDM02 93 Name O 1 AN 1/60
	str = ParseElement(strIn);
	if (str != "" && strEntity == "BM") {
		OutputData(OutputString, strFmt);
		OutputData(OutputString, str + "\r\n");
	}

	//RDM03 364 Communication Number O 1 AN 1/256
	str = ParseElement(strIn);
	if (str != "" && (strEntity == "EM" || strEntity == "FT" || strEntity == "OL")) {
		OutputData(OutputString, strFmt);
		OutputData(OutputString, str + "\r\n");
	}

	//RDM04 C040 REFERENCE IDENTIFIER O 1
	str = ParseElement(strIn); // Not Used

	//RDM05 C040 REFERENCE IDENTIFIER O 1
	str = ParseElement(strIn); // Not Used

	m_OutputFile.Write(OutputString, OutputString.GetLength());
}

CString CANSI835Parser::GetRemarkCode(CString strCode) {

	// (j.jones 2015-07-06 10:41) - PLID 66359 - this content is now in AdjustmentCodesT with a Type of 3
	if (!g_bAdjustmentCodesInitialized) {
		InitializeAdjustmentCodes();
	}

	CString strDescription = g_mapRemarkCodes[strCode];
	if (strDescription.IsEmpty()) {
		//perhaps this is a newer code we do not support yet?		
		strDescription = "Undefined Remark Code";

		//if a developer hits this, it means it's time to update our master remark code list
		ASSERT(FALSE);
	}

	return strDescription;
}

CString CANSI835Parser::GetReasonCode(CString strCode) {

	// (j.jones 2015-07-06 10:41) - PLID 66359 - this content is now in AdjustmentCodesT with a Type of 2
	if (!g_bAdjustmentCodesInitialized) {
		InitializeAdjustmentCodes();
	}

	CString strDescription = g_mapReasonCodes[strCode];
	if (strDescription.IsEmpty()) {
		//perhaps this is a newer code we do not support yet?		
		strDescription = "Undefined Adjustment Reason";

		//if a developer hits this, it means it's time to update our master reason code list
		ASSERT(FALSE);
	}

	if (strCode == "18") {
		//Duplicate claim/service

		//special case, this is used to allow skipping processing claims

		EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

		//set as the current claim date
		if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
			EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
			pInfo->bDuplicateClaim = TRUE;
		}
	}

	return strDescription;
}

// (j.jones 2015-07-06 10:39) - PLID 66359 - renamed to GetAdjustmentPLBCode to clarify
// that this is not the same list as the conventional adjustment reason codes that
// are stored in data
CString CANSI835Parser::GetAdjustmentPLBCode(CString strCode) {

	if (strCode == "50")
		return "Late Charge";
	else if (strCode == "51")
		return "Interest Penalty Charge";
	else if (strCode == "72")
		return "Authorized Return";
	else if (strCode == "90")
		return "Early Payment Allowance";
	else if (strCode == "AM")
		return "Applied to Borrower’s Account";
	else if (strCode == "AP")
		return "Acceleration of Benefits";
	else if (strCode == "B2")
		return "Rebate";
	else if (strCode == "B3")
		return "Recovery Allowance";
	else if (strCode == "BD")
		return "Bad Debt Adjustment";
	else if (strCode == "BN")
		return "Bonus";
	else if (strCode == "C5")
		return "Temporary Allowance";
	else if (strCode == "CR")
		return "Capitation Interest";
	else if (strCode == "CS")
		return "Adjustment";
	else if (strCode == "CT")
		return "Capitation Payment";
	else if (strCode == "CV")
		return "Capital Passthru";
	else if (strCode == "CW")
		return "Certified Registered Nurse Anesthetist Passthru.";
	else if (strCode == "DM")
		return "Direct Medical Education Passthru";
	else if (strCode == "E3")
		return "Withholding";
	else if (strCode == "FB")
		return "Forwarding Balance";
	else if (strCode == "FC")
		return "Fund Allocation";
	else if (strCode == "GO")
		return "Graduate Medical Education Passthru";
	else if (strCode == "IP")
		return "Incentive Premium Payment";
	else if (strCode == "IR")
		return "Internal Revenue Service Withholding";
	else if (strCode == "IS")
		return "Interim Settlement";
	else if (strCode == "J1")
		return "Nonreimbursable";
	else if (strCode == "L3")
		return "Penalty";
	else if (strCode == "L6")
		return "Interest Owed";
	else if (strCode == "LE")
		return "Levy";
	else if (strCode == "LS")
		return "Lump Sum";
	else if (strCode == "OA")
		return "Organ Acquisition Passthru";
	else if (strCode == "OB")
		return "Offset for Affiliated Providers";
	else if (strCode == "PI")
		return "Periodic Interim Payment";
	else if (strCode == "PL")
		return "Payment Final";
	else if (strCode == "RA")
		return "Retro-activity Adjustment";
	else if (strCode == "RE")
		return "Return on Equity";
	else if (strCode == "SL")
		return "Student Loan Repayment";
	else if (strCode == "TL")
		return "Third Party Liability";
	else if (strCode == "WO")
		return "Overpayment Recovery";
	else if (strCode == "WU")
		return "Unspecified Recovery";
	else if (strCode == "ZZ")
		return "Unknown";
	else
		return "Unknown";
}

//adds an adjustment to either a charge or claim
// (j.jones 2006-11-16 10:08) - PLID 23551 - added the GroupCode and ReasonCode as parameters,
// as they are now stored on the adjustment
void CANSI835Parser::AddAdjustment(COleCurrency cyAdj, BOOL bPatResp, CString strGroupCode, CString strReasonCode, CString strReason) {

	EOBInfo *ptrEOBInfo = GetCurrentEOBInfo();

	//because this file is created in structured loops, you can be certain
	//that the adjustment is for the last charge added to the last claim added

	EOBAdjustmentInfo *pAdjInfo = new (EOBAdjustmentInfo);
	pAdjInfo->cyAdjustmentAmt = cyAdj;
	pAdjInfo->bPatResp = bPatResp;
	pAdjInfo->strReason = strReason;
	// (j.jones 2006-11-16 10:33) - PLID 23551 - aupported groupcode and reasoncode
	pAdjInfo->strGroupCode = strGroupCode;
	pAdjInfo->strReasonCode = strReasonCode;
	// (j.jones 2012-04-20 08:57) - PLID 49846 - initialize the takeback adjustment info.
	pAdjInfo->nReversedAdjustmentID = -1;
	pAdjInfo->bMissingReversedAdjustmentOK = FALSE;

	if (m_strLoopNumber == "2100") {
		//claim adjustment
		if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
			EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
			//if the adjustment is negative, insert it at the beginning,
			//because later it is preferable if we post negative adjustments
			//prior to positive adjustments
			if (pAdjInfo->cyAdjustmentAmt < COleCurrency(0, 0)) {
				pInfo->arypEOBAdjustmentInfo.InsertAt(0, pAdjInfo);
			}
			else {
				pInfo->arypEOBAdjustmentInfo.Add(pAdjInfo);
			}
		}
	}
	else if (m_strLoopNumber == "2110") {
		//line item adjustment
		if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
			const EOBClaimInfo *pInfo = ptrEOBInfo->arypEOBClaimInfo[ptrEOBInfo->arypEOBClaimInfo.GetSize() - 1];
			if (pInfo->arypEOBLineItemInfo.GetSize() > 0) {
				EOBLineItemInfo *pLineInfo = pInfo->arypEOBLineItemInfo[pInfo->arypEOBLineItemInfo.GetSize() - 1];
				//if the adjustment is negative, insert it at the beginning,
				//because later it is preferable if we post negative adjustments
				//prior to positive adjustments
				if (pAdjInfo->cyAdjustmentAmt < COleCurrency(0, 0)) {
					pLineInfo->arypEOBAdjustmentInfo.InsertAt(0, pAdjInfo);
				}
				else {
					pLineInfo->arypEOBAdjustmentInfo.Add(pAdjInfo);
				}
			}
		}
	}
}

// (j.jones 2014-02-27 10:40) - PLID 50630 - Track the highest charge count per bill
// in the system, we'll use this to invalidate bad trace numbers.
// Don't worry about deleted charges here.
long CalculateMaxChargeIndex() {

	long nMaxChargeIndex = 10;
	_RecordsetPtr rsMaxChargeCount = CreateParamRecordset("SELECT Max(CountCharges) AS MaxChargeIndex "
		"FROM (SELECT Count(ID) AS CountCharges FROM ChargesT GROUP BY BillID) AS ChargesQ");
	if (!rsMaxChargeCount->eof) {
		nMaxChargeIndex = VarLong(rsMaxChargeCount->Fields->Item["MaxChargeIndex"]->Value, 10);
		//for safety purposes, force this to be no less than 10
		if (nMaxChargeIndex < 10) {
			nMaxChargeIndex = 10;
		}
	}
	rsMaxChargeCount->Close();
	return nMaxChargeIndex;
}

// (j.armen 2012-02-16 16:12) - PLID 47644 - Parameratized
void CANSI835Parser::CalcInternalIDs(EOBInfo *ptrEOBInfo) {

	//attempt to associate claims with patients and bills, line items with charges, and insurance companies

	m_ptrProgressBar->SetStep(1);

	//Step 1. Assign Patient IDs

	m_ptrProgressBar->SetPos(0);

	m_ptrProgressBar->SetRange(0, ptrEOBInfo->arypEOBClaimInfo.GetSize());

	CString str;
	if (m_CountOfEOBs == 1)
		str = "Assigning Patient IDs...";
	else
		str.Format("Assigning Patient IDs (EOB %li)...", ptrEOBInfo->nIndex + 1);

	m_ptrProgressStatus->SetWindowText(str);

	// (j.jones 2005-11-01 11:08) - PLID 17531 - might have issues if we are using the BillID lookup preference
	BOOL bIsUsingBillIDLookup = GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 1;

	//loop through all claims
	// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
	for (int i = 0; i < ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {

		EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[i];

		// (j.jones 2010-10-12 09:37) - PLID 40892 - 5010 remits have line item trace numbers
		// that, if filled in, should be our charge IDs. If they are valid charge IDs, we can
		// skip a lot of this processing.

		//we know we can't have any IDs yet, so initialize them here
		pClaimInfo->nPatientID = -1;
		// (j.jones 2014-08-26 16:13) - PLID 63397 - initialize the user defined ID
		pClaimInfo->nUserDefinedID = -1;
		pClaimInfo->nBillID = -1;

		// (j.jones 2011-03-14 15:45) - PLID 42806 - do not do this unless it is a 5010 remit
		if (m_bIs5010File) {

			// (j.jones 2014-02-27 10:40) - PLID 50630 - Track the highest charge count per bill
			// in the system, we'll use this to invalidate bad trace numbers.
			// Don't worry about deleted charges here.
			// This is static so this only happens once per Practice session.
			static long nMaxChargeIndex = CalculateMaxChargeIndex();

			//loop through all charges
			for (int j = 0; j < pClaimInfo->arypEOBLineItemInfo.GetSize(); j++) {

				EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[j];

				//we know we can't have any IDs yet, so initialize them here
				pLineItemInfo->nChargeID = -1;
				pLineItemInfo->nProviderID = -1;

				if (!pLineItemInfo->strTraceNumber.IsEmpty()) {
					long nPotentialChargeID = atoi(pLineItemInfo->strTraceNumber);

					// (j.jones 2014-02-27 10:30) - PLID 50630 - Ignore this trace number if it is possibly
					// a charge index on a bill (1, 2, 3, etc.), because that is not a valid trace number.
					if (nPotentialChargeID <= nMaxChargeIndex) {
						//ignore this trace number
						nPotentialChargeID = -1;

						Log("CalcInternalIDs: Trace Number failed - returned trace number could be a charge index. "
							"(Trace ID returned: %li, "
							"Name returned: %s %s %s) "
							"The normal lookup will be performed.", nPotentialChargeID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast);
					}

					if (nPotentialChargeID > 0) {

						// (j.jones 2015-12-23 15:11) - PLID 67669 - If this is a corrected charge,
						// the newest corrected charge ID will be found.
						// If the charge is only voided, -1 will be returned.
						// If this charge does not exist, the nPotentialChargeID won't change.
						long nOriginalTraceID = nPotentialChargeID;
						nPotentialChargeID = FindNewestCorrectedLineItemID(nOriginalTraceID);

						if (nPotentialChargeID == -1) {
							//this is only possible if the charge was found, but was voided,
							//this code isn't hit if the charge wasn't actually found
							//the trace found nothing, log this
							Log("CalcInternalIDs: Trace Number failed - the charge was voided without being corrected, and cannot be posted to. "
								"(Trace ID returned: %li, "
								"Name returned: %s %s %s) "
								"The normal lookup will be performed.", nOriginalTraceID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast);

							//we DO want to perform the normal lookup, as they may have voided the charge
							//and manually recreated it, instead of voiding & correcting
						}
						else {

							// (j.jones 2015-12-23 15:17) - PLID 67669 - moved this logic earlier since we already know
							// whether or not the charge was corrected
							CString strCorrected = "";
							if (nOriginalTraceID != nPotentialChargeID) {
								strCorrected.Format("***Original Charge ID %li has been corrected to New ChargeID %li.*** ", nOriginalTraceID, nPotentialChargeID);
							}

							//so far so good, now find out if this charge exists
							// (j.jones 2011-08-29 16:58) - PLID 44804 - look at LineItemCorrectionsT, if this is correcting an "original"
							// charge, we should use the "new" charge ID instead
							// (j.jones 2013-07-19 08:57) - PLID 57616 - get the bill's insured party ID and other insured party ID
							// (j.jones 2014-02-27 09:33) - PLID 50630 - Get the patient's name and see if it matches, using the same
							// first/middle OR last/suffix logic that we use elsewhere to validate patient ID matching. This will
							// likely fail if we somehow allowed correcting the charge and moving to another patient.
							// (j.jones 2015-12-23 14:36) - PLID 67669 - ensured we do not return original/voided charges
							_RecordsetPtr rsCharge = CreateParamRecordset("SELECT "
								"ChargesT.ID, "
								"ChargesT.BillID, "
								"BillsT.PatientID, "
								"ChargesT.DoctorsProviders, "
								"PatientsT.UserDefinedID, "
								"LineItemT.Deleted AS ChargeDeleted, "
								"BillsT.Deleted AS BillDeleted, "
								"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
								"PersonT.FullName, PersonT.Title, "
								"CASE WHEN (PersonT.First = {STRING} OR PersonT.First + ' ' + PersonT.Middle = {STRING}) OR (PersonT.Last = {STRING} OR PersonT.Last + ' ' + PersonT.Title = {STRING}) "
								"	THEN Convert(bit, 1) ELSE Convert(bit, 0) END AS PatientNameMatches "
								"FROM ChargesT "
								"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
								"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
								"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
								"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
								"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
								"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
								"WHERE ChargesT.ID = {INT} "
								"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL)",
								pClaimInfo->strPatientFirst, pClaimInfo->strPatientFirst, pClaimInfo->strPatientLast, pClaimInfo->strPatientLast,
								nPotentialChargeID);
							if (rsCharge->eof) {
								//the trace found nothing, log this
								Log("CalcInternalIDs: Trace Number failed - did not match any Charge ID in the system. "
									"(Trace ID returned: %li, "
									"Name returned: %s %s %s) "
									"The normal lookup will be performed.", nOriginalTraceID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast);
							}
							else {
								//we have a record - but is it deleted?
								BOOL bChargeDeleted = AdoFldBool(rsCharge, "ChargeDeleted", FALSE);
								BOOL bBillDeleted = AdoFldBool(rsCharge, "BillDeleted", FALSE);
								long nActualChargeID = AdoFldLong(rsCharge, "ID");
								long nActualBillID = AdoFldLong(rsCharge, "BillID");
								long nActualPatientID = AdoFldLong(rsCharge, "PatientID");
								long nActualProviderID = AdoFldLong(rsCharge, "DoctorsProviders", -1); // (c.haag 2016-05-02 10:00) - NX-100403 - This column is nullable now
								long nActualUserDefinedID = AdoFldLong(rsCharge, "UserDefinedID");
								// (j.jones 2013-07-19 08:57) - PLID 57616 - track the bill's insured party ID and other insured party ID
								long nActualBillInsuredPartyID = AdoFldLong(rsCharge, "InsuredPartyID", -1);
								long nActualBillOthrInsuredPartyID = AdoFldLong(rsCharge, "OthrInsuredPartyID", -1);

								if (bChargeDeleted) {
									Log("CalcInternalIDs: Trace Number failed - matched up with a deleted Charge ID. "
										"(Charge ID: %li, "
										"Name returned: %s %s %s) %s"
										"The normal lookup will be performed.", nActualChargeID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast, strCorrected);
								}
								else if (bBillDeleted) {
									//would technically be bad data, you can't have a deleted bill and not a deleted charge
									Log("CalcInternalIDs: Trace Number failed - matched up with a Charge ID for a deleted bill. "
										"(Charge ID: %li, "
										"Name returned: %s %s %s) %s"
										"The normal lookup will be performed.", nActualChargeID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast, strCorrected);
								}
								// (j.jones 2014-02-27 09:33) - PLID 50630 - If the patient name does not match, refuse to use this trace number.
								// This uses the same first/middle OR last/suffix logic that we use elsewhere to validate patient ID matching,
								// which corrects cases where their last name changes or the first name was abbreviated, etc.
								else if (!AdoFldBool(rsCharge, "PatientNameMatches", FALSE)) {
									CString strEOBName, strRealPatientName;
									strEOBName.Format("%s, %s", pClaimInfo->strPatientLast, pClaimInfo->strPatientFirst);
									strRealPatientName = AdoFldString(rsCharge, "FullName", "");
									CString strTitle = AdoFldString(rsCharge, "Title", "");
									if (!strTitle.IsEmpty()) {
										strRealPatientName += " ";
										strRealPatientName += strTitle;
									}
									Log("CalcInternalIDs: Trace Number failed - matched up with a Charge ID for a different patient. "
										"(Charge ID: %li, "
										"Name in the EOB: %s, Name in Practice: %s) "
										"The normal lookup will be performed.", nActualChargeID, strEOBName, strRealPatientName);
								}
								else {
									//not deleted, and the patient name matches, so let's assign our IDs
									pLineItemInfo->nChargeID = nActualChargeID;
									pLineItemInfo->nProviderID = nActualProviderID;
									pClaimInfo->nPatientID = nActualPatientID;
									pClaimInfo->nBillID = nActualBillID;
									// (j.jones 2014-08-26 16:13) - PLID 63397 - track the user defined ID
									pClaimInfo->nUserDefinedID = nActualUserDefinedID;
									// (j.jones 2013-07-19 08:57) - PLID 57616 - track the bill's insured party ID and other insured party ID
									pClaimInfo->nBillInsuredPartyID = nActualBillInsuredPartyID;
									pClaimInfo->nBillOthrInsuredPartyID = nActualBillOthrInsuredPartyID;

									Log("CalcInternalIDs: Trace Number matched up with a Charge ID. "
										"(Charge ID: %li, "
										"Name returned: %s %s %s) %s"
										"No further processing is necessary.", nActualChargeID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast, strCorrected);
								}
							}
							rsCharge->Close();
						}
					}
				}
			}
		}

		// (j.jones 2010-10-12 10:13) - PLID 40892 - we may have already figured out the patient ID,
		// if so, we can skip all this code
		if (pClaimInfo->nPatientID == -1) {

			//figure out which patient this is

			//we will always be sending the user defined ID in claims, so that will be what we get back

			// (j.jones 2014-08-26 16:18) - PLID 63397 - renamed this to nReportedPatientID,
			// to reflect that this is merely what we read from the file
			long nReportedUserDefinedID = atoi(pClaimInfo->strPatientID);

			if (AsString(nReportedUserDefinedID) != pClaimInfo->strPatientID) {
				//hmm, doesn't match up!
				//try trimming zeros
				CString strPatientID = pClaimInfo->strPatientID;
				strPatientID.TrimLeft();
				strPatientID.TrimRight();
				strPatientID.TrimLeft("0");
				if (AsString(nReportedUserDefinedID) != strPatientID) {
					//doesn't match, which can happen if we get "00006014ABC123", it would atoi to 6014!
					nReportedUserDefinedID = -1;
				}
			}

			// (j.jones 2014-08-26 16:18) - PLID 63397 - if filled, nActualUserDefinedID
			// will represent the real UserDefinedID from data
			long nActualUserDefinedID = -1;
			long nPersonID = -1;
			_RecordsetPtr rsPatID = CreateParamRecordset(
				"SELECT PersonID, Last, First, Middle, Title FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"WHERE UserDefinedID = {INT}",
				nReportedUserDefinedID);
			if (!rsPatID->eof) {
				nPersonID = AdoFldLong(rsPatID, "PersonID", -1);
				// (j.jones 2014-08-26 16:19) - PLID 63397 - we know the reported patient ID was the real patient ID
				nActualUserDefinedID = nReportedUserDefinedID;

				// (j.jones 2006-06-21 16:43) - PLID 21142 - do a simple name match, see if the first OR last name
				// returned in the EOB match the person for our ID. If neither match, then we can be rather sure we
				// got a bad ID and should search by name instead. If one and not the other matches, it could be a 
				// maiden name change or we only have a shortened version of their first name. 99% of the time,
				// if a returned ID matches an ID in the system, it's correct, so this can't be too strict

				// (j.jones 2014-02-04 09:56) - PLID 59615 - handle cases where the remit file includes
				// the Last + Suffix in the same field and/or the First + Middle
				// in the same field (which they should not be doing... and yet...)

				CString strLastFromEOB = pClaimInfo->strPatientLast;
				CString strFirstFromEOB = pClaimInfo->strPatientFirst;

				CString strLastFromData = AdoFldString(rsPatID, "Last", "");
				CString strFirstFromData = AdoFldString(rsPatID, "First", "");
				CString strLastTitleFromData = strLastFromData + " " + AdoFldString(rsPatID, "Title", "");
				CString strFirstMiddleFromData = strFirstFromData + " " + AdoFldString(rsPatID, "Middle", "");

				strLastFromEOB.TrimRight();
				strFirstFromEOB.TrimRight();
				strLastFromData.TrimRight();
				strFirstFromData.TrimRight();
				strLastTitleFromData.TrimRight();
				strFirstMiddleFromData.TrimRight();

				if (!strLastFromEOB.IsEmpty() && strLastFromEOB.CompareNoCase(strLastFromData) != 0
					&& !strLastFromEOB.IsEmpty() && strLastFromEOB.CompareNoCase(strLastTitleFromData) != 0
					&& !strFirstFromEOB.IsEmpty() && strFirstFromEOB.CompareNoCase(strFirstFromData) != 0
					&& !strFirstFromEOB.IsEmpty() && strFirstFromEOB.CompareNoCase(strFirstMiddleFromData) != 0) {

					//neither the first nor last names from the EOB match the data

					// (j.jones 2006-08-14 11:00) - PLID 21960 - see if the Last + First from the EOB
					// matches the either name from the data, or vice versa
					CString strLastFirstFromEOB = strLastFromEOB + " " + strFirstFromEOB;
					CString strFirstLastFromEOB = strFirstFromEOB + " " + strLastFromEOB;

					//don't bother trying the middle name / title comparisons here

					if (strLastFirstFromEOB.CompareNoCase(strLastFromData) != 0 &&
						strFirstLastFromEOB.CompareNoCase(strLastFromData) != 0 &&
						strLastFirstFromEOB.CompareNoCase(strFirstFromData) != 0 &&
						strFirstLastFromEOB.CompareNoCase(strFirstFromData) != 0) {


						//now we can presume that the ID we received was pure garbage.
						//Log appropriately, and next we'll search by name alone.

						//no match by ID, so log as such, we will search by name next
						Log("CalcInternalIDs: Failed to calculate Patient ID because the UserDefinedID matched a patient with a different name. "
							"(ID returned: %li, "
							"Name returned: %s %s, "
							"Name found in data: %s %s) %s"
							"Now trying to check by name.", nReportedUserDefinedID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientLast,
							strLastFromData, strFirstFromData, bIsUsingBillIDLookup ? "SearchByBillID Is Enabled. " : "");

						nPersonID = -1;
					}
				}
			}
			else {
				//no match by ID, so log as such, we will search by name next
				Log("CalcInternalIDs: Failed to calculate Patient ID because the UserDefinedID didn't match. "
					"(ID returned: %li, "
					"Name returned: %s %s %s) %s"
					"Now trying to check by name.", nReportedUserDefinedID, pClaimInfo->strPatientFirst, pClaimInfo->strPatientMiddle, pClaimInfo->strPatientLast,
					bIsUsingBillIDLookup ? "SearchByBillID Is Enabled. " : "");

				nPersonID = -1;
			}
			rsPatID->Close();

			// (j.jones 2005-10-31 08:31) - PLID 17448 - attempt to match the patient by name, if the ID doesn't match
			if (nPersonID == -1) {

				//the patient ID returned doesn't match an ID in the system, that's a huge problem,
				//but we can at least attempt to recover by checking by name
				if (!pClaimInfo->strPatientLast.IsEmpty() && !pClaimInfo->strPatientFirst.IsEmpty()) {
					// (j.jones 2014-02-04 09:46) - PLID 59615 - check against First + Middle and Last + Title
					// incase the remit file includes the Last + Suffix in the same field and/or the First + Middle
					// in the same field (which they should not be doing... and yet...)
					rsPatID = CreateParamRecordset(
						"SELECT PersonID, UserDefinedID FROM PatientsT "
						"WHERE PersonID IN ("
						"	SELECT ID FROM PersonT "
						"	WHERE (First = {STRING} OR First + ' ' + Middle = {STRING}) AND (Last = {STRING} OR Last + ' ' + Title = {STRING}))",
						pClaimInfo->strPatientFirst, pClaimInfo->strPatientFirst, pClaimInfo->strPatientLast, pClaimInfo->strPatientLast);
					if (!rsPatID->eof) {
						if (rsPatID->GetRecordCount() > 1) {
							//more than one person matched by first and last name

							// (j.jones 2005-11-01 11:12) - PLID 17531 - if the BillID lookup preference is enabled,
							// see if the ID quasi-matches
							if (bIsUsingBillIDLookup) {

								Log("CalcInternalIDs: Too many records matched by first and last name, and SearchByBillID is enabled, now matching by left of ID string.");

								long nSuggestedIDToUse = -1;
								long nCountFound = 0;

								while (!rsPatID->eof) {
									long nTestUserDefinedID = AdoFldLong(rsPatID, "UserDefinedID", -1);
									CString strTestUserDefinedID = AsString(nTestUserDefinedID);;
									// (j.jones 2007-07-27 13:04) - PLID 26849 - compare to the original returned ID,
									// not our hacked up, converted long which is probably -1 right now!
									CString strUserDefinedID = pClaimInfo->strPatientID;
									if (strUserDefinedID.GetLength() > strTestUserDefinedID.GetLength()
										&& strUserDefinedID.Left(strTestUserDefinedID.GetLength()) == strTestUserDefinedID) {

										//the name matches, and the EOB's ID begins with the patient's ID, so log that we found one
										nSuggestedIDToUse = AdoFldLong(rsPatID, "PersonID", -1);
										nCountFound++;
									}
									rsPatID->MoveNext();
								}

								if (nCountFound == 1) {
									nPersonID = nSuggestedIDToUse;
									Log("CalcInternalIDs: Check by name and left of ID string successful.");
								}
								else if (nCountFound > 1) {
									Log("CalcInternalIDs: Check by name and left of ID string returned multiple records.");
								}
								else {
									Log("CalcInternalIDs: Check by name and left of ID string returned no records.");
								}
							}

							if (nPersonID == -1) {

								//try filtering by middle initial if it exists

								Log("CalcInternalIDs: Too many records matched by first and last name, now matching by middle initial.");

								if (!pClaimInfo->strPatientMiddle.IsEmpty()) {
									// (j.jones 2014-02-04 09:46) - PLID 59615 - check against Last + Title as well incase the
									// remit file includes the Last + Suffix in the same field
									_RecordsetPtr rsPatIDByMid = CreateParamRecordset("SELECT PersonID, UserDefinedID FROM PatientsT "
										"WHERE PersonID IN (SELECT ID FROM PersonT WHERE First = {STRING} AND (Last = {STRING} OR Last + ' ' + Title = {STRING}) AND Middle LIKE {STRING})",
										pClaimInfo->strPatientFirst, pClaimInfo->strPatientLast, pClaimInfo->strPatientLast, pClaimInfo->strPatientMiddle.Left(1) + CString("%%"));
									if (!rsPatIDByMid->eof) {
										if (rsPatIDByMid->GetRecordCount() > 1) {

											// (j.jones 2005-11-01 11:12) - PLID 17531 - if the BillID lookup preference is enabled,
											// see if the ID quasi-matches
											if (bIsUsingBillIDLookup) {

												Log("CalcInternalIDs: Too many records matched by first, last, and middle initial, and SearchByBillID is enabled, now matching by left of ID string.");

												long nSuggestedIDToUse = -1;
												long nCountFound = 0;

												while (!rsPatIDByMid->eof) {

													long nTestUserDefinedID = AdoFldLong(rsPatIDByMid, "UserDefinedID", -1);
													CString strTestUserDefinedID = AsString(nTestUserDefinedID);
													CString strReportedUserDefinedID = AsString(nReportedUserDefinedID);
													if (strReportedUserDefinedID.GetLength() > strTestUserDefinedID.GetLength()
														&& strReportedUserDefinedID.Left(strTestUserDefinedID.GetLength()) == strTestUserDefinedID) {

														//the name matches, and the EOB's ID begins with the patient's ID, so we are set
														nSuggestedIDToUse = AdoFldLong(rsPatIDByMid, "PersonID", -1);
														nCountFound++;
													}
													// (j.jones 2011-03-23 09:43) - PLID 42966 - fixed which recordset we called MoveNext() on
													rsPatIDByMid->MoveNext();
												}

												if (nCountFound == 1) {
													nPersonID = nSuggestedIDToUse;
													Log("CalcInternalIDs: Check by first/last/middle initial and left of ID string successful.");
												}
												else if (nCountFound > 1) {
													Log("CalcInternalIDs: Check by first/last/middle initial and left of ID string returned multiple records.");
												}
												else {
													Log("CalcInternalIDs: Check by first/last/middle initial and left of ID string returned no records.");
												}
											}

											if (nPersonID == -1)
												Log("CalcInternalIDs: Too many records matched by first, last, and middle initial.");
										}
										else {
											nPersonID = AdoFldLong(rsPatIDByMid, "PersonID", -1);
											Log("CalcInternalIDs: Check by first/last/middle initial successful.");
										}
									}
									else {
										//no match by first/last/middle initial
										Log("CalcInternalIDs: Failed to match any records by first/last/middle initial.");
									}
									rsPatIDByMid->Close();
								}
								else {
									Log("CalcInternalIDs: Failed to calculate Patient ID because the middle name is blank.");
								}
							}
						}
						else {
							//one person matched by first and last name, which is great, so use that ID
							nPersonID = AdoFldLong(rsPatID, "PersonID", -1);
							Log("CalcInternalIDs: Check by name successful.");
						}
					}
					else {
						//no match by first and last name
						Log("CalcInternalIDs: Failed to match any records by first and last name.");
					}
					rsPatID->Close();
				}
				else {
					Log("CalcInternalIDs: Failed to calculate Patient ID because either the first or last name is blank.");
				}
			}

			// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
			pClaimInfo->nUserDefinedID = nReportedUserDefinedID;
			pClaimInfo->nPatientID = nPersonID;
		}

		m_ptrProgressBar->StepIt();

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_ANSI835Parser();
	}

	//Step 2. Assign Insurance Co ID (uses Patient ID)
	CalcInsCoID(ptrEOBInfo);

	if (ptrEOBInfo->nLikelyInsuranceCoID == -1 && ptrEOBInfo->nHCFAGroupID != -1) {
		//set nLikelyInsuranceCoID to the first insurance company in the group

		// (j.jones 2011-06-28 16:16) - PLID 44363 - If we have a HCFA Group but no likely insurance company ID,
		// find the company that the most patients have, and use that. Even if each patient has only one unique
		// company, we would at least still pick one that actually was in use.
		CArray<long, long> aryPatientIDs;
		for (int i = 0; i<ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {
			const EOBClaimInfo *pClaim = ptrEOBInfo->arypEOBClaimInfo[i];
			if (pClaim->nPatientID != -1) {
				aryPatientIDs.Add(pClaim->nPatientID);
			}
		}
		if (aryPatientIDs.GetSize() > 0) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.InsuranceCoID "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"WHERE InsuranceCoT.HCFASetupGroupID = {INT} AND InsuredPartyT.PatientID IN ({INTARRAY}) "
				"GROUP BY InsuredPartyT.InsuranceCoID "
				"ORDER BY Count(*) DESC", ptrEOBInfo->nHCFAGroupID, aryPatientIDs);
			if (!rs->eof) {
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "InsuranceCoID", -1);
			}
			rs->Close();
		}

		// (j.jones 2011-06-28 16:20) - PLID 44363 - this should be much less likely to happen now,
		// but if we do get a -1 nLikelyInsuranceCoID, then continue selecting the first company
		if (ptrEOBInfo->nLikelyInsuranceCoID == -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 PersonID FROM InsuranceCoT "
				"WHERE HCFASetupGroupID = {INT} "
				"ORDER BY Name ASC", ptrEOBInfo->nHCFAGroupID);
			if (!rs->eof) {
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID", -1);
			}
			rs->Close();
		}
	}

	m_ptrProgressBar->SetPos(0);

	m_ptrProgressBar->SetRange(0, ptrEOBInfo->arypEOBClaimInfo.GetSize());

	if (m_CountOfEOBs == 1)
		str = "Assigning Bill IDs...";
	else
		str.Format("Assigning Bill IDs (EOB %li)...", ptrEOBInfo->nIndex + 1);

	m_ptrProgressStatus->SetWindowText(str);

	//Step 3. Assign Bill and Charge IDs (uses Patient ID and potentially Insurance Co ID)
	for (int i = 0; i < ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {

		EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[i];

		// (j.jones 2010-10-12 10:13) - PLID 40892 - we may have already figured out the bill ID,
		// if so, we can skip this function
		if (pClaimInfo->nBillID == -1) {
			CalcBillID(pClaimInfo, ptrEOBInfo->nHCFAGroupID);
		}

		// (j.jones 2013-07-19 08:57) - PLID 57616 - Track the bill's insured party ID and other insured party ID.
		// If we matched by a trace number, we should have already loaded these. If we matched through some other
		// reason, we will not have loaded these IDs yet, and need to do so now.
		if (pClaimInfo->nBillID != -1 && pClaimInfo->nBillInsuredPartyID == -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyID, OthrInsuredPartyID FROM BillsT WHERE ID = {INT}", pClaimInfo->nBillID);
			if (!rs->eof) {
				pClaimInfo->nBillInsuredPartyID = AdoFldLong(rs, "InsuredPartyID", -1);
				pClaimInfo->nBillOthrInsuredPartyID = AdoFldLong(rs, "OthrInsuredPartyID", -1);
			}
			rs->Close();
		}

		//loop through all charges
		for (int j = 0; j < pClaimInfo->arypEOBLineItemInfo.GetSize(); j++) {

			EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[j];

			// (j.jones 2011-02-09 13:38) - PLID 42391 - if a reversal, they give us the charge amount as a negative amount,
			// which is a stupid way of doing things, but out of our control
			COleCurrency cyChargeAmtToSearch = pLineItemInfo->cyLineItemChargeAmt;
			if (pClaimInfo->bIsReversedClaim && cyChargeAmtToSearch < COleCurrency(0, 0)) {
				//make it a positive charge amount so it will actually match our real charge total
				cyChargeAmtToSearch = -cyChargeAmtToSearch;
			}

			// (j.jones 2010-10-12 10:13) - PLID 40892 - we may have already figured out the charge ID,
			// if so, we can skip this code
			if (pLineItemInfo->nChargeID == -1) {

				//now figure out which charge this is

				// (j.dinatale 2012-04-03 12:31) - PLID 49386 - remove the filter on LineItemT.Amount
				CSqlFragment sqlChargeAmountFilter("AND dbo.GetChargeTotal(ChargesT.ID) = {OLECURRENCY}", cyChargeAmtToSearch);
				CSqlFragment sqlDateFilter;
				if (pLineItemInfo->bChargeDatePresent) {
					sqlDateFilter.Create("AND dbo.AsDateNoTime(ChargesT.ServiceDateFrom) = dbo.AsDateNoTime({OLEDATETIME})", pLineItemInfo->dtChargeDate);

					// (j.jones 2011-02-10 15:11) - PLID 42433 - if we have a date, and the charge amount
					// is reported as zero, then allow matching on just date, bill ID, and CPT code,
					// and permit the charge amount to not match
					if (cyChargeAmtToSearch == COleCurrency(0, 0)) {
						sqlChargeAmountFilter.Create("");
					}
				}

				// (j.jones 2008-06-18 14:22) - PLID 21921 - now we require the provider ID
				// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
				// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
				_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID, ChargesT.DoctorsProviders "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE BillID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND ItemCode = {STRING} "
					"{SQL} "
					"{SQL} "
					"ORDER BY ChargesT.ID ASC",
					pClaimInfo->nBillID, pLineItemInfo->strServiceID,
					sqlChargeAmountFilter, sqlDateFilter);
				long nLastChargeID = -1;
				long nLastProviderID = -1;
				while (!rsCharges->eof && pLineItemInfo->nChargeID == -1) {
					long nChargeID = AdoFldLong(rsCharges, "ID", -1);
					long nProviderID = AdoFldLong(rsCharges, "DoctorsProviders", -1);
					//save this ID incase we can't get one that hasn't already been used
					nLastChargeID = nChargeID;
					nLastProviderID = nProviderID;

					//see if we have already used this charge
					BOOL bUsed = FALSE;
					for (int y = 0; y<ptrEOBInfo->arypEOBClaimInfo.GetSize(); y++) {
						const EOBClaimInfo *pClaimInfo2 = ptrEOBInfo->arypEOBClaimInfo[y];
						//loop through all charges
						for (int z = 0; z<pClaimInfo2->arypEOBLineItemInfo.GetSize(); z++) {
							const EOBLineItemInfo *pLineItemInfo2 = pClaimInfo2->arypEOBLineItemInfo[z];
							if (pLineItemInfo2->nChargeID == nChargeID)
								bUsed = TRUE;
						}
					}

					//if not already used, take it!
					if (!bUsed) {
						pLineItemInfo->nChargeID = nChargeID;
						pLineItemInfo->nProviderID = nProviderID;
					}

					rsCharges->MoveNext();
				}

				//if we didn't get a unique one, use what was found anyways
				if (pLineItemInfo->nChargeID == -1) {
					pLineItemInfo->nChargeID = nLastChargeID;
					pLineItemInfo->nProviderID = nLastProviderID;
				}

				rsCharges->Close();
			}

			// (j.jones 2012-04-20 15:52) - We can't yet search for reversed payment/adjustment IDs,
			// because currently we do not know the insured party ID to use.

			// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
			// function, as we may have disabled this ability
			PeekAndPump_ANSI835Parser();
		}

		m_ptrProgressBar->StepIt();

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_ANSI835Parser();
	}
}

// (j.armen 2012-02-20 09:56) - PLID 48234 - Parameratized
void CANSI835Parser::CalcBillID(EOBClaimInfo *pClaimInfo, long nHCFAGroupID)
{
	// (j.jones 2010-10-12 10:13) - PLID 40892 - we may have already figured out the bill ID,
	// if so, we can skip this function
	if (pClaimInfo->nBillID != -1) {
		return;
	}

	CArray<long, long> arynStep1BillIDs;
	CArray<long, long> arynStep2BillIDs;

	CSqlFragment sqlDateFilter;

	//Step 1. Try to find all BillIDs with the date given (if one is given), PatientID, and exact charge total
	if (pClaimInfo->bBillDatePresent)
		sqlDateFilter.Create("AND dbo.AsDateNoTime(BillsT.Date) = dbo.AsDateNoTime({OLEDATETIME})", pClaimInfo->dtBillDate);

	// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
	// (j.jones 2014-01-15 11:03) - PLID 58872 - ordered by most recent bill date, then first bill ID on that date
	_RecordsetPtr rsBills = CreateParamRecordset(
		"SELECT BillsT.ID FROM BillsT "
		"WHERE BillsT.EntryType = 1 "
		"AND BillsT.Deleted = 0 "
		"AND BillsT.PatientID = {INT} "
		"{SQL} "
		"ORDER BY BillsT.Date DESC, BillsT.ID ASC",
		pClaimInfo->nPatientID, sqlDateFilter);
	while (!rsBills->eof) {
		long nBillID = AdoFldLong(rsBills, "ID", -1);
		COleCurrency cyBillTotal, cyPays, cyAdj, cyRef, cyIns;
		if (GetBillTotals(nBillID, pClaimInfo->nPatientID, &cyBillTotal, &cyPays, &cyAdj, &cyRef, &cyIns)) {
			if (cyBillTotal == pClaimInfo->cyClaimChargeAmt)
				arynStep1BillIDs.Add(nBillID);
		}
		rsBills->MoveNext();
	}
	rsBills->Close();

	//Step 2. Try to find all BillIDs with the CPT Codes, dates, and prices given, for that PatientID
	if (pClaimInfo->arypEOBLineItemInfo.GetSize() > 0) {

		CArray<long, long> arynBillIDs;

		//loop through all charges and put them into strChargeQuery
		for (int i = 0; i<pClaimInfo->arypEOBLineItemInfo.GetSize(); i++) {

			const EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[i];

			// (j.jones 2011-02-09 13:38) - PLID 42391 - if a reversal, they give us the charge amount as a negative amount,
			// which is a stupid way of doing things, but out of our control
			COleCurrency cyChargeAmtToSearch = pLineItemInfo->cyLineItemChargeAmt;
			if (pClaimInfo->bIsReversedClaim && cyChargeAmtToSearch < COleCurrency(0, 0)) {
				//make it a positive charge amount so it will actually match our real charge total
				cyChargeAmtToSearch = -cyChargeAmtToSearch;
			}


			if (pLineItemInfo->bChargeDatePresent)
				sqlDateFilter.Create("AND dbo.AsDateNoTime(ChargesT.ServiceDateFrom) = dbo.AsDateNoTime({OLEDATETIME})", pLineItemInfo->dtChargeDate);
			else
				sqlDateFilter.Create("");

			// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
			// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
			// (j.dinatale 2012-04-03 12:31) - PLID 49386 - remove the filter on LineItemT.Amount
			// (j.jones 2014-01-15 11:03) - PLID 58872 - ordered by most recent bill date, then first bill ID on that date
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT BillsT.ID AS BillID "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND dbo.GetChargeTotal(ChargesT.ID) = {OLECURRENCY} "
				"AND ItemCode = {STRING} AND LineItemT.PatientID = {INT} "
				"{SQL} "
				"GROUP BY BillsT.ID, BillsT.Date "
				"ORDER BY BillsT.Date DESC, BillsT.ID ASC",
				cyChargeAmtToSearch, pLineItemInfo->strServiceID,
				pClaimInfo->nPatientID, sqlDateFilter);
			if (!rs->eof){
				while (!rs->eof) {
					arynBillIDs.Add(AdoFldLong(rs, "BillID"));
					rs->MoveNext();
				}
			}
			else {
				//if empty, try without the charge date
				rs->Close();
				// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
				// (j.jones 2011-08-29 16:58) - PLID 44804 - ignore original & void charges
				// (j.dinatale 2012-04-03 12:31) - PLID 49386 - remove the filter on LineItemT.Amount
				// (j.jones 2014-01-15 11:03) - PLID 58872 - ordered by most recent bill date, then first bill ID on that date
				rs = CreateParamRecordset(
					"SELECT BillsT.ID AS BillID "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND dbo.GetChargeTotal(ChargesT.ID) = {OLECURRENCY} "
					"AND ItemCode = {STRING} AND LineItemT.PatientID = {INT} "
					"GROUP BY BillsT.ID, BillsT.Date "
					"ORDER BY BillsT.Date DESC, BillsT.ID ASC",
					cyChargeAmtToSearch, pLineItemInfo->strServiceID, pClaimInfo->nPatientID);
				while (!rs->eof) {
					arynBillIDs.Add(AdoFldLong(rs, "BillID"));
					rs->MoveNext();
				}
			}
			rs->Close();
		}

		// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
		// (j.jones 2014-01-15 11:03) - PLID 58872 - ordered by most recent bill date, then first bill ID on that date
		rsBills = CreateParamRecordset(
			"SELECT BillsT.ID "
			"FROM BillsT "
			"WHERE BillsT.EntryType = 1 "
			"	AND BillsT.Deleted = 0 "
			"	AND BillsT.PatientID = {INT} "
			"	{SQL} "
			"ORDER BY BillsT.Date DESC, BillsT.ID ASC",
			pClaimInfo->nPatientID,
			arynBillIDs.IsEmpty() ? CSqlFragment() : CSqlFragment("AND BillsT.ID IN ({INTARRAY})", arynBillIDs));

		while (!rsBills->eof) {
			long nBillID = AdoFldLong(rsBills, "ID", -1);
			if (nBillID != -1)
				arynStep2BillIDs.Add(nBillID);
			rsBills->MoveNext();
		}
		rsBills->Close();

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_ANSI835Parser();
	}

	//Step 3. Intelligently choose the right Bill ID
	//-With luck, each array will only have one Bill ID, and they will match.
	//-If either array has more than one ID, then we hope that they each only have one that matches.
	//-If there are multiple bills that match, find the one with a greater open balance and choose it,
	//while first checking to see if it is already in our EOB or not.
	//-If neither array as a bill that matches, filter down to only bills for that Insurance Company

	//both empty
	if (arynStep1BillIDs.GetSize() == 0 && arynStep2BillIDs.GetSize() == 0) {
		//this is beyond not cool
		pClaimInfo->nBillID = -1;

		Log("CalcBillID: Failed to calculate Bill ID for Patient ID (%li) because "
			"no bills matched the date and charge total, or CPT matches.", pClaimInfo->nPatientID);
		return;
	}
	//Step 1 returned 1, Step 2 returned 0
	if (arynStep1BillIDs.GetSize() == 1 && arynStep2BillIDs.GetSize() == 0) {
		//distressing, but not the end of the world
		pClaimInfo->nBillID = arynStep1BillIDs[0];

		Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
			"matching dates and charge totals, but no CPT matches.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
		return;
	}
	//Step 1 returned 0, Step 2 returned 1
	if (arynStep1BillIDs.GetSize() == 0 && arynStep2BillIDs.GetSize() == 1) {
		//this isn't so bad, as Step 2 is likely correct
		pClaimInfo->nBillID = arynStep2BillIDs[0];

		Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
			"matching CPT codes, but not dates and charge totals.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
		return;
	}

	//Step 1 returned 1, Step 2 returned 1
	if (arynStep1BillIDs.GetSize() == 1 && arynStep2BillIDs.GetSize() == 1) {
		const long& nBillID1 = arynStep1BillIDs[0];
		const long& nBillID2 = arynStep2BillIDs[0];
		if (nBillID1 == nBillID2) {
			//jackpot - this is be the ideal case
			pClaimInfo->nBillID = nBillID2;

			Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
				"matching, dates, charge totals, and CPT Codes.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
			return;
		}
	}

	CArray<long, long> arynStep3BillIDs;

	//at least one array returned multiple IDs, or both arrays returned one ID each that doesn't match
	if (arynStep1BillIDs.GetSize() > 0 && arynStep2BillIDs.GetSize() > 0) {

		for (int x = 0; x < arynStep1BillIDs.GetSize(); x++) {
			const long& nBillID1 = arynStep1BillIDs[x];
			for (int y = 0; y < arynStep2BillIDs.GetSize(); y++) {
				const long& nBillID2 = arynStep2BillIDs[y];
				if (nBillID1 == nBillID2) {
					//add to the array of matches
					bool bFound = false;
					for (int z = 0; z < arynStep3BillIDs.GetSize(); z++) {
						if (arynStep3BillIDs[z] == nBillID2) {
							bFound = true;
						}
					}
					if (!bFound) {
						arynStep3BillIDs.Add(nBillID2);
					}
				}
			}
		}
	}

	if (arynStep3BillIDs.GetSize() == 0) {
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		//if we get here, then we have no matching Bill IDs, so add all of them to the array
		for (int i = 0; i < arynStep1BillIDs.GetSize(); i++) {
			const long& nBillID = arynStep1BillIDs[i];
			//add to the array of matches
			bool bFound = false;
			for (int z = 0; z < arynStep3BillIDs.GetSize(); z++) {
				if (arynStep3BillIDs[z] == nBillID) {
					bFound = true;
				}
			}
			if (!bFound) {
				arynStep3BillIDs.Add(nBillID);
			}
		}

		for (int i = 0; i < arynStep2BillIDs.GetSize(); i++) {
			const long& nBillID = arynStep2BillIDs[i];
			//add to the array of matches
			bool bFound = false;
			for (int z = 0; z < arynStep3BillIDs.GetSize(); z++) {
				if (arynStep3BillIDs[z] == nBillID) {
					bFound = true;
				}
			}
			if (!bFound) {
				arynStep3BillIDs.Add(nBillID);
			}
		}
	}

	//now we have a list of potential matching bills, which we now need to trim down if they do not
	//have an insurance company in the group that we are using
	for (int i = arynStep3BillIDs.GetSize() - 1; i >= 0; i--) {
		const long& nBillID = arynStep3BillIDs[i];
		// (j.jones 2008-11-10 15:15) - PLID 31975 - we must filter on bills only!
		// (j.jones 2009-08-31 13:53) - PLID 34343 - actually filtered by bill ID, nice job old-me!
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.ID FROM BillsT "
			"LEFT JOIN InsuredPartyT PriInsuredPartyT ON BillsT.InsuredPartyID = PriInsuredPartyT.PersonID "
			"LEFT JOIN InsuredPartyT SecInsuredPartyT ON BillsT.InsuredPartyID = SecInsuredPartyT.PersonID "
			"WHERE BillsT.ID = {INT} "
			"AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND "
			"(PriInsuredPartyT.InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = {INT}) "
			"OR SecInsuredPartyT.InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = {INT}))",
			nBillID, nHCFAGroupID, nHCFAGroupID);
		if (rs->eof) {
			//this bill ID doesn't match this insurance group
			arynStep3BillIDs.RemoveAt(i);
		}
	}

	int size = arynStep3BillIDs.GetSize();
	if (size == 1) {
		//finally, we have a Bill ID
		pClaimInfo->nBillID = arynStep3BillIDs[0];

		Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
			"matching against HCFA Group (%li).", pClaimInfo->nBillID, pClaimInfo->nPatientID, nHCFAGroupID);
		return;
	}
	else if (size == 0) {
		//no bills with this insurance group???

		//if that's the case, try to use the first bill from the list, starting with Step 2
		if (arynStep2BillIDs.GetSize() > 0) {
			pClaimInfo->nBillID = arynStep2BillIDs[0];

			Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
				"taking first bill from CPT match list.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
		}
		else if (arynStep1BillIDs.GetSize() > 0) {
			pClaimInfo->nBillID = arynStep1BillIDs[0];

			Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
				"taking first bill from date/charge amount match list.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
		}

		return;

	}

	//we have multiple Bills in aryStep3BillIDs with this insurance group
	//we now must try to use these bills, in order, from Step 2, then Step 1

	// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
	for (int x = 0; x < arynStep3BillIDs.GetSize(); x++) {
		const long& nBillID3 = arynStep3BillIDs[x];
		for (int y = 0; y < arynStep2BillIDs.GetSize(); y++) {
			const long& nBillID2 = arynStep2BillIDs[y];
			if (nBillID3 == nBillID2) {
				//finally, for the love of god, return!
				pClaimInfo->nBillID = nBillID3;

				Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
					"comparing first matching bill between HCFA Groups and CPT matches.", pClaimInfo->nBillID, pClaimInfo->nPatientID);

				return;
			}
		}
	}
	for (int x = 0; x < arynStep3BillIDs.GetSize(); x++) {
		const long& nBillID3 = arynStep3BillIDs[x];
		for (int y = 0; y < arynStep1BillIDs.GetSize(); y++) {
			const long& nBillID1 = arynStep1BillIDs[y];
			if (nBillID3 == nBillID1) {
				//finally, for the love of god, return!
				pClaimInfo->nBillID = nBillID3;

				Log("CalcBillID: Calculated Bill ID (%li) for Patient ID (%li) by "
					"comparing first matching bill between HCFA Groups and date/charge amount matches.", pClaimInfo->nBillID, pClaimInfo->nPatientID);
				return;
			}
		}
	}

	//according to my calculations, and Visual Studio's compiling, it should be impossible to get here
	//if we manage to get here, face it, we have not a clue in the world what bill it is
	pClaimInfo->nBillID = -1;

	Log("CalcBillID: Could not find any Bill ID matches!");
	ASSERT(FALSE);
	return;
}

// (j.armen 2012-02-20 10:08) - PLID 48235 - Parameratized and commented out unused steps
void CANSI835Parser::CalcInsCoID(EOBInfo *ptrEOBInfo) {
	// (j.armen 2012-02-20 10:07) - PLID 48235 - Step 1 and 5 are not currently used
	//CArray<long, long> aryStep1InsCoNameMatches;
	CArray<long, long> aryStep2InsCoPatientMatches;
	CArray<long, long> aryStep3HCFAGroupPatientMatches;
	//CArray<long, long> aryStep5InsCoPayerIDMatches;

	// (j.jones 2010-07-21 10:19) - PLID 27142 - for Step 4, we need to create a count
	// of how many times a given company or group was used
	struct IDCount {
		long nInsuranceCoID;
		long nHCFASetupID;
		long nCount;
		IDCount(){};
		IDCount(long nInsuranceCoID, long nHCFASetupID, long nCount)
		{
			this->nInsuranceCoID = nInsuranceCoID;
			this->nHCFASetupID = nHCFASetupID;
			this->nCount = nCount;
		}
	};

	CArray<IDCount, IDCount> aryStep4InsCoPolicyNumberMatches;
	CArray<IDCount, IDCount> aryStep4HCFAGroupPolicyNumberMatches;

	bool bStep4FoundAllPolicyNumbers = false;

	m_ptrProgressBar->SetPos(0);

	m_ptrProgressBar->SetRange(0, ptrEOBInfo->arypEOBClaimInfo.GetSize() * 3);

	CString str;
	if (m_CountOfEOBs == 1)
		str = "Cross-referencing insured parties...";
	else
		str.Format("Cross-referencing insured parties (EOB %li)...", ptrEOBInfo->nIndex + 1);

	m_ptrProgressStatus->SetWindowText(str);

	// (j.jones 2005-04-27 16:11) - PLID 16324 - We need to be able to apply an EOB payment to multiple insurance companies
	// from the same HCFA group. This means we need to narrow down to one HCFA group, and then return a HCFAGroupID
	// and the ID of the most likely Insurance Company the payment is from. Later, on a per patient level,
	// we will more specifically find each individual insured party.

	// (j.armen 2012-02-16 16:57) - PLID 48235 - This step is not currently being used.  Don't even run the query
	//Step 1. Try to match up the Insurance Co Name with one in our database. We should be so lucky.
	//{
	//	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuranceCoT WHERE Name = {STRING}", ptrEOBInfo->strInsuranceCoName);

	//	Log("CalcInsCoID: Found %li insurance companies that match the name '%s'", prs->GetRecordCount(), ptrEOBInfo->strInsuranceCoName);

	//	while(!rs->eof) {
	//		aryStep1InsCoNameMatches.Add(AdoFldLong(rs, "PersonID"));
	//		rs->MoveNext();
	//	}
	//}

	//Step 2. For all the patients included in the remittance file, try to match up and see if one insurance company exists for all of them.
	{
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		for (int i = 0; i < ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {
			const EOBClaimInfo *pClaim = ptrEOBInfo->arypEOBClaimInfo[i];
			const long& nPatientID = pClaim->nPatientID;
			if (nPatientID != -1) {
				// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
				// (j.jones 2016-04-13 16:20) - NX-100184 - removed the resp. type exclusion filter
				_RecordsetPtr rs = CreateParamRecordset(
					"SELECT InsuranceCoID FROM InsuredPartyT "
					"WHERE PatientID = {INT}",
					nPatientID);
				CArray<long, long> aryPatientInsCoIDs;
				while (!rs->eof) {
					aryPatientInsCoIDs.Add(AdoFldLong(rs, "InsuranceCoID"));
					rs->MoveNext();
				}
				rs->Close();

				//now update the list of insurance companies for all patients
				if (i == 0) {
					//if the first iteration, add all the insurance companies for this patient
					aryStep2InsCoPatientMatches.Append(aryPatientInsCoIDs);
				}
				else {
					//not the first iteration, so what we need to do is clear out any items in
					//arydwInsCoIDs that are not in arydwPatientInsCoIDs
					for (int a = aryStep2InsCoPatientMatches.GetSize() - 1; a >= 0; a--) {
						bool bFound = false;
						for (int b = 0; b < aryPatientInsCoIDs.GetSize(); b++) {
							if (aryStep2InsCoPatientMatches[a] == aryPatientInsCoIDs[b])
								bFound = true;
						}
						if (!bFound) {
							//if not found, remove
							aryStep2InsCoPatientMatches.RemoveAt(a);
						}
					}
				}
			}
			m_ptrProgressBar->StepIt();
		}
		Log("CalcInsCoID: Found %li insurance companies that every patient has.", aryStep2InsCoPatientMatches.GetSize());

		// (j.jones 2010-07-21 11:26) - DO NOT assume that because every patient has the same company, that it's the right one.
		// move this logic above the policy number checks. Why? Because imagine a small EOB with
		// five patients who have Medicare primary and an assortment of BCBS companies as secondary. If a BCBS check came in, we would
		// auto-post as Medicare if matching by same-company was the highest priority. This is why we match by insurance IDs first.
	}

	//Step 3. For all the patients included in the remittance file, try to match up and see if one insurance group exists for all of them.
	{
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		for (int i = 0; i < ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {
			const EOBClaimInfo *pClaim = ptrEOBInfo->arypEOBClaimInfo[i];
			const long& nPatientID = pClaim->nPatientID;
			if (nPatientID != -1) {
				// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
				// (j.jones 2016-04-13 16:20) - NX-100184 - removed the resp. type exclusion filter
				_RecordsetPtr rs = CreateParamRecordset(
					"SELECT HCFASetupGroupID FROM InsuranceCoT "
					"WHERE PersonID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PatientID = {INT})",
					nPatientID);
				CArray<long, long> aryPatientHCFAGroupIDs;
				while (!rs->eof) {
					aryPatientHCFAGroupIDs.Add(AdoFldLong(rs, "HCFASetupGroupID", -1));
					rs->MoveNext();
				}

				//now update the list of HCFA groups for all patients
				if (i == 0) {
					//if the first iteration, add all the HCFA Groups for this patient
					aryStep3HCFAGroupPatientMatches.Append(aryPatientHCFAGroupIDs);
				}
				else {
					//not the first iteration, so what we need to do is clear out any items in
					//arydwHCFAGroupIDs that are not in arydwPatientInsCoIDs
					for (int a = aryStep3HCFAGroupPatientMatches.GetSize() - 1; a >= 0; a--) {
						bool bFound = false;
						for (int b = 0; b<aryPatientHCFAGroupIDs.GetSize(); b++) {
							if (aryStep3HCFAGroupPatientMatches[a] == aryPatientHCFAGroupIDs[b])
								bFound = true;
						}
						if (!bFound) {
							//if not found, remove
							aryStep3HCFAGroupPatientMatches.RemoveAt(a);
						}
					}
				}
			}
			m_ptrProgressBar->StepIt();
		}
		Log("CalcInsCoID: Found %li insurance groups that every patient has.", aryStep3HCFAGroupPatientMatches.GetSize());
	}

	//Step 4. Start confirming based on the patient's insurance ID.
	{
		//set to TRUE, we will revert to false if any record isn't found
		bStep4FoundAllPolicyNumbers = true;

		for (int i = 0; i < ptrEOBInfo->arypEOBClaimInfo.GetSize(); i++) {
			const EOBClaimInfo *pClaim = ptrEOBInfo->arypEOBClaimInfo[i];
			const long& nPatientID = pClaim->nPatientID;
			if (nPatientID != -1) {
				// (j.jones 2011-03-16 16:32) - PLID 42866 - we now track an original and corrected ID
				CString strOriginalInsuranceID = pClaim->strOriginalPatientInsuranceID;
				CString strCorrectedInsuranceID = pClaim->strCorrectedPatientInsuranceID;
				CString strGroupNum = pClaim->strGroupOrPolicyNum;

				// (j.jones 2011-03-16 16:34) - PLID 42866 - eliminate confusion if the corrected ID
				// is an ID we already have
				if (strCorrectedInsuranceID == strGroupNum || strCorrectedInsuranceID == strOriginalInsuranceID) {
					strCorrectedInsuranceID = "";
				}

				if (strOriginalInsuranceID.IsEmpty() && !strCorrectedInsuranceID.IsEmpty()) {
					strOriginalInsuranceID = strCorrectedInsuranceID;
					strCorrectedInsuranceID = "";
				}

				if (strOriginalInsuranceID.IsEmpty() && !strGroupNum.IsEmpty()) {
					strOriginalInsuranceID = strGroupNum;
					strGroupNum = "";
				}

				// (j.jones 2011-03-16 16:34) - PLID 42866 - eliminate confusion if the corrected ID
				// is an ID we already have
				if (strCorrectedInsuranceID == strGroupNum || strCorrectedInsuranceID == strOriginalInsuranceID) {
					strCorrectedInsuranceID = "";
				}

				if (strOriginalInsuranceID.GetLength() > 0) {
					CStringArray arystrInsuranceIDs;

					//also search for the group number
					if (!strGroupNum.IsEmpty()) {
						arystrInsuranceIDs.Add(strGroupNum);
					}

					// (j.jones 2011-03-16 16:36) - PLID 42866 - also search for strCorrectedInsuranceID
					if (!strCorrectedInsuranceID.IsEmpty()) {
						if (strCorrectedInsuranceID.Find(" ") != -1) {

							CString strInsuranceID1, strInsuranceID2;
							strInsuranceID1 = strCorrectedInsuranceID.Left(strCorrectedInsuranceID.Find(" "));
							strInsuranceID1.TrimRight();
							strInsuranceID2 = strCorrectedInsuranceID.Right(strCorrectedInsuranceID.GetLength() - strCorrectedInsuranceID.Find(" "));
							strInsuranceID2.TrimLeft();

							arystrInsuranceIDs.Add(strCorrectedInsuranceID);
							arystrInsuranceIDs.Add(strInsuranceID1);
							arystrInsuranceIDs.Add(strInsuranceID2);
						}
						else {
							arystrInsuranceIDs.Add(strCorrectedInsuranceID);
						}
					}

					if (strOriginalInsuranceID.Find(" ") != -1) {
						// (j.jones 2005-04-15 14:23) - we could potentially have multiple IDs to test with,
						// we should test with the whole string and with the string broken at the space
						// incase they sent both IDs on one line.
						CString strInsuranceID1, strInsuranceID2;
						strInsuranceID1 = strOriginalInsuranceID.Left(strOriginalInsuranceID.Find(" "));
						strInsuranceID1.TrimRight();
						strInsuranceID2 = strOriginalInsuranceID.Right(strOriginalInsuranceID.GetLength() - strOriginalInsuranceID.Find(" "));
						strInsuranceID2.TrimLeft();

						arystrInsuranceIDs.Add(strOriginalInsuranceID);
						arystrInsuranceIDs.Add(strInsuranceID1);
						arystrInsuranceIDs.Add(strInsuranceID2);
					}
					else {
						// (j.jones 2005-04-15 14:23) - we just have one ID to test with
						arystrInsuranceIDs.Add(strOriginalInsuranceID);
					}

					// (j.jones 2013-07-10 10:29) - PLID 57263 - include the resp. type exclusion filter
					// (j.jones 2016-04-13 16:20) - NX-100184 - removed the resp. type exclusion filter
					_RecordsetPtr rs = CreateParamRecordset(
						"SELECT InsuranceCoID, HCFASetupGroupID FROM InsuredPartyT "
						"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						"WHERE PatientID = {INT} AND (IDForInsurance IN ({STRINGARRAY}) OR PolicyGroupNum IN ({STRINGARRAY}))",
						nPatientID,
						arystrInsuranceIDs, arystrInsuranceIDs);
					if (rs->eof) {
						bStep4FoundAllPolicyNumbers = false;
					}
					else {
						while (!rs->eof) {
							CString str;
							long nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID");
							long nHCFASetupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

							// (j.jones 2010-07-21 10:21) - PLID 27142 - track counts of each ID
							bool bFound = false;
							for (int i = 0; i < aryStep4InsCoPolicyNumberMatches.GetSize() && !bFound; i++) {
								if (aryStep4InsCoPolicyNumberMatches[i].nInsuranceCoID == nInsuranceCoID) {
									bFound = true;
									aryStep4InsCoPolicyNumberMatches[i].nCount++;
								}
							}
							if (!bFound) {
								aryStep4InsCoPolicyNumberMatches.Add(IDCount(nInsuranceCoID, nHCFASetupID, 1));
							}

							bFound = false;
							for (int i = 0; i<aryStep4HCFAGroupPolicyNumberMatches.GetSize() && !bFound; i++) {
								if (aryStep4HCFAGroupPolicyNumberMatches[i].nHCFASetupID == nHCFASetupID) {
									bFound = true;
									aryStep4HCFAGroupPolicyNumberMatches[i].nCount++;
								}
							}
							if (!bFound) {
								aryStep4HCFAGroupPolicyNumberMatches.Add(IDCount(-1, nHCFASetupID, 1));
							}
							rs->MoveNext();
						}
					}
					rs->Close();
				}
				else {
					bStep4FoundAllPolicyNumbers = false;
				}
			}
			else {
				bStep4FoundAllPolicyNumbers = false;
			}

			m_ptrProgressBar->StepIt();
		}

		if (bStep4FoundAllPolicyNumbers) {
			Log("CalcInsCoID: Found a matching insurance company or group for all policy numbers.");
		}
		else {
			Log("CalcInsCoID: Could not find a matching insurance company or group for all policy numbers.");
		}
	}

	// (j.armen 2012-02-20 10:09) - PLID 48235 - This step is not used. Do not run the query
	//Step 5. Using the payer ID sent back, try to find the InsCo (or multiple InsCos) that use it
	//{
	//	if(!ptrEOBInfo->strPayerID.IsEmpty()) {
	//		// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
	//		// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure, incase we ever commented this code in
	//		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuranceCoT "
	//			"WHERE InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
	//			"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
	//			"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
	//			"	OR InsuranceCoT.PersonID IN ("
	//			"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
	//			"		FROM InsuranceLocationPayerIDsT "
	//			"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
	//			"		WHERE EbillingInsCoIDs.EbillingID = {STRING})",
	//			ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID);
	//
	//		Log("CalcInsCoID: Found %li insurance companies that match payer ID '%s'", rs->GetRecordCount(), ptrEOBInfo->strPayerID);
	//
	//		while(!rs->eof) {
	//			aryStep5InsCoPayerIDMatches.Add(AdoFldLong(rs, "PersonID"));
	//			prs->MoveNext();
	//		}
	//	}
	//}

	///END GENERATING SUSPECTED COMPANIES
	/////////////////////////////////////

	//at this point we have all our lists of possible Insurance Companies and HCFA Groups to choose from

	/////////////////////////////////////
	///BEGIN INSURANCE ID DECISIONS	

	//right now we have the following: 

	//aryStep1InsCoNameMatches - a list of Insurance Company IDs that match the name of the InsCo on this EOB - Not Used
	//aryStep2InsCoPatientMatches - a list of Insurance Company IDs that every patient on this EOB has
	//aryStep3HCFAGroupPatientMatches - a list of HCFA Groups that every patient on this EOB has
	//aryStep4InsCoPolicyNumberMatches - a list of Insurance Company IDs that every patient on this EOB has
	//aryStep4HCFAGroupPolicyNumberMatches - a list of HCFA Groups that every patient on this EOB has
	//aryStep5InsCoPayerIDMatches - a list of Insurance Company IDs that match the payer ID on this EOB - Not Used

	//try to deduce the proper insurance company / HCFA group in order of best case -> worst case

	//Step 4. the best cases are if all the patients match one insco or one HCFA group by policy number
	if (bStep4FoundAllPolicyNumbers) {

		//first check based on insurance company

		// (j.jones 2010-07-21 10:21) - PLID 27142 - we now track counts of each ID, so find the most-used company
		long nLikelyInsuranceCoID = -1;
		long nLikelyHCFAGroupID = -1;
		long nLikelyInsuranceCoCount = 0;
		for (int i = 0; i < aryStep4InsCoPolicyNumberMatches.GetSize(); i++) {
			const IDCount& icInfo = aryStep4InsCoPolicyNumberMatches[i];
			if (icInfo.nCount > nLikelyInsuranceCoCount) {
				nLikelyInsuranceCoID = icInfo.nInsuranceCoID;
				nLikelyHCFAGroupID = icInfo.nHCFASetupID;
				nLikelyInsuranceCoCount = icInfo.nCount;
			}
		}

		if (nLikelyInsuranceCoID != -1) {

			ptrEOBInfo->nLikelyInsuranceCoID = nLikelyInsuranceCoID;
			ptrEOBInfo->nHCFAGroupID = nLikelyHCFAGroupID;

			// (j.jones 2010-07-21 10:30) - PLID 27142 - log appropriately, and call NarrowInsuranceSelection if needed
			if (aryStep4InsCoPolicyNumberMatches.GetSize() == 1) {
				//most ideal case - we know that each patient has an insured party for only one insurance company
				//that matches each policy number returned. This case is only likely in a "perfect" Medicare file.

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all policy numbers to one insurance company. (Step 4a)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
			}
			else {
				//also an ideal case - this means we matched up an Insured Party to every policy number returned,
				//but it is more than one insurance company. This is also the ideal case for BCBS files,
				//provided that aryStep4HCFAGroupPolicyNumberMatches had only one entry.

				//double check we only have one HCFA Group, if not try to narrow the selection
				if (aryStep4HCFAGroupPolicyNumberMatches.GetSize() > 1) {

					CArray<long, long> arynStep4HCFAGroupPolicyNumberMatches;
					for (int i = 0; i < aryStep4HCFAGroupPolicyNumberMatches.GetSize(); i++) {
						arynStep4HCFAGroupPolicyNumberMatches.Add(aryStep4HCFAGroupPolicyNumberMatches[i].nHCFASetupID);
					}

					CSqlFragment sql("SELECT ID FROM HCFASetupT WHERE ID IN ({INTARRAY})", arynStep4HCFAGroupPolicyNumberMatches);
					_RecordsetPtr rs2 = CreateParamRecordset(sql);
					if (!rs2->eof) {
						if (rs2->GetRecordCount() > 1) {
							//more than one HCFA group, so attempt to narrow the selection by payer ID or payer name
							if (NarrowInsuranceSelection(ptrEOBInfo, sql, "matching all policy numbers to multiple insurance companies. (Step 4b)"))
								return;
						}
					}
					rs2->Close();
				}

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all policy numbers to multiple insurance companies. (Step 4b)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
				return;
			}

			return;
		}

		//next check based on HCFA Group
		//**note, this SHOULD have already been caught in the else statement above

		// (j.jones 2010-07-21 10:21) - PLID 27142 - we now track counts of each ID, so find the most-used group
		nLikelyHCFAGroupID = -1;
		long nLikelyHCFAGroupCount = 0;
		for (int i = 0; i < aryStep4HCFAGroupPolicyNumberMatches.GetSize(); i++) {
			const IDCount& icInfo = aryStep4HCFAGroupPolicyNumberMatches[i];
			if (icInfo.nCount > nLikelyHCFAGroupCount) {
				nLikelyHCFAGroupID = icInfo.nHCFASetupID;
				nLikelyHCFAGroupCount = icInfo.nCount;
			}
		}

		if (nLikelyHCFAGroupID != -1) {

			ptrEOBInfo->nLikelyInsuranceCoID = -1;
			ptrEOBInfo->nHCFAGroupID = nLikelyHCFAGroupID;

			// (j.jones 2010-07-21 10:30) - PLID 27142 - log appropriately, and call NarrowInsuranceSelection if needed
			if (aryStep4HCFAGroupPolicyNumberMatches.GetSize() == 1) {
				//also an ideal case - this means we matched up an Insured Party to every policy number returned,
				//for multiple insurance companies in one HCFA Group. This is the ideal case for BCBS files
				Log("CalcInsCoID: Calculated HCFA Group ID (%li) by "
					"matching all policy numbers to one HCFA Group. (Step 4c)", ptrEOBInfo->nHCFAGroupID);
				return;
			}
			else {
				//This is a problem, most likely in the setup of HCFA Groups - it means we found matching
				//insured party for each policy number returned, but they are for insurance companies in multiple HCFA
				//groups. We will try to find the right group anyways when narrowing down insured parties.

				CArray<long, long> arynStep4HCFAGroupPolicyNumberMatches;
				for (int i = 0; i < aryStep4HCFAGroupPolicyNumberMatches.GetSize(); i++) {
					arynStep4HCFAGroupPolicyNumberMatches.Add(aryStep4HCFAGroupPolicyNumberMatches[i].nHCFASetupID);
				}

				CSqlFragment sql("SELECT ID FROM HCFASetupT WHERE ID IN ({INTARRAY})", arynStep4HCFAGroupPolicyNumberMatches);
				_RecordsetPtr rs2 = CreateParamRecordset(sql);
				if (!rs2->eof) {
					if (rs2->GetRecordCount() > 1) {

						//attempt to narrow the selection by payer ID or payer name
						if (NarrowInsuranceSelection(ptrEOBInfo, sql, "matching all policy numbers to multiple HCFA Groups. (Step 4d)"))
							return;
					}
				}

				Log("CalcInsCoID: Calculated HCFA Group ID (%li) by "
					"matching all policy numbers to multiple HCFA Groups. (Step 4d)", ptrEOBInfo->nHCFAGroupID);

				return;
			}
		}
	}
	//end policy number check

	//if we get this far it means we did not match every policy number to an insured party, which, sadly, is going to be pretty common

	long nStep2LikelyInsuranceCoID = -1;
	long nStep2HCFAGroupID = -1;

	// (j.jones 2010-07-21 11:26) - DO NOT move this logic above the policy number checks. Why? Because imagine a small EOB with
	// five patients who have Medicare primary and an assortment of BCBS companies as secondary. If a BCBS check came in, we would
	// auto-post as Medicare if matching by same-company was the highest priority. This is why we match by insurance IDs first.

	//Step 2. the next most reliable check is to try and see if each patient has the same insurance company
	_RecordsetPtr rs = CreateParamRecordset(
		"SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT "
		"WHERE PersonID IN ({INTARRAY})",
		aryStep2InsCoPatientMatches);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//this is an ideal case - we know that each patient has an insured party for only one insurance company overall
			nStep2LikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			nStep2HCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"matching all patients to one insurance company. (Step 2a)", nStep2LikelyInsuranceCoID, nStep2HCFAGroupID);

			//WAIT: do not return yet
			//return;
		}
		else {
			//otherwise, this tells us nothing

			//don't narrow further here, instead use Step 3 (HCFA Groups)
		}
	}
	rs->Close();

	//Step 3. the next most reliable check is to try and see if each patient has the same HCFA Group
	CSqlFragment sql("SELECT ID FROM HCFASetupT WHERE ID IN ({INTARRAY})", aryStep3HCFAGroupPatientMatches);
	rs = CreateParamRecordset(sql);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//also an ideal case - this means we matched up an Insured Party to every policy number returned,
			//for multiple insurance companies in one HCFA Group. This is the ideal case for BCBS files
			ptrEOBInfo->nLikelyInsuranceCoID = -1;
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "ID");

			//use the insurance co ID from Step 2
			if (ptrEOBInfo->nHCFAGroupID == nStep2HCFAGroupID) {
				ptrEOBInfo->nLikelyInsuranceCoID = nStep2LikelyInsuranceCoID;
			}
			else {
				//should be impossible
				Log("CalcInsCoID: Step 2 Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all patients to one insurance company, but Step 3 calculated HCFA Group ID (%li) by "
					"matching all patients to one differing HCFA Group.", nStep2LikelyInsuranceCoID, nStep2HCFAGroupID, ptrEOBInfo->nHCFAGroupID);
			}

			Log("CalcInsCoID: Calculated HCFA Group ID (%li) by "
				"matching all patients to one HCFA Group. (Step 3)", ptrEOBInfo->nHCFAGroupID);
			return;
		}
		else {
			//attempt to narrow the selection by payer ID or payer name
			if (NarrowInsuranceSelection(ptrEOBInfo, sql, "matching all patients to one HCFA Group. (Step 3)"))
				return;
		}
	}

	//Step 1. Try to find a matching Insurance Company (and its group) by Insurance Company name
	//try to compare using Step 2 or Step 3
	sql.Create("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT WHERE Name = {STRING} AND PersonID IN ({INTARRAY})", ptrEOBInfo->strInsuranceCoName, aryStep2InsCoPatientMatches);
	rs = CreateParamRecordset(sql);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//this is an ideal case - we know that each patient has an insured party for only one insurance company overall
			//that also matches the name returned by the EOB
			ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"matching all patients to one insurance company by company name. (Step 1a)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
			return;
		}
		else {
			//otherwise, this means that every patient has the same combination of multiple insurance companies
			//that also have the same name but are not the same insurance company record
			//this is semi-plausible if there are only one or two patients in the system with let's say "BCBS"
			//as primary and secondary, but they are not the same BCBS record. If that's the case, 
			//assume they are in the same HCFA Group and return that group (we assume that only one group would match)
			ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			//attempt to narrow the selection by payer ID
			if (NarrowInsuranceSelection(ptrEOBInfo, sql, "matching all patients to one insurance company by company name. (Step 1a)"))
				return;

			Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"matching all patients to multiple insurance companies by company name. (Step 1b)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
			return;
		}
	}
	rs->Close();

	//try again with the HCFA Group
	rs = CreateParamRecordset("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT WHERE Name = {STRING} AND HCFASetupGroupID IN ({INTARRAY})", ptrEOBInfo->strInsuranceCoName, aryStep3HCFAGroupPatientMatches);
	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//not ideal but highly possible - we know that each patient has an insured party for only one insurance group overall
			//that has an insurance company with the name returned by the EOB
			//this could happen if they get a BCBS claim and there are multiple BCBS companies under one group
			//**note, this SHOULD have already been caught in the else statement above
			ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"matching all patients to one HCFA Group by company name. (Step 1c)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
			return;
		}
		else {
			//otherwise, this means that every patient has the same combination of multiple insurance groups
			//that each have companies with the same name but are not the same insurance company record
			//this is ridiculously unlikely, I can't fathom how it could possibly happen.
			//since we only have one resort left, try further filtering by payer ID to narrow it down

			//first grab the current values incase our next test is EOF
			ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			rs->Close();
			// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
			// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
			rs = CreateParamRecordset(
				"SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT "
				"WHERE Name = {STRING} AND HCFASetupGroupID IN ({INTARRAY}) "
				"AND (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
				"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
				"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
				"	OR InsuranceCoT.PersonID IN ("
				"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
				"		FROM InsuranceLocationPayerIDsT "
				"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
				"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
				")",
				ptrEOBInfo->strInsuranceCoName, aryStep3HCFAGroupPatientMatches, ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID);

			if (!rs->eof) {
				if (rs->GetRecordCount() == 1) {
					//alright, we filtered one matching record by PayerID, thankfully
					ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
					ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

					Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
						"matching all patients to multiple HCFA Groups by company name, one group by company Payer ID. (Step 1d)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
					return;
				}
				else {
					//something is seriously wrong with their HCFA Group setup if they get here,
					//so just return what we can and let the insured-party finder try its hand later
					ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
					ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

					Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
						"matching all patients to multiple HCFA Groups by company name, and multiple groups by company Payer ID. (Step 1e)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
					return;
				}
			}
			//if we get here, we couldn't further filter by payer ID

			Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"matching all patients to multiple HCFA Groups by company name. (Step 1f)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
			return;
		}
	}
	rs->Close();


	//Step 5. Try to find a matching Insurance Company (and its group) by Payer ID
	//try to compare using Step 2 or Step 3

	if (!ptrEOBInfo->strPayerID.IsEmpty()) {
		// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
		// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
		rs = CreateParamRecordset(
			"SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT "
			"WHERE (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
			"	OR InsuranceCoT.PersonID IN ("
			"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
			"		FROM InsuranceLocationPayerIDsT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING})) "
			"AND PersonID IN ({INTARRAY})", ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID, aryStep2InsCoPatientMatches);
		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//this would be an ideal case as it means we know that each patient 
				//has an insured party for only one insurance company overall that also
				//matches the payer ID returned by the EOB
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
				ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all patients to one insurance company by Payer ID. (Step 5a)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
				return;
			}
			else {
				//otherwise, this means that every patient has the same combination of multiple insurance companies
				//that also have the same payer ID but are not the same insurance company record
				//this is semi-plausible if there are only one or two patients in the system with a BCBS company
				//as primary and secondary, but they are not the same BCBS record. If that's the case, 
				//assume they are in the same HCFA Group and return that group (we assume that only one group would match)
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
				ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all patients to multiple insurance companies by Payer ID. (Step 5b)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
				return;
			}
		}

		//try again with the HCFA Group
		// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
		// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
		rs = CreateParamRecordset(
			"SELECT PersonID, HCFASetupGroupID "
			"FROM InsuranceCoT "
			"WHERE (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
			"	OR InsuranceCoT.PersonID IN ("
			"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
			"		FROM InsuranceLocationPayerIDsT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING})) "
			"AND HCFASetupGroupID IN ({INTARRAY})", ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID, aryStep3HCFAGroupPatientMatches);
		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//not ideal but highly possible - we know that each patient has an insured party for only one insurance group overall
				//that has a payer ID that matches the one returned by the EOB
				//this could happen if they get a BCBS claim and there are multiple BCBS companies under one group
				//**note, this SHOULD have already been caught in the else statement above
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
				ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all patients to one HCFA Group by company Payer ID. (Step 5c)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
				return;
			}
			else {
				//otherwise, this means that every patient has the same combination of multiple insurance groups
				//that each have companies with the same payer ID but are not the same insurance company record
				//this is quite unlikely, I suppose it's plausible through some messed-up payer ID selections
				//and multiple commercial groups... maybe?
				//so just return what we can and let the insured-party finder try its hand later
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
				ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

				Log("CalcInsCoID: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"matching all patients to multiple HCFA Groups by company Payer ID. (Step 5c)", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID);
				return;
			}
		}
	}

	//if we get here we:
	//- couldn't reliably match by a policy number for all patients
	//- couldn't reliably match by all patients by either same insurance company or same insurance group
	//- couldn't reliably match by insurance company name
	//- couldn't reliably match by payer ID

	ptrEOBInfo->nLikelyInsuranceCoID = -1;
	ptrEOBInfo->nHCFAGroupID = -1;

	Log("CalcInsCoID: Could not determine any insurance company or HCFA Group!");
	//ASSERT(FALSE);
	return;
}

//DRT 4/10/2007 - PLID 25564 - Removed PeekAndPump in favor of a global version.

EOBInfo* CANSI835Parser::GetCurrentEOBInfo()
{
	ASSERT(m_arypEOBInfo.GetSize() != 0);

	return m_arypEOBInfo[m_arypEOBInfo.GetSize() - 1];
}

// (j.dinatale 2012-11-05 17:32) - PLID 50792 - need the last claim info
EOBClaimInfo* CANSI835Parser::GetLastClaimInfo()
{
	EOBInfo *pCurrentEOBInfo = GetCurrentEOBInfo();
	ASSERT(pCurrentEOBInfo->arypEOBClaimInfo.GetSize() != 0);
	return pCurrentEOBInfo->arypEOBClaimInfo[pCurrentEOBInfo->arypEOBClaimInfo.GetSize() - 1];
}

//this function is used when you have multiple insurance results and need to narrow down by payer ID or InsCoName
// (j.armen 2012-02-20 10:16) - PLID 48237 - Parameratized
BOOL CANSI835Parser::NarrowInsuranceSelection(EOBInfo *ptrEOBInfo, CSqlFragment sqlLastRecordsetSql, CString strPendingLog)
{
	if (sqlLastRecordsetSql->m_strSql.Find("SELECT ID FROM HCFASetupT") != -1) {
		sqlLastRecordsetSql->m_strSql.Replace("SELECT ID FROM HCFASetupT", "SELECT ID AS HCFASetupGroupID FROM HCFASetupT");
	}
	else if (sqlLastRecordsetSql->m_strSql.Find("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT") != -1) {
		sqlLastRecordsetSql->m_strSql.Replace("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT", "SELECT HCFASetupGroupID FROM InsuranceCoT");
	}

	//first go by insco name
	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT WHERE Name = {STRING} "
		"AND HCFASetupGroupID IN ({SQL})", ptrEOBInfo->strInsuranceCoName, sqlLastRecordsetSql);

	if (!rs->eof) {
		if (rs->GetRecordCount() == 1) {
			//we had one record, so our filtering worked!

			//we now have one insurance company that matches our previous filtering, plus payer name
			ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
			ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

			Log("CalcInsCoID:NarrowInsuranceSelection: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
				"%s and then matching by payer name %s", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID, strPendingLog, ptrEOBInfo->strInsuranceCoName);

			return TRUE;
		}
		else {

			//we had more than one record - but is it more than one HCFA Group?

			//double check we only have one HCFA Group
			_RecordsetPtr rs2 = CreateParamRecordset("SELECT ID FROM HCFASetupT WHERE ID IN "
				"(SELECT HCFASetupGroupID FROM InsuranceCoT WHERE Name = {STRING} AND HCFASetupGroupID IN ({SQL}))", ptrEOBInfo->strInsuranceCoName, sqlLastRecordsetSql);
			if (!rs2->eof) {
				if (rs2->GetRecordCount() == 1) {
					//ah, only one HCFA group, that's good, so let's return

					ptrEOBInfo->nLikelyInsuranceCoID = -1;
					ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs2, "ID", -1);

					Log("CalcInsCoID:NarrowInsuranceSelection: Calculated HCFA Group ID (%li) by "
						"%s and then matching one HCFA Group by payer name %s", ptrEOBInfo->nHCFAGroupID, strPendingLog, ptrEOBInfo->strInsuranceCoName);

					return TRUE;
				}
				else {
					//if more than one HCFA Group, aside from being bizarre, we need to filter on Payer ID

					if (!ptrEOBInfo->strPayerID.IsEmpty()) {
						// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
						// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
						_RecordsetPtr rs3 = CreateParamRecordset("SELECT ID FROM HCFASetupT WHERE ID IN "
							"(SELECT HCFASetupGroupID FROM InsuranceCoT WHERE Name = {STRING} "
							"AND (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
							"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
							"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
							"	OR InsuranceCoT.PersonID IN ("
							"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
							"		FROM InsuranceLocationPayerIDsT "
							"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
							"		WHERE EbillingInsCoIDs.EbillingID = {STRING})) "
							"AND HCFASetupGroupID IN ({SQL}))",
							ptrEOBInfo->strInsuranceCoName, ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID, sqlLastRecordsetSql);
						if (!rs3->eof) {
							if (rs3->GetRecordCount() == 1) {
								//ah, only one HCFA group, that's good, so let's return

								ptrEOBInfo->nLikelyInsuranceCoID = -1;
								ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs3, "ID", -1);

								Log("CalcInsCoID:NarrowInsuranceSelection: Calculated HCFA Group ID (%li) by "
									"%s and then matching one HCFA Group by payer name %s and payer ID %s", ptrEOBInfo->nHCFAGroupID, strPendingLog, ptrEOBInfo->strInsuranceCoName, ptrEOBInfo->strPayerID);

								return TRUE;
							}
							else {
								//if more than one HCFA Group, we have no checks left to do
								ASSERT(FALSE);
							}
						}
						rs3->Close();
					}
				}
			}
			rs2->Close();

		}
	}
	rs->Close();

	//if we got here, matching on name failed, so match by payer ID
	if (!ptrEOBInfo->strPayerID.IsEmpty()) {

		// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
		// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
		rs = CreateParamRecordset("SELECT PersonID, HCFASetupGroupID FROM InsuranceCoT "
			"WHERE (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
			"	OR InsuranceCoT.PersonID IN ("
			"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
			"		FROM InsuranceLocationPayerIDsT "
			"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
			"		WHERE EbillingInsCoIDs.EbillingID = {STRING})) "
			"AND HCFASetupGroupID IN ({SQL})",
			ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID, sqlLastRecordsetSql);

		if (!rs->eof) {
			if (rs->GetRecordCount() == 1) {
				//we had one record, so our filtering worked!

				//we now have one insurance company that matches our previous filtering, plus payer ID
				ptrEOBInfo->nLikelyInsuranceCoID = AdoFldLong(rs, "PersonID");
				ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);

				Log("CalcInsCoID:NarrowInsuranceSelection: Calculated Insurance Co ID (%li) and HCFA Group ID (%li) by "
					"%s and then matching by payer ID %s", ptrEOBInfo->nLikelyInsuranceCoID, ptrEOBInfo->nHCFAGroupID, strPendingLog, ptrEOBInfo->strPayerID);

				return TRUE;
			}
			else {

				//we had more than one record - but is it more than one HCFA Group?
				//double check we only have one HCFA Group
				// (j.jones 2009-08-05 10:43) - PLID 34467 - also include InsuranceLocationPayerIDsT
				// (j.jones 2012-08-01 17:10) - PLID 51918 - supported the new payer ID structure
				_RecordsetPtr rs2 = CreateParamRecordset("SELECT ID FROM HCFASetupT WHERE ID IN "
					"(SELECT HCFASetupGroupID FROM InsuranceCoT "
					"WHERE (InsuranceCoT.PersonID IN (SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
					"		INNER JOIN EbillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EbillingInsCoIDs.ID "
					"		WHERE EbillingInsCoIDs.EbillingID = {STRING}) "
					"	OR InsuranceCoT.PersonID IN ("
					"		SELECT InsuranceLocationPayerIDsT.InsuranceCoID "
					"		FROM InsuranceLocationPayerIDsT "
					"		INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
					"		WHERE EbillingInsCoIDs.EbillingID = {STRING})) "
					"AND HCFASetupGroupID IN ({SQL}))",
					ptrEOBInfo->strPayerID, ptrEOBInfo->strPayerID, sqlLastRecordsetSql);
				if (!rs2->eof) {
					if (rs2->GetRecordCount() == 1) {
						//ah, only one HCFA group, that's good, so let's return

						ptrEOBInfo->nLikelyInsuranceCoID = -1;
						ptrEOBInfo->nHCFAGroupID = AdoFldLong(rs2, "ID", -1);

						Log("CalcInsCoID:NarrowInsuranceSelection: Calculated HCFA Group ID (%li) by "
							"%s and then matching one HCFA Group by payer ID %s", ptrEOBInfo->nHCFAGroupID, strPendingLog, ptrEOBInfo->strPayerID);

						return TRUE;
					}
					else {
						//if more than one HCFA Group, we have no checks left to do

						// (j.jones 2011-09-29 09:02) - I periodically hit this assert on various client remit files,
						// but the EOB ends up posting perfectly fine. But I'm leaving it in here because at some point
						// we need to decide whether to remove the assert, or add new code.
						ASSERT(FALSE);
					}
				}
				rs2->Close();
			}
		}
		rs->Close();
	}

	//if we got here, we have no idea

	Log("CalcInsCoID:NarrowInsuranceSelection: Found multiple by "
		"%s but failed to match by payer ID %s or payer name %s", strPendingLog, ptrEOBInfo->strPayerID, ptrEOBInfo->strInsuranceCoName);

	return FALSE;
}

// (j.jones 2008-11-24 15:04) - PLID 32075 - CheckAdjustmentCodesToIgnore will check the setup in ERemitIgnoredAdjCodesT,
// and make sure that all adjustments with matching group & reason codes are updated to be $0.00
// (j.jones 2016-04-26 14:08) - NX-100327 - this no longer reduces all secondary adjustments to $0.00, that is done upon posting
void CANSI835Parser::CheckAdjustmentCodesToIgnore()
{
	try {

		//query the list of code combinations to ignore
		// (j.jones 2010-09-23 12:55) - PLID 40653 - these are now IDs
		// (j.jones 2016-04-19 17:33) - NX-100246 - added support for ERemitAllowNegativePostingAdjCodesT
		_RecordsetPtr rs = CreateParamRecordset("SELECT AdjustmentGroupCodesT.Code AS GroupCode, AdjustmentReasonCodesT.Code AS ReasonCode "
			"FROM ERemitIgnoredAdjCodesT "
			"INNER JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON ERemitIgnoredAdjCodesT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"INNER JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON ERemitIgnoredAdjCodesT.ReasonCodeID = AdjustmentReasonCodesT.ID; \r\n"
			""
			"SELECT AdjustmentGroupCodesT.Code AS GroupCode, AdjustmentReasonCodesT.Code AS ReasonCode "
			"FROM ERemitAllowNegativePostingAdjCodesT "
			"INNER JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON ERemitAllowNegativePostingAdjCodesT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"INNER JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON ERemitAllowNegativePostingAdjCodesT.ReasonCodeID = AdjustmentReasonCodesT.ID;");
		CString strCodeCombinationsIgnored;
		while (!rs->eof) {

			CString strGroupCode = AdoFldString(rs, "GroupCode", "");
			CString strReasonCode = AdoFldString(rs, "ReasonCode", "");

			if (strGroupCode == "-1" || strReasonCode == "-1"
				|| strGroupCode.IsEmpty() || strReasonCode.IsEmpty()) {

				//should be impossible
				ASSERT(FALSE);

				//skip this bad data
				rs->MoveNext();
				continue;
			}

			long nCountReduced = 0;

			int i = 0, j = 0, k = 0, l = 0;
			//search all EOBs for matching adjustments
			for (i = 0; i<m_arypEOBInfo.GetSize(); i++) {
				const EOBInfo *ptrEOBInfo = m_arypEOBInfo[i];
				if (ptrEOBInfo) {
					if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
						for (j = 0; j<ptrEOBInfo->arypEOBClaimInfo.GetSize(); j++) {
							const EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[j];
							if (pClaimInfo->arypEOBLineItemInfo.GetSize() > 0) {
								for (k = 0; k<pClaimInfo->arypEOBLineItemInfo.GetSize(); k++) {
									const EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[k];
									if (pLineItemInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
										for (l = 0; l<pLineItemInfo->arypEOBAdjustmentInfo.GetSize(); l++) {
											//does this adjustment match our group & reason?
											EOBAdjustmentInfo *pAdj = pLineItemInfo->arypEOBAdjustmentInfo[l];
											if (pAdj->strGroupCode == strGroupCode && pAdj->strReasonCode == strReasonCode) {
												//it does, so reduce its value to $0.00 and increment our count
												//and yes, we do allow doing this if bPatResp is TRUE
												pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
												nCountReduced++;
											}
										}
									}
								}
							}
							if (pClaimInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
								for (k = 0; k<pClaimInfo->arypEOBAdjustmentInfo.GetSize(); k++) {
									//does this adjustment match our group & reason?
									EOBAdjustmentInfo *pAdj = pClaimInfo->arypEOBAdjustmentInfo[k];
									if (pAdj->strGroupCode == strGroupCode && pAdj->strReasonCode == strReasonCode) {
										//it does, so reduce its value to $0.00 and increment our count
										//and yes, we do allow doing this if bPatResp is TRUE
										pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
										nCountReduced++;
									}
								}
							}
						}
					}
				}
			}

			if (nCountReduced > 0) {
				//if we found an adjustment and reduced it, log that we did so
				Log("Ignored Adjustment Group/Reason %s*%s %li times.", strGroupCode, strReasonCode, nCountReduced);
			}

			rs->MoveNext();
		}

		// (j.jones 2016-04-26 14:08) - NX-100327 - this no longer reduces all secondary adjustments to $0.00, that is done upon posting
		//BOOL bIgnoreSecondaryAdjs = (GetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", 1, 0, "<None>", true) == 1);

		// (j.jones 2016-04-19 17:33) - NX-100246 - last pass, look for negative adjustments
		// and reduce them to zero unless the setup specifically states to permit them

		rs = rs->NextRecordset(NULL);

		bool bPostAllNegativeAdjustments = (GetRemotePropertyInt("ERemit_AllowAllNegativeAdjustmentsToBePosted", FALSE, 0, "<None>", true) == 1) ? true : false;

		//if we're posting all negative adjustments, we do not need to skip anything
		if (!bPostAllNegativeAdjustments) {
			
			CArray<CString, LPCTSTR> aryGroupCodes;
			CArray<CString, LPCTSTR> aryReasonCodes;

			while (!rs->eof) {

				CString strGroupCode = AdoFldString(rs, "GroupCode", "");
				CString strReasonCode = AdoFldString(rs, "ReasonCode", "");

				if (strGroupCode == "-1" || strReasonCode == "-1"
					|| strGroupCode.IsEmpty() || strReasonCode.IsEmpty()) {

					//should be impossible
					ASSERT(FALSE);

					//skip this bad data
					rs->MoveNext();
					continue;
				}

				//save in our array
				aryGroupCodes.Add(strGroupCode);
				aryReasonCodes.Add(strReasonCode);

				rs->MoveNext();
			}

			ASSERT(aryGroupCodes.GetSize() == aryReasonCodes.GetSize());

			//if the arrays are empty, we still have to go through all
			//the adjustments, because all negatives will change to zero
			
			long nCountReduced = 0;

			//search all EOBs for matching adjustments
			for (int i = 0; i<m_arypEOBInfo.GetSize(); i++) {
				const EOBInfo *ptrEOBInfo = m_arypEOBInfo[i];
				if (ptrEOBInfo) {
					if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
						for (int j = 0; j<ptrEOBInfo->arypEOBClaimInfo.GetSize(); j++) {
							const EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[j];

							//we do NOT want to skip negative adjustments on reversed claims
							if (pClaimInfo->bIsReversedClaim) {
								continue;
							}

							if (pClaimInfo->arypEOBLineItemInfo.GetSize() > 0) {
								for (int k = 0; k<pClaimInfo->arypEOBLineItemInfo.GetSize(); k++) {
									const EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[k];
									if (pLineItemInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
										for (int l = 0; l<pLineItemInfo->arypEOBAdjustmentInfo.GetSize(); l++) {											
											EOBAdjustmentInfo *pAdj = pLineItemInfo->arypEOBAdjustmentInfo[l];
											//is this a negative adjustment? (skip patient resp types)
											if (pAdj->cyAdjustmentAmt < COleCurrency(0, 0) && !pAdj->bPatResp) {
												//it is negative - see if it's a permitted group or reason code
												bool bFound = false;
												for (int z = 0; z < aryReasonCodes.GetSize() && !bFound; z++) {
													CString strGroupCode = aryGroupCodes[z];
													CString strReasonCode = aryReasonCodes[z];
													if (pAdj->strGroupCode == strGroupCode && pAdj->strReasonCode == strReasonCode) {
														//found a match
														bFound = true;
													}
												}
												if(!bFound) {
													//this adjustment's group & reason code is not set to allow
													//negatives, so reduce its value to $0.00
													pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
													nCountReduced++;
												}
											}
										}
									}
								}
							}
							if (pClaimInfo->arypEOBAdjustmentInfo.GetSize() > 0) {
								for (int k = 0; k<pClaimInfo->arypEOBAdjustmentInfo.GetSize(); k++) {
									EOBAdjustmentInfo *pAdj = pClaimInfo->arypEOBAdjustmentInfo[k];
									//is this a negative adjustment? (skip patient resp types)
									if (pAdj->cyAdjustmentAmt < COleCurrency(0, 0) && !pAdj->bPatResp) {
										//it is negative - see if it's a permitted group or reason code
										bool bFound = false;
										for (int z = 0; z < aryReasonCodes.GetSize() && !bFound; z++) {
											CString strGroupCode = aryGroupCodes[z];
											CString strReasonCode = aryReasonCodes[z];
											if (pAdj->strGroupCode == strGroupCode && pAdj->strReasonCode == strReasonCode) {
												//found a match
												bFound = true;
											}
										}
										if (!bFound) {
											//this adjustment's group & reason code is not set to allow
											//negatives, so reduce its value to $0.00
											pAdj->cyAdjustmentAmt = COleCurrency(0, 0);
											nCountReduced++;
										}
									}
								}
							}
						}
					}
				}
			}

			if (nCountReduced > 0) {
				//if we found an adjustment and reduced it, log that we did so
				Log("Ignored %li negative adjustments.", nCountReduced);
			}
		}

		rs->Close();

	}NxCatchAll("Error in CANSI835Parser::CheckAdjustmentCodesToIgnore");
}

// (j.jones 2008-11-25 16:32) - PLID 32133 - CheckWarnPaymentOverage will report to the user how much of the payment
// may be converted to adjustments. This has to be called after the EOB is loaded onto the screen because claim level
// adjustments and other changes may alter the payment amounts that were parsed from the EOB.
// (j.jones 2012-02-13 10:34) - PLID 48084 - this now just warns of paying past zero, they would be payments now, not adjustments
void CANSI835Parser::CheckWarnPaymentOverage()
{
	try {

		COleCurrency cyTotalOverpaidAmount = COleCurrency(0, 0);

		int i = 0, j = 0, k = 0;
		//search all EOBs for matching adjustments
		for (i = 0; i<m_arypEOBInfo.GetSize(); i++) {
			const EOBInfo *ptrEOBInfo = m_arypEOBInfo[i];
			if (ptrEOBInfo) {

				COleCurrency cyRemainingTotal = ptrEOBInfo->cyTotalPaymentAmt;

				if (ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {
					for (j = 0; j<ptrEOBInfo->arypEOBClaimInfo.GetSize(); j++) {
						const EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[j];

						// (j.jones 2011-02-09 14:01) - PLID 42391 - if there is a reversal, do not count
						// it as part of the payment overage, because we currently do not post any reversed
						// charges, and thus will not overpay due to these charges
						// (j.jones 2011-10-03 10:29) - PLID 45785 - If the pay totals are invalid,
						// we will also not post the payment, and thus it would not contribute to an overage.
						// I also changed this code to use the claim payment amount, not the charge payment
						// amounts, because now when they differ, they would not be posted.
						// (We've also noticed that in cases where they differ, the charge payment is usually 
						// higher - and wrong.)
						// (j.jones 2012-04-18 15:59) - PLID 35306 - we support posting reversals now,
						// which means we will update the remaining total with the reversal's negative payment
						if (!HasInvalidPayTotals(pClaimInfo)) {
							//reduce our remaining total by this payment amount
							cyRemainingTotal -= pClaimInfo->cyClaimPaymentAmt;
						}
					}
				}

				//if our batch payment balance is negative, add the absolute value
				//of that balance to our total overage
				if (cyRemainingTotal < COleCurrency(0, 0)) {
					cyTotalOverpaidAmount += -(cyRemainingTotal);
				}
			}
		}

		if (cyTotalOverpaidAmount > COleCurrency(0, 0)) {
			//if we have any overpayments, log this, but also warn the user

			// (j.jones 2012-02-13 10:41) - PLID 48084 - Reworded these warnings to reflect the new logic
			// of applying payments past zero, resulting in a negative batch payment balance.
			// We no longer apply adjustments in lieu of payments in these scenarios.
			Log("Warning: This EOB will potentially exceed the batch payment balance and apply up to %s more payments than the check is for.\n"
				"The resulting batch payment may end up with a negative balance.", FormatCurrencyForInterface(cyTotalOverpaidAmount));

			CString strWarning;
			strWarning.Format("The imported EOB has an overpayment of %s that will potentially exceed the batch payment balance.\n"
				"This can happen if an insurance takeback or overpayment occurs, and the EOB is attempting to apply "
				"more credits than the payment is for.\n\n"
				"If the batch payment is completely applied to all patients, posting this EOB will continue to apply payments "
				"for the overage amount. The batch payment may end up with a negative balance.", FormatCurrencyForInterface(cyTotalOverpaidAmount));
			MessageBox(m_pParentWnd->GetSafeHwnd(), strWarning, "Practice", MB_ICONINFORMATION | MB_OK);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
// can optionally disable PeekAndPump usage for the import process
void CANSI835Parser::PeekAndPump_ANSI835Parser()
{
	if (m_bEnablePeekAndPump) {
		PeekAndPump();
	}
}

// (j.jones 2011-03-18 15:11) - PLID 42905 - given a patient ID, find all of that patient's
// charges that we are about to post to, and add to the list
void CANSI835Parser::GetChargeIDsByPatientID(long nPatientID, CArray<long, long> &aryUsedCharges)
{
	//ensure this is blank to begin with
	aryUsedCharges.RemoveAll();

	if (nPatientID == -1) {
		//do nothing if we weren't given a patient ID
		return;
	}

	//for each EOB
	for (int i = 0; i<m_arypEOBInfo.GetSize(); i++) {

		const EOBInfo *ptrEOBInfo = m_arypEOBInfo[i];

		if (ptrEOBInfo != NULL && ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {

			//for each claim
			for (int j = 0; j<ptrEOBInfo->arypEOBClaimInfo.GetSize(); j++) {

				const EOBClaimInfo *pClaimInfo = ptrEOBInfo->arypEOBClaimInfo[j];

				//is this claim for our patient?
				if (pClaimInfo != NULL && pClaimInfo->nPatientID != -1
					&& pClaimInfo->nPatientID == nPatientID
					&& pClaimInfo->arypEOBLineItemInfo.GetSize() > 0) {

					//for each charge, grab its id
					for (int k = 0; k<pClaimInfo->arypEOBLineItemInfo.GetSize(); k++) {

						const EOBLineItemInfo *pLineItemInfo = pClaimInfo->arypEOBLineItemInfo[k];
						if (pLineItemInfo != NULL && pLineItemInfo->nChargeID != -1) {
							//gotcha! (don't worry about adding duplicates)
							aryUsedCharges.Add(pLineItemInfo->nChargeID);
						}
					}
				}
			}
		}
	}
}

// (j.jones 2011-03-21 14:53) - PLID 42917 - this function backs up the remit file
// and EOB.txt to the server's NexTech\ConvertedEOBs path, and also ensures that files
// > 30 days old are deleted
void CANSI835Parser::CopyConvertedEOBToServer()
{
	try {

		//calculate the server's EOB path
		CString strServerPath = GetNxConvertedEOBsPath();

		//remove all files that are > 30 days old
		{
			CTime tm;
			//this assumes that at least the system's date is accurate
			CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(30, 0, 0, 0);

			//first remove EOB files
			CFileFind finder;
			BOOL bWorking = finder.FindFile(strServerPath ^ "EOB*.txt");
			while (bWorking) {
				bWorking = finder.FindNextFile();

				CString strFilePath = finder.GetFilePath();
				tm = FileUtils::GetFileModifiedTime(strFilePath);
				if (tm < tmMin) {
					DeleteFile(strFilePath);
				}
			}

			//now remove all files with the remit backup extension
			bWorking = finder.FindFile(strServerPath ^ "*.rmtbak");
			while (bWorking) {
				bWorking = finder.FindNextFile();

				CString strFilePath = finder.GetFilePath();
				tm = FileUtils::GetFileModifiedTime(strFilePath);
				if (tm < tmMin) {
					DeleteFile(strFilePath);
				}
			}
		}

		COleDateTime dtServer = GetRemoteServerTime();

		CString strNewFileName;
		//now copy our EOB.txt to the server
		{
			strNewFileName.Format("%s\\EOB_%s_%s.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));

			//it's very unlikely that this file will exist, but handle the case anyways
			int nCount = 0;
			while (DoesExist(strNewFileName)) {

				//try adding an index to the end
				nCount++;

				if (nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy EOB to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\EOB_%s_%s_%li.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if (!CopyFile(m_strOutputFile, strNewFileName, TRUE)) {
				//failed
				ThrowNxException("Cannot copy EOB to server, filename: %s", strNewFileName);
			}
		}

		// (b.spivey, October 9th, 2014) PLID 62701 - move it to our more permanent server storage. 
		m_strStoredParsedFile = CopyParsedEOBToServerStorage(strNewFileName);

		//now copy our remit file to the server
		{
			CString strFileNameNoExt = FileUtils::GetFileName(m_strFileName);
			int nDot = strFileNameNoExt.ReverseFind('.');
			if (nDot != -1) {
				strFileNameNoExt = strFileNameNoExt.Left(nDot - 1);
			}

			CString strNewFileName;
			strNewFileName.Format("%s\\%s_%s_%s.rmtbak", strServerPath, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));

			//it's very unlikely that this file will exist, but handle the case anyways
			int nCount = 0;
			while (DoesExist(strNewFileName)) {

				//try adding an index to the end
				nCount++;

				if (nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy remit file to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\%s_%s_%s_%li.rmtbak", strServerPath, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if (!CopyFile(m_strFileName, strNewFileName, TRUE)) {
				//failed
				ThrowNxException("Cannot copy remit file to server, filename: %s", strNewFileName);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, October 9th, 2014) PLID 62701 - Accessor
CString CANSI835Parser::GetStoredParsedFilePath()
{
	return m_strStoredParsedFile;
}

// (j.jones 2011-09-28 14:00) - PLID 45486 - returns true if the cyClaimPaymentAmt
// does not match the total of payments for each charge
// (j.jones 2011-10-03 10:34) - PLID 45785 - moved into the parser class, from the EOBDlg
BOOL CANSI835Parser::HasInvalidPayTotals(const EOBClaimInfo *pClaim)
{
	if (pClaim == NULL) {
		ThrowNxException("CANSI835Parser::HasInvalidPayTotals called on a NULL claim!");
	}

	//sum up the total amount paid for each charge
	COleCurrency cyTotalPaidPerCharge = GetPaymentTotalsForCharges(pClaim);

	//return TRUE if this total does not match the claim payment amount
	return (cyTotalPaidPerCharge != pClaim->cyClaimPaymentAmt);
}

// (j.jones 2011-09-28 14:00) - PLID 45486 - actually calculates the amt. paid for each charge
// in the claim, which may or may not match the cyClaimPaymentAmt
// (j.jones 2011-10-03 10:34) - PLID 45785 - moved into the parser class, from the EOBDlg
COleCurrency CANSI835Parser::GetPaymentTotalsForCharges(const EOBClaimInfo *pClaim)
{
	if (pClaim == NULL) {
		ThrowNxException("CANSI835Parser::GetPaymentTotalsForCharges called on a NULL claim!");
	}

	//sum up the total amount paid for each charge
	COleCurrency cyTotalPaidPerCharge = COleCurrency(0, 0);
	for (int i = 0; i<pClaim->arypEOBLineItemInfo.GetSize(); i++) {
		const EOBLineItemInfo *pCharge = pClaim->arypEOBLineItemInfo[i];
		if (pCharge) {
			cyTotalPaidPerCharge += pCharge->cyLineItemPaymentAmt;
		}
	}

	return cyTotalPaidPerCharge;
}

// (j.jones 2012-04-23 12:20) - PLID 49846 - Most remit files with reversals will list the reversal,
// then the re-posted claim immediately following. If so, we link them together during the parsing.
// But if they aren't listed this way, we need to link them together at the end of posting.
// This function will handle linking reversed & re-posted claims that are not already linked.
void CANSI835Parser::VerifyReversedClaimLinks()
{
	try {

		for (int i = 0; i<m_arypEOBInfo.GetSize(); i++) {

			const EOBInfo *ptrEOBInfo = m_arypEOBInfo[i];
			if (ptrEOBInfo != NULL && ptrEOBInfo->arypEOBClaimInfo.GetSize() > 0) {

				//for each claim
				for (int j = 0; j<ptrEOBInfo->arypEOBClaimInfo.GetSize(); j++) {

					EOBClaimInfo *pReversedClaim = ptrEOBInfo->arypEOBClaimInfo[j];
					if (pReversedClaim->bIsReversedClaim && pReversedClaim->pRepostedSibling == NULL) {
						//This is a reversed claim with no reposted sibling.
						//Find the reposted claim within the same EOB, if it exists. It is possible there isn't one.

						BOOL bFound = FALSE;
						for (int k = 0; k<ptrEOBInfo->arypEOBClaimInfo.GetSize() && !bFound; k++) {

							EOBClaimInfo *pRepostedClaim = ptrEOBInfo->arypEOBClaimInfo[k];

							if (pRepostedClaim == pReversedClaim) {
								continue;
							}

							//look for claims not marked as being reversed or re-posted,
							//where it's for the same patient and the opposite charge amount
							if (!pRepostedClaim->bIsReversedClaim && !pRepostedClaim->bIsRepostedClaim
								&& pRepostedClaim->pReversedSibling == NULL
								&& pRepostedClaim->strPatientID == pReversedClaim->strPatientID
								&& pRepostedClaim->cyClaimChargeAmt == -(pReversedClaim->cyClaimChargeAmt)) {

								//found our reposted claim, link them up

								pRepostedClaim->bIsRepostedClaim = TRUE;
								pRepostedClaim->pReversedSibling = pReversedClaim;
								pReversedClaim->pRepostedSibling = pRepostedClaim;

								bFound = TRUE;
							}
						}

						//Not a failure, just used for debugging.
						ASSERT(bFound);
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}