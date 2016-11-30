// OPOSPrinterDevice.cpp : implementation file
//

#include "stdafx.h"
#include "OPOSPrinterDevice.h"
#include "OPOSPtr.h"
#include "OPOS.h"

#include <boost/bind.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2007-05-09 13:01) - PLID 9802 - POS functionality for Receipt Printer


// (a.walling 2011-03-21 17:32) - PLID 42931 - Significant changes here, too numerous to label individually.
// Basically, use POSPrinterAccess to claim and release the printer.
// Some useful logging information is added, as well as some more error checking

void POSPrinterAccess::EnsureClaim()
{
	try {
		if (!m_pDevice) {
			CMainFrame* pMainFrame = GetMainFrame();
			if (!pMainFrame) {
				ThrowNxException("No mainframe available!");
			}

			m_pDevice = pMainFrame->GetOPOSPrinterDevice();
		}

		if (!m_pDevice) {
			return;
		}

		// (b.savon 2014-09-05 09:52) - PLID 59621 - If we can't acquire the claim, return
		if (!m_pDevice->AcquireClaim()){ // may throw if failure
			return;
		}
		m_bIsValid = true;

	} NxCatchAllThread(__FUNCTION__);
}

void POSPrinterAccess::ReleaseClaim()
{
	try {
		// (a.walling 2011-06-09 17:18) - PLID 42931 - Don't try to release if we never claimed it to begin with.
		if (!m_pDevice || !m_bIsValid) {
			return;
		}

		m_pDevice->ReleaseClaim();
		m_bIsValid = false;

	} NxCatchAllThread(__FUNCTION__);
}

//

