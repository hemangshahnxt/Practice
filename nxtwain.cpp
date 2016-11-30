// This source don't impress me much.
#include <stdafx.h>
#include "nxtwain.h"
#include "twain.h"
#include "historyutils.h"
#include "globalutils.h"
#include "historydlg.h"
#include "mergeengine.h"
#include "nxprogressdlg.h"
#include "practicerc.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include "fileutils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Message levels
#define ML_NONE 	0
#define ML_ERROR	1
#define ML_INFO		2
#define ML_FULL 	3

#define MAX_RETURNCODE 10
#define MAX_CONDITIONCODE 14

#define  WIDTHBYTES(i)    ((i+31)/32*4)

// (a.walling 2010-01-28 15:28) - PLID 37107 - Are we using TWAIN 2.0?
bool UsingTwain2() {
	static bool bCheckedReg = false;
	static bool bUsingTwain2 = true;

	if (!bCheckedReg) {
		bCheckedReg = true;
		// (j.armen 2011-10-24 14:33) - PLID 46139 - GetPracPath is referencing ConfigRT
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		CString strPath = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
		bUsingTwain2 = GetRemotePropertyInt("TWAIN_UseTwain2", TRUE, 0, strPath, true) ? true : false;
	}

	return bUsingTwain2;
}
NXTWAINlib::CPracticeTwainInterface* g_pPracticeTwainInterface = NULL;

BOOL TWDSMOpen = FALSE;    // glue code flag for an Open Source Manager
BOOL TWDSOpen  = FALSE;    // glue code flag for an Open Source
BOOL TWDSEnabled  = FALSE; // glue code flag for an Open Source
TW_IDENTITY appID, dsID;          // storage for App and DS (Source) states
DSMENTRYPROC lpDSM_Entry;         // entry point to the SM
TW_STATUS gGlobalStatus = {0, 0};
HMODULE hDSMDLL = NULL;     // handle to Source Manager for explicit load
int g_AutoFeedBOOL = FALSE;
TW_USERINTERFACE  twUI;	  // Fix as per Courisimault: 940518 - bd
unsigned long g_nPersonID = -25;
long g_nCategoryID = -1;
long g_nLabStepID = -1;    // (m.hancock 2006-06-27 15:53) - PLID 21071 - Added field for associating MailSent records with lab step records
long g_nPicID = -1;		// (a.walling 2008-09-03 11:43) - PLID 19638 - Silly global variable for PicID
// (a.walling 2009-12-11 13:38) - PLID 36518 - eScanToImage by default
NXTWAINlib::EScanTargetFormat g_eTargetFormat = NXTWAINlib::eScanToImage;
CString g_strDocumentPath;

// (a.walling 2010-01-28 09:14) - PLID 28806 - Cached properties as globals
bool g_bShowUI = true;
bool g_bAutoFeed = false;

CString g_strRemoteFolder;
bool g_bSave8BitAsJPEG = false;
// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the monochrome palette, default to false.
bool g_bApplyMonochromePalette = false;
// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the greyscale palette, default to true
bool g_bApplyGreyscalePalette = true;
bool g_bScanToRemoteFolder = false;
bool g_bScanToDocumentFolder = true;

bool g_bPDFAutoLandscape = true;
bool g_bPDFUseThumbs = true;
long g_nPDFPageSize = NxPdf::psLetter;

// (a.walling 2010-01-28 14:09) - PLID 28806
bool g_bIsPhotoJPEG;
bool g_bIsPhotoPNG;

// (a.walling 2008-07-24 13:28) - PLID 30836 - Pass in the image as well
// (a.walling 2008-09-03 13:10) - PLID 22821 - Use GdiPlus object instead of CxImage
// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
static void (WINAPI *g_cb)(NXTWAINlib::EScanType, const CString&, BOOL&, void*, CxImage&)  = NULL;
// (a.walling 2010-01-28 14:42) - PLID 28806 - Now has a connection param
static void (WINAPI *g_cbPrecompress)(const LPBITMAPINFO, BOOL&, BOOL&, long&, ADODB::_Connection*) = NULL;
void* g_pUserData = NULL;
HWND g_hwndMessageWnd = NULL;
BOOL g_bTerminateImmediately = FALSE; // If this is true, that means were are in a state of instability, and we have to quit immediately.

CWinThread* g_pTWAINThread = NULL;
DWORD g_dwTWAINThreadID = 0;
HANDLE g_hevAcquiring = NULL;

WORD DibNumColors (VOID FAR *pv); // Defined in globalutils.cpp

CStiDll g_dllSti;

/////////////////////////////////////////////////////
// BEGIN CNxTwainProgressDlg
/////////////////////////////////////////////////////
//
// (c.haag 2006-07-19 10:10) - PLID 21505 - I totally ripped this from the connecting feedback dialog code
//

UINT AFX_CDECL Thread_ShowTwainProgressDlg(LPVOID lpParam)
{
	HANDLE hEventWaitForTerm = (HANDLE)lpParam;

	// Create the dialog but don't display it
	// (z.manning, 05/16/2008) - PLID 30050 - Converted to NxDialog
	// (a.walling 2012-07-16 12:31) - PLID 46648 - Dialogs must set a parent
	CNxDialog dlg(CWnd::GetDesktopWindow());
	
	
	// Set a two-second timer, after which we will show the window
	//TES 2003-1-6: This used to set a timer.  The problem is, in order to do that, you have to create the window, and if
	//you create a window whose parent is the Desktop, then destroy that window without ever having shown it, it screws up
	//the z-order.  So we will instead use the GetTickCount, and check it every 10 ms.
	DWORD dwTickStart = GetTickCount();
	CString strText = "SENTINEL_UNCHANGED_TEXT";
	
	// Pump the message queue, waiting for either a quit message, or the event to be set
	while (true) {
		DWORD dwWaitInterval = 10;//We'll check every 10 ms whether we should show the window.
		DWORD dwResult = MsgWaitForMultipleObjectsEx(1, &hEventWaitForTerm, dwWaitInterval, QS_ALLEVENTS, 0);
		if (dwResult == WAIT_OBJECT_0 + 1) {
			// There's a message in the queue, pump until there are none or the event is set
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message != WM_QUIT) {
					// Pump the message 
					if (msg.message == NXM_CHANGE_CONNECTINGFEEDBACK_TEXT /*&& dlg.GetSafeHwnd()*/) {
						// Change the visible text in the message
						BSTR bstr = (BSTR)msg.lParam;
						strText = bstr;
						SysFreeString(bstr);
						if(dlg.GetSafeHwnd()) {
							dlg.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE | RDW_ERASENOW);
							dlg.SetDlgItemText(IDC_STATIC_MESSAGE, strText);							
						}
					} else {
						// Just pump the message the normal way
						if (!TranslateMessage(&msg)) {
							DispatchMessage(&msg);
						}
					}
				} else {
					// Got the quit message, so quit
					if(dlg.GetSafeHwnd())
						dlg.DestroyWindow();
					return 1;
				}

				// We're pumping messages, but still let's just check to see if the event was set
				if (WaitForSingleObject(hEventWaitForTerm, 0) == WAIT_OBJECT_0) {
					// Event was set so return
					if(dlg.GetSafeHwnd())
						dlg.DestroyWindow();
					return 2;
				}
			}
		} 
		else if(dwResult == WAIT_TIMEOUT) {
			if(!dlg.GetSafeHwnd()) {
				//Well, it's been a little while, let's see if it's time to show the window.
				if(dwTickStart + 2000 <= GetTickCount()) {
					// It's the timer, kill the timer and show the window
					dlg.Create(IDD_OPENING_DLG, CWnd::GetDesktopWindow());
					dlg.SetWindowText("NexTech Practice Image Scanning");
					if(strText != "SENTINEL_UNCHANGED_TEXT") {
							dlg.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE | RDW_ERASENOW);
							dlg.SetDlgItemText(IDC_STATIC_MESSAGE, strText);
					}
					dlg.ShowWindow(SW_RESTORE); // (z.manning, 05/08/2007) - PLID 25467 - Changed to SW_RESTORE instead of SW_SHOW.
					dlg.CenterWindow();
					dlg.UpdateWindow();
					dlg.BringWindowToTop();
					//We don't need to check this anymore, so let's not do anything else until we get a message.
					dwWaitInterval = INFINITE;
				}
			}
		}
		else {
			// Either the event was set or the wait failed in some unexpected way
			ASSERT(dwResult == WAIT_OBJECT_0); // I don't know how it could be a WAIT_TIMEOUT, WAIT_IO_COMPLETION, or WAIT_ABANDONED_0+
			if(dlg.GetSafeHwnd())
				dlg.DestroyWindow();
			return 0;
		}
	}
}

struct CNxTwainProgressDlgResources
{
	HANDLE m_hEventWaitForTerm;
	CWinThread *m_pThread;
};

UINT AFX_CDECL Thread_ReleaseTwainProgressDlgResources(LPVOID lpParam)
{
	if (lpParam) {
		// Convert the param
		CNxTwainProgressDlgResources *pResources = (CNxTwainProgressDlgResources *)lpParam;

		// Wait for the event to finish
		WaitForSingleObject(pResources->m_pThread->m_hThread, INFINITE);
		
		// Delete the thread object now that we know it's finished
		delete pResources->m_pThread;
		pResources->m_pThread = NULL;
		
		// Close the event handle because now that the thread is guaranteed finished, nothing else needs the handle to exist
		CloseHandle(pResources->m_hEventWaitForTerm);
		pResources->m_hEventWaitForTerm = NULL;

		// Finally deallocate the memory that was storing the resource pointers
		delete pResources;
		
		// Return success
		return 0;
	} else {
		// Return failure
		return 1;
	}
}

