// ImportWizardTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImportWizardDlg.h"
#include "ImportWizardTypeDlg.h"
#include "Globalutils.h"
// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum FileTypeValues{
	ftvCSV=0,
	ftvTab,
	ftvOther,
};

/////////////////////////////////////////////////////////////////////////////
// CImportWizardTypeDlg dialog

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
IMPLEMENT_DYNCREATE(CImportWizardTypeDlg, CNxPropertyPage)

CImportWizardTypeDlg::CImportWizardTypeDlg() : CNxPropertyPage(CImportWizardTypeDlg::IDD)
{
	Construct(IDD_IMPORT_WIZARD_TYPE);
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = "Import File Wizard";

	//{{AFX_DATA_INIT(CImportWizardTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	m_bNeedInit = TRUE;
}


void CImportWizardTypeDlg::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialog::OnFinalRelease();
}

void CImportWizardTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportWizardTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_IMPORT_FILE_NAME, m_nxeditImportFileName);
	DDX_Control(pDX, IDC_IMPORT_FIELD_DELIMITER, m_nxeditImportFieldDelimiter);
	DDX_Control(pDX, IDC_IMPORT_TEXT_QUALIFIER, m_nxeditImportTextQualifier);
	DDX_Control(pDX, IDC_IMPORT_DELIMITER_LABEL, m_nxstaticImportDelimiterLabel);
	DDX_Control(pDX, IDC_IMPORT_TYPE_GROUPBOX, m_btnImportTypeGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportWizardTypeDlg, CNxPropertyPage)
	//{{AFX_MSG_MAP(CImportWizardTypeDlg)
	ON_BN_CLICKED(IDC_IMPORT_PATIENT_FILE, OnImportPatientFile)
	ON_BN_CLICKED(IDC_IMPORT_PROVIDER_FILE, OnImportProviderFile)
	ON_BN_CLICKED(IDC_IMPORT_REF_PHYS_FILE, OnImportRefPhysFile)
	ON_BN_CLICKED(IDC_IMPORT_USER_FILE, OnImportUserFile)
	ON_BN_CLICKED(IDC_IMPORT_SUPPLIER_FILE, OnImportSupplierFile)
	ON_BN_CLICKED(IDC_IMPORT_OTHER_CONTACT_FILE, OnImportOtherContactFile)
	ON_BN_CLICKED(IDC_IMPORT_MEDINOTES_FILE, OnImportMedinotesFile)
	ON_BN_CLICKED(IDC_IMPORT_SERVICE_CODE_FILE, OnImportServiceCodeFile)
	ON_BN_CLICKED(IDC_IMPORT_RESOURCE_FILE, OnImportResourceCodeFile)
	ON_BN_CLICKED(IDC_IMPORT_PRODUCT_FILE, OnImportProductCodeFile)
	ON_BN_CLICKED(IDC_IMPORT_INSCO_FILE, OnImportInsCoCodeFile)
	ON_EN_CHANGE(IDC_IMPORT_FILE_NAME, OnChangeImportFileName)
	ON_BN_CLICKED(IDC_IMPORT_FILE_BROWSE, OnImportFileBrowse)
	ON_BN_CLICKED(IDC_IMPORT_USE_TEXT_QUALIFIER, OnImportUseTextQualifier)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_IMPORT_PATIENT_NOTE_FILE, &CImportWizardTypeDlg::OnImportmportPatientNoteFile)
	ON_BN_CLICKED(IDC_IMPORT_RECALL_FILE, &CImportWizardTypeDlg::OnBnClickedImportRecallFile)
	ON_BN_CLICKED(IDC_IMPORT_APPTS_FILE, &CImportWizardTypeDlg::OnBnClickedImportApptsFile)
	ON_BN_CLICKED(IDC_IMPORT_INSURED_PARTIES, &CImportWizardTypeDlg::OnBnClickedImportInsuredParties)
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_IMPORT_RACES_FILE, &CImportWizardTypeDlg::OnBnClickedImportRacesFile)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CImportWizardTypeDlg, CNxPropertyPage)
	//{{AFX_DISPATCH_MAP(CImportWizardTypeDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IImportWizardTypeDlg to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {31DD5896-CBAC-4FB4-86D6-0111F33AAEF6}
static const IID IID_IImportWizardTypeDlg =
{ 0x31dd5896, 0xcbac, 0x4fb4, { 0x86, 0xd6, 0x1, 0x11, 0xf3, 0x3a, 0xae, 0xf6 } };

