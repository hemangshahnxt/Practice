// LabRequisitionDlg.cpp : implementation file
//

//TES 11/17/2009 - PLID 36190 - Created.  The vast majority of the code in this file was moved here from LabEntryDlg.

#include "stdafx.h"
#include "PatientsRc.h"
#include "LabRequisitionDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "EditComboBox.h"
#include "MultiSelectDlg.h"
#include "LabFormNumberEditorDlg.h"
#include "LabEditDiagnosisDlg.h"
#include "SignatureDlg.h"
#include "LabEntryDlg.h"
#include "EMRProblemEditDlg.h"
#include "EmrProblemChooserDlg.h"
#include "WellnessDataUtils.h"
#include "LabRequisitionsTabDlg.h"
#include "TodoUtils.h"
#include "LabsToBeOrderedSetupDlg.h"
#include "DecisionRuleUtils.h"
#include "LabCustomFieldsDlg.h"
#include "MedlinePlusUtils.h"
#include "NxAutoQuantum.h"
#include "EMRProblemListDlg.h"

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.
// (c.haag 2010-12-14 9:11) - PLID 41806 - Removed Completed fields


enum ProviderComboColumns{
	pccProviderID = 0,
	pccLastName,
	pccFirstName,
	pccMiddleName,
	pccFullName,
};
enum LocationComboColumns{
	lccLocationID = 0,
	lccName,
};
enum ReceivingLabColumns{
	rlcLabLocationID = 0,
	rlcName,
	rlcCustomReportNumber, //TES 2/2/2010 - PLID 37143
	rlcCustomReportNumber_Results, //TES 7/27/2012 - PLID 51849
};

// (d.lange 2010-12-30 17:38) - PLID 29065 - Biopsy Type
enum BiopsyTypeColumns{
	btcBiopsyTypeID = 0,
	btcDescription,
};

enum AnatomicLocationColumns{
	alcAnatomyID = 0,
	alcDescription,
};
//TES 11/10/2009 - PLID 36128
enum AnatomicLocationQualifierColumns {
	alqcID = 0,
	alqcName,
};

enum MedicalAssistantListColumns {
	malcUserID = 0,
	malcName,
	malcUserName,
};

enum InsuranceListColumns {
	ilcInsuredPartyID = 0,
	ilcOrder,
	ilcInsuranceCo,
	ilcType,
};

enum EToBeOrderedListColumns
{
	tbolcID = 0,
	tbolcDescription,
	tbolcDisplayInstructions,	// (c.haag 2009-05-07 11:06) - PLID 28550
	tbolcLOINCID,				//TES 7/12/2011 - PLID 44538
};

// CLabRequisitionDlg dialog

IMPLEMENT_DYNAMIC(CLabRequisitionDlg, CNxDialog)

CLabRequisitionDlg::CLabRequisitionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabRequisitionDlg::IDD, pParent)
{
	m_nLabID = -1;
	m_nPatientID = -1;
	m_nDefaultLocationID = -1;
	m_nLabProcedureID = -1;
	m_ltType = ltInvalid;
	m_nMedAssistantID = -1;
	m_nInsuredPartyID = -1;
	m_nCurAnatomyID = -1;
	m_nCurAnatomyQualifierID = -1;
	BOOL m_bChangedProvider = FALSE;
	m_bSavedMelanomaHistory = FALSE;
	m_bSavedPreviousBiopsy = FALSE;
	m_nSavedLocationID = -1;
	m_nSavedLabLocationID = -1;
	m_nSavedAnatomyID = -1;
	m_nSavedAnatomyQualifierID = -1;
	m_varSignatureInkData.vt = VT_NULL;
	m_pLabEntryDlg = NULL;
	m_nSavedAnatomySide = -1;
	m_varSignatureTextData.vt = VT_NULL;
	
	m_nInitialAnatomicLocationID = -1;
	m_nInitialAnatomicQualifierID = -1;
	AnatomySide m_asInitialAnatomicSide = asNone;

	m_dtInputDate = COleDateTime::GetCurrentTime();
	m_dtSavedBiopsyDate.SetStatus(COleDateTime::invalid);

	//TES 11/25/2009 - PLID 36193 - Remember our parent (we should only ever be the child of a CLabRequisitionsTabDlg)
	m_pLabRequisitionsTabDlg = (CLabRequisitionsTabDlg*)pParent;

	m_nDefaultLabLocationID = -1;
	m_nLabLocationID = -1;
	m_nCurBiopsyTypeID = -1;
	m_nSavedBiopsyTypeID = -1;

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	m_bSavedCCPatient = FALSE;
	m_bSavedCCRefPhys = FALSE;
	m_bSavedCCPCP = FALSE;

	// (r.gonet 10/14/2011) - PLID 45124
	m_pcfTemplateInstance = NULL;
	m_pFieldsDlg = NULL;
}

CLabRequisitionDlg::~CLabRequisitionDlg()
{
	// (z.manning 2009-05-26 13:00) - PLID 34340 - Clean up problem links
	for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++) {
		CEmrProblemLink *pLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
		if(pLink != NULL) {
			delete pLink;
		}
	}

	// Free up the template instance of the custom fields, if we have one
	if(m_pcfTemplateInstance) {
		delete m_pcfTemplateInstance;
		m_pcfTemplateInstance = NULL;
	}

	// (r.gonet 10/14/2011) - PLID 45124 - Destroy and delete!!
	if(m_pFieldsDlg) {
		m_pFieldsDlg->DestroyWindow();
		delete m_pFieldsDlg;
		m_pFieldsDlg = NULL;
	}
}

void CLabRequisitionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAB_MULTI_PROV, m_nxlProviderLabel);
	DDX_Control(pDX, IDC_CLINICAL_DATA, m_nxeditClinicalData);
	DDX_Control(pDX, IDC_EDIT_INSTRUCTIONS, m_nxeditInstructions);
	DDX_Text(pDX, IDC_EDIT_INSTRUCTIONS, m_strEditInstructions);
	DDV_MaxChars(pDX, m_strEditInstructions, 5000);
	DDX_Control(pDX, IDC_EDIT_ANATOMY, m_EditAnatomyBtn);
	DDX_Control(pDX, IDC_EDIT_TO_BE_ORDERED, m_btnEditToBeOrdered);
	DDX_Control(pDX, IDC_TO_BE_ORDERED_TEXT, m_nxeditToBeOrderedText);
	DDX_Control(pDX, IDC_MELANOMA_HISTORY, m_nxbtnMelanomaHistory);
	DDX_Control(pDX, IDC_PREVIOUS_BIOPSIES, m_nxbtnPreviousBiopsy);
	DDX_Control(pDX, IDC_BTN_ADD_INITIAL_DIAGNOSIS, m_nxbAddInitialDiagnosis);
	DDX_Control(pDX, IDC_EDIT_INITIAL_DIAGNOSIS, m_nxbEditInitialDiagnosis);
	DDX_Control(pDX, IDC_INITIAL_DIAGNOSIS, m_nxeInitialDiagnosis);
	DDX_Control(pDX, IDC_TO_BE_ORDERED_LABEL, m_nxstaticToBeOrderedLabel);
	DDX_Control(pDX, IDC_BIOPSY_DATE, m_dtpBiopsy);
	DDX_Control(pDX, IDC_EDIT_ANATOMIC_QUALIFIERS, m_nxbEditAnatomyQualifiers);
	DDX_Control(pDX, IDC_LAB_ENTRY_PROBLEM_LIST, m_btnProblemList);
	DDX_Control(pDX, IDC_SPECIMEN, m_nxeditSpecimen);
	DDX_Control(pDX, IDC_LEFT_SIDE, m_nxbtnSideLeft);
	DDX_Control(pDX, IDC_RIGHT_SIDE, m_nxbtnSideRight);
	DDX_Control(pDX, IDC_SIGN_REQUISITION, m_signBtn);
	DDX_Control(pDX, IDC_EDIT_SIGNED_BY, m_nxeditLabSignedBy);
	DDX_Control(pDX, IDC_EDIT_SIGNATURE_DATE, m_nxeditLabSignedDate);
	DDX_Control(pDX, IDC_STATIC_SIGNED_BY, m_nxstaticSignedBy);
	DDX_Control(pDX, IDC_STATIC_SIG_DATE, m_nxstaticSignedDate);
	DDX_Control(pDX, IDC_EDIT_BIOPSY_TYPE, m_btnEditBiopsyType);
	DDX_Control(pDX, IDC_BIOPSY_TYPE_LABEL, m_nxstaticBiopsyTypeLabel);
	DDX_Control(pDX, IDC_CC_PATIENT, m_nxbtnCCPatient);
	DDX_Control(pDX, IDC_CC_REF_PHYS, m_nxbtnCCRefPhys);
	DDX_Control(pDX, IDC_CC_PCP, m_nxbtnCCPCP);
	DDX_Control(pDX, IDC_LOINC_CODE, m_nxeditLoincCode);
	DDX_Control(pDX, IDC_LOINC_DESCRIPTION, m_nxeditLoincDescription);
	DDX_Control(pDX, IDC_LOINC_CODE_LABEL, m_nxlLoincLabel);
	DDX_Control(pDX, IDC_LAB_CUSTOM_FIELDS_BUTTON, m_nxbtnAdditionalFields);
	DDX_Control(pDX, IDC_PT_EDUCATION_LABEL, m_nxlabelPatientEducation);
	DDX_Control(pDX, IDC_BTN_PT_EDUCATION, m_btnPatientEducation);
}

BEGIN_MESSAGE_MAP(CLabRequisitionDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_ANATOMY, &CLabRequisitionDlg::OnEditAnatomy)
	ON_BN_CLICKED(IDC_EDIT_TO_BE_ORDERED, &CLabRequisitionDlg::OnEditToBeOrdered)
	ON_BN_CLICKED(IDC_EDIT_ANATOMIC_QUALIFIERS, &CLabRequisitionDlg::OnEditAnatomicQualifiers)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_ADD_INITIAL_DIAGNOSIS, &CLabRequisitionDlg::OnAddInitialDiagnosis)
	ON_BN_CLICKED(IDC_EDIT_INITIAL_DIAGNOSIS, &CLabRequisitionDlg::OnEditInitialDiagnosis)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_SIGN_REQUISITION, OnSignRequisition)
	ON_BN_CLICKED(IDC_LAB_ENTRY_PROBLEM_LIST, OnBnClickedLabEntryProblemList)
	ON_EN_KILLFOCUS(IDC_SPECIMEN, OnKillFocusSpecimen)
	ON_BN_CLICKED(IDC_LEFT_SIDE, OnLeftSide)
	ON_BN_CLICKED(IDC_RIGHT_SIDE, OnRightSide)
	ON_WM_SIZE()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_BIOPSY_DATE, OnDtnDatetimechangeBiopsyDate)
	ON_BN_CLICKED(IDC_EDIT_BIOPSY_TYPE, &CLabRequisitionDlg::OnBnClickedEditBiopsyType)
	ON_BN_CLICKED(IDC_LAB_CUSTOM_FIELDS_BUTTON, &CLabRequisitionDlg::OnBnClickedLabCustomFieldsButton)
	ON_BN_CLICKED(IDC_BTN_PT_EDUCATION, &CLabRequisitionDlg::OnBtnPtEducation)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CLabRequisitionDlg message handlers
BOOL CLabRequisitionDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbAddInitialDiagnosis.AutoSet(NXB_NEW);
		m_btnProblemList.SetIcon(IDI_EMR_PROBLEM_EMPTY); // (z.manning 2009-05-26 10:45) - PLID 34340

		// (z.manning 2009-06-23 16:29) - PLID 34340 - Only allow problems if licensed for EMR
		if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			m_btnProblemList.EnableWindow(FALSE);
			m_btnProblemList.ShowWindow(SW_HIDE);
		}

		// (j.jones 2010-05-27 16:09) - PLID 38863 - you now need special permission to edit this field
		if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic2) {
			m_nxeditClinicalData.SetReadOnly(FALSE);
		}
		else {
			m_nxeditClinicalData.SetReadOnly(TRUE);
		}

		m_nxeditToBeOrderedText.SetLimitText(1000); // (z.manning 2008-10-24 09:50) - PLDI 31807
		//TES 12/3/2009 - PLID 36193 - Limit the Specimen field.
		m_nxeditSpecimen.SetLimitText(50);

		m_pProviderCombo = BindNxDataList2Ctrl(IDC_LAB_PROVIDER);
		m_pLocationCombo = BindNxDataList2Ctrl(IDC_LAB_LOCATION, false);
		//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		m_pLocationCombo->WhereClause = _bstr_t("Active = 1 AND TypeID = 1 AND " + GetAllowedLocationClause("ID"));
		m_pLocationCombo->Requery();
		m_pReceivingLabCombo = BindNxDataList2Ctrl(IDC_RECEIVING_LAB);
		m_pAnatomyCombo = BindNxDataList2Ctrl(IDC_ANATOMIC_LOCATION);

		m_pMedAssistantCombo = BindNxDataList2Ctrl(IDC_LAB_MEDASSISTANT);
		m_pInsuranceCombo = BindNxDataList2Ctrl(IDC_LAB_INSURANCE, false);
		m_pToBeOrderedCombo = BindNxDataList2Ctrl(IDC_LABS_TO_BE_ORDERED, false); // (z.manning 2008-10-24 09:25) - PLID 31807
		//TES 11/10/2009 - PLID 36128
		m_pAnatomyQualifiersCombo = BindNxDataList2Ctrl(IDC_ANATOMIC_QUALIFIER);
		// (d.lange 2010-12-30 16:24) - PLID 29065 - Biopsy Type datalist
		m_pBiopsyTypeCombo = BindNxDataList2Ctrl(IDC_BIOPSY_TYPE);

		// (a.walling 2006-07-24 15:42) - PLID 21571 - No User can be selected for a lab, so let's display it
		IRowSettingsPtr pNoMedAssistantRow = m_pMedAssistantCombo->GetNewRow();
		pNoMedAssistantRow->PutValue(malcUserID, (long)-1);
		pNoMedAssistantRow->PutValue(malcName, _bstr_t("<No User>"));
		m_pMedAssistantCombo->AddRowBefore(pNoMedAssistantRow, m_pMedAssistantCombo->GetFirstRow());

		m_nxlProviderLabel.SetColor(0x00FFB9A8);
		m_nxlProviderLabel.SetText("");
		m_nxlProviderLabel.SetType(dtsHyperlink);

		//TES 7/12/2011 - PLID 44107 - Added LOINC fields
		m_nxlLoincLabel.SetColor(0x00FFB9A8);
		m_nxlLoincLabel.SetType(dtsHyperlink);
		m_nxlLoincLabel.SetText("Order Code (LOINC):");
		m_nxlLoincLabel.AskParentToRedrawWindow();
		m_nxeditLoincCode.SetLimitText(255);
		m_nxeditLoincDescription.SetLimitText(255);
		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Init the preferences to show or hide the patient education related controls.
		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);

		// (j.jones 2013-10-22 12:54) - PLID 58979 - added infobutton abilities
		m_btnPatientEducation.SetIcon(IDI_INFO_ICON);
		m_nxlabelPatientEducation.SetText("Pat. Edu.");
		
		// (r.gonet 2014-01-27 10:21) - PLID 59339 - Have a preference to toggle patient education options off.
		if(!bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education info button
			m_btnPatientEducation.ShowWindow(SW_HIDE);
		}

		if(bShowPatientEducationLink) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Show the patient education link
			m_nxlabelPatientEducation.SetType(dtsHyperlink);
		} else if(!bShowPatientEducationLink && bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We just need the label as a description of the info button.
			m_nxlabelPatientEducation.SetType(dtsText);
		} else if(!bShowPatientEducationLink && !bShowPatientEducationButton) { 
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We don't need the label as either a link or a description label, so hide it.
			m_nxlabelPatientEducation.ShowWindow(SW_HIDE);
		}
		
		//TES 7/16/2010 - PLID 39575
		m_nxtBiopsyTime = BindNxTimeCtrl(this, IDC_BIOPSY_TIME);

	}NxCatchAll("Error in CLabRequisitionDlg::OnInitDialog()");

	return TRUE;
}

//TES 11/17/2009 - PLID 36190 - Call to do any processing that needs to wait until data has loaded.
void CLabRequisitionDlg::PostLoad()
{
	// (r.galicki 2008-10-17 12:15) - PLID 31552 - Configure dialog based on labtype
	//	As of 10-17-2008, there are really only two dialogs, the current one, and the 'lite' version
	//  Following code is in case future variations arise
	NXDATALIST2Lib::IColumnSettingsPtr pCol;

	int nShowToBeOrdered = SW_HIDE;

	// (r.galicki 2008-10-24 16:38) - PLID 31552 - If type has not been set, pull type from data
	/*if(m_ltType == ltInvalid) {
		_RecordsetPtr rs = CreateRecordset("SELECT LabProceduresT.Type FROM LabProceduresT INNER JOIN LabsT ON LabProceduresT.ID = LabsT.LabProcedureID"
			" WHERE LabsT.ID = %li", m_nLabID);
		if(!rs->eof) {
			m_ltType = LabType(AdoFldByte(rs->GetFields(), "Type"));
		}
		else { //display default window type
			m_ltType = ltBiopsy;
		}
	}*/
	switch(m_ltType) {
		case ltLabWork:
		case ltCultures:
		case ltDiagnostics:
			nShowToBeOrdered = SW_SHOW;
			/* Remove Anatomic Location group */
			GetDlgItem(IDC_ANATOMY_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ANATOMY_LABEL)->ShowWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_LOCATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_LOCATION)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_ANATOMY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_ANATOMY)->ShowWindow(FALSE);
			//TES 11/10/2009 - PLID 36128 - Replaced Left/Right with a dropdown
			//TES 12/7/2009 - PLID 36470 - Restored Left/Right
			GetDlgItem(IDC_LEFT_SIDE)->EnableWindow(FALSE);
			GetDlgItem(IDC_LEFT_SIDE)->ShowWindow(FALSE);
			GetDlgItem(IDC_RIGHT_SIDE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RIGHT_SIDE)->ShowWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_QUALIFIER)->EnableWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_QUALIFIER)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_ANATOMIC_QUALIFIERS)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_ANATOMIC_QUALIFIERS)->ShowWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_QUALIFIER_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ANATOMIC_QUALIFIER_LABEL)->ShowWindow(FALSE);
			// (d.lange 2010-12-30 16:48) - PLID 29065 - Hide Biopsy type label
			GetDlgItem(IDC_BIOPSY_TYPE_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_BIOPSY_TYPE_LABEL)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_BIOPSY_TYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_BIOPSY_TYPE)->ShowWindow(FALSE);
			GetDlgItem(IDC_BIOPSY_TYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BIOPSY_TYPE)->ShowWindow(FALSE);
			// (z.manning 2008-11-24 13:53) - PLID 31990 - Hide melanoma history and previous biopsy checks
			// on non biopsy labs
			m_nxbtnMelanomaHistory.ShowWindow(SW_HIDE);
			m_nxbtnPreviousBiopsy.ShowWindow(SW_HIDE);

			// (z.manning 2009-05-19 09:21) - PLID 28554 - Set the default to be ordered text.
			if(m_nLabID == -1) {
				m_nxeditToBeOrderedText.SetWindowText(m_strDefaultToBeOrdered);
			}

			break;
		case ltBiopsy:
		default:
			break;
	}

	// (z.manning 2008-10-23 13:19) - PLID 31807 - Show/hide the to be ordered fields based on
	// the lab type
	GetDlgItem(IDC_LABS_TO_BE_ORDERED)->ShowWindow(nShowToBeOrdered);	
	m_btnEditToBeOrdered.ShowWindow(nShowToBeOrdered);
	m_nxstaticToBeOrderedLabel.ShowWindow(nShowToBeOrdered);
	m_nxeditToBeOrderedText.ShowWindow(nShowToBeOrdered);
}

