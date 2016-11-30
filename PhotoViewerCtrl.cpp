// PhotoViewerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "PhotoViewerCtrl.h"
#include "pracprops.h"
//For GetPatientDocumentPath()
#include "MergeEngine.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalDrawingUtils.h"
#include "PhotoViewerDlg.h"
#include "Mirror.h"
#include "UnitedLink.h"
#include "SelectImageDlg.h"
#include "MultiSelectDlg.h"
#include "FileUtils.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include "IconUtils.h"
#include "AuditTrail.h"
#include "MirrorPatientImageMgr.h"
#include "TaskEditDlg.h"
// (a.walling 2010-12-20 16:05) - PLID 41917
#include <boost/make_shared.hpp>
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include "PicContainerDlg.h"
#include "HL7Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2011-01-26 09:29) - PLID 42178 - Deleted some more commented-out stuff that started to make things hard to follow


// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2008-12-08 13:35) - PLID 32363 - All instances on CSingleLock were assuming the lock was initially acquired;
// this was incorrect. As a result, all the constructors were modified to take a second param of TRUE to acquire initially.


// (a.walling 2010-12-20 16:05) - PLID 41917 - Take a const CString reference
void LoadErrorImage(HBITMAP &hImage, const CString &strFile);


// (a.walling 2010-12-20 16:05) - PLID 41917 - Container + critical section for smart pointers passed asynchronously in this window via PostMessage
std::vector<LoadingImageInformationPtr> g_arLoadingImagesPosted;
CCriticalSection g_csLoadingImagesPosted;


// (a.walling 2011-01-26 09:29) - PLID 42178 - Years of accumulated hacks could have been avoided by simply moving the thread logic for loading images
// into a class that separates that logic from that of the photoviewer control. This simplified code greatly, as well as eliminated a race condition
// with the way the asyncloadinfo used to be passed to the thread.
// (z.manning 2011-05-06 10:16) - PLEASE NOTE: It is not safe to auto-delete this thread because this class stops the thread in
// AsyncImageLoader::Stop but after triggering the thread to end still references class members.
class AsyncImageLoader : public AsyncThread
{
public:
	AsyncImageLoader(HWND hwndToNotify);

	~AsyncImageLoader();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Queue this image to be loaded
	void QueueImage(LoadingImageInformationPtr pLoad);

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Remove all queued images
	void ClearQueue();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Are we busy or idle?
	BOOL IsLoading();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Stop processing and eventually exit and delete the thread
	void Stop(DWORD dwWait);

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Get the critical section
	CCriticalSection& GetLock();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Get the mirror critical section (for the CMirrorPatientImageMgr)
	CCriticalSection& GetMirrorLock();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Get the list of images to load (to be sorted, for example)
	CList<LoadingImageInformationPtr,LoadingImageInformationPtr>& GetImagesLoading();

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Set or reset the mirror image manager
	void SetMirrorPatientImageMgr(class CMirrorPatientImageMgr* pThreadMirrorImageMgr);

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Return true if a mirror image manager is set.
	bool HasMirrorPatientImageLoader();

protected:
	HWND hwndMessage; // HWND to which NXM_PHOTO_LOADED responses shall be posted

	CList<LoadingImageInformationPtr,LoadingImageInformationPtr> imagesLoading; // list of images to load
	
	CCriticalSection cs; // generic critical section for the list and other internal states
	CCriticalSection csMirror;  // critical section for the mirror image manager

	HANDLE eventContinue; // event is set when an image is queued, reset when idle
	HANDLE eventQuit; // event is set when the thread should stop and delete itself

	// (c.haag 2010-02-23 13:40) - PLID 37364 - This object acts as a manager / cache source for Mirror image-related data
	class CMirrorPatientImageMgr* m_pThreadMirrorImageMgr;

	virtual UINT RunThread(); // virtual function from AsyncThread; wraps LoadImageLoop and also waits for eventQuit in case of exceptions (to prevent auto-delete)

	UINT LoadImageLoop(); // Runs the actual logic.
}; 


AsyncImageLoader::AsyncImageLoader(HWND hwndToNotify)
	: AsyncThread(TRUE) // auto delete
	, eventContinue(CreateEvent(NULL, TRUE, TRUE, NULL))
	, eventQuit(CreateEvent(NULL, TRUE, FALSE, NULL))
	, hwndMessage(hwndToNotify)
	, m_pThreadMirrorImageMgr(NULL)
{
}

AsyncImageLoader::~AsyncImageLoader()
{
	::CloseHandle(eventContinue);
	eventContinue = NULL;

	::CloseHandle(eventQuit);
	eventQuit = NULL;

	// (c.haag 2010-02-23 16:00) - PLID 37364 - Cleanup
	delete m_pThreadMirrorImageMgr;
	m_pThreadMirrorImageMgr = NULL;
}

void AsyncImageLoader::QueueImage(LoadingImageInformationPtr pLoad)
{
	CSingleLock sl(&cs, TRUE);

	imagesLoading.AddTail(pLoad);

	SetEvent(eventContinue);
}

void AsyncImageLoader::ClearQueue()
{
	CSingleLock sl(&cs, TRUE);

	imagesLoading.RemoveAll();

	SetEvent(eventContinue);
}

BOOL AsyncImageLoader::IsLoading()
{
	{
		CSingleLock sl(&cs, TRUE);

		if (!imagesLoading.IsEmpty()) {
			return TRUE;
		}
	}

	if (WAIT_OBJECT_0 == WaitForSingleObject(eventContinue, 0)) {
		return TRUE;
	}

	return FALSE;
}

void AsyncImageLoader::Stop(DWORD dwWait)
{
	{
		CSingleLock sl(&cs, TRUE);

		ResetEvent(eventContinue);
		SetEvent(eventQuit);
	}

	int nResult = WaitForSingleObject(m_hThread, dwWait);

	ASSERT(nResult == WAIT_OBJECT_0); // if this fails, the thread did not stop in a timely fashion.
}

CCriticalSection& AsyncImageLoader::GetLock()
{
	return cs;
}

CCriticalSection& AsyncImageLoader::GetMirrorLock()
{
	return csMirror;
}

CList<LoadingImageInformationPtr,LoadingImageInformationPtr>& AsyncImageLoader::GetImagesLoading()
{
	return imagesLoading;
}

void AsyncImageLoader::SetMirrorPatientImageMgr(CMirrorPatientImageMgr* pThreadMirrorImageMgr)
{
	CSingleLock sl(&csMirror, TRUE);
	delete m_pThreadMirrorImageMgr;
	m_pThreadMirrorImageMgr = pThreadMirrorImageMgr;
}

bool AsyncImageLoader::HasMirrorPatientImageLoader()
{
	return m_pThreadMirrorImageMgr != NULL;
}

UINT AsyncImageLoader::RunThread()
{
	try {
		return LoadImageLoop();
	} NxCatchAllThread(__FUNCTION__);

	// uh oh, an error. Well just wait for the quit event then.
	WaitForSingleObject(eventQuit, INFINITE);
	return -1;
}

UINT AsyncImageLoader::LoadImageLoop()
{
	// (a.walling 2008-09-23 11:47) - PLID 31479 - This now loops. The Continue event should be set
	// whenever there is something to load in the list. The quit event signals the thread to exit.
	// (a.walling 2010-12-20 16:05) - PLID 41917 - Now we wait for both handles, and set bExit when
	// the Quit event is called. This thread will stay alive for the duration of the dialog now.
	// When we run out of images to load, we reset the continue event and wait for it (or the quit event)
	// to be signalled.
	bool bExit = false;
	HANDLE arHandles[2] = {eventContinue, eventQuit};
	while (!bExit) {
		int nResult = WaitForMultipleObjects(2, arHandles, FALSE, INFINITE);

		if ((WAIT_OBJECT_0) != nResult) { // quit event set, or something failed
			bExit = true;
			continue;
		}

		nResult = WaitForSingleObject(eventQuit, 0);
		if (nResult == WAIT_OBJECT_0) { // quit event set
			bExit = true;
		}

		// (a.walling 2010-12-20 16:05) - PLID 41917
		LoadingImageInformationPtr pLoad;
		CSingleLock slMirror(&csMirror, TRUE);
		{
			// (a.walling 2008-12-08 13:38) - PLID 32363 - Pass TRUE to ensure we initially lock the critical section
			CSingleLock sl(&cs, TRUE);

			if (!imagesLoading.IsEmpty()) {
				pLoad = imagesLoading.RemoveHead();
			}

			if (!pLoad) {
				// nothing left in the list, set our event.
				ResetEvent(eventContinue);
				continue;
			}
		}

		if (pLoad->ii->nType != eImageSrcMirror) {
			slMirror.Unlock();
		}
		
		// Only load the image if we actually have information to work with
		switch(pLoad->ii->nType) {
		case eImageSrcMirror:
			{
				//LoadMirrorImage gives us the thumbnail, which we'll use for everything.
				// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
				// (c.haag 2010-02-23 13:31) - PLID 37364 - Use the manager instead of accessing the Mirror namespace directly
				if (NULL != m_pThreadMirrorImageMgr) {
					pLoad->ii->hFullImage = m_pThreadMirrorImageMgr->LoadMirrorImage(pLoad->ii->nID, pLoad->nMirrorCount, -1);
				} else {
					ThrowNxException("Attempted to load a Mirror image without a valid image manager");
				}
				slMirror.Unlock();

				if (pLoad->ii->hFullImage == NULL) {
					pLoad->ii->bIsImageValid = false;
					pLoad->ii->hThumb = NULL;
					pLoad->ii->bIsThumbValid = false;
				}
				else {
					pLoad->ii->bIsImageValid = true;
					pLoad->ii->hThumb = pLoad->ii->hFullImage;
					pLoad->ii->bIsThumbValid = true;
				}
			}
			break;
		case eImageSrcUnited:
			pLoad->ii->hFullImage = GetMainFrame()->GetUnitedLink()->LoadImage(pLoad->nUnitedID, pLoad->ii->nID);
			if (pLoad->ii->hFullImage == NULL) {
				pLoad->ii->bIsImageValid = false;
				pLoad->ii->hThumb = NULL;
				pLoad->ii->bIsThumbValid = false;
			}
			else {
				pLoad->ii->bIsImageValid = true;
				pLoad->ii->hThumb = pLoad->ii->hFullImage;
				pLoad->ii->bIsThumbValid = true;
			}
			// (c.haag 2007-07-03 15:00) - PLID 26550 - There used to not be
			// a break here.
			break;
		case eImageSrcPractice:
			{
				// (a.walling 2007-01-30 09:08) - PLID 24475 - Use this function to load the thumbnail. It will be (re)written if necessary transparent to us.
				// (a.walling 2013-04-24 14:57) - PLID 56247 - Renamed, still behaves the same
				pLoad->ii->hThumb = LoadThumbFromFile(pLoad->ii->strFullFilePath);
				if (pLoad->ii->hThumb != NULL) {
					pLoad->ii->bIsThumbValid = true;
				}
				else {
					pLoad->ii->bIsThumbValid = false;
				}
			}
			break;
		case eImageSrcDiagram:
			{
				// (a.walling 2007-01-30 09:08) - PLID 24475 - Use this function to load the thumbnail. It will be (re)written if necessary transparent to us.
				// (a.walling 2013-04-24 14:57) - PLID 56247 - Renamed, still behaves the same
				pLoad->ii->hThumb = LoadThumbFromFile(pLoad->ii->strFullFilePath);
				if (pLoad->ii->hThumb != NULL) {
					pLoad->ii->bIsThumbValid = true;
				}
				else {
					pLoad->ii->bIsThumbValid = false;
				}
			}
			break;
		}

		// (a.walling 2006-10-20 12:34) - PLID 23168 - Now let's cleanup
		if (pLoad->ii->bIsImageValid) {
			if (pLoad->ii->bIsThumbValid) {
				// a.walling - if the thumb is valid, delete the possibly huge full image object.
				// (a.walling 2007-02-15 17:33) - PLID 23168 - Mirror returns the thumb and we put that in full image
				// so be sure to only delete the full image if they are inequal.
				if (pLoad->ii->hFullImage != pLoad->ii->hThumb) {
					DeleteObject(pLoad->ii->hFullImage);
				}
				pLoad->ii->hFullImage = pLoad->ii->hThumb;
			}
			else {
				pLoad->ii->hThumb = pLoad->ii->hFullImage;
			}
		}
		else {
			pLoad->ii->bIsImageValid = false;
			pLoad->ii->hFullImage = NULL;
		}

		if (!(pLoad->ii->bIsThumbValid)) {
			// (a.walling 2006-10-26 09:23) - PLID 23168 - There is no thumb at all! Load an error message
			//		this should be taken care of with more detail above. this is a failsafe.
			CString str("Error");
			LoadErrorImage(pLoad->ii->hThumb, str);
			pLoad->ii->bIsThumbValid = true;
		}

		// Now tell the window that we are done loading the image
		// (a.walling 2008-09-23 11:51) - PLID 31479 - Only post if valid window; the window should
		// be responsible for cleaning up. If it is invalid, we can clean up the memory
		if (::IsWindow(hwndMessage)) {
			// (a.walling 2010-12-20 16:05) - PLID 41917 - Append this to the global posted images vector to ensure
			// the pointer stays alive during the async message.
			CSingleLock sl(&g_csLoadingImagesPosted, TRUE);
			g_arLoadingImagesPosted.push_back(pLoad);
			::PostMessage(hwndMessage, NXM_PHOTO_LOADED, (WPARAM)pLoad.get(), NULL);
		}
	}

	return 0;
}


ProcNameRect::~ProcNameRect() {
	/*if(pRgn) {
		if(pRgn->GetSafeHandle()) {
			pRgn->DeleteObject();
			delete pRgn;
			pRgn = NULL;
		}
	}*/
}
ProcNameRect::ProcNameRect(ProcNameRect &pnrSource){
	// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
	arynProcedureIDs.RemoveAll();
	arynProcedureIDs.Append(pnrSource.arynProcedureIDs);
	strProcedureNames = pnrSource.strProcedureNames;
	rDrawn = pnrSource.rDrawn;
	pRgn = pnrSource.pRgn;
}
void ProcNameRect::operator =(ProcNameRect &pnrSource) {
	// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
	arynProcedureIDs.RemoveAll();
	arynProcedureIDs.Append(pnrSource.arynProcedureIDs);
	strProcedureNames = pnrSource.strProcedureNames;
	rDrawn = pnrSource.rDrawn;
	pRgn = pnrSource.pRgn;
}

//(e.lally 2012-04-13) PLID 49637
CString ImageInformation::GetTextToDraw()
{
	CString strTextToDraw;
	//(e.lally 2012-04-13) PLID 49637 - Preference for which fields to show and what order. Value is the index, positive indicates "show".
	CStringArray aryTextToDraw;
	aryTextToDraw.SetSize(10);
	//Default to NOT show
	long nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Attached"), -1, 0, GetCurrentUserName(), true);

	long nDOSIndex = GetRemotePropertyInt(CString("PhotosLbl_ServiceDate"), 2, 0, GetCurrentUserName(), true);

	//Use "A:" prefix for attached if both dates are showing
	CString strAttachDatePrefix = (nDOSIndex > 0 ) ? "A: " : "";
	//Use "DOS: " prefix for service date if both dates are showing
	CString strServiceDatePrefix = (nAryIndex > 0 ) ? "DOS: " : "";

	//Attached Date
	if(nAryIndex > 0 && dtAttached.m_dt > 0 && dtAttached.m_status == COleDateTime::valid){
		aryTextToDraw.SetAt(nAryIndex-1, strAttachDatePrefix + FormatDateTimeForInterface(dtAttached, 0, dtoDate));
	}

	//(e.lally 2012-04-16) PLID 39543
	//Service Date
	if(nDOSIndex > 0 && dtService.m_dt > 0 && dtService.m_status == COleDateTime::valid){
		aryTextToDraw.SetAt(nDOSIndex-1, strServiceDatePrefix + FormatDateTimeForInterface(dtService, 0, dtoDate));
	}

	//Procedures - Default to NOT show
	nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Procedures"), -3, 0, GetCurrentUserName(), true);
	if(nAryIndex > 0){
		aryTextToDraw.SetAt(nAryIndex-1, strProcedureNames);
	}

	//Staff - Default to NOT show
	nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Staff"), -5, 0, GetCurrentUserName(), true);
	if(nAryIndex > 0){
		aryTextToDraw.SetAt(nAryIndex-1, strStaff);
	}

	bool bShowFilename = false;
	CString strFileNoPath = strFullFilePath.Mid(strFullFilePath.ReverseFind('\\')+1);
	//File Name - Default to NOT show
	nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Filename"), -6, 0, GetCurrentUserName(), true);
	if(nAryIndex > 0){
		bShowFilename = true;
		aryTextToDraw.SetAt(nAryIndex-1, strFileNoPath);
	}

	//Notes
	nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Notes"), 4, 0, GetCurrentUserName(), true);
	if(nAryIndex > 0){
		//Don't show the notes if we are already showing the filename and it is redundant
		if(bShowFilename == false || (strFileNoPath != strNotes && strFullFilePath != strNotes)){
			aryTextToDraw.SetAt(nAryIndex-1, strNotes);
		}
	}

	//Category
	nAryIndex = GetRemotePropertyInt(CString("PhotosLbl_Category"), 7, 0, GetCurrentUserName(), true);
	if(nAryIndex > 0){
		aryTextToDraw.SetAt(nAryIndex-1, strCategory);
	}


	for(int i =0; i<aryTextToDraw.GetCount(); i++){
		if(!aryTextToDraw.ElementAt(i).IsEmpty()){
			strTextToDraw += aryTextToDraw.ElementAt(i) + "\r\n";
		}
	}

	// (j.jones 2012-06-26 11:50) - PLID 49637 - replace & with && or else it turns into accelerators
	strTextToDraw.Replace("&", "&&");
	
	strTextToDraw.TrimRight("\r\n");
	return strTextToDraw;
}

/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerCtrl
#define IDC_TEMP_EDIT	1000

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_ROTATE_90	40100
#define ID_ROTATE_180	40101
#define ID_ROTATE_270	40102
#define ID_OPEN_PICTURE	40103
#define ID_TOGGLE_PRIMARY	40104
#define ID_EDIT_NOTES	40105
#define ID_LAUNCH_MIRROR	40106
#define	ID_LAUNCH_UNITED	40107
#define ID_DETACH		40108
#define ID_DETACH_AND_DELETE	40109
//DRT 8/1/2005 - PLID 17125 - Procedures no longer require to be the highest ID.  I've also added categories which
//	are a similar popup menu type.
//Note:  Due to the way we shift numbers to generate our IDs, you cannot create any menu IDs greater than 7208960
//	(110 << 16), as those numbers are variable for the IDs.  We are however limited to IDs lower than 65536, which
//	I am claiming an acceptable compromise at this point.
#define	ID_ASSIGN_PROC_BASE		40110
#define ID_ASSIGN_CATEGORY_BASE	40111
#define ID_MARK_AS_NON_PHOTO	40112
#define ID_NEW_TODO	40113

#define HOVER_TICK	100
#define KILL_TIP_NCMOVE	101
#define EDIT_TIMER	102

// (a.walling 2008-09-24 13:34) - PLID 31495 - Fake scrollbar message for easier handling of wheel deltas
#define SB_NX_MOUSEWHEEL 0xFCA5CADE

CPhotoViewerCtrl::CPhotoViewerCtrl()
{
	m_AsyncImageLoader = NULL; // (a.walling 2011-01-26 09:29) - PLID 42178
	m_nPersonID = 0;
	m_nCurrentHotImage = -1;
	m_pToolTipWnd = NULL;
	m_bControlDown = false;
	m_nScrollBarWidth = 0;
	m_nScrollPos = 0;
	m_nPrimaryImageID = -1;
	m_pTempEdit = NULL;
	m_pIconFont = NULL;
	m_bCanEditNotes = false;
	m_bShowPreviewArea = true;
	m_bAllowMultiSelect = true;
	m_bShowUnited = true;
	m_bShowMirror = true;
	m_bIsPatient = false;
	m_nDragType = dtNull;
	m_nDraggedImage = -1;
	m_bDragStarted = false;
	m_pGhostlyImageWnd = NULL;
	m_hScreenshot = NULL;
	m_nEMRLevel = -1;
	m_nPicID = -1;
	m_strAdditionalFilter = "";
	m_hLoadingThumb = NULL;
	m_nSortInfoCount = 0;
	m_bUsingCategoryFilter = false;
	// The default sort order is procedure name ascending, then date descending
	SetPrimarySort(dscAttachDate, FALSE); // set secondary first
	SetPrimarySort(dscProcedureName, TRUE); // then override it with the primary
	// (a.walling 2010-12-22 08:28) - PLID 41918 - dirty flag for sorting
	m_bNeedsSort = true;

	// (a.walling 2008-09-24 10:31) - PLID 31495
	m_nMaxHeight = 0;
	m_nNumColumns = 0;

	// (c.haag 2010-02-25 10:35) - PLID 37364
	m_pMirrorImageMgr = NULL;

	m_bDrawTextLabels = true; //(e.lally 2012-04-24) PLID 49637

	m_pPicContainer = NULL;
}

CPhotoViewerCtrl::~CPhotoViewerCtrl()
{
	try {

		ClearImageArray();
		ClearProcNameRects();

		//DRT 12/30/2004 - PLID 15144 - Fixed memory leak, we need to call
		//	'delete' even if there is no hwnd.
		if(m_pToolTipWnd) {
			if(m_pToolTipWnd->GetSafeHwnd()) {
				m_pToolTipWnd->DestroyWindow();
			}

			delete m_pToolTipWnd;
			m_pToolTipWnd = NULL;
		}

		if(m_pIconFont) {
			m_pIconFont->DeleteObject();
			delete m_pIconFont;
			m_pIconFont = NULL;
		}

		if(m_pGhostlyImageWnd) {
			m_pGhostlyImageWnd->DestroyWindow();
			delete m_pGhostlyImageWnd;
			m_pGhostlyImageWnd = NULL;
		}

		if(m_pTempEdit) {
			m_pTempEdit->DestroyWindow();
			delete m_pTempEdit;
			m_pTempEdit = NULL;
		}

		if(m_hLoadingThumb) {
			DeleteObject(m_hLoadingThumb);
			m_hLoadingThumb = NULL;
		}

		// (c.haag 2010-02-25 10:31) - PLID 37364 - Cleanup
		EnsureNotMirrorImageMgr();

		// (a.walling 2010-12-20 16:05) - PLID 41917
		g_arLoadingImagesPosted.clear();

	} NxCatchAll("Destroying CPhotoViewerCtrl");
}


BEGIN_MESSAGE_MAP(CPhotoViewerCtrl, CWnd)
	//{{AFX_MSG_MAP(CPhotoViewerCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_ROTATE_90, OnRotate90)
	ON_COMMAND(ID_ROTATE_180, OnRotate180)
	ON_COMMAND(ID_ROTATE_270, OnRotate270)
	ON_COMMAND(ID_OPEN_PICTURE, OnOpenPicture)
	ON_COMMAND(ID_TOGGLE_PRIMARY, OnTogglePrimary)
	ON_COMMAND(ID_EDIT_NOTES, OnEditNotes)	
	ON_EN_KILLFOCUS(IDC_TEMP_EDIT, OnKillFocusTempEdit)
	ON_EN_CHANGE(IDC_TEMP_EDIT, OnChangeTempEdit)
	ON_COMMAND(ID_LAUNCH_MIRROR, OnLaunchMirror)
	ON_COMMAND(ID_LAUNCH_UNITED, OnLaunchUnited)
	ON_COMMAND(ID_ASSIGN_PROC_BASE, OnAssignProc)
	ON_WM_DESTROY()
	ON_COMMAND(ID_DETACH, OnDetach)
	ON_MESSAGE(NXM_DETACH, OnNxmDetach)
	ON_COMMAND(ID_DETACH_AND_DELETE, OnDetachAndDelete)
	ON_MESSAGE(NXM_DETACH_AND_DELETE, OnNxmDetachAndDelete)
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(NXM_REFRESH_PHOTO_VIEWER, OnRefresh)
	ON_WM_CAPTURECHANGED()
	ON_MESSAGE(NXM_PHOTO_LOADED, OnPhotoLoaded)
	ON_COMMAND(ID_MARK_AS_NON_PHOTO, OnMarkAsNonPhoto)
	ON_COMMAND(ID_NEW_TODO, OnNewTodo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerCtrl message handlers
#define BUFFER_VERT	20
#define BUFFER_HORIZ	10
#define IMAGE_SIZE	100
void CPhotoViewerCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBorrowDeviceContext bdcMem(&dcMem);

	//Draw our background
	CRect rClient;
	GetClientRect(&rClient);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rClient.Width(), rClient.Height());
	
	dcMem.SelectObject(&bmp);

	if(!m_pIconFont) {
		//Let's load the system icon font.
		LOGFONT lf;
		if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
			m_pIconFont = new CFont;
			m_pIconFont->CreateFontIndirect(&lf);
		}
	}

	dcMem.SelectStockObject(WHITE_BRUSH);
	// (a.walling 2011-10-20 11:21) - PLID 46069 - Do not use a border
	BOOL bRet = ::FillRect(dcMem, rClient, (HBRUSH)GetStockObject(WHITE_BRUSH));

	CRect rMain = GetDisplayArea(daMainList);

	//Do we have our images loaded?
	if(!m_bImagesLoaded) {
		//Load them (this may be slow).
		LoadArray();
	}
	// (a.walling 2010-12-22 08:28) - PLID 41918 - Sort only if necessary
	if (m_bNeedsSort) {
		SortArray();
		m_bNeedsSort = false;
	}
	
	
	
	//Draw the main image area (this will recalculate the position of all the images).
	

	//Now that that's all done, show the scrollbar as necessary.
	int nOldPos = m_nScrollPos;
	SetScroll();

	DrawMainList(&dcMem, rMain);

	DrawPreviewArea(&dcMem, rClient);

	//Finally, draw our ghostly image.
	if(m_nDragType == dtImage) {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		DrawGhostlyImage(&dcMem, m_arImages.GetAt(m_nDraggedImage)->hThumb, pt);
	}

	dc.BitBlt(0,0,rClient.Width(),rClient.Height(), &dcMem, 0,0, SRCCOPY);
	bmp.DeleteObject();
	// Do not call CWnd::OnPaint() for painting messages
}

// (a.walling 2008-09-22 16:34) - PLID 31479 - Structure for visible image
struct VisibleImage {
	long nIndex; // index in m_arImages
	long nDrawn; // amount drawn (in px)
};

