//PaymentechUtils.cpp
#include "stdafx.h"
#include "Paymentechutils.h"
#include "Globalutils.h"
#include "PinPadUtils.h"

// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - All string operations on a hex number (ie 0x0A) are now explicitly cast as char

// (j.gruber 2007-08-27 16:34) - PLID 15416 - Credit card processing
namespace PaymentechUtils {

	CString FormatCurrency(const COleCurrency &cy)
	{
		//When saving to Sql, ALWAYS format the American way.
		CString strReturn = cy.Format(0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));

		long nResult = strReturn.Find(".");
		if (nResult == -1) {
			strReturn += ".00";
		}
		else {
			//we found a . but need to make sure it has 2 digits after it
			CString strTemp = strReturn.Right(strReturn.GetLength() - (nResult + 1));
			if (strTemp.GetLength() > 2) {
				ASSERT(FALSE);
				//cut it off
				strReturn = strReturn.Left(nResult) + "." +  strTemp.Left(2);
			}
			else if (strTemp.GetLength() == 1) {
				strReturn = strReturn.Left(nResult)  + "." + strTemp.Left(1) + "0";
			}
			
		}
				
		return strReturn;
	}

	CString ZeroFill(long nNumberToFill, long nLengthOfField, CString strCurrentField) {

		//first get the length of the number we already have
		CString strReturn;
		strReturn.Format("%li", nNumberToFill);

		long nCurrentLength = strReturn.GetLength();

		if (nCurrentLength > nLengthOfField) {
			ThrowNxException("Error in ZeroFillNumber:Error calculating length: " + strCurrentField);
		}

		for (int i=0; i < (nLengthOfField - nCurrentLength); i++) {
			strReturn = '0' + strReturn;
		}

		//ASSERT(strReturn.GetLength() == nLengthOfField);

		return strReturn;

	}

		


	CString ZeroFill(CString strStringToFill, long nLengthOfField, CString strCurrentField) {

		//first get the length of the number we already have
		CString strReturn = strStringToFill;
		
		long nCurrentLength = strReturn.GetLength();

		if (nCurrentLength > nLengthOfField) {
			ThrowNxException("Error in ZeroFillString:Error calculating length: " + strCurrentField);
		}

		for (int i=0; i < (nLengthOfField - nCurrentLength); i++) {
			strReturn = '0' + strReturn;
		}

		//ASSERT(strReturn.GetLength() == nLengthOfField);

		return strReturn;

	}

	// (a.walling 2007-07-02 10:26) - Space fill
	CString SpaceFillString(CString strStringToFill, long nLengthOfField, CString strCurrentField) {

		//first get the length of the number we already have
		CString strReturn = strStringToFill;
		
		long nCurrentLength = strReturn.GetLength();

		if (nCurrentLength > nLengthOfField) {
			ThrowNxException("Error in SpaceFillString:Error calculating length: " + strCurrentField);
		}


		for (int i=0; i < (nLengthOfField - nCurrentLength); i++) {
			strReturn = ' ' + strReturn;
		}

		//ASSERT(strReturn.GetLength() == nLengthOfField);

		return strReturn;

	}

	// (a.walling 2007-07-02 10:25)
	CString ZeroFillFloat(double dToFill, long nLengthOfField, CString strCurrentField) {

		//first get the length of the number we already have
		CString strReturn;
		strReturn.Format("%.2g", dToFill);
		
		long nCurrentLength = strReturn.GetLength();

		if (nCurrentLength > nLengthOfField) {
			ThrowNxException("Error in ZerFillFloat:Error calculating length: " + strCurrentField);
		}

		for (int i=0; i < (nLengthOfField - nCurrentLength); i++) {
			strReturn = '0' + strReturn;
		}

		//ASSERT(strReturn.GetLength() == nLengthOfField);

		return strReturn;

	}


	CString CheckAndConvertToString(long nNumber, CString strCurrentField) {

		//These assertions should be thrown errors in Practice
		CString strReturn;

		if (nNumber < 0) {
		
			ThrowNxException("Error in CheckAndConvertToString:Field < 0 " + strCurrentField + AsString(nNumber));
		}
		
		strReturn.Format("%li", nNumber);

		if (strReturn.IsEmpty()) {
			ThrowNxException("Error in CheckAndConvertToString:Return value is empty" + strCurrentField + AsString(nNumber));
		}

		return strReturn;

	}

	// (a.walling 2007-11-07 09:34) - PLID 27998 - VS2008 - Overload for no field parameter
	CString CheckString(CString str, long nLength /*=-1*/) {
		return CheckString(str, "", nLength);
	}

	CString CheckString(CString str, CString strCurrentField, long nLength /*=-1*/) {

		if (nLength == -1) {

			if (str.IsEmpty()) {
				ThrowNxException("Error in CheckString:String is empty: " + strCurrentField);
				return str;
			}
		}
		else {

			if (str.GetLength() != nLength) {
				ThrowNxException("Error in CheckString:String length does not equal length specified: " + strCurrentField + " " + str + " " + AsString(nLength));
			}
		}

		return str;

	}


	CString FormatDateTime(COleDateTime dtDate) {

		//return mmddyyyyhhmmss
		CString strReturn;

		strReturn += ZeroFill(dtDate.GetMonth(), 2, "FormatDateTime Month");
		strReturn += ZeroFill(dtDate.GetDay(), 2, "FormatDateTime Day");
		strReturn += AsString((long)dtDate.GetYear());
		strReturn += ZeroFill(dtDate.GetHour(), 2, "FormatDateTime Hour");
		strReturn += ZeroFill(dtDate.GetMinute(), 2, "FormatDateTime Minute");
		strReturn += ZeroFill(dtDate.GetSecond(), 2, "FormatDateTime Second");
		
		return strReturn;

	}

	CString TrimString(CString str) {

		str.TrimLeft();
		str.TrimRight();

		return str;
	}



	#define TU(string) ToUpper(string)

	#define _ZFN(number, length, field) ZeroFill(number, length, field)
	#define _ZFS(string, length, field) ZeroFill(string, length, field)

	#define _CTN(number, strCurrentField) CheckAndConvertToString(number, strCurrentField)
	#define _CHS(string, field, number) CheckString(string, field, number)
	#define _CS(string, strField) CheckString(string, strField)
	#define _TS(string) TrimString(string)



	BOOL IsPaymentechInstalled() {

		Paymentech::IIPOSTransactionPtr pPayTechTrans;

		pPayTechTrans.CreateInstance(__uuidof(Paymentech::IPOSTransaction));

		if (pPayTechTrans) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}


	//this defined the first 10 Fields of the Header because they are the same in all message types
	BOOL InitTerminalMessage(CString &strMessage, TransStruct Message) {

		//These are the specifications for a HCS Message

		/***************************************A. HEADER INFORMATION*******************************************************************************************/
		//1. PIC X(1)  
		//02 in hex
		strMessage += chStartText;
			
		//2. PIC X(2)
		//we need a system indicator of 
		//since we are doing a host capture, we want L. 
		// if we were just doing a Auth. Check it'd be M.
		strMessage += "L.";

		//3. PIC X(6) 
		//a routing indication, always 'A02000'
		strMessage += _CHS(Message.strRoutingIndicator, "Routing Indicator", 6);


		//4. PIC 9(4) 
		// client number
		//0002 for our test acct.
		strMessage += _ZFN(Message.nClientNumber, 4, "Client Number");

		//5. PIC 9(12)
		//merchant number
		// 7000000006727 for our test acct.
		strMessage += _CHS(Message.strMerchantNumber, "Merchant Number", 12);

		//6. PIC 9(3)
		//Terminal Number
		// 001 for our test acct
		strMessage += _ZFN(Message.nTerminalNumber, 3, "Terminal Number");

		//7. PIC 9(1)
		//Transaction Sequence Flag
		// 1 for single transaction or last auth
		// 2 for multi transaction or not last auth
		strMessage += _CHS(_CTN(Message.nTransactionSequenceFlag, "Transaction Sequence Flag"), "Transaction Sequence Flag", 1);

		//8. PIC 9(6)
		// Sequence Number
		strMessage += _ZFN(Message.nSequenceNumber, 6, "Sequence Number");

		//9. PIC X(1)
		// Transaction Class
		// always F
		strMessage += _CS(Message.chTransactionClass, "Transaction Class");

		//10. PIC 9(2)
		// Transaction Code
		//Defined with each industry
		//since we are retail, these are the codes we can choose from
		/*  01 - Credit Sale
			02 - Authorization Only
			03 - Prior Authorization Sale
			06 - Credit Return
			07 - Cash Advance
			11 - Offline Transaction Posting Sale
			12 - Offline Transaction Posting Cash Advanced Sale
			16 - Credit Prepaid Stand Alone Bal. Inq
			17 - Prior Auth Cash Adv.
			21 - Debit Sale
			22 - Debit Sale w/Cash Back
			24 - Debit Return
			27 - Interac Debit Sale
			28 - Interac Debit Sale with Cash Back
			29 - Interac Debit Return
			35 - Check Auth Request
			41 - Transaction Void
			44 - Interac Debit Sale Void
			45 - Interac Debit Return Void
			46 - Reversal Advice
			47 - Reversal Advice - Interac
			48 - MAC Reversal - Interac
			49 - Interac Debit Current Key Request
			50 - Batch Inquiry
			51 - Batch Release
			56 - Interac Debit Current Key Request
			57 - EMV Parameter Download Request
			61 - EBT Sale - Cash Benefits (Sale, Sale with Cash Back, Sale Card Back Only)
			61 - EBT Sale - Food Stamps (Sale)
			63 - EBT Prior/Force Sale - Cash Benefits (Sale, Sale with Cash Back, Sale Card Back Only)
			63 - EBT Prior/Force Sale - Food Stamps (Sale)
			65 - EBT Return - Food Stamps Only
			66 - EBT Balance Inquiry - Cash Benefit, Food Samps
			70 - Stored Value Issuance/Add Value
			71 - Stored Value Activation
			73 - Stored Value Redemption
			74 - Stored Value Prior Issuance/Add Value
			75 - Stored Value Prior Activation
			77 - Stored Value Prior Redemption (Force)
			78 - Stored Value Void
			79 - Balance Inquiry
			82 - Stored Value Deactivation
			83 - Stored Value Reactivation
		*/
		strMessage += _ZFN(Message.tcTransactionCode, 2, "Transaction Code");

		return TRUE;
	}



	//this function is used for Getting the Current Key for a Interac supported pin pad
	//starts at the 11th field
	BOOL GetTerminalCurrentKeyRequest(CString &strMessage, TransStruct Message) {

		//11. Field Separator
		strMessage += chFieldSeparator;

		//12. PIC 9(8)
		// LRR #
		strMessage += "00000000";

		// 13. 11 Field Separators
		for (int i = 0; i < 11; i++) {
			strMessage += chFieldSeparator;
		}

		/************************************ J. Token Data  ********************************************/
		//1. PIC X(2)
		//POS Date / Time Indicator
		strMessage += "DT";
		
		//2. PIC 9(14)
		//POS Date /Time
		strMessage += FormatDateTime(COleDateTime::GetCurrentTime());

		//3. Field Separator
		strMessage += chFieldSeparator;

		//4. PIC X(2)
		//Pin pad Serial Number Indicatior
		strMessage += "PP";

		//5. PIC X(16)
		//PIN Pad serial number
		strMessage += _CS(Message.strPinPadSerialNumber, "Pin Pad Serial Number");

		//6. Field Separator
		strMessage += chFieldSeparator;

		/************************************ K. End of Packet *********************************************/

		//1. PIC X(1)
		// End of Text Indicator
		// 03 in hex
		strMessage +=  chEndText;

		//2. PIC X(1)
		// LRC
		char chLRC = 0x00;
		for (i = 1; i < strMessage.GetLength() - 5; i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		strMessage += chLRC;

		return TRUE;
	}

	//this function defines that Batch Inquiry and Batch Release Request messages
	//starting at field 11
	BOOL GetTerminalBatchMessage(CString &strMessage, TransStruct Message) {

		//11. PIC 9(6)
		//Batch Number
		//this is always going to be 000000 because the host will ONLY send back totals for the current batch
		strMessage += _ZFN(Message.nBatchNumber, 6, "Batch Number");

		//12 PIC 9(3)
		//Batch Sequence Number
		//this is also always 0's
		strMessage += _ZFN(Message.nBatchSequenceNumber, 3, "Batch Sequence Number");

		//13. PIC 9(1)
		//Batch offset
		//always 0
		strMessage += _CHS(_CTN(Message.nBatchOffset, "Batch Offset"), "Batch Offset", 1);

		//14. PIC 9(6)
		//Transaction Count
		//this field is not checked, so it can also just be all zeros
		strMessage += _ZFN(Message.nTransactionCount, 6, "Transaction Count");
		

		//15. PIC X(11)
		//Net Amount
		//this field is also not checked, and can be all zeros
		strMessage += FormatCurrency(Message.cyNetAmount);

		//16. PIC X(1)
		//Field Separator
		strMessage += chFieldSeparator;

		//17. PIC X(23)
		// System Information
		//Asked our rep, she said the documentation is outdated and this is just the release date and version number
		strMessage += _ZFS(Message.strSysInfo.Left(23), 23, "System Info");

		//18. PIC X(1)
		//Field Separator
		strMessage += chFieldSeparator;

		//19. PIC 9(8)
		//LRR #
		strMessage += _CHS(Message.strLRR, "LRR", 8);

		//20. PIC X(1)
		// End of Text Indicator
		// 03 in hex
		strMessage +=  chEndText;

		//21. PIC X(1)
		// LRC
		char chLRC = 0x00;
		for (int i = 1; i < strMessage.GetLength() - 5; i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		strMessage += chLRC;


		return TRUE;

	}

	//this is for U.S. Debit cards only, starts at section D, PIN Block
	BOOL GetTerminalDebitMessage(CString &strMessage, TransStruct Message) {

		/*******************************************D. PIN Block*********************************/		

		//1. PIC X(16)
		//DUKPT KSN
		//Key Sequence Number in the clear
		//strMessage += 



		return TRUE;
	}


	//Starting at field 11 in the header, this defines the VOID message
	BOOL GetTerminalVoidMessage(CString &strMessage, TransStruct Message) {

		//11. PIC 9(8)
		//Last Retrieval Reference Number
		strMessage += _ZFN(Message.nLastRetrievalReferenceNumber, 8, "Last Ret. Ref. Number");

		//12. PIC X(1)
		//Field Separator
		strMessage += chFieldSeparator;

		/*******************************************************G. Miscellaneous Information ******************************************/

		//1. PIC 9(8)
		//Retrieval Reference Number
		strMessage += _ZFN(Message.nRetrievalReferenceNumber, 8, "Ret. Ref. Number");

		//2. PIC X(1)
		//Field Separator
		strMessage += chFieldSeparator;

		//3. PIC 9(19)
		//Account Number
		strMessage += _CS(Message.strAccountNumber, "Account Number");


		//2. PIC X(1)
		//Field Separator
		strMessage += chFieldSeparator;

		/****************************************************** K. End of Packet ********************************************/


		//1. PIC X(1)
		// End of Text Indicator
		// 03 in hex
		strMessage +=  chEndText;

		//2. PIC X(1)
		// LRC
		char chLRC = 0x00;
		for (int i = 1; i < strMessage.GetLength() - 5; i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		strMessage += chLRC;

		return TRUE;

	}

	//this goes from section A, part 11 until the end of section C
	BOOL GetTerminalRetailDebitMessage(CString &strMessage, TransStruct Message) {

		//11. PIC 9(1)
		//PIN Capability Code
		/*  0 - Unknown
			1 - Terminal Device accepts PIN entry
			2 - Terminal Device does not accept PIN entry
			8 - PIN Pad Inoperable
			9 - PIN Verified by Terminal Device
		*/
		//for now, let's use 0
		strMessage += _CHS(_CTN(Message.nPINCapabilityMode, "PinCapabilityMode"), "PinCapabilityMode", 1);

		//12. PIC 9(2)
		// Entry Data Source
		/*  01 - Card Swipe, origin unknown
			02 - Manually Entered
			03 - Track 2
			04 - Track 1
			05 - Scanned, check reader
			06 - IVR
			12 - Unattended Terminal, manually entered
			13 - Unattended Terminal, Track 2
			14 - Unattended Terminal, Track 1
			31 - Contactless Device, Track 1 from RFID (Visa, MasterCard, Amex Only)
			32 - Contactless Device, Track 2 from RFID (Visa, MasterCard, Amex Only)
			33 - Contactless Device, Track 1 from Mag Stripe (All card Types)
			34 - Contactless Device, Track 2 from Mag Stripe (All Card Types)
			35 - Contactless Device, manually entered (All Card Types)
			36 - Chip capable Device, chip read, Track 2 (Visa, MasterCard Only)
			37 - Chip Capable Device, chip read attempted, fall back to magnetic stripe, Track 1 (Visa, MasterCard Only)
			38 - Chip Capable Device, chip read attempted, fall back to magnetic stripe, Track 2 (Visa, MasterCard Only)
			39 - Chip Capable Device, chip read attempted, magnetic read attempted, fall back to manually entered (Visa, MasterCard Only)
			40 - Chip Capable Device, RFID read(chip technology to read the card), Track 1 (Visa, MasterCard Only)
			41 - Chip Capable Device, RFID read(chip technology to read the card), Track 2 (Visa, MasterCard Only)
			42 - Chip Capable Device, RFID read(mag stripe technology to read the card), Track 1 (Visa, MasterCard Only)
			43 - Chip Capable Device, RFID read(mag stripe technology to read the card), Track 2 (Visa, MasterCard Only)
			44 - Chip Capable Device, mag stripe read, Track 1 (All Card Types)
			45 - Chip Capable Device, mag stripe read, Track 2 (All Card Types)
			46 - Chip Capable Device, manually entered (All Card Types)
		*/
		//for now, send 01
		strMessage += _ZFN(Message.edsEntryDataSource, 2, "Entry Data Source");



		/*****************************B. CARD HOLDER INFORMATION**********************************************************/

		if (Message.bSwiped) {
			//1. PIC X(76)
			//If the card is swiped, then it is the full magnetic stripe information
			//Max of 76
			//Get just track 1
			strMessage += _CS(Message.strMagneticStripe, "Magnetic Stripe");

			//2. PIC X(1)
			// 1C in Hex
			strMessage += chFieldSeparator;
		}
		else {
			//1. PIC 9(19)
			// Max length is 19, defined by card type
			//Account Number
			strMessage += _CS(Message.strAccountNumber, "Account Number");

			//2. PIC X(1)
			//1C in hex
			strMessage +=  chFieldSeparator;

			//3. PIC 9(4)
			//Expiration Date
			strMessage += _CHS(_ZFS(Message.strExpireDate, 4, "Expiration Date"), "Expiration Date", 4);

			//4. PIC X(1)
			//1c in hex
			strMessage +=  chFieldSeparator;

		}
		

		/*************************************C. Transaction Information********************************************************/

		//1. PIC X(8)
		//Transaction Amount
		// minimum is "0.01"

		//for interac, this needs to be 8 characters
		if (Message.tcTransactionCode == tcInteracSale ||
			Message.tcTransactionCode == tcInteracSaleWithCashBack ||
			Message.tcTransactionCode == tcInteracReturn ||
			Message.tcTransactionCode == tcMACReversal) {

			strMessage += _CHS(_ZFS(FormatCurrency(Message.cyTransAmount), 8, "Transaction Amount"), "Transaction Amount", 8);
		}
		else {
			strMessage += _CS(FormatCurrency(Message.cyTransAmount), "Transaction Amount");
		}

		//2. PIC X(1)
		// 1C in Hex
		strMessage +=  chFieldSeparator;

		//3. PIC 9(8)
		//Filler - LRR#
		//this is just 00000000
		strMessage += _CHS(Message.strLRR, "LRR", 8);

		//4. PIC X(1)
		//1C in Hex
		strMessage += chFieldSeparator;

		return TRUE;


	}

	BOOL GetTerminalInteracMessage(CString &strMessage, TransStruct Message) {

		/********************************G. Miscellaneous Information *****************************************/

		//1. PIC X(16)
		//PIN Block
		strMessage += _CHS(Message.strPinBlock, "PinBlock", 16);

		//2. FS
		strMessage += chFieldSeparator;

		if (Message.tcTransactionCode == tcInteracSaleWithCashBack) {

			//3. PIC X(9)  optional
			//cash back amount
			strMessage += _CS(FormatCurrency(Message.cyCashBackAmount), "Cash Back Amount");

		}

		//8 Field separators
		for ( int i = 0; i < 8; i++) {
			strMessage += chFieldSeparator;
		}

		/*************************************J. Token Data *************************************************/

		//1. PIC X(2)
		//Interac Surcharge Amount Indicator
		strMessage += "IS";

		//2. PIC X(8)
		//Interac Surcharge Amount
		//it says that this field is setup at the POS, so I guess we know it
		strMessage += _CS(_ZFS(FormatCurrency(Message.cyInteracSurchargeAmount), 8, "Interac Surcharge Amount"), "Interac Surcharge Amount");

		//3. Field Separator
		strMessage += chFieldSeparator;

		//4. PIC X(2)
		// POS Date/Time Indicator
		strMessage += "DT";
		

		//5. PIC 9(14)
		// POS Date/Time mmddyyyhhmmss
		strMessage += FormatDateTime(COleDateTime::GetCurrentTime());

		//6. Field Separator
		strMessage += chFieldSeparator;

		//7. PIC X(2)
		// Interac Specific Data
		strMessage += "IN";

		//8. PAN Number
		//Account number, left justified, speace filled
		CString strPanNumber = Message.strAccountNumber;
		while (strPanNumber.GetLength() < 19) {
			strPanNumber += ' ';
		}
		strMessage += _CHS(strPanNumber, "PanNumber", 19);

		//9. Account Type
		strMessage += AsString(Message.nAccountType);

		//10. Field Separator
		strMessage += chFieldSeparator;

		//11. PIC X(2) 
		// Pin Pad Serial Number Indicator
		strMessage += "PP";

		//12. PIC X(16)
		//Pin Pad Serial Number, variable length from 9 to 16
		strMessage += _TS(Message.strPinPadSerialNumber);

		//13. Field Separator
		strMessage += chFieldSeparator;

		//14. PIC X(2)
		//Encrypted Key Index Indicator
		strMessage += "EK";

		//15. PIC 9(1)
		//Encrypted Key Index
		//TODO: find out what this is
		strMessage += "1";

		//16. Field Separator
		strMessage += chFieldSeparator;


		//17. PIC X(2)
		//MAC Indicator
		strMessage += "MC";

		//18. PIC X(8)
		//MAC Value
		strMessage += _CHS(Message.strMACBlock, "MACBlock", 8);

		//19. Field separator
		strMessage += chFieldSeparator;

		/*************************************K. End of Packet *******************************************/

		//1. PIC X(1)
		// End of Text Indicator
		// 03 in hex
		strMessage +=  chEndText;

		//2. PIC X(1)
		// LRC
		char chLRC = 0x00;
		for (i = 1; i < strMessage.GetLength() - 5; i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		strMessage += chLRC;



		return TRUE;

	}


	//this defines the Retail Messages (Sale, Return, Authorization Only, Force Sale) 
	//starting at Section D , Market Specific Data
	BOOL GetTerminalRetailMessage(CString &strMessage, TransStruct Message) {
	

		/**************************************D. Market Specific Data ***********************************************************/


		//1. PIC X(35)
		// This is only used in the Petroleum Industry, so we are skipping it

		//2. PIC X(1)
		//1C in hex
		strMessage +=  chFieldSeparator;

		/****************************************E. Interchange Compliance ***************************************************/

		//1. PIC X(40)
		// TODO: figure out exactly what this is and how to get the auth numbers
		//for now we are leaving it empty


		//2. PIC X(1)
		// 1C in Hex
		strMessage +=  chFieldSeparator;

		/***************************************** F. Industry Specific Information ***********************************************/

		//1. PIC 9 (3)
		//Industry Code
		//for us, retail, its 004
		strMessage += _ZFN(Message.nIndustryCode, 3, "Industry Code");

		//2. PIC 9 (6)
		// Invoice Number
		//needs to be zero fill, right justify
		strMessage += _ZFN(Message.nInvoiceNumber, 6, "Invoice Number");

		//3. PIC 9(20);
		//Item Code
		// array of five 4 byte item codes zero fill if unused
		strMessage += _CHS(Message.strItemCode, "Item Code", 20);

		
		/************************************************G. Micellaneous Information ************************************************/

		if (Message.tcTransactionCode == tcSale || Message.tcTransactionCode == tcAuthOnly || Message.tcTransactionCode == tcReturn) {
			//Sales, Authorizations, Returns
			//1. PIC X(1)
			// 1C in hex
			strMessage +=  chFieldSeparator;


			//2. PIC X(6)
			//Authorization Code
			//Required for all Prior Sale Offline transactions
			// leaving blank for now

			//3. PIC X(1)
			//1C in hex
			strMessage +=  chFieldSeparator;

			//4. PIC 9(8)
			// Retrieval Reference Number
			// Used to VOID a transaction
			//leaving blank for now

			//5. PIC X(1)
			//1C in hex
			strMessage +=  chFieldSeparator;
		}
		else if (Message.tcTransactionCode == tcForceSale) {
			//Force sale

			//1. PIC X(1)
			// Field Separator
			// 1C in hex
			strMessage +=  chFieldSeparator;

			//2. PIC X (6)
			// Authorization Code
			strMessage += _CHS(Message.strAuthorizationCode, "Authorization Code", 6);

			//3. PIC X(1)
			// Field Separator
			// 1C in hex
			strMessage +=  chFieldSeparator;

			//4. PIC X(1)
			// Field Separator
			// 1C in hex
			strMessage +=  chFieldSeparator;

			//5. PIC X(1)
			// Field Separator
			// 1C in hex
			strMessage +=  chFieldSeparator;

			//6. PIC X(1)
			// Field Separator
			// 1C in hex
			strMessage +=  chFieldSeparator;
		}


		/**************************************************H. Address Verification Information ***************************************/

		if (Message.tcTransactionCode == tcSale || Message.tcTransactionCode == tcAuthOnly || Message.tcTransactionCode == tcReturn) {

			//this just needs to be filled in if a card is not swiped
			if (Message.bSwiped) {

				//1. PIC X(20)
				//Street Address
				

				//2. PIC X(20)
				//Extended Street Address
				// Visa Only

				//3. PIC X(1)
				// 1C in Hex
				strMessage +=  chFieldSeparator;

				//4. PIC X(9)
				// Cardholder Zip Code

				//5. PIC X(1)
				//1C in Hex
				strMessage +=  chFieldSeparator;
			}
			else {

				//1. PIC X(20)
				//Street Address
				// (j.gruber 2007-11-01 08:59) - PLID 15416 - I took the check string out because if 
				// they cancel the address verification dialog, then the string is blank, as it should be
				strMessage += Message.strCardHolderStreet.Left(20);
				

				//2. PIC X(20)
				//Extended Street Address
				// Visa Only
				strMessage += Message.strExtendedStreetInfo.Left(20);
				
				//3. PIC X(1)
				// 1C in Hex
				strMessage +=  chFieldSeparator;

				//4. PIC X(9)
				// Cardholder Zip Code
				strMessage += Message.strCardHolderZip;

				//5. PIC X(1)
				//1C in Hex
				strMessage +=  chFieldSeparator;
			}


		}
		/**************************************************** I. Purchasing Card Information ***********************************/

		//TODO: look at Processing & interchange to see what we need to do here
		//1. PIC X(44)
		// Purchasing Card
		//TODO: figure out what to do with this

		//2. PIC X(1)
		// 1C in hex
		strMessage +=  chFieldSeparator;

		/****************************************************** J. Token Data *************************************************/

		if (Message.bSwiped) {

			//we need an extra field separator
			strMessage +=  chFieldSeparator;
		}

		//TODO: figure out if we need any other tokens except TA

		// (a.walling 2007-06-21 13:45)

		//Token Allowed Indication - TA
		//1. PIC X(2)
		//Token Indication
		strMessage += "TA";

		//2. PIC X(1)
		//Token Data Request
		strMessage += "Y";

		//3. PIC X(1)
		// 1C in Hex
		strMessage += chFieldSeparator;

		if (Message.strCustomerDefinedData.GetLength()) {
			// CD - Customer Defined Data
			// this is stored on the paymentech system, and will be seen by the user in the back-end reports.
			strMessage += "CD";

			ASSERT(Message.strCustomerDefinedData.GetLength() <= 30);
			strMessage += _ZFS("EXAMPLE DATA", 30, "Customer Defined Data"); // 30 chars max.

			strMessage += chFieldSeparator;
		}

		// CV - CVD data
		/*
		T1 CVD Indicator Pic X (2) CV = CVV2/CVC2/CID/CVD2 Indicator
		T2 Presence Indicator Pic X (2) PI = Presence Indicator
		T3 Value of Presence Indicator Pic 9 (1) Flag indicating the status of the cardholder verification data.
			(See Appendix A for values) If “0, 2, or 9”, skip next fields
			and proceed to this section’s Field Separator [FS].
		T4 Verification Indicator Pic X (2) VF = Verification Indicator
		T5 VF Field Length Pic 9 (1) Length of data elements to follow.
		T6 Verification Data Pic 9 (4) Variable length field containing the data printed on the card.
		T7 FS Pic X (1) 1Ch – (Required if Token data is sent.)*/

		// Manually entered transactions only
		if (!Message.bSwiped) {

			strMessage += "CV";

			strMessage += "PI";
	
			/*
			0 Value deliberately bypassed or not provided. Do not send CVV2/CVC2/CID data
				to host if not provided or bypassed. Field value is information only.
			1 Value provided (This is the default value).
			2 Value on card but illegible.
			9 Cardholder	 states data not available.
			*/

			strMessage += _CTN(Message.piPresenceIndicator, "Presence Indicator");

			if (Message.piPresenceIndicator != piBypassed && Message.piPresenceIndicator != piIllegible && Message.piPresenceIndicator != piUnavailable) {
				strMessage += "VF";

				ASSERT(Message.strCVD.GetLength() < 10);
				strMessage += _ZFN(Message.strCVD.GetLength(), 1, "CVD");

				strMessage += Message.strCVD;
			}

			strMessage += chFieldSeparator;
		}

		// DU - Duplicate transaction checking
		// I'm not sure what we should do about this one, but it may be useful
		//the class B var says we have to either send this token or support a reversal advice message, so we are going to do this
		strMessage += "DU";

		//we don't allow duplicate transactions, so this will always be 00
		strMessage += "00";

		strMessage += chFieldSeparator;

		// DC - Dynamic Currency Conversion
		// Probably won't be implementing this.

		/*** Interac Tokens ***/
		
		BOOL bInterac = FALSE;
		if (bInterac) {

			// IS - Interac Surcharge
			/*
			1 Interac Surcharge Amount Indicator Pic X (2) IS = Interac Surcharge Amount Indicator
			2 Interac Surcharge Amount Pic X (8) Fixed length. This amount is defined and set up at the
				POS. Amount must include a decimal.
			3 FS Pic X (1) 1Ch – Required field if IS token present
			*/

			strMessage += "IS";

			double dAmount = 0.0;
			strMessage += ZeroFillFloat(dAmount, 8, "Amount");

			strMessage += chFieldSeparator;
		
			// DT - Date Time indicator
			/*
			4 POS Date / Time Indicator Pic X (2) ã DT = POS Date / Time Indicator
			5 POS Date / Time Pic 9 (14) Format = mmddyyyyhhmmss
			6 FS Pic X (1) ã 1Ch
			*/
			
			strMessage += "DT";
			COleDateTime dt = COleDateTime::GetCurrentTime();

			strMessage += dt.Format("%m%d%Y%H%M%S");

			strMessage += chFieldSeparator;

			// IN - Interac Specfic Data
			/*
			7 Interac Specific Data Pic X (2)  IN = Interac Required Data Fields
			8 PAN Pic X (19)  Account Number used for the transaction. Left
				Justified / Space Filled.
			9 Account Type Pic 9 (1)  Account type selected by cardholder:
			1 = Chequing
			2 = Savings
			10 FS Pic X (1) 1Ch
			*/

			long nAccountType = 1; // Checking

			strMessage += "IN";

			strMessage += SpaceFillString(Message.strAccountNumber, 19, "Account Number");
			strMessage += _CTN(nAccountType, "Account Type");

			strMessage += chFieldSeparator;

			// PP - Pin Pad Serial number
			/*
			11 PIN Pad Serial Number Indicator Pic X (2)  PP = PIN Pad Serial Number Indicator
			12 PIN Pad Serial Number Pic X (16)  PIN Pad serial number. Variable . minimum length of
				9, maximum length of 16.
			13 FS Pic X (1)  1Ch
			*/

			strMessage += "PP";

			CString strSerialNumber = "0123456789";

			ASSERT(strSerialNumber.GetLength() >= 9 || strSerialNumber.GetLength() <= 16);
			strMessage += strSerialNumber;
			strMessage += chFieldSeparator;

			// EK - Encrypted Key Indicator
			/*
			14 Encrypted Key Index Indicator Pic X (2)  EK = Encrypted Key Indicator
			15 Encrypted Key Index Pic 9 (1)  Index indicating keys in use by PIN Pad. Present in
				requests and responses. Normal values from 1 through
				9. Zero (0) is used by the terminal when no encryption
				keys have been established. Transaction requests
				should never be sent with a value of zero (0). MAC
				reversals must be sent with the key index from the
				original transaction to insure that the PIN block can be
				properly translated by the host.
			16 FS Pic X (1)  1Ch
			*/

			long nKeyIndex = 1;
			strMessage += "EK";

			ASSERT(nKeyIndex != 0);
			strMessage += _CTN(nKeyIndex, "Key Index");

			strMessage += chFieldSeparator;

			// MC - MAC Indicator
			/*
			17 MAC Indicator Pic X (2)  MC = MAC Indicator
			18 MAC Value Pic X (8)  MAC Value . Present in requests and responses to
				confirm that the key data elements of the transaction
				have not been tampered with. This value is generated
				based on the appropriate structure of data for the
				request or response. These structures are defined at
				the beginning of this chapter. MAC reversals can
				contain the MAC from the original request. It will
				NOT be validated by the host.
			19 FS Pic X (1)  1Ch
			*/

			strMessage += "MC";

			// no idea what goes here exactly
//#pragma message("Figure out the MAC")
			strMessage += "MACMACMC";

			strMessage += chFieldSeparator;
		}


		/****************************************************** K. End of Packet ********************************************/


		//1. PIC X(1)
		// End of Text Indicator
		// 03 in hex
		strMessage +=  chEndText;

		//2. PIC X(1)
		// LRC
		char chLRC = 0x00;
		for (int i = 1; i < strMessage.GetLength() - 5; i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		strMessage += chLRC;

		return TRUE;
		
	}

	void ClearMessage(TransStruct &Message) {

		Message.bCashBackAllowed = FALSE;
		Message.bShowSurcharge = FALSE;
		Message.bSurchargeAccepted = FALSE;
		Message.bSwiped = FALSE;
		Message.bTipAllowed = FALSE;
		Message.chTaxFlag = '0';
		Message.chTransactionClass = '0';
		Message.ctCardType = ctNone;
		Message.cyCashBackAmount = COleCurrency(0,0);
		Message.cyInteracSurchargeAmount = COleCurrency(0,0);
		Message.cyNetAmount = COleCurrency(0,0);
		Message.cySalesTaxAmt = COleCurrency(0,0);
		Message.cySurchargeAmount = COleCurrency(0,0);
		Message.cyTipAmount = COleCurrency(0,0);
		Message.cyTransAmount = COleCurrency(0,0);
		Message.edsEntryDataSource = edsNone;
		Message.iTransID = itiNone;
		Message.lcLangCode = lcEnglish;
		Message.nAccountType = -1;
		Message.nBatchNumber = -1;
		Message.nBatchOffset = -1;
		Message.nBatchSequenceNumber = -1;
		Message.nClientNumber = -1;
		Message.nIndustryCode = -1;
		Message.nInvoiceNumber = -1;
		Message.nLastRetrievalReferenceNumber = -1;
		Message.nPINCapabilityMode = -1;
		Message.nRetrievalReferenceNumber = -1;
		Message.nSequenceNumber = -1;
		Message.nTerminalNumber = -1;
		Message.nTransactionCount = -1;
		Message.nTransactionSequenceFlag = -1;
		Message.piPresenceIndicator = piNone;
		Message.strAccountNumber = "";
		Message.strAuthorizationCode = "";
		Message.strCardHolderStreet = "";
		Message.strCardHolderZip = "";
		Message.strCoreLibraryVersion = "";
		Message.strCustomerDefinedData = "";
		Message.strCustomerReferenceNumber = "";
		Message.strCVD = "";
		Message.strDestinationZip = "";
		Message.strExpireDate = "";
		Message.strExtendedStreetInfo = "";
		Message.strItemCode = "";
		Message.strLRR = "";
		Message.strMACBlock = "";
		Message.strMagneticStripe = "";
		Message.strMerchantNumber = "";
		Message.strPinBlock = "";
		Message.strPinPadSerialNumber = "";
		Message.strRAMVersion = "";
		Message.strROMVersion = "";
		Message.strRoutingIndicator = "";
		Message.strSecurityLibraryVersion = "";
		Message.strSerialApplicationName  = "";
		Message.strSysInfo = "";
		Message.strSystemIndicator = "";
		Message.strTokenData = "";
		Message.strTokenIndicator = "";
		Message.tcTransactionCode = tcNone;

	}



	void InitTransaction(Paymentech::IIPOSTransactionPtr &pTrans, TransStruct &Message) {

		try {

			//pTrans->MerchantID = "700000006727";
			pTrans->MerchantID = _bstr_t(GetRemotePropertyText("CCProcessingMerchantNumber", "", 0, "<None>", TRUE));
			pTrans->TerminalID = _bstr_t(GetPropertyText("CCProcessingTerminalNumber", "", 0, TRUE));
			pTrans->Username = _bstr_t(GetRemotePropertyText("CCProcessingUsername", "", 0, "<None>", TRUE));
			pTrans->Password = _bstr_t(GetRemotePropertyText("CCProcessingPassword", "", 0, "<None>", TRUE));
			pTrans->TransactionType = Paymentech::TransTypeStateless;

			//Fill in what we know will always be true
			Message.strSystemIndicator = "L.";
			Message.strRoutingIndicator = "A02000";
			Message.chTransactionClass = 'F';
			Message.strLRR = "00000000";
			Message.nIndustryCode = 4;
			Message.strItemCode = "00000000000000000000";
			
			//Messaging Info
			Message.nClientNumber = atoi(GetRemotePropertyText("CCProcessingClientNumber", "", 0, "<None>", TRUE));
			//Message.strMerchantNumber = "700000206544";
			Message.strMerchantNumber = GetRemotePropertyText("CCProcessingMerchantNumber", "", 0, "<None>", TRUE);
			Message.nTerminalNumber = atoi(GetPropertyText("CCProcessingTerminalNumber", "", 0, TRUE));

			//since we are initing, our transaction number will always be 1
			Message.nSequenceNumber = 1;

			//this will be a device setting that we can pull
			if (GetPropertyInt("CCProcessingUsingPinPad", 0, 0, TRUE) == 1) {
				Message.nPINCapabilityMode = 1;
			}
			else {
				Message.nPINCapabilityMode = 2;
			}
				

		}NxCatchAll("Error in InitTransaction");

	}



	//this processes interac responses starting with the 9th field
	BOOL ProcessInteracResponse(CString &strResponse, ResponseStruct &Resp) {

		// 9. PIC X(2)
		// Card Type
		Resp.strCardType = strResponse.Left(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//10. Field separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//11. PIC X(8)
		//Trace Number

		//just search until the next FS
		long nResult = strResponse.Find(chFieldSeparator);
		Resp.strTraceNumber = strResponse.Left(nResult);

		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//12. Field Separator
		//strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//13. PIC 9(2)
		// Authorizing Network ID
		Resp.nAuthorizingNetworkID = atoi(strResponse.Left(2));

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//14. Authorizing Source
		Resp.chAuthSource = strResponse.GetAt(0);

		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//15. Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//16. PIC X(16)
		//Additional Data - to be ignored -- not required
		nResult = strResponse.Find(chFieldSeparator);
		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//6 more field separators
		strResponse = strResponse.Right(strResponse.GetLength() - 6);

		return TRUE;


	}


	BOOL ProcessCurrentKeyResponse(CString &strResponse, ResponseStruct &Resp) {

		//1. Start of Text Indicator
		// make sure the first character is the start of text character (02 in hex)
		char ch = strResponse.GetAt(0);
		if (ch != 0x02) {
			ASSERT(FALSE);
			AfxMessageBox("First character was not start of text character");
			return FALSE;
		}

		//2. PIC X(1) 
		// Action Code, either A for Approved, or E for Error
		if (strResponse.GetAt(1) == 'A') {
			Resp.bApproved = TRUE;
		}
		else if (strResponse.GetAt(1) == 'E') {
			Resp.bApproved = FALSE;
		}
		else {
			ASSERT(FALSE);
		}

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//3. PIC 9(6)
		//Batch Number
		// The batch number is made up of 2 parts, the first 3 digits are the julian date, the last 3 are the batch releases
		Resp.strBatchNumber = strResponse.Left(6);
		
		strResponse = strResponse.Right(strResponse.GetLength() - 6);				
		
		
		//4. PIC 9(8)
		// Retrieval Reference Number
		Resp.nRetreivalReferenceNumber = atoi(strResponse.Left(8));
		strResponse = strResponse.Right(strResponse.GetLength() - 8);

		//5. PIC 9(6)
		//Sequence Number
		Resp.nSequenceNumber = atoi(strResponse.Left(6));
		strResponse = strResponse.Right(strResponse.GetLength() - 6);

		//6. PIC X(32)
		//Response Message
		long nResult = strResponse.Find(chFieldSeparator);
		ASSERT(nResult != -1);
		Resp.strResponseMessage = strResponse.Left(nResult);

		//this takes off the field separator also
		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//now we have 9 for field separators
		strResponse = strResponse.Right(strResponse.GetLength() - 9);
		
		return TRUE;
	}
 
	BOOL ProcessInteracTokenData(CString &strResponse, ResponseStruct &Resp) {

		/************************************ J. Token Data *************************************************/
		//we can parse this here because it is all required
		
		//1. Host Date/Time Indicator
		CString strTokenIndicator, strTokenData;
		strTokenIndicator = strResponse.Left(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//2. Host Date Date/Time
		strTokenData = strResponse.Left(14);

		//add this to the map
		Resp.mapTokens.SetAt(strTokenIndicator, strTokenData);

		strResponse = strResponse.Right(strResponse.GetLength() - 14);

		//3. Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//4. PIC X(2)
		//Current Interac Key Indicator
		strTokenIndicator = strResponse.Left(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//5. PIC X(16), PIC X(16), PIC 9(1)
		// we are going to store this as one item
		long nResult = strResponse.Find(chFieldSeparator);
		strTokenData = strResponse.Left(nResult);

		Resp.mapTokens.SetAt(strTokenIndicator, strTokenData);

		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//9-11.  Encrypted Key Index Token
		strTokenIndicator = strResponse.Left(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		nResult = strResponse.Find(chFieldSeparator);
		strTokenData = strResponse.Left(nResult);

		Resp.mapTokens.SetAt(strTokenIndicator, strTokenData);

		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//12 - 14 MAC Indicator
		strTokenIndicator = strResponse.Left(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		nResult = strResponse.Find(chFieldSeparator);
		strTokenData = strResponse.Left(nResult);

		Resp.mapTokens.SetAt(strTokenIndicator, strTokenData);

		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		// (a.walling 2007-07-27 08:49) - Parse any remaining tokens
		long nNextFS = -1;
		
		nNextFS = strResponse.Find(chFieldSeparator, nNextFS == -1 ? 0 : nNextFS);
		
		while (nNextFS != -1) {
			CString strToken = strResponse.Left(nNextFS);
			strResponse = strResponse.Right(strResponse.GetLength() - nNextFS);

			strResponse.TrimLeft(chFieldSeparator);

			// (a.walling 2007-06-21 13:46)
			ParseToken(strToken, Resp);

//#pragma message("Does the final token have a <FS> after it, or just an <ETX>?")
			nNextFS = strResponse.Find(chFieldSeparator, nNextFS == -1 ? 0 : nNextFS);
		}

		// 15. End of Text and LRX are left
		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		ASSERT(strResponse.IsEmpty());

		ProcessTokens(Resp);

		return TRUE;
		
	}
	

	//This processes up to the 8th field for a response message
	BOOL ProcessResponseHeader(CString &strResponse, ResponseStruct &Resp) {

		//1. Start of Text Indicator
		// make sure the first character is the start of text character (02 in hex)
		char ch = strResponse.GetAt(0);
		if (ch != 0x02) {
			ASSERT(FALSE);
			AfxMessageBox("First character was not start of text character");
			return FALSE;
		}

		//2. PIC X(1) 
		// Action Code, either A for Approved, or E for Error
		if (strResponse.GetAt(1) == 'A') {
			Resp.bApproved = TRUE;
		}
		else if (strResponse.GetAt(1) == 'E') {
			Resp.bApproved = FALSE;
		}
		else {
			ASSERT(FALSE);
		}

		//3. PIC X (1)
		//Address Verification Response Code
		//TODO: this can be different depending on the card
		Resp.chAddressVerificationCode = strResponse.GetAt(2);

		strResponse = strResponse.Right(strResponse.GetLength() - 3);

		//4. PIC X(6)
		//Authorization Code
		if (Resp.bApproved) {
			Resp.strAuthCode = strResponse.Left(6);
		}
		else {
			Resp.strErrorCode = strResponse.Left(6);
		}

		strResponse = strResponse.Right(strResponse.GetLength() - 6);

		//5. PIC 9(6)
		//Batch Number
		// The batch number is made up of 2 parts, the first 3 digits are the julian date, the last 3 are the batch releases
		Resp.strBatchNumber = strResponse.Left(6);
		

		strResponse = strResponse.Right(strResponse.GetLength() - 6);

		//6. PIC 9(8)
		//Retrieval Reference Number
		Resp.nRetreivalReferenceNumber = atoi(strResponse.Left(8));
		
		strResponse = strResponse.Right(strResponse.GetLength() - 8);

		//7.PIC 9(6)
		//Sequence Number
		Resp.nSequenceNumber = atoi(strResponse.Left(6));
		strResponse = strResponse.Right(strResponse.GetLength() - 6);

		//8. PIC X(32)
		//Response Message
		Resp.strResponseMessage = strResponse.Left(32);
		strResponse = strResponse.Right(strResponse.GetLength() - 32);

		

		return TRUE;

	}

	BOOL ProcessBatchResponse(CString &strResponse, ResponseStruct &Resp) {

		BOOL bRemovedEndText = FALSE;

		//9 PIC X(1)
		//Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//The next 2 fields are optional, but it would be very difficult if not impossible to parse
		// without them, so I'm goin to assume they are there
		
		//search up to the next field separator which will be after the 15th field
		CString strTemp = (char)0x1C;
		long nResult = strResponse.Find(strTemp);
		ASSERT(nResult >= 0);
		strTemp = strResponse.Left(nResult);

	//	ASSERT(strTemp.GetLength() >= 33);

		//check the length to see what we are dealing with
		//if (strTemp.GetLength() == 56) {
			//they included both fields 10 and 11

			//10. PIC X(1)
			//Download Flag
			Resp.chDownloadFlag = strResponse.GetAt(0);
			strResponse = strResponse.Right(strResponse.GetLength() - 1);

			//11. PIC X(1)
			//Multi Message Flag
			Resp.chMultiMessageFlag = strResponse.GetAt(0);
			strResponse = strResponse.Right(strResponse.GetLength() - 1);

			//12. PIC 9(10)
			//Batch Open Date/Time
			//MMDDYYHHMM format
			strTemp = strResponse.Left(10);
			long nYear, nMonth, nDay, nHour, nMinute;
			nYear = atoi(strTemp.Mid(4,2)) + 2000;
			nMonth = atoi(strTemp.Left(2));
			nDay = atoi(strTemp.Mid(2,2));
			nHour = atoi(strTemp.Mid(6,2));
			nMinute = atoi(strTemp.Right(2));

			Resp.dtBatchOpen.SetDateTime(nYear, nMonth, nDay, nHour, nMinute, 0);
			strResponse = strResponse.Right(strResponse.GetLength() - 10);

			//13. PIC 9(10)
			//Batch Close Date/Time
			//MMDDYYHHMM format
			strTemp = strResponse.Left(10);
			nYear = atoi(strTemp.Mid(4,2)) + 2000;
			nMonth = atoi(strTemp.Left(2));
			nDay = atoi(strTemp.Mid(2,2));
			nHour = atoi(strTemp.Mid(6,2));
			nMinute = atoi(strTemp.Right(2));

			Resp.dtBatchClose.SetDateTime(nYear, nMonth, nDay, nHour, nMinute, 0);
			strResponse = strResponse.Right(strResponse.GetLength() - 10);

			//14. PIC 9(6)
			//Batch Transaction Count
			Resp.nBatchTransactionCount = atoi(strResponse.Left(6));
			strResponse = strResponse.Right(strResponse.GetLength() - 6);

			//15. PIC X(11)
			//Batch Net Amount
			strTemp = (char)0x1C;
			nResult = strResponse.Find(strTemp);
			Resp.cyBatchNetAmount.ParseCurrency(strResponse.Left(nResult));
			strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));
			
		/*}else if (strTemp.GetLength() == 55) {
			//the included field is either 10 or 11, but we really can't tell which one
			ASSERT(FALSE);
		}else if (strTemp.GetLength() == 54) {
			
			//12. PIC 9(10)
			//Batch Open Date/Time
			//MMDDYYHHMM format
			strTemp = strResponse.Left(10);
			long nYear, nMonth, nDay, nHour, nMinute;
			nYear = atoi(strTemp.Mid(4,2));
			nMonth = atoi(strTemp.Left(2));
			nDay = atoi(strTemp.Mid(2,2));
			nHour = atoi(strTemp.Mid(6,2));
			nMinute = atoi(strTemp.Right(2));

			Resp.dtBatchOpen.SetDateTime(nYear, nMonth, nDay, nHour, nMinute, 0);
			strResponse = strResponse.Right(strResponse.GetLength() - 10);

			//13. PIC 9(10)
			//Batch Close Date/Time
			//MMDDYYHHMM format
			strTemp = strResponse.Left(10);
			nYear = atoi(strTemp.Mid(4,2));
			nMonth = atoi(strTemp.Left(2));
			nDay = atoi(strTemp.Mid(2,2));
			nHour = atoi(strTemp.Mid(6,2));
			nMinute = atoi(strTemp.Right(2));

			Resp.dtBatchClose.SetDateTime(nYear, nMonth, nDay, nHour, nMinute, 0);
			strResponse = strResponse.Right(strResponse.GetLength() - 10);

			//14. PIC 9(6)
			//Batch Transaction Count
			Resp.nBatchTransactionCount = atoi(strResponse.Left(6));
			strResponse = strResponse.Right(strResponse.GetLength() - 6);

			//15. PIC X(11)
			//Batch Net Amount
			strTemp = 0x1C;
			nResult = strResponse.Find(strTemp);
			Resp.cyBatchNetAmount.ParseCurrency(strResponse.Left(nResult));
			strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));
		}*/
		
		//16. PIC X(1)
		//Field Separator, already taken care of

		//17. PIC X(16)
		//Additional Data - Variable
		//They say to ignore this
		strTemp = (char)0x1C;
		nResult = strResponse.Find(strTemp);
		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//18. PIC X(1)
		//Field Separator
		//already taken care of

		//see if these are even there
		if (Resp.nBatchTransactionCount > 0) {

			//19. PIC X(3
			//Payment Type #1
			Resp.strPayType1 = strResponse.Left(3);
			strResponse = strResponse.Right(strResponse.GetLength() - 3);

			//20. PIC 9(6)
			//Transaction Count #1
			Resp.nNumTransactionsPayType1 = atoi(strResponse.Left(6));
			strResponse = strResponse.Right(strResponse.GetLength() - 6);

			//21. PIC X(6)
			//Net Amount Pay Type #1
			strTemp = (char)0x1C;
			nResult = strResponse.Find(strTemp);
			if (nResult == -1 ) {
				
				//we must be at the end
				strTemp = (char)0x03;
				nResult = strResponse.Find(strTemp);
				bRemovedEndText = TRUE;

				//initialize all our other type variables
				Resp.nNumTransactionsPayType2 = 0;
				Resp.nNumTransactionsPayType3 = 0;
				Resp.nNumTransactionsPayType4 = 0;

				Resp.cyNetAmountPayType2 = COleCurrency(0,0);
				Resp.cyNetAmountPayType3 = COleCurrency(0,0);
				Resp.cyNetAmountPayType4 = COleCurrency(0,0);

				Resp.strPayType2 = "";
				Resp.strPayType3 = "";
				Resp.strPayType4 = "";

				if (nResult == -1) {
					ASSERT(FALSE);
				}
			}
			Resp.cyNetAmountPayType1.ParseCurrency(strResponse.Left(nResult));
			strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

			//22. PIC X(1)
			//Field Separator, already taken care of

			if (!bRemovedEndText) {
			
				//23. PIC X(3)
				//Payment Type #2
				Resp.strPayType2 = strResponse.Left(3);
				strResponse = strResponse.Right(strResponse.GetLength() - 3);

				//24. PIC 9(6)
				//Transaction Count #2
				Resp.nNumTransactionsPayType2 = atoi(strResponse.Left(6));
				strResponse = strResponse.Right(strResponse.GetLength() - 6);

				//25. PIC X(6)
				//Net Amount Pay Type #2
				strTemp = (char)0x1C;
				nResult = strResponse.Find(strTemp);
				if (nResult == -1 ) {
					
					//we must be at the end
					strTemp = (char)0x03;
					nResult = strResponse.Find(strTemp);
					bRemovedEndText = TRUE;

					//initialize all our other type variables
					Resp.nNumTransactionsPayType3 = 0;
					Resp.nNumTransactionsPayType4 = 0;

					Resp.cyNetAmountPayType3 = COleCurrency(0,0);
					Resp.cyNetAmountPayType4 = COleCurrency(0,0);

					Resp.strPayType3 = "";
					Resp.strPayType4 = "";

					if (nResult == -1) {
						ASSERT(FALSE);
					}
				}
				Resp.cyNetAmountPayType2.ParseCurrency(strResponse.Left(nResult));
				strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

				//26. PIC X(1)
				//Field Separator, already taken care of
			}

			if (!bRemovedEndText) {

				//27. PIC X(3
				//Payment Type #3
				Resp.strPayType3 = strResponse.Left(3);
				strResponse = strResponse.Right(strResponse.GetLength() - 3);

				//28. PIC 9(6)
				//Transaction Count #3
				Resp.nNumTransactionsPayType3 = atoi(strResponse.Left(6));
				strResponse = strResponse.Right(strResponse.GetLength() - 6);

				//29. PIC X(6)
				//Net Amount Pay Type #3
				strTemp = (char)0x1C;
				nResult = strResponse.Find(strTemp);
				if (nResult == -1 ) {
					
					//we must be at the end
					strTemp = (char)0x03;
					nResult = strResponse.Find(strTemp);
					bRemovedEndText = TRUE;

					//initialize the pay4 type
					Resp.nNumTransactionsPayType4 = 0;

					Resp.cyNetAmountPayType4 = COleCurrency(0,0);

					Resp.strPayType4 = "";

					if (nResult == -1) {
						ASSERT(FALSE);
					}
				}
				Resp.cyNetAmountPayType3.ParseCurrency(strResponse.Left(nResult));
				strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

				//30. PIC X(1)
				//Field Separator, already taken care of
			}

			if (!bRemovedEndText) {

				//31. PIC X(3
				//Payment Type #4
				Resp.strPayType4 = strResponse.Left(3);
				strResponse = strResponse.Right(strResponse.GetLength() - 3);

				//32. PIC 9(6)
				//Transaction Count #4
				Resp.nNumTransactionsPayType4 = atoi(strResponse.Left(6));
				strResponse = strResponse.Right(strResponse.GetLength() - 6);

				//33. PIC X(6)
				//Net Amount Pay Type #4
				strTemp = (char)0x1C;
				nResult = strResponse.Find(strTemp);
				if (nResult == -1 ) {
					
					//we must be at the end
					strTemp = (char)0x03;
					nResult = strResponse.Find(strTemp);

					if (nResult == -1) {
						ASSERT(FALSE);
					}
				}
				Resp.cyNetAmountPayType4.ParseCurrency(strResponse.Left(nResult));
				strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));
			
			
				//34. PIC X(1)
				//Field Separator, already taken care of

				//35. PIC X(1)
				// End of Text
				strResponse = strResponse.Right(strResponse.GetLength() - 1);
			}
			
		}
		else {
			//initialize everything to zero
			Resp.nNumTransactionsPayType1 = 0;
			Resp.nNumTransactionsPayType2 = 0;
			Resp.nNumTransactionsPayType3 = 0;
			Resp.nNumTransactionsPayType4 = 0;

			Resp.cyNetAmountPayType1 = COleCurrency(0,0);
			Resp.cyNetAmountPayType2 = COleCurrency(0,0);
			Resp.cyNetAmountPayType3 = COleCurrency(0,0);
			Resp.cyNetAmountPayType4 = COleCurrency(0,0);

			Resp.strPayType1 = "";
			Resp.strPayType2 = "";
			Resp.strPayType3 = "";
			Resp.strPayType4 = "";
		}


		if (! bRemovedEndText) {
			//35. PIC X(1)
			// End of Text
			strResponse = strResponse.Right(strResponse.GetLength() - 1);
		}

		//36. PIC X(1)
		//LRC
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		
		ASSERT(strResponse.IsEmpty());

		
		

		return TRUE;
	}

	BOOL ProcessRetailResponse(CString &strResponse, ResponseStruct &Resp) {


		//9. PIC X(2)
		//CardType
		Resp.strCardType = strResponse.Left(2);
		strResponse = strResponse.Right(strResponse.GetLength() - 2);


		//10. Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//11. PIC X(40)
		//Interchange Compliance
		//this is not required, so let's just read until the field separator
		CString strTemp = (char)0x1C;
		long nResult = strResponse.Find(strTemp);
		ASSERT(nResult >= 0);
		Resp.strInterchangeCompliance = strResponse.Left(nResult);
		strResponse = strResponse.Right(strResponse.GetLength() - (nResult + 1));

		//12. Field Separator, already taken care of

		//13. PIC 9(2)
		//Authorizing Network ID
		Resp.nAuthorizingNetworkID = atoi(strResponse.Left(2));
		strResponse = strResponse.Right(strResponse.GetLength() - 2);

		//14. PIC X(1)
		Resp.chAuthSource = strResponse.GetAt(0);
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//15. Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//16. Field Separator
		strResponse = strResponse.Right(strResponse.GetLength() - 1);

		//We could be done now, so let's look for out end of text deliminator
		strTemp = char(0x03);
		nResult = strResponse.Find(strTemp);
		ASSERT(nResult >= 0);
		
		if (nResult == 0) {
			//we are done
		}
		else {
			CString strOptionalToken = strResponse.Left(nResult);

			//17. PIC X(120)
			//Optional Data
			//we might have optionalData (although unlikely since we are retail)
			strTemp = (char)0x1C;
			nResult = strOptionalToken.Find(strTemp);
			ASSERT(nResult >= 0);
			Resp.strOptionalData = strOptionalToken.Left(nResult);
			strOptionalToken = strOptionalToken.Right(strOptionalToken.GetLength() - (nResult + 1));

			//18. Fields Separator
			//already taken care of

			//let's see if there are any tokens
			if (strOptionalToken.IsEmpty()) {
				//there aren't any, we are done
			}
			else {

				//there are some tokens
				// (a.walling 2007-07-02 10:20) - Process the tokens

				long nNextFS = -1;
				
				nNextFS = strOptionalToken.Find(chFieldSeparator, nNextFS == -1 ? 0 : nNextFS);
				
				while (nNextFS != -1) {
					CString strToken = strOptionalToken.Left(nNextFS);
					strOptionalToken = strOptionalToken.Right(strOptionalToken.GetLength() - nNextFS);

					strOptionalToken.TrimLeft(chFieldSeparator);

					// (a.walling 2007-06-21 13:46)
					ParseToken(strToken, Resp);

//#pragma message("Does the final token have a <FS> after it, or just an <ETX>?")
					nNextFS = strOptionalToken.Find(chFieldSeparator, nNextFS == -1 ? 0 : nNextFS);
				}
			}
		}

//		ASSERT(strResponse.IsEmpty());

		ProcessTokens(Resp);

		return TRUE;
		
	}



	CString MessageLookup(CString strErrorCode, CString strResponseMessage, LanguageCode lcCode) {

		CString strMessage;
		BOOL bCallNexTech = FALSE;

		if (lcCode == lcEnglish) {

			switch(atoi(strErrorCode)) {

				/************************************************Issuer Errors **************************************************/

				case 200:
				case 20001:
					strMessage = "Authorization Declined, Cardholder's bank did not approve the transaction.";
				break;

				case 201:
				case 20102:
					strMessage = "Call Voice Operator - Authorizer needs more information for approval.";
				break;

				case 202:
				case 20203:
					strMessage = "Hold card and Call Voice Operator - Card issuer does not want that card used.  Call for further instructions.";
				break;

				case 203:
				case 20304:
					strMessage = "Call Voice Operator - Authorizer did not respond within the allotted time.";
				break;

				case 204:
				case 20405:
					strMessage = "Invalid Card Number - Account Number/Magnetic Stripe is invalid.";
				break;

				case 205:
				case 20506:
					strMessage = "Invalid Expiration Date - Expiration Date is either incorrect format or expired.";
				break;

				case 206:
				case 20607:
					strMessage = "Invalid ICA Number - Invalid International Control Account number.";
				break;

				case 207:
				case 20708:
					strMessage = "Invalid ABA Number - Invalid American Banking Association number.";
				break;

				case 208:
				case 20809:
					strMessage = "Invalid PIN Number - The Personal ID number is incorrect.";
				break;

				case 209:
				case 20910:
					strMessage = "Invalid Bank MID - The Bank Merchant ID is incorrect";
				break;

				case 210:
				case 21011:
					strMessage = "Invalid Terminal Number - The Merchant ID is not valid or incorrect.";
				break;

				case 211:
				case 21112:
					strMessage = "Invalid Amount - The amount is either 0 or incorrectly formatted.";
					bCallNexTech = TRUE;
				break;

				case 213:
				case 21314:
					strMessage = "Invalid Transaction Format";
					bCallNexTech = TRUE;
				break;

				case 214:
				case 21415:
					strMessage = "Call Voice Operator - Authorization Center cannot be reached.";
				break;

				case 215:
				case 21516:
					strMessage = "This card has been reported lost or stolen.";
				break;

				case 216:
				case 21617:
					strMessage = "Invalid PIN - Personal ID code is incorrect.";
				break;

				case 217:
				case 21718:
					strMessage = "Amount requested exceeds credit limit.";
				break;

				case 218:
				case 21819:
					strMessage = "Request Denied - Transaction is not valid for this authorizer.";
				break;

				case 220:
				case 22020:
					strMessage = "Not Online to XX - Fatal Communications Error";
					bCallNexTech = TRUE;
				break;

				case 221:
				case 22121:
					strMessage = "Debit Authorizer temporarily unavailble, please try again later.";
				break;

				case 222:
				case 22222:
					strMessage = "Authorization Declined - Vehicle not found in positive file.";
				break;

				case 223:
				case 22323:
					strMessage = "Invalid PIN Number - Driver # not found in positive file.";
				break;

				case 224:
				case 22424:
					strMessage = "Authorization Declined - Card is on private label negative file.";
				break;

				case 225:
				case 22525:
					strMessage = "Card not allowed - Merchant does not accept this card.";
				break;

				case 226:
					strMessage = "Merchant not set up for Private Label.";
				break;

				case 227:
					strMessage = "BIN not allowed - Merchant cannot accept this Private Label BIN range.";
				break;
				
				case 228:
					strMessage = "Card Not Allowed - Merchant cannot accept this card.";
				break;

				case 229:
					strMessage = "Invalid Merchant Restriction Code - Restriction Code contains invalid data.";
				break;

				case 230:
					strMessage = "Product Restricted - Merchant attempted a product code not permitted by this merchant.";
				break;

				case 231:
					strMessage = "Product Not On File - Merchant attempted a product code that does not exist on host.";
				break;

				case 232:
					strMessage = "Authorization Declined - Invalid card type for Prior Authorization sale.";
				break;

				case 233:
					strMessage = "Authorization Declined - Terminal Type not supported.";
				break;

				case 234:
					strMessage = "Authorization Declined - T&E card used for Sale when merchant only allows Authorization Only.";
				break;

				case 235:
					strMessage = "Request Denied - Prior Authorization selected with no Authorization Code provided.";
				break;

				case 238:
					strMessage = "Invalid Driver Number - The Driver Number entered is invalid.";
				break;

				case 245:
					strMessage = "Cannot Process Request - SV error on prior authorization transaction.";
				break;

				case 246:
					strMessage = "Unable to Void - SV issuer unable to void transaction.";
				break;

				case 247:
					strMessage = "PIN not selected - EBT recipient has not selected a PIN for this card.";
				break;

				case 248:
					strMessage = "Unmatched Vehicle Information - Voucher submitted does not match one previously issued.";
				break;

				case 249:
					strMessage = "Transaction Not Defined - This type of transaction is not allowed for this type of card.";
				break;

				case 257:
					strMessage = "Block Activation No Allowed - The merchant is not allowed to process Stored Value Block Activations.";
				break;

				case 258:
					strMessage = "The activation amount requested does not match the predenominated amount for the card.";
				break;

				case 292:
					strMessage = "Authorization Down - Retry";
				break;

				case 293:
					strMessage = "293 - Authorizer is not available at this time.";
				break;

				case 294:
					strMessage = "294 - Authorizer is not available at this time.";
				break;

				case 297:
					strMessage = "297 - Authorizer is not available at this time.";
				break;

				case 298:
					strMessage = "298 - Debit Authorizer experienced an error.";
				break;

				case 299:
					strMessage = "299 - Debit Authorizer experienced an error.";
				break;

				case 300:
					strMessage = "Invalid Terminal ID - The length of the merchant ID is incorrect or contains invalid data.";
				break;

				/**************************************************** Format Errors *************************************************/

				case 301:
					strMessage = "Invalid Function - Transaction Code is incorrect or wrong length";
					bCallNexTech = TRUE;
				break;

				case 302:
					strMessage = "Invalid Card - Magnetic Stripe contains invalid data or account number is greater than 19 digits.";
				break;

				case 303:
					strMessage = "Invalid Expiration Date - Card has expired.";
				break;

				case 304:
					strMessage = "Invalid Action Code - Action Code is longer than 1 digit.";
					bCallNexTech = TRUE;
				break;

				case 305:
					strMessage = "Amount Entry Error - Amount contains invalid data.";
				break;

				case 306:
					strMessage = "Invalid PIN - Incorrect PIN block length.";
				break;
			
				case 307:
					strMessage = "Invalid Card - Invalid card type or account number";
				break;

				case 308:
					strMessage = "Authorization Number Not entered - Authorization Code was not entered.";
				break;

				case 309:
					strMessage = "Insurance Down payment indication is invalid.";
				break;

				case 310:
					strMessage = "Insurance policy number is an incorrect length.";
				break;

				case 311:
					strMessage = "Invalid Industry Code";
				break;
				
				case 312:
					strMessage = "Invalid Function -  The transcation in invalid.";
				break;

				case 313:
					strMessage = "Entry Mode Invalid - POS Entry mode does not contain a valid value.";
				break;

				case 314:
					strMessage = "Invalid Industry Data - The Industry Specific field contains invalid data.";
				break;

				case 315:
					strMessage = "Industry Fleet Data - The fleet card field contains invalid data.";
				break;

				case 316:
					strMessage = "Invalid System Info - The system information field contains invalid data.";
				break;

				case 317:
					strMessage = "Invalid Format - Payment Service indicator or Transaction ID is invalid.";
				break;

				case 318:
					strMessage = "Invalid Transaction Class - Transaction class is not F for Financial Transaction.";
					bCallNexTech = TRUE;
				break;

				case 319:
					strMessage = "Invalid PIN Capability - The PIN capapbility code contains invalid data.";
					bCallNexTech = TRUE;
				break;

				case 320:
					strMessage = "Invalid/Missing Retieval Reference.";
				break;

				case 321:
					strMessage = "Market Specific Data field contains invalid data.";
				break;

				case 322:
					strMessage = "Markes Specific Data fields duration is 00, blank, or missing.";
				break;

				case 323:
					strMessage = "Preferred Customer indicator contains invalid data";
				break;

				case 324:
					strMessage = "Mail/Telephone Order Number is invalid.";
				break;

				case 325:
					strMessage = "Hotel Sale Code, Charge Description, or Folio contains invalid data.";
				break;

				case 326:
					strMessage = "Multiple Clearing sequence number is invalid.";
				break;

				case 327:
					strMessage = "Purchasing card field contains invalid data.";
				break;

				case 328:
					strMessage = "Insurance transactionn not from VRU.";
				break;

				case 329:
					strMessage = "Invalid Electronic Commerce Data";
				break;

				case 330:
					strMessage = "System Problem";
					bCallNexTech = TRUE;
				break;

				case 331:
					strMessage = "An Invalid Token Value was received";
				break;

				case 332:
					strMessage = "Error with the cardholder verification data.";
				break;

				case 333:
					strMessage = "System Problem";
					bCallNexTech = TRUE;
				break;

				case 400:
					strMessage = "Invalid Terminal ID - Merchant ID not found in merchant file.";
					bCallNexTech = TRUE;
				break;

				case 401:
					strMessage = "Invalid Terminal ID - Merchant ID not found in merchant file.";
					bCallNexTech = TRUE;
				break;

				case 402:
					strMessage = "Terminal Not Activated - Active flag for merchant set to 'N'.";
				break;

				case 403:
					strMessage = "Invalid Act Code - Merchant not set up for cash advance function.";
				break;

				case 404:
					strMessage = "Void Not Allowed - The transaction requested for voiding is not an EFT transaction.";
				break;

				case 405:
					strMessage = "Ref Number not found - Transaction requested for reversal not found.";
				break;

				case 406:
					strMessage = "Proc Error 7 - The host cannot clear all transaction records for the requested Batch Release.";
				break;

				case 407:
					strMessage = "Too Many Batches - There are 999 open batches for this merchant.";
				break;

				case 408:
					strMessage = "Release Batch - Current Batch has 999 records, release batch before continuing.";
				break;

				case 409:
					strMessage = "Invalid Function - Debit transaction requested but debit flag is set to 'N'.";
				break;

				case 410:
					strMessage = "Invalid Terminal ID - The terminal ID portion of the merchant ID is incorrect.";
				break;

				case 411:
					strMessage = "Invalid Terminal ID - The maximum retries for this merchant have been exceded.";
				break;

				case 412:
					strMessage = "Proc Error 13 - Unable to read reference number file.";
				break;

				case 413:
					strMessage = "Proc Error 14 - Unable to read reference number file";
				break;

				case 414:
					strMessage = "Proc Error 15 - Unable to read reference number file";
				break;

				case 415:
					strMessage = "Invalid Function - Merchant is Authorization Only and a draft capture record was sent.";
				break;

				case 416:
					strMessage = "Invalid Function - Merchant is Authorization Only and a debit record was sent.";
				break;

				case 417:
					strMessage = "Invalid Function - Private label flag is 'N' but a private label account number was sent.";
				break;

				case 418:
					strMessage = "Please Try Again - Incorrect debit working key.";
				break;
				
				case 419:
					strMessage = "Invalid Function - Manually entered transactions are not allowed for this terminal ID.";
				break;

				case 420:
					strMessage = "Amount too Large - Maximum sale amount exceeded.";
				break;

				case 421:
					strMessage = "Amount too Large - Maximum return amount exceeded.";
				break;

				case 422:
					strMessage = "Invalid Terminal ID - Host couldn't read terminal file within the specified time.";
				break;

				case 423:
					strMessage = "Proc Error 24 - Host couldn't read reference number file within specified time.";
				break;

				case 424:
					strMessage = "Invalid Terminal ID - Transaction open flag has been set to 'Y' within prior 3 minutes.";
				break;

				case 425:
					strMessage = "Invalid Function - Cash management not allowed for this merchant ID.";
				break;

				case 426:
					strMessage = "Rev not allowed - Host found no batch number matching the one sent.";
				break;

				case 427:
					strMessage = "Rev not allowed - Host found no transactions meeting the specifications sent.";
				break;

				case 428:
					strMessage = "Discover Not Allowed - Merchant not set up for Discover transactions.";
				break;

				case 429:
					strMessage = "Rev not allowed - The batch containing the transaction to void has been released.";
				break;

				case 430:
					strMessage = "Discover Not Allowed - Merchant not set up for Discover.";
				break;

				case 431:
					strMessage = "Diners Club not allowed - Merchant not set up for Diners Club.";
				break;

				case 432:
					strMessage = "Carte Blanche not allowed - Merchant not set up for Carte Blanche.";
				break;

				case 433:
					strMessage = "Invalid Key - No AMEX subscriber number, process control ID, or product code set up.";
				break;

				case 434:
					strMessage = "Invalid Key - Future Use.";
				break;

				case 435:
					strMessage = "Failed, Please Call Paymentech - Debit transaction being sent to an authorizer not set up on host file.";
				break;

				case 436:
					strMessage = "Failed, Please Call Paymentech - Debit Security key does not exist on the security management file.";
				break;

				case 437:
					strMessage = "Failed, Please Call Paymentech - Failure occurred during encyrption/decryption of PIN.";
				break;

				case 438:
					strMessage = "Failed, Please Call Paymentech - Errro occurred while generating a debit working key.";
				break;

				case 439:
					strMessage = "Failed, Please Call Paymentech - The debitsponsor institution on the merchant file is not set up on the sponsor file.";
				break;

				case 440:
					strMessage = "The network set up on the sponsoring bnk file for this institution is not set up on the host's network file.";
				break;

				case 441:
					strMessage = "The host is unable to communicate with decryption device.";
				break;

				case 442:
					strMessage = "JCB not allowed - the JCB flag on merchant file is not set up for JCB transactions.";
				break;

				case 443:
					strMessage = "JCB not allowed - JCB subscriber number not set up for JCB transactions.";
				break;

				case 444:
					strMessage = "Bank not on File - Debit BIN not set up for this merchant routing table.";
				break;

				case 445:
					strMessage = "No Sponsor Inst - No valid sponsorship was found on Merchant record.";
				break;

				case 446:
					strMessage = "Failed, Please Call Paymentech - Future Use";
				break;

				case 447:
					strMessage = "WX not available - Merchant not setup to accept WEX.";
				break;

				case 448:
					strMessage = "Amount too large - Amount exceeds maximum limit.";
				break;

				case 449:
					strMessage = "Reenter Odometer - Odometer was 0000000 or contained non-numeric data.";
				break;

				case 450:
					strMessage = "Duplicate Transaction - No ACK reversal was followed by a duplicate request.";
				break;

				case 451:
					strMessage = "Transaction Now Allowed - Requested Transaction type is not allowed for this card/merchant.";
				break;

				case 452:
					strMessage = "Batch Already Released - Batch has already been released.";
				break;

				case 453:
					strMessage = "Invalid Routing Indicator";
				break;

				case 454:
					strMessage = "AMEX not Allowed";
				break;

				case 493:
					strMessage = "Amount Required Over Limit - Total authorized amount will exceed the allowed Sales or Return limit for this device.";
				break;

				case 999:
					strMessage = "Invalid Merchant - Merchant number not on file.";
				break;

				/*****************************************DEBIT/EBT Specific Records*****************************************/

				case 602:
					strMessage = "Call Voice Operator - Authorization Center cannot be reached.";
				break;

				case 692:
					strMessage = "Debit authorizer temporarily unavailable, please retry.";
				break;

				case 693:
					strMessage = "Queue for debit authorizer too long, please retry";
				break;

				case 694:
					strMessage = "Debit authorizer not responding in time, please retry.";
				break;

				/*******************************************Batch Management Errors ********************************************/

				case 105:
					strMessage = "Invalid Terminal ID - Merchant ID on a batch inquiry or release is incorrect.";
				break;

				case 106:
					strMessage = "Terminal not active - Active flag for the merchant ID is set to 'N'";
				break;

				case 107:
					strMessage = "No Transactions - A batch Inquiry or Release was requested but no open batch exists.";
				break;

				case 108:
					strMessage = "Batch Already Released - A second batch release was attempted.";
				break;

				case 109:
					strMessage = "Batch Not Found - Requested batch does not exist.";
				break;

				default: 
					strMessage = strResponseMessage;
				break;
			}
		
			CString strReturn;
			strReturn.Format("Failed with error %li, Message: %s", atoi(strErrorCode), strMessage);
			return strReturn;
		}
		else if (lcCode == lcFrench) {

			switch (atoi(strErrorCode)) {

				case 200: 
					strMessage = "L'autorisation a diminué, la banque du détenteur de carte n'a pas approuvé la transaction.";
				break;

				case 201:
					strMessage = "Opérateur de voix d'appel - Authorizer a besoin de plus d'information pour approbation.";
				break;

				case 202:
					strMessage = "Carte de prise et opérateur de voix d'appel - l'émetteur de carte ne veut pas que la carte ait employé. Appel pour des instructions complémentaires.";
				break;

				case 203:
					strMessage = "Opérateur de voix d'appel - Authorizer n'a pas répondu dans le temps réparti.";
				break;

				case 204:
					strMessage = "Nombre de carte inadmissible - la raie du compte Number/Magnetic est inadmissible.";
				break;

				case 205:
					strMessage = "Date d'échéance inadmissible - la date d'échéance est l'un ou l'autre format incorrect ou expiré.";
				break;

				case 206:
					strMessage = "Nombre inadmissible d'ICA - nombre international inadmissible de compte collectif.";
				break;

				case 207:
					strMessage = "Invalid ABA Number - Invalid American Banking Association number.";
				break;

				case 208:
					strMessage = "Invalid PIN Number - The Personal ID number is incorrect.";
				break;

				case 209:
					strMessage = "Invalid Bank MID - The Bank Merchant ID is incorrect";
				break;

				case 210:
					strMessage = "Invalid Terminal Number - The Merchant ID is not valid or incorrect.";
				break;

				case 211:
					strMessage = "Invalid Amount - The amount is either 0 or incorrectly formatted.";
					bCallNexTech = TRUE;
				break;

				case 213:
					strMessage = "Invalid Transaction Format";
					bCallNexTech = TRUE;
				break;

				case 214:
					strMessage = "Call Voice Operator - Authorization Center cannot be reached.";
				break;

				case 215:
					strMessage = "This card has been reported lost or stolen.";
				break;

				case 216:
					strMessage = "Invalid PIN - Personal ID code is incorrect.";
				break;

				case 217:
					strMessage = "Amount requested exceeds credit limit.";
				break;

				case 218:
					strMessage = "Request Denied - Transaction is not valid for this authorizer.";
				break;

				case 220:
					strMessage = "Not Online to XX - Fatal Communications Error";
					bCallNexTech = TRUE;
				break;

				case 221:
					strMessage = "Debit Authorizer temporarily unavailble, please try again later.";
				break;

				case 222:
					strMessage = "Authorization Declined - Vehicle not found in positive file.";
				break;

				case 223:
					strMessage = "Invalid PIN Number - Driver # not found in positive file.";
				break;

				case 224:
					strMessage = "Authorization Declined - Card is on private label negative file.";
				break;

				case 225:
					strMessage = "Card not allowed - Merchant does not accept this card.";
				break;

				case 226:
					strMessage = "Merchant not set up for Private Label.";
				break;

				case 227:
					strMessage = "BIN not allowed - Merchant cannot accept this Private Label BIN range.";
				break;
				
				case 228:
					strMessage = "Card Not Allowed - Merchant cannot accept this card.";
				break;

				case 229:
					strMessage = "Invalid Merchant Restriction Code - Restriction Code contains invalid data.";
				break;

				case 230:
					strMessage = "Product Restricted - Merchant attempted a product code not permitted by this merchant.";
				break;

				case 231:
					strMessage = "Product Not On File - Merchant attempted a product code that does not exist on host.";
				break;

				case 232:
					strMessage = "Authorization Declined - Invalid card type for Prior Authorization sale.";
				break;

				case 233:
					strMessage = "Authorization Declined - Terminal Type not supported.";
				break;

				case 234:
					strMessage = "Authorization Declined - T&E card used for Sale when merchant only allows Authorization Only.";
				break;

				case 235:
					strMessage = "Request Denied - Prior Authorization selected with no Authorization Code provided.";
				break;

				case 238:
					strMessage = "Invalid Driver Number - The Driver Number entered is invalid.";
				break;

				case 245:
					strMessage = "Cannot Process Request - SV error on prior authorization transaction.";
				break;

				case 246:
					strMessage = "Unable to Void - SV issuer unable to void transaction.";
				break;

				case 247:
					strMessage = "PIN not selected - EBT recipient has not selected a PIN for this card.";
				break;

				case 248:
					strMessage = "Unmatched Vehicle Information - Voucher submitted does not match one previously issued.";
				break;

				case 249:
					strMessage = "Transaction Not Defined - This type of transaction is not allowed for this type of card.";
				break;

				case 257:
					strMessage = "Block Activation No Allowed - The merchant is not allowed to process Stored Value Block Activations.";
				break;

				case 258:
					strMessage = "The activation amount requested does not match the predenominated amount for the card.";
				break;

				case 292:
					strMessage = "Authorization Down - Retry";
				break;

				case 293:
					strMessage = "293 - Authorizer is not available at this time.";
				break;

				case 294:
					strMessage = "294 - Authorizer is not available at this time.";
				break;

				case 297:
					strMessage = "297 - Authorizer is not available at this time.";
				break;

				case 298:
					strMessage = "298 - Debit Authorizer experienced an error.";
				break;

				case 299:
					strMessage = "299 - Debit Authorizer experienced an error.";
				break;

				case 300:
					strMessage = "Invalid Terminal ID - The length of the merchant ID is incorrect or contains invalid data.";
				break;

				/**************************************************** Format Errors *************************************************/

				case 301:
					strMessage = "Invalid Function - Transaction Code is incorrect or wrong length";
					bCallNexTech = TRUE;
				break;

				case 302:
					strMessage = "Invalid Card - Magnetic Stripe contains invalid data or account number is greater than 19 digits.";
				break;

				case 303:
					strMessage = "Invalid Expiration Date - Card has expired.";
				break;

				case 304:
					strMessage = "Invalid Action Code - Action Code is longer than 1 digit.";
					bCallNexTech = TRUE;
				break;

				case 305:
					strMessage = "Amount Entry Error - Amount contains invalid data.";
				break;

				case 306:
					strMessage = "Invalid PIN - Incorrect PIN block length.";
				break;
			
				case 307:
					strMessage = "Invalid Card - Invaid card type or account number";
				break;

				case 308:
					strMessage = "Authorization Number Not entered - Authorization Code was not entered.";
				break;

				case 309:
					strMessage = "Insurance Down payment indication is invalid.";
				break;

				case 310:
					strMessage = "Insurance policy number is an incorrect length.";
				break;

				case 311:
					strMessage = "Invalid Industry Code";
				break;
				
				case 312:
					strMessage = "Invalid Function -  The transcation in invalid.";
				break;

				case 313:
					strMessage = "Entry Mode Invalid - POS Entry mode does not contain a valid value.";
				break;

				case 314:
					strMessage = "Invalid Industry Data - The Industry Specific field contains invalid data.";
				break;

				case 315:
					strMessage = "Industry Fleet Data - The fleet card field contains invalid data.";
				break;

				case 316:
					strMessage = "Invalid System Info - The system information field contains invalid data.";
				break;

				case 317:
					strMessage = "Invalid Format - Payment Service indicator or Transaction ID is invalid.";
				break;

				case 318:
					strMessage = "Invalid Transaction Class - Transaction class is not F for Financial Transaction.";
					bCallNexTech = TRUE;
				break;

				case 319:
					strMessage = "Invalid PIN Capability - The PIN capapbility code contains invalid data.";
					bCallNexTech = TRUE;
				break;

				case 320:
					strMessage = "Invalid/Missing Retieval Reference.";
				break;

				case 321:
					strMessage = "Market Specific Data field contains invalid data.";
				break;

				case 322:
					strMessage = "Markes Specific Data fields duration is 00, blank, or missing.";
				break;

				case 323:
					strMessage = "Preferred Customer indicator contains invalid data";
				break;

				case 324:
					strMessage = "Mail/Telephone Order Number is invalid.";
				break;

				case 325:
					strMessage = "Hotel Sale Code, Charge Description, or Folio contains invalid data.";
				break;

				case 326:
					strMessage = "Multiple Clearing sequence number is invalid.";
				break;

				case 327:
					strMessage = "Purchasing card field contains invalid data.";
				break;

				case 328:
					strMessage = "Insurance transactionn not from VRU.";
				break;

				case 329:
					strMessage = "Invalid Electronic Commerce Data";
				break;

				case 330:
					strMessage = "System Problem";
					bCallNexTech = TRUE;
				break;

				case 331:
					strMessage = "An Invalid Token Value was received";
				break;

				case 332:
					strMessage = "Error with the cardholder verification data.";
				break;

				case 333:
					strMessage = "System Problem";
					bCallNexTech = TRUE;
				break;

				case 400:
					strMessage = "Invalid Terminal ID - Merchant ID not found in merchant file.";
					bCallNexTech = TRUE;
				break;

				case 401:
					strMessage = "Invalid Terminal ID - Merchant ID not found in merchant file.";
					bCallNexTech = TRUE;
				break;

				case 402:
					strMessage = "Terminal Not Activated - Active flag for merchant set to 'N'.";
				break;

				case 403:
					strMessage = "Invalid Act Code - Merchant not set up for cash advance function.";
				break;

				case 404:
					strMessage = "Void Not Allowed - The transaction requested for voiding is not an EFT transaction.";
				break;

				case 405:
					strMessage = "Ref Number not found - Transaction requested for reversal not found.";
				break;

				case 406:
					strMessage = "Proc Error 7 - The host cannot clear all transaction records for the requested Batch Release.";
				break;

				case 407:
					strMessage = "Too Many Batches - There are 999 open batches for this merchant.";
				break;

				case 408:
					strMessage = "Release Batch - Current Batch has 999 records, release batch before continuing.";
				break;

				case 409:
					strMessage = "Invalid Function - Debit transaction requested but debit flag is set to 'N'.";
				break;

				case 410:
					strMessage = "Invalid Terminal ID - The terminal ID portion of the merchant ID is incorrect.";
				break;

				case 411:
					strMessage = "Invalid Terminal ID - The maximum retries for this merchant have been exceded.";
				break;

				case 412:
					strMessage = "Proc Error 13 - Unable to read reference number file.";
				break;

				case 413:
					strMessage = "Proc Error 14 - Unable to read reference number file";
				break;

				case 414:
					strMessage = "Proc Error 15 - Unable to read reference number file";
				break;

				case 415:
					strMessage = "Invalid Function - Merchant is Authorization Only and a draft capture record was sent.";
				break;

				case 416:
					strMessage = "Invalid Function - Merchant is Authorization Only and a debit record was sent.";
				break;

				case 417:
					strMessage = "Invalid Function - Private label flag is 'N' but a private label account number was sent.";
				break;

				case 418:
					strMessage = "Please Try Again - Incorrect debit working key.";
				break;
				
				case 419:
					strMessage = "Invalid Function - Manually entered transactions are not allowed for this terminal ID.";
				break;

				case 420:
					strMessage = "Amount too Large - Maximum sale amount exceeded.";
				break;

				case 421:
					strMessage = "Amount too Large - Maximum return amount exceeded.";
				break;

				case 422:
					strMessage = "Invalid Terminal ID - Host couldn't read terminal file within the specified time.";
				break;

				case 423:
					strMessage = "Proc Error 24 - Host couldn't read reference number file within specified time.";
				break;

				case 424:
					strMessage = "Invalid Terminal ID - Transaction open flag has been set to 'Y' within prior 3 minutes.";
				break;

				case 425:
					strMessage = "Invalid Function - Cash management not allowed for this merchant ID.";
				break;

				case 426:
					strMessage = "Rev not allowed - Host found no batch number matching the one sent.";
				break;

				case 427:
					strMessage = "Rev not allowed - Host found no transactions meeting the specifications sent.";
				break;

				case 428:
					strMessage = "Discover Not Allowed - Merchant not set up for Discover transactions.";
				break;

				case 429:
					strMessage = "Rev not allowed - The batch containing the transaction to void has been released.";
				break;

				case 430:
					strMessage = "Discover Not Allowed - Merchant not set up for Discover.";
				break;

				case 431:
					strMessage = "Diners Club not allowed - Merchant not set up for Diners Club.";
				break;

				case 432:
					strMessage = "Carte Blanche not allowed - Merchant not set up for Carte Blanche.";
				break;

				case 433:
					strMessage = "Invalid Key - No AMEX subscriber number, process control ID, or product code set up.";
				break;

				case 434:
					strMessage = "Invalid Key - Future Use.";
				break;

				case 435:
					strMessage = "Failed, Please Call Paymentech - Debit transaction being sent to an authorizer not set up on host file.";
				break;

				case 436:
					strMessage = "Failed, Please Call Paymentech - Debit Security key does not exist on the security management file.";
				break;

				case 437:
					strMessage = "Failed, Please Call Paymentech - Failure occurred during encyrption/decryption of PIN.";
				break;

				case 438:
					strMessage = "Failed, Please Call Paymentech - Errro occurred while generating a debit working key.";
				break;

				case 439:
					strMessage = "Failed, Please Call Paymentech - The debitsponsor institution on the merchant file is not set up on the sponsor file.";
				break;

				case 440:
					strMessage = "The network set up on the sponsoring bnk file for this institution is not set up on the host's network file.";
				break;

				case 441:
					strMessage = "The host is unable to communicate with decryption device.";
				break;

				case 442:
					strMessage = "JCB not allowed - the JCB flag on merchant file is not set up for JCB transactions.";
				break;

				case 443:
					strMessage = "JCB not allowed - JCB subscriber number not set up for JCB transactions.";
				break;

				case 444:
					strMessage = "Bank not on File - Debit BIN not set up for this merchant routing table.";
				break;

				case 445:
					strMessage = "No Sponsor Inst - No valid sponsorship was found on Merchant record.";
				break;

				case 446:
					strMessage = "Failed, Please Call Paymentech - Future Use";
				break;

				case 447:
					strMessage = "WX not available - Merchant not setup to accept WEX.";
				break;

				case 448:
					strMessage = "Amount too large - Amount exceeds maximum limit.";
				break;

				case 449:
					strMessage = "Reenter Odometer - Odometer was 0000000 or contained non-numeric data.";
				break;

				case 450:
					strMessage = "Duplicate Transaction - No ACK reversal was followed by a duplicate request.";
				break;

				case 451:
					strMessage = "Transaction Now Allowed - Requested Transaction type is not allowed for this card/merchant.";
				break;

				case 452:
					strMessage = "Batch Already Released - Batch has already been released.";
				break;

				case 453:
					strMessage = "Invalid Routing Indicator";
				break;

				case 454:
					strMessage = "AMEX not Allowed";
				break;

				case 493:
					strMessage = "Amount Required Over Limit - Total authorized amount will exceed the allowed Sales or Return limit for this device.";
				break;

				case 999:
					strMessage = "Invalid Merchant - Merchant number not on file.";
				break;

				/*****************************************DEBIT/EBT Specific Records*****************************************/

				case 602:
					strMessage = "Call Voice Operator - Authorization Center cannot be reached.";
				break;

				case 692:
					strMessage = "Debit authorizer temporarily unavailable, please retry.";
				break;

				case 693:
					strMessage = "Queue for debit authorizer too long, please retry";
				break;

				case 694:
					strMessage = "Debit authorizer not responding in time, please retry.";
				break;

				/*******************************************Batch Management Errors ********************************************/

				case 105:
					strMessage = "Invalid Terminal ID - Merchant ID on a batch inquiry or release is incorrect.";
				break;

				case 106:
					strMessage = "Terminal not active - Active flag for the merchant ID is set to 'N'";
				break;

				case 107:
					strMessage = "No Transactions - A batch Inquiry or Release was requested but no open batch exists.";
				break;

				case 108:
					strMessage = "Batch Already Released - A second batch release was attempted.";
				break;

				case 109:
					strMessage = "Batch Not Found - Requested batch does not exist.";
				break;

				default: 
					strMessage = strResponseMessage;
				break;				
			}

			return strMessage;
		}
		else {
			ASSERT(FALSE);
			return "";
		}
	}

	// (a.walling 2007-07-18 14:41)
	void ParseToken(CString strToken, ResponseStruct& Resp)
	{
		CString strTokenID = strToken.Left(2);
		strToken = strToken.Right(strToken.GetLength() - 2);
		CString strStoredToken;

		strToken.TrimLeft(chFieldSeparator);
		strToken.TrimRight(chFieldSeparator);

		if (Resp.mapTokens.Lookup(strTokenID, strStoredToken)) {
			//we already have data for this token!
			ASSERT(FALSE);
			ThrowNxException("Recieved duplicate tokens (%s)!", strTokenID);
		} else {
			Resp.mapTokens.SetAt(strTokenID, strToken);
		}
	}

	// (a.walling 2007-07-30 14:40)
	void ProcessTokens(ResponseStruct &Resp, OPTIONAL long nFlags /*= 0*/)
	{
		// A lot of tokens are processed outside of this function. Need to figure out
		// if that logic should be moved in here, or if the token should be removed
		// from the map after processing.

		CString strToken;
		POSITION pos = Resp.mapTokens.GetStartPosition();
		CString strTokenID, strTokenData;

		while (pos) {
			Resp.mapTokens.GetNextAssoc(pos, strTokenID, strTokenData);

			TRACE("Token(%s) - Data(%s)\r\n", strTokenID, strTokenData);

			if (strTokenID == "ER") { // Error response
				TRACE("The token '%s' raised an error response\r\n", strTokenID);

			} else if (strTokenID == "CV") { // CVV2/CVC2/CID Indicator
				CString strIndicator = strToken.Left(2);

				ASSERT(strIndicator == "VR");

				strTokenData = strToken.Right(strToken.GetLength() - 2);

				CString strResponse = strTokenData.Left(1);

				/*
				Flag indicating the result of CVD request
				M = Match
				N = No Match
				P = Not Processed
				S = Data should be on card, but indication was not present.
				U = Issuer is not certified and /or not provided encryption keys.
				*/

				// Not exactly sure what I should do in code depending on the result.
				CString strMsg;
				strMsg.Format("CV Response %s\r\n", strResponse);
				TRACE(strMsg); 

			} else if (strTokenID == "DC") { // Dynamic Currency Conversion
				// probably won't be implementing this
				return;
			} else {
				// ignore unsupported tokens
				return;
			}
		}
	}

	CString GetMessageFromType(TransactionCode tcCode) {

		switch (tcCode) {

			case tcSale :
				return "Credit Sale";
			break;

			case tcAuthOnly: 
				return "Credit Auth";
			break;

			case tcReturn:
				return "Credit Return";
			break;
			
			case tcForceSale:
				return "Credit Forced Sale";
			break;
			
			case tcVOID:
				return "Credit Void";
			break;

			case tcBatchInquiry:
				return "Batch Inquiry";
			break;

			case tcBatchRelease:
				return "Batch Release";
			break;

			case tcDebit:
				return "US Debit";
			break;

			case tcInteracSale:
				return "Interac Sale";
			break;

			case tcInteracSaleWithCashBack:
				return "Interac Sale with Cash Back";
			break;

			case tcInteracReturn:
				return "Interac Return";
			break;

			case tcMACReversal:
				return "MAC Reversal";
			break;

			case tcCurrentKeyRequest:
				return "Current Key Request";
			break;

			default:
				return "Unknown Transaction";
			break;
		}
	}

	BOOL AuthorizeTransaction(TransStruct &ptsTransaction, ResponseStruct &pResponse) {

		//let's setup our transaction
		Paymentech::IIPOSTransactionPtr pPayTechTrans;
		Paymentech::IIPOSResponsePtr pPayTechResponse;

		pPayTechTrans.CreateInstance(__uuidof(Paymentech::IPOSTransaction));

		long nErrorCode = 0;

		PaymentechUtils::InitTransaction(pPayTechTrans, ptsTransaction);

		//now we have to fill the transaction with things specific to this payment
		//let's see what type we are doing

		CString strMessage;

		PaymentechUtils::InitTerminalMessage(strMessage, ptsTransaction);

		//now set the rest of the message according to its type
		switch (ptsTransaction.tcTransactionCode) {

			case PaymentechUtils::tcSale:
			case PaymentechUtils::tcAuthOnly:
			case PaymentechUtils::tcReturn:
			case PaymentechUtils::tcForceSale:
				
				
				PaymentechUtils::GetTerminalRetailDebitMessage(strMessage, ptsTransaction);
				PaymentechUtils::GetTerminalRetailMessage(strMessage,ptsTransaction);

			break;

			case PaymentechUtils::tcDebit:
				PaymentechUtils::GetTerminalRetailDebitMessage(strMessage, ptsTransaction);
				PaymentechUtils::GetTerminalDebitMessage(strMessage, ptsTransaction);
			break;

			case PaymentechUtils::tcInteracSale:
			case PaymentechUtils::tcInteracReturn:
			case PaymentechUtils::tcInteracSaleWithCashBack:
			
				PaymentechUtils::GetTerminalRetailDebitMessage(strMessage, ptsTransaction);

								
				if (! PinPadUtils::SendInteracRequest(ptsTransaction.iTransID, ptsTransaction.cyTransAmount, ptsTransaction.strAccountNumber, 
					//information for MACing
					ptsTransaction.strMerchantNumber, ZeroFill(ptsTransaction.nTerminalNumber, 3, "Terminal Number"), ZeroFill(ptsTransaction.tcTransactionCode,2, "Transaction Code"),
					ZeroFill(ptsTransaction.nSequenceNumber, 6, "Sequence Number"),
					//return values
					ptsTransaction.nAccountType, ptsTransaction.strMACBlock, ptsTransaction.strPinBlock, ptsTransaction.strPinPadSerialNumber)) {

					return FALSE;
				}


				PaymentechUtils::GetTerminalInteracMessage(strMessage, ptsTransaction);
			break;

		}

#ifdef _DEBUG
		CFile flOut("Output.txt", CFile::modeCreate|CFile::modeWrite|CFile::shareCompat);
		flOut.Write((LPCTSTR)strMessage, strMessage.GetLength());

		PinPadUtils::LogCCProcess("Sending " + GetMessageFromType(ptsTransaction.tcTransactionCode) + " to Paymentech: " + strMessage + "\r\n");
#endif

		VARIANT v;
		unsigned char HUGEP *pData;

		VariantInit(&v);
		v.vt = VT_ARRAY|VT_UI1;
		v.parray = SafeArrayCreateVector(VT_UI1, 0, strMessage.GetLength() + 1);

		SafeArrayAccessData(v.parray, (void HUGEP **)&pData);

		memcpy(pData, (LPCTSTR) strMessage, strMessage.GetLength() + 1);		

		SafeArrayUnaccessData(v.parray);
		
		pPayTechResponse = pPayTechTrans->Process(v);

		
		CString strGatewayError = (LPCTSTR)pPayTechResponse->GetGatewayError();
		CString strHTTPError = (LPCTSTR)pPayTechResponse->GetHTTPError();

		_variant_t varResponse = pPayTechResponse->Data;

	
		if (pPayTechResponse->error != 0 ) {

			nErrorCode = pPayTechResponse->error;
		}
		else if (strGatewayError != "") {
			nErrorCode = atoi(strGatewayError);
		}
		else if (strHTTPError != "") {
			nErrorCode = atoi(strHTTPError);
		}

		if (nErrorCode != 0 ) {
			//CString strError;
			//strError.Format("Error %li Encountered; Description: " + pPayTechResponse->ErrorString, nErrorCode);
			//AfxMessageBox(strError);
			return FALSE;
		}
		HandleResult(_ZFN(ptsTransaction.nTerminalNumber, 3, "Terminal Number"), pPayTechResponse->Data, pResponse, ptsTransaction.tcTransactionCode, ptsTransaction.lcLangCode);

		return TRUE;
	}

	void HandleResult(CString strTerminalID, const VARIANT &v, ResponseStruct &pResp, PaymentechUtils::TransactionCode tcTransType, PaymentechUtils::LanguageCode lcLangCode) {

		long nUpper, nLower;
		unsigned char HUGEP *pData;
		unsigned char element;
		CString strResponse;

		SafeArrayGetLBound(v.parray, 1, &nLower);
		SafeArrayGetUBound(v.parray, 1, &nUpper);

		SafeArrayAccessData(v.parray, (void HUGEP**)&pData);

		for (int i=nLower; i <= nUpper; i++) {

			element = pData[i];

			strResponse += element;
		}

		SafeArrayUnaccessData(v.parray);

#ifdef _DEBUG

		CFile flResp("Response.txt", CFile::modeCreate|CFile::modeWrite|CFile::shareCompat);
		flResp.Write((LPCTSTR)strResponse, strResponse.GetLength());

		PinPadUtils::LogCCProcess("Reponse received from Paymentech: " + strResponse + "\r\n");
#endif

		pResp.lcLangCode = lcLangCode;

		switch (tcTransType) {

			case PaymentechUtils::tcSale:
			case PaymentechUtils::tcAuthOnly:
			case PaymentechUtils::tcReturn:
			case PaymentechUtils::tcForceSale:	
			case PaymentechUtils::tcVOID:				
				PaymentechUtils::ProcessResponseHeader(strResponse, pResp);
				PaymentechUtils::ProcessRetailResponse(strResponse, pResp);
			break;	
					
			case PaymentechUtils::tcBatchInquiry:
			case PaymentechUtils::tcBatchRelease:
				PaymentechUtils::ProcessResponseHeader(strResponse, pResp);
				PaymentechUtils::ProcessBatchResponse(strResponse, pResp);
			break;

			case PaymentechUtils::tcCurrentKeyRequest:
				if (PaymentechUtils::ProcessCurrentKeyResponse(strResponse, pResp)) {
					if (PaymentechUtils::ProcessInteracTokenData(strResponse, pResp)) {
						PerformInteracResponseAnalysis(pResp);
					}
				}
			break;

			case PaymentechUtils::tcInteracSale:
			case PaymentechUtils::tcInteracSaleWithCashBack:
			case PaymentechUtils::tcInteracReturn:
			case PaymentechUtils::tcMACReversal:
				if (PaymentechUtils::ProcessResponseHeader(strResponse, pResp)) {
					if (PaymentechUtils::ProcessInteracResponse(strResponse, pResp)) {
						if (PaymentechUtils::ProcessInteracTokenData(strResponse, pResp)) {
							PerformInteracResponseAnalysis(pResp);
						}
					}
				} 
			break;

		}

		
		//now we have to do something with the response
/*		if (!pResp.bApproved) {
			//bummer, we got an error, we'll have to show them something
			CString strError = PaymentechUtils::MessageLookup(pResp.strErrorCode);
			if (strError.IsEmpty()) {
				//AfxMessageBox(pResp.strResponseMessage);
			}
			else {
				//AfxMessageBox(strError);
			}

			return;
		}
		else {
			
			//AfxMessageBox("Approved!");
			//SetDlgItemText(IDC_AUTH_NUM, Resp.strAuthCode);

		}*/

	}

	
	void PerformInteracResponseAnalysis(PaymentechUtils::ResponseStruct &Resp) {

		//first we have to get the keys from the Key token that came back in the reponse
		CString strTokenIndicator, strTokenData;
		strTokenIndicator = "CK";
		CString strTPK, strTAK;
		if (Resp.mapTokens.Lookup(strTokenIndicator, strTokenData) >= 0) {

			strTPK = strTokenData.Left(16);

			strTokenData = strTokenData.Right(strTokenData.GetLength() - 16);

			strTAK = strTokenData.Left(16);

			strTokenData = strTokenData.Right(strTokenData.GetLength() - 16);

			//check the Forced current key request
				long nForcedCurKey = atoi(strTokenData.Left(1));

			if (nForcedCurKey == 1) {
				
				//we have to perform a current key request message
				SendCurrentKeyRequest();
			}
		}
		else {
			ASSERT(FALSE);
		}		

		//next, get the MAC data from the token
		CString strMACData;
		strTokenIndicator = "MC";
		if (Resp.mapTokens.Lookup(strTokenIndicator, strTokenData) >= 0) {
			strMACData = strTokenData;		
		}
		else {
			ASSERT(FALSE);
		}		

		//now figure out what our message should be
		CString strTopLine, strBottomLine;
		if (Resp.bApproved) {
			strTopLine = "Approved!";
			strBottomLine = "Please Come Again";
		}
		else {
			strTopLine = "Not Approved";
			//TODO: lookup an appropriate message here
			strBottomLine = "UH OHHHHHHHHH";
		}
				
		CString strReturnMessage;

		if (! PinPadUtils::PerformInteracResponseAnalysis(Resp.lcLangCode, "", "", strTPK, strTAK, strTopLine, strBottomLine, strMACData, strReturnMessage)) {
			//AfxMessageBox(strReturnMessage);
			//print out an error to the pinpad
			PinPadUtils::DisplayMessage("Communications", "Error");
		}

	}

	//this sends both batch inquiries and batch closes depending on the TransactionCode of the message
	BOOL SendBatchMessage(TransStruct &tsTransaction, ResponseStruct &Response) {

		//let's setup our transaction
		Paymentech::IIPOSTransactionPtr pPayTechTrans;
		Paymentech::IIPOSResponsePtr pPayTechResponse;

		pPayTechTrans.CreateInstance(__uuidof(Paymentech::IPOSTransaction));

		long nErrorCode = 0;

		PaymentechUtils::InitTransaction(pPayTechTrans, tsTransaction);

		//now we have to fill the transaction with things specific to this payment
		//let's see what type we are doing

		CString strMessage;

		tsTransaction.nTransactionSequenceFlag = 1;
		tsTransaction.nBatchNumber = 0;
		tsTransaction.nBatchSequenceNumber = 0;
		tsTransaction.nBatchOffset = 0;
		tsTransaction.nTransactionCount = 0;
		tsTransaction.cyNetAmount = COleCurrency(0,0);
		tsTransaction.strSysInfo = "Practice07060708.04.0003";

		PaymentechUtils::InitTerminalMessage(strMessage, tsTransaction);

		//gather our batch message
		PaymentechUtils::GetTerminalBatchMessage(strMessage, tsTransaction);
		
#ifdef _DEBUG
		CFile flOut("Output.txt", CFile::modeCreate|CFile::modeWrite|CFile::shareCompat);
		flOut.Write((LPCTSTR)strMessage, strMessage.GetLength());

		PinPadUtils::LogCCProcess("Sending " + GetMessageFromType(tsTransaction.tcTransactionCode) + " to Paymentech: " + strMessage + "\r\n");
#endif

		VARIANT v;
		unsigned char HUGEP *pData;

		VariantInit(&v);
		v.vt = VT_ARRAY|VT_UI1;
		v.parray = SafeArrayCreateVector(VT_UI1, 0, strMessage.GetLength() + 1);

		SafeArrayAccessData(v.parray, (void HUGEP **)&pData);

		memcpy(pData, (LPCTSTR) strMessage, strMessage.GetLength() + 1);		

		SafeArrayUnaccessData(v.parray);
		
		pPayTechResponse = pPayTechTrans->Process(v);

		
		CString strGatewayError = (LPCTSTR)pPayTechResponse->GetGatewayError();
		CString strHTTPError = (LPCTSTR)pPayTechResponse->GetHTTPError();

		_variant_t varResponse = pPayTechResponse->Data;

	
		if (pPayTechResponse->error != 0 ) {

			nErrorCode = pPayTechResponse->error;
		}
		else if (strGatewayError != "") {
			nErrorCode = atoi(strGatewayError);
		}
		else if (strHTTPError != "") {
			nErrorCode = atoi(strHTTPError);
		}

		if (nErrorCode != 0 ) {
			//CString strError;
			//strError.Format("Error %li Encountered; Description: " + pPayTechResponse->ErrorString, nErrorCode);
			//AfxMessageBox(strError);
			return FALSE;
		}
		HandleResult(_ZFN(tsTransaction.nTerminalNumber, 3, "Terminal Number"), pPayTechResponse->Data, Response, tsTransaction.tcTransactionCode, tsTransaction.lcLangCode);

		return TRUE;
	}


	CString GetNameFromType(CString strPaymentType) {

		strPaymentType.TrimLeft();
		strPaymentType.TrimRight();
		
		if (strPaymentType == "CR") {
			return "Credit Cards";
		}
		else if (strPaymentType == "DB") {
			return "Debit Cards";
		}
		else if (strPaymentType == "AE") {
			return "American Express";
		}
		else if (strPaymentType == "EB") {
			ASSERT(FALSE);
			return "Electronic Benefits Cards";
		}

		ASSERT(FALSE);
		return "";

	}

	BOOL SendVoidMessage(TransStruct &tsTransaction, ResponseStruct &Resp) {

		//let's setup our transaction
		Paymentech::IIPOSTransactionPtr pPayTechTrans;
		Paymentech::IIPOSResponsePtr pPayTechResponse;

		tsTransaction.nTransactionSequenceFlag = 1;
		

		pPayTechTrans.CreateInstance(__uuidof(Paymentech::IPOSTransaction));

		long nErrorCode = 0;

		PaymentechUtils::InitTransaction(pPayTechTrans, tsTransaction);

		//now we have to fill the transaction with things specific to this payment
		//let's see what type we are doing

		CString strMessage;

		PaymentechUtils::InitTerminalMessage(strMessage, tsTransaction);

		//gather our batch message
		PaymentechUtils::GetTerminalVoidMessage(strMessage, tsTransaction);				
		
#ifdef _DEBUG
		CFile flOut("Output.txt", CFile::modeCreate|CFile::modeWrite|CFile::shareCompat);
		flOut.Write((LPCTSTR)strMessage, strMessage.GetLength());

		PinPadUtils::LogCCProcess("Sending " + GetMessageFromType(tsTransaction.tcTransactionCode) + " to Paymentech: " + strMessage + "\r\n");
#endif

		VARIANT v;
		unsigned char HUGEP *pData;

		VariantInit(&v);
		v.vt = VT_ARRAY|VT_UI1;
		v.parray = SafeArrayCreateVector(VT_UI1, 0, strMessage.GetLength() + 1);

		SafeArrayAccessData(v.parray, (void HUGEP **)&pData);

		memcpy(pData, (LPCTSTR) strMessage, strMessage.GetLength() + 1);		

		SafeArrayUnaccessData(v.parray);
		
		pPayTechResponse = pPayTechTrans->Process(v);

		
		CString strGatewayError = (LPCTSTR)pPayTechResponse->GetGatewayError();
		CString strHTTPError = (LPCTSTR)pPayTechResponse->GetHTTPError();

		_variant_t varResponse = pPayTechResponse->Data;

	
		if (pPayTechResponse->error != 0 ) {

			nErrorCode = pPayTechResponse->error;
		}
		else if (strGatewayError != "") {
			nErrorCode = atoi(strGatewayError);
		}
		else if (strHTTPError != "") {
			nErrorCode = atoi(strHTTPError);
		}

		if (nErrorCode != 0 ) {
			//CString strError;
			//strError.Format("Error %li Encountered; Description: " + pPayTechResponse->ErrorString, nErrorCode);
			//AfxMessageBox(strError);
			return FALSE;
		}
		HandleResult(_ZFN(tsTransaction.nTerminalNumber, 3, "Terminal Number"), pPayTechResponse->Data, Resp, tsTransaction.tcTransactionCode, tsTransaction.lcLangCode);

		return TRUE;
	}

	BOOL SendCurrentKeyRequest() {

		TransStruct tsMessage;
		ResponseStruct rsResponse;
		tsMessage.tcTransactionCode = tcCurrentKeyRequest;

		if (! PinPadUtils::FillSerialInformation(tsMessage.strSerialApplicationName,
			tsMessage.strCoreLibraryVersion,
			tsMessage.strSecurityLibraryVersion,
			tsMessage.strPinPadSerialNumber,
			tsMessage.strROMVersion,
			tsMessage.strRAMVersion)) {

			MsgBox("Could not obtain serial number from pinpad");
			return FALSE;
		}

		long nLang = GetRemotePropertyInt("CCProcessingLanguage", 0, 0, "<None>", TRUE);
		if (nLang == 0) {
			tsMessage.lcLangCode = lcEnglish;
		}
		else {
			tsMessage.lcLangCode = lcFrench;
		}

		//let's setup our transaction
		Paymentech::IIPOSTransactionPtr pPayTechTrans;
		Paymentech::IIPOSResponsePtr pPayTechResponse;

		tsMessage.nTransactionSequenceFlag = 1;
		
		pPayTechTrans.CreateInstance(__uuidof(Paymentech::IPOSTransaction));

		long nErrorCode = 0;

		PaymentechUtils::InitTransaction(pPayTechTrans, tsMessage);

		//now we have to fill the transaction with things specific to this payment
		//let's see what type we are doing

		CString strMessage;

		//set up the message
		InitTerminalMessage(strMessage, tsMessage);

		//gather our batch message
		GetTerminalCurrentKeyRequest(strMessage, tsMessage);
		
#ifdef _DEBUG
		CFile flOut("Output.txt", CFile::modeCreate|CFile::modeWrite|CFile::shareCompat);
		flOut.Write((LPCTSTR)strMessage, strMessage.GetLength());

		PinPadUtils::LogCCProcess("Sending " + GetMessageFromType(tsMessage.tcTransactionCode) + " to Paymentech: " + strMessage + "\r\n");
#endif

		VARIANT v;
		unsigned char HUGEP *pData;

		VariantInit(&v);
		v.vt = VT_ARRAY|VT_UI1;
		v.parray = SafeArrayCreateVector(VT_UI1, 0, strMessage.GetLength() + 1);

		SafeArrayAccessData(v.parray, (void HUGEP **)&pData);

		memcpy(pData, (LPCTSTR) strMessage, strMessage.GetLength() + 1);		

		SafeArrayUnaccessData(v.parray);
		
		pPayTechResponse = pPayTechTrans->Process(v);

		
		CString strGatewayError = (LPCTSTR)pPayTechResponse->GetGatewayError();
		CString strHTTPError = (LPCTSTR)pPayTechResponse->GetHTTPError();

		_variant_t varResponse = pPayTechResponse->Data;

	
		if (pPayTechResponse->error != 0 ) {

			nErrorCode = pPayTechResponse->error;
		}
		else if (strGatewayError != "") {
			nErrorCode = atoi(strGatewayError);
		}
		else if (strHTTPError != "") {
			nErrorCode = atoi(strHTTPError);
		}

		if (nErrorCode != 0 ) {
			//CString strError;
			//strError.Format("Error %li Encountered; Description: " + pPayTechResponse->ErrorString, nErrorCode);
			//AfxMessageBox(strError);
			return FALSE;
		}
		HandleResult(_ZFN(tsMessage.nTerminalNumber, 3, "Terminal Number"), pPayTechResponse->Data, rsResponse, tsMessage.tcTransactionCode, tsMessage.lcLangCode);

		return TRUE;

	}




		


}