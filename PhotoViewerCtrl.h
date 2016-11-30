#if !defined(AFX_PHOTOVIEWERCTRL_H__C7061B15_A6AF_43BD_835D_C4D0A9EF9F3A__INCLUDED_)
#define AFX_PHOTOVIEWERCTRL_H__C7061B15_A6AF_43BD_835D_C4D0A9EF9F3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhotoViewerCtrl.h : header file
//
#include "NxToolTip.h"
//Has a handy enum
#include "MirrorImageButton.h"
#include "FloatingImageWnd.h"
#include "dyngdi32.h"
/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerCtrl window

// (a.walling 2011-01-26 09:29) - PLID 42178 - Deleted some more commented-out stuff that started to make things hard to follow



//This struct and enum are shared between this and CPhotoViewerDlg
// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
typedef boost::shared_ptr<class ImageInformation> ImageInformationPtr;
class ImageInformation {
public:
	ImageInformation() 
		: hFullImage(NULL)
		, hThumb(NULL)
		// (a.walling 2008-09-24 09:13) - PLID 31479
		, nNotesHeight(-1)
		, nID(-1)
		, bIsImageValid(false)
		, bIsThumbValid(false)
		, nType(eImageSrcPractice)
		, dtAttached(g_cdtInvalid)
		, dtService(g_cdtInvalid) //(e.lally 2012-04-16) PLID 39543
	{
	}

	bool operator==(const ImageInformation &iiSource) const {
		if(iiSource.nType != nType) return false;
		if(iiSource.nType == eImageSrcDiagram) {
			return iiSource.strFullFilePath == strFullFilePath;
		}
		else {
			return iiSource.nID == nID;
		}
	}

	long nID;
	// (j.jones 2013-09-19 14:19) - PLID 58547 - cleaned up these variables
	// so that strMailSentFilePath is exactly what is in MailSent (which is often,
	// but not always, just a file name only)
	// and strFullFilePath is always the full patch to the file
	CString strMailSentFilePath;
	CString strFullFilePath;
	HBITMAP hFullImage;
	HBITMAP hThumb;
	CString strToolText;
	CString strNotes;
	long nNotesHeight; // (a.walling 2008-09-24 09:13) - PLID 31479
	bool bIsImageValid;
	bool bIsThumbValid;
	EImageSource nType;
	CArray<long, long> arynProcedureIDs;	// (j.jones 2009-10-12 16:22) - PLID 35894 - changed to be an array of IDs
	CString strProcedureNames;
	COleDateTime dtAttached;
	COleDateTime dtService;	//(e.lally 2012-04-16) PLID 39543
	CString strCategory;
	CString strStaff;

	CString GetTextToDraw(); //(e.lally 2012-04-13) PLID 49637
};

enum ImageLayout {
	il1Image,
	il2TopBottom,
	il2SideSide,
	il4Images,
};

struct ProcNameRect {
	// (j.jones 2009-10-13 09:37) - PLID 35894 - supported multiple procedures
	CArray<long, long> arynProcedureIDs;
	CString strProcedureNames;
	CRect rDrawn;
	CRgn * pRgn;
	ProcNameRect() {pRgn = NULL;}
	~ProcNameRect();
	ProcNameRect(ProcNameRect &pnrSource);
	void operator =(ProcNameRect &pnrSource);

};

enum HitTestType {
	httImage,
	httImageNotes,
	httPreviewArea,
	httProcNames,
	httNull
};

enum DragType {
	dtFocusRect,
	dtImage,
	dtNull,
};

//Passed to our thread that loads the images.
// (c.haag 2009-10-26 10:17) - PLID 36019 - Changed this from a struct to a class and
// intentionally privatized the construction and destruction. The reason is so that we
// can more easily track allocation and deletion when debugging crashes or race conditions.
// Because this object can be created in the main thread and destroyed in a worker thread,
// I think having the ability is a big help (as it was for this item).
// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
typedef boost::shared_ptr<class LoadingImageInformation> LoadingImageInformationPtr;
class LoadingImageInformation {
public:
	// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
	ImageInformationPtr ii;
	CString strMirrorID;
	long nMirrorCount;
	long nUnitedID;
};