BEGIN_INTERFACE_MAP(CImportWizardTypeDlg, CNxPropertyPage)
	INTERFACE_PART(CImportWizardTypeDlg, IID_IImportWizardTypeDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportWizardTypeDlg message handlers

BOOL CImportWizardTypeDlg::OnInitDialog() 
{
	try{
		CNxPropertyPage::OnInitDialog();

	   //(s.dhole 4/8/2015 10:26 AM ) - PLID 65229  Proirity preference
	   // (j.politis 2015-04-06 14:17) - PLID 65232 - Limit all note imports to a maximum of 100k records.
		//(s.dhole 4/29/2015 2:18 PM ) - PLID 65712 added AllowedParagraphPerColumnInImport
		g_propManager.CachePropertiesInBulk("CImportWizardTypeDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'MyDefaultNotesPriority'  OR "
			"Name = 'MaxNumOfNoteRecordsToImport' OR "
			"Name = 'AllowedParagraphPerColumnInImport' "
			")",
			_Q(GetCurrentUserName()));

		m_pFileType = BindNxDataList2Ctrl(this, IDC_IMPORT_FILE_TYPE, NULL, false);

		//(e.lally 2007-06-18) PLID 26364 - (Labeling the parsing part of the import wizard, split from 5288)
		//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
		// (b.savon 2015-07-06 11:16) - PLID 66491 - The text qualifier checkbox on the import should default to be checked
		CheckDlgButton(IDC_IMPORT_USE_TEXT_QUALIFIER, BST_CHECKED);
		OnImportUseTextQualifier();

		IRowSettingsPtr pRow = m_pFileType->GetNewRow();
		pRow->PutValue(0, (long)ftvCSV);
		pRow->PutValue(1, "Comma Separated Values (.csv)");
		m_pFileType->AddRowAtEnd(pRow, NULL);

		pRow = m_pFileType->GetNewRow();
		pRow->PutValue(0, (long)ftvTab);
		pRow->PutValue(1, "Tab Delimited (.txt)");
		m_pFileType->AddRowAtEnd(pRow, NULL);

		pRow = m_pFileType->GetNewRow();
		pRow->PutValue(0, (long)ftvOther);
		pRow->PutValue(1, "Other");
		m_pFileType->AddRowAtEnd(pRow, NULL);

		// (j.jones 2010-10-19 08:43) - PLID 34571 - disable options that they do
		// not have permissions to create entries for

		/*
		IF ANY CHANGES ARE MADE TO THE FOLLOWING PERMISSIONS CHECKS, THEN PLEASE REVIEW CMainFrame::ImportShouldBeEnabled()
		IF NECESSARY, MAKE SURE YOUR CHANGES ARE APPLIED THERE AS WELL.
		*/

		//the Medinotes import is also a patient import
		if(!(GetCurrentUserPermissions(bioPatient) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_PATIENT_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_MEDINOTES_FILE)->EnableWindow(FALSE);
		}
		//all contact creation is controlled by only one permission
		if(!(GetCurrentUserPermissions(bioContact) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_PROVIDER_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_REF_PHYS_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_USER_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_SUPPLIER_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_OTHER_CONTACT_FILE)->EnableWindow(FALSE);
		}
		if(!(GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_SERVICE_CODE_FILE)->EnableWindow(FALSE);
		}
		// (r.farnworth 2015-03-30 12:26) - PLID 65197 - Add a new import type, Resources, to the import utility
		if (!(GetCurrentUserPermissions(bioAdminScheduler) & SPT___W________ANDPASS)) {
			GetDlgItem(IDC_IMPORT_RESOURCE_FILE)->EnableWindow(FALSE);
		}
		// (r.farnworth 2015-03-30 12:28) - PLID 65238 - Add a new import type, Products, to the't import utility
		if (!(GetCurrentUserPermissions(bioInvItem) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_PRODUCT_FILE)->EnableWindow(FALSE);
		}

		// (b.savon 2015-03-30 15:48) - PLID 65231 - Add a new import type, Recalls, to the import utility
		if (!(GetCurrentUserPermissions(bioRecallSystem) & SPT____C_______ANDPASS)){
			GetDlgItem(IDC_IMPORT_RECALL_FILE)->EnableWindow(FALSE);
		}

		// (r.farnworth 2015-04-01 12:45) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
		if (!(GetCurrentUserPermissions(bioInsuranceCo) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_INSCO_FILE)->EnableWindow(FALSE);
		}

		// (b.savon 2015-04-06 08:59) - PLID 65215 - Add a new import type, Appointments, to the import utility
		if (!(GetCurrentUserPermissions(bioAppointment) & SPT____C_______ANDPASS)){
			GetDlgItem(IDC_IMPORT_APPTS_FILE)->EnableWindow(FALSE);
		}
		//(s.dhole 4/8/2015 1:50 PM ) - PLID 65224 Add new patient note utility
		if (!(GetCurrentUserPermissions(bioPatientNotes) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_PATIENT_NOTE_FILE)->EnableWindow(FALSE);
		}
		//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
		if (!(GetCurrentUserPermissions(bioPatientInsurance) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_IMPORT_INSURED_PARTIES)->EnableWindow(FALSE);
		}

		// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
		//Get our current size
		{
			CRect rc;
			GetClientRect(&rc);
			//Remember our current size
			m_ClientSize = rc.Size();
		}
		//Remember that we no longer need to be initialized
		m_bNeedInit = FALSE;

		return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	}NxCatchAll("Error in CImportWizardTypeDlg::OnInitDialog");
	return FALSE;
}

BOOL CImportWizardTypeDlg::OnSetActive()
{

	// (j.politis 2015-04-30 10:49) - PLID 65524 - Allow the import wizard to be resizable
	//When we become active, tell our parent
	CImportWizardDlg *pSheet = dynamic_cast<CImportWizardDlg *>(GetParentSheet());
	if (pSheet != NULL) {
		pSheet->OnPageSetActive(this);
	}

	//Fill things in from our parent.

	// (j.jones 2010-10-19 08:43) - PLID 34571 - don't set a value if they don't have permission,
	// also don't set a value by default if they need to enter a password

	//the Medinotes import is also a patient import
	if((((CImportWizardDlg*)GetParent())->m_irtRecordType == irtPatients
		|| ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtMediNotes)
		&& (!(GetCurrentUserPermissions(bioPatient) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioPatient) & sptCreateWithPass))) {
		
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	//all contact creation is controlled by only one permission
	if((((CImportWizardDlg*)GetParent())->m_irtRecordType == irtProviders
		|| ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtReferringPhysicians 
		|| ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtUsers
		|| ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtSuppliers 
		|| ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtOtherContacts)
		&& (!(GetCurrentUserPermissions(bioContact) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioContact) & sptCreateWithPass))) {
		
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	if(((CImportWizardDlg*)GetParent())->m_irtRecordType == irtServiceCodes
		&& (!(GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS)
			|| (GetCurrentUserPermissions(bioServiceCodes) & sptCreateWithPass))) {

		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	// (r.farnworth 2015-03-16 16:02) - PLID 65197 - Add a new import type, Resources, to the import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtResources
		&& (!(GetCurrentUserPermissions(bioAdminScheduler) & SPT___W________ANDPASS)
		|| (GetCurrentUserPermissions(bioAdminScheduler) & sptWriteWithPass))) {

		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	// (r.farnworth 2015-03-19 11:17) - PLID 65238 - Add a new import type, Products, to the't import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtProducts
		&& (!(GetCurrentUserPermissions(bioInvItem) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioInvItem) & sptCreateWithPass))) {

		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	//(s.dhole 3/30/2015 11:19 AM ) - PLID 65224
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtPatientNotes
		&& (!(GetCurrentUserPermissions(bioPatientNotes) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioPatientNotes) & sptCreateWithPass))) {
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}

	// (b.savon 2015-03-30 15:48) - PLID 65231 - Add a new import type, Recalls, to the import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtRecalls
		&& (!(GetCurrentUserPermissions(bioRecallSystem) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioRecallSystem) & sptCreateWithPass))) {
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}

	// (r.farnworth 2015-04-01 17:15) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtInsuranceCos
		&& (!(GetCurrentUserPermissions(bioInsuranceCo) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioInsuranceCo) & sptCreateWithPass))) {
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}

	// (b.savon 2015-04-06 09:01) - PLID 65215 - Add a new import type, Appointments, to the import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtAppointments
		&& (!(GetCurrentUserPermissions(bioAppointment) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioAppointment) & sptCreateWithPass))){
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190  - Add a new import type, Insure Party, to the import utility
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtInsuredParties 
		&& (!(GetCurrentUserPermissions(bioPatientInsurance) & SPT____C_______ANDPASS)
		|| (GetCurrentUserPermissions(bioPatientInsurance) & sptCreateWithPass))) {
		UpdateWizardButtons();
		return CNxPropertyPage::OnSetActive();
	}
	UINT idcToCheck;
	switch(((CImportWizardDlg*)GetParent())->m_irtRecordType) {
	default:
	case irtPatients:
		idcToCheck = IDC_IMPORT_PATIENT_FILE;
		break;
	case irtProviders:
		idcToCheck = IDC_IMPORT_PROVIDER_FILE;
		break;
	case irtReferringPhysicians:
		idcToCheck = IDC_IMPORT_REF_PHYS_FILE;
		break;
	case irtUsers:
		idcToCheck = IDC_IMPORT_USER_FILE;
		break;
	case irtSuppliers:
		idcToCheck = IDC_IMPORT_SUPPLIER_FILE;
		break;
	case irtOtherContacts:
		idcToCheck = IDC_IMPORT_OTHER_CONTACT_FILE;
		break;
	case irtMediNotes:
		idcToCheck = IDC_IMPORT_MEDINOTES_FILE;
		break;	
	case irtServiceCodes:
		// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
		idcToCheck = IDC_IMPORT_SERVICE_CODE_FILE;
		break;
	case irtResources:
		// (r.farnworth 2015-03-16 16:06) - PLID 65197 - Add a new import type, Resources, to the import utility
		idcToCheck = IDC_IMPORT_RESOURCE_FILE;
		break;
	case irtProducts:
		idcToCheck = IDC_IMPORT_PRODUCT_FILE;
		break;
	case irtPatientNotes:
		//(s.dhole 3/30/2015 10:49 AM ) - PLID 65224  Patient note import
		idcToCheck = IDC_IMPORT_PATIENT_NOTE_FILE;	
		break;
	case irtRecalls: // (b.savon 2015-03-30 16:11) - PLID 65231 - Add a new import type, Recalls, to the import utility
		idcToCheck = IDC_IMPORT_RECALL_FILE;
		break;
	case irtInsuranceCos: // (r.farnworth 2015-04-01 12:49) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
		idcToCheck = IDC_IMPORT_INSCO_FILE;
		break;
	case irtAppointments: // (b.savon 2015-04-06 09:01) - PLID 65215 - Add a new import type, Appointments, to the import utility
		idcToCheck = IDC_IMPORT_APPTS_FILE;
		break;
	case irtInsuredParties: //(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
		idcToCheck = IDC_IMPORT_INSURED_PARTIES;
		break;
	case irtRaces: // (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add new import type, Races, to the import utility
		idcToCheck = IDC_IMPORT_RACES_FILE;
		break;
	}

	CheckRadioButton(IDC_IMPORT_PATIENT_FILE, IDC_IMPORT_RACES_FILE, idcToCheck);
	HandleRadioButton();

	UpdateWizardButtons();

	return CNxPropertyPage::OnSetActive();
}


