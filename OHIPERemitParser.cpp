// OHIPERemitParser.cpp: implementation of the COHIPERemitParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OHIPERemitParser.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "FileUtils.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2008-06-16 12:21) - PLID 30413 - created

// (j.jones 2012-05-25 13:21) - PLID 44367 - All calls to AfxMessageBox were changed to MessageBox
// to make the modeless dialog behave better.

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COHIPERemitParser::COHIPERemitParser()
{
	m_pParentWnd = NULL;
	m_ptrProgressBar = NULL;
	m_ptrProgressStatus = NULL;
	m_CountOfEOBs = 0;	
	m_bEnablePeekAndPump = TRUE;
	m_bParsingFailed = FALSE;
	// (b.spivey, October 01, 2012) - PLID 52666 - Grab the custom field which is only set in the OHIP properties. 
	m_nHealthNumCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
	// (j.jones 2012-10-04 15:28) - PLID 52929 - added cache for the OHIPIgnoreMissingPatients preference
	// (the bulk cache for this is in the calling class, CEOBDlg)
	m_bIgnoreMissingPatients = (GetRemotePropertyInt("OHIPIgnoreMissingPatients", 0, 0, "<None>", true) == 1);
}

COHIPERemitParser::~COHIPERemitParser()
{
	ClearEOB();
}

// (j.jones 2012-10-04 15:28) - PLID 52929 - take in the current claim pointer (optional), we might
// skip outputting any data for the current claim
void COHIPERemitParser::OutputData(CString &OutputString, CString strNewData, OPTIONAL IN OHIPEOBClaimInfo *pCurrentClaimInfo)
{	
	BOOL bOutput = TRUE;

	// (j.jones 2012-10-04 15:37) - PLID 52929 - If this data is for a claim (the claim pointer is NULL if it's a
	// non-claim-specific piece of content) and the claim is not found in the system, and the preference
	// is enabled to skip missing claims, we need to skip outputting any content regarding this claim
	if(pCurrentClaimInfo != NULL && pCurrentClaimInfo->nBillID == -1 && m_bIgnoreMissingPatients) {
		bOutput = FALSE;
	}
		
	if(bOutput) {
		OutputString += strNewData;
	}
}

