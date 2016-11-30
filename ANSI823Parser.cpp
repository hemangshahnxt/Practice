// (d.singleton 2014-07-15 15:08) - PLID 62896 - parse the data from an ANSI 823 file and store data in memory
#include "StdAfx.h"
#include "ANSI823Parser.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "FileUtils.h"

//need to keep track of what loop we are currently in
enum LockboxLoop {
	lblDEP = 0,
	lblBAT,
	lblBPR,
	lblRMR,
};

CANSI823Parser::CANSI823Parser(CWnd* pWnd, CProgressCtrl* pProgressBar, CEdit* pEdit)
{
	m_pParentWnd = pWnd;
	m_pcProgressBar = pProgressBar;
	m_eProgressText = pEdit;
	m_chElementTerminator = '*';
	m_strAnsi823FileName = "";
	m_strFilePath = "";
}

CANSI823Parser::~CANSI823Parser()
{
}

BOOL CANSI823Parser::ParseFile()
{
	CWaitCursor pWait;
	try {
		//get the file if we dont have one already
		if (m_strFilePath.IsEmpty() || !FileUtils::DoesFileOrDirExist(m_strFilePath)) {
			CFileDialog FileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, NULL, m_pParentWnd);			
			if (FileDlg.DoModal() != IDOK) {
				return FALSE;
			}
			m_strFilePath = FileDlg.GetPathName();
		}

		CFile InputFile;
		if (!InputFile.Open(m_strFilePath, CFile::modeReadWrite | CFile::shareCompat)) {
			MessageBox(m_pParentWnd->GetSafeHwnd(), "Failed to open input file.", "Nextech Practice", MB_ICONEXCLAMATION);
			return FALSE;
		}

		const int LEN_16_KB = 16384;
		CString strIn;	//input string
		long iFileSize = InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
		strIn.ReleaseBuffer(iFileSize);		

		CString strLastSegment = "";	//stores data between reads

		if (iFileSize > 0) {
			// (a.walling 2007-08-29 09:54) - PLID 27226 - This should only be done once per file
			// rather than every segment.

			// (a.walling 2008-01-28 16:05) - PLID 28722 - Need to filter any linebreaks here,
			// so they won't be mistakenly used as a terminator.
			strIn.Replace("\r", "");
			strIn.Replace("\n", "");

			int nIndex = strIn.Find("GS*");
			if (nIndex != -1) {
				CString strSegmentTerminator = strIn.Mid(nIndex - 1, 1);
				if (strSegmentTerminator.GetLength() > 0)
					m_chSegmentTerminator = strSegmentTerminator.GetAt(0);
			}
			//make sure this is a valid file
			CString strFileType = strIn.Mid(nIndex + 3, 2);
			if (strFileType != "LB") {
				MessageBox(m_pParentWnd->GetSafeHwnd(), "Please choose a valid Lockbox Payment file before importing.", "Nextech Practice", MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		//warn if it has already been imported before
		if (InputFile.GetFileName().Find("POSTED") != -1) {
			//this has already been imported,  warn them and let them cancel
			if (IDYES != MessageBox(m_pParentWnd->GetSafeHwnd(), "This file has already been imported, are you sure you wish to continue?", "Nextech Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
				return FALSE;
			}
		}

		//parse all the records and store in an array		
		while (iFileSize > 0) {

			strIn.Replace("\r", "");
			strIn.Replace("\n", "");

			m_pcProgressBar->SetPos(0);
			m_pcProgressBar->SetRange(0, strIn.GetLength());
			
			while ((strIn.GetLength() > 0) && (strIn.Find(m_chSegmentTerminator) != -1)) {
				//get our first segment
				CString strSegment = ParseSegment(strIn);
				//set our progress
				m_pcProgressBar->SetPos(m_pcProgressBar->GetPos() + strSegment.GetLength());
				//call function depending on type of segment
				CString strIdent = ParseElement(strSegment);
				if (strIdent == "DEP") {
					ANSI_DEP(strSegment);
				}
				else if (strIdent == "N1") {
					ANSI_N1(strSegment);
				}
				else if (strIdent == "AMT") {
					ANSI_AMT(strSegment);
				}
				else if (strIdent == "QTY") {
					ANSI_QTY(strSegment);
				}
				else if (strIdent == "BPR") {
					ANSI_BPR(strSegment);
				}
				else if (strIdent == "REF") {
					ANSI_REF(strSegment);
				}
				else if (strIdent == "DTM") {
					ANSI_DTM(strSegment);
				}
				else if (strIdent == "RMR") {
					ANSI_RMR(strSegment);
				}

				//dont want to look frozen
				PeekAndPump();
			}
			strLastSegment = strIn;

			iFileSize = InputFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);

			strIn = strLastSegment += strIn;
		}
		
		//rename the file so they know its already been imported		
		m_strAnsi823FileName = InputFile.GetFilePath();		

		InputFile.Close();
		
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

//return the current payment being parsed
LockboxPaymentInfoPtr& CANSI823Parser::GetCurrentPaymentInfo()
{
	ASSERT(m_ldiDeposit.aryLockboxPayments.size() != 0);
	return m_ldiDeposit.aryLockboxPayments.back();
}

//return the current check being parsed
LockboxCheckInfoPtr& CANSI823Parser::GetCurrentCheckInfo()
{
	ASSERT(m_ldiDeposit.aryLockboxChecks.size() != 0);
	return m_ldiDeposit.aryLockboxChecks.back();
}

CString CANSI823Parser::ParseSegment(CString& strIn)
{
	return ParseSection(strIn, m_chSegmentTerminator);
}

CString CANSI823Parser::ParseElement(CString& strIn)
{
	return ParseSection(strIn, m_chElementTerminator);
}

CString CANSI823Parser::ParseSection(CString& strIn, char chDelimiter)
{
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

//parse the monetary amount segment
void CANSI823Parser::ANSI_AMT(CString& strIn)
{
	//	AMT01 522 Amount Qualifier Code			Description: Code to qualify amount			M	 ID		1 / 3	    Must use
	//	Code:	Name:
	//	2		Batch Total
	//  3		Deposit Total
	CString strQCode = ParseElement(strIn);

	//	AMT02 782 Monetary Amount				Description : Monetary amount				M	 R		1 / 18		Must use
	CString strAmount = ParseElement(strIn);
	if (strQCode == "2") {
		//not sure if we need to do anything on the batch level
	}
	else if (strQCode == "3") {
		m_ldiDeposit.cAmount.ParseCurrency(strAmount);
	}
}


void CANSI823Parser::ANSI_BPR(CString& strIn)
{
	//this is a new payment
	LockboxCheckInfoPtr pInfo(new LockboxCheckInfo);

	//	BPR01 305 Transaction Handling Code		Description : Code designating the action to be					M	ID	1 / 2	Must use
	//											taken by all parties
	// Code:	 Name:
	// I		 Remittance Information Only
	ParseElement(strIn);

	//	BPR02 782 Monetary Amount				Description : Monetary amount									M	R	1 / 18	Must use
	CString strAmount = ParseElement(strIn);
	if (strAmount.GetLength() > 0) {
		pInfo->cAmount.ParseCurrency(strAmount);
	}

	//	BPR03 478 Credit / Debit Flag Code		Description : Code indicating whether amount is a				M	ID	1 / 1	Must use
	//											credit or debit
	//											All valid standard codes are used.
	CString strType = ParseElement(strIn);
	if (strType == "C") {
		pInfo->ptType = LockboxCheckInfo::Credit;
	}
	else if (strType == "D") {
		pInfo->ptType = LockboxCheckInfo::Debit;
	}
				  
	//	BPR04 591 Payment Method Code			Description : Code identifying the method for the				M	ID	3 / 3	Must use
	//											movement of payment instructions
	// Code:	 Name:
	// CHK		 Check
	ParseElement(strIn);

	//	BPR05 812 Payment Format Code			Description : Code identifying the payment format				O	ID	1 / 10	Used
	//											to be used
	// Code:	 Name:
	// PBC		 Commercial / Corporate Check
	ParseElement(strIn);
	
	//	BPR06 506 (DFI)ID Number Qualifier		Description : Code identifying the type of						X	ID	2 / 2	Used
	//											identification number of Depository Financial
	//											Institution(DFI)
	// Code:	 Name:
	// 01		 ABA Transit Routing Number Including Check Digits(9 digits)
	ParseElement(strIn);
				  
	//	BPR07 507 (DFI)Identification Number	Description : Depository Financial Institution(DFI)				X	AN	3 / 12	Used
	//											identification number
	CString strRoutingNum = ParseElement(strIn);
	if (strRoutingNum.GetLength() > 0) {
		pInfo->strRoutingNumber = strRoutingNum;
	}

	//	BPR08 569 Account Number Qualifier		Description : Code indicating the type of account				O	ID	1 / 3	Used				  
	// Code:	 Name:
	// DA		 Demand Deposit
	ParseElement(strIn);

	//	BPR09 508 Account Number				Description : Account number assigned							X	AN	1 / 35	Used				  
	CString strAccNum = ParseElement(strIn);
	if (strAccNum.GetLength() > 0) {
		pInfo->strAccountNumber = strAccNum;
	}

	m_ldiDeposit.aryLockboxChecks.push_back(std::move(pInfo));
}

//parse the deposit segment
void CANSI823Parser::ANSI_DEP(CString& strIn)
{
	//	DEP01 127 Reference Identification		Description : Reference information as defined for a			M	AN	1 / 30	Must use
	//											particular Transaction Set or as specified by the
	//											Reference Identification Qualifier
	ParseElement(strIn);

	//	DEP02 373 Date							Description : Date expressed as CCYYMMDD						M	DT	8 / 8	Must use
	CString strDepDate = ParseElement(strIn);
	COleDateTime dt(atoi(strDepDate.Left(4)), atoi(strDepDate.Mid(4, 2)), atoi(strDepDate.Right(2)), 0, 0, 0);
	if (dt.GetStatus() == COleDateTime::valid) {
		m_ldiDeposit.dtDepositDate = dt;
	}
				  
	//	DEP03 337 Time							Description : Time expressed in 24 - hour clock time			O	TM	4 / 8	Used
	//											as follows : HHMM, or HHMMSS, or HHMMSSD, or
	//											HHMMSSDD, where H = hours(00 - 23), M =
	//											minutes(00 - 59), S = integer seconds(00 - 59) and
	//											DD = decimal seconds; decimal seconds are
	//											expressed as follows : D = tenths(0 - 9) and DD =
	//											hundredths(00 - 99)
	ParseElement(strIn);

	//	DEP04 127 Reference Identification		Description : Reference information as defined for a			O	AN	1 / 30	Used
	//											particular Transaction Set or as specified by the
	//											Reference Identification Qualifier
	ParseElement(strIn);

	//	DEP05 506 (DFI)ID Number Qualifier		Description : Code identifying the type of						M	ID	2 / 2	Must use
	//											identification number of Depository Financial
	//											Institution(DFI)
	// Code:	Name:
	// 01		ABA Transit Routing Number Including Check Digits(9 digits)
	ParseElement(strIn);

	//	DEP06 507 (DFI)Identification Number	Description : Depository Financial Institution(DFI)				M	AN	3 / 12	Must use
	//											identification number
	ParseElement(strIn);

	//	DEP07 569 Account Number Qualifier		Description : Code indicating the type of account				X	ID	1 / 3	Used				  
	// Code:	Name:
	// DA		Demand Deposit
	ParseElement(strIn);

	//	DEP08 508 Account Number				Description : Account number assigned							X	AN	1 / 35	Used
	ParseElement(strIn);				  				  
}

void CANSI823Parser::ANSI_DTM(CString& strIn)
{
	//not sure if we need this
}

//parse the quantity segment
void CANSI823Parser::ANSI_QTY(CString& strIn)
{
	//	QTY01 673 Quantity Qualifier			Description : Code specifying the type of quantity		M	ID		2 / 2	 Must use
	//	Code:	Name:
	//	41		Number of Batches
	//	42		Number of Checks
	ParseElement(strIn);

	//	QTY02 380 Quantity						Description : Numeric value of quantity					X	 R		1 / 15	Used
	ParseElement(strIn);
}

void CANSI823Parser::ANSI_REF(CString& strIn)
{
	//	REF01 128 Reference Identification Qualifier	Description : Code qualifying the Reference		M	ID		2 / 3	Must use
	//													Identification
	// Code:	 Name:
	// BT        Batch Number
	// CK		 Check Number
	// CR		 Customer Reference Number
	CString strQualifier = ParseElement(strIn);

	//	REF02 127 Reference Identification				Description : Reference information				X	AN		1 / 30	Used
	//													as defined for a particular Transaction Set
	//													or as specified by the Reference 
	//													Identification Qualifier
	CString strRefID = ParseElement(strIn);

	if (strQualifier == "CK") {
		LockboxCheckInfoPtr& pInfo = GetCurrentCheckInfo();
		pInfo->strCheckNumber = strRefID;
	}	
}

void CANSI823Parser::ANSI_N1(CString& strIn)
{
	//	N101 98 Entity Identifier Code			Description : Code identifying an organizational		M	ID		2 / 3	Must use
	//											entity, a physical location, property or an individual 
	// Code:	 Name:
	// BK		 Bank
	// PE		 Payee
	CString strEntityID = ParseElement(strIn);

	//	N102 93 Name							Description : Free - form name							X	AN		1 / 60	Used
	CString strName = ParseElement(strIn);
	if (strEntityID == "BK") {
		m_ldiDeposit.strBankName = strName;
	}

	//	N103 66 Identification Code Qualifier	Description : Code designating the system / method		X	ID		1 / 2	Used
	//											of code structure used for Identification Code(67)
	// Code:	 Name:
	// 13		 Federal Reserve Routing Code(FRRC)
	ParseElement(strIn);

	//	N104 67 Identification Code				Description : Code identifying a party or other code	X	AN		2 / 80	Used				  
	ParseElement(strIn);
}

void CANSI823Parser::ANSI_RMR(CString& strIn)
{
	//RMR01	128	Reference Identification Qualifier	Description: IV to indicate that RMR02 contains		C	ID		2 / 3
	//												an invoice number.
	CString RefID = ParseElement(strIn);
	//if the value is not IV we dont want this data
	if(RefID != "IV") {
		return;
	}

	//get the current check that was parsed,  this will have data the payments need as well
	LockboxCheckInfoPtr& pCurrentCheckInfo = GetCurrentCheckInfo();
	//our new payment
	LockboxPaymentInfoPtr pPaymentInfo(new LockboxPaymentInfo);
	//RMR02	127	Reference Identification			Description:Invoice number reported in the			C	AN		1 / 30
	//												payment remittance information.Provided only
	//												if captured in lockbox processing.Default value is N / A
	pPaymentInfo->strPatientID = ParseElement(strIn);

	//RMR03	482	Payment Action Code					Description: Code specifying the accounts			O	ID		2 / 2
	//												receivable open item(s), if any, to be 
	//												included in the cash application.
	//												PO – Payment on account
	ParseElement(strIn);

	//RMR04	782	Monetary Amount						Description: Either gross amount of the				O	R		1/18
	//												invoice or net amount of the payment against
	//												the invoice.Determined by remittance information
	//												format.Provided only if captured in lockbox 
	//												processing.Default value is 0.00.
	ParseElement(strIn);

	// (b.spivey, April 20th, 2015) - PLID 65406 - The payment amount comes fromt he check, however we create
	// the payments here and assign them, so I just left this here. Now we know what RMR04 is used for without actually using it. 
	pPaymentInfo->cAmount = pCurrentCheckInfo->cAmount;

	//now copy the check number, routing, account numbers from the check info to the payment
	pPaymentInfo->strCheckNumber = pCurrentCheckInfo->strCheckNumber;
	pPaymentInfo->strAccountNumber = pCurrentCheckInfo->strAccountNumber;
	pPaymentInfo->strRoutingNumber = pCurrentCheckInfo->strRoutingNumber;
	//copy to deposit array
	m_ldiDeposit.aryLockboxPayments.push_back(std::move(pPaymentInfo));
}
