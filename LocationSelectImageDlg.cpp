// LocationSelectImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "LocationSelectImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocationSelectImageDlg dialog

// (d.moore 2007-07-16 15:16) - PLID 14799 - This dialog was created to handle 
//  adding images for locations. Currently images may be either a logo for the 
//  location or a general picture (such as a picture of the building or of the 
//  staff).


// Require both a location ID and a value to indicate if we are selecting
//  a logo or other image.
CLocationSelectImageDlg::CLocationSelectImageDlg(long nLocationID, EImageType nType, CWnd* pParent)
	: CNxDialog(CLocationSelectImageDlg::IDD, pParent)
	, m_nLogoWidth(100)
	, m_nImageType(nType)
	, m_nLocationID(nLocationID)
{
	//{{AFX_DATA_INIT(CLocationSelectImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLocationSelectImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocationSelectImageDlg)
	DDX_Control(pDX, IDC_LOCATION_IMAGE, m_btnLocationImage);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_IMAGE, m_btnSelectImage);
	DDX_Control(pDX, IDC_CLEAR_IMAGE, m_btnClearImage);
	DDX_Control(pDX, IDC_LOCATION_IMAGE_LABEL, m_nxstaticLocationImageLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocationSelectImageDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLocationSelectImageDlg)
	ON_BN_CLICKED(IDC_SELECT_IMAGE, OnSelectImage)
	ON_BN_CLICKED(IDC_CLEAR_IMAGE, OnClearImage)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_LOCATION_LOGO_WIDTH_EDIT, &CLocationSelectImageDlg::OnEnKillfocusLocationLogoWidthEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocationSelectImageDlg message handlers

