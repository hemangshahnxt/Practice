// ImportWizardFields.cpp : implementation file
//

#include "stdafx.h"
#include "ImportWizardFieldsDlg.h"
#include "ImportWizardPreviewDlg.h"
#include "globalutils.h"
#include "InternationalUtils.h"
#include "ShowProgressFeedbackDlg.h"
// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
#include "ImportWizardDlg.h"
#include "GlobalDrawingUtils.h"
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
#include "CachedData.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//(s.dhole 7/9/2015 9:29 AM ) - PLID 65712
#define IMPORTDOUBLEQUOTE		"\""

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum AvailableFieldsColumns{
	afcFieldID=0,
	afcName,
	afcType,
};
enum AvailableFieldTypes{
	aftIgnore = -1,
	aftString = 0,
	aftLong,
	aftDate,
	aftCurrency,
	aftDouble,
	aftBool,	// (j.jones 2010-04-05 10:33) - PLID 38050
};

// (r.farnworth 2015-03-23 14:17) - PLID 65246
enum PatientIDMappingTypes{
	pmtUserDefined = 0,
	pmtGen1Custom1,
	pmtGen1Custom2,
	pmtGen1Custom3,
	pmtGen1Custom4,
};
/////////////////////////////////////////////////////////////////////////////
// CImportWizardFieldsDlg property page

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
IMPLEMENT_DYNCREATE(CImportWizardFieldsDlg, CNxPropertyPage)

CImportWizardFieldsDlg::CImportWizardFieldsDlg() : CNxPropertyPage(CImportWizardFieldsDlg::IDD)
{
	//{{AFX_DATA_INIT(CImportWizardFieldsDlg)
		m_nPreviewRecords =20;
		m_nCurrentColumn=0;
		m_bFirstRowIsHeader = FALSE;
		
	//}}AFX_DATA_INIT
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	m_bNeedInit = TRUE;
}

CImportWizardFieldsDlg::~CImportWizardFieldsDlg()
{
}

void CImportWizardFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportWizardFieldsDlg)
	DDX_Control(pDX, IDC_IMPORT_FIELD_LEFT, m_btnPrevField);
	DDX_Control(pDX, IDC_IMPORT_FIELD_RIGHT, m_btnNextField);
	DDX_Control(pDX, IDC_IMPORT_FIELD_HEADER, m_nxeditImportFieldHeader);
	DDX_Control(pDX, IDC_MAPPING_LABEL, m_nxstaticMappingLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportWizardFieldsDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CImportWizardFieldsDlg)
	ON_BN_CLICKED(IDC_IMPORT_FIELD_LEFT, OnImportFieldPrevious)
	ON_BN_CLICKED(IDC_IMPORT_FIELD_RIGHT, OnImportFieldNext)
	ON_BN_CLICKED(IDC_IMPORT_FILE_HAS_HEADER, OnImportFileHasHeader)
	//}}AFX_MSG_MAP
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_APPT_TYPES_AS_CONVERSION1, &CImportWizardFieldsDlg::OnBnClickedApptTypesAsConversion1)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CImportWizardFieldsDlg, CNxPropertyPage)
    //{{AFX_EVENTSINK_MAP(CImportWizardFieldsDlg)
	ON_EVENT(CImportWizardFieldsDlg, IDC_IMPORT_FIELD_TABLE, 19 /* LeftClick */, OnLeftClickFieldTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CImportWizardFieldsDlg, IDC_IMPORT_AVAIL_FIELDS, 16 /* SelChosen */, OnSelChosenAvailField, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportWizardFieldsDlg message handlers

BOOL CImportWizardFieldsDlg::OnInitDialog() 
{
	try{
		CNxPropertyPage::OnInitDialog();
	
		m_pFieldList = BindNxDataList2Ctrl(this, IDC_IMPORT_FIELD_TABLE, NULL,false);
		m_pAvailFields = BindNxDataList2Ctrl(this, IDC_IMPORT_AVAIL_FIELDS, NULL, false);
		// (r.farnworth 2015-03-23 11:04) - PLID 65246
		m_pPatientIDMapping = BindNxDataList2Ctrl(this, IDC_PATIENTID_MAPPING, NULL, false);
		FillMappingCombo();
		m_bHasBeenResized = FALSE;

		m_btnPrevField.AutoSet(NXB_LEFT);
		m_btnNextField.AutoSet(NXB_RIGHT);

		//default to 20, but leave ability for user to change at some point
		m_nPreviewRecords =20;
		m_bNeedsReset = TRUE;

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

		return TRUE;
	}NxCatchAll("Error in CImportWizardFieldsDlg::OnInitDialog");
	return FALSE;  
}


BOOL CImportWizardFieldsDlg::OnSetActive()
{
	((CImportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	if(m_bNeedsReset == TRUE){
		m_strDelimiter = ((CImportWizardDlg*)GetParent())->m_strFieldSeparator;
		//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
		m_strTextQualifier = ((CImportWizardDlg*)GetParent())->m_strTextQualifier;
		
		ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
		ShowMappingCombo(irtCurrent); // (r.farnworth 2015-03-23 11:04) - PLID 65246
		LoadAvailableFields(irtCurrent);
		LoadFile();
		m_nCurrentColumn =0;
		if(irtCurrent == irtMediNotes){
			SetMediNotesColumns();
		}

		// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - set up appointment related options
		m_bHasApptPurpose = false;
		if (irtCurrent == irtAppointments) {
			GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION1)->ShowWindow(TRUE);
			GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION1)->EnableWindow(TRUE);
			GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->ShowWindow(TRUE);
			// (r.goldschmidt 2016-03-15 15:10) - PLID 67974 - adding to note is no longer a dependent option
			GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION1)->ShowWindow(FALSE);
			GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION1)->EnableWindow(FALSE);
			GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->ShowWindow(FALSE);
			GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->EnableWindow(FALSE);
		}

		HandleSelectedColumn(m_nCurrentColumn);
		GetDlgItem(IDC_IMPORT_FIELD_TABLE)->SetFocus();
		m_bNeedsReset = FALSE;
	}

	// (j.politis 2015-04-30 10:49) - PLID 65524 - Allow the import wizard to be resizable
	//When we become active, tell our parent
	CImportWizardDlg *pSheet = dynamic_cast<CImportWizardDlg *>(GetParentSheet());
	if (pSheet != NULL) {
		pSheet->OnPageSetActive(this);
	}

	return CNxPropertyPage::OnSetActive();
}

BOOL CImportWizardFieldsDlg::OnKillActive()
{
	try {
		//(e.lally 2007-08-13) PLID 26844 - Use a boolean instead of an array to
			//determine whether or not to leave this page
		if(m_bCanContinue == TRUE)
			return CNxPropertyPage::OnKillActive();
	}NxCatchAll("Error in CImportWizardFieldsDlg::OnKillActive");
	return FALSE;
}

LRESULT CImportWizardFieldsDlg::OnWizardBack()
{
	try{
		//Clear the rows, delete the columns, they have to start over
		m_pFieldList->Clear();
		for(int i = m_pFieldList->GetColumnCount() -1; i>=0; i--){
			m_pFieldList->RemoveColumn(i);
		}
		// (b.savon 2015-07-07 07:04) - PLID 66490 - Reset the "header" checkbox
		CheckDlgButton(IDC_IMPORT_FILE_HAS_HEADER, BST_UNCHECKED);

		// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - reset appointment specific options
		CheckDlgButton(IDC_APPT_TYPES_AS_CONVERSION1, BST_UNCHECKED);
		CheckDlgButton(IDC_APPT_TYPE_TO_NOTES1, BST_UNCHECKED);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->SetWindowText("Prepend Appointment Type to Appointment Notes ");

		//(e.lally 2007-08-13) PLID 26844 - Let the user leave this page
		m_bCanContinue = TRUE;
		m_bNeedsReset = TRUE;
		return CNxPropertyPage::OnWizardBack();
	}NxCatchAll("Error in CImportWizardFieldsDlg::OnWizardBack");
	return FALSE;
}

