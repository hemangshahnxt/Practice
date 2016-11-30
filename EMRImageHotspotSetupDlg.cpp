// EMRImageHotspotSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EMRImageHotspotSetupDlg.h"
#include "EMRActionDlg.h"
#include "EMNDetail.h"
#include "EmrHotspotLocationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//DRT 1/11/2008 - PLID 28602 - Created.

/////////////////////////////////////////////////////////////////////////////
// CEMRImageHotspotSetupDlg dialog


//
//Modes.  These are the possible states of the dialog.  These are used to define what various
//	mouse presses do, which can vary depending what action the user is performing.
#define MODE_NORMAL		0		//This is the standard mode.  No drawing is allowed, clicking on hotspots launches their action data.
#define MODE_ADDING		1		//This is the mode for adding a new hotspot.  When on, clicking begins a hotspot.  You cannot edit any action data.
#define MODE_REMOVING	2		//This is the mode for deleting a hotspot.  When on, clicking on a hotspot will delete that hotspot.
//End Modes
//


CEMRImageHotspotSetupDlg::CEMRImageHotspotSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRImageHotspotSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRImageHotspotSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nMode = MODE_NORMAL;
	pCurrentSpot = NULL;
	m_pCurrentEMN = NULL;
	m_nInfoID = -1;
	m_hImage = NULL;
	m_bMaintainCurrentImage = false;
}


void CEMRImageHotspotSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRImageHotspotSetupDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_DELETE_HOTSPOT, m_btnDelete);
	DDX_Control(pDX, IDC_DEFINE_NEW, m_btnNew);
	DDX_Control(pDX, IDC_HOTSPOT_FRAME, m_btnHotspotFrame);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRImageHotspotSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRImageHotspotSetupDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_DEFINE_NEW, OnDefineNew)
	ON_BN_CLICKED(IDC_DELETE_HOTSPOT, OnDeleteHotspot)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRImageHotspotSetupDlg message handlers

