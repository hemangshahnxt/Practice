// MirrorSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MirrorSettingsDlg.h"
#include "Mirror.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMirrorSettingsDlg dialog


CMirrorSettingsDlg::CMirrorSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMirrorSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMirrorSettingsDlg)
	m_bAllowImportOverwrite = FALSE;
	m_bAllowExportOverwrite = FALSE;
	m_bAssignProvider = FALSE;
	m_bAssignProviderExisting = FALSE;
	m_bLinkPatIDToMRN = FALSE;
	m_bLinkPatIDToSSN = FALSE;
	m_strImagePath = _T("");
	m_strDataPath = _T("");
	m_bAutoExport = FALSE;
	m_iImageDisplay = -1;
	m_bHiResImages = FALSE;
	m_bDisableLink = FALSE;
	m_bShowMirror = FALSE;
	m_bFullImageInPhotoViewerDlg = FALSE;
	//}}AFX_DATA_INIT
}


void CMirrorSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMirrorSettingsDlg)
	DDX_Control(pDX, IDC_NEW_PAT_CHECK, m_btnNewPat);
	DDX_Control(pDX, IDC_LINK_PATID_TO_SSN, m_btnLinkSSN);
	DDX_Control(pDX, IDC_LINK_PATID_TO_MRN, m_btnLinkMRN);
	DDX_Control(pDX, IDC_CHECK_ALLOW_IMPORT_OVERWRITE, m_btnImportOverwrite);
	DDX_Control(pDX, IDC_CHECK_ASSIGN_PROVIDER, m_btnAssignProv);
	DDX_Control(pDX, IDC_RADIO_NOIMAGES, m_btnNoImages);
	DDX_Control(pDX, IDC_RADIO_FULLCOLOR, m_btnFullColor);
	DDX_Control(pDX, IDC_RADIO_256COLOR, m_btn256Color);
	DDX_Control(pDX, IDC_RADIO_GRAYSCALE, m_btnGrayscale);
	DDX_Control(pDX, IDC_HIRES, m_btnHiRes);
	DDX_Control(pDX, IDC_CHECK_DISABLE, m_btnDisable);
	DDX_Control(pDX, IDC_CHECK_HIDEMIRROR, m_btnHideMirror);
	DDX_Control(pDX, IDC_CHECK_ASSIGN_PROVIDER_EXISTING, m_btnAssignProvExist);
	DDX_Control(pDX, IDC_CHECK_ALLOW_EXPORT_OVERWRITE, m_btnExportOverwrite);
	DDX_Control(pDX, IDC_CHECK_FORCE_MIRROR_60, m_btnForce60);
	DDX_Check(pDX, IDC_CHECK_ALLOW_IMPORT_OVERWRITE, m_bAllowImportOverwrite);
	DDX_Check(pDX, IDC_CHECK_ALLOW_EXPORT_OVERWRITE, m_bAllowExportOverwrite);
	DDX_Check(pDX, IDC_CHECK_ASSIGN_PROVIDER, m_bAssignProvider);
	DDX_Check(pDX, IDC_CHECK_ASSIGN_PROVIDER_EXISTING, m_bAssignProviderExisting);
	DDX_Check(pDX, IDC_LINK_PATID_TO_MRN, m_bLinkPatIDToMRN);
	DDX_Check(pDX, IDC_LINK_PATID_TO_SSN, m_bLinkPatIDToSSN);
	DDX_Text(pDX, IDC_IMAGE_OVERRIDE, m_strImagePath);
	DDX_Text(pDX, IDC_MIRROR_DATA_PATH, m_strDataPath);
	DDX_Check(pDX, IDC_NEW_PAT_CHECK, m_bAutoExport);
	DDX_Radio(pDX, IDC_RADIO_NOIMAGES, m_iImageDisplay);
	DDX_Check(pDX, IDC_HIRES, m_bHiResImages);
	DDX_Check(pDX, IDC_CHECK_DISABLE, m_bDisableLink);
	DDX_Check(pDX, IDC_CHECK_HIDEMIRROR, m_bShowMirror);
	DDX_Check(pDX, IDC_CHECK_FORCE_MIRROR_60, m_bForceMirror60);
	DDX_Control(pDX, IDC_MIRROR_DATA_PATH, m_nxeditMirrorDataPath);
	DDX_Control(pDX, IDC_IMAGE_OVERRIDE, m_nxeditImageOverride);
	DDX_Control(pDX, IDC_STATIC_MIRRORDATA, m_nxstaticMirrordata);
	DDX_Control(pDX, IDC_STATIC_MIRRORIMAGES, m_nxstaticMirrorimages);
	DDX_Control(pDX, IDC_IMAGE_GROUPBOX, m_btnImageGroupbox);
	DDX_Control(pDX, IDC_MIRROR60_GROUPBOX, m_btnMirror60Groupbox);
	DDX_Control(pDX, IDC_EXPORT_GROUPBOX, m_btnExportGroupbox);
	DDX_Control(pDX, IDC_VISIBILITY, m_btnVisibility);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Check(pDX, IDC_CHECK_FULL_HISTORY_IMAGE, m_bFullImageInPhotoViewerDlg);
	DDX_Control(pDX, IDC_CHECK_FULL_HISTORY_IMAGE, m_btnFullImageInPhotoViewerDlg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMirrorSettingsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMirrorSettingsDlg)
	ON_BN_CLICKED(IDC_CHECK_ASSIGN_PROVIDER, OnCheckAssignProvider)
	ON_BN_CLICKED(IDC_BTN_MIRRORDATA_BROWSE, OnBtnMirrordataBrowse)
	ON_BN_CLICKED(IDC_BTN_MIRRORIMAGE_BROWSE, OnBtnMirrorimageBrowse)
	ON_BN_CLICKED(IDC_CHECK_DISABLE, OnCheckDisable)
	ON_BN_CLICKED(IDC_CHECK_ALLOW_IMPORT_OVERWRITE, OnCheckAllowImportOverwrite)
	ON_BN_CLICKED(IDC_CHECK_FORCE_MIRROR_60, OnCheckForceMirror60)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorSettingsDlg message handlers

