//PinPadUtils.h

#ifndef _PINPAD_API_H_
#define _PINPAD_API_H_

#pragma once



	#define WM_PINPAD_OPEN_FAILED		WM_USER + 30101
	// wParam = 0
	// lParam = 0 if CreateFile failed
	//			1 if SetCommState failed

	#define WM_PINPAD_SCAN			WM_USER + 30102
	// wParam = size of string
	// lParam = pointer to BSTR of characters

	#define WM_PINPAD_OVERFLOW		WM_USER + 30103
	// wParam = size of string
	// lParam = pointer to BSTR of characters


	#define WM_PINPAD_MESSAGE_INITIALIZE_DONE  WM_USER + 30104
	//wParam size of string
	//lParam = pointer to BSTR of characters

	#define WM_PINPAD_MESSAGE_READ_TRACK_DONE  WM_USER + 30105
	//wParam size of string
	//lParam = pointer to BSTR of characters

	#define WM_PINPAD_MESSAGE_CLEAR_DONE WM_USER + 30106
	//wParam size of string
	//lParam = pointer to BSTR of characters

	#define WM_PINPAD_MESSAGE_INTERAC_DEBIT_DONE WM_USER + 30107
	//wParam size of string
	//lParam = pointer to BSTR of characters

	#define WM_PINPAD_MESSAGE_UNKNOWN_READ WM_USER + 30108
	//wParam size of string
	//lParam = pointer to BSTR of characters


namespace PinPadUtils {

	enum DeviceType {
		dtVerifoneSCCanada5000,
		dtIngenicoI3070, /*we don't actually have this one yet*/

		/*todo: add more!*/
	};

	enum MessageType {
		mtNone,
		mtInitializeCredDebApp,
		mtReadTrackInformation,
		mtClearReadTrackInformation,
		mtInteracDebit,
		mtDisplayMessage,
		mtGetSerialNumber,
		mtSendInteracRequest,
		mtSendConfirmAmountTipCashSurcharge,
		mtSendInteracResponseAnalysis,
	};

	enum LanguageCode {
		lcEnglish = 0,
		lcFrench  = 1,
	};	


	BOOL InitializePinPad(HWND hWnd, const char* szComPort = "COM1");
	DWORD WINAPI Read_Thread( LPVOID lpParam );
	void PinPadClose();
	BOOL WriteToPinPad(CString strMessage);
	char CalcLRC(CString strMessage);


	CString GetDescFromMessageType(MessageType msgType);
	CString GetCustomerSwipe();
	void ClearCustomerSwipe();
	BOOL IsInitialized();
	BOOL SendInitMessage();
	void DisplayMessage(CString strTop, CString strBottom);
	BOOL FillSerialInformation(CString &strApplicationName, CString &strCoreLibraryVersion, CString &strSecurityLibraryVersion, 
		CString &strPinPadSerialNumber, CString &strROMVersion, CString &strRAMVersion);

	BOOL GetTrack2InformationFromWholeSwipe(CString strWholeSwipe, CString &strTrack2, CString &strTrack1, CString &strTrack3);

	BOOL SendInteracRequest(IN long nTransID, IN COleCurrency cyTransAmount, IN CString strAccountNumber, 
		//information for MACing
		IN CString strMerchantNumber, IN CString strTerminalNumber, IN CString strTransactionCode, 
		IN CString strSequenceNumber, 		
		//out parameters
		OUT long &nAccountType, OUT CString &strMACBlock, OUT CString& strPINBlock, OUT CString &strPinPadSerialNumber);

	BOOL SendConfirmAmountTipCashSurcharge(long lcCode, COleCurrency cyTransAmount, BOOL bTipFlag, BOOL bCashBackFlag, BOOL bSurchargeFlag, COleCurrency cySurchargeAmount,
		//out parameters
		OUT COleCurrency &cyTipAmount, OUT COleCurrency &cyCashBackAmount, BOOL &bSurchargeAccepted, CString &strReturnMessage);

	BOOL PerformInteracResponseAnalysis(long lcLangCode, CString strBalanceEncryption, CString strMessageKey, CString strTPK, CString strTAK, CString strTopLine, CString strBottomLine, CString strMACData, CString &strReturnMessage);

	void ParseTrackInformation(CString strTrack1, CString strTrack2, CString strTrack3, CString &strAccountNumber, CString &strFirstName, CString & strMiddleName,
		CString &strLastName, CString &strSuffix, CString &strTitle, CString &strCardType, CString &strCD, COleDateTime &dtExpireDate);

	BOOL GetStringField(IN CString strString, IN long nFieldStart, IN BOOL bFixedLength, IN CString strFixedLengthOrFieldSeparator, 
		OUT CString &strField, OUT long &nNextFieldStart);

	void GetCreditCardTypeFromNumber(CString strAccountNumber, CString &strDescription, CString &strCD);


	void LogCCProcess(LPCTSTR strToLog);
	CFile *GetLogFileCCTransactions();
	void LogCCProcessLockAndClose();
		
}


#endif