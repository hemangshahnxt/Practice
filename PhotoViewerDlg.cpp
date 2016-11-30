// PhotoViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "PhotoViewerDlg.h"
#include "GlobalDrawingUtils.h"
#include "IconUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerDlg dialog

using namespace ADODB;
CPhotoViewerDlg::CPhotoViewerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPhotoViewerDlg::IDD, pParent)
{
	m_nIndex = 0;
	m_pIconFont = NULL;
	m_pIconFontPrint = NULL;
	//{{AFX_DATA_INIT(CPhotoViewerDlg)
	//}}AFX_DATA_INIT
}

CPhotoViewerDlg::~CPhotoViewerDlg()
{
	if(m_pIconFont) {
		m_pIconFont->DeleteObject();
		delete m_pIconFont;
		m_pIconFont = NULL;
	}
	if(m_pIconFontPrint) {
		m_pIconFontPrint->DeleteObject();
		delete m_pIconFontPrint;
		m_pIconFontPrint = NULL;
	}
}


void CPhotoViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhotoViewerDlg)
	DDX_Control(pDX, IDC_PRINT_PHOTOS, m_btnPrint);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_PHOTO_RIGHT, m_btnRight);
	DDX_Control(pDX, IDC_PHOTO_PAGE_RIGHT, m_btnPageRight);
	DDX_Control(pDX, IDC_PHOTO_LEFT, m_btnLeft);
	DDX_Control(pDX, IDC_PHOTO_PAGE_LEFT, m_btnPageLeft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPhotoViewerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPhotoViewerDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_PHOTO_PAGE_LEFT, OnPhotoPageLeft)
	ON_BN_CLICKED(IDC_PHOTO_LEFT, OnPhotoLeft)
	ON_BN_CLICKED(IDC_PHOTO_PAGE_RIGHT, OnPhotoPageRight)
	ON_BN_CLICKED(IDC_PHOTO_RIGHT, OnPhotoRight)
	ON_BN_CLICKED(IDC_PRINT_PHOTOS, OnPrintPhotos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPhotoViewerDlg, CNxDialog)
	ON_EVENT(CPhotoViewerDlg, IDC_PHOTO_VIEW_FONTSIZE_COMBO, 1, OnSelChangingFontSizeList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPhotoViewerDlg, IDC_PHOTO_VIEW_FONTSIZE_COMBO, 16, OnSelChosenFontSizeList, VTS_DISPATCH)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerDlg message handlers