BOOL CMirrorSettingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_ProviderCombo = BindNxDataListCtrl(this, IDC_LIST_MIRROR_PROVIDERS, GetRemoteData(), true);

	// Load the current configuration into all the controls
	BeginWaitCursor();
	Load();
	EndWaitCursor();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMirrorSettingsDlg::Load()
{
	m_bAutoExport = GetPropertyInt("NewPatExportToMirror", 0);
	m_bLinkPatIDToSSN = Mirror::GetLinkMirrorSSNToUserDefinedID();
	m_bLinkPatIDToMRN = Mirror::GetLinkMirrorMRNToUserDefinedID();
	m_bAllowImportOverwrite = GetRemotePropertyInt("MirrorImportOverwrite", 0);
	m_bAllowExportOverwrite = GetRemotePropertyInt("MirrorExportOverwrite", 1);
	m_bAssignProvider = GetRemotePropertyInt("MirrorImportAssignProvider", 0);
	m_bAssignProviderExisting = GetRemotePropertyInt("MirrorImportAssignProviderExisting", 0);
	m_iImageDisplay = GetPropertyInt("MirrorImageDisplay", GetPropertyInt("MirrorShowImages", 1));
	m_bDisableLink = GetPropertyInt("MirrorDisable", 0);
	m_bShowMirror = GetPropertyInt("MirrorShowSoftware", 0);
	m_bForceMirror60 = (GetPropertyInt("MirrorAllow61Link", 1)) ? FALSE : TRUE;
	// (c.haag 2006-12-01 10:26) - PLID 23725 - Even when disabled, these should be loaded. Otherwise,
	// if we toggle Mirror 6.0 integration, these can be lost.
	m_strImagePath = Mirror::GetMirrorImagePath();
	m_strDataPath = Mirror::GetMirrorDataPath();
	m_bHiResImages = Mirror::GetHighRes();
	// (c.haag 2008-06-19 09:43) - PLID 28886 - Ability to show the full image from the modal photo viewer dialog
	m_bFullImageInPhotoViewerDlg = GetPropertyInt("MirrorUseFullImageInPhotoViewerDlg", 1);
	// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
	const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
	if (bUsingSDKFunctionality)
	{
		GetDlgItem(IDC_MIRROR_DATA_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_IMAGE_OVERRIDE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MIRRORDATA_BROWSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MIRRORIMAGE_BROWSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_MIRRORDATA)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_MIRRORIMAGES)->EnableWindow(FALSE);
		GetDlgItem(IDC_HIRES)->EnableWindow(FALSE);
		// (c.haag 2006-10-23 11:02) - PLID 23181 - This has been depreciated
		//m_bPrimaryImageOnly = Mirror::GetPrimaryImageOnly();
	}
	else
	{
		GetDlgItem(IDC_RADIO_256COLOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_GRAYSCALE)->EnableWindow(FALSE);
		// (c.haag 2006-10-23 11:02) - PLID 23181 - This has been depreciated
		//GetDlgItem(IDC_CHECK_MIRROR_PRIMARY_THUMB)->EnableWindow(FALSE);
		// (c.haag 2004-11-12 08:53) - Do not disable "hide mirror" in case
		// we come across a technical support issue that is caused by the
		// presence or absence of the Mirror application on the screen.
		// GetDlgItem(IDC_CHECK_HIDEMIRROR)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
	OnCheckAssignProvider();
}

