#if !defined(AFX_EMRIMAGEHOTSPOTSETUPDLG_H__7A8632A9_5F20_41A0_A389_616D4A2BAA92__INCLUDED_)
#define AFX_EMRIMAGEHOTSPOTSETUPDLG_H__7A8632A9_5F20_41A0_A389_616D4A2BAA92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRImageHotspotSetupDlg.h : header file
//

//DRT 1/11/2008 - PLID 28602 - Created.

#include "EMRHotSpot.h"

class CEMN;

/////////////////////////////////////////////////////////////////////////////
// CEMRImageHotspotSetupDlg dialog

class CEMRImageHotspotSetupDlg : public CNxDialog
{
// Construction
public:
	CEMRImageHotspotSetupDlg(CWnd* pParent);   // standard constructor



	//
	//Data passed in from the caller

	//This is needed for the action dialog
	CEMN *m_pCurrentEMN;

	//The current info ID, needed in the action dialog
	long m_nInfoID;

	//The current item name, for the action dialog title bar
	CString m_strSourceItemName;

	//The background image.
	CString m_strBackgroundImage;

	//An array of all hotspots we should draw and allow the user to interact with.  This list should
	//	not change once the dialog starts, so that we can properly track the changes.
	CArray<CEMRHotSpot*, CEMRHotSpot*> m_aryStartingHotSpots;

	//DRT 2/26/2008 - PLID 28603 - Pointer to an array of details that exist of this info item if we're
	//	editing on the fly.  We cannot allow the hotspot to be deleted if this is set and it exists in 
	//	the array as in use.
	bool m_bMaintainCurrentImage;
	CArray<CEMNDetail*, CEMNDetail*> *m_paryCurrentImageDetails;

	//End data
	//

	//
	//Data sent back to the caller
	// (z.manning 2011-07-25 11:01) - PLID 44676 - Changed the type of these arrays.
	CEMRHotSpotArray m_aryNewSpots;
	CEMRHotSpotArray m_aryRemovedSpots;
	CEMRHotSpotArray m_aryChangedSpots;		//TODO:  Not handling actions yet

	//End data
	//

// Dialog Data
	//{{AFX_DATA(CEMRImageHotspotSetupDlg)
	enum { IDD = IDD_EMR_IMAGE_HOTSPOT_SETUP_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnNew;
	NxButton	m_btnHotspotFrame;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRImageHotspotSetupDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//The current 'mode' of the dialog -- see mode types defined in the .cpp
	short m_nMode;


	//HotSpot data.  This is only used for drawing, see the public arrays for saving/loading data.
	CArray<CEMRHotSpot*, CEMRHotSpot*> m_arySpots;

	//The hotspot currently being dealth with.  This is added to the array above immediately
	//	upon creation, but a pointer is kept for continued work until it is done.
	CEMRHotSpot *pCurrentSpot;

	//DRT 2/19/2008 - PLID 28602 - Had to move this here so its saved and we can use it for constraining the mouse while updating hotspots.
	CRect m_rcImage;

	// (c.haag 2010-03-17 16:20) - PLID 37223 - This function will ensure all the values in memory are valid for saving.
	BOOL ValidateData();

	//Interface updates
	void EnsureDisplay();

	//Offsets a point by the amount we are drawing, and applies the current aspect ratio to it for
	//	the image being scaled.
	void OffsetPointForDrawingArea(CPoint *lpPoint);

	//Gets the size of a given HBITMAP
	CSize GetImageSize(HBITMAP hbmp);

	//The ratio is the scale of the image.  Our dialog is a set size, and the image is shrunk or stretched
	//	to fit that size.  This ratio tells you how.  Currently the ratio should remain constant between
	//	the x & y (meaning ratioX and ratioY will always be the same), but I have them separated in 
	//	case we change our minds on that.
	double m_dblRatioX;
	double m_dblRatioY;

	//The offset is how much the image is moved inside the frame.  This is needed because of the scaling, one
	//	of these will be 0 (whichever presses against the frame), and the other will be the number of pixels
	//	on 1 side that is offset to the start of the image.
	long m_nOffsetX;
	long m_nOffsetY;

	//A handle to the background image, loaded once at the beginning and destroyed once at the end
	HBITMAP m_hImage;

	//TES 2/9/2010 - PLID 37223 - Copied from OnLButtonDown into its own function.
	void OpenActionsDlg(CEMRHotSpot *pSpot);

	//TES 2/9/2010 - PLID 37223 - Similar, new function for modifying the anatomic location
	void OpenAnatomicLocationDlg(CEMRHotSpot *pSpot);

	// Generated message map functions
	//{{AFX_MSG(CEMRImageHotspotSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnDefineNew();
	afx_msg void OnDeleteHotspot();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRIMAGEHOTSPOTSETUPDLG_H__7A8632A9_5F20_41A0_A389_616D4A2BAA92__INCLUDED_)