BOOL CPhotoViewerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	

	//OK, first, let's size ourselves to be as big as possible.
	CRect rDesktop;
	SystemParametersInfo(SPI_GETWORKAREA,NULL, &rDesktop, NULL);
	MoveWindow(rDesktop);

	//(e.lally 2012-04-23) PLID 29583
	g_propManager.CachePropertiesInBulk("PhotoViewerDlg", propNumber,
		"(Username IN('<None>', '%s') AND "
		"Name IN('PhotoPreviewFontSize' "
		", 'PhotoPreviewNotes' "
		") )", GetCurrentUserName());

	//(e.lally 2012-04-23) PLID 29583 - Build our font size list
	m_pFontSizeList = BindNxDataList2Ctrl(IDC_PHOTO_VIEW_FONTSIZE_COMBO, false);
	m_nFontSize = GetRemotePropertyInt("PhotoPreviewFontSize", 24, 0, GetCurrentUserName(), true);
	CArray<long, long> aryAvailFontSizes;
	aryAvailFontSizes.Add(10);
	aryAvailFontSizes.Add(12);
	aryAvailFontSizes.Add(14);
	aryAvailFontSizes.Add(16);
	aryAvailFontSizes.Add(20);
	aryAvailFontSizes.Add(24);
	aryAvailFontSizes.Add(28);

	NXDATALIST2Lib::IRowSettingsPtr pRow;
	for(int i=0; i < aryAvailFontSizes.GetCount(); i++){
		pRow = m_pFontSizeList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(0, (long)aryAvailFontSizes.GetAt(i));
			m_pFontSizeList->AddRowSorted(pRow, NULL);
			if(m_nFontSize == aryAvailFontSizes.GetAt(i)){
				//This was the previous selection
				m_pFontSizeList->CurSel = pRow;
			}
		}
	}

	//Now, move all our buttons.
	//The OK button is flush with the bottom, horizontally centered.
	CRect rClient;
	GetClientRect(&rClient);
	CRect rButton;
	CRect rOK;
	GetDlgItem(IDOK)->GetClientRect(&rOK);
	rButton = rOK;
	int nLeft = rClient.left + rClient.Width()/2 - rOK.Width()/2;
	GetDlgItem(IDOK)->SetWindowPos(NULL, nLeft, rClient.bottom-rButton.Height(), 0, 0, SWP_NOSIZE);
	m_btnOK.AutoSet(NXB_CLOSE);
	//Left button
	m_btnLeft.GetClientRect(&rButton);
	nLeft = nLeft - 5 - rButton.Width();
	m_btnLeft.SetWindowPos(NULL, nLeft, rClient.bottom-rButton.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_btnLeft.AutoSet(NXB_LEFT);
	//Page left button
	m_btnPageLeft.GetClientRect(&rButton);
	nLeft = nLeft - 5 - rButton.Width();
	m_btnPageLeft.SetWindowPos(NULL, nLeft, rClient.bottom-rButton.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_btnPageLeft.AutoSet(NXB_LLEFT);
	//Right button.
	nLeft = rClient.left + rClient.Width()/2 + rOK.Width()/2 + 5;
	m_btnRight.GetClientRect(rButton);
	m_btnRight.SetWindowPos(NULL, nLeft, rClient.bottom - rButton.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_btnRight.AutoSet(NXB_RIGHT);
	//Page right button.
	nLeft = nLeft + rButton.Width() + 5;
	m_btnPageRight.GetClientRect(rButton);
	m_btnPageRight.SetWindowPos(NULL, nLeft, rClient.bottom - rButton.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_btnPageRight.AutoSet(NXB_RRIGHT);
	//Print button.
	m_btnPrint.GetClientRect(rButton);
	m_btnPrint.SetWindowPos(NULL, rClient.right-5-rButton.Width(), rClient.bottom-rButton.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_btnPrint.AutoSet(NXB_PRINT);

	//(e.lally 2012-04-23) PLID 29583 - Move the font size list and label
	//Font size list.
	CRect rFontSize, rPrintWnd;
	m_btnPrint.GetWindowRect(&rPrintWnd);
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_COMBO)->GetClientRect(&rFontSize);
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_COMBO)->SetWindowPos(NULL, (rPrintWnd.left - rFontSize.Width()) - 5, rClient.bottom-rFontSize.Height()-5, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	//Font size label
	CRect rFontSizeLbl, rFontSizeWnd;
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_COMBO)->GetWindowRect(&rFontSizeWnd);
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_LBL)->GetClientRect(&rFontSizeLbl);
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_LBL)->SetWindowPos(NULL, (rFontSizeWnd.left - rFontSizeLbl.Width()) - 5, rClient.bottom-rFontSizeLbl.Height()-5, 0, 0, SWP_NOSIZE|SWP_NOZORDER);


	//Enable/disable scrolling.
	//First off, if we're only showing 1 image, the "paging" is meaningless.
	if(m_ilLayout == il1Image) {
		m_btnPageLeft.ShowWindow(SW_HIDE);
		m_btnPageRight.ShowWindow(SW_HIDE);
	}
	//Now, do we have more items than we're showing.
	int nPageSize;
	switch(m_ilLayout) {
	case il1Image:
		nPageSize = 1;
		break;
	case il2TopBottom:
	case il2SideSide:
		nPageSize = 2;
		break;
	case il4Images:
		nPageSize = 4;
		break;
	}
	if(m_arImages.GetSize() <= nPageSize) {
		//Nope, disable scrolling.
		m_btnPageLeft.EnableWindow(FALSE);
		m_btnLeft.EnableWindow(FALSE);
		m_btnRight.EnableWindow(FALSE);
		m_btnPageRight.EnableWindow(FALSE);
	}

	m_strPatientName.Trim();

	if (!m_strPatientName.IsEmpty()) {
		CString strCaption;
		strCaption.Format("Images for %s", m_strPatientName);
		// (j.jones 2016-05-11 11:28) - NX-100505 - now we actually show this caption
		// in the window header - what a novel concept!
		SetWindowText(strCaption);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPhotoViewerDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBorrowDeviceContext bdc(&dc);
	
	//OK, let's draw our first image in our first rectangle.
	CRect rFirstRect, rFirstTextRect;
	CRect rClient;
	GetClientRect(&rClient);
	CRect rOKButton;
	m_btnOK.GetClientRect(&rOKButton);
	rClient.DeflateRect(0,0,0,rOKButton.Height());
	BOOL bIncludeNotes = GetRemotePropertyInt("PhotoPreviewNotes", 0, 0, GetCurrentUserName(), true);
	
	if(bIncludeNotes) {
		//Prepare for text drawing.
		// (z.manning, 05/12/2008) - PLID 30013 - Let NxDialog handle the background coloring
		//dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
		
		//Let's load the system icon font (except bigger).
		if(!m_pIconFont) {
			LOGFONT lf;
			if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
				//We want a 24 point font (I copied this formula off the MSDN).
				//(e.lally 2012-04-23) PLID 29583 - Use selected font size value now
				lf.lfHeight = -MulDiv(m_nFontSize, GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY), 72);
				m_pIconFont = new CFont;
				m_pIconFont->CreateFontIndirect(&lf);
			}
		}
		dc.SelectObject(m_pIconFont);
	}

	//In all cases, big as possible with 5 pixels padding.
	switch(m_ilLayout) {
	case il1Image:
		rFirstRect = rClient;
		rClient.DeflateRect(5,5);
		break;
	case il2TopBottom:
		rFirstRect = CRect(rClient.left+5, rClient.top+5, rClient.right-5, rClient.top + rClient.Height()/2 - 5);
		break;
	case il2SideSide:
		rFirstRect = CRect(rClient.left+5, rClient.top+5, rClient.left + rClient.Width()/2 - 5, rClient.bottom-5);
		break;
	case il4Images:
		rFirstRect = CRect(rClient.left+5, rClient.top+5, rClient.left + rClient.Width()/2 - 5, rClient.top + rClient.Height()/2 -5);
		break;
	}
	//Now, draw our first image (if we have one).
	if(m_arImages.GetSize() > 0) {
		if(bIncludeNotes) {
			//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
			CString strText = m_arImages.GetAt(m_nIndex)->GetTextToDraw();
			CalcRects(&dc, strText, rFirstRect, rFirstTextRect);
			dc.DrawText(strText, &rFirstTextRect, DT_WORDBREAK|DT_CENTER);
		}
		DrawDIBitmapInRect(&dc, rFirstRect, m_arImages.GetAt(m_nIndex)->hFullImage);
	}
	
	//Now our next image.
	if(m_ilLayout == il2TopBottom || m_ilLayout == il2SideSide || m_ilLayout == il4Images) {
		int nNextImage = m_nIndex+1;
		if(nNextImage >= m_arImages.GetSize() && nNextImage > 1) {
			nNextImage = 0;
		}
		CRect rSecondRect, rSecondTextRect;
		switch(m_ilLayout)
		{
		case il2TopBottom:
			rSecondRect = CRect(rClient.left+5, rClient.top + rClient.Height()/2 + 5, rClient.right-5, rClient.bottom-5);
			break;
		case il2SideSide:
			rSecondRect = CRect(rClient.left + rClient.Width()/2 + 5, rClient.top+5, rClient.right-5, rClient.bottom-5);
			break;
		case il4Images:
			rSecondRect = CRect(rClient.left + rClient.Width()/2 + 5, rClient.top+5, rClient.right-5, rClient.top + rClient.Height()/2 - 5);
			break;
		}
		//If we have an image here, draw it.
		if(m_arImages.GetSize() > nNextImage) {
			if(bIncludeNotes) {
				//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
				CString strText = m_arImages.GetAt(nNextImage)->GetTextToDraw();
				CalcRects(&dc, strText, rSecondRect, rSecondTextRect);
				dc.DrawText(strText, &rSecondTextRect, DT_WORDBREAK|DT_CENTER);
			}
			DrawDIBitmapInRect(&dc, rSecondRect, m_arImages.GetAt(nNextImage)->hFullImage);
		}

		//Now the last 2
		if(m_ilLayout == il4Images) {
			int nThirdImage = nNextImage+1;
			if(nThirdImage >= m_arImages.GetSize() && nThirdImage > 2) {
				nThirdImage = 0;
			}
			int nFourthImage = nThirdImage+1;
			if(nFourthImage >= m_arImages.GetSize() && nFourthImage > 3) {
				nFourthImage = 0;
			}
			CRect rThirdRect(rClient.left+5, rClient.top + rClient.Height()/2 + 5, rClient.left + rClient.Width()/2 - 5, rClient.bottom-5);
			CRect rFourthRect(rClient.left + rClient.Width()/2 + 5, rClient.top + rClient.Height()/2 + 5, rClient.right - 5, rClient.bottom - 5);
			CRect rThirdTextRect, rFourthTextRect;
			//Draw 'em if you got 'em.
			if(m_arImages.GetSize() > nThirdImage) {
				if(bIncludeNotes) {
					//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
					CString strText = m_arImages.GetAt(nThirdImage)->GetTextToDraw();
					CalcRects(&dc, strText, rThirdRect, rThirdTextRect);
					dc.DrawText(strText, &rThirdTextRect, DT_WORDBREAK|DT_CENTER);
				}
				DrawDIBitmapInRect(&dc, rThirdRect, m_arImages.GetAt(nThirdImage)->hFullImage);
			}
			if(m_arImages.GetSize() > nFourthImage) {
				if(bIncludeNotes) {
					//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
					// (j.jones 2013-04-02 10:36) - PLID 55885 - fixed to pull from the 4th image, it was pulling notes from the 3rd image
					CString strText = m_arImages.GetAt(nFourthImage)->GetTextToDraw();
					CalcRects(&dc, strText, rFourthRect, rFourthTextRect);
					dc.DrawText(strText, &rFourthTextRect, DT_WORDBREAK|DT_CENTER);
				}
				DrawDIBitmapInRect(&dc, rFourthRect, m_arImages.GetAt(nFourthImage)->hFullImage);
			}
		}
	}

	//(e.lally 2012-04-23) PLID 29583
	GetDlgItem(IDC_PHOTO_VIEW_FONTSIZE_LBL)->Invalidate();

	// Do not call CDialog::OnPaint() for painting messages
}

