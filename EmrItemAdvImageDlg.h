#pragma once

// EmrItemAdvImageDlg.h : header file
//

#include "EmrItemAdvDlg.h"

#define NXM_POPUP_IMAGE 34576

//TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.
#define INK_PICTURE_IDC	1000
#define NX3D_IDC		1001 // (z.manning 2011-07-21 15:51) - PLID 44649
/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvImageDlg dialog

// (a.walling 2010-05-19 08:26) - PLID 38558 - Include NxInkPictureImport.h rather than #import "NxInkPicture.tlb" so the proper tlb is chosen based on current SDK path (patch or main)
#include "NxInkPictureImport.h"
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXINKPICTURELib;

class CImageArray;

class CEmrItemAdvImageDlg : public CEmrItemAdvDlg
{
// Construction
public:
	CEmrItemAdvImageDlg(class CEMNDetail *pDetail);


	CString m_strCurrentImageFile; //Used to track when the image changes.

	// (j.armen 2014-07-23 11:19) - PLID 62836 - Methods for getting Pen Color / Size
	const inline long GetCurrentPenColor() const { return m_pInkPicture->CurrentPenColor; }
	const inline void SetCurrentPenColor(const long& nPenColor) const { m_pInkPicture->CurrentPenColor = nPenColor; }
	const inline float GetCurrentPenSize() const { return m_pInkPicture->CurrentPenSize; }
	const inline void SetCurrentPenSize(const float& fPenSize) const { m_pInkPicture->CurrentPenSize = fPenSize; }

	BOOL m_bIsEraserState;

	// (c.haag 2007-10-09 13:02) - PLID 27599 - If this is set to
	// FALSE, then the internal ink control will not accept ink
	// strokes. If set to TRUE, it will accept ink strokes if m_bReadOnly
	// is FALSE.
	BOOL m_bEnableInkInput;

	// (c.haag 2008-06-03 14:15) - PLID 27777 - TRUE if the topic is
	// visible (default value is TRUE)
	BOOL m_bIsTopicVisible;
	
	// (a.walling 2011-05-25 17:57) - PLID 43847 - Allow limiting the max size
	void GetIdealDimensions(OUT long &nWidth, OUT long &nHeight, bool bUseMaxSize = false);

	// (a.walling 2011-05-25 09:38) - PLID 43843 - Get the displayed image size from the control itself
	CRect GetDisplayedImageSize();

	virtual void ReflectCurrentContent();

	// (a.wetta 2007-04-10 10:08) - PLID 25532 - Be able to access the custom stamps
	void SetCustomStamps(CString strCustomStamps);

	// (r.gonet 02/14/2012) - PLID 37682 - Be able to set the current filter on the stamps.
	void SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter);

	// (j.jones 2010-02-24 15:27) - PLID 37312 - obsolete, we would never need
	// to get the stamps from an ink control since they are configured in Practice now
	//CString GetCustomStamps();

	//Is a valid image currently displaying in this detail?
	BOOL HasValidImage();

	//TES 2/24/2012 - PLID 45127 - Returns the index of the stamp at the given point (in screen coordinates).  
	// If there is no stamp there, returns -1.
	long GetStampIndexFromPoint(long x, long y);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvImageDlg)
	//}}AFX_VIRTUAL

public:
	virtual void ReflectCurrentState();
	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);

public:
	// (c.haag 2007-10-09 12:59) - PLID 27599 - This function
	// sets the ink control to be read-only based on m_bReadOnly
	// and m_bEnableInkInput
	virtual void UpdateInkPictureReadOnlyState();

	// (c.haag 2007-10-09 13:00) - PLID 27599 - This function
	// toggles the ability for the internal ink control to accept
	// ink strokes. If bEnable is FALSE, then the control will not
	// accept ink strokes even if m_bReadOnly is FALSE.
	virtual void EnableInkInput(BOOL bEnable);

	// (j.jones 2010-02-18 10:14) - PLID 37423 - this function will tell the NxInkPicture
	// whether the image is or is not a SmartStampImage
	void SetIsSmartStampImage(BOOL bIsSmartStampImage);