BOOL CEMRImageHotspotSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Copy all the starting hotspots to the drawing array
		for(int i = 0; i < m_aryStartingHotSpots.GetSize(); i++) {
			m_arySpots.Add(m_aryStartingHotSpots.GetAt(i));
		}

		//Setup NxIconButtons
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnNew.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Load the image background once at the beginning and save a handle in m_hImage for
		//	the drawing to use as needed.
		if(!m_strBackgroundImage.IsEmpty()) {
			if(!LoadImageFile(GetSharedPath() ^ "Images" ^ m_strBackgroundImage, m_hImage, -1)) {
				//Failure to load
				AfxThrowNxException("Failed to load background image '%s' for hotspot definition.", m_strBackgroundImage);
			}
		}

		//TES 2/24/2010 - PLID 37294 - We have a preference now, so cache it.
		g_propManager.CachePropertiesInBulk("EMRImageHotSpotSetupDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PromptAnatomicLocationOnNewHotSpot' "
			")", _Q(GetCurrentUserName())
		);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRImageHotspotSetupDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {
		switch(m_nMode) {
		case MODE_NORMAL:
			{
				//First off, find out what hotspot they clicked in, if any
				//	Remember to offset for scaling
				OffsetPointForDrawingArea(&point);

				int i = 0;
				CEMRHotSpot *pClickedSpot = NULL;
				for(i = 0; i < m_arySpots.GetSize() && pClickedSpot == NULL; i++) {
					CEMRHotSpot *pSpot = m_arySpots.GetAt(i);

					if(pSpot->PtInSpot(point)) {
						pClickedSpot = pSpot;
					}
				}

				//If we found a spot, then load the actions from it
				if(pClickedSpot) {
					//TES 2/9/2010 - PLID 37223 - Moved to its own function.
					OpenActionsDlg(pClickedSpot);
				}
			}
			break;
		case MODE_ADDING:
			{
				//Create our hotspot
				pCurrentSpot = new CEMRHotSpot();

				//Add it to the array
				m_arySpots.Add(pCurrentSpot);

				//Add this point as the start of the hotspot
				OffsetPointForDrawingArea(&point);
				pCurrentSpot->AddPoint(point);
			}
			break;
		case MODE_REMOVING:
			//If the point is in a hotspot, this will prompt to delete it.
			long nSpotCount = m_arySpots.GetSize();
			bool bFound = false;
			OffsetPointForDrawingArea(&point);

			//reverse search so the one on top is deleted first if overlaps exist.
			for(int i = nSpotCount - 1;  !bFound && i >= 0; i--) {
				CEMRHotSpot *pSpot = m_arySpots.GetAt(i);

				if(pSpot->PtInSpot(point)) {
					bFound = true;

					//DRT 2/26/2008 - PLID 28603 - Before allowing them to delete, see if this hotspot is in use by any current details if we're
					//	editing on the fly.  If so, they are not allowed to delete.
					bool bAllowedToDelete = true;
					if(m_bMaintainCurrentImage) {
						//Search the array of details
						for(int nSearch = 0; nSearch < m_paryCurrentImageDetails->GetSize() && bAllowedToDelete; nSearch++) {
							CEMNDetail *pDetail = m_paryCurrentImageDetails->GetAt(nSearch);
							if(pDetail->IsHotSpotSelected(pSpot->GetID())) {
								//If it's selected in any, we must fail the deletion
								AfxMessageBox("This hotspot is in use on a detail that is currently open for editing.  You may not delete "
									"hotspots that are currently in use.");
								bAllowedToDelete = false;
							}
						}
					}


					//They clicked a spot.  Prompt to remove it.
					if(bAllowedToDelete && AfxMessageBox("Are you sure you wish to remove this hot spot?  You will erase all actions associated with the spot.", MB_YESNO) == IDYES) {
						//Do it!

						//Remove it from the full list
						m_arySpots.RemoveAt(i);

						//If it's a new element, just remove from there and be done, our caller doesn't
						//	need to know that it ever existed.
						bool bFoundNew = false;
						for(int nNewIdx = 0; nNewIdx < m_aryNewSpots.GetSize() && !bFoundNew; nNewIdx++) {
							if(m_aryNewSpots.GetAt(nNewIdx) == pSpot) {
								//Found it
								bFoundNew = true;

								//Remove it
								m_aryNewSpots.RemoveAt(nNewIdx);

								//Extra!  Since this was created here and deleted without going back, we are
								//	responsible for cleaning up its memory.
								delete pSpot;
								pSpot = NULL;
							}
						}

						//Otherwise, add it to the removal list.
						if(!bFoundNew) {
							m_aryRemovedSpots.Add(pSpot);
						}

						//Force redrawing to not draw it
						Invalidate(TRUE);
					}

					//Revoke the status, even if they said no.
					m_nMode = MODE_NORMAL;
					EnsureDisplay();
				}
			}
			break;
		}


	} NxCatchAll("Error in OnLButtonDown");

	CDialog::OnLButtonDown(nFlags, point);
}

void CEMRImageHotspotSetupDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	try {
		switch(m_nMode) {
		case MODE_NORMAL:
			//This does nothing
			break;
		case MODE_ADDING:
			//This ends the addition of a new hotspot.

			if(pCurrentSpot) {
				//Mark the spot as complete
				pCurrentSpot->MarkComplete();

				//Now that this spot is complete, put it in our "new" array
				m_aryNewSpots.Add(pCurrentSpot);

				//We no longer need or should have a current spot
				//TES 2/24/2010 - PLID 37294 - However, note it in case we need to pass it to the AnatomicLocationDlg in a few lines.
				CEMRHotSpot *pSpot = pCurrentSpot;
				pCurrentSpot = NULL;

				//Set the mode back to normal
				m_nMode = MODE_NORMAL;

				//Re-enable buttons
				EnsureDisplay();

				//DRT 2/19/2008 - PLID 28602 - Re-enable the mouse so you can move outside the image
				ClipCursor(NULL);

				//Force a redraw
				Invalidate(FALSE);

				//TES 2/10/2010 - PLID 37294 - If the preference is on, prompt to assign this new spot to an Anatomic Location
				if(GetRemotePropertyInt("PromptAnatomicLocationOnNewHotSpot", 1, 0, "<None>")) {
					OpenAnatomicLocationDlg(pSpot);
				}
			}
			else {
				//Somehow we're in the "addition" phase, but lack a current spot.  This should
				//	not be possible.  Correct the situation so they can start again, and throw
				//	an error.
				m_nMode = MODE_NORMAL;
				EnsureDisplay();
				//DRT 2/19/2008 - PLID 28602 - Re-enable the mouse so you can move outside the image
				ClipCursor(NULL);

				//DRT 1/29/2008 - This is unfortunately possible if you do weird things like double
				//	click the "define new spot" button repeatedly.  Eventually you'll reach a point
				//	where you get the LButtonUp without having created a new spot in LButtonDown (I
				//	think the button handler is stealing it, but we get the up for it when we should not).
				//So in that bizarre case, we just "fix" the state and pretend nothing happened.
				//AfxThrowNxException("Incorrect mode 'Adding' with no current hotspot.");
			}
			break;
		case MODE_REMOVING:
			//This does nothing
			break;
		}


	} NxCatchAll("Error in OnLButtonUp");

	CDialog::OnLButtonUp(nFlags, point);
}

void CEMRImageHotspotSetupDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	try {
		switch(m_nMode) {
		case MODE_NORMAL:
			//This does nothing
			break;
		case MODE_ADDING:
			if(pCurrentSpot) {
				//Just add this point
				OffsetPointForDrawingArea(&point);
				pCurrentSpot->AddPoint(point);

				//Force a redraw
				Invalidate(FALSE);
			}
			else {
				//We'll be getting mouse move messages before they start to draw.  So just ignore
				//	anything that comes while we lack a current spot.
			}
			break;
		case MODE_REMOVING:
			//This does nothing
			break;
		}


	} NxCatchAll("Error in OnMouseMove");

	CDialog::OnMouseMove(nFlags, point);
}

void CEMRImageHotspotSetupDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	try {
		CDC dcCopy;
		dcCopy.CreateCompatibleDC(&dc);

		//The client rect coords of the frame (where the DC needs to take up space).  This is
		//	the area we are constrained to drawing in.
		CRect rcFrame;
		GetDlgItem(IDC_HOTSPOT_FRAME)->GetWindowRect(rcFrame);
		ScreenToClient(rcFrame);

		//The client coordinates of the same frame.  These are needed so we can blt the image
		//	properly to fit the frame size and handle stretching.
		CRect rcFrameClient;
		GetDlgItem(IDC_HOTSPOT_FRAME)->GetClientRect(&rcFrameClient);


		//Get the bpp capabilities of the device (if you get this wrong you'll end up in black/white)
		long nCaps = dcCopy.GetDeviceCaps(BITSPIXEL);

		//Create a bitmap of the target size and select it into the device.  This is necessary (and I'm not 100% sure 
		//	why) for the drawing to work correctly when we are using DrawDIBitmapInRect below.
		CBitmap bmp;
		bmp.CreateBitmap(rcFrame.Width(), rcFrame.Height(), 1, nCaps, NULL);
		CBitmap *pOldBmp = (CBitmap*)dcCopy.SelectObject(&bmp);


		//
		//1)  Draw the background image to an off-screen DC
		if(m_hImage != NULL) {
			//We loaded the image file.  Now draw it.
			m_rcImage = DrawDIBitmapInRect(&dcCopy, rcFrameClient, m_hImage);

			//DRT 2/19/2008 - m_rcImage is now the screen coordinates of the image itself.  This is used later to constrain the
			//	mouse inside the image, so you can't go drawing crazy hotspots off in the nether.
			ClientToScreen(m_rcImage);
			m_rcImage.left += rcFrame.left;
			m_rcImage.right += rcFrame.left;
			m_rcImage.top += rcFrame.top;
			m_rcImage.bottom += rcFrame.top;

			CSize sz = GetImageSize(m_hImage);


			//Attempted description:  This code determines the ratios of the image scaling that we're doing.  We're putting a proportionate
			//	image into a random rectangle, so the DrawDIB... function above will stretch 1 side (cx or cy) to the max, and scale the
			//	other as much as possible, then fill the rest with black.  So we need to determine the "offset", if any (meaning any black
			//	space).  The offset is the amount on 1 side, not total.
			//We then need to determine the ratio.  This is how much to shrink our points due to the stretching.  So if our real image is 
			//	200 px, we stretch it to 600 px, that's a Scale of 3.0.  The ratio (for shrinking later) would be 0.33.  We want to save
			//	all points in data as a 1:1 ratio with the image itself.  So a line across the top of a 200 px image should save in data
			//	points from 0,0 to 200,0, and no more, even though on screen we're working with 0,0 to 600,0.

			//This is the scale used to make the image bigger (or smaller if it's already larger than our frame).
			double dblScaleX = (double)rcFrame.Width() / (double)sz.cx;
			double dblScaleY = (double)rcFrame.Height() / (double)sz.cy;

			//Pick the lower value, both sides will be set to that scale.
			double dblProportionedRatio = dblScaleX < dblScaleY ? dblScaleX : dblScaleY;

			//This is now the ratio that the entire image was multiplied by, since we keep X & Y coords proportionate to each other
			//	We find our offset by subtracting the "scaled size" from the "frame size", which tells us how much was leftover space, 
			//	then divide by 2 (the image is centered).
			m_nOffsetX = (long)((double)(rcFrame.Width() - (long)(sz.cx * dblProportionedRatio)) / 2.0);
			m_nOffsetY = (long)((double)(rcFrame.Height() - (long)(sz.cy * dblProportionedRatio)) / 2.0);


			//This ratio is the opposite of the above, the number used to scale down.
			double dblRatioX = (double)sz.cx / (double)(rcFrame.Width());
			double dblRatioY = (double)sz.cy / (double)(rcFrame.Height());

			//Take the greater, the image scales, because the image is proportionate.
			m_dblRatioX = dblRatioX > dblRatioY ? dblRatioX : dblRatioY;
			m_dblRatioY = m_dblRatioX;
		}
		else {
			//If there's no image, oh well, we just won't draw a background.  It makes this dialog fairly confusing, but they 
			//	should have been warned in the image loading.
		}

		//
		//2)  Draw all the hotspots.  Draw in reverse order, so that the first one created is on top if there
		//	is any overlap.  Again, draw to the off-screen DC.
		//Pen setup
		// (a.walling 2008-04-28 13:08) - PLID 29674 - Use the hotspot's selected color
		CPen *pOldPen, penNew(PS_SOLID, 4, CHotSpot::GetUnselectedColor());
		pOldPen = dcCopy.SelectObject(&penNew);

		long nSpotCount = m_arySpots.GetSize();
		for(int i = nSpotCount - 1; i >= 0; i--) {
			CEMRHotSpot *pSpot = m_arySpots.GetAt(i);

			//We've gone to all the trouble of saving the points in a 1:1 fashion, so now we need to draw
			//	them scaled back to our setup now.
			//pSpot->Draw(&dcCopy, m_dblRatioX, m_dblRatioY, m_nOffsetX, m_nOffsetY);
			// (c.haag 2009-02-19 13:15) - PLID 31327 - TRUE to always draw in regular mode (as opposed to gray, inactive hotspots)
			pSpot->Draw(&dcCopy, m_dblRatioX, m_dblRatioY, TRUE, m_nOffsetX, m_nOffsetY);
		}

		//Revert to old objects
		dcCopy.SelectObject(pOldPen);

		//
		//3)  Now copy the off-screen DC to the first (double buffering) so we don't get a ton of flicker while moving aboot
		dc.BitBlt(rcFrame.left, rcFrame.top, rcFrame.Width(), rcFrame.Height(), &dcCopy, 0, 0, SRCCOPY);

		//Reselect the old bitmap
		dcCopy.SelectObject(pOldBmp);

	} NxCatchAll("Error in OnPaint");

	// Do not call CDialog::OnPaint() for painting messages
}

