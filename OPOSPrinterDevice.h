#if !defined(AFX_OPOSPRINTERDEVICE_H__64240F79_5A86_4556_9103_90F52B267A78__INCLUDED_)
#define AFX_OPOSPRINTERDEVICE_H__64240F79_5A86_4556_9103_90F52B267A78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OPOSPrinterDevice.h : header file
//

// (j.gruber 2007-05-09 13:01) - PLID 9802 - POS functionality for Receipt Printer
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "OPOSPOSPrinter.tlb"

#include <boost/noncopyable.hpp>

/////////////////////////////////////////////////////////////////////////////
// COPOSPrinterDevice window

#define IDC_PRINTER_CTRL	1000


// (a.walling 2011-03-21 17:32) - PLID 42931 - Significant changes here, too numerous to label individually.
// Basically, use POSPrinterAccess to claim and release the printer.
// Some useful logging information is added, as well as some more error checking


struct FontType {
	CString strFontName;
	long nFontNumber;
	long nFontChars;
};

class POSPrinterAccess;

class COPOSPrinterDevice : public CWnd
{
	friend class POSPrinterAccess;
// Construction
public:
	COPOSPrinterDevice(CWnd *pParentWnd);

	HRESULT GetPrinterState();

	// (a.walling 2011-04-28 10:02) - PLID 43492
	BOOL OpenPOSPrinter();
	BOOL ClosePOSPrinter();
	BOOL ResetPOSPrinter();
	BOOL CreateAndPrepareOPOSPrinter(CString strPrinterName);
	BOOL CreateAndPrepareOPOSPrinterWindow();
	BOOL InitiatePOSPrinterDevice(CString strPrinterDeviceName);

	//TES 12/6/2007 - PLID 28192 - Functions to claim and release the POS Printer (before printing, it must be claimed,
	// and after it is claimed, it must be released before any third-party applications will be able to print to it).
	/*BOOL ClaimPOSPrinter();
	void ReleasePOSPrinter();*/

	BOOL PrintText(CString strTextToPrint);
	BOOL PrintBitmap(CString strPath);

	// (a.walling 2011-04-28 10:02) - PLID 43492
	// (a.walling 2011-04-29 12:24) - PLID 43507 - This will automatically feed the minimum number of lines for a paper cut now
	BOOL FlushAndTryCut();

	long GetLineWidth();

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Replace unavailable chars with spaces
	inline static bool IsPrintable(unsigned char c)
	{
		switch (c)
		{
			case 0x0A: // \n
			case 0x0D: // \r
			case 0x1B: // esc
				return true;
		};

		if (c < ' ') {
			return false;
		}

		if (c > '~') {
			return false;
		}

		return true;
	};

	BOOL m_bIsLoading;
	BOOL CheckStatus();

	// (a.walling 2011-04-28 10:02) - PLID 43492
	long GetState();

	// (j.gruber 2008-01-16 17:35) - 28661 - added function for formatting header and footer in Sales receipt
	BOOL IsBoldSupported();
	BOOL IsItalicSupported();
	BOOL IsUnderlineSupported();
	BOOL IsDoubleWideSupported();
	BOOL IsDoubleHighSupported();
	BOOL IsDoubleWideAndHighSupported();
	BOOL GetSupportedFonts(CPtrArray *paryFonts);


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COPOSPrinterDevice)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COPOSPrinterDevice();

	// Generated message map functions
protected:

	// (a.walling 2011-04-28 10:02) - PLID 43492
	struct Output
	{
		enum OutputType
		{
			TextOutput = 0,
			BitmapOutput = 1
		};

		Output(OutputType _type, const CString& _str)
			: type(_type)
			, str(_str)
		{	
		}

		OutputType type;
		CString str;

		static Output Text(const CString& strText)
		{
			return Output(Output::TextOutput, strText);
		};

		static Output Bitmap(const CString& strPath)
		{
			return Output(Output::BitmapOutput, strPath);
		};
	};

	// (a.walling 2011-04-28 10:02) - PLID 43492
	std::vector<Output> m_queuedOutput;

	// (a.walling 2011-04-28 10:02) - PLID 43492
	BOOL InternalPrintText(LPCTSTR szText);
	BOOL InternalCutPaper();
	BOOL InternalPrintBitmap(LPCTSTR szPath);

	// (a.walling 2011-04-29 12:24) - PLID 43507 - See how many LFs are already enqueued
	void ReverseCoalesceLineFeeds(std::vector<Output>& queuedOutput, int& nLineFeeds);

	// (a.walling 2011-04-28 10:02) - PLID 43492
	BOOL InternalFlushAndTryCut(std::vector<Output>& queuedOutput);

	bool AcquireClaim(); // (b.savon 2014-09-05 09:52) - PLID 59621 - Return success or failure and throw exceptions
	void ReleaseClaim();

	void AttemptClaim(); // throws exceptions
	void AttemptRelease();
	int m_nClaimCount;

	CString m_strPrinterName;
	OposPOSPrinter_1_8_Lib::IOPOSPOSPrinterPtr m_pPrinter;

	CWnd *m_pParentWnd;

public:
	// (a.walling 2011-04-28 10:02) - PLID 43492
	bool CheckError(long nResultCode, const CString& strMessage, bool bShowMessageBox = true, bool bCheckLastResultCode = false);	
	CString GetErrorString(long nResultCode, const CString& strMessage);
	static CString GetExtendedMessage(long code);
};

// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
// not to mention providing exception safety. This object will claim and signal to release when it goes
// out of scope. The release is actually slightly delayed as well. This also provides -> overloads
// so it can be used as if it was a COPOSPrinterDevice*.
class POSPrinterAccess : private boost::noncopyable
{
public:
	POSPrinterAccess(COPOSPrinterDevice* pDevice = NULL) // no explicit device will ask the MainFrame
		: m_bIsValid(false)
		, m_pDevice(pDevice)
	{
		EnsureClaim();
	};

	~POSPrinterAccess()
	{
		ReleaseClaim();
	};

	// allows if(posPrinterAccess)
	// and allows passing to functions/vars that expect a COPOSPrinterDevice
	operator COPOSPrinterDevice*() const
	{
		return m_bIsValid ? m_pDevice : NULL;
	};

	// allows posPrinterAccess->SomeFunction
	COPOSPrinterDevice* operator->() const
	{
		return m_bIsValid ? m_pDevice : NULL;
	};

protected:
	void EnsureClaim();
	void ReleaseClaim();

	bool m_bIsValid;
	COPOSPrinterDevice* m_pDevice;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPOSPRINTERDEVICE_H__64240F79_5A86_4556_9103_90F52B267A78__INCLUDED_)