void CLabRequisitionDlg::LoadNew()
{
	// (j.jones 2010-05-13 08:59) - PLID 38564 - ensure the "saved" fields are properly initialized
	m_bSavedMelanomaHistory = FALSE;
	m_bSavedPreviousBiopsy = FALSE;
	m_nSavedLocationID = -1;
	m_nSavedLabLocationID = -1;
	m_nSavedAnatomyID = -1;
	m_nSavedAnatomyQualifierID = -1;
	m_nSavedInputBy = -1;
	m_dtSavedInputDate.SetStatus(COleDateTime::invalid);
	m_varSignatureInkData.vt = VT_NULL;
	m_nSavedAnatomySide = -1;
	m_varSignatureTextData.vt = VT_NULL;
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	m_strSavedSpecimen = m_strSavedClinicalData = m_strSavedInstructions = m_strSavedToBeOrdered = m_strProvStartString = m_strSavedInitialDiagnosis = m_strSavedLoincCode = m_strSavedLoincDescription = "";
	m_dtSavedBiopsyDate.SetStatus(COleDateTime::invalid);
	// (c.haag 2010-11-23 10:50) - PLID 41590 - New signature fields
	m_nSignedByUserID = -1;
	m_nSavedSignedByUserID = -1;
	m_dtSigned.SetStatus(COleDateTime::invalid);
	m_dtSavedSigned.SetStatus(COleDateTime::invalid);
	m_signBtn.AutoSet(NXB_MODIFY);
	m_nSavedBiopsyTypeID = -1;		// (d.lange 2011-01-03 10:54) - PLID 29065 - Added Biopsy type
	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	long nDefaultLocationID = m_nDefaultLocationID;
	if(nDefaultLocationID == -1) {
		nDefaultLocationID = GetCurrentLocationID();
	}
	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	m_bSavedCCPatient = FALSE;
	m_bSavedCCRefPhys = FALSE;
	m_bSavedCCPCP = FALSE;

	// (r.gonet 07/22/2013) - PLID 45187 - Now, based on the preference for where the provider comes from,
	//  choose which set of providers to associate with this appointment.
	EDefaultLabProvider edlpDefaultProvider = (EDefaultLabProvider)GetRemotePropertyInt("DefaultLabProvider", edlpGeneral1Provider, 0, "<None>", true);
	// (r.gonet 07/22/2013) - PLID 45187 - Eliminate the cost of finding the most recent appointment if we don't have to.
	bool bGetMostRecentAppointmentResources = false;
	if(edlpDefaultProvider == edlpAppointmentResource && m_dwDefaultProviderIDs.GetSize() == 0) {
		bGetMostRecentAppointmentResources = true;
	}
	// (r.gonet 07/22/2013) - PLID 45187 - Get the maximum number of days to search for an appointment in the past.
	long nDefaultLabProvider_ApptMaxDays = GetRemotePropertyInt("DefaultLabProvider_ApptMaxDays", 1, 0, "<None>", true);
	// (z.manning 2008-11-24 16:04) - PLID 31990 - Parameterized
	// (r.gonet 07/22/2013) - PLID 45187 - Added a look up for the most recent appointment the patient has.
	_RecordsetPtr rs = CreateParamRecordset(
		"DECLARE @PatientID INT SET @PatientID = {INT}; \r\n"
		"DECLARE @DefaultLabProvider_ApptMaxDays INT SET @DefaultLabProvider_ApptMaxDays = {INT}; \r\n"
		"DECLARE @GetMostRecentAppointmentResources BIT SET @GetMostRecentAppointmentResources = {BIT}; \r\n"
		"SELECT PatientsT.MainPhysician FROM PatientsT \r\n"
		" INNER JOIN PersonT ON PatientsT.MainPhysician = PersonT.ID \r\n"
		" WHERE PersonT.Archived = 0 AND PatientsT.PersonID = @PatientID;\r\n"
		"\r\n"
		// (r.gonet 07/22/2013) - PLID 45187 - Retrieve the providers from the most recent appointment
		"IF @GetMostRecentAppointmentResources = 1 \r\n"
		"BEGIN \r\n"
			"SELECT MostRecentAppointmentQ.AppointmentID, ResourceProviderLinkT.ProviderID \r\n"
			"FROM ResourceProviderLinkT \r\n"
			"INNER JOIN AppointmentResourceT ON ResourceProviderLinkT.ResourceID = AppointmentResourceT.ResourceID \r\n"
			"INNER JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID \r\n"
			"INNER JOIN \r\n"
			"( \r\n"
				"SELECT TOP 1 AppointmentsT.ID AS AppointmentID \r\n"
				"FROM AppointmentsT \r\n"
				"WHERE AppointmentsT.PatientID = @PatientID \r\n"
				"AND dbo.AsDateNoTime(AppointmentsT.StartTime) >= DATEADD(D, -1 * @DefaultLabProvider_ApptMaxDays, DATEDIFF(D, 0, GETDATE())) \r\n"
				"AND AppointmentsT.StartTime <= GETDATE() \r\n"
				"AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 \r\n"
				"ORDER BY StartTime DESC \r\n"
			") MostRecentAppointmentQ ON AppointmentResourceT.AppointmentID = MostRecentAppointmentQ.AppointmentID \r\n"
			"WHERE ResourceProviderLinkT.ProviderID Is Not Null AND PersonT.Archived = 0 \r\n"
		"END \r\n"
		"\r\n"
		"SELECT Type from LabProceduresT where ID = {INT} \r\n"
		// (z.manning 2008-11-24 16:00) - PLID 31990 - Added a separate query to pull the values
		// for previous biopsy and melanoma history so we can "remember" them.
		"SELECT TOP 1 MelanomaHistory, PreviousBiopsy FROM LabsT \r\n"
		"WHERE PatientID = @PatientID AND Type = {INT} AND Deleted = 0 \r\n"
		"ORDER BY InputDate DESC \r\n"
		// (z.manning 2010-01-11 12:35) - PLID 24044 - Load the default receiving lab
		"SELECT DefaultLabID FROM LocationsT WHERE ID = {INT}; \r\n"
		, m_nPatientID, nDefaultLabProvider_ApptMaxDays, bGetMostRecentAppointmentResources, m_nLabProcedureID, ltBiopsy, nDefaultLocationID);

	// (j.jones 2010-05-06 09:42) - PLID 38520 - added InputDate
	m_dtInputDate = COleDateTime::GetCurrentTime();

	//Biopsy date
	_variant_t varBiopsy;
	varBiopsy = m_varCurDate;
	if(varBiopsy.vt != VT_NULL) {
		m_dtpBiopsy.SetValue(varBiopsy);
		//TES 7/16/2010 - PLID 39575 - We don't want to default to the current time, only the current date.
		m_nxtBiopsyTime->Clear();
	}
	
	//TES 9/29/2010 - PLID 40644 - We may have been given default providers.
	if(m_dwDefaultProviderIDs.GetSize()) {
		m_dwProviders.Copy(m_dwDefaultProviderIDs);
		m_strProviderNames = m_strDefaultProviderNames;
		if(m_dwProviders.GetSize() == 1) {
			IRowSettingsPtr pRow = m_pProviderCombo->SetSelByColumn(pccProviderID, (long)m_dwProviders[0]);
			if(!pRow) {
				//TES 9/29/2010 - PLID 40644 - Must be inactive
				m_pProviderCombo->PutComboBoxText(_bstr_t(m_strProviderNames));
			}
		}
		else {
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_SHOW);
			m_nxlProviderLabel.SetText(m_strProviderNames);
		}
	}
	else {
		// (r.gonet 07/22/2013) - PLID 45187 - A temporary list of providers that we are going to try to
		// associate with this lab (if they are not inactive).
		CDWordArray dwProviders;

		// (r.gonet 07/22/2013) - PLID 45187 - Get the main physician
		_variant_t varMainPhys = g_cvarNull;
		if(!rs->eof) {
			varMainPhys = rs->Fields->Item["MainPhysician"]->Value;
		}

		// (r.gonet 07/22/2013) - PLID 45187 - Get the most recent appointment resource provider(s).
		// (r.gonet 07/24/2013) - PLID 45187 - We didn't query for the appointment resource if we didn't need it
		CDWordArray dwAppointmentProviders;
		if(bGetMostRecentAppointmentResources) {
			rs = rs->NextRecordset(NULL);

			while(!rs->eof) {
				dwAppointmentProviders.Add(VarLong(rs->Fields->Item["ProviderID"]->Value));
				rs->MoveNext();
			}
		}

		// (r.gonet 07/22/2013) - PLID 45187 - If we fail with the appointment or EMN provider though, then we default to the General 1 provider. 
		bool bFallBackToGeneral1Provider = false;
		if(edlpDefaultProvider == edlpNone) {
			// (r.gonet 07/22/2013) - PLID 45187 - No default provider. Leave the field blank.
		} else if(edlpDefaultProvider == edlpAppointmentResource) {
			// (r.gonet 07/22/2013) - PLID 45187 - Try to get the most recent appointment
			if(dwAppointmentProviders.GetSize() > 0) {
				// (r.gonet 07/22/2013) - PLID 45187 - Use the appointment resource providers as the lab providers.
				dwProviders.Copy(dwAppointmentProviders);
			} else {
				// (r.gonet 07/22/2013) - PLID 45187 - We didn't find any recent appointment, so fall back on the Gen 1 provider.
				bFallBackToGeneral1Provider = true;
			}
		} else if(edlpDefaultProvider == edlpEMNPrimaryProvider) {
			if(m_dwEMNProviderIDs.GetSize() > 0) {
				// (r.gonet 07/22/2013) - PLID 57683 - Use the EMN's primary providers as the lab providers.
				dwProviders.Copy(m_dwEMNProviderIDs);
			} else {
				// (r.gonet 07/22/2013) - PLID 57683 - This lab is not associated with an EMN or it has no primary provider(s).
				//  fall back on the Gen 1 provider.
				bFallBackToGeneral1Provider = true;
			}
		}

		// (r.gonet 07/22/2013) - PLID 45187 - We may set the general 1 provider as the lab provider if they want us to do that or if we failed with other methods.
		if(varMainPhys.vt != VT_NULL && (edlpDefaultProvider == edlpGeneral1Provider || bFallBackToGeneral1Provider == true)) {
			//Default Provider (from gen1)
			dwProviders.Add(VarLong(varMainPhys));
		}

		// (r.gonet 07/22/2013) - PLID 45187 - Clear out any providers already assigned to this lab.
		m_dwProviders.RemoveAll();
		m_strProviderNames = "";

		// (r.gonet 07/22/2013) - PLID 45187 - Go through all the providers we collected
		// and try and assign them to the lab. 
		for(int i = 0; i < dwProviders.GetSize(); i++) {
			long nProviderID = dwProviders.GetAt(i);
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderCombo->SetSelByColumn(pccProviderID, nProviderID);
			if(pRow){
				//set the current selecttion in the array
				m_dwProviders.Add(nProviderID);
				//TES 9/29/2010 - PLID 40644 - Need to track the name
				// (r.gonet 07/22/2013) - PLID 45187 - Append the proivder names, since we can now have a default of multiple.
				m_strProviderNames += VarString(pRow->GetValue(pccFullName)) + ", ";
			}
			else {
				// (j.jones 2007-09-12 10:06) - PLID 27358 - don't assert, their main physician could be inactive
				//ASSERT(FALSE);
			}
		}

		// (r.gonet 07/22/2013) - PLID 45187 - Eliminate any extra commas.
		if(!m_strProviderNames.IsEmpty()) {
			m_strProviderNames = m_strProviderNames.Left(m_strProviderNames.GetLength() - 2);
		}

		// (r.gonet 07/22/2013) - PLID 45187 - Set the interface controls since we show different
		// controls for if we have no/one vs multiple providers.
		if(m_dwProviders.GetSize() <= 1) {
			SetDlgItemText(IDC_LAB_MULTI_PROV, "");
			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_HIDE);
			m_nxlProviderLabel.SetText(m_strProviderNames);
		}
	}

	//Default Location (from currently logged into)
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	if(m_pLocationCombo->TrySetSelByColumn_Deprecated(lccLocationID, nDefaultLocationID) == sriNoRowYet_WillFireEvent){
		//The combo box is still loading. Let the user know what's going on.
			m_pLocationCombo->PutComboBoxText("Defaulting to current location...");
	}


	//MedAssistant will default to current user for now
	m_nMedAssistantID = GetCurrentUserID();
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_pMedAssistantCombo->TrySetSelByColumn_Deprecated(malcUserID, _variant_t(m_nMedAssistantID));

	// (a.walling 2006-07-25 10:21) - PLID 21575 - Load the insurance combo, select nothing by default (no insurance)
	CString strWherePatient;
	strWherePatient.Format("RespTypeID <> -1 AND PatientID = %li", m_nPatientID);
	m_pInsuranceCombo->PutWhereClause(_bstr_t(strWherePatient));
	m_pInsuranceCombo->Requery();

	// (a.walling 2006-07-25 11:56) - PLID 21575 - No Insurance item
	IRowSettingsPtr pNoInsuranceRow = m_pInsuranceCombo->GetNewRow();
	pNoInsuranceRow->PutValue(ilcInsuredPartyID, (long)-1);
	pNoInsuranceRow->PutValue(ilcInsuranceCo, _bstr_t("<No Insurance>"));
	m_pInsuranceCombo->AddRowBefore(pNoInsuranceRow, m_pInsuranceCombo->GetFirstRow());
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	if (GetRemotePropertyInt("LabChoosePrimary", 1, 0, "<None>", true) == 1) {
		m_pInsuranceCombo->TrySetSelByColumn_Deprecated(ilcType, _bstr_t("Primary"));
	}
	else {
		m_pInsuranceCombo->TrySetSelByColumn_Deprecated(ilcInsuredPartyID, _variant_t((long)-1));
	}

	m_nInsuredPartyID = -1;

	// (r.galicki 2008-10-28 17:45) - PLID 31555 - Load type from data
	rs = rs->NextRecordset(NULL);
	m_ltType = LabType(AdoFldByte(rs->GetFields(), "Type", ltBiopsy));

	// (z.manning 2008-11-24 16:05) - PLID 31990 - Another query to load previous values of
	// previous biopsy and melanoma history.
	rs = rs->NextRecordset(NULL);
	if(!rs->eof) {
		m_bSavedMelanomaHistory = AdoFldBool(rs->GetFields(), "MelanomaHistory");
		m_nxbtnMelanomaHistory.SetCheck(m_bSavedMelanomaHistory ? BST_CHECKED : BST_UNCHECKED);
		m_bSavedPreviousBiopsy = AdoFldBool(rs->GetFields(), "PreviousBiopsy");
		m_nxbtnPreviousBiopsy.SetCheck(m_bSavedPreviousBiopsy ? BST_CHECKED : BST_UNCHECKED);
	}

	rs = rs->NextRecordset(NULL);
	// (z.manning 2010-01-11 12:38) - PLID 24044 - Default receiving lab
	if(!rs->eof) {
		_variant_t varDefaultLabID;
		//TES 9/29/2010 - PLID 40644 - We may have been given a default
		if(m_nDefaultLabLocationID == -1) {
			varDefaultLabID = rs->GetFields()->GetItem("DefaultLabID")->GetValue();
		}
		else {
			varDefaultLabID = m_nDefaultLabLocationID;
		}
		if(varDefaultLabID.vt == VT_I4) {
			m_pReceivingLabCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			m_pReceivingLabCombo->SetSelByColumn(rlcLabLocationID, VarLong(varDefaultLabID));
			//TES 2/1/2010 - PLID 37143 - Our parent needs to know which custom request form to use, based on this new location.
			IRowSettingsPtr pRow = m_pReceivingLabCombo->CurSel;
			if(pRow) {
				m_nLabLocationID = VarLong(pRow->GetValue(rlcLabLocationID),-1);
				m_pLabEntryDlg->SetRequestForm(VarLong(pRow->GetValue(rlcCustomReportNumber),-2));
				//TES 7/27/2012 - PLID 51849 - Also set the Results Form
				m_pLabEntryDlg->SetResultsForm(VarLong(pRow->GetValue(rlcCustomReportNumber_Results),-2));
				m_pLabEntryDlg->SetHL7Link(m_nLabLocationID);// (a.vengrofski 2010-05-12 17:24) - PLID <38547> - Update the HL7 Link box too
			}
			else {
				//TES 10/4/2010 - PLID 40644 - The default Lab may be inactive, if so, load its information.
				if(VarLong(varDefaultLabID) > 0) {
					m_nLabLocationID = VarLong(varDefaultLabID);
					_RecordsetPtr rsLabLocationInfo = CreateParamRecordset("SELECT LocationsT.Name, "
						"CustomReportQ.ReportNumber AS DefaultReport, CustomResultReportQ.ReportNumber AS DefaultResultsForm "
						"FROM LocationsT LEFT JOIN (SELECT * FROM LocationCustomReportsT WHERE ReportID = 658) "
						"AS CustomReportQ ON LocationsT.ID = CustomReportQ.LocationID "
						"LEFT JOIN (SELECT * FROM LocationCustomReportsT WHERE ReportID = 567) "
						"AS CustomResultReportQ ON LocationsT.ID = CustomResultReportQ.LocationID "
						"WHERE LocationsT.ID = {INT}", m_nLabLocationID);
					m_pReceivingLabCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLabLocationInfo, "Name")));
					m_pLabEntryDlg->SetRequestForm(AdoFldLong(rsLabLocationInfo, "DefaultReport", -2));
					//TES 7/27/2012 - PLID 51849 - Also set the Results Form
					m_pLabEntryDlg->SetResultsForm(AdoFldLong(rsLabLocationInfo, "DefaultResultsForm", -2));
					m_pLabEntryDlg->SetHL7Link(m_nLabLocationID);
				}
				else {
					m_pLabEntryDlg->SetRequestForm(-2);
					//TES 7/27/2012 - PLID 51849 - Also set the Results Form
					m_pLabEntryDlg->SetResultsForm(-2);
					m_pLabEntryDlg->SetHL7Link(-1);// (a.vengrofski 2010-05-12 17:24) - PLID <38547> - Update the HL7 Link box too
				}
			}

			
		}
	}

	// (j.jones 2010-06-25 14:10) - PLID 39185 - filter the 'To Be Ordered' list by location,
	// this means the list will be empty unless a location is selected
	CString strToBeOrderedWhere;
	strToBeOrderedWhere.Format("LocationID = %li", m_nLabLocationID);
	m_pToBeOrderedCombo->PutWhereClause(_bstr_t(strToBeOrderedWhere));
	m_pToBeOrderedCombo->Requery();

	//TES 2/15/2010 - PLID 37375 - We may have pre-loaded values for the Anatomic Location information.  If so, load that information 
	// (code is copied from LoadExisting()).
	if(m_nInitialAnatomicLocationID != -1) {
		m_nSavedAnatomyID = m_nInitialAnatomicLocationID;
		//TES 11/6/2009 - PLID 36189 - Remember our AnatomyID
		m_nCurAnatomyID = m_nSavedAnatomyID;
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nAnatomyTrySetResult = m_pAnatomyCombo->TrySetSelByColumn_Deprecated(alcAnatomyID, m_nInitialAnatomicLocationID);
		// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
		// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
		// query to get the name and put it in the combo text.
		if((nAnatomyTrySetResult == sriNoRowYet_WillFireEvent || nAnatomyTrySetResult == sriNoRow) && m_nCurAnatomyID != -1) {
			//if we ever add inactivity for anatomic locations, uncomment this section and check for correctness
			//TES 11/6/2009 - PLID 36189 - OK!
			_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Description FROM LabAnatomyT WHERE ID = {INT}", m_nCurAnatomyID);
			if(!rsAnat->eof) {
				m_pAnatomyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Description", "")));
			}
			else {
				//This should not happen. What now?
				ASSERT(FALSE);
				m_pAnatomyCombo->PutCurSel(m_pAnatomyCombo->GetFirstRow());
			}				
		}
		/*else if(nAnatomyTrySetResult == sriNoRowYet_WillFireEvent) {
			//It hasn't loaded yet so we will let the user know.
			m_pAnatomyCombo->PutComboBoxText("Loading anatomic location...");
		}*/
	}

	//Anatomic side
	//TES 12/7/2009 - PLID 36470 - AnatomySide is back!
	m_nSavedAnatomySide = m_asInitialAnatomicSide;
	m_nxbtnSideLeft.SetCheck((m_nSavedAnatomySide == asLeft)?BST_CHECKED:BST_UNCHECKED);
	m_nxbtnSideRight.SetCheck((m_nSavedAnatomySide == asRight)?BST_CHECKED:BST_UNCHECKED);


	if(m_nInitialAnatomicQualifierID != -1) {
		//TES 11/10/2009 - PLID 36128 - This is now the AnatomyQualifierID
		m_nSavedAnatomyQualifierID = m_nInitialAnatomicQualifierID;
		m_nCurAnatomyQualifierID = m_nSavedAnatomyQualifierID;
		long nAnatomyTrySetResult = m_pAnatomyQualifiersCombo->TrySetSelByColumn_Deprecated(alqcID, m_nInitialAnatomicQualifierID);
		// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
		// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
		// query to get the name and put it in the combo text.
		if((nAnatomyTrySetResult == sriNoRowYet_WillFireEvent || nAnatomyTrySetResult == sriNoRow) && m_nCurAnatomyQualifierID != -1) {
			//TES 11/10/2009 - PLID 36128 - It must be inactive
			_RecordsetPtr rsQual = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", m_nCurAnatomyQualifierID);
			if(!rsQual->eof) {
				m_pAnatomyQualifiersCombo->PutComboBoxText(_bstr_t(AdoFldString(rsQual, "Name", "")));
			}
			else {
				//Wha???
				ThrowNxException("Invalid Anatomic Location Qualifier %li found!", m_nCurAnatomyQualifierID);
			}
		}
		/*else if(nAnatomyTrySetResult == sriNoRowYet_WillFireEvent) {
			m_pAnatomyQualifiersCombo->PutComboBoxText("Loading location qualifier...");
		}*/
	}

	// (z.manning 2010-05-05 10:52) - PLID 37190 - We may have a default value for the intial diagnosis
	SetDlgItemText(IDC_INITIAL_DIAGNOSIS, m_strDefaultInitialDiagnosis);

	//TES 9/29/2010 - PLID 40644 - Likewise for the Comments field.
	SetDlgItemText(IDC_CLINICAL_DATA, m_strDefaultComments);

	// (z.manning 2010-06-02 13:34) - PLID 38976 - Auto-generate the specimen
	char chSpecimen = m_pLabRequisitionsTabDlg->GetNextSpecimenCode();
	if(chSpecimen != (char)0) {
		SetDlgItemText(IDC_SPECIMEN, CString(chSpecimen));
		m_pLabRequisitionsTabDlg->HandleSpecimenChange(this, CString(chSpecimen));
	}

	// (r.gonet 09/21/2011) - PLID 45124 - Load the custom fields
	m_pcfTemplateInstance = NULL;
	CLabProcCFTemplatePtr pcfTemplate = make_shared<CLabProcCFTemplate>();
	try {
		if(pcfTemplate->LoadByLabProcedureID(m_nLabProcedureID)) {
			if(m_pcfTemplateInstance != NULL) {
				// (r.gonet 12/09/2011) - PLID 45124 - LoadNew was called twice? I don't see how to produce this but unless we delete now, we are going to have
				//  a memory leak. Assert for a developer to look at this and make sure that we are now handling custom fields correctly.
				ASSERT(FALSE);
				delete m_pcfTemplateInstance;
				m_pcfTemplateInstance = NULL;
			}
			m_pcfTemplateInstance = new CCFTemplateInstance(pcfTemplate);
		}
		if(m_pcfTemplateInstance && m_pcfTemplateInstance->GetFieldInstanceCount() == 0) {
			// (r.gonet 09/21/2011) - PLID 45124 - We don't have a custom fields template for this procedure, so disable the button.
			delete m_pcfTemplateInstance;
			m_pcfTemplateInstance = NULL;
		}

	} NxCatchAll("CLabRequisitionDlg::LoadNew : Could not load custom fields template.");

	if(m_pcfTemplateInstance == NULL) {
		m_nxbtnAdditionalFields.EnableWindow(FALSE);
		m_nxbtnAdditionalFields.SetTextColor(RGB(0, 0, 0));
	} else {
		// (r.gonet 09/21/2011) - PLID 45124 - Success! We have fields!
		m_nxbtnAdditionalFields.EnableWindow(TRUE);
		m_nxbtnAdditionalFields.SetTextColor(RGB(255, 0, 0));
	}
}