template<typename T>
BOOL RetryOPOSBusyOrFail(COPOSPrinterDevice* pPOSPrinter, const CString& strMessage, T& OPOS_call)
{
	// (a.walling 2011-04-28 10:02) - PLID 43492
	
	{
		static const DWORD dwWaitIdleMax = 4000;

		DWORD dwWaitIdle = 30;
		DWORD dwWaitIdleTicks = GetTickCount();

		// first poll the device state
		long state;
		do {
			state = pPOSPrinter->GetState();

			DWORD dwAttemptTicks = GetTickCount();

			switch (state) {
				case OPOS_S_CLOSED:
					pPOSPrinter->CheckError(OPOS_E_CLOSED, strMessage, true);
					return FALSE;
				case OPOS_S_IDLE:
					// we are good to go!
					break;
				case OPOS_S_BUSY:
					if ((dwAttemptTicks - dwWaitIdleTicks) < dwWaitIdleMax) {
						Sleep(dwWaitIdle);
						dwWaitIdle = min(dwWaitIdle * 2, 1000);
					} else {
						pPOSPrinter->CheckError(OPOS_E_BUSY, strMessage, true);
						return FALSE;
					}
					break;
				case OPOS_S_ERROR:
				default:
					pPOSPrinter->CheckError(0, strMessage, true, true);
					return FALSE;
					break;
			}
		} while (state == OPOS_S_BUSY);
	}
	
	{
		static const DWORD dwWaitCallMax = 2000;

		DWORD dwWaitCall = 30;
		DWORD dwWaitCallTicks = GetTickCount();

		for (;;) {
			long result = OPOS_call();

			DWORD dwAttemptTicks = GetTickCount();
			
			bool bShowMessage = true;
			bool bRetry = true;
			if (result == OPOS_E_BUSY) {
				if ((dwAttemptTicks - dwWaitCallTicks) < dwWaitCallMax) {
					bShowMessage = false;
				} else {
					bRetry = false;
				}
			} else {
				bRetry = false;
			}
			
			if (!pPOSPrinter->CheckError(result, strMessage, bShowMessage)) {

				if (bRetry && (result == OPOS_E_BUSY)) {
					Sleep(dwWaitCall);
					dwWaitCall = min(dwWaitCall * 2, 1000);
				} else {
					return FALSE;
				}
			} else {
				return TRUE;
			}
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// COPOSPrinterDevice

COPOSPrinterDevice::COPOSPrinterDevice(CWnd *pParentWnd)
	: m_nClaimCount(0)
	, m_pParentWnd(pParentWnd)
{
}

COPOSPrinterDevice::~COPOSPrinterDevice()
{
}

// (b.savon 2014-09-05 09:53) - PLID 59621 - Return success or failure and throws exceptions
bool COPOSPrinterDevice::AcquireClaim() 
{
	LogDetail("Acquire OPOS Printer device claim; previous ref count %li", m_nClaimCount);
	
	if (!m_pPrinter) {
		ThrowNxException("No OposPOSPrinter interface!!");
	}

	for (;;) // returned or exception thrown
	{
		CString strError;
		try {
			AttemptClaim();
			m_nClaimCount++;
			return true;
		} catch (CException* e) {
			strError = FormatError(e, "%s", "An error occurred while attempting to claim the POS Printer!");
			e->Delete();
		} catch (_com_error& e) {
			strError = FormatError(e, "%s", "An error occurred while attempting to claim the POS Printer!");
		}

		CString strFinalError;
		strFinalError.Format("An error occurred while attempting to claim the POS Printer: %s", strError);

		LogDetail("%s", strFinalError);

		// (b.savon 2014-09-05 09:53) - PLID 59621 - Don't throw an exception here, give the user a clean error after the full detailed exception message to retry.
		if (IDRETRY != AfxMessageBox(strFinalError, MB_RETRYCANCEL, 0)) {
			AfxMessageBox(
				"Nextech was unable to connect to the POS printer, please make sure it is connected and installed properly.  If the problem continues try restarting your computer.  If you still have problems call Nextech for support.", 
				MB_ICONINFORMATION|MB_OK, 
				0
			);
			return false;
		}
		
		LogDetail("Retrying...");
		Sleep(2000);
	}	
}

void COPOSPrinterDevice::AttemptClaim()
{
	if (!m_pPrinter) {
		ThrowNxException("No OposPOSPrinter interface!!");
	}

	//LogDetail("AttemptClaim: Checking claimed...");
	if (!m_pPrinter->Claimed) {				
		//LogDetail("Claiming...");
		
		// (a.walling 2011-06-09 17:21) - PLID 42931 - Apparently this is TOO LONG for some printers.
		// (a.walling 2011-06-09 17:21) - PLID 42931 - Looks like it's time for another preference
		long nClaimTimeoutWait = GetPropertyInt("OPOSPrinter_ClaimTimeout", 10000);
		long nClaimAttempts = 0;

		DWORD dwTicks = GetTickCount();
		long nElapsed = 0;

		long nResult = OPOS_E_FAILURE;
		do {
			nElapsed = GetTickCount() - dwTicks;
			nResult = m_pPrinter->ClaimDevice(500 + (nClaimAttempts++ * 125)); // basically we'll try at 500 (default) and then keep upping the timeout by 125
		} while ((nResult != OPOS_SUCCESS) && (nElapsed < nClaimTimeoutWait));

		if (nResult != OPOS_SUCCESS) {
			ThrowNxException(GetErrorString(nResult, "Error claiming receipt printer"));
		}
	} else {
		//LogDetail("Already claimed.");
	}

	//LogDetail("Checking enabled...");
	if (!m_pPrinter->DeviceEnabled) {
		//LogDetail("Enabling...");
		m_pPrinter->DeviceEnabled = VARIANT_TRUE;
	} else {				
		//LogDetail("Already enabled");
	}
	//LogDetail("Ready to go!");
}

void COPOSPrinterDevice::ReleaseClaim() // throws exceptions
{
	LogDetail("Delay release OPOS Printer device claim; previous ref count %li", m_nClaimCount);
	m_nClaimCount--;

	if (m_nClaimCount == 0) {
		AttemptRelease();
	}
}

void COPOSPrinterDevice::AttemptRelease()
{
	//TES 12/6/2007 - PLID 28192 - Disable the printer and release it, so that third-party applications can use it.
	if (!m_pPrinter) {
		return;
	}

	LogDetail("Releasing OPOS Printer device now... current ref count %li", m_nClaimCount);

	//LogDetail("Checking enabled...");
	if (m_pPrinter->DeviceEnabled) {
		LogDetail("Disabling...");
		m_pPrinter->DeviceEnabled = VARIANT_FALSE;
	}
	//LogDetail("Checking claimed...");
	if (m_pPrinter->Claimed) {				
		LogDetail("Releasing...");
		CheckError(m_pPrinter->ReleaseDevice(), "Error releasing device", false);
	}

	LogDetail("Released!");
}

/////////////////////////////////////////////////////////////////////////////
// COPOSPrinterDevice message handlers

HRESULT COPOSPrinterDevice::GetPrinterState() {
	if (!m_pPrinter) {
		return OPOS_S_ERROR;
	}
	
	return m_pPrinter->GetState();
}


BOOL COPOSPrinterDevice::CreateAndPrepareOPOSPrinterWindow() {

	try {		
		OposPOSPrinter_1_8_Lib::IOPOSPOSPrinterPtr pPrinter = GetControlUnknown();

		if (!pPrinter) {	
	//		if (CreateControl(__uuidof(OposPOSPrinter_1_11_Lib::OPOSPOSPrinter), NULL, WS_CHILD, CRect(0,0,0,0), m_pParentWnd, IDC_PRINTER_CTRL)) {
			if (!CreateControl(__uuidof(OposPOSPrinter_1_8_Lib::OPOSPOSPrinter), NULL, WS_CHILD, CRect(0,0,0,0), m_pParentWnd, IDC_PRINTER_CTRL)) {
				AfxMessageBox("Create and PrepareOPOSPrinterWindow FAILED");
				return FALSE;
			}

			pPrinter = GetControlUnknown();
		}

		if (pPrinter) {
			m_pPrinter = pPrinter;
			return TRUE;
		}


		AfxMessageBox("Created but could not access OPOSPrinterWindow");
		return FALSE;
	}NxCatchAll("Error in CreateAndPreparePrinterwindow");
	return FALSE;
}


BOOL COPOSPrinterDevice::CreateAndPrepareOPOSPrinter(CString strPrinterName) {

	try  {
		m_strPrinterName = strPrinterName;

		if (!CreateAndPrepareOPOSPrinterWindow()) {
			return FALSE;
		}

		if (!OpenPOSPrinter()) {
			return FALSE;
		}

		POSPrinterAccess printerAccess(this);
		
		//TES 12/6/2007 - PLID 28192 - Make sure we can claim the printer.
		if(!printerAccess) {
			//We couldn't, just give up.
			return FALSE;
		}

		return TRUE;
	}NxCatchAll("Error In CreateandPrepareOPOSPrinter");
	return FALSE;
}	

// (a.walling 2011-04-28 10:02) - PLID 43492
BOOL COPOSPrinterDevice::OpenPOSPrinter()
{
	if (!m_pPrinter) {
		return FALSE;
	}

	try {
		if (!CheckError(m_pPrinter->Open(_bstr_t(m_strPrinterName)), "Error opening specified printer")) {
			return FALSE;
		}

		return TRUE;
	}NxCatchAll("Error In OpenPOSPrinter");

	return FALSE;
}

BOOL COPOSPrinterDevice::ClosePOSPrinter() {
	try {

		if (!m_pPrinter) {
			return TRUE;
		}

		if (m_pPrinter->DeviceEnabled) {
			m_pPrinter->ClearOutput();
			m_pPrinter->PutDeviceEnabled(FALSE);
		}

		if (m_pPrinter->Claimed) {
			if (!CheckError(m_pPrinter->ReleaseDevice(), "Error releasing device"))
			{
				return FALSE;
			}
		}

		m_nClaimCount = 0;
		
		m_pPrinter->Close();

		// (a.walling 2011-03-23 13:20) - PLID 42931 - Don't actually close this here

		//CWnd *pWnd = GetDlgItem(IDC_PRINTER_CTRL);
		//if (pWnd) {
		//	pWnd->DestroyWindow();
		//}

		//m_pPrinter = NULL;

		return TRUE;
	}NxCatchAll("Error in ClosePOSPrinter");
	return FALSE;
}

// (a.walling 2011-04-28 10:02) - PLID 43492
BOOL COPOSPrinterDevice::ResetPOSPrinter()
{
	try {

		if (!ClosePOSPrinter()) {
			return FALSE;
		}

		if (!OpenPOSPrinter()) {
			return FALSE;
		}

		// (b.savon 2014-09-05 09:52) - PLID 59621 - If we can't acquire the claim, return
		if (!AcquireClaim()){
			return FALSE;
		}

		return TRUE;
	} NxCatchAll("Error resetting POS Printer");
	return FALSE;
}

BOOL COPOSPrinterDevice::PrintText(CString strTextToPrint)
{
	if (strTextToPrint.IsEmpty()) {
		return TRUE;
	}

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strTextToPrint.Replace("\r\n", "\n"); // this would not handle situations where a \n was appended to an existing \r, but 
	// I did not find any places where this would be the case. This is just a simple assertion reminder when debugging.
	ASSERT(-1 == strTextToPrint.Find("\r"));

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Replace unavailable chars with spaces
	for (int i = 0; i < strTextToPrint.GetLength(); i++)
	{
		if (!IsPrintable((unsigned char)strTextToPrint[i])) {
			//LogDetail("OPOSPrinterDevice warning: Unprintable character 0x%02x in '%s'", (int)strTextToPrint[i], strTextToPrint);
			strTextToPrint.SetAt(i, ' ');
		}
	}

	if (strTextToPrint[strTextToPrint.GetLength() - 1] != '\n') {
		strTextToPrint += "\n";
	}

	m_queuedOutput.push_back(Output::Text(strTextToPrint));

	return TRUE;
}

BOOL COPOSPrinterDevice::InternalPrintText(LPCTSTR szText)
{
	if (!m_pPrinter) {
		return FALSE;
	}
		
	//TES 12/6/2007 - PLID 28192 - We used to try to put the printer into AsyncMode here, but we didn't have any
	// particular reason to, and if we release the printer before any asynchronous operations (which include printing
	// and cutting, which is basically all we do) are completed, then they will never get done, and we will never
	// know that they weren't done.  So, we'll just leave the printer in its default synchronous mode.
	//pPrinter->PutAsyncMode(varTrue);

	// (a.walling 2011-04-28 10:02) - PLID 43492
	if (!RetryOPOSBusyOrFail(this, "Error printing text", boost::bind(&OposPOSPrinter_1_8_Lib::IOPOSPOSPrinter::PrintNormal, m_pPrinter.GetInterfacePtr(), PTR_S_RECEIPT, szText)))
	{
		LogDetail("Attempted to print text: %s", szText);

		return FALSE;
	}

	return TRUE;
}

BOOL COPOSPrinterDevice::PrintBitmap(CString strPath)
{
	m_queuedOutput.push_back(Output::Bitmap(strPath));
	return TRUE;
}

// (a.walling 2011-04-28 10:02) - PLID 43492
BOOL COPOSPrinterDevice::InternalPrintBitmap(LPCTSTR szPath)
{
	if (!m_pPrinter) {
		return FALSE;
	}
		
	//TES 12/6/2007 - PLID 28192 - We used to try to put the printer into AsyncMode here, but we didn't have any
	// particular reason to, and if we release the printer before any asynchronous operations (which include printing
	// and cutting, which is basically all we do) are completed, then they will never get done, and we will never
	// know that they weren't done.  So, we'll just leave the printer in its default synchronous mode.
	//pPrinter->PutAsyncMode(varTrue);
	
	// (a.walling 2011-04-28 10:02) - PLID 43492
	if (!RetryOPOSBusyOrFail(this, "Error printing bitmap", boost::bind(&OposPOSPrinter_1_8_Lib::IOPOSPOSPrinter::PrintBitmap, m_pPrinter.GetInterfacePtr(), PTR_S_RECEIPT, szPath, PTR_BM_ASIS, PTR_BM_CENTER)))
	{
		LogDetail("Attempted to print bitmap: %s", szPath);

		return FALSE;
	}

	return TRUE;
}

// (a.walling 2011-04-28 10:02) - PLID 43492
BOOL COPOSPrinterDevice::InternalCutPaper()
{	
	if (!m_pPrinter) {
		return FALSE;
	}
	
	
	// (a.walling 2011-03-21 17:32) - PLID 42931 - This is an alternative way to cut paper (which had no effect in testing)
	/*
	if (false) {
		CheckError(m_pPrinter->PrintNormal(PTR_S_RECEIPT, "\x1b|fP"), 
			"Error cutting receipt paper");
	} else*/

	// (a.walling 2011-04-29 12:24) - PLID 43507 - Some do not have paper cutters apparently
	if (m_pPrinter->CapRecPapercut) {		
		// (a.walling 2011-04-28 10:02) - PLID 43492
		if (!RetryOPOSBusyOrFail(this, "Error cutting receipt paper", boost::bind(&OposPOSPrinter_1_8_Lib::IOPOSPOSPrinter::CutPaper, m_pPrinter.GetInterfacePtr(), PTR_CP_FULLCUT)))
		{
			return FALSE;
		}
	} else {
		AfxMessageBox("The receipt has printed and is ready to be torn. Please tear or cut the receipt before continuing.", MB_OK | MB_ICONINFORMATION);
	}

	return TRUE;
}

// (a.walling 2011-04-29 12:24) - PLID 43507 - See how many LFs are already enqueued
void COPOSPrinterDevice::ReverseCoalesceLineFeeds(std::vector<Output>& queuedOutput, int& nLineFeeds)
{
	for (std::vector<Output>::reverse_iterator it = queuedOutput.rbegin(); it != queuedOutput.rend() && nLineFeeds > 0; ++it) {
		if (it->type != Output::TextOutput) {
			break;
		}

		for (int i = it->str.GetLength() - 1; i >= 0 && nLineFeeds > 0; --i) {
			char c = it->str[i];

			switch (c) {
				case ' ':
				case '\t':
					break;
				case '\n':
					nLineFeeds--;
					break;
				default:
					return;
					break;
			}
		}
	}
}

// (a.walling 2011-04-28 10:02) - PLID 43492
// (a.walling 2011-04-29 12:24) - PLID 43507 - This will automatically feed the minimum number of lines for a paper cut now
BOOL COPOSPrinterDevice::FlushAndTryCut()
{
	if (!m_pPrinter) {
		return FALSE;
	}

	int nLineFeeds = 3;
	try {
		nLineFeeds = m_pPrinter->RecLinesToPaperCut;
	} NxCatchAllIgnore();

	// (a.walling 2011-04-29 12:24) - PLID 43507 - See how many LFs are already enqueued
	ReverseCoalesceLineFeeds(m_queuedOutput, nLineFeeds);

	if (nLineFeeds > 0) {
		m_queuedOutput.push_back(Output::Text(CString('\n', nLineFeeds)));
	}

	std::vector<Output> queuedOutput;
	queuedOutput.swap(m_queuedOutput);

	

	int nAttempt = 0;
	for(;;) {
		nAttempt++;

		if (InternalFlushAndTryCut(queuedOutput)) {
			return TRUE;
		}

		if (IDRETRY != AfxMessageBox("There was an error attempting to print this receipt. Do you want to retry?", MB_RETRYCANCEL | MB_ICONEXCLAMATION, 0)) {
			return FALSE;
		}

		if (nAttempt > 1 && IDYES == AfxMessageBox("Do you want to try resetting the printer device?", MB_YESNO | MB_ICONQUESTION, 0)) {
			if (!ResetPOSPrinter()) {
				return FALSE;
			}
		}
	}

	return FALSE;
}

// (a.walling 2011-04-28 10:02) - PLID 43492
BOOL COPOSPrinterDevice::InternalFlushAndTryCut(std::vector<Output>& queuedOutput)
{
	for (std::vector<Output>::iterator it = queuedOutput.begin(); it != queuedOutput.end(); ++it) {
		switch (it->type) {
			case Output::TextOutput:
				{
					CString strText;

					for (; it != queuedOutput.end() && it->type == Output::TextOutput; ++it) {
						strText.Append(it->str);
					}

					if (!InternalPrintText(strText)) {
						return FALSE;
					}

					if (it != queuedOutput.end()) {
						--it;
					}
				}
				break;
			case Output::BitmapOutput:
				if (!InternalPrintBitmap(it->str)) {
					return FALSE;
				}
				break;
			default:
				ThrowNxException("Invalid output type %li!", it->type);
				break;
		}

		if (it == queuedOutput.end()) {
			break;
		}
	}

	if (!InternalCutPaper()) {
		return FALSE;
	}

	return TRUE;
}

long COPOSPrinterDevice::GetLineWidth() {

	long nWidth = -1;

	if (m_pPrinter) {

		nWidth = m_pPrinter->GetRecLineChars();
	}

	if (nWidth == -1) {
		AfxThrowNxException("Could not retrieve width specifications from POS Printer.");
	}

	return nWidth;
}

BOOL COPOSPrinterDevice::InitiatePOSPrinterDevice(CString strPrinterDeviceName)
{
	try {
		m_bIsLoading = TRUE;
		if (!ClosePOSPrinter()) {
			return FALSE;
		}

		BOOL bResult = CreateAndPrepareOPOSPrinter(strPrinterDeviceName);

		m_bIsLoading = FALSE;
		return bResult;

	}NxCatchAll("Error in COPOSPrinterDevice::InitiatePrinterDevice");
	return FALSE;
}

BOOL COPOSPrinterDevice::CheckStatus() {
	if (!m_pPrinter) {
		MsgBox(MB_ICONEXCLAMATION, "Cannot access receipt printer.");
		return FALSE;
	}

	if (m_pPrinter->CoverOpen) {
		MsgBox(MB_ICONEXCLAMATION, "The receipt printer cover is open, please fix this before attempting to print.");
		return FALSE;
	}
	else if (m_pPrinter->RecEmpty) {
		MsgBox(MB_ICONEXCLAMATION, "The receipt printer is out of paper, please fix this before attempting to print.");
		return FALSE;
	}
	else if (m_pPrinter->RecNearEnd) {
		MsgBox(MB_ICONEXCLAMATION, "The receipt printer is almost out of paper, please fix this soon.");
		return TRUE;
	}
	else {
		return TRUE;
	}
}

// (a.walling 2011-04-28 10:02) - PLID 43492
long COPOSPrinterDevice::GetState()
{
	if (!m_pPrinter) {
		return OPOS_S_CLOSED;
	}

	try {
		return m_pPrinter->GetState();
	} NxCatchAllIgnore();

	return OPOS_S_ERROR;
}


// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsBoldSupported() {

	if (m_pPrinter && m_pPrinter->CapRecBold) {
		return TRUE;
	}
	
	return FALSE;
}

// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsItalicSupported() {

	if (m_pPrinter && m_pPrinter->CapRecItalic) {
		return TRUE;
	}

	return FALSE;
}

// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsUnderlineSupported() {

	if (m_pPrinter && m_pPrinter->CapRecUnderline) {
		return TRUE;
	}

	return FALSE;
}

// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::GetSupportedFonts(CPtrArray *paryFonts) {

	if (!m_pPrinter) {
		return FALSE;
	}

	long nSize = paryFonts->GetSize();
	if (nSize > 0) {
		for (int i = nSize - 1; i >= 0; i--) {
			FontType *pFont = (FontType*)paryFonts->GetAt(i);
			paryFonts->RemoveAt(i);
			if (pFont) {
				delete pFont;
			}
		}
	}

	CString strChars = (LPCTSTR)m_pPrinter->FontTypefaceList;
	CString strSizes = (LPCTSTR)m_pPrinter->RecLineCharsList;

	LogDetail("OPOSPrinterDevice: Fonts: '%s', Sizes: '%s'", strChars, strSizes);
	
	// (a.walling 2011-04-29 12:24) - PLID 43507 - A blank string means that no alternative fonts are available.
	if (strChars.IsEmpty()) {
		FontType* pfntType = new FontType;
		pfntType->nFontNumber = 1;
		pfntType->strFontName = "Default";
		pfntType->nFontChars = m_pPrinter->GetRecLineChars();
		paryFonts->Add(pfntType);
		return TRUE;
	}

	long nResult = strChars.Find(",");
	long nSizeResult = strSizes.Find(",");
	long nFontNumber = 1;
	while (nResult != -1 && nSizeResult != -1) {
		FontType* pfntType = new FontType;

		// (a.walling 2011-04-29 12:24) - PLID 43459 - It is only coincidental that the Citizen printer we used started its fonts with a number; others may not.
		// They should be numbered in order.

		pfntType->nFontNumber = nFontNumber;
		pfntType->strFontName = strChars.Left(nResult);
		pfntType->nFontChars = atoi(strSizes.Left(nSizeResult)); // (a.walling 2011-04-27 10:08) - PLID 43459 - AHOY MATEY! Was using the wrong var here
		
		strChars = strChars.Right(strChars.GetLength() - (nResult + 1));
		strSizes = strSizes.Right(strSizes.GetLength() - (nSizeResult + 1));
		
		nResult = strChars.Find(",");
		nSizeResult = strSizes.Find(",");
		nFontNumber++;

		paryFonts->Add(pfntType);

	}

	FontType* pfntType = new FontType;
	pfntType->nFontNumber = nFontNumber;
	pfntType->strFontName = strChars;
	pfntType->nFontChars = atoi(strSizes);

	paryFonts->Add(pfntType);

	return TRUE;
}




// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsDoubleWideSupported() {

	if (m_pPrinter && m_pPrinter->CapRecDwide) {
		return TRUE;
	}

	return FALSE;
}

// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsDoubleHighSupported() {
		
	if (m_pPrinter && m_pPrinter->CapRecDhigh) {
		return TRUE;
	}

	return FALSE;

}

// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
BOOL COPOSPrinterDevice::IsDoubleWideAndHighSupported() {

	if (m_pPrinter && m_pPrinter->CapRecDwideDhigh) {
		return TRUE;
	}

	return FALSE;
		

}

// (a.walling 2011-04-28 10:02) - PLID 43492
bool COPOSPrinterDevice::CheckError(long nResultCode, const CString& strMessage, bool bShowMessageBox, bool bCheckLastResultCode)
{
	if (bCheckLastResultCode) {
		try {
			nResultCode = m_pPrinter->ResultCode;
		} catch (...) {
			nResultCode = OPOS_E_FAILURE;
			LogDetail("Failure getting last result code");
		}
	}
	CString strFinalMessage = GetErrorString(nResultCode, strMessage);

	if (strFinalMessage.IsEmpty()) {
		return true;
	}

	LogDetail("OPOSPrinterDevice: %s", strFinalMessage);

	if (bShowMessageBox) {
		AfxMessageBox(strFinalMessage, MB_ICONERROR);
	}

	return false;
}

