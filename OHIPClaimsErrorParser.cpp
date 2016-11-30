// OHIPClaimsErrorParser.cpp: implementation of the COHIPClaimsErrorParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OHIPClaimsErrorParser.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

// (j.jones 2008-07-07 11:18) - PLID 21968 - created

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COHIPClaimsErrorParser::COHIPClaimsErrorParser()
{
	m_strHealthNumbersT = "";
}

COHIPClaimsErrorParser::~COHIPClaimsErrorParser()
{

}

void COHIPClaimsErrorParser::OutputData(CString &OutputString, CString strNewData) {

	OutputString += strNewData;
}

CString COHIPClaimsErrorParser::ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim /*= FALSE*/)
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

COleCurrency COHIPClaimsErrorParser::ParseOHIPCurrency(CString strAmount, CString strSign)
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

BOOL COHIPClaimsErrorParser::ParseFile()
{	
	CWaitCursor pWait;

	try {

		/*
		//first get the file
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
		strOutputFile.Format("ClaimsErrorReport_%s.txt", m_strFileName);
		
		strOutputFile = GetNxTempPath() ^ strOutputFile;

		BOOL bIsValidFile = FALSE;	
		
		//open the file for reading
		if(!m_InputFile.Open(strFullFilePath,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The claims error report input file could not be found or opened.");
			return FALSE;
		}

		if(!m_OutputFile.Open(strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The claims error report output file could not be created.");
			return FALSE;
		}

		CWaitCursor pWait;

		Log("Importing claims error report file: " + strFullFilePath);

		CArchive arIn(&m_InputFile, CArchive::load);

		CString strLine;

		//we will track if any rejections were found
		BOOL bHasRejections = FALSE;

		CString strOutputString;
		strOutputString.Format("OHIP Claims Error Report\r\n"
			"======================================================================================================\r\n");
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

			//if the initial part of the line matches the OHIP specs,
			//assume it is a valid file
			if(strIdentifier.Left(2) == "HX" && strRecordID.GetLength() == 1) {		
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
					long nRecordID = atoi(strRecordID);
					if(strIdentifier.Left(2) == "HR" && nRecordID >= 1 && nRecordID <= 8) {
						
						//it seems to be a remit file
						AfxMessageBox("The file you are trying to parse is not an OHIP claims error file. "
							"It appears to be an OHIP remittance file, and should be imported through "
							"the Electronic Remittance feature in the Batch Payments tab.");
					}
					// (j.jones 2008-07-07 11:19) - PLID 30604 - see if it is a batch edit file
					else if(strIdentifier.Left(2) == "HB") {
						
						//it seems to be a batch edit file
						// (j.jones 2008-12-17 15:47) - PLID 31900 - the 'Format Batch Edit Report' button has been removed,
						// and you can only get to this code through the Report Manager, which should have not allowed us to
						// parse the wrong report.
						AfxMessageBox("The file you are trying to parse is not an OHIP remittance file. "
							"It appears to be an OHIP batch edit file. Please contact NexTech for assistance.");
					}
					else {
						AfxMessageBox("The file you are trying to parse is not an OHIP claims error file.");
					}
					return FALSE;
				}
			}

			//determine which record to parse
			char cRecordID = strRecordID[0];
			switch(cRecordID) {
				case '1':
					HX1_Header(strLine);
					break;
				case 'H':
					HXH_ClaimsHeader1(strLine);
					break;
				case 'R':
					HXR_ClaimsHeader2(strLine);
					break;
				case 'T':
					HXT_ClaimItem(strLine);
					break;
				case '8':
					HX8_ExplanCode(strLine);
					break;
				case '9':
					HX9_Trailer(strLine);
					break;
			}			

			// (j.jones 2009-02-03 17:50) - PLID 31900 - removed, it was pointless
			//PeekAndPump();
		}

		//leave if it's not a real file
		if(!bIsValidFile) {

			//close the file
			arIn.Close();
			m_InputFile.Close();
			m_OutputFile.Close();

			// (j.jones 2009-10-09 12:37) - PLID 35904 - drop our temp table
			if(!m_strHealthNumbersT.IsEmpty()) {
				ExecuteSql("DROP TABLE %s", m_strHealthNumbersT);
				m_strHealthNumbersT = "";
			}

			AfxMessageBox("The file you are trying to parse is not an OHIP claims error file.");
			return FALSE;
		}

		//close the file
		arIn.Close();
		m_InputFile.Close();
		m_OutputFile.Close();

		// (j.jones 2009-10-09 12:37) - PLID 35904 - drop our temp table
		if(!m_strHealthNumbersT.IsEmpty()) {
			ExecuteSql("DROP TABLE %s", m_strHealthNumbersT);
			m_strHealthNumbersT = "";
		}

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

	}NxCatchAll("Error parsing OHIP Claims Error file.");

	return FALSE;
}

/*
//Format Legend

A - Alphabetic

N - Numeric

X - Alphanumeric

D - Date (YYYYMMDD)

S - Spaces

Record Types:

HX1 - Group/Provider Header Record

HXH - Claims Header 1 Record

HXR - Claims Header 2 Record (RMB Claims Only)

HXT - Claim Item Record

HX8 - Explan Code Message Record (Optional)

HX9 - Group/Provider Trailer Record
*/

//HX1 - Group/Provider Header Record
void COHIPClaimsErrorParser::HX1_Header(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always '1'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Tech Spec Release Identifier	4 / 3 / X				Always 'V03'

	CString strTechSpecIdent = ParseElement(strLine, 4, 3);

	//MOH Office Code				7 / 1 / X				From batch header

	CString strMOHOfficeCode = ParseElement(strLine, 7, 1);

	//Reserved For MOH Use			8 / 10 / X				Spaces

	 CString strReserved1 = ParseElement(strLine, 8, 10);

	//Operator Number				18 / 6 / X				From batch header

	 CString strOperatorNumber = ParseElement(strLine, 18, 6);

	//Group Number					24 / 4 / X				From batch header

	 CString strGroupNumber = ParseElement(strLine, 24, 4);

	//Provider Number				28 / 6 / X				From batch header

	 CString strProviderNumber = ParseElement(strLine, 28, 6);

	//Specialty Code				34 / 2 / X				From batch header

	 CString strSpecialtyCode = ParseElement(strLine, 34, 2);

	 // (j.jones 2008-12-12 10:11) - PLID 32419 - altered this line to optionally show the Station Number in it
	 CString strHeader;
	 strHeader.Format("Provider Number: %s\tGroup Number: %s\tSpecialty Code: %s\t\tMOH Office Code: %s", strProviderNumber, strGroupNumber, strSpecialtyCode, strMOHOfficeCode);
	 OutputData(strOutputString, str);

	//Station Number				36 / 3 / X				Ministry assigned

	 CString strStationNumber = ParseElement(strLine, 36, 3);

	 if(!strStationNumber.IsEmpty()) {
		str.Format("\tStation Number: %s", strStationNumber);
		strHeader += str;
	 }

	 strHeader += "\r\n";
	 OutputData(strOutputString, strHeader);

	//Claim Process Date			39 / 8 / D				Date claim was processed

	 CString strClaimProcessDate = ParseElement(strLine, 39, 8);
	 if(strClaimProcessDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strClaimProcessDate.Left(4)), atoi(strClaimProcessDate.Mid(4, 2)), atoi(strClaimProcessDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			str.Format("\r\nClaim Process Date: %s\r\n", FormatDateTimeForInterface(dt, NULL, dtoDate));
			OutputData(strOutputString, str);
		}
	 }

	//Reserved For MOH Use			47 / 33 / X				Spaces

	 CString strReserved2 = ParseElement(strLine, 47, 33);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//HXH - Claims Header 1 Record	
void COHIPClaimsErrorParser::HXH_ClaimsHeader1(CString strLine)
{
	CString strOutputString, str;

	OutputData(strOutputString, "======================================================================================================\r\n\r\n");

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always 'H'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Health Number					4 / 10 / X				From claim header

	CString strHealthNumber = ParseElement(strLine, 4, 10);

	CString strPatientLine = "Patient Name: <Unknown>\t\tPatient ID: <Unknown>\r\n";

	if(!strHealthNumber.IsEmpty()) {
		//attempt to find the patient ID and name
		long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);

		// (j.jones 2009-10-09 12:26) - PLID 35904 - create a temp table with patient health numbers,
		// once per instance of this class
		if(m_strHealthNumbersT.IsEmpty()) {
			m_strHealthNumbersT.Format("#TempHealthNumbers%lu", GetTickCount());
			ExecuteSql("CREATE TABLE %s (PatientID int, HealthCard nvarchar(255)) "
				"INSERT INTO %s (PatientID, HealthCard) "
				"SELECT PersonT.ID, Replace(Replace(CustomFieldDataT.TextParam, '-',''), ' ', '') AS HealthCard "
				"FROM CustomFieldDataT "
				"INNER JOIN PersonT ON CustomFieldDataT.PersonID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE CustomFieldDataT.FieldID = %li", m_strHealthNumbersT, m_strHealthNumbersT, nHealthNumberCustomField);
		}

		//this field will correspond to a patient's custom text field
		// (j.jones 2008-12-05 17:47) - PLID 32349 - replace hyphens and spaces in our search
		CString strHealthNumberToSearch = strHealthNumber;
		strHealthNumberToSearch.Replace("-","");
		strHealthNumberToSearch.Replace(" ","");
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT UserDefinedID, Last + ', ' + First + ' ' + Middle AS PatientName "
			"FROM %s "
			"INNER JOIN PersonT ON %s.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE HealthCard = {STRING}", m_strHealthNumbersT, m_strHealthNumbersT), strHealthNumberToSearch);
		if(!rs->eof) {

			long nUserDefinedID = AdoFldLong(rs, "UserDefinedID");
			CString strPatientName = AdoFldString(rs, "PatientName", "");

			strPatientLine.Format("Patient Name: %s%s\tPatient ID: %li", strPatientName, strPatientName.GetLength() < 14 ? "\t" : "", nUserDefinedID);
		}
		rs->Close();
	}

	//Version Code					14 / 2 / X				From claim header

	CString strVersionCode = ParseElement(strLine, 14, 2);

	//Patient Birthdate				16 / 8 / X				From claim header

	CString strPatientBirthdate = ParseElement(strLine, 16, 8);
	if(strPatientBirthdate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strPatientBirthdate.Left(4)), atoi(strPatientBirthdate.Mid(4, 2)), atoi(strPatientBirthdate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			str.Format("\t\tPatient Birthdate: %s", FormatDateTimeForInterface(dt, NULL, dtoDate));
			strPatientLine += str;
		}	
	}

	// (j.jones 2008-12-12 10:25) - PLID 32419 - rearranged the way the patient information is output
	strPatientLine += "\r\n";
	OutputData(strOutputString, strPatientLine);

	str.Format("Health Number: %s\t\tVersion Code: %s\r\n", strHealthNumber, strVersionCode);
	OutputData(strOutputString, str);

	//Accounting Number				24 / 8 / X				From claim header

	CString strAccountingNumber = ParseElement(strLine, 24, 8);

	//Payment Program				32 / 3 / X				From claim header

	CString strPaymentProgram = ParseElement(strLine, 32, 3);

	//Payee							35 / 1 / X				From claim header

	CString strPayee = ParseElement(strLine, 35, 1);

	str.Format("Accounting Number: %s\tPayment Program: %s\tPayee: %s\r\n", strAccountingNumber, strPaymentProgram, strPayee);
	OutputData(strOutputString, str);

	//Referring Provider Number		36 / 6 / X				From claim header

	CString strReferringProviderNum = ParseElement(strLine, 36, 6);

	CString strRefPhyRefLabLine;
	if(!strReferringProviderNum.IsEmpty()) {
		str.Format("Referring Provider Number: %s", strReferringProviderNum);
		strRefPhyRefLabLine += str;
	}

	//Facility Number				42 / 4 / X				From claim header

	CString strFacilityNumber = ParseElement(strLine, 42, 4);

	CString strLocationIDs;
	if(!strFacilityNumber.IsEmpty()) {
		str.Format("Facility Number: %s", strFacilityNumber);
		strLocationIDs += str;
	}

	//Patient Admission Date		46 / 8 / X				From claim header
	
	CString strPatientAdmissionDate = ParseElement(strLine, 46, 8);	

	//Referring Lab Licence Number	54 / 4 / X				From claim header

	CString strReferringLabLicenceNumber = ParseElement(strLine, 54, 4);

	if(!strReferringLabLicenceNumber.IsEmpty()) {
		str.Format("Referring Lab Licence Number: %s", strReferringLabLicenceNumber);
		if(!strRefPhyRefLabLine.IsEmpty()) {
			strRefPhyRefLabLine += "\t";
		}
		strRefPhyRefLabLine += str;
	}

	//Location Code					58 / 4 / X				From claim header

	CString strLocationCode = ParseElement(strLine, 58, 4);

	if(!strLocationCode.IsEmpty()) {
		str.Format("Location Code: %s", strLocationCode);
		if(!strLocationIDs.IsEmpty()) {
			strLocationIDs += "\t";
		}
		strLocationIDs += str;
	}

	// (j.jones 2008-12-12 10:34) - PLID 32419 - combined the location and facility IDs into one line
	if(!strLocationIDs.IsEmpty()) {
		strLocationIDs += "\r\n";		
		OutputData(strOutputString, strLocationIDs);
	}

	if(strPatientAdmissionDate.GetLength() == 8) {

		COleDateTime dt;
		dt.SetDate(atoi(strPatientAdmissionDate.Left(4)), atoi(strPatientAdmissionDate.Mid(4, 2)), atoi(strPatientAdmissionDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			str.Format("Patient Admission Date: %s\r\n", FormatDateTimeForInterface(dt, NULL, dtoDate));
			OutputData(strOutputString, str);
		}
	}

	// (j.jones 2008-12-12 10:30) - PLID 32419 - combined the referring physician and referring lab license IDs into one line
	if(!strRefPhyRefLabLine.IsEmpty()) {
		strRefPhyRefLabLine += "\r\n";		
		OutputData(strOutputString, strRefPhyRefLabLine);
	}

	//Reserved For MOH Use			62 / 3 / X				Spaces

	CString strReserved = ParseElement(strLine, 62, 3);

	BOOL bHasErrors = FALSE;

	//Error Code 1					65 / 3 / X				Refer to error code list

	CString strError1 = ParseElement(strLine, 65, 3);

	if(!strError1.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError1, GetErrorDescription(strError1));
		OutputData(strOutputString, str);
	}

	//Error Code 2					68 / 3 / X				Refer to error code list

	CString strError2 = ParseElement(strLine, 68, 3);

	if(!strError2.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError2, GetErrorDescription(strError2));
		OutputData(strOutputString, str);
	}

	//Error Code 3					71 / 3 / X				Refer to error code list

	CString strError3 = ParseElement(strLine, 71, 3);

	if(!strError3.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError3, GetErrorDescription(strError3));
		OutputData(strOutputString, str);
	}

	//Error Code 4					74 / 3 / X				Refer to error code list

	CString strError4 = ParseElement(strLine, 74, 3);

	if(!strError4.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError4, GetErrorDescription(strError4));
		OutputData(strOutputString, str);
	}

	//Error Code 5					77 / 3 / X				Refer to error code list

	CString strError5 = ParseElement(strLine, 77, 3);

	if(!strError5.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError5, GetErrorDescription(strError5));
		OutputData(strOutputString, str);
	}

	if(bHasErrors) {
		//add one blank line after the errors
		OutputData(strOutputString, "\r\n");
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//HXR - Claims Header 2 Record (RMB Claims Only)
void COHIPClaimsErrorParser::HXR_ClaimsHeader2(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always 'R'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Registration Number			4 / 12 / X				From claims header 2	

	CString strRegistrationNum = ParseElement(strLine, 4, 12);

	//Patient Last Name				16 / 9 / X				From claims header 2

	CString strPatientLast = ParseElement(strLine, 16, 9);

	//Patient First Name			25 / 5 / X				From claims header 2

	CString strPatientFirst = ParseElement(strLine, 25, 5);

	//Patient Sex					30 / 1 / X				From claims header 2

	CString strPatientSex = ParseElement(strLine, 30, 1);

	//Province Code					31 / 2 / X				From claims header 2

	CString strProvinceCode = ParseElement(strLine, 31, 2);

	// (j.jones 2008-12-12 10:40) - PLID 32419 - output all of these fields cleanly on two lines
	CString strLine1, strLine2;
	if(!strPatientLast.IsEmpty() || !strPatientFirst.IsEmpty()) {
		str.Format("Patient Name: %s, %s", strPatientLast, strPatientFirst);
		strLine1 += str;
	}
	if(!strPatientSex.IsEmpty()) {
		str.Format("Patient Gender: %s", strPatientSex);
		if(!strLine1.IsEmpty()) {
			strLine1 += "\t";
		}
		strLine1 += str;
	}
	if(!strRegistrationNum.IsEmpty()) {
		str.Format("Registration Number: %s", strRegistrationNum);
		strLine2 += str;
	}
	if(!strProvinceCode.IsEmpty()) {
		str.Format("Province Code: %s", strProvinceCode);
		if(!strLine2.IsEmpty()) {
			strLine2 += "\t";
		}
		strLine2 += str;
	}

	if(!strLine1.IsEmpty()) {
		strLine1 += "\r\n";
		OutputData(strOutputString, strLine1);
	}
	if(!strLine2.IsEmpty()) {
		strLine2 += "\r\n";
		OutputData(strOutputString, strLine2);
	}

	//Reserved For MOH Use			33 / 32 / X				Spaces

	CString strReserved = ParseElement(strLine, 33, 32);

	BOOL bHasErrors = FALSE;

	//Error Code 1					65 / 3 / X				Refer to error code list

	CString strError1 = ParseElement(strLine, 65, 3);

	if(!strError1.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError1, GetErrorDescription(strError1));
		OutputData(strOutputString, str);
	}

	//Error Code 2					68 / 3 / X				Refer to error code list

	CString strError2 = ParseElement(strLine, 68, 3);

	if(!strError2.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError2, GetErrorDescription(strError2));
		OutputData(strOutputString, str);
	}

	//Error Code 3					71 / 3 / X				Refer to error code list

	CString strError3 = ParseElement(strLine, 71, 3);

	if(!strError3.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError3, GetErrorDescription(strError3));
		OutputData(strOutputString, str);
	}

	//Error Code 4					74 / 3 / X				Refer to error code list

	CString strError4 = ParseElement(strLine, 74, 3);

	if(!strError4.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError4, GetErrorDescription(strError4));
		OutputData(strOutputString, str);
	}

	//Error Code 5					77 / 3 / X				Refer to error code list

	CString strError5 = ParseElement(strLine, 77, 3);

	if(!strError5.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError5, GetErrorDescription(strError5));
		OutputData(strOutputString, str);
	}

	if(bHasErrors) {
		//add one blank line after the errors
		OutputData(strOutputString, "\r\n");
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//HXT - Claim Item Record
void COHIPClaimsErrorParser::HXT_ClaimItem(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always 'T'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Service Code					4 / 5 / X				From claim item record

	CString strServiceCode = ParseElement(strLine, 4, 5);


	//Reserved For MOH Use			9 / 2 / X				Spaces

	CString strReserved1 = ParseElement(strLine, 9, 2);

	//Fee Submitted					11 / 6 / X				From claim item record

	CString strFeeSubmitted = ParseElement(strLine, 11, 6);
	COleCurrency cyFeeSubmitted = ParseOHIPCurrency(strFeeSubmitted, " ");

	//Number Of Services			17 / 2 / X				From claim item record

	CString strNumberOfServices = ParseElement(strLine, 17, 2);

	CString strServiceLine;	

	//Service Date					19 / 8 / X				From claim item record

	CString strServiceDate = ParseElement(strLine, 19, 8);
	CString strDateText;
	if(strServiceDate.GetLength() == 8) {
		
		COleDateTime dt;
		dt.SetDate(atoi(strServiceDate.Left(4)), atoi(strServiceDate.Mid(4, 2)), atoi(strServiceDate.Right(2)));
		if(dt.GetStatus() != COleDateTime::invalid) {

			strDateText.Format("Service Date: %s\t", FormatDateTimeForInterface(dt, NULL, dtoDate));
		}
	}

	//Diagnostic Code				27 / 4 / X				From claim item record

	CString strDiagnosticCode = ParseElement(strLine, 27, 4);

	CString strDiagText;
	if(!strDiagnosticCode.IsEmpty()) {
		strDiagText.Format("\tDiagnostic Code: %s", strDiagnosticCode);
	}

	// (j.jones 2008-12-12 10:51) - PLID 32419 - reformatted these fields to output everything on one line, preceded by a blank line
	strServiceLine.Format("\r\nService Code: %s\t%sFee Submitted: %s\tQuantity: %li%s\r\n", strServiceCode, strDateText, FormatCurrencyForInterface(cyFeeSubmitted, TRUE, TRUE, TRUE), atoi(strNumberOfServices), strDiagText);
	OutputData(strOutputString, strServiceLine);

	//Reserved For MOH Use			31 / 32 / X				Spaces

	CString strReserved2 = ParseElement(strLine, 31, 32);

	//Explan Code					63 / 2 / X				EDT error report explanation code

	CString strExplanCode = ParseElement(strLine, 63, 2);

	if(!strExplanCode.IsEmpty()) {
		str.Format("Explanation Code: %s\r\n", strExplanCode);
		OutputData(strOutputString, str);
	}

	BOOL bHasErrors = FALSE;

	//Error Code 1					65 / 3 / X				Refer to error code list

	CString strError1 = ParseElement(strLine, 65, 3);

	if(!strError1.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError1, GetErrorDescription(strError1));
		OutputData(strOutputString, str);
	}

	//Error Code 2					68 / 3 / X				Refer to error code list

	CString strError2 = ParseElement(strLine, 68, 3);

	if(!strError2.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError2, GetErrorDescription(strError2));
		OutputData(strOutputString, str);
	}

	//Error Code 3					71 / 3 / X				Refer to error code list

	CString strError3 = ParseElement(strLine, 71, 3);

	if(!strError3.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError3, GetErrorDescription(strError3));
		OutputData(strOutputString, str);
	}

	//Error Code 4					74 / 3 / X				Refer to error code list

	CString strError4 = ParseElement(strLine, 74, 3);

	if(!strError4.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError4, GetErrorDescription(strError4));
		OutputData(strOutputString, str);
	}

	//Error Code 5					77 / 3 / X				Refer to error code list

	CString strError5 = ParseElement(strLine, 77, 3);

	if(!strError5.IsEmpty()) {
		if(!bHasErrors) {
			//add one blank line before errors
			OutputData(strOutputString, "\r\n");
		}
		bHasErrors = TRUE;
		str.Format(" *** Error: %s - %s\r\n", strError5, GetErrorDescription(strError5));
		OutputData(strOutputString, str);
	}

	if(bHasErrors) {
		//add one blank line after the errors
		OutputData(strOutputString, "\r\n");
	}

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//HX8 - Explan Code Message Record (Optional)
void COHIPClaimsErrorParser::HX8_ExplanCode(CString strLine)
{
	CString strOutputString, str;

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always '8'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Explan Code					4 / 2 / X				Error report explanatory code

	CString strExplanCode = ParseElement(strLine, 4, 2);

	CString strExplanationLine;
	if(!strExplanCode.IsEmpty()) {
		strExplanationLine.Format("Explanatory Code: %s", strExplanCode);
	}

	//Explan Description			6 / 55 / X				Explanatory code description

	CString strExplanDesc = ParseElement(strLine, 6, 55);

	if(!strExplanDesc.IsEmpty()) {
		str.Format("Explanatory Description: %s", strExplanDesc);
		if(!strExplanationLine.IsEmpty()) {
			strExplanationLine += "\t";
		}
		strExplanationLine += str;
	}

	// (j.jones 2008-12-12 11:00) - PLID 32419 - combined these fields into one line
	if(!strExplanationLine.IsEmpty()) {
		strExplanationLine += "\r\n";
		OutputData(strOutputString, strExplanationLine);
	}

	//Reserved For MOH Use			61 / 19 / X				Spaces

	CString strReserved = ParseElement(strLine, 61, 19);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//HX9 - Group/Provider Trailer Record
void COHIPClaimsErrorParser::HX9_Trailer(CString strLine)
{
	CString strOutputString, str;

	OutputData(strOutputString, "======================================================================================================\r\n\r\n");

	//Field Name					Start Pos./Len/Format	Field Description

	//Transaction Identifier		1 / 2 / X				Always 'HX'

	CString strTransactionIdent = ParseElement(strLine, 1, 2);

	//Record Identifier				3 / 1 / X				Always '9'

	CString strRecordIdentifier = ParseElement(strLine, 3, 1);

	//Header 1 Count				4 / 7 / N				Count of HXH Records

	CString strHXHCount = ParseElement(strLine, 4, 7);

	long nClaimCount = atoi(strHXHCount);

	if(nClaimCount > 0) {
		str.Format("*********************************************\r\n"
				   "     Total Claims Rejected: %li\r\n"
				   "*********************************************\r\n\r\n", nClaimCount);
		OutputData(strOutputString, str);
	}

	//Header 2 Count				11 / 7 / N				Count of HXR Records

	CString strHXRCount = ParseElement(strLine, 11, 7);

	//Item Count					18 / 7 / N				Count of HXT Records

	CString strHXTCount = ParseElement(strLine, 18, 7);

	//Message Count					25 / 7 / N				Count of HX8 Records

	CString strHX8Count = ParseElement(strLine, 25, 7);

	//Reserved For MOH Use			32 / 48 / X				Spaces

	CString strReserved = ParseElement(strLine, 32, 48);

	if(!strOutputString.IsEmpty()) {
		m_OutputFile.Write(strOutputString ,strOutputString.GetLength());
	}
}

//takes in an error code and returns the full description
CString COHIPClaimsErrorParser::GetErrorDescription(CString strErrorCode)
{
	strErrorCode.MakeUpper();

	CString strErrorDesc = "<Unknown Error Code>";

	if(strErrorCode == "AC4") {
		strErrorDesc = "A valid Referring/Requisitioning Health Care Provider number must be present for this service code.";
	}
	else if(strErrorCode == "ADM") {
		strErrorDesc = "Emergency equivalent/other visits.";
	}
	else if(strErrorCode == "AH8") {
		strErrorDesc = "In-Patient Admission Date and/or Facility Number are missing and are required for this service code.";
	}
	else if(strErrorCode == "AH9") {
		strErrorDesc = "In-Patient diagnostic service is not allowed with a hospital assessment.";
	}
	else if(strErrorCode == "A2A") {
		strErrorDesc = "Patient is underage or overage for this service code.";
	}
	else if(strErrorCode == "A2B") {
		strErrorDesc = "This service is not normally performed for this sex. Please check your records.";
	}
	else if(strErrorCode == "A3E") {
		strErrorDesc = "No such service code for date of service.";
	}
	else if(strErrorCode == "A3F") {
		strErrorDesc = "No fee exists for this service code on this date of service.";
	}
	else if(strErrorCode == "A34") {
		strErrorDesc = "Multiple duplicate claims.";
	}
	else if(strErrorCode == "A4D") {
		strErrorDesc = "Invalid specialty for this service code.";
	}
	else if(strErrorCode == "EH1") {
		strErrorDesc = "Service date is prior to eligibility start date.";
	}
	else if(strErrorCode == "EH2") {
		strErrorDesc = "Version code does not match health number version code for service date.";
	}
	else if(strErrorCode == "EH4") {
		strErrorDesc = "Service date is greater than eligibility end date.";
	}
	else if(strErrorCode == "EH5") {
		strErrorDesc = "Service date is not within an eligible period.";
	}
	else if(strErrorCode == "EPA") {
		strErrorDesc = "PCN billing not approved.";
	}
	else if(strErrorCode == "EPC") {
		strErrorDesc = "Patient not rostered / rostered to another PCN";
	}
	else if(strErrorCode == "EPD") {
		strErrorDesc = "Roster/HRR payment discrepancy.";
	}
	else if(strErrorCode == "EQ1") {
		strErrorDesc = "Solo or affiliated Health Care Provider is not registered with the Ministry of Health.";
	}
	else if(strErrorCode == "EQ2") {
		strErrorDesc = "Specialty Code is inactive or not registered on date of service.";
	}
	else if(strErrorCode == "EQ3") {
		strErrorDesc = "Health Care Provider is registered as OPTED-IN for date or service. Claim submitted as Pay Patient.";
	}
	else if(strErrorCode == "EQ4") {
		strErrorDesc = "Health Care Provider is registered as OPTED-OUT for date or service. Claim submitted as Pay Provider.";
	}
	else if(strErrorCode == "EQ6") {
		strErrorDesc = "Referring/Requisitioning Health Care Provider Number is not registered with the Ministry of Health.";
	}
	else if(strErrorCode == "EQ9") {
		strErrorDesc = "Laboratory Licence Number is not registered with the Ministry of Health.";
	}
	else if(strErrorCode == "EQB") {
		strErrorDesc = "Solo Health Care Provider Number is not actively registered with the Ministry of Health on this date of service.";
	}
	else if(strErrorCode == "EQC") {
		strErrorDesc = "Group Number is not registered with the Ministry of Health.";
	}
	else if(strErrorCode == "EQD") {
		strErrorDesc = "Group Number is not actively registered with the Ministry of Health on this date of service.";
	}
	else if(strErrorCode == "EQE") {
		strErrorDesc = "Health Care Provider is not registered with the Ministry of Health as an affiliate of this Group on this date of service.";
	}
	else if(strErrorCode == "EQF") {
		strErrorDesc = "Health Care Provider is not actively registered with the Ministry of Health as an affiliate of this Group on this date of service.";
	}
	else if(strErrorCode == "EQG") {
		strErrorDesc = "Referring Laboratory is not registered with the Ministry of Health.";
	}
	else if(strErrorCode == "ERF") {
		strErrorDesc = "Referring Physician number is currently ineligible for referrals.";
	}
	else if(strErrorCode == "VJ5") {
		strErrorDesc = "Date of Service is an invalid value.";
	}
	else if(strErrorCode == "VJ7") {
		strErrorDesc = "Date of Service is six (6) months prior to Ministry of Health system run date.";
	}
	else if(strErrorCode == "V02") {
		strErrorDesc = "Incorrect MOH office code. Missing / not D, E, F, G, J, N, P, R, or U.";
	}
	else if(strErrorCode == "V05") {
		strErrorDesc = "Date of Service is greater then Ministry of Health system run date.";
	}
	else if(strErrorCode == "V07") {
		strErrorDesc = "Health Care Provider number is missing / not 6 numerics.";
	}
	else if(strErrorCode == "V08") {
		strErrorDesc = "Specialty code is missing or invalid.";
	}
	else if(strErrorCode == "V09") {
		strErrorDesc = "Referring Health Care Provider number is missing or invalid.";
	}
	else if(strErrorCode == "V10") {
		strErrorDesc = "Patient's last name is missing or invalid.";
	}
	else if(strErrorCode == "V12") {
		strErrorDesc = "Patient's first name is missing or invalid.";
	}
	else if(strErrorCode == "V13") {
		strErrorDesc = "Patient's birthdate is missing or invalid.";
	}
	else if(strErrorCode == "V14") {
		strErrorDesc = "Patient Sex must be 1 (male) or 2 (female).";
	}
	else if(strErrorCode == "V16") {
		strErrorDesc = "Diagnostic code is invalid or not allowed.";
	}
	else if(strErrorCode == "V17") {
		strErrorDesc = "Payee must be P (Provider) or S (Patient).";
	}
	else if(strErrorCode == "V18") {
		strErrorDesc = "In-Patient admission date is an invalid value.";
	}
	else if(strErrorCode == "V19") {
		strErrorDesc = "Chiropractic Diagnostic Code is missing/invalid.";
	}
	else if(strErrorCode == "V20") {
		strErrorDesc = "Service code is invalid for the patient's age.";
	}
	else if(strErrorCode == "V21") {
		strErrorDesc = "Diagnostic code is required for this service.";
	}
	else if(strErrorCode == "V22") {
		strErrorDesc = "Diagnostic code is not a valid code.";
	}
	else if(strErrorCode == "V23") {
		strErrorDesc = "Service code ends in B or C and the number of services is not greater than 01.";
	}
	else if(strErrorCode == "V28") {
		strErrorDesc = "Facility Number is not a valid number.";
	}
	else if(strErrorCode == "V30") {
		strErrorDesc = "FSC/DX Code Combination NAB.";
	}
	else if(strErrorCode == "V31") {
		strErrorDesc = "Missing all of the following: Group Number, Health Care Provider Number, Specialty Code, OHIP Number.";
	}
	else if(strErrorCode == "V34") {
		strErrorDesc = "Service code is not valid with this Health Care Provider number.";
	}
	else if(strErrorCode == "V36") {
		strErrorDesc = "Check input criteria required for sessional billing.";
	}
	else if(strErrorCode == "V39") {
		strErrorDesc = "Number of Items exceeds the maximum (99).";
	}
	else if(strErrorCode == "V40") {
		strErrorDesc = "Service code is missing or is in an invalid format.";
	}
	else if(strErrorCode == "V41") {
		strErrorDesc = "Fee Submitted is missing or is in an invalid format.";
	}
	else if(strErrorCode == "V42") {
		strErrorDesc = "Number of services is missing or is in an invalid format.";
	}
	else if(strErrorCode == "V47") {
		strErrorDesc = "Fee Submitted is not evenly divisible by number of services.";
	}
	else if(strErrorCode == "V51") {
		strErrorDesc = "Invalid location code - must be blank or four numerics.";
	}
	else if(strErrorCode == "V63") {
		strErrorDesc = "Referring Laboratory number must start with 5. (5###)";
	}
	else if(strErrorCode == "V70") {
		strErrorDesc = "The Date of Service is greater than the file/batch creation date.";
	}
	else if(strErrorCode == "AI4") {
		strErrorDesc = "Records show this service has been rendered by another practitioner, group, or IHF.";
	}
	else if(strErrorCode == "EF1") {
		strErrorDesc = "IHF number not approved for billing on the date specified.";
	}
	else if(strErrorCode == "EF2") {
		strErrorDesc = "IHF not licensed or grandfathered to bill FSC on the date specified.";
	}
	else if(strErrorCode == "EF3") {
		strErrorDesc = "Insured services are excluded from IHF billings.";
	}
	else if(strErrorCode == "EF4") {
		strErrorDesc = "Provider is not approved to bill IHF fee on date specified.";
	}
	else if(strErrorCode == "EF5") {
		strErrorDesc = "IHF practitioner 991000 is not allowed to bill insured services.";
	}
	else if(strErrorCode == "EF7") {
		strErrorDesc = "Referring Physician number is required for the IHF facility fee billed.";
	}
	else if(strErrorCode == "EF8") {
		strErrorDesc = "I Service Codes are exclusive to IHFs.";
	}
	else if(strErrorCode == "EF9") {
		strErrorDesc = "Mobile site number required";
	}
	else if(strErrorCode == "R01") {
		strErrorDesc = "Missing registration number.";
	}
	else if(strErrorCode == "R02") {
		strErrorDesc = "Number of digits disagrees with the corresponding Province Code.";
	}
	else if(strErrorCode == "R03") {
		strErrorDesc = "Province code missing or invalid.";
	}
	else if(strErrorCode == "R04") {
		strErrorDesc = "Fee Schedule Code excluded from RMB.";
	}
	else if(strErrorCode == "R05") {
		strErrorDesc = "ON (Ontario Province Code) not valid for RMB.";
	}
	else if(strErrorCode == "R06") {
		strErrorDesc = "Wrong Health Care Provider for RMB (begins with 3, 4, 8, or 9).";
	}
	else if(strErrorCode == "R07") {
		strErrorDesc = "Invalid pay type for RMB (must be P).";
	}
	else if(strErrorCode == "R08") {
		strErrorDesc = "Invalid referral number.";
	}
	else if(strErrorCode == "R09") {
		strErrorDesc = "Claim Header-2 is missing and the payment program is RMB.";
	}
	else if(strErrorCode == "VW1") {
		strErrorDesc = "This service not valid for WCB.";
	}
	else if(strErrorCode == "VHO" || strErrorCode == "VH0") {
		strErrorDesc = "Claim Header-2 present on MRI claim submitted with Health Number in Claim Header-1.";
	}
	else if(strErrorCode == "VH1") {
		strErrorDesc = "Health Number is missing or invalid.";
	}
	else if(strErrorCode == "VH2") {
		strErrorDesc = "Health Number is not present.";
	}
	else if(strErrorCode == "VH3") {
		strErrorDesc = "The payment program is missing or not equal to HCP, RMB, WCB.";
	}
	else if(strErrorCode == "VH4") {
		strErrorDesc = "Invalid Version Code.";
	}
	else if(strErrorCode == "VH5") {
		strErrorDesc = "Claim Header-2 is missing.";
	}
	else if(strErrorCode == "VH8") {
		strErrorDesc = "Date of birth does not match the Health Number submitted.";
	}
	else if(strErrorCode == "VH9") {
		strErrorDesc = "Health Number is not registered with Ministry of Health.";
	}
	// (j.jones 2009-04-07 15:22) - PLID 33891 - supported new error HM
	else if(strErrorCode == "HM") {
		strErrorDesc = "Invalid Hospital Master Number on Date of Service";
	}

	return strErrorDesc;
}