LRESULT CImportWizardFieldsDlg::OnWizardNext()
{
	try {
		//Re-parse the full file (yuck, I know), this time checking for validity of the data
		CFile fImportFile;
		CString strFileName = ((CImportWizardDlg*)GetParent())->m_strFileName;
		if(!fImportFile.Open(strFileName, CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite)) {
			MessageBox("Could not open the import file");
			return FALSE;
		}

		//Check for required fields
		//Users
		ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
		if(irtCurrent == irtUsers){
			//(e.lally 2007-06-12) PLID 26273 - Make sure a column is chosen as the username
			//Look for the username column
			IColumnSettingsPtr pCol;
			BOOL bFound = FALSE;
			for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount && bFound == FALSE; i++){
				pCol = m_pFieldList->GetColumn(i);
				//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
				if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnUserUsername))
					bFound = TRUE;
			}
			if(bFound == FALSE){
				m_bCanContinue = FALSE;
				//The user never specified a username field
				MessageBox("All 'User' imports must have a 'User Name' field specified before you can continue.");
				return FALSE;
			}
		}

		//Suggested fields
		//(e.lally 2007-07-02) PLID 26503 - Before re-parsing, detect the absence of first or last name fields and warn the user
		// (j.jones 2010-04-05 10:15) - PLID 16717 - only when not a service code
		// (b.savon 2015-03-30 16:54) - PLID 65231 - Add a new import type, Recalls, to the import utility
		// (r.farnworth 2015-04-06 12:35) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
		//(s.dhole 4/8/2015 9:53 AM ) - PLID 65230 Added PatientNotes exclusion
		// (b.savon 2015-04-06 10:34) - PLID 65216 - Create fields for the Appointment object for the import utility.
		//(s.dhole 4/14/2015 10:28 AM ) - PLID 65191  Exclude InsuredParty
		// (r.goldschmidt 2016-02-10 10:06) - PLID 68163 - exclude race import
		if (irtCurrent != irtServiceCodes && irtCurrent != irtResources && irtCurrent != irtProducts && irtCurrent != irtRecalls && irtCurrent != irtInsuranceCos && irtCurrent != irtAppointments && irtCurrent != irtPatientNotes 
			&& irtCurrent != irtInsuredParties && irtCurrent != irtRaces) {
			BOOL bHasFirstName = FALSE, bHasLastName = FALSE;
			IColumnSettingsPtr pCol;
			for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasFirstName == FALSE || bHasLastName==FALSE); i++){
				pCol = m_pFieldList->GetColumn(i);
				//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
				if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnFirstName))
					bHasFirstName = TRUE;
				//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
				if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnLastName))
					bHasLastName = TRUE;
			}

			// (b.savon 2015-03-24 10:27) - PLID 65160 - Modify validation for Patients import -- Patient Last Name 
			if (irtCurrent == irtPatients && bHasLastName == FALSE){
				m_bCanContinue = FALSE;
				MessageBox("All 'Patient' imports must have a 'Last Name' field specified before you can continue.", "Nextech", MB_ICONINFORMATION);
				return FALSE;
			}

			if(bHasFirstName == FALSE || bHasLastName==FALSE){
				m_bCanContinue = FALSE;
				CString strMessage;
				//The user never specified a first name field
				if(!bHasFirstName && bHasLastName)
					strMessage = "No column contains the 'First Name' field. Are you sure you wish to continue?";
				//The user never specified a last name field
				else if(bHasFirstName && !bHasLastName)
					strMessage = "No column contains the 'Last Name' field. Are you sure you wish to continue?";
				//The user never specified a first name or last name field
				else
					strMessage = "No column contains the 'First Name' or 'Last Name' fields. Are you sure you wish to continue?";

				long nResult = MessageBox(strMessage, NULL, MB_YESNO);
				if(nResult == IDNO)
					return FALSE;
			}
		}
		// (j.jones 2010-04-05 10:15) - PLID 16717 - service codes require code & description
		else if(irtCurrent == irtServiceCodes) {
			BOOL bHasCode = FALSE, bHasDesc = FALSE;
			IColumnSettingsPtr pCol;
			for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasCode == FALSE || bHasDesc == FALSE); i++){
				pCol = m_pFieldList->GetColumn(i);
				if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnServiceCode)) {
					bHasCode = TRUE;
				}
				if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnServiceName)) {
					bHasDesc = TRUE;
				}
			}
			if(bHasCode == FALSE || bHasDesc == FALSE){
				m_bCanContinue = FALSE;
				CString strMessage;
				if(!bHasCode && bHasDesc) {
					MessageBox("All 'Service Code' imports must have a 'Service Code' field specified before you can continue.");
					return FALSE;
				}
				else if(bHasCode && !bHasDesc) {
					MessageBox("All 'Service Code' imports must have a 'Description' field specified before you can continue.");
					return FALSE;
				}
				else {
					MessageBox("All 'Service Code' imports must have a 'Service Code' and a 'Description' field specified before you can continue.");
					return FALSE;
				}
			}
		}
		// (r.farnworth 2015-03-17 09:27) - PLID 65197 - Add a new import type, Resources, to the import utility
		else if (irtCurrent == irtResources) {
			BOOL bHasResourceName = FALSE;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasResourceName == FALSE); i++){
				pCol = m_pFieldList->GetColumn(i);
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnResourceName))
					bHasResourceName = TRUE;
			}
			if (!bHasResourceName){
				m_bCanContinue = FALSE;

				MessageBox("All 'Resource' imports must have a 'Resource Name' field specified before you can continue.");
				return FALSE;
			}
		}
		// (r.farnworth 2015-03-19 09:38) - PLID 65238 - Create fields for the Products object for the import utility.
		else if (irtCurrent == irtProducts) {
			BOOL bHasProductName = FALSE;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasProductName == FALSE); i++){
				pCol = m_pFieldList->GetColumn(i);
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnProductName))
					bHasProductName = TRUE;
			}
			if (!bHasProductName){
				m_bCanContinue = FALSE;

				MessageBox("All 'Product' imports must have a 'Product Name' field specified before you can continue.");
				return FALSE;
			}
		}
		// (r.farnworth 2015-04-01 14:55) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
		else if (irtCurrent == irtInsuranceCos) {
			BOOL bHasInsCoName = FALSE;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasInsCoName == FALSE); i++){
				pCol = m_pFieldList->GetColumn(i);
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnInsCoName))
					bHasInsCoName = TRUE;
			}
			if (!bHasInsCoName){
				m_bCanContinue = FALSE;

				MessageBox("All 'Insurance Company' imports must have an 'Insurance Co. Name' field specified before you can continue.");
				return FALSE;
			}
		}
		else if (irtCurrent == irtRecalls){ // (b.savon 2015-04-02 11:08) - PLID 65233 - Ensure we can't import a recall object without all the fields defined.
			bool bHasPatientMappingID = false;
			bool bHasRecallTemplate = false;
			bool bHasRecallDate = false;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i < nCount; i++){

				//If we have all the fields, don't bother checking anymore
				if (bHasPatientMappingID && bHasRecallTemplate && bHasRecallDate){
					break;
				}
				pCol = m_pFieldList->GetColumn(i);

				//Set flag if we encounter the column in question
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnCustomPatientID)){
					bHasPatientMappingID = true;
				}
				else if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnRecallDate)){
					bHasRecallDate = true;
				}
				else if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnRecallTemplateName)){
					bHasRecallTemplate = true;
				}
			}

			//If any of your flags are still cleared, we can't continue and need to display a message
			if (!bHasPatientMappingID || !bHasRecallTemplate || !bHasRecallDate){
				m_bCanContinue = FALSE;

				CString strMessage = "All 'Recall' imports must have a ";
				bool bHasOneAlready = false;
				if (!bHasPatientMappingID){
					strMessage += "'Patient Mapping ID' ";
					bHasOneAlready = true;
				}
				if (!bHasRecallDate){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Recall Date' ";
					bHasOneAlready = true;
				}
				if (!bHasRecallTemplate){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Recall Template' ";
				}

				strMessage += "field specified before you can continue.";

				MessageBox(strMessage, "Nextech Practice", MB_ICONWARNING);
				return FALSE;
			}
		}

		//(s.dhole 4/6/2015 5:00 PM ) - PLID  65226 Checking for requred patient note fields
		else if (irtCurrent == irtPatientNotes) {
			//(s.dhole 4/8/2015 4:24 PM ) - PLID 65224 patient notes field is required
			//(s.dhole 4/8/2015 4:24 PM ) - PLID 65226 Patient id field is required
			//(s.dhole 4/8/2015 4:24 PM ) - PLID 65229 Note Priority field is required
			//(s.dhole 5/4/2015 3:56 PM ) - PLID 65227 Note Date filed is required
			bool bHasCustomPatientID = false ;
			bool bHasPatientNote = false;
			//bool bHasPatientNotePriority = false;
			bool bHasPatientNoteDate = false;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i < nCount; i++){
				pCol = m_pFieldList->GetColumn(i);
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnCustomPatientID)){
					bHasCustomPatientID = true;
				}
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnPatientNoteText)){
					bHasPatientNote = true;
				}
				//Priority is not required
			/*	if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnPatientNotePriority)){
					bHasPatientNotePriority = true;
				}*/
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnPatientNoteDateTime)){
					bHasPatientNoteDate = true;
				}
			}
			CString strMessage = "All 'Patient Notes' imports must have a ";
			bool bHasOneAlready = false;
			if (!bHasCustomPatientID){
				strMessage += "'Patient Mapping ID' ";
				bHasOneAlready = true;
			}
			if (!bHasPatientNote){
				if (bHasOneAlready){
					strMessage += "and a ";
				}
				strMessage += "'Note' ";
				bHasOneAlready = true;
			}
		/*	if (!bHasPatientNotePriority){
				if (bHasOneAlready){
					strMessage += "and a ";
				}
				strMessage += "'Note Priority' ";
			}*/
			if (!bHasPatientNoteDate){
				if (bHasOneAlready){
					strMessage += "and a ";
				}
				strMessage += "'Note Date' ";
			}
			if (!bHasCustomPatientID || !bHasPatientNote /*|| !bHasPatientNotePriority*/ || !bHasPatientNoteDate){
				m_bCanContinue = FALSE;
				strMessage += "field specified before you can continue.";
				MessageBox(strMessage, "Nextech Practice", MB_ICONWARNING);
				return FALSE;
			}
			else
			{
				// nothing
			}
		}
		else if (irtCurrent == irtAppointments){// (b.savon 2015-04-06 15:09) - PLID 65216 - Create fields for the Appointment object for the import utility.
			bool bHasPatientMappingID = false;
			bool bHasDate = false;
			bool bHasResource = false;
			bool bHasStartTime = false;
			bool bHasEndTime = false;
			bool bHasDuration = false;
			bool bIsEvent = false;
			bool bHasValidTimeConfiguration = false;

			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i < nCount; i++){
				

				//If we have all the fields, don't bother checking anymore
				if (bHasPatientMappingID && bHasResource && bHasDate && bHasValidTimeConfiguration){
					break;
				}
				pCol = m_pFieldList->GetColumn(i);

				CString strColumnTitle = CString((LPCTSTR)pCol->GetColumnTitle());
				//Set flag if we encounter the column in question
				if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)){
					bHasPatientMappingID = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnResourceName)){
					bHasResource = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDate)){
					bHasDate = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentStartTime)){
					bHasStartTime = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentEndTime)){
					bHasEndTime = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDuration)){
					bHasDuration = true;
				}
				else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentIsEvent)){
					bIsEvent = true;
				}

				bHasValidTimeConfiguration = ((bIsEvent) || (bHasStartTime && bHasEndTime) || (bHasStartTime && bHasDuration));
			}

			//If any of your flags are still cleared, we can't continue and need to display a message
			if (!(bHasPatientMappingID && bHasResource && bHasDate && bHasValidTimeConfiguration)){
				m_bCanContinue = FALSE;

				CString strMessage = "All 'Appointments' imports must have a ";
				bool bHasOneAlready = false;
				if (!bHasPatientMappingID){
					strMessage += "'Patient Mapping ID' ";
					bHasOneAlready = true;
				}
				if (!bHasResource){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Resource name' ";
					bHasOneAlready = true;
				}
				if (!bHasDate){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Date' ";
					bHasOneAlready = true;
				}
				if (!bHasValidTimeConfiguration){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Start Time' with an 'End Time' or 'Duration' ";
				}

				strMessage += "field specified before you can continue.";

				MessageBox(strMessage, "Nextech Practice", MB_ICONWARNING);
				return FALSE;
			}
		}
		//(s.dhole 4/13/2015 3:07 PM ) - PLID 65191   - Add a new import type, Insure Party, to the import utility
		//(s.dhole 4/8/2015 4:24 PM ) - PLID 65194 Patient id field is required
		//(s.dhole 4/8/2015 4:24 PM ) - PLID 65193 Insurance conversion field is required
		//(s.dhole 4/8/2015 4:24 PM ) - PLID 65196 Relation field is required
		else if (irtCurrent == irtInsuredParties ) {
			bool bHasPatientMappingID = false;
			bool bHasInsuranceCompanyConversionID = false;
			bool bHasInsuredPartyRelation = false;
			IColumnSettingsPtr pCol;
			// check if required columns are exists and selected
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i < nCount; i++){
				//If we have all the fields, don't bother checking anymore
				if (bHasPatientMappingID && bHasInsuranceCompanyConversionID &&  bHasInsuredPartyRelation){
					break;
				}
				pCol = m_pFieldList->GetColumn(i);
				CString strColumnTitle = CString((LPCTSTR)pCol->GetColumnTitle());
				if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)){
					bHasPatientMappingID = true;
				}
				// required insurance comapny name or conversion id
				if (strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyConversionID) || strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName)){
					bHasInsuranceCompanyConversionID  = true;
				}

				if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRelation)){
					bHasInsuredPartyRelation = true;
				}
			}
			//If any of your flags are still cleared, we can't continue and need to display a message
			if (!(bHasPatientMappingID && bHasInsuranceCompanyConversionID && bHasInsuredPartyRelation)){
				m_bCanContinue = FALSE;

				CString strMessage = "All 'Insured Parties' imports must have a ";
				bool bHasOneAlready = false;
				if (!bHasPatientMappingID){
					strMessage += "'Patient Mapping ID' ";
					bHasOneAlready = true;
				}
				if (!bHasInsuranceCompanyConversionID){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Insurance Company (Name or Conversion ID)' ";
					bHasOneAlready = true;
				}
				if (!bHasInsuredPartyRelation){
					if (bHasOneAlready){
						strMessage += "and a ";
					}
					strMessage += "'Patient Relationship to the Insured Party' ";
				}

				strMessage += "field specified before you can continue.";

				MessageBox(strMessage, "Nextech Practice", MB_ICONWARNING);
				return FALSE;
			}
			
		}
		// (r.goldschmidt 2016-02-09 17:59) - PLID 68163 - Add new import type, Races, to the import utility
		else if (irtCurrent == irtRaces) {
			BOOL bHasPreferredName = FALSE;
			IColumnSettingsPtr pCol;
			for (int i = 0, nCount = m_pFieldList->GetColumnCount(); i<nCount && (bHasPreferredName == FALSE); i++) {
				pCol = m_pFieldList->GetColumn(i);
				if (CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnRacePreferredName))
					bHasPreferredName = TRUE;
			}
			if (!bHasPreferredName) {
				m_bCanContinue = FALSE;

				MessageBox("All 'Race' imports must have a 'Race Preferred Name' field specified before you can continue.");
				return FALSE;
			}
		}

		//(e.lally 2007-08-13) PLID 26844 - Reset our boolean for the existance of truncated data
		m_bContainsTruncatedData = FALSE;

		// (r.gonet 2010-09-01) - PLID 39733 - Reset the flag for existence of blank names in the data
		m_bContainsBlankNames = FALSE;

		// (b.eyers 2016-06-01) - PLID 68613 - We have too many columns in this row compared to the others. Warn and don't let them import
		if (m_bTooManyFields) {
			MessageBox("There is at least one row with too many fields. Mostly likely there are commas that need text qualifiers. "
				"You will need to fix the file before you can import it. ", "Warning", MB_ICONWARNING);
			return FALSE;
		}

		//Now parse the input file
		ParseFileToInterface(fImportFile, TRUE);

		// (r.goldschmidt 2016-02-02 12:44) - PLID 67976 - check if Conversion appointment type is valid
		bool bApptTypeAsConversionIsValid = ValidateApptTypeAsConversion();

		//Do we have any invalid data, if so, we have to stay here
		if(m_bCanContinue == FALSE){
			//we have invalid data
			//update the interface, tell the user, and return
			CString strMessage, strTruncMsg, strApptTypeConversionMsg, strTooManyFieldsMsg;
			strTruncMsg.Format("  - One or more values are longer than the maximum length for its selected field type. They are highlighted in orange.\n");
			CString strBlankNamesMsg = FormatString("  - At least one patient has both a blank first and last name.\n");
			strApptTypeConversionMsg.Format("  - Conversion Appointment Type needs to be fixed in Admin module > Scheduler tab.\n");
			// (b.eyers 2016-06-01) - PLID 68613 - There is a possibility we can get here and have the 'too many columns' issue
			strTooManyFieldsMsg.Format("  - There is at least one row with too many fields. You will need to fix the file before you can import it.\n");
			// (r.gonet 2010-09-01) - PLID 39733 - Show a specific message when a patient has blank first and last name.
			strMessage.Format("The following problems were detected with the data in the selected fields:\n"
				"  - One or more fields have invalid values for its selected field type. They are highlighted in red.\n"
				"%s"
				"%s"
				"%s"
				"%s"
				"\nYou may only continue once all selected fields have been validated.", 
				m_bContainsTruncatedData == TRUE ? strTruncMsg : "",
				m_bContainsBlankNames == TRUE ? strBlankNamesMsg : "",
				bApptTypeAsConversionIsValid ? "" : strApptTypeConversionMsg,
				m_bTooManyFields == TRUE ? strTooManyFieldsMsg : "");

			MessageBox(strMessage, "Invalid Data Found", MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
		if(m_bContainsTruncatedData){
			// (r.gonet 2010-09-01) - PLID 39733 - Not related to my item, but I corrected usage of "anyways" to its correct form, "anyway"
			if(IDCANCEL == MessageBox("One or more values are longer than the maximum length for its selected field type. \nThe shortened values are highlighted in orange.\n\n"
				"Do you want to continue anyway?", "Invalid Data Found", MB_OKCANCEL|MB_ICONQUESTION)){
				m_bCanContinue = FALSE;
				return FALSE;
			}
		}

		//All the fields are valid, now we can cast the columns as such
		UpdateColumnDataTypes();
		//This convoluted mess of a statement gets the active page index of this window then gets the next page
			//based on that and casts it as a preview dlg so that we can copy our datalist to the preview's datalist
		CImportWizardPreviewDlg  *pPreviewPage = (CImportWizardPreviewDlg*)((CImportWizardDlg*)GetParent())->GetPage(((CImportWizardDlg*)GetParent())->GetActiveIndex()+1);
		// (b.cardillo 2015-07-15 22:36) - PLID 66545 - Tell the preview page to leave a few rows
		// Tell the preview page to leave some records in our list in case the user wants to come 
		// back so they don't have to re-select columns.
		pPreviewPage->SetCopyFromDatalist(m_pFieldList, m_nPreviewRecords);
		// (r.farnworth 2015-03-23 16:21) - PLID 65246
		if (!m_mapPersonIDstoImportedUserDefined.IsEmpty()) {
			pPreviewPage->SetMappingFromFields(&m_mapPersonIDstoImportedUserDefined);
		}

		// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - carry over settings to next page so user is aware
		pPreviewPage->SetApptConversionCheckbox(IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION1));
		pPreviewPage->SetApptNoteCheckbox(IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES1));
		
		CString strText;
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->GetWindowText(strText);
		pPreviewPage->SetApptNoteCheckboxText(strText);

		// (r.goldschmidt 2016-02-02 17:13) - PLID 67976 - let the user know which columns won't convert
		// (r.goldschmidt 2016-03-15 15:10) - PLID 67974 - adding to note is no longer a dependent option; possible to have purpose without type now
		if (irtCurrent == irtAppointments) {
			if (IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION1) || IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES1)) {
				if (m_bHasApptType && m_bHasApptPurpose) {
					pPreviewPage->AddExplanatoryText("Selected Appointment Type and Purpose will not be converted.");
				}
				else if (m_bHasApptType) {
					pPreviewPage->AddExplanatoryText("Selected Appointment Type will not be converted.");
				}
				else if (m_bHasApptPurpose) {
					pPreviewPage->AddExplanatoryText("Selected Appointment Purpose will not be converted.");
				}
				else {
					pPreviewPage->AddExplanatoryText("");
				}
			}
		}

		return CNxPropertyPage::OnWizardNext();
	}NxCatchAll("Error in CImportWizardFieldsDlg::OnWizardNext");
	return FALSE;
}

void CImportWizardFieldsDlg::UpdateColumnDataTypes()
{
	IColumnSettingsPtr pCol;
	IRowSettingsPtr pAvailFieldsRow;
	CString strTitle;
	long nStyle;
	for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount; i++){
		nStyle =-1;
		pCol = m_pFieldList->GetColumn(i);
		if(pCol){
			//lookup the column style for this field
			strTitle = VarString(pCol->GetColumnTitle(), "");
			pAvailFieldsRow = m_pAvailFields->FindByColumn(afcName, _bstr_t(strTitle), NULL, FALSE);
			if(pAvailFieldsRow)
				nStyle = VarLong(pAvailFieldsRow->GetValue(afcType), -1);
			switch((AvailableFieldTypes) nStyle){
			case aftLong:
				pCol->PutDataType(VT_I4);
				break;
			case aftDate:
				pCol->PutDataType(VT_DATE);
				break;
			case aftCurrency:
				pCol->PutDataType(VT_CY);
				break;
			case aftDouble:
				pCol->PutDataType(VT_R8);
				break;
			// (j.jones 2010-04-05 10:36) - PLID 38050 - supported boolean types, but they are still tracked as strings
			case aftBool:
				pCol->PutDataType(VT_BSTR);
				break;
			}
		}
	}

}

#define ADD_AVAIL_ROW(ifnField, aftType) pRow = m_pAvailFields->GetNewRow(); \
		pRow->PutValue(afcFieldID, (long)ifnField); \
		pRow->PutValue(afcName, _bstr_t(GetImportFieldHeader(ifnField))); \
		pRow->PutValue(afcType, (long)aftType); \
		m_pAvailFields->AddRowAtEnd(pRow, NULL)
void CImportWizardFieldsDlg::LoadAvailableFields(ImportRecordType irtCurrent)
{
	m_pAvailFields->Clear();

	IRowSettingsPtr pRow;
	//Set our ignore row for all types
	pRow = m_pAvailFields->GetNewRow();
	pRow->PutValue(afcFieldID, (long)ifnIgnore);
	pRow->PutValue(afcName, _bstr_t(GetImportFieldHeader(ifnIgnore)));
	pRow->PutValue(afcType, (long)aftIgnore);
	m_pAvailFields->AddRowAtEnd(pRow, NULL);

	switch(irtCurrent){
	case irtPatients:
		LoadPersonFields();
		LoadPatientFields();
		LoadLocationFields(); // (b.savon 2015-03-19 14:59) - PLID 65144 - Support location
		break;
	case irtProviders:
		//(e.lally 2007-06-08) PLID 26262 - Add fields for providers
		LoadPersonFields();
		LoadProviderFields();
		break;
	case irtReferringPhysicians:
		//(e.lally 2007-06-11) PLID 10320 - Add fields for referring physicians
		LoadPersonFields();
		LoadRefPhysFields();
		break;
	case irtUsers:
		//(e.lally 2007-06-12) PLID 26273 - Add fields for users
		LoadPersonFields();
		LoadUserFields();
		break;
	case irtSuppliers:
		//(e.lally 2007-06-11) PLID 26274 - Add fields for suppliers
		LoadPersonFields();
		LoadSupplierFields();
		break;
	case irtOtherContacts:
		//(e.lally 2007-06-12) PLID 26275 - Add fields for other contacts
		LoadPersonFields();
		LoadOtherContactFields();
		break;
	case irtServiceCodes:
		// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
		LoadServiceFields();
		LoadCPTCodeFields();
		break;
	case irtMediNotes:
		//(e.lally 2007-06-29) PLID 26509 - Add fields for a MediNotes patient files
		ADD_AVAIL_ROW(ifnLastName, aftString);
		ADD_AVAIL_ROW(ifnFirstName, aftString);
		ADD_AVAIL_ROW(ifnMiddleName, aftString);
		ADD_AVAIL_ROW(ifnPatientID, aftLong);
		ADD_AVAIL_ROW(ifnAddress1, aftString);
		ADD_AVAIL_ROW(ifnCity, aftString);
		ADD_AVAIL_ROW(ifnState, aftString);
		ADD_AVAIL_ROW(ifnZip, aftString);
		ADD_AVAIL_ROW(ifnSocialSecurity, aftString);
		ADD_AVAIL_ROW(ifnHomePhone, aftString);
		ADD_AVAIL_ROW(ifnBirthdate, aftDate);
		ADD_AVAIL_ROW(ifnGender, aftString);
		//(e.lally 2007-06-29) PLID 26509 - Add work phone as an available field since it was
		// once part of our importer.
		ADD_AVAIL_ROW(ifnWorkPhone, aftString);

		break;
	case irtResources:
		// (r.farnworth 2015-03-16 14:23) - PLID 65197 - Add a new import type, Resources, to the import utility
		LoadResourceFields();
		break;
	case irtProducts:
		// (r.farnworth 2015-03-19 09:45) - PLID 65238 - Add a new import type, Products, to the't import utility
		LoadProductFields();
		break;
	case irtRecalls:
		// (b.savon 2015-03-30 15:40) - PLID 65231 - Add a new import type, Recalls, to the import utility
		LoadRecallFields();
		break;
	case irtInsuranceCos:
		// (r.farnworth 2015-04-01 15:24) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
		LoadInsuranceCoFields();
		break;
	case irtPatientNotes:
		//(s.dhole 4/2/2015 4:36 PM ) - PLID 65225  load patient note fields
		LoadPatientNoteFields();
		break;
	case irtAppointments: // (b.savon 2015-04-06 09:55) - PLID 65216 - Create fields for the Appointment object for the import utility.
		LoadResourceFields();
		LoadLocationFields();
		LoadAppointmentFields();
		break;
	case irtInsuredParties : //(s.dhole 4/13/2015 3:07 PM ) - PLID 65191   - Add a new import type, Insure Party, to the import utility
		LoadInsuredPartiesFields();
		break;
	case irtRaces: // (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add new import type, Races, to the import utility
		LoadRaceFields();
		break;

	}

	m_pAvailFields->Sort();
}