// (a.walling 2011-01-26 09:29) - PLID 42178 - Generic base class for an async CWinThread-derived thread. Calls virtual function RunThread, with CoInitialize and exception handling built-in.
class AsyncThread : public CWinThread
{
public:
	AsyncThread(BOOL bAutoDelete)
		: CWinThread(AsyncThread::ThreadProc, NULL)
	{
		m_bAutoDelete = bAutoDelete;
		m_pThreadParams = (void*)this;
	};
	
protected:
	static UINT ThreadProc(LPVOID pParam)
	{
		CoInitialize(NULL);

		AsyncThread* pThread = (AsyncThread*)pParam;

		UINT returnVal = -1;
		try {
			returnVal = pThread->RunThread();
		} NxCatchAllThread(__FUNCTION__);

		CoUninitialize();

		return returnVal;
	};

	virtual UINT RunThread() = 0;
};

class CPhotoViewerCtrl : public CWnd
{
// Construction
public:
	CPhotoViewerCtrl();

// Attributes
public:
	// (z.manning, 06/07/2007) - PLID 23862 - Added a public variable for the image path to use in case 
	// we don't want to use the default.
	CString m_strImagePath;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhotoViewerCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPhotoViewerCtrl();
	void Refresh(bool bSynchronous = false);
	void SetPersonID(long nPersonID);
	void SetPicID(long nPicID);
	void SetAdditionalFilter(CString strAdditionalFilter);
	void SetPrimaryImage(long nPrimaryImageID);
	// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
	void SetPicContainer(class CPicContainerDlg* pPicContainer);

	bool m_bShowPreviewArea;
	bool m_bAllowMultiSelect;
	// (j.jones 2009-10-13 12:27) - PLID 35894 - changed to be an array of pointers
	// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
	void GetSelectedImages(CArray<ImageInformationPtr,ImageInformationPtr> &arSelected);
	bool m_bShowMirror;
	bool m_bShowUnited;

	bool m_bUsingCategoryFilter;
	void SetUsingCategoryFilter(bool bUsing);

	// (a.walling 2008-09-23 11:37) - PLID 31479 - No longer const (uses critical section)
	BOOL IsLoadingImages();
	void AbortImageLoading();

	//Call on initialization, will perform all startup tasks (e.g., creating the scrollbar).
	void PrepareWindow();

	//(e.lally 2012-04-24) PLID 49637
	void ShowTextLabels(bool bShowLabels);
	bool TextLabelsAreVisible();

	// (j.jones 2013-06-18 17:12) - PLID 57207 - Added ability to select all currently displayed images.
	// If a filter is in place, the selection will only apply to the filtered images.
	void SelectAllDisplayedImages();

public:
	// (b.cardillo 2005-06-28 18:03) - PLID 16454 - These are the functions to 
	// be used to by the caller to access the sort orders and criteria.

	enum EDisplaySortCriteria {
		
		dscStaff = -1,
		dscAttachDate = -2,
		dscNote = -3,
		dscCategory = -4,
		dscProcedureName = -5,
		dscServiceDate = -6, //(e.lally 2012-04-16) PLID 39543

		DSC_COUNT = 6,
		DSC_INVALID = 0,
	};

	//(e.lally 2012-04-30) PLID 49639 - Defined higher in the code
	enum DisplayAreas
	{
		daMainList,
		daPreview,
		daPreview1,
		daPreviewTopBottom,
		daPreviewSideSide,
		daPreview4,
	};
	
	// This puts edscCriterion first, gives it an order of bAscending, and shifts 
	// everything else down.  If edscCriterion is already first and in bAscending 
	// order, this function has no effect.
	// Returns TRUE if something was changed, otherwise FALSE.
	BOOL SetPrimarySort(IN const EDisplaySortCriteria edscCriterion, IN const BOOL bAscending);
	// This puts edscCriterion first, and leaves the order the same if it was 
	// already in the list.  If it wasn't already in the list, it defaults the 
	// sort order to TRUE (ascending).
	// Returns FALSE iff edscCriterion was already the primary sort criterion.
	BOOL SetPrimarySortCriterion(IN const EDisplaySortCriteria edscCriterion);
	// This takes the current primary sort criterion and gives it a sort order of 
	// bAscending.
	// Returns FALSE iff there was no primary sort criterion or it was already 
	// sorted in bAscending order.
	BOOL SetPrimarySortOrder(IN const BOOL bAscending);