void CPhotoViewerCtrl::DrawMainList(CDC *pDC, CRect rRect)
{
	// (a.walling 2008-09-22 16:34) - PLID 31479 - List of images that have actually been drawn
	CArray<VisibleImage, VisibleImage> arVisibleImages;

	for(int i = 0; i < m_parRgnsToDelete.GetSize(); i++) {
		if(m_parRgnsToDelete.GetAt(i)) {
			//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
			((CRgn*)m_parRgnsToDelete.GetAt(i))->DeleteObject();
			delete (CRgn*)m_parRgnsToDelete.GetAt(i);
		}
	}
	m_parRgnsToDelete.RemoveAll();

	ClearProcNameRects();
	CBorrowDeviceContext bdc(pDC);

	CRect rClient;
	GetClientRect(rClient);

	// (a.walling 2008-09-24 09:55) - PLID 31495 - Need to adjust the relative scroll pos
	// to an absolute pixel value
	
	long nDispNumColumns = 0;
	long nTestPosX = 0;
	for(int z = 0; z < m_arImages.GetSize(); z++) {
		nTestPosX += BUFFER_HORIZ;
		if (nTestPosX + BUFFER_HORIZ + IMAGE_SIZE + BUFFER_HORIZ + m_nScrollBarWidth > rRect.Width()) {
			m_nNumColumns = nDispNumColumns;
			break;
		} else {
			nDispNumColumns++;
		}

		nTestPosX += IMAGE_SIZE;
	}

	if (nDispNumColumns == 0) {
		m_nNumColumns = 1;
	}

	nDispNumColumns = 0;

	if (m_nNumColumns == 0) m_nNumColumns = 1;

	long nNumRows = (m_arImages.GetSize() / m_nNumColumns);
	if (m_arImageRects.GetSize() > 0) {
		long nVertMax = m_arImageRects[m_arImageRects.GetSize() - 1].bottom;
		long nVertMin = m_arImageRects[0].top;

		m_nMaxHeight = (nVertMax - nVertMin) - (rRect.Height() / 2);
	} else {
		// estimate the maximum height
		m_nMaxHeight = ((IMAGE_SIZE + BUFFER_VERT) * nNumRows) - (rRect.Height() / 2);
	}
	double dRelativeScrollPos = m_nScrollPos / double(32767);
	long nAdjScrollPos = long(m_nMaxHeight * dRelativeScrollPos);

	int nCurrentPosX = 0;
	int nCurrentPosY = BUFFER_VERT-nAdjScrollPos;
	int nImageHeight = 0;
	CString strCurrentProcNames;
	CRgn *pRgnCurrentProcs = new CRgn;
	m_parRgnsToDelete.Add((void*)pRgnCurrentProcs);
	pRgnCurrentProcs->CreateRectRgn(0,0,0,0);


	// (a.walling 2008-09-24 09:17) - PLID 31479 - Set our clip region before the loop
	//Don't draw over our border.
	rClient.DeflateRect(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));
	CRgn rgnClient;
	rgnClient.CreateRectRgn(rClient.left, rClient.top, rClient.right, rClient.bottom);
	pDC->SelectClipRgn(&rgnClient);

	for(i = 0; i < m_arImages.GetSize(); i++) {
		//Find our next position.
		nCurrentPosX += BUFFER_HORIZ;

		//I'm factoring in the scroll bar width even though the scroll bar may not be visible, because I'm lazy.
		if((nDispNumColumns > 0) && nCurrentPosX + BUFFER_HORIZ + IMAGE_SIZE + BUFFER_HORIZ + m_nScrollBarWidth > rRect.Width()) {
			// (a.walling 2008-09-24 13:38) - PLID 31495 - Ensure our previously calculated column count matches up
			// with the columns we are actually drawing
			ASSERT(nDispNumColumns == m_nNumColumns);
			//Move to the next row.
			nCurrentPosX = BUFFER_HORIZ;
			nCurrentPosY += nImageHeight + BUFFER_VERT;
			nImageHeight = 0;
			nDispNumColumns = 1;
		} else {
			nDispNumColumns++;
		}
		
		long nDrawn = 0;
		// (a.walling 2008-09-24 09:16) - PLID 31479 - We'll set the clipping region before the loop
		CRect rDrawn = DrawImage(pDC, i, nCurrentPosX, nCurrentPosY, rClient, FALSE, &nDrawn);

		// (a.walling 2008-09-22 16:34) - PLID 31479 - If we drew something, and it has not loaded,
		// add it to our list in sorted order (by amount drawn, and by index)
		if (nDrawn > 0 && m_arImages[i]->hThumb == m_hLoadingThumb) {
			VisibleImage vi;
			vi.nIndex = i;
			vi.nDrawn = nDrawn;

			BOOL bAdded = FALSE;
			for (int f = 0; f < arVisibleImages.GetSize(); f++ ) {
				if (arVisibleImages[f].nDrawn >= vi.nDrawn) {
					arVisibleImages.InsertAt(f, vi);
					bAdded = TRUE;
					break;
				}
			}

			if (!bAdded) {
				arVisibleImages.Add(vi);
			}
		}
		
		// (j.jones 2009-10-13 11:10) - PLID 35894 - reworked to run off of procedure names
		if(m_arImages.GetAt(i)->strProcedureNames != strCurrentProcNames) {
			//The procedure has changed.  If the previous one was for a valid procedure, draw it.
			if(!strCurrentProcNames.IsEmpty()) {
				//Our previous region is complete.  Outline it.
				int nThickness = (strCurrentProcNames == m_strCurrentHotProcedures) ? 2 : 1;
				CBrush brBlack(RGB(0,0,0));
				bdc.m_pDC->FrameRgn(pRgnCurrentProcs, &brBlack, nThickness, nThickness);
				
				//Now, find the "upper left" of this region, to draw the name.
				CRect rBox;
				int nRgnType = pRgnCurrentProcs->GetRgnBox(&rBox);
				CPoint ptUpperLeft(rBox.left+1, rBox.top+1);
				//Now, move right until we're actually in the region.
				while(!pRgnCurrentProcs->PtInRegion(ptUpperLeft) && ptUpperLeft.x <= rBox.right) ptUpperLeft.x++;
				//OK, now we have the upper left.  Let's offset, say, five pixels.
				ptUpperLeft.x += 5;
				//How tall should our rect be?
				pDC->SelectObject(m_pIconFont);
				LOGFONT lf;
				m_pIconFont->GetLogFont(&lf);
				if(strCurrentProcNames == m_strCurrentHotProcedures) {
					lf.lfWeight = FW_BOLD;
					CFont fBold;
					fBold.CreateFontIndirect(&lf);
					pDC->SelectObject(fBold);
				}
				ptUpperLeft.y--;
				CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
				pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
				if(rText.right > rBox.right) rText.right = rBox.right;
				pDC->FillSolidRect(rText, RGB(255,255,255));
				pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_SINGLELINE);
				ProcNameRect *pnr = new ProcNameRect;
				pnr->rDrawn = rText;
				// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
				pnr->arynProcedureIDs.RemoveAll();
				pnr->arynProcedureIDs.Append(m_arImages.GetAt(i-1)->arynProcedureIDs);
				pnr->strProcedureNames = m_arImages.GetAt(i-1)->strProcedureNames;
				pnr->pRgn = new CRgn;
				m_parRgnsToDelete.Add((void*)pnr->pRgn);
				pnr->pRgn->CreateRectRgn(0,0,1,1);
				pnr->pRgn->CopyRgn(pRgnCurrentProcs);
				m_arProcNameRects.Add(pnr);

				if(nRgnType == COMPLEXREGION) {
					//This region wrapped a line, draw a second label on the next line.  Same algorithm as before, but moving
					//down instead of right.
					ptUpperLeft = CPoint(rBox.left+1, rBox.top+1);
					//Now, move down until we're actually in the region.
					while(!pRgnCurrentProcs->PtInRegion(ptUpperLeft) && ptUpperLeft.y <= rBox.bottom) ptUpperLeft.y++;
					//OK, now we have the upper left.  Let's offset, say, five pixels.
					ptUpperLeft.x += 5;
					//How tall should our rect be?
					pDC->SelectObject(m_pIconFont);
					LOGFONT lf;
					m_pIconFont->GetLogFont(&lf);
					if(strCurrentProcNames == m_strCurrentHotProcedures) {
						lf.lfWeight = FW_BOLD;
						CFont fBold;
						fBold.CreateFontIndirect(&lf);
						pDC->SelectObject(fBold);
					}
					CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
					pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
					if(rText.right > rBox.right) rText.right = rBox.right;
					pDC->FillSolidRect(rText, RGB(255,255,255));
					pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_SINGLELINE);
					ProcNameRect *pnr = new ProcNameRect;
					pnr->rDrawn = rText;
					// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
					pnr->arynProcedureIDs.RemoveAll();
					pnr->arynProcedureIDs.Append(m_arImages.GetAt(i-1)->arynProcedureIDs);
					pnr->strProcedureNames = m_arImages.GetAt(i-1)->strProcedureNames;
					pnr->pRgn = new CRgn;
					m_parRgnsToDelete.Add((void*)pnr->pRgn);
					pnr->pRgn->CreateRectRgn(0,0,1,1);
					pnr->pRgn->CopyRgn(pRgnCurrentProcs);
					m_arProcNameRects.Add(pnr);
				}
			}
			//Now, initialize our new region.
			pRgnCurrentProcs->SetRectRgn(rDrawn.left - BUFFER_HORIZ/2, rDrawn.top - BUFFER_VERT/2, rDrawn.right + BUFFER_HORIZ/2, rDrawn.bottom + BUFFER_VERT/2);
		}
		else {
			//It's the same procedure, so add to our current region.
			CRgn rgnNew;
			rgnNew.CreateRectRgn(rDrawn.left - BUFFER_HORIZ/2, rDrawn.top - BUFFER_VERT/2, rDrawn.right + BUFFER_HORIZ/2, rDrawn.bottom + BUFFER_VERT/2);
			pRgnCurrentProcs->CombineRgn(pRgnCurrentProcs, &rgnNew, RGN_OR);
			//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
			rgnNew.DeleteObject();
		}
		strCurrentProcNames = m_arImages.GetAt(i)->strProcedureNames;

		if(rDrawn.Height() > nImageHeight) nImageHeight = rDrawn.Height();
		m_arImageRects.SetAt(i, rDrawn);
		nCurrentPosX += IMAGE_SIZE;
	}
	
	//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we create.
	rgnClient.DeleteObject();

	// (a.walling 2008-09-22 16:35) - PLID 31479 - Now any visible images that are not loaded
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object
	if (m_AsyncImageLoader) {
		// (a.walling 2008-12-08 13:38) - PLID 32363 - Pass TRUE to ensure we initially lock the critical section
		CSingleLock sl(&m_AsyncImageLoader->GetLock(), TRUE);

		CList<LoadingImageInformationPtr,LoadingImageInformationPtr>& imagesLoading(m_AsyncImageLoader->GetImagesLoading());

		for (int ixPush = 0; ixPush < arVisibleImages.GetSize(); ixPush++) {
			// (a.walling 2010-12-20 16:05) - PLID 41917
			ImageInformationPtr ii = m_arImages[arVisibleImages[ixPush].nIndex];

			POSITION pos = imagesLoading.GetHeadPosition();
			while (pos) {
				POSITION posPrev = pos;
				// (a.walling 2010-12-20 16:05) - PLID 41917
				LoadingImageInformationPtr pLoadInfo = imagesLoading.GetNext(pos);

				if (pLoadInfo != NULL && pLoadInfo->ii == ii) {
					imagesLoading.RemoveAt(posPrev);
					imagesLoading.AddHead(pLoadInfo);
					break;
				}
			}
		}
	}

	if(!strCurrentProcNames.IsEmpty() && i > 0) {
		//Our lastregion is complete, and for a procedure.  Outline it.
		int nThickness = (strCurrentProcNames == m_strCurrentHotProcedures) ? 2 : 1;
		CBrush brBlack(RGB(0,0,0));
		bdc.m_pDC->FrameRgn(pRgnCurrentProcs, &brBlack, nThickness, nThickness);

		//Now, find the "upper left" of this region, to draw the name.
		CRect rBox;
		int nRgnType = pRgnCurrentProcs->GetRgnBox(&rBox);
		CPoint ptUpperLeft(rBox.left+1, rBox.top+1);
		//Now, move right until we're actually in the region.
		while(!pRgnCurrentProcs->PtInRegion(ptUpperLeft) && ptUpperLeft.x <= rBox.right) ptUpperLeft.x++;
		//OK, now we have the upper left.  Let's offset, say, five pixels.
		ptUpperLeft.x += 5;
		//How tall should our rect be?
		pDC->SelectObject(m_pIconFont);
		LOGFONT lf;
		m_pIconFont->GetLogFont(&lf);
		if(strCurrentProcNames == m_strCurrentHotProcedures) {
			lf.lfWeight = FW_BOLD;
			CFont fBold;
			fBold.CreateFontIndirect(&lf);
			pDC->SelectObject(fBold);
		}
		ptUpperLeft.y--;
		CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
		pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
		if(rText.right > rBox.right) rText.right = rBox.right;
		pDC->FillSolidRect(rText, RGB(255,255,255));
		pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_SINGLELINE);
		ProcNameRect *pnr = new ProcNameRect;
		pnr->rDrawn = rText;
		// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
		pnr->arynProcedureIDs.RemoveAll();
		pnr->arynProcedureIDs.Append(m_arImages.GetAt(i-1)->arynProcedureIDs);
		pnr->strProcedureNames = m_arImages.GetAt(i-1)->strProcedureNames;
		pnr->pRgn = new CRgn;
		m_parRgnsToDelete.Add((void*)pnr->pRgn);
		pnr->pRgn->CreateRectRgn(0,0,1,1);
		pnr->pRgn->CopyRgn(pRgnCurrentProcs);
		m_arProcNameRects.Add(pnr);

		if(nRgnType == COMPLEXREGION) {
			//This region wrapped a line, draw a second label on the next line.  Same algorithm as before, but moving
			//down instead of right.
			ptUpperLeft = CPoint(rBox.left+1, rBox.top+1);
			//Now, move down until we're actually in the region.
			while(!pRgnCurrentProcs->PtInRegion(ptUpperLeft) && ptUpperLeft.y <= rBox.bottom) ptUpperLeft.y++;
			//OK, now we have the upper left.  Let's offset, say, five pixels.
			ptUpperLeft.x += 5;
			//How tall should our rect be?
			pDC->SelectObject(m_pIconFont);
			LOGFONT lf;
			m_pIconFont->GetLogFont(&lf);
			if(strCurrentProcNames == m_strCurrentHotProcedures) {
				lf.lfWeight = FW_BOLD;
				CFont fBold;
				fBold.CreateFontIndirect(&lf);
				pDC->SelectObject(fBold);
			}
			CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
			pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
			if(rText.right > rBox.right) rText.right = rBox.right;
			pDC->FillSolidRect(rText, RGB(255,255,255));
			pDC->DrawText(m_arImages.GetAt(i-1)->strProcedureNames, &rText, DT_SINGLELINE);
			ProcNameRect *pnr = new ProcNameRect;
			pnr->rDrawn = rText;
			// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
			pnr->arynProcedureIDs.RemoveAll();
			pnr->arynProcedureIDs.Append(m_arImages.GetAt(i-1)->arynProcedureIDs);
			pnr->strProcedureNames = m_arImages.GetAt(i-1)->strProcedureNames;
			pnr->pRgn = new CRgn;
			m_parRgnsToDelete.Add((void*)pnr->pRgn);
			pnr->pRgn->CreateRectRgn(0,0,1,1);
			pnr->pRgn->CopyRgn(pRgnCurrentProcs);
			m_arProcNameRects.Add(pnr);
		}

	}

	//rgnCurrentProcs.DeleteObject();
}

void CPhotoViewerCtrl::Refresh(bool bForceSynchronous /*= false*/)
{
	// (b.cardillo 2010-03-11 11:05) - PLID 37705 - Remember what was selected before so we can re-select it when we're done
	m_arynImageIDsSelectedBeforeRefresh.RemoveAll();
	for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
		m_arynImageIDsSelectedBeforeRefresh.Add(m_listImagesSelected.GetAt(p)->nID);
	}
	//Just set the flag.
	m_bImagesLoaded = false;

	// (a.walling 2010-12-20 16:05) - PLID 41917 - Really we want to keep the thread running but get rid of any pending operations; we'll load them again shortly.
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object
	if (m_AsyncImageLoader) {
		m_AsyncImageLoader->ClearQueue();
		m_AsyncImageLoader->SetMirrorPatientImageMgr(NULL);
	}

	for(int i = 0; i < m_arImages.GetSize(); i++) {
		if(m_arImages.GetAt(i)->bIsImageValid) 
			DeleteObject(m_arImages.GetAt(i)->hFullImage);
		if(m_arImages.GetAt(i)->bIsThumbValid && m_arImages.GetAt(i)->hThumb != m_hLoadingThumb)
			DeleteObject(m_arImages.GetAt(i)->hThumb);
	}

	if(m_pToolTipWnd) {
		m_pToolTipWnd->ShowWindow(SW_HIDE);
	}

	//TES 7/27/2006 - PLID 21626 - Dismiss the temp edit window (for editing notes).
	if(m_pTempEdit && ::IsWindow(m_pTempEdit->GetSafeHwnd())) {
		PostMessage(WM_KEYDOWN, VK_ESCAPE);
	}

	KillTimer(HOVER_TICK);
	KillTimer(KILL_TIP_NCMOVE);
	KillTimer(EDIT_TIMER);
	m_nCurrentHotImage = -1;
	m_strCurrentHotProcedures = "";
	m_nScrollPos = 0;
	ClearImageArray();
	m_arImageRects.RemoveAll();
	ClearProcNameRects();
	for(i = 0; i < m_parRgnsToDelete.GetSize(); i++) {
		if(m_parRgnsToDelete.GetAt(i)) {
			//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
			((CRgn*)m_parRgnsToDelete.GetAt(i))->DeleteObject();
			delete (CRgn*)m_parRgnsToDelete.GetAt(i);
		}
	}
	m_parRgnsToDelete.RemoveAll();

	// (c.haag 2010-02-25 10:31) - PLID 37364 - Reset the Mirror Image Manager object because patient imaging
	// info may have changed since the last refresh.
	EnsureNotMirrorImageMgr();

	//TES 12/7/2007 - PLID 28285 - We just unselected all images, therefore there is now nothing available for detaching.
	GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)FALSE);
	if(bForceSynchronous) {
		LoadArray();
	}
	Invalidate();
}

void CPhotoViewerCtrl::LoadArray()
{
	//TES 3/17/04: Copied out of LoadAttachedPatientImage
	try {
		// if the person ID has not been set yet, then don't proceed any further. -1 is a valid PhotoViewerCtrl m_nPersonID value that is used for viewing
		// non-patient-specific images (like anatomic diagrams), so allow the array to be loaded for that value.
		if (m_nPersonID != -1 && m_nPersonID <= 0)
		{
			return;
		}
		// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
		m_bNeedsSort = true;

		// (z.manning, 06/07/2007) - PLID 23862 - If we didn't set an image path, use the default.
		if(m_strImagePath.IsEmpty()) {
			m_strImagePath = GetSharedPath() ^ "Images";
		}

		ClearImageArray();
		m_arImageRects.RemoveAll();
		ClearProcNameRects();
		for(int i = 0; i < m_parRgnsToDelete.GetSize(); i++) {
			if(m_parRgnsToDelete.GetAt(i)) {
				//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
				((CRgn*)m_parRgnsToDelete.GetAt(i))->DeleteObject();
				delete (CRgn*)m_parRgnsToDelete.GetAt(i);
			}
		}
		m_parRgnsToDelete.RemoveAll();

		if(!m_hLoadingThumb) {
			//Create our special "loading..." thumbnail.
			CBorrowDeviceContext bdc(this);
			if(!m_pIconFont) {
				//Let's load the system icon font.
				LOGFONT lf;
				if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
					m_pIconFont = new CFont;
					m_pIconFont->CreateFontIndirect(&lf);
				}
			}
			CDC dcMem;
			dcMem.CreateCompatibleDC(bdc.m_pDC);
			m_hLoadingThumb = CreateCompatibleBitmap(dcMem.GetSafeHdc(), IMAGE_SIZE, IMAGE_SIZE);
			dcMem.SelectObject(m_pIconFont);
			HBITMAP hBmpOld = (HBITMAP)dcMem.SelectObject(m_hLoadingThumb);
			dcMem.Rectangle(0,0,IMAGE_SIZE,IMAGE_SIZE);
			dcMem.DrawText("Loading...", CRect(5,40,95,60), DT_CENTER);
			dcMem.SelectObject(hBmpOld);
			dcMem.DeleteDC();
		}

		if(m_nPersonID == -1) {
			//This is a special value, use the Images folder in the shared directory.
			CFileFind ff;
			BOOL bSuccess = ff.FindFile(m_strImagePath ^ "*.*");
			int nID = 1;
			if (bSuccess)
			{
				while(ff.FindNextFile()) {
					if(!ff.IsDirectory() && !ff.IsDots()) {
						// (a.walling 2007-07-25 13:44) - PLID 17467 - Use shared function
						if(IsImageFile(ff.GetFileName())) {

							// (a.walling 2010-12-20 16:05) - PLID 41917
							ImageInformationPtr ii = boost::make_shared<ImageInformation>();
							ii->nType = eImageSrcDiagram;
							ii->nID = nID++;
							ii->strMailSentFilePath = ff.GetFilePath();
							ii->strFullFilePath = ff.GetFilePath();
							ii->strNotes = ff.GetFileName();
							ii->strToolText = ff.GetFileName();
							ii->hFullImage = NULL;
							ii->hThumb = m_hLoadingThumb;
							ii->bIsThumbValid = true;
							ii->arynProcedureIDs.RemoveAll();
							ii->strCategory = "";
							ii->strStaff = "";
							m_arImages.Add(ii);
							//These two arrays need to be the same size.
							m_arImageRects.Add(CRect(0,0,0,0));

							// (a.walling 2010-12-20 16:05) - PLID 41917
							LoadingImageInformationPtr pLoad = boost::make_shared<LoadingImageInformation>();
							pLoad->ii = ii;

							LoadImageAsync(pLoad);
						}
					}
				}
				//Because of the nature of CFileFind, we need to add one more.
				if(!ff.IsDirectory() && !ff.IsDots()) {
					// (a.walling 2007-07-25 13:44) - PLID 17467 - Use shared function
					if(IsImageFile(ff.GetFileName())) {

						// (a.walling 2010-12-20 16:05) - PLID 41917
						ImageInformationPtr ii = boost::make_shared<ImageInformation>();
						ii->nType = eImageSrcDiagram;
						ii->nID = nID++;
						ii->strMailSentFilePath = ff.GetFilePath();
						ii->strFullFilePath = ff.GetFilePath();
						ii->strNotes = ff.GetFileName();
						ii->strToolText = ff.GetFileName();
						ii->hFullImage = NULL;
						ii->hThumb = m_hLoadingThumb;
						ii->bIsThumbValid = true;
						ii->arynProcedureIDs.RemoveAll();
						ii->strCategory = "";
						ii->strStaff = "";
						m_arImages.Add(ii);
						//These two arrays need to be the same size.
						m_arImageRects.Add(CRect(0,0,0,0));

						// (a.walling 2010-12-20 16:05) - PLID 41917
						LoadingImageInformationPtr pLoad = boost::make_shared<LoadingImageInformation>();
						pLoad->ii = ii;

						LoadImageAsync(pLoad);
					}
				}
			}
		}

		else {
		
			//TES 4/1/2004: Load Mirror first, then United, then attached images.
			//Are we attached to Mirror?
			CString strMirrorID;
			long nUnitedID;
			if(m_bIsPatient) {
				// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
				_RecordsetPtr rsMirrorID = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT MirrorID, UnitedID FROM PatientsT WHERE PersonID = {INT}", m_nPersonID);
				strMirrorID = AdoFldString(rsMirrorID, "MirrorID", "");
				nUnitedID = AdoFldLong(rsMirrorID, "UnitedID", -1);
				rsMirrorID->Close();
			}
			else {
				strMirrorID = "";
				nUnitedID = -1;
			}
			
			if(m_bShowMirror && Mirror::IsMirrorEnabled()) {
				if(strMirrorID != "") {

					// (c.haag 2010-02-23 13:27) - PLID 37364 -Ensure the Mirror image manager exists.
					EnsureMirrorImageMgr();

					// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
					// (c.haag 2010-02-23 13:27) - PLID 37364 - Construct a Mirror patient image manager so
					// we can minimize traffic to the Canfield SDK. Use the manager instead of accessing the 
					// Mirror namespace directly.
					CMirrorPatientImageMgr mgrMirrorImages(strMirrorID);	
					long nMirrorCount = mgrMirrorImages.GetImageCount();
					for(int i = 0; i < nMirrorCount; i++) {
						// (a.walling 2010-12-20 16:05) - PLID 41917
						ImageInformationPtr ii = boost::make_shared<ImageInformation>();
						ii->nType = eImageSrcMirror;
						ii->nID = i;
						ii->strToolText.Format("Mirror Image %i", i+1);
						ii->strNotes.Format("Mirror Image %i", i+1);

						// (c.haag 2005-03-07 12:41) - PLID 15819 - With Mirror images, we will
						// use the filename field to store the Mirror ID. Think of the filename
						// as the primary key from the storage source where we get images from.
						// Perhaps in a future release, we could just replace strFullFilePath with
						// an image key object....but only if it's worth the hassle.
						ii->strMailSentFilePath.Format("%s-%d", strMirrorID, i); // Zero-based index
						ii->strFullFilePath = ii->strMailSentFilePath;
						
						ii->arynProcedureIDs.RemoveAll();
						ii->strProcedureNames = "";

						ii->strCategory = "";
						ii->strStaff = "";

						ii->hThumb = m_hLoadingThumb;
						ii->bIsThumbValid = true;

						m_arImages.Add(ii);
						//These two arrays need to be the same size.
						m_arImageRects.Add(CRect(0,0,0,0));

						// (a.walling 2010-12-20 16:05) - PLID 41917
						LoadingImageInformationPtr pLoad = boost::make_shared<LoadingImageInformation>();
						pLoad->ii = ii;
						pLoad->strMirrorID = strMirrorID;
						pLoad->nMirrorCount = nMirrorCount;

						LoadImageAsync(pLoad);
					}
				}
			}
			if(m_bShowUnited) {
				//Now United.
				if(nUnitedID != -1) {
					if (g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrUse) && g_userPermission[UnitedIntegration] != 0 && IsUnitedEnabled()) {
						CUnitedLink *pUnitedLink = GetMainFrame()->GetUnitedLink();
						if (pUnitedLink && pUnitedLink->GetRemotePath() != "") {
							long nUnitedCount = pUnitedLink->GetImageCount(nUnitedID);
							for(int i = 0; i < nUnitedCount; i++) {
								// (a.walling 2010-12-20 16:05) - PLID 41917
								ImageInformationPtr ii = boost::make_shared<ImageInformation>();
								ii->nType = eImageSrcUnited;
								ii->nID = i;
								ii->strToolText.Format("United Image %i", i+1);
								ii->strNotes.Format("United Image %i", i+1);
								
								ii->arynProcedureIDs.RemoveAll();
								ii->strProcedureNames = "";

								ii->strCategory = "";
								ii->strStaff = "";

								ii->hThumb = m_hLoadingThumb;
								ii->bIsThumbValid = true;

								m_arImages.Add(ii);
								//These two arrays need to be the same size.
								m_arImageRects.Add(CRect(0,0,0,0));

								// (a.walling 2010-12-20 16:05) - PLID 41917
								LoadingImageInformationPtr pLoad = boost::make_shared<LoadingImageInformation>();
								pLoad->ii = ii;
								pLoad->nUnitedID = nUnitedID;

								LoadImageAsync(pLoad);
							}
						}
					}
				}
			}
			_RecordsetPtr rsPhotos;
			CString strWhere = "";
			//If we got a PicID, filter on it.
			if(m_nPicID != -1) {
				strWhere.Format("AND MailSent.PicID = %li ", m_nPicID);
			}
			
			if(!m_strAdditionalFilter.IsEmpty()) {
				strWhere += "AND ";
				strWhere += m_strAdditionalFilter;
			}
			
			// (j.jones 2008-09-05 08:53) - PLID 30288 - supported MailSentNotesT
			// (j.jones 2009-10-12 16:08) - PLID 35894 - removed ProcedureID from this recordset
			//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
			// (a.walling 2012-02-22 09:01) - PLID 48321 - Create a base fragment for the source
			//(e.lally 2012-04-16) PLID 39543 - Added service date
			CSqlFragment loadSql = CSqlFragment("SELECT MailSent.MailID, PathName, Note, Date, ServiceDate, Sender, CategoryID "
				"FROM MailSent "
				"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
				"WHERE PersonID = {INT} AND dbo.IsImageFile(PathName) = 1 AND {SQLFRAGMENT} "
				"{CONST_STR} "
				, m_nPersonID, GetAllowedCategoryClause_Param("MailSent.CategoryID"), strWhere);

			// (a.walling 2012-02-22 09:01) - PLID 48321
			//(e.lally 2012-04-16) PLID 39543 - Added service date
			// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
			rsPhotos = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SubQ.MailID, SubQ.PathName, SubQ.Note, SubQ.Date, SubQ.ServiceDate, SubQ.Sender, NoteCatsF.Description AS CategoryName "
				"FROM ({SQL}) SubQ LEFT JOIN NoteCatsF ON SubQ.CategoryID = NoteCatsF.ID "
				"ORDER BY [Date] Desc", loadSql);
			
			// (j.jones 2009-10-13 08:57) - PLID 35894 - load the procedure info
			// (a.walling 2012-02-22 09:01) - PLID 48321 - Load it once rather than query one time for each image
			struct MailSentProcedureInfo
			{
				MailSentProcedureInfo()
				{
				}
				
				// (a.walling 2012-02-22 09:01) - PLID 48321 - MFC arrays have no copy constructor
				MailSentProcedureInfo(const MailSentProcedureInfo& r)
					: strProcedureNames(r.strProcedureNames)
				{
					arynProcedureIDs.Copy(r.arynProcedureIDs);
				}

				// (a.walling 2012-02-22 09:01) - PLID 48321 - MFC arrays have no assignment operator
				MailSentProcedureInfo& operator=(const MailSentProcedureInfo& r)
				{
					if (this != &r) 
					{
						strProcedureNames = r.strProcedureNames;
						arynProcedureIDs.Copy(r.arynProcedureIDs);
					}

					return *this;
				}

				CArray<long, long> arynProcedureIDs;
				CString strProcedureNames;
			};

			// (a.walling 2012-02-22 09:01) - PLID 48321 - Populate the map
			std::map<long, MailSentProcedureInfo> mapInfo;
			{
				// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
				_RecordsetPtr rsProcs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ProcedureT.ID, ProcedureT.Name, SubQ.MailID "
					"FROM ({SQL}) SubQ "
					"INNER JOIN MailSentProcedureT ON SubQ.MailID = MailSentProcedureT.MailSentID "
					"INNER JOIN ProcedureT ON MailSentProcedureT.ProcedureID = ProcedureT.ID "
					"ORDER BY SubQ.MailID, ProcedureT.Name", loadSql);

				while(!rsProcs->eof) {
					MailSentProcedureInfo& info = mapInfo[AdoFldLong(rsProcs, "MailID")];

					info.strProcedureNames.AppendFormat(
						info.strProcedureNames.IsEmpty() ? "%s" : ", %s"
						, AdoFldString(rsProcs, "Name")
					);

					//(e.lally 2012-04-17) PLID 49634 - This is needed for sort order
					info.arynProcedureIDs.Add(AdoFldLong(rsProcs, "ID"));

#ifdef _DEBUG
					MailSentProcedureInfo infoCopy = mapInfo[AdoFldLong(rsProcs, "MailID")];
					MailSentProcedureInfo& infoRef2 = mapInfo[AdoFldLong(rsProcs, "MailID")];

					ASSERT(info.strProcedureNames == infoCopy.strProcedureNames);
					ASSERT(infoRef2.strProcedureNames == infoCopy.strProcedureNames);
					ASSERT(-1 != info.strProcedureNames.Find(AdoFldString(rsProcs, "Name")));
					ASSERT(-1 != infoCopy.strProcedureNames.Find(AdoFldString(rsProcs, "Name")));
					ASSERT(-1 != infoRef2.strProcedureNames.Find(AdoFldString(rsProcs, "Name")));
					ASSERT(info.arynProcedureIDs.GetCount() == infoCopy.arynProcedureIDs.GetCount());
					ASSERT(info.arynProcedureIDs.GetCount() == infoRef2.arynProcedureIDs.GetCount());
					ASSERT(info.arynProcedureIDs.GetCount() > 0);
#endif

					rsProcs->MoveNext();
				}
				rsProcs->Close();
			}

			// (a.walling 2012-02-22 09:01) - PLID 48321 - This was also querying the DB
			CString strPatientDocumentPath = GetPatientDocumentPath(m_nPersonID);

			while(!rsPhotos->eof) {
				bool bReload = false;
				// (a.walling 2010-12-20 16:05) - PLID 41917
				ImageInformationPtr ii = boost::make_shared<ImageInformation>();
				ii->nType = eImageSrcPractice;
				CString strMailSentFilePath;
				CString strDoc;
				ii->nID = AdoFldLong(rsPhotos, "MailID");

				//Regardless, our thumbnail for now will be the 'loading..." one.
				ii->hThumb = m_hLoadingThumb;
					
				HBITMAP hImage = NULL;
				
				strMailSentFilePath = AdoFldString(rsPhotos, "PathName");

				// (j.jones 2013-09-19 14:19) - PLID 58547 - cleaned up these variables
				// so that strMailSentFilePath is exactly what is in MailSent (which is often,
				// but not always, just a file name only)
				// and strFullFilePath is always the full patch to the file
				
				// (a.walling 2007-01-30 09:28) - PLID 24475 - Get the full file path to add to the loading info
				CString strFullFilePath;
				CString strFileNameOnly = strMailSentFilePath.Mid(strMailSentFilePath.ReverseFind('\\')+1);
				// (a.walling 2012-02-22 09:01) - PLID 48321 - Use the strPatientDocumentPath we already loaded
				if(strFileNameOnly == strMailSentFilePath) {
					strFullFilePath = strPatientDocumentPath ^ strMailSentFilePath;
				}
				else {
					strFullFilePath = strMailSentFilePath;
				}

				//We're not loading the full image yet, we're not going to unless we have to.
				ii->bIsImageValid = false;
				ii->hFullImage = NULL;
				// (a.walling 2006-10-20 09:47) - PLID 22991 - Clear these just in case.
				ii->bIsThumbValid = false;
				ii->hThumb = NULL;

				ii->strCategory = AdoFldString(rsPhotos, "CategoryName", "");
				ii->strStaff = AdoFldString(rsPhotos, "Sender");

				ii->strMailSentFilePath = strMailSentFilePath;
				ii->strFullFilePath = strFullFilePath;
				ii->strNotes = AdoFldString(rsPhotos, "Note","");
				ii->dtAttached = AdoFldDateTime(rsPhotos, "Date");
				//(e.lally 2012-04-16) PLID 39543 - Added service date
				ii->dtService = AdoFldDateTime(rsPhotos, "ServiceDate", g_cdtInvalid);

				// (j.jones 2009-10-13 08:57) - PLID 35894 - load the procedure info
				// (a.walling 2012-02-22 09:01) - PLID 48321 - Load it once rather than query one time for each image
				ii->arynProcedureIDs.Copy(mapInfo[ii->nID].arynProcedureIDs);
				ii->strProcedureNames = mapInfo[ii->nID].strProcedureNames;
				ii->strProcedureNames.TrimRight(", ");

				//(e.lally 2012-04-16) PLID 39543 - Added service date
				CString strServiceDate;
				if(ii->dtService.m_dt > 0 && ii->dtService.m_status == COleDateTime::valid){
					strServiceDate = FormatDateTimeForInterface(ii->dtService, 0, dtoDate, true);
				}
				ii->strToolText = "Filename: " + strFullFilePath.Mid(strFullFilePath.ReverseFind('\\')+1) + "\r\n" +
					"Staff: " + ii->strStaff + "\r\n" + 
					"Attach Date: " + FormatDateTimeForInterface(ii->dtAttached, 0, dtoDate, true) + "\r\n" +
					"Service Date: " + strServiceDate + "\r\n" +
					"Procedure: " + ii->strProcedureNames + "\r\n" +
					"Note: " + ii->strNotes;

				m_arImages.Add(ii);
				//These two arrays need to be the same size.
				m_arImageRects.Add(CRect(0,0,0,0));

				// (a.walling 2010-12-20 16:05) - PLID 41917
				LoadingImageInformationPtr pLoad = boost::make_shared<LoadingImageInformation>();
				pLoad->ii = ii;

				LoadImageAsync(pLoad);
				rsPhotos->MoveNext();
			}
		}

		// (b.cardillo 2010-03-11 11:05) - PLID 37705 - Reselect whatever was selected before, if we can
		if (m_arynImageIDsSelectedBeforeRefresh.GetCount() > 0) {
			// (a.walling 2010-12-20 16:05) - PLID 41917
			CArray<ImageInformationPtr, ImageInformationPtr> aryImagesToSelect;
			bool bFoundAll = true;
			// Make sure ALL the previously selected IDs are still present.  If not, we won't reselect any.  And while 
			// we're looping here, we'll grab the image info pointers so we'll have them in an array in the correct order.
			for (int iToSel=0, nCountToSel=m_arynImageIDsSelectedBeforeRefresh.GetCount(); iToSel<nCountToSel; iToSel++) {
				const long nIDToSel = m_arynImageIDsSelectedBeforeRefresh.GetAt(iToSel);
				bool bFound = false;
				for (int iImg = 0, nCountImg = m_arImages.GetSize(); iImg < nCountImg; iImg++) {
					// (a.walling 2010-12-20 16:05) - PLID 41917
					ImageInformationPtr pimg = m_arImages.GetAt(iImg);
					if (pimg->nID == nIDToSel) {
						aryImagesToSelect.Add(pimg);
						bFound = true;
						break;
					}
				}
				if (!bFound) {
					bFoundAll = false;
					break;
				}
			}

			// If we found all the images, re-select them
			if (bFoundAll) {
				// Just loop through our array of images to select, and add them to the list of selected images.  Check if 
				// any are practice images so we can tell the detach/delete button how to behave.
				BOOL bPracticeSelected = FALSE;
				ASSERT(m_listImagesSelected.IsEmpty());
				for (int i=0, nCount=aryImagesToSelect.GetCount(); i<nCount; i++) {
					// (a.walling 2010-12-20 16:05) - PLID 41917
					ImageInformationPtr pimg = aryImagesToSelect.GetAt(i);
					m_listImagesSelected.AddTail(pimg);
					if (pimg->nType == eImageSrcPractice) {
						bPracticeSelected = TRUE;
					}
				}
				
				//Do we have any practice images selected?
				GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)bPracticeSelected);

				// Since we could be within an ongoing paint we need to invalidate the preview area before drawing 
				// it; and we don't need to draw it ourselves because if we are in a paint then we rely on the fact 
				// that the preview area is drawn after the images are loaded, and if we're not in a paint then our 
				// invalidate here will queue a paint to happen soon.
				InvalidateRect(GetDisplayArea(daPreview), FALSE);
			}
		}

		m_bImagesLoaded = true;

	} NxCatchAll("Error loading patient image");
}

