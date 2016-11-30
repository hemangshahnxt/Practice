//Verifone_SC5000_Utils.h
#include "PinPadUtils.h"

#ifndef _VERIFONE_SC5000_CANADA_UTILS_H_
#define _VERIFONE_SC5000_CANADA_UTILS_H_

#pragma once




namespace Verifone_SC5000_Canada_Utils {
	// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - All CString operators with 0x ints, are now explicityly cast as chars
	#define STARTTEXT (char)0x02
	#define ENDTEXT (char)0x03
	#define FIELDSEPARATOR (char)0x1C


	char CalcLRC(CString strMessage);
	BOOL SendInitMessage();
	BOOL SendQueryAndInitMessage();
	BOOL ClearSwipe();
	BOOL SendDisplayMessage(CString strTop, CString strBottom);
	BOOL SendPrinterDownMessage();
	BOOL SendTrack2Message();
	
	BOOL SendSerialNumber();

	BOOL ParseSerialNumberResponse(CString strWholeMessage,	CString &strApplicationName, CString &strCoreLibraryVersion, CString &strSecurityLibraryVersion, 
		 CString &strPinPadSerialNumber, CString &strROMVersion, CString &strRAMVersion);

	BOOL GetTrack2InformationFromWholeSwipe(CString strSwipe, CString &strTrack2, CString &strTrack1, CString &strTrack3);


	BOOL SendInteracRequestMessage(long nTransID, COleCurrency cyTransAmount, CString strAccountNumber, CString strMerchantNumber,
		CString strTerminalNumber, CString strTransactionCode, CString strSequenceNumber);
	BOOL ParseInteracResponse(CString strWholeMessage,long &nAccountType, CString &strMACBlock, CString &strPINBlock, CString &strPinPadSerialNumber);


	BOOL SendConfirmAmountTipCashSurcharge(long lcCode, COleCurrency cyTransAmount, BOOL bTipFlag, BOOL bCashBackFlag, BOOL bSurchargeFlag, COleCurrency cySurchargeAmount);
	BOOL ParseConfirmAmountTipCashSurcharge(CString g_strLastMessageRead,
		BOOL bTipsFlag, BOOL bCashBackFlag,
		//out
		COleCurrency &cyTipAmount, COleCurrency &cyCashBackAmount, BOOL &bSurchargeAccepted, CString &strMessage);
	
	BOOL PerformInteracResponseAnalysis(long lcLangCode, CString strBalanceEncryption, CString strMessageKey, CString strTPK, CString strTAK, CString strTopLine, CString strBottomLine, CString strMACData);
	BOOL ParseInteracResponseAnalysis(CString strWholeMessage, CString &strReturnMessage);

}


#endif