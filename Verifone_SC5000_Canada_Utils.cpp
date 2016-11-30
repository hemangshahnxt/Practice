//Verifone_SC5000_Utils.cpp
#include "stdafx.h"
#include "Verifone_SC5000_Canada_Utils.h"
#include "PinPadUtils.h"
#include "GlobalUtils.h"

// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - All CString operators with 0x ints, are now explicityly cast as chars


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Verifone_SC5000_Canada_Utils {

	BOOL SendInitMessage() {

		//Build the initialize message
		CString strMessage;
		
		strMessage += (char)0x02;
		
		//we want the credit/debit application
		strMessage += 'S';

		strMessage += (char)0x03;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);

	}



	BOOL SendQueryAndInitMessage() {

		CString strMessage;

		strMessage += STARTTEXT;

		strMessage += "S01";

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);


	}


	BOOL ClearSwipe() {

		CString strMessage;

		//strMessage += STARTTEXT;

		strMessage += (char)0x18;

		//strMessage += ENDTEXT;

		//strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);


	}


	BOOL SendPrinterDownMessage() {

		CString strMessage;

		strMessage += STARTTEXT;
		strMessage += 'S';
		strMessage += "10";
		
		strMessage += "1";//GetLanguageCode();

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);


	}


	BOOL SendDisplayMessage(CString strTop, CString strBottom) {

		CString strMessage;

		strMessage += STARTTEXT;
		strMessage += "S09";

		strMessage += strTop;
		strMessage += FIELDSEPARATOR;

		strMessage += strBottom;

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);
	}


	BOOL SendTrack2Message() {

		CString strMessage;

		strMessage += STARTTEXT;
		strMessage += "S20";

		//wait 30 seconds for swipe
		strMessage += "030";
		
		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);
	}

	


	BOOL SendInteracRequestMessage(long nTransID, COleCurrency cyTransAmount, CString strAccountNumber, CString strMerchantNumber,
		CString strTerminalNumber, CString strTransactionCode, CString strSequenceNumber){

		CString strMessage;

		strMessage += STARTTEXT;
		strMessage += "S70";

		//Language Code
		strMessage += "0";

		strMessage += AsString(nTransID);

		CString strAmount;
		strAmount = cyTransAmount.Format(0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));

		if (strAmount.Find(".") == -1) {
			strAmount += ".00";
		}

		strMessage += strAmount;

		strMessage += FIELDSEPARATOR;

		strMessage += strAccountNumber;

		strMessage += FIELDSEPARATOR;

		//PNS Merchant ID
		strMessage += strMerchantNumber;

		//PSN Terminal ID
		strMessage += strTerminalNumber;

		//transaction Code
		strMessage += strTransactionCode;

		//PAN Number
		//this is the account number left justified and space filled
		CString strPanNumber;
		strPanNumber = strAccountNumber;
		while (strPanNumber.GetLength() < 19) {
			strPanNumber += ' ';
		}
		strMessage += strPanNumber;

		strMessage += strAmount;

		//sequence Number
		strMessage += strSequenceNumber;

		strMessage += "*D";
		
		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);
	}

	BOOL SendSerialNumber() {

		CString strMessage;

		strMessage += STARTTEXT;
		strMessage += "S95";

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);

	}

	BOOL ParseSerialNumberResponse(CString strWholeMessage,
						CString &strApplicationName, CString &strCoreLibraryVersion, CString &strSecurityLibraryVersion, 
						CString &strPinPadSerialNumber, CString &strROMVersion, CString &strRAMVersion) {

		//first take off the starttext
		ASSERT(strWholeMessage.GetAt(0) == STARTTEXT);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 1);

		//the next 2 characters are the response code
		if (strWholeMessage.Left(2) == "01") {
			//it failed
			return FALSE;
		}

		//take off the 00 successful response code
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 2);

		//if we got here we are successful
		//next we have the application name
		//in 0 - 15 characters, followed by a field separator
		long nResult = strWholeMessage.Find(FIELDSEPARATOR);
		ASSERT(nResult != -1);

		strApplicationName = strWholeMessage.Left(nResult);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - (nResult + 1));

		//next is ust a bunch of zeros that we don't need and a field separator 
		nResult = strWholeMessage.Find(FIELDSEPARATOR);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - (nResult + 1));

		//the next 9 characters are the Core Library Version
		strCoreLibraryVersion = strWholeMessage.Left(9);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 9);

		//the next 9 are the Security Library Version
		strSecurityLibraryVersion = strWholeMessage.Left(9);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 9);

		//the next 16 characters are the serial number left justified with spaces,
		//followed by a field separator
		nResult = strWholeMessage.Find(FIELDSEPARATOR);
		strPinPadSerialNumber = strWholeMessage.Left(nResult);
		strPinPadSerialNumber.TrimRight();
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - (nResult + 1));

		//the next 9 characters are the ROM version
		strROMVersion = strWholeMessage.Left(9);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 9);

		//and the next 9 are the RAM version
		strRAMVersion = strWholeMessage.Left(9);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 9);

		return TRUE;
	}


	BOOL GetTrack2InformationFromWholeSwipe(CString strSwipe, CString &strTrack2, CString &strTrack1, CString &strTrack3) {

		//the documentation indicates that in Version 1 its would always be just track 2, but in 
		// version 2, it'd be <STX><response Code><Track2><FS><Track1><FS><Track3><ETX><LRC>

		//first, get the starttext off
		strSwipe = strSwipe.Right(strSwipe.GetLength() - 1);

		//now get the response code
		CString strResponse = strSwipe.Left(2);
		if (strResponse == "01") {

			//it was unsuccessful, send back the response code
			strTrack2 = strResponse;
			return FALSE;
		}
		else if (strResponse == "02") {

			//it timed out
			strTrack2 = "TIME_OUT";
			return FALSE;
		}
		else if (strResponse  == "03") {

			strTrack2 = "CANCELLED";
			return FALSE;
		}
		
		//if we got here we are good to go
		
		//take off the response code
		strSwipe = strSwipe.Right(strSwipe.GetLength() - 2);

		//look for the field separator
		long nResult = strSwipe.Find(FIELDSEPARATOR);
		strTrack2 = strSwipe.Left(nResult);
		strSwipe = strSwipe.Right(strSwipe.GetLength() - (nResult + 1));

		
		//take off the start character and end character
		if (strTrack2.GetAt(0) == ';' || strTrack2.GetAt(0) == '%') {

			//take it off
			strTrack2 = strTrack2.Right(strTrack2.GetLength() - 1);
		}
		else {
			ASSERT(FALSE);
		}

		TRACE("track 2: " + strTrack2 + "\r\n");

		//take off the last character
		strTrack2 = strTrack2.Left(strTrack2.GetLength() - 1);

		TRACE("Track 2 Trimmed: " + strTrack2 + "\r\n");

		//look for the next field separator
		nResult = strSwipe.Find(FIELDSEPARATOR);
		strTrack1 = strSwipe.Left(nResult);
		strSwipe = strSwipe.Right(strSwipe.GetLength() - (nResult + 1));

		//take off the start character and end character
		if (strTrack1.GetAt(0) == ';' || strTrack1.GetAt(0) == '%') {

			//take it off
			strTrack1 = strTrack1.Right(strTrack1.GetLength() - 1);
		}
		else {
			ASSERT(FALSE);
		}

		TRACE("track 1: " + strTrack1 + "\r\n");

		//take off the last character
		strTrack1 = strTrack1.Left(strTrack1.GetLength() - 1);

		TRACE("Track 1 Trimmed: " + strTrack1 + "\r\n");

		//and track 3
		//credit cards don't have 3 tracks
		//strTrack3 = strSwipe;	
		//TRACE(strTrack3);

		return TRUE;
	}


	BOOL ParseInteracResponse(CString strWholeMessage, long &nAccountType, CString &strMACBlock, 
		CString &strPINBlock, CString &strPinPadSerialNumber) {


		//the format is: <STX><response code><response data><ETX><LRC>

		//the response data is: AccountType, MAC block, PIN black, PIN PAD serial number

		
		//first, take off the STX
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 1);

		//now for the response code
		CString strRespCode = strWholeMessage.Left(2);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 2);
		if (strRespCode == "01") {
			strMACBlock = "Unsuccessful";
			return FALSE;
		}
		else if (strRespCode == "02") {
			strMACBlock = "TIME_OUT";
			return FALSE;
		}
		else if (strRespCode == "03") {
			strMACBlock == "CANCELLED";
			return FALSE;
		}
		else if (strRespCode == "04") {
			strMACBlock = "CORR key pressed";
			return FALSE;
		}
		else if (strRespCode == "06") {
			strMACBlock = "Invalid Account Number Length";
			return FALSE;
		}
		else if (strRespCode == "79") {
			strMACBlock = "No Keys";
			return FALSE;
		}

		//if we got here, we succeded!!

		//account type
		//1 for checking, 2 for savings
		nAccountType = atoi(strWholeMessage.Left(1));
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 1);

		//MAC Block
		//8 characters
		strMACBlock = strWholeMessage.Left(8);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 8);

		//PIN Block
		//16 characters
		strPINBlock = strWholeMessage.Left(16);
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 16);

		//Pin Pad Serial Number
		strPinPadSerialNumber = strWholeMessage.Left(16);
		strPinPadSerialNumber.TrimLeft();
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 16);
		
		return TRUE;


	}


	BOOL SendConfirmAmountTipCashSurcharge(long lcCode, COleCurrency cyTransAmount, BOOL bTipFlag, BOOL bCashBackFlag, BOOL bSurchargeFlag, COleCurrency cySurchargeAmount) {

		CString strMessage;

		strMessage += STARTTEXT;

		strMessage += "S21";

		strMessage += AsString((long)lcCode);

		CString strAmount;
		strAmount = cyTransAmount.Format(0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));

		if (strAmount.Find(".") == -1) {
			strAmount += ".00";
		}

		strMessage += strAmount;

		//field separator
		strMessage += FIELDSEPARATOR;

		if (bTipFlag) {
			strMessage += "1";
		}
		else {
			strMessage += "0";
		}

		if (bCashBackFlag) {
			strMessage += "1";
		}
		else {
			strMessage += "0";
		}

		if (bSurchargeFlag) {
			strMessage += "1";
		}
		else {
			strMessage += "0";
		}

		strMessage += FIELDSEPARATOR;

		strAmount = cySurchargeAmount.Format(0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));

		if (strAmount.Find(".") == -1) {
			strAmount += ".00";
		}

		strMessage += strAmount;

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);
	}
	
	
	BOOL ParseConfirmAmountTipCashSurcharge(CString strWholeMessage, BOOL bTipsFlag, BOOL bCashBackFlag,
		//out
		COleCurrency &cyTipAmount, COleCurrency &cyCashBackAmount, BOOL &bSurchargeAccepted, CString &strMessage) {

		//first take off the start text character
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 1);

		CString strResponse = strWholeMessage.Left(2);
		if (strResponse == "01") {
			strMessage = "Confirm Amount/Tip/Surcharge Unsuccessful";
			return FALSE;
		}
		else if (strResponse == "02") {
			strMessage = "Confirm Amount/Tip/Surcharge Timed Out";
			return FALSE;
		}
		else if (strResponse == "03") {
			strMessage = "Confirm Amount/Tip/Surcharge Cancelled";
			return FALSE;
		}
		else if (strResponse == "04") {
			strMessage = "Confirm Amount/Tip/Surcharge Correction Key Pressed";
			return FALSE;
		}

		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 2);

		//if we got here, we succeeded
		if (bTipsFlag) {

			//we need to look for the tip amount
			long nResult = strWholeMessage.Find(".");
			
			//we found the decimal, and there are always 2 places past the decimal
			nResult += 2;
			CString strTmp = strWholeMessage.Left(nResult);
			cyTipAmount.ParseCurrency(strTmp);

			strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - (nResult + 1));
		}


		if (bCashBackFlag) {

			//we need to look for the tip amount
			long nResult = strWholeMessage.Find(".");
			
			//we found the decimal, and there are always 2 places past the decimal
			nResult += 2;
			CString strTmp = strWholeMessage.Left(nResult);
			cyCashBackAmount.ParseCurrency(strTmp);

			strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - (nResult + 1));
		}

		//now the surcharge
		long nSurcharge = atoi(strWholeMessage.Left(1));

		if (nSurcharge == 0) {
			bSurchargeAccepted = FALSE;
		}
		else if (nSurcharge == 1) {
			bSurchargeAccepted = TRUE;
		}

		return TRUE;
	}

	BOOL PerformInteracResponseAnalysis(long lcLangCode, CString strBalanceEncryption, CString strMessageKey, CString strTPK, CString strTAK, CString strTopLine, CString strBottomLine, CString strMACData) {
		
		CString strMessage;

		//start of text
		strMessage += STARTTEXT;

		strMessage += "S71";

		strMessage += AsString(lcLangCode);

		strMessage += strMACData;

		strMessage += FIELDSEPARATOR;

		strMessage += strBalanceEncryption;

		strMessage += FIELDSEPARATOR;

		strMessage += strMessageKey;

		strMessage += FIELDSEPARATOR;

		strMessage += strTPK;

		strMessage += FIELDSEPARATOR;

		strMessage += strTAK;

		strMessage += FIELDSEPARATOR;

		strMessage += strTopLine;

		strMessage += FIELDSEPARATOR;

		strMessage += strBottomLine;

		strMessage += FIELDSEPARATOR;

		CString strDataBuffer;
		
		strMessage +=  strDataBuffer;

		strMessage += ENDTEXT;

		strMessage += CalcLRC(strMessage);

		return PinPadUtils::WriteToPinPad(strMessage);

	}

	BOOL ParseInteracResponseAnalysis(CString strWholeMessage, CString &strReturnMessage) {

		//first take off the start text character
		strWholeMessage = strWholeMessage.Right(strWholeMessage.GetLength() - 1);

		CString strRespCode = strWholeMessage.Left(2);

		if (strRespCode == "01") {
			strReturnMessage = "Unsuccessful";
			return FALSE;
		}
		else if (strRespCode == "05") {
			strReturnMessage = "Wrong Prefix";
			return FALSE;
		}
		else if (strRespCode == "07") {
			strReturnMessage = "MAC did not verify";
			return FALSE;
		}
		else if (strRespCode == "09") {
			strReturnMessage = "No MAC block";
		}
		else if (strRespCode == "11") {
			strReturnMessage = "Bad Key";
			return FALSE;
		}
		else if (strRespCode == "16") {
			strReturnMessage = "New Working Key load failed, successfully reverted to previous Working Key set";
			return FALSE;
		}
		else if (strRespCode == "21") {
			strReturnMessage = "Cannot calculate MAC";
			return FALSE;
		}
		else if (strRespCode == "23") {
			strReturnMessage = "Invalid Field Length";
			return FALSE;
		}
		else if (strRespCode == "98") {
			strReturnMessage = "New working keys load failed, Cannot revet to previos working keys";
			return FALSE;
		}

		//if we got here, we succeeded
		return TRUE;
				

	}
			
	
	char CalcLRC(CString strMessage) {

		char chLRC = (char)0x00;

		//take off the STX char
		for (int i = 1; i < strMessage.GetLength(); i++) {
			chLRC ^= strMessage.GetAt(i);
		}
		
		return chLRC;
	}
}