// EyeDrawDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EyeDrawDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEyeDrawDlg dialog


CEyeDrawDlg::CEyeDrawDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEyeDrawDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEyeDrawDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEyeDrawDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEyeDrawDlg)
	DDX_Control(pDX, IDC_IMAGE, m_Image);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEyeDrawDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEyeDrawDlg)
	ON_BN_CLICKED(IDC_COLOR, OnColor)
	ON_BN_CLICKED(IDC_UNDO, OnUndo)
	ON_BN_CLICKED(IDC_SMALL, OnSmall)
	ON_BN_CLICKED(IDC_MEDIUM, OnMedium)
	ON_BN_CLICKED(IDC_LARGE, OnLarge)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEyeDrawDlg message handlers

BOOL CEyeDrawDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	//might as well be set false, but this function is called just incase...
	if(!CanUndo())
		GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);

	m_bDrawMode = false; //set DrawMode default to false
	m_dc = GetDC(); //get handle to current display context
	m_nCurForeColor = RGB(0,0,0);  //set the default color to black
	
	//set to a medium sized pen
	OnMedium();

	//set up the member dc (that saves the current bitmap)
	m_memdc.CreateCompatibleDC(NULL);
	//now load the default bitmap into the m_memdc
	SwitchBitmaps(IDB_CORNEA);

	//set the last draw point to be at the origin of the drawing space.
	m_ptLastPoint.x = m_rcImageRect.left;  
	m_ptLastPoint.y = m_rcImageRect.top;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEyeDrawDlg::SwitchBitmaps(int nID) {

	//get the bitmap display context
	CBitmap bmpDC;
	bmpDC.CreateCompatibleBitmap(GetDC(), 640, 480);
	m_memdc.SelectObject(&bmpDC);

	//load the passed-in bitmap
	CDC dc;
	CBitmap bmp;
	bmp.LoadBitmap(nID);

	//get the dc to it
	dc.CreateCompatibleDC(NULL);
	dc.SelectObject(&bmp);

	//prepare the load
	m_Image.GetWindowRect(m_rcImageRect);
	ScreenToClient(m_rcImageRect);

	// Now blit the static bitmap on it
	m_memdc.BitBlt(0,0, 640, 480, NULL, 0,0, WHITENESS);
	m_memdc.BitBlt(0,0, m_rcImageRect.Width(), m_rcImageRect.Height(), &dc, 0,0,SRCCOPY);

	dc.DeleteDC();
}

void CEyeDrawDlg::OnColor() 
{
	//load the color palette dialog
	CColorDialog dlg(m_nCurForeColor, CC_FULLOPEN|CC_ANYCOLOR|CC_RGBINIT);
	if (dlg.DoModal() == IDOK) {
		m_nCurForeColor = dlg.m_cc.rgbResult;
	}
	//cleanly change the pen to the new color
	CPen NewPen(PS_SOLID,m_iPenSize,m_nCurForeColor);
	m_pen = &NewPen;
	m_OldPen = m_dc->SelectObject(m_pen);
}

void CEyeDrawDlg::OnUndo() 
{
	if(CanUndo()) {
		//cleanly remove the item from memory
		CDrawEvent *pEvent = NULL;
		pEvent = (CDrawEvent *)m_aryDrawEvents.GetAt(m_aryDrawEvents.GetSize()-1);
		if (pEvent)
			pEvent->Clear();
		delete pEvent;
		//remove last event
		m_aryDrawEvents.RemoveAt(m_aryDrawEvents.GetSize()-1,1);
	}

	//update undo status
	if(m_aryDrawEvents.GetSize()==0)
		GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);

	//redraw
	//Invalidate();

	RedrawWindow();
}

void CEyeDrawDlg::OnSmall() 
{
	//switch to a larger pen
	m_iPenSize = 2;
	CPen NewPen(PS_SOLID,m_iPenSize,m_nCurForeColor);
	m_pen = &NewPen;
	m_OldPen = m_dc->SelectObject(m_pen);
}

void CEyeDrawDlg::OnMedium() 
{
	//switch to a larger pen
	m_iPenSize = 4;
	CPen NewPen(PS_SOLID,m_iPenSize,m_nCurForeColor);
	m_pen = &NewPen;
	m_OldPen = m_dc->SelectObject(m_pen);
}

