// ANSIReportParserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ANSIReportParserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CANSIReportParserDlg dialog

CANSIReportParserDlg::CANSIReportParserDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CANSIReportParserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CANSIReportParserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CANSIReportParserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CANSIReportParserDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_PARSE, m_btnParseFile);
	DDX_Control(pDX, IDC_FILE_INPUT_NAME, m_nxeditFileInputName);
	DDX_Control(pDX, IDC_FILE_OUTPUT_NAME, m_nxeditFileOutputName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CANSIReportParserDlg, CNxDialog)
	//{{AFX_MSG_MAP(CANSIReportParserDlg)
	ON_BN_CLICKED(IDC_BROWSE_IN, OnBrowseIn)
	ON_BN_CLICKED(IDC_PARSE, OnParse)
	ON_BN_CLICKED(IDC_BROWSE_OUT, OnBrowseOut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CANSIReportParserDlg message handlers

BOOL CANSIReportParserDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		
		// (j.jones 2008-05-07 10:49) - PLID 29854 - added nxiconbuttons for modernization
		m_btnClose.AutoSet(NXB_CLOSE);

	}NxCatchAll("Error in CANSIReportParserDlg::OnInitDialog");
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CANSIReportParserDlg::OnBrowseIn() 
{
	CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST);
	if (BrowseFiles.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_FILE_INPUT_NAME,BrowseFiles.GetPathName());
}

void CANSIReportParserDlg::OnBrowseOut() 
{
	CFileDialog BrowseFiles(FALSE,NULL,"report.txt");
	if (BrowseFiles.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_FILE_OUTPUT_NAME,BrowseFiles.GetPathName());
}

void CANSIReportParserDlg::OnParse() 
{
	CWaitCursor pWait;

	CString strNameIn, strNameOut;
	
	//get the file names
	GetDlgItemText(IDC_FILE_INPUT_NAME,strNameIn);
	GetDlgItemText(IDC_FILE_OUTPUT_NAME,strNameOut);
	
	try {

		BOOL bIsValidFile = FALSE;

		//open the file for reading
		if(!m_InputFile.Open(strNameIn,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The input file could not be found or opened. Please double-check your path and filename.");
			return;
		}

		//open the file for writing
		if(!m_OutputFile.Open(strNameOut,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The output file could not be created. Please double-check your path and filename.");
			return;
		}

		const int LEN_16_KB = 16384;
		CString strIn;	//input string
		long iFileSize = m_InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
		strIn.ReleaseBuffer(iFileSize);
		while(iFileSize > 0) {

			CString strSegment,	//current segment
					strElement;	//current element

			strIn.Replace("\r","");
			strIn.Replace("\n","");

			while((strIn.GetLength() > 0) && (strIn.Find("~") != -1)) {

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
					bIsValidFile = ANSI_ST(strSegment);

					if(!bIsValidFile) {
						m_InputFile.Close();
						m_OutputFile.Close();
						return;
					}
				}
				else if(strIdent == "AK1")
					ANSI_AK1(strSegment);
				else if(strIdent == "AK2")
					ANSI_AK2(strSegment);
				else if(strIdent == "AK3")
					ANSI_AK3(strSegment);
				else if(strIdent == "AK4")
					ANSI_AK4(strSegment);
				else if(strIdent == "AK5")
					ANSI_AK5(strSegment);
				else if(strIdent == "AK9")
					ANSI_AK9(strSegment);
				else if(strIdent == "SE")
					ANSI_SE(strSegment);
				else if(strIdent == "GE")
					ANSI_GE(strSegment);
				else if(strIdent == "IEA")
					ANSI_IEA(strSegment);
			}

			iFileSize = m_InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);
		}


		//close the files
		m_InputFile.Close();
		m_OutputFile.Close();

		//don't display junk if it's not a real file
		if(!bIsValidFile) {
			AfxMessageBox("The file you are trying to parse is not an ANSI 997 acknowledgement file.");			
			return;
		}

		//open the finished file in notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.jones 2010-09-02 15:44) - PLID 40388 - fixed crash caused by passing in "this" as a parent
		HINSTANCE hInst = ShellExecute((HWND)GetDesktopWindow(), NULL, "notepad.exe", ("'" + strNameOut + "'"), NULL, SW_SHOW);

	} catch (CException &e) {
		AfxMessageBox("Error!");
		e.Delete();
	}
}