void CPhotoViewerCtrl::SetPersonID(long nPersonID)
{
	if(nPersonID != m_nPersonID) {
		m_nPersonID = nPersonID;
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
		m_bIsPatient = ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", m_nPersonID);
		//We need to refresh.
		PostMessage(NXM_REFRESH_PHOTO_VIEWER);
	}
}

// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
void CPhotoViewerCtrl::SetPicContainer(class CPicContainerDlg* pPicContainer)
{
	if(m_pPicContainer != pPicContainer) {
		m_pPicContainer = pPicContainer;
		Refresh();
	}
}

void CPhotoViewerCtrl::SetPicID(long nPicID)
{
	if(nPicID != m_nPicID) {
		m_nPicID = nPicID;
		//We need to refresh.
		Refresh();
	}
}

void CPhotoViewerCtrl::SetAdditionalFilter(CString strAdditionalFilter)
{
	// (j.jones 2012-07-13 11:29) - PLID 49636 - All the code that calls this function
	// assumes that a refresh will be called, since this can be called as a result of
	// a tablechecker. So always refresh upon updating the filter, even if it didn't change.
	//if(m_strAdditionalFilter != strAdditionalFilter)
	
	//We need to refresh.
	m_strAdditionalFilter = strAdditionalFilter;	
	Refresh();
}

void CPhotoViewerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	//We want to process the KillFocus, or the LButtonDown, but not both.
	if(m_pTempEdit) {
		return;
	}

	//First off, if we're not loaded, fuggedahboutit.
	if(!m_bImagesLoaded) return;

	HitTestType htt;
	CArray<HBITMAP, HBITMAP> ahImagesToDelete; // (c.haag 2008-06-19 09:37) - PLID 28886
	// (j.jones 2009-10-13 09:53) - PLID 35894 - this function now also returns the procedure names
	CString strClickedProcedureNames;
	int nClicked = HitTest(point.x, point.y, htt, strClickedProcedureNames);
	switch(htt) {
	case httPreviewArea:
		//(e.lally 2012-04-30) PLID 49639
		PreviewSelectedPhotos((DisplayAreas) nClicked);		
		break;

	case httNull:
		if(!m_bControlDown) {
			//Unselect everything.
			UnselectAll();
			//Start dragging.
			//TES 9/7/2004 - PLID 13057 - Don't drag if they can't select multiple.
			if(m_bAllowMultiSelect) {
				m_nDragType = dtFocusRect;
				SetCapture();
				m_ptDragStart = point;
				CBorrowDeviceContext bdc(this);
				DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
				//We're not detachable any more.
				GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)FALSE);
			}
		}
	break;

	case httImage:
	case httImageNotes:
		{
			if(m_bControlDown) {
				//We need to put this image in our selected list, at the appropriate spot.
				if(IsImageSelected(nClicked)) {
					UnselectImage(nClicked);
				}
				if(m_arImages.GetAt(nClicked)->bIsThumbValid) {
					if(m_bAllowMultiSelect) {
						//Advance the list to the right index.
						int i = 0;
						for(POSITION p = m_listImagesSelected.GetHeadPosition(); i<m_nCtrlSelIndex && p; m_listImagesSelected.GetNext(p)) i++;
						m_listImagesSelected.InsertBefore(p, m_arImages.GetAt(nClicked));
						m_nCtrlSelIndex++;
					}
					else {
						UnselectAll();
						m_listImagesSelected.AddHead(m_arImages.GetAt(nClicked));
					}
				}
			}
			else {
				ImageInformationPtr pImage = m_arImages.GetAt(nClicked);
				bool bTryDrag = true;
				//toggle the image in question.
				if(IsImageSelected(nClicked)) {
					if(htt == httImageNotes && m_bCanEditNotes && pImage->nType == eImageSrcPractice 
						//TES 7/27/2006 - Only edit if exactly one image is selected.
						&& m_listImagesSelected.GetCount() == 1) {
						//Great, let's edit the notes.
						OnEditNotes();
						return;
					}
					else {
						UnselectImage(nClicked);
					}
				}
				else {
					if(m_arImages.GetAt(nClicked)->bIsThumbValid) {
						//It wasn't selected, select it.
						if(!m_bAllowMultiSelect) UnselectAll();
						m_listImagesSelected.AddTail(m_arImages.GetAt(nClicked));
						//If they clicked in the notes, and this is the only selected image.
						if(htt == httImageNotes && m_listImagesSelected.GetCount() == 1 && m_nPersonID != -1) {
							//They have, let's say, 2 seconds to edit the notes.
							m_bCanEditNotes = true;
							SetTimer(EDIT_TIMER, 2000, NULL);
							bTryDrag = false;
						}
					}
				}

				// (a.walling 2012-03-16 11:22) - PLID 48946 - Start a drag and drop operation if necessary
				//(e.lally 2012-05-01) PLID 48946 - Changed the criteria slighly allow multiselect mode to do drag and drops
				//	but only if there are 0/1 photos marked as "selected".
				// (a.walling 2012-06-06 13:22) - PLID 48946 - This is functional, which is enough for now. Created PLID 50840 to review and improve in the future.
				if (bTryDrag && m_listImagesSelected.GetCount() <= 1 && (pImage->nType == eImageSrcPractice || pImage->nType == eImageSrcDiagram) && !pImage->strFullFilePath.IsEmpty() && INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(pImage->strFullFilePath)) {
					CPoint ptScreen = point;
					ClientToScreen(&ptScreen);

					ptScreen.Offset(-12, -12);

					CRect dragRect(ptScreen, CSize(24, 24));

					COleDataSource dataSource;

					{
						CSharedFile glob(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE);

						DROPFILES dropFiles = {sizeof(DROPFILES)};

						glob.Write(&dropFiles, sizeof(DROPFILES));
						glob.Write((LPCTSTR)pImage->strFullFilePath, pImage->strFullFilePath.GetLength() + 1);

						FORMATETC etc = { 
							CF_HDROP,
							NULL,
							DVASPECT_CONTENT,
							-1,
							TYMED_HGLOBAL
						};

						dataSource.CacheGlobalData(CF_HDROP, glob.Detach(), &etc);
					}

					DROPEFFECT dropEffect = dataSource.DoDragDrop(DROPEFFECT_COPY, &dragRect, NULL);

					if (DROPEFFECT_NONE != dropEffect) {
						return;
					}
				} else if(m_arImages.GetAt(nClicked)->nType == eImageSrcPractice) {
					//Start dragging.
					m_nDragType = dtImage;
					SetCapture();
					m_nDraggedImage = nClicked;
				}
			}
			InvalidateImage(nClicked);
			//Do we have any practice images selected?
			BOOL bPracticeSelected = false;
			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p && !bPracticeSelected; m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) bPracticeSelected = true;
			}
			GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)bPracticeSelected);

		
			CBorrowDeviceContext bdc(this);
			DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
		}
		break;
	
	case httProcNames:
		{
			if(m_bAllowMultiSelect) {
				//If a single image is unselected, select all images in this procedure.
				bool bUnselectedFound = false;
				for(int i = 0; i < m_arImages.GetSize() && !bUnselectedFound; i++) {
					if(m_arImages.GetAt(i)->strProcedureNames == strClickedProcedureNames && !IsImageSelected(i)) {
						bUnselectedFound = true;
					}
				}
				if(bUnselectedFound) {
					for(int i = 0; i < m_arImages.GetSize(); i++) {
						if(m_arImages.GetAt(i)->strProcedureNames == strClickedProcedureNames && !IsImageSelected(i)) {
							m_listImagesSelected.AddTail(m_arImages.GetAt(i));
							//InvalidateImage(i);
						}
					}
				}
				else {
					//Unselect everything.
					for(int i = 0; i < m_arImages.GetSize(); i++) {
						if(m_arImages.GetAt(i)->strProcedureNames == strClickedProcedureNames && IsImageSelected(i)) {
							UnselectImage(i);
							//InvalidateImage(i);
						}
					}
				}
				//Do we have any practice images selected?
				BOOL bPracticeSelected = false;
				for(POSITION p = m_listImagesSelected.GetHeadPosition(); p && !bPracticeSelected; m_listImagesSelected.GetNext(p)) {
					if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) bPracticeSelected = true;
				}
				GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)bPracticeSelected);

				InvalidateProcedure(strClickedProcedureNames);
				CBorrowDeviceContext bdc(this);
				DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
			}
		}
		break;
	}

	CWnd::OnLButtonDown(nFlags, point);
}

//Returns the index in m_arImages of the given point.
// (j.jones 2009-10-13 09:53) - PLID 35894 - this function now also returns the procedure names
int CPhotoViewerCtrl::HitTest(int x, int y, HitTestType OUT &nType, CString OUT &strProcedureNames)
{
	if(GetDisplayArea(daMainList).PtInRect(CPoint(x,y))) {
		//Check the procedure names first.
		for(int i = 0; i < m_arProcNameRects.GetSize(); i++) {
			if(m_arProcNameRects.GetAt(i)->rDrawn.PtInRect(CPoint(x,y))) {
				nType = httProcNames;
				strProcedureNames = m_arProcNameRects.GetAt(i)->strProcedureNames;
				return i;
			}
		}
		for(i = 0; i < m_arImageRects.GetSize(); i++) {
			CRect rTotal = m_arImageRects.GetAt(i);
			if(rTotal.PtInRect(CPoint(x,y))) {
				rTotal.DeflateRect(0,IMAGE_SIZE,0,0);
				nType = rTotal.PtInRect(CPoint(x,y)) ? httImageNotes : httImage;
				return i;
			}
		}
	}
	else {
		if(GetDisplayArea(daPreview1).PtInRect(CPoint(x,y))) {nType = httPreviewArea; return daPreview1;}
		else if(GetDisplayArea(daPreviewTopBottom).PtInRect(CPoint(x,y))) {nType = httPreviewArea; return daPreviewTopBottom;}
		else if(GetDisplayArea(daPreviewSideSide).PtInRect(CPoint(x,y))) {nType = httPreviewArea; return daPreviewSideSide;}
		else if(GetDisplayArea(daPreview4).PtInRect(CPoint(x,y))) {nType = httPreviewArea; return daPreview4;}
	}
	nType = httNull;
	return -1;
}
void CPhotoViewerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{	
	CBorrowDeviceContext bdc(this);
	//First off, if we're not loaded, fuggedahboutit.
	if(!m_bImagesLoaded) return;
	if(m_nDragType == dtFocusRect) {
		//Forget all this other stuff, let's update our drag rect.
		CRect rDrag(m_ptDragStart.x, m_ptDragStart.y, point.x, point.y);
		rDrag.NormalizeRect();
		//Now, only show the area that's in our main area. (You didn't know CRect had an &= operator, did you?)
		rDrag &= GetDisplayArea(daMainList);
		CBorrowDeviceContext bdc(this);
		CDC *pDC = bdc.m_pDC;
		
		//Calling this function twice in succession for the same rect erases the rect
		if(m_bDragStarted) {
			pDC->DrawFocusRect(m_rLastDragRect);
		}
		pDC->DrawFocusRect(rDrag);
		m_rLastDragRect = rDrag;
		//At this point, we've definitely actually dragged.
		if(!m_bDragStarted) {
			m_bDragStarted = true;
			
		}	
		//Update our selected list
		bool bSelectedChanged = false;
		for(int i = 0; i < m_arImageRects.GetSize(); i++) {
			CRect rImage = m_arImageRects.GetAt(i);
			rImage.InflateRect(BUFFER_HORIZ/2,BUFFER_VERT/2);
			if((rImage & m_rLastDragRect).IsRectEmpty()) {
				//Unselect this image.
				if(IsImageSelected(i)) {
					bSelectedChanged = true;
					UnselectImage(i);
					InvalidateImage(i);
				}
			}
			else {
				//Select this image.
				if(!IsImageSelected(i) && m_arImages.GetAt(i)->bIsThumbValid) {
					bSelectedChanged = true;
					if(m_bAllowMultiSelect || m_listImagesSelected.IsEmpty()) {
						m_listImagesSelected.AddTail(m_arImages.GetAt(i));
						InvalidateImage(i);
					}
				}
				
			}
		}
		if(bSelectedChanged) {
			//Do we have any practice images selected?
			BOOL bPracticeSelected = false;
			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p && !bPracticeSelected; m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) bPracticeSelected = true;
			}
			GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)bPracticeSelected);
			
			DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
		}
		
	}
	else {
		if(m_nDragType == dtImage) {
			CBorrowDeviceContext bdc(this);
			//Forget all this other stuff, let's just show our ghostly image.
			m_bDragStarted = true;
			CRect rClient;
			GetClientRect(rClient);
			if(rClient.PtInRect(point)) {
				m_rLastDragRect = DrawGhostlyImage(bdc.m_pDC, m_arImages.GetAt(m_nDraggedImage)->hThumb, point);
			}
		}
		//Now, we have a hot procedure if a.) we are dragging an image, and over an image with a procedure, or 
		//b.) we are over a procedure name, dragging or not.
		bool bPreviewEnum = false;
		bool bInNotes = false;
		HitTestType httType;
		// (j.jones 2009-10-13 09:53) - PLID 35894 - this function now also returns the procedure names
		CString strHotProcedureNames;
		int nHotItem = HitTest(point.x, point.y, httType, strHotProcedureNames);
		if(httType == httPreviewArea) {
			if(m_nHotPreviewArea != nHotItem) {
				m_nHotPreviewArea = nHotItem;
				DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
			}
		}
		else if(httType == httImage || httType == httImageNotes) {
			//All images with this procedure are now "hot."
			// (j.jones 2009-10-13 09:34) - PLID 35894 - converted to compare by procedure names
			if(m_nDragType == dtImage && !m_arImages.GetAt(nHotItem)->strProcedureNames.IsEmpty()) {
				CString strProcedureNames = m_arImages.GetAt(nHotItem)->strProcedureNames;
				if(strProcedureNames != m_strCurrentHotProcedures) {
					if(m_pToolTipWnd) {
						m_pToolTipWnd->ShowWindow(SW_HIDE);
					}
					//No invalidating if they're editing notes.
					if(!m_pTempEdit) {
						if(!m_strCurrentHotProcedures.IsEmpty()) {
							CString strPreviousHot = m_strCurrentHotProcedures;
							m_strCurrentHotProcedures = strProcedureNames;
							InvalidateProcedure(strPreviousHot);
							InvalidateProcedure(m_strCurrentHotProcedures);
						}
						else {
							m_strCurrentHotProcedures = strProcedureNames;
							InvalidateProcedure(m_strCurrentHotProcedures);
						}
					}
				}

			}
			else {
				if(m_nHotPreviewArea != -1) {
					m_nHotPreviewArea = -1;
					DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
				}
				if(!m_strCurrentHotProcedures.IsEmpty()) {
					CString strOldHot = m_strCurrentHotProcedures;
					m_strCurrentHotProcedures = "";
					InvalidateProcedure(strOldHot);
				}
				//If the mouse is over a different item, record that, and invalidate both the old and new item.
				if(nHotItem != m_nCurrentHotImage) {
					//Kill the tip.
					if(m_pToolTipWnd) {
						m_pToolTipWnd->ShowWindow(SW_HIDE);
					}
					//No invalidating if they're editing notes.
					if(!m_pTempEdit) {
						if(m_nCurrentHotImage != -1) {
							int nPreviousHot = m_nCurrentHotImage;
							m_nCurrentHotImage = nHotItem;
							InvalidateImage(nPreviousHot);
							if(m_nCurrentHotImage != -1) {
								InvalidateImage(m_nCurrentHotImage);
							}
						}
						else {
							m_nCurrentHotImage = nHotItem;
							if(m_nCurrentHotImage != -1) {
								InvalidateImage(m_nCurrentHotImage);
							}
						}
					}
				}
			}
		}
		else if(httType == httProcNames) {
			//All images with this procedure are now "hot."
			//TES 6/15/2004: Don't highlight all of them if they're not allowed to select all of them.
			if(m_bAllowMultiSelect) {
				// (j.jones 2009-10-13 10:20) - PLID 35894 - supported multiple procedures
				if(strHotProcedureNames != m_strCurrentHotProcedures) {
					if(m_pToolTipWnd) {
						m_pToolTipWnd->ShowWindow(SW_HIDE);
					}
					//No invalidating if they're editing notes.
					if(!m_pTempEdit) {
						if(!m_strCurrentHotProcedures.IsEmpty()) {
							CString strPreviousHot = strHotProcedureNames;
							m_strCurrentHotProcedures = strHotProcedureNames;
							InvalidateProcedure(strPreviousHot);
							InvalidateProcedure(m_strCurrentHotProcedures);
						}
						else {
							m_strCurrentHotProcedures = strHotProcedureNames;
							InvalidateProcedure(m_strCurrentHotProcedures);
						}
					}
				}
			}
		}
		else if(httType == httNull) {
			if(m_nCurrentHotImage != -1) {
				//No invalidating if they're editing notes.
				if(!m_pTempEdit) {
					int nOldHot = m_nCurrentHotImage;
					m_nCurrentHotImage = -1;
					InvalidateImage(nOldHot);
				}
			}
			if(!m_strCurrentHotProcedures.IsEmpty()) {
				//No invalidating if they're editing notes.
				if(!m_pTempEdit) {
					// (j.jones 2009-10-13 10:20) - PLID 35894 - supported multiple procedures
					CString strOldHot = m_strCurrentHotProcedures;
					m_strCurrentHotProcedures = "";
					InvalidateProcedure(strOldHot);
				}
			}
		}


		//Now, if the tip isn't already showing, restart the timer.
		//Note: don't show the tip while we're editing notes.
		if((!m_pToolTipWnd || !m_pToolTipWnd->IsWindowVisible()) && !m_pTempEdit) {
			//Start the tooltip timer.
			KillTimer(HOVER_TICK);
			SetTimer(HOVER_TICK, 500, NULL);
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CPhotoViewerCtrl::OnTimer(UINT nIDEvent) 
{
	try {
		switch(nIDEvent) {
		case HOVER_TICK:
			{
				//Don't show the tip if the mouse button down.
				if(GetAsyncKeyState(VK_LBUTTON) & 0xE000 ||	GetAsyncKeyState(VK_RBUTTON) & 0xE000) {				
					if(m_pToolTipWnd) {
						m_pToolTipWnd->ShowWindow(SW_HIDE);
					}
				}
				else {
					//Is the mouse still over us?
					if(m_nCurrentHotImage == -1 || m_nCurrentHotImage >= m_arImageRects.GetSize()) {
						//????  Let's hide the tool tip.
						if(m_pToolTipWnd) {
							m_pToolTipWnd->ShowWindow(SW_HIDE);
						}
					}
					else {
						POINT point;
						GetCursorPos(&point);
						ScreenToClient(&point);
						if(m_arImageRects.GetAt(m_nCurrentHotImage).PtInRect(point)) {
							if(!m_pToolTipWnd) {
								// (a.walling 2010-06-23 17:06) - PLID 39330 - Call CNxToolTip's static helper classes to create this
								m_pToolTipWnd = CNxToolTip::CreateNxToolTip(this);
							}
							CString strTip = m_arImages.GetAt(m_nCurrentHotImage)->strToolText;
							if(!m_pToolTipWnd->IsWindowVisible() || m_pToolTipWnd->m_strTip != strTip) {

								// (c.haag 2004-12-28 11:22) - Trim off whitespace related
								// characters. I would use CString::TrimRight for this but
								// it hasn't worked from my experience.
								while (strTip.GetLength())
								{
									char c = strTip[ strTip.GetLength() - 1 ];
									if (c == '\r' || c == '\n' || c == '\t')
									{
										strTip = strTip.Left( strTip.GetLength() - 1 );
									}
									else {
										break;
									}
								}

								// (c.haag 2003-10-01 09:09) - If there is no tip text, our
								// goal is to hide the tooltip.
								if (strTip.IsEmpty())
								{
									// If it's visible, make it invisible.
									if(m_pToolTipWnd->IsWindowVisible()) {								
										m_pToolTipWnd->ShowWindow(SW_HIDE);
									}
									// If it's invisible, don't do anything since that's
									// what we want.

									// Now empty the tool tip text because we should always leave
									// this case statement having m_strTip equal to GetTextForTip().
									m_pToolTipWnd->m_strTip.Empty();
								}
								else
								{
									//Hide it while we redraw everything.
									m_pToolTipWnd->ShowWindow(SW_HIDE);
									m_pToolTipWnd->m_strTip = strTip;
									//This will tell it to set its position and everything.
									m_pToolTipWnd->PrepareWindow();
									//Now we can show it again.
									m_pToolTipWnd->ShowWindow(SW_SHOWNOACTIVATE);
									//Start the timers to dismiss the tooltip.
									SetTimer(KILL_TIP_NCMOVE, 100, NULL); //If they move the mouse out of the window.
								}
							}
						}
						else {
							if(m_pToolTipWnd) {
								m_pToolTipWnd->ShowWindow(SW_HIDE);
								KillTimer(KILL_TIP_NCMOVE);
							}
						}
					}
					KillTimer(HOVER_TICK);
				}
			}
			break;

		case KILL_TIP_NCMOVE:
			{
				//Is the mouse still over us?
				if(m_nCurrentHotImage == -1) {
					//????  Let's hide the tool tip.
					if(m_pToolTipWnd) {
						m_pToolTipWnd->ShowWindow(SW_HIDE);
					}
					KillTimer(KILL_TIP_NCMOVE);
				}
				else {

					POINT point;
					GetCursorPos(&point);
					ScreenToClient(&point);
					if(!m_arImageRects.GetAt(m_nCurrentHotImage).PtInRect(point)) {
						CRect rTip;
						if(m_pToolTipWnd) {
							m_pToolTipWnd->GetWindowRect(rTip);
							ClientToScreen(&point);
							if(!rTip.PtInRect(point)) {
								m_pToolTipWnd->ShowWindow(SW_HIDE);
								KillTimer(KILL_TIP_NCMOVE);
							}
						}
						else {
							KillTimer(KILL_TIP_NCMOVE);
						}
					}
				}
			}
			break;
		case EDIT_TIMER:
			m_bCanEditNotes = false;
			break;
		}
		
	}NxCatchAll("Error in CPhotoViewerCtrl::OnTimer()");
	
	CWnd::OnTimer(nIDEvent);
}

// (a.walling 2008-09-22 16:32) - PLID 31479 - Also returns the height (px) of the image drawn
// (a.walling 2008-09-24 09:15) - PLID 31479 - Option to set clipping region or not
CRect CPhotoViewerCtrl::DrawImage(CDC *pDC, int nIndex, int nLeft, int nTop, const CRect& rcClient, BOOL bSetClip, long* pnDrawnAmount)
{
	// (a.walling 2010-12-20 16:05) - PLID 41917
	ImageInformationPtr ii = (ImageInformationPtr)m_arImages.GetAt(nIndex);
	//Make sure we don't draw off the edge of the window.

	// (a.walling 2008-09-24 09:16) - PLID 31479 - Set clip if instructed to do so
	if (bSetClip) {
		//Don't draw over our border.
		CRgn rgnClient;
		rgnClient.CreateRectRgn(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
		pDC->SelectClipRgn(&rgnClient);
		//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we create.
		rgnClient.DeleteObject();
	}

	CRect rImage(nLeft, nTop, nLeft+IMAGE_SIZE, nTop+IMAGE_SIZE);

	//Now, put the filename below.
	CRect rText(rImage.left, rImage.bottom, rImage.right, rImage.bottom);
	// (a.walling 2008-09-24 09:14) - PLID 31479 - One of our slower operations was calculating
	// the text height for every image every single time we need to paint.

	//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
	//(e.lally 2012-04-24) PLID 49637 - Check if we are flagged to show or hide the labels
	CString strTextToDraw = "";
	if(m_bDrawTextLabels){
		if(m_nPersonID == -1 ){
			//(e.lally 2012-04-25) PLID 49637 - Non-patient photo. Use the filename
			strTextToDraw = ii->strFullFilePath.Mid(ii->strFullFilePath.ReverseFind('\\')+1);
		}
		else {
			strTextToDraw = ii->GetTextToDraw();
		}
	}
	if (!strTextToDraw.IsEmpty() && ii->nNotesHeight == -1) {
		
		CFont* pOldFont = pDC->SelectObject(m_pIconFont);
		//First, just expand our rect to be tall enough.
		CRect rTmp = rText;
		//(e.lally 2012-04-13) PLID 49637 - Use strTextToDraw here
		pDC->DrawText(strTextToDraw, rTmp, DT_CENTER|DT_WORDBREAK|DT_CALCRECT|DT_WORD_ELLIPSIS);
		rText.bottom = rText.top + rTmp.Height();

		ii->nNotesHeight = rTmp.Height();

		pDC->SelectObject(pOldFont);
	} else {
		//(e.lally 2012-04-24) PLID 49637 - Check if we are flagged to show or hide the labels
		if (m_bDrawTextLabels && ii->nNotesHeight > 0) {
			rText.bottom = rText.top + ii->nNotesHeight;
		}
	}

	//Now, actually draw it.
	//Actually, first draw our rectangle, because the text, if we're the "hot" item, may go on top of the rectangle.
	CRect rTotalRect;
	rTotalRect.UnionRect(rImage, rText);

		
	// (a.walling 2006-08-04 17:15) - PLID 21794 - Only draw it if it is visible,
	//			or if it is a loading thumb (so the window sizes correctly)
	// (a.walling 2008-09-22 16:33) - PLID 31479 - Get the entire drawing area
	CRect rcVisibleArea(0, 0, rcClient.Width(), rcClient.Height());
	CRect rcAdjImageRect = rTotalRect;
	rcAdjImageRect.InflateRect(3, 3);

	// (a.walling 2008-09-22 16:33) - PLID 31479 - Only need to actually draw anything if the area is visible
	CRect rcIntersect;
	BOOL bVisible = rcIntersect.IntersectRect(rcVisibleArea, rcAdjImageRect);

	if (bVisible) {
		// (a.walling 2008-09-24 09:18) - PLID 31479 - Only need to "borrow" the DC (meaning restore the attribs) if we are actually drawing
		CBorrowDeviceContext bdc(pDC);

		pDC->SelectObject(m_pIconFont);
		//TRACE("Actually drawing image %li (%li, %li)\n", nIndex, nLeft, nTop);
		DrawDIBitmapInRect(pDC, rImage, ii->hThumb);

		// (a.walling 2008-09-22 16:33) - PLID 31479 - Return the amount of the actual image drawn
		if (pnDrawnAmount) {
			CRect rcIntersectImage;
			if (rcIntersectImage.IntersectRect(rcVisibleArea, rImage)) {
				*pnDrawnAmount = rcIntersectImage.Height();
			} else {
				*pnDrawnAmount = 0;
			}
		}
		
		bool bIsHot = false;

		if(m_nDragType == dtNull && (nIndex == m_nCurrentHotImage || (!m_arImages.GetAt(nIndex)->strProcedureNames.IsEmpty() && m_arImages.GetAt(nIndex)->strProcedureNames == m_strCurrentHotProcedures)))
			bIsHot = true;

		//OK, we'll definitely be drawing some sort of rectangle.  What will it look like?
		CPen pHighlight;
		if(IsImageSelected(nIndex)) {
			if(bIsHot) {
				//Thick, blue
				pHighlight.CreatePen(PS_SOLID|PS_INSIDEFRAME, 2, RGB(0,0,255));
			}
			else {
				//Thin, blue
				pHighlight.CreatePen(PS_SOLID|PS_INSIDEFRAME, 1, RGB(0,0,255));
			}
		}
		else {
			if(bIsHot) {
				//thin, and light orange.
				pHighlight.CreatePen(PS_SOLID|PS_INSIDEFRAME, 1, RGB(255,200,127));
			}
			else {
				//No rectangle.
				pHighlight.CreatePen(PS_NULL|PS_INSIDEFRAME, 1, RGB(255,255,255));
			}

		}
		//Draw a highlight rectangle.
		rTotalRect.InflateRect(3,3);
		pDC->SelectObject(&pHighlight);
		pDC->SelectStockObject(HOLLOW_BRUSH);
		pDC->Rectangle(rTotalRect);
		
		//Now we can draw the text.
		if(m_arImages.GetAt(nIndex)->nID == m_nPrimaryImageID && m_arImages.GetAt(nIndex)->nType == eImageSrcPractice) {
			pDC->SetTextColor(RGB(0,0,255));
		}
		//(e.lally 2012-04-13) PLID 49637 - Use strTextToDraw here
		pDC->DrawText(strTextToDraw, rText, DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS|DT_WORD_ELLIPSIS);
		rTotalRect.DeflateRect(3,3);
	}
	return rTotalRect;
}

void CPhotoViewerCtrl::InvalidateImage(int nIndex)
{
	if(!IsWindowVisible() || nIndex >= m_arImageRects.GetSize()) return;

	//OK, here's the thing.  The only area that will potentially change is the area in the buffer zone.
	//So, let's white that area out.
	CBorrowDeviceContext bdc(this);
	CDC *pDC = bdc.m_pDC;
	CRgn rgnTotal, rgnImage, rgnBuffer, rgnClient;
	CRect rImage = m_arImageRects.GetAt(nIndex);
	//Get the big rect.
	CRect rBuffer = rImage;
	rBuffer.InflateRect(BUFFER_HORIZ/2,BUFFER_VERT/2);
	rBuffer.DeflateRect(1,1);//Now, don't draw over the procedure outline.
	rgnTotal.CreateRectRgn(rBuffer.left, rBuffer.top, rBuffer.right, rBuffer.bottom);
	//Now, the small rect.
	rgnImage.CreateRectRgn(rImage.left, rImage.top, rImage.right, rImage.bottom);
	//Finally, the client rect (we don't want to draw on somebody else's stuff.
	CRect rClient;
	GetClientRect(rClient);
	//Don't draw over our border.
	rClient.DeflateRect(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));
	rgnClient.CreateRectRgn(rClient.left, rClient.top, rClient.right, rClient.bottom);

	//Now, create an arbitrary region in rgnBuffer, because CRgn is retarded and forces you to.
	rgnBuffer.CreateRectRgn(0,0,1,1);
	//And subtract the big from the small.
	rgnBuffer.CombineRgn(&rgnTotal, &rgnImage, RGN_DIFF);
	//And make sure we're in the client rgn.
	rgnBuffer.CombineRgn(&rgnBuffer, &rgnClient, RGN_AND);

	//OK, now we have the region, paint it white.
	CBrush brWhite(RGB(255,255,255));
	pDC->FillRgn(&rgnBuffer, &brWhite);

	//Now, go ahead and redraw.
	DrawImage(pDC, nIndex, rImage.left, rImage.top, rClient);

	//Now, any procedure names that were in our image, redraw.
	for(int i = 0; i < m_arProcNameRects.GetSize(); i++) {
		if(rgnBuffer.RectInRegion(&(m_arProcNameRects.GetAt(i)->rDrawn))) {
			pDC->SelectObject(m_pIconFont);
			pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, m_arProcNameRects.GetAt(i)->rDrawn, DT_SINGLELINE);
		}
	}

	//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
	rgnTotal.DeleteObject();
	rgnImage.DeleteObject();
	rgnClient.DeleteObject();
	rgnBuffer.DeleteObject();
}

bool CPhotoViewerCtrl::IsImageSelected(int nIndex)
{
	long nID = m_arImages.GetAt(nIndex)->nID;
	EImageSource nType = m_arImages.GetAt(nIndex)->nType;
	for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
		if(m_listImagesSelected.GetAt(p)->nID == nID && nType == m_listImagesSelected.GetAt(p)->nType) return true;
	}
	return false;
}