void CImportWizardFieldsDlg::LoadPersonFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnFirstName, aftString);
	ADD_AVAIL_ROW(ifnMiddleName, aftString);
	ADD_AVAIL_ROW(ifnLastName, aftString);
	ADD_AVAIL_ROW(ifnTitle, aftString);
	ADD_AVAIL_ROW(ifnAddress1, aftString);
	ADD_AVAIL_ROW(ifnAddress2, aftString);
	ADD_AVAIL_ROW(ifnCity, aftString);
	ADD_AVAIL_ROW(ifnState, aftString);
	ADD_AVAIL_ROW(ifnZip, aftString);
	ADD_AVAIL_ROW(ifnSocialSecurity, aftString);
	ADD_AVAIL_ROW(ifnBirthdate, aftDate);
	ADD_AVAIL_ROW(ifnGender, aftString);
	ADD_AVAIL_ROW(ifnHomePhone, aftString);
	ADD_AVAIL_ROW(ifnWorkPhone, aftString);
	ADD_AVAIL_ROW(ifnWorkExt, aftString);
	ADD_AVAIL_ROW(ifnCellPhone, aftString);
	ADD_AVAIL_ROW(ifnPager, aftString);
	ADD_AVAIL_ROW(ifnOtherPhone, aftString);
	ADD_AVAIL_ROW(ifnFax, aftString);
	ADD_AVAIL_ROW(ifnEmail, aftString);
	// (j.jones 2010-04-19 08:37) - PLID 38241 - added Company
	ADD_AVAIL_ROW(ifnCompany, aftString);

}

// (b.savon 2015-03-19 14:48) - PLID 65144 - Make location name its own function
void CImportWizardFieldsDlg::LoadLocationFields()
{
	IRowSettingsPtr pRow;
	ADD_AVAIL_ROW(ifnLocation, aftString);
}

void CImportWizardFieldsDlg::LoadPatientFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnPatientID, aftLong);
	// (b.savon 2015-03-19 14:55) - PLID 65144 - Add new fields to patient object for the import utility
	ADD_AVAIL_ROW(ifnProviderName, aftString);
	ADD_AVAIL_ROW(ifnEthnicity, aftString);
	ADD_AVAIL_ROW(ifnLanguage, aftString);
	ADD_AVAIL_ROW(ifnRace, aftString);
	ADD_AVAIL_ROW(ifnReferringPhysicianName, aftString);
	ADD_AVAIL_ROW(ifnPrimaryCarePhysicianName, aftString);
	ADD_AVAIL_ROW(ifnPatientCurrentStatus, aftString);
	ADD_AVAIL_ROW(ifnFirstContactDate, aftDate);
	ADD_AVAIL_ROW(ifnReferralSourceName, aftString);
	ADD_AVAIL_ROW(ifnEmergencyContactFirstName, aftString);
	ADD_AVAIL_ROW(ifnEmergencyContactLastName, aftString);
	ADD_AVAIL_ROW(ifnEmergencyContactRelation, aftString);
	ADD_AVAIL_ROW(ifnEmergencyContactHomePhone, aftString);
	ADD_AVAIL_ROW(ifnEmergencyContactWorkPhone, aftString);
	ADD_AVAIL_ROW(ifnWarningMessage, aftString);
	ADD_AVAIL_ROW(ifnNote, aftString);
	ADD_AVAIL_ROW(ifnGen1Custom1, aftString);
	ADD_AVAIL_ROW(ifnGen1Custom2, aftString);
	ADD_AVAIL_ROW(ifnGen1Custom3, aftString);
	ADD_AVAIL_ROW(ifnGen1Custom4, aftString);
	ADD_AVAIL_ROW(ifnCustomText1, aftString);
	ADD_AVAIL_ROW(ifnCustomText2, aftString);
	ADD_AVAIL_ROW(ifnCustomText3, aftString);
	ADD_AVAIL_ROW(ifnCustomText4, aftString);
	ADD_AVAIL_ROW(ifnCustomText5, aftString);
	ADD_AVAIL_ROW(ifnCustomText6, aftString);
	ADD_AVAIL_ROW(ifnCustomText7, aftString);
	ADD_AVAIL_ROW(ifnCustomText8, aftString);
	ADD_AVAIL_ROW(ifnCustomText9, aftString);
	ADD_AVAIL_ROW(ifnCustomText10, aftString);
	ADD_AVAIL_ROW(ifnCustomText11, aftString);
	ADD_AVAIL_ROW(ifnCustomText12, aftString);
	ADD_AVAIL_ROW(ifnCustomNote, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox1, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox2, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox3, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox4, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox5, aftString);
	ADD_AVAIL_ROW(ifnCustomCheckbox6, aftString);
	ADD_AVAIL_ROW(ifnCustomDate1, aftDate);
	ADD_AVAIL_ROW(ifnCustomDate2, aftDate);
	ADD_AVAIL_ROW(ifnCustomDate3, aftDate);
	ADD_AVAIL_ROW(ifnCustomDate4, aftDate);
	ADD_AVAIL_ROW(ifnMaritalStatus, aftString);
}

//(e.lally 2007-06-08) PLID 26262 - Add fields for providers
void CImportWizardFieldsDlg::LoadProviderFields()
{
	IRowSettingsPtr pRow;

	// (j.jones 2010-04-22 10:17) - PLID 38241 - removed Company, it is now in all Person fields
	//ADD_AVAIL_ROW(ifnCompany, aftString);
	ADD_AVAIL_ROW(ifnAccount, aftString);
	ADD_AVAIL_ROW(ifnProviderNPI, aftString);
	ADD_AVAIL_ROW(ifnProviderFederalEmpNumber, aftString);
	ADD_AVAIL_ROW(ifnProviderWorkersCompNumber, aftString);
	ADD_AVAIL_ROW(ifnProviderMedicaidNumber, aftString);
	ADD_AVAIL_ROW(ifnProviderLicenseNumber, aftString);
	ADD_AVAIL_ROW(ifnProviderBCBSNumber, aftString);
	ADD_AVAIL_ROW(ifnProviderTaxonomyCode, aftString);
	ADD_AVAIL_ROW(ifnProviderUPIN, aftString);
	ADD_AVAIL_ROW(ifnProviderMedicare, aftString);
	ADD_AVAIL_ROW(ifnProviderOtherID, aftString);
	ADD_AVAIL_ROW(ifnProviderDEANumber, aftString);

}

//(e.lally 2007-06-11) PLID 10320 - Add fields for referring physicians
void CImportWizardFieldsDlg::LoadRefPhysFields()
{
	IRowSettingsPtr pRow;

	// (j.jones 2010-04-22 10:17) - PLID 38241 - removed Company, it is now in all Person fields
	//ADD_AVAIL_ROW(ifnCompany, aftString);
	ADD_AVAIL_ROW(ifnAccount, aftString);
	ADD_AVAIL_ROW(ifnRefPhysNPI, aftString);
	ADD_AVAIL_ROW(ifnRefPhysFederalEmpNumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysWorkersCompNumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysMedicaidNumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysLicenseNumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysBCBSNumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysTaxonomyCode, aftString);
	ADD_AVAIL_ROW(ifnRefPhysUPIN, aftString);
	ADD_AVAIL_ROW(ifnRefPhysMedicare, aftString);
	ADD_AVAIL_ROW(ifnRefPhysOtherID, aftString);
	ADD_AVAIL_ROW(ifnRefPhysDEANumber, aftString);
	ADD_AVAIL_ROW(ifnRefPhysID, aftString);

}

//(e.lally 2007-06-12) PLID 26273 - Add fields for users
void CImportWizardFieldsDlg::LoadUserFields()
{
	IRowSettingsPtr pRow;

	// (j.jones 2010-04-22 10:17) - PLID 38241 - removed Company, it is now in all Person fields
	//ADD_AVAIL_ROW(ifnCompany, aftString);
	ADD_AVAIL_ROW(ifnAccount, aftString);
	ADD_AVAIL_ROW(ifnUserUsername, aftString);
	ADD_AVAIL_ROW(ifnUserNationalEmpNum, aftString);
	ADD_AVAIL_ROW(ifnUserDateOfHire, aftDate);
	// (j.jones 2010-04-05 10:33) - PLID 38050 - converted to boolean
	ADD_AVAIL_ROW(ifnUserPatientCoord, aftBool);

}

//(e.lally 2007-06-11) PLID 26274 - Add fields for suppliers
void CImportWizardFieldsDlg::LoadSupplierFields()
{
	IRowSettingsPtr pRow;

	// (j.jones 2010-04-22 10:17) - PLID 38241 - removed Company, it is now in all Person fields
	//ADD_AVAIL_ROW(ifnCompany, aftString);
	ADD_AVAIL_ROW(ifnAccount, aftString);
	ADD_AVAIL_ROW(ifnSupplierPayMethod, aftString);
}

//(e.lally 2007-06-12) PLID 26275 - Add fields for other contacts
void CImportWizardFieldsDlg::LoadOtherContactFields()
{
	IRowSettingsPtr pRow;

	// (j.jones 2010-04-22 10:17) - PLID 38241 - removed Company, it is now in all Person fields
	//ADD_AVAIL_ROW(ifnCompany, aftString);
	ADD_AVAIL_ROW(ifnAccount, aftString);
	// (j.jones 2010-04-05 10:33) - PLID 38050 - converted these to boolean
	ADD_AVAIL_ROW(ifnOtherContactNurse, aftBool);
	ADD_AVAIL_ROW(ifnOtherContactAnesthesiologist, aftBool);
}

// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
void CImportWizardFieldsDlg::LoadServiceFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnServiceName, aftString);
	ADD_AVAIL_ROW(ifnServicePrice, aftCurrency);
	// (j.jones 2010-04-05 10:35) - PLID 38050 - converted to a bool
	ADD_AVAIL_ROW(ifnTaxable1, aftBool);
	ADD_AVAIL_ROW(ifnTaxable2, aftBool);
	ADD_AVAIL_ROW(ifnBarcode, aftString);
}

// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
void CImportWizardFieldsDlg::LoadCPTCodeFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnServiceCode, aftString);
	ADD_AVAIL_ROW(ifnServiceSubCode, aftString);
	ADD_AVAIL_ROW(ifnRVU, aftDouble);
	ADD_AVAIL_ROW(ifnGlobalPeriod, aftLong);
	// (j.jones 2010-04-05 10:35) - PLID 38050 - converted to a bool
	// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	/*ADD_AVAIL_ROW(ifnPromptForCopay, aftBool);*/
}

// (r.farnworth 2015-03-16 14:44) - PLID 65197 - Add a new import type, Resources, to the import utility
void CImportWizardFieldsDlg::LoadResourceFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnResourceName, aftString);
}

// (r.farnworth 2015-03-19 09:45) - PLID 65238 - Add a new import type, Products, to the't import utility
void CImportWizardFieldsDlg::LoadProductFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnProductName, aftString);
	ADD_AVAIL_ROW(ifnProductDescription, aftString);
	ADD_AVAIL_ROW(ifnProductPrice, aftCurrency);
	ADD_AVAIL_ROW(ifnProductLastCost, aftCurrency);
	ADD_AVAIL_ROW(ifnProductTaxable1, aftBool);
	ADD_AVAIL_ROW(ifnProductTaxable2, aftBool);
	ADD_AVAIL_ROW(ifnProductBarcode, aftString);
	ADD_AVAIL_ROW(ifnProductBillable, aftBool);
	ADD_AVAIL_ROW(ifnProductOnHand, aftLong);
}

//(s.dhole 4/2/2015 4:36 PM ) - PLID 65225  load patient notes fields
void CImportWizardFieldsDlg::LoadPatientNoteFields()
{
	IRowSettingsPtr pRow;
	ADD_AVAIL_ROW(ifnCustomPatientID, aftString);
	ADD_AVAIL_ROW(ifnPatientNoteCategory, aftString);
	ADD_AVAIL_ROW(ifnPatientNoteText, aftString);
	ADD_AVAIL_ROW(ifnPatientNoteDateTime, aftDate);
	ADD_AVAIL_ROW(ifnPatientNotePriority, aftString);
}
// (b.savon 2015-03-30 17:26) - PLID 65233 - Create fields for the Recalls object for the import utility.
void CImportWizardFieldsDlg::LoadRecallFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnCustomPatientID, aftString);
	ADD_AVAIL_ROW(ifnRecallDate, aftDate);
	ADD_AVAIL_ROW(ifnRecallTemplateName, aftString);
}

// (r.farnworth 2015-04-01 15:26) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
void CImportWizardFieldsDlg::LoadInsuranceCoFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnConversionID, aftString);
	ADD_AVAIL_ROW(ifnInsCoName, aftString);
	ADD_AVAIL_ROW(ifnAddress1, aftString);
	ADD_AVAIL_ROW(ifnAddress2, aftString);
	ADD_AVAIL_ROW(ifnCity, aftString);
	ADD_AVAIL_ROW(ifnState, aftString);
	ADD_AVAIL_ROW(ifnZip, aftString);
	ADD_AVAIL_ROW(ifnContactFirst, aftString);
	ADD_AVAIL_ROW(ifnContactLast, aftString);
	ADD_AVAIL_ROW(ifnContactTitle, aftString);
	ADD_AVAIL_ROW(ifnContactPhone, aftString);
	ADD_AVAIL_ROW(ifnContactFax, aftString);
	ADD_AVAIL_ROW(ifnContactNote, aftString);
	ADD_AVAIL_ROW(ifnHCFAPayerID, aftString);
	ADD_AVAIL_ROW(ifnUBPayerID, aftString);
	ADD_AVAIL_ROW(ifnEligibilityPayerID, aftString);
}

// (b.savon 2015-04-06 09:57) - PLID 65216 - Create fields for the Appointment object for the import utility.
void CImportWizardFieldsDlg::LoadAppointmentFields()
{
	IRowSettingsPtr pRow;

	ADD_AVAIL_ROW(ifnCustomPatientID, aftString);
	ADD_AVAIL_ROW(ifnAppointmentDate, aftDate);
	ADD_AVAIL_ROW(ifnAppointmentIsEvent, aftString);
	ADD_AVAIL_ROW(ifnAppointmentStartTime, aftDate);
	ADD_AVAIL_ROW(ifnAppointmentEndTime, aftDate);
	ADD_AVAIL_ROW(ifnAppointmentDuration, aftLong);
	ADD_AVAIL_ROW(ifnAppointmentType, aftString);
	ADD_AVAIL_ROW(ifnAppointmentPurpose, aftString);
	ADD_AVAIL_ROW(ifnAppointmentNotes, aftString);
	ADD_AVAIL_ROW(ifnAppointmentIsConfirmed, aftString);
	ADD_AVAIL_ROW(ifnAppointmentIsCancelled, aftString);
	ADD_AVAIL_ROW(ifnAppointmentIsNoShow, aftString);
}

