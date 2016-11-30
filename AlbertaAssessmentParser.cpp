#include "stdafx.h"
#include "AlbertaAssessmentParser.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2014-07-24 10:47) - PLID 62579 - created

using namespace ADODB;

CAlbertaAssessmentParser::CAlbertaAssessmentParser()
{
	m_pParentWnd = NULL;
	m_dtFileDate = g_cdtNull;
	m_dtFirstExpectedPaymentDate = g_cdtNull;
	m_cyTotalAssessedAmount = g_ccyInvalid;

	m_pChargeList = make_shared<AlbertaAssessments::ChargeList>();
}

CAlbertaAssessmentParser::~CAlbertaAssessmentParser()
{

}

bool CAlbertaAssessmentParser::ParseFile(CWnd *pParentWnd)
{
	try {

		m_pParentWnd = pParentWnd;

		//TES 8/5/2014 - PLID 62580 - We may have been given a filename
		if (m_strFileName.IsEmpty()) {
			//browse for a file
			// (j.jones 2013-06-18 09:12) - PLID 57191 - added option for All Files, not just text files, because these reports
			// usually do not have extensions
			CFileDialog dlgFiles(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, "All Files (*.*)|*.*|Text Files (*.txt)|*.txt||", m_pParentWnd);
			if (dlgFiles.DoModal() == IDCANCEL) {
				return false;
			}

			m_strFileName = dlgFiles.GetPathName();
		}

		if (m_strFileName.IsEmpty() || !DoesExist(m_strFileName)) {
			return false;
		}

		m_strOutputFile = GetNxTempPath() ^ "AssessmentFile.txt";

		bool bIsValidFile = false;

		CStdioFile inputFile, outputFile;
		
		//open the files for reading/writing		
		if (!inputFile.Open(m_strFileName, CFile::modeRead | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The input file could not be found or opened.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		if (!outputFile.Open(m_strOutputFile, CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "The output file could not be created.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}


		CWaitCursor pWait;
		
		//make sure the file is a valid assessement file ( first line always has the word "HEADER" in it )
		CString strTemp;
		inputFile.ReadString(strTemp);
		if (strTemp.Find("HEADER") != -1)
		{
			//this is a valid file, so let's initialize the total payments to zero
			m_cyTotalAssessedAmount = COleCurrency(0, 0);

			inputFile.SeekToBegin();
			//loop through lines in txt file , grab date from 1st line and all the real info from claims in file
			while (inputFile.ReadString(strTemp) != NULL)
			{
				CString strFormattedText;

				if (strTemp.Find("HEADER") != -1)
				{
					CString strDate = strTemp.Left(19);
					if (!m_dtFileDate.ParseDateTime(strDate) || m_dtFileDate.GetStatus() != COleDateTime::valid) {
						//if we hit this, try to figure out why
						ASSERT(FALSE);
						m_dtFileDate = COleDateTime::GetCurrentTime();

						strFormattedText = "Warning: Invalid Date Provided (" + strDate + "). Today's date will be used instead.\n";
						outputFile.WriteString(strFormattedText);
					}
					strFormattedText = "Date: " + FormatDateTimeForInterface(m_dtFileDate, NULL, dtoDateTime) + "\n";
					outputFile.WriteString(strFormattedText);

					inputFile.ReadString(strTemp);
				}

				if (strTemp.Find("TRAILER") == -1)//the last line has word "TRAILER"
				{
					// (j.jones 2014-07-24 13:43) - PLID 62579 - This now creates a memory object
					// for later use in E-Remittance.

					//TES 8/5/2014 - PLID 62580 - Changed this to a pointer
					AlbertaAssessments::ChargeInfoPtr pNewRecord = make_shared<AlbertaAssessments::ChargeInfo>();

					//Claim Number (01-15) A(15)
					pNewRecord->strClaimNum = strTemp.Left(15);
					//Transaction Tag Number (16-19) N(04)
					pNewRecord->strTransactionTagNumber = strTemp.Mid(15, 4);
					
					//Transaction Action Code (20-20) A(01)
					CString strTransactionActionCode = strTemp.Mid(19, 1);
					strTransactionActionCode.TrimLeft(); strTransactionActionCode.TrimRight();
					CString strParsedTransactionActionCode = TransactionActionCode(strTransactionActionCode, pNewRecord);					

					//Transaction Reassess Reason (21-24) A(04)
					CString strTransactionReassessReason = strTemp.Mid(20, 4);
					strTransactionReassessReason.TrimLeft(); strTransactionReassessReason.TrimRight();
					CString strParsedTransactionReassessReason = FindAlbertaReassessReason(strTransactionReassessReason, pNewRecord);
					
					//Assessment Result Action (25-25) A(01)
					CString strAssessmentResultAction_Main = strTemp.Mid(24, 1);
					strAssessmentResultAction_Main.TrimLeft(); strAssessmentResultAction_Main.TrimRight();
					CString strParsedAssessmentResult_Main = AssessmentResultMain(strAssessmentResultAction_Main, pNewRecord);

					//Assessment Result Action (26-26) A(01)		(sometimes (not often) there are two)
					CString strAssessmentResultAction_Additional = strTemp.Mid(25, 1);					
					strAssessmentResultAction_Additional.TrimLeft(); strAssessmentResultAction_Additional.TrimRight();
					CString strParsedAssessmentResult_Additional = AssessmentResultAdditional(strAssessmentResultAction_Additional, pNewRecord);

					//Chart Number (27-40) A(14)
					pNewRecord->strChartNumber = strTemp.Mid(26, 14);
					//Service Recipient ULI (41-49) N(09)
					pNewRecord->strServiceRecipientULI = strTemp.Mid(40, 9);
					//Service Recipient Registration No. (50-61) N(12)
					pNewRecord->strServiceRecipientRegistrationNo = strTemp.Mid(49, 12);
					//Statement of Assessment Reference Number (62-70) N(09)
					pNewRecord->strStatementofAssessmentReferenceNumber = strTemp.Mid(61, 9);
					
					//Expected Payment Date (71-78) N(08)
					CString strExpectedPaymentDate = strTemp.Mid(70, 8);
					strExpectedPaymentDate = FormatDate(strExpectedPaymentDate);
					if (!pNewRecord->dtExpectedPaymentDate.ParseDateTime(strExpectedPaymentDate) || pNewRecord->dtExpectedPaymentDate.GetStatus() != COleDateTime::valid) {
						//find out why this happened
						ASSERT(FALSE);
						pNewRecord->dtExpectedPaymentDate = g_cdtNull;
					}
					else {
						//track the earliest payment date, for later use in batch payments
						if (m_dtFirstExpectedPaymentDate.GetStatus() != COleDateTime::valid || m_dtFirstExpectedPaymentDate > pNewRecord->dtExpectedPaymentDate) {
							m_dtFirstExpectedPaymentDate = pNewRecord->dtExpectedPaymentDate;
						}
					}
					
					//Assessment Date (79-86) N(08)
					CString strAssessmentDate = strTemp.Mid(78, 8);
					strAssessmentDate = FormatDate(strAssessmentDate);
					if (!pNewRecord->dtAssessmentDate.ParseDateTime(strAssessmentDate) || pNewRecord->dtAssessmentDate.GetStatus() != COleDateTime::valid) {
						//find out why this happened
						ASSERT(FALSE);
						pNewRecord->dtAssessmentDate = g_cdtNull;
					}

					//Final Assessed Amount (87-95) N(09)

					// (j.jones 2013-06-18 08:37) - PLID 57191 - The Final Assessed Amount was off by one character,
					// I fixed it so it wouldn't multiply all amounts by 10.
					CString strFinalAssessedAmount = strTemp.Mid(86, 9);
					strFinalAssessedAmount = FormatMoney(strFinalAssessedAmount);
					if (!pNewRecord->cyFinalAssessedAmount.ParseCurrency(strFinalAssessedAmount) || pNewRecord->cyFinalAssessedAmount.GetStatus() != COleCurrency::valid) {
						//find out why this happened
						ASSERT(FALSE);
						pNewRecord->cyFinalAssessedAmount = g_ccyNull;
					}
					else {
						//add this amount to our total
						m_cyTotalAssessedAmount += pNewRecord->cyFinalAssessedAmount;
					}

					//Claimed Amount (96-104) N(09)
					CString strClaimedAmount = strTemp.Mid(95, 9);
					strClaimedAmount = FormatMoney(strClaimedAmount);
					if (!pNewRecord->cyClaimedAmount.ParseCurrency(strClaimedAmount) || pNewRecord->cyClaimedAmount.GetStatus() != COleCurrency::valid) {
						//find out why this happened
						ASSERT(FALSE);
						pNewRecord->cyClaimedAmount = g_ccyNull;
					}

					//Claimed Amount Indicator (105-105) A(01)
					pNewRecord->strClaimedAmountIndicator = strTemp.Mid(104, 1);
					
					//Explanation Codes (106-135) A(05) X6
					CString strExplanationCodes = strTemp.Mid(105, 30);
					strExplanationCodes.TrimLeft(); strExplanationCodes.TrimRight();
					CString strParsedExplanationCodes = FindAlbertaExplanationCodes(strExplanationCodes, pNewRecord);
					
					//EMSAF Status (136-139) A(04)
					pNewRecord->strEMSAFStatus = strTemp.Mid(135, 4);
					pNewRecord->strEMSAFStatus.TrimLeft(); pNewRecord->strEMSAFStatus.TrimRight();
					//Fee Modifiers Used (140-199) A(06) X 10 (occurs 8 times for a total of 48 characters - the specs say X10 and they are wrong)
					pNewRecord->strFeeModifiersUsed = strTemp.Mid(139, 48);
					//FR Reference Number (188-196) N(09)
					pNewRecord->strFRReferenceNumber = strTemp.Mid(187, 9);
					//Unused (197-199) A(03) (Leave Blank)
					//....what?
					//Business Arrangement Number (200-206) N(07)
					pNewRecord->strBusinessArrangementNumber = strTemp.Mid(199, 7);
					//Service Provider ULI (207-215) N(09)
					pNewRecord->strServiceProviderULI = strTemp.Mid(206, 9);
					
					//Service Date (216-223) N(08)
					CString strServiceDate = strTemp.Mid(215, 8);
					strServiceDate = FormatDate(strServiceDate);
					if (!pNewRecord->dtServiceDate.ParseDateTime(strServiceDate) || pNewRecord->dtServiceDate.GetStatus() != COleDateTime::valid) {
						//find out why this happened
						ASSERT(FALSE);
						pNewRecord->dtServiceDate = g_cdtNull;
					}

					//Service Code (224-230) A(07)
					pNewRecord->strServiceCode = strTemp.Mid(223, 7);
					pNewRecord->strServiceCode.TrimLeft(); pNewRecord->strServiceCode.TrimRight();

					//Pay to Code (231-234) A(04)
					pNewRecord->strPayToCode = strTemp.Mid(230, 4);

					//find the patient
					FindPatient(pNewRecord);
					CString strPatientNameToDisplay = pNewRecord->strPatientFullName;
					if (pNewRecord->nPatientID == -1) {
						strPatientNameToDisplay = "No patient found with that chart number";
					}
					
					//find the provider
					FindProviderByULI(pNewRecord);
					CString strProviderNameToDisplay = pNewRecord->strProviderFullName;
					if (pNewRecord->nProviderID == -1) {
						strProviderNameToDisplay = "No provider found with that ULI number";
					}

					//find the charge
					FindCharge(pNewRecord);

					// (j.jones 2014-07-24 13:43) - PLID 62579 - track this record in memory
					m_pChargeList->push_back(pNewRecord);

					//now write each line to the file

					strFormattedText = "\n============================================================================================\n\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Chart Number: " + Trim(pNewRecord->strChartNumber) + "\t\t\tPatient Name: " + strPatientNameToDisplay + "\n\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Service Provider ULI: " + pNewRecord->strServiceProviderULI + "\t\tProvider Name: " + strProviderNameToDisplay + "\n\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Claim Number: " + pNewRecord->strClaimNum + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Transaction Tag Number: " + pNewRecord->strTransactionTagNumber + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Transaction Action Code: " + strParsedTransactionActionCode + "\n\n";
					outputFile.WriteString(strFormattedText);

					if (!strParsedTransactionReassessReason.IsEmpty()) {
						strFormattedText = "Transaction Reassess Reason: " + strParsedTransactionReassessReason + "\n";
						outputFile.WriteString(strFormattedText);
					}

					strFormattedText = "Assessment Results Action: " + strParsedAssessmentResult_Main + "\n";
					outputFile.WriteString(strFormattedText);
						
					// (j.jones 2013-06-18 09:06) - PLID 57191 - there can be two assessment results
					if (!strParsedAssessmentResult_Additional.IsEmpty()) {
						strFormattedText = "Assessment Results Action: " + strParsedAssessmentResult_Additional + "\n";
						outputFile.WriteString(strFormattedText);
					}

					if (!strParsedExplanationCodes.IsEmpty()) {
						strFormattedText = "Explanation Code(s): \n" + strParsedExplanationCodes + "\n";
						outputFile.WriteString(strFormattedText);
					}

					strFormattedText = "Service Recipient ULI: " + pNewRecord->strServiceRecipientULI + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Service Recipient Registration No.: " + pNewRecord->strServiceRecipientRegistrationNo + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Statement of Assessment Reference Number: " + pNewRecord->strStatementofAssessmentReferenceNumber + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Expected Payment Date: " + FormatDateTimeForInterface(pNewRecord->dtExpectedPaymentDate, NULL, dtoDate) + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Assessment Date: " + FormatDateTimeForInterface(pNewRecord->dtAssessmentDate, NULL, dtoDate) + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Final Assessed Amount: " + FormatCurrencyForInterface(pNewRecord->cyFinalAssessedAmount) + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Claimed Amount: " + FormatCurrencyForInterface(pNewRecord->cyClaimedAmount) + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Claimed Amount Indicator: " + pNewRecord->strClaimedAmountIndicator + "\n";
					outputFile.WriteString(strFormattedText);

					if (!pNewRecord->strEMSAFStatus.IsEmpty()) {
						strFormattedText = "EMSAF Status: " + pNewRecord->strEMSAFStatus + "\n";
						outputFile.WriteString(strFormattedText);
					}

					strFormattedText = "Fee Modifiers Used: " + pNewRecord->strFeeModifiersUsed + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "FR Reference Number: " + pNewRecord->strFRReferenceNumber + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Business Arrangement Number: " + pNewRecord->strBusinessArrangementNumber + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Service Date: " + FormatDateTimeForInterface(pNewRecord->dtServiceDate) + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Service Code: " + pNewRecord->strServiceCode + "\n";
					outputFile.WriteString(strFormattedText);

					strFormattedText = "Pay to Code: " + pNewRecord->strPayToCode + "\n";
					outputFile.WriteString(strFormattedText);
				}
			}

			//TES 8/5/2014 - PLID 62580 - Added variables for insurance info
			CalcInsuranceIDs();

			//write our total, if we have one
			outputFile.WriteString("\n============================================================================================\n\n");
			if (m_cyTotalAssessedAmount.GetStatus() == COleCurrency::valid) {
				outputFile.WriteString(FormatString("Total Assessed Amount: %s\n", FormatCurrencyForInterface(m_cyTotalAssessedAmount)));
			}
			else {
				outputFile.WriteString("Total Assessed Amount: <No Assessed Amounts Provided>\n");
			}
			

			inputFile.Close();
			outputFile.Close();

			// this function backs up both the remit file and the EOB.txt to the server,
			// which we would only do if it was a valid remit file
			CopyConvertedEOBToServer();

			ShellExecute(NULL, "open", "notepad.exe", m_strOutputFile, "", SW_SHOW);

			return true;
		}
		else
		{
			MessageBox(m_pParentWnd->GetSafeHwnd(), "Please choose a valid assessment file.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		}

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2014-07-24 10:48) - PLID 62579 - moved these functions from the EbillingFormDlg to this class

// (d.singleton 2011-07-05) - PLID 44422 - fucntion to format dates
CString CAlbertaAssessmentParser::FormatDate(CString s)
{
	s.Insert(4, "-");
	s.Insert(7, "-");

	return s;
}

// (d.singleton 2011-07-05) - PLID 44422 - takes parsed chart number and returns patient name
// (j.jones 2014-07-24 14:55) - PLID 62579 - changed this to just fill our pRecord information
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
void CAlbertaAssessmentParser::FindPatient(AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->nPatientID = -1;
	pRecord->strPatientFullName = "";
	
	long nID = AsLong(_bstr_t(pRecord->strChartNumber));
	if (nID > 0) {
		_RecordsetPtr rsResults = CreateParamRecordset("SELECT ID, FullName From PersonT INNER JOIN PatientsT on PersonT.ID = PatientsT.PersonID "
			"WHERE UserDefinedID = {INT}", nID);
		if (!rsResults->eof)
		{
			pRecord->nPatientID = AdoFldLong(rsResults, "ID");
			pRecord->strPatientFullName = AdoFldString(rsResults, "FullName", "");
		}
		rsResults->Close();
	}
}

// (d.singleton 2011-07-05) - PLID 44422 - Takes ULI and returns provider name
// (j.jones 2014-07-24 17:01) - PLID 62579 - changed this to just fill our pRecord information
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
void CAlbertaAssessmentParser::FindProviderByULI(AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->nProviderID = -1;
	pRecord->strProviderFullName = "";

	if (!pRecord->strServiceProviderULI.IsEmpty()) {
		_RecordsetPtr rsResults = CreateParamRecordset("SELECT PersonT.ID, PersonT.FullName FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"WHERE REPLACE(REPLACE(NPI, '-', ''), ' ', '') = {STRING}", pRecord->strServiceProviderULI);
		if (!rsResults->eof)
		{
			pRecord->nProviderID = AdoFldLong(rsResults, "ID");
			pRecord->strProviderFullName = AdoFldString(rsResults, "FullName", "");
		}
		rsResults->Close();
	}
}

// (j.jones 2014-07-25 09:07) - PLID 62579 - added the ability to find the charge
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
void CAlbertaAssessmentParser::FindCharge(AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->nChargeID = -1;

	if (!pRecord->strClaimNum.IsEmpty()) {
		//this should be the transaction number we sent to Alberta,
		//which is tracked in ClaimHistoryDetailsT
		//TES 8/5/2014 - PLID 62580 - Added BillID
		_RecordsetPtr rsResults = CreateParamRecordset("SELECT ClaimHistoryDetailsT.ChargeID, ChargesT.BillID "
			"FROM ClaimHistoryDetailsT "
			"INNER JOIN LineItemT ON ClaimHistoryDetailsT.ChargeID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ClaimHistoryDetailsT.AlbertaTransNum = {STRING}", pRecord->strClaimNum);
		if (!rsResults->eof)
		{
			pRecord->nChargeID = AdoFldLong(rsResults, "ChargeID");
			//TES 8/5/2014 - PLID 62580 - Added BillID
			pRecord->nBillID = AdoFldLong(rsResults, "BillID");
		}
		rsResults->Close();
	}

	if (pRecord->nChargeID == -1) {
		//If we could not find the charge by transaction number, Alberta failed to send us
		//correct information. As a backup, we can try to find the charge by date and code.
		if (pRecord->dtServiceDate.GetStatus() == COleDateTime::valid && !pRecord->strServiceCode.IsEmpty()) {
			//TES 8/5/2014 - PLID 62580 - Added BillID
			_RecordsetPtr rsResults = CreateParamRecordset("SELECT ChargesT.ID, ChargesT.BillID "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND dbo.AsDateNoTime(LineItemT.Date) = dbo.AsDateNoTime({OLEDATETIME}) "
				"AND ChargesT.ItemCode = {STRING}",
				pRecord->dtServiceDate, pRecord->strServiceCode);
			if (!rsResults->eof)
			{
				pRecord->nChargeID = AdoFldLong(rsResults, "ID");
				//TES 8/5/2014 - PLID 62580 - Added BillID
				pRecord->nBillID = AdoFldLong(rsResults, "BillID");
			}
			rsResults->Close();
		}
	}
}

// (d.singleton 2011-07-05) - PLID 44422 - formats the assessment result
// (j.jones 2014-07-25 09:45) - PLID 62579 - this now returns a friendly string and updates
// the record to use the proper enumeration
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::AssessmentResultMain(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->eAssessmentResultAction_Main = aramInvalid;

	if (strResult.CompareNoCase("R") == 0)
	{
		pRecord->eAssessmentResultAction_Main = aramRefused;
		return "Refused";
	}
	else if (strResult.CompareNoCase("H") == 0)
	{
		pRecord->eAssessmentResultAction_Main = aramHeld;
		return "Held";
	}
	else if (strResult.CompareNoCase("A") == 0)
	{
		pRecord->eAssessmentResultAction_Main = aramApplied;
		return "Applied";
	}
	else
	{
		//find out how this happened
		ASSERT(FALSE);

		//our tracked enum will remain invalid, but for parsing purposes
		//we will simply report the value Alberta sent us
		return strResult;
	}
}

// (j.jones 2014-07-25 09:50) - PLID 62579 - added correct handling, this also returns a
// friendly string and updates the record to use the proper enumeration
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::AssessmentResultAdditional(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->eAssessmentResultAction_Additional = araaCurrent;

	if (strResult.CompareNoCase("R") == 0)
	{
		pRecord->eAssessmentResultAction_Additional = araaReversal;
		return "Reversal Of Previous Assessment";
	}
	else if (strResult.IsEmpty() || strResult == " ") {
		//no result, which is normal
		pRecord->eAssessmentResultAction_Additional = araaCurrent;
		return "";
	}
	else
	{
		//this would be an unknown code, so
		//find out how this happened
		ASSERT(FALSE);

		//our tracked enum will remain araaCurrent, but for parsing purposes
		//we will simply report the value Alberta sent us
		return strResult;
	}
}

// (j.jones 2014-07-25 09:55) - PLID 62579 - added enum translation for TransactionActionCode,
// returns a friendly string, updates the record with the proper enum
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::TransactionActionCode(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord)
{
	pRecord->eTransactionActionCode = tacInvalid;

	if (strResult.CompareNoCase("A") == 0)
	{
		pRecord->eTransactionActionCode = tacAdd;
		return "Claim Added";
	}
	else if (strResult.CompareNoCase("C") == 0)
	{
		pRecord->eTransactionActionCode = tacChange;
		return "Change Transaction";
	}
	else if (strResult.CompareNoCase("D") == 0)
	{
		pRecord->eTransactionActionCode = tacDelete;
		return "Delete Transaction";
	}
	else if (strResult.CompareNoCase("R") == 0)
	{
		pRecord->eTransactionActionCode = tacReassessment;
		return "Re-assessment Transaction";
	}
	else
	{
		//find out how this happened
		ASSERT(FALSE);

		//our tracked enum will remain invalid, but for parsing purposes
		//we will simply report the value Alberta sent us
		return strResult;
	}
}

// (d.singleton 2011-07-05) - PLID 44422 - fucntion to format money, get rid of leading 0's
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::FormatMoney(CString s)
{
	while (s.GetAt(0) == '0')
	{
		s.Delete(0, 1);
	}
	if (s.GetLength() < 1)
	{
		return "0.00";
	}
	else
	{
		s.Insert(s.GetLength() - 2, ".");
		return s;
	}
}

// (d.singleton 2011-07-05) - PLID 44422 - fucntion to return the reason(s) for the rejection
// (j.jones 2014-07-25 10:16) - PLID 62579 - this now fills the explanation reason(s) in the record,
// and returns a clean concatenated string for display purposes
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::FindAlbertaExplanationCodes(CString s, AlbertaAssessments::ChargeInfoPtr pRecord)
{
	CString strFinalText = "";
	pRecord->aryExplanationCodes.clear();

	for (int i = 0; i <= 30; i += 5)
	{
		CString strTempCode = s.Mid(i, i + 5);
		strTempCode.Trim();
		if (strTempCode != "")
		{
			AssessmentExplanation pExplanationCode;
			pExplanationCode.strCode = strTempCode;

			// (j.jones 2014-07-25 10:05) - PLID 62579 - added a map to reduce calls to the database
			pExplanationCode.strDescription = m_mapExplanationCodeToDesc[strTempCode];

			//do not trim this value, if the mapped value is a space it means
			//we already know this code does not exist

			if (pExplanationCode.strDescription.IsEmpty()) {
				//TES 9/18/2014 - PLID 62581 - The Title and Description are now stored in separate fields.
				_RecordsetPtr rsResults = CreateParamRecordset("SELECT Title, Description FROM AlbertaBillingErrorCodesT WHERE Code = {STRING}", pExplanationCode.strCode);
				if (!rsResults->eof)
				{
					pExplanationCode.strDescription = AdoFldString(rsResults, "Title") + "\r\n" + AdoFldString(rsResults, "Description");
					//map this description
					m_mapExplanationCodeToDesc[pExplanationCode.strCode] = pExplanationCode.strDescription;
				}
				else {
					//map a non-empty description so we don't call this query again
					m_mapExplanationCodeToDesc[pExplanationCode.strCode] = " ";
				}
				rsResults->Close();
			}

			//now trim the description so we eliminate spaces
			pExplanationCode.strDescription.Trim();

			//track this code/desc
			pRecord->aryExplanationCodes.push_back(pExplanationCode);
			
			if (!pExplanationCode.strDescription.IsEmpty()) {
				strFinalText += pExplanationCode.strCode + ": " + pExplanationCode.strDescription + "\n";
			}
			else
			{
				strFinalText += "No description found for explanation code \"" + pExplanationCode.strCode + "\".\n";
			}
		}
	}
	return strFinalText;
}

// (j.jones 2013-06-18 09:02) - PLID 57191 - gets the translation of a reassess reason
// (j.jones 2014-07-25 09:45) - PLID 62579 - this now returns a friendly string and updates
// the record to use the proper enumeration
//TES 8/5/2014 - PLID 62580 - Pass in a pointer
CString CAlbertaAssessmentParser::FindAlbertaReassessReason(CString strCode, AlbertaAssessments::ChargeInfoPtr pRecord) {

	pRecord->eTransactionReassessReason = trrNone;

	strCode.TrimLeft(); strCode.TrimRight();
	strCode.MakeUpper();

	if (strCode == "SRLE") {
		pRecord->eTransactionReassessReason = trrHSCChange;
		return "HSC retro-active change";
	}
	else if (strCode == "RTRO") {
		pRecord->eTransactionReassessReason = trrProviderChange;
		return "Service Provider retro-active change";
	}
	else if (strCode == "CHEL") {
		pRecord->eTransactionReassessReason = trrRecipientChange;
		return "Service Recipient retro-active change";
	}
	else if (strCode == "ARUL") {
		pRecord->eTransactionReassessReason = trrAnotherService;
		return "Affected by another Service - etc.";
	}
	else if (strCode == "PRRQ") {
		//this is not in the specs, we have no idea what it means, but we've found it in a file before
		pRecord->eTransactionReassessReason = trrPRRQ;
		return "PRRQ - Unknown Reason";
	}
	else if (strCode == "MASC") {
		//TES 8/8/2014 - PLID 62580 - Came across this in data from Hall-Findlay
		//this is not in the specs, we have no idea what it means, but we've found it in a file before
		pRecord->eTransactionReassessReason = trrMASC;
		return "MASC - Unknown Reason";
	}
	else if (strCode.IsEmpty()) {
		//no code, which is normal
		pRecord->eTransactionReassessReason = trrNone;
		return "";
	}
	else
	{
		//this would be an unknown code, so
		//find out how this happened
		ASSERT(FALSE);

		//our tracked enum will remain "none", but for parsing purposes
		//we will simply report the value Alberta sent us
		return strCode;
	}
}

//TES 8/5/2014 - PLID 62580 - Created
void CAlbertaAssessmentParser::CalcInsuranceIDs()
{
	//initialize to -1
	m_nLikelyInsuranceCoID = -1;

	CString strPatientIDs;
	foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_pChargeList)) {
	
		if (pCharge->nPatientID != -1) {

			//We're building two complex CASE statements here, one is going to give us a tally
			//of all insured parties matching our patient ID and plan name, the other will
			//give us a tally of all matching by patient ID only. Highest tally wins.

			CString str;
			str.Format("PatientID = %li", pCharge->nPatientID);
			if (!strPatientIDs.IsEmpty()) {
				strPatientIDs += " OR ";
			}
			strPatientIDs += str;
		}
	}

	if (!strPatientIDs.IsEmpty()) {

		//the where clause here makes it such that we can't parameterize this query
		_RecordsetPtr rs = CreateRecordset("SELECT InsuranceCoID, "
			"Sum(CASE WHEN %s THEN 1 ELSE 0 END) AS TotalPatients "
			"FROM InsuredPartyT "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"GROUP BY InsuranceCoID "
			"ORDER BY "
			"Sum(CASE WHEN %s THEN 1 ELSE 0 END) DESC",
			strPatientIDs, strPatientIDs);

		if (!rs->eof) {

			long nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID", -1);
			long nTotalPatients = AdoFldLong(rs, "TotalPatients", 0);

			//whatever we got for nInsuranceCoID, use it
			if (nInsuranceCoID != -1) {
				m_nLikelyInsuranceCoID = nInsuranceCoID;

				Log(FormatString("CalcInsuranceIDs: Selected Insurance Co ID %li due to %li matching patients.", nInsuranceCoID, nTotalPatients));
			}
		}
		rs->Close();
	}

	if (m_nLikelyInsuranceCoID == -1) {
		Log(FormatString("CalcInsuranceIDs: Failed to calculate an Insurance Co ID due to no matching patients."));
	}

	//now try to calculate insured party IDs

	//for each claim, find the insured party for the insurance co ID,
	//otherwise use their first insured party

	//loop through all claims
	foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_pChargeList)) {

		//initialize to -1
		pCharge->nInsuredPartyID = -1;
		// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
		pCharge->nRespTypeID = -1;
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		pCharge->bSubmitAsPrimary = FALSE;
		pCharge->nInsuranceCoID = -1;

		if (pCharge->nPatientID != -1) {

			//try to find an insured party, ANY insured party, but in order of priority of
			//matching by plan name and insurance company ID, just insurance company ID,
			//or any insured party, in order of responsibility

			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
				"InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, InsuranceCoID, InsuranceCoT.Name, RespTypeID, "
				"(CASE "
				"WHEN InsuranceCoID = {INT} THEN 1 "
				"ELSE 2 END) AS Priority "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"ORDER BY "
				"(CASE "
				"WHEN InsuranceCoID = {INT} THEN 1 "
				"ELSE 2 END), "
				"(CASE WHEN RespTypeID <> -1 THEN RespTypeID ELSE (SELECT Coalesce(Max(ID), 0) + 1 FROM RespTypeT) END) ",
				m_nLikelyInsuranceCoID,
				pCharge->nPatientID,
				m_nLikelyInsuranceCoID);

			if (!rs->eof) {

				//grab all the values and log accordingly

				pCharge->nInsuredPartyID = AdoFldLong(rs, "PersonID", -1);
				// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
				pCharge->nRespTypeID = AdoFldLong(rs, "RespTypeID", -1);
				// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
				pCharge->bSubmitAsPrimary = AdoFldBool(rs, "SubmitAsPrimary", 0);
				pCharge->nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID", -1);
				pCharge->strInsuranceCoName = AdoFldString(rs, "Name", "");
				long nRespTypeID = AdoFldLong(rs, "RespTypeID", -1);

				if (pCharge->nInsuredPartyID == -1) {
					Log(FormatString("CalcInsuranceIDs: Failed to calculate an Insured Party ID for Patient ID %li because only a -1 insured party ID was found.", pCharge->nPatientID));
				}
				else {

					BOOL bInsCoMatches = (pCharge->nInsuranceCoID == m_nLikelyInsuranceCoID);
					
					if (bInsCoMatches) {
						//we matched only on insurance co ID
						Log(FormatString("CalcInsuranceIDs: Calculated Insured Party ID %li (RespTypeID: %li) for Patient ID %li "
							"by matching Insurance Co ID %li (%s).",
							pCharge->nInsuredPartyID, nRespTypeID, pCharge->nPatientID,
							pCharge->nInsuranceCoID, pCharge->strInsuranceCoName));
					}
					else {
						//all purpose message, even if plan name matched we didn't search for it like this
						Log(FormatString("CalcInsuranceIDs: Warning: Calculated Insured Party ID %li (RespTypeID: %li) for Patient ID %li "
							"using non-matching Insurance Co ID %li (%s).",
							pCharge->nInsuredPartyID, nRespTypeID, pCharge->nPatientID,
							pCharge->nInsuranceCoID, pCharge->strInsuranceCoName));
					}
				}
			}
			else {
				Log(FormatString("CalcInsuranceIDs: Failed to calculate an Insured Party ID for Patient ID %li because no insured parties were found.", pCharge->nPatientID));
			}
			rs->Close();
		}
		else {
			Log(FormatString("CalcInsuranceIDs: Failed to calculate an Insured Party ID due to a Patient ID of -1."));
		}
	}
}

//TES 8/5/2014 - PLID 62580 - Added
void CAlbertaAssessmentParser::ClearAllEOBs() {

	try {

		//clear out the EOB
		
		m_pChargeList->clear();

		m_strFileName = "";
		
	}NxCatchAll("Error cleaning up EOB.");
}

//TES 8/5/2014 - PLID 62580 - Added
void CAlbertaAssessmentParser::GetChargeIDsByPatientID(long nPatientID, CArray<long, long> &aryUsedCharges)
{
	foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_pChargeList)) {
		if (pCharge->nPatientID == nPatientID) {
			aryUsedCharges.Add(pCharge->nChargeID);
		}
	}
}

//TES 9/15/2014 - PLID 62777 - Used to access charges based on the INT stored in the EOB datalist
AlbertaAssessments::ChargeInfoPtr CAlbertaAssessmentParser::GetChargeFromRawPointer(AlbertaAssessments::ChargeInfo* pRawPtr)
{
	foreach(AlbertaAssessments::ChargeInfoPtr pCharge, (*m_pChargeList)) {
		if (pCharge.get() == pRawPtr) {
			return pCharge;
		}
	}
	return AlbertaAssessments::ChargeInfoPtr(NULL);
}

// (j.jones 2014-10-08 16:48) - PLID 62579 - this function backs up the remit file
// and EOB.txt to the server's NexTech\ConvertedEOBs path, and also ensures that files
// > 365 days old are deleted
void CAlbertaAssessmentParser::CopyConvertedEOBToServer()
{
	try {

		//calculate the server's EOB path
		CString strServerPath = GetNxConvertedEOBsPath();

		//remove all files that are > 365 days old
		{
			CTime tm;
			//this assumes that at least the system's date is accurate
			CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(365, 0, 0, 0);

			//first remove EOB files
			CFileFind finder;
			BOOL bWorking = finder.FindFile(strServerPath ^ "AssessmentFile*.txt");
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
		//now copy our AssessmentFile.txt to the server
		{
			strNewFileName.Format("%s\\AssessmentFile_%s_%s.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));

			//it's very unlikely that this file will exist, but handle the case anyways
			int nCount = 0;
			while (DoesExist(strNewFileName)) {

				//try adding an index to the end
				nCount++;

				if (nCount > 10) {
					//something is seriously wrong
					ThrowNxException("Cannot copy assessment file to server, too many files with the name like: %s", strNewFileName);
				}

				strNewFileName.Format("%s\\AssessmentFile_%s_%s_%li.txt", strServerPath, dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"), nCount);
			}

			if (!CopyFile(m_strOutputFile, strNewFileName, TRUE)) {
				//failed
				ThrowNxException("Cannot copy assessment file to server, filename: %s", strNewFileName);
			}
		}

		// (b.spivey, October 9th, 2014) PLID 62701 - move it to our more permanent server storage. 
		m_strStoredParsedFile = CopyParsedEOBToServerStorage(strNewFileName);

		//now copy our remit file to the server
		{
			CString strFileNameNoExt = FileUtils::GetFileName(m_strFileName);
			int nDot = strFileNameNoExt.ReverseFind('.');
			if (nDot != -1) {
				strFileNameNoExt = strFileNameNoExt.Left(nDot);
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
CString CAlbertaAssessmentParser::GetStoredParsedFilePath()
{
	return m_strStoredParsedFile;
}