void CMirrorSettingsDlg::Save()
{
	try {

		UpdateData(TRUE);

		SetPropertyInt("NewPatExportToMirror", m_bAutoExport);
		Mirror::SetLinkMirrorSSNToUserDefinedID(m_bLinkPatIDToSSN);
		Mirror::SetLinkMirrorMRNToUserDefinedID(m_bLinkPatIDToMRN);
		SetRemotePropertyInt("MirrorImportOverwrite", m_bAllowImportOverwrite);
		SetRemotePropertyInt("MirrorExportOverwrite", m_bAllowExportOverwrite);
		SetRemotePropertyInt("MirrorImportAssignProvider", m_bAssignProvider);
		SetRemotePropertyInt("MirrorImportAssignProviderExisting", m_bAssignProviderExisting);
		long nProviderID = 0;
		if(m_ProviderCombo->CurSel != -1)
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0),0);
		SetRemotePropertyInt("MirrorImportProvider", nProviderID);
		SetPropertyInt("MirrorShowSoftware", m_bShowMirror);
		SetPropertyInt("MirrorImageDisplay", m_iImageDisplay);
		SetPropertyInt("MirrorDisable", m_bDisableLink);
		SetPropertyInt("MirrorAllow61Link", (m_bForceMirror60) ? 0 : 1);

		// (c.haag 2006-12-01 10:26) - PLID 23725 - Even when using 6.1, these should be loaded. Otherwise,
		// if we toggle Mirror 6.0 integration, they can be lost.
		Mirror::SetMirrorImagePath(m_strImagePath);
		Mirror::SetMirrorDataPath(m_strDataPath);
		Mirror::SetHighRes(m_bHiResImages);
		// (c.haag 2008-06-19 09:43) - PLID 28886 - Ability to show the full image from the modal photo viewer dialog
		SetPropertyInt("MirrorUseFullImageInPhotoViewerDlg", (m_bFullImageInPhotoViewerDlg) ? 1 : 0);
		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();

		if (bUsingSDKFunctionality)
		{
			// (c.haag 2006-10-23 11:02) - PLID 23181 - This has been depreciated
			//Mirror::SetPrimaryImageOnly(m_bPrimaryImageOnly);
			Mirror::ShowApplication(m_bShowMirror);
		}

	}NxCatchAll("Error saving mirror settings.");
}

void CMirrorSettingsDlg::OnOK() 
{
	BeginWaitCursor();
	Save();	
	EndWaitCursor();
	CDialog::OnOK();
}

void CMirrorSettingsDlg::OnCheckAssignProvider() 
{
	UpdateData(TRUE);

	if (m_bAssignProvider)
	{
		GetDlgItem(IDC_LIST_MIRROR_PROVIDERS)->EnableWindow(TRUE);
		if (-1 == m_ProviderCombo->FindByColumn(0, (long)GetRemotePropertyInt("MirrorImportProvider", 0), 0, TRUE))
		{
			if (m_ProviderCombo->GetRowCount() > 0)
				m_ProviderCombo->CurSel = 0;
		}
	}
	else
	{
		GetDlgItem(IDC_LIST_MIRROR_PROVIDERS)->EnableWindow(FALSE);
	}

	if (m_bAssignProvider && m_bAllowImportOverwrite)
	{
		GetDlgItem(IDC_CHECK_ASSIGN_PROVIDER_EXISTING)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_ASSIGN_PROVIDER_EXISTING)->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_ASSIGN_PROVIDER_EXISTING))->SetCheck(0);
	}
}

void CMirrorSettingsDlg::OnBtnMirrordataBrowse() 
{
	CString strInitPath,
			strInOutPath;

	CFileDialog dlgBrowse(TRUE, "mdb", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Mirror Database Files|*.mde;*.mdb|All Files|*.*|");

	strInitPath = Mirror::GetMirrorDataPath(); // We need to store this because the next line is a pointer to it
	if (strInitPath != "")
		GetFilePath(strInitPath);
	else strInitPath = "c:\\";

	dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
	if (dlgBrowse.DoModal() == IDOK) 
	{
		// If the user clicked okay, that means she selected a file so remember it
		m_strDataPath = dlgBrowse.GetPathName();
		UpdateData(FALSE);
	} 
}

void CMirrorSettingsDlg::OnBtnMirrorimageBrowse() 
{
	CString strStartPath = m_strImagePath;
	BrowseToFolder(&m_strImagePath, "Mirror Picture Folder", GetSafeHwnd(), NULL, strStartPath);
	UpdateData(FALSE);
}

void CMirrorSettingsDlg::OnCheckDisable() 
{
	UpdateData(TRUE);
	if (m_bDisableLink)
	{
		if (IDNO == MsgBox(MB_YESNO, "Are you ABSOLUTELY SURE you wish to disable the link with Mirror on this computer? This will apply to any user who logs into Practice from this computer."))
		{
			((CButton*)GetDlgItem(IDC_CHECK_DISABLE))->SetCheck(0);
			return;
		}
	}
}

void CMirrorSettingsDlg::OnCheckAllowImportOverwrite() 
{
	OnCheckAssignProvider();
}


void CMirrorSettingsDlg::OnCheckForceMirror60()
{
	//
	// (c.haag 2006-12-07 09:06) - PLID 23725 - If the user checks the "Force Mirror 6.0" checkbox, 
	// we have to warn them that *everyone* needs to restart Practice for the change to take effect.
	// The program-wide implementation was not designed to consider the possibility that the way we 
	// link with Mirror changes on the fly.
	//
	MsgBox("You are changing the way that NexTech Practice integrates with Canfield Mirror. Please have all users restart NexTech Practice to continue properly using the Mirror link after clicking OK to dismiss this window.");
}
