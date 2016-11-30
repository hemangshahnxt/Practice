// HCFAImageSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HCFAImageSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHCFAImageSetupDlg dialog


CHCFAImageSetupDlg::CHCFAImageSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHCFAImageSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHCFAImageSetupDlg)
	//}}AFX_DATA_INIT
}


void CHCFAImageSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHCFAImageSetupDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_USE_NPI_HCFA_IMAGE, m_radioUseNPIHCFAImage);
	DDX_Control(pDX, IDC_RADIO_USE_INTERMEDIATE_HCFA_IMAGE, m_radioUseIntermediateHCFAImage);
	DDX_Control(pDX, IDC_RADIO_USE_PRE_NPI_HCFA_IMAGE, m_radioUsePreNPIHCFAImage);
	DDX_Control(pDX, IDC_CHECK_SEND_BOX32_NPI, m_checkSendBox32NPI);
	DDX_Control(pDX, IDC_CHECK_SWAP_HCFA_IMAGE_DIAGS, m_checkSwapDiagPlacement);
	DDX_Control(pDX, IDC_CHECK_SHOW_FACILITY_CODE, m_checkShowFacilityCode);
	DDX_Control(pDX, IDC_CHECK_USE_DECIMALS, m_checkUseDecimals);
	DDX_Control(pDX, IDC_CHECK_SHOW_PAYER_ID, m_checkShowPayerID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHCFAImageSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHCFAImageSetupDlg)
	ON_BN_CLICKED(IDC_CHECK_SHOW_FACILITY_CODE, OnCheckShowFacilityCode)
	ON_BN_CLICKED(IDC_RADIO_USE_INTERMEDIATE_HCFA_IMAGE, OnRadioUseIntermediateHcfaImage)
	ON_BN_CLICKED(IDC_RADIO_USE_NPI_HCFA_IMAGE, OnRadioUseNpiHcfaImage)
	ON_BN_CLICKED(IDC_RADIO_USE_PRE_NPI_HCFA_IMAGE, OnRadioUsePreNpiHcfaImage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHCFAImageSetupDlg message handlers

BOOL CHCFAImageSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 09:29) - PLID 29953 - added nxiconbuttons for modernization
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnOK.AutoSet(NXB_OK);
	
	// (j.jones 2006-11-09 16:15) - PLID 22668 - added option to use the legacy HCFA Image
	// (j.jones 2007-05-14 11:25) - PLID 25953 - converted the image settings to support three versions of the image,
	// 0 - intermediate version, 1 - pre-npi version, 2 - official NPI version
	int iEnableLegacyHCFAImage = GetRemotePropertyInt("EnableLegacyHCFAImage",2,0,"<None>",true);
	if(iEnableLegacyHCFAImage == 0) {
		//intermediate (pre-npi with NPI appended)
		m_radioUseIntermediateHCFAImage.SetCheck(TRUE);
	}
	else if(iEnableLegacyHCFAImage == 1) {
		//legacy (pre-2007 HCFA, no NPI)
		m_radioUsePreNPIHCFAImage.SetCheck(TRUE);
	}
	else {
		//new HCFA (2007 HCFA, with NPI in normal locations)
		m_radioUseNPIHCFAImage.SetCheck(TRUE);

		// (j.jones 2008-05-06 09:40) - PLID 29128 - we're going to hide all the legacy controls
		// when loading the dialog with NPI image checker
		HideLegacyControls();
	}

	// (j.jones 2009-08-04 12:26) - PLID 14573 - renamed to be regular payer ID, retaining the old preference name
	m_checkShowPayerID.SetCheck(GetRemotePropertyInt("HCFAImageShowTHINNumber",0,0,"<None>",true) == 1);
	m_checkUseDecimals.SetCheck(GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);
	m_checkShowFacilityCode.SetCheck(GetRemotePropertyInt("HCFAImageShowFacilityCode",0,0,"<None>",true) == 1);
	// (j.jones 2006-11-10 11:50) - PLID 23414 - migrated diag code swap option from preferences
	m_checkSwapDiagPlacement.SetCheck(GetRemotePropertyInt("HCFAImageDiagCodeSetup",0,0,"<None>",true) == 1);
	// (j.jones 2007-05-04 11:02) - PLID 25908 - added option to send Box 32 NPI
	m_checkSendBox32NPI.SetCheck(GetRemotePropertyInt("HCFAImageSendBox32NPI",1,0,"<None>",true) == 1);

	// (j.jones 2008-05-06 09:41) - PLID 29128 - don't bother calling OnImageTypeChanged()
	// if we just loaded the NPI image, unless we are in debug mode

#ifdef _DEBUG

	OnImageTypeChanged();

#else

	if(iEnableLegacyHCFAImage != 2) {
		OnImageTypeChanged();
	}