BOOL CLocationSelectImageDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		// (z.manning, 05/01/2008) - PLID 29864 - Set button styles
		m_btnCancel.AutoSet(NXB_CLOSE);
		m_btnClearImage.AutoSet(NXB_DELETE);
		m_btnSelectImage.AutoSet(NXB_MODIFY);

		if (m_nImageType != eitLogo) {
			// (a.walling 2010-09-16 10:46) - PLID 31435 - hide the preview pane width controls

			CWnd* pLogoWidthEdit = SafeGetDlgItem<CWnd>(IDC_LOCATION_LOGO_WIDTH_EDIT);
			CWnd* pLogoWidthLabel = SafeGetDlgItem<CWnd>(IDC_LOCATION_LOGO_WIDTH_LABEL);

			CRect rcImage;
			m_btnLocationImage.GetWindowRect(rcImage);
			ScreenToClient(rcImage);

			CRect rcEdit;
			pLogoWidthEdit->GetWindowRect(rcEdit);
			ScreenToClient(rcEdit);

			rcImage.bottom = rcEdit.bottom;

			pLogoWidthEdit->ShowWindow(SW_HIDE);
			pLogoWidthLabel->ShowWindow(SW_HIDE);

			pLogoWidthEdit->EnableWindow(FALSE);
			pLogoWidthLabel->EnableWindow(FALSE);

			m_btnLocationImage.MoveWindow(rcImage);
		}

		switch(m_nImageType) {
			case eitLogo:
				m_strImagePathDbField = "LogoImagePath";
				break;
			case eitGeneralImage:
				m_strImagePathDbField = "GeneralImagePath";
				break;
		}

		// (a.walling 2010-09-16 10:51) - PLID 31435 - Load the values
		m_strImagePath = GetLocationImagePath();

		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			FormatString(
				"SELECT %s, LogoWidth FROM LocationsT WHERE ID = {INT}"
				, m_strImagePathDbField)
			, m_nLocationID);

		if (!prs->eof) {
			m_strImageFile = AdoFldString(prs, m_strImagePathDbField, "");
			m_nLogoWidth = AdoFldLong(prs, "LogoWidth", 100);
		}

		if (m_nLogoWidth <= 0 || m_nLogoWidth > 100) {
			m_nLogoWidth = 100;
		}

		UpdateView();
	
	}NxCatchAll("Error In: CLocationSelectImageDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Open the image select dialog and save the new image file if they commit it.
void CLocationSelectImageDlg::OnSelectImage() 
{
	try
	{
		CSelectImageDlg dlg(this);
		dlg.m_strImagePath = m_strImagePath;
		dlg.m_strMainTabName = "Location Images";
		dlg.m_nPatientID = -1;
		if (dlg.DoModal() == IDOK) 
		{
			ASSERT(dlg.m_nImageType == itDiagram);

			ExecuteSql(
				"UPDATE LocationsT "
				"SET %s = '%s' "
				"WHERE ID = %li", 
				m_strImagePathDbField, 
				_Q(dlg.m_strFileName), 
				m_nLocationID);
			
			m_strImageFile = dlg.m_strFileName;

			// (a.walling 2008-07-01 16:32) - PLID 30586
			if (m_nLocationID == GetCurrentLocationID() && m_strImagePathDbField == "LogoImagePath") {
				// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
				SetCurrentLocationLogo(dlg.m_strFileName, m_nLogoWidth);
			}
			UpdateView();
		}

	}NxCatchAll("Error In: CLocationSelectImageDlg::OnSelectImage");
}

void CLocationSelectImageDlg::OnClearImage() 
{
	try {
		if(m_btnLocationImage.m_image != NULL) {
			// If they have an image, confirm the clearing of it.
			if(MessageBox("Are you sure you want to remove this image?",NULL,MB_YESNO) != IDYES) {
				return;
			}
		}

		// Now clear out the image.
		ExecuteSql(
			"UPDATE LocationsT "
			"SET %s = '' "
			"WHERE ID = %li", 
			m_strImagePathDbField, 
			m_nLocationID);
		
		// (a.walling 2008-07-01 16:32) - PLID 30586
		if (m_nLocationID == GetCurrentLocationID() && m_strImagePathDbField == "LogoImagePath") {
			// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
			SetCurrentLocationLogo("", m_nLogoWidth);
		}

		m_strImageFile.Empty();
		UpdateView();
	} NxCatchAll("Error In: CLocationSelectImageDlg::OnClearImage");
}

void CLocationSelectImageDlg::OnCancel() 
{
	try {
		// (a.walling 2010-09-16 10:51) - PLID 31435 - Save the width
		if (SaveLogoWidth()) {
			CNxDialog::OnCancel();
		}
	} NxCatchAll(__FUNCTION__);
}

void CLocationSelectImageDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// Set the image control.

	// (a.walling 2009-12-16 14:08) - PLID 31062 - Clear out the old image handle, if it exists
	if (m_btnLocationImage.m_image) {
		::DeleteObject(m_btnLocationImage.m_image);
		m_btnLocationImage.m_image = NULL;
	}
	HBITMAP hImage;
	if(!m_strImageFile.IsEmpty() 
		&& LoadImageFile(m_strImagePath ^ m_strImageFile, hImage, -1)) {
		m_btnLocationImage.m_image = hImage;
		m_btnLocationImage.ShowWindow(SW_SHOW);
	} 
	else {
		m_btnLocationImage.m_image = NULL;
		m_btnLocationImage.ShowWindow(SW_HIDE);
	}

	// (a.walling 2008-08-13 16:54) - PLID 30099 - Redraw the window
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);

	// (a.walling 2010-09-16 10:51) - PLID 31435 - Set the width in the edit box
	SetDlgItemInt(IDC_LOCATION_LOGO_WIDTH_EDIT, m_nLogoWidth);
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CLocationSelectImageDlg::OnOK()
{
	//Eat the message
}

void CLocationSelectImageDlg::OnEnKillfocusLocationLogoWidthEdit()
{
	try {
		// (a.walling 2010-09-16 10:51) - PLID 31435 - Save the width
		SaveLogoWidth();
	} NxCatchAll(__FUNCTION__);
}

bool CLocationSelectImageDlg::SaveLogoWidth()
{	
	// (a.walling 2010-09-16 10:51) - PLID 31435 - Save the width
	if (m_nImageType != eitLogo) {
		return true;
	}

	int nNewWidth = GetDlgItemInt(IDC_LOCATION_LOGO_WIDTH_EDIT);

	if (nNewWidth == m_nLogoWidth) {
		return true;
	}

	if (nNewWidth <= 0 || nNewWidth > 100) {
		// (a.walling 2011-01-24 16:29) - PLID 31435 - Set the value before popping up the messagebox, since that will cause the lostfocus message to be handled.
		SetDlgItemInt(IDC_LOCATION_LOGO_WIDTH_EDIT, m_nLogoWidth);
		MessageBox("A valid width must be between 1 and 100, designating the percentage of the page width that the logo should occupy.", NULL, MB_ICONSTOP);
		return false;
	}

	ExecuteParamSql("UPDATE LocationsT SET LogoWidth = {INT} WHERE ID = {INT}", nNewWidth, m_nLocationID);

	m_nLogoWidth = nNewWidth;

	if (m_nLocationID == GetCurrentLocationID()) {
		// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
		SetCurrentLocationLogo(m_strImageFile, m_nLogoWidth);
	}

	return true;
}