//(s.dhole 4/13/2015 3:07 PM ) - PLID 65191  - Add a new import type, Insure Party, to the import utility
void CImportWizardFieldsDlg::LoadInsuredPartiesFields()
{
	IRowSettingsPtr pRow;
	ADD_AVAIL_ROW(ifnCustomPatientID, aftString);
	ADD_AVAIL_ROW(ifnInsuranceCompanyConversionID, aftString);
	ADD_AVAIL_ROW(ifnInsuranceCompanyName, aftString);
	ADD_AVAIL_ROW(ifnFirstName, aftString);
	ADD_AVAIL_ROW(ifnMiddleName, aftString);//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
	ADD_AVAIL_ROW(ifnLastName, aftString);
	ADD_AVAIL_ROW(ifnAddress1 , aftString);
	ADD_AVAIL_ROW(ifnAddress2, aftString);
	ADD_AVAIL_ROW(ifnCity, aftString);
	ADD_AVAIL_ROW(ifnState, aftString);
	ADD_AVAIL_ROW(ifnZip, aftString);
	ADD_AVAIL_ROW(ifnInsuredEmployer, aftString);
	ADD_AVAIL_ROW(ifnBirthdate, aftDate);
	ADD_AVAIL_ROW(ifnHomePhone, aftString);
	ADD_AVAIL_ROW(ifnGender,  aftString);
	ADD_AVAIL_ROW(ifnInsuredInsuranceID,  aftString);
	ADD_AVAIL_ROW(ifnInsuredGroupNo, aftString);//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
	ADD_AVAIL_ROW(ifnInsuredCopay, aftCurrency);
	ADD_AVAIL_ROW(ifnInsuredCopayPercent, aftLong);
	ADD_AVAIL_ROW(ifnTitle, aftString);
	ADD_AVAIL_ROW(ifnInsuredEffectiveDate, aftDate);
	ADD_AVAIL_ROW(ifnInsuredInactiveDate, aftDate);
	ADD_AVAIL_ROW(ifnInsuredPartyRespTypeID, aftString);
	ADD_AVAIL_ROW(ifnInsuredPartyRelation, aftString);
}

// (r.goldschmidt 2016-02-09 15:23) - PLID 68163 - Add new import type, Races, to import utility
void CImportWizardFieldsDlg::LoadRaceFields()
{
	IRowSettingsPtr pRow;
	ADD_AVAIL_ROW(ifnRacePreferredName, aftString);
	ADD_AVAIL_ROW(ifnRaceCDCCode, aftString);
}

void CImportWizardFieldsDlg::LoadFile()
{
	try {
		//Open the file and start parsing
		CFile fImportFile;
		CString strFileName = ((CImportWizardDlg*)GetParent())->m_strFileName;
		if(!fImportFile.Open(strFileName, CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite)) {
			MessageBox("Could not open the import file");
			return;
		}
		// (b.eyers 2016-06-01) - PLID 68613
		m_bTooManyFields = FALSE;

		ParseFileToInterface(fImportFile); //This will close the import file
	}NxCatchAll("Error in CImportWizardFieldsDlg::LoadFile");
}


#define UPDATE_COLUMN_NAME(nColNum, strTitle, strDataField) \
if(m_pFieldList->GetColumnCount() > nColNum){ \
	pCol = m_pFieldList->GetColumn(nColNum); \
	if(pCol){ \
		pCol->PutColumnTitle(_bstr_t(strTitle)); \
		pCol->PutFieldName(_bstr_t(strDataField));}}
void CImportWizardFieldsDlg::SetMediNotesColumns()
{
	//(e.lally 2007-06-29) PLID 26509 - Auto set the fields for a MediNotes patient file to be the same order
		//as our previous importer.
	IColumnSettingsPtr pCol;
	int nColumn =0, nCol =0;
	
	//(e.lally 2007-08-13) PLID 26844 - Set our extended information for fields in use
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnLastName));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnLastName),	GetImportFieldDataField(ifnLastName)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnFirstName));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnFirstName),	GetImportFieldDataField(ifnFirstName)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnMiddleName));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnMiddleName), GetImportFieldDataField(ifnMiddleName)); nCol++;

	//(e.lally 2007-08-13) PLID 5288 - The old importer actually just ignored these Patient ID values. We
		//should default to the same behavior
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnIgnore),	GetImportFieldDataField(ifnIgnore));nCol++;

	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnAddress1));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnAddress1),	GetImportFieldDataField(ifnAddress1)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnCity));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnCity),		GetImportFieldDataField(ifnCity)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnState));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnState),		GetImportFieldDataField(ifnState)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnZip));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnZip),		GetImportFieldDataField(ifnZip)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnSocialSecurity));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnSocialSecurity), GetImportFieldDataField(ifnSocialSecurity)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnHomePhone));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnHomePhone),	GetImportFieldDataField(ifnHomePhone)); nCol++;
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnBirthdate),	GetImportFieldDataField(ifnBirthdate)); nCol++;
	AddColumnExtendedInfo(nCol, GetMaxFieldLength(ifnGender));
	UPDATE_COLUMN_NAME(nCol, GetImportFieldHeader(ifnGender),		GetImportFieldDataField(ifnGender)); nCol++;
}


#define INSERT_COLUMN(nColNum, strDataField, strName, nWidth, nStyle, FieldType) \
	m_pFieldList->InsertColumn(nColNum, _bstr_t(strDataField), strName, nWidth, nStyle); \
	pCol = m_pFieldList->GetColumn(nColumn); \
	pCol->PutFieldType(FieldType)


//(s.dhole 7/2/2015 4:15 PM ) - PLID 65712
long CImportWizardFieldsDlg::LastIndexOf(const CString& strWord, const CString& strSearch)
{
	long nfound = -1;
	long next_pos = strWord.GetLength();
	while (next_pos >= 0)
	{
		nfound = strWord.Find(strSearch, next_pos);
		if (nfound>0)
			return nfound;
		next_pos--;
	};
	return -1;
}