//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
BOOL CImportWizardTypeDlg::IsFileValid(const CString &strFileName)
{
	CFile fImportFile;
	fImportFile.SetFilePath(strFileName);

	if (!fImportFile.Open(strFileName, CFile::modeRead | CFile::shareCompat)) {

		MessageBox("Could not read the specified filename. Please enter a valid filename for this import.");

		return FALSE;
	}

	if (DoesRecordTypeHaveFileLengthLimit()){
		return IsFileLengthValid(fImportFile);
	}

	return TRUE;
}

//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
BOOL CImportWizardTypeDlg::DoesRecordTypeHaveFileLengthLimit()
{
	switch (((CImportWizardDlg*)GetParent())->m_irtRecordType){
		case irtPatientNotes:
		{
			return TRUE;
		}
		default:
		{
			return FALSE;
		}
	}
}

//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
long CImportWizardTypeDlg::GetFileLengthLimitForRecordType()
{
	switch (((CImportWizardDlg*)GetParent())->m_irtRecordType){
		case irtPatientNotes:
		{
			return GetRemotePropertyInt("MaxNumOfNoteRecordsToImport", 100000, 0, "<None>", true);
		}
		default:
		{
			//If you get here, you must handle the record type you added in DoesRecordTypeHaveFileLengthLimit()
			ASSERT(FALSE);
			return -1;
		}
	}
}