public:
	// (c.haag 2009-02-17 17:54) - PLID 31327 - Toggles for stamping on hotspots
	BOOL GetEnableHotSpotClicks();
	void SetEnableHotSpotClicks(BOOL bSet);

public:
	void OnFullScreenInkPicture();
	void OnFullScreen3DControl(); // (j.armen 2011-09-06 17:19) - PLID 45347

	// (z.manning 2011-09-26 09:27) - PLID 45664
	void Get3DModelOutputData(OUT CImageArray *paryImages);

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

	// (a.walling 2011-05-25 17:57) - PLID 43847 - Restore the ideal size of the image
	void RestoreIdealSize(bool bUseMaxSize);

// Implementation
protected:
	CWnd m_wndInkPic;
	NXINKPICTURELib::_DNxInkPicturePtr m_pInkPicture;
	Nx3DLib::_DNx3DPtr m_p3DControl;
	// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - More versatile replacement for g_EmrImageCache

protected:
	CSize m_szMinSize;

	// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
	void UpdateRememberedPenState();

	void Handle3DStampChange(); // (z.manning 2011-09-12 11:20) - PLID 45335

	void HandleStampSetup(); // (z.manning 2011-10-25 14:47) - PLID 39401

	// (r.gonet 02/14/2012) - PLID 37682
	void HandleStampFilterSetup();

protected:

	// (z.manning, 01/22/2008) - PLID 28690 - Added message handler for clicking on hot spots.
	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvImageDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPictureChangedInkPicture(LPCTSTR strNewPicture);
	afx_msg void OnStrokeInkPicture(LPDISPATCH Cursor, LPDISPATCH Stroke, BOOL FAR* Cancel);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnBrowseInkPicture(BOOL FAR* pbProcessed);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LRESULT OnShowTopic(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTextChangedInkPicture(NXINKPICTURELib::ETextChangeType TextChangeType);
	afx_msg void OnCustomStampsChangedInkPicture(LPCTSTR strNewCustomStamps);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message); // (z.manning, 08/20/2007) - PLID 23867
	afx_msg void OnClickedHotSpotInkPicture(long nHotSpotID);
	afx_msg void OnClickedHotSpot3DControl(short n3DHotSpotID); // (z.manning 2011-07-25 12:01) - PLID 44649
	afx_msg void OnSmartStampAdd3DControl(); // (z.manning 2011-09-12 11:09) - PLID 45335
	afx_msg void OnSmartStampErase3DControl(); // (z.manning 2011-09-12 11:09) - PLID 45335
	afx_msg LRESULT OnToggleHotSpot(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-02-10 14:44) - PLID 37312 - handle when the image tries to open the stamp setup
	afx_msg void OnOpenStampSetupInkPicture();
	afx_msg void OnOpenStampSearch();
	afx_msg void OnOpenStampSearch3D(); // (j.gruber 2012-08-14 12:24) - PLID 52134
	// (r.gonet 02/14/2012) - PLID 37682 - Handle when the user clicks the stamp filter setup buttons
	afx_msg void OnOpenStampFilterSetupInkPicture();
	afx_msg void OnOpenStampFilterSetup3DControl();
	afx_msg void OnOpenStampSetup3DControl(); // (z.manning 2011-09-14 10:13) - PLID 44693
	afx_msg void OnPreviewModified3DControl(); // (z.manning 2011-10-05 13:33) - PLID 45842
	afx_msg void OnClickedTopazSignature();	// (d.singleton 2013-05-03 16:59) - PLID 56421 - add button and functionality to the NxInkControl to use the topaz signature pad
	// (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom level and pan offsets.
	//afx_msg void OnZoomLevelChangedInkPicture(double dZoomLevel);
	//afx_msg void OnViewportOffsetsChangedInkPicture(long nOffsetX, long nOffsetY);
	// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle PenColorChanged and PenSizeChanged
	afx_msg void OnCurrentPenColorChanged();
	afx_msg void OnCurrentPenSizeChanged();
	// (a.walling 2010-10-25 10:04) - PLID 41043 - Handle OnShowWindow to clear out the cached image when no longer necessary to eat up memory
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