void CPhotoViewerCtrl::DrawPreviewArea(CDC *pDC, CRect rInvalid)
{
	if(!m_bShowPreviewArea) return;	

	CBorrowDeviceContext bdc(pDC);
	CPen pThin(PS_SOLID, 1, RGB(0,0,0));
	pDC->SelectObject(&pThin);
	pDC->SelectStockObject(NULL_BRUSH);
	
	//First, the one image area.
	CRect r1Image = GetDisplayArea(daPreview1);
	if(r1Image & rInvalid) {
		//Draw a thin outline.
		pDC->Rectangle(r1Image);
		//Now, put a smaller rectangle for the actual image.
		//Let's make it half the width, and three-quarters the height
		FillPreviewRect(pDC, CRect(r1Image.left+r1Image.Width()/4, r1Image.top+r1Image.Height()/8, 
			r1Image.right-r1Image.Width()/4,r1Image.bottom-r1Image.bottom/8), 0,m_nHotPreviewArea==daPreview1);
	}

	//Now, the one on top of the other area.
	CRect rTopBottom = GetDisplayArea(daPreviewTopBottom);
	if(rTopBottom & rInvalid) {
		//Draw a thin outline.
		pDC->Rectangle(rTopBottom);
		//Fill in the two images.  Each will be three-quarters the width, and 2/7 the height.
		FillPreviewRect(pDC, CRect(rTopBottom.left+rTopBottom.Width()/8, rTopBottom.top+rTopBottom.Height()/7,
			rTopBottom.right-rTopBottom.Width()/8,rTopBottom.top+3*rTopBottom.Height()/7),0,m_nHotPreviewArea==daPreviewTopBottom);
		FillPreviewRect(pDC, CRect(rTopBottom.left+rTopBottom.Width()/8, rTopBottom.top+4*rTopBottom.Height()/7,
			rTopBottom.right-rTopBottom.Width()/8,rTopBottom.top+6*rTopBottom.Height()/7),1,m_nHotPreviewArea==daPreviewTopBottom);
	}
	
	//Now, the side by side area.
	CRect rSideSide = GetDisplayArea(daPreviewSideSide);
	if(rSideSide & rInvalid) {
		//Draw a thin outline.
		pDC->Rectangle(rSideSide);
		//Fill in the two images.  Each will be 1/3 the width, and 3/4 the height.
		FillPreviewRect(pDC, CRect(rSideSide.left+rSideSide.Width()/9, rSideSide.top+rSideSide.Height()/8,
			rSideSide.left+4*rSideSide.Width()/9,rSideSide.bottom-rSideSide.Height()/8),0,m_nHotPreviewArea==daPreviewSideSide);
		FillPreviewRect(pDC, CRect(rSideSide.left+5*rSideSide.Width()/9, rSideSide.top+rSideSide.Height()/8,
			rSideSide.left+8*rSideSide.Width()/9,rSideSide.bottom-rSideSide.Height()/8),1,m_nHotPreviewArea==daPreviewSideSide);
	}
	
	//Now, the four image area.
	CRect r4Image = GetDisplayArea(daPreview4);
	if(r4Image & rInvalid) {
		//Draw a thin outline.
		pDC->Rectangle(r4Image);
		//Fill in the four images.  Each will be 1/3 the width and 1/3 the height.
		FillPreviewRect(pDC, CRect(r4Image.left+r4Image.Width()/9, r4Image.top+r4Image.Width()/9,
			r4Image.left+4*r4Image.Width()/9,r4Image.top+4*r4Image.Height()/9),0,m_nHotPreviewArea==daPreview4);
		FillPreviewRect(pDC, CRect(r4Image.left+5*r4Image.Width()/9, r4Image.top+r4Image.Width()/9,
			r4Image.left+8*r4Image.Width()/9,r4Image.top+4*r4Image.Height()/9),1,m_nHotPreviewArea==daPreview4);
		FillPreviewRect(pDC, CRect(r4Image.left+r4Image.Width()/9, r4Image.top+5*r4Image.Width()/9,
			r4Image.left+4*r4Image.Width()/9,r4Image.top+8*r4Image.Height()/9),2,m_nHotPreviewArea==daPreview4);
		FillPreviewRect(pDC, CRect(r4Image.left+5*r4Image.Width()/9, r4Image.top+5*r4Image.Width()/9,
			r4Image.left+8*r4Image.Width()/9,r4Image.top+8*r4Image.Height()/9),3,m_nHotPreviewArea==daPreview4);
	}

}

CRect CPhotoViewerCtrl::GetDisplayArea(DisplayAreas da)
{
	//General description: The preview area is 4 squares down the right-hand side. Thus, each square's width and height is equal
	//to the client area's height / 4.  The chart area is of an equal width, the full height of the screen, just to the right.
	CRect rClient;
	GetClientRect(rClient);
	int nSquareDimension = rClient.Height() / 4;
	switch(da) {
	case daMainList:
		if(m_bShowPreviewArea) {
			return CRect(rClient.left, rClient.top, rClient.right - nSquareDimension, rClient.bottom);
		}
		else {
			return rClient;
		}
		break;
	case daPreview:
		if(m_bShowPreviewArea) {
			return CRect(rClient.right-nSquareDimension, rClient.top, rClient.right, rClient.bottom);
		}
		else {
			return CRect(0,0,0,0);
		}
		break;
	case daPreview1:
		if(m_bShowPreviewArea) {
			return CRect(rClient.right-nSquareDimension, rClient.top, rClient.right, rClient.top + nSquareDimension);					
		}
		else {
			return CRect(0,0,0,0);
		}
		break;
	case daPreviewSideSide:
		if(m_bShowPreviewArea) {
			return CRect(rClient.right-nSquareDimension, rClient.top + nSquareDimension, rClient.right, rClient.top + 2*nSquareDimension);
		}
		else {
			return CRect(0,0,0,0);
		}
		break;
	case daPreviewTopBottom:
		if(m_bShowPreviewArea) {
			return CRect(rClient.right-nSquareDimension, rClient.top + 2*nSquareDimension, rClient.right, rClient.top + 3*nSquareDimension);
		}
		else {
			return CRect(0,0,0,0);
		}
		break;
	case daPreview4:
		if(m_bShowPreviewArea) {
			return CRect(rClient.right-nSquareDimension, rClient.top + 3*nSquareDimension, rClient.right, rClient.top + 4*nSquareDimension);
		}
		else {
			return CRect(0,0,0,0);
		}
		break;
	default:
		ASSERT(FALSE);
		return CRect(0,0,0,0);
		break;
	}
}

void CPhotoViewerCtrl::FillPreviewRect(CDC *pDC, CRect rRect, int nSelectedIndex, bool bHot)
{
	CBorrowDeviceContext bdc(pDC);

	CPen pOutline(PS_SOLID, 1, RGB(0,0,0));
	CBrush brGray(RGB(160,160,160));
	pDC->SelectObject(&pOutline);
	pDC->SelectStockObject(NULL_BRUSH);
	//We're going to Polyline rather than Rectangle for greater precision.
	CPoint *pRect = new CPoint[5];
	pRect[0] = CPoint(rRect.left, rRect.top);
	pRect[1] = CPoint(rRect.right, rRect.top);
	pRect[2] = CPoint(rRect.right, rRect.bottom);
	pRect[3] = CPoint(rRect.left, rRect.bottom);
	pRect[4] = CPoint(rRect.left, rRect.top);
	pDC->Polyline(pRect,5);
	//Now, draw an extra rectangle, one pixel farther out.
	CPen pOuter;
	if(bHot) {
		pOuter.CreatePen(PS_SOLID, 1,  RGB(0,0,0));
	}
	else {
		pOuter.CreatePen(PS_SOLID, 1, RGB(255,255,255));
	}
	pDC->SelectObject(&pOuter);
	CPoint *pRectOuter = new CPoint[5];
	pRectOuter[0] = CPoint(rRect.left-1, rRect.top-1);
	pRectOuter[1] = CPoint(rRect.right+1, rRect.top-1);
	pRectOuter[2] = CPoint(rRect.right+1, rRect.bottom+1);
	pRectOuter[3] = CPoint(rRect.left-1, rRect.bottom+1);
	pRectOuter[4] = CPoint(rRect.left-1, rRect.top-1);
	pDC->Polyline(pRectOuter, 5);

	
	//If we have an actual image, draw it, just inside the rectangle.
	if(m_listImagesSelected.GetCount() > nSelectedIndex) {
		rRect.DeflateRect(1,1,0,0);
		//Advance to that point in our list, get the image there.
		int i = 0;
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); i < nSelectedIndex; m_listImagesSelected.GetNext(p)) i++;
		HBITMAP hImage = m_listImagesSelected.GetAt(p)->hThumb;

		//We need to draw the bitmap, and fill in the region that wasn't drawn (careful to avoid painting over our outline rect.
		CRgn rgnImage;
		rgnImage.CreateRectRgn(rRect.left, rRect.top, rRect.right, rRect.bottom);
		CRect rDrawn = DrawDIBitmapInRect(pDC, rRect, hImage);
		CRgn rgnDrawn;
		rgnDrawn.CreateRectRgn(rDrawn.left, rDrawn.top, rDrawn.right, rDrawn.bottom);
		rgnImage.CombineRgn(&rgnImage, &rgnDrawn, RGN_DIFF);
		pDC->FillRgn(&rgnImage, &brGray);

		//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
		rgnImage.DeleteObject();
		rgnDrawn.DeleteObject();
	}
	else {
		//Just fill it in gray.
		//We're using Polygon rather than Rectangle, again, for greater precision.
		pDC->SelectObject(&pOutline);
		pDC->SelectObject(&brGray);
		pDC->Polygon(pRect,5);
	}

	delete[] pRect;			// (a.walling 2006-07-07 09:36) - PLID 21325 Free memory for each item in the array instead of just the first
	delete[] pRectOuter;
}

void CPhotoViewerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//Secure dragging operations, Mr. Moe
	if(m_nDragType == dtFocusRect) {

		ReleaseCapture();
	
		//If we actually ever dragged
		if(m_bDragStarted) {
			//Calling this function twice in succession for the same rect erases the rect
			CBorrowDeviceContext bdc(this);
			bdc.m_pDC->DrawFocusRect(m_rLastDragRect);
			for(int i = 0; i < m_arImages.GetSize(); i++) {
				if(IsImageSelected(i)) InvalidateImage(i);
			}
		}
	}
	else if(m_nDragType == dtImage) {
		ReleaseCapture();

		//If we actually dragged.
		if(m_bDragStarted && m_nDraggedImage != -1) {
			InvalidateRect(m_rLastDragRect);
			CRect rClient;
			GetClientRect(rClient);
			if(rClient.PtInRect(point)) {
				//OK, which region did they drop it in.
				CString strProcedureNames = "";
				CArray<long, long> aryProcedureIDs;
				int i=0;
				for(i=0; i < m_arProcNameRects.GetSize() && strProcedureNames.IsEmpty(); i++) {
					if(m_arProcNameRects.GetAt(i)->pRgn->PtInRegion(point)) {
						// (j.jones 2009-10-13 10:26) - PLID 35894 - supported multiple procedures
						strProcedureNames = m_arProcNameRects.GetAt(i)->strProcedureNames;
						aryProcedureIDs.Append(m_arProcNameRects.GetAt(i)->arynProcedureIDs);
					}
				}
				if(m_arImages.GetAt(m_nDraggedImage)->strProcedureNames != strProcedureNames) {
					// (j.jones 2009-10-12 15:51) - PLID 35894 - supported multiple procedures
					CString strSqlBatch;
					long nMailID = m_arImages.GetAt(m_nDraggedImage)->nID;
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentProcedureT WHERE MailSentID = %li", nMailID);
					for(int i=0; i<aryProcedureIDs.GetSize(); i++) {
						//(s.dhole 10/9/2014 11:18 AM ) - PLID  37718 Try to insert MailSentProcedureT , if records exist in MailSent table
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MailSentProcedureT (MailSentID, ProcedureID) Select MailID, %li AS ProcedureID FROM  MailSent  Where MailID =%li", aryProcedureIDs.GetAt(i), nMailID);
					}
					ExecuteSqlBatch(strSqlBatch);

					// (j.jones 2012-07-12 17:37) - PLID 49636 - send a tablechecker
					// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
					CClient::RefreshMailSentTable(m_nPersonID, nMailID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

					// (j.jones 2009-10-14 11:33) - PLID 35894 - changing procedures requires ReloadImage to be called
					ReloadImage(m_nDraggedImage);
					//ReloadImage will ensure the image remains selected, if it was previously selected

					m_nDraggedImage = -1;
					Invalidate();
				}
			}
		}
		if(m_pGhostlyImageWnd) {
			m_pGhostlyImageWnd->DestroyWindow();
			delete m_pGhostlyImageWnd;
			m_pGhostlyImageWnd = NULL;
		}
		DeleteObject(m_hScreenshot);
		m_hScreenshot = NULL;
	}

	m_nDragType = dtNull;
	m_bDragStarted = false;

	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CPhotoViewerCtrl::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) {
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_CONTROL) {
			if(!m_bControlDown) {
				m_bControlDown = true;
				m_nCtrlSelIndex = 0;
			}
		}
		else if(pMsg->wParam == VK_ESCAPE) {
			if(m_pTempEdit) {
				//Cancel their changes.
				//First of all, set m_pTempEdit to null, so that no other functions (say OnKillFocus) try to do anything with it.
				CNxEdit *pTmp = m_pTempEdit;
				m_pTempEdit = NULL;
				pTmp->DestroyWindow();
				delete pTmp;
				return TRUE;
			}
		}
		else if(pMsg->wParam == VK_RETURN) {
			if(m_bControlDown) {
				//Commit their changes.
				OnKillFocusTempEdit();
				return TRUE;
			}
		}
		// (r.farnworth 2015-06-15 11:26) - PLID 64785 -Clicking up or down while renaming a photo was crashing Nextech
		else if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN) {
			if (m_pTempEdit) {
				return TRUE;
			}
		}
		break;
	case WM_KEYUP:
		if(pMsg->wParam == VK_CONTROL) {
			m_bControlDown = false;
		}
		break;
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void CPhotoViewerCtrl::UnselectImage(int nIndex)
{
	bool bFound = false;
	for(POSITION p = m_listImagesSelected.GetHeadPosition(); p && !bFound; m_listImagesSelected.GetNext(p)) {
		if(m_listImagesSelected.GetAt(p)->nID == m_arImages.GetAt(nIndex)->nID && m_arImages.GetAt(nIndex)->nType == m_listImagesSelected.GetAt(p)->nType) {
			bFound = true;
			//It was selected, unselect it.
			m_listImagesSelected.RemoveAt(p);
			//(e.lally 2012-04-24) PLID 49637 - Return here to avoid access violation
			return;
		}
	}
}

void CPhotoViewerCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	CRect rMainList = GetDisplayArea(daMainList);
	if(m_sbVert.GetSafeHwnd()) {
		m_sbVert.SetWindowPos(this, rMainList.right - m_nScrollBarWidth, rMainList.top, m_nScrollBarWidth, rMainList.Height(), SWP_NOZORDER);
		SetScroll();
	}

	// (a.walling 2008-09-24 13:36) - PLID 31495 - These are no longer valid, reset them.
	m_nMaxHeight = 0;
	m_nNumColumns = 0;
	Invalidate();
}

void CPhotoViewerCtrl::PrepareWindow()
{
	if(!m_sbVert.GetSafeHwnd()) {
		//I'm giving this a control ID of 100.  Why?  Because that's how the MSDN example code did it.
		m_sbVert.Create(SBS_VERT|SBS_RIGHTALIGN|WS_CHILD|WS_VISIBLE, GetDisplayArea(daMainList), this, 100);
		m_sbVert.ShowScrollBar(TRUE);
		CRect rScroll;
		m_sbVert.GetClientRect(rScroll);
		m_nScrollBarWidth = rScroll.Width();
	}

	
}

void CPhotoViewerCtrl::SetScroll()
{
	if(!m_sbVert.GetSafeHwnd()) return;

	CRect rClient = GetDisplayArea(daMainList);
	int nVertMin = 0;
	int nVertMax = 0;
	for(int i = 0; i < m_arImageRects.GetSize(); i++) {
		if(m_arImageRects.GetAt(i).top-BUFFER_VERT < nVertMin) nVertMin = m_arImageRects.GetAt(i).top-BUFFER_VERT;
		if(m_arImageRects.GetAt(i).bottom > nVertMax) nVertMax = m_arImageRects.GetAt(i).bottom;

		// (a.walling 2008-09-24 10:01) - PLID 31495
		if(nVertMax-nVertMin > rClient.Height()) {
			break;
		}		
	}

	if(nVertMax-nVertMin <= rClient.Height()) {
		// (a.walling 2008-09-24 10:01) - PLID 31495
		m_sbVert.EnableScrollBar(ESB_DISABLE_BOTH);
		m_nScrollPos = 0;
	} else {

		m_sbVert.ShowScrollBar(TRUE);
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE|SIF_PAGE|SIF_POS;
		si.nMin = 0;
		// (a.walling 2008-11-10 10:27) - PLID 31495 - Since we are now always 32767 max, set the page size
		// to the appropriate number of 'rows' within our list. This would be 32767 / (number of images / number of images per row)
		si.nPage = 32767 / (m_arImages.GetSize() / (m_nNumColumns > 0 ? m_nNumColumns : 1));
		// (a.walling 2008-09-24 10:01) - PLID 31495
		si.nMax = 32767;
		si.nPos = m_nScrollPos;

		m_sbVert.SetScrollInfo(&si);

		
		m_sbVert.EnableScrollBar(ESB_ENABLE_BOTH);
	}
}


void CPhotoViewerCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(!m_sbVert.IsWindowVisible()) return;

	CRect rcClient;
	GetClientRect(rcClient);

	int nNewPos;
	int nOldPos = m_sbVert.GetScrollPos();
	bool bDown = false;
	double dScrollScale = double(32767) / ((m_nMaxHeight > 0) ? m_nMaxHeight : 32767);
	long nSingleLine = long(dScrollScale * ((IMAGE_SIZE + BUFFER_VERT)/4));
	BOOL bRedraw = TRUE;
	switch (nSBCode) {
	case SB_NX_MOUSEWHEEL:
		// (a.walling 2008-09-24 12:51) - PLID 31495 - nPos is actually the number of 'notches' the mousewheel moved
		nNewPos = m_sbVert.GetScrollPos()-(nSingleLine * nPos);
		break;
	case SB_LINEUP:
		nNewPos = m_sbVert.GetScrollPos()-nSingleLine;
		break;
	case SB_LINEDOWN:
		bDown = true;
		nNewPos = m_sbVert.GetScrollPos()+nSingleLine;
		break;
	case SB_PAGEUP:
		{
			nNewPos = long(m_sbVert.GetScrollPos()-((rcClient.Height() * 32767) / m_nMaxHeight) );
		}
		break;
	case SB_PAGEDOWN:
		{
			bDown = true;
			nNewPos = long(m_sbVert.GetScrollPos()+((rcClient.Height() * 32767) / m_nMaxHeight) );
		}
		break;
	case SB_TOP:
		nNewPos = 0;
		break;
	case SB_BOTTOM:
		bDown = true;
		nNewPos = m_sbVert.GetScrollLimit();
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			bRedraw = FALSE;
			nNewPos = nPos;
		}
		break;
	case SB_ENDSCROLL:
		CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
		break;
	default:
		// Do nothing special
		break;
	}
	
	if(nNewPos < 0) nNewPos = bDown ? m_sbVert.GetScrollLimit() : 0;
	if(nNewPos > m_sbVert.GetScrollLimit()) nNewPos = m_sbVert.GetScrollLimit();

	m_nScrollPos = nNewPos;
	m_sbVert.SetScrollPos(m_nScrollPos, bRedraw);
	
	if(nOldPos != nNewPos) {
		CRect rInvalid = GetDisplayArea(daMainList);
		rInvalid.DeflateRect(0,0,m_nScrollBarWidth,0);
		InvalidateRect(rInvalid);
	}

	//CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPhotoViewerCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	try {
		SetFocus();
		
		//First off, if we're not loaded, fuggedahboutit.
		if(!m_bImagesLoaded) return;

		HitTestType htt;
		// (j.jones 2009-10-13 09:53) - PLID 35894 - this function now also returns the procedure names
		CString strClickedProcedureNames;
		int nClicked = HitTest(point.x, point.y, htt, strClickedProcedureNames);
		
		if((htt == httImage || htt == httImageNotes) && nClicked != -1 && m_arImages.GetAt(nClicked)->bIsThumbValid) {
			if(!IsImageSelected(nClicked)) {
				if(!m_bAllowMultiSelect) UnselectAll();
				m_listImagesSelected.AddTail(m_arImages.GetAt(nClicked));
				InvalidateImage(nClicked);
				CBorrowDeviceContext bdc(this);
				DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));
				if(m_arImages.GetAt(nClicked)->nType == eImageSrcPractice) {
					GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)TRUE);
				}
			}
			//Only show the right-click menu if at least one of the selected images is an attached photo, they don't apply to mirror images.
			bool bPracticeFound = false;
			bool bMirrorFound = false;
			bool bUnitedFound = false;
			bool bDiagramFound = false;
			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) bPracticeFound = true;
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcMirror) bMirrorFound = true;
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcUnited) bUnitedFound = true;
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcDiagram) bDiagramFound = true;
			}

			CMenu mnuProcs;
			mnuProcs.m_hMenu = CreatePopupMenu();
			CMenu mnuRotate;
			mnuRotate.m_hMenu = CreatePopupMenu();
			CMenu mnuCats;
			mnuCats.m_hMenu = CreatePopupMenu();
			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			int nIndex = 0;
			
			if(bMirrorFound) {
				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_LAUNCH_MIRROR, "Launch Mirror");
				nIndex++;
			}
			if(bUnitedFound) {
				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_LAUNCH_UNITED, "Launch United");
				nIndex++;
			}
			if(bPracticeFound) {
				// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
				m_bNeedsSort = true;

				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_OPEN_PICTURE, "Open With Default Imaging Program");
				nIndex++;

				if(m_listImagesSelected.GetCount() == 1) {
					
					if(m_bIsPatient) {
						if(m_arImages.GetAt(nClicked)->nID == m_nPrimaryImageID) {
							mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_TOGGLE_PRIMARY, "Reset primary picture");
						}
						else {
							mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_TOGGLE_PRIMARY, "Mark as primary picture");
						}
						nIndex++;
					}
					mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_EDIT_NOTES, "Edit Notes");
					nIndex++;
				}

				mnuRotate.InsertMenu(0, MF_BYPOSITION, ID_ROTATE_90, "90°");
				mnuRotate.InsertMenu(1, MF_BYPOSITION, ID_ROTATE_180, "180°");
				mnuRotate.InsertMenu(2, MF_BYPOSITION, ID_ROTATE_270, "270°");
				mnu.InsertMenu(nIndex, MF_BYPOSITION|MF_POPUP, (UINT)mnuRotate.m_hMenu, "Rotate Clockwise");
				nIndex++;

				//DRT 8/1/2005 - PLID 17125 - I changed the way the ID numbers are handled.  Previously the procedure base
				//	was given an ID of 110.  The code then added 110 + procedure ID to get the id for the menu item.  This
				//	didn't work well when I wanted to add categories to do the same thing.  So I've modified it to split
				//	up the values into a 32 bit integer, broken into 2 16-bit sections.  It looks as follows:
				//	|         nType|       nData|
				//	where nType is the type of popup (ID_ASSIGN_PROC_BASE or ID_ASSIGN_CATEGORY_BASE), and nData is the actual
				//	ID number in the corresponding table.  So filled out data will look something like:
				//	|           110|         199|
				//	which is a Procedure (110) of ID 199.

				//Give them the choice of a.) any procedures they already have groups for, b.) any procedures being tracked.
				//c.) any procedures they have an EMR for.
				int nSubIndex = 0;
				mnuProcs.InsertMenu(nSubIndex, MF_BYPOSITION, ID_ASSIGN_PROC_BASE, "Select Multiple Procedures...");
				nSubIndex++;
				// (c.haag 2009-01-08 12:57) - PLID 32539 - Hide inactive procedures
				// (j.jones 2009-10-12 15:58) - PLID 35894 - supported MailSentProcedureT
				_RecordsetPtr rsProcs = CreateParamRecordset("SELECT ID, Name FROM ProcedureT "
					"INNER JOIN "
					"(SELECT ProcedureID "
						"FROM MailSent "
						"INNER JOIN MailSentProcedureT ON MailSent.MailID = MailSentProcedureT.MailSentID "
						"WHERE PersonID = {INT} "
						"GROUP BY ProcedureID "
					" UNION SELECT ProcedureID FROM ProcInfoDetailsT INNER JOIN ProcInfoT ON ProcInfoDetailsT.ProcInfoID = "
					"              ProcInfoT.ID WHERE ProcInfoT.PatientID = {INT} GROUP BY ProcedureID "
					" UNION SELECT ProcedureID FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EmrMasterT "
					"INNER JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EmrProcedureT ON EmrMasterT.ID = EmrProcedureT.EMRID WHERE PatientID = {INT} GROUP BY ProcedureID) SubQ "
					"ON ProcedureT.ID = SubQ.ProcedureID WHERE ProcedureT.Inactive = 0 ORDER BY ProcedureT.Name", m_nPersonID, m_nPersonID, m_nPersonID);
				while(!rsProcs->eof) {
					DWORD dwID = (WORD)ID_ASSIGN_PROC_BASE;
					dwID = dwID << 16;
					dwID += AdoFldLong(rsProcs, "ID");

					mnuProcs.InsertMenu(nSubIndex, MF_BYPOSITION, dwID, AdoFldString(rsProcs, "Name"));
					nSubIndex++;
					rsProcs->MoveNext();
				}				
				mnu.InsertMenu(nIndex, MF_BYPOSITION|MF_POPUP, (UINT)mnuProcs.m_hMenu, "Assign to Procedure...");
				nIndex++;

				//
				//DRT 8/1/2005 - PLID 17125 - Allow them to recategorize images from this menu as well.
				{
					CStringArray arySelectedCats;
					for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
						if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
							arySelectedCats.Add(m_listImagesSelected.GetAt(p)->strCategory);
						}
					}

					int nSubIndex = 0;

					//Add an empty row first
					{
						DWORD dwID = (WORD)ID_ASSIGN_CATEGORY_BASE;
						dwID = dwID << 16;
						dwID += 0;	//0 for no id

						//If the any selected images have no category, we'll check this
						UINT nFlags = MF_BYPOSITION;
						for(int nCnt = 0; nCnt < arySelectedCats.GetSize(); nCnt++) {
							if(arySelectedCats.GetAt(nCnt).CompareNoCase("") == 0) {
								nFlags |= MF_CHECKED;
								break;
							}
						}

						mnuCats.InsertMenu(nSubIndex, nFlags, dwID, "{No Category Selected}");
						nSubIndex++;
					}

					//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
					_RecordsetPtr prsCats = CreateParamRecordset("SELECT ID, Description FROM NoteCatsF WHERE {SQLFRAGMENT} ORDER BY Description",
						GetAllowedCategoryClause_Param("ID"));
					while(!prsCats->eof) {
						DWORD dwID = (WORD)ID_ASSIGN_CATEGORY_BASE;
						dwID = dwID << 16;
						dwID += AdoFldLong(prsCats, "ID");

						CString strCat = AdoFldString(prsCats, "Description");

						//If the category we're looking at is current for one of our selected images, then we'll
						//	check it.
						UINT nFlags = MF_BYPOSITION;
						for(int nCnt = 0; nCnt < arySelectedCats.GetSize(); nCnt++) {
							if(arySelectedCats.GetAt(nCnt).CompareNoCase(strCat) == 0) {
								nFlags |= MF_CHECKED;
								break;
							}
						}

						mnuCats.InsertMenu(nSubIndex, nFlags, dwID, strCat);
						nSubIndex++;
						prsCats->MoveNext();
					}
					mnu.InsertMenu(nIndex, MF_BYPOSITION|MF_POPUP, (UINT)mnuCats.m_hMenu, "Assign to Category...");
					nIndex++;
				}
				//Done categories
				//

				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_DETACH, "Detach");
				nIndex++;
				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_DETACH_AND_DELETE, "Detach and Delete");
				nIndex++;

				// (a.wetta 2007-07-09 13:55) - PLID 17467 - Add option to mark photos as non-photos
				mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_MARK_AS_NON_PHOTO, "Mark as non-photo");
				nIndex++;

				// (c.haag 2010-05-26 11:59) - PLID 38731 - Have the option to create a new task
				if (m_listImagesSelected.GetCount() == 1 && m_bIsPatient && m_nPersonID > 0) {
					mnu.InsertMenu(nIndex, MF_BYPOSITION, ID_NEW_TODO, "Create To-Do Task For This Photo");
					nIndex++;
				}
			}

			//Get rid of the tip.
			if(m_pToolTipWnd) {
				m_pToolTipWnd->ShowWindow(SW_HIDE);
			}
			KillTimer(HOVER_TICK);
			
			if(nIndex > 0) {
				CPoint pos;		
				GetCursorPos(&pos);

				if(AfxIsExtendedFrameClass(GetTopLevelFrame())) {
					AfxGetPracticeApp()->ShowPopupMenu(mnu, pos.x, pos.y, this, TRUE);
				} else {
					mnu.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this, NULL);
				}

				return;
			}
		}
	}NxCatchAll("Error in CPhotoViewerCtrl::OnRButtonDown()");

	CWnd::OnRButtonDown(nFlags, point);
}
//(e.lally 2005-05-27) inserting the condition for trying to rotate a deleted image
//Also consolidated OnRotate functions into one function. Do not rotate strictly by degrees, it will shrink and distort the images.