void CLabRequisitionDlg::LoadExisting()
{
	// (j.jones 2007-07-19 15:50) - PLID 26751 - added DiagnosisDesc and ClinicalDiagnosisDesc
	//(e.lally 2007-08-08) PLID 26978 - Labs store the patient name in the record and must pull from those fields
	// (j.gruber 2008-09-18 15:40) - PLID 31332 - take out result fields
	// (z.manning 2008-10-16 10:32) - PLID 21082 - Added signature fields
	// (z.manning 2008-10-24 10:22) - PLID 31807 - Added to be ordered
	// (r.galicki 2008-10-28 15:01) - PLID 31552 - Added lab procedure type
	// (z.manning 2008-10-30 12:47) - PLID 31613 - Load the lab procedure ID
	// (z.manning 2008-11-24 13:47) - PLID 31990 - Added melanoma history and previous biopsy
	// (c.haag 2009-05-11 14:09) - PLID 28515 - Added instructions
	//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
	//TES 6/23/2009 - PLID 34698 - Fixed bad join that caused the Input user to display as the Completed user.
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/7/2009 - PLID 36470 - Restored AnatomySide
	//TES 2/2/2010 - PLID 37143 - Added ReportNumber, the number of the custom report that is the default for this lab's Receiving Lab
	// (j.jones 2010-04-12 17:53) - PLID 38166 - added SignatureTextData
	// (j.jones 2010-05-06 09:42) - PLID 38520 - added InputDate
	// (a.vengrofski 2010-05-27 09:45) - PLID <38547> - Added DefaultLabID
	// (c.haag 2010-11-23 10:50) - PLID 41590 - Added SignedByUsername, SignedBy, SignedDate, removed CompletedByUserName
	// (c.haag 2010-12-13 12:10) - PLID 41806 - Removed Completed fields
	// (d.lange 2011-01-03 10:40) - PLID 29065 - Added Biopsy Type
	//TES 6/22/2011 - PLID 44261 - Bizarrely, this was joining Hl7SettingsT on LabsT.LocationID = HL7SettingsT.ID, and then using 
	// the DefaultLabID in HL7SettingsT to set the receiving lab.  Not only is the join clearly ridiculous, but if we're loading
	// an existing requisition, we don't want to change the receiving lab!
	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	//TES 7/27/2012 - PLID 51849 - Added ReportNumber_Results, the number of the custom Results Form that is the default for this lab's Receiving Lab
	_RecordsetPtr rs = CreateParamRecordset("SELECT LabsT.Specimen, LabsT.BiopsyDate, LabsT.InputDate, "
		"LabsT.ClinicalData, LabsT.Instructions, "
		"LabsT.LocationID, LabsT.AnatomyID, LabsT.AnatomySide, LabsT.AnatomyQualifierID, "
		"LabsT.LabLocationID, InsuredPartyID, ToBeOrdered, "
		"PreviousBiopsy, MelanomaHistory, LabsT.MedAssistant AS MedAssistant, LabsT.Specimen, "
		"LabsT.InitialDiagnosis, LabsT.SignatureImageFile, LabsT.SignatureInkData, LabsT.SignatureTextData, "
		"LabsT.SignedBy, LabsT.SignedDate, LabsT.BiopsyTypeID, "
		"CustomRequestFormsQ.ReportNumber, "
		"SignedByUser.UserName AS SignedByUsername, "
		"LabsT.CC_Patient, LabsT.CC_RefPhys, LabsT.CC_PCP, "
		"LabsT.LOINC_Code, LabsT.LOINC_Description, "
		"CustomResultFormsQ.ReportNumber AS ReportNumber_Results "
		""
		"FROM LabsT "
		"LEFT JOIN UsersT AS SignedByUser ON LabsT.SignedBy = SignedByUser.PersonID "
		"LEFT JOIN (SELECT * FROM LocationCustomReportsT WHERE ReportID = 658) AS CustomRequestFormsQ "
		"ON LabsT.LabLocationID = CustomRequestFormsQ.LocationID "
		"LEFT JOIN (SELECT * FROM LocationCustomReportsT WHERE ReportID = 567) AS CustomResultFormsQ "
		"ON LabsT.LabLocationID = CustomResultFormsQ.LocationID "
		""
		"WHERE LabsT.ID = {INT} ", m_nLabID);
	if(!rs->eof){
		_variant_t varBiopsy, varReceived;
		
		// (z.manning 2008-11-24 13:48) - PLID 31990 - Previous biopsy and melanoa history checkboxes
		m_bSavedMelanomaHistory = AdoFldBool(rs->GetFields(), "MelanomaHistory");
		m_nxbtnMelanomaHistory.SetCheck(m_bSavedMelanomaHistory ? BST_CHECKED : BST_UNCHECKED);
		m_bSavedPreviousBiopsy = AdoFldBool(rs->GetFields(), "PreviousBiopsy");
		m_nxbtnPreviousBiopsy.SetCheck(m_bSavedPreviousBiopsy ? BST_CHECKED : BST_UNCHECKED);

		// (a.walling 2006-07-25 09:58) - PLID 21575 - We have the patient ID so load the insurance combo
		CString strWherePatient;
		strWherePatient.Format("RespTypeID <> -1 AND PatientID = %li", m_nPatientID);
		m_pInsuranceCombo->PutWhereClause(_bstr_t(strWherePatient));
		m_pInsuranceCombo->Requery();

		// (a.walling 2006-07-25 11:56) - PLID 21575 - No Insurance item
		IRowSettingsPtr pNoInsuranceRow = m_pInsuranceCombo->GetNewRow();
		pNoInsuranceRow->PutValue(ilcInsuredPartyID, (long)-1);
		pNoInsuranceRow->PutValue(ilcInsuranceCo, _bstr_t("<No Insurance>"));
		m_pInsuranceCombo->AddRowBefore(pNoInsuranceRow, m_pInsuranceCombo->GetFirstRow());

		//We want all dates to default to an invalid date
		COleDateTime dtDefault = COleDateTime(0.00);
		dtDefault.SetStatus(COleDateTime::invalid);

		//TES 11/25/2009 - PLID 36193 - The Specimen identifier is now its own, editable field.
		//Specimen
		m_strSavedSpecimen = AdoFldString(rs, "Specimen", "");
		SetDlgItemText(IDC_SPECIMEN, m_strSavedSpecimen);
		m_pLabRequisitionsTabDlg->HandleSpecimenChange(this, m_strSavedSpecimen);

		// (j.jones 2010-05-06 09:42) - PLID 38520 - added InputDate
		m_dtInputDate = AdoFldDateTime(rs, "InputDate");
					
		//Biopsy Date
		varBiopsy = rs->Fields->Item["BiopsyDate"]->Value;
		m_dtSavedBiopsyDate = VarDateTime(varBiopsy, dtDefault);
		if(varBiopsy.vt != VT_NULL){
			m_dtpBiopsy.SetValue(varBiopsy);
			//TES 7/16/2010 - PLID 39575 - Also set the time.
			COleDateTime dtBiopsyTime = VarDateTime(varBiopsy);
			dtBiopsyTime.SetTime(dtBiopsyTime.GetHour(), dtBiopsyTime.GetMinute(), dtBiopsyTime.GetSecond());
			if(dtBiopsyTime.GetHour() != 0 || dtBiopsyTime.GetMinute() != 0 || dtBiopsyTime.GetSecond() != 0) {
				m_nxtBiopsyTime->SetDateTime(dtBiopsyTime);
			}
			else {
				m_nxtBiopsyTime->Clear();
			}
		}
		else{
			//Default the biopsy date to today
			m_dtpBiopsy.SetValue(m_varCurDate);
			//but we want it unchecked
			m_dtpBiopsy.SetValue(COleVariant());
			//TES 7/16/2010 - PLID 39575 - We don't want to default to the current time, only the current date.
			m_nxtBiopsyTime->Clear();
			//TES 7/16/2010 - PLID 39575 - Also, since the date box is unchecked, disable the time box.
			m_nxtBiopsyTime->Enabled = g_cvarFalse;
		}

		// (a.walling 2006-07-24 15:48) - PLID 21571 - Medical Assistant
		//Medical Assistant
		m_nMedAssistantID = AdoFldLong(rs, "MedAssistant", -1); // NULL is no medassistant
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nMedAsstTrySetResult = m_pMedAssistantCombo->TrySetSelByColumn_Deprecated(malcUserID, m_nMedAssistantID);
		if (nMedAsstTrySetResult == sriNoRow) {
			if (m_nMedAssistantID != -1) {
				// user may have become inactive
				_RecordsetPtr rsMedAsst = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First AS Name FROM PersonT WHERE ID = %li", m_nMedAssistantID);
				if (!rsMedAsst->eof) {
					m_pMedAssistantCombo->PutComboBoxText(_bstr_t(AdoFldString(rsMedAsst, "Name", "")));
				}
				else {
					// this should not happen!
					ASSERT(FALSE);
					m_pMedAssistantCombo->PutCurSel(NULL); // same as sriNoRow, clear selection
				}
			}
			else {
				// this should not happen! there should always be the -1, <No User> row.
				ASSERT(FALSE);
				m_pMedAssistantCombo->PutCurSel(NULL); // same as sriNoRow, clear selection
			}
		}
		else if (nMedAsstTrySetResult == sriNoRowYet_WillFireEvent) {
			m_pMedAssistantCombo->PutComboBoxText("Loading Assistant...");
		}

		// (a.walling 2006-07-25 09:54) - PLID 21575 - Insurance info
		//Insured Party ID
		m_nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID", -1); // NULL is no insurance
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nInsuredPartyTrySetResult = m_pInsuranceCombo->TrySetSelByColumn_Deprecated(ilcInsuredPartyID, m_nInsuredPartyID);
		if (nInsuredPartyTrySetResult == sriNoRow) {
			if (m_nInsuredPartyID > 0) {
				//not found in the query?
				_RecordsetPtr rsParty = CreateRecordset("SELECT InsuranceCoT.Name AS InsuranceCoName FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID WHERE InsuredPartyT.PersonID = %li", m_nInsuredPartyID);
				if (!rsParty->eof) {
					m_pInsuranceCombo->PutComboBoxText(_bstr_t(AdoFldString(rsParty, "InsuranceCoName", "")));
				}
				else {
					ASSERT(FALSE);
					m_pInsuranceCombo->PutCurSel(NULL);
				}
			}
			else {
				ASSERT(FALSE);
				m_pInsuranceCombo->PutCurSel(NULL); // select nothing; they have no insurance selected
			}
		}
		else if (nInsuredPartyTrySetResult == sriNoRowYet_WillFireEvent) {
			m_pInsuranceCombo->PutComboBoxText("Loading Insurance...");
		}

		//Clinical Data
		m_strSavedClinicalData = AdoFldString(rs, "ClinicalData","");
		SetDlgItemText (IDC_CLINICAL_DATA, m_strSavedClinicalData);

		// (c.haag 2009-05-11 13:53) - PLID 28515 - Instructions
		m_strSavedInstructions = AdoFldString(rs, "Instructions","");
		SetDlgItemText(IDC_EDIT_INSTRUCTIONS, m_strSavedInstructions);

		// (z.manning 2008-10-24 10:22) - PLID 31807 - to be ordered
		m_strSavedToBeOrdered = AdoFldString(rs->GetFields(), "ToBeOrdered", "");
		m_nxeditToBeOrderedText.SetWindowText(m_strSavedToBeOrdered);

		// (z.manning 2008-10-16 10:37) - PLID 21082 - Signature fields
		m_strSignatureFileName = AdoFldString(rs->GetFields(), "SignatureImageFile", "");
		m_varSignatureInkData = rs->GetFields()->GetItem("SignatureInkData")->Value;
		// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
		m_varSignatureTextData = rs->GetFields()->GetItem("SignatureTextData")->Value;
		// (c.haag 2010-11-23 10:50) - PLID 41590 - New signature fields
		m_dtSigned = AdoFldDateTime(rs, "SignedDate", dtDefault);
		m_dtSavedSigned = m_dtSigned;
		if(m_dtSigned.GetStatus() != COleDateTime::invalid)
			SetDlgItemText(IDC_EDIT_SIGNATURE_DATE, FormatDateTimeForInterface(m_dtSigned));
		m_nSignedByUserID = AdoFldLong(rs, "SignedBy", -1);
		m_nSavedSignedByUserID = m_nSignedByUserID;
		CString strSignedUser = AdoFldString(rs, "SignedByUsername", "");
		if (m_nSignedByUserID > 0) {
			SetDlgItemText(IDC_EDIT_SIGNED_BY, strSignedUser);
			// (c.haag 2010-11-23 10:50) - PLID 41590 - Update the button text and make it green
			// indiciating it's done
			GetDlgItem(IDC_SIGN_REQUISITION)->SetWindowText("View Signature");
			m_signBtn.SetTextColor(RGB(0,0,0));
		} else {
			m_signBtn.AutoSet(NXB_MODIFY);
		}

		//Provider
		//PLID 21568 - Multi Provider
		_RecordsetPtr rsProvs = CreateRecordset("SELECT ProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name "
			" FROM LabMultiProviderT LEFT JOIN PersonT ON LabMultiProviderT.ProviderID = PersonT.ID WHERE LabID = %li ORDER BY Last, First, Middle ASC ", m_nLabID);
		long nCount = 0;
		//TES 9/29/2010 - PLID 40644 - We need to remember the provider names
		m_strProviderNames = "";
		CDWordArray dwTemp;
		while (! rsProvs->eof) {

			m_dwProviders.Add(AdoFldLong(rsProvs, "ProviderID"));
			dwTemp.Add(AdoFldLong(rsProvs, "ProviderID"));
			
			m_strProviderNames += AdoFldString(rsProvs, "Name") + ", ";
			nCount++;
			rsProvs->MoveNext();
		}
		rsProvs->Close();

		SortDWordArray(dwTemp);
		m_strProvStartString = GetIDStringFromArray(&dwTemp);
		m_strProviderNames = m_strProviderNames.Left(m_strProviderNames.GetLength() - 2);

		if (nCount > 1) {

			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_HIDE);
			
			m_nxlProviderLabel.SetText(m_strProviderNames);
		}
		else if (nCount == 1) {

			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_SHOW);

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderCombo->SetSelByColumn(pccProviderID,(long)m_dwProviders.GetAt(0));
			if (pRow) {
			}
			else {
				//they have an inactive provider
				m_pProviderCombo->PutComboBoxText(_bstr_t(m_strProviderNames));
				
			}
			
		}
		else {

			//I'm told this should never occur....
			//ASSERT(FALSE);
			m_pProviderCombo->PutCurSel(NULL);
			GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_SHOW);
		}

		//TES 9/29/2010 - PLID 40644 - Copy to our "Last Saved" variables.
		m_dwSavedProviderIDs.Copy(m_dwProviders);
		m_strSavedProviderNames = m_strProviderNames;

		//Practice location
		_variant_t var = rs->Fields->Item["LocationID"]->Value;
		m_nSavedLocationID = VarLong(var, -1);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nLocTrySetResult = m_pLocationCombo->TrySetSelByColumn_Deprecated(lccLocationID, var);
		if(nLocTrySetResult == sriNoRow) {
			//they may have an inactive location
			_RecordsetPtr rsLoc = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LocationID FROM LabsT WHERE ID = %li)", m_nLabID);
			if(!rsLoc->eof) {
				m_pLocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else{
				//This should not happen. What now?
				ASSERT(FALSE);
				m_pLocationCombo->PutCurSel(m_pLocationCombo->GetFirstRow());
			}
		}
		else if(nLocTrySetResult == sriNoRowYet_WillFireEvent) {
			//It hasn't loaded yet so we will let the user know.
			m_pLocationCombo->PutComboBoxText("Loading location...");
		}

		//Receiving lab location
		var = rs->Fields->Item["LabLocationID"]->Value;
		m_nSavedLabLocationID = VarLong(var, -1);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nRecLabTrySetResult = m_pReceivingLabCombo->TrySetSelByColumn_Deprecated(rlcLabLocationID, var);
		if(nRecLabTrySetResult == sriNoRow) {
			//they may have an inactive receiving lab
			_RecordsetPtr rsRecLab = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LabLocationID FROM LabsT WHERE ID = %li)", m_nLabID);
			if(!rsRecLab->eof) {
				m_pReceivingLabCombo->PutComboBoxText(_bstr_t(AdoFldString(rsRecLab, "Name", "")));
			}
			else{
				//This should not happen. What now?
				ASSERT(FALSE);
				m_pReceivingLabCombo->PutCurSel(m_pReceivingLabCombo->GetFirstRow());
			}
		}
		else if(nRecLabTrySetResult == sriNoRowYet_WillFireEvent){
			//It hasn't loaded yet so we will let the user know.
			m_pReceivingLabCombo->PutComboBoxText("Loading receiving lab...");
		}

		// (j.jones 2010-06-25 14:10) - PLID 39185 - filter the 'To Be Ordered' list by location,
		// this means the list will be empty unless a location is selected
		CString strToBeOrderedWhere;
		strToBeOrderedWhere.Format("LocationID = %li", m_nSavedLabLocationID);
		m_pToBeOrderedCombo->PutWhereClause(_bstr_t(strToBeOrderedWhere));
		m_pToBeOrderedCombo->Requery();

		//TES 2/1/2010 - PLID 37143 - Our parent needs to know which request form to use.
		m_pLabEntryDlg->SetRequestForm(AdoFldLong(rs, "ReportNumber", -2));
		//TES 7/27/2012 - PLID 51849 - They also need the results form
		m_pLabEntryDlg->SetResultsForm(AdoFldLong(rs, "ReportNumber_Results", -2));
		//TES 6/22/2011 - PLID 44261 - Use the lab location ID we just loaded, not one pulled from a random HL7SettingsT record.
		m_pLabEntryDlg->SetHL7Link(m_nSavedLabLocationID);// (a.vengrofski 2010-05-27 09:58) - PLID <38547> - Update the parent.

		// (d.lange 2011-01-03 10:43) - PLID 29065 - Added Biopsy Type
		var = rs->Fields->Item["BiopsyTypeID"]->Value;
		m_nSavedBiopsyTypeID = VarLong(var, -1);
		m_nCurBiopsyTypeID = m_nSavedBiopsyTypeID;

		long nBiopsyTypeTrySetResult = m_pBiopsyTypeCombo->TrySetSelByColumn_Deprecated(btcBiopsyTypeID, var);
		if((nBiopsyTypeTrySetResult == sriNoRowYet_WillFireEvent || nBiopsyTypeTrySetResult == sriNoRow) && m_nCurBiopsyTypeID != -1) {
			_RecordsetPtr rsBiopsyType = CreateParamRecordset("SELECT Description FROM LabBiopsyTypeT WHERE ID = {INT}", m_nCurBiopsyTypeID);
			if(!rsBiopsyType->eof) {
				m_pBiopsyTypeCombo->PutComboBoxText(_bstr_t(AdoFldString(rsBiopsyType, "Description", "")));
			}else {
				ASSERT(FALSE);
				m_pBiopsyTypeCombo->PutCurSel(m_pBiopsyTypeCombo->GetFirstRow());
			}
		}

		//Anatomic location
		var = rs->Fields->Item["AnatomyID"]->Value;
		m_nSavedAnatomyID = VarLong(var, -1);
		//TES 11/6/2009 - PLID 36189 - Remember our AnatomyID
		m_nCurAnatomyID = m_nSavedAnatomyID;
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nAnatomyTrySetResult = m_pAnatomyCombo->TrySetSelByColumn_Deprecated(alcAnatomyID, var);
		// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
		// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
		// query to get the name and put it in the combo text.
		if((nAnatomyTrySetResult == sriNoRowYet_WillFireEvent || nAnatomyTrySetResult == sriNoRow) && m_nCurAnatomyID != -1) {
			//if we ever add inactivity for anatomic locations, uncomment this section and check for correctness
			//TES 11/6/2009 - PLID 36189 - OK!
			_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Description FROM LabAnatomyT WHERE ID = {INT}", m_nCurAnatomyID);
			if(!rsAnat->eof) {
				m_pAnatomyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Description", "")));
			}
			else {
				//This should not happen. What now?
				ASSERT(FALSE);
				m_pAnatomyCombo->PutCurSel(m_pAnatomyCombo->GetFirstRow());
			}				
		}
		/*else if(nAnatomyTrySetResult == sriNoRowYet_WillFireEvent) {
			//It hasn't loaded yet so we will let the user know.
			m_pAnatomyCombo->PutComboBoxText("Loading anatomic location...");
		}*/

		//Anatomic side
		//TES 12/7/2009 - PLID 36470 - AnatomySide is back!
		m_nSavedAnatomySide = AdoFldLong(rs, "AnatomySide", 0);
		m_nxbtnSideLeft.SetCheck((m_nSavedAnatomySide == asLeft)?BST_CHECKED:BST_UNCHECKED);
		m_nxbtnSideRight.SetCheck((m_nSavedAnatomySide == asRight)?BST_CHECKED:BST_UNCHECKED);


		//TES 11/10/2009 - PLID 36128 - This is now the AnatomyQualifierID
		var = rs->Fields->Item["AnatomyQualifierID"]->Value;
		m_nSavedAnatomyQualifierID = VarLong(var, -1);
		m_nCurAnatomyQualifierID = m_nSavedAnatomyQualifierID;
		nAnatomyTrySetResult = m_pAnatomyQualifiersCombo->TrySetSelByColumn_Deprecated(alqcID, var);
		// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
		// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
		// query to get the name and put it in the combo text.
		if((nAnatomyTrySetResult == sriNoRowYet_WillFireEvent || nAnatomyTrySetResult == sriNoRow) && m_nCurAnatomyQualifierID != -1) {
			//TES 11/10/2009 - PLID 36128 - It must be inactive
			_RecordsetPtr rsQual = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", m_nCurAnatomyQualifierID);
			if(!rsQual->eof) {
				m_pAnatomyQualifiersCombo->PutComboBoxText(_bstr_t(AdoFldString(rsQual, "Name", "")));
			}
			else {
				//Wha???
				ThrowNxException("Invalid Anatomic Location Qualifier %li found!", m_nCurAnatomyQualifierID);
			}
		}
		/*else if(nAnatomyTrySetResult == sriNoRowYet_WillFireEvent) {
			m_pAnatomyQualifiersCombo->PutComboBoxText("Loading location qualifier...");
		}*/
			

		//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
		m_strSavedInitialDiagnosis = AdoFldString(rs, "InitialDiagnosis","");
		SetDlgItemText(IDC_INITIAL_DIAGNOSIS, m_strSavedInitialDiagnosis);

		//TES 7/11/2011 - PLID 36445 - Added CC Fields
		m_bSavedCCPatient = AdoFldBool(rs, "CC_Patient");
		m_nxbtnCCPatient.SetCheck(m_bSavedCCPatient?BST_CHECKED:BST_UNCHECKED);
		m_bSavedCCRefPhys = AdoFldBool(rs, "CC_RefPhys");
		m_nxbtnCCRefPhys.SetCheck(m_bSavedCCRefPhys?BST_CHECKED:BST_UNCHECKED);
		m_bSavedCCPCP = AdoFldBool(rs, "CC_PCP");
		m_nxbtnCCPCP.SetCheck(m_bSavedCCPCP?BST_CHECKED:BST_UNCHECKED);

		//TES 7/12/2011 - PLID 44107 - Added LOINC fields
		m_strSavedLoincCode = AdoFldString(rs, "LOINC_Code");
		SetDlgItemText(IDC_LOINC_CODE, m_strSavedLoincCode);
		m_strSavedLoincDescription = AdoFldString(rs, "LOINC_Description");
		SetDlgItemText(IDC_LOINC_DESCRIPTION, m_strSavedLoincDescription);

	}
	rs->Close();

	// (z.manning 2009-05-26 16:06) - PLID 34340 - Load problems
	LoadExistingProblems();

	if(m_pcfTemplateInstance != NULL) {
		// (r.gonet 12/09/2011) - PLID 45124 - LoadExisting was called twice? I don't see how to produce this but unless we delete now, we are going to have
		//  a memory leak. Assert for a developer to look at this to ensure we are now handling custom fields correctly.
		ASSERT(FALSE);
		delete m_pcfTemplateInstance;
		m_pcfTemplateInstance = NULL;
	}

	// (r.gonet 09/21/2011) - PLID 45124 - Load the custom fields for this lab.
	m_pcfTemplateInstance = new CCFTemplateInstance();
	try {
		if(!m_pcfTemplateInstance->LoadByLabID(m_nLabID) || m_pcfTemplateInstance->GetFieldInstanceCount() == 0) {
			// (r.gonet 09/21/2011) - PLID 45124 - We don't have a custom fields template for this procedure, so disable the button.
			delete m_pcfTemplateInstance;
			m_pcfTemplateInstance = NULL;
		}

	} NxCatchAll("CLabRequisitionDlg::LoadExisting : Could not load custom fields template.");

	if(m_pcfTemplateInstance == NULL) {
		m_nxbtnAdditionalFields.EnableWindow(FALSE);
		m_nxbtnAdditionalFields.SetTextColor(RGB(0, 0, 0));
	} else {
		// (r.gonet 09/21/2011) - PLID 45124 - Success! We have fields!
		m_nxbtnAdditionalFields.EnableWindow(TRUE);
		m_nxbtnAdditionalFields.SetTextColor(RGB(255, 0, 0));
	}
}

//TES 11/25/2009 - PLID 36193 - The SetNew() function is no longer needed, instead our parent will just add a new CLabRequisitionDlg