//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
CString CImportWizardTypeDlg::GetFileLengthLimitWarningMessage(long nMaxLimit)
{
	CString strNMax = FormatNumberForInterface(nMaxLimit, FALSE, TRUE, 0);
	return FormatString("There are too many records in your file, the maximum allowed is %s.\r\n\r\n"
		"Please split your file into multiple files with %s records or less and try again.", strNMax, strNMax);
}

//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
BOOL CImportWizardTypeDlg::IsFileLengthValid(CFile &fImportFile)
{
	CArchive ar(&fImportFile, CArchive::load);	//easier to read whole lines with archives
	
	//Construct the error message
	long nMaxLimit = GetFileLengthLimitForRecordType();
	
	//Count how many lines we read in, stop just 1 after our max
	CString strIn;
	int nCount = 0;
	while (ar.ReadString(strIn)){
		//If we read in more than our max, display an error message and return
		if (++nCount > nMaxLimit){
			CString strMessage = GetFileLengthLimitWarningMessage(nMaxLimit);
			MessageBox(strMessage, "Nextech Practice", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CImportWizardTypeDlg::OnKillActive()
{
	try {
		//(e.lally 2007-06-18) PLID 26364 - (Labeling the parsing part of the import wizard, split from 5288)
		//They have a filename, check if it is valid
		CString strFileName;
		GetDlgItemText(IDC_IMPORT_FILE_NAME, strFileName);
		if(strFileName.IsEmpty()){
			MessageBox("You must enter a filename for this import");
			return FALSE;
		}

		//(j.politis 2015-04-02 13:30) PLID 65232 - Bounce note imports with over 100k records
		if (!IsFileValid(strFileName)) {
			return FALSE;
		}

		//All is well, set the filename
		((CImportWizardDlg*)GetParent())->m_strFileName = strFileName;

		//Make sure a file format was selected
		IRowSettingsPtr pRow = m_pFileType->GetCurSel();
		if(pRow == NULL){
			MessageBox("A File Format must be specified before continuing.");
			return FALSE;
		}

		//Check what type of file it is and see if we need to set the delimiter
		if(VarLong(pRow->GetValue(0)) == (long)ftvOther){
			CString strDelimiter;
			GetDlgItemText(IDC_IMPORT_FIELD_DELIMITER, strDelimiter);
			((CImportWizardDlg*)GetParent())->m_strFieldSeparator = strDelimiter;
			//we should force a delimiter to be specified
			if(strDelimiter.IsEmpty()){
				MessageBox("The Field Delimiter must be specified with the 'Other' file format.");
				return FALSE;
			}
		}

		//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
		//check if we need the field qualifier
		CString strQualifier = ""; //empty/none
		if(IsDlgButtonChecked(IDC_IMPORT_USE_TEXT_QUALIFIER)){
			GetDlgItemText(IDC_IMPORT_TEXT_QUALIFIER, strQualifier);
			//we should force a text qualifier to be specified
			if(strQualifier.IsEmpty()){
				MessageBox("The Text Qualifier may not be blank if it is checked to be in use.");
				return FALSE;
			}
		}
		((CImportWizardDlg*)GetParent())->m_strTextQualifier = strQualifier;


		return CNxPropertyPage::OnKillActive();
	}NxCatchAll("Error in CImportWizardTypeDlg::OnKillActive");
	return FALSE;
}

void CImportWizardTypeDlg::UpdateWizardButtons()
{
	
	//They can't go Back (we're the first page), and they can only go Next if 
	// -they have filled in a file name
	// -they have specified the type of import being performed
	//(s.dhole 3/30/2015 10:51 AM ) - PLID 65224 Added IDC_IMPORT_PATIENT_NOTE_FILE
	// (b.savon 2015-03-30 16:18) - PLID 65231 - Add a new import type, Recalls, to the import utility
	// (b.savon 2015-04-06 09:01) - PLID 65215 - Add a new import type, Appointments, to the import utility
	//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
	// (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add new import type, Races, to the import utility
	CString strFileName;
	GetDlgItemText(IDC_IMPORT_FILE_NAME, strFileName);
	if(strFileName.IsEmpty()) {
		((CImportWizardDlg*)GetParent())->SetWizardButtons(0);
	}
	else if(!IsDlgButtonChecked(IDC_IMPORT_PATIENT_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_PROVIDER_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_REF_PHYS_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_USER_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_SUPPLIER_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_OTHER_CONTACT_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_MEDINOTES_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_SERVICE_CODE_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_RESOURCE_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_PRODUCT_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_PATIENT_NOTE_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_RECALL_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_INSCO_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_APPTS_FILE) &&
			!IsDlgButtonChecked(IDC_IMPORT_INSURED_PARTIES) &&
			!IsDlgButtonChecked(IDC_IMPORT_RACES_FILE)
			)
	{
		((CImportWizardDlg*)GetParent())->SetWizardButtons(0);
	}
	else {
		//they have a type of import selected
		//enable the next button
		((CImportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_NEXT);
	}
}



void CImportWizardTypeDlg::OnImportPatientFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Patient File");
}

void CImportWizardTypeDlg::OnImportProviderFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Provider File");
}

void CImportWizardTypeDlg::OnImportRefPhysFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Referring Physician File");
}

void CImportWizardTypeDlg::OnImportUserFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: User File");
}

void CImportWizardTypeDlg::OnImportSupplierFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Supplier File");
}

