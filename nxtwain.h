#ifndef __NXTWAIN_H__
#define __NXTWAIN_H__

#pragma once

#define NXTWAIN_USE_CIMAGEBUFFER

// (a.walling 2010-01-27 14:22) - PLID 36568 - Include the NxTWAIN header and import the library
#include "NxTWAINLib.h"
#ifdef _DEBUG
#pragma comment(lib, "NxTWAIND.lib")
#else
#pragma comment(lib, "NxTWAIN.lib")
#endif

#include "NxGdiPlusUtils.h"

#import "NxPdf.tlb"

//
// (c.haag 2006-07-19 10:10) - PLID 21505 - I totally ripped this from the connecting feedback dialog code
//
// (a.walling 2010-01-27 14:59) - PLID 36568 - Moved to header
class CNxTwainProgressDlg
{
public:
	CNxTwainProgressDlg();
	~CNxTwainProgressDlg();
	void SetProgressText(const CString& strText);

protected:
	HANDLE m_hEventWaitForTerm;
	CWinThread *m_pThread;
};

namespace NXTWAINlib
{
	typedef enum {
		imgNone = 0x00000000,
		imgBMP = 0x00000001,
		imgJPG = 0x00000002,
	} EimgType;

	typedef enum {
		eScanToPatientFolder = 0,
		eScanToRemoteFolder
	} EScanType;

	typedef enum {
		// (a.walling 2009-12-10 08:26) - PLID 36518
		eScanToImage = 1,	// if this bit is set, the default file format of the driver will be used
		eScanToNative, // force scanning in native mode
		eScanToPDF,
		eScanToMultiPDF,
	} EScanTargetFormat;

	// (a.walling 2008-07-24 13:28) - PLID 30836 - Pass in the image as well
/*	void (*NXTWAIN_ONDOCSCANNED_CALLBACK) (
		NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
//		const CString& strFileName, /* The full filename of the document that was scanned */
//		BOOL& bAttach, /* Defaults to true; set to false if you don't want NxTwain to attach the document */
//		void* pUserData, /* Optional user-defined data. */
//		Gdiplus::Bitmap* pBitmap /* The image that was scanned */ );

/*	void (*NXTWAIN_PRECOMPRESSION_CALLBACK) {
		const LPBITMAPINFO pDib,	/* The image that was scanned */
//		BOOL& bAttach, /* Defaults to true; set to false if you don't want NxTwain to compress and attach this data */
//		BOOL& bUseChecksum, /* Defaults to false, set to true if you want to set something in the Checksum field.
//		long& nChecksum /* The checksum, ignored if bUseChecksum is false*/);

	
	// (a.walling 2008-07-24 13:28) - PLID 30836 - Also pass the image in memory	
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	typedef void (WINAPI *cbProc)(EScanType, const CString&, BOOL&, void*, CxImage&);
	// (a.walling 2010-01-28 14:42) - PLID 28806 - Now has a connection param
	typedef void (WINAPI *cbPrecompressProc)(const LPBITMAPINFO, BOOL&, BOOL&, long&, ADODB::_Connection*);

	class CNxTwain
	{
	public:
		CString m_strDocumentPath;
		long m_nPersonID;
		cbProc m_cb;
		cbPrecompressProc m_cbPrecompress;
		void* m_pUserData;
		CString m_strDeviceName;
		long m_nCategoryID;
		// (m.hancock 2006-06-27 15:53) - PLID 21071 - Added field for associating MailSent records with lab step records
		long m_nLabStepID;
		// (a.walling 2008-09-03 11:35) - PLID 19638 - Also include PicID
		long m_nPicID;

		// (a.walling 2008-09-05 13:14) - PLID 22821 - Scan target type
		EScanTargetFormat m_eTargetFormat;

		// (a.walling 2010-01-28 08:40) - PLID 28806 - Load properties from the main thread
		void LoadProperties();

		
		// (a.walling 2010-01-28 09:11) - PLID 28806 - Cached properties, loaded via constructor via LoadProperties
		bool m_bShowUI;
		bool m_bAutoFeed; // this preference is not exposed anywhere anymore, but I'm keeping it around just in case.

		CString m_strRemoteFolder;
		bool m_bSave8BitAsJPEG;
		bool m_bApplyMonochromePalette; // (a.walling 2010-04-13 10:53) - PLID 38171
		bool m_bApplyGreyscalePalette; // (a.walling 2010-04-13 10:53) - PLID 38171
		bool m_bScanToRemoteFolder;
		bool m_bScanToDocumentFolder;

		bool m_bPDFAutoLandscape;
		bool m_bPDFUseThumbs;
		long m_nPDFPageSize;

		// (a.walling 2010-01-28 14:09) - PLID 28806
		bool m_bIsPhotoJPEG;
		bool m_bIsPhotoPNG;