//TES 11/25/2009 - PLID 36193 - We are now responsible for saving ourselves, given some shared information (i.e., patient name, form number)
//TES 12/1/2009 - PLID 36452 - Added an output parameter for the new ID.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CLabRequisitionDlg::SaveNew(IN long &nAuditTransID, IN const SharedLabInformation &sli, OUT long &nNewLabID, IN OUT CDWordArray &arNewCDSInterventions)
{
	//TES 11/25/2009 - PLID 36193 - The Specimen identifier is now its own editable field
	//Specimen
	CString strSpecimen;
	GetDlgItemText(IDC_SPECIMEN, strSpecimen);
	if(!strSpecimen.IsEmpty()) {
		strSpecimen = "'" + _Q(strSpecimen) + "'";
	}
	else {
		strSpecimen = "NULL";
	}

	CString strBiopsyDate = "NULL";
	_variant_t varBiopsy = m_dtpBiopsy.GetValue();
	if(varBiopsy.vt != VT_NULL){
		COleDateTime dtBiopsyDate = VarDateTime(varBiopsy);
		//TES 7/16/2010 - PLID 39575 - Also pull the time.
		if(m_nxtBiopsyTime->GetStatus() == 1) {
			COleDateTime dtBiopsyTime = m_nxtBiopsyTime->GetDateTime();
			dtBiopsyDate.SetDateTime(dtBiopsyDate.GetYear(), dtBiopsyDate.GetMonth(), dtBiopsyDate.GetDay(), dtBiopsyTime.GetHour(), dtBiopsyTime.GetMinute(), dtBiopsyTime.GetSecond());
		}
		else {
			dtBiopsyDate.SetDateTime(dtBiopsyDate.GetYear(), dtBiopsyDate.GetMonth(), dtBiopsyDate.GetDay(), 0, 0, 0);
		}
		//TES 7/16/2010 - PLID 39575 - varBiopsy is used down below by the auditing, so update it as well.
		varBiopsy = _variant_t(dtBiopsyDate, VT_DATE);
		strBiopsyDate = "'" + _Q(FormatDateTimeForSql(dtBiopsyDate)) + "'";
	}

	CString strAnatomyID = "NULL";
	long nAnatomyID = -1;
	CString strAnatomicLocation;
	IRowSettingsPtr pAnatomyRow = m_pAnatomyCombo->CurSel;
	if(pAnatomyRow) {
		nAnatomyID = VarLong(pAnatomyRow->GetValue(alcAnatomyID));
		strAnatomicLocation = VarString(m_pAnatomyCombo->GetCurSel()->GetValue(alcDescription));
		if(nAnatomyID != -1) {
			strAnatomyID.Format("%li", nAnatomyID);
		}
	}

	// (d.lange 2010-12-30 17:36) - PLID 29065 - Biopsy Type
	CString strBiopsyTypeID = "NULL";
	long nBiopsyTypeID = -1;
	CString strBiopsyType;
	IRowSettingsPtr pBiopsyTypeRow = m_pBiopsyTypeCombo->CurSel;
	if(pBiopsyTypeRow) {
		nBiopsyTypeID = VarLong(pBiopsyTypeRow->GetValue(btcBiopsyTypeID));
		strBiopsyType = VarString(pBiopsyTypeRow->GetValue(btcDescription));
		if(nBiopsyTypeID != -1) {
			strBiopsyTypeID.Format("%li", nBiopsyTypeID);
		}
	}

	//TES 12/7/2009 - PLID 36470 - AnatomySide
	long nAnatomySide = asNone;
	if(IsDlgButtonChecked(IDC_LEFT_SIDE))
		nAnatomySide = asLeft;
	else if(IsDlgButtonChecked(IDC_RIGHT_SIDE))
		nAnatomySide = asRight;

	CString strAnatomyQualifierID = "NULL";
	long nAnatomyQualifierID = -1;
	CString strAnatomicLocationQualifier;
	IRowSettingsPtr pQual = m_pAnatomyQualifiersCombo->CurSel;
	if(pQual) {
		nAnatomyQualifierID = VarLong(pQual->GetValue(alqcID),-1);
		strAnatomicLocationQualifier = VarString(pQual->GetValue(alqcName),"");
	}
	else {
		if(m_pAnatomyQualifiersCombo->IsComboBoxTextInUse) {
			nAnatomyQualifierID = m_nCurAnatomyQualifierID;
			strAnatomicLocationQualifier = CString((LPCTSTR)m_pAnatomyQualifiersCombo->ComboBoxText);
		}
	}
	if(nAnatomyQualifierID != -1) {
		strAnatomyQualifierID.Format("%li", nAnatomyQualifierID);
	}

	CString strMedAssistant = "NULL";
	long nMedAssistantID = -1;
	IRowSettingsPtr pMedAssistant = m_pMedAssistantCombo->GetCurSel();
	if (pMedAssistant) {
		nMedAssistantID = VarLong(pMedAssistant->GetValue(malcUserID), -1);
	}
	if(nMedAssistantID != -1) {
		strMedAssistant.Format("%li", nMedAssistantID);
	}

	CString strInsParty = "NULL";
	long nInsuredPartyID = -1;
	IRowSettingsPtr pParty = m_pInsuranceCombo->GetCurSel();
	if (pParty) {
		nInsuredPartyID = VarLong(pParty->GetValue(ilcInsuredPartyID), -1);
	}
	if(nInsuredPartyID != -1) {
		strInsParty.Format("%li", nInsuredPartyID);
	}

	// (z.manning 2008-10-16 13:31) - PLID 21082 - Signature ink data
	CString strSigInkData;
	if(m_varSignatureInkData.vt == VT_NULL || m_varSignatureInkData.vt == VT_EMPTY) {
		strSigInkData = "NULL";
	}
	else {
		strSigInkData = CreateByteStringFromSafeArrayVariant(m_varSignatureInkData);
	}

	// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
	CString strSigTextData;
	if(m_varSignatureTextData.vt == VT_NULL || m_varSignatureTextData.vt == VT_EMPTY) {
		strSigTextData = "NULL";
	}
	else {
		strSigTextData = CreateByteStringFromSafeArrayVariant(m_varSignatureTextData);
	}

	// (c.haag 2010-11-23 10:50) - PLID 41590 - Signature date
	CString strSignedDate = "NULL";
	if(m_dtSigned.GetStatus() == COleDateTime::valid && m_dtSigned > COleDateTime(0.00)) {
		strSignedDate = "'" + FormatDateTimeForSql(m_dtSigned) + "'";
	}
	// (c.haag 2010-11-23 10:50) - PLID 41590 - Signed by
	CString strSignedBy = "NULL";
	if(m_nSignedByUserID > 0){
		strSignedBy.Format("%li", m_nSignedByUserID);
	}

	CString strClinicalData;
	GetDlgItemText(IDC_CLINICAL_DATA, strClinicalData);

	CString strInstructions;
	GetDlgItemText(IDC_EDIT_INSTRUCTIONS, strInstructions);

	CString strToBeOrdered;
	m_nxeditToBeOrderedText.GetWindowText(strToBeOrdered);

	long nLocationID =-1;
	if(m_pLocationCombo->GetIsComboBoxTextInUse() != FALSE){
		//The location combo is still loading, so we will just get the current location that it is defaulting to.
		nLocationID = GetCurrentLocationID();
	}
	else {
		nLocationID = VarLong(m_pLocationCombo->CurSel->GetValue(lccLocationID));
	}

	BOOL bMelanomaHistory = m_nxbtnMelanomaHistory.GetCheck() == BST_CHECKED;
	BOOL bPreviousBiopsy = m_nxbtnPreviousBiopsy.GetCheck() == BST_CHECKED;

	CString strInitialDiagnosis;
	GetDlgItemText(IDC_INITIAL_DIAGNOSIS, strInitialDiagnosis);

	// (z.manning 2010-05-13 14:19) - PLID 37405 - If we having any pending to-dos for this newly created lab,
	// we now need to set the regarding ID.
	CString strTodoSql;
	if(m_arynPendingTodoIDs.GetSize() > 0) {
		strTodoSql.Format("UPDATE TodoList SET RegardingID = @nNewLabID WHERE TaskID IN (%s) \r\n"
			, ArrayAsString(m_arynPendingTodoIDs));
	}

	//TES 10/4/2010 - PLID 40644 - The lab might be inactive, and thus not have a row selected, so use m_nLabLocationID which will
	// have been set in LoadNew().
	IRowSettingsPtr pLabRow = m_pReceivingLabCombo->CurSel;
	long nLabLocationID = m_nLabLocationID;
	if(pLabRow) {
		nLabLocationID = VarLong(pLabRow->GetValue(rlcLabLocationID));
	}

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	BOOL bCCPatient = m_nxbtnCCPatient.GetCheck() == BST_CHECKED;
	BOOL bCCRefPhys = m_nxbtnCCRefPhys.GetCheck() == BST_CHECKED;
	BOOL bCCPCP = m_nxbtnCCPCP.GetCheck() == BST_CHECKED;

	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	CString strLoincCode, strLoincDescription;
	GetDlgItemText(IDC_LOINC_CODE, strLoincCode);
	GetDlgItemText(IDC_LOINC_DESCRIPTION, strLoincDescription);

	// (r.gonet 09/21/2011) - PLID 45124 - Save the custom fields and the values the user has entered into the database.
	CString strTemplateInstanceID = "NULL";
	if(m_pcfTemplateInstance) {
		m_pcfTemplateInstance->Save();
		strTemplateInstanceID = FormatString("%li", m_pcfTemplateInstance->GetID());
	}

	// (z.manning 2008-10-24 10:31) - PLID 31807 - Added to be ordered
	// (r.galicki 2008-10-28 15:26) - PLID 31552 - Added Lab type
	// (z.manning 2008-11-24 13:57) - PLID 31990 - Added previous biopsy and melanoma history
	// (z.manning 2009-02-27 10:12) - PLID 33141 - Added SourceDataGroupID
	// (c.haag 2009-05-11 14:09) - PLID 28515 - Added instructions
	//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/7/2009 - PLID 36470 - Restored AnatomySide
	// (z.manning 2010-02-26 16:51) - PLID 37540 - SourceDetailImageStampID
	// (j.jones 2010-04-12 17:31) - PLID 38166 - added SignatureTextData
	// (c.haag 2010-11-23 10:50) - PLID 41590 - Added Signed Date, Signed By
	// (c.haag 2010-12-13 12:10) - PLID 41806 - Removed Completed fields
	// (d.lange 2010-12-30 17:44) - PLID 29065 - Added Biopsy Type
	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	// (r.gonet 09/21/2011) - PLID 45124 - This lab shall now point to custom fields template instance.
	_RecordsetPtr prsID = CreateRecordset(
		"SET NOCOUNT ON \r\n "
		"DECLARE @nNewLabID int \r\n"
		"INSERT INTO LabsT (FormNumberTextID, Specimen, PatientID, "
		"LocationID, BiopsyDate, "
		"AnatomyID, AnatomySide, AnatomyQualifierID, "
		"InputDate, InputBy, "
		"LabLocationID, ClinicalData, Instructions, "
		"Deleted, MedAssistant, InsuredPartyID, "
		"PatientFirst, PatientMiddle, PatientLast, SourceActionID, SourceDetailID, "
		"PicID, LabProcedureID, SignatureImageFile, SignatureInkData, SignatureTextData, "
		"ToBeOrdered, Type, "
		"MelanomaHistory, PreviousBiopsy, SourceDataGroupID, OrderSetID, InitialDiagnosis, "
		"SourceDetailImageStampID, SignedDate, SignedBy, BiopsyTypeID, "
		"CC_Patient, CC_RefPhys, CC_PCP, "
		"LOINC_Code, LOINC_Description, CFTemplateInstanceID) "
		"VALUES ('%s', %s, %li, "
		"%li, %s, "
		"%s, %li, %s, "
		"GetDate(), %li, "
		"%li, '%s', "
		"'%s', "
		"0, %s, %s, "
		"'%s', '%s', '%s', %s, %s, "
		"%s, %li, '%s', %s, %s, "
		"'%s', %li, "
		"%i, %i, %s, %s, '%s', "
		"%s, %s, %s, %s, "
		"%i, %i, %i, "
		"'%s', '%s', %s) \r\n"
		"SET @nNewLabID = (SELECT Convert(int, SCOPE_IDENTITY())) \r\n"
		"%s "
		"SET NOCOUNT OFF \r\n"
		"SELECT @nNewLabID AS NewNum",
		_Q(sli.strFormTextID), strSpecimen, m_nPatientID,
		nLocationID, strBiopsyDate, 
		strAnatomyID, nAnatomySide, strAnatomyQualifierID,
		GetCurrentUserID(),
		nLabLocationID, _Q(strClinicalData), 
		_Q(strInstructions),
		strMedAssistant, strInsParty,
		_Q(sli.strPatientFirst), _Q(sli.strPatientMiddle), _Q(sli.strPatientLast),
		sli.strSourceActionID, sli.strSourceDetailID,
		sli.strPicID, m_nLabProcedureID, _Q(m_strSignatureFileName), strSigInkData, strSigTextData,
		_Q(strToBeOrdered), m_ltType,
		bMelanomaHistory ? 1 : 0, bPreviousBiopsy ? 1 : 0, sli.strSourceDataGroupID, sli.strOrderSetID,
		_Q(strInitialDiagnosis), sli.strSourceDetailImageStampID, strSignedDate, strSignedBy, strBiopsyTypeID,
		bCCPatient ? 1 : 0, bCCRefPhys ? 1 : 0, bCCPCP ? 1 : 0,
		_Q(strLoincCode), _Q(strLoincDescription), strTemplateInstanceID
		, strTodoSql);

	if (NULL == prsID) {
		//If this is NULL, something bad happened
		AfxThrowNxException("The lab record could not be made. Please try saving your changes again or restart NexTech Practice if the problem persists.");
	}
	//TES 11/25/2009 - PLID 36193 - We're now saved, remember our ID
	m_nLabID = AdoFldLong(prsID, "NewNum");
	//TES 12/1/2009 - PLID 36452 - Output the new ID
	nNewLabID = m_nLabID;

	//PLID 21568: MultiProviders
	//I'm not sure why this isn't using the SqlBatch functionality, but I'll carry on what is happening here
	CString strSql = "SET NOCOUNT ON\r\n";
	CString strTmpProvInsert;
	int nCount = m_dwProviders.GetSize();
	for (int i = 0; i < nCount; i++) {
		strTmpProvInsert.Format("INSERT INTO LabMultiProviderT (LabID, ProviderID) "
			" VALUES (%li, %li)\r\n", m_nLabID, m_dwProviders.GetAt(i));
		strSql += strTmpProvInsert;
	}

	//Save the new step data

	// (m.hancock 2006-07-06 15:05) - PLID 21187 - Added a check to make sure inactive steps are not copied over
	// (m.hancock 2006-07-10 09:37) - PLID 21187 - We added StepOrder and Name to LabStepsT so we need to also
	// copy over the steps information.
	// (e.lally 2006-07-11 10:00) - PLID 21074 - If the Lab is marked completed, we need to mark all the steps 
	  //as completed too.
	// (z.manning 2008-10-09 12:11) - PLID 31633 - LabProcedureID is now in LabsT (instead of LabStepsT)
	// (c.haag 2010-12-15 16:58) - PLID 41825 - Lab requisitions themselves can no longer be completed; use
	// functions to get the completed status from the results
	CString strCompletedBy = "NULL";
	CString strCompletedDate = "NULL";
	// (c.haag 2011-01-27) - PLID 41825 - This behavior has again changed. We no longer relate lab completions to step completions
	// in any way.
	/*BOOL bLabCompleted;
	long nLabCompletedBy;
	COleDateTime dtLabCompletedDate;
	if ((bLabCompleted = m_pLabEntryDlg->IsLabCompleted(-1, nLabCompletedBy, dtLabCompletedDate)))
	{
		strCompletedBy = AsString(nLabCompletedBy);
		strCompletedDate = CString("'") + FormatDateTimeForSql(dtLabCompletedDate) + "'";
	}*/

	CString strTempInsert;
	strTempInsert.Format("INSERT INTO LabStepsT (LabProcedureStepID, LabID, "
		" StepCompletedDate, StepCompletedBy, StepOrder, Name) "
		" SELECT StepID, %li, %s, %s, StepOrder, LabProcedureStepsT.Name "
		" FROM LabProcedureStepsT "
		" LEFT JOIN LabProceduresT ON LabProcedureStepsT.LabProcedureID = LabProceduresT.ID "
		" WHERE LabProceduresT.ID = %li AND LabProcedureStepsT.Inactive = 0; "
		,m_nLabID, strCompletedDate, strCompletedBy
		,m_nLabProcedureID);
	strSql += strTempInsert;
	strSql += FormatString("\r\n"
		" SET NOCOUNT OFF \r\n"
		" SELECT TOP 1 LabStepsT.StepID, CreateLadder "
		" FROM LabStepsT "
		" LEFT JOIN LabProcedureStepsT ON LabStepsT.LabProcedureStepID = LabProcedureStepsT.StepID "
		" WHERE LabStepsT.LabID  = %li "
		" ORDER BY LabStepsT.StepOrder ASC "
		, m_nLabID);
	//JMM - this really should be a STD though
	_RecordsetPtr prsSteps = CreateRecordsetStd(strSql);

	BOOL bCreateLadder = FALSE;
	long nFirstStepID = -1;
	if(!prsSteps->eof) {
		bCreateLadder = AdoFldBool(prsSteps->GetFields(), "CreateLadder", FALSE);
		nFirstStepID = AdoFldLong(prsSteps->GetFields(), "StepID");
	}

	// (z.manning 2008-10-06 12:06) - PLID 21094 - Keep track of any labs created in this dialog
	EMNLab lab;
	lab.nID = m_nLabID;
	// (z.manning 2008-10-09 12:57) - PLID 31628 - Now track lab procedure ID too.
	lab.nLabProcedureID = m_nLabProcedureID;
	// (z.manning 2008-10-07 11:25) - PLID 31561 - Pass in data to display on an EMR is applicable.
	lab.strAnatomicLocation = strAnatomicLocation;

	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	lab.eAnatomicSide = (AnatomySide)nAnatomySide;
	lab.strAnatomicLocationQualifier = strAnatomicLocationQualifier;
	lab.strClinicalData = strClinicalData;
	lab.strToBeOrdered = strToBeOrdered; // (z.manning 2008-10-24 10:36) - PLID 31807
	lab.eType = m_ltType;
	m_pLabEntryDlg->AddNewEmnLab(lab);

	/******************************************************
	// Auditing
	*******************************************************/
	CString strAuditNewValue;
	CString strAuditPatientName = GetExistingPatientName(m_nPatientID);
	// (z.manning 2008-10-31 12:53) - PLID 31864 - Use to be ordered instead of anatomic location for
	// non-biopsy labs.
	strAuditNewValue.Format("New Lab Created - %s - %s"
		, sli.strFormTextID, m_ltType == ltBiopsy ? lab.strAnatomicLocation : lab.strToBeOrdered);
	AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabNew, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);

	//(e.lally 2007-08-01) PLID 26890 - Audit all entered data on initial save
	//Patient Name
	if(!sli.strPatientFirst.IsEmpty())
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabFirst, m_nLabID, "", sli.strPatientFirst, aepMedium, aetCreated);
	if(!sli.strPatientMiddle.IsEmpty())
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabMiddle, m_nLabID, "", sli.strPatientMiddle, aepMedium, aetCreated);
	if(!sli.strPatientLast.IsEmpty())
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabLast, m_nLabID, "", sli.strPatientLast, aepMedium, aetCreated);

	//TES 11/17/2009 - PLID 36190 - Tell our Requisition tab to do any post-save activities (mainly auditing).
	//m_dlgRequisitions.PostSaveNew(nAuditTransID);
	

	if(bCreateLadder)
	{
		// (z.manning 2008-10-21 10:33) - PLID 31371 - The first step in this lab procedure is set
		// to create a ladder, so prompt them to do so.
		if(PhaseTracking::PromptToAddPtnProcedures(m_nPatientID)) {
			// (z.manning 2008-12-01 16:02) - PLID 32286 - Make sure we don't prompt them to sign
			// a lab that's already been signed. It's also not necessary to mark this step complete
			// since the entire lab must be complete aready if it's signed.
			// (c.haag 2010-12-15 17:19) - PLID 41825 - Do not check the requisition signature; check
			// for any unsigned results
			// (c.haag 2011-01-27) - PLID 41825 - This behavior has again changed. We no longer relate lab completions to step completions
			/*if(m_pLabEntryDlg->LabHasUnsignedResults(-1)) {
				// (z.manning 2008-10-21 10:34) - PLID 31371 - If they created a ladder then we need
				// to mark the first step complete.
				//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
				_RecordsetPtr prsStepComplete = CreateParamRecordset(
					"SET NOCOUNT ON \r\n"
					"UPDATE LabStepsT SET StepCompletedDate = GetDate(), StepCompletedBy = {INT} \r\n"
					"WHERE StepID = {INT} \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT COUNT(*) AS IncompleteStepCount FROM LabStepsT \r\n"
					"WHERE LabID = {INT} AND StepCompletedDate IS NULL \r\n"
					, GetCurrentUserID(), nFirstStepID, m_nLabID);
				CString strOld = GenerateStepCompletionAuditOld(nFirstStepID);
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiPatientLabStepMarkedComplete, nFirstStepID, strOld, "<Completed>", aepMedium, aetChanged);
				if(AdoFldLong(prsStepComplete->GetFields(), "IncompleteStepCount") == 0) {
					// (z.manning 2008-10-21 10:34) - PLID 31371 - In the unlikely event that the first
					// step that creates a ladder is the only step in this lab, let's go ahead and mark
					// the lab as complete to be consistent with other places that mark steps complete.
					// (z.manning 2008-10-31 10:33) - PLID 21082 - There's a global function to mark labs
					// completed now including getting the now required signature.
					PromptToSignAndMarkLabComplete(this, m_nLabID);
				}
			}*/
			//TES 1/5/2011 - PLID 37877 - Pass in the patient ID
			SyncTodoWithLab(m_nLabID, m_nPatientID);
		}
	}
	/******************************************************
	// Auditing
	*******************************************************/
	//Location
	if(m_pLocationCombo->GetCurSel() != NULL || m_pLocationCombo->GetIsComboBoxTextInUse()){
		if(m_pLocationCombo->GetCurSel() != NULL)
			strAuditNewValue = VarString(m_pLocationCombo->CurSel->GetValue(lccName));
		else
			strAuditNewValue = GetCurrentLocationName();
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabLocation, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}
	//Receiving Lab
	if(m_pReceivingLabCombo->GetCurSel() != NULL || m_pReceivingLabCombo->GetIsComboBoxTextInUse()){
		if(m_pReceivingLabCombo->GetCurSel() != NULL)
			strAuditNewValue = VarString(m_pReceivingLabCombo->CurSel->GetValue(rlcName));
		else
			strAuditNewValue = "<No Location>";
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabLabSentTo, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}
		
	//Providers
	if(m_dwProviders.GetSize() >0){

		CString strProvs = "";
		strAuditNewValue = "";
		SortDWordArray(m_dwProviders);
		strProvs = GetIDStringFromArray(&m_dwProviders);
		_RecordsetPtr rsNew = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name "
			" FROM PersonT WHERE ID IN (%s) ORDER BY Last, First, Middle ASC", strProvs);
		while (! rsNew->eof) {
			//Make a semicolon delimited list
			strAuditNewValue += AdoFldString(rsNew, "Name", "") + "; ";
			rsNew->MoveNext();
		}
		rsNew->Close();

		//take off the last semicolon
		strAuditNewValue = strAuditNewValue.Left(strAuditNewValue.GetLength() - 2);

		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabProvider, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}
	//Insurance
	if(m_pInsuranceCombo->CurSel != NULL){
		strAuditNewValue = VarString(m_pInsuranceCombo->CurSel->GetValue(ilcInsuranceCo), "");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabInsuredParty, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}
	//Med Assistant
	if (m_pMedAssistantCombo->CurSel != NULL) {
		strAuditNewValue = VarString(m_pMedAssistantCombo->CurSel->GetValue(malcName), "");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabMedAssistant, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	//Biopsy Date
	if(varBiopsy.vt != VT_NULL){
		strAuditNewValue = FormatDateTimeForInterface(VarDateTime(varBiopsy));
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabBiopsyDate, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}
	//Anatomic location
	//TES 11/6/2009 - PLID 36189 - It may be inactive
	if(m_pAnatomyCombo->CurSel != NULL || m_pAnatomyCombo->IsComboBoxTextInUse){
		strAuditNewValue = (m_pAnatomyCombo->CurSel == NULL) ? CString((LPCTSTR)m_pAnatomyCombo->ComboBoxText) :VarString(m_pAnatomyCombo->CurSel->GetValue(alcDescription));
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabAnatomicLocation, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	//Biopsy Type
	// (d.lange 2011-01-03 14:42) - PLID 29065 - Added Biopsy Type
	if(m_pBiopsyTypeCombo->CurSel != NULL || m_pBiopsyTypeCombo->IsComboBoxTextInUse) {
		strAuditNewValue = (m_pBiopsyTypeCombo->CurSel == NULL) ? CString((LPCTSTR)m_pBiopsyTypeCombo->ComboBoxText) : VarString(m_pBiopsyTypeCombo->CurSel->GetValue(btcDescription));
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabBiopsyType, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	//Anatomy Side
	//TES 12/7/2009 - PLID 36470 - AnatomySide is back.
	if(nAnatomySide == asRight)
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabAnatomicSide, nNewLabID, "", "Right", aepMedium, aetCreated);
	if(nAnatomySide == asLeft)
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabAnatomicSide, nNewLabID, "", "Left", aepMedium, aetCreated);
	//TES 11/10/2009 - PLID 36128 - This is now AnatomyQualifierID
	if(m_pAnatomyQualifiersCombo->CurSel != NULL || m_pAnatomyQualifiersCombo->IsComboBoxTextInUse) {
		strAuditNewValue = (m_pAnatomyQualifiersCombo->CurSel == NULL) ? CString((LPCTSTR)m_pAnatomyQualifiersCombo->ComboBoxText) : VarString(m_pAnatomyQualifiersCombo->CurSel->GetValue(alqcName));
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabAnatomicLocationQualifier, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	//Clinical Data
	if(!strClinicalData.IsEmpty()){
		//Make the audit string more readable, remove the new line characters
		strAuditNewValue = strClinicalData;
		strAuditNewValue.Replace("\r\n", "  ");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabClinicalData, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	// (c.haag 2009-05-11 14:11) - PLID 28515
	if (!strInstructions.IsEmpty()) {
		//Make the audit string more readable, remove the new line characters
		strAuditNewValue = strInstructions;
		strAuditNewValue.Replace("\r\n", "  ");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabInstructions, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	// (z.manning 2008-10-24 10:33) - PLID 31807 - to be ordered
	if(!strToBeOrdered.IsEmpty()) {
		strAuditNewValue = strToBeOrdered;
		strAuditNewValue.Replace("\r\n", " ");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabToBeOrdered, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	// (z.manning 2008-11-24 15:28) - PLID 31990 - Melanoma history and previous biopsy-- only audit
	// these if they changed.
	if(bMelanomaHistory != m_bSavedMelanomaHistory) {
		CString strAuditOldValue;
		if(bMelanomaHistory) {
			strAuditOldValue = "No";
			strAuditNewValue = "Yes";
		}
		else {
			strAuditOldValue = "Yes";
			strAuditNewValue = "No";
		}
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabMelanomaHistory, m_nLabID, strAuditOldValue, strAuditNewValue, aepMedium, aetCreated);
	}

	if(bPreviousBiopsy != m_bSavedPreviousBiopsy) {
		CString strAuditOldValue;
		if(bPreviousBiopsy) {
			strAuditOldValue = "No";
			strAuditNewValue = "Yes";
		}
		else {
			strAuditOldValue = "Yes";
			strAuditNewValue = "No";
		}
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabPreviousBiopsy, m_nLabID, strAuditOldValue, strAuditNewValue, aepMedium, aetCreated);
	}
	//TES 5/14/2009 - PLID 33792 - Added Initial Diagnosis
	if(!strInitialDiagnosis.IsEmpty()){
		//Make the audit string more readable, remove the new line characters
		strAuditNewValue = strInitialDiagnosis;
		strAuditNewValue.Replace("\r\n", "  ");
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabInitialDiagnosis, m_nLabID, "", strAuditNewValue, aepMedium, aetCreated);
	}

	// (c.haag 2010-12-10 9:02) - PLID 41590 - Audit signing separately
	if(m_nSignedByUserID > 0) {
		strAuditNewValue = "<Signed>";
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabSigned, m_nLabID, GenerateCompletionAuditOld(), strAuditNewValue, aepMedium, aetCreated);
	}

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	if(bCCPatient != m_bSavedCCPatient) {
		CString strAuditOldValue;
		if(bCCPatient) {
			strAuditOldValue = "No";
			strAuditNewValue = "Yes";
		}
		else {
			strAuditOldValue = "Yes";
			strAuditNewValue = "No";
		}
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabCCPatient, m_nLabID, strAuditOldValue, strAuditNewValue, aepMedium, aetCreated);
	}
	if(bCCRefPhys != m_bSavedCCRefPhys) {
		CString strAuditOldValue;
		if(bCCRefPhys) {
			strAuditOldValue = "No";
			strAuditNewValue = "Yes";
		}
		else {
			strAuditOldValue = "Yes";
			strAuditNewValue = "No";
		}
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabCCRefPhys, m_nLabID, strAuditOldValue, strAuditNewValue, aepMedium, aetCreated);
	}
	if(bCCPCP != m_bSavedCCPCP) {
		CString strAuditOldValue;
		if(bCCPCP) {
			strAuditOldValue = "No";
			strAuditNewValue = "Yes";
		}
		else {
			strAuditOldValue = "Yes";
			strAuditNewValue = "No";
		}
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabCCPCP, m_nLabID, strAuditOldValue, strAuditNewValue, aepMedium, aetCreated);
	}

	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	if(strLoincCode != m_strSavedLoincCode) {
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabLoincCode, m_nLabID, m_strSavedLoincCode, strLoincCode, aepMedium, aetCreated);
	}
	if(strLoincCode != m_strSavedLoincDescription) {
		AuditEvent(m_nPatientID, strAuditPatientName, nAuditTransID, aeiPatientLabLoincDescription, m_nLabID, m_strSavedLoincDescription, strLoincDescription, aepMedium, aetCreated);
	}


	//TES 11/25/2009 - PLID 36191 - Now save our EMR Problems
	//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
	SaveProblems(nAuditTransID, arNewCDSInterventions);

	// (z.manning 2010-05-05 10:46) - PLID 37190 - Update the stored value of last saved initial diagnosis
	m_strSavedInitialDiagnosis = strInitialDiagnosis;

	//TES 9/29/2010 - PLID 40644 - Also the Comments (clinical data), Lab Location, Location, and Provider
	m_strSavedClinicalData = strClinicalData;
	m_nSavedLabLocationID = nLabLocationID;
	m_nSavedLocationID = nLocationID;
	m_dwSavedProviderIDs.Copy(m_dwProviders);
	m_strSavedProviderNames = m_strProviderNames;

	// (j.jones 2010-10-07 13:20) - PLID 40743 - update several more "last saved" values
	m_pLabEntryDlg->SetSavedFormNumber(sli.strFormTextID);
	m_strSavedSpecimen = strSpecimen;	
	m_strSavedInstructions = strInstructions;
	m_strSavedToBeOrdered = strToBeOrdered;
	m_nMedAssistantID = nMedAssistantID;
	m_nInsuredPartyID = nInsuredPartyID;	
	m_nSavedAnatomyID = nAnatomyID;
	m_nSavedAnatomySide = nAnatomySide;
	m_nSavedAnatomyQualifierID = nAnatomyQualifierID;

	if(varBiopsy.vt == VT_DATE) {
		m_dtSavedBiopsyDate = VarDateTime(varBiopsy);
	}
	else {
		m_dtSavedBiopsyDate.SetStatus(COleDateTime::invalid);
	}

	m_bSavedMelanomaHistory = bMelanomaHistory;
	m_bSavedPreviousBiopsy = bPreviousBiopsy;	

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	m_bSavedCCPatient = bCCPatient;
	m_bSavedCCRefPhys = bCCRefPhys;
	m_bSavedCCPCP = bCCPCP;

	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	m_strSavedLoincCode = strLoincCode;
	m_strSavedLoincDescription = strLoincDescription;


	// (c.haag 2010-07-19 15:55) - PLID 30894 - Fire a table checker
	// (r.gonet 08/25/2014) - PLID 63221 - Send the patient ID as well
	CClient::RefreshLabsTable(m_nPatientID, m_nLabID);
}

CString CLabRequisitionDlg::GenerateCompletionAuditOld()
{
	//Format is Form - Specimen - Location, R/L

	//TES 11/25/2009 - PLID 36193 - We've got the specimen now; get the form number from our parent.
	CString strOld = m_pLabEntryDlg->GetFormNumber();
	CString strSpecimen;
	GetDlgItemText(IDC_SPECIMEN, strSpecimen);
	if(!strSpecimen.IsEmpty()) {
		strOld += " - " + strSpecimen;
	}

	// (z.manning 2008-11-03 14:15) - PLID 31864 - Factor in the lab type
	if(m_ltType == ltBiopsy)
	{
		//get the anatomic location
		IRowSettingsPtr pRowLoc = m_pAnatomyCombo->GetCurSel();
		if(pRowLoc == NULL) {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			if(m_pAnatomyCombo->IsComboBoxTextInUse) {
				strOld += " - " + CString((LPCTSTR)m_pAnatomyCombo->ComboBoxText);
			}
		}
		else {
			strOld += " - " + VarString(pRowLoc->GetValue(alcDescription), "");
		}

		//TES 12/7/2009 - PLID 36470 - Restored AnatomySide, we'll put it before the qualifier;
		CString strSide;
		if(IsDlgButtonChecked(IDC_LEFT_SIDE)) {
			strSide = "Left";
		}
		else if(IsDlgButtonChecked(IDC_RIGHT_SIDE)) {
			strSide = "Right";
		}
		//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
		IRowSettingsPtr pQualRow = m_pAnatomyQualifiersCombo->CurSel;
		CString strQual;
		if(pQualRow == NULL) {
			if(m_pAnatomyQualifiersCombo->IsComboBoxTextInUse) {
				strQual = CString((LPCTSTR)m_pAnatomyQualifiersCombo->ComboBoxText);
			}
		}
		else {
			strQual = VarString(pQualRow->GetValue(alqcName),"");
		}
		CString strFullQual;
		if(!strSide.IsEmpty() && !strQual.IsEmpty()) {
			strFullQual = strSide + " " + strQual;
		}
		else {
			strFullQual = strSide + strQual;
		}
		if(!strFullQual.IsEmpty()) {
			strOld += ", " + strFullQual;
		}
	}
	else {
		CString strToBeOrdered;
		m_nxeditToBeOrderedText.GetWindowText(strToBeOrdered);
		if(!strOld.IsEmpty()) {
			strOld += " - " + strToBeOrdered;
		}
	}

	return strOld;
}

//Returns FALSE for bad data (NULL ptr, invalid columns), or if the given ID is not found.  Returns TRUE otherwise.
//	strText is only modified if return value is TRUE
//This function will not change the current selection of the given datalist2
BOOL FindTextFromDatalist2(NXDATALIST2Lib::_DNxDataListPtr pList, long nIDValueToFind, short nColForID, short nColForText, OUT CString& strText)
{
	if(pList == NULL)
		return FALSE;

	//ensure we have enough columns
	if(pList->ColumnCount < nColForID || pList->ColumnCount < nColForText)
		return FALSE;

	//showtime!
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->FindByColumn(nColForID, (long)nIDValueToFind, NULL, VARIANT_FALSE);
	if(pRow == NULL) {
		//Failed to find
		return FALSE;
	}

	strText = VarString(pRow->GetValue(nColForText), "");

	return TRUE;
}


