// EnterpriseSchedulerBlockedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "EnterpriseSchedulerBlockedDlg.h"


//TES 12/23/2008 - PLID 32145 - Created
// CEnterpriseSchedulerBlockedDlg dialog

IMPLEMENT_DYNAMIC(CEnterpriseSchedulerBlockedDlg, CNxDialog)

CEnterpriseSchedulerBlockedDlg::CEnterpriseSchedulerBlockedDlg(CWnd* pParent)
	: CNxDialog(CEnterpriseSchedulerBlockedDlg::IDD, pParent)
{

}

CEnterpriseSchedulerBlockedDlg::~CEnterpriseSchedulerBlockedDlg()
{
}

void CEnterpriseSchedulerBlockedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterpriseSchedulerBlockedDlg)
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDC_OPEN_MANUAL, m_nxbOpenManual);
	DDX_Control(pDX, IDC_CLOSE_CENTERED, m_nxbCloseCentered);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterpriseSchedulerBlockedDlg, CNxDialog)
	ON_BN_CLICKED(IDC_OPEN_MANUAL, &CEnterpriseSchedulerBlockedDlg::OnOpenManual)
	ON_BN_CLICKED(IDC_CLOSE_CENTERED, &CEnterpriseSchedulerBlockedDlg::OnCloseCentered)
END_MESSAGE_MAP()


extern CPracticeApp theApp;
// CEnterpriseSchedulerBlockedDlg message handlers
BOOL CEnterpriseSchedulerBlockedDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//TES 12/23/2008 - PLID 32145 - NxIconify
		m_nxbOK.AutoSet(NXB_CLOSE);
		m_nxbCloseCentered.AutoSet(NXB_CLOSE);

		//TES 12/23/2008 - PLID 32145 - For the main caption, we want a really big font.
		if (!CreateCompatiblePointFont(&m_CaptionFont, 150, "Arial Bold")) {
			return TRUE;
		}
		//TES 01/05/2009 - PLID 32145 - A slightly smaller one for the "For more information..." label (this happens
		// to be theApp.m_subtitleFont, but I create one here for consistency).
		if (!CreateCompatiblePointFont(&m_MoreInfoFont, 120, "Arial Bold")) {
			return TRUE;
		}

		//TES 12/23/2008 - PLID 32145 - For the feature description, we want the same as the MoreInfoFont, except
		// with italics.
		LOGFONT lfFeatureDesc;
		lfFeatureDesc.lfHeight = 120;
		lfFeatureDesc.lfWidth = 0;//Don't care.
		lfFeatureDesc.lfEscapement = 0;
		lfFeatureDesc.lfOrientation = 0;
		lfFeatureDesc.lfWeight = FW_BOLD;
		lfFeatureDesc.lfItalic = TRUE;
		lfFeatureDesc.lfUnderline = FALSE;
		lfFeatureDesc.lfStrikeOut = FALSE;
		lfFeatureDesc.lfCharSet = DEFAULT_CHARSET;
		lfFeatureDesc.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lfFeatureDesc.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFeatureDesc.lfQuality = DEFAULT_QUALITY;
		lfFeatureDesc.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
		strcpy(lfFeatureDesc.lfFaceName, "Arial Bold");

		if(!CreateCompatiblePointFontIndirect(&m_FeatureDescFont, &lfFeatureDesc)) {
			return TRUE;
		}
		
		//TES 01/05/2009 - PLID 32145 - Set the fonts on our labels.
		GetDlgItem(IDC_FEATURE_DESC)->SetFont(&m_FeatureDescFont);
		GetDlgItem(IDC_MORE_INFO_LABEL)->SetFont(&m_MoreInfoFont);
		GetDlgItem(IDC_BLOCKED_CAPTION)->SetFont(&m_CaptionFont);

		//TES 12/23/2008 - PLID 32145 - Show the feature description (if we have one).
		if(!m_strFeatureDescription.IsEmpty()) {
			SetDlgItemText(IDC_FEATURE_DESC, "Feature: " + m_strFeatureDescription);
		}
		
		//TES 12/23/2008 - PLID 32145 - If we don't have a help manual page, hide the button to open the manual
		if(m_strManualBookmark.IsEmpty()) {
			m_nxbOpenManual.ShowWindow(SW_HIDE);
			//TES 12/23/2008 - PLID 32145 - Hide the regular Close button, and show the one that's centered in the dialog.
			m_nxbOK.ShowWindow(SW_HIDE);
			m_nxbCloseCentered.ShowWindow(SW_SHOWNA);
		}
		
	}
	NxCatchAll("Error in CEnterpriseSchedulerBlockedDlg::OnInitDialog()");

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}
void CEnterpriseSchedulerBlockedDlg::OnOpenManual()
{
	try {
		//TES 12/23/2008 - PLID 32145 - Open the manual to the page we were given.
		OpenManual("NexTech_Practice_Manual.chm", m_strManualBookmark);
	}NxCatchAll("Error in CEnterpriseSchedulerBlockedDlg::OnOpenManual()");
}

void CEnterpriseSchedulerBlockedDlg::OnCloseCentered()
{
	//TES 12/23/2008 - PLID 32145 - Just call OnOK().
	CNxDialog::OnOK();
}