	public:
		CNxTwain(long nPersonID, const CString& strDocumentPath, 
			cbProc cb, cbPrecompressProc cbPrecompress, void* pUserData,
			const CString& strDeviceName = "", long nCategoryID = -1, long nLabStepID = -1, long nPicID = -1, EScanTargetFormat eTargetFormat = eScanToImage);
		virtual ~CNxTwain();
	};

	// Construction/destruction
	BOOL Initialize(void);
	void CloseConnection();

	// Messages
	BOOL ProcessTWMessage(LPMSG lpMsg);

	// Functions!
	BOOL IsAcquiring();
	BOOL Acquire(long nPersonID, /* Name of the person who we're scanning for */
		const CString& strDocumentPath, /* The patient's document path */
		
		// (a.walling 2008-07-24 13:28) - PLID 30836 - Also pass the image in memory
		// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
		void (WINAPI *cb)(EScanType, const CString&, BOOL&, void*, CxImage&), /* Optional callback that reports
										 whenever a document is scanned. Can
										 be NULL. */
		// (a.walling 2010-01-28 14:42) - PLID 28806 - Now has a connection param
		void (WINAPI *cbPrecompress)(const LPBITMAPINFO, BOOL&, BOOL&, long&, ADODB::_Connection*), /*Optional callback, called before the image is compressed, can be NULL.*/
		void* pUserData	/* Optional user-defined data. */,
		const CString& strDeviceName = "",
		long nCategoryID = -1,
		long nLabStepID = -1, /*// (m.hancock 2006-06-27 16:04) - PLID 21071 - Added field for associating MailSent records with lab step records*/
		long nPicID = -1, // (a.walling 2008-09-03 11:35) - PLID 19638 - Also include PicID
		EScanTargetFormat eScanTargetFormat = eScanToImage	// (a.walling 2008-09-05 15:50) - PLID 22821 - Support PDF targets
		);
	BOOL SelectSource();
	
	void SetTwainMessageRecipient(HWND hwndMessageWnd);

	
	// (a.walling 2008-09-09 15:47) - PLID 30389 - Allow passing in an override for personID
	// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection
	CString GetUserDefinedOutputFilename(ADODB::_Connection* lpCon, long nPersonID = -1, CString strExt = "");


	
	// (a.walling 2010-01-27 14:29) - PLID 36568 - Implement our NxTWAIN interface to handle events
	class CPracticeTwainInterface : public NxTwainInterface
	{
	public:
		CPracticeTwainInterface()
			: m_pTwainProgressDlg(NULL),
			  m_hwndNotify(NULL)
		{
		};

		void SetNotifyWindow(HWND hwnd) {
			m_hwndNotify = hwnd;
		};

		void ReportErrors();

		virtual void OnNativeTransfer(long nCurTransfer, BITMAPINFOHEADER* pDIB, BYTE* pBits, LPCTSTR szImageInfo, LPCTSTR szExImageInfo);

		virtual void OnTransfersComplete();

		virtual void OnError(LPCTSTR szError);

		virtual void OnProgressBegin();

		virtual void OnProgressText(LPCTSTR szProgressText);

		virtual void OnProgressEnd();
		
		// return true to cancel processing, if necessary
		virtual bool ProcessMessage(MSG* pMsg);

		virtual void OnThreadCreate();
		virtual void OnThreadFinalize();

		CStringArray m_saErrors;

	protected:
		HWND m_hwndNotify;

		ADODB::_ConnectionPtr m_pCon;

		CStringArray m_saTempPdfFiles;
		void DestroyProgressDialog();
		CNxTwainProgressDlg* m_pTwainProgressDlg;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin code for sti.dll wrapper
#include "NxException.h"
#include <sti.h>

// Notice we temporarily define ThrowStringException as ThrowNxException and we undefine it 
// below.  This goes on the assumption that this macro is only ever temporarily defined.  If 
// someone defines it globally as something else, then our assumption is broken and we may 
// have to re-assess ImportDll.h's exception-throwing strategy.
#define ThrowStringException	ThrowNxException

// CStiDll wrapper for sti.dll
#include "ImportDll.h"
BEGIN_DLL_FUNCTION_FORWARD(CStiDll)

	BEGIN_DLL_DEFINE_FUNCTIONS(CStiDll)
		DEFINE_DLL_FUNCTION_FORWARD_T	(HRESULT,		StiCreateInstance,						(HINSTANCE hinst, DWORD dwVer, interface IStillImageW **ppSti, LPUNKNOWN punkOuter), (hinst, dwVer, ppSti, punkOuter))
	BEGIN_DLL_DEFINE_FUNCTIONS(CStiDll)

	BEGIN_DLL_LOAD_FUNCTIONS(CStiDll, "Sti.dll")
		LOAD_DLL_FUNCTION_FORWARD_T	(StiCreateInstance)
	END_DLL_LOAD_FUNCTIONS(CStiDll)

END_DLL_FUNCTION_FORWARD(CStiDll)

#undef ThrowStringException


// Make the global variable available to anyone who wants it
extern CStiDll g_dllSti;
// End code for User32.dll wrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
