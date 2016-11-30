// ContactSelectImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "ContactSelectImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CContactSelectImageDlg dialog


CContactSelectImageDlg::CContactSelectImageDlg(long nProviderID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CContactSelectImageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContactSelectImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nProviderID = nProviderID;
}


void CContactSelectImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactSelectImageDlg)
	DDX_Control(pDX, IDC_CONTACT_IMAGE, m_btnImage);
	DDX_Control(pDX, IDC_SELECT_IMAGE, m_btnSelectImage);
	DDX_Control(pDX, IDC_CLEAR_IMAGE, m_btnClearImage);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContactSelectImageDlg, CNxDialog)
	//{{AFX_MSG_MAP(CContactSelectImageDlg)
	ON_BN_CLICKED(IDC_SELECT_IMAGE, OnSelectImage)
	ON_BN_CLICKED(IDC_CLEAR_IMAGE, OnClearImage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactSelectImageDlg message handlers

BOOL CContactSelectImageDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnSelectImage.AutoSet(NXB_MODIFY);
		m_btnClearImage.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_strImageFile = VarString(GetTableField("ProvidersT", "ImagePath", "PersonID", m_nProviderID), "");
		UpdateView();
	
	}NxCatchAll("CContactSelectImageDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CContactSelectImageDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 06/07/2007) - PLID 23862 - Set the image control
	HBITMAP hImage;
	if(!m_strImageFile.IsEmpty() && LoadImageFile(GetContactImagePath() ^ m_strImageFile, hImage, -1)) {
		m_btnImage.m_image = hImage;
		m_btnImage.ShowWindow(SW_SHOW);
	} 
	else {
		m_btnImage.m_image = NULL;
		m_btnImage.ShowWindow(SW_HIDE);
	}
	m_btnImage.Invalidate();
}

// (z.manning, 06/07/2007) - PLID 23862 - Open the image select dialog and save the new image file if they commit it.
void CContactSelectImageDlg::OnSelectImage() 
{
	try
	{
		CSelectImageDlg dlg(this);
		dlg.m_strImagePath = GetContactImagePath();
		dlg.m_strMainTabName = "Provider images";
		dlg.m_nPatientID = -1;
		if (dlg.DoModal() == IDOK) 
		{
			ASSERT(dlg.m_nImageType == itDiagram);

			ExecuteSql("UPDATE ProvidersT SET ImagePath = '%s' WHERE PersonID = %li", _Q(dlg.m_strFileName), m_nProviderID);
			m_strImageFile = dlg.m_strFileName;
			UpdateView();
		}

	}NxCatchAll("CContactSelectImageDlg::OnSelectImage");
}

void CContactSelectImageDlg::OnClearImage()
{
	if(m_btnImage.m_image != NULL) {
		// (z.manning, 06/19/2007) - PLID 23862 - If they have an image, confirm the clearing of it.
		if(MessageBox("Are you sure you want to remove this image?",NULL,MB_YESNO) != IDYES) {
			return;
		}
	}

	// (z.manning, 06/19/2007) - PLID 23862 - Now clear out the image.
	ExecuteSql("UPDATE ProvidersT SET ImagePath = '' WHERE PersonID = %li", m_nProviderID);
	m_strImageFile.Empty();
	UpdateView();
}

// (z.manning, 06/13/2007) - PLID 26255 - Moved GetContactImagePath to GlobalUtils.