void CPhotoViewerDlg::OnPhotoPageLeft() 
{
	int nPageSize;
	switch(m_ilLayout) {
	case il1Image:
		nPageSize = 1;
		break;
	case il2TopBottom:
	case il2SideSide:
		nPageSize = 2;
		break;
	case il4Images:
		nPageSize = 4;
		break;
	}

	for(int i = 0; i < nPageSize; i++) {
		m_nIndex--;
		if(m_nIndex < 0) m_nIndex = m_arImages.GetSize()-1;
	}
	Invalidate();
	
}

void CPhotoViewerDlg::OnPhotoLeft() 
{
	m_nIndex--;
	if(m_nIndex < 0) m_nIndex = m_arImages.GetSize()-1;
	Invalidate();
}

void CPhotoViewerDlg::OnPhotoPageRight() 
{
	int nPageSize;
	switch(m_ilLayout) {
	case il1Image:
		nPageSize = 1;
		break;
	case il2TopBottom:
	case il2SideSide:
		nPageSize = 2;
		break;
	case il4Images:
		nPageSize = 4;
		break;
	}

	for(int i = 0; i < nPageSize; i++) {
		m_nIndex++;
		if(m_nIndex >= m_arImages.GetSize()) m_nIndex = 0;
	}
	Invalidate();
}

