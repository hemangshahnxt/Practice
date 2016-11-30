// ANSI277ReportParserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ANSI277ReportParserDlg.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CANSI277ReportParserDlg dialog


CANSI277ReportParserDlg::CANSI277ReportParserDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CANSI277ReportParserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CANSI277ReportParserDlg)
	//}}AFX_DATA_INIT
}


void CANSI277ReportParserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CANSI277ReportParserDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_PARSE, m_btnParseFile);
	DDX_Control(pDX, IDC_FILE_INPUT_NAME, m_nxeditFileInputName);
	DDX_Control(pDX, IDC_FILE_OUTPUT_NAME, m_nxeditFileOutputName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CANSI277ReportParserDlg, CNxDialog)
	//{{AFX_MSG_MAP(CANSI277ReportParserDlg)
	ON_BN_CLICKED(IDC_BROWSE_IN, OnBrowseIn)
	ON_BN_CLICKED(IDC_BROWSE_OUT, OnBrowseOut)
	ON_BN_CLICKED(IDC_PARSE, OnParse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CANSI277ReportParserDlg message handlers

BOOL CANSI277ReportParserDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		
		// (j.jones 2008-05-07 10:49) - PLID 29854 - added nxiconbuttons for modernization
		m_btnClose.AutoSet(NXB_CLOSE);

	}NxCatchAll("Error in CANSI277ReportParserDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CANSI277ReportParserDlg::OnBrowseIn() 
{
	CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST);
	if (BrowseFiles.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_FILE_INPUT_NAME,BrowseFiles.GetPathName());
}

void CANSI277ReportParserDlg::OnBrowseOut() 
{
	CFileDialog BrowseFiles(FALSE,NULL,"report.txt");
	if (BrowseFiles.DoModal() == IDCANCEL) return;
	SetDlgItemText(IDC_FILE_OUTPUT_NAME,BrowseFiles.GetPathName());
}

void CANSI277ReportParserDlg::OnParse() 
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
				else if(strIdent == "SE")
					ANSI_SE(strSegment);
				else if(strIdent == "GE")
					ANSI_GE(strSegment);
				else if(strIdent == "IEA")
					ANSI_IEA(strSegment);
				
				//277-specific fields
				else if(strIdent == "BHT")
					ANSI_BHT(strSegment);
				else if(strIdent == "HL")
					ANSI_HL(strSegment);
				else if(strIdent == "NM1")
					ANSI_NM1(strSegment);
				else if(strIdent == "PER")
					ANSI_PER(strSegment);
				else if(strIdent == "DMG")
					ANSI_DMG(strSegment);
				else if(strIdent == "TRN")
					ANSI_TRN(strSegment);
				else if(strIdent == "STC")
					ANSI_STC(strSegment);
				else if(strIdent == "REF")
					ANSI_REF(strSegment);
				else if(strIdent == "DTP")
					ANSI_DTP(strSegment);
				else if(strIdent == "SVC")
					ANSI_SVC(strSegment);
				else {
					Sleep(0);
				}
			}

			iFileSize = m_InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);
		}


		//close the files
		m_InputFile.Close();
		m_OutputFile.Close();

		//don't display junk if it's not a real file
		if(!bIsValidFile) {
			AfxMessageBox("The file you are trying to parse is not an ANSI 277 response file.");
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

void CANSI277ReportParserDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CNxDialog::OnOK();
}