void CImportWizardTypeDlg::OnImportOtherContactFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Other Contact File");
}

void CImportWizardTypeDlg::OnImportMedinotesFile() 
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: MediNotes File");
}

// (j.jones 2010-04-05 10:07) - PLID 16717 - supported service codes
void CImportWizardTypeDlg::OnImportServiceCodeFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Service Code File");
}

// (r.farnworth 2015-03-16 16:51) - PLID 65197 - Add a new import type, Resources, to the import utility
void CImportWizardTypeDlg::OnImportResourceCodeFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Resource Code File");
}

// (r.farnworth 2015-03-19 11:30) - PLID 65238 - Add a new import type, Products, to the't import utility
void CImportWizardTypeDlg::OnImportProductCodeFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Product Code File");
}

// (r.farnworth 2015-04-01 14:41) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
void CImportWizardTypeDlg::OnImportInsCoCodeFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Insurance Companies Code File");
}

//(s.dhole 3/30/2015 10:54 AM ) - PLID 65224
void CImportWizardTypeDlg::OnImportmportPatientNoteFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Patient Note File");
}

// (b.savon 2015-03-30 16:01) - PLID 65231 - Add a new import type, Recalls, to the import utility
void CImportWizardTypeDlg::OnBnClickedImportRecallFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Recall File");
}