CString COHIPERemitParser::ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim /*= FALSE*/)
{
	//we could just have used CString::Mid() but if the line isn't the proper length,
	//we should handle it more gracefully, just returning ""

	//the specs state the position to start at using 1-based counting,
	//but CString uses 0-based counting, so subtract 1 from nStart
	nStart--;

	long nLineLength = strLine.GetLength();

	if(nStart > nLineLength) {
		// (j.jones 2010-09-22 07:35) - PLID 40621 - don't assert, instead track
		// that parsing failed, as this should not be possible in a valid file
		m_bParsingFailed = TRUE;
		return "";
	}

	if(nStart + nLength > nLineLength) {
		
		// (j.jones 2010-09-22 07:35) - PLID 40621 - don't assert, instead track
		// that parsing failed, as this should not be possible in a valid file
		m_bParsingFailed = TRUE;

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

COleCurrency COHIPERemitParser::ParseOHIPCurrency(CString strAmount, CString strSign)
{
	COleCurrency cyAmt = COleCurrency(0,0);

	if(strAmount.GetLength() > 2) {
		CString strAmt;
		strAmt.Format("%s%s.%s", strSign, strAmount.Left(strAmount.GetLength() - 2), strAmount.Right(2));
		strAmt.TrimLeft();
		if(!cyAmt.ParseCurrency(strAmt)) {
			cyAmt = COleCurrency(0,0);
			ASSERT(FALSE);
		}
	}
	else {
		cyAmt = COleCurrency(0,0);
		ASSERT(FALSE);
	}

	return cyAmt;
}

//returns the current EOB Info object we are building (in OHIP there should only be one)
OHIPEOBInfo* COHIPERemitParser::GetCurrentEOBInfo()
{
	if(m_paryOHIPEOBInfo.GetSize() == 0) {
		ThrowNxException("GetCurrentEOBInfo failed to find an EOB!");
	}

	return (OHIPEOBInfo*)m_paryOHIPEOBInfo.GetAt(m_paryOHIPEOBInfo.GetSize()-1);
}

//returns the current Claim Info object we are building (last claim added to our EOB)
OHIPEOBClaimInfo* COHIPERemitParser::GetCurrentClaimInfo()
{
	OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();

	if(pEOBInfo == NULL) {
		ThrowNxException("GetCurrentClaimInfo failed to find an EOB!");
	}

	if(pEOBInfo->paryEOBClaimInfo.GetSize() == 0) {
		ThrowNxException("GetCurrentClaimInfo failed to find a claim!");
	}

	return (OHIPEOBClaimInfo*)pEOBInfo->paryEOBClaimInfo.GetAt(pEOBInfo->paryEOBClaimInfo.GetSize()-1);
}

//returns the current Line Item Info object we are building (last line item added to the last claim)
OHIPEOBLineItemInfo* COHIPERemitParser::GetCurrentLineItemInfo()
{
	OHIPEOBClaimInfo *pEOBClaimInfo = GetCurrentClaimInfo();

	if(pEOBClaimInfo == NULL) {
		ThrowNxException("GetCurrentLineItemInfo failed to find a claim!");
	}

	if(pEOBClaimInfo->paryOHIPEOBLineItemInfo.GetSize() == 0) {
		ThrowNxException("GetCurrentLineItemInfo failed to find a line item!");
	}

	return (OHIPEOBLineItemInfo*)pEOBClaimInfo->paryOHIPEOBLineItemInfo.GetAt(pEOBClaimInfo->paryOHIPEOBLineItemInfo.GetSize()-1);
}

//creates and intializes a new EOB info object, adds to our tracked list, and returns the pointer
OHIPEOBInfo* COHIPERemitParser::GenerateNewEOBInfo()
{
	//create a new EOB object
	OHIPEOBInfo *ptrOHIPEOBInfo = new OHIPEOBInfo;
	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor

	//fill the index
	ptrOHIPEOBInfo->nIndex = m_CountOfEOBs;

	//add to our list
	m_paryOHIPEOBInfo.Add(ptrOHIPEOBInfo);

	return ptrOHIPEOBInfo;
}

//creates and intializes a new EOB claim info object, adds to our tracked list, and returns the pointer
OHIPEOBClaimInfo* COHIPERemitParser::GenerateNewEOBClaimInfo()
{
	OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();

	//create a new claim object, initialize all non-CStrings
	OHIPEOBClaimInfo *pClaimInfo = new OHIPEOBClaimInfo;

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	
	//add to our list
	pEOBInfo->paryEOBClaimInfo.Add(pClaimInfo);

	return pClaimInfo;
}

//creates and intializes a new EOB line item info object, adds to our tracked list, and returns the pointer
OHIPEOBLineItemInfo* COHIPERemitParser::GenerateNewEOBLineItemInfo()
{
	OHIPEOBClaimInfo *ptrEOBClaimInfo = GetCurrentClaimInfo();

	OHIPEOBLineItemInfo *pLineItemInfo = new OHIPEOBLineItemInfo;

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	
	//add to our list
	ptrEOBClaimInfo->paryOHIPEOBLineItemInfo.Add(pLineItemInfo);

	return pLineItemInfo;
}

// (j.jones 2009-09-25 10:32) - PLID 34453 - added accounting transactions
OHIPEOBAccountingTransaction* COHIPERemitParser::GenerateNewEOBAccountingTransaction()
{
	OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();

	//create a adjustment object
	OHIPEOBAccountingTransaction *pTxnInfo = new OHIPEOBAccountingTransaction;

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	
	//add to our list
	pEOBInfo->paryOHIPEOBAccountingTransaction.Add(pTxnInfo);

	return pTxnInfo;
}

// (j.jones 2012-05-25 13:53) - PLID 44367 - pass in a parent
BOOL COHIPERemitParser::ParseFile(CWnd *pParentWnd) 
{
	CWaitCursor pWait;
	
	try {

		// (j.jones 2012-05-25 13:54) - PLID 44367 - track the parent window
		m_pParentWnd = pParentWnd;

		// (j.jones 2010-09-22 07:35) - PLID 40621 - reset this flag
		m_bParsingFailed = FALSE;

		// (j.jones 2008-12-19 09:39) - PLID 32519 - Supported auto-opening a file,
		// which would be passed in as m_strFileName. So if we don't have a file,
		// or it is not valid, then browse.
		if(m_strFileName.IsEmpty() || !DoesExist(m_strFileName)) {

			//browse for a file
			CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST, NULL, m_pParentWnd);
			if (BrowseFiles.DoModal() == IDCANCEL) {
				return FALSE;
			}
			
			m_strFileName = BrowseFiles.GetPathName();
		}

		m_strOutputFile = GetNxTempPath() ^ "EOB.txt";

		BOOL bIsValidFile = FALSE;
		BOOL bIsCompleteFile = FALSE;
		
		//open the file for reading
		if(!m_InputFile.Open(m_strFileName,CFile::modeRead | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The E-Remittance input file could not be found or opened.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return FALSE;
		}

		if(!m_OutputFile.Open(m_strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The E-Remittance output file could not be created.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return FALSE;
		}

		CWaitCursor pWait;

		Log("Importing remittance file: " + m_strFileName);

		//arbitrary length, we will change this after the file is parsed
		m_ptrProgressBar->SetRange(0, 100);
		m_ptrProgressBar->SetPos(0);		

		CArchive arIn(&m_InputFile, CArchive::load);

		CString strLine;

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
			if(strIdentifier.Left(2) == "HR" && nRecordID >= 1 && nRecordID <= 8) {
				
				bIsValidFile = TRUE;
			}
			else {

				//don't display junk if it's not a real file
				if(!bIsValidFile) {

					//close the file
					arIn.Close();
					m_InputFile.Close();
					m_OutputFile.Close();

					// (j.jones 2008-07-07 11:02) - PLID 30604 - see if it is a batch edit file
					if(strIdentifier.Left(2) == "HB") {						
						//it seems to be a batch edit file
						// (j.jones 2008-12-17 15:47) - PLID 31900 - the 'Format Batch Edit Report' button has been removed,
						// it's only available through the report manager, so reflect that in this warning
						MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an OHIP remittance file. It appears to be an OHIP batch edit file.\n"
							"Please use the Report Manager feature on the E-Billing tab to parse and view this report.", "Practice", MB_ICONINFORMATION|MB_OK);
					}
					// (j.jones 2008-07-07 11:27) - PLID 21968 - see if it is a claims error file
					else if(strIdentifier.Left(2) == "HX") {
						// (j.jones 2008-12-17 15:47) - PLID 31900 - the 'Format Claims Error Report' button has been removed,
						// it's only available through the report manager, so reflect that in this warning
						MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an OHIP remittance file. It appears to be an OHIP claims error file.\n"
							"Please use the Report Manager feature on the E-Billing tab to parse and view this report.", "Practice", MB_ICONINFORMATION|MB_OK);
					}
					else {
						MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to parse is not an OHIP remittance file.", "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return FALSE;
				}
			}

			switch(nRecordID) {
				case 1:
					FileHeader_1(strLine);
					break;
				case 2:
					AddressRecord1_2(strLine);
					break;
				case 3:
					AddressRecord2_3(strLine);
					break;
				case 4:
					ClaimHeader_4(strLine);
					break;
				case 5:
					ClaimItem_5(strLine);
					break;
				case 6:
					BalanceForward_6(strLine);
					break;
				case 7:
					AccountingTransaction_7(strLine);
					break;
				case 8:
					// (j.jones 2010-09-21 17:44) - PLID 40621 - If HR8 exists, then we have reached the end
					// (or near the end) of the file, so we can safely assume we've received the entire file.
					// Keep in mind there is usually dozens and dozens of HR8 lines, so this isn't actually
					// the last line in the file.
					bIsCompleteFile = TRUE;
					MessageFacility_8(strLine);
					break;
			}

			// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
			// function, as we may have disabled this ability
			PeekAndPump_OHIPERemitParser();
		}

		// (j.jones 2010-09-22 07:37) - PLID 40621 - if parsing failed, the file can't be complete
		if(bIsCompleteFile && m_bParsingFailed) {
			//This is not likely to happen, but I have seen a file where RMB claims
			//showed up after the HR8 notes. In the unlikely event that the very
			//last RMB line is cut off in the middle, this will catch that case.
			bIsCompleteFile = FALSE;
		}

		//leave if it's not a real file
		// (j.jones 2010-09-21 17:44) - PLID 40621 - also leave if it is not a complete file
		if(!bIsValidFile || !bIsCompleteFile) {

			//close the file
			arIn.Close();
			m_InputFile.Close();
			m_OutputFile.Close();
			
			if(!bIsValidFile) {
				MessageBox(m_pParentWnd->GetSafeHwnd(), "The file you are trying to import is not an OHIP remittance file.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			// (j.jones 2010-09-21 17:44) - PLID 40621 - warn if not a complete file
			else if(!bIsCompleteFile) {
				MessageBox(m_pParentWnd->GetSafeHwnd(), "The OHIP remittance file you are trying to import is incomplete. "
					"It is possible that a problem has occurred when downloading this file.\n\n"
					"You must download a new copy of this file from OHIP in order to import it.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return FALSE;
		}

		//now write the messages, if any
		OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();
		if(pEOBInfo) {

			CString strOutputString, str;

			if(pEOBInfo->arystrMessages.GetSize() > 0) {
				OutputData(strOutputString, "\r\n=========================================== Extra Messages ==============================================\r\n\r\n", NULL);
			}

			for(int i=0; i<pEOBInfo->arystrMessages.GetSize(); i++) {
				str.Format("%s\r\n", pEOBInfo->arystrMessages.GetAt(i));
				OutputData(strOutputString, str, NULL);
			}

			if(!strOutputString.IsEmpty()) {
				m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
			}
		}

		// (j.jones 2012-10-04 16:29) - PLID 52929 - If the setting to ignore missing patients is enabled,
		// remove their pointers from the parsed data and output a count of the total claims ignored.
		// The nCountSkippedClaims is tracked so the EOBDlg can detect that claims were skipped.
		if(m_bIgnoreMissingPatients) {
			long nTotalSkippedClaims = 0;

			for(int e=0;e<m_paryOHIPEOBInfo.GetSize();e++) {
				OHIPEOBInfo *pEOB = (OHIPEOBInfo*)m_paryOHIPEOBInfo.GetAt(e);
				//initialize to zero, though they should already be zero
				pEOB->nCountSkippedClaims = 0;
				pEOB->cyTotalPaidAfterSkipping = COleCurrency(0,0);

				for(int c = pEOB->paryEOBClaimInfo.GetSize() - 1; c>=0; c--) {
					OHIPEOBClaimInfo *pClaim = (OHIPEOBClaimInfo*)pEOB->paryEOBClaimInfo.GetAt(c);
					if(pClaim->nBillID == -1) {
						//we are going to skip -1 BillIDs
						pEOB->nCountSkippedClaims++;

						//delete this claim pointer
						delete pClaim;
						pEOB->paryEOBClaimInfo.RemoveAt(c);
					}
					else {
						//calculate the total payments of what will be paid
						for(int l=0; l<pClaim->paryOHIPEOBLineItemInfo.GetSize(); l++) {
							OHIPEOBLineItemInfo *pLineItemInfo = (OHIPEOBLineItemInfo*)pClaim->paryOHIPEOBLineItemInfo.GetAt(l);
							pEOB->cyTotalPaidAfterSkipping += pLineItemInfo->cyLineItemPaymentAmt;
						}
					}
				}

				//track the total
				nTotalSkippedClaims += pEOB->nCountSkippedClaims;

				//if the amount paid is less than the check total, output that change
				if(pEOB->cyTotalPaidAfterSkipping < pEOB->cyTotalPaymentAmt) {
					CString strOutputString;
					strOutputString.Format("\r\n=========================================== Payment Changed ==========================================\r\n\r\n"
						"The payment received for %s will be reduced to %s to reflect the amount that will be applied to patients.\r\n",
						FormatCurrencyForInterface(pEOB->cyTotalPaymentAmt, TRUE, TRUE), FormatCurrencyForInterface(pEOB->cyTotalPaidAfterSkipping, TRUE, TRUE));
					m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
				}
			}

			if(nTotalSkippedClaims > 0) {
				//output how many claims will be skipped
				CString strOutputString;
				strOutputString.Format("\r\n=========================================== Skipped Claims ===========================================\r\n\r\n"
					"%li claims were reported in this remittance file for claims and or patients not found in the system.\r\n"
					"Because your preferences are set to ignore payments for patients and bills not found in the system,\r\n"
					"these %li claims were removed from this report and will not be displayed in Practice.\r\n", nTotalSkippedClaims, nTotalSkippedClaims);
				m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
			}
		}

		//close the file
		arIn.Close();
		m_InputFile.Close();
		m_OutputFile.Close();

		// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up both the remit
		// file and the EOB.txt to the server, which we would only do if it was a valid remit file
		CopyConvertedEOBToServer();

		// (j.jones 2012-10-04 14:55) - handle the fact that ShellExecuteEx still doesn't work sometimes in VS2008
#ifdef DEBUG
		return TRUE;
#endif

		//open the finished file in notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.jones 2010-09-02 15:44) - PLID 40388 - fixed crash caused by passing in "this" as a parent
		HINSTANCE hInst = ShellExecute((HWND)GetDesktopWindow(), NULL, "notepad.exe", ("'" + m_strOutputFile + "'"), NULL, SW_SHOW);

		return TRUE;

	}NxCatchAll("Error parsing OHIP E-Remittance file.");

	return FALSE;
}

void COHIPERemitParser::ClearEOB() {

	try {

		m_CountOfEOBs = 0;

		//clear out the EOB and all its sub-arrays

		// (j.jones 2012-10-04 17:03) - PLID 52929 - I moved each object's memory cleanup
		// to destructor functions, so this function only needs to delete the EOB pointers.

		for(int a=m_paryOHIPEOBInfo.GetSize()-1;a>=0;a--) {
			OHIPEOBInfo *ptrOHIPEOBInfo = (OHIPEOBInfo*)m_paryOHIPEOBInfo.GetAt(a);
			if(ptrOHIPEOBInfo) {
				delete ptrOHIPEOBInfo;
			}

			ptrOHIPEOBInfo = NULL;
			m_paryOHIPEOBInfo.RemoveAt(a);
		}

		m_strFileName = "";

	}NxCatchAll("Error cleaning up EOB.");
}

/*

//Record Types:

ID - Name - Description

1 - File Header - Health care provider information

2 - Address Record 1 - Name and address Line 1 of billing agent or health care provider

3 - Address Record 2 - Name and address Line 2 and 3 of billing agent or health care provider

4 - Claim Header - Common control information for each claim

5 - Claim Item - Detailed information for each item of service within a claim (code, date, amounts)

6 - Balance Forward - This record is present only if the previous month's remittance was NEGATIVE.

7 - Accounting Transaction - This record is present only if an accounting transaction is posted to the remittance advice
							(advance, reduction, advance repayment)

8 - Message Facility - A facility for the MOH to send messages to health care providers. May or may not be present.
						May have up to 99,999 occurrences.

//Format Legend

A - Alphabetic

N - Numeric

X - Alphanumeric

D - Date (YYYYMMDD)

S - Spaces

- All alphabetic characters are uppercase unless otherwise stated.

- The last 2 digits of amount fields are cents.

//Layout

1 - File Header

2 - Address Record 1

3 - Address Record 2

	4 - Claim Header

		5 - Claim Item

			6 - Balance Forward

			7 - Accounting Transaction

			8 - Message Facility

		5 - Claim Item

		..repeat..

	4 - Claim Header

	..repeat..

8 - Message Facility

*/

//Health care provider information (once per file)
void COHIPERemitParser::FileHeader_1(CString strLine)
{
	CString strOutputString, str;

	//get a new OHIP EOB Info object
	OHIPEOBInfo *ptrOHIPEOBInfo = GenerateNewEOBInfo();

	//increment after the addition
	m_CountOfEOBs++;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'
	
	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '1'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Tech Spec Release Identifier	4 / 3 / X				Always 'V03'

	CString strTechSpec = ParseElement(strLine, 4, 3);

	//Reserved for MOH Use			7 / 1 / X				Always '0' (zero)

	CString strReserved1 = ParseElement(strLine, 7, 1);

	//Group # or Lab. Licence #		8 / 4 / X

	CString strGroupNum = ParseElement(strLine, 8, 4);

	if(!strGroupNum.IsEmpty()) {
		ptrOHIPEOBInfo->strGroupNumber = strGroupNum;
	}

	//Health Care Provider Number	12 / 6 / N

	CString strProvNum = ParseElement(strLine, 12, 6);

	if(!strProvNum.IsEmpty()) {
		ptrOHIPEOBInfo->strProvNumber = strProvNum;
	}

	//Specialty						18 / 2 / X				Space if no HR 4/5 records, otherwise numeric

	CString strSpecialty = ParseElement(strLine, 18, 2);

	//MOH Office Code				20 / 1 / A				

	CString strMOHOfficeCode = ParseElement(strLine, 20, 1);

	//Remit. Advice Data Sequence	21 / 1 / N				Number representing sort sequence

	CString strDataSeq = ParseElement(strLine, 21, 1);

	//Payment Date					22 / 8 / D				CheCK or direct deposit date

	CString strPayDate = ParseElement(strLine, 22, 8);

	CString strPaymentDate;
	if(strPayDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strPayDate.Left(4)), atoi(strPayDate.Mid(4, 2)), atoi(strPayDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {
			ptrOHIPEOBInfo->dtPaymentDate = dt;
			// (j.jones 2008-12-15 11:11) - PLID 32322 - track this to output at the end of the function
			strPaymentDate = FormatDateTimeForInterface(dt, NULL, dtoDate);
		}
	}

	//Payee Name					30 / 30 / X				Name of Payee as registered with MOH, solo providers displays as Last Name (25), Title (3), Initials (2)

	CString strPayeeName = ParseElement(strLine, 30, 30, TRUE);
	CString strLast = ParseElement(strPayeeName, 1, 25);
	CString strTitle = ParseElement(strPayeeName, 26, 3);
	CString strInit = ParseElement(strPayeeName, 29, 2);

	CString strPayeeNameToOutput;
	if(!strPayeeName.IsEmpty()) {

		//if the last name is shorter than 25 characters and we have an initial or a title,
		//format as a name, otherwise assume it is a group name and output the entire line
		if(strLast.GetLength() < 25 && (!strTitle.IsEmpty() || !strInit.IsEmpty())) {
			//format as title, init, last
			strPayeeNameToOutput.Format("%s %s %s", strTitle, strInit, strLast);
		}
		else {
			//send the whole payee name
			strPayeeNameToOutput = strPayeeName;
		}

		strPayeeNameToOutput.TrimLeft();
		strPayeeNameToOutput.TrimRight();

		if(!strPayeeNameToOutput.IsEmpty()) {
			ptrOHIPEOBInfo->strPayeeName = strPayeeNameToOutput;
		}
	}
	
	//Total Amount Payable			60 / 9 / N				Accumulation of amt. paid for all claim items in the remit file, +/- any accounting transactions and balance forward amounts

	CString strTotalPayable = ParseElement(strLine, 60, 9);

	//Total Amount Payable Sign		69 / 1 / S or X			Space if total amt. payable is positive, a - sign if negative

	CString strTotalPayableSign = ParseElement(strLine, 69, 1);

	COleCurrency cyTotalAmountPayable = ParseOHIPCurrency(strTotalPayable, strTotalPayableSign);
	ptrOHIPEOBInfo->cyTotalPaymentAmt = cyTotalAmountPayable;

	//Cheque Number					70 / 8 / X				If Pay To Provider, cheCK number or all 9s if direct deposit, Is Pay to Patient it is all spaces (Cheque, really?)

	CString strCheckNumber = ParseElement(strLine, 70, 8);

	if(!strCheckNumber.IsEmpty()) {
		ptrOHIPEOBInfo->strCheckNumber = strCheckNumber;
	}

	//Reserved for MOH Use			78 / 2 / S				Spaces

	CString strReserved2 = ParseElement(strLine, 78, 2);

	// (j.jones 2008-12-15 11:07) - PLID 32322 - moved all the output down here so we can output cleanly
	// in the order and layout of our choosing as opposed to the order the data appears in the file

	CString strPaymentLine;
	strPaymentLine.Format("Payment Amount Received: %s", FormatCurrencyForInterface(cyTotalAmountPayable, TRUE, TRUE));

	if(!strPaymentDate.IsEmpty()) {
		str.Format("\tPayment Date: %s", strPaymentDate);
		strPaymentLine += str;
	}

	if(!strCheckNumber.IsEmpty()) {

		//see if it is direct deposit
		if(strCheckNumber == "99999999") {
			str = "\tCheque Number: <Direct Deposit>";
		}
		else {
			str.Format("\tCheque Number: %s", strCheckNumber);
		}
		strPaymentLine += str;
	}
	
	//add a blank line
	strPaymentLine += "\r\n\r\n";
	OutputData(strOutputString, strPaymentLine, NULL);

	CString strPayeeLine;

	if(!strPayeeNameToOutput.IsEmpty()) {
		strPayeeLine.Format("Payee Name: %s", strPayeeNameToOutput);		
	}

	if(!strProvNum.IsEmpty()) {
		str.Format("Provider Number: %s", strProvNum);
		if(!strPayeeLine.IsEmpty()) {
			strPayeeLine += "\t";
		}
		strPayeeLine += str;
	}

	if(!strGroupNum.IsEmpty()) {
		str.Format("Group Number: %s", strGroupNum);
		if(!strPayeeLine.IsEmpty()) {
			strPayeeLine += "\t";
		}
		strPayeeLine += str;
	}

	if(!strPayeeLine.IsEmpty()) {
		strPayeeLine += "\r\n";
		OutputData(strOutputString, strPayeeLine, NULL);
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//Name and address Line 1 of billing agent or health care provider (once per file)
void COHIPERemitParser::AddressRecord1_2(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '2'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Billing Agent's Name			4 / 30 / X				Spaces if no Billing Agent registered for the provider

	CString strBillingName = ParseElement(strLine, 4, 30);

	//Address Line 1				34 / 25 / X				Address 1 of the provider or billing agent

	CString strAddress1 = ParseElement(strLine, 34, 25);

	if(!strAddress1.IsEmpty()) {
		str.Format("Provider Address:\r\n%s\r\n", strAddress1);
		OutputData(strOutputString, str, NULL);
	}

	//Reserved for MOH Use			59 / 21 / S				Spaces

	CString strReserved = ParseElement(strLine, 59, 21);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//Name and address Line 2 and 3 of billing agent or health care provider (once per file)
void COHIPERemitParser::AddressRecord2_3(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '3'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Address Line 2				4 / 25 / X				Address 2 as registered with MOH

	CString strAddress2 = ParseElement(strLine, 4, 25);

	if(!strAddress2.IsEmpty()) {
		str.Format("%s\r\n", strAddress2);
		OutputData(strOutputString, str, NULL);
	}

	//Address Line 3				29 / 25 / X				Address 3 as registered with MOH

	CString strAddress3 = ParseElement(strLine, 29, 25);

	if(!strAddress3.IsEmpty()) {
		str.Format("%s\r\n", strAddress3);
		OutputData(strOutputString, str, NULL);
	}

	//Reserved for MOH Use			54 / 26 / S				Spaces

	CString strReserved = ParseElement(strLine, 54, 26);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//Common control information for each claim
void COHIPERemitParser::ClaimHeader_4(CString strLine)
{
	// (j.jones 2012-10-04 15:35) - PLID 52929 - I changed OutputString to require a claim pointer.
	// NULL if we do not have one. In this function, all calls to OutputString will send a pointer.

	CString strOutputString, str;

	OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();

	// (j.jones 2010-01-27 11:32) - PLID 36998 - check whether the last claim had any services on it
	if(pEOBInfo->paryEOBClaimInfo.GetSize() > 0) {
		OHIPEOBClaimInfo *pLastInfo = (OHIPEOBClaimInfo*)pEOBInfo->paryEOBClaimInfo.GetAt(pEOBInfo->paryEOBClaimInfo.GetSize() - 1);
		if(pLastInfo->paryOHIPEOBLineItemInfo.GetSize() == 0) {
			//this is not good, the EOB had a claim listed that had no services,
			//we need to output a warning
			CString strFmt;
			strFmt.Format("-----------------------------------------------------------------------------------\r\n"
						  "****WARNING: The remit file did not report any services for the above claim!\r\n"
						  "You must contact OHIP to receive a corrected remit file.\r\n"
						  "No posting will be made for the claim listed above without a corrected remit file.\r\n"
						  "-----------------------------------------------------------------------------------\r\n\r\n");
			OutputData(strOutputString, strFmt, pLastInfo);
		}
	}

	OHIPEOBClaimInfo *pClaimInfo = GenerateNewEOBClaimInfo();

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '4'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Claim Number					4 / 11 / X				MOH Reference Number

	CString strClaimNumber = ParseElement(strLine, 4, 11);

	if(!strClaimNumber.IsEmpty()) {
		pClaimInfo->strClaimNumber = strClaimNumber;
	}

	//Transaction Type				15 / 1 / N				1 (original claim) or 2 (adjustment to original claim)

	CString strTransactionType = ParseElement(strLine, 15, 1);

	//Health Care Provider Number	16 / 6 / N

	CString strProvNum = ParseElement(strLine, 16, 6);

	if(!strProvNum.IsEmpty()) {
		pClaimInfo->strProvNumber = strProvNum;
	}

	//Specialty						22 / 2 / N				Health Care Provider's Specialty Code, as on Health Claim Header 1

	CString strSpecialty = ParseElement(strLine, 22, 2);

	//Accounting Number				24 / 8 / X				Accounting Number, as on Health Claim Header 1

	CString strAccountingNum = ParseElement(strLine, 24, 8);

	if(!strAccountingNum.IsEmpty()) {
		pClaimInfo->strAccountingNumber = strAccountingNum;
	}

	//Patient's Last Name			32 / 14 / S or A		Spaces except for RMB claims

	CString strPatLast = ParseElement(strLine, 32, 14);

	//Patient's First Name			46 / 5 / S or A			Spaces except for RMB claims (first 5 characters only)

	CString strPatFirst = ParseElement(strLine, 46, 5);

	//Province Code					51 / 2 / A				Refer to Appendix F - Province Codes

	CString strProvince = ParseElement(strLine, 51, 2);

	//Health Registration Number	53 / 12 / X				Left Justified

	CString strHealthRegNum = ParseElement(strLine, 53, 12);

	if(!strHealthRegNum.IsEmpty()) {
		pClaimInfo->strHealthRegistrationNumber = strHealthRegNum;
	}

	//Version Code					65 / 2 / A				Version code as on Health Encounter Claim Header 1

	CString strVersionCode = ParseElement(strLine, 65, 2);

	if(!strVersionCode.IsEmpty()) {
		pClaimInfo->strVersionCode = strVersionCode;
	}

	//Payment Program				67 / 3 / A				Payment Program as on Health Encounter Claim Header 1

	CString strPaymentProgram = ParseElement(strLine, 67, 3);

	if(!strPaymentProgram.IsEmpty()) {
		pClaimInfo->strPaymentProgram = strPaymentProgram;
	}

	//Location Code					70 / 4 / N or S			4 numerics or spaces, Location Code as on Health Encounter Claim Header 1

	CString strLocationCode = ParseElement(strLine, 70, 4);

	if(!strLocationCode.IsEmpty()) {
		pClaimInfo->strLocationCode = strLocationCode;
	}

	//Reserved for MOH Use			74 / 6 / S				Spaces

	CString strReserved = ParseElement(strLine, 74, 6);

	// (j.jones 2008-12-15 12:06) - PLID 32322 - moved all the output down here so we can output cleanly
	// in the order and layout of our choosing as opposed to the order the data appears in the file

	//calculate the bill & patient IDs now, so we can also get the patient name
	long nBillID = -1;
	long nPatientID = -1;
	CString strPatientName = "";
	// (b.spivey, October 01, 2012) - PLID 52666 - Pass in the OHIP's Health Reg #
	if(!CalcBillAndPatientID(strAccountingNum, strHealthRegNum, nBillID, nPatientID, strPatientName)) {
		//if this failed, output <Unknown Patient> for patient name
		strPatientName = "<Unknown Patient>";
	}

	//we're now responsible for assigning these IDs to pClaimInfo because we called
	//CalcBillAndPatientID, instead of CalcInternalIDs
	pClaimInfo->nBillID = nBillID;
	pClaimInfo->nPatientID = nPatientID;
	
	//this is now the first line output for a claim (add a blank line before and after this line)
	OutputData(strOutputString, "\r\n======================================================================================================\r\n\r\n", pClaimInfo);

	{
		CString strPatientLine1;
		strPatientLine1.Format("Patient Name: %s", strPatientName);

		if(strPatientName.GetLength() < 14) {
			strPatientLine1 += "\t";
		}

		if(!strAccountingNum.IsEmpty()) {
			str.Format("\tBill ID: %s", strAccountingNum);
			strPatientLine1 += str;
		}

		strPatientLine1 += "\r\n";
		OutputData(strOutputString, strPatientLine1, pClaimInfo);
	}

	{
		CString strPatientLine2;

		if(!strHealthRegNum.IsEmpty()) {
			//add a tab after this
			str.Format("Health Number: %s\t", strHealthRegNum);
			strPatientLine2 += str;
		}

		if(!strVersionCode.IsEmpty()) {
			str.Format("Version Code: %s", strVersionCode);
			if(!strPatientLine2.IsEmpty()) {
				strPatientLine2 += "\t";
			}
			strPatientLine2 += str;
		}

		if(!strProvince.IsEmpty()) {
			str.Format("Province: %s\r\n", strProvince);
			if(!strPatientLine2.IsEmpty()) {
				strPatientLine2 += "\t";
			}
			strPatientLine2 += str;
		}

		strPatientLine2 += "\r\n";
		OutputData(strOutputString, strPatientLine2, pClaimInfo);
	}
	
	//this would duplicate the patient name on the EOB, if an RMB claim
	//(which is unlikely to happen), but in this case we'd be showing
	//what name that MOH thinks is correct
	if(!strPatLast.IsEmpty() || !strPatFirst.IsEmpty()) {
		CString strRMBName;
		strRMBName.Format("Patient Name (RMB Claim): %s, %s\r\n", strPatLast, strPatFirst);
		OutputData(strOutputString, strRMBName, pClaimInfo);
	}

	{
		CString strClaimLine;

		long nTxType = atoi(strTransactionType);
		if(nTxType == 1) {
			strClaimLine = "Transaction Type: 1 - Original Claim";
		}
		else if(nTxType == 2) {
			strClaimLine = "Transaction Type: 2 - Adjustment To Original Claim";
		}

		if(!strClaimNumber.IsEmpty()) {
			str.Format("Claim Number: %s", strClaimNumber);
			if(!strClaimLine.IsEmpty()) {
				strClaimLine += "\t";
			}
			strClaimLine += str;
		}

		strClaimLine += "\r\n";
		OutputData(strOutputString, strClaimLine, pClaimInfo);
	}

	{
		CString strProviderLine;

		if(!strProvNum.IsEmpty()) {
			str.Format("Health Care Provider Number: %s", strProvNum);
			strProviderLine += str;
		}

		if(!strSpecialty.IsEmpty()) {
			str.Format("Specialty: %s", strSpecialty);
			if(!strProviderLine.IsEmpty()) {
				strProviderLine += "\t";
			}
			strProviderLine += str;
		}

		strProviderLine += "\r\n";
		OutputData(strOutputString, strProviderLine, pClaimInfo);
	}

	{
		CString strPaymentProgramLine;
		if(!strPaymentProgram.IsEmpty()) {
			str.Format("Payment Program: %s", strPaymentProgram);
			strPaymentProgramLine += str;
		}

		if(!strLocationCode.IsEmpty()) {
			str.Format("Location Code: %s", strLocationCode);
			if(!strPaymentProgramLine.IsEmpty()) {
				strPaymentProgramLine += "\t";
			}
			strPaymentProgramLine += str;
		}

		strPaymentProgramLine += "\r\n";
		OutputData(strOutputString, strPaymentProgramLine, pClaimInfo);
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//Detailed information for each item of service within a claim (code, date, amounts)
void COHIPERemitParser::ClaimItem_5(CString strLine)
{
	// (j.jones 2012-10-04 15:35) - PLID 52929 - I changed OutputString to require a claim pointer.
	// NULL if we do not have one. In this function, all calls to OutputString will send a pointer.

	CString strOutputString, str;

	OHIPEOBClaimInfo *pClaimInfo = GetCurrentClaimInfo();
	OHIPEOBLineItemInfo *pLineItemInfo = GenerateNewEOBLineItemInfo();

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '5'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Claim Number					4 / 11 / X				MOH Reference Number

	CString strClaimNumber = ParseElement(strLine, 4, 11);

	//Transaction Type				15 / 1 / N				1 (original claim) or 2 (adjustment to original claim)

	CString strTransactionType = ParseElement(strLine, 15, 1);

	//Service Date					16 / 8 / D				Service Date as on Health Encounter Item Record

	CString strServiceDate = ParseElement(strLine, 16, 8);

	CString strServiceDateFmt;
	if(strServiceDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strServiceDate.Left(4)), atoi(strServiceDate.Mid(4, 2)), atoi(strServiceDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			strServiceDateFmt = FormatDateTimeForInterface(dt, NULL, dtoDate);

			pLineItemInfo->dtServiceDate = dt;
			pLineItemInfo->bChargeDatePresent = TRUE;
		}
	}

	//Number of Services			24 / 2 / N				Number of Services as on Health Encounter Item Record

	CString strNumServices = ParseElement(strLine, 24, 2);

	if(!strNumServices.IsEmpty()) {
		pLineItemInfo->nNumberOfServices = atoi(strNumServices);
	}

	//Service Code					26 / 5 / X

	CString strServiceCode = ParseElement(strLine, 26, 5);

	if(!strServiceCode.IsEmpty()) {
		pLineItemInfo->strServiceCode = strServiceCode;
	}

	//Reserved for MOH Use			31 / 1 / S				Spaces

	CString strReserved1 = ParseElement(strLine, 31, 1);

	//Amount Submitted				32 / 6 / N				Amount submitted as on Health Encounter Item Record

	CString strAmtSubmitted = ParseElement(strLine, 32, 6);

	COleCurrency cyAmountSubmitted = ParseOHIPCurrency(strAmtSubmitted, "");

	pLineItemInfo->cyLineItemChargeAmt = cyAmountSubmitted;

	//Amount Paid					38 / 6 / N

	CString strAmtPaid = ParseElement(strLine, 38, 6);

	//Amount Paid Sign				44 / 1 / S or X			Space if amt. paid is positive, "-" if amt. paid is negative

	CString strAmtPaidSign = ParseElement(strLine, 44, 1);

	COleCurrency cyAmtPaid = ParseOHIPCurrency(strAmtPaid, strAmtPaidSign);

	pLineItemInfo->cyLineItemPaymentAmt = cyAmtPaid;

	//Explanatory Code				45 / 2 / X				Refer to Remittance Advice Explanatory Codes

	CString strExplanatoryCode = ParseElement(strLine, 45, 2);

	pLineItemInfo->strExplanatoryCode = strExplanatoryCode;

	// (j.jones 2008-12-15 16:10) - PLID 32329 - look up what the code description is, and output that (after the service line)
	CString strExplanatoryDescription = GetExplanatoryDescriptionFromCode(strExplanatoryCode);
	pLineItemInfo->strExplanatoryDescription = strExplanatoryDescription;

	//if 35, set the duplicate charge flag to TRUE
	if(strExplanatoryCode == "35") {
		pLineItemInfo->bDuplicateCharge = TRUE;
	}

	//Reserved for MOH Use			47 / 33 / S				Spaces

	CString strReserved2 = ParseElement(strLine, 47, 33);

	// (j.jones 2008-12-15 14:24) - PLID 32322 - moved all the output down here so we can output cleanly
	// in the order and layout of our choosing as opposed to the order the data appears in the file

	{
		CString strServiceLine1;
		//add a blank line before each service

		strServiceLine1.Format("\r\nService Code: %s", strServiceCode);

		if(!strNumServices.IsEmpty()) {
			str.Format("\tNumber Of Services: %s", strNumServices);
			strServiceLine1 += str;
		}

		if(!strServiceDateFmt.IsEmpty()) {
			str.Format("\tService Date: %s", strServiceDateFmt);
			strServiceLine1 += str;
		}

		strServiceLine1 += "\r\n";
		OutputData(strOutputString, strServiceLine1, pClaimInfo);
	}

	{
		CString strServiceLine2;
		strServiceLine2.Format("Amount Submitted: %s\tAmount Paid: %s\r\n", FormatCurrencyForInterface(cyAmountSubmitted, TRUE, TRUE),
			FormatCurrencyForInterface(cyAmtPaid, TRUE, TRUE));

		OutputData(strOutputString, strServiceLine2, pClaimInfo);
	}

	// (j.jones 2008-12-15 17:12) - PLID 32329 - output the explanatory code and description, if we have one
	if(!strExplanatoryCode.IsEmpty()) {
		CString strExplanatoryLine;
		if(strExplanatoryDescription.IsEmpty()) {
			strExplanatoryDescription = "<Unknown Explanatory Code>";
		}
		//insert a blank line
		strExplanatoryLine.Format("\r\nExplanatory Code: %s - %s\r\n", strExplanatoryCode, strExplanatoryDescription);
		OutputData(strOutputString, strExplanatoryLine, pClaimInfo);
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//This record is present only if the previous month's remittance was NEGATIVE.
void COHIPERemitParser::BalanceForward_6(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '6'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Amount Brought Forward -		4 / 9 / N				This field will contain the value other than zeros when the Total Remittance Payable
	//Claims Adjustment										does not exceed the total debit items for adjusted claims. This amount is always negative.

	CString strABFClaimsAdjustment = ParseElement(strLine, 4, 9);

	//Amount Brought Forward -		13 / 1 / X				Space if the Claims Adjustment field contains zeros, otherwise "-"
	//Claims Adjustment Sign

	CString strABFClaimsAdjustmentSign = ParseElement(strLine, 13, 1);

	COleCurrency cyABFClaimsAdjustment = ParseOHIPCurrency(strABFClaimsAdjustment, strABFClaimsAdjustmentSign);

	// (j.jones 2008-12-15 17:12) - PLID 32329 - continue to output one line for each ABF

	if(cyABFClaimsAdjustment != COleCurrency(0,0)) {

		// (j.jones 2009-04-03 11:24) - PLID 33851 - we previously thought that this adjustment was per claim, but it apparently
		// is per office, independent of patients, so we need a separator for each adjustment
		OutputData(strOutputString, "\r\n========================================= Amount Brought Forward ===========================================\r\n\r\n", NULL);

		str.Format("Amount Brought Forward - Claims Adjustment: %s\r\n", FormatCurrencyForInterface(cyABFClaimsAdjustment, TRUE, TRUE));
		OutputData(strOutputString, str, NULL);
	}

	//Amount Brought Forward -		14 / 9 / N				This field will contain a value other than zeros when a Record Type 7 (Transaction Code 10 - Advance)
	//Advances												on a previous Remittance Advice fails to recover the full value of an advance. Always negative.

	CString strABFAdvances = ParseElement(strLine, 14, 9);

	//Amount Brought Forward -		23 / 1 / S or X			Space if the Advances field contains zeros, otherwise "-"
	//Advances Sign

	CString strABFAdvancesSign = ParseElement(strLine, 23, 1);

	COleCurrency cyABFAdvances = ParseOHIPCurrency(strABFAdvances, strABFAdvancesSign);

	if(cyABFAdvances != COleCurrency(0,0)) {

		// (j.jones 2009-04-03 11:24) - PLID 33851 - we previously thought that this adjustment was per claim, but it apparently
		// is per office, independent of patients, so we need a separator for each adjustment
		OutputData(strOutputString, "\r\n========================================= Amount Brought Forward ===========================================\r\n\r\n", NULL);

		str.Format("Amount Brought Forward - Advances: %s\r\n", FormatCurrencyForInterface(cyABFAdvances, TRUE, TRUE));
		OutputData(strOutputString, str, NULL);
	}

	//Amount Brought Forward -		24 / 9 / N				This field will contain a value other than zeros when a Record Type 7 (Transaction Code 20 - Reduction)
	//Reductions											on a previous Remittance Advice cannot be satisfied by the Total Remittance Payable. Always negative.

	CString strABFReductions = ParseElement(strLine, 24, 9);

	//Amount Brought Forward -		33 / 1 / S or X			Space if the Reductions field contains zeros, otherwise "-"
	//Reductions Sign

	CString strABFReductionsSign = ParseElement(strLine, 33, 1);

	COleCurrency cyABFReductions = ParseOHIPCurrency(strABFReductions, strABFReductionsSign);

	if(cyABFReductions != COleCurrency(0,0)) {

		// (j.jones 2009-04-03 11:24) - PLID 33851 - we previously thought that this adjustment was per claim, but it apparently
		// is per office, independent of patients, so we need a separator for each adjustment
		OutputData(strOutputString, "\r\n========================================= Amount Brought Forward ===========================================\r\n\r\n", NULL);

		str.Format("Amount Brought Forward - Reductions: %s\r\n", FormatCurrencyForInterface(cyABFReductions, TRUE, TRUE));
		OutputData(strOutputString, str, NULL);
	}

	//Amount Brought Forward -		34 / 9 / N				For future use, presently zero-filled.
	//Other Deductions

	CString strABFOtherDeductions = ParseElement(strLine, 34, 9);

	//Amount Brought Forward -		43 / 1 / S or X			For future use, presently a space.
	//Other Deductions Sign

	CString strABFOtherDeductionsSign = ParseElement(strLine, 43, 1);

	COleCurrency cyABFOtherDeductions = ParseOHIPCurrency(strABFOtherDeductions, strABFOtherDeductionsSign);

	if(cyABFOtherDeductions != COleCurrency(0,0)) {

		// (j.jones 2009-04-03 11:24) - PLID 33851 - we previously thought that this adjustment was per claim, but it apparently
		// is per office, independent of patients, so we need a separator for each adjustment
		OutputData(strOutputString, "\r\n========================================= Amount Brought Forward ===========================================\r\n\r\n", NULL);

		str.Format("Amount Brought Forward - Other Deductions: %s\r\n", FormatCurrencyForInterface(cyABFOtherDeductions, TRUE, TRUE));
		OutputData(strOutputString, str, NULL);
	}

	//Reserved for MOH Use			44 / 36 / S				Spaces

	CString strReserved2 = ParseElement(strLine, 44, 36);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//This record is present only if an accounting transaction is posted to the remittance advice (advance, reduction, advance repayment)
void COHIPERemitParser::AccountingTransaction_7(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '7'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Transaction Code				4 / 2 / X				10 - Advance
	//														20 - Reduction
	//														30 - Unused
	//														40 - Advance Repayment
	//														50 - Accounting Adjustment
	//														70 - Attachments

	CString strTransactionCode = ParseElement(strLine, 4, 2);
	strTransactionCode.TrimLeft();
	strTransactionCode.TrimRight();

	long nTxCode = atoi(strTransactionCode);

	CString strReason;
	CString strReasonOutput;

	switch(nTxCode) {
	case 10:
		strReasonOutput = "Transaction Code: 10 - Advance";
		strReason = "Advance";
		break;
	case 20:
		strReasonOutput = "Transaction Code: 20 - Reduction";
		strReason = "Reduction";
		break;
	case 30:
		strReasonOutput = "Transaction Code: 30 - Unused";
		strReason = "Unused";
		break;
	case 40:
		strReasonOutput = "Transaction Code: 40 - Advance Repayment";
		strReason = "Advance Repayment";
		break;
	case 50:
		strReasonOutput = "Transaction Code: 50 - Accounting Adjustment";
		strReason = "Accounting Adjustment";
		break;
	case 70:
		strReasonOutput = "Transaction Code: 70 - Attachments";
		strReason = "Attachments";
		break;
	}

	//Cheque Indicator				6 / 1 / X

	CString strCheckIndicator = ParseElement(strLine, 6, 1);

	//Transaction Date				7 / 8 / D				Date of transaction created

	CString strTransactionDate = ParseElement(strLine, 7, 8);

	COleDateTime dtDate;
	dtDate.SetStatus(COleDateTime::invalid);

	CString strTxDateFmt;

	if(strTransactionDate.GetLength() == 8) {
		
		dtDate.SetDate(atoi(strTransactionDate.Left(4)), atoi(strTransactionDate.Mid(4, 2)), atoi(strTransactionDate.Right(2)));
		if(dtDate.GetStatus() != COleDateTime::invalid) {
			strTxDateFmt.Format("Transaction Date: %s", FormatDateTimeForInterface(dtDate, NULL, dtoDate));
		}
	}

	//Transaction Amount			15 / 8 / N

	CString strTransactionAmt = ParseElement(strLine, 15, 8);

	//Transaction Amount Sign		23 / 1 / S or X			Space if Transaction Amount is positive, otherwise "-"

	CString strTransactionAmtSign = ParseElement(strLine, 23, 1);

	COleCurrency cyTransactionAmt = ParseOHIPCurrency(strTransactionAmt, strTransactionAmtSign);

	//Transaction Message			24 / 50 / S or X		Description of transaction

	CString strTransactionMessage = ParseElement(strLine, 24, 50);

	// (j.jones 2008-12-16 09:26) - PLID 32334 - do NOT add an adjustment, this is not a patient adjustment,
	// if anything this would be a batch payment adjustment, but for now we are not processing this value
	// (j.jones 2009-09-25 10:39) - PLID 34453 - now we are tracking these values as accounting adjustments,
	// we are not altering the batch payment total, merely storing this information in data
	OHIPEOBAccountingTransaction *pNewTx = GenerateNewEOBAccountingTransaction();
	pNewTx->dtDate = dtDate;
	pNewTx->strReasonCode = strTransactionCode;
	pNewTx->strReasonDesc = strReason;
	pNewTx->strMessage = strTransactionMessage;
	pNewTx->cyAmount = cyTransactionAmt;

	//Reserved for MOH Use			74 / 6 / S				Spaces

	CString strReserved = ParseElement(strLine, 74, 6);

	// (j.jones 2008-12-16 09:18) - PLID 32334 - we previously thought that this adjustment was per claim, but it apparently
	// is per office, independent of patients, so we need a separator for each adjustment
	OutputData(strOutputString, "\r\n========================================= Accounting Adjustment ===========================================\r\n\r\n", NULL);

	{
		CString strReasonLine;

		if(!strReasonOutput.IsEmpty()) {
			strReasonLine = strReasonOutput;
		}

		if(!strCheckIndicator.IsEmpty()) {
			str.Format("Check Indicator: %s", strCheckIndicator);
			if(!strReasonLine.IsEmpty()) {
				strReasonLine += "\t";
			}
			strReasonLine += str;
		}

		if(!strReasonLine.IsEmpty()) {
			strReasonLine += "\r\n";
			OutputData(strOutputString, strReasonLine, NULL);
		}
	}

	{
		CString strTransactionDateLine;

		if(!strTxDateFmt.IsEmpty()) {
			strTransactionDateLine = strTxDateFmt;
		}

		if(!strTransactionMessage.IsEmpty()) {
			str.Format("Transaction Message: %s", strTransactionMessage);
			if(!strTransactionDateLine.IsEmpty()) {
				strTransactionDateLine += "\t";
			}
			strTransactionDateLine += str;
		}

		if(!strTransactionDateLine.IsEmpty()) {
			strTransactionDateLine += "\r\n";
			OutputData(strOutputString, strTransactionDateLine, NULL);
		}
	}

	//put the transaction amount on its own line
	str.Format("Transaction Amount: %s\r\n", FormatCurrencyForInterface(cyTransactionAmt, TRUE, TRUE));
	OutputData(strOutputString, str, NULL);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//A facility for the MOH to send messages to health care providers. May or may not be present. May have up to 99,999 occurrences.
void COHIPERemitParser::MessageFacility_8(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / A				Always 'HR'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Type					3 / 1 / X				Always '8'

	CString strRecordType = ParseElement(strLine, 3, 1);

	//Message Text					4 / 70 / X				Message (contains upper and lower case)

	CString strMessageText = ParseElement(strLine, 4, 70);

	//don't export yet, just append to the memory object

	if(!strMessageText.IsEmpty()) {
		OHIPEOBInfo *pEOBInfo = GetCurrentEOBInfo();
		pEOBInfo->arystrMessages.Add(strMessageText);
	}

	//Reserved for MOH Use			74 / 6 / S				Spaces

	CString strReserved = ParseElement(strLine, 74, 6);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

// (j.jones 2008-06-17 17:31) - PLID 21921 - tries to associate reported claims and charges with internal bill, patient, and charge IDs
void COHIPERemitParser::CalcInternalIDs(OHIPEOBInfo *ptrOHIPEOBInfo) {

	//attempt to associate claims with patients and bills, line items with charges, and insurance companies

	m_ptrProgressBar->SetStep(1);

	//Step 1. Assign Patient IDs

	m_ptrProgressBar->SetPos(0);

	m_ptrProgressBar->SetRange(0,ptrOHIPEOBInfo->paryEOBClaimInfo.GetSize());

	CString str;
	if(m_CountOfEOBs == 1) {
		str = "Matching Charges...";
	}
	else {
		str.Format("Matching Charges (EOB %li)...", ptrOHIPEOBInfo->nIndex+1);
	}
	m_ptrProgressStatus->SetWindowText(str);

	//loop through all claims
	int i = 0;
	for(i=0; i<ptrOHIPEOBInfo->paryEOBClaimInfo.GetSize(); i++) {

		OHIPEOBClaimInfo *pClaimInfo = (OHIPEOBClaimInfo*)ptrOHIPEOBInfo->paryEOBClaimInfo.GetAt(i);

		// (j.jones 2008-12-15 12:23) - PLID 32322 - I moved the BillID and PatientID code into
		// CalcBillAndPatientID, which is now called while parsing, meaning that pClaimInfo->nBillID
		// and pClaimInfo->nPatientID should be filled in prior to this function being called, and
		// -1 means we didn't find matching records

		if(pClaimInfo->nBillID != -1) {

			//loop through all charges
			for(int j=0; j<pClaimInfo->paryOHIPEOBLineItemInfo.GetSize(); j++) {

				OHIPEOBLineItemInfo *pLineItemInfo = (OHIPEOBLineItemInfo*)pClaimInfo->paryOHIPEOBLineItemInfo.GetAt(j);

				//ensure this ID starts out as -1
				pLineItemInfo->nChargeID = -1;
				pLineItemInfo->nProviderID = -1;

				//now figure out which charge this is

				CString strDateWhere = "";
				if(pLineItemInfo->bChargeDatePresent) {
					strDateWhere.Format("AND (CONVERT(datetime,CONVERT(nvarchar, ChargesT.ServiceDateFrom, 101))) = (CONVERT(datetime,CONVERT(nvarchar, '%s', 101)))",FormatDateTimeForSql(pLineItemInfo->dtServiceDate, dtoDate));
				}

				//with the optional where clause, this cannot be parameterized
				// (j.jones 2010-04-27 13:58) - PLID 34216 - now we match by item code first,
				// but can accept a charge of the same total if we can't match by item code
				_RecordsetPtr rsCharges = CreateRecordset("SELECT ChargesT.ID, ChargesT.DoctorsProviders "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE BillID = %li AND LineItemT.Deleted = 0 "
					"AND (dbo.GetChargeTotal(ChargesT.ID) = Convert(money,'%s') OR LineItemT.Amount = Convert(money,'%s')) "
					"%s "
					"ORDER BY CASE WHEN ItemCode = '%s' THEN 0 ELSE 1 END ASC, ChargesT.ID ASC",
					pClaimInfo->nBillID,
					FormatCurrencyForSql(pLineItemInfo->cyLineItemChargeAmt),
					FormatCurrencyForSql(pLineItemInfo->cyLineItemChargeAmt),
					strDateWhere,
					_Q(pLineItemInfo->strServiceCode));

				long nFirstChargeID = -1;
				long nFirstProviderID = -1;
				while(!rsCharges->eof && pLineItemInfo->nChargeID == -1) {

					long nChargeID = AdoFldLong(rsCharges, "ID",-1);
					long nProviderID = AdoFldLong(rsCharges, "DoctorsProviders",-1);

					//save the first ID incase we can't get one that hasn't already been used
					if(nFirstChargeID == -1) {
						nFirstChargeID = nChargeID;
						nFirstProviderID = nProviderID;
					}

					//see if we have already used this charge
					BOOL bUsed = FALSE;
					for(int y=0; y<ptrOHIPEOBInfo->paryEOBClaimInfo.GetSize(); y++) {
						OHIPEOBClaimInfo *pClaimInfo2 = (OHIPEOBClaimInfo*)ptrOHIPEOBInfo->paryEOBClaimInfo.GetAt(y);
						//loop through all charges
						for(int z=0; z<pClaimInfo2->paryOHIPEOBLineItemInfo.GetSize(); z++) {
							OHIPEOBLineItemInfo *pLineItemInfo2 = (OHIPEOBLineItemInfo*)pClaimInfo2->paryOHIPEOBLineItemInfo.GetAt(z);
							if(pLineItemInfo2->nChargeID == nChargeID) {
								bUsed = TRUE;
							}
						}
					}

					//if not already used, take it!
					if(!bUsed) {
						pLineItemInfo->nChargeID = nChargeID;
						pLineItemInfo->nProviderID = nProviderID;
					}

					rsCharges->MoveNext();
				}

				//if we didn't get a unique one, use what was found anyways
				if(pLineItemInfo->nChargeID == -1) {
					pLineItemInfo->nChargeID = nFirstChargeID;
					pLineItemInfo->nProviderID = nFirstProviderID;
				}

				rsCharges->Close();

				// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
				// function, as we may have disabled this ability
				PeekAndPump_OHIPERemitParser();
			}
		}

		m_ptrProgressBar->StepIt();

		// (j.jones 2010-03-15 10:48) - PLID 32184 - call our local PeekAndPump
		// function, as we may have disabled this ability
		PeekAndPump_OHIPERemitParser();
	}

	//Step 2. Assign Insurance Co ID (uses Patient ID) and Insured Party IDs
	CalcInsuranceIDs(ptrOHIPEOBInfo);
}

// (j.jones 2008-06-17 17:31) - PLID 21921 - tries to determine the Insurance Co ID from the EOB, and insured party IDs
void COHIPERemitParser::CalcInsuranceIDs(OHIPEOBInfo *ptrEOBInfo)
{
	CString str;
	if(m_CountOfEOBs == 1) {
		str = "Cross-referencing insured parties...";
	}
	else {
		str.Format("Cross-referencing insured parties (EOB %li)...",ptrEOBInfo->nIndex+1);
	}
	m_ptrProgressStatus->SetWindowText(str);

	m_ptrProgressBar->SetPos(0);
	m_ptrProgressBar->SetRange(0,ptrEOBInfo->paryEOBClaimInfo.GetSize());

	//initialize to -1
	ptrEOBInfo->nLikelyInsuranceCoID = -1;

	CString strPatientPlans;
	CString strPatientIDs;

	//loop through all claims
	int i = 0;
	for(i=0; i<ptrEOBInfo->paryEOBClaimInfo.GetSize(); i++) {

		OHIPEOBClaimInfo *pClaimInfo = (OHIPEOBClaimInfo*)ptrEOBInfo->paryEOBClaimInfo.GetAt(i);

		if(pClaimInfo->nPatientID != -1) {

			//We're building two complex CASE statements here, one is going to give us a tally
			//of all insured parties matching our patient ID and plan name, the other will
			//give us a tally of all matching by patient ID only. Highest tally wins.
			
			CString str;
			str.Format("(PatientID = %li AND InsurancePlansT.PlanName = '%s')", pClaimInfo->nPatientID, pClaimInfo->strPaymentProgram);
			if(!strPatientPlans.IsEmpty()) {
				strPatientPlans += " OR ";
			}
			strPatientPlans += str;	
			
			str.Format("PatientID = %li", pClaimInfo->nPatientID);
			if(!strPatientIDs.IsEmpty()) {
				strPatientIDs += " OR ";
			}
			strPatientIDs += str;
		}

		m_ptrProgressBar->StepIt();
	}

	if(!strPatientPlans.IsEmpty() && !strPatientIDs.IsEmpty()) {

		//the where clause here makes it such that we can't parameterize this query
		_RecordsetPtr rs = CreateRecordset("SELECT InsuranceCoID, "
			"Sum(CASE WHEN %s THEN 1 ELSE 0 END) AS TotalPatientPlans, "
			"Sum(CASE WHEN %s THEN 1 ELSE 0 END) AS TotalPatients "
			"FROM InsuredPartyT "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"GROUP BY InsuranceCoID "
			"ORDER BY Sum(CASE WHEN %s THEN 1 ELSE 0 END) DESC, "
			"Sum(CASE WHEN %s THEN 1 ELSE 0 END) DESC",
			strPatientPlans, strPatientIDs, strPatientPlans, strPatientIDs);

		if(!rs->eof) {

			long nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID", -1);
			long nTotalPatientPlans = AdoFldLong(rs, "TotalPatientPlans", 0);
			long nTotalPatients = AdoFldLong(rs, "TotalPatients", 0);

			//whatever we got for nInsuranceCoID, use it
			if(nInsuranceCoID != -1) {
				ptrEOBInfo->nLikelyInsuranceCoID = nInsuranceCoID;

				str.Format("CalcInsuranceIDs: Selected Insurance Co ID %li due to %li matching patient/plans and %li matching patients.", nInsuranceCoID, nTotalPatientPlans, nTotalPatients);
				Log(str);
			}
		}
		rs->Close();
	}

	if(ptrEOBInfo->nLikelyInsuranceCoID == -1) {
		str.Format("CalcInsuranceIDs: Failed to calculate an Insurance Co ID due to no matching patients.");
		Log(str);
	}

	//now try to calculate insured party IDs

	m_ptrProgressBar->SetPos(0);
	m_ptrProgressBar->SetRange(0, ptrEOBInfo->paryEOBClaimInfo.GetSize());		

	if(m_CountOfEOBs == 1) {
		str = "Assigning Insured Party IDs...";
	}
	else {
		str.Format("Assigning Insured Party (EOB %li)...", ptrEOBInfo->nIndex+1);
	}
	m_ptrProgressStatus->SetWindowText(str);

	//for each claim, find the insured party for the insurance co ID,
	//otherwise use their first insured party

	//loop through all claims
	for(i=0; i<ptrEOBInfo->paryEOBClaimInfo.GetSize(); i++) {

		OHIPEOBClaimInfo *pClaimInfo = (OHIPEOBClaimInfo*)ptrEOBInfo->paryEOBClaimInfo.GetAt(i);

		//initialize to -1
		pClaimInfo->nInsuredPartyID = -1;
		// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
		pClaimInfo->nRespTypeID = -1;
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		pClaimInfo->bSubmitAsPrimary = FALSE;
		pClaimInfo->nInsuranceCoID = -1;

		if(pClaimInfo->nPatientID != -1) {

			//try to find an insured party, ANY insured party, but in order of priority of
			//matching by plan name and insurance company ID, just insurance company ID,
			//or any insured party, in order of responsibility

			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
				"InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, InsurancePlansT.PlanName, InsuranceCoID, InsuranceCoT.Name, RespTypeID, "
				"(CASE "
				"WHEN InsuranceCoID = {INT} AND InsurancePlansT.PlanName = {STRING} THEN 0 "
				"WHEN InsuranceCoID = {INT} THEN 1 "
				"ELSE 2 END) AS Priority "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"ORDER BY "
				"(CASE "
				"WHEN InsuranceCoID = {INT} AND InsurancePlansT.PlanName = {STRING} THEN 0 "
				"WHEN InsuranceCoID = {INT} THEN 1 "
				"ELSE 2 END), "
				"(CASE WHEN RespTypeID <> -1 THEN RespTypeID ELSE (SELECT Coalesce(Max(ID), 0) + 1 FROM RespTypeT) END) ",
				ptrEOBInfo->nLikelyInsuranceCoID, pClaimInfo->strPaymentProgram,
				ptrEOBInfo->nLikelyInsuranceCoID,
				pClaimInfo->nPatientID,
				ptrEOBInfo->nLikelyInsuranceCoID, pClaimInfo->strPaymentProgram,
				ptrEOBInfo->nLikelyInsuranceCoID);

			if(!rs->eof) {

				//grab all the values and log accordingly

				pClaimInfo->nInsuredPartyID = AdoFldLong(rs, "PersonID", -1);
				// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
				pClaimInfo->nRespTypeID = AdoFldLong(rs, "RespTypeID", -1);
				// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
				pClaimInfo->bSubmitAsPrimary = AdoFldBool(rs, "SubmitAsPrimary", 0);
				CString strPlanName = AdoFldString(rs, "PlanName", "");
				pClaimInfo->nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID", -1);
				pClaimInfo->strInsuranceCoName = AdoFldString(rs, "Name", "");
				long nRespTypeID = AdoFldLong(rs, "RespTypeID", -1);

				if(pClaimInfo->nInsuredPartyID == -1) {
					str.Format("CalcInsuranceIDs: Failed to calculate an Insured Party ID for Patient ID %li because only a -1 insured party ID was found.", pClaimInfo->nPatientID);
					Log(str);
				}
				else {

					BOOL bInsCoMatches = (pClaimInfo->nInsuranceCoID == ptrEOBInfo->nLikelyInsuranceCoID);
					BOOL bPlanNameMatches = (strPlanName.CompareNoCase(pClaimInfo->strPaymentProgram) == 0);

					if(bInsCoMatches && bPlanNameMatches) {
						//we matched on insurance co ID and plan name
						str.Format("CalcInsuranceIDs: Calculated Insured Party ID %li (RespTypeID: %li) for Patient ID %li "
							"by matching Insurance Co ID %li (%s) and Plan Name '%s'.",
							pClaimInfo->nInsuredPartyID, nRespTypeID, pClaimInfo->nPatientID,
							pClaimInfo->nInsuranceCoID, pClaimInfo->strInsuranceCoName, strPlanName);
						Log(str);
					}
					else if(bInsCoMatches) {
						//we matched only on insurance co ID
						str.Format("CalcInsuranceIDs: Calculated Insured Party ID %li (RespTypeID: %li) for Patient ID %li "
							"by matching Insurance Co ID %li (%s).",
							pClaimInfo->nInsuredPartyID, nRespTypeID, pClaimInfo->nPatientID,
							pClaimInfo->nInsuranceCoID, pClaimInfo->strInsuranceCoName);
						Log(str);
					}						
					else {
						//all purpose message, even if plan name matched we didn't search for it like this
						str.Format("CalcInsuranceIDs: Warning: Calculated Insured Party ID %li (RespTypeID: %li) for Patient ID %li "
							"using non-matching Insurance Co ID %li (%s).",
							pClaimInfo->nInsuredPartyID, nRespTypeID, pClaimInfo->nPatientID,
							pClaimInfo->nInsuranceCoID, pClaimInfo->strInsuranceCoName);
						Log(str);
					}
				}
			}
			else {
				str.Format("CalcInsuranceIDs: Failed to calculate an Insured Party ID for Patient ID %li because no insured parties were found.", pClaimInfo->nPatientID);
				Log(str);
			}
			rs->Close();								
		}
		else {
			str.Format("CalcInsuranceIDs: Failed to calculate an Insured Party ID due to a Patient ID of -1.");
			Log(str);
		}
	}
}

// (j.jones 2008-12-15 12:20) - PLID 32322 - created CalcBillAndPatientID and moved its code from CalcInternalIDs,
// so we can call it while parsing and output the patient name in the EOB.txt
// Returns TRUE if we found a matching bill and patient, otherwise FALSE.
// (b.spivey, October 01, 2012) - PLID 52666 - Modified this to fail if the health reg numbers don't match. 
BOOL COHIPERemitParser::CalcBillAndPatientID(const IN CString strAccountingNumber, IN CString strHealthRegistrationNumber,
											 OUT long &nBillID, OUT long &nPatientID, OUT CString &strPatientName)
{
	//pass exceptions to the caller

	//ensure these IDs start out as -1
	nBillID = -1;
	nPatientID = -1;
	strPatientName = "";
	// (b.spivey, October 01, 2012) - PLID 52666 - We need to grab OUR Health Reg #
	CString strHealthRegNum = "";

	//And we want to get rid of spaces and dashes. 
	strHealthRegistrationNumber.Replace("-", "");
	strHealthRegistrationNumber.Replace(" ","");


	CString str;

	//figure out which patient and bill this is

	//we send the Bill ID as the accounting number, so try to find that
	long nBillIDToCheck = atoi(strAccountingNumber);
	//confirm it is non-zero
	if(nBillIDToCheck != 0) {

		//now confirm the bill exists and grab the patient ID and name
		// (b.spivey, October 01, 2012) - PLID 52666 - We now pull the health reg number. 
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, Last + ', ' + First + ' ' + Middle AS PatientName, "
			"OHIPHealthCardCustomT.TextParam AS HealthRegNum "
			"FROM BillsT "
			"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
			"LEFT JOIN "
			"	( "
			"		SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = {INT} "
			"	) AS OHIPHealthCardCustomT "
			"ON PersonT.ID = OHIPHealthCardCustomT.PersonID "
			"WHERE BillsT.ID = {INT} AND BillsT.Deleted = 0", m_nHealthNumCustomField, nBillIDToCheck);
		if(!rs->eof) {

			//we found it, we can now confirm we have the bill ID and patient ID			
			nBillID = nBillIDToCheck;
			nPatientID = AdoFldLong(rs, "PatientID");
			strPatientName = AdoFldString(rs, "PatientName");

			// (b.spivey, October 01, 2012) - PLID 52666 - Hold our Health Reg # and strip spaces/dashes.
			strHealthRegNum = AdoFldString(rs, "HealthRegNum", ""); 
			strHealthRegNum.Replace("-","");
			strHealthRegNum.Replace(" ","");
			
			str.Format("CalcInternalIDs: Successfully linked Accounting Number (Bill ID) %s to Patient ID %li, Name: %s.", strAccountingNumber, nPatientID, strPatientName);
			Log(str);
		}
		rs->Close();
	}

	if(nBillID == -1) {
		str.Format("CalcInternalIDs: Failed to calculate Bill and Patient IDs because the Accounting Number (Bill ID) %s, does not conform to any bill ID.", strAccountingNumber);

		// (j.jones 2012-10-04 15:45) - PLID 52929 - if the preference is enabled to skip missing claims,
		// note in the log that this patient will be completely hidden
		if(m_bIgnoreMissingPatients) {
			str += " Additionally, the preference to ignore missing claims is enabled. This missing bill will not be included in the EOB report or dialog.";
		}

		Log(str);

		return FALSE;
	}
	// (b.spivey, October 01, 2012) - PLID 52666 - We check the OHIP # against our #. 
	//	 if this doesn't match, we need to assume this patient and bill doesn't exist in practice. 
	else if(strHealthRegistrationNumber.CompareNoCase(strHealthRegNum) != 0) {
		str.Format("CalcInternalIDs: Failed to calculate Bill and Patient IDs because the Health Registration Number (%s), "
			"does not conform to the patient's Health Registration Number (%s) matching the bill in Practice.", 
			strHealthRegistrationNumber, strHealthRegNum);

		// (j.jones 2012-10-04 15:45) - PLID 52929 - if the preference is enabled to skip missing claims,
		// note in the log that this patient will be completely hidden
		if(m_bIgnoreMissingPatients) {
			str += " Additionally, the preference to ignore missing claims is enabled. This missing bill will not be included in the EOB report or dialog.";
		}

		Log(str);

		//These need to come out with no link to practice, because they don't exist in practice.
		nBillID = -1;
		nPatientID = -1;
		strPatientName = "";
		strHealthRegNum = "";

		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (j.jones 2008-12-15 16:10) - PLID 32329 - given an explanatory code, return its description
CString COHIPERemitParser::GetExplanatoryDescriptionFromCode(CString strExplanatoryCode)
{
	try {

		//blank is ok, it means there is no special message - this should be the most common situation
		if(strExplanatoryCode == "") {
			return "";
		}

		if(strExplanatoryCode == "EA") {
			return "Service date is not within an eligible period - services provided on or after the 20th of this month will not be paid unless eligibility status changes";
		}
		else if(strExplanatoryCode == "EV") {
			return "Check health card for current version";
		}
		else if(strExplanatoryCode == "EF") {
			return "Incorrect version code - services provided on or after the 20th of this month will not be paid unless the current version code is provided";
		}
		else if(strExplanatoryCode == "E1") {
			return "Service date is prior to start of eligibility";
		}
		else if(strExplanatoryCode == "E2") {
			return "Incorrect version code for service date";
		}
		else if(strExplanatoryCode == "E4") {
			return "Service date is after the eligibility termination date";
		}
		else if(strExplanatoryCode == "E5") {
			return "Service date is not within an eligible period";
		}
		else if(strExplanatoryCode == "J7") {
			return "Claim submitted six months after service date";
		}
		else if(strExplanatoryCode == "GF") {
			return "Coverage lapsed - bill patient for future claims";
		}
		else if(strExplanatoryCode == "30") {
			return "This service is not a benefit of the ministry";
		}
		else if(strExplanatoryCode == "32") {
			return "Ministry records show that this service has already been claimed for payment to the patient";
		}
		else if(strExplanatoryCode == "35") {	//this is key, 35 means it's a duplicate service
			return "Ministry records show this service rendered by you has been claimed previously";
		}
		else if(strExplanatoryCode == "36") {
			return "Ministry records show this service has been rendered by another practitioner, group, lab";
		}
		else if(strExplanatoryCode == "37") {
			return "Effective April, 1993 the listed benefit for this code is 0 LMS units";
		}
		else if(strExplanatoryCode == "40") {
			return "This service or related service allowed only once for same patient";
		}
		else if(strExplanatoryCode == "48") {
			return "Paid as submitted - clinical records may be requested for verification purposes";
		}
		else if(strExplanatoryCode == "49") {
			return "Paid according to the average fee for this service - independent consideration will be given if clinical records/operative reports are presented";
		}
		else if(strExplanatoryCode == "50") {
			return "Fee allowed according to the appropriate item in the current ministry Schedule of Benefits for physician services";
		}
		else if(strExplanatoryCode == "51") {
			return "Fee Schedule Code changed in accordance with Schedule of Benefits";
		}
		else if(strExplanatoryCode == "52") {
			return "Fee for service assessed by medical consultant";
		}
		else if(strExplanatoryCode == "53") {
			return "Fee allowed according to appropriate item in a previous ministry Schedule of Benefits";
		}
		else if(strExplanatoryCode == "54") {
			return "Interim payment claim under review Technical Specifications Interface to Health Care Systems Machine Readable Output Specifications";
		}
		else if(strExplanatoryCode == "55") {
			return "This deduction is an adjustment on an earlier account";
		}
		else if(strExplanatoryCode == "56") {
			return "Claim under review";
		}
		else if(strExplanatoryCode == "57") {
			return "This payment is an adjustment on an earlier account";
		}
		else if(strExplanatoryCode == "58") {
			return "Claimed by another physician within your group";
		}
		else if(strExplanatoryCode == "59") {
			return "Health Care Provider’s notification - WCB claims";
		}
		else if(strExplanatoryCode == "61") {
			return "OOC claim paid at greater than $9999.99 (prior approval on file)";
		}
		else if(strExplanatoryCode == "65") {
			return "Service included in approved hospital payment";
		}
		else if(strExplanatoryCode == "68") {
			return "Hospital accommodation paid at standard ward rate";
		}
		else if(strExplanatoryCode == "69") {
			return "Elective services paid at 75% of insured costs";
		}
		else if(strExplanatoryCode == "70") {
			return "OHIP records show corresponding procedure(s)/visit(s) on this day claimed previously";
		}
		else if(strExplanatoryCode == "80") {
			return "Technical fee adjustment for hospitals and IHFs";
		}
		else if(strExplanatoryCode == "AP") {
			return "This payment is in accordance with legislation-if you disagree with the payment you may appeal";
		}
		else if(strExplanatoryCode == "DM") {
			return "Paid/disallowed in accordance with ministry policy regarding emergency department equivalent";
		}
		else if(strExplanatoryCode == "EB") {
			return "Additional payment for the claim shown";
		}
		else if(strExplanatoryCode == "I2") {
			return "Service is globally funded";
		}
		else if(strExplanatoryCode == "J3") {
			return "Approved for stale date processing";
		}
		else if(strExplanatoryCode == "Q8") {
			return "Laboratory not licensed to perform this test on date of service";
		}
		else if(strExplanatoryCode == "SR") {
			return "Fee reduced based on ministry utilization adjustment - contact your physician/practitioner";
		}
		else if(strExplanatoryCode == "TH") {
			return "Fee reduced per ministry Payment Policy - contact your physician";
		}
		else if(strExplanatoryCode == "C1") {
			return "Allowed as repeat/limited consultation/midwife-requested emergency assessment";
		}
		else if(strExplanatoryCode == "C2") {
			return "Allowed at reassessment fee";
		}
		else if(strExplanatoryCode == "C3") {
			return "Allowed at minor assessment fee";
		}
		else if(strExplanatoryCode == "C4") {
			return "Consultation not allowed with this service - paid as assessment";
		}
		else if(strExplanatoryCode == "C5") {
			return "Allowed as multiple systems assessment";
		}
		else if(strExplanatoryCode == "C6") {
			return "Allowed as Type 2 Admission Assessment";
		}
		else if(strExplanatoryCode == "C7") {
			return "An admission assessment C003A or general re-assessment C004A may not be claimed by any physician within 30 days following a pre-dental/pre-operative assessment Critical Care";
		}
		else if(strExplanatoryCode == "G1") {
			return "Other critical/comprehensive care already paid Diagnostic and Therapeutic Procedures";
		}
		else if(strExplanatoryCode == "D1") {
			return "Allowed as repeat procedure; initial procedure previously claimed";
		}
		else if(strExplanatoryCode == "D2") {
			return "Additional procedures allowed at 50%";
		}
		else if(strExplanatoryCode == "D3") {
			return "Not allowed in addition to visit fee";
		}
		else if(strExplanatoryCode == "D4") {
			return "Procedure allowed at 50% with visit";
		}
		else if(strExplanatoryCode == "D5") {
			return "Procedure already allowed - visit fee adjusted";
		}
		else if(strExplanatoryCode == "D6") {
			return "Limit of payment for this procedure reached";
		}
		else if(strExplanatoryCode == "D7") {
			return "Not allowed in addition to other procedure";
		}
		else if(strExplanatoryCode == "D8") {
			return "Allowed with specific procedures only";
		}
		else if(strExplanatoryCode == "D9") {
			return "Not allowed to a hospital department";
		}
		else if(strExplanatoryCode == "DA") {
			return "Maximum for this procedure reached - paid as repeat/chronic procedure";
		}
		else if(strExplanatoryCode == "DB") {
			return "Other dialysis procedure already paid";
		}
		else if(strExplanatoryCode == "DC") {
			return "Procedure paid previously not allowed in addition to this procedure - fee adjusted to pay the difference";
		}
		else if(strExplanatoryCode == "DD") {
			return "Not allowed as diagnostic code is unrelated to original major eye exam";
		}
		else if(strExplanatoryCode == "DE") {
			return "Laboratory tests already paid - visit fee adjusted";
		}
		else if(strExplanatoryCode == "DG") {
			return "Diagnostic/miscellaneous services for hospital patients are payable on a fee-for-service basis - included in hospital global budget";
		}
		else if(strExplanatoryCode == "DH") {
			return "Ventilatory support allowed with Haemodialysis";
		}
		else if(strExplanatoryCode == "DL") {
			return "Allowed as laboratory test in private office";
		}
		else if(strExplanatoryCode == "DM") {
			return "Paid/disallowed in accordance with MOH policy regarding an Emergency Department Equivalent";
		}
		else if(strExplanatoryCode == "DN") {
			return "Allowed as pudendal block in addition to procedure as per stated policy";
		}
		else if(strExplanatoryCode == "DP") {
			return "Procedure paid previously allowed at 50% in addition to this procedure - fee adjusted to pay the difference";
		}
		else if(strExplanatoryCode == "DV") {
			return "Service is included in Monthly Management Fee for Long-Term Care Patients Fractures";
		}
		else if(strExplanatoryCode == "F1") {
			return "Additional fractures/dislocations allowed at 85%";
		}
		else if(strExplanatoryCode == "F2") {
			return "Allowed in accordance with transferred care";
		}
		else if(strExplanatoryCode == "F3") {
			return "Previous attempted reductions (open or closed) allowed at 85%";
		}
		else if(strExplanatoryCode == "F5") {
			return "Two weeks aftercare included in fracture fee";
		}
		else if(strExplanatoryCode == "F6") {
			return "Allowed as Minor/Partial Assessment Hospital Visits";
		}
		else if(strExplanatoryCode == "H1") {
			return "Admission assessment or ER assessment already paid";
		}
		else if(strExplanatoryCode == "H2") {
			return "Allowed as subsequent visit; initial visit previously claimed";
		}
		else if(strExplanatoryCode == "H3") {
			return "Maximum fee allowed per week after 5th week";
		}
		else if(strExplanatoryCode == "H4") {
			return "Maximum fee allowed per week after 6th week to paediatricians";
		}
		else if(strExplanatoryCode == "H5") {
			return "Maximum fee allowed per month after 13th week";
		}
		else if(strExplanatoryCode == "H6") {
			return "Allowed as supportive or concurrent care";
		}
		else if(strExplanatoryCode == "H7") {
			return "Allowed as chronic care";
		}
		else if(strExplanatoryCode == "H8") {
			return "Hospital number and/or admission date required for in-hospital service";
		}
		else if(strExplanatoryCode == "H9") {
			return "Concurrent care already claimed by another doctor";
		}
		else if(strExplanatoryCode == "HA") {
			return "Admission assessment claimed by another physician - hospital visit fee applied";
		}
		else if(strExplanatoryCode == "HF") {
			return "Concurrent or Supportive Care already claimed in period";
		}
		else if(strExplanatoryCode == "L1") {
			return "This service paid to another laboratory";
		}
		else if(strExplanatoryCode == "L2") {
			return "Not allowed to non-medical laboratory director";
		}
		else if(strExplanatoryCode == "L3") {
			return "Not allowed in addition to this laboratory procedure";
		}
		else if(strExplanatoryCode == "L4") {
			return "Not allowed to attending physicians";
		}
		else if(strExplanatoryCode == "L5") {
			return "Not allowed in addition to other procedure paid to another laboratory";
		}
		else if(strExplanatoryCode == "L6") {
			return "Procedure paid previously to another laboratory, not allowed in addition to this procedure - fee adjusted to pay difference";
		}
		else if(strExplanatoryCode == "L7") {
			return "Not allowed - referred specimen";
		}
		else if(strExplanatoryCode == "L8") {
			return "Not to be claimed with prenatal/fetal assessment as of July 1, 1993";
		}
		else if(strExplanatoryCode == "L9") {
			return "Laboratory services for hospital in-patients are not payable on a fee-for-service basis-included in the hospital global budget";
		}
		else if(strExplanatoryCode == "LS") {
			return "Paid in accordance to Special Lab Agreement Paediatric Care";
		}
		else if(strExplanatoryCode == "P2") {
			return "Maximum fee allowed for low-birth weight care";
		}
		else if(strExplanatoryCode == "P3") {
			return "Maximum fee allowed for newborn care";
		}
		else if(strExplanatoryCode == "P4") {
			return "Fee for newborn/low-birth weight care is not billable with neonatal intensive care";
		}
		else if(strExplanatoryCode == "P5") {
			return "Over-age for paediatric rates of payment";
		}
		else if(strExplanatoryCode == "P6") {
			return "Over-age for well baby care Obstetrics";
		}
		else if(strExplanatoryCode == "O1") {
			return "Fee for obstetric care apportioned";
		}
		else if(strExplanatoryCode == "O2") {
			return "Previous prenatal care already claimed";
		}
		else if(strExplanatoryCode == "O3") {
			return "Previous prenatal care already claimed by another doctor";
		}
		else if(strExplanatoryCode == "O4") {
			return "Office visits relating to pregnancy and claimed prior to delivery included in obstetric fee";
		}
		else if(strExplanatoryCode == "O5") {
			return "Not allowed in addition to delivery";
		}
		else if(strExplanatoryCode == "O6") {
			return "Medical induction/stimulation of labour allowed once per pregnancy";
		}
		else if(strExplanatoryCode == "O7") {
			return "Allowed as subsequent prenatal visit. Initial prenatal visit already claimed";
		}
		else if(strExplanatoryCode == "O8") {
			return "Allowed once per pregnancy";
		}
		else if(strExplanatoryCode == "O9") {
			return "Not allowed in addition to post-natal care";
		}
		else if(strExplanatoryCode == "V1") {
			return "Allowed as repeat assessment - initial assessment previously claimed";
		}
		else if(strExplanatoryCode == "V2") {
			return "Allowed as extra patient seen in the home";
		}
		else if(strExplanatoryCode == "V3") {
			return "Not allowed in addition to procedural fee";
		}
		else if(strExplanatoryCode == "V4") {
			return "Date of service was not a Saturday, Sunday, or a statutory holiday";
		}
		else if(strExplanatoryCode == "V5") {
			return "Only one oculo-visual assessment (OVA) allowed within a 12-month period for age 19 and under or 65 and over and one within 24 months for age 20-64";
		}
		else if(strExplanatoryCode == "V6") {
			return "Allowed as minor assessment - initial assessment already claimed";
		}
		else if(strExplanatoryCode == "V7") {
			return "Allowed at medical/specific reassessment fee";
		}
		else if(strExplanatoryCode == "V8") {
			return "This service paid at lower fee as per stated ministry policy";
		}
		else if(strExplanatoryCode == "V9") {
			return "Only one initial office visit allowed within 12-month period";
		}

		if(strExplanatoryCode == "VA") {
			return "Procedure fee reduced. Consultation/visit fees not allowed in addition";
		}
		else if(strExplanatoryCode == "VB") {
			return "Additional OVA is allowed once within the second year for patients aged 20-64, following a periodic OVA";
		}
		else if(strExplanatoryCode == "VG") {
			return "Only one geriatric general assessment premium per patient per 12-month period";
		}
		else if(strExplanatoryCode == "VM") {
			return "Oculo-visual minor assessment is allowed within 12 consecutive months following a major eye exam";
		}
		else if(strExplanatoryCode == "VP") {
			return "Allowed with specific visit only";
		}
		else if(strExplanatoryCode == "VS") {
			return "Date of service was a Saturday, Sunday or statutory holiday";
		}
		else if(strExplanatoryCode == "VX") {
			return "Compexity Premium not applicable to visit fee";
		}
		else if(strExplanatoryCode == "X2") {
			return "G.I. tract includes cine and video tape";
		}
		else if(strExplanatoryCode == "X3") {
			return "G.I. tract includes survey film of abdomen";
		}
		else if(strExplanatoryCode == "X4") {
			return "Only one BMD allowed within a 24 month period for a low risk patient";
		}	
		else if(strExplanatoryCode == "S1") {
			return "Bilateral surgery, one stage, allowed at 85% higher than unilateral";
		}
		else if(strExplanatoryCode == "S2") {
			return "Bilateral surgery, two stage, allowed at 85% higher than unilateral";
		}
		else if(strExplanatoryCode == "S3") {
			return "Second surgical procedure allowed at 85%";
		}
		else if(strExplanatoryCode == "S4") {
			return "Procedure fee reduced when paid with related surgery or anaesthetic";
		}
		else if(strExplanatoryCode == "S5") {
			return "Not allowed in addition to major surgical fee";
		}
		else if(strExplanatoryCode == "S6") {
			return "Allowed as subsequent procedure-initial procedure previously claimed";
		}
		else if(strExplanatoryCode == "S7") {
			return "Normal pre-operative and post-operative care included in surgical fee";
		}
		else if(strExplanatoryCode == "SA") {
			return "Surgical procedure allowed at consultation fee";
		}
		else if(strExplanatoryCode == "SB") {
			return "Normal pre-operative visit included in surgical fee - visit fee previously paid-surgical fee adjusted";
		}
		else if(strExplanatoryCode == "SC") {
			return "Not allowed major pre-operative visit already claimed";
		}
		else if(strExplanatoryCode == "SD") {
			return "Not allowed-team/assist fee already claimed";
		}
		else if(strExplanatoryCode == "SE") {
			return "Major pre-operative visit previously paid and admission assessment previously paid - surgery fee reduced by the admission assessment";
		}	
		else if(strExplanatoryCode == "T1") {
			return "Fee allowed according to surgery claim";
		}
		else if(strExplanatoryCode == "R1") {
			return "Only one health exam allowed in a 12-month period";
		}	
		else if(strExplanatoryCode == "M1") {
			return "Maximum fee allowed or maximum number of services has been reached same/any provider";
		}
		else if(strExplanatoryCode == "M2") {
			return "Maximum allowance for radiographic examination(s) by one or more practitioners";
		}
		else if(strExplanatoryCode == "M3") {
			return "Maximum fee allowed for prenatal care";
		}
		else if(strExplanatoryCode == "M4") {
			return "Maximum fee allowed for these services by one or more practitioners has been reached";
		}
		else if(strExplanatoryCode == "M5") {
			return "Monthly maximum has been reached";
		}
		else if(strExplanatoryCode == "M6") {
			return "Maximum fee allowed for special visit premium - additional patient seen";
		}
		else if(strExplanatoryCode == "MC") {
			return "Maximum of 2 patient case conferences has been reached in a 12-month period";
		}
		else if(strExplanatoryCode == "MN") {
			return "Maximum number of sessions has been reached";
		}
		else if(strExplanatoryCode == "MS") {
			return "Maximum allowable for sleep studies in a 12-month period by one or more physicians has been reached";
		}	
		else if(strExplanatoryCode == "MX") {
			return "Maximum of 2 arthroscopy ‘R’ codes with E595 has been reached	";
		}	
		else if(strExplanatoryCode == "60") {
			return "Not a benefit of RMB agreement";
		}
		else if(strExplanatoryCode == "RD") {
			return "Duplicate, paid by RMB";
		}
		else if(strExplanatoryCode == "FF") {
			return "Additional payment for the claim shown";
		}
		else if(strExplanatoryCode == "I2") {
			return "Service is globally funded";
		}
		else if(strExplanatoryCode == "I3") {
			return "FSC is not on the IHF licence profile for the date specified";
		}
		else if(strExplanatoryCode == "I4") {
			return "Records show this service has been rendered by another practitioner, group or IHF";
		}
		else if(strExplanatoryCode == "I5") {
			return "Service is globally funded and FSC is not on IHF licence profile";
		}
		// (j.jones 2009-04-07 15:22) - PLID 33891 - supported new error HM
		else if(strExplanatoryCode == "HM") {
			return "Invalid Hospital Master Number on Date of Service";
		}
		else {
			//not found
			ASSERT(FALSE);
			return "<Unknown Explanatory Code>";
		}

	}NxCatchAll("Error in COHIPERemitParser::GetExplanatoryDescriptionFromCode");

	return "";
}

// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
// can optionally disable PeekAndPump usage for the import process
void COHIPERemitParser::PeekAndPump_OHIPERemitParser()
{
	if(m_bEnablePeekAndPump) {
		PeekAndPump();
	}
}

// (j.jones 2011-03-18 15:11) - PLID 42905 - given a patient ID, find all of that patient's
// charges that we are about to post to, and add to the list
void COHIPERemitParser::GetChargeIDsByPatientID(long nPatientID, CArray<long,long> &aryUsedCharges)
{
	//ensure this is blank to begin with
	aryUsedCharges.RemoveAll();

	if(nPatientID == -1) {
		//do nothing if we weren't given a patient ID
		return;
	}

	//for each EOB
	for(int i=0; i<m_paryOHIPEOBInfo.GetSize(); i++) {
		
		OHIPEOBInfo *ptrEOBInfo = (OHIPEOBInfo*)m_paryOHIPEOBInfo.GetAt(i);
		
		if(ptrEOBInfo != NULL && ptrEOBInfo->paryEOBClaimInfo.GetSize() > 0) {

			//for each claim
			for(int j=0; j<ptrEOBInfo->paryEOBClaimInfo.GetSize(); j++) {
				
				OHIPEOBClaimInfo *pClaimInfo = (OHIPEOBClaimInfo*)ptrEOBInfo->paryEOBClaimInfo.GetAt(j);
				
				//is this claim for our patient?
				if(pClaimInfo != NULL && pClaimInfo->nPatientID != -1
					&& pClaimInfo->nPatientID == nPatientID
					&& pClaimInfo->paryOHIPEOBLineItemInfo.GetSize() > 0) {

					//for each charge, grab its id
					for(int k=0; k<pClaimInfo->paryOHIPEOBLineItemInfo.GetSize(); k++) {

						OHIPEOBLineItemInfo *pLineItemInfo = (OHIPEOBLineItemInfo*)pClaimInfo->paryOHIPEOBLineItemInfo.GetAt(k);
						if(pLineItemInfo != NULL && pLineItemInfo->nChargeID != -1) {
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
// > 365 days old are deleted
void COHIPERemitParser::CopyConvertedEOBToServer()
{
	try {

		//calculate the server's EOB path
		CString strServerPath = GetNxConvertedEOBsPath();

		//remove all files that are > 365 days old
		{
			CTime tm;
			//this assumes that at least the system's date is accurate
			CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(365,0,0,0);

			//first remove EOB files
			CFileFind finder;
			BOOL bWorking = finder.FindFile(strServerPath ^ "EOB*.txt");	
			while(bWorking) {
				bWorking = finder.FindNextFile();

				CString strFilePath = finder.GetFilePath();
				tm = FileUtils::GetFileModifiedTime(strFilePath); 
				if(tm < tmMin) {
					DeleteFile(strFilePath);
				}
			}

			//now remove all files with the remit backup extension
			bWorking = finder.FindFile(strServerPath ^ "*.rmtbak");
			while(bWorking) {
				bWorking = finder.FindNextFile();

				CString strFilePath = finder.GetFilePath();
				tm = FileUtils::GetFileModifiedTime(strFilePath); 
				if(tm < tmMin) {
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
			while(DoesExist(strNewFileName)) {
				
				//try adding an index to the end
				nCount++;

				if(nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy EOB to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\EOB_%s_%s_%li.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if(!CopyFile(m_strOutputFile, strNewFileName, TRUE)) {
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
			if(nDot != -1) {
				strFileNameNoExt = strFileNameNoExt.Left(nDot - 1);
			}

			CString strNewFileName;
			strNewFileName.Format("%s\\%s_%s_%s.rmtbak", strServerPath, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));
			
			//it's very unlikely that this file will exist, but handle the case anyways
			int nCount = 0;
			while(DoesExist(strNewFileName)) {
				
				//try adding an index to the end
				nCount++;

				if(nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy remit file to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\%s_%s_%s_%li.rmtbak", strServerPath, strFileNameNoExt, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if(!CopyFile(m_strFileName, strNewFileName, TRUE)) {
				//failed
				ThrowNxException("Cannot copy remit file to server, filename: %s", strNewFileName);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, October 9th, 2014) PLID 62701 - Accessor
CString COHIPERemitParser::GetStoredParsedFilePath()
{
	return m_strStoredParsedFile;
}