CString CANSI277ReportParserDlg::ParseSection(CString &strIn, char chDelimiter) {

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

CString CANSI277ReportParserDlg::ParseSegment(CString &strIn) {

	return ParseSection(strIn,'~');
}

CString CANSI277ReportParserDlg::ParseElement(CString &strIn) {

	return ParseSection(strIn,'*');
}

CString CANSI277ReportParserDlg::ParseComposite(CString &strIn) {

	return ParseSection(strIn,':');
}

BOOL CANSI277ReportParserDlg::ANSI_ST(CString &strIn) {
	
	//010		ST			Transaction Set Header				M			1

	CString OutputString, str, strFmt;
	
	//Ref.		Data		Name								Attributes
	//Des.		Element

	//ST01		143			Transaction Set Identifier Code		M	ID	3/3

	str = ParseElement(strIn);
	//this should always be 277, if not, give a warning
	if(str != "277") {
		if(str == "997")
			AfxMessageBox("The file you are trying to parse is not an ANSI 277 response file,\n"
			"it is an ANSI 997 acknowledgement file. Please click on the \"Format 997 Report\" button to view this file.");
		else
			AfxMessageBox("The file you are trying to parse is not an ANSI 277 response file.");
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

void CANSI277ReportParserDlg::ANSI_SE(CString &strIn) {

	//080		SE			Transaction Set Trailer				M			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element


	//SE01		96			Number Of Included Segments			M	N0	1/10

	//SE02		329			Transaction Set Control Number		M	AN	4/9

	//OutputString = "SE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_ISA(CString &strIn) {

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

void CANSI277ReportParserDlg::ANSI_IEA(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

	//IEA02	I12			Interchange Control Number				M	N0	9/9

	//OutputString = "IEA\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_TA1(CString &strIn) {

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

void CANSI277ReportParserDlg::ANSI_GS(CString &strIn) {

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

void CANSI277ReportParserDlg::ANSI_GE(CString &strIn) {

	CString OutputString, str, strFmt;

	//Ref.	Data		Name									Attributes
	//Des.	Element

	//GE01	97			Number of Transaction Sets Included		M	N0	1/6

	//GE02	28			Group Control Number					M	N0	1/9

	//OutputString = "GE\r\n";
	//m_OutputFile.Write(OutputString,OutputString.GetLength());
}

///////////////////////////////////////
//Begin the 277-specific fields      //
///////////////////////////////////////

void CANSI277ReportParserDlg::ANSI_BHT(CString &strIn) {

	//126		BHT			Beginning of Hierarchical Transaction		R		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//BHT01		1005		Hierarchical Structure Code			M	ID	4/4

	//the only data is "0010" - "Information Source, Information Receiver, Provider "
	//							"of Service, Subscriber, Dependent"
	str = ParseElement(strIn);
	//always "0010", we don't care

	//BHT02		353			Transaction Set Purpose Code		M	ID	2/2
	//the only data is "8" - "Status"
	str = ParseElement(strIn);
	//always "8", we don't care

	//BHT03		127			Reference Identification			O	AN	1/30
	//the originator's internal ID that tracks this transaction
	str = ParseElement(strIn);
	strFmt.Format("Transaction Identifier Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	//BHT04		373			Date								O	DT	8/8
	str = ParseElement(strIn);
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
	strFmt.Format("Transaction Set Creation Date: %s\r\n\r\n",strDate);
	OutputString += strFmt;

	//NOT USED BHT05 337	Time								O	TM	4/8
	str = ParseElement(strIn);

	//BHT06		640			Transaction Type Code				O	ID	2/2
	//the only data is "DG" - "Response"
	str = ParseElement(strIn);
	//always "DG", we don't care


	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_HL(CString &strIn) {

	//Hierarchical Level, used in:
	//128		HL			Information Source Level			R		1
	//136		HL			Information Receiver Level			R		1
	//141		HL			Service Provider Level				R		1
	//146		HL			Subscriber Level					R		1
	//190		HL			Dependent Level						S		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//HL01		628			Hierarchical ID Number				M	AN	1/12
	str = ParseElement(strIn);

	//HL02 734		Hierarchical Parent ID Number		O	AN	1/12
	//if 2000A (HL03 = "20"), then NOT USED
	str = ParseElement(strIn);

	//HL03		735			Hierarchical Level Code				M	ID	1/2
	//"20" - 2000A - Information Source
	//"21" - 2000B - Information Receiver
	//"19" - 2000C - Provider Of Service
	//"22" - 2000D - Subscriber
	//"23" - 2000E - Dependent
	str = ParseElement(strIn);

	if(str == "19") {
		OutputString += "*******************************************************************\r\n"
			"*******************************************************************\r\n\r\n";
	}
	else if(str == "22") {
		OutputString += "*******************************************************************\r\n\r\n";
	}

	//HL04		736			Hierarchical Child Code				O	ID	1/1
	//indicates whether or not there are subordinate (or child) HL
	//segments related to the current HL segment.
	//"0" - No Subordinate HL Segment in This Hierarchical Structure.
	//"1" - Additional Subordinate HL Data Segment in This Hierarchical Structure.
	//if HL03 = "23" then NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_NM1(CString &strIn) {

	//Individual or Organizational Name, used in:
	//130		NM1			Payer Name							R		1
	//138		NM1			Information Receiver Name			R		1
	//143		NM1			Provider Name						R		1
	//150		NM1			Subscriber Name						R		1
	//194		NM1			Dependent Name						R		1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	CString strIdent, strEntityType, strOrganization, strLast, strFirst, strMiddle, strPrefix, strSuffix;
	CString strQual, strID;

	//NM101		98			Entity Identifier Code				M	ID	2/3
	//"PR" - 2100A - Payer
	//"41" - 2100B - Submitter
	//"1P" - 2100C - Provider
	//"IL" - 2100D - Insured or Subscriber
	//"QC" - 2100D or 2100E - Patient (in 2100D it is used only when the subscriber is the patient.)
	strIdent = ParseElement(strIn);

	if(strIdent == "PR")
		strFmt = "Payer: ";
	else if(strIdent == "41")
		strFmt = "Submitter: ";
	else if(strIdent == "1P")
		strFmt = "Provider: ";
	else if(strIdent == "IL")
		strFmt = "Insured/Subscriber: ";
	else if(strIdent == "QC")
		strFmt = "Patient: ";
	else
		strFmt = "Unknown Person: ";
	OutputString = strFmt;

	//NM102		1065		Entity Type Qualifier				M	ID	1/1
	//"1" - Person
	//"2" - Non-Person Entity
	strEntityType = ParseElement(strIn);

	//NM103		1035		Name Last or Organization Name		O	AN	1/35
	//if NM101 = "PR" then "Payer Name"
	//if NM101 = "41" then "Receiver Name"
	str = ParseElement(strIn);
	if(strEntityType == "1")
		strLast = str;
	else if(strEntityType == "2")
		strOrganization = str;

	//NM104		1036		Name First							O	AN	1/25
	//if NM101 = "PR" or NM102 = "2" then NOT USED
	strFirst = ParseElement(strIn);

	//NM105		1037		Name Middle							O	AN	1/25
	//if NM101 = "PR" or NM102 = "2" then NOT USED
	strMiddle = ParseElement(strIn);

	//NM106		1038		Name Prefix							O	AN	1/10
	//if NM101 = "PR" or NM102 = "2" then NOT USED
	strPrefix = ParseElement(strIn);

	//NM107		1039		Name Suffix							O	AN	1/10
	//if NM101 = "PR" or NM102 = "2" then NOT USED
	strSuffix = ParseElement(strIn);

	if(strEntityType == "1") {
		strFmt.Format("%s, %s %s",strLast,strFirst,strMiddle);
		OutputString += strFmt;
	}
	else if(strEntityType == "2") {
		strFmt = strOrganization;
		OutputString += strFmt;
	}
	
	OutputString += "\r\n";

	//NM108		66			Identification Code Qualifier		X	ID	1/2

	//if NM101 = "PR" then:
		//Payer identifiers should be used with the following preferences:
		//(PI) Payer ID
		//(NI) NAIC Code
		//(AD) If the Payer is a Blue Cross or Blue Shield Plan, BCBSA Plan Code
		//(PP) If the Payer is a Pharmacy Processor, Pharmacy Processor Number
		//(FI) Tax ID
		//(21) If other codes are not available or known, use HIN or Payer Identification Number

		//CODE DEFINITION
		//21 - Health Industry Number (HIN)
		//AD - Blue Cross Blue Shield Association Plan Code
		//FI - Federal Taxpayer’s Identification Number
		//NI - National Association of Insurance Commissioners (NAIC) Identification
		//PI - Payor Identification
		//PP - Pharmacy Processor Number
		//XV - Health Care Financing Administration National PlanID

	//if NM101 = "41" then:
		//46 - Electronic Transmitter Identification Number (ETIN)
		//FI - Federal Taxpayer’s Identification Number
		//XX - Health Care Financing Administration National Provider Identifier

	//if NM101 = "1P" then:
		//FI - Federal Taxpayer’s Identification Number
		//SV - Service Provider Number
		//XX - Health Care Financing Administration National Provider Identifier

	//if NM101 = "IL" then:
		//24 - Employer’s Identification Number
		//MI - Member Identification Number
		//ZZ - Mutually Defined

	//if NM101 = "QC" then:
		//MI - Member Identification Number
		//ZZ - Mutually Defined

	strQual = ParseElement(strIn);

	//NM109		67			Identification Code					X	AN	2/80
	
	//if NM101 = "PR" then Payor ID
	//if NM101 = "41" then Receiver ID
	//if NM101 = "1P" then Provider ID
	//if NM101 = "IL" then Subscriber ID
	//if NM101 = "QC" then Patient ID	

	strID = ParseElement(strIn);

	if(strIdent == "PR")
		strFmt.Format("Payer ID: %s\r\n\r\n", strID);
	else if(strIdent == "41")
		strFmt.Format("Receiver ID: %s\r\n\r\n", strID);
	else if(strIdent == "1P")
		strFmt.Format("Provider ID: %s\r\n\r\n", strID);
	else if(strIdent == "IL")
		strFmt.Format("Subscriber ID: %s\r\n\r\n", strID);
	else if(strIdent == "QC")
		strFmt.Format("Patient ID: %s\r\n\r\n", strID);
	else
		strFmt.Format("ID: %s\r\n\r\n", strID);
	OutputString += strFmt;

	//NM110		706			Entity Relationship Code			X	ID	2/2
	//NOT USED
	str = ParseElement(strIn);

	//NM111		98			Entity Identifier Code				O	ID	2/3
	//NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_PER(CString &strIn) {

	//In loop 2100A:
	//133		PER			Payer Contact Information			S			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//PER01		366			Contact Function Code				M	ID	2/2
	str = ParseElement(strIn);

	//PER02		93			Name								O	AN	1/60
	str = ParseElement(strIn);

	//PER03		365			Communication Number Qualifier		X	ID	2/2
	//"ED" - Electronic Data Interchange Access Number
	//"EM" - Electronic Mail
	//"TE" - Telephone
	str = ParseElement(strIn);

	//PER04		364			Communication Number				X	AN	1/80
	//the type of number is determined from PER03
	str = ParseElement(strIn);

	//PER05		365			Communication Number Qualifier		X	ID	2/2
	//"EX" - Telephone Extension
	str = ParseElement(strIn);

	//PER06		364			Communication Number				X	AN	1/80
	//phone extensions only
	str = ParseElement(strIn);

	//PER07		365			Communication Number Qualifier		X	ID	2/2
	//"EX" - Telephone Extension
	//"FX" - Facsimile
	str = ParseElement(strIn);	

	//PER08		364			Communication Number				X	AN	1/80
	//the type of number is determined from PER07
	str = ParseElement(strIn);

	//NOT USED PER09 443	Contact Inquiry Reference			O	AN	1/20
	str = ParseElement(strIn);

	/*
	//TODO: do we need to output this? What is it?
	strFmt.Format("Transaction Set Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());
	*/
}

void CANSI277ReportParserDlg::ANSI_DMG(CString &strIn) {

	//Demographic Information, used in:

	//148		DMG			Subscriber Demographic Information	R			1
	//192		DMG			Dependent Demographic Information	R			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//DMG01		1250		Date Time Period Format Qualifier	X	ID	2/3
	//"D8" - Date Expressed in Format CCYYMMDD
	str = ParseElement(strIn);

	//DMG02		1251		Date Time Period					X	AN	1/35
	//Subscriber Birth Date
	str = ParseElement(strIn);

	//DMG03		1068		Gender Code							O	ID	1/1
	//"F" - Female
	//"M" - Male
	//"U" - Unknown
	str = ParseElement(strIn);

	//DMG04		1067		Marital Status Code					O	ID	1/1
	//NOT USED
	str = ParseElement(strIn);

	//DMG05		1109		Race or Ethnicity Code				O	ID	1/1
	//NOT USED
	str = ParseElement(strIn);

	//DMG06		1066		Citizenship Status Code				O	ID	1/2
	//NOT USED
	str = ParseElement(strIn);

	//DMG07		26			Country Code						O	ID	2/3
	//NOT USED
	str = ParseElement(strIn);

	//DMG08		659			Basis of Verification Code			O	ID	1/2
	//NOT USED
	str = ParseElement(strIn);

	//DMG09		380			Quantity							O	R	1/15
	//NOT USED
	str = ParseElement(strIn);

	/*
	//TODO: do we need to output this? What is it?
	strFmt.Format("Transaction Set Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());
	*/
}

void CANSI277ReportParserDlg::ANSI_TRN(CString &strIn) {

	//Loop 2200D:
	//153		TRN			Claim Submitter Trace Number		R			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//TRN01		481			Trace Type Code						M	ID	1/2
	//"2" - Referenced Transaction Trace Numbers
	str = ParseElement(strIn);

	//TRN02		127			Reference Identification			M	AN	1/30
	//Trace Number
	str = ParseElement(strIn);

	strFmt.Format("Claim Trace Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	//TRN03		509			Originating Company Identifier		O	AN	10/10
	//NOT USED
	str = ParseElement(strIn);

	//TRN04		127			Reference Identification			O	AN	1/30
	//NOT USED
	str = ParseElement(strIn);

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_STC(CString &strIn) {

	//Loop 2200D:
	//154		STC			Claim Level Status Information		R			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//STC01		C043		HEALTH CARE CLAIM STATUS			M
	//Used to convey status of the entire claim or a specific service line

	CString strComposite = ParseElement(strIn);

	//STC01 - 1 1271		Industry Code						M	AN	1/30
	//"This is the Category code. Use code source 507."
	str = ParseComposite(strComposite);
	if(str != "") {		
		strFmt.Format("Status Category: %s\r\n",GetStatusCategoryCode(str));
		OutputString += strFmt;
	}

	//STC01 - 2 1271		Industry Code						M	AN	1/30
	//"This is the Status code. Use code source 508."
	str = ParseComposite(strComposite);
	if(str != "") {
		strFmt.Format("Claim Status: %s\r\n",GetStatusCode(atoi(str)));
		OutputString += strFmt;
	}

	//STC01 - 3 98			Entity Identifier Code				O	ID	2/3
	//"Code identifying an organizational entity, a physical location, property or an individual."
	//"STC01-3 further modifies the status code in STC01-2.
	//Required if additional detail applicable to claim status is needed to clarify "
	//"the status and the payer’s system supports this level of detail."
	str = ParseComposite(strComposite);
	if(str != "") {
		if(str == "PR")
			strFmt = "Entity: Payer";
		else if(str == "41")
			strFmt = "Entity: Submitter";
		else if(str == "1P")
			strFmt = "Entity: Provider";
		else if(str == "IL")
			strFmt = "Entity: Insured/Subscriber";
		else if(str == "QC")
			strFmt = "Entity: Patient";
		else
			strFmt = "Entity: Unknown";
		OutputString += strFmt;
		OutputString += "\r\n";
	}

	if(strFmt != "")
		OutputString += "\r\n";

	strFmt = "";

	//STC02		373			Date								O	DT	8/8
	//Status Information Effective Date (CCYYMMDD)
	str = ParseElement(strIn);

	//STC03		306			Action Code							O	ID	1/2
	//NOT USED
	str = ParseElement(strIn);

	//STC04		782			Monetary Amount						O	R	1/18
	//Total Claim Charge Amount
	str = ParseElement(strIn);
	if(str != "") {
		COleCurrency cy;
		cy.ParseCurrency(str);
		strFmt.Format("Total Claim Charge Amount: %s\r\n",FormatCurrencyForInterface(cy,TRUE,TRUE));
		OutputString += strFmt;
	}

	//STC05		782			Monetary Amount						O	R	1/18
	//Claim Payment Amount
	str = ParseElement(strIn);
	if(str != "") {
		COleCurrency cy;
		cy.ParseCurrency(str);
		strFmt.Format("Claim Payment Amount: %s\r\n",FormatCurrencyForInterface(cy,TRUE,TRUE));
		OutputString += strFmt;
	}

	//STC06		373			Date								O	DT	8/8
	//Adjudication or Payment Date (CCYYMMDD)
	str = ParseElement(strIn);
	if(str != "") {
		strFmt.Format("Adjudication/Payment Date: %s/%s/%s\r\n",str.Mid(4,2),str.Right(2),str.Left(4));
		OutputString += strFmt;
	}

	//STC07		591			Payment Method Code					O	ID	3/3
	//"Code identifying the method for the movement of payment instructions."
	//"Will be used when claim has a dollar payment to the provider of service."
		//"ACH" - Automated Clearing House (ACH)
			//Use this code to move money electronically through
			//the Automated Clearing House (ACH). When this
			//code is used, information in BPR05 through BPR15
			//also must be included.
		//"BOP" - Financial Institution Option
			//Use this code to indicate that the third party
			//processor will choose the method of payment based
			//on end point requests or capabilities.
		//"CHK" - Check
			//Use this code to indicate that a check was issued for payment.
		//"FWT" - Federal Reserve Funds/Wire Transfer - Nonrepetitive
			//Use this code to indicate that the funds were sent through the wire system.
		//"NON" - Non-Payment Data
			//Use this code to indicate that this is information only and no dollars are to be moved.

	str = ParseElement(strIn);
	if(str != "") {
		strFmt = "Payment Method: ";
		if(str == "ACH")
			strFmt += "Automated Clearing House";
		else if(str == "BOP")
			strFmt += "Financial Institution Option";
		else if(str == "CHK")
			strFmt += "Check";
		else if(str == "FWT")
			strFmt += "Federal Reserve Funds/Wire Transfer";
		else if(str == "NON")
			strFmt += "Non-Payment Data";
		else
			strFmt += "Unknown";
		OutputString += strFmt;
		OutputString += "\r\n";
	}

	//STC08		373			Date								O	DT	8/8
	//Check Issue or EFT Effective Date (CCYYMMDD)
	str = ParseElement(strIn);
	if(str != "") {
		strFmt.Format("Check Issue Date/EFT Effective Date: %s/%s/%s/\r\n",str.Mid(4,2),str.Right(2),str.Left(4));
	}

	//STC09		429			Check Number						O	AN	1/16
	//Check or EFT Trace Number
	str = ParseElement(strIn);
	if(str != "") {
		strFmt.Format("Check/EFT Trace Number: %s\r\n",str);
	}

	strFmt = "";

	//STC10		C043		HEALTH CARE CLAIM STATUS			O
		//Used to convey status of the entire claim or a specific service line
		//Use this element if a second claim status is needed.

	strComposite = ParseElement(strIn);

	//STC10 - 1 1271		Industry Code						M	AN	1/30
	//"This is the Category code. Use code source 507."
	str = ParseComposite(strComposite);
	if(str != "") {		
		strFmt.Format("Status Category: %s\r\n",GetStatusCategoryCode(str));
		OutputString += strFmt;
	}

	//STC10 - 2 1271		Industry Code						M	AN	1/30
	//"This is the Status code. Use code source 508."
	str = ParseComposite(strComposite);
	if(str != "") {
		strFmt.Format("Claim Status: %s\r\n",GetStatusCode(atoi(str)));
		OutputString += strFmt;
		OutputString += "\r\n";
	}

	//STC10 - 3 98			Entity Identifier Code				O	ID	2/3
	//"Code identifying an organizational entity, a physical location, property or an individual."
	//"STC10-3 further modifies the status code in STC10-2.
	//Required if additional detail applicable to claim status is needed to clarify "
	//"the status and the payer’s system supports this level of detail."
	str = ParseComposite(strComposite);
	if(str != "") {
		if(str == "PR")
			strFmt = "Entity: Payer";
		else if(str == "41")
			strFmt = "Entity: Submitter";
		else if(str == "1P")
			strFmt = "Entity: Provider";
		else if(str == "IL")
			strFmt = "Entity: Insured/Subscriber";
		else if(str == "QC")
			strFmt = "Entity: Patient";
		else
			strFmt = "Entity: Unknown";
		OutputString += strFmt;
		OutputString += "\r\n";
	}

	if(strFmt != "")
		OutputString += "\r\n";

	strFmt = "";

	//STC11		C043		HEALTH CARE CLAIM STATUS			O
		//Used to convey status of the entire claim or a specific service line
		//Use this element if a third claim status is needed.

	strComposite = ParseElement(strIn);

	//STC11 - 1 1271		Industry Code						M	AN	1/30
	//"This is the Category code. Use code source 507."
	str = ParseComposite(strComposite);
	if(str != "") {		
		strFmt.Format("Status Category: %s\r\n",GetStatusCategoryCode(str));
		OutputString += strFmt;
	}

	//STC11 - 2 1271		Industry Code						M	AN	1/30
	//"This is the Status code. Use code source 508."
	str = ParseComposite(strComposite);
	if(str != "") {
		strFmt.Format("Claim Status: %s\r\n",GetStatusCode(atoi(str)));
		OutputString += strFmt;
	}

	//STC11 - 3 98			Entity Identifier Code				O	ID	2/3
	//"Code identifying an organizational entity, a physical location, property or an individual."
	//"STC11-3 further modifies the status code in STC11-2.
	//Required if additional detail applicable to claim status is needed to clarify "
	//"the status and the payer’s system supports this level of detail."
	str = ParseComposite(strComposite);
	if(str != "") {
		if(str == "PR")
			strFmt = "Entity: Payer";
		else if(str == "41")
			strFmt = "Entity: Submitter";
		else if(str == "1P")
			strFmt = "Entity: Provider";
		else if(str == "IL")
			strFmt = "Entity: Insured/Subscriber";
		else if(str == "QC")
			strFmt = "Entity: Patient";
		else
			strFmt = "Entity: Unknown";
		OutputString += strFmt;
		OutputString += "\r\n";
	}

	if(strFmt != "")
		OutputString += "\r\n";

	strFmt = "";

	//STC12		933			Free-Form Message Text				O	AN	1/264
	//NOT USED
	str = ParseElement(strIn);

	OutputString += "\r\n";

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_REF(CString &strIn) {

	//Reference Identification, used in:

	//165		REF			Payer Claim Identification Number	S			1
	//167		REF			Institutional Bill Type Ident.		S			1
	//169		REF			Medical Record Identification		S			1
	//187		REF			Service Line Item Identification	S			1
	//210		REF			Payer Claim Identification Number	R			1
	//212		REF			Institutional Bill Type Ident.		S			1
	//214		REF			Medical Record Identification		S			1
	//231		REF			Service Line Item Identification	S			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//REF01		128			Reference Identification Qualifier	M	ID	2/3
	//"1K" - Payor’s Claim Number
			//"This data element corresponds to the value given in the ANSI ASC X12 837 transaction in CLM01."
	//"BLT" - Billing Type
			//"Man I could really go for a BLT right now."
	//"EA" - Medical Record Identification Number
			//"It's in the game."
	//"FJ" - Line Item Control Number
	//"LU" - Location Number (Group Number)
	str = ParseElement(strIn);

	//REF02		127			Reference Identification			X	AN	1/30
	//the ID number specified by REF01
	str = ParseElement(strIn);

	//REF03		352			Description							X	AN	1/80
	//NOT USED
	str = ParseElement(strIn);

	//REF04		C040		REFERENCE IDENTIFIER				O
	//NOT USED
	str = ParseElement(strIn);

	/*
	//TODO: do we need to output this? What is it?
	strFmt.Format("Transaction Set Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;
	*/

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_DTP(CString &strIn) {

	//Date or Time or Period, used in:
	//171		DTP			Claim Service Date					S			1
	//188		DTP			Service Line Date					S			1
	//216		DTP			Claim Service Date					S			1
	//232		DTP			Service Line Date					S			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	CString strIdent, strQual, strDate;

	//DTP01		374			Date/Time Qualifier					M	ID	3/3
	//"232" - 2200D or 2200E - Claim Statement Period Start (Claim Service Date)
	//"472" - 2220D or 2220E - Line Item Service Date
	strIdent = ParseElement(strIn);

	//DTP02		1250		Date Time Period Format Qualifier	M	ID	2/3
	//"RD8" - Range of Dates Expressed in Format CCYYMMDD-CCYYMMDD
		//If there is a single date of service, the begin date equals the end date.
	strQual = ParseElement(strIn);

	//DTP03		1251		Date Time Period					M	AN	1/35
	//in the format specified by DTP02
	strDate = ParseElement(strIn);

	if(strIdent == "232")
		strFmt = "Claim Service Date: ";
	else if(strIdent == "472")
		strFmt = "Line Item Service Date: ";
	else
		strFmt = "Date: ";

	if(strQual == "RD8") {
		CString strDateRange;
		if(strDate.Find("-") != -1) {
			CString strDate1, strDate2;
			strDate1 = strDate.Left(strDate.Find("-"));
			strDate2 = strDate.Right(strDate.GetLength() - strDate.Find("-") - 1);
			strDateRange.Format("%s/%s/%s - %s/%s/%s",strDate1.Mid(4,2),strDate1.Right(2),strDate1.Left(4),strDate2.Mid(4,2),strDate2.Right(2),strDate2.Left(4));
		}
		else {
			//one date
			strDateRange.Format("%s/%s/%s",strDate.Mid(4,2),strDate.Right(2),strDate.Left(4));
		}
		
		strFmt += strDateRange;
	}
	else
		strFmt += strDate;

	OutputString = strFmt;
	OutputString += "\r\n\r\n";

	m_OutputFile.Write(OutputString,OutputString.GetLength());
}

void CANSI277ReportParserDlg::ANSI_SVC(CString &strIn) {

	//Service Line Information, used in:
	//173		SVC			Service Line Information			S			1
	//218		SVC			Service Line Information			S			1

	CString OutputString, str, strFmt;

	//Ref.		Data		Name								Attributes
	//Des.		Element

	//SVC01		C003		COMPOSITE MEDICAL PROCEDURE IDENT.	M

	CString strComposite = ParseElement(strIn);

	//SVC01 - 1 235			Product/Service ID Qualifier		M	ID	2/2
	//can be many things, but hopefully we will only encounter:
	//"HC" - CPT/HCPCS Code
	str = ParseComposite(strComposite);

	//SVC01 - 2	234			Product/Service ID					M	AN	1/48
	//Service Identification Code, the type of which is identified by SVC01
	str = ParseComposite(strComposite);

	//SVC01 - 3	1339		Procedure Modifier					O	AN	2/2
	str = ParseComposite(strComposite);

	//SVC01 - 4	1339		Procedure Modifier					O	AN	2/2
	str = ParseComposite(strComposite);

	//SVC01 - 5	1339		Procedure Modifier					O	AN	2/2
	str = ParseComposite(strComposite);

	//SVC01 - 6	1339		Procedure Modifier					O	AN	2/2
	str = ParseComposite(strComposite);

	//SVC01 - 7 352			Description							O	AN	1/80
	//NOT USED
	str = ParseComposite(strComposite);

	//SVC02		782			Monetary Amount						M	R	1/18
	//Line Item Charge Amount (This amount is the original submitted charge.)
	str = ParseElement(strIn);

	//SVC03		782			Monetary Amount						O	R	1/18
	//Line Item Provider Payment Amount (This amount is the amount paid.)
	str = ParseElement(strIn);

	//SVC04		234			Product/Service ID					O	AN	1/48
	//Revenue Code
	str = ParseElement(strIn);

	//SVC05		380			Quantity							O	R	1/15
	//NOT USED
	str = ParseElement(strIn);

	//SVC06		C003		COMPOSITE MEDICAL PROCEDURE IDENT.	O
	//NOT USED
	str = ParseElement(strIn);

	//SVC07		380			Quantity							O	R	1/15
	//Original Units of Service Count (This quantity is the submitted units of service.)
	str = ParseElement(strIn);

	/*
	//TODO: do we need to output this? What is it?
	strFmt.Format("Transaction Set Control Number: %s\r\n\r\n",str);
	OutputString = strFmt;

	m_OutputFile.Write(OutputString,OutputString.GetLength());
	*/
}

CString CANSI277ReportParserDlg::GetStatusCategoryCode(CString strCodeID) {

	if(strCodeID == "A0") {
		return "Acknowledgement/Forwarded-The claim/encounter has been forwarded to another entity.";
	}
	else if(strCodeID == "A1") {
		return "Acknowledgement/Receipt-The claim/encounter has been received. This does not mean that the claim has been accepted for adjudication.";
	}
	else if(strCodeID == "A2") {
		return "Acknowledgement/Acceptance into adjudication system-The claim/encounter has been accepted into the adjudication system.";
	}
	else if(strCodeID == "A3") {
		return "Acknowledgement/Returned as unprocessable claim-The claim/encounter has been rejected and has not been entered into the adjudication system.";
	}
	else if(strCodeID == "A4") {
		return "Acknowledgement/Not Found-The claim/encounter can not be found in the adjudication system.";
	}
	else if(strCodeID == "A5") {
		return "Acknowledgement/Split Claim-The claim/encounter has been split upon acceptance into the adjudication system.";
	}
	else if(strCodeID == "A6") {
		return "Acknowledgement/Rejected for Missing Information - The claim/encounter is missing the information specified in the Status details and has been rejected.";
	}
	else if(strCodeID == "A7") {
		return "Acknowledgement/Rejected for Invalid Information - The claim/encounter has invalid information as specified in the Status details and has been rejected.";
	}
	else if(strCodeID == "P0") {
		return "Pending: Adjudication/Details-This is a generic message about a pended claim. A pended claim is one for which no remittance advice has been issued, or only part of the claim has been paid.";
	}
	else if(strCodeID == "P1") {
		return "Pending/In Process-The claim or encounter is in the adjudication system.";
	}
	else if(strCodeID == "P2") {
		return "Pending/In Review-The claim/encounter is suspended pending review.";
	}
	else if(strCodeID == "P3") {
		return "Pending/Requested Information-The claim or encounter is waiting for information that has already been requested.";
	}
	else if(strCodeID == "P4") {
		return "Pending/Patient Requested Information";
	}
	else if(strCodeID == "F0") {
		return "Finalized-The claim/encounter has completed the adjudication cycle and no more action will be taken.";
	}
	else if(strCodeID == "F1") {
		return "Finalized/Payment-The claim/line has been paid.";
	}
	else if(strCodeID == "F2") {
		return "Finalized/Denial-The claim/line has been denied.";
	}
	else if(strCodeID == "F3") {
		return "Finalized/Revised - Adjudication information has been changed.";
	}
	else if(strCodeID == "F3F") {
		return "Finalized/Forwarded-The claim/encounter processing has been completed. Any applicable payment has been made and the claim/encounter has been forwarded to a subsequent entity as identified on the original claim or in this payer's records.";
	}
	else if(strCodeID == "F3N") {
		return "Finalized/Not Forwarded-The claim/encounter processing has been completed. Any applicable payment has been made. The claim/encounter has NOT been forwarded to any subsequent entity identified on the original claim.";
	}
	else if(strCodeID == "F4") {
		return "Finalized/Adjudication Complete - No payment forthcoming-The claim/encounter has been adjudicated and no further payment is forthcoming.";
	}
	else if(strCodeID == "F5") {
		return "Finalized/Cannot Process";
	}
	else if(strCodeID == "R0") {
		return "Requests for additional Information/General Requests-Requests that don't fall into other R-type categories.";
	}
	else if(strCodeID == "R1") {
		return "Requests for additional Information/Entity Requests-Requests for information about specific entities (subscribers, patients, various providers).";
	}
	else if(strCodeID == "R3") {
		return "Requests for additional Information/Claim/Line-Requests for information that could normally be submitted on a claim.";
	}
	else if(strCodeID == "R4") {
		return "Requests for additional Information/Documentation-Requests for additional supporting documentation. Examples: certification, x-ray, notes.";
	}
	else if(strCodeID == "R5") {
		return "Request for additional information/more specific detail-Additional information as a follow up to a previous request is needed. The original information was received but is inadequate. More specific/detailed information is requested.";
	}
	else if(strCodeID == "RQ") {
		return "General Questions (Yes/No Responses)-Questions that may be answered by a simple 'yes' or 'no'.";
	}
	else if(strCodeID == "E0") {
		return "Response not possible - error on submitted request data";
	}
	else if(strCodeID == "E1") {
		return "Response not possible - System Status";
	}
	else if(strCodeID == "E2") {
		return "Information Holder is not responding; resubmit at a later time.";
	}
	else if(strCodeID == "D0") {
		return "Entity not found - change search criteria";
	}
	else {
		return "";
	}

}

CString CANSI277ReportParserDlg::GetStatusCode(long CodeID) {

	switch (CodeID) {
		case 0	:	return "Cannot provide further status electronically.";	break;
		case 1	:	return "For more detailed information, see remittance advice.";	break;
		case 2	:	return "More detailed information in letter.";	break;
		case 3	:	return "Claim has been adjudicated and is awaiting payment cycle.";	break;
		case 4	:	return "This is a subsequent request for information from the original request.";	break;
		case 5	:	return "This is a final request for information.";	break;
		case 6	:	return "Balance due from the subscriber.";	break;
		case 7	:	return "Claim may be reconsidered at a future date.";	break;
		case 8	:	return "No payment due to contract/plan provisions.";	break;
		case 9	:	return "No payment will be made for this claim.";	break;
		case 10	:	return "All originally submitted procedure codes have been combined.";	break;
		case 11	:	return "Some originally submitted procedure codes have been combined.";	break;
		case 12	:	return "One or more originally submitted procedure codes have been combined.";	break;
		case 13	:	return "All originally submitted procedure codes have been modified.";	break;
		case 14	:	return "Some all originally submitted procedure codes have been modified.";	break;
		case 15	:	return "One or more originally submitted procedure code have been modified.";	break;
		case 16	:	return "Claim/encounter has been forwarded to entity.";	break;
		case 17	:	return "Claim/encounter has been forwarded by third party entity to entity.";	break;
		case 18	:	return "Entity received claim/encounter, but returned invalid status.";	break;
		case 19	:	return "Entity acknowledges receipt of claim/encounter.";	break;
		case 20	:	return "Accepted for processing.";	break;
		case 21	:	return "Missing or invalid information.";	break;
		case 22	:	return "... before entering the adjudication system.";	break;
		case 23	:	return "Returned to Entity.";	break;
		case 24	:	return "Entity not approved as an electronic submitter.";	break;
		case 25	:	return "Entity not approved.";	break;
		case 26	:	return "Entity not found.";	break;
		case 27	:	return "Policy canceled.";	break;
		case 28	:	return "Claim submitted to wrong payer.";	break;
		case 29	:	return "Subscriber and policy number/contract number mismatched.";	break;
		case 30	:	return "Subscriber and subscriber id mismatched.";	break;
		case 31	:	return "Subscriber and policyholder name mismatched.";	break;
		case 32	:	return "Subscriber and policy number/contract number not found.";	break;
		case 33	:	return "Subscriber and subscriber id not found.";	break;
		case 34	:	return "Subscriber and policyholder name not found.";	break;
		case 35	:	return "Claim/encounter not found.";	break;
		case 37	:	return "Predetermination is on file, awaiting completion of services.";	break;
		case 38	:	return "Awaiting next periodic adjudication cycle.";	break;
		case 39	:	return "Charges for pregnancy deferred until delivery.";	break;
		case 40	:	return "Waiting for final approval.";	break;
		case 41	:	return "Special handling required at payer site.";	break;
		case 42	:	return "Awaiting related charges.";	break;
		case 44	:	return "Charges pending provider audit.";	break;
		case 45	:	return "Awaiting benefit determination.";	break;
		case 46	:	return "Internal review/audit.";	break;
		case 47	:	return "Internal review/audit - partial payment made.";	break;
		case 48	:	return "Referral/authorization.";	break;
		case 49	:	return "Pending provider accreditation review.";	break;
		case 50	:	return "Claim waiting for internal provider verification.";	break;
		case 51	:	return "Investigating occupational illness/accident.";	break;
		case 52	:	return "Investigating existence of other insurance coverage.";	break;
		case 53	:	return "Claim being researched for Insured ID/Group Policy Number error.";	break;
		case 54	:	return "Duplicate of a previously processed claim/line.";	break;
		case 55	:	return "Claim assigned to an approver/analyst.";	break;
		case 56	:	return "Awaiting eligibility determination.";	break;
		case 57	:	return "Pending COBRA information requested.";	break;
		case 59	:	return "Non-electronic request for information.";	break;
		case 60	:	return "Electronic request for information.";	break;
		case 61	:	return "Eligibility for extended benefits.";	break;
		case 64	:	return "Re-pricing information.";	break;
		case 65	:	return "Claim/line has been paid.";	break;
		case 66	:	return "Payment reflects usual and customary charges.";	break;
		case 67	:	return "Payment made in full.";	break;
		case 68	:	return "Partial payment made for this claim.";	break;
		case 69	:	return "Payment reflects plan provisions.";	break;
		case 70	:	return "Payment reflects contract provisions.";	break;
		case 71	:	return "Periodic installment released.";	break;
		case 72	:	return "Claim contains split payment.";	break;
		case 73	:	return "Payment made to entity, assignment of benefits not on file.";	break;
		case 78	:	return "Duplicate of an existing claim/line, awaiting processing.";	break;
		case 81	:	return "Contract/plan does not cover pre-existing conditions.";	break;
		case 83	:	return "No coverage for newborns.";	break;
		case 84	:	return "Service not authorized.";	break;
		case 85	:	return "Entity not primary.";	break;
		case 86	:	return "Diagnosis and patient gender mismatch.";	break;
		case 87	:	return "Denied: Entity not found.";	break;
		case 88	:	return "Entity not eligible for benefits for submitted dates of service.";	break;
		case 89	:	return "Entity not eligible for dental benefits for submitted dates of service.";	break;
		case 90	:	return "Entity not eligible for medical benefits for submitted dates of service.";	break;
		case 91	:	return "Entity not eligible/not approved for dates of service.";	break;
		case 92	:	return "Entity does not meet dependent or student qualification.";	break;
		case 93	:	return "Entity is not selected primary care provider.";	break;
		case 94	:	return "Entity not referred by selected primary care provider.";	break;
		case 95	:	return "Requested additional information not received.";	break;
		case 96	:	return "No agreement with entity.";	break;
		case 97	:	return "Patient eligibility not found with entity.";	break;
		case 98	:	return "Charges applied to deductible.";	break;
		case 99	:	return "Pre-treatment review.";	break;
		case 100	:	return "Pre-certification penalty taken.";	break;
		case 101	:	return "Claim was processed as adjustment to previous claim.";	break;
		case 102	:	return "Newborn's charges processed on mother's claim.";	break;
		case 103	:	return "Claim combined with other claim(s).";	break;
		case 105	:	return "Claim/line is capitated.";	break;
		case 106	:	return "This amount is not entity's responsibility.";	break;
		case 107	:	return "Processed according to contract/plan provisions.";	break;
		case 108	:	return "Coverage has been canceled for this entity.";	break;
		case 109	:	return "Entity not eligible.";	break;
		case 110	:	return "Claim requires pricing information.";	break;
		case 111	:	return "At the policyholder's request these claims cannot be submitted electronically.";	break;
		case 112	:	return "Policyholder processes their own claims.";	break;
		case 113	:	return "Cannot process individual insurance policy claims.";	break;
		case 114	:	return "Should be handled by entity.";	break;
		case 115	:	return "Cannot process HMO claims";	break;
		case 116	:	return "Claim submitted to incorrect payer.";	break;
		case 117	:	return "Claim requires signature-on-file indicator.";	break;
		case 118	:	return "TPO rejected claim/line because payer name is missing.";	break;
		case 119	:	return "TPO rejected claim/line because certification information is missing";	break;
		case 120	:	return "TPO rejected claim/line because claim does not contain enough information";	break;
		case 121	:	return "Service line number greater than maximum allowable for payer.";	break;
		case 122	:	return "Missing/invalid data prevents payer from processing claim.";	break;
		case 123	:	return "Additional information requested from entity.";	break;
		case 124	:	return "Entity's name, address, phone and id number.";	break;
		case 125	:	return "Entity's name.";	break;
		case 126	:	return "Entity's address.";	break;
		case 127	:	return "Entity's phone number.";	break;
		case 128	:	return "Entity's tax id.";	break;
		case 129	:	return "Entity's Blue Cross provider id";	break;
		case 130	:	return "Entity's Blue Shield provider id";	break;
		case 131	:	return "Entity's Medicare provider id.";	break;
		case 132	:	return "Entity's Medicaid provider id.";	break;
		case 133	:	return "Entity's UPIN";	break;
		case 134	:	return "Entity's CHAMPUS provider id.";	break;
		case 135	:	return "Entity's commercial provider id.";	break;
		case 136	:	return "Entity's health industry id number.";	break;
		case 137	:	return "Entity's plan network id.";	break;
		case 138	:	return "Entity's site id .";	break;
		case 139	:	return "Entity's health maintenance provider id (HMO).";	break;
		case 140	:	return "Entity's preferred provider organization id (PPO).";	break;
		case 141	:	return "Entity's administrative services organization id (ASO).";	break;
		case 142	:	return "Entity's license/certification number.";	break;
		case 143	:	return "Entity's state license number.";	break;
		case 144	:	return "Entity's specialty license number.";	break;
		case 145	:	return "Entity's specialty code.";	break;
		case 146	:	return "Entity's anesthesia license number.";	break;
		case 147	:	return "Entity's qualification degree/designation (e.g. RN,PhD,MD)";	break;
		case 148	:	return "Entity's social security number.";	break;
		case 149	:	return "Entity's employer id.";	break;
		case 150	:	return "Entity's drug enforcement agency (DEA) number.";	break;
		case 152	:	return "Pharmacy processor number.";	break;
		case 153	:	return "Entity's id number.";	break;
		case 154	:	return "Relationship of surgeon & assistant surgeon.";	break;
		case 155	:	return "Entity's relationship to patient";	break;
		case 156	:	return "Patient relationship to subscriber";	break;
		case 157	:	return "Entity's Gender";	break;
		case 158	:	return "Entity's date of birth";	break;
		case 159	:	return "Entity's date of death";	break;
		case 160	:	return "Entity's marital status";	break;
		case 161	:	return "Entity's employment status";	break;
		case 162	:	return "Entity's health insurance claim number (HICN).";	break;
		case 163	:	return "Entity's policy number.";	break;
		case 164	:	return "Entity's contract/member number.";	break;
		case 165	:	return "Entity's employer name, address and phone.";	break;
		case 166	:	return "Entity's employer name.";	break;
		case 167	:	return "Entity's employer address.";	break;
		case 168	:	return "Entity's employer phone number.";	break;
		case 169	:	return "Entity's employer id.";	break;
		case 170	:	return "Entity's employee id.";	break;
		case 171	:	return "Other insurance coverage information (health, liability, auto, etc.).";	break;
		case 172	:	return "Other employer name, address and telephone number.";	break;
		case 173	:	return "Entity's name, address, phone, gender, DOB, marital status, employment status and relation to subscriber.";	break;
		case 174	:	return "Entity's student status.";	break;
		case 175	:	return "Entity's school name.";	break;
		case 176	:	return "Entity's school address.";	break;
		case 177	:	return "Transplant recipient's name, date of birth, gender, relationship to insured.";	break;
		case 178	:	return "Submitted charges.";	break;
		case 179	:	return "Outside lab charges.";	break;
		case 180	:	return "Hospital s semi-private room rate.";	break;
		case 181	:	return "Hospital s room rate.";	break;
		case 182	:	return "Allowable/paid from primary coverage.";	break;
		case 183	:	return "Amount entity has paid.";	break;
		case 184	:	return "Purchase price for the rented durable medical equipment.";	break;
		case 185	:	return "Rental price for durable medical equipment.";	break;
		case 186	:	return "Purchase and rental price of durable medical equipment.";	break;
		case 187	:	return "Date(s) of service.";	break;
		case 188	:	return "Statement from-through dates.";	break;
		case 189	:	return "Hospital admission date.";	break;
		case 190	:	return "Hospital discharge date.";	break;
		case 191	:	return "Date of Last Menstrual Period (LMP)";	break;
		case 192	:	return "Date of first service for current series/symptom/illness.";	break;
		case 193	:	return "First consultation/evaluation date.";	break;
		case 194	:	return "Confinement dates.";	break;
		case 195	:	return "Unable to work dates.";	break;
		case 196	:	return "Return to work dates.";	break;
		case 197	:	return "Effective coverage date(s).";	break;
		case 198	:	return "Medicare effective date.";	break;
		case 199	:	return "Date of conception and expected date of delivery.";	break;
		case 200	:	return "Date of equipment return.";	break;
		case 201	:	return "Date of dental appliance prior placement.";	break;
		case 202	:	return "Date of dental prior replacement/reason for replacement.";	break;
		case 203	:	return "Date of dental appliance placed.";	break;
		case 204	:	return "Date dental canal(s) opened and date service completed.";	break;
		case 205	:	return "Date(s) dental root canal therapy previously performed.";	break;
		case 206	:	return "Most recent date of curettage, root planing, or periodontal surgery.";	break;
		case 207	:	return "Dental impression and seating date.";	break;
		case 104	:	return "Processed according to plan provisions.";	break;
		case 208	:	return "Most recent date pacemaker was implanted.";	break;
		case 209	:	return "Most recent pacemaker battery change date.";	break;
		case 210	:	return "Date of the last x-ray.";	break;
		case 211	:	return "Date(s) of dialysis training provided to patient.";	break;
		case 212	:	return "Date of last routine dialysis.";	break;
		case 213	:	return "Date of first routine dialysis.";	break;
		case 214	:	return "Original date of prescription/orders/referral.";	break;
		case 215	:	return "Date of tooth extraction/evolution.";	break;
		case 216	:	return "Drug information.";	break;
		case 217	:	return "Drug name, strength and dosage form.";	break;
		case 218	:	return "NDC number.";	break;
		case 219	:	return "Prescription number.";	break;
		case 220	:	return "Drug product id number.";	break;
		case 221	:	return "Drug days supply and dosage.";	break;
		case 222	:	return "Drug dispensing units and average wholesale price (AWP).";	break;
		case 223	:	return "Route of drug/myelogram administration.";	break;
		case 224	:	return "Anatomical location for joint injection.";	break;
		case 225	:	return "Anatomical location.";	break;
		case 226	:	return "Joint injection site.";	break;
		case 227	:	return "Hospital information.";	break;
		case 228	:	return "Type of bill for UB-92 claim.";	break;
		case 229	:	return "Hospital admission source.";	break;
		case 230	:	return "Hospital admission hour.";	break;
		case 231	:	return "Hospital admission type.";	break;
		case 232	:	return "Admitting diagnosis.";	break;
		case 233	:	return "Hospital discharge hour.";	break;
		case 234	:	return "Patient discharge status.";	break;
		case 235	:	return "Units of blood furnished.";	break;
		case 236	:	return "Units of blood replaced.";	break;
		case 237	:	return "Units of deductible blood.";	break;
		case 238	:	return "Separate claim for mother/baby charges.";	break;
		case 239	:	return "Dental information.";	break;
		case 240	:	return "Tooth surface(s) involved.";	break;
		case 241	:	return "List of all missing teeth (upper and lower).";	break;
		case 242	:	return "Tooth numbers, surfaces, and/or quadrants involved.";	break;
		case 243	:	return "Months of dental treatment remaining.";	break;
		case 244	:	return "Tooth number or letter.";	break;
		case 245	:	return "Dental quadrant/arch.";	break;
		case 246	:	return "Total orthodontic service fee, initial appliance fee, monthly fee, length of service.";	break;
		case 247	:	return "Line information.";	break;
		case 248	:	return "Accident date, state, description and cause.";	break;
		case 249	:	return "Place of service.";	break;
		case 250	:	return "Type of service.";	break;
		case 251	:	return "Total anesthesia minutes.";	break;
		case 252	:	return "Authorization/certification number.";	break;
		case 253	:	return "Procedure/revenue code for service(s) rendered. Please use codes 454 or 455.";	break;
		case 254	:	return "Primary diagnosis code.";	break;
		case 255	:	return "Diagnosis code.";	break;
		case 256	:	return "DRG code(s).";	break;
		case 257	:	return "ADSM-III-R code for services rendered.";	break;
		case 258	:	return "Days/units for procedure/revenue code.";	break;
		case 259	:	return "Frequency of service.";	break;
		case 260	:	return "Length of medical necessity, including begin date.";	break;
		case 261	:	return "Obesity measurements.";	break;
		case 262	:	return "Type of surgery/service for which anesthesia was administered.";	break;
		case 263	:	return "Length of time for services rendered.";	break;
		case 264	:	return "Number of liters/minute & total hours/day for respiratory support.";	break;
		case 265	:	return "Number of lesions excised.";	break;
		case 266	:	return "Facility point of origin and destination - ambulance.";	break;
		case 267	:	return "Number of miles patient was transported.";	break;
		case 268	:	return "Location of durable medical equipment use.";	break;
		case 269	:	return "Length/size of laceration/tumor.";	break;
		case 270	:	return "Subluxation location.";	break;
		case 271	:	return "Number of spine segments.";	break;
		case 272	:	return "Oxygen contents for oxygen system rental.";	break;
		case 273	:	return "Weight.";	break;
		case 274	:	return "Height.";	break;
		case 275	:	return "Claim.";	break;
		case 276	:	return "UB-92/HCFA-1450/HCFA-1500 claim form.";	break;
		case 277	:	return "Paper claim.";	break;
		case 278	:	return "Signed claim form.";	break;
		case 279	:	return "Itemized claim.";	break;
		case 280	:	return "Itemized claim by provider.";	break;
		case 281	:	return "Related confinement claim.";	break;
		case 282	:	return "Copy of prescription.";	break;
		case 283	:	return "Medicare worksheet.";	break;
		case 284	:	return "Copy of Medicare ID card.";	break;
		case 285	:	return "Vouchers/explanation of benefits (EOB).";	break;
		case 286	:	return "Other payer's Explanation of Benefits/payment information.";	break;
		case 287	:	return "Medical necessity for service.";	break;
		case 288	:	return "Reason for late hospital charges.";	break;
		case 289	:	return "Reason for late discharge.";	break;
		case 290	:	return "Pre-existing information.";	break;
		case 291	:	return "Reason for termination of pregnancy.";	break;
		case 292	:	return "Purpose of family conference/therapy.";	break;
		case 293	:	return "Reason for physical therapy.";	break;
		case 294	:	return "Supporting documentation.";	break;
		case 295	:	return "Attending physician report.";	break;
		case 296	:	return "Nurse's notes.";	break;
		case 297	:	return "Medical notes/report.";	break;
		case 298	:	return "Operative report.";	break;
		case 299	:	return "Emergency room notes/report.";	break;
		case 300	:	return "Lab/test report/notes/results.";	break;
		case 301	:	return "MRI report.";	break;
		case 302	:	return "Refer to codes 300 for lab notes and 311 for pathology notes";	break;
		case 303	:	return "Physical therapy notes. Please use code 297:6O (6 'OH' - not zero)";	break;
		case 304	:	return "Reports for service.";	break;
		case 305	:	return "X-ray reports/interpretation.";	break;
		case 306	:	return "Detailed description of service.";	break;
		case 307	:	return "Narrative with pocket depth chart.";	break;
		case 308	:	return "Discharge summary.";	break;
		case 309	:	return "Code was duplicate of code 299";	break;
		case 310	:	return "Progress notes for the six months prior to statement date.";	break;
		case 311	:	return "Pathology notes/report.";	break;
		case 312	:	return "Dental charting.";	break;
		case 313	:	return "Bridgework information.";	break;
		case 314	:	return "Dental records for this service.";	break;
		case 315	:	return "Past perio treatment history.";	break;
		case 316	:	return "Complete medical history.";	break;
		case 317	:	return "Patient's medical records.";	break;
		case 318	:	return "X-rays.";	break;
		case 319	:	return "Pre/post-operative x-rays/photographs.";	break;
		case 320	:	return "Study models.";	break;
		case 321	:	return "Radiographs or models.";	break;
		case 322	:	return "Recent fm x-rays.";	break;
		case 323	:	return "Study models, x-rays, and/or narrative.";	break;
		case 324	:	return "Recent x-ray of treatment area and/or narrative.";	break;
		case 325	:	return "Recent fm x-rays and/or narrative.";	break;
		case 326	:	return "Copy of transplant acquisition invoice.";	break;
		case 327	:	return "Periodontal case type diagnosis and recent pocket depth chart with narrative.";	break;
		case 328	:	return "Speech therapy notes. Please use code 297:6R";	break;
		case 329	:	return "Exercise notes.";	break;
		case 330	:	return "Occupational notes.";	break;
		case 331	:	return "History and physical.";	break;
		case 332	:	return "Authorization/certification (include period covered).";	break;
		case 333	:	return "Patient release of information authorization.";	break;
		case 334	:	return "Oxygen certification.";	break;
		case 335	:	return "Durable medical equipment certification.";	break;
		case 336	:	return "Chiropractic certification.";	break;
		case 337	:	return "Ambulance certification/documentation.";	break;
		case 338	:	return "Home health certification. Please use code 332:4Y";	break;
		case 339	:	return "Enteral/parenteral certification.";	break;
		case 340	:	return "Pacemaker certification.";	break;
		case 341	:	return "Private duty nursing certification.";	break;
		case 342	:	return "Podiatric certification.";	break;
		case 343	:	return "Documentation that facility is state licensed and Medicare approved as a surgical facility.";	break;
		case 344	:	return "Documentation that provider of physical therapy is Medicare Part B approved.";	break;
		case 345	:	return "Treatment plan for service/diagnosis";	break;
		case 346	:	return "Proposed treatment plan for next 6 months.";	break;
		case 347	:	return "Refer to code 345 for treatment plan and code 282 for prescription";	break;
		case 348	:	return "Chiropractic treatment plan.";	break;
		case 349	:	return "Psychiatric treatment plan. Please use codes 345:5I, 5J, 5K, 5L, 5M, 5N, 5O (5 'OH' - not zero), 5P";	break;
		case 350	:	return "Speech pathology treatment plan. Please use code 345:6R";	break;
		case 351	:	return "Physical/occupational therapy treatment plan. Please use codes 345:6O (6 'OH' - not zero), 6N";	break;
		case 352	:	return "Duration of treatment plan.";	break;
		case 353	:	return "Orthodontics treatment plan.";	break;
		case 354	:	return "Treatment plan for replacement of remaining missing teeth.";	break;
		case 355	:	return "Has claim been paid?";	break;
		case 356	:	return "Was blood furnished?";	break;
		case 357	:	return "Has or will blood be replaced?";	break;
		case 358	:	return "Does provider accept assignment of benefits?";	break;
		case 359	:	return "Is there a release of information signature on file?";	break;
		case 360	:	return "Is there an assignment of benefits signature on file?";	break;
		case 361	:	return "Is there other insurance?";	break;
		case 362	:	return "Is the dental patient covered by medical insurance?";	break;
		case 363	:	return "Will worker's compensation cover submitted charges?";	break;
		case 364	:	return "Is accident/illness/condition employment related?";	break;
		case 365	:	return "Is service the result of an accident?";	break;
		case 366	:	return "Is injury due to auto accident?";	break;
		case 367	:	return "Is service performed for a recurring condition or new condition?";	break;
		case 368	:	return "Is medical doctor (MD) or doctor of osteopath (DO) on staff of this facility?";	break;
		case 369	:	return "Does patient condition preclude use of ordinary bed?";	break;
		case 370	:	return "Can patient operate controls of bed?";	break;
		case 371	:	return "Is patient confined to room?";	break;
		case 372	:	return "Is patient confined to bed?";	break;
		case 373	:	return "Is patient an insulin diabetic?";	break;
		case 374	:	return "Is prescribed lenses a result of cataract surgery?";	break;
		case 375	:	return "Was refraction performed?";	break;
		case 376	:	return "Was charge for ambulance for a round-trip?";	break;
		case 377	:	return "Was durable medical equipment purchased new or used?";	break;
		case 378	:	return "Is pacemaker temporary or permanent?";	break;
		case 379	:	return "Were services performed supervised by a physician?";	break;
		case 380	:	return "Were services performed by a CRNA under appropriate medical direction?";	break;
		case 381	:	return "Is drug generic?";	break;
		case 382	:	return "Did provider authorize generic or brand name dispensing?";	break;
		case 383	:	return "Was nerve block used for surgical procedure or pain management?";	break;
		case 384	:	return "Is prosthesis/crown/inlay placement an initial placement or a replacement?";	break;
		case 385	:	return "Is appliance upper or lower arch & is appliance fixed or removable?";	break;
		case 386	:	return "Is service for orthodontic purposes?";	break;
		case 387	:	return "Date patient last examined by entity";	break;
		case 388	:	return "Date post-operative care assumed";	break;
		case 389	:	return "Date post-operative care relinquished";	break;
		case 390	:	return "Date of most recent medical event necessitating service(s)";	break;
		case 391	:	return "Date(s) dialysis conducted";	break;
		case 392	:	return "Date(s) of blood transfusion(s)";	break;
		case 393	:	return "Date of previous pacemaker check";	break;
		case 394	:	return "Date(s) of most recent hospitalization related to service";	break;
		case 395	:	return "Date entity signed certification/recertification";	break;
		case 396	:	return "Date home dialysis began";	break;
		case 397	:	return "Date of onset/exacerbation of illness/condition";	break;
		case 398	:	return "Visual field test results";	break;
		case 399	:	return "Report of prior testing related to this service, including dates";	break;
		case 400	:	return "Claim is out of balance";	break;
		case 401	:	return "Source of payment is not valid";	break;
		case 402	:	return "Amount must be greater than zero";	break;
		case 403	:	return "Entity referral notes/orders/prescription";	break;
		case 404	:	return "Specific findings, complaints, or symptoms necessitating service";	break;
		case 405	:	return "Summary of services";	break;
		case 406	:	return "Brief medical history as related to service(s)";	break;
		case 407	:	return "Complications/mitigating circumstances";	break;
		case 408	:	return "Initial certification";	break;
		case 409	:	return "Medication logs/records (including medication therapy)";	break;
		case 410	:	return "Explain differences between treatment plan and patient's condition";	break;
		case 411	:	return "Medical necessity for non-routine service(s)";	break;
		case 412	:	return "Medical records to substantiate decision of non-coverage";	break;
		case 413	:	return "Explain/justify differences between treatment plan and services rendered.";	break;
		case 414	:	return "Need for more than one physician to treat patient";	break;
		case 415	:	return "Justify services outside composite rate";	break;
		case 416	:	return "Verification of patient's ability to retain and use information";	break;
		case 417	:	return "Prior testing, including result(s) and date(s) as related to service(s)";	break;
		case 418	:	return "Indicating why medications cannot be taken orally";	break;
		case 419	:	return "Individual test(s) comprising the panel and the charges for each test";	break;
		case 420	:	return "Name, dosage and medical justification of contrast material used for radiology procedure";	break;
		case 421	:	return "Medical review attachment/information for service(s)";	break;
		case 422	:	return "Homebound status";	break;
		case 423	:	return "Prognosis";	break;
		case 424	:	return "Statement of non-coverage including itemized bill";	break;
		case 425	:	return "Itemize non-covered services";	break;
		case 426	:	return "All current diagnoses";	break;
		case 427	:	return "Emergency care provided during transport";	break;
		case 428	:	return "Reason for transport by ambulance";	break;
		case 429	:	return "Loaded miles and charges for transport to nearest facility with appropriate services";	break;
		case 430	:	return "Nearest appropriate facility";	break;
		case 431	:	return "Provide condition/functional status at time of service";	break;
		case 432	:	return "Date benefits exhausted";	break;
		case 433	:	return "Copy of patient revocation of hospice benefits";	break;
		case 434	:	return "Reasons for more than one transfer per entitlement period";	break;
		case 435	:	return "Notice of Admission";	break;
		case 436	:	return "Short term goals";	break;
		case 437	:	return "Long term goals";	break;
		case 438	:	return "Number of patients attending session";	break;
		case 439	:	return "Size, depth, amount, and type of drainage wounds";	break;
		case 440	:	return "why non-skilled caregiver has not been taught procedure";	break;
		case 441	:	return "Entity professional qualification for service(s)";	break;
		case 442	:	return "Modalities of service";	break;
		case 443	:	return "Initial evaluation report";	break;
		case 444	:	return "Method used to obtain test sample";	break;
		case 445	:	return "Explain why hearing loss not correctable by hearing aid";	break;
		case 446	:	return "Documentation from prior claim(s) related to service(s)";	break;
		case 447	:	return "Plan of teaching";	break;
		case 448	:	return "Invalid billing combination. See STC12 for details. This code should only be used to indicate an inconsistency between two or more data elements on the claim. A detailed explanation is required in STC12 when this code is used.";	break;
		case 449	:	return "Projected date to discontinue service(s)";	break;
		case 450	:	return "Awaiting spend down determination";	break;
		case 451	:	return "Preoperative and post-operative diagnosis";	break;
		case 452	:	return "Total visits in total number of hours/day and total number of hours/week";	break;
		case 453	:	return "Procedure Code Modifier(s) for Service(s) Rendered";	break;
		case 454	:	return "Procedure code for services rendered.";	break;
		case 455	:	return "Revenue code for services rendered.";	break;
		case 456	:	return "Covered Day(s)";	break;
		case 457	:	return "Non-Covered Day(s)";	break;
		case 458	:	return "Coinsurance Day(s)";	break;
		case 459	:	return "Lifetime Reserve Day(s)";	break;
		case 460	:	return "NUBC Condition Code(s)";	break;
		case 461	:	return "NUBC Occurrence Code(s) and Date(s)";	break;
		case 462	:	return "NUBC Occurrence Span Code(s) and Date(s)";	break;
		case 463	:	return "NUBC Value Code(s) and/or Amount(s)";	break;
		case 464	:	return "Payer Assigned Control Number";	break;
		case 465	:	return "Principal Procedure Code for Service(s) Rendered";	break;
		case 466	:	return "Entities Original Signature";	break;
		case 467	:	return "Entity Signature Date";	break;
		case 468	:	return "Patient Signature Source";	break;
		case 469	:	return "Purchase Service Charge";	break;
		case 470	:	return "Was service purchased from another entity?";	break;
		case 471	:	return "Were services related to an emergency?";	break;
		case 472	:	return "Ambulance Run Sheet";	break;
		case 473	:	return "Missing or invalid lab indicator";	break;
		case 474	:	return "Procedure code and patient gender mismatch";	break;
		case 475	:	return "Procedure code not valid for patient age";	break;
		case 476	:	return "Missing or invalid units of service";	break;
		case 477	:	return "Diagnosis code pointer is missing or invalid";	break;
		case 478	:	return "Claim submitter's identifier (patient account number) is missing";	break;
		case 479	:	return "Other Carrier payer ID is missing or invalid";	break;
		case 480	:	return "Other Carrier Claim filing indicator is missing or invalid";	break;
		case 481	:	return "Claim/submission format is invalid.";	break;
		case 482	:	return "Date Error, Century Missing";	break;
		case 483	:	return "Maximum coverage amount met or exceeded for benefit period.";	break;
		case 484	:	return "Business Application Currently Not Available";	break;
		case 485	:	return "More information available than can be returned in real time mode. Narrow your current search criteria.";	break;
		case 486	:	return "Principle Procedure Date";	break;
		case 487	:	return "Claim not found, claim should have been submitted to/through 'entity'";	break;
		case 488	:	return "Diagnosis code(s) for the services rendered.";	break;
		case 489	:	return "Attachment Control Number";	break;
		case 490	:	return "Other Procedure Code for Service(s) Rendered";	break;
		case 491	:	return "Entity not eligible for encounter submission";	break;
		case 492	:	return "Other Procedure Date";	break;
		case 493	:	return "Version/Release/Industry ID code not currently supported by information holder";	break;
		case 494	:	return "Real-Time requests not supported by the information holder, resubmit as batch request";	break;
		case 495	:	return "Requests for re-adjudication must reference the newly assigned payer claim control number for this previously adjusted claim. Correct the payer claim control number and re-submit.";	break;

		default:	
			return "Status Unknown";
	}
}