void CPhotoViewerCtrl::RotateImageCW(int nDegrees)
{
	//Loop through all our selected images.
	bool bAllSuccess = true;
	bool bAllExist = true;

	// (a.walling 2008-10-13 13:54) - PLID 31670 - Check for readonly statuses first, then prompt
	long nReadOnly = 0;
	for(POSITION pPre = m_listImagesSelected.GetHeadPosition(); pPre; m_listImagesSelected.GetNext(pPre)) {
		// (a.walling 2008-10-13 12:11) - PLID 31670 - Prompt if readonly source
		CString strPath = m_listImagesSelected.GetAt(pPre)->strFullFilePath;
		CString strFile = strPath.Mid(strPath.ReverseFind('\\')+1);
		if(strPath == strFile) strPath = GetPatientDocumentPath(m_nPersonID) ^ strFile;

		DWORD attribs = GetFileAttributes(strPath);
		if ((INVALID_FILE_ATTRIBUTES != attribs) && (attribs & FILE_ATTRIBUTE_READONLY)) {
			nReadOnly++;
		}
	}

	bool bMarkWritable = false;
	if (nReadOnly > 0) {
		if (IDYES == MessageBox(FormatString("%li file(s) are marked read-only. Rotating the image requires writing to the file. Do you want to make these files writable?\r\n\r\nIf you choose No, any read-only files will be skipped.", nReadOnly), NULL, MB_YESNO|MB_ICONQUESTION)) {
			bMarkWritable = true;
		}
	}

	for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {/* for loop 1 */
		bool bThisSuccess = false;
		if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) { /* if 1 */
			CString strPath = m_listImagesSelected.GetAt(p)->strFullFilePath;
			CString strFile = strPath.Mid(strPath.ReverseFind('\\')+1);
			if(strPath == strFile) strPath = GetPatientDocumentPath(m_nPersonID) ^ strFile;

			CWaitCursor cws;
			CString strTempFile = GetNxTempPath() ^ strFile;

			DWORD attribs = GetFileAttributes(strPath);
			if ((INVALID_FILE_ATTRIBUTES != attribs) && (attribs & FILE_ATTRIBUTE_READONLY)) {
				if (bMarkWritable) {
					SetFileAttributes(strPath, attribs & (~(DWORD)FILE_ATTRIBUTE_READONLY));
				} else {
					continue;
				}
			}

			// (a.walling 2008-10-13 12:14) - PLID 31670 - if the temp file exists, delete it.
			DWORD tempattribs = GetFileAttributes(strTempFile);
			if (INVALID_FILE_ATTRIBUTES != tempattribs) {
				if (tempattribs & FILE_ATTRIBUTE_READONLY) {
					SetFileAttributes(strTempFile, tempattribs & (~(DWORD)FILE_ATTRIBUTE_READONLY));
				}
				DeleteFile(strTempFile);
			}

			if(CopyFile(strPath, strTempFile, FALSE)) {/* if 3 */
				CxImage image;
				if(image.Load(strTempFile)) {/* if 4 */
					DWORD dwType = image.GetType();
					//use parameter to determine which rotate to use
					switch(nDegrees){
						case 90:
						{
							image.RotateRight();
							break;
						}//end case 90
						case 180:
						{
							image.Rotate180();
							break;		 
						}//end case 180
						case 270:
						{
							image.RotateLeft();
							break;
						}//end case 270
						default:
						{
							//unknown rotate amount
						}
					}//end switch
					if(image.Save(strTempFile, dwType)) {/* if 5 */
						//OK, we've succeeded.  Write it back.
						if(CopyFile(strTempFile, strPath, FALSE)) {/* if 6 */
							bThisSuccess = true;
							//We need to update it in our main list.
							for(int i = 0; i < m_arImages.GetSize();  i++) {/* for loop 2 */
								if(m_listImagesSelected.GetAt(p)->nID == m_arImages.GetAt(i)->nID && m_listImagesSelected.GetAt(p)->nType == m_arImages.GetAt(i)->nType) {/* if 7 */
									if(m_arImages.GetAt(i)->nType == eImageSrcPractice) {/* if 8 */
										//Regenerate the thumbnail.
										// (a.walling 2013-04-24 14:57) - PLID 56247 - Unnecessary; will be refreshed the usual way. This is a rare operation, not a big deal.
										// (a.walling 2008-06-17 18:08) - PLID 30424 - this HBITMAP leak was causing all sorts of wierd issues
										// with an office when running out of memory rotating huge images. Ack!
										/*HBITMAP hBmpRotatedImage = image.MakeBitmap();
										WriteThumbnailFromImage(hBmpRotatedImage, strPath);
										DeleteObject(hBmpRotatedImage);*/
									}// end if 8
									ReloadImage(i);
									//ReloadImage will ensure the image remains selected
								}// end if 7 
							}// end for loop 2 
						}// end if 6 
					}// end if 5 
				}// end if 4 

				// (a.walling 2008-10-13 12:13) - PLID 31670 - Delete the temp file
				DeleteFileWhenPossible(strTempFile);
			}// end if 3 
			else if(!m_listImagesSelected.GetAt(p)->bIsImageValid){
				MsgBox("Could not rotate image file '" + strPath + "'.\n\nThis file may have been deleted. " +
					"If the file you are looking for is on another computer, " +
					"make sure that your network is functioning properly.");
				bAllExist = false;
				//continue rotating the rest of the images
			}//end else
		}// end if 1 
		if(!bThisSuccess) bAllSuccess = false; /* if 9 *//*end if 9*/
	}// end for loop 1
	if(!bAllSuccess && bAllExist) {
		MsgBox("At least one image could not be rotated.  You may not have permission to edit this file, or it may be read-only.");
	}
	CBorrowDeviceContext bdc(this);
	DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));

}//end RotateImage

void CPhotoViewerCtrl::OnRotate90()
{
	RotateImageCW(90);
	
}

void CPhotoViewerCtrl::OnRotate180()
{
	RotateImageCW(180);

}

void CPhotoViewerCtrl::OnRotate270()
{
	RotateImageCW(270);

}

void CPhotoViewerCtrl::OnOpenPicture()
{
	try {
		if(m_nPersonID != -1) {
			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
					OpenDocument(m_listImagesSelected.GetAt(p)->strFullFilePath, m_nPersonID);
				}
			}
		}
	}NxCatchAll("Error opening image");
}


void CPhotoViewerCtrl::OnTogglePrimary()
{
	try {
		// (b.cardillo 2010-03-11 11:05) - PLID 37705 - Make sure there's a selected image
		// Get the image pointer (if there is one, which there generally should be)
		// (a.walling 2010-12-20 16:05) - PLID 41917
		ImageInformationPtr pImg;
		{
			POSITION p = m_listImagesSelected.GetHeadPosition(); //We should never get here with more or less than one selected.
			if (p != NULL) {
				pImg = m_listImagesSelected.GetAt(p);
			}
		}
		// If we got an image, toggle it as primary
		if (pImg) {
			// (a.walling 2010-12-27 12:34) - PLID 40908
			_variant_t varNewPrimary = g_cvarNull;
			if (pImg->nID == m_nPrimaryImageID) {
				//OK, we're setting it to no primary.
			} else if (pImg->nType == eImageSrcPractice){
				//We're setting this one as the primary.
				varNewPrimary = pImg->nID;
			}
			
			//(s.dhole 10/8/2014 3:59 PM ) - PLID  37718 Try to update 		PatientsT.PatPrimaryHistImage
			_RecordsetPtr pPrimaryrs = CreateParamRecordset("SET NOCOUNT ON "
				" DECLARE @PersonID  int  "
				" DECLARE @MailID  int  "
				" SET @PersonID = {INT}  "
				" SET @MailID = {VT_I4}  "
				" UPDATE PatientsQ SET PatPrimaryHistImage = MailSent.MailID FROM  PatientsT PatientsQ INNER JOIN  MailSent  ON PatientsQ.PersonID = MailSent.PersonID Where MailSent.PersonID = @PersonID AND MailSent.MailID = @MailID "
				" SET NOCOUNT OFF  "
				" SELECT  PersonID FROM PatientsT WHERE   PersonID = @PersonID AND PatPrimaryHistImage = @MailID", m_nPersonID, varNewPrimary);
			// return records mean, we could update PatPrimaryHistImage 
			if (!pPrimaryrs->eof)
			{
				SetPrimaryImage(varNewPrimary.vt == VT_I4 ? (long)varNewPrimary : -1);
				// (a.walling 2010-12-27 09:14) - PLID 40908 - Send a table checker that G1 info has changed (since the primary photo is on G1)
				CClient::RefreshTable(NetUtils::PatG1, m_nPersonID);
				//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
				// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
				// this shared function will handle sending accordingly
				SendPatientPrimaryPhotoHL7Message(m_nPersonID);
			}
			else
			{
				//No return record , we fail to update PatPrimaryHistImage  and photo is removed, shoud call refresh
				CClient::RefreshMailSentTable(m_nPersonID, pImg->nID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);
			}
		} else {
			// This should almost never happen because we can only be called by the context menu 
			// and only if there is exactly one image selected.  But it's possible for the 
			// selection to change during the short interval that the context menu is on screen, 
			// so we give a friendly message here.
			MessageBox(_T("There is no image selected.  Please select an image and try again."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	}NxCatchAll("Error setting primary image");
}

void CPhotoViewerCtrl::ReloadImage(int nIndex)
{
	// (a.walling 2014-04-29 16:49) - PLID 61965 - Added try/catch here
	try {
		if(nIndex < 0 || nIndex >= m_arImages.GetSize()) return;

		// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
		m_bNeedsSort = true;

		switch(m_arImages.GetAt(nIndex)->nType) {
		case eImageSrcMirror:
			{
				_RecordsetPtr rsMirrorID = CreateRecordset("SELECT MirrorID FROM PatientsT WHERE PersonID = %li", m_nPersonID);
				CString strMirrorID = AdoFldString(rsMirrorID, "MirrorID", "");
				rsMirrorID->Close();
				if(strMirrorID != "") {
					// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
					long nMirrorCount = Mirror::GetImageCount(strMirrorID);
					// (a.walling 2010-12-20 16:05) - PLID 41917
					ImageInformationPtr iiNew = boost::make_shared<ImageInformation>();
					iiNew->nType = eImageSrcMirror;
					iiNew->nID = m_arImages.GetAt(nIndex)->nID;
					iiNew->strToolText.Format("Mirror Image %i", iiNew->nID);
					iiNew->strNotes.Format("Mirror Image %i", iiNew->nID);
					iiNew->strCategory = "";
					iiNew->strStaff = "";
						
					//This gives us the thumbnail, which is all we get from mirror.
					// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
					iiNew->hFullImage = Mirror::LoadMirrorImage(strMirrorID, iiNew->nID, nMirrorCount, -1);
					if (iiNew->hFullImage == NULL) {
						iiNew->bIsImageValid = false;
						iiNew->hThumb = NULL;
						iiNew->bIsThumbValid = false;
					}
					else {
						iiNew->bIsImageValid = true;
						iiNew->hThumb = iiNew->hFullImage;
						iiNew->bIsThumbValid = true;
					}

					//is this image currently selected?
					for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
						if(m_listImagesSelected.GetAt(p)->nID == m_arImages.GetAt(nIndex)->nID && m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
							//switch the pointer to be the new pointer instead
							m_listImagesSelected.SetAt(p, iiNew);
						}
					}

					//remove the existing image
					if(m_arImages.GetAt(nIndex)->bIsImageValid) 
						DeleteObject(m_arImages.GetAt(nIndex)->hFullImage);
					if(m_arImages.GetAt(nIndex)->bIsThumbValid && m_arImages.GetAt(nIndex)->hThumb != m_hLoadingThumb)
						DeleteObject(m_arImages.GetAt(nIndex)->hThumb);

					m_arImages.RemoveAt(nIndex);

					//add the new image
					m_arImages.InsertAt(nIndex, iiNew);
				}
			}
		break;
		case eImageSrcUnited:
			{
				_RecordsetPtr rsUnitedID = CreateRecordset("SELECT UnitedID FROM PatientsT WHERE PersonID = %li", m_nPersonID);
				long nUnitedID = AdoFldLong(rsUnitedID, "UnitedID", -1);
				rsUnitedID->Close();
				if(nUnitedID != -1) {
					if (g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrUse) && g_userPermission[UnitedIntegration] != 0 && IsUnitedEnabled()) {
						CUnitedLink *pUnitedLink = GetMainFrame()->GetUnitedLink();
						if (pUnitedLink && pUnitedLink->GetRemotePath() != "") {
							long nUnitedCount = pUnitedLink->GetImageCount(nUnitedID);

							// (a.walling 2010-12-20 16:05) - PLID 41917
							ImageInformationPtr iiNew = boost::make_shared<ImageInformation>();
							iiNew->nType = eImageSrcUnited;
							iiNew->nID = m_arImages.GetAt(nIndex)->nID;
							iiNew->strToolText.Format("United Image %i", iiNew->nID+1);
							iiNew->strNotes.Format("United Image %i", iiNew->nID+1);
							iiNew->strCategory = "";
							iiNew->strStaff = "";
								
							//This gives us the thumb, which is all we get from United.
							iiNew->hFullImage = pUnitedLink->LoadImage(nUnitedID, iiNew->nID);
							if (iiNew->hFullImage == NULL) {
								iiNew->bIsImageValid = false;
								iiNew->hThumb = NULL;
								iiNew->bIsThumbValid = false;
							}
							else {
								iiNew->bIsImageValid = true;
								iiNew->hThumb = iiNew->hFullImage;
								iiNew->bIsThumbValid = true;
							}

							//is this image currently selected?
							for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
								if(m_listImagesSelected.GetAt(p)->nID == m_arImages.GetAt(nIndex)->nID && m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
									//switch the pointer to be the new pointer instead
									m_listImagesSelected.SetAt(p, iiNew);
								}
							}

							//remove the existing image
							if(m_arImages.GetAt(nIndex)->bIsImageValid) 
								DeleteObject(m_arImages.GetAt(nIndex)->hFullImage);
							if(m_arImages.GetAt(nIndex)->bIsThumbValid && m_arImages.GetAt(nIndex)->hThumb != m_hLoadingThumb)
								DeleteObject(m_arImages.GetAt(nIndex)->hThumb);

							m_arImages.RemoveAt(nIndex);

							//add the new image
							m_arImages.InsertAt(nIndex, iiNew);
						}
					}
				}
			}
			break;
		case eImageSrcPractice:
			{
				//We assume the id is still valid.
				// (a.walling 2010-12-20 16:05) - PLID 41917
				ImageInformationPtr iiNew = boost::make_shared<ImageInformation>();
				iiNew->nID = m_arImages.GetAt(nIndex)->nID;
				iiNew->nType = m_arImages.GetAt(nIndex)->nType;
				
				CString strMailSentFilePath;
				CString strDoc;
				
				// (j.jones 2008-09-05 08:53) - PLID 30288 - supported MailSentNotesT
				// (j.jones 2009-10-13 09:01) - PLID 35894 - removed ProcedureID from this recordset
				//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
				//(e.lally 2012-04-17) PLID 39543 - Added service date
				// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
				_RecordsetPtr rsPhoto = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PathName, Sender, Date, ServiceDate, Note, "
					"(SELECT Description FROM NoteCatsf WHERE NoteCatsf.ID = MailSent.CategoryID) AS CategoryName "
					"FROM MailSent "
					"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
					"WHERE MailSent.MailID = {INT} AND {SQLFRAGMENT}", iiNew->nID, GetAllowedCategoryClause_Param("MailSent.CategoryID"));

				if(!rsPhoto->eof) {

					// (a.walling 2006-10-23 10:47) - PLID 22991 - Check for thumb stream
					//bool bThumbExists = false;
					strMailSentFilePath = AdoFldString(rsPhoto, "PathName");

					// (j.jones 2013-09-19 14:19) - PLID 58547 - cleaned up these variables
					// so that strMailSentFilePath is exactly what is in MailSent (which is often,
					// but not always, just a file name only)
					// and strFullFilePath is always the full patch to the file

					// (a.walling 2007-01-30 09:28) - PLID 24475 - Get the full file path to add to the loading info
					CString strFullFilePath;
					CString strFileNameOnly = strMailSentFilePath.Mid(strMailSentFilePath.ReverseFind('\\')+1);
					// (a.walling 2012-02-22 09:01) - PLID 48321 - Use the strPatientDocumentPath we already loaded
					if(strFileNameOnly == strMailSentFilePath) {
						strFullFilePath = GetPatientDocumentPath(m_nPersonID) ^ strMailSentFilePath;
					}
					else {
						strFullFilePath = strMailSentFilePath;
					}

					iiNew->hThumb = LoadThumbFromFile(strFullFilePath);
					if (iiNew->hThumb != NULL) {
						iiNew->bIsThumbValid = true;
					}
					else {
						iiNew->bIsThumbValid = false;
					}

					if (!iiNew->bIsThumbValid) {
						// (a.walling 2006-10-26 09:23) - PLID 23168 - There is no thumb at all! Load an error message
						//		this should be taken care of with more detail above. this is a failsafe.
						CString str("Error");
						LoadErrorImage(iiNew->hThumb, str);
						iiNew->bIsThumbValid = true;
					}

					//We're not loading the full image yet, we're not going to unless we have to.
					iiNew->bIsImageValid = false;
					iiNew->hFullImage = NULL;

					iiNew->strCategory = AdoFldString(rsPhoto, "CategoryName", "");
					iiNew->strStaff = AdoFldString(rsPhoto, "Sender");

					iiNew->strMailSentFilePath = strMailSentFilePath;
					iiNew->strFullFilePath = strFullFilePath;
					iiNew->strNotes = AdoFldString(rsPhoto, "Note", "");
					iiNew->dtAttached = AdoFldDateTime(rsPhoto, "Date");
					//(e.lally 2012-04-16) PLID 39543 - Added service date
					iiNew->dtService = AdoFldDateTime(rsPhoto, "ServiceDate", g_cdtInvalid);

					// (j.jones 2009-10-13 08:57) - PLID 35894 - load the procedure info
					// (a.walling 2013-12-12 16:51) - PLID 60003 - Snapshot isolation loading Photo Viewer
					_RecordsetPtr rsProcs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ProcedureT.ID, ProcedureT.Name "
						"FROM ProcedureT "
						"INNER JOIN MailSentProcedureT ON ProcedureT.ID = MailSentProcedureT.ProcedureID "
						"WHERE MailSentProcedureT.MailSentID = {INT} "
						"ORDER BY ProcedureT.Name", iiNew->nID);
					while(!rsProcs->eof) {
						iiNew->arynProcedureIDs.Add(AdoFldLong(rsProcs, "ID"));
						if(!iiNew->strProcedureNames.IsEmpty()) {
							iiNew->strProcedureNames += ", ";
						}
						iiNew->strProcedureNames += AdoFldString(rsProcs, "Name");

						rsProcs->MoveNext();
					}
					rsProcs->Close();

					//(e.lally 2012-04-16) PLID 39543 - Added service date
					CString strServiceDate;
					if(iiNew->dtService.m_dt > 0 && iiNew->dtService.m_status == COleDateTime::valid){
						strServiceDate = FormatDateTimeForInterface(iiNew->dtService, 0, dtoDate, true);
					}
					iiNew->strToolText = "Filename: " + strFullFilePath.Mid(strFullFilePath.ReverseFind('\\')+1) + "\r\n" +
						"Staff: " + iiNew->strStaff + "\r\n" + 
						"Attach Date: " + FormatDateTimeForInterface(iiNew->dtAttached, 0, dtoDate, true) + "\r\n" +
						"Service Date: " + strServiceDate + "\r\n" +
						"Procedure: " + iiNew->strProcedureNames + "\r\n" +
						"Note: " + iiNew->strNotes;
				}
				else {
					iiNew->bIsThumbValid = false;
				}

				//Now, if the notes have changed, we need to reposition all the images, unless we're on the last line.
				BOOL bNotesChanged = m_arImages.GetAt(nIndex)->strNotes != iiNew->strNotes;

				//is this image currently selected?
				for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
					if(m_listImagesSelected.GetAt(p)->nID == m_arImages.GetAt(nIndex)->nID && m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
						//switch the pointer to be the new pointer instead
						m_listImagesSelected.SetAt(p, iiNew);
					}
				}

				//remove the existing image
				if(m_arImages.GetAt(nIndex)->bIsImageValid) 
					DeleteObject(m_arImages.GetAt(nIndex)->hFullImage);
				if(m_arImages.GetAt(nIndex)->bIsThumbValid && m_arImages.GetAt(nIndex)->hThumb != m_hLoadingThumb)
					DeleteObject(m_arImages.GetAt(nIndex)->hThumb);

				m_arImages.RemoveAt(nIndex);

				//add the new image
				m_arImages.InsertAt(nIndex, iiNew);

				if(bNotesChanged) {				
					bool bOnLastLine = true;
					for(int i = 0; i < m_arImageRects.GetSize(); i++) {
						if(m_arImageRects.GetAt(i).top > m_arImageRects.GetAt(nIndex).bottom) bOnLastLine = false;
					}
					if(bOnLastLine) {
						InvalidateRect(m_arImageRects.GetAt(nIndex));
					}
					else {
						Refresh();
					}
				}
				else {
					InvalidateRect(m_arImageRects.GetAt(nIndex));
				}
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

void CPhotoViewerCtrl::SetPrimaryImage(long nPrimaryImageID)
{
	// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
	m_bNeedsSort = true;
	long nOldID = m_nPrimaryImageID;
	m_nPrimaryImageID = nPrimaryImageID;
	for(int i = 0; i < m_arImages.GetSize(); i++) {
		if(m_arImages.GetAt(i)->nType == eImageSrcPractice && 
			(m_arImages.GetAt(i)->nID == nOldID || m_arImages.GetAt(i)->nID == m_nPrimaryImageID)) {
			InvalidateImage(i);
		}
	}
}

void CPhotoViewerCtrl::OnEditNotes()
{
	//Kill the tip.
	if(m_pToolTipWnd) {
		m_pToolTipWnd->ShowWindow(SW_HIDE);
	}
	KillTimer(HOVER_TICK);
	SetCapture();

	POSITION p = m_listImagesSelected.GetHeadPosition(); //We should never get here with more or less than one selected.
	//Now let's translate that to an index in our main list.
	int nIndex = -1;
	for(int i = 0; i < m_arImages.GetSize() && nIndex == -1; i++) {
		if(m_arImages.GetAt(i)->nID == m_listImagesSelected.GetAt(p)->nID && m_arImages.GetAt(i)->nType == m_listImagesSelected.GetAt(p)->nType) nIndex = i;
	}
	
	//OK, now what we're a-going to do is, create an edit control on top of the rectangle where the text is.
	CRect rTotal = m_arImageRects.GetAt(nIndex);
	rTotal.DeflateRect(0,IMAGE_SIZE,0,0);
	rTotal.InflateRect(2,2,2,2);
	
	if(m_pTempEdit) {
		//What?  This should be null!
		return;
	}
	m_pTempEdit = new CNxEdit;
	//Store the initial rect.
	//(e.lally 2012-04-24) PLID 49637 - Text label may be hidden, so ensure we have a min height.
	if(rTotal.Height() < 50){
		rTotal.bottom = rTotal.top + 50;
	}
	m_nStartEditHeight = rTotal.Height();
	//Scroll to the top of the note.
	if(rTotal.top < 0) {
		int nDiff = -1 * rTotal.top;
		rTotal.top += nDiff;
		rTotal.bottom += nDiff;

		// (a.walling 2008-09-24 12:36) - PLID 31495
		double dRelativeScrollPos = m_nScrollPos / double(32767);
		long nAdjScrollPos = long(m_nMaxHeight * dRelativeScrollPos);
		nAdjScrollPos -= nDiff;

		// now reverse!
		m_nScrollPos = (nAdjScrollPos * 32767) / m_nMaxHeight;

		SetScrollPos(SB_VERT, m_nScrollPos, TRUE);
		Invalidate();
	}
	//Fit in the window.
	CRect rClient;
	GetClientRect(rClient);
	if(rTotal.bottom > rClient.bottom) rTotal.bottom = rClient.bottom;

	m_pTempEdit->Create(WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_BORDER|ES_WANTRETURN|ES_AUTOVSCROLL|ES_CENTER, rTotal, this, IDC_TEMP_EDIT);
	m_pTempEdit->SetFont(m_pIconFont);
	m_pTempEdit->BringWindowToTop();
	m_pTempEdit->SetWindowText(m_arImages.GetAt(nIndex)->strNotes);
	m_pTempEdit->SetSel(0,-1, TRUE);
	m_pTempEdit->SetLimitText(4000);
	m_pTempEdit->SetFocus();
}

static bool bDestroyingTempEdit = false;
void CPhotoViewerCtrl::OnKillFocusTempEdit()
{
	try {
		if(bDestroyingTempEdit) return;

		ReleaseCapture();
		// (a.walling 2014-04-29 16:49) - PLID 61965 - Exit if no images selected
		if (m_listImagesSelected.IsEmpty()) {
			return;
		}
		
		ImageInformationPtr pSelImage = m_listImagesSelected.GetHead(); //We should never get here with more or less than one selected.
		if (!pSelImage) {
			return;
		}

		//Now let's translate that to an index in our main list.
		int nIndex = -1;
		ImageInformationPtr pCurImage;
		for(int i = 0; i < m_arImages.GetSize(); i++) {
			ImageInformationPtr pImage = m_arImages.GetAt(i);
			if (!pImage) {
				continue;
			}
			if(pImage->nID == pSelImage->nID && pImage->nType == pSelImage->nType) {
				nIndex = i;
				pCurImage = pImage;
				break;
			}
		}

		if (nIndex == -1 || !pCurImage) {
			return; // (a.walling 2014-04-29 16:49) - PLID 61965 - previously this kept going, ending up with an access violation
		}

		if(!m_pTempEdit) {
			//What?  How can this be?
			return;
		}

		CString strNewNotes;
		m_pTempEdit->GetWindowText(strNewNotes);
		
		// (j.jones 2008-09-05 08:57) - PLID 30288 - supported 4000 characters
		if(strNewNotes.GetLength() > 4000) {
			//Whaa?? This shouldn't be possible, we called SetLimitText!
			ASSERT(FALSE);
			MsgBox("The note you have entered is longer than 4000 characters.  It will be truncated.");
			strNewNotes = strNewNotes.Left(4000);
		}

		//Put this in its own try...catch block, so that if it fails the image will still be reloaded and the edit box destroyed.
		try {
			// (j.jones 2008-09-04 13:45) - PLID 30288 - supported MailSentNotesT
			ExecuteSql("UPDATE MailSentNotesT SET Note = '%s' WHERE MailID = %li", _Q(strNewNotes), pCurImage->nID);
		}NxCatchAll("Error updating data in CPhotoViewerCtrl::OnKillFocusTempEdit()");

		ReloadImage(nIndex);
		//ReloadImage will ensure the image remains selected

		//Now, ditch this edit box.
		bDestroyingTempEdit = true;
		m_pTempEdit->DestroyWindow();
		delete m_pTempEdit;
		m_pTempEdit = NULL;
		bDestroyingTempEdit = false;

		// (b.cardillo 2005-07-21 13:07) - PLID 16454 - The order may have changed so we need to redraw.
		Invalidate(FALSE);
	}NxCatchAll("Error in CPhotoViewerCtrl::OnKillFocusTempEdit()");
}

void CPhotoViewerCtrl::OnChangeTempEdit()
{
	//Change the size of the edit control.
	CRect rc, rcMax;
	GetClientRect(rcMax);
	m_pTempEdit->GetWindowRect(rc);
	ScreenToClient(rc);
	CString str;
	m_pTempEdit->GetWindowText(str);
	CBorrowDeviceContext bdc(this);
	CDC* pDC = bdc.m_pDC;
	long nTextHeight = pDC->DrawText(str, rc, DT_CENTER|DT_WORDBREAK|DT_CALCRECT);
	
	int nMaxHeight = rcMax.bottom - rc.top;
	long nNewBottom = rc.top + nTextHeight + 4;
	if (nNewBottom - rc.top > nMaxHeight) {
		nNewBottom = rc.top + nMaxHeight;
	}
	if(nNewBottom - rc.top < m_nStartEditHeight) {//Never shrink the edit box (then you would see the original label underneath).
		nNewBottom = rc.top + m_nStartEditHeight;
	}
	if(rc.top < rcMax.top) rc.top = rcMax.top;
	if(nNewBottom > rcMax.bottom) nNewBottom = rcMax.bottom;
	if(rc.Width() < IMAGE_SIZE+4) rc.right = rc.left + IMAGE_SIZE + 4;

	if (rc.bottom != nNewBottom) {
		rc.bottom = nNewBottom;
		m_pTempEdit->MoveWindow(rc);
	}
}

void CPhotoViewerCtrl::OnLaunchMirror()
{
	// (a.walling 2008-07-07 17:40) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	Mirror::Run(m_nPersonID);
}

void CPhotoViewerCtrl::OnLaunchUnited()
{
	try {
		///////////////////////////////////////////////////////
		// Open united (TODO: Put this in the generic link)
		_ConnectionPtr pConRemote(__uuidof(Connection));
		CUnitedLink *pUnitedLink = GetMainFrame()->GetUnitedLink();
		CString strPath = GetRemotePropertyText ("UnitedDataPath", "", 0, "<None>");
		CString strParams, strID;
		CString strCon, strSQL;

		// Get the correct path
		// (j.armen 2011-10-25 17:33) - PLID 46136 - We already have the current directory, no need to get it
		//GetCurrentDirectory(512, szCurrentDirectory);

		// Set the path because United needs it that way
		strPath = strPath.Left( strPath.ReverseFind('\\') );
		SetCurrentDirectory(strPath);
		strPath += "\\uni32.exe";
		
		/*
		// Set the external ID to the current time
		strID.Format("%d", time(NULL));
		strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
			(pUnitedLink->GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + pUnitedLink->GetRemotePassword() + ";") : "") +
			"Data Source=" + pUnitedLink->GetRemotePath() + ";";
		strSQL.Format("UPDATE tblPatient SET uExternalID = '%s' WHERE ID = %d",
			strID, m_nUnitedID);

		pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL);
		pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);
		pConRemote->Close();
		pConRemote.Detach();*/

		
		// Get the active patient ID
		_RecordsetPtr rs = CreateRecordset("SELECT UserDefinedID, UnitedID FROM PatientsT WHERE PersonID = %d",
			m_nPersonID);
		strID.Format("%d", AdoFldLong(rs->Fields->Item["UserDefinedID"]));
		long nUnitedID = AdoFldLong(rs, "UnitedID");
		rs->Close();

		// Update the ID in United
		strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
			(pUnitedLink->GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + pUnitedLink->GetRemotePassword() + ";") : "") +
			"Data Source=" + pUnitedLink->GetRemotePath() + ";";
		strSQL.Format("UPDATE tblPatient SET uExternalID = '%s' WHERE ID = %d",
			strID, nUnitedID);
		pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL);
		pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);
		pConRemote->Close();
		pConRemote.Detach();

		// Build the actual parameters
		strParams.Format("%s %s", strPath, strID);
		int nReturnCode = WinExec (strParams, SW_SHOW);
		if(nReturnCode != 0 && nReturnCode != 33) {//TES 2/25/2004: It consistently returns 33 for me even after successfully
													//opening.  It's a mystery to me.
			if(nReturnCode == ERROR_FILE_NOT_FOUND || nReturnCode == ERROR_PATH_NOT_FOUND) {
				MsgBox("Failed to open United.  The specified path (%s) could not be found.", strPath);
			}
			else {
				MsgBox("Failed to open United.  Unspecified error.");
			}
		}

	}NxCatchAll("Error in CPhotoViewerCtrl::OnLaunchUnited()");

	try {
	// (j.armen 2011-10-25 17:35) - PLID 46136 - Set our current directory back to the session path
	SetCurrentDirectory(GetPracPath(PracPath::SessionPath));
	} NxCatchAll("Error in CPhotoViewerCtrl::OnLaunchUnited()::SetCurrentDirectory()");
}