	// Retrieves the current primary sort criterion (returns DSC_INVALID if there 
	// is no primary sort criterion right now)
	EDisplaySortCriteria GetPrimarySortCriterion();
	// Retrieves the current primary sort order (TRUE is ascending, FALSE is 
	// descending, -2 if there's no primary sort criterion)
	BOOL GetPrimarySortOrder();

	//(e.lally 2012-04-30) PLID 49639
	void PreviewSelectedPhotos(DisplayAreas eDisplayAreas);

protected:
	// (a.walling 2011-01-26 09:29) - PLID 42178 - The asynchronous image loader object
	// the class itself is defined in the .cpp
	class AsyncImageLoader* m_AsyncImageLoader;

	// (b.cardillo 2005-06-28 18:04) - PLID 16454 - These are the actual storage 
	// for the sort orders and criteria.

	// Sort info (criteria and order)
	// This is an array so that we can have a sort hierarchy (i.e. sort first by 
	// category ascending, then by date descending, etc., etc.).  The access 
	// functions (above) ensure that these arrays contain only one instance of 
	// any given sort criterion (e.g. dscAttachDate will only occur ONCE in the 
	// m_aryedscSortCriterion array).
	EDisplaySortCriteria m_aryedscSortCriterion[DSC_COUNT];
	BOOL m_arybSortAscending[DSC_COUNT];
	// Indicates how many elements of the above array are valid.
	short m_nSortInfoCount;
	// (a.walling 2010-12-22 08:28) - PLID 41918 - dirty flag for sorting
	bool m_bNeedsSort;

	bool m_bDrawTextLabels; //(e.lally 2012-04-24) PLID 49637

	// Generated message map functions
protected:
	bool m_bImagesLoaded;
	// (j.jones 2009-10-13 12:27) - PLID 35894 - changed to be an array of pointers
	// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
	CArray<ImageInformationPtr, ImageInformationPtr> m_arImages;
	CList<ImageInformationPtr, ImageInformationPtr> m_listImagesSelected;
	// (b.cardillo 2010-03-11 11:05) - PLID 37705 - Temporary storage to remember selected image IDs so current selection can be restored after refresh.
	CDWordArray m_arynImageIDsSelectedBeforeRefresh;
	bool IsImageSelected(int nIndex);
	void UnselectImage(int nIndex);

	void LoadArray();
	long m_nPersonID;
	long m_nPicID;
	// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
	class CPicContainerDlg* m_pPicContainer;
	CString m_strAdditionalFilter;
	BOOL m_bIsPatient;

	// (a.walling 2008-09-24 10:11) - PLID 31495
	long m_nMaxHeight; // maximum height of the images list
	long m_nNumColumns; // number of columns drawn

	CArray<CRect, CRect&> m_arImageRects; //Needed for hit testing.
	// (j.jones 2009-10-13 12:27) - PLID 35894 - changed to be an array of pointers
	CArray<ProcNameRect*, ProcNameRect*> m_arProcNameRects;
	//Returns the index in m_arImages of the given point.
	// (j.jones 2009-10-13 09:53) - PLID 35894 - this function now also returns the procedure names
	int HitTest(int x, int y, HitTestType OUT &nType, CString OUT &strProcedureNames);
	
	int m_nCurrentHotImage;
	// (j.jones 2009-10-13 09:09) - PLID 35894 - this is now a string,
	// as we group by multiple names rather than one ID
	CString m_strCurrentHotProcedures;

	CNxToolTip* m_pToolTipWnd;
	
	//Loads the given filename into hImage, returns success or failure (hImage will be an image describing the error on failure).
	bool LoadImageFile(CString &strFile, HBITMAP &hImage);

	//Takes an ImageInfo, asynchronously sets the hFullImage, hThumb and bIsThumbValid members, and then finds the 
	//corresponding image in our array and fills it in.
	// (a.walling 2010-12-20 16:05) - PLID 41917
	void LoadImageAsync(LoadingImageInformationPtr pLoad);

	HBITMAP m_hLoadingThumb;