//Updates any interface elements depending on the mode
void CEMRImageHotspotSetupDlg::EnsureDisplay()
{
	//How to set the status of each button?
	BOOL bOKCancel = FALSE;
	BOOL bNew = FALSE;
	BOOL bExisting = FALSE;
	BOOL bDelete = FALSE;

	//For the deleting button
	CString strDeleteText = "De&lete HotSpot";

	switch(m_nMode) {
	case MODE_NORMAL:
		bOKCancel = TRUE;
		bNew = TRUE;
		bExisting = TRUE;
		bDelete = TRUE;
		break;
	case MODE_ADDING:
		//We're now adding, so disable all the buttons
		//	No work, all defaults are false
		break;
	case MODE_REMOVING:
		//We're removing, so disable all buttons except the removing, which should change to 'cancel'
		//	All defaults are false.
		strDeleteText = "Cancel De&lete";
		bDelete = TRUE;
		break;
	}

	GetDlgItem(IDOK)->EnableWindow(bOKCancel);
	GetDlgItem(IDCANCEL)->EnableWindow(bOKCancel);
	GetDlgItem(IDC_DEFINE_NEW)->EnableWindow(bNew);
	GetDlgItem(IDC_DELETE_HOTSPOT)->EnableWindow(bDelete);
	GetDlgItem(IDC_DELETE_HOTSPOT)->SetWindowText(strDeleteText);
}

void CEMRImageHotspotSetupDlg::OnDefineNew() 
{
	try {
		//Change the mode
		m_nMode = MODE_ADDING;
		EnsureDisplay();

		//DRT 2/19/2008 - PLID 28602 - Clip the mouse to the image, we don't want them drawing outside that
		ClipCursor(&m_rcImage);

	} NxCatchAll("Error in OnDefineNew");
}

void CEMRImageHotspotSetupDlg::OnDeleteHotspot() 
{
	try {
		//This button also acts as a cancel.  If we're currently deleting, cancel it
		if(m_nMode == MODE_REMOVING) {
			m_nMode = MODE_NORMAL;
		}
		else {
			//Change the mode
			m_nMode = MODE_REMOVING;
		}

		EnsureDisplay();

	} NxCatchAll("Error in OnDeleteHotspot");
}

BOOL CEMRImageHotspotSetupDlg::DestroyWindow() 
{
	try {
		//We are done with the image, clean it up
		if(m_hImage != NULL) {
			DeleteObject(m_hImage);
			m_hImage = NULL;
		}

	} NxCatchAll("Error in DestroyWindow");

	return CDialog::DestroyWindow();
}

void CEMRImageHotspotSetupDlg::OffsetPointForDrawingArea(CPoint *lpPoint)
{
	//Since all drawing is done to an off-screen DC, and that DC is shifted onto the 
	//	IDC_HOTSPOT_FRAME, we need to shift all points by the same amount, otherwise
	//	when you draw, the lines get drawn away from your mouse by an offset.
	CRect rcFrame;
	GetDlgItem(IDC_HOTSPOT_FRAME)->GetWindowRect(rcFrame);
	ScreenToClient(rcFrame);

	//Move our point back by subtracting...
	//	where the frame starts on left
	lpPoint->x -= rcFrame.left;
	//	our offset in x (for image scaling)
	lpPoint->x -= m_nOffsetX;
	//	where the frame starts on top
	lpPoint->y -= rcFrame.top;
	//	our offset in y (for image scaling)
	lpPoint->y -= m_nOffsetY;

	//Now scale the point so it fits with the actual image size, not the current stretched value.  Truncate
	//	to the nearest int.
	lpPoint->x = (long)(lpPoint->x * m_dblRatioX);
	lpPoint->y = (long)(lpPoint->y * m_dblRatioY);
}