//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CLabRequisitionDlg::SaveExisting(long &nAuditTransID, IN OUT CDWordArray &arNewCDSInterventions)
{
	//Find out what has changed
	CString strNewSpecimen, strNewClinicalData, strNewToBeOrdered, strInstructions, strNewInitialDiagnosis;

	BOOL bEmrDataChanged = FALSE;
	EMNLab emnlab;
	emnlab.nID = m_nLabID;
	emnlab.eType = m_ltType;
	// (z.manning 2008-10-09 12:57) - PLID 31628 - Now track lab procedure ID too.
	emnlab.nLabProcedureID = m_nLabProcedureID;
	BOOL bNeedToUpdateTodos = FALSE;

	//We want all dates to default to an invalid date
	COleDateTime dtDefault = COleDateTime(0.00);
	dtDefault.SetStatus(COleDateTime::invalid);

	CString strSql= "";
	CString strTempSet ="";

	//DRT 7/3/2006 - PLID 21083 - Audit!
	if(nAuditTransID == -1) {
		nAuditTransID = BeginAuditTransaction();
	}
	CString strName = GetExistingPatientName(m_nPatientID);

	//Form Number
	//TES 11/25/2009 - PLID 36193 - Get the old and new values from the LabEntryDlg
	CString strNewFormNumber = m_pLabEntryDlg->GetFormNumber();
	CString strOldFormNumber = m_pLabEntryDlg->GetSavedFormNumber();
	if(strOldFormNumber != strNewFormNumber){
		strTempSet.Format(" FormNumberTextID = '%s', ", _Q(strNewFormNumber));
		strSql += strTempSet;

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabFormNum, m_nLabID, strOldFormNumber, strNewFormNumber, aepMedium, aetChanged);
	}

	//TES 11/25/2009 - PLID 36193 - The specimen is now its own, editable field.
	//Specimen
	GetDlgItemText(IDC_SPECIMEN, strNewSpecimen);
	if(m_strSavedSpecimen != strNewSpecimen){
		strTempSet.Format(" Specimen = '%s', ", _Q(strNewSpecimen));
		strSql += strTempSet;

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabSpecimen, m_nLabID, m_strSavedSpecimen, strNewSpecimen, aepMedium, aetChanged);
	}

	//Clinical Data
	GetDlgItemText(IDC_CLINICAL_DATA, strNewClinicalData);
	emnlab.strClinicalData = strNewClinicalData;
	if(m_strSavedClinicalData != strNewClinicalData){

		// (z.manning 2008-10-08 10:12) - PLID 31613 - EMR related data has changed
		bEmrDataChanged = TRUE;

		strTempSet.Format(" ClinicalData = '%s', ", _Q(strNewClinicalData));
		strSql += strTempSet;

		//(e.lally 2007-08-01) - Made the audit strings more readable 
		CString strOld = m_strSavedClinicalData, strNew = strNewClinicalData;
		strOld.Replace("\r\n", "  ");
		strNew.Replace("\r\n", "  ");

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabClinicalData, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	// (c.haag 2009-05-11 13:55) - PLID 28515 - Instructions
	m_nxeditInstructions.GetWindowText(strInstructions);
	if(m_strSavedInstructions != strInstructions)
	{
		strTempSet.Format(" Instructions = '%s', ", _Q(strInstructions));
		strSql += strTempSet;

		CString strOld = m_strSavedInstructions;
		CString strNew = strInstructions;
		strOld.Replace("\r\n", " ");
		strNew.Replace("\r\n", " ");
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabInstructions, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}
	
	// (z.manning 2008-10-24 10:24) - PLID 31807 - to be ordered
	m_nxeditToBeOrderedText.GetWindowText(strNewToBeOrdered);
	emnlab.strToBeOrdered = strNewToBeOrdered;
	if(m_strSavedToBeOrdered != strNewToBeOrdered)
	{
		bEmrDataChanged = TRUE;

		strTempSet.Format(" ToBeOrdered = '%s', ", _Q(strNewToBeOrdered));
		strSql += strTempSet;

		CString strOld = m_strSavedToBeOrdered;
		CString strNew = strNewToBeOrdered;
		strOld.Replace("\r\n", " ");
		strNew.Replace("\r\n", " ");
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabToBeOrdered, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	// (a.walling 2006-07-24 15:55) - PLID 21571 - Add Medical Assistant auditing and saving
	//Lab Medical Assistant
	long nNewMedID;

	if(m_pMedAssistantCombo->GetIsComboBoxTextInUse() != FALSE)
	{
		nNewMedID = m_nMedAssistantID;
	}
	else
	{
		IRowSettingsPtr pRow = m_pMedAssistantCombo->GetCurSel();
		if (pRow) {
			nNewMedID= VarLong(pRow->GetValue(malcUserID), -1);
		}
		else {
			nNewMedID= -1;
		}
	}

	if(m_nMedAssistantID != nNewMedID){
		if (nNewMedID > 0) {
			strTempSet.Format(" MedAssistant = %li, ", nNewMedID);
		}
		else {
			strTempSet = " MedAssistant = NULL, ";
		}
		strSql += strTempSet;

		//Auditing
		CString strOld, strNew;
		if(m_nMedAssistantID > 0) {
			if(FindTextFromDatalist2(m_pMedAssistantCombo, m_nMedAssistantID, malcUserID, malcName, strOld) == FALSE) {
				//Old MedAssistant does not exist... inactive or deleted.
				_RecordsetPtr prs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First AS Name FROM PersonT WHERE ID = %li", m_nMedAssistantID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Name", "");
				}
				else {
					//Once again: Deleted?  Bad data?  This shouldn't be possible, but we will have to report no user
					strOld = "<No User>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No User>";
		}

		if(nNewMedID > 0) {
			if(FindTextFromDatalist2(m_pMedAssistantCombo, nNewMedID, malcUserID, malcName, strNew) == FALSE) {
				//New MedAssistant does not exist... inactive or deleted.
				_RecordsetPtr prs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First AS Name FROM PersonT WHERE ID = %li", m_nMedAssistantID);
				if(!prs->eof) {
					strNew = AdoFldString(prs, "Name", "");
				}
				else {
					//Once again: Deleted?  Bad data?  This shouldn't be possible, but we will have to report no user
					strNew = "<No User>";
				}
				prs->Close();
			}
		}
		else {
			strNew = "<No User>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabMedAssistant, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	// (a.walling 2006-07-25 10:28) - PLID 21575 - Insured Party
	//InsuredPartyID
	long nNewInsuredPartyID;

	if(m_pInsuranceCombo->GetIsComboBoxTextInUse() != FALSE)
	{
		nNewInsuredPartyID = m_nInsuredPartyID;
	}
	else
	{	
		IRowSettingsPtr pRow = m_pInsuranceCombo->GetCurSel();
		if (pRow) {
			nNewInsuredPartyID= VarLong(pRow->GetValue(ilcInsuredPartyID), -1);
		}
		else {
			nNewInsuredPartyID= -1;
		}
	}

	if(m_nInsuredPartyID != nNewInsuredPartyID){
		if (nNewInsuredPartyID > 0) {
			strTempSet.Format(" InsuredPartyID = %li, ", nNewInsuredPartyID);
		}
		else {
			strTempSet = " InsuredPartyID = NULL, ";
		}
		strSql += strTempSet;

		//Auditing
		CString strOld, strNew;
		if(m_nInsuredPartyID > 0) {
			if(FindTextFromDatalist2(m_pInsuranceCombo, m_nInsuredPartyID, ilcInsuredPartyID, ilcInsuranceCo, strOld) == FALSE) {
				//Old party does not exist... inactive or deleted.
				_RecordsetPtr prs = CreateRecordset("SELECT InsuranceCoT.Name AS InsuranceCoName FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID WHERE InsuredPartyT.PersonID = %li", m_nInsuredPartyID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "InsuranceCoName", "");
				}
				else {
					//Once again: Deleted?  Bad data?  This shouldn't be possible, but we will have to report no user
					strOld = "<No Insurance>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No Insurance>";
		}

		if(nNewInsuredPartyID > 0) {
			if(FindTextFromDatalist2(m_pInsuranceCombo, nNewInsuredPartyID, ilcInsuredPartyID, ilcInsuranceCo, strNew) == FALSE) {
				//New party does not exist... inactive or deleted.
				_RecordsetPtr prs = CreateRecordset("SELECT InsuranceCoT.Name AS InsuranceCoName FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID WHERE InsuredPartyT.PersonID = %li", m_nInsuredPartyID);
				if(!prs->eof) {
					strNew = AdoFldString(prs, "InsuranceCoName", "");
				}
				else {
					//Once again: Deleted?  Bad data?  This shouldn't be possible, but we will have to report no user
					strNew = "<No Insurance>";
				}
				prs->Close();
			}
		}
		else {
			strNew = "<No Insurance>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabInsuredParty, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	//LocationID
	long nNewLocationID;
	if(m_pLocationCombo->GetIsComboBoxTextInUse() != FALSE)
		nNewLocationID = m_nSavedLocationID;
	else
		nNewLocationID = VarLong(m_pLocationCombo->CurSel->GetValue(lccLocationID));
	if(m_nSavedLocationID != nNewLocationID){
		strTempSet.Format(" LocationID = %li, ", nNewLocationID);
		strSql += strTempSet;

		//Auditing
		CString strOld, strNew;
		if(m_nSavedLocationID > 0) {
			if(FindTextFromDatalist2(m_pLocationCombo, m_nSavedLocationID, lccLocationID, lccName, strOld) == FALSE) {
				//Old location is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", m_nSavedLocationID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no Location
					strOld = "<No Location>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No Location>";
		}

		if(nNewLocationID > 0) {
			if(FindTextFromDatalist2(m_pLocationCombo, nNewLocationID, lccLocationID, lccName, strNew) == FALSE) {
				//New location is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", nNewLocationID);
				if(!prs->eof) {
					strNew = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no location
					strNew = "<No Location>";
				}
				prs->Close();
			}
		}
		else {
			strNew = "<No Location>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabLocation, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	// (d.lange 2011-01-03 11:49) - PLID 29065 - Biopsy Type
	IRowSettingsPtr pBiopsyTypeRow = m_pBiopsyTypeCombo->CurSel;
	long nNewBiopsyTypeID = -1;
	CString strNewBiopsyTypeID;
	if(pBiopsyTypeRow) {
		nNewBiopsyTypeID = VarLong(pBiopsyTypeRow->GetValue(btcBiopsyTypeID));
		strNewBiopsyTypeID.Format("%li", nNewBiopsyTypeID);
	}else {
		if(m_pBiopsyTypeCombo->IsComboBoxTextInUse) {
			nNewBiopsyTypeID = m_nCurBiopsyTypeID;
			strNewBiopsyTypeID.Format("%li", m_nCurBiopsyTypeID);
		}else {
			strNewBiopsyTypeID.Format("NULL");
		}
	}

	CString strNewBiopsyType;
	if(m_nSavedBiopsyTypeID != nNewBiopsyTypeID) {
		CString strNewBiopsyTypeID = "NULL";
		if(nNewBiopsyTypeID > 0) {
			strNewBiopsyTypeID.Format("%li", nNewBiopsyTypeID);
		}
		strTempSet.Format(" BiopsyTypeID = %s, ", strNewBiopsyTypeID);
		strSql += strTempSet;

		//Auditing
		CString strOld;
		if(m_nSavedBiopsyTypeID > 0) {
			if(FindTextFromDatalist2(m_pBiopsyTypeCombo, m_nSavedBiopsyTypeID, btcBiopsyTypeID, btcDescription, strOld) == FALSE) {
				//The old biopsy type is not in the datalist
				_RecordsetPtr prs = CreateParamRecordset("SELECT Description FROM LabBiopsyTypeT WHERE ID = {INT}", m_nSavedBiopsyTypeID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Description", "");
				}else {
					strOld = "<No Biopsy Type>";
				}
				prs->Close();
			}
		}else {
			strOld = "<No Biopsy Type>";
		}

		if(nNewBiopsyTypeID > 0) {
			if(FindTextFromDatalist2(m_pBiopsyTypeCombo, nNewBiopsyTypeID, btcBiopsyTypeID, btcDescription, strNewBiopsyType) == FALSE) {
				// (d.lange 2011-01-14 11:43) - PLID 29065 - grab the previous saved biopsy type thats been inactived for auditing
				_RecordsetPtr prs = CreateParamRecordset("SELECT Description FROM LabBiopsyTypeT WHERE ID = {INT}", nNewBiopsyTypeID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Description", "");
				}else {
					strOld = "<No Biopsy Type>";
				}
				prs->Close();
			}
		}else {
			strNewBiopsyType = "<No Biopsy Type>";
		}

		//Audit the event
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabBiopsyType, m_nLabID, strOld, strNewBiopsyType, aepMedium, aetChanged);
	}

	//AnatomyID
	// (r.galicki 2008-10-20 17:09) - PLID 31552 - Needed to handle case when anatomy combo was hidden from user (no choice made)
	IRowSettingsPtr pAnatomyRow = m_pAnatomyCombo->CurSel;
	long nNewAnatomyID = -1;
	CString strNewAnatomyID;
	if(pAnatomyRow) {
		nNewAnatomyID = VarLong(pAnatomyRow->GetValue(alcAnatomyID));
		strNewAnatomyID.Format("%li", nNewAnatomyID);
	}
	else {
		//TES 11/6/2009 - PLID 36189 - It may be inactive
		if(m_pAnatomyCombo->IsComboBoxTextInUse) {
			nNewAnatomyID = m_nCurAnatomyID;
			strNewAnatomyID.Format("%li", m_nCurAnatomyID);
		}
		else {
			strNewAnatomyID.Format("NULL");
		}
	}

	CString strNewAnatomicLocation;
	//TES 11/10/2009 - PLID 36266 - Checking for positive nNewAnatomyIDs meant that you could never change an existing lab to have 
	// an empty Anatomic Location, which is supposed to be legal now.
	if(/*nNewAnatomyID != -1 && */m_nSavedAnatomyID != nNewAnatomyID){

		// (z.manning 2008-10-08 10:12) - PLID 31613 - EMR related data has changed
		bEmrDataChanged = TRUE;

		CString strNewAnatomyID = "NULL";
		if(nNewAnatomyID > 0) {
			strNewAnatomyID.Format("%li", nNewAnatomyID);
		}
		strTempSet.Format(" AnatomyID = %s, ", strNewAnatomyID);
		strSql += strTempSet;

		//Auditing
		CString strOld;
		if(m_nSavedAnatomyID > 0) {
			if(FindTextFromDatalist2(m_pAnatomyCombo, m_nSavedAnatomyID, alcAnatomyID, alcDescription, strOld) == FALSE) {
				//Old anatomy is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Description FROM LabAnatomyT WHERE ID = %li", m_nSavedAnatomyID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Description", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no location
					//TES 12/3/2008 - PLID 32302 - NOTE: This is ok now.
					strOld = "<No Anatomical Location>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No Anatomical Location>";
		}

		if(nNewAnatomyID > 0) {
			if(FindTextFromDatalist2(m_pAnatomyCombo, nNewAnatomyID, alcAnatomyID, rlcName, strNewAnatomicLocation) == FALSE) {
				//New anatomy is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Description FROM LabAnatomyT WHERE ID = %li", nNewAnatomyID);
				if(!prs->eof) {
					strNewAnatomicLocation = AdoFldString(prs, "Description", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no location
					//TES 12/3/2008 - PLID 32302 - NOTE: This is ok now.
					strNewAnatomicLocation = "<No Anatomical Location>";
				}
				prs->Close();
			}
		}
		else {
			strNewAnatomicLocation = "<No Anatomical Location>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabAnatomicLocation, m_nLabID, strOld, strNewAnatomicLocation, aepMedium, aetChanged);
	}
	// (r.galicki 2008-10-20 17:13) - PLID 31552 - Check if anatomy combo is hidden
	if(pAnatomyRow) {
		emnlab.strAnatomicLocation = VarString(pAnatomyRow->GetValue(alcDescription), "");
	}
	else {
		//TES 11/6/2009 - PLID 36189 - It may be inactive
		if(m_pAnatomyCombo->IsComboBoxTextInUse) {
			emnlab.strAnatomicLocation = CString((LPCTSTR)m_pAnatomyCombo->ComboBoxText);
		}
		else {
			emnlab.strAnatomicLocation = "";
		}
	}
	//AnatomySide
	//TES 12/7/2009 - PLID 36470 - Brought AnatomySide back.
	long nNewAnatomySide = asNone;
	if(IsDlgButtonChecked(IDC_LEFT_SIDE))
		nNewAnatomySide = asLeft;
	else if(IsDlgButtonChecked(IDC_RIGHT_SIDE))
		nNewAnatomySide = asRight;
	emnlab.eAnatomicSide = (AnatomySide)nNewAnatomySide;
	if(m_nSavedAnatomySide != nNewAnatomySide) {
		// (z.manning 2008-10-08 10:12) - PLID 31613 - EMR related data has changed
		bEmrDataChanged = TRUE;
		strTempSet.Format(" AnatomySide = %li, ", nNewAnatomySide);
		strSql += strTempSet;

		CString strOld = "<None>", strNew = "<None>";
		if(m_nSavedAnatomySide == asLeft)
			strOld = "Left";
		else if(m_nSavedAnatomySide == asRight)
			strOld = "Right";

		if(nNewAnatomySide == asLeft) {
			strNew = "Left";
		}
		else if(nNewAnatomySide == asRight) {
			strNew = "Right";
		}
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabAnatomicSide, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	//TES 11/10/2009 - PLID 36128 - This is now AnatomyQualifierID
	IRowSettingsPtr pQualifierRow = m_pAnatomyQualifiersCombo->CurSel;
	long nNewQualifierID = -1;
	CString strNewQualifierID;
	if(pQualifierRow) {
		nNewQualifierID = VarLong(pQualifierRow->GetValue(alqcID));
		if(nNewQualifierID == -1) {
			strNewQualifierID = "NULL";
		}
		else {
			strNewQualifierID.Format("%li", nNewQualifierID);
		}
	}
	else {
		//TES 11/10/2009 - PLID 36128 - It may be inactive
		if(m_pAnatomyQualifiersCombo->IsComboBoxTextInUse) {
			nNewQualifierID = m_nCurAnatomyQualifierID;
			strNewQualifierID.Format("%li", m_nCurAnatomyQualifierID);
		}
		else {
			strNewQualifierID.Format("NULL");
		}
	}
	CString strNewQualifier;
	if(m_nSavedAnatomyQualifierID != nNewQualifierID){

		// (z.manning 2008-10-08 10:12) - PLID 31613 - EMR related data has changed
		bEmrDataChanged = TRUE;

		strTempSet.Format(" AnatomyQualifierID = %s, ", strNewQualifierID);
		strSql += strTempSet;

		//Auditing
		CString strOld;
		if(m_nSavedAnatomyQualifierID > 0) {
			if(FindTextFromDatalist2(m_pAnatomyQualifiersCombo, m_nSavedAnatomyQualifierID, alqcID, alqcName, strOld) == FALSE) {
				//Old qualifier is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = %li", m_nSavedAnatomyQualifierID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no location
					//TES 12/3/2008 - PLID 32302 - NOTE: This is ok now.
					strOld = "<No Anatomical Location Qualifier>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No Anatomical Location Qualifier>";
		}

		if(nNewQualifierID > 0) {
			if(FindTextFromDatalist2(m_pAnatomyQualifiersCombo, nNewQualifierID, alqcID, alqcName, strNewQualifier) == FALSE) {
				//New qualifier is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = %li", nNewQualifierID);
				if(!prs->eof) {
					strNewQualifier = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no location
					//TES 12/3/2008 - PLID 32302 - NOTE: This is ok now.
					strNewQualifier = "<No Anatomical Location Qualifier>";
				}
				prs->Close();
			}
		}
		else {
			strNewQualifier = "<No Anatomical Location Qualifier>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabAnatomicLocationQualifier, m_nLabID, strOld, strNewQualifier, aepMedium, aetChanged);
	}
	// (r.galicki 2008-10-20 17:13) - PLID 31552 - Check if anatomy combo is hidden
	if(pQualifierRow) {
		emnlab.strAnatomicLocationQualifier = VarString(pQualifierRow->GetValue(alqcName), "");
	}
	else {
		//TES 11/10/2009 - PLID 36128 - It may be inactive
		if(m_pAnatomyQualifiersCombo->IsComboBoxTextInUse) {
			emnlab.strAnatomicLocationQualifier = CString((LPCTSTR)m_pAnatomyQualifiersCombo->ComboBoxText);
		}
		else {
			emnlab.strAnatomicLocationQualifier = "";
		}
	}
	
	//BiopsyDate
	_variant_t varBiopsy = m_dtpBiopsy.GetValue();
	COleDateTime dtNewBiopsyDate;
	if(varBiopsy.vt == VT_NULL) {
		dtNewBiopsyDate.SetStatus(COleDateTime::invalid);
	}
	else {
		dtNewBiopsyDate = VarDateTime(varBiopsy, dtDefault);
		//TES 7/16/2010 - PLID 39575 - We also need the time.
		if(m_nxtBiopsyTime->GetStatus() == 1) {
			COleDateTime dtBiopsyTime = m_nxtBiopsyTime->GetDateTime();
			dtNewBiopsyDate.SetDateTime(dtNewBiopsyDate.GetYear(), dtNewBiopsyDate.GetMonth(), dtNewBiopsyDate.GetDay(), dtBiopsyTime.GetHour(), dtBiopsyTime.GetMinute(), dtBiopsyTime.GetSecond());
		}
		else {
			dtNewBiopsyDate.SetDateTime(dtNewBiopsyDate.GetYear(), dtNewBiopsyDate.GetMonth(), dtNewBiopsyDate.GetDay(), 0, 0, 0);
		}
	}
	if(m_dtSavedBiopsyDate != dtNewBiopsyDate){
		// (j.jones 2010-05-13 08:58) - PLID 38564 - check the validity of each datetime,
		// do not check varBiopsy anymore
		if(dtNewBiopsyDate.GetStatus() == COleDateTime::valid)
			strTempSet.Format(" BiopsyDate = '%s', ", FormatDateTimeForSql(dtNewBiopsyDate));
		else
			strTempSet= " BiopsyDate = NULL, ";
		strSql += strTempSet;

		//TES 7/16/2010 - PLID 39575 - We now have a time, so include it in the auditing.
		CString strOld, strNew;
		if(m_dtSavedBiopsyDate.GetStatus() == COleDateTime::valid) 
			strOld = FormatDateTimeForInterface(m_dtSavedBiopsyDate, NULL);
		else
			strOld = "<No Biopsy Date>";
		if(dtNewBiopsyDate.GetStatus() == COleDateTime::valid)
			strNew = FormatDateTimeForInterface(dtNewBiopsyDate, NULL);
		else
			strNew = "<No Biopsy Date>";

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabBiopsyDate, m_nLabID, strOld, strNew, aepMedium, aetChanged);

		//now assign the saved biopsy date
		m_dtSavedBiopsyDate = dtNewBiopsyDate;
	}

	//LabLocationID
	long nNewLabLocationID;
	if(m_pReceivingLabCombo->GetIsComboBoxTextInUse() != FALSE)
		nNewLabLocationID = m_nSavedLabLocationID;
	else
		nNewLabLocationID = VarLong(m_pReceivingLabCombo->CurSel->GetValue(rlcLabLocationID));
	if(m_nSavedLabLocationID != nNewLabLocationID){
		strTempSet.Format(" LabLocationID = %li, ", nNewLabLocationID);
		strSql += strTempSet;

		//Auditing
		CString strOld, strNew;
		if(m_nSavedLabLocationID > 0) {
			if(FindTextFromDatalist2(m_pReceivingLabCombo, m_nSavedLabLocationID, rlcLabLocationID, rlcName, strOld) == FALSE) {
				//Old location is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", m_nSavedLabLocationID);
				if(!prs->eof) {
					strOld = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no Location
					strOld = "<No Location>";
				}
				prs->Close();
			}
		}
		else {
			strOld = "<No Location>";
		}

		if(nNewLabLocationID > 0) {
			if(FindTextFromDatalist2(m_pReceivingLabCombo, nNewLabLocationID, rlcLabLocationID, rlcName, strNew) == FALSE) {
				//New location is not in datalist
				_RecordsetPtr prs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", nNewLabLocationID);
				if(!prs->eof) {
					strNew = AdoFldString(prs, "Name", "");
				}
				else {
					//Deleted?  Bad data?  This shouldn't be possible, but we will have to report no Location
					strNew = "<No Location>";
				}
				prs->Close();
			}
		}
		else {
			strNew = "<No Location>";
		}

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabLabSentTo, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}
		
	// (z.manning 2008-11-24 13:55) - PLID 31990 - Added previous biopsy and melanoma history
	BOOL bMelanomaHistory = m_nxbtnMelanomaHistory.GetCheck() == BST_CHECKED;
	if(bMelanomaHistory != m_bSavedMelanomaHistory) {
		CString strOld, strNew;
		if(bMelanomaHistory) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		strSql += FormatString(" MelanomaHistory = %i, ", bMelanomaHistory ? 1 : 0);
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabMelanomaHistory, m_nLabID, strOld, strNew, aepMedium, aetCreated);
	}
	BOOL bPreviousBiopsy = m_nxbtnPreviousBiopsy.GetCheck() == BST_CHECKED;
	if(bPreviousBiopsy != m_bSavedPreviousBiopsy) {
		CString strOld, strNew;
		if(bPreviousBiopsy) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		strSql += FormatString(" PreviousBiopsy = %i, ", bPreviousBiopsy ? 1 : 0);
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabPreviousBiopsy, m_nLabID, strOld, strNew, aepMedium, aetCreated);
	}

	//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
	GetDlgItemText(IDC_INITIAL_DIAGNOSIS, strNewInitialDiagnosis);
	if(m_strSavedInitialDiagnosis != strNewInitialDiagnosis){

		strTempSet.Format(" InitialDiagnosis = '%s', ", _Q(strNewInitialDiagnosis));
		strSql += strTempSet;

		//(e.lally 2007-08-01) - Made the audit strings more readable 
		CString strOld = m_strSavedInitialDiagnosis, strNew = strNewInitialDiagnosis;
		strOld.Replace("\r\n", "  ");
		strNew.Replace("\r\n", "  ");

		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabInitialDiagnosis, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	// (z.manning 2008-10-16 10:53) - PLID 21082 - Signature fields
	// (a.walling 2010-01-05 09:20) - PLID 33887 - Need to escape this filename
	strSql += FormatString(" SignatureImageFile = '%s', ", _Q(m_strSignatureFileName));
	CString strSigInkData;
	if(m_varSignatureInkData.vt == VT_NULL || m_varSignatureInkData.vt == VT_EMPTY) {
		strSigInkData = "NULL";
	}
	else {
		strSigInkData = CreateByteStringFromSafeArrayVariant(m_varSignatureInkData);
	}
	strSql += FormatString(" SignatureInkData = %s, ", strSigInkData);

	// (c.haag 2010-11-23 10:50) - PLID 41590 - New signature fields
	if(m_dtSavedSigned != m_dtSigned){
		//Check for invalid/NULL state
		if(m_dtSigned.GetStatus() == COleDateTime::valid){
			strTempSet.Format(" SignedDate = '%s', ", FormatDateTimeForSql(m_dtSigned));
		}
		else
			strTempSet = " SignedDate = NULL, ";
		strSql += strTempSet;
		// This cannot be edited, and therefore does not need audited.  There is 1 generic audit for
		//"signed".
	}
	if(m_nSavedSignedByUserID != m_nSignedByUserID){
		//Check for invalid/NULL state
		if(m_nSignedByUserID > 0)
			strTempSet.Format(" SignedBy = %li, ", m_nSignedByUserID);
		else
			strTempSet = " SignedBy = NULL, ";
		strSql += strTempSet;
		CString strOld = GenerateCompletionAuditOld();
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabSigned, m_nLabID, strOld, "<Signed>", aepMedium, aetChanged);
	}

	// (j.jones 2010-04-12 17:32) - PLID 38166 - added a date/timestamp
	CString strSigTextData;
	if(m_varSignatureTextData.vt == VT_NULL || m_varSignatureTextData.vt == VT_EMPTY) {
		strSigTextData = "NULL";
	}
	else {
		strSigTextData = CreateByteStringFromSafeArrayVariant(m_varSignatureTextData);
	}
	strSql += FormatString(" SignatureTextData = %s, ", strSigTextData);

	// (r.galicki 2008-10-28 15:35) - PLID 31552 - Add procedure type
	if(m_ltType != ltInvalid) {
		strSql += FormatString(" Type = %li, ", m_ltType);
	}
	else { //should never happen
		ASSERT(FALSE);
	}

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	BOOL bCCPatient = m_nxbtnCCPatient.GetCheck() == BST_CHECKED;
	if(bCCPatient != m_bSavedCCPatient) {
		CString strOld, strNew;
		if(bCCPatient) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		strSql += FormatString(" CC_Patient = %i, ", bCCPatient ? 1 : 0);
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabCCPatient, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}
	BOOL bCCRefPhys = m_nxbtnCCRefPhys.GetCheck() == BST_CHECKED;
	if(bCCRefPhys != m_bSavedCCRefPhys) {
		CString strOld, strNew;
		if(bCCRefPhys) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		strSql += FormatString(" CC_RefPhys = %i, ", bCCRefPhys ? 1 : 0);
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabCCRefPhys, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}
	BOOL bCCPCP = m_nxbtnCCPCP.GetCheck() == BST_CHECKED;
	if(bCCPCP != m_bSavedCCPCP) {
		CString strOld, strNew;
		if(bCCPCP) {
			strOld = "No";
			strNew = "Yes";
		}
		else {
			strOld = "Yes";
			strNew = "No";
		}
		strSql += FormatString(" CC_PCP = %i, ", bCCPCP ? 1 : 0);
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabCCPCP, m_nLabID, strOld, strNew, aepMedium, aetChanged);
	}

	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	CString strLoincCode;
	GetDlgItemText(IDC_LOINC_CODE, strLoincCode);
	if(strLoincCode != m_strSavedLoincCode) {
		strSql += FormatString(" LOINC_Code = '%s', ", _Q(strLoincCode));
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabLoincCode, m_nLabID, m_strSavedLoincCode, strLoincCode, aepMedium, aetChanged);
	}
	CString strLoincDescription;
	GetDlgItemText(IDC_LOINC_DESCRIPTION, strLoincDescription);
	if(strLoincDescription != m_strSavedLoincDescription) {
		strSql += FormatString(" LOINC_Description = '%s', ", _Q(strLoincDescription));
		AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabLoincDescription, m_nLabID, m_strSavedLoincDescription, strLoincDescription, aepMedium, aetChanged);
	}

	BOOL bFireLabsTableChecker = FALSE; // (c.haag 2010-07-19 15:56) - PLID 30894
	if(!strSql.IsEmpty()){
		bFireLabsTableChecker = TRUE;
		strSql = "UPDATE LabsT SET " + strSql;
		//Trim the last ", "
		strSql.TrimRight(", ");
		CString strWhere;
		strWhere.Format(" WHERE ID = %li", m_nLabID);
		strSql += strWhere;
	}

	// (z.manning 2009-05-27 15:51) - PLID 34340 - Save problems
	//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
	SaveProblems(nAuditTransID, arNewCDSInterventions);

	if (m_bChangedProvider) {

		bFireLabsTableChecker = TRUE; // (c.haag 2010-07-28 13:40) - PLID 30894

		CString strProvs = "";
		SortDWordArray(m_dwProviders);
		strProvs = GetIDStringFromArray(&m_dwProviders);

		//see if our strings are the same
		if (m_strProvStartString == strProvs) {

			//there is nothing to save
		}
		else {

			//the strings aren't the same, so we have to change everything up
			//first, we need to delete every provider on this lab already
			CString strDelete;
			strDelete.Format("DELETE FROM LabMultiProviderT WHERE LabID = %li \r\n", m_nLabID);
			strSql += strDelete;

			//now we have to add all the new providers back in
			CString strSave = "", strTmp;
			long nCount = m_dwProviders.GetSize();
			for (int i = 0; i < nCount; i++) {
				strTmp.Format("INSERT INTO LabMultiProviderT (LabID, ProviderID) VALUES (%li, %li)\r\n", m_nLabID, m_dwProviders.GetAt(i));

				strSave += strTmp;
			}

			strSql += strSave;

			_RecordsetPtr rsOld = CreateRecordset("SELECT dbo.GetLabProviderString(%li) as OldProvString", m_nLabID);
			CString strOldNames, strNewNames;
			if (!rsOld->eof) {
				strOldNames = AdoFldString(rsOld, "OldProvString", "");
			}
			rsOld->Close();
			
			_RecordsetPtr rsNew = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name "
				" FROM PersonT WHERE ID IN (%s) ORDER BY Last, First, Middle ASC", strProvs);
			while (! rsNew->eof) {
				//(e.lally 2007-08-01) Made this a semicolon delimited list
				strNewNames += AdoFldString(rsNew, "Name", "") + "; ";
				rsNew->MoveNext();
			}
			rsNew->Close();

			//take off the last semicolon
			strNewNames = strNewNames.Left(strNewNames.GetLength() - 2);
			
			if (strOldNames.IsEmpty()) {
				strOldNames = "<No Provider>";
			}

			if (strNewNames.IsEmpty()){
				strNewNames = "<No Provider>";
			}

			AuditEvent(m_nPatientID, strName, nAuditTransID, aeiPatientLabProvider, m_nLabID, strOldNames, strNewNames, aepMedium, aetChanged);
		}

	}

	// (c.haag 2010-12-15 16:58) - PLID 41825 - Lab requisitions themselves can no longer be completed; use
	// functions to get the completed status from the results
	// (c.haag 2011-01-27) - PLID 41825 - This behavior has again changed. We no longer relate lab completions to step completions
	/*long nLabCompletedBy;
	COleDateTime dtLabCompletedDate;
	BOOL bCompleteSteps = FALSE;
	if(m_pLabEntryDlg->IsLabCompleted(m_nLabID, nLabCompletedBy, dtLabCompletedDate))
	{
		CString strUpdateLabSteps;
		strUpdateLabSteps.Format("UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li "
			"WHERE LabID = %li;", FormatDateTimeForSql(dtLabCompletedDate), nLabCompletedBy, m_nLabID);
		strSql += strUpdateLabSteps;
		// (z.manning 2008-10-13 16:34) - PLID 31667 - We are marking steps complete so we need 
		// to make sure we update todos when we commit this save.
		bNeedToUpdateTodos = TRUE;
		bFireLabsTableChecker = TRUE; // (c.haag 2010-07-28 13:40) - PLID 30894
	}*/

	if(bEmrDataChanged) {
		// (z.manning 2008-10-08 10:21) - PLID 31613 - Update any EMR details that pull data from this lab
		strSql += 
			FormatString(
			"UPDATE EmrDetailsT SET Text = '%s' \r\n"
			"FROM EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
			"WHERE EmrDetailsT.LabID = %li AND EmrMasterT.Status <> 2 \r\n"
			, _Q(emnlab.GetText()), m_nLabID);
	}

	//Run the update, insert and delete queries
	if(!strSql.IsEmpty()) {
		// (z.manning 2008-10-08 10:43) - PLID 31617 - This needs to be ExecuteSqlStd
		ExecuteSqlStd(strSql);

		if(bNeedToUpdateTodos) {
			// (z.manning 2008-10-13 16:34) - PLID 31667 - Make sure we update any todos for this lab
			//TES 1/5/2011 - PLID 37877 - Pass in the patient ID
			SyncTodoWithLab(m_nLabID, m_nPatientID);
		}

		if (bEmrDataChanged) {
			// (b.cardillo 2009-06-04 17:32) - PLID 34370 - Normally we'd need to update the patient wellness qualification 
			// data here, but in dev testing I realized it could never have any effect because at the moment there's no 
			// way to configure wellness templates to have criteria based on lab results.  That made this code a needless 
			// performance hit, so I removed it.  If we ever add the ability to use lab results, we'll need to add back a 
			// call to UpdatePatientWellnessQualification_EMRDetails() here.
		}
	}

	// (z.manning 2009-05-26 16:01) - PLID 34340 - Save problems
	//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
	SaveProblems(nAuditTransID, arNewCDSInterventions);
	
	// (r.gonet 09/21/2011) - PLID 45124 - Lab Custom Fields saved separately. Saves any updates to custom field instances.
	if(m_pcfTemplateInstance) {
		m_pcfTemplateInstance->Save();
	}
	
	// (z.manning 2008-10-08 11:40) - PLID 31613 - Remember the last saved existing lab.
	//TES 12/17/2009 - PLID 36622 - This is now an array.
	bool bFound = false;
	for(int i = 0; i < m_pLabEntryDlg->m_arSavedExistingLabs.GetSize() && !bFound; i++) {
		if(m_pLabEntryDlg->m_arSavedExistingLabs[i].nID == m_nLabID) {
			//TES 12/18/2009 - PLID 36622 - Remove this from the array, we now have a newer version of this lab.
			m_pLabEntryDlg->m_arSavedExistingLabs.RemoveAt(i);
			bFound = true;
		}
	}
	m_pLabEntryDlg->m_arSavedExistingLabs.Add(emnlab);

	// (j.jones 2010-08-30 09:05) - PLID 40095 - need to update the new lab list as well,
	// if this lab exists in the new lab list
	bFound = false;
	for(int i = 0; i < m_pLabEntryDlg->m_aryNewLabs.GetSize() && !bFound; i++) {
		if(m_pLabEntryDlg->m_aryNewLabs[i].nID == m_nLabID) {
			//Remove this from the array, we now have a newer version of this lab.
			m_pLabEntryDlg->m_aryNewLabs.RemoveAt(i);
			bFound = true;
		}
	}
	if(bFound) {
		m_pLabEntryDlg->m_aryNewLabs.Add(emnlab);
	}

	// (z.manning 2010-05-05 10:46) - PLID 37190 - Update the stored value of last saved initial diagnosis
	m_strSavedInitialDiagnosis = strNewInitialDiagnosis;

	//TES 9/29/2010 - PLID 40644 - Also the location, provider(s), lab location, and comments
	m_nSavedLocationID = nNewLocationID;
	m_dwSavedProviderIDs.Copy(m_dwProviders);
	m_strSavedProviderNames = m_strProviderNames;
	m_nSavedLabLocationID = nNewLabLocationID;
	m_strSavedClinicalData = strNewClinicalData;

	// (j.jones 2010-10-07 13:20) - PLID 40743 - update several more "last saved" values
	m_pLabEntryDlg->SetSavedFormNumber(strNewFormNumber);
	m_strSavedSpecimen = strNewSpecimen;	
	m_strSavedInstructions = strInstructions;
	m_strSavedToBeOrdered = strNewToBeOrdered;
	m_nMedAssistantID = nNewMedID;
	m_nInsuredPartyID = nNewInsuredPartyID;	
	m_nSavedAnatomyID = nNewAnatomyID;
	m_nSavedAnatomySide = nNewAnatomySide;
	m_nSavedAnatomyQualifierID = nNewQualifierID;
	m_dtSavedBiopsyDate = dtNewBiopsyDate;	
	m_bSavedMelanomaHistory = bMelanomaHistory;
	m_bSavedPreviousBiopsy = bPreviousBiopsy;
	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	m_bSavedCCPatient = bCCPatient;
	m_bSavedCCRefPhys = bCCRefPhys;
	m_bSavedCCPCP = bCCPCP;
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	m_strSavedLoincCode = strLoincCode;
	m_strSavedLoincDescription = strLoincDescription;

	// (c.haag 2010-07-19 15:55) - PLID 30894 - Fire a table checker if the LabsT record changed
	if (bFireLabsTableChecker) {
		// (r.gonet 08/25/2014) - PLID 63221 - Send the patient ID as well
		CClient::RefreshLabsTable(m_nPatientID, m_nLabID);
	}
}

BOOL CLabRequisitionDlg::IsLabValid()
{
	// (r.gonet 10/14/2011) - PLID 45124 - We need to save and close the custom fields dlg if it is open
	if(m_pFieldsDlg && m_pFieldsDlg->GetSafeHwnd() != NULL && m_pFieldsDlg->IsWindowVisible()) {
		if(IDYES == MessageBox("The Additional Fields window must be closed first. Save and close the Additional Fields window?",
			NULL, MB_YESNO|MB_ICONQUESTION)) 
		{
			if(!m_pFieldsDlg->SaveAndClose()) {
				m_pFieldsDlg->BringWindowToTop();
				return FALSE;
			}
		} else {
			return FALSE;
		}
	}

	//Provider
	if (m_dwProviders.GetSize() == 0) {
		MessageBox("This lab cannot be saved because you must select a Provider first.");
		return FALSE;
	}


	//Location
	IRowSettingsPtr pLoc = m_pLocationCombo->GetCurSel();
	if(m_pLocationCombo->GetIsComboBoxTextInUse() == FALSE){
		if(pLoc == NULL || VarLong(pLoc->GetValue(lccLocationID)) <= 0){
			MessageBox("This lab cannot be saved because you must select a Location first.");
			return FALSE;
		}
	}
	//Biopsy Date
	if(m_dtpBiopsy.GetValue().vt != VT_NULL){
		//We just want to compare the date part in case the time got saved too.
		COleDateTime dtTemp, dtBiopsyVal;
		dtTemp = VarDateTime(m_dtpBiopsy.GetValue());
		dtBiopsyVal.SetDate(dtTemp.GetYear(), dtTemp.GetMonth(), dtTemp.GetDay());

		//TES 7/16/2010 - PLID 39575 - Check if the Biopsy Time is in an invalid state.
		if(m_nxtBiopsyTime->GetStatus() == 2) {
			MessageBox("This lab cannot be saved because you have entered an invalid Time value");
			return FALSE;
		}

		// (j.jones 2010-05-25 09:33) - PLID 38862 - removed the limitation of having requisition dates in the future
		/*
		if(dtBiopsyVal > dtCurDate){
			// (z.manning 2008-10-30 12:05) - PLID 31864 - Renamed biopsy date to date in the message
			MessageBox("You cannot have a date that is in the future.");
			return FALSE;
		}
		*/
	}
	//Receiving Lab
	IRowSettingsPtr pRecLab = m_pReceivingLabCombo->GetCurSel();
	if(m_pReceivingLabCombo->GetIsComboBoxTextInUse() == FALSE){
		if(pRecLab == NULL || VarLong(pRecLab->GetValue(rlcLabLocationID)) <= 0){
			MessageBox("This lab cannot be saved because you must select a Receiving Lab first.");
			return FALSE;
		}
	}
	
	//Medical Assistant user
	IRowSettingsPtr pMedAssistant = m_pMedAssistantCombo->GetCurSel();
	if(m_pMedAssistantCombo->GetIsComboBoxTextInUse() == FALSE){
		if(pMedAssistant == NULL){
			MessageBox("This lab cannot be saved because you must select a Medical Assistant first.");
			return FALSE;
		}
	}

	//Insured Party
	IRowSettingsPtr pInsParty = m_pInsuranceCombo->GetCurSel();
	if(m_pInsuranceCombo->GetIsComboBoxTextInUse() == FALSE){
		if(pInsParty == NULL){
			MessageBox("This lab cannot be saved because you must select insurance first.");
			return FALSE;
		}
	}

	//Anatomic Location
	// (r.galicki 2008-10-20 12:29) - PLID 31552 Only need to save Anatomic location if its Biopsy lab
	// (z.manning 2011-08-05 13:35) - PLID 39978 - Only show this warning for the currently visible req.
	// and do not show it at all on existing labs if the results tab is active.
	if(m_ltType == ltBiopsy && (IsWindowVisible() || IsNew()))
	{
		IRowSettingsPtr pAnatLoc = m_pAnatomyCombo->GetCurSel();
		//TES 11/6/2009 - PLID 36189 - It may be inactive
		if((pAnatLoc == NULL || VarLong(pAnatLoc->GetValue(alcAnatomyID)) <= 0) && !m_pAnatomyCombo->IsComboBoxTextInUse ){
			//TES 12/3/2008 - PLID 32302 - We no longer require an anatomic location, but they still really
			// ought to have one, so make sure they're doing this intentionally.
			if(IDYES != MsgBox(MB_YESNO, "You have not selected an Anatomic Location, are you sure you wish to continue?")) {
				return FALSE;
			}
		}
	}

	// (z.manning 2009-05-29 14:08) - PLID 29911 - Added preference to require problems
	if(GetRemotePropertyInt("LabProblemsRequired", 0, 0, "<None>"))
	{
		if(!HasProblems()) {
			MessageBox("This lab cannot be saved because it must be linked to at least one problem.");
			return FALSE;
		}
	}

	// (r.gonet 09/21/2011) - PLID 45124 - Check if all of the required custom fields have been filled in.
	if(m_pcfTemplateInstance) {
		CArray<CLabCustomFieldPtr, CLabCustomFieldPtr> aryUnsetRequiredFields;
		m_pcfTemplateInstance->GetUnsetRequiredFields(aryUnsetRequiredFields);
		if(aryUnsetRequiredFields.GetSize() > 0) {
			CString str = 
				"Not all of the required fields in Additional Fields have been filled in. Are you sure you want to save?\r\n"
				"\r\n"
				"The following fields are required but not filled in:\r\n";

			for(int i = 0; i < min(9,aryUnsetRequiredFields.GetSize()); i++) {
				str += FormatString("* %s\r\n", aryUnsetRequiredFields[i]->GetFriendlyName());
			}
			if(i < aryUnsetRequiredFields.GetSize()) {
				str += FormatString("%li More...\r\n", aryUnsetRequiredFields.GetSize() - i);
			}
			str = str.Left(str.GetLength() - 2);
			if(IDYES != MessageBox(str, 0, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}
void CLabRequisitionDlg::OnEditAnatomy()
{
	try{
		//(e.lally 2006-07-07) PLID 21356 - Added anatomic locations to the edit combo box supported objects.
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 14, "Edit Anatomic Location List");

		//TES 11/6/2009 - PLID 36189 - Don't pass in the datalist, since it filters out inactive
		IRowSettingsPtr pRow = m_pAnatomyCombo->GetCurSel();
		if (pRow != NULL) {
			m_nCurAnatomyID = VarLong(pRow->GetValue(alcAnatomyID),-1);

			// (j.jones 2007-07-20 12:13) - PLID 26749 - set the
			// current anatomy ID, if we have one
			if(m_nCurAnatomyID > 0)
				dlg.m_nCurIDInUse = m_nCurAnatomyID;
		}
		else {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			dlg.m_nCurIDInUse = m_nCurAnatomyID;
		}

		dlg.DoModal();

		//TES 11/6/2009 - PLID 36189 - EditComboBox won't requery automatically for us any more.
		m_pAnatomyCombo->Requery();

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pAnatomyCombo->TrySetSelByColumn_Deprecated(0, m_nCurAnatomyID);
	}NxCatchAll("Error in CLabRequisitionDlg::OnEditAnatomy()");
}
BEGIN_EVENTSINK_MAP(CLabRequisitionDlg, CNxDialog)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_LOCATION, 20, CLabRequisitionDlg::OnTrySetSelFinishedLabLocation, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_RECEIVING_LAB, 20, CLabRequisitionDlg::OnTrySetSelFinishedReceivingLab, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_ANATOMIC_LOCATION, 20, CLabRequisitionDlg::OnTrySetSelFinishedAnatomicLocation, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_MEDASSISTANT, 20, CLabRequisitionDlg::OnTrySetSelFinishedLabMedassistant, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_INSURANCE, 20, CLabRequisitionDlg::OnTrySetSelFinishedLabInsurance, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_PROVIDER, 18, CLabRequisitionDlg::OnRequeryFinishedLabProvider, VTS_I2)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_PROVIDER, 16, CLabRequisitionDlg::OnSelChosenLabProvider, VTS_DISPATCH)
	ON_EVENT(CLabRequisitionDlg, IDC_LABS_TO_BE_ORDERED, 16, CLabRequisitionDlg::OnSelChosenLabsToBeOrdered, VTS_DISPATCH)
	ON_EVENT(CLabRequisitionDlg, IDC_ANATOMIC_LOCATION, 18, CLabRequisitionDlg::OnRequeryFinishedAnatomicLocation, VTS_I2)
	ON_EVENT(CLabRequisitionDlg, IDC_ANATOMIC_QUALIFIER, 18, CLabRequisitionDlg::OnRequeryFinishedAnatomicQualifier, VTS_I2)
	ON_EVENT(CLabRequisitionDlg, IDC_ANATOMIC_QUALIFIER, 20, CLabRequisitionDlg::OnTrySetSelFinishedAnatomicQualifier, VTS_I4 VTS_I4)
	ON_EVENT(CLabRequisitionDlg, IDC_LAB_PROVIDER, 1, CLabRequisitionDlg::OnSelChangingLabProvider, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabRequisitionDlg, IDC_RECEIVING_LAB, 16, CLabRequisitionDlg::OnSelChosenReceivingLab, VTS_DISPATCH)
	ON_EVENT(CLabRequisitionDlg, IDC_BIOPSY_TYPE, 20, CLabRequisitionDlg::TrySetSelFinishedBiopsyType, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CLabRequisitionDlg::OnTrySetSelFinishedLabLocation(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive location
			_RecordsetPtr rsLoc = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LocationID FROM LabsT WHERE ID = %li)", m_nLabID);
			if(!rsLoc->eof) {
				m_pLocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else 
				m_pLocationCombo->PutCurSel(m_pLocationCombo->GetFirstRow());
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedLocation()");
}

void CLabRequisitionDlg::OnTrySetSelFinishedReceivingLab(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive receiving lab
			_RecordsetPtr rsRecLab = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LabLocationID FROM LabsT WHERE ID = %li)", m_nLabID);
			if(!rsRecLab->eof) {
				m_pReceivingLabCombo->PutComboBoxText(_bstr_t(AdoFldString(rsRecLab, "Name", "")));
			}
			else 
				m_pReceivingLabCombo->PutCurSel(m_pReceivingLabCombo->GetFirstRow());
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedReceivingLab()");
}

void CLabRequisitionDlg::OnTrySetSelFinishedAnatomicLocation(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			if(m_nCurAnatomyID > 0) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Description FROM LabAnatomyT WHERE ID = {INT}", m_nCurAnatomyID);
				if(!rsAnat->eof) {
					m_pAnatomyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Description", "")));
					return;
				}
			}
			//Else if it failed to load anything, check if we need to clear the combo box text
			if(m_pAnatomyCombo->GetIsComboBoxTextInUse() != FALSE)
				m_pAnatomyCombo->PutComboBoxText("");
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedAnatomicLocation()");
}

void CLabRequisitionDlg::OnTrySetSelFinishedLabMedassistant(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			// (a.walling 2006-07-25 11:44) - PLID 21571 - Write the inactive name in the combo
			_RecordsetPtr rsMed = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First AS Name FROM PersonT WHERE ID = %li", m_nMedAssistantID);
			if(!rsMed->eof) {
				m_pMedAssistantCombo->PutComboBoxText(_bstr_t(AdoFldString(rsMed, "Name", "")));
			}
			//Else if it failed to load anything, check if we need to clear the combo box text
			else {
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				if(m_pMedAssistantCombo->GetIsComboBoxTextInUse() == FALSE)
					m_pMedAssistantCombo->TrySetSelByColumn_Deprecated(malcUserID, _variant_t((long)-1));
			}
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedMedAssistant()");	
}

void CLabRequisitionDlg::OnTrySetSelFinishedLabInsurance(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			if (m_nInsuredPartyID > 0) {
				_RecordsetPtr rsParty = CreateRecordset("SELECT InsuranceCoT.Name AS InsuranceCoName FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID WHERE InsuredPartyT.PersonID = %li", m_nInsuredPartyID);
				if(!rsParty->eof) {
					m_pInsuranceCombo->PutComboBoxText(_bstr_t(AdoFldString(rsParty, "InsuranceCoName", "")));
					return;
				}
			}
			else { // this is a new lab, so select primary if possible.
				if (GetRemotePropertyInt("LabChoosePrimary", 1, 0, "<None>", true) == 1) {
					IRowSettingsPtr pRow = m_pInsuranceCombo->FindByColumn(ilcType, _bstr_t("Primary"), m_pInsuranceCombo->GetFirstRow(), true);
					if (pRow) {
						m_pInsuranceCombo->PutCurSel(pRow);
					}
					else {
						if (m_pInsuranceCombo->GetIsComboBoxTextInUse() == FALSE) {
							// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
							m_pInsuranceCombo->TrySetSelByColumn_Deprecated(ilcInsuredPartyID, _variant_t((long)-1));
						}
					}
				}
				else if (m_pInsuranceCombo->GetIsComboBoxTextInUse() == FALSE) {
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					m_pInsuranceCombo->TrySetSelByColumn_Deprecated(ilcInsuredPartyID, _variant_t((long)-1));
				}
			}
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedInsurance()");
}

void CLabRequisitionDlg::OnRequeryFinishedLabProvider(short nFlags)
{
	try {
		//make the {Multiple Providers} selection
	 
		NXDATALIST2Lib::IRowSettingsPtr  pRow;

		pRow = m_pProviderCombo->GetNewRow();

		pRow->PutValue(0, (long) -2);
		pRow->PutValue(1, _variant_t(""));
		pRow->PutValue(2, _variant_t(""));
		pRow->PutValue(3, _variant_t(""));
		pRow->PutValue(4, _variant_t("{Multiple Providers}"));

		m_pProviderCombo->AddRowBefore(pRow, m_pProviderCombo->GetFirstRow());

	}NxCatchAll("Error in CLabRequisitionDlg::OnRequeryFinishedLabProvider()");
}

void CLabRequisitionDlg::OnSelChosenLabProvider(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr  pRow(lpRow);

		if (pRow) {

			m_bChangedProvider = TRUE;

			long nID = pRow->GetValue(0);
			if (nID == -2) {

				OnMultiSelectProvs();
			}
			else {
				m_dwProviders.RemoveAll();
				m_dwProviders.Add(nID);
				//TES 9/29/2010 - PLID 40644 - Need to remember the names
				m_strProviderNames = VarString(pRow->GetValue(pccFullName));
			}
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnSelChosenLabProvider()");
}

LRESULT CLabRequisitionDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_LAB_MULTI_PROV:
			OnMultiSelectProvs();
			break;
		//TES 7/12/2011 - PLID 44107 - Added LOINC fields
		case IDC_LOINC_CODE_LABEL:
		{
			CString strPreviousCode, strPreviousDescription;
			GetDlgItemText(IDC_LOINC_CODE, strPreviousCode);
			GetDlgItemText(IDC_LOINC_DESCRIPTION, strPreviousDescription);
			// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
			CEditComboBox dlg(this, 71, "Select or Edit LOINC Codes");
			if (!strPreviousCode.IsEmpty()) {
				dlg.m_varInitialSel = _variant_t(strPreviousCode);
			}
			if (IDOK == dlg.DoModal()) {
				if (dlg.m_nLastSelID != -1) {
					_RecordsetPtr prsLOINC = CreateParamRecordset("SELECT Code, ShortName FROM LabLOINCT WHERE ID = {INT}", dlg.m_nLastSelID);
					if (!prsLOINC->eof) {
						CString strCode = AdoFldString(prsLOINC, "Code");
						CString strName = AdoFldString(prsLOINC, "ShortName");

						if (strCode != strPreviousCode || strName != strPreviousDescription ) {

							if (!strPreviousCode.IsEmpty() || !strPreviousDescription.IsEmpty()) {
								CString strPreviousCodeDesc;
								if (strPreviousCode.IsEmpty()) {
									strPreviousCodeDesc.Format("%s", strPreviousDescription);
								} else {
									strPreviousCodeDesc.Format("%s (%s)", strPreviousDescription, strPreviousCode);
								}
								CString strNewCodeDesc;
								if (strCode.IsEmpty()) {
									strNewCodeDesc.Format("%s", strName);
								} else {
									strNewCodeDesc.Format("%s (%s)", strName, strCode);
								}
								if (IDNO == MessageBox(FormatString("Are you sure you want to replace the current order code and description:\r\n"
									"\r\n"
									"%s\r\n"
									"\r\n"
									"with:\r\n"
									"\r\n"
									"%s?", strPreviousCodeDesc, strNewCodeDesc), NULL, MB_YESNO | MB_ICONQUESTION)) {
									return 0;
								}
							}

							SetDlgItemText(IDC_LOINC_CODE, strCode);
							SetDlgItemText(IDC_LOINC_DESCRIPTION, strName);
						}
					}
				}
			}
		}
		break;

		// (j.jones 2013-10-22 12:56) - PLID 58979 - added patient education link
		case IDC_PT_EDUCATION_LABEL: {
			//force a save
			if(!Save()){
				return 0;
			}

			if(m_nLabID == -1) {
				//should not be possible, because we forced a save
				ASSERT(FALSE);
				ThrowNxException("No valid lab ID was found.");
			}

			// (r.gonet 10/30/2013) - PLID 58980 - The patient education hyperlink goes to the Medline Plus website
			LookupMedlinePlusInformationViaSearch(this, mlpLabID, m_nLabID);
			break;
			}

		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CLabRequisitionDlg::OnMultiSelectProvs()
{
	try {

		m_bChangedProvider = TRUE;

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		dlg.PreSelect(m_dwProviders);
		CString strFrom, strWhere;
		strFrom = "	(SELECT PersonT.Archived, PersonT.ID, PersonT.Last, PersonT.First, PersonT.Middle, (PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle) AS FullName FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS Doctors";
		//just in case
		CString strIDs = GetIDStringFromArray(&m_dwProviders);
		strIDs.TrimLeft();
		strIDs.TrimRight();
		if (strIDs.IsEmpty()) {
//			ASSERT(FALSE);
			strWhere = " Archived = 0 ";
		}
		else {
			strWhere.Format("Archived = 0 OR ID IN (%s)", strIDs);
		}

		if(IDOK == dlg.Open(strFrom, strWhere, "ID", "FullName", "Select one or more providers:", 1)) {
			dlg.FillArrayWithIDs(m_dwProviders);
		
			if(m_dwProviders.GetSize() > 1) {
				ShowDlgItem(IDC_LAB_PROVIDER, SW_HIDE);
				//TES 9/29/2010 - PLID 40644 - Need to remember the names
				m_strProviderNames = dlg.GetMultiSelectString();
				m_nxlProviderLabel.SetText(m_strProviderNames);
				m_nxlProviderLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_LAB_MULTI_PROV, SW_SHOW);
				InvalidateDlgItem(IDC_LAB_MULTI_PROV);
			}
			else if(m_dwProviders.GetSize() == 1) {
				//They selected exactly one.
				ShowDlgItem(IDC_LAB_MULTI_PROV, SW_HIDE);
				ShowDlgItem(IDC_LAB_PROVIDER, SW_SHOW);
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				pRow = m_pProviderCombo->SetSelByColumn(pccProviderID,(long)m_dwProviders.GetAt(0));
				if(pRow) {
					//TES 9/29/2010 - PLID 40644 - Need to remember the names
					m_strProviderNames = VarString(pRow->GetValue(pccFullName));
				}
				else {
					//they may have an inactive provider
					_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li ORDER BY Last, First, Middle ASC", m_dwProviders.GetAt(0));
					if(!rsProv->eof) {
						//TES 9/29/2010 - PLID 40644 - Need to remember the names
						m_strProviderNames = AdoFldString(rsProv, "Name", "");
						m_pProviderCombo->PutComboBoxText(_bstr_t(m_strProviderNames));
					}
					else{
						//if its not inactive and doesn't exist in the list, then where did it come from?
						ASSERT(FALSE);
						m_pProviderCombo->PutCurSel(NULL);

						//clear out the array, just to be sure
						m_dwProviders.RemoveAll();
					}
				}
				
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			//Check if they have "multiple" selected
			if(m_dwProviders.GetSize() > 1) {
				ShowDlgItem(IDC_LAB_PROVIDER, SW_HIDE);
				CString strIDList, strID, strProvList;
				for(int i=0; i < m_dwProviders.GetSize(); i++) {
					strID.Format("%li, ", (long)m_dwProviders.GetAt(i));
					strIDList += strID;
				}
				strIDList = strIDList.Left(strIDList.GetLength()-2);
				_RecordsetPtr rsProvs = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN (%s) ORDER BY Last, First, Middle ASC", strIDList);
				while(!rsProvs->eof) {
					strProvList += AdoFldString(rsProvs, "Name") + ", ";
					rsProvs->MoveNext();
				}
				rsProvs->Close();
				strProvList = strProvList.Left(strProvList.GetLength()-2);
				m_nxlProviderLabel.SetText(strProvList);
				m_nxlProviderLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_LAB_MULTI_PROV, SW_SHOW);
				InvalidateDlgItem(IDC_LAB_MULTI_PROV);
			}
			else {
				//They selected exactly one.
				ShowDlgItem(IDC_LAB_MULTI_PROV, SW_HIDE);
				ShowDlgItem(IDC_LAB_PROVIDER, SW_SHOW);
				if (m_dwProviders.GetSize() > 0) {
					NXDATALIST2Lib::IRowSettingsPtr pRow;
					pRow = m_pProviderCombo->SetSelByColumn(pccProviderID,(long)m_dwProviders.GetAt(0));
					if(pRow) {

					}
					else {
						//they may have an inactive provider
						_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li ORDER BY Last, First, Middle ASC", m_dwProviders.GetAt(0));
						if(!rsProv->eof) {
							m_pProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						}
						else{
							//This should not happen. What now?
							//ASSERT(FALSE);
							m_pProviderCombo->PutCurSel(NULL);
							m_dwProviders.RemoveAll();
						}
					}
					
				}
				else {
					//I'm not sure this could happen, but let's handle it anyway
					m_pProviderCombo->PutCurSel(NULL);
					m_dwProviders.RemoveAll();
					GetDlgItem(IDC_LAB_MULTI_PROV)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_LAB_PROVIDER)->ShowWindow(SW_SHOW);
				}
			}
		}
	}NxCatchAll("CLabRequisitionDlg::OnMultiSelectProvs()");
}

void CLabRequisitionDlg::OnSelChosenLabsToBeOrdered(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			ASSERT(FALSE);
		}

		CString strToBeOrdered;
		m_nxeditToBeOrderedText.GetWindowText(strToBeOrdered);
		if(!strToBeOrdered.IsEmpty()) {
			strToBeOrdered += ", ";
		}
		strToBeOrdered += VarString(pRow->GetValue(tbolcDescription), "");
		if(strToBeOrdered.GetLength() > 1000) {
			strToBeOrdered = strToBeOrdered.Left(1000);
		}

		m_nxeditToBeOrderedText.SetWindowText(strToBeOrdered);

		// (c.haag 2009-05-07 11:05) - PLID 28550 - See whether there are any instructions
		// that we need to pop up for this order
		CString strDisplayInstructions = VarString(pRow->GetValue(tbolcDisplayInstructions), "");
		strDisplayInstructions.TrimLeft();
		strDisplayInstructions.TrimRight();
		if (!strDisplayInstructions.IsEmpty()) {
			AfxMessageBox(strDisplayInstructions, MB_ICONINFORMATION);
		}

		// (c.haag 2009-05-11 14:40) - PLID 28515 - Append the instructions to the instruction field
		if (!strDisplayInstructions.IsEmpty()) {
			CString strInstructions;
			m_nxeditInstructions.GetWindowText(strInstructions);
			if(!strInstructions.IsEmpty()) {
				strInstructions += ", ";
			}
			strInstructions += strDisplayInstructions;
			if(strInstructions.GetLength() > 5000) {
				strInstructions = strInstructions.Left(5000);
			}

			m_nxeditInstructions.SetWindowText(strInstructions);
		}

		//TES 7/12/2011 - PLID 44538 - See if this test has an associated LOINC code
		long nLOINCID = VarLong(pRow->GetValue(tbolcLOINCID), -1);
		if(nLOINCID > 0) {
			//TES 7/12/2011 - PLID 44538 - There is, load it
			_RecordsetPtr rsLOINC = CreateParamRecordset("SELECT Code, ShortName FROM LabLOINCT WHERE ID = {INT}", nLOINCID);
			ASSERT(!rsLOINC->eof);
			CString strCode = AdoFldString(rsLOINC, "Code");
			CString strName = AdoFldString(rsLOINC, "ShortName");
			//TES 7/12/2011 - PLID 44538 - Do we already have a code and description filled in?
			bool bCopyCode = true;
			CString strCurrentCode;
			GetDlgItemText(IDC_LOINC_CODE, strCurrentCode);
			CString strCurrentDescription;
			GetDlgItemText(IDC_LOINC_DESCRIPTION, strCurrentDescription);
			if( (!strCurrentCode.IsEmpty() && strCurrentCode != strCode) || (!strCurrentDescription.IsEmpty() && strCurrentDescription != strName)) {
				if(IDYES != MsgBox(MB_YESNO, "The test you have selected is associated with the LOINC Code %s - %s.  Would you like to replace your current "
					"code, %s - %s, with the one associated with this test?", strCode, strName, strCurrentCode, strCurrentDescription)) {
						bCopyCode = false;
				}
			}

			if(bCopyCode) {
				SetDlgItemText(IDC_LOINC_CODE, strCode);
				SetDlgItemText(IDC_LOINC_DESCRIPTION, strName);
			}
		}

		// (j.jones 2010-06-28 09:08) - PLID 39185 - clear the selection as it is now meaningless
		m_pToBeOrderedCombo->PutCurSel(NULL);

	}NxCatchAll("CLabRequisitionDlg::OnSelChosenLabsToBeOrdered");
}

void CLabRequisitionDlg::OnEditToBeOrdered()
{
	try {

		// (j.jones 2010-06-25 16:10) - PLID 39185 - changed to open the dedicated setup for this list

		long nRecLabLocID = -1;
		IRowSettingsPtr pRow = m_pReceivingLabCombo->GetCurSel();
		if(pRow) {
			nRecLabLocID = VarLong(pRow->GetValue(rlcLabLocationID),-1);
		}

		CLabsToBeOrderedSetupDlg dlg(this);
		dlg.m_nDefaultLocationID = nRecLabLocID;
		dlg.DoModal();

		//always requery, we did this before as well, because the
		//selection does not need to be maintained
		m_pToBeOrderedCombo->Requery();

	}NxCatchAll("CLabRequisitionDlg::OnEditToBeOrdered");
}

void CLabRequisitionDlg::OnRequeryFinishedAnatomicLocation(short nFlags)
{
	try {
		if(m_nCurAnatomyID != -1) {
			if (NULL == m_pAnatomyCombo->FindByColumn(0, m_nCurAnatomyID, NULL, VARIANT_TRUE)) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Description FROM LabAnatomyT WHERE ID = {INT}", m_nCurAnatomyID);
				if(!rsAnat->eof) {
					m_pAnatomyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Description", "")));
				}
				else {
					//This should not happen. What now?
					ASSERT(FALSE);
					m_pAnatomyCombo->PutCurSel(m_pAnatomyCombo->GetFirstRow());
				}	
			}
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnRequeryFinishedAnatomicLocation()");
}

void CLabRequisitionDlg::OnEditAnatomicQualifiers()
{
	try {
		//TES 11/10/2009 - PLID 36128 - Replaced the Left/Right checkboxes with an editable list, function copied from OnEditAnatomyList().
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 70, "Edit Anatomic Location Qualifier List");

		//TES 11/6/2009 - PLID 36189 - Don't pass in the datalist, since it filters out inactive
		IRowSettingsPtr pRow = m_pAnatomyQualifiersCombo->GetCurSel();
		if (pRow != NULL) {
			m_nCurAnatomyQualifierID = VarLong(pRow->GetValue(alqcID),-1);

			// (j.jones 2007-07-20 12:13) - PLID 26749 - set the
			// current anatomy ID, if we have one
			if(m_nCurAnatomyQualifierID > 0)
				dlg.m_nCurIDInUse = m_nCurAnatomyQualifierID;
		}
		else {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			dlg.m_nCurIDInUse = m_nCurAnatomyQualifierID;
		}

		dlg.DoModal();

		//TES 11/6/2009 - PLID 36189 - EditComboBox won't requery automatically for us any more.
		m_pAnatomyQualifiersCombo->Requery();

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pAnatomyQualifiersCombo->TrySetSelByColumn_Deprecated(0, m_nCurAnatomyQualifierID);

	}NxCatchAll("Error in CLabRequisitionDlg::OnEditAnatomicQualifiers()");
}

void CLabRequisitionDlg::OnRequeryFinishedAnatomicQualifier(short nFlags)
{
	try {
		//TES 11/10/2009 - PLID 36128 - Added, based on OnRequeryFinishedAnatomicLocation
		if(m_nCurAnatomyQualifierID != -1) {
			if (NULL == m_pAnatomyQualifiersCombo->FindByColumn(0, m_nCurAnatomyQualifierID, NULL, VARIANT_TRUE)) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", m_nCurAnatomyQualifierID);
				if(!rsAnat->eof) {
					m_pAnatomyQualifiersCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Name", "")));
				}
				else {
					//This should not happen. What now?
					ASSERT(FALSE);
					m_pAnatomyQualifiersCombo->PutCurSel(m_pAnatomyQualifiersCombo->GetFirstRow());
				}	
			}
		}

	}NxCatchAll("Error in CLabRequisitionDlg::OnRequeryFinishedAnatomicQualifier()");
}

void CLabRequisitionDlg::OnTrySetSelFinishedAnatomicQualifier(long nRowEnum, long nFlags)
{
	try {
		//TES 11/10/2009 - PLID 36128 - Added, based on OnTrySetSelFinishedAnatomicLocation
		if(nFlags == dlTrySetSelFinishedFailure) {
			if(m_nCurAnatomyQualifierID > 0) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				_RecordsetPtr rsAnat = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", m_nCurAnatomyQualifierID);
				if(!rsAnat->eof) {
					m_pAnatomyQualifiersCombo->PutComboBoxText(_bstr_t(AdoFldString(rsAnat, "Name", "")));
					return;
				}
			}
			//Else if it failed to load anything, check if we need to clear the combo box text
			if(m_pAnatomyQualifiersCombo->GetIsComboBoxTextInUse() != FALSE)
				m_pAnatomyQualifiersCombo->PutComboBoxText("");
		}
	}NxCatchAll("Error in CLabRequisitionDlg::OnTrySetSelFinishedAnatomicQualifier()");
}

void CLabRequisitionDlg::SetLabProcedureType(LabType ltType)
{
	m_ltType = ltType;
}

void CLabRequisitionDlg::SetLabID(long nLabID)
{
	m_nLabID = nLabID;
}

void CLabRequisitionDlg::SetDefaultToBeOrdered(const CString &strToBeOrdered)
{
	m_strDefaultToBeOrdered = strToBeOrdered;
}

void CLabRequisitionDlg::SetPatientID(long nPatientID)
{
	m_nPatientID = nPatientID;
}

// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
void CLabRequisitionDlg::SetDefaultLocationID(long nLocationID)
{
	m_nDefaultLocationID = nLocationID;
}

// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
void CLabRequisitionDlg::SetEMNProviders(CDWordArray &dwProviderIDs)
{
	m_dwEMNProviderIDs.Copy(dwProviderIDs);
}

void CLabRequisitionDlg::SetLabProcedureID(long nLabProcedureID)
{
	m_nLabProcedureID = nLabProcedureID;
}

void CLabRequisitionDlg::SetCurrentDate(_variant_t varCurDate)
{
	m_varCurDate = varCurDate;
}

COleDateTime CLabRequisitionDlg::GetBiopsyDate()
{
	//TES 7/16/2010 - PLID 39575 - We also have a time now.
	COleDateTime dtBiopsy = m_dtpBiopsy.GetValue();
	if(m_nxtBiopsyTime->GetStatus() == 1) {
		COleDateTime dtBiopsyTime = m_nxtBiopsyTime->GetDateTime();
		dtBiopsy.SetDateTime(dtBiopsy.GetYear(), dtBiopsy.GetMonth(), dtBiopsy.GetDay(), dtBiopsyTime.GetHour(), dtBiopsyTime.GetMinute(), dtBiopsyTime.GetSecond());
	}
	else {
		dtBiopsy.SetDateTime(dtBiopsy.GetYear(), dtBiopsy.GetMonth(), dtBiopsy.GetDay(), 0, 0, 0);
	}
	return dtBiopsy;
}

// (j.jones 2010-05-06 09:37) - PLID 38520 - added GetInputDate
COleDateTime CLabRequisitionDlg::GetInputDate()
{
	return m_dtInputDate;
}

void CLabRequisitionDlg::SortDWordArray(CDWordArray &dw) {

	CDWordArray dwSorted;
	long nCount = dw.GetSize();

	for (int i = 0; i < nCount; i++) {
		DWORD nItem = m_dwProviders.GetAt(i);

		//loop through the sorted one to see where we go
		if (dwSorted.GetSize() == 0 ) {
			dwSorted.InsertAt(0, nItem);
		}
		else {
			BOOL bInserted = FALSE;
			int j = 0;
			while (j < dwSorted.GetSize() && (!bInserted)) {
				if (nItem < dwSorted.GetAt(j)) {
					dwSorted.InsertAt(j, nItem);
					bInserted = TRUE;
				}
				else if (nItem == dwSorted.GetAt(j)) {
					dwSorted.InsertAt(j + 1, nItem);
					bInserted = TRUE;
				}
				j++;
			}
			if (! bInserted) {
				//insert at the end
				dwSorted.InsertAt(j, nItem);
			}
		}
	}

	//now remove it
	dw.RemoveAll();
	for (int h = 0; h < dwSorted.GetSize(); h++) {
		dw.Add(dwSorted.GetAt(h));
	}
}

BOOL CLabRequisitionDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_dwProviders.GetSize() > 1)
		{
			CRect rc;
			GetDlgItem(IDC_LAB_MULTI_PROV)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		//TES 7/12/2011 - PLID 44107 - Added LOINC fields
		GetDlgItem(IDC_LOINC_CODE_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if(rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		// (j.jones 2013-10-22 12:57) - PLID 58979 - added patient education link
		CRect rcPatEducation;
		m_nxlabelPatientEducation.GetWindowRect(rcPatEducation);
		ScreenToClient(&rcPatEducation);
		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Set the link cursor if we are in the pat education link and we are showing it as a link.
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		if((bShowPatientEducationLink && rcPatEducation.PtInRect(pt))) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAllIgnore();

		
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

CString CLabRequisitionDlg::GetIDStringFromArray(CDWordArray *dw, CString strDelim /*= ", "*/) 
{

	long nCount = dw->GetSize();
	CString strReturn;
	for (int i = 0; i < nCount; i++) {
		strReturn += AsString((long)dw->GetAt(i)) + strDelim;
	}
	
	//now take off the last delim
	strReturn = strReturn.Left(strReturn.GetLength() - strDelim.GetLength());

	return strReturn;
}

void CLabRequisitionDlg::OnSelChangingLabProvider(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// (b.cardillo 2006-11-28 13:09) - PLID 23265 - Since we're literally adding a 
			// reference to the object pointed to by lpOldSel, we need to call AddRef().  We 
			// would normally call Release() on *lppNewSel before setting it too, but since 
			// we already know it's NULL, there's nothing to call Release() on.
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("CLabRequisitionDlg::OnSelChangingLabProvider");
}

void CLabRequisitionDlg::OnAddInitialDiagnosis()
{
	//TES 5/14/2009 - PLID 33792 - Added, copied from OnAddDiagnosis (but much simpler, because it doesn't have to deal 
	// with the Microscopic Description
	try {
		// Have the user choose diagnosis codes to add. The "Convert(nvarchar(1000)" statement was
		// carried over from the legacy code; the description field is an ntext type
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "DiagCodes");
		// (z.manning 2008-10-28 11:43) - PLID 24630 - We can now link diag codes to lab diagnoses,
		// so if we have a link, include the diag code in the text in the multi select dialog.
		CString strFrom = "LabDiagnosisT LEFT JOIN DiagCodes ON LabDiagnosisT.DiagID = DiagCodes.ID";
		CString strValueField = "CASE WHEN DiagCodes.CodeNumber IS NOT NULL THEN DiagCodes.CodeNumber + ' - ' ELSE '' END + convert(nvarchar(1000), LabDiagnosisT.Description)";
		if (IDOK != dlg.Open(strFrom, "", "LabDiagnosisT.ID", strValueField, "Please choose one or more diagnoses", 1))
			return;
		
		CString strCurDescription;
		GetDlgItemText(IDC_INITIAL_DIAGNOSIS, strCurDescription);

		CString strNewDesc = dlg.GetMultiSelectString("\r\n");

		if(!strCurDescription.IsEmpty()) strCurDescription += "\r\n";
		strCurDescription += strNewDesc;

		//now update our description
		SetDlgItemText(IDC_INITIAL_DIAGNOSIS, strCurDescription);

	} NxCatchAll("Error in CLabRequisitionDlg::OnAddInitialDiagnosis()");
}

void CLabRequisitionDlg::OnEditInitialDiagnosis()
{
	try{
		//TES 5/14/2009 - PLID 33792 - Same as OnEditLabDescriptions()
		CLabEditDiagnosisDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll("Error in CLabRequisitionDlg::OnEditInitialDiagnosis");
}

// (c.haag 2010-11-30 11:30) - PLID 38633 - This is now split from marking complete
void CLabRequisitionDlg::OnSignRequisition()
{
	try {
		// (c.haag 2010-12-10 10:08) - PLID 38633 - sptDynamic0 was defined as "Mark Completed". 
		// We actually now check sptDynamic3 which is "Signed". Anyone who had sptDynamic0 permission
		// before has sptDynamic3 now.
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic3))
			return;

		BOOL bAlreadySigned = m_dtSigned.GetStatus() == COleDateTime::valid;

		CSignatureDlg dlgSign(this);
		dlgSign.SetSignature(m_strSignatureFileName, m_varSignatureInkData, m_varSignatureTextData);
		dlgSign.m_bRequireSignature = TRUE;
		if(bAlreadySigned) {
			dlgSign.m_bReadOnly = TRUE;
		}
		dlgSign.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordLab", 1, 0, GetCurrentUserName()) == 1);
		if(dlgSign.DoModal() != IDOK) {
			return;
		}

		if(bAlreadySigned) {
			return;
		}

		m_strSignatureFileName = dlgSign.GetSignatureFileName();
		m_varSignatureInkData = dlgSign.GetSignatureInkData();
		// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
		m_varSignatureTextData = dlgSign.GetSignatureTextData();

		GetDlgItem(IDC_SIGN_REQUISITION)->SetWindowText("View Signature");
		SetDlgItemText(IDC_EDIT_SIGNED_BY, GetCurrentUserName());
		m_signBtn.SetTextColor(RGB(0,0,0));
		m_nSignedByUserID = GetCurrentUserID();
		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
		if(!rs->eof){
			m_dtSigned = AdoFldDateTime(rs, "CurDateTime");
			SetDlgItemText(IDC_EDIT_SIGNATURE_DATE, FormatDateTimeForInterface(m_dtSigned));
		}
		else {
			m_dtSigned = COleDateTime::GetCurrentTime();
		}

		if(m_varSignatureTextData.vt != VT_EMPTY && m_varSignatureTextData.vt != VT_NULL) 
		{
			// (j.jones 2010-04-13 08:48) - PLID 38166 - the loaded text is the words "Date/Time",
			// we need to replace it with the actual date/time
			// (c.haag 2010-12-13 12:20) - PLID 41806 - Signing and completion dates used to be
			// synonymous, but now we only deal with signature dates.
			CNxInkPictureText nipt;
			nipt.LoadFromVariant(m_varSignatureTextData);
			CString strDate;
			if(dlgSign.GetSignatureDateOnly()) {
				strDate = FormatDateTimeForInterface(m_dtSigned, NULL, dtoDate);					
			}
			else {
				strDate = FormatDateTimeForInterface(m_dtSigned, DTF_STRIP_SECONDS, dtoDateTime);					
			}

			//technically, there should only be one SIGNATURE_DATE_STAMP_ID in the array,
			//and currently we don't support more than one stamp in the signature
			//anyways, but just incase, replace all instances of the SIGNATURE_DATE_STAMP_ID with
			//the current date/time
			for(int i=0; i<nipt.GetStringCount(); i++) {
				if(nipt.GetStampIDByIndex(i) == SIGNATURE_DATE_STAMP_ID) {
					nipt.SetStringByIndex(i, strDate);
				}
			}
			m_varSignatureTextData = nipt.GetAsVariant();
		}
	}
	NxCatchAll(__FUNCTION__);
}

//TES 11/25/2009 - PLID 36191 - All EMR Problem code was moved here from CLabEntryDlg

// (z.manning 2009-05-26 10:57) - PLID 34340 - Load any problems and links that are associated with this lab.
void CLabRequisitionDlg::LoadExistingProblems()
{
	try
	{
		// (z.manning 2009-06-23 16:38) - PLID 34340 - No point to this if no EMR license
		if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			return;
		}

		// (z.manning 2009-05-26 17:33) - PLID 34340 - Load all problems and links relevant to this lab.
		// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		// (s.tullis 2015-02-23 15:44) - PLID 64723 
		// (r.gonet 2015-03-09 19:24) - PLID 64723 - Added DoNotShowOnProblemPrompt.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT EmrProblemsT.ID, Description, EnteredDate, ModifiedDate, OnsetDate, StatusID \r\n"
			"	, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EmrRegardingType, EmrRegardingID, EmrDataID \r\n"
			"	, EmrProblemLinkT.ID AS LinkID \r\n"
			"	, EmrProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt \r\n"
			"FROM EmrProblemsT \r\n"
			"INNER JOIN EmrProblemLinkT ON EmrProblemsT.ID = EmrProblemLinkT.EmrProblemID \r\n"
			"WHERE EmrRegardingType = {CONST_INT} AND EmrRegardingID = {INT} AND EmrProblemsT.Deleted = 0 \r\n"
			, eprtLab, m_nLabID);

		for(; !prs->eof; prs->MoveNext())
		{
			FieldsPtr flds = prs->GetFields();
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);

			// (z.manning 2009-05-26 17:34) - PLID 34340 - Load the problem
			// (b.spivey, October 23, 2013) - PLID 58677 - Added CodeID
			// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			// (r.gonet 2015-03-09 19:24) - PLID 64723 - Pass DoNotShowOnProblemPrompt.
			CEmrProblem *pProblem = AllocateEmrProblem(AdoFldLong(flds,"ID"), m_nPatientID, AdoFldString(flds,"Description")
				, AdoFldDateTime(flds,"EnteredDate"), AdoFldDateTime(flds,"ModifiedDate"), AdoFldDateTime(flds,"OnsetDate",dtInvalid)
				, AdoFldLong(flds,"StatusID"), AdoFldLong(flds,"DiagCodeID",-1), AdoFldLong(flds,"DiagCodeID_ICD10",-1), AdoFldLong(flds,"ChronicityID",-1)
				, FALSE, AdoFldLong(flds, "CodeID", -1),AdoFldBool(flds,"DoNotShowOnCCDA",FALSE), AdoFldBool(flds, "DoNotShowOnProblemPrompt", FALSE));

			// (z.manning 2009-05-26 17:34) - PLID 34340 - Load the problem link
			CEmrProblemLink *pLink = new CEmrProblemLink(pProblem, AdoFldLong(flds,"LinkID"), (EMRProblemRegardingTypes)AdoFldLong(flds,"EmrRegardingType")
				, AdoFldLong(flds,"EmrRegardingID"), AdoFldLong(flds,"EmrDataID",-1));
			m_arypProblemLinks.Add(pLink);

			// (z.manning 2009-05-26 17:35) - PLID 34340 - Release our local reference to this problem
			pProblem->Release();
		}

		UpdateProblemButton();
		UpdateProblemLinkLabText();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-26 11:43) - PLID 34340