// (b.savon 2015-04-06 09:03) - PLID 65215 - Add a new import type, Appointments, to the import utility
void CImportWizardTypeDlg::OnBnClickedImportApptsFile()
{
	try{
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Appointment File");
}

// (r.goldschmidt 2016-02-09 14:50) - PLID 68163 - added races import
void CImportWizardTypeDlg::OnBnClickedImportRacesFile()
{
	try {
		HandleRadioButton();
	}NxCatchAll("Error selecting import file type: Races File");
}

void CImportWizardTypeDlg::HandleRadioButton()
{
	bool bChanged = false;
	if(IsDlgButtonChecked(IDC_IMPORT_PATIENT_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioPatient, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_PATIENT_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtPatients);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtPatients;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_PROVIDER_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioContact, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_PROVIDER_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtProviders);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtProviders;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_REF_PHYS_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioContact, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_REF_PHYS_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtReferringPhysicians);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtReferringPhysicians;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_USER_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioContact, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_USER_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtUsers);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtUsers;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_SUPPLIER_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioContact, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_SUPPLIER_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtSuppliers);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtSuppliers;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_OTHER_CONTACT_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioContact, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_OTHER_CONTACT_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtOtherContacts);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtOtherContacts;
	}
	else if(IsDlgButtonChecked(IDC_IMPORT_MEDINOTES_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioPatient, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_MEDINOTES_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtMediNotes);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtMediNotes;
	}
	// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
	else if(IsDlgButtonChecked(IDC_IMPORT_SERVICE_CODE_FILE)) {

		// (j.jones 2010-10-19 08:43) - PLID 34571 - check create permission
		if(!CheckCurrentUserPermissions(bioServiceCodes, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_SERVICE_CODE_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtServiceCodes);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtServiceCodes;
	}
	// (r.farnworth 2015-03-17 09:00) - PLID 65197 - Add a new import type, Resources, to the import utility
	else if (IsDlgButtonChecked(IDC_IMPORT_RESOURCE_FILE)) {
		if (!CheckCurrentUserPermissions(bioAdminScheduler, sptWrite)) {
			CheckDlgButton(IDC_IMPORT_RESOURCE_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtResources);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtResources;
	}
	// (r.farnworth 2015-03-19 11:30) - PLID 65238 - Add a new import type, Products, to the't import utility
	else if (IsDlgButtonChecked(IDC_IMPORT_PRODUCT_FILE)) {
		if (!CheckCurrentUserPermissions(bioInvItem, sptCreate)) {
		CheckDlgButton(IDC_IMPORT_PRODUCT_FILE, FALSE);
		UpdateWizardButtons();
		return;
	}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtProducts);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtProducts;
	}
	// (r.farnworth 2015-04-01 14:43) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
	else if (IsDlgButtonChecked(IDC_IMPORT_INSCO_FILE)) {
		if (!CheckCurrentUserPermissions(bioInsuranceCo, sptCreate)) {
			CheckDlgButton(IDC_IMPORT_INSCO_FILE, FALSE);
			UpdateWizardButtons();
			return;
		}

		bChanged = (((CImportWizardDlg*)GetParent())->m_irtRecordType != irtInsuranceCos);
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtInsuranceCos;
	}
	//(s.dhole 3/30/2015 11:13 AM ) - PLID 65224  patient note import
	else if (IsDlgButtonChecked(IDC_IMPORT_PATIENT_NOTE_FILE) && HandleRadioButtonEx(IDC_IMPORT_PATIENT_NOTE_FILE, irtPatientNotes, bioPatientNotes, sptCreate) == FALSE) 
	{
		return;
	}
	else if (IsDlgButtonChecked(IDC_IMPORT_RECALL_FILE) && HandleRadioButtonEx(IDC_IMPORT_RECALL_FILE, irtRecalls, bioRecallSystem, sptCreate) == FALSE) {
		// (b.savon 2015-03-30 16:18) - PLID 65231 - Add a new import type, Recalls, to the import utility
		return;
	}
	else if (IsDlgButtonChecked(IDC_IMPORT_APPTS_FILE) && HandleRadioButtonEx(IDC_IMPORT_APPTS_FILE, irtAppointments, bioAppointment, sptCreate) == FALSE){
		// (b.savon 2015-04-06 09:03) - PLID 65215 - Add a new import type, Appointments, to the import utility
		return;
	}
	
	else if (IsDlgButtonChecked(IDC_IMPORT_INSURED_PARTIES) && HandleRadioButtonEx(IDC_IMPORT_INSURED_PARTIES, irtInsuredParties , bioPatientInsurance, sptCreate) == FALSE){
		//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
		return;
		
	}
	else if (IsDlgButtonChecked(IDC_IMPORT_RACES_FILE)) {
		// (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add new import type, Races, to the import utility
		((CImportWizardDlg*)GetParent())->m_irtRecordType = irtRaces;
	}

	UpdateWizardButtons();

}