void CEyeDrawDlg::OnLarge() 
{
	//switch to a larger pen
	m_iPenSize = 8;
	CPen NewPen(PS_SOLID,m_iPenSize,m_nCurForeColor);
	m_pen = &NewPen;
	m_OldPen = m_dc->SelectObject(m_pen);
}

void CEyeDrawDlg::Save() 
{
	CFile f;
	//Create a new file and open to write
	if(!f.Open(m_strFileName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
		
		//m.hancock - PLID 18077 - 10/27/05 - The call to CreateDirectory is passed the 
		//wrong directory to be created for storing the eye sketches.  Since this is a 
		//reusable dialog, we need to chop off the filename portion of m_strFileName and
		//create the path.  This way the directory that is created is not hard coded as
		//the Refractive directory but rather the file path specified by m_strFileName is created.
		
		//Cut off the file name and store the path to be created
		int nLoc = m_strFileName.ReverseFind('\\');
		CString strPath = m_strFileName.Left(m_strFileName.GetLength() - (m_strFileName.GetLength() - nLoc)) + "\\";
		
		//Create the path
		CreateDirectory(strPath, NULL);
		
		//Try to create the file again
		if(!f.Open(m_strFileName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			AfxMessageBox("The eye sketch file could not be created. Contact Nextech for assistance.");
			return;
		}
	}
	m_iFileSize = m_aryDrawEvents.GetSize();  //first get the number of brush strokes
	CArchive ar( &f, CArchive::store, 512);   //start storing
	ar << m_iFileSize;		//add that number
	//add all brush strokes
	for(int i=0;i<m_aryDrawEvents.GetSize();i++)
		m_aryDrawEvents.GetAt(i)->Serialize(ar);
	ar.Close();
	f.Close();
}

void CEyeDrawDlg::Load() 
{
	CFile f;
	//open existing file to read
	if(!f.Open(m_strFileName, CFile::modeRead | CFile::shareCompat ) )
		return;

	CArchive ar( &f, CArchive::load, 512); //start loading
	ClearDrawItems();	//clear all brush strokes
	ar >> m_iFileSize;	//get the number of brush strokes
	for(int i=0;i<m_iFileSize;i++) {
		m_aryDrawEvents.Add(new CDrawEvent);   //create event
		m_aryDrawEvents.GetAt(i)->Serialize(ar);   //fill it (doesn't auto-create)
	}
	ar.Close();
	f.Close();

	//update undo status
	if(!CanUndo())
		GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);	
	
	//redraw
	//Invalidate();
	RedrawWindow();
}

void CEyeDrawDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	//if in the drawing space, set the cursor
	if(PtInRect(&m_rcImageRect,point)) {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_CROSS));
	}
	else {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
	//if Lbutton clicked, call continue draw
	if((nFlags & MK_LBUTTON) == MK_LBUTTON)
		ContinueDraw(nFlags,point);
	
	CDialog::OnMouseMove(nFlags, point);
}

void CEyeDrawDlg::OnPaint() 
{
	CDialog::OnPaint();

	m_Image.GetWindowRect(m_rcImageRect);
	ScreenToClient(m_rcImageRect);
	//copy the stored bitmap into the drawing space
	GetDC()->BitBlt(m_rcImageRect.left,m_rcImageRect.top,m_rcImageRect.Width(),m_rcImageRect.Height(),&m_memdc,0,0,SRCCOPY);
	//Get the pen
	m_OldPen = m_dc->SelectObject(m_pen);
	//now loop through the array of draw events and draw all the lines
	CDrawEvent *pEvent = NULL;
	long nCount = m_aryDrawEvents.GetSize();
	for (long i=0; i<nCount; i++) {
		pEvent = (CDrawEvent *)m_aryDrawEvents.GetAt(i);
		if (pEvent) {
			pEvent->Draw(GetDC());
		}
	}
}

void CEyeDrawDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//if in the drawing space, set the cursor
	if(PtInRect(&m_rcImageRect,point)) {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_CROSS));
	}
	else {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
	//start drawing
	StartDraw(nFlags,point);
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CEyeDrawDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//if in the drawing space, set the cursor (otherwise it will revert to an arrow again)
	if(PtInRect(&m_rcImageRect,point)) {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_CROSS));
	}
	else {
		DestroyCursor(GetCursor());
		SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
	//stop drawing
	EndDraw();
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CEyeDrawDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	ClearDrawItems();

	//free up resources
	m_dc->SelectObject(m_OldPen);
	m_dc->DeleteDC();
}

//checks to see if the DrawEvent array is 0, and therefore there are no strokes to undo
bool CEyeDrawDlg::CanUndo()
{
	if(m_aryDrawEvents.GetSize()==0)
		return false;
	else
		return true;
}

//called from LbuttonDown and ContinueDraw, this begins draw mode and accounts for 1-point dots
void CEyeDrawDlg::StartDraw(UINT nFlags, CPoint point) {

	/*set the last point to be this point, since there is no last point (it is initialized in
	OnInitDialog for safety. This should be outside the PtInRect incase you start a stroke from
	outside the drawing space*/
	m_ptLastPoint = point;
	
	//if in the drawing space...
	if(PtInRect(&m_rcImageRect,point)) {

		m_dc = GetDC();
		CPen NewPen(PS_SOLID,m_iPenSize,m_nCurForeColor);
		m_pen = &NewPen;
		m_OldPen = m_dc->SelectObject(m_pen);

		//start a new drawing event, with the current pensize and color
		m_pCurDrawEvent = new CDrawEvent;
		m_aryDrawEvents.Add(m_pCurDrawEvent);
		m_pCurDrawEvent->m_nForeColor = m_nCurForeColor;
		m_pCurDrawEvent->m_nPenSize = m_iPenSize;

		//start draw mode
		m_bDrawMode = true;
		//Get the pen
		m_OldPen = m_dc->SelectObject(m_pen);
		/*currently there is no size 1 pen, but there used to be. the moveto(point)/lineto(samepoint)
		didn't work with a one pixel pen, so you need to use SetPixel*/
		if(m_iPenSize==1) {
			m_dc->SetPixel(point,m_nCurForeColor);
			m_pCurDrawEvent->m_aryDrawToPoints.Add(new CPoint(point));
		}
		//since this is vector-based, you can't have a single point. So we are making one point.
		else {
			m_dc->MoveTo(m_ptLastPoint); //just for safety
			m_dc->MoveTo(point); //move to this point now
			m_pCurDrawEvent->m_aryDrawToPoints.Add(new CPoint(point)); //add the starting point
			m_dc->LineTo(point); //draw the line
			m_pCurDrawEvent->m_aryDrawToPoints.Add(new CPoint(point)); //add the ending point
		}
	}
}

//called from onmousemove, this continues draw mode, or begins it if it is not already drawing
void CEyeDrawDlg::ContinueDraw(UINT nFlags, CPoint point) {

	//if in draw space
	if(PtInRect(&m_rcImageRect,point)) {
		//if not drawing, start drawing! this is so you can draw outside and back inside the window
		if(!m_bDrawMode)
			StartDraw(nFlags,point);
		m_dc->MoveTo(m_ptLastPoint); //move to the last point
		m_dc->LineTo(point); //draw to this point
		m_pCurDrawEvent->m_aryDrawToPoints.Add(new CPoint(point)); //add this point to the array
		m_ptLastPoint = point; //set this one as last
	}
	//if you draw outside the draw space, stop drawing
	else EndDraw();
}

//called from LButtonUp, this ends draw mode and updates the Undo button status
void CEyeDrawDlg::EndDraw() {
	//stop drawing
	if (m_bDrawMode)
		m_bDrawMode = false;
	//update undo status
	if(!CanUndo())
		GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
}

void CEyeDrawDlg::ClearDrawItems()
{
	//delete the array of draw events
	CDrawEvent *pEvent = NULL;
	long nCount = m_aryDrawEvents.GetSize();
	for (long i=0; i<nCount; i++) {
		pEvent = (CDrawEvent *)m_aryDrawEvents.GetAt(i);
		if (pEvent) {
			pEvent->Clear();
		}
		delete pEvent;
	}

	m_aryDrawEvents.RemoveAll();
}