void CLabRequisitionDlg::OnBnClickedLabEntryProblemList()
{
	try
	{
		//pop-out a menu of options
		enum {
			miOpenList = 1,
			miNewProblem,
			miExistingProblem,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.InsertMenu(0, MF_ENABLED|MF_BYPOSITION, miOpenList, "&View Problems");
		// (c.haag 2009-06-29 16:02) - PLID 34249 - Needed New Problem text to be consistent with other modules
		mnu.InsertMenu(1, MF_ENABLED|MF_BYPOSITION, miNewProblem, "Link with New &Problem");
		mnu.InsertMenu(2, MF_ENABLED|MF_BYPOSITION, miExistingProblem, "&Link with Existing Problems");

		CRect rc;
		m_btnProblemList.GetWindowRect(&rc);

		UpdateProblemLinkLabText();
		
		// Pop up the menu to the right of the button
		int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, rc.right, rc.top, this, NULL);
		switch(nCmdId)
		{
			case miOpenList: {

				if(m_arypProblemLinks.GetSize() == 0) {
					MessageBox("There are no problems currently associated with this lab.");
					return;
				}

				//close the current problem list, if there is one
				CMainFrame *pFrame = GetMainFrame();
				if(pFrame) {
					pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
				}

				CEMRProblemListDlg dlg(this);
				// (z.manning 2009-05-29 10:01) - PLID 34345 - Need to pass in the lab entry dialog pointer
				dlg.SetLabRequisitionDlg(this);
				dlg.SetDefaultFilter(m_nPatientID, eprtLab, m_nLabID, GenerateCompletionAuditOld());				
				dlg.LoadFromProblemList(this, &m_arypProblemLinks);
				// (z.manning 2009-07-01 16:03) - PLID 34765 - When opening the problem list dialog from
				// a lab, we want to prevent access to problems that are not only locked due to others
				// having any EMN write tokens, but also any EMNs that the current user currently has.
				dlg.m_bExcludeCurrentUserFromEmnConcurrencyCheck = FALSE;
				// (z.manning 2009-07-01 17:42) - PLID 34765 - To be extra safe, include EMR level problems
				// when doing concurrency checks against EMN write tokens. This could potentially result
				// in us not being able to modify a problem that is not acutally locked on any EMN, but
				// that is a rare and unpractical case that we are willing to live with in the name of safety.
				dlg.m_bIncludeEmrLevelProblemsInConcurrencyCheck = TRUE;
				dlg.DoModal();

				UpdateProblemButton();
				
				}
				break;

			case miNewProblem: {	//Add new problem

				if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
					return;
				}

				//there are no EMR-level write access protections, so always allow adding new problems
				CEMRProblemEditDlg dlg(this);

				dlg.AddLinkedObjectInfo(-1, eprtLab, GenerateCompletionAuditOld(), "", m_nLabID);
				dlg.SetPatientID(m_nPatientID);

				if (IDOK == dlg.DoModal()) {
					if (!dlg.ProblemWasDeleted()) {

						COleDateTime dtInvalid;
						dtInvalid.SetStatus(COleDateTime::invalid);

						// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
						long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
						dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);
						// (s.tullis 2015-02-23 15:44) - PLID 64723 - Added DoNotshowonCCDA
						// (r.gonet 2015-03-09 19:24) - PLID 64723 - Added DoNotShowOnProblemPrompt.
						CEmrProblem *pProblem = AllocateEmrProblem(dlg.GetProblemID(), m_nPatientID, dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
							dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(), dlg.GetProblemDoNotShowOnCCDA(), 
							dlg.GetProblemDoNotShowOnProblemPrompt());
						CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtLab, m_nLabID, -1);
						m_arypProblemLinks.Add(pNewLink);
						pProblem->Release();

						UpdateProblemButton();
					}
				}
			}
			break;

			case miExistingProblem:
			{
				CArray<CEmrProblem*,CEmrProblem*> arypAllProblems;
				GetAllProblems(arypAllProblems, TRUE);
				CEMRProblemChooserDlg dlg(arypAllProblems, arypAllProblems, m_nPatientID, this);
				if(dlg.HasProblems())
				{
					CArray<CEmrProblem*,CEmrProblem*> arypSelectionsInMemory;
					CArray<long,long> arynSelectionsInData;
					if(IDOK == dlg.Open(arypSelectionsInMemory, arynSelectionsInData))
					{
						// User chose at least one problem
						
						// First, go through the problems that already exist in memory (and owned
						// by this lab), and create problem links for them.
						for(int nMemoryLinkIndex = 0; nMemoryLinkIndex < arypSelectionsInMemory.GetSize(); nMemoryLinkIndex++) {
							CEmrProblemLink* pNewLink = new CEmrProblemLink(arypSelectionsInMemory.GetAt(nMemoryLinkIndex), -1, eprtLab, m_nLabID, -1);
							m_arypProblemLinks.Add(pNewLink);
						}
						// Next, go through the problems that exist in data and not in memory, create
						// problem objects for them, and then links for those. Here's the neat part:
						// If the problem is already in memory, but linked with nothing; then the EMR
						// is smart enough to just give you the problem already in memory when calling
						// the function to allocate it.
						// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
						// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
						// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
						// (s.tullis 2015-02-23 15:44) - PLID 64723 - Added DoNotshowonCCDA
						// (r.gonet 2015-03-09 19:58) - PLID 65008 - Added DoNotShowOnProblemPrompt.
						if (arynSelectionsInData.GetSize() > 0)
						{
							_RecordsetPtr prs = CreateRecordset(
								"SELECT EMRProblemsT.ID, Description, StatusID, EnteredDate, ModifiedDate, \r\n"
								"	OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EMRProblemsT.PatientID, \r\n"
								"	EMRProblemsT.CodeID, EMRProblemsT.DoNotShowOnCCDA ,EMRProblemsT.DoNotShowOnProblemPrompt \r\n"
								"FROM EMRProblemsT \r\n"
								"WHERE Deleted = 0 AND ID IN (%s) \r\n"
								, ArrayAsString(arynSelectionsInData));
							for(; !prs->eof; prs->MoveNext()) {
								CEmrProblem* pNewProblem = AllocateEmrProblem(prs->Fields);
								CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtLab, m_nLabID, -1);
								m_arypProblemLinks.Add(pNewLink);
								pNewProblem->Release();
							}
						}

						UpdateProblemButton();

					}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
					else {
						// User changed their mind
					}
				}
				else {
					// Dialog has no visible problems
					MessageBox("There are no available problems to choose from.");
				}
			}
			break;
		}
		mnu.DestroyMenu();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-26 12:14) - PLID 34340 - Updates the icon of the problem button depending on whether