// (j.jones 2009-10-13 12:27) - PLID 35894 - changed to be an array of pointers
// (a.walling 2010-12-20 16:05) - PLID 41917
void CPhotoViewerCtrl::GetSelectedImages(CArray<ImageInformationPtr,ImageInformationPtr> &arSelected)
{
	for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
		arSelected.Add(m_listImagesSelected.GetAt(p));
	}
}

void CPhotoViewerCtrl::UnselectAll()
{
	for(int i = 0; i < m_arImages.GetSize(); i++) {
		if(IsImageSelected(i)) {
			UnselectImage(i);
			InvalidateImage(i);
		}
	}
}

// (a.walling 2006-10-26 09:21) - PLID 23168 - General function to draw error message without relying on LoadImageFile
// (a.walling 2010-12-20 16:05) - PLID 41917 - Take a const CString reference
void LoadErrorImage(HBITMAP &hImage, const CString &strFile)
{
	CDC dcMem;
	//CBorrowDeviceContext bdc(pViewerWnd);
	
	dcMem.CreateCompatibleDC(GetMainFrame()->GetDC());
	hImage = CreateCompatibleBitmap(dcMem.m_hDC, IMAGE_SIZE,IMAGE_SIZE);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(dcMem.m_hDC, hImage);
	dcMem.SetBoundsRect(CRect(0,0,IMAGE_SIZE,IMAGE_SIZE),DCB_DISABLE);

	CFont IconFont;
	LOGFONT lf;
	if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
		IconFont.CreateFontIndirect(&lf);
	}
	else {
		ASSERT(FALSE);
	}

	dcMem.SelectObject(IconFont);
	dcMem.SelectStockObject(BLACK_PEN);
	dcMem.SelectStockObject(WHITE_BRUSH);
	dcMem.Rectangle(0,0,IMAGE_SIZE,IMAGE_SIZE);
	//We will have to have two lines of text.
	CRect rText1 = CRect(1,IMAGE_SIZE/2,IMAGE_SIZE-1,IMAGE_SIZE/2);
	CRect rText2 = rText1;
	dcMem.DrawText(strFile, &rText1, DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS|DT_CALCRECT);
	dcMem.DrawText("could not be loaded.", &rText2, DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS|DT_CALCRECT);
	//Re-center vertically.
	CRect rActual1 = CRect(1,IMAGE_SIZE/2-((rText1.Height()+rText2.Height())/2),IMAGE_SIZE-1,IMAGE_SIZE/2-((rText1.Height()+rText2.Height())/2)+rText1.Height());
	CRect rActual2 = CRect(1,IMAGE_SIZE/2+((rText1.Height()+rText2.Height())/2)-rText2.Height(),IMAGE_SIZE-1,IMAGE_SIZE/2+((rText1.Height()+rText2.Height())/2));
	dcMem.DrawText(strFile, &rActual1, DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS);
	dcMem.DrawText("could not be loaded.", &rActual2, DT_CENTER|DT_WORDBREAK|DT_PATH_ELLIPSIS);
	dcMem.SelectObject(hbmpOld);

	IconFont.DeleteObject();
}

bool CPhotoViewerCtrl::LoadImageFile(CString &strFile, HBITMAP &hImage)
{
	bool bReturn = ::LoadImageFile(strFile, hImage, m_nPersonID);
	if(!bReturn) {
		//Draw an error message.
		LoadErrorImage(hImage, strFile);
	}
	return bReturn;
}


void CPhotoViewerCtrl::OnAssignProc()
{
	try {
		
		// (j.jones 2009-10-13 11:31) - PLID 35894 - The intention of this code
		// was to set a default procedure only if the selected images all had
		// the same procedure selected. We now allow multiple procedures, and
		// follow the same logic.
		CArray<long, long> aryDefaultProcedureIDs;
		CString strDefaultProcedureNames;
		BOOL bDefaultProcedureIDsInitialized = FALSE;
		BOOL bCanUseDefault = TRUE;
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); p != NULL && bCanUseDefault; m_listImagesSelected.GetNext(p)) {
			if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
				if(!bDefaultProcedureIDsInitialized) {
					//set the list to match our first image (could be an empty list)
					aryDefaultProcedureIDs.Append(m_listImagesSelected.GetAt(p)->arynProcedureIDs);
					strDefaultProcedureNames = m_listImagesSelected.GetAt(p)->strProcedureNames;
					bDefaultProcedureIDsInitialized = TRUE;
					bCanUseDefault = TRUE;
				}
				else {
					//if the list has been initialized, compare to the current image by names
					if(strDefaultProcedureNames != m_listImagesSelected.GetAt(p)->strProcedureNames) {
						//they do not match, so we cannot use a default
						aryDefaultProcedureIDs.RemoveAll();
						strDefaultProcedureNames = "";
						bCanUseDefault = FALSE;
					}
				}
			}
		}

		if(!bDefaultProcedureIDsInitialized) {
			//None of the selected images are MailSent images, we can do nothing here.
			return;
		}

		//Now, prompt the user.
		// (a.walling 2007-03-27 14:51) - PLID 25376 - Modified to support the new modular select procedure dialog
		// (j.jones 2009-10-13 11:38) - PLID 35894 - supported multiple procedures
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProcedureT");

		if(bCanUseDefault && aryDefaultProcedureIDs.GetSize() > 0) {
			dlg.PreSelect(aryDefaultProcedureIDs);
		}

		dlg.m_strNameColTitle = "Procedure";
		int nResult = dlg.Open("ProcedureT", "Inactive = 0", "ProcedureT.ID", "ProcedureT.Name", "Select procedures to assign to this photo:");
		if(nResult == IDOK) {

			CArray<long, long> aryProcedureIDs;
			dlg.FillArrayWithIDs(aryProcedureIDs);

			CVariantArray vaNewNames;
			dlg.FillArrayWithNames(vaNewNames);
			CString strNewProcedureNames;
			int i=0;
			for(i=0; i<vaNewNames.GetSize(); i++) {
				if(!strNewProcedureNames.IsEmpty()) {
					strNewProcedureNames += ", ";
				}
				strNewProcedureNames += AsString(vaNewNames[i]);
			}

			CString strSqlBatch;

			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {

					// (j.jones 2009-10-12 15:51) - PLID 35894 - update this image's procedures
					long nMailID = m_listImagesSelected.GetAt(p)->nID;
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentProcedureT WHERE MailSentID = %li", nMailID);
					for(i=0; i<aryProcedureIDs.GetSize(); i++) {

						//(s.dhole 10/8/2014 3:59 PM ) - PLID  37718 Try to insert MailSentProcedureT only if records exist in MailSent
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MailSentProcedureT (MailSentID, ProcedureID) Select MailID, %li AS ProcedureID FROM  MailSent  Where MailID =%li  ", aryProcedureIDs.GetAt(i),nMailID);
						
					}
					ExecuteSqlBatch(strSqlBatch);

					// (j.jones 2012-07-12 17:37) - PLID 49636 - send a tablechecker
					// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
					CClient::RefreshMailSentTable(m_nPersonID, nMailID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

					// (j.jones 2009-10-14 11:33) - PLID 35894 - changing procedures requires ReloadImage to be called
					int nIndex = -1;
					for(i=0; i<m_arImages.GetSize() && nIndex == -1; i++) {
						if(m_arImages.GetAt(i)->nID == m_listImagesSelected.GetAt(p)->nID && m_arImages.GetAt(i)->nType == m_listImagesSelected.GetAt(p)->nType) {
							nIndex = i;
						}
					}
					if(nIndex != -1) {
						ReloadImage(nIndex);
						//ReloadImage will ensure the image remains selected
					}
				}
			}

			if(!strSqlBatch.IsEmpty()) {
				ExecuteSqlBatch(strSqlBatch);
			}

			//Redraw, this will put the image in the new location with the rest of that procedure's images.
			Invalidate();
		}
	}NxCatchAll("Error in CPhotoViewerCtrl::OnAssignProc()");
}
					
// Utility function used only by CompareImageInfo()
// Compares the two given ImageInfo objects based on the given sort criterion and order.
// Returns 0 if pii1 and pii2 are equivalent based on the criterion and order, returns 1 if 
// pii1 is considered "later than" pii2, and returns -1 if pii1 is considered "before" pii2.
// This is the real core of the comparison process.  It compares one item to another given 
// a certain field to compare on and sort order to consider for that field.
// (a.walling 2010-12-20 16:05) - PLID 41917
int CompareImageInfoSingle(const ImageInformationPtr& pii1, const ImageInformationPtr& pii2, const CPhotoViewerCtrl::EDisplaySortCriteria edscSortCriterion, const BOOL bSortAscending)
{
	int nAns;

	// (b.cardillo 2005-07-15 14:54) - PLID 16454 - For now we force SUPER-PRIMARY sorting 
	// by procedure ascending.  This is because the photo viewer can't handle it when there 
	// are procedural photos that aren't grouped together at the top of the sort order, a 
	// limitation of the photo viewer that will be addressed in PLID 17037.  When it is 
	// fixed, this block can be removed and the sorting capability will return to its full 
	// functionality.  Also, we'll have to give the user an interface for selecting the 
	// procedure name as part of the sort criteria.

	// (b.cardillo 2005-07-21 12:51) - PLID 17078 - Removed the special overriding code now 
	// that PLID 17037 is done.

	// Compare the two ImageInfo objects based on the given criteria
	switch (edscSortCriterion) {
	case CPhotoViewerCtrl::dscStaff:
		// Compare by staff name
		nAns = pii1->strStaff.CompareNoCase(pii2->strStaff);
		break;
	case CPhotoViewerCtrl::dscAttachDate:
	{
		// Compare by attach date
		//(e.lally 2012-04-25) PLID 49637 - Need to check validity now that we initialize attached to invalid
		//	This pertains mostly to non-patient images
		bool bP1IsValid = (pii1->dtAttached.m_dt > 0 && pii1->dtAttached.m_status == COleDateTime::valid);
		bool bP2IsValid = (pii2->dtAttached.m_dt > 0 && pii2->dtAttached.m_status == COleDateTime::valid);
		if(!bP1IsValid && !bP2IsValid){
			//Both invalid, consider equal
			nAns = 0;
		}
		else if(!bP1IsValid){
			nAns = -1;
		}
		else if(!bP2IsValid){
			nAns = 1;
		}
		//Both are valid
		else if (pii1->dtAttached > pii2->dtAttached) {
			nAns = 1;
		} else if (pii1->dtAttached < pii2->dtAttached) {
			nAns = -1;
		} else {
			nAns = 0;
		}
		break;
	}
	//(e.lally 2012-04-16) PLID 39543 - Added service date
	case CPhotoViewerCtrl::dscServiceDate:
	{
		// Compare by service date
		//(e.lally 2012-04-16) PLID 39543 - Technically service date can be null in data, account for that here
		bool bP1IsValid = (pii1->dtService.m_dt > 0 && pii1->dtService.m_status == COleDateTime::valid);
		bool bP2IsValid = (pii2->dtService.m_dt > 0 && pii2->dtService.m_status == COleDateTime::valid);
		if(!bP1IsValid && !bP2IsValid){
			//Both invalid, consider equal
			nAns = 0;
		}
		else if(!bP1IsValid){
			nAns = -1;
		}
		else if(!bP2IsValid){
			nAns = 1;
		}
		//Both are valid
		else if (pii1->dtService > pii2->dtService) {
			nAns = 1;
		} else if (pii1->dtService < pii2->dtService) {
			nAns = -1;
		} else {
			nAns = 0;
		}
		break;
	}
	case CPhotoViewerCtrl::dscNote:
		// Compare by note
		nAns = pii1->strNotes.CompareNoCase(pii2->strNotes);
		break;
	case CPhotoViewerCtrl::dscCategory:
		// Compare by category
		// First compare on EImageSource type
		if (pii1->nType > pii2->nType) {
			nAns = 1;
		} else if (pii1->nType < pii2->nType) {
			nAns = -1;
		} else {
			// The types are equivalent, so now compare on category (except, empty string 
			// should be considered "greater than" any string)
			if (pii1->strCategory.IsEmpty()) {
				if (pii2->strCategory.IsEmpty()) {
					// Both are empty, consider them equivalent
					nAns = 0;
				} else {
					// pii1 is empty and pii2 is not.  So pii1 is greater (see comment above)
					nAns = 1;
				}
			} else if (pii2->strCategory.IsEmpty()) {
				// pii1 is non-empty and pii2 is empty.  So pii1 is less (see comment above)
				nAns = -1;
			} else {
				// Both strings are good, just do the compare
				nAns = pii1->strCategory.CompareNoCase(pii2->strCategory);
			}
		}
		break;
	case CPhotoViewerCtrl::dscProcedureName:

		// (j.jones 2009-10-13 09:07) - PLID 35894 - modified code to handle
		// multiple procedures

		// Compare by procedure name
		// By definition, something with a procedure name comes before something without
		if (pii1->arynProcedureIDs.GetSize() == 0) {
			// pii1 is non-procedural, see if pii2 is
			if (pii2->arynProcedureIDs.GetSize() == 0) {
				// Both non-procedural.  So since we're comparing on procedure name, pii1 
				// and pii2 must be considered equivalent
				nAns = 0;
			} else {
				// pii1 is non-procedural and pii1 is procedural, so that's a "by 
				// definition" easy answer: pii1 comes after pii2.
				nAns = 1;
			}
		} else if (pii2->arynProcedureIDs.GetSize() == 0) {
			// pii1 is procedural and pii2 is not, so that's a "by definition" easy 
			// answer: pii1 comes before pii2.
			nAns = -1;
		} else {
			// They're both procedural, so compare on the procedure name
			nAns = pii1->strProcedureNames.CompareNoCase(pii2->strProcedureNames);
		}
		break;
	default:
		// Unknown criteria
		ASSERT(FALSE);
		nAns = 0;
	}

	
	// We now know our basic answer, but if we've been asked to compare in a "descending" way, we need to reverse the answer.
	if (bSortAscending) {
		return nAns;
	} else {
		return -nAns;
	}
}

// Utility function used only by SortArray__FindFirst()
// Compares the two given ImageInfo objects based on the given sort criteria and orders.
// Returns 0 if pii1 and pii2 are equivalent based on the criteria and orders, returns 1 if 
// pii1 is considered "later than" pii2, and returns -1 if pii1 is considered "before" pii2.
// (a.walling 2010-12-20 16:05) - PLID 41917
int CompareImageInfo(const ImageInformationPtr& pii1, const ImageInformationPtr& pii2, const long nSortOrderAndCriteriaCount, const CPhotoViewerCtrl::EDisplaySortCriteria *aryedscSortCriteria, const BOOL *arybSortAscending)
{
	for (long i=0; i<nSortOrderAndCriteriaCount; i++) {
		// Compare the two elements using JUST this comparison criteria and order
		int nCurCmp = CompareImageInfoSingle(pii1, pii2, aryedscSortCriteria[i], arybSortAscending[i]);
		if (nCurCmp != 0) {
			// Excellent, we found a difference.  That means we're done.
			return nCurCmp;
		}
	}
	// If we made it here, we were unable to distinguish between pii1 and 
	// pii2.  Thus we consider them to be equivalent so return 0.
	return 0;
}

// Utility function used only by CPhotoViewerCtrl::SortArray()
// Takes an array of ImageInfo objects and finds the index of the "first" one, according to the given 
// sort order and criteria.  Returns the index of the "first" element in the array.
// NOTE: DO NOT pass an empty array, as this function assumes there is at least one entry.
// (j.jones 2009-10-13 13:13) - PLID 35894 - converted the array to be pointers
// (a.walling 2010-12-20 16:05) - PLID 41917
int SortArray__FindFirst(CArray<ImageInformationPtr, ImageInformationPtr> &aryImages, const int nStartIndex, const long nSortOrderAndCriteriaCount, const CPhotoViewerCtrl::EDisplaySortCriteria *aryedscSortCriterion, const BOOL *arybSortAscending)
{
	// Find the "first" image (according to the given sort criteria and order)
	
	// Start out assuming the 0th element is the "first"
	int nFirstIndex = nStartIndex;
	// (a.walling 2010-12-20 16:05) - PLID 41917
	ImageInformationPtr piiFirst = aryImages.ElementAt(nFirstIndex);
	// Now loop through the rest, and compare each to our running "first" 
	// item.  If you find one that compares as "less than" the running 
	// "first", then track that as the new "first" item.
	long nCount = aryImages.GetSize();
	for (int i = nFirstIndex + 1; i < nCount; i++) {
		// Get this element
		ImageInformationPtr pii = aryImages.ElementAt(i);
		// Compare it to our running "first" element
		if (CompareImageInfo(pii, piiFirst, nSortOrderAndCriteriaCount, aryedscSortCriterion, arybSortAscending) < 0) {
			// This element is strictly "less than" our "first" element, so this is our new "first".
			piiFirst = pii;
			nFirstIndex = i;
		}
	}
	// When we're done looping, whatever was left as our running "first" 
	// is now officially THE one and only "first" element.
	return nFirstIndex;
}

// (b.cardillo 2005-06-28 16:02) - PLID 16454 - This now sorts based on the criteria and sort orders 
// stored in our member variables.
void CPhotoViewerCtrl::SortArray()
{
	//There's probably a better algorithm for this, but I don't care.
	
	// (b.cardillo 2005-06-28 11:29) - PLID 16454 - Actually, this was (roughly) the "insertion sort" 
	// algorithm, which is one of the most efficient O(n2) sorts out there.  Doesn't compare with the 
	// O(nlogn) ones, but we really don't need that kind of performance in a situation where we're 
	// only ever sorting a limited number of elements.  But the real problems in this implementation 
	// were cramping the elegance of the "insertion sort" and (therefore) hurting efficiency, so 
	// rather than changing the algorithm I've only eliminated those inefficiencies, and of course 
	// (per PLID 16454) updated the comparison mechanism to respect the sort settings.  Here are the 
	// efficiency improvements I made:
	//    1. made it use ElementAt() in several places instaed of GetAt(), which helps reduce the 
	//      number of times the structure is copied, 
	//    2. made it only start using the seperate array when it finds an element out of place, so 
	//      if the elements are already sorted properly, we'll never use arSorted, and 
	//    3. the first time arSorted is used, I made it set its size to the final size that it WILL 
	//      ultimately need, which makes it so we never have to re-allocate the array to grow it.
	// These changes don't affect the nature of the algorithm itself, but at least they improve the 
	// practical performance a bit.  For improvement 3 above, I chose to break it into two loops, but 
	// the basic algorithm is still the same.

	// First loop: increment i until we find an element that's not in the right spot
	int i=0;
	int nFirstIndex;
	int nCount = m_arImages.GetSize();
	for (; i<nCount; i++) {
		// Find the "first" image (compare according to our criteria and order)
		nFirstIndex = SortArray__FindFirst(m_arImages, i, m_nSortInfoCount, m_aryedscSortCriterion, m_arybSortAscending);
		if (nFirstIndex != i) {
			// The i'th element should actually be replaced with the nFirstIndex'th element.  So now 
			// we know, our original list is NOT in the right order.  Break out of the loop, and let 
			// the second loop take it from here.
			break;
		}
	}

	// Second loop: may or may not be necessary; if we didn't make it all the way through the first 
	// loop, that means we'll need our second array; so allocate it and then place all the remaining 
	// elements into it from the official array, then once their in the right order, move them back.
	if (i < m_arImages.GetSize()) {
		// Set arSorted to "grow by" the size we now know we're going to need (which is our 
		// total array size minus the first i elements, because those were already in the right 
		// spot so we won't have to move them from the original array to the sorted array).
		// (j.jones 2009-10-13 12:27) - PLID 35894 - changed to be an array of pointers
		// (a.walling 2010-12-20 16:05) - PLID 41917
		CArray<ImageInformationPtr, ImageInformationPtr> arSorted;
		arSorted.SetSize(0, m_arImages.GetSize() - i); // Setting the growby here, not the size
		// Notice i doesn't change in this loop, it's the array size that changes
		while (true) {
			// We know which entry from m_arImages is the "first".  Add it in 
			// the next spot in our sorted array.
			arSorted.Add(m_arImages.ElementAt(nFirstIndex));
			// And remove it from the original array (do not delete the object)
			m_arImages.RemoveAt(nFirstIndex);
			// Now that we've moved that "first" from m_arImages to arSorted, we now search again for 
			// the new "first" from what's left in m_arImages (if there's anything left in m_arImages)
			if (i < m_arImages.GetSize()) {
				// There's something left, search it
				nFirstIndex = SortArray__FindFirst(m_arImages, i, m_nSortInfoCount, m_aryedscSortCriterion, m_arybSortAscending);
			} else {
				// No more elements left to search
				break;
			}
		}
		
		// Ok, we've now reduced our official list down to just the first x elements that were already 
		// in the right spots, and we've put the remainder of the elements in the right order into a 
		// separate array, so all we have to do is append THAT array to the end of our official one.
		m_arImages.Append(arSorted);
	}
}

// This puts edscCriterion first, gives it an order of bAscending, and shifts 
// everything else down.  If edscCriterion is already first and in bAscending 
// order, this function has no effect.
// Returns TRUE if something was changed, otherwise FALSE.
BOOL CPhotoViewerCtrl::SetPrimarySort(IN const EDisplaySortCriteria edscCriterion, IN const BOOL bAscending)
{
	// Find the existing position of edscCriterion, if it exists
	short nExistingPos = -1;
	{
		for (short i=0; i<m_nSortInfoCount; i++) {
			if (m_aryedscSortCriterion[i] == edscCriterion) {
				nExistingPos = i;
				break;
			}
		}
	}

	// Now if it's already in the 0 spot, all we have to do is make sure the bAscending 
	// matches.  Otherwise, we have to shift things around.
	if (nExistingPos == 0) {
		// It's already in the top slot.  See about the ascending-ness.
		if (m_arybSortAscending[0] && !bAscending || !m_arybSortAscending[0] && bAscending) {
			// We've got to change the order
			m_arybSortAscending[0] = bAscending ? TRUE : FALSE;
			// And since we've changed something, we have to tell the caller so
			// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
			m_bNeedsSort = true;
			return TRUE;
		} else {
			// We're already clean, so we have nothing to do.
			return FALSE;
		}
	} else {
		// We know we're going to have to shift some stuff.  See how much.
		if (nExistingPos == -1) {
			// We are going to have to add an element
			if (m_nSortInfoCount < DSC_COUNT) {
				nExistingPos = m_nSortInfoCount;
				m_nSortInfoCount++;
			} else {
				// This should be impossible, because if DSC_COUNT is correct and there's only 
				// ever 0 or 1 occurrence of any given EDisplaySortCriteria value, then how 
				// could m_nSortInfoCount ever exceed DSC_COUNT?
				ASSERT(m_nSortInfoCount < DSC_COUNT);
				// We'll just have to drop the last one
				nExistingPos = m_nSortInfoCount - 1;
			}
		}
		// Now between 0 and nExistingPos, shift everything one slot, thus emptying the 0 slot
		for (long i = nExistingPos; i > 0; i--) {
			m_aryedscSortCriterion[i] = m_aryedscSortCriterion[i-1];
			m_arybSortAscending[i] = m_arybSortAscending[i-1];
		}
		// And now that the 0 spot is free, we put in the given parameters
		m_aryedscSortCriterion[0] = edscCriterion;
		m_arybSortAscending[0] = bAscending ? TRUE : FALSE;
		// And return TRUE because we definitely changed something
		// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
		m_bNeedsSort = true;
		return TRUE;
	}
}