CString CANSIReportParserDlg::ParseSection(CString &strIn, char chDelimiter) {

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

CString CANSIReportParserDlg::ParseSegment(CString &strIn) {

	return ParseSection(strIn,'~');
}

CString CANSIReportParserDlg::ParseElement(CString &strIn) {

	return ParseSection(strIn,'*');
}

CString CANSIReportParserDlg::ParseComposite(CString &strIn) {

	return ParseSection(strIn,':');
}

	//Pos. #	Seg. ID		Name								Req. Des.	Max Use		Loop Repeat

BOOL CANSIReportParserDlg::ANSI_ST(CString &strIn) {
	
	//010		ST			Transaction Set Header				M			1

	CString OutputString, str, strFmt;
	
	//Ref.		Data		Name								Attributes
	//Des.		Element

	//ST01		143			Transaction Set Identifier Code		M	ID	3/3

	str = ParseElement(strIn);
	//this should always be 997, if not, give a warning
	if(str != "997") {
		if(str == "277")
			AfxMessageBox("The file you are trying to parse is not an ANSI 997 acknowledgement file,\n"
				"it is an ANSI 277 response file. Please click on the \"Format 277 Report\" button to view this file.");
		else
			AfxMessageBox("The file you are trying to parse is not an ANSI 997 acknowledgement file.");
		return FALSE;
	}
	
	//ST02		329			Transaction Set Control Number		M	AN	4/9

	str = ParseElement(strIn);
	//this is the batch number
	strFmt.Format("Batch Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());

	return TRUE;
}

void CANSIReportParserDlg::ANSI_AK1(CString &strIn) {

	//020		AK1			Functional Group Response Header	M			1

	CString OutputString, str, strFmt;
	
	//Ref.		Data		Name								Attributes
	//Des.		Element

	//AK101		479			Functional Identifier Code			M	ID	2/2

	str = ParseElement(strIn);
	//always "HC", we don't care

	//AK102		28			Group Control Number				M	N0	1/9

	str = ParseElement(strIn);
	//TODO: do we need to output this? What is it?
	strFmt.Format("Group Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

			//LOOP ID - AK2																999999

void CANSIReportParserDlg::ANSI_AK2(CString &strIn) {

	//030		AK2			Transaction Set Response Header		O			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//AK201		143			Transaction Set Identifier Code		M	ID	3/3

	str = ParseElement(strIn);
	//always "837", we don't care

	//AK202		329			Transaction Set Control Number		M	AN	4/9

	str = ParseElement(strIn);
	//TODO: do we need to output this? What is it?
	strFmt.Format("Transaction Set Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

			//LOOP ID - AK2/AK3															999999

void CANSIReportParserDlg::ANSI_AK3(CString &strIn) {

	//040		AK3			Data Segment Note					O			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	OutputString = "\r\n";

	//AK301		721			Segment ID Code						M	ID	2/3

	str = ParseElement(strIn);
	//the segment ID (ie. NM1, PER, etc.)
	strFmt.Format("Segment ID: %s\r\n",str);
	OutputString += strFmt;

	//AK302		719			Segment Position In Transaction Set	M	N0	1/6

	str = ParseElement(strIn);
	//the actual segment position, character wise, in the file.
	//I wouldn't normally output this, but it wouldn't suprise me if some
	//unhelpful EDI worker used this as a reference number.
	strFmt.Format("Segment Position In Transaction: %s\r\n",str);
	OutputString += strFmt;

	//AK303		447			Loop Identifier Code				O	AN	1/6

	str = ParseElement(strIn);
	//this is the very helpful loop ID number!
	strFmt.Format("Loop ID: %s\r\n",str);
	OutputString += strFmt;

	//AK304		720			Segment Syntax Error Code			O	ID	1/3

	str = ParseElement(strIn);
	//this is the error code for this segment
	if(str.GetLength() > 0) {
		int iErrorCode = atoi(str);
		switch(iErrorCode) {
			case 1:
				//Unrecognized segment ID
				strFmt.Format("Segment Error %li: Unrecognized segment ID\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 2:
				//Unexpected segment
				strFmt.Format("Segment Error %li: Unexpected segment\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 3:
				//Mandatory segment missing
				strFmt.Format("Segment Error %li: Mandatory segment missing\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 4:
				//Loop occurs over maximum times
				strFmt.Format("Segment Error %li: Loop occurs over maximum times\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 5:
				//Segment exceeds maximum use
				strFmt.Format("Segment Error %li: Segment exceeds maximum use\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 6:
				//Segment not defined in transaction set
				strFmt.Format("Segment Error %li: Segment not defined in transaction set\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 7:
				//Segment not in proper sequence
				strFmt.Format("Segment Error %li: Segment not in proper sequence\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 8:
				//Segment has data element errors
				strFmt.Format("Segment Error %li: Segment has data element errors\r\n",iErrorCode);
				OutputString += strFmt;
				break;
		}
	}

	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_AK4(CString &strIn) {

	//050		AK4			Data Element Note					O			99

	CString OutputString, str, strFmt;

	//Ref.		Data		Name										Attributes
	//Des.		Element

	OutputString = "\r\n";

	//AK401		C030		POSITION IN SEGMENT							M

	CString strComposite = ParseElement(strIn);
	//Composites work differently, they are one element with sub-elements
	//separated by a colon, but ParseComposite handles this.

	//AK401-1	722			Element Position In Segment					M	N0	1/2

	str = ParseComposite(strComposite);
	//This is the character location in the segment.
	//Much like AK302, I don't see a need for this, but I can bet someone, somewhere
	//will use this as a reference number.
	strFmt.Format("Element Position In Segment: %s\r\n",str);
	OutputString = strFmt;

	//AK401-2	1528		Component Element Position In Composite		O	N0	1/2

	str = ParseComposite(strComposite);
	//if the error is in a composite, this is the location of that character
	if(str.GetLength() > 0) {
		strFmt.Format("Component Element Position In Composite: %s\r\n",str);
		OutputString += strFmt;
	}

	//AK402		725			Data Element Reference Number				O	N0	1/4

	str = ParseElement(strIn);
	//finally we see a use for the reference number! For example, this element's
	//reference number is 725.
	strFmt.Format("Data Element Reference Number: %s\r\n",str);
	OutputString += strFmt;

	//AK403		723			Data Element Syntax Error Code				M	ID	1/3

	str = ParseElement(strIn);
	//this is the error code for this element
	if(str.GetLength() > 0) {
		int iErrorCode = atoi(str);
		switch(iErrorCode) {
			case 1:
				//Mandatory data element missing
				strFmt.Format("Element Error %li: Mandatory data element missing\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 2:
				//Conditional required data element missing
				strFmt.Format("Element Error %li: Conditional required data element missing\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 3:
				//Too many data elements
				strFmt.Format("Element Error %li: Too many data elements\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 4:
				//Data element too short
				strFmt.Format("Element Error %li: Data element too short\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 5:
				//Data element too long
				strFmt.Format("Element Error %li: Data element too long\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 6:
				//Invalid character in data element
				strFmt.Format("Element Error %li: Invalid character in data element\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 7:
				//Invalid code value
				strFmt.Format("Element Error %li: Invalid code value\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 8:
				//Invalid date
				strFmt.Format("Element Error %li: Invalid date\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 9:
				//Invalid time
				strFmt.Format("Element Error %li: Invalid time\r\n",iErrorCode);
				OutputString += strFmt;
				break;
			case 10:
				//Exclusion Condition Violated
				strFmt.Format("Element Error %li: Exclusion Condition Violated\r\n",iErrorCode);
				OutputString += strFmt;
				break;
		}
	}

	//AK404		724			Copy Of Bad Data Element					O	AN	1/99

	str = ParseElement(strIn);
	//this is the data in the element
	if(str.GetLength() > 0) {
		strFmt.Format("Data in element: %s\r\n",str);
		OutputString += strFmt;
	}

	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_AK5(CString &strIn) {

	//060		AK5			Transaction Set Response Trailer	M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name									Attributes
	//Des.		Element

	//AK501		717			Transaction Set Acknowledgement Code	M	ID	1/1

	str = ParseElement(strIn);
	//this is the accepted/rejected status of the batch as a whole
	if(str.GetLength() > 0) {
		char chErrorCode = str.GetAt(0);
		switch(chErrorCode) {
			case 'A':
				//Accepted
				strFmt.Format("Batch Status: Accepted\r\n");
				OutputString = strFmt;
				break;
			case 'E':
				//Accepted But Errors Were Noted
				strFmt.Format("Batch Status: Accepted But Errors Were Noted\r\n");
				OutputString = strFmt;
				break;
			case 'M':
				//Rejected, Message Authentication Code (MAC) Failed
				strFmt.Format("Batch Status: Rejected, Message Authentication Code (MAC) Failed\r\n");
				OutputString = strFmt;
				break;
			case 'R':
				//Rejected
				strFmt.Format("Batch Status: Rejected\r\n");
				OutputString = strFmt;
				break;
			case 'W':
				//Rejected, Assurance Failed Validity Tests
				strFmt.Format("Batch Status: Rejected, Assurance Failed Validity Tests\r\n");
				OutputString = strFmt;
				break;
			case 'X':
				//Rejected, Content After Decryption Could Not Be Analyzed
				strFmt.Format("Batch Status: Rejected, Content After Decryption Could Not Be Analyzed\r\n");
				OutputString = strFmt;
				break;
		}
	}

	//AK502		718			Transaction Set Syntax Error Code		O	ID	1/3
	//AK503		718			Transaction Set Syntax Error Code		O	ID	1/3
	//AK504		718			Transaction Set Syntax Error Code		O	ID	1/3
	//AK505		718			Transaction Set Syntax Error Code		O	ID	1/3
	//AK506		718			Transaction Set Syntax Error Code		O	ID	1/3

	//as this is the same thing repeatedly, just run a loop

	while(strIn.GetLength() > 0) {
		str = ParseElement(strIn);
		//this is the batch error code
		if(str.GetLength() > 0) {
			int iErrorCode = atoi(str);
			switch(iErrorCode) {
				case 1:
					//Transaction Set Not Supported
					strFmt.Format("Transaction Error %li: Transaction Set Not Supported\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 2:
					//Transaction Set Trailer Missing
					strFmt.Format("Transaction Error %li: Transaction Set Not Supported\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 3:
					//Transaction Set Control Number in Header and Trailer Do Not Match
					strFmt.Format("Transaction Error %li: Transaction Set Control Number in Header and Trailer Do Not Match\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 4:
					//Number Of Included Segments Does Not Match Actual Count
					strFmt.Format("Transaction Error %li: Number Of Included Segments Does Not Match Actual Count\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 5:
					//One Or More Segments In Error
					strFmt.Format("Transaction Error %li: One Or More Segments In Error\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 6:
					//Missing Or Invalid Transaction Set Identifier
					strFmt.Format("Transaction Error %li: Missing Or Invalid Transaction Set Identifier\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 7:
					//Missing Or Invalid Transaction Set Control Number
					strFmt.Format("Transaction Error %li: Missing Or Invalid Transaction Set Control Number\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 8:
					//Authentication Key Name Unknown
					strFmt.Format("Transaction Error %li: Authentication Key Name Unknown\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 9:
					//Encryption Key Name Unknown
					strFmt.Format("Transaction Error %li: Encryption Key Name Unknown\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 10:
					//Requested Service (Authentication or Encrypted) Not Available
					strFmt.Format("Transaction Error %li: Requested Service (Authentication or Encrypted) Not Available\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 11:
					//Unknown Security Recipient
					strFmt.Format("Transaction Error %li: Unknown Security Recipient\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 12:
					//Incorrect Message Length (Encryption Only)
					strFmt.Format("Transaction Error %li: Incorrect Message Length (Encryption Only)\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 13:
					//Message Authentication Code Failed
					strFmt.Format("Transaction Error %li: Message Authentication Code Failed\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 14:
					//oddly, there is no 14!
					break;
				case 15:
					//Unknown Security Originator
					strFmt.Format("Transaction Error %li: Unknown Security Originator\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 16:
					//Syntax Error in Decrypted Text
					strFmt.Format("Transaction Error %li: Syntax Error in Decrypted Text\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 17:
					//Security Not Supported
					strFmt.Format("Transaction Error %li: Security Not Supported\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 18:
					//there is no 18 through 22
					break;
				case 19:
					break;
				case 20:
					break;
				case 21:
					break;
				case 22:
					break;
				case 23:
					//Transaction Set Control Number Not Unique Within The Functional Group
					strFmt.Format("Transaction Error %li: Transaction Set Control Number Not Unique Within The Functional Group\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 24:
					//S3E Security End Segment Missing for S3S Security Start Segment
					strFmt.Format("Transaction Error %li: S3E Security End Segment Missing for S3S Security Start Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 25:
					//S3S Security Start Segment Missing for S3E Security End Segment
					strFmt.Format("Transaction Error %li: S3S Security Start Segment Missing for S3E Security End Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 26:
					//S4E Security End Segment Missing for S4S Security Start Segment
					strFmt.Format("Transaction Error %li: S4E Security End Segment Missing for S4S Security Start Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 27:
					//S4S Security Start Segment Missing for S4E Security End Segment
					strFmt.Format("Transaction Error %li: S4S Security Start Segment Missing for S4E Security End Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
			}
		}		
	}
	
	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_AK9(CString &strIn) {

	//070		AK9			Functional Group Response Trailer	M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//AK901		715			Functional Group Acknowledge Code	M	ID	1/1

	str = ParseElement(strIn);
	//this is the accepted/rejected status of the functional group
	if(str.GetLength() > 0) {
		char chErrorCode = str.GetAt(0);
		switch(chErrorCode) {
			case 'A':
				//Accepted
				strFmt.Format("Functional Group Status: Accepted\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'E':
				//Accepted But Errors Were Noted
				strFmt.Format("Functional Group Status: Accepted But Errors Were Noted\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'M':
				//Rejected, Message Authentication Code (MAC) Failed
				strFmt.Format("Functional Group Status: Rejected, Message Authentication Code (MAC) Failed\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'P':
				//Partially Accepted, At Least One Transaction Set Was Rejected
				strFmt.Format("Functional Group Status: Partially Accepted, At Least One Transaction Set Was Rejected\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'R':
				//Rejected
				strFmt.Format("Functional Group Status: Rejected\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'W':
				//Rejected, Assurance Failed Validity Tests
				strFmt.Format("Functional Group Status: Rejected, Assurance Failed Validity Tests\r\n\r\n");
				OutputString = strFmt;
				break;
			case 'X':
				//Rejected, Content After Decryption Could Not Be Analyzed
				strFmt.Format("Functional Group Status: Rejected, Content After Decryption Could Not Be Analyzed\r\n\r\n");
				OutputString = strFmt;
				break;
		}
	}

	//AK902		97			Number of Transaction Sets Included	M	N0	1/6

	str = ParseElement(strIn);
	//TODO: isn't this always going to be 1?
	strFmt.Format("Number of Transaction Sets Included: %s\r\n\r\n",str);
	OutputString += strFmt;

	//AK903		123			Number of Received Transaction Sets	M	N0	1/6

	str = ParseElement(strIn);
	//TODO: isn't this always going to be 1?
	strFmt.Format("Number of Received Transaction sets: %s\r\n\r\n",str);
	OutputString += strFmt;

	//AK904		2			Number of Accepted Transaction Sets	M	N0	1/6

	str = ParseElement(strIn);
	//TODO: this too may not be needed
	strFmt.Format("Number of Accepted Transaction sets: %s\r\n\r\n",str);
	OutputString += strFmt;

	//AK905		716			Functional Group Syntax Error Code	O	ID	1/3
	//AK906		716			Functional Group Syntax Error Code	O	ID	1/3
	//AK907		716			Functional Group Syntax Error Code	O	ID	1/3
	//AK908		716			Functional Group Syntax Error Code	O	ID	1/3
	//AK909		716			Functional Group Syntax Error Code	O	ID	1/3

	//loop through all of these
	while(strIn.GetLength() > 0) {
		str = ParseElement(strIn);
		//this is the functional group error code
		if(str.GetLength() > 0) {
			int iErrorCode = atoi(str);
			switch(iErrorCode) {
				case 1:
					//Functional Group Not Supported
					strFmt.Format("Functional Group Error %li: Functional Group Not Supported\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 2:
					//Functional Group Trailer Missing
					strFmt.Format("Functional Group Error %li: Functional Group Not Supported\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 3:
					//Group Control Number in Header and Trailer Do Not Match
					strFmt.Format("Functional Group Error %li: Group Control Number in Header and Trailer Do Not Match\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 4:
					//Group Control Number in the Functional Group Header and Trailer Do Not Agree
					strFmt.Format("Functional Group Error %li: Group Control Number in the Functional Group Header and Trailer Do Not Agree\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 5:
					//Number Of Included Transaction Sets Does Not Match Actual Count
					strFmt.Format("Functional Group Error %li: Number Of Included Transaction Sets Does Not Match Actual Count\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 6:
					//Group Control Number Violates Syntax
					strFmt.Format("Functional Group Error %li: Group Control Number Violates Syntax\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 7:
					//there are no errors 7 - 9
					break;
				case 8:
					break;
				case 9:
					break;
				case 10:
					//Authentication Key Name Unknown
					strFmt.Format("Functional Group Error %li: Authentication Key Name Unknown\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 11:
					//Encryption Key Name Unknown
					strFmt.Format("Functional Group Error %li: Encryption Key Name Unknown\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 12:
					//Requested Service (Authentication Or Encryption) Not Available
					strFmt.Format("Functional Group Error %li: Requested Service (Authentication Or Encryption) Not Available\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 13:
					//Unknown Security Recipient
					strFmt.Format("Functional Group Error %li: Unknown Security Recipient\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 14:
					//Unknown Security Originator
					strFmt.Format("Functional Group Error %li: Unknown Security Originator\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 15:
					//Syntax Error in Decrypted Text
					strFmt.Format("Functional Group Error %li: Syntax Error in Decrypted Text\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 16:
					//Security Not Supported
					strFmt.Format("Functional Group Error %li: Security Not Supported\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 17:
					//Incorrect Message Length (Encryption Only)
					strFmt.Format("Functional Group Error %li: Incorrect Message Length (Encryption Only)\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 18:
					//Message Authentication Code Failed
					strFmt.Format("Functional Group Error %li: Message Authentication Code Failed\r\n\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 19:
					//there is no 19 - 22
					break;
				case 20:
					break;
				case 21:
					break;
				case 22:
					break;
				case 23:
					//S3E Security End Segment Missing for S3S Security Start Segment
					strFmt.Format("Functional Group Error %li: S3E Security End Segment Missing for S3S Security Start Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 24:
					//S3S Security Start Segment Missing for S3E Security End Segment
					strFmt.Format("Functional Group Error %li: S3S Security Start Segment Missing for S3E Security End Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 25:
					//S4E Security End Segment Missing for S4S Security Start Segment
					strFmt.Format("Functional Group Error %li: S4E Security End Segment Missing for S4S Security Start Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
				case 26:
					//S4S Security Start Segment Missing for S4E Security End Segment
					strFmt.Format("Functional Group Error %li: S4S Security Start Segment Missing for S4E Security End Segment\r\n",iErrorCode);
					OutputString += strFmt;
					break;
			}
		}		
	}

	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_SE(CString &strIn) {

	//080		SE			Transaction Set Trailer				M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element


	//SE01		96			Number Of Included Segments			M	N0	1/10

	//SE02		329			Transaction Set Control Number		M	AN	4/9

	//OutputString = "SE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_ISA(CString &strIn) {

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
	if(str.GetLength() == 6) {
		//format YYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/20" + str.Left(2);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	else if(str.GetLength() == 8) {
		//format CCYYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/" + str.Left(4);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	strFmt.Format("Processing Date: %s",strDate);
	OutputString = strFmt;

	//ISA10	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//processing time
	
	CString strTime;
	if(str.GetLength() == 4) {
		//format HHMM into HH:MM
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	else if(str.GetLength() >= 6) {
		//format HHMMSS into HH:MM (also works for HHMMSSDD)
		str = str.Left(4);
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	strFmt.Format(" %s\r\n",strTime);
	OutputString += strFmt;

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
	//we won't need this

	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_IEA(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

	//IEA02	I12			Interchange Control Number				M	N0	9/9

	//OutputString = "IEA\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_TA1(CString &strIn) {

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

	CString strDate;
	if(str.GetLength() == 6) {
		//format YYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/20" + str.Left(2);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	else if(str.GetLength() == 8) {
		//format CCYYMMDD into MM/DD/CCYY
		strDate = str.Right(4) + "/" + str.Left(4);
		strDate = strDate.Left(2) + "/" + strDate.Right(7);
	}
	strFmt.Format("Claim Creation Date: %s",strDate);
	OutputString += strFmt;

	//TA103	I09			Interchange Time						M	TM	4/4

	str = ParseElement(strIn);
	//claim time
	
	CString strTime;
	if(str.GetLength() == 4) {
		//format HHMM into HH:MM
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	else if(str.GetLength() >= 6) {
		//format HHMMSS into HH:MM (also works for HHMMSSDD)
		str = str.Left(4);
		strTime = str.Left(2) + ":" + str.Right(2);
	}
	strFmt.Format(" %s\r\n\r\n",strTime);
	OutputString += strFmt;

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
	OutputString += strFmt;

	//TA105	I18			Interchange Note Code					M	ID	3/3

	str = ParseElement(strIn);
	//interchange error code
	if(str == "000") {
		//No error
		strFmt.Format("Interchange Error Code %s: No Error\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "001") {
		//The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement
		strFmt.Format("Interchange Error Code %s: The Interchange Control Number in the Header and Trailer Do Not Match. The Value From the Header is Used in the Acknowledgement\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "002") {
		//This Standard as Noted in the Control Standards Identifier is Not Supported
		strFmt.Format("Interchange Error Code %s: This Standard as Noted in the Control Standards Identifier is Not Supported\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "003") {
		//This Version of the Controls is Not Supported
		strFmt.Format("Interchange Error Code %s: This Version of the Controls is Not Supported\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "004") {
		//The Segment Terminator is Invalid
		strFmt.Format("Interchange Error Code %s: The Segment Terminator is Invalid\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "005") {
		//Invalid Interchange ID Qualifier for Sender
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Sender\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "006") {
		//Invalid Interchange Sender ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Sender ID\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "007") {
		//Invalid Interchange ID Qualifier for Receiver
		strFmt.Format("Interchange Error Code %s: Invalid Interchange ID Qualifier for Receiver\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "008") {
		//Invalid Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Receiver ID\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "009") {
		//Unknown Interchange Receiver ID
		strFmt.Format("Interchange Error Code %s: Unknown Interchange Receiver ID\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "010") {
		//Invalid Authorization Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Qualifier Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "011") {
		//Invalid Authorization Information Value
		strFmt.Format("Interchange Error Code %s: Invalid Authorization Information Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "012") {
		//Invalid Security Information Qualifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Security Information Qualifier Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "013") {
		//Invalid Security Information
		strFmt.Format("Interchange Error Code %s: Invalid Security Information\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "014") {
		//Invalid Interchange Date Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Date Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "015") {
		//Invalid Interchange Time Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Time Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "016") {
		//Invalid Interchange Standards Identifier Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Standards Identifier Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "017") {
		//Invalid Interchange Version ID Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Version ID Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "018") {
		//Invalid Interchange Control Number Value
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control Number Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "019") {
		//Invalid Acknowledgement Requested Value
		strFmt.Format("Interchange Error Code %s: Invalid Acknowledgement Requested Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "020") {
		//Invalid Test Indicator Value
		strFmt.Format("Interchange Error Code %s: Invalid Test Indicator Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "021") {
		//Invalid Number of Included Groups Value
		strFmt.Format("Interchange Error Code %s: Invalid Number of Included Groups Value\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "022") {
		//Invalid Control Structure
		strFmt.Format("Interchange Error Code %s: Invalid Control Structure\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "023") {
		//Improper (Premature) End-Of-File (Transmission)
		strFmt.Format("Interchange Error Code %s: Improper (Premature) End-Of-File (Transmission)\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "024") {
		//Invalid Interchange Control (e.g., Invalid GS Segment)
		strFmt.Format("Interchange Error Code %s: Invalid Interchange Control (e.g., Invalid GS Segment)\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "025") {
		//Duplicate Interchange Control Number
		strFmt.Format("Interchange Error Code %s: Duplicate Interchange Control Number\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "026") {
		//Invalid Data Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Data Element Separator\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "027") {
		//Invalid Component Element Separator
		strFmt.Format("Interchange Error Code %s: Invalid Component Element Separator\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "028") {
		//Invalid Delivery Date in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Date in Deferred Delivery Request\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "029") {
		//Invalid Delivery Time in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time in Deferred Delivery Request\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "030") {
		//Invalid Delivery Time Code in Deferred Delivery Request
		strFmt.Format("Interchange Error Code %s: Invalid Delivery Time Code in Deferred Delivery Request\r\n",str);
		OutputString += strFmt;
	}
	else if(str == "031") {
		//Invalid Grade of Service Code
		strFmt.Format("Interchange Error Code %s: Invalid Grade of Service Code\r\n",str);
		OutputString += strFmt;
	}

	OutputString += "\r\n";
	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_GS(CString &strIn) {

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

	//OutputString = "GS\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSIReportParserDlg::ANSI_GE(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GE01	97			Number of Transaction Sets Included		M	N0	1/6

	//GE02	28			Group Control Number					M	N0	1/9

	//OutputString = "GE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}