void CPhotoViewerDlg::OnPhotoRight() 
{
	m_nIndex++;
	if(m_nIndex >= m_arImages.GetSize()) m_nIndex = 0;
	Invalidate();
	
}

void CPhotoViewerDlg::CalcRects(CDC *pDC, const CString &strNotes, CRect &rImage, CRect &rText)
{

	CRect rTempText(0,0,rImage.Width(), 1);
	pDC->DrawText(strNotes, &rTempText, DT_CALCRECT|DT_WORDBREAK|DT_CENTER);
	//The height is either the height of the text, or half the height of the image, whichever is smaller.
	int nHeight = rTempText.Height();
	if(nHeight > rImage.Height()/2) nHeight = rImage.Height()/2;
	int nImageHeight = rImage.Height()-nHeight;

	rImage = CRect(rImage.left, rImage.top, rImage.right, rImage.top+nImageHeight);
	rText = CRect(rImage.left, rImage.bottom, rImage.right, rImage.bottom+nHeight);
}

void CPhotoViewerDlg::OnPrintPhotos() 
{
	try {
		//Here goes nothing...
		//How many pages do we have?
		int nPages, nImagesPerPage;
		switch(m_ilLayout) {
		case il1Image:
			nImagesPerPage = 1;
			nPages = m_arImages.GetSize();
			break;
		case il2TopBottom:
		case il2SideSide:
			nImagesPerPage = 2;
			nPages = m_arImages.GetSize()/2 + (m_arImages.GetSize()%2==0 ? 0 : 1);
			break;
		case il4Images:
			nImagesPerPage = 4;
			nPages = m_arImages.GetSize()/4 + (m_arImages.GetSize()%4==0 ? 0 : 1);
			break;
		}

		PRINTDLG pd;
		pd.lStructSize = sizeof(PRINTDLG);
		pd.hwndOwner = GetSafeHwnd();
		pd.hDevMode = NULL;
		pd.hDevNames = NULL;
		pd.hDC = NULL;
		pd.Flags = PD_HIDEPRINTTOFILE|PD_RETURNDC|PD_NOSELECTION|PD_USEDEVMODECOPIESANDCOLLATE|PD_CURRENTPAGE;
		pd.nFromPage = 1;
		pd.nToPage = 1;
		pd.nMinPage = 1;
		pd.nMaxPage = nPages;
		pd.nCopies = 1;
		pd.hInstance = NULL;
		pd.lCustData = 0;
		pd.lpfnPrintHook = NULL;
		pd.lpfnSetupHook = NULL;
		pd.lpPrintTemplateName = NULL;
		pd.lpSetupTemplateName = NULL;
		pd.hPrintTemplate = NULL;
		pd.hSetupTemplate = NULL;

		//OK, now prompt the user.
		if(PrintDlg(&pd)) {
			CDC *dcPrint = CDC::FromHandle(pd.hDC);

			DWORD caps = GetDeviceCaps(pd.hDC, RASTERCAPS);
			if(!(caps & RC_STRETCHDIB) || !(caps & RC_DI_BITMAP)) {
				MsgBox("Cannot print images: the selected device does not support a necessary operation");
				return;
			}
			BOOL bIncludeNotes = GetRemotePropertyInt("PhotoPreviewNotes", 0, 0, GetCurrentUserName(), true);
			if(bIncludeNotes) {
				if(!m_pIconFontPrint) {
					//Take our drawing font, scale it.
					LOGFONT lf;
					m_pIconFont->GetLogFont(&lf);
					CBorrowDeviceContext bdc(this);
					lf.lfHeight = MulDiv(lf.lfHeight, GetDeviceCaps(dcPrint->GetSafeHdc(), LOGPIXELSY), GetDeviceCaps(bdc.m_pDC->GetSafeHdc(), LOGPIXELSY));
					m_pIconFontPrint = new CFont;
					m_pIconFontPrint->CreateFontIndirect(&lf);
				}
				int nRet = (int)dcPrint->SelectObject(m_pIconFontPrint);
				DWORD dwErr = GetLastError();
			}
			//OK, they want to print.  Let's setup a DOCINFO.
			DOCINFO di;
			di.cbSize = sizeof(DOCINFO);
			di.lpszDocName = m_strPatientName;
			di.lpszOutput = (LPTSTR) NULL; 
			di.lpszDatatype = (LPTSTR) NULL; 
			di.fwType = 0; 
 
			//Begin a print job by calling the StartDoc function. 
			if(StartDoc(dcPrint->GetSafeHdc(), &di) == -1) {
				MsgBox("Failed to start print job!");
				return;
			}
			
			//Now, print each page.
			//Calculate our page range (0-based!)
			int nFirstPage, nLastPage;
			if(pd.Flags & PD_PAGENUMS) {
				nFirstPage = pd.nFromPage-1;
				nLastPage = pd.nToPage-1;
			}
			else if(pd.Flags & PD_CURRENTPAGE) {
				nFirstPage = 0;
				nLastPage = 0;
			}
			else {
				nFirstPage = 0;
				nLastPage = nPages-1;
			}
			
			bool bPrintedLastImage = false;
			for(int i = nFirstPage; i <= nLastPage; i++) {
				if(StartPage(dcPrint->GetSafeHdc()) == -1) {
					MsgBox("Failed to initialize page!");
					return;
				}

				//Now, figure out where to put the images on this page.
				CRect rPage = CRect(0,0,GetDeviceCaps(dcPrint->GetSafeHdc(), HORZRES),GetDeviceCaps(dcPrint->GetSafeHdc(), VERTRES));
				CRect rFirstRect;
				int nFirstImage = m_nIndex + nImagesPerPage * i;
				if(nFirstImage >= m_arImages.GetSize() && nFirstImage > 0) {
					nFirstImage = 0;
				}
				if(nFirstImage != m_nIndex || i == 0) {
					switch(m_ilLayout) {
					case il1Image:
						rFirstRect = rPage;
						rFirstRect.DeflateRect(5,5);
						break;
					case il2TopBottom:
						rFirstRect = CRect(rPage.left+5, rPage.top+5, rPage.right-5, rPage.top + rPage.Height()/2 - 5);
						break;
					case il2SideSide:
						rFirstRect = CRect(rPage.left+5, rPage.top+5, rPage.left + rPage.Width()/2 - 5, rPage.bottom-5);
						break;
					case il4Images:
						rFirstRect = CRect(rPage.left+5, rPage.top+5, rPage.left + rPage.Width()/2 - 5, rPage.top + rPage.Height()/2 -5);
						break;
					}
					if(bIncludeNotes) {
						CRect rText;
						//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
						CString strText = m_arImages.GetAt(nFirstImage)->GetTextToDraw();
						CalcRects(dcPrint, strText, rFirstRect, rText);
						dcPrint->DrawText(strText, &rText, DT_WORDBREAK|DT_CENTER);
					}				
					DrawDIBitmapInRect(dcPrint, rFirstRect, m_arImages.GetAt(nFirstImage)->hFullImage);
				}

				if(m_ilLayout == il2TopBottom || m_ilLayout == il2SideSide || m_ilLayout == il4Images) {
					int nNextImage = nFirstImage+1;
					if(nNextImage >= m_arImages.GetSize() && nNextImage > 1) {
						nNextImage = 0;
					}
					if(nNextImage != m_nIndex) {
						CRect rSecondRect, rSecondTextRect;
						switch(m_ilLayout)
						{
						case il2TopBottom:
							rSecondRect = CRect(rPage.left+5, rPage.top + rPage.Height()/2 + 5, rPage.right-5, rPage.bottom-5);
							break;
						case il2SideSide:
							rSecondRect = CRect(rPage.left + rPage.Width()/2 + 5, rPage.top+5, rPage.right-5, rPage.bottom-5);
							break;
						case il4Images:
							rSecondRect = CRect(rPage.left + rPage.Width()/2 + 5, rPage.top+5, rPage.right-5, rPage.top + rPage.Height()/2 - 5);
							break;
						}
						//If we have an image here, draw it.
						if(m_arImages.GetSize() > nNextImage) {
							if(bIncludeNotes) {
								//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
								CString strText = m_arImages.GetAt(nNextImage)->GetTextToDraw();
								CalcRects(dcPrint, strText, rSecondRect, rSecondTextRect);
								dcPrint->DrawText(strText, &rSecondTextRect, DT_WORDBREAK|DT_CENTER);
							}
							DrawDIBitmapInRect(dcPrint, rSecondRect, m_arImages.GetAt(nNextImage)->hFullImage);
						}

						//Now the last 2
						if(m_ilLayout == il4Images) {
							int nThirdImage = nNextImage+1;
							if(nThirdImage >= m_arImages.GetSize() && nThirdImage > 2) {
								nThirdImage = 0;
							}
							int nFourthImage = nThirdImage+1;
							if(nFourthImage >= m_arImages.GetSize() && nFourthImage > 3) {
								nFourthImage = 0;
							}
							CRect rThirdRect(rPage.left+5, rPage.top + rPage.Height()/2 + 5, rPage.left + rPage.Width()/2 - 5, rPage.bottom-5);
							CRect rFourthRect(rPage.left + rPage.Width()/2 + 5, rPage.top + rPage.Height()/2 + 5, rPage.right - 5, rPage.bottom - 5);
							CRect rThirdTextRect, rFourthTextRect;
							//Draw 'em if you got 'em.
							if(nThirdImage != m_nIndex) {
								if(m_arImages.GetSize() > nThirdImage) {
									if(bIncludeNotes) {
										//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
										CString strText = m_arImages.GetAt(nThirdImage)->GetTextToDraw();
										CalcRects(dcPrint, strText, rThirdRect, rThirdTextRect);
										dcPrint->DrawText(strText, &rThirdTextRect, DT_WORDBREAK|DT_CENTER);
									}
									DrawDIBitmapInRect(dcPrint, rThirdRect, m_arImages.GetAt(nThirdImage)->hFullImage);
								}
								if(nFourthImage != m_nIndex) {
									if(m_arImages.GetSize() > nFourthImage) {
										if(bIncludeNotes) {
											//(e.lally 2012-04-13) PLID 49637 - Use the GetTextToDraw method now
											CString strText = m_arImages.GetAt(nFourthImage)->GetTextToDraw();
											CalcRects(dcPrint, strText, rFourthRect, rFourthTextRect);
											dcPrint->DrawText(strText, &rFourthTextRect, DT_WORDBREAK|DT_CENTER);
										}
										DrawDIBitmapInRect(dcPrint, rFourthRect, m_arImages.GetAt(nFourthImage)->hFullImage);
									}
								}
							}
						}
					}
				}
				if(EndPage(dcPrint->GetSafeHdc()) == -1) {
					MsgBox("Failed to finalize page!");
					return;
				}
			}
   
			//OK, we're done.
			if(EndDoc(dcPrint->GetSafeHdc()) == -1) {
				MsgBox("Failed to finalize document!");
				return;
			}
			DeleteDC(dcPrint->GetSafeHdc()); 
		}
	}NxCatchAll("Error in CPhotoViewerCtrl::OnPrintPhotos()");
}

//(e.lally 2012-04-23) PLID 29583
void CPhotoViewerDlg::OnSelChangingFontSizeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel);

		//Do not let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-23) PLID 29583
void CPhotoViewerDlg::OnSelChosenFontSizeList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		m_nFontSize = VarLong(pRow->GetValue(0));
		SetRemotePropertyInt("PhotoPreviewFontSize", m_nFontSize, 0, GetCurrentUserName());
		//(e.lally 2012-04-23) PLID 29583 - Delete the font so it gets recreated with the new size
		if(m_pIconFont) {
			m_pIconFont->DeleteObject();
			delete m_pIconFont;
			m_pIconFont = NULL;
		}
		Invalidate();

	} NxCatchAll(__FUNCTION__);
}