// This puts edscCriterion first, and leaves the order the same if it was 
// already in the list.  If it wasn't already in the list, it defaults the 
// sort order to TRUE (ascending).
// Returns FALSE iff edscCriterion was already the primary sort criterion.
BOOL CPhotoViewerCtrl::SetPrimarySortCriterion(IN const EDisplaySortCriteria edscCriterion)
{
	// Find the existing position of edscCriterion, if it exists
	short nExistingPos = -1;
	{
		for (short i=0; i<m_nSortInfoCount; i++) {
			if (m_aryedscSortCriterion[i] == edscCriterion) {
				nExistingPos = i;
				break;
			}
		}
	}

	// Now if it's already in the 0 spot, all we have to do is make sure the bAscending 
	// matches.  Otherwise, we have to shift things around.
	if (nExistingPos == 0) {
		// It's already in the top slot.  Nothing to do.
		return FALSE;
	} else {
		// We know we're going to have to shift some stuff.  See how much.
		BOOL bAscending;
		if (nExistingPos == -1) {
			// We are going to have to add an element
			if (m_nSortInfoCount < DSC_COUNT) {
				nExistingPos = m_nSortInfoCount;
				m_nSortInfoCount++;
			} else {
				// This should be impossible, because if DSC_COUNT is correct and there's only 
				// ever 0 or 1 occurrence of any given EDisplaySortCriteria value, then how 
				// could m_nSortInfoCount ever exceed DSC_COUNT?
				ASSERT(m_nSortInfoCount < DSC_COUNT);
				// We'll just have to drop the last one
				nExistingPos = m_nSortInfoCount - 1;
			}
			// It's new to the list, so default the order to ascending
			bAscending = TRUE;
		} else {
			// We are using an existing element
			bAscending = m_arybSortAscending[nExistingPos];
		}
		// Now between 0 and nExistingPos, shift everything one slot, thus emptying the 0 slot
		for (long i = nExistingPos; i > 0; i--) {
			m_aryedscSortCriterion[i] = m_aryedscSortCriterion[i-1];
			m_arybSortAscending[i] = m_arybSortAscending[i-1];
		}
		// And now that the 0 spot is free, we put in the given parameters
		m_aryedscSortCriterion[0] = edscCriterion;
		m_arybSortAscending[0] = bAscending ? TRUE : FALSE;
		// And return TRUE because we definitely changed something
		// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
		m_bNeedsSort = true;
		return TRUE;
	}
}

// This takes the current primary sort criterion and gives it a sort order of 
// bAscending.
// Returns FALSE iff there was no primary sort criterion or it was already 
// sorted in bAscending order.
BOOL CPhotoViewerCtrl::SetPrimarySortOrder(IN const BOOL bAscending)
{
	if (m_nSortInfoCount > 0) {
		if (bAscending) {
			if (m_arybSortAscending[0]) {
				// Already the same
				return FALSE;
			} else {
				// Need to change
				m_arybSortAscending[0] = TRUE;
				// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
				m_bNeedsSort = true;
				return TRUE;
			}
		} else {
			if (!m_arybSortAscending[0]) {
				// Already the same
				return FALSE;
			} else {
				// Need to change
				m_arybSortAscending[0] = FALSE;
				// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
				m_bNeedsSort = true;
				return TRUE;
			}
		}
	} else {
		// Nothing to check
		return FALSE;
	}
}

// Retrieves the current primary sort criterion
CPhotoViewerCtrl::EDisplaySortCriteria CPhotoViewerCtrl::GetPrimarySortCriterion()
{
	if (m_nSortInfoCount > 0) {
		return m_aryedscSortCriterion[0];
	} else {
		return DSC_INVALID;
	}
}

// Retrieves the current primary sort order (TRUE is ascending)
BOOL CPhotoViewerCtrl::GetPrimarySortOrder()
{
	if (m_nSortInfoCount > 0) {
		if (m_arybSortAscending[0]) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return -2;
	}
}

void CPhotoViewerCtrl::OnDestroy()
{
	for(int i = 0; i < m_parRgnsToDelete.GetSize(); i++) {
		if(m_parRgnsToDelete.GetAt(i)) {
			//TES 12/13/2007 - PLID 28257 - We need to clean up any CRgns we created.
			((CRgn*)m_parRgnsToDelete.GetAt(i))->DeleteObject();
			delete (CRgn*)m_parRgnsToDelete.GetAt(i);
		}
	}

	try {
		// (a.walling 2008-09-23 11:43) - PLID 31479 - AbortImageLoading will clear the list
		AbortImageLoading();
		// (a.walling 2010-12-20 16:05) - PLID 41917 - Got rid of some old, commented-out code
	} NxCatchAll("Error cleaning up image loader");

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

CRect CPhotoViewerCtrl::DrawGhostlyImage(CDC *pDC, HBITMAP hImage, CPoint ptUpperLeft)
{
	CBorrowDeviceContext bdcCntrl(pDC);

	if(!m_pGhostlyImageWnd) {
		m_pGhostlyImageWnd = new CFloatingImageWnd;
		m_pGhostlyImageWnd->Create(NULL, "", WS_VISIBLE|WS_OVERLAPPED, CRect(ptUpperLeft.x, ptUpperLeft.y, ptUpperLeft.x+IMAGE_SIZE, ptUpperLeft.y+IMAGE_SIZE), GetMainFrame(), 100);
	}
	if(!m_hScreenshot) {
		//Take a screenshot, before the ghostly image is on screen.
		CRect rClient;
		GetClientRect(rClient);
		CBitmap bmpScreen;
		CDC dcScreen;
		dcScreen.CreateCompatibleDC(pDC);
		bmpScreen.CreateCompatibleBitmap(pDC, rClient.Width(), rClient.Height());
		HGDIOBJ bmpScreenOld = dcScreen.SelectObject(bmpScreen.GetSafeHandle());
		BitBlt(dcScreen.GetSafeHdc(), 0, 0, rClient.Width(), rClient.Height(), pDC->GetSafeHdc(), 0, 0, SRCCOPY);
		dcScreen.SelectObject(bmpScreenOld);
		dcScreen.DeleteDC();
		m_hScreenshot = (HBITMAP)bmpScreen.Detach();
		BITMAP bm;
		GetObject(m_hScreenshot, sizeof(BITMAP), &bm);
		bm.bmWidth;
		
	}
	BITMAP bm;
	GetObject(m_hScreenshot, sizeof(BITMAP), &bm);
	bm.bmWidth;

	//Let's figure out our actual upper left.
	CRect rClient = GetDisplayArea(daMainList);
	if(ptUpperLeft.x < rClient.left) ptUpperLeft.x = rClient.left;
	if(ptUpperLeft.x + IMAGE_SIZE > rClient.right) ptUpperLeft.x = rClient.right - IMAGE_SIZE;
	if(ptUpperLeft.y < rClient.top) ptUpperLeft.y = rClient.top;
	if(ptUpperLeft.y + IMAGE_SIZE > rClient.bottom) ptUpperLeft.y = rClient.bottom - IMAGE_SIZE;

	CBorrowDeviceContext bdcWnd(m_pGhostlyImageWnd);
	//OK, here's what we need to do: 1.)draw the image on one memory dc, then 2.) draw the correct area of our stored screenshot
	//onto another memory dc, 3.) blend the image onto that one (we can't have the bitmap be the destination, because it will 
	//modify hImage), and 4.) pass the resulting image into our window.
	
	CDC dcBmp, dcScreen, dcRect;
	CBitmap bmRect;
	HGDIOBJ bmOld1, bmOld2, bmOld3;
	
	//1.) 
	dcBmp.CreateCompatibleDC(bdcWnd.m_pDC);
	bmOld1 = dcBmp.SelectObject(hImage);
	
	//2.) 
	dcScreen.CreateCompatibleDC(pDC);
	bmOld2 = dcScreen.SelectObject(m_hScreenshot);
	
	dcRect.CreateCompatibleDC(pDC);
	bmRect.CreateCompatibleBitmap(bdcWnd.m_pDC, IMAGE_SIZE,IMAGE_SIZE);
	bmOld3 = dcRect.SelectObject(bmRect.GetSafeHandle());
	BitBlt(dcRect.GetSafeHdc(), 0,0,IMAGE_SIZE,IMAGE_SIZE, dcScreen.GetSafeHdc(), ptUpperLeft.x, ptUpperLeft.y, SRCCOPY);
	
	//3.) 
	BLENDFUNCTION bf;
	bf.AlphaFormat = 0;
	bf.BlendFlags = 0;
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 127;
	m_gdi32.AlphaBlend(dcRect.GetSafeHdc(), 0, 0, IMAGE_SIZE, IMAGE_SIZE, dcBmp.GetSafeHdc(), 0,0, IMAGE_SIZE, IMAGE_SIZE, bf);
	//AlphaBlend(dcRect.GetSafeHdc(), 0, 0, IMAGE_SIZE, IMAGE_SIZE, dcBmp.GetSafeHdc(), 0,0, IMAGE_SIZE, IMAGE_SIZE, bf);

	dcBmp.SelectObject(bmOld1);
	dcBmp.DeleteDC();
	dcScreen.SelectObject(bmOld2);
	dcScreen.DeleteDC();
	dcRect.SelectObject(bmOld3);
	dcRect.DeleteDC();
	
	//4.) 
	m_pGhostlyImageWnd->m_hImage = (HBITMAP)bmRect.GetSafeHandle();
	CPoint pt = ptUpperLeft;
	ClientToScreen(&pt);
	GetMainFrame()->ScreenToClient(&pt);
	CRect rWindowPos(pt.x, pt.y, pt.x+IMAGE_SIZE, pt.y+IMAGE_SIZE);
	m_pGhostlyImageWnd->MoveWindow(&rWindowPos);
	m_pGhostlyImageWnd->RedrawWindow();
	
	GetMainFrame()->ClientToScreen(&rWindowPos);
	ScreenToClient(&rWindowPos);
	return rWindowPos;
}

// (j.jones 2009-10-13 09:35) - PLID 35894 - converted to run by procedure name(s)
void CPhotoViewerCtrl::InvalidateProcedure(CString strProcedureNames)
{
	CBorrowDeviceContext bdc(this);
	
	//Invalidate every image in the procedure.
	for(int i = 0; i < m_arImages.GetSize(); i++) {
		if(m_arImages.GetAt(i)->strProcedureNames == strProcedureNames) {
			InvalidateImage(i);
		}
	}

	//Redraw the frame.
	for(i = 0; i < m_arProcNameRects.GetSize(); i++) {
		if(m_arProcNameRects.GetAt(i)->strProcedureNames == strProcedureNames) {
			//First, find the rectangle the text would be drawn in if it were bold, and clean that off.
			{
				CRect rBox;
				int nRgnType = m_arProcNameRects.GetAt(i)->pRgn->GetRgnBox(&rBox);
				CPoint ptUpperLeft(rBox.left+1, rBox.top+1);
				//Now, move right until we're actually in the region.
				while(!m_arProcNameRects.GetAt(i)->pRgn->PtInRegion(ptUpperLeft) && ptUpperLeft.x <= rBox.right) {
					ptUpperLeft.x++;
				}
				//OK, now we have the upper left.  Let's offset, say, five pixels.
				ptUpperLeft.x += 5;
				bdc.m_pDC->SelectObject(m_pIconFont);
				LOGFONT lf;
				m_pIconFont->GetLogFont(&lf);
				lf.lfWeight = FW_BOLD;
				CFont fBold;
				fBold.CreateFontIndirect(&lf);
				bdc.m_pDC->SelectObject(fBold);
				ptUpperLeft.y--;
				CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
				bdc.m_pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
				if(rText.right > rBox.right) {
					rText.right = rBox.right;
				}
				bdc.m_pDC->FillSolidRect(rText, RGB(255,255,255));
			}

			//First frame white to eliminate remnants.
			CBrush brWhite(RGB(255,255,255));
			bdc.m_pDC->FrameRgn(m_arProcNameRects.GetAt(i)->pRgn, &brWhite, 2, 2);
			
			int nThickness = strProcedureNames == m_strCurrentHotProcedures ? 2 : 1;
			CBrush brBlack(RGB(0,0,0));
			bdc.m_pDC->FrameRgn(m_arProcNameRects.GetAt(i)->pRgn, &brBlack, nThickness, nThickness);

			//Now, find the "upper left" of this region, to draw the name.
			CRect rBox;
			int nRgnType = m_arProcNameRects.GetAt(i)->pRgn->GetRgnBox(&rBox);
			CPoint ptUpperLeft(rBox.left+1, rBox.top+1);
			//Now, move right until we're actually in the region.
			while(!m_arProcNameRects.GetAt(i)->pRgn->PtInRegion(ptUpperLeft) && ptUpperLeft.x <= rBox.right) {
				ptUpperLeft.x++;
			}
			//OK, now we have the upper left.  Let's offset, say, five pixels.
			ptUpperLeft.x += 5;
			//How tall should our rect be?
			bdc.m_pDC->SelectObject(m_pIconFont);
			LOGFONT lf;
			m_pIconFont->GetLogFont(&lf);
			if(strProcedureNames == m_strCurrentHotProcedures) {
				lf.lfWeight = FW_BOLD;
				CFont fBold;
				fBold.CreateFontIndirect(&lf);
				bdc.m_pDC->SelectObject(fBold);
			}
			ptUpperLeft.y--;
			CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
			bdc.m_pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
			if(rText.right > rBox.right) rText.right = rBox.right;
			bdc.m_pDC->FillSolidRect(rText, RGB(255,255,255));
			bdc.m_pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, &rText, DT_SINGLELINE);
			
			if(nRgnType == COMPLEXREGION) {
				//This region wrapped a line, draw a second label on the next line.  Same algorithm as before, but moving
				//down instead of right.
				ptUpperLeft = CPoint(rBox.left+1, rBox.top+1);
				//Now, move down until we're actually in the region.
				while(!m_arProcNameRects.GetAt(i)->pRgn->PtInRegion(ptUpperLeft) && ptUpperLeft.y <= rBox.bottom) {
					ptUpperLeft.y++;
				}
				//OK, now we have the upper left.  Let's offset, say, five pixels.
				ptUpperLeft.x += 5;
				//How tall should our rect be?
				bdc.m_pDC->SelectObject(m_pIconFont);
				LOGFONT lf;
				m_pIconFont->GetLogFont(&lf);
				if(strProcedureNames == m_strCurrentHotProcedures) {
					lf.lfWeight = FW_BOLD;
					CFont fBold;
					fBold.CreateFontIndirect(&lf);
					bdc.m_pDC->SelectObject(fBold);
				}
				CRect rText(ptUpperLeft.x, ptUpperLeft.y-BUFFER_VERT/2-lf.lfHeight/2, ptUpperLeft.x+1, ptUpperLeft.y-BUFFER_VERT/2+lf.lfHeight/2);
				bdc.m_pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, &rText, DT_CALCRECT|DT_SINGLELINE);
				if(rText.right > rBox.right) {
					rText.right = rBox.right;
				}
				bdc.m_pDC->FillSolidRect(rText, RGB(255,255,255));
				bdc.m_pDC->DrawText(m_arProcNameRects.GetAt(i)->strProcedureNames, &rText, DT_SINGLELINE);
			}
		}
	}
}

BOOL CPhotoViewerCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/1/2005 - PLID 17125 - Modified the way IDs are retrieved to be in a new format (see the code
	//	that generates the IDs).  Applied that minor change to the procedures section.
	//Also newly added the category section of code.
	try {

		if(HIWORD(wParam) == ID_ASSIGN_PROC_BASE) {
			long nProcID = LOWORD(wParam);
			_RecordsetPtr rsProcName = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nProcID);
			if(!rsProcName->eof) {
				CString strProcName = AdoFldString(rsProcName, "Name");
				rsProcName->Close();
				for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
					if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {

						// (j.jones 2009-10-12 15:51) - PLID 35894 - supported multiple procedures
						long nMailID = m_listImagesSelected.GetAt(p)->nID;
						ExecuteParamSql("DELETE FROM MailSentProcedureT WHERE MailSentID = {INT}", nMailID);
						if(nProcID != -1) {
							//(s.dhole 10/9/2014 11:18 AM ) - PLID  37718 Try to insert MailSentProcedureT , if records exist in MailSent table
							ExecuteParamSql("INSERT INTO MailSentProcedureT (MailSentID, ProcedureID) Select MailID,  {INT} AS ProcedureID FROM  MailSent  Where MailID =  {INT}  ", nProcID, nMailID);
						}

						// (j.jones 2012-07-12 17:37) - PLID 49636 - send a tablechecker
						// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
						CClient::RefreshMailSentTable(m_nPersonID, nMailID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

						// (j.jones 2009-10-14 11:33) - PLID 35894 - changing procedures requires ReloadImage to be called
						int nIndex = -1;
						for(int i=0; i<m_arImages.GetSize() && nIndex == -1; i++) {
							if(m_arImages.GetAt(i)->nID == m_listImagesSelected.GetAt(p)->nID && m_arImages.GetAt(i)->nType == m_listImagesSelected.GetAt(p)->nType) {
								nIndex = i;
							}
						}
						if(nIndex != -1) {
							ReloadImage(nIndex);
							//ReloadImage will ensure the image remains selected
						}

						// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
						m_bNeedsSort = true;
					}
				}
				Invalidate();
			}
		}
		else if(HIWORD(wParam) == ID_ASSIGN_CATEGORY_BASE) {
			//The category ID is the last 16 bits of the parameter
			long nCatID = LOWORD(wParam);

			_RecordsetPtr rsCatName = CreateRecordset("SELECT Description FROM NoteCatsF WHERE ID = %li", nCatID);
			CString strCatName;
			if(!rsCatName->eof) {
				strCatName = AdoFldString(rsCatName, "Description");
				rsCatName->Close();
			}
			//else catname will be empty string

			for(POSITION p = m_listImagesSelected.GetHeadPosition(); p;  m_listImagesSelected.GetNext(p)) {
				if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
					//Update data to the new category
					CString strCat = "NULL";
					if(nCatID != 0)
						strCat.Format("%li", nCatID);
					ExecuteSql("UPDATE MailSent SET CategoryID = %s WHERE MailID = %li", strCat, m_listImagesSelected.GetAt(p)->nID);

					// (j.jones 2012-07-12 17:37) - PLID 49636 - send a tablechecker
					// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
					CClient::RefreshMailSentTable(m_nPersonID, m_listImagesSelected.GetAt(p)->nID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

					//Update with the new category name
					// (a.walling 2010-12-20 16:05) - PLID 41917
					ImageInformationPtr ii = (ImageInformationPtr)m_listImagesSelected.GetAt(p);
					ii->strCategory = strCatName;
					//(e.lally 2012-04-25) PLID 49637 - Force text to recalculate box size
					ii->nNotesHeight = -1;

					// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
					m_bNeedsSort = true;
				}
			}

			//DRT PLID 17125 - Additionally, if we are filtering on a certain category right now, we need to Refresh if
			//	the user changed the category of any image (because it may no longer meet our criteria).
			if(m_bUsingCategoryFilter) {
				Refresh();
			}
			else {
				//If we aren't filtering, then there's no reason to refresh.  Just call invalidate.
				Invalidate();
			}
		}

	} NxCatchAll("Error in CPhotoViewerCtrl::OnCommand");

	return CWnd::OnCommand(wParam, lParam);
}

void CPhotoViewerCtrl::OnDetach()
{
	try {

		// (j.jones 2007-04-19 17:05) - PLID 25425 - check the delete permission
		if(!CheckCurrentUserPermissions(bioPatientHistory, sptDelete))
			return;

		//How many Nextech items are selected?
		// (m.hancock 2006-06-30 14:06) - PLID 21307 - Added code to assemble a list of MailIDs to check for association with Lab records
		CString strMailIDs;
		int nSelectedCount = 0;
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
			if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
				nSelectedCount++;
				// (m.hancock 2006-06-30 14:09) - PLID 21307 - assemble a list of MailIDs
				CString strTemp;
				strTemp.Format("%li", m_listImagesSelected.GetAt(p)->nID);
				if(strMailIDs.IsEmpty())
					strMailIDs = strTemp;
				else
					strMailIDs += ", " + strTemp;
			}
		}

		//TES 12/7/2007 - PLID 28285 - Check for there being nothing selected BEFORE calculating the prompt.  For one
		// thing, IsLabAttachment() assumes that strMailIDs is not empty, and for another, it's just wasted effort.
		if(nSelectedCount == 0) {
			AfxMessageBox("There are no detachable photos selected.");
			GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)FALSE);
			return;
		}
		
		// (m.hancock 2006-06-30 14:27) - PLID 21307 - Compare MailIDs against MailSent for associated Lab records and display message if necessary
		CString strPromptSingle = "Are you absolutely sure you want to detach the selected photo?";
		CString strPromptMultiple = "Are you absolutely sure you want to detach all selected photos?";
		BOOL bIsLabAttached = IsLabAttachment(strMailIDs);
		if(bIsLabAttached) {
			strPromptSingle = "This photo is associated with a lab record or lab result(s).\n"
				"If you detach the selected photo, the association with the lab will also be lost.\n\n"
				+ strPromptSingle;
			strPromptMultiple ="At least one of the selected photos is associated with at least a lab record or lab result(s).\n"
				"If you detach the selected photos, the association with the labs will also be lost.\n\n"
				+ strPromptMultiple;
		}

		if(nSelectedCount == 1) {
			if(IDYES != MsgBox(MB_YESNO, strPromptSingle)) return;
		}
		else {
			if(IDYES != MsgBox(MB_YESNO, strPromptMultiple)) return;
		}

		CString strPersonName = "";
		if(m_nPersonID != -1) {
			if(m_bIsPatient) {
				strPersonName = GetExistingPatientName(m_nPersonID);
			}
			else {
				//not a patient, so skip the GetExisting logic and go directly to SQL
				_RecordsetPtr rs = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS FullName FROM PersonT WHERE ID = {INT}", m_nPersonID);
				if(!rs->eof) {
					strPersonName = AdoFldString(rs, "FullName","");
				}
				rs->Close();
			}
		}

		//OK, let's do it.
		for(p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {

			// (c.haag 2009-10-26 17:17) - PLID 36019 - No sense in calling m_listImagesSelected.GetAt a dozen
			// times when we can just do it once and store it in a local variable.
			// (a.walling 2010-12-20 16:05) - PLID 41917
			ImageInformationPtr pImageInfo = m_listImagesSelected.GetAt(p);

			if(pImageInfo->nType == eImageSrcPractice) {

				CString strFullFilePath = pImageInfo->strFullFilePath;
				CString strMailSentFilePath = pImageInfo->strMailSentFilePath;
				CString strNotes = pImageInfo->strNotes;

				if(pImageInfo->nID == m_nPrimaryImageID) {
					ExecuteSql("UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PatPrimaryHistImage = %li", 
						pImageInfo->nID);
					SetPrimaryImage(-1);
					//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
					// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
					// this shared function will handle sending accordingly
					SendPatientPrimaryPhotoHL7Message(m_nPersonID);
				}

				// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2008-10-16 12:41) - PLID 31432 - clear the lab result connection
				CString strSqlBatch = BeginSqlBatch();
				long nAuditTransactionID = -1;
				if (bIsLabAttached) {
					CString strWhere;
					strWhere.Format("MailID = %li",pImageInfo->nID);
					nAuditTransactionID = HandleLabResultAudit(strWhere, GetExistingPatientName(m_nPersonID), m_nPersonID);
				}

				// (j.jones 2013-09-19 11:48) - PLID 58547 - audit this deletion
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//audit the note if the filename is empty
				CString strAudit;
				if (strMailSentFilePath.IsEmpty()) {
					strAudit = strNotes;
				} else {
					strAudit = strMailSentFilePath;
				}

				//the new value is the same regardless of whether it is detached, or also deleted
				AuditEvent(m_nPersonID, strPersonName, nAuditTransactionID, m_bIsPatient ? aeiPatientDocDetach : aeiContactDocDetach, m_nPersonID, strAudit, "<Detached>", aepHigh, aetDeleted);

				// (c.haag 2009-05-06 13:26) - PLID 33789 - Also clear out the Units field
				AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT Set MailID = NULL, Value = '', Units = '' WHERE MailID = %li",pImageInfo->nID);
				// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
				AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("%li", pImageInfo->nID));
				try {
					ExecuteSqlBatch(strSqlBatch);
					if (nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
					}
				}NxCatchAllSilentCallThrow(
					if (nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
						nAuditTransactionID = -1;
					}
				);

				// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
				CClient::RefreshMailSentTable(m_nPersonID, pImageInfo->nID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

				// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
				// we have to send those as well.
				CClient::RefreshTable(NetUtils::TodoList, -1);
				// (r.gonet 09/02/2014) - PLID 63221 - Don't send a tablechecker for labs. The mailsent one takes care of things on its own.
			}
		}
		Refresh();
	}NxCatchAll("Error in CPhotoViewerCtrl::OnDetach()");
}