//(e.lally 2007-06-18) PLID 26364 - (Labeling the parsing part of the import wizard, split from 5288)
//This function will parse the file to the interface.  We can either parse a few records or all records with a verify
void CImportWizardFieldsDlg::ParseFileToInterface(CFile &fImportFile, BOOL bValidateFullFile /*=FALSE*/)
{
	//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
	CShowProgressFeedbackDlg *pProgressDlg = NULL;
	// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
	BOOL bRedrawDisabled = FALSE;
	try {
		CWaitCursor pWait;
		//Create a progress dialog since the file could be of unlimited length
		pProgressDlg = ((CImportWizardDlg*)GetParent())->m_pProgressDlg;
		if(pProgressDlg == NULL)
			pProgressDlg = new CShowProgressFeedbackDlg(500, FALSE);

		pProgressDlg->SetCaption("Reading Input File...");
		pProgressDlg->SetProgress(0, 100, 0);
		pProgressDlg->ShowProgress(SW_SHOW);


		//remove any rows in our preview like table
		m_pFieldList->Clear();
		//(e.lally 2007-08-13) PLID 26844 - Reset our boolean for allowing the user to leave this page
		m_bCanContinue = TRUE;

		CArchive ar(&fImportFile, CArchive::load);	//easier to read whole lines with archives

		//We'll make an array of strings to save the first few records so we don't have to keep opening the file
		m_aryPreviewRecords.RemoveAll();	//if anything was in it previously, we'll toss that out.

		CString strIn;
		int nCnt = 0;
		//(s.dhole 4/29/2015 12:22 PM ) - PLID   we allow to load  more then m_nPreviewRecords if user has m_strTextQualifier
		long nRowMultiplier =1;
		if (!m_strTextQualifier.IsEmpty())
		{
			nRowMultiplier = GetRemotePropertyInt("AllowedParagraphPerColumnInImport", 25, 0, "<None>", true);
		}

		while (ar.ReadString(strIn) && (nCnt < (m_nPreviewRecords * nRowMultiplier) || bValidateFullFile == TRUE)) {
			nCnt++;

			m_aryPreviewRecords.Add(strIn);
		}

		//Done with the file, we can close it up.
		ar.Close();
		fImportFile.Close();

		//Make sure there are records to import
		if(m_aryPreviewRecords.GetSize() == 0) {
			//Stop our progress bar
			if(pProgressDlg) {
				delete pProgressDlg;
				pProgressDlg = NULL;
				((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
			}
			//Stop the progress bar, THEN show the message box
			MessageBox("There are no records in your file.  Please choose a file which has data to import.");
			return;
		}

		long nTotalRecords = nCnt;
		if(nTotalRecords == 0) //This should NEVER be zero!
			nTotalRecords++;

		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		CProgressParameter progParam;
		//Set our progress subroutine to still use the 0-100 overall scale
		progParam.Init(pProgressDlg, 0, 100);
		//This section will fill out the 0-5 part of the 100
		progParam.SetSubRange(0, 5);

		BOOL bHasHeader = FALSE;
		if(IsDlgButtonChecked(IDC_IMPORT_FILE_HAS_HEADER)) {
			bHasHeader = TRUE;
		}

		//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
		BOOL bHasTextQualifier = FALSE;
		if(!m_strTextQualifier.IsEmpty())
			bHasTextQualifier = TRUE;

		_variant_t varNull;
		varNull.vt = VT_NULL;

		long nCurrentRecord = 0;

		//Task #1 - Read through the first record, see how many fields there are, and create columns.
			//Only needed if we aren't validating the full file.
		int nFieldCnt = 0;
		if(bValidateFullFile==FALSE){
			//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712  Row count
			long nRow = 0;  
			//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712  Row count
			//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712  first row caa be divided
			CString strTest = m_aryPreviewRecords.GetAt(nRow);	//always the first row

			//the last field doesn't end in a delimiter (by definition -- if it does, then we'll be adding 1 more afterwards)
			strTest += m_strDelimiter;

			//look for each delimiter
			long nPos =0;
			//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with a text qualifier
			//If we have a qualifier and our current character is that qualifier, find the ending qual. + 1
			//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
			strTest.TrimLeft();
			if(bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
				//We have a qualifier and this is a text field
				//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
				GetTextQualifierColumnValue(strTest, nPos, nRow);
			}
			else{
				//Just get the next delimiter position
				nPos = strTest.Find(m_strDelimiter);
			}

			while(nPos > -1) {
				//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
				//We don't know how many fields there will be, so set the max progress to end at 100
				progParam.SetProgress(0, 100, nFieldCnt);
				//delimiter found!
				nFieldCnt++;

				//Add a row to our field list.  If the header box is checked, we will label it as whatever data
				//	is in the field.  If it's not checked, we have no choice but to just call it "FieldX"
				CString strField;
				if(bHasHeader){
					//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with text qualifier
					//Remove any qualifiers
					if(bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
						strField = strTest.Left(nPos);
						strField.TrimRight(m_strTextQualifier);
						strField.TrimLeft(m_strTextQualifier);
					}
					else{
						strField = strTest.Left(nPos);
					}
				}
				else
					strField.Format("Field%li", nFieldCnt);

				//The preview pane needs to be setup with columns (all fields will be put in as text), so we need to 
				//	generate those at this time.
				IColumnSettingsPtr pCol;
				int nColumn = nFieldCnt - 1;
				CString strColumnHeader = "";
				//This is weird, but we are going to store the user given field name in the columns data field property.
				m_mapHeaders.SetAt((nFieldCnt-1), strField);
				//(s.dhole 4/29/2015 12:02 PM ) - PLID 65712 changed filed type to cftTextWordWrap 
				INSERT_COLUMN(nColumn, _bstr_t(""), _bstr_t(strColumnHeader), 100, csVisible, cftTextWordWrap);

				//now trim the strTest variable so this field is no longer part of it
				strTest = strTest.Right(strTest.GetLength() - nPos - 1);

				//look for another instance of our delimiter
				//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with a text qualifier
				//If we have a qualifier and our current character is that qualifier, find the ending qual. + 1
				//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
				strTest.TrimLeft();
				if(bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
					//We have a qualifier and this is a text field
					//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
					GetTextQualifierColumnValue(strTest, nPos, nRow);
				}
				else{
					//Just get the next delimiter position
					nPos = strTest.Find(m_strDelimiter);
				}
			}
		}//end if validating full file
		else{
			nFieldCnt = m_pFieldList->GetColumnCount();
		}

		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		//Set our subroutine to still use 0-100 overall
		progParam.Init(pProgressDlg, 0, 100);
		// (b.cardillo 2015-05-10 10:55) - PLID 65951 - Breaking into two phases
		//This section will be in charge of 5-30 of the 0-100
		progParam.SetSubRange(5, 30);
		pProgressDlg->SetCaption("Adding Preview Records...");
		nCurrentRecord = 0;
		//Task#2 - The field columns are now setup.  We need to scan through the records themselves
		//	and make a preview display.
		long i = 0;
		long nActualRowCount = 0;
		if (bHasHeader)
		{
			i++;	//skip the header field
			nActualRowCount++;
		}

		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		CCachedData cache;
		//(s.dhole 4/29/2015 10:39 AM ) - PLID 65712 change to while loop
		// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
		m_pFieldList->SetRedraw(VARIANT_FALSE);
		bRedrawDisabled = TRUE;
		// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
		DWORD dwLastProgressUpdate = GetTickCount() - 150;
		while (i < m_aryPreviewRecords.GetSize()) {
			//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
			// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
			if ((GetTickCount() - dwLastProgressUpdate) >= 150) {
				progParam.SetProgress(0, nTotalRecords, nCurrentRecord);
				dwLastProgressUpdate = GetTickCount();
			}
			//Increment our current record counter after setting the progress bar
			nCurrentRecord++;


			//Parse the fields out and setup a row in the preview list.  The first row parsed (above) is considered the final
			//	say in the count of fields.  If there is a discrepancy, we will:
			//	- If more fields in this row than the count, all extra fields will be thrown into the final field (with separators intact)
			//	- If there are less fields in this row than the count, all the missing fields (on the end) will be left blank.
			CString strTest = m_aryPreviewRecords.GetAt(i);

			//the last field doesn't end in a delimiter (by definition -- if it does, then we'll be adding 1 more afterwards)
			strTest += m_strDelimiter;

			int nCnt = 0;
			long nPos =0;
			//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with a text qualifier
			//If we have a qualifier and our current character is that qualifier, find the ending qual. + 1
			//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
			strTest.TrimLeft();
			if(bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
				//We have a qualifier and this is a text field
				//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
				GetTextQualifierColumnValue(strTest, nPos, i);
			}
			else{
				//Just get the next delimiter position
				nPos = strTest.Find(m_strDelimiter);
			}
			IRowSettingsPtr pRow = m_pFieldList->GetNewRow();	//current row
			CString strData;
			while(nPos > -1) {
				//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with a text qualifier
				//Remove any qualifiers
				if (bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
					strData = strTest.Left(nPos);
					//(s.dhole 7/2/2015 5:39 PM ) - PLID 65712
					// we want to remove only first and last text Qualifier 
					CString strTemp = strData;
					strTemp.TrimRight();
					strTemp.TrimLeft();
					if ((strTemp.GetLength() > 2) && (m_strTextQualifier == IMPORTDOUBLEQUOTE)
						&& (strTemp.Find(m_strTextQualifier) == 0 && strTemp.GetAt(strTemp.GetLength() - 1) == (CString)m_strTextQualifier)
						)
					{

						strData.Delete(strData.Find(m_strTextQualifier), 1);
						strData.Delete(LastIndexOf(strData, m_strTextQualifier), 1);
					}
					// Remove any double quotes
					strData.Replace(FormatString("%s%s", IMPORTDOUBLEQUOTE, IMPORTDOUBLEQUOTE), IMPORTDOUBLEQUOTE);
				}
				else{
					strData = strTest.Left(nPos);
				}

				//remove this data (+ delimiter) from the strTest string
				strTest = strTest.Right(strTest.GetLength() - nPos - 1);

				//increment the cnt of fields we've done
				nCnt++;

				//If we have just filled our last field, 
				//	and have anything leftover, we will have to append it to the data
				if(nCnt == nFieldCnt) {
					// (b.eyers 2016-06-01) - PLID 68613 - too many fields! warn and set bools
					if (!strTest.IsEmpty()) {
						//hey, there are still fields, warn about this and stop import
						CString strMessage;
						strMessage.Format("There are too many fields in line number %li. Mostly likely there are commas that need text qualifiers. "
							"You will need to fix the file before you can import it. ", i+1);
						MessageBox(strMessage, "Warning", MB_ICONWARNING);
						m_bTooManyFields = TRUE;
						m_bCanContinue = FALSE;
						strData += strTest;
						strTest.Empty();
						return;
					}
				}

				//next position
				//(e.lally 2007-06-07) PLID 26251 - Add ability to parse input files with a text qualifier
				//If we have a qualifier and our current character is that qualifier, find the ending qual. + 1
				//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
				strTest.TrimLeft();
				if(bHasTextQualifier && strTest.Left(1) == m_strTextQualifier){
					//We have a qualifier and this is a text field
					//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 
					GetTextQualifierColumnValue(strTest, nPos, i);
				}
				else{
					//Just get the next delimiter position
					nPos = strTest.Find(m_strDelimiter);
				}

				//Lastly, update the column in the current row
				pRow->PutValue(nCnt-1, _bstr_t(strData));
				strData="";//Just in case
			}

			if(nCnt < nFieldCnt) {
				//We have some extra fields, and don't want to leave them as VT_EMPTY's, just causes problems.
				//	Loop through these and put them as empty fields.
				for(int j = nCnt; j < nFieldCnt; j++) {
					pRow->PutValue(j, _bstr_t(""));
				}
			}

			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - For duplicate detection broke into two 
			// phases: insert and validate. This allows the validation to check for count > 1.

			// Simply add the plain row to the list (because this is the insert phase. We'll go 
			// back and validate all rows after we've added them.
			m_pFieldList->AddRowAtEnd(pRow, NULL);
			//(s.dhole 4/29/2015 10:39 AM ) - PLID 65712
			i++;
			nActualRowCount++;
			if ((nActualRowCount >= m_nPreviewRecords) && bValidateFullFile == FALSE)
			{
				// we are in non-preview mode and will show only m_nPreviewRecords rows
				break;
			}
		}
		// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
		progParam.SetProgress(0, nTotalRecords, nCurrentRecord);

		// (b.cardillo 2015-05-10 10:55) - PLID 65951 - For duplicate detection broke into two 
		// phases: insert and validate. This allows the validation to check for count > 1.
		//Set our subroutine to still use 0-100 overall
		progParam.Init(pProgressDlg, 0, 100);
		//This section will be in charge of 30-55 of the 0-100
		progParam.SetSubRange(30, 55);
		pProgressDlg->SetCaption("Validating Records...");
		nCurrentRecord = 0;
		
		// (b.cardillo 2015-05-10 10:55) - PLID 65951 - For duplicate detection broke into two 
		// phases: insert and validate. This allows the validation to check for count > 1.
		
		// Now go back and do the validation of all those rows (validation phase)
		dwLastProgressUpdate = GetTickCount() - 150;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldList->GetFirstRow(); pRow != NULL;) {
			if ((GetTickCount() - dwLastProgressUpdate) >= 150) {
				progParam.SetProgress(0, nTotalRecords, nCurrentRecord);
				dwLastProgressUpdate = GetTickCount();
			}
			//Increment our current record counter after setting the progress bar
			nCurrentRecord++;

			// We have one job: validate this row
			// (b.cardillo 2015-05-11 13:15) - It's odd to me that we're validating but then below 
			// we only reflect the validation result on screen if bValidateFullFile is in effect. 
			// So then why are we calling IsValid() every time? Indeed, why not check bValidateFullFile 
			// once before our loop and skip the loop entirely if it's false? Unfortunately, at the 
			// moment I can't guarantee that IsValid() doesn't DO something (beyond just checking 
			// for this particular row's validity; it has write access to our member variables, so 
			// it could be doing who knows what, in which case we could screw things up by failing 
			// to call it), so I'm not going to change it for now.
			RowValidityType rvtCurRow = IsValid(pRow, cache);

			// Get the next row (for iteration of our loop) before we manipulate our current row
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();

			// (r.farnworth 2015-03-25 11:25) - PLID 65164 - Make the rows where validation failed show at the top of the list.
			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - For duplicate detection broke into two 
			// phases: insert and validate. This allows the validation to check for count > 1.
			if (rvtCurRow == rvtInvalid || rvtCurRow == rvtInvalidSize){
				// Move the row to the beginning of the list by removing and then re-inserting it 
				// at the right spot (the beginning)
				m_pFieldList->RemoveRow(pRow);
				pRow = m_pFieldList->AddRowBefore(pRow, m_pFieldList->GetFirstRow());
			} else {
				// Valid, so leave the row where it is in the list.
			}

			//check for validity if we are doing the whole file
			if(bValidateFullFile){
				if(rvtCurRow == rvtInvalid){
					//This piece of data is not valid - stop the user from continuing
					m_bCanContinue = FALSE;
					InvalidateRow(pRow);
				}
				else if(rvtCurRow == rvtInvalidSize){
					InvalidateRow(pRow, TRUE);
				}
				else{
					ValidateRow(pRow);
				}
			}
			// Then iterate to the next row
			pRow = pNextRow;
		}
		// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
		progParam.SetProgress(0, nTotalRecords, nCurrentRecord);

		// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
		if (bRedrawDisabled) {
			m_pFieldList->SetRedraw(VARIANT_TRUE);
			bRedrawDisabled = FALSE;
		}

		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		//Stop our progress bar, but don't destroy it yet
		if(pProgressDlg) {
			pProgressDlg->ShowProgress(SW_HIDE);
			((CImportWizardDlg*)GetParent())->m_pProgressDlg = pProgressDlg;
			pProgressDlg = NULL;
		}
		return;
	} NxCatchAll("Error in ParseFileToInterface");
	// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
	try {
		if (bRedrawDisabled) {
			if (m_pFieldList) {
				m_pFieldList->SetRedraw(VARIANT_TRUE);
				bRedrawDisabled = FALSE;
			}
		}
	} NxCatchAllIgnore();
	try {
		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		//Let's make sure the progress dlg is cleaned up if we catch an error
		if(pProgressDlg){
			pProgressDlg->ShowProgress(SW_HIDE);
			((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
			delete pProgressDlg;
			pProgressDlg = NULL;
		}
	} NxCatchAllIgnore();
}

// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
RowValidityType CImportWizardFieldsDlg::IsValid(IRowSettingsPtr pRow, CCachedData &cache)
{
	//(e.lally 2007-08-13) PLID 26844 - Use enums for row and field validity instead of booleans.
		//This way we can have more than two states.
	RowValidityType rvtRowValidity = rvtValid;
	FieldValidityType fvtFieldValidity = fvtValid;
	CString strData;
	//(s.dhole 4/10/2015 9:12 AM ) - PLID 65226 Required import type
	ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
	//Clear out the array in case it contains old values
	m_mapCellValidity.RemoveAll();
	// (b.savon 2015-04-13 08:13) - PLID 65219 - Store appointment data for group dependency validation
	AppointmentData adDate, adStartTime, adEndTime, adDuration, adEvent;

	// (r.gonet 2010-09-01) - PLID 39733 - We need to tell if both the patient last name and first name are blank
	BOOL bLastNameBlank = FALSE, bFirstNameBlank = FALSE;
	// (r.gonet 2010-09-01) - PLID 39733 - Store the last name and first name field numbers in case we need to invalidate both together
	long nLastNameField = -1, nFirstNameField = -1;

	for(int i =0, nColumnCount = m_pFieldList->GetColumnCount(); i< nColumnCount; i++){
		fvtFieldValidity = fvtValid;
		_variant_t varData = pRow->GetValue(i);
		if (varData.vt != VT_EMPTY){
			strData = VarString(varData, "");

			//Cleanup the string to remove leading and trailing white spaces
			strData.TrimLeft();
			strData.TrimRight();
		}

		//get the field name of the column
		IColumnSettingsPtr pCol;
		pCol = m_pFieldList->GetColumn(i);
		ASSERT(pCol != NULL);

		CString strColumnTitle = VarString(pCol->GetColumnTitle(), "");

		//Handle special cases for required fields
		//(e.lally 2007-06-12) PLID 26273 - User names cannot be blank, cannot exist in the DB, and cannot exist
			//elsewhere in the import file
		if(strColumnTitle == GetImportFieldHeader(ifnUserUsername)){

			//check for empty names
			strData.TrimLeft();
			strData.TrimRight();
			if (strData.IsEmpty()){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

			//Check if this product exists elsewhere in our table. This could take a bit of processing depending on the number of rows
			// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
			if (cache.Count_DatalistValueInColumn(m_pFieldList, i, strData) > 1) {
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

			//Check if this username exists in the database
			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			if (cache.Exists_QueryValue("SELECT Username FROM UsersT WHERE Username <> ''", strData)) {
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

		}
		//(e.lally 2007-08-13) PLID 26844 - Patient IDs cannot exist in the DB, and cannot exist
			//elsewhere in the import file
		if(strColumnTitle == GetImportFieldHeader(ifnPatientID)){

			//Check if this patient ID exists elsewhere in our table. This could take a bit of processing depending on the number of rows
			// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
			if (cache.Count_DatalistValueInColumn(m_pFieldList, i, strData) > 1 || atol(strData) < -1){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

			//if it is blank, zero or -1, we can skip right over this part - these will signify autogenerated IDs
			if (strData.IsEmpty() || atol(strData) == 0 || atol(strData) == -1){
				fvtFieldValidity = fvtValid;
				continue;
			}

			//Check if this patientID exists in the database
			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			if (cache.Exists_QueryValue("SELECT UserDefinedID FROM PatientsT", atol(strData))) {
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}
		}

		// (r.farnworth 2015-04-01 10:14) - PLID 65246 - For Import objects that support Patient Mapping ID, add a dropdown list to allow the user to select the 
		// field to use to map the patient and support the map during import
		if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)){
			// (v.maida 2015-04-22 13:23) - PLID 65667 - Validate the patient mapping ID.
			// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
			BOOL bValidMapID = ValidatePatientMappingID(cache, strData);
			
			if (( bValidMapID == FALSE) || strData.IsEmpty()){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}
		}

		// (r.farnworth 2015-04-01 10:20) - PLID 65200 - Add validation for Resource object -- Resource Name field and ensure valid values are saved to data.
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateResourceName(cache, strData, strColumnTitle, m_pFieldList, i, ((CImportWizardDlg*)GetParent())->m_irtRecordType == irtAppointments)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (r.farnworth 2015-03-20 11:12) - PLID 65240 - Add validation for Products object -- Name field and ensure valid values are saved to data.
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateProductName(cache, strData, strColumnTitle, m_pFieldList, i)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-03-20 11:34) - PLID 65241 - Add validation for Products object -- Price field and ensure valid values are saved to data.
		if (!ValidateProductPrice(strData, strColumnTitle)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-03-20 11:55) - PLID 65242 - Add validation for Products object -- Barcode field and ensure valid values are saved to data.
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateProductBarcode(cache, strData, strColumnTitle, m_pFieldList, i)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-03-20 14:14) - PLID 65244 - Add validation for Products object -- On Hand Amount field and ensure valid values are saved to data.
		if (!ValidateProductOnHand(strData, strColumnTitle)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-04-06 14:45) - PLID 65168 - Add validation for Insurance Company object -- Insurance Company Name field and ensure valid values are saved to data.
		if (!ValidateInsCoName(strData, strColumnTitle)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-04-06 15:20) - PLID 65169 - Add validation for Insurance Company object -- Insurance Company Conversion ID field and ensure valid values are saved to data.
		// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
		if (!ValidateConversionID(cache, strData, strColumnTitle, m_pFieldList, i)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		// (r.farnworth 2015-04-07 08:58) - PLID 65189 - Add validation for Insurance Company object -- Default HCFA PayerID, Default UB PayerID, and Eligbility PayerID fields and ensure valid values are saved to data.
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidatePayerIDs(cache, strData, strColumnTitle)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-24 08:36) - PLID 65160 - Add validation for Patient Last Name
		if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtPatients){
			if (!ValidatePatientLastName(strData, strColumnTitle)){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}
		}

		// (r.gonet 2010-09-01) - PLID 39733 - Disallow patients without first and last names.
		//  We don't want to be importing a ton of blank records.
		if(strColumnTitle == GetImportFieldHeader(ifnFirstName) && strData.IsEmpty()) {
			bFirstNameBlank = TRUE;
			nFirstNameField = i;
		}
		else if(strColumnTitle == GetImportFieldHeader(ifnLastName) && strData.IsEmpty()) {
			bLastNameBlank = TRUE;
			nLastNameField = i;
		}
		if(bFirstNameBlank && bLastNameBlank) {
			// (r.gonet 2010-09-01) - PLID 39733 - We have determined that both the patient first and last names are blank, so mark the row as invalid
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(nFirstNameField, fvtInvalid);
			m_mapCellValidity.SetAt(nLastNameField, fvtInvalid);
			// (r.gonet 2010-09-01) - PLID 39733 - Set a flag for blank names in order for the caller to display a specific error message
			m_bContainsBlankNames = TRUE;
			continue;
		}

		// (j.jones 2010-04-05 11:37) - PLID 16717 - handle duplicate CPT / Subcodes
		if(strColumnTitle == GetImportFieldHeader(ifnServiceCode)){
			//if it is blank, mark it as invalid right now
			if(strData.IsEmpty()) {
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

			//find the subcode, if we have one
			CString strSubCode = "";
			int iSubCodeColIndex = -1;
			for(int j=0; j<nColumnCount && iSubCodeColIndex == -1; j++) {
				
				IColumnSettingsPtr pCol2;
				pCol2 = m_pFieldList->GetColumn(j);
				ASSERT(pCol2 != NULL);

				CString strColumnTitle2 = VarString(pCol2->GetColumnTitle(), "");
				if(strColumnTitle2 == GetImportFieldHeader(ifnServiceSubCode)) {
					iSubCodeColIndex = j;

					_variant_t varData2 = pRow->GetValue(iSubCodeColIndex);
					if(varData2.vt != VT_EMPTY) {
						strSubCode = VarString(varData2, "");
					}
				}
			}

			//Check if this code/subcode exists in the database
			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			if (cache.Exists_QueryValue("SELECT Code + '_andsubcode_' + SubCode FROM CPTCodeT", strData + "_andsubcode_" + strSubCode)) {
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}

			//Check if this combination exists elsewhere in our table. This could take a bit of processing depending on the number of rows
			IRowSettingsPtr pCurRow = m_pFieldList->GetFirstRow();
			BOOL bExists = FALSE;
			while(pCurRow != NULL && bExists == FALSE){
				// (b.cardillo 2015-06-28 20:51) - PLID 65951 (supplemental) - When I changed the 
				// way the validation loop worked (broke it up into two loops) I failed to find 
				// and convert this code that relied on the previous validate-as-you-go strategy. 
				// With our new add-all-then-validate-all strategy we have to exclude ourselves 
				// from our search.
				if (pCurRow != pRow) {
					CWaitCursor pWait;

					//if they did not select a subcode column, it will be treated as empty
					CString strSubCodeToCompare = "";
					if(iSubCodeColIndex != -1) {
						strSubCodeToCompare = VarString(pCurRow->GetValue(iSubCodeColIndex), "<None>");
					}

					if(VarString(pCurRow->GetValue(i), "<None>").CompareNoCase(strData) == 0
						&& strSubCodeToCompare.CompareNoCase(strSubCode) == 0) {
						bExists = TRUE;
					}
				}
				pCurRow =  pCurRow->GetNextRow();
			}
			if(bExists == TRUE || atol(strData) < -1){ 
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}
		}

		// (b.savon 2015-03-20 07:22) - PLID 65153 - Add validation for Race
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateRace(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-20 15:01) - PLID 65154 - Add validation for Ethnicity
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateEthnicity(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 07:32) - PLID 65155 - Add validation for Language
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateLanguage(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 08:13) - PLID 65156 - Add validation for Patients Current Status
		if (!ValidatePatientCurrentStatus(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 10:18) - PLID 65157 - Add validation for Location
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateLocation(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 11:28) - PLID 65158 - Add validation for Referral Source Name
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateReferralSource(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 13:18) - PLID 65159 - Add validation for First Contact Date
		if (!ValidateFirstContactDate(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-23 15:55) - PLID 65161 - Add validation for Marital Status
		if (!ValidateMaritalStatus(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-25 13:26) - PLID 65150 - Add validation for Patients import -- Provider Name
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidatePatientProvider(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-03-26 13:26) - PLID 65151 - Add validation for Patients import -- Referring Physician Name
		// (b.savon 2015-03-26 14:23) - PLID 65152 - Add validation for Patients import -- PCP Name 
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidatePatientReferringOrPCPName(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-01 11:31) - PLID 65235 - Add validation for Recalls object -- Recall date
		if (!ValidateRecallDate(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-02 09:18) - PLID 65236 - Add validation for Recalls object -- Template Name
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateRecallTemplateName(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-07 14:59) - PLID 65218 - Add validation for Appointment Date
		if (!ValidateAppointmentDate(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-10 13:02) - PLID 65219 - Add validation Appointment Start/End Time
		if (!ValidateAppointmentStartEndTime(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-07 15:24) - PLID 65220 - Add validation for Appointment Type
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - pass new parameter letting validation know if the set to conversion option is checked
		// (r.goldschmidt 2016-03-15 16:55) - PLID 67976 - also check notes option
		if (!ValidateAppointmentType(cache, strData, strColumnTitle, !!IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION1), !!IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES1))){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-10 10:58) - PLID 65221 - Add validation for Appointment Purpose
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		// (r.goldschmidt 2016-01-27 16:00) - PLID 67974 - pass new parameter letting validation know if the set to notes option is checked
		// (r.goldschmidt 2016-03-15 16:55) - PLID 67976 - also check conversion option
		if (!ValidateAppointmentPurpose(cache, strData, strColumnTitle, !!IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION1), !!IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES1))){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		
		//(s.dhole 4/15/2015 2:27 PM ) - PLID 65195  Add validation for Insurance Respo type
		if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRespTypeID)){
			// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
			CSqlFragment PatientSql = GetPatietnSQL(cache, pRow);
			if (!ValidateInsuredPartyRespoType(strData, strColumnTitle, pRow, m_pFieldList, PatientSql)){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
			}
		}

		//(s.dhole 4/15/2015 2:27 PM ) - PLID 65196 Add validation for Insured  relation 
		if (!ValidateInsuredPartyRelations(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		//(s.dhole 4/28/2015 9:48 AM ) - PLID 65193 Validate Insurace conversion id or insurance name
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if ((irtCurrent == irtInsuredParties) && (!ValidateInsuredPartyInsuComp(cache, strData, strColumnTitle, pRow, m_pFieldList))){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		
		// (r.farnworth 2015-04-29 10:48) - PLID 65525 - Ensure boolean values are validated before the preview screen silently changes them to false if they are invalid
		if (!ValidateBooleanValue(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		
		
		//(s.dhole 5/1/2015 2:17 PM ) - PLID 65755 
		if (!ValidateCopayPercent(strData, strColumnTitle)){
				fvtFieldValidity = fvtInvalid;
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(i, fvtInvalid);
				continue;
		}

		//(s.dhole 4/7/2015 8:55 AM ) - PLID 65229 Validate patient note priority
		if (!ValidatePatientPriority(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		//(s.dhole 4/7/2015 9:03 AM ) - PLID 65227 validate date
		if (!ValidateDate(ifnPatientNoteDateTime, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		//(s.dhole 4/7/2015 9:03 AM ) - PLID 65228 validate note category
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!ValidateNoteCategory(cache, strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
		if (!ValidateBirthDate(strData, strColumnTitle)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}
		//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
		if (!ValidateInsuredPartyInActiveDate(strData, strColumnTitle, pRow, m_pFieldList)){
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (r.goldschmidt 2016-02-10 12:44) - PLID 68163 - Validate Race Preferred Name
		if (!ValidateRacePreferredName(cache, strData, strColumnTitle, m_pFieldList, i)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (r.goldschmidt 2016-02-09 18:22) - PLID 68163 - Validate Race CDC Codes
		if (!ValidateRaceCDCCode(cache, strData, strColumnTitle)) {
			fvtFieldValidity = fvtInvalid;
			rvtRowValidity = rvtInvalid;
			m_mapCellValidity.SetAt(i, fvtInvalid);
			continue;
		}

		// (b.savon 2015-04-13 08:16) - PLID 65219 - Save appointment data
		if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDate)){
			adDate.nColumnIndex = i;
			adDate.strData = strData;
		} else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentStartTime)){
			adStartTime.nColumnIndex = i;
			adStartTime.strData = strData;
		} else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentEndTime)){
			adEndTime.nColumnIndex = i;
			adEndTime.strData = strData;
		} else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDuration)){
			adDuration.nColumnIndex = i;
			adDuration.strData = strData;
		} else if (strColumnTitle == GetImportFieldHeader(ifnAppointmentIsEvent)){
			adEvent.nColumnIndex = i;
			adEvent.strData = strData;
		}

		//If we get this far, empty fields are considered empty or null and can be considered valid values
		if(strData.IsEmpty()){
			fvtFieldValidity = fvtValid;
			continue;
		}

		//Get the data type of the corresponding available field entry
		if(strColumnTitle.IsEmpty()){
			strColumnTitle = GetImportFieldHeader(ifnIgnore);
		}

		//See if the data string can be converted into the datatype
		IRowSettingsPtr pAvailRow;
		pAvailRow = m_pAvailFields->FindByColumn(afcName, _bstr_t(strColumnTitle), m_pAvailFields->GetFirstRow(), FALSE);
		long nAvailFieldType = VarLong(pAvailRow->GetValue(afcType), (long) aftIgnore);
		if(nAvailFieldType == (long) aftIgnore){
			//This is always valid
		}
		else if(nAvailFieldType == (long) aftBool) {
			// (j.jones 2010-04-05 10:35) - PLID 38050 - booleans are always valid
			// because anything we can't determine to be a boolean is turned
			// into FALSE
		}
		else if(nAvailFieldType == (long) aftString){
			//(e.lally 2007-08-13) PLID 26844 - Account for strings that are longer than their max field size
			//This is always valid, unless it is too long
			int nMaxLength = -1;
			//Don't assert the lookup, assert the result of the lookup. Then it executes in release mode, silly.
			BOOL bLookup = m_mapStrColumnMaxLength.Lookup(i, nMaxLength);
			ASSERT(bLookup);
			if(strData.GetLength() > nMaxLength){
				//We have to truncate the data and warn the user.
				strData = strData.Left(nMaxLength);
				pRow->PutValue(i, _bstr_t(strData));
				m_mapCellValidity.SetAt(i, fvtInvalidSize);
				m_bContainsTruncatedData = TRUE;
				if(rvtRowValidity != rvtInvalid)
					rvtRowValidity = rvtInvalidSize;
				fvtFieldValidity = fvtInvalidSize;
			}
		}
		else if(nAvailFieldType == (long) aftLong){
			try{
				//For whatever reason atol doesn't throw errors on bad data
				long nValue = atol(strData);
				CString strValue;
				strValue.Format("%li", nValue);
				if(strData != strValue){
					rvtRowValidity = rvtInvalid;
					fvtFieldValidity = fvtInvalid;
				}
			}catch(...){rvtRowValidity = rvtInvalid; fvtFieldValidity = fvtInvalid;}
		}
		else if(nAvailFieldType == (long) aftDate){
			try{
				COleDateTime dtValue, dtMin, dtMax;
				// (v.maida 2015-04-08 10:28) - PLID 65423 - Use the ParseDateTime() utility function, now updated to consider ISO-8601 date formats.
				dtValue = ParseDateTime(strData, false);
				dtValue.SetDate(dtValue.GetYear(), dtValue.GetMonth(), dtValue.GetDay());
				dtMin.SetDate(1900,1,1);
				dtMax.SetDate(9999,12,31);
				if((dtValue.GetStatus() == COleDateTime::invalid) || dtValue < dtMin || dtValue > dtMax) {
					rvtRowValidity = rvtInvalid;
					fvtFieldValidity = fvtInvalid;
				}
			}catch(...){rvtRowValidity = rvtInvalid; fvtFieldValidity = fvtInvalid;}
		}
		else if(nAvailFieldType == (long) aftCurrency) {
			try{
				COleCurrency cyValue;
				cyValue.ParseCurrency(strData);
				// (b.eyers 2015-11-06) - PLID 34061 - round this before displaying since it'll be saved rounded
				RoundCurrency(cyValue);
				// (j.jones 2010-04-05 11:53) - PLID 38050 - improved the validation of currencies,
				// including disallowing negative currencies, since there is no current need for them
				// nor any perceived future needs for them
				if(cyValue.GetStatus() == COleCurrency::invalid || cyValue < COleCurrency(0,0)) {
					rvtRowValidity = rvtInvalid;
					fvtFieldValidity = fvtInvalid;
				}
				else if(strData.SpanIncluding("123456789").GetLength() > 1 && cyValue == COleCurrency(0,0)) {
					//somehow we parsed into a valid currency of 0.00, but the original data
					//had a non-zero number in it, so it can't possibly be intended to be 0.00
					rvtRowValidity = rvtInvalid;
					fvtFieldValidity = fvtInvalid;
				}
			}catch(...){rvtRowValidity = rvtInvalid; fvtFieldValidity = fvtInvalid;}
		}
		else if(nAvailFieldType == (long) aftDouble){
			try{
				//For whatever reason atof doesn't throw errors on bad data
				// (j.jones 2010-04-05 12:16) - PLID 16717 - I was getting strange
				// type mismatch exceptions in this code when the field was blank
				strData.TrimLeft();
				strData.TrimRight();
				if(strData.IsEmpty()) {
					rvtRowValidity = rvtInvalid;
					fvtFieldValidity = fvtInvalid;
				}
				else {
					double dblValue = atof(strData);
					CString strValue;
					strValue.Format("%0.09g", dblValue);
					if(strData != strValue){
						rvtRowValidity = rvtInvalid;
						fvtFieldValidity = fvtInvalid;
					}
				}
			}catch(...){rvtRowValidity = rvtInvalid; fvtFieldValidity = fvtInvalid;}
		}
		else{
			ASSERT(FALSE); //We need to implement whatever type is here
			rvtRowValidity = rvtInvalid;
			fvtFieldValidity = fvtInvalid;
		}
		

		if(fvtFieldValidity == fvtInvalid){
			m_mapCellValidity.SetAt(i,fvtInvalid);
		}

		fvtFieldValidity = fvtValid;
	}

	// (b.savon 2015-04-13 09:05) - PLID 65219 - Do appointment group dependency validation
	if (((CImportWizardDlg*)GetParent())->m_irtRecordType == irtAppointments){
		bool bIsEvent, bHasDate, bHasStartTime, bHasEndTime, bHasDuration;
		bIsEvent = false;

		if (adEvent.nColumnIndex != -1){
			bIsEvent = !GetSqlValueFromBoolString(adEvent.strData) ? false : true;
		}
		bHasDate = adDate.nColumnIndex != -1 ? true : false;
		bHasStartTime = adStartTime.nColumnIndex != -1 ? true : false;
		bHasEndTime = adEndTime.nColumnIndex != -1 ? true : false;
		bHasDuration = adDuration.nColumnIndex != -1 ? true : false;

		if (!bIsEvent && !bHasDate && !bHasStartTime && !bHasEndTime && !bHasDuration){
			return rvtRowValidity;
		}

		if (!bIsEvent){
			/* Start Time -- May never be blank unless it's an event */
			if (adStartTime.strData.IsEmpty()){
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(adStartTime.nColumnIndex, fvtInvalid);
			}

			/* End Time/Duration -- May never by empty unless Duration is greater than zero or it's an event */
			long nDuration = atol(adDuration.strData);
			CString strValue;
			strValue.Format("%li", nDuration);
			if (adEndTime.strData.IsEmpty() && nDuration <= 0){
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(adEndTime.nColumnIndex, fvtInvalid);
				m_mapCellValidity.SetAt(adDuration.nColumnIndex, fvtInvalid);
			}

			/* Start Time and End Time must fall in the same day and may not cross midnight and End Time must be after Start Time*/
			COleDateTime dtStartTime, dtEndTime;
			dtStartTime = ParseDateTime(adStartTime.strData, false);
			dtEndTime = ParseDateTime(adEndTime.strData, false);
			if (dtStartTime.GetStatus() == COleDateTime::valid && (dtEndTime.GetStatus() == COleDateTime::valid || nDuration > 0)){
				//If the end date is valid, compare the start time and end time are the same day.
				if (dtEndTime.GetStatus() == COleDateTime::valid){
					if (dtStartTime.GetYear() != dtEndTime.GetYear() || dtStartTime.GetMonth() != dtEndTime.GetMonth() || dtStartTime.GetDay() != dtEndTime.GetDay() || dtEndTime <= dtStartTime){
						rvtRowValidity = rvtInvalid;
						m_mapCellValidity.SetAt(adStartTime.nColumnIndex, fvtInvalid);
						m_mapCellValidity.SetAt(adEndTime.nColumnIndex, fvtInvalid);
					}
					else{
						m_mapCellValidity.RemoveKey(adStartTime.nColumnIndex);
						m_mapCellValidity.RemoveKey(adEndTime.nColumnIndex);
					}
				}else{ //If the end date isn't valid, then they have a valid duration -- Make sure the start time plus the duration is within today
					COleDateTime dtStartTimePlusDuration = dtStartTime + COleDateTimeSpan(0, 0, nDuration, 0);
					if (dtStartTimePlusDuration.GetStatus() == COleDateTime::valid){
						// If the constructed "end time" doesn't have the same day, flag it.
						if (dtStartTime.GetYear() != dtStartTimePlusDuration.GetYear() || dtStartTime.GetMonth() != dtStartTimePlusDuration.GetMonth() || dtStartTime.GetDay() != dtStartTimePlusDuration.GetDay()){
							rvtRowValidity = rvtInvalid;
							m_mapCellValidity.SetAt(adStartTime.nColumnIndex, fvtInvalid);
							m_mapCellValidity.SetAt(adDuration.nColumnIndex, fvtInvalid);
						}
						else{
							m_mapCellValidity.RemoveKey(adStartTime.nColumnIndex);
							m_mapCellValidity.RemoveKey(adDuration.nColumnIndex);
						}
					}
					else{
						/* Duration is invalid */
						rvtRowValidity = rvtInvalid;
						m_mapCellValidity.SetAt(adDuration.nColumnIndex, fvtInvalid);
					}
				}
			}
			else{
				rvtRowValidity = rvtInvalid;
				m_mapCellValidity.SetAt(adEvent.nColumnIndex, fvtInvalid);
				m_mapCellValidity.SetAt(adStartTime.nColumnIndex, fvtInvalid);
				m_mapCellValidity.SetAt(adEndTime.nColumnIndex, fvtInvalid);
				m_mapCellValidity.SetAt(adDuration.nColumnIndex, fvtInvalid);
			}
		}
		else{
			m_mapCellValidity.RemoveKey(adStartTime.nColumnIndex);
			m_mapCellValidity.RemoveKey(adEndTime.nColumnIndex);
			m_mapCellValidity.RemoveKey(adDuration.nColumnIndex);
		}

		if (m_mapCellValidity.GetCount() == 0){
			rvtRowValidity = rvtValid;
		}
	}

	//return validity of entire row
	return rvtRowValidity;
}

// (b.cardillo 2015-05-13 16:20) - PLID 66099 - Share a single FormatSettings object
NXDATALIST2Lib::IFormatSettingsPtr CImportWizardFieldsDlg::GetHyperlinkFormatSettings()
{
	if (m_pHyperLink == NULL) {
		m_pHyperLink = NXDATALIST2Lib::IFormatSettingsPtr(__uuidof(NXDATALIST2Lib::FormatSettings));
		//(s.dhole 4/29/2015 12:02 PM ) - PLID 65712 changed filed type to cftTextWordWrap
		m_pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextWordWrapLink);
	}
	return m_pHyperLink;
}

void CImportWizardFieldsDlg::InvalidateRow(IRowSettingsPtr pRow, BOOL bWarningOnly /* =FALSE*/)
{
	//(e.lally 2007-08-13) PLID 26844 - Invalid rows will use more than just light red. Set the
		//problematic cell to the darker color.
	FieldValidityType fvtCell;
	// (r.farnworth 2015-03-27 10:43) - PLID 65163 - Add Hyperlinking

	for(int i=0, nColCount = m_pFieldList->GetColumnCount(); i < nColCount; i++){
		fvtCell = fvtValid;
		m_mapCellValidity.Lookup(i, fvtCell);
		
		//(e.lally 2007-08-13) PLID 26844 - Store our permanent coloring of this cell in the selection color field.
			//This will be used to toggle back and forth between selected columns since we manually control
			//the coloring.
		switch(fvtCell){
		default:
		case fvtValid:
			if(bWarningOnly == FALSE){
				pRow->PutCellBackColor(i, ocLightRed);
				pRow->PutCellBackColorSel(i, ocLightRed);
			}
			else{
				pRow->PutCellBackColor(i, ocLightOrange);
				pRow->PutCellBackColorSel(i, ocLightOrange);
			}

			break;
		case fvtInvalid:
			pRow->PutCellBackColor(i, ocDarkRed);
			pRow->PutCellBackColorSel(i, ocDarkRed);

			// (r.farnworth 2015-03-27 10:08) - PLID 65163 - Add Hyperlinking
			pRow->PutCellLinkStyle(i, dlLinkStyleTrue);
			// (b.cardillo 2015-05-13 16:20) - PLID 66099 - Share a single FormatSettings object
			pRow->PutRefCellFormatOverride(i, GetHyperlinkFormatSettings());
			break;
		case fvtInvalidSize:
			pRow->PutCellBackColor(i, ocDarkOrange);
			pRow->PutCellBackColorSel(i, ocDarkOrange);

			// (r.farnworth 2015-03-27 10:08) - PLID 65163 - Add Hyperlinking
			pRow->PutCellLinkStyle(i, dlLinkStyleTrue);
			// (b.cardillo 2015-05-13 16:20) - PLID 66099 - Share a single FormatSettings object
			pRow->PutRefCellFormatOverride(i, GetHyperlinkFormatSettings());
			break;
		}
		//If this is our selected column, overwrite the displayed colors with the ones for selected columns.
		if(i == m_nCurrentColumn){
			pRow->PutCellBackColor(i, ocNavyBlue);
			pRow->PutCellForeColor(i, ocWhite);
		}
	}

}

void CImportWizardFieldsDlg::ValidateRow(IRowSettingsPtr pRow)
{
	for(int i=0, nColCount = m_pFieldList->GetColumnCount(); i < nColCount; i++){
		//(e.lally 2007-08-13) PLID 26844 - Store our permanent coloring of this cell in the selection field
			//This will be used to toggle back and forth between selected columns since we manually control
			//the coloring.
		pRow->PutCellBackColorSel(i, ocLightGreen);

		if(i != m_nCurrentColumn)
			pRow->PutCellBackColor(i, ocLightGreen);
		else{
			pRow->PutCellBackColor(i, ocNavyBlue);
			pRow->PutCellForeColor(i, ocWhite);
		}
	}
}


void CImportWizardFieldsDlg::OnLeftClickFieldTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try{
		IRowSettingsPtr pRow(lpRow);
		// (b.savon 2015-05-01 13:56) - PLID 65485 - Check if the row is null before continuing
		if (pRow == NULL){
			return; 
		}

		// (r.farnworth 2015-03-27 10:43) - PLID 65163 - Check if they are clicking a hyperlinked cell
		//(s.dhole 4/29/2015 12:09 PM ) - PLID 65712
		if (pRow->GetCellLinkStyle(nCol) == dlLinkStyleTrue && pRow->GetCellFormatOverride(nCol)->GetFieldType() == NXDATALIST2Lib::cftTextWordWrapLink) {
			CString strColumnTitle = VarString(m_pFieldList->GetColumn(nCol)->GetColumnTitle(), "");
			ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;

			int nMaxLength;
			bool bIncludeLength = false;
			if (m_mapStrColumnMaxLength.Lookup(nCol, nMaxLength)){
				bIncludeLength = true;
			}

			MessageBoxA(LookupErrorMessage(strColumnTitle, nMaxLength, bIncludeLength, irtCurrent));
		}
		else {
			HandleSelectedColumn(nCol);
		}
	}NxCatchAll("Error selecting field column");
}


void CImportWizardFieldsDlg::HandleSelectedColumn(short nCol) 
{
	m_pFieldList->SetRedraw(FALSE);
	IColumnSettingsPtr pCol;
	
	//Get the previously selected column
	if(m_nCurrentColumn != nCol && m_nCurrentColumn >= 0){
		pCol = m_pFieldList->GetColumn(m_nCurrentColumn);
		if(pCol){
			//Unhighlight it
			IRowSettingsPtr pRow = m_pFieldList->GetFirstRow();
			while(pRow){
				//Use the *hidden* selection colors to fill our actual displayed colors since we are controlling
					//all the selecting and coloring.
				pRow->PutCellBackColor(m_nCurrentColumn, pRow->GetCellBackColorSel(m_nCurrentColumn));
				pRow->PutCellForeColor(m_nCurrentColumn, pRow->GetCellForeColorSel(m_nCurrentColumn));
				pRow = pRow->GetNextRow();
			}

		}
	}
	//Get the new selected column
	pCol = m_pFieldList->GetColumn(nCol);
	if(pCol){

		//This is our selected column, highlight it
		//Add this color to the cell in this column because 
		//the row coloring has higher precedence than the column but the 
		//cell color has the highest precedence
		IRowSettingsPtr pRow = m_pFieldList->GetFirstRow();
		while(pRow){
			pRow->PutCellBackColor(nCol, ocNavyBlue);
			pRow->PutCellForeColor(nCol, ocWhite);
			pRow = pRow->GetNextRow();
		}


		//Update our displayed header information 
		CString strHeader;
		m_mapHeaders.Lookup(nCol, strHeader);
		SetDlgItemText(IDC_IMPORT_FIELD_HEADER, strHeader);
		//Set the field list dropdown
		if(m_pAvailFields->SetSelByColumn(afcName, pCol->GetColumnTitle())==NULL){
			//For now, do nothing if this happens.
		}
	}
	
	m_pFieldList->SetRedraw(TRUE);

	//Set our new current column ID
	m_nCurrentColumn = nCol;
	
	//Update the left, right arrows
	UpdateArrows();
	
}

void CImportWizardFieldsDlg::OnImportFieldPrevious() 
{
	try{
		if(m_nCurrentColumn>0){
			//this wasn't the first column, go previous
			HandleSelectedColumn(m_nCurrentColumn-1);
		}
	}NxCatchAll("Error selecting previous field");
}

void CImportWizardFieldsDlg::OnImportFieldNext() 
{
	try{
		if(m_nCurrentColumn < m_pFieldList->GetColumnCount()-1){
			//this wasn't the last column, go next
			HandleSelectedColumn(m_nCurrentColumn+1);	
		}
	}NxCatchAll("Error selecting previous field");
}

void CImportWizardFieldsDlg::UpdateArrows() 
{
	BOOL bNextEnabled = TRUE, bPreviousEnabled = TRUE;
	if(m_nCurrentColumn == m_pFieldList->GetColumnCount()-1)
		bNextEnabled = FALSE;
	if(m_nCurrentColumn == 0)
		bPreviousEnabled = FALSE;

	m_btnPrevField.EnableWindow(bPreviousEnabled);
	m_btnNextField.EnableWindow(bNextEnabled);

}


void CImportWizardFieldsDlg::OnSelChosenAvailField(LPDISPATCH lpRow) 
{
	try{
		IRowSettingsPtr pRow(lpRow);
		// (b.savon 2015-05-01 13:56) - PLID 65485 - Check if the row is null before continuing on
		if (pRow == NULL){
			return;
		}

		long nChosenFieldID = VarLong(pRow->GetValue(afcFieldID), (long)ifnIgnore);
		CString strChosenColumnHeader = GetImportFieldHeader((ImportFieldNumber)nChosenFieldID);
		CString strChosenDataField = GetImportFieldDataField((ImportFieldNumber)nChosenFieldID);
		long nFieldType = VarLong(pRow->GetValue(afcType), -1);
		//Are we ignoring this field?
		if(nChosenFieldID != (long)ifnIgnore){
			//No, then does any other field have this also selected
			CString strTempTitle, strTempDisplayName;
			IColumnSettingsPtr pCol;
			//Loop through each column and check for a duplicate selection
			for(int i=0, nColCount = m_pFieldList->GetColumnCount(); i < nColCount; i++){
				pCol= m_pFieldList->GetColumn(i);
				if(pCol != NULL && i != m_nCurrentColumn){
					_variant_t var;
					strTempTitle = VarString(pCol->GetColumnTitle(), "");
					var = pCol->GetFieldName();
					//(e.lally 2007-09-18) PLID 26554 - Reset the temp display name for correctness
						//Lookup the header for the current column in our loop, not the selected FieldID from the dropdown
					strTempDisplayName = "";
					m_mapHeaders.Lookup(i, strTempDisplayName);
					if(strChosenColumnHeader == strTempTitle){
						//This one is a duplicate
						CString strMessage;
						//(e.lally 2007-07-05) PLID 26554 - Don't allow multiple selections of the same field
						strMessage.Format("The '%s' field has already been selected by the '%s' column.\r\n\r\n"
							"Do you wish to mark the '%s' column as 'Ignore'? "
							"Selecting 'No' will cancel the field selection for the current column.", strChosenColumnHeader, strTempDisplayName, strTempDisplayName);
						if(IDYES == MessageBox(strMessage, "Warning", MB_YESNO)){
							//Set the other occurrence of this field to Ignore
							UPDATE_COLUMN_NAME(i, GetImportFieldHeader(ifnIgnore), "");
							RemoveColumnExtendedInfo(i);
						}
						else{
							//Cancel the selection of this field and reselect the previous avail field in the list
							IColumnSettingsPtr pCol = m_pFieldList->GetColumn(m_nCurrentColumn);
							CString strPreviousColumnHeader;
							if(pCol)
								strPreviousColumnHeader = VarString(pCol->GetColumnTitle(),"");
							m_pAvailFields->SetSelByColumn(afcName, _bstr_t(strPreviousColumnHeader));
							return;
						}
					}
				}//end if pCol
			}//end for loop
		}//end if chosen ID is not ignored

		//Finally, update the column title to be the new selected title
		IColumnSettingsPtr pCol;
		UPDATE_COLUMN_NAME(m_nCurrentColumn, strChosenColumnHeader, strChosenDataField);
		//(e.lally 2007-08-13) PLID 26844 - Set our extended information for the selected field
		if((AvailableFieldTypes)nFieldType == aftString)
			AddColumnExtendedInfo(m_nCurrentColumn, GetMaxFieldLength((ImportFieldNumber)nChosenFieldID));

		// (r.goldschmidt 2016-01-28 17:00) - PLID 67974 - adjust checkbox text if appt purpose is a set column
		// (r.goldschmidt 2016-03-16 17:00) - PLID 67974 - keep track if there is appt type too
		ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
		if (irtCurrent = irtAppointments) {
			m_bHasApptType = false;
			m_bHasApptPurpose = false;
			CString strTempTitle;
			IColumnSettingsPtr pColumn;
			//Loop through each column and check for appt purpose
			for (int i = 0, nColCount = m_pFieldList->GetColumnCount(); i < nColCount; i++) {
				pColumn = m_pFieldList->GetColumn(i);
				strTempTitle = VarString(pColumn->GetColumnTitle(), "");
				m_bHasApptType = m_bHasApptType || (strTempTitle == GetImportFieldHeader(ifnAppointmentType));
				m_bHasApptPurpose = m_bHasApptPurpose || (strTempTitle == GetImportFieldHeader(ifnAppointmentPurpose));
			}
			if (m_bHasApptPurpose) {
				GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->SetWindowText("Prepend Appointment Type and Purpose to Appointment Notes ");
			}
			else {
				GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->SetWindowText("Prepend Appointment Type to Appointment Notes ");
			}
		}

		// (b.savon 2015-07-06 14:49) - PLID 66492 - Automatically move to the next column in the list when you are finished with the current.
		OnImportFieldNext();
	}NxCatchAll("Error selecting available field name for selected column");
	
}

//(e.lally 2007-08-13) PLID 26844 - Utility functions for setting additional information not captured in the
	//datalist. We need to know the column index to manipulate this information.
void CImportWizardFieldsDlg::AddColumnExtendedInfo(long nColumn, int nMaxLength)
{
	m_mapStrColumnMaxLength.SetAt(nColumn, nMaxLength);

}

void CImportWizardFieldsDlg::RemoveColumnExtendedInfo(long nColumn)
{
	m_mapStrColumnMaxLength.RemoveKey(nColumn);
}

void CImportWizardFieldsDlg::OnImportFileHasHeader() 
{
	//(e.lally 2007-06-06) PLID 26241 - Be able to drop the first record and add it to the header information
		// as well as undo that change.
	try{
		if(IsDlgButtonChecked(IDC_IMPORT_FILE_HAS_HEADER))
			m_bFirstRowIsHeader = TRUE;
		else
			m_bFirstRowIsHeader =FALSE;

		TryAutoMatchColumnToDataField(); // (b.savon 2015-07-06 14:09) - PLID 66490
		HandleFieldHeaders();
	}NxCatchAll("Error selecting import file header option");
}

void CImportWizardFieldsDlg::HandleFieldHeaders()
{
	//(e.lally 2007-06-06) PLID 26241 - Be able to drop the first record and add it to the header information
		// as well as undo that change.
	IRowSettingsPtr pRow;
	if(m_bFirstRowIsHeader){
		//The first row contains header information so remove all the current header info
		pRow = m_pFieldList->GetFirstRow();
		if(pRow == NULL)
			return;
		m_mapHeaders.RemoveAll();
		CString strDisplayName;
		//Now go through all the columns of the first row and add the text to the header map.
		for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount; i++){
			strDisplayName = VarString(pRow->GetValue(i), "");
			m_mapHeaders.SetAt((long)i, strDisplayName);
			if(i == m_nCurrentColumn){
				//Update the display for the current field
				SetDlgItemText(IDC_IMPORT_FIELD_HEADER, strDisplayName);
			}
		}
		//Remove the first row from the table.
		m_pFieldList->RemoveRow(pRow);

	}
	else {
		//Move the header information back to the first row
		pRow = m_pFieldList->GetNewRow();
		CString strDisplayName;
		//For each column, get the current header display name, put it in our new row
		for(int i=0, nCount = m_pFieldList->GetColumnCount(); i<nCount; i++){
			m_mapHeaders.Lookup(i, strDisplayName);
			pRow->PutValue(i, _bstr_t(strDisplayName));
			strDisplayName.Format("Field %li", i+1); //First field should be Field 1
			m_mapHeaders.SetAt((long)i, strDisplayName);
			if(i == m_nCurrentColumn){
				//Update the display for the current field
				SetDlgItemText(IDC_IMPORT_FIELD_HEADER, strDisplayName);
				pRow->PutCellBackColor(i, ocNavyBlue);
				pRow->PutCellForeColor(i, ocWhite);
			}
		}
		//Add the row to the top of the list
		m_pFieldList->AddRowBefore(pRow, m_pFieldList->GetFirstRow());

	}
}

// (r.farnworth 2015-03-23 11:45) - PLID 65246 - Show/Hide and fill the data for the mapping combo box
void CImportWizardFieldsDlg::FillMappingCombo()
{
		IRowSettingsPtr pRow;
		pRow = m_pPatientIDMapping->GetNewRow();
		pRow->PutValue(0, pmtUserDefined);
		pRow->PutValue(1, "User Defined ID");
		m_pPatientIDMapping->AddRowAtEnd(pRow, NULL);
		pRow = m_pPatientIDMapping->GetNewRow();
		pRow->PutValue(0, pmtGen1Custom1);
		pRow->PutValue(1, "Gen 1 Custom 1");
		m_pPatientIDMapping->AddRowAtEnd(pRow, NULL);
		pRow = m_pPatientIDMapping->GetNewRow();
		pRow->PutValue(0, pmtGen1Custom2);
		pRow->PutValue(1, "Gen 1 Custom 2");
		m_pPatientIDMapping->AddRowAtEnd(pRow, NULL);
		pRow = m_pPatientIDMapping->GetNewRow();
		pRow->PutValue(0, pmtGen1Custom3);
		pRow->PutValue(1, "Gen 1 Custom 3");
		m_pPatientIDMapping->AddRowAtEnd(pRow, NULL);
		pRow = m_pPatientIDMapping->GetNewRow();
		pRow->PutValue(0, pmtGen1Custom4);
		pRow->PutValue(1, "Gen 1 Custom 4");
		m_pPatientIDMapping->AddRowAtEnd(pRow, NULL);
}

// (r.farnworth 2015-03-24 11:52) - PLID 65246 - Resize the table and hide/show the dropdown dynamically
void CImportWizardFieldsDlg::ShowMappingCombo(ImportRecordType irtCurrent)
{
	if (irtCurrent == irtRecalls || irtCurrent == irtInsuredParties
		|| irtCurrent == irtPatientNotes || irtCurrent == irtAppointments) {

		if (m_bHasBeenResized == TRUE) {
			CRect rcDatalist;
			CWnd *pwndDatalist = GetDlgItem(IDC_IMPORT_FIELD_TABLE);
			GetControlChildRect(this, pwndDatalist, &rcDatalist);
			rcDatalist.top += 30;
			MoveChildControl(this, pwndDatalist, rcDatalist);
			m_bHasBeenResized = FALSE;
		}

		GetDlgItem(IDC_PATIENTID_MAPPING)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MAPPING_LABEL)->ShowWindow(SW_SHOW);
		m_pPatientIDMapping->SetSelByColumn(0, 0);
	}
	else {
		GetDlgItem(IDC_PATIENTID_MAPPING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MAPPING_LABEL)->ShowWindow(SW_HIDE);

		if (m_bHasBeenResized == FALSE) {
			CRect rcDatalist;
			CWnd *pwndDatalist = GetDlgItem(IDC_IMPORT_FIELD_TABLE);
			GetControlChildRect(this, pwndDatalist, &rcDatalist);
			rcDatalist.top -= 30;
			MoveChildControl(this, pwndDatalist, rcDatalist);
			m_bHasBeenResized = TRUE;
		}
	}

}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardFieldsDlg::OnSize(UINT nType, int cx, int cy)
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
	try
	{
		ChangeDlgItemPos(this, IDC_IMPORT_FILE_HAS_HEADER, dx, dy, EChangeDlgItemPosAnchor::TopRight);
		ChangeDlgItemPos(this, IDC_IMPORT_FIELD_RIGHT, dx, dy, EChangeDlgItemPosAnchor::TopRight);
		ChangeDlgItemPos(this, IDC_IMPORT_AVAIL_FIELDS, dx, dy, EChangeDlgItemPosAnchor::LeftTopRight);
		ChangeDlgItemPos(this, IDC_IMPORT_FIELD_HEADER, dx, dy, EChangeDlgItemPosAnchor::LeftTopRight);
		ChangeDlgItemPos(this, IDC_PATIENTID_MAPPING, dx, dy, EChangeDlgItemPosAnchor::LeftTopRight);
		ChangeDlgItemPos(this, IDC_IMPORT_FIELD_TABLE, dx, dy, EChangeDlgItemPosAnchor::LeftTopRightBottom);
		ChangeDlgItemPos(this, IDC_APPT_TYPES_AS_CONVERSION1, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
		ChangeDlgItemPos(this, IDC_APPT_TYPE_TO_NOTES1, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
	}NxCatchAllIgnore()
}

//(s.dhole 4/28/2015 9:49 AM ) - PLID 65193 build patient sql 
// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
CSqlFragment  CImportWizardFieldsDlg::GetPatietnSQL(CCachedData &cache, NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	CSqlFragment PatientSql("");
	CString strData;
	for (int i = 0, nColumnCount = m_pFieldList->GetColumnCount(); i < nColumnCount; i++){
		IColumnSettingsPtr pCol;
		pCol = m_pFieldList->GetColumn(i);
		ASSERT(pCol != NULL);
		CString strColumnTitle = VarString(pCol->GetColumnTitle(), "");
		if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)){
			_variant_t varData = pRow->GetValue(i);
			if (varData.vt != VT_EMPTY){
				strData = VarString(varData, "");
				//Cleanup the string to remove leading and trailing white spaces
				strData.TrimLeft();
				strData.TrimRight();
			}
			// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
			BOOL bValidMapID = ValidatePatientMappingID(cache, strData);
			//get the field name of the column
			if (bValidMapID)
			{
				IRowSettingsPtr pRow = m_pPatientIDMapping->GetCurSel();
				long nMapID = pRow->GetValue(0);

				if (nMapID == pmtUserDefined) {
					if (atoi(strData) > 0 || atoi(strData) == -25) {
						PatientSql = CSqlFragment("SELECT PersonID FROM PatientsT WHERE UserDefinedID = {STRING}", strData);
						break;
					}
				}
				else if (nMapID == pmtGen1Custom1 || nMapID == pmtGen1Custom2 || nMapID == pmtGen1Custom3 || nMapID == pmtGen1Custom4) {
					PatientSql = CSqlFragment("SELECT PersonID FROM CustomFieldDataT WHERE FieldID = {INT} AND TextParam = {STRING}", nMapID, strData);
					break;
				}
			}
		}
	}
	return PatientSql;

}

// (v.maida 2015-04-22 10:41) - PLID 65667 - Checks if a patient mapping ID is valid.
// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
BOOL CImportWizardFieldsDlg::ValidatePatientMappingID(CCachedData &cache, CString& mappingID)
{
	long nMapID = -1;
	long nConvertedID = atol(mappingID);
	long nPersonID = -1;

	if (m_pPatientIDMapping != NULL && m_pPatientIDMapping->GetCurSel() != NULL) {
		nMapID = m_pPatientIDMapping->GetCurSel()->GetValue(0);
		if (!mappingID.IsEmpty() && mappingID.GetLength() <= 255 && nMapID != -1) {

			if (nMapID == pmtUserDefined) {
				// mapping ID corresponds to a UserDefinedID, which has stricter requirements than the custom fields.
				// IsNumeric() function strictly checks for positive numbers within a string. atol(), used above on mappingID, would still parse strings which have numbers 
				// somewhere inside them, but don't necessarily just contain numbers (like date strings), so atol() only used for checking the converted value's range, if it's a valid number.
				
				// UserDefinedIDs must be a positive number or -25, and must not exceed the maximum allowed value for a long variable.
				if ( (!IsNumeric(mappingID) && mappingID != "-25") || (nConvertedID <= 0 && nConvertedID != -25) || nConvertedID >= LONG_MAX) {
					return FALSE;
				}
			}

			// check that the ID exists within the database, storing the database PersonID in nPersonID, if validation succeeds
			// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
			if (!ValidatePatientMappingRecords(cache, nMapID, mappingID, nPersonID)) {
				return FALSE;
			}


			// not empty, less than 255 characters, and uniquely exists within the database
			// this ID has passed all other validation tests, so it can be added to the map and labeled as TRUE
			m_mapPersonIDstoImportedUserDefined[mappingID] = nPersonID;
			return TRUE;

		}
	}
	return FALSE;
}

// (v.maida 2015-04-22 10:41) - PLID 65667 - Checks if a patient mapping ID is valid within the database. Sets nPersonID to the database ID, in the event that mappingID is valid.
// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs all at 
// once so no matter how many times we are called, we only reach out to data the first time.
BOOL CImportWizardFieldsDlg::ValidatePatientMappingRecords(CCachedData &cache, long nMapID, CString &mappingID, long& nPersonID)
{
	// Figure out based on nMapID what SQL will return the map of unique mappingIDs (string) to 
	// nPersonIDs (long)
	CString sql;
	if (nMapID == pmtUserDefined) {
		// Easy, UserDefinedID is guaranteed unique already, so the query is simple.
		// (static string to avoid re-allocating on every call)
		static const CString s = "SELECT CONVERT(NVARCHAR, UserDefinedID), PersonID FROM PatientsT";
		sql = s;
	} else if (nMapID == pmtGen1Custom1 || nMapID == pmtGen1Custom2 || nMapID == pmtGen1Custom3 || nMapID == pmtGen1Custom4) {
		// Here it's possible for different patients to have the same custom field value, so we 
		// have to find only the distinct values that are used by exactly one patient
		// (static string to avoid re-allocating on every call)
		LPCTSTR sqlfmt = R"(
SELECT TextParam, PersonID 
FROM CustomFieldDataT 
WHERE FieldID IN (%li) 
GROUP BY TextParam, PersonID
HAVING COUNT(*) = 1
)"
			;
		// Create one (static) string for each kind of nMapID. Get a reference to that one.
		switch (nMapID) {
		case pmtGen1Custom1: { static const CString s = FormatString(sqlfmt, pmtGen1Custom1); sql = s; break; }
		case pmtGen1Custom2: { static const CString s = FormatString(sqlfmt, pmtGen1Custom2); sql = s; break; }
		case pmtGen1Custom3: { static const CString s = FormatString(sqlfmt, pmtGen1Custom3); sql = s; break; }
		case pmtGen1Custom4: { static const CString s = FormatString(sqlfmt, pmtGen1Custom4); sql = s; break; }
		default:
			// It should be impossible to get here because in our else if above we established 
			// that nMapID is one of these four values, so how could our switch have missed it?
			ASSERT(FALSE);
			ThrowNxException("Invalid nMapID value %li after establishing it was an expected value.", nMapID);
		}
	} else {
		ThrowNxException("Unknown nMapID value %li.", nMapID);
	}

	// We now have our reference to a static SQL statement (same every time). Use our caching 
	// mechanism to look up the mappingID (after, the first time, populating the cache).
	long ans;
	if (cache.GetValue_Query(sql, mappingID, ans)) {
		// Found it, return the result along with success
		nPersonID = ans;
		return TRUE;
	} else {
		// Didn't find it, return failure
		return FALSE;
	}
}



//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 parse column value if it has textQualifier
void CImportWizardFieldsDlg::GetTextQualifierColumnValue(CString &strLineText, long &nPos, long &nRow)
{
	long nNextDelimite = 1;
	bool bContinue = true;
	long nLineCount = 0;
	long nMAxRowPerColumn = GetRemotePropertyInt("AllowedParagraphPerColumnInImport", 25, 0, "<None>", true);
	while (bContinue)
	{
		//Get next Qualifier Position 
		// we want to check if TextQualifier is double QUOTE
		if (m_strTextQualifier == IMPORTDOUBLEQUOTE){
			CString sTemp = strLineText;
			// this it tem replacement
			sTemp.Replace(FormatString("%s%s", IMPORTDOUBLEQUOTE, IMPORTDOUBLEQUOTE), "--");
			nNextDelimite = sTemp.Find(m_strTextQualifier, nNextDelimite) + 1;
		}
		else
		{
			nNextDelimite = strLineText.Find(m_strTextQualifier, nNextDelimite) + 1;
		}
		// if we could not find  next TextQualifier position , that's mean we have to scan next line if it is available
		if (nNextDelimite == 0 && nRow  < m_aryPreviewRecords.GetSize())
		{

			// advance line count
			nRow++;
			// cleanup right delimiter which we added before
			int nDelimLength = m_strDelimiter.GetLength();
			CString strEndOfLine = strLineText.Right(nDelimLength);
			if (strEndOfLine == m_strDelimiter)
			{
				strLineText.Delete(strLineText.GetLength() - nDelimLength, nDelimLength);
			}

			strLineText += "\r\n";
			if (nRow < m_aryPreviewRecords.GetSize())
			{
				strLineText += m_aryPreviewRecords.GetAt(nRow);
			}
			else
			{// we do not want to continue if we are at last line
				break;
			}
			// Add again right delimiter 
			strLineText += m_strDelimiter;
			//Advance posistion since  it is move back to 0
			nNextDelimite = 1;
		}

		//now  check immediate character
		long nTemp = -1;
		if (m_strTextQualifier == IMPORTDOUBLEQUOTE){

			CString sTemp = strLineText;
			sTemp.Replace(FormatString("%s%s", IMPORTDOUBLEQUOTE, IMPORTDOUBLEQUOTE), "--");
			nTemp = sTemp.Find(m_strDelimiter, nNextDelimite);
		}
		else
		{
			nTemp = strLineText.Find(m_strDelimiter, nNextDelimite);
		}

		// check if we are at  Delimiteer position
		if (nTemp == nNextDelimite)
		{
			// we scan all characters and we have column value
			bContinue = false;
			nNextDelimite--;
		}
		nLineCount++;
		//This is failsafe mechanism, we are exxeding our limit to allow new line per column
		if (nLineCount > nMAxRowPerColumn)
		{
			bContinue = false;
		}
	}
	//We have a qualifier and this is a text field
	nPos = strLineText.Find(m_strTextQualifier, nNextDelimite) + 1;
	if (nPos == 0) nPos--;
}

// (b.savon 2015-07-06 12:37) - PLID 66490
void CImportWizardFieldsDlg::TryAutoMatchColumnToDataField()
{
	try{
		// Don't do anything if there is something wrong with our field list
		if (m_pFieldList == NULL){
			return;
		}

		// Get the first row in the file
		IRowSettingsPtr pDataRow = m_pFieldList->GetFirstRow();

		// We don't have a valid row, bail.
		if (pDataRow == NULL){
			return;
		}

		// Go through every column in the file
		for (int fileColumnIdx = 0; fileColumnIdx < m_pFieldList->GetColumnCount(); fileColumnIdx++){
			//Grab the data from the cell
			CString strFirstRowData = VarString(pDataRow->GetValue(fileColumnIdx), "");

			//Search for that name, exactly, in the available fields list
			IRowSettingsPtr pAvailableFieldsRow = m_pAvailFields->FindByColumn(afcName, AsBstr(strFirstRowData), NULL, VARIANT_FALSE);

			//If we found a match..
			if (pAvailableFieldsRow != NULL){
				//Check for uniqueness -- only check columns that we've previously attempted automatching on
				bool bIgnore = false;
				for (int uxFileColumn = 0; uxFileColumn < m_pFieldList->GetColumnCount(); uxFileColumn++){
					// Grab the column title
					CString strColumnTitle = AsString(m_pFieldList->GetColumn(uxFileColumn)->GetColumnTitle());

					//If it matches...
					if (strFirstRowData.CompareNoCase(strColumnTitle) == 0){
						//Set ignore flag
						bIgnore = true;
					}
				}

				// We have a match, and if it isn't already matched, match it.
				if (!bIgnore){
					long nFieldType = VarLong(pAvailableFieldsRow->GetValue(afcType), -1);
					long nChosenFieldID = VarLong(pAvailableFieldsRow->GetValue(afcFieldID), (long)ifnIgnore);

					IColumnSettingsPtr pCol;
					// (r.goldschmidt 2016-03-18 15:06) - PLID 68597 - fix column header to be the correct name to ensure proper validation later
					UPDATE_COLUMN_NAME(fileColumnIdx, GetImportFieldHeader((ImportFieldNumber)nChosenFieldID), GetImportFieldDataField((ImportFieldNumber)nChosenFieldID));

					//Set our extended information for the selected field
					if ((AvailableFieldTypes)nFieldType == aftString){
						AddColumnExtendedInfo(fileColumnIdx, GetMaxFieldLength((ImportFieldNumber)nChosenFieldID));
					}

					// (b.savon 2015-07-07 07:25) - PLID 66490 - If the current column is one we've matched, reflect that selection
					// in the available fields list too.
					if (fileColumnIdx == m_nCurrentColumn){
						m_pAvailFields->SetSelByColumn(afcFieldID, nChosenFieldID);
					}

					// (r.goldschmidt 2016-03-15 12:34) - PLID 67974 - account for possibly adding appt purpose column
					if (strFirstRowData.CompareNoCase(GetImportFieldHeader(ifnAppointmentPurpose)) == 0) {
						m_bHasApptPurpose = true;
						GetDlgItem(IDC_APPT_TYPE_TO_NOTES1)->SetWindowText("Prepend Appointment Type and Purpose to Appointment Notes ");
					}
				}
			}		
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - adjust secondary option depending on primary option state
void CImportWizardFieldsDlg::OnBnClickedApptTypesAsConversion1()
{
	try {
		// (r.goldschmidt 2016-03-15 15:10) - PLID 67974 - adding to note is no longer a dependent option
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-02-02 12:44) - PLID 67976 - check if Conversion appointment type is valid
bool CImportWizardFieldsDlg::ValidateApptTypeAsConversion()
{
	try {
		ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
		if (irtCurrent != irtAppointments) {
			return true;
		}
		else 
		{
			if (!IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION1)) {
				return true;
			}
			else { // the conversion option is checked
				   //This convoluted mess of a statement gets the active page index of this window then gets the next page; need to ensure we have a single source for type name
				CImportWizardPreviewDlg  *pPreviewPage = (CImportWizardPreviewDlg*)((CImportWizardDlg*)GetParent())->GetPage(((CImportWizardDlg*)GetParent())->GetActiveIndex() + 1);
				// (r.goldschmidt 2016-03-16 18:02) - PLID 67976 - only use one query
				_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(ID) AS Count FROM AptTypeT WHERE Name = {STRING}", pPreviewPage->GetApptTypeNameConversion());
				if (!prs->eof) {
					long nCount = AdoFldLong(prs, "Count", 0);
					bool bConversionExistsAsType = (nCount > 0);
					bool bConversionExistsAsTypeOnce = (nCount == 1);
					if (bConversionExistsAsType != bConversionExistsAsTypeOnce) { // data is bad; this should never happen because NAME is a unique key for AptTypeT
						m_bCanContinue = FALSE;
						return false;
					}
				}
				return true;
			}
		}

	}NxCatchAll(__FUNCTION__);
	return true;
}