#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHCFAImageSetupDlg::OnOK() 
{
	// (j.jones 2006-11-09 16:15) - PLID 22668 - added option to use the legacy HCFA Image
	// (j.jones 2007-05-14 11:25) - PLID 25953 - converted the image settings to support three versions of the image,
	// 0 - intermediate version, 1 - pre-npi version, 2 - official NPI version
	int iEnableLegacyHCFAImage = 2;
	if(m_radioUseIntermediateHCFAImage.GetCheck()) {
		//intermediate (pre-npi with NPI appended)
		iEnableLegacyHCFAImage = 0;
	}
	else if(m_radioUsePreNPIHCFAImage.GetCheck()) {
		//legacy (pre-2007 HCFA, no NPI)
		iEnableLegacyHCFAImage = 1;
	}
	else {
		//new HCFA (2007 HCFA, with NPI in normal locations)
		iEnableLegacyHCFAImage = 2;
	}
	SetRemotePropertyInt("EnableLegacyHCFAImage",iEnableLegacyHCFAImage,0,"<None>");

	// (j.jones 2009-08-04 12:26) - PLID 14573 - renamed to be regular payer ID, retaining the old preference name
	SetRemotePropertyInt("HCFAImageShowTHINNumber", m_checkShowPayerID.GetCheck() ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("HCFAImageUseDecimals", m_checkUseDecimals.GetCheck() ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("HCFAImageShowFacilityCode", m_checkShowFacilityCode.GetCheck() ? 1 : 0, 0, "<None>");
	// (j.jones 2006-11-10 11:50) - PLID 23414 - migrated diag code swap option from preferences
	SetRemotePropertyInt("HCFAImageDiagCodeSetup", m_checkSwapDiagPlacement.GetCheck() ? 1 : 0, 0, "<None>");
	// (j.jones 2007-05-04 11:02) - PLID 25908 - added option to send Box 32 NPI
	SetRemotePropertyInt("HCFAImageSendBox32NPI", m_checkSendBox32NPI.GetCheck() ? 1 : 0, 0, "<None>");
	
	CDialog::OnOK();
}

void CHCFAImageSetupDlg::OnCheckShowFacilityCode() 
{
	BOOL bWarn = FALSE;
	BOOL bEnable = FALSE;

	// (j.jones 2006-11-09 16:15) - PLID 22668 - if not using the legacy image, this option is irrelevant
	if(!IsDlgButtonChecked(IDC_CHECK_ENABLE_LEGACY_IMAGE))
		return;

	if(GetRemotePropertyInt("HCFAImageShowFacilityCode", 0, 0, "<None>", true) == 0
		&& IsDlgButtonChecked(IDC_CHECK_SHOW_FACILITY_CODE)) {

		//they are changing the checkbox to enable the feature
		bWarn = TRUE;
		bEnable = TRUE;
	}
	else if(GetRemotePropertyInt("HCFAImageShowFacilityCode", 0, 0, "<None>", true) == 1
		&& !IsDlgButtonChecked(IDC_CHECK_SHOW_FACILITY_CODE)) {

		//they are changing the checkbox to disable the feature
		bWarn = TRUE;
		bEnable = FALSE;
	}

	if(bWarn) {

		if(IDNO == MessageBox("You will need to send a test claim to be remapped by your\n"
							  "clearinghouse in order to support this change.\n\n"							  
							  "Are you sure you want to change the Facility Code export?",
							  "Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			CheckDlgButton(IDC_CHECK_SHOW_FACILITY_CODE, !bEnable);
			return;
		}
		else {
			AfxMessageBox("Remember, you MUST have your new claims remapped prior to submitting production claims.\n\n"
				"Please call your clearinghouse for assistance in remapping.");
		}
	}

}

// (j.jones 2007-05-14 10:53) - PLID 25953 - added options for all three image types
void CHCFAImageSetupDlg::OnRadioUseIntermediateHcfaImage() 
{
	OnImageTypeChanged();
}

void CHCFAImageSetupDlg::OnRadioUseNpiHcfaImage() 
{
	OnImageTypeChanged();	
}

void CHCFAImageSetupDlg::OnRadioUsePreNpiHcfaImage() 
{
	OnImageTypeChanged();
}

// (j.jones 2007-05-14 10:53) - PLID 25953 - added options for all three image types
void CHCFAImageSetupDlg::OnImageTypeChanged() {

	try {

		// (j.jones 2006-11-09 16:52) - PLID 22668 - I considered unchecking the facility code
		// option, but decided their legacy settings should be left alone
		m_checkShowFacilityCode.EnableWindow(m_radioUsePreNPIHCFAImage.GetCheck());

		// (j.jones 2006-11-10 11:52) - PLID 23414 - also disable the swap diag placement option
		m_checkSwapDiagPlacement.EnableWindow(m_radioUsePreNPIHCFAImage.GetCheck());

		// (j.jones 2007-05-04 11:01) - PLID 25908 - allow the Box 32 NPI option only on the Intermediate HCFA
		m_checkSendBox32NPI.EnableWindow(m_radioUseIntermediateHCFAImage.GetCheck());

	}NxCatchAll("Error in CHCFAImageSetupDlg::OnImageTypeChanged");
}

// (j.jones 2008-05-06 09:39) - PLID 29128 - this will hide all irrelevant controls
// when on the NPI form type, even the type selections themselves
void CHCFAImageSetupDlg::HideLegacyControls()
{
	try {

#ifdef _DEBUG

		//if in debug mode, don't hide all these controls		
		return;
#endif

		GetDlgItem(IDC_RADIO_USE_NPI_HCFA_IMAGE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_USE_INTERMEDIATE_HCFA_IMAGE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_USE_PRE_NPI_HCFA_IMAGE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_SEND_BOX32_NPI)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_SWAP_HCFA_IMAGE_DIAGS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_SHOW_FACILITY_CODE)->ShowWindow(SW_HIDE);

		//resize the dialog more attractively

		{
			//first move the controls
			CRect rcNPIImage;
			GetDlgItem(IDC_RADIO_USE_NPI_HCFA_IMAGE)->GetWindowRect(&rcNPIImage);
			ScreenToClient(&rcNPIImage);

			CRect rcDecimalOption;
			GetDlgItem(IDC_CHECK_USE_DECIMALS)->GetWindowRect(&rcDecimalOption);
			ScreenToClient(&rcDecimalOption);

			CRect rcPayerID;
			GetDlgItem(IDC_CHECK_SHOW_PAYER_ID)->GetWindowRect(&rcPayerID);
			ScreenToClient(&rcPayerID);

			long nDecimalHeight = rcDecimalOption.Height();
			rcDecimalOption.top = rcNPIImage.top;
			rcDecimalOption.bottom = rcDecimalOption.top + nDecimalHeight;
			GetDlgItem(IDC_CHECK_USE_DECIMALS)->MoveWindow(&rcDecimalOption);

			long nPayerIDHeight = rcPayerID.Height();
			rcPayerID.top = rcNPIImage.bottom + (nDecimalHeight / 2);
			rcPayerID.bottom = rcPayerID.top + nPayerIDHeight;
			GetDlgItem(IDC_CHECK_SHOW_PAYER_ID)->MoveWindow(&rcPayerID);

			//now move the NxColor, the OK/Cancel buttons, and the window
			CRect rcColor;
			GetDlgItem(IDC_COLOR)->GetWindowRect(&rcColor);
			ScreenToClient(&rcColor);

			CRect rcOK;
			GetDlgItem(IDOK)->GetWindowRect(&rcOK);
			ScreenToClient(&rcOK);

			CRect rcCancel;
			GetDlgItem(IDCANCEL)->GetWindowRect(&rcCancel);
			ScreenToClient(&rcCancel);

			CRect rcWindow;
			GetWindowRect(&rcWindow);
			ScreenToClient(&rcWindow);

			long nOKHeight = rcOK.bottom - rcOK.top;
			long nColorOKDiff = rcOK.top - rcColor.bottom;
			long nOKWindowDiff = rcWindow.bottom - rcOK.bottom;

			rcColor.bottom = rcPayerID.bottom + (rcDecimalOption.top - rcColor.top);
			GetDlgItem(IDC_COLOR)->MoveWindow(&rcColor);

			rcOK.top = rcColor.bottom + nColorOKDiff;
			rcOK.bottom = rcOK.top + nOKHeight;
			GetDlgItem(IDOK)->MoveWindow(&rcOK);

			rcCancel.top = rcColor.bottom + nColorOKDiff;
			rcCancel.bottom = rcCancel.top + nOKHeight;
			GetDlgItem(IDCANCEL)->MoveWindow(&rcCancel);
			
			rcWindow.bottom = rcCancel.bottom + nOKWindowDiff;

			//when resizing our window, ensure it remains centered
			CRect rcDesktop;
			SystemParametersInfo(SPI_GETWORKAREA, 0, rcDesktop, 0);
			long nScreenCenterX = rcDesktop.Width()/2;
			long nScreenCenterY = rcDesktop.Height()/2;
			MoveWindow(nScreenCenterX - (rcWindow.Width()/2), nScreenCenterY - (rcWindow.Height()/2), rcWindow.Width(), rcWindow.Height());
		}

	}NxCatchAll("Error in CHCFAImageSetupDlg::HideLegacyControls");
}