CString COPOSPrinterDevice::GetErrorString(long nResultCode, const CString& strMessage)
{
	if (nResultCode == OPOS_SUCCESS) {
		return "";
	}

	CString strFinalMessage, strResultCode, strExtendedResultCode, strErrorLevel, strDeviceMessage;
	
	strResultCode.Format("%s (code %li)", OPOS::GetMessage(nResultCode), nResultCode);

	// (a.walling 2011-04-28 15:30) - PLID 43459 - an extended result code can always be present, not necessarily only when OPOS_E_EXTENDED is the error.
	long nResultCodeExtended = 0;
	if (m_pPrinter && SUCCEEDED(m_pPrinter->get_ResultCodeExtended(&nResultCodeExtended)) && nResultCodeExtended != 0) {
		strExtendedResultCode.Format(" - Extended error: %s (code %li)", GetExtendedMessage(nResultCodeExtended), nResultCodeExtended);
	}

	long errorLevel = 0;
	m_pPrinter->get_ErrorLevel(&errorLevel);

	switch (errorLevel) {
		case PTR_EL_NONE:
			strErrorLevel = "Ignored";
			break;
		case PTR_EL_RECOVERABLE:
			strErrorLevel = "Recoverable";
			break;
		case PTR_EL_FATAL:
			strErrorLevel = "Fatal";
			break;
		default:
			strErrorLevel = "Unknown";
			break;
	}

	if (nResultCode == OPOS_SUCCESS) {
		strDeviceMessage = "Success";
	} else if (m_pPrinter) {
		BSTR errorString = 0;
		if (SUCCEEDED(m_pPrinter->get_ErrorString(&errorString))) {
			_bstr_t bstrErrorString(errorString, false);

			strDeviceMessage = (LPCTSTR)bstrErrorString;
		}
	}

	strFinalMessage.Format("%s error - %s (%s)"
		, strErrorLevel
		, strMessage
		, strResultCode);

	if (!strExtendedResultCode.IsEmpty()) {
		strFinalMessage += strExtendedResultCode;
	}

	if (!strDeviceMessage.IsEmpty()) {
		strFinalMessage += FormatString(" - Device reports: %s", strDeviceMessage);
	}

	return strFinalMessage;
}

CString COPOSPrinterDevice::GetExtendedMessage(long code)
{
	switch (code) {

	case OPOS_EPTR_COVER_OPEN:
		return "Printer cover open";
	case OPOS_EPTR_REC_EMPTY:
		return "Receipt printer empty";
	case OPOS_EPTR_TOOBIG:
		return "Bitmap too big";
	case OPOS_EPTR_BADFORMAT:
		return "Bitmap invalid format";
	case OPOS_EPTR_REC_CARTRIDGE_REMOVED:
		return "Printer cartridge removed";
	case OPOS_EPTR_REC_CARTRIDGE_EMPTY:
		return "Printer cartridge empty";
	case OPOS_EPTR_REC_HEAD_CLEANING:
		return "Printer cartridge cleaning";
	default:
		return "Unknown";
	}
}
