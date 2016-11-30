// (r.gonet 09/21/2011) - PLID 45555 - Added

// LabCustomFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabCustomField.h"
#include "LabCustomFieldsDlg.h"
#include "LabCustomFieldsView.h"

// CLabCustomFieldsDlg dialog

IMPLEMENT_DYNAMIC(CLabCustomFieldsDlg, CNxDialog)

// (r.gonet 09/21/2011) - PLID 45555 - Create the dialog along with a new fields view that is in value editing mode since we are going to be editing instances of fields.
CLabCustomFieldsDlg::CLabCustomFieldsDlg(CCFTemplateInstance *pTemplateInstance, CWnd* pParent /*=NULL*/)
: CNxDialog(CLabCustomFieldsDlg::IDD, pParent), m_dlgFieldsView(this, pTemplateInstance, CLabCustomFieldControlManager::emValues)
{
}

// (r.gonet 09/21/2011) - PLID 45555 - We don't need to destroy anything.
CLabCustomFieldsDlg::~CLabCustomFieldsDlg()
{
}

void CLabCustomFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAB_CF_DLG_COLOR, m_nxcolor);
	DDX_Control(pDX, IDC_LAB_CF_VIEW_PLACEHOLDER, m_nxsPlaceholder);
	DDX_Control(pDX, IDOK, m_nxbClose);
}


BEGIN_MESSAGE_MAP(CLabCustomFieldsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CLabCustomFieldsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLabCustomFieldsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CLabCustomFieldsDlg message handlers

// (r.gonet 09/21/2011) - PLID 45555
BOOL CLabCustomFieldsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// Create the fields view on the spot in the spot where placeholder is.
		m_dlgFieldsView.Create(IDD_LAB_CUSTOM_FIELDS_VIEW, this);	

		CRect rcPlaceholderRect;
		m_nxsPlaceholder.GetWindowRect(&rcPlaceholderRect);
		ScreenToClient(&rcPlaceholderRect);
		
		m_dlgFieldsView.MoveWindow(&rcPlaceholderRect);
		m_dlgFieldsView.SetWindowPos(&m_nxsPlaceholder, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		m_dlgFieldsView.ShowWindow(SW_SHOW);
		m_dlgFieldsView.ShowControls(true);
		m_nxbClose.AutoSet(NXB_CLOSE);

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 09/21/2011) - PLID 45555 - Save the custom field values when the dialog is closed.
void CLabCustomFieldsDlg::OnBnClickedOk()
{
	try {
		SaveAndClose();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45555
bool CLabCustomFieldsDlg::SaveAndClose()
{
	try {
		// Validate input
		if(m_dlgFieldsView.SyncControlsToFields()) {
			CNxDialog::OnOK();
			return true;
		}
	} NxCatchAll(__FUNCTION__);

	return false;
}