	//Returns the rect that the image took up.
	// (a.walling 2008-09-22 16:32) - PLID 31479 - Also returns the height (px) of the image drawn
	// (a.walling 2008-09-24 09:15) - PLID 31479 - Option to set clipping region or not
	CRect DrawImage(CDC *pDC, int nIndex, int nLeft, int nTop, const CRect& rcClient, BOOL bSetClip = TRUE, long* pnDrawnAmount = NULL);

	//Redraws only the given image, in a (hopefully) non-flickering way (assumes nothing has changed).
	void InvalidateImage(int nIndex);

	//Redraws all images in the given procedure; as well as the procedure frame, in a (hopefully) non-flickering way.
	// (j.jones 2009-10-13 09:35) - PLID 35894 - converted to run by procedure name(s)
	void InvalidateProcedure(CString strProcedureNames);

	//Rotates the selected image files clockwise by nDegrees
	void RotateImageCW(int nDegrees);

	//Re-loads the given image from the data and from its file.
	void ReloadImage(int nIndex);

	//Draws the main image area in the given area..
	void DrawMainList(CDC *pDC, CRect rRect);
	//Draws the area with the preview options.
	void DrawPreviewArea(CDC *pDC, CRect rInvalid);
	//Fills in the given rectangle with the image at the given index.
	void FillPreviewRect(CDC *pDC, CRect rRect, int nSelectedIndex, bool bHot);

	// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
	// and is ready for getting image counts and loading images
	void EnsureMirrorImageMgr();
	// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
	// not exist. If it did, it is deleted.
	void EnsureNotMirrorImageMgr();

	DragType m_nDragType;//Set on LButtonDown
	bool m_bDragStarted; //Set on MouseMove
	//Valid if m_nDragType == dtFocusRect:
	CPoint m_ptDragStart;
	//Valid if m_nDragType == dtImage:
	int m_nDraggedImage;
	//Valid for both (last focus rect, or last ghostly image rect.
	CRect m_rLastDragRect;

	//Handling the Ctrl+click to set image order.
	bool m_bControlDown;
	int m_nCtrlSelIndex;

	CRect GetDisplayArea(DisplayAreas da);

	//Should always be -1 or one of the five small preview boxes.
	int m_nHotPreviewArea;
	
	CScrollBar m_sbVert;
	int m_nScrollBarWidth;
	void SetScroll();
	int m_nScrollPos;

	long m_nPrimaryImageID; 

	CNxEdit *m_pTempEdit;
	CFont *m_pIconFont;

	bool m_bCanEditNotes;
	int m_nStartEditHeight;

	// (c.haag 2010-02-23 13:40) - PLID 37364 - This object acts as a manager / cache source for Mirror image-related data
	class CMirrorPatientImageMgr* m_pMirrorImageMgr;

	void UnselectAll();

	//Puts m_arImages in the order we want them on screen.
	void SortArray();

	CPtrArray m_parRgnsToDelete;

	CRect DrawGhostlyImage(CDC *pDC, HBITMAP hImage, CPoint ptUpperLeft);
	CFloatingImageWnd *m_pGhostlyImageWnd;
	HBITMAP m_hScreenshot;

	int m_nEMRLevel;

	// (j.jones 2009-10-14 09:17) - PLID 35894 - added functions to clean up lists
	void ClearImageArray();
	void ClearProcNameRects();

	GDI32 m_gdi32;
	//{{AFX_MSG(CPhotoViewerCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRotate90();
	afx_msg void OnRotate180();
	afx_msg void OnRotate270();
	afx_msg void OnOpenPicture();
	afx_msg void OnTogglePrimary();
	afx_msg void OnEditNotes();
	afx_msg void OnKillFocusTempEdit();
	afx_msg void OnChangeTempEdit();
	afx_msg void OnLaunchMirror();
	afx_msg void OnLaunchUnited();
	afx_msg void OnAssignProc();
	afx_msg void OnDestroy();
	afx_msg BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDetach();
	afx_msg void OnDetachAndDelete();
	afx_msg LRESULT OnNxmDetach(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNxmDetachAndDelete(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnRefresh(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg LRESULT OnPhotoLoaded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMarkAsNonPhoto();
	afx_msg void OnNewTodo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHOTOVIEWERCTRL_H__C7061B15_A6AF_43BD_835D_C4D0A9EF9F3A__INCLUDED_)