// or not there are problems associated with this lab.
void CLabRequisitionDlg::UpdateProblemButton()
{
	try
	{
		BOOL bHasProblems = FALSE, bHasOpen = FALSE, bHasClosed = FALSE;
		for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_arypProblemLinks.GetSize() && !bHasOpen; nProblemLinkIndex++)
		{
			CEmrProblemLink *pLink = m_arypProblemLinks.GetAt(nProblemLinkIndex);
			if(pLink != NULL) {
				if(!pLink->GetIsDeleted() && !pLink->GetProblem()->m_bIsDeleted) {
					bHasProblems = TRUE;
					if(pLink->GetProblem()->m_nStatusID == 2) {
						//we have a closed problem
						bHasClosed = TRUE;
					}
					else if(pLink->GetProblem()->m_nStatusID == 1) {
						//we have an open problem
						bHasOpen = TRUE;
					}
					else {
						// (z.manning 2009-05-29 15:12) - PLID 34340 - There are a few places in EMR that
						// do not currently handle this at all.  I will handle this here by querying data
						// but I made a note in 34402 to improve this when we fix it in EMR.
						_RecordsetPtr prsStatus = CreateParamRecordset(
							"SELECT Resolved FROM EmrProblemStatusT WHERE ID = {INT};"
							, pLink->GetProblem()->m_nStatusID);
						if(!prsStatus->eof) {
							if(AdoFldBool(prsStatus->GetFields(), "Resolved")) {
								bHasClosed = TRUE;
							}
							else {
								bHasOpen = TRUE;
							}
						}
					}
				}
			}
		}

		if(!bHasProblems) {
			m_btnProblemList.SetIcon(IDI_EMR_PROBLEM_EMPTY);
		}
		else if(bHasOpen) {
			m_btnProblemList.SetIcon(IDI_EMR_PROBLEM_FLAG);
		}
		else if(bHasClosed) {
			m_btnProblemList.SetIcon(IDI_EMR_PROBLEM_CLOSED);
		}
		else {
			// (z.manning 2009-05-26 12:57) - PLID 34340 - Shouldn't be possible
			ASSERT(FALSE);
		}
		m_btnProblemList.Invalidate();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-26 17:36) - PLID 34340
// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
// (s.tullis 2015-03-11 10:41) - PLID 64723 - Add DonotshowonCCDA
// (r.gonet 2015-03-09 19:58) - PLID 65008 - Added DoNotShowOnProblemPrompt.
CEmrProblem* CLabRequisitionDlg::AllocateEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
		long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA, BOOL bDoNotShowOnProblemPrompt)
{
	CEmrProblem* pProblem = GetProblemByID(nID);
	// (s.dhole 2014-01-27 10:00) - PLID 60472  If problem nID is not set then add as new problem object
	if(NULL == pProblem || nID <= 0) {
		// (b.spivey, October 23, 2013) - PLID 58677 - Added CodeID
		// (r.gonet 2015-03-09 19:58) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
		pProblem = new CEmrProblem(nID, nPatientID, strDescription, dtEnteredDate, dtModifiedDate, dtOnsetDate, nStatusID, nDiagCodeID_ICD9, nDiagCodeID_ICD10, nChronicityID, bIsModified, nCodeID,bDoNotShowOnCCDA, bDoNotShowOnProblemPrompt);
		// (z.manning 2009-05-26 17:36) - PLID 34340 - Do NOT add a reference here because we don't 
		// actually use the problem in this function. So the caller assumes control of the default
		// 1st reference.
	}
	else {
		// (z.manning 2009-05-26 17:38) - PLID 34340 - The caller is adding a reference to an already
		// existing problem pointer.
		pProblem->AddRef();
	}
	return pProblem;
}

// (z.manning 2009-05-27 14:52) - PLID 34340
CEmrProblem* CLabRequisitionDlg::AllocateEmrProblem(ADODB::FieldsPtr& f)
{
	CEmrProblem* pProblem = GetProblemByID(AdoFldLong(f,"ID"));
	if(NULL == pProblem) {
		pProblem = new CEmrProblem(f);
		// (z.manning 2009-05-26 17:36) - PLID 34340 - Do NOT add a reference here because we don't 
		// actually use the problem in this function. So the caller assumes control of the default
		// 1st reference.
	}
	else {
		// (z.manning 2009-05-26 17:38) - PLID 34340 - The caller is adding a reference to an already
		// existing problem pointer.
		pProblem->AddRef();
	}
	return pProblem;
}

// (z.manning 2009-05-26 15:41) - PLID 34340 - Generates the sql to save all relevant problems and problem links
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CLabRequisitionDlg::SaveProblems(long &nAuditTransactionID, IN OUT CDWordArray &arNewCDSInterventions)
{
	// (z.manning 2009-05-28 15:13) - PLID 34345 - Update the lab text in the problems as it's used when auditing.
	UpdateProblemLinkLabText();

	// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
	Nx::Quantum::Batch strSql;
	strSql += GenerateEMRBatchSaveNewObjectTableDeclaration();
	strSql.AddRaw("SET NOCOUNT ON \r\n");
	strSql.AddRaw("BEGIN TRY"); // (a.walling 2014-05-01 15:29) - PLID 62008 - begin sql try / catch block

	CMapPtrToPtr mapSavedObjects;
	BOOL bSavedAtLeastOneProblem = FALSE;

	// (z.manning 2009-05-26 17:40) - PLID 34340 - Save problems
	CArray<CEmrProblem*,CEmrProblem*> arypProblems;
	GetAllProblems(arypProblems, TRUE);
	for(int nProbIndex = 0; nProbIndex < arypProblems.GetSize(); nProbIndex++) {
		CEmrProblem *pProblem = arypProblems.GetAt(nProbIndex);
		Nx::Quantum::Batch strProblems;

		BOOL bWasDeleted = pProblem->m_bIsDeleted;

		// (c.haag 2009-05-28 13:11) - PLID 34277 - The last parameter is now a notification window. This dialog doesn't appear to need one.
		// (z.manning 2016-04-12 14:14) - NX-100140 - Pass in an empty set for deleted problems as we aren't concerned with that in labs
		std::set<long> set;
		SaveProblem(strProblems, nAuditTransactionID, pProblem, mapSavedObjects, m_nPatientID, GetExistingPatientName(m_nPatientID), NULL, set);

		// (j.jones 2009-06-18 16:54) - PLID 34487 - It is possible for the SaveProblem function to
		// abort saving and flag the problem as deleted, but it should NOT be possible to have it
		// happen from a lab. Still, if it were to happen, let's cleanly mark all its links as deleted,
		// though the saving code should be smart enough to handle that case.
		if(!bWasDeleted && pProblem->m_bIsDeleted) {

			//this shouldn't be possible in a lab, so if you hit this,
			//let's find out why and make sure it's ok
			ASSERT(FALSE);

			CArray<CEmrProblemLink*,CEmrProblemLink*> arypProblemLinks;
			GetAllProblemLinks(arypProblemLinks, pProblem, FALSE);
			for(int i=0; i<arypProblemLinks.GetSize(); i++) {
				CEmrProblemLink *pLink = arypProblemLinks.GetAt(i);
				pLink->SetDeleted();
			}
		}

		if(!strProblems.IsEmpty()) {
			bSavedAtLeastOneProblem = TRUE;
			strSql += strProblems;
		}
	}

	// (z.manning 2009-05-26 17:40) - PLID 34340 - Save problem links
	ASSERT(m_nLabID != -1);
	Nx::Quantum::Batch strProblemLinks;
	SaveProblemLinkArray(strProblemLinks, m_arypProblemLinks, AsString(m_nLabID), mapSavedObjects, nAuditTransactionID, m_nPatientID, GetExistingPatientName(m_nPatientID));
	if(!strProblemLinks.IsEmpty()) {
		bSavedAtLeastOneProblem = TRUE;
		strSql += strProblemLinks;
	}

	// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
	strSql.AddRaw("SET NOCOUNT OFF \r\n");
	strSql.AddStatement("SELECT ID, Type, ObjectPtr FROM #NewObjectsT \r\n"); // (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs

	// (a.walling 2014-05-01 15:29) - PLID 62008 - Ensure the transaction is rolled back and rethrow the exception.
	strSql.AddRaw("END TRY");
	strSql.AddRaw(
		"BEGIN CATCH \r\n"
		"IF @@TRANCOUNT > 0 BEGIN \r\n"
		"ROLLBACK TRANSACTION\r\n"
		"END\r\n"
		"SET NOCOUNT OFF\r\n"
		"DECLARE @errNumber INT, @errMessage NVARCHAR(4000), @errSeverity INT, @errState INT, @errProc NVARCHAR(128), @errLine INT \r\n"
		"SET @errNumber = ERROR_NUMBER(); SET @errMessage = ERROR_MESSAGE(); SET @errSeverity = ERROR_SEVERITY(); SET @errState = ERROR_STATE(); SET @errProc = ERROR_PROCEDURE(); SET @errLine = ERROR_LINE(); \r\n"
		"RAISERROR(N'Caught error %d from line %d in ''%s'': %s', @errSeverity, @errState, @errNumber, @errLine, @errProc, @errMessage) \r\n"
		"RETURN \r\n"
		"END CATCH"
	);

	// (a.walling 2014-02-05 11:23) - PLID 60546 - Execute the flattened batch according to UseAutoQuantumBatch, log if any errors
	_RecordsetPtr prsSaveProblems = ExecuteWithLog(strSql);
	for(; !prsSaveProblems->eof; prsSaveProblems->MoveNext())
	{
		BOOL bFound = FALSE;
		FieldsPtr flds = prsSaveProblems->GetFields();
		const long nObjectID = AdoFldLong(flds, "ID");
		const EmrSaveObjectType eObjectType = (EmrSaveObjectType)AdoFldLong(flds, "Type");
		const long nObjectPtr = AdoFldLong(flds, "ObjectPtr");

		if(eObjectType == esotProblem)
		{
			for(int nProbIndex = 0; nProbIndex < arypProblems.GetSize(); nProbIndex++) {
				CEmrProblem *pProblem = arypProblems.GetAt(nProbIndex);
				if(pProblem == (CEmrProblem*)nObjectPtr) {
					pProblem->m_nID = nObjectID;
					pProblem->AuditNew(nAuditTransactionID);
					bFound = TRUE;
				}
			}
		}
		else if(eObjectType == esotProblemLink)
		{
			for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++) {
				CEmrProblemLink *pProblemLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
				if(pProblemLink == (CEmrProblemLink*)nObjectPtr) {
					pProblemLink->SetID(nObjectID);
					pProblemLink->Audit(aeiEMNProblemLinkCreated, nAuditTransactionID, GetExistingPatientName(m_nPatientID));
					bFound = TRUE;
				}
			}
		}
		else {
			ThrowNxException("CLabEntryDlg::SaveProblems - Invalid object type - %i (ID = %li)", eObjectType, nObjectID);
		}

		ASSERT(bFound);
	}

	if(bSavedAtLeastOneProblem) {
		//TES 6/3/2009 - PLID 34371 - Update Patient Wellness data
		UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), m_nPatientID);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - Create todo alarms for decisions
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
		CClient::RefreshTable(NetUtils::EMRProblemsT, m_nPatientID);
	}
}

// (z.manning 2009-05-26 16:20) - PLID 34340
void CLabRequisitionDlg::GetAllProblems(CArray<CEmrProblem*,CEmrProblem*> &arypProblems, BOOL bIncludeDeleted)
{
	arypProblems.RemoveAll();
	for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++)
	{
		CEmrProblemLink *pLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
		if(bIncludeDeleted || !pLink->GetProblem()->m_bIsDeleted) {
			EnsureProblemInArray(arypProblems, pLink->GetProblem());
		}
	}
}

// (z.manning 2009-05-26 16:20) - PLID 34340
CEmrProblem* CLabRequisitionDlg::GetProblemByID(const long nProblemID)
{
	CArray<CEmrProblem*,CEmrProblem*> arypProblems;
	GetAllProblems(arypProblems, TRUE);
	for(int nProbIndex = 0; nProbIndex < arypProblems.GetSize(); nProbIndex++)
	{
		CEmrProblem *pProblem = arypProblems.GetAt(nProbIndex);
		if(pProblem != NULL && pProblem->m_nID == nProblemID) {
			return pProblem;
		}
	}

	return NULL;
}

// (z.manning 2009-05-28 15:47) - PLID 34345
void CLabRequisitionDlg::UpdateProblemLinkLabText()
{
	for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++) {
		CEmrProblemLink *pProblemLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
		pProblemLink->SetLabText(GenerateCompletionAuditOld());
	}
}

// (z.manning 2009-05-29 09:47) - PLID 34345
void CLabRequisitionDlg::GetAllProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*> &arypProblemLinks, CEmrProblem *pFilterProblem /* = NULL */, BOOL bIncludeDeleted /* = FALSE */)
{
	for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++)
	{
		CEmrProblemLink *pProblemLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
		CEmrProblem *pProblem = pProblemLink->GetProblem();
		if(pProblem != NULL && (pFilterProblem == NULL || pFilterProblem == pProblem) &&
			(!pProblemLink->GetIsDeleted() || bIncludeDeleted))
		{
			//add if it doesn't already exist
			BOOL bFound = FALSE;
			for(int nOutputProbLinkIndex = 0; nOutputProbLinkIndex < arypProblemLinks.GetSize() && !bFound; nOutputProbLinkIndex++) {
				if(arypProblemLinks.GetAt(nOutputProbLinkIndex) == pProblemLink) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				arypProblemLinks.Add(pProblemLink);
			}
		}
	}	
}

// (z.manning 2009-05-29 14:11) - PLID 29911
BOOL CLabRequisitionDlg::HasProblems()
{
	for(int nProbLinkIndex = 0; nProbLinkIndex < m_arypProblemLinks.GetSize(); nProbLinkIndex++) {
		CEmrProblemLink *pProblemLink = m_arypProblemLinks.GetAt(nProbLinkIndex);
		if(!pProblemLink->GetIsDeleted()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (j.jones 2009-06-08 09:12) - PLID 34487 - added ability to add a problem link from the problem list
void CLabRequisitionDlg::AddProblemLink(CEmrProblemLink* pNewLink)
{
	//make sure it doesn't already exist
	const int nProblemLinks = m_arypProblemLinks.GetSize();
	for (int i=0; i < nProblemLinks; i++) {
		if (m_arypProblemLinks[i] == pNewLink) {
			return;
		}
	}
	m_arypProblemLinks.Add(pNewLink);
}

//TES 11/25/2009 - PLID 36193 - We need to have access to the parent CLabEntryDlg
void CLabRequisitionDlg::SetLabEntryDlg(CLabEntryDlg *pDlg)
{
	m_pLabEntryDlg = pDlg;
}

BOOL CLabRequisitionDlg::Save()
{
	//TES 11/25/2009 - PLID 36193 - Just tell CLabEntryDlg
	return m_pLabEntryDlg->Save();
}

//TES 11/25/2009 - PLID 36193 - Access the current Specimen identifier
CString CLabRequisitionDlg::GetSpecimen()
{
	CString strSpecimen;
	GetDlgItemText(IDC_SPECIMEN, strSpecimen);
	return strSpecimen;
}

void CLabRequisitionDlg::OnKillFocusSpecimen()
{
	try {
		CString strSpecimen;
		GetDlgItemText(IDC_SPECIMEN, strSpecimen);
		//TES 11/25/2009 - PLID 36193 - Tell our parent that this changed (it may want to update its tab labels).
		m_pLabRequisitionsTabDlg->HandleSpecimenChange(this, strSpecimen);
	}NxCatchAll("Error in CLabRequisitionDlg::OnKillFocusSpecimen()");
}

//TES 11/25/2009 - PLID 36193 - Has this lab ever been saved?
BOOL CLabRequisitionDlg::IsNew()
{
	return (m_nLabID == -1);
}

void CLabRequisitionDlg::Close()
{
	//TES 11/25/2009 - PLID 36193 - Close the CLabEntryDlg, that will close us.
	//TES 10/13/2011 - PLID 45965 - If we just call EndDialog, MainFrame will still think this lab is open, and thus will fail to open it again.
	// CloseCleanup() is the appropriate function here
	m_pLabEntryDlg->CloseCleanup(IDOK);
}

long CLabRequisitionDlg::GetLabID()
{
	//TES 11/30/2009 - PLID 36193 - Just return our ID (even if it's -1, that's ok).
	return m_nLabID;
}

void CLabRequisitionDlg::OnLeftSide()
{
	try {
		if(IsDlgButtonChecked(IDC_RIGHT_SIDE)) {
			CheckDlgButton(IDC_RIGHT_SIDE, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLabRequisitionDlg::OnRightSide()
{
	try {
		if(IsDlgButtonChecked(IDC_LEFT_SIDE)) {
			CheckDlgButton(IDC_LEFT_SIDE, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}
void CLabRequisitionDlg::OnSelChosenReceivingLab(LPDISPATCH lpRow)
{
	try {
		//TES 2/1/2010 - PLID 37143 - Our parent needs to know which custom Request Form to use, pull it out of the datalist.
		IRowSettingsPtr pRow(lpRow);
		long nLabLocationID = -1;
		if(pRow == NULL) {
			m_pLabEntryDlg->SetRequestForm(-2);
			//TES 7/27/2012 - PLID 51849 - Also set the Results Form
			m_pLabEntryDlg->SetResultsForm(-2);
		}
		else {
			nLabLocationID = VarLong(pRow->GetValue(rlcLabLocationID),-1);
			m_pLabEntryDlg->SetRequestForm(VarLong(pRow->GetValue(rlcCustomReportNumber),-2));			
			//TES 7/27/2012 - PLID 51849 - Also set the Results Form
			m_pLabEntryDlg->SetResultsForm(VarLong(pRow->GetValue(rlcCustomReportNumber_Results),-2));
		}

		m_pLabEntryDlg->SetHL7Link(nLabLocationID);// (a.vengrofski 2010-05-12 17:24) - PLID <38547> - Update the HL7 Link box too

		// (j.jones 2010-06-25 14:10) - PLID 39185 - filter the 'To Be Ordered' list by location,
		// this means the list will be empty unless a location is selected
		// (we do not need to maintain any existing selection)
		CString strToBeOrderedWhere;
		strToBeOrderedWhere.Format("LocationID = %li", nLabLocationID);
		m_pToBeOrderedCombo->PutWhereClause(_bstr_t(strToBeOrderedWhere));
		m_pToBeOrderedCombo->Requery();

	}NxCatchAll(__FUNCTION__);

}

long CLabRequisitionDlg::GetRequestForm()
{
	//TES 2/1/2010 - PLID 37143 - Look it up in the datalist.
	IRowSettingsPtr pRow = m_pReceivingLabCombo->CurSel;
	if(pRow == NULL) {
		return -2;
	}
	else {
		return VarLong(pRow->GetValue(rlcCustomReportNumber),-2);
	}
}

long CLabRequisitionDlg::GetResultsForm()
{
	//TES 7/27/2012 - PLID 51849 - Look it up in the datalist.
	IRowSettingsPtr pRow = m_pReceivingLabCombo->CurSel;
	if(pRow == NULL) {
		return -2;
	}
	else {
		return VarLong(pRow->GetValue(rlcCustomReportNumber_Results),-2);
	}
}

//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
void CLabRequisitionDlg::SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide)
{
	m_nInitialAnatomicLocationID = nAnatomicLocationID;
	m_nInitialAnatomicQualifierID = nAnatomicQualifierID;
	m_asInitialAnatomicSide = asSide;
}

// (z.manning 2010-04-29 16:04) - PLID 38420
void CLabRequisitionDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);
		SetControlPositions();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-05 10:15) - PLID 37190
void CLabRequisitionDlg::SetDefaultInitialDiagnosis(CString strDefaultInitialDiagnosis)
{
	m_strDefaultInitialDiagnosis = strDefaultInitialDiagnosis;
}

// (z.manning 2010-05-05 10:50) - PLID 37190
CString CLabRequisitionDlg::GetLastSavedInitialDiagnosis()
{
	return m_strSavedInitialDiagnosis;
}

//TES 9/29/2010 - PLID 40644
void CLabRequisitionDlg::SetDefaultComments(const CString &strDefaultComments)
{
	m_strDefaultComments = strDefaultComments;
}

//TES 9/29/2010 - PLID 40644
CString CLabRequisitionDlg::GetLastSavedComments()
{
	return m_strSavedClinicalData;
}

//TES 9/29/2010 - PLID 40644
void CLabRequisitionDlg::SetDefaultLabLocationID(long nLabLocationID)
{
	m_nDefaultLabLocationID = nLabLocationID;
}

//TES 9/29/2010 - PLID 40644
long CLabRequisitionDlg::GetLastSavedLabLocationID()
{
	return m_nSavedLabLocationID;
}

//TES 9/29/2010 - PLID 40644
long CLabRequisitionDlg::GetLastSavedLocationID()
{
	return m_nSavedLocationID;
}

//TES 9/29/2010 - PLID 40644
void CLabRequisitionDlg::SetDefaultProviders(const CDWordArray &dwDefaultProviderIDs, const CString &strDefaultProviderNames)
{
	m_dwDefaultProviderIDs.Copy(dwDefaultProviderIDs);
	m_strDefaultProviderNames = strDefaultProviderNames;
}

//TES 9/29/2010 - PLID 40644
void CLabRequisitionDlg::GetLastSavedProviderIDs(OUT CDWordArray &dwProviderIDs)
{
	dwProviderIDs.Copy(m_dwSavedProviderIDs);
}

// (j.jones 2016-02-22 10:01) - PLID 68348 - added ability to get the current provider IDs
void CLabRequisitionDlg::GetCurrentProviderIDs(OUT CDWordArray &dwProviderIDs)
{
	dwProviderIDs.Copy(m_dwProviders);
}

// (j.jones 2016-02-22 10:01) - PLID 68348 - added ability to get the current location ID
long CLabRequisitionDlg::GetCurrentLocationID()
{
	long nLocationID = -1;
	if (m_pLocationCombo->GetIsComboBoxTextInUse() != FALSE) {
		//The location combo is still loading, so we will just get the current location that it is defaulting to.
		nLocationID = GetCurrentLocationID();
	}
	else {
		nLocationID = VarLong(m_pLocationCombo->CurSel->GetValue(lccLocationID));
	}
	return nLocationID;
}

//TES 9/29/2010 - PLID 40644
CString CLabRequisitionDlg::GetLastSavedProviderNames()
{
	return m_strSavedProviderNames;
}

// (j.jones 2010-05-06 11:15) - PLID 38524 - added OnDtnDatetimechangeBiopsyDate
void CLabRequisitionDlg::OnDtnDatetimechangeBiopsyDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
	
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

		m_pLabEntryDlg->CalculateAndDisplayPatientAge();

		//TES 7/16/2010 - PLID 39575 - If they uncheck the date box, disable the time box.
		_variant_t varDate = m_dtpBiopsy.GetValue();
		if(varDate.vt == VT_NULL) {
			m_nxtBiopsyTime->Enabled = g_cvarFalse;
		}
		else {
			m_nxtBiopsyTime->Enabled = g_cvarTrue;
		}
		
		*pResult = 0;

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-13 13:57) - PLID 37405
void CLabRequisitionDlg::AddPendingTodoID(const long nTaskID)
{
	m_arynPendingTodoIDs.Add(nTaskID);
}

// (z.manning 2010-05-13 13:57) - PLID 37405
void CLabRequisitionDlg::ClearPendingTodos()
{
	m_arynPendingTodoIDs.RemoveAll();
}

// (z.manning 2010-05-13 13:57) - PLID 37405
void CLabRequisitionDlg::DeletePendingTodos()
{
	for(int nTaskIndex = 0; nTaskIndex < m_arynPendingTodoIDs.GetSize(); nTaskIndex++) {
		long nTaskID = m_arynPendingTodoIDs.GetAt(nTaskIndex);
		TodoDelete(nTaskID, FALSE);
		// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
		CClient::RefreshTodoTable(nTaskID, m_nPatientID, -1, TableCheckerDetailIndex::tddisDeleted);
	}
	m_arynPendingTodoIDs.RemoveAll();
}

// (z.manning 2010-05-13 14:48) - PLID 37405
int CLabRequisitionDlg::GetPendingTodoCount()
{
	return m_arynPendingTodoIDs.GetSize();
}

// (d.lange 2010-12-30 17:14) - PLID 29065 - Added to edit Biopsy Types
void CLabRequisitionDlg::OnBnClickedEditBiopsyType()
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 75, "Edit Biopsy Type List");

		IRowSettingsPtr pRow = m_pBiopsyTypeCombo->GetCurSel();
		if(pRow) {
			m_nCurBiopsyTypeID = VarLong(pRow->GetValue(btcBiopsyTypeID), -1);
			if(m_nCurBiopsyTypeID != -1) {
				dlg.m_nCurIDInUse = m_nCurBiopsyTypeID;
			}
		}else {
			dlg.m_nCurIDInUse = m_nCurBiopsyTypeID;
		}

		dlg.DoModal();

		m_pBiopsyTypeCombo->Requery();

		m_pBiopsyTypeCombo->TrySetSelByColumn_Deprecated(0, m_nCurBiopsyTypeID);

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2011-01-03 14:08) - PLID 29065 - Added for Biopsy type
void CLabRequisitionDlg::TrySetSelFinishedBiopsyType(long nRowEnum, long nFlags)
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			if(m_nCurBiopsyTypeID > 0) {
				_RecordsetPtr rsBiopsy = CreateParamRecordset("SELECT Description FROM LabBiopsyTypeT WHERE ID = {INT}", m_nCurBiopsyTypeID);
				if(!rsBiopsy->eof) {
					m_pBiopsyTypeCombo->PutComboBoxText(_bstr_t(AdoFldString(rsBiopsy, "Description", "")));
					return;
				}
			}
			//Else if it failed to load anything, check if we need to clear the combo box text
			if(m_pBiopsyTypeCombo->GetIsComboBoxTextInUse() != FALSE)
				m_pBiopsyTypeCombo->PutComboBoxText("");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/14/2011) - PLID 45124 - Show a modeless window of the lab custom fields.
void CLabRequisitionDlg::OnBnClickedLabCustomFieldsButton()
{
	try {
		if(m_pcfTemplateInstance) {
			if(!m_pFieldsDlg) {
				m_pFieldsDlg = new CLabCustomFieldsDlg(m_pcfTemplateInstance, this);
				m_pFieldsDlg->Create(IDD_LAB_CUSTOM_FIELDS_DLG, this); // (a.walling 2012-07-16 12:10) - PLID 46648 - Specify ourselves as the parent
			}
			m_pFieldsDlg->ShowWindow(SW_SHOW);
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 28, 2013) - PLID 46295 - Get the anatomical location string from the controls in the req. 
CString CLabRequisitionDlg::GetAnatomicLocationString() 
{
	// (b.spivey, August 29, 2013) - PLID 46295 - only try this on biopsies, these datalists don't exist otherwise. 
	if(m_ltType == ltBiopsy) {
		CString strSide = "";

		//left or right side; this leads the anatomical location string so add a space at the end. 
		if(IsDlgButtonChecked(IDC_LEFT_SIDE)) {
			strSide = "left ";
		}
		else if (IsDlgButtonChecked(IDC_RIGHT_SIDE)) {
			strSide = "right ";
		}

		//Location (face, body, stuff like that) 
		CString strAnatomyLocation = ""; 
		IRowSettingsPtr pRow = m_pAnatomyCombo->GetCurSel(); 
		if (pRow) {
			strAnatomyLocation = VarString(pRow->GetValue(alcDescription), "");
		}
		else if (m_nSavedAnatomyID > 0) {
			// (b.spivey, August 29, 2013) - PLID 46295 - fail safe
			// (b.spivey, September 05, 2013) - PLID 46295 - proper fail safe
			// (b.spivey, September 05, 2013) - PLID 46295 - actual proper fail safe
			strAnatomyLocation = VarString(m_pAnatomyCombo->GetComboBoxText(), "");
		}

		//Qualifier (exterior, dorsal, etc.)
		CString strAnatomyQualifier = ""; 
		pRow = m_pAnatomyQualifiersCombo->GetCurSel(); 
		if (pRow) {
			strAnatomyQualifier = VarString(pRow->GetValue(alqcName), "");
			//Qualifier comes before location and after side, so if we have one we need to add a space here. 
			strAnatomyQualifier += (strAnatomyQualifier.IsEmpty() ? "" : " "); 
		}
		else if (m_nSavedAnatomyQualifierID > 0) {
			// (b.spivey, August 29, 2013) - PLID 46295 - fail safe
			// (b.spivey, September 05, 2013) - PLID 46295 - proper fail safe
			// (b.spivey, September 05, 2013) - PLID 46295 - actual proper fail safe
			strAnatomyQualifier = VarString(m_pAnatomyQualifiersCombo->GetComboBoxText(), "");
			//Qualifier comes before location and after side, so if we have one we need to add a space here. 
			strAnatomyQualifier += (strAnatomyQualifier.IsEmpty() ? "" : " "); 		
		}

		//concat and return. 
		CString strReturn = strSide + strAnatomyQualifier + strAnatomyLocation; 
		return strReturn; 
	}

	return ""; 
}

// (j.jones 2013-10-22 12:52) - PLID 58979 - added infobutton
void CLabRequisitionDlg::OnBtnPtEducation()
{
	try {

		//force a save
		if(!Save()){
			return;
		}

		if(m_nLabID == -1) {
			//should not be possible, because we forced a save
			ASSERT(FALSE);
			ThrowNxException("No valid lab ID was found.");
		}

		//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
		LookupMedlinePlusInformationViaURL(this, mlpLabID, m_nLabID);

	}NxCatchAll(__FUNCTION__);
}