CNxTwainProgressDlg::CNxTwainProgressDlg() 
{
	// Create the event
	m_hEventWaitForTerm = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Spawn the thread (make sure it isn't set to auto-delete)
	m_pThread = AfxBeginThread(Thread_ShowTwainProgressDlg, m_hEventWaitForTerm, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}
	
CNxTwainProgressDlg::~CNxTwainProgressDlg()
{
	// Set the event, which tells the thread it's allowed to close
	SetEvent(m_hEventWaitForTerm);

	// This seems weird but it's really not.  See we could just sit here and wait for the m_pThread to finish 
	// but we don't really need to.  But if we ever want to call CloseHandle on our event handle, we need to 
	// wait until the m_pThread returns.  So instead of waiting for it here, we spawn a different thread (a 
	// thread that MFC will auto-delete for us) that will take responsibility for destroying our member 
	// variables for us.  That way it can run in it's own time without stopping us from returning.  But in 
	// the end everything gets properly destroyed in as timely a manner as possible. (notice the secondary 
	// thread also deallocates the CNxTwainProgressDlgResources that we create right here so we don't
	// have to call delete on it ourselfs)
	CNxTwainProgressDlgResources *pParam = new CNxTwainProgressDlgResources;
	pParam->m_hEventWaitForTerm = m_hEventWaitForTerm;
	pParam->m_pThread = m_pThread;
	AfxBeginThread(Thread_ReleaseTwainProgressDlgResources, (LPVOID)pParam); // note this is autodeleted
	// Tempting though it is, do not call delete on the pParam we just created, the thread does that for us


	// We've passed our parameters into the "release resources" thread, so we're no longer responsible for them
	m_hEventWaitForTerm = NULL;
	m_pThread = NULL;
}

void CNxTwainProgressDlg::SetProgressText(const CString& strText)
{
	// Make sure we actually have the thread running
	if (m_pThread == NULL) return;
	// Post the message to change the text to the thread
	PostThreadMessage(m_pThread->m_nThreadID, NXM_CHANGE_CONNECTINGFEEDBACK_TEXT, 0, (LPARAM)strText.AllocSysString());
}
/////////////////////////////////////////////////////
// END CNxTwainProgressDlg
/////////////////////////////////////////////////////

namespace NXTWAINlib
{
	using namespace ADODB;
	typedef struct
	{
		const char* pszItemName;
		TW_UINT16 ItemId;
	} TABLEENTRY, *pTABLEENTRY;

	TABLEENTRY ReturnCode[MAX_RETURNCODE] =
									{
										{"TWRC_SUCCESS", TWRC_SUCCESS},
										{"TWRC_FAILURE", TWRC_FAILURE},
										{"TWRC_CHECKSTATUS", TWRC_CHECKSTATUS},
										{"TWRC_CANCEL", TWRC_CANCEL},
										{"TWRC_DSEVENT", TWRC_DSEVENT},
										{"TWRC_NOTDSEVENT", TWRC_NOTDSEVENT},
										{"TWRC_XFERDONE", TWRC_XFERDONE},
										{"TWRC_ENDOFLIST", TWRC_ENDOFLIST},
										{"TWRC_INFONOTSUPPORTED", TWRC_INFONOTSUPPORTED},
										{"TWRC_DATANOTAVAILABLE", TWRC_DATANOTAVAILABLE}
									};

	TABLEENTRY ConditionCode[MAX_CONDITIONCODE]	=
									{
										{"TWCC_SUCCESS", TWCC_SUCCESS},
										{"TWCC_BUMMER", TWCC_BUMMER},
										{"TWCC_LOWMEMORY", TWCC_LOWMEMORY},
										{"TWCC_NODS", TWCC_NODS},
										{"TWCC_MAXCONNECTIONS", TWCC_MAXCONNECTIONS},
										{"TWCC_OPERATIONERROR", TWCC_OPERATIONERROR},
										{"TWCC_BADCAP", TWCC_BADCAP},
										{"TWCC_BADPROTOCOL", TWCC_BADPROTOCOL},
										{"TWCC_BADVALUE", TWCC_BADVALUE},
										{"TWCC_SEQERROR", TWCC_SEQERROR},
										{"TWCC_BADDEST", TWCC_BADDEST},
										{"TWCC_CAPUNSUPPORTED", TWCC_CAPUNSUPPORTED},
										{"TWCC_CAPBADOPERATION", TWCC_CAPBADOPERATION},
										{"TWCC_CAPSEQERROR", TWCC_CAPSEQERROR}
									};

	TW_UINT16 MessageLevel()
	{
		return ML_ERROR;
	}

	HWND GetTwainWindow()
	{
		return g_hwndMessageWnd == NULL ? AfxGetMainWnd()->GetSafeHwnd() : g_hwndMessageWnd;
	}

	/*
	* Function: MatchTwainInt
	* Author: TWAIN Working Group
	* Input:
	*		pTable - Pointer to a Table entry that contain the value for the initialization
	*		uiTableSize - Maximum of item in table 
	*		uiCap -	ID for the current capability 
	*		pString - 
	* Output:
	*		TW_BOOL -	TRUE is successful
	* Comments:
	*/
	TW_BOOL MatchTwainInt(pTABLEENTRY pTable, TW_UINT32 uiTableSize,
													TW_INT32 uiCapId, LPSTR pString)
	{
		TW_BOOL result = FALSE;
		TW_UINT16 i = 0;

		ASSERT(pTable);
		ASSERT(pString);

		for(i = 0; i < uiTableSize; i++)
		{
			if (pTable[i].ItemId == uiCapId)
			{
				lstrcpy(pString, pTable[i].pszItemName);
				result = TRUE;
				break;
			}   
		}   
		return  result;
	}

	TW_UINT16 CallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,
											TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData)
	{
		try {
			TW_UINT16 twRC = (*lpDSM_Entry)(pApp, pSrc, DG, DAT, MSG, pData);

			if((twRC != TWRC_SUCCESS)&&(DAT!=DAT_EVENT))
			{
				if ((*lpDSM_Entry)(pApp, pSrc, DG_CONTROL, DAT_STATUS, MSG_GET, 
							(TW_MEMREF)&gGlobalStatus) != TWRC_SUCCESS)
				{
					LogDetail("CallDSMEntry failed and error code was unretrieveable");
				}
			}
			return twRC;
		}
		catch (...)
		{
			MsgBox("An error occured within your third-party scanning software, and the image "
				"transfer has been aborted. Please ensure you have the latest drivers from your "
				"camera or scanner manufacturer.\n\nNexTech Practice will now shut down. If the problem persists, please contact NexTech "
				"technical support for assistance.");
			g_bTerminateImmediately = TRUE;
			if (g_pTWAINThread) g_pTWAINThread->PostThreadMessage(WM_QUIT, 0, 0);
			GetMainFrame()->PostMessage(WM_CLOSE);
		}
		return (TW_UINT16)-1;
	}

	TW_HANDLE _DSM_Alloc(TW_UINT32 _size)
	{
	  return ::GlobalAlloc(GPTR, _size);
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper functions for memory management
	//////////////////////////////////////////////////////////////////////////////
	void _DSM_Free(TW_HANDLE _hMemory)
	{
	  ::GlobalFree(_hMemory);
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper functions for memory management
	//////////////////////////////////////////////////////////////////////////////
	TW_MEMREF _DSM_LockMemory(TW_HANDLE _hMemory)
	{
	  return (TW_MEMREF)::GlobalLock(_hMemory);
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper functions for memory management
	//////////////////////////////////////////////////////////////////////////////
	void _DSM_UnlockMemory(TW_HANDLE _hMemory)
	{
	  ::GlobalUnlock(_hMemory);
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper function to get a capability
	TW_INT16 GetCAP(TW_CAPABILITY& _cap, TW_UINT16 _msg)
	{
	  if(_msg != MSG_GET && _msg != MSG_GETCURRENT && _msg != MSG_GETDEFAULT && _msg != MSG_RESET)
	  {
		return TWCC_BUMMER;
	  }

	  TW_INT16 CondCode = TWCC_SUCCESS;

	  // Check if this capability structure has memory already alloc'd.
	  // If it does, free that memory before the call else we'll have a memory
	  // leak because the source allocates memory during a MSG_GET.
	  if(0 != _cap.hContainer)
	  {
		_DSM_Free(_cap.hContainer);
		_cap.hContainer = 0;
	  }

	  _cap.ConType = TWON_DONTCARE16;

	  // capability structure is set, make the call to the source now
	  TW_UINT16 twrc = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, _msg, (TW_MEMREF)&_cap);

	  switch(twrc)
	  {
		case TWRC_FAILURE:
			return TWCC_BUMMER;
	  }

	  return CondCode;
	}
	
	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper function to set a capability
	TW_UINT16 SetCAPOneValue(TW_UINT16 Cap, const int _value, TW_UINT16 _type)
	{
	  TW_INT16        twrc = TWRC_FAILURE;
	  TW_CAPABILITY   cap;
	         
	  cap.Cap         = Cap;
	  cap.ConType     = TWON_ONEVALUE;
	  cap.hContainer  = _DSM_Alloc(sizeof(TW_ONEVALUE));// Largest int size
	  if(0 == cap.hContainer)
	  {
		return twrc;
	  }

	  pTW_ONEVALUE pVal = (pTW_ONEVALUE)_DSM_LockMemory(cap.hContainer);

	  pVal->ItemType  = _type;
	  switch(_type)
	  {
		case TWTY_INT8:
		  *(TW_INT8*)&pVal->Item = (TW_INT8)_value;
		break;

		case TWTY_INT16:
		  *(TW_INT16*)&pVal->Item = (TW_INT16)_value;
		break;

		case TWTY_INT32:
		  *(TW_INT32*)&pVal->Item = (TW_INT32)_value;
		break;

		case TWTY_UINT8:
		  *(TW_UINT8*)&pVal->Item = (TW_UINT8)_value;
		break;

		case TWTY_UINT16:
		  *(TW_UINT16*)&pVal->Item = (TW_UINT16)_value;
		break;

		case TWTY_UINT32:
		  *(TW_UINT32*)&pVal->Item = (TW_UINT32)_value;
		break;

		case TWTY_BOOL:
		  *(TW_BOOL*)&pVal->Item = (TW_BOOL)_value;
		break;
	  }
	  // capability structure is set, make the call to the source now
	  twrc = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&(cap));
	  if(TWRC_CHECKSTATUS == twrc)
	  {

	  }
	  else if(TWRC_FAILURE == twrc)
	  {

	  }

	  _DSM_UnlockMemory(cap.hContainer);
	  _DSM_Free(cap.hContainer);
	  return twrc;
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper function to get a capability value from a capability structure
	bool GetCurrent(TW_CAPABILITY *pCap, TW_UINT32& val)
	{
	  bool bret = false;

	  if(0 != pCap->hContainer)
	  {
		if(TWON_ENUMERATION == pCap->ConType)
		{
		  pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(pCap->hContainer);
		  switch(pCapPT->ItemType)
		  {
		  case TWTY_INT32:
			val = (TW_INT32)((pTW_INT32)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_UINT32:
			val = (TW_INT32)((pTW_UINT32)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_INT16:
			val = (TW_INT32)((pTW_INT16)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_UINT16:
			val = (TW_INT32)((pTW_UINT16)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_INT8:
			val = (TW_INT32)((pTW_INT8)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_UINT8:
			val = (TW_INT32)((pTW_UINT8)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  case TWTY_BOOL:
			val = (TW_INT32)((pTW_BOOL)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
			bret = true;
			break;

		  }
		  _DSM_UnlockMemory(pCap->hContainer);
		}
		else if(TWON_ONEVALUE == pCap->ConType)
		{
		  pTW_ONEVALUE pCapPT = (pTW_ONEVALUE)_DSM_LockMemory(pCap->hContainer);
		  if(pCapPT->ItemType < TWTY_FIX32)
		  {
			val = pCapPT->Item;
			bret = true;
		  }
		  _DSM_UnlockMemory(pCap->hContainer);
		}
		else if(TWON_RANGE == pCap->ConType)
		{
		  pTW_RANGE pCapPT = (pTW_RANGE)_DSM_LockMemory(pCap->hContainer);
		  if(pCapPT->ItemType < TWTY_FIX32)
		  {
			val = pCapPT->CurrentValue;
			bret = true;
		  }
		  _DSM_UnlockMemory(pCap->hContainer);
		}
	  }

	  return bret;
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Helper function to query support for a capability
	TW_INT16 QuerySupportCAP(TW_UINT16 _cap, TW_UINT32 &_QS)
	{
	  TW_CAPABILITY   cap = {0};
	  cap.Cap         = _cap;
	  cap.hContainer  = 0;
	  cap.ConType     = TWON_ONEVALUE;
	  _QS             = 0;

	  // capability structure is set, make the call to the source now
	  TW_UINT16 twrc = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_QUERYSUPPORT, (TW_MEMREF)&cap);

	  switch(twrc)
	  {
	  case TWRC_SUCCESS:
		if(cap.ConType == TWON_ONEVALUE)
		{
		  pTW_ONEVALUE pVal = (pTW_ONEVALUE)_DSM_LockMemory(cap.hContainer);
		  _QS = pVal->Item;
		 _DSM_UnlockMemory(cap.hContainer);
		}
		_DSM_Free(cap.hContainer);
		break;
	  }

	  return twrc;
	}

	// (a.walling 2009-12-11 13:39) - PLID 36518 - Convert a ICAP_IMAGEFILEFORMAT value to a standard extension
	const char* convertICAP_IMAGEFILEFORMAT_toExt(const TW_UINT16 _unItem)
	{
	  const char* text;

	  switch(_unItem)
	  {
	  case TWFF_PICT:
		text = "pict";
		break;

	  case TWFF_BMP:
		text = "bmp";
		break;

	  case TWFF_XBM:
		text = "xbm";
		break;

	  case TWFF_JFIF:
		text = "jpg";
		break;

	  case TWFF_FPX:
		text = "fpx";
		break;

	  case TWFF_TIFF:
	  case TWFF_TIFFMULTI:
		text = "tiff";
		break;

	  case TWFF_PNG:
		text = "png";
		break;

	  case TWFF_SPIFF:
		text = "spiff";
		break;

	  case TWFF_EXIF:
		text = "exif";
		break;

	  case TWFF_JP2:
		text = "jp2";
		break;

	  case TWFF_JPX:
		text = "jpx";
		break;

	  case TWFF_DEJAVU:
		text = "dejavu";
		break;

	  case TWFF_PDF:
	  case TWFF_PDFA:
	  case TWFF_PDFA2:
		text = "pdf";
		break;
	  }

	  return text;
	}


	BOOL IsDSEnabled()
	{
		return (TWDSEnabled);
	}

	BOOL DisableDS()
	{
		TW_UINT16 twRC = TWRC_FAILURE;
		TW_USERINTERFACE twUI;
		HWND hWnd = GetTwainWindow();

		memset(&twUI, 0, sizeof(TW_USERINTERFACE));


		// only disable enabled Source's
		if (TWDSEnabled!=TRUE)
		{
/*			if (MessageLevel()  >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Cannot Disable Source\nSource Not Enabled", 
							"Sequence Error");
			}*/
		}
		else
		{
			twUI.hParent = hWnd;
			twUI.ShowUI = TWON_DONTCARE8;

			twRC = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL,
						DAT_USERINTERFACE,
						MSG_DISABLEDS,
						(TW_MEMREF)&twUI);

			if (twRC == TWRC_SUCCESS)
			{   
				//LogMessage("DS Disabled\r\n");

				TWDSEnabled = FALSE;
				/*if (MessageLevel() >= ML_FULL)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"Source Disabled",
								"DG_CONTROL/DAT_USERINTERFACE/MSG_DISABLEDS");
				}*/
			}
			else
			{
				/*if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, twRC, 1,
								"",
								"DG_CONTROL/DAT_USERINTERFACE/MSG_DISABLEDS");
				}*/
			}
		}    	
		return (twRC==TWRC_SUCCESS);
	} 

	BOOL CloseDSM()
	{
		TW_UINT16 twRC = TWRC_FAILURE;
		HWND hWnd = GetTwainWindow();
		char buffer[80];

		memset(buffer, 0, sizeof(char[80]));

		if (!TWDSMOpen)
		{
/*			if (MessageLevel()  >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"Cannot Close Source Manager\nSource Manager Not Open", 
							"Sequence Error");
			}*/
		}
		else
		{
			if (TWDSOpen==TRUE)
			{
/*				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"A Source is Currently Open", "Cannot Close Source Manager");
				}*/
			}
			else
			{
				// Only close something which is already open
				if (TWDSMOpen==TRUE)
				{
					// This call performs one important function:
					// - tells the SM which application, appID.id, is requesting SM to close
					// - be sure to test return code, failure indicates SM did not close !!

				twRC = CallDSMEntry(&appID,
									NULL,
									DG_CONTROL,
									DAT_PARENT,
									MSG_CLOSEDSM,
									&hWnd);

					if (twRC != TWRC_SUCCESS)
					{
						// Trouble closing the SM, inform the user
						/*if (MessageLevel() >= ML_ERROR)
						{
							ShowRC_CC(hWnd, 1, twRC, 0,
										"",
										"DG_CONTROL/DAT_PARENT/MSG_CLOSEDSM");
						}

						wsprintf(buffer,"CloseDSM failure -- twRC = %d\r\n",twRC);
						LogMessage(buffer);*/
					}
					else
					{
						TWDSMOpen = FALSE;
						// Explicitly free the SM library
						if (hDSMDLL)
						{        
							FreeLibrary (hDSMDLL);
							hDSMDLL=NULL;
							// the data source id will no longer be valid after
							// twain is killed.  If the id is left around the
							// data source can not be found or opened
							dsID.Id = 0;  
						}
						/*if (MessageLevel() >= ML_FULL)
						{
							ShowRC_CC(hWnd, 0, 0, 0,
										"Source Manager was Closed successfully", 
										"TWAIN Information");
						}*/
					}
				}
			}
		}
		// Let the caller know what happened
		return (twRC==TWRC_SUCCESS);
	} // TWCloseDSM

	BOOL OpenDSM()
	{
		#define			VALID_HANDLE    32      // valid windows handle SB >= 32
		#define       WINDIRPATHSIZE 160
		TW_UINT16     twRC = TWRC_FAILURE;
		OFSTRUCT      OpenFiles;
		char          WinDir[WINDIRPATHSIZE];
		TW_STR32      DSMName;
		HWND		  hWnd = GetTwainWindow();

		memset(&OpenFiles, 0, sizeof(OFSTRUCT));
		memset(WinDir, 0, sizeof(char[WINDIRPATHSIZE]));
		memset(DSMName, 0, sizeof(TW_STR32));

		// Only open SM if currently closed
		if (TWDSMOpen!=TRUE)
		{
			// Open the SM, Refer explicitly to the library so we can post a nice
			// message to the the user in place of the "Insert TWAIN.DLL in drive A:"
			// posted by Windows if the SM is not found.

			//DRT 3/22/2007 - PLID 25314 - GetWindowsDirectory() actually returns a private
			//	windows space if you are on a terminal server.  GetSystemWindowsDirectory()
			//	is actually the function we wish to call.  See MSDN documentation for 
			//	GetWindowsDirectory(), there is a remark about it.
			//GetWindowsDirectory (WinDir, WINDIRPATHSIZE);
			GetSystemWindowsDirectory(WinDir, WINDIRPATHSIZE);

			// Hardcode to seperate dca_main.c & dca_glue.c more completely
			 lstrcpy (DSMName, "TWAIN_32.DLL");

			if (WinDir[strlen(WinDir)-1] != '\\')
			{
				lstrcat (WinDir, "\\");
			}
			lstrcat (WinDir, DSMName);

			Log("Attempting to load TWAIN library at %s", WinDir);

			if ((OpenFile(WinDir, &OpenFiles, OF_EXIST) != -1) &&
					(hDSMDLL =     LoadLibrary(WinDir)) != NULL &&
					(hDSMDLL >= (HANDLE)VALID_HANDLE) &&
					(lpDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, (LPCTSTR)0x00000001)) != NULL)
			{
				// This call performs four important functions:
				//  	- opens/loads the SM
				//    	- passes the handle to the app's window to the SM
				//    	- returns the SM assigned appID.id field
				//    	- be sure to test the return code for SUCCESSful open of SM


				twRC = CallDSMEntry(&appID,
									NULL,
									DG_CONTROL,
									DAT_PARENT,
									MSG_OPENDSM,
									(TW_MEMREF)&hWnd);

				switch (twRC)
				{
					case TWRC_SUCCESS:
						// Needed for house keeping.  Do single open and do not
						// close SM which is not already open ....
						TWDSMOpen = TRUE;
						break;

					case TWRC_FAILURE:
						//LogMessage("OpenDSM failure\r\n");
					default:
						// Trouble opening the SM, inform the user
						TWDSMOpen = FALSE;
						if (MessageLevel() >= ML_ERROR)                
						{
							MsgBox("OpenDSM Failure");
//							ShowRC_CC(hWnd, 1, twRC, 0,	//Source Manager
//										"",
//										"DG_CONTROL/DAT_PARENT/MSG_OPENDSM");
						}
						break;
				}
			}
			else
			{
				if (MessageLevel() >= ML_ERROR)
				{
					MsgBox("Error loading TWAIN_32.DLL from the Windows system folder. This is most likely an indication that the software package that came with your input device (i.e. scanner or camera) needs to be installed or re-installed on this computer.");
/*					ShowRC_CC(hWnd, 0, 0, 0,
								"Error in Open, LoadLibrary, or GetProcAddress.\nTwain DLL may not exist.",
								#ifdef WIN32
									"TWAIN_32.DLL");
								#else
									"TWAIN.DLL");
								#endif*/
				}
			}
		}  
		// Let the caller know what happened
		return (TWDSMOpen);
	}

	////////////////////////////////////////////////////////////////////////
	// FUNCTION: TWEnableDS
	//
	// ARGS:    none
	//
	// RETURNS: BOOL for TRUE=open; FALSE=not open/fail
	//
	// NOTES:    1). only enable an open Source
	//           2). call the Source Manager to:
	//                - bring up the Source's User Interface
	//
	BOOL EnableDS (TW_BOOL Show)
	{
		BOOL Result = FALSE;
		TW_UINT16 twRC = TWRC_FAILURE;
		HWND hWnd = GetTwainWindow();

		if (TWDSOpen==FALSE)
		{
			if (MessageLevel() >= ML_ERROR)
			{
				MsgBox("Cannot enable source");
/*				ShowRC_CC(hWnd, 0, 0, 0,
							"Cannot Enable Source\nNo Source Open", 
							"TWAIN Error");*/
			}
		}
		else
		{	
			// only enable open Source's
			if (TWDSEnabled==TRUE)	//Source is alredy enabled
			{
/*				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"Cannot Enable Source, already enabled", 
								"TWAIN Error");
				}*/
			}
			else
			{
				// This will display the Source User Interface. The Source should only display
				// a user interface that is compatible with the group defined
				// by appID.SupportedGroups (in our case DG_IMAGE | DG_CONTROL)
				memset(&twUI, 0, sizeof(TW_USERINTERFACE));
				twUI.hParent = hWnd;
				twUI.ShowUI  = Show;

				twRC = CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL,
							DAT_USERINTERFACE,
							MSG_ENABLEDS,
							(TW_MEMREF)&twUI);

				if (twRC!=TWRC_SUCCESS)
				{
					// (a.walling 2008-09-26 17:15) - PLID 31498 - the WIA interface is modal above, as compared to most
					// dialogs which are modeless and return cancelled status via MSG_CLOSEDSREQ. So we need to notify that
					// the xfer is done (although no images were transferred)
					if (twRC == TWRC_CANCEL) {
						GetMainFrame()->PostMessage(NXM_TWAIN_XFERDONE);
					}
/*					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 1,
									"",
									"DG_CONTROL/DAT_USERINTERFACE/MSG_ENABLEDS");
					}*/
				}
				else
				{
					Result = TRUE;
					TWDSEnabled = TRUE;
/*					if (MessageLevel() >= ML_FULL)
					{
						if(twUI.ShowUI == TRUE)
						{
							ShowRC_CC(hWnd, 0, 0, 0,
										"Source Enabled",
										"DG_CONTROL/DAT_USERINTERFACE/MSG_ENABLEDS");
						}
					}*/
				}
			}
		}
		return Result;
	} 

	BOOL Initialize(void)
	{
		// (a.walling 2010-01-28 15:27) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		if (UsingTwain2()) {
			// (a.walling 2010-01-27 14:36) - PLID 36568 - Create our NxTWAIN interface
			g_pPracticeTwainInterface = new CPracticeTwainInterface();

			// (j.armen 2011-10-24 14:36) - PLID 46139 - No need to call GetCurrentDirectory.  
			// The current directory was saved to PracPath::SessionPath, so reference it from there.
			CString strTwainLogPath = GetPracPath(PracPath::SessionPath) ^ "NxTwain.log";
			g_pPracticeTwainInterface->Initialize(strTwainLogPath);

		} else {
			appID.Id = 0; 				// init to 0, but Source Manager will assign real value
			appID.Version.MajorNum = 1;
			appID.Version.MinorNum = 703;
			appID.Version.Language = TWLG_USA;
			appID.Version.Country  = TWCY_USA;
			// (b.cardillo 2005-02-11 13:29) - PLID 15179 - Got confirmation from c.haag that it was safe, 
			// even for existing clients, to change these two lines to both say "NexTech Practice".
			lstrcpy (appID.Version.Info,  "NexTech Practice");
			lstrcpy (appID.ProductName,   "NexTech Practice");
			appID.ProtocolMajor = 1;//TWON_PROTOCOLMAJOR;
			appID.ProtocolMinor = 7;//TWON_PROTOCOLMINOR;
			appID.SupportedGroups =  DG_IMAGE | DG_CONTROL;
			// (a.walling 2014-01-03 09:56) - PLID 59324 - User-visible NexTech Inc. should be NexTech LLC
			lstrcpy (appID.Manufacturer,  "NexTech Systems LLC");
			lstrcpy (appID.ProductFamily, "NexTech Systems LLC");
		}

		return 0;
	}

	/*
	* Function: Autofeed -- 
	* Author: TWAIN Working Group
	* Input:
	*		(None)
	* Output:
	*		TW_INT16
	* Comments:
	*		Checks if the autofeed option on the FILE menu is
	* checked and if so begins a capability negotiation.  If the ADF is already
	* enabled, (check this) the return value is ??? otherwise the application
	* negotiates with the source to enable the capability
	*
	* If the ADF at the source cannot be enabled, the application should post an 
	* error and not allow a transfer (ie: the Source's UI to come up).
	*/
	TW_INT16 Autofeed()
	{
		TW_CAPABILITY   cap;
		pTW_ONEVALUE    pval = NULL;
		TW_INT16        status = TWRC_SUCCESS;

		memset(&cap, 0, sizeof(TW_CAPABILITY));
		g_AutoFeedBOOL = FALSE;

		// Return success if the user does not want to use auto feed
		// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
		if (!g_bAutoFeed)
			return TWRC_SUCCESS;


		cap.Cap = CAP_FEEDERENABLED;
		cap.ConType = TWON_ONEVALUE;

		status = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL, 
						DAT_CAPABILITY, 
						MSG_GET,
						(TW_MEMREF)&cap);

		if (status != TWRC_SUCCESS)
		{
			status = TWRC_FAILURE;
			GlobalFree(cap.hContainer);
		}
		else
		{   
			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			if (pval->Item == TRUE)
			{   
				/*
				* Feeder is enabled no need to negotiate
				*/
				GlobalUnlock(cap.hContainer);
				GlobalFree((HANDLE)cap.hContainer); 
			}
			else     
			{
				/*
				* Negotiate with the source
				* Try to turn on CAP_FEEDERENABLED
				*/
				pval->ItemType = TWTY_BOOL;
				pval->Item = TRUE;
				GlobalUnlock(cap.hContainer);

				status = CallDSMEntry(&appID,
								&dsID,
								DG_CONTROL, 
								DAT_CAPABILITY, 
								MSG_SET,
								(TW_MEMREF)&cap);

				/*
				* free here because the GET call will allocate a new container
				*/
				GlobalFree(cap.hContainer);

				if (status == TWRC_SUCCESS)
				{   
					/*
					* Verify that CAP_FEEDERENABLED is now TRUE
					*/
					status = CallDSMEntry(&appID,
									&dsID,
									DG_CONTROL, 
									DAT_CAPABILITY, 
									MSG_GET,
									(TW_MEMREF)&cap);

					if (status == TWRC_SUCCESS)
					{
						pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
						if (pval->Item != TRUE) 
						{
							/*
							* Verification failed -- CAP_FEEDERENABLED is FALSE even after successful set to TRUE
							*/
							status = TWRC_FAILURE;
						}
						GlobalUnlock(cap.hContainer);
						GlobalFree((HANDLE)cap.hContainer);    
					}
					else
					{
						status = TWRC_FAILURE;                  
						//strcpy(Details, "CAP_FEEDERENABLED");                   
					}               
				}
				else
				{    
					/*
					* MSG_SET of CAP_FEEDERENABLED to TRUE did not succeed
					*/
					status = TWRC_FAILURE;
				}
			}

			if (status == TWRC_SUCCESS)
			{
				/*
				* CAP_AUTOFEED
				*/
				cap.Cap = CAP_AUTOFEED;
				cap.ConType = TWON_ONEVALUE;

				status = CallDSMEntry(&appID,
								&dsID,
								DG_CONTROL, 
								DAT_CAPABILITY, 
								MSG_GET,
								(TW_MEMREF)&cap);

				if (status != TWRC_SUCCESS)
				{
					/*
					* MSG_GET on CAP_AUTOFEED did not succeed
					*/
					status = TWRC_FAILURE;
				}

				/*
				* MSG_GET on CAP_AUTOFEED returned success
				*/
				pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
				if (pval->Item == TRUE)
				{
					/*
					* CAP_AUTOFEED is on
					*/
					GlobalUnlock(cap.hContainer);
					GlobalFree((HANDLE)cap.hContainer);         
				}
				else
				{
					/*
					* Try to set CAP_AUTOFEED to TRUE
					*/
					pval->ItemType = TWTY_BOOL;
					pval->Item = TRUE;
					GlobalUnlock(cap.hContainer);

					status = CallDSMEntry(&appID,
										&dsID,
										DG_CONTROL, 
										DAT_CAPABILITY, 
										MSG_SET,
										(TW_MEMREF)&cap);

					GlobalFree((HANDLE)cap.hContainer);     

					if (status == TWRC_SUCCESS)
					{   
						/*
						* Verify that CAP_AUTOFEED is on
						*/
						status = CallDSMEntry(&appID,
										&dsID,
										DG_CONTROL, 
										DAT_CAPABILITY, 
										MSG_GET,
										(TW_MEMREF)&cap);

						if (status == TWRC_SUCCESS)
						{
							pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
							if (pval->Item != TRUE)
							{
								status = TWRC_FAILURE;
								/*
								* CAP_AUTOFEED returns FALSE even after successful set to TRUE
								*/
							}
							GlobalUnlock(cap.hContainer);
							GlobalFree((HANDLE)cap.hContainer); 
						}
						else
						{
							status = TWRC_FAILURE;
						}
					}
					else
					{
						/*
						* MSG_SET of CAP_AUTOFEED to TRUE did not succeed
						*/
						status = TWRC_FAILURE;
					}
				}   
			}   
		}   

		/*
		* Set Local Autofeed Variable on/off        
		*/
		if (status == TWRC_SUCCESS)
		{
			g_AutoFeedBOOL = TRUE;
		}
		else
		{
			g_AutoFeedBOOL = FALSE;
		}
		return status;
	}  


	BOOL SetImageFormat(EimgType type)
	{
		TW_CAPABILITY		cap;
		pTW_ONEVALUE		pval = NULL;
		TW_UINT16 val;

		val = TWFF_BMP;
		
		/*
		*	Initialize all structures
		*/
		memset(&cap, 0, sizeof(TW_CAPABILITY));

		cap.Cap = ICAP_IMAGEFILEFORMAT;
		cap.ConType = TWON_ONEVALUE;

		/*
		* alloc the container
		*/
		if (cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE)))
		{
			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			pval->ItemType = TWTY_UINT16;
			pval->Item = val;

			/*
			* file transfer currently fixed to bitmap format
			*
			* get the filename to save as
			* check formats supported by the source            
			*/

			GlobalUnlock(cap.hContainer);

			if (TWRC_SUCCESS == CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL, 
							DAT_CAPABILITY, 
							MSG_SET,
							(TW_MEMREF)&cap))
			{
				if (val == TWFF_JFIF)
				{
					cap.Cap = ICAP_COMPRESSION;
					pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
					pval->ItemType = TWTY_UINT16;
					pval->Item = TWCP_JPEG;
					GlobalUnlock(cap.hContainer);

					if (TWRC_SUCCESS == CallDSMEntry(&appID,
									&dsID,
									DG_CONTROL, 
									DAT_CAPABILITY, 
									MSG_SET,
									(TW_MEMREF)&cap))
					{
						GlobalFree((HANDLE)cap.hContainer);
						return FALSE;
					}
				}
			}

			GlobalFree((HANDLE)cap.hContainer);
		}
		return TRUE;
	}
	
	// (a.walling 2009-12-10 08:30) - PLID 36518 - Determine what our current transfer mechanism is
	TW_UINT32 GetXferMech()
	{
		TW_UINT32  mech;
		TW_CAPABILITY  Cap;

		Cap.Cap = ICAP_XFERMECH;
		Cap.hContainer = 0;

		if(TWCC_SUCCESS != GetCAP(Cap, MSG_GETCURRENT) || !GetCurrent(&Cap, mech) )
		{
			return TWSX_NATIVE;
		}

		if(Cap.hContainer)
		{
			_DSM_Free(Cap.hContainer);
		}

		return mech;
	}

	// (a.walling 2009-12-10 08:30) - PLID 36518 - Set the transfer mechanism
	TW_INT16 SetXferMech(TW_UINT32 mech)
	{
		TW_CAPABILITY   cap;
		pTW_ONEVALUE    pval = NULL;
		TW_INT16        status = TWRC_FAILURE;  
		HWND hWnd = GetTwainWindow();

		/*
		*	Initialize all structures
		*/
		memset(&cap, 0, sizeof(TW_CAPABILITY));

		cap.Cap = ICAP_XFERMECH;
		cap.ConType = TWON_ONEVALUE;

		/*
		* alloc the container
		*/
		if (cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE)))
		{
			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			pval->ItemType = TWTY_UINT16;

			/*
			* native transfer
			*/
			//
			// (c.haag 2006-07-18 17:00) - PLID 21505 - We now do native transfers, which means
			// we get a handle to a DIB instead of raw pixels
			//
			pval->Item = mech;

			/*
			* check formats supported by the source            
			*/
			GlobalUnlock(cap.hContainer);

			status = CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL, 
							DAT_CAPABILITY, 
							MSG_SET,
							(TW_MEMREF)&cap);

			GlobalFree((HANDLE)cap.hContainer);

/*			if (status != TWRC_SUCCESS)
			{
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, status, 1, "","MSG_SET of ICAP_XFERMECH"); 
				}
			}*/
		}
		else
		{
			status = TWRC_FAILURE;
			/*if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0, "Memory Allocation Failed","MSG_SET of ICAP_XFERMECH"); 
			}*/
		}
		return status;
	}
	
	// (a.walling 2009-12-10 08:30) - PLID 36518 - Try to determine the best xfer method. Use file if necessary, otherwise return false so the caller will fallback to native
	bool TrySetFileXfer(CNxTwain* pTwain)
	{
		if (TWRC_SUCCESS != SetXferMech(TWSX_FILE)) {
			return false;
		}

		// first we'll try to see what file formats are available to us
		TW_UINT32 QS = 0;
		QuerySupportCAP(ICAP_IMAGEFILEFORMAT, QS);

		if ( (QS & TWQC_GET) && (QS & TWQC_SET) ) {

			TW_CAPABILITY  Cap;
			TW_INT16        status = TWRC_FAILURE;

			Cap.Cap = ICAP_IMAGEFILEFORMAT;
			Cap.hContainer = 0;

			if (TWCC_SUCCESS != GetCAP(Cap, MSG_GET))
			{
				// error
				return false;
			} else {
				if (Cap.ConType == TWON_ENUMERATION) {
					pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(Cap.hContainer);

					CDWordArray dwaFormats;

					for(TW_UINT32 x=0; x<pCapPT->NumItems; ++x)
					{
						TW_UINT16 supportedFileFormat = ((pTW_UINT16)(&pCapPT->ItemList))[x];
						//TRACE("Supported file format %li\n", (long)supportedFileFormat);
						Log("Supports file format %li - %s", (long)supportedFileFormat, convertICAP_IMAGEFILEFORMAT_toExt(supportedFileFormat));
						dwaFormats.Add(supportedFileFormat);
					}

					_DSM_UnlockMemory(Cap.hContainer);

					if(Cap.hContainer)
					{
						_DSM_Free(Cap.hContainer);
					}


					int desiredFormat = -1;
					int i = 0;
					if (pTwain->m_eTargetFormat == eScanToPDF) {
						// want to scan to PDF, can we support it?
						for (i = 0; i < dwaFormats.GetSize(); i++) {
							if (dwaFormats[i] == TWFF_PDF || dwaFormats[i] == TWFF_PDFA || dwaFormats[i] == TWFF_PDFA2) {
								if ((int)dwaFormats[i] > desiredFormat) {
									desiredFormat = dwaFormats[i];
								}
							}
						}

						if (desiredFormat == -1) {
							// dang, no PDF, so we must use our own internal PDF creator, which limits us to JPEG and PNG, so let's just 
							// use Native transfer and decide which to use based on color and other info once we get it.
							return false;
						} else {
							if (TWRC_SUCCESS != SetCAPOneValue(ICAP_IMAGEFILEFORMAT, desiredFormat, TWTY_UINT16)) {
								return false;
							}
						}
					} else if (pTwain->m_eTargetFormat == eScanToMultiPDF) {
						// for multi PDF, we are limited to JPEG and PNG
						Cap.Cap = ICAP_IMAGEFILEFORMAT;
						Cap.hContainer = 0;

						if (TWCC_SUCCESS != GetCAP(Cap, MSG_GET)) {
							return false;
						}

						TW_UINT32 currentFormat = 0;
						bool bGotValue = GetCurrent(&Cap, currentFormat);
						
						if(Cap.hContainer)
						{
							_DSM_Free(Cap.hContainer);
						}

						if (!bGotValue) {
							return false;
						}

						
						Log("Current file format %li - %s", (long)currentFormat, convertICAP_IMAGEFILEFORMAT_toExt((TW_UINT16)currentFormat));

						if (currentFormat == TWFF_JFIF || currentFormat == TWFF_PNG) {
							// we are already good, let's assume the driver knows what it is doing
							
							

							return true;
						}

						// otherwise, we don't know which to get. So, we'll just get a JPEG.
						for (i = 0; desiredFormat == -1 && i < dwaFormats.GetSize(); i++) {
							if (dwaFormats[i] == TWFF_JFIF) {
								desiredFormat = dwaFormats[i];
							}
						}

						if (desiredFormat == -1) {
							// use Native transfer and decide which to use based on color and other info once we get it.
							return false;
						} else {
							if (TWRC_SUCCESS != SetCAPOneValue(ICAP_IMAGEFILEFORMAT, desiredFormat, TWTY_UINT16)) {
								return false;
							}

							return true;
						}
					} else {
						// we are scanning to an image, not for use in a PDF, so we'll use whatever is default assuming we actually have one
						// as long as it is a format that makes some bit of sense to us

						Cap.Cap = ICAP_IMAGEFILEFORMAT;
						Cap.hContainer = 0;

						if (TWCC_SUCCESS != GetCAP(Cap, MSG_GET)) {
							return false;
						}

						TW_UINT32 currentFormat = 0;
						bool bGotValue = GetCurrent(&Cap, currentFormat);

						if(Cap.hContainer)
						{
							_DSM_Free(Cap.hContainer);
						}

						if (!bGotValue) {
							return false;
						}

						switch (currentFormat) {
							case TWFF_TIFF:
							case TWFF_JFIF:
							case TWFF_TIFFMULTI:
							case TWFF_PNG:
							case TWFF_SPIFF:
							case TWFF_PDF:
							case TWFF_JP2:
							case TWFF_JPX:
							case TWFF_DEJAVU:
							case TWFF_PDFA:
							case TWFF_PDFA2:
								return true;
							default:
								// not a file format we should be using, so use native transfer
								return false;
						}
					}
				} else {
					if(Cap.hContainer)
					{
						_DSM_Free(Cap.hContainer);
					}
				}
			}
		}

		return false;
	}
	

	TW_INT16 XferMech(CNxTwain* pTwain)
	{
		// (a.walling 2009-12-15 17:07) - PLID 36518 - The fujitsu scanner is ignoring the file transfer setup message and scanning in the wrong format 
		// and in the wrong location. So, I am scrapping the file transfer options for now until we have a chance to investigate the strange behavior 
		// on other scanners. This will still save the images in a much more optimized format, but we will not rely on scanners to format things
		// for us just yet.
		/*
		if (pTwain->m_eTargetFormat != eScanToNative && (FALSE == GetRemotePropertyInt("TWAINDisableFileXfer", FALSE, 0, "<None>")) ) {			
			Log("Attempting file transfer");
			if (TrySetFileXfer(pTwain)) {
				return TWRC_SUCCESS;
			}
		}
		*/

		// fallback to native
		Log("Setting native transfer");
		return SetXferMech(TWSX_NATIVE);
	}

	BOOL OpenDS()
	{
		TW_UINT16 twRC = TWRC_FAILURE;

		if (TWDSMOpen==FALSE)
			return FALSE;

		// This will open the TWAIN source.
		twRC = CallDSMEntry(&appID,
							NULL,
						DG_CONTROL,
						DAT_IDENTITY,
						MSG_OPENDS,
						&dsID);

		switch (twRC)
		{
			case TWRC_SUCCESS:
				LogDetail("OpenDS success\r\n");

				// do not change flag unless we successfully open
				TWDSOpen = TRUE;
				break;

			case TWRC_FAILURE:
				LogDetail("OpenDS failure\r\n");

				// Trouble opening the Source
				char Details[255];
				char Details2[1024];

				memset(Details, 0, sizeof(char[255]));
				memset(Details2, 0, sizeof(char[1024]));

				MatchTwainInt(ReturnCode, MAX_RETURNCODE, (TW_INT32)twRC, 
											Details2);

				MatchTwainInt(ConditionCode,MAX_CONDITIONCODE, 
											(TW_INT32)gGlobalStatus.ConditionCode, 
											Details);

				// (a.walling 2008-07-25 16:34) - PLID 30836 - We should know that our xfer is done, even if it was 0 images.
				GetMainFrame()->PostMessage(NXM_TWAIN_XFERDONE);

				LogDetail("Practice was unable to access the input device that is selected for data acquisition. Please go to Tools=>Scanner/Digital Camera Settings=>Select Source menu and ensure that one item is selected. If there are no items to choose from, please install or troubleshoot the software package that came with your scanner, camera or other hardware.\n\nReturn code: %s/%s",
					Details2, Details);
				MsgBox("Practice was unable to access the input device that is selected for data acquisition. Please go to Tools=>Scanner/Digital Camera Settings=>Select Source menu and ensure that one item is selected. If there are no items to choose from, please install or troubleshoot the software package that came with your scanner, camera or other hardware.\n\nReturn code: %s/%s",
					Details2, Details);
				break;

			default:
				// (a.walling 2008-07-25 16:34) - PLID 30836 - We should know that our xfer is done, even if it was 0 images.
				GetMainFrame()->PostMessage(NXM_TWAIN_XFERDONE);

				LogDetail("The data acquisition failed with an unknown error message (0x%08x)", twRC);
				MsgBox("The data acquisition failed with an unknown error message (0x%08x)", twRC);
				break;
		}
		return TWDSOpen;
	}

	BOOL CloseDS (VOID)
	{
		TW_UINT16 twRC = TWRC_FAILURE;

		if (!TWDSOpen)
		{
			return TRUE;
		}
		else
		{
			if (TWDSEnabled == TRUE)
			{				
				LogDetail("Failed to close TWAIN DS");
/*				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"Source is Currently Enabled", "Cannot Close Source");
				}*/
				return FALSE;
			}
			else
			{
				if (TWDSOpen==TRUE)
				{
					// Close an open Source
					twRC = CallDSMEntry(&appID,
									NULL,
									DG_CONTROL,
									DAT_IDENTITY,
									MSG_CLOSEDS,
									&dsID);

					// show error on close
					if (twRC != TWRC_SUCCESS) 
					{
						LogDetail("Failed to close TWAIN DS");
/*						if (MessageLevel() >= ML_ERROR)
						{
							ShowRC_CC(hWnd, 1, twRC, 0,
										"",
										"DG_CONTROL/DAT_IDENTITY/MSG_CLOSEDS");
						}*/
					} 
					else 
					{
						TWDSOpen = FALSE;
						dsID.Id = 0;
						dsID.ProductName[0] = 0;
					}

				}
			}
		}
		return(twRC==TWRC_SUCCESS);
	}

	void CloseConnection()
	{
		// (a.walling 2010-01-28 15:27) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		// (a.walling 2010-04-29 17:57) - PLID 38438 - Just check for the existence of the interface rather than call a remote prop (UsingTwain2)
		if (g_pPracticeTwainInterface) {
			// (a.walling 2010-01-27 14:37) - PLID 36568
			g_pPracticeTwainInterface->Destroy();
			delete g_pPracticeTwainInterface;
			g_pPracticeTwainInterface = NULL;
		} else {
			if (g_bTerminateImmediately)
				return;

			if (DisableDS())
			{
				if (CloseDS())
				{
					CloseDSM();
				}
			}
			if(g_pTWAINThread) g_pTWAINThread->PostThreadMessage(WM_QUIT, 0, 0);
			g_pTWAINThread = NULL;
		}
	}

	// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection
	void Attach(ADODB::_Connection* lpCon, const CString &path, BOOL bAttachChecksum /*= FALSE*/, long nChecksum /*= 0*/, LPCTSTR strSelection = SELECTION_FILE /* (historydlg.h) */)
	{
		try {
			CString strPathName;
			if (GetFilePath(path).CompareNoCase(g_strDocumentPath) == 0) {
				strPathName = GetFileName(path);
			} else {
				strPathName = path;
			}
			// Insert the record in MailSent
			// (m.hancock 2006-06-27 15:53) - PLID 21071 - Added field for associating MailSent records with lab step records
			// (a.walling 2008-09-03 11:44) - PLID 19638 - Pass in the PIC id as well
			// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection to the standard CreateNewMailSentEntry function

			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);

			BOOL bIsPhoto = FALSE;
			if (g_bIsPhotoJPEG && IsFileType(strPathName, ".jpg")) {
				bIsPhoto = TRUE;
			} else if (g_bIsPhotoPNG && IsFileType(strPathName, ".png")) {
				bIsPhoto = TRUE;
			}

			CreateNewMailSentEntry(lpCon, g_nPersonID, strPathName, strSelection, strPathName, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, bAttachChecksum ? nChecksum : -1, g_nCategoryID, g_nPicID, g_nLabStepID, bIsPhoto, -1, "", ctNone);

			// (a.walling 2008-09-03 11:42) - PLID 19638 - need to send an EMR tablechecker if in a PIC that has an EMR
			if (g_nPicID != -1) {
				CClient::RefreshTable(NetUtils::EMRMasterT, g_nPersonID);
			}
		}NxCatchAll("Error in attaching file.");
		// (a.walling 2008-01-29 14:31) - PLID 28716 - This has never worked correctly. Look carefully and you'll see why:
		// the result of ReverseFind should not be subtracted from the Length. However this is being removed entirely.
		// SetPropertyText("Attach History Path", path.Left(path.GetLength() - path.ReverseFind('\\')));
	}

	// (a.walling 2008-09-05 13:52) - PLID 22821 - Get temp output filename
	CString GetTempOutputFilename()
	{
		return GetNxTempPath() ^ FormatString("ScannedTempFile%lu.jpg", GetUniqueSessionNum());
	}

	// (a.walling 2008-09-05 13:52) - PLID 22821 - Get temp output filename
	// (a.walling 2009-12-10 16:20) - PLID 36518
	CString GetTempOutputFilename(const char* szExtension)
	{
		return GetNxTempPath() ^ FormatString("ScannedTempFile%lu.%s", GetUniqueSessionNum(), szExtension);
	}

	// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection
	CString GetStandardOutputFilename(ADODB::_Connection* lpCon)
	{
		// (c.haag 2003-10-24 10:16) - Apparently GetPatientDocumentName works
		// for contacts too!
		// (a.walling 2008-07-24 14:48) - PLID 30836 - Prevent recordsets
		if (g_nPersonID == -25) {
			return g_strDocumentPath ^ FormatString("ScannedDocument%lu.jpg", GetUniqueSessionNum()); // should be temp folder
		} else {
			if (g_eTargetFormat == eScanToPDF || g_eTargetFormat == eScanToMultiPDF) {
				return g_strDocumentPath ^ GetPatientDocumentName(lpCon, g_nPersonID, "pdf");
			} else {
				return g_strDocumentPath ^ GetPatientDocumentName(lpCon, g_nPersonID, "jpg");
			}
		}
	}

	// (a.walling 2009-12-10 16:20) - PLID 36518 - Support passing in an extension
	// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection
	CString GetStandardOutputFilename(ADODB::_Connection* lpCon, const char* szExtension)
	{
		// (c.haag 2003-10-24 10:16) - Apparently GetPatientDocumentName works
		// for contacts too!
		// (a.walling 2008-07-24 14:48) - PLID 30836 - Prevent recordsets
		if (g_nPersonID == -25) {
			return g_strDocumentPath ^ FormatString("ScannedDocument%lu.%s", GetUniqueSessionNum(), szExtension); // should be temp folder
		} else {
			return g_strDocumentPath ^ GetPatientDocumentName(lpCon, g_nPersonID, szExtension);
		}
	}

	// (a.walling 2008-09-09 15:47) - PLID 30389 - Allow passing in an override for personID
	// (a.walling 2010-01-28 13:45) - PLID 28806 - Must pass in a connection
	CString GetUserDefinedOutputFilename(ADODB::_Connection* lpCon, long nPersonID, CString strExt)
	{
		if (nPersonID == -1) {
			nPersonID = g_nPersonID;
		}
		// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
		CString strPath = g_strRemoteFolder;
		CString strID;

		///////////////////////////////////////////////////////////////////////////
		// Change the path variables into patient information

		// Patient ID
		strID.Format("%d", nPersonID);
		strPath.Replace("%ID%", strID);

		// Patient name
		try {
			_RecordsetPtr prs = CreateRecordset(lpCon, "SELECT First, Middle, Last FROM PersonT WHERE ID = %d", nPersonID);
			if (!prs->eof)
			{
				strPath.Replace("%FIRST%", AdoFldString(prs->Fields->Item["First"]));
				strPath.Replace("%MIDDLE%", AdoFldString(prs->Fields->Item["Middle"]));
				strPath.Replace("%LAST%", AdoFldString(prs->Fields->Item["Last"]));

				if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strPath))
					CreateDirectory(strPath, NULL);
			}
		} NxCatchAll("Error getting user-defined output filename");
		
		if (strExt.IsEmpty()) {
			if (g_eTargetFormat == eScanToPDF || g_eTargetFormat == eScanToMultiPDF) {
				return strPath ^ GetPatientDocumentName(lpCon, nPersonID, "pdf");
			} else {
				return strPath ^ GetPatientDocumentName(lpCon, nPersonID, "jpg");
			}
		} else {
			return strPath ^ GetPatientDocumentName(lpCon, nPersonID, strExt);
		}
	}

	
	// (a.walling 2009-12-10 16:20) - PLID 36518 - Save the bitmap to a temp file. Basically, use a JPEG if true color, or a PNG if 256 or less. Return the filename in strTempFilename
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	bool SaveToTempFile(CxImage& cxImage, const LPBITMAPINFOHEADER pbi, CString& strTempFilename)
	{
		if (pbi == NULL) {
			return false;
		}
		// this is a less-than-ideal solution. There is a lot more I want to do. But, until I have a real scanner
		// to develop and test with, I don't want to be too risky. I am a bit concerned about the DIB->Gdiplus::Bitmap->CxImage
		// conversions, but it seems to work out alright, though since there is little documentation about the internal workings
		// of GDI+ I fear it will silently mess with the bitdepth in some situations. Still, this definitely seems to be a better
		// approach than previously, especially since the file transfer method will delegate a lot of this behavior to the driver.

		// (a.walling 2010-04-08 08:29) - PLID 38170 - We are going back to CxImage since we have much more control and knowledge over its internal machinations

		bool bSuccess = false;

		/*
		DWORD dwError = 0;
		Gdiplus::Status s = Gdiplus::Ok;

		if (pbi->biClrUsed != 0) {
			// this is an indexed color image

			HBITMAP hbm;
			if (Gdiplus::Ok == (s = pBitmap->GetHBITMAP(Gdiplus::Color::Black, &hbm))) {
				CxImage img;
				if (img.CreateFromHBITMAP(hbm)) {
					// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
					if (pbi->biClrUsed == 256 && g_bSave8BitAsJPEG ) {
						if (!img.IsGrayScale()) {
							img.GrayScale();
						}
						if (img.GetBpp() > 8) {
							img.DecreaseBpp(8, false);
						}

						// save as a grayscale jpeg

						strTempFilename = GetTempOutputFilename("jpg");

						DWORD dwCodecOptions = img.GetCodecOption(CXIMAGE_FORMAT_JPG);
						
						const DWORD CXIMAJPG_ENCODE_GRAYSCALE = 0x4;
						dwCodecOptions |= CXIMAJPG_ENCODE_GRAYSCALE;
						// Q75 is much better compressed than the default (90) and subimage artifacts are barely noticable until you get down to Q50 or Q60
						img.SetJpegQuality(75);
						//img.SetCodecOption(dwCodecOptions, CXIMAGE_FORMAT_JPG);
						if (!img.Save(strTempFilename, CXIMAGE_FORMAT_JPG)) {
							Log("Could not save image to grayscale JPG: %s", img.GetLastError());
						} else {
							bSuccess = true;
						}
					} else {
						if (pbi->biClrUsed == 256 && img.GetBpp() > 8) {
							img.DecreaseBpp(8, false);
						} else if (pbi->biClrUsed == 16 && img.GetBpp() > 4) {
							img.DecreaseBpp(4, false);
						} else if (pbi->biClrUsed == 2 && img.GetBpp() > 1) {
							img.DecreaseBpp(1, false);
						}

						// less than 256 colors, so let's save as a PNG
						strTempFilename = GetTempOutputFilename("png");
						if (!img.Save(strTempFilename, CXIMAGE_FORMAT_PNG)) {
							Log("Could not save image to PNG: %s", img.GetLastError());
						} else {
							bSuccess = true;
						}
					}
				} else {
					Log("Could not load hbitmap: %s", img.GetLastError());
					bSuccess = false;
					dwError = GetLastError();
				}

				DeleteObject(hbm);
			} else {
				if (s == Gdiplus::Win32Error) dwError = GetLastError();
				bSuccess = false;
			}
		}

		// if this is a full-color image, or we have already tried and failed the CxImage approach, fallback to the gdi+ approach.
		if (!bSuccess) {
			strTempFilename = GetTempOutputFilename("jpg");

			// Q75 is much better compressed than the default (90) and subimage artifacts are barely noticable until you get down to Q50 or Q60
			s = NxGdi::SaveToJPEG(pBitmap, strTempFilename, 75);
			if (s == Gdiplus::Win32Error) dwError = GetLastError();
			bSuccess = (Gdiplus::Ok == s);
		}
		*/

		if (cxImage.GetBpp() <= 8) {
			// indexed image
			if (g_bSave8BitAsJPEG && cxImage.GetBpp() == 8 && cxImage.IsGrayScale()) {

				// save as a grayscale jpeg

				strTempFilename = GetTempOutputFilename("jpg");

				DWORD dwCodecOptions = cxImage.GetCodecOption(CXIMAGE_FORMAT_JPG);
				
				const DWORD CXIMAJPG_ENCODE_GRAYSCALE = 0x4;
				dwCodecOptions |= CXIMAJPG_ENCODE_GRAYSCALE;
				// Q75 is much better compressed than the default (90) and subimage artifacts are barely noticable until you get down to Q50 or Q60
				cxImage.SetJpegQuality(75);
				cxImage.SetCodecOption(dwCodecOptions, CXIMAGE_FORMAT_JPG);
				if (!cxImage.Save(strTempFilename, CXIMAGE_FORMAT_JPG)) {
					Log("Could not save image to grayscale JPG: %s", cxImage.GetLastError());
				} else {
					bSuccess = true;
				}
			}

			if (!bSuccess) {
				// let's save this image as a PNG
				
				strTempFilename = GetTempOutputFilename("png");
				if (!cxImage.Save(strTempFilename, CXIMAGE_FORMAT_PNG)) {
					Log("Could not save image to PNG: %s", cxImage.GetLastError());
				} else {
					bSuccess = true;
				}
			}
		}
		
		if (!bSuccess) {
			// full color image, or we have failed to save as a png above
			strTempFilename = GetTempOutputFilename("jpg");
			
			// Q75 is much better compressed than the default (90) and subimage artifacts are barely noticable until you get down to Q50 or Q60
			cxImage.SetJpegQuality(75);
			if (!cxImage.Save(strTempFilename, CXIMAGE_FORMAT_JPG)) {
				Log("Could not save image to JPG: %s", cxImage.GetLastError());
			} else {
				bSuccess = true;
			}
		}

		if (bSuccess) {
			Log("Temp file %s saved successfully.", strTempFilename);
			return true;
		} else {
			// failures logged above
			strTempFilename.Empty();
			return false;
		}
	}

	// (a.walling 2008-09-03 13:11) - PLID 22821 - Use Gdiplus bitmap instead of CxImage
	// (a.walling 2009-12-11 13:45) - PLID 36518 - Pass in the already-saved filename if we use a file xfer method
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	BOOL SaveNativeImage(ADODB::_Connection* lpCon, CxImage& cxImage, const LPBITMAPINFOHEADER pbi,
		CNxTwainProgressDlg& dlgProgress, long nCurTransfer, CStringArray& saTempPdfFiles, const CString& strAlreadySavedFileName)
	{
		LPBITMAPINFO pbmi = (LPBITMAPINFO)pbi;
		CString strProgressText;
		CString strTempFilename;

		bool bHasDIB = true;
		
		

		// (a.walling 2008-09-05 13:19) - PLID 22821 - Prepare temp file
		// (a.walling 2009-12-10 17:28) - PLID 36518 - Always use a temp file
		if (!strAlreadySavedFileName.IsEmpty()) {
			bHasDIB = false;
			strTempFilename = strAlreadySavedFileName;
		} else if (!SaveToTempFile(cxImage, pbi, strTempFilename)) {
			return FALSE;
		}
		
		saTempPdfFiles.Add(strTempFilename);

		if (g_eTargetFormat == eScanToMultiPDF) {
			return TRUE; // we handle saving after all these
		}
		
		CString strExtension = FileUtils::GetFileExtension(strTempFilename);
		strExtension.MakeLower();

		// (a.walling 2009-12-11 13:46) - PLID 36518 - Use an appropriate extension
		CString strStdFilename;
		if (g_eTargetFormat == eScanToPDF) {
			strStdFilename = GetStandardOutputFilename(lpCon, "pdf");
		} else {
			strStdFilename = GetStandardOutputFilename(lpCon, strExtension);
		}

		//
		// Scan to the patient's document folder if necessary
		//
		// (a.walling 2008-07-24 16:27) - PLID 30836 - If we are scanning to a temp folder (no patient ID) then 
		// we must always at least save something regardless of what their preference is.
		// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
		if (g_nPersonID == -25 || g_bScanToDocumentFolder)
		{
			strProgressText.Format("Saving image #%d...", nCurTransfer + 1, FileUtils::GetFileName(strStdFilename));
			dlgProgress.SetProgressText(strProgressText);
			LogDetail("Scanning in downloaded image '%s' for patient %d", strStdFilename, g_nPersonID);

			//TES 3/31/2004: Let them abort before the time-consuming compression.
			BOOL bAttach = TRUE;
			BOOL bAttachChecksum = FALSE;
			long nChecksum = 0;
			if(g_cbPrecompress && bHasDIB) g_cbPrecompress(pbmi, bAttach, bAttachChecksum, nChecksum, lpCon);
			if(bAttach) {
				if (g_eTargetFormat == eScanToPDF) {
					// (a.walling 2008-09-05 13:19) - PLID 22821 - Create target PDF
					try {
						NxPdf::IPdfDocumentPtr pPdfDocument;
						HRESULT hr = pPdfDocument.CreateInstance("NxPdf.PdfDocument");
						if (pPdfDocument) {
							// (a.walling 2008-09-08 12:58) - PLID 31293 - Apply PDF preferences
							// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
							pPdfDocument->AutoLandscape = g_bPDFAutoLandscape ? VARIANT_TRUE : VARIANT_FALSE;
							pPdfDocument->UseThumbs = g_bPDFUseThumbs ? VARIANT_TRUE : VARIANT_FALSE;
							pPdfDocument->PageSize = g_nPDFPageSize;
							// (a.walling 2009-12-11 13:45) - PLID 36518 - Support PNG and JPEG for our internal PDF creations
							if (strExtension == "png") {
								pPdfDocument->AddNewPageFromPNG(_bstr_t(strTempFilename));
							} else {
								pPdfDocument->AddNewPageFromJPEG(_bstr_t(strTempFilename));
							}
							pPdfDocument->SaveToFile(_bstr_t(strStdFilename));							
							Log("File %s saved successfully.", strStdFilename);
							if (g_cb && bHasDIB) g_cb(eScanToPatientFolder, strStdFilename, bAttach, g_pUserData, cxImage);
							if (bAttach) Attach(lpCon, strStdFilename, bAttachChecksum, nChecksum); /* Now attach the saved dib to the patient */
						} else {
							_com_issue_error(hr);
						}
					} NxCatchAllThread("Error generating PDF file!");
				} else if (g_eTargetFormat == eScanToImage || g_eTargetFormat == eScanToNative) {
					if (CopyFile(strTempFilename, strStdFilename, TRUE)) {
						Log("File %s saved successfully.", strStdFilename);
						if (g_cb && bHasDIB) g_cb(eScanToPatientFolder, strStdFilename, bAttach, g_pUserData, cxImage);
						if (bAttach) Attach(lpCon, strStdFilename, bAttachChecksum, nChecksum); /* Now attach the saved dib to the patient */
					} else {
						Log("Failed to save %s. Error code: %li", strStdFilename, GetLastError());
					}
				} else {
					ASSERT(FALSE);
				}
			}
		}
		//
		// Scan to the user-defined folder if necessary
		//
		// (a.walling 2008-07-24 16:27) - PLID 30836 - No need to do this if we are just scanning to a temp directory atm.
		if (g_nPersonID != -25) {
			// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
			if (g_bScanToRemoteFolder)
			{
				// (a.walling 2009-12-11 13:46) - PLID 36518 - Use an appropriate extension
				CString strUserDefinedFilename;
				if (g_eTargetFormat == eScanToPDF) {
					strUserDefinedFilename = GetUserDefinedOutputFilename(lpCon, -1, "pdf");
				} else {
					strUserDefinedFilename = GetUserDefinedOutputFilename(lpCon, -1, strExtension);
				}

				BOOL bCopySuccess = FALSE;
				// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
				if (g_bScanToDocumentFolder)
				{
					if (CopyFile(strStdFilename, strUserDefinedFilename, FALSE))
					{
						Log("File %s saved successfully.", strUserDefinedFilename);
						BOOL bAttach = TRUE;
						bCopySuccess = TRUE;
						// (a.walling 2008-07-24 13:28) - PLID 30836 - Also pass the image in memory
						if (g_cb && bHasDIB) g_cb(eScanToRemoteFolder, strUserDefinedFilename, bAttach, g_pUserData, cxImage);
						if (bAttach) Attach(lpCon, strUserDefinedFilename, FALSE, 0);
					}
				}
				if (!bCopySuccess)
				{
					strProgressText.Format("Saving image #%d to %s...", nCurTransfer + 1, FileUtils::GetFileName(strUserDefinedFilename));
					dlgProgress.SetProgressText(strProgressText);
					LogDetail("Scanning in downloaded image '%s' for patient %d", strUserDefinedFilename, g_nPersonID);
				
					//TES 3/31/2004: Let them abort before the time-consuming compression.
					BOOL bAttach = TRUE;
					BOOL bAttachChecksum = FALSE;
					long nChecksum = 0;
					if(g_cbPrecompress && bHasDIB) g_cbPrecompress(pbmi, bAttach, bAttachChecksum, nChecksum, lpCon);
					if(bAttach) {	
						if (g_eTargetFormat == eScanToPDF) {
							// (a.walling 2008-09-05 13:19) - PLID 22821 - Create target PDF
							try {
								NxPdf::IPdfDocumentPtr pPdfDocument;
								HRESULT hr = pPdfDocument.CreateInstance("NxPdf.PdfDocument");
								if (pPdfDocument) {									
									// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
									pPdfDocument->AutoLandscape = g_bPDFAutoLandscape ? VARIANT_TRUE : VARIANT_FALSE;
									pPdfDocument->UseThumbs = g_bPDFUseThumbs ? VARIANT_TRUE : VARIANT_FALSE;
									pPdfDocument->PageSize = g_nPDFPageSize;
									// (a.walling 2009-12-11 13:45) - PLID 36518 - Support PNG and JPEG for our internal PDF creations
									if (strExtension == "png") {
										pPdfDocument->AddNewPageFromPNG(_bstr_t(strTempFilename));
									} else {
										pPdfDocument->AddNewPageFromJPEG(_bstr_t(strTempFilename));
									}
									pPdfDocument->SaveToFile(_bstr_t(strUserDefinedFilename));
									Log("File %s saved successfully.", strUserDefinedFilename);
									if (g_cb && bHasDIB) g_cb(eScanToRemoteFolder, strUserDefinedFilename, bAttach, g_pUserData, cxImage);
									if (bAttach) Attach(lpCon, strUserDefinedFilename, bAttachChecksum, nChecksum); /* Now attach the saved dib to the patient */	
								} else {
									_com_issue_error(hr);
								}
							} NxCatchAllThread("Error generating PDF file in userdefined path!");
						} else if (g_eTargetFormat == eScanToImage || g_eTargetFormat == eScanToNative) {
							if (CopyFile(strTempFilename, strUserDefinedFilename, TRUE)) {
								Log("File %s saved successfully.", strUserDefinedFilename);
								// (a.walling 2008-07-24 13:28) - PLID 30836 - Also pass the image in memory
								if (g_cb && bHasDIB) g_cb(eScanToRemoteFolder, strUserDefinedFilename, bAttach, g_pUserData, cxImage);
								if (bAttach) Attach(lpCon, strUserDefinedFilename, bAttachChecksum, nChecksum); /* Now attach the saved dib to the patient */	
							} else {
								Log("Failed to save %s. Error code: %li", strUserDefinedFilename, GetLastError());
							}
						} else {
							ASSERT(FALSE);
						}
					}
				}
			}
		}
		return TRUE;
	}

	// (a.walling 2009-12-11 13:49) - PLID 36518 - Moved to shared function so it can be called after both native and file xfers
	void CreateMultiPDF(ADODB::_Connection* lpCon, CStringArray& saTempPdfFiles)
	{
		if (g_eTargetFormat == eScanToMultiPDF) {
			try {

				// (b.spivey, July 18, 2013) - PLID 36603 - No pages, don't bother creating bad pdfs!
				if (saTempPdfFiles.IsEmpty()) {
					return;
				}

				// (a.walling 2008-09-05 14:21) - PLID 22821 - PDF object for multi-page files
				NxPdf::IPdfDocumentPtr pPdfDocument;

				HRESULT hr = pPdfDocument.CreateInstance("NxPdf.PdfDocument");

				if (pPdfDocument) {			
					// (a.walling 2008-09-08 12:58) - PLID 31293 - Apply PDF preferences
					// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
					pPdfDocument->AutoLandscape = g_bPDFAutoLandscape ? VARIANT_TRUE : VARIANT_FALSE;
					pPdfDocument->UseThumbs = g_bPDFUseThumbs ? VARIANT_TRUE : VARIANT_FALSE;
					pPdfDocument->PageSize = g_nPDFPageSize;

					for (int x = 0; x < saTempPdfFiles.GetSize(); x++) {
						CString strFilePage = saTempPdfFiles[x];
						CString strExtension = FileUtils::GetFileExtension(strFilePage);
						strExtension.MakeLower();

						if (strExtension == "png") {
							pPdfDocument->AddNewPageFromPNG(_bstr_t(strFilePage));
						} else {
							pPdfDocument->AddNewPageFromJPEG(_bstr_t(strFilePage));
						}
					}
					CString strStdFilename;
					CString strUserDefinedFilename;

					BOOL bStandardSaved = FALSE, bUserSaved = FALSE;
					// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
					if (g_bScanToDocumentFolder) {
						strStdFilename = GetStandardOutputFilename(lpCon);

						pPdfDocument->SaveToFile(_bstr_t(strStdFilename));
						bStandardSaved = TRUE;
					}
					// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
					if (g_bScanToRemoteFolder) {
						strUserDefinedFilename = GetUserDefinedOutputFilename(lpCon);
						BOOL bCopySuccess = FALSE;
						// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
						if (g_bScanToDocumentFolder) {
							
							if (CopyFile(strStdFilename, strUserDefinedFilename, FALSE)) {
								// we are good!
								bCopySuccess = TRUE;
							}
						}
						if (!bCopySuccess) {
							pPdfDocument->SaveToFile(_bstr_t(strUserDefinedFilename));
							bUserSaved = TRUE;
							// don't attach if the documents one was attached. This would not attach anyway
							// if the previous one was attached simply because it has the same checksum.
						}
					}

					if (bStandardSaved) {
						// (b.spivey, July 18, 2013) - PLID 45194 - call the callback if we're gonna save. 
						BOOL bAttach = TRUE;
						CxImage cx; 
						if (g_cb && !saTempPdfFiles.IsEmpty()) g_cb(eScanToRemoteFolder, strStdFilename, bAttach, g_pUserData, cx);
						Attach(lpCon, strStdFilename, FALSE, 0);
					} else if (bUserSaved) {
						// (b.spivey, July 18, 2013) - PLID 45194 - call the callback if we're gonna save. 
						BOOL bAttach = TRUE;
						CxImage cx; 
						if (g_cb && !saTempPdfFiles.IsEmpty()) g_cb(eScanToRemoteFolder, strUserDefinedFilename, bAttach, g_pUserData, cx);
						Attach(lpCon, strUserDefinedFilename, FALSE, 0);
					} else {
						ThrowNxException("No documents to attach");
					}
				} else {
					_com_issue_error(hr);
				}
			} NxCatchAllThread("Failed to generate PDF document!");
		}
	}

	// (a.walling 2010-04-08 09:10) - PLID 38171
	void ProcessNativeImage(LPBITMAPINFOHEADER lpdib, OUT CxImage& cxImage)
	{
		/*
		// Make sure important values are assigned
		if (lpdib->biSizeImage == 0) {
			lpdib->biSizeImage = WIDTHBYTES ((DWORD)lpdib->biWidth * lpdib->biBitCount)	* lpdib->biHeight;
		}
		if (lpdib->biClrUsed == 0) {
			lpdib->biClrUsed = DibNumColors(lpdib);
		}

		// Log the original dimensions for debugging purposes
		Log("Dimensions: %d x %d x %d. Size: %d.", lpdib->biWidth, lpdib->biHeight,
			lpdib->biClrUsed, lpdib->biSizeImage);

		// Calculate the color table size, then point past the BITMAPINFOHEADER
		// and color table, to the byte array of bitmap bits.
		dwColorTableSize = (DWORD)(DibNumColors (lpdib) * sizeof(RGBQUAD));
		lpBits = (LPSTR)lpdib + lpdib->biSize + dwColorTableSize;

		*/

		LPBITMAPINFOHEADER pDIB = lpdib;

		DWORD dwPaletteSize = 0;

		switch(pDIB->biBitCount)
		{
			case 1:
				dwPaletteSize = 2;
				break;
			case 4:
				dwPaletteSize = 16;
				break;
			case 8:
				dwPaletteSize = 256;
				break;
			case 24:
				break;
			default:
				Log("Bitcount invalid %li", DWORD(pDIB->biBitCount));
				break;
		}

		// If the driver did not fill in the biSizeImage field, then compute it
		// Each scan line of the image is aligned on a DWORD (32bit) boundary
		if( pDIB->biSizeImage == 0 )
		{
			pDIB->biSizeImage = ((((pDIB->biWidth * pDIB->biBitCount) + 31) & ~31) / 8) * pDIB->biHeight;

			// If a compression scheme is used the result may infact be larger
			// Increase the size to account for this.
			if (pDIB->biCompression != 0)
			{
				Log("biCompression is set: %li", DWORD(pDIB->biCompression));
				pDIB->biSizeImage = (pDIB->biSizeImage * 3) / 2;
			}
		}

		Log("DIB info (%lu): biSize(%lu) (%lu); biWidth(%li); biHeight(%li); biPlanes(%li); biBitCount(%li); biCompression(%lu); biSizeImage(%lu); biXPel(%li); biYPel(%li); biClrUsed(%lu); biClrImportant(%lu)",
			pDIB->biSizeImage + (sizeof(RGBQUAD)*dwPaletteSize) + sizeof(BITMAPINFOHEADER),
			pDIB->biSize,
			DWORD(sizeof(BITMAPINFOHEADER)),
			pDIB->biWidth,
			pDIB->biHeight,
			LONG(pDIB->biPlanes),
			LONG(pDIB->biBitCount),
			pDIB->biCompression,
			pDIB->biSizeImage,
			pDIB->biXPelsPerMeter,
			pDIB->biYPelsPerMeter,
			pDIB->biClrUsed,
			pDIB->biClrImportant);

		BYTE* pBits = (BYTE*)pDIB + (sizeof(RGBQUAD)*dwPaletteSize) + sizeof(BITMAPINFOHEADER);

		// (a.walling 2010-04-08 09:10) - PLID 38171
		BYTE* pPalette = (BYTE*)pDIB + sizeof(BITMAPINFOHEADER);

		// Now load the DIB into a CxImage object
		/*
		imgCX.CreateFromArray((BYTE*)lpBits,
			lpdib->biWidth,
			lpdib->biHeight,
			lpdib->biBitCount,
			(lpdib->biSizeImage / lpdib->biHeight),
			false);

		// Now force the image to be 24 BPP or better
		if (imgCX.GetBpp() < 24) {
			Log("Image pixel depth is %d bits. Resampling to 24.", imgCX.GetBpp());
			imgCX.IncreaseBpp(24);
		}*/

		// (a.walling 2008-09-03 14:48) - PLID 22821 - Just use microsoft's built in function
		// and avoid the wierd CxImage workarounds above
		// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
		//pBitmap = Gdiplus::Bitmap::FromBITMAPINFO((BITMAPINFO*)lpdib, pBits);

		// (a.walling 2010-04-08 09:10) - PLID 38171 - We are going back to CxImage, mostly because GDI+ has trouble with certain
		// DIBs that get passed in. Specifically some DIBs do not have a proper color palette, and GDI+ will end up inverting the
		// colors. We let CxImage take care of all this for us now, which works out fine, and we set the palette ourselves if we
		// are not in b&w mode, which was the critical piece missing in our previous CxImage approach.
		cxImage.CreateFromArray(pBits, pDIB->biWidth, pDIB->biHeight, pDIB->biBitCount, pDIB->biSizeImage / pDIB->biHeight, false);
		
		// (j.dinatale 2012-11-13 11:35) - PLID 53726 - need to calculate the DPI if we have it available to us
		if(pDIB->biXPelsPerMeter != 0 && pDIB->biYPelsPerMeter != 0){
			// Best we can do here is round. There are 39.3701 inches to every meter (roughly), 
			// so lets convert our per meter metric to per inch.
			cxImage.SetXDPI((long)floor((float)pDIB->biXPelsPerMeter / 39.3701f + .5f));
			cxImage.SetYDPI((long)floor((float)pDIB->biYPelsPerMeter / 39.3701f + .5f));
		}

		// (a.walling 2010-04-08 09:10) - PLID 38171 - In general, we will only set a palette if we are not monochrome (or the preferences are set)
		if (dwPaletteSize != 0 && ( (dwPaletteSize == 2 && g_bApplyMonochromePalette) || (dwPaletteSize > 2 && g_bApplyGreyscalePalette) ) ) {
			cxImage.SetPalette((RGBQUAD*)pPalette, dwPaletteSize);
		}
	}

	void DoNativeTransfer()
	{
		//
		// (c.haag 2006-07-18 17:19) - PLID 21505 - This is the new way of acquiring images
		// through the TWAIN code.
		//
		// Old way
		// =======
		// Do a memory transfer, which involves retrieving multiple buffers from a device,
		// putting them into a DIB, then resampling the DIB to 24 BPP, then writing them to
		// a compression stream and saving the stream to a file.
		//
		// New way
		// =======
		// Do a native transfer, which involves retrieving a handle to a DIB from a device,
		// then copying it to a CxImage object, and saving it to a file using CxImage's internal
		// functionality.
		//
		// We have much less overhead to deal with in the new way; in addition, more of the process
		// is black boxed, and it's proven to work in cases where the old way failed (i.e. Dr. Pillersdorf).
		// I'm also of the opinion that the code is easier to maintain, and there is certainly much
		// less of it to maintain.
		//
		
		// (a.walling 2010-01-28 13:43) - PLID 28806 - Threads need to have their own connection, let's create one, based on the global connection.
		// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
		_ConnectionPtr pCon = GetThreadRemoteData();

		CNxTwainProgressDlg dlgProgress;
		CString strProgressText;
		TW_PENDINGXFERS     twPendingXfer;
		TW_UINT16           twRC = TWRC_FAILURE;
		TW_UINT16           twRC2 = TWRC_FAILURE;
		TW_UINT32           hBitMap = NULL;
		HANDLE              hbm_acq = NULL;     // handle to bit map from Source to ret to App
		LPSTR               lpBits = NULL;      // handle to bit map bits
		DWORD               dwColorTableSize = 0;
		LPBITMAPINFOHEADER  lpdib = NULL;
		//CxImage             imgCX; // (a.walling 2008-09-03 13:31) - PLID 22821 - Deprecated, use Gdiplus
		// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
		//Gdiplus::Bitmap*	pBitmap = NULL;
		long nCurTransfer = 0;

		// (a.walling 2008-09-05 14:21) - PLID 22821 - Array of temporary files for PDF
		CStringArray saTempPdfFiles;

		memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));

		/*
		* Do until there are no more pending transfers
		* explicitly initialize the our flags
		*/
		do 
		{
			strProgressText.Format("Acquiring image #%d for attachment", nCurTransfer + 1);
			dlgProgress.SetProgressText(strProgressText);
			Log(strProgressText);

			/*
			* Initiate Native Transfer
			*/
			twRC = CallDSMEntry(&appID,
							&dsID, 
							DG_IMAGE,
							DAT_IMAGENATIVEXFER, 
							MSG_GET, 
							(TW_MEMREF)&hBitMap);

			switch (twRC)
			{
				case TWRC_XFERDONE:  // Session is in State 7
					if (hBitMap && (lpdib = (LPBITMAPINFOHEADER)GlobalLock((void*)hBitMap))!=NULL)
					{

						// Cleanup
						// (a.walling 2009-02-23 12:33) - PLID 33197 - We have never cleaned up the DIB that was passed into us.
						
						CxImage cxImage;
						// (a.walling 2010-04-08 09:10) - PLID 38171
						ProcessNativeImage(lpdib, cxImage);
						// Now save the image to disk
						SaveNativeImage(pCon, cxImage, lpdib, dlgProgress, nCurTransfer, saTempPdfFiles, "");
						
						// (a.walling 2010-04-08 08:22) - PLID 38170
						if (hBitMap != NULL) {
							if (0 == GlobalUnlock((void*)hBitMap)) {
								GlobalFree((void*)hBitMap);
							}
						}

						// Cleanup
						// (a.walling 2010-04-08 08:23) - PLID 38170
						//delete pBitmap;
						//pBitmap = NULL;

						/*ReleaseDC(hDC);

						if (hDibPal) {
							DeleteObject(hDibPal);
							hDibPal = NULL;
						}*/
					}

					/*
					* Acknowledge the end of the transfer 
					* and transition to state 6/5
					*/
					twRC2 = CallDSMEntry(&appID,
										&dsID, 
										DG_CONTROL,
										DAT_PENDINGXFERS, 
										MSG_ENDXFER,
										(TW_MEMREF)&twPendingXfer);
					nCurTransfer++;
					break;

				/*
				* the user canceled or wants to rescan the image
				* something wrong, abort the transfer and delete the image
				* pass a null ptr back to App
				*/
				case TWRC_CANCEL:   // Session is in State 7
					/*
					* Source (or User) Canceled Transfer
					* transistion to state 6/5
					*/
					twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer);
					break;

				case TWRC_FAILURE:  //Session is in State 6
					/*
					* Abort the image
					* Enhancement: Check Condition Code and attempt recovery
					*/
					twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer); 
					break;

				default:    //Sources should never return any other RC
					/*
					* Abort the image
					* Enhancement: Check Condition Code and attempt recovery instead
					*/
					twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer);
					break;
			}   

		} while (twPendingXfer.Count != 0);

		// Close the DS and DSM
		//
		if (DisableDS()) {
			if (CloseDS()) {
				CloseDSM();
			}
		}

		CreateMultiPDF(pCon, saTempPdfFiles);

		// (a.walling 2008-09-05 14:20) - PLID 22821 - Clear out any temp files we may have created
		for (int x = 0; x < saTempPdfFiles.GetSize(); x++) {
			if (!DeleteFile(saTempPdfFiles[x])) {
				ASSERT(FALSE);
				// (a.walling 2010-01-28 13:34) - PLID 28806 - Don't call into the mainframe from a different thread
				MoveFileAtStartup(saTempPdfFiles[x], NULL);
			}
		}

		GetMainFrame()->PostMessage(NXM_TWAIN_XFERDONE);
	}

	BOOL ProcessTWMessage(LPMSG lpMsg)
	{
		TW_UINT16  twRC = TWRC_NOTDSEVENT;
		TW_EVENT   twEvent;

		if (!TWDSOpen || !TWDSMOpen)
			return FALSE;

		HWND hWnd = GetTwainWindow();
		memset(&twEvent, 0, sizeof(TW_EVENT));

		// A Source provides a modeless dialog box as its user interface.
		// The following call relays Windows messages down to the Source's
		// UI that were intended for its dialog box.  It also retrieves TWAIN
		// messages sent from the Source to our  Application.
		//

		twEvent.pEvent = (TW_MEMREF)lpMsg;

		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_CONTROL, 
						DAT_EVENT,
						MSG_PROCESSEVENT, 
						(TW_MEMREF)&twEvent);

		switch (twEvent.TWMessage)
		{
			case MSG_XFERREADY:
				{
					// (a.walling 2009-12-10 08:30) - PLID 36518 - Determine what our current transfer mechanism is
					TW_UINT32 mech = GetXferMech();
					if (mech == TWSX_NATIVE) {
						DoNativeTransfer();
					}
				}
				break;
			case MSG_CLOSEDSREQ:
			case MSG_CLOSEDSOK:
				// (a.walling 2008-07-25 12:06) - PLID 30836 - Close!
				if (DisableDS()) {
					if (CloseDS()) {
						CloseDSM();
					}
				}
				GetMainFrame()->PostMessage(NXM_TWAIN_XFERDONE);
				break;
			case MSG_NULL:
				break;
			default:
#ifdef _DEBUG
				TRACE("Unhandled TWAIN message 0x%08x\n", twEvent.TWMessage);
#endif
				break;
		}
		// (a.walling 2009-03-31 13:59) - PLID 33763 - Return TRUE if this is a data source event, otherwise false
		return (twRC==TWRC_DSEVENT);
	}

	BOOL SelectDS()
	{
		//////////////////////////////////////////////////////////////////////////
		// FUNCTION: TWSelectDS
		//
		// ARGS:    none
		//
		// RETURNS: twRC TWAIN status return code
		//
		// NOTES:   1). call the Source Manager to:
		//              - have the SM put up a list of the available Sources
		//              - get information about the user selected Source from
		//                NewDSIdentity, filled by Source
		//
		TW_UINT16 twRC = TWRC_FAILURE;
		TW_IDENTITY NewDSIdentity;

		memset(&NewDSIdentity, 0, sizeof(TW_IDENTITY));

		if (TWDSOpen)
		{
			twRC = TWRC_FAILURE;
		}
		else
		{
			// I will settle for the system default.  Shouldn't I get a highlight
			// on system default without this call?
			twRC = CallDSMEntry(&appID,
						NULL,
						DG_CONTROL,
						DAT_IDENTITY,
						MSG_GETDEFAULT,
						(TW_MEMREF)&NewDSIdentity);

			// This call performs one important function:
			// - should cause SM to put up dialog box of available Source's
			// - tells the SM which application, appID.id, is requesting, REQUIRED
			// - returns the SM assigned NewDSIdentity.id field, you check if changed
			//  (needed to talk to a particular Data Source)
			// - be sure to test return code, failure indicates SM did not close !!
			//
			if (TWRC_SUCCESS == (twRC = CallDSMEntry(&appID,
							NULL,
							DG_CONTROL,
							DAT_IDENTITY,
							MSG_USERSELECT,
							(TW_MEMREF)&NewDSIdentity)))
			{
				dsID = NewDSIdentity;
			}
		}
		return (twRC);
	}  // TWSelectDS

	BOOL SelectDS(const CString& strDeviceName)
	{
		CString strDevice = strDeviceName;
		TW_UINT16 twRC = TWRC_FAILURE;
		TW_IDENTITY NewDSIdentity;

		memset(&NewDSIdentity, 0, sizeof(TW_IDENTITY));
		strDevice.MakeUpper();

		if (TWDSOpen)
		{
/*			//A Source is already open
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 0, 0, 0,
							"A Source is already open\nClose Source before Selecting a New Source",
							"DG_CONTROL/DAT_IDENTITY/MSG_USERSELECT");
			}*/
			twRC = TWRC_FAILURE;
		}
		else
		{
			twRC = CallDSMEntry(&appID,
					NULL,
					DG_CONTROL,
					DAT_IDENTITY,
					MSG_GETFIRST,
					(TW_MEMREF)&NewDSIdentity);

			if (twRC != TWRC_FAILURE)
			{
				CString strDSID = NewDSIdentity.ProductName;
				strDSID.MakeUpper();
				if (-1 != strDSID.Find(strDevice) ||
					-1 != strDevice.Find(strDSID))
				{
					dsID = NewDSIdentity;
					return TWRC_SUCCESS;
				}
				do
				{
					twRC = CallDSMEntry(&appID,
							NULL,
							DG_CONTROL,
							DAT_IDENTITY,
							MSG_GETNEXT,
							(TW_MEMREF)&NewDSIdentity);

					if (twRC != TWRC_FAILURE)
					{
						CString strDSID = NewDSIdentity.ProductName;
						strDSID.MakeUpper();
						if (-1 != strDSID.Find(strDevice) ||
							-1 != strDevice.Find(strDSID))
						{
							dsID = NewDSIdentity;
							return TWRC_SUCCESS;
						}
					}

				} while (twRC != TWRC_FAILURE);
			}

		}
		return (twRC);
	}

	// (a.walling 2010-01-27 14:50) - PLID 36568 - For simplicity, set globals from CNxTwain object here
	void SetGlobals(CNxTwain* pTwain)
	{
		g_nPersonID = pTwain->m_nPersonID;
		g_strDocumentPath = pTwain->m_strDocumentPath;
		g_cb = pTwain->m_cb;
		g_cbPrecompress = pTwain->m_cbPrecompress;
		g_pUserData = pTwain->m_pUserData;
		g_nCategoryID = pTwain->m_nCategoryID;
		// (a.walling 2008-09-03 11:43) - PLID 19638 - Silly global variable for PicID
		g_nPicID = pTwain->m_nPicID;
		// (a.walling 2008-09-05 13:21) - PLID 22821 - Set scan type
		g_eTargetFormat = pTwain->m_eTargetFormat;

		// (m.hancock 2006-06-27 15:53) - PLID 21071 - Added field for associating MailSent records with lab step records
		g_nLabStepID = pTwain->m_nLabStepID;


		// (a.walling 2010-01-28 09:11) - PLID 28806 - Set the global cached properties now
		g_bShowUI = pTwain->m_bShowUI;
		g_bAutoFeed = pTwain->m_bAutoFeed;

		g_strRemoteFolder = pTwain->m_strRemoteFolder;
		g_bSave8BitAsJPEG = pTwain->m_bSave8BitAsJPEG;
		// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the monochrome palette, default to false.
		g_bApplyMonochromePalette = pTwain->m_bApplyMonochromePalette;
		// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the greyscale palette, default to true
		g_bApplyGreyscalePalette = pTwain->m_bApplyGreyscalePalette;

		g_bScanToRemoteFolder = pTwain->m_bScanToRemoteFolder;
		g_bScanToDocumentFolder = pTwain->m_bScanToDocumentFolder;

		g_bPDFAutoLandscape = pTwain->m_bPDFAutoLandscape;
		g_bPDFUseThumbs = pTwain->m_bPDFUseThumbs;
		g_nPDFPageSize = pTwain->m_nPDFPageSize;		

		// (a.walling 2010-01-28 14:10) - PLID 28806
		g_bIsPhotoJPEG = pTwain->m_bIsPhotoJPEG;
		g_bIsPhotoPNG = pTwain->m_bIsPhotoPNG;
	}

	// (a.walling 2010-04-08 13:14) - PLID 38172 - Negotiate and restrict capabilities
	void NegotiateCaps()
	{	
		try {
			TW_CAPABILITY  Cap;

			Cap.Cap = ICAP_PIXELTYPE;
			Cap.hContainer = 0;

			if(TWCC_SUCCESS == GetCAP(Cap, MSG_GET))
			{
				// yarrr
				if (Cap.ConType == TWON_ENUMERATION) {
					pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(Cap.hContainer);

					CDWordArray dwaSupported;
					TW_UINT32 nDefaultIndex = 0xFFFFFFFF;
					TW_UINT32 nCurrentIndex = 0xFFFFFFFF;

					for (TW_UINT32 x = 0; x < pCapPT->NumItems; x++) {
						TW_UINT16 value = ((pTW_UINT16)(&pCapPT->ItemList))[x];

						if (x == pCapPT->DefaultIndex) {
							TRACE("ICAP_PIXELTYPE default: %lu\n", value);
						} else {
							TRACE("ICAP_PIXELTYPE available: %lu\n", value);
						}

						switch (value) {
							case TWPT_BW:
							case TWPT_GRAY:
							case TWPT_RGB:
								if (x == pCapPT->DefaultIndex) {
									nDefaultIndex = dwaSupported.GetCount();
								}
								if (x == pCapPT->CurrentIndex) {
									nCurrentIndex = dwaSupported.GetCount();
								}
								dwaSupported.Add(value);
								break;
						}
					}
					
				    _DSM_UnlockMemory(Cap.hContainer);
				    _DSM_Free(Cap.hContainer);

					if (dwaSupported.GetCount() > 0) {
						// now limit the available values

						if (nDefaultIndex == 0xFFFFFFFF) {
							// default is not available in our usual list of accepted types
							nDefaultIndex = nCurrentIndex;
						}
						if (nCurrentIndex == 0xFFFFFFFF) {
							// hm, no current value available either? we can try to set to default.
							nCurrentIndex = nDefaultIndex;
						}
						// at this point, both values should be filled, or neither. in that case, set them both to the last value.
						if (nCurrentIndex == 0xFFFFFFFF && nDefaultIndex == 0xFFFFFFFF) {
							nCurrentIndex = dwaSupported.GetCount() - 1;
							nDefaultIndex = dwaSupported.GetCount() - 1;
						}
						
						TW_UINT32 twEnumerationHeaderSize = sizeof(TW_ENUMERATION) - sizeof(TW_UINT8);

						TW_CAPABILITY LimitCap;
						LimitCap.Cap = ICAP_PIXELTYPE;
						LimitCap.ConType = TWON_ENUMERATION;
						LimitCap.hContainer = _DSM_Alloc(twEnumerationHeaderSize + (sizeof(TW_UINT16) * dwaSupported.GetCount()));
						
						pTW_ENUMERATION pLimitCapPT = (pTW_ENUMERATION)_DSM_LockMemory(LimitCap.hContainer);

						pLimitCapPT->ItemType = TWTY_UINT16;
						pLimitCapPT->NumItems = dwaSupported.GetCount();
						pLimitCapPT->CurrentIndex = nCurrentIndex;
						pLimitCapPT->DefaultIndex = nDefaultIndex;

						int availPixelTypeIndex = 0;
						for (availPixelTypeIndex = 0; availPixelTypeIndex < dwaSupported.GetCount(); availPixelTypeIndex++) {
							((pTW_UINT16)(&pLimitCapPT->ItemList))[availPixelTypeIndex] = (TW_UINT16)dwaSupported.GetAt(availPixelTypeIndex);
						}

						
						TW_INT16 twrcAvail = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&(LimitCap));

						TRACE("Setting avail values for ICAP_PIXELTYPE returns %li\n", (long)twrcAvail);
						
					    _DSM_UnlockMemory(LimitCap.hContainer);
					    _DSM_Free(LimitCap.hContainer);

						// now we need to set the supported bit depths for each supported type
						for (availPixelTypeIndex = 0; availPixelTypeIndex < dwaSupported.GetCount(); availPixelTypeIndex++) {
							DWORD dwCurrentPixelType = dwaSupported.GetAt(availPixelTypeIndex);
							if (TWRC_SUCCESS == SetCAPOneValue(ICAP_PIXELTYPE, (int)dwCurrentPixelType, TWTY_UINT16)) {
								// OK, we've set our mode to this pixel type. Now we need to enum the bit depths and choose which to use.

								if (dwCurrentPixelType != TWPT_BW) {
									TW_CAPABILITY BitDepthCap;
									BitDepthCap.Cap = ICAP_BITDEPTH;
									BitDepthCap.hContainer = 0;
										
									long nCurrentBitDepth = -1;
									long nBestSupportedBitDepth = -1;

									if(TWCC_SUCCESS == GetCAP(BitDepthCap, MSG_GET))
									{
										if (BitDepthCap.ConType == TWON_ENUMERATION) {
											pTW_ENUMERATION pBitDepthCapPT = (pTW_ENUMERATION)_DSM_LockMemory(BitDepthCap.hContainer);

											TW_UINT32 nNumBitDepths = pBitDepthCapPT->NumItems;

											CDWordArray dwaSupportedBitDepths;
											TW_UINT32 nDefaultBitDepthIndex = 0xFFFFFFFF;
											TW_UINT32 nCurrentBitDepthIndex = 0xFFFFFFFF;


											for (TW_UINT32 x = 0; x < pBitDepthCapPT->NumItems; x++) {
												TW_UINT16 value = ((pTW_UINT16)(&pBitDepthCapPT->ItemList))[x];

												if (x == pBitDepthCapPT->DefaultIndex) {
													TRACE("ICAP_BITDEPTH default: %lu\n", value);
												} else {
													TRACE("ICAP_BITDEPTH available: %lu\n", value);
												}

												if (x == pBitDepthCapPT->CurrentIndex) {
													nCurrentBitDepth = (long)value;
												}

												if (dwCurrentPixelType == TWPT_GRAY) {
													switch (value) {
														case 4:
														case 8:
															if (nBestSupportedBitDepth < (long)value) {
																nBestSupportedBitDepth = (long)value;
															}
															if (x == pBitDepthCapPT->DefaultIndex) {
																nDefaultBitDepthIndex = dwaSupportedBitDepths.GetCount();
															}
															if (x == pBitDepthCapPT->CurrentIndex) {
																nCurrentBitDepthIndex = dwaSupportedBitDepths.GetCount();
															}
															dwaSupportedBitDepths.Add(value);
															break;
													}
												} else if (dwCurrentPixelType == TWPT_RGB) {
													switch (value) {
														case 24:
															nBestSupportedBitDepth = (long)value;
															// fallthrough
														case 32:
															if (nBestSupportedBitDepth == -1) {
																// we prefer 24, so don't override that value
																nBestSupportedBitDepth = (long)value;
															}
															if (x == pBitDepthCapPT->DefaultIndex) {
																nDefaultBitDepthIndex = dwaSupportedBitDepths.GetCount();
															}
															if (x == pBitDepthCapPT->CurrentIndex) {
																nCurrentBitDepthIndex = dwaSupportedBitDepths.GetCount();
															}
															dwaSupportedBitDepths.Add(value);
															break;
													}
												}
											}
											
											_DSM_UnlockMemory(BitDepthCap.hContainer);
											_DSM_Free(BitDepthCap.hContainer);

											if (dwaSupportedBitDepths.GetCount() > 0 && dwaSupportedBitDepths.GetCount() != (int)nNumBitDepths) {

												// now limit the available values

												if (nDefaultBitDepthIndex == 0xFFFFFFFF) {
													// default is not available in our usual list of accepted types
													nDefaultBitDepthIndex = nCurrentBitDepthIndex;
												}
												if (nCurrentBitDepthIndex == 0xFFFFFFFF) {
													// hm, no current value available either? we can try to set to default.
													nCurrentBitDepthIndex = nDefaultBitDepthIndex;
												}
												// at this point, both values should be filled, or neither. in that case, set them both to the last value.
												if (nCurrentBitDepthIndex == 0xFFFFFFFF && nDefaultBitDepthIndex == 0xFFFFFFFF) {
													nCurrentBitDepthIndex = dwaSupported.GetCount() - 1;
													nDefaultBitDepthIndex = dwaSupported.GetCount() - 1;
												}
												
												TW_UINT32 twEnumerationHeaderSize = sizeof(TW_ENUMERATION) - sizeof(TW_UINT8);

												TW_CAPABILITY BitDepthLimitCap;
												BitDepthLimitCap.Cap = ICAP_BITDEPTH;
												BitDepthLimitCap.ConType = TWON_ENUMERATION;
												BitDepthLimitCap.hContainer = _DSM_Alloc(twEnumerationHeaderSize + (sizeof(TW_UINT16) * dwaSupportedBitDepths.GetCount()));
												
												pTW_ENUMERATION pBitDepthLimitCapPT = (pTW_ENUMERATION)_DSM_LockMemory(BitDepthLimitCap.hContainer);

												pBitDepthLimitCapPT->ItemType = TWTY_UINT16;
												pBitDepthLimitCapPT->NumItems = dwaSupportedBitDepths.GetCount();
												pBitDepthLimitCapPT->CurrentIndex = nCurrentBitDepthIndex;
												pBitDepthLimitCapPT->DefaultIndex = nDefaultBitDepthIndex;

												int availBitDepthIndex = 0;
												for (availBitDepthIndex = 0; availBitDepthIndex < dwaSupportedBitDepths.GetCount(); availBitDepthIndex++) {
													((pTW_UINT16)(&pBitDepthLimitCapPT->ItemList))[availBitDepthIndex] = (TW_UINT16)dwaSupportedBitDepths.GetAt(availBitDepthIndex);
												}
												
												TW_INT16 twrcAvailBitDepths = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&(BitDepthLimitCap));

												TRACE("Setting avail values for ICAP_BITDEPTH returns %li\n", (long)twrcAvailBitDepths);
												
												_DSM_UnlockMemory(BitDepthLimitCap.hContainer);
												_DSM_Free(BitDepthLimitCap.hContainer);
											}
										}
									}									

									// (a.walling 2010-04-13 12:32) - PLID 38172 - Final effort is to ensure the current settings for this pixel type
									// is using a pixel depth that we support. Some TWAIN drivers may not necessarily implement restricting the available
									// values using MSG_SET as above.
									if (dwCurrentPixelType == TWPT_GRAY) {
										if (nCurrentBitDepth != 4 && nCurrentBitDepth != 8) {
											TW_INT16 twrcFinalPixelDepth = TWRC_SUCCESS;

											if (nBestSupportedBitDepth != -1) {
												twrcFinalPixelDepth = SetCAPOneValue(ICAP_BITDEPTH, (int)nBestSupportedBitDepth, TWTY_UINT16);
											} else {
												// who knows, the driver may have failed to give us a proper enumeration.
												twrcFinalPixelDepth = SetCAPOneValue(ICAP_BITDEPTH, (int)8, TWTY_UINT16);
											}
											
											TRACE("Final set pixel depth returns %li\n", (long)twrcFinalPixelDepth);
										}
									} else if (dwCurrentPixelType == TWPT_RGB) {
										if (nCurrentBitDepth != 24 && nCurrentBitDepth != 32) {
											TW_INT16 twrcFinalPixelDepth = TWRC_SUCCESS;

											if (nBestSupportedBitDepth != -1) {
												twrcFinalPixelDepth = SetCAPOneValue(ICAP_BITDEPTH, (int)nBestSupportedBitDepth, TWTY_UINT16);
											} else {
												// who knows, the driver may have failed to give us a proper enumeration.
												twrcFinalPixelDepth = SetCAPOneValue(ICAP_BITDEPTH, (int)24, TWTY_UINT16);
											}
											
											TRACE("Final set pixel depth returns %li\n", (long)twrcFinalPixelDepth);
										}
									}
								}

								// if black and white or grayscale, ensure we are chocolate
								if (dwCurrentPixelType == TWPT_BW || dwCurrentPixelType == TWPT_GRAY) {
									TW_INT16 twrcPixelFlavor = SetCAPOneValue(ICAP_PIXELFLAVOR, TWPF_CHOCOLATE, TWTY_UINT16);
									TRACE("Setting pixel flavor returns %li\n", (long)twrcPixelFlavor);
								}
							}
						}
						// finally reset back to default
						{									
							SetCAPOneValue(ICAP_PIXELTYPE, (int)dwaSupported.GetAt(nCurrentIndex), TWTY_UINT16);
						}
					}
				}

			}
		} NxCatchAllThread("Error negotiating scan parameters");

		try {
			// Restrict to no compression
			
			TW_UINT32 twEnumerationHeaderSize = sizeof(TW_ENUMERATION) - sizeof(TW_UINT8);

			TW_CAPABILITY CompressionCap;
			CompressionCap.Cap = ICAP_COMPRESSION;
			CompressionCap.ConType = TWON_ENUMERATION;
			CompressionCap.hContainer = _DSM_Alloc(twEnumerationHeaderSize + sizeof(TW_UINT16));
			
			pTW_ENUMERATION pCompressionCapPT = (pTW_ENUMERATION)_DSM_LockMemory(CompressionCap.hContainer);

			pCompressionCapPT->ItemType = TWTY_UINT16;
			pCompressionCapPT->NumItems = 1;
			pCompressionCapPT->CurrentIndex = 0;
			pCompressionCapPT->DefaultIndex = 0;
			((pTW_UINT16)(&pCompressionCapPT->ItemList))[0] = (TW_UINT16)TWCP_NONE;

			TW_INT16 twrcAvailCompression = CallDSMEntry(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&(CompressionCap));

			TRACE("Setting avail values for ICAP_COMPRESSION returns %li\n", (long)twrcAvailCompression);
			
			_DSM_UnlockMemory(CompressionCap.hContainer);
			_DSM_Free(CompressionCap.hContainer);

			// Finally ensure no compression is being used
			SetCAPOneValue(ICAP_COMPRESSION, TWCP_NONE, TWTY_UINT16);

		} NxCatchAllThread("Error negotiating compression parameters");
	}

	BOOL AcquireInThread(CNxTwain* pTwain)
	{
		BOOL result = TRUE;

		if (OpenDSM())
		{
			if (pTwain->m_strDeviceName.GetLength())
			{
				if (TWRC_SUCCESS != SelectDS(pTwain->m_strDeviceName))
				{
					if (TWRC_SUCCESS != SelectDS())
						return TRUE;
				}
			}

			if (OpenDS())
			{
				// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
				BOOL bShow = g_bShowUI ? TRUE : FALSE;
				// (a.walling 2010-01-27 14:50) - PLID 36568
				SetGlobals(pTwain);

				SetImageFormat(imgBMP);

				if (XferMech(pTwain) == TWRC_SUCCESS)
				{
					if (Autofeed() == TWRC_SUCCESS)
					{						
						// (a.walling 2010-04-08 13:14) - PLID 38172 - Negotiate and restrict capabilities
						NegotiateCaps();

						if (!IsDSEnabled())
						{
							result = EnableDS(bShow);
						}
						else
						{
							if (CloseDS())
							{
								CloseDSM();
							}
							return TRUE;
						}
					}
					else
					{
						MsgBox("Your hardware does not support automatic document feeding capabilities from this computer. Please disable ADF from the NexTech Practice Tools=>Scanner/Digital Camera Settings menu, or contact your hardware administrator for support.");
						if (CloseDS())
						{
							CloseDSM();
						}
						return TRUE;
					}
				}
				else
				{
					MsgBox("The Scanner/Camera download mechanism failed. Please contact NexTech for technical support.");
					if (CloseDS())
					{
						CloseDSM();
					}
					return TRUE;
				}
			}
			else
			{
				CloseDSM();
				return TRUE;
			}

			/*
			* Cannot Enable Source
			*/
			if (result == FALSE) 
			{
				if (CloseDS())
					CloseDSM();

				return TRUE;
			}
		}
		return FALSE;
	}

	UINT TWAINThread(LPVOID pData)
	{
		// (a.walling 2008-09-05 13:19) - PLID 22821 - Initialize COM
		CoInitialize(NULL);

		// (z.manning, 05/16/2008) - PLID 30050 - Converted to NxDialog
		/*
		CNxDialog dlg;
		dlg.Create(IDD_NXTWAIN, NULL);
		SetTwainMessageRecipient(dlg.GetSafeHwnd());
		*/
		// (a.walling 2008-09-18 10:11) - PLID 31285 - Use a generic CWnd rather than a dialog, and use our mainframe as the parent
		CWnd wnd;
		if (wnd.CreateEx(WS_EX_NOPARENTNOTIFY, AfxRegisterWndClass(NULL, NULL, NULL, NULL), "NxTWAINWindow", WS_POPUP, CRect(0, 0, 0, 0), CWnd::FromHandle(GetMainFrame()->GetSafeHwnd()), NULL, NULL)) {
			SetTwainMessageRecipient(wnd.GetSafeHwnd());
		} else {
			ASSERT(FALSE);
		}

		HANDLE hevThreadInitialized = (HANDLE)pData;
		if (hevThreadInitialized) {
			SetEvent(hevThreadInitialized);
		}

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			// (a.walling 2009-06-08 12:11) - PLID 34512 - Ensure we don't log off due to inactivity because the twain thread is eating
			// all the events. Unlikely but possible (and easily reproducible if you are trying to do so).
			// (a.walling 2009-08-13 16:43) - PLID 35205 - This should be posted to the mainframe!
			// (a.walling 2009-12-03 10:17) - PLID 35205 - NXM_EXTEND_INACTIVITY_TIMEOUT is again considered an input message and will
			// reset the timer when it is processed by the message filter in the UI thread. So we avoid this loop now simply by not posting
			// it in response to itself, which should no longer happen anyway since we are posting to the mainframe (which has its own queue).
			if (IsInputMsg(msg.message) && msg.message != NXM_EXTEND_INACTIVITY_TIMEOUT && ::IsWindow(GetMainFrame()->GetSafeHwnd())) {
				GetMainFrame()->PostMessage(NXM_EXTEND_INACTIVITY_TIMEOUT, 0, 0);
			}
			if (msg.message == WM_QUIT) {
				break;
			}
			else if (msg.message == WM_USER + 2000)
			{				
				// (a.walling 2008-09-26 17:31) - PLID 31498 - Center the window (WIA wrapper creates a modal dialog based on where the parent window is,
				// while most other source dialogs would center themselves)
				wnd.CenterWindow(CWnd::GetDesktopWindow());

				CNxTwain* pTwain = (CNxTwain*)msg.lParam;
				if (pTwain) {
					// Create and/or reset our acquire event so we can't do more than
					// one acquisition at a time.
					if (!g_hevAcquiring) {
						g_hevAcquiring = CreateEvent(NULL, TRUE, FALSE, NULL);
					} else {
						ResetEvent(g_hevAcquiring);
					}
					AcquireInThread(pTwain);
					delete pTwain;
					// Set our event saying we are no longer acquiring images
					SetEvent(g_hevAcquiring);
				}

				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			} else if (!NXTWAINlib::ProcessTWMessage(&msg)) {
				// (a.walling 2009-03-31 14:01) - PLID 33763 - Only continue dispatching if we get FALSE from ProcessTWMessage, which
				// will be FALSE if this is not a data source event.
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}

		// (a.walling 2008-09-08 08:34) - PLID 31285 - Release thread resources, invalidate global pointer
		g_pTWAINThread = NULL;

		try {
			DisableDS();
		} catch(...) {};
		try {
			CloseDS();
		} catch(...) {};
		try {
			CloseDSM();
		} catch(...) {};

		// (a.walling 2008-09-05 13:19) - PLID 22821 - Uninitialize COM
		CoUninitialize();
		return 0;
	}
	
	CNxTwain::CNxTwain(long nPersonID, const CString& strDocumentPath, 
		cbProc cb, cbPrecompressProc cbPrecompress, void* pUserData,
		const CString& strDeviceName, long nCategoryID, long nLabStepID, long nPicID, EScanTargetFormat eTargetFormat)
	{
		m_nPersonID = nPersonID;
		m_strDocumentPath = strDocumentPath;
		m_cb = cb;
		m_cbPrecompress = cbPrecompress;
		m_pUserData = pUserData;
		m_strDeviceName = strDeviceName;
		m_nCategoryID = nCategoryID;
		
		// (m.hancock 2006-06-27 15:53) - PLID 21071 - Added field for associating MailSent records with lab step records
		m_nLabStepID = nLabStepID;

		// (a.walling 2008-09-03 11:35) - PLID 19638 - Also include PicID
		m_nPicID = nPicID;

		m_eTargetFormat = eTargetFormat;

		// (a.walling 2010-01-28 08:43) - PLID 28806 - Load properties from the main thread
		LoadProperties();
	}

	CNxTwain::~CNxTwain()
	{
	}

	// (a.walling 2010-01-28 08:43) - PLID 28806 - Load properties from the main thread
	void CNxTwain::LoadProperties()
	{		
		// (a.walling 2010-04-13 11:06) - PLID 38171 - Bulk cache properties
		g_propManager.CachePropertiesInBulk("TWAINPropertiesNum", propNumber,
			"(Username = '<None>') AND ("
			"Name IN ("
				"'TWAINSave8BitAsJPEG_2', 'TWAINApplyMonochromePalette', 'TWAINApplyGreyscalePalette', "
				"'TWAINScanToRemoteFolder', 'TWAINScanToDocumentFolder', 'PDF_AutoLandscape', "
				"'PDF_UseThumbs', 'PDF_PageSize'"
			"))");

		g_propManager.CachePropertiesInBulk("TWAINPropertiesText", propText,
			"(Username = '<None>') AND ("
			"Name IN ("
				"'TWAINRemoteFolder'"
			"))");

		// (j.armen 2011-10-25 14:08) - PLID 46139 - GetPracPath is referencing ConfigRT
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
		g_propManager.CachePropertiesInBulk("TWAINPropertiesNumLocal", propNumber,
			"(Username = '%s') AND ("
			"Name IN ("
				"'TWAINShowUI', 'TWAINAutoFeed'"
			"))", _Q(strUserParam));

		m_bShowUI = GetRemotePropertyInt("TWAINShowUI", 1, 0, strUserParam, false) ? true : false;
		m_bAutoFeed = GetRemotePropertyInt("TWAINAutoFeed", 0, 0, strUserParam, false) ? true : false;

		m_strRemoteFolder = GetRemotePropertyText("TWAINRemoteFolder","",0,"<None>", true);
		// (a.walling 2010-04-08 17:56) - PLID 38171 - Created 'new' hidden property for this, with the default of TRUE. I've noticed several 
		// offices scanning gigantic 5mb PNG files in grayscale because the contrast is not up enough, so PNG tries to encode all the noise.
		m_bSave8BitAsJPEG = GetRemotePropertyInt("TWAINSave8BitAsJPEG_2", TRUE, 0, "<None>") ? true : false;
		// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the monochrome palette, default to false.
		m_bApplyMonochromePalette = GetRemotePropertyInt("TWAINApplyMonochromePalette", FALSE, 0, "<None>") ? true : false; 
		// (a.walling 2010-04-13 10:53) - PLID 38171 - Whether to apply the greyscale palette, default to true
		m_bApplyGreyscalePalette = GetRemotePropertyInt("TWAINApplyGreyscalePalette", TRUE, 0, "<None>") ? true : false; 
		m_bScanToRemoteFolder = GetRemotePropertyInt("TWAINScanToRemoteFolder",0,0,"<None>", true) ? true : false;
		m_bScanToDocumentFolder = GetRemotePropertyInt("TWAINScanToDocumentFolder",1,0,"<None>", true) ? true : false;

		m_bPDFAutoLandscape = GetRemotePropertyInt("PDF_AutoLandscape", TRUE, 0, "<None>") ? true : false;
		m_bPDFUseThumbs = GetRemotePropertyInt("PDF_UseThumbs", TRUE, 0, "<None>") ? true : false;
		m_nPDFPageSize = GetRemotePropertyInt("PDF_PageSize", NxPdf::psLetter, 0, "<None>", true);	

		// (a.walling 2010-01-28 14:09) - PLID 28806
		// (a.walling 2010-02-01 12:33) - PLID 28806 - These prompt, but we don't know whether we will save as a JPEG or PNG
		// until we have already scanned and we are in a thread. So, we check these silently. If there is a a prompt required,
		// then we simply prompt the user whether to attach as a photo. The user probably does not care what filetype the scanned
		// image is saved as internally. If there is a mismatch, but no prompt, then we will respect the preferences.
		bool bJPEGPrompt = false;
		bool bPNGPrompt = false;
		m_bIsPhotoJPEG = IsHistoryImageAPhoto("badger.jpg", true, &bJPEGPrompt) ? true : false;
		m_bIsPhotoPNG = IsHistoryImageAPhoto("wombat.png", true, &bPNGPrompt) ? true : false;

		// Don't prompt if we are scanning to a PDF, since it doesn't matter for that.
		if (m_eTargetFormat != eScanToMultiPDF && m_eTargetFormat != eScanToPDF) {
			// Also don't prompt if we don't have a valid personID, since that is used to denote that we do not intend to attach
			if (m_nPersonID != -25) {
				// (a.walling 2010-02-01 12:36) - PLID 28806 - One or more file types required a prompt. So prompt now, filetype-agnostic.
				if (bJPEGPrompt || bPNGPrompt) {
					if (IDYES == AfxMessageBox("Would you like to attach the scanned image as a photo?  This means that the image will appear in the "
								"'Photos' category of the History tab.", MB_YESNO|MB_ICONQUESTION))
					{
						m_bIsPhotoJPEG = true;
						m_bIsPhotoPNG = true;
					} else {
						m_bIsPhotoJPEG = false;
						m_bIsPhotoPNG = false;
					}
				}
			}
		}
	}

	void EnsureTWAINThreadActive()
	{
		if (!g_pTWAINThread) {
			// (a.walling 2008-09-18 09:50) - PLID 31285 - Create a handle event to wait for the thread to initialize
			HANDLE hevThreadInitialized = CreateEvent(NULL, TRUE, FALSE, NULL);
			g_pTWAINThread = AfxBeginThread(TWAINThread, hevThreadInitialized, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);			
			//g_pTWAINThread->m_bAutoDelete = FALSE;
			// (a.walling 2008-09-08 08:35) - PLID 31285 - We want to auto-delete ourself!
			g_pTWAINThread->m_bAutoDelete = TRUE;
			g_pTWAINThread->ResumeThread();

			if (hevThreadInitialized) {
				// (a.walling 2008-09-18 09:50) - PLID 31285 - Don't return until the thread is initialized
				DWORD dwWaitResult = WaitForSingleObject(hevThreadInitialized, INFINITE);
				if (dwWaitResult == WAIT_OBJECT_0) {
					// success!
				} else {
					// failure?
					LogDetail("NXTWAINLib: failure waiting for thread initialized event (0x%08x, 0x%08x)", dwWaitResult, GetLastError());
				}
				CloseHandle(hevThreadInitialized);
			}
		}
	}

	BOOL IsAcquiring()
	{
		// (a.walling 2010-01-28 15:28) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		if (UsingTwain2()) {
			// (a.walling 2010-01-27 14:42) - PLID 36568 - Find out if the interface is busy
			if (g_pPracticeTwainInterface) {
				return g_pPracticeTwainInterface->IsBusy() ? TRUE : FALSE;
			} else {
				return FALSE;
			}
		} else {
			if (g_hevAcquiring) {
				if (WAIT_TIMEOUT == WaitForSingleObject(g_hevAcquiring, 0))
				{
					return TRUE;
				}
			}
			return FALSE;
		}
	}
	
	// (a.walling 2008-07-24 13:28) - PLID 30836 - Also pass the image in memory
	// (a.walling 2008-09-03 11:35) - PLID 19638 - Also include PicID
	// (a.walling 2008-09-03 13:29) - PLID 22821 - Use Gdiplus rather than CxImage
	// (a.walling 2008-09-05 15:50) - PLID 22821 - Support PDF targets
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	BOOL Acquire(long nPersonID, const CString& strDocumentPath, void (WINAPI *cb)(EScanType, const CString&, BOOL&, void*, CxImage&),
		void (WINAPI *cbPrecompress)(const LPBITMAPINFO, BOOL&, BOOL&, long&, ADODB::_Connection*), 
		void* pUserData, const CString& strDeviceName /* = "" */, long nCategoryID /*= -1*/, long nLabStepID /*= -1*/, long nPicID /*= -1*/,
		EScanTargetFormat eTargetFormat /* = eScanToImage*/)
	{
		// Make sure the user has Windows permission to write to this path
		if (!FileUtils::IsPathWritable(strDocumentPath))
		{
			CString permissionsMessage = "The file can not be written to the file path \n'" + strDocumentPath + "'. \nMake sure you have Windows permission to write to this folder.";
			AfxMessageBox(permissionsMessage);
			return FALSE;
		}

		// (m.hancock 2006-06-27 16:04) - PLID 21071 - Added field for associating MailSent records with lab step records
		CNxTwain* pTwain = new CNxTwain(nPersonID, strDocumentPath, cb,
			cbPrecompress, pUserData, strDeviceName, nCategoryID, nLabStepID, nPicID, eTargetFormat);

		// (a.walling 2010-01-28 15:29) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		if (UsingTwain2()) {
			if (!g_pPracticeTwainInterface->IsBusy()) {
				g_pPracticeTwainInterface->SetNotifyWindow(GetMainFrame()->GetSafeHwnd());

				SetGlobals(pTwain);
				delete pTwain;
				pTwain = NULL;

				// (a.walling 2010-01-28 13:28) - PLID 28806 - Use the cached property
				// (a.walling 2010-01-27 14:50) - PLID 36568 - Go ahead and scan!
				g_pPracticeTwainInterface->Acquire(::GetActiveWindow(), g_bShowUI, g_bAutoFeed, strDeviceName);

				return TRUE;
			} else {
				delete pTwain;
				pTwain = NULL;
				return FALSE;
			}
		} else {
			EnsureTWAINThreadActive();
			BOOL bSuccess = g_pTWAINThread->PostThreadMessage(WM_USER + 2000, 0, (LPARAM)pTwain);
			// (a.walling 2008-09-08 08:41) - PLID 31285 - Log if we fail communicating. This should not
			// fail now that we reset our global thread pointer when the thread message pump is destroyed
			if (!bSuccess) {
				Log(FormatLastError("Failed to communicate with TWAIN thread."));
			}
			return FALSE;
		}
	}

	BOOL SelectSource()
	{
		// (a.walling 2010-01-28 15:29) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		if (UsingTwain2()) {
			// (a.walling 2010-02-04 08:35) - PLID 36568 - Throw and catch any exceptions here
			HRESULT hr = ERROR_RESOURCE_NOT_PRESENT;
			try {
				if (g_pPracticeTwainInterface) {
					// (a.walling 2010-01-27 14:44) - PLID 36568 - Select our default source
					hr = g_pPracticeTwainInterface->SelectSource(::GetActiveWindow());
				}

				if (!SUCCEEDED(hr)) {
					_com_issue_error(hr);
				}
			} NxCatchAllThread("Failed to select TWAIN source");

			return SUCCEEDED(hr) ? TRUE : FALSE;
		} else {
			// (a.walling 2008-09-18 09:39) - PLID 31285 - Ensure our TWAIN thread is active so we know we have a valid
			// window for TWAIN messages.
			EnsureTWAINThreadActive();
			if (OpenDSM())
			{
				SelectDS();
				CloseDSM();
			}
			return FALSE;
		}
	}

	void SetTwainMessageRecipient(HWND hwndMessageWnd)
	{
		g_hwndMessageWnd = hwndMessageWnd;
	}





	// (a.walling 2010-01-27 14:32) - PLID 36568 - A native transfer has completed, so we need to encode and save into history
	void CPracticeTwainInterface::OnNativeTransfer(long nCurTransfer, BITMAPINFOHEADER* pDIB, BYTE* pBits, LPCTSTR szImageInfo, LPCTSTR szExImageInfo)
	{
		// (a.walling 2008-09-03 14:48) - PLID 22821 - Just use microsoft's built in function
		// and avoid the wierd CxImage workarounds above

		CString strImageInfo;
		strImageInfo.Format("Scanned image info:\r\n%s", szImageInfo);
		strImageInfo.Replace("\r\n", "\r\n\t");
		LogString(strImageInfo);
		
		strImageInfo.Format("Scanned image extended info:\r\n%s", szExImageInfo);
		strImageInfo.Replace("\r\n", "\r\n\t");
		LogString(strImageInfo);

		// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
		//Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromBITMAPINFO((BITMAPINFO*)pDIB, pBits);
		CxImage cxImage;
		// (a.walling 2010-04-08 09:10) - PLID 38171
		ProcessNativeImage(pDIB, cxImage);
		
		// Now save the image to disk
		SaveNativeImage(m_pCon, cxImage, pDIB, *m_pTwainProgressDlg, nCurTransfer, m_saTempPdfFiles, "");
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - All transfers complete, so send the NXM_TWAIN_XFERDONE message
	void CPracticeTwainInterface::OnTransfersComplete()
	{
		CreateMultiPDF(m_pCon, m_saTempPdfFiles);

		// (a.walling 2008-09-05 14:20) - PLID 22821 - Clear out any temp files we may have created
		for (int x = 0; x < m_saTempPdfFiles.GetSize(); x++) {
			if (!DeleteFile(m_saTempPdfFiles[x])) {
				ASSERT(FALSE);
				// (a.walling 2010-01-28 13:34) - PLID 28806 - Don't call into the mainframe from a different thread
				MoveFileAtStartup(m_saTempPdfFiles[x], NULL);
			}
		}

		m_saTempPdfFiles.RemoveAll();

		::PostMessage(m_hwndNotify, NXM_TWAIN_XFERDONE, 0, 0);

		ReportErrors();
	}

	// (a.walling 2010-01-28 12:38) - PLID 36568 - Add the error string to our list. We'll throw all these as a single exception later.
	void CPracticeTwainInterface::OnError(LPCTSTR szError)
	{
		m_saErrors.Add(szError);
	}

	// (a.walling 2010-01-28 12:38) - PLID 36568 - Report all of our errors and clear out the list.
	void CPracticeTwainInterface::ReportErrors()
	{
		if (!m_saErrors.IsEmpty()) {

			CString strErrors;
			for (int i = 0; i < m_saErrors.GetSize(); i++) {
				strErrors += m_saErrors.GetAt(i);
				strErrors.TrimRight("\n");
				strErrors += "\n";
			}

			strErrors.TrimRight("\n");

			// (a.walling 2010-03-15 14:59) - PLID 37109 - Special case -- show a friendlier message box
			if (strErrors.Find("There are no data sources available.") != -1) {
				// probably not the best thing to do in a thread, but this is how legacy TWAIN does it.
				AfxMessageBox("Practice was unable to access the input device that is selected for data acquisition. Please go to Tools=>Scanner/Digital Camera Settings=>Select Source menu and ensure that one item is selected. If there are no items to choose from, please install or troubleshoot the software package that came with your scanner, camera or other hardware.");

				m_saErrors.RemoveAll(); // will already be logged anyway
			}else if (strErrors.Find("TWCC_BUMMER") != -1 || strErrors.Find("Failed to open data source") != -1 ){
				// (b.savon 2013-06-25 10:18) - PLID 49821 - Handle common error gracefully
				// Using message box definition from above special case
				AfxMessageBox("Practice was unable to access the input device that is selected for data acquisition.  Please restart your scanner and/or computer and try again.  Otherwise, please contact NexTech support for more assistance.", MB_ICONINFORMATION);

				m_saErrors.RemoveAll();
			}else if (strErrors.Find("TWCC_OPERATIONERROR") != -1 || strErrors.Find("Cannot enable source") != -1){
				// (b.savon 2014-01-22 10:22) - PLID 59200 - Exception: Some trouble was encountered while scanning  An error has occurred: 
				// 'Cannot enable source' The condition code is: TWCC_OPERATIONERROR There was an error enabling the data source.
				AfxMessageBox("Practice was unable to enable the TWAIN data source.  Please restart your scanner and/or computer and ensure no other programs are currently using the device.  If this problem persists, please contact NexTech support for more assistance.", MB_ICONINFORMATION);
				
				m_saErrors.RemoveAll();
			}else if (strErrors.Find("Canceled transfer image") != -1){ // Note: I didn't include TWCC_SUCCESS here because I imagine there are others that we should be aware of and make appopriate elegant errors that have a different meaningful message
				// (b.savon 2014-01-22 10:34) - PLID 58368 - Exception: Some trouble was encountered while scanning
				// An error has occurred: 'Canceled transfer image' The condition code is: TWCC_SUCCES
				AfxMessageBox("Practice was unable to transfer the image.  Please restart your scanner and/or computer and ensure no other programs are currently using the device.  If this problem persists, please contact NexTech support for more assistance.", MB_ICONINFORMATION);
				
				m_saErrors.RemoveAll();
		    }else {
				try {
					ThrowNxException("%s", strErrors);
				} NxCatchAllThread("Some trouble was encountered while scanning");

				m_saErrors.RemoveAll();
			}
		}
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - Create our progress dialog
	void CPracticeTwainInterface::OnProgressBegin()
	{
		DestroyProgressDialog();
		m_pTwainProgressDlg = new CNxTwainProgressDlg;
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - Set the dialog's text
	void CPracticeTwainInterface::OnProgressText(LPCTSTR szProgressText)
	{
		if (m_pTwainProgressDlg) {
			m_pTwainProgressDlg->SetProgressText(szProgressText);
		}
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - And destroy the dialog
	void CPracticeTwainInterface::OnProgressEnd()
	{
		DestroyProgressDialog();
	}
	
	// return true to cancel processing, if necessary
	bool CPracticeTwainInterface::ProcessMessage(MSG* pMsg)
	{
		if (pMsg->message != NXM_EXTEND_INACTIVITY_TIMEOUT && IsInputMsg(pMsg->message)) {
			::PostMessage(m_hwndNotify, NXM_EXTEND_INACTIVITY_TIMEOUT, 0, 0);
		}

		return NxTwainInterface::ProcessMessage(pMsg);
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - The scanning thread was created, so ensure everything we need is ready
	void CPracticeTwainInterface::OnThreadCreate()
	{
		NxTwainInterface::OnThreadCreate();
		
		// (a.walling 2010-01-28 13:43) - PLID 28806 - Threads need to have their own connection, let's create one, based on the global connection.
		// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
		_ConnectionPtr pConn = GetThreadRemoteData();

		m_pCon = pConn;
	}

	// (a.walling 2010-01-27 14:32) - PLID 36568 - The thread is ending, so report any errors and destroy anything
	void CPracticeTwainInterface::OnThreadFinalize()
	{
		ReportErrors();
		DestroyProgressDialog();
		m_pCon->Close();
		m_pCon = NULL;
		NxTwainInterface::OnThreadFinalize();
	}
	
	// (a.walling 2010-01-27 14:32) - PLID 36568
	void CPracticeTwainInterface::DestroyProgressDialog()
	{
		if (m_pTwainProgressDlg) {
			delete m_pTwainProgressDlg;
			m_pTwainProgressDlg = NULL;
		}
	}
};