void CPhotoViewerCtrl::OnDetachAndDelete()
{
	try {

		// (j.jones 2007-04-19 17:05) - PLID 25425 - check the delete permission
		if(!CheckCurrentUserPermissions(bioPatientHistory, sptDelete))
			return;

		//How many Nextech items are selected?
		// (m.hancock 2006-06-30 14:06) - PLID 21307 - Added code to assemble a list of MailIDs to check for association with Lab records
		CString strMailIDs;
		int nSelectedCount = 0;
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
			if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
				nSelectedCount++;
				// (m.hancock 2006-06-30 14:09) - PLID 21307 - assemble a list of MailIDs
				CString strTemp;
				strTemp.Format("%li", m_listImagesSelected.GetAt(p)->nID);
				if(strMailIDs.IsEmpty())
					strMailIDs = strTemp;
				else
					strMailIDs += ", " + strTemp;
			}
		}

		//TES 12/7/2007 - PLID 28285 - Check for there being nothing selected BEFORE calculating the prompt.  For one
		// thing, IsLabAttachment() assumes that strMailIDs is not empty, and for another, it's just wasted effort.
		if(nSelectedCount == 0) {
			AfxMessageBox("There are no detachable photos selected.");
			GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)FALSE);
			return;
		}

		// (m.hancock 2006-06-30 14:27) - PLID 21307 - Compare MailIDs against MailSent for associated Lab records and display message if necessary
		CString strPromptSingle = "Are you absolutely sure you want to permanently delete the selected photo?";
		CString strPromptMultiple = "Are you absolutely sure you want to permanently delete all selected photos?";
		BOOL bIsLabAttached = IsLabAttachment(strMailIDs);
		if(bIsLabAttached) {
			strPromptSingle = "This photo is associated with a lab record.\n"
				"If you delete the selected photo, the association with the lab will also be lost.\n\n"
				+ strPromptSingle;
			strPromptMultiple ="At least one of the selected photos is associated with at least one lab record.\n"
				"If you delete the selected photos, the association with the labs will also be lost.\n\n"
				+ strPromptMultiple;
		}

		if(nSelectedCount == 1) {
			if(IDYES != MsgBox(MB_YESNO, strPromptSingle)) return;
		}
		else {
			if(IDYES != MsgBox(MB_YESNO, strPromptMultiple)) return;
		}

		CString strPersonName = "";
		if(m_nPersonID != -1) {
			if(m_bIsPatient) {
				strPersonName = GetExistingPatientName(m_nPersonID);
			}
			else {
				//not a patient, so skip the GetExisting logic and go directly to SQL
				_RecordsetPtr rs = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS FullName FROM PersonT WHERE ID = {INT}", m_nPersonID);
				if(!rs->eof) {
					strPersonName = AdoFldString(rs, "FullName","");
				}
				rs->Close();
			}
		}

		//OK, let's do it.
		for(p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {

			// (c.haag 2009-10-26 17:17) - PLID 36019 - No sense in calling m_listImagesSelected.GetAt a dozen
			// times when we can just do it once and store it in a local variable.
			// (a.walling 2010-12-20 16:05) - PLID 41917
			ImageInformationPtr pImageInfo = m_listImagesSelected.GetAt(p);
			long nID = pImageInfo->nID;
			CString strFullFilePath = pImageInfo->strFullFilePath;
			CString strMailSentFilePath = pImageInfo->strMailSentFilePath;
			CString strNotes = pImageInfo->strNotes;

			if(pImageInfo->nType == eImageSrcPractice || pImageInfo->nType == eImageSrcDiagram) {
				// (j.jones 2013-09-19 12:16) - PLID 58547 - disallow deleting if the file is an image on an EMR,
				// which sometimes - but not always - has a \ in front of it

				//This image might be attached to multiple patients, or for a contact.
				//If this is just a patient-specific image, we should filter on that patient.
				bool bFileNameIsPerPatient = true;
				if(!m_bIsPatient || strMailSentFilePath.Find('\\') != -1 || strMailSentFilePath.Find("MultiPatDoc") != -1) {
					//the file name is a full path that is not specific to this patient
					bFileNameIsPerPatient = false;
				}

				//If the image is local to a patient documents folder, filter by patient, else look at all EMNs.
				//This code intentionally does not exclude deleted EMNs or details.
				if((bFileNameIsPerPatient && ReturnsRecordsParam("SELECT EMRDetailsT.ID FROM EMRDetailsT "
					"INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
					"WHERE EMRMasterT.PatientID = {INT} "
					"AND (EMRDetailsT.InkImagePathOverride = {STRING} OR EMRDetailsT.InkImagePathOverride = '\\' + {STRING})",
					m_nPersonID, strMailSentFilePath, strMailSentFilePath))
					||
					(!bFileNameIsPerPatient && ReturnsRecordsParam("SELECT EMRDetailsT.ID FROM EMRDetailsT "
					"WHERE EMRDetailsT.InkImagePathOverride = {STRING}", strMailSentFilePath))
					) {

					CString strWarn;
					strWarn.Format("The document '%s' is in use in an image item on at least one EMN. It cannot be deleted.", strMailSentFilePath);
					MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
					continue;
				}

				// (j.jones 2013-09-19 12:16) - PLID 58547 - check to see if it is use in this EMR,
				// but perhaps not saved yet
				if(m_pPicContainer) {
					if(m_pPicContainer->IsImageFileInUseOnEMR(strMailSentFilePath)) {
						CString strWarn;
						strWarn.Format("The document '%s' is in use in an image item in this EMR. It cannot be deleted.", strMailSentFilePath);
						MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
						continue;
					}
				}
			}

			if(pImageInfo->nType == eImageSrcPractice) {

				// (j.jones 2013-09-19 12:16) - PLID 58547 - warn if the file is attached to a PIC that is not the one we are currently in
				//this is ok if m_nPicID is -1
				if(ReturnsRecordsParam("SELECT MailID FROM MailSent WHERE MailID = {INT} "
					"AND ((PicID Is Not Null AND PicID <> {INT}) OR EMNID Is Not Null)", nID, m_nPicID)) {
					CString strWarn;
					strWarn.Format("The document '%s' is attached to the History of at least one PIC or EMN. Are you sure you wish to detach it?", strMailSentFilePath);
					if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						continue;
					}
				}

				//Is this file attached to multiple patients?  (Unlikely, but possible).
				// (c.haag 2004-01-28 11:29) - First make sure nobody else is attached to this file
				// (as is the case for multi-patient documents).
				//TES 3/1/2004: A file is attached to multiple patients if a.) it has a \ in the name (meaning it's not
				//relative to the patient's folder), or b.) it has "MultiPat" in the name (which, astonishingly, is the
				//only way to identify a file as belonging to the multi-patient folder).
				BOOL bDeleteMultiPat = FALSE;
				if(strMailSentFilePath.Find('\\') != -1 || strMailSentFilePath.Find("MultiPatDoc") != -1) {
					// (a.walling 2008-06-04 15:47) - PLID 29900 - Use the correct patient ID
					_RecordsetPtr prs = CreateRecordset("SELECT TOP 21 Last, First, Middle FROM MailSent INNER JOIN PersonT ON PersonT.ID = MailSent.PersonID WHERE PathName = '%s' AND PersonID <> %d",
						_Q(strMailSentFilePath), m_nPersonID);
					if (!prs->eof)
					{
						CString str, strNames;
						long nMaxNames = 20;
						strNames.Format("The image file '%s' is also attached to the following patients:\n\n",
							strMailSentFilePath);
						while (!prs->eof && nMaxNames > 0)
						{
							str.Format("%s, %s %s\n", AdoFldString(prs, "Last"),
								 AdoFldString(prs, "First"),
								 AdoFldString(prs, "Middle"));
							strNames += str;
							nMaxNames--;
							prs->MoveNext();
						}
						if (!prs->eof && nMaxNames == 0)
						{
							strNames += "<MORE NAMES OMITTED>\n";
						}
						prs->Close();
						strNames += "\n\nIf you proceed with detaching and deleting this image file, it will be detached from all the other patients, and there will be no way to determine the names of those patients after the image file is deleted! Are you ABSOLUTELY SURE you wish to detach and delete this image file?";
						if (IDNO == MsgBox(MB_YESNO, strNames))
							continue;
						bDeleteMultiPat = TRUE;
					}
				}
				if(DeleteFile(strFullFilePath)) {

					CString strSqlBatch = BeginSqlBatch();

					//OK, we deleted it, let's continue with the detaching.
					if(pImageInfo->nID == m_nPrimaryImageID) {
						AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PatPrimaryHistImage = %li", 
							pImageInfo->nID);
						SetPrimaryImage(-1);
						//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
						// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
						// this shared function will handle sending accordingly
						SendPatientPrimaryPhotoHL7Message(m_nPersonID);
					}
					
					// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
					// (j.gruber 2008-10-16 12:56) - PLID 31432 - LabResultsT
					long nAuditTransactionID = -1;
					if (bIsLabAttached) {
						CString strWhere;
						strWhere.Format("MailID = %li", pImageInfo->nID);
						nAuditTransactionID = HandleLabResultAudit(strWhere, GetExistingPatientName(m_nPersonID), m_nPersonID);
					}

					// (j.jones 2013-09-19 11:48) - PLID 58547 - audit this deletion
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					//audit the note if the filename is empty
					CString strAudit;
					if (strMailSentFilePath.IsEmpty()) {
						strAudit = strNotes;
					} else {
						strAudit = strMailSentFilePath;
					}

					//the new value is the same regardless of whether it is detached, or also deleted
					AuditEvent(m_nPersonID, strPersonName, nAuditTransactionID, m_bIsPatient ? aeiPatientDocDetach : aeiContactDocDetach, m_nPersonID, strAudit, "<Detached>", aepHigh, aetDeleted);

					// (c.haag 2009-05-06 13:26) - PLID 33789 - Also clear out the Units field
					AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID = %li",pImageInfo->nID);
					// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
					AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("%li", pImageInfo->nID));
					try {
						ExecuteSqlBatch(strSqlBatch);
						if (nAuditTransactionID != -1) {
							CommitAuditTransaction(nAuditTransactionID);
						}
					}NxCatchAllSilentCallThrow(
						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
							nAuditTransactionID = -1;
						}
					);
					
					// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
					CClient::RefreshMailSentTable(m_nPersonID, pImageInfo->nID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

					// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
					// we have to send those as well.
					CClient::RefreshTable(NetUtils::TodoList, -1);
					// (r.gonet 09/02/2014) - PLID 63221 - Don't send a tablechecker for labs. The mailsent one takes care of things on its own.

					if (bDeleteMultiPat)
					{
						// (j.gruber 2009-10-28 16:15) - PLID 36045 - make sure we have a filename
						if (strMailSentFilePath.IsEmpty()) {
							//this is an error condition, it should not be able to be blank if its a multipatdoc because it wasn't blank at the beginning of this function
							ThrowNxException("An error occurred while detaching the file from other patients, please attempt the process again.");
						}

						// (j.jones 2011-07-22 15:41) - PLID 21784 - get a list of the MailIDs affected (rarely more than one)
						CArray<long, long> aryMailIDs;
						CArray<long, long> aryPatientIDs;
						// (j.jones 2014-08-04 17:25) - PLID 63159 - need to load the patient IDs too
						_RecordsetPtr rs = CreateParamRecordset("SELECT MailID, PersonID FROM MailSent WHERE PathName = {STRING}", strMailSentFilePath);
						while(!rs->eof) {
							aryMailIDs.Add(VarLong(rs->Fields->Item["MailID"]->Value));
							aryPatientIDs.Add(VarLong(rs->Fields->Item["PersonID"]->Value));
							rs->MoveNext();
						}
						rs->Close();
						CString strIDs;
						for(int i=0; i<aryMailIDs.GetSize(); i++) {
							if(!strIDs.IsEmpty()) {
								strIDs += ",";
							}
							strIDs += AsString(aryMailIDs.GetAt(i));
						}

						CString strSqlBatch = BeginSqlBatch();

						// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
						// (j.gruber 2008-10-16 12:57) - PLID 31432 - LabResultsT
						long nAuditTransactionID = -1;
						if (bIsLabAttached) {
							CString strWhere;
							strWhere.Format(" MailID IN (%s) ", strIDs);
							nAuditTransactionID = HandleLabResultAudit(strWhere, GetExistingPatientName(m_nPersonID), m_nPersonID);
						}

						//TES 9/28/2015 - PLID 66192 - Track any patients who have their primary image changed, so we can update HL7
						CArray<long, long> arPatIDsToUpdate;
						// (j.jones 2013-09-19 11:48) - PLID 58547 - audit for every entry we are about to delete
						_RecordsetPtr rsMultiPat = CreateParamRecordset("SELECT MailSent.PersonID, "
							"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
							"MailSentNotesT.Note, PatientsT.UserDefinedID, "
							"CASE WHEN MailSent.MailID = PatientsT.PatPrimaryHistImage THEN 1 ELSE 0 END AS IsPrimaryImage "
							"FROM MailSent "
							"INNER JOIN PersonT ON MailSent.PersonID = PersonT.ID "
							"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
							"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
							"WHERE MailSent.PathName = {STRING}", strMailSentFilePath);
						while(!rsMultiPat->eof) {

							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}

							CString strMultiPersonName = VarString(rsMultiPat->Fields->Item["FullName"]->Value, "");
							long nMultiPersonID = VarLong(rsMultiPat->Fields->Item["PersonID"]->Value, -1);
							long nUserDefinedID = VarLong(rsMultiPat->Fields->Item["UserDefinedID"]->Value, -1);
							CString strMultiPersonNote = VarString(rsMultiPat->Fields->Item["Note"]->Value, "");

							//audit the note if the filename is empty
							CString strAudit;
							if (strMailSentFilePath.IsEmpty()) {
								strAudit = strMultiPersonNote;
							} else {
								strAudit = strMailSentFilePath;
							}

							//the new value is the same regardless of whether it is detached, or also deleted
							AuditEvent(nMultiPersonID, strMultiPersonName, nAuditTransactionID, nUserDefinedID != -1 ? aeiPatientDocDetach : aeiContactDocDetach, nMultiPersonID, strAudit, "<Detached>", aepHigh, aetDeleted);

							//TES 9/28/2015 - PLID 66192 - Was this their primary image?
							if (AsBool(rsMultiPat->Fields->GetItem("IsPrimaryImage")->Value)) {
								arPatIDsToUpdate.Add(nMultiPersonID);
							}

							rsMultiPat->MoveNext();
						}
						rsMultiPat->Close();

						AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PatPrimaryHistImage IN (%s)", strIDs);
						// (c.haag 2009-05-06 13:26) - PLID 33789 - Also clear out the Units field
						AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (%s)", strIDs);
						// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
						AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE PathName = '%s'", _Q(strMailSentFilePath)));
						try {
							ExecuteSqlBatch(strSqlBatch);
							if (nAuditTransactionID != -1) {
								CommitAuditTransaction(nAuditTransactionID);
							}
						}NxCatchAllSilentCallThrow(
							if (nAuditTransactionID != -1) {
								RollbackAuditTransaction(nAuditTransactionID);
								nAuditTransactionID = -1;
							}
						);

						// (j.jones 2011-07-22 15:43) - PLID 21784 - send a tablechecker for each file changed, usually it is only one
						ASSERT(aryPatientIDs.GetSize() == aryMailIDs.GetSize());
						for(int i=0; i<aryMailIDs.GetSize(); i++) {
							// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always true here
							CClient::RefreshMailSentTable(aryPatientIDs.GetAt(i), aryMailIDs.GetAt(i), TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);
						}
						// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
						// we have to send those as well.
						CClient::RefreshTable(NetUtils::TodoList, -1);

						//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
						foreach(long nPatID, arPatIDsToUpdate) {
							// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
							// this shared function will handle sending accordingly
							SendPatientPrimaryPhotoHL7Message(nPatID);
						}
						// (r.gonet 09/02/2014) - PLID 63221 - Don't send a tablechecker for labs. The mailsent one takes care of things on its own.
					}
				}
				else {
					DWORD nLastError = GetLastError();
					CString str;
					LPVOID lpMsgBuf;
					FormatMessage( 
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						nLastError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL 
					);
					str.Format("The image file '%s' could not be deleted, and was also not detached. Reason: %s", strMailSentFilePath, (LPCTSTR)lpMsgBuf);
					MsgBox("%s", str);
				}
			}
		}
		Refresh();

	// (c.haag 2009-10-29 09:01) - PLID 36019 - This needs to read OnDetachAndDelete; not OnDetach.
	}NxCatchAll("Error in CPhotoViewerCtrl::OnDetachAndDelete()");
}

LRESULT CPhotoViewerCtrl::OnNxmDetach(WPARAM wParam, LPARAM lParam)
{
	OnDetach();
	return TRUE;
}

LRESULT CPhotoViewerCtrl::OnNxmDetachAndDelete(WPARAM wParam, LPARAM lParam)
{
	OnDetachAndDelete();
	return TRUE;
}

BOOL CPhotoViewerCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nNotches = zDelta / WHEEL_DELTA;
	//TRACE("MouseWheel: nFlags %li, zDelta %li, notches %li\n", nFlags, (long)zDelta, nNotches);
	
	// (a.walling 2008-09-24 13:37) - PLID 31495 - Pass this using our fake SB flag
	OnVScroll(SB_NX_MOUSEWHEEL, nNotches, NULL);

	/*
	if(zDelta > 0) {
		for(int i = 0; i < nNotches; i++) {
			OnVScroll(SB_NX_MOUSEWHEEL, nNotches, NULL);
		}
	}
	else {
		for(int i = 0; i > nNotches; i--) {
			OnVScroll(SB_NX_MOUSEWHEEL, nNotches, NULL);
		}
	}
	*/
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

LRESULT CPhotoViewerCtrl::OnRefresh(WPARAM wParam, LPARAM lParam)
{
	Refresh();
	return 0;
}

void CPhotoViewerCtrl::OnCaptureChanged(CWnd *pWnd)
{
	if(m_pTempEdit) {
		OnKillFocusTempEdit();
	}
}

// (a.walling 2010-12-20 16:05) - PLID 41917
void CPhotoViewerCtrl::LoadImageAsync(LoadingImageInformationPtr pLoad)
{
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object

	if (!m_AsyncImageLoader) {
		// (a.walling 2007-07-23 12:50) - PLID 26769 - Cache these in bulk before spawning the thread
		// (c.haag 2011-01-20) - PLID 42166 - MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp
		g_propManager.CachePropertiesInBulk("PhotoViewerCtrl", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'JPEGThumbnailQuality' OR "
			"Name = 'LosslessThumbnail' OR "
			"Name = 'MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp' "
			")",
			_Q(GetCurrentUserName()));

		m_AsyncImageLoader = new AsyncImageLoader(GetSafeHwnd());
		// (z.manning 2011-04-27 11:04) - PLID 43461 - We do not want to auto-delete image loader thread.
		m_AsyncImageLoader->m_bAutoDelete = FALSE;

		// (c.haag 2010-02-23 16:00) - PLID 37364 - Assign the Mirror image manager. This can be NULL if the patient
		// is not linked with Mirror.
		if (NULL != m_pMirrorImageMgr) {
			m_AsyncImageLoader->SetMirrorPatientImageMgr(new CMirrorPatientImageMgr(m_pMirrorImageMgr));
		}

		m_AsyncImageLoader->CreateThread();
		m_AsyncImageLoader->SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	}

	//If we're already loading, add to our list.
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object
	if (m_AsyncImageLoader) {
		m_AsyncImageLoader->QueueImage(pLoad);
	}
}

// (a.walling 2008-09-23 11:37) - PLID 31479 - No longer const (uses critical section)
BOOL CPhotoViewerCtrl::IsLoadingImages()
{
	// (a.walling 2008-09-23 11:38) - PLID 31479 - This code is no longer called anymore, anyway.

	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object
	return m_AsyncImageLoader ? m_AsyncImageLoader->IsLoading() : FALSE;
}

void CPhotoViewerCtrl::AbortImageLoading()
{
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Use the AsyncImageLoader object
	if (m_AsyncImageLoader) {
		m_AsyncImageLoader->ClearQueue();
		m_AsyncImageLoader->Stop(5000);
		// (z.manning 2011-04-27 11:02) - PLID 43461 - The image loader thread no longer auto-deletes so let's manually delete it.
		delete m_AsyncImageLoader;
		m_AsyncImageLoader = NULL;
	}
	
	// Clear out any pending "a photo has loaded" messages in our queue.
	
	// (a.walling 2010-12-20 16:05) - PLID 41917 - Only process PostedMessages, don't dispatch others.
	// Since all posted images would be inserted into the g_arLoadingImagesPosted vector, we can simply clear that vector out.
	MSG msg;
	while (::PeekMessage(&msg, GetSafeHwnd(), NXM_PHOTO_LOADED, NXM_PHOTO_LOADED, PM_REMOVE | PM_QS_POSTMESSAGE | PM_NOYIELD)) {}
	
	{
		CSingleLock sl(&g_csLoadingImagesPosted, TRUE);
		g_arLoadingImagesPosted.clear();
	}

	//
	// Reset the images loaded flag just as the legacy code did
	//
	m_bImagesLoaded = false;

	// (a.walling 2010-12-20 16:05) - PLID 41917 - Got rid of old, commented-out code
}

LRESULT CPhotoViewerCtrl::OnPhotoLoaded(WPARAM wParam, LPARAM lParam)
{	
	LoadingImageInformationPtr pLoad;
	{
		LoadingImageInformation* pLoadRawPtr = (LoadingImageInformation*)wParam;

		// (a.walling 2010-12-20 16:05) - PLID 41917 - Find the image in the posted images vector to ensure it is both valid and not orphaned by a refresh/abort etc
		CSingleLock sl(&g_csLoadingImagesPosted, TRUE);
		std::vector<LoadingImageInformationPtr>::iterator it = std::find_if(g_arLoadingImagesPosted.begin(), g_arLoadingImagesPosted.end(), 
			boost::bind(&LoadingImageInformationPtr::get, _1) == pLoadRawPtr);

		if (it == g_arLoadingImagesPosted.end()) {
			// not found! must be an orphaned message.
			return 0;
		}

		pLoad = *it;

		g_arLoadingImagesPosted.erase(it);
	}

	// (a.walling 2010-12-20 16:05) - PLID 41917 - Got rid of old, commented-out code

	// (a.walling 2008-09-23 11:54) - PLID 31479 - We use the same thread now rather than create one per image. So all 
	// we really need to do is make sure our display is updated and handle any memory cleanup

	bool bNotifiedParent = false;
	if(pLoad) {
		//Find this item, set it.
		for(int i = 0; i < m_arImages.GetSize(); i++) {
			if(m_arImages.GetAt(i) == pLoad->ii) {
				if (!bNotifiedParent) {
					if (GetParent() && IsWindow(GetParent()->GetSafeHwnd())) {
						GetParent()->SendMessage(NXM_PHOTO_LOADED);
					}

					bNotifiedParent = true;
				}
				pLoad->ii->nNotesHeight = m_arImages.GetAt(i)->nNotesHeight;
				m_arImages.SetAt(i, pLoad->ii);

				// (a.walling 2008-09-24 09:46) - PLID 31479 - Ensure the notes height is copied, and
				// also only bother invalidating if it actually intersects. Also we are guaranteed that
				// only one image has actually updated, so we can break as soon as we find it.
				if (m_arImageRects.GetSize() > 0) {
					CRect rcClient;
					GetClientRect(rcClient);
					CRect rcIntersect;
					if (rcIntersect.IntersectRect(rcClient, m_arImageRects[i])) {
						InvalidateRect(m_arImageRects.GetAt(i));
					}
				}
				break;
			}
		}		
	}

	return 0;
}

void CPhotoViewerCtrl::SetUsingCategoryFilter(bool bUsing)
{
	//If this is true, our parent dialog is using a category filter.  This is used primarily
	//	for refreshing purposes when the users make changes to the categories of images.
	m_bUsingCategoryFilter = bUsing;
}

// (a.wetta 2007-07-09 14:19) - PLID 17467 - Marks the selected images as non-photos, removing them from the Photos tab
void CPhotoViewerCtrl::OnMarkAsNonPhoto()
{
	try {
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
			// We need to mark this as a non-photo
			ExecuteParamSql("UPDATE MailSent SET IsPhoto = 0 WHERE MailID = {INT}", m_listImagesSelected.GetAt(p)->nID);

			// (j.jones 2012-07-12 17:37) - PLID 49636 - send a tablechecker
			// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker - set IsPhoto
			// to true despite marking this as a non-photo, such that other photo panes know to refresh
			CClient::RefreshMailSentTable(m_nPersonID, m_listImagesSelected.GetAt(p)->nID, TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto);

			// Make sure that this image is not a primary image
			if(m_listImagesSelected.GetAt(p)->nID == m_nPrimaryImageID)
			{
				//OK, we're setting it to no primary.
				ExecuteParamSql("UPDATE PatientsT SET PatPrimaryHistImage = NULL WHERE PersonID = {INT}", m_nPersonID);
				SetPrimaryImage(-1);
				//TES 9/28/2015 - PLID 66192 - Update HL7 with the new primary image
				// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
				// this shared function will handle sending accordingly
				SendPatientPrimaryPhotoHL7Message(m_nPersonID);
			}
		}
		Refresh();
	}NxCatchAll("Error in CPhotoViewerCtrl::OnMarkAsNonPhoto");
}

// (j.jones 2009-10-14 09:17) - PLID 35894 - added functions to clean up lists
void CPhotoViewerCtrl::ClearImageArray()
{
	try {
		// (a.walling 2010-12-22 08:28) - PLID 41918 - Flag as needing to be sorted
		m_bNeedsSort = true;

		m_listImagesSelected.RemoveAll();

		for(int i=m_arImages.GetSize()-1; i>=0; i--) {
			if(m_arImages.GetAt(i)->bIsImageValid) 
				DeleteObject(m_arImages.GetAt(i)->hFullImage);
			if(m_arImages.GetAt(i)->bIsThumbValid && m_arImages.GetAt(i)->hThumb != m_hLoadingThumb)
				DeleteObject(m_arImages.GetAt(i)->hThumb);

			m_arImages.RemoveAt(i);
		}

	}NxCatchAll("Error in CPhotoViewerCtrl::ClearImageArray");
}

void CPhotoViewerCtrl::ClearProcNameRects()
{
	try {

		for(int i=m_arProcNameRects.GetSize()-1; i>=0; i--) {
			delete m_arProcNameRects.GetAt(i);
			m_arProcNameRects.RemoveAt(i);
		}

	}NxCatchAll("Error in CPhotoViewerCtrl::ClearProcNameRects");
}

// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
// and is ready for getting image counts and loading images
void CPhotoViewerCtrl::EnsureMirrorImageMgr()
{
	if (m_bIsPatient && m_nPersonID > 0) {
		if (NULL == m_pMirrorImageMgr) {
			m_pMirrorImageMgr = new CMirrorPatientImageMgr(m_nPersonID);
		}

		// (a.walling 2011-01-26 09:29) - PLID 42178 - Set the Mirror object for the loader
		if (m_AsyncImageLoader && !m_AsyncImageLoader->HasMirrorPatientImageLoader()) {
			m_AsyncImageLoader->SetMirrorPatientImageMgr(new CMirrorPatientImageMgr(m_pMirrorImageMgr));
		}
	}
}

// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
// not exist. If it did, it is deleted.
void CPhotoViewerCtrl::EnsureNotMirrorImageMgr()
{
	if (NULL != m_pMirrorImageMgr) {
		delete m_pMirrorImageMgr;
		m_pMirrorImageMgr = NULL;
	}
	// (a.walling 2011-01-26 09:29) - PLID 42178 - Clear the Mirror object for the loader
	if (m_AsyncImageLoader) {
		m_AsyncImageLoader->SetMirrorPatientImageMgr(NULL);
	}
}

// (c.haag 2010-05-26 12:00) - PLID 38731 - Assign a todo task to this photo.
void CPhotoViewerCtrl::OnNewTodo()
{
	try {
		// (a.walling 2010-12-20 16:05) - PLID 41917
		ImageInformationPtr pImg;
		{
			POSITION p = m_listImagesSelected.GetHeadPosition(); //We should never get here with more or less than one selected.
			if (p != NULL) {
				pImg = m_listImagesSelected.GetAt(p);
			}
		}

		if (pImg) {

			// This dialog also checks permissions.
			CTaskEditDlg dlg(this);
			dlg.m_nPersonID = m_nPersonID;
			dlg.SetNewTaskRegardingOverrides( pImg->nID, ttMailSent );
			//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
			dlg.m_bIsNew = TRUE;

			dlg.DoModal();

		} else {
			// This should almost never happen because we can only be called by the context menu 
			// and only if there is exactly one image selected.  But it's possible for the 
			// selection to change during the short interval that the context menu is on screen, 
			// so we give a friendly message here.
			MessageBox(_T("There is no image selected.  Please select an image and try again."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-24) PLID 49637 - Shows/Hide text label captions under the photos
void CPhotoViewerCtrl::ShowTextLabels(bool bShowLabels)
{
	m_bDrawTextLabels = bShowLabels;
}

//(e.lally 2012-04-24) PLID 49637
bool CPhotoViewerCtrl::TextLabelsAreVisible()
{
	return m_bDrawTextLabels;
}

//(e.lally 2012-04-30) PLID 49639 - Moved this code into its own public function
void CPhotoViewerCtrl::PreviewSelectedPhotos(DisplayAreas eDisplayAreas)
{
	// (j.jones 2016-05-11 10:59) - NX-100505 - added exception handling
	try {

		CArray<HBITMAP, HBITMAP> ahImagesToDelete; // (c.haag 2008-06-19 09:37) - PLID 28886
		//Show the preview area we just clicked on.
		CPhotoViewerDlg dlg(this);
		// (j.jones 2016-05-11 10:59) - NX-100505 - switched from a recordset to GetExistingPatientName
		dlg.m_strPatientName = GetExistingPatientName(m_nPersonID);

		switch ((DisplayAreas)eDisplayAreas)
		{
		case daPreview1:
			dlg.m_ilLayout = il1Image;
			break;
		case daPreviewTopBottom:
			dlg.m_ilLayout = il2TopBottom;
			break;
		case daPreviewSideSide:
			dlg.m_ilLayout = il2SideSide;
			break;
		case daPreview4:
			dlg.m_ilLayout = il4Images;
			break;
		}
		for (POSITION p = m_listImagesSelected.GetHeadPosition(); p; m_listImagesSelected.GetNext(p)) {
			//Have we loaded the full image yet?
			if (!m_listImagesSelected.GetAt(p)->bIsImageValid) {
				//Nope, we need to add it.
				if (m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
					_RecordsetPtr rsPath = CreateRecordset("SELECT PathName FROM MailSent WHERE MailID = %li", m_listImagesSelected.GetAt(p)->nID);
					HBITMAP hImage = NULL;
					m_listImagesSelected.GetAt(p)->bIsImageValid = LoadImageFile(AdoFldString(rsPath, "PathName"), hImage);
					if (m_listImagesSelected.GetAt(p)->bIsImageValid) {
						m_listImagesSelected.GetAt(p)->hFullImage = hImage;
						//Also, update in our main list, so we won't have to load again.
						for (int i = 0; i < m_arImages.GetSize(); i++) {
							if (m_arImages.GetAt(i)->nID == m_listImagesSelected.GetAt(p)->nID) {
								// (a.walling 2010-12-20 16:05) - PLID 41917
								ImageInformationPtr ii = (ImageInformationPtr)m_arImages.GetAt(i);
								ii->bIsImageValid = m_listImagesSelected.GetAt(p)->bIsImageValid;
								ii->hFullImage = hImage;
							}
						}
					}//end if image selected is valid

					//(e.lally 2005-05-27 PL16596) Added a case where the picture has a thumbnail
					//but has been deleted from it's location
					else
					{
						CString strTempFilePath;
						strTempFilePath = AdoFldString(rsPath, "PathName");
						MsgBox("Error opening the image file '" + strTempFilePath + "'" +
							"\n\nThe file may have been deleted or does not exist. " +
							"If the file you are looking for is on another computer, " +
							"make sure that your network is functioning properly." +
							"\nThe thumbnail Practice displays is not an actual copy and can be detached for complete removal."
							, "Error - File Not Found", MB_OK);
						return;
					}
				}
				else {
					//If it is mirror or united, bIsThumbValid and bIsImageValid should always be identical, 
					//and if !bIsThumbValid, the image should never be selected.
					ASSERT(FALSE);
				}
			}
			//OK, at this point the full image is valid.
			// (a.walling 2010-12-20 16:05) - PLID 41917
			ImageInformationPtr info = (ImageInformationPtr)m_listImagesSelected.GetAt(p);
			// (c.haag 2008-06-19 09:28) - PLID 28886 - If we're zooming in
			// a Mirror image, then by default, we need to load the full mirror
			// image.
			if (eImageSrcMirror == info->nType) {
				if (GetPropertyInt("MirrorUseFullImageInPhotoViewerDlg", 1)) {
					CWaitCursor wc;
					long i;
					if (-1 != (i = info->strFullFilePath.Find("-"))) {
						CString strRecNum = info->strFullFilePath.Left(i);
						CString str = info->strFullFilePath.Right(info->strFullFilePath.GetLength() - i - 1);
						long nIndex = atol(str);
						long nCount = nIndex + 1;
						int nCurrentQuality = GetPropertyInt("MirrorImageDisplay", GetPropertyInt("MirrorShowImages", 1));
						if (nCurrentQuality > 0 /* 0 = Don't show images */) {
							// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
							info->hFullImage = Mirror::LoadMirrorImage(strRecNum, nIndex, nCount, nCurrentQuality + 3);
							ahImagesToDelete.Add(info->hFullImage);
						}
					}
					else {
						// Something wrong with the filename formatting. Just use
						// the existing thumbnail image
					}
				}
			}
			dlg.m_arImages.Add(info);
		}
		if (dlg.m_arImages.GetSize() > 0) {
			dlg.DoModal();
		}
		// (c.haag 2008-06-19 09:36) - PLID 28886 - Delete the full images we loaded from Mirror
		while (ahImagesToDelete.GetSize() > 0) {
			DeleteObject(ahImagesToDelete[0]);
			ahImagesToDelete.RemoveAt(0);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-18 17:12) - PLID 57207 - Added ability to select all currently displayed images.
// If a filter is in place, the selection will only apply to the filtered images.
void CPhotoViewerCtrl::SelectAllDisplayedImages()
{
	try {

		//this function should not have been called during
		//a usage of the control that disallows multiselect
		if(!m_bAllowMultiSelect) {
			ASSERT(FALSE);
			return;
		}

		//This function is not a toggle. It will always try to select all images.
		//It won't try to unselect images if all are already selected.
		
		for(int i = 0; i < m_arImages.GetSize(); i++) {
			//we do not need to filter here because m_arImages should already
			//contain all visible images in our current display filter
			if(!IsImageSelected(i)) {
				m_listImagesSelected.AddTail(m_arImages.GetAt(i));
			}
		}

		//Do we have any practice images selected?
		BOOL bPracticeSelected = false;
		for(POSITION p = m_listImagesSelected.GetHeadPosition(); p && !bPracticeSelected; m_listImagesSelected.GetNext(p)) {
			if(m_listImagesSelected.GetAt(p)->nType == eImageSrcPractice) {
				bPracticeSelected = true;
			}
		}
		GetParent()->PostMessage(NXM_PHOTO_VIEWER_DETACHABLE, (WPARAM)bPracticeSelected);

		Invalidate();
		CBorrowDeviceContext bdc(this);
		DrawPreviewArea(bdc.m_pDC, GetDisplayArea(daPreview));

	}NxCatchAll(__FUNCTION__);
}