//(s.dhole 3/30/2015 11:10 AM ) - PLID 65224
BOOL CImportWizardTypeDlg::HandleRadioButtonEx(UINT RedioButtonId, ImportRecordType RecordType, EBuiltInObjectIDs oBuiltInObjectID, ESecurityPermissionType SecurityPermissionType)
{
	if (!CheckCurrentUserPermissions(oBuiltInObjectID, SecurityPermissionType)) {
		CheckDlgButton(RedioButtonId, FALSE);
		UpdateWizardButtons();
		return FALSE ;
	}
	((CImportWizardDlg*)GetParent())->m_irtRecordType = RecordType;
	return TRUE;
}

void CImportWizardTypeDlg::OnChangeImportFileName()
{
	try{
		UpdateWizardButtons();
	}NxCatchAll("Error changing import file name");


}

BOOL CImportWizardTypeDlg::GetImportFileName(CString &strFileName) {

	CFileDialog dlgBrowse(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Comma Separated Values (*.csv)|*.csv|Tab Delimited Text Files (*.txt)|*.txt;*.tab|All Files|*.*|");
	
	// (b.savon 2015-07-06 14:29) - PLID 66489 - It appears to not remember by design.  It's overwriting the "last path" with the shared path.
	// Removed explicity set.

	if (dlgBrowse.DoModal() == IDOK) 
	{	// If the user clicked okay, that means he or she selected a file so remember it
		strFileName = dlgBrowse.GetPathName();
		return TRUE;
	}
	else {
		//they cancelled so we are not going to do anything
		return FALSE;
	}
}

void CImportWizardTypeDlg::OnImportFileBrowse() 
{
	try{
		CString strImportFileName;
		if(GetImportFileName(strImportFileName)){
			SetDlgItemText(IDC_IMPORT_FILE_NAME, strImportFileName);
			if(strImportFileName.Right(4).CompareNoCase(".txt") == 0){
				//Set file type to tab
				m_pFileType->SetSelByColumn(0, (long)ftvTab);
				HandleSelectedFileType(ftvTab);
			}
			else if(strImportFileName.Right(4).CompareNoCase(".csv") ==0){
				//Set file type to csv
				m_pFileType->SetSelByColumn(0, (long)ftvCSV);
				HandleSelectedFileType(ftvCSV);
			}
		}
	}NxCatchAll("Error browsing for import file");
	
}

BEGIN_EVENTSINK_MAP(CImportWizardTypeDlg, CNxPropertyPage)
    //{{AFX_EVENTSINK_MAP(CImportWizardTypeDlg)
	ON_EVENT(CImportWizardTypeDlg, IDC_IMPORT_FILE_TYPE, 2 /* SelChanged */, OnSelChangedImportFileType, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CImportWizardTypeDlg, IDC_IMPORT_FILE_TYPE, 16 /* SelChosen */, OnSelChosenImportFileType, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CImportWizardTypeDlg::OnSelChangedImportFileType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{

}

void CImportWizardTypeDlg::OnSelChosenImportFileType(LPDISPATCH lpRow) 
{
	try{
		IRowSettingsPtr pRow(lpRow);
		long nFileType = VarLong(pRow->GetValue(0), -1);
		HandleSelectedFileType(nFileType);

	}NxCatchAll("Error selecting Import File type");
}

//(e.lally 2007-06-18) PLID 26364 - (Labeling the parsing part of the import wizard, split from 5288)
void CImportWizardTypeDlg::HandleSelectedFileType(long nFileType)
{
	switch(nFileType){
	case ftvCSV:
		((CImportWizardDlg*)GetParent())->m_strFieldSeparator = ",";
		break;
	case ftvTab:
		((CImportWizardDlg*)GetParent())->m_strFieldSeparator = "\t";
		break;
	case ftvOther:
		GetDlgItem(IDC_IMPORT_FIELD_DELIMITER)->ShowWindow(TRUE);
		GetDlgItem(IDC_IMPORT_DELIMITER_LABEL)->ShowWindow(TRUE);
		return;// we can stop here
	}
	GetDlgItem(IDC_IMPORT_FIELD_DELIMITER)->ShowWindow(FALSE);
	GetDlgItem(IDC_IMPORT_DELIMITER_LABEL)->ShowWindow(FALSE);
}

void CImportWizardTypeDlg::OnImportUseTextQualifier()
{
	//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
	try{
		BOOL bEnabled = FALSE;
		if(IsDlgButtonChecked(IDC_IMPORT_USE_TEXT_QUALIFIER)){
			bEnabled = TRUE;
			CString strQualifier;
			GetDlgItemText(IDC_IMPORT_TEXT_QUALIFIER, strQualifier);
			if(strQualifier.IsEmpty())
				SetDlgItemText(IDC_IMPORT_TEXT_QUALIFIER, "\""); //double quote
		}
		else{
			bEnabled = FALSE;
		}

		GetDlgItem(IDC_IMPORT_TEXT_QUALIFIER)->EnableWindow(bEnabled);


	}NxCatchAll("Error enabling the use of a Text Qualifier");
}



//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
void CImportWizardTypeDlg::OnBnClickedImportInsuredParties()
{
	try{
		HandleRadioButton();

	}NxCatchAll("Error selecting import file type: Insured Party File");
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardTypeDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	if (m_bNeedInit) {
		return;
	}

	//Get the delta, the difference in size
	int dx = cx - m_ClientSize.cx;
	int dy = cy - m_ClientSize.cy;

	//Rember the new size
	m_ClientSize.cx = cx;
	m_ClientSize.cy = cy;

	//Change the positions of each of the items, according to their anchors
	try{
		ChangeDlgItemPos(this, IDC_IMPORT_FILE_NAME, dx, dy, EChangeDlgItemPosAnchor::LeftTopRight);
		ChangeDlgItemPos(this, IDC_IMPORT_FILE_BROWSE, dx, dy, EChangeDlgItemPosAnchor::TopRight);
	}NxCatchAllIgnore();
}