CSize CEMRImageHotspotSetupDlg::GetImageSize(HBITMAP hbmp)
{
	CSize szImage;
	if(hbmp) {
		BITMAP tmpBmp;
		GetObject(hbmp, sizeof(tmpBmp), &tmpBmp);
		szImage.cx = tmpBmp.bmWidth;
		szImage.cy = tmpBmp.bmHeight;
	}
	else {
		szImage.cx = 0;
		szImage.cy = 0;
	}
	
	return szImage;	
}

// (c.haag 2010-03-17 16:20) - PLID 37223 - This function will ensure all the values in memory are valid for saving.
BOOL CEMRImageHotspotSetupDlg::ValidateData()
{
	// The reason for this function is to check for any anatomic locations or location qualifier ID's that are not in data,
	// and to reset them and warn the user if they exist.
	CArray<long,long> anAnatomicIDs;
	CArray<long,long> anQualifierIDs;
	BOOL bHotspotsChanged = FALSE;
	CArray<CEMRHotSpot*,CEMRHotSpot*> aryHotSpots;
	int i;

	// 1) Build a list of all the anatomic and qualifier IDs that are in use in memory
	aryHotSpots.Append(m_aryStartingHotSpots);
	aryHotSpots.Append(m_aryChangedSpots);
	aryHotSpots.Append(m_aryNewSpots);
	for (i=0; i < aryHotSpots.GetSize(); i++) {
		CEMRHotSpot* p = aryHotSpots[i];
		if (p->GetAnatomicLocationID() > 0) { anAnatomicIDs.Add( p->GetAnatomicLocationID()); }
		if (p->GetAnatomicQualifierID() > 0) { anQualifierIDs.Add(p->GetAnatomicQualifierID()); }
	}

	// 2) Get a map of all the available ID's in data. This will be a subset of the contents of the two arrays.
	CMap<long,long,BOOL,BOOL> mapExistingAnatomicIDs;
	CMap<long,long,BOOL,BOOL> mapExistingQualifierIDs;

	if (anAnatomicIDs.GetSize() > 0) {
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID FROM LabAnatomyT WHERE ID IN (%s)", ArrayAsString(anAnatomicIDs));
		while (!prs->eof) {
			mapExistingAnatomicIDs.SetAt( AdoFldLong(prs, "ID"), TRUE );				
			prs->MoveNext();
		}
	}
	if (anQualifierIDs.GetSize() > 0) {
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID FROM AnatomyQualifiersT WHERE ID IN (%s)", ArrayAsString(anQualifierIDs));
		while (!prs->eof) {
			mapExistingQualifierIDs.SetAt( AdoFldLong(prs, "ID"), TRUE );				
			prs->MoveNext();
		}
	}

	// 3) For all the ID's in memory that are not in the maps, reset them to -1 for the hotspots
	for (int i=0; i < aryHotSpots.GetSize(); i++) {
		CEMRHotSpot* p = aryHotSpots[i];
		BOOL bDummy;
		if (p->GetAnatomicLocationID() > 0 && !mapExistingAnatomicIDs.Lookup( p->GetAnatomicLocationID(), bDummy )) {
			bHotspotsChanged = TRUE;
			p->SetAnatomicLocation(-1, "");
		}

		if (p->GetAnatomicQualifierID() > 0 && !mapExistingQualifierIDs.Lookup( p->GetAnatomicQualifierID(), bDummy )) {
			bHotspotsChanged = TRUE;
			p->SetAnatomicQualifier(-1, "");
		}
	}

	if (bHotspotsChanged) {
		MessageBox("One or more anatomic locations or qualifiers were removed from data, and your hotspots have been modified.\n\n"
			"Please review your hotspots before saving.", "NexTech Practice", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else {
		return TRUE;
	}
}

void CEMRImageHotspotSetupDlg::OnOK() 
{
	try {

		// (c.haag 2010-03-17 16:20) - PLID 37223 - Validate the data in memory first
		if (!ValidateData()) {
			return;
		}

		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void CEMRImageHotspotSetupDlg::OnCancel() 
{
	try {
		//If we are cancelling, we need to delete the memory in our "new" spots, because they will
		//	never be transferred back to the item entry dialog.
		for(int i = 0; i < m_aryNewSpots.GetSize(); i++) {
			CEMRHotSpot *pSpot = m_aryNewSpots.GetAt(i);
			delete pSpot;
		}
		m_aryNewSpots.RemoveAll();

		CDialog::OnCancel();

	} NxCatchAll("Error in OnCancel");
}

void CEMRImageHotspotSetupDlg::OnContextMenu(CWnd *pWnd, CPoint pos)
{
	try {
		switch(m_nMode) {
		case MODE_NORMAL:
			{
				//TES 2/9/2010 - PLID 37223 - Need to pop up a right-click menu, if they are on a spot.

				//First off, find out what hotspot they clicked in, if any
				//	Remember to offset for scaling
				ScreenToClient(&pos);
				OffsetPointForDrawingArea(&pos);

				int i = 0;
				CEMRHotSpot *pClickedSpot = NULL;
				for(i = 0; i < m_arySpots.GetSize() && pClickedSpot == NULL; i++) {
					CEMRHotSpot *pSpot = m_arySpots.GetAt(i);

					if(pSpot->PtInSpot(pos)) {
						pClickedSpot = pSpot;
					}
				}

				if(pClickedSpot) {
					enum {
						eSetAnatomicLocation = 1,
						eModifyActions = 2,
					};

					//TES 2/9/2010 - PLID 37223 - Create the menu
					CMenu mnu;
					mnu.CreatePopupMenu();
					//TES 2/9/2010 - PLID 37223 - Add an option to set the Anatomic Location of this spot.
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eSetAnatomicLocation, "&Set Anatomic Location...");
					//TES 2/9/2010 - PLID 37223 - Also add an option to modify the actions.
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eModifyActions, "&Modify Actions...");

					CPoint pt;
					GetCursorPos(&pt);

					int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

					switch(nRet) {
					case eSetAnatomicLocation:
						{
							//TES 2/9/2010 - PLID 37223 - Call our function
							OpenAnatomicLocationDlg(pClickedSpot);
						}
						break;
					case eModifyActions:
						//TES 2/9/2010 - PLID 37223 - Call the function (also called from OnLButtonDown).
						OpenActionsDlg(pClickedSpot);
						break;
					}
				}
			}
			break;

		default:
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

//TES 2/9/2010 - PLID 37223 - Copied from OnLButtonDown into its own function.
void CEMRImageHotspotSetupDlg::OpenActionsDlg(CEMRHotSpot *pSpot)
{
	// (z.manning 2011-07-25 10:48) - PLID 44676 - Moved this to a global function
	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	OpenHotSpotActionEditor(this, pSpot, m_nInfoID, m_strSourceItemName, m_pCurrentEMN, &m_aryChangedSpots);
}

void CEMRImageHotspotSetupDlg::OpenAnatomicLocationDlg(CEMRHotSpot* pSpot)
{
	CEmrHotspotLocationDlg dlg(this);
	dlg.SetSpot(pSpot);
	if(IDOK == dlg.DoModal()) {
		//TES 2/9/2010 - PLID 37223 - As with the actions, if this hotspot is existing (it has a valid positive ID number), then it is 
		// now changed.  We want to flag the hotspot so that any changes can be applied when saving.  Note that if they open and just hit 
		// OK, the end saving code will just detect that nothing changed.
		if(pSpot->GetID() > 0) {
			//Things could change multiple times.  Make sure it doesn't already exist
			bool bFound = false;
			for(int nChangeIdx = 0; nChangeIdx < m_aryChangedSpots.GetSize() && bFound == false; nChangeIdx++) {
				if(m_aryChangedSpots.GetAt(nChangeIdx) == pSpot) {
					bFound = true;
				}
			}

			//Remember these are just pointers to spots, so if it's there already, it gets our
			//	changes regardless.  We just want to avoid adding twice.
			if(!bFound) {
				m_aryChangedSpots.Add(pSpot);
			}
		}
		else {
			//Non-positive ID means that the hotspot is brand new.  Thus, it must already be in our m_aryNew..., and cannot
			//	be a changed spot.